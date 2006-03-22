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
#include "SliceEncoder.h"
#include "MbCoder.h"
#include "CodingParameter.h"
#include "RecPicBuffer.h"
#include "H264AVCCommonLib/PocCalculator.h"
#include "H264AVCCommonLib/FrameMng.h"
#include "H264AVCCommonLib/Transform.h"

#include "H264AVCCommonLib/CFMO.h"

H264AVC_NAMESPACE_BEGIN


SliceEncoder::SliceEncoder():
  m_pcMbEncoder       ( NULL ),
  m_pcMbCoder         ( NULL ),
  m_pcControlMng      ( NULL ),
  m_pcCodingParameter ( NULL ),
  m_pcPocCalculator   ( NULL ),
  m_bInitDone         ( false ),
  m_uiFrameCount(0),
  m_eSliceType        ( I_SLICE ),
  m_bTraceEnable      ( true ),
  m_pcTransform       ( NULL )
{
}


SliceEncoder::~SliceEncoder()
{
}

ErrVal SliceEncoder::create( SliceEncoder*& rpcSliceEncoder )
{
  rpcSliceEncoder = new SliceEncoder;

  ROT( NULL == rpcSliceEncoder );

  return Err::m_nOK;
}


ErrVal SliceEncoder::destroy()
{
  delete this;
  return Err::m_nOK;
}



ErrVal SliceEncoder::init( MbEncoder* pcMbEncoder,
                           MbCoder* pcMbCoder,
                           ControlMngIf* pcControlMng,
                           CodingParameter* pcCodingParameter,
                           PocCalculator* pcPocCalculator,
                           Transform* pcTransform)
{
  ROT( m_bInitDone );
  ROT( NULL == pcMbEncoder );
  ROT( NULL == pcMbCoder );
  ROT( NULL == pcControlMng );
  ROT( NULL == pcPocCalculator );
  ROT( NULL == pcTransform );

  m_pcTransform = pcTransform;
  m_pcMbEncoder = pcMbEncoder;
  m_pcMbCoder = pcMbCoder;
  m_pcControlMng = pcControlMng;
  m_pcCodingParameter = pcCodingParameter;
  m_pcPocCalculator = pcPocCalculator;


  m_uiFrameCount = 0;
  m_eSliceType =  I_SLICE;
  m_bTraceEnable = true;

  m_bInitDone = true;
  return Err::m_nOK;
}


ErrVal SliceEncoder::uninit()
{
  ROF( m_bInitDone );
  m_pcMbEncoder =  NULL;
  m_pcMbCoder =  NULL;
  m_pcControlMng =  NULL;
  m_bInitDone = false;

  m_uiFrameCount = 0;
  m_eSliceType =  I_SLICE;
  m_bTraceEnable = false;
  return Err::m_nOK;
}






