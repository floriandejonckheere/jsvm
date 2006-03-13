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
#include "H264AVCCommonLib/FrameMng.h"

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
  m_pcFrameMng( NULL ),
  m_BitCounter( NULL )
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
  m_bISlice                 = rcSH.isIntra ();
  m_bBSlice                 = rcSH.isInterB();
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
                 FrameMng*              pcFrameMng,
                 XDistortion*           pcXDistortion )
{
  ROT( NULL == pcFrameMng );
  ROT( NULL == pcTransform );
  ROT( NULL == pcIntraPrediction );
  ROT( NULL == pcMotionEstimation );
  ROT( NULL == pcCodingParameter );
  ROT( NULL == pcRateDistortionIf );
  ROT( NULL == pcXDistortion );

  m_pcFrameMng            = pcFrameMng;
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


Void
MbEncoder::xSetMaxRefPics( MbDataAccess& rcMbDataAccess )
{
  m_uiMaxRefPics[ LIST_0 ] = m_uiMaxRefFrames[ LIST_0 ];
  m_uiMaxRefPics[ LIST_1 ] = m_uiMaxRefFrames[ LIST_1 ];
}



ErrVal
MbEncoder::encodeIntra( MbDataAccess&  rcMbDataAccess,
                        MbDataAccess*  pcMbDataAccessBase,
                        IntFrame*      pcFrame,
                        IntFrame*      pcRecSubband,
                        IntFrame*      pcBaseLayer,
                        IntFrame*      pcPredSignal,
                        Double         dLambda )
{
  ROF( bInitDone );

  UInt  uiQp    = rcMbDataAccess.getMbData().getQp();
  RNOK( m_pcRateDistortionIf->setMbQpLambda( rcMbDataAccess, uiQp, dLambda ) )

  m_pcIntMbBestIntraChroma  = NULL;
  m_pcIntMbBestData   ->init( rcMbDataAccess );
  m_pcIntMbTempData   ->init( rcMbDataAccess );
  m_pcIntMbBest8x8Data->init( rcMbDataAccess );
  m_pcIntMbTemp8x8Data->init( rcMbDataAccess );

  m_pcIntPicBuffer = pcFrame->getFullPelYuvBuffer();
  m_pcXDistortion->loadOrgMbPelData( m_pcIntPicBuffer, m_pcIntOrgMbPelData );
  m_pcTransform->setQp( rcMbDataAccess, true );

  if( rcMbDataAccess.getSH().getBaseLayerId() != MSYS_UINT_MAX )
  {
    ROF ( pcBaseLayer );
    //===== check intra base mode (if base layer is available) =====

    if( pcMbDataAccessBase->getMbData().isIntra() || !rcMbDataAccess.isConstrainedInterLayerPred( pcMbDataAccessBase ) )
    if ( pcMbDataAccessBase->getMbData().getInCropWindowFlag() ) // TMM_ESS
    RNOK( xEstimateMbIntraBL( m_pcIntMbTempData, m_pcIntMbBestData, pcBaseLayer, false, pcMbDataAccessBase  ) );
  }
  if( rcMbDataAccess.getSH().getBaseLayerId() == MSYS_UINT_MAX ||
      rcMbDataAccess.getSH().getAdaptivePredictionFlag() )
  {
    //===== check normal intra modes (if base layer or adaptive prediction) =====
    RNOK( xEstimateMbIntra16( m_pcIntMbTempData, m_pcIntMbBestData,              false  ) );
    RNOK( xEstimateMbIntra8 ( m_pcIntMbTempData, m_pcIntMbBestData,              false  ) );
    RNOK( xEstimateMbIntra4 ( m_pcIntMbTempData, m_pcIntMbBestData,              false  ) );
    RNOK( xEstimateMbPCM    ( m_pcIntMbTempData, m_pcIntMbBestData,              false  ) );
  }

  RNOK( m_pcRateDistortionIf->fixMacroblockQP( *m_pcIntMbBestData ) );
  xStoreEstimation( rcMbDataAccess, *m_pcIntMbBestData, pcRecSubband, pcPredSignal, false, NULL );
  
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
                                 MbDataAccess*     pcMbDataAccessBaseMotion )
{
  ROTRS( ! rpcMbTempData->getSH().getPPS().getTransform8x8ModeFlag(), Err::m_nOK );
  ROTRS( ! pcMbRefData->is8x8TrafoFlagPresent(),                      Err::m_nOK );

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
    rpcMbTempData->setBLQRefFlag        (           pcMbRefData->getBLQRefFlag        () );
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

  RNOK( xSetRdCost8x8InterMb( *rpcMbTempData, pcMbDataAccessBaseMotion, rcRefFrameList0, rcRefFrameList1 ) );
  RNOK( xCheckBestEstimation(  rpcMbTempData, rpcMbBestData ) );

  return Err::m_nOK;
}



ErrVal
MbEncoder::encodeMacroblock( MbDataAccess&  rcMbDataAccess,
                             IntFrame*      pcFrame,
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
  m_pcTransform->setQp( rcMbDataAccess, rcMbDataAccess.getSH().getKeyPictureFlag() );

  //====== evaluate macroblock modes ======
  if( rcMbDataAccess.getSH().isInterP() )
  {
    RNOK( xEstimateMbSkip     ( m_pcIntMbTempData,  m_pcIntMbBestData,  rcList0,  rcList1 ) );
  }
  if( rcMbDataAccess.getSH().isInterB() )
  {
    RNOK( xEstimateMbDirect   ( m_pcIntMbTempData,  m_pcIntMbBestData,  rcList0,  rcList1,                                                    NULL, false ) );
  }
  if( rcMbDataAccess.getSH().isInterP() || rcMbDataAccess.getSH().isInterB() )
  {
    RNOK( xEstimateMb16x16    ( m_pcIntMbTempData,  m_pcIntMbBestData,  rcList0,  rcList1,  false,  uiNumMaxIter, uiIterSearchRange,  false,  NULL, false ) );
    RNOK( xEstimateMb16x8     ( m_pcIntMbTempData,  m_pcIntMbBestData,  rcList0,  rcList1,  false,  uiNumMaxIter, uiIterSearchRange,  false,  NULL, false ) );
    RNOK( xEstimateMb8x16     ( m_pcIntMbTempData,  m_pcIntMbBestData,  rcList0,  rcList1,  false,  uiNumMaxIter, uiIterSearchRange,  false,  NULL, false ) );
    RNOK( xEstimateMb8x8      ( m_pcIntMbTempData,  m_pcIntMbBestData,  rcList0,  rcList1,  false,  uiNumMaxIter, uiIterSearchRange,  false,  NULL, false ) );
    RNOK( xEstimateMb8x8Frext ( m_pcIntMbTempData,  m_pcIntMbBestData,  rcList0,  rcList1,  false,  uiNumMaxIter, uiIterSearchRange,  false,  NULL, false ) );
  }
  RNOK(   xEstimateMbIntra16  ( m_pcIntMbTempData,  m_pcIntMbBestData,  rcMbDataAccess.getSH().isInterB() ) );
  RNOK(   xEstimateMbIntra8   ( m_pcIntMbTempData,  m_pcIntMbBestData,  rcMbDataAccess.getSH().isInterB() ) );
  RNOK(   xEstimateMbIntra4   ( m_pcIntMbTempData,  m_pcIntMbBestData,  rcMbDataAccess.getSH().isInterB() ) );
  RNOK(   xEstimateMbPCM      ( m_pcIntMbTempData,  m_pcIntMbBestData,  rcMbDataAccess.getSH().isInterB() ) );

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
                         IntFrame*        pcFrame,
                         IntFrame*        pcRecSubband,
                         IntFrame*        pcPredSignal,
                         IntFrame*        pcBaseLayerRec,
                         IntFrame*        pcBaseLayerSbb,
                         RefFrameList&    rcRefFrameList0,
                         RefFrameList*    pcRefFrameList0Base,
                         Double           dLambda )
{
  ROF( bInitDone );

  UInt  uiQp  = rcMbDataAccess.getMbData().getQp();
  RNOK( m_pcRateDistortionIf->setMbQpLambda( rcMbDataAccess, uiQp, dLambda ) )

  m_pcIntMbBestIntraChroma  = NULL;
  m_pcIntMbBestData   ->init( rcMbDataAccess );
  m_pcIntMbTempData   ->init( rcMbDataAccess );
  m_pcIntMbBest8x8Data->init( rcMbDataAccess );
  m_pcIntMbTemp8x8Data->init( rcMbDataAccess );

  m_pcIntPicBuffer = pcFrame->getFullPelYuvBuffer();
  m_pcXDistortion->loadOrgMbPelData( m_pcIntPicBuffer, m_pcIntOrgMbPelData );
  m_pcTransform->setQp      ( rcMbDataAccess, true );
  m_pcTransform->setClipMode( false );


  RefFrameList   cRefFrameList1;
  IntYuvMbBuffer cBaseLayerBuffer;


  //===== residual prediction =====
  if( rcMbDataAccess.getSH().getBaseLayerId() != MSYS_UINT_MAX )
  {
    ROF( pcBaseLayerSbb );
    cBaseLayerBuffer    .loadBuffer ( pcBaseLayerSbb->getFullPelYuvBuffer() );
    if( ! cBaseLayerBuffer.isZero() ) // HS: search only with residual prediction, when residual signal is non-zero
    {
      m_pcIntOrgMbPelData->subtract   ( cBaseLayerBuffer );

      if( ! pcMbDataAccessBase->getMbData().isIntra() )
      {
        //--- only if base layer is in inter mode ---
          
          // TMM_ESS 
	      if ( pcMbDataAccessBase->getMbData().getInCropWindowFlag() ) 
					RNOK( xEstimateMbBLSkip   ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, cRefFrameList1, pcBaseLayerRec, false, iSpatialScalabilityType,  pcMbDataAccessBase, true ) );
      }

      if( rcMbDataAccess.getSH().getAdaptivePredictionFlag() )
      {
        //--- only if adaptive inter-layer prediction ---
        RNOK( xEstimateMb16x16    ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, cRefFrameList1, false, 0, 0, false,                        pcMbDataAccessBase, true ) );
        RNOK( xEstimateMb16x8     ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, cRefFrameList1, false, 0, 0, false,                        pcMbDataAccessBase, true ) );
        RNOK( xEstimateMb8x16     ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, cRefFrameList1, false, 0, 0, false,                        pcMbDataAccessBase, true ) );
        RNOK( xEstimateMb8x8      ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, cRefFrameList1, false, 0, 0, false,                        pcMbDataAccessBase, true ) );
        RNOK( xEstimateMb8x8Frext ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, cRefFrameList1, false, 0, 0, false,                        pcMbDataAccessBase, true ) );
      }

      m_pcIntOrgMbPelData->add( cBaseLayerBuffer );
    }
  }


  //===== intra base layer mode =====
  if( rcMbDataAccess.getSH().getBaseLayerId() != MSYS_UINT_MAX )
  {
    //===== only when intra BL is allowed =====
    if( pcMbDataAccessBase->getMbData().isIntra() || ! rcMbDataAccess.isConstrainedInterLayerPred( pcMbDataAccessBase ) )
    {
		  if ( pcMbDataAccessBase->getMbData().getInCropWindowFlag() ) // TMM_ESS
			  RNOK( xEstimateMbIntraBL( m_pcIntMbTempData, m_pcIntMbBestData, pcBaseLayerRec, false, pcMbDataAccessBase ) );
    }
  }


  //===== without residual prediction =====
  if( rcMbDataAccess.getSH().getBaseLayerId           () == MSYS_UINT_MAX ||
      rcMbDataAccess.getSH().getAdaptivePredictionFlag() )
  {
    if( ( pcMbDataAccessBase && pcMbDataAccessBase->getMbData().isIntra() ) || rcMbDataAccess.getSH().getAdaptivePredictionFlag() )
    {
      //--- only if base layer is in intra mode or adaptive prediction is enabled ---
      // TMM_ESS 
      if ( pcMbDataAccessBase->getMbData().getInCropWindowFlag() ) 
				RNOK( xEstimateMbBLSkip   ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, cRefFrameList1, pcBaseLayerRec, false, iSpatialScalabilityType,  pcMbDataAccessBase, false ) );
    }

    // if 2 reference frames are supplied, do not evaluate the skip mode here
    if( pcRefFrameList0Base == 0 )
      RNOK  ( xEstimateMbSkip     ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, cRefFrameList1 ) );
    RNOK  ( xEstimateMb16x16    ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, cRefFrameList1, false, 0, 0, false,                        pcMbDataAccessBase, false ) );
    RNOK  ( xEstimateMb16x8     ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, cRefFrameList1, false, 0, 0, false,                        pcMbDataAccessBase, false ) );
    RNOK  ( xEstimateMb8x16     ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, cRefFrameList1, false, 0, 0, false,                        pcMbDataAccessBase, false ) );
    RNOK  ( xEstimateMb8x8      ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, cRefFrameList1, false, 0, 0, false,                        pcMbDataAccessBase, false ) );
    RNOK  ( xEstimateMb8x8Frext ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, cRefFrameList1, false, 0, 0, false,                        pcMbDataAccessBase, false ) );
  }

  // motion estimation was made with the enhancement reference frame
  // cost evaluation with the actual reference frame
  if( pcRefFrameList0Base != 0 && ! m_pcIntMbBestData->getMbDataAccess().getMbData().isIntra() )
  {
    Bool bResidualPredUsed;

    bResidualPredUsed = false;
    if( rcMbDataAccess.getSH().getBaseLayerId() != MSYS_UINT_MAX )
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
      RNOK( xSetRdCostInterMb   ( *m_pcIntMbBestData, pcMbDataAccessBase, *pcRefFrameList0Base, cRefFrameList1 ) );
    }

    if( bResidualPredUsed )
      m_pcIntOrgMbPelData->add( cBaseLayerBuffer );

    // get skip mode motion vector
    Mv  cMvPredL0, cCurrentMv;

    m_pcIntMbBestData->getMbDataAccess().getMvPredictorSkipMode( cMvPredL0 );
    cCurrentMv = m_pcIntMbBestData->getMbMotionData( LIST_0 ).getMv();

    // check skip mode only when motion vector is equal to skip mode motion vector
    if (m_pcIntMbBestData->getMbMode() == MODE_16x16 && cCurrentMv == cMvPredL0 )
    {
      RNOK  ( xEstimateMbSkip     ( m_pcIntMbTempData, m_pcIntMbBestData, *pcRefFrameList0Base, cRefFrameList1 ) );
    }
  }

  m_pcTransform->setClipMode( true );

  //==== normal intra modes =====
  if( rcMbDataAccess.getSH().getBaseLayerId           () == MSYS_UINT_MAX ||
      rcMbDataAccess.getSH().getAdaptivePredictionFlag() )
  {
    RNOK  ( xEstimateMbIntra16  ( m_pcIntMbTempData, m_pcIntMbBestData,                 false ) );
    RNOK  ( xEstimateMbIntra8   ( m_pcIntMbTempData, m_pcIntMbBestData,                 false ) );
    RNOK  ( xEstimateMbIntra4   ( m_pcIntMbTempData, m_pcIntMbBestData,                 false ) );
    RNOK  ( xEstimateMbPCM      ( m_pcIntMbTempData, m_pcIntMbBestData,                 false ) );
  }

  RNOK( m_pcRateDistortionIf->fixMacroblockQP( *m_pcIntMbBestData ) );
  xStoreEstimation( rcMbDataAccess, *m_pcIntMbBestData, pcRecSubband, pcPredSignal, false, &cBaseLayerBuffer  );

  m_pcIntMbBestData   ->uninit();
  m_pcIntMbTempData   ->uninit();
  m_pcIntMbBest8x8Data->uninit();
  m_pcIntMbTemp8x8Data->uninit();

  return Err::m_nOK;
}





