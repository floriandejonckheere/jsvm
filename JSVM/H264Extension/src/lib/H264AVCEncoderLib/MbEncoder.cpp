/*
********************************************************************************

NOTE - One of the two copyright statements below may be chosen
       that applies for the software.

********************************************************************************

This software module was originally developed by

Heiko Schwarz    (Fraunhofer HHI),
Tobias Hinz      (Fraunhofer HHI),
Karsten Suehring (Fraunhofer HHI)

in the course of development of the ISO/IEC 14496-10:2005 Amd.1 (Scalable Video
Coding) for reference purposes and its performance may not have been optimized.
This software module is an implementation of one or more tools as specified by
the ISO/IEC 14496-10:2005 Amd.1 (Scalable Video Coding).

Those intending to use this software module in products are advised that its
use may infringe existing patents. ISO/IEC have no liability for use of this
software module or modifications thereof.

Assurance that the originally developed software module can be used
(1) in the ISO/IEC 14496-10:2005 Amd.1 (Scalable Video Coding) once the
ISO/IEC 14496-10:2005 Amd.1 (Scalable Video Coding) has been adopted; and
(2) to develop the ISO/IEC 14496-10:2005 Amd.1 (Scalable Video Coding):

To the extent that Fraunhofer HHI owns patent rights that would be required to
make, use, or sell the originally developed software module or portions thereof
included in the ISO/IEC 14496-10:2005 Amd.1 (Scalable Video Coding) in a
conforming product, Fraunhofer HHI will assure the ISO/IEC that it is willing
to negotiate licenses under reasonable and non-discriminatory terms and
conditions with applicants throughout the world.

Fraunhofer HHI retains full right to modify and use the code for its own
purpose, assign or donate the code to a third party and to inhibit third
parties from using the code for products that do not conform to MPEG-related
ITU Recommendations and/or ISO/IEC International Standards.

This copyright notice must be included in all copies or derivative works.
Copyright (c) ISO/IEC 2005.

********************************************************************************

COPYRIGHT AND WARRANTY INFORMATION

Copyright 2005, International Telecommunications Union, Geneva

The Fraunhofer HHI hereby donate this source code to the ITU, with the following
understanding:
    1. Fraunhofer HHI retain the right to do whatever they wish with the
       contributed source code, without limit.
    2. Fraunhofer HHI retain full patent rights (if any exist) in the technical
       content of techniques and algorithms herein.
    3. The ITU shall make this code available to anyone, free of license or
       royalty fees.

DISCLAIMER OF WARRANTY

These software programs are available to the user without any license fee or
royalty on an "as is" basis. The ITU disclaims any and all warranties, whether
express, implied, or statutory, including any implied warranties of
merchantability or of fitness for a particular purpose. In no event shall the
contributor or the ITU be liable for any incidental, punitive, or consequential
damages of any kind whatsoever arising from the use of these programs.

This disclaimer of warranty extends to the user of these programs and user's
customers, employees, agents, transferees, successors, and assigns.

The ITU does not represent or warrant that the programs furnished hereunder are
free of infringement of any third-party patents. Commercial implementations of
ITU-T Recommendations, including shareware, may be subject to royalty fees to
patent holders. Information regarding the ITU-T patent policy is available from
the ITU Web site at http://www.itu.int.

THIS IS NOT A GRANT OF PATENT RIGHTS - SEE THE ITU-T PATENT POLICY.

********************************************************************************
*/




#include "H264AVCEncoderLib.h"
#include "MbEncoder.h"
#include "H264AVCCommonLib/Tables.h"


#include "H264AVCCommonLib/Transform.h"

#include "IntraPredictionSearch.h"
#include "MotionEstimation.h"
#include "CodingParameter.h"

#include "RateDistortionIf.h"


H264AVC_NAMESPACE_BEGIN

typedef MotionEstimation::MEBiSearchParameters  BSParams;


const UChar g_aucFrameBits[32] =
{
  0,
  1,
  3, 3,
  5, 5, 5, 5,
  7, 7, 7, 7, 7, 7, 7, 7,
  9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9
};

//JVT-V079 Low-complexity MB mode decision {
#define SHIFT_QP 12

static const Int g_iQP2quant[40] = {
 1, 1, 1, 1, 2, 2, 2, 2,
 3, 3, 3, 4, 4, 4, 5, 6,
 6, 7, 8, 9, 10, 11, 13, 14,
 16, 18, 20, 23, 25, 29, 32, 36,
 40, 45, 51, 57, 64, 72, 81, 91
};
//JVT-V079 Low-complexity MB mode decision }

__inline UChar MbEncoder::xGetFrameBits( ListIdx eLstIdx, Int iRefPic )
{
  return (m_uiMaxRefPics[eLstIdx] > 1) ? g_aucFrameBits[iRefPic] : 0;
}



MbEncoder::MbEncoder():
  m_pcCodingParameter( NULL ),
  m_pcTransform( NULL ),
  m_pcIntraPrediction( NULL ),
  m_pcMotionEstimation( NULL ),
  m_pcRateDistortionIf( NULL ),
  m_pcXDistortion( 0 ),
  bInitDone( false ),
  m_bISlice( false ),
  m_bBSlice( false ),
  m_bCabac( false ),
  m_pcIntMbBestData( NULL ),
  m_pcIntMbTempData( NULL ),
  m_pcIntMbBest8x8Data( NULL ),
  m_pcIntMbTemp8x8Data( NULL ),
  m_pcIntMbBestIntraChroma( NULL ),
  m_pcIntOrgMbPelData( NULL ),
  m_pcIntPicBuffer( NULL ),
  m_pcIntraPredPicBuffer( NULL ),
  m_BitCounter( NULL )
  ,m_bLARDOEnable( false ), //JVT-R057 LA-RDO
  m_uiMBSSD( 0 ),           //JVT-R057 LA-RDO
  m_pcFrameEcEp ( NULL ),   //JVT-R057 LA-RDO
  m_iEpRef ( 0 ),           //JVT-R057 LA-RDO
  m_dWr0 ( 0.5 ),           //JVT-R057 LA-RDO
  m_dWr1 ( 0.5 )            //JVT-R057 LA-RDO
  //S051{
  ,m_bUseBDir(true)
  //S051}
  //JVT-U106 Behaviour at slice boundaries{
  ,m_bIntraBLFlag( true )
  //JVT-U106 Behaviour at slice boundaries}
{
  m_uiMaxRefFrames[LIST_0] = m_uiMaxRefFrames[LIST_1] = 0;
  m_uiMaxRefPics  [LIST_0] = m_uiMaxRefPics  [LIST_1] = 0;
}





MbEncoder::~MbEncoder()
{
}



ErrVal
MbEncoder::create( MbEncoder*& rpcMbEncoder )
{
  rpcMbEncoder = new MbEncoder;
  ROT( NULL == rpcMbEncoder );

  return Err::m_nOK;
}


ErrVal
MbEncoder::destroy()
{
  delete this;

  return Err::m_nOK;
}


ErrVal
MbEncoder::initSlice( const SliceHeader& rcSH )
{
  m_bISlice                 = rcSH.isIntraSlice ();
  m_bBSlice                 = rcSH.isBSlice();
  m_bCabac                  = rcSH.getPPS().getEntropyCodingModeFlag();
  m_uiMaxRefFrames[LIST_0]  = rcSH.getNumRefIdxActive( LIST_0 );
  m_uiMaxRefFrames[LIST_1]  = rcSH.getNumRefIdxActive( LIST_1 );
  m_uiMaxRefPics  [LIST_0]  = m_uiMaxRefPics  [LIST_1] = 0;
  RNOK( MbCoder::initSlice( rcSH, this, MbEncoder::m_pcRateDistortionIf ) );

  m_BitCounter            = (BitWriteBufferIf*)this;
  RNOK( UvlcWriter::init( m_BitCounter ) );
  RNOK( UvlcWriter::startSlice( rcSH ) );

  m_pcIntMbBestData       = &m_acIntMbTempData[0];
  m_pcIntMbTempData       = &m_acIntMbTempData[1];
  m_pcIntMbBest8x8Data    = &m_acIntMbTempData[2];
  m_pcIntMbTemp8x8Data    = &m_acIntMbTempData[3];

  return Err::m_nOK;
}



ErrVal
MbEncoder::init( Transform*             pcTransform,
                 IntraPredictionSearch* pcIntraPrediction,
                 MotionEstimation*      pcMotionEstimation,
                 CodingParameter*       pcCodingParameter,
                 RateDistortionIf*      pcRateDistortionIf,
                 XDistortion*           pcXDistortion )
{
  ROT( NULL == pcTransform );
  ROT( NULL == pcIntraPrediction );
  ROT( NULL == pcMotionEstimation );
  ROT( NULL == pcCodingParameter );
  ROT( NULL == pcRateDistortionIf );
  ROT( NULL == pcXDistortion );

  m_pcRateDistortionIf    = pcRateDistortionIf;
  m_pcCodingParameter     = pcCodingParameter;
  m_pcXDistortion         = pcXDistortion;

  m_pcTransform           = pcTransform;
  m_pcIntraPrediction     = pcIntraPrediction;
  m_pcMotionEstimation    = pcMotionEstimation;
  bInitDone               = true;

  return Err::m_nOK;
}


ErrVal
MbEncoder::uninit()
{
  RNOK( MbCoder::uninit() );

  m_pcTransform = NULL;
  m_pcIntraPrediction = NULL;
  m_pcMotionEstimation = NULL;
  bInitDone = false;
  return Err::m_nOK;
}


ErrVal
MbEncoder::encodeIntra( MbDataAccess&  rcMbDataAccess,
                        MbDataAccess*  pcMbDataAccessBase,
                        Frame*			pcOrgFrame,
                        Frame*      pcFrame,
                        Frame*      pcRecSubband,
                        Frame*      pcBaseLayer,
                        Frame*      pcPredSignal,
                        Double        dLambda,
                        Double&       rdCost )
{
  ROF( bInitDone );

  UInt  uiQp    = rcMbDataAccess.getMbData().getQp();

  if (rcMbDataAccess.getSH().getTCoeffLevelPredictionFlag())
  {
    uiQp    = rcMbDataAccess.getLastQp();        // to avoid QP get changed by fixMacroblockQP
    rcMbDataAccess.getMbData().setQp( uiQp );
  }

  RNOK( m_pcRateDistortionIf->setMbQpLambda( rcMbDataAccess, uiQp, dLambda ) )

  m_pcIntMbBestIntraChroma  = NULL;
  m_pcIntMbBestData   ->init( rcMbDataAccess );
  m_pcIntMbTempData   ->init( rcMbDataAccess );
  m_pcIntMbBest8x8Data->init( rcMbDataAccess );
  m_pcIntMbTemp8x8Data->init( rcMbDataAccess );

  m_pcIntPicBuffer = pcFrame->getFullPelYuvBuffer();
  m_pcXDistortion->loadOrgMbPelData( pcOrgFrame->getFullPelYuvBuffer(), m_pcIntOrgMbPelData );
  m_pcTransform->setQp( rcMbDataAccess, true );

  Bool bBaseLayerAvailable = (NULL != pcMbDataAccessBase) && ! rcMbDataAccess.getSH().getNoInterLayerPredFlag();

  if( bBaseLayerAvailable )
  {
    ROF ( pcBaseLayer );
    //===== check intra base mode (if base layer is available) =====

    if( pcMbDataAccessBase->getMbData().isIntra() )
    if ( pcMbDataAccessBase->getMbData().getInCropWindowFlag() )
      RNOK( xEstimateMbIntraBL( rcMbDataAccess, m_pcIntMbTempData, m_pcIntMbBestData, pcBaseLayer, false, pcMbDataAccessBase  ) );
  }

  if( !bBaseLayerAvailable || rcMbDataAccess.getSH().getSPS().getNumberOfQualityLevelsCGSSNR() == 1 )
  if( rcMbDataAccess.getSH().getNoInterLayerPredFlag() ||
      rcMbDataAccess.getSH().getAdaptiveBaseModeFlag() )
  {
    //===== check normal intra modes (if base layer or adaptive prediction) =====
    RNOK( xEstimateMbIntra16( m_pcIntMbTempData, m_pcIntMbBestData,              false  ) );
    RNOK( xEstimateMbIntra8 ( m_pcIntMbTempData, m_pcIntMbBestData,              false  ) );
    RNOK( xEstimateMbIntra4 ( rcMbDataAccess, m_pcIntMbTempData, m_pcIntMbBestData,              false  ) );
    RNOK( xEstimateMbPCM    ( m_pcIntMbTempData, m_pcIntMbBestData,              false  ) );
  }

  RNOK( m_pcRateDistortionIf->fixMacroblockQP( *m_pcIntMbBestData ) );
  xStoreEstimation( rcMbDataAccess, *m_pcIntMbBestData, pcRecSubband, pcPredSignal, false, NULL );


  //JVT-R057 LA-RDO{
  if(m_bLARDOEnable)
  {
	  Int x,y;
	  MbMode mode=rcMbDataAccess.getMbData().getMbMode();
	  Bool bInter= !rcMbDataAccess.getSH().getNoInterLayerPredFlag();
	  Int KBlock = m_pcIntPicBuffer->getLWidth()/4;

	  Int blockX=rcMbDataAccess.getMbX()*4;
	  Int blockY=rcMbDataAccess.getMbY()*4;
	  Int ec_rec,ec_ep;
	  m_pcIntPicBuffer->getYuvBufferCtrl().initMb();

	  UInt p=1;
	  UInt q=100;
	  if(bInter)
	  {
		  for(UInt i=0;i<=m_uiLayerID;i++)
		  {
			  //Bug_Fix JVT-R057 0806{
			  p=p*(100-m_auiPLR[i]);
			  //Bug_Fix JVT-R057 0806}
		  }
		  //q=(UInt)pow(100,(m_uiLayerID+1));
       q=(UInt)pow(100.0,(int)(m_uiLayerID+1));
	  }
	  else
	  {
		  p=100-m_auiPLR[m_uiLayerID];
	  }

	  if(mode!=INTRA_BL)
	  {
		  for(y=blockY;y<(blockY+4);y++)
			  for(x=blockX;x<(blockX+4);x++)
			  {
				  if(m_pcFrameEcEp)
					  ec_ep=m_pcFrameEcEp->getChannelDistortion()[y*KBlock+x];
				  else
					  ec_ep=0;
				  m_pcIntPicBuffer->getYuvBufferCtrl().initMb();
				  if(m_pcFrameEcEp)
					  ec_rec=GetEC_REC(m_pcIntPicBuffer,m_pcFrameEcEp->getFullPelYuvBuffer(),x,y);
				  else
					  ec_rec=0;
				  pcFrame->getChannelDistortion()[y*KBlock+x]=(q-p)*(ec_rec+ec_ep)/q;
			  }
	  }
	  else
	  {
		  Int ep_base;
		  Int blockIndex;
		  Int xx,yy;
		  for(y=blockY;y<(blockY+4);y++)
			  for(x=blockX;x<(blockX+4);x++)
			  {
				  if(m_pcFrameEcEp)
					  ec_ep=m_pcFrameEcEp->getChannelDistortion()[y*KBlock+x];
				  else
					  ec_ep=0;
				  m_pcIntPicBuffer->getYuvBufferCtrl().initMb();
				  if(m_pcFrameEcEp)
					  ec_rec=GetEC_REC(m_pcIntPicBuffer,m_pcFrameEcEp->getFullPelYuvBuffer(),x,y);
				  else
					  ec_rec=0;

				  xx=(Int)(x/m_aadRatio[m_uiLayerID][0]);
				  yy=(Int)(y/m_aadRatio[m_uiLayerID][1]);
				  blockIndex=yy*(Int)(KBlock/m_aadRatio[m_uiLayerID][0])+xx;

				  ep_base=pcBaseLayer->getChannelDistortion()[blockIndex];
				  pcFrame->getChannelDistortion()[y*KBlock+x]=(p*ep_base+(q-p)*(ec_rec+ec_ep))/q;
			  }
	  }
  }
  //JVT-R057 LA-RDO}


  rdCost = m_pcIntMbBestData->rdCost();
  m_pcIntMbBestData   ->uninit();
  m_pcIntMbTempData   ->uninit();
  m_pcIntMbBest8x8Data->uninit();
  m_pcIntMbTemp8x8Data->uninit();

  return Err::m_nOK;
}






ErrVal
MbEncoder::xCheckInterMbMode8x8( IntMbTempData*&   rpcMbTempData,
                                 IntMbTempData*&   rpcMbBestData,
                                 IntMbTempData*    pcMbRefData,
                                 RefFrameList&     rcRefFrameList0,
                                 RefFrameList&     rcRefFrameList1,
                                 MbDataAccess*     pcMbDataAccessBaseMotion,
                                 Frame*         pcBaseLayerRec
                                 )
{
  ROTRS( ! rpcMbTempData->getSH().getPPS().getTransform8x8ModeFlag(), Err::m_nOK );
  ROTRS( ! pcMbRefData->is8x8TrafoFlagPresent( rpcMbTempData->getSH().getSPS().getDirect8x8InferenceFlag() ), Err::m_nOK );

  if( pcMbRefData == rpcMbTempData )
  {
    rpcMbTempData->clearCost          ();
    rpcMbTempData->getMbTCoeffs       ().clear();
    rpcMbTempData->setTransformSize8x8( true );
  }
  else
  {
    rpcMbTempData->clear                ();
    rpcMbTempData->setMbMode            (           pcMbRefData->getMbMode            () );
    rpcMbTempData->setBLSkipFlag        (           pcMbRefData->getBLSkipFlag        () );
    rpcMbTempData->setResidualPredFlags (           pcMbRefData->getResidualPredFlags () );
    rpcMbTempData->setBlkMode           ( B_8x8_0,  pcMbRefData->getBlkMode           ( B_8x8_0 ) );
    rpcMbTempData->setBlkMode           ( B_8x8_1,  pcMbRefData->getBlkMode           ( B_8x8_1 ) );
    rpcMbTempData->setBlkMode           ( B_8x8_2,  pcMbRefData->getBlkMode           ( B_8x8_2 ) );
    rpcMbTempData->setBlkMode           ( B_8x8_3,  pcMbRefData->getBlkMode           ( B_8x8_3 ) );
    rpcMbTempData->setTransformSize8x8  ( true );

    rpcMbTempData->getMbMotionData( LIST_0 ).copyFrom( pcMbRefData->getMbMotionData( LIST_0 ) );
    rpcMbTempData->getMbMotionData( LIST_1 ).copyFrom( pcMbRefData->getMbMotionData( LIST_1 ) );
    rpcMbTempData->getMbMvdData   ( LIST_0 ).copyFrom( pcMbRefData->getMbMvdData   ( LIST_0 ) );
    rpcMbTempData->getMbMvdData   ( LIST_1 ).copyFrom( pcMbRefData->getMbMvdData   ( LIST_1 ) );
  }

  RNOK( xSetRdCost8x8InterMb( *rpcMbTempData, pcMbDataAccessBaseMotion, rcRefFrameList0, rcRefFrameList1,
                              false, 0, false, pcBaseLayerRec ) );

  //JVT-R057 LA-RDO{
  if(m_bLARDOEnable)
	  rpcMbTempData->rdCost()+=getEpRef();
  //JVT-R057 LA-RDO}

  RNOK( xCheckBestEstimation(  rpcMbTempData, rpcMbBestData ) );

  return Err::m_nOK;
}



ErrVal
MbEncoder::encodeMacroblock( MbDataAccess&  rcMbDataAccess,
                             Frame*      pcFrame,
                             RefFrameList&  rcList0,
                             RefFrameList&  rcList1,
                             UInt           uiNumMaxIter,
                             UInt           uiIterSearchRange,
                             Double         dLambda )
{
  ROF( m_bInitDone );

  UInt  uiQp = rcMbDataAccess.getMbData().getQp();
  RNOK( m_pcRateDistortionIf->setMbQpLambda( rcMbDataAccess, uiQp, dLambda ) );

  m_pcIntMbBestIntraChroma  = NULL;
  m_pcIntMbBestData   ->init( rcMbDataAccess );
  m_pcIntMbTempData   ->init( rcMbDataAccess );
  m_pcIntMbBest8x8Data->init( rcMbDataAccess );
  m_pcIntMbTemp8x8Data->init( rcMbDataAccess );

  m_pcIntPicBuffer = pcFrame->getFullPelYuvBuffer();
  m_pcXDistortion->loadOrgMbPelData( m_pcIntPicBuffer, m_pcIntOrgMbPelData );
  m_pcTransform->setQp( rcMbDataAccess, rcMbDataAccess.getSH().getTemporalId() == 0 );

  //====== evaluate macroblock modes ======
  if( rcMbDataAccess.getSH().isPSlice() )
  {
    RNOK( xEstimateMbSkip     ( m_pcIntMbTempData,  m_pcIntMbBestData,  rcList0,  rcList1 ) );
  }
  if( rcMbDataAccess.getSH().isBSlice() )
  {
    RNOK( xEstimateMbDirect   ( m_pcIntMbTempData,  m_pcIntMbBestData,  rcList0,  rcList1,                                                 NULL, false,  rcMbDataAccess.getMbData().getQp() ) );
  }
  if( rcMbDataAccess.getSH().isPSlice() || rcMbDataAccess.getSH().isBSlice() )
  {
    RNOK( xEstimateMb16x16    ( m_pcIntMbTempData,  m_pcIntMbBestData,  rcList0,  rcList1,  false,  uiNumMaxIter, uiIterSearchRange,  false,  NULL, false ) );
    RNOK( xEstimateMb16x8     ( m_pcIntMbTempData,  m_pcIntMbBestData,  rcList0,  rcList1,  false,  uiNumMaxIter, uiIterSearchRange,  false,  NULL, false ) );
    RNOK( xEstimateMb8x16     ( m_pcIntMbTempData,  m_pcIntMbBestData,  rcList0,  rcList1,  false,  uiNumMaxIter, uiIterSearchRange,  false,  NULL, false ) );
    RNOK( xEstimateMb8x8      ( m_pcIntMbTempData,  m_pcIntMbBestData,  rcList0,  rcList1,  false,  uiNumMaxIter, uiIterSearchRange,  false,  NULL, false ) );
    RNOK( xEstimateMb8x8Frext ( m_pcIntMbTempData,  m_pcIntMbBestData,  rcList0,  rcList1,  false,  uiNumMaxIter, uiIterSearchRange,  false,  NULL, false ) );
  }
  RNOK(   xEstimateMbIntra16  ( m_pcIntMbTempData,  m_pcIntMbBestData,  rcMbDataAccess.getSH().isBSlice() ) );
  RNOK(   xEstimateMbIntra8   ( m_pcIntMbTempData,  m_pcIntMbBestData,  rcMbDataAccess.getSH().isBSlice() ) );
  RNOK(   xEstimateMbIntra4   ( rcMbDataAccess, m_pcIntMbTempData,  m_pcIntMbBestData,  rcMbDataAccess.getSH().isBSlice() ) );
  RNOK(   xEstimateMbPCM      ( m_pcIntMbTempData,  m_pcIntMbBestData,  rcMbDataAccess.getSH().isBSlice() ) );

  //===== fix estimation =====
  RNOK( m_pcRateDistortionIf->fixMacroblockQP( *m_pcIntMbBestData ) );
  xStoreEstimation( rcMbDataAccess, *m_pcIntMbBestData, NULL, NULL, false, NULL );

  //===== uninit =====
  m_pcIntMbBestData   ->uninit();
  m_pcIntMbTempData   ->uninit();
  m_pcIntMbBest8x8Data->uninit();
  m_pcIntMbTemp8x8Data->uninit();

  return Err::m_nOK;
}



ErrVal
MbEncoder::encodeInterP( MbDataAccess&    rcMbDataAccess,
                         MbDataAccess*    pcMbDataAccessBase,
                         Int              iSpatialScalabilityType,
                         Frame*        pcOrgFrame,
                         Frame*        pcFrame,
                         Frame*        pcRecSubband,
                         Frame*        pcPredSignal,
                         Frame*        pcBaseLayerRec,
                         Frame*        pcBaseLayerSbb,
                         RefFrameList&    rcRefFrameList0,
                         RefFrameList*    pcRefFrameList0Base,
                         Double           dLambda,
                         Double&          rdCost,
                         Bool             bSkipModeAllowed )
{
  ROF( bInitDone );

  UInt uiQp  = rcMbDataAccess.getMbData().getQp();
  if (rcMbDataAccess.getSH().getTCoeffLevelPredictionFlag())
  {
    uiQp    = rcMbDataAccess.getLastQp();        // to avoid QP get changed by fixMacroblockQP
    rcMbDataAccess.getMbData().setQp( uiQp );
  }

  RNOK( m_pcRateDistortionIf->setMbQpLambda( rcMbDataAccess, uiQp, dLambda ) )

  rcMbDataAccess.setMbDataAccessBase(pcMbDataAccessBase);
  m_pcIntMbBestIntraChroma  = NULL;
  m_pcIntMbBestData   ->init( rcMbDataAccess );
  m_pcIntMbTempData   ->init( rcMbDataAccess );
  m_pcIntMbBest8x8Data->init( rcMbDataAccess );
  m_pcIntMbTemp8x8Data->init( rcMbDataAccess );

  m_pcIntPicBuffer = pcFrame->getFullPelYuvBuffer();
  m_pcXDistortion->loadOrgMbPelData( pcOrgFrame->getFullPelYuvBuffer(), m_pcIntOrgMbPelData );
  m_pcTransform->setQp      ( rcMbDataAccess, true );
  m_pcTransform->setClipMode( false );


  //JVT-V079 Low-complexity MB mode decision
  Bool bLowComplexMbEnable = m_bLowComplexMbEnable[rcMbDataAccess.getSH().getDependencyId()];
  
  RefFrameList   cRefFrameList1;
  YuvMbBuffer cBaseLayerBuffer;

  Bool bBaseLayerAvailable = (NULL != pcMbDataAccessBase) && ! rcMbDataAccess.getSH().getNoInterLayerPredFlag();

  Bool bIntraEnable = ! rcMbDataAccess.isFieldMbInMbaffFrame() || rcMbDataAccess.isTopMb() || rcMbDataAccess.getMbDataComplementary().isIntra();
  Bool bInterEnable = ! rcMbDataAccess.isFieldMbInMbaffFrame() || rcMbDataAccess.isTopMb() || ! rcMbDataAccess.getMbDataComplementary().isIntra();

  //--  JVT-T037
  Bool  b = false;
  if ( rcMbDataAccess.isFieldMbInMbaffFrame() && rcMbDataAccess.getMbPicType() == BOT_FIELD && rcMbDataAccess.getMbDataComplementary().isIntra() )
  {
    if ( pcMbDataAccessBase )
    {
      if ( !pcMbDataAccessBase->getMbData().isIntra()      || !pcMbDataAccessBase->getMbDataComplementary().isIntra() )
      {
        b = true;
      }
    }
  }

  bInterEnable |= b;
  //--

  Bool  bDefaultResPredFlag = false;
  Bool avcRewriteFlag = rcMbDataAccess.getSH().getTCoeffLevelPredictionFlag();
  Bool coeffResidualPredFlag = rcMbDataAccess.getCoeffResidualPredFlag();

  if( rcMbDataAccess.getSH().getPPS().getEntropyCodingModeFlag() &&
      !rcMbDataAccess.getSH().getNoInterLayerPredFlag() )
  {
    ROF( pcBaseLayerSbb );
    cBaseLayerBuffer.loadBuffer ( const_cast<Frame*>(pcBaseLayerSbb)->getFullPelYuvBuffer() );
    if (!avcRewriteFlag && !coeffResidualPredFlag)
      bDefaultResPredFlag     = cBaseLayerBuffer.isZero();	
  }

  //===== residual prediction =====
  Bool disableCoeffResidualPred = rcMbDataAccess.getDisableCoeffResidualPredFlag();
  
  if( bBaseLayerAvailable && bInterEnable && (!disableCoeffResidualPred || avcRewriteFlag))
  {
    ROF( pcBaseLayerSbb );
    cBaseLayerBuffer    .loadBuffer ( pcBaseLayerSbb->getFullPelYuvBuffer() );
    {
      if( !avcRewriteFlag)
        m_pcIntOrgMbPelData->subtract   ( cBaseLayerBuffer );

	    if( !pcMbDataAccessBase->getMbData().isIntra() && rcRefFrameList0.getActive() ) // JVT-Q065 EIDR
      {
        //--- only if base layer is in inter mode ---
        // TMM_ESS
        if ( pcMbDataAccessBase->getMbData().getInCropWindowFlag()&& ( !cBaseLayerBuffer.isZero() || coeffResidualPredFlag || avcRewriteFlag || !rcMbDataAccess.getSH().getAdaptiveResidualPredictionFlag() ) )
        {
          RNOK( xEstimateMbBLSkip   ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, cRefFrameList1, pcBaseLayerRec, false, iSpatialScalabilityType,  pcMbDataAccessBase, rcMbDataAccess, true ) );
        }
      }

      if( rcMbDataAccess.getSH().getAdaptiveBaseModeFlag() && rcRefFrameList0.getActive() && !avcRewriteFlag) // JVT-Q065 EIDR
      {
        //--- only if adaptive inter-layer prediction ---
        RNOK( xEstimateMb16x16    ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, cRefFrameList1, false, 0, 0, false,                        pcMbDataAccessBase, true ) );
        RNOK( xEstimateMb16x8     ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, cRefFrameList1, false, 0, 0, false,                        pcMbDataAccessBase, true ) );
        RNOK( xEstimateMb8x16     ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, cRefFrameList1, false, 0, 0, false,                        pcMbDataAccessBase, true ) );
        RNOK( xEstimateMb8x8      ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, cRefFrameList1, false, 0, 0, false,                        pcMbDataAccessBase, true ) );
        RNOK( xEstimateMb8x8Frext ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, cRefFrameList1, false, 0, 0, false,                        pcMbDataAccessBase, true ) );
      }

      if( !avcRewriteFlag )
        m_pcIntOrgMbPelData->add( cBaseLayerBuffer );
    }
  }


  //===== intra base layer mode =====
  if( bBaseLayerAvailable && bIntraEnable )
  {
    //===== only when intra BL is allowed =====
    if( ( avcRewriteFlag && pcMbDataAccessBase->getMbData().isIntra() )
      || (!avcRewriteFlag && (pcMbDataAccessBase->getMbData().isIntra() )) )
    {
		  if ( pcMbDataAccessBase->getMbData().getInCropWindowFlag() ) // TMM_ESS
			  RNOK( xEstimateMbIntraBL( rcMbDataAccess, m_pcIntMbTempData, m_pcIntMbBestData, pcBaseLayerRec, false, pcMbDataAccessBase ) );
    }
  }

  if( bInterEnable )
  //===== without residual prediction =====
  if( rcMbDataAccess.getSH().getNoInterLayerPredFlag() ||
      rcMbDataAccess.getSH().getAdaptiveBaseModeFlag() )
  {
	  if(( ( pcMbDataAccessBase && pcMbDataAccessBase->getMbData().isIntra() ) || rcMbDataAccess.getSH().getAdaptiveBaseModeFlag() ) && rcRefFrameList0.getActive() )  // JVT-Q065 EIDR
	  {
      //--- only if base layer is in intra mode or adaptive prediction is enabled ---
      // TMM_ESS
      if ( pcMbDataAccessBase->getMbData().getInCropWindowFlag() )
       RNOK( xEstimateMbBLSkip   ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, cRefFrameList1, pcBaseLayerRec, false, iSpatialScalabilityType,  pcMbDataAccessBase, rcMbDataAccess, bDefaultResPredFlag ) );// TMM_INTERLACE
    }

    // if 2 reference frames are supplied, do not evaluate the skip mode here
	  if( rcRefFrameList0.getActive() )  // JVT-Q065 EIDR
	  {
        if( pcRefFrameList0Base == 0 && bSkipModeAllowed && ( !bLowComplexMbEnable ) )  // JVT-V079
 		  RNOK  ( xEstimateMbSkip     ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, cRefFrameList1 ) );
		RNOK  ( xEstimateMb16x16    ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, cRefFrameList1, false, 0, 0, false,                        pcMbDataAccessBase, false ) );
		RNOK  ( xEstimateMb16x8     ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, cRefFrameList1, false, 0, 0, false,                        pcMbDataAccessBase, false ) );
		RNOK  ( xEstimateMb8x16     ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, cRefFrameList1, false, 0, 0, false,                        pcMbDataAccessBase, false ) );
		RNOK  ( xEstimateMb8x8      ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, cRefFrameList1, false, 0, 0, false,                        pcMbDataAccessBase, false ) );
		RNOK  ( xEstimateMb8x8Frext ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, cRefFrameList1, false, 0, 0, false,                        pcMbDataAccessBase, false ) );
	  }
  }

  // motion estimation was made with the enhancement reference frame
  // cost evaluation with the actual reference frame
  if( bInterEnable )
  if( pcRefFrameList0Base != 0 && ! m_pcIntMbBestData->getMbDataAccess().getMbData().isIntra() )
  {
    Bool bResidualPredUsed;

    bResidualPredUsed = false;

    //===== residual prediction =====
    if( bBaseLayerAvailable )
    {
      if( m_pcIntMbBestData->getMbDataAccess().getMbData().getResidualPredFlag( PART_16x16 ) )
        bResidualPredUsed = true;
    }

    if( bResidualPredUsed )
    {
      ROF( pcBaseLayerSbb );
      cBaseLayerBuffer    .loadBuffer ( pcBaseLayerSbb->getFullPelYuvBuffer() );
      m_pcIntOrgMbPelData->subtract( cBaseLayerBuffer );
    }
    if(m_pcIntMbBestData->getMbDataAccess().getMbData().isTransformSize8x8())
    {
      RNOK( xSetRdCost8x8InterMb( *m_pcIntMbBestData, pcMbDataAccessBase, *pcRefFrameList0Base, cRefFrameList1 ) );
    }
    else
    {
      RNOK( xSetRdCostInterMb   ( *m_pcIntMbBestData, pcMbDataAccessBase, *pcRefFrameList0Base, cRefFrameList1 ) );//, false, 0, false, 0, bLowComplexMbEnable ) );
    }
    if( bResidualPredUsed )
      m_pcIntOrgMbPelData->add( cBaseLayerBuffer );

    // get skip mode motion vector
    Mv  cMvPredL0, cCurrentMv;

    m_pcIntMbBestData->getMbDataAccess().getMvPredictorSkipMode( cMvPredL0 );
    cCurrentMv = m_pcIntMbBestData->getMbMotionData( LIST_0 ).getMv();

    // check skip mode only when motion vector is equal to skip mode motion vector
    if (m_pcIntMbBestData->getMbMode() == MODE_16x16 && cCurrentMv == cMvPredL0 && bSkipModeAllowed )
    {
      RNOK  ( xEstimateMbSkip     ( m_pcIntMbTempData, m_pcIntMbBestData, *pcRefFrameList0Base, cRefFrameList1 ) );
    }
  }
  m_pcTransform->setClipMode( true );

  //==== normal intra modes =====
  if( !bBaseLayerAvailable || rcMbDataAccess.getSH().getSPS().getNumberOfQualityLevelsCGSSNR() == 1 )
  if( bIntraEnable )
  if( rcMbDataAccess.getSH().getNoInterLayerPredFlag() ||
      rcMbDataAccess.getSH().getAdaptiveBaseModeFlag() )
  {
    RNOK  ( xEstimateMbIntra16  ( m_pcIntMbTempData, m_pcIntMbBestData,                 false ) );
    RNOK  ( xEstimateMbIntra8   ( m_pcIntMbTempData, m_pcIntMbBestData,                 false ) );
    RNOK  ( xEstimateMbIntra4   ( rcMbDataAccess, m_pcIntMbTempData, m_pcIntMbBestData, false ) );
    RNOK  ( xEstimateMbPCM      ( m_pcIntMbTempData, m_pcIntMbBestData,                 false ) );
  }

  // JVT-V079 Low-complexity MB mode decision {
  if ( (bLowComplexMbEnable==true) && (!m_pcIntMbBestData->isIntra()) )
  {
   if ( (m_pcIntMbBestData->getMbMode() == MODE_16x16) & (m_pcIntMbBestData->cbp()==0) &
         (m_pcIntMbBestData->getMbMotionData( LIST_0 ).getRefIdx()==1) )
    RNOK  ( xEstimateMbSkip     ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, cRefFrameList1 ) );
    

   /* Inter mode needs motion compensation */
   RNOK( xSetRdCostInterMb   ( *m_pcIntMbBestData, pcMbDataAccessBase, rcRefFrameList0, cRefFrameList1 ) );
  }
  //JVT-V079 Low-complexity MB mode decision }
  
  RNOK( m_pcRateDistortionIf->fixMacroblockQP( *m_pcIntMbBestData ) );

  xStoreEstimation( rcMbDataAccess, *m_pcIntMbBestData, pcRecSubband, pcPredSignal, false, &cBaseLayerBuffer  );

  //JVT-R057 LA-RDO{
  if(m_bLARDOEnable)
  {
	  Int x,y;
	  MbMode mode=rcMbDataAccess.getMbData().getMbMode();
	  Bool bInter= !rcMbDataAccess.getSH().getNoInterLayerPredFlag();
	  Int KBlock = m_pcIntPicBuffer->getLWidth()/4;

	  Int blockX=rcMbDataAccess.getMbX()*4;
	  Int blockY=rcMbDataAccess.getMbY()*4;
	  Int ep_ref,ec_rec,ec_ep;
	  m_pcIntPicBuffer->getYuvBufferCtrl().initMb();

	  UInt p=1;
	  UInt q=100;
	  if(bInter)
	  {
		  for(UInt i=0;i<=m_uiLayerID;i++)
		  {
			  //Bug_Fix JVT-R057 0806{
			  //p=p*(100-m_auiPLR[m_uiLayerID]);
			  p=p*(100-m_auiPLR[i]);
			  //Bug_Fix JVT-R057 0806}
		  }
		  //q=(UInt)pow(100,(m_uiLayerID+1));
	    q=(UInt)pow(100.0,(int)(m_uiLayerID+1));
    }
	  else
	  {
		  p=100-m_auiPLR[m_uiLayerID];
	  }

	  if(mode==INTRA_BL)
	  {
		  Int ep_base;
		  Int blockIndex;
		  Int xx,yy;
		  for(y=blockY;y<(blockY+4);y++)
			  for(x=blockX;x<(blockX+4);x++)
			  {
				  if(m_pcFrameEcEp)
					  ec_ep=m_pcFrameEcEp->getChannelDistortion()[y*KBlock+x];
				  else
					  ec_ep=0;
				  m_pcIntPicBuffer->getYuvBufferCtrl().initMb();
				  if(m_pcFrameEcEp)
					  ec_rec=GetEC_REC(m_pcIntPicBuffer,m_pcFrameEcEp->getFullPelYuvBuffer(),x,y);
				  else
					  ec_rec=0;
				  xx=(Int)(x/m_aadRatio[m_uiLayerID][0]);
				  yy=(Int)(y/m_aadRatio[m_uiLayerID][1]);
				  blockIndex=yy*(Int)(KBlock/m_aadRatio[m_uiLayerID][0])+xx;
				  ep_base=pcBaseLayerRec->getChannelDistortion()[blockIndex];
				  pcFrame->getChannelDistortion()[y*KBlock+x]=(p*ep_base+(q-p)*(ec_rec+ec_ep))/q;
			  }
	  }

	  else if(mode==MODE_SKIP||mode==MODE_16x16||mode==MODE_16x8||mode==MODE_8x16||mode==MODE_8x8||mode==MODE_8x8ref0)
	  {
		  for( Int n = 0; n <16; n++)
		  {
			  Int iRefIdx[2];
			  iRefIdx [0]=rcMbDataAccess.getMbMotionData(LIST_0).getRefIdx(B4x4Idx(n));
			  iRefIdx [1]=rcMbDataAccess.getMbMotionData(LIST_1).getRefIdx(B4x4Idx(n));
			  Frame* pcRefFrame0 = ( iRefIdx [0] > 0 ? rcRefFrameList0[ iRefIdx [0] ] : NULL );
			  Frame* pcRefFrame1 = ( iRefIdx [1] > 0 ? cRefFrameList1[ iRefIdx [1] ] : NULL );
			  Int iMvX;
			  Int iMvY;
			  Int iDLIST0=0,iDLIST1=0;
			  if(pcRefFrame0)
			  {
				  iMvX=rcMbDataAccess.getMbMotionData(LIST_0).getMv(B4x4Idx(n)).getHor();
				  iMvY=rcMbDataAccess.getMbMotionData(LIST_0).getMv(B4x4Idx(n)).getVer();
				  getChannelDistortion(rcMbDataAccess,*pcRefFrame0,&iDLIST0,iMvX,iMvY,n%4,n/4,1,1);
			  }
			  if(pcRefFrame1)
			  {
				  iMvX=rcMbDataAccess.getMbMotionData(LIST_1).getMv(B4x4Idx(n)).getHor();
				  iMvY=rcMbDataAccess.getMbMotionData(LIST_1).getMv(B4x4Idx(n)).getVer();
				  getChannelDistortion(rcMbDataAccess,*pcRefFrame1,&iDLIST1,iMvX,iMvY,n%4,n/4,1,1);
				  iDLIST0=(iDLIST0+iDLIST1)/2;
			  }
			  ep_ref=iDLIST0;

			  x=blockX+n%4;
			  y=blockY+n/4;
			  if(m_pcFrameEcEp)
				  ec_ep=m_pcFrameEcEp->getChannelDistortion()[y*KBlock+x];
			  else
				  ec_ep=0;
			  m_pcIntPicBuffer->getYuvBufferCtrl().initMb();
			  if(m_pcFrameEcEp)
				  ec_rec=GetEC_REC(m_pcIntPicBuffer,m_pcFrameEcEp->getFullPelYuvBuffer(),x,y);
			  else
				  ec_rec=0;

			  pcFrame->getChannelDistortion()[y*KBlock+x]=(p*ep_ref+(q-p)*(ec_rec+ec_ep))/q;
		  }
	  }
	  else
	  {
		  for(y=blockY;y<(blockY+4);y++)
			  for(x=blockX;x<(blockX+4);x++)
			  {
				  if(m_pcFrameEcEp)
					  ec_ep=m_pcFrameEcEp->getChannelDistortion()[y*KBlock+x];
				  else
					  ec_ep=0;
				  m_pcIntPicBuffer->getYuvBufferCtrl().initMb();
				  if(m_pcFrameEcEp)
					  ec_rec=GetEC_REC(m_pcIntPicBuffer,m_pcFrameEcEp->getFullPelYuvBuffer(),x,y);
				  else
					  ec_rec=0;
				  pcFrame->getChannelDistortion()[y*KBlock+x]=(q-p)*(ec_rec+ec_ep)/q;
			  }
	  }
  }
  //JVT-R057 LA-RDO}

  rdCost = m_pcIntMbBestData->rdCost();

  m_pcIntMbBestData   ->uninit();
  m_pcIntMbTempData   ->uninit();
  m_pcIntMbBest8x8Data->uninit();
  m_pcIntMbTemp8x8Data->uninit();

  return Err::m_nOK;
}





ErrVal
MbEncoder::encodeResidual( MbDataAccess&  rcMbDataAccess,
                           Frame*      pcOrgFrame,
                           Frame*      pcFrame,
                           Frame*      pcResidual,
                           Frame*      pcBaseSubband,
													 Bool&          rbCoded,
                           Double         dLambda,
                           Int            iMaxQpDelta )
{
  ROF( bInitDone );

  m_pcIntPicBuffer    = pcFrame->getFullPelYuvBuffer();

  UInt    uiMinTrafo  = 0;
  UInt    uiMaxTrafo  = ( ( rcMbDataAccess.getSH().getPPS().getTransform8x8ModeFlag() && rcMbDataAccess.getMbData().is8x8TrafoFlagPresent(rcMbDataAccess.getSH().getSPS().getDirect8x8InferenceFlag()) ) ? 2 : 1 );
  UChar   ucMinQp     = (UChar)min( MAX_QP, max( MIN_QP, rcMbDataAccess.getMbData().getQp() - iMaxQpDelta ) );
  UChar   ucMaxQp     = (UChar)min( MAX_QP, max( MIN_QP, rcMbDataAccess.getMbData().getQp() + iMaxQpDelta ) );
  Double  dMinCost    = 1e30;
  UInt    uiDist=0, uiRate;
  Bool    bCoded      = false;
  Double  dCost       = 1e30;

  UInt    uiRPred     = ( rcMbDataAccess.getSH().getNoInterLayerPredFlag() ? 0 :
                          rcMbDataAccess.getSH().getAdaptiveBaseModeFlag() ? 2 : 1 );
  Int     iMinCnt     = ( uiRPred == 0 || uiRPred == 2 ? 0 : 1 );
  Int     iMaxCnt     = ( uiRPred == 1 || uiRPred == 2 ? 2 : 1 );
  Bool    bPotentialBSkip = true;
  Bool avcRewriteFlag = rcMbDataAccess.getSH().getTCoeffLevelPredictionFlag();
  Bool coeffResidualPredFlag = rcMbDataAccess.getCoeffResidualPredFlag();

  //>>> fix (skip mode in hierarchical P pictures) - H. Schwarz
  Bool    bRemovePSkip= false;
  //<<< fix (skip mode in hierarchical P pictures) - H. Schwarz

  Bool disableCoeffResidualPred =  rcMbDataAccess.getDisableCoeffResidualPredFlag();
  if (iMaxCnt > 1 && (!avcRewriteFlag && disableCoeffResidualPred))
  {
    iMaxCnt = 1;  // disable residual prediction if it can not be done in transform domain
  }

  if( (avcRewriteFlag || coeffResidualPredFlag) && (iMaxCnt > 1) )
  {
    if( (rcMbDataAccess.getMbDataAccessBase()->getMbData().isTransformSize8x8() == true)
      && (uiMaxTrafo < 2) )
    { // if AVC rewrite is enabled, do not allow residual prediction if base layer and enhancement layer transofrm size do not match
      iMaxCnt = 1;      
    }
  }

  Bool  bDefaultResPredFlag = false;
  if( rcMbDataAccess.getSH().getPPS().getEntropyCodingModeFlag() &&
     !rcMbDataAccess.getSH().getNoInterLayerPredFlag() )
  {
    ROF( pcBaseSubband );
    YuvMbBuffer cBaseLayerBuffer;
    cBaseLayerBuffer.loadBuffer( const_cast<Frame*>(pcBaseSubband)->getFullPelYuvBuffer() );

    if (!avcRewriteFlag && !coeffResidualPredFlag)
      bDefaultResPredFlag = cBaseLayerBuffer.isZero();
  }

  m_pcXDistortion->loadOrgMbPelData( m_pcIntPicBuffer, m_pcIntOrgMbPelData );
  m_pcTransform     ->setStoreCoeffFlag(true);

  Int iCnt;
  for( iCnt = iMinCnt; iCnt < iMaxCnt; iCnt++ )
  {
		//-- JVT-R091
		YuvMbBuffer cPrdMbBuffer, cBaseResMbBuffer, cMbBuffer, cNewPrdMbBuffer, cOrgMbBuffer;

		// store original residual
		cOrgMbBuffer		.loadLuma	 ( *m_pcIntOrgMbPelData										);
		cOrgMbBuffer		.loadChroma( *m_pcIntOrgMbPelData										);

    for( UChar ucQp = ucMinQp; ucQp <= ucMaxQp; ucQp++ )
    {
      for( UInt uiTrafo8x8 = uiMinTrafo; uiTrafo8x8 < uiMaxTrafo; uiTrafo8x8++ )
      {
        m_pcIntMbTempData ->init( rcMbDataAccess );

				if( iCnt )
        {
          if (avcRewriteFlag || coeffResidualPredFlag)
          {
  		    if( avcRewriteFlag )
              m_pcIntMbTempData->setAllSamplesToZero();
            else
              m_pcIntMbTempData->loadBuffer( pcBaseSubband->getFullPelYuvBuffer() );

            m_pcIntMbTempData->setResidualPredFlag( true );

            if( m_pcIntMbTempData->getResidualPredFlag( PART_16x16 ) )
            {
              Bool bBaseTransSize8x8 = m_pcIntMbTempData->getMbDataAccess().getMbDataAccessBase()->getMbData().isTransformSize8x8();
              UInt uiBaseCbp = m_pcIntMbTempData->getMbDataAccess().getMbDataAccessBase()->getMbData().getMbCbp();

              // Enforce restrictions on transforms size between layers
              if( uiTrafo8x8==0 && bBaseTransSize8x8 )
                uiTrafo8x8++;

              if( uiTrafo8x8==1 && !bBaseTransSize8x8 && uiBaseCbp)
                break;

            }
          }
          else
            m_pcIntMbTempData->loadBuffer( pcBaseSubband->getFullPelYuvBuffer() );
        }
        else
        {
          m_pcIntMbTempData->setAllSamplesToZero();
        }

        m_pcIntMbTempData ->getTempYuvMbBuffer().loadLuma( *m_pcIntMbTempData );
        m_pcIntMbTempData ->setQp( ucQp );

  			m_pcTransform   ->setQp( *m_pcIntMbTempData, rcMbDataAccess.getSH().isIntraSlice() );

        //----- encode luminance signal -----
        UInt  uiExtCbp    = 0;
        UInt  uiCoeffCost = 0;
        UInt  uiMbBits    = 0;
        UInt  uiB8Thres   = 4;
        UInt  uiMBThres   = 5;

        if( uiTrafo8x8 )
        {
          for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
          {
            xSetCoeffCost( 0 );
            UInt uiBits = 0;
            UInt uiCbp  = 0;

            RNOK( xEncode8x8InterBlock( *m_pcIntMbTempData, c8x8Idx, uiBits, uiCbp ) );
            if( uiCbp )
            {
							//-- JVT-R091
							// note: do not use coefficient skip as IntraBL does
							if( xGetCoeffCost() <= uiB8Thres )
							//--
              {
                m_pcIntMbTempData->loadLuma( m_pcIntMbTempData->getTempYuvMbBuffer(), c8x8Idx );

                m_pcIntMbTempData->clearLumaLevels8x8Block( c8x8Idx );

                if( coeffResidualPredFlag && m_pcIntMbTempData->getResidualPredFlag( PART_16x16 ) )
                {                  
                  reCalcBlock8x8(*m_pcIntMbTempData, c8x8Idx, 0);
                }
                else if( avcRewriteFlag && m_pcIntMbTempData->getResidualPredFlag( PART_16x16 ) )
                  {
                  reCalcBlock8x8Rewrite(*m_pcIntMbTempData, c8x8Idx, 0);                  
                }
              }
              else
              {
                uiCoeffCost += xGetCoeffCost();
                uiExtCbp    += uiCbp;
                uiMbBits    += uiBits;
              }
            }
          }
        }
        else
        {
          for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
          {
            xSetCoeffCost( 0 );
            UInt uiBits = 0;
            UInt uiCbp  = 0;
            for( S4x4Idx cIdx( c8x8Idx ); cIdx.isLegal( c8x8Idx ); cIdx++ )
            {
              RNOK( xEncode4x4InterBlock( *m_pcIntMbTempData, cIdx, uiBits, uiCbp ) );
            }
            if( uiCbp )
            {
							//-- JVT-R091
							// note: do not use coefficient skip as IntraBL does
              if( xGetCoeffCost() <= uiB8Thres )
							//--
              {
                m_pcIntMbTempData->loadLuma( m_pcIntMbTempData->getTempYuvMbBuffer(), c8x8Idx );

                m_pcIntMbTempData->clearLumaLevels8x8( c8x8Idx );

                if( coeffResidualPredFlag && m_pcIntMbTempData->getResidualPredFlag( PART_16x16 ) )
                {
                  reCalcBlock8x8(*m_pcIntMbTempData, c8x8Idx, 1);
                }
                else if( avcRewriteFlag && m_pcIntMbTempData->getResidualPredFlag( PART_16x16 ) )
                {
                  reCalcBlock8x8Rewrite(*m_pcIntMbTempData, c8x8Idx, 1);                  
                }
              }
              else
              {
                uiCoeffCost += xGetCoeffCost();
                uiExtCbp    += uiCbp;
                uiMbBits    += uiBits;
              }
            }
          }
        }
				//-- JVT-R091
				// note: do not use coefficient skip as IntraBL does
        if( 0 != uiExtCbp && uiCoeffCost <= uiMBThres )
				//--
        {
          m_pcIntMbTempData->loadLuma( m_pcIntMbTempData->getTempYuvMbBuffer() );
          uiExtCbp = 0;

          m_pcIntMbTempData->clearLumaLevels();

          if( coeffResidualPredFlag &&  m_pcIntMbTempData->getResidualPredFlag( PART_16x16 ) )
          { 
            Bool bBaseTransSize8x8 = m_pcIntMbTempData->getMbDataAccess().getMbDataAccessBase()->getMbData().isTransformSize8x8();
            for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
            {
              reCalcBlock8x8(*m_pcIntMbTempData, c8x8Idx, !bBaseTransSize8x8);
            }
          }
          else if(avcRewriteFlag &&  m_pcIntMbTempData->getResidualPredFlag( PART_16x16 ) )
          {
            Bool bBaseTransSize8x8 = m_pcIntMbTempData->getMbDataAccess().getMbDataAccessBase()->getMbData().isTransformSize8x8();
            for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
            {
              reCalcBlock8x8Rewrite(*m_pcIntMbTempData, c8x8Idx, !bBaseTransSize8x8);
            }            
          }
        }
        MbDataAccess*  pcMbDataAccessBase = rcMbDataAccess.getMbDataAccessBase();
        if(!pcMbDataAccessBase||pcMbDataAccessBase->getMbData().m_bRPSafe||m_pcIntMbTempData->getResidualPredFlag(PART_16x16)==false)
          m_pcIntMbTempData->distY() = m_pcXDistortion->getLum16x16( m_pcIntMbTempData->getMbLumAddr(), m_pcIntMbTempData->getLStride() );
        else
          m_pcIntMbTempData->distY() = m_pcXDistortion->getLum16x16RP( m_pcIntMbTempData->getMbLumAddr(), m_pcIntMbTempData->getLStride() );

        //----- encode chrominance signal -----
        RNOK( xEncodeChromaTexture( *m_pcIntMbTempData, uiExtCbp, uiMbBits ) );

        //----- set parameters ----
        bCoded                         = ( uiExtCbp > 0 );
        m_pcIntMbTempData->bits()      = uiMbBits;
        m_pcIntMbTempData->cbp()       = xCalcMbCbp( uiExtCbp );
        m_pcIntMbTempData->coeffCost() = uiCoeffCost;

        Bool  b8x8Trafo = ( uiTrafo8x8 && ( m_pcIntMbTempData->getMbCbp() & 0x0F ) );
        m_pcIntMbTempData ->setTransformSize8x8( b8x8Trafo );

        //----- fix QP ------
        RNOK( m_pcRateDistortionIf->fixMacroblockQP( *m_pcIntMbTempData ) );


        //--- set parameters ---
        uiDist  = m_pcIntMbTempData->distY     ();
        uiDist += m_pcXDistortion  ->get8x8Cb  ( m_pcIntMbTempData->getMbCbAddr (), m_pcIntMbTempData->getCStride() );
        uiDist += m_pcXDistortion  ->get8x8Cr  ( m_pcIntMbTempData->getMbCrAddr (), m_pcIntMbTempData->getCStride() );
        RNOK(   BitCounter::init() );
        RNOK(   MbCoder::m_pcMbSymbolWriteIf->cbp( *m_pcIntMbTempData ) );
        if( m_pcIntMbTempData->cbp() )
        {
          RNOK( MbCoder::m_pcMbSymbolWriteIf->deltaQp( *m_pcIntMbTempData ) );
        }
        uiRate  = uiMbBits + BitCounter::getNumberOfWrittenBits();

        //>>> fix (skip mode in hierarchical P pictures) - H. Schwarz
        Bool bCoefficientsInPSkip = false;
        if( rcMbDataAccess.getSH().getSliceType()  == P_SLICE   &&
            rcMbDataAccess.getMbData().getMbMode() == MODE_SKIP &&
            ( uiExtCbp != 0 || iCnt != 0 ) )
        {
          // temporarily convert SKIP mode into 16x16 mode
          Mv cMvPred, cMvd;
          rcMbDataAccess.getMvPredictor( cMvPred, 1, LIST_0 );
          cMvd  = rcMbDataAccess.getMbMotionData( LIST_0 ).getMv();
          cMvd -= cMvPred;
          rcMbDataAccess.getMbMvdData( LIST_0 ).setAllMv( cMvd );
          rcMbDataAccess.getMbData().setMbMode( MODE_16x16 );
          RNOK( BitCounter::init() );
          RNOK( MbCoder::m_pcMbSymbolWriteIf->mbMode( rcMbDataAccess ) );
          RNOK( MbCoder::xWriteReferenceFrames      ( rcMbDataAccess, MODE_16x16, LIST_0 ) );
          RNOK( MbCoder::xWriteMotionPredFlags      ( rcMbDataAccess, MODE_16x16, LIST_0 ) );
          RNOK( MbCoder::xWriteMotionVectors        ( rcMbDataAccess, MODE_16x16, LIST_0 ) );
          uiRate += BitCounter::getNumberOfWrittenBits();
          // reset original SKIP mode
          rcMbDataAccess.getMbMvdData( LIST_0 ).setAllMv( Mv::m_cMvZero );
          rcMbDataAccess.getMbData().setMbMode( MODE_SKIP );
          // set flag
          bCoefficientsInPSkip = true;
        }
        //<<< fix (skip mode in hierarchical P pictures) - H. Schwarz

        bPotentialBSkip = false;
        if( rcMbDataAccess.getSH().getSliceType() == B_SLICE &&
            rcMbDataAccess.getMbData().getMbMode() == MODE_SKIP &&
            iCnt == 0 /* no residual prediction */ && uiExtCbp == 0 )
        {
          uiRate = ( rcMbDataAccess.getSH().getNoInterLayerPredFlag() ? 1 : 0 );
          bPotentialBSkip = true;
        }

        dCost   = (Double)uiDist + dLambda * (Double)uiRate;

        // QP changed by fixMacroblockQP(), this will cause mismatch when avcRewrite enabled, so do not choose this mode for now
        // TODO: need to redo the prediction using the new QP
        if (avcRewriteFlag && m_pcIntMbTempData->getQp() != ucQp)
          dCost= 1e30;

        if( dCost < dMinCost )
        {
          dMinCost  = dCost;
          rbCoded   = bCoded;

          //----- store parameters and reconstructed signal to Frame and MbDataAccess -----
          m_pcIntPicBuffer                  ->loadBuffer        ( m_pcIntMbTempData );
          pcResidual->getFullPelYuvBuffer() ->loadBuffer        ( m_pcIntMbTempData );
          m_pcIntMbTempData                 ->copyResidualDataTo( rcMbDataAccess );

          //----- set residual prediction flag -----
          rcMbDataAccess.getMbData().setResidualPredFlag( iCnt > 0 ? true : ( bPotentialBSkip ? false : bDefaultResPredFlag ) );

          //>>> fix (skip mode in hierarchical P pictures) - H. Schwarz
          bRemovePSkip = bCoefficientsInPSkip;
          //<<< fix (skip mode in hierarchical P pictures) - H. Schwarz
        }

        m_pcIntMbTempData->uninit();
      }
    }

    if (avcRewriteFlag && (dMinCost == 1e30))  // in case, no valid mode is chosen due to we disallow the mode whose QP is changed by fixMacroblockQP()
    {
      // in rare case, no mode is selected, assign the last mode it
      fprintf(stderr, "no mode is selected at poc %d, MB(%d, %d)", rcMbDataAccess.getSH().getPoc(), rcMbDataAccess.getMbY(), rcMbDataAccess.getMbX());
      dMinCost  = dCost;
      rbCoded   = bCoded;

      //----- store parameters and reconstructed signal to Frame and MbDataAccess -----
      m_pcIntPicBuffer                  ->loadBuffer        ( m_pcIntMbTempData );
      pcResidual->getFullPelYuvBuffer() ->loadBuffer        ( m_pcIntMbTempData );
      m_pcIntMbTempData                 ->copyResidualDataTo( rcMbDataAccess );

      //----- set residual prediction flag -----
      rcMbDataAccess.getMbData().setResidualPredFlag( iCnt > 0 ? true : ( bPotentialBSkip ? false : bDefaultResPredFlag ) );
    }

    //-- JVT-R091
    // restore original residual
    m_pcIntOrgMbPelData->loadLuma		( cOrgMbBuffer );
    m_pcIntOrgMbPelData->loadChroma	( cOrgMbBuffer );
    //--
  }

  //>>> fix (skip mode in hierarchical P pictures) - H. Schwarz
  if( bRemovePSkip )
  {
    // convert SKIP mode into 16x16 mode
    Mv cMvPred, cMvd;
    rcMbDataAccess.getMvPredictor( cMvPred, 1, LIST_0 );
    cMvd  = rcMbDataAccess.getMbMotionData( LIST_0 ).getMv();
    cMvd -= cMvPred;
    rcMbDataAccess.getMbMvdData( LIST_0 ).setAllMv( cMvd );
    rcMbDataAccess.getMbData().setMbMode( MODE_16x16 );
  }
  //<<< fix (skip mode in hierarchical P pictures) - H. Schwarz

  return Err::m_nOK;
}




ErrVal
MbEncoder::estimatePrediction( MbDataAccess&   rcMbDataAccess,
                               MbDataAccess*   pcMbDataAccessBase,
                               Int             iSpatialScalabilityType,
                               RefFrameList&   rcRefFrameList0,
                               RefFrameList&   rcRefFrameList1,
                               const Frame* pcBaseLayerFrame,
                               const Frame* pcBaseLayerResidual,
                               const Frame& rcOrigFrame,
                               Frame&       rcIntraRecFrame,
                               Bool            bBiPredOnly,
                               UInt            uiNumMaxIter,
                               UInt            uiIterSearchRange,
							                 Bool				     bBLSkipEnable, // JVT-Q065 EIDR
                               Double          dLambda,
                               Double&         rdCost,
                               Bool            bSkipModeAllowed )
{
  ROF( bInitDone );

  rcMbDataAccess.setMbDataAccessBase(pcMbDataAccessBase);

  Bool  bBSlice = rcMbDataAccess.getSH().getSliceType () == B_SLICE;
  UInt  uiQp    = rcMbDataAccess.getSH().getSliceQp     ();
  RNOK( m_pcRateDistortionIf->setMbQpLambda( rcMbDataAccess, uiQp, dLambda ) )

  m_pcIntMbBestIntraChroma  = NULL;
  m_pcIntMbBestData   ->init( rcMbDataAccess );
  m_pcIntMbTempData   ->init( rcMbDataAccess );
  m_pcIntMbBest8x8Data->init( rcMbDataAccess );
  m_pcIntMbTemp8x8Data->init( rcMbDataAccess );

  
  m_pcIntPicBuffer                  = rcIntraRecFrame.getFullPelYuvBuffer();
  YuvPicBuffer*  pcOrgPicBuffer  = const_cast<Frame&>( rcOrigFrame ).getFullPelYuvBuffer();
  m_pcXDistortion->loadOrgMbPelData ( pcOrgPicBuffer, m_pcIntOrgMbPelData );
  m_pcTransform  ->setQp            ( rcMbDataAccess, false );

  YuvMbBuffer  cBaseLayerBuffer;

  Bool bBaseLayerAvailable = (NULL != pcMbDataAccessBase) && !rcMbDataAccess.getSH().getNoInterLayerPredFlag();
  Bool bIntraEnable = ! rcMbDataAccess.isFieldMbInMbaffFrame() || rcMbDataAccess.isTopMb() || rcMbDataAccess.getMbDataComplementary().isIntra();
  Bool bInterEnable = ! rcMbDataAccess.isFieldMbInMbaffFrame() || rcMbDataAccess.isTopMb() || ! rcMbDataAccess.getMbDataComplementary().isIntra();

 //--  JVT-T037
  Bool  b = false;
  if ( rcMbDataAccess.isFieldMbInMbaffFrame() && rcMbDataAccess.getMbPicType() == BOT_FIELD && rcMbDataAccess.getMbDataComplementary().isIntra() )
  {
    if ( pcMbDataAccessBase )
    {
      if (!pcMbDataAccessBase->getMbData().isIntra()      || !pcMbDataAccessBase->getMbDataComplementary().isIntra() )
      {
        b = true;
      }
    }
  }

  bInterEnable |= b;
  //--
  Bool avcRewriteFlag = rcMbDataAccess.getSH().getTCoeffLevelPredictionFlag();
  Bool coeffResidualPredFlag = rcMbDataAccess.getCoeffResidualPredFlag();

  Bool  bDefaultResPredFlag = false;
  if( rcMbDataAccess.getSH().getPPS().getEntropyCodingModeFlag() &&
     !rcMbDataAccess.getSH().getNoInterLayerPredFlag() )
  {
    ROF( pcBaseLayerResidual );
    cBaseLayerBuffer.loadBuffer ( const_cast<Frame*>(pcBaseLayerResidual)->getFullPelYuvBuffer() );
    if (!avcRewriteFlag && !coeffResidualPredFlag)
      bDefaultResPredFlag  = cBaseLayerBuffer.isZero();
  }

  //>>> fix (skip mode in hierarchical P pictures) - H. Schwarz
  //TMM_INTERLACE {
  //if( ! rcMbDataAccess.getSH().isMbaffFrame() ) // don't know why, but it doesn't work for MbAff (when there is a layer on top of it)
                                           // looks like the problem lies somewhere else ... may be in intra padding process before upsampling
   //TMM_INTERLACE }
  if( bSkipModeAllowed && ! bBSlice )
  {
    RNOK( xEstimateMbSkip( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, rcRefFrameList1 ) );
  }
  //<<< fix (skip mode in hierarchical P pictures) - H. Schwarz


  //===== residual prediction =====
  if( bInterEnable )
  if( bBaseLayerAvailable )
  {
    //--- subtract (upsampled) base layer residual from original macroblock data ---
    ROF( pcBaseLayerResidual );
    cBaseLayerBuffer    .loadBuffer ( const_cast<Frame*>(pcBaseLayerResidual)->getFullPelYuvBuffer() );

    if( avcRewriteFlag || !cBaseLayerBuffer.isZero() ) // HS: search only with residual prediction, when residual signal is non-zero
    {
      if (!avcRewriteFlag)
        m_pcIntOrgMbPelData->subtract   ( cBaseLayerBuffer );

      if( ! pcMbDataAccessBase->getMbData().isIntra() && bBLSkipEnable) // JVT-Q065 EIDR
      {
        //--- only if base layer is in intra mode ---
        // TMM_ESS
		    if ( pcMbDataAccessBase->getMbData().getInCropWindowFlag() )
        {
          RNOK( xEstimateMbBLSkip( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, rcRefFrameList1, pcBaseLayerFrame, bBSlice, iSpatialScalabilityType, pcMbDataAccessBase, rcMbDataAccess, true ) );
        }
      }

      if( rcMbDataAccess.getSH().getAdaptiveBaseModeFlag() )
      {
        //S051{
        if(m_bUseBDir)
          //S051}
         RNOK( xEstimateMbDirect   ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, rcRefFrameList1,
                                     pcMbDataAccessBase, true, rcMbDataAccess.getMbData().getQp(), bSkipModeAllowed ) );
        RNOK( xEstimateMb16x16    ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, rcRefFrameList1, bBiPredOnly, uiNumMaxIter, uiIterSearchRange, false,  pcMbDataAccessBase, true ) );
        RNOK( xEstimateMb16x8     ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, rcRefFrameList1, bBiPredOnly, uiNumMaxIter, uiIterSearchRange, false,  pcMbDataAccessBase, true ) );
        RNOK( xEstimateMb8x16     ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, rcRefFrameList1, bBiPredOnly, uiNumMaxIter, uiIterSearchRange, false,  pcMbDataAccessBase, true ) );
        RNOK( xEstimateMb8x8      ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, rcRefFrameList1, bBiPredOnly, uiNumMaxIter, uiIterSearchRange, false,  pcMbDataAccessBase, true ) );
        RNOK( xEstimateMb8x8Frext ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, rcRefFrameList1, bBiPredOnly, uiNumMaxIter, uiIterSearchRange, false,  pcMbDataAccessBase, true ) );
      }

      //--- recover original macroblock data ---
      if (!avcRewriteFlag)
        m_pcIntOrgMbPelData->add        ( cBaseLayerBuffer );
    }
  }


  //===== intra base layer mode =====
  if( bIntraEnable )
  if( bBaseLayerAvailable )
  {
    if( pcMbDataAccessBase->getMbData().isIntra() )
      if ( pcMbDataAccessBase->getMbData().getInCropWindowFlag() ) // TMM_ESS
        RNOK  ( xEstimateMbIntraBL  ( rcMbDataAccess, m_pcIntMbTempData, m_pcIntMbBestData, pcBaseLayerFrame, bBSlice,                                                              pcMbDataAccessBase ) );
  }


  //===== without residual prediction =====
  if( bInterEnable )
  if( ! bBaseLayerAvailable ||
      rcMbDataAccess.getSH().getAdaptiveBaseModeFlag() )
  {
    if( ( pcMbDataAccessBase && pcMbDataAccessBase->getMbData().isIntra() ) || rcMbDataAccess.getSH().getAdaptiveBaseModeFlag() )
    {
      // TMM_ESS
		if( pcMbDataAccessBase )
		if ( pcMbDataAccessBase->getMbData().getInCropWindowFlag() && bBLSkipEnable) // JVT-Q065 EIDR
      	RNOK( xEstimateMbBLSkip   ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, rcRefFrameList1, pcBaseLayerFrame, bBSlice, iSpatialScalabilityType,         pcMbDataAccessBase, rcMbDataAccess, false ) );
    }


	//S051{
	if(m_bUseBDir)
	//S051}
     RNOK  ( xEstimateMbDirect   ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, rcRefFrameList1,
                                   pcMbDataAccessBase, bDefaultResPredFlag,
                                   rcMbDataAccess.getMbData().getQp(), bSkipModeAllowed ) );
    RNOK  ( xEstimateMbDirect   ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, rcRefFrameList1,
                                  pcMbDataAccessBase, false,               
                                  rcMbDataAccess.getMbData().getQp(), bSkipModeAllowed ) ); // skip mode
    RNOK  ( xEstimateMb16x16    ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, rcRefFrameList1, bBiPredOnly, uiNumMaxIter, uiIterSearchRange, false,  pcMbDataAccessBase, bDefaultResPredFlag ) );
    RNOK  ( xEstimateMb16x8     ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, rcRefFrameList1, bBiPredOnly, uiNumMaxIter, uiIterSearchRange, false,  pcMbDataAccessBase, bDefaultResPredFlag ) );
    RNOK  ( xEstimateMb8x16     ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, rcRefFrameList1, bBiPredOnly, uiNumMaxIter, uiIterSearchRange, false,  pcMbDataAccessBase, bDefaultResPredFlag ) );
    RNOK  ( xEstimateMb8x8      ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, rcRefFrameList1, bBiPredOnly, uiNumMaxIter, uiIterSearchRange, false,  pcMbDataAccessBase, bDefaultResPredFlag ) );
    RNOK  ( xEstimateMb8x8Frext ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, rcRefFrameList1, bBiPredOnly, uiNumMaxIter, uiIterSearchRange, false,  pcMbDataAccessBase, bDefaultResPredFlag ) );

  }


  //===== normal intra mode =====
  if( !bBaseLayerAvailable || rcMbDataAccess.getSH().getSPS().getNumberOfQualityLevelsCGSSNR() == 1 )
  if( bIntraEnable )
  if( (( ! bBaseLayerAvailable ) ||
      rcMbDataAccess.getSH().getAdaptiveBaseModeFlag() ) )
  {
    RNOK  ( xEstimateMbIntra16  ( m_pcIntMbTempData, m_pcIntMbBestData,                   bBSlice ) );
    RNOK  ( xEstimateMbIntra8   ( m_pcIntMbTempData, m_pcIntMbBestData,                   bBSlice ) );
    RNOK  ( xEstimateMbIntra4   ( rcMbDataAccess, m_pcIntMbTempData, m_pcIntMbBestData,                   bBSlice ) );
    RNOK  ( xEstimateMbPCM      ( m_pcIntMbTempData, m_pcIntMbBestData,                   bBSlice ) );
  }
  xStoreEstimation( rcMbDataAccess, *m_pcIntMbBestData, NULL, NULL, true, &cBaseLayerBuffer );



  //JVT-R057 LA-RDO{
  if(m_bLARDOEnable)
  {
	  Int x,y;
	  MbMode mode=rcMbDataAccess.getMbData().getMbMode();
	  Bool bInter=!rcMbDataAccess.getSH().getNoInterLayerPredFlag();
	  Int KBlock = m_pcIntPicBuffer->getLWidth()/4;

	  Int blockX=rcMbDataAccess.getMbX()*4;
	  Int blockY=rcMbDataAccess.getMbY()*4;
	  Int ep_ref,ec_rec,ec_ep;
	  m_pcIntPicBuffer->getYuvBufferCtrl().initMb();

	  UInt p=1;
	  UInt q=100;
	  if(bInter)
	  {
		  for(UInt i=0;i<=m_uiLayerID;i++)
		  {
			  //Bug_Fix JVT-R057 0806{
			  //p=p*(100-m_auiPLR[m_uiLayerID]);
			  p=p*(100-m_auiPLR[i]);
			  //Bug_Fix JVT-R057 0806}
		  }
		  //q=(UInt)pow(100,(m_uiLayerID+1));
      q=(UInt)pow(100.0,(int)(m_uiLayerID+1));
	  }
	  else
	  {
		  p=100-m_auiPLR[m_uiLayerID];
	  }

	  if(mode==INTRA_BL)
	  {
		  Int ep_base;
		  Int blockIndex;
		  Int xx,yy;
		  for(y=blockY;y<(blockY+4);y++)
			  for(x=blockX;x<(blockX+4);x++)
			  {
				  if(m_pcFrameEcEp)
					  ec_ep=m_pcFrameEcEp->getChannelDistortion()[y*KBlock+x];
				  else
					  ec_ep=0;
				  m_pcIntPicBuffer->getYuvBufferCtrl().initMb();
				  if(m_pcFrameEcEp)
					  ec_rec=GetEC_REC(m_pcIntPicBuffer,m_pcFrameEcEp->getFullPelYuvBuffer(),x,y);
				  else
					  ec_rec=0;

				  xx=(Int)(x/m_aadRatio[m_uiLayerID][0]);
				  yy=(Int)(y/m_aadRatio[m_uiLayerID][1]);
				  blockIndex=yy*(Int)(KBlock/m_aadRatio[m_uiLayerID][0])+xx;

				  ep_base=const_cast<Frame*>(pcBaseLayerFrame)->getChannelDistortion()[blockIndex];
				  const_cast<Frame&>(rcOrigFrame).getChannelDistortion()[y*KBlock+x]=(p*ep_base+(q-p)*(ec_rec+ec_ep))/q;
			  }
	  }
	  else if(mode==MODE_SKIP||mode==MODE_16x16||mode==MODE_16x8||mode==MODE_8x16||mode==MODE_8x8||mode==MODE_8x8ref0)
	  {
		  for( Int n = 0; n <16; n++)
		  {
			  Int iRefIdx[2];
			  iRefIdx [0]=rcMbDataAccess.getMbMotionData(LIST_0).getRefIdx(B4x4Idx(n));
			  iRefIdx [1]=rcMbDataAccess.getMbMotionData(LIST_1).getRefIdx(B4x4Idx(n));
			  Frame* pcRefFrame0 = ( iRefIdx [0] > 0 ? rcRefFrameList0[ iRefIdx [0] ] : NULL );
			  Frame* pcRefFrame1 = ( iRefIdx [1] > 0 ? rcRefFrameList1[ iRefIdx [1] ] : NULL );
			  Int iMvX;
			  Int iMvY;
			  Int iDLIST0=0,iDLIST1=0;
			  if(pcRefFrame0)
			  {
				  iMvX=rcMbDataAccess.getMbMotionData(LIST_0).getMv(B4x4Idx(n)).getHor();
				  iMvY=rcMbDataAccess.getMbMotionData(LIST_0).getMv(B4x4Idx(n)).getVer();
				  getChannelDistortion(rcMbDataAccess,*pcRefFrame0,&iDLIST0,iMvX,iMvY,n%4,n/4,1,1);
			  }
			  if(pcRefFrame1)
			  {
				  iMvX=rcMbDataAccess.getMbMotionData(LIST_1).getMv(B4x4Idx(n)).getHor();
				  iMvY=rcMbDataAccess.getMbMotionData(LIST_1).getMv(B4x4Idx(n)).getVer();
				  getChannelDistortion(rcMbDataAccess,*pcRefFrame1,&iDLIST1,iMvX,iMvY,n%4,n/4,1,1);
				  iDLIST0=(iDLIST0+iDLIST1)/2;
			  }
			  ep_ref=iDLIST0;

			  x=blockX+n%4;
			  y=blockY+n/4;
			  if(m_pcFrameEcEp)
				  ec_ep=m_pcFrameEcEp->getChannelDistortion()[y*KBlock+x];
			  else
				  ec_ep=0;
			  m_pcIntPicBuffer->getYuvBufferCtrl().initMb();
			  if(m_pcFrameEcEp)
				  ec_rec=GetEC_REC(m_pcIntPicBuffer,m_pcFrameEcEp->getFullPelYuvBuffer(),x,y);
			  else
				  ec_rec=0;

			  const_cast<Frame&>(rcOrigFrame).getChannelDistortion()[y*KBlock+x]=(p*ep_ref+(q-p)*(ec_rec+ec_ep))/q;
		  }
	  }
	  else
	  {
		  for(y=blockY;y<(blockY+4);y++)
			  for(x=blockX;x<(blockX+4);x++)
			  {
				  if(m_pcFrameEcEp)
					  ec_ep=m_pcFrameEcEp->getChannelDistortion()[y*KBlock+x];
				  else
					  ec_ep=0;
				  m_pcIntPicBuffer->getYuvBufferCtrl().initMb();
				  if(m_pcFrameEcEp)
					  ec_rec=GetEC_REC(m_pcIntPicBuffer,m_pcFrameEcEp->getFullPelYuvBuffer(),x,y);
				  else
					  ec_rec=0;
				  const_cast<Frame&>(rcOrigFrame).getChannelDistortion()[y*KBlock+x]=(q-p)*(ec_rec+ec_ep)/q;
			  }
	  }
  }
  //JVT-R057 LA-RDO}

  rdCost = m_pcIntMbBestData->rdCost();

  m_pcIntMbBestData   ->uninit();
  m_pcIntMbTempData   ->uninit();
  m_pcIntMbBest8x8Data->uninit();
  m_pcIntMbTemp8x8Data->uninit();

  return Err::m_nOK;
}



ErrVal
MbEncoder::compensatePrediction( MbDataAccess&   rcMbDataAccess,
                                 Frame*       pcMCFrame,
                                 RefFrameList&   rcRefFrameList0,
                                 RefFrameList&   rcRefFrameList1,
                                 Bool            bCalcMv,
                                 Bool            bFaultTolerant)
{
  YuvMbBuffer  cYuvMbBuffer;
  if( rcMbDataAccess.getMbData().isIntra() )
  {
    cYuvMbBuffer.setAllSamplesToZero();
  }
  else
  {
    if( rcMbDataAccess.getMbData().getMbMode() == MODE_8x8 || rcMbDataAccess.getMbData().getMbMode() == MODE_8x8ref0 )
    {
      for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
      {
        RNOK( m_pcMotionEstimation->compensateSubMb( c8x8Idx, rcMbDataAccess, rcRefFrameList0, rcRefFrameList1, &cYuvMbBuffer, bCalcMv, bFaultTolerant ) );
      }
    }
    else
    {
      //----- motion compensated prediction -----
      RNOK( m_pcMotionEstimation->compensateMb( rcMbDataAccess, rcRefFrameList0, rcRefFrameList1, &cYuvMbBuffer, bCalcMv ) );
    }
  }

  RNOK(m_pcMotionEstimation->compensateMbBLSkipIntra(rcMbDataAccess, &cYuvMbBuffer, getBaseLayerRec()));

  //===== insert into frame =====
  RNOK( pcMCFrame->getFullPelYuvBuffer()->loadBuffer( &cYuvMbBuffer ) );

  return Err::m_nOK;
}


ErrVal
MbEncoder::compensateUpdate(  MbDataAccess&   rcMbDataAccess,
                              Frame*       pcMCFrame,
                              Int             iRefIdx,
                              ListIdx         eListPrd,
                              Frame*       pcPrdFrame)
{

  if( rcMbDataAccess.getMbData().isIntra() )
  {
    return Err::m_nOK;
  }
  else
  {
    if( rcMbDataAccess.getMbData().getMbMode() == MODE_8x8 || rcMbDataAccess.getMbData().getMbMode() == MODE_8x8ref0 )
    {
      for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
      {
        if( rcMbDataAccess.getMbData().getMbMotionData(eListPrd).getRefIdx( c8x8Idx.b8x8() ) == iRefIdx )
          RNOK( m_pcMotionEstimation->updateSubMb( c8x8Idx, rcMbDataAccess, pcMCFrame, pcPrdFrame, eListPrd ) );
      }
    }
    else
    {
      RNOK( m_pcMotionEstimation->updateMb( rcMbDataAccess, pcMCFrame, pcPrdFrame, eListPrd, iRefIdx ) );
    }
  }

  return Err::m_nOK;
}




ErrVal
MbEncoder::xCheckBestEstimation( IntMbTempData*& rpcMbTempData,
                                 IntMbTempData*& rpcMbBestData )
{
  ROFRS( rpcMbTempData->rdCost() < rpcMbBestData->rdCost(), Err::m_nOK );

  //----- switch data objects -----
  IntMbTempData*  pcTempData  = rpcMbTempData;
  rpcMbTempData               = rpcMbBestData;
  rpcMbBestData               = pcTempData;

  // Maintain state between encoder/decoder
  if( rpcMbBestData->getSH().getTCoeffLevelPredictionFlag() &&
    rpcMbBestData->getMbDataAccess().getMbData().is8x8TrafoFlagPresent(rpcMbBestData->getSH().getSPS().getDirect8x8InferenceFlag())
	  && rpcMbBestData->getMbDataAccess().getMbData().getBLSkipFlag() )
  {
	  if( !(rpcMbBestData->getMbDataAccess().getMbData().getMbCbp() & 0x0F) )
	  {
		  rpcMbBestData->getMbDataAccess().getMbData().setTransformSize8x8( false );
	  }
  }

  // Maintain state between encoder/decoder
  if( rpcMbBestData->getMbDataAccess().getMbData().getResidualPredFlag( PART_16x16 ) &&
    rpcMbBestData->getMbDataAccess().getCoeffResidualPredFlag() &&
    rpcMbBestData->getMbDataAccess().getMbData().is8x8TrafoFlagPresent(rpcMbBestData->getSH().getSPS().getDirect8x8InferenceFlag())
    )
  {
    if( !(rpcMbBestData->getMbDataAccess().getMbData().getMbCbp() & 0x0F) )
    {
      rpcMbBestData->getMbDataAccess().getMbData().setTransformSize8x8( false );
    }
  }

  return Err::m_nOK;
}



ErrVal
MbEncoder::xEstimateMbIntraBL( MbDataAccess&    rcMbDataAccess,   // JVT-V079,
                               IntMbTempData*&  rpcMbTempData,
                               IntMbTempData*&  rpcMbBestData,
                               const Frame*  pcBaseLayerRec,
                               Bool             bBSlice,
                               MbDataAccess*    pcMbDataAccessBase )
{
  ROF( pcBaseLayerRec );

  //JVT-U106 Behaviour at slice boundaries{
  if(!m_bIntraBLFlag)
      return Err::m_nOK;
  //JVT-U106 Behaviour at slice boundaries}

  //JVT-V079 Low-complexity MB mode decision
  Bool bLowComplexMbEnable = m_bLowComplexMbEnable[rpcMbTempData->getSH().getDependencyId()];
  
  Bool            bBLSkip           = pcMbDataAccessBase->getMbData().isIntra();
  UInt            uiCoeffBits       = 0;
  YuvMbBuffer& rcYuvMbBuffer     = *rpcMbTempData;
  YuvMbBuffer& rcTempYuvMbBuffer =  rpcMbTempData->getTempYuvMbBuffer();

  rpcMbTempData->clear();
  rpcMbTempData->setMbMode( INTRA_BL );
  rpcMbTempData->setBLSkipFlag( bBLSkip );
  rpcMbTempData->setTransformSize8x8( false );

  if (rpcMbTempData->getSH().getTCoeffLevelPredictionFlag())
  {
    Bool bClipValue                   = m_pcTransform->getClipMode();
    m_pcTransform->setClipMode( true );

    switch( pcMbDataAccessBase->getMbData().getMbMode() )
    {
    case INTRA_4X4:
      if( pcMbDataAccessBase->getMbData().isTransformSize8x8() == false )
      {
        RNOK( xEstimateMbIntra4 ( rcMbDataAccess, rpcMbTempData, rpcMbBestData, bBSlice, bBLSkip ) );
      }
      else
      {
        RNOK( xEstimateMbIntra8 (  rpcMbTempData, rpcMbBestData, bBSlice, bBLSkip ) );
      }
      break;
    case INTRA_BL:
      printf("Check if AvcRewriteFlag is on for all the layers below the current layer\n");
      break;
    case MODE_PCM:
      // Refinement of PCM data not supported by encoder.
      break;
    default:
      RNOK( xEstimateMbIntra16 (  rpcMbTempData, rpcMbBestData, bBSlice, bBLSkip ) );
      break;
    }

    m_pcTransform->setClipMode( bClipValue );
    return Err::m_nOK;
  }

  // else avc rewrite is not enabled
#if 0 //(HK) not sure what this 'if' is good for
  if( rpcMbTempData->getSH().getPPS().getEntropyCodingModeFlag() )
#endif
  {
    rpcMbTempData->setResidualPredFlag( true );
  }

  if( rcMbDataAccess.getSH().getCoeffResidualPredFlag() )
  {
    pcMbDataAccessBase->getMbTCoeffs().copyPredictionTo( rcYuvMbBuffer );
  }
  else
  {
    rcYuvMbBuffer    .loadBuffer( ((Frame*)pcBaseLayerRec)->getFullPelYuvBuffer() );
  }

  rcTempYuvMbBuffer.loadLuma  ( rcYuvMbBuffer );
  rcTempYuvMbBuffer.loadChroma( rcYuvMbBuffer );

  //===== encode residual and get rate for coefficients =====
  UInt  uiCoeffCost = 0;
  UInt  uiExtCbp    = 0;
  //--- LUMA ---
  for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
  {
    xSetCoeffCost( 0 );
    UInt  uiBits = 0;
    UInt  uiCbp  = 0;

    for( S4x4Idx cIdx( c8x8Idx ); cIdx.isLegal( c8x8Idx ); cIdx++ )
    {
      RNOK( xEncode4x4InterBlock( *rpcMbTempData, cIdx, uiBits, uiCbp ) );
    }
    if( uiCbp )
    {
      uiCoeffCost += xGetCoeffCost();
      uiExtCbp    += uiCbp;
      uiCoeffBits += uiBits;
    }
  }
  //--- CHROMA ---
  RNOK( xEncodeChromaTexture( *rpcMbTempData, uiExtCbp, uiCoeffBits ) );
  //--- get CBP ---
  rpcMbTempData->cbp() = xCalcMbCbp( uiExtCbp );

  rpcMbTempData->getMbMotionData( LIST_0 ).setMotPredFlag( false );
  rpcMbTempData->getMbMotionData( LIST_1 ).setMotPredFlag( false );


  //JVT-V079 Low-complexity MB mode decision {
  if ( !bLowComplexMbEnable )
  {
   RNOK( xSetRdCostIntraMb     ( *rpcMbTempData, uiCoeffBits, bBSlice, bBLSkip ) );
  }
  else
  {
   RNOK( xSetRdCostIntraMb     ( *rpcMbTempData, uiCoeffBits, bBSlice, bBLSkip ) );
   XPel*         pPel        = rpcMbTempData->getMbLumAddr();
   Int           iStride     = rpcMbTempData->getLStride();
   UInt uiDist  = m_pcXDistortion->getLum16x16( pPel, iStride, DF_HADAMARD );
    
    //--- store estimated parameters ---
   rpcMbTempData->rdCost() = uiDist;
  }
  //JVT-V079 Low-complexity MB mode decision }



  //JVT-R057 LA-RDO{
  if(m_bLARDOEnable)
  {
	  MbDataAccess& rcMbDataAccessl  = rpcMbTempData->getMbDataAccess();
	  Int x,y,blockX,blockY;
	  blockX=rcMbDataAccessl.getMbX()*4;
	  blockY=rcMbDataAccessl.getMbY()*4;
	  Int blockIndex;
	  Int ep_ref=0;
	  Int KBlock=m_pcIntPicBuffer->getLWidth()/4;
	  Int xx,yy;
	  for(y=blockY;y<(blockY+4);y++)
	  {
		  for(x=blockX;x<(blockX+4);x++)
		  {
			  xx=(Int)(x/m_aadRatio[m_uiLayerID][0]);
			  yy=(Int)(y/m_aadRatio[m_uiLayerID][1]);
			  blockIndex=yy*(Int)(KBlock/m_aadRatio[m_uiLayerID][0])+xx;
			  ep_ref+=const_cast<Frame*>(pcBaseLayerRec)->getChannelDistortion()[blockIndex];
		  }
	  }
	  setEpRef(ep_ref);
	  rpcMbTempData->rdCost()+=ep_ref;
  }
  //JVT-R057 LA-RDO}
  RNOK( xCheckBestEstimation  (  rpcMbTempData, rpcMbBestData ) );
  RNOK( xEstimateMbIntraBL8x8 (  rpcMbTempData, pcMbDataAccessBase, rpcMbBestData, pcBaseLayerRec, bBSlice, bBLSkip ) );

  return Err::m_nOK;
}


ErrVal
MbEncoder::xEstimateMbIntraBL8x8( IntMbTempData*&  rpcMbTempData,
                                  MbDataAccess*    pcMbDataAccessBase,
                                  IntMbTempData*&  rpcMbBestData,
                                  const Frame*  pcBaseLayerRec,
                                  Bool             bBSlice,
                                  Bool             bBLSkip )
{
  ROFRS( pcBaseLayerRec, Err::m_nOK  );
  ROTRS( ! rpcMbTempData->getSH().getPPS().getTransform8x8ModeFlag(), Err::m_nOK );

  rpcMbTempData->clear();
  rpcMbTempData->setMbMode( INTRA_BL );
  rpcMbTempData->setBLSkipFlag( bBLSkip );
  rpcMbTempData->setTransformSize8x8( true );

  UInt            uiCoeffBits       = 0;
  YuvMbBuffer& rcYuvMbBuffer     = *rpcMbTempData;
  YuvMbBuffer& rcTempYuvMbBuffer =  rpcMbTempData->getTempYuvMbBuffer();

  rpcMbTempData->setResidualPredFlag( true );
  if( rpcMbTempData->getSH().getCoeffResidualPredFlag() )
  {
    pcMbDataAccessBase->getMbTCoeffs().copyPredictionTo( rcYuvMbBuffer );
  }
  else
  {
    rcYuvMbBuffer    .loadBuffer( ((Frame*)pcBaseLayerRec)->getFullPelYuvBuffer() );
  }

  rcTempYuvMbBuffer.loadLuma  ( rcYuvMbBuffer );
  rcTempYuvMbBuffer.loadChroma( rcYuvMbBuffer );


  //===== encode residual and get rate for coefficients =====
  UInt  uiCoeffCost = 0;
  UInt  uiExtCbp    = 0;
  //--- LUMA ---
  for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
  {
    xSetCoeffCost( 0 );
    UInt  uiBits = 0;
    UInt  uiCbp  = 0;

    RNOK( xEncode8x8InterBlock( *rpcMbTempData, c8x8Idx, uiBits, uiCbp ) );
    if( uiCbp )
    {
      uiCoeffCost += xGetCoeffCost();
      uiExtCbp    += uiCbp;
      uiCoeffBits += uiBits;
    }
  }
  //--- CHROMA ---
  RNOK( xEncodeChromaTexture( *rpcMbTempData, uiExtCbp, uiCoeffBits ) );
  //--- get CBP ---
  rpcMbTempData->cbp() = xCalcMbCbp( uiExtCbp );

  rpcMbTempData->getMbMotionData( LIST_0 ).setMotPredFlag( false );
  rpcMbTempData->getMbMotionData( LIST_1 ).setMotPredFlag( false );


  RNOK( xSetRdCostIntraMb   ( *rpcMbTempData, uiCoeffBits, bBSlice, bBLSkip ) );

  //JVT-R057 LA-RDO{
  if(m_bLARDOEnable)
  {
	  rpcMbTempData->rdCost()+=getEpRef();
  }
  //JVT-R057 LA-RDO}

  ROT( rpcMbBestData->getSH().getTCoeffLevelPredictionFlag() );
  if( !rpcMbBestData->getMbDataAccess().getCoeffResidualPredFlag() )
  {
    RNOK( xCheckBestEstimation(  rpcMbTempData, rpcMbBestData ) );
  }
  else if( pcMbDataAccessBase->getMbData().isTransformSize8x8() )
  {
    IntMbTempData *pcSwitchPtr = rpcMbTempData;
    rpcMbTempData = rpcMbBestData;
    rpcMbBestData = pcSwitchPtr;
  }
  return Err::m_nOK;
}





ErrVal
MbEncoder::xEstimateMbIntra16( IntMbTempData*&  rpcMbTempData,
                               IntMbTempData*&  rpcMbBestData,
                               Bool             bBSlice,
							                 Bool             bBLSkip
                               )
{
  rpcMbTempData->clear();

  //JVT-V079 Low-complexity MB mode decision
  Bool bLowComplexMbEnable = m_bLowComplexMbEnable[rpcMbTempData->getSH().getDependencyId()];
  
  //----- init intra prediction -----
  rpcMbTempData->loadIntraPredictors( m_pcIntPicBuffer );
  rpcMbTempData->clearIntraPredictionModes( false );
  m_pcIntraPrediction->setAvailableMaskMb( rpcMbTempData->getMbDataAccess().getAvailableMask() );

  Int           iScalMat    = 0;
  const UChar*  pucScale    = ( rpcMbTempData->getSH().isScalingMatrixPresent(iScalMat) ? rpcMbTempData->getSH().getScalingMatrix(iScalMat) : NULL );
  XPel*         pPel        = rpcMbTempData->getMbLumAddr();
  Int           iStride     = rpcMbTempData->getLStride();
  UInt          uiPredMode  = 0;
  UInt          uiBestRd    = MSYS_UINT_MAX;
  UInt          uiBestBits  = 0;
  UInt          uiDist, uiBits = 0, uiRd, uiCost;

  // Make sure the uiPredMode is equal to the baselayer when AVC rewriting is enabled.
  Bool avcRewriteFlag = rpcMbTempData->getMbDataAccess().getSH().getTCoeffLevelPredictionFlag();

  if( avcRewriteFlag && bBLSkip )
    uiPredMode = rpcMbTempData->getMbDataAccess().getMbDataAccessBase()->getMbData().intraPredMode();

  for( Int n = 0; n < 4; n++ )
  {
    Bool bValid = true;
    RNOK( m_pcIntraPrediction->predictSLumaMb( rpcMbTempData, n, bValid ) );

    if( avcRewriteFlag && bBLSkip )
    {
      UInt ui_BLintraPredMode = rpcMbTempData->getMbDataAccess().getMbDataAccessBase()->getMbData().intraPredMode();
      bValid = ( n==ui_BLintraPredMode ) ? bValid : false;
    }

    if( ! bValid )
    {
      continue;
    }

    UInt  uiDcAbs = 0;
    UInt  uiAcAbs = 0;
    if ( !bLowComplexMbEnable )
    {

      RNOK( m_pcTransform->transformMb16x16( m_pcIntOrgMbPelData, rpcMbTempData, rpcMbTempData->get( B4x4Idx(0) ), pucScale, uiDcAbs, uiAcAbs ) );
      uiDist = m_pcXDistortion->getLum16x16( pPel, iStride );

      if( avcRewriteFlag && bBLSkip )
      {
        // Note: This currently makes uiDcAbs and uiAcAbs meaningless
        m_pcTransform->predictMb16x16( rpcMbTempData->get( B4x4Idx(0) ),
          rpcMbTempData->getMbDataAccess().getMbDataAccessBase()->getMbTCoeffs().get( B4x4Idx(0) ),
          rpcMbTempData->getMbDataAccess().getMbDataAccessBase()->getMbData().getQp(),
          uiDcAbs, uiAcAbs );
      }

      BitCounter::init();

      ROF( rpcMbTempData->getMbDataAccess().getMbData().getMbMode() == MODE_SKIP );
      rpcMbTempData->getMbDataAccess().getMbData().setMbMode( MbMode(INTRA_4X4 + 1) ); // one arbitrary Intra 16x16 mode

      if( avcRewriteFlag && bBLSkip )
      {
        for( B4x4Idx b4x4Idx; b4x4Idx.isLegal(); b4x4Idx++ )
          RNOK( MbCoder::xScanLumaBlock( *rpcMbTempData, *rpcMbTempData, b4x4Idx ) );
      }
      else
      {
        RNOK( MbCoder::xScanLumaIntra16x16( *rpcMbTempData, *rpcMbTempData, uiAcAbs != 0 ) );
      }

      rpcMbTempData->getMbDataAccess().getMbData().setMbMode( MODE_SKIP );

      uiBits = BitCounter::getNumberOfWrittenBits();

      uiRd = (UInt)floor(m_pcRateDistortionIf->getCost( uiBits, uiDist ));
    }
    else
     uiRd = uiDist = m_pcXDistortion->getLum16x16( pPel, iStride, DF_HADAMARD );


    if( uiRd < uiBestRd )
    {
      uiBestRd   = uiRd;
      uiPredMode = n;
      uiBestBits = uiBits;
    }
  }

  Bool  bValid  = true;
  UInt  uiDcAbs = 0;
  UInt  uiAcAbs = 0;
  RNOK( m_pcIntraPrediction->predictSLumaMb( rpcMbTempData, uiPredMode, bValid ) );

  if (avcRewriteFlag && !bValid && bBLSkip)
  {
    return Err::m_nOK;    // no valid mode is found
  }

  rpcMbTempData->getTempYuvMbBuffer().loadLuma( *rpcMbTempData );

  RNOK( m_pcTransform->transformMb16x16( m_pcIntOrgMbPelData, rpcMbTempData, rpcMbTempData->get( B4x4Idx(0) ), pucScale, uiDcAbs, uiAcAbs ) );

  UInt uiExtCbp = 0;
  if( avcRewriteFlag && bBLSkip )
  {
	  m_pcTransform->predictMb16x16( rpcMbTempData->get( B4x4Idx(0) ),
		  rpcMbTempData->getMbDataAccess().getMbDataAccessBase()->getMbTCoeffs().get( B4x4Idx(0) ),
		  rpcMbTempData->getMbDataAccess().getMbDataAccessBase()->getMbData().getQp(),
		  uiDcAbs, uiAcAbs );

	  // Note: uiDcAbs and uiAcAbs are now meaningless

	  // Creat a new ExtCbp
	  for( B4x4Idx b4x4Idx; b4x4Idx.isLegal(); b4x4Idx++ )
	  {
		  TCoeff* pcTCoeff = rpcMbTempData->get( b4x4Idx );
		  for( UInt n=0; n<16; n++ )
			  if( pcTCoeff[n]!=0 )
			  {
				  uiExtCbp |= 1 << b4x4Idx;
				  break;
			  }
	  }
  }

  if ( !bLowComplexMbEnable )
  {
   UInt    uiMBits = ( bBSlice ? 9 : 5 ) + 2;
   const DFunc&  rcDFunc = m_pcCodingParameter->getMotionVectorSearchParams().getSubPelDFunc();
   uiDist  = m_pcXDistortion->getLum16x16( pPel, iStride, rcDFunc );
   uiCost  = uiDist + m_pcMotionEstimation->getRateCost( uiBestBits + uiMBits, rcDFunc == DF_SAD );
  }
  else
  {
   uiCost = uiBestRd;
  }

  UInt uiChromaCbp = 0;
  if( avcRewriteFlag && bBLSkip )
	  rpcMbTempData->setMbMode( INTRA_BL );
  else
  rpcMbTempData->setMbMode( INTRA_4X4 ); // for right scaling matrix
  RNOK( xEncodeChromaIntra( *rpcMbTempData, uiChromaCbp, uiBestBits, bLowComplexMbEnable ) );
    
  if( avcRewriteFlag && (uiBestBits == MSYS_UINT_MAX ))
    return Err::m_nOK;    // no valid mode is found

  UInt  uiMbType  = INTRA_4X4 + 1;
        uiMbType += uiPredMode;
        uiMbType += ( uiAcAbs != 0 ) ? 12 : 0;
        uiMbType += uiChromaCbp >> 14;

  // needed for CABAC
  if (avcRewriteFlag)
  {
    if( bBLSkip )
      rpcMbTempData->cbp()  =   xCalcMbCbp( uiExtCbp + uiChromaCbp );
    else
      rpcMbTempData->cbp()  =   xCalcMbCbp( ( uiAcAbs != 0 ? 0xFFFF : 0 ) + uiChromaCbp );

    rpcMbTempData->setMbMode( bBLSkip ? INTRA_BL : MbMode( uiMbType ) );
    rpcMbTempData->setBLSkipFlag( bBLSkip );
  }
  else
  {
    rpcMbTempData->cbp()  =   xCalcMbCbp( ( uiAcAbs != 0 ? 0xFFFF : 0 ) + uiChromaCbp );
    rpcMbTempData->setMbMode( MbMode( uiMbType ) );
    rpcMbTempData->setBLSkipFlag( false );
  }

  rpcMbTempData->getMbMotionData( LIST_0 ).setMotPredFlag( false );
  rpcMbTempData->getMbMotionData( LIST_1 ).setMotPredFlag( false );

  //--- store estimated parameters ---
  rpcMbTempData->rdCost() = uiCost;


  if ( !bLowComplexMbEnable )
   RNOK( xSetRdCostIntraMb   ( *rpcMbTempData, uiBestBits, bBSlice, false ) );
  RNOK( xCheckBestEstimation(  rpcMbTempData, rpcMbBestData ) );

  return Err::m_nOK;
}



ErrVal
MbEncoder::xEstimateMbIntra4( MbDataAccess&    rcMbDataAccess, 
                                IntMbTempData*&  rpcMbTempData,
                                IntMbTempData*&  rpcMbBestData,
                                Bool             bBSlice,
                                Bool           bBLSkip
)
{
  /* JVT-V079: parameter rcMbDataAccess used to catch most probable intra pred mode */
 
  rpcMbTempData->clear();

  Bool avcRewriteFlag = rpcMbTempData->getSH().getTCoeffLevelPredictionFlag();
  if (avcRewriteFlag)
  {
    rpcMbTempData->setMbMode( bBLSkip?INTRA_BL:INTRA_4X4 );
    rpcMbTempData->setBLSkipFlag( bBLSkip );
  }
  else
  {
    rpcMbTempData->setMbMode( INTRA_4X4 );
    rpcMbTempData->setBLSkipFlag( false );
  }

  UInt uiQp    = rcMbDataAccess.getMbData().getQp();
  UInt uiCost, uiTempCost =0;
  
  //JVT-V079 Low-complexity MB mode decision
  Bool bLowComplexMbEnable = m_bLowComplexMbEnable[rpcMbTempData->getSH().getDependencyId()];
  
  //----- init intra prediction -----
  rpcMbTempData->loadIntraPredictors( m_pcIntPicBuffer );
  rpcMbTempData->clearIntraPredictionModes( false );
  m_pcIntraPrediction->setAvailableMaskMb( rpcMbTempData->getMbDataAccess().getAvailableMask() );


  UInt uiExtCbp = 0;
  UInt uiMbBits = 0;

  UInt tmp_max_qp = ( uiQp - SHIFT_QP );  
  if ( tmp_max_qp<0) tmp_max_qp=0;
  UInt lambda_val = 4 * g_iQP2quant[tmp_max_qp];
  
  if ( bLowComplexMbEnable )
  {
    for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
    {
      UInt uiBits = 0;
      UInt uiCbp = 0;
      for( S4x4Idx cIdx( c8x8Idx ); cIdx.isLegal( c8x8Idx ); cIdx++ )
      {
        Int mpMode = rcMbDataAccess.mostProbableIntraPredMode(LumaIdx(cIdx));
        RNOK( xEncode4x4IntraBlock( *rpcMbTempData, cIdx, uiBits, uiCbp, 
              mpMode, lambda_val, bLowComplexMbEnable ) );
        
        if( avcRewriteFlag && (uiBits == MSYS_UINT_MAX ))
          return Err::m_nOK;    // no valid mode is found

        rpcMbTempData->copyTo     ( rcMbDataAccess );
        uiTempCost += uiBits;
      }
      uiTempCost += 6 * g_iQP2quant[tmp_max_qp];

      if( uiCbp != 0 )
      {
        uiMbBits += uiBits;
        uiExtCbp += uiCbp;
      }
      else
      {
        uiMbBits += uiBits-4;
      }
    }
  }
  else
  {
    for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
    {
      UInt uiBits = 0;
      UInt uiCbp = 0;
      for( S4x4Idx cIdx( c8x8Idx ); cIdx.isLegal( c8x8Idx ); cIdx++ )
      {
        RNOK( xEncode4x4IntraBlock( *rpcMbTempData, cIdx, uiBits, uiCbp, 0, 0, false ) );
        if( avcRewriteFlag && (uiBits == MSYS_UINT_MAX ))
          return Err::m_nOK;    // no valid mode is found
      }
  
      if( uiCbp != 0 )
      {
        uiMbBits += uiBits;
        uiExtCbp += uiCbp;
      }
      else
      {
        uiMbBits += uiBits-4;
      }
    }
  }

  if ( !bLowComplexMbEnable )
  {
    XPel*         pPel    = rpcMbTempData->getMbLumAddr();
    Int           iStride = rpcMbTempData->getLStride();
    const DFunc&  rcDFunc = m_pcCodingParameter->getMotionVectorSearchParams().getSubPelDFunc();
    UInt          uiDist  = m_pcXDistortion->getLum16x16( pPel, iStride, rcDFunc );
    UInt          uiMBits = ( bBSlice ? 9 : 5 );
                  uiCost  = uiDist + m_pcMotionEstimation->getRateCost( uiMbBits + uiMBits, rcDFunc == DF_SAD );
  }
  else
    uiCost = uiTempCost;

  RNOK( xEncodeChromaIntra( *rpcMbTempData, uiExtCbp, uiMbBits, bLowComplexMbEnable ) );

  if( avcRewriteFlag && (uiMbBits == MSYS_UINT_MAX ))
    return Err::m_nOK;    	// no valid mode is found

  //--- store estimated parameters ---
  rpcMbTempData->rdCost() = uiCost;
  rpcMbTempData->cbp()    = xCalcMbCbp( uiExtCbp );

  rpcMbTempData->getMbMotionData( LIST_0 ).setMotPredFlag( false );
  rpcMbTempData->getMbMotionData( LIST_1 ).setMotPredFlag( false );

  if ( !bLowComplexMbEnable )
    RNOK( xSetRdCostIntraMb   ( *rpcMbTempData, uiMbBits, bBSlice, false ) );
  RNOK( xCheckBestEstimation(  rpcMbTempData, rpcMbBestData ) );

  return Err::m_nOK;
}



ErrVal
MbEncoder::xEstimateMbIntra8( IntMbTempData*&  rpcMbTempData,
                              IntMbTempData*&  rpcMbBestData,
                              Bool             bBSlice,
                              Bool bBLSkip
                              )
{
  ROTRS( ! rpcMbTempData->getSH().getPPS().getTransform8x8ModeFlag(), Err::m_nOK );

  rpcMbTempData->clear();

  Bool avcRewriteFlag = rpcMbTempData->getSH().getTCoeffLevelPredictionFlag();
  if (avcRewriteFlag)
  {
    rpcMbTempData->setMbMode( bBLSkip?INTRA_BL:INTRA_4X4 );
    rpcMbTempData->setBLSkipFlag( bBLSkip );
  }
  else
  {
    rpcMbTempData->setMbMode( INTRA_4X4 );
    rpcMbTempData->setBLSkipFlag( false );
  }

  rpcMbTempData->setTransformSize8x8( true );

  //----- init intra prediction -----
  rpcMbTempData->loadIntraPredictors( m_pcIntPicBuffer );
  rpcMbTempData->clearIntraPredictionModes( true );
  m_pcIntraPrediction->setAvailableMaskMb( rpcMbTempData->getMbDataAccess().getAvailableMask() );

  UInt uiExtCbp = 0;
  UInt uiMbBits = 0;

  for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
  {
    RNOK( xEncode8x8IntraBlock( *rpcMbTempData, c8x8Idx, uiMbBits, uiExtCbp ) );     
    if( avcRewriteFlag && (uiMbBits == MSYS_UINT_MAX ))
      return Err::m_nOK;  // no valid mode is found
  }

  XPel*         pPel      = rpcMbTempData->getMbLumAddr();
  Int           iStride   = rpcMbTempData->getLStride();
  const DFunc&  rcDFunc   = m_pcCodingParameter->getMotionVectorSearchParams().getSubPelDFunc();
  UInt          uiDist    = m_pcXDistortion->getLum16x16( pPel, iStride, rcDFunc );
  UInt          uiMBits   = ( bBSlice ? 9 : 5 );
  UInt          uiCost    = uiDist + m_pcMotionEstimation->getRateCost( uiMbBits + uiMBits, rcDFunc == DF_SAD );

  RNOK( xEncodeChromaIntra( *rpcMbTempData, uiExtCbp, uiMbBits ) );   
  if( avcRewriteFlag && (uiMbBits == MSYS_UINT_MAX ))
    return Err::m_nOK;    // no valid mode is found

  //----- store estimated parameters -----
  rpcMbTempData->rdCost() = uiCost;
  rpcMbTempData->cbp()    = xCalcMbCbp( uiExtCbp );

  rpcMbTempData->getMbMotionData( LIST_0 ).setMotPredFlag( false );
  rpcMbTempData->getMbMotionData( LIST_1 ).setMotPredFlag( false );

  RNOK( xSetRdCostIntraMb   ( *rpcMbTempData, uiMbBits, bBSlice, false ) );
  RNOK( xCheckBestEstimation(  rpcMbTempData, rpcMbBestData ) );

  return Err::m_nOK;
}


ErrVal
MbEncoder::xEstimateMbPCM( IntMbTempData*&   rpcMbTempData,
                           IntMbTempData*&   rpcMbBestData,
                           Bool              bBSlice  )
{
  return Err::m_nOK;

  rpcMbTempData->clear();
  rpcMbTempData->setMbMode( MODE_PCM );
  rpcMbTempData->setBLSkipFlag( false );

  rpcMbTempData->loadLuma   ( *m_pcIntOrgMbPelData );
  rpcMbTempData->loadChroma ( *m_pcIntOrgMbPelData );

  Pel*  pucDest   = rpcMbTempData->getPelBuffer();
  XPel* pucSrc    = rpcMbTempData->getMbLumAddr();
  Int   iStride   = rpcMbTempData->getLStride();
  UInt  uiDist    = 0;
  UInt  uiMbBits  = 8*8*8*6+(bBSlice?11:9)+4;
  Int   iDelta   = 1;
  Int   n, m, n1, m1, dest, cnt, diff, idx;

  for( n = 0; n < 16; n+=iDelta )
  {
    for( m = 0; m < 16; m+=iDelta )
    {
      dest  = 0;
      cnt   = 0;
      for( n1=0; n1<  iDelta; n1++)
      for( m1=m; m1<m+iDelta; m1++)
      {
        dest += pucSrc[m1+n1*iStride];
        cnt  ++;
      }
      dest  = ( dest + cnt / 2 ) / cnt;
      dest  = min( 255, max( 1, dest ) );

      *pucDest = (Pel)dest;
      pucDest++;

      for( n1=0; n1<  iDelta; n1++)
      for( m1=m; m1<m+iDelta; m1++)
      {
        idx  = m1 + n1*iStride;
        diff = pucSrc[idx] - dest;
        pucSrc[idx]        = dest;
        uiDist += diff * diff;
      }
    }
    pucSrc  += iDelta*iStride;
  }

  pucSrc  = rpcMbTempData->getMbCbAddr();
  iStride = rpcMbTempData->getCStride();

  for( n = 0; n < 8; n+=iDelta )
  {
    for( m = 0; m < 8; m+=iDelta )
    {
      dest  = 0;
      cnt   = 0;
      for( n1=0; n1<  iDelta; n1++)
      for( m1=m; m1<m+iDelta; m1++)
      {
        dest += pucSrc[m1+n1*iStride];
        cnt  ++;
      }
      dest  = ( dest + cnt / 2 ) / cnt;
      dest  = min( 255, max( 1, dest ) );

      *pucDest = (Pel)dest;
      pucDest++;

      for( n1=0; n1<  iDelta; n1++)
      for( m1=m; m1<m+iDelta; m1++)
      {
        idx  = m1 + n1*iStride;
        diff = pucSrc[idx] - dest;
        pucSrc[idx]        = dest;
        uiDist += diff * diff;
      }
    }
    pucSrc  += iDelta*iStride;
  }

  pucSrc  = rpcMbTempData->getMbCrAddr();

  for( n = 0; n < 8; n+=iDelta )
  {
    for( m = 0; m < 8; m+=iDelta )
    {
      dest  = 0;
      cnt   = 0;
      for( n1=0; n1<  iDelta; n1++)
      for( m1=m; m1<m+iDelta; m1++)
      {
        dest += pucSrc[m1+n1*iStride];
        cnt  ++;
      }
      dest  = ( dest + cnt / 2 ) / cnt;
      dest  = min( 255, max( 1, dest ) );

      *pucDest = (Pel)dest;
      pucDest++;

      for( n1=0; n1<  iDelta; n1++)
      for( m1=m; m1<m+iDelta; m1++)
      {
        idx  = m1 + n1*iStride;
        diff = pucSrc[idx] - dest;
        pucSrc[idx]        = dest;
        uiDist += diff * diff;
      }
    }
    pucSrc  += iDelta*iStride;
  }

  rpcMbTempData->getTempYuvMbBuffer().setAllSamplesToZero();

  const DFunc&  rcDFunc   = m_pcCodingParameter->getMotionVectorSearchParams().getSubPelDFunc();
  rpcMbTempData->rdCost() = uiDist + m_pcMotionEstimation->getRateCost( uiMbBits, rcDFunc == DF_SAD );
  rpcMbTempData->cbp()    = 0;

  rpcMbTempData->getMbMotionData( LIST_0 ).setMotPredFlag( false );
  rpcMbTempData->getMbMotionData( LIST_1 ).setMotPredFlag( false );

  rpcMbTempData->rdCost() = m_pcRateDistortionIf->getCost( uiMbBits, uiDist );

  RNOK( xCheckBestEstimation( rpcMbTempData, rpcMbBestData ) );

  return Err::m_nOK;
}







UInt MbEncoder::xCalcMbCbp( UInt uiExtCbp )
{
  UInt uiMbCbp;
  {
    UInt uiCbp = uiExtCbp;
    uiMbCbp  = (0 != (uiCbp & 0x33)) ? 1 : 0;
    uiMbCbp += (0 != (uiCbp & 0xcc)) ? 2 : 0;
    uiCbp >>= 8;
    uiMbCbp += (0 != (uiCbp & 0x33)) ? 4 : 0;
    uiMbCbp += (0 != (uiCbp & 0xcc)) ? 8 : 0;
  }
  uiMbCbp += (uiExtCbp >> 16) << 4;

  return (uiMbCbp<<24) | uiExtCbp;
}






__inline Void MbEncoder::xUpDateBest( IntMbTempData& rcMbTempData )
{
  // no bits available
  if( ! rcMbTempData.isPCM() )
  {
    BitCounter::init();
    if( ! rcMbTempData.isIntra16x16() )
    {
      rcMbTempData.cbp() = xCalcMbCbp( rcMbTempData.cbp() );
      MbCoder::m_pcMbSymbolWriteIf->cbp( rcMbTempData );
      if( rcMbTempData.cbp() )
      {
        MbCoder::m_pcMbSymbolWriteIf->deltaQp( rcMbTempData );
      }
    }
    else
    {
      MbCoder::m_pcMbSymbolWriteIf->deltaQp( rcMbTempData );
    }


    if( ! rcMbTempData.isSkiped() )
    {
      MbCoder::m_pcMbSymbolWriteIf->mbMode( rcMbTempData/*, false*/ );

      rcMbTempData.bits() += (rcMbTempData.getSH().isIntraSlice() ? 0 : 1); // run
    }
    else
    {
      rcMbTempData.bits() += 1; // run
    }

    rcMbTempData.bits() += BitCounter::getNumberOfWrittenBits();
  }


  UInt uiDist;
  uiDist  = rcMbTempData.distY();
  uiDist += rcMbTempData.distU();
  uiDist += rcMbTempData.distV();

  rcMbTempData.rdCost() = m_pcRateDistortionIf->getCost( rcMbTempData.bits(), uiDist );

  ROFVS( rcMbTempData.rdCost() < m_pcIntMbBestData->rdCost() )

  IntMbTempData* pcMbTempData = &rcMbTempData;

  m_pcIntMbTempData = m_pcIntMbBestData;

  m_pcIntMbBestData = pcMbTempData;
}





ErrVal
MbEncoder::xEncode4x4InterBlock( IntMbTempData& rcMbTempData,
                                 LumaIdx        cIdx,
                                 UInt&          ruiBits,
                                 UInt&          ruiExtCbp )
{
  rcMbTempData.set4x4Block( cIdx );
  m_pcIntOrgMbPelData->set4x4Block( cIdx );

  UInt uiBits = 0;
  UInt uiAbsSum = 0;

  Int           iScalMat  = ( rcMbTempData.isIntra() ? 0 : 3 );
  const UChar*  pucScale  = ( rcMbTempData.getSH().isScalingMatrixPresent(iScalMat) ? rcMbTempData.getSH().getScalingMatrix(iScalMat) : NULL );

  Bool coeffResidualPredFlag = rcMbTempData.getMbDataAccess().getCoeffResidualPredFlag();
  if( coeffResidualPredFlag &&  rcMbTempData.getResidualPredFlag( PART_16x16 ) )
  {
		TCoeff* piCoeffBase = rcMbTempData.getMbDataAccess().getMbDataAccessBase()->getMbTCoeffs().get( cIdx );
    RNOK( m_pcTransform->transform4x4BlkCGS( m_pcIntOrgMbPelData, rcMbTempData, rcMbTempData.get( cIdx ), piCoeffBase, pucScale, uiAbsSum ) );
  }
  else
    RNOK( m_pcTransform->transform4x4Blk( m_pcIntOrgMbPelData, rcMbTempData, rcMbTempData.get( cIdx ), pucScale, uiAbsSum ) );

  if( rcMbTempData.getSH().getTCoeffLevelPredictionFlag() && rcMbTempData.getResidualPredFlag( PART_16x16 ) )
  {
	  RNOK( m_pcTransform->predict4x4Blk( rcMbTempData.get( cIdx ),
		  rcMbTempData.getMbDataAccess().getMbDataAccessBase()->getMbTCoeffs().get( cIdx ),
		  rcMbTempData.getMbDataAccess().getMbDataAccessBase()->getMbData().getQp(),
		  uiAbsSum ) );
  }

  if( 0 == uiAbsSum )
  {
    ruiBits += 1;
    return Err::m_nOK;
  }

  BitCounter::init();
  RNOK( MbCoder::xScanLumaBlock( rcMbTempData, rcMbTempData, cIdx ) );
  uiBits = BitCounter::getNumberOfWrittenBits();
  AOT_DBG( uiBits == 0);

  ruiExtCbp |= 1 << cIdx;
  ruiBits   += uiBits;

  return Err::m_nOK;
}



ErrVal
MbEncoder::xEncode8x8InterBlock( IntMbTempData& rcMbTempData,
                                 B8x8Idx        c8x8Idx,
                                 UInt&          ruiBits,
                                 UInt&          ruiExtCbp )
{
  rcMbTempData.set4x4Block( c8x8Idx );
  m_pcIntOrgMbPelData->set4x4Block( c8x8Idx );

  UInt uiBits     = 0;
  UInt uiAbsSum   = 0;

  Int           iScalMat  = ( rcMbTempData.isIntra() ? 6 : 7 );
  const UChar*  pucScale  = ( rcMbTempData.getSH().isScalingMatrixPresent(iScalMat) ? rcMbTempData.getSH().getScalingMatrix(iScalMat) : NULL );

  Bool coeffResidualPredFlag = rcMbTempData.getMbDataAccess().getCoeffResidualPredFlag();
  if ( coeffResidualPredFlag &&  rcMbTempData.getResidualPredFlag( PART_16x16 ) )
  {
		TCoeff* piCoeffBase = rcMbTempData.getMbDataAccess().getMbDataAccessBase()->getMbTCoeffs().get8x8( c8x8Idx );
    RNOK( m_pcTransform->transform8x8BlkCGS( m_pcIntOrgMbPelData, rcMbTempData, rcMbTempData.get8x8( c8x8Idx ), piCoeffBase, pucScale, uiAbsSum ) );
  }
  else
    RNOK( m_pcTransform->transform8x8Blk( m_pcIntOrgMbPelData, rcMbTempData, rcMbTempData.get8x8( c8x8Idx ), pucScale, uiAbsSum ) );

  if( rcMbTempData.getSH().getTCoeffLevelPredictionFlag() && rcMbTempData.getResidualPredFlag( PART_16x16 ) )
  {
	  RNOK( m_pcTransform->predict8x8Blk( rcMbTempData.get8x8( c8x8Idx ),
		  rcMbTempData.getMbDataAccess().getMbDataAccessBase()->getMbTCoeffs().get8x8( c8x8Idx ),
		  rcMbTempData.getMbDataAccess().getMbDataAccessBase()->getMbData().getQp(),
		  uiAbsSum ) );
  }

  if( 0 == uiAbsSum )
  {
    ruiBits += 1;
    return Err::m_nOK;
  }

  {
    BitCounter::init();
    RNOK( m_pcMbSymbolWriteIf->residualBlock8x8( rcMbTempData, c8x8Idx, LUMA_SCAN ) )
    uiBits = ( BitCounter::getNumberOfWrittenBits() );
    AOT_DBG( uiBits == 0);
  }

  ruiExtCbp |= 0x33 << c8x8Idx.b4x4();
  ruiBits   += uiBits;

  return Err::m_nOK;
}



ErrVal
MbEncoder::xEncode16x16ResidualMB( IntMbTempData& rcMbTempData,
                                   UInt&          ruiBits,
                                   UInt&          ruiExtCbp )
{
  UInt uiDCAbs  = 0;
  UInt uiACAbs  = 0;

  Int           iScalMat  = ( 0 );
  const UChar*  pucScale  = ( rcMbTempData.getSH().isScalingMatrixPresent(iScalMat) ? rcMbTempData.getSH().getScalingMatrix(iScalMat) : NULL );

  RNOK( m_pcTransform->transformMb16x16( m_pcIntOrgMbPelData,
                                         rcMbTempData,
                                         rcMbTempData.get( B4x4Idx(0) ), pucScale, uiDCAbs, uiACAbs )  );

  BitCounter::init();
  ROF( rcMbTempData.getMbDataAccess().getMbData().getMbMode() == MODE_SKIP );
  rcMbTempData.getMbDataAccess().getMbData().setMbMode( MbMode(INTRA_4X4 + 1) ); // one arbitrary Intra 16x16 mode
  RNOK( MbCoder::xScanLumaIntra16x16( rcMbTempData, rcMbTempData, uiACAbs != 0 ) );
  rcMbTempData.getMbDataAccess().getMbData().setMbMode( MODE_SKIP );
  ruiBits   = ( BitCounter::getNumberOfWrittenBits() );
  ruiExtCbp = ( uiACAbs ? 0xFFFF : 0 );

  return Err::m_nOK;
}



ErrVal
MbEncoder::xEncode8x8IntraBlock( IntMbTempData& rcMbTempData,
                                 B8x8Idx        c8x8Idx,
                                 UInt&          ruiBits,
                                 UInt&          ruiExtCbp )
{
  rcMbTempData.        set4x4Block( c8x8Idx );
  m_pcIntOrgMbPelData->set4x4Block( c8x8Idx );

  UInt   uiAbsSum;
  Double fBestRd     = DOUBLE_MAX;
  UInt   uiBestMode  = 2;
  UInt   uiBestBits  = 0;
  Int    iPredMode   = 0;
  Bool   bValid      = true;

  Bool avcRewriteFlag = rcMbTempData.getSH().getTCoeffLevelPredictionFlag();
  if( avcRewriteFlag && rcMbTempData.isIntraBL() )
	  rcMbTempData.intraPredMode( c8x8Idx ) = rcMbTempData.getMbDataAccess().getMbDataAccessBase()->getMbData().intraPredMode( c8x8Idx );

  Int           iScalMat  = 6;
  const UChar*  pucScale  = ( rcMbTempData.getSH().isScalingMatrixPresent(iScalMat) ? rcMbTempData.getSH().getScalingMatrix(iScalMat) : NULL );

  for( UInt n = 0; n < 9; n++)
  {
    bValid    = true;
    iPredMode = n;

    if( avcRewriteFlag && rcMbTempData.isIntraBL() && bValid )
    {
      bValid = (n==rcMbTempData.intraPredMode( c8x8Idx ) );
    }

    if( bValid )
    {
      RNOK( m_pcIntraPrediction->predictSLumaBlock8x8( rcMbTempData, iPredMode, c8x8Idx, bValid ) );
    }

    if( bValid )
    {
      RNOK( m_pcTransform->transform8x8Blk( m_pcIntOrgMbPelData, rcMbTempData, rcMbTempData.get8x8( c8x8Idx ), pucScale, uiAbsSum = 0 ) );
      UInt uiDist = m_pcXDistortion->getLum8x8( rcMbTempData.getLumBlk(), rcMbTempData.getLStride() );

      Int uiBits = 0;

      if( avcRewriteFlag && rcMbTempData.isIntraBL() )
      {
        RNOK( m_pcTransform->predict8x8Blk( rcMbTempData.get8x8( c8x8Idx ),
          rcMbTempData.getMbDataAccess().getMbDataAccessBase()->getMbTCoeffs().get8x8( c8x8Idx ),
          rcMbTempData.getMbDataAccess().getMbDataAccessBase()->getMbData().getQp(),
          uiAbsSum ) );
      }
      else
      {
        rcMbTempData.intraPredMode(c8x8Idx) = iPredMode;

        BitCounter::init();
        m_pcMbSymbolWriteIf->intraPredModeLuma( rcMbTempData, c8x8Idx );
        uiBits = BitCounter::getNumberOfWrittenBits() + 1;
      }

      if( 0 != uiAbsSum )
      {
        BitCounter::init();
        RNOK( m_pcMbSymbolWriteIf->residualBlock8x8( rcMbTempData, c8x8Idx, LUMA_SCAN ) )
        uiBits += ( BitCounter::getNumberOfWrittenBits() ) - 1;
      }

      Double fCost = m_pcRateDistortionIf->getFCost( uiBits, uiDist );

      if( fCost < fBestRd )
      {
        fBestRd     = fCost;
        uiBestBits  = uiBits;
        uiBestMode  = iPredMode;
      }
    }
  }

  RNOK( m_pcIntraPrediction->predictSLumaBlock8x8( rcMbTempData, uiBestMode, c8x8Idx, bValid ) );
  {
    S4x4Idx cIdx4x4(c8x8Idx);
    for( Int n = 0; n < 4; n++, cIdx4x4++ )
    {
      rcMbTempData.intraPredMode( cIdx4x4 ) = uiBestMode;
    }
  }

  rcMbTempData.getTempYuvMbBuffer().loadLuma( rcMbTempData, c8x8Idx );

  RNOK( m_pcTransform->transform8x8Blk( m_pcIntOrgMbPelData, rcMbTempData, rcMbTempData.get8x8( c8x8Idx ), pucScale, uiAbsSum = 0 ) );

  if( avcRewriteFlag && rcMbTempData.isIntraBL() )
  {
	  RNOK( m_pcTransform->predict8x8Blk( rcMbTempData.get8x8( c8x8Idx ),
		  rcMbTempData.getMbDataAccess().getMbDataAccessBase()->getMbTCoeffs().get8x8( c8x8Idx ),
		  rcMbTempData.getMbDataAccess().getMbDataAccessBase()->getMbData().getQp(),
		  uiAbsSum ) );
  }

  if( 0 != uiAbsSum )
  {
    ruiExtCbp |= 0x33 << c8x8Idx.b4x4();
  }

  if ( avcRewriteFlag && rcMbTempData.isIntraBL() && rcMbTempData.getMbDataAccess().getMbDataAccessBase() )
  {
    if( uiBestMode != rcMbTempData.getMbDataAccess().getMbDataAccessBase()->getMbData().intraPredMode( c8x8Idx ) )
      ruiBits = MSYS_UINT_MAX;  // mode is different from base layer, this is not allowed when avc rewriting is enabled
  }

  if( ruiBits != MSYS_UINT_MAX )
    ruiBits += uiBestBits;

  return Err::m_nOK;
}





ErrVal
MbEncoder::xEncode4x4IntraBlock( IntMbTempData& rcMbTempData,
                                 LumaIdx        cIdx,
                                 UInt&          ruiBits,
                                 UInt&          ruiExtCbp,
                                 UInt           mpMode,
                                 UInt           lambda_val,
                                 Bool           bLowComplexity )
{
  rcMbTempData.        set4x4Block( cIdx );
  m_pcIntOrgMbPelData->set4x4Block( cIdx );
  UInt uiAbsSum;
  Double fCost;
  UInt uiDist, uiBits=0;

  Double fBestRd = DOUBLE_MAX;
  UInt uiBestMode = 2;
  UInt uiBestBits = 0;
  Int iPredMode = 0;
  Bool bValid = true;

  Bool avcRewriteFlag = rcMbTempData.getSH().getTCoeffLevelPredictionFlag();
  if( avcRewriteFlag && rcMbTempData.isIntraBL() )
	  rcMbTempData.intraPredMode(cIdx) = rcMbTempData.getMbDataAccess().getMbDataAccessBase()->getMbData().intraPredMode(cIdx);

  Int           iScalMat  = 0;
  const UChar*  pucScale  = ( rcMbTempData.getSH().isScalingMatrixPresent(iScalMat) ? rcMbTempData.getSH().getScalingMatrix(iScalMat) : NULL );

  for( UInt n = 0; n < 9; n++)
  {
    bValid    = true;
    iPredMode = n;

    if( avcRewriteFlag && rcMbTempData.isIntraBL() && bValid )
    {
      bValid = (n==rcMbTempData.intraPredMode(cIdx));
    }

    if( bValid )
    {
      RNOK( m_pcIntraPrediction->predictSLumaBlock( rcMbTempData, iPredMode, cIdx, bValid ) );
    }

    if( bValid )
    {
      if ( !bLowComplexity )
      {
        RNOK( m_pcTransform->transform4x4Blk( m_pcIntOrgMbPelData, rcMbTempData, rcMbTempData.get( cIdx ), pucScale, uiAbsSum = 0 ) );

        uiDist = m_pcXDistortion->getLum4x4( rcMbTempData.getLumBlk(), rcMbTempData.getLStride() );

        uiBits = 0;
        if( avcRewriteFlag && rcMbTempData.isIntraBL() )
        {
          RNOK( m_pcTransform->predict4x4Blk( rcMbTempData.get( cIdx ),
          rcMbTempData.getMbDataAccess().getMbDataAccessBase()->getMbTCoeffs().get( cIdx ),
          rcMbTempData.getMbDataAccess().getMbDataAccessBase()->getMbData().getQp(),
          uiAbsSum ) );
        }
        else
        {
          BitCounter::init();
          rcMbTempData.intraPredMode(cIdx) = iPredMode;

          m_pcMbSymbolWriteIf->intraPredModeLuma( rcMbTempData, cIdx );
          uiBits = BitCounter::getNumberOfWrittenBits() + 1;
        }

        if( 0 != uiAbsSum )
        {
          BitCounter::init();
          RNOK( MbCoder::xScanLumaBlock( rcMbTempData, rcMbTempData, cIdx ) );
          uiBits += BitCounter::getNumberOfWrittenBits() - 1;
        }

        fCost = m_pcRateDistortionIf->getFCost( uiBits, uiDist );
      }
      else
      {
        fCost = uiDist = m_pcXDistortion->getLum4x4( rcMbTempData.getLumBlk(), rcMbTempData.getLStride(), DF_SAD );
        fCost += ( iPredMode == mpMode ) ? 0 : lambda_val;
      }
        

      if( fCost < fBestRd )
      {
        fBestRd = fCost;
        uiBestBits = uiBits;
        uiBestMode = iPredMode;
      }
    }
  }

  RNOK( m_pcIntraPrediction->predictSLumaBlock( rcMbTempData, uiBestMode, cIdx, bValid ) );
  rcMbTempData.intraPredMode( cIdx ) = uiBestMode;

  rcMbTempData.getTempYuvMbBuffer().loadLuma( rcMbTempData, cIdx );

  RNOK( m_pcTransform->transform4x4Blk( m_pcIntOrgMbPelData, rcMbTempData, rcMbTempData.get( cIdx ), pucScale, uiAbsSum = 0 ) );

  if( avcRewriteFlag && rcMbTempData.isIntraBL() )
  {
	  RNOK( m_pcTransform->predict4x4Blk( rcMbTempData.get( cIdx ),
		  rcMbTempData.getMbDataAccess().getMbDataAccessBase()->getMbTCoeffs().get( cIdx ),
		  rcMbTempData.getMbDataAccess().getMbDataAccessBase()->getMbData().getQp(),
		  uiAbsSum ) );
  }

  if( 0 != uiAbsSum )
  {
    ruiExtCbp |= 1 << cIdx;
  }

  if ( avcRewriteFlag && rcMbTempData.isIntraBL() && rcMbTempData.getMbDataAccess().getMbDataAccessBase() )
  {
    if( uiBestMode != rcMbTempData.getMbDataAccess().getMbDataAccessBase()->getMbData().intraPredMode( cIdx ) )
      ruiBits = MSYS_UINT_MAX;
  }
  if( ruiBits != MSYS_UINT_MAX )
    ruiBits += uiBestBits;

  if ( bLowComplexity )
    ruiBits = (UInt) fBestRd; //warning
  
  return Err::m_nOK;
}




Void
MbEncoder::xReStoreParameter( MbDataAccess&   rcMbDataAccess,
                              IntMbTempData&  rcMbBestData )
{
  if( rcMbBestData.isIntra() )
  {
    rcMbBestData.getMbMotionData( LIST_0 ).clear( BLOCK_NOT_PREDICTED );
    rcMbBestData.getMbMvdData   ( LIST_0 ).clear();

    if( rcMbDataAccess.getSH().isBSlice() )
    {
      rcMbBestData.getMbMotionData( LIST_1 ).clear( BLOCK_NOT_PREDICTED );
      rcMbBestData.getMbMvdData   ( LIST_1 ).clear();
    }
  }
  else if( rcMbBestData.isSkiped() )
  {
    rcMbBestData.getMbMvdData( LIST_0 ).clear();

    if( rcMbDataAccess.getSH().isBSlice() )
    {
      rcMbBestData.getMbMvdData( LIST_1 ).clear();
    }
  }

  UInt uiFwdBwd = 0;
  if( rcMbDataAccess.getSH().isBSlice() )
  {
    for( Int n = 3; n >= 0; n--)
    {
      uiFwdBwd <<= 4;
      uiFwdBwd += (0 < rcMbBestData.getMbMotionData( LIST_0 ).getRefIdx( Par8x8(n) )) ? 1:0;
      uiFwdBwd += (0 < rcMbBestData.getMbMotionData( LIST_1 ).getRefIdx( Par8x8(n) )) ? 2:0;
    }
  }

  if( rcMbDataAccess.getSH().isPSlice() )
  {
    for( Int n = 3; n >= 0; n--)
    {
      uiFwdBwd <<= 4;
      uiFwdBwd +=  (0 < rcMbBestData.getMbMotionData( LIST_0 ).getRefIdx( Par8x8(n) )) ? 1:0;
    }
  }
  rcMbBestData.setFwdBwd  ( uiFwdBwd );
  rcMbBestData.copyTo     ( rcMbDataAccess );
  m_pcIntPicBuffer->loadBuffer( &rcMbBestData );
}



Void
MbEncoder::xStoreEstimation( MbDataAccess&   rcMbDataAccess,
                             IntMbTempData&  rcMbBestData,
                             Frame*       pcRecSubband,
                             Frame*       pcPredSignal,
                             Bool            bMotionFieldEstimation,
                             YuvMbBuffer* pcBaseLayerBuffer )
{
  if( bMotionFieldEstimation )
  {
    //===== reset parameters: IMPORTANT FOR ENCODING OF MOTION FIELDS =====
    rcMbBestData.cbp() = 0;
    rcMbBestData.setAllCoeffCount( 0 );
  }

  if( rcMbBestData.isIntra() )
  {
    rcMbBestData.getMbMotionData( LIST_0 ).clear( BLOCK_NOT_PREDICTED );
    rcMbBestData.getMbMvdData   ( LIST_0 ).clear();

    if( rcMbDataAccess.getSH().isBSlice() )
    {
      rcMbBestData.getMbMotionData( LIST_1 ).clear( BLOCK_NOT_PREDICTED );
      rcMbBestData.getMbMvdData   ( LIST_1 ).clear();
    }

    if( bMotionFieldEstimation )
    {
      rcMbBestData.setMbMode( INTRA_4X4 );
    }
  }
  else if( rcMbBestData.isSkiped() )
  {
    rcMbBestData.getMbMvdData( LIST_0 ).clear();

    if( rcMbDataAccess.getSH().isBSlice() )
    {
      rcMbBestData.getMbMvdData( LIST_1 ).clear();
    }
  }

  UInt uiFwdBwd = 0;
  if( rcMbDataAccess.getSH().isBSlice() )
  {
    for( Int n = 3; n >= 0; n--)
    {
      uiFwdBwd <<= 4;
      uiFwdBwd += (0 < rcMbBestData.getMbMotionData( LIST_0 ).getRefIdx( Par8x8(n) )) ? 1:0;
      uiFwdBwd += (0 < rcMbBestData.getMbMotionData( LIST_1 ).getRefIdx( Par8x8(n) )) ? 2:0;
    }
  }

  if( rcMbDataAccess.getSH().isPSlice() )
  {
    for( Int n = 3; n >= 0; n--)
    {
      uiFwdBwd <<= 4;
      uiFwdBwd +=  (0 < rcMbBestData.getMbMotionData( LIST_0 ).getRefIdx( Par8x8(n) )) ? 1:0;
    }
  }
  rcMbBestData.setFwdBwd  ( uiFwdBwd );

  // pin the prediction to the transform coefficients
  if( rcMbBestData.isIntraBL() && !rcMbDataAccess.getSH().getTCoeffLevelPredictionFlag() )
  {
    rcMbBestData.getMbTCoeffs().copyPredictionFrom( rcMbBestData.getTempYuvMbBuffer() );
  }
  else
  {
    rcMbBestData.getMbTCoeffs().copyPredictionFrom( rcMbBestData );
  }

  rcMbBestData.copyTo     ( rcMbDataAccess );
  Bool avcRewriteFlag = rcMbDataAccess.getSH().getTCoeffLevelPredictionFlag();

  if( ! rcMbDataAccess.getMbData().isIntra4x4() && ( rcMbDataAccess.getMbData().getMbCbp() & 0x0F ) == 0 )
  {
    if (!avcRewriteFlag || (avcRewriteFlag && !rcMbDataAccess.getMbData().isIntraBL()) )
      rcMbDataAccess.getMbData().setTransformSize8x8( false );
  }

  if( rcMbDataAccess.getMbData().getResidualPredFlag( PART_16x16 ) && ! rcMbDataAccess.getMbData().isIntra() )
  {
    AOF( pcBaseLayerBuffer );
    if (!avcRewriteFlag)
      ((YuvMbBuffer&)rcMbBestData).add( *pcBaseLayerBuffer );
  }

  if( pcPredSignal )
  {
    ANOK( pcPredSignal->getFullPelYuvBuffer()->loadBuffer( &rcMbBestData.getTempYuvMbBuffer() ) );
  }

  if( pcRecSubband )
  {
    YuvMbBuffer cYuvMbBuffer;

    if( rcMbBestData.isIntra() )
    {
      cYuvMbBuffer.setAllSamplesToZero();
    }
    else
    {
      cYuvMbBuffer.loadLuma   ( rcMbBestData );
      cYuvMbBuffer.loadChroma ( rcMbBestData );
      cYuvMbBuffer.subtract   ( rcMbBestData.getTempYuvMbBuffer() );
    }

    ANOK( pcRecSubband->getFullPelYuvBuffer()->loadBuffer( &cYuvMbBuffer ) );
  }

  rcMbBestData.clip();
  if( m_pcIntPicBuffer )
    m_pcIntPicBuffer->loadBuffer( &rcMbBestData );
}





ErrVal
MbEncoder::xEncodeChromaIntra( IntMbTempData& rcMbTempData,
                               UInt&          ruiExtCbp,
                               UInt&          ruiBits,
                               Bool           bLowComplexity )   // JVT-V079
{
  Double fBestRd = DOUBLE_MAX;        // moved from below
  Bool avcRewriteFlag = rcMbTempData.getSH().getTCoeffLevelPredictionFlag();

  bLowComplexity = false;
  
  // do it once
  if (avcRewriteFlag ||  ( m_pcIntMbBestIntraChroma == NULL))
  {
    m_pcIntMbBestIntraChroma = &m_acIntMbTempData[4];

	  if( avcRewriteFlag && rcMbTempData.isIntraBL() )
		  rcMbTempData.setChromaPredMode( rcMbTempData.getMbDataAccess().getMbDataAccessBase()->getMbData().getChromaPredMode() );

    for( UInt uiPM = 0; uiPM < 4; uiPM++ )
    {
      Bool bValid = false;
      RNOK( m_pcIntraPrediction->predictSChromaBlock( rcMbTempData, uiPM, bValid ) );
      rcMbTempData.getTempYuvMbBuffer().loadChroma( rcMbTempData );

      if( avcRewriteFlag && rcMbTempData.isIntraBL() )
        bValid = ( rcMbTempData.getChromaPredMode() == uiPM ) ? bValid : false;

      if( bValid )
      {
        UInt uiBits = 0;
        UInt uiCbp  = 0;
        RNOK( xEncodeChromaTexture( rcMbTempData, uiCbp, uiBits, bLowComplexity ) );
        UInt uiDist = rcMbTempData.distU() + rcMbTempData.distV();
        rcMbTempData.setChromaPredMode( uiPM );

        if( !avcRewriteFlag || !rcMbTempData.isIntraBL() )
        {
          BitCounter::init();
          RNOK( UvlcWriter::intraPredModeChroma( rcMbTempData ) );
          uiBits += BitCounter::getNumberOfWrittenBits();
        }

        Double fNewRd = m_pcRateDistortionIf->getCost( uiBits, uiDist );

        if( fNewRd < fBestRd )
        {
          m_pcIntMbBestIntraChroma->loadChromaData( rcMbTempData );
          m_pcIntMbBestIntraChroma->cbp()    = uiCbp;
          m_pcIntMbBestIntraChroma->bits()   = uiBits;
          fBestRd = fNewRd;
        }
      }
    }
  }

  if (avcRewriteFlag && (fBestRd == DOUBLE_MAX))        // no valid mode is found
  {
    ruiBits = MSYS_UINT_MAX;
    return Err::m_nOK;
  }

  rcMbTempData.loadChromaData( *m_pcIntMbBestIntraChroma );
  ruiExtCbp += m_pcIntMbBestIntraChroma->cbp();
  ruiBits   += m_pcIntMbBestIntraChroma->bits();

  return Err::m_nOK;
}





ErrVal
MbEncoder::xEncodeChromaTexture( IntMbTempData& rcMbTempData,
                                 UInt&          ruiExtCbp,
                                 UInt&          ruiBits,
                                 Bool           bLowComplexity )   // JVT-V079
{
  TCoeff aiCoeff[128];

  bLowComplexity = false;
  
  XPel* pucCb   = rcMbTempData.getMbCbAddr();
  XPel* pucCr   = rcMbTempData.getMbCrAddr();
  Int   iStride = rcMbTempData.getCStride();

  Int           iScalMatCb  = ( rcMbTempData.isIntra() ? 1 : 4 );
  Int           iScalMatCr  = ( rcMbTempData.isIntra() ? 2 : 5 );
  const UChar*  pucScaleCb  = ( rcMbTempData.getSH().isScalingMatrixPresent(iScalMatCb) ? rcMbTempData.getSH().getScalingMatrix(iScalMatCb) : NULL );
  const UChar*  pucScaleCr  = ( rcMbTempData.getSH().isScalingMatrixPresent(iScalMatCr) ? rcMbTempData.getSH().getScalingMatrix(iScalMatCr) : NULL );

  UInt uiDcCb = 0;
  UInt uiAcCb = 0;
  UInt uiDcCr = 0;
  UInt uiAcCr = 0;

  Bool coeffResidualPredFlag = rcMbTempData.getMbDataAccess().getCoeffResidualPredFlag();
  if ( coeffResidualPredFlag &&  rcMbTempData.getResidualPredFlag( PART_16x16 ) )
  {
	  TCoeff* piCoeffBase = rcMbTempData.getMbDataAccess().getMbDataAccessBase()->getMbTCoeffs().get( CIdx(0)) ; // U
    RNOK( m_pcTransform->transformChromaBlocksCGS( m_pcIntOrgMbPelData->getMbCbAddr(), pucCb, CIdx(0), iStride, rcMbTempData.get( CIdx(0) ), aiCoeff+0x00, piCoeffBase, pucScaleCb, uiDcCb, uiAcCb ) );

    piCoeffBase = rcMbTempData.getMbDataAccess().getMbDataAccessBase()->getMbTCoeffs().get( CIdx(4) ); // V
    RNOK( m_pcTransform->transformChromaBlocksCGS( m_pcIntOrgMbPelData->getMbCrAddr(), pucCr, CIdx(4), iStride, rcMbTempData.get( CIdx(4) ), aiCoeff+0x40, piCoeffBase, pucScaleCr, uiDcCr, uiAcCr ) );
  }
  else
  {
    RNOK( m_pcTransform->transformChromaBlocks( m_pcIntOrgMbPelData->getMbCbAddr(), pucCb, CIdx(0), iStride, rcMbTempData.get( CIdx(0) ), aiCoeff+0x00, pucScaleCb, uiDcCb, uiAcCb ) );
    RNOK( m_pcTransform->transformChromaBlocks( m_pcIntOrgMbPelData->getMbCrAddr(), pucCr, CIdx(4), iStride, rcMbTempData.get( CIdx(4) ), aiCoeff+0x40, pucScaleCr, uiDcCr, uiAcCr ) );
  }
  
  UInt uiChromaCbp = 0;
  UInt uiDcBits = 0;
  UInt uiDcAbs = uiDcCb + uiDcCr;

  Bool avcRewriteFlag = rcMbTempData.getSH().getTCoeffLevelPredictionFlag();

  if (avcRewriteFlag && ( rcMbTempData.isIntraBL() || ( !rcMbTempData.isIntra() && rcMbTempData.getResidualPredFlag( PART_16x16 )) ) )
  {
	  UInt uiBaseQp = rcMbTempData.getMbDataAccess().getMbDataAccessBase()->getSH().getChromaQp( rcMbTempData.getMbDataAccess().getMbDataAccessBase()->getMbData().getQp() );

	  RNOK( m_pcTransform->predictChromaBlocks( rcMbTempData.get( CIdx(0) ),
		rcMbTempData.getMbDataAccess().getMbDataAccessBase()->getMbTCoeffs().get( CIdx(0) ),
		uiBaseQp, uiDcCb, uiAcCb ) );

	  RNOK( m_pcTransform->predictChromaBlocks( rcMbTempData.get( CIdx(4) ),
		rcMbTempData.getMbDataAccess().getMbDataAccessBase()->getMbTCoeffs().get( CIdx(4) ),
		uiBaseQp, uiDcCr, uiAcCr ) );

	  uiDcAbs = uiDcCb + uiDcCr;
  }

  if( uiDcAbs )
  {
    BitCounter::init();
    MbCoder::xScanChromaDc( rcMbTempData, rcMbTempData );
    uiDcBits = BitCounter::getNumberOfWrittenBits();
    uiChromaCbp = 1;
  }

  UInt uiCbp1 = uiChromaCbp;
  UInt uiCbp2 = uiChromaCbp;
  UInt uiAcBits1 = 4;
  UInt uiAcBits2 = 4;

  if( uiAcCr )
  {
    uiCbp2 = 2;
    xSetCoeffCost( 0 );
    BitCounter::init();
    MbCoder::xScanChromaAcV( rcMbTempData, rcMbTempData );
    uiAcBits2 = BitCounter::getNumberOfWrittenBits();

    if( 4 > xGetCoeffCost() )
    {
      if( uiAcBits2 > 4 )
      {
        for( CIdx cCIdx(4); cCIdx.isLegal(8); cCIdx++)
        {
          rcMbTempData.clearAcBlk( cCIdx );
        }
        uiAcBits2 = 4;
       
        if( coeffResidualPredFlag &&  m_pcIntMbTempData->getResidualPredFlag( PART_16x16 ) )
        {          
          // add the base layer coeff back
          TCoeff* piCoeff = &aiCoeff[0x40];
          TCoeff* ptCoeff = rcMbTempData.getMbDataAccess().getMbTCoeffs().get( CIdx(4) );
          TCoeff* piCoeffBase = rcMbTempData.getMbDataAccess().getMbDataAccessBase()->getMbTCoeffs().get( CIdx(4) );
          for( UInt x=0x00; x<0x40; x+=0x10 )
            for( UInt n=1; n<16; n++ )
            {
              piCoeff[x+n] = piCoeffBase[x+n].getLevel();
              ptCoeff[x+n].setLevel(piCoeffBase[x+n].getLevel()); // store the coefficients in TCoeff.level field
            }
        }
        else if (avcRewriteFlag && ( rcMbTempData.isIntraBL() || ( !rcMbTempData.isIntra() && rcMbTempData.getResidualPredFlag( PART_16x16 )) ) )      
        {
          // Predict AC Scaled Coefficients
          UInt uiBaseQp = rcMbTempData.getMbDataAccess().getMbDataAccessBase()->getSH().getChromaQp( rcMbTempData.getMbDataAccess().getMbDataAccessBase()->getMbData().getQp() );

          m_pcTransform->predictScaledACCoeffs( &aiCoeff[0x40],
            rcMbTempData.getMbDataAccess().getMbDataAccessBase()->getMbTCoeffs().get( CIdx(4) ),
            uiBaseQp  );
       
          TCoeff* ptCoeff = rcMbTempData.getMbDataAccess().getMbTCoeffs().get( CIdx(4) );
          for( UInt x=0x00; x<0x40; x+=0x10 )
          {
            for( UInt n=1; n<16; n++ )
            {              
              ptCoeff[x+n].setLevel(aiCoeff[0x40+x+n].getCoeff()); // store the coefficients in TCoeff.level field              
            }
          }
        }
        else
        {
          ::memset( &aiCoeff[0x41], 0x00, 15*sizeof(TCoeff) );
          ::memset( &aiCoeff[0x51], 0x00, 15*sizeof(TCoeff) );
          ::memset( &aiCoeff[0x61], 0x00, 15*sizeof(TCoeff) );
          ::memset( &aiCoeff[0x71], 0x00, 15*sizeof(TCoeff) );
        }
      }

      uiCbp2 = uiChromaCbp;
    }
  }


  if( uiAcCb )
  {
    uiCbp1 = 2;
    xSetCoeffCost( 0 );
    BitCounter::init();
    MbCoder::xScanChromaAcU( rcMbTempData, rcMbTempData );
    uiAcBits1 = BitCounter::getNumberOfWrittenBits();

    if( 4 > xGetCoeffCost() )
    {
      if( uiAcBits1 > 4 )
      {
        for( CIdx cCIdx(0); cCIdx.isLegal(4); cCIdx++)
        {
          rcMbTempData.clearAcBlk( cCIdx );
        }
        uiAcBits1 = 4;

        if( coeffResidualPredFlag &&  m_pcIntMbTempData->getResidualPredFlag( PART_16x16 ) )
        {          
          // add the base layer coeff back and also store the dequantized coeffs
          TCoeff* piCoeff = &aiCoeff[0x00];
          TCoeff* ptCoeff = rcMbTempData.getMbDataAccess().getMbTCoeffs().get( CIdx(0) );
          TCoeff* piCoeffBase = rcMbTempData.getMbDataAccess().getMbDataAccessBase()->getMbTCoeffs().get( CIdx(0) );
          for( UInt x=0x00; x<0x40; x+=0x10 )
            for( UInt n=1; n<16; n++ )
            {
              piCoeff[x+n] = piCoeffBase[x+n].getLevel();
              ptCoeff[x+n].setLevel(piCoeffBase[x+n].getLevel()); // store the coefficients in TCoeff.level field
            }
        }
        else if ((avcRewriteFlag) && ( rcMbTempData.isIntraBL() || ( !rcMbTempData.isIntra() && rcMbTempData.getResidualPredFlag( PART_16x16 )) ) )
        {
          // Predict AC Scaled Coefficients
          UInt uiBaseQp = rcMbTempData.getMbDataAccess().getMbDataAccessBase()->getSH().getChromaQp( rcMbTempData.getMbDataAccess().getMbDataAccessBase()->getMbData().getQp() );

          m_pcTransform->predictScaledACCoeffs( &aiCoeff[0x00],
            rcMbTempData.getMbDataAccess().getMbDataAccessBase()->getMbTCoeffs().get( CIdx(0) ),
            uiBaseQp  );

          TCoeff* ptCoeff = rcMbTempData.getMbDataAccess().getMbTCoeffs().get( CIdx(0) );
          for( UInt x=0x00; x<0x40; x+=0x10 )
            for( UInt n=1; n<16; n++ )
            {              
              ptCoeff[x+n].setLevel(aiCoeff[x+n].getCoeff()); // store the coefficients in TCoeff.level field              
            }
        }
        else
        {
          ::memset( &aiCoeff[0x01], 0x00, 15*sizeof(TCoeff) );
          ::memset( &aiCoeff[0x11], 0x00, 15*sizeof(TCoeff) );
          ::memset( &aiCoeff[0x21], 0x00, 15*sizeof(TCoeff) );
          ::memset( &aiCoeff[0x31], 0x00, 15*sizeof(TCoeff) );
        }
      }
      uiCbp1 = uiChromaCbp;
    }
  }

  if( (uiAcBits1 + uiAcBits2) > 8)
  {
    ruiBits += uiAcBits1 + uiAcBits2;
    uiChromaCbp = max( uiCbp1, uiCbp2 );
  }

  if ((coeffResidualPredFlag&&rcMbTempData.getResidualPredFlag( PART_16x16 )) || avcRewriteFlag || uiCbp1)
  {
    Int iQpScale = pucScaleCb ? pucScaleCb[0] : 16 ;
    m_pcTransform->invTransformChromaDc( &aiCoeff[0x00], iQpScale );
    RNOK( m_pcTransform->invTransformChromaBlocks( pucCb, iStride, &aiCoeff[0x00] ) );
  }

  if ((coeffResidualPredFlag&&rcMbTempData.getResidualPredFlag( PART_16x16 )) || avcRewriteFlag || uiCbp2)
  {
    Int iQpScale = pucScaleCr ? pucScaleCr[0] : 16 ;
    m_pcTransform->invTransformChromaDc( &aiCoeff[0x40], iQpScale );
    RNOK( m_pcTransform->invTransformChromaBlocks( pucCr, iStride, &aiCoeff[0x40] ) );
  }

  if ( bLowComplexity )
  {
    m_pcIntMbTempData->distU() = m_pcXDistortion->get8x8Cr( pucCr, iStride, DF_SAD );
    m_pcIntMbTempData->distV() = m_pcXDistortion->get8x8Cb( pucCb, iStride, DF_SAD );
  }
  else
  {
    m_pcIntMbTempData->distU() = m_pcXDistortion->get8x8Cr( pucCr, iStride );
    m_pcIntMbTempData->distV() = m_pcXDistortion->get8x8Cb( pucCb, iStride );
  }

  if( uiChromaCbp )
  {
    ruiBits += uiDcBits;
  }

  ruiExtCbp |= uiChromaCbp << 16;

  return Err::m_nOK;
}






ErrVal
MbEncoder::xSetRdCostIntraMb( IntMbTempData&  rcMbTempData,
                              UInt            uiCoeffBits,
                              Bool            bSlice,
                              Bool            bBLSkip )
{
  YuvMbBuffer& rcYuvMbBuffer     = rcMbTempData;
  MbDataAccess&   rcMbDataAccess    = rcMbTempData.getMbDataAccess();
  UInt            uiMbDist          = 0;
  UInt            uiMbBits          = 0;

  //===== get distortion =====
  uiMbDist  += rcMbTempData.distY() = m_pcXDistortion->getLum16x16 ( rcYuvMbBuffer.getMbLumAddr(), rcYuvMbBuffer.getLStride() );
  uiMbDist  += rcMbTempData.distU() = m_pcXDistortion->get8x8Cb    ( rcYuvMbBuffer.getMbCbAddr (), rcYuvMbBuffer.getCStride() );
  uiMbDist  += rcMbTempData.distV() = m_pcXDistortion->get8x8Cr    ( rcYuvMbBuffer.getMbCrAddr (), rcYuvMbBuffer.getCStride() );

  //===== get rate =====
  SliceType eRealSliceType  = rcMbDataAccess.getSH().getSliceType();
  if( ! bSlice )
  {
    rcMbDataAccess.getSH().setSliceType( P_SLICE );
  }

  RNOK(   BitCounter::init() );

  MbMode  eMbMode = rcMbDataAccess.getMbData().getMbMode();
  if( eMbMode == INTRA_BL )
  {
    rcMbDataAccess.getMbData().setMbMode( INTRA_4X4 );
  }

  if( ! bBLSkip )
  {
    RNOK( MbCoder::m_pcMbSymbolWriteIf->mbMode    ( rcMbDataAccess/*, false*/ ) );
  }

  rcMbDataAccess.getMbData().setMbMode( eMbMode );

  rcMbDataAccess.getSH().setSliceType( eRealSliceType );

  RNOK(   MbCoder::m_pcMbSymbolWriteIf->cbp       ( rcMbDataAccess ) );
  uiMbBits  += BitCounter::getNumberOfWrittenBits();
  uiMbBits  += uiCoeffBits + 1; // 1 for chroma pred mode

  rcMbTempData.bits() = uiMbBits;

  //===== set rd-cost =====
  rcMbTempData.rdCost() = m_pcRateDistortionIf->getCost( uiMbBits, uiMbDist );

  return Err::m_nOK;
}




Bool
MbEncoder::xCheckUpdate( YuvMbBuffer&   rcPredBuffer,
                         YuvMbBuffer&   rcOrigBuffer,
                         LumaIdx           cIdx,
                         Int               iXSize,
                         Int               iYSize )
{
  XPel* pPred     = rcPredBuffer.getYBlk( cIdx );
  XPel* pOrig     = rcOrigBuffer.getYBlk( cIdx );
  Int   iStride   = rcPredBuffer.getLStride();
  Int   x, y;

  Int   iSumPred2 = 0;
  Int   iSumOrig2 = 0;
  Int   iSumRes2  = 0;
  Int   iSumPred1 = 0;
  Int   iSumOrig1 = 0;
  Int   iSumRes1  = 0;

  for( y = 0; y < iYSize; y++ )
  {
    for( x = 0; x < iXSize; x++ )
    {
      Int iRes   = pOrig[x] - pPred[x];
      iSumPred2 += pPred[x] * pPred[x];
      iSumOrig2 += pOrig[x] * pOrig[x];
      iSumRes2  += iRes     * iRes;
      iSumPred1 += pPred[x];
      iSumOrig1 += pOrig[x];
      iSumRes1  += iRes;
    }
    pPred += iStride;
    pOrig += iStride;
  }

  Double  dSamples  = iXSize * iYSize;
  Double  dVarPred  = (Double)iSumPred2/dSamples - ( (Double)iSumPred1/dSamples ) * ( (Double)iSumPred1/dSamples );
  Double  dVarOrig  = (Double)iSumOrig2/dSamples - ( (Double)iSumOrig1/dSamples ) * ( (Double)iSumOrig1/dSamples );
  Double  dVarRes   = (Double)iSumRes2 /dSamples - ( (Double)iSumRes1 /dSamples ) * ( (Double)iSumRes1 /dSamples );

  Bool   bUpdate = ( dVarRes < 0.5*dVarOrig && dVarRes < 0.5*dVarPred );
  return bUpdate;
}


ErrVal
MbEncoder::xSetRdCostInterMb( IntMbTempData&  rcMbTempData,
                              MbDataAccess*   pcMbDataAccessBase,
                              RefFrameList&   rcRefFrameList0,
                              RefFrameList&   rcRefFrameList1,
                              Bool            bBLSkip,
                              UInt            uiAdditionalBits,
                              Bool            bSkipMCPrediction,
                              Frame*       pcBaseLayerRec,
                              Bool            bLowComplexity )   // JVT-V079
{
  YuvMbBuffer& rcYuvMbBuffer     = rcMbTempData;
  YuvMbBuffer& rcTempYuvMbBuffer = rcMbTempData  .getTempYuvMbBuffer ();
  MbDataAccess&   rcMbDataAccess    = rcMbTempData  .getMbDataAccess    ();
  MbData&         rcMbData          = rcMbDataAccess.getMbData          ();
  UInt            uiMbDist          = 0;
  UInt            uiMbBits          = 0;
  MbMode          eMbMode           = rcMbData.getMbMode();
  Bool            b8x8Mode          = ( eMbMode == MODE_8x8 || eMbMode == MODE_8x8ref0 );
  UInt            uiFwdBwd          = 0;

  if ( bLowComplexity && (rcMbTempData.rdCost() != DOUBLE_MAX ) )
  { 
    return Err::m_nOK;
  }
  
  Bool coeffResidualPredFlag = rcMbDataAccess.getCoeffResidualPredFlag();
  Bool avcRewriteFlag = rcMbDataAccess.getSH().getTCoeffLevelPredictionFlag();
  
  // enforce to use the same transform size as base
  if( pcMbDataAccessBase && pcMbDataAccessBase->getMbData().isTransformSize8x8()
    && (avcRewriteFlag || coeffResidualPredFlag)  && rcMbTempData.getResidualPredFlag( PART_16x16 ) )
  {
    // 4x4-trafo not allowed here
    // Encoders can consider 8x8 transform though...
    rcMbTempData.rdCost() = DOUBLE_MAX;
    return Err::m_nOK;
  }

  //===== set forward / backward indication =====
  if( rcMbDataAccess.getSH().isBSlice() )
  {
    for( Int n = 3; n >= 0; n--)
    {
      uiFwdBwd <<= 4;
      uiFwdBwd += (0 < rcMbDataAccess.getMbMotionData( LIST_0 ).getRefIdx( Par8x8(n) )) ? 1:0;
      uiFwdBwd += (0 < rcMbDataAccess.getMbMotionData( LIST_1 ).getRefIdx( Par8x8(n) )) ? 2:0;
    }
  }
  if( rcMbDataAccess.getSH().isPSlice() )
  {
    for( Int n = 3; n >= 0; n--)
    {
      uiFwdBwd <<= 4;
      uiFwdBwd +=  (0 < rcMbDataAccess.getMbMotionData( LIST_0 ).getRefIdx( Par8x8(n) )) ? 1:0;
    }
  }
  rcMbTempData.setFwdBwd( uiFwdBwd );

  if( ! bSkipMCPrediction )
  {
    //===== get prediction and copy to temp buffer =====
    if( b8x8Mode )
    {
      for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
      {
        RNOK( m_pcMotionEstimation->compensateSubMb( c8x8Idx, rcMbDataAccess, rcRefFrameList0, rcRefFrameList1,
                                                     &rcYuvMbBuffer, false, false ) );
      }
    }
    else
    {
      RNOK  ( m_pcMotionEstimation->compensateMb  ( rcMbDataAccess, rcRefFrameList0, rcRefFrameList1,
                                                    &rcYuvMbBuffer, false) );
    }
    if(pcBaseLayerRec)
    {
      RNOK(m_pcMotionEstimation->compensateMbBLSkipIntra(rcMbDataAccess,  &rcYuvMbBuffer, pcBaseLayerRec));
    }
  }
  rcTempYuvMbBuffer.loadLuma  ( rcYuvMbBuffer );
  rcTempYuvMbBuffer.loadChroma( rcYuvMbBuffer );

  if ( bLowComplexity ) 
  { 
    rcMbTempData.rdCost() = m_pcXDistortion->getLum16x16 ( rcYuvMbBuffer.getMbLumAddr(), rcYuvMbBuffer.getLStride(), DF_SAD );
    return Err::m_nOK;
  }

  //===== encode residual and get rate for coefficients =====
  UInt  uiCoeffCost = 0;
  UInt  uiExtCbp    = 0;
  Bool  bSkipMode   = ( eMbMode == MODE_SKIP && rcMbDataAccess.getSH().isPSlice() );

  if( ! bSkipMode )
  {
    //--- LUMA ---
    for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
    {
      xSetCoeffCost( 0 );
      UInt  uiBits = 0;
      UInt  uiCbp  = 0;

      for( S4x4Idx cIdx( c8x8Idx ); cIdx.isLegal( c8x8Idx ); cIdx++ )
      {
        RNOK( xEncode4x4InterBlock( rcMbTempData, cIdx, uiBits, uiCbp ) );
      }
      if( uiCbp )
      {
        if( xGetCoeffCost() <= 4 )
        {
          rcYuvMbBuffer.loadLuma( rcTempYuvMbBuffer, c8x8Idx );
          rcMbTempData.clearLumaLevels8x8( c8x8Idx ); 

          if( coeffResidualPredFlag && rcMbTempData.getResidualPredFlag( PART_16x16 ) )
          {            
            reCalcBlock8x8(rcMbTempData, c8x8Idx, 1);
          }
          else if ( avcRewriteFlag && rcMbTempData.getResidualPredFlag( PART_16x16 ) )
          {
            reCalcBlock8x8Rewrite(rcMbTempData, c8x8Idx, 1);
          }              
        }
        else
        {
          uiCoeffCost += xGetCoeffCost();
          uiExtCbp    += uiCbp;
          uiMbBits    += uiBits;
        }
      }
    }
    if( uiExtCbp && uiCoeffCost <= 5 )
    {
      rcYuvMbBuffer.loadLuma( rcTempYuvMbBuffer );
      uiExtCbp  = 0;
      uiMbBits  = 0;

      rcMbTempData.clearLumaLevels();
      if( coeffResidualPredFlag && rcMbTempData.getResidualPredFlag( PART_16x16 ) )
      {
        for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
        {
          reCalcBlock8x8(rcMbTempData, c8x8Idx, 1);
        }
      }
      else if( avcRewriteFlag && rcMbTempData.getResidualPredFlag( PART_16x16 ) )
      {
        for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
        {
          reCalcBlock8x8Rewrite(rcMbTempData, c8x8Idx, 1);
        }
      }
    }

    //--- CHROMA ---
    RNOK( xEncodeChromaTexture( rcMbTempData, uiExtCbp, uiMbBits ) );
  }
  rcMbTempData.cbp() = xCalcMbCbp( uiExtCbp );
 
  //===== get distortion =====
  if(!pcMbDataAccessBase||pcMbDataAccessBase->getMbData().m_bRPSafe||rcMbTempData.getResidualPredFlag(PART_16x16)==false)
    uiMbDist  += m_pcXDistortion->getLum16x16 ( rcYuvMbBuffer.getMbLumAddr(), rcYuvMbBuffer.getLStride() );
  else
    uiMbDist  += m_pcXDistortion->getLum16x16RP ( rcYuvMbBuffer.getMbLumAddr(), rcYuvMbBuffer.getLStride() );
  uiMbDist  += m_pcXDistortion->get8x8Cb    ( rcYuvMbBuffer.getMbCbAddr (), rcYuvMbBuffer.getCStride() );
  uiMbDist  += m_pcXDistortion->get8x8Cr    ( rcYuvMbBuffer.getMbCrAddr (), rcYuvMbBuffer.getCStride() );

  if( rcMbDataAccess.getSH().getSliceType() == B_SLICE &&
      rcMbDataAccess.getMbData().getMbMode() == MODE_SKIP &&
      uiExtCbp == 0 &&
      !rcMbDataAccess.getMbData().getResidualPredFlag( PART_16x16 ) )
  {
    bSkipMode = true;
    if( rcMbDataAccess.getSH().getNoInterLayerPredFlag() )
    {
      uiMbBits += 1;
    }
  }

  //===== get rate =====
  if( ! bSkipMode )
  {
    RNOK(   BitCounter::init() );

    if( ! bBLSkip )
    {
      RNOK(   MbCoder::m_pcMbSymbolWriteIf->mbMode    ( rcMbDataAccess/*, false*/ ) );
      if( b8x8Mode )
      {
        RNOK( MbCoder::m_pcMbSymbolWriteIf->blockModes( rcMbDataAccess ) );
      }
    }

    RNOK(     MbCoder::m_pcMbSymbolWriteIf->cbp       ( rcMbDataAccess ) );

    if( rcRefFrameList0.getActive() && !bBLSkip )
    {
      RNOK(   MbCoder::xWriteMotionPredFlags          ( rcMbDataAccess, eMbMode, LIST_0 ) );
      RNOK(   MbCoder::xWriteReferenceFrames          ( rcMbDataAccess, eMbMode, LIST_0 ) );
      RNOK(   MbCoder::xWriteMotionVectors            ( rcMbDataAccess, eMbMode, LIST_0 ) );
    }

    if( rcRefFrameList1.getActive() && !bBLSkip )
    {
      RNOK(   MbCoder::xWriteMotionPredFlags          ( rcMbDataAccess, eMbMode, LIST_1 ) );
      RNOK(   MbCoder::xWriteReferenceFrames          ( rcMbDataAccess, eMbMode, LIST_1 ) );
      RNOK(   MbCoder::xWriteMotionVectors            ( rcMbDataAccess, eMbMode, LIST_1 ) );
    }

    uiMbBits  += BitCounter::getNumberOfWrittenBits();
  }

  //===== set rd-cost =====
  rcMbTempData.rdCost() = m_pcRateDistortionIf->getCost( uiMbBits+uiAdditionalBits, uiMbDist );

  return Err::m_nOK;
}




ErrVal
MbEncoder::xScale4x4Block( TCoeff*            piCoeff,
                           const UChar*       pucScale,
                           UInt               uiStart,
                           const QpParameter& rcQP )
{
  if( pucScale )
  {
    Int iAdd = ( rcQP.per() <= 3 ? ( 1 << ( 3 - rcQP.per() ) ) : 0 );

    for( UInt ui = uiStart; ui < 16; ui++ )
    {
      piCoeff[ui] = ( ( piCoeff[ui] * g_aaiDequantCoef[rcQP.rem()][ui] * pucScale[ui] + iAdd ) << rcQP.per() ) >> 4;
    }
  }
  else
  {
    for( UInt ui = uiStart; ui < 16; ui++ )
    {
      piCoeff[ui] *= ( g_aaiDequantCoef[rcQP.rem()][ui] << rcQP.per() );
    }
  }

  return Err::m_nOK;
}


ErrVal
MbEncoder::xScale8x8Block( TCoeff*            piCoeff,
                           const UChar*       pucScale,
                           const QpParameter& rcQP )
{
  Int iAdd = ( rcQP.per() <= 5 ? ( 1 << ( 5 - rcQP.per() ) ) : 0 );

  if( pucScale )
  {
    for( UInt ui = 0; ui < 64; ui++ )
    {
      piCoeff[ui] = ( ( piCoeff[ui] * g_aaiDequantCoef64[rcQP.rem()][ui] * pucScale[ui] + iAdd ) << rcQP.per() ) >> 6;
    }
  }
  else
  {
    for( UInt ui = 0; ui < 64; ui++ )
    {
      piCoeff[ui] = ( ( piCoeff[ui] * g_aaiDequantCoef64[rcQP.rem()][ui] * 16 + iAdd ) << rcQP.per() ) >> 6;
    }
  }

  return Err::m_nOK;
}


ErrVal
MbEncoder::xScaleTCoeffs( MbDataAccess& rcMbDataAccess, MbTransformCoeffs& rcTCoeffs )
{
  const Int aaiDequantDcCoef[6] = {  10, 11, 13, 14, 16, 18 };
  Quantizer cQuantizer;
  cQuantizer.setQp( rcMbDataAccess, false );

  const QpParameter&  cLQp      = cQuantizer.getLumaQp  ();
  const QpParameter&  cCQp      = cQuantizer.getChromaQp();
  Bool                bIntra    = rcMbDataAccess.getMbData().isIntra();
  Bool                b8x8      = rcMbDataAccess.getMbData().isTransformSize8x8();
  Bool                b16x16    = rcMbDataAccess.getMbData().isIntra16x16();
  UInt                uiYScalId = ( bIntra ? ( b8x8 && !b16x16 ? 6 : 0 ) : ( b8x8 ? 7 : 3 ) );
  UInt                uiUScalId = ( bIntra ? 1 : 4 );
  UInt                uiVScalId = ( bIntra ? 2 : 5 );
  const UChar*        pucScaleY = rcMbDataAccess.getSH().getScalingMatrix( uiYScalId );
  const UChar*        pucScaleU = rcMbDataAccess.getSH().getScalingMatrix( uiUScalId );
  const UChar*        pucScaleV = rcMbDataAccess.getSH().getScalingMatrix( uiVScalId );

  //===== copy all coefficients =====
  rcTCoeffs.copyFrom( rcMbDataAccess.getMbTCoeffs() );

  //===== luma =====
  if( b16x16 )
  {
    //===== INTRA_16x16 =====
    Int iScaleY  = aaiDequantDcCoef[cLQp.rem()] << cLQp.per();
    if( pucScaleY )
    {
      iScaleY  *= pucScaleY[0];
      iScaleY >>= 4;
    }
    for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
    {
      RNOK( xScale4x4Block( rcTCoeffs.get( cIdx ), pucScaleY, 1, cLQp ) );
    }
  }
  else if( b8x8 )
  {
    //===== 8x8 BLOCKS =====
    for( B8x8Idx cIdx; cIdx.isLegal(); cIdx++ )
    {
      RNOK( xScale8x8Block( rcTCoeffs.get8x8( cIdx ), pucScaleY, cLQp ) );
    }
  }
  else
  {
    //===== 4x4 BLOCKS =====
    for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
    {
      RNOK( xScale4x4Block( rcTCoeffs.get( cIdx ), pucScaleY, 0, cLQp ) );
    }
  }

  //===== chroma =====
  Int iScaleU  = g_aaiDequantCoef[cCQp.rem()][0] << cCQp.per();
  Int iScaleV  = g_aaiDequantCoef[cCQp.rem()][0] << cCQp.per();
  /* HS: old scaling modified:
     (It did not work for scaling matrices, when QpPer became less than 5 in an FGS enhancement) */
  for( CIdx cIdx; cIdx.isLegal(); cIdx++ )
  {
    RNOK( xScale4x4Block( rcTCoeffs.get( cIdx ), ( cIdx.plane() ? pucScaleV : pucScaleU ), 1, cCQp ) );
  }
  UInt    uiDCIdx;
  TCoeff* piCoeff = rcTCoeffs.get( CIdx(0) );
  for( uiDCIdx = 0; uiDCIdx < 4; uiDCIdx++ )
  {
    piCoeff[16*uiDCIdx] *= iScaleU;
  }
  piCoeff = rcTCoeffs.get( CIdx(4) );
  for( uiDCIdx = 0; uiDCIdx < 4; uiDCIdx++ )
  {
    piCoeff[16*uiDCIdx] *= iScaleV;
  }


  return Err::m_nOK;
}



ErrVal
MbEncoder::xSetRdCost8x8InterMb ( IntMbTempData&  rcMbTempData,
                                  MbDataAccess*   pcMbDataAccessBaseMotion,
                                  RefFrameList&   rcRefFrameList0,
                                  RefFrameList&   rcRefFrameList1,
                                  Bool            bBLSkip,
                                  UInt            uiAdditionalBits,
                                  Bool            bSkipMCPrediction,
                                  Frame*       pcBaseLayerRec
                                  )
{
  YuvMbBuffer& rcYuvMbBuffer     = rcMbTempData;
  YuvMbBuffer& rcTempYuvMbBuffer = rcMbTempData.getTempYuvMbBuffer();
  MbDataAccess&   rcMbDataAccess    = rcMbTempData.getMbDataAccess();
  MbData&         rcMbData          = rcMbDataAccess.getMbData();
  UInt            uiMbDist          = 0;
  UInt            uiMbBits          = 0;
  MbMode          eMbMode           = rcMbData.getMbMode();
  Bool            b8x8Mode          = ( eMbMode == MODE_8x8 || eMbMode == MODE_8x8ref0 );
  UInt            uiFwdBwd          = 0;

  Bool avcRewriteFlag = rcMbDataAccess.getSH().getTCoeffLevelPredictionFlag();
  Bool coeffResidualPredFlag = rcMbDataAccess.getCoeffResidualPredFlag();

  if( pcMbDataAccessBaseMotion && !pcMbDataAccessBaseMotion->getMbData().isTransformSize8x8()
    && (avcRewriteFlag ||coeffResidualPredFlag) && rcMbDataAccess.getMbData().getResidualPredFlag(PART_16x16))
  {
    //8x8-trafo not allowed here...
    rcMbTempData.rdCost() = DOUBLE_MAX;
    return Err::m_nOK;
  }

  //=== check ===
  ROT( eMbMode == MODE_SKIP && rcMbDataAccess.getSH().isPSlice() ); // not for skip mode
  ROT((eMbMode == MODE_8x8 || eMbMode == MODE_8x8ref0) && rcMbData.getBlkMode( B_8x8_0 ) != BLK_SKIP && rcMbData.getBlkMode( B_8x8_0 ) != BLK_8x8 );
  ROT((eMbMode == MODE_8x8 || eMbMode == MODE_8x8ref0) && rcMbData.getBlkMode( B_8x8_1 ) != BLK_SKIP && rcMbData.getBlkMode( B_8x8_1 ) != BLK_8x8 );
  ROT((eMbMode == MODE_8x8 || eMbMode == MODE_8x8ref0) && rcMbData.getBlkMode( B_8x8_2 ) != BLK_SKIP && rcMbData.getBlkMode( B_8x8_2 ) != BLK_8x8 );
  ROT((eMbMode == MODE_8x8 || eMbMode == MODE_8x8ref0) && rcMbData.getBlkMode( B_8x8_3 ) != BLK_SKIP && rcMbData.getBlkMode( B_8x8_3 ) != BLK_8x8 );



  //===== set forward / backward indication =====
  if( rcMbDataAccess.getSH().isBSlice() )
  {
    for( Int n = 3; n >= 0; n--)
    {
      uiFwdBwd <<= 4;
      uiFwdBwd += (0 < rcMbDataAccess.getMbMotionData( LIST_0 ).getRefIdx( Par8x8(n) )) ? 1:0;
      uiFwdBwd += (0 < rcMbDataAccess.getMbMotionData( LIST_1 ).getRefIdx( Par8x8(n) )) ? 2:0;
    }
  }
  if( rcMbDataAccess.getSH().isPSlice() )
  {
    for( Int n = 3; n >= 0; n--)
    {
      uiFwdBwd <<= 4;
      uiFwdBwd +=  (0 < rcMbDataAccess.getMbMotionData( LIST_0 ).getRefIdx( Par8x8(n) )) ? 1:0;
    }
  }
  rcMbTempData.setFwdBwd( uiFwdBwd );


  if( ! bSkipMCPrediction )
  {
    //===== get prediction and copy to temp buffer =====
    if( b8x8Mode )
    {
      for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
      {
        RNOK( m_pcMotionEstimation->compensateSubMb( c8x8Idx, rcMbDataAccess, rcRefFrameList0, rcRefFrameList1,
                                                     &rcYuvMbBuffer, false, false ) );
      }
    }
    else
    {
      RNOK( m_pcMotionEstimation->compensateMb( rcMbDataAccess, rcRefFrameList0, rcRefFrameList1,
                                                &rcYuvMbBuffer, false ) );
    }
    if(pcBaseLayerRec)
    {
      RNOK(m_pcMotionEstimation->compensateMbBLSkipIntra(rcMbDataAccess, &rcYuvMbBuffer, pcBaseLayerRec));
    }
  }
  rcTempYuvMbBuffer.loadLuma  ( rcYuvMbBuffer );
  rcTempYuvMbBuffer.loadChroma( rcYuvMbBuffer );



  //===== encode residual and get rate for coefficients =====
  UInt  uiCoeffCost = 0;
  UInt  uiExtCbp    = 0;
  rcMbTempData.setTransformSize8x8( true );

  //--- LUMA ---
  for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
  {
    xSetCoeffCost( 0 );
    UInt  uiBits = 0;
    UInt  uiCbp  = 0;

    RNOK( xEncode8x8InterBlock( rcMbTempData, c8x8Idx, uiBits, uiCbp ) );
    if( uiCbp )
    {
      if( xGetCoeffCost() <= 4 )
      {
        rcYuvMbBuffer.loadLuma( rcTempYuvMbBuffer, c8x8Idx );
        rcMbTempData.clearLumaLevels8x8Block( c8x8Idx );

        if( coeffResidualPredFlag && rcMbTempData.getResidualPredFlag( PART_16x16 ) )
        {                  
          reCalcBlock8x8(rcMbTempData, c8x8Idx, 0);
        }
        else if( avcRewriteFlag && rcMbTempData.getResidualPredFlag( PART_16x16 ) )      
        {
          reCalcBlock8x8Rewrite(rcMbTempData, c8x8Idx, 0);          
        }
      }
      else
      {
        uiCoeffCost += xGetCoeffCost();
        uiExtCbp    += uiCbp;
        uiMbBits    += uiBits;
      }
    }
  }
  if( uiExtCbp && uiCoeffCost <= 5 )
  {
    rcYuvMbBuffer.loadLuma( rcTempYuvMbBuffer );
    uiExtCbp  = 0;
    uiMbBits  = 0;

    rcMbTempData.clearLumaLevels();

    if( coeffResidualPredFlag && rcMbTempData.getResidualPredFlag( PART_16x16 ) )
    {
      for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
      {
        reCalcBlock8x8(rcMbTempData, c8x8Idx, 0);
      }
    }
    else if ( avcRewriteFlag && rcMbTempData.getResidualPredFlag( PART_16x16 ) )
    {
      for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
      {
        reCalcBlock8x8Rewrite(rcMbTempData, c8x8Idx, 0);        
      }
    }
  }

  //--- CHROMA ---
  RNOK( xEncodeChromaTexture( rcMbTempData, uiExtCbp, uiMbBits ) );


  rcMbTempData.cbp() = xCalcMbCbp( uiExtCbp );

  //===== get distortion =====
  uiMbDist  += m_pcXDistortion->getLum16x16 ( rcYuvMbBuffer.getMbLumAddr(), rcYuvMbBuffer.getLStride() );
  uiMbDist  += m_pcXDistortion->get8x8Cb    ( rcYuvMbBuffer.getMbCbAddr (), rcYuvMbBuffer.getCStride() );
  uiMbDist  += m_pcXDistortion->get8x8Cr    ( rcYuvMbBuffer.getMbCrAddr (), rcYuvMbBuffer.getCStride() );

  //===== get rate =====
  Bool      bOneListOnly    = ! ( rcRefFrameList0.getActive() && rcRefFrameList1.getActive() );
  SliceType eRealSliceType  = rcMbDataAccess.getSH().getSliceType();
  if( bOneListOnly )
  {
    rcMbDataAccess.getSH().setSliceType( P_SLICE );
  }


  Bool  bMbPredFlag = false;
  RNOK(   BitCounter::init() );

  if( ! bMbPredFlag && !bBLSkip )
  {
    RNOK(   MbCoder::m_pcMbSymbolWriteIf->mbMode    ( rcMbDataAccess/*, false*/ ) );
    if( b8x8Mode )
    {
      RNOK( MbCoder::m_pcMbSymbolWriteIf->blockModes( rcMbDataAccess ) );
    }
  }

  rcMbDataAccess.getSH().setSliceType( eRealSliceType );

  RNOK(   MbCoder::m_pcMbSymbolWriteIf->cbp       ( rcMbDataAccess ) );

  if( rcRefFrameList0.getActive() && !bBLSkip )
  {
    RNOK( MbCoder::xWriteMotionPredFlags          ( rcMbDataAccess, eMbMode, LIST_0 ) );
    RNOK( MbCoder::xWriteReferenceFrames          ( rcMbDataAccess, eMbMode, LIST_0 ) );
    RNOK( MbCoder::xWriteMotionVectors            ( rcMbDataAccess, eMbMode, LIST_0 ) );
  }

  if( rcRefFrameList1.getActive() && !bBLSkip )
  {
    RNOK( MbCoder::xWriteMotionPredFlags          ( rcMbDataAccess, eMbMode, LIST_1 ) );
    RNOK( MbCoder::xWriteReferenceFrames          ( rcMbDataAccess, eMbMode, LIST_1 ) );
    RNOK( MbCoder::xWriteMotionVectors            ( rcMbDataAccess, eMbMode, LIST_1 ) );
  }

  uiMbBits  += BitCounter::getNumberOfWrittenBits();


  //===== set rd-cost =====
  rcMbTempData.rdCost() = m_pcRateDistortionIf->getCost( uiMbBits+uiAdditionalBits, uiMbDist );

  return Err::m_nOK;
}




ErrVal
MbEncoder::xSetRdCostInterSubMb( IntMbTempData&  rcMbTempData,
                                 RefFrameList&   rcRefFrameList0,
                                 RefFrameList&   rcRefFrameList1,
                                 B8x8Idx         c8x8Idx,
                                 Bool            bTrafo8x8,
                                 UInt            uiAddBits,
                                 Bool            bLowComplexity )   // JVT-V079
{
  YuvMbBuffer& rcYuvMbBuffer     = rcMbTempData;
  YuvMbBuffer& rcTempYuvMbBuffer = rcMbTempData.getTempYuvMbBuffer();
  MbDataAccess&   rcMbDataAccess    = rcMbTempData.getMbDataAccess();
  UInt            uiSubMbDist       = 0;
  UInt            uiSubMbBits       = 0;
  Bool            coeffResidualPredFlag = rcMbDataAccess.getCoeffResidualPredFlag();


  if ( bLowComplexity )
  {
    
    if ( rcMbTempData.rdCost() == DOUBLE_MAX )
    {
    //===== get prediction and copy to temp buffer =====
      RNOK( m_pcMotionEstimation->compensateSubMb( c8x8Idx, rcMbDataAccess, 
            rcRefFrameList0, rcRefFrameList1,
            &rcYuvMbBuffer, false, false ) );
      rcTempYuvMbBuffer.loadLuma  ( rcYuvMbBuffer, c8x8Idx );
    
    //===== get distortion =====
      m_pcIntOrgMbPelData->set4x4Block( c8x8Idx );
      rcYuvMbBuffer       .set4x4Block( c8x8Idx );
      uiSubMbDist += m_pcXDistortion->getLum8x8 ( rcYuvMbBuffer.getLumBlk(), rcYuvMbBuffer.getLStride(), DF_SAD );

    //===== set rd-cost =====
      rcMbTempData.rdCost() = uiSubMbDist;
    }
  }
  else
  {
    //===== get prediction and copy to temp buffer =====
    RNOK( m_pcMotionEstimation->compensateSubMb( c8x8Idx, rcMbDataAccess, rcRefFrameList0, rcRefFrameList1,
                                                &rcYuvMbBuffer, false, false ) );
    rcTempYuvMbBuffer.loadLuma  ( rcYuvMbBuffer, c8x8Idx );
  
  
    //===== encode residual and get rate for coefficients =====
    xSetCoeffCost( 0 );
    UInt  uiBits = 0;
    UInt  uiCbp  = 0;
    if( bTrafo8x8 )
    {
      RNOK( xEncode8x8InterBlock( rcMbTempData, c8x8Idx, uiBits, uiCbp ) );
      if( uiCbp )
      {
        if( xGetCoeffCost() <= 4 )
        {
          rcYuvMbBuffer.loadLuma( rcTempYuvMbBuffer, c8x8Idx );
  
          rcMbTempData.clearLumaLevels8x8Block( c8x8Idx );
  
          if( coeffResidualPredFlag && rcMbTempData.getResidualPredFlag( PART_16x16 ) )
          {      
            reCalcBlock8x8(rcMbTempData, c8x8Idx, 0);
          }
          else if ( rcMbDataAccess.getSH().getTCoeffLevelPredictionFlag() && rcMbTempData.getResidualPredFlag( PART_16x16 ) )
          {
            reCalcBlock8x8Rewrite(rcMbTempData, c8x8Idx, 0);            
          }
        }
        else
        {
          uiSubMbBits += uiBits;
        }
      }
    }
    else
    {
      for( S4x4Idx cIdx( c8x8Idx ); cIdx.isLegal( c8x8Idx ); cIdx++ )
      {
        RNOK( xEncode4x4InterBlock( rcMbTempData, cIdx, uiBits, uiCbp ) );
      }
      if( uiCbp )
      {
        if( xGetCoeffCost() <= 4 )
        {
          rcYuvMbBuffer.loadLuma( rcTempYuvMbBuffer, c8x8Idx );
  
          rcMbTempData.clearLumaLevels8x8( c8x8Idx );
  
          if( coeffResidualPredFlag && rcMbTempData.getResidualPredFlag( PART_16x16 ) )
          {
            reCalcBlock8x8(rcMbTempData, c8x8Idx, 1);
          }
          else if( rcMbDataAccess.getSH().getTCoeffLevelPredictionFlag() && rcMbTempData.getResidualPredFlag( PART_16x16 ) )
          {
            reCalcBlock8x8Rewrite(rcMbTempData, c8x8Idx, 1);            
          }
        }
        else
        {
          uiSubMbBits += uiBits;
        }
      }
    }
    //===== get distortion =====
    m_pcIntOrgMbPelData->set4x4Block( c8x8Idx );
    rcYuvMbBuffer       .set4x4Block( c8x8Idx );
    uiSubMbDist += m_pcXDistortion->getLum8x8 ( rcYuvMbBuffer.getLumBlk(), rcYuvMbBuffer.getLStride() );
  
    //===== get rate =====
    uiSubMbBits += uiAddBits;
  
    //===== set rd-cost =====
    rcMbTempData.rdCost() = m_pcRateDistortionIf->getCost( uiSubMbBits, uiSubMbDist );
  }
  
  return Err::m_nOK;
}






ErrVal
MbEncoder::xEstimateMbDirect( IntMbTempData*&  rpcMbTempData,
                              IntMbTempData*&  rpcMbBestData,
                              RefFrameList&    rcRefFrameList0,
                              RefFrameList&    rcRefFrameList1,
                              MbDataAccess*    pcMbDataAccessBaseMotion,
                              Bool             bResidualPred,
                              UInt             uiQp,
                              Bool             bSkipModeAllowed)
{
  ROFRS( rcRefFrameList0.getActive() && rcRefFrameList1.getActive(), Err::m_nOK );

  Int iRefIdxL0 = 1;
  Int iRefIdxL1 = 1;
  Mv  cMvPredL0;
  Mv  cMvPredL1;


  //JVT-V079 Low-complexity MB mode decision
  Bool bLowComplexMbEnable = m_bLowComplexMbEnable[rpcMbTempData->getSH().getDependencyId()];
  
  rpcMbTempData->getMbDataAccess    ().getMvPredictor         ( cMvPredL0, iRefIdxL0, LIST_0 );
  rpcMbTempData->getMbDataAccess    ().getMvPredictor         ( cMvPredL1, iRefIdxL1, LIST_1 );
  rpcMbTempData->clear              ();
  rpcMbTempData->setMbMode          ( MODE_SKIP );
  rpcMbTempData->setBLSkipFlag      ( false );
  rpcMbTempData->getMbMotionData    ( LIST_0 ).setRefIdx      ( iRefIdxL0 );
  rpcMbTempData->getMbMotionData    ( LIST_0 ).setAllMv       ( cMvPredL0 );
  rpcMbTempData->getMbMvdData       ( LIST_0 ).setAllMv       ( Mv::ZeroMv() );
  rpcMbTempData->getMbMotionData    ( LIST_0 ).setMotPredFlag ( false );
  rpcMbTempData->getMbMotionData    ( LIST_1 ).setRefIdx      ( iRefIdxL1 );
  rpcMbTempData->getMbMotionData    ( LIST_1 ).setAllMv       ( cMvPredL1 );
  rpcMbTempData->getMbMvdData       ( LIST_1 ).setAllMv       ( Mv::ZeroMv() );
  rpcMbTempData->getMbMotionData    ( LIST_1 ).setMotPredFlag ( false );
  rpcMbTempData->setResidualPredFlag( bResidualPred );


  if( rpcMbTempData->getSH().isH264AVCCompatible() )
  {
    //===== H.264/AVC compatible direct mode =====
    Bool            bOneMv          = false;
    Bool            bFaultTolerant  = false;
    MbDataAccess&   rcMbDataAccess  = rpcMbTempData->getMbDataAccess();
    B8x8Idx         c8x8Idx;
    ROFRS( rcMbDataAccess.getMvPredictorDirect( c8x8Idx.b8x8(), bOneMv, bFaultTolerant, &rcRefFrameList0, &rcRefFrameList1 ), Err::m_nOK ); c8x8Idx++;
    ROFRS( rcMbDataAccess.getMvPredictorDirect( c8x8Idx.b8x8(), bOneMv, bFaultTolerant, &rcRefFrameList0, &rcRefFrameList1 ), Err::m_nOK ); c8x8Idx++;
    ROFRS( rcMbDataAccess.getMvPredictorDirect( c8x8Idx.b8x8(), bOneMv, bFaultTolerant, &rcRefFrameList0, &rcRefFrameList1 ), Err::m_nOK ); c8x8Idx++;
    ROFRS( rcMbDataAccess.getMvPredictorDirect( c8x8Idx.b8x8(), bOneMv, bFaultTolerant, &rcRefFrameList0, &rcRefFrameList1 ), Err::m_nOK );
	}

    if( ! bSkipModeAllowed )
    {
      return Err::m_nOK;
    }


  IntMbTempData* pcMbRefData = rpcMbTempData;

  RNOK( xSetRdCostInterMb   ( *rpcMbTempData, pcMbDataAccessBaseMotion, rcRefFrameList0, rcRefFrameList1, false, 0, false, 0, bLowComplexMbEnable ) );


  if ( bLowComplexMbEnable )
  {
    Int tmp_max_qp = ( uiQp - SHIFT_QP );  
    if ( tmp_max_qp<0) tmp_max_qp=0;
    rpcMbTempData->rdCost() -= 8 * g_iQP2quant[tmp_max_qp];
  } 
  
  //JVT-R057 LA-RDO{
  if(m_bLARDOEnable)
  {
	  MbDataAccess&   rcMbDataAccess  = rpcMbTempData->getMbDataAccess();
	  Int distortion1=0,distortion2=0,distortion=0;
	  for( Int n = 0; n <1; n++)
	  {
		  Int iRefIdx[2];
		  iRefIdx [0]=rpcMbTempData->getMbMotionData(LIST_0).getRefIdx(B4x4Idx(n));
		  iRefIdx [1]=rpcMbTempData->getMbMotionData(LIST_1).getRefIdx(B4x4Idx(n));
		  Frame* pcRefFrame0 = ( iRefIdx [0] > 0 ? rcRefFrameList0[ iRefIdx [0] ] : NULL );
		  Frame* pcRefFrame1 = ( iRefIdx [1] > 0 ? rcRefFrameList1[ iRefIdx [1] ] : NULL );
		  Int iMvX;
		  Int iMvY;

		  if(pcRefFrame0)
		  {
			  iMvX=rpcMbTempData->getMbMotionData(LIST_0).getMv(B4x4Idx(n)).getHor();
			  iMvY=rpcMbTempData->getMbMotionData(LIST_0).getMv(B4x4Idx(n)).getVer();
			  getChannelDistortion(rcMbDataAccess,*pcRefFrame0,&distortion1,iMvX,iMvY,0,0,4,4);
		  }
		  if(pcRefFrame1)
		  {
			  iMvX=rpcMbTempData->getMbMotionData(LIST_1).getMv(B4x4Idx(n)).getHor();
			  iMvY=rpcMbTempData->getMbMotionData(LIST_1).getMv(B4x4Idx(n)).getVer();
			  getChannelDistortion(rcMbDataAccess,*pcRefFrame1,&distortion2,iMvX,iMvY,0,0,4,4);
			  if(pcRefFrame0)
				  distortion1=(Int)(m_dWr0*distortion1+m_dWr0*distortion2);
			  else
				  distortion1=distortion2;
		  }
		  distortion+=distortion1;
	  }
	  setEpRef(distortion);
	  rpcMbTempData->rdCost()+=distortion;
  }
  //JVT-R057 LA-RDO}

  RNOK( xCheckBestEstimation(  rpcMbTempData, rpcMbBestData ) );

  RNOK( xCheckInterMbMode8x8(  rpcMbTempData, rpcMbBestData, pcMbRefData, rcRefFrameList0, rcRefFrameList1, pcMbDataAccessBaseMotion ) );

  return Err::m_nOK;
}



ErrVal
MbEncoder::xEstimateMbBLSkip( IntMbTempData*&   rpcIntMbTempData,
                              IntMbTempData*&   rpcIntMbBestData,
                              RefFrameList&     rcRefFrameList0,
                              RefFrameList&     rcRefFrameList1,
                              const Frame*   pcBaseLayerRec,
                              Bool              bBSlice,
                              Int               iSpatialScalabilityType,
                              MbDataAccess*     pcMbDataAccessBase,
                              MbDataAccess&     rcMbDataAccess,
                              Bool              bResidualPred )
{
  ROF( pcMbDataAccessBase );

  PicType eMbPicType = rcMbDataAccess.getMbPicType();

  //JVT-V079 Low-complexity MB mode decision
  Bool bLowComplexMbEnable = m_bLowComplexMbEnable[rpcIntMbTempData->getSH().getDependencyId()];
  
  if( eMbPicType != FRAME && pcMbDataAccessBase->getMbPicType() == FRAME )
  {
    // check inter intra mixed modes
    if( pcMbDataAccessBase->getMbData().isIntra() != pcMbDataAccessBase->getMbDataComplementary().isIntra() )
    {
      printf( "  %d %d ", pcMbDataAccessBase->getMbData().getMbMode(), pcMbDataAccessBase->getMbDataComplementary().getMbMode());
      return Err::m_nOK;
    }
    if( ! pcMbDataAccessBase->getMbData().isIntra() && pcMbDataAccessBase->getMbData().getSliceId() != pcMbDataAccessBase->getMbDataComplementary().getSliceId() )
    {
      printf( "  %d %d ", pcMbDataAccessBase->getMbData().getSliceId(), pcMbDataAccessBase->getMbDataComplementary().getSliceId());
      return Err::m_nOK;
    }
  }

  if( ! pcMbDataAccessBase->getMbData().isIntra() )
  {
    ROF( bResidualPred || rpcIntMbTempData->getSH().getAdaptiveBaseModeFlag() );

    //===== BASE LAYER MODE IS INTER =====
    rpcIntMbTempData->clear               ();
    rpcIntMbTempData->copyMotion          ( pcMbDataAccessBase->getMbData() );
    rpcIntMbTempData->setBLSkipFlag       ( true  );
    rpcIntMbTempData->getMbMvdData        ( LIST_0 ).setAllMv( Mv::ZeroMv() );
    rpcIntMbTempData->getMbMvdData        ( LIST_1 ).setAllMv( Mv::ZeroMv() );

    rpcIntMbTempData->setResidualPredFlag ( bResidualPred );

    IntMbTempData* pcMbRefData = rpcIntMbTempData;

    RNOK( xSetRdCostInterMb   ( *rpcIntMbTempData, pcMbDataAccessBase, rcRefFrameList0, rcRefFrameList1, true,
          0, false, (Frame *)pcBaseLayerRec, bLowComplexMbEnable ) ) ;

    //JVT-R057 LA-RDO{
    if(m_bLARDOEnable)
    {
      MbDataAccess&  rcMbDataAccess1  = rpcIntMbTempData->getMbDataAccess();
      Int distortion1=0,distortion2=0,distortion=0;
      //Bug_Fix JVT-R057 0806{

      for( Int n = 0; n <16; n++)
      {
        Int iRefIdx[2];
        iRefIdx [0]=rcMbDataAccess1.getMbMotionData(LIST_0).getRefIdx(B4x4Idx(n));
        iRefIdx [1]=rcMbDataAccess1.getMbMotionData(LIST_1).getRefIdx(B4x4Idx(n));
        Frame* pcRefFrame0 = ( iRefIdx [0] > 0 ? rcRefFrameList0[ iRefIdx [0] ] : NULL );
        Frame* pcRefFrame1 = ( iRefIdx [1] > 0 ? rcRefFrameList1[ iRefIdx [1] ] : NULL );
        Int iMvX;
        Int iMvY;

        if(pcRefFrame0)
        {
          iMvX=rcMbDataAccess1.getMbMotionData(LIST_0).getMv(B4x4Idx(n)).getHor();
          iMvY=rcMbDataAccess1.getMbMotionData(LIST_0).getMv(B4x4Idx(n)).getVer();
          getChannelDistortion(rcMbDataAccess1,*pcRefFrame0,&distortion1,iMvX,iMvY,n%4,n/4,1,1);
        }
        if(pcRefFrame1)
        {
          iMvX=rcMbDataAccess1.getMbMotionData(LIST_1).getMv(B4x4Idx(n)).getHor();
          iMvY=rcMbDataAccess1.getMbMotionData(LIST_1).getMv(B4x4Idx(n)).getVer();
          getChannelDistortion(rcMbDataAccess1,*pcRefFrame1,&distortion2,iMvX,iMvY,n%4,n/4,1,1);
          distortion1=(Int)(m_dWr0*distortion1+m_dWr0*distortion2);
        }
        distortion+=distortion1;
      }
      //Bug_Fix JVT-R057 0806}

      setEpRef(distortion);
      rpcIntMbTempData->rdCost()+=distortion;
    }
    //JVT-R057 LA-RDO}

    RNOK( xCheckBestEstimation(  rpcIntMbTempData, rpcIntMbBestData ) );

    RNOK( xCheckInterMbMode8x8(  rpcIntMbTempData, rpcIntMbBestData, pcMbRefData, rcRefFrameList0, rcRefFrameList1, pcMbDataAccessBase,
                                (Frame *)pcBaseLayerRec) );
  }
  else
  {
    //===== INTRA MODE =====
    if ( pcMbDataAccessBase->getMbData().getInCropWindowFlag() ) // TMM_ESS
	    RNOK( xEstimateMbIntraBL  (  rcMbDataAccess, rpcIntMbTempData, rpcIntMbBestData, pcBaseLayerRec, bBSlice, pcMbDataAccessBase ) );
  }

  return Err::m_nOK;
}


ErrVal
MbEncoder::xEstimateMbSkip( IntMbTempData*&  rpcMbTempData,
                            IntMbTempData*&  rpcMbBestData,
                            RefFrameList&    rcRefFrameList0,
                            RefFrameList&    rcRefFrameList1 )
{
  ROFRS( rpcMbTempData->getSH().getSliceType() == P_SLICE, Err::m_nOK );
  ROF( rcRefFrameList0.getActive() );

  Int iRefIdxL0 = 1;
  Int iRefIdxL1 = BLOCK_NOT_PREDICTED;
  Mv  cMvPredL0;
  Mv  cMvPredL1;

  //JVT-V079 Low-complexity MB mode decision
  Bool bLowComplexMbEnable = m_bLowComplexMbEnable[rpcMbTempData->getSH().getDependencyId()];
  
  rpcMbTempData->clear();
  rpcMbTempData->setMbMode( MODE_SKIP );
  rpcMbTempData->setBLSkipFlag( false );
  rpcMbTempData->getMbDataAccess().getMvPredictorSkipMode( cMvPredL0 );
  rpcMbTempData->getMbMotionData( LIST_0 ).setRefIdx( iRefIdxL0 );
  rpcMbTempData->getMbMotionData( LIST_0 ).setAllMv ( cMvPredL0 );
  rpcMbTempData->getMbMvdData   ( LIST_0 ).setAllMv ( Mv::ZeroMv() );
  rpcMbTempData->getMbMotionData( LIST_1 ).setRefIdx( iRefIdxL1 );
  rpcMbTempData->getMbMotionData( LIST_1 ).setAllMv ( cMvPredL1 );
  rpcMbTempData->getMbMvdData   ( LIST_1 ).setAllMv ( Mv::ZeroMv() );
  rpcMbTempData->setResidualPredFlag(false);

  rpcMbTempData->getMbMotionData( LIST_0 ).setMotPredFlag( false );
  rpcMbTempData->getMbMotionData( LIST_1 ).setMotPredFlag( false );

  RNOK( xSetRdCostInterMb   ( *rpcMbTempData, NULL, rcRefFrameList0, rcRefFrameList1,
        false, 0, false, 0, bLowComplexMbEnable) );


  //JVT-R057 LA-RDO}
  if(m_bLARDOEnable)
  {
	  MbDataAccess&   rcMbDataAccess  = rpcMbTempData->getMbDataAccess();
	  Int distortion1=0,distortion2=0,distortion=0;
	  for( Int n = 0; n <1; n++)
	  {
		  Int iRefIdx[2];
		  iRefIdx [0]=rpcMbTempData->getMbMotionData(LIST_0).getRefIdx(B4x4Idx(n));
		  iRefIdx [1]=rpcMbTempData->getMbMotionData(LIST_1).getRefIdx(B4x4Idx(n));
		  Frame* pcRefFrame0 = ( iRefIdx [0] > 0 ? rcRefFrameList0[ iRefIdx [0] ] : NULL );
		  Frame* pcRefFrame1 = ( iRefIdx [1] > 0 ? rcRefFrameList1[ iRefIdx [1] ] : NULL );
		  Int iMvX;
		  Int iMvY;

		  if(pcRefFrame0)
		  {
			  iMvX=rpcMbTempData->getMbMotionData(LIST_0).getMv(B4x4Idx(n)).getHor();
			  iMvY=rpcMbTempData->getMbMotionData(LIST_0).getMv(B4x4Idx(n)).getVer();
			  getChannelDistortion(rcMbDataAccess,*pcRefFrame0,&distortion1,iMvX,iMvY,0,0,4,4);
		  }
		  if(pcRefFrame1)
		  {
			  iMvX=rpcMbTempData->getMbMotionData(LIST_1).getMv(B4x4Idx(n)).getHor();
			  iMvY=rpcMbTempData->getMbMotionData(LIST_1).getMv(B4x4Idx(n)).getVer();
			  getChannelDistortion(rcMbDataAccess,*pcRefFrame1,&distortion2,iMvX,iMvY,0,0,4,4);
			  if(pcRefFrame0)
				  distortion1=(Int)(m_dWr0*distortion1+m_dWr0*distortion2);
			  else
				  distortion1=distortion2;
		  }
		  distortion+=distortion1;
	  }
	  setEpRef(distortion);
	  rpcMbTempData->rdCost()+=distortion;
  }
  //JVT-R057 LA-RDO}

  RNOK( xCheckBestEstimation(  rpcMbTempData, rpcMbBestData ) );

  return Err::m_nOK;
}


__inline
UInt
getRefIdxBits( Int iRefIdx, RefFrameList& rcRefFrameList )
{
  AOT  ( rcRefFrameList.getActive() == 0    );
  ROTRS( rcRefFrameList.getActive() == 1, 0 );
  ROTRS( rcRefFrameList.getActive() == 2, 1 );

  return g_aucFrameBits[ iRefIdx ];
}

ErrVal
MbEncoder::xEstimateMb16x16( IntMbTempData*&  rpcMbTempData,
                             IntMbTempData*&  rpcMbBestData,
                             RefFrameList&    rcRefFrameList0,
                             RefFrameList&    rcRefFrameList1,
                             Bool             bBiPredOnly,
                             UInt             uiNumMaxIter,
                             UInt             uiIterSearchRange,
                             Bool             bQPelRefinementOnly,
                             MbDataAccess*    pcMbDataAccessBase,
                             Bool             bResidualPred )
{
  ROF( rcRefFrameList0.getActive() <= 32 );
  ROF( rcRefFrameList1.getActive() <= 32 );
  ROF( rcRefFrameList0.getActive() );
  ROF( ! bBiPredOnly || rcRefFrameList1.getActive() );

  
  Bool            bPSlice       = rpcMbTempData->getMbDataAccess().getSH().isPSlice();
  Double          fRdCost       = 0;
  UInt            uiCost  [3]   = { MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX }, uiCostTest;
  UInt            uiMbBits[3]   = { ( ! bPSlice ? 3 : 1 ), 3, 5 };
  Int             iRefIdx [2]   = { 0, 0 }, iRefIdxBi[2], iRefIdxTest;
  UInt            uiBits  [3]   = { 0, 0, 0 }, uiBitsTest;
  Mv              cMv[2], cMvBi[2], cMvPred[2][33], cMvLastEst[2][33], cMvd[2];
  YuvMbBuffer  cYuvMbBuffer[2];
  Frame*       pcRefFrame;
  
  //JVT-V079 Low-complexity MB mode decision
  Bool bLowComplexMbEnable = m_bLowComplexMbEnable[rpcMbTempData->getSH().getDependencyId()];

  Bool            bBLPred   [2] = { false, false };
  Bool            bBLPredBi [2] = { false, false };
  Int             iBLRefIdx [2] = { -1, -1 };
  Mv              cBLMvPred [2], cBLMvLastEst[2];

  if( pcMbDataAccessBase )
  {
    if( pcMbDataAccessBase->getMbMotionData( LIST_0 ).getRefIdx() >  0                           &&
        pcMbDataAccessBase->getMbMotionData( LIST_0 ).getRefIdx() <= (Int)rcRefFrameList0.getActive()   )
    {
      iBLRefIdx [0] = pcMbDataAccessBase->getMbMotionData( LIST_0 ).getRefIdx ();
      cBLMvPred [0] = pcMbDataAccessBase->getMbMotionData( LIST_0 ).getMv     ();
    }
    if( pcMbDataAccessBase->getMbMotionData( LIST_1 ).getRefIdx() >  0                           &&
        pcMbDataAccessBase->getMbMotionData( LIST_1 ).getRefIdx() <= (Int)rcRefFrameList1.getActive()   )
    {
      iBLRefIdx [1] = pcMbDataAccessBase->getMbMotionData( LIST_1 ).getRefIdx ();
      cBLMvPred [1] = pcMbDataAccessBase->getMbMotionData( LIST_1 ).getMv     ();
    }
  }

  UInt  uiBasePredType = MSYS_UINT_MAX;
  rpcMbTempData->clear();
  rpcMbTempData->setMbMode( MODE_16x16 );
  rpcMbTempData->setBLSkipFlag( false );
  rpcMbTempData->setResidualPredFlag( bResidualPred );

  //===== LIST 0 PREDICTION ======
  for( iRefIdxTest = 1; iRefIdxTest <= (Int)rcRefFrameList0.getActive(); iRefIdxTest++ )
  {
    rpcMbTempData->getMbDataAccess().getMvPredictor   ( cMvPred[0][iRefIdxTest], iRefIdxTest, LIST_0 );
    uiBitsTest                  = ( uiBasePredType == 0 ? 0 : uiMbBits[0] ) + getRefIdxBits( iRefIdxTest, rcRefFrameList0 );
    cMvLastEst[0][iRefIdxTest]  = cMvPred [0][iRefIdxTest];
    pcRefFrame                  = rcRefFrameList0[iRefIdxTest];
    Bool bQPel = ( bQPelRefinementOnly &&
                   iRefIdxTest == iBLRefIdx[0] &&
                   cMvPred[0][iRefIdxTest] == cBLMvPred[0] );
    RNOK( m_pcMotionEstimation->estimateBlockWithStart( *rpcMbTempData, *pcRefFrame,
                                                        cMvLastEst[0][iRefIdxTest],
                                                        cMvPred   [0][iRefIdxTest],
                                                        uiBitsTest, uiCostTest,
                                                        PART_16x16, MODE_16x16, bQPel, 0,
                                                        &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdxTest, rpcMbTempData->getFieldFlag()) ) );
    if( uiCostTest < uiCost[0] )
    {
      bBLPred   [0] = false;
      iRefIdx   [0] = iRefIdxTest;
      cMv       [0] = cMvLastEst[0][iRefIdxTest];
      uiBits    [0] = uiBitsTest;
      uiCost    [0] = uiCostTest;
    }

    if( iRefIdxTest == iBLRefIdx[0] )
    {
      uiBitsTest      = ( uiBasePredType == 0 ? 0 : uiMbBits[0] );
      cBLMvLastEst[0] = cBLMvPred [0];

      rpcMbTempData->getMbDataAccess().setMvPredictorsBL( cBLMvPred[0], LIST_0 );
      RNOK( m_pcMotionEstimation->estimateBlockWithStart( *rpcMbTempData, *pcRefFrame,
                                                          cBLMvLastEst[0],
                                                          cBLMvPred   [0],
                                                          uiBitsTest, uiCostTest,
                                                          PART_16x16, MODE_16x16, bQPelRefinementOnly, 0,
                                                          &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdxTest, rpcMbTempData->getFieldFlag()) ) );
      if( uiCostTest < uiCost[0] )
      {
        bBLPred [0] = true;
        iRefIdx [0] = iRefIdxTest;
        cMv     [0] = cBLMvLastEst[0];
        uiBits  [0] = uiBitsTest;
        uiCost  [0] = uiCostTest;
      }
    }
  }


  //===== LIST 1 PREDICTION =====
  for( iRefIdxTest = 1; iRefIdxTest <= (Int)rcRefFrameList1.getActive(); iRefIdxTest++ )
  {
    rpcMbTempData->getMbDataAccess().getMvPredictor   ( cMvPred[1][iRefIdxTest], iRefIdxTest,
                                                        LIST_1 );
    uiBitsTest                  = ( uiBasePredType == 1 ? 0 : uiMbBits[1] ) + getRefIdxBits( iRefIdxTest, rcRefFrameList1 );
    cMvLastEst[1][iRefIdxTest]  = cMvPred [1][iRefIdxTest];
    pcRefFrame                  = rcRefFrameList1[iRefIdxTest];
    Bool bQPel = ( bQPelRefinementOnly &&
                   iRefIdxTest == iBLRefIdx[1] &&
                   cMvPred[1][iRefIdxTest] == cBLMvPred[1] );
    RNOK( m_pcMotionEstimation->estimateBlockWithStart(  *rpcMbTempData, *pcRefFrame,
                                                        cMvLastEst[1][iRefIdxTest],
                                                        cMvPred   [1][iRefIdxTest],
                                                        uiBitsTest,
                                                        uiCostTest,
                                                        PART_16x16,
                                                        MODE_16x16,
                                                        bQPel,
                                                        0,
                                                        &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdxTest, rpcMbTempData->getFieldFlag()) ) );
    if( uiCostTest < uiCost[1] )
    {
      bBLPred   [1] = false;
      iRefIdx   [1] = iRefIdxTest;
      cMv       [1] = cMvLastEst[1][iRefIdxTest];
      uiBits    [1] = uiBitsTest;
      uiCost    [1] = uiCostTest;

      RNOK( m_pcMotionEstimation->compensateBlock     ( &cYuvMbBuffer[1],
                                                        PART_16x16, MODE_16x16 ) );
    }

    if( iRefIdxTest == iBLRefIdx[1] )
    {
      uiBitsTest      = ( uiBasePredType == 1 ? 0 : uiMbBits[1] );
      cBLMvLastEst[1] = cBLMvPred [1];

      rpcMbTempData->getMbDataAccess().setMvPredictorsBL( cBLMvPred[1], LIST_1 );
      RNOK( m_pcMotionEstimation->estimateBlockWithStart( *rpcMbTempData, *pcRefFrame,
                                                          cBLMvLastEst[1],
                                                          cBLMvPred   [1],
                                                          uiBitsTest,
                                                          uiCostTest,
                                                          PART_16x16,
                                                          MODE_16x16,
                                                          bQPelRefinementOnly,
                                                          0,
                                                          &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdxTest, rpcMbTempData->getFieldFlag()) ) );
      if( uiCostTest < uiCost[1] )
      {
        bBLPred [1] = true;
        iRefIdx [1] = iRefIdxTest;
        cMv     [1] = cBLMvLastEst[1];
        uiBits  [1] = uiBitsTest;
        uiCost  [1] = uiCostTest;

        RNOK( m_pcMotionEstimation->compensateBlock     ( &cYuvMbBuffer[1],
                                                          PART_16x16, MODE_16x16 ) );
      }
    }
  }

  //===== BI PREDICTION =====
  if( rcRefFrameList0.getActive() && rcRefFrameList1.getActive() )
  {
    //----- initialize with forward and backward estimation -----
    cMvBi           [0] = cMv     [0];
    cMvBi           [1] = cMv     [1];
    iRefIdxBi       [0] = iRefIdx [0];
    iRefIdxBi       [1] = iRefIdx [1];
    bBLPredBi       [0] = bBLPred [0];
    bBLPredBi       [1] = bBLPred [1];
    UInt  uiMotBits [2] = { uiBits[0] - uiMbBits[0], uiBits[1] - uiMbBits[1] };
    uiBits          [2] = uiMbBits[2] + uiMotBits[0] + uiMotBits[1];

    if( ! uiNumMaxIter )
    {
      uiNumMaxIter      = 1;
      uiIterSearchRange = 0;
    }

    //----- iterative search -----
    for( UInt uiIter = 0; uiIter < uiNumMaxIter; uiIter++ )
    {
      Bool          bChanged        = false;
      UInt          uiDir           = uiIter % 2;
      RefFrameList& rcRefFrameList  = ( uiDir ? rcRefFrameList1 : rcRefFrameList0 );
      BSParams      cBSParams;
      cBSParams.pcAltRefFrame       = ( uiDir ? rcRefFrameList0 : rcRefFrameList1 )[ iRefIdxBi[ 1 - uiDir ] ];
      cBSParams.pcAltRefPelData     = &cYuvMbBuffer[1-uiDir];
      cBSParams.uiL1Search          = uiDir;
      cBSParams.apcWeight[LIST_0]   = &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdxBi[LIST_0], rpcMbTempData->getFieldFlag() );
      cBSParams.apcWeight[LIST_1]   = &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdxBi[LIST_1], rpcMbTempData->getFieldFlag() );

      for( iRefIdxTest = 1; iRefIdxTest <= (Int)rcRefFrameList.getActive(); iRefIdxTest++ )
      {
        uiBitsTest  = ( uiBasePredType == 2 ? 0 : uiMbBits[2] ) + uiMotBits[1-uiDir] + getRefIdxBits( iRefIdxTest, rcRefFrameList );
        pcRefFrame  = rcRefFrameList[iRefIdxTest];
        Bool bQPel = ( bQPelRefinementOnly &&
                       iRefIdxTest == iBLRefIdx[uiDir] &&
                       cMvPred[uiDir][iRefIdxTest] == cBLMvPred[uiDir] );
        RNOK( m_pcMotionEstimation->estimateBlockWithStart( *rpcMbTempData, *pcRefFrame,
                                                            cMvLastEst[uiDir][iRefIdxTest],
                                                            cMvPred   [uiDir][iRefIdxTest],
                                                            uiBitsTest, uiCostTest,
                                                            PART_16x16, MODE_16x16, bQPel,
                                                            uiIterSearchRange, 0, &cBSParams ) );

        if( uiCostTest < uiCost[2] )
        {
          bChanged          = true;
          bBLPredBi [uiDir] = false;
          iRefIdxBi [uiDir] = iRefIdxTest;
          cMvBi     [uiDir] = cMvLastEst[uiDir][iRefIdxTest];
          uiMotBits [uiDir] = uiBitsTest - uiMbBits[2] - uiMotBits[1-uiDir];
          uiBits    [2]     = uiBitsTest;
          uiCost    [2]     = uiCostTest;

          RNOK( m_pcMotionEstimation->compensateBlock     ( &cYuvMbBuffer[uiDir],
                                                            PART_16x16, MODE_16x16 ) );
        }

        if( iRefIdxTest == iBLRefIdx[uiDir] )
        {
          uiBitsTest      = ( uiBasePredType == 2 ? 0 : uiMbBits[2] ) + uiMotBits[1-uiDir];
          RNOK( m_pcMotionEstimation->estimateBlockWithStart( *rpcMbTempData, *pcRefFrame,
                                                              cBLMvLastEst[uiDir],
                                                              cBLMvPred   [uiDir],
                                                              uiBitsTest, uiCostTest,
                                                              PART_16x16, MODE_16x16, bQPelRefinementOnly,
                                                              uiIterSearchRange, 0, &cBSParams ) );
          if( uiCostTest < uiCost[2] )
          {
            bChanged          = true;
            bBLPredBi [uiDir] = true;
            iRefIdxBi [uiDir] = iRefIdxTest;
            cMvBi     [uiDir] = cBLMvLastEst[uiDir];
            uiMotBits [uiDir] = uiBitsTest - uiMbBits[2] - uiMotBits[1-uiDir];
            uiBits    [2]     = uiBitsTest;
            uiCost    [2]     = uiCostTest;

            RNOK( m_pcMotionEstimation->compensateBlock     ( &cYuvMbBuffer[uiDir],
                                                              PART_16x16, MODE_16x16 ) );
          }
        }
      }

      if( ! bChanged )
      {
        break;
      }
    }

    if( bBiPredOnly )
    {
      uiCost[0] = MSYS_UINT_MAX;
      uiCost[1] = MSYS_UINT_MAX;
    }
  }


  //===== chose parameters =====
  if( uiCost[2] <= uiCost[0] && uiCost[2] <= uiCost[1] )
  {
    //----- bi-directional prediction -----
    fRdCost     = uiCost    [2];
    iRefIdx [0] = iRefIdxBi [0];
    iRefIdx [1] = iRefIdxBi [1];
    bBLPred [0] = bBLPredBi [0];
    bBLPred [1] = bBLPredBi [1];
    if( bBLPred[0] )  cMvPred[0][iRefIdx[0]] = cBLMvPred[0];
    if( bBLPred[1] )  cMvPred[1][iRefIdx[1]] = cBLMvPred[1];
    cMv     [0] = cMvBi     [0];
    cMv     [1] = cMvBi     [1];
    cMvd    [0] = cMv       [0] - cMvPred[0][iRefIdx[0]];
    cMvd    [1] = cMv       [1] - cMvPred[1][iRefIdx[1]];
  }
  else if( uiCost[0] <= uiCost[1] )
  {
    //----- list 0 prediction -----
    fRdCost     = uiCost[0];
    iRefIdx [1] = BLOCK_NOT_PREDICTED;
    bBLPred [1] = false;
    if( bBLPred[0] )  cMvPred[0][iRefIdx[0]] = cBLMvPred[0];
    cMv     [1] = Mv::ZeroMv();
    cMvd    [0] = cMv[0] - cMvPred[0][iRefIdx[0]];
  }
  else
  {
    //----- list 1 prediction -----
    fRdCost     = uiCost[1];
    iRefIdx [0] = BLOCK_NOT_PREDICTED;
    bBLPred [0] = false;
    if( bBLPred[1] )  cMvPred[1][iRefIdx[1]] = cBLMvPred[1];
    cMv     [0] = Mv::ZeroMv();
    cMvd    [1] = cMv[1] - cMvPred[1][iRefIdx[1]];
  }


  //===== set parameters and compare =====
  rpcMbTempData->rdCost() = fRdCost;
  rpcMbTempData->getMbMotionData( LIST_0 ).setRefIdx    ( iRefIdx [0] );
  rpcMbTempData->getMbMotionData( LIST_0 ).setAllMv     ( cMv     [0] );
  rpcMbTempData->getMbMvdData   ( LIST_0 ).setAllMv     ( cMvd    [0] );

  rpcMbTempData->getMbMotionData( LIST_1 ).setRefIdx    ( iRefIdx [1] );
  rpcMbTempData->getMbMotionData( LIST_1 ).setAllMv     ( cMv     [1] );
  rpcMbTempData->getMbMvdData   ( LIST_1 ).setAllMv     ( cMvd    [1] );

  rpcMbTempData->getMbMotionData( LIST_0 ).setMotPredFlag( bBLPred [0] );
  rpcMbTempData->getMbMotionData( LIST_1 ).setMotPredFlag( bBLPred [1] );
  ROT( bBLPred[0] && iRefIdx[0] != iBLRefIdx[0] );
  ROT( bBLPred[1] && iRefIdx[1] != iBLRefIdx[1] );

  IntMbTempData* pcMbRefData = rpcMbTempData;

  RNOK( xSetRdCostInterMb   ( *rpcMbTempData, pcMbDataAccessBase, rcRefFrameList0, rcRefFrameList1, 
        false, 0, false, 0, bLowComplexMbEnable ) );


  //JVT-R057 LA-RDO}
  if(m_bLARDOEnable)
  {
	  MbDataAccess&   rcMbDataAccess  = rpcMbTempData->getMbDataAccess();
	  Int distortion1=0,distortion2=0,distortion=0;
	  for( Int n = 0; n <1; n++)
	  {
		  iRefIdx [0]=rpcMbTempData->getMbMotionData(LIST_0).getRefIdx(B4x4Idx(n));
		  iRefIdx [1]=rpcMbTempData->getMbMotionData(LIST_1).getRefIdx(B4x4Idx(n));
		  Frame* pcRefFrame0 = ( iRefIdx [0] > 0 ? rcRefFrameList0[ iRefIdx [0] ] : NULL );
		  Frame* pcRefFrame1 = ( iRefIdx [1] > 0 ? rcRefFrameList1[ iRefIdx [1] ] : NULL );
		  Int iMvX;
		  Int iMvY;

		  if(pcRefFrame0)
		  {
			  iMvX=rpcMbTempData->getMbMotionData(LIST_0).getMv(B4x4Idx(n)).getHor();
			  iMvY=rpcMbTempData->getMbMotionData(LIST_0).getMv(B4x4Idx(n)).getVer();
			  getChannelDistortion(rcMbDataAccess,*pcRefFrame0,&distortion1,iMvX,iMvY,0,0,4,4);
		  }
		  if(pcRefFrame1)
		  {
			  iMvX=rpcMbTempData->getMbMotionData(LIST_1).getMv(B4x4Idx(n)).getHor();
			  iMvY=rpcMbTempData->getMbMotionData(LIST_1).getMv(B4x4Idx(n)).getVer();
			  getChannelDistortion(rcMbDataAccess,*pcRefFrame1,&distortion2,iMvX,iMvY,0,0,4,4);
			  if(pcRefFrame0)
				  distortion1=(Int)(m_dWr0*distortion1+m_dWr0*distortion2);
			  else
				  distortion1=distortion2;
		  }
		  distortion+=distortion1;
	  }
	  setEpRef(distortion);
	  rpcMbTempData->rdCost()+=distortion;
  }
  //JVT-R057 LA-RDO}

  RNOK( xCheckBestEstimation(  rpcMbTempData, rpcMbBestData ) );

  RNOK( xCheckInterMbMode8x8(  rpcMbTempData, rpcMbBestData, pcMbRefData, rcRefFrameList0, rcRefFrameList1, pcMbDataAccessBase ) );

  return Err::m_nOK;
}




ErrVal
MbEncoder::xEstimateMb16x8 ( IntMbTempData*&  rpcMbTempData,
                             IntMbTempData*&  rpcMbBestData,
                             RefFrameList&    rcRefFrameList0,
                             RefFrameList&    rcRefFrameList1,
                             Bool             bBiPredOnly,
                             UInt             uiNumMaxIter,
                             UInt             uiIterSearchRange,
                             Bool             bQPelRefinementOnly,
                             MbDataAccess*    pcMbDataAccessBase,
                             Bool             bResidualPred )
{
  ROF( rcRefFrameList0.getActive() <= 32 );
  ROF( rcRefFrameList1.getActive() <= 32 );
  ROF( rcRefFrameList0.getActive() );
  ROF( ! bBiPredOnly || rcRefFrameList1.getActive() );

  const UInt aauiMbBits[2][3][3] = { { {0,0,3}, {0,0,0}, {0,0,0} } , { {5,7,7}, {7,5,7}, {9-3,9-3,9-3} } };

  rpcMbTempData->clear();
  rpcMbTempData->setMbMode( MODE_16x8 );
  rpcMbTempData->setBLSkipFlag( false );
  rpcMbTempData->setResidualPredFlag( bResidualPred );

  Bool   bPSlice       = rpcMbTempData->getMbDataAccess().getSH().isPSlice();
  Double fRdCost       = 0;
  UInt   uiLastMode    = 0;

  //JVT-V079 Low-complexity MB mode decision
  Bool bLowComplexMbEnable = m_bLowComplexMbEnable[rpcMbTempData->getSH().getDependencyId()];
  
  for( UInt   uiBlk = 0; uiBlk < 2; uiBlk++ )
  {
    ParIdx16x8      eParIdx     = ( uiBlk ? PART_16x8_1 : PART_16x8_0 );
    UInt            uiCost  [3] = { MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX }, uiCostTest;
    Int             iRefIdx [2] = { 0, 0 }, iRefIdxBi[2], iRefIdxTest;
    UInt            uiMbBits[3] = { 3, 0, 0 };
    UInt            uiBits  [3] = { 0, 0, 0 }, uiBitsTest;
    Mv              cMv[2], cMvBi[2], cMvPred[2][33], cMvLastEst[2][33], cMvd[2];
    YuvMbBuffer  cYuvMbBuffer[2];
    Frame*       pcRefFrame;


    Bool            bBLPred   [2] = { false, false };
    Bool            bBLPredBi [2] = { false, false };
    Int             iBLRefIdx [2] = { -1, -1 };
    Mv              cBLMvPred [2], cBLMvLastEst[2];

    if( pcMbDataAccessBase )
    {
      if( pcMbDataAccessBase->getMbMotionData( LIST_0 ).getRefIdx( eParIdx ) >  0                           &&
          pcMbDataAccessBase->getMbMotionData( LIST_0 ).getRefIdx( eParIdx ) <= (Int)rcRefFrameList0.getActive()   )
      {
        iBLRefIdx [0] = pcMbDataAccessBase->getMbMotionData( LIST_0 ).getRefIdx ( eParIdx );
        cBLMvPred [0] = pcMbDataAccessBase->getMbMotionData( LIST_0 ).getMv     ( eParIdx );
      }
      if( pcMbDataAccessBase->getMbMotionData( LIST_1 ).getRefIdx( eParIdx ) >  0                           &&
          pcMbDataAccessBase->getMbMotionData( LIST_1 ).getRefIdx( eParIdx ) <= (Int)rcRefFrameList1.getActive()   )
      {
        iBLRefIdx [1] = pcMbDataAccessBase->getMbMotionData( LIST_1 ).getRefIdx ( eParIdx );
        cBLMvPred [1] = pcMbDataAccessBase->getMbMotionData( LIST_1 ).getMv     ( eParIdx );
      }
    }

    UInt  uiBasePredType = MSYS_UINT_MAX;

    //----- set macroblock bits -----
    if( ! bPSlice )
    {
      ::memcpy( uiMbBits, aauiMbBits[uiBlk][uiLastMode], 3*sizeof(UInt) );
    }

    //===== LIST 0 PREDICTION ======
    for( iRefIdxTest = 1; iRefIdxTest <= (Int)rcRefFrameList0.getActive(); iRefIdxTest++ )
    {
      rpcMbTempData->getMbDataAccess().getMvPredictor   ( cMvPred[0][iRefIdxTest], iRefIdxTest,
                                                          LIST_0, eParIdx );
      uiBitsTest                  = ( uiBasePredType == 0 ? 0 : uiMbBits[0] ) + getRefIdxBits( iRefIdxTest, rcRefFrameList0 );
      cMvLastEst[0][iRefIdxTest]  = cMvPred [0][iRefIdxTest];
      pcRefFrame                  = rcRefFrameList0[iRefIdxTest];
      Bool bQPel = ( bQPelRefinementOnly &&
                     iRefIdxTest == iBLRefIdx[0] &&
                     cMvPred[0][iRefIdxTest] == cBLMvPred[0] );
      RNOK( m_pcMotionEstimation->estimateBlockWithStart(  *rpcMbTempData, *pcRefFrame,
                                                          cMvLastEst[0][iRefIdxTest],
                                                          cMvPred   [0][iRefIdxTest],
                                                          uiBitsTest, uiCostTest,
                                                          eParIdx, MODE_16x8, bQPel, 0,
                                                          &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdxTest, rpcMbTempData->getFieldFlag()) ) );
      if( uiCostTest < uiCost[0] )
      {
        bBLPred [0] = false;
        iRefIdx [0] = iRefIdxTest;
        cMv     [0] = cMvLastEst[0][iRefIdxTest];
        uiBits  [0] = uiBitsTest;
        uiCost  [0] = uiCostTest;
      }

      if( iRefIdxTest == iBLRefIdx[0] )
      {
        uiBitsTest      = ( uiBasePredType == 0 ? 0 : uiMbBits[0] );
        cBLMvLastEst[0] = cBLMvPred [0];

        rpcMbTempData->getMbDataAccess().setMvPredictorsBL( cBLMvPred[0], LIST_0, eParIdx );
        RNOK( m_pcMotionEstimation->estimateBlockWithStart( *rpcMbTempData, *pcRefFrame,
                                                            cBLMvLastEst[0],
                                                            cBLMvPred   [0],
                                                            uiBitsTest, uiCostTest,
                                                            eParIdx, MODE_16x8, bQPelRefinementOnly, 0,
                                                            &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdxTest, rpcMbTempData->getFieldFlag()) ) );
        if( uiCostTest < uiCost[0] )
        {
          bBLPred [0] = true;
          iRefIdx [0] = iRefIdxTest;
          cMv     [0] = cBLMvLastEst[0];
          uiBits  [0] = uiBitsTest;
          uiCost  [0] = uiCostTest;
        }
      }
    }


    //===== LIST 1 PREDICTION =====
    for( iRefIdxTest = 1; iRefIdxTest <= (Int)rcRefFrameList1.getActive(); iRefIdxTest++ )
    {
      rpcMbTempData->getMbDataAccess().getMvPredictor   ( cMvPred[1][iRefIdxTest], iRefIdxTest,
                                                          LIST_1, eParIdx );
      uiBitsTest                  = ( uiBasePredType == 1 ? 0 : uiMbBits[1] ) + getRefIdxBits( iRefIdxTest, rcRefFrameList1 );
      cMvLastEst[1][iRefIdxTest]  = cMvPred [1][iRefIdxTest];
      pcRefFrame                  = rcRefFrameList1[iRefIdxTest];
      Bool bQPel = ( bQPelRefinementOnly &&
                     iRefIdxTest == iBLRefIdx[1] &&
                     cMvPred[1][iRefIdxTest] == cBLMvPred[1] );
      RNOK( m_pcMotionEstimation->estimateBlockWithStart(  *rpcMbTempData, *pcRefFrame,
                                                          cMvLastEst[1][iRefIdxTest],
                                                          cMvPred   [1][iRefIdxTest],
                                                          uiBitsTest, uiCostTest,
                                                          eParIdx, MODE_16x8, bQPel, 0,
                                                          &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdxTest, rpcMbTempData->getFieldFlag()) ) );
      if( uiCostTest < uiCost[1] )
      {
        bBLPred [1] = false;
        iRefIdx [1] = iRefIdxTest;
        cMv     [1] = cMvLastEst[1][iRefIdxTest];
        uiBits  [1] = uiBitsTest;
        uiCost  [1] = uiCostTest;

        RNOK( m_pcMotionEstimation->compensateBlock     ( &cYuvMbBuffer[1],
                                                          eParIdx, MODE_16x8 ) );
      }

      if( iRefIdxTest == iBLRefIdx[1] )
      {
        uiBitsTest      = ( uiBasePredType == 1 ? 0 : uiMbBits[1] );
        cBLMvLastEst[1] = cBLMvPred [1];

        rpcMbTempData->getMbDataAccess().setMvPredictorsBL( cBLMvPred[1], LIST_1, eParIdx );
        RNOK( m_pcMotionEstimation->estimateBlockWithStart( *rpcMbTempData, *pcRefFrame,
                                                            cBLMvLastEst[1],
                                                            cBLMvPred   [1],
                                                            uiBitsTest, uiCostTest,
                                                            eParIdx, MODE_16x8, bQPelRefinementOnly, 0,
                                                            &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdxTest, rpcMbTempData->getFieldFlag()) ) );
        if( uiCostTest < uiCost[1] )
        {
          bBLPred [1] = true;
          iRefIdx [1] = iRefIdxTest;
          cMv     [1] = cBLMvLastEst[1];
          uiBits  [1] = uiBitsTest;
          uiCost  [1] = uiCostTest;

          RNOK( m_pcMotionEstimation->compensateBlock     ( &cYuvMbBuffer[1],
                                                            eParIdx, MODE_16x8 ) );
        }
      }
    }

    //===== BI PREDICTION =====
    if( rcRefFrameList0.getActive() && rcRefFrameList1.getActive() )
    {
      //----- initialize with forward and backward estimation -----
      cMvBi           [0] = cMv[0];
      cMvBi           [1] = cMv[1];
      iRefIdxBi       [0] = iRefIdx [0];
      iRefIdxBi       [1] = iRefIdx [1];
      bBLPredBi       [0] = bBLPred [0];
      bBLPredBi       [1] = bBLPred [1];
      UInt  uiMotBits[2]  = { uiBits[0] - uiMbBits[0], uiBits[1] - uiMbBits[1] };
      uiBits[2]           = uiMbBits[2] + uiMotBits[0] + uiMotBits[1];

      if( ! uiNumMaxIter )
      {
        uiNumMaxIter      = 1;
        uiIterSearchRange = 0;
      }

      //----- iterative search -----
      for( UInt uiIter = 0; uiIter < uiNumMaxIter; uiIter++ )
      {
        Bool  bChanged                = false;
        UInt  uiDir                   = uiIter % 2;
        RefFrameList& rcRefFrameList  = ( uiDir ? rcRefFrameList1 : rcRefFrameList0 );
        BSParams      cBSParams;
        cBSParams.pcAltRefFrame       = ( uiDir ? rcRefFrameList0 : rcRefFrameList1 )[ iRefIdxBi[ 1 - uiDir ] ];
        cBSParams.pcAltRefPelData     = &cYuvMbBuffer[1-uiDir];
        cBSParams.uiL1Search          = uiDir;
        cBSParams.apcWeight[LIST_0]   = &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdxBi[LIST_0], rpcMbTempData->getFieldFlag() );
        cBSParams.apcWeight[LIST_1]   = &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdxBi[LIST_1], rpcMbTempData->getFieldFlag() );

        for( iRefIdxTest = 1; iRefIdxTest <= (Int)rcRefFrameList.getActive(); iRefIdxTest++ )
        {
          uiBitsTest        = ( uiBasePredType == 2 ? 0 : uiMbBits[2] ) + uiMotBits[1-uiDir] + getRefIdxBits( iRefIdxTest, rcRefFrameList );
          pcRefFrame        = rcRefFrameList[iRefIdxTest];

          Bool bQPel = ( bQPelRefinementOnly &&
                         iRefIdxTest == iBLRefIdx[uiDir] &&
                         cMvPred[uiDir][iRefIdxTest] == cBLMvPred[uiDir] );
          RNOK( m_pcMotionEstimation->estimateBlockWithStart(  *rpcMbTempData, *pcRefFrame,
                                                              cMvLastEst[uiDir][iRefIdxTest],
                                                              cMvPred   [uiDir][iRefIdxTest],
                                                              uiBitsTest, uiCostTest,
                                                              eParIdx, MODE_16x8, bQPel,
                                                              uiIterSearchRange, 0, &cBSParams ) );
          if( uiCostTest < uiCost[2] )
          {
            bChanged          = true;
            bBLPredBi [uiDir] = false;
            iRefIdxBi [uiDir] = iRefIdxTest;
            cMvBi     [uiDir] = cMvLastEst[uiDir][iRefIdxTest];
            uiMotBits [uiDir] = uiBitsTest - uiMbBits[2] - uiMotBits[1-uiDir];
            uiBits    [2]     = uiBitsTest;
            uiCost    [2]     = uiCostTest;

            RNOK( m_pcMotionEstimation->compensateBlock     ( &cYuvMbBuffer[uiDir],
                                                              eParIdx, MODE_16x8 ) );
          }

          if( iRefIdxTest == iBLRefIdx[uiDir] )
          {
            uiBitsTest  = ( uiBasePredType == 2 ? 0 : uiMbBits[2] ) + uiMotBits[1-uiDir];
            RNOK( m_pcMotionEstimation->estimateBlockWithStart(  *rpcMbTempData, *pcRefFrame,
                                                                cBLMvLastEst[uiDir],
                                                                cBLMvPred   [uiDir],
                                                                uiBitsTest, uiCostTest,
                                                                eParIdx, MODE_16x8, bQPelRefinementOnly,
                                                                uiIterSearchRange, 0, &cBSParams ) );
            if( uiCostTest < uiCost[2] )
            {
              bChanged          = true;
              bBLPredBi [uiDir] = true;
              iRefIdxBi [uiDir] = iRefIdxTest;
              cMvBi     [uiDir] = cBLMvLastEst[uiDir];
              uiMotBits [uiDir] = uiBitsTest - uiMbBits[2] - uiMotBits[1-uiDir];
              uiBits    [2]     = uiBitsTest;
              uiCost    [2]     = uiCostTest;

              RNOK( m_pcMotionEstimation->compensateBlock     ( &cYuvMbBuffer[uiDir],
                                                                eParIdx, MODE_16x8 ) );
            }
          }
        }

        if( ! bChanged )
        {
          break;
        }
      }

      if( bBiPredOnly )
      {
        uiCost[0] = MSYS_UINT_MAX;
        uiCost[1] = MSYS_UINT_MAX;
      }
    }


    //===== chose parameters =====
    if( uiCost[2] <= uiCost[0] && uiCost[2] <= uiCost[1] )
    {
      //----- bi-directional prediction -----
      uiLastMode  = 2;
      fRdCost    += uiCost    [2];
      iRefIdx [0] = iRefIdxBi [0];
      iRefIdx [1] = iRefIdxBi [1];
      bBLPred [0] = bBLPredBi [0];
      bBLPred [1] = bBLPredBi [1];
      if( bBLPred[0] )  cMvPred[0][iRefIdx[0]] = cBLMvPred[0];
      if( bBLPred[1] )  cMvPred[1][iRefIdx[1]] = cBLMvPred[1];
      cMv     [0] = cMvBi     [0];
      cMv     [1] = cMvBi     [1];
      cMvd    [0] = cMv       [0] - cMvPred[0][iRefIdx[0]];
      cMvd    [1] = cMv       [1] - cMvPred[1][iRefIdx[1]];
    }
    else if( uiCost[0] <= uiCost[1] )
    {
      //----- list 0 prediction -----
      uiLastMode  = 0;
      fRdCost    += uiCost[0];
      iRefIdx [1] = BLOCK_NOT_PREDICTED;
      bBLPred [1] = false;
      if( bBLPred[0] )  cMvPred[0][iRefIdx[0]] = cBLMvPred[0];
      cMv     [1] = Mv::ZeroMv();
      cMvd    [0] = cMv[0] - cMvPred[0][iRefIdx[0]];
    }
    else
    {
      //----- list 1 prediction -----
      uiLastMode  = 1;
      fRdCost    += uiCost[1];
      iRefIdx [0] = BLOCK_NOT_PREDICTED;
      bBLPred [0] = false;
      if( bBLPred[1] )  cMvPred[1][iRefIdx[1]] = cBLMvPred[1];
      cMv     [0] = Mv::ZeroMv();
      cMvd    [1] = cMv[1] - cMvPred[1][iRefIdx[1]];
    }


    //===== set parameters and compare =====
    rpcMbTempData->rdCost() = fRdCost;
    rpcMbTempData->getMbMotionData( LIST_0 ).setRefIdx( iRefIdx [0],  eParIdx );
    rpcMbTempData->getMbMotionData( LIST_0 ).setAllMv ( cMv     [0],  eParIdx );
    rpcMbTempData->getMbMvdData   ( LIST_0 ).setAllMv ( cMvd    [0],  eParIdx );
    rpcMbTempData->getMbMotionData( LIST_1 ).setRefIdx( iRefIdx [1],  eParIdx );
    rpcMbTempData->getMbMotionData( LIST_1 ).setAllMv ( cMv     [1],  eParIdx );
    rpcMbTempData->getMbMvdData   ( LIST_1 ).setAllMv ( cMvd    [1],  eParIdx );

    rpcMbTempData->getMbMotionData( LIST_0 ).setMotPredFlag( bBLPred[0], eParIdx );
    rpcMbTempData->getMbMotionData( LIST_1 ).setMotPredFlag( bBLPred[1], eParIdx );
    ROT( bBLPred[0] && iRefIdx[0] != iBLRefIdx[0] );
    ROT( bBLPred[1] && iRefIdx[1] != iBLRefIdx[1] );
  }

  IntMbTempData* pcMbRefData = rpcMbTempData;

  RNOK( xSetRdCostInterMb   ( *rpcMbTempData, pcMbDataAccessBase, rcRefFrameList0, rcRefFrameList1, 
        false, 0, false, 0, bLowComplexMbEnable ) );


  //JVT-R057 LA-RDO{
  if(m_bLARDOEnable)
  {
	  MbDataAccess&   rcMbDataAccess  = rpcMbTempData->getMbDataAccess();
	  Int distortion1=0,distortion2=0,distortion=0;
	  for( Int n = 0; n <2; n++)
	  {
		  Int iRefIdx[2];
		  Int Tab[2]={0,8};
		  iRefIdx [0]=rpcMbTempData->getMbMotionData(LIST_0).getRefIdx(B4x4Idx(Tab[n]));
		  iRefIdx [1]=rpcMbTempData->getMbMotionData(LIST_1).getRefIdx(B4x4Idx(Tab[n]));
		  Frame* pcRefFrame0 = ( iRefIdx [0] > 0 ? rcRefFrameList0[ iRefIdx [0] ] : NULL );
		  Frame* pcRefFrame1 = ( iRefIdx [1] > 0 ? rcRefFrameList1[ iRefIdx [1] ] : NULL );
		  Int iMvX;
		  Int iMvY;

		  if(pcRefFrame0)
		  {
			  iMvX=rpcMbTempData->getMbMotionData(LIST_0).getMv(B4x4Idx(Tab[n])).getHor();
			  iMvY=rpcMbTempData->getMbMotionData(LIST_0).getMv(B4x4Idx(Tab[n])).getVer();
			  getChannelDistortion(rcMbDataAccess,*pcRefFrame0,&distortion1,iMvX,iMvY,Tab[n]%4,Tab[n]/4,4,2);
		  }
		  if(pcRefFrame1)
		  {
			  iMvX=rpcMbTempData->getMbMotionData(LIST_1).getMv(B4x4Idx(Tab[n])).getHor();
			  iMvY=rpcMbTempData->getMbMotionData(LIST_1).getMv(B4x4Idx(Tab[n])).getVer();
			  getChannelDistortion(rcMbDataAccess,*pcRefFrame1,&distortion2,iMvX,iMvY,Tab[n]%4,Tab[n]/4,4,2);
			  if(pcRefFrame0)
				  distortion1=(Int)(m_dWr0*distortion1+m_dWr0*distortion2);
			  else
				  distortion1=distortion2;
		  }
		  distortion+=distortion1;
	  }
	  setEpRef(distortion);
	  rpcMbTempData->rdCost()+=distortion;
  }
  //JVT-R057 LA-RDO}

  RNOK( xCheckBestEstimation(  rpcMbTempData, rpcMbBestData ) );

  RNOK( xCheckInterMbMode8x8(  rpcMbTempData, rpcMbBestData, pcMbRefData, rcRefFrameList0, rcRefFrameList1, pcMbDataAccessBase ) );

  return Err::m_nOK;
}





ErrVal
MbEncoder::xEstimateMb8x16 ( IntMbTempData*&  rpcMbTempData,
                             IntMbTempData*&  rpcMbBestData,
                             RefFrameList&    rcRefFrameList0,
                             RefFrameList&    rcRefFrameList1,
                             Bool             bBiPredOnly,
                             UInt             uiNumMaxIter,
                             UInt             uiIterSearchRange,
                             Bool             bQPelRefinementOnly,
                             MbDataAccess*    pcMbDataAccessBase,
                             Bool             bResidualPred )
{
  ROF( rcRefFrameList0.getActive() <= 32 );
  ROF( rcRefFrameList1.getActive() <= 32 );
  ROF( rcRefFrameList0.getActive() );
  ROF( ! bBiPredOnly || rcRefFrameList1.getActive() );

  const UInt aauiMbBits[2][3][3] = { { {0,2,3}, {0,0,0}, {0,0,0} } , { {5,7,7}, {7-2,7-2,9-2}, {9-3,9-3,9-3} } };

  rpcMbTempData->clear();
  rpcMbTempData->setMbMode( MODE_8x16 );
  rpcMbTempData->setBLSkipFlag( false );
  rpcMbTempData->setResidualPredFlag( bResidualPred );

  Bool   bPSlice       = rpcMbTempData->getMbDataAccess().getSH().isPSlice();
  Double fRdCost       = 0;
  UInt   uiLastMode    = 0;

  //JVT-V079 Low-complexity MB mode decision
  Bool bLowComplexMbEnable = m_bLowComplexMbEnable[rpcMbTempData->getSH().getDependencyId()];
  
  for( UInt   uiBlk = 0; uiBlk < 2; uiBlk++ )
  {
    ParIdx8x16      eParIdx     = ( uiBlk ? PART_8x16_1 : PART_8x16_0 );
    UInt            uiCost  [3] = { MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX }, uiCostTest;
    Int             iRefIdx [2] = { 0, 0 }, iRefIdxBi[2], iRefIdxTest;
    UInt            uiMbBits[3] = { 3, 0, 0 };
    UInt            uiBits  [3] = { 0, 0, 0 }, uiBitsTest;
    Mv              cMv[2], cMvBi[2], cMvPred[2][33], cMvLastEst[2][33], cMvd[2];
    YuvMbBuffer  cYuvMbBuffer[2];
    Frame*       pcRefFrame;


    Bool            bBLPred   [2] = { false, false };
    Bool            bBLPredBi [2] = { false, false };
    Int             iBLRefIdx [2] = { -1, -1 };
    Mv              cBLMvPred [2], cBLMvLastEst[2];

    if( pcMbDataAccessBase )
    {
      if( pcMbDataAccessBase->getMbMotionData( LIST_0 ).getRefIdx( eParIdx ) >  0                           &&
          pcMbDataAccessBase->getMbMotionData( LIST_0 ).getRefIdx( eParIdx ) <= (Int)rcRefFrameList0.getActive()   )
      {
        iBLRefIdx [0] = pcMbDataAccessBase->getMbMotionData( LIST_0 ).getRefIdx ( eParIdx );
        cBLMvPred [0] = pcMbDataAccessBase->getMbMotionData( LIST_0 ).getMv     ( eParIdx );
      }
      if( pcMbDataAccessBase->getMbMotionData( LIST_1 ).getRefIdx( eParIdx ) >  0                           &&
          pcMbDataAccessBase->getMbMotionData( LIST_1 ).getRefIdx( eParIdx ) <= (Int)rcRefFrameList1.getActive()   )
      {
        iBLRefIdx [1] = pcMbDataAccessBase->getMbMotionData( LIST_1 ).getRefIdx ( eParIdx );
        cBLMvPred [1] = pcMbDataAccessBase->getMbMotionData( LIST_1 ).getMv     ( eParIdx );
      }
    }

    UInt  uiBasePredType = MSYS_UINT_MAX;

    //----- set macroblock bits -----
    if( ! bPSlice )
    {
      ::memcpy( uiMbBits, aauiMbBits[uiBlk][uiLastMode], 3*sizeof(UInt) );
    }

    //===== LIST 0 PREDICTION ======
    for( iRefIdxTest = 1; iRefIdxTest <= (Int)rcRefFrameList0.getActive(); iRefIdxTest++ )
    {
      rpcMbTempData->getMbDataAccess().getMvPredictor   ( cMvPred[0][iRefIdxTest], iRefIdxTest,
                                                          LIST_0, eParIdx );
      uiBitsTest                  = ( uiBasePredType == 0 ? 0 : uiMbBits[0] ) + getRefIdxBits( iRefIdxTest, rcRefFrameList0 );
      cMvLastEst[0][iRefIdxTest]  = cMvPred [0][iRefIdxTest];
      pcRefFrame                  = rcRefFrameList0[iRefIdxTest];
      Bool bQPel = ( bQPelRefinementOnly &&
                     iRefIdxTest == iBLRefIdx[0] &&
                     cMvPred[0][iRefIdxTest] == cBLMvPred[0] );
      RNOK( m_pcMotionEstimation->estimateBlockWithStart( *rpcMbTempData, *pcRefFrame,
                                                          cMvLastEst[0][iRefIdxTest],
                                                          cMvPred   [0][iRefIdxTest],
                                                          uiBitsTest, uiCostTest,
                                                          eParIdx, MODE_8x16, bQPel, 0,
                                                          &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdxTest, rpcMbTempData->getFieldFlag()) ) );
      if( uiCostTest < uiCost[0] )
      {
        bBLPred [0] = false;
        iRefIdx [0] = iRefIdxTest;
        cMv     [0] = cMvLastEst[0][iRefIdxTest];
        uiBits  [0] = uiBitsTest;
        uiCost  [0] = uiCostTest;
      }

      if( iRefIdxTest == iBLRefIdx[0] )
      {
        uiBitsTest      = ( uiBasePredType == 0 ? 0 : uiMbBits[0] );
        cBLMvLastEst[0] = cBLMvPred [0];

        rpcMbTempData->getMbDataAccess().setMvPredictorsBL( cBLMvPred[0], LIST_0, eParIdx );
        RNOK( m_pcMotionEstimation->estimateBlockWithStart( *rpcMbTempData, *pcRefFrame,
                                                            cBLMvLastEst[0],
                                                            cBLMvPred   [0],
                                                            uiBitsTest, uiCostTest,
                                                            eParIdx, MODE_8x16, bQPelRefinementOnly, 0,
                                                            &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdxTest, rpcMbTempData->getFieldFlag()) ) );
        if( uiCostTest < uiCost[0] )
        {
          bBLPred [0] = true;
          iRefIdx [0] = iRefIdxTest;
          cMv     [0] = cBLMvLastEst[0];
          uiBits  [0] = uiBitsTest;
          uiCost  [0] = uiCostTest;
        }
      }
    }


    //===== LIST 1 PREDICTION =====
    for( iRefIdxTest = 1; iRefIdxTest <= (Int)rcRefFrameList1.getActive(); iRefIdxTest++ )
    {
      rpcMbTempData->getMbDataAccess().getMvPredictor   ( cMvPred[1][iRefIdxTest], iRefIdxTest,
                                                          LIST_1, eParIdx );
      uiBitsTest                  = ( uiBasePredType == 1 ? 0 : uiMbBits[1] ) + getRefIdxBits( iRefIdxTest, rcRefFrameList1 );
      cMvLastEst[1][iRefIdxTest]  = cMvPred [1][iRefIdxTest];
      pcRefFrame                  = rcRefFrameList1[iRefIdxTest];
      Bool bQPel = ( bQPelRefinementOnly &&
                     iRefIdxTest == iBLRefIdx[1] &&
                     cMvPred[1][iRefIdxTest] == cBLMvPred[1] );
      RNOK( m_pcMotionEstimation->estimateBlockWithStart(  *rpcMbTempData, *pcRefFrame,
                                                          cMvLastEst[1][iRefIdxTest],
                                                          cMvPred   [1][iRefIdxTest],
                                                          uiBitsTest, uiCostTest,
                                                          eParIdx, MODE_8x16, bQPel, 0,
                                                          &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdxTest, rpcMbTempData->getFieldFlag()) ) );
      if( uiCostTest < uiCost[1] )
      {
        bBLPred [1] = false;
        iRefIdx [1] = iRefIdxTest;
        cMv     [1] = cMvLastEst[1][iRefIdxTest];
        uiBits  [1] = uiBitsTest;
        uiCost  [1] = uiCostTest;

        RNOK( m_pcMotionEstimation->compensateBlock     ( &cYuvMbBuffer[1],
                                                          eParIdx, MODE_8x16 ) );
      }

      if( iRefIdxTest == iBLRefIdx[1] )
      {
        uiBitsTest      = ( uiBasePredType == 1 ? 0 : uiMbBits[1] );
        cBLMvLastEst[1] = cBLMvPred [1];

        rpcMbTempData->getMbDataAccess().setMvPredictorsBL( cBLMvPred[1], LIST_1, eParIdx );
        RNOK( m_pcMotionEstimation->estimateBlockWithStart(  *rpcMbTempData, *pcRefFrame,
                                                            cBLMvLastEst[1],
                                                            cBLMvPred   [1],
                                                            uiBitsTest, uiCostTest,
                                                            eParIdx, MODE_8x16, bQPelRefinementOnly, 0,
                                                            &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdxTest, rpcMbTempData->getFieldFlag()) ) );
        if( uiCostTest < uiCost[1] )
        {
          bBLPred [1] = true;
          iRefIdx [1] = iRefIdxTest;
          cMv     [1] = cBLMvLastEst[1];
          uiBits  [1] = uiBitsTest;
          uiCost  [1] = uiCostTest;

          RNOK( m_pcMotionEstimation->compensateBlock     ( &cYuvMbBuffer[1],
                                                            eParIdx, MODE_8x16 ) );
        }
      }
    }


    //===== BI PREDICTION =====
    if( rcRefFrameList0.getActive() && rcRefFrameList1.getActive() )
    {
      //----- initialize with forward and backward estimation -----
      iRefIdxBi       [0] = iRefIdx [0];
      iRefIdxBi       [1] = iRefIdx [1];
      bBLPredBi       [0] = bBLPred [0];
      bBLPredBi       [1] = bBLPred [1];
      cMvBi           [0] = cMv     [0];
      cMvBi           [1] = cMv     [1];
      UInt  uiMotBits[2]  = { uiBits[0] - uiMbBits[0], uiBits[1] - uiMbBits[1] };
      uiBits[2]           = uiMbBits[2] + uiMotBits[0] + uiMotBits[1];

      if( ! uiNumMaxIter )
      {
        uiNumMaxIter      = 1;
        uiIterSearchRange = 0;
      }

      //----- iterative search -----
      for( UInt uiIter = 0; uiIter < uiNumMaxIter; uiIter++ )
      {
        Bool          bChanged        = false;
        UInt          uiDir           = uiIter % 2;
        RefFrameList& rcRefFrameList  = ( uiDir ? rcRefFrameList1 : rcRefFrameList0 );
        BSParams      cBSParams;
        cBSParams.pcAltRefFrame       = ( uiDir ? rcRefFrameList0 : rcRefFrameList1 )[ iRefIdxBi[ 1 - uiDir ] ];
        cBSParams.pcAltRefPelData     = &cYuvMbBuffer[1-uiDir];
        cBSParams.uiL1Search          = uiDir;
        cBSParams.apcWeight[LIST_0]   = &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdxBi[LIST_0], rpcMbTempData->getFieldFlag() );
        cBSParams.apcWeight[LIST_1]   = &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdxBi[LIST_1], rpcMbTempData->getFieldFlag() );

        for( iRefIdxTest = 1; iRefIdxTest <= (Int)rcRefFrameList.getActive(); iRefIdxTest++ )
        {
          uiBitsTest  = ( uiBasePredType == 2 ? 0 : uiMbBits[2] ) + uiMotBits[1-uiDir] + getRefIdxBits( iRefIdxTest, rcRefFrameList );
          pcRefFrame  = rcRefFrameList[iRefIdxTest];

          Bool bQPel = ( bQPelRefinementOnly &&
                         iRefIdxTest == iBLRefIdx[uiDir] &&
                         cMvPred[uiDir][iRefIdxTest] == cBLMvPred[uiDir] );
          RNOK( m_pcMotionEstimation->estimateBlockWithStart(  *rpcMbTempData, *pcRefFrame,
                                                              cMvLastEst[uiDir][iRefIdxTest],
                                                              cMvPred   [uiDir][iRefIdxTest],
                                                              uiBitsTest, uiCostTest,
                                                              eParIdx, MODE_8x16, bQPel,
                                                              uiIterSearchRange, 0, &cBSParams ) );
          if( uiCostTest < uiCost[2] )
          {
            bChanged          = true;
            bBLPredBi [uiDir] = false;
            iRefIdxBi [uiDir] = iRefIdxTest;
            cMvBi     [uiDir] = cMvLastEst[uiDir][iRefIdxTest];
            uiMotBits [uiDir] = uiBitsTest - uiMbBits[2] - uiMotBits[1-uiDir];
            uiBits    [2]     = uiBitsTest;
            uiCost    [2]     = uiCostTest;

            RNOK( m_pcMotionEstimation->compensateBlock     ( &cYuvMbBuffer[uiDir],
                                                              eParIdx, MODE_8x16 ) );
          }

          if( iRefIdxTest == iBLRefIdx[uiDir] )
          {
            uiBitsTest  = ( uiBasePredType == 2 ? 0 : uiMbBits[2] ) + uiMotBits[1-uiDir];
            RNOK( m_pcMotionEstimation->estimateBlockWithStart( *rpcMbTempData, *pcRefFrame,
                                                                cBLMvLastEst[uiDir],
                                                                cBLMvPred   [uiDir],
                                                                uiBitsTest, uiCostTest,
                                                                eParIdx, MODE_8x16, bQPelRefinementOnly,
                                                                uiIterSearchRange, 0, &cBSParams ) );
            if( uiCostTest < uiCost[2] )
            {
              bChanged          = true;
              bBLPredBi [uiDir] = true;
              iRefIdxBi [uiDir] = iRefIdxTest;
              cMvBi     [uiDir] = cBLMvLastEst[uiDir];
              uiMotBits [uiDir] = uiBitsTest - uiMbBits[2] - uiMotBits[1-uiDir];
              uiBits    [2]     = uiBitsTest;
              uiCost    [2]     = uiCostTest;

              RNOK( m_pcMotionEstimation->compensateBlock     ( &cYuvMbBuffer[uiDir],
                                                                eParIdx, MODE_8x16 ) );
            }
          }
        }

        if( ! bChanged )
        {
          break;
        }
      }

      if( bBiPredOnly )
      {
        uiCost[0] = MSYS_UINT_MAX;
        uiCost[1] = MSYS_UINT_MAX;
      }
    }


    //===== chose parameters =====
    if( uiCost[2] <= uiCost[0] && uiCost[2] <= uiCost[1] )
    {
      //----- bi-directional prediction -----
      uiLastMode  = 2;
      fRdCost    += uiCost    [2];
      iRefIdx [0] = iRefIdxBi [0];
      iRefIdx [1] = iRefIdxBi [1];
      bBLPred [0] = bBLPredBi [0];
      bBLPred [1] = bBLPredBi [1];
      if( bBLPred[0] )  cMvPred[0][iRefIdx[0]] = cBLMvPred[0];
      if( bBLPred[1] )  cMvPred[1][iRefIdx[1]] = cBLMvPred[1];
      cMv     [0] = cMvBi     [0];
      cMv     [1] = cMvBi     [1];
      cMvd    [0] = cMv       [0] - cMvPred[0][iRefIdx[0]];
      cMvd    [1] = cMv       [1] - cMvPred[1][iRefIdx[1]];
    }
    else if( uiCost[0] <= uiCost[1] )
    {
      //----- list 0 prediction -----
      uiLastMode  = 0;
      fRdCost    += uiCost[0];
      iRefIdx [1] = BLOCK_NOT_PREDICTED;
      bBLPred [1] = false;
      if( bBLPred[0] )  cMvPred[0][iRefIdx[0]] = cBLMvPred[0];
      cMv     [1] = Mv::ZeroMv();
      cMvd    [0] = cMv[0] - cMvPred[0][iRefIdx[0]];
    }
    else
    {
      //----- list 1 prediction -----
      uiLastMode  = 1;
      fRdCost    += uiCost[1];
      iRefIdx [0] = BLOCK_NOT_PREDICTED;
      bBLPred [0] = false;
      if( bBLPred[1] )  cMvPred[1][iRefIdx[1]] = cBLMvPred[1];
      cMv     [0] = Mv::ZeroMv();
      cMvd    [1] = cMv[1] - cMvPred[1][iRefIdx[1]];
    }


    //===== set parameters and compare =====
    rpcMbTempData->rdCost() = fRdCost;
    rpcMbTempData->getMbMotionData( LIST_0 ).setRefIdx( iRefIdx [0],  eParIdx );
    rpcMbTempData->getMbMotionData( LIST_0 ).setAllMv ( cMv     [0],  eParIdx );
    rpcMbTempData->getMbMvdData   ( LIST_0 ).setAllMv ( cMvd    [0],  eParIdx );
    rpcMbTempData->getMbMotionData( LIST_1 ).setRefIdx( iRefIdx [1],  eParIdx );
    rpcMbTempData->getMbMotionData( LIST_1 ).setAllMv ( cMv     [1],  eParIdx );
    rpcMbTempData->getMbMvdData   ( LIST_1 ).setAllMv ( cMvd    [1],  eParIdx );

    rpcMbTempData->getMbMotionData( LIST_0 ).setMotPredFlag( bBLPred[0], eParIdx );
    rpcMbTempData->getMbMotionData( LIST_1 ).setMotPredFlag( bBLPred[1], eParIdx );
    ROT( bBLPred[0] && iRefIdx[0] != iBLRefIdx[0] );
    ROT( bBLPred[1] && iRefIdx[1] != iBLRefIdx[1] );
  }

  IntMbTempData* pcMbRefData = rpcMbTempData;

  RNOK( xSetRdCostInterMb   ( *rpcMbTempData, pcMbDataAccessBase, rcRefFrameList0, rcRefFrameList1, 
        false, 0, false, 0, bLowComplexMbEnable ) );


  //JVT-R057 LA-RDO{
  if(m_bLARDOEnable)
  {
	  MbDataAccess&   rcMbDataAccess  = rpcMbTempData->getMbDataAccess();
	  Int distortion1=0,distortion2,distortion=0;
	  for( Int n = 0; n <2; n++)
	  {
		  Int iRefIdx[2];
		  Int Tab[2]={0,2};
		  iRefIdx [0]=rpcMbTempData->getMbMotionData(LIST_0).getRefIdx(B4x4Idx(Tab[n]));
		  iRefIdx [1]=rpcMbTempData->getMbMotionData(LIST_1).getRefIdx(B4x4Idx(Tab[n]));
		  Frame* pcRefFrame0 = ( iRefIdx [0] > 0 ? rcRefFrameList0[ iRefIdx [0] ] : NULL );
		  Frame* pcRefFrame1 = ( iRefIdx [1] > 0 ? rcRefFrameList1[ iRefIdx [1] ] : NULL );
		  Int iMvX;
		  Int iMvY;

		  if(pcRefFrame0)
		  {
			  iMvX=rpcMbTempData->getMbMotionData(LIST_0).getMv(B4x4Idx(Tab[n])).getHor();
			  iMvY=rpcMbTempData->getMbMotionData(LIST_0).getMv(B4x4Idx(Tab[n])).getVer();
			  getChannelDistortion(rcMbDataAccess,*pcRefFrame0,&distortion1,iMvX,iMvY,Tab[n]%4,Tab[n]/4,2,4);
		  }
		  if(pcRefFrame1)
		  {
			  iMvX=rpcMbTempData->getMbMotionData(LIST_1).getMv(B4x4Idx(Tab[n])).getHor();
			  iMvY=rpcMbTempData->getMbMotionData(LIST_1).getMv(B4x4Idx(Tab[n])).getVer();
			  getChannelDistortion(rcMbDataAccess,*pcRefFrame1,&distortion2,iMvX,iMvY,Tab[n]%4,Tab[n]/4,2,4);
			  if(pcRefFrame0)
				  distortion1=(Int)(m_dWr0*distortion1+m_dWr0*distortion2);
			  else
				  distortion1=distortion2;
		  }
		  distortion+=distortion1;
	  }
	  setEpRef(distortion);
	  rpcMbTempData->rdCost()+=distortion;
  }
  //JVT-R057 LA-RDO}

  RNOK( xCheckBestEstimation(  rpcMbTempData, rpcMbBestData ) );

  RNOK( xCheckInterMbMode8x8(  rpcMbTempData, rpcMbBestData, pcMbRefData, rcRefFrameList0, rcRefFrameList1, pcMbDataAccessBase ) );

  return Err::m_nOK;
}




ErrVal
MbEncoder::xEstimateMb8x8 ( IntMbTempData*&   rpcMbTempData,
                            IntMbTempData*&   rpcMbBestData,
                            RefFrameList&     rcRefFrameList0,
                            RefFrameList&     rcRefFrameList1,
                            Bool              bBiPredOnly,
                            UInt              uiNumMaxIter,
                            UInt              uiIterSearchRange,
                            Bool              bQPelRefinementOnly,
                            MbDataAccess*     pcMbDataAccessBase,
                            Bool              bResidualPred )
{
  Bool  bPSlice  = rpcMbTempData->getMbDataAccess().getSH().isPSlice();
  UInt  uiBits   = ( ! bPSlice ? 9 : 5 ); // for signalling macroblock mode

  rpcMbTempData->clear    ();
  rpcMbTempData->setMbMode( MODE_8x8 );
  rpcMbTempData->setBLSkipFlag( false );
  rpcMbTempData->setResidualPredFlag( bResidualPred );
  rpcMbTempData->rdCost   () = 0;

  //JVT-V079 Low-complexity MB mode decision
  Bool bLowComplexMbEnable = m_bLowComplexMbEnable[rpcMbTempData->getSH().getDependencyId()];
  
  for( Par8x8 ePar8x8 = B_8x8_0; ePar8x8 < 4; ePar8x8 = Par8x8( ePar8x8 + 1 ), uiBits = 0 )
  {
    ParIdx8x8 aeParIdx8x8[4]  = { PART_8x8_0, PART_8x8_1, PART_8x8_2, PART_8x8_3 };
    ParIdx8x8 eParIdx8x8      = aeParIdx8x8[ ePar8x8 ];

    m_pcIntMbBest8x8Data->clear ();
	//S051{
	if(m_bUseBDir)
	//S051}
    RNOK( xEstimateSubMbDirect  ( ePar8x8, m_pcIntMbTemp8x8Data, m_pcIntMbBest8x8Data, rcRefFrameList0, rcRefFrameList1, false,                                               uiBits,                      pcMbDataAccessBase ) );
    RNOK( xEstimateSubMb8x8     ( ePar8x8, m_pcIntMbTemp8x8Data, m_pcIntMbBest8x8Data, rcRefFrameList0, rcRefFrameList1, false, bBiPredOnly, uiNumMaxIter, uiIterSearchRange, uiBits, bQPelRefinementOnly, pcMbDataAccessBase ) );
    RNOK( xEstimateSubMb8x4   ( ePar8x8, m_pcIntMbTemp8x8Data, m_pcIntMbBest8x8Data, rcRefFrameList0, rcRefFrameList1,        bBiPredOnly, uiNumMaxIter, uiIterSearchRange, uiBits, bQPelRefinementOnly, pcMbDataAccessBase ) );
    RNOK( xEstimateSubMb4x8   ( ePar8x8, m_pcIntMbTemp8x8Data, m_pcIntMbBest8x8Data, rcRefFrameList0, rcRefFrameList1,        bBiPredOnly, uiNumMaxIter, uiIterSearchRange, uiBits, bQPelRefinementOnly, pcMbDataAccessBase ) );
    RNOK( xEstimateSubMb4x4   ( ePar8x8, m_pcIntMbTemp8x8Data, m_pcIntMbBest8x8Data, rcRefFrameList0, rcRefFrameList1,        bBiPredOnly, uiNumMaxIter, uiIterSearchRange, uiBits, bQPelRefinementOnly, pcMbDataAccessBase ) );
  
    //----- store parameters in MbTempData -----
    rpcMbTempData->rdCost()  += m_pcIntMbBest8x8Data->rdCost    ();
    BlkMode eBlkMode          = m_pcIntMbBest8x8Data->getBlkMode( ePar8x8 );
    rpcMbTempData->setBlkMode ( ePar8x8, eBlkMode );

    rpcMbTempData->getMbMotionData( LIST_0 ).copyFrom( m_pcIntMbBest8x8Data->getMbMotionData( LIST_0 ), eParIdx8x8 );
    rpcMbTempData->getMbMotionData( LIST_1 ).copyFrom( m_pcIntMbBest8x8Data->getMbMotionData( LIST_1 ), eParIdx8x8 );
    rpcMbTempData->getMbMvdData   ( LIST_0 ).copyFrom( m_pcIntMbBest8x8Data->getMbMvdData   ( LIST_0 ), eParIdx8x8 );
    rpcMbTempData->getMbMvdData   ( LIST_1 ).copyFrom( m_pcIntMbBest8x8Data->getMbMvdData   ( LIST_1 ), eParIdx8x8 );

    //----- set parameters in MbTemp8x8Data for prediction of next block -----
    m_pcIntMbTemp8x8Data->getMbMotionData( LIST_0 ).copyFrom( rpcMbTempData->getMbMotionData( LIST_0 ), eParIdx8x8 );
    m_pcIntMbTemp8x8Data->getMbMotionData( LIST_1 ).copyFrom( rpcMbTempData->getMbMotionData( LIST_1 ), eParIdx8x8 );

    m_pcIntMbBest8x8Data->getMbMotionData( LIST_0 ).copyFrom( rpcMbTempData->getMbMotionData( LIST_0 ), eParIdx8x8 );
    m_pcIntMbBest8x8Data->getMbMotionData( LIST_1 ).copyFrom( rpcMbTempData->getMbMotionData( LIST_1 ), eParIdx8x8 );
  }

  RNOK( xSetRdCostInterMb   ( *rpcMbTempData, pcMbDataAccessBase, rcRefFrameList0, rcRefFrameList1, 
        false, 0, false, 0, bLowComplexMbEnable ) );



  //JVT-R057 LA-RDO{
  if(m_bLARDOEnable)
  {
	  MbDataAccess&   rcMbDataAccess  = rpcMbTempData->getMbDataAccess();
	  Int distortion1=0,distortion2=0,distortion=0;
	  for( Int n = 0; n <16; n++)
	  {
		  Int iRefIdx[2];
		  iRefIdx [0]=rcMbDataAccess.getMbMotionData(LIST_0).getRefIdx(B4x4Idx(n));
		  iRefIdx [1]=rcMbDataAccess.getMbMotionData(LIST_1).getRefIdx(B4x4Idx(n));
		  Frame* pcRefFrame0 = ( iRefIdx [0] > 0 ? rcRefFrameList0[ iRefIdx [0] ] : NULL );
		  Frame* pcRefFrame1 = ( iRefIdx [1] > 0 ? rcRefFrameList1[ iRefIdx [1] ] : NULL );
		  Int iMvX;
		  Int iMvY;

		  if(pcRefFrame0)
		  {
			  iMvX=rcMbDataAccess.getMbMotionData(LIST_0).getMv(B4x4Idx(n)).getHor();
			  iMvY=rcMbDataAccess.getMbMotionData(LIST_0).getMv(B4x4Idx(n)).getVer();
			  getChannelDistortion(rcMbDataAccess,*pcRefFrame0,&distortion1,iMvX,iMvY,n%4,n/4,1,1);
		  }
		  if(pcRefFrame1)
		  {
			  iMvX=rcMbDataAccess.getMbMotionData(LIST_1).getMv(B4x4Idx(n)).getHor();
			  iMvY=rcMbDataAccess.getMbMotionData(LIST_1).getMv(B4x4Idx(n)).getVer();
			  getChannelDistortion(rcMbDataAccess,*pcRefFrame1,&distortion2,iMvX,iMvY,n%4,n/4,1,1);
			  distortion1=(Int)(m_dWr0*distortion1+m_dWr0*distortion2);
		  }
		  distortion+=distortion1;
	  }
	  rpcMbTempData->rdCost()+=distortion;
  }

  //JVT-R057 LA-RDO}

  RNOK( xCheckBestEstimation(  rpcMbTempData, rpcMbBestData ) );

  return Err::m_nOK;
}




ErrVal
MbEncoder::xEstimateMb8x8Frext( IntMbTempData*&   rpcMbTempData,
                                IntMbTempData*&   rpcMbBestData,
                                RefFrameList&     rcRefFrameList0,
                                RefFrameList&     rcRefFrameList1,
                                Bool              bBiPredOnly,
                                UInt              uiNumMaxIter,
                                UInt              uiIterSearchRange,
                                Bool              bQPelRefinementOnly,
                                MbDataAccess*     pcMbDataAccessBase,
                                Bool              bResidualPred )
{
  ROTRS( !rpcMbTempData->getSH().getPPS().getTransform8x8ModeFlag(), Err::m_nOK );


  Bool  bPSlice  = rpcMbTempData->getMbDataAccess().getSH().isPSlice();
  UInt  uiBits   = ( ! bPSlice ? 9 : 5 ); // for signalling macroblock mode

  rpcMbTempData->clear    ();
  rpcMbTempData->setMbMode( MODE_8x8 );
  rpcMbTempData->setBLSkipFlag( false );
  rpcMbTempData->setResidualPredFlag( bResidualPred );
  rpcMbTempData->rdCost   () = 0;

  for( Par8x8 ePar8x8 = B_8x8_0; ePar8x8 < 4; ePar8x8 = Par8x8( ePar8x8 + 1 ), uiBits = 0 )
  {
    ParIdx8x8 aeParIdx8x8[4]  = { PART_8x8_0, PART_8x8_1, PART_8x8_2, PART_8x8_3 };
    ParIdx8x8 eParIdx8x8      = aeParIdx8x8[ ePar8x8 ];

    m_pcIntMbBest8x8Data->clear ();
	//S051{
	if(m_bUseBDir)
	//S051}
    RNOK( xEstimateSubMbDirect  ( ePar8x8, m_pcIntMbTemp8x8Data, m_pcIntMbBest8x8Data, rcRefFrameList0, rcRefFrameList1, true,                                               uiBits,                      pcMbDataAccessBase ) );
    RNOK( xEstimateSubMb8x8     ( ePar8x8, m_pcIntMbTemp8x8Data, m_pcIntMbBest8x8Data, rcRefFrameList0, rcRefFrameList1, true, bBiPredOnly, uiNumMaxIter, uiIterSearchRange, uiBits, bQPelRefinementOnly, pcMbDataAccessBase ) );

    //----- store parameters in MbTempData -----
    rpcMbTempData->rdCost()  += m_pcIntMbBest8x8Data->rdCost    ();
    BlkMode eBlkMode          = m_pcIntMbBest8x8Data->getBlkMode( ePar8x8 );
    rpcMbTempData->setBlkMode ( ePar8x8, eBlkMode );

    rpcMbTempData->getMbMotionData( LIST_0 ).copyFrom( m_pcIntMbBest8x8Data->getMbMotionData( LIST_0 ), eParIdx8x8 );
    rpcMbTempData->getMbMotionData( LIST_1 ).copyFrom( m_pcIntMbBest8x8Data->getMbMotionData( LIST_1 ), eParIdx8x8 );
    rpcMbTempData->getMbMvdData   ( LIST_0 ).copyFrom( m_pcIntMbBest8x8Data->getMbMvdData   ( LIST_0 ), eParIdx8x8 );
    rpcMbTempData->getMbMvdData   ( LIST_1 ).copyFrom( m_pcIntMbBest8x8Data->getMbMvdData   ( LIST_1 ), eParIdx8x8 );

    //----- set parameters in MbTemp8x8Data for prediction of next block -----
    m_pcIntMbTemp8x8Data->getMbMotionData( LIST_0 ).copyFrom( rpcMbTempData->getMbMotionData( LIST_0 ), eParIdx8x8 );
    m_pcIntMbTemp8x8Data->getMbMotionData( LIST_1 ).copyFrom( rpcMbTempData->getMbMotionData( LIST_1 ), eParIdx8x8 );

    m_pcIntMbBest8x8Data->getMbMotionData( LIST_0 ).copyFrom( rpcMbTempData->getMbMotionData( LIST_0 ), eParIdx8x8 );
    m_pcIntMbBest8x8Data->getMbMotionData( LIST_1 ).copyFrom( rpcMbTempData->getMbMotionData( LIST_1 ), eParIdx8x8 );
  }

  IntMbTempData* pcMbRefData = rpcMbTempData;


  //JVT-R057 LA-RDO{
  if(m_bLARDOEnable)
  {
	  MbDataAccess&   rcMbDataAccess  = rpcMbTempData->getMbDataAccess();
	  Int distortion1=0,distortion2=0,distortion=0;
	  for( Int n = 0; n <16; n++)
	  {
		  Int iRefIdx[2];
		  iRefIdx [0]=rcMbDataAccess.getMbMotionData(LIST_0).getRefIdx(B4x4Idx(n));
		  iRefIdx [1]=rcMbDataAccess.getMbMotionData(LIST_1).getRefIdx(B4x4Idx(n));
		  Frame* pcRefFrame0 = ( iRefIdx [0] > 0 ? rcRefFrameList0[ iRefIdx [0] ] : NULL );
		  Frame* pcRefFrame1 = ( iRefIdx [1] > 0 ? rcRefFrameList1[ iRefIdx [1] ] : NULL );
		  Int iMvX;
		  Int iMvY;

		  if(pcRefFrame0)
		  {
			  iMvX=rcMbDataAccess.getMbMotionData(LIST_0).getMv(B4x4Idx(n)).getHor();
			  iMvY=rcMbDataAccess.getMbMotionData(LIST_0).getMv(B4x4Idx(n)).getVer();
			  getChannelDistortion(rcMbDataAccess,*pcRefFrame0,&distortion1,iMvX,iMvY,n%4,n/4,1,1);
		  }
		  if(pcRefFrame1)
		  {
			  iMvX=rcMbDataAccess.getMbMotionData(LIST_1).getMv(B4x4Idx(n)).getHor();
			  iMvY=rcMbDataAccess.getMbMotionData(LIST_1).getMv(B4x4Idx(n)).getVer();
			  getChannelDistortion(rcMbDataAccess,*pcRefFrame1,&distortion2,iMvX,iMvY,n%4,n/4,1,1);
			  distortion1=(Int)(m_dWr0*distortion1+m_dWr0*distortion2);
		  }
		  distortion+=distortion1;
	  }
	  setEpRef(distortion);
  }
  //JVT-R057 LA-RDO}

  RNOK( xCheckInterMbMode8x8(  rpcMbTempData, rpcMbBestData, pcMbRefData, rcRefFrameList0, rcRefFrameList1, pcMbDataAccessBase ) );

  return Err::m_nOK;
}



ErrVal
MbEncoder::xEstimateSubMbDirect( Par8x8            ePar8x8,
                                 IntMbTempData*&   rpcMbTempData,
                                 IntMbTempData*&   rpcMbBestData,
                                 RefFrameList&     rcRefFrameList0,
                                 RefFrameList&     rcRefFrameList1,
                                 Bool              bTrafo8x8,
                                 UInt              uiAddBits,
                                 MbDataAccess*     pcMbDataAccessBase )
{
  ROFRS( rcRefFrameList0.getActive() && rcRefFrameList1.getActive(), Err::m_nOK );

  ParIdx8x8 aeParIdx8x8 [4] = { PART_8x8_0, PART_8x8_1, PART_8x8_2, PART_8x8_3 };
  ParIdx8x8 eParIdx8x8      = aeParIdx8x8[ ePar8x8 ];

  //JVT-V079 Low-complexity MB mode decision
  Bool bLowComplexMbEnable = m_bLowComplexMbEnable[rpcMbTempData->getSH().getDependencyId()];
  
  Int       iMaxCntL0       = 1;
  Int       iMaxCntL1       = 1;

  for( Int iCntL0 = 0; iCntL0 < iMaxCntL0; iCntL0++ )
  for( Int iCntL1 = 0; iCntL1 < iMaxCntL1; iCntL1++ )
  {
    Int       iRefIdxL0       = 1;
    Int       iRefIdxL1       = 1;
    Mv        cMvPredL0;
    Mv        cMvPredL1;

    MbDataAccess* pTmp = rpcMbTempData->getMbDataAccess().getMbDataAccessBase();
    rpcMbTempData->getMbDataAccess().setMbDataAccessBase( NULL );
    rpcMbTempData->getMbDataAccess().getMvPredictor( cMvPredL0, iRefIdxL0, LIST_0, eParIdx8x8 );
    rpcMbTempData->getMbDataAccess().getMvPredictor( cMvPredL1, iRefIdxL1, LIST_1, eParIdx8x8 );
    rpcMbTempData->getMbDataAccess().setMbDataAccessBase( pTmp );

    if( iCntL0 )
    {
      cMvPredL0 = pcMbDataAccessBase->getMbMotionData( LIST_0 ).getMv     ( eParIdx8x8 );
      iRefIdxL0 = pcMbDataAccessBase->getMbMotionData( LIST_0 ).getRefIdx ( eParIdx8x8 );
    }
    if( iCntL1 )
    {
      cMvPredL1 = pcMbDataAccessBase->getMbMotionData( LIST_1 ).getMv     ( eParIdx8x8 );
      iRefIdxL1 = pcMbDataAccessBase->getMbMotionData( LIST_1 ).getRefIdx ( eParIdx8x8 );
    }

    rpcMbTempData->clear();
    rpcMbTempData->setBlkMode( ePar8x8, BLK_SKIP );
    rpcMbTempData->getMbMotionData( LIST_0 ).setRefIdx( iRefIdxL0,    eParIdx8x8 );
    rpcMbTempData->getMbMotionData( LIST_0 ).setAllMv ( cMvPredL0,    eParIdx8x8 );
    rpcMbTempData->getMbMvdData   ( LIST_0 ).setAllMv ( Mv::ZeroMv(), eParIdx8x8 );
    rpcMbTempData->getMbMotionData( LIST_1 ).setRefIdx( iRefIdxL1,    eParIdx8x8 );
    rpcMbTempData->getMbMotionData( LIST_1 ).setAllMv ( cMvPredL1,    eParIdx8x8 );
    rpcMbTempData->getMbMvdData   ( LIST_1 ).setAllMv ( Mv::ZeroMv(), eParIdx8x8 );

    rpcMbTempData->getMbMotionData( LIST_0 ).setMotPredFlag( iCntL0 > 0, eParIdx8x8 );
    rpcMbTempData->getMbMotionData( LIST_1 ).setMotPredFlag( iCntL1 > 0, eParIdx8x8 );

    if( rpcMbTempData->getSH().isH264AVCCompatible() )
    {
      Bool bOneMv = false;
      Bool bFaultTolerant = false;
      MbDataAccess&  rcMbDataAccess = rpcMbTempData->getMbDataAccess();
      ROFRS( rcMbDataAccess.getMvPredictorDirect( eParIdx8x8, bOneMv, bFaultTolerant, &rcRefFrameList0, &rcRefFrameList1 ), Err::m_nOK );
    }

    RNOK( xSetRdCostInterSubMb( *rpcMbTempData, rcRefFrameList0, rcRefFrameList1, B8x8Idx( ePar8x8 ), bTrafo8x8, 1+uiAddBits, bLowComplexMbEnable ) );


	//JVT-R057 LA-RDO{
	if(m_bLARDOEnable)
	{
		MbDataAccess&   rcMbDataAccess  = rpcMbTempData->getMbDataAccess();
		Int distortion1=0,distortion2=0,distortion=0;
		for( Int n = 0; n <1; n++)
		{
			Int iRefIdx[2];
			iRefIdx [0]=rpcMbTempData->getMbMotionData(LIST_0).getRefIdx(eParIdx8x8);
			iRefIdx [1]=rpcMbTempData->getMbMotionData(LIST_1).getRefIdx(eParIdx8x8);
			Frame* pcRefFrame0 = ( iRefIdx [0] > 0 ? rcRefFrameList0[ iRefIdx [0] ] : NULL );
			Frame* pcRefFrame1 = ( iRefIdx [1] > 0 ? rcRefFrameList1[ iRefIdx [1] ] : NULL );
			Int iMvX;
			Int iMvY;

			if(pcRefFrame0)
			{
				iMvX=rpcMbTempData->getMbMotionData(LIST_0).getMv(eParIdx8x8).getHor();
				iMvY=rpcMbTempData->getMbMotionData(LIST_0).getMv(eParIdx8x8).getVer();
				getChannelDistortion(rcMbDataAccess,*pcRefFrame0,&distortion1,iMvX,iMvY,eParIdx8x8%4,eParIdx8x8/4,2,2);
			}

			if(pcRefFrame1)
			{
				iMvX=rpcMbTempData->getMbMotionData(LIST_1).getMv(eParIdx8x8).getHor();
				iMvY=rpcMbTempData->getMbMotionData(LIST_1).getMv(eParIdx8x8).getVer();
				getChannelDistortion(rcMbDataAccess,*pcRefFrame1,&distortion2,iMvX,iMvY,eParIdx8x8%4,eParIdx8x8/4,2,2);
				if(pcRefFrame0)
					distortion1=(Int)(m_dWr0*distortion1+m_dWr0*distortion2);
				else
					distortion1=distortion2;
			}
			distortion+=distortion1;
		}
		rpcMbTempData->rdCost()+=distortion;

	}
	//JVT-R057 LA-RDO}

    RNOK( xCheckBestEstimation(  rpcMbTempData, rpcMbBestData ) );
  }

  return Err::m_nOK;
}

ErrVal
MbEncoder::xEstimateSubMb8x8( Par8x8            ePar8x8,
                              IntMbTempData*&   rpcMbTempData,
                              IntMbTempData*&   rpcMbBestData,
                              RefFrameList&     rcRefFrameList0,
                              RefFrameList&     rcRefFrameList1,
                              Bool              bTrafo8x8,
                              Bool              bBiPredOnly,
                              UInt              uiNumMaxIter,
                              UInt              uiIterSearchRange,
                              UInt              uiAddBits,
                              Bool              bQPelRefinementOnly,
                              MbDataAccess*     pcMbDataAccessBase )
{
  ROF( rcRefFrameList0.getActive() <= 32 );
  ROF( rcRefFrameList1.getActive() <= 32 );
  ROF( rcRefFrameList0.getActive() );
  ROF( ! bBiPredOnly || rcRefFrameList1.getActive() );

  //JVT-V079 Low-complexity MB mode decision
  Bool bLowComplexMbEnable = m_bLowComplexMbEnable[rpcMbTempData->getSH().getDependencyId()];

  Bool            bPSlice         = rpcMbTempData->getMbDataAccess().getSH().isPSlice();
  Double          fRdCost         = 0;
  UInt            uiSubMbBits     = 0;
  ParIdx8x8       aeParIdx8x8 [4] = { PART_8x8_0, PART_8x8_1, PART_8x8_2, PART_8x8_3 };
  ParIdx8x8       eParIdx8x8      = aeParIdx8x8[ ePar8x8 ];
  UInt            uiCost      [3] = { MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX }, uiCostTest;
  UInt            uiBlkBits   [3] = { ( ! bPSlice ? 3 : 1 ) + uiAddBits, 3 + uiAddBits, 5 + uiAddBits };
  Int             iRefIdx     [2] = { 0, 0 }, iRefIdxBi[2], iRefIdxTest;
  UInt            uiBits      [3] = { 0, 0, 0 }, uiBitsTest;
  Mv              cMv[2], cMvBi[2], cMvPred[2][33], cMvLastEst[2][33], cMvd[2];
  YuvMbBuffer  cYuvMbBuffer[2];
  Frame*       pcRefFrame;


  Bool            bBLPred   [2] = { false, false };
  Bool            bBLPredBi [2] = { false, false };
  Int             iBLRefIdx [2] = { -1, -1 };
  Mv              cBLMvPred [2], cBLMvLastEst[2];

  if( pcMbDataAccessBase )
  {
    if( pcMbDataAccessBase->getMbMotionData( LIST_0 ).getRefIdx( eParIdx8x8 ) >  0 &&
        pcMbDataAccessBase->getMbMotionData( LIST_0 ).getRefIdx( eParIdx8x8 ) <= (Int)rcRefFrameList0.getActive() )
    {
      iBLRefIdx [0] = pcMbDataAccessBase->getMbMotionData( LIST_0 ).getRefIdx ( eParIdx8x8 );
      cBLMvPred [0] = pcMbDataAccessBase->getMbMotionData( LIST_0 ).getMv     ( eParIdx8x8 );
    }
    if( pcMbDataAccessBase->getMbMotionData( LIST_1 ).getRefIdx( eParIdx8x8 ) >  0 &&
        pcMbDataAccessBase->getMbMotionData( LIST_1 ).getRefIdx( eParIdx8x8 ) <= (Int)rcRefFrameList1.getActive() )
    {
      iBLRefIdx [1] = pcMbDataAccessBase->getMbMotionData( LIST_1 ).getRefIdx ( eParIdx8x8 );
      cBLMvPred [1] = pcMbDataAccessBase->getMbMotionData( LIST_1 ).getMv     ( eParIdx8x8 );
    }
  }

  UInt  uiBasePredType = MSYS_UINT_MAX;
  rpcMbTempData->clear();
  rpcMbTempData->setBlkMode( ePar8x8, BLK_8x8 );

  //===== LIST 0 PREDICTION ======
  for( iRefIdxTest = 1; iRefIdxTest <= (Int)rcRefFrameList0.getActive(); iRefIdxTest++ )
  {
    rpcMbTempData->getMbDataAccess().getMvPredictor   ( cMvPred[0][iRefIdxTest], iRefIdxTest,
                                                        LIST_0, eParIdx8x8);
    uiBitsTest                  = ( uiBasePredType == 0 ? 0 : uiBlkBits[0] ) + getRefIdxBits( iRefIdxTest, rcRefFrameList0 );
    cMvLastEst[0][iRefIdxTest]  = cMvPred   [0][iRefIdxTest];
    pcRefFrame                  = rcRefFrameList0[iRefIdxTest];
    Bool bQPel = ( bQPelRefinementOnly &&
                   iRefIdxTest == iBLRefIdx[0] &&
                   cMvPred[0][iRefIdxTest] == cBLMvPred[0] );
    RNOK( m_pcMotionEstimation->estimateBlockWithStart(  *rpcMbTempData, *pcRefFrame,
                                                        cMvLastEst[0][iRefIdxTest],
                                                        cMvPred   [0][iRefIdxTest],
                                                        uiBitsTest, uiCostTest,
                                                        eParIdx8x8+SPART_8x8, BLK_8x8, bQPel, 0,
                                                        &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdxTest, rpcMbTempData->getFieldFlag()) ) );
    if( uiCostTest < uiCost[0] )
    {
      bBLPred [0] = false;
      iRefIdx [0] = iRefIdxTest;
      cMv     [0] = cMvLastEst[0][iRefIdxTest];
      uiBits  [0] = uiBitsTest;
      uiCost  [0] = uiCostTest;
    }

    if( iRefIdxTest == iBLRefIdx[0] )
    {
      uiBitsTest      = ( uiBasePredType == 0 ? 0 : uiBlkBits[0] );
      cBLMvLastEst[0] = cBLMvPred [0];

      rpcMbTempData->getMbDataAccess().setMvPredictorsBL( cBLMvPred[0], LIST_0, eParIdx8x8 );
      RNOK( m_pcMotionEstimation->estimateBlockWithStart(  *rpcMbTempData, *pcRefFrame,
                                                          cBLMvLastEst[0],
                                                          cBLMvPred   [0],
                                                          uiBitsTest, uiCostTest,
                                                          eParIdx8x8+SPART_8x8, BLK_8x8, bQPelRefinementOnly, 0,
                                                          &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdxTest, rpcMbTempData->getFieldFlag()) ) );
      if( uiCostTest < uiCost[0] )
      {
        bBLPred [0] = true;
        iRefIdx [0] = iRefIdxTest;
        cMv     [0] = cBLMvLastEst[0];
        uiBits  [0] = uiBitsTest;
        uiCost  [0] = uiCostTest;
      }
    }
  }


  //===== LIST 1 PREDICTION =====
  for( iRefIdxTest = 1; iRefIdxTest <= (Int)rcRefFrameList1.getActive(); iRefIdxTest++ )
  {
    rpcMbTempData->getMbDataAccess().getMvPredictor   ( cMvPred[1][iRefIdxTest], iRefIdxTest,
                                                        LIST_1, eParIdx8x8 );
    uiBitsTest                  = ( uiBasePredType == 1 ? 0 : uiBlkBits[1] ) + getRefIdxBits( iRefIdxTest, rcRefFrameList1 );
    cMvLastEst[1][iRefIdxTest]  = cMvPred   [1][iRefIdxTest];
    pcRefFrame                  = rcRefFrameList1[iRefIdxTest];
    Bool bQPel = ( bQPelRefinementOnly &&
                   iRefIdxTest == iBLRefIdx[1] &&
                   cMvPred[1][iRefIdxTest] == cBLMvPred[1] );
    RNOK( m_pcMotionEstimation->estimateBlockWithStart(  *rpcMbTempData, *pcRefFrame,
                                                        cMvLastEst[1][iRefIdxTest],
                                                        cMvPred   [1][iRefIdxTest],
                                                        uiBitsTest, uiCostTest,
                                                        eParIdx8x8+SPART_8x8, BLK_8x8, bQPel, 0,
                                                        &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdxTest, rpcMbTempData->getFieldFlag()) ) );
    if( uiCostTest < uiCost[1] )
    {
      bBLPred [1] = false;
      iRefIdx [1] = iRefIdxTest;
      cMv     [1] = cMvLastEst[1][iRefIdxTest];
      uiBits  [1] = uiBitsTest;
      uiCost  [1] = uiCostTest;

      RNOK( m_pcMotionEstimation->compensateBlock     ( &cYuvMbBuffer[1],
                                                        eParIdx8x8+SPART_8x8, BLK_8x8 ) );
    }

    if( iRefIdxTest == iBLRefIdx[1] )
    {
      uiBitsTest      = ( uiBasePredType == 1 ? 0 : uiBlkBits[1] );
      cBLMvLastEst[1] = cBLMvPred [1];

      rpcMbTempData->getMbDataAccess().setMvPredictorsBL( cBLMvPred[1], LIST_1, eParIdx8x8 );
      RNOK( m_pcMotionEstimation->estimateBlockWithStart(  *rpcMbTempData, *pcRefFrame,
                                                          cBLMvLastEst[1],
                                                          cBLMvPred   [1],
                                                          uiBitsTest, uiCostTest,
                                                          eParIdx8x8+SPART_8x8, BLK_8x8, bQPelRefinementOnly, 0,
                                                          &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdxTest, rpcMbTempData->getFieldFlag()) ) );
      if( uiCostTest < uiCost[1] )
      {
        bBLPred [1] = true;
        iRefIdx [1] = iRefIdxTest;
        cMv     [1] = cBLMvLastEst[1];
        uiBits  [1] = uiBitsTest;
        uiCost  [1] = uiCostTest;

        RNOK( m_pcMotionEstimation->compensateBlock     ( &cYuvMbBuffer[1],
                                                          eParIdx8x8+SPART_8x8, BLK_8x8 ) );
      }
    }
  }

  //===== BI PREDICTION =====
  if( rcRefFrameList0.getActive() && rcRefFrameList1.getActive() )
  {
    //----- initialize with forward and backward estimation -----
    iRefIdxBi [0]       = iRefIdx [0];
    iRefIdxBi [1]       = iRefIdx [1];
    bBLPredBi [0]       = bBLPred [0];
    bBLPredBi [1]       = bBLPred [1];
    cMvBi     [0]       = cMv     [0];
    cMvBi     [1]       = cMv     [1];
    UInt  uiMotBits[2]  = { uiBits[0] - uiBlkBits[0], uiBits[1] - uiBlkBits[1] };
    uiBits[2]           = uiBlkBits[2] + uiMotBits[0] + uiMotBits[1];

    if( ! uiNumMaxIter )
    {
      uiNumMaxIter      = 1;
      uiIterSearchRange = 0;
    }

    //----- iterative search -----
    for( UInt uiIter = 0; uiIter < uiNumMaxIter; uiIter++ )
    {
      Bool          bChanged        = false;
      UInt          uiDir           = uiIter % 2;
      RefFrameList& rcRefFrameList  = ( uiDir ? rcRefFrameList1 : rcRefFrameList0 );
      BSParams      cBSParams;
      cBSParams.pcAltRefFrame       = ( uiDir ? rcRefFrameList0 : rcRefFrameList1 )[ iRefIdxBi[ 1 - uiDir ] ];
      cBSParams.pcAltRefPelData     = &cYuvMbBuffer[1-uiDir];
      cBSParams.uiL1Search          = uiDir;
      cBSParams.apcWeight[LIST_0]   = &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdxBi[LIST_0], rpcMbTempData->getFieldFlag() );
      cBSParams.apcWeight[LIST_1]   = &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdxBi[LIST_1], rpcMbTempData->getFieldFlag());

      for( iRefIdxTest = 1; iRefIdxTest <= (Int)rcRefFrameList.getActive(); iRefIdxTest++ )
      {
        uiBitsTest  = ( uiBasePredType == 2 ? 0 : uiBlkBits[2] ) + uiMotBits[1-uiDir] + getRefIdxBits( iRefIdxTest, rcRefFrameList );
        pcRefFrame  = rcRefFrameList[iRefIdxTest];

        Bool bQPel = ( bQPelRefinementOnly &&
                       iRefIdxTest == iBLRefIdx[uiDir] &&
                       cMvPred[uiDir][iRefIdxTest] == cBLMvPred[uiDir] );
        RNOK( m_pcMotionEstimation->estimateBlockWithStart(  *rpcMbTempData, *pcRefFrame,
                                                            cMvLastEst[uiDir][iRefIdxTest],
                                                            cMvPred   [uiDir][iRefIdxTest],
                                                            uiBitsTest, uiCostTest,
                                                            eParIdx8x8+SPART_8x8, BLK_8x8, bQPel,
                                                            uiIterSearchRange, 0, &cBSParams ) );
        if( uiCostTest < uiCost[2] )
        {
          bChanged          = true;
          bBLPredBi [uiDir] = false;
          iRefIdxBi [uiDir] = iRefIdxTest;
          cMvBi     [uiDir] = cMvLastEst[uiDir][iRefIdxTest];
          uiMotBits [uiDir] = uiBitsTest - uiBlkBits[2] - uiMotBits[1-uiDir];
          uiBits    [2]     = uiBitsTest;
          uiCost    [2]     = uiCostTest;

          RNOK( m_pcMotionEstimation->compensateBlock     ( &cYuvMbBuffer[uiDir],
                                                            eParIdx8x8+SPART_8x8, BLK_8x8 ) );
        }

        if( iRefIdxTest == iBLRefIdx[uiDir] )
        {
          uiBitsTest  = ( uiBasePredType == 2 ? 0 : uiBlkBits[2] ) + uiMotBits[1-uiDir];
          RNOK( m_pcMotionEstimation->estimateBlockWithStart(  *rpcMbTempData, *pcRefFrame,
                                                              cBLMvLastEst[uiDir],
                                                              cBLMvPred   [uiDir],
                                                              uiBitsTest, uiCostTest,
                                                              eParIdx8x8+SPART_8x8, BLK_8x8, bQPelRefinementOnly,
                                                              uiIterSearchRange, 0, &cBSParams ) );
          if( uiCostTest < uiCost[2] )
          {
            bChanged          = true;
            bBLPredBi [uiDir] = true;
            iRefIdxBi [uiDir] = iRefIdxTest;
            cMvBi     [uiDir] = cBLMvLastEst[uiDir];
            uiMotBits [uiDir] = uiBitsTest - uiBlkBits[2] - uiMotBits[1-uiDir];
            uiBits    [2]     = uiBitsTest;
            uiCost    [2]     = uiCostTest;

            RNOK( m_pcMotionEstimation->compensateBlock     ( &cYuvMbBuffer[uiDir],
                                                              eParIdx8x8+SPART_8x8, BLK_8x8 ) );
          }
        }
      }

      if( ! bChanged )
      {
        break;
      }
    }

    if( bBiPredOnly )
    {
      uiCost[0] = MSYS_UINT_MAX;
      uiCost[1] = MSYS_UINT_MAX;
    }
  }


  //===== chose parameters =====
  if( uiCost[2] <= uiCost[0] && uiCost[2] <= uiCost[1] )
  {
    //----- bi-directional prediction -----
    fRdCost     = uiCost    [2];
    uiSubMbBits = uiBits    [2];
    iRefIdx [0] = iRefIdxBi [0];
    iRefIdx [1] = iRefIdxBi [1];
    bBLPred [0] = bBLPredBi [0];
    bBLPred [1] = bBLPredBi [1];
    if( bBLPred[0] )  cMvPred[0][iRefIdx[0]] = cBLMvPred[0];
    if( bBLPred[1] )  cMvPred[1][iRefIdx[1]] = cBLMvPred[1];
    cMv     [0] = cMvBi     [0];
    cMv     [1] = cMvBi     [1];
    cMvd    [0] = cMv       [0] - cMvPred[0][iRefIdx[0]];
    cMvd    [1] = cMv       [1] - cMvPred[1][iRefIdx[1]];
  }
  else if( uiCost[0] <= uiCost[1] )
  {
    //----- list 0 prediction -----
    fRdCost     = uiCost[0];
    uiSubMbBits = uiBits[0];
    iRefIdx [1] = BLOCK_NOT_PREDICTED;
    bBLPred [1] = false;
    if( bBLPred[0] )  cMvPred[0][iRefIdx[0]] = cBLMvPred[0];
    cMv     [1] = Mv::ZeroMv();
    cMvd    [0] = cMv[0] - cMvPred[0][iRefIdx[0]];
  }
  else
  {
    //----- list 1 prediction -----
    fRdCost     = uiCost[1];
    uiSubMbBits = uiBits[1];
    iRefIdx [0] = BLOCK_NOT_PREDICTED;
    bBLPred [0] = false;
    if( bBLPred[1] )  cMvPred[1][iRefIdx[1]] = cBLMvPred[1];
    cMv     [0] = Mv::ZeroMv();
    cMvd    [1] = cMv[1] - cMvPred[1][iRefIdx[1]];
  }


  //===== set parameters and compare =====
  rpcMbTempData->rdCost() = fRdCost;
  rpcMbTempData->getMbMotionData( LIST_0 ).setRefIdx( iRefIdx [0],  eParIdx8x8 );
  rpcMbTempData->getMbMotionData( LIST_0 ).setAllMv ( cMv     [0],  eParIdx8x8 );
  rpcMbTempData->getMbMvdData   ( LIST_0 ).setAllMv ( cMvd    [0],  eParIdx8x8 );
  rpcMbTempData->getMbMotionData( LIST_1 ).setRefIdx( iRefIdx [1],  eParIdx8x8 );
  rpcMbTempData->getMbMotionData( LIST_1 ).setAllMv ( cMv     [1],  eParIdx8x8 );
  rpcMbTempData->getMbMvdData   ( LIST_1 ).setAllMv ( cMvd    [1],  eParIdx8x8 );

  rpcMbTempData->getMbMotionData( LIST_0 ).setMotPredFlag( bBLPred[0], eParIdx8x8 );
  rpcMbTempData->getMbMotionData( LIST_1 ).setMotPredFlag( bBLPred[1], eParIdx8x8 );
  ROT( bBLPred[0] && iRefIdx[0] != iBLRefIdx[0] );
  ROT( bBLPred[1] && iRefIdx[1] != iBLRefIdx[1] );

  RNOK( xSetRdCostInterSubMb( *rpcMbTempData, rcRefFrameList0, rcRefFrameList1, B8x8Idx( ePar8x8 ), bTrafo8x8, uiSubMbBits, bLowComplexMbEnable ) );


  //JVT-R057 LA-RDO{
  if(m_bLARDOEnable)
  {
	  MbDataAccess&   rcMbDataAccess  = rpcMbTempData->getMbDataAccess();
	  Int distortion1=0,distortion2=0,distortion=0;
	  for( Int n = 0; n <1; n++)
	  {
		  iRefIdx [0]=rpcMbTempData->getMbMotionData(LIST_0).getRefIdx(eParIdx8x8);
		  iRefIdx [1]=rpcMbTempData->getMbMotionData(LIST_1).getRefIdx(eParIdx8x8);
		  Frame* pcRefFrame0 = ( iRefIdx [0] > 0 ? rcRefFrameList0[ iRefIdx [0] ] : NULL );
		  Frame* pcRefFrame1 = ( iRefIdx [1] > 0 ? rcRefFrameList1[ iRefIdx [1] ] : NULL );
		  Int iMvX;
		  Int iMvY;

		  if(pcRefFrame0)
		  {
			  iMvX=rpcMbTempData->getMbMotionData(LIST_0).getMv(eParIdx8x8).getHor();
			  iMvY=rpcMbTempData->getMbMotionData(LIST_0).getMv(eParIdx8x8).getVer();
			  getChannelDistortion(rcMbDataAccess,*pcRefFrame0,&distortion1,iMvX,iMvY,eParIdx8x8%4,eParIdx8x8/4,2,2);
		  }

		  if(pcRefFrame1)
		  {
			  iMvX=rpcMbTempData->getMbMotionData(LIST_1).getMv(eParIdx8x8).getHor();
			  iMvY=rpcMbTempData->getMbMotionData(LIST_1).getMv(eParIdx8x8).getVer();
			  getChannelDistortion(rcMbDataAccess,*pcRefFrame1,&distortion2,iMvX,iMvY,eParIdx8x8%4,eParIdx8x8/4,2,2);
			  if(pcRefFrame0)
				  distortion1=(Int)(m_dWr0*distortion1+m_dWr0*distortion2);
			  else
				  distortion1=distortion2;
		  }
		  distortion+=distortion1;
	  }
	  rpcMbTempData->rdCost()+=distortion;
  }
  //JVT-R057 LA-RDO}

  RNOK( xCheckBestEstimation(  rpcMbTempData, rpcMbBestData ) );

  return Err::m_nOK;
}

ErrVal
MbEncoder::xEstimateSubMb8x4( Par8x8            ePar8x8,
                              IntMbTempData*&   rpcMbTempData,
                              IntMbTempData*&   rpcMbBestData,
                              RefFrameList&     rcRefFrameList0,
                              RefFrameList&     rcRefFrameList1,
                              Bool              bBiPredOnly,
                              UInt              uiNumMaxIter,
                              UInt              uiIterSearchRange,
                              UInt              uiAddBits,
                              Bool              bQPelRefinementOnly,
                              MbDataAccess*     pcMbDataAccessBase )
{
  ROF( rcRefFrameList0.getActive() <= 32 );
  ROF( rcRefFrameList1.getActive() <= 32 );
  ROF( rcRefFrameList0.getActive() );
  ROF( ! bBiPredOnly || rcRefFrameList1.getActive() );

  //JVT-V079 Low-complexity MB mode decision
  Bool bLowComplexMbEnable = m_bLowComplexMbEnable[rpcMbTempData->getSH().getDependencyId()];

  Bool            bPSlice         = rpcMbTempData->getMbDataAccess().getSH().isPSlice();
  Double          fRdCost         = 0;
  UInt            uiSubMbBits     = 0;
  SParIdx8x4      aeParIdx8x4 [2] = { SPART_8x4_0, SPART_8x4_1 };
  ParIdx8x8       aeParIdx8x8 [4] = { PART_8x8_0, PART_8x8_1, PART_8x8_2, PART_8x8_3 };
  ParIdx8x8       eParIdx8x8      = aeParIdx8x8[ ePar8x8 ];
  UInt            uiCost      [3] = { MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX }, uiCostTest;
  UInt            uiBlkBits   [3] = { ( ! bPSlice ? 5 : 3 ) + uiAddBits, 5 + uiAddBits, 7 + uiAddBits };
  Int             iRefIdx     [2] = { 0, 0 }, iRefIdxBi[2], iRefIdxTest;
  UInt            uiBits      [3] = { 0, 0, 0 }, uiBitsTest;
  Mv              cMv[2][2], cMvBi[2][2], cMvd[2][2], cMvLastEst[2][33][2], cMvPred[2][33][2], cMvPredBi[2][2];
  YuvMbBuffer  cYuvMbBuffer[2], cTmpYuvMbBuffer;
  Frame*       pcRefFrame;


  Bool            bBLPred   [2] = { false, false };
  Bool            bBLPredBi [2] = { false, false };
  Int             iBLRefIdx [2] = { -1, -1 };
  Mv              cBLMvPred [2][2], cBLMvLastEst[2][2];

  if( pcMbDataAccessBase )
  {
    if( pcMbDataAccessBase->getMbMotionData( LIST_0 ).getRefIdx( eParIdx8x8 ) >  0 &&
        pcMbDataAccessBase->getMbMotionData( LIST_0 ).getRefIdx( eParIdx8x8 ) <= (Int)rcRefFrameList0.getActive() )
    {
      iBLRefIdx [0]    = pcMbDataAccessBase->getMbMotionData( LIST_0 ).getRefIdx ( eParIdx8x8 );
      cBLMvPred [0][0] = pcMbDataAccessBase->getMbMotionData( LIST_0 ).getMv     ( eParIdx8x8, SPART_8x4_0 );
      cBLMvPred [0][1] = pcMbDataAccessBase->getMbMotionData( LIST_0 ).getMv     ( eParIdx8x8, SPART_8x4_1 );
    }
    if( pcMbDataAccessBase->getMbMotionData( LIST_1 ).getRefIdx( eParIdx8x8 ) >  0 &&
        pcMbDataAccessBase->getMbMotionData( LIST_1 ).getRefIdx( eParIdx8x8 ) <= (Int)rcRefFrameList1.getActive() )
    {
      iBLRefIdx [1]    = pcMbDataAccessBase->getMbMotionData( LIST_1 ).getRefIdx ( eParIdx8x8 );
      cBLMvPred [1][0] = pcMbDataAccessBase->getMbMotionData( LIST_1 ).getMv     ( eParIdx8x8, SPART_8x4_0 );
      cBLMvPred [1][1] = pcMbDataAccessBase->getMbMotionData( LIST_1 ).getMv     ( eParIdx8x8, SPART_8x4_1 );
    }
  }

  UInt  uiBasePredType = MSYS_UINT_MAX;

  rpcMbTempData->clear();
  rpcMbTempData->setBlkMode( ePar8x8, BLK_8x4 );


  //===== LIST 0 PREDICTION ======
  for( iRefIdxTest = 1; iRefIdxTest <= (Int)rcRefFrameList0.getActive(); iRefIdxTest++ )
  {
    rpcMbTempData->getMbMotionData( LIST_0 ).setRefIdx( iRefIdxTest, eParIdx8x8 );
    uiBitsTest  = 0;
    uiCostTest  = 0;
    pcRefFrame  = rcRefFrameList0[iRefIdxTest];

    for( UInt uiBlk = 0; uiBlk < 2; uiBlk++ )
    {
      SParIdx8x4  eSubParIdx  = aeParIdx8x4[ uiBlk ];
      UInt        uiTmpBits   = ( uiBlk || uiBasePredType == 0 ? 0 : uiBlkBits[0] + getRefIdxBits( iRefIdxTest, rcRefFrameList0 ) );
      UInt        uiTmpCost;

      rpcMbTempData->getMbDataAccess().getMvPredictor   ( cMvPred[0][iRefIdxTest][uiBlk], iRefIdxTest,
                                                          LIST_0, eParIdx8x8, eSubParIdx );
      cMvLastEst[0][iRefIdxTest][uiBlk] = cMvPred[0][iRefIdxTest][uiBlk];

      Bool bQPel = ( bQPelRefinementOnly &&
                     iRefIdxTest == iBLRefIdx[0] &&
                     cMvPred[0][iRefIdxTest][uiBlk] == cBLMvPred[0][uiBlk] );
      RNOK( m_pcMotionEstimation->estimateBlockWithStart(  *rpcMbTempData, *pcRefFrame,
                                                          cMvLastEst[0][iRefIdxTest][uiBlk],
                                                          cMvPred   [0][iRefIdxTest][uiBlk],
                                                          uiTmpBits, uiTmpCost,
                                                          eParIdx8x8+eSubParIdx, BLK_8x4, bQPel, 0,
                                                          &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdxTest, rpcMbTempData->getFieldFlag()) ) );
      rpcMbTempData->getMbMotionData( LIST_0 ).setAllMv ( cMvLastEst[0][iRefIdxTest][uiBlk], eParIdx8x8, eSubParIdx );

      uiBitsTest  += uiTmpBits;
      uiCostTest  += uiTmpCost;
    }

    if( uiCostTest < uiCost[0] )
    {
      bBLPred [0]     = false;
      iRefIdx [0]     = iRefIdxTest;
      cMv     [0][0]  = cMvLastEst[0][iRefIdxTest][0];
      cMv     [0][1]  = cMvLastEst[0][iRefIdxTest][1];
      uiBits  [0]     = uiBitsTest;
      uiCost  [0]     = uiCostTest;
    }


    if( iRefIdxTest == iBLRefIdx[0] )
    {
      uiBitsTest  = 0;
      uiCostTest  = 0;
      for( UInt uiBlk = 0; uiBlk < 2; uiBlk++ )
      {
        SParIdx8x4  eSubParIdx  = aeParIdx8x4[ uiBlk ];
        UInt        uiTmpBits   = ( uiBlk || uiBasePredType == 0 ? 0 : uiBlkBits[0] );
        UInt        uiTmpCost;
        cBLMvLastEst[0][uiBlk]  = cBLMvPred[0][uiBlk];

        rpcMbTempData->getMbDataAccess().setMvPredictorsBL( cBLMvPred[0][uiBlk], LIST_0, eParIdx8x8, eSubParIdx );
        RNOK( m_pcMotionEstimation->estimateBlockWithStart(  *rpcMbTempData, *pcRefFrame,
                                                            cBLMvLastEst[0][uiBlk],
                                                            cBLMvPred   [0][uiBlk],
                                                            uiTmpBits, uiTmpCost,
                                                            eParIdx8x8+eSubParIdx, BLK_8x4, bQPelRefinementOnly, 0,
                                                            &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdxTest, rpcMbTempData->getFieldFlag()) ) );
        rpcMbTempData->getMbMotionData( LIST_0 ).setAllMv ( cBLMvLastEst[0][uiBlk], eParIdx8x8, eSubParIdx );
        uiBitsTest  += uiTmpBits;
        uiCostTest  += uiTmpCost;
      }

      if( uiCostTest < uiCost[0] )
      {
        bBLPred [0]     = true;
        iRefIdx [0]     = iRefIdxTest;
        cMv     [0][0]  = cBLMvLastEst[0][0];
        cMv     [0][1]  = cBLMvLastEst[0][1];
        uiBits  [0]     = uiBitsTest;
        uiCost  [0]     = uiCostTest;
      }
    }
  }


  //===== LIST 1 PREDICTION =====
  for( iRefIdxTest = 1; iRefIdxTest <= (Int)rcRefFrameList1.getActive(); iRefIdxTest++ )
  {
    rpcMbTempData->getMbMotionData( LIST_1 ).setRefIdx( iRefIdxTest, eParIdx8x8 );
    uiBitsTest  = 0;
    uiCostTest  = 0;
    pcRefFrame  = rcRefFrameList1[iRefIdxTest];

    for( UInt uiBlk = 0; uiBlk < 2; uiBlk++ )
    {
      SParIdx8x4  eSubParIdx  = aeParIdx8x4[ uiBlk ];
      UInt        uiTmpBits   = ( uiBlk || uiBasePredType == 1 ? 0 : uiBlkBits[1] + getRefIdxBits( iRefIdxTest, rcRefFrameList1 ) );
      UInt        uiTmpCost;

      rpcMbTempData->getMbDataAccess().getMvPredictor   ( cMvPred[1][iRefIdxTest][uiBlk], iRefIdxTest,
                                                          LIST_1, eParIdx8x8, eSubParIdx );
      cMvLastEst[1][iRefIdxTest][uiBlk] = cMvPred[1][iRefIdxTest][uiBlk];

      Bool bQPel = ( bQPelRefinementOnly &&
                     iRefIdxTest == iBLRefIdx[1] &&
                     cMvPred[1][iRefIdxTest][uiBlk] == cBLMvPred[1][uiBlk] );
      RNOK( m_pcMotionEstimation->estimateBlockWithStart(  *rpcMbTempData, *pcRefFrame,
                                                          cMvLastEst[1][iRefIdxTest][uiBlk],
                                                          cMvPred   [1][iRefIdxTest][uiBlk],
                                                          uiTmpBits, uiTmpCost,
                                                          eParIdx8x8+eSubParIdx, BLK_8x4, bQPel, 0,
                                                          &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdxTest, rpcMbTempData->getFieldFlag()) ) );
      RNOK( m_pcMotionEstimation->compensateBlock       ( &cTmpYuvMbBuffer,
                                                          eParIdx8x8+eSubParIdx, BLK_8x4 ) );
      rpcMbTempData->getMbMotionData( LIST_1 ).setAllMv ( cMvLastEst[1][iRefIdxTest][uiBlk], eParIdx8x8, eSubParIdx );

      uiBitsTest  += uiTmpBits;
      uiCostTest  += uiTmpCost;
    }

    if( uiCostTest < uiCost[1] )
    {
      bBLPred [1]     = false;
      iRefIdx [1]     = iRefIdxTest;
      cMv     [1][0]  = cMvLastEst[1][iRefIdxTest][0];
      cMv     [1][1]  = cMvLastEst[1][iRefIdxTest][1];
      uiBits  [1]     = uiBitsTest;
      uiCost  [1]     = uiCostTest;

      cYuvMbBuffer[1].loadLuma( cTmpYuvMbBuffer, B8x8Idx( ePar8x8 ) );
    }


    if( iRefIdxTest == iBLRefIdx[1] )
    {
      uiBitsTest  = 0;
      uiCostTest  = 0;
      for( UInt uiBlk = 0; uiBlk < 2; uiBlk++ )
      {
        SParIdx8x4  eSubParIdx  = aeParIdx8x4[ uiBlk ];
        UInt        uiTmpBits   = ( uiBlk || uiBasePredType == 1 ? 0 : uiBlkBits[1] );
        UInt        uiTmpCost;
        cBLMvLastEst[1][uiBlk]  = cBLMvPred[1][uiBlk];

        rpcMbTempData->getMbDataAccess().setMvPredictorsBL( cBLMvPred[1][uiBlk], LIST_1, eParIdx8x8, eSubParIdx );
        RNOK( m_pcMotionEstimation->estimateBlockWithStart(  *rpcMbTempData, *pcRefFrame,
                                                            cBLMvLastEst[1][uiBlk],
                                                            cBLMvPred   [1][uiBlk],
                                                            uiTmpBits, uiTmpCost,
                                                            eParIdx8x8+eSubParIdx, BLK_8x4, bQPelRefinementOnly, 0,
                                                            &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdxTest, rpcMbTempData->getFieldFlag() ) ) );
        RNOK( m_pcMotionEstimation->compensateBlock       ( &cTmpYuvMbBuffer,
                                                            eParIdx8x8+eSubParIdx, BLK_8x4 ) );
        rpcMbTempData->getMbMotionData( LIST_1 ).setAllMv ( cBLMvLastEst[1][uiBlk], eParIdx8x8, eSubParIdx );
        uiBitsTest  += uiTmpBits;
        uiCostTest  += uiTmpCost;
      }

      if( uiCostTest < uiCost[1] )
      {
        bBLPred [1]     = true;
        iRefIdx [1]     = iRefIdxTest;
        cMv     [1][0]  = cBLMvLastEst[1][0];
        cMv     [1][1]  = cBLMvLastEst[1][1];
        uiBits  [1]     = uiBitsTest;
        uiCost  [1]     = uiCostTest;
        cYuvMbBuffer[1].loadLuma( cTmpYuvMbBuffer, B8x8Idx( ePar8x8 ) );
      }
    }
  }

  //===== BI PREDICTION =====
  if( rcRefFrameList0.getActive() && rcRefFrameList1.getActive() )
  {
    //----- initialize with forward and backward estimation -----
    iRefIdxBi [0] = iRefIdx [0];
    iRefIdxBi [1] = iRefIdx [1];
    bBLPredBi [0] = bBLPred [0];
    bBLPredBi [1] = bBLPred [1];

    ::memcpy( cMvBi,      cMv,      2*2*sizeof(Mv ) );

    cMvPredBi[0][0]     = cMvPred[0][iRefIdx[0]][0];
    cMvPredBi[0][1]     = cMvPred[0][iRefIdx[0]][1];
    cMvPredBi[1][0]     = cMvPred[1][iRefIdx[1]][0];
    cMvPredBi[1][1]     = cMvPred[1][iRefIdx[1]][1];
    UInt  uiMotBits[2]  = { uiBits[0] - uiBlkBits[0], uiBits[1] - uiBlkBits[1] };
    uiBits[2]           = uiBlkBits[2] + uiMotBits[0] + uiMotBits[1];

    if( ! uiNumMaxIter )
    {
      uiNumMaxIter      = 1;
      uiIterSearchRange = 0;
    }

    //----- iterative search -----
    for( UInt uiIter = 0; uiIter < uiNumMaxIter; uiIter++ )
    {
      Bool          bChanged        = false;
      UInt          uiDir           = uiIter % 2;
      ListIdx       eListIdx        = ListIdx( uiDir );
      RefFrameList& rcRefFrameList  = ( uiDir ? rcRefFrameList1 : rcRefFrameList0 );
      BSParams      cBSParams;
      cBSParams.pcAltRefFrame       = ( uiDir ? rcRefFrameList0 : rcRefFrameList1 )[ iRefIdxBi[ 1 - uiDir ] ];
      cBSParams.pcAltRefPelData     = &cYuvMbBuffer[1-uiDir];
      cBSParams.uiL1Search          = uiDir;
      cBSParams.apcWeight[LIST_0]   = &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdxBi[LIST_0], rpcMbTempData->getFieldFlag() );
      cBSParams.apcWeight[LIST_1]   = &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdxBi[LIST_1], rpcMbTempData->getFieldFlag() );

      for( iRefIdxTest = 1; iRefIdxTest <= (Int)rcRefFrameList.getActive(); iRefIdxTest++ )
      {
        Mv  cMvPredTest[2];

        rpcMbTempData->getMbMotionData( eListIdx ).setRefIdx( iRefIdxTest, eParIdx8x8 );
        uiBitsTest  = 0;
        uiCostTest  = 0;
        pcRefFrame  = rcRefFrameList[iRefIdxTest];

        for( UInt uiBlk = 0; uiBlk < 2; uiBlk++ )
        {
          SParIdx8x4  eSubParIdx  = aeParIdx8x4[ uiBlk ];
          UInt        uiTmpBits   = ( uiBlk || uiBasePredType == 2 ? 0 : uiBlkBits[2] + uiMotBits[1-uiDir] + getRefIdxBits( iRefIdxTest, rcRefFrameList ) );
          UInt        uiTmpCost;

          rpcMbTempData->getMbDataAccess().getMvPredictor     ( cMvPredTest[uiBlk], iRefIdxTest,
                                                                eListIdx, eParIdx8x8, eSubParIdx );
          Bool bQPel = ( bQPelRefinementOnly &&
                         iRefIdxTest == iBLRefIdx[uiDir] &&
                         cMvPredTest[uiBlk] == cBLMvPred[uiDir][uiBlk] );
          RNOK( m_pcMotionEstimation->estimateBlockWithStart  (  *rpcMbTempData, *pcRefFrame,
                                                                cMvLastEst[uiDir][iRefIdxTest][uiBlk],
                                                                cMvPredTest                   [uiBlk],
                                                                uiTmpBits, uiTmpCost,
                                                                eParIdx8x8+eSubParIdx, BLK_8x4, bQPel,
                                                                uiIterSearchRange, 0, &cBSParams ) );
          RNOK( m_pcMotionEstimation->compensateBlock         ( &cTmpYuvMbBuffer,
                                                                eParIdx8x8+eSubParIdx, BLK_8x4 ) );
          rpcMbTempData->getMbMotionData( eListIdx ).setAllMv ( cMvLastEst[uiDir][iRefIdxTest][uiBlk], eParIdx8x8, eSubParIdx );

          uiBitsTest  += uiTmpBits;
          uiCostTest  += uiTmpCost;
        }

        if( uiCostTest < uiCost[2] )
        {
          bChanged              = true;
          bBLPredBi [uiDir]     = false;
          iRefIdxBi [uiDir]     = iRefIdxTest;
          cMvBi     [uiDir][0]  = cMvLastEst[uiDir][iRefIdxTest][0];
          cMvBi     [uiDir][1]  = cMvLastEst[uiDir][iRefIdxTest][1];
          cMvPredBi [uiDir][0]  = cMvPredTest                   [0];
          cMvPredBi [uiDir][1]  = cMvPredTest                   [1];
          uiMotBits [uiDir]     = uiBitsTest - uiBlkBits[2] - uiMotBits[1-uiDir];
          uiBits    [2]         = uiBitsTest;
          uiCost    [2]         = uiCostTest;

          cYuvMbBuffer[uiDir].loadLuma( cTmpYuvMbBuffer, B8x8Idx( ePar8x8 ) );
        }


        if( iRefIdxTest == iBLRefIdx[uiDir] )
        {
          uiBitsTest  = 0;
          uiCostTest  = 0;
          for( UInt uiBlk = 0; uiBlk < 2; uiBlk++ )
          {
            SParIdx8x4  eSubParIdx  = aeParIdx8x4[ uiBlk ];
            UInt        uiTmpBits   = ( uiBlk || uiBasePredType == 2 ? 0 : uiBlkBits[2] + uiMotBits[1-uiDir] );
            UInt        uiTmpCost;
            RNOK( m_pcMotionEstimation->estimateBlockWithStart  (  *rpcMbTempData, *pcRefFrame,
                                                                  cBLMvLastEst[uiDir][uiBlk],
                                                                  cBLMvPred   [uiDir][uiBlk],
                                                                  uiTmpBits, uiTmpCost,
                                                                  eParIdx8x8+eSubParIdx, BLK_8x4, bQPelRefinementOnly,
                                                                  uiIterSearchRange, 0, &cBSParams ) );
            RNOK( m_pcMotionEstimation->compensateBlock         ( &cTmpYuvMbBuffer,
                                                                  eParIdx8x8+eSubParIdx, BLK_8x4 ) );
            rpcMbTempData->getMbMotionData( eListIdx ).setAllMv ( cBLMvLastEst[uiDir][uiBlk], eParIdx8x8, eSubParIdx );
            uiBitsTest  += uiTmpBits;
            uiCostTest  += uiTmpCost;
          }

          if( uiCostTest < uiCost[2] )
          {
            bChanged              = true;
            bBLPredBi [uiDir]     = true;
            iRefIdxBi [uiDir]     = iRefIdxTest;
            cMvBi     [uiDir][0]  = cBLMvLastEst[uiDir][0];
            cMvBi     [uiDir][1]  = cBLMvLastEst[uiDir][1];
            uiMotBits [uiDir]     = uiBitsTest - uiBlkBits[2] - uiMotBits[1-uiDir];
            uiBits    [2]         = uiBitsTest;
            uiCost    [2]         = uiCostTest;

            cYuvMbBuffer[uiDir].loadLuma( cTmpYuvMbBuffer, B8x8Idx( ePar8x8 ) );
          }
        }
      }

      if( ! bChanged )
      {
        break;
      }
    }

    if( bBiPredOnly )
    {
      uiCost[0] = MSYS_UINT_MAX;
      uiCost[1] = MSYS_UINT_MAX;
    }
  }


  //===== chose parameters =====
  if( uiCost[2] <= uiCost[0] && uiCost[2] <= uiCost[1] )
  {
    //----- bi-directional prediction -----
    fRdCost         = uiCost    [2];
    uiSubMbBits     = uiBits    [2];
    iRefIdx [0]     = iRefIdxBi [0];
    iRefIdx [1]     = iRefIdxBi [1];
    bBLPred [0]     = bBLPredBi [0];
    bBLPred [1]     = bBLPredBi [1];
    if( bBLPred[0] )  { cMvPredBi[0][0] = cBLMvPred[0][0]; cMvPredBi[0][1] = cBLMvPred[0][1];  }
    if( bBLPred[1] )  { cMvPredBi[1][0] = cBLMvPred[1][0]; cMvPredBi[1][1] = cBLMvPred[1][1];  }
    cMv     [0][0]  = cMvBi     [0][0];
    cMv     [0][1]  = cMvBi     [0][1];
    cMv     [1][0]  = cMvBi     [1][0];
    cMv     [1][1]  = cMvBi     [1][1];
    cMvd    [0][0]  = cMv       [0][0]  - cMvPredBi[0][0];
    cMvd    [0][1]  = cMv       [0][1]  - cMvPredBi[0][1];
    cMvd    [1][0]  = cMv       [1][0]  - cMvPredBi[1][0];
    cMvd    [1][1]  = cMv       [1][1]  - cMvPredBi[1][1];
  }
  else if( uiCost[0] <= uiCost[1] )
  {
    //----- list 0 prediction -----
    fRdCost         = uiCost    [0];
    uiSubMbBits     = uiBits    [0];
    iRefIdx [1]     = BLOCK_NOT_PREDICTED;
    bBLPred [1]     = false;
    if( bBLPred[0] )  { cMvPred[0][iRefIdx[0]][0] = cBLMvPred[0][0]; cMvPred[0][iRefIdx[0]][1] = cBLMvPred[0][1];  }
    cMv     [1][0]  = Mv::ZeroMv();
    cMv     [1][1]  = Mv::ZeroMv();
    cMvd    [0][0]  = cMv[0][0] - cMvPred[0][iRefIdx[0]][0];
    cMvd    [0][1]  = cMv[0][1] - cMvPred[0][iRefIdx[0]][1];
  }
  else
  {
    //----- list 1 prediction -----
    fRdCost         = uiCost    [1];
    uiSubMbBits     = uiBits    [1];
    iRefIdx [0]     = BLOCK_NOT_PREDICTED;
    bBLPred [0]     = false;
    if( bBLPred[1] )  { cMvPred[1][iRefIdx[1]][0] = cBLMvPred[1][0]; cMvPred[1][iRefIdx[1]][1] = cBLMvPred[1][1];  }
    cMv     [0][0]  = Mv::ZeroMv();
    cMv     [0][1]  = Mv::ZeroMv();
    cMvd    [1][0]  = cMv[1][0] - cMvPred[1][iRefIdx[1]][0];
    cMvd    [1][1]  = cMv[1][1] - cMvPred[1][iRefIdx[1]][1];
  }


  //===== set parameters and compare =====
  rpcMbTempData->rdCost() = fRdCost;
  rpcMbTempData->getMbMotionData( LIST_0 ).setRefIdx( iRefIdx [0],    eParIdx8x8 );
  rpcMbTempData->getMbMotionData( LIST_0 ).setAllMv ( cMv     [0][0], eParIdx8x8, SPART_8x4_0 );
  rpcMbTempData->getMbMotionData( LIST_0 ).setAllMv ( cMv     [0][1], eParIdx8x8, SPART_8x4_1 );
  rpcMbTempData->getMbMvdData   ( LIST_0 ).setAllMv ( cMvd    [0][0], eParIdx8x8, SPART_8x4_0 );
  rpcMbTempData->getMbMvdData   ( LIST_0 ).setAllMv ( cMvd    [0][1], eParIdx8x8, SPART_8x4_1 );
  rpcMbTempData->getMbMotionData( LIST_1 ).setRefIdx( iRefIdx [1],    eParIdx8x8 );
  rpcMbTempData->getMbMotionData( LIST_1 ).setAllMv ( cMv     [1][0], eParIdx8x8, SPART_8x4_0 );
  rpcMbTempData->getMbMotionData( LIST_1 ).setAllMv ( cMv     [1][1], eParIdx8x8, SPART_8x4_1 );
  rpcMbTempData->getMbMvdData   ( LIST_1 ).setAllMv ( cMvd    [1][0], eParIdx8x8, SPART_8x4_0 );
  rpcMbTempData->getMbMvdData   ( LIST_1 ).setAllMv ( cMvd    [1][1], eParIdx8x8, SPART_8x4_1 );

  rpcMbTempData->getMbMotionData( LIST_0 ).setMotPredFlag( bBLPred[0], eParIdx8x8 );
  rpcMbTempData->getMbMotionData( LIST_1 ).setMotPredFlag( bBLPred[1], eParIdx8x8 );
  ROT( bBLPred[0] && iRefIdx[0] != iBLRefIdx[0] );
  ROT( bBLPred[1] && iRefIdx[1] != iBLRefIdx[1] );

  RNOK( xSetRdCostInterSubMb( *rpcMbTempData, rcRefFrameList0, rcRefFrameList1, B8x8Idx( ePar8x8 ), false, uiSubMbBits, bLowComplexMbEnable ) );


  //JVT-R057 LA-RDO{
  if(m_bLARDOEnable)
  {
	  MbDataAccess&   rcMbDataAccess  = rpcMbTempData->getMbDataAccess();
	  Int distortion1=0,distortion2=0,distortion=0;
	  for( Int n = 0; n <2; n++)
	  {
		  iRefIdx [0]=rpcMbTempData->getMbMotionData(LIST_0).getRefIdx(eParIdx8x8);
		  iRefIdx [1]=rpcMbTempData->getMbMotionData(LIST_1).getRefIdx(eParIdx8x8);
		  Frame* pcRefFrame0 = ( iRefIdx [0] > 0 ? rcRefFrameList0[ iRefIdx [0] ] : NULL );
		  Frame* pcRefFrame1 = ( iRefIdx [1] > 0 ? rcRefFrameList1[ iRefIdx [1] ] : NULL );
		  Int iMvX;
		  Int iMvY;

		  if(pcRefFrame0)
		  {
			  iMvX=cMv[LIST_0][n].getHor();
			  iMvY=cMv[LIST_0][n].getVer();
			  getChannelDistortion(rcMbDataAccess,*pcRefFrame0,&distortion1,iMvX,iMvY,(eParIdx8x8+aeParIdx8x4[n])%4,(eParIdx8x8+aeParIdx8x4 [n])/4,2,1);
		  }
		  if(pcRefFrame1)
		  {
			  iMvX=cMv[LIST_1][n].getHor();
			  iMvY=cMv[LIST_1][n].getVer();
			  getChannelDistortion(rcMbDataAccess,*pcRefFrame1,&distortion2,iMvX,iMvY,(eParIdx8x8+aeParIdx8x4[n])%4,(eParIdx8x8+aeParIdx8x4 [n])/4,2,1);
			  if(pcRefFrame0)
				  distortion1=(Int)(m_dWr0*distortion1+m_dWr0*distortion2);
			  else
				  distortion1=distortion2;
		  }
		  distortion+=distortion1;
	  }
	  rpcMbTempData->rdCost()+=distortion;
  }
  //JVT-R057 LA-RDO}

  RNOK( xCheckBestEstimation(  rpcMbTempData, rpcMbBestData ) );

  return Err::m_nOK;
}

ErrVal
MbEncoder::xEstimateSubMb4x8( Par8x8            ePar8x8,
                              IntMbTempData*&   rpcMbTempData,
                              IntMbTempData*&   rpcMbBestData,
                              RefFrameList&     rcRefFrameList0,
                              RefFrameList&     rcRefFrameList1,
                              Bool              bBiPredOnly,
                              UInt              uiNumMaxIter,
                              UInt              uiIterSearchRange,
                              UInt              uiAddBits,
                              Bool              bQPelRefinementOnly,
                              MbDataAccess*     pcMbDataAccessBase )
{
  ROF( rcRefFrameList0.getActive() <= 32 );
  ROF( rcRefFrameList1.getActive() <= 32 );
  ROF( rcRefFrameList0.getActive() );
  ROF( ! bBiPredOnly || rcRefFrameList1.getActive() );

  //JVT-V079 Low-complexity MB mode decision
  Bool bLowComplexMbEnable = m_bLowComplexMbEnable[rpcMbTempData->getSH().getDependencyId()];
  
  Bool            bPSlice         = rpcMbTempData->getMbDataAccess().getSH().isPSlice();
  Double          fRdCost         = 0;
  UInt            uiSubMbBits     = 0;
  SParIdx4x8      aeParIdx4x8 [2] = { SPART_4x8_0, SPART_4x8_1 };
  ParIdx8x8       aeParIdx8x8 [4] = { PART_8x8_0, PART_8x8_1, PART_8x8_2, PART_8x8_3 };
  ParIdx8x8       eParIdx8x8      = aeParIdx8x8[ ePar8x8 ];
  UInt            uiCost      [3] = { MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX }, uiCostTest;
  UInt            uiBlkBits   [3] = { ( ! bPSlice ? 5 : 3 ) + uiAddBits, 7 + uiAddBits, 7 + uiAddBits };
  Int             iRefIdx     [2] = { 0, 0 }, iRefIdxBi[2], iRefIdxTest;
  UInt            uiBits      [3] = { 0, 0, 0 }, uiBitsTest;
  Mv              cMv[2][2], cMvBi[2][2], cMvd[2][2], cMvLastEst[2][33][2], cMvPred[2][33][2], cMvPredBi[2][2];
  YuvMbBuffer  cYuvMbBuffer[2], cTmpYuvMbBuffer;
  Frame*       pcRefFrame;


  Bool            bBLPred   [2] = { false, false };
  Bool            bBLPredBi [2] = { false, false };
  Int             iBLRefIdx [2] = { -1, -1 };
  Mv              cBLMvPred [2][2], cBLMvLastEst[2][2];

  if( pcMbDataAccessBase )
  {
    if( pcMbDataAccessBase->getMbMotionData( LIST_0 ).getRefIdx( eParIdx8x8 ) >  0 &&
        pcMbDataAccessBase->getMbMotionData( LIST_0 ).getRefIdx( eParIdx8x8 ) <= (Int)rcRefFrameList0.getActive() )
    {
      iBLRefIdx [0]    = pcMbDataAccessBase->getMbMotionData( LIST_0 ).getRefIdx ( eParIdx8x8 );
      cBLMvPred [0][0] = pcMbDataAccessBase->getMbMotionData( LIST_0 ).getMv     ( eParIdx8x8, SPART_4x8_0 );
      cBLMvPred [0][1] = pcMbDataAccessBase->getMbMotionData( LIST_0 ).getMv     ( eParIdx8x8, SPART_4x8_1 );
    }
    if( pcMbDataAccessBase->getMbMotionData( LIST_1 ).getRefIdx( eParIdx8x8 ) >  0 &&
        pcMbDataAccessBase->getMbMotionData( LIST_1 ).getRefIdx( eParIdx8x8 ) <= (Int)rcRefFrameList1.getActive() )
    {
      iBLRefIdx [1]    = pcMbDataAccessBase->getMbMotionData( LIST_1 ).getRefIdx ( eParIdx8x8 );
      cBLMvPred [1][0] = pcMbDataAccessBase->getMbMotionData( LIST_1 ).getMv     ( eParIdx8x8, SPART_4x8_0 );
      cBLMvPred [1][1] = pcMbDataAccessBase->getMbMotionData( LIST_1 ).getMv     ( eParIdx8x8, SPART_4x8_1 );
    }
  }

  UInt  uiBasePredType = MSYS_UINT_MAX;

  rpcMbTempData->clear();
  rpcMbTempData->setBlkMode( ePar8x8, BLK_4x8 );


  //===== LIST 0 PREDICTION ======
  for( iRefIdxTest = 1; iRefIdxTest <= (Int)rcRefFrameList0.getActive(); iRefIdxTest++ )
  {
    rpcMbTempData->getMbMotionData( LIST_0 ).setRefIdx( iRefIdxTest, eParIdx8x8 );
    uiBitsTest  = 0;
    uiCostTest  = 0;
    pcRefFrame  = rcRefFrameList0[iRefIdxTest];

    for( UInt uiBlk = 0; uiBlk < 2; uiBlk++ )
    {
      SParIdx4x8  eSubParIdx  = aeParIdx4x8[ uiBlk ];
      UInt        uiTmpBits   = ( uiBlk || uiBasePredType == 0 ? 0 : uiBlkBits[0] + getRefIdxBits( iRefIdxTest, rcRefFrameList0 ) );
      UInt        uiTmpCost;

      rpcMbTempData->getMbDataAccess().getMvPredictor   ( cMvPred[0][iRefIdxTest][uiBlk], iRefIdxTest,
                                                          LIST_0, eParIdx8x8, eSubParIdx );
      cMvLastEst[0][iRefIdxTest][uiBlk] = cMvPred[0][iRefIdxTest][uiBlk];

      Bool bQPel = ( bQPelRefinementOnly &&
                     iRefIdxTest == iBLRefIdx[0] &&
                     cMvPred[0][iRefIdxTest][uiBlk] == cBLMvPred[0][uiBlk] );
      RNOK( m_pcMotionEstimation->estimateBlockWithStart(  *rpcMbTempData, *pcRefFrame,
                                                          cMvLastEst[0][iRefIdxTest][uiBlk],
                                                          cMvPred   [0][iRefIdxTest][uiBlk],
                                                          uiTmpBits, uiTmpCost,
                                                          eParIdx8x8+eSubParIdx, BLK_4x8, bQPel, 0,
                                                          &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdxTest, rpcMbTempData->getFieldFlag()) ) );
      rpcMbTempData->getMbMotionData( LIST_0 ).setAllMv ( cMvLastEst[0][iRefIdxTest][uiBlk], eParIdx8x8, eSubParIdx );

      uiBitsTest  += uiTmpBits;
      uiCostTest  += uiTmpCost;
    }

    if( uiCostTest < uiCost[0] )
    {
      bBLPred [0]     = false;
      iRefIdx [0]     = iRefIdxTest;
      cMv     [0][0]  = cMvLastEst[0][iRefIdxTest][0];
      cMv     [0][1]  = cMvLastEst[0][iRefIdxTest][1];
      uiBits  [0]     = uiBitsTest;
      uiCost  [0]     = uiCostTest;
    }


    if( iRefIdxTest == iBLRefIdx[0] )
    {
      uiBitsTest  = 0;
      uiCostTest  = 0;
      for( UInt uiBlk = 0; uiBlk < 2; uiBlk++ )
      {
        SParIdx4x8  eSubParIdx  = aeParIdx4x8[ uiBlk ];
        UInt        uiTmpBits   = ( uiBlk || uiBasePredType == 0 ? 0 : uiBlkBits[0] );
        UInt        uiTmpCost;
        cBLMvLastEst[0][uiBlk]  = cBLMvPred[0][uiBlk];

        rpcMbTempData->getMbDataAccess().setMvPredictorsBL( cBLMvPred[0][uiBlk], LIST_0, eParIdx8x8, eSubParIdx );
        RNOK( m_pcMotionEstimation->estimateBlockWithStart( *rpcMbTempData, *pcRefFrame,
                                                            cBLMvLastEst[0][uiBlk],
                                                            cBLMvPred   [0][uiBlk],
                                                            uiTmpBits, uiTmpCost,
                                                            eParIdx8x8+eSubParIdx, BLK_4x8, bQPelRefinementOnly, 0,
                                                            &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdxTest, rpcMbTempData->getFieldFlag()) ) );
        rpcMbTempData->getMbMotionData( LIST_0 ).setAllMv ( cBLMvLastEst[0][uiBlk], eParIdx8x8, eSubParIdx );
        uiBitsTest  += uiTmpBits;
        uiCostTest  += uiTmpCost;
      }

      if( uiCostTest < uiCost[0] )
      {
        bBLPred [0]     = true;
        iRefIdx [0]     = iRefIdxTest;
        cMv     [0][0]  = cBLMvLastEst[0][0];
        cMv     [0][1]  = cBLMvLastEst[0][1];
        uiBits  [0]     = uiBitsTest;
        uiCost  [0]     = uiCostTest;
      }
    }
  }


  //===== LIST 1 PREDICTION =====
  for( iRefIdxTest = 1; iRefIdxTest <= (Int)rcRefFrameList1.getActive(); iRefIdxTest++ )
  {
    rpcMbTempData->getMbMotionData( LIST_1 ).setRefIdx( iRefIdxTest, eParIdx8x8 );
    uiBitsTest  = 0;
    uiCostTest  = 0;
    pcRefFrame  = rcRefFrameList1[iRefIdxTest];

    for( UInt uiBlk = 0; uiBlk < 2; uiBlk++ )
    {
      SParIdx4x8  eSubParIdx  = aeParIdx4x8[ uiBlk ];
      UInt        uiTmpBits   = ( uiBlk || uiBasePredType == 1 ? 0 : uiBlkBits[1] + getRefIdxBits( iRefIdxTest, rcRefFrameList1 ) );
      UInt        uiTmpCost;

      rpcMbTempData->getMbDataAccess().getMvPredictor   ( cMvPred[1][iRefIdxTest][uiBlk], iRefIdxTest,
                                                          LIST_1, eParIdx8x8, eSubParIdx );
      cMvLastEst[1][iRefIdxTest][uiBlk] = cMvPred[1][iRefIdxTest][uiBlk];

      Bool bQPel = ( bQPelRefinementOnly &&
                     iRefIdxTest == iBLRefIdx[1] &&
                     cMvPred[1][iRefIdxTest][uiBlk] == cBLMvPred[1][uiBlk] );
      RNOK( m_pcMotionEstimation->estimateBlockWithStart(  *rpcMbTempData, *pcRefFrame,
                                                          cMvLastEst[1][iRefIdxTest][uiBlk],
                                                          cMvPred   [1][iRefIdxTest][uiBlk],
                                                          uiTmpBits, uiTmpCost,
                                                          eParIdx8x8+eSubParIdx, BLK_4x8, bQPel, 0,
                                                          &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdxTest, rpcMbTempData->getFieldFlag()) ) );
      RNOK( m_pcMotionEstimation->compensateBlock       ( &cTmpYuvMbBuffer,
                                                          eParIdx8x8+eSubParIdx, BLK_4x8 ) );
      rpcMbTempData->getMbMotionData( LIST_1 ).setAllMv ( cMvLastEst[1][iRefIdxTest][uiBlk], eParIdx8x8, eSubParIdx );

      uiBitsTest  += uiTmpBits;
      uiCostTest  += uiTmpCost;
    }

    if( uiCostTest < uiCost[1] )
    {
      bBLPred [1]     = false;
      iRefIdx [1]     = iRefIdxTest;
      cMv     [1][0]  = cMvLastEst[1][iRefIdxTest][0];
      cMv     [1][1]  = cMvLastEst[1][iRefIdxTest][1];
      uiBits  [1]     = uiBitsTest;
      uiCost  [1]     = uiCostTest;

      cYuvMbBuffer[1].loadLuma( cTmpYuvMbBuffer, B8x8Idx( ePar8x8 ) );
    }

    if( iRefIdxTest == iBLRefIdx[1] )
    {
      uiBitsTest  = 0;
      uiCostTest  = 0;
      for( UInt uiBlk = 0; uiBlk < 2; uiBlk++ )
      {
        SParIdx4x8  eSubParIdx  = aeParIdx4x8[ uiBlk ];
        UInt        uiTmpBits   = ( uiBlk || uiBasePredType == 1 ? 0 : uiBlkBits[1] );
        UInt        uiTmpCost;
        cBLMvLastEst[1][uiBlk] = cBLMvPred[1][uiBlk];

        rpcMbTempData->getMbDataAccess().setMvPredictorsBL( cBLMvPred[1][uiBlk], LIST_1, eParIdx8x8, eSubParIdx );
        RNOK( m_pcMotionEstimation->estimateBlockWithStart(  *rpcMbTempData, *pcRefFrame,
                                                            cBLMvLastEst[1][uiBlk],
                                                            cBLMvPred   [1][uiBlk],
                                                            uiTmpBits, uiTmpCost,
                                                            eParIdx8x8+eSubParIdx, BLK_4x8, bQPelRefinementOnly, 0,
                                                            &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdxTest, rpcMbTempData->getFieldFlag()) ) );
        RNOK( m_pcMotionEstimation->compensateBlock       ( &cTmpYuvMbBuffer,
                                                            eParIdx8x8+eSubParIdx, BLK_4x8 ) );
        rpcMbTempData->getMbMotionData( LIST_1 ).setAllMv ( cBLMvLastEst[1][uiBlk], eParIdx8x8, eSubParIdx );
        uiBitsTest  += uiTmpBits;
        uiCostTest  += uiTmpCost;
      }

      if( uiCostTest < uiCost[1] )
      {
        bBLPred [1]     = true;
        iRefIdx [1]     = iRefIdxTest;
        cMv     [1][0]  = cBLMvLastEst[1][0];
        cMv     [1][1]  = cBLMvLastEst[1][1];
        uiBits  [1]     = uiBitsTest;
        uiCost  [1]     = uiCostTest;

        cYuvMbBuffer[1].loadLuma( cTmpYuvMbBuffer, B8x8Idx( ePar8x8 ) );
      }
    }
  }

  //===== BI PREDICTION =====
  if( rcRefFrameList0.getActive() && rcRefFrameList1.getActive() )
  {
    //----- initialize with forward and backward estimation -----
    iRefIdxBi [0] = iRefIdx [0];
    iRefIdxBi [1] = iRefIdx [1];
    bBLPredBi [0] = bBLPred [0];
    bBLPredBi [1] = bBLPred [1];

    ::memcpy( cMvBi,      cMv,      2*2*sizeof(Mv ) );

    cMvPredBi [0][0]    = cMvPred [0][iRefIdx[0]][0];
    cMvPredBi [0][1]    = cMvPred [0][iRefIdx[0]][1];
    cMvPredBi [1][0]    = cMvPred [1][iRefIdx[1]][0];
    cMvPredBi [1][1]    = cMvPred [1][iRefIdx[1]][1];
    UInt  uiMotBits[2]  = { uiBits[0] - uiBlkBits[0], uiBits[1] - uiBlkBits[1] };
    uiBits[2]           = uiBlkBits[2] + uiMotBits[0] + uiMotBits[1];

    if( ! uiNumMaxIter )
    {
      uiNumMaxIter      = 1;
      uiIterSearchRange = 0;
    }

    //----- iterative search -----
    for( UInt uiIter = 0; uiIter < uiNumMaxIter; uiIter++ )
    {
      Bool          bChanged        = false;
      UInt          uiDir           = uiIter % 2;
      ListIdx       eListIdx        = ListIdx( uiDir );
      RefFrameList& rcRefFrameList  = ( uiDir ? rcRefFrameList1 : rcRefFrameList0 );
      BSParams      cBSParams;
      cBSParams.pcAltRefFrame       = ( uiDir ? rcRefFrameList0 : rcRefFrameList1 )[ iRefIdxBi[ 1 - uiDir ] ];
      cBSParams.pcAltRefPelData     = &cYuvMbBuffer[1-uiDir];
      cBSParams.uiL1Search          = uiDir;
      cBSParams.apcWeight[LIST_0]   = &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdxBi[LIST_0], rpcMbTempData->getFieldFlag() );
      cBSParams.apcWeight[LIST_1]   = &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdxBi[LIST_1], rpcMbTempData->getFieldFlag() );

      for( iRefIdxTest = 1; iRefIdxTest <= (Int)rcRefFrameList.getActive(); iRefIdxTest++ )
      {
        Mv  cMvPredTest[2];

        rpcMbTempData->getMbMotionData( eListIdx ).setRefIdx( iRefIdxTest, eParIdx8x8 );
        uiBitsTest  = 0;
        uiCostTest  = 0;
        pcRefFrame  = rcRefFrameList[iRefIdxTest];

        for( UInt uiBlk = 0; uiBlk < 2; uiBlk++ )
        {
          SParIdx4x8  eSubParIdx  = aeParIdx4x8[ uiBlk ];
          UInt        uiTmpBits   = ( uiBlk || uiBasePredType == 2 ? 0 : uiBlkBits[2] + uiMotBits[1-uiDir] + getRefIdxBits( iRefIdxTest, rcRefFrameList ) );
          UInt        uiTmpCost;

          rpcMbTempData->getMbDataAccess().getMvPredictor     ( cMvPredTest[uiBlk], iRefIdxTest,
                                                                eListIdx, eParIdx8x8, eSubParIdx );
          Bool bQPel = ( bQPelRefinementOnly &&
                         iRefIdxTest == iBLRefIdx[uiDir] &&
                         cMvPredTest[uiBlk] == cBLMvPred[uiDir][uiBlk] );
          RNOK( m_pcMotionEstimation->estimateBlockWithStart  (  *rpcMbTempData, *pcRefFrame,
                                                                cMvLastEst[uiDir][iRefIdxTest][uiBlk],
                                                                cMvPredTest                   [uiBlk],
                                                                uiTmpBits, uiTmpCost,
                                                                eParIdx8x8+eSubParIdx, BLK_4x8, bQPel,
                                                                uiIterSearchRange, 0, &cBSParams ) );
          RNOK( m_pcMotionEstimation->compensateBlock         ( &cTmpYuvMbBuffer,
                                                                eParIdx8x8+eSubParIdx, BLK_4x8 ) );
          rpcMbTempData->getMbMotionData( eListIdx ).setAllMv ( cMvLastEst[uiDir][iRefIdxTest][uiBlk], eParIdx8x8, eSubParIdx );

          uiBitsTest += uiTmpBits;
          uiCostTest += uiTmpCost;
        }

        if( uiCostTest < uiCost[2] )
        {
          bChanged              = true;
          bBLPredBi [uiDir]     = false;
          iRefIdxBi [uiDir]     = iRefIdxTest;
          cMvBi     [uiDir][0]  = cMvLastEst[uiDir][iRefIdxTest][0];
          cMvBi     [uiDir][1]  = cMvLastEst[uiDir][iRefIdxTest][1];
          cMvPredBi [uiDir][0]  = cMvPredTest                   [0];
          cMvPredBi [uiDir][1]  = cMvPredTest                   [1];
          uiMotBits [uiDir]     = uiBitsTest - uiBlkBits[2] - uiMotBits[1-uiDir];
          uiBits    [2]         = uiBitsTest;
          uiCost    [2]         = uiCostTest;

          cYuvMbBuffer[uiDir].loadLuma( cTmpYuvMbBuffer, B8x8Idx( ePar8x8 ) );
        }


        if( iRefIdxTest == iBLRefIdx[uiDir] )
        {
          uiBitsTest  = 0;
          uiCostTest  = 0;
          for( UInt uiBlk = 0; uiBlk < 2; uiBlk++ )
          {
            SParIdx4x8  eSubParIdx  = aeParIdx4x8[ uiBlk ];
            UInt        uiTmpBits   = ( uiBlk || uiBasePredType == 2 ? 0 : uiBlkBits[2] + uiMotBits[1-uiDir] );
            UInt        uiTmpCost;
            RNOK( m_pcMotionEstimation->estimateBlockWithStart  ( *rpcMbTempData, *pcRefFrame,
                                                                  cBLMvLastEst[uiDir][uiBlk],
                                                                  cBLMvPred   [uiDir][uiBlk],
                                                                  uiTmpBits, uiTmpCost,
                                                                  eParIdx8x8+eSubParIdx, BLK_4x8, bQPelRefinementOnly,
                                                                  uiIterSearchRange, 0, &cBSParams ) );
            RNOK( m_pcMotionEstimation->compensateBlock         ( &cTmpYuvMbBuffer,
                                                                  eParIdx8x8+eSubParIdx, BLK_4x8 ) );
            rpcMbTempData->getMbMotionData( eListIdx ).setAllMv ( cBLMvLastEst[uiDir][uiBlk], eParIdx8x8, eSubParIdx );
            uiBitsTest += uiTmpBits;
            uiCostTest += uiTmpCost;
          }

          if( uiCostTest < uiCost[2] )
          {
            bChanged              = true;
            bBLPredBi [uiDir]     = true;
            iRefIdxBi [uiDir]     = iRefIdxTest;
            cMvBi     [uiDir][0]  = cBLMvLastEst[uiDir][0];
            cMvBi     [uiDir][1]  = cBLMvLastEst[uiDir][1];
            uiMotBits [uiDir]     = uiBitsTest - uiBlkBits[2] - uiMotBits[1-uiDir];
            uiBits    [2]         = uiBitsTest;
            uiCost    [2]         = uiCostTest;

            cYuvMbBuffer[uiDir].loadLuma( cTmpYuvMbBuffer, B8x8Idx( ePar8x8 ) );
          }
        }
      }

      if( ! bChanged )
      {
        break;
      }
    }

    if( bBiPredOnly )
    {
      uiCost[0] = MSYS_UINT_MAX;
      uiCost[1] = MSYS_UINT_MAX;
    }
  }


  //===== chose parameters =====
  if( uiCost[2] <= uiCost[0] && uiCost[2] <= uiCost[1] )
  {
    //----- bi-directional prediction -----
    fRdCost         = uiCost    [2];
    uiSubMbBits     = uiBits    [2];
    iRefIdx [0]     = iRefIdxBi [0];
    iRefIdx [1]     = iRefIdxBi [1];
    bBLPred [0]     = bBLPredBi [0];
    bBLPred [1]     = bBLPredBi [1];
    if( bBLPred[0] )  { cMvPredBi[0][0] = cBLMvPred[0][0]; cMvPredBi[0][1] = cBLMvPred[0][1];  }
    if( bBLPred[1] )  { cMvPredBi[1][0] = cBLMvPred[1][0]; cMvPredBi[1][1] = cBLMvPred[1][1];  }
    cMv     [0][0]  = cMvBi     [0][0];
    cMv     [0][1]  = cMvBi     [0][1];
    cMv     [1][0]  = cMvBi     [1][0];
    cMv     [1][1]  = cMvBi     [1][1];
    cMvd    [0][0]  = cMv       [0][0]  - cMvPredBi[0][0];
    cMvd    [0][1]  = cMv       [0][1]  - cMvPredBi[0][1];
    cMvd    [1][0]  = cMv       [1][0]  - cMvPredBi[1][0];
    cMvd    [1][1]  = cMv       [1][1]  - cMvPredBi[1][1];
  }
  else if( uiCost[0] <= uiCost[1] )
  {
    //----- list 0 prediction -----
    fRdCost         = uiCost    [0];
    uiSubMbBits     = uiBits    [0];
    iRefIdx [1]     = BLOCK_NOT_PREDICTED;
    bBLPred [1]     = false;
    if( bBLPred[0] )  { cMvPred[0][iRefIdx[0]][0] = cBLMvPred[0][0]; cMvPred[0][iRefIdx[0]][1] = cBLMvPred[0][1];  }
    cMv     [1][0]  = Mv::ZeroMv();
    cMv     [1][1]  = Mv::ZeroMv();
    cMvd    [0][0]  = cMv[0][0] - cMvPred[0][iRefIdx[0]][0];
    cMvd    [0][1]  = cMv[0][1] - cMvPred[0][iRefIdx[0]][1];
  }
  else
  {
    //----- list 1 prediction -----
    fRdCost         = uiCost    [1];
    uiSubMbBits     = uiBits    [1];
    iRefIdx [0]     = BLOCK_NOT_PREDICTED;
    bBLPred [0]     = false;
    if( bBLPred[1] )  { cMvPred[1][iRefIdx[1]][0] = cBLMvPred[1][0]; cMvPred[1][iRefIdx[1]][1] = cBLMvPred[1][1];  }
    cMv     [0][0]  = Mv::ZeroMv();
    cMv     [0][1]  = Mv::ZeroMv();
    cMvd    [1][0]  = cMv[1][0] - cMvPred[1][iRefIdx[1]][0];
    cMvd    [1][1]  = cMv[1][1] - cMvPred[1][iRefIdx[1]][1];
  }


  //===== set parameters and compare =====
  rpcMbTempData->rdCost() = fRdCost;
  rpcMbTempData->getMbMotionData( LIST_0 ).setRefIdx( iRefIdx [0],    eParIdx8x8 );
  rpcMbTempData->getMbMotionData( LIST_0 ).setAllMv ( cMv     [0][0], eParIdx8x8, SPART_4x8_0 );
  rpcMbTempData->getMbMotionData( LIST_0 ).setAllMv ( cMv     [0][1], eParIdx8x8, SPART_4x8_1 );
  rpcMbTempData->getMbMvdData   ( LIST_0 ).setAllMv ( cMvd    [0][0], eParIdx8x8, SPART_4x8_0 );
  rpcMbTempData->getMbMvdData   ( LIST_0 ).setAllMv ( cMvd    [0][1], eParIdx8x8, SPART_4x8_1 );
  rpcMbTempData->getMbMotionData( LIST_1 ).setRefIdx( iRefIdx [1],    eParIdx8x8 );
  rpcMbTempData->getMbMotionData( LIST_1 ).setAllMv ( cMv     [1][0], eParIdx8x8, SPART_4x8_0 );
  rpcMbTempData->getMbMotionData( LIST_1 ).setAllMv ( cMv     [1][1], eParIdx8x8, SPART_4x8_1 );
  rpcMbTempData->getMbMvdData   ( LIST_1 ).setAllMv ( cMvd    [1][0], eParIdx8x8, SPART_4x8_0 );
  rpcMbTempData->getMbMvdData   ( LIST_1 ).setAllMv ( cMvd    [1][1], eParIdx8x8, SPART_4x8_1 );

  rpcMbTempData->getMbMotionData( LIST_0 ).setMotPredFlag( bBLPred[0], eParIdx8x8 );
  rpcMbTempData->getMbMotionData( LIST_1 ).setMotPredFlag( bBLPred[1], eParIdx8x8 );
  ROT( bBLPred[0] && iRefIdx[0] != iBLRefIdx[0] );
  ROT( bBLPred[1] && iRefIdx[1] != iBLRefIdx[1] );

  RNOK( xSetRdCostInterSubMb( *rpcMbTempData, rcRefFrameList0, rcRefFrameList1, B8x8Idx( ePar8x8 ), false, uiSubMbBits, bLowComplexMbEnable ) );


  //JVT-R057 LA-RDO{
  if(m_bLARDOEnable)
  {
	  MbDataAccess&   rcMbDataAccess  = rpcMbTempData->getMbDataAccess();
	  Int distortion1=0,distortion2,distortion=0;
	  for( Int n = 0; n <2; n++)
	  {
		  iRefIdx [0]=rpcMbTempData->getMbMotionData(LIST_0).getRefIdx(eParIdx8x8);
		  iRefIdx [1]=rpcMbTempData->getMbMotionData(LIST_1).getRefIdx(eParIdx8x8);
		  Frame* pcRefFrame0 = ( iRefIdx [0] > 0 ? rcRefFrameList0[ iRefIdx [0] ] : NULL );
		  Frame* pcRefFrame1 = ( iRefIdx [1] > 0 ? rcRefFrameList1[ iRefIdx [1] ] : NULL );
		  Int iMvX;
		  Int iMvY;

		  if(pcRefFrame0)
		  {
			  iMvX=cMv[LIST_0][n].getHor();
			  iMvY=cMv[LIST_0][n].getVer();
			  getChannelDistortion(rcMbDataAccess,*pcRefFrame0,&distortion1,iMvX,iMvY,(eParIdx8x8+aeParIdx4x8[n])%4,(eParIdx8x8+aeParIdx4x8 [n])/4,1,2);
		  }
		  if(pcRefFrame1)
		  {
			  iMvX=cMv[LIST_1][n].getHor();
			  iMvY=cMv[LIST_1][n].getVer();
			  getChannelDistortion(rcMbDataAccess,*pcRefFrame1,&distortion2,iMvX,iMvY,(eParIdx8x8+aeParIdx4x8[n])%4,(eParIdx8x8+aeParIdx4x8 [n])/4,1,2);
			  if(pcRefFrame0)
				  distortion1=(Int)(m_dWr0*distortion1+m_dWr0*distortion2);
			  else
				  distortion1=distortion2;
		  }
		  distortion+=distortion1;
	  }
	  rpcMbTempData->rdCost()+=distortion;
  }
  //JVT-R057 LA-RDO}

  RNOK( xCheckBestEstimation(  rpcMbTempData, rpcMbBestData ) );

  return Err::m_nOK;
}





ErrVal
MbEncoder::xEstimateSubMb4x4( Par8x8            ePar8x8,
                              IntMbTempData*&   rpcMbTempData,
                              IntMbTempData*&   rpcMbBestData,
                              RefFrameList&     rcRefFrameList0,
                              RefFrameList&     rcRefFrameList1,
                              Bool              bBiPredOnly,
                              UInt              uiNumMaxIter,
                              UInt              uiIterSearchRange,
                              UInt              uiAddBits,
                              Bool              bQPelRefinementOnly,
                              MbDataAccess*     pcMbDataAccessBase )
{
  ROF( rcRefFrameList0.getActive() <= 32 );
  ROF( rcRefFrameList1.getActive() <= 32 );
  ROF( rcRefFrameList0.getActive() );
  ROF( ! bBiPredOnly || rcRefFrameList1.getActive() );

  //JVT-V079 Low-complexity MB mode decision
  Bool bLowComplexMbEnable = m_bLowComplexMbEnable[rpcMbTempData->getSH().getDependencyId()];
  
  Bool            bPSlice         = rpcMbTempData->getMbDataAccess().getSH().isPSlice();
  Double          fRdCost         = 0;
  UInt            uiSubMbBits     = 0;
  SParIdx4x4      aeParIdx4x4 [4] = { SPART_4x4_0, SPART_4x4_1, SPART_4x4_2, SPART_4x4_3 };
  ParIdx8x8       aeParIdx8x8 [4] = { PART_8x8_0, PART_8x8_1, PART_8x8_2, PART_8x8_3 };
  ParIdx8x8       eParIdx8x8      = aeParIdx8x8[ ePar8x8 ];
  UInt            uiCost      [3] = { MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX }, uiCostTest;
  UInt            uiBlkBits   [3] = { ( ! bPSlice ? 7 : 5 ) + uiAddBits, 7 + uiAddBits, 7 + uiAddBits };
  Int             iRefIdx     [2] = { 0, 0 }, iRefIdxBi[2], iRefIdxTest;
  UInt            uiBits      [3] = { 0, 0, 0 }, uiBitsTest;
  Mv              cMv[2][4], cMvBi[2][4], cMvd[2][4], cMvLastEst[2][33][4], cMvPred[2][33][4], cMvPredBi[2][4];
  YuvMbBuffer  cYuvMbBuffer[2], cTmpYuvMbBuffer;
  Frame*       pcRefFrame;


  Bool            bBLPred   [2] = { false, false };
  Bool            bBLPredBi [2] = { false, false };
  Int             iBLRefIdx [2] = { -1, -1 };
  Mv              cBLMvPred [2][4], cBLMvLastEst[2][4];

  if( pcMbDataAccessBase )
  {
    if( pcMbDataAccessBase->getMbMotionData( LIST_0 ).getRefIdx( eParIdx8x8 ) >  0 &&
        pcMbDataAccessBase->getMbMotionData( LIST_0 ).getRefIdx( eParIdx8x8 ) <= (Int)rcRefFrameList0.getActive() )
    {
      iBLRefIdx [0]    = pcMbDataAccessBase->getMbMotionData( LIST_0 ).getRefIdx ( eParIdx8x8 );
      cBLMvPred [0][0] = pcMbDataAccessBase->getMbMotionData( LIST_0 ).getMv     ( eParIdx8x8, SPART_4x4_0 );
      cBLMvPred [0][1] = pcMbDataAccessBase->getMbMotionData( LIST_0 ).getMv     ( eParIdx8x8, SPART_4x4_1 );
      cBLMvPred [0][2] = pcMbDataAccessBase->getMbMotionData( LIST_0 ).getMv     ( eParIdx8x8, SPART_4x4_2 );
      cBLMvPred [0][3] = pcMbDataAccessBase->getMbMotionData( LIST_0 ).getMv     ( eParIdx8x8, SPART_4x4_3 );
    }
    if( pcMbDataAccessBase->getMbMotionData( LIST_1 ).getRefIdx( eParIdx8x8 ) >  0 &&
        pcMbDataAccessBase->getMbMotionData( LIST_1 ).getRefIdx( eParIdx8x8 ) <= (Int)rcRefFrameList1.getActive() )
    {
      iBLRefIdx [1]    = pcMbDataAccessBase->getMbMotionData( LIST_1 ).getRefIdx ( eParIdx8x8 );
      cBLMvPred [1][0] = pcMbDataAccessBase->getMbMotionData( LIST_1 ).getMv     ( eParIdx8x8, SPART_4x4_0 );
      cBLMvPred [1][1] = pcMbDataAccessBase->getMbMotionData( LIST_1 ).getMv     ( eParIdx8x8, SPART_4x4_1 );
      cBLMvPred [1][2] = pcMbDataAccessBase->getMbMotionData( LIST_1 ).getMv     ( eParIdx8x8, SPART_4x4_2 );
      cBLMvPred [1][3] = pcMbDataAccessBase->getMbMotionData( LIST_1 ).getMv     ( eParIdx8x8, SPART_4x4_3 );
    }
  }

  UInt  uiBasePredType = MSYS_UINT_MAX;

  rpcMbTempData->clear();
  rpcMbTempData->setBlkMode( ePar8x8, BLK_4x4 );


  //===== LIST 0 PREDICTION ======
  for( iRefIdxTest = 1; iRefIdxTest <= (Int)rcRefFrameList0.getActive(); iRefIdxTest++ )
  {
    rpcMbTempData->getMbMotionData( LIST_0 ).setRefIdx( iRefIdxTest, eParIdx8x8 );
    uiBitsTest  = 0;
    uiCostTest  = 0;
    pcRefFrame  = rcRefFrameList0[iRefIdxTest];

    for( UInt uiBlk = 0; uiBlk < 4; uiBlk++ )
    {
      SParIdx4x4  eSubParIdx  = aeParIdx4x4[ uiBlk ];
      UInt        uiTmpBits   = ( uiBlk || uiBasePredType == 0 ? 0 : uiBlkBits[0] + getRefIdxBits( iRefIdxTest, rcRefFrameList0 ) );
      UInt        uiTmpCost;

      rpcMbTempData->getMbDataAccess().getMvPredictor   ( cMvPred[0][iRefIdxTest][uiBlk], iRefIdxTest,
                                                          LIST_0, eParIdx8x8, eSubParIdx );
      cMvLastEst[0][iRefIdxTest][uiBlk] = cMvPred[0][iRefIdxTest][uiBlk];

      Bool bQPel = ( bQPelRefinementOnly &&
                     iRefIdxTest == iBLRefIdx[0] &&
                     cMvPred[0][iRefIdxTest][uiBlk] == cBLMvPred[0][uiBlk] );
      RNOK( m_pcMotionEstimation->estimateBlockWithStart(  *rpcMbTempData, *pcRefFrame,
                                                          cMvLastEst[0][iRefIdxTest][uiBlk],
                                                          cMvPred   [0][iRefIdxTest][uiBlk],
                                                          uiTmpBits, uiTmpCost,
                                                          eParIdx8x8+eSubParIdx, BLK_4x4, bQPel, 0,
                                                          &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdxTest, rpcMbTempData->getFieldFlag()) ) );
      rpcMbTempData->getMbMotionData( LIST_0 ).setAllMv ( cMvLastEst[0][iRefIdxTest][uiBlk], eParIdx8x8, eSubParIdx );

      uiBitsTest  += uiTmpBits;
      uiCostTest  += uiTmpCost;
    }

    if( uiCostTest < uiCost[0] )
    {
      bBLPred [0]     = false;
      iRefIdx [0]     = iRefIdxTest;
      cMv     [0][0]  = cMvLastEst[0][iRefIdxTest][0];
      cMv     [0][1]  = cMvLastEst[0][iRefIdxTest][1];
      cMv     [0][2]  = cMvLastEst[0][iRefIdxTest][2];
      cMv     [0][3]  = cMvLastEst[0][iRefIdxTest][3];
      uiBits  [0]     = uiBitsTest;
      uiCost  [0]     = uiCostTest;
    }

    if( iRefIdxTest == iBLRefIdx[0] )
    {
      uiBitsTest  = 0;
      uiCostTest  = 0;
      for( UInt uiBlk = 0; uiBlk < 4; uiBlk++ )
      {
        SParIdx4x4  eSubParIdx  = aeParIdx4x4[ uiBlk ];
        UInt        uiTmpBits   = ( uiBlk || uiBasePredType == 0 ? 0 : uiBlkBits[0] );
        UInt        uiTmpCost;
        cBLMvLastEst[0][uiBlk]  = cBLMvPred[0][uiBlk];

        rpcMbTempData->getMbDataAccess().setMvPredictorsBL( cBLMvPred[0][uiBlk], LIST_0, eParIdx8x8, eSubParIdx );
        RNOK( m_pcMotionEstimation->estimateBlockWithStart( *rpcMbTempData, *pcRefFrame,
                                                            cBLMvLastEst[0][uiBlk],
                                                            cBLMvPred   [0][uiBlk],
                                                            uiTmpBits, uiTmpCost,
                                                            eParIdx8x8+eSubParIdx, BLK_4x4, bQPelRefinementOnly, 0,
                                                            &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdxTest, rpcMbTempData->getFieldFlag()) ) );
        rpcMbTempData->getMbMotionData( LIST_0 ).setAllMv ( cBLMvLastEst[0][uiBlk], eParIdx8x8, eSubParIdx );
        uiBitsTest  += uiTmpBits;
        uiCostTest  += uiTmpCost;
      }

      if( uiCostTest < uiCost[0] )
      {
        bBLPred [0]     = true;
        iRefIdx [0]     = iRefIdxTest;
        cMv     [0][0]  = cBLMvLastEst[0][0];
        cMv     [0][1]  = cBLMvLastEst[0][1];
        cMv     [0][2]  = cBLMvLastEst[0][2];
        cMv     [0][3]  = cBLMvLastEst[0][3];
        uiBits  [0]     = uiBitsTest;
        uiCost  [0]     = uiCostTest;
      }
    }
  }


  //===== LIST 1 PREDICTION =====
  for( iRefIdxTest = 1; iRefIdxTest <= (Int)rcRefFrameList1.getActive(); iRefIdxTest++ )
  {
    rpcMbTempData->getMbMotionData( LIST_1 ).setRefIdx( iRefIdxTest, eParIdx8x8 );
    uiBitsTest  = 0;
    uiCostTest  = 0;
    pcRefFrame  = rcRefFrameList1[iRefIdxTest];

    for( UInt uiBlk = 0; uiBlk < 4; uiBlk++ )
    {
      SParIdx4x4  eSubParIdx  = aeParIdx4x4[ uiBlk ];
      UInt        uiTmpBits   = ( uiBlk || uiBasePredType == 1 ? 0 : uiBlkBits[1] + getRefIdxBits( iRefIdxTest, rcRefFrameList1 ) );
      UInt        uiTmpCost;

      rpcMbTempData->getMbDataAccess().getMvPredictor   ( cMvPred[1][iRefIdxTest][uiBlk], iRefIdxTest,
                                                          LIST_1, eParIdx8x8, eSubParIdx );
      cMvLastEst[1][iRefIdxTest][uiBlk] = cMvPred[1][iRefIdxTest][uiBlk];

      Bool bQPel = ( bQPelRefinementOnly &&
                     iRefIdxTest == iBLRefIdx[1] &&
                     cMvPred[1][iRefIdxTest][uiBlk] == cBLMvPred[1][uiBlk] );
      RNOK( m_pcMotionEstimation->estimateBlockWithStart( *rpcMbTempData, *pcRefFrame,
                                                          cMvLastEst[1][iRefIdxTest][uiBlk],
                                                          cMvPred   [1][iRefIdxTest][uiBlk],
                                                          uiTmpBits, uiTmpCost,
                                                          eParIdx8x8+eSubParIdx, BLK_4x4, bQPel, 0,
                                                          &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdxTest, rpcMbTempData->getFieldFlag()) ) );
      RNOK( m_pcMotionEstimation->compensateBlock       ( &cTmpYuvMbBuffer,
                                                          eParIdx8x8+eSubParIdx, BLK_4x4 ) );
      rpcMbTempData->getMbMotionData( LIST_1 ).setAllMv ( cMvLastEst[1][iRefIdxTest][uiBlk], eParIdx8x8, eSubParIdx );

      uiBitsTest  += uiTmpBits;
      uiCostTest  += uiTmpCost;
    }

    if( uiCostTest < uiCost[1] )
    {
      bBLPred [1]     = false;
      iRefIdx [1]     = iRefIdxTest;
      cMv     [1][0]  = cMvLastEst[1][iRefIdxTest][0];
      cMv     [1][1]  = cMvLastEst[1][iRefIdxTest][1];
      cMv     [1][2]  = cMvLastEst[1][iRefIdxTest][2];
      cMv     [1][3]  = cMvLastEst[1][iRefIdxTest][3];
      uiBits  [1]     = uiBitsTest;
      uiCost  [1]     = uiCostTest;

      cYuvMbBuffer[1].loadLuma( cTmpYuvMbBuffer, B8x8Idx( ePar8x8 ) );
    }

    if( iRefIdxTest == iBLRefIdx[1] )
    {
      uiBitsTest  = 0;
      uiCostTest  = 0;
      for( UInt uiBlk = 0; uiBlk < 4; uiBlk++ )
      {
        SParIdx4x4  eSubParIdx  = aeParIdx4x4[ uiBlk ];
        UInt        uiTmpBits   = ( uiBlk || uiBasePredType == 1 ? 0 : uiBlkBits[1] );
        UInt        uiTmpCost;
        cBLMvLastEst[1][uiBlk]  = cBLMvPred[1][uiBlk];

        rpcMbTempData->getMbDataAccess().setMvPredictorsBL( cBLMvPred[1][uiBlk], LIST_1, eParIdx8x8, eSubParIdx );
        RNOK( m_pcMotionEstimation->estimateBlockWithStart( *rpcMbTempData, *pcRefFrame,
                                                            cBLMvLastEst[1][uiBlk],
                                                            cBLMvPred   [1][uiBlk],
                                                            uiTmpBits, uiTmpCost,
                                                            eParIdx8x8+eSubParIdx, BLK_4x4, bQPelRefinementOnly, 0,
                                                            &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdxTest, rpcMbTempData->getFieldFlag()) ) );
        RNOK( m_pcMotionEstimation->compensateBlock       ( &cTmpYuvMbBuffer,
                                                            eParIdx8x8+eSubParIdx, BLK_4x4 ) );
        rpcMbTempData->getMbMotionData( LIST_1 ).setAllMv ( cBLMvLastEst[1][uiBlk], eParIdx8x8, eSubParIdx );
        uiBitsTest  += uiTmpBits;
        uiCostTest  += uiTmpCost;
      }

      if( uiCostTest < uiCost[1] )
      {
        bBLPred [1]     = true;
        iRefIdx [1]     = iRefIdxTest;
        cMv     [1][0]  = cBLMvLastEst[1][0];
        cMv     [1][1]  = cBLMvLastEst[1][1];
        cMv     [1][2]  = cBLMvLastEst[1][2];
        cMv     [1][3]  = cBLMvLastEst[1][3];
        uiBits  [1]     = uiBitsTest;
        uiCost  [1]     = uiCostTest;

        cYuvMbBuffer[1].loadLuma( cTmpYuvMbBuffer, B8x8Idx( ePar8x8 ) );
      }
    }
  }

  //===== BI PREDICTION =====
  if( rcRefFrameList0.getActive() && rcRefFrameList1.getActive() )
  {
    //----- initialize with forward and backward estimation -----
    iRefIdxBi [0] = iRefIdx [0];
    iRefIdxBi [1] = iRefIdx [1];
    bBLPredBi [0] = bBLPred [0];
    bBLPredBi [1] = bBLPred [1];

    ::memcpy( cMvBi,      cMv,      2*4*sizeof(Mv ) );

    cMvPredBi [0][0]    = cMvPred [0][iRefIdx[0]][0];
    cMvPredBi [0][1]    = cMvPred [0][iRefIdx[0]][1];
    cMvPredBi [0][2]    = cMvPred [0][iRefIdx[0]][2];
    cMvPredBi [0][3]    = cMvPred [0][iRefIdx[0]][3];
    cMvPredBi [1][0]    = cMvPred [1][iRefIdx[1]][0];
    cMvPredBi [1][1]    = cMvPred [1][iRefIdx[1]][1];
    cMvPredBi [1][2]    = cMvPred [1][iRefIdx[1]][2];
    cMvPredBi [1][3]    = cMvPred [1][iRefIdx[1]][3];
    UInt  uiMotBits[2]  = { uiBits[0] - uiBlkBits[0], uiBits[1] - uiBlkBits[1] };
    uiBits[2]           = uiBlkBits[2] + uiMotBits[0] + uiMotBits[1];

    if( ! uiNumMaxIter )
    {
      uiNumMaxIter      = 1;
      uiIterSearchRange = 0;
    }

    //----- iterative search -----
    for( UInt uiIter = 0; uiIter < uiNumMaxIter; uiIter++ )
    {
      Bool          bChanged        = false;
      UInt          uiDir           = uiIter % 2;
      ListIdx       eListIdx        = ListIdx( uiDir );
      RefFrameList& rcRefFrameList  = ( uiDir ? rcRefFrameList1 : rcRefFrameList0 );
      BSParams      cBSParams;
      cBSParams.pcAltRefFrame       = ( uiDir ? rcRefFrameList0 : rcRefFrameList1 )[ iRefIdxBi[ 1 - uiDir ] ];
      cBSParams.pcAltRefPelData     = &cYuvMbBuffer[1-uiDir];
      cBSParams.uiL1Search          = uiDir;
      cBSParams.apcWeight[LIST_0]   = &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdxBi[LIST_0], rpcMbTempData->getFieldFlag() );
      cBSParams.apcWeight[LIST_1]   = &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdxBi[LIST_1], rpcMbTempData->getFieldFlag() );

      for( iRefIdxTest = 1; iRefIdxTest <= (Int)rcRefFrameList.getActive(); iRefIdxTest++ )
      {
        Mv  cMvPredTest[4];

        rpcMbTempData->getMbMotionData( eListIdx ).setRefIdx( iRefIdxTest, eParIdx8x8 );
        uiBitsTest  = 0;
        uiCostTest  = 0;
        pcRefFrame  = rcRefFrameList[iRefIdxTest];

        for( UInt uiBlk = 0; uiBlk < 4; uiBlk++ )
        {
          SParIdx4x4  eSubParIdx  = aeParIdx4x4[ uiBlk ];
          UInt        uiTmpBits   = ( uiBlk || uiBasePredType == 2 ? 0 : uiBlkBits[2] + uiMotBits[1-uiDir] + getRefIdxBits( iRefIdxTest, rcRefFrameList ) );
          UInt        uiTmpCost;

          rpcMbTempData->getMbDataAccess().getMvPredictor     ( cMvPredTest[uiBlk], iRefIdxTest,
                                                                eListIdx, eParIdx8x8, eSubParIdx );
          Bool bQPel = ( bQPelRefinementOnly &&
                         iRefIdxTest == iBLRefIdx[uiDir] &&
                         cMvPredTest[uiBlk] == cBLMvPred[uiDir][uiBlk] );
          RNOK( m_pcMotionEstimation->estimateBlockWithStart  (  *rpcMbTempData, *pcRefFrame,
                                                                cMvLastEst[uiDir][iRefIdxTest][uiBlk],
                                                                cMvPredTest                   [uiBlk],
                                                                uiTmpBits, uiTmpCost,
                                                                eParIdx8x8+eSubParIdx, BLK_4x4, bQPel,
                                                                uiIterSearchRange, 0, &cBSParams ) );
          RNOK( m_pcMotionEstimation->compensateBlock         ( &cTmpYuvMbBuffer,
                                                                eParIdx8x8+eSubParIdx, BLK_4x4 ) );
          rpcMbTempData->getMbMotionData( eListIdx ).setAllMv ( cMvLastEst[uiDir][iRefIdxTest][uiBlk], eParIdx8x8, eSubParIdx );

          uiBitsTest += uiTmpBits;
          uiCostTest += uiTmpCost;
        }

        if( uiCostTest < uiCost[2] )
        {
          bChanged              = true;
          bBLPredBi [uiDir]     = false;
          iRefIdxBi [uiDir]     = iRefIdxTest;
          cMvBi     [uiDir][0]  = cMvLastEst[uiDir][iRefIdxTest][0];
          cMvBi     [uiDir][1]  = cMvLastEst[uiDir][iRefIdxTest][1];
          cMvBi     [uiDir][2]  = cMvLastEst[uiDir][iRefIdxTest][2];
          cMvBi     [uiDir][3]  = cMvLastEst[uiDir][iRefIdxTest][3];
          cMvPredBi [uiDir][0]  = cMvPredTest                   [0];
          cMvPredBi [uiDir][1]  = cMvPredTest                   [1];
          cMvPredBi [uiDir][2]  = cMvPredTest                   [2];
          cMvPredBi [uiDir][3]  = cMvPredTest                   [3];
          uiMotBits [uiDir]     = uiBitsTest - uiBlkBits[2] - uiMotBits[1-uiDir];
          uiBits    [2]         = uiBitsTest;
          uiCost    [2]         = uiCostTest;

          cYuvMbBuffer[uiDir].loadLuma( cTmpYuvMbBuffer, B8x8Idx( ePar8x8 ) );
        }


        if( iRefIdxTest == iBLRefIdx[uiDir] )
        {
          uiBitsTest  = 0;
          uiCostTest  = 0;
          for( UInt uiBlk = 0; uiBlk < 4; uiBlk++ )
          {
            SParIdx4x4  eSubParIdx  = aeParIdx4x4[ uiBlk ];
            UInt        uiTmpBits   = ( uiBlk || uiBasePredType == 2 ? 0 : uiBlkBits[2] + uiMotBits[1-uiDir] );
            UInt        uiTmpCost;
            RNOK( m_pcMotionEstimation->estimateBlockWithStart  (  *rpcMbTempData, *pcRefFrame,
                                                                  cBLMvLastEst[uiDir][uiBlk],
                                                                  cBLMvPred   [uiDir][uiBlk],
                                                                  uiTmpBits, uiTmpCost,
                                                                  eParIdx8x8+eSubParIdx, BLK_4x4, bQPelRefinementOnly,
                                                                  uiIterSearchRange, 0, &cBSParams ) );
            RNOK( m_pcMotionEstimation->compensateBlock         ( &cTmpYuvMbBuffer,
                                                                  eParIdx8x8+eSubParIdx, BLK_4x4 ) );
            rpcMbTempData->getMbMotionData( eListIdx ).setAllMv ( cBLMvLastEst[uiDir][uiBlk], eParIdx8x8, eSubParIdx );
            uiBitsTest += uiTmpBits;
            uiCostTest += uiTmpCost;
          }

          if( uiCostTest < uiCost[2] )
          {
            bChanged              = true;
            bBLPredBi [uiDir]     = true;
            iRefIdxBi [uiDir]     = iRefIdxTest;
            cMvBi     [uiDir][0]  = cBLMvLastEst[uiDir][0];
            cMvBi     [uiDir][1]  = cBLMvLastEst[uiDir][1];
            cMvBi     [uiDir][2]  = cBLMvLastEst[uiDir][2];
            cMvBi     [uiDir][3]  = cBLMvLastEst[uiDir][3];
            uiMotBits [uiDir]     = uiBitsTest - uiBlkBits[2] - uiMotBits[1-uiDir];
            uiBits    [2]         = uiBitsTest;
            uiCost    [2]         = uiCostTest;

            cYuvMbBuffer[uiDir].loadLuma( cTmpYuvMbBuffer, B8x8Idx( ePar8x8 ) );
          }
        }
      }

      if( ! bChanged )
      {
        break;
      }
    }

    if( bBiPredOnly )
    {
      uiCost[0] = MSYS_UINT_MAX;
      uiCost[1] = MSYS_UINT_MAX;
    }
  }


  //===== chose parameters =====
  if( uiCost[2] <= uiCost[0] && uiCost[2] <= uiCost[1] )
  {
    //----- bi-directional prediction -----
    fRdCost         = uiCost    [2];
    uiSubMbBits     = uiBits    [2];
    iRefIdx [0]     = iRefIdxBi [0];
    iRefIdx [1]     = iRefIdxBi [1];
    bBLPred [0]     = bBLPredBi [0];
    bBLPred [1]     = bBLPredBi [1];
    if( bBLPred[0] )
    {
      cMvPredBi[0][0] = cBLMvPred[0][0];
      cMvPredBi[0][1] = cBLMvPred[0][1];
      cMvPredBi[0][2] = cBLMvPred[0][2];
      cMvPredBi[0][3] = cBLMvPred[0][3];
    }
    if( bBLPred[1] )
    {
      cMvPredBi[1][0] = cBLMvPred[1][0];
      cMvPredBi[1][1] = cBLMvPred[1][1];
      cMvPredBi[1][2] = cBLMvPred[1][2];
      cMvPredBi[1][3] = cBLMvPred[1][3];
    }
    cMv     [0][0]  = cMvBi     [0][0];
    cMv     [0][1]  = cMvBi     [0][1];
    cMv     [0][2]  = cMvBi     [0][2];
    cMv     [0][3]  = cMvBi     [0][3];
    cMv     [1][0]  = cMvBi     [1][0];
    cMv     [1][1]  = cMvBi     [1][1];
    cMv     [1][2]  = cMvBi     [1][2];
    cMv     [1][3]  = cMvBi     [1][3];
    cMvd    [0][0]  = cMv       [0][0]  - cMvPredBi[0][0];
    cMvd    [0][1]  = cMv       [0][1]  - cMvPredBi[0][1];
    cMvd    [0][2]  = cMv       [0][2]  - cMvPredBi[0][2];
    cMvd    [0][3]  = cMv       [0][3]  - cMvPredBi[0][3];
    cMvd    [1][0]  = cMv       [1][0]  - cMvPredBi[1][0];
    cMvd    [1][1]  = cMv       [1][1]  - cMvPredBi[1][1];
    cMvd    [1][2]  = cMv       [1][2]  - cMvPredBi[1][2];
    cMvd    [1][3]  = cMv       [1][3]  - cMvPredBi[1][3];
  }
  else if( uiCost[0] <= uiCost[1] )
  {
    //----- list 0 prediction -----
    fRdCost         = uiCost    [0];
    uiSubMbBits     = uiBits    [0];
    iRefIdx [1]     = BLOCK_NOT_PREDICTED;
    bBLPred [1]     = false;
    if( bBLPred[0] )
    {
      cMvPred[0][iRefIdx[0]][0] = cBLMvPred[0][0];
      cMvPred[0][iRefIdx[0]][1] = cBLMvPred[0][1];
      cMvPred[0][iRefIdx[0]][2] = cBLMvPred[0][2];
      cMvPred[0][iRefIdx[0]][3] = cBLMvPred[0][3];
    }
    cMv     [1][0]  = Mv::ZeroMv();
    cMv     [1][1]  = Mv::ZeroMv();
    cMv     [1][2]  = Mv::ZeroMv();
    cMv     [1][3]  = Mv::ZeroMv();
    cMvd    [0][0]  = cMv[0][0] - cMvPred[0][iRefIdx[0]][0];
    cMvd    [0][1]  = cMv[0][1] - cMvPred[0][iRefIdx[0]][1];
    cMvd    [0][2]  = cMv[0][2] - cMvPred[0][iRefIdx[0]][2];
    cMvd    [0][3]  = cMv[0][3] - cMvPred[0][iRefIdx[0]][3];
  }
  else
  {
    //----- list 1 prediction -----
    fRdCost         = uiCost    [1];
    uiSubMbBits     = uiBits    [1];
    iRefIdx [0]     = BLOCK_NOT_PREDICTED;
    bBLPred [0]     = false;
    if( bBLPred[1] )
    {
      cMvPred[1][iRefIdx[1]][0] = cBLMvPred[1][0];
      cMvPred[1][iRefIdx[1]][1] = cBLMvPred[1][1];
      cMvPred[1][iRefIdx[1]][2] = cBLMvPred[1][2];
      cMvPred[1][iRefIdx[1]][3] = cBLMvPred[1][3];
    }
    cMv     [0][0]  = Mv::ZeroMv();
    cMv     [0][1]  = Mv::ZeroMv();
    cMv     [0][2]  = Mv::ZeroMv();
    cMv     [0][3]  = Mv::ZeroMv();
    cMvd    [1][0]  = cMv[1][0] - cMvPred[1][iRefIdx[1]][0];
    cMvd    [1][1]  = cMv[1][1] - cMvPred[1][iRefIdx[1]][1];
    cMvd    [1][2]  = cMv[1][2] - cMvPred[1][iRefIdx[1]][2];
    cMvd    [1][3]  = cMv[1][3] - cMvPred[1][iRefIdx[1]][3];
  }


  //===== set parameters and compare =====
  rpcMbTempData->rdCost() = fRdCost;
  rpcMbTempData->getMbMotionData( LIST_0 ).setRefIdx( iRefIdx [0],    eParIdx8x8 );
  rpcMbTempData->getMbMotionData( LIST_0 ).setAllMv ( cMv     [0][0], eParIdx8x8, SPART_4x4_0 );
  rpcMbTempData->getMbMotionData( LIST_0 ).setAllMv ( cMv     [0][1], eParIdx8x8, SPART_4x4_1 );
  rpcMbTempData->getMbMotionData( LIST_0 ).setAllMv ( cMv     [0][2], eParIdx8x8, SPART_4x4_2 );
  rpcMbTempData->getMbMotionData( LIST_0 ).setAllMv ( cMv     [0][3], eParIdx8x8, SPART_4x4_3 );
  rpcMbTempData->getMbMvdData   ( LIST_0 ).setAllMv ( cMvd    [0][0], eParIdx8x8, SPART_4x4_0 );
  rpcMbTempData->getMbMvdData   ( LIST_0 ).setAllMv ( cMvd    [0][1], eParIdx8x8, SPART_4x4_1 );
  rpcMbTempData->getMbMvdData   ( LIST_0 ).setAllMv ( cMvd    [0][2], eParIdx8x8, SPART_4x4_2 );
  rpcMbTempData->getMbMvdData   ( LIST_0 ).setAllMv ( cMvd    [0][3], eParIdx8x8, SPART_4x4_3 );
  rpcMbTempData->getMbMotionData( LIST_1 ).setRefIdx( iRefIdx [1],    eParIdx8x8 );
  rpcMbTempData->getMbMotionData( LIST_1 ).setAllMv ( cMv     [1][0], eParIdx8x8, SPART_4x4_0 );
  rpcMbTempData->getMbMotionData( LIST_1 ).setAllMv ( cMv     [1][1], eParIdx8x8, SPART_4x4_1 );
  rpcMbTempData->getMbMotionData( LIST_1 ).setAllMv ( cMv     [1][2], eParIdx8x8, SPART_4x4_2 );
  rpcMbTempData->getMbMotionData( LIST_1 ).setAllMv ( cMv     [1][3], eParIdx8x8, SPART_4x4_3 );
  rpcMbTempData->getMbMvdData   ( LIST_1 ).setAllMv ( cMvd    [1][0], eParIdx8x8, SPART_4x4_0 );
  rpcMbTempData->getMbMvdData   ( LIST_1 ).setAllMv ( cMvd    [1][1], eParIdx8x8, SPART_4x4_1 );
  rpcMbTempData->getMbMvdData   ( LIST_1 ).setAllMv ( cMvd    [1][2], eParIdx8x8, SPART_4x4_2 );
  rpcMbTempData->getMbMvdData   ( LIST_1 ).setAllMv ( cMvd    [1][3], eParIdx8x8, SPART_4x4_3 );

  rpcMbTempData->getMbMotionData( LIST_0 ).setMotPredFlag( bBLPred[0], eParIdx8x8 );
  rpcMbTempData->getMbMotionData( LIST_1 ).setMotPredFlag( bBLPred[1], eParIdx8x8 );
  ROT( bBLPred[0] && iRefIdx[0] != iBLRefIdx[0] );
  ROT( bBLPred[1] && iRefIdx[1] != iBLRefIdx[1] );

  RNOK( xSetRdCostInterSubMb( *rpcMbTempData, rcRefFrameList0, rcRefFrameList1, B8x8Idx( ePar8x8 ), false, uiSubMbBits, bLowComplexMbEnable ) );


  //JVT-R057 LA-RDO{
  if(m_bLARDOEnable)
  {
	  MbDataAccess&   rcMbDataAccess  = rpcMbTempData->getMbDataAccess();
	  Int distortion1=0,distortion2=0,distortion=0;
	  for( Int n = 0; n <4; n++)
	  {
		  iRefIdx [0]=rpcMbTempData->getMbMotionData(LIST_0).getRefIdx(eParIdx8x8);
		  iRefIdx [1]=rpcMbTempData->getMbMotionData(LIST_1).getRefIdx(eParIdx8x8);
		  Frame* pcRefFrame0 = ( iRefIdx [0] > 0 ? rcRefFrameList0[ iRefIdx [0] ] : NULL );
		  Frame* pcRefFrame1 = ( iRefIdx [1] > 0 ? rcRefFrameList1[ iRefIdx [1] ] : NULL );
		  Int iMvX;
		  Int iMvY;

		  if(pcRefFrame0)
		  {
			  iMvX=cMv[LIST_0][n].getHor();
			  iMvY=cMv[LIST_0][n].getVer();
			  getChannelDistortion(rcMbDataAccess,*pcRefFrame0,&distortion1,iMvX,iMvY,(eParIdx8x8+aeParIdx4x4[n])%4,(eParIdx8x8+aeParIdx4x4 [n])/4,1,1);
		  }
		  if(pcRefFrame1)
		  {
			  iMvX=cMv[LIST_1][n].getHor();
			  iMvY=cMv[LIST_1][n].getVer();
			  getChannelDistortion(rcMbDataAccess,*pcRefFrame1,&distortion2,iMvX,iMvY,(eParIdx8x8+aeParIdx4x4[n])%4,(eParIdx8x8+aeParIdx4x4 [n])/4,1,1);
			  if(pcRefFrame0)
				  distortion1=(Int)(m_dWr0*distortion1+m_dWr0*distortion2);
			  else
				  distortion1=distortion2;
		  }
		  distortion+=distortion1;
	  }
	  rpcMbTempData->rdCost()+=distortion;
  }
  //JVT-R057 LA-RDO}

  RNOK( xCheckBestEstimation(  rpcMbTempData, rpcMbBestData ) );

  return Err::m_nOK;
}




//TMM_WP
ErrVal MbEncoder::getPredWeights( SliceHeader& rcSH, ListIdx eLstIdx,
                                  Double(*pafWeight)[3], Frame* pOrgFrame,
                                  RefFrameList& rcRefFrameListX)
{
  YuvPicBuffer *pcOrgPicBuffer;
  YuvPicBuffer *pcRefPicBuffer;
  Frame* pRefFrame;

  pcOrgPicBuffer = pOrgFrame->getFullPelYuvBuffer();

  Int iRefPic = 0;

  for( iRefPic = 0; iRefPic < (Int)rcRefFrameListX.getActive(); iRefPic++ )
  {
      pRefFrame = rcRefFrameListX.getEntry(iRefPic);
      pcRefPicBuffer = pRefFrame->getFullPelYuvBuffer();

      m_pcXDistortion->getLumaWeight(  pcOrgPicBuffer, pcRefPicBuffer, pafWeight[iRefPic][0], rcSH.getLumaLog2WeightDenom() );
      m_pcXDistortion->getChromaWeight(pcOrgPicBuffer, pcRefPicBuffer, pafWeight[iRefPic][1], rcSH.getChromaLog2WeightDenom(), true );
      m_pcXDistortion->getChromaWeight(pcOrgPicBuffer, pcRefPicBuffer, pafWeight[iRefPic][2], rcSH.getChromaLog2WeightDenom(), false );

//      printf( "\n%2.4f  %2.4f %2.4f", pafWeight[iRefPic][0],pafWeight[iRefPic][1],pafWeight[iRefPic][2]);
  }

  return Err::m_nOK;
}

ErrVal MbEncoder::getPredOffsets( SliceHeader& rcSH, ListIdx eLstIdx,
                                  Double(*pafOffsets)[3], Frame* pOrgFrame,
                                  RefFrameList& rcRefFrameListX)
{
  YuvPicBuffer *pcOrgPicBuffer;
  YuvPicBuffer *pcRefPicBuffer;
  Frame* pRefFrame;

  pcOrgPicBuffer = pOrgFrame->getFullPelYuvBuffer();

  Int iRefPic = 0;

  for( iRefPic = 0; iRefPic < (Int)rcRefFrameListX.getActive(); iRefPic++ )
  {
      pRefFrame = rcRefFrameListX.getEntry(iRefPic);
      pcRefPicBuffer = pRefFrame->getFullPelYuvBuffer();

      m_pcXDistortion->getLumaOffsets(  pcOrgPicBuffer, pcRefPicBuffer, pafOffsets[iRefPic][0] );
      m_pcXDistortion->getChromaOffsets(pcOrgPicBuffer, pcRefPicBuffer, pafOffsets[iRefPic][1], true );
      m_pcXDistortion->getChromaOffsets(pcOrgPicBuffer, pcRefPicBuffer, pafOffsets[iRefPic][2], false );

//      printf( "\nOffsets:%2.4f  %2.4f %2.4f\n", pafOffsets[iRefPic][0],pafOffsets[iRefPic][1],pafOffsets[iRefPic][2]);
  }

  return Err::m_nOK;
}
//TMM_WP



//JVT-R057 LA-RDO{
Int
MbEncoder::GetEC_REC(YuvPicBuffer* pPic1,
                     YuvPicBuffer* pPic2,
                     Int              blockX,
                     Int              blockY)
{

	XPel* pS1,*pS2;
	Int   iStride = pPic1->getLStride();
	Int uiDiff;
	UInt uiSSD;
  Int j, i;

	uiSSD=0;
	pS1=pPic1->getMbLumAddr();
	pS2=pPic2->getMbLumAddr();
	for(j=blockY*4;j<blockY*4+4;j++)
	{
		for( i=blockX*4;i<blockX*4+4;i++)
		{
			uiDiff=pS1[j*iStride+i]-pS2[j*iStride+i];
			uiSSD=uiSSD+uiDiff*uiDiff;
		}
	}
	pS1=pPic1->getMbCbAddr();
	pS2=pPic2->getMbCbAddr();
	for( j=blockY*2;j<blockY*2+2;j++)
	{
		for( i=blockX*2;i<blockX*2+2;i++)
		{
			uiDiff=pS1[j*(iStride/2)+i]-pS2[j*(iStride/2)+i];
			uiSSD=uiSSD+uiDiff*uiDiff;
		}
	}
	pS1=pPic1->getMbCrAddr();
	pS2=pPic2->getMbCrAddr();
	for( j=blockY*2;j<blockY*2+2;j++)
	{
		for( i=blockX*2;i<blockX*2+2;i++)
		{
			uiDiff=pS1[j*(iStride/2)+i]-pS2[j*(iStride/2)+i];
			uiSSD=uiSSD+uiDiff*uiDiff;
		}
	}
	return uiSSD;
}

Void
MbEncoder::getChannelDistortion(MbDataAccess&   rcMbDataAccess,
								Frame&       rcRefFrame,
								                Int             *distortion,
								                Int             iMvX,
								                Int             iMvY,
								                Int             startX,
								                Int             startY,
								                Int             blockX,
								                Int             blockY,
								                Bool            bSpatial)
{
#define MBK_SIZE 16
#define BLK_PER_MB 4
#define BLK_SIZE 4

	Int blkIdxX, blkIdxY;
	Int i0, j0;
	Int i1, j1;
	Int i2, j2;
	Int k0, l0;
	Int picWidth;
	Int picHeight;
	Int mbIdxRef;
	Int mbkPerLine;
	UInt *pDistortion;

	distortion[0] = 0;


	YuvPicBuffer* pTemp;
	pTemp=rcRefFrame.getFullPelYuvBuffer();

	picWidth   = pTemp->getLWidth();
	mbkPerLine = picWidth / MBK_SIZE;


	picHeight=pTemp->getLHeight();



	// 1:    (1-p) * Dc(n-1, j)
	for (blkIdxY = startY; blkIdxY <startY+blockY; blkIdxY += 1)
	{
		for (blkIdxX = startX; blkIdxX < startX+blockX; blkIdxX += 1)
		{

			// the starting position of current block in pixel: k0, l0
			k0 = (rcMbDataAccess.getMbX() * BLK_PER_MB + blkIdxX) * BLK_SIZE;
			l0 = (rcMbDataAccess.getMbY() * BLK_PER_MB + blkIdxY) * BLK_SIZE;

			// Absolute motion vector coordinates of the macroblock
			pDistortion = rcRefFrame.getChannelDistortion();

			i0 = k0 * 4+iMvX ;
			j0 = l0 * 4+iMvY ;

			// the starting position of the ref block in pixel: i0, j0
			i0 = i0 / 4;
			j0 = j0 / 4;
			if (i0 < 0) i0 = 0;
			if (j0 < 0) j0 = 0;
			if (i0 >= picWidth) i0 = picWidth - 1;
			if (j0 >= picHeight) j0 = picHeight - 1;

			// calculate the distortion here:
			for (j1 = j0; j1 < j0 + 4; j1++) {
				for (i1 = i0; i1 < i0 + 4; i1++) {
					i2 = i1;
					j2 = j1;

					if (i2 >= picWidth) i2 = picWidth - 1;
					if (j2 >= picHeight) j2 = picHeight - 1;
					//Bug_Fix JVT-R057 0806{
					//if(bSpatial)
					//	mbIdxRef = (j2 / MBK_SIZE/2*4) * (mbkPerLine/2*4) + i2 / MBK_SIZE/2*4;
					//else
					//	mbIdxRef  = (j2 / MBK_SIZE*4) * (mbkPerLine*4) + i2 / MBK_SIZE*4;
					mbIdxRef  = (j2 / 4) * (mbkPerLine*4) + i2 / 4;
					//Bug_Fix JVT-R057 0806}
					distortion[0] += pDistortion[mbIdxRef];  //  / 256.0
				}
			}

		}
	}
	distortion[0] = distortion[0] >> 4; //  / 256
}

//JVT-R057 LA-RDO}


// for residual prediction in transform demain. jzhao@sharplabs
Void
MbEncoder::reCalcBlock8x8(IntMbTempData& rcMbTempData, B8x8Idx c8x8Idx, Int is4x4)
{  
  TCoeff* pcCoeff, *piCoeffBase;

  if (!is4x4) // 8x8 mode
  {
    pcCoeff = rcMbTempData.get8x8( c8x8Idx );  
    piCoeffBase = rcMbTempData.getMbDataAccess().getMbDataAccessBase()->getMbTCoeffs().get8x8( c8x8Idx );

    UInt ui=0;
	for( ui=0; ui<64; ui++ )
    {
      pcCoeff[ui] = piCoeffBase[ui].getLevel();
      pcCoeff[ui].setLevel(piCoeffBase[ui].getLevel());      
    }

    m_pcTransform->invTransform8x8Blk( rcMbTempData.getYBlk( c8x8Idx ),  rcMbTempData.getLStride(), pcCoeff );  

    // only clear the coefficients
    for( ui=0; ui<64; ui++ )
    {
      pcCoeff[ui].setCoeff(0);      
    }
  }
  else {      // 4x4 mode
    for( S4x4Idx cIdx( c8x8Idx ); cIdx.isLegal( c8x8Idx ); cIdx++ )
    {
      pcCoeff = rcMbTempData.get( cIdx );
      piCoeffBase = rcMbTempData.getMbDataAccess().getMbDataAccessBase()->getMbTCoeffs().get( cIdx );

      // add the base layer dequantized coeff
      UInt ui=0;
	  for( ui=0; ui<16; ui++ )
      {
        pcCoeff[ui] = piCoeffBase[ui].getLevel();
        pcCoeff[ui].setLevel(piCoeffBase[ui].getLevel());
      }

      m_pcTransform->invTransform4x4Blk( rcMbTempData.getYBlk( cIdx ),
        rcMbTempData.getLStride(), pcCoeff );

      // only clear the coefficients
      for( ui=0; ui<16; ui++ )
      {
        pcCoeff[ui].setCoeff(0);      
      }
    }
  }  
}

// for CGS SVC-AVC rewrite  JVT-V035
ErrVal
MbEncoder::reCalcBlock8x8Rewrite(IntMbTempData& rcMbTempData, B8x8Idx c8x8Idx, Int is4x4)
{  
  TCoeff* pcCoeff;

  if (!is4x4) // 8x8 mode
  {
    pcCoeff = rcMbTempData.get8x8( c8x8Idx );
    UInt uiAbsSum = 0;

    RNOK( m_pcTransform->predict8x8Blk( pcCoeff,
      rcMbTempData.getMbDataAccess().getMbDataAccessBase()->getMbTCoeffs().get8x8( c8x8Idx ),
      rcMbTempData.getMbDataAccess().getMbDataAccessBase()->getMbData().getQp(),
      uiAbsSum ) );

    for( unsigned int i=0; i<64; i++ )
      pcCoeff[i] = -pcCoeff[i];

    xScale8x8Block( pcCoeff, NULL, m_pcTransform->getLumaQp() );

    m_pcTransform->invTransform8x8Blk( rcMbTempData.getYBlk( c8x8Idx ),
      rcMbTempData.getLStride(), pcCoeff );
    // only clear the coefficients
    for( UInt ui=0; ui<64; ui++ )
    {
      pcCoeff[ui].setLevel(pcCoeff[ui].getCoeff());
      pcCoeff[ui].setCoeff(0);      
    }
  }
  else  // 4x4 mode
  {
    for( S4x4Idx cIdx( c8x8Idx ); cIdx.isLegal( c8x8Idx ); cIdx++ )
    {
      pcCoeff = rcMbTempData.get( cIdx );
      UInt uiAbsSum = 0;

      RNOK( m_pcTransform->predict4x4Blk( pcCoeff,
        rcMbTempData.getMbDataAccess().getMbDataAccessBase()->getMbTCoeffs().get( cIdx ),
        rcMbTempData.getMbDataAccess().getMbDataAccessBase()->getMbData().getQp(),
        uiAbsSum ) );

      for( unsigned int i=0; i<16; i++ )
        pcCoeff[i] = -pcCoeff[i];

      xScale4x4Block( pcCoeff, NULL, 0, m_pcTransform->getLumaQp() );

      m_pcTransform->invTransform4x4Blk( rcMbTempData.getYBlk( cIdx ), rcMbTempData.getLStride(), pcCoeff );

      // only clear the coefficients
      for( UInt ui=0; ui<16; ui++ )
      {
        pcCoeff[ui].setLevel(pcCoeff[ui].getCoeff());
        pcCoeff[ui].setCoeff(0);      
      }
    }
  }

  return Err::m_nOK;
}

H264AVC_NAMESPACE_END