ErrVal
SliceEncoder::encodeInterPictureP( UInt&            ruiBits,
                                   IntFrame*        pcFrame,
                                   IntFrame*        pcRecSubband,
                                   IntFrame*        pcPredSignal,
                                   ControlData&     rcControlData,
                                   UInt             uiMbInRow,
                                   RefFrameList&    rcRefFrameList,
                                   RefFrameList&    rcRefFrameListBase )
{
  ROF( m_bInitDone );

  SliceHeader&  rcSliceHeader         = *rcControlData.getSliceHeader           ();
  MbDataCtrl*   pcMbDataCtrl          =  rcControlData.getMbDataCtrl            ();
  IntFrame*     pcBaseLayerFrame      =  rcControlData.getBaseLayerRec          ();
  IntFrame*     pcBaseLayerResidual   =  rcControlData.getBaseLayerSbb          ();
  MbDataCtrl*   pcBaseLayerCtrl       =  rcControlData.getBaseLayerCtrl         ();
  Double        dLambda               =  rcControlData.getLambda                ();
  Int           iSpatialScalabilityType =  rcControlData.getSpatialScalabilityType(); // TMM_ESS
  UInt          uiMbAddress           =  rcSliceHeader.getFirstMbInSlice        ();
  UInt          uiLastMbAddress       =  rcSliceHeader.getLastMbInSlice         ();
  UInt          uiBits                =  m_pcMbCoder ->getBitCount              ();
    

  //====== initialization ======
  RNOK( pcMbDataCtrl      ->initSlice         ( rcSliceHeader, ENCODE_PROCESS, false, NULL ) );
  if( pcBaseLayerCtrl )
  {
    RNOK( pcBaseLayerCtrl ->initSlice         ( rcSliceHeader, PRE_PROCESS,    false, NULL ) );
  }
  RNOK( m_pcControlMng    ->initSliceForCoding( rcSliceHeader ) );


  //===== loop over macroblocks =====
  for( ; uiMbAddress <= uiLastMbAddress; ) //--ICU/ETRI FMO Implementation
  {
    ETRACE_NEWMB( uiMbAddress );

    UInt          uiMbY               = uiMbAddress / uiMbInRow;
    UInt          uiMbX               = uiMbAddress % uiMbInRow;
    MbDataAccess* pcMbDataAccess      = 0;
    MbDataAccess* pcMbDataAccessBase  = 0;

    RNOK( pcMbDataCtrl      ->initMb          ( pcMbDataAccess,     uiMbY, uiMbX ) );
    if( pcBaseLayerCtrl )
    {
      RNOK( pcBaseLayerCtrl ->initMb          ( pcMbDataAccessBase, uiMbY, uiMbX ) );
    }
    RNOK( m_pcControlMng    ->initMbForCoding ( *pcMbDataAccess,    uiMbAddress  ) );
    pcMbDataAccess->getMbData().deactivateMotionRefinement();

    if( rcRefFrameListBase.getSize() )
    {
      RNOK( m_pcMbEncoder     ->encodeInterP    ( *pcMbDataAccess,
                                                  pcMbDataAccessBase,
                                                  iSpatialScalabilityType,
                                                  pcFrame,
                                                  pcRecSubband,
                                                  pcPredSignal,
                                                  pcBaseLayerFrame,
                                                  pcBaseLayerResidual,
                                                  rcRefFrameList,
                                                  & rcRefFrameListBase,
                                                  dLambda ) );
    }
    else
    {
      RNOK( m_pcMbEncoder     ->encodeInterP    ( *pcMbDataAccess,
                                                  pcMbDataAccessBase,
                                                  iSpatialScalabilityType,
                                                  pcFrame,
                                                  pcRecSubband,
                                                  pcPredSignal,
                                                  pcBaseLayerFrame,
                                                  pcBaseLayerResidual,
                                                  rcRefFrameList,
                                                  0,
                                                  dLambda ) );
    }
    RNOK( m_pcMbCoder       ->encode         ( *pcMbDataAccess,
                                                pcMbDataAccessBase,
                                                iSpatialScalabilityType,
                                                (uiMbAddress == uiLastMbAddress) ) );

    if( pcMbDataAccess->getMbData().getResidualPredFlag( PART_16x16 ) )
    {
      pcMbDataAccess->getMbData().setMbExtCbp( pcMbDataAccess->getMbData().getMbExtCbp() | pcMbDataAccessBase->getMbData().getMbExtCbp() );
    }

    uiMbAddress = rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress );
  }
  
  ruiBits += m_pcMbCoder->getBitCount() - uiBits;

  return Err::m_nOK;
}