ErrVal
MbEncoder::encodeResidual( MbDataAccess&  rcMbDataAccess,
                           MbDataAccess*  pcMbDataAccessBase,
                           IntFrame*      pcFrame,
                           IntFrame*      pcResidual,
                           IntFrame*      pcBaseSubband,
													 IntFrame*			pcSRFrame, // JVT-R091
                           Bool&          rbCoded,
                           Double         dLambda,
                           Int            iMaxQpDelta )
{
  ROF( bInitDone );

  m_pcIntPicBuffer    = pcFrame->getFullPelYuvBuffer();

  UInt    uiMinTrafo  = 0;
  UInt    uiMaxTrafo  = ( ( rcMbDataAccess.getSH().getPPS().getTransform8x8ModeFlag() && rcMbDataAccess.getMbData().is8x8TrafoFlagPresent() ) ? 2 : 1 );
  UChar   ucMinQp     = (UChar)min( MAX_QP, max( MIN_QP, rcMbDataAccess.getMbData().getQp() - iMaxQpDelta ) );
  UChar   ucMaxQp     = (UChar)min( MAX_QP, max( MIN_QP, rcMbDataAccess.getMbData().getQp() + iMaxQpDelta ) );
  Double  dMinCost    = 1e30;
  UInt    uiDist, uiRate;
  Bool    bCoded;
  Double  dCost;

  UInt    uiRPred     = ( rcMbDataAccess.getSH().getBaseLayerId           () == MSYS_UINT_MAX ? 0 :
                          rcMbDataAccess.getSH().getAdaptivePredictionFlag()                  ? 2 : 1 );
  Int     iMinCnt     = ( uiRPred == 0 || uiRPred == 2 ? 0 : 1 );
  Int     iMaxCnt     = ( uiRPred == 1 || uiRPred == 2 ? 2 : 1 );

	//-- JVT-R091
	Bool		bSmoothedRef = false;
	rcMbDataAccess.getMbData().setSmoothedRefFlag( false );
	if ( uiRPred == 2 && rcMbDataAccess.getMbData().getBLSkipFlag() && rcMbDataAccess.isConstrainedInterLayerPred( pcMbDataAccessBase ) )
	{
		iMaxCnt = 3;
	}
	//--
  
  m_pcXDistortion->loadOrgMbPelData( m_pcIntPicBuffer, m_pcIntOrgMbPelData );

  for( Int iCnt = iMinCnt; iCnt < iMaxCnt; iCnt++ )
  {
		//-- JVT-R091
		IntYuvMbBuffer cPrdMbBuffer, cBaseResMbBuffer, cMbBuffer, cNewPrdMbBuffer, cOrgMbBuffer;

		// store original residual
		cOrgMbBuffer		.loadLuma	 ( *m_pcIntOrgMbPelData										);
		cOrgMbBuffer		.loadChroma( *m_pcIntOrgMbPelData										);

		bSmoothedRef = ( iCnt == 2 );
		if ( bSmoothedRef )
		{
			// obtain P & Rb
			cPrdMbBuffer		.loadBuffer( pcSRFrame->getFullPelYuvBuffer()			);
			cBaseResMbBuffer.loadBuffer( pcBaseSubband->getFullPelYuvBuffer()	);

			// P+Rb & save to pcPredSR
			cMbBuffer				.loadBuffer( pcSRFrame->getFullPelYuvBuffer()			);
			cMbBuffer				.add			 ( cBaseResMbBuffer											);
			pcSRFrame->getFullPelYuvBuffer()->loadBuffer( &cMbBuffer );

			// S(P+Rb) & save to cMbBuffer
			pcSRFrame->getFullPelYuvBuffer()->smoothMbInside();
			if ( rcMbDataAccess.isAboveMbExisting() )
			{
				pcSRFrame->getFullPelYuvBuffer()->smoothMbTop();
			}
			if ( rcMbDataAccess.isLeftMbExisting() )
			{
				pcSRFrame->getFullPelYuvBuffer()->smoothMbLeft();
			}
			cMbBuffer.loadBuffer( pcSRFrame->getFullPelYuvBuffer() );

			// restore pcPredSR
			pcSRFrame->getFullPelYuvBuffer()->loadBuffer( &cPrdMbBuffer );

			// compute new prediction -> S(P+Rb)-Rb
			cNewPrdMbBuffer.loadLuma			( cMbBuffer					);
			cNewPrdMbBuffer.loadChroma		( cMbBuffer					);
			cNewPrdMbBuffer.subtract			( cBaseResMbBuffer	);

			// compute new residual -> O-S(P+Rb)+Rb
			m_pcIntOrgMbPelData->add			( cPrdMbBuffer			);
			m_pcIntOrgMbPelData->subtract	( cNewPrdMbBuffer		);
		}
		//--

    for( UChar ucQp = ucMinQp; ucQp <= ucMaxQp; ucQp++ )
    {
      for( UInt uiTrafo8x8 = uiMinTrafo; uiTrafo8x8 < uiMaxTrafo; uiTrafo8x8++ )
      {
        m_pcIntMbTempData ->init( rcMbDataAccess );

				if( iCnt )
        {
          m_pcIntMbTempData->loadBuffer( pcBaseSubband->getFullPelYuvBuffer() );
        }
        else
        {
          m_pcIntMbTempData->setAllSamplesToZero();
        }

        m_pcIntMbTempData ->getTempYuvMbBuffer().loadLuma( *m_pcIntMbTempData );
        m_pcIntMbTempData ->setQp( ucQp );

				//-- JVT-R091
				// note: use intra offset as IntraBL does
				if ( bSmoothedRef )
				{
					m_pcTransform		->setQp( *m_pcIntMbTempData, true );
				}
				else
				{
					m_pcTransform   ->setQp( *m_pcIntMbTempData, rcMbDataAccess.getSH().isIntra() );
				}
				//--

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
							if( !bSmoothedRef && xGetCoeffCost() <= uiB8Thres )
							//--
              {
                m_pcIntMbTempData->loadLuma( m_pcIntMbTempData->getTempYuvMbBuffer(), c8x8Idx );

                m_pcIntMbTempData->clearLumaLevels8x8Block( c8x8Idx );
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
              if( !bSmoothedRef && xGetCoeffCost() <= uiB8Thres )
							//--
              {
                m_pcIntMbTempData->loadLuma( m_pcIntMbTempData->getTempYuvMbBuffer(), c8x8Idx );

                m_pcIntMbTempData->clearLumaLevels8x8( c8x8Idx );
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
        if( !bSmoothedRef && 0 != uiExtCbp && uiCoeffCost <= uiMBThres )
				//--
        {
          m_pcIntMbTempData->loadLuma( m_pcIntMbTempData->getTempYuvMbBuffer() );
          uiExtCbp = 0;

          m_pcIntMbTempData->clearLumaLevels();
        }
        m_pcIntMbTempData->distY() = m_pcXDistortion->getLum16x16( m_pcIntMbTempData->getMbLumAddr(), m_pcIntMbTempData->getLStride() );

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
        dCost   = (Double)uiDist + dLambda * (Double)uiRate;

        if( dCost < dMinCost )
        {
          dMinCost  = dCost;
          rbCoded   = bCoded;

          //----- store parameters and reconstrcuted signal to Frame and MbDataAccess -----
          m_pcIntPicBuffer                  ->loadBuffer        ( m_pcIntMbTempData );
          pcResidual->getFullPelYuvBuffer() ->loadBuffer        ( m_pcIntMbTempData );
          m_pcIntMbTempData                 ->copyResidualDataTo( rcMbDataAccess );

          //----- set residual prediction flag -----
          rcMbDataAccess.getMbData().setResidualPredFlag( iCnt > 0, PART_16x16 );

					//-- JVT-R091
					if ( bSmoothedRef )
					{
						// update prediction signal
						pcSRFrame->getFullPelYuvBuffer()->loadBuffer				( &cNewPrdMbBuffer	);

						// set flag
						rcMbDataAccess.getMbData().setSmoothedRefFlag( true );
					}
					//--
        }

        m_pcIntMbTempData->uninit();
      }
    }

		//-- JVT-R091
		// restore original residual
		m_pcIntOrgMbPelData->loadLuma		( cOrgMbBuffer );
		m_pcIntOrgMbPelData->loadChroma	( cOrgMbBuffer );
		//--
  }

  return Err::m_nOK;
}





ErrVal
MbEncoder::encodeResidual( MbDataAccess&  rcMbDataAccess,
                           MbDataAccess&  rcMbDataAccessBL,
                           IntFrame*      pcResidual,
                           Double         dLambda,
                           Bool           bLowPass,
                           Int            iMaxQpDelta )
{
  ROF( bInitDone );

  m_pcIntPicBuffer    = pcResidual->getFullPelYuvBuffer();
  Bool    bIntra16x16 = rcMbDataAccessBL.getMbData().isIntra16x16 ();
  Bool    bIntra8x8   = rcMbDataAccessBL.getMbData().isIntra4x4   () &&  rcMbDataAccessBL.getMbData().isTransformSize8x8();
  Bool    bIntra4x4   = rcMbDataAccessBL.getMbData().isIntra4x4   () && !rcMbDataAccessBL.getMbData().isTransformSize8x8();
  Bool    bIntra      = ( bIntra16x16 || bIntra8x8 || bIntra4x4 );
  Bool    bInter      = ! bIntra;
  Bool    b8x8Ok      = rcMbDataAccessBL.getSH().getPPS().getTransform8x8ModeFlag() && rcMbDataAccessBL.getMbData().is8x8TrafoFlagPresent();

  UChar   ucMinQp     = (UChar)min( MAX_QP, max( MIN_QP, rcMbDataAccess.getMbData().getQp() - iMaxQpDelta ) );
  UChar   ucMaxQp     = (UChar)min( MAX_QP, max( MIN_QP, rcMbDataAccess.getMbData().getQp() + iMaxQpDelta ) );
  UInt    uiMinTrafo  = ( bIntra8x8                       ? 1 : 0 );
  UInt    uiMaxTrafo  = ( bIntra8x8 || (bInter && b8x8Ok) ? 2 : 1 );
  Double  dMinCost    = 1e30;
  UInt    uiDist, uiRate;
  Double  dCost;

  m_pcXDistortion->loadOrgMbPelData( m_pcIntPicBuffer, m_pcIntOrgMbPelData );

  for( UChar ucQp = ucMinQp; ucQp <= ucMaxQp; ucQp++ )
  {
    for( UInt uiTrafo8x8 = uiMinTrafo; uiTrafo8x8 < uiMaxTrafo; uiTrafo8x8++ )
    {
      m_pcIntMbTempData ->init( rcMbDataAccess );
      m_pcIntMbTempData ->setAllSamplesToZero();
      m_pcIntMbTempData ->getTempYuvMbBuffer().loadLuma( *m_pcIntMbTempData );
      m_pcIntMbTempData ->setQp( ucQp );
      m_pcTransform     ->setQp( *m_pcIntMbTempData, bLowPass || bIntra );


      //----- encode luminance signal -----
      UInt  uiExtCbp    = 0;
      UInt  uiCoeffCost = 0;
      UInt  uiMbBits    = 0;
      UInt  uiB8Thres   = 4;
      UInt  uiMBThres   = 5;

      if( uiTrafo8x8 == 2 )
      {
        AOT(1);
        //===== 16x16 trafo =====
        RNOK( xEncode16x16ResidualMB( *m_pcIntMbTempData, uiMbBits, uiExtCbp ) );
        uiCoeffCost = 1000;
      }
      else if( uiTrafo8x8 )
      {
        for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
        {
          xSetCoeffCost( 0 );
          UInt uiBits = 0;
          UInt uiCbp  = 0;
          
          RNOK( xEncode8x8InterBlock( *m_pcIntMbTempData, c8x8Idx, uiBits, uiCbp ) );
          if( uiCbp )
          {
            if( xGetCoeffCost() <= uiB8Thres && ! rcMbDataAccess.getSH().isIntra() && ! bLowPass && ! bIntra )
            {
              m_pcIntMbTempData->loadLuma( m_pcIntMbTempData->getTempYuvMbBuffer(), c8x8Idx );

              m_pcIntMbTempData->clearLumaLevels8x8Block( c8x8Idx );
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
            if( xGetCoeffCost() <= uiB8Thres && ! rcMbDataAccess.getSH().isIntra() && ! bLowPass && ! bIntra )
            {
              m_pcIntMbTempData->loadLuma( m_pcIntMbTempData->getTempYuvMbBuffer(), c8x8Idx );

              m_pcIntMbTempData->clearLumaLevels8x8( c8x8Idx );
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
      if( 0 != uiExtCbp && uiCoeffCost <= uiMBThres && ! rcMbDataAccess.getSH().isIntra() && ! bLowPass && ! bIntra )
      {
        m_pcIntMbTempData->loadLuma( m_pcIntMbTempData->getTempYuvMbBuffer() );
        uiExtCbp = 0;

        m_pcIntMbTempData->clearLumaLevels();
      }
      m_pcIntMbTempData->distY() = m_pcXDistortion->getLum16x16( m_pcIntMbTempData->getMbLumAddr(), m_pcIntMbTempData->getLStride() );

      //----- encode chrominance signal -----
      RNOK( xEncodeChromaTexture( *m_pcIntMbTempData, uiExtCbp, uiMbBits ) );

      //----- set parameters ----
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
      dCost   = (Double)uiDist + dLambda * (Double)uiRate;

      if( dCost < dMinCost )
      {
        dMinCost  = dCost;

        //----- store parameters to MbDataAccess -----
        m_pcIntMbTempData->copyResidualDataTo( rcMbDataAccess );
      }

      m_pcIntMbTempData->uninit();
    }
  }  

  return Err::m_nOK;
}





ErrVal
MbEncoder::estimatePrediction( MbDataAccess&   rcMbDataAccess,
                               MbDataAccess*   pcMbDataAccessBase,
                               Int             iSpatialScalabilityType,
                               RefFrameList&   rcRefFrameList0,
                               RefFrameList&   rcRefFrameList1,
                               const IntFrame* pcBaseLayerFrame,
                               const IntFrame* pcBaseLayerResidual,
                               const IntFrame& rcOrigFrame,
                               IntFrame&       rcIntraRecFrame,
                               Bool            bBiPredOnly,
                               UInt            uiNumMaxIter,
                               UInt            uiIterSearchRange,
                               UInt            uiIntraMode,
                               Double          dLambda )
{
  ROF( bInitDone );

  Bool  bBSlice = rcMbDataAccess.getSH().getSliceType () == B_SLICE;
  UInt  uiQp    = rcMbDataAccess.getSH().getPicQp     ();
  RNOK( m_pcRateDistortionIf->setMbQpLambda( rcMbDataAccess, uiQp, dLambda ) )

  m_pcIntMbBestIntraChroma  = NULL;
  m_pcIntMbBestData   ->init( rcMbDataAccess );
  m_pcIntMbTempData   ->init( rcMbDataAccess );
  m_pcIntMbBest8x8Data->init( rcMbDataAccess );
  m_pcIntMbTemp8x8Data->init( rcMbDataAccess );

  m_pcIntPicBuffer                  = rcIntraRecFrame.getFullPelYuvBuffer();
  IntYuvPicBuffer*  pcOrgPicBuffer  = const_cast<IntFrame&>( rcOrigFrame ).getFullPelYuvBuffer();
  m_pcXDistortion->loadOrgMbPelData ( pcOrgPicBuffer, m_pcIntOrgMbPelData );
  m_pcTransform  ->setQp            ( rcMbDataAccess, false );

  IntYuvMbBuffer  cBaseLayerBuffer;


  //===== residual prediction =====
  if( rcMbDataAccess.getSH().getBaseLayerId() != MSYS_UINT_MAX )
  {
    //--- subtract (upsampled) base layer residual from original macroblock data ---
    ROF( pcBaseLayerResidual );
    cBaseLayerBuffer    .loadBuffer ( const_cast<IntFrame*>(pcBaseLayerResidual)->getFullPelYuvBuffer() );
    if( ! cBaseLayerBuffer.isZero() ) // HS: search only with residual prediction, when residual signal is non-zero
    {
      m_pcIntOrgMbPelData->subtract   ( cBaseLayerBuffer );

      if( ! pcMbDataAccessBase->getMbData().isIntra() )
      {
        //--- only if base layer is in intra mode ---
          // TMM_ESS 
	      if ( pcMbDataAccessBase->getMbData().getInCropWindowFlag() ) 
					RNOK( xEstimateMbBLSkip   ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, rcRefFrameList1, pcBaseLayerFrame, bBSlice, iSpatialScalabilityType,         pcMbDataAccessBase, true ) );

				//-- JVT-R091
	      if ( pcMbDataAccessBase->getMbData().getInCropWindowFlag() && rcMbDataAccess.isConstrainedInterLayerPred( pcMbDataAccessBase ) ) 
					RNOK( xEstimateMbSR				( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, rcRefFrameList1, pcBaseLayerResidual, bBSlice, iSpatialScalabilityType,			pcMbDataAccessBase, true ) );
				//--
      }

      if( rcMbDataAccess.getSH().getAdaptivePredictionFlag() )
      {
        RNOK( xEstimateMbDirect   ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, rcRefFrameList1,                                                       pcMbDataAccessBase, true ) );
        RNOK( xEstimateMb16x16    ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, rcRefFrameList1, bBiPredOnly, uiNumMaxIter, uiIterSearchRange, false,  pcMbDataAccessBase, true ) );
        RNOK( xEstimateMb16x8     ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, rcRefFrameList1, bBiPredOnly, uiNumMaxIter, uiIterSearchRange, false,  pcMbDataAccessBase, true ) );
        RNOK( xEstimateMb8x16     ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, rcRefFrameList1, bBiPredOnly, uiNumMaxIter, uiIterSearchRange, false,  pcMbDataAccessBase, true ) );
        RNOK( xEstimateMb8x8      ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, rcRefFrameList1, bBiPredOnly, uiNumMaxIter, uiIterSearchRange, false,  pcMbDataAccessBase, true ) );
        RNOK( xEstimateMb8x8Frext ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, rcRefFrameList1, bBiPredOnly, uiNumMaxIter, uiIterSearchRange, false,  pcMbDataAccessBase, true ) );
      }

      //--- recover original macroblock data ---
      m_pcIntOrgMbPelData->add        ( cBaseLayerBuffer );
    }
  }


  //===== intra base layer mode =====
  if( rcMbDataAccess.getSH().getBaseLayerId() != MSYS_UINT_MAX )
  {
    if( pcMbDataAccessBase->getMbData().isIntra() || !rcMbDataAccess.isConstrainedInterLayerPred( pcMbDataAccessBase ) )
      if ( pcMbDataAccessBase->getMbData().getInCropWindowFlag() ) // TMM_ESS
    RNOK  ( xEstimateMbIntraBL  ( m_pcIntMbTempData, m_pcIntMbBestData, pcBaseLayerFrame, bBSlice,                                                              pcMbDataAccessBase ) );
  }


  //===== without residual prediction =====
  if( rcMbDataAccess.getSH().getBaseLayerId           () == MSYS_UINT_MAX ||
      rcMbDataAccess.getSH().getAdaptivePredictionFlag() )
  {
    if( ( pcMbDataAccessBase && pcMbDataAccessBase->getMbData().isIntra() ) || rcMbDataAccess.getSH().getAdaptivePredictionFlag() )
    {
      // TMM_ESS 
      if ( pcMbDataAccessBase->getMbData().getInCropWindowFlag() ) 
				RNOK( xEstimateMbBLSkip   ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, rcRefFrameList1, pcBaseLayerFrame, bBSlice, iSpatialScalabilityType,         pcMbDataAccessBase, false ) );
    }

    RNOK  ( xEstimateMbDirect   ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, rcRefFrameList1,                                                       pcMbDataAccessBase, false ) );
    RNOK  ( xEstimateMb16x16    ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, rcRefFrameList1, bBiPredOnly, uiNumMaxIter, uiIterSearchRange, false,  pcMbDataAccessBase, false ) );
    RNOK  ( xEstimateMb16x8     ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, rcRefFrameList1, bBiPredOnly, uiNumMaxIter, uiIterSearchRange, false,  pcMbDataAccessBase, false ) );
    RNOK  ( xEstimateMb8x16     ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, rcRefFrameList1, bBiPredOnly, uiNumMaxIter, uiIterSearchRange, false,  pcMbDataAccessBase, false ) );
    RNOK  ( xEstimateMb8x8      ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, rcRefFrameList1, bBiPredOnly, uiNumMaxIter, uiIterSearchRange, false,  pcMbDataAccessBase, false ) );
    RNOK  ( xEstimateMb8x8Frext ( m_pcIntMbTempData, m_pcIntMbBestData, rcRefFrameList0, rcRefFrameList1, bBiPredOnly, uiNumMaxIter, uiIterSearchRange, false,  pcMbDataAccessBase, false ) );
  }
  

  //===== normal intra mode =====
  if( uiIntraMode &&
    ( rcMbDataAccess.getSH().getBaseLayerId           () == MSYS_UINT_MAX ||
      rcMbDataAccess.getSH().getAdaptivePredictionFlag() ) )
  {
    RNOK  ( xEstimateMbIntra16  ( m_pcIntMbTempData, m_pcIntMbBestData,                   bBSlice ) );
    RNOK  ( xEstimateMbIntra8   ( m_pcIntMbTempData, m_pcIntMbBestData,                   bBSlice ) );
    RNOK  ( xEstimateMbIntra4   ( m_pcIntMbTempData, m_pcIntMbBestData,                   bBSlice ) );
    RNOK  ( xEstimateMbPCM      ( m_pcIntMbTempData, m_pcIntMbBestData,                   bBSlice ) );
  }
  xStoreEstimation( rcMbDataAccess, *m_pcIntMbBestData, NULL, NULL, true, &cBaseLayerBuffer );

  m_pcIntMbBestData   ->uninit();
  m_pcIntMbTempData   ->uninit();
  m_pcIntMbBest8x8Data->uninit();
  m_pcIntMbTemp8x8Data->uninit();

  return Err::m_nOK;
}



ErrVal
MbEncoder::compensatePrediction( MbDataAccess&   rcMbDataAccess,
                                 IntFrame*       pcMCFrame,
                                 RefFrameList&   rcRefFrameList0,
                                 RefFrameList&   rcRefFrameList1,
                                 Bool            bCalcMv,
                                 Bool            bFaultTolerant )
{
  IntYuvMbBuffer  cYuvMbBuffer;

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
      RNOK( m_pcMotionEstimation->compensateMb( rcMbDataAccess, rcRefFrameList0, rcRefFrameList1, &cYuvMbBuffer, bCalcMv, bFaultTolerant ) );
    }
  }

  //===== insert into frame =====
  RNOK( pcMCFrame->getFullPelYuvBuffer()->loadBuffer( &cYuvMbBuffer ) );
  
  return Err::m_nOK;
}


ErrVal
MbEncoder::compensateUpdate(  MbDataAccess&   rcMbDataAccess,
                              IntFrame*       pcMCFrame,
                              Int             iRefIdx,
                              ListIdx         eListPrd,
                              IntFrame*       pcPrdFrame)
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

  return Err::m_nOK;
}






