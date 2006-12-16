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
#include "GOPEncoder.h"

#include "SliceEncoder.h"
#include "CreaterH264AVCEncoder.h"
#include "ControlMngH264AVCEncoder.h"
#include "RateDistortionIf.h"
#include "H264AVCCommonLib/ParameterSetMng.h"
#include "H264AVCCommonLib/FrameMng.h"
#include "H264AVCCommonLib/Sei.h"
#include "FGSSubbandEncoder.h"


#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "H264AVCCommonLib/CFMO.h"

//JVT-U106 Behaviour at slice boundaries{
#include "H264AVCCommonLib/ReconstructionBypass.h"
//JVT-U106 Behaviour at slice boundaries}

H264AVC_NAMESPACE_BEGIN



#define FACTOR_22_HP  0.70710678118654752440084436210485  //sqrt( 1.0/ 2.0)
#define FACTOR_22_LP  1.4142135623730950488016887242097   //sqrt( 2.0/ 1.0)
// RWTH Aachen, m11728 (heiko.schwarz@hhi.fhg.de on behalf of Mathias Wien [mathias.wien@rwth-aachen.de])
//#define FACTOR_53_HP  0.81649658092772603273242802490196  //sqrt( 2.0/ 3.0)
//#define FACTOR_53_LP  1.1795356492391770676634011002828   //sqrt(32.0/23.0)
#define FACTOR_53_HP  0.84779124789065851738306954082825  //sqrt(23.0/32.0)
#define FACTOR_53_LP  1.2247448713915890490986420373529   //sqrt( 3.0/ 2.0)

//{{Scaling factor Base Layer-France Telecom R&D (nathalie.cammas@francetelecom.com), JVTO045 and m11876
#define FACTOR_53_HP_BL 1
#define FACTOR_22_HP_BL 1
//}}Scaling factor Base Layer

__inline Void printSpaces( UInt uiNum )
{
  while( uiNum-- ) printf(" ");
}



MCTFEncoder::MCTFEncoder()
//----- references -----
: m_pcSPS                           ( 0 )
, m_pcPPSLP                         ( 0 )
, m_pcPPSHP                         ( 0 )
, m_pcSPS_FGS                           ( 0 )
, m_pcPPSLP_FGS                         ( 0 )
, m_pcPPSHP_FGS                         ( 0 )
, m_pcYuvFullPelBufferCtrl          ( 0 )
, m_pcYuvHalfPelBufferCtrl          ( 0 )
, m_pcPocCalculator                 ( 0 )
, m_pcH264AVCEncoder                ( 0 )
, m_pcSliceEncoder                  ( 0 )
, m_pcNalUnitEncoder                ( 0 )
, m_pcLoopFilter                    ( 0 )
, m_pcQuarterPelFilter              ( 0 )
, m_pcMotionEstimation              ( 0 )
, m_pcRQFGSEncoder                  ( 0 )
//----- fixed control parameters -----
, m_bTraceEnable                    ( true )
, m_uiLayerId                       ( 0 )
, m_uiScalableLayerId                ( 0 )
, m_uiBaseLayerId                   ( MSYS_UINT_MAX )
, m_uiBaseQualityLevel              ( 3 )
, m_uiQualityLevelForPrediction     ( 3 )
, m_uiFrameWidthInMb                ( 0 )
, m_uiFrameHeightInMb               ( 0 )
, m_uiMbNumber                      ( 0 )
, m_uiMaxGOPSize                    ( 0 )
, m_uiDecompositionStages           ( 0 )
, m_uiTemporalResolution            ( 0 )
, m_uiNotCodedMCTFStages            ( 0 )
, m_uiFrameDelay                    ( 0 )
, m_uiMaxNumRefFrames               ( 0 )
, m_uiLowPassIntraPeriod            ( 0 )
, m_uiNumMaxIter                    ( 0 )
, m_uiIterSearchRange               ( 0 )
, m_iMaxDeltaQp                     ( 0 )
, m_uiClosedLoopMode                ( 1 )
, m_bH264AVCCompatible              ( true  )
, m_bInterLayerPrediction           ( true  )
, m_bAdaptivePrediction             ( true  )
, m_bHaarFiltering                  ( false )
, m_bBiPredOnly                     ( false )
, m_bWriteSubSequenceSei            ( false )
, m_dBaseQPResidual                 ( 0.0 )
, m_dNumFGSLayers                   ( 0.0 )
, m_uiFilterIdc                     ( 0 )
, m_iAlphaOffset                    ( 0 )
, m_iBetaOffset                     ( 0 )
, m_bLoadMotionInfo                 ( false )
, m_bSaveMotionInfo                 ( false )
, m_pMotionInfoFile                 ( 0 )
//----- variable control parameters -----
, m_bInitDone                       ( false )
, m_bFirstGOPCoded                  ( false )
, m_uiGOPSize                       ( 0 )
, m_uiFrameCounter                  ( 0 )
, m_uiFrameNum                      ( 0 )
, m_uiGOPNumber                     ( 0 )
//----- frame memories -----
, m_papcFrame                       ( 0 )
, m_papcOrgFrame                    ( 0 )
, m_papcBQFrame                     ( 0 )
, m_papcCLRecFrame                  ( 0 )
, m_papcResidual                    ( 0 )
, m_papcSubband                     ( 0 )
, m_pcLowPassBaseReconstruction     ( 0 )
//TMM_WP
, m_bBaseLayerWp                    (false)
//TMM_WP
, m_pcAnchorFrameOriginal           ( 0 )
, m_pcAnchorFrameReconstructed      ( 0 )
, m_pcBaseLayerFrame                ( 0 )
, m_pcBaseLayerResidual             ( 0 )
, m_papcSmoothedFrame                ( 0 ) // JVT-R091
//----- control data arrays -----
, m_pacControlData                  ( 0 )
, m_pcBaseLayerCtrl                 ( 0 )
, m_pcBaseLayerCtrlEL        ( 0 )
, m_pacControlDataEL        ( 0 )
//----- auxiliary buffers -----
, m_uiWriteBufferSize               ( 0 )
, m_pucWriteBuffer                  ( 0 )
//----- PSNR & rate -----
, m_fOutputFrameRate                ( 0.0 )
, m_uiParameterSetBits              ( 0 )
//--- FGS ---
, m_uiFGSMode                       ( 0 )
, m_pFGSFile                        ( 0 )
, m_dFGSBitRateFactor               ( 0.0 )
, m_dFGSRoundingOffset              ( 0.0 )
, m_iLastFGSError                   ( 0 )
, m_uiNotYetConsideredBaseLayerBits ( 0 )
, m_pcResizeParameters              ( 0 )//TMM_ESS
, m_bUseDiscardableUnit             ( false )//JVT-P031
, m_dPredFGSBitRateFactor               ( 0.0 )//JVT-P031
, m_iPredLastFGSError                   ( 0 )//JVT-P031
// JVT-Q065 EIDR{
, m_iIDRPeriod            ( 0 )
, m_bBLSkipEnable          ( false )
// JVT-Q065 EIDR}
, m_bLARDOEnable                    ( false ) //JVT-R057 LA-RDO
, m_uiNonRequiredWrite        ( 0 )  //NonRequired JVT-Q066 (06-04-08)
, m_uiSuffixUnitEnable        ( 0 ) //JVT-S036 lsj
, m_uiMMCOBaseEnable            ( 0 ) //JVT-S036 lsj
// JVT-S054 (ADD) ->
, m_bIroiSliceDivisionFlag ( false )
, m_uiNumSliceMinus1       ( 0 )
, m_puiFirstMbInSlice      ( 0 )
, m_puiLastMbInSlice       ( 0 )
// JVT-S054 (ADD) <-
//S051{
, m_uiTotalFrame          ( 0 )
, m_auiFrameBits          ( NULL )
, m_uiAnaSIP            ( 0 )
, m_bEncSIP              ( false )
, m_cInSIPFileName          ( "none" )
, m_cOutSIPFileName          ( "none" )
//S051}
//JVT-T054{
, m_uiLayerCGSSNR                       ( 0 )
, m_uiQualityLevelCGSSNR                ( 0 )
, m_uiBaseLayerCGSSNR                       ( MSYS_UINT_MAX )
, m_uiBaseQualityLevelCGSSNR                ( 0 )
//JVT-T054}
// JVT-U085 LMI 
, m_bTlevelNestingFlag  (true)
// JVT-U116 LMI 
, m_bExtensionFlag      (false)
//JVT-U106 Behaviour at slice boundaries{
, m_bCIUFlag      (false)
, m_pbIntraBLFlag  ( NULL)
//JVT-U106 Behaviour at slice boundaries}
, m_bGOPInitialized( false )
{
  ::memset( m_abIsRef,          0x00, sizeof( m_abIsRef           ) );
  ::memset( m_apcFrameTemp,     0x00, sizeof( m_apcFrameTemp      ) );

  UInt ui;
  for( ui = 0; ui <= MAX_DSTAGES; ui++ )
  {
    m_auiNumFramesCoded [ui]  = 0;
    m_adPSNRSumY        [ui]  = 0.0;
    m_adPSNRSumU        [ui]  = 0.0;
    m_adPSNRSumV        [ui]  = 0.0;
  }
  m_uiNewlyCodedBits      = 0;
  m_dLowPassEnhRef        = AR_FGS_DEFAULT_LOW_PASS_ENH_REF;
  m_uiLowPassFgsMcFilter  = AR_FGS_DEFAULT_FILTER;

  for( UInt uiIndex = 0; uiIndex < 2; uiIndex ++ )
  {
    // at least one layer, which is the base layer
    m_uiNumLayers[uiIndex] = 1;

    for( UInt uiLayerIdx = 0; uiLayerIdx < MAX_FGS_LAYERS + 1; uiLayerIdx ++ )
      m_aapcFGSRecon[uiIndex][uiLayerIdx] = 0;
  }
  for( ui = 0; ui < MAX_SCALABLE_LAYERS; ui++ )
  {
    m_auiCurrGOPBits    [ui] = 0;
    m_adSeqBits          [ui] = 0.0;
  }

  m_aapcFGSPredFrame = 0;

}





MCTFEncoder::~MCTFEncoder()
{
}





ErrVal
MCTFEncoder::create( MCTFEncoder*& rpcMCTFEncoder )
{
  rpcMCTFEncoder = new MCTFEncoder;
  ROT( NULL == rpcMCTFEncoder );
  return Err::m_nOK;
}





ErrVal
MCTFEncoder::destroy()
{
  delete this;
  return Err::m_nOK;
}





ErrVal
MCTFEncoder::init( CodingParameter*   pcCodingParameter,
                   LayerParameters*   pcLayerParameters,
                   H264AVCEncoder*    pcH264AVCEncoder,
                   SliceEncoder*      pcSliceEncoder,
                   RQFGSEncoder*      pcRQFGSEncoder,
                   LoopFilter*        pcLoopFilter,
                   PocCalculator*     pcPocCalculator,
                   NalUnitEncoder*    pcNalUnitEncoder,
                   YuvBufferCtrl*     pcYuvFullPelBufferCtrl,
                   YuvBufferCtrl*     pcYuvHalfPelBufferCtrl,
                   QuarterPelFilter*  pcQuarterPelFilter,
                   MotionEstimation*  pcMotionEstimation
				   //JVT-U106 Behaviour at slice boundaries{
				   ,ReconstructionBypass* pcReconstructionBypass
				   //JVT-U106 Behaviour at slice boundaries}
				   )
{
  ROF( pcCodingParameter );
  ROF( pcLayerParameters );
  ROF( pcH264AVCEncoder );
  ROF( pcSliceEncoder );
  ROF( pcRQFGSEncoder );
  ROF( pcLoopFilter );
  ROF( pcPocCalculator );
  ROF( pcNalUnitEncoder );
  ROF( pcYuvFullPelBufferCtrl );
  ROF( pcYuvHalfPelBufferCtrl );
  ROF( pcQuarterPelFilter );
  ROF( pcMotionEstimation );
  //JVT-U106 Behaviour at slice boundaries{
  ROF( pcReconstructionBypass );
  //JVT-U106 Behaviour at slice boundaries}

  //----- references -----
  m_pcSPS                   = 0;
  m_pcPPSLP                 = 0;
  m_pcPPSHP                 = 0;
  m_pcSPS_FGS                  = 0;
  m_pcPPSLP_FGS                = 0;
  m_pcPPSHP_FGS                = 0;
  m_pcYuvFullPelBufferCtrl  = pcYuvFullPelBufferCtrl;
  m_pcYuvHalfPelBufferCtrl  = pcYuvHalfPelBufferCtrl;
  m_pcPocCalculator         = pcPocCalculator;
  m_pcH264AVCEncoder        = pcH264AVCEncoder;
  m_pcSliceEncoder          = pcSliceEncoder;
  m_pcNalUnitEncoder        = pcNalUnitEncoder;
  m_pcLoopFilter            = pcLoopFilter;
  m_pcQuarterPelFilter      = pcQuarterPelFilter;
  m_pcMotionEstimation      = pcMotionEstimation;
  m_pcRQFGSEncoder          = pcRQFGSEncoder;
  //JVT-U106 Behaviour at slice boundaries{
  m_pcReconstructionBypass  = pcReconstructionBypass;
  //JVT-U106 Behaviour at slice boundaries}
  //----- fixed control parameters -----
  m_dLowPassEnhRef          = pcLayerParameters->getLowPassEnhRef();
  m_uiLowPassFgsMcFilter    = pcCodingParameter->getLowPassFgsMcFilter();

  pcLayerParameters->getAdaptiveRefFGSWeights(
    m_uiBaseWeightZeroBaseBlock, m_uiBaseWeightZeroBaseCoeff);
  m_uiFgsEncStructureFlag = pcLayerParameters->getFgsEncStructureFlag();

  m_uiLayerId               = pcLayerParameters->getLayerId                 ();
  m_uiBaseLayerId           = pcLayerParameters->getBaseLayerId             ();
  m_uiBaseQualityLevel      = pcLayerParameters->getBaseQualityLevel        ();

// JVT-Q065 EIDR{
  m_iIDRPeriod        = pcLayerParameters->getIDRPeriod        ();
  m_bBLSkipEnable      = pcLayerParameters->getBLSkipEnable      ();
// JVT-Q065 EIDR}
// JVT-U085 LMI
  m_bTlevelNestingFlag      = pcCodingParameter->getTlevelNestingFlag();
// JVT-U116 LMI
  m_bExtensionFlag          = pcCodingParameter->getExtensionFlag();

  m_bExtendedPriorityId     = pcCodingParameter->getExtendedPriorityId();

  m_uiQualityLevelForPrediction = 3;

  if( pcCodingParameter->getNumberOfLayers() > pcLayerParameters->getLayerId() + 1 )
  {
    m_uiQualityLevelForPrediction = (pcLayerParameters + 1)->getBaseQualityLevel();

    if( m_uiQualityLevelForPrediction > 3)
      m_uiQualityLevelForPrediction = 3;
  }
  m_uiDecompositionStages   = pcLayerParameters->getDecompositionStages     ();
  m_uiTemporalResolution    = pcLayerParameters->getTemporalResolution      ();
  m_uiNotCodedMCTFStages    = pcLayerParameters->getNotCodedMCTFStages      ();
  m_uiFrameDelay            = pcLayerParameters->getFrameDelay              ();
  m_uiMaxNumRefFrames       = pcCodingParameter->getNumRefFrames            ();
  m_uiLowPassIntraPeriod    = pcCodingParameter->getIntraPeriodLowPass      ();
  m_uiNumMaxIter            = pcCodingParameter->getMotionVectorSearchParams().getNumMaxIter      ();
  m_uiIterSearchRange       = pcCodingParameter->getMotionVectorSearchParams().getIterSearchRange ();
  m_iMaxDeltaQp             = pcLayerParameters->getMaxAbsDeltaQP           ();
  m_uiClosedLoopMode        = pcLayerParameters->getClosedLoop              ();
//  m_bH264AVCCompatible      = pcCodingParameter->getBaseLayerMode           ()  > 0 && m_uiLayerId == 0;
  m_bH264AVCCompatible      = m_uiLayerId == 0; //bug-fix suffix
  m_bInterLayerPrediction   = pcLayerParameters->getInterLayerPredictionMode()  > 0;
  m_bAdaptivePrediction     = pcLayerParameters->getInterLayerPredictionMode()  > 1;
  m_bHaarFiltering          = false;
  m_bBiPredOnly             = false;
  m_bForceReOrderingCommands= pcLayerParameters->getForceReorderingCommands ()  > 0;
  m_bWriteSubSequenceSei    = pcCodingParameter->getBaseLayerMode           ()  > 1 && m_uiLayerId == 0;



  //JVT-R057 LA-RDO{
  if(pcCodingParameter->getLARDOEnable()!=0)
  {
    static UInt auiPLR[5];
    static UInt aauiSize[5][2];
    static Double dRatio[5][2];
    auiPLR[m_uiLayerId]      = pcLayerParameters->getPLR                     ();
    m_bLARDOEnable            = pcCodingParameter->getLARDOEnable()==0? false:true;
    m_bLARDOEnable            = m_bLARDOEnable&&((Int)pcLayerParameters->getNumFGSLayers()==0);
    aauiSize[m_uiLayerId][0]  =pcLayerParameters->getFrameWidth();
    aauiSize[m_uiLayerId][1]  =pcLayerParameters->getFrameHeight();
    if(m_uiLayerId==0||pcLayerParameters->getBaseLayerId()==MSYS_UINT_MAX)
    {
      dRatio[m_uiLayerId][0]=1;
      dRatio[m_uiLayerId][1]=1;
    }
    else
    {
      dRatio[m_uiLayerId][0]=(Double)aauiSize[m_uiLayerId][0]/aauiSize[pcLayerParameters->getBaseLayerId()][0];
      dRatio[m_uiLayerId][1]=(Double)aauiSize[m_uiLayerId][1]/aauiSize[pcLayerParameters->getBaseLayerId()][1];
    }
    m_pcSliceEncoder->getMbEncoder()->setRatio(dRatio);
    m_pcSliceEncoder->getMbEncoder()->setPLR(auiPLR);
    pcLayerParameters->setContrainedIntraForLP();
  }
  //JVT-R057 LA-RDO}

  m_uiSuffixUnitEnable = pcCodingParameter->getSuffixUnitEnable();//JVT-S036 lsj
  m_uiMMCOBaseEnable   = pcCodingParameter->getMMCOBaseEnable();  //JVT-S036 lsj

  // TMM_ESS
  m_pcResizeParameters = pcLayerParameters->getResizeParameters();

  for( UInt uiStage = 0; uiStage < MAX_DSTAGES; uiStage++ )
  {
    m_adBaseQpLambdaMotion[uiStage] = pcLayerParameters->getQpModeDecision( uiStage );
  }
  m_dBaseQpLambdaMotionLP   = pcLayerParameters->getQpModeDecisionLP        ();
  m_dBaseQPResidual         = pcLayerParameters->getBaseQpResidual          ();
  m_dNumFGSLayers           = pcLayerParameters->getNumFGSLayers            ();

  m_uiFilterIdc             = pcCodingParameter->getLoopFilterParams        ().getFilterIdc       ();
  m_iAlphaOffset            = pcCodingParameter->getLoopFilterParams        ().getAlphaOffset     ();
  m_iBetaOffset             = pcCodingParameter->getLoopFilterParams        ().getBetaOffset      ();

  m_bLoadMotionInfo         = pcLayerParameters->getMotionInfoMode          () == 1;
  m_bSaveMotionInfo         = pcLayerParameters->getMotionInfoMode          () == 2;
  m_pMotionInfoFile         = 0;

  m_uiFGSMotionMode         = pcLayerParameters->getFGSMotionMode();

  if( m_bLoadMotionInfo )
  {
    m_pMotionInfoFile = ::fopen( pcLayerParameters->getMotionInfoFilename().c_str(), "rb" );
    if( ! m_pMotionInfoFile )
    {
      fprintf( stderr, "\nCANNOT OPEN MOTION INFO FILE \"%s\"\n\n", pcLayerParameters->getMotionInfoFilename().c_str() );
      return Err::m_nERR;
    }
  }
  else if( m_bSaveMotionInfo )
  {
    m_pMotionInfoFile = ::fopen( pcLayerParameters->getMotionInfoFilename().c_str(), "wb" );
    if( ! m_pMotionInfoFile )
    {
      fprintf( stderr, "\nCANNOT OPEN MOTION INFO FILE \"%s\"\n\n", pcLayerParameters->getMotionInfoFilename().c_str() );
      return Err::m_nERR;
    }
  }


  //----- PSNR and rate -----
  m_fOutputFrameRate          = pcLayerParameters->getOutputFrameRate();
  m_uiParameterSetBits        = 0;

  UInt ui;
  for( ui = 0; ui <= MAX_DSTAGES; ui++ )
  {
    m_auiNumFramesCoded [ui]  = 0;
    m_adPSNRSumY        [ui]  = 0.0;
    m_adPSNRSumU        [ui]  = 0.0;
    m_adPSNRSumV        [ui]  = 0.0;
  }
  for( ui = 0; ui < MAX_SCALABLE_LAYERS; ui++ )
  {
    m_auiCurrGOPBits    [ui] = 0;
    m_adSeqBits         [ui] = 0.0;
  }
  m_uiNewlyCodedBits  = 0;

  //----- FGS -----
  m_uiFGSMode = pcLayerParameters->getFGSMode();
  m_pFGSFile  = 0;
  //FIX_FRAG_CAVLC
  m_bUseDiscardableUnit = pcLayerParameters->getUseDiscardable();
  if(m_bUseDiscardableUnit)
  {
    m_pFGSFile = ::fopen( pcLayerParameters->getFGSFilename().c_str(), "rt" );
    if (!m_pFGSFile)
      fprintf( stderr, "Error: FGS failed '%s' couldn't be Opened\n",pcLayerParameters->getFGSFilename().c_str());

    ROF( m_pFGSFile );
  }
  else
  {
    //~FIX_FRAG_CAVLC
  if( m_uiFGSMode == 1 )
  {
    m_pFGSFile = ::fopen( pcLayerParameters->getFGSFilename().c_str(), "wt" );
    if (!m_pFGSFile)
      fprintf( stderr, "Error: FGS failed '%s' couldn't be created\n",pcLayerParameters->getFGSFilename().c_str());
    ROF( m_pFGSFile );
  }
  if( m_uiFGSMode == 2 )
  {
    m_pFGSFile = ::fopen( pcLayerParameters->getFGSFilename().c_str(), "rt" );
    if (!m_pFGSFile)
      fprintf( stderr, "Error: FGS failed '%s' couldn't be Opened\n",pcLayerParameters->getFGSFilename().c_str());

    ROF( m_pFGSFile );
  }
 } //FIX_FRAG_CAVLC
  m_dFGSBitRateFactor               = 0.0;
  m_dFGSRoundingOffset              = 0.0;
  m_iLastFGSError                   = 0;
  m_uiNotYetConsideredBaseLayerBits = 0;

  // analyse and set parameters
  //JVT-P031
  m_bUseDiscardableUnit = pcLayerParameters->getUseDiscardable();
  m_dPredFGSCutFactor = 0.0;
  m_dPredFGSRoundingOffset              = 0.0;
  m_iPredLastFGSError = 0;
  if(m_bUseDiscardableUnit || m_uiFGSMode == 2)
  //JVT-P031
  {
    Char  acLine        [1000];
    UInt  uiNumFrames                       =     0;
    UInt  uiBaseBits                        =     0;
    UInt  uiSumBaseBits                     =     0;
    UInt  uiSumFGSBits  [MAX_FGS_LAYERS]    =   { 0, 0, 0 };
    UInt  uiFGSBits     [MAX_FGS_LAYERS]    =   { 0, 0, 0 };
    UInt  uiDummy;

    for( ; ; )
    {
      Int i, c;
      for( i = 0; ( c = fgetc(m_pFGSFile), ( c != '\n' && c != EOF ) ); acLine[i++] = c );
      acLine[i] = '\0';
      if( feof(m_pFGSFile) )
      {
        break;
      }

      sscanf( acLine, "%d %d %d %d %d %d %d ",
        &uiBaseBits, &uiDummy, &uiFGSBits[0], &uiDummy, &uiFGSBits[1], &uiDummy, &uiFGSBits[2] );

      uiNumFrames     ++;
      uiSumBaseBits   += uiBaseBits;
      uiSumFGSBits[0] += uiFGSBits[0];
      uiSumFGSBits[1] += uiFGSBits[1];
      uiSumFGSBits[2] += uiFGSBits[2];
    }
    ROF( uiNumFrames );
    //FIX_FRAG_CAVLC
    if(m_uiFGSMode == 2)
    {
      //~FIX_FRAG_CAVLC
    Double  dTargetBits   = 1000.0 * pcLayerParameters->getFGSRate() * (Double)uiNumFrames / pcLayerParameters->getOutputFrameRate();
    UInt    uiTargetBits  = (UInt)floor( dTargetBits + 0.5 );
    UInt    uiSumAllBits  = uiSumBaseBits + uiSumFGSBits[0] + uiSumFGSBits[1] + uiSumFGSBits[2];

    if( uiTargetBits <= uiSumBaseBits )
    {
      ROF( uiTargetBits );
      printf("Warning: Layer %d bitrate overflow (only base layer coded)\n", m_uiLayerId );
      m_dFGSCutFactor     = 0.0;
      m_dFGSBitRateFactor = (Double)uiTargetBits / (Double)uiSumBaseBits; // there is a chance that only coding the base layer is not the right thing for closed-loop
      m_dNumFGSLayers     = 0.0;
    }
    else if( uiTargetBits >= uiSumAllBits )
    {
      printf("Warning: Layer %d bitrate underflow (code as much as possible)\n", m_uiLayerId );
      m_dFGSCutFactor     = 3.0;
      m_dFGSBitRateFactor = (Double)uiTargetBits / (Double)uiSumAllBits; // it is possible that not all layers have been coded during the analysis run (e.g. for closed-loop)
      m_dNumFGSLayers     = 3.0;

      for( UInt uiFGSLayer = 0; uiFGSLayer < MAX_FGS_LAYERS; uiFGSLayer++ )
      {
        if (uiSumFGSBits[uiFGSLayer] == 0)
        {
          m_dNumFGSLayers = (Double)uiFGSLayer;
          break;
        }
      }
    }
    else
    {
      uiTargetBits   -= uiSumBaseBits;
      for( UInt uiFGSLayer = 0; uiFGSLayer < MAX_FGS_LAYERS; uiFGSLayer++ )
      {
        if( uiTargetBits < uiSumFGSBits[uiFGSLayer] )
        {
          m_dFGSCutFactor = (Double)uiFGSLayer + (Double)uiTargetBits / (Double)uiSumFGSBits[uiFGSLayer];
          m_dNumFGSLayers = uiFGSLayer + 1;
          break;
        }
        uiTargetBits -= uiSumFGSBits[uiFGSLayer];
      }
      m_dFGSBitRateFactor = 0.0;
    }
    pcLayerParameters->setNumFGSLayers( m_dNumFGSLayers ); // (HS): fix - also store in layer parameters
    }//FIX_FRAG_CAVLC
    //JVT-P031
    if(m_bUseDiscardableUnit)
    {
    Double  dPredTargetBits   = 1000.0 * pcLayerParameters->getPredFGSRate() * (Double)uiNumFrames / pcLayerParameters->getOutputFrameRate();
    UInt    uiPredTargetBits  = (UInt)floor( dPredTargetBits + 0.5 );
    UInt    uiSumAllBits  = uiSumBaseBits + uiSumFGSBits[0] + uiSumFGSBits[1] + uiSumFGSBits[2]; //FIX_FRAG_CAVLC

    if( uiPredTargetBits <= uiSumBaseBits )
    {
      ROF( uiPredTargetBits );
      printf("Warning: Layer %d bitrate overflow (only base layer coded)\n", m_uiLayerId );
      m_dPredFGSCutFactor     = 0.0;
      m_dPredFGSBitRateFactor = (Double)uiPredTargetBits / (Double)uiSumBaseBits; // there is a chance that only coding the base layer is not the right thing for closed-loop
    }
    else if( uiPredTargetBits >= uiSumAllBits )
    {
      printf("Warning: Layer %d bitrate underflow (code as much as possible)\n", m_uiLayerId );
      m_dPredFGSCutFactor     = 3.0;
      m_dPredFGSBitRateFactor = (Double)uiPredTargetBits / (Double)uiSumAllBits; // it is possible that not all layers have been coded during the analysis run (e.g. for closed-loop)
    }
    else
    {
      uiPredTargetBits   -= uiSumBaseBits;
      for( UInt uiFGSLayer = 0; uiFGSLayer < MAX_FGS_LAYERS; uiFGSLayer++ )
      {
        if( uiPredTargetBits < uiSumFGSBits[uiFGSLayer] )
        {
          m_dPredFGSCutFactor = (Double)uiFGSLayer + (Double)uiPredTargetBits / (Double)uiSumFGSBits[uiFGSLayer];
          break;
        }
        uiPredTargetBits -= uiSumFGSBits[uiFGSLayer];
      }
      m_dPredFGSBitRateFactor = 0.0;
    }
    }
    //~JVT-P031
    ::fseek( m_pFGSFile, 0, SEEK_SET );
  }


  // JVT-S054 (ADD) ->
  m_bIroiSliceDivisionFlag = pcLayerParameters->m_bSliceDivisionFlag;
  if (m_bIroiSliceDivisionFlag)
  {
    m_uiNumSliceMinus1 = pcLayerParameters->m_uiNumSliceMinus1;
    if (m_puiFirstMbInSlice != NULL)
      free(m_puiFirstMbInSlice);
    m_puiFirstMbInSlice = (UInt*)malloc((m_uiNumSliceMinus1+1)*sizeof(UInt));
    memcpy( m_puiFirstMbInSlice, pcLayerParameters->m_puiFirstMbInSlice, (m_uiNumSliceMinus1+1)*sizeof(UInt) );

    if (m_puiLastMbInSlice != NULL)
      free(m_puiLastMbInSlice);
    m_puiLastMbInSlice = (UInt*)malloc((m_uiNumSliceMinus1+1)*sizeof(UInt));
    memcpy( m_puiLastMbInSlice, pcLayerParameters->m_puiLastMbInSlice, (m_uiNumSliceMinus1+1)*sizeof(UInt) );
  }
  // JVT-S054 (ADD) <-

  //S051{
  m_uiTotalFrame  = pcCodingParameter->getTotalFrames();
  m_uiAnaSIP    = pcLayerParameters->getAnaSIP();
  m_cOutSIPFileName  = pcLayerParameters->getOutSIPFileName();
  if(m_uiAnaSIP==1)
    m_bInterLayerPrediction=true;
  if(m_uiAnaSIP==2)
    m_bInterLayerPrediction=m_bAdaptivePrediction=false;

  if(pcCodingParameter->getNumberOfLayers() > m_uiLayerId+1)
  {
    m_bEncSIP      = pcCodingParameter->getLayerParameters( m_uiLayerId+1).getEncSIP();
    m_cInSIPFileName  = pcCodingParameter->getLayerParameters( m_uiLayerId+1).getInSIPFileName();
  }
  //S051}

  //JVT-U106 Behaviour at slice boundaries{
  m_bCIUFlag=pcCodingParameter->getCIUFlag()!=0;
  //JVT-U106 Behaviour at slice boundaries}
  
  m_uiFramesInCompleteGOPsProcessed = 0;
  m_uiMinScalableLayer              = 0;
  for( UInt uiBaseLayerId = 0; uiBaseLayerId < m_uiLayerId; uiBaseLayerId++ )
  {
    m_uiMinScalableLayer += (      pcCodingParameter->getLayerParameters( uiBaseLayerId ).getDecompositionStages() -
      pcCodingParameter->getLayerParameters( uiBaseLayerId ).getNotCodedMCTFStages () + 1U ) *
      ( 1U +
      (UInt)pcCodingParameter->getLayerParameters( uiBaseLayerId ).getNumFGSLayers       ()      );
  }

  return Err::m_nOK;
}



__inline UInt downround2powerof2( UInt i ) { UInt r = 1; for( ; (UInt)( 1 << r ) <= i; r++ ); return ( 1 << ( r - 1 ) ); }

ErrVal
MCTFEncoder::initParameterSetsForFGS( const SequenceParameterSet& rcSPS,
                                const PictureParameterSet&  rcPPSLP,
                                const PictureParameterSet&  rcPPSHP )
{
  m_pcSPS_FGS                 = &rcSPS;
  m_pcPPSLP_FGS               = &rcPPSLP;
  m_pcPPSHP_FGS               = &rcPPSHP;
  return Err::m_nOK;
}

ErrVal
MCTFEncoder::initParameterSets( const SequenceParameterSet& rcSPS,
                                const PictureParameterSet&  rcPPSLP,
                                const PictureParameterSet&  rcPPSHP )
{
  //===== set references =====
  m_pcSPS                 = &rcSPS;
  m_pcPPSLP               = &rcPPSLP;
  m_pcPPSHP               = &rcPPSHP;

  m_pcSPS_FGS                 = &rcSPS;
  m_pcPPSLP_FGS               = &rcPPSLP;
  m_pcPPSHP_FGS               = &rcPPSHP;


  //===== get and set relevant parameters =====
  UInt  uiMaxDPBSize      = rcSPS.getMaxDPBSize       ();
  m_uiFrameWidthInMb      = rcSPS.getFrameWidthInMbs  ();
  m_uiFrameHeightInMb     = rcSPS.getFrameHeightInMbs ();
  m_uiMbNumber            = m_uiFrameWidthInMb * m_uiFrameHeightInMb;
  m_uiMaxGOPSize          = downround2powerof2( uiMaxDPBSize );

  //===== re-allocate dynamic memory =====
  RNOK( xDeleteData()        );
  RNOK( xCreateData( rcSPS ) );

  //===== initialize some parameters =====
  m_bInitDone             = true;
  m_bFirstGOPCoded        = false;
  m_uiFrameCounter        = 0;
  m_uiFrameNum            = 0;
  m_uiGOPNumber           = 0;
  ::memset( m_abIsRef, 0x00, sizeof( m_abIsRef ) );

  return Err::m_nOK;
}




ErrVal
MCTFEncoder::addParameterSetBits( UInt uiParameterSetBits )
{
  m_uiParameterSetBits += uiParameterSetBits;
  return Err::m_nOK;
}