ErrVal SliceEncoder::encodeIntraPicture( UInt&        ruiBits,
                                         ControlData& rcControlData, 
                                         IntFrame*    pcFrame, 
                                         IntFrame*    pcRecSubband,
                                         IntFrame*    pcBaseLayer,
                                         IntFrame*    pcPredSignal,
                                         UInt         uiMbInRow, 
                                         Double       dLambda )
{
  ROF( m_bInitDone );

  SliceHeader&  rcSliceHeader     = *rcControlData.getSliceHeader           ();
  MbDataCtrl*   pcMbDataCtrl      =  rcControlData.getMbDataCtrl            ();
  MbDataCtrl*   pcBaseLayerCtrl   =  rcControlData.getBaseLayerCtrl         ();
  Int           iSpatialScalabilityType =  rcControlData.getSpatialScalabilityType(); // TMM_ESS

  RNOK( pcMbDataCtrl      ->initSlice         ( rcSliceHeader, ENCODE_PROCESS, false, NULL ) );
  if( pcBaseLayerCtrl )
  {
    RNOK( pcBaseLayerCtrl ->initSlice         ( rcSliceHeader, PRE_PROCESS,    false, NULL ) );
  }
  RNOK( m_pcControlMng    ->initSliceForCoding( rcSliceHeader ) );

  //====== initialization ======
  UInt          uiMbAddress         = rcSliceHeader.getFirstMbInSlice ();
  UInt          uiLastMbAddress     = rcSliceHeader.getLastMbInSlice  ();
  UInt          uiBits              = m_pcMbCoder ->getBitCount       ();

  if(uiMbAddress == -1) return Err::m_nOK;

  //===== loop over macroblocks =====
  for(  ; uiMbAddress <= uiLastMbAddress;  ) //--ICU/ETRI FMO Implementation
  {
    ETRACE_NEWMB( uiMbAddress );

    UInt          uiMbY                 = uiMbAddress / uiMbInRow;
    UInt          uiMbX                 = uiMbAddress % uiMbInRow;
    MbDataAccess* pcMbDataAccess        = 0;
    MbDataAccess* pcMbDataAccessBase    = 0;

    RNOK( pcMbDataCtrl      ->initMb          ( pcMbDataAccess,     uiMbY, uiMbX ) );
    if( pcBaseLayerCtrl )
    {
      RNOK( pcBaseLayerCtrl ->initMb          ( pcMbDataAccessBase, uiMbY, uiMbX ) );
    }
    RNOK( m_pcControlMng    ->initMbForCoding ( *pcMbDataAccess,    uiMbAddress  ) );
    pcMbDataAccess->getMbData().deactivateMotionRefinement();

    RNOK( m_pcMbEncoder     ->encodeIntra     ( *pcMbDataAccess,
                                                pcMbDataAccessBase,
                                                pcFrame,
                                                pcRecSubband,
                                                pcBaseLayer,
                                                pcPredSignal,
                                                dLambda ) );
    RNOK( m_pcMbCoder       ->encode          ( *pcMbDataAccess,
                                                pcMbDataAccessBase,
                                                iSpatialScalabilityType,
                                                ( uiMbAddress == uiLastMbAddress ) ) );

    uiMbAddress = rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress );
  }
  
  ruiBits += m_pcMbCoder->getBitCount() - uiBits;

  return Err::m_nOK;
}