ErrVal
MbEncoder::xEstimateMbIntraBL( IntMbTempData*&  rpcMbTempData,
                               IntMbTempData*&  rpcMbBestData,
                               const IntFrame*  pcBaseLayerRec,
                               Bool             bBSlice,
                               MbDataAccess*    pcMbDataAccessBase )
{
  ROF( pcBaseLayerRec );

  Bool            bBLSkip           = pcMbDataAccessBase->getMbData().isIntra();
  UInt            uiCoeffBits       = 0;
  IntYuvMbBuffer& rcYuvMbBuffer     = *rpcMbTempData;
  IntYuvMbBuffer& rcTempYuvMbBuffer =  rpcMbTempData->getTempYuvMbBuffer();

  rpcMbTempData->clear();
  rpcMbTempData->setMbMode( INTRA_BL );
  rpcMbTempData->setBLSkipFlag( bBLSkip );
  rpcMbTempData->setBLQRefFlag( false );

  rcYuvMbBuffer    .loadBuffer( ((IntFrame*)pcBaseLayerRec)->getFullPelYuvBuffer() );
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


  RNOK( xSetRdCostIntraMb     ( *rpcMbTempData, uiCoeffBits, bBSlice, bBLSkip ) );

  RNOK( xCheckBestEstimation  (  rpcMbTempData, rpcMbBestData ) );
  RNOK( xEstimateMbIntraBL8x8 (  rpcMbTempData, rpcMbBestData, pcBaseLayerRec, bBSlice, bBLSkip ) );

  return Err::m_nOK;
}






ErrVal
MbEncoder::xEstimateMbIntraBL8x8( IntMbTempData*&  rpcMbTempData,
                                  IntMbTempData*&  rpcMbBestData,
                                  const IntFrame*  pcBaseLayerRec,
                                  Bool             bBSlice,
                                  Bool             bBLSkip )
{
  ROFRS( pcBaseLayerRec, Err::m_nOK  );
  ROTRS( ! rpcMbTempData->getSH().getPPS().getTransform8x8ModeFlag(), Err::m_nOK );

  rpcMbTempData->clear();
  rpcMbTempData->setMbMode( INTRA_BL );
  rpcMbTempData->setBLSkipFlag( bBLSkip );
  rpcMbTempData->setBLQRefFlag( false );
  rpcMbTempData->setTransformSize8x8( true );

  UInt            uiCoeffBits       = 0;
  IntYuvMbBuffer& rcYuvMbBuffer     = *rpcMbTempData;
  IntYuvMbBuffer& rcTempYuvMbBuffer =  rpcMbTempData->getTempYuvMbBuffer();

  rcYuvMbBuffer    .loadBuffer( ((IntFrame*)pcBaseLayerRec)->getFullPelYuvBuffer() );
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
  RNOK( xCheckBestEstimation(  rpcMbTempData, rpcMbBestData ) );

  return Err::m_nOK;
}





ErrVal
MbEncoder::xEstimateMbIntra16( IntMbTempData*&  rpcMbTempData,
                               IntMbTempData*&  rpcMbBestData,
                               Bool             bBSlice  )
{
  rpcMbTempData->clear();

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

  for( Int n = 0; n < 4; n++ )
  {
    Bool bValid = true;
    RNOK( m_pcIntraPrediction->predictSLumaMb( rpcMbTempData, n, bValid ) );

    if( ! bValid )
    {
      continue;
    }

    UInt  uiDcAbs = 0;
    UInt  uiAcAbs = 0;
    RNOK( m_pcTransform->transformMb16x16( m_pcIntOrgMbPelData, rpcMbTempData, rpcMbTempData->get( B4x4Idx(0) ), pucScale, uiDcAbs, uiAcAbs ) );
    UInt uiDist = m_pcXDistortion->getLum16x16( pPel, iStride );

    BitCounter::init();
    RNOK( MbCoder::xScanLumaIntra16x16( *rpcMbTempData, *rpcMbTempData, uiAcAbs != 0 ) );
    UInt uiBits = BitCounter::getNumberOfWrittenBits();

    UInt uiRd = (UInt)floor(m_pcRateDistortionIf->getCost( uiBits, uiDist ));
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

  rpcMbTempData->getTempYuvMbBuffer().loadLuma( *rpcMbTempData );

  RNOK( m_pcTransform->transformMb16x16( m_pcIntOrgMbPelData, rpcMbTempData, rpcMbTempData->get( B4x4Idx(0) ), pucScale, uiDcAbs, uiAcAbs ) );

  const DFunc&  rcDFunc = m_pcCodingParameter->getMotionVectorSearchParams().getSubPelDFunc();
  UInt          uiDist  = m_pcXDistortion->getLum16x16( pPel, iStride, rcDFunc );
  UInt          uiMBits = ( bBSlice ? 9 : 5 ) + 2;
  UInt          uiCost  = uiDist + m_pcMotionEstimation->getRateCost( uiBestBits + uiMBits, rcDFunc == DF_SAD );


  UInt uiChromaCbp = 0;
  rpcMbTempData->setMbMode( INTRA_4X4 ); // for right scaling matrix
  RNOK( xEncodeChromaIntra( *rpcMbTempData, uiChromaCbp, uiBestBits ) );

  UInt  uiMbType  = INTRA_4X4 + 1;
        uiMbType += uiPredMode;
        uiMbType += ( uiAcAbs != 0 ) ? 12 : 0;
        uiMbType += uiChromaCbp >> 14;

  // needed for CABAC
  rpcMbTempData->cbp()  =   xCalcMbCbp( ( uiAcAbs != 0 ? 0xFFFF : 0 ) + uiChromaCbp );
  rpcMbTempData->setMbMode( MbMode( uiMbType ) );
  rpcMbTempData->setBLSkipFlag( false );
  rpcMbTempData->setBLQRefFlag( false );

  rpcMbTempData->getMbMotionData( LIST_0 ).setMotPredFlag( false );
  rpcMbTempData->getMbMotionData( LIST_1 ).setMotPredFlag( false );

  //--- store estimated parameters ---
  rpcMbTempData->rdCost() = uiCost;


  RNOK( xSetRdCostIntraMb   ( *rpcMbTempData, uiBestBits, bBSlice, false ) );
  RNOK( xCheckBestEstimation(  rpcMbTempData, rpcMbBestData ) );

  return Err::m_nOK;
}



ErrVal
MbEncoder::xEstimateMbIntra4( IntMbTempData*&  rpcMbTempData,
                              IntMbTempData*&  rpcMbBestData,
                              Bool             bBSlice  )
{
  rpcMbTempData->clear();
  rpcMbTempData->setMbMode( INTRA_4X4 );
  rpcMbTempData->setBLSkipFlag( false );
  rpcMbTempData->setBLQRefFlag( false );

  //----- init intra prediction -----
  rpcMbTempData->loadIntraPredictors( m_pcIntPicBuffer );
  rpcMbTempData->clearIntraPredictionModes( false );
  m_pcIntraPrediction->setAvailableMaskMb( rpcMbTempData->getMbDataAccess().getAvailableMask() );


  UInt uiExtCbp = 0;
  UInt uiMbBits = 0;

  for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
  {
    UInt uiBits = 0;
    UInt uiCbp = 0;
    for( S4x4Idx cIdx( c8x8Idx ); cIdx.isLegal( c8x8Idx ); cIdx++ )
    {
      RNOK( xEncode4x4IntraBlock( *rpcMbTempData, cIdx, uiBits, uiCbp ) );
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

  XPel*         pPel    = rpcMbTempData->getMbLumAddr();
  Int           iStride = rpcMbTempData->getLStride();
  const DFunc&  rcDFunc = m_pcCodingParameter->getMotionVectorSearchParams().getSubPelDFunc();
  UInt          uiDist  = m_pcXDistortion->getLum16x16( pPel, iStride, rcDFunc );
  UInt          uiMBits = ( bBSlice ? 9 : 5 );
  UInt          uiCost  = uiDist + m_pcMotionEstimation->getRateCost( uiMbBits + uiMBits, rcDFunc == DF_SAD );

  RNOK( xEncodeChromaIntra( *rpcMbTempData, uiExtCbp, uiMbBits ) );

  //--- store estimated parameters ---
  rpcMbTempData->rdCost() = uiCost;
  rpcMbTempData->cbp()    = xCalcMbCbp( uiExtCbp );

  rpcMbTempData->getMbMotionData( LIST_0 ).setMotPredFlag( false );
  rpcMbTempData->getMbMotionData( LIST_1 ).setMotPredFlag( false );

  RNOK( xSetRdCostIntraMb   ( *rpcMbTempData, uiMbBits, bBSlice, false ) );
  RNOK( xCheckBestEstimation(  rpcMbTempData, rpcMbBestData ) );

  return Err::m_nOK;
}



ErrVal
MbEncoder::xEstimateMbIntra8( IntMbTempData*&  rpcMbTempData,
                              IntMbTempData*&  rpcMbBestData,
                              Bool             bBSlice )
{
  ROTRS( ! rpcMbTempData->getSH().getPPS().getTransform8x8ModeFlag(), Err::m_nOK );

  rpcMbTempData->clear();
  rpcMbTempData->setMbMode( INTRA_4X4 );
  rpcMbTempData->setTransformSize8x8( true );
  rpcMbTempData->setBLSkipFlag( false );
  rpcMbTempData->setBLQRefFlag( false );

  //----- init intra prediction -----
  rpcMbTempData->loadIntraPredictors( m_pcIntPicBuffer );
  rpcMbTempData->clearIntraPredictionModes( true );
  m_pcIntraPrediction->setAvailableMaskMb( rpcMbTempData->getMbDataAccess().getAvailableMask() );

  UInt uiExtCbp = 0;
  UInt uiMbBits = 0;

  for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
  {
    RNOK( xEncode8x8IntraBlock( *rpcMbTempData, c8x8Idx, uiMbBits, uiExtCbp ) );
  }

  XPel*         pPel      = rpcMbTempData->getMbLumAddr();
  Int           iStride   = rpcMbTempData->getLStride();
  const DFunc&  rcDFunc   = m_pcCodingParameter->getMotionVectorSearchParams().getSubPelDFunc();
  UInt          uiDist    = m_pcXDistortion->getLum16x16( pPel, iStride, rcDFunc );
  UInt          uiMBits   = ( bBSlice ? 9 : 5 );
  UInt          uiCost    = uiDist + m_pcMotionEstimation->getRateCost( uiMbBits + uiMBits, rcDFunc == DF_SAD );

  RNOK( xEncodeChromaIntra( *rpcMbTempData, uiExtCbp, uiMbBits ) );

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
  rpcMbTempData->setBLQRefFlag( false );

  rpcMbTempData->loadLuma   ( *m_pcIntOrgMbPelData );
  rpcMbTempData->loadChroma ( *m_pcIntOrgMbPelData );
  
  Pel*  pucDest   = rpcMbTempData->getPelBuffer();
  XPel* pucSrc    = rpcMbTempData->getMbLumAddr();
  Int   iStride   = rpcMbTempData->getLStride();
  UInt  uiDist    = 0;
  UInt  uiMbBits  = 8*8*8*6+(bBSlice?11:9)+4;
  UInt  uiDelta   = 1;
  Int   n, m, n1, m1, dest, cnt, diff, idx;

  for( n = 0; n < 16; n+=uiDelta )
  {
    for( m = 0; m < 16; m+=uiDelta )
    {
      dest  = 0;
      cnt   = 0;
      for( n1=0; n1<  (Int)uiDelta; n1++)
      for( m1=m; m1<m+(Int)uiDelta; m1++)
      {
        dest += pucSrc[m1+n1*iStride];
        cnt  ++;
      }
      dest  = ( dest + cnt / 2 ) / cnt;
      dest  = min( 255, max( 1, dest ) );

      *pucDest = (Pel)dest;
      pucDest++;

      for( n1=0; n1<  (Int)uiDelta; n1++)
      for( m1=m; m1<m+(Int)uiDelta; m1++)
      {
        idx  = m1 + n1*iStride;
        diff = pucSrc[idx] - dest;
        pucSrc[idx]        = dest;
        uiDist += diff * diff;
      }
    }
    pucSrc  += uiDelta*iStride;
  }

  pucSrc  = rpcMbTempData->getMbCbAddr();
  iStride = rpcMbTempData->getCStride();

  for( n = 0; n < 8; n+=uiDelta )
  {
    for( m = 0; m < 8; m+=uiDelta )
    {
      dest  = 0;
      cnt   = 0;
      for( n1=0; n1<  (Int)uiDelta; n1++)
      for( m1=m; m1<m+(Int)uiDelta; m1++)
      {
        dest += pucSrc[m1+n1*iStride];
        cnt  ++;
      }
      dest  = ( dest + cnt / 2 ) / cnt;
      dest  = min( 255, max( 1, dest ) );

      *pucDest = (Pel)dest;
      pucDest++;

      for( n1=0; n1<  (Int)uiDelta; n1++)
      for( m1=m; m1<m+(Int)uiDelta; m1++)
      {
        idx  = m1 + n1*iStride;
        diff = pucSrc[idx] - dest;
        pucSrc[idx]        = dest;
        uiDist += diff * diff;
      }
    }
    pucSrc  += uiDelta*iStride;
  }

  pucSrc  = rpcMbTempData->getMbCrAddr();

  for( n = 0; n < 8; n+=uiDelta )
  {
    for( m = 0; m < 8; m+=uiDelta )
    {
      dest  = 0;
      cnt   = 0;
      for( n1=0; n1<  (Int)uiDelta; n1++)
      for( m1=m; m1<m+(Int)uiDelta; m1++)
      {
        dest += pucSrc[m1+n1*iStride];
        cnt  ++;
      }
      dest  = ( dest + cnt / 2 ) / cnt;
      dest  = min( 255, max( 1, dest ) );

      *pucDest = (Pel)dest;
      pucDest++;

      for( n1=0; n1<  (Int)uiDelta; n1++)
      for( m1=m; m1<m+(Int)uiDelta; m1++)
      {
        idx  = m1 + n1*iStride;
        diff = pucSrc[idx] - dest;
        pucSrc[idx]        = dest;
        uiDist += diff * diff;
      }
    }
    pucSrc  += uiDelta*iStride;
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

      rcMbTempData.bits() += (rcMbTempData.getSH().isIntra() ? 0 : 1); // run
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

  RNOK( m_pcTransform->transform4x4Blk( m_pcIntOrgMbPelData, rcMbTempData, rcMbTempData.get( cIdx ), pucScale, uiAbsSum ) );

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

  RNOK( m_pcTransform->transform8x8Blk( m_pcIntOrgMbPelData, rcMbTempData, rcMbTempData.get8x8( c8x8Idx ), pucScale, uiAbsSum ) );

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
  UInt uiBits   = 0;
  UInt uiDCAbs  = 0;
  UInt uiACAbs  = 0;

  Int           iScalMat  = ( 0 );
  const UChar*  pucScale  = ( rcMbTempData.getSH().isScalingMatrixPresent(iScalMat) ? rcMbTempData.getSH().getScalingMatrix(iScalMat) : NULL );

  RNOK( m_pcTransform->transformMb16x16( m_pcIntOrgMbPelData,
                                         rcMbTempData,
                                         rcMbTempData.get( B4x4Idx(0) ), pucScale, uiDCAbs, uiACAbs )  );

  BitCounter::init();
  RNOK( MbCoder::xScanLumaIntra16x16( rcMbTempData, rcMbTempData, uiACAbs != 0 ) );
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

  Int           iScalMat  = 6;
  const UChar*  pucScale  = ( rcMbTempData.getSH().isScalingMatrixPresent(iScalMat) ? rcMbTempData.getSH().getScalingMatrix(iScalMat) : NULL );

  for( UInt n = 0; n < 9; n++)
  {
    bValid    = true;
    iPredMode = n;

    if( bValid )
    {
      RNOK( m_pcIntraPrediction->predictSLumaBlock8x8( rcMbTempData, iPredMode, c8x8Idx, bValid ) );
    }

    if( bValid )
    {
      RNOK( m_pcTransform->transform8x8Blk( m_pcIntOrgMbPelData, rcMbTempData, rcMbTempData.get8x8( c8x8Idx ), pucScale, uiAbsSum = 0 ) );
      UInt uiDist = m_pcXDistortion->getLum8x8( rcMbTempData.getLumBlk(), rcMbTempData.getLStride() );

      rcMbTempData.intraPredMode(c8x8Idx) = iPredMode;

      BitCounter::init();
      m_pcMbSymbolWriteIf->intraPredModeLuma( rcMbTempData, c8x8Idx );
      Int uiBits = BitCounter::getNumberOfWrittenBits() + 1;

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

  if( 0 != uiAbsSum )
  {
    ruiExtCbp |= 0x33 << c8x8Idx.b4x4();
  }

  ruiBits += uiBestBits;

  return Err::m_nOK;
}





ErrVal
MbEncoder::xEncode4x4IntraBlock( IntMbTempData& rcMbTempData,
                                 LumaIdx        cIdx,
                                 UInt&          ruiBits,
                                 UInt&          ruiExtCbp )
{
  rcMbTempData.        set4x4Block( cIdx );
  m_pcIntOrgMbPelData->set4x4Block( cIdx );
  UInt uiAbsSum;

  Double fBestRd = DOUBLE_MAX;
  UInt uiBestMode = 2;
  UInt uiBestBits = 0;
  Int iPredMode = 0;
  Bool bValid = true;

  Int           iScalMat  = 0;
  const UChar*  pucScale  = ( rcMbTempData.getSH().isScalingMatrixPresent(iScalMat) ? rcMbTempData.getSH().getScalingMatrix(iScalMat) : NULL );

  for( UInt n = 0; n < 9; n++)
  {
    bValid    = true;
    iPredMode = n;
    if( bValid )
    {
      RNOK( m_pcIntraPrediction->predictSLumaBlock( rcMbTempData, iPredMode, cIdx, bValid ) );
    }

    if( bValid )
    {
      RNOK( m_pcTransform->transform4x4Blk( m_pcIntOrgMbPelData, rcMbTempData, rcMbTempData.get( cIdx ), pucScale, uiAbsSum = 0 ) );

      UInt uiDist = m_pcXDistortion->getLum4x4( rcMbTempData.getLumBlk(), rcMbTempData.getLStride() );

      BitCounter::init();
      rcMbTempData.intraPredMode(cIdx) = iPredMode;

      m_pcMbSymbolWriteIf->intraPredModeLuma( rcMbTempData, cIdx );
      Int uiBits = BitCounter::getNumberOfWrittenBits() + 1;

      if( 0 != uiAbsSum )
      {
        BitCounter::init();
        RNOK( MbCoder::xScanLumaBlock( rcMbTempData, rcMbTempData, cIdx ) );
        uiBits += BitCounter::getNumberOfWrittenBits() - 1;
      }

      Double fCost = m_pcRateDistortionIf->getFCost( uiBits, uiDist );

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

  if( 0 != uiAbsSum )
  {
    ruiExtCbp |= 1 << cIdx;
  }
  ruiBits += uiBestBits;

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

    if( rcMbDataAccess.getSH().isInterB() )
    {
      rcMbBestData.getMbMotionData( LIST_1 ).clear( BLOCK_NOT_PREDICTED );
      rcMbBestData.getMbMvdData   ( LIST_1 ).clear();
    }
  }
  else if( rcMbBestData.isSkiped() )
  {
    rcMbBestData.getMbMvdData( LIST_0 ).clear();

    if( rcMbDataAccess.getSH().isInterB() )
    {
      rcMbBestData.getMbMvdData( LIST_1 ).clear();
    }
  }

  UInt uiFwdBwd = 0;
  if( rcMbDataAccess.getSH().isInterB() )
  {
    for( Int n = 3; n >= 0; n--)
    {
      uiFwdBwd <<= 4;
      uiFwdBwd += (0 < rcMbBestData.getMbMotionData( LIST_0 ).getRefIdx( Par8x8(n) )) ? 1:0;
      uiFwdBwd += (0 < rcMbBestData.getMbMotionData( LIST_1 ).getRefIdx( Par8x8(n) )) ? 2:0;
    }
  }

  if( rcMbDataAccess.getSH().isInterP() )
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
                             IntFrame*       pcRecSubband,
                             IntFrame*       pcPredSignal,
                             Bool            bMotionFieldEstimation,
                             IntYuvMbBuffer* pcBaseLayerBuffer )
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

    if( rcMbDataAccess.getSH().isInterB() )
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

    if( rcMbDataAccess.getSH().isInterB() )
    {
      rcMbBestData.getMbMvdData( LIST_1 ).clear();
    }
  }

  UInt uiFwdBwd = 0;
  if( rcMbDataAccess.getSH().isInterB() )
  {
    for( Int n = 3; n >= 0; n--)
    {
      uiFwdBwd <<= 4;
      uiFwdBwd += (0 < rcMbBestData.getMbMotionData( LIST_0 ).getRefIdx( Par8x8(n) )) ? 1:0;
      uiFwdBwd += (0 < rcMbBestData.getMbMotionData( LIST_1 ).getRefIdx( Par8x8(n) )) ? 2:0;
    }
  }

  if( rcMbDataAccess.getSH().isInterP() )
  {
    for( Int n = 3; n >= 0; n--)
    {
      uiFwdBwd <<= 4;
      uiFwdBwd +=  (0 < rcMbBestData.getMbMotionData( LIST_0 ).getRefIdx( Par8x8(n) )) ? 1:0;
    }
  }
  rcMbBestData.setFwdBwd  ( uiFwdBwd );
  rcMbBestData.copyTo     ( rcMbDataAccess );

  if( ! rcMbDataAccess.getMbData().isIntra4x4() && ( rcMbDataAccess.getMbData().getMbCbp() & 0x0F ) == 0 )
  {
    rcMbDataAccess.getMbData().setTransformSize8x8( false );
  }

  if( rcMbDataAccess.getMbData().getResidualPredFlag( PART_16x16 ) )
  {
    AOF( pcBaseLayerBuffer );
    ((IntYuvMbBuffer&)rcMbBestData).add( *pcBaseLayerBuffer );
  }

  if( pcPredSignal )
  {
    ANOK( pcPredSignal->getFullPelYuvBuffer()->loadBuffer( &rcMbBestData.getTempYuvMbBuffer() ) );
  }

  if( pcRecSubband )
  {
    IntYuvMbBuffer cYuvMbBuffer;

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
  m_pcIntPicBuffer->loadBuffer( &rcMbBestData );
}





ErrVal
MbEncoder::xEncodeChromaIntra( IntMbTempData& rcMbTempData,
                               UInt&          ruiExtCbp,
                               UInt&          ruiBits )
{
  // do it once
  if( m_pcIntMbBestIntraChroma == NULL)
  {
    m_pcIntMbBestIntraChroma = &m_acIntMbTempData[4];
    Double fBestRd = DOUBLE_MAX;
    for( UInt uiPM = 0; uiPM < 4; uiPM++ )
    {
      Bool bValid = false;
      RNOK( m_pcIntraPrediction->predictSChromaBlock( rcMbTempData, uiPM, bValid ) );
      rcMbTempData.getTempYuvMbBuffer().loadChroma( rcMbTempData );

      if( bValid )
      {
        UInt uiBits = 0;
        UInt uiCbp  = 0;
        RNOK( xEncodeChromaTexture( rcMbTempData, uiCbp, uiBits ) );
        UInt uiDist = rcMbTempData.distU() + rcMbTempData.distV();
        rcMbTempData.setChromaPredMode( uiPM );

        BitCounter::init();
        RNOK( UvlcWriter::intraPredModeChroma( rcMbTempData ) );
        uiBits += BitCounter::getNumberOfWrittenBits();

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
  rcMbTempData.loadChromaData( *m_pcIntMbBestIntraChroma );
  ruiExtCbp += m_pcIntMbBestIntraChroma->cbp();
  ruiBits   += m_pcIntMbBestIntraChroma->bits();

  return Err::m_nOK;
}





ErrVal
MbEncoder::xEncodeChromaTexture( IntMbTempData& rcMbTempData,
                                 UInt&          ruiExtCbp,
                                 UInt&          ruiBits )
{
  TCoeff aiCoeff[128];

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
  RNOK( m_pcTransform->transformChromaBlocks( m_pcIntOrgMbPelData->getMbCbAddr(), pucCb, iStride, rcMbTempData.get( CIdx(0) ), aiCoeff+0x00, pucScaleCb, uiDcCb, uiAcCb ) );
  RNOK( m_pcTransform->transformChromaBlocks( m_pcIntOrgMbPelData->getMbCrAddr(), pucCr, iStride, rcMbTempData.get( CIdx(4) ), aiCoeff+0x40, pucScaleCr, uiDcCr, uiAcCr ) );

  UInt uiChromaCbp = 0;
  UInt uiDcBits = 0;
  UInt uiDcAbs = uiDcCb + uiDcCr;
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

        ::memset( &aiCoeff[0x41], 0x00, 15*sizeof(TCoeff) );
        ::memset( &aiCoeff[0x51], 0x00, 15*sizeof(TCoeff) );
        ::memset( &aiCoeff[0x61], 0x00, 15*sizeof(TCoeff) );
        ::memset( &aiCoeff[0x71], 0x00, 15*sizeof(TCoeff) );
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

        ::memset( &aiCoeff[0x01], 0x00, 15*sizeof(TCoeff) );
        ::memset( &aiCoeff[0x11], 0x00, 15*sizeof(TCoeff) );
        ::memset( &aiCoeff[0x21], 0x00, 15*sizeof(TCoeff) );
        ::memset( &aiCoeff[0x31], 0x00, 15*sizeof(TCoeff) );
      }
      uiCbp1 = uiChromaCbp;
    }
  }

  if( (uiAcBits1 + uiAcBits2) > 8)
  {
    ruiBits += uiAcBits1 + uiAcBits2;
    uiChromaCbp = max( uiCbp1, uiCbp2 );
  }

  const QpParameter& rcChromaQp = m_pcTransform->getChromaQp();

  if( uiCbp1 )
  {
    Int iQpScale = g_aaiDequantCoef[ rcChromaQp.rem() ][0] << ( rcChromaQp.per() );
    Int iShift   = 1;
    if( pucScaleCb )
    {
      if( rcChromaQp.per() < 5 )
      {
        iQpScale = pucScaleCb[0] * g_aaiDequantCoef[ rcChromaQp.rem() ][0];
        iShift   = 5-rcChromaQp.per();
      }
      else
      {
        iQpScale = ( ( pucScaleCb[0] * g_aaiDequantCoef[ rcChromaQp.rem() ][0] ) << rcChromaQp.per() ) >> 5;
        iShift   = 0;
      }
    }

    m_pcTransform->invTransformChromaDc( &aiCoeff[0x00], iQpScale, iShift );
    RNOK( m_pcTransform->invTransformChromaBlocks( pucCb, iStride, &aiCoeff[0x00] ) );
  }

  if( uiCbp2 )
  {
    Int iQpScale = g_aaiDequantCoef[ rcChromaQp.rem() ][0] << ( rcChromaQp.per() );
    Int iShift   = 1;
    if( pucScaleCr )
    {
      if( rcChromaQp.per() < 5 )
      {
        iQpScale = pucScaleCr[0] * g_aaiDequantCoef[ rcChromaQp.rem() ][0];
        iShift   = 5-rcChromaQp.per();
      }
      else
      {
        iQpScale = ( ( pucScaleCr[0] * g_aaiDequantCoef[ rcChromaQp.rem() ][0] ) << rcChromaQp.per() ) >> 5;
        iShift   = 0;
      }
    }

    m_pcTransform->invTransformChromaDc( &aiCoeff[0x40], iQpScale, iShift );
    RNOK( m_pcTransform->invTransformChromaBlocks( pucCr, iStride, &aiCoeff[0x40] ) );
  }


  m_pcIntMbTempData->distU() = m_pcXDistortion->get8x8Cr( pucCr, iStride );
  m_pcIntMbTempData->distV() = m_pcXDistortion->get8x8Cb( pucCb, iStride );

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
  IntYuvMbBuffer& rcYuvMbBuffer     = rcMbTempData;
  MbDataAccess&   rcMbDataAccess    = rcMbTempData.getMbDataAccess();
  UInt            uiMbDist          = 0;
  UInt            uiMbBits          = 0;

  //===== get distortion =====
  uiMbDist  += m_pcXDistortion->getLum16x16 ( rcYuvMbBuffer.getMbLumAddr(), rcYuvMbBuffer.getLStride() );
  uiMbDist  += m_pcXDistortion->get8x8Cb    ( rcYuvMbBuffer.getMbCbAddr (), rcYuvMbBuffer.getCStride() );
  uiMbDist  += m_pcXDistortion->get8x8Cr    ( rcYuvMbBuffer.getMbCrAddr (), rcYuvMbBuffer.getCStride() );

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

  //===== set rd-cost =====
  rcMbTempData.rdCost() = m_pcRateDistortionIf->getCost( uiMbBits, uiMbDist );

  return Err::m_nOK;
}




Bool
MbEncoder::xCheckUpdate( IntYuvMbBuffer&   rcPredBuffer,
                         IntYuvMbBuffer&   rcOrigBuffer,
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
                              UInt            uiAdditionalBits )
{
  IntYuvMbBuffer& rcYuvMbBuffer     = rcMbTempData;
  IntYuvMbBuffer& rcTempYuvMbBuffer = rcMbTempData  .getTempYuvMbBuffer ();
  MbDataAccess&   rcMbDataAccess    = rcMbTempData  .getMbDataAccess    ();
  MbData&         rcMbData          = rcMbDataAccess.getMbData          ();
  UInt            uiMbDist          = 0;
  UInt            uiMbBits          = 0;
  MbMode          eMbMode           = rcMbData.getMbMode();
  Bool            b8x8Mode          = ( eMbMode == MODE_8x8 || eMbMode == MODE_8x8ref0 );
  UInt            uiFwdBwd          = 0;


  //===== set forward / backward indication =====
  if( rcMbDataAccess.getSH().isInterB() )
  {
    for( Int n = 3; n >= 0; n--)
    {
      uiFwdBwd <<= 4;
      uiFwdBwd += (0 < rcMbDataAccess.getMbMotionData( LIST_0 ).getRefIdx( Par8x8(n) )) ? 1:0;
      uiFwdBwd += (0 < rcMbDataAccess.getMbMotionData( LIST_1 ).getRefIdx( Par8x8(n) )) ? 2:0;
    }
  }
  if( rcMbDataAccess.getSH().isInterP() )
  {
    for( Int n = 3; n >= 0; n--)
    {
      uiFwdBwd <<= 4;
      uiFwdBwd +=  (0 < rcMbDataAccess.getMbMotionData( LIST_0 ).getRefIdx( Par8x8(n) )) ? 1:0;
    }
  }
  rcMbTempData.setFwdBwd( uiFwdBwd );


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
                                                  &rcYuvMbBuffer, false, false ) );
  }
  rcTempYuvMbBuffer.loadLuma  ( rcYuvMbBuffer );
  rcTempYuvMbBuffer.loadChroma( rcYuvMbBuffer );

  //===== encode residual and get rate for coefficients =====
  UInt  uiCoeffCost = 0;
  UInt  uiExtCbp    = 0;
  Bool  bSkipMode   = ( eMbMode == MODE_SKIP && rcMbDataAccess.getSH().isInterP() );
  
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
    }

    //--- CHROMA ---
    RNOK( xEncodeChromaTexture( rcMbTempData, uiExtCbp, uiMbBits ) );
  }
  rcMbTempData.cbp() = xCalcMbCbp( uiExtCbp );

  //===== get distortion =====
  uiMbDist  += m_pcXDistortion->getLum16x16 ( rcYuvMbBuffer.getMbLumAddr(), rcYuvMbBuffer.getLStride() );
  uiMbDist  += m_pcXDistortion->get8x8Cb    ( rcYuvMbBuffer.getMbCbAddr (), rcYuvMbBuffer.getCStride() );
  uiMbDist  += m_pcXDistortion->get8x8Cr    ( rcYuvMbBuffer.getMbCrAddr (), rcYuvMbBuffer.getCStride() );

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
      RNOK(   MbCoder::xWriteMotionPredFlags          ( rcMbDataAccess, pcMbDataAccessBase, eMbMode, LIST_0 ) );

      RNOK(   MbCoder::xWriteReferenceFrames          ( rcMbDataAccess,                     eMbMode, LIST_0 ) );
      RNOK(   MbCoder::xWriteMotionVectors            ( rcMbDataAccess,                     eMbMode, LIST_0 ) );
    }

    if( rcRefFrameList1.getActive() && !bBLSkip )
    {
      RNOK(   MbCoder::xWriteMotionPredFlags          ( rcMbDataAccess, pcMbDataAccessBase, eMbMode, LIST_1 ) );

      RNOK(   MbCoder::xWriteReferenceFrames          ( rcMbDataAccess,                     eMbMode, LIST_1 ) );
      RNOK(   MbCoder::xWriteMotionVectors            ( rcMbDataAccess,                     eMbMode, LIST_1 ) );
    }
    
    uiMbBits  += BitCounter::getNumberOfWrittenBits();
  }

  //===== set rd-cost =====
  rcMbTempData.rdCost() = m_pcRateDistortionIf->getCost( uiMbBits+uiAdditionalBits, uiMbDist );

  return Err::m_nOK;
}