ErrVal
MCTFEncoder::xCreateData( const SequenceParameterSet& rcSPS )
{
  UInt uiIndex;


  //========== CREATE FRAME MEMORIES ==========
  ROFS   ( ( m_papcFrame                     = new IntFrame* [ m_uiMaxGOPSize + 1 ]      ) );
  ROFS   ( ( m_papcOrgFrame                  = new IntFrame* [ m_uiMaxGOPSize + 1 ]      ) );
  if( m_uiClosedLoopMode == 2 )
  {
    ROFS ( ( m_papcBQFrame                   = new IntFrame* [ m_uiMaxGOPSize + 1 ]      ) );
  }

  if( m_uiQualityLevelForPrediction < 3 || m_bUseDiscardableUnit) //JVT-P031
  {
    ROFS ( ( m_papcCLRecFrame                = new IntFrame* [ m_uiMaxGOPSize + 1 ]      ) );
  }
  ROFS   ( ( m_papcResidual                  = new IntFrame* [ m_uiMaxGOPSize + 1 ]      ) );
  ROFS   ( ( m_papcSubband                   = new IntFrame* [ m_uiMaxGOPSize + 1 ]      ) );

  //-- JVT-R091
  ROFS   ( ( m_papcSmoothedFrame              = new IntFrame* [ m_uiMaxGOPSize + 1 ]      ) );
  //--

  for( uiIndex = 0; uiIndex <= m_uiMaxGOPSize; uiIndex++ )
  {
    ROFS ( ( m_papcFrame         [ uiIndex ] = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                              *m_pcYuvHalfPelBufferCtrl ) ) );
    ROFS ( ( m_papcOrgFrame      [ uiIndex ] = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                              *m_pcYuvFullPelBufferCtrl ) ) );
    if( m_papcBQFrame )
    {
      ROFS(( m_papcBQFrame       [ uiIndex ] = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                              *m_pcYuvFullPelBufferCtrl ) ) );
    }
    if( m_papcCLRecFrame )
    {
      ROFS(( m_papcCLRecFrame    [ uiIndex ] = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                              *m_pcYuvHalfPelBufferCtrl ) ) );
    }

    //-- JVT-R091
    if ( m_papcSmoothedFrame )
    {
      ROFS(( m_papcSmoothedFrame  [ uiIndex ] = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                              *m_pcYuvHalfPelBufferCtrl ) ) );
    }
    //--

    ROFS ( ( m_papcResidual      [ uiIndex ] = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                              *m_pcYuvHalfPelBufferCtrl ) ) );
    ROFS ( ( m_papcSubband       [ uiIndex ] = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                              *m_pcYuvHalfPelBufferCtrl ) ) );
    RNOK  (   m_papcFrame         [ uiIndex ] ->init        () );
    RNOK  (   m_papcOrgFrame      [ uiIndex ] ->init        () );
    if( m_papcBQFrame )
    {
      RNOK(   m_papcBQFrame       [ uiIndex ] ->init        () );
    }
    if( m_papcCLRecFrame )
    {
      RNOK(   m_papcCLRecFrame    [ uiIndex ] ->init        () );
    }

    //-- JVT-R091
    if ( m_papcSmoothedFrame )
    {
      RNOK(   m_papcSmoothedFrame  [ uiIndex ] ->init        () );
    }
    //--

    RNOK  (   m_papcResidual      [ uiIndex ] ->init        () );
    RNOK  (   m_papcSubband       [ uiIndex ] ->init        () );
  }

  for( uiIndex = 0; uiIndex < NUM_TMP_FRAMES;  uiIndex++ )
  {
    ROFS ( ( m_apcFrameTemp      [ uiIndex ] = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                              *m_pcYuvHalfPelBufferCtrl ) ) );
    RNOK  (   m_apcFrameTemp      [ uiIndex ] ->init        () );
  }

  for( uiIndex = 0; uiIndex < 2;  uiIndex++ )
  {
    for( UInt uiLayerIdx = 0; uiLayerIdx < 4; uiLayerIdx ++ )
    {
      ROFS   ( ( m_aapcFGSRecon[uiIndex][uiLayerIdx]   = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                              *m_pcYuvHalfPelBufferCtrl ) ) );
      RNOK    (   m_aapcFGSRecon[uiIndex][uiLayerIdx]   ->init        () );
    }
  }

  ROFS   ( ( m_aapcFGSPredFrame   = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                          *m_pcYuvHalfPelBufferCtrl ) ) );
  RNOK    (   m_aapcFGSPredFrame   ->init        () );

  ROFS   ( ( m_pcLowPassBaseReconstruction   = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                              *m_pcYuvHalfPelBufferCtrl ) ) );
  RNOK    (   m_pcLowPassBaseReconstruction   ->init        () );

  ROFS   ( ( m_pcAnchorFrameOriginal         = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                              *m_pcYuvHalfPelBufferCtrl ) ) );
  RNOK    (   m_pcAnchorFrameOriginal         ->init        () );

  ROFS   ( ( m_pcAnchorFrameReconstructed    = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                              *m_pcYuvHalfPelBufferCtrl ) ) );
  RNOK    (   m_pcAnchorFrameReconstructed    ->init        () );

  ROFS   ( ( m_pcBaseLayerFrame              = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                              *m_pcYuvHalfPelBufferCtrl ) ) );
  RNOK    (   m_pcBaseLayerFrame              ->init        () );

  ROFS   ( ( m_pcBaseLayerResidual           = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                              *m_pcYuvHalfPelBufferCtrl ) ) );
  RNOK    (   m_pcBaseLayerResidual           ->init        () );



  //========== CREATE MACROBLOCK DATA MEMORIES ==========
  ROFS   ( ( m_pacControlData  = new ControlData[ m_uiMaxGOPSize + 1 ] ) );
  ROFS   ( ( m_pacControlDataEL  = new ControlData[ m_uiMaxGOPSize + 1 ] ) );

  for( uiIndex = 0; uiIndex <= m_uiMaxGOPSize; uiIndex++ )
  {
    MbDataCtrl*   pcMbDataCtrl                = 0;
    ROFS ( (     pcMbDataCtrl                = new MbDataCtrl  () ) );
    RNOK  (       pcMbDataCtrl                ->init            ( rcSPS ) );
    RNOK  (       m_pacControlData[ uiIndex ] . setMbDataCtrl   ( pcMbDataCtrl ) );
    RNOK  (       m_pacControlData[ uiIndex ] .initFGSData      ( m_uiFrameWidthInMb * m_uiFrameHeightInMb ) );

    MbDataCtrl*   pcMbDataCtrlEL                = 0;
    ROFS ( (     pcMbDataCtrlEL                = new MbDataCtrl  () ) );
    RNOK  (       pcMbDataCtrlEL                ->init            ( rcSPS ) );
    RNOK  (       m_pacControlDataEL[ uiIndex ] . setMbDataCtrl   ( pcMbDataCtrlEL ) );
    RNOK  (       m_pacControlDataEL[ uiIndex ] .initFGSData      ( m_uiFrameWidthInMb * m_uiFrameHeightInMb ) );

    Bool          bLowPass                    = ( ( uiIndex % ( 1 << m_uiDecompositionStages ) ) == 0 );
    SliceHeader*  pcSliceHeader               = 0;
    ROFS ( (     pcSliceHeader               = new SliceHeader ( *m_pcSPS, bLowPass ? *m_pcPPSLP : *m_pcPPSHP ) ) );
    RNOK  (       m_pacControlData[ uiIndex ] . setSliceHeader  (  pcSliceHeader ) );

  // ICU/ETRI FGS_MOT_USE
  SliceHeader*  pcSliceHeaderEL               = 0;
  ROFS ( (     pcSliceHeaderEL               = new SliceHeader ( *m_pcSPS, bLowPass ? *m_pcPPSLP : *m_pcPPSHP ) ) );
  RNOK  (       m_pacControlDataEL[ uiIndex ] . setSliceHeader  (  pcSliceHeaderEL ) );

    if( m_uiClosedLoopMode == 2 )
    {
      RNOK(       m_pacControlData[ uiIndex ] .initBQData       ( m_uiFrameWidthInMb * m_uiFrameHeightInMb ) );
      // ICU/ETRI FGS_MOT_USE
    RNOK(       m_pacControlDataEL[ uiIndex ] .initBQData       ( m_uiFrameWidthInMb * m_uiFrameHeightInMb ) );
    }

    m_pacControlData[ uiIndex ].getMbDataCtrl()->initFgsBQData(m_uiFrameWidthInMb * m_uiFrameHeightInMb);
    m_pacControlDataEL[ uiIndex ].getMbDataCtrl()->initFgsBQData(m_uiFrameWidthInMb * m_uiFrameHeightInMb);
  }

  ROFS   ( ( m_pcBaseLayerCtrl = new MbDataCtrl() ) );
  RNOK    (   m_pcBaseLayerCtrl ->init          ( rcSPS ) );

  ROFS   ( ( m_pcBaseLayerCtrlEL = new MbDataCtrl() ) );
  RNOK    (   m_pcBaseLayerCtrlEL ->init          ( rcSPS ) );



  //========== CREATE UPDATE WEIGHTS ARRAY and WRITE BUFFER ==========
  UInt  uiNum4x4Blocks        = m_uiFrameWidthInMb * m_uiFrameHeightInMb * 4 * 4;
  m_uiWriteBufferSize         = 3 * ( uiNum4x4Blocks * 4 * 4 );
  ROFS( ( m_pucWriteBuffer   = new UChar [ m_uiWriteBufferSize ] ) );
  ROT ( m_cDownConvert    .init   ( m_uiFrameWidthInMb<<4, m_uiFrameHeightInMb<<4 ) );
  RNOK( m_pcRQFGSEncoder ->initSPS( rcSPS ) );

  //S051{
  ROFRS( m_auiFrameBits  =  new UInt[m_uiTotalFrame], Err::m_nERR );
  memset( m_auiFrameBits,0,sizeof(UInt)*m_uiTotalFrame);
  if(m_bEncSIP)
  {
    FILE* file=fopen(m_cInSIPFileName.c_str(),"rt");
    if(file==NULL)
    {
      printf("\nCan't open SIP file %s",m_cInSIPFileName.c_str());
      return Err::m_nOK;
    }
    while(!feof(file))
  {
    UInt tmp;
    fscanf(file,"%d",&tmp);
    m_cPOCList.push_back(tmp);
  }
    fclose(file);
  }
  //S051}

  return Err::m_nOK;
}





ErrVal
MCTFEncoder::xDeleteData()
{
  UInt uiIndex;

  //========== DELETE FRAME MEMORIES ==========
  if( m_papcFrame )
  {
    for( uiIndex = 0; uiIndex <= m_uiMaxGOPSize; uiIndex++ )
    {
      if( m_papcFrame[ uiIndex ] )
      {
    //JVT-R057 LA-RDO{
    if(m_bLARDOEnable)
       m_papcFrame[uiIndex]->uninitChannelDistortion();
    //JVT-R057 LA-RDO}
        RNOK(   m_papcFrame[ uiIndex ]->uninit() );
        delete  m_papcFrame[ uiIndex ];
        m_papcFrame[ uiIndex ] = 0;
      }
    }
    delete [] m_papcFrame;
    m_papcFrame = 0;
  }

  if( m_papcOrgFrame )
  {
    for( uiIndex = 0; uiIndex <= m_uiMaxGOPSize; uiIndex++ )
    {
      if( m_papcOrgFrame[ uiIndex ] )
      {
        RNOK(   m_papcOrgFrame[ uiIndex ]->uninit() );
        delete  m_papcOrgFrame[ uiIndex ];
        m_papcOrgFrame[ uiIndex ] = 0;
      }
    }
    delete [] m_papcOrgFrame;
    m_papcOrgFrame = 0;
  }

  if( m_papcBQFrame )
  {
    for( uiIndex = 0; uiIndex <= m_uiMaxGOPSize; uiIndex++ )
    {
      if( m_papcBQFrame[ uiIndex ] )
      {
        RNOK(   m_papcBQFrame[ uiIndex ]->uninit() );
        delete  m_papcBQFrame[ uiIndex ];
        m_papcBQFrame[ uiIndex ] = 0;
      }
    }
    delete [] m_papcBQFrame;
    m_papcBQFrame = 0;
  }

  if( m_papcCLRecFrame )
  {
    for( uiIndex = 0; uiIndex <= m_uiMaxGOPSize; uiIndex++ )
    {
      if( m_papcCLRecFrame[ uiIndex ] )
      {
        RNOK(   m_papcCLRecFrame[ uiIndex ]->uninit() );
        delete  m_papcCLRecFrame[ uiIndex ];
        m_papcCLRecFrame[ uiIndex ] = 0;
      }
    }
    delete [] m_papcCLRecFrame;
    m_papcCLRecFrame = 0;
  }

  //-- JVT-R091
  if ( m_papcSmoothedFrame )
  {
    for( uiIndex = 0; uiIndex <= m_uiMaxGOPSize; uiIndex++ )
    {
      if( m_papcSmoothedFrame[ uiIndex ] )
      {
        RNOK(   m_papcSmoothedFrame[ uiIndex ]->uninit() );
        delete  m_papcSmoothedFrame[ uiIndex ];
        m_papcSmoothedFrame[ uiIndex ] = 0;
      }
    }
    delete [] m_papcSmoothedFrame;
    m_papcSmoothedFrame = 0;
  }
  //--

  if( m_papcResidual )
  {
    for( uiIndex = 0; uiIndex <= m_uiMaxGOPSize; uiIndex++ )
    {
      if( m_papcResidual[ uiIndex ] )
      {
        RNOK(   m_papcResidual[ uiIndex ]->uninit() );
        delete  m_papcResidual[ uiIndex ];
        m_papcResidual[ uiIndex ] = 0;
      }
    }
    delete [] m_papcResidual;
    m_papcResidual = 0;
  }

  if( m_papcSubband )
  {
    for( uiIndex = 0; uiIndex <= m_uiMaxGOPSize; uiIndex++ )
    {
      if( m_papcSubband[ uiIndex ] )
      {
        RNOK(   m_papcSubband[ uiIndex ]->uninit() );
        delete  m_papcSubband[ uiIndex ];
        m_papcSubband[ uiIndex ] = 0;
      }
    }
    delete [] m_papcSubband;
    m_papcSubband = 0;
  }

  for( uiIndex = 0; uiIndex < NUM_TMP_FRAMES; uiIndex++ )
  {
    if( m_apcFrameTemp[ uiIndex ] )
    {
      RNOK(   m_apcFrameTemp[ uiIndex ]->uninit() );
      delete  m_apcFrameTemp[ uiIndex ];
      m_apcFrameTemp[ uiIndex ] = 0;
    }
  }

  for( uiIndex = 0; uiIndex < 2; uiIndex ++ )
  {
    for( UInt uiLayerIdx = 0; uiLayerIdx < 4; uiLayerIdx++ )
    {
      if( m_aapcFGSRecon[uiIndex][uiLayerIdx] )
      {
        RNOK(   m_aapcFGSRecon[uiIndex][uiLayerIdx]->uninit() );
        delete  m_aapcFGSRecon[uiIndex][uiLayerIdx];
        m_aapcFGSRecon[uiIndex][uiLayerIdx]  = 0;
      }
    }
  }

  if( m_aapcFGSPredFrame)
  {
    RNOK(   m_aapcFGSPredFrame->uninit() );
    delete  m_aapcFGSPredFrame;
    m_aapcFGSPredFrame  = 0;
  }

  if( m_pcLowPassBaseReconstruction )
  {
  // JVT-R057 LA-RDO{
  if(m_bLARDOEnable)
    m_pcLowPassBaseReconstruction->uninitChannelDistortion();
  // JVT-R057 LA-RDO}
    RNOK(   m_pcLowPassBaseReconstruction->uninit() );
    delete  m_pcLowPassBaseReconstruction;
    m_pcLowPassBaseReconstruction = 0;
  }

  if( m_pcAnchorFrameOriginal )
  {
    RNOK(   m_pcAnchorFrameOriginal->uninit() );
    delete  m_pcAnchorFrameOriginal;
    m_pcAnchorFrameOriginal = 0;
  }

  if( m_pcAnchorFrameReconstructed )
  {
    RNOK(   m_pcAnchorFrameReconstructed->uninit() );
    delete  m_pcAnchorFrameReconstructed;
    m_pcAnchorFrameReconstructed = 0;
  }

  if( m_pcBaseLayerFrame )
  {
    RNOK(   m_pcBaseLayerFrame->uninit() );
    delete  m_pcBaseLayerFrame;
    m_pcBaseLayerFrame = 0;
  }

  if( m_pcBaseLayerResidual )
  {
    RNOK(   m_pcBaseLayerResidual->uninit() );
    delete  m_pcBaseLayerResidual;
    m_pcBaseLayerResidual = 0;
  }


  //========== DELETE MACROBLOCK DATA MEMORIES (and SLICE HEADER) ==========
  if( m_pacControlData )
  {
    for( uiIndex = 0; uiIndex <= m_uiMaxGOPSize; uiIndex++ )
    {
      RNOK( m_pacControlData[ uiIndex ].uninitBQData() );
      RNOK( m_pacControlData[ uiIndex ].uninitFGSData() );

      RNOK( m_pacControlData[ uiIndex ].getMbDataCtrl()->uninitFgsBQData() );

      MbDataCtrl*   pcMbDataCtrl  = m_pacControlData[ uiIndex ].getMbDataCtrl  ();
      SliceHeader*  pcSliceHeader = m_pacControlData[ uiIndex ].getSliceHeader ();
      if( pcMbDataCtrl )
      {
        RNOK( pcMbDataCtrl->uninit() );
      }
      delete pcMbDataCtrl;
      delete pcSliceHeader;
    }
    delete [] m_pacControlData;
    m_pacControlData = 0;
  }

  // ICU/ETRI FGS_MOT_USE
  if( m_pacControlDataEL )
  {
    for( uiIndex = 0; uiIndex <= m_uiMaxGOPSize; uiIndex++ )
    {
      RNOK( m_pacControlDataEL[ uiIndex ].uninitBQData() );
      RNOK( m_pacControlDataEL[ uiIndex ].uninitFGSData() );

      RNOK( m_pacControlDataEL[ uiIndex ].getMbDataCtrl()->uninitFgsBQData() );

      MbDataCtrl*   pcMbDataCtrlEL  = m_pacControlDataEL[ uiIndex ].getMbDataCtrl  ();
      SliceHeader*  pcSliceHeaderEL = m_pacControlDataEL[ uiIndex ].getSliceHeader ();
      if( pcMbDataCtrlEL )
      {
        RNOK( pcMbDataCtrlEL->uninit() );
      }
      delete pcMbDataCtrlEL;
      delete pcSliceHeaderEL;
    }
    delete [] m_pacControlDataEL;
    m_pacControlDataEL = 0;
  }


  if( m_pcBaseLayerCtrl )
  {
    RNOK( m_pcBaseLayerCtrl->uninit() );
    delete m_pcBaseLayerCtrl;
    m_pcBaseLayerCtrl = 0;
  }

  // ICU/ETRI FGS_MOT_USE
  if( m_pcBaseLayerCtrlEL )
  {
    RNOK( m_pcBaseLayerCtrlEL->uninit() );
    delete m_pcBaseLayerCtrlEL;
    m_pcBaseLayerCtrlEL = 0;
  }


  //========== DELETE UPDATE WEIGHTS ARRAY and WRITE BUFFER ==========
  delete [] m_pucWriteBuffer;
  m_pucWriteBuffer    = 0;
  m_uiWriteBufferSize = 0;

  //S051{
  delete[]  m_auiFrameBits;
  //S051}

  return Err::m_nOK;
}





ErrVal
MCTFEncoder::uninit()
{
  m_bInitDone  = false;

  xDeleteData();

  if( m_pMotionInfoFile )
  {
    ::fclose( m_pMotionInfoFile );
  }

  if( m_pFGSFile )
  {
    ::fclose( m_pFGSFile );
  }

  // JVT-S054 (ADD) ->
  if( m_puiFirstMbInSlice )
  {
    free(m_puiFirstMbInSlice);
    m_puiFirstMbInSlice = 0;
  }
  if( m_puiLastMbInSlice )
  {
    free(m_puiLastMbInSlice);
    m_puiLastMbInSlice = 0;
  }
  // JVT-S054 (ADD) <-

  //JVT-U106 Behaviour at slice boundaries{
  if(m_pbIntraBLFlag)
  {
	  delete[] m_pbIntraBLFlag;
	  m_pbIntraBLFlag=NULL;
  }
  //JVT-U106 Behaviour at slice boundaries{
  return Err::m_nOK;
}





ErrVal
MCTFEncoder::xFillAndUpsampleFrame( IntFrame* rcFrame )
{
  RNOK( m_pcYuvFullPelBufferCtrl->initMb() );
  RNOK( m_pcYuvHalfPelBufferCtrl->initMb() );

  if( ! rcFrame->isHalfPel() )
  {
    RNOK( rcFrame->initHalfPel() );
  }

  RNOK( rcFrame->extendFrame( m_pcQuarterPelFilter ) );

  return Err::m_nOK;
}





ErrVal
MCTFEncoder::xFillAndExtendFrame( IntFrame* rcFrame )
{
  RNOK( m_pcYuvFullPelBufferCtrl->initMb() );

  RNOK( rcFrame->extendFrame( NULL ) );

  return Err::m_nOK;
}





ErrVal
MCTFEncoder::xMotionEstimation( RefFrameList*    pcRefFrameList0,
                                RefFrameList*    pcRefFrameList1,
                                const IntFrame*  pcOrigFrame,
                                IntFrame*        pcIntraRecFrame,
                                ControlData&     rcControlData,
                                Bool             bBiPredOnly,
                                UInt             uiNumMaxIter,
                                // JVT-S054 (REPLACE) ->
                                //UInt             uiIterSearchRange )
                                UInt             uiIterSearchRange,
                                UInt             uiFrameIdInGOP )
                                // JVT-S054 (REPLACE) <-
{
  MbEncoder*    pcMbEncoder           =  m_pcSliceEncoder->getMbEncoder         ();
  SliceHeader&  rcSliceHeader         = *rcControlData.getSliceHeader           ();
  MbDataCtrl*   pcMbDataCtrl          =  rcControlData.getMbDataCtrl            ();
  IntFrame*     pcBaseLayerFrame      =  rcControlData.getBaseLayerRec          ();
  IntFrame*     pcBaseLayerResidual   =  rcControlData.getBaseLayerSbb          ();
  MbDataCtrl*   pcBaseLayerCtrl       =  rcControlData.getBaseLayerCtrl         ();
  Int           iSpatialScalabilityType = rcControlData.getSpatialScalabilityType();
  Bool          bEstimateBase         =  rcSliceHeader.getBaseLayerId           () == MSYS_UINT_MAX && ! pcBaseLayerCtrl;
  Bool          bEstimateMotion       =  rcSliceHeader.getAdaptivePredictionFlag() || bEstimateBase;

  // JVT-S054 (ADD)
  MbDataCtrl*     pcMbDataCtrlL1    = xGetMbDataCtrlL1( rcSliceHeader, uiFrameIdInGOP );

// JVT-S054 (REPLACE) ->
  //===== copy motion if non-adaptive prediction =====
  if( ! bEstimateMotion )
  {
    ROF ( pcBaseLayerCtrl )

    if (m_bIroiSliceDivisionFlag)
    {
      for (UInt uiSliceId=0; uiSliceId <= m_uiNumSliceMinus1; uiSliceId++)
      {
        rcSliceHeader.setFirstMbInSlice(m_puiFirstMbInSlice[uiSliceId]);
        rcSliceHeader.setLastMbInSlice(m_puiLastMbInSlice[uiSliceId]);
        // JVT-S054 (2) (ADD)
        rcSliceHeader.setNumMbsInSlice(rcSliceHeader.getFMO()->getNumMbsInSlice(rcSliceHeader.getFirstMbInSlice(), rcSliceHeader.getLastMbInSlice()));
        UInt uiMbAddress       = rcSliceHeader.getFirstMbInSlice();
        UInt uiLastMbAddress   = rcSliceHeader.getLastMbInSlice();
        UInt uiNumMBInSlice;
        //===== initialization =====
        RNOK( pcMbDataCtrl->initSlice( rcSliceHeader, ENCODE_PROCESS, false, pcMbDataCtrlL1 ) );

        //===== loop over macroblocks =====
        for(  uiNumMBInSlice = 0; uiMbAddress <= uiLastMbAddress;  )
        {
          UInt          uiMbY               = uiMbAddress / m_uiFrameWidthInMb;
          UInt          uiMbX               = uiMbAddress % m_uiFrameWidthInMb;
          MbDataAccess* pcMbDataAccess      = 0;
          MbDataAccess* pcMbDataAccessBase  = 0;

          //===== init macroblock =====
          RNOK  ( pcMbDataCtrl            ->initMb( pcMbDataAccess,     uiMbY, uiMbX ) );
          if    ( pcBaseLayerCtrl )
          {
            RNOK( pcBaseLayerCtrl         ->initMb( pcMbDataAccessBase, uiMbY, uiMbX ) );
          }

          RNOK( pcMbDataCtrl->getMbData(uiMbX, uiMbY).copyMotion( pcBaseLayerCtrl->getMbData(uiMbX, uiMbY), pcMbDataCtrl->getSliceId() ) );
          // <<<< bug fix by heiko.schwarz@hhi.fhg.de
          if( pcBaseLayerFrame ) // the motion data are not just copied, but inferred from the base layer
          {
            pcMbDataCtrl->getMbDataByIndex( uiMbAddress ).getMbMvdData( LIST_0 ).clear();
            pcMbDataCtrl->getMbDataByIndex( uiMbAddress ).getMbMvdData( LIST_1 ).clear();
          }
          // >>>> bug fix by heiko.schwarz@hhi.fhg.de
          uiMbAddress = rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress);
          uiNumMBInSlice++;
        }
      }
    }
    else
    {
      FMO* pcFMO = rcControlData.getSliceHeader()->getFMO();
      for (UInt iSliceGroupID=0;!pcFMO->SliceGroupCompletelyCoded(iSliceGroupID);iSliceGroupID++)
      {
        rcSliceHeader.setFirstMbInSlice(pcFMO->getFirstMacroblockInSlice(iSliceGroupID));
        rcSliceHeader.setLastMbInSlice(pcFMO->getLastMBInSliceGroup(iSliceGroupID));
        // JVT-S054 (2) (ADD)
        rcSliceHeader.setNumMbsInSlice(rcSliceHeader.getFMO()->getNumMbsInSlice(rcSliceHeader.getFirstMbInSlice(), rcSliceHeader.getLastMbInSlice()));
        UInt uiMbAddress       = rcSliceHeader.getFirstMbInSlice();
        UInt uiLastMbAddress   = rcSliceHeader.getLastMbInSlice();
        UInt uiNumMBInSlice;
        //===== initialization =====
        RNOK( pcMbDataCtrl->initSlice( rcSliceHeader, ENCODE_PROCESS, false, pcMbDataCtrlL1 ) );

        //===== loop over macroblocks =====
        for(  uiNumMBInSlice = 0; uiMbAddress <= uiLastMbAddress;  )
        {
          UInt          uiMbY               = uiMbAddress / m_uiFrameWidthInMb;
          UInt          uiMbX               = uiMbAddress % m_uiFrameWidthInMb;
          MbDataAccess* pcMbDataAccess      = 0;
          MbDataAccess* pcMbDataAccessBase  = 0;

          //===== init macroblock =====
          RNOK  ( pcMbDataCtrl            ->initMb( pcMbDataAccess,     uiMbY, uiMbX ) );
          if    ( pcBaseLayerCtrl )
          {
            RNOK( pcBaseLayerCtrl         ->initMb( pcMbDataAccessBase, uiMbY, uiMbX ) );
          }

          RNOK( pcMbDataCtrl->getMbData(uiMbX, uiMbY).copyMotion( pcBaseLayerCtrl->getMbData(uiMbX, uiMbY), pcMbDataCtrl->getSliceId() ) );
          // <<<< bug fix by heiko.schwarz@hhi.fhg.de
          if( pcBaseLayerFrame ) // the motion data are not just copied, but inferred from the base layer
          {
            pcMbDataCtrl->getMbDataByIndex( uiMbAddress ).getMbMvdData( LIST_0 ).clear();
            pcMbDataCtrl->getMbDataByIndex( uiMbAddress ).getMbMvdData( LIST_1 ).clear();
          }
          // >>>> bug fix by heiko.schwarz@hhi.fhg.de
          uiMbAddress = rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress);
          uiNumMBInSlice++;
        }
      }
    }
    return Err::m_nOK;
  }

  if (m_bIroiSliceDivisionFlag)
  {
    for (UInt uiSliceId=0; uiSliceId <= m_uiNumSliceMinus1; uiSliceId++)
    {
      rcSliceHeader.setFirstMbInSlice(m_puiFirstMbInSlice[uiSliceId]);
      rcSliceHeader.setLastMbInSlice(m_puiLastMbInSlice[uiSliceId]);
      // JVT-S054 (2) (ADD)
      rcSliceHeader.setNumMbsInSlice(rcSliceHeader.getFMO()->getNumMbsInSlice(rcSliceHeader.getFirstMbInSlice(), rcSliceHeader.getLastMbInSlice()));
      UInt uiMbAddress       = rcSliceHeader.getFirstMbInSlice();
      UInt uiLastMbAddress   = rcSliceHeader.getLastMbInSlice();
      UInt uiNumMBInSlice;

      //===== initialization =====
      RNOK( pcMbDataCtrl->initSlice( rcSliceHeader, ENCODE_PROCESS, false, pcMbDataCtrlL1 ) );
      if( ! m_bLoadMotionInfo )
      {
        RNOK( m_pcMotionEstimation->initSlice( rcSliceHeader ) );
        RNOK( pcMbEncoder         ->initSlice( rcSliceHeader ) );
        RNOK( m_pcMotionEstimation->getSW()->initSlice( rcSliceHeader ) );
      }

      //===== loop over macroblocks =====
      //for( UInt uiMbIndex = 0; uiMbIndex < m_uiMbNumber; uiMbIndex++ )
      for(  uiNumMBInSlice = 0; uiMbAddress <= uiLastMbAddress;  )
      {
        UInt          uiMbY               = uiMbAddress / m_uiFrameWidthInMb;
        UInt          uiMbX               = uiMbAddress % m_uiFrameWidthInMb;
        MbDataAccess* pcMbDataAccess      = 0;
        MbDataAccess* pcMbDataAccessBase  = 0;

        //===== init macroblock =====
        RNOK  ( pcMbDataCtrl            ->initMb( pcMbDataAccess,     uiMbY, uiMbX ) );
        if    ( pcBaseLayerCtrl )
        {
          RNOK( pcBaseLayerCtrl         ->initMb( pcMbDataAccessBase, uiMbY, uiMbX ) );
        }

        if( ! m_bLoadMotionInfo )
        {
          //===== initialisation =====
          RNOK( m_pcYuvFullPelBufferCtrl->initMb( uiMbY, uiMbX ) );
          RNOK( m_pcYuvHalfPelBufferCtrl->initMb( uiMbY, uiMbX ) );
          RNOK( m_pcMotionEstimation    ->initMb( uiMbY, uiMbX, *pcMbDataAccess ) );

          RNOK( pcMbEncoder->estimatePrediction ( *pcMbDataAccess,
                            pcMbDataAccessBase,
                            iSpatialScalabilityType,
                            *pcRefFrameList0,
                            *pcRefFrameList1,
                            pcBaseLayerFrame,
                            pcBaseLayerResidual,
                            *pcOrigFrame,
                            *pcIntraRecFrame,
                            bBiPredOnly,
                            uiNumMaxIter,
                            uiIterSearchRange,
                            m_bBLSkipEnable, //JVT-Q065 EIDR
                            rcControlData.getLambda() ) );
          if( m_bSaveMotionInfo )
          {
            //===== save prediction data =====
            RNOK( pcMbDataAccess->getMbData().saveAll( m_pMotionInfoFile ) );
          }
        }
        else
        {
          //===== load prediction data =====
            RNOK( pcMbDataAccess->getMbData().loadAll( m_pMotionInfoFile ) );
        }
        uiMbAddress = rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress);
        uiNumMBInSlice++;
      }
    }
  }
  else
  {
    FMO* pcFMO = rcControlData.getSliceHeader()->getFMO();
    for (UInt iSliceGroupID=0;!pcFMO->SliceGroupCompletelyCoded(iSliceGroupID);iSliceGroupID++)
    {
      rcSliceHeader.setFirstMbInSlice(pcFMO->getFirstMacroblockInSlice(iSliceGroupID));
      rcSliceHeader.setLastMbInSlice(pcFMO->getLastMBInSliceGroup(iSliceGroupID));
      // JVT-S054 (2) (ADD)
      rcSliceHeader.setNumMbsInSlice(rcSliceHeader.getFMO()->getNumMbsInSlice(rcSliceHeader.getFirstMbInSlice(), rcSliceHeader.getLastMbInSlice()));
      UInt uiMbAddress       = rcSliceHeader.getFirstMbInSlice();
      UInt uiLastMbAddress   = rcSliceHeader.getLastMbInSlice();
      UInt uiNumMBInSlice;

      //===== initialization =====
      RNOK( pcMbDataCtrl->initSlice( rcSliceHeader, ENCODE_PROCESS, false, pcMbDataCtrlL1 ) );
      if( ! m_bLoadMotionInfo )
      {
        RNOK( m_pcMotionEstimation->initSlice( rcSliceHeader ) );
        RNOK( pcMbEncoder         ->initSlice( rcSliceHeader ) );
        RNOK( m_pcMotionEstimation->getSW()->initSlice( rcSliceHeader ) );
      }

      //===== loop over macroblocks =====
      //for( UInt uiMbIndex = 0; uiMbIndex < m_uiMbNumber; uiMbIndex++ )
      for(  uiNumMBInSlice = 0; uiMbAddress <= uiLastMbAddress;  )
      {
        UInt          uiMbY               = uiMbAddress / m_uiFrameWidthInMb;
        UInt          uiMbX               = uiMbAddress % m_uiFrameWidthInMb;
        MbDataAccess* pcMbDataAccess      = 0;
        MbDataAccess* pcMbDataAccessBase  = 0;

        //===== init macroblock =====
        RNOK  ( pcMbDataCtrl            ->initMb( pcMbDataAccess,     uiMbY, uiMbX ) );
        if    ( pcBaseLayerCtrl )
        {
          RNOK( pcBaseLayerCtrl         ->initMb( pcMbDataAccessBase, uiMbY, uiMbX ) );
        }

        if( ! m_bLoadMotionInfo )
        {
          //===== initialisation =====
          RNOK( m_pcYuvFullPelBufferCtrl->initMb( uiMbY, uiMbX ) );
          RNOK( m_pcYuvHalfPelBufferCtrl->initMb( uiMbY, uiMbX ) );
          RNOK( m_pcMotionEstimation    ->initMb( uiMbY, uiMbX, *pcMbDataAccess ) );
		  
		  //JVT-U106 Behaviour at slice boundaries{
          if( rcSliceHeader.getBaseLayerId() != MSYS_UINT_MAX )
		        pcMbEncoder->setIntraBLFlag(m_pbIntraBLFlag[uiMbAddress]);
		  //JVT-U106 Behaviour at slice boundaries}

          RNOK( pcMbEncoder->estimatePrediction ( *pcMbDataAccess,
                            pcMbDataAccessBase,
                            iSpatialScalabilityType,
                            *pcRefFrameList0,
                            *pcRefFrameList1,
                            pcBaseLayerFrame,
                            pcBaseLayerResidual,
                            *pcOrigFrame,
                            *pcIntraRecFrame,
                            bBiPredOnly,
                            uiNumMaxIter,
                            uiIterSearchRange,
                            m_bBLSkipEnable, //JVT-Q065 EIDR
                            rcControlData.getLambda() ) );
          if( m_bSaveMotionInfo )
          {
            //===== save prediction data =====
            RNOK( pcMbDataAccess->getMbData().saveAll( m_pMotionInfoFile ) );
          }
        }
        else
        {
          //===== load prediction data =====
            RNOK( pcMbDataAccess->getMbData().loadAll( m_pMotionInfoFile ) );
        }
        uiMbAddress = rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress);
        uiNumMBInSlice++;
      }
    }
  }
/*
  //===== copy motion if non-adaptive prediction =====
  if( ! bEstimateMotion )
  {
    ROF ( pcBaseLayerCtrl )
    RNOK( pcMbDataCtrl->copyMotion( *pcBaseLayerCtrl ) );
    // <<<< bug fix by heiko.schwarz@hhi.fhg.de
    if( pcBaseLayerFrame ) // the motion data are not just copied, but inferred from the base layer
    {
      for( UInt uiIndex = 0; uiIndex < m_uiMbNumber; uiIndex++ )
      {
        pcMbDataCtrl->getMbDataByIndex( uiIndex ).getMbMvdData( LIST_0 ).clear();
        pcMbDataCtrl->getMbDataByIndex( uiIndex ).getMbMvdData( LIST_1 ).clear();
      }
    }
    // >>>> bug fix by heiko.schwarz@hhi.fhg.de
    return Err::m_nOK;
  }


  //===== initialization =====
  if( ! m_bLoadMotionInfo )
  {
    RNOK( m_pcMotionEstimation->initSlice( rcSliceHeader ) );
    RNOK( pcMbEncoder         ->initSlice( rcSliceHeader ) );
    RNOK( m_pcMotionEstimation->getSW()->initSlice( rcSliceHeader ) );
  }


  //===== loop over macroblocks =====
  for( UInt uiMbIndex = 0; uiMbIndex < m_uiMbNumber; uiMbIndex++ )
  {
    UInt          uiMbY               = uiMbIndex / m_uiFrameWidthInMb;
    UInt          uiMbX               = uiMbIndex % m_uiFrameWidthInMb;
    MbDataAccess* pcMbDataAccess      = 0;
    MbDataAccess* pcMbDataAccessBase  = 0;

    //===== init macroblock =====
    RNOK  ( pcMbDataCtrl            ->initMb( pcMbDataAccess,     uiMbY, uiMbX ) );
    if    ( pcBaseLayerCtrl )
    {
      RNOK( pcBaseLayerCtrl         ->initMb( pcMbDataAccessBase, uiMbY, uiMbX ) );
    }

    if( ! m_bLoadMotionInfo )
    {
      //===== initialisation =====
      RNOK( m_pcYuvFullPelBufferCtrl->initMb( uiMbY, uiMbX ) );
      RNOK( m_pcYuvHalfPelBufferCtrl->initMb( uiMbY, uiMbX ) );
      RNOK( m_pcMotionEstimation    ->initMb( uiMbY, uiMbX, *pcMbDataAccess ) );

    RNOK( pcMbEncoder->estimatePrediction ( *pcMbDataAccess,
                        pcMbDataAccessBase,
                        iSpatialScalabilityType,
                        *pcRefFrameList0,
                        *pcRefFrameList1,
                        pcBaseLayerFrame,
                        pcBaseLayerResidual,
                        *pcOrigFrame,
                        *pcIntraRecFrame,
                        bBiPredOnly,
                        uiNumMaxIter,
                        uiIterSearchRange,
                        m_bBLSkipEnable, //JVT-Q065 EIDR
                        rcControlData.getLambda() ) );
      if( m_bSaveMotionInfo )
      {
        //===== save prediction data =====
        RNOK( pcMbDataAccess->getMbData().saveAll( m_pMotionInfoFile ) );
      }
    }
    else
    {
        //===== load prediction data =====
        RNOK( pcMbDataAccess->getMbData().loadAll( m_pMotionInfoFile ) );
    }
  }
*/
// JVT-S054 (REPLACE) <-
  return Err::m_nOK;
}




ErrVal
MCTFEncoder::xMotionCompensation( IntFrame*        pcMCFrame,
                                  RefFrameList*    pcRefFrameList0,
                                  RefFrameList*    pcRefFrameList1,
                                  MbDataCtrl*      pcMbDataCtrl,
                                  SliceHeader&     rcSH, 
                                  Bool             bSR )
{
  Bool        bCalcMv         = false;
  Bool        bFaultTolerant  = false;
  MbEncoder*  pcMbEncoder     = m_pcSliceEncoder->getMbEncoder();

  RNOK( pcMbDataCtrl        ->initSlice( rcSH, PRE_PROCESS, false, NULL ) );
  RNOK( m_pcMotionEstimation->initSlice( rcSH              ) );

  pcMbEncoder->setBaseLayerRec(m_pcBaseLayerFrame);
  MbDataCtrl*      pcBaseMbDataCtrl = getBaseMbDataCtrl();

  for( UInt uiMbIndex = 0; uiMbIndex < m_uiMbNumber; uiMbIndex++ )
  {
    UInt          uiMbY           = uiMbIndex / m_uiFrameWidthInMb;
    UInt          uiMbX           = uiMbIndex % m_uiFrameWidthInMb;
    MbDataAccess* pcMbDataAccess  = 0;
    MbDataAccess* pcMbDataAccessBase  = 0;
    if    ( pcBaseMbDataCtrl )
    {
      RNOK( pcBaseMbDataCtrl         ->initMb( pcMbDataAccessBase, uiMbY, uiMbX ) );
    }


    RNOK( pcMbDataCtrl            ->initMb( pcMbDataAccess, uiMbY, uiMbX                  ) );
    RNOK( m_pcYuvFullPelBufferCtrl->initMb(                 uiMbY, uiMbX ) );
    RNOK( m_pcYuvHalfPelBufferCtrl->initMb(                 uiMbY, uiMbX ) );
    RNOK( m_pcMotionEstimation    ->initMb(                 uiMbY, uiMbX, *pcMbDataAccess ) );

    pcMbDataAccess->setMbDataAccessBase(pcMbDataAccessBase);

    RNOK( pcMbEncoder->compensatePrediction( *pcMbDataAccess, pcMCFrame,
                                             *pcRefFrameList0, *pcRefFrameList1,
                                             bCalcMv, bFaultTolerant, bSR ) );

  }

  return Err::m_nOK;
}