ErrVal SliceEncoder::encodeHighPassPicture( UInt&         ruiMbCoded,
                                            UInt&         ruiBits,
                                            UInt&         ruiBitsRes,
                                            SliceHeader&  rcSH,
                                            IntFrame*     pcFrame,
                                            IntFrame*     pcResidual,
                                            IntFrame*     pcPredSignal,
																						IntFrame*			pcSRFrame, // JVT-R091
                                            IntFrame*     pcBaseSubband,
                                            IntFrame*     pcBaseLayer,
                                            MbDataCtrl*   pcMbDataCtrl,
                                            MbDataCtrl*   pcMbDataCtrlBaseMotion,
                                            UInt          uiMbInRow,
                                            Double        dLambda,
                                            Int           iMaxDeltaQp,
                                            Int           iSpatialScalabilityType)
{
  ROF( m_bInitDone );

  RNOK( pcMbDataCtrl  ->initSlice         ( rcSH, PRE_PROCESS, false, NULL ) );
  RNOK( m_pcControlMng->initSliceForCoding( rcSH              ) );

  //====== initialization ======
  UInt            uiMbAddress       = rcSH.getFirstMbInSlice();
  UInt            uiLastMbAddress   = rcSH.getLastMbInSlice();
  UInt            uiBits            = m_pcMbCoder->getBitCount();
  Int             iQPRes            = rcSH.getPicQp();
  Int             iQPIntra          = rcSH.getPicQp(); //- 2;

  IntYuvMbBuffer  cZeroBuffer;
  cZeroBuffer.setAllSamplesToZero();

  //===== loop over macroblocks =====
  for(  ruiMbCoded = 0; uiMbAddress <= uiLastMbAddress;  ) //--ICU/ETRI FMO Implementation
  {
    ETRACE_NEWMB( uiMbAddress );

    UInt          uiMbY                 = uiMbAddress / uiMbInRow;
    UInt          uiMbX                 = uiMbAddress % uiMbInRow;
    MbDataAccess* pcMbDataAccess        = 0;
    Bool          bCoded;


    RNOK( pcMbDataCtrl    ->initMb          (  pcMbDataAccess,    uiMbY, uiMbX ) );
    RNOK( m_pcControlMng  ->initMbForCoding ( *pcMbDataAccess,    uiMbAddress  ) );
    pcMbDataAccess->getMbData().deactivateMotionRefinement();

    MbDataAccess* pcMbDataAccessBase  = 0;
    Bool          bWriteMotion        = rcSH.getBaseLayerId() == MSYS_UINT_MAX || rcSH.getAdaptivePredictionFlag();

    if( pcMbDataCtrlBaseMotion)
    {
      RNOK( pcMbDataCtrlBaseMotion->initMb  ( pcMbDataAccessBase, uiMbY, uiMbX ) );
    }

    if( pcMbDataAccess->getMbData().isIntra() )
    {
      pcMbDataAccess->getMbData().setQp( iQPIntra );

      RNOK( m_pcMbEncoder ->encodeIntra   ( *pcMbDataAccess, pcMbDataAccessBase, pcFrame, pcResidual, pcBaseLayer, pcPredSignal, dLambda ) );
      RNOK( m_pcMbCoder   ->encode        ( *pcMbDataAccess, pcMbDataAccessBase, iSpatialScalabilityType, (uiMbAddress == uiLastMbAddress ) ) );

			//-- JVT-R091
			// update with best data (intra)
			pcSRFrame->getFullPelYuvBuffer()->loadBuffer( m_pcMbEncoder->getBestIntData() );	
			//--

      ruiMbCoded++;
    }
    else
    {
      pcMbDataAccess->getMbData().setQp( iQPRes );

      m_pcTransform->setClipMode( false );
      RNOK( m_pcMbEncoder ->encodeResidual  ( *pcMbDataAccess, pcMbDataAccessBase, pcFrame, pcResidual, pcBaseSubband,
																							 pcSRFrame, // JVT-R091
																							 bCoded, dLambda, iMaxDeltaQp ) );

      if( pcMbDataAccess->getSH().getBaseLayerId() != MSYS_UINT_MAX && ! pcMbDataAccess->getSH().getAdaptivePredictionFlag() )
      {
        ROF( pcMbDataAccess->getMbData().getResidualPredFlag( PART_16x16 ) );
        pcMbDataAccess->getMbData().setBLSkipFlag( true );
      }

      m_pcTransform->setClipMode( true );

      RNOK( m_pcMbCoder->encode( *pcMbDataAccess, pcMbDataAccessBase, iSpatialScalabilityType, (uiMbAddress == uiLastMbAddress ) ) );

      if( bCoded )
      {
        ruiMbCoded++;
      }


      if( pcMbDataAccess->getMbData().getResidualPredFlag( PART_16x16 ) )
      {
        pcMbDataAccess->getMbData().setMbExtCbp( pcMbDataAccess->getMbData().getMbExtCbp() | pcMbDataAccessBase->getMbData().getMbExtCbp() );
      }

			//-- JVT-R091
			// update with best-data (inter)
			IntYuvMbBuffer	cPredBuffer, cResBuffer;
			cPredBuffer.loadBuffer	( ((IntFrame*)pcSRFrame		)->getFullPelYuvBuffer() );
			cResBuffer.	loadBuffer	( ((IntFrame*)pcResidual  )->getFullPelYuvBuffer() );
			cPredBuffer.add					( cResBuffer );	
			cPredBuffer.clip				();
			pcSRFrame->getFullPelYuvBuffer()->loadBuffer( &cPredBuffer );
			//--

      RNOK( pcPredSignal->getFullPelYuvBuffer()->loadBuffer( &cZeroBuffer ) );
    }

    uiMbAddress = rcSH.getFMO()->getNextMBNr(uiMbAddress);
  }

  ruiBits += m_pcMbCoder->getBitCount() - uiBits;

  return Err::m_nOK;
}