//-- JVT-R091
ErrVal
MbEncoder::xSetRdCostInterMbSR( IntMbTempData&  rcMbTempData,
																MbDataAccess*   pcMbDataAccessBase,
																RefFrameList&   rcRefFrameList0,
																RefFrameList&   rcRefFrameList1,
																IntFrame*				pcBaseLayerSbb,
																Bool            bBLSkip,
																UInt            uiAdditionalBits )
{
  IntYuvMbBuffer& rcYuvMbBuffer     = rcMbTempData;
  IntYuvMbBuffer& rcTempYuvMbBuffer = rcMbTempData  .getTempYuvMbBuffer ();
  MbDataAccess&   rcMbDataAccess    = rcMbTempData  .getMbDataAccess    ();
  MbData&         rcMbData          = rcMbDataAccess.getMbData          ();
  UInt            uiMbDist          = 0;
  UInt            uiMbBits          = 0;
  MbMode          eMbMode           = rcMbData.getMbMode();
  Bool            b8x8Mode          = ( eMbMode == MODE_8x8 );
  UInt            uiFwdBwd          = 0;

  //===== set forward / backward indication =====
  if( rcMbDataAccess.getSH().isInterB() )
  {
    for( Int n = 3; n >= 0; n--)
    {
      uiFwdBwd <<= 4;
      uiFwdBwd += (0 < rcMbDataAccess.getMbMotionData( LIST_0 ).getRefIdx( Par8x8(n) )) ? 1:0;
      uiFwdBwd += (0 < rcMbDataAccess.getMbMotionData( LIST_1 ).getRefIdx( Par8x8(n) )) ? 2:0;
    }
  }
  if( rcMbDataAccess.getSH().isInterP() )
  {
    for( Int n = 3; n >= 0; n--)
    {
      uiFwdBwd <<= 4;
      uiFwdBwd +=  (0 < rcMbDataAccess.getMbMotionData( LIST_0 ).getRefIdx( Par8x8(n) )) ? 1:0;
    }
  }
  rcMbTempData.setFwdBwd( uiFwdBwd );


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
                                                  &rcYuvMbBuffer, false, false ) );
  }

	// compute smoothed reference prediction: P+Rb
	IntYuvMbBuffer cBaseResMbBuffer;
	cBaseResMbBuffer.loadBuffer	( pcBaseLayerSbb->getFullPelYuvBuffer() );
	rcYuvMbBuffer		.add				( cBaseResMbBuffer );

	// S(P+Rb)
	// note: only pixels inside MB are considered
	pcBaseLayerSbb->getFullPelYuvBuffer()->loadBuffer( &rcYuvMbBuffer		);
	pcBaseLayerSbb->getFullPelYuvBuffer()->smoothMbInside();

	// S(P+Rb)-Rb
	rcYuvMbBuffer.loadBuffer		( pcBaseLayerSbb->getFullPelYuvBuffer() );
	rcYuvMbBuffer.subtract			( cBaseResMbBuffer );

	// load into prediction buffer
  rcTempYuvMbBuffer.loadLuma  ( rcYuvMbBuffer );
  rcTempYuvMbBuffer.loadChroma( rcYuvMbBuffer );

  //===== encode residual and get rate for coefficients =====
  UInt  uiCoeffCost = 0;
  UInt  uiExtCbp    = 0;
  Bool  bSkipMode   = ( eMbMode == MODE_SKIP && rcMbDataAccess.getSH().isInterP() );
  
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
    }

    //--- CHROMA ---
    RNOK( xEncodeChromaTexture( rcMbTempData, uiExtCbp, uiMbBits ) );
  }
  rcMbTempData.cbp() = xCalcMbCbp( uiExtCbp );

  //===== get distortion =====
  uiMbDist  += m_pcXDistortion->getLum16x16 ( rcYuvMbBuffer.getMbLumAddr(), rcYuvMbBuffer.getLStride() );
  uiMbDist  += m_pcXDistortion->get8x8Cb    ( rcYuvMbBuffer.getMbCbAddr (), rcYuvMbBuffer.getCStride() );
  uiMbDist  += m_pcXDistortion->get8x8Cr    ( rcYuvMbBuffer.getMbCrAddr (), rcYuvMbBuffer.getCStride() );

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
      RNOK(   MbCoder::xWriteMotionPredFlags          ( rcMbDataAccess, pcMbDataAccessBase, eMbMode, LIST_0 ) );

      RNOK(   MbCoder::xWriteReferenceFrames          ( rcMbDataAccess,                     eMbMode, LIST_0 ) );
      RNOK(   MbCoder::xWriteMotionVectors            ( rcMbDataAccess,                     eMbMode, LIST_0 ) );
    }

    if( rcRefFrameList1.getActive() && !bBLSkip )
    {
      RNOK(   MbCoder::xWriteMotionPredFlags          ( rcMbDataAccess, pcMbDataAccessBase, eMbMode, LIST_1 ) );

      RNOK(   MbCoder::xWriteReferenceFrames          ( rcMbDataAccess,                     eMbMode, LIST_1 ) );
      RNOK(   MbCoder::xWriteMotionVectors            ( rcMbDataAccess,                     eMbMode, LIST_1 ) );
    }
    
    uiMbBits  += BitCounter::getNumberOfWrittenBits();
  }

  //===== set rd-cost =====
  rcMbTempData.rdCost() = m_pcRateDistortionIf->getCost( uiMbBits+uiAdditionalBits, uiMbDist );

  return Err::m_nOK;
}