ErrVal
MCTFEncoder::xMotionCompensationSRFrame( IntFrame*        pcSRFrame,
                                         RefFrameList*    pcRefFrameList0,
                                         RefFrameList*    pcRefFrameList1,
                                         MbDataCtrl*      pcMbDataCtrl,
                                         SliceHeader&     rcSH,
                                         MbDataCtrl*      pcBaseMbDataCtrl )
{
  MbEncoder*  pcMbEncoder     = m_pcSliceEncoder->getMbEncoder();

  RNOK( pcMbDataCtrl        ->initSlice( rcSH, PRE_PROCESS, false, NULL ) );
  RNOK( m_pcMotionEstimation->initSlice( rcSH              ) );

  pcSRFrame->getFullPelYuvBuffer()->clear();

  for( UInt uiMbIndex = 0; uiMbIndex < m_uiMbNumber; uiMbIndex++ )
  {
    UInt          uiMbY               = uiMbIndex / m_uiFrameWidthInMb;
    UInt          uiMbX               = uiMbIndex % m_uiFrameWidthInMb;
    MbDataAccess* pcMbDataAccess      = 0;
    MbDataAccess* pcMbDataAccessBase  = 0;

    if( pcBaseMbDataCtrl )
    {
      RNOK( pcBaseMbDataCtrl        ->initMb( pcMbDataAccessBase, uiMbY, uiMbX ) );
    }

    RNOK( pcMbDataCtrl            ->initMb( pcMbDataAccess,     uiMbY, uiMbX     ) );
    RNOK( m_pcYuvFullPelBufferCtrl->initMb(                     uiMbY, uiMbX     ) );
    RNOK( m_pcYuvHalfPelBufferCtrl->initMb(                     uiMbY, uiMbX     ) );
    RNOK( m_pcMotionEstimation    ->initMb(                     uiMbY, uiMbX, *pcMbDataAccess ) );

    pcMbDataAccess->setMbDataAccessBase(pcMbDataAccessBase);

    if( !pcMbDataAccess->getMbData().isIntra() && pcMbDataAccess->getMbData().getBLSkipFlag() ) 
    {
      RNOK( pcMbEncoder->compensateMbSR( *pcMbDataAccess, pcSRFrame, *pcRefFrameList0, *pcRefFrameList1, pcMbDataAccessBase) );
    }
    else
    {
      IntYuvMbBuffer  cYuvMbBuffer;

      cYuvMbBuffer.setAllSamplesToZero();
      RNOK( pcSRFrame->getFullPelYuvBuffer()->loadBuffer( &cYuvMbBuffer ) );
    }
  }

  return Err::m_nOK;
}


ErrVal
MCTFEncoder::xGetConnections( Double&       rdL0Rate,
                              Double&       rdL1Rate,
                              Double&       rdBiRate )
{
  //=== just a guess ===
  if( m_bHaarFiltering )
  {
    rdL0Rate = 1.0;
    rdL1Rate = 0.0;
    rdBiRate = 0.0;
  }
  else if( m_bBiPredOnly )
  {
    rdL0Rate = 0.0;
    rdL1Rate = 0.0;
    rdBiRate = 1.0;
  }
  else
  {
    rdL0Rate = 0.2;
    rdL1Rate = 0.2;
    rdBiRate = 0.6;
  }
  return Err::m_nOK;
}




ErrVal
MCTFEncoder::xZeroIntraMacroblocks( IntFrame*    pcFrame,
                                   ControlData& rcCtrlData )
{
  MbDataCtrl*       pcMbDataCtrl  = rcCtrlData.getMbDataCtrl       ();
  SliceHeader*      pcSliceHeader = rcCtrlData.getSliceHeader      ();
  IntYuvPicBuffer*  pcPicBuffer   = pcFrame  ->getFullPelYuvBuffer ();

  RNOK( pcMbDataCtrl->initSlice( *pcSliceHeader, PRE_PROCESS, false, NULL ) );

  IntYuvMbBuffer cZeroMbBuffer;
  cZeroMbBuffer.setAllSamplesToZero();

  for( UInt uiMbIndex = 0; uiMbIndex < m_uiMbNumber; uiMbIndex++ )
  {
    UInt          uiMbY           = uiMbIndex / m_uiFrameWidthInMb;
    UInt          uiMbX           = uiMbIndex % m_uiFrameWidthInMb;
    MbDataAccess* pcMbDataAccess  = 0;

    RNOK( pcMbDataCtrl            ->initMb( pcMbDataAccess, uiMbY, uiMbX ) );
    RNOK( m_pcYuvFullPelBufferCtrl->initMb(                 uiMbY, uiMbX ) );

    if( pcMbDataAccess->getMbData().isIntra() )
    {
      pcPicBuffer->loadBuffer( &cZeroMbBuffer );
    }
  }

  return Err::m_nOK;
}



ErrVal
MCTFEncoder::xClipIntraMacroblocks( IntFrame*    pcFrame,
                                   ControlData& rcCtrlData, Bool bClipAll )
{
  MbDataCtrl*       pcMbDataCtrl  = rcCtrlData.getMbDataCtrl       ();
  SliceHeader*      pcSliceHeader = rcCtrlData.getSliceHeader      ();
  IntYuvPicBuffer*  pcPicBuffer   = pcFrame  ->getFullPelYuvBuffer ();
  IntYuvMbBuffer    cMbBuffer;

  RNOK( pcMbDataCtrl->initSlice( *pcSliceHeader, PRE_PROCESS, false, NULL ) );

  for( UInt uiMbIndex = 0; uiMbIndex < m_uiMbNumber; uiMbIndex++ )
  {
    UInt          uiMbY           = uiMbIndex / m_uiFrameWidthInMb;
    UInt          uiMbX           = uiMbIndex % m_uiFrameWidthInMb;
    MbDataAccess* pcMbDataAccess  = 0;

    RNOK( pcMbDataCtrl            ->initMb( pcMbDataAccess, uiMbY, uiMbX ) );
    RNOK( m_pcYuvFullPelBufferCtrl->initMb(                 uiMbY, uiMbX ) );

    if( bClipAll || pcMbDataAccess->getMbData().isIntra() )
    {
      cMbBuffer   .loadBuffer( pcPicBuffer );
      cMbBuffer   .clip      ();
      pcPicBuffer->loadBuffer( &cMbBuffer );
    }
  }

  return Err::m_nOK;
}


ErrVal
MCTFEncoder::xInitExtBinDataAccessor( ExtBinDataAccessor& rcExtBinDataAccessor )
{
  ROF( m_pucWriteBuffer );
  m_cBinData.reset          ();
  m_cBinData.set            ( m_pucWriteBuffer, m_uiWriteBufferSize );
  m_cBinData.setMemAccessor ( rcExtBinDataAccessor );

  return Err::m_nOK;
}


ErrVal
MCTFEncoder::xAppendNewExtBinDataAccessor( ExtBinDataAccessorList& rcExtBinDataAccessorList,
                                          ExtBinDataAccessor*     pcExtBinDataAccessor,
                                          Bool                    bModifyDataAccessor )
{
  ROF( pcExtBinDataAccessor );
  ROF( pcExtBinDataAccessor->data() );

  UInt    uiNewSize     = pcExtBinDataAccessor->size();
  UChar*  pucNewBuffer  = new UChar [ uiNewSize ];
  ROF( pucNewBuffer );
  ::memcpy( pucNewBuffer, pcExtBinDataAccessor->data(), uiNewSize * sizeof( UChar ) );

  ExtBinDataAccessor* pcNewExtBinDataAccessor = new ExtBinDataAccessor;
  ROF( pcNewExtBinDataAccessor );

  m_cBinData              .reset          ();
  m_cBinData              .set            (  pucNewBuffer, uiNewSize );
  m_cBinData              .setMemAccessor ( *pcNewExtBinDataAccessor );
  rcExtBinDataAccessorList.push_back      (  pcNewExtBinDataAccessor );

  m_cBinData              .reset          ();
  ROFRS( bModifyDataAccessor, Err::m_nOK );

  m_cBinData              .setMemAccessor ( *pcExtBinDataAccessor );

  return Err::m_nOK;
}



ErrVal
MCTFEncoder::xAddBaseLayerResidual( ControlData& rcControlData,
                                    IntFrame*    pcFrame,
                                    Bool         bSubtract )
{
  ROFRS( rcControlData.getBaseLayerSbb(), Err::m_nOK );

  MbDataCtrl*       pcMbDataCtrl  = rcControlData.getMbDataCtrl       ();
  SliceHeader*      pcSliceHeader = rcControlData.getSliceHeader      ();
  IntYuvPicBuffer*  pcBLResidual  = rcControlData.getBaseLayerSbb     ()->getFullPelYuvBuffer();
  IntYuvPicBuffer*  pcPicBuffer   = pcFrame     ->getFullPelYuvBuffer ();
  IntYuvMbBuffer    cBLResBuffer;
  IntYuvMbBuffer    cMbBuffer;

  RNOK( pcMbDataCtrl->initSlice( *pcSliceHeader, PRE_PROCESS, false, NULL ) );

  for( UInt uiMbIndex = 0; uiMbIndex < m_uiMbNumber; uiMbIndex++ )
  {
    UInt          uiMbY           = uiMbIndex / m_uiFrameWidthInMb;
    UInt          uiMbX           = uiMbIndex % m_uiFrameWidthInMb;
    MbDataAccess* pcMbDataAccess  = 0;

    RNOK( pcMbDataCtrl            ->initMb( pcMbDataAccess, uiMbY, uiMbX ) );
    RNOK( m_pcYuvFullPelBufferCtrl->initMb(                 uiMbY, uiMbX ) );

    if( ! pcMbDataAccess->getMbData().isIntra() && pcMbDataAccess->getMbData().getResidualPredFlag( PART_16x16 ) )
    {
      cMbBuffer   .loadBuffer( pcPicBuffer  );
      cBLResBuffer.loadBuffer( pcBLResidual );

      if( bSubtract ) cMbBuffer.subtract( cBLResBuffer );
      else            cMbBuffer.add     ( cBLResBuffer );

      pcPicBuffer->loadBuffer( &cMbBuffer );
    }
  }

  return Err::m_nOK;
}

ErrVal
MCTFEncoder::xEncodeFGSLayer( ExtBinDataAccessorList& rcOutExtBinDataAccessorList,
                              ControlData&            rcControlData,
                              IntFrame*               pcFrame,      // <- orig   -> rec frame
                              IntFrame*               pcResidual,   //           -> rec residual
                              IntFrame*               pcPredSignal, // <- pred
                              IntFrame*               pcTempFrame,
                              IntFrame*               pcSubband,
                              IntFrame*               pcCLRec,
                              UInt                    uiFrameIdInGOP,
                              IntFrame*               pcOrgFrame,
                              IntFrame*               pcHighPassPredSignal,
                              RefFrameList&           rcRefFrameList0,
                              RefFrameList&           rcRefFrameList1,
                              UInt&                   ruiBits,
                              PicOutputDataList&      rcPicOutputDataList )
{
  Bool          bFinished     = false;
  SliceHeader*  pcSliceHeader = rcControlData.getSliceHeader();
  IntFrame*     pcOrgResidual = pcTempFrame;
  UInt          uiRealBLBits  = ruiBits;
  UInt          uiLastRecLayer = 0;
  ruiBits                     = 0;
  SliceType     eBaseSliceType = pcSliceHeader->getSliceType();
  IntFrame      *pcRecTemp = m_apcFrameTemp[2];

  pcSliceHeader->setFGSCodingMode( m_pcSPS_FGS->getFGSCodingMode( 0 ) );
  pcSliceHeader->setGroupingSize ( m_pcSPS_FGS->getGroupingSize ( 0 ) );
  UInt ui = 0;
  for(ui = 0; ui < 16; ui++)
  {
    pcSliceHeader->setPosVect( ui, m_pcSPS_FGS->getPosVect(0, ui) );
  }
  RNOK( pcSliceHeader->checkPosVectors() );
  pcSliceHeader->setFGSCycleAlignedFragment ( m_pcSPS_FGS->getFGSCycleAlignedFragment() );

  //save PPSId
  UInt uiPPSId = pcSliceHeader->getPicParameterSetId();
  Bool bUseBaseRep = pcSliceHeader->getUseBasePredictionFlag();
  //update PPSId for enhancement FGS nal unit
  pcSliceHeader->setPicParameterSetId(bUseBaseRep ? m_pcPPSLP_FGS->getPicParameterSetId() : m_pcPPSHP_FGS->getPicParameterSetId());

  // Martin.Winken@hhi.fhg.de: original residual now set in RQFGSEncoder::xResidualTranform()

  UInt uiTarget                   =   0;
  UInt uiFGSMaxBits = 0;
  UInt uiBaseBits                 =   0;
  UInt uiEstimatedHeaderBits      =   0;
  UInt uiFGSBits [MAX_FGS_LAYERS] = { 0, 0, 0 };
  Double  dRealValuedFGSBits          = 0.0;
  //JVT-P031
  Bool bAlreadyReconstructed = false;
  UInt uiPredFGSMaxBits = 0;
  UInt uiPredTarget = 0;
  Bool bFinishedFrag = false;
  Bool bSetToDiscardable = false;
  Bool bLastFragToCode = false;
  UInt uiFGSCutBits = 0;
  Double  dPredRealValuedFGSBits          = 0.0;
  if(m_bUseDiscardableUnit || m_uiFGSMode == 2)
  //~JVT-P031
  {
    Char  acLine    [1000];
    Int   i, c;
    for(  i = 0; ( c = fgetc(m_pFGSFile), ( c != '\n' && c != EOF ) ); acLine[i++] = c );
    acLine[i] = '\0';
    ROT( feof(m_pFGSFile) );

    UInt uiDummy;
    sscanf( acLine, "%d %d %d %d %d %d %d ",
      &uiBaseBits, &uiEstimatedHeaderBits, &uiFGSBits[0], &uiDummy, &uiFGSBits[1], &uiDummy, &uiFGSBits[2] );
    uiEstimatedHeaderBits = uiFGSBits[0] - uiEstimatedHeaderBits;

    //FIX_FRAG_CAVLC
    if(m_uiFGSMode == 2)
    {
      //~FIX_FRAG_CAVLC
    if( m_dFGSBitRateFactor == 0.0 ) // target rate lies inside the range of the analysis run
    {
      ROF( m_dFGSCutFactor > 0 && m_dFGSCutFactor < 3 );

      dRealValuedFGSBits   += (Double)uiBaseBits;
      Double    dFactor     = m_dFGSCutFactor;
      for( UInt uiFGSLayer  = 0; dFactor > 0; uiFGSLayer++ )
      {
        dRealValuedFGSBits += (Double)uiFGSBits[uiFGSLayer] * min( dFactor, 1.0 );
        dFactor            -= 1.0;
      }
    }
    else
    {
      dRealValuedFGSBits    = uiBaseBits + ( m_dFGSCutFactor > 0 ? uiFGSBits[0] + uiFGSBits[1] + uiFGSBits[2] : 0 );
      dRealValuedFGSBits   *= m_dFGSBitRateFactor;
    }

    dRealValuedFGSBits     += m_dFGSRoundingOffset + 0.0000001;
    uiFGSMaxBits            = (UInt)floor( dRealValuedFGSBits );
    m_dFGSRoundingOffset    = dRealValuedFGSBits - (Double)uiFGSMaxBits;

    //----- correct base layer bits -----#
    if( uiFGSMaxBits >= uiRealBLBits )
    {
      uiFGSMaxBits -= uiRealBLBits;
    }
    else
    {
      m_iLastFGSError += uiRealBLBits - uiFGSMaxBits;
      uiFGSMaxBits     = 0;
    }
    //----- consider last FGS errors -----
    if( (Int)uiFGSMaxBits >= m_iLastFGSError )
    {
      uiFGSMaxBits   -= m_iLastFGSError;
      m_iLastFGSError = 0;
    }
    else
    {
      m_iLastFGSError -= (Int)uiFGSMaxBits;
      uiFGSMaxBits     = 0;
    }
    uiTarget = uiFGSMaxBits;
    }//FIX_FRAG_CAVLC
//JVT-P031
    if(m_bUseDiscardableUnit)
    {
    if( m_dPredFGSBitRateFactor == 0.0 ) // target rate lies inside the range of the analysis run
    {
      ROF( m_dPredFGSCutFactor > 0 && m_dPredFGSCutFactor < 3 );

      dPredRealValuedFGSBits   += (Double)uiBaseBits;
      Double    dPredFactor     = m_dPredFGSCutFactor;
      for( UInt uiFGSLayer  = 0; dPredFactor > 0; uiFGSLayer++ )
      {
        dPredRealValuedFGSBits += (Double)uiFGSBits[uiFGSLayer] * min( dPredFactor, 1.0 );
        dPredFactor            -= 1.0;
      }
    }
    else
    {
      dPredRealValuedFGSBits    = uiBaseBits + ( m_dPredFGSCutFactor > 0 ? uiFGSBits[0] + uiFGSBits[1] + uiFGSBits[2] : 0 );
      dPredRealValuedFGSBits   *= m_dPredFGSBitRateFactor;
    }

    dPredRealValuedFGSBits     += m_dPredFGSRoundingOffset + 0.0000001;
    uiPredFGSMaxBits            = (UInt)floor( dPredRealValuedFGSBits );
    m_dPredFGSRoundingOffset    = dPredRealValuedFGSBits - (Double)uiPredFGSMaxBits;
   //----- correct base layer bits -----#
    if( uiPredFGSMaxBits >= uiRealBLBits )
    {
      uiPredFGSMaxBits -= uiRealBLBits;
    }
    else
    {
      m_iPredLastFGSError += uiRealBLBits - uiPredFGSMaxBits;
      uiPredFGSMaxBits     = 0;
    }
    //----- consider last FGS errors -----
    if( (Int)uiPredFGSMaxBits >= m_iPredLastFGSError )
    {
      uiPredFGSMaxBits   -= m_iPredLastFGSError;
      m_iPredLastFGSError = 0;
    }
    else
    {
      m_iPredLastFGSError -= (Int)uiPredFGSMaxBits;
      uiPredFGSMaxBits     = 0;
    }
    uiPredTarget = uiPredFGSMaxBits;
    }
    //~JVT-P031
  }

  //===== initialize FGS encoder =====
  pcSliceHeader->setAdaptivePredictionFlag( ! pcSliceHeader->isIntra() &&
                                            ( m_uiFGSMotionMode >  1 ||
                                              m_uiFGSMotionMode == 1 && ! pcSliceHeader->getUseBasePredictionFlag() ) );
  RNOK( m_pcRQFGSEncoder->initPicture( rcControlData.getSliceHeader(),
                                       rcControlData.getMbDataCtrl(),
                                       pcOrgResidual,
                                       // FGS_MOTION {
                                       pcOrgFrame,
                                       pcHighPassPredSignal ? pcHighPassPredSignal : pcPredSignal,
                                       &rcRefFrameList0,
                                       &rcRefFrameList1,
                                       m_uiNumMaxIter,
                                       m_uiIterSearchRange,
                                       // } FGS_MOTION
                                       m_dNumFGSLayers,
                                       rcControlData.getLambda(),
                                       m_iMaxDeltaQp,
                                       bFinished,
                                       ( m_uiFGSMode == 2 ),
                                       m_bUseDiscardableUnit ) );
  RNOK( xAddBaseLayerResidual( rcControlData, m_pcRQFGSEncoder->getBaseLayerSbb(), false ) );

  // HS: bug-fix for m_uiQualityLevelForPrediction == 0
  if( 0 == m_uiQualityLevelForPrediction && !bAlreadyReconstructed)
  {
    RNOK( m_pcRQFGSEncoder->reconstruct   ( pcRecTemp ) );
    RNOK( pcResidual      ->copy          ( pcRecTemp ) );
    RNOK( xZeroIntraMacroblocks           ( pcResidual, rcControlData ) );
    RNOK( pcRecTemp         ->add           ( pcPredSignal ) );
    RNOK( xClipIntraMacroblocks   ( pcRecTemp, rcControlData, pcSliceHeader->getTemporalLevel() == 0 ) );
    RNOK( pcSubband       ->copy          ( pcRecTemp ) );
    bAlreadyReconstructed = true;

    RNOK( rcControlData.saveMbDataQpAndCbp() );
  }

  if( m_uiFGSMode == 2 && uiTarget < 2*uiEstimatedHeaderBits ) // the payload should be at least so big as the header
  {
    bFinished        = true;
    m_iLastFGSError -= (Int)uiFGSMaxBits;
  }

  //JVT-P031
  if( m_uiFGSMode == 2 && m_bUseDiscardableUnit && uiPredTarget < 2*uiEstimatedHeaderBits ) // the payload should be at least so big as the header
  {
    bSetToDiscardable = true;
    m_iPredLastFGSError -= (Int)uiPredFGSMaxBits;
  }
  //~JVT-P031

  //===== encoding of FGS packets =====
  for( UInt uiRecLayer = 1; !bFinished; uiRecLayer++ )
  {
    UInt  auiPDFragBits[16];
    UInt  uiNumPDFrags = 0;
    UInt  uiPacketBits  = 0;
    Bool bBaseLayerKeyPic = pcSliceHeader->getTemporalLevel() == 0;;
    RefFrameList  cRefListDiff;

    pcSliceHeader->setArFgsUsageFlag              ( bBaseLayerKeyPic );
    pcSliceHeader->setFgsEntropyOrderFlag(0);

    if( eBaseSliceType == P_SLICE && bBaseLayerKeyPic)
    {
      pcSliceHeader->setBaseWeightZeroBaseBlock ( m_uiBaseWeightZeroBaseBlock );
      pcSliceHeader->setBaseWeightZeroBaseCoeff ( m_uiBaseWeightZeroBaseCoeff );
      pcSliceHeader->setLowPassFgsMcFilter      ( m_uiLowPassFgsMcFilter      );
      pcSliceHeader->setFgsEntropyOrderFlag     ( m_uiFgsEncStructureFlag == 0 && uiRecLayer != 1);

      if(uiRecLayer == 1)
      {
        m_aapcFGSPredFrame->copy(pcPredSignal);
        rcControlData.getMbDataCtrl()->storeFgsBQLayerQpAndCbp();
        m_pcRQFGSEncoder->xStoreBQLayerSigMap();
      }

      IntFrame *pcLowPassRefFrameBase, *pcLowPassRefFrameEnh;

      if(m_uiFgsEncStructureFlag == 0)
      {
        // use the discrete base layer and the current layer as 2 references
        pcLowPassRefFrameBase = m_aapcFGSRecon[0][0];
        pcLowPassRefFrameEnh  = (uiRecLayer < m_uiNumLayers[0])
          ? m_aapcFGSRecon[0][uiRecLayer] : m_aapcFGSRecon[0][m_uiNumLayers[0] - 1];
      }
      else
      {
        // use the discrete base layer and the top layer as 2 references
        pcLowPassRefFrameBase = m_aapcFGSRecon[0][0];
        pcLowPassRefFrameEnh  = m_aapcFGSRecon[0][1];
      }

      setDiffPrdRefLists( cRefListDiff, pcLowPassRefFrameBase, pcLowPassRefFrameEnh, m_pcYuvFullPelBufferCtrl );
      RNOK( m_pcRQFGSEncoder->initArFgs( m_aapcFGSPredFrame, &cRefListDiff ) );

      m_pcMotionEstimation->loadAdaptiveRefPredictors(
        m_pcYuvFullPelBufferCtrl,
        pcPredSignal, m_aapcFGSPredFrame,
        &cRefListDiff,
        rcControlData.getMbDataCtrl(),
        m_pcRQFGSEncoder,
        rcControlData.getSliceHeader());
    }
    if( bBaseLayerKeyPic )
    {
      if(m_uiFgsEncStructureFlag == 0)
        m_uiNumLayers[1] = uiRecLayer + 1;
      else
        m_uiNumLayers[1] = 2;
    }

    //JVT-P031
    UInt uiFrac = 0;
    UInt uiFragmentBits;
  // Currently, fragmented NAL units are only designed to truncate correctly the
  // progressive refinement slices NAL units
  // Thus, only two fragments are created, the first containing the part of the
  // progressive refinement slice NAL unit that is retained for the target bit-rate
    bFinishedFrag = false;
    while(!bFinishedFrag)
    {
      uiFragmentBits = 0;
      //~JVT-P031

    //JVT-P031
    // set fragment status
      Bool bFragmented = ( m_bUseDiscardableUnit && ((!bSetToDiscardable && (uiPredTarget < uiFGSBits[uiRecLayer-1] - uiEstimatedHeaderBits)) || uiFrac!=0) );

      if( pcSliceHeader->getFGSCycleAlignedFragment() && pcSliceHeader->getPosVect( 0 ) < 16 )
        bFragmented = true;

      if(bFragmented)
      {
        pcSliceHeader->setFragmentedFlag(true);
        pcSliceHeader->setFragmentOrder(uiFrac);
        // only 2 fragments
        pcSliceHeader->setLastFragmentFlag((uiFrac==0) ? false : true);
        if(uiFrac != 0)
        {
            pcSliceHeader->setDiscardableFlag(true);
            bSetToDiscardable = true;
        }
      }
      else
      {
        pcSliceHeader->setFragmentedFlag(false);
        pcSliceHeader->setFragmentOrder(0);
        pcSliceHeader->setLastFragmentFlag(true);
         if(bSetToDiscardable) //JVT-T054
            pcSliceHeader->setDiscardableFlag(true);
      }
      //~JVT-P031

    FMO* pcFMO = pcSliceHeader->getFMO();
    m_pcRQFGSEncoder->prepareEncode(uiFrac,uiFrac);

    Int iQp = 0;

    // JVT-S054 (ADD) ->
    if (m_bIroiSliceDivisionFlag)
    {
      for (UInt uiSliceId=0; uiSliceId <= m_uiNumSliceMinus1; uiSliceId++)
      {
        pcSliceHeader->setFirstMbInSlice(m_puiFirstMbInSlice[uiSliceId]);
        pcSliceHeader->setLastMbInSlice(m_puiLastMbInSlice[uiSliceId]);
        // JVT-S054 (2) (ADD)
        pcSliceHeader->setNumMbsInSlice(pcSliceHeader->getFMO()->getNumMbsInSlice(pcSliceHeader->getFirstMbInSlice(), pcSliceHeader->getLastMbInSlice()));

        //----- init NAL UNIT -----
        RNOK( xInitExtBinDataAccessor               (  m_cExtBinDataAccessor ) );
        RNOK( m_pcNalUnitEncoder->initNalUnit       ( &m_cExtBinDataAccessor ) );

        //---- write Slice Header -----
        ETRACE_NEWSLICE;
        //pcSliceHeader->setQualityLevel       ( uiRecLayer );
        xAssignSimplePriorityId( pcSliceHeader );

        RNOK( m_pcNalUnitEncoder->write  ( *pcSliceHeader ) );

        iQp = pcSliceHeader->getPicQp();
        uiLastRecLayer = uiRecLayer;

        //---- encode next bit-plane for current NAL unit ----

        uiFGSCutBits = (bFragmented && uiFrac == 0 || !bFragmented && m_bUseDiscardableUnit && !bSetToDiscardable  ) ? uiPredFGSMaxBits : uiFGSMaxBits; //JVT-P031

        Bool bCorrupted = false;

        //JVT-P031
        if(m_uiFGSMode == 2 && uiFGSCutBits < uiFGSBits[uiRecLayer-1] - uiEstimatedHeaderBits) // probably truncate in payload
        {
           uiFGSCutBits -= uiEstimatedHeaderBits;
        }
        RNOK( m_pcRQFGSEncoder->encodeNextLayer     ( bFinished, bCorrupted, uiFGSCutBits, auiPDFragBits, uiNumPDFrags, uiFrac, bFragmented, ( m_uiFGSMode == 1 && !m_bUseDiscardableUnit ? m_pFGSFile : 0 ) ) ); //FIX_FRAG_CAVLC

        //--ICU/ETRI
        if(!bFinished) RNOK( m_pcRQFGSEncoder->updateQP(bCorrupted, bFinished,bFragmented, uiFrac, (uiSliceId == m_uiNumSliceMinus1) ));

        //FIX_FRAG_CAVLC
        if(bFragmented && uiFrac == 0 && bCorrupted == false) // first fragment ends exactly at the end of a nal unit
        {
          bFinishedFrag = true;
        }
        //~FIX_FRAG_CAVLC
        if(uiFrac != 0)
        {
          bFinishedFrag = true;
        }
        //~JVT-P031

        //----- close NAL UNIT -----
        RNOK( m_pcNalUnitEncoder->closeNalUnit      ( uiFragmentBits ) ); //JVT-P031

        RNOK( xAppendNewExtBinDataAccessor          ( rcOutExtBinDataAccessorList, &m_cExtBinDataAccessor ) );
      }
    }
    else
    {
    // JVT-S054 (ADD) <-
      for(Int iSliceGroupID=0;!pcFMO->SliceGroupCompletelyCoded(iSliceGroupID);iSliceGroupID++)
      {
        //----- init NAL UNIT -----
        RNOK( xInitExtBinDataAccessor               (  m_cExtBinDataAccessor ) );
        RNOK( m_pcNalUnitEncoder->initNalUnit       ( &m_cExtBinDataAccessor ) );

        //---- write Slice Header -----
        ETRACE_NEWSLICE;
        //pcSliceHeader->setQualityLevel       ( uiRecLayer );
        xAssignSimplePriorityId( pcSliceHeader );

        //--ICU/ETRI FMO Implementation 1206
        m_pcRQFGSEncoder->setSliceGroup(iSliceGroupID);

        RNOK( m_pcNalUnitEncoder->write  ( *pcSliceHeader ) );

        iQp = pcSliceHeader->getPicQp();
        uiLastRecLayer = uiRecLayer;

        //---- encode next bit-plane for current NAL unit ----

        uiFGSCutBits = (bFragmented && uiFrac == 0 || !bFragmented && m_bUseDiscardableUnit && !bSetToDiscardable  ) ? uiPredFGSMaxBits : uiFGSMaxBits; //JVT-P031

        Bool bCorrupted = false;

        //JVT-P031
        if(m_uiFGSMode == 2 && uiFGSCutBits < uiFGSBits[uiRecLayer-1] - uiEstimatedHeaderBits) // probably truncate in payload
        {
           uiFGSCutBits -= uiEstimatedHeaderBits;
        }
        RNOK( m_pcRQFGSEncoder->encodeNextLayer     ( bFinished, bCorrupted, uiFGSCutBits, auiPDFragBits, uiNumPDFrags, uiFrac, bFragmented, ( m_uiFGSMode == 1 && !m_bUseDiscardableUnit ? m_pFGSFile : 0 ) ) ); //FIX_FRAG_CAVLC

        //--ICU/ETRI
        if(!bFinished) RNOK( m_pcRQFGSEncoder->updateQP(bCorrupted, bFinished,bFragmented, uiFrac, pcFMO->SliceGroupCompletelyCoded(iSliceGroupID+1) != 0));

        //FIX_FRAG_CAVLC
        if(bFragmented && uiFrac == 0 && bCorrupted == false) // first fragment ends exactly at the end of a nal unit
        {
          bFinishedFrag = true;
        }
        //~FIX_FRAG_CAVLC
        if(uiFrac != 0)
        {
          bFinishedFrag = true;
        }
        //~JVT-P031

        //----- close NAL UNIT -----
        // this is temporary solution, does not work with discardable streams
        if( uiNumPDFrags > 1 )
        {
          UInt uiFragIdx;
          Bool bLastFrag;

          RNOK( m_pcNalUnitEncoder->terminateMultiFragments ( auiPDFragBits[uiNumPDFrags] ) );
          for( uiFragIdx = 0; uiFragIdx < uiNumPDFrags; uiFragIdx ++ )
          {
            bLastFrag = (uiFragIdx == uiNumPDFrags - 1);

            if( uiFragIdx > 0 )
            {
              RNOK( xInitExtBinDataAccessor               (  m_cExtBinDataAccessor ) );
              RNOK( m_pcNalUnitEncoder->initNalUnit       ( &m_cExtBinDataAccessor ) );

              // need to write the slice header for the fragment
              pcSliceHeader->setFragmentedFlag            ( true );
              pcSliceHeader->setFragmentOrder             ( uiFragIdx );
              pcSliceHeader->setLastFragmentFlag          ( bLastFrag );

              // was incremented inside the FGS coder
              if( ! bFinished )
                pcSliceHeader->setQualityLevel              ( pcSliceHeader->getQualityLevel() - 1 );
              RNOK( m_pcNalUnitEncoder->write             ( *pcSliceHeader ) );
              if( ! bFinished )
                pcSliceHeader->setQualityLevel              ( pcSliceHeader->getQualityLevel() + 1 );
            }

            RNOK( m_pcNalUnitEncoder->attachFragmentData  ( uiFragmentBits, auiPDFragBits[uiFragIdx]/8, auiPDFragBits[uiFragIdx + 1]/8 ) );
            RNOK( xAppendNewExtBinDataAccessor            ( rcOutExtBinDataAccessorList, &m_cExtBinDataAccessor ) );

            uiPacketBits += 4*8 + uiFragmentBits;
          }
        }
        else
        {
          RNOK( m_pcNalUnitEncoder->closeNalUnit      ( uiFragmentBits ) ); //JVT-P031
          RNOK( xAppendNewExtBinDataAccessor          ( rcOutExtBinDataAccessorList, &m_cExtBinDataAccessor ) );

          uiPacketBits += 4*8 + uiFragmentBits;
        }

        if(!m_bUseDiscardableUnit || ((bFragmented && uiFrac == 1) || (!bFragmented && !bSetToDiscardable)))
            ruiBits += uiPacketBits;
      }  // end for SG maybe this or LN 2059 (before JVT-P031)
    // JVT-S054 (ADD)
    }

    uiFrac++;

    if( bBaseLayerKeyPic && m_uiFgsEncStructureFlag == 0 )
    {
      IntFrame *pcTempFgsFrame = m_aapcFGSRecon[1][uiRecLayer];

      RNOK( m_pcRQFGSEncoder->reconstruct   ( pcTempFgsFrame ) );

      RNOK( pcTempFgsFrame  ->add           ( pcPredSignal ) );
      RNOK( xClipIntraMacroblocks           ( pcTempFgsFrame, rcControlData, true ) );
    }

    if((m_bUseDiscardableUnit && !bSetToDiscardable) )//|| (!bFragmented && !m_bUseDiscardableUnit ))
    {
      RNOK( m_pcRQFGSEncoder->reconstruct   ( pcRecTemp ) );
      RNOK( pcResidual      ->copy          ( pcRecTemp ) );
      RNOK( xZeroIntraMacroblocks           ( pcResidual, rcControlData ) );
      RNOK( pcRecTemp         ->add           ( pcPredSignal ) );
      RNOK( xClipIntraMacroblocks   ( pcRecTemp, rcControlData, pcSliceHeader->getTemporalLevel() == 0 ) );
      RNOK( pcSubband       ->copy          ( pcRecTemp ) );
      bAlreadyReconstructed = true;

      RNOK( rcControlData.saveMbDataQpAndCbp() );
    }
    //~JVT-P031

    //JVT-P031
    if( m_uiFGSMode == 2 )
      {
        if( uiRecLayer == 3 || uiFragmentBits >= uiTarget || bFinished )
        {
           m_iLastFGSError += (Int)uiFragmentBits - (Int)uiTarget;
           bFinished = true;
           bFinishedFrag = true;
        }
        else
        {
          uiTarget     -= uiFragmentBits;
          uiFGSMaxBits  = uiTarget;
          if( uiTarget < 2*uiEstimatedHeaderBits ) // the payload should be at least so big as the header
          {
             bFinished        = true;
             m_iLastFGSError -= (Int)uiFGSMaxBits;
             bFinishedFrag = true;
          }
        }
      }

      if( m_bUseDiscardableUnit && !bSetToDiscardable )
      {
        if( uiRecLayer == 3 || uiFragmentBits >= uiPredTarget )
        {
           m_iPredLastFGSError += (Int)uiFragmentBits - (Int)uiPredTarget;
           bSetToDiscardable = true;
           bLastFragToCode = true;
           if( m_uiFGSMode == 2 && (uiTarget < 2*uiEstimatedHeaderBits) )//FIX_FRAG_CAVLC
          {
             bFinishedFrag = true;
          }
        }
        else
        {
          uiPredTarget     -= uiFragmentBits;
          uiPredFGSMaxBits  = uiPredTarget;
          if( uiPredTarget < 2*uiEstimatedHeaderBits ) // the payload should be at least so big as the header
          {
             bSetToDiscardable = true;
             m_iPredLastFGSError -= (Int)uiPredFGSMaxBits;
          }
        }
      }

      PicOutputData cPicOutputData;
      cPicOutputData.FirstPicInAU  = false;
      cPicOutputData.Poc           = rcControlData.getSliceHeader()->getPoc           ();
      cPicOutputData.FrameType[0]  = 'P';
      cPicOutputData.FrameType[1]  = 'R';
      cPicOutputData.FrameType[2]  = '\0';
      cPicOutputData.DependencyId  = rcControlData.getSliceHeader()->getLayerId       ();
      cPicOutputData.QualityId     = uiRecLayer;
      cPicOutputData.TemporalId    = rcControlData.getSliceHeader()->getTemporalLevel ();
      cPicOutputData.Qp            = iQp; 
      cPicOutputData.Bits          = uiPacketBits;
      cPicOutputData.YPSNR         = 0.0;
      cPicOutputData.UPSNR         = 0.0;
      cPicOutputData.VPSNR         = 0.0;
      rcPicOutputDataList.push_back( cPicOutputData );

    //S051{
    if(m_uiAnaSIP>0)
      m_auiFrameBits[rcControlData.getSliceHeader()->getPoc()]+=uiPacketBits;
    //S051}

      if(!bFragmented)
          break;
    }
    RNOK( freeDiffPrdRefLists( cRefListDiff ) );
    //~JVT-P031
    if( m_uiFGSMode == 1 && m_pFGSFile && !m_bUseDiscardableUnit ) //FIX_FRAG_CAVLC
    {
      fprintf( m_pFGSFile, "\t%d", uiPacketBits );
    }

    m_auiCurrGOPBits[m_uiScalableLayerId + uiRecLayer] += uiPacketBits;
    if( uiRecLayer == m_uiQualityLevelForPrediction && !bAlreadyReconstructed) //JVT-P031
    {
      RNOK( m_pcRQFGSEncoder->reconstruct   ( pcRecTemp ) );
      RNOK( pcResidual      ->copy          ( pcRecTemp ) );
      RNOK( xZeroIntraMacroblocks           ( pcResidual, rcControlData ) );
      RNOK( pcRecTemp         ->add           ( pcPredSignal ) );
      RNOK( xClipIntraMacroblocks           ( pcRecTemp, rcControlData, pcSliceHeader->getTemporalLevel() == 0 ) );
      RNOK( pcSubband       ->copy          ( pcRecTemp ) );
      bAlreadyReconstructed = true;
//JVT-T054{
     bSetToDiscardable = true;
//JVT-T054}
      RNOK( rcControlData.saveMbDataQpAndCbp() );
    }
  }

  // ICU/ETRI FGS_MOT_USE
  // 2006.10.02 FGS_MOT_USE Bug Fix
  if (uiLastRecLayer == m_uiQualityLevelForPrediction
    || (m_dNumFGSLayers < m_uiQualityLevelForPrediction && uiLastRecLayer == m_dNumFGSLayers )
     )
  {
  m_pacControlDataEL[uiFrameIdInGOP].getMbDataCtrl()
    ->copyMotion(*(m_pcRQFGSEncoder->getMbDataCtrlEL()));

  m_pacControlDataEL[uiFrameIdInGOP].getMbDataCtrl()
    ->SetMbStride(m_pcRQFGSEncoder->getMbDataCtrlEL()->GetMbStride());

  m_pacControlDataEL[uiFrameIdInGOP].getMbDataCtrl()
    ->xSetDirect8x8InferenceFlag(m_pcRQFGSEncoder
    ->getMbDataCtrlEL()->xGetDirect8x8InferenceFlagPublic());
  }


  if(bAlreadyReconstructed)  // x. wang, Nokia
    pcFrame->copy(pcRecTemp);


  //===== reconstruction =====
  if( uiLastRecLayer < m_uiQualityLevelForPrediction &&  !bAlreadyReconstructed) //JVT-P031
  {
    RNOK( m_pcRQFGSEncoder->reconstruct   ( pcFrame ) );
    RNOK( pcResidual      ->copy          ( pcFrame ) );
    RNOK( xZeroIntraMacroblocks           ( pcResidual, rcControlData ) );
    RNOK( pcFrame         ->add           ( pcPredSignal ) );
    RNOK( xClipIntraMacroblocks   ( pcFrame, rcControlData, pcSliceHeader->getTemporalLevel() == 0 ) );
    RNOK( pcSubband       ->copy          ( pcFrame ) );
    RNOK( rcControlData.saveMbDataQpAndCbp() );
  }

  //===== highest layer reconstruction for closed-loop coding
  if( pcCLRec )
  {
    RNOK( m_pcRQFGSEncoder->reconstruct   ( pcCLRec ) );
    RNOK( pcCLRec         ->add           ( pcPredSignal ) );
    RNOK( xClipIntraMacroblocks           ( pcCLRec, rcControlData, pcSliceHeader->getTemporalLevel() == 0 ) );
  }


  RNOK( m_pcRQFGSEncoder->finishPicture () );

  //restore PPSId
  pcSliceHeader->setPicParameterSetId(uiPPSId);

  return Err::m_nOK;
}