ErrVal
SliceEncoder::encodeSlice( SliceHeader&  rcSliceHeader,
                           IntFrame*     pcFrame,
                           MbDataCtrl*   pcMbDataCtrl,
                           RefFrameList& rcList0,
                           RefFrameList& rcList1,
                           UInt          uiMbInRow,
                           Double        dlambda )
{
  ROF( pcFrame );
  ROF( pcMbDataCtrl );

  //===== get co-located picture =====
  MbDataCtrl* pcMbDataCtrlL1 = NULL;
  if( rcList1.getActive() && rcList1.getEntry( 0 )->getRecPicBufUnit() )
  {
    pcMbDataCtrlL1 = rcList1.getEntry( 0 )->getRecPicBufUnit()->getMbDataCtrl();
  }
  ROT( rcSliceHeader.isInterB() && ! pcMbDataCtrlL1 );

  //===== initialization =====
  RNOK( pcMbDataCtrl  ->initSlice         ( rcSliceHeader, ENCODE_PROCESS, false, pcMbDataCtrlL1 ) );
  RNOK( m_pcControlMng->initSliceForCoding( rcSliceHeader ) );

  //===== loop over macroblocks =====
  for( UInt uiMbAddress = rcSliceHeader.getFirstMbInSlice(); uiMbAddress <= rcSliceHeader.getLastMbInSlice(); uiMbAddress = rcSliceHeader.getFMO()->getNextMBNr( uiMbAddress ) )
  {
    ETRACE_NEWMB( uiMbAddress );

    UInt          uiMbY           = uiMbAddress / uiMbInRow;
    UInt          uiMbX           = uiMbAddress % uiMbInRow;
    MbDataAccess* pcMbDataAccess  = 0;

    RNOK( pcMbDataCtrl  ->initMb          (  pcMbDataAccess, uiMbY, uiMbX ) );
    RNOK( m_pcControlMng->initMbForCoding ( *pcMbDataAccess, uiMbAddress  ) );

    RNOK( m_pcMbEncoder ->encodeMacroblock( *pcMbDataAccess,
                                             pcFrame,
                                             rcList0,
                                             rcList1,
                                             m_pcCodingParameter->getMotionVectorSearchParams().getNumMaxIter(),
                                             m_pcCodingParameter->getMotionVectorSearchParams().getIterSearchRange(),
                                             dlambda ) );
    RNOK( m_pcMbCoder   ->encode          ( *pcMbDataAccess, NULL, SST_RATIO_1,
                                             ( uiMbAddress == rcSliceHeader.getLastMbInSlice() ) ) );
  }

  return Err::m_nOK;
}

//TMM_WP
ErrVal SliceEncoder::xInitDefaultWeights(Double *pdWeights, UInt uiLumaWeightDenom, 
                                         UInt uiChromaWeightDenom)
{
    const Int iLumaWeight = 1 << uiLumaWeightDenom;
    const Int iChromaWeight = 1 << uiChromaWeightDenom;
    
    pdWeights[0] = iLumaWeight;
    pdWeights[1] = pdWeights[2] = iChromaWeight;

    return Err::m_nOK;
}


ErrVal SliceEncoder::xSetPredWeights( SliceHeader& rcSH, 
                                      IntFrame* pOrgFrame,
                                      RefFrameList& rcRefFrameList0,
                                      RefFrameList& rcRefFrameList1)
    