ErrVal
MbEncoder::xCheckInterMbMode8x8SR( IntMbTempData*&   rpcMbTempData,
																	 IntMbTempData*&   rpcMbBestData,
																	 IntMbTempData*    pcMbRefData,
																	 RefFrameList&     rcRefFrameList0,
																	 RefFrameList&     rcRefFrameList1,
																	 IntFrame*				 pcBaseLayerSbb,
																	 MbDataAccess*     pcMbDataAccessBaseMotion )
{
  ROTRS( ! rpcMbTempData->getSH().getPPS().getTransform8x8ModeFlag(), Err::m_nOK );
  ROTRS( ! pcMbRefData->is8x8TrafoFlagPresent(),                      Err::m_nOK );

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
    rpcMbTempData->setBLQRefFlag        (           pcMbRefData->getBLQRefFlag        () );
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

  RNOK( xSetRdCost8x8InterMbSR( *rpcMbTempData, pcMbDataAccessBaseMotion, rcRefFrameList0, rcRefFrameList1, pcBaseLayerSbb ) );
  RNOK( xCheckBestEstimation	(  rpcMbTempData, rpcMbBestData ) );

  return Err::m_nOK;
}

ErrVal
MbEncoder::xSetRdCost8x8InterMbSR ( IntMbTempData&  rcMbTempData,
																		MbDataAccess*   pcMbDataAccessBaseMotion,
																		RefFrameList&   rcRefFrameList0,
																		RefFrameList&   rcRefFrameList1,
																		IntFrame*				pcBaseLayerSbb,
																		Bool            bBLSkip,
																		UInt            uiAdditionalBits )
{
  IntYuvMbBuffer& rcYuvMbBuffer     = rcMbTempData;
  IntYuvMbBuffer& rcTempYuvMbBuffer = rcMbTempData.getTempYuvMbBuffer();
  MbDataAccess&   rcMbDataAccess    = rcMbTempData.getMbDataAccess();
  MbData&         rcMbData          = rcMbDataAccess.getMbData();
  UInt            uiMbDist          = 0;
  UInt            uiMbBits          = 0;
  MbMode          eMbMode           = rcMbData.getMbMode();
  Bool            b8x8Mode          = ( eMbMode == MODE_8x8 );
  UInt            uiFwdBwd          = 0;

  //=== check ===
  ROT( eMbMode == MODE_SKIP && rcMbDataAccess.getSH().isInterP() ); // not for skip mode
  ROT( eMbMode == MODE_8x8  && rcMbData.getBlkMode( B_8x8_0 ) != BLK_SKIP && rcMbData.getBlkMode( B_8x8_0 ) != BLK_8x8 );
  ROT( eMbMode == MODE_8x8  && rcMbData.getBlkMode( B_8x8_1 ) != BLK_SKIP && rcMbData.getBlkMode( B_8x8_1 ) != BLK_8x8 );
  ROT( eMbMode == MODE_8x8  && rcMbData.getBlkMode( B_8x8_2 ) != BLK_SKIP && rcMbData.getBlkMode( B_8x8_2 ) != BLK_8x8 );
  ROT( eMbMode == MODE_8x8  && rcMbData.getBlkMode( B_8x8_3 ) != BLK_SKIP && rcMbData.getBlkMode( B_8x8_3 ) != BLK_8x8 );



  //===== set forward / backward indication =====
  if( rcMbDataAccess.getSH().isInterB() )
  {
    for( Int n = 3; n >= 0; n--)
    {
      uiFwdBwd <<= 4;
      uiFwdBwd += (0 < rcMbDataAccess.getMbMotionData( LIST_0 ).getRefIdx( Par8x8(n) )) ? 1:0;
      uiFwdBwd += (0 < rcMbDataAccess.getMbMotionData( LIST_1 ).getRefIdx( Par8x8(n) )) ? 2:0;
    }
  }
  if( rcMbDataAccess.getSH().isInterP() )
  {
    for( Int n = 3; n >= 0; n--)
    {
      uiFwdBwd <<= 4;
      uiFwdBwd +=  (0 < rcMbDataAccess.getMbMotionData( LIST_0 ).getRefIdx( Par8x8(n) )) ? 1:0;
    }
  }
  rcMbTempData.setFwdBwd( uiFwdBwd );


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
                                              &rcYuvMbBuffer, false, false ) );
  }

	// compute smoothed reference prediction: P+Rb
	IntYuvMbBuffer cBaseResMbBuffer;
	cBaseResMbBuffer.loadBuffer	( pcBaseLayerSbb->getFullPelYuvBuffer() );
	rcYuvMbBuffer		.add				( cBaseResMbBuffer );

	// S(P+Rb)
	// note: only pixels inside MB are considered
	pcBaseLayerSbb->getFullPelYuvBuffer()->loadBuffer( &rcYuvMbBuffer		);
	pcBaseLayerSbb->getFullPelYuvBuffer()->smoothMbInside();

	// S(P+Rb)-Rb
	rcYuvMbBuffer.loadBuffer		( pcBaseLayerSbb->getFullPelYuvBuffer() );
	rcYuvMbBuffer.subtract			( cBaseResMbBuffer );

	// load into prediction buffer
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
    RNOK( MbCoder::xWriteMotionPredFlags          ( rcMbDataAccess, pcMbDataAccessBaseMotion, eMbMode, LIST_0 ) );

    RNOK( MbCoder::xWriteReferenceFrames          ( rcMbDataAccess, eMbMode, LIST_0 ) );
    RNOK( MbCoder::xWriteMotionVectors            ( rcMbDataAccess, eMbMode, LIST_0 ) );
  }

  if( rcRefFrameList1.getActive() && !bBLSkip )
  {
    RNOK( MbCoder::xWriteMotionPredFlags          ( rcMbDataAccess, pcMbDataAccessBaseMotion, eMbMode, LIST_1 ) );

    RNOK( MbCoder::xWriteReferenceFrames          ( rcMbDataAccess, eMbMode, LIST_1 ) );
    RNOK( MbCoder::xWriteMotionVectors            ( rcMbDataAccess, eMbMode, LIST_1 ) );
  }
  
  uiMbBits  += BitCounter::getNumberOfWrittenBits();

  
  //===== set rd-cost =====
  rcMbTempData.rdCost() = m_pcRateDistortionIf->getCost( uiMbBits+uiAdditionalBits, uiMbDist );

  return Err::m_nOK;
}
//--

ErrVal
MbEncoder::xSetRdCost8x8InterMb ( IntMbTempData&  rcMbTempData,
                                  MbDataAccess*   pcMbDataAccessBaseMotion,
                                  RefFrameList&   rcRefFrameList0,
                                  RefFrameList&   rcRefFrameList1,
                                  Bool            bBLSkip,
                                  UInt            uiAdditionalBits )
{
  IntYuvMbBuffer& rcYuvMbBuffer     = rcMbTempData;
  IntYuvMbBuffer& rcTempYuvMbBuffer = rcMbTempData.getTempYuvMbBuffer();
  MbDataAccess&   rcMbDataAccess    = rcMbTempData.getMbDataAccess();
  MbData&         rcMbData          = rcMbDataAccess.getMbData();
  UInt            uiMbDist          = 0;
  UInt            uiMbBits          = 0;
  MbMode          eMbMode           = rcMbData.getMbMode();
  Bool            b8x8Mode          = ( eMbMode == MODE_8x8 || eMbMode == MODE_8x8ref0 );
  UInt            uiFwdBwd          = 0;


  //=== check ===
  ROT( eMbMode == MODE_SKIP && rcMbDataAccess.getSH().isInterP() ); // not for skip mode
  ROT((eMbMode == MODE_8x8 || eMbMode == MODE_8x8ref0) && rcMbData.getBlkMode( B_8x8_0 ) != BLK_SKIP && rcMbData.getBlkMode( B_8x8_0 ) != BLK_8x8 );
  ROT((eMbMode == MODE_8x8 || eMbMode == MODE_8x8ref0) && rcMbData.getBlkMode( B_8x8_1 ) != BLK_SKIP && rcMbData.getBlkMode( B_8x8_1 ) != BLK_8x8 );
  ROT((eMbMode == MODE_8x8 || eMbMode == MODE_8x8ref0) && rcMbData.getBlkMode( B_8x8_2 ) != BLK_SKIP && rcMbData.getBlkMode( B_8x8_2 ) != BLK_8x8 );
  ROT((eMbMode == MODE_8x8 || eMbMode == MODE_8x8ref0) && rcMbData.getBlkMode( B_8x8_3 ) != BLK_SKIP && rcMbData.getBlkMode( B_8x8_3 ) != BLK_8x8 );



  //===== set forward / backward indication =====
  if( rcMbDataAccess.getSH().isInterB() )
  {
    for( Int n = 3; n >= 0; n--)
    {
      uiFwdBwd <<= 4;
      uiFwdBwd += (0 < rcMbDataAccess.getMbMotionData( LIST_0 ).getRefIdx( Par8x8(n) )) ? 1:0;
      uiFwdBwd += (0 < rcMbDataAccess.getMbMotionData( LIST_1 ).getRefIdx( Par8x8(n) )) ? 2:0;
    }
  }
  if( rcMbDataAccess.getSH().isInterP() )
  {
    for( Int n = 3; n >= 0; n--)
    {
      uiFwdBwd <<= 4;
      uiFwdBwd +=  (0 < rcMbDataAccess.getMbMotionData( LIST_0 ).getRefIdx( Par8x8(n) )) ? 1:0;
    }
  }
  rcMbTempData.setFwdBwd( uiFwdBwd );


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
                                              &rcYuvMbBuffer, false, false ) );
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
    RNOK( MbCoder::xWriteMotionPredFlags          ( rcMbDataAccess, pcMbDataAccessBaseMotion, eMbMode, LIST_0 ) );

    RNOK( MbCoder::xWriteReferenceFrames          ( rcMbDataAccess, eMbMode, LIST_0 ) );
    RNOK( MbCoder::xWriteMotionVectors            ( rcMbDataAccess, eMbMode, LIST_0 ) );
  }

  if( rcRefFrameList1.getActive() && !bBLSkip )
  {
    RNOK( MbCoder::xWriteMotionPredFlags          ( rcMbDataAccess, pcMbDataAccessBaseMotion, eMbMode, LIST_1 ) );

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
                                 UInt            uiAddBits )
{
  IntYuvMbBuffer& rcYuvMbBuffer     = rcMbTempData;
  IntYuvMbBuffer& rcTempYuvMbBuffer = rcMbTempData.getTempYuvMbBuffer();
  MbDataAccess&   rcMbDataAccess    = rcMbTempData.getMbDataAccess();
  MbData&         rcMbData          = rcMbDataAccess.getMbData();
  UInt            uiSubMbDist       = 0;
  UInt            uiSubMbBits       = 0;
  UInt            uiFwdBwd          = 0;


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

  return Err::m_nOK;
}






ErrVal
MbEncoder::xEstimateMbDirect( IntMbTempData*&  rpcMbTempData,
                              IntMbTempData*&  rpcMbBestData,
                              RefFrameList&    rcRefFrameList0,
                              RefFrameList&    rcRefFrameList1,
                              MbDataAccess*    pcMbDataAccessBaseMotion,
                              Bool             bResidualPred )
{
  ROFRS( rcRefFrameList0.getActive() && rcRefFrameList1.getActive(), Err::m_nOK );

  Int iRefIdxL0 = 1;
  Int iRefIdxL1 = 1;
  Mv  cMvPredL0;
  Mv  cMvPredL1;

  rpcMbTempData->getMbDataAccess    ().getMvPredictor         ( cMvPredL0, iRefIdxL0, LIST_0, PART_16x16 );
  rpcMbTempData->getMbDataAccess    ().getMvPredictor         ( cMvPredL1, iRefIdxL1, LIST_1, PART_16x16 );
  rpcMbTempData->clear              ();
  rpcMbTempData->setMbMode          ( MODE_SKIP );
  rpcMbTempData->setBLSkipFlag      ( false );
  rpcMbTempData->setBLQRefFlag      ( false );
  rpcMbTempData->getMbMotionData    ( LIST_0 ).setRefIdx      ( iRefIdxL0,    PART_16x16 );
  rpcMbTempData->getMbMotionData    ( LIST_0 ).setAllMv       ( cMvPredL0,    PART_16x16 );
  rpcMbTempData->getMbMvdData       ( LIST_0 ).setAllMv       ( Mv::ZeroMv(), PART_16x16 );
  rpcMbTempData->getMbMotionData    ( LIST_0 ).setMotPredFlag ( false );
  rpcMbTempData->getMbMotionData    ( LIST_1 ).setRefIdx      ( iRefIdxL1,    PART_16x16 );
  rpcMbTempData->getMbMotionData    ( LIST_1 ).setAllMv       ( cMvPredL1,    PART_16x16 );
  rpcMbTempData->getMbMvdData       ( LIST_1 ).setAllMv       ( Mv::ZeroMv(), PART_16x16 );
  rpcMbTempData->getMbMotionData    ( LIST_1 ).setMotPredFlag ( false );
  rpcMbTempData->setResidualPredFlag( bResidualPred, PART_16x16 );

  
  if( rpcMbTempData->getSH().isH264AVCCompatible() )
  {
    //===== H.264/AVC compatible direct mode =====
    Bool            bOneMv          = false;
    Bool            bFaultTolerant  = false;
    MbDataAccess&   rcMbDataAccess  = rpcMbTempData->getMbDataAccess();
    B8x8Idx         c8x8Idx;
    ROFRS( rcMbDataAccess.getMvPredictorDirect( c8x8Idx.b8x8(), bOneMv, bFaultTolerant ), Err::m_nOK ); c8x8Idx++;
    ROFRS( rcMbDataAccess.getMvPredictorDirect( c8x8Idx.b8x8(), bOneMv, bFaultTolerant ), Err::m_nOK ); c8x8Idx++;
    ROFRS( rcMbDataAccess.getMvPredictorDirect( c8x8Idx.b8x8(), bOneMv, bFaultTolerant ), Err::m_nOK ); c8x8Idx++;
    ROFRS( rcMbDataAccess.getMvPredictorDirect( c8x8Idx.b8x8(), bOneMv, bFaultTolerant ), Err::m_nOK ); 
  }

  IntMbTempData* pcMbRefData = rpcMbTempData;

  RNOK( xSetRdCostInterMb   ( *rpcMbTempData, pcMbDataAccessBaseMotion, rcRefFrameList0, rcRefFrameList1, false ) );
  RNOK( xCheckBestEstimation(  rpcMbTempData, rpcMbBestData ) );

  RNOK( xCheckInterMbMode8x8(  rpcMbTempData, rpcMbBestData, pcMbRefData, rcRefFrameList0, rcRefFrameList1, pcMbDataAccessBaseMotion ) );
 
  return Err::m_nOK;
}



ErrVal
MbEncoder::xEstimateMbBLSkip( IntMbTempData*&   rpcIntMbTempData,
                              IntMbTempData*&   rpcIntMbBestData,
                              RefFrameList&     rcRefFrameList0,
                              RefFrameList&     rcRefFrameList1,
                              const IntFrame*   pcBaseLayerRec,
                              Bool              bBSlice,
                              Int               iSpatialScalabilityType,
                              MbDataAccess*     pcMbDataAccessBase,
                              Bool              bResidualPred )
{
  ROF( pcMbDataAccessBase );

  if( ! pcMbDataAccessBase->getMbData().isIntra() )
  {
    ROF( bResidualPred || rpcIntMbTempData->getSH().getAdaptivePredictionFlag() );

    //===== BASE LAYER MODE IS INTER =====
    rpcIntMbTempData->clear               ();
    rpcIntMbTempData->copyMotion          ( pcMbDataAccessBase->getMbData() );
    rpcIntMbTempData->setBLSkipFlag       ( true  );
    rpcIntMbTempData->setBLQRefFlag       ( false );
    rpcIntMbTempData->getMbMvdData        ( LIST_0 ).setAllMv( Mv::ZeroMv() );
    rpcIntMbTempData->getMbMvdData        ( LIST_1 ).setAllMv( Mv::ZeroMv() );

    rpcIntMbTempData->setResidualPredFlag ( bResidualPred, PART_16x16 );

    IntMbTempData* pcMbRefData = rpcIntMbTempData;

    RNOK( xSetRdCostInterMb   ( *rpcIntMbTempData, pcMbDataAccessBase, rcRefFrameList0, rcRefFrameList1, true ) );
    RNOK( xCheckBestEstimation(  rpcIntMbTempData, rpcIntMbBestData ) );

    RNOK( xCheckInterMbMode8x8(  rpcIntMbTempData, rpcIntMbBestData, pcMbRefData, rcRefFrameList0, rcRefFrameList1, pcMbDataAccessBase ) );
  }
  else
  {
    ROT( bResidualPred );

    //===== INTRA MODE =====
    if ( pcMbDataAccessBase->getMbData().getInCropWindowFlag() ) // TMM_ESS
	    RNOK( xEstimateMbIntraBL  (  rpcIntMbTempData, rpcIntMbBestData, pcBaseLayerRec, bBSlice, pcMbDataAccessBase ) );
  }

  if( (iSpatialScalabilityType != SST_RATIO_1) && ! pcMbDataAccessBase->getMbData().isIntra() && rpcIntMbTempData->getSH().getAdaptivePredictionFlag() )
  {
    UInt  uiMaxIter = 3;

    switch( pcMbDataAccessBase->getMbData().getMbMode() )
    {
    case MODE_16x16:
      {
        RNOK( xQPelEstimateMb16x16( rpcIntMbTempData, rpcIntMbBestData, rcRefFrameList0, rcRefFrameList1, uiMaxIter, true, pcMbDataAccessBase, bResidualPred ) );
        break;
      }
    case MODE_16x8:
      {
        RNOK( xQPelEstimateMb16x8 ( rpcIntMbTempData, rpcIntMbBestData, rcRefFrameList0, rcRefFrameList1, uiMaxIter, true, pcMbDataAccessBase, bResidualPred ) );
        break;
      }
    case MODE_8x16:
      {
        RNOK( xQPelEstimateMb8x16 ( rpcIntMbTempData, rpcIntMbBestData, rcRefFrameList0, rcRefFrameList1, uiMaxIter, true, pcMbDataAccessBase, bResidualPred ) );
        break;
      }
    case MODE_8x8:
    case MODE_8x8ref0:
      {
        RNOK( xQPelEstimateMb8x8  ( rpcIntMbTempData, rpcIntMbBestData, rcRefFrameList0, rcRefFrameList1, uiMaxIter, true, pcMbDataAccessBase, bResidualPred ) );
        break;
      }
    default:
      ROT(1);
      break;
    }
  }


  return Err::m_nOK;
}

//-- JVT-R091
ErrVal
MbEncoder::xEstimateMbSR( IntMbTempData*&   rpcIntMbTempData,
                          IntMbTempData*&   rpcIntMbBestData,
                          RefFrameList&     rcRefFrameList0,
                          RefFrameList&     rcRefFrameList1,
                          const IntFrame*		pcBaseLayerSbb,
                          Bool              bBSlice,
                          Int               iSpatialScalabilityType,
                          MbDataAccess*     pcMbDataAccessBase,
                          Bool              bResidualPred )
{
  ROF( pcMbDataAccessBase );

  if( ! pcMbDataAccessBase->getMbData().isIntra() )
  {
    ROF( bResidualPred || rpcIntMbTempData->getSH().getAdaptivePredictionFlag() );

    //===== BASE LAYER MODE IS INTER =====
    rpcIntMbTempData->clear               ();
    rpcIntMbTempData->copyMotion          ( pcMbDataAccessBase->getMbData() );
    rpcIntMbTempData->setBLSkipFlag       ( true  );
    rpcIntMbTempData->setBLQRefFlag       ( false );
    rpcIntMbTempData->getMbMvdData        ( LIST_0 ).setAllMv( Mv::ZeroMv() );
    rpcIntMbTempData->getMbMvdData        ( LIST_1 ).setAllMv( Mv::ZeroMv() );

    rpcIntMbTempData->setResidualPredFlag ( bResidualPred, PART_16x16 );
		rpcIntMbTempData->setSmoothedRefFlag	( true );

    IntMbTempData* pcMbRefData = rpcIntMbTempData;

		IntFrame* pcTempFrame = (IntFrame*)pcBaseLayerSbb;
    RNOK( xSetRdCostInterMbSR   ( *rpcIntMbTempData, pcMbDataAccessBase, rcRefFrameList0, rcRefFrameList1, pcTempFrame, true ) );
    RNOK( xCheckBestEstimation	(  rpcIntMbTempData, rpcIntMbBestData ) );
    RNOK( xCheckInterMbMode8x8SR(  rpcIntMbTempData, rpcIntMbBestData, pcMbRefData, rcRefFrameList0, rcRefFrameList1, pcTempFrame, pcMbDataAccessBase ) );
  }

  return Err::m_nOK;
}
//--