ErrVal
MCTFEncoder::xEncodeLowPassSignal( ExtBinDataAccessorList&  rcOutExtBinDataAccessorList,
                                   ControlData&             rcControlData,
                                   IntFrame*                pcFrame,
                                   IntFrame*                pcResidual,
                                   IntFrame*                pcPredSignal,
                                   UInt&                    ruiBits,
                                   PicOutputDataList&       rcPicOutputDataList )
{
  UInt          uiBits              = 0;
  UInt          uiBitsSEI           = 0;
  IntFrame*     pcBaseLayerFrame    = rcControlData.getBaseLayerRec ();
  SliceHeader*  pcSliceHeader       = rcControlData.getSliceHeader  ();


  //----- subsequence SEI -----
  if( m_bWriteSubSequenceSei && m_bH264AVCCompatible )
  {
    RNOK( xWriteSEI( rcOutExtBinDataAccessorList, *pcSliceHeader, uiBitsSEI ) );
  }

  // JVT-S054 (ADD) ->
  if (m_bIroiSliceDivisionFlag)
  {
    for (UInt uiSliceId=0; uiSliceId <= m_uiNumSliceMinus1; uiSliceId++)
    {
      pcSliceHeader->setFirstMbInSlice(m_puiFirstMbInSlice[uiSliceId]);
      pcSliceHeader->setLastMbInSlice(m_puiLastMbInSlice[uiSliceId]);
      // JVT-S054 (2) (ADD)
      pcSliceHeader->setNumMbsInSlice(pcSliceHeader->getFMO()->getNumMbsInSlice(pcSliceHeader->getFirstMbInSlice(), pcSliceHeader->getLastMbInSlice()));

      if(m_uiMMCOBaseEnable  && m_bH264AVCCompatible)  //bug-fix suffix
      {//JVT-S036 lsj
        UInt iFrameNum =  m_pcLowPassBaseReconstruction->getFrameNum();
        RNOK( xSetMmcoBase( *pcSliceHeader, iFrameNum ) );
      }

      //----- init NAL UNIT -----
      RNOK( xInitExtBinDataAccessor                 (  m_cExtBinDataAccessor ) );
      RNOK( m_pcNalUnitEncoder->initNalUnit         ( &m_cExtBinDataAccessor ) );

      //---- write Slice Header -----
      ETRACE_NEWSLICE;
      xAssignSimplePriorityId( pcSliceHeader );

      RNOK( m_pcNalUnitEncoder->write               ( *pcSliceHeader ) );

      rcControlData.getPrdFrameList( LIST_0 ).reset();
      rcControlData.getPrdFrameList( LIST_1 ).reset();

      //----- encode slice data -----
      if( pcSliceHeader->isIntra() )
      {
        RNOK( m_pcSliceEncoder->encodeIntraPicture  ( uiBits,
                                                      rcControlData,
                                                      pcFrame,
                                                      pcResidual,
                                                      pcBaseLayerFrame,
                                                      pcPredSignal,
                                                      m_uiFrameWidthInMb,
                                                      rcControlData.getLambda() ) );
      }
      else
      {
        //----- initialize reference lists -----
        ROF ( pcSliceHeader->getNumRefIdxActive ( LIST_0 ) == 1 );
        ROF ( pcSliceHeader->getNumRefIdxActive ( LIST_1 ) == 0 );

        RefFrameList  cDPCMRefFrameListBase;
        cDPCMRefFrameListBase.reset();

        IntFrame *pcBaseRefCopyFrm = m_apcFrameTemp[2];   // not used anywhere else

        if(!m_pcLowPassBaseReconstruction->getUnusedForRef())  // JVT-Q065 EIDR
        {
          if( m_dNumFGSLayers > 0.0 && m_dLowPassEnhRef > 0 )
          {
            pcBaseRefCopyFrm->copy(m_pcLowPassBaseReconstruction);
            m_pcLowPassBaseReconstruction->getFullPelYuvBuffer()->addWeighted
              ( m_papcFrame[0]->getFullPelYuvBuffer(), m_dLowPassEnhRef );

            RNOK( rcControlData.getPrdFrameList ( LIST_0 ).add  ( m_pcLowPassBaseReconstruction ) );
            RNOK( xFillAndUpsampleFrame                         ( m_pcLowPassBaseReconstruction ) );

            RNOK( cDPCMRefFrameListBase.add                     ( pcBaseRefCopyFrm ) );
            RNOK( xFillAndUpsampleFrame                         ( pcBaseRefCopyFrm ) );
          }
          else
          {
            RNOK( rcControlData.getPrdFrameList ( LIST_0 ).add  ( m_pcLowPassBaseReconstruction ) );
            RNOK( xFillAndUpsampleFrame                         ( m_pcLowPassBaseReconstruction ) );
          }
        }

        RNOK( m_pcSliceEncoder->encodeInterPictureP ( uiBits,
                                                      pcFrame,
                                                      pcResidual,
                                                      pcPredSignal,
                                                      rcControlData,
                                                      m_uiFrameWidthInMb,
                                                      rcControlData.getPrdFrameList( LIST_0 ),
                                                      cDPCMRefFrameListBase ) );

        // restore the base layer
        if( m_dNumFGSLayers > 0.0 && m_dLowPassEnhRef > 0 )
        {
          m_pcLowPassBaseReconstruction->copy       ( pcBaseRefCopyFrm );
          RNOK( pcBaseRefCopyFrm->uninitHalfPel     () );
        }

        RNOK( m_pcLowPassBaseReconstruction->uninitHalfPel() );
      }

      //----- close NAL UNIT -----
      RNOK( m_pcNalUnitEncoder->closeNalUnit        ( uiBits ) );
      RNOK( xAppendNewExtBinDataAccessor            ( rcOutExtBinDataAccessorList, &m_cExtBinDataAccessor ) );
      uiBits += 4*8;

      //JVT-S036 lsj start
      if( m_uiSuffixUnitEnable )
      {
        if ( pcSliceHeader->getNalUnitType() == NAL_UNIT_CODED_SLICE|| pcSliceHeader->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR )
        {
          RNOK( xWriteSuffixUnit( rcOutExtBinDataAccessorList, *pcSliceHeader, uiBits ) );
        }
      }
      //JVT-S036 lsj end

      PicOutputData cPicOutputData;
      cPicOutputData.FirstPicInAU  = pcSliceHeader->getBaseLayerId           () == MSYS_UINT_MAX;
      cPicOutputData.Poc           = pcSliceHeader->getPoc                   ();
      cPicOutputData.FrameType[0]  = pcSliceHeader->getSliceType             () == B_SLICE ? 'B' :
                                     pcSliceHeader->getSliceType             () == P_SLICE ? 'P' : 'I';
      cPicOutputData.FrameType[1]  = pcSliceHeader->getUseBasePredictionFlag ()            ? 'K' : ' ';
      cPicOutputData.FrameType[2]  = '\0';
      cPicOutputData.DependencyId  = pcSliceHeader->getLayerCGSSNR           ();
      cPicOutputData.QualityId     = pcSliceHeader->getQualityLevelCGSSNR    ();
      cPicOutputData.TemporalId    = pcSliceHeader->getTemporalLevel         ();
      cPicOutputData.Qp            = pcSliceHeader->getPicQp                 ();        
      cPicOutputData.Bits          = uiBits + uiBitsSEI;
      cPicOutputData.YPSNR         = 0.0;
      cPicOutputData.UPSNR         = 0.0;
      cPicOutputData.VPSNR         = 0.0;
      rcPicOutputDataList.push_back( cPicOutputData );
  //S051{
  if(m_uiAnaSIP>0)
    m_auiFrameBits[rcControlData.getSliceHeader()->getPoc()]=uiBits+uiBitsSEI;
  //S051}

      ETRACE_NEWFRAME;

      ruiBits = ruiBits + uiBits + uiBitsSEI;
      uiBitsSEI=0;
    }
  }
  else
  {
  // JVT-S054 (ADD) <-
    FMO* pcFMO = pcSliceHeader->getFMO();
    for (UInt iSliceGroupID=0;!pcFMO->SliceGroupCompletelyCoded(iSliceGroupID);iSliceGroupID++)
    {
      pcSliceHeader->setFirstMbInSlice(pcFMO->getFirstMacroblockInSlice(iSliceGroupID));
      pcSliceHeader->setLastMbInSlice(pcFMO->getLastMBInSliceGroup(iSliceGroupID));
      // JVT-S054 (2) (ADD)
      pcSliceHeader->setNumMbsInSlice(pcFMO->getNumMbsInSlice(pcSliceHeader->getFirstMbInSlice(), pcSliceHeader->getLastMbInSlice()));

      if(m_uiMMCOBaseEnable  && m_bH264AVCCompatible)  //bug-fix suffix
      {//JVT-S036 lsj
        UInt iFrameNum =  m_pcLowPassBaseReconstruction->getFrameNum();
        RNOK( xSetMmcoBase( *pcSliceHeader, iFrameNum ) );
      }

      //----- init NAL UNIT -----
      RNOK( xInitExtBinDataAccessor                 (  m_cExtBinDataAccessor ) );
      RNOK( m_pcNalUnitEncoder->initNalUnit         ( &m_cExtBinDataAccessor ) );

      //---- write Slice Header -----
      ETRACE_NEWSLICE;
      xAssignSimplePriorityId( pcSliceHeader );

      RNOK( m_pcNalUnitEncoder->write               ( *pcSliceHeader ) );

      rcControlData.getPrdFrameList( LIST_0 ).reset();
      rcControlData.getPrdFrameList( LIST_1 ).reset();

      //----- encode slice data -----
      if( pcSliceHeader->isIntra() )
      {
        RNOK( m_pcSliceEncoder->encodeIntraPicture  ( uiBits,
                                                      rcControlData,
                                                      pcFrame,
                                                      pcResidual,
                                                      pcBaseLayerFrame,
                                                      pcPredSignal,
                                                      m_uiFrameWidthInMb,
                                                      rcControlData.getLambda() ) );
      }
      else
      {
        //----- initialize reference lists -----
        ROF ( pcSliceHeader->getNumRefIdxActive ( LIST_0 ) == 1 );
        ROF ( pcSliceHeader->getNumRefIdxActive ( LIST_1 ) == 0 );

        RefFrameList  cDPCMRefFrameListBase;
        cDPCMRefFrameListBase.reset();

        IntFrame *pcBaseRefCopyFrm = m_apcFrameTemp[2];   // not used anywhere else

        if(!m_pcLowPassBaseReconstruction->getUnusedForRef())  // JVT-Q065 EIDR
        {
          if( m_dNumFGSLayers > 0.0 && m_dLowPassEnhRef > 0 )
          {
            pcBaseRefCopyFrm->copy(m_pcLowPassBaseReconstruction);
            m_pcLowPassBaseReconstruction->getFullPelYuvBuffer()->addWeighted
                  ( m_papcFrame[0]->getFullPelYuvBuffer(), m_dLowPassEnhRef );

            RNOK( rcControlData.getPrdFrameList ( LIST_0 ).add  ( m_pcLowPassBaseReconstruction ) );
            RNOK( xFillAndUpsampleFrame                         ( m_pcLowPassBaseReconstruction ) );

            RNOK( cDPCMRefFrameListBase.add                     ( pcBaseRefCopyFrm ) );
            RNOK( xFillAndUpsampleFrame                         ( pcBaseRefCopyFrm ) );
          }
          else
          {
            RNOK( rcControlData.getPrdFrameList ( LIST_0 ).add  ( m_pcLowPassBaseReconstruction ) );
            RNOK( xFillAndUpsampleFrame                         ( m_pcLowPassBaseReconstruction ) );
          }
        }

        RNOK( m_pcSliceEncoder->encodeInterPictureP ( uiBits,
                                                      pcFrame,
                                                      pcResidual,
                                                      pcPredSignal,
                                                      rcControlData,
                                                      m_uiFrameWidthInMb,
                                                      rcControlData.getPrdFrameList( LIST_0 ),
                                                      cDPCMRefFrameListBase ) );

        // restore the base layer
        if( m_dNumFGSLayers > 0.0 && m_dLowPassEnhRef > 0 )
        {
          m_pcLowPassBaseReconstruction->copy       ( pcBaseRefCopyFrm );
          RNOK( pcBaseRefCopyFrm->uninitHalfPel     () );
        }

        RNOK( m_pcLowPassBaseReconstruction->uninitHalfPel() );
      }

      //----- close NAL UNIT -----
      RNOK( m_pcNalUnitEncoder->closeNalUnit        ( uiBits ) );
      RNOK( xAppendNewExtBinDataAccessor            ( rcOutExtBinDataAccessorList, &m_cExtBinDataAccessor ) );
      uiBits += 4*8;

      //JVT-S036 lsj start
      if( m_uiSuffixUnitEnable )
      {
        if ( pcSliceHeader->getNalUnitType() == NAL_UNIT_CODED_SLICE|| pcSliceHeader->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR )
        {
          RNOK( xWriteSuffixUnit( rcOutExtBinDataAccessorList, *pcSliceHeader, uiBits ) );
        }
      }
      //JVT-S036 lsj end

      PicOutputData cPicOutputData;
      cPicOutputData.FirstPicInAU  = pcSliceHeader->getBaseLayerId           () == MSYS_UINT_MAX;
      cPicOutputData.Poc           = pcSliceHeader->getPoc                   ();
      cPicOutputData.FrameType[0]  = pcSliceHeader->getSliceType             () == B_SLICE ? 'B' :
                                     pcSliceHeader->getSliceType             () == P_SLICE ? 'P' : 'I';
      cPicOutputData.FrameType[1]  = pcSliceHeader->getUseBasePredictionFlag ()            ? 'K' : ' ';
      cPicOutputData.FrameType[2]  = '\0';
      cPicOutputData.DependencyId  = pcSliceHeader->getLayerCGSSNR           ();
      cPicOutputData.QualityId     = pcSliceHeader->getQualityLevelCGSSNR    ();
      cPicOutputData.TemporalId    = pcSliceHeader->getTemporalLevel         ();
      cPicOutputData.Qp            = pcSliceHeader->getPicQp                 ();        
      cPicOutputData.Bits          = uiBits + uiBitsSEI;
      cPicOutputData.YPSNR         = 0.0;
      cPicOutputData.UPSNR         = 0.0;
      cPicOutputData.VPSNR         = 0.0;
      rcPicOutputDataList.push_back( cPicOutputData );

      ETRACE_NEWFRAME;

      ruiBits = ruiBits + uiBits + uiBitsSEI;
      uiBitsSEI=0;
    }
  // JVT-S054 (ADD)
  }

  return Err::m_nOK;
}







ErrVal
MCTFEncoder::xEncodeHighPassSignal( ExtBinDataAccessorList&  rcOutExtBinDataAccessorList,
                                    ControlData&             rcControlData,
                                    IntFrame*                pcOrgFrame, 
                                    IntFrame*                pcFrame,
                                    IntFrame*                pcResidual,
                                    IntFrame*                pcPredSignal,
                                    IntFrame*                pcSRFrame, // JVT-R091
                                    UInt&                    ruiBits,
                                    UInt&                    ruiBitsRes,
                                    PicOutputDataList&       rcPicOutputDataList )
{
  UInt  uiBitsSEI   = 0;

  //----- Subsequence SEI -----
  if( m_bWriteSubSequenceSei &&   m_bH264AVCCompatible )
  {
    RNOK( xWriteSEI( rcOutExtBinDataAccessorList, *rcControlData.getSliceHeader(), uiBitsSEI ) );
  }

  // JVT-S054 (ADD) ->
  if (m_bIroiSliceDivisionFlag)
  {
    for (UInt uiSliceId=0; uiSliceId <= m_uiNumSliceMinus1; uiSliceId++)
    {
      UInt  uiBits      = 0;
      UInt  uiBitsRes   = 0;
      UInt  uiMbCoded   = 0;

      rcControlData.getSliceHeader()->setFirstMbInSlice(m_puiFirstMbInSlice[uiSliceId]);
      rcControlData.getSliceHeader()->setLastMbInSlice(m_puiLastMbInSlice[uiSliceId]);
      // JVT-S054 (2) (ADD)
      rcControlData.getSliceHeader()->setNumMbsInSlice(rcControlData.getSliceHeader()->getFMO()->getNumMbsInSlice(rcControlData.getSliceHeader()->getFirstMbInSlice(), rcControlData.getSliceHeader()->getLastMbInSlice()));

      rcControlData.getSliceHeader()->setAdaptiveRefPicMarkingFlag( false );  //JVT-S036 lsj

      //----- init NAL UNIT -----
      RNOK( xInitExtBinDataAccessor        (  m_cExtBinDataAccessor ) );
      RNOK( m_pcNalUnitEncoder->initNalUnit( &m_cExtBinDataAccessor ) );

      //---- write Slice Header -----
      ETRACE_NEWSLICE;
      xAssignSimplePriorityId( rcControlData.getSliceHeader() );

      RNOK( m_pcNalUnitEncoder->write( *rcControlData.getSliceHeader() ) );

      //----- write slice data -----
      RNOK( m_pcSliceEncoder->encodeHighPassPicture ( uiMbCoded, uiBits, 
                                                      *rcControlData.getSliceHeader            (),
                                                      pcOrgFrame,
                                                      pcFrame,
                                                      pcResidual,
                                                      pcPredSignal,
                                                      pcSRFrame, // JVT-R091
                                                      rcControlData.getBaseLayerSbb           (),
                                                      rcControlData.getBaseLayerRec           (),
                                                      rcControlData.getMbDataCtrl             (),
                                                      rcControlData.getBaseLayerCtrl          (),
                                                      m_uiFrameWidthInMb,
                                                      rcControlData.getLambda                 (),
                                                      m_iMaxDeltaQp,
                                                      rcControlData.getSpatialScalabilityType () ) );

      //----- close NAL UNIT -----
      RNOK( m_pcNalUnitEncoder->closeNalUnit( uiBits ) );

      //----- update -----
      RNOK( xAppendNewExtBinDataAccessor( rcOutExtBinDataAccessorList, &m_cExtBinDataAccessor ) );
      uiBits += 4*8;
      //JVT-S036 lsj start
      if( m_uiSuffixUnitEnable )
      {
        if ( rcControlData.getSliceHeader()->getNalUnitType() == NAL_UNIT_CODED_SLICE|| rcControlData.getSliceHeader()->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR )
        {
          RNOK( xWriteSuffixUnit( rcOutExtBinDataAccessorList, *rcControlData.getSliceHeader(), uiBits ) );
        }
      }
      //JVT-S036 lsj end

      PicOutputData cPicOutputData;
      cPicOutputData.FirstPicInAU  = rcControlData.getSliceHeader()->getBaseLayerId          () == MSYS_UINT_MAX;
      cPicOutputData.Poc           = rcControlData.getSliceHeader()->getPoc                  ();
      cPicOutputData.FrameType[0]  = rcControlData.getSliceHeader()->getSliceType            () == B_SLICE ? 'B' :
                                     rcControlData.getSliceHeader()->getSliceType            () == P_SLICE ? 'P' : 'I';
      cPicOutputData.FrameType[1]  = rcControlData.getSliceHeader()->getUseBasePredictionFlag()            ? 'K' : ' ';
      cPicOutputData.FrameType[2]  = '\0';
      cPicOutputData.DependencyId  = rcControlData.getSliceHeader()->getLayerCGSSNR          ();
      cPicOutputData.QualityId     = rcControlData.getSliceHeader()->getQualityLevelCGSSNR   ();
      cPicOutputData.TemporalId    = rcControlData.getSliceHeader()->getTemporalLevel        ();
      cPicOutputData.Qp            = rcControlData.getSliceHeader()->getPicQp                ();        
      cPicOutputData.Bits          = uiBits + uiBitsSEI;
      cPicOutputData.YPSNR         = 0.0;
      cPicOutputData.UPSNR         = 0.0;
      cPicOutputData.VPSNR         = 0.0;
      rcPicOutputDataList.push_back( cPicOutputData );

    //S051{
    if(m_uiAnaSIP>0)
    m_auiFrameBits[rcControlData.getSliceHeader()->getPoc()]=uiBits+uiBitsSEI;
    //S051}
      ruiBits     += uiBits+uiBitsSEI;
      ruiBitsRes  += uiBitsRes;
      uiBitsSEI =0;
    }
  }
  else
  {
  // JVT-S054 (ADD) <-
    FMO* pcFMO = rcControlData.getSliceHeader()->getFMO();
    for (UInt iSliceGroupID=0;!pcFMO->SliceGroupCompletelyCoded(iSliceGroupID);iSliceGroupID++)
    {
      UInt  uiBits      = 0;
      UInt  uiBitsRes   = 0;
      UInt  uiMbCoded   = 0;

      rcControlData.getSliceHeader()->setFirstMbInSlice(pcFMO->getFirstMacroblockInSlice(iSliceGroupID));
      rcControlData.getSliceHeader()->setLastMbInSlice(pcFMO->getLastMBInSliceGroup(iSliceGroupID));
      rcControlData.getSliceHeader()->setNumMbsInSlice(pcFMO->getNumMbsInSlice(rcControlData.getSliceHeader()->getFirstMbInSlice(), rcControlData.getSliceHeader()->getLastMbInSlice()));

      rcControlData.getSliceHeader()->setAdaptiveRefPicMarkingFlag( false );  //JVT-S036 lsj

      //----- init NAL UNIT -----
      RNOK( xInitExtBinDataAccessor        (  m_cExtBinDataAccessor ) );
      RNOK( m_pcNalUnitEncoder->initNalUnit( &m_cExtBinDataAccessor ) );

      //---- write Slice Header -----
      ETRACE_NEWSLICE;
      xAssignSimplePriorityId( rcControlData.getSliceHeader() );

      RNOK( m_pcNalUnitEncoder->write( *rcControlData.getSliceHeader() ) );

      //----- write slice data -----
      RNOK( m_pcSliceEncoder->encodeHighPassPicture ( uiMbCoded, uiBits, 
                                                      *rcControlData.getSliceHeader            (),
                                                      pcOrgFrame,
                                                      pcFrame,
                                                      pcResidual,
                                                      pcPredSignal,
                                                      pcSRFrame, // JVT-R091
                                                      rcControlData.getBaseLayerSbb           (),
                                                      rcControlData.getBaseLayerRec           (),
                                                      rcControlData.getMbDataCtrl             (),
                                                      rcControlData.getBaseLayerCtrl          (),
                                                      m_uiFrameWidthInMb,
                                                      rcControlData.getLambda                 (),
                                                      m_iMaxDeltaQp,
                                                      rcControlData.getSpatialScalabilityType () ) );

      //----- close NAL UNIT -----
      RNOK( m_pcNalUnitEncoder->closeNalUnit( uiBits ) );

      //----- update -----
      RNOK( xAppendNewExtBinDataAccessor( rcOutExtBinDataAccessorList, &m_cExtBinDataAccessor ) );
      uiBits += 4*8;
      //JVT-S036 lsj start
      if( m_uiSuffixUnitEnable )
      {
        if ( rcControlData.getSliceHeader()->getNalUnitType() == NAL_UNIT_CODED_SLICE|| rcControlData.getSliceHeader()->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR )
        {
          RNOK( xWriteSuffixUnit( rcOutExtBinDataAccessorList, *rcControlData.getSliceHeader(), uiBits ) );
        }
      }
      //JVT-S036 lsj end

      PicOutputData cPicOutputData;
      cPicOutputData.FirstPicInAU  = rcControlData.getSliceHeader()->getBaseLayerId          () == MSYS_UINT_MAX;
      cPicOutputData.Poc           = rcControlData.getSliceHeader()->getPoc                  ();
      cPicOutputData.FrameType[0]  = rcControlData.getSliceHeader()->getSliceType            () == B_SLICE ? 'B' :
                                     rcControlData.getSliceHeader()->getSliceType            () == P_SLICE ? 'P' : 'I';
      cPicOutputData.FrameType[1]  = rcControlData.getSliceHeader()->getUseBasePredictionFlag()            ? 'K' : ' ';
      cPicOutputData.FrameType[2]  = '\0';
      cPicOutputData.DependencyId  = rcControlData.getSliceHeader()->getLayerCGSSNR          ();
      cPicOutputData.QualityId     = rcControlData.getSliceHeader()->getQualityLevelCGSSNR   ();
      cPicOutputData.TemporalId    = rcControlData.getSliceHeader()->getTemporalLevel        ();
      cPicOutputData.Qp            = rcControlData.getSliceHeader()->getPicQp                ();        
      cPicOutputData.Bits          = uiBits + uiBitsSEI;
      cPicOutputData.YPSNR         = 0.0;
      cPicOutputData.UPSNR         = 0.0;
      cPicOutputData.VPSNR         = 0.0;
      rcPicOutputDataList.push_back( cPicOutputData );

      ruiBits     += uiBits+uiBitsSEI;
      ruiBitsRes  += uiBitsRes;
      uiBitsSEI =0;
    }
  // JVT-S054 (ADD)
  }

  ETRACE_NEWFRAME;

  return Err::m_nOK;
}




ErrVal
MCTFEncoder::xInitGOP( PicBufferList&  rcPicBufferInputList )
{
  //========== INITIALIZE DECOMPOSITION STRUCTURES ==========
  m_uiGOPSize                         = rcPicBufferInputList.size ();
  PicBufferList::iterator cInputIter  = rcPicBufferInputList.begin();
  UInt                    uiFrame     = 0;
  if( m_bFirstGOPCoded )
  {
    m_papcFrame          [ uiFrame ]->copyAll( m_pcAnchorFrameOriginal );
    m_papcOrgFrame      [ uiFrame ]->copyAll( m_pcAnchorFrameOriginal );
    m_papcSmoothedFrame [ uiFrame ]->copyAll( m_pcAnchorFrameOriginal ); // JVT-R091
    uiFrame    ++;
  }
  else
  {
    m_uiGOPSize--;
  }
  for( ; uiFrame <= m_uiGOPSize; uiFrame++, cInputIter++ )
  {
    PicBuffer*  pcPicBuffer = *cInputIter;
    m_papcFrame          [ uiFrame ]->load         ( pcPicBuffer );
    m_papcOrgFrame      [ uiFrame ]->load         ( pcPicBuffer );
    m_papcSmoothedFrame  [ uiFrame ]->load         ( pcPicBuffer ); // JVT-R091
    m_papcFrame          [ uiFrame ]->setPOC       ( m_uiFrameCounter++ << m_uiTemporalResolution );
  }
  if( m_uiGOPSize == ( 1 << m_uiDecompositionStages ) ) // store original anchor frame
  {
    RNOK( m_pcAnchorFrameOriginal->copyAll( m_papcFrame[ m_uiGOPSize ] ) );
  }

    // TMM_ESS {
    if ( m_pcResizeParameters->m_pParamFile != NULL )
    {
        if ( m_uiGOPNumber == 0 )
        {
            m_pcResizeParameters->readPictureParameters( 0 );
        }

        for(uiFrame=1 ; uiFrame <= m_uiGOPSize; uiFrame++ )
        {
            int poc = m_papcFrame[uiFrame]->getPOC();
            m_pcResizeParameters->readPictureParameters( poc );
        }
        //for(uiFrame=0 ; uiFrame <= m_uiGOPSize; uiFrame++ )
        //{
        //    int poc = m_papcFrame[uiFrame]->getPOC();
        //    printf("MCTFEncoder::xInitGOP - fr %d - poc %d - posY %d\n",uiFrame,poc,m_pcResizeParameters->getCurrentPictureParameters(poc)->m_iPosY);
        //}
    }
    else {
        for(uiFrame=0 ; uiFrame < m_uiGOPSize; uiFrame++ )
        {
            m_pcResizeParameters->setPictureParametersByValue(uiFrame,
                m_pcResizeParameters->m_iPosX,
                m_pcResizeParameters->m_iPosY,
                m_pcResizeParameters->m_iOutWidth,
                m_pcResizeParameters->m_iOutHeight,
                m_pcResizeParameters->m_iBaseChromaPhaseX,
                m_pcResizeParameters->m_iBaseChromaPhaseY);
        }
    }
    // TMM_ESS }

  //========== INITIALIZE SLICE HEADERS (the decomposition structure is determined at this point) ==========
  if( ! m_bFirstGOPCoded )
  {
    RNOK  ( xInitSliceHeader( 0, 0 ) );
  }
  else
  {
    //----- copy frame_num of anchor frame -> needed for RPLR command setting -----
    m_pacControlData[0].getSliceHeader()->setFrameNum( m_cLPFrameNumList.front()  );
  }
  UInt uiTemporalLevel;
  for( uiTemporalLevel = 0; uiTemporalLevel <= m_uiDecompositionStages; uiTemporalLevel++ )
  {
    UInt      uiStep    = ( 1 << ( m_uiDecompositionStages - uiTemporalLevel ) );
    for( UInt uiFrameId = uiStep; uiFrameId <= m_uiGOPSize; uiFrameId += ( uiStep << 1 ) )
    {
      RNOK( xInitSliceHeader( uiTemporalLevel, uiFrameId ) );
    }
  }

// JVT-Q065 EIDR{
 for( uiTemporalLevel = 0; uiTemporalLevel <= m_uiDecompositionStages; uiTemporalLevel++ )
  {
    UInt      uiStep    = ( 1 << ( m_uiDecompositionStages - uiTemporalLevel ) );
    for( UInt uiFrameId = uiStep; uiFrameId <= m_uiGOPSize; uiFrameId += ( uiStep << 1 ) )
    {
      SliceHeader* pcSliceHeader = m_pacControlData[uiFrameId].getSliceHeader();
      if(pcSliceHeader->isIdrNalUnit())
      {
        for( uiFrame = 0; uiFrame <= m_uiGOPSize; uiFrame++)
        {
          if(m_pacControlData[uiFrame].getSliceHeader()->getTemporalLevel() < pcSliceHeader->getTemporalLevel() || (m_pacControlData[uiFrame].getSliceHeader()->getTemporalLevel() == pcSliceHeader->getTemporalLevel()&& uiFrame < uiFrameId))
          {
            //bug-fix shenqiu EIDR{
            if(m_papcBQFrame)
            {
            m_papcBQFrame[uiFrame]->setUnusedForRef(true);
            }
            //bug-fix shenqiu EIDR}
            m_papcFrame[uiFrame]->setUnusedForRef(true);
            m_papcOrgFrame[uiFrame]->setUnusedForRef(true);
            if(m_papcCLRecFrame)
            {
              m_papcCLRecFrame[uiFrame]->setUnusedForRef(true);
            }
          }
        }
      }
    }
  }

  //bug-fix shenqiu EIDR{
  if((m_iIDRPeriod != 0) && (m_papcFrame[m_uiGOPSize]->getPOC() % m_iIDRPeriod >= (Int)m_uiGOPSize))
  {
    m_bBLSkipEnable = true;
  }
  //bug-fix shenqiu EIDR}
// JVT-Q065 EIDR}

  //===== RPLR and MMCO commands =====
  for( uiFrame = m_bFirstGOPCoded ? 1 : 0; uiFrame <= m_uiGOPSize; uiFrame++ )
  {
    RNOK( xInitReordering( uiFrame ) );
  }


  //========== INITIALIZE SCALING FACTORS ==========
  for( uiFrame = 0; uiFrame <= m_uiGOPSize; uiFrame++ )
  {
    m_pacControlData[ uiFrame ].clear();
    m_pacControlData[ uiFrame ].setScalingFactor( 1.0 );
    MbDataCtrl* pcMbDataCtrl = m_pacControlData[ uiFrame ].getMbDataCtrl();
    RNOK( pcMbDataCtrl->reset () );
    RNOK( pcMbDataCtrl->clear () );
  }


  m_uiNotYetConsideredBaseLayerBits = 0;
  m_uiNotYetConsideredBaseLayerBits = m_uiParameterSetBits;
  m_uiNewlyCodedBits               += m_uiParameterSetBits;
  m_auiCurrGOPBits      [0]         = m_uiParameterSetBits;
  for( UInt uiSXL = 1; uiSXL < MAX_SCALABLE_LAYERS; uiSXL++ )
  {
    m_auiCurrGOPBits    [uiSXL]     = 0;
  }

  return Err::m_nOK;
}


ErrVal
MCTFEncoder::xInitBitCounts()
{
  UInt  uiLowerLayerBits             = m_pcH264AVCEncoder->getNewBits( m_uiBaseLayerId );
  m_uiNotYetConsideredBaseLayerBits += uiLowerLayerBits;
  m_uiNewlyCodedBits                += uiLowerLayerBits;
  return Err::m_nOK;
}


ErrVal
MCTFEncoder::getBaseLayerStatus( Bool& bExists,
                                 Bool& bMotion,
                                 Int   iPoc )
{
  bExists = false;
  bMotion = false;

  for( UInt uiFrame = 0; uiFrame <= m_uiGOPSize; uiFrame++ )
  {
    if( m_papcFrame[uiFrame]->getPOC() == iPoc )
    {
      bExists = m_pacControlData[uiFrame].getSliceHeader()->getTemporalLevel() <= ( m_uiDecompositionStages - m_uiNotCodedMCTFStages );
      bMotion = bExists || !m_bH264AVCCompatible;
      break;
    }
  }

  //S051{
  if(m_bEncSIP)
  {
    if(xSIPCheck(iPoc))
    {
      bExists = false;
      bMotion = bExists || !m_bH264AVCCompatible;
    }
  }
  //S051}

  return Err::m_nOK;
}


ErrVal
MCTFEncoder::getBaseLayerData( IntFrame*&     pcFrame,
                               IntFrame*&     pcResidual,
                               MbDataCtrl*&   pcMbDataCtrl,
                               MbDataCtrl*& pcMbDataCtrlEL,    // ICU/ETRI FGS_MOT_USE
                               Bool&          bConstrainedIPredBL,
                               Bool&          bForCopyOnly,
                               Int            iSpatialScalability,
                               Int            iPoc,
                               Bool           bMotion )
{
  UInt  uiPos   = MSYS_UINT_MAX;
  pcFrame       = 0;
  pcResidual    = 0;
  pcMbDataCtrl  = 0;
  bForCopyOnly  = 0;
  pcMbDataCtrlEL = 0;

  for( UInt uiFrame = 0; uiFrame <= m_uiGOPSize; uiFrame++ )
  {
    if( m_papcFrame[uiFrame]->getPOC() == iPoc )
    {
      if( m_pacControlData[uiFrame].getSliceHeader()->getTemporalLevel() <= ( m_uiDecompositionStages - m_uiNotCodedMCTFStages ) )
      {
        pcFrame       = m_papcSubband   [uiFrame];
        pcResidual    = m_papcResidual  [uiFrame];

        if( !bMotion )                                                    // HS -> that's the correct place for it
        m_pacControlData[uiFrame].activateMbDataCtrlForQpAndCbp( false ); //    -> need correct CBP values for loop filter of next layer (residual prediction)
                                                                          //    -> it cannot be reset after this assignment !!!
        pcMbDataCtrl  = m_pacControlData[uiFrame].getMbDataCtrl();        //    -> it is reset in ControlData::clear at the beginning of the next GOP

        // ICU/ETRI FGS_MOT_USE
        pcMbDataCtrlEL  = m_pacControlDataEL[uiFrame].getMbDataCtrl();        //    -> it is reset in ControlData::clear at the beginning of the next GOP

        bForCopyOnly  = false;

        bConstrainedIPredBL = m_pacControlData[uiFrame].getSliceHeader()->getPPS().getConstrainedIntraPredFlag();
      }
      uiPos = uiFrame;
      break;
    }
  }


  if( iSpatialScalability != SST_RATIO_1 )
  {
    RNOK( m_apcFrameTemp[0]->copy( pcFrame ) );
  //JVT-R057 LA-RDO{
  if(m_bLARDOEnable)
    m_apcFrameTemp[0]->setChannelDistortion(pcFrame);
  //JVT-R057 LA-RDO}
    pcFrame = m_apcFrameTemp[0];

    if ( m_pacControlData[uiPos].getSliceHeader()->getPPS().getConstrainedIntraPredFlag() )
    {
      m_pcLoopFilter->setFilterMode( LoopFilter::LFMode( LoopFilter::LFM_NO_INTER_FILTER + LoopFilter::LFM_EXTEND_INTRA_SUR ) );

  RNOK( m_pcLoopFilter->process(*m_pacControlData[uiPos].getSliceHeader (),
                                     pcFrame,
                                     m_pacControlData[uiPos].getMbDataCtrl  (),
                                     m_pacControlData[uiPos].getMbDataCtrl  (),
                                     m_uiFrameWidthInMb,
                                     NULL,
                                     NULL,
                   true,
                                     m_pacControlData[uiPos].getSpatialScalability()) );  // SSUN@SHARP
      m_pcLoopFilter->setFilterMode();
    }
    else
    {
      m_pcLoopFilter->setHighpassFramePointer( pcResidual );

      RNOK( m_pcLoopFilter->process(*m_pacControlData[uiPos].getSliceHeader (),
                                     pcFrame,
                                     m_pacControlData[uiPos].getMbDataCtrl  (),
                                     m_pacControlData[uiPos].getMbDataCtrl  (),
                                     m_uiFrameWidthInMb,
                                    &m_pacControlData[uiPos].getPrdFrameList( LIST_0 ),
                                    &m_pacControlData[uiPos].getPrdFrameList( LIST_1 ),
                  true,
                                     m_pacControlData[uiPos].getSpatialScalability()) );  // SSUN@SHARP
    }
  }

  return Err::m_nOK;
}