{
  RNOK( rcSH.getPredWeightTable(LIST_0).uninit() );
  RNOK( rcSH.getPredWeightTable(LIST_1).uninit() );
  RNOK( rcSH.getPredWeightTable(LIST_0).init( rcSH.getNumRefIdxActive( LIST_0) ) );
  RNOK( rcSH.getPredWeightTable(LIST_1).init( rcSH.getNumRefIdxActive( LIST_1) ) );

  ROTRS( rcSH.isIntra(), Err::m_nOK );

  const SampleWeightingParams& rcSWP = m_pcCodingParameter->getSampleWeightingParams(rcSH.getLayerId());

  { // determine denoms
    const UInt uiLumaDenom = rcSWP.getLumaDenom();
    rcSH.setLumaLog2WeightDenom  ( ( uiLumaDenom == MSYS_UINT_MAX ) ? gIntRandom(0,7) : uiLumaDenom );

    const UInt uiChromaDenom = rcSWP.getChromaDenom();
    rcSH.setChromaLog2WeightDenom( ( uiChromaDenom == MSYS_UINT_MAX ) ? gIntRandom(0,7) : uiChromaDenom );
  }

  const Int iChromaScale = 1<<rcSH.getChromaLog2WeightDenom();
  const Int iLumaScale   = 1<<rcSH.getLumaLog2WeightDenom();

   m_pcControlMng->initSliceForWeighting(rcSH);

  if( rcSH.isInterB() )
  {
      ROTRS( 1 != rcSH.getPPS().getWeightedBiPredIdc(), Err::m_nOK );
  }
  else
  {
    ROTRS( ! rcSH.getPPS().getWeightedPredFlag(), Err::m_nOK );
  }

  if( rcSH.isInterB() )
  {
      RNOK( rcSH.getPredWeightTable(LIST_1).initDefaults( rcSH.getLumaLog2WeightDenom(), rcSH.getChromaLog2WeightDenom() ) );
  }
  RNOK( rcSH.getPredWeightTable(LIST_0).initDefaults( rcSH.getLumaLog2WeightDenom(), rcSH.getChromaLog2WeightDenom() ) );

  Double afFwWeight[MAX_REF_FRAMES][3];
  Double afBwWeight[MAX_REF_FRAMES][3];

  Double afFwOffsets[MAX_REF_FRAMES][3];
  Double afBwOffsets[MAX_REF_FRAMES][3];

  Double fDiscardThr = m_pcCodingParameter->getSampleWeightingParams(rcSH.getLayerId()).getDiscardThr();

  /* init arrays with default weights */
  for (UInt x = 0; x < MAX_REF_FRAMES; x++)
  {
      xInitDefaultWeights(afFwWeight[x], rcSH.getLumaLog2WeightDenom(), rcSH.getChromaLog2WeightDenom());
      xInitDefaultWeights(afBwWeight[x], rcSH.getLumaLog2WeightDenom(), rcSH.getChromaLog2WeightDenom());

      afFwOffsets[x][0] = afFwOffsets[x][1] = afFwOffsets[x][2] = 0;
      afBwOffsets[x][0] = afBwOffsets[x][1] = afBwOffsets[x][2] = 0;
  }
  
  if( rcSH.isInterB() )
  {
      RNOK( m_pcMbEncoder->getPredWeights( rcSH, LIST_1, afBwWeight, 
                                           pOrgFrame, rcRefFrameList1 ) );      
      RNOK( rcSH.getPredWeightTable( LIST_1).setPredWeightsAndFlags( iLumaScale, iChromaScale, 
                                                                     afBwWeight, fDiscardThr ) );
  }

  RNOK( m_pcMbEncoder->getPredWeights( rcSH, LIST_0, afFwWeight, pOrgFrame, rcRefFrameList0 ) );
  RNOK( rcSH.getPredWeightTable( LIST_0).setPredWeightsAndFlags( iLumaScale, iChromaScale, 
                                                                 afFwWeight, fDiscardThr ) );  

  return Err::m_nOK;
}
//TMM_WP


H264AVC_NAMESPACE_END