ErrVal
MbEncoder::xEstimateMbSkip( IntMbTempData*&  rpcMbTempData,
                            IntMbTempData*&  rpcMbBestData,
                            RefFrameList&    rcRefFrameList0,
                            RefFrameList&    rcRefFrameList1 )
{
  ROFRS( rpcMbTempData->getSH().isH264AVCCompatible() &&
         rpcMbTempData->getSH().getSliceType() == P_SLICE, Err::m_nOK );

  ROF( rcRefFrameList0.getActive() );

  Int iRefIdxL0 = 1;
  Int iRefIdxL1 = BLOCK_NOT_PREDICTED;
  Mv  cMvPredL0;
  Mv  cMvPredL1;

  rpcMbTempData->clear();
  rpcMbTempData->setMbMode( MODE_SKIP );
  rpcMbTempData->setBLSkipFlag( false );
  rpcMbTempData->setBLQRefFlag( false );
  rpcMbTempData->getMbDataAccess().getMvPredictorSkipMode( cMvPredL0 );
  rpcMbTempData->getMbMotionData( LIST_0 ).setRefIdx( iRefIdxL0,    PART_16x16 );
  rpcMbTempData->getMbMotionData( LIST_0 ).setAllMv ( cMvPredL0,    PART_16x16 );
  rpcMbTempData->getMbMvdData   ( LIST_0 ).setAllMv ( Mv::ZeroMv(), PART_16x16 );
  rpcMbTempData->getMbMotionData( LIST_1 ).setRefIdx( iRefIdxL1,    PART_16x16 );
  rpcMbTempData->getMbMotionData( LIST_1 ).setAllMv ( cMvPredL1,    PART_16x16 );
  rpcMbTempData->getMbMvdData   ( LIST_1 ).setAllMv ( Mv::ZeroMv(), PART_16x16 );

  rpcMbTempData->getMbMotionData( LIST_0 ).setMotPredFlag( false );
  rpcMbTempData->getMbMotionData( LIST_1 ).setMotPredFlag( false );

  RNOK( xSetRdCostInterMb   ( *rpcMbTempData, NULL, rcRefFrameList0, rcRefFrameList1 ) );
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

  Bool            bPSlice       = rpcMbTempData->getMbDataAccess().getSH().isInterP();
  Double          fRdCost       = 0;
  UInt            uiCost  [3]   = { MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX }, uiCostTest;
  UInt            uiMbBits[3]   = { ( ! bPSlice ? 3 : 1 ), 3, 5 };
  Int             iRefIdx [2]   = { 0, 0 }, iRefIdxBi[2], iRefIdxTest;
  UInt            uiBits  [3], uiBitsTest;
  Mv              cMv[2], cMvBi[2], cMvPred[2][33], cMvLastEst[2][33], cMvd[2];
  IntYuvMbBuffer  cYuvMbBuffer[2];
  IntFrame*       pcRefFrame;


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
  rpcMbTempData->setBLQRefFlag( false );
  rpcMbTempData->setResidualPredFlag( bResidualPred, PART_16x16 );

  //===== LIST 0 PREDICTION ======
  for( iRefIdxTest = 1; iRefIdxTest <= (Int)rcRefFrameList0.getActive(); iRefIdxTest++ )
  {
    rpcMbTempData->getMbDataAccess().getMvPredictor   ( cMvPred[0][iRefIdxTest], iRefIdxTest,
                                                        LIST_0, PART_16x16 );
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
                                                        &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdxTest ) ) );
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
  
      rpcMbTempData->getMbDataAccess().setMvPredictorsBL( cBLMvPred[0], LIST_0, PART_16x16 );
      RNOK( m_pcMotionEstimation->estimateBlockWithStart( *rpcMbTempData, *pcRefFrame,
                                                          cBLMvLastEst[0],
                                                          cBLMvPred   [0],
                                                          uiBitsTest, uiCostTest,
                                                          PART_16x16, MODE_16x16, bQPelRefinementOnly, 0,
                                                          &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdxTest ) ) );
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
                                                        LIST_1, PART_16x16 );
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
                                                        PART_16x16, MODE_16x16, bQPel, 0,
                                                        &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdxTest ) ) );
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
  
      rpcMbTempData->getMbDataAccess().setMvPredictorsBL( cBLMvPred[1], LIST_1, PART_16x16 );
      RNOK( m_pcMotionEstimation->estimateBlockWithStart( *rpcMbTempData, *pcRefFrame,
                                                          cBLMvLastEst[1],
                                                          cBLMvPred   [1],
                                                          uiBitsTest, uiCostTest,
                                                          PART_16x16, MODE_16x16, bQPelRefinementOnly, 0,
                                                          &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdxTest ) ) );
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
      cBSParams.apcWeight[LIST_0]   = &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdxBi[LIST_0]  );
      cBSParams.apcWeight[LIST_1]   = &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdxBi[LIST_1]  );

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
  rpcMbTempData->getMbMotionData( LIST_0 ).setRefIdx    ( iRefIdx [0],  PART_16x16 );
  rpcMbTempData->getMbMotionData( LIST_0 ).setAllMv     ( cMv     [0],  PART_16x16 );
  rpcMbTempData->getMbMvdData   ( LIST_0 ).setAllMv     ( cMvd    [0],  PART_16x16 );
  
  rpcMbTempData->getMbMotionData( LIST_1 ).setRefIdx    ( iRefIdx [1],  PART_16x16 );
  rpcMbTempData->getMbMotionData( LIST_1 ).setAllMv     ( cMv     [1],  PART_16x16 );
  rpcMbTempData->getMbMvdData   ( LIST_1 ).setAllMv     ( cMvd    [1],  PART_16x16 );

  rpcMbTempData->getMbMotionData( LIST_0 ).setMotPredFlag( bBLPred [0] );
  rpcMbTempData->getMbMotionData( LIST_1 ).setMotPredFlag( bBLPred [1] );
  ROT( bBLPred[0] && iRefIdx[0] != iBLRefIdx[0] );
  ROT( bBLPred[1] && iRefIdx[1] != iBLRefIdx[1] );

  IntMbTempData* pcMbRefData = rpcMbTempData;
  
  RNOK( xSetRdCostInterMb   ( *rpcMbTempData, pcMbDataAccessBase, rcRefFrameList0, rcRefFrameList1 ) );
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
  rpcMbTempData->setBLQRefFlag( false );
  rpcMbTempData->setResidualPredFlag( bResidualPred, PART_16x16 );

  Bool   bPSlice       = rpcMbTempData->getMbDataAccess().getSH().isInterP();
  Double fRdCost       = 0;
  UInt   uiLastMode    = 0;
  
  for( UInt   uiBlk = 0; uiBlk < 2; uiBlk++ )
  {
    ParIdx16x8      eParIdx     = ( uiBlk ? PART_16x8_1 : PART_16x8_0 );
    UInt            uiCost  [3] = { MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX }, uiCostTest;
    Int             iRefIdx [2] = { 0, 0 }, iRefIdxBi[2], iRefIdxTest;
    UInt            uiMbBits[3] = { 3, 0, 0 };
    UInt            uiBits  [3], uiBitsTest;
    Mv              cMv[2], cMvBi[2], cMvPred[2][33], cMvLastEst[2][33], cMvd[2];
    IntYuvMbBuffer  cYuvMbBuffer[2];
    IntFrame*       pcRefFrame;


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
      if( pcMbDataAccessBase->getMbMotionData( LIST_1 ).getRefIdx( eParIdx ) >  1                           &&
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
                                                          &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdxTest ) ) );
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
                                                            &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdxTest ) ) );
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
                                                          &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdxTest ) ) );
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
                                                            &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdxTest ) ) );
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
        cBSParams.apcWeight[LIST_0]   = &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdxBi[LIST_0]  );
        cBSParams.apcWeight[LIST_1]   = &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdxBi[LIST_1]  );

        for( iRefIdxTest = 1; iRefIdxTest <= (Int)rcRefFrameList.getActive(); iRefIdxTest++ )
        {
          UInt  uiBitsTest  = ( uiBasePredType == 2 ? 0 : uiMbBits[2] ) + uiMotBits[1-uiDir] + getRefIdxBits( iRefIdxTest, rcRefFrameList );
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

  RNOK( xSetRdCostInterMb   ( *rpcMbTempData, pcMbDataAccessBase, rcRefFrameList0, rcRefFrameList1 ) );
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
  rpcMbTempData->setBLQRefFlag( false );
  rpcMbTempData->setResidualPredFlag( bResidualPred, PART_16x16 );

  Bool   bPSlice       = rpcMbTempData->getMbDataAccess().getSH().isInterP();
  Double fRdCost       = 0;
  UInt   uiLastMode    = 0;

  for( UInt   uiBlk = 0; uiBlk < 2; uiBlk++ )
  {
    ParIdx8x16      eParIdx     = ( uiBlk ? PART_8x16_1 : PART_8x16_0 );
    UInt            uiCost  [3] = { MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX }, uiCostTest;
    Int             iRefIdx [2] = { 0, 0 }, iRefIdxBi[2], iRefIdxTest;
    UInt            uiMbBits[3] = { 3, 0, 0 };
    UInt            uiBits  [3], uiBitsTest;
    Mv              cMv[2], cMvBi[2], cMvPred[2][33], cMvLastEst[2][33], cMvd[2];
    IntYuvMbBuffer  cYuvMbBuffer[2];
    IntFrame*       pcRefFrame;


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
                                                          &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdxTest ) ) );
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
                                                            &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdxTest ) ) );
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
                                                          &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdxTest ) ) );
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
                                                            &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdxTest ) ) );
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
        cBSParams.apcWeight[LIST_0]   = &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdxBi[LIST_0]  );
        cBSParams.apcWeight[LIST_1]   = &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdxBi[LIST_1]  );

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

  RNOK( xSetRdCostInterMb   ( *rpcMbTempData, pcMbDataAccessBase, rcRefFrameList0, rcRefFrameList1 ) );
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
  Bool  bPSlice  = rpcMbTempData->getMbDataAccess().getSH().isInterP();
  UInt  uiBits   = ( ! bPSlice ? 9 : 5 ); // for signalling macroblock mode

  rpcMbTempData->clear    ();
  rpcMbTempData->setMbMode( MODE_8x8 );
  rpcMbTempData->setBLSkipFlag( false );
  rpcMbTempData->setBLQRefFlag( false );
  rpcMbTempData->setResidualPredFlag( bResidualPred, PART_16x16 );
  rpcMbTempData->rdCost   () = 0;

  for( Par8x8 ePar8x8 = B_8x8_0; ePar8x8 < 4; ePar8x8 = Par8x8( ePar8x8 + 1 ), uiBits = 0 )
  {
    ParIdx8x8 aeParIdx8x8[4]  = { PART_8x8_0, PART_8x8_1, PART_8x8_2, PART_8x8_3 };
    ParIdx8x8 eParIdx8x8      = aeParIdx8x8[ ePar8x8 ];

    m_pcIntMbBest8x8Data->clear ();
    RNOK( xEstimateSubMbDirect  ( ePar8x8, m_pcIntMbTemp8x8Data, m_pcIntMbBest8x8Data, rcRefFrameList0, rcRefFrameList1, false,                                               uiBits,                      pcMbDataAccessBase ) );
    RNOK( xEstimateSubMb8x8     ( ePar8x8, m_pcIntMbTemp8x8Data, m_pcIntMbBest8x8Data, rcRefFrameList0, rcRefFrameList1, false, bBiPredOnly, uiNumMaxIter, uiIterSearchRange, uiBits, bQPelRefinementOnly, pcMbDataAccessBase ) );
    RNOK( xEstimateSubMb8x4     ( ePar8x8, m_pcIntMbTemp8x8Data, m_pcIntMbBest8x8Data, rcRefFrameList0, rcRefFrameList1,        bBiPredOnly, uiNumMaxIter, uiIterSearchRange, uiBits, bQPelRefinementOnly, pcMbDataAccessBase ) );
    RNOK( xEstimateSubMb4x8     ( ePar8x8, m_pcIntMbTemp8x8Data, m_pcIntMbBest8x8Data, rcRefFrameList0, rcRefFrameList1,        bBiPredOnly, uiNumMaxIter, uiIterSearchRange, uiBits, bQPelRefinementOnly, pcMbDataAccessBase ) );
    RNOK( xEstimateSubMb4x4     ( ePar8x8, m_pcIntMbTemp8x8Data, m_pcIntMbBest8x8Data, rcRefFrameList0, rcRefFrameList1,        bBiPredOnly, uiNumMaxIter, uiIterSearchRange, uiBits, bQPelRefinementOnly, pcMbDataAccessBase ) );

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

  RNOK( xSetRdCostInterMb   ( *rpcMbTempData, pcMbDataAccessBase, rcRefFrameList0, rcRefFrameList1 ) );
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
  

  Bool  bPSlice  = rpcMbTempData->getMbDataAccess().getSH().isInterP();
  UInt  uiBits   = ( ! bPSlice ? 9 : 5 ); // for signalling macroblock mode

  rpcMbTempData->clear    ();
  rpcMbTempData->setMbMode( MODE_8x8 );
  rpcMbTempData->setBLSkipFlag( false );
  rpcMbTempData->setBLQRefFlag( false );
  rpcMbTempData->setResidualPredFlag( bResidualPred, PART_16x16 );
  rpcMbTempData->rdCost   () = 0;

  for( Par8x8 ePar8x8 = B_8x8_0; ePar8x8 < 4; ePar8x8 = Par8x8( ePar8x8 + 1 ), uiBits = 0 )
  {
    ParIdx8x8 aeParIdx8x8[4]  = { PART_8x8_0, PART_8x8_1, PART_8x8_2, PART_8x8_3 };
    ParIdx8x8 eParIdx8x8      = aeParIdx8x8[ ePar8x8 ];

    m_pcIntMbBest8x8Data->clear ();
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
      ROFRS( rcMbDataAccess.getMvPredictorDirect( eParIdx8x8, bOneMv, bFaultTolerant ), Err::m_nOK ); 
    }

    RNOK( xSetRdCostInterSubMb( *rpcMbTempData, rcRefFrameList0, rcRefFrameList1, B8x8Idx( ePar8x8 ), bTrafo8x8, 1+uiAddBits ) );
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
  

  Bool            bPSlice         = rpcMbTempData->getMbDataAccess().getSH().isInterP();
  Double          fRdCost         = 0;
  UInt            uiSubMbBits     = 0;  
  ParIdx8x8       aeParIdx8x8 [4] = { PART_8x8_0, PART_8x8_1, PART_8x8_2, PART_8x8_3 };
  ParIdx8x8       eParIdx8x8      = aeParIdx8x8[ ePar8x8 ];
  UInt            uiCost      [3] = { MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX }, uiCostTest;
  UInt            uiBlkBits   [3] = { ( ! bPSlice ? 3 : 1 ) + uiAddBits, 3 + uiAddBits, 5 + uiAddBits };
  Int             iRefIdx     [2] = { 0, 0 }, iRefIdxBi[2], iRefIdxTest;
  UInt            uiBits      [3], uiBitsTest;
  Mv              cMv[2], cMvBi[2], cMvPred[2][33], cMvLastEst[2][33], cMvd[2];
  IntYuvMbBuffer  cYuvMbBuffer[2];
  IntFrame*       pcRefFrame;


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
                                                        LIST_0, eParIdx8x8, SPART_8x8 );
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
                                                        &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdxTest ) ) );
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
  
      rpcMbTempData->getMbDataAccess().setMvPredictorsBL( cBLMvPred[0], LIST_0, eParIdx8x8, SPART_8x8 );
      RNOK( m_pcMotionEstimation->estimateBlockWithStart(  *rpcMbTempData, *pcRefFrame,
                                                          cBLMvLastEst[0],
                                                          cBLMvPred   [0],
                                                          uiBitsTest, uiCostTest,
                                                          eParIdx8x8+SPART_8x8, BLK_8x8, bQPelRefinementOnly, 0,
                                                          &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdxTest ) ) );
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
                                                        LIST_1, eParIdx8x8, SPART_8x8 );
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
                                                        &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdxTest ) ) );
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
  
      rpcMbTempData->getMbDataAccess().setMvPredictorsBL( cBLMvPred[1], LIST_1, eParIdx8x8, SPART_8x8 );
      RNOK( m_pcMotionEstimation->estimateBlockWithStart(  *rpcMbTempData, *pcRefFrame,
                                                          cBLMvLastEst[1],
                                                          cBLMvPred   [1],
                                                          uiBitsTest, uiCostTest,
                                                          eParIdx8x8+SPART_8x8, BLK_8x8, bQPelRefinementOnly, 0,
                                                          &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdxTest ) ) );
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
      cBSParams.apcWeight[LIST_0]   = &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdxBi[LIST_0]  );
      cBSParams.apcWeight[LIST_1]   = &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdxBi[LIST_1]  );

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
  rpcMbTempData->getMbMotionData( LIST_0 ).setAllMv ( cMv     [0],  eParIdx8x8, SPART_8x8 );
  rpcMbTempData->getMbMvdData   ( LIST_0 ).setAllMv ( cMvd    [0],  eParIdx8x8, SPART_8x8 );
  rpcMbTempData->getMbMotionData( LIST_1 ).setRefIdx( iRefIdx [1],  eParIdx8x8 );
  rpcMbTempData->getMbMotionData( LIST_1 ).setAllMv ( cMv     [1],  eParIdx8x8, SPART_8x8 );
  rpcMbTempData->getMbMvdData   ( LIST_1 ).setAllMv ( cMvd    [1],  eParIdx8x8, SPART_8x8 );

  rpcMbTempData->getMbMotionData( LIST_0 ).setMotPredFlag( bBLPred[0], eParIdx8x8 );
  rpcMbTempData->getMbMotionData( LIST_1 ).setMotPredFlag( bBLPred[1], eParIdx8x8 );
  ROT( bBLPred[0] && iRefIdx[0] != iBLRefIdx[0] );
  ROT( bBLPred[1] && iRefIdx[1] != iBLRefIdx[1] );

  RNOK( xSetRdCostInterSubMb( *rpcMbTempData, rcRefFrameList0, rcRefFrameList1, B8x8Idx( ePar8x8 ), bTrafo8x8, uiSubMbBits ) );
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
  

  Bool            bPSlice         = rpcMbTempData->getMbDataAccess().getSH().isInterP();
  Double          fRdCost         = 0;
  UInt            uiSubMbBits     = 0;
  SParIdx8x4      aeParIdx8x4 [2] = { SPART_8x4_0, SPART_8x4_1 };
  ParIdx8x8       aeParIdx8x8 [4] = { PART_8x8_0, PART_8x8_1, PART_8x8_2, PART_8x8_3 };
  ParIdx8x8       eParIdx8x8      = aeParIdx8x8[ ePar8x8 ];
  UInt            uiCost      [3] = { MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX }, uiCostTest;
  UInt            uiBlkBits   [3] = { ( ! bPSlice ? 5 : 3 ) + uiAddBits, 5 + uiAddBits, 7 + uiAddBits };
  Int             iRefIdx     [2] = { 0, 0 }, iRefIdxBi[2], iRefIdxTest;
  UInt            uiBits      [3], uiBitsTest;
  Mv              cMv[2][2], cMvBi[2][2], cMvd[2][2], cMvLastEst[2][33][2], cMvPred[2][33][2], cMvPredBi[2][2];
  IntYuvMbBuffer  cYuvMbBuffer[2], cTmpYuvMbBuffer;
  IntFrame*       pcRefFrame;


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
                                                          &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdxTest ) ) );
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
                                                            &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdxTest ) ) );
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
                                                          &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdxTest ) ) );
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
                                                            &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdxTest ) ) );
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
      cBSParams.apcWeight[LIST_0]   = &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdxBi[LIST_0]  );
      cBSParams.apcWeight[LIST_1]   = &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdxBi[LIST_1]  );

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

  RNOK( xSetRdCostInterSubMb( *rpcMbTempData, rcRefFrameList0, rcRefFrameList1, B8x8Idx( ePar8x8 ), false, uiSubMbBits ) );
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
  
  Bool            bPSlice         = rpcMbTempData->getMbDataAccess().getSH().isInterP();
  Double          fRdCost         = 0;
  UInt            uiSubMbBits     = 0;
  SParIdx4x8      aeParIdx4x8 [2] = { SPART_4x8_0, SPART_4x8_1 };
  ParIdx8x8       aeParIdx8x8 [4] = { PART_8x8_0, PART_8x8_1, PART_8x8_2, PART_8x8_3 };
  ParIdx8x8       eParIdx8x8      = aeParIdx8x8[ ePar8x8 ];
  UInt            uiCost      [3] = { MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX }, uiCostTest;
  UInt            uiBlkBits   [3] = { ( ! bPSlice ? 5 : 3 ) + uiAddBits, 7 + uiAddBits, 7 + uiAddBits };
  Int             iRefIdx     [2] = { 0, 0 }, iRefIdxBi[2], iRefIdxTest;
  UInt            uiBits      [3], uiBitsTest;
  Mv              cMv[2][2], cMvBi[2][2], cMvd[2][2], cMvLastEst[2][33][2], cMvPred[2][33][2], cMvPredBi[2][2];
  IntYuvMbBuffer  cYuvMbBuffer[2], cTmpYuvMbBuffer;
  IntFrame*       pcRefFrame;


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
                                                          &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdxTest ) ) );
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
                                                            &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdxTest ) ) );
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
                                                          &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdxTest ) ) );
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
                                                            &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdxTest ) ) );
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
      cBSParams.apcWeight[LIST_0]   = &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdxBi[LIST_0]  );
      cBSParams.apcWeight[LIST_1]   = &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdxBi[LIST_1]  );

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

  RNOK( xSetRdCostInterSubMb( *rpcMbTempData, rcRefFrameList0, rcRefFrameList1, B8x8Idx( ePar8x8 ), false, uiSubMbBits ) );
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


  Bool            bPSlice         = rpcMbTempData->getMbDataAccess().getSH().isInterP();
  Double          fRdCost         = 0;
  UInt            uiSubMbBits     = 0;
  SParIdx4x4      aeParIdx4x4 [4] = { SPART_4x4_0, SPART_4x4_1, SPART_4x4_2, SPART_4x4_3 };
  ParIdx8x8       aeParIdx8x8 [4] = { PART_8x8_0, PART_8x8_1, PART_8x8_2, PART_8x8_3 };
  ParIdx8x8       eParIdx8x8      = aeParIdx8x8[ ePar8x8 ];
  UInt            uiCost      [3] = { MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX }, uiCostTest;
  UInt            uiBlkBits   [3] = { ( ! bPSlice ? 7 : 5 ) + uiAddBits, 7 + uiAddBits, 7 + uiAddBits };
  Int             iRefIdx     [2] = { 0, 0 }, iRefIdxBi[2], iRefIdxTest;
  UInt            uiBits      [3], uiBitsTest;
  Mv              cMv[2][4], cMvBi[2][4], cMvd[2][4], cMvLastEst[2][33][4], cMvPred[2][33][4], cMvPredBi[2][4];
  IntYuvMbBuffer  cYuvMbBuffer[2], cTmpYuvMbBuffer;
  IntFrame*       pcRefFrame;


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
                                                          &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdxTest ) ) );
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
                                                            &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdxTest ) ) );
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
                                                          &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdxTest ) ) );
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
                                                            &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdxTest ) ) );
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
      cBSParams.apcWeight[LIST_0]   = &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdxBi[LIST_0]  );
      cBSParams.apcWeight[LIST_1]   = &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdxBi[LIST_1]  );

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

  RNOK( xSetRdCostInterSubMb( *rpcMbTempData, rcRefFrameList0, rcRefFrameList1, B8x8Idx( ePar8x8 ), false, uiSubMbBits ) );
  RNOK( xCheckBestEstimation(  rpcMbTempData, rpcMbBestData ) );

  return Err::m_nOK;
}