ErrVal
MCTFEncoder::getBaseLayerSH( SliceHeader*&  rpcSliceHeader,
                             Int            iPoc )
{
  rpcSliceHeader = 0;

  for( UInt uiFrame = 0; uiFrame <= m_uiGOPSize; uiFrame++ )
  {
    if( m_papcFrame[uiFrame]->getPOC() == iPoc )
    {
      if( m_pacControlData[uiFrame].getSliceHeader()->getTemporalLevel() <= ( m_uiDecompositionStages - m_uiNotCodedMCTFStages ) )
      {
        rpcSliceHeader = m_pacControlData[uiFrame].getSliceHeader();
      }
      break;
    }
  }
  ROF( rpcSliceHeader );

  return Err::m_nOK;
}


ErrVal
MCTFEncoder::xGetListSizes( UInt  uiTemporalLevel,
                            UInt  uiFrameIdInGOP,
                            UInt  auiPredListSize[2] )
{
  //----- get delay decomposition stages -----
  UInt  uiDelayDecompositionStages = 0;
  for( ; m_uiFrameDelay >> uiDelayDecompositionStages; uiDelayDecompositionStages++ );
  uiDelayDecompositionStages = min( m_uiDecompositionStages, uiDelayDecompositionStages );


  //----- loop over prediction and update steps -----
  for( UInt uiLevel = uiTemporalLevel; uiLevel <= m_uiDecompositionStages; uiLevel++ )
  {
    //----- get parameters base GOP size and cut-off frame id -----
    UInt  uiBaseLevel       = m_uiDecompositionStages - uiLevel;
    UInt  uiFrameIdLevel    = uiFrameIdInGOP >> uiBaseLevel;
    UInt  uiBaseGOPSize     = ( 1 << uiDelayDecompositionStages ) >> min( uiBaseLevel, uiDelayDecompositionStages );
    UInt  uiCutOffFrame     = max( 0, Int( uiBaseGOPSize - ( m_uiFrameDelay >> uiBaseLevel ) - 1 ) );

    if( uiLevel == uiTemporalLevel )
    {
      //=========== PREDICTION LIST SIZES ==========
      auiPredListSize[0]    = ( uiFrameIdLevel + 1 ) >> 1;
      UInt  uiFrameIdWrap   = ( uiFrameIdLevel % uiBaseGOPSize );
      if( uiFrameIdWrap > uiCutOffFrame )
      {
        auiPredListSize[1]  = ( uiBaseGOPSize - uiFrameIdWrap + 1 ) >> 1;
      }
      else
      {
        auiPredListSize[1]  = ( uiCutOffFrame - uiFrameIdWrap + 1 ) >> 1;
      }

      auiPredListSize[0]    = min( m_uiMaxNumRefFrames, auiPredListSize[0] );
      auiPredListSize[1]    = min( m_uiMaxNumRefFrames, auiPredListSize[1] );

      //----- take into account actual GOP size -----
      {
        UInt  uiMaxL1Size   = ( ( m_uiGOPSize >> uiBaseLevel ) + 1 - uiFrameIdLevel ) >> 1;
        auiPredListSize[1]  = min( uiMaxL1Size,         auiPredListSize[1] );
      }

      if( m_bHaarFiltering )
      {
        auiPredListSize[1]  = 0;
      }
    }
  }

  //----- check intra -----
  if( m_uiLowPassIntraPeriod != MSYS_UINT_MAX )
  {
    UInt  uiCurrFrame   = (   m_uiGOPNumber                << m_uiDecompositionStages ) + uiFrameIdInGOP;
    UInt  uiIntraPeriod = ( ( m_uiLowPassIntraPeriod + 1 ) << m_uiDecompositionStages );
    if( ( uiCurrFrame % uiIntraPeriod ) == 0 )
    {
      auiPredListSize[0] = 0;
      auiPredListSize[1] = 0;
    }
  }

  return Err::m_nOK;
}



ErrVal
MCTFEncoder::xSetBaseLayerData( UInt uiFrameIdInGOP )
{
  ControlData&  rcControlData       = m_pacControlData[ uiFrameIdInGOP ];
  SliceHeader*  pcSliceHeader       = rcControlData .getSliceHeader ();
  Int           iPoc                = pcSliceHeader->getPoc         ();
  UInt          uiBaseLayerId       = m_uiBaseLayerId;
  UInt          uiBaseLayerIdMotion = m_uiBaseLayerId;
  Int           iSpatialScalabilityType = SST_RATIO_1;

  if( ! m_bInterLayerPrediction )
  {
    pcSliceHeader->setBaseLayerId           ( MSYS_UINT_MAX );
    pcSliceHeader->setBaseQualityLevel      ( 0 );
    pcSliceHeader->setAdaptivePredictionFlag( false );
    rcControlData .setBaseLayer             ( MSYS_UINT_MAX, MSYS_UINT_MAX );
    rcControlData .setSpatialScalabilityType   ( SST_RATIO_1 );
    return Err::m_nOK;
  }

  RNOK( m_pcH264AVCEncoder->getBaseLayerStatus( uiBaseLayerId, uiBaseLayerIdMotion, iSpatialScalabilityType, m_uiLayerId, iPoc ) );

  Bool  bAdaptive = ( ! pcSliceHeader->getTemporalLevel() && ( ! pcSliceHeader->isIntra() || (iSpatialScalabilityType != SST_RATIO_1) ) ? true : m_bAdaptivePrediction );

  if( uiBaseLayerId != uiBaseLayerIdMotion && ( bAdaptive || (iSpatialScalabilityType != SST_RATIO_1) ) )
  {
    uiBaseLayerIdMotion = uiBaseLayerId;
  }

  pcSliceHeader->setBaseLayerId             ( uiBaseLayerId );
  pcSliceHeader->setBaseQualityLevel        ( m_uiBaseQualityLevel );
  pcSliceHeader->setAdaptivePredictionFlag  ( uiBaseLayerId != MSYS_UINT_MAX ? bAdaptive : false );
  rcControlData .setBaseLayer               ( uiBaseLayerId, uiBaseLayerIdMotion );
  rcControlData .setSpatialScalability( iSpatialScalabilityType > SST_RATIO_1 );  // SSUN@SHARP
  rcControlData .setSpatialScalabilityType  ( iSpatialScalabilityType );

  return Err::m_nOK;
}



ErrVal
MCTFEncoder::xInitSliceHeader( UInt uiTemporalLevel,
                               UInt uiFrameIdInGOP )
{
  SliceHeader*  pcSliceHeader = m_pacControlData[ uiFrameIdInGOP ].getSliceHeader();
  ROF( pcSliceHeader );


  //===== get maximum sizes of prediction and update lists ( decomposition structure ) =====
  UInt  auiPredListSize             [2];
  RNOK( xGetListSizes( uiTemporalLevel, uiFrameIdInGOP, auiPredListSize ) );


  //===== get slice header parameters =====
  NalRefIdc     eNalRefIdc;

  if ( uiFrameIdInGOP == 0 || uiFrameIdInGOP == ( 1 << m_uiDecompositionStages ) )
    eNalRefIdc = NAL_REF_IDC_PRIORITY_HIGHEST;
  else
    eNalRefIdc = NalRefIdc( min( 2, max( 0, (Int)( m_uiDecompositionStages - m_uiNotCodedMCTFStages - uiTemporalLevel ) ) ) );

  NalUnitType   eNalUnitType    = ( m_bH264AVCCompatible
                                    ? ( uiFrameIdInGOP ? NAL_UNIT_CODED_SLICE          : NAL_UNIT_CODED_SLICE_IDR          )
                                    : ( uiFrameIdInGOP ? NAL_UNIT_CODED_SLICE_SCALABLE : NAL_UNIT_CODED_SLICE_IDR_SCALABLE ) );
// JVT-Q065 EIDR{
  if( (m_papcFrame[uiFrameIdInGOP]->getPOC() == 0) || (m_iIDRPeriod != 0 && m_papcFrame[uiFrameIdInGOP]->getPOC() % m_iIDRPeriod == 0 ) )
  {
    eNalUnitType = m_bH264AVCCompatible ? NAL_UNIT_CODED_SLICE_IDR : NAL_UNIT_CODED_SLICE_IDR_SCALABLE;
  }
// JVT-Q065 EIDR}

  SliceType     eSliceType      = ( auiPredListSize[1] ? B_SLICE : auiPredListSize[0] ? P_SLICE : I_SLICE );
  Bool          bUseBaseRep     = ( eNalRefIdc == NAL_REF_IDC_PRIORITY_HIGHEST ) ? 1 : 0;

  //===== set simple slice header parameters =====
  pcSliceHeader->setNalRefIdc                   ( eNalRefIdc            );
  pcSliceHeader->setNalUnitType                 ( eNalUnitType          );
  pcSliceHeader->setLayerId                     ( m_uiLayerId           );
  pcSliceHeader->setTemporalLevel               ( uiTemporalLevel       );
  pcSliceHeader->setQualityLevel                ( 0                     );


  pcSliceHeader->setDiscardableFlag             ( false                 );

  // JVT-U116 LMI {
  pcSliceHeader->setExtensionFlag               ( m_bExtensionFlag      );
  SliceHeader*  pcTl0SliceHeader;
  pcTl0SliceHeader = m_pacControlData[ 0 ].getSliceHeader();
  ROF( pcTl0SliceHeader );
  UInt uiPrevTl0FrameIdx = pcTl0SliceHeader->getPrevTl0FrameIdx();

  if ( pcSliceHeader->getLayerId() == 0 )
  {
    pcTl0SliceHeader->setNumTl0FrameIdxUpdate ( pcTl0SliceHeader->getNumTl0FrameIdxUpdate() + 1 );
    if ( eNalUnitType == NAL_UNIT_CODED_SLICE_IDR )
    {
      pcSliceHeader->setTl0FrameIdx        ( 0 );
      if ( m_papcFrame[uiFrameIdInGOP]->getPOC() == 0 ) 
      {
        pcTl0SliceHeader->setPrevTl0FrameIdx    ( 0 );
        pcTl0SliceHeader->setNumTl0FrameIdxUpdate ( 0 );
        pcTl0SliceHeader->setTl0FrameIdxResetFlag ( false );
      }
      else
        pcTl0SliceHeader->setTl0FrameIdxResetFlag ( true );
    }
    else if ( uiTemporalLevel == 0 ) 
      pcSliceHeader->setTl0FrameIdx             ( (uiPrevTl0FrameIdx + 1) % 256 );
    else 
      pcSliceHeader->setTl0FrameIdx             ( uiPrevTl0FrameIdx );

    if ( pcTl0SliceHeader->getNumTl0FrameIdxUpdate() == m_uiGOPSize )
    {
        pcTl0SliceHeader->setNumTl0FrameIdxUpdate ( 0 );
        pcTl0SliceHeader->setPrevTl0FrameIdx( (uiPrevTl0FrameIdx + 1) % 256);
        if ( pcTl0SliceHeader->getTl0FrameIdxResetFlag() )
        {
          pcTl0SliceHeader->setPrevTl0FrameIdx( 0 );
          pcTl0SliceHeader->setTl0FrameIdxResetFlag( false );
        }
    }
  }
  // JVT-U116 LMI }
  pcSliceHeader->setSimplePriorityId            ( 0                      );


  pcSliceHeader->setFirstMbInSlice              ( 0                     );
  pcSliceHeader->setLastMbInSlice               ( m_uiMbNumber - 1      );
  pcSliceHeader->setSliceType                   ( eSliceType            );
  pcSliceHeader->setFrameNum                    ( m_uiFrameNum          );
  pcSliceHeader->setIdrPicId                    ( 0                     );
  pcSliceHeader->setDirectSpatialMvPredFlag     ( true                  );
  pcSliceHeader->setUseBaseRepresentationFlag   ( bUseBaseRep           );
//pcSliceHeader->setKeyPicFlagScalable          ( false          ); //JVT-S036 lsj //bug-fix suffix shenqiu
  pcSliceHeader->setNumRefIdxActiveOverrideFlag ( false                 );
  pcSliceHeader->setCabacInitIdc                ( 0                     );
  pcSliceHeader->setSliceHeaderQp               ( 0                     );
  // Currently hard-coded
  pcSliceHeader->setNumMbsInSlice               ( m_uiMbNumber          );
  pcSliceHeader->setFragmentOrder               ( 0 );
//JVT-T054{
  pcSliceHeader->setLayerCGSSNR                 (m_uiLayerCGSSNR);
  pcSliceHeader->setQualityLevelCGSSNR          (m_uiQualityLevelCGSSNR);
  pcSliceHeader->setBaseLayerCGSSNR                 (m_uiBaseLayerCGSSNR);
  pcSliceHeader->setBaseQualityLevelCGSSNR          (m_uiBaseQualityLevelCGSSNR);
//JVT-T054}
//JVT-Q054 Red. Picture {
  if ( pcSliceHeader->getPPS().getRedundantPicCntPresentFlag())
  {
    pcSliceHeader->setRedundantPicCnt( 0 );
  }
// JVT-Q054 Red. Picture }

  //JVT-U106 Behaviour at slice boundaries{
  pcSliceHeader->setCIUFlag(m_bCIUFlag);
  //JVT-U106 Behaviour at slice boundaries{

  //===== set prediction and update list sizes =====
  {
    //--- prediction ---
    pcSliceHeader->setNumRefIdxActive( LIST_0, 0 );
    pcSliceHeader->setNumRefIdxActive( LIST_1, 0 );

    UInt  uiMaxLists = ( eSliceType == B_SLICE ? 2 : eSliceType == P_SLICE ? 1 : 0 );
    for( UInt uiList = 0; uiList < uiMaxLists; uiList++ )
    {
      ListIdx eListIdx  = ListIdx( uiList );
      ROF( auiPredListSize[ uiList ] );

      pcSliceHeader->setNumRefIdxActive( eListIdx, auiPredListSize[ uiList ] );
      if( pcSliceHeader->getPPS().getNumRefIdxActive( eListIdx ) != auiPredListSize[ uiList ] )
      {
        pcSliceHeader->setNumRefIdxActiveOverrideFlag( true );
      }
    }
  }

  //===== de-blocking filter parameters =====
  if( pcSliceHeader->getPPS().getDeblockingFilterParametersPresentFlag() )
  {
    pcSliceHeader->getDeblockingFilterParameterScalable().getDeblockingFilterParameter().setDisableDeblockingFilterIdc ( m_uiFilterIdc       );
    pcSliceHeader->getDeblockingFilterParameterScalable().getDeblockingFilterParameter().setSliceAlphaC0Offset         ( 2 * m_iAlphaOffset  );
    pcSliceHeader->getDeblockingFilterParameterScalable().getDeblockingFilterParameter().setSliceBetaOffset            ( 2 * m_iBetaOffset   );
  }

  //===== set remaining slice header parameters =====
  RNOK( m_pcPocCalculator->setPoc( *pcSliceHeader, m_papcFrame[uiFrameIdInGOP]->getPOC() ) );

  //bug-fix shenqiu EIDR{
  if(m_papcBQFrame)
  {
  m_papcBQFrame[uiFrameIdInGOP]->setPOC(m_papcFrame[uiFrameIdInGOP]->getPOC());
  }
  //bug-fix shenqiu EIDR}

  //===== set base layer data =====
  RNOK( xSetBaseLayerData( uiFrameIdInGOP ) );

    // TMM_ESS {
    if (m_uiLayerId > 0)
    {
        int index = pcSliceHeader->getPoc();
        pcSliceHeader->setLeftOffset(m_pcResizeParameters->getLeftOffset(index));
        pcSliceHeader->setRightOffset(m_pcResizeParameters->getRightOffset(index));
        pcSliceHeader->setTopOffset(m_pcResizeParameters->getTopOffset(index));
        pcSliceHeader->setBottomOffset(m_pcResizeParameters->getBottomOffset(index));
        pcSliceHeader->setBaseChromaPhaseX(m_pcResizeParameters->m_iBaseChromaPhaseX);  // Shijun-bug-fix
        pcSliceHeader->setBaseChromaPhaseY(m_pcResizeParameters->m_iBaseChromaPhaseY);  // Shijun-bug-fix
    }
    // TMM_ESS }

  //===== update some parameters =====
  if( eNalRefIdc )
  {
    m_uiFrameNum = ( m_uiFrameNum + 1 ) % ( 1 << pcSliceHeader->getSPS().getLog2MaxFrameNum() );
  }

  pcSliceHeader->setSliceGroupChangeCycle(1);
  pcSliceHeader->FMOInit();

  //S051{
  if(m_bEncSIP)
  {
    if(xSIPCheck(pcSliceHeader->getPoc()))
      pcSliceHeader->setDiscardableFlag( true );
  }
  //S051}

  return Err::m_nOK;
}




ErrVal
MCTFEncoder::xInitReordering ( UInt uiFrameIdInGOP )
{
  SliceHeader*  pcSliceHeader = m_pacControlData[ uiFrameIdInGOP ].getSliceHeader();
  ROF( pcSliceHeader );


  //===== set RPLR and MMCO =====
  if( pcSliceHeader->getTemporalLevel() == 0 )
  {
    //===== low-pass frames =====
    RNOK( xSetRplrAndMmco( *pcSliceHeader ) );
  }
  else if( m_uiDecompositionStages - pcSliceHeader->getTemporalLevel() >= m_uiNotCodedMCTFStages )
  {
    UIntList cFrameNumList;
    pcSliceHeader->getMmcoBuffer().clear();
    pcSliceHeader->setAdaptiveRefPicBufferingFlag( false );

    if( pcSliceHeader->getSliceType() == B_SLICE )
    {
      if( m_bForceReOrderingCommands || pcSliceHeader->getNumRefIdxActive( LIST_0 ) > 1 )
      {
        RNOK( xGetFrameNumList( *pcSliceHeader,                       cFrameNumList, LIST_0, uiFrameIdInGOP ) );
        RNOK( xSetRplr        ( pcSliceHeader->getRplrBuffer(LIST_0), cFrameNumList, pcSliceHeader->getFrameNum() ) );
      }
      else
      {
        pcSliceHeader->getRplrBuffer( LIST_0 ).clear();
        pcSliceHeader->getRplrBuffer( LIST_0 ).setRefPicListReorderingFlag( false );
      }
      if( m_bForceReOrderingCommands )
      {
        RNOK( xGetFrameNumList( *pcSliceHeader,                       cFrameNumList, LIST_1, uiFrameIdInGOP ) );
        RNOK( xSetRplr        ( pcSliceHeader->getRplrBuffer(LIST_1), cFrameNumList, pcSliceHeader->getFrameNum() ) );
      }
      else
      {
        pcSliceHeader->getRplrBuffer( LIST_1 ).clear();
        pcSliceHeader->getRplrBuffer( LIST_1 ).setRefPicListReorderingFlag( false );
      }
    }
    else
    {
      ROF( pcSliceHeader->getSliceType() == P_SLICE );

      RNOK( xGetFrameNumList( *pcSliceHeader,                       cFrameNumList, LIST_0, uiFrameIdInGOP ) );
      RNOK( xSetRplr        ( pcSliceHeader->getRplrBuffer(LIST_0), cFrameNumList, pcSliceHeader->getFrameNum() ) );

      pcSliceHeader->getRplrBuffer( LIST_1 ).clear();
      pcSliceHeader->getRplrBuffer( LIST_1 ).setRefPicListReorderingFlag( false );
    }
  }

  return Err::m_nOK;
}






ErrVal
MCTFEncoder::xSetScalingFactors()
{
  for( UInt uiLevel = 0; uiLevel < m_uiDecompositionStages; uiLevel++ )
  {
    RNOK( xSetScalingFactors( uiLevel ) );
  }
  return Err::m_nOK;
}


ErrVal
MCTFEncoder::xSetScalingFactors( UInt uiBaseLevel )
{
  Double  adRateL0 [( 1 << MAX_DSTAGES )];
  Double  adRateL1 [( 1 << MAX_DSTAGES )];
  Double  adRateBi [( 1 << MAX_DSTAGES )];

  Double  dScalingBase    = m_pacControlData[0].getScalingFactor();
  Double  dScalingLowPass = 0.0;
  Int     iLowPassSize    = ( m_uiGOPSize >> uiBaseLevel );
  Int     iFrame;


  //===== get connection data =====
  for( iFrame = 1; iFrame <= iLowPassSize; iFrame += 2 )
  {
    RNOK( xGetConnections( adRateL0[iFrame], adRateL1[iFrame], adRateBi[iFrame] ) );
  }


  //===== get low-pass scaling =====
  for( iFrame = 0; iFrame <= iLowPassSize; iFrame += 2 )
  {
    Double  dScalLPCurr = 1.0;

    if( iFrame > 0 )
    {
      if( ( iFrame + 1 ) < iLowPassSize )
      {
        dScalLPCurr = ( adRateBi[iFrame-1] + adRateBi[iFrame+1] ) * ( FACTOR_53_LP - 1.0 ) / 2.0 +
                      ( adRateL1[iFrame-1] + adRateL0[iFrame+1] ) * ( FACTOR_22_LP - 1.0 ) / 2.0 + 1.0;
      }
      else
      {
        dScalLPCurr = ( adRateBi[iFrame-1] / 2.0 ) * ( FACTOR_53_LP - 1.0 ) +
                      ( adRateL1[iFrame-1]       ) * ( FACTOR_22_LP - 1.0 ) + 1.0;
      }
    }
    else
    {
      if( iLowPassSize )
      {
        dScalLPCurr = ( adRateBi[iFrame+1] / 2.0 ) * ( FACTOR_53_LP - 1.0 ) +
                      ( adRateL0[iFrame+1]       ) * ( FACTOR_22_LP - 1.0 ) + 1.0;
      }
    }

    dScalingLowPass += dScalLPCurr;
  }
  dScalingLowPass /= (Double)( 1 + ( iLowPassSize >> 1 ) );

  //{{Scaling factor Base Layer-France Telecom R&D (nathalie.cammas@francetelecom.com), JVTO045 and m11876
  Double dFactor53;
  Double dFactor22;
#if SCALING_FACTOR_HACK
  // heiko.schwarz@hhi.fhg.de: This is a bad hack for ensuring that the
  // closed-loop config files work and use identical scaling factor as
  // the MCTF version. The non-update scaling factors don't work and shall
  // be completely removed in future versions.
  if( m_uiLayerId == 0 && m_uiFrameWidthInMb <= 11 )
#else
  if( false )
#endif
  {
    dFactor53 = FACTOR_53_HP_BL;
    dFactor22 = FACTOR_22_HP_BL;
  }
  else
  {
    dFactor53 = FACTOR_53_HP;
    dFactor22 = FACTOR_22_HP;
  }
  //}}Scaling factor Base Layer


  //===== get high-pass scaling and set scaling factors =====
  for( iFrame = 0; iFrame <= iLowPassSize; iFrame++ )
  {
    Double dScal = dScalingBase;

    if( iFrame % 2 )
    {
      //===== high-pass pictures =====
      //{{Scaling factor Base Layer-France Telecom R&D (nathalie.cammas@francetelecom.com), JVTO045 and m11876
      dScal *= ( adRateBi[iFrame]                    ) * ( dFactor53 - 1.0 ) +
               ( adRateL0[iFrame] + adRateL1[iFrame] ) * ( dFactor22 - 1.0 ) + 1.0;
      //}}Scaling factor Base Layer
    }
    else
    {
      //===== low-pass pictures =====
      dScal *= dScalingLowPass;
    }
    m_pacControlData[ iFrame << uiBaseLevel ].setScalingFactor( dScal );
  }

  return Err::m_nOK;
}



ErrVal
MCTFEncoder::xClearBufferExtensions()
{
  for( UInt uiFrame = 0; uiFrame <= m_uiGOPSize; uiFrame++ )
  {
    RNOK( m_papcFrame   [uiFrame]->uninitHalfPel() );
    RNOK( m_papcResidual[uiFrame]->uninitHalfPel() );

    if( m_papcCLRecFrame )
    {
      RNOK( m_papcCLRecFrame[uiFrame]->uninitHalfPel() );
    }
  }

  for( UInt uiLayerIdx = 0; uiLayerIdx < m_uiNumLayers[0]; uiLayerIdx ++ )
    m_aapcFGSRecon[0][uiLayerIdx]->uninitHalfPel();

  return Err::m_nOK;
}



ErrVal
MCTFEncoder::xGetPredictionLists( RefFrameList& rcRefList0,
                                  RefFrameList& rcRefList1,
                                  UInt          uiBaseLevel,
                                  UInt          uiFrame,
                                  Bool          bHalfPel )
{
  rcRefList0.reset();
  rcRefList1.reset();

  UInt          uiFrameIdInGOP  = ( uiFrame << uiBaseLevel );
  SliceHeader*  pcSliceHeader   = m_pacControlData[uiFrameIdInGOP].getSliceHeader();
  UInt          uiList0Size     = pcSliceHeader->getNumRefIdxActive( LIST_0 );
  UInt          uiList1Size     = pcSliceHeader->getNumRefIdxActive( LIST_1 );

  //===== list 0 =====
  {
    Int iFrameId;
  for( iFrameId = Int( uiFrame - 1 ); iFrameId >= 0; iFrameId -= 2 )
    {
      IntFrame* pcFrame = m_papcFrame[ iFrameId << uiBaseLevel ];
    if(!pcFrame->getUnusedForRef())  // JVT-Q065 EIDR
    {
    if( ! pcFrame->isExtended() )
    {
      if( bHalfPel )
      {
      RNOK( xFillAndUpsampleFrame ( pcFrame ) );
      }
      else
      {
      RNOK( xFillAndExtendFrame   ( pcFrame ) );
      }
    }

    RNOK( rcRefList0.add( pcFrame ) );
    //bug-fix shenqiu EIDR{
    if(uiList0Size > 0)
    {
    uiList0Size--;
    }
    else
    {
      rcRefList0.decActive();
    }
    //bug-fix shenqiu EIDR}
    }
    }

// JVT-Q065 EIDR{
  //bug-fix shenqiu EIDR{
  for( iFrameId = Int( uiFrame + 1 ); iFrameId <= (Int)( m_uiGOPSize >> uiBaseLevel ) ; iFrameId += 2 )
  {
    IntFrame* pcFrame = m_papcFrame[ iFrameId << uiBaseLevel ];

    //----- create half-pel buffer -----
    if(!pcFrame->getUnusedForRef())
    {
      if( ! pcFrame->isExtended() )
      {
        if( bHalfPel )
        {
          RNOK( xFillAndUpsampleFrame ( pcFrame ) );
        }
        else
        {
          RNOK( xFillAndExtendFrame   ( pcFrame ) );
        }
      }

      RNOK( rcRefList0.add( pcFrame ) );

      if(uiList0Size > 0)
      {
      uiList0Size--;
    }
      else
      {
        rcRefList0.decActive();
      }
    }
  }
  //bug-fix shenqiu EIDR}
// JVT-Q065 EIDR}

  ROT( uiList0Size );
  }

  //===== list 1 =====
  {
    Int iFrameId;
  for( iFrameId = Int( uiFrame + 1 ); iFrameId <= (Int)( m_uiGOPSize >> uiBaseLevel ); iFrameId += 2 )
    {
      IntFrame* pcFrame = m_papcFrame[ iFrameId << uiBaseLevel ];

      //----- create half-pel buffer -----
    if(!pcFrame->getUnusedForRef()) // JVT-Q065 EIDR{
    {
    if( ! pcFrame->isExtended() )
    {
      if( bHalfPel )
      {
      RNOK( xFillAndUpsampleFrame ( pcFrame ) );
      }
      else
      {
      RNOK( xFillAndExtendFrame   ( pcFrame ) );
      }
    }

    RNOK( rcRefList1.add( pcFrame ) );
    //bug-fix shenqiu EIDR{
    if(uiList1Size > 0)
    {
    uiList1Size--;
    }
    else
    {
      rcRefList1.decActive();
    }
    //bug-fix shenqiu EIDR}
    }
    }
// JVT-Q065 EIDR{
  //bug-fix shenqiu EIDR{
  for( iFrameId = Int( uiFrame - 1 ); iFrameId >= 0; iFrameId -= 2 )
  {
    IntFrame* pcFrame = m_papcFrame[ iFrameId << uiBaseLevel ];

    if(!pcFrame->getUnusedForRef())
    {
      if( ! pcFrame->isExtended() )
      {
        if( bHalfPel )
        {
          RNOK( xFillAndUpsampleFrame ( pcFrame ) );
        }
        else
        {
          RNOK( xFillAndExtendFrame   ( pcFrame ) );
        }
      }

      RNOK( rcRefList1.add( pcFrame ) );

      if(uiList1Size > 0)
      {
      uiList1Size--;
    }
      else
      {
        rcRefList1.decActive();
      }
    }
  }
  //bug-fix shenqiu EIDR}
// JVT-Q065 EIDR}
  ROT( uiList1Size );
  }

// JVT-Q065 EIDR{
  //bug-fix shenqiu EIDR{
  if( rcRefList1.getSize() >= 2 && rcRefList0.getSize() == rcRefList1.getSize())
  {
    Bool bSwitch = true;
    for( UInt uiPos = 0; uiPos < rcRefList0.getSize(); uiPos++ )
    {
      if( rcRefList0.getEntry(uiPos) != rcRefList1.getEntry(uiPos) )
      {
        bSwitch = false;
        break;
      }
    }
    if( bSwitch )
    {
      rcRefList1.switchFirst();
    }
  }
  //bug-fix shenqiu EIDR}
// JVT-Q065 EIDR}

  return Err::m_nOK;
}


ErrVal
MCTFEncoder::xGetBQPredictionLists( RefFrameList& rcRefList0,
                                    RefFrameList& rcRefList1,
                                    UInt          uiBaseLevel,
                                    UInt          uiFrame )
{
  rcRefList0.reset();
  rcRefList1.reset();

  UInt          uiFrameIdInGOP  = ( uiFrame << uiBaseLevel );
  SliceHeader*  pcSliceHeader   = m_pacControlData[uiFrameIdInGOP].getSliceHeader();
  UInt          uiList0Size     = pcSliceHeader->getNumRefIdxActive( LIST_0 );
  UInt          uiList1Size     = pcSliceHeader->getNumRefIdxActive( LIST_1 );

  //===== list 0 =====
  {
    Int iFrameId;
  for( iFrameId = Int( uiFrame - 1 ); iFrameId >= 0; iFrameId -= 2 )
    {
      IntFrame* pcFrame = m_papcBQFrame[ iFrameId << uiBaseLevel ];

    if(!pcFrame->getUnusedForRef()) // JVT-Q065 EIDR
    {
      RNOK( xFillAndExtendFrame   ( pcFrame ) );

      RNOK( rcRefList0.add( pcFrame ) );
      //bug-fix shenqiu EIDR{
      if(uiList0Size > 0)
      {
      uiList0Size--;
    }
      else
      {
        rcRefList0.decActive();
      }
      //bug-fix shenqiu EIDR}
    }
    }
// JVT-Q065 EIDR{
  //bug-fix shenqiu EIDR{
  for( iFrameId = Int( uiFrame + 1 ); iFrameId <= (Int)( m_uiGOPSize >> uiBaseLevel ); iFrameId += 2 )
  {
    IntFrame* pcFrame = m_papcBQFrame[ iFrameId << uiBaseLevel ];
    if(!pcFrame->getUnusedForRef())
    {
      RNOK( xFillAndExtendFrame   ( pcFrame ) );

      RNOK( rcRefList0.add( pcFrame ) );

      if(uiList0Size > 0)
      {
      uiList0Size--;
    }
      else
      {
        rcRefList0.decActive();
      }
    }
  }
  //bug-fix shenqiu EIDR}
// JVT-Q065 EIDR}

    ROT( uiList0Size );
  }

  //===== list 1 =====
  {
    Int iFrameId;
  for( iFrameId = Int( uiFrame + 1 ); iFrameId <= (Int)( m_uiGOPSize >> uiBaseLevel ); iFrameId += 2 )
    {
      IntFrame* pcFrame = m_papcBQFrame[ iFrameId << uiBaseLevel ];
    if(!pcFrame->getUnusedForRef()) // JVT-Q065 EIDR
    {
    RNOK( xFillAndExtendFrame   ( pcFrame ) );

    RNOK( rcRefList1.add( pcFrame ) );
    //bug-fix shenqiu EIDR{
    if(uiList1Size > 0)
    {
    uiList1Size--;
    }
    else
    {
      rcRefList1.decActive();
    }
    //bug-fix shenqiu EIDR}
    }
    }

// JVT-Q065 EIDR{
  //bug-fix shenqiu EIDR{
  for( iFrameId = Int( uiFrame - 1 ); iFrameId >= 0; iFrameId -= 2 )
  {
    IntFrame* pcFrame = m_papcBQFrame[ iFrameId << uiBaseLevel ];
    if(!pcFrame->getUnusedForRef())
    {
      RNOK( xFillAndExtendFrame   ( pcFrame ) );

      RNOK( rcRefList1.add( pcFrame ) );

      if(uiList1Size > 0)
      {
      uiList1Size--;
    }
      else
      {
        rcRefList1.decActive();
      }
  }
  }
  //bug-fix shenqiu EIDR}
// JVT-Q065 EIDR}

    ROT( uiList1Size );
  }

// JVT-Q065 EIDR{
  //bug-fix shenqiu EIDR{
  if( rcRefList1.getSize() >= 2 && rcRefList0.getSize() == rcRefList1.getSize())
  {
    Bool bSwitch = true;
    for( UInt uiPos = 0; uiPos < rcRefList0.getSize(); uiPos++ )
    {
      if( rcRefList0.getEntry(uiPos) != rcRefList1.getEntry(uiPos) )
      {
        bSwitch = false;
        break;
      }
    }
    if( bSwitch )
    {
      rcRefList1.switchFirst();
    }
  }
  //bug-fix shenqiu EIDR}
// JVT-Q065 EIDR}
  return Err::m_nOK;
}



ErrVal
MCTFEncoder::xGetCLRecPredictionLists( RefFrameList& rcRefList0,
                                       RefFrameList& rcRefList1,
                                       UInt          uiBaseLevel,
                                       UInt          uiFrame,
                                       Bool          bHalfPel )
{
  rcRefList0.reset();
  rcRefList1.reset();

  UInt          uiFrameIdInGOP  = ( uiFrame << uiBaseLevel );
  SliceHeader*  pcSliceHeader   = m_pacControlData[uiFrameIdInGOP].getSliceHeader();
  UInt          uiList0Size     = pcSliceHeader->getNumRefIdxActive( LIST_0 );
  UInt          uiList1Size     = pcSliceHeader->getNumRefIdxActive( LIST_1 );

  //===== list 0 =====
  {
    Int iFrameId;
  for( iFrameId = Int( uiFrame - 1 ); iFrameId >= 0; iFrameId -= 2 )
    {
      IntFrame* pcFrame = m_papcCLRecFrame[ iFrameId << uiBaseLevel ];

    if(!pcFrame->getUnusedForRef()) // JVT-Q065 EIDR
    {
    if( ! pcFrame->isExtended() )
    {
      if( bHalfPel )
      {
      RNOK( xFillAndUpsampleFrame ( pcFrame ) );
      }
      else
      {
      RNOK( xFillAndExtendFrame   ( pcFrame ) );
      }
    }

    RNOK( rcRefList0.add( pcFrame ) );
    //bug-fix shenqiu EIDR{
    if(uiList0Size > 0)
    {
    uiList0Size--;
    }
    else
    {
      rcRefList0.decActive();
    }
    //bug-fix shenqiu EIDR}
    }
    }

// JVT-Q065 EIDR{
  //bug-fix shenqiu EIDR{
  for( iFrameId = Int( uiFrame + 1 ); iFrameId <= (Int)( m_uiGOPSize >> uiBaseLevel ); iFrameId += 2 )
  {
    IntFrame* pcFrame = m_papcCLRecFrame[ iFrameId << uiBaseLevel ];
    if(!pcFrame->getUnusedForRef())
    {
      //----- create half-pel buffer -----
      if( ! pcFrame->isExtended() )
      {
        if( bHalfPel )
        {
          RNOK( xFillAndUpsampleFrame ( pcFrame ) );
        }
        else
        {
          RNOK( xFillAndExtendFrame   ( pcFrame ) );
        }
      }

      RNOK( rcRefList0.add( pcFrame ) );
      if(uiList0Size > 0)
      {
      uiList0Size--;
    }
      else
      {
        rcRefList0.decActive();
      }
    }
  }
  //bug-fix shenqiu EIDR}
// JVT-Q065 EIDR}
  ROT( uiList0Size );
  }

  //===== list 1 =====
  {
    Int iFrameId;
  for( iFrameId = Int( uiFrame + 1 ); iFrameId <= (Int)( m_uiGOPSize >> uiBaseLevel ); iFrameId += 2 )
    {
      IntFrame* pcFrame = m_papcCLRecFrame[ iFrameId << uiBaseLevel ];
    if(!pcFrame->getUnusedForRef()) // JVT-Q065 EIDR
    {
    //----- create half-pel buffer -----
    if( ! pcFrame->isExtended() )
    {
      if( bHalfPel )
      {
      RNOK( xFillAndUpsampleFrame ( pcFrame ) );
      }
      else
      {
      RNOK( xFillAndExtendFrame   ( pcFrame ) );
      }
    }

    RNOK( rcRefList1.add( pcFrame ) );
    //bug-fix shenqiu EIDR{
    if(uiList1Size > 0)
    {
    uiList1Size--;
    }
    else
    {
      rcRefList1.decActive();
    }
    //bug-fix shenqiu EIDR}
    }
    }

// JVT-Q065 EIDR{
  //bug-fix shenqiu EIDR{
  for( iFrameId = Int( uiFrame - 1 ); iFrameId >= 0; iFrameId -= 2 )
  {
    IntFrame* pcFrame = m_papcCLRecFrame[ iFrameId << uiBaseLevel ];

    if(!pcFrame->getUnusedForRef()) // JVT-Q065 EIDR
    {
      if( ! pcFrame->isExtended() )
      {
        if( bHalfPel )
        {
        RNOK( xFillAndUpsampleFrame ( pcFrame ) );
      }
        else
        {
        RNOK( xFillAndExtendFrame   ( pcFrame ) );
        }
    }

      RNOK( rcRefList1.add( pcFrame ) );
      if(uiList1Size > 0)
    {
        uiList1Size--;
    }
      else
      {
        rcRefList1.decActive();
      }
  }
  }
  //bug-fix shenqiu EIDR}
// JVT-Q065 EIDR}

  ROT( uiList1Size );
  }
// JVT-Q065 EIDR{
  //bug-fix shenqiu EIDR{
  if( rcRefList1.getSize() >= 2 && rcRefList0.getSize() == rcRefList1.getSize())
  {
    Bool bSwitch = true;
    for( UInt uiPos = 0; uiPos < rcRefList0.getSize(); uiPos++ )
    {
      if( rcRefList0.getEntry(uiPos) != rcRefList1.getEntry(uiPos) )
      {
        bSwitch = false;
        break;
      }
    }
    if( bSwitch )
    {
      rcRefList1.switchFirst();
    }
  }
  //bug-fix shenqiu EIDR}
// JVT-Q065 EIDR}
  return Err::m_nOK;
}




ErrVal
MCTFEncoder::xInitBaseLayerData( ControlData& rcControlData,
                                 UInt          uiBaseLevel, //TMM_ESS
                                 UInt          uiFrame,     //TMM_ESS
                                 Bool bMotion
                 )
{
  //===== init =====
  rcControlData.setBaseLayerRec ( 0 );
  rcControlData.setBaseLayerSbb ( 0 );
  rcControlData.setBaseLayerCtrl( 0 );


  IntFrame*     pcBaseFrame         = 0;
  IntFrame*     pcBaseResidual      = 0;
  MbDataCtrl*   pcBaseDataCtrl      = 0;
  MbDataCtrl*   pcBaseDataCtrlEL    = 0;

  Bool          bConstrainedIPredBL = false;
  Bool          bForCopyOnly        = false;
  Bool          bBaseDataAvailable  = false;

  if( rcControlData.getBaseLayerIdMotion() != MSYS_UINT_MAX )
  {
    RNOK( m_pcH264AVCEncoder->getBaseLayerData( pcBaseFrame,
                                                pcBaseResidual,
                                                pcBaseDataCtrl,
                                                pcBaseDataCtrlEL,
                                                bConstrainedIPredBL,
                                                bForCopyOnly,
                                                rcControlData.getSpatialScalabilityType (),
                                                rcControlData.getBaseLayerIdMotion      (),
                                                rcControlData.getSliceHeader()->getPoc  (),
                                                bMotion  ));

    bBaseDataAvailable = pcBaseFrame && pcBaseResidual && pcBaseDataCtrl;
  }


  //===== motion data =====
  if( pcBaseDataCtrl )
  {
    RNOK( m_pcBaseLayerCtrl->initSlice( *rcControlData.getSliceHeader(), PRE_PROCESS, false, NULL ) );
    RNOK( pcBaseDataCtrl->switchMotionRefinement() );

    if( m_pcResizeParameters->m_iExtendedSpatialScalability == ESS_PICT )
      {
        RNOK( xFillPredictionLists_ESS( uiBaseLevel, uiFrame) );
      }

    // ICU/ETRI FGS_MOT_USE
    if (rcControlData.getSliceHeader()->getSliceType() == I_SLICE)
    {
      RNOK( m_pcBaseLayerCtrl->upsampleMotion( *pcBaseDataCtrl, (bForCopyOnly ? NULL : m_pcResizeParameters) ) );
    }
    else
    {
      RNOK( m_pcBaseLayerCtrl->upsampleMotion( *pcBaseDataCtrlEL, (bForCopyOnly ? NULL : m_pcResizeParameters) ) );
    }

    RNOK( pcBaseDataCtrl->switchMotionRefinement() );

    rcControlData.setBaseLayerCtrl( m_pcBaseLayerCtrl );

    rcControlData.getSliceHeader()->setBaseLayerUsesConstrainedIntraPred( bConstrainedIPredBL );

//TMM_WP
    SliceHeader *pcSliceHeaderCurr, *pcSliceHeaderBase;
    pcSliceHeaderCurr = rcControlData.getSliceHeader();
    pcSliceHeaderBase = pcBaseDataCtrl->getSliceHeader();

    //indicates whether the base layer use wp or not
    m_bBaseLayerWp = pcSliceHeaderBase->getPPS().getWeightedPredFlag();

    /* copy LIST_0 wp */
    pcSliceHeaderCurr->copyWeightedPred(pcSliceHeaderBase->getPredWeightTable(LIST_0),
                                        pcSliceHeaderBase->getLumaLog2WeightDenom(),
                                        pcSliceHeaderBase->getChromaLog2WeightDenom(),
                                        LIST_0, false);

    /* copy LIST_1 wp */
    pcSliceHeaderCurr->copyWeightedPred(pcSliceHeaderBase->getPredWeightTable(LIST_1),
                                        pcSliceHeaderBase->getLumaLog2WeightDenom(),
                                        pcSliceHeaderBase->getChromaLog2WeightDenom(),
                                        LIST_1, false);
//TMM_WP
  }


  //===== residual data =====
  if( bBaseDataAvailable )
  {
    RNOK( m_pcBaseLayerResidual->copy( pcBaseResidual ) );
    // TMM_ESS
    m_pcBaseLayerResidual->upsampleResidual(m_cDownConvert, m_pcResizeParameters, pcBaseDataCtrl, false);

    rcControlData.setBaseLayerSbb( m_pcBaseLayerResidual );
  }


  //==== reconstructed (intra) data =====
  if( bBaseDataAvailable )
  {
	  //JVT-U106 Behaviour at slice boundaries{
	  if(!m_pbIntraBLFlag)
		  m_pbIntraBLFlag=new Bool[rcControlData.getMbDataCtrl()->getSize()];
	  for(UInt i=0;i<rcControlData.getMbDataCtrl()->getSize();i++)
		  m_pbIntraBLFlag[i]=true;
	  m_pcSliceEncoder->setIntraBLFlag(m_pbIntraBLFlag);
	  if(m_bCIUFlag)
	  {
		  xConstrainedIntraUpsampling(pcBaseFrame,m_pcBaseLayerFrame,m_apcFrameTemp[0],pcBaseDataCtrl,m_pcReconstructionBypass,m_pcResizeParameters);
	  }
	  else
	  {
		  RNOK( m_pcBaseLayerFrame->copy( pcBaseFrame ) );
		  // TMM_ESS
		  m_pcBaseLayerFrame->upsample(m_cDownConvert, m_pcResizeParameters, true);
	  }
	  //JVT-U106 Behaviour at slice boundaries}
	  
      //JVT-R057 LA-RDO{
	  if(m_bLARDOEnable)
		  m_pcBaseLayerFrame->setChannelDistortion(pcBaseFrame);
	  //JVT-R057 LA-RDO}
	  rcControlData.setBaseLayerRec( m_pcBaseLayerFrame );
  }

  setMCResizeParameters(m_pcResizeParameters);

  return Err::m_nOK;
}

Void MCTFEncoder::setMCResizeParameters   (ResizeParameters*				resizeParameters)
{
  m_pcMotionEstimation->setResizeParameters(resizeParameters);
} 


ErrVal
MCTFEncoder::xInitControlDataMotion( UInt uiBaseLevel,
                                     UInt uiFrame,
                                     Bool bMotionEstimation )
{
  UInt            uiFrameIdInGOP    = uiFrame << uiBaseLevel;
  ControlData&    rcControlData     = m_pacControlData[uiFrameIdInGOP];
  SliceHeader*    pcSliceHeader     = rcControlData.getSliceHeader  ();
  Double          dScalFactor       = rcControlData.getScalingFactor();
  Double          dQpPredData       = m_adBaseQpLambdaMotion[ uiBaseLevel ] - 6.0 * log10( dScalFactor ) / log10( 2.0 );
  Double          dLambda           = 0.85 * pow( 2.0, min( 52.0, dQpPredData ) / 3.0 - 4.0 );
  Int             iQp               = max( MIN_QP, min( MAX_QP, (Int)floor( dQpPredData + 0.5 ) ) );

  pcSliceHeader->setSliceHeaderQp( iQp );
  rcControlData. setLambda       ( dLambda );

  if( bMotionEstimation )
  {
    //TMM_ESS
    RNOK( xInitBaseLayerData( rcControlData,uiBaseLevel, uiFrame,true) );
  }

  return Err::m_nOK;
}


ErrVal
MCTFEncoder::xInitControlDataLowPass( UInt uiFrameIdInGOP,
                                     UInt          uiBaseLevel,//TMM_ESS
                                     UInt          uiFrame)    //TMM_ESS
{
  ControlData&  rcControlData   = m_pacControlData[ uiFrameIdInGOP ];
  SliceHeader*  pcSliceHeader   = rcControlData.getSliceHeader();

  Double        dScalFactor     = rcControlData.getScalingFactor();
  Double        dQP             = m_dBaseQpLambdaMotionLP - 6.0 * log10( dScalFactor ) / log10( 2.0 );
  Double        dLambda         = 0.85 * pow( 2.0, min( 52.0, dQP ) / 3.0 - 4.0 );
  dQP                           = m_dBaseQPResidual - 6.0 * log10( dScalFactor ) / log10( 2.0 );
  Int           iQP             = max( MIN_QP, min( MAX_QP, (Int)floor( dQP + 0.5 ) ) );

  pcSliceHeader->setSliceHeaderQp ( iQP );
  rcControlData. setLambda        ( dLambda );

  RNOK( xInitBaseLayerData( rcControlData,uiBaseLevel,uiFrame) );//TMM_ESS

//TMM_WP
  rcControlData.getPrdFrameList ( LIST_0 ).reset();
  RNOK( rcControlData.getPrdFrameList ( LIST_0 ).add  ( m_pcLowPassBaseReconstruction ) );
  RefFrameList& rcRefFrameList0 = rcControlData.getPrdFrameList( LIST_0 );
  RefFrameList& rcRefFrameList1 = rcControlData.getPrdFrameList( LIST_1 );

  /* el & has correspongding bl frame & bl does wp then use bl wts */
  if((m_uiLayerId > 0 && m_bBaseLayerWp &&
      rcControlData.getBaseLayerIdMotion() != MSYS_UINT_MAX) )
  {
      /* use same wts as bl */
      pcSliceHeader->setBasePredWeightTableFlag(true);
  }
  else
  {
      /* call function to calculate the weights */
      m_pcSliceEncoder->xSetPredWeights( *pcSliceHeader,
                                         m_papcFrame[uiFrameIdInGOP],
                                         rcRefFrameList0,
                                         rcRefFrameList1
                                         //m_pcLowPassBaseReconstruction,
                                         //NULL,
                                         );
      pcSliceHeader->setBasePredWeightTableFlag(false);
  }

  rcControlData.getPrdFrameList ( LIST_0 ).reset();
//TMM_WP

  return Err::m_nOK;
}


ErrVal
MCTFEncoder::xInitControlDataHighPass( UInt uiFrameIdInGOP,
                                      UInt          uiBaseLevel,//TMM_ESS
                                      UInt          uiFrame)    //TMM_ESS
{
  ControlData&  rcControlData   = m_pacControlData[ uiFrameIdInGOP ];
  SliceHeader*  pcSliceHeader   = rcControlData.getSliceHeader();

  Double        dScalFactor     = rcControlData.getScalingFactor();
  Double        dQP             = m_dBaseQPResidual - 6.0 * log10( dScalFactor ) / log10( 2.0 );
  Double        dLambda         = 0.85 * pow( 2.0, min( 52.0, dQP ) / 3.0 - 4.0 );
  Int           iQP             = max( MIN_QP, min( MAX_QP, (Int)floor( dQP + 0.5 ) ) );

  pcSliceHeader->setSliceHeaderQp ( iQP );
  rcControlData. setLambda        ( dLambda );

  RNOK( xInitBaseLayerData( rcControlData,uiBaseLevel, uiFrame) );

  return Err::m_nOK;
}



ErrVal
MCTFEncoder::xMotionEstimationFrame( UInt uiBaseLevel, UInt uiFrame )
{
  UInt          uiFrameIdInGOP  = uiFrame << uiBaseLevel;
  ControlData&  rcControlData   = m_pacControlData[uiFrameIdInGOP];
  IntFrame*     pcFrame         = m_papcFrame     [uiFrameIdInGOP];
  IntFrame*     pcIntraRecFrame = m_apcFrameTemp  [0];
  SliceHeader*  pcSliceHeader   = rcControlData.getSliceHeader ();
  RefFrameList& rcRefFrameList0 = rcControlData.getPrdFrameList( LIST_0 );
  RefFrameList& rcRefFrameList1 = rcControlData.getPrdFrameList( LIST_1 );

  //===== get reference frame lists =====
  if( m_papcCLRecFrame )
  {
    RNOK( xGetCLRecPredictionLists( rcRefFrameList0, rcRefFrameList1, uiBaseLevel, uiFrame, true ) );
  }
  else
  {
    RNOK( xGetPredictionLists     ( rcRefFrameList0, rcRefFrameList1, uiBaseLevel, uiFrame, true ) );
  }

  //===== set lambda and QP =====
  RNOK( xInitControlDataMotion( uiBaseLevel, uiFrame, true ) );

  //S051{
  m_pcSliceEncoder->setUseBDir(true);
  if(m_bEncSIP)
  {
    if(m_bH264AVCCompatible||!rcControlData.getSliceHeader()->getDirectSpatialMvPredFlag())
    {
      int        pos          = xGetMbDataCtrlL1Pos( *rcControlData.getSliceHeader(), uiFrameIdInGOP );
      if(pos!=-1)
      {
        SliceHeader* pcSliceHeaderL1     = m_pacControlData[pos].getSliceHeader  ();
        if(xSIPCheck(pcSliceHeaderL1->getPoc()))
          m_pcSliceEncoder->setUseBDir(false);
      }
    }
  }
  //S051}

//TMM_WP
  /* el & has correspongding bl frame & bl uses wp then use bl wts */
  if((m_uiLayerId > 0 && m_bBaseLayerWp &&
      rcControlData.getBaseLayerIdMotion() != MSYS_UINT_MAX) )
  {
    /* use same wts as bl */
    pcSliceHeader->setBasePredWeightTableFlag(true);
  }
  else
  {
    /* call function to calculate the weights */
    m_pcSliceEncoder->xSetPredWeights( *pcSliceHeader,
                                        pcFrame,
                                        rcRefFrameList0,
                                        rcRefFrameList1
                                        //rcRefFrameList0.getEntry(0),
                                        //rcRefFrameList1.getEntry(0),
                                        );
    pcSliceHeader->setBasePredWeightTableFlag(false);
  }
//TMM_WP
  // JVT-R057 LA-RDO{
  if(m_bLARDOEnable)
  {
    pcFrame->initChannelDistortion();
    m_pcSliceEncoder->getMbEncoder()->setFrameEcEp(m_papcFrame[(uiFrame-1)<<uiBaseLevel]);
  }
  // JVT-R057 LA-RDO}
  //===== motion estimation =====
  RNOK( xMotionEstimation     ( &rcRefFrameList0, &rcRefFrameList1,
                                pcFrame, pcIntraRecFrame, rcControlData,
                                // JVT-S054 (REPLACE)
                                //m_bBiPredOnly, m_uiNumMaxIter, m_uiIterSearchRange ) );
                                m_bBiPredOnly, m_uiNumMaxIter, m_uiIterSearchRange, uiFrameIdInGOP ) );

  return Err::m_nOK;
}


ErrVal
MCTFEncoder::xDecompositionFrame( UInt uiBaseLevel, UInt uiFrame )
{
  UInt          uiFrameIdInGOP  = uiFrame << uiBaseLevel;
  ControlData&  rcControlData   = m_pacControlData[uiFrameIdInGOP];
  IntFrame*     pcFrame         = m_papcFrame     [uiFrameIdInGOP];
  IntFrame*     pcResidual      = m_papcResidual  [uiFrameIdInGOP];
  IntFrame*     pcMCFrame       = m_apcFrameTemp  [0];
  IntFrame*     pcBQFrame       = 0;
  RefFrameList& rcRefFrameList0 = rcControlData.getPrdFrameList( LIST_0 );
  RefFrameList& rcRefFrameList1 = rcControlData.getPrdFrameList( LIST_1 );
  RefFrameList  acBQRefFrameList[2];

  
  //===== get reference frame lists =====
  if( m_papcBQFrame )
  {
    pcBQFrame = m_papcBQFrame[uiFrameIdInGOP];
    RNOK( xGetBQPredictionLists ( acBQRefFrameList[0], acBQRefFrameList[1], uiBaseLevel, uiFrame ) );
  }
  if( m_papcCLRecFrame )
  {
    RNOK( xGetCLRecPredictionLists( rcRefFrameList0, rcRefFrameList1, uiBaseLevel, uiFrame, false ) );
  }
  else
  {
    RNOK( xGetPredictionLists     ( rcRefFrameList0, rcRefFrameList1, uiBaseLevel, uiFrame, false ) );
  }

  //===== set lambda and QP =====
  RNOK( xInitControlDataMotion( uiBaseLevel, uiFrame, true ) );

  setBaseMbDataCtrl(rcControlData.getBaseLayerCtrl());

  IntFrame*     pcSRFrame       = m_papcSmoothedFrame [uiFrameIdInGOP];

  //--- closed-loop control for base quality layer ---
  if( pcBQFrame )
  {
    RNOK( xMotionCompensation   ( pcMCFrame, &acBQRefFrameList[0], &acBQRefFrameList[1],
                                  rcControlData.getMbDataCtrl(), *rcControlData.getSliceHeader() ) );
    RNOK( pcBQFrame->prediction ( pcMCFrame, pcFrame ) );
  }

  //===== prediction =====
  RNOK( xMotionCompensation   ( pcMCFrame, &rcRefFrameList0, &rcRefFrameList1,
                                rcControlData.getMbDataCtrl(), *rcControlData.getSliceHeader() ) );
  RNOK( pcFrame ->prediction  ( pcMCFrame, pcFrame ) );

  // store the new prediction frame that is obtained by invoking the special MC 
  if( rcControlData.getBaseLayerCtrl() )
  {
    // obtain base-layer data
    RNOK( xInitBaseLayerData( rcControlData, uiBaseLevel, uiFrame, true ) );

    if( pcBQFrame ) 
    {
      RNOK( xMotionCompensationSRFrame   ( pcSRFrame, &acBQRefFrameList[0], &acBQRefFrameList[1],
        rcControlData.getMbDataCtrl(), *rcControlData.getSliceHeader(), 
        rcControlData.getBaseLayerCtrl() ) );
    }
    else
    {
      RNOK( xMotionCompensationSRFrame   ( pcSRFrame, &rcRefFrameList0, &rcRefFrameList1,
        rcControlData.getMbDataCtrl(), *rcControlData.getSliceHeader(), 
        rcControlData.getBaseLayerCtrl() ) );
    }
  }

  //===== set residual =====
  RNOK( pcResidual->copy      ( pcFrame ) );
  RNOK( xZeroIntraMacroblocks ( pcResidual, rcControlData ) );

  return Err::m_nOK;
}



ErrVal
MCTFEncoder::xCompositionFrame( UInt uiBaseLevel, UInt uiFrame, PicBufferList& rcPicBufferInputList )
{
  UInt          uiFrameIdInGOP  = uiFrame << uiBaseLevel;
  ControlData&  rcControlData   = m_pacControlData[uiFrameIdInGOP];
  IntFrame*     pcFrame         = m_papcFrame     [uiFrameIdInGOP];
  IntFrame*     pcResidual      = m_papcResidual  [uiFrameIdInGOP]; // Hanke@RWTH
  IntFrame*     pcMCFrame       = m_apcFrameTemp  [0];

  //-- JVT-R091
  // obtain base-layer data
  if ( rcControlData.getBaseLayerId() != MSYS_UINT_MAX )
  {
    xInitBaseLayerData( rcControlData, uiBaseLevel, uiFrame, true );
  }
  //--

  setBaseMbDataCtrl(rcControlData.getBaseLayerCtrl());

  //--- closed-loop coding of base quality layer ---
  IntFrame*     pcBQFrame       = 0;
  IntFrame*     pcBQResidual    = 0;
  RefFrameList  acBQRefFrameList[2];
  if( m_papcBQFrame )
  {
    pcBQFrame       = m_papcBQFrame   [uiFrameIdInGOP];
    pcBQResidual    = m_apcFrameTemp  [1];
    RNOK( xGetBQPredictionLists       ( acBQRefFrameList[0], acBQRefFrameList[1], uiBaseLevel, uiFrame ) );

    RNOK( pcBQResidual->copy( pcBQFrame ) );
    RNOK( xZeroIntraMacroblocks( pcBQResidual, rcControlData ) );

    //===== prediction =====
    RNOK( rcControlData.switchBQLayerQpAndCbp() );
    RNOK( xMotionCompensation         ( pcMCFrame, &acBQRefFrameList[0], &acBQRefFrameList[1],
                                        rcControlData.getMbDataCtrl(), *rcControlData.getSliceHeader(), true ) );
    RNOK( rcControlData.switchBQLayerQpAndCbp() );
    RNOK( pcBQFrame->inversePrediction( pcMCFrame, pcBQFrame ) );
  }

  //--- highest FGS reference for closed-loop coding ---
  IntFrame*     pcCLRecFrame       = 0;
  IntFrame*     pcCLRecResidual    = 0;
  RefFrameList  acCLRecRefFrameList[2];
  if( m_papcCLRecFrame )
  {
    pcCLRecFrame       = m_papcCLRecFrame   [uiFrameIdInGOP];
    pcCLRecResidual    = m_apcFrameTemp     [2];
    RNOK( xGetCLRecPredictionLists       ( acCLRecRefFrameList[0], acCLRecRefFrameList[1], uiBaseLevel, uiFrame ) );

    RNOK( pcCLRecResidual->copy( pcCLRecFrame ) );
    RNOK( xZeroIntraMacroblocks( pcCLRecResidual, rcControlData ) );

    //===== prediction =====
    RNOK( xMotionCompensation         ( pcMCFrame, &acCLRecRefFrameList[0], &acCLRecRefFrameList[1],
                                        rcControlData.getMbDataCtrl(), *rcControlData.getSliceHeader(), true ) );
    RNOK( pcCLRecFrame->inversePrediction( pcMCFrame, pcCLRecFrame ) );
  }

  //===== get reference frame lists =====
  RefFrameList& rcRefFrameList0 = rcControlData.getPrdFrameList( LIST_0 );
  RefFrameList& rcRefFrameList1 = rcControlData.getPrdFrameList( LIST_1 );
  RNOK( xGetPredictionLists         ( rcRefFrameList0, rcRefFrameList1,
                                      uiBaseLevel, uiFrame, false ) );

  //===== prediction =====
  RNOK( xMotionCompensation         ( pcMCFrame, &rcRefFrameList0, &rcRefFrameList1,
                                      rcControlData.getMbDataCtrl(), *rcControlData.getSliceHeader(), true ) );
  RNOK( pcFrame ->inversePrediction ( pcMCFrame, pcFrame ) );
  //--

  //----- store non-deblocked signal for inter-layer prediction -----
  RNOK( m_papcSubband[uiFrameIdInGOP]->copy( pcFrame ) );

  // JVT-R057 LA-RDO{
  if(m_bLARDOEnable)
  {
    m_papcSubband[uiFrameIdInGOP]->setChannelDistortion(pcFrame);
    if(m_papcCLRecFrame)
      m_papcCLRecFrame[uiFrameIdInGOP]->setChannelDistortion(pcFrame);
  }
  // JVT-R057 LA-RDO}

  //===== de-blocking =====
  // Hanke@RWTH: set pointer to current residual frame
  m_pcLoopFilter->setHighpassFramePointer( pcResidual );
  RNOK( m_pcLoopFilter->process     ( *rcControlData.getSliceHeader(),
                                       pcFrame,
                                       rcControlData.getMbDataCtrl (),
                                       rcControlData.getMbDataCtrl (),
                                       m_uiFrameWidthInMb,
                                       &rcRefFrameList0,
                                       &rcRefFrameList1,
                                       true,
                                       rcControlData.getSpatialScalability()) );  // SSUN@SHARP


  //--- highest FGS reference for closed-loop coding ---
  if( pcCLRecFrame )
  {
    RNOK( rcControlData.activateMbDataCtrlForQpAndCbp( true ) );
    m_pcLoopFilter->setHighpassFramePointer( pcCLRecResidual );

    RNOK( m_pcLoopFilter->process( *rcControlData.getSliceHeader(),
                                    pcCLRecFrame,
                                    rcControlData.getMbDataCtrl (),
                                    rcControlData.getMbDataCtrl (),
                                    m_uiFrameWidthInMb,
                                    &acCLRecRefFrameList[0],
                                    &acCLRecRefFrameList[1],
                                    true,
                                    rcControlData.getSpatialScalability()) );  // SSUN@SHARP
  }

  //--- closed-loop coding of base quality layer ---
  if( pcBQFrame )
  {
    RNOK( rcControlData.switchBQLayerQpAndCbp() );
    m_pcLoopFilter->setHighpassFramePointer( pcBQResidual );

    RNOK( m_pcLoopFilter->process( *rcControlData.getSliceHeader(),
                                    pcBQFrame,
                                    rcControlData.getMbDataCtrl (),
                                    rcControlData.getMbDataCtrl (),
                                    m_uiFrameWidthInMb,
                                    &acBQRefFrameList[0],
                                    &acBQRefFrameList[1],
                                    true,
                                    rcControlData.getSpatialScalability()) );  // SSUN@SHARP

    RNOK( rcControlData.switchBQLayerQpAndCbp() );
  }

  return Err::m_nOK;
}

ErrVal
MCTFEncoder::xFixOrgResidual( IntFrame*      pcFrame,
                              IntFrame*      pcOrgPred,
                              IntFrame*      pcResidual,
                              IntFrame*      pcSRFrame,
                              ControlData&  rcCtrlData )
{
  MbDataCtrl*       pcMbDataCtrl  = rcCtrlData.getMbDataCtrl       ();
  SliceHeader*      pcSliceHeader = rcCtrlData.getSliceHeader      ();
  IntYuvPicBuffer*  pcPicBuffer   = pcFrame  ->getFullPelYuvBuffer ();

  RNOK( pcMbDataCtrl->initSlice( *pcSliceHeader, PRE_PROCESS, false, NULL ) );

  IntYuvMbBuffer cMbBuffer, cResMbBuffer, cPrdMbBuffer, cRecMbBuffer;
  for( UInt uiMbIndex = 0; uiMbIndex < m_uiMbNumber; uiMbIndex++ )
  {
    UInt          uiMbY           = uiMbIndex / m_uiFrameWidthInMb;
    UInt          uiMbX           = uiMbIndex % m_uiFrameWidthInMb;
    MbDataAccess* pcMbDataAccess  = 0;

    RNOK( pcMbDataCtrl            ->initMb( pcMbDataAccess, uiMbY, uiMbX ) );
    RNOK( m_pcYuvFullPelBufferCtrl->initMb(                 uiMbY, uiMbX ) );

    if( pcMbDataAccess->getMbData().getSmoothedRefFlag() )
    {
      // load macroblocks
      cRecMbBuffer.loadBuffer( pcSRFrame  ->getFullPelYuvBuffer()  );
      cResMbBuffer.loadBuffer( pcResidual  ->getFullPelYuvBuffer()  );
      cPrdMbBuffer.loadBuffer( pcOrgPred  ->getFullPelYuvBuffer()  );

      // O
      cMbBuffer    .loadBuffer( pcPicBuffer                        );
      cMbBuffer    .add       ( cPrdMbBuffer                        );

      // O-(Rec-Res) = O-P = R
      cMbBuffer    .subtract   ( cRecMbBuffer                        );
      cMbBuffer    .add       ( cResMbBuffer                        );

      // store to pcFrame
      pcPicBuffer->loadBuffer( &cMbBuffer                          );
    }
  }

  return Err::m_nOK;
}
//--


ErrVal
MCTFEncoder::xEncodeKeyPicture( Bool&               rbKeyPicCoded,
                                UInt                uiFrame,
                                AccessUnitList&     rcAccessUnitList,
                                PicOutputDataList&  rcPicOutputDataList )
{
  rbKeyPicCoded         = false;
  UInt   uiFrameIdInGOP = uiFrame << m_uiDecompositionStages;
  ROTRS( uiFrameIdInGOP > m_uiGOPSize, Err::m_nOK );

  //===== check for first GOP =====
  if( uiFrame == 0 && m_uiGOPNumber )
  {
    //====== don't code first anchor picture if it was coded within the last GOP =====
    //bug-fix shenqiu EIDR{
    m_pcAnchorFrameReconstructed->setUnusedForRef(m_papcFrame[ uiFrameIdInGOP ]->getUnusedForRef());
    //bug-fix shenqiu EIDR}
    RNOK( m_papcFrame[ uiFrameIdInGOP ] ->copyAll( m_pcAnchorFrameReconstructed  ) );
    // JVT-R057 LA-RDO{
    if(m_bLARDOEnable)
    {
      m_papcFrame[uiFrameIdInGOP]->copyChannelDistortion(m_pcLowPassBaseReconstruction);
      m_papcSubband[uiFrameIdInGOP]->setChannelDistortion(m_papcFrame[uiFrameIdInGOP]);
    }
    // JVT-R057 LA-RDO}
    return Err::m_nOK;
  }

  rbKeyPicCoded                         = true;
  UInt                    uiBits        = 0;
  ControlData&            rcControlData = m_pacControlData[ uiFrameIdInGOP ];
  SliceHeader*            pcSliceHeader = rcControlData.getSliceHeader();
  MbDataCtrl*             pcMbDataCtrl  = rcControlData.getMbDataCtrl ();
  IntFrame*               pcFrame       = m_papcFrame     [ uiFrameIdInGOP ];
  IntFrame*               pcResidual    = m_papcResidual  [ uiFrameIdInGOP ];
  IntFrame*               pcPredSignal  = m_apcFrameTemp  [ 0 ];
  IntFrame*               pcBLRecFrame  = m_apcFrameTemp  [ 1 ];
  AccessUnit&             rcAccessUnit  = rcAccessUnitList.getAccessUnit  ( pcSliceHeader->getPoc() );
  ExtBinDataAccessorList& rcOutputList  = rcAccessUnit    .getNalUnitList ();

  m_pcLowPassBaseReconstruction->setUnusedForRef(m_papcFrame[0]->getUnusedForRef());  // JVT-Q065 EIDR

  //===== initialize =====
  RNOK( xInitControlDataLowPass ( uiFrameIdInGOP, m_uiDecompositionStages-1,uiFrame ) );

  //NonRequired JVT-Q066 (06-04-08){{
  if(m_uiLayerId != 0 && m_uiNonRequiredWrite != 0)
  {
    if( pcSliceHeader->getBaseLayerId() == MSYS_UINT_MAX || m_uiLayerId - pcSliceHeader->getBaseLayerId() > 1 || pcSliceHeader->getBaseQualityLevel() != 3 )
    {
      rcAccessUnit.CreatNonRequiredSei();
    }
    xSetNonRequiredSEI(pcSliceHeader, rcAccessUnit.getNonRequiredSei());
    if(m_uiNonRequiredWrite == 2 && rcAccessUnit.getNonRequiredSei() != NULL)
    {
      xWriteNonRequiredSEI(rcOutputList, rcAccessUnit.getNonRequiredSei(), uiBits);
    }
  }
  //NonRequired JVT-Q066 (06-04-08)}}

  //===== base layer encoding =====
  // JVT-Q054 Red. Picture {
  //===== primary picture coding =====
  if ( pcSliceHeader->getPPS().getRedundantPicCntPresentFlag() )
  {
    pcSliceHeader->setRedundantPicCnt( 0 );  // set redundant_pic_cnt to 0 for primary coded picture
  }
  // JVT-Q054 Red. Picture }
  RNOK( pcBLRecFrame->copy      ( pcFrame ) );

  // JVT-R057 LA-RDO{
  if(m_bLARDOEnable)
  {
    pcFrame->initChannelDistortion();
    m_pcLowPassBaseReconstruction->initChannelDistortion();
    if( uiFrame == 0 && m_uiGOPNumber==0 )
    {
      pcFrame->zeroChannelDistortion();
      m_pcSliceEncoder->getMbEncoder()->setFrameEcEp(NULL);
    }
    else
    {
      m_pcSliceEncoder->getMbEncoder()->setFrameEcEp(m_pcLowPassBaseReconstruction);
    }
    pcBLRecFrame->setChannelDistortion(pcFrame);

  }
  // JVT-R057 LA-RDO}


  RNOK( xEncodeLowPassSignal    ( rcOutputList,
                                  rcControlData,
                                  pcBLRecFrame,
                                  pcResidual,
                                  pcPredSignal,
                                  uiBits,
                                  rcPicOutputDataList ) );
  // JVT-Q054 Red. Picture {
  if ( pcSliceHeader->getPPS().getRedundantPicCntPresentFlag() )
  {
    // in current version, slice repetition is supported for each primary coded slice
    UInt  uiRedundantPicNum = 1;  // number of redundant pictures for each primary coded picture
    UInt  uiRedundantPicCnt = 0;
    for ( uiRedundantPicCnt = 1; uiRedundantPicCnt <= uiRedundantPicNum; uiRedundantPicCnt++)
    {
      pcSliceHeader->setRedundantPicCnt( uiRedundantPicCnt );

      RNOK( pcBLRecFrame->copy      ( pcFrame ) );
      RNOK( xEncodeLowPassSignal    ( rcOutputList,
                                      rcControlData,
                                      pcBLRecFrame,
                                      pcResidual,
                                      pcPredSignal,
                                      uiBits,
                                      rcPicOutputDataList ) );
    }
    pcSliceHeader->setRedundantPicCnt( 0 );
  }
  // JVT-Q054 Red. Picture }

  // JVT-R057 LA-RDO{
  if(m_bLARDOEnable)
  {
    m_pcSliceEncoder->getMbEncoder()->setFrameEcEp(NULL);
    pcBLRecFrame->setChannelDistortion(NULL);
  }
  // JVT-R057 LA-RDO}

  m_uiNewlyCodedBits += uiBits;
  m_auiNumFramesCoded [ pcSliceHeader->getTemporalLevel() ] ++;
  m_auiCurrGOPBits    [ m_uiScalableLayerId ] += uiBits;

  //===== write FGS info to file =====
  if( m_uiFGSMode == 1 && m_pFGSFile && !m_bUseDiscardableUnit ) //FIX_FRAG_CAVLC
  {
    fprintf( m_pFGSFile, "%d", uiBits + m_uiNotYetConsideredBaseLayerBits );
    m_uiNotYetConsideredBaseLayerBits = 0;
  }


  //===== deblock and store picture for prediction of following low-pass frames =====
  ROF( pcSliceHeader->getNumRefIdxActive( LIST_0 ) == ( pcSliceHeader->isIntra() ? 0 : 1 ) );
  ROF( pcSliceHeader->getNumRefIdxActive( LIST_1 ) == 0 );

  //----- store for inter-layer prediction (non-deblocked version) -----
  RNOK( m_papcSubband[uiFrameIdInGOP] ->copy( pcBLRecFrame ) );

  // JVT-R057 LA-RDO{
  if(m_bLARDOEnable)
  {
    m_pcLowPassBaseReconstruction->copyChannelDistortion(pcFrame);
    m_papcSubband[uiFrameIdInGOP]->setChannelDistortion(pcFrame);
  }
  // JVT-R057 LA-RDO}
  //----- de-blocking -----
  m_pcLoopFilter->setHighpassFramePointer( pcResidual );

  RNOK( m_pcLoopFilter->process ( *pcSliceHeader,
                                  pcBLRecFrame,
                                  ( pcSliceHeader->isIntra() ? NULL : pcMbDataCtrl ),
                                  pcMbDataCtrl,
                                  m_uiFrameWidthInMb,
                                  &rcControlData.getPrdFrameList( LIST_0 ),
                                  &rcControlData.getPrdFrameList( LIST_1 ),
                true,
                                  rcControlData.getSpatialScalability()) );  // SSUN@SHARP

  m_uiNumLayers[0] = m_uiNumLayers[1];

  for( UInt uiLayerIdx = 0; uiLayerIdx < m_uiNumLayers[0]; uiLayerIdx ++ )
  {
    RNOK( m_aapcFGSRecon[0][uiLayerIdx]->copy( m_aapcFGSRecon[1][uiLayerIdx] ) );
  }

  RNOK( m_aapcFGSRecon[0][0]->copy( m_pcLowPassBaseReconstruction ) );

  //----- store for prediction of following low-pass pictures -----
  ROF( pcSliceHeader->getNalRefIdc() );
  RNOK( m_pcLowPassBaseReconstruction ->copy( pcBLRecFrame ) );
  m_pcLowPassBaseReconstruction->setFrameNum(pcSliceHeader->getFrameNum());  //JVT-S036 lsj
  // at least the same as the base layer
  RNOK( rcControlData.saveMbDataQpAndCbp() );

  //--- closed-loop coding of base quality layer ---
  if( m_papcBQFrame )
  {
    RNOK( m_papcBQFrame[uiFrameIdInGOP]->copy( pcBLRecFrame ) ); // save base quality layer reconstruction
    //Bug_Fix JVT-R057 0806{
    if(m_bLARDOEnable)
    {
      m_papcBQFrame[uiFrameIdInGOP]->setChannelDistortion(pcFrame);
    }
  //Bug_Fix JVT-R057 0806}
  }

  //===== FGS enhancement layers =====
  if( m_dNumFGSLayers == 0.0 )
  {
    RNOK( pcFrame->copy   ( pcBLRecFrame ) );

    if( m_papcCLRecFrame )
    {
      RNOK( m_papcCLRecFrame[uiFrameIdInGOP]->copy( pcFrame ) );
      // JVT-R057 LA-RDO{
      if(m_bLARDOEnable&&m_papcCLRecFrame)
        m_papcCLRecFrame[uiFrameIdInGOP]->setChannelDistortion(pcFrame);
      // JVT-R057 LA-RDO}
    }
  }
  else
  {
    //----- encode FGS enhancement -----
    if( m_uiFGSMode == 2 )
    {
      uiBits += m_uiNotYetConsideredBaseLayerBits;
      m_uiNotYetConsideredBaseLayerBits = 0;
    }

    RefFrameList cRefFrameList0;
    RefFrameList cRefFrameListDummy;
    IntFrame*    pcLowPassRefFrameEnh = m_aapcFGSRecon[0][m_uiFgsEncStructureFlag == 0 ? m_uiNumLayers[0] - 1 : 1];
    RNOK( xFillAndUpsampleFrame( pcLowPassRefFrameEnh ) );
    RNOK( cRefFrameList0.add( pcLowPassRefFrameEnh ) );
    RNOK( xEncodeFGSLayer ( rcOutputList,
                            rcControlData,
                            pcFrame,
                            pcResidual,
                            pcPredSignal,
                            pcBLRecFrame,
                            m_papcSubband[uiFrameIdInGOP],
                            m_papcCLRecFrame ? m_papcCLRecFrame[uiFrameIdInGOP] : 0,
                            uiFrameIdInGOP,
                            // FGS_MOTION {
                            pcFrame,
                            NULL,
                            cRefFrameList0,
                            cRefFrameListDummy,
                            // } FGS_MOTION
                            uiBits,
                            rcPicOutputDataList ) );

    m_uiNewlyCodedBits += uiBits;

    //----- de-blocking -----
    m_pcLoopFilter->setHighpassFramePointer( pcResidual );
    RNOK( m_pcLoopFilter->process ( *pcSliceHeader,
                                    pcFrame,
                                    ( pcSliceHeader->isIntra() ? NULL : pcMbDataCtrl ),
                                    pcMbDataCtrl,
                                    m_uiFrameWidthInMb,
                                    &rcControlData.getPrdFrameList( LIST_0 ),
                                    &rcControlData.getPrdFrameList( LIST_1 ),
                  true,
                                    rcControlData.getSpatialScalability()) );  // SSUN@SHARP

    RNOK( m_aapcFGSRecon[1][m_uiNumLayers[1] - 1]->copy( pcFrame ) );

    if( m_papcCLRecFrame )
    {
      RNOK( rcControlData.activateMbDataCtrlForQpAndCbp( true ) );
      m_pcLoopFilter->setHighpassFramePointer( pcResidual );
      RNOK( m_pcLoopFilter->process ( *pcSliceHeader,
                                      m_papcCLRecFrame[uiFrameIdInGOP],
                                      ( pcSliceHeader->isIntra() ? NULL : pcMbDataCtrl ),
                                      pcMbDataCtrl,
                                      m_uiFrameWidthInMb,
                                      &rcControlData.getPrdFrameList( LIST_0 ),
                                      &rcControlData.getPrdFrameList( LIST_1 ),
                  true,
                                      rcControlData.getSpatialScalability()) );  // SSUN@SHARP
    }
  }


  if( m_uiFGSMode == 1 && m_pFGSFile && !m_bUseDiscardableUnit ) //FIX_FRAG_CAVLC)
  {
    fprintf( m_pFGSFile, "\n" );
  }

  return Err::m_nOK;
}