ErrVal
MbEncoder::xQPelEstimateMb16x16( IntMbTempData*&  rpcMbTempData,
                                 IntMbTempData*&  rpcMbBestData,
                                 RefFrameList&    rcRefFrameList0,
                                 RefFrameList&    rcRefFrameList1,
                                 UInt             uiNumMaxIter,
                                 Bool             bQPelOnly,
                                 MbDataAccess*    pcMbDataAccessBase,
                                 Bool             bResidualPred )
{
  ROF( pcMbDataAccessBase );

  UInt            uiFwdBwd        = pcMbDataAccessBase->getMbData().getFwdBwd();
  Bool            bList0          = ( ( uiFwdBwd & 0x1111 ) != 0 );
  Bool            bList1          = ( ( uiFwdBwd & 0x2222 ) != 0 );
  Int             iRefIdx     [2] = { BLOCK_NOT_PREDICTED, BLOCK_NOT_PREDICTED };
  Mv              cMv         [2];
  Mv              cMvd        [2];
  Mv              cMvPred     [2];
  UInt            uiBits      [2];
  IntYuvMbBuffer  cYuvMbBuffer[2];
  IntFrame*       pcRefFrame;
  UInt            uiCostTest, uiMvdBits;

  rpcMbTempData->clear();
  rpcMbTempData->setMbMode( MODE_16x16 );
  rpcMbTempData->setBLSkipFlag( false );
  rpcMbTempData->setBLQRefFlag( true  );
  rpcMbTempData->setResidualPredFlag( bResidualPred, PART_16x16 );

  if( bList0 )
  {
    //===== LIST 0 PREDICTION ======
    iRefIdx [0] = pcMbDataAccessBase->getMbMotionData( LIST_0 ).getRefIdx ();
    cMv     [0] = pcMbDataAccessBase->getMbMotionData( LIST_0 ).getMv     ();
    cMvPred [0] = cMv[0];
    uiBits  [0] = 0;
    pcRefFrame  = rcRefFrameList0[iRefIdx[0]];
    RNOK( m_pcMotionEstimation->estimateBlockWithStart( *rpcMbTempData, *pcRefFrame,
                                                        cMv[0], cMvPred[0], uiBits[0], uiCostTest,
                                                        PART_16x16, MODE_16x16, bQPelOnly, 0,
                                                        &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdx[0] ) ) );
    if( bQPelOnly )
    {
      if      ( uiBits[0] == 2 )  uiBits[0] = 2;
      else if ( uiBits[0] == 4 )  uiBits[0] = 3;
      else if ( uiBits[0] == 6 )  uiBits[0] = 4;
      else                        ROT(1);
    }
  }
  
  if( bList1 )
  {
    //===== LIST 1 PREDICTION ======
    iRefIdx [1] = pcMbDataAccessBase->getMbMotionData( LIST_1 ).getRefIdx ();
    cMv     [1] = pcMbDataAccessBase->getMbMotionData( LIST_1 ).getMv     ();
    cMvPred [1] = cMv[1];
    uiBits  [1] = 0;
    pcRefFrame  = rcRefFrameList1[iRefIdx[1]];
    RNOK( m_pcMotionEstimation->estimateBlockWithStart( *rpcMbTempData, *pcRefFrame,
                                                        cMv[1], cMvPred[1], uiBits[1], uiCostTest,
                                                        PART_16x16, MODE_16x16, bQPelOnly, 0,
                                                        &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdx[1] ) ) );
    RNOK( m_pcMotionEstimation->compensateBlock       ( &cYuvMbBuffer[1],
                                                        PART_16x16, MODE_16x16 ) );
    if( bQPelOnly )
    {
      if      ( uiBits[1] == 2 )  uiBits[1] = 2;
      else if ( uiBits[1] == 4 )  uiBits[1] = 3;
      else if ( uiBits[1] == 6 )  uiBits[1] = 4;
      else                        ROT(1);
    }
  }

  if( bList0 && bList1 )
  {
    //===== BI PREDICTION =====
    if( uiNumMaxIter == 0 )
    {
      uiNumMaxIter = 1;
    }

    //----- iterative search -----
    for( UInt uiMinCost = MSYS_UINT_MAX, uiIter = 0; uiIter < uiNumMaxIter; uiIter++ )
    {
      UInt          uiBitsTest;
      Mv            cMvTest;
      UInt          uiDir           = uiIter % 2;
      RefFrameList& rcRefFrameList  = ( uiDir ? rcRefFrameList1 : rcRefFrameList0 );
      BSParams      cBSParams;
      cBSParams.pcAltRefFrame       = ( uiDir ? rcRefFrameList0 : rcRefFrameList1 )[ iRefIdx[ 1 - uiDir ] ];
      cBSParams.pcAltRefPelData     = &cYuvMbBuffer[1-uiDir];
      cBSParams.uiL1Search          = uiDir;
      cBSParams.apcWeight[LIST_0]   = &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdx[LIST_0]  );
      cBSParams.apcWeight[LIST_1]   = &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdx[LIST_1]  );

      cMvTest     = cMvPred [uiDir];
      uiBitsTest  = uiBits[1-uiDir];
      pcRefFrame  = rcRefFrameList[iRefIdx[uiDir]];
      RNOK( m_pcMotionEstimation->estimateBlockWithStart( *rpcMbTempData, *pcRefFrame,
                                                          cMvTest, cMvPred[uiDir], uiBitsTest, uiCostTest,
                                                          PART_16x16, MODE_16x16, bQPelOnly, 0, 0, &cBSParams ) );

      if( uiCostTest >= uiMinCost )
      {
        break;
      }

      uiMinCost     = uiCostTest;
      cMv   [uiDir] = cMvTest;
      uiBits[uiDir] = uiBitsTest - uiBits[1-uiDir];

      if( bQPelOnly )
      {
        if      ( uiBits[uiDir] == 2 )  uiBits[uiDir] = 2;
        else if ( uiBits[uiDir] == 4 )  uiBits[uiDir] = 3;
        else if ( uiBits[uiDir] == 6 )  uiBits[uiDir] = 4;
        else                            ROT(1);
      }

      RNOK( m_pcMotionEstimation->compensateBlock       ( &cYuvMbBuffer[uiDir],
                                                          PART_16x16, MODE_16x16 ) );
    }
  }


  //===== set parameters =====
  uiMvdBits = ( bList0 ? uiBits[0] : 0 ) + ( bList1 ? uiBits[1] : 0 );
  cMvd  [0] = cMv[0] - cMvPred[0];
  cMvd  [1] = cMv[1] - cMvPred[1];
  
  rpcMbTempData->getMbMotionData( LIST_0 ).setRefIdx      ( iRefIdx [0],  PART_16x16 );
  rpcMbTempData->getMbMotionData( LIST_0 ).setAllMv       ( cMv     [0],  PART_16x16 );
  rpcMbTempData->getMbMvdData   ( LIST_0 ).setAllMv       ( cMvd    [0],  PART_16x16 );
  rpcMbTempData->getMbMotionData( LIST_0 ).setMotPredFlag ( true );
  rpcMbTempData->getMbMotionData( LIST_1 ).setRefIdx      ( iRefIdx [1],  PART_16x16 );
  rpcMbTempData->getMbMotionData( LIST_1 ).setAllMv       ( cMv     [1],  PART_16x16 );
  rpcMbTempData->getMbMvdData   ( LIST_1 ).setAllMv       ( cMvd    [1],  PART_16x16 );
  rpcMbTempData->getMbMotionData( LIST_1 ).setMotPredFlag ( true );

  IntMbTempData* pcMbRefData = rpcMbTempData;

  RNOK( xSetRdCostInterMb   ( *rpcMbTempData, pcMbDataAccessBase, rcRefFrameList0, rcRefFrameList1, true, 1+uiMvdBits ) );
  RNOK( xCheckBestEstimation(  rpcMbTempData, rpcMbBestData ) );

  RNOK( xCheckInterMbMode8x8(  rpcMbTempData, rpcMbBestData, pcMbRefData, rcRefFrameList0, rcRefFrameList1, pcMbDataAccessBase ) );

  return Err::m_nOK;
}





ErrVal
MbEncoder::xQPelEstimateMb16x8 ( IntMbTempData*&  rpcMbTempData,
                                 IntMbTempData*&  rpcMbBestData,
                                 RefFrameList&    rcRefFrameList0,
                                 RefFrameList&    rcRefFrameList1,
                                 UInt             uiNumMaxIter,
                                 Bool             bQPelOnly,
                                 MbDataAccess*    pcMbDataAccessBase,
                                 Bool             bResidualPred )
{
  ROF( pcMbDataAccessBase );

  rpcMbTempData->clear();
  rpcMbTempData->setMbMode( MODE_16x8 );
  rpcMbTempData->setBLSkipFlag( false );
  rpcMbTempData->setBLQRefFlag( true  );
  rpcMbTempData->setResidualPredFlag( bResidualPred, PART_16x16 );

  UInt  uiMvdBits = 0;

  for( UInt uiBlk = 0; uiBlk < 2; uiBlk++ )
  {
    ParIdx16x8      eParIdx         = ( uiBlk ? PART_16x8_1 : PART_16x8_0 );
    UInt            uiFwdBwd        = pcMbDataAccessBase->getMbData().getFwdBwd( B4x4Idx( eParIdx ) );
    Bool            bList0          = ( ( uiFwdBwd & 0x1111 ) != 0 );
    Bool            bList1          = ( ( uiFwdBwd & 0x2222 ) != 0 );
    Int             iRefIdx     [2] = { BLOCK_NOT_PREDICTED, BLOCK_NOT_PREDICTED };
    Mv              cMv         [2];
    Mv              cMvd        [2];
    Mv              cMvPred     [2];
    UInt            uiBits      [2];
    IntYuvMbBuffer  cYuvMbBuffer[2];
    IntFrame*       pcRefFrame;
    UInt            uiCostTest;

    if( bList0 )
    {
      //===== LIST 0 PREDICTION ======
      iRefIdx [0] = pcMbDataAccessBase->getMbMotionData( LIST_0 ).getRefIdx ( eParIdx );
      cMv     [0] = pcMbDataAccessBase->getMbMotionData( LIST_0 ).getMv     ( eParIdx );
      cMvPred [0] = cMv[0];
      uiBits  [0] = 0;
      pcRefFrame  = rcRefFrameList0[iRefIdx[0]];
      RNOK( m_pcMotionEstimation->estimateBlockWithStart( *rpcMbTempData, *pcRefFrame,
                                                          cMv[0], cMvPred[0], uiBits[0], uiCostTest,
                                                          eParIdx, MODE_16x8, bQPelOnly, 0,
                                                          &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdx[0] ) ) );
      if( bQPelOnly )
      {
        if      ( uiBits[0] == 2 )  uiBits[0] = 2;
        else if ( uiBits[0] == 4 )  uiBits[0] = 3;
        else if ( uiBits[0] == 6 )  uiBits[0] = 4;
        else                        ROT(1);
      }
    }

    if( bList1 )
    {
      //===== LIST 1 PREDICTION ======
      iRefIdx [1] = pcMbDataAccessBase->getMbMotionData( LIST_1 ).getRefIdx ( eParIdx );
      cMv     [1] = pcMbDataAccessBase->getMbMotionData( LIST_1 ).getMv     ( eParIdx );
      cMvPred [1] = cMv[1];
      uiBits  [1] = 0;
      pcRefFrame  = rcRefFrameList1[iRefIdx[1]];
      RNOK( m_pcMotionEstimation->estimateBlockWithStart( *rpcMbTempData, *pcRefFrame,
                                                          cMv[1], cMvPred[1], uiBits[1], uiCostTest,
                                                          eParIdx, MODE_16x8, bQPelOnly, 0,
                                                          &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdx[1] ) ) );
      RNOK( m_pcMotionEstimation->compensateBlock       ( &cYuvMbBuffer[1],
                                                          eParIdx, MODE_16x8 ) );
      if( bQPelOnly )
      {
        if      ( uiBits[1] == 2 )  uiBits[1] = 2;
        else if ( uiBits[1] == 4 )  uiBits[1] = 3;
        else if ( uiBits[1] == 6 )  uiBits[1] = 4;
        else                        ROT(1);
      }
    }

    if( bList0 && bList1 )
    {
      //===== BI PREDICTION =====
      if( uiNumMaxIter == 0 )
      {
        uiNumMaxIter = 1;
      }

      //----- iterative search -----
      for( UInt uiMinCost = MSYS_UINT_MAX, uiIter = 0; uiIter < uiNumMaxIter; uiIter++ )
      {
        UInt          uiBitsTest;
        Mv            cMvTest;
        UInt          uiDir           = uiIter % 2;
        RefFrameList& rcRefFrameList  = ( uiDir ? rcRefFrameList1 : rcRefFrameList0 );
        BSParams      cBSParams;
        cBSParams.pcAltRefFrame       = ( uiDir ? rcRefFrameList0 : rcRefFrameList1 )[ iRefIdx[ 1 - uiDir ] ];
        cBSParams.pcAltRefPelData     = &cYuvMbBuffer[1-uiDir];
        cBSParams.uiL1Search          = uiDir;
        cBSParams.apcWeight[LIST_0]   = &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdx[LIST_0]  );
        cBSParams.apcWeight[LIST_1]   = &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdx[LIST_1]  );

        cMvTest     = cMvPred [uiDir];
        uiBitsTest  = uiBits[1-uiDir];
        pcRefFrame  = rcRefFrameList[iRefIdx[uiDir]];
        RNOK( m_pcMotionEstimation->estimateBlockWithStart( *rpcMbTempData, *pcRefFrame,
                                                            cMvTest, cMvPred[uiDir], uiBitsTest, uiCostTest,
                                                            eParIdx, MODE_16x8, bQPelOnly, 0, 0, &cBSParams ) );
        if( uiCostTest >= uiMinCost )
        {
          break;
        }

        uiMinCost     = uiCostTest;
        cMv   [uiDir] = cMvTest;
        uiBits[uiDir] = uiBitsTest - uiBits[1-uiDir];

        if( bQPelOnly )
        {
          if      ( uiBits[uiDir] == 2 )  uiBits[uiDir] = 2;
          else if ( uiBits[uiDir] == 4 )  uiBits[uiDir] = 3;
          else if ( uiBits[uiDir] == 6 )  uiBits[uiDir] = 4;
          else                            ROT(1);
        }
        
        RNOK( m_pcMotionEstimation->compensateBlock       ( &cYuvMbBuffer[uiDir],
                                                            eParIdx, MODE_16x8 ) );
      }
    }


    //===== set parameters =====
    uiMvdBits+= ( bList0 ? uiBits[0] : 0 ) + ( bList1 ? uiBits[1] : 0 );
    cMvd  [0] = cMv[0] - cMvPred[0];
    cMvd  [1] = cMv[1] - cMvPred[1];

    rpcMbTempData->getMbMotionData( LIST_0 ).setRefIdx      ( iRefIdx [0],  eParIdx );
    rpcMbTempData->getMbMotionData( LIST_0 ).setAllMv       ( cMv     [0],  eParIdx );
    rpcMbTempData->getMbMvdData   ( LIST_0 ).setAllMv       ( cMvd    [0],  eParIdx );
    rpcMbTempData->getMbMotionData( LIST_0 ).setMotPredFlag ( true,         eParIdx );
    rpcMbTempData->getMbMotionData( LIST_1 ).setRefIdx      ( iRefIdx [1],  eParIdx );
    rpcMbTempData->getMbMotionData( LIST_1 ).setAllMv       ( cMv     [1],  eParIdx );
    rpcMbTempData->getMbMvdData   ( LIST_1 ).setAllMv       ( cMvd    [1],  eParIdx );
    rpcMbTempData->getMbMotionData( LIST_1 ).setMotPredFlag ( true,         eParIdx );
  }

  IntMbTempData* pcMbRefData = rpcMbTempData;

  RNOK( xSetRdCostInterMb   ( *rpcMbTempData, pcMbDataAccessBase, rcRefFrameList0, rcRefFrameList1, true, 1+uiMvdBits ) );
  RNOK( xCheckBestEstimation(  rpcMbTempData, rpcMbBestData ) );

  RNOK( xCheckInterMbMode8x8(  rpcMbTempData, rpcMbBestData, pcMbRefData, rcRefFrameList0, rcRefFrameList1, pcMbDataAccessBase ) );

  return Err::m_nOK;
}