ErrVal
MCTFEncoder::xEncodeNonKeyPicture( UInt                 uiBaseLevel,
                                   UInt                 uiFrame,
                                   AccessUnitList&      rcAccessUnitList,
                                   PicOutputDataList&   rcPicOutputDataList )
{
  UInt                    uiFrameIdInGOP  = uiFrame << uiBaseLevel;
  UInt                    uiBits          = 0;
  UInt                    uiBitsRes       = 0;
  IntFrame*               pcFrame         = m_papcFrame     [uiFrameIdInGOP];
  IntFrame*               pcOrgFrame      = m_papcOrgFrame [uiFrameIdInGOP];
  IntFrame*               pcBQFrame       = ( m_papcBQFrame ? m_papcBQFrame[uiFrameIdInGOP] : 0 );
  IntFrame*               pcResidual      = m_papcResidual  [uiFrameIdInGOP];
  IntFrame*               pcPredSignal    = m_apcFrameTemp  [0];
  IntFrame*               pcBLRecFrame    = m_apcFrameTemp  [1];
  IntFrame*               pcSRFrame       = m_papcSmoothedFrame[uiFrameIdInGOP]; // JVT-R091
  ControlData&            rcControlData   = m_pacControlData[uiFrameIdInGOP];
  SliceHeader*            pcSliceHeader   = rcControlData.getSliceHeader();
  IntFrame*               pcRedBQFrame    = m_apcFrameTemp  [3];  // JVT-Q054 Red. Picture
  IntFrame*               pcRedSRFrame    = m_apcFrameTemp  [4];  // JVT-Q054 Red. Picture
  AccessUnit&             rcAccessUnit    = rcAccessUnitList.getAccessUnit  ( pcSliceHeader->getPoc() );
  ExtBinDataAccessorList& rcOutputList    = rcAccessUnit    .getNalUnitList ();

  RNOK( xInitControlDataHighPass( uiFrameIdInGOP,uiBaseLevel,uiFrame ) );

  //NonRequired JVT-Q066 (06-04-08){{
  if(m_uiLayerId != 0 && m_uiNonRequiredWrite != 0)
  {
    if( pcSliceHeader->getBaseLayerId() == MSYS_UINT_MAX || m_uiLayerId - pcSliceHeader->getBaseLayerId() > 1 || pcSliceHeader->getBaseQualityLevel() != 3 )
    {
      rcAccessUnit.CreatNonRequiredSei();
    }
    xSetNonRequiredSEI(pcSliceHeader, rcAccessUnit.getNonRequiredSei());
    if(m_uiNonRequiredWrite == 2 && rcAccessUnit.getNonRequiredSei() != NULL)
    {
      xWriteNonRequiredSEI(rcOutputList, rcAccessUnit.getNonRequiredSei(), uiBits);
    }
  }
  //NonRequired JVT-Q066 (06-04-08)}}

  //===== base layer encoding =====
  // JVT-Q054 Red. Picture {
  if ( pcSliceHeader->getPPS().getRedundantPicCntPresentFlag() )
  {
    pcSliceHeader->setRedundantPicCnt( 0 );  // set redundant_pic_cnt to 0 for primary coded picture
  }
  // JVT-Q054 Red. Picture }
  
  //--- closed-loop coding of base quality layer ---
  if( pcBQFrame )
  {
    //JVT-R057 LA-RDO{
    //Bug_Fix JVT-R057 0806{
    if(m_bLARDOEnable)
    {
      pcBQFrame->setChannelDistortion(pcFrame);
      m_pcSliceEncoder->getMbEncoder()->setFrameEcEp(m_papcFrame[(uiFrame-1)<<uiBaseLevel]);
    }
    //Bug_Fix JVT-R057 0806}
    //JVT-R057 LA-RDO}
    RNOK( pcRedBQFrame->copy      ( pcBQFrame            ) ); // JVT-Q054 Red. Picture
    RNOK( pcRedSRFrame->copy      ( pcSRFrame            ) ); // JVT-Q054 Red. Picture
    RNOK( xEncodeHighPassSignal   ( rcOutputList,
                                    rcControlData,
                                    pcOrgFrame, 
                                    pcBQFrame,
                                    pcResidual,
                                    pcPredSignal,
  																	pcSRFrame, 
                                    uiBits, uiBitsRes, rcPicOutputDataList ) );
    RNOK( rcControlData.storeBQLayerQpAndCbp() );
  }
  else
  {
    RNOK( pcBLRecFrame->copy      ( pcFrame ) );
    RNOK( pcRedSRFrame->copy      ( pcSRFrame               ) ); // JVT-Q054 Red. Picture
    //JVT-R057 LA-RDO{
    //Bug_Fix JVT-R057 0806{
    if(m_bLARDOEnable)
    {
      pcBLRecFrame->setChannelDistortion(pcFrame);
      m_pcSliceEncoder->getMbEncoder()->setFrameEcEp(m_papcFrame[(uiFrame-1)<<uiBaseLevel]);
    }
    //Bug_Fix JVT-R057 0806}
    //JVT-R057 LA-RDO}
	  RNOK( xEncodeHighPassSignal   ( rcOutputList,
                                    rcControlData,
                                    pcOrgFrame, 
                                    pcBLRecFrame,
                                    pcResidual,
                                    pcPredSignal,
		  															pcSRFrame, 
                                    uiBits, uiBitsRes, rcPicOutputDataList ) );
  }

  //JVT-Q054 Red. Picture {
  if ( pcSliceHeader->getPPS().getRedundantPicCntPresentFlag() )
  {
    // currently only slice repetition is supported for each primary coded slice
    UInt  uiRedundantPicNum = 1;  // number of redundant picture for each primary coded picture
    UInt  uiRedundantPicCnt = 0;
    for ( uiRedundantPicCnt = 1; uiRedundantPicCnt <= uiRedundantPicNum; uiRedundantPicCnt++)
    {
      pcSliceHeader->setRedundantPicCnt( uiRedundantPicCnt );
      if( pcBQFrame )
      {
        //JVT-R057 LA-RDO{
        if(m_bLARDOEnable)
        {
          pcBQFrame->setChannelDistortion(pcFrame);
          m_pcSliceEncoder->getMbEncoder()->setFrameEcEp( m_papcFrame[(uiFrame-1)<<uiBaseLevel] );
        }
        //JVT-R057 LA-RDO}
        RNOK( pcSRFrame->copy          ( pcRedSRFrame         ) );  // JVT-Q054
        RNOK( pcBQFrame->copy         ( pcRedBQFrame         ) ); // JVT-Q054
        RNOK( xEncodeHighPassSignal   ( rcOutputList,
                                        rcControlData,
                                        pcOrgFrame, 
                                        pcBQFrame,
                                        pcResidual,
                                        pcPredSignal,
                                        pcSRFrame, 
                                        uiBits, uiBitsRes, rcPicOutputDataList ) );
        RNOK( rcControlData.storeBQLayerQpAndCbp() );
      }
      else
      {
        RNOK( pcBLRecFrame->copy      ( pcFrame ) );
        RNOK( pcSRFrame->copy          ( pcRedSRFrame            ) );  // JVT-Q054
        //JVT-R057 LA-RDO{
        if(m_bLARDOEnable)
        {
          pcBLRecFrame->setChannelDistortion(pcFrame);
          m_pcSliceEncoder->getMbEncoder()->setFrameEcEp( m_papcFrame[(uiFrame-1)<<uiBaseLevel] );
        }
        //JVT-R057 LA-RDO}
        RNOK( xEncodeHighPassSignal   ( rcOutputList,
                                        rcControlData,
                                        pcOrgFrame, 
                                        pcBLRecFrame,
                                        pcResidual,
                                        pcPredSignal,
                                        pcSRFrame, 
                                        uiBits, uiBitsRes, rcPicOutputDataList ) );
      }
    }
    pcSliceHeader->setRedundantPicCnt( 0 );
  }
  // JVT-Q054 Red. Picture }

  m_uiNewlyCodedBits += uiBits;
  m_auiCurrGOPBits    [ m_uiScalableLayerId ] += uiBits;
  m_auiNumFramesCoded [ pcSliceHeader->getTemporalLevel() ] ++;

  //===== save FGS info =====
  if( m_uiFGSMode == 1 && m_pFGSFile && !m_bUseDiscardableUnit ) //FIX_FRAG_CAVLC)
  {
    fprintf( m_pFGSFile, "%d", uiBits + m_uiNotYetConsideredBaseLayerBits );
    m_uiNotYetConsideredBaseLayerBits = 0;
  }

  // at least the same as the base layer
  RNOK( rcControlData.saveMbDataQpAndCbp() );


  //===== FGS enhancement ====
  if( m_dNumFGSLayers == 0.0 )
  {
    //--- closed-loop coding of base quality layer ---
    if( pcBQFrame )
    {
      RNOK( pcFrame->copy         ( pcBQFrame ) );
    }
    else
    {
      RNOK( pcFrame->copy         ( pcBLRecFrame ) );
    }

    RNOK( m_papcSubband[uiFrameIdInGOP]->copy( pcFrame ) );

    // JVT-R057 LA-RDO{
    if(m_bLARDOEnable)
    {
      m_papcSubband[uiFrameIdInGOP]->setChannelDistortion(pcFrame);
    }
    // JVT-R057 LA-RDO}

    if( m_papcCLRecFrame )
    {
      RNOK( m_papcCLRecFrame[uiFrameIdInGOP]->copy( pcFrame ) );
      // JVT-R057 LA-RDO{
      if(m_bLARDOEnable)
      {
        m_papcCLRecFrame[uiFrameIdInGOP]->setChannelDistortion(pcFrame);
      }
      // JVT-R057 LA-RDO}
    }
  }
  else
  {
    if( m_uiFGSMode == 2 )
    {
      uiBits += m_uiNotYetConsideredBaseLayerBits;
      m_uiNotYetConsideredBaseLayerBits = 0;
    }

    IntFrame*    pcHighPassPredSignal = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                      *m_pcYuvFullPelBufferCtrl );
    ROF ( pcHighPassPredSignal );
    RNOK( pcHighPassPredSignal->init() );

    RefFrameList cRefFrameList0, cRefFrameList1;
    if( m_papcCLRecFrame )
    {
      RNOK( xGetCLRecPredictionLists( cRefFrameList0, cRefFrameList1, uiBaseLevel, uiFrame, true ) );
    }
    else
    {
      RNOK( xGetPredictionLists( cRefFrameList0, cRefFrameList1, uiBaseLevel, uiFrame, true ) );
    }

    RNOK( xMotionCompensation( pcHighPassPredSignal, &cRefFrameList0, &cRefFrameList1,
                               rcControlData.getMbDataCtrl(), *pcSliceHeader, true ) );
    RNOK( pcHighPassPredSignal->add( pcPredSignal ) ); // add  intra-prediction signal

    RNOK( xEncodeFGSLayer ( rcOutputList,
                            rcControlData,
                            pcFrame,
                            pcResidual,
                            pcPredSignal,
                            pcBLRecFrame,
                            m_papcSubband[uiFrameIdInGOP],
                            m_papcCLRecFrame ? m_papcCLRecFrame[uiFrameIdInGOP] : 0,
                            uiFrameIdInGOP,
                            // FGS_MOTION {
                            pcOrgFrame,
                            pcHighPassPredSignal,
                            cRefFrameList0,
                            cRefFrameList1,
                            // } FGS_MOTION
                            uiBits, rcPicOutputDataList ) );
    RNOK( pcHighPassPredSignal->uninit() );
    delete pcHighPassPredSignal; pcHighPassPredSignal = 0;

    m_uiNewlyCodedBits += uiBits;
  }


  if( m_uiFGSMode == 1 && m_pFGSFile && !m_bUseDiscardableUnit ) //FIX_FRAG_CAVLC)
  {
    fprintf( m_pFGSFile, "\n" );
  }

  return Err::m_nOK;
}



ErrVal
MCTFEncoder::xOutputPicData( PicOutputDataList& rcPicOutputDataList )
{
  while( rcPicOutputDataList.size() )
  {
    PicOutputData cPicOutputData = rcPicOutputDataList.popFront();
    if( cPicOutputData.YPSNR )
    {
      printf("%2s %5d: %2s   T%1d L%1d Q%1d   QP%3d   Y%8.4lf  U%8.4lf  V%8.4lf  %8d bit\n",
        cPicOutputData.FirstPicInAU ? "AU" : "  ",
        cPicOutputData.Poc,
        cPicOutputData.FrameType,
        cPicOutputData.TemporalId,
        cPicOutputData.DependencyId,
        cPicOutputData.QualityId,
        cPicOutputData.Qp,
        cPicOutputData.YPSNR,
        cPicOutputData.UPSNR,
        cPicOutputData.VPSNR,
        cPicOutputData.Bits );
    }
    else
    {
      printf("%2s %5d: %2s   T%1d L%1d Q%1d   QP%3d                                    %8d bit\n",
        cPicOutputData.FirstPicInAU ? "AU" : "  ",
        cPicOutputData.Poc,
        cPicOutputData.FrameType,
        cPicOutputData.TemporalId,
        cPicOutputData.DependencyId,
        cPicOutputData.QualityId,
        cPicOutputData.Qp,
        cPicOutputData.Bits );
    }
  }
  return Err::m_nOK;
}



ErrVal
MCTFEncoder::xStoreReconstruction( PicBufferList&  rcPicBufferOutputList )
{
  PicBufferList::iterator cOutputIter = rcPicBufferOutputList.begin();
  for( UInt uiIndex = (m_bFirstGOPCoded?1:0); uiIndex <= (m_uiGOPSize >> m_uiNotCodedMCTFStages); uiIndex++, cOutputIter++ )
  {
    PicBuffer*  pcPicBuffer = *cOutputIter;
    if( m_papcCLRecFrame )
    {
      RNOK( m_papcCLRecFrame[uiIndex<<m_uiNotCodedMCTFStages]->store( pcPicBuffer ) );
    }
    else
    {
      RNOK( m_papcFrame[uiIndex<<m_uiNotCodedMCTFStages]->store( pcPicBuffer ) );
    }
  }
  return Err::m_nOK;
}




ErrVal
MCTFEncoder::initGOP( AccessUnitList& rcAccessUnitList,
                      PicBufferList&  rcPicBufferInputList )
{
  ROT ( m_bGOPInitialized );
  RNOK( xInitGOP              ( rcPicBufferInputList ) );
  RNOK( xSetScalingFactors    () );
  if  ( m_bFirstGOPCoded )
  {
    //==== copy picture zero =====
    PicOutputDataList cPicOutputDataList;
    Bool              bPicCoded = false;
    RNOK( xEncodeKeyPicture ( bPicCoded, 0, rcAccessUnitList, cPicOutputDataList ) );
    ROT ( bPicCoded );
  }
  m_bGOPInitialized = true;
  return Err::m_nOK;
}



ErrVal
MCTFEncoder::process( UInt            uiAUIndex,
                      AccessUnitList& rcAccessUnitList,
                      PicBufferList&  rcPicBufferInputList,
                      PicBufferList&  rcPicBufferOutputList,
                      PicBufferList&  rcPicBufferUnusedList,
                      Double          m_aaauidSeqBits[MAX_LAYERS][MAX_TEMP_LEVELS][MAX_QUALITY_LEVELS] )
{
  ROTRS ( rcPicBufferInputList.empty(),              Err::m_nOK );
  ROTRS ( rcPicBufferInputList.size () <= uiAUIndex, Err::m_nOK );
  ROF   ( m_bGOPInitialized );

  //===== init some parameters =====
  UInt  uiCurrIdx = 0;
  Bool  bPicCoded = false; 
  g_nLayer        = m_uiLayerId;
  ETRACE_LAYER(     m_uiLayerId );
  if( m_bLARDOEnable )
  {
    m_pcSliceEncoder->getMbEncoder()->setLARDOEnable( m_bLARDOEnable );
    m_pcSliceEncoder->getMbEncoder()->setLayerID    ( m_uiLayerId    );
  }

  RNOK( xInitBitCounts() );

  //===== encode key pictures =====
  for( UInt uiKeyFrame = ( m_bFirstGOPCoded ? 1 : 0 ); uiKeyFrame <= ( m_uiGOPSize >> m_uiDecompositionStages ) && ! bPicCoded; uiKeyFrame++, uiCurrIdx++ )
  {
    if( uiAUIndex == uiCurrIdx )
    {
      PicOutputDataList cPicOutputDataList;
      m_uiScalableLayerId         = m_uiMinScalableLayer;
      RNOK( xEncodeKeyPicture     ( bPicCoded,               uiKeyFrame, rcAccessUnitList,      cPicOutputDataList ) );
      ROF ( bPicCoded );
      RNOK( xCalculateAndAddPSNR  ( m_uiDecompositionStages, uiKeyFrame, rcPicBufferInputList,  cPicOutputDataList ) );
      RNOK( xOutputPicData        (                                                             cPicOutputDataList ) );
      RNOK( xClearBufferExtensions() );
      if  ( uiKeyFrame == 1 )
      {
        RNOK( m_pcAnchorFrameReconstructed->copyAll( m_papcFrame[ m_uiGOPSize ] ) );
      }
    }
  }


  //===== encode non-key pictures =====
  for( Int iLevel = (Int)m_uiDecompositionStages - 1; iLevel >= (Int)m_uiNotCodedMCTFStages && ! bPicCoded; iLevel-- )
  {
    m_uiScalableLayerId = m_uiMinScalableLayer + ( m_uiDecompositionStages - (UInt)iLevel ) * ( 1 + (UInt)m_dNumFGSLayers );

    for( UInt uiFrame = 1; uiFrame <= ( m_uiGOPSize >> iLevel ) && ! bPicCoded; uiFrame += 2, uiCurrIdx++ )
    {
      if( uiAUIndex == uiCurrIdx )
      {
        PicOutputDataList cPicOutputDataList;
        RNOK( xMotionEstimationFrame( iLevel, uiFrame ) );
        RNOK( xDecompositionFrame   ( iLevel, uiFrame ) );
        RNOK( xEncodeNonKeyPicture  ( iLevel, uiFrame, rcAccessUnitList,     cPicOutputDataList ) );
        RNOK( xCompositionFrame     ( iLevel, uiFrame, rcPicBufferInputList ) );
        RNOK( xCalculateAndAddPSNR  ( iLevel, uiFrame, rcPicBufferInputList, cPicOutputDataList ) );
        RNOK( xOutputPicData        (                                        cPicOutputDataList ) );
        RNOK( xClearBufferExtensions() );
        bPicCoded = true;
      }
    }
  }


  //===== finish GOP =====
  if( uiAUIndex == rcPicBufferInputList.size() - 1 )
  {
    m_bGOPInitialized   = false;
    m_uiScalableLayerId = m_uiMinScalableLayer + ( m_uiDecompositionStages - m_uiNotCodedMCTFStages + 1 ) * ( 1 + (UInt)m_dNumFGSLayers );

    RNOK( xStoreReconstruction( rcPicBufferOutputList ) );
    RNOK( xFinishGOP          ( rcPicBufferInputList,
                                rcPicBufferOutputList,
                                rcPicBufferUnusedList,
                                m_aaauidSeqBits ) );
  }

  return Err::m_nOK;
}



ErrVal
MCTFEncoder::xFinishGOP( PicBufferList& rcPicBufferInputList,
                         PicBufferList& rcPicBufferOutputList,
                         PicBufferList& rcPicBufferUnusedList,
                         Double         m_aaauidSeqBits[MAX_LAYERS][MAX_TEMP_LEVELS][MAX_QUALITY_LEVELS] )
{
  UInt  uiLowPassSize = m_uiGOPSize >> m_uiNotCodedMCTFStages;

  while( rcPicBufferOutputList.size() > uiLowPassSize + ( m_bFirstGOPCoded ? 0 : 1 ) )
  {
    PicBuffer*  pcPicBuffer = rcPicBufferOutputList.popBack();
    rcPicBufferUnusedList.push_back( pcPicBuffer );
  }

  //--- closed-loop coding of base quality level ----
  // move last LP frame to the beginning -> for next GOP
  if( m_papcBQFrame && m_uiGOPSize == (1<<m_uiDecompositionStages) ) // full GOP
  {
    IntFrame*  pcTempFrame      = m_papcBQFrame[0];
    m_papcBQFrame[0]            = m_papcBQFrame[m_uiGOPSize];
    m_papcBQFrame[m_uiGOPSize]  = pcTempFrame;
  }

  //--- highest FGS layer for closed-loop coding ----
  // move last LP frame to the beginning -> for next GOP
  if( m_papcCLRecFrame && m_uiGOPSize == (1<<m_uiDecompositionStages) ) // full GOP
  {
    IntFrame*  pcTempFrame         = m_papcCLRecFrame[0];
    m_papcCLRecFrame[0]            = m_papcCLRecFrame[m_uiGOPSize];
    m_papcCLRecFrame[m_uiGOPSize]  = pcTempFrame;
  }

  //===== update bit counts etc. =====
  UInt uiLevel;
  for( uiLevel = 0; uiLevel < MAX_SCALABLE_LAYERS; uiLevel++ )
  {
    m_adSeqBits     [uiLevel] += (Double)m_auiCurrGOPBits    [uiLevel];
  }
  UInt uiLayerOffset = (UInt)m_uiScalableLayerId - ( m_uiDecompositionStages-m_uiNotCodedMCTFStages+1 )*( (UInt)m_dNumFGSLayers+1 );
  for( uiLevel = 0; uiLevel <= m_uiDecompositionStages-m_uiNotCodedMCTFStages; uiLevel++ )
    for( UInt uiFGS = 0; uiFGS <= (UInt)m_dNumFGSLayers; uiFGS++ )
      m_aaauidSeqBits [m_uiLayerId][uiLevel][uiFGS] = m_adSeqBits[uiLayerOffset+uiLevel * ((UInt)m_dNumFGSLayers+1) + uiFGS];

  //===== update parameters =====
  m_uiParameterSetBits  = 0;
  m_bFirstGOPCoded      = true;
  m_uiGOPNumber        ++;

  return Err::m_nOK;
}



ErrVal
MCTFEncoder::xCalculateAndAddPSNR( UInt               uiStage,
                                   UInt               uiFrame,
                                   PicBufferList&     rcPicBufferInputList,
                                   PicOutputDataList& rcPicOutputDataList )
{
  //===== initialize buffer control =====
  RNOK( m_pcYuvFullPelBufferCtrl->initMb() );


  const YuvBufferCtrl::YuvBufferParameter&  cBufferParam    = m_pcYuvFullPelBufferCtrl->getBufferParameter();
  PicBufferList::iterator                   cIter           = rcPicBufferInputList.begin();
  UInt                                      uiFrameIdInGOP  = uiFrame << uiStage;
  PicBuffer*                                pcPicBuffer     = 0;
  IntFrame*                                 pcFrame         = ( m_papcCLRecFrame ? m_papcCLRecFrame : m_papcFrame )[uiFrameIdInGOP];
  Int                                       iPoc            = m_pacControlData[uiFrameIdInGOP].getSliceHeader()->getPoc();
  Double                                    dYPSNR          = 0.0;
  Double                                    dUPSNR          = 0.0;
  Double                                    dVPSNR          = 0.0;


  //===== get correct picture buffer =====
  for( UInt uiIndex = uiFrameIdInGOP - ( m_bFirstGOPCoded ? 1 : 0 ); uiIndex; uiIndex--, cIter++ )
  {
    ROT( cIter == rcPicBufferInputList.end() );    
  }
  pcPicBuffer = *cIter;
  ROF  ( pcPicBuffer );


  //===== calculate PSNR =====
  {
    Pel*  pPelOrig  = pcPicBuffer->getBuffer() + cBufferParam.getMbLum();
    XPel* pPelRec   = pcFrame->getFullPelYuvBuffer()->getMbLumAddr();
    Int   iStride   = cBufferParam.getStride();
    Int   iWidth    = cBufferParam.getWidth ();
    Int   iHeight   = cBufferParam.getHeight();
    UInt  uiSSDY    = 0;
    UInt  uiSSDU    = 0;
    UInt  uiSSDV    = 0;
    Int   x, y;

    for( y = 0; y < iHeight; y++ )
    {
      for( x = 0; x < iWidth; x++ )
      {
        Int iDiff = (Int)pPelOrig[x] - (Int)pPelRec[x];
        uiSSDY   += iDiff * iDiff;
      }
      pPelOrig += iStride;
      pPelRec  += iStride;
    }

    iHeight >>= 1;
    iWidth  >>= 1;
    iStride >>= 1;
    pPelOrig  = pcPicBuffer->getBuffer() + cBufferParam.getMbCb();
    pPelRec   = pcFrame->getFullPelYuvBuffer()->getMbCbAddr();

    for( y = 0; y < iHeight; y++ )
    {
      for( x = 0; x < iWidth; x++ )
      {
        Int iDiff = (Int)pPelOrig[x] - (Int)pPelRec[x];
        uiSSDU   += iDiff * iDiff;
      }
      pPelOrig += iStride;
      pPelRec  += iStride;
    }

    pPelOrig  = pcPicBuffer->getBuffer() + cBufferParam.getMbCr();
    pPelRec   = pcFrame->getFullPelYuvBuffer()->getMbCrAddr();

    for( y = 0; y < iHeight; y++ )
    {
      for( x = 0; x < iWidth; x++ )
      {
        Int iDiff = (Int)pPelOrig[x] - (Int)pPelRec[x];
        uiSSDV   += iDiff * iDiff;
      }
      pPelOrig += iStride;
      pPelRec  += iStride;
    }

    Double fRefValueY = 255.0 * 255.0 * 16.0 * 16.0 * (Double)m_uiMbNumber;
    Double fRefValueC = fRefValueY / 4.0;
    dYPSNR            = ( uiSSDY ? 10.0 * log10( fRefValueY / (Double)uiSSDY ) : 99.99 );
    dUPSNR            = ( uiSSDU ? 10.0 * log10( fRefValueC / (Double)uiSSDU ) : 99.99 );
    dVPSNR            = ( uiSSDV ? 10.0 * log10( fRefValueC / (Double)uiSSDV ) : 99.99 );
  }

  //===== add PSNR =====
  for( UInt uiLevel = m_uiDecompositionStages - uiStage; uiLevel <= MAX_DSTAGES; uiLevel++ )
  {
    m_adPSNRSumY[ uiLevel ] += dYPSNR;
    m_adPSNRSumU[ uiLevel ] += dUPSNR;
    m_adPSNRSumV[ uiLevel ] += dVPSNR;
  }

  //===== output PSNR =====
  PicOutputData& rcPicOutputData = rcPicOutputDataList.back();
  ROF( rcPicOutputData.Poc == iPoc );
  rcPicOutputData.YPSNR = dYPSNR;
  rcPicOutputData.UPSNR = dUPSNR;
  rcPicOutputData.VPSNR = dVPSNR;

  return Err::m_nOK;
}


ErrVal
MCTFEncoder::finish( UInt&    ruiNumCodedFrames,
                     Double&  rdOutputRate,
                     Double*  rdOutputFramerate,
                     Double*  rdOutputBitrate,
                     Double   m_aaauidSeqBits[MAX_LAYERS][MAX_TEMP_LEVELS][MAX_QUALITY_LEVELS] )
{
  ROFRS( m_auiNumFramesCoded[0], Err::m_nOK );

  UInt  uiStage;
  UInt  uiMaxStage        = m_uiDecompositionStages - m_uiNotCodedMCTFStages;
  // Bug fix: yiliang.bao@nokia.com
  // uiMaxStage is unsigned, it has a problem when uiMaxStage == 0,
  // uiMaxStage - 1 will result in a large number
  UInt  uiMinStage        = 0; //bugfix replace

  Char  acResolution[10];
//bugfix delete

  sprintf( acResolution, "%dx%d", 16*m_uiFrameWidthInMb, 16*m_uiFrameHeightInMb );

  //===== set final sum of bits and average PSNR's =====
  for( uiStage = 0; uiStage <= MAX_DSTAGES; uiStage++ )
  {
    if( uiStage  )
    {
      m_auiNumFramesCoded   [uiStage] += m_auiNumFramesCoded        [uiStage-1];
    }
    if( m_auiNumFramesCoded [uiStage] )
    {
      m_adPSNRSumY          [uiStage] /= (Double)m_auiNumFramesCoded[uiStage];
      m_adPSNRSumU          [uiStage] /= (Double)m_auiNumFramesCoded[uiStage];
      m_adPSNRSumV          [uiStage] /= (Double)m_auiNumFramesCoded[uiStage];
    }
  }


  UInt uiLayerOffset = (UInt)m_uiScalableLayerId - ( uiMaxStage+1 )*( (UInt)m_dNumFGSLayers+1 );
  for( uiStage = 1; uiStage < MAX_TEMP_LEVELS * MAX_QUALITY_LEVELS; uiStage++ )
  {
    m_adSeqBits  [uiStage]  += m_adSeqBits  [uiStage-1];
  }
  static Double aaadCurrBits[MAX_LAYERS][MAX_TEMP_LEVELS][MAX_QUALITY_LEVELS];
  for( UInt uiLevel = 0; uiLevel < MAX_TEMP_LEVELS; uiLevel++ )
  {
    Double dBits = 0;
    if( uiLevel == 0 ) //TL = 0
    {
      for( UInt uiFGS = 0; uiFGS < MAX_QUALITY_LEVELS; uiFGS++ )
      {
        if( m_uiBaseLayerId < MAX_LAYERS ) //not base layer
        {
// BUG_FIX liuhui{
          if( uiFGS ) // D > 0, T = 0, Q != 0
            dBits = aaadCurrBits[m_uiLayerId][uiLevel][uiFGS-1];
          else // D > 0, T = 0, Q = 0
          {
            dBits = aaadCurrBits[m_uiBaseLayerId][uiLevel][m_uiBaseQualityLevel];
//bugfix delete
          }
// BUG_FIX liuhui}
        }
//bugfix delete
        dBits += m_aaauidSeqBits[m_uiLayerId][uiLevel][uiFGS];
        aaadCurrBits[m_uiLayerId][uiLevel][uiFGS] = dBits;
      }
    }
    else //TL != 0
    {
      for( UInt uiFGS = 0; uiFGS < MAX_QUALITY_LEVELS; uiFGS++ )
      {
        dBits = 0;
        if( m_uiBaseLayerId < MAX_LAYERS ) //not base layer
        {
// BUG_FIX liuhui{
          dBits = aaadCurrBits[m_uiBaseLayerId][uiLevel][m_uiBaseQualityLevel];
//bugfix delete
// BUG_FIX liuhui}
        }
        for( UInt uiTIndex = 0; uiTIndex <= uiLevel; uiTIndex++ )
        {
          for( UInt uiFIndex = 0; uiFIndex <= uiFGS; uiFIndex++ )
          {
            dBits += m_aaauidSeqBits[m_uiLayerId][uiTIndex][uiFIndex];
          }
        }
        aaadCurrBits[m_uiLayerId][uiLevel][uiFGS] = dBits;
      }
    }
  }

// BUG_FIX liuhui{
  if( m_uiLayerId == 0 )
  {
    printf( " \n\n\nSUMMARY:\n" );
    printf( "                      " "  SNR Lv" "  bitrate " "   Min-bitr" "   Y-PSNR" "   U-PSNR" "   V-PSNR\n" );
    printf( "                      " " -------" " ---------" " ----------" " --------" " --------" " --------\n" );
  }
// BUG_FIX liuhui}
  for( uiStage = uiMinStage; uiStage <= uiMaxStage; uiStage++ )
  {
    Double  dFps    = m_fOutputFrameRate / (Double)( 1 << ( uiMaxStage - uiStage ) );
    Double  dScale  = dFps / 1000 / (Double)m_auiNumFramesCoded[uiStage];
    for( UInt uiFGS = 0; uiFGS <= m_dNumFGSLayers; uiFGS++ )
    {
      UInt uiIndex = uiLayerOffset + uiStage*( (UInt)m_dNumFGSLayers+1 ) + uiFGS ;
      Double dBitrate = aaadCurrBits[m_uiLayerId][uiStage][uiFGS] * dScale;
      rdOutputFramerate[ uiIndex ] = dFps;
      rdOutputBitrate[ uiIndex ] = dBitrate;

// BUG_FIX liuhui{
      static Double adMinBitrate[MAX_LAYERS][MAX_TEMP_LEVELS];
      if( uiFGS == 0 ) // not FGS layer
      {
        if( m_uiLayerId == 0 )
        {
          adMinBitrate[m_uiLayerId][uiStage] = aaadCurrBits[m_uiLayerId][uiStage][0] * dScale;
        }
        else //D!=0
        {
          if( adMinBitrate[m_uiBaseLayerId][uiStage] ) //base layer with the same TL exists
            {
            adMinBitrate[m_uiLayerId][uiStage] = adMinBitrate[m_uiBaseLayerId][uiStage];
            for(UInt uiTIndex = 0; uiTIndex <= uiStage; uiTIndex++)
            {
            adMinBitrate[m_uiLayerId][uiStage] += m_aaauidSeqBits[m_uiLayerId][uiTIndex][0] * dScale;
            }
            }
          else // base layer non-exists
            {
            if(adMinBitrate[m_uiLayerId][0] == 0.0) // first time for layer, uiStage = 0
            {
            if( m_uiBaseLayerId == 0 && adMinBitrate[m_uiBaseLayerId][0] == 0.0 ) //AVC-COMPATIBLE
            {
              UInt uiTL;
              for( uiTL = 0; uiTL < MAX_TEMP_LEVELS; uiTL++) //search minimum base layer bitrate
                if( adMinBitrate[m_uiBaseLayerId][uiTL] )
                  break;
              adMinBitrate[m_uiLayerId][uiStage] = m_aaauidSeqBits[m_uiLayerId][uiStage][uiFGS]*dScale +
                ( adMinBitrate[m_uiBaseLayerId][uiTL]*m_auiNumFramesCoded[uiTL]/m_auiNumFramesCoded[uiStage] ) / ( 1 << ( uiTL - uiStage ) );
            }
          }
          else //high layer without corresponding TL in base layer
          {
            adMinBitrate[m_uiLayerId][uiStage] = m_aaauidSeqBits[m_uiLayerId][uiStage][0]*dScale +
              adMinBitrate[m_uiLayerId][uiStage-1]*m_auiNumFramesCoded[uiStage-1]/m_auiNumFramesCoded[uiStage]*2;
          }//if(adMinBitrate[m_uiBaseLayerId][uiStage]) //base layer exist for same TL
        }
        }
      } // if( uiFGS == 0 )
      if( uiFGS == 0 && m_dNumFGSLayers != 0 )
      {
        printf( " %9s @ %7.4lf" " %8.2lf"" %10.4lf" " %10.4lf""\n",
          acResolution,
          dFps,
          (Double)uiFGS,
          dBitrate,
          adMinBitrate[m_uiLayerId][uiStage] );
      }
      else if ( uiFGS == 0 && m_dNumFGSLayers == 0 )
      {
        printf( " %9s @ %7.4lf" " %8.2lf"" %10.4lf" " %10.4lf" " %8.4lf" " %8.4lf" " %8.4lf" "\n",
          acResolution,
          dFps,
          (Double)uiFGS,
          dBitrate,
          adMinBitrate[m_uiLayerId][uiStage],
          m_adPSNRSumY    [uiStage],
          m_adPSNRSumU    [uiStage],
          m_adPSNRSumV    [uiStage] );
      }
      else if( uiFGS == m_dNumFGSLayers )
      {
        printf( " %9s @ %7.4lf" " %8.2lf" " %10.4lf""           " " %8.4lf" " %8.4lf" " %8.4lf" "\n",
          acResolution,
          dFps,
          (Double)uiFGS,
          dBitrate,
          m_adPSNRSumY  [uiStage],
          m_adPSNRSumU  [uiStage],
          m_adPSNRSumV  [uiStage] );
      }
      else //uiFGS != 0, != m_dNumFGSLayers
      {
        printf( " %9s @ %7.4lf" " %8.2lf" " %10.4lf""\n",
          acResolution,
          dFps,
          (Double)uiFGS,
          dBitrate );
      }
// BUG_FIX liuhui}
    }
  }

  ruiNumCodedFrames = m_auiNumFramesCoded[uiMaxStage];
  rdOutputRate      = m_fOutputFrameRate;

  //S051{
  if(m_uiAnaSIP>0&&m_cOutSIPFileName.length())
  {
    FILE* file=fopen(m_cOutSIPFileName.c_str(),"wt");

    if(file==NULL)
      return Err::m_nOK;

    for(UInt poc=0;poc<m_uiTotalFrame;poc++)
    {
      if(m_auiFrameBits[poc]!=0)
        fprintf(file,"%d ",m_auiFrameBits[poc]);
    }
    fclose(file);
  }
  //S051}

  return Err::m_nOK;
}