ErrVal
MbEncoder::xQPelEstimateMb8x16 ( IntMbTempData*&  rpcMbTempData,
                                 IntMbTempData*&  rpcMbBestData,
                                 RefFrameList&    rcRefFrameList0,
                                 RefFrameList&    rcRefFrameList1,
                                 UInt             uiNumMaxIter,
                                 Bool             bQPelOnly,
                                 MbDataAccess*    pcMbDataAccessBase,
                                 Bool             bResidualPred )
{
  ROF( pcMbDataAccessBase );

  rpcMbTempData->clear();
  rpcMbTempData->setMbMode( MODE_8x16 );
  rpcMbTempData->setBLSkipFlag( false );
  rpcMbTempData->setBLQRefFlag( true  );
  rpcMbTempData->setResidualPredFlag( bResidualPred, PART_16x16 );

  UInt  uiMvdBits = 0;

  for( UInt uiBlk = 0; uiBlk < 2; uiBlk++ )
  {
    ParIdx8x16      eParIdx         = ( uiBlk ? PART_8x16_1 : PART_8x16_0 );
    UInt            uiFwdBwd        = pcMbDataAccessBase->getMbData().getFwdBwd( B4x4Idx( eParIdx ) );
    Bool            bList0          = ( ( uiFwdBwd & 0x1111 ) != 0 );
    Bool            bList1          = ( ( uiFwdBwd & 0x2222 ) != 0 );
    Int             iRefIdx     [2] = { BLOCK_NOT_PREDICTED, BLOCK_NOT_PREDICTED };
    Mv              cMv         [2];
    Mv              cMvd        [2];
    Mv              cMvPred     [2];
    UInt            uiBits      [2];
    IntYuvMbBuffer  cYuvMbBuffer[2];
    IntFrame*       pcRefFrame;
    UInt            uiCostTest;

    if( bList0 )
    {
      //===== LIST 0 PREDICTION ======
      iRefIdx [0] = pcMbDataAccessBase->getMbMotionData( LIST_0 ).getRefIdx ( eParIdx );
      cMv     [0] = pcMbDataAccessBase->getMbMotionData( LIST_0 ).getMv     ( eParIdx );
      cMvPred [0] = cMv[0];
      uiBits  [0] = 0;
      pcRefFrame  = rcRefFrameList0[iRefIdx[0]];
      RNOK( m_pcMotionEstimation->estimateBlockWithStart( *rpcMbTempData, *pcRefFrame,
                                                          cMv[0], cMvPred[0], uiBits[0], uiCostTest,
                                                          eParIdx, MODE_8x16, bQPelOnly, 0,
                                                          &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdx[0] ) ) );
      if( bQPelOnly )
      {
        if      ( uiBits[0] == 2 )  uiBits[0] = 2;
        else if ( uiBits[0] == 4 )  uiBits[0] = 3;
        else if ( uiBits[0] == 6 )  uiBits[0] = 4;
        else                        ROT(1);
      }
    }

    if( bList1 )
    {
      //===== LIST 1 PREDICTION ======
      iRefIdx [1] = pcMbDataAccessBase->getMbMotionData( LIST_1 ).getRefIdx ( eParIdx );
      cMv     [1] = pcMbDataAccessBase->getMbMotionData( LIST_1 ).getMv     ( eParIdx );
      cMvPred [1] = cMv[1];
      uiBits  [1] = 0;
      pcRefFrame  = rcRefFrameList1[iRefIdx[1]];
      RNOK( m_pcMotionEstimation->estimateBlockWithStart( *rpcMbTempData, *pcRefFrame,
                                                          cMv[1], cMvPred[1], uiBits[1], uiCostTest,
                                                          eParIdx, MODE_8x16, bQPelOnly, 0,
                                                          &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdx[1] ) ) );
      RNOK( m_pcMotionEstimation->compensateBlock       ( &cYuvMbBuffer[1],
                                                          eParIdx, MODE_8x16 ) );
      if( bQPelOnly )
      {
        if      ( uiBits[1] == 2 )  uiBits[1] = 2;
        else if ( uiBits[1] == 4 )  uiBits[1] = 3;
        else if ( uiBits[1] == 6 )  uiBits[1] = 4;
        else                        ROT(1);
      }
    }

    if( bList0 && bList1 )
    {
      //===== BI PREDICTION =====
      if( uiNumMaxIter == 0 )
      {
        uiNumMaxIter = 1;
      }

      //----- iterative search -----
      for( UInt uiMinCost = MSYS_UINT_MAX, uiIter = 0; uiIter < uiNumMaxIter; uiIter++ )
      {
        UInt          uiBitsTest;
        Mv            cMvTest;
        UInt          uiDir           = uiIter % 2;
        RefFrameList& rcRefFrameList  = ( uiDir ? rcRefFrameList1 : rcRefFrameList0 );
        BSParams      cBSParams;
        cBSParams.pcAltRefFrame       = ( uiDir ? rcRefFrameList0 : rcRefFrameList1 )[ iRefIdx[ 1 - uiDir ] ];
        cBSParams.pcAltRefPelData     = &cYuvMbBuffer[1-uiDir];
        cBSParams.uiL1Search          = uiDir;
        cBSParams.apcWeight[LIST_0]   = &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdx[LIST_0]  );
        cBSParams.apcWeight[LIST_1]   = &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdx[LIST_1]  );

        cMvTest     = cMvPred [uiDir];
        uiBitsTest  = uiBits[1-uiDir];
        pcRefFrame  = rcRefFrameList[iRefIdx[uiDir]];
        RNOK( m_pcMotionEstimation->estimateBlockWithStart( *rpcMbTempData, *pcRefFrame,
                                                            cMvTest, cMvPred[uiDir], uiBitsTest, uiCostTest,
                                                            eParIdx, MODE_8x16, bQPelOnly, 0, 0, &cBSParams ) );
        if( uiCostTest >= uiMinCost )
        {
          break;
        }

        uiMinCost     = uiCostTest;
        cMv   [uiDir] = cMvTest;
        uiBits[uiDir] = uiBitsTest - uiBits[1-uiDir];

        if( bQPelOnly )
        {
          if      ( uiBits[uiDir] == 2 )  uiBits[uiDir] = 2;
          else if ( uiBits[uiDir] == 4 )  uiBits[uiDir] = 3;
          else if ( uiBits[uiDir] == 6 )  uiBits[uiDir] = 4;
          else                            ROT(1);
        }
        
        RNOK( m_pcMotionEstimation->compensateBlock       ( &cYuvMbBuffer[uiDir],
                                                            eParIdx, MODE_8x16 ) );
      }
    }


    //===== set parameters =====
    uiMvdBits+= ( bList0 ? uiBits[0] : 0 ) + ( bList1 ? uiBits[1] : 0 );
    cMvd  [0] = cMv[0] - cMvPred[0];
    cMvd  [1] = cMv[1] - cMvPred[1];

    rpcMbTempData->getMbMotionData( LIST_0 ).setRefIdx      ( iRefIdx [0],  eParIdx );
    rpcMbTempData->getMbMotionData( LIST_0 ).setAllMv       ( cMv     [0],  eParIdx );
    rpcMbTempData->getMbMvdData   ( LIST_0 ).setAllMv       ( cMvd    [0],  eParIdx );
    rpcMbTempData->getMbMotionData( LIST_0 ).setMotPredFlag ( true,         eParIdx );
    rpcMbTempData->getMbMotionData( LIST_1 ).setRefIdx      ( iRefIdx [1],  eParIdx );
    rpcMbTempData->getMbMotionData( LIST_1 ).setAllMv       ( cMv     [1],  eParIdx );
    rpcMbTempData->getMbMvdData   ( LIST_1 ).setAllMv       ( cMvd    [1],  eParIdx );
    rpcMbTempData->getMbMotionData( LIST_1 ).setMotPredFlag ( true,         eParIdx );
  }

  IntMbTempData* pcMbRefData = rpcMbTempData;

  RNOK( xSetRdCostInterMb   ( *rpcMbTempData, pcMbDataAccessBase, rcRefFrameList0, rcRefFrameList1, true, 1+uiMvdBits ) );
  RNOK( xCheckBestEstimation(  rpcMbTempData, rpcMbBestData ) );

  RNOK( xCheckInterMbMode8x8(  rpcMbTempData, rpcMbBestData, pcMbRefData, rcRefFrameList0, rcRefFrameList1, pcMbDataAccessBase ) );

  return Err::m_nOK;
}





ErrVal
MbEncoder::xQPelEstimateMb8x8( IntMbTempData*&  rpcMbTempData,
                               IntMbTempData*&  rpcMbBestData,
                               RefFrameList&    rcRefFrameList0,
                               RefFrameList&    rcRefFrameList1,
                               UInt             uiNumMaxIter,
                               Bool             bQPelOnly,
                               MbDataAccess*    pcMbDataAccessBase,
                               Bool             bResidualPred )
{
  ROF( pcMbDataAccessBase );

  rpcMbTempData->clear();
  rpcMbTempData->setMbMode( MODE_8x8 );
  rpcMbTempData->setBLSkipFlag( false );
  rpcMbTempData->setBLQRefFlag( true  );
  rpcMbTempData->setResidualPredFlag( bResidualPred, PART_16x16 );

  UInt  uiMvdBits = 0;

  for( UInt uiBlk = 0; uiBlk < 4; uiBlk++ )
  {
    ParIdx8x8       eParIdx         = ( uiBlk==3 ? PART_8x8_3 : uiBlk==2 ? PART_8x8_2 : uiBlk==1 ? PART_8x8_1 : PART_8x8_0 );
    BlkMode         eBlkMode        = pcMbDataAccessBase->getMbData().getBlkMode( Par8x8 ( uiBlk   ) );
    UInt            uiFwdBwd        = pcMbDataAccessBase->getMbData().getFwdBwd ( B4x4Idx( eParIdx ) );
    Bool            bList0          = ( ( uiFwdBwd & 0x1111 ) != 0 );
    Bool            bList1          = ( ( uiFwdBwd & 0x2222 ) != 0 );
    Int             iRefIdx     [2] = { BLOCK_NOT_PREDICTED, BLOCK_NOT_PREDICTED };
    Mv              cMv         [2];
    Mv              cMvd        [2];
    Mv              cMvPred     [2];
    UInt            uiBits      [2];
    IntYuvMbBuffer  cYuvMbBuffer[2];
    IntFrame*       pcRefFrame;
    UInt            uiCostTest;

// TMM_ESS {
    if ( eBlkMode != BLK_8x8 )
    {
      // no QPEL reft in case of sub-partitioning of 8x8 blk
      rpcMbTempData->rdCost() = DOUBLE_MAX;
      return Err::m_nOK;
    }
// TMM_ESS }

    ROF( eBlkMode == BLK_8x8 );
    rpcMbTempData->setBlkMode( Par8x8(uiBlk), eBlkMode );

    if( bList0 )
    {
      //===== LIST 0 PREDICTION ======
      iRefIdx [0] = pcMbDataAccessBase->getMbMotionData( LIST_0 ).getRefIdx ( eParIdx );
      cMv     [0] = pcMbDataAccessBase->getMbMotionData( LIST_0 ).getMv     ( eParIdx );
      cMvPred [0] = cMv[0];
      uiBits  [0] = 0;
      pcRefFrame  = rcRefFrameList0[iRefIdx[0]];
      RNOK( m_pcMotionEstimation->estimateBlockWithStart( *rpcMbTempData, *pcRefFrame,
                                                          cMv[0], cMvPred[0], uiBits[0], uiCostTest,
                                                          eParIdx, BLK_8x8, bQPelOnly, 0,
                                                          &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdx[0] ) ) );
      if( bQPelOnly )
      {
        if      ( uiBits[0] == 2 )  uiBits[0] = 2;
        else if ( uiBits[0] == 4 )  uiBits[0] = 3;
        else if ( uiBits[0] == 6 )  uiBits[0] = 4;
        else                        ROT(1);
      }
    }

    if( bList1 )
    {
      //===== LIST 1 PREDICTION ======
      iRefIdx [1] = pcMbDataAccessBase->getMbMotionData( LIST_1 ).getRefIdx ( eParIdx );
      cMv     [1] = pcMbDataAccessBase->getMbMotionData( LIST_1 ).getMv     ( eParIdx );
      cMvPred [1] = cMv[1];
      uiBits  [1] = 0;
      pcRefFrame  = rcRefFrameList1[iRefIdx[1]];
      RNOK( m_pcMotionEstimation->estimateBlockWithStart( *rpcMbTempData, *pcRefFrame,
                                                          cMv[1], cMvPred[1], uiBits[1], uiCostTest,
                                                          eParIdx, BLK_8x8, bQPelOnly, 0,
                                                          &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdx[1] ) ) );
      RNOK( m_pcMotionEstimation->compensateBlock       ( &cYuvMbBuffer[1],
                                                          eParIdx, BLK_8x8 ) );
      if( bQPelOnly )
      {
        if      ( uiBits[1] == 2 )  uiBits[1] = 2;
        else if ( uiBits[1] == 4 )  uiBits[1] = 3;
        else if ( uiBits[1] == 6 )  uiBits[1] = 4;
        else                        ROT(1);
      }
    }

    if( bList0 && bList1 )
    {
      //===== BI PREDICTION =====
      if( uiNumMaxIter == 0 )
      {
        uiNumMaxIter = 1;
      }

      //----- iterative search -----
      for( UInt uiMinCost = MSYS_UINT_MAX, uiIter = 0; uiIter < uiNumMaxIter; uiIter++ )
      {
        UInt          uiBitsTest;
        Mv            cMvTest;
        UInt          uiDir           = uiIter % 2;
        RefFrameList& rcRefFrameList  = ( uiDir ? rcRefFrameList1 : rcRefFrameList0 );
        BSParams      cBSParams;
        cBSParams.pcAltRefFrame       = ( uiDir ? rcRefFrameList0 : rcRefFrameList1 )[ iRefIdx[ 1 - uiDir ] ];
        cBSParams.pcAltRefPelData     = &cYuvMbBuffer[1-uiDir];
        cBSParams.uiL1Search          = uiDir;
        cBSParams.apcWeight[LIST_0]   = &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_0, iRefIdx[LIST_0]  );
        cBSParams.apcWeight[LIST_1]   = &rpcMbTempData->getMbDataAccess().getSH().getPredWeight( LIST_1, iRefIdx[LIST_1]  );

        cMvTest     = cMvPred [uiDir];
        uiBitsTest  = uiBits[1-uiDir];
        pcRefFrame  = rcRefFrameList[iRefIdx[uiDir]];
        RNOK( m_pcMotionEstimation->estimateBlockWithStart( *rpcMbTempData, *pcRefFrame,
                                                            cMvTest, cMvPred[uiDir], uiBitsTest, uiCostTest,
                                                            eParIdx, BLK_8x8, bQPelOnly, 0, 0, &cBSParams ) );
        if( uiCostTest >= uiMinCost )
        {
          break;
        }

        uiMinCost     = uiCostTest;
        cMv   [uiDir] = cMvTest;
        uiBits[uiDir] = uiBitsTest - uiBits[1-uiDir];

        if( bQPelOnly )
        {
          if      ( uiBits[uiDir] == 2 )  uiBits[uiDir] = 2;
          else if ( uiBits[uiDir] == 4 )  uiBits[uiDir] = 3;
          else if ( uiBits[uiDir] == 6 )  uiBits[uiDir] = 4;
          else                            ROT(1);
        }
        
        RNOK( m_pcMotionEstimation->compensateBlock       ( &cYuvMbBuffer[uiDir],
                                                            eParIdx, BLK_8x8 ) );
      }
    }


    //===== set parameters =====
    uiMvdBits+= ( bList0 ? uiBits[0] : 0 ) + ( bList1 ? uiBits[1] : 0 );
    cMvd  [0] = cMv[0] - cMvPred[0];
    cMvd  [1] = cMv[1] - cMvPred[1];

    rpcMbTempData->getMbMotionData( LIST_0 ).setRefIdx      ( iRefIdx [0],  eParIdx );
    rpcMbTempData->getMbMotionData( LIST_0 ).setAllMv       ( cMv     [0],  eParIdx );
    rpcMbTempData->getMbMvdData   ( LIST_0 ).setAllMv       ( cMvd    [0],  eParIdx );
    rpcMbTempData->getMbMotionData( LIST_0 ).setMotPredFlag ( true,         eParIdx );
    rpcMbTempData->getMbMotionData( LIST_1 ).setRefIdx      ( iRefIdx [1],  eParIdx );
    rpcMbTempData->getMbMotionData( LIST_1 ).setAllMv       ( cMv     [1],  eParIdx );
    rpcMbTempData->getMbMvdData   ( LIST_1 ).setAllMv       ( cMvd    [1],  eParIdx );
    rpcMbTempData->getMbMotionData( LIST_1 ).setMotPredFlag ( true,         eParIdx );
  }

  IntMbTempData* pcMbRefData = rpcMbTempData;

  RNOK( xSetRdCostInterMb   ( *rpcMbTempData, pcMbDataAccessBase, rcRefFrameList0, rcRefFrameList1, true, 1+uiMvdBits ) );
  RNOK( xCheckBestEstimation(  rpcMbTempData, rpcMbBestData ) );

  RNOK( xCheckInterMbMode8x8(  rpcMbTempData, rpcMbBestData, pcMbRefData, rcRefFrameList0, rcRefFrameList1, pcMbDataAccessBase ) );

  return Err::m_nOK;
}



H264AVC_NAMESPACE_END