// BUG_FIX liuhui{
ErrVal
MCTFEncoder::SingleLayerFinish( Double aaadBits[MAX_LAYERS][MAX_TEMP_LEVELS][MAX_QUALITY_LEVELS],
                 Double aaadSingleBitrate[MAX_LAYERS][MAX_TEMP_LEVELS][MAX_QUALITY_LEVELS] )
{
  ROFRS( m_auiNumFramesCoded[0], Err::m_nOK );
  Double aaadCurrBits[MAX_LAYERS][MAX_TEMP_LEVELS][MAX_QUALITY_LEVELS];
  for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++)
    for( UInt uiTempLevel = 0; uiTempLevel < MAX_TEMP_LEVELS; uiTempLevel++)
      for( UInt uiQualityLevel = 0; uiQualityLevel < MAX_QUALITY_LEVELS; uiQualityLevel++ )
        aaadCurrBits[uiLayer][uiTempLevel][uiQualityLevel] = aaadBits[uiLayer][uiTempLevel][uiQualityLevel];
  UInt  uiStage;
  UInt  uiMaxStage        = m_uiDecompositionStages - m_uiNotCodedMCTFStages;
  UInt  uiMinStage        = 0; //bugfix replace
  //bugfix delete
  {
    for( UInt uiFGS = 0; uiFGS <= m_dNumFGSLayers; uiFGS++)
      for( UInt uiTempLevel = 0; uiTempLevel < uiMinStage; uiTempLevel++)
        aaadCurrBits[m_uiLayerId][uiMinStage][uiFGS] += aaadCurrBits[m_uiLayerId][uiTempLevel][uiFGS];
  }
  for( uiStage = uiMinStage; uiStage <= uiMaxStage; uiStage++ )
  {
    Double  dFps    = m_fOutputFrameRate / (Double)( 1 << ( uiMaxStage - uiStage ) );
    Double  dScale  = dFps / 1000 / (Double)m_auiNumFramesCoded[uiStage];

    for( UInt uiFGS = 0; uiFGS <= m_dNumFGSLayers; uiFGS++ )
    {
      aaadSingleBitrate[m_uiLayerId][uiStage][uiFGS] = aaadCurrBits[m_uiLayerId][uiStage][uiFGS] * dScale;
    }
  }
  return Err::m_nOK;
}
// BUG_FIX liuhui}


ErrVal
MCTFEncoder::xSetRplrAndMmco( SliceHeader& rcSH )
{
  // clear L1 rplr buffer
  rcSH.getRplrBuffer(LIST_1).setRefPicListReorderingFlag( false );
  rcSH.getRplrBuffer(LIST_1).clear();

  // clear mmco buffer
  rcSH.getMmcoBuffer().clear();
  rcSH.setAdaptiveRefPicBufferingFlag( false );

  UInt uiCurrFrameNr = rcSH.getFrameNum();

  // leave if idr
  if( rcSH.isIdrNalUnit() )
  {
    m_cLPFrameNumList.clear();
    m_cLPFrameNumList.push_front( uiCurrFrameNr );
    return Err::m_nOK;
  }

  // generate rplr commands
  AOT( m_cLPFrameNumList.size() < rcSH.getNumRefIdxActive( LIST_0 ) );
  UIntList            cTempList;
  UIntList::iterator  iter = m_cLPFrameNumList.begin();
  for( UInt n = 0; n < rcSH.getNumRefIdxActive( LIST_0 ); n++ )
  {
    cTempList.push_back( *iter++ );
  }
  xSetRplr( rcSH.getRplrBuffer(LIST_0), cTempList, uiCurrFrameNr );

  // calculate number of mmco commands
  const UInt  uiMaxFrameNumber  = ( 1 << rcSH.getSPS().getLog2MaxFrameNum() );
  const Int   iDiffA             = m_cLPFrameNumList.front() - uiCurrFrameNr;
  UInt        uiDiffA            = ( uiMaxFrameNumber - iDiffA ) % uiMaxFrameNumber;

  // generate mmco commands for inter b frames
  UInt uiPos = 0;
  while( --uiDiffA )
  {
    rcSH.getMmcoBuffer().set( uiPos++, Mmco( MMCO_SHORT_TERM_UNUSED, uiDiffA-1 ) );
  }

  // generate mmco command for high-pass frame
  UInt uiNeedLowPassBefore = max( 1, rcSH.getNumRefIdxActive( LIST_0 ) );
  if( m_cLPFrameNumList.size() > uiNeedLowPassBefore )
  {
    const Int iDiffB   = m_cLPFrameNumList.popBack() - uiCurrFrameNr;
    UInt      uiDiffB  = ( uiMaxFrameNumber - iDiffB ) % uiMaxFrameNumber;
    rcSH.getMmcoBuffer().set( uiPos++, Mmco( MMCO_SHORT_TERM_UNUSED, uiDiffB-1 ) );
  }

  // end of command list
  if ( uiPos )
  {
  rcSH.getMmcoBuffer().set( uiPos, Mmco( MMCO_END) );

  rcSH.setAdaptiveRefPicBufferingFlag( true );
  }
  else
  rcSH.setAdaptiveRefPicBufferingFlag( false );

  // insert frame_num
  m_cLPFrameNumList.push_front( uiCurrFrameNr );

  return Err::m_nOK;
}


ErrVal
MCTFEncoder::xSetRplr( RplrBuffer&  rcRplrBuffer,
                       UIntList     cFrameNumList,
                       UInt         uiCurrFrameNr )
{
  rcRplrBuffer.clear();

  if( cFrameNumList.empty() )
  {
    rcRplrBuffer.setRefPicListReorderingFlag( false );
    return Err::m_nOK;
  }

  UIntList::iterator  iter            = cFrameNumList.begin();
  UInt                uiCurrReorderNr = uiCurrFrameNr;
  UInt                uiCount         = 0;
  Int                 iSum            = 0;
  Bool                bNeg            = false;
  Bool                bPos            = false;

  for( ; iter != cFrameNumList.end(); iter++)
  {
    Int  iDiff = *iter - uiCurrReorderNr;
    AOF( iDiff );

    if( iDiff < 0 )
    {
      Int iVal  = -iDiff - 1;
      rcRplrBuffer.set( uiCount++, Rplr( RPLR_NEG, iVal) );
      bNeg      = true;
      iSum     += iVal;
    }
    else
    {
      Int iVal  =  iDiff - 1;
      rcRplrBuffer.set( uiCount++, Rplr( RPLR_POS, iVal) );
      bPos      = true;
      iSum     += iVal;
    }
    uiCurrReorderNr = *iter;
  }
  rcRplrBuffer.set( uiCount++, Rplr( RPLR_END ) );
  rcRplrBuffer.setRefPicListReorderingFlag( true );

  if( iSum == 0 && ( bPos == ! bNeg ) && !m_bForceReOrderingCommands )
  {
    rcRplrBuffer.clear();
    rcRplrBuffer.setRefPicListReorderingFlag( false );
  }

  return Err::m_nOK;
}


//JVT-S036 lsj start
ErrVal
MCTFEncoder::xSetMmcoBase( SliceHeader& pcSliceHeader, UInt iNum )
{
  SliceHeader& rcSH = pcSliceHeader;
  rcSH.getMmcoBaseBuffer().clear();
  rcSH.setAdaptiveRefPicMarkingFlag( false );

  // leave if idr
    if( rcSH.isIdrNalUnit() )
  {
    return Err::m_nOK;
  }
  //generate mmco commands
  if( rcSH.getUseBasePredictionFlag() )
  {
    UInt uiPos = 0;
	//bug-fix 11/16/06{{
	Int   iDiff             =  iNum - rcSH.getFrameNum(); 
	const UInt  uiMaxFrameNumber  = ( 1 << rcSH.getSPS().getLog2MaxFrameNum() );
	UInt        uiDiff            = ( uiMaxFrameNumber - iDiff ) % uiMaxFrameNumber;

	rcSH.getMmcoBaseBuffer().set( uiPos++, Mmco( MMCO_SHORT_TERM_UNUSED, uiDiff-1 ) );
	//bug-fix 11/16/06}}

    rcSH.getMmcoBaseBuffer().set( uiPos, Mmco( MMCO_END) );
    rcSH.setAdaptiveRefPicMarkingFlag( true );

  }

  return Err::m_nOK;
}
//JVT-S036 lsj end


ErrVal
MCTFEncoder::xWriteSEI( ExtBinDataAccessorList& rcOutExtBinDataAccessorList, SliceHeader& rcSH, UInt& ruiBit )
{
  UInt uiBit = 0;
  Bool m_bWriteSEI = true;
  if( m_bWriteSEI )
  {
    RNOK( xInitExtBinDataAccessor        (  m_cExtBinDataAccessor ) );
    RNOK( m_pcNalUnitEncoder->initNalUnit( &m_cExtBinDataAccessor ) );

    SEI::MessageList cSEIMessageList;
    SEI::SubSeqInfo* pcSubSeqInfo;
    RNOK( SEI::SubSeqInfo::create( pcSubSeqInfo ) );

    const UInt uiSubSeqLayer         = /*m_uiDecompositionStages+1-*/rcSH.getTemporalLevel();
    const Bool bFirstRefPicFlag      = rcSH.getNalRefIdc() && ! m_abIsRef[uiSubSeqLayer];
    const Bool bLeadingNonRefPicFlag = (0 == rcSH.getNalRefIdc());

    m_abIsRef[uiSubSeqLayer] |= (0 != rcSH.getNalRefIdc());

    cSEIMessageList.push_back( pcSubSeqInfo );
    RNOK( pcSubSeqInfo->init( uiSubSeqLayer, 0, bFirstRefPicFlag, bLeadingNonRefPicFlag ) );

    RNOK( m_pcNalUnitEncoder->write( cSEIMessageList ) );

    RNOK( m_pcNalUnitEncoder->closeNalUnit( uiBit ) );
    RNOK( xAppendNewExtBinDataAccessor( rcOutExtBinDataAccessorList, &m_cExtBinDataAccessor ) );
    uiBit += 4*8;
    ruiBit += uiBit;
  }

  return Err::m_nOK;
}

//JVT-S036 lsj start
ErrVal
MCTFEncoder::xWriteSuffixUnit( ExtBinDataAccessorList& rcOutExtBinDataAccessorList, SliceHeader& rcSH, UInt& ruiBit )
{
  UInt uiBit = 0;
  Bool m_bWriteSuffixUnit = true;
  if( m_bWriteSuffixUnit )
  {
    RNOK( xInitExtBinDataAccessor        (  m_cExtBinDataAccessor ) );
    RNOK( m_pcNalUnitEncoder->initNalUnit( &m_cExtBinDataAccessor ) );

  NalUnitType eNalUnitType = rcSH.getNalUnitType();
  UInt eLayerId = rcSH.getLayerId();
  UInt eQualityLevel = rcSH.getQualityLevel();


  rcSH.setLayerId( 0 );
  rcSH.setQualityLevel( 0 );

  if( eNalUnitType == NAL_UNIT_CODED_SLICE )
  {
    rcSH.setNalUnitType( NAL_UNIT_CODED_SLICE_SCALABLE );
  }
  else if ( eNalUnitType == NAL_UNIT_CODED_SLICE_IDR )
  {
    rcSH.setNalUnitType( NAL_UNIT_CODED_SLICE_IDR_SCALABLE );
  }
  else
  {
    return Err::m_nERR;
  }

    RNOK( m_pcNalUnitEncoder->write( rcSH ) );

    RNOK( m_pcNalUnitEncoder->closeNalUnit( uiBit ) );
    RNOK( xAppendNewExtBinDataAccessor( rcOutExtBinDataAccessorList, &m_cExtBinDataAccessor ) );
    uiBit += 4*8;
    ruiBit += uiBit;

  rcSH.setNalUnitType( eNalUnitType );
  rcSH.setLayerId( eLayerId );
  rcSH.setQualityLevel( eQualityLevel );
  }

  return Err::m_nOK;
}

//JVT-S036 lsj end

//NonRequired JVT-Q066 (06-04-08){{
ErrVal
MCTFEncoder::xSetNonRequiredSEI(SliceHeader* pcSliceHeader, SEI::NonRequiredSei* pcNonRequiredSei)
{
  if( pcSliceHeader->getBaseLayerId() == MSYS_UINT_MAX || m_uiLayerId - pcSliceHeader->getBaseLayerId() > 1 )
  {
    if(pcNonRequiredSei->getNumInfoEntriesMinus1() != MSYS_UINT_MAX)
      pcNonRequiredSei->setNumInfoEntriesMinus1(pcNonRequiredSei->getNumInfoEntriesMinus1()+1);
    else
      pcNonRequiredSei->setNumInfoEntriesMinus1(0);
    pcNonRequiredSei->setEntryDependencyId(pcNonRequiredSei->getNumInfoEntriesMinus1(), m_uiLayerId);

    UInt temp = 0;
    UInt i = pcNonRequiredSei->getNumInfoEntriesMinus1();
    UInt j = 0;

    if(pcSliceHeader->getBaseLayerId() == MSYS_UINT_MAX)
      temp = m_uiLayerId + 1;
    else
      temp = m_uiLayerId - pcSliceHeader->getBaseLayerId();

    while(temp > 1)
    {
      if(pcNonRequiredSei->getNumNonRequiredPicsMinus1(i) != MSYS_UINT_MAX)
        pcNonRequiredSei->setNumNonRequiredPicsMinus1(i,pcNonRequiredSei->getNumNonRequiredPicsMinus1(i)+4);
      else
        pcNonRequiredSei->setNumNonRequiredPicsMinus1(i, 3);

      for(UInt k = 0; j <= pcNonRequiredSei->getNumNonRequiredPicsMinus1(i) ; k++, j++)
      {
        pcNonRequiredSei->setNonNonRequiredPicDependencyId(i, j, m_uiLayerId + 1 - temp);
        pcNonRequiredSei->setNonNonRequiredPicQulityLevel(i, j, k);
        pcNonRequiredSei->setNonNonRequiredPicFragmentOrder(i, j, 0);
      }
      temp--;
    }
  }
  else if(pcSliceHeader->getBaseQualityLevel() != 3)
  {
    if(pcNonRequiredSei->getNumInfoEntriesMinus1() != MSYS_UINT_MAX)
      pcNonRequiredSei->setNumInfoEntriesMinus1(pcNonRequiredSei->getNumInfoEntriesMinus1()+1);
    else
      pcNonRequiredSei->setNumInfoEntriesMinus1(0);
    pcNonRequiredSei->setEntryDependencyId(pcNonRequiredSei->getNumInfoEntriesMinus1(), m_uiLayerId);

    UInt i = pcNonRequiredSei->getNumInfoEntriesMinus1();

    pcNonRequiredSei->setNumNonRequiredPicsMinus1(i, 3 - pcSliceHeader->getBaseQualityLevel() - 1);

    for(UInt j = 0; j <= pcNonRequiredSei->getNumNonRequiredPicsMinus1(i) ; j++)
    {
      pcNonRequiredSei->setNonNonRequiredPicDependencyId(i, j, pcSliceHeader->getBaseLayerId());
      pcNonRequiredSei->setNonNonRequiredPicQulityLevel(i, j, pcSliceHeader->getBaseQualityLevel() + j + 1);
      pcNonRequiredSei->setNonNonRequiredPicFragmentOrder(i, j, 0);
    }
  }
  return Err::m_nOK;
}

ErrVal
MCTFEncoder::xWriteNonRequiredSEI( ExtBinDataAccessorList& rcOutExtBinDataAccessorList, SEI::NonRequiredSei* pcNonRequiredSei, UInt& ruiBit )
{
  UInt uiBit = 0;

  RNOK( xInitExtBinDataAccessor        (  m_cExtBinDataAccessor ) );
  RNOK( m_pcNalUnitEncoder->initNalUnit( &m_cExtBinDataAccessor ) );

  SEI::MessageList cSEIMessageList;
  cSEIMessageList.push_back( pcNonRequiredSei );

  RNOK( m_pcNalUnitEncoder->write( cSEIMessageList ) );
  RNOK( m_pcNalUnitEncoder->closeNalUnit( uiBit ) );

  ROF( &m_cExtBinDataAccessor );
  ROF( m_cExtBinDataAccessor.data() );
  UInt    uiNewSize     = m_cExtBinDataAccessor.size();
  UChar*  pucNewBuffer  = new UChar [ uiNewSize ];
  ROF( pucNewBuffer );
  ::memcpy( pucNewBuffer, m_cExtBinDataAccessor.data(), uiNewSize * sizeof( UChar ) );

  ExtBinDataAccessor* pcNewExtBinDataAccessor = new ExtBinDataAccessor;
  ROF( pcNewExtBinDataAccessor );
  m_cBinData              .reset          ();
  m_cBinData              .set            (  pucNewBuffer, uiNewSize );
  m_cBinData              .setMemAccessor ( *pcNewExtBinDataAccessor );
  rcOutExtBinDataAccessorList.push_front     (  pcNewExtBinDataAccessor );
  m_cBinData              .reset          ();
  m_cBinData              .setMemAccessor ( m_cExtBinDataAccessor );

  uiBit += 4*8;
  ruiBit += uiBit;

  return Err::m_nOK;
}
/*
ErrVal
MCTFEncoder::xWriteNonRequiredSEI( ExtBinDataAccessorList& rcOutExtBinDataAccessorList, UInt& ruiBit )
{
UInt uiBit = 0;
Bool m_bWriteSEI = true;
UInt temp1, temp2;

if( m_bWriteSEI )
{
RNOK( xInitExtBinDataAccessor        (  m_cExtBinDataAccessor ) );
RNOK( m_pcNalUnitEncoder->initNalUnit( &m_cExtBinDataAccessor ) );

SEI::MessageList cSEIMessageList;
SEI::NonRequiredSei* pcNonRequiredSei;
RNOK( SEI::NonRequiredSei::create( pcNonRequiredSei ) );

cSEIMessageList.push_back( pcNonRequiredSei );


//----- set the non-required sei parameter -----
// these parameters should be write according to cfg file or the details of encoding
pcNonRequiredSei->setNumInfoEntriesMinus1(0);
temp1 = pcNonRequiredSei->getNumInfoEntriesMinus1() + 1;
for(UInt i = 0; i < temp1; i++)
{
pcNonRequiredSei->setEntryDependencyId(i, 2);
pcNonRequiredSei->setNumNonRequiredPicsMinus1(i,0);
temp2 = pcNonRequiredSei->getNumNonRequiredPicsMinus1(i) + 1;
for(UInt j = 0; j < temp2; j++)
{
pcNonRequiredSei->setNonNonRequiredPicDependencyId(i,j,1);
pcNonRequiredSei->setNonNonRequiredPicQulityLevel(i,j,1);
pcNonRequiredSei->setNonNonRequiredPicFragmentOrder(i,j,0);
}
}


RNOK( m_pcNalUnitEncoder->write( cSEIMessageList ) );

RNOK( m_pcNalUnitEncoder->closeNalUnit( uiBit ) );
RNOK( xAppendNewExtBinDataAccessor( rcOutExtBinDataAccessorList, &m_cExtBinDataAccessor ) );
uiBit += 4*8;
ruiBit += uiBit;
}

return Err::m_nOK;
}*/
//NonRequired JVT-Q066 (06-04-08)}}

ErrVal
MCTFEncoder::xGetFrameNumList( SliceHeader& rcSH, UIntList& rcFrameNumList, ListIdx eLstIdx, UInt uiCurrBasePos )
{
  rcFrameNumList.clear();

  const UInt uiLevel   = rcSH.getTemporalLevel  ();
  const UInt uiMaxSize = rcSH.getNumRefIdxActive( eLstIdx );
  ROF( uiMaxSize );


  if( eLstIdx == LIST_1 )
  {
    for( Int i = (Int)uiCurrBasePos+1; i <= (Int)m_uiGOPSize; i++ )
    {
    if( m_pacControlData[i].getSliceHeader()->getTemporalLevel() < uiLevel && !m_papcFrame[i]->getUnusedForRef())  // JVT-Q065 EIDR
      {
        rcFrameNumList.push_back( m_pacControlData[i].getSliceHeader()->getFrameNum() );
        if( rcFrameNumList.size() == uiMaxSize )
        {
          break;
        }
      }
    }

// JVT-Q065 EIDR{
  if(rcFrameNumList.size() < uiMaxSize)
  {
    for( Int i = uiCurrBasePos-1; i >= 0; i-- )
    {
      if( m_pacControlData[i].getSliceHeader()->getTemporalLevel() < uiLevel && !m_papcFrame[i]->getUnusedForRef())
      {
        rcFrameNumList.push_back( m_pacControlData[i].getSliceHeader()->getFrameNum() );
        if( rcFrameNumList.size() == uiMaxSize )
        {
          break;
        }
      }
    }
  }
// JVT-Q065 EIDR}
  }
  else
  {
    for( Int i = uiCurrBasePos-1; i >= 0; i-- )
    {
    if( m_pacControlData[i].getSliceHeader()->getTemporalLevel() < uiLevel && !m_papcFrame[i]->getUnusedForRef()) // JVT-Q065 EIDR
      {
        rcFrameNumList.push_back( m_pacControlData[i].getSliceHeader()->getFrameNum() );
        if( rcFrameNumList.size() == uiMaxSize )
        {
          break;
        }
      }
    }
// JVT-Q065 EIDR{
  if(rcFrameNumList.size() < uiMaxSize)
  {
    for( Int i = (Int)uiCurrBasePos+1; i <= (Int)m_uiGOPSize; i++ )
    {
      if( m_pacControlData[i].getSliceHeader()->getTemporalLevel() < uiLevel && !m_papcFrame[i]->getUnusedForRef())
      {
        rcFrameNumList.push_back( m_pacControlData[i].getSliceHeader()->getFrameNum() );
        if( rcFrameNumList.size() == uiMaxSize )
        {
          break;
        }
      }
    }
    }
// JVT-Q065 EIDR}
  }

  ROF( rcFrameNumList.size() == uiMaxSize );

  return Err::m_nOK;
}



MbDataCtrl*
MCTFEncoder::xGetMbDataCtrlL1( SliceHeader& rcSH, UInt uiCurrBasePos )
{
  const UInt uiLevel   = rcSH.getTemporalLevel();

  for( UInt i = uiCurrBasePos+1; i <= m_uiGOPSize; i++ )
  {
    if( m_pacControlData[i].getSliceHeader() && uiLevel > m_pacControlData[i].getSliceHeader()->getTemporalLevel() )
    {
      return m_pacControlData[i].getMbDataCtrl();
    }
  }
  return 0;
}

Void
MCTFEncoder::xAssignSimplePriorityId( SliceHeader* pcSliceHeader )
{
    // Lookup simple priority ID from mapping table (J. Ridge, Y-K. Wang @ Nokia)
    Bool bFound = false;
//JVT-S036 lsj start
 //   for ( UInt uiSimplePriId = 0; uiSimplePriId < (1 << PRI_ID_BITS); uiSimplePriId++ )
  //  {
   //     UInt uiLayer, uiTempLevel, uiQualLevel;
   //     m_pcSPS->getSimplePriorityMap( uiSimplePriId, uiTempLevel, uiLayer, uiQualLevel );
  //      if ( pcSliceHeader->getTemporalLevel() == uiTempLevel && m_uiLayerId == uiLayer && pcSliceHeader->getQualityLevel() == uiQualLevel )
    //    {
            pcSliceHeader->setSimplePriorityId ( 0 ); //lsj The syntax element is not used by the decoding process specified in this Recommendation
            bFound = true;
      //      break;
    //    }
   // }
//JVT-S036 lsj end
    //AOF( bFound );
}

//TMM_ESS {
ErrVal
MCTFEncoder::xFillPredictionLists_ESS( UInt          uiBaseLevel,
                                      UInt          uiFrame)

{
    UInt          uiFrameIdInGOP  = ( uiFrame << uiBaseLevel );
    SliceHeader*  pcSliceHeader   = m_pacControlData[uiFrameIdInGOP].getSliceHeader();
    UInt          uiList0Size     = pcSliceHeader->getNumRefIdxActive( LIST_0 );
    UInt          uiList1Size     = pcSliceHeader->getNumRefIdxActive( LIST_1 );
    m_pcResizeParameters->initRefListPoc();

    //===== list 0 =====
    Int iFrameId,idx=0;
    for( iFrameId = Int( uiFrame - 1 ); iFrameId >= 0 && uiList0Size; iFrameId -= 2)
    {
        IntFrame* pcFrame = m_papcFrame[ iFrameId << uiBaseLevel ];
        m_pcResizeParameters->m_aiRefListPoc[0][idx++]=pcFrame->getPOC();
        uiList0Size--;
    }

    //===== list 1 =====
    idx=0;
    for( iFrameId = Int( uiFrame + 1 ); iFrameId <= (Int)( m_uiGOPSize >> uiBaseLevel ) && uiList1Size; iFrameId += 2 )
    {
        IntFrame* pcFrame = m_papcFrame[ iFrameId << uiBaseLevel ];
        m_pcResizeParameters->m_aiRefListPoc[1][idx++]=pcFrame->getPOC();
        uiList1Size--;
    }

    return Err::m_nOK;
}
//TMM_ESS }



ErrVal
MCTFEncoder::setDiffPrdRefLists( RefFrameList& diffPrdRefList, IntFrame* baseFrame, IntFrame* enhFrame,
                                 YuvBufferCtrl* pcYuvFullPelBufferCtrl )
{
  IntFrame* pcDiffFrame;
  ROFRS   ( ( pcDiffFrame                  = new IntFrame( *pcYuvFullPelBufferCtrl,
                                                           *pcYuvFullPelBufferCtrl ) ), Err::m_nERR );

  pcDiffFrame->init();

  pcDiffFrame->subtract(enhFrame, baseFrame);
  RNOK( pcDiffFrame->extendFrame( NULL ) );
  RNOK( diffPrdRefList.add( pcDiffFrame ) );

  return Err::m_nOK;
}


ErrVal
MCTFEncoder::freeDiffPrdRefLists( RefFrameList& diffPrdRefList)
{
  for(UInt i=0; i< diffPrdRefList.getSize(); i++)
  {
    diffPrdRefList.getEntry(i)->uninit();
    free(diffPrdRefList.getEntry(i));
  }

  return Err::m_nOK;
}

//S051{
Bool MCTFEncoder:: xSIPCheck  (UInt POC)
{
  if(POC==0)               //There seems to be  a bug in decoder if we can
    return false;        //discard picture with POC=0. So here I forbid POC=0
  if(std::find(m_cPOCList.begin(),m_cPOCList.end(),POC)!=m_cPOCList.end())
    return true;
  return false;
}

int MCTFEncoder::xGetMbDataCtrlL1Pos( const SliceHeader& rcSH, UInt uiCurrBasePos )
{
  const UInt uiLevel   = rcSH.getTemporalLevel();
  for( UInt i = uiCurrBasePos+1; i <= m_uiGOPSize; i++ )
  {
    if( m_pacControlData[i].getSliceHeader() && uiLevel > m_pacControlData[i].getSliceHeader()->getTemporalLevel() )
    {
    return i;
    }
  }
  return -1;
}
//S051}

//JVT-U106 Behaviour at slice boundaries{
ErrVal
MCTFEncoder::xConstrainedIntraUpsampling(IntFrame* pcFrame,
										 IntFrame* pcUpsampling, 
										 IntFrame* pcTemp,
										 MbDataCtrl* pcBaseDataCtrl,
										 ReconstructionBypass* pcReconstructionBypass,
										 ResizeParameters* pcResizeParameters)
{
	int input_width   = pcResizeParameters->m_iInWidth;
	int output_width  = pcResizeParameters->m_iGlobWidth;  
	int output_height = pcResizeParameters->m_iGlobHeight;

	if(pcResizeParameters->m_iSpatialScalabilityType)
	{
		UInt uiMbInRow=input_width>>4;
		Int** ppiMaskL,**ppiMaskC;
		Int* piXL,*piXC,*piYL,*piYC;
		Int  k,l,m,n;
		UInt  uiSliceNbr=1;
 
		ppiMaskL=new Int*[output_height];
		for(k=0;k<output_height;k++)
		{
			ppiMaskL[k]=new Int[output_width];
		}
		ppiMaskC=new Int*[output_height/2];
		for(k=0;k<output_height/2;k++)
		{
			ppiMaskC[k]=new Int[output_width/2];
		}

		piXL=new Int[output_width];
		piXC=new Int[output_width/2];
		piYL=new Int[output_height];
		piYC=new Int[output_height/2];
		xGetPosition(pcResizeParameters,piXL,piYL,false);
		xGetPosition(pcResizeParameters,piXC,piYC,true);

		//when enhancement layer macroblock covers more than one slice 
		//in its base layer,it cannot be coded using Intra_BL
		for(k=0;k<output_height/16;k++)
		{
			for(l=0;l<output_width/16;l++)
			{
				Bool bIntra=true;
				UInt uiLastSliceID = MSYS_UINT_MAX;
				UInt uiCurrSliceID;
				for(m=0;m<16;m++)
				{
					for(n=0;n<16;n++)
					{
						if(piXL[l*16+n]!=-128&&piYL[k*16+m]!=-128)
						{
							Int  iMbX=piXL[l*16+n]>>4;
							Int  iMbY=piYL[k*16+m]>>4;
							uiCurrSliceID=pcBaseDataCtrl->getMbData(iMbX,iMbY).getSliceId();
							if(uiLastSliceID==-1)
							{
                                 uiLastSliceID=uiCurrSliceID;
								 continue;
							}
							else if(uiLastSliceID!=uiCurrSliceID)
							     bIntra=false;
							uiLastSliceID=uiCurrSliceID;
						}
					}
				}
				m_pbIntraBLFlag[k*uiMbInRow+l]=bIntra;
			}          
		}


		m_apcFrameTemp[1]->setZero();
		pcTemp->copy(pcFrame);

		//Assume slice id is ordered from 1 2 3...
		//Get number of slices in picture
		for(UInt i=0;i<pcBaseDataCtrl->getSize();i++)
		{
			UInt          uiMbY             = i / uiMbInRow;
			UInt          uiMbX             = i % uiMbInRow;
			if(pcBaseDataCtrl->getMbData(uiMbX,uiMbY).getSliceId()>uiSliceNbr)
				uiSliceNbr=pcBaseDataCtrl->getMbData(uiMbX,uiMbY).getSliceId();
		}

		//Upsampling slice by slice and copy to the dest buffer
		for(UInt iSliceID=1;iSliceID<=uiSliceNbr;iSliceID++)
		{
			pcFrame->copyPortion(pcTemp);
			for(UInt uiMbAddress= 0 ;uiMbAddress<pcBaseDataCtrl->getSize();uiMbAddress++)
				//===== loop over macroblocks use raster scan =====
			{
				UInt          uiMbY             = uiMbAddress / uiMbInRow;
				UInt          uiMbX             = uiMbAddress % uiMbInRow;
				pcFrame->getFullPelYuvBuffer()->getYuvBufferCtrl().initMb(uiMbY, uiMbX);
				UInt uiMask = 0;
				RNOK( pcBaseDataCtrl->getBoundaryMaskCIU( uiMbY, uiMbX, uiMask, iSliceID ) );
				if( uiMask )
				{
					IntYuvMbBufferExtension cBuffer;
					cBuffer.setAllSamplesToZero();

					cBuffer.loadSurrounding( pcFrame->getFullPelYuvBuffer() );

					RNOK( pcReconstructionBypass->padRecMb( &cBuffer, uiMask ) );
					pcFrame->getFullPelYuvBuffer()->loadBuffer( &cBuffer );
				}
			}

			RNOK( pcUpsampling->copy( pcFrame ) );

			pcUpsampling->upsample(m_cDownConvert, pcResizeParameters, true);
			for(k=0;k<output_height;k++)
			{
				for(l=0;l<output_width;l++)
				{
					if(piXL[l]==-128||piYL[k]==-128)
						ppiMaskL[k][l]=0;
					else
					{
						Int  iMbX=piXL[l]>>4;
						Int  iMbY=piYL[k]>>4;
						if(pcBaseDataCtrl->getMbData(iMbX,iMbY).isIntra()&&(pcBaseDataCtrl->getMbData(iMbX,iMbY).getSliceId()==iSliceID))
							ppiMaskL[k][l]=1;
						else
							ppiMaskL[k][l]=0;
					}
				}          
			}

			for(k=0;k<output_height/2;k++)
			{
				for(l=0;l<output_width/2;l++)
				{
					if(piXC[l]==-128||piYC[k]==-128)
						ppiMaskL[k][l]=0;
					else
					{
						Int  iMbX=piXC[l]>>3;
						Int  iMbY=piYC[k]>>3;
						if(pcBaseDataCtrl->getMbData(iMbX,iMbY).isIntra()&&(pcBaseDataCtrl->getMbData(iMbX,iMbY).getSliceId()==iSliceID))
							ppiMaskC[k][l]=1;
						else
							ppiMaskC[k][l]=0;
					}
				}          
			}
			m_apcFrameTemp[1]->copyMask(pcUpsampling,ppiMaskL,ppiMaskC);
		}
		pcUpsampling->copy(m_apcFrameTemp[1]);

		//Memory free
		delete[]piXL;
		delete[]piYL;
		delete[]piXC;
		delete[]piYC;
		for(k=0;k<output_height;k++)
		{
			delete[]ppiMaskL[k];
		}
		delete[] ppiMaskL;
		for(k=0;k<output_height/2;k++)
		{
			delete[]ppiMaskC[k];
		}
		delete[] ppiMaskC;
	}
  else // fix by H. Schwarz
  {
    pcUpsampling->copy( pcFrame );
  }
	return Err::m_nOK;
}

void MCTFEncoder::xGetPosition(ResizeParameters* pcResizeParameters,Int*px,Int*py,bool uv_flag)

{
	Int iratio=uv_flag?2:1;
	int input_width   = pcResizeParameters->m_iInWidth/iratio;
	int input_height  = pcResizeParameters->m_iInHeight/iratio;
	int output_width  = pcResizeParameters->m_iGlobWidth/iratio;  
	int output_height = pcResizeParameters->m_iGlobHeight/iratio;
	int crop_x0 = pcResizeParameters->m_iPosX/iratio;
	int crop_y0 = pcResizeParameters->m_iPosY/iratio;
	int crop_w = pcResizeParameters->m_iOutWidth/iratio;
	int crop_h = pcResizeParameters->m_iOutHeight/iratio;  
	int input_chroma_phase_shift_x = pcResizeParameters->m_iBaseChromaPhaseX;
	int input_chroma_phase_shift_y = pcResizeParameters->m_iBaseChromaPhaseY;
	int output_chroma_phase_shift_x = pcResizeParameters->m_iChromaPhaseX;
	int output_chroma_phase_shift_y = pcResizeParameters->m_iChromaPhaseY;

	int i, j;
	bool ratio1_flag = ( input_width == crop_w );
	unsigned short deltaa, deltab;

	for(i=0; i<crop_x0; i++)  px[i] = -128;

	for(i=crop_x0+crop_w; i<output_width; i++)  px[i] = -128;

	if(ratio1_flag)
	{
		for(i = 0; i < crop_w; i++)
		{
			px[i+crop_x0] = i*16+4*(2+output_chroma_phase_shift_x)-4*(2+input_chroma_phase_shift_x);
		}
	}
	else
	{
		deltaa = ((input_width<<16) + (crop_w>>1))/crop_w;
		if(uv_flag)
		{
			deltab = ((input_width<<14) + (crop_w>>1))/crop_w;
			for(i = 0; i < crop_w; i++)
			{
				px[i+crop_x0] = ((i*deltaa + (2 + output_chroma_phase_shift_x)*deltab + 2048)>>12) - 4*(2 + input_chroma_phase_shift_x);
				px[i+crop_x0] =(Int)(px[i+crop_x0]/16.0+0.5);
			}
		}
		else
		{
			deltab = ((input_width<<15) + (crop_w>>1))/crop_w;
			for(i = 0; i < crop_w; i++)
			{
				px[i+crop_x0] = (i*deltaa + deltab - 30720)>>12;
				px[i+crop_x0] =(Int)(px[i+crop_x0]/16.0+0.5);
			}
		}
	}

	ratio1_flag = ( input_height == crop_h );

	for(j=0; j<crop_y0; j++)   py[j] = -128;

	for(j=crop_y0+crop_h; j<output_height; j++)  py[j] = -128;

	if(ratio1_flag)
	{
		for(j = 0; j < crop_h; j++)
		{
			py[j+crop_y0] = j*16+4*(2+output_chroma_phase_shift_y)-4*(2+input_chroma_phase_shift_y);
		}
	}
	else
	{
		deltaa = ((input_height<<16) + (crop_h>>1))/crop_h;
		if(uv_flag)
		{
			deltab = ((input_height<<14) + (crop_h>>1))/crop_h;
			for(j = 0; j < crop_h; j++)
			{
				py[j+crop_y0] = ((j*deltaa + (2 + output_chroma_phase_shift_y)*deltab + 2048)>>12) - 4*(2 + input_chroma_phase_shift_y);
				py[j+crop_y0] =(Int)(py[j+crop_y0]/16.0+0.5);
			}
		}
		else
		{
			deltab = ((input_height<<15) + (crop_h>>1))/crop_h;
			for(j = 0; j < crop_h; j++)
			{
				py[j+crop_y0] = (j*deltaa + deltab - 30720)>>12;
				py[j+crop_y0] =(Int)(py[j+crop_y0]/16.0+0.5);
			}
		}
	}
}
//JVT-U106 Behaviour at slice boundaries}
H264AVC_NAMESPACE_END


