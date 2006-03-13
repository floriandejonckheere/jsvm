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
, m_uiScalableLayerId								( 0 )
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
, m_uiIntraMode                     ( 0 )
, m_uiNumMaxIter                    ( 0 )
, m_uiIterSearchRange               ( 0 )
, m_iMaxDeltaQp                     ( 0 )
, m_uiClosedLoopMode                ( 0 )
, m_bUpdate                         ( false )
, m_bH264AVCCompatible              ( true  )
, m_bInterLayerPrediction           ( true  )
, m_bAdaptivePrediction             ( true  )
, m_bHaarFiltering                  ( false )
, m_bBiPredOnly                     ( false )
, m_bAdaptiveQP                     ( true  )
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
, m_papcBQFrame                     ( 0 )
, m_papcCLRecFrame                  ( 0 )
, m_papcResidual                    ( 0 )
, m_papcSubband                     ( 0 )
, m_pcLowPassBaseReconstruction     ( 0 )
, m_pcAnchorFrameOriginal           ( 0 )
, m_pcAnchorFrameReconstructed      ( 0 )
, m_pcBaseLayerFrame                ( 0 )
, m_pcBaseLayerResidual             ( 0 )
, m_papcSmoothedFrame								( 0 ) // JVT-R091
//----- control data arrays -----
, m_pacControlData                  ( 0 )
, m_pcBaseLayerCtrl                 ( 0 )
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
{
  ::memset( m_abIsRef,          0x00, sizeof( m_abIsRef           ) );
  ::memset( m_apcFrameTemp,     0x00, sizeof( m_apcFrameTemp      ) );
  ::memset( m_apcLowPassTmpOrg, 0x00, sizeof( m_apcLowPassTmpOrg  ) );

  UInt ui;
  for( ui = 0; ui <= MAX_DSTAGES; ui++ )
  {
    m_auiNumFramesCoded [ui]  = 0;
    m_auiCurrGOPBitsBase[ui]  = 0;
    m_auiCurrGOPBitsFGS [ui]  = 0;
    m_adSeqBitsBase     [ui]  = 0.0;
    m_adSeqBitsFGS      [ui]  = 0.0;
    m_adPSNRSumY        [ui]  = 0.0;
    m_adPSNRSumU        [ui]  = 0.0;
    m_adPSNRSumV        [ui]  = 0.0;
  }
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
		m_auiCurrGOPBits		[ui] = 0;
		m_adSeqBits					[ui] = 0.0;
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
                   MotionEstimation*  pcMotionEstimation )
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

  //----- references -----
  m_pcSPS                   = 0;
  m_pcPPSLP                 = 0;
  m_pcPPSHP                 = 0;
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

  //----- fixed control parameters -----
  m_dLowPassEnhRef          = pcCodingParameter->getLowPassEnhRef();
  m_uiLowPassFgsMcFilter    = pcCodingParameter->getLowPassFgsMcFilter();

  pcLayerParameters->getAdaptiveRefFGSWeights(
    m_uiBaseWeightZeroBaseBlock, m_uiBaseWeightZeroBaseCoeff);
  m_uiFgsEncStructureFlag = pcLayerParameters->getFgsEncStructureFlag();

  m_uiLayerId               = pcLayerParameters->getLayerId                 ();
  m_uiBaseLayerId           = pcLayerParameters->getBaseLayerId             ();
  m_uiBaseQualityLevel      = pcLayerParameters->getBaseQualityLevel        ();

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
  m_uiIntraMode             = pcLayerParameters->getMCTFIntraMode           ();
  m_uiNumMaxIter            = pcCodingParameter->getMotionVectorSearchParams().getNumMaxIter      ();
  m_uiIterSearchRange       = pcCodingParameter->getMotionVectorSearchParams().getIterSearchRange ();
  m_iMaxDeltaQp             = pcLayerParameters->getMaxAbsDeltaQP           ();
  m_uiClosedLoopMode        = pcLayerParameters->getClosedLoop              ();
  m_bUpdate                 = pcLayerParameters->getUpdateStep              ()  > 0  &&
                            ( pcCodingParameter->getBaseLayerMode           () == 0 || m_uiLayerId >  0 );
  ROT( m_uiClosedLoopMode && m_bUpdate ); // double-check
  m_bH264AVCCompatible      = pcCodingParameter->getBaseLayerMode           ()  > 0 && m_uiLayerId == 0;
  m_bInterLayerPrediction   = pcLayerParameters->getInterLayerPredictionMode()  > 0;
  m_bAdaptivePrediction     = pcLayerParameters->getInterLayerPredictionMode()  > 1;
  m_bHaarFiltering          = false;
  m_bBiPredOnly             = false;
  m_bAdaptiveQP             = pcLayerParameters->getAdaptiveQPSetting       ()  > 0;
  m_bForceReOrderingCommands= pcLayerParameters->getForceReorderingCommands ()  > 0;
  m_bWriteSubSequenceSei    = pcCodingParameter->getBaseLayerMode           ()  > 1 && m_uiLayerId == 0;

#if MULTIPLE_LOOP_DECODING
  m_bCompletelyDecodeLayer          = ( pcCodingParameter->getNumberOfLayers() > m_uiLayerId+1 &&
                                        pcCodingParameter->getLayerParameters( m_uiLayerId+1).getInterLayerPredictionMode() > 0 &&
                                        pcCodingParameter->getLayerParameters( m_uiLayerId+1).getDecodingLoops() > 1 );
  m_bHighestLayer                   = ( pcCodingParameter->getNumberOfLayers() == m_uiLayerId + 1 );
#endif

  // TMM_ESS 
  m_pcResizeParameters = pcLayerParameters->getResizeParameters();

  for( UInt uiStage = 0; uiStage < MAX_DSTAGES; uiStage++ )
  {
    m_adBaseQpLambdaMotion[uiStage] = pcLayerParameters->getQpModeDecision( uiStage );
  }
  m_dBaseQPResidual         = pcLayerParameters->getBaseQpResidual          ();
  m_dNumFGSLayers           = pcLayerParameters->getNumFGSLayers            ();

  m_uiFilterIdc             = pcCodingParameter->getLoopFilterParams        ().getFilterIdc       ();
  m_iAlphaOffset            = pcCodingParameter->getLoopFilterParams        ().getAlphaOffset     ();
  m_iBetaOffset             = pcCodingParameter->getLoopFilterParams        ().getBetaOffset      ();

  m_bLoadMotionInfo         = pcLayerParameters->getMotionInfoMode          () == 1;
  m_bSaveMotionInfo         = pcLayerParameters->getMotionInfoMode          () == 2;
  m_pMotionInfoFile         = 0;

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
    m_auiCurrGOPBitsBase[ui]  = 0;
    m_auiCurrGOPBitsFGS [ui]  = 0;
    m_adSeqBitsBase     [ui]  = 0.0;
    m_adSeqBitsFGS      [ui]  = 0.0;
    m_adPSNRSumY        [ui]  = 0.0;
    m_adPSNRSumU        [ui]  = 0.0;
    m_adPSNRSumV        [ui]  = 0.0;
  }
	for( ui = 0; ui < MAX_SCALABLE_LAYERS; ui++ ) 
	{
		m_auiCurrGOPBits		[ui] = 0;
		m_adSeqBits					[ui] = 0.0;
	}

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
  
  //{{Adaptive GOP structure
  // --ETRI & KHU
  m_uiUseAGS = pcCodingParameter->getUseAGS();
  m_uiMaxDecStages = m_uiDecompositionStages; // -- 10.18.2005
  if (m_uiUseAGS) 
  {
	  m_uiWriteGOPMode = pcCodingParameter->getWriteGOPMode();

	  if ( m_uiWriteGOPMode ) 
    {
		  FILE* d_gop;
		  m_cGOPModeFilename = pcCodingParameter->getGOPModeFile();
		  d_gop = fopen(pcCodingParameter->getGOPModeFile().c_str(), "w");
		  fclose(d_gop);
		  m_uiSelect = NULL;
		  m_dMSETemp = 0;
		  m_uiSelectPos = 0;
	  }
	  else 
    {
		  FILE* d_gop;
      d_gop = fopen(pcCodingParameter->getGOPModeFile().c_str(), "rt");
//      ROTREPORT( d_gop == NULL, "need \"gop mode\" file\n");
		  if (d_gop == NULL) 
      {
			  printf("need \"gop mode\" file\n");
			  exit(0);
		  }
		  
      m_bFinish = 0;

		  UInt temp;
		  int i, j;
		  int line = 0;
		  char ch;
		  while(EOF != (ch = fgetc(d_gop))) 
      {
			if (ch == '\n')
				line++;
		  }

		  m_uiSelect = new UInt*[line];
		  
		  for(j = 0; j < line + 1; j++) 
      { 
			  m_uiSelect[j] = new UInt[8];
			  for (i = 0; i < 8; i++) {
				  m_uiSelect[j][i] = 0;
			  }
		  }
		  fseek(d_gop, 0, SEEK_SET);
		  
		  for(j = 0; j < line; j++) 
      {
			  UInt sum = 0;
        for(i = 0;sum < (UInt)(1<<(pcLayerParameters->getDecompositionStages())); i++)
        {
				  fscanf(d_gop, "%d ", &temp);
				  if (temp > 6)
					  break;
				  //m_uiSelect[j][i] = temp + m_uiLayerId;
          m_uiSelect[j][i] = temp + (pcLayerParameters->getDecompositionStages() 
                                      - pcCodingParameter->getLayerParameters(0).getDecompositionStages());
				  sum += (1<<m_uiSelect[j][i]);
			  }
			  if (temp > 6)
				  break;
		  }

		  if (m_uiSelect[0][0] == 0) 
      {
			  printf("\"gop mode\" file is empty\n");
			  exit(0);		
		  }
		  
		  fclose(d_gop);
	  }
  }
  //}}Adaptive GOP structure

  return Err::m_nOK;
}



__inline UInt downround2powerof2( UInt i ) { UInt r = 1; for( ; (UInt)( 1 << r ) <= i; r++ ); return ( 1 << ( r - 1 ) ); }

ErrVal
MCTFEncoder::initParameterSets( const SequenceParameterSet& rcSPS,
                                const PictureParameterSet&  rcPPSLP,
                                const PictureParameterSet&  rcPPSHP )
{
  //===== set references =====
  m_pcSPS                 = &rcSPS;
  m_pcPPSLP               = &rcPPSLP;
  m_pcPPSHP               = &rcPPSHP;

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
  ROFRS   ( ( m_papcFrame                     = new IntFrame* [ m_uiMaxGOPSize + 1 ]      ), Err::m_nERR );
  if( m_uiClosedLoopMode == 2 )
  {
    ROFRS ( ( m_papcBQFrame                   = new IntFrame* [ m_uiMaxGOPSize + 1 ]      ), Err::m_nERR );
  }

  if( m_uiQualityLevelForPrediction < 3 || m_bUseDiscardableUnit) //JVT-P031
  {
    ROFRS ( ( m_papcCLRecFrame                = new IntFrame* [ m_uiMaxGOPSize + 1 ]      ), Err::m_nERR );
  }
  ROFRS   ( ( m_papcResidual                  = new IntFrame* [ m_uiMaxGOPSize + 1 ]      ), Err::m_nERR );
  ROFRS   ( ( m_papcSubband                   = new IntFrame* [ m_uiMaxGOPSize + 1 ]      ), Err::m_nERR );

	//-- JVT-R091
	ROFRS   ( ( m_papcSmoothedFrame							= new IntFrame* [ m_uiMaxGOPSize + 1 ]      ), Err::m_nERR );
	//--
  
  for( uiIndex = 0; uiIndex <= m_uiMaxGOPSize; uiIndex++ )
  {
    ROFRS ( ( m_papcFrame         [ uiIndex ] = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                              *m_pcYuvHalfPelBufferCtrl ) ), Err::m_nERR );
    if( m_papcBQFrame )
    {
      ROFRS(( m_papcBQFrame       [ uiIndex ] = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                              *m_pcYuvFullPelBufferCtrl ) ), Err::m_nERR );
    }
    if( m_papcCLRecFrame )
    {
      ROFRS(( m_papcCLRecFrame    [ uiIndex ] = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                              *m_pcYuvHalfPelBufferCtrl ) ), Err::m_nERR );
    }

		//-- JVT-R091
		if ( m_papcSmoothedFrame )
		{
      ROFRS(( m_papcSmoothedFrame	[ uiIndex ] = new IntFrame( *m_pcYuvFullPelBufferCtrl,
																															*m_pcYuvHalfPelBufferCtrl ) ), Err::m_nERR );
		}
		//--

    ROFRS ( ( m_papcResidual      [ uiIndex ] = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                              *m_pcYuvHalfPelBufferCtrl ) ), Err::m_nERR );
    ROFRS ( ( m_papcSubband       [ uiIndex ] = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                              *m_pcYuvHalfPelBufferCtrl ) ), Err::m_nERR );
    RNOK  (   m_papcFrame         [ uiIndex ] ->init        () );
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
			RNOK(   m_papcSmoothedFrame	[ uiIndex ] ->init				() );
		}
		//--

    RNOK  (   m_papcResidual      [ uiIndex ] ->init        () );
    RNOK  (   m_papcSubband       [ uiIndex ] ->init        () );
  }
  
  for( uiIndex = 0; uiIndex < NUM_TMP_FRAMES;  uiIndex++ )
  {
    ROFRS ( ( m_apcFrameTemp      [ uiIndex ] = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                              *m_pcYuvHalfPelBufferCtrl ) ), Err::m_nERR );
    RNOK  (   m_apcFrameTemp      [ uiIndex ] ->init        () );
  }
  
  if( m_bH264AVCCompatible )
  {
    for( uiIndex = 0; uiIndex < 2;  uiIndex++ )
    {
      ROFRS ( ( m_apcLowPassTmpOrg[ uiIndex ] = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                              *m_pcYuvHalfPelBufferCtrl ) ), Err::m_nERR );
      RNOK  (   m_apcLowPassTmpOrg[ uiIndex ]   ->init        () );
    }
  }
  
  for( uiIndex = 0; uiIndex < 2;  uiIndex++ )
  {
    for( UInt uiLayerIdx = 0; uiLayerIdx < 4; uiLayerIdx ++ )
    {
      ROFRS   ( ( m_aapcFGSRecon[uiIndex][uiLayerIdx]   = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                              *m_pcYuvHalfPelBufferCtrl ) ), Err::m_nERR );
      RNOK    (   m_aapcFGSRecon[uiIndex][uiLayerIdx]   ->init        () );
    }
  }

  ROFRS   ( ( m_aapcFGSPredFrame   = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                          *m_pcYuvHalfPelBufferCtrl ) ), Err::m_nERR );
  RNOK    (   m_aapcFGSPredFrame   ->init        () );

  ROFRS   ( ( m_pcLowPassBaseReconstruction   = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                              *m_pcYuvHalfPelBufferCtrl ) ), Err::m_nERR );
  RNOK    (   m_pcLowPassBaseReconstruction   ->init        () );
    
  ROFRS   ( ( m_pcAnchorFrameOriginal         = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                              *m_pcYuvHalfPelBufferCtrl ) ), Err::m_nERR );
  RNOK    (   m_pcAnchorFrameOriginal         ->init        () );
  
  ROFRS   ( ( m_pcAnchorFrameReconstructed    = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                              *m_pcYuvHalfPelBufferCtrl ) ), Err::m_nERR );
  RNOK    (   m_pcAnchorFrameReconstructed    ->init        () );
  
  ROFRS   ( ( m_pcBaseLayerFrame              = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                              *m_pcYuvHalfPelBufferCtrl ) ), Err::m_nERR );
  RNOK    (   m_pcBaseLayerFrame              ->init        () );
  
  ROFRS   ( ( m_pcBaseLayerResidual           = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                              *m_pcYuvHalfPelBufferCtrl ) ), Err::m_nERR );
  RNOK    (   m_pcBaseLayerResidual           ->init        () );



  //========== CREATE MACROBLOCK DATA MEMORIES ==========
  ROFRS   ( ( m_pacControlData  = new ControlData[ m_uiMaxGOPSize + 1 ] ), Err::m_nERR );

  for( uiIndex = 0; uiIndex <= m_uiMaxGOPSize; uiIndex++ )
  {
    MbDataCtrl*   pcMbDataCtrl                = 0;
    ROFRS ( (     pcMbDataCtrl                = new MbDataCtrl  () ), Err::m_nERR );
    RNOK  (       pcMbDataCtrl                ->init            ( rcSPS ) );
    RNOK  (       m_pacControlData[ uiIndex ] . setMbDataCtrl   ( pcMbDataCtrl ) );
    RNOK  (       m_pacControlData[ uiIndex ] .initFGSData      ( m_uiFrameWidthInMb * m_uiFrameHeightInMb ) );

    Bool          bLowPass                    = ( ( uiIndex % ( 1 << m_uiDecompositionStages ) ) == 0 );
    SliceHeader*  pcSliceHeader               = 0;
    ROFRS ( (     pcSliceHeader               = new SliceHeader ( *m_pcSPS, bLowPass ? *m_pcPPSLP : *m_pcPPSHP ) ), Err::m_nERR );
    RNOK  (       m_pacControlData[ uiIndex ] . setSliceHeader  (  pcSliceHeader ) );

    if( m_uiClosedLoopMode == 2 )
    {
      RNOK(       m_pacControlData[ uiIndex ] .initBQData       ( m_uiFrameWidthInMb * m_uiFrameHeightInMb ) );
    }

    m_pacControlData[ uiIndex ].getMbDataCtrl()->initFgsBQData(m_uiFrameWidthInMb * m_uiFrameHeightInMb);
  }
  
  MbDataCtrl*   pcMbDataCtrl    = 0;
  ROFRS   ( (   pcMbDataCtrl    = new MbDataCtrl() ), Err::m_nERR );
  RNOK    (     pcMbDataCtrl    ->init          ( rcSPS ) );
  RNOK    (     m_cControlDataUpd.setMbDataCtrl ( pcMbDataCtrl ) );

  ROFRS   ( ( m_pcBaseLayerCtrl = new MbDataCtrl() ), Err::m_nERR );
  RNOK    (   m_pcBaseLayerCtrl ->init          ( rcSPS ) );



  //========== CREATE UPDATE WEIGHTS ARRAY and WRITE BUFFER ==========
  UInt  uiNum4x4Blocks        = m_uiFrameWidthInMb * m_uiFrameHeightInMb * 4 * 4;
  m_uiWriteBufferSize         = 3 * ( uiNum4x4Blocks * 4 * 4 );
  ROFRS( ( m_pucWriteBuffer   = new UChar [ m_uiWriteBufferSize ] ), Err::m_nERR );
  ROT ( m_cDownConvert    .init   ( m_uiFrameWidthInMb<<4, m_uiFrameHeightInMb<<4 ) );
  RNOK( m_pcRQFGSEncoder ->initSPS( rcSPS ) );

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
        RNOK(   m_papcFrame[ uiIndex ]->uninit() );
        delete  m_papcFrame[ uiIndex ];
        m_papcFrame[ uiIndex ] = 0;
      }
    }
    delete [] m_papcFrame;
    m_papcFrame = 0;
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
  
  for( uiIndex = 0; uiIndex < 2; uiIndex++ )
  {
    if( m_apcLowPassTmpOrg[ uiIndex ] )
    {
      RNOK(   m_apcLowPassTmpOrg[ uiIndex ]->uninit() );
      delete  m_apcLowPassTmpOrg[ uiIndex ];
      m_apcLowPassTmpOrg[ uiIndex ] = 0;
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


  if( m_pcBaseLayerCtrl )
  {
    RNOK( m_pcBaseLayerCtrl->uninit() );
    delete m_pcBaseLayerCtrl;
    m_pcBaseLayerCtrl = 0;
  }
  

  //========== DELETE UPDATE WEIGHTS ARRAY and WRITE BUFFER ==========
  delete [] m_pucWriteBuffer;
  m_pucWriteBuffer    = 0;
  m_uiWriteBufferSize = 0;


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
                                UInt             uiIterSearchRange,
                                UInt             uiIntraMode )
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
   
      //===== estimate prediction data =====
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
                                               uiIntraMode,
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

  return Err::m_nOK;
}




ErrVal
MCTFEncoder::xMotionCompensation( IntFrame*        pcMCFrame,
                                 RefFrameList*    pcRefFrameList0,
                                 RefFrameList*    pcRefFrameList1,
                                 MbDataCtrl*      pcMbDataCtrl,
                                 SliceHeader&     rcSH )
{
  Bool        bCalcMv         = false;
  Bool        bFaultTolerant  = false;
  MbEncoder*  pcMbEncoder     = m_pcSliceEncoder->getMbEncoder();

  RNOK( pcMbDataCtrl        ->initSlice( rcSH, PRE_PROCESS, false, NULL ) );
  RNOK( m_pcMotionEstimation->initSlice( rcSH              ) );

  for( UInt uiMbIndex = 0; uiMbIndex < m_uiMbNumber; uiMbIndex++ )
  {
    UInt          uiMbY           = uiMbIndex / m_uiFrameWidthInMb;
    UInt          uiMbX           = uiMbIndex % m_uiFrameWidthInMb;
    MbDataAccess* pcMbDataAccess  = 0;


    RNOK( pcMbDataCtrl            ->initMb( pcMbDataAccess, uiMbY, uiMbX                  ) );
    RNOK( m_pcYuvFullPelBufferCtrl->initMb(                 uiMbY, uiMbX ) );
    RNOK( m_pcYuvHalfPelBufferCtrl->initMb(                 uiMbY, uiMbX ) );
    RNOK( m_pcMotionEstimation    ->initMb(                 uiMbY, uiMbX, *pcMbDataAccess ) );

    RNOK( pcMbEncoder->compensatePrediction( *pcMbDataAccess, pcMCFrame,
                                             *pcRefFrameList0, *pcRefFrameList1,
                                             bCalcMv, bFaultTolerant ) );
  }

  return Err::m_nOK;
}





ErrVal
MCTFEncoder::xGetConnections( ControlData&  rcControlData,
                              Double&       rdL0Rate,
                              Double&       rdL1Rate,
                              Double&       rdBiRate )
{
  if( ! m_bAdaptiveQP )
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


  MbDataCtrl*   pcMbDataCtrl  = rcControlData.getMbDataCtrl();
  SliceHeader*  pcSliceHeader = rcControlData.getSliceHeader();
  UInt          uiL0Pred      = 0;
  UInt          uiL1Pred      = 0;
  UInt          uiBiPred      = 0;

  RNOK( pcMbDataCtrl->initSlice( *pcSliceHeader, PRE_PROCESS, false, NULL ) );

  for( UInt uiMbIndex = 0; uiMbIndex < m_uiMbNumber; uiMbIndex++ )
  {
    UInt          uiMbY           = uiMbIndex / m_uiFrameWidthInMb;
    UInt          uiMbX           = uiMbIndex % m_uiFrameWidthInMb;
    MbDataAccess* pcMbDataAccess  = 0;

    RNOK( pcMbDataCtrl->initMb( pcMbDataAccess, uiMbY, uiMbX ) );

    for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
    {
      UShort    usFwdBwd = pcMbDataAccess->getMbData().getFwdBwd( cIdx );
      if      ( usFwdBwd == 1 )  { uiL0Pred++; }
      else if ( usFwdBwd == 2 )  { uiL1Pred++; }
      else if ( usFwdBwd == 3 )  { uiBiPred++; }
    }
  }

  UInt  uiSum = ( m_uiFrameWidthInMb * m_uiFrameHeightInMb * 16 );
  rdL0Rate    = (Double)uiL0Pred / (Double)uiSum;
  rdL1Rate    = (Double)uiL1Pred / (Double)uiSum;
  rdBiRate    = (Double)uiBiPred / (Double)uiSum;

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
                              UInt&                   ruiBits )
{
  Bool          bFinished     = false;
  SliceHeader*  pcSliceHeader = rcControlData.getSliceHeader();
  MbDataCtrl*   pcMbDataCtrl  = rcControlData.getMbDataCtrl ();
  IntFrame*     pcOrgResidual = pcTempFrame;
  UInt          uiRealBLBits  = ruiBits;
  UInt          uiLastRecLayer = 0;
  ruiBits                     = 0;
  SliceType     eBaseSliceType = pcSliceHeader->getSliceType();
  IntFrame      *pcRecTemp = m_apcFrameTemp[2];   


  pcSliceHeader->setFirstMbInSlice(0);
  pcSliceHeader->setLastMbInSlice(pcSliceHeader->getMbInPic()-1);

  //===== set original residual signal =====
  RNOK( pcOrgResidual->subtract( pcFrame, pcPredSignal ) );
  RNOK( xAddBaseLayerResidual( rcControlData, pcOrgResidual, true ) );

  UInt uiTarget;
  UInt uiFGSMaxBits = 0;
  UInt uiBaseBits                 =   0;
  UInt uiEstimatedHeaderBits      =   0;
  UInt uiFrameBits                =   0;
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
  RNOK( m_pcRQFGSEncoder->initPicture( rcControlData.getSliceHeader(),
                                       rcControlData.getMbDataCtrl(),
                                       pcOrgResidual,
                                       m_dNumFGSLayers,
                                       rcControlData.getLambda(),
                                       m_iMaxDeltaQp,
                                       bFinished,
                                       (m_uiFGSMode == 2 ), 
                                       m_bUseDiscardableUnit) ); //JVT-P031

  // HS: bug-fix for m_uiQualityLevelForPrediction == 0
  if( 0 == m_uiQualityLevelForPrediction && !bAlreadyReconstructed) 
  {
    RNOK( m_pcRQFGSEncoder->reconstruct   ( pcRecTemp ) );
    RNOK( xAddBaseLayerResidual           ( rcControlData, pcRecTemp, false ) );
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
    UInt  uiPacketBits  = 0;
    Bool bBaseLayerKeyPic = pcSliceHeader->getTemporalLevel() == 0;;

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

      RefFrameList  cRefListDiff;

      setDiffPrdRefLists( cRefListDiff, pcLowPassRefFrameBase, pcLowPassRefFrameEnh, m_pcYuvFullPelBufferCtrl );

      m_pcMotionEstimation->loadAdaptiveRefPredictors(
        m_pcYuvFullPelBufferCtrl, 
        pcPredSignal, m_aapcFGSPredFrame, 
        &cRefListDiff, 
        rcControlData.getMbDataCtrl(), 
        m_pcRQFGSEncoder,
        rcControlData.getSliceHeader());

      freeDiffPrdRefLists(cRefListDiff);

      RNOK( pcOrgResidual->subtract( pcFrame,  pcPredSignal ) );

      RNOK( xAddBaseLayerResidual( rcControlData, pcOrgResidual, true ) );

      // this is not necessary as the pointer was passed to the FGS encoder already
      m_pcRQFGSEncoder->setNewOriginalResidual( pcOrgResidual );

    }
    if( bBaseLayerKeyPic )
    {
      if(m_uiFgsEncStructureFlag == 0)
        m_uiNumLayers[1] = uiRecLayer + 1;
      else
        m_uiNumLayers[1] = 2;
    }

    //JVT-P031
    Bool bCorrupted = false;
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

    //----- init NAL UNIT -----
    RNOK( xInitExtBinDataAccessor               (  m_cExtBinDataAccessor ) );
    RNOK( m_pcNalUnitEncoder->initNalUnit       ( &m_cExtBinDataAccessor ) );

    //---- write Slice Header -----
    ETRACE_NEWSLICE;
    //pcSliceHeader->setQualityLevel       ( uiRecLayer );
    xAssignSimplePriorityId( pcSliceHeader );

    //JVT-P031
    // set fragment status
      Bool bFragmented = ( m_bUseDiscardableUnit && ((!bSetToDiscardable && (uiPredTarget < uiFGSBits[uiRecLayer-1] - uiEstimatedHeaderBits)) || uiFrac!=0) );
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
        if(m_bUseDiscardableUnit == true && bSetToDiscardable)
            pcSliceHeader->setDiscardableFlag(true);
      }
    //~JVT-P031

    RNOK( m_pcNalUnitEncoder->write  ( *pcSliceHeader ) );

    Int iQp = pcSliceHeader->getPicQp();
    uiLastRecLayer = uiRecLayer;

    //---- encode next bit-plane for current NAL unit ----
    
    uiFGSCutBits = (bFragmented && uiFrac == 0 || !bFragmented && m_bUseDiscardableUnit && !bSetToDiscardable  ) ? uiPredFGSMaxBits : uiFGSMaxBits; //JVT-P031  

    Bool bCorrupted = false;
    
    //JVT-P031
    if(m_uiFGSMode == 2 && uiFGSCutBits < uiFGSBits[uiRecLayer-1] - uiEstimatedHeaderBits) // probably truncate in payload
    {
         uiFGSCutBits -= uiEstimatedHeaderBits;
    }
    RNOK( m_pcRQFGSEncoder->encodeNextLayer     ( bFinished, bCorrupted, uiFGSCutBits, uiFrac, bFragmented, ( m_uiFGSMode == 1 && !m_bUseDiscardableUnit ? m_pFGSFile : 0 ) ) ); //FIX_FRAG_CAVLC
     
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
    uiFrac++;
    //~JVT-P031

    //----- close NAL UNIT -----
    RNOK( m_pcNalUnitEncoder->closeNalUnit      ( uiFragmentBits ) ); //JVT-P031
      
    RNOK( xAppendNewExtBinDataAccessor          ( rcOutExtBinDataAccessorList, &m_cExtBinDataAccessor ) );

    if( bBaseLayerKeyPic && m_uiFgsEncStructureFlag == 0 )
    {
      IntFrame *pcTempFgsFrame = m_aapcFGSRecon[1][uiRecLayer];

      RNOK( m_pcRQFGSEncoder->reconstruct   ( pcTempFgsFrame ) );

      RNOK( xAddBaseLayerResidual           ( rcControlData, pcTempFgsFrame, false ) );
      RNOK( pcTempFgsFrame  ->add           ( pcPredSignal ) );
      RNOK( xClipIntraMacroblocks           ( pcTempFgsFrame, rcControlData, true ) );
    }

    //JVT-P031
    uiFragmentBits += 4*8;
    uiPacketBits += uiFragmentBits;
    if(!m_bUseDiscardableUnit || ((bFragmented && uiFrac == 1) || (!bFragmented && !bSetToDiscardable)))
         ruiBits += uiPacketBits;
  
    if((m_bUseDiscardableUnit && !bSetToDiscardable) )//|| (!bFragmented && !m_bUseDiscardableUnit ))
    {
      RNOK( m_pcRQFGSEncoder->reconstruct   ( pcRecTemp ) );
      RNOK( xAddBaseLayerResidual           ( rcControlData, pcRecTemp, false ) );
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

      printf("  Frame %4d ( LId%2d, TL%2d, QL%2d, PrRef,              QP%3d ) %10d bits\n",
      rcControlData.getSliceHeader()->getPoc                    (),
      rcControlData.getSliceHeader()->getLayerId                (),
      rcControlData.getSliceHeader()->getTemporalLevel          (),
      uiRecLayer, iQp, uiPacketBits );

      if(!bFragmented)
          break;         
    }
    //~JVT-P031
    if( m_uiFGSMode == 1 && m_pFGSFile && !m_bUseDiscardableUnit ) //FIX_FRAG_CAVLC
    {
      fprintf( m_pFGSFile, "\t%d", uiPacketBits );
    }
 
    m_auiCurrGOPBits[m_uiScalableLayerId + uiRecLayer] += uiPacketBits;
    if( uiRecLayer == m_uiQualityLevelForPrediction && !bAlreadyReconstructed) //JVT-P031
    {
      RNOK( m_pcRQFGSEncoder->reconstruct   ( pcRecTemp ) );
      RNOK( xAddBaseLayerResidual           ( rcControlData, pcRecTemp, false ) );
      RNOK( pcResidual      ->copy          ( pcRecTemp ) );
      RNOK( xZeroIntraMacroblocks           ( pcResidual, rcControlData ) );
      RNOK( pcRecTemp         ->add           ( pcPredSignal ) );
      RNOK( xClipIntraMacroblocks           ( pcRecTemp, rcControlData, pcSliceHeader->getTemporalLevel() == 0 ) );
      RNOK( pcSubband       ->copy          ( pcRecTemp ) );
      bAlreadyReconstructed = true;

      RNOK( rcControlData.saveMbDataQpAndCbp() );
    }
  }
  
  if(bAlreadyReconstructed)  // x. wang, Nokia
    pcFrame->copy(pcRecTemp);


  //===== reconstruction =====
  if( uiLastRecLayer < m_uiQualityLevelForPrediction &&  !bAlreadyReconstructed) //JVT-P031
  {
    RNOK( m_pcRQFGSEncoder->reconstruct   ( pcFrame ) );
    RNOK( xAddBaseLayerResidual           ( rcControlData, pcFrame, false ) );
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
    RNOK( xAddBaseLayerResidual           ( rcControlData, pcCLRec, false ) );
    RNOK( pcCLRec         ->add           ( pcPredSignal ) );
    RNOK( xClipIntraMacroblocks           ( pcCLRec, rcControlData, pcSliceHeader->getTemporalLevel() == 0 ) );
  }


  RNOK( m_pcRQFGSEncoder->finishPicture () );


  return Err::m_nOK;
}




ErrVal
MCTFEncoder::xEncodeLowPassSignal( ExtBinDataAccessorList&  rcOutExtBinDataAccessorList,
                                   ControlData&             rcControlData,
                                   IntFrame*                pcFrame,
                                   IntFrame*                pcResidual,
                                   IntFrame*                pcPredSignal,
                                   UInt&                    ruiBits )
{
  UInt          uiBits              = 0;
  UInt          uiBitsSEI           = 0;
  IntFrame*     pcBaseLayerFrame    = rcControlData.getBaseLayerRec ();
  IntFrame*     pcBaseLayerResidual = rcControlData.getBaseLayerSbb ();
  SliceHeader*  pcSliceHeader       = rcControlData.getSliceHeader  ();
    
  
  //----- subsequence SEI -----
  if( m_bWriteSubSequenceSei && m_bH264AVCCompatible )
  {
    RNOK( xWriteSEI( rcOutExtBinDataAccessorList, *pcSliceHeader, uiBitsSEI ) );
  }

  FMO* pcFMO = pcSliceHeader->getFMO();

  
  for (UInt iSliceGroupID=0;!pcFMO->SliceGroupCompletelyCoded(iSliceGroupID);iSliceGroupID++)  
  {
    UInt          uiBits              = 0;

    pcSliceHeader->setFirstMbInSlice(pcFMO->getFirstMacroblockInSlice(iSliceGroupID));
    pcSliceHeader->setLastMbInSlice(pcFMO->getLastMBInSliceGroup(iSliceGroupID));

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

  printf("  Frame %4d ( LId%2d, TL%2d, QL%2d, %s-%c, BId%2d, AP%2d, QP%3d ) %10d bits\n",
    pcSliceHeader->getPoc                    (),
    pcSliceHeader->getLayerId                (),
    pcSliceHeader->getTemporalLevel          (),
    pcSliceHeader->getQualityLevel           (),
    pcSliceHeader->isH264AVCCompatible       () ? "AVC" : " LP",
    pcSliceHeader->getSliceType              () == I_SLICE ? 'I' : 'P',
    pcSliceHeader->getBaseLayerId            (),
    pcSliceHeader->getAdaptivePredictionFlag () ? 1 : 0,
    pcSliceHeader->getPicQp                  (),
    uiBits + uiBitsSEI );

  ETRACE_NEWFRAME;

  ruiBits = ruiBits + uiBits + uiBitsSEI;
  uiBitsSEI=0;
  }
  
  return Err::m_nOK;
}







ErrVal
MCTFEncoder::xEncodeHighPassSignal( ExtBinDataAccessorList&  rcOutExtBinDataAccessorList,
                                    ControlData&             rcControlData,
                                    IntFrame*                pcFrame,
                                    IntFrame*                pcResidual,
                                    IntFrame*                pcPredSignal,
																		IntFrame*								 pcSRFrame, // JVT-R091
                                    UInt&                    ruiBits,
                                    UInt&                    ruiBitsRes )
{
  UInt  uiMbCoded   = 0;
  UInt  uiBits      = 0;
  UInt  uiBitsSEI   = 0;
  UInt  uiBitsRes   = 0;

  //----- Subsequence SEI -----
  if( m_bWriteSubSequenceSei &&   m_bH264AVCCompatible )
  {
    RNOK( xWriteSEI( rcOutExtBinDataAccessorList, *rcControlData.getSliceHeader(), uiBitsSEI ) );
  }


  FMO* pcFMO = rcControlData.getSliceHeader()->getFMO();
 
  
  for (UInt iSliceGroupID=0;!pcFMO->SliceGroupCompletelyCoded(iSliceGroupID);iSliceGroupID++)
  {
    UInt  uiBits      = 0;
    UInt  uiBitsRes   = 0;
    UInt  uiMbCoded   = 0;

    rcControlData.getSliceHeader()->setFirstMbInSlice(pcFMO->getFirstMacroblockInSlice(iSliceGroupID));
    rcControlData.getSliceHeader()->setLastMbInSlice(pcFMO->getLastMBInSliceGroup(iSliceGroupID));

    //----- init NAL UNIT -----
    RNOK( xInitExtBinDataAccessor        (  m_cExtBinDataAccessor ) );
    RNOK( m_pcNalUnitEncoder->initNalUnit( &m_cExtBinDataAccessor ) );

    //---- write Slice Header -----
    ETRACE_NEWSLICE;
    xAssignSimplePriorityId( rcControlData.getSliceHeader() );
    RNOK( m_pcNalUnitEncoder->write( *rcControlData.getSliceHeader() ) );

    //----- write slice data -----
    RNOK( m_pcSliceEncoder->encodeHighPassPicture ( uiMbCoded, uiBits, uiBitsRes,
                                                  *rcControlData.getSliceHeader            (),
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


    printf("  Frame %4d ( LId%2d, TL%2d, QL%2d, %s-%c, BId%2d, AP%2d, QP%3d ) %10d bits\n",
      rcControlData.getSliceHeader()->getPoc                    (),
      rcControlData.getSliceHeader()->getLayerId                (),
      rcControlData.getSliceHeader()->getTemporalLevel          (),
      rcControlData.getSliceHeader()->getQualityLevel           (),
      // heiko.schwarz@hhi.fhg.de: corrected output
      // rcControlData.getSliceHeader()->isH264AVCCompatible       () ? "AVC" : " LP",
      rcControlData.getSliceHeader()->isH264AVCCompatible       () ? "AVC" : " HP",
      rcControlData.getSliceHeader()->getSliceType              () == B_SLICE ? 'B' : 'P',
      rcControlData.getSliceHeader()->getBaseLayerId            (),
      rcControlData.getSliceHeader()->getAdaptivePredictionFlag () ? 1 : 0,
      rcControlData.getSliceHeader()->getPicQp                  (),
      uiBits + uiBitsSEI );

    ruiBits     += uiBits+uiBitsSEI;
    ruiBitsRes  += uiBitsRes;
    uiBitsSEI =0;
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
    m_papcFrame					[ uiFrame ]->copyAll( m_pcAnchorFrameOriginal );
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
    m_papcFrame					[ uiFrame ]->load         ( pcPicBuffer );
		m_papcSmoothedFrame	[ uiFrame ]->load         ( pcPicBuffer ); // JVT-R091
    m_papcFrame					[ uiFrame ]->setPOC       ( m_uiFrameCounter++ << m_uiTemporalResolution );
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
  for( UInt uiTemporalLevel = 0; uiTemporalLevel <= m_uiDecompositionStages; uiTemporalLevel++ )
  {
    UInt      uiStep    = ( 1 << ( m_uiDecompositionStages - uiTemporalLevel ) );
    for( UInt uiFrameId = uiStep; uiFrameId <= m_uiGOPSize; uiFrameId += ( uiStep << 1 ) )
    {
      RNOK( xInitSliceHeader( uiTemporalLevel, uiFrameId ) );
    }
  }
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
  

  UInt uiStage;
  m_uiNotYetConsideredBaseLayerBits = 0;
  UInt* pauiBLGopBitsBase           = m_pcH264AVCEncoder->getGOPBitsBase( m_uiBaseLayerId );
  UInt* pauiBLGopBitsFGS            = m_pcH264AVCEncoder->getGOPBitsFGS ( m_uiBaseLayerId );
 	UInt* pauiBLGopBits								= m_pcH264AVCEncoder->getGOPBits		( m_uiBaseLayerId );
  for( uiStage = 0; uiStage <= MAX_DSTAGES; uiStage++ )
  {
    m_auiCurrGOPBitsBase[uiStage]      = ( pauiBLGopBitsBase ? pauiBLGopBitsBase [uiStage] : 0 );
    m_auiCurrGOPBitsFGS [uiStage]      = ( pauiBLGopBitsFGS  ? pauiBLGopBitsFGS  [uiStage] : 0 );

    m_uiNotYetConsideredBaseLayerBits += m_auiCurrGOPBitsBase[uiStage];
    m_uiNotYetConsideredBaseLayerBits += m_auiCurrGOPBitsFGS [uiStage];
  }
	for( uiStage = 0; uiStage < MAX_SCALABLE_LAYERS; uiStage++ )
	{
		m_auiCurrGOPBits    [uiStage]      = ( pauiBLGopBits ? pauiBLGopBits[uiStage] : 0 );
	}	
  m_auiCurrGOPBitsBase  [0]           += m_uiParameterSetBits;
  m_uiNotYetConsideredBaseLayerBits   += m_uiParameterSetBits;
	m_auiCurrGOPBits      [0]           += m_uiParameterSetBits;

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

  return Err::m_nOK;
}


ErrVal
MCTFEncoder::getBaseLayerData( IntFrame*&     pcFrame,
                               IntFrame*&     pcResidual,
                               MbDataCtrl*&   pcMbDataCtrl,
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
        bForCopyOnly  = false;

        bConstrainedIPredBL = m_pacControlData[uiFrame].getSliceHeader()->getPPS().getConstrainedIntraPredFlag();
      }
      else if( ! m_bH264AVCCompatible && ! m_uiClosedLoopMode )
      {
        pcResidual      = 0;
        pcFrame         = 0;
        m_pacControlData[uiFrame].activateMbDataCtrlForQpAndCbp( false );
        pcMbDataCtrl    = m_pacControlData[uiFrame].getMbDataCtrl();
        bForCopyOnly    = true;
      }
      uiPos = uiFrame;
      break;
    }
  }


  if( iSpatialScalability != SST_RATIO_1 )
  {
    RNOK( m_apcFrameTemp[0]->copy( pcFrame ) );
    pcFrame = m_apcFrameTemp[0];

#if MULTIPLE_LOOP_DECODING
    if ( m_pacControlData[uiPos].getSliceHeader()->getPPS().getConstrainedIntraPredFlag() && !m_bCompletelyDecodeLayer )
#else
    if ( m_pacControlData[uiPos].getSliceHeader()->getPPS().getConstrainedIntraPredFlag() )
#endif
    {
      m_pcLoopFilter->setFilterMode( LoopFilter::LFMode( LoopFilter::LFM_NO_INTER_FILTER + LoopFilter::LFM_EXTEND_INTRA_SUR ) );
      RNOK( m_pcLoopFilter->process(*m_pacControlData[uiPos].getSliceHeader (),
                                     pcFrame,
                                     m_pacControlData[uiPos].getMbDataCtrl  (),
                                     m_pacControlData[uiPos].getMbDataCtrl  (),
                                     m_uiFrameWidthInMb,
                                     NULL,
                                     NULL,
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
                            UInt  auiPredListSize[2],
                            UInt  aauiUpdListSize[MAX_DSTAGES][2] )
{
  //----- clear update list sizes -----
  ::memset( aauiUpdListSize, 0x00, 2 * MAX_DSTAGES * sizeof( UInt ) );


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
    else if( m_bUpdate )
    {
      //========== UPDATE LIST SIZES ==========
      UInt* pauiUpdListSize = aauiUpdListSize[uiLevel-1];
      UInt  uiFrameIdWrap   = ( uiFrameIdLevel == 0 ? 0 : ( ( uiFrameIdLevel - 1 ) % uiBaseGOPSize ) + 1 );
      if( uiFrameIdWrap > uiCutOffFrame )
      {
        pauiUpdListSize[0]  = ( uiFrameIdWrap - uiCutOffFrame ) >> 1;
        pauiUpdListSize[1]  = ( uiBaseGOPSize - uiFrameIdWrap ) >> 1;
      }
      else
      {
        pauiUpdListSize[0]  = uiFrameIdWrap >> 1;
        pauiUpdListSize[1]  = ( uiFrameIdLevel == 0 ? 0 : ( uiCutOffFrame - uiFrameIdWrap + 1 ) >> 1 );
      }

      pauiUpdListSize[0]    = min( m_uiMaxNumRefFrames, pauiUpdListSize[0] );
      pauiUpdListSize[1]    = min( m_uiMaxNumRefFrames, pauiUpdListSize[1] );

      //----- take into account actual GOP size -----
      {
        UInt  uiMaxL1Size   = ( ( m_uiGOPSize >> uiBaseLevel ) + 1 - uiFrameIdLevel ) >> 1;
        pauiUpdListSize[1]  = min( uiMaxL1Size,         pauiUpdListSize[1] );
      }

      if( m_bHaarFiltering )
      {
        pauiUpdListSize[0]  = 0;
      }
    }
  }

  //----- check intra -----
  if( m_uiLowPassIntraPeriod != MSYS_UINT_MAX )
  {
	//{{Adaptive GOP structure -- 10.18.2005
    // --ETRI & KHU
    if (!m_uiUseAGS) {
    //}}Adaptive GOP structure -- 10.18.2005

		UInt  uiCurrFrame   = (   m_uiGOPNumber                << m_uiDecompositionStages ) + uiFrameIdInGOP;
		UInt  uiIntraPeriod = ( ( m_uiLowPassIntraPeriod + 1 ) << m_uiDecompositionStages );
		if( ( uiCurrFrame % uiIntraPeriod ) == 0 )
		{
		  auiPredListSize[0] = 0;
		  auiPredListSize[1] = 0;
		}
	//{{Adaptive GOP structure -- 10.18.2005
    // --ETRI & KHU
    }
    else {
      UInt  uiCurrFrame   = m_uiFrameCounter - 1 - m_uiGOPSize + uiFrameIdInGOP;
	    UInt  uiIntraPeriod = ( ( m_uiLowPassIntraPeriod + 1 ) << m_uiMaxDecStages );
      if( ( uiCurrFrame % uiIntraPeriod ) == 0 )
      {
        auiPredListSize[0] = 0;
        auiPredListSize[1] = 0;
      }
    }
    //}}Adaptive GOP structure -- 10.18.2005
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
  UInt  aauiUpdListSize[MAX_DSTAGES][2];
  RNOK( xGetListSizes( uiTemporalLevel, uiFrameIdInGOP, auiPredListSize, aauiUpdListSize ) );


  //===== get slice header parameters =====
  NalRefIdc     eNalRefIdc;

  if ( uiFrameIdInGOP == 0 || uiFrameIdInGOP == ( 1 << m_uiDecompositionStages ) )
	  eNalRefIdc = NAL_REF_IDC_PRIORITY_HIGHEST;
  else
	  eNalRefIdc = NalRefIdc( min( 2, max( 0, (Int)( m_uiDecompositionStages - m_uiNotCodedMCTFStages - uiTemporalLevel ) ) ) );

  NalUnitType   eNalUnitType    = ( m_bH264AVCCompatible
                                    ? ( uiFrameIdInGOP ? NAL_UNIT_CODED_SLICE          : NAL_UNIT_CODED_SLICE_IDR          )
                                    : ( uiFrameIdInGOP ? NAL_UNIT_CODED_SLICE_SCALABLE : NAL_UNIT_CODED_SLICE_IDR_SCALABLE ) );
  SliceType     eSliceType      = ( auiPredListSize[1] ? B_SLICE : auiPredListSize[0] ? P_SLICE : I_SLICE );
  UInt          uiGOPSize       = uiFrameIdInGOP ? m_uiGOPSize             : 1;
  UInt          uiDecStages     = uiFrameIdInGOP ? m_uiDecompositionStages : 0;

  Bool          bKeyPicture     = ( eNalRefIdc == NAL_REF_IDC_PRIORITY_HIGHEST ) ? 1 : 0;

  //===== set simple slice header parameters =====
  pcSliceHeader->setNalRefIdc                   ( eNalRefIdc            );
  pcSliceHeader->setNalUnitType                 ( eNalUnitType          );
  pcSliceHeader->setLayerId                     ( m_uiLayerId           );
  pcSliceHeader->setTemporalLevel               ( uiTemporalLevel       );
  pcSliceHeader->setQualityLevel                ( 0                     );
  //{{Variable Lengh NAL unit header data with priority and dead substream flag
  //France Telecom R&D- (nathalie.cammas@francetelecom.com)
  pcSliceHeader->setDiscardableFlag             ( false                 );
  pcSliceHeader->setExtensionFlag	              ( m_bExtendedPriorityId );
  pcSliceHeader->setSimplePriorityId            ( 0	                    );
  //}}Variable Lengh NAL unit header data with priority and dead substream flag

  pcSliceHeader->setFirstMbInSlice              ( 0                     );
  pcSliceHeader->setLastMbInSlice               ( m_uiMbNumber - 1      );
  pcSliceHeader->setSliceType                   ( eSliceType            );
  pcSliceHeader->setFrameNum                    ( m_uiFrameNum          );
  pcSliceHeader->setIdrPicId                    ( 0                     );
  pcSliceHeader->setDirectSpatialMvPredFlag     ( true                  );
  pcSliceHeader->setKeyPictureFlag              ( bKeyPicture           );
  pcSliceHeader->setNumRefIdxActiveOverrideFlag ( false                 );
  pcSliceHeader->setCabacInitIdc                ( 0                     );
  pcSliceHeader->setSliceHeaderQp               ( 0                     );
  // Currently hard-coded
  pcSliceHeader->setNumMbsInSlice               ( m_uiMbNumber          );
  pcSliceHeader->setFragmentOrder               ( 0 );

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

    //--- update ---
    for( UInt uiLevel = uiTemporalLevel; uiLevel < m_uiDecompositionStages; uiLevel++ )
    {
      pcSliceHeader->setNumRefIdxUpdate( uiLevel, LIST_0, aauiUpdListSize[uiLevel][0] );
      pcSliceHeader->setNumRefIdxUpdate( uiLevel, LIST_1, aauiUpdListSize[uiLevel][1] );
    }
  }

  //===== de-blocking filter parameters =====
  if( pcSliceHeader->getPPS().getDeblockingFilterParametersPresentFlag() )
  {
    pcSliceHeader->getDeblockingFilterParameter().setDisableDeblockingFilterIdc ( m_uiFilterIdc       );
    pcSliceHeader->getDeblockingFilterParameter().setSliceAlphaC0Offset         ( 2 * m_iAlphaOffset  );
    pcSliceHeader->getDeblockingFilterParameter().setSliceBetaOffset            ( 2 * m_iBetaOffset   );
  }

  //===== set remaining slice header parameters =====
  RNOK( m_pcPocCalculator->setPoc( *pcSliceHeader, m_papcFrame[uiFrameIdInGOP]->getPOC() ) );

  //===== set base layer data =====
  RNOK( xSetBaseLayerData( uiFrameIdInGOP ) );

  //===== prediction weights =====
  if( pcSliceHeader->isInterP() )
  {
    RNOK( pcSliceHeader->getPredWeightTable( LIST_0 ).uninit() );
    RNOK( pcSliceHeader->getPredWeightTable( LIST_0 ).init( 64 ) );

    if( pcSliceHeader->getPPS().getWeightedPredFlag() )
    {
      pcSliceHeader->setBasePredWeightTableFlag( false );
#if INFER_ELAYER_PRED_WEIGHTS
      if( pcSliceHeader->getBaseLayerId() != MSYS_UINT_MAX )
      {
        pcSliceHeader->setBasePredWeightTableFlag( true );
      }
#endif
      if( ! pcSliceHeader->getBasePredWeightTableFlag() )
      {
        pcSliceHeader->setLumaLog2WeightDenom   ( 6 );
        pcSliceHeader->setChromaLog2WeightDenom ( 6 );
        RNOK( pcSliceHeader->getPredWeightTable ( LIST_0 ).initDefaults( pcSliceHeader->getLumaLog2WeightDenom(), pcSliceHeader->getChromaLog2WeightDenom() ) );
        RNOK( pcSliceHeader->getPredWeightTable ( LIST_0 ).createRandomParameters() );
      }
      else
      {
        SliceHeader* pcBaseLayerSH = 0;
        RNOK( m_pcH264AVCEncoder->getBaseLayerSH( pcBaseLayerSH, pcSliceHeader->getBaseLayerId(), pcSliceHeader->getPoc() ) );
        ROF ( pcBaseLayerSH );
        RNOK( pcSliceHeader->getPredWeightTable( LIST_0 ).copy( pcBaseLayerSH->getPredWeightTable( LIST_0 ) ) );
      }
    }
  }
  else if( pcSliceHeader->isInterB() )
  {
    RNOK( pcSliceHeader->getPredWeightTable( LIST_0 ).uninit() );
    RNOK( pcSliceHeader->getPredWeightTable( LIST_1 ).uninit() );
    RNOK( pcSliceHeader->getPredWeightTable( LIST_0 ).init( 64 ) );
    RNOK( pcSliceHeader->getPredWeightTable( LIST_1 ).init( 64 ) );

    if( pcSliceHeader->getPPS().getWeightedBiPredIdc() == 1 )
    {
      pcSliceHeader->setBasePredWeightTableFlag( false );
#if INFER_ELAYER_PRED_WEIGHTS
      if( pcSliceHeader->getBaseLayerId() != MSYS_UINT_MAX )
      {
        pcSliceHeader->setBasePredWeightTableFlag( true );
      }
#endif
      if( ! pcSliceHeader->getBasePredWeightTableFlag() )
      {
        pcSliceHeader->setLumaLog2WeightDenom   ( 6 );
        pcSliceHeader->setChromaLog2WeightDenom ( 6 );
        RNOK( pcSliceHeader->getPredWeightTable ( LIST_0 ).initDefaults( pcSliceHeader->getLumaLog2WeightDenom(), pcSliceHeader->getChromaLog2WeightDenom() ) );
        RNOK( pcSliceHeader->getPredWeightTable ( LIST_1 ).initDefaults( pcSliceHeader->getLumaLog2WeightDenom(), pcSliceHeader->getChromaLog2WeightDenom() ) );
        RNOK( pcSliceHeader->getPredWeightTable ( LIST_0 ).createRandomParameters() );
        RNOK( pcSliceHeader->getPredWeightTable ( LIST_1 ).createRandomParameters() );
      }
      else
      {
        SliceHeader* pcBaseLayerSH = 0;
        RNOK( m_pcH264AVCEncoder->getBaseLayerSH( pcBaseLayerSH, pcSliceHeader->getBaseLayerId(), pcSliceHeader->getPoc() ) );
        ROF ( pcBaseLayerSH );
        RNOK( pcSliceHeader->getPredWeightTable( LIST_0 ).copy( pcBaseLayerSH->getPredWeightTable( LIST_0 ) ) );
        RNOK( pcSliceHeader->getPredWeightTable( LIST_1 ).copy( pcBaseLayerSH->getPredWeightTable( LIST_1 ) ) );
      }
    }
  }

    // TMM_ESS {
    if (m_uiLayerId > 0)
    {
        int index = pcSliceHeader->getPoc();
        const PictureParameters* pc = m_pcResizeParameters->getCurrentPictureParameters (index);
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
MCTFEncoder::xSetScalingFactorsAVC()
{
  Bool  bAdaptiveQP = m_bAdaptiveQP;
  m_bAdaptiveQP     = false;

  for( UInt uiLevel = 0; uiLevel < m_uiDecompositionStages; uiLevel++ )
  {
    RNOK( xSetScalingFactorsMCTF( uiLevel ) );
  }

  m_bAdaptiveQP     = bAdaptiveQP;
  return Err::m_nOK;
}


ErrVal
MCTFEncoder::xSetScalingFactorsMCTF( UInt uiBaseLevel )
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
    RNOK( xGetConnections( m_pacControlData[ iFrame << uiBaseLevel ], adRateL0[iFrame], adRateL1[iFrame], adRateBi[iFrame] ) );
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
  if( m_bUpdate == 0 && m_uiLayerId == 0 && m_uiFrameWidthInMb <= 11 )
#else
  if( m_bUpdate == 0 && m_uiClosedLoopMode == 0 )
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
    for( Int iFrameId = Int( uiFrame - 1 ); iFrameId >= 0 && uiList0Size; iFrameId -= 2 )
    {
      IntFrame* pcFrame = m_papcFrame[ iFrameId << uiBaseLevel ];

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
      uiList0Size--;
    }
    ROT( uiList0Size );
  }

  //===== list 1 =====
  {
    for( Int iFrameId = Int( uiFrame + 1 ); iFrameId <= (Int)( m_uiGOPSize >> uiBaseLevel ) && uiList1Size; iFrameId += 2 )
    {
      IntFrame* pcFrame = m_papcFrame[ iFrameId << uiBaseLevel ];

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
      uiList1Size--;
    }
    ROT( uiList1Size );
  }

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
    for( Int iFrameId = Int( uiFrame - 1 ); iFrameId >= 0 && uiList0Size; iFrameId -= 2 )
    {
      IntFrame* pcFrame = m_papcBQFrame[ iFrameId << uiBaseLevel ];
      RNOK( xFillAndExtendFrame   ( pcFrame ) );

      RNOK( rcRefList0.add( pcFrame ) );
      uiList0Size--;
    }
    ROT( uiList0Size );
  }

  //===== list 1 =====
  {
    for( Int iFrameId = Int( uiFrame + 1 ); iFrameId <= (Int)( m_uiGOPSize >> uiBaseLevel ) && uiList1Size; iFrameId += 2 )
    {
      IntFrame* pcFrame = m_papcBQFrame[ iFrameId << uiBaseLevel ];
      RNOK( xFillAndExtendFrame   ( pcFrame ) );

      RNOK( rcRefList1.add( pcFrame ) );
      uiList1Size--;
    }
    ROT( uiList1Size );
  }

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
    for( Int iFrameId = Int( uiFrame - 1 ); iFrameId >= 0 && uiList0Size; iFrameId -= 2 )
    {
      IntFrame* pcFrame = m_papcCLRecFrame[ iFrameId << uiBaseLevel ];

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
      uiList0Size--;
    }
    ROT( uiList0Size );
  }

  //===== list 1 =====
  {
    for( Int iFrameId = Int( uiFrame + 1 ); iFrameId <= (Int)( m_uiGOPSize >> uiBaseLevel ) && uiList1Size; iFrameId += 2 )
    {
      IntFrame* pcFrame = m_papcCLRecFrame[ iFrameId << uiBaseLevel ];

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
      uiList1Size--;
    }
    ROT( uiList1Size );
  }

  return Err::m_nOK;
}


ErrVal  
MCTFEncoder::xGetUpdateLists( RefFrameList& rcRefList0,
                              RefFrameList& rcRefList1,
                              CtrlDataList& rcCtrlList0,
                              CtrlDataList& rcCtrlList1,
                              UInt          uiBaseLevel,
                              UInt          uiFrame )
{
  rcRefList0  .reset();
  rcRefList1  .reset();
  rcCtrlList0 .reset();
  rcCtrlList1 .reset();

  UInt          uiFrameIdInGOP  = ( uiFrame << uiBaseLevel );
  SliceHeader*  pcSliceHeader   = m_pacControlData[uiFrameIdInGOP].getSliceHeader();
  UInt          uiUpdateLevel   = m_uiDecompositionStages - uiBaseLevel - 1;
  UInt          uiList0Size     = pcSliceHeader->getNumRefIdxUpdate( uiUpdateLevel, LIST_0 );
  UInt          uiList1Size     = pcSliceHeader->getNumRefIdxUpdate( uiUpdateLevel, LIST_1 );

  //===== list 0 =====
  {
    for( Int iFrameId = Int( uiFrame - 1 ); iFrameId >= 0 && uiList0Size; iFrameId -= 2 )
    {
      IntFrame*     pcFrame       =  m_papcResidual   [ iFrameId << uiBaseLevel ];
      ControlData*  pcControlData = &m_pacControlData [ iFrameId << uiBaseLevel ];

      if( ! pcFrame->isExtended() )
      {
        RNOK( xFillAndExtendFrame( pcFrame ) );
      }

      RNOK( rcRefList0  .add( pcFrame ) );
      RNOK( rcCtrlList0 .add( pcControlData ) );
      uiList0Size--;
    }
    ROT( uiList0Size );
  }

  //===== list 1 =====
  {
    for( Int iFrameId = Int( uiFrame + 1 ); iFrameId <= (Int)( m_uiGOPSize >> uiBaseLevel ) && uiList1Size; iFrameId += 2 )
    {
      IntFrame*     pcFrame       =  m_papcResidual   [ iFrameId << uiBaseLevel ];
      ControlData*  pcControlData = &m_pacControlData [ iFrameId << uiBaseLevel ];

      if( ! pcFrame->isExtended() )
      {
        RNOK( xFillAndExtendFrame( pcFrame ) );
      }

      RNOK( rcRefList1  .add( pcFrame ) );
      RNOK( rcCtrlList1 .add( pcControlData ) );
      uiList1Size--;
    }
    ROT( uiList1Size );
  }

  return Err::m_nOK;
}







ErrVal
MCTFEncoder::xInitBaseLayerData( ControlData& rcControlData, 
                                 UInt          uiBaseLevel, //TMM_ESS
                                 UInt          uiFrame,     //TMM_ESS
                                 Bool bMotion )
{
  //===== init =====
  rcControlData.setBaseLayerRec ( 0 );
  rcControlData.setBaseLayerSbb ( 0 );
  rcControlData.setBaseLayerCtrl( 0 );

  
  IntFrame*     pcBaseFrame         = 0;
  IntFrame*     pcBaseResidual      = 0;
  MbDataCtrl*   pcBaseDataCtrl      = 0;
  Bool          bConstrainedIPredBL = false;
  Bool          bForCopyOnly        = false;
  Bool          bBaseDataAvailable  = false;

  if( rcControlData.getBaseLayerIdMotion() != MSYS_UINT_MAX )
  {
    RNOK( m_pcH264AVCEncoder->getBaseLayerData( pcBaseFrame,
                                                pcBaseResidual,
                                                pcBaseDataCtrl,
                                                bConstrainedIPredBL,
                                                bForCopyOnly,
                                                rcControlData.getSpatialScalabilityType (),
                                                rcControlData.getBaseLayerIdMotion      (),
                                                rcControlData.getSliceHeader()->getPoc  (), 
                                                bMotion ) );
    bBaseDataAvailable = pcBaseFrame && pcBaseResidual && pcBaseDataCtrl;
  }


  //===== motion data =====
  if( pcBaseDataCtrl )
  {
    RNOK( m_pcBaseLayerCtrl->initSlice( *rcControlData.getSliceHeader(), PRE_PROCESS, false, NULL ) );
    // TMM_ESS {
    if (m_pcResizeParameters->m_iSpatialScalabilityType == SST_RATIO_1)
    {
      if( bForCopyOnly )    
      {
        RNOK( m_pcBaseLayerCtrl->copyMotion  ( *pcBaseDataCtrl ) );
      }
      else
      {
        RNOK( m_pcBaseLayerCtrl->copyMotionBL( *pcBaseDataCtrl, m_pcResizeParameters ) );  
      }
    }
    else
    {
      ROT( bForCopyOnly );

      if( m_pcResizeParameters->m_iExtendedSpatialScalability == ESS_PICT ) 
      {
        RNOK( xFillPredictionLists_ESS( uiBaseLevel, uiFrame) );
      }

      RNOK( m_pcBaseLayerCtrl->upsampleMotion( *pcBaseDataCtrl, m_pcResizeParameters) );
    }
    // TMM_ESS }

    rcControlData.getMbDataCtrl()->copyBaseResidualAvailFlags( *m_pcBaseLayerCtrl );
    rcControlData.setBaseLayerCtrl( m_pcBaseLayerCtrl );

    rcControlData.getSliceHeader()->setBaseLayerUsesConstrainedIntraPred( bConstrainedIPredBL );
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
    RNOK( m_pcBaseLayerFrame->copy( pcBaseFrame ) );
    // TMM_ESS 
    m_pcBaseLayerFrame->upsample(m_cDownConvert, m_pcResizeParameters, true);
        
    rcControlData.setBaseLayerRec( m_pcBaseLayerFrame );
  }


  return Err::m_nOK;
}



ErrVal
MCTFEncoder::xInitControlDataMotion( UInt uiBaseLevel,
                                     UInt uiFrame,
                                     Bool bMotionEstimation )
{
  UInt            uiFrameIdInGOP    = uiFrame << uiBaseLevel;
  ControlData&    rcControlData     = m_pacControlData[uiFrameIdInGOP];
  SliceHeader*    pcSliceHeader     = rcControlData.getSliceHeader  ();
  MbDataCtrl*     pcMbDataCtrl      = rcControlData.getMbDataCtrl   ();
  MbDataCtrl*     pcMbDataCtrlL1    = xGetMbDataCtrlL1( *pcSliceHeader, uiFrameIdInGOP );

  SliceType       eSliceType        = pcSliceHeader->getSliceType   ();
  Double          dScalFactor       = rcControlData.getScalingFactor();
  if( ! m_bH264AVCCompatible && ! m_uiClosedLoopMode )
  {
    dScalFactor *= ( eSliceType == B_SLICE ? FACTOR_53_HP : FACTOR_22_HP );
  }
  Double          dQpPredData       = m_adBaseQpLambdaMotion[ uiBaseLevel ] - 6.0 * log10( dScalFactor ) / log10( 2.0 );
  Double          dLambda           = 0.85 * pow( 2.0, min( 52.0, dQpPredData ) / 3.0 - 4.0 );
  Int             iQp               = max( MIN_QP, min( MAX_QP, (Int)floor( dQpPredData + 0.5 ) ) );

  pcSliceHeader->setSliceHeaderQp( iQp );
  rcControlData. setLambda       ( dLambda );

  if( bMotionEstimation )
  {
    //TMM_ESS
    RNOK( xInitBaseLayerData( rcControlData,uiBaseLevel, uiFrame,true ) ); 
  
    RNOK( pcMbDataCtrl->initSlice( *pcSliceHeader, ENCODE_PROCESS, false, pcMbDataCtrlL1 ) );
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
  Double        dQP             = m_dBaseQPResidual - 6.0 * log10( dScalFactor ) / log10( 2.0 );
  Double        dLambda         = 0.85 * pow( 2.0, min( 52.0, dQP ) / 3.0 - 4.0 );
  Int           iQP             = max( MIN_QP, min( MAX_QP, (Int)floor( dQP + 0.5 ) ) );

  pcSliceHeader->setSliceHeaderQp ( iQP );
  rcControlData. setLambda        ( dLambda );

  RNOK( xInitBaseLayerData( rcControlData,uiBaseLevel,uiFrame) );//TMM_ESS

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
MCTFEncoder::xMotionEstimationStage( UInt uiBaseLevel )
{
  Bool      bMotEst = false;
  for( UInt uiFrame = 1; uiFrame <= ( m_uiGOPSize >> uiBaseLevel ); uiFrame += 2 )
  {
    if( uiFrame == 1 ) printf("              ");
    printSpaces( uiFrame == 1 ? 1 << uiBaseLevel : ( ( 1 << ( uiBaseLevel + 1 ) ) - 1 ) );
    printf("M");

    bMotEst                       = true;
    UInt          uiFrameIdInGOP  = uiFrame << uiBaseLevel;
    ControlData&  rcControlData   = m_pacControlData[uiFrameIdInGOP];
    IntFrame*     pcFrame         = m_papcFrame     [uiFrameIdInGOP];
    IntFrame*     pcIntraRecFrame = m_apcFrameTemp  [0];

    //===== get reference frame lists =====
    RefFrameList& rcRefFrameList0 = rcControlData.getPrdFrameList( LIST_0 );
    RefFrameList& rcRefFrameList1 = rcControlData.getPrdFrameList( LIST_1 );
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

    //===== motion estimation =====
    RNOK( xMotionEstimation     ( &rcRefFrameList0, &rcRefFrameList1,
                                  pcFrame, pcIntraRecFrame, rcControlData,
                                  m_bBiPredOnly, m_uiNumMaxIter, m_uiIterSearchRange, m_uiIntraMode ) );
  }
  if( bMotEst ) printf("\n");

  //===== clear half-pel buffers and buffer extension status =====
  RNOK( xClearBufferExtensions() );

  return Err::m_nOK;
}


ErrVal
MCTFEncoder::xDecompositionStage( UInt uiBaseLevel )
{
  MbEncoder*  pcMbEncoder;
  pcMbEncoder = m_pcSliceEncoder->getMbEncoder();

  //===== PREDICTION =====
  Bool      bPrediction = false;
  for( UInt uiFramePrd  = 1; uiFramePrd <= ( m_uiGOPSize >> uiBaseLevel ); uiFramePrd += 2 )
  {
    if( uiFramePrd == 1 )  printf("              ");
    printSpaces( uiFramePrd == 1 ? 1 << uiBaseLevel : ( ( 1 << ( uiBaseLevel + 1 ) ) - 1 ) );
    printf("P");

    bPrediction                   = true;
    UInt          uiFrameIdInGOP  = uiFramePrd << uiBaseLevel;
    ControlData&  rcControlData   = m_pacControlData[uiFrameIdInGOP];
    IntFrame*     pcFrame         = m_papcFrame     [uiFrameIdInGOP];
    IntFrame*     pcResidual      = m_papcResidual  [uiFrameIdInGOP];
    IntFrame*     pcMCFrame       = m_apcFrameTemp  [0];

    //--- closed-loop for base quality layer ---
    IntFrame*     pcBQFrame       = 0;
    RefFrameList  acBQRefFrameList[2];
    if( m_papcBQFrame )
    {
      pcBQFrame = m_papcBQFrame[uiFrameIdInGOP];
      RNOK( xGetBQPredictionLists ( acBQRefFrameList[0], acBQRefFrameList[1], uiBaseLevel, uiFramePrd ) );
    }

    //===== get reference frame lists =====
    RefFrameList& rcRefFrameList0 = rcControlData.getPrdFrameList( LIST_0 );
    RefFrameList& rcRefFrameList1 = rcControlData.getPrdFrameList( LIST_1 );
    if( m_papcCLRecFrame )
    {
      RNOK( xGetCLRecPredictionLists( rcRefFrameList0, rcRefFrameList1, uiBaseLevel, uiFramePrd, false ) );
    }
    else
    {
      RNOK( xGetPredictionLists     ( rcRefFrameList0, rcRefFrameList1, uiBaseLevel, uiFramePrd, false ) );
    }

    //===== set lambda and QP =====
    RNOK( xInitControlDataMotion( uiBaseLevel, uiFramePrd, false ) );

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

    //===== set residual =====
    RNOK( pcResidual->copy      ( pcFrame ) );
    RNOK( xZeroIntraMacroblocks ( pcResidual, rcControlData ) );
  }
  if( bPrediction ) printf("\n");

  //===== clear half-pel buffers and buffer extension status =====
  RNOK( xClearBufferExtensions() );



  //===== UPDATE =====
  Bool      bUpdate    = false;
  // Direct update without motion vector derivation
  for( UInt uiFrameUpd = 2; uiFrameUpd <= ( m_uiGOPSize >> uiBaseLevel ); uiFrameUpd += 2 )
  {
    bUpdate                       = true;

    UInt          uiFrameIdInGOP  = uiFrameUpd << uiBaseLevel;
    IntFrame*     pcFrame         = m_papcFrame     [uiFrameIdInGOP];

    //===== get reference lists for update =====
    RefFrameList  acRefFrameListUpd[2];
    CtrlDataList  acCtrlDataList[2];
    RNOK( xGetUpdateLists       ( acRefFrameListUpd[0], acRefFrameListUpd[1],
                                  acCtrlDataList[0], acCtrlDataList[1], uiBaseLevel, uiFrameUpd ) );

    if( uiFrameUpd == 2 ) printf("              -");
    printSpaces( ( 1 << ( uiBaseLevel + 1 ) ) - 1 );
    printf( acRefFrameListUpd[0].getActive() || acRefFrameListUpd[1].getActive() ? "U" : "-" );

    //===== list 0 motion compensation =====
    xUpdateCompensation( pcFrame, &acRefFrameListUpd[0], &acCtrlDataList[0], LIST_0 );

    //===== list 1 motion compensation =====
    xUpdateCompensation( pcFrame, &acRefFrameListUpd[1], &acCtrlDataList[1], LIST_1 );
  }

  if( bUpdate ) printf("\n");

  //===== clear half-pel buffers and buffer extension status =====
  RNOK( xClearBufferExtensions() );


  return Err::m_nOK;
}





ErrVal
MCTFEncoder::xCompositionStage( UInt uiBaseLevel, PicBufferList& rcPicBufferInputList )
{
  MbEncoder*  pcMbEncoder;
  pcMbEncoder = m_pcSliceEncoder->getMbEncoder();


  //===== PREDICTION =====
  Bool      bPrediction = false;
  for( UInt uiFramePrd  = 1; uiFramePrd <= ( m_uiGOPSize >> uiBaseLevel ); uiFramePrd += 2 )
  {
    //===== clear half-pel buffers and buffer extension status =====
    RNOK( xClearBufferExtensions() );

    if( uiFramePrd == 1 )  printf("              ");
    printSpaces( uiFramePrd == 1 ? 1 << uiBaseLevel : ( ( 1 << ( uiBaseLevel + 1 ) ) - 1 ) );
    printf("P");

    bPrediction                   = true;
    UInt          uiFrameIdInGOP  = uiFramePrd << uiBaseLevel;
    ControlData&  rcControlData   = m_pacControlData[uiFrameIdInGOP];
    IntFrame*     pcFrame         = m_papcFrame     [uiFrameIdInGOP];
    IntFrame*     pcResidual      = m_papcResidual  [uiFrameIdInGOP]; // Hanke@RWTH
    IntFrame*     pcMCFrame       = m_apcFrameTemp  [0];
		IntFrame*			pcSRFrame				= m_papcSmoothedFrame[uiFrameIdInGOP];	// JVT-R091

		//-- JVT-R091
		// obtain base-layer data
		if ( rcControlData.getBaseLayerId() != MSYS_UINT_MAX )
		{
			xInitBaseLayerData( rcControlData, uiBaseLevel, uiFramePrd, true );
		}
		//--

    //--- closed-loop coding of base quality layer ---
    IntFrame*     pcBQFrame       = 0;
    IntFrame*     pcBQResidual    = 0;
    RefFrameList  acBQRefFrameList[2];
    if( m_papcBQFrame )
    {
      pcBQFrame       = m_papcBQFrame   [uiFrameIdInGOP];
      pcBQResidual    = m_apcFrameTemp  [1];
      RNOK( xGetBQPredictionLists       ( acBQRefFrameList[0], acBQRefFrameList[1], uiBaseLevel, uiFramePrd ) );

      RNOK( pcBQResidual->copy( pcBQFrame ) );
      RNOK( xZeroIntraMacroblocks( pcBQResidual, rcControlData ) );

      //===== prediction =====
      RNOK( xMotionCompensation         ( pcMCFrame, &acBQRefFrameList[0], &acBQRefFrameList[1],
                                          rcControlData.getMbDataCtrl(), *rcControlData.getSliceHeader() ) );
      RNOK( xFixMCPrediction						( pcMCFrame, pcSRFrame, rcControlData ) );	// JVT-R091
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
      RNOK( xGetCLRecPredictionLists       ( acCLRecRefFrameList[0], acCLRecRefFrameList[1], uiBaseLevel, uiFramePrd ) );

      RNOK( pcCLRecResidual->copy( pcCLRecFrame ) );
      RNOK( xZeroIntraMacroblocks( pcCLRecResidual, rcControlData ) );

      //===== prediction =====
      RNOK( xMotionCompensation         ( pcMCFrame, &acCLRecRefFrameList[0], &acCLRecRefFrameList[1],
                                          rcControlData.getMbDataCtrl(), *rcControlData.getSliceHeader() ) );
      RNOK( xFixMCPrediction						( pcMCFrame, pcSRFrame, rcControlData ) );	// JVT-R091
			RNOK( pcCLRecFrame->inversePrediction( pcMCFrame, pcCLRecFrame ) );
    }

    //===== get reference frame lists =====
    RefFrameList& rcRefFrameList0 = rcControlData.getPrdFrameList( LIST_0 );
    RefFrameList& rcRefFrameList1 = rcControlData.getPrdFrameList( LIST_1 );
    RNOK( xGetPredictionLists         ( rcRefFrameList0, rcRefFrameList1,
																				uiBaseLevel, uiFramePrd, false ) );

    //===== prediction =====
    RNOK( xMotionCompensation         ( pcMCFrame, &rcRefFrameList0, &rcRefFrameList1,
                                        rcControlData.getMbDataCtrl(), *rcControlData.getSliceHeader() ) );
		RNOK( xFixMCPrediction						( pcMCFrame, pcSRFrame, rcControlData ) );	// JVT-R091
    RNOK( pcFrame ->inversePrediction ( pcMCFrame, pcFrame ) );
		//--

    //----- store non-deblocked signal for inter-layer prediction -----
    RNOK( m_papcSubband[uiFrameIdInGOP]->copy( pcFrame ) );

    //===== de-blocking =====
#if MULTIPLE_LOOP_DECODING
    if( m_bCompletelyDecodeLayer )
      rcControlData.activateMbDataCtrlForQpAndCbp( false );
#endif
    // Hanke@RWTH: set pointer to current residual frame
    m_pcLoopFilter->setHighpassFramePointer( pcResidual ); 
    RNOK( m_pcLoopFilter->process     ( *rcControlData.getSliceHeader(),
                                         pcFrame,
                                         rcControlData.getMbDataCtrl (),
                                         rcControlData.getMbDataCtrl (),
                                         m_uiFrameWidthInMb,
                                         &rcRefFrameList0,
                                         &rcRefFrameList1,
                                         rcControlData.getSpatialScalability()) );  // SSUN@SHARP

#if MULTIPLE_LOOP_DECODING
    if( m_bCompletelyDecodeLayer )
      rcControlData.activateMbDataCtrlForQpAndCbp( true );
#endif

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
                                     rcControlData.getSpatialScalability()) );  // SSUN@SHARP
      RNOK( rcControlData.switchBQLayerQpAndCbp() );
    }
  }
  if( bPrediction ) printf("\n");

  //===== clear half-pel buffers and buffer extension status =====
  RNOK( xClearBufferExtensions() );

  //{{Adaptive GOP structure
  // --ETRI & KHU
  if (!m_uiUseAGS) 
  {
    //}}Adaptive GOP structure
    RNOK( xCalculateAndAddPSNR( rcPicBufferInputList, m_uiDecompositionStages - uiBaseLevel, uiBaseLevel == m_uiNotCodedMCTFStages ) );
    //{{Adaptive GOP structure
    // --ETRI & KHU
  }
  else 
  {
    RNOK( xCalculateAndAddPSNR( rcPicBufferInputList, (UInt)( log10((double)m_uiMaxGOPSize)/log10(2.0) ) - uiBaseLevel, uiBaseLevel == m_uiNotCodedMCTFStages ) );
  }
  //}}Adaptive GOP structure


  return Err::m_nOK;
}

//-- JVT-R091
ErrVal
MCTFEncoder::xFixMCPrediction	( IntFrame*    pcMCFrame,
																IntFrame*		 pcBQFrame,
																ControlData& rcCtrlData )
{
  MbDataCtrl*       pcMbDataCtrl		= rcCtrlData.getMbDataCtrl					();
  SliceHeader*      pcSliceHeader		= rcCtrlData.getSliceHeader					();
  IntYuvPicBuffer*  pcPicBuffer			= pcMCFrame->getFullPelYuvBuffer		();
	IntYuvPicBuffer*	pcBaseResBuffer	= rcCtrlData.getBaseLayerSbb				()->getFullPelYuvBuffer();
	IntYuvPicBuffer*	pcBQPicBuffer		= pcBQFrame->getFullPelYuvBuffer		();

  RNOK( pcMbDataCtrl->initSlice( *pcSliceHeader, PRE_PROCESS, false, NULL ) );

  IntYuvMbBuffer cBaseResMbBuffer, cPrdMbBuffer, cBQMbBuffer;

  for( UInt uiMbIndex = 0; uiMbIndex < m_uiMbNumber; uiMbIndex++ )
  {
    UInt          uiMbY           = uiMbIndex / m_uiFrameWidthInMb;
    UInt          uiMbX           = uiMbIndex % m_uiFrameWidthInMb;
    MbDataAccess* pcMbDataAccess  = 0;

    RNOK( pcMbDataCtrl            ->initMb( pcMbDataAccess, uiMbY, uiMbX ) );
    RNOK( m_pcYuvFullPelBufferCtrl->initMb(                 uiMbY, uiMbX ) );

    if( pcMbDataAccess->getMbData().getSmoothedRefFlag() )
    {
			// load P & Rb
			cPrdMbBuffer		.loadBuffer	( pcPicBuffer				);
			cBaseResMbBuffer.loadBuffer	( pcBaseResBuffer		);

			// store current MB of base-quality frame
			cBQMbBuffer		  .loadBuffer	( pcBQPicBuffer			);

			// compute prediction: S(P+Rb)-Rb
			cPrdMbBuffer		.add				( cBaseResMbBuffer	);
			pcBQPicBuffer->loadBuffer		( &cPrdMbBuffer			);

			pcBQPicBuffer->smoothMbInside();
			if ( pcMbDataAccess->isAboveMbExisting() )
			{
				pcBQPicBuffer->smoothMbTop();
			}
			if ( pcMbDataAccess->isLeftMbExisting() )
			{
				pcBQPicBuffer->smoothMbLeft();
			}

			cPrdMbBuffer.loadBuffer			( pcBQPicBuffer			);
			cPrdMbBuffer.subtract				( cBaseResMbBuffer	);

			// restore current MB of base-quality frame
			pcBQPicBuffer->loadBuffer		( &cBQMbBuffer			);

			// store prediction
			pcPicBuffer->loadBuffer			( &cPrdMbBuffer			);
    }
  }

  return Err::m_nOK;
}

ErrVal
MCTFEncoder::xFixOrgResidual( IntFrame*			pcFrame,
															IntFrame*			pcOrgPred,
															IntFrame*			pcResidual,
															IntFrame*			pcSRFrame,
                              ControlData&	rcCtrlData )
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
			cRecMbBuffer.loadBuffer( pcSRFrame	->getFullPelYuvBuffer()	);
			cResMbBuffer.loadBuffer( pcResidual	->getFullPelYuvBuffer()	);
			cPrdMbBuffer.loadBuffer( pcOrgPred	->getFullPelYuvBuffer()	);

			// O
			cMbBuffer		.loadBuffer( pcPicBuffer												);
			cMbBuffer		.add			 ( cPrdMbBuffer												);
			
			// O-(Rec-Res) = O-P = R
			cMbBuffer		.subtract	 ( cRecMbBuffer												);
			cMbBuffer		.add			 ( cResMbBuffer												);

			// store to pcFrame
      pcPicBuffer->loadBuffer( &cMbBuffer													);
    }
  }

  return Err::m_nOK;
}
//--


ErrVal
MCTFEncoder::xEncodeLowPassPictures( AccessUnitList&  rcAccessUnitList )
{
  for( UInt uiFrame = 0; uiFrame <= 1; uiFrame++ )
  {
    UInt uiFrameIdInGOP = uiFrame << m_uiDecompositionStages;
    if(  uiFrameIdInGOP > m_uiGOPSize )
    {
      continue;
    }


    if( uiFrame == 0 && m_uiGOPNumber )
    {
      //====== don't code first anchor picture if it was coded within the last GOP =====
      RNOK( m_papcFrame[ uiFrameIdInGOP ] ->copyAll( m_pcAnchorFrameReconstructed  ) );
      continue;
    }

    UInt          uiBits          = 0;
    ControlData&  rcControlData   = m_pacControlData[ uiFrameIdInGOP ];
    SliceHeader*  pcSliceHeader   = rcControlData.getSliceHeader();
    MbDataCtrl*   pcMbDataCtrl    = rcControlData.getMbDataCtrl ();
    IntFrame*     pcFrame         = m_papcFrame     [ uiFrameIdInGOP ];
    IntFrame*     pcResidual      = m_papcResidual  [ uiFrameIdInGOP ];
    IntFrame*     pcPredSignal    = m_apcFrameTemp  [ 0 ];
    IntFrame*     pcBLRecFrame    = m_apcFrameTemp  [ 1 ];

    AccessUnit&             rcAccessUnit  = rcAccessUnitList.getAccessUnit  ( pcSliceHeader->getPoc() );
    ExtBinDataAccessorList& rcOutputList  = rcAccessUnit    .getNalUnitList ();

    //===== initialize =====
    RNOK( xInitControlDataLowPass ( uiFrameIdInGOP, m_uiDecompositionStages-1,uiFrame ) );

    //===== base layer encoding =====
    RNOK( pcBLRecFrame->copy      ( pcFrame ) );

    RNOK( xEncodeLowPassSignal    ( rcOutputList,
                                    rcControlData,
                                    pcBLRecFrame,
                                    pcResidual,
                                    pcPredSignal,
                                    uiBits ) );
    //{{Adaptive GOP structure
    // --ETRI & KHU
    if (!m_uiUseAGS) 
    {
      //}}Adaptive GOP structure
      m_auiCurrGOPBitsBase[ pcSliceHeader->getTemporalLevel() ] += uiBits;
      m_auiNumFramesCoded [ pcSliceHeader->getTemporalLevel() ] ++;
      m_auiCurrGOPBits	  [ m_uiScalableLayerId ] += uiBits;
      //{{Adaptive GOP structure
      // --ETRI & KHU
    }
    else
    {
      m_auiCurrGOPBitsBase[ pcSliceHeader->getTemporalLevel() + ((UInt)( log10((double)m_uiMaxGOPSize)/log10(2.0) ) - m_uiDecompositionStages)] += uiBits;
      m_auiNumFramesCoded [ pcSliceHeader->getTemporalLevel() + ((UInt)( log10((double)m_uiMaxGOPSize)/log10(2.0) ) - m_uiDecompositionStages)] ++;
    }
    //}}Adaptive GOP structure

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

    //----- de-blocking -----
    m_pcLoopFilter->setHighpassFramePointer( pcResidual );
    RNOK( m_pcLoopFilter->process ( *pcSliceHeader,
                                    pcBLRecFrame,
                                    ( pcSliceHeader->isIntra() ? NULL : pcMbDataCtrl ),
                                    pcMbDataCtrl,
                                    m_uiFrameWidthInMb,
                                    &rcControlData.getPrdFrameList( LIST_0 ),
                                    &rcControlData.getPrdFrameList( LIST_1 ),
                                    rcControlData.getSpatialScalability()) );  // SSUN@SHARP

    m_uiNumLayers[0] = m_uiNumLayers[1];

    for( UInt uiLayerIdx = 0; uiLayerIdx < m_uiNumLayers[0]; uiLayerIdx ++ )
    {
      RNOK( m_aapcFGSRecon[0][uiLayerIdx]->copy( m_aapcFGSRecon[1][uiLayerIdx] ) );
    }

    RNOK( m_aapcFGSRecon[0][0]->copy( m_pcLowPassBaseReconstruction ) );

    //----- store for prediction of following low-pass pictures -----
    RNOK( m_pcLowPassBaseReconstruction ->copy( pcBLRecFrame ) );

    // at least the same as the base layer
    RNOK( rcControlData.saveMbDataQpAndCbp() );

	  //{{Adaptive GOP structure
    // --ETRI & KHU
	  if (m_uiUseAGS) {
		  if (m_uiWriteGOPMode) {
			  const YuvBufferCtrl::YuvBufferParameter& cBufferParam = m_pcYuvFullPelBufferCtrl->getBufferParameter();
			  UInt usize = (cBufferParam.getWidth ()+2*32) * (cBufferParam.getHeight()+2*64) * 3/2;
			  PicBuffer *reconst_picbuf = new PicBuffer(new UChar[ usize ]);
			  IntFrame*     pcOrgResidual = new IntFrame( *m_pcYuvFullPelBufferCtrl, *m_pcYuvFullPelBufferCtrl );
			  pcOrgResidual->init(false);
			  pcOrgResidual->setZero();
			  pcOrgResidual->subtract( pcFrame, pcBLRecFrame );

			  xAddBaseLayerResidual( rcControlData, pcOrgResidual, true );
			  pcOrgResidual->store(reconst_picbuf);

			  Pel*  pPelRec_rec   = reconst_picbuf ->getBuffer() + cBufferParam.getMbLum();
			  Int   iStride   = cBufferParam.getStride();
			  Int   iWidth    = cBufferParam.getWidth ();
			  Int   iHeight   = cBufferParam.getHeight();

			  UInt uiSSETemp = 0;

			  for(int y = 0; y < iHeight; y++ )
			  {
				  for(int x = 0; x < iWidth; x++ )
				  {
					  Int iDiff = (Int)pPelRec_rec[x];
					  uiSSETemp += iDiff * iDiff;
				  }
				  pPelRec_rec  += iStride;
			  }
			  m_dMSETemp += (double)uiSSETemp/(double)(iHeight*iWidth);

        delete reconst_picbuf;
			  delete pcOrgResidual;
		  }
	  }
	  //}}Adaptive GOP structure


    //--- closed-loop coding of base quality layer ---
    if( m_papcBQFrame )
    {
      RNOK( m_papcBQFrame[uiFrameIdInGOP]->copy( pcBLRecFrame ) ); // save base quality layer reconstruction
    }

    //===== FGS enhancement layers =====
    if( m_dNumFGSLayers == 0.0 )
    {
      RNOK( pcFrame->copy   ( pcBLRecFrame ) );

      if( m_papcCLRecFrame )
      {
        RNOK( m_papcCLRecFrame[uiFrameIdInGOP]->copy( pcFrame ) );
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

      RNOK( xEncodeFGSLayer ( rcOutputList,
                              rcControlData,
                              pcFrame,
                              pcResidual,
                              pcPredSignal,
                              pcBLRecFrame,
                              m_papcSubband[uiFrameIdInGOP],
                              m_papcCLRecFrame ? m_papcCLRecFrame[uiFrameIdInGOP] : 0,
                              uiFrameIdInGOP,
                              uiBits ) );
      //{{Adaptive GOP structure
      // --ETRI & KHU
      if (!m_uiUseAGS) 
      {
      //}}Adaptive GOP structure

        m_auiCurrGOPBitsFGS[ pcSliceHeader->getTemporalLevel() ] += uiBits;
    
      //{{Adaptive GOP structure
      // --ETRI & KHU
      }
      else
      {
        m_auiCurrGOPBitsFGS[ pcSliceHeader->getTemporalLevel() + ((UInt)( log10((double)m_uiMaxGOPSize)/log10(2.0) ) - m_uiDecompositionStages)] += uiBits;
      }
      //}}Adaptive GOP structure
      

      //----- store for inter-layer prediction (non-deblocked version) -----

      //----- de-blocking -----
#if MULTIPLE_LOOP_DECODING
      if( m_bCompletelyDecodeLayer )
        rcControlData.activateMbDataCtrlForQpAndCbp( false );
#endif
      m_pcLoopFilter->setHighpassFramePointer( pcResidual );
      RNOK( m_pcLoopFilter->process ( *pcSliceHeader,
                                      pcFrame,
                                      ( pcSliceHeader->isIntra() ? NULL : pcMbDataCtrl ),
                                      pcMbDataCtrl,
                                      m_uiFrameWidthInMb,
                                      &rcControlData.getPrdFrameList( LIST_0 ),
                                      &rcControlData.getPrdFrameList( LIST_1 ),
                                      rcControlData.getSpatialScalability()) );  // SSUN@SHARP
      RNOK( m_aapcFGSRecon[1][m_uiNumLayers[1] - 1]->copy( pcFrame ) );
#if MULTIPLE_LOOP_DECODING
      if( m_bCompletelyDecodeLayer )
        rcControlData.activateMbDataCtrlForQpAndCbp( true );
#endif

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
                                        rcControlData.getSpatialScalability()) );  // SSUN@SHARP
      }
    }


    if( m_uiFGSMode == 1 && m_pFGSFile && !m_bUseDiscardableUnit ) //FIX_FRAG_CAVLC)
    {
      fprintf( m_pFGSFile, "\n" );
    }

   }

	m_uiScalableLayerId += (UInt)(m_dNumFGSLayers+1);
  return Err::m_nOK;
}





ErrVal
MCTFEncoder::xEncodeHighPassPictures( AccessUnitList&   rcAccessUnitList,
                                      UInt              uiBaseLevel )
{
  ROFRS( m_uiNotCodedMCTFStages <= uiBaseLevel, Err::m_nOK );  // does not belong to this layer


  for( UInt uiFrame = 1; uiFrame <= ( m_uiGOPSize >> uiBaseLevel ); uiFrame += 2 )
  {
    UInt          uiFrameIdInGOP  = uiFrame << uiBaseLevel;
    UInt          uiBits          = 0;
    UInt          uiBitsRes       = 0;
    IntFrame*     pcFrame         = m_papcFrame     [uiFrameIdInGOP];
    IntFrame*     pcBQFrame       = ( m_papcBQFrame ? m_papcBQFrame[uiFrameIdInGOP] : 0 );
    IntFrame*     pcResidual      = m_papcResidual  [uiFrameIdInGOP];
    IntFrame*     pcPredSignal    = m_apcFrameTemp  [0];
    IntFrame*     pcBLRecFrame    = m_apcFrameTemp  [1];
		IntFrame*			pcSRFrame				= m_papcSmoothedFrame[uiFrameIdInGOP]; // JVT-R091
    ControlData&  rcControlData   = m_pacControlData[uiFrameIdInGOP];
    SliceHeader*  pcSliceHeader   = rcControlData.getSliceHeader();
		IntFrame*			pcOrgPred				= m_apcFrameTemp	[2];	// JVT-R091

    AccessUnit&             rcAccessUnit  = rcAccessUnitList.getAccessUnit  ( pcSliceHeader->getPoc() );
    ExtBinDataAccessorList& rcOutputList  = rcAccessUnit    .getNalUnitList ();
    
    RNOK( xInitControlDataHighPass( uiFrameIdInGOP,uiBaseLevel,uiFrame ) );

    //===== base layer encoding =====
    //--- closed-loop coding of base quality layer ---
    if( pcBQFrame )
    {
			RNOK( pcSRFrame->subtract			( pcSRFrame, pcBQFrame ) ); // JVT-R091
			RNOK( pcOrgPred->copy					( pcSRFrame						 ) );	// JVT-R091
      RNOK( xEncodeHighPassSignal   ( rcOutputList,
                                      rcControlData,
                                      pcBQFrame,
                                      pcResidual,
                                      pcPredSignal,
																			pcSRFrame, // JVT-R091
                                      uiBits, uiBitsRes ) );
      RNOK( rcControlData.storeBQLayerQpAndCbp() );
    }
    else
    {
      RNOK( pcBLRecFrame->copy      ( pcFrame ) );
			RNOK( pcSRFrame		->subtract	( pcSRFrame, pcBLRecFrame ) );	// JVT-R091
			RNOK( pcOrgPred->copy					( pcSRFrame								) );	// JVT-R091
      RNOK( xEncodeHighPassSignal   ( rcOutputList,
                                      rcControlData,
                                      pcBLRecFrame,
                                      pcResidual,
                                      pcPredSignal,
																			pcSRFrame, // JVT-R091
                                      uiBits, uiBitsRes ) );
    }

    //{{Adaptive GOP structure
    // --ETRI & KHU
    if (!m_uiUseAGS) 
    {
      //}}Adaptive GOP structure
      m_auiCurrGOPBitsBase[ pcSliceHeader->getTemporalLevel() ] += uiBits;
      m_auiNumFramesCoded [ pcSliceHeader->getTemporalLevel() ] ++;
      m_auiCurrGOPBits		[ m_uiScalableLayerId ] += uiBits;
      //{{Adaptive GOP structure
      // --ETRI & KHU
    }
    else
    {
      m_auiCurrGOPBitsBase[ pcSliceHeader->getTemporalLevel() +((UInt)( log10((double)m_uiMaxGOPSize)/log10(2.0) ) - m_uiDecompositionStages)] += uiBits;
      m_auiNumFramesCoded [ pcSliceHeader->getTemporalLevel() +((UInt)( log10((double)m_uiMaxGOPSize)/log10(2.0) ) - m_uiDecompositionStages)] ++;
    }
    //}}Adaptive GOP structure
    
    //===== save FGS info =====
    if( m_uiFGSMode == 1 && m_pFGSFile && !m_bUseDiscardableUnit ) //FIX_FRAG_CAVLC)
    {
      fprintf( m_pFGSFile, "%d", uiBits + m_uiNotYetConsideredBaseLayerBits );
      m_uiNotYetConsideredBaseLayerBits = 0;
    }

    // at least the same as the base layer
    RNOK( rcControlData.saveMbDataQpAndCbp() );

	//{{Adaptive GOP structure
  // --ETRI & KHU
	if (m_uiUseAGS) 
	{
		if (m_uiWriteGOPMode) 
		{
			const YuvBufferCtrl::YuvBufferParameter& cBufferParam = m_pcYuvFullPelBufferCtrl->getBufferParameter();
			UInt usize = (cBufferParam.getWidth ()+2*32) * (cBufferParam.getHeight()+2*64) * 3/2;
			PicBuffer *reconst_picbuf = new PicBuffer(new UChar[ usize ]);
			IntFrame*     pcOrgResidual = new IntFrame( *m_pcYuvFullPelBufferCtrl, *m_pcYuvFullPelBufferCtrl );
			pcOrgResidual->init(false);
			pcOrgResidual->setZero();
			pcOrgResidual->subtract( pcFrame, pcBLRecFrame );

			xAddBaseLayerResidual( rcControlData, pcOrgResidual, true );
			pcOrgResidual->store(reconst_picbuf);

			Pel*  pPelRec_rec   = reconst_picbuf ->getBuffer() + cBufferParam.getMbLum();
			Int   iStride   = cBufferParam.getStride();
			Int   iWidth    = cBufferParam.getWidth ();
			Int   iHeight   = cBufferParam.getHeight();

			UInt uiSSETemp = 0;
			for(int y = 0; y < iHeight; y++ )
			{
				for(int x = 0; x < iWidth; x++ )
				{
					Int iDiff = (Int)pPelRec_rec[x];
					uiSSETemp += iDiff * iDiff;
				}
				pPelRec_rec  += iStride;
			}
			m_dMSETemp += (double)uiSSETemp/(double)(iHeight*iWidth);

			delete reconst_picbuf;
			delete pcOrgResidual;
		}
	}
	//}}Adaptive GOP structure

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

      if( m_papcCLRecFrame )
      {
        RNOK( m_papcCLRecFrame[uiFrameIdInGOP]->copy( pcFrame ) );
      }
    }
    else
    {
      if( m_uiFGSMode == 2 )
      {
        uiBits += m_uiNotYetConsideredBaseLayerBits;
        m_uiNotYetConsideredBaseLayerBits = 0;
      }

			//-- JVT-R091
			// note: fix original residual wih considerations of smoothed reference
			if ( rcControlData.getSliceHeader()->getBaseLayerId() != MSYS_UINT_MAX )
			{
				xFixOrgResidual( pcFrame, pcOrgPred, pcResidual, pcSRFrame, rcControlData );
			}
			//--

      RNOK( xEncodeFGSLayer ( rcOutputList,
                              rcControlData,
                              pcFrame,
                              pcResidual,
                              pcPredSignal,
                              pcBLRecFrame,
                              m_papcSubband[uiFrameIdInGOP],
                              m_papcCLRecFrame ? m_papcCLRecFrame[uiFrameIdInGOP] : 0,
                              uiFrameIdInGOP,
                              uiBits ) );
      //{{Adaptive GOP structure
      // --ETRI & KHU
      if (!m_uiUseAGS) 
      {
      //}}Adaptive GOP structure

        m_auiCurrGOPBitsFGS[ pcSliceHeader->getTemporalLevel() ] += uiBits;    

      //{{Adaptive GOP structure
      // --ETRI & KHU
      }
      else
      {
        m_auiCurrGOPBitsFGS[ pcSliceHeader->getTemporalLevel() +((UInt)( log10((double)m_uiMaxGOPSize)/log10(2.0) ) - m_uiDecompositionStages) ] += uiBits;    
      }
      //}}Adaptive GOP structure
    }


    if( m_uiFGSMode == 1 && m_pFGSFile && !m_bUseDiscardableUnit ) //FIX_FRAG_CAVLC)
    {
      fprintf( m_pFGSFile, "\n" );
    }
  }
  m_uiScalableLayerId += (UInt)(m_dNumFGSLayers+1);
  
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
MCTFEncoder::xStoreOriginalPictures( )
{
  ROF( m_bH264AVCCompatible );

  RNOK( m_apcLowPassTmpOrg[0]->copyAll( m_papcFrame[0] ) );
  RNOK( m_apcLowPassTmpOrg[1]->copyAll( m_papcFrame[1<<m_uiDecompositionStages] ) );

  return Err::m_nOK;
}


ErrVal
MCTFEncoder::xSwitchOriginalPictures()
{
  ROF( m_bH264AVCCompatible );

  IntFrame* pcTempBuffer;

  pcTempBuffer          = m_apcLowPassTmpOrg[0];
  m_apcLowPassTmpOrg[0] = m_papcFrame       [0];
  m_papcFrame       [0] = pcTempBuffer;

  Int                X  = 1 << m_uiDecompositionStages;
  pcTempBuffer          = m_apcLowPassTmpOrg[1];
  m_apcLowPassTmpOrg[1] = m_papcFrame       [X];
  m_papcFrame       [X] = pcTempBuffer;

  return Err::m_nOK;
}



ErrVal
MCTFEncoder::xProcessClosedLoop( AccessUnitList&  rcAccessUnitList,
                                 PicBufferList&   rcPicBufferInputList,
                                 PicBufferList&   rcPicBufferOutputList,
                                 PicBufferList&   rcPicBufferUnusedList,
                                 Double           m_aaauidSeqBits[MAX_LAYERS][MAX_TEMP_LEVELS][MAX_QUALITY_LEVELS] )
{
  Int iLevel;
  g_nLayer = m_uiLayerId;
  ETRACE_LAYER(m_uiLayerId);

  //===== init group of pictures =====
  RNOK( xInitGOP( rcPicBufferInputList ) );

  
  //===== set the scaling factors =====
  RNOK( xSetScalingFactorsAVC     () );

  //===== encode low-pass pictures =====
  printf("\nLOW-PASS CODING:\n");
  RNOK( xEncodeLowPassPictures    ( rcAccessUnitList ) );

  //===== store anchor frame =====
  if( m_uiGOPSize == ( 1 << m_uiDecompositionStages ) )
  {
    RNOK( m_pcAnchorFrameReconstructed->copyAll( m_papcFrame[ m_uiGOPSize ] ) );
  }
  RNOK( xCalculateAndAddPSNR( rcPicBufferInputList, 0, m_uiDecompositionStages == m_uiNotCodedMCTFStages ) );

  //===== loop over temporal levels =====
  for( iLevel = m_uiDecompositionStages - 1; iLevel >= (Int)m_uiNotCodedMCTFStages; iLevel-- )
  {
    //===== motion estimation and decomposition =====
    printf("\nDECOMPOSITION:\n");
    RNOK( xMotionEstimationStage  ( iLevel ) );
    RNOK( xDecompositionStage     ( iLevel ) );

    //===== encoding ======
    printf("\nHIGH-PASS CODING:\n");
    RNOK( xEncodeHighPassPictures ( rcAccessUnitList, iLevel ) );

    //===== reconstruction =====
    printf("\nRECONSTRUCTION:\n");
    RNOK( xCompositionStage       ( iLevel, rcPicBufferInputList ) );
  }

  //===== finish GOP =====
  RNOK( xStoreReconstruction( rcPicBufferOutputList ) );
  RNOK( xFinishGOP          ( rcPicBufferInputList,
                              rcPicBufferOutputList,
                              rcPicBufferUnusedList,
                              m_aaauidSeqBits ) );

  return Err::m_nOK;
}



    
ErrVal
MCTFEncoder::process( AccessUnitList&   rcAccessUnitList,
                      PicBufferList&    rcPicBufferInputList,
                      PicBufferList&    rcPicBufferOutputList,
                      PicBufferList&    rcPicBufferUnusedList,
                      Double            m_aaauidSeqBits[MAX_LAYERS][MAX_TEMP_LEVELS][MAX_QUALITY_LEVELS] )
{
  if( m_uiClosedLoopMode )
  {
    RNOK( xProcessClosedLoop( rcAccessUnitList,
                              rcPicBufferInputList,
                              rcPicBufferOutputList,
                              rcPicBufferUnusedList,
                              m_aaauidSeqBits ) );
    return Err::m_nOK;
  }

  
  Int iLevel;
  g_nLayer = m_uiLayerId;
  ETRACE_LAYER(m_uiLayerId);

  //===== init group of pictures =====
  RNOK( xInitGOP( rcPicBufferInputList ) );

  
  //===== DECOMPOSITION AND ENCODING =====
  if( ! m_bH264AVCCompatible )
  {
    printf("\nDECOMPOSITION:\n");

    //===== MCTF decomposition =====
    for( iLevel = 0; iLevel < (Int)m_uiDecompositionStages; iLevel++ )
    {
      RNOK( xMotionEstimationStage  ( iLevel ) );
      RNOK( xDecompositionStage     ( iLevel ) );
      RNOK( xSetScalingFactorsMCTF  ( iLevel ) );
    }

    printf("\nCODING:\n");

    //===== encode low-pass pictures =====
    RNOK( xEncodeLowPassPictures    ( rcAccessUnitList ) );
  }
  else
  {
    //===== set all scaling factors =====
    RNOK( xSetScalingFactorsAVC     () );

    //===== encode low-pass pictures =====
    RNOK( xStoreOriginalPictures    () );
    printf("\nLOW-PASS CODING:\n");
    RNOK( xEncodeLowPassPictures    ( rcAccessUnitList ) );
    RNOK( xSwitchOriginalPictures   () );

    printf("\nDECOMPOSITION:\n");

    //===== motion estimation =====
    for( iLevel = m_uiDecompositionStages - 1; iLevel >= (Int)m_uiNotCodedMCTFStages; iLevel-- )
    {
      RNOK( xMotionEstimationStage  ( iLevel ) );
    }

    //===== decomposition =====
    for( iLevel = m_uiNotCodedMCTFStages; iLevel < (Int)m_uiDecompositionStages; iLevel++ )
    {
      RNOK( xDecompositionStage     ( iLevel ) );
    }

    //===== restore coded pictures =====
    RNOK( xSwitchOriginalPictures   () );
    printf("\nHIGH-PASS CODING:\n");
  }
  //===== encode high-pass pictures =====
  for( iLevel = m_uiDecompositionStages; iLevel-- > 0; )
  {
    RNOK( xEncodeHighPassPictures   ( rcAccessUnitList, iLevel ) );
  }


  //===== RECONSTRUCTION =====
  {
    printf("\nRECONSTRUCTION:\n");

    RNOK( xCalculateAndAddPSNR( rcPicBufferInputList, 0, m_uiDecompositionStages == 0 ) );

    UInt   uiStage    = m_uiDecompositionStages;
    while( uiStage--  > m_uiNotCodedMCTFStages )
    {
      RNOK( xCompositionStage( uiStage, rcPicBufferInputList ) );
    }
  }

  //===== store reconstructed anchor frame =====
  if( m_uiGOPSize == ( 1 << m_uiDecompositionStages ) )
  {
    RNOK( m_pcAnchorFrameReconstructed->copyAll( m_papcFrame[ m_uiGOPSize ] ) );
  }

  //===== finish GOP =====
  RNOK( xStoreReconstruction( rcPicBufferOutputList ) );
  RNOK( xFinishGOP          ( rcPicBufferInputList,
                              rcPicBufferOutputList,
                              rcPicBufferUnusedList,
                              m_aaauidSeqBits ) );
  
  return Err::m_nOK;
}




//{{Adaptive GOP structure
// --ETRI & KHU
ErrVal
MCTFEncoder::process_ags ( AccessUnitList&   rcAccessUnitList,
                      PicBufferList&    rcPicBufferInputList,
                      PicBufferList&    rcPicBufferOutputList,
                      PicBufferList&    rcPicBufferUnusedList,
                      Double            m_aaauidSeqBits[MAX_LAYERS][MAX_TEMP_LEVELS][MAX_QUALITY_LEVELS] )
{
	int i, j;
	
	UInt	GN = m_uiDecompositionStages;
	UInt	first = (m_bFirstGOPCoded == 0);
	UInt	tmp_first = first;

	UInt uiStage;
	
  AccessUnitList rcAccessUnitList_tmp;
  rcAccessUnitList_tmp.clear();
	PicBufferList	rcPicBufferOutputList_gop;
	rcPicBufferOutputList_gop.clear();
	PicBufferList	rcPicBufferInputList_save;
	rcPicBufferInputList_save.clear();
	rcPicBufferInputList_save += rcPicBufferInputList;
	PicBufferList	rcPicBufferOutputList_save;
	rcPicBufferOutputList_save.clear();
	rcPicBufferOutputList_save += rcPicBufferOutputList;
	PicBufferList	rcPicBufferInputList_temp;
	rcPicBufferInputList_temp.clear();
	rcPicBufferInputList_temp += rcPicBufferInputList;
	PicBufferList	rcPicBufferOutputList_temp;
	rcPicBufferOutputList_temp.clear();
	rcPicBufferOutputList_temp += rcPicBufferOutputList;
	m_puiGOPMode = new UInt[(1<<GN)/2];
	for (i = 0; i < (1<<GN)/2; i++)
		m_puiGOPMode[i] = 0;

	if (m_uiWriteGOPMode) // for Mode Decision
  {
		UInt                          auiNumFramesCoded [MAX_DSTAGES+1];
		UInt                          auiCurrGOPBitsBase[MAX_DSTAGES+1];
		UInt                          auiCurrGOPBitsFGS [MAX_DSTAGES+1];
    UInt                          auiCurrGOPBits    [MAX_DSTAGES * MAX_QUALITY_LEVELS];
		Double                        adSeqBitsBase     [MAX_DSTAGES+1];
		Double                        adSeqBitsFGS      [MAX_DSTAGES+1];
    Double                        adSeqBits         [MAX_DSTAGES * MAX_QUALITY_LEVELS];
		Double                        adPSNRSumY        [MAX_DSTAGES+1];
		Double                        adPSNRSumU        [MAX_DSTAGES+1];
		Double                        adPSNRSumV        [MAX_DSTAGES+1];
		Bool                          abIsRef[MAX_DSTAGES];

		for( uiStage = 0; uiStage <= MAX_DSTAGES; uiStage++ ) 
    {
			auiNumFramesCoded[uiStage] = m_auiNumFramesCoded[uiStage];
			auiCurrGOPBitsBase[uiStage] = m_auiCurrGOPBitsBase[uiStage];
			auiCurrGOPBitsFGS[uiStage] = m_auiCurrGOPBitsFGS[uiStage];
			adSeqBitsBase[uiStage] = m_adSeqBitsBase[uiStage];
			adSeqBitsFGS[uiStage] = m_adSeqBitsFGS[uiStage];
			adPSNRSumY[uiStage] = m_adPSNRSumY[uiStage];
			adPSNRSumU[uiStage] = m_adPSNRSumU[uiStage];
			adPSNRSumV[uiStage] = m_adPSNRSumV[uiStage];
      if (uiStage != MAX_DSTAGES)
				abIsRef[uiStage] = m_abIsRef[uiStage];
		}
	  for( uiStage = 0; uiStage < MAX_SCALABLE_LAYERS; uiStage++ )
		{
			auiCurrGOPBits[uiStage] = m_auiCurrGOPBits[uiStage];
			adSeqBits			[uiStage] = m_adSeqBits			[uiStage];
		}
		Bool bFirstGOPCoded_save = m_bFirstGOPCoded;
		UInt uiFrameNum_save = m_uiFrameNum;
		UIntList cLPFrameNumList_save;
		cLPFrameNumList_save += m_cLPFrameNumList;
		UInt uiParameterSetBits_save = m_uiParameterSetBits;
		UInt uiFrameCounter_save = m_uiFrameCounter;
		UInt uiGOPNumber_save = m_uiGOPNumber;
		UInt uiDecompositionStages_save = m_uiDecompositionStages;
		IntFrame* pcBaseLayerRecFrame_save = new IntFrame( *m_pcYuvFullPelBufferCtrl, *m_pcYuvFullPelBufferCtrl );
		IntFrame* pcBaseLayerResidual_save = new IntFrame( *m_pcYuvFullPelBufferCtrl, *m_pcYuvFullPelBufferCtrl );
		IntFrame* pcRecAnchorFrame_save = new IntFrame( *m_pcYuvFullPelBufferCtrl, *m_pcYuvFullPelBufferCtrl );
		IntFrame* pcOrgAnchorFrame_save = new IntFrame( *m_pcYuvFullPelBufferCtrl, *m_pcYuvFullPelBufferCtrl );
		IntFrame* pcLowPassBaseReconstruction_save = new IntFrame( *m_pcYuvFullPelBufferCtrl, *m_pcYuvFullPelBufferCtrl );
		pcLowPassBaseReconstruction_save->uninit();
		pcLowPassBaseReconstruction_save->init(false);
		pcLowPassBaseReconstruction_save->setZero();
		pcLowPassBaseReconstruction_save->copyAll(m_pcLowPassBaseReconstruction);
		pcBaseLayerRecFrame_save->uninit();
		pcBaseLayerRecFrame_save->init(false);
		pcBaseLayerRecFrame_save->setZero();
		pcBaseLayerRecFrame_save->copyAll(m_pcBaseLayerFrame);
		pcBaseLayerResidual_save->uninit();
		pcBaseLayerResidual_save->init(false);
		pcBaseLayerResidual_save->setZero();
		pcBaseLayerResidual_save->copyAll(m_pcBaseLayerResidual);
		pcRecAnchorFrame_save->uninit();
		pcRecAnchorFrame_save->init(false);
		pcRecAnchorFrame_save->setZero();
		pcRecAnchorFrame_save->copyAll(m_pcAnchorFrameReconstructed);
		pcOrgAnchorFrame_save->uninit();
		pcOrgAnchorFrame_save->init(false);
		pcOrgAnchorFrame_save->setZero();
		pcOrgAnchorFrame_save->copyAll(m_pcAnchorFrameOriginal);

		UInt uiIndex;
		IntFrame*               apcFrameTemp_save    [NUM_TMP_FRAMES];
	
		for( uiIndex = 0; uiIndex < NUM_TMP_FRAMES; uiIndex++ )
		{
			apcFrameTemp_save      [uiIndex] = new IntFrame( *m_pcYuvFullPelBufferCtrl, *m_pcYuvFullPelBufferCtrl );
			apcFrameTemp_save[uiIndex]->uninit();
			apcFrameTemp_save[uiIndex]->init(false);
			apcFrameTemp_save[uiIndex]->setZero();
			apcFrameTemp_save[uiIndex]->copyAll(m_apcFrameTemp[uiIndex]);
		}
		
		FILE* pMotionInfoFile = m_pMotionInfoFile;
		FILE* pFGSFile = m_pFGSFile;
		
		m_pMotionInfoFile = fopen("tmp/tmp_motion.dat", "wb");
		m_pFGSFile = fopen("tmp/tmp_fgs.dat", "wt");
		
		Double	**mse = new Double*[6];
		
		for (i = 0; i < 6; i++) {
			mse[i] = new Double[(1<<6)-i];
			for (j = 0; j < (1<<6)-i; j++)
				mse[i][j] = 0;
		}
		
    // GOP=2^N
		first = tmp_first;
		for( uiIndex = 0; uiIndex < NUM_TMP_FRAMES; uiIndex++ )	
    {
			m_apcFrameTemp[uiIndex]->setZero();
			m_apcFrameTemp[uiIndex]->copyAll(apcFrameTemp_save[uiIndex]);
		}
		m_uiDecompositionStages = GN;
		m_uiGOPNumber = uiGOPNumber_save;
		m_bFirstGOPCoded = bFirstGOPCoded_save;
		m_uiFrameNum = uiFrameNum_save;
		for( uiStage = 0; uiStage <= MAX_DSTAGES; uiStage++ ) 
    {
			m_auiNumFramesCoded[uiStage] = auiNumFramesCoded[uiStage];
			m_auiCurrGOPBitsBase[uiStage] = auiCurrGOPBitsBase[uiStage];
			m_auiCurrGOPBitsFGS[uiStage] = auiCurrGOPBitsFGS[uiStage];
			m_adSeqBitsBase[uiStage] = adSeqBitsBase[uiStage];
			m_adSeqBitsFGS[uiStage] = adSeqBitsFGS[uiStage];
			m_adPSNRSumY[uiStage] = adPSNRSumY[uiStage];
			m_adPSNRSumU[uiStage] = adPSNRSumU[uiStage];
			m_adPSNRSumV[uiStage] = adPSNRSumV[uiStage];
			if (uiStage != MAX_DSTAGES)
				m_abIsRef[uiStage] = abIsRef[uiStage];
		}
	  for( uiStage = 0; uiStage < MAX_SCALABLE_LAYERS; uiStage++ )
		{
			m_auiCurrGOPBits	[uiStage] = auiCurrGOPBits	[uiStage];
			m_adSeqBits				[uiStage] = adSeqBits				[uiStage];
		}
		m_uiFrameCounter = uiFrameCounter_save;
		m_uiParameterSetBits = uiParameterSetBits_save;
		m_pcAnchorFrameReconstructed->setZero();
		m_pcAnchorFrameReconstructed->copyAll(pcRecAnchorFrame_save);
		m_pcAnchorFrameOriginal->setZero();
		m_pcAnchorFrameOriginal->copyAll(pcOrgAnchorFrame_save);
		m_pcBaseLayerFrame->setZero();
		m_pcBaseLayerFrame->copyAll(pcBaseLayerRecFrame_save);
		m_pcBaseLayerResidual->setZero();
		m_pcBaseLayerResidual->copyAll(pcBaseLayerResidual_save);
		m_pcLowPassBaseReconstruction->setZero();
		m_pcLowPassBaseReconstruction->copyAll(pcLowPassBaseReconstruction_save);
		m_cLPFrameNumList.clear();
		m_cLPFrameNumList += cLPFrameNumList_save;
		rcPicBufferInputList.clear();
		rcPicBufferOutputList.clear();
    rcAccessUnitList.clear();
		rcPicBufferInputList_temp.clear();
		rcPicBufferInputList_temp += rcPicBufferInputList_save;
		rcPicBufferOutputList_temp.clear();
		rcPicBufferOutputList_temp += rcPicBufferOutputList_save;

    for(i = 0; i < (Int)(1<<(GN - m_uiDecompositionStages)); i++) 
    {
			if (rcPicBufferInputList_temp.size() == 0) break;
      for(j = 0; j < (Int)(1<<m_uiDecompositionStages) + (Int)first; j++) 
      {
				if (rcPicBufferInputList_temp.size() == 0) break;
				rcPicBufferInputList.push_back(rcPicBufferInputList_temp.front());
				rcPicBufferInputList_temp.pop_front();
				rcPicBufferOutputList.push_back(rcPicBufferOutputList_temp.front());
				rcPicBufferOutputList_temp.pop_front();
			}				
			m_dMSETemp = 0;
      process (rcAccessUnitList, rcPicBufferInputList, rcPicBufferOutputList, rcPicBufferUnusedList, m_aaauidSeqBits );
      mse[4][i] = m_dMSETemp/((double)(1<<m_uiDecompositionStages)+(double)first);
      if (first == 1) 
        first = 0;
      rcAccessUnitList.clear();
			rcPicBufferInputList.clear();
			rcPicBufferOutputList.clear();
		}
		
    // GOP=2^N-1
		first = tmp_first;
		for( uiIndex = 0; uiIndex < NUM_TMP_FRAMES; uiIndex++ )	
    {
			m_apcFrameTemp[uiIndex]->setZero();
			m_apcFrameTemp[uiIndex]->copyAll(apcFrameTemp_save[uiIndex]);
		}		
		m_uiDecompositionStages = GN-1;
		m_uiGOPNumber = uiGOPNumber_save;
		m_bFirstGOPCoded = bFirstGOPCoded_save;
		m_uiFrameNum = uiFrameNum_save;
		for( uiStage = 0; uiStage <= MAX_DSTAGES; uiStage++ ) 
    {
			m_auiNumFramesCoded[uiStage] = auiNumFramesCoded[uiStage];
			m_auiCurrGOPBitsBase[uiStage] = auiCurrGOPBitsBase[uiStage];
			m_auiCurrGOPBitsFGS[uiStage] = auiCurrGOPBitsFGS[uiStage];
			m_adSeqBitsBase[uiStage] = adSeqBitsBase[uiStage];
			m_adSeqBitsFGS[uiStage] = adSeqBitsFGS[uiStage];
			m_adPSNRSumY[uiStage] = adPSNRSumY[uiStage];
			m_adPSNRSumU[uiStage] = adPSNRSumU[uiStage];
			m_adPSNRSumV[uiStage] = adPSNRSumV[uiStage];
			if (uiStage != MAX_DSTAGES)
				m_abIsRef[uiStage] = abIsRef[uiStage];
		}			
	  for( uiStage = 0; uiStage < MAX_SCALABLE_LAYERS; uiStage++ )
		{
			m_auiCurrGOPBits	[uiStage] = auiCurrGOPBits	[uiStage];
			m_adSeqBits				[uiStage] = adSeqBits				[uiStage];
		}
		m_uiFrameCounter = uiFrameCounter_save;
		m_uiParameterSetBits = uiParameterSetBits_save;
		m_pcAnchorFrameReconstructed->setZero();
		m_pcAnchorFrameReconstructed->copyAll(pcRecAnchorFrame_save);
		m_pcAnchorFrameOriginal->setZero();
		m_pcAnchorFrameOriginal->copyAll(pcOrgAnchorFrame_save);
		m_pcBaseLayerFrame->setZero();
		m_pcBaseLayerFrame->copyAll(pcBaseLayerRecFrame_save);
		m_pcBaseLayerResidual->setZero();
		m_pcBaseLayerResidual->copyAll(pcBaseLayerResidual_save);			
		m_pcLowPassBaseReconstruction->setZero();
		m_pcLowPassBaseReconstruction->copyAll(pcLowPassBaseReconstruction_save);
		m_cLPFrameNumList.clear();
		m_cLPFrameNumList += cLPFrameNumList_save;
		rcPicBufferInputList.clear();
		rcPicBufferOutputList.clear();
    rcAccessUnitList.clear();			
		rcPicBufferInputList_temp.clear();
		rcPicBufferInputList_temp += rcPicBufferInputList_save;			
		rcPicBufferOutputList_temp.clear();
		rcPicBufferOutputList_temp += rcPicBufferOutputList_save;
		
    for(i = 0; i < (Int)(1<<(GN-m_uiDecompositionStages)); i++) 
    {
			if (rcPicBufferInputList_temp.size() == 0) break;
      for(j = 0; j < (Int)(1<<m_uiDecompositionStages) + (Int)first; j++) 
      {
				if (rcPicBufferInputList_temp.size() == 0) break;
				rcPicBufferInputList.push_back(rcPicBufferInputList_temp.front());
				rcPicBufferInputList_temp.pop_front();					
				rcPicBufferOutputList.push_back(rcPicBufferOutputList_temp.front());
				rcPicBufferOutputList_temp.pop_front();
			}				
			m_dMSETemp = 0;
      process (rcAccessUnitList, rcPicBufferInputList, rcPicBufferOutputList, rcPicBufferUnusedList, m_aaauidSeqBits );
      mse[3][i] = m_dMSETemp/((double)(1<<m_uiDecompositionStages)+(double)first);
      if (first == 1) 
        first = 0;
      rcAccessUnitList.clear();
			rcPicBufferInputList.clear();
			rcPicBufferOutputList.clear();
		}

    // GOP=2^N-2
		first = tmp_first;
		for( uiIndex = 0; uiIndex < NUM_TMP_FRAMES; uiIndex++ )	
    {
			m_apcFrameTemp[uiIndex]->setZero();
			m_apcFrameTemp[uiIndex]->copyAll(apcFrameTemp_save[uiIndex]);
		}
    m_uiDecompositionStages = GN-2;
		m_uiGOPNumber = uiGOPNumber_save;
		m_bFirstGOPCoded = bFirstGOPCoded_save;
		m_uiFrameNum = uiFrameNum_save;
		for( uiStage = 0; uiStage <= MAX_DSTAGES; uiStage++ ) 
    {
			m_auiNumFramesCoded[uiStage] = auiNumFramesCoded[uiStage];
			m_auiCurrGOPBitsBase[uiStage] = auiCurrGOPBitsBase[uiStage];
			m_auiCurrGOPBitsFGS[uiStage] = auiCurrGOPBitsFGS[uiStage];
			m_adSeqBitsBase[uiStage] = adSeqBitsBase[uiStage];
			m_adSeqBitsFGS[uiStage] = adSeqBitsFGS[uiStage];
			m_adPSNRSumY[uiStage] = adPSNRSumY[uiStage];
			m_adPSNRSumU[uiStage] = adPSNRSumU[uiStage];
			m_adPSNRSumV[uiStage] = adPSNRSumV[uiStage];
			if (uiStage != MAX_DSTAGES)
				m_abIsRef[uiStage] = abIsRef[uiStage];
		}
	  for( uiStage = 0; uiStage < MAX_SCALABLE_LAYERS; uiStage++ )
		{
			m_auiCurrGOPBits	[uiStage] = auiCurrGOPBits	[uiStage];
			m_adSeqBits				[uiStage] = adSeqBits				[uiStage];
		}
		m_uiFrameCounter = uiFrameCounter_save;
		m_uiParameterSetBits = uiParameterSetBits_save;
		m_pcAnchorFrameReconstructed->setZero();
		m_pcAnchorFrameReconstructed->copyAll(pcRecAnchorFrame_save);
		m_pcAnchorFrameOriginal->setZero();
		m_pcAnchorFrameOriginal->copyAll(pcOrgAnchorFrame_save);
		m_pcBaseLayerFrame->setZero();
		m_pcBaseLayerFrame->copyAll(pcBaseLayerRecFrame_save);
		m_pcBaseLayerResidual->setZero();
		m_pcBaseLayerResidual->copyAll(pcBaseLayerResidual_save);			
		m_pcLowPassBaseReconstruction->setZero();
		m_pcLowPassBaseReconstruction->copyAll(pcLowPassBaseReconstruction_save);
		m_cLPFrameNumList.clear();
		m_cLPFrameNumList += cLPFrameNumList_save;
		rcPicBufferInputList.clear();
		rcPicBufferOutputList.clear();
    rcAccessUnitList.clear();			
		rcPicBufferInputList_temp.clear();
		rcPicBufferInputList_temp += rcPicBufferInputList_save;			
		rcPicBufferOutputList_temp.clear();
		rcPicBufferOutputList_temp += rcPicBufferOutputList_save;			

    for(i = 0; i < (Int)(1<<(GN-m_uiDecompositionStages)); i++) 
    {
			if (rcPicBufferInputList_temp.size() == 0) break;
      for(j = 0; j < (Int)(1<<m_uiDecompositionStages) + (Int)first; j++) 
      {
				if (rcPicBufferInputList_temp.size() == 0) break;
				rcPicBufferInputList.push_back(rcPicBufferInputList_temp.front());
				rcPicBufferInputList_temp.pop_front();					
				rcPicBufferOutputList.push_back(rcPicBufferOutputList_temp.front());
				rcPicBufferOutputList_temp.pop_front();
			}				
			m_dMSETemp = 0;
      process (rcAccessUnitList, rcPicBufferInputList, rcPicBufferOutputList, rcPicBufferUnusedList, m_aaauidSeqBits );
      mse[2][i] = m_dMSETemp/((double)(1<<m_uiDecompositionStages)+(double)first);
      if (first == 1) 
        first = 0;
      rcAccessUnitList.clear();
			rcPicBufferInputList.clear();
			rcPicBufferOutputList.clear();
		}

    // GOP=2^N-3
		first = tmp_first;
		for( uiIndex = 0; uiIndex < NUM_TMP_FRAMES; uiIndex++ )	
    {
			m_apcFrameTemp[uiIndex]->setZero();
			m_apcFrameTemp[uiIndex]->copyAll(apcFrameTemp_save[uiIndex]);
		}				
    m_uiDecompositionStages = GN-3;
		m_uiGOPNumber = uiGOPNumber_save;
		m_bFirstGOPCoded = bFirstGOPCoded_save;
		m_uiFrameNum = uiFrameNum_save;
		for( uiStage = 0; uiStage <= MAX_DSTAGES; uiStage++ ) 
    {
			m_auiNumFramesCoded[uiStage] = auiNumFramesCoded[uiStage];
			m_auiCurrGOPBitsBase[uiStage] = auiCurrGOPBitsBase[uiStage];
			m_auiCurrGOPBitsFGS[uiStage] = auiCurrGOPBitsFGS[uiStage];
			m_adSeqBitsBase[uiStage] = adSeqBitsBase[uiStage];
			m_adSeqBitsFGS[uiStage] = adSeqBitsFGS[uiStage];
			m_adPSNRSumY[uiStage] = adPSNRSumY[uiStage];
			m_adPSNRSumU[uiStage] = adPSNRSumU[uiStage];
			m_adPSNRSumV[uiStage] = adPSNRSumV[uiStage];
			if (uiStage != MAX_DSTAGES)
				m_abIsRef[uiStage] = abIsRef[uiStage];
		}
	  for( uiStage = 0; uiStage < MAX_SCALABLE_LAYERS; uiStage++ )
		{
			m_auiCurrGOPBits	[uiStage] = auiCurrGOPBits	[uiStage];
			m_adSeqBits				[uiStage] = adSeqBits				[uiStage];
		}
		m_uiFrameCounter = uiFrameCounter_save;
		m_uiParameterSetBits = uiParameterSetBits_save;
		m_pcAnchorFrameReconstructed->setZero();
		m_pcAnchorFrameReconstructed->copyAll(pcRecAnchorFrame_save);
		m_pcAnchorFrameOriginal->setZero();
		m_pcAnchorFrameOriginal->copyAll(pcOrgAnchorFrame_save);
		m_pcBaseLayerFrame->setZero();
		m_pcBaseLayerFrame->copyAll(pcBaseLayerRecFrame_save);
		m_pcBaseLayerResidual->setZero();
		m_pcBaseLayerResidual->copyAll(pcBaseLayerResidual_save);			
		m_pcLowPassBaseReconstruction->setZero();
		m_pcLowPassBaseReconstruction->copyAll(pcLowPassBaseReconstruction_save);
		m_cLPFrameNumList.clear();
		m_cLPFrameNumList += cLPFrameNumList_save;
		rcPicBufferInputList.clear();
		rcPicBufferOutputList.clear();
    rcAccessUnitList.clear();			
		rcPicBufferInputList_temp.clear();
		rcPicBufferInputList_temp += rcPicBufferInputList_save;			
		rcPicBufferOutputList_temp.clear();
		rcPicBufferOutputList_temp += rcPicBufferOutputList_save;			

    for(i = 0; i < (Int)(1<<(GN-m_uiDecompositionStages)); i++) 
    {
			if (rcPicBufferInputList_temp.size() == 0) break;
      for(j = 0; j < (Int)(1<<m_uiDecompositionStages) + (Int)first; j++) 
      {
				if (rcPicBufferInputList_temp.size() == 0) break;
				rcPicBufferInputList.push_back(rcPicBufferInputList_temp.front());
				rcPicBufferInputList_temp.pop_front();
				
				rcPicBufferOutputList.push_back(rcPicBufferOutputList_temp.front());
				rcPicBufferOutputList_temp.pop_front();
			}				
			m_dMSETemp = 0;
      process (rcAccessUnitList, rcPicBufferInputList, rcPicBufferOutputList, rcPicBufferUnusedList, m_aaauidSeqBits );
      mse[1][i] = m_dMSETemp/((double)(1<<m_uiDecompositionStages)+(double)first);
      if (first == 1) first = 0;
      rcAccessUnitList.clear();
			rcPicBufferInputList.clear();
			rcPicBufferOutputList.clear();
		}
		
		xSelectGOPMode(mse, m_puiGOPMode, 0, GN, 0, 0);

		fclose(m_pMotionInfoFile);
		fclose(m_pFGSFile);
		for (i = 0; i < (Int)GN+1; i++)
			delete mse[i];
		delete mse;
		m_pMotionInfoFile = pMotionInfoFile;
		m_pFGSFile = pFGSFile;
		
		for( uiIndex = 0; uiIndex < NUM_TMP_FRAMES; uiIndex++ )	
    {
			m_apcFrameTemp[uiIndex]->setZero();
			m_apcFrameTemp[uiIndex]->copyAll(apcFrameTemp_save[uiIndex]);
		}
		for( uiStage = 0; uiStage <= MAX_DSTAGES; uiStage++ ) 
    {
			m_auiNumFramesCoded[uiStage] = auiNumFramesCoded[uiStage];
			m_auiCurrGOPBitsBase[uiStage] = auiCurrGOPBitsBase[uiStage];
			m_auiCurrGOPBitsFGS[uiStage] = auiCurrGOPBitsFGS[uiStage];
			m_adSeqBitsBase[uiStage] = adSeqBitsBase[uiStage];
			m_adSeqBitsFGS[uiStage] = adSeqBitsFGS[uiStage];
			m_adPSNRSumY[uiStage] = adPSNRSumY[uiStage];
			m_adPSNRSumU[uiStage] = adPSNRSumU[uiStage];
			m_adPSNRSumV[uiStage] = adPSNRSumV[uiStage];
			if (uiStage != MAX_DSTAGES)
				m_abIsRef[uiStage] = abIsRef[uiStage];
		}
	  for( uiStage = 0; uiStage < MAX_SCALABLE_LAYERS; uiStage++ )
		{
			m_auiCurrGOPBits	[uiStage] = auiCurrGOPBits	[uiStage];
			m_adSeqBits				[uiStage] = adSeqBits				[uiStage];
		}
		m_uiFrameCounter = uiFrameCounter_save;
		m_uiGOPNumber = uiGOPNumber_save;
		m_bFirstGOPCoded = bFirstGOPCoded_save;
		m_uiFrameNum = uiFrameNum_save;
		m_uiDecompositionStages = uiDecompositionStages_save;
		m_uiParameterSetBits = uiParameterSetBits_save;
		m_pcAnchorFrameReconstructed->setZero();
		m_pcAnchorFrameReconstructed->copyAll(pcRecAnchorFrame_save);
		m_pcAnchorFrameOriginal->setZero();
		m_pcAnchorFrameOriginal->copyAll(pcOrgAnchorFrame_save);
		m_pcBaseLayerFrame->setZero();
		m_pcBaseLayerFrame->copyAll(pcBaseLayerRecFrame_save);
		m_pcBaseLayerResidual->setZero();
		m_pcBaseLayerResidual->copyAll(pcBaseLayerResidual_save);		
		m_pcLowPassBaseReconstruction->setZero();
		m_pcLowPassBaseReconstruction->copyAll(pcLowPassBaseReconstruction_save);

		FILE* d_gop; // write to file

		d_gop = fopen(m_cGOPModeFilename.c_str(), "a");
		for(i = 0; i < (1<<GN)/2; i++) 
    {
			if (m_puiGOPMode[i]) 
      {
				printf("%d ", m_puiGOPMode[i]);
				fprintf(d_gop, "%d ", m_puiGOPMode[i]);
			}
		}
		fprintf(d_gop, "\n");
		fclose(d_gop);

		m_cLPFrameNumList.clear();
		m_cLPFrameNumList += cLPFrameNumList_save;
		first = tmp_first;
		rcPicBufferInputList.clear();
		rcPicBufferOutputList.clear();
    rcAccessUnitList.clear();
		rcPicBufferInputList_temp.clear();
		rcPicBufferInputList_temp += rcPicBufferInputList_save;
		rcPicBufferOutputList_temp.clear();
		rcPicBufferOutputList_temp += rcPicBufferOutputList_save;
		for(i = 0; (i < 8) &&  m_puiGOPMode[i]; i++) {
			m_uiSelectPos = i;
			if (rcPicBufferInputList_temp.size() == 0) break;
			for(j = 0; j < (Int)(1<<m_puiGOPMode[i]) + (Int)first; j++) 
      {
				if (rcPicBufferInputList_temp.size() == 0) break;
				rcPicBufferInputList.push_back(rcPicBufferInputList_temp.front());
				rcPicBufferInputList_temp.pop_front();
				rcPicBufferOutputList.push_back(rcPicBufferOutputList_temp.front());
				rcPicBufferOutputList_temp.pop_front();
			}
			if (first == 1) first = 0;
			m_uiDecompositionStages = m_puiGOPMode[i];
      process (rcAccessUnitList, rcPicBufferInputList, rcPicBufferOutputList, rcPicBufferUnusedList, m_aaauidSeqBits );
			rcPicBufferOutputList_gop += rcPicBufferOutputList;
			rcPicBufferInputList.clear();
			rcPicBufferOutputList.clear();
		}
		rcPicBufferOutputList += rcPicBufferOutputList_gop;
		rcPicBufferInputList += rcPicBufferInputList_save;
		m_uiDecompositionStages = GN;		
	}

  // AGS coding
	else 
  {
		UInt order = m_uiFrameCounter/(1<<GN);
		UInt pre_encoded = 0;
		first = 0;
		m_puiGOPMode[0] = m_uiSelect[order][m_uiSelectPos];	
    
		for(i = 0; i < (Int)m_uiSelectPos;i++) 
    {
			pre_encoded += 1 << m_uiSelect[order][i];
		}

		if (order == 0 && m_uiSelectPos == 0)
			first = 1;
		else if (order == 0 && m_uiSelectPos != 0)
			pre_encoded++;
		
    if (m_bFinish)
      return Err::m_nOK;

    if ((Int)rcPicBufferInputList.size()-(Int)pre_encoded < (Int)m_uiSelect[order][m_uiSelectPos]) 
    {
      m_puiGOPMode[0] = rcPicBufferInputList.size() - (Int)pre_encoded;
      m_bFinish = 1;
    }

		rcPicBufferInputList.clear();
		rcPicBufferOutputList.clear();
		rcPicBufferInputList_temp.clear();
		rcPicBufferInputList_temp += rcPicBufferInputList_save;
		rcPicBufferOutputList_temp.clear();
		rcPicBufferOutputList_temp += rcPicBufferOutputList_save;
		
		for (i = 0; i < (Int)pre_encoded; i++) 
    {
			if (rcPicBufferInputList_temp.size() == 0) break;
			rcPicBufferInputList_temp.pop_front();			
			rcPicBufferUnusedList.push_back(rcPicBufferOutputList_temp.front());
			rcPicBufferOutputList_temp.pop_front();
		}
		
		for(j = 0; j < (Int)(1<<m_puiGOPMode[0]) + (Int)first; j++) 
    {
			if (rcPicBufferInputList_temp.size() == 0) break;
			rcPicBufferInputList.push_back(rcPicBufferInputList_temp.front());
			rcPicBufferInputList_temp.pop_front();			
			rcPicBufferOutputList.push_back(rcPicBufferOutputList_temp.front());
			rcPicBufferOutputList_temp.pop_front();
		}
		if (first == 1) first = 0;
		m_uiDecompositionStages = m_puiGOPMode[0];
    process (rcAccessUnitList, rcPicBufferInputList, rcPicBufferOutputList, rcPicBufferUnusedList, m_aaauidSeqBits );
		
		rcPicBufferInputList.clear();
		rcPicBufferInputList += rcPicBufferInputList_save;
		rcPicBufferUnusedList += rcPicBufferOutputList_temp;
		m_uiDecompositionStages = GN;		
	}
  return Err::m_nOK;
}

UInt MCTFEncoder::xSelectGOPMode (Double** mse,
									UInt* seltype,
									UInt pos,
									UInt gop_size,
									UInt gn_pos,
									UInt gn_1_pos)
{
	int i;

	UInt mode = 0;
	Double tmp_min = 0xffffffff;

	Double tmp_mse[4];

	// GOP 2^N size

	tmp_mse[0] = mse[4][0];
	tmp_mse[1] = (mse[3][0] + mse[3][1])/2.0;
	tmp_mse[2] = (mse[2][0] + mse[2][1] + mse[2][2] + mse[2][3])/4.0;
	tmp_mse[3] = (mse[1][0] + mse[1][1] + mse[1][2] + mse[1][3] + mse[1][4] + mse[1][5] + mse[1][6] + mse[1][7])/8.0;

//	printf("%.6lf %.6lf %.6lf %.6lf \n\n", tmp_mse[0], tmp_mse[1], tmp_mse[2], tmp_mse[3]);

	for(i = 0; i < 4; i++) {
		if (tmp_mse[i] < tmp_min) {
			mode = i;
			tmp_min = tmp_mse[i];
		}
	}

	if (mode == 0) {		// GOP == 2^N
		seltype[pos++] = gop_size;
		return pos;
	}

	// GOP 2^N-1 size (0)

	tmp_mse[1] = mse[3][0];
	tmp_mse[2] = (mse[2][0] + mse[2][1])/2.0;
	tmp_mse[3] = (mse[1][0] + mse[1][1] + mse[1][2] + mse[1][3])/4.0;

//	printf("%.6lf %.6lf %.6lf \n\n", tmp_mse[1], tmp_mse[2], tmp_mse[3]);

	tmp_min = 0xffffffff;
	for(i = 1; i < 4; i++) {
		if (tmp_mse[i] < tmp_min) {
			mode = i;
			tmp_min = tmp_mse[i];
		}
	}

	if (mode != 1) { // !=2^N-1

		tmp_mse[2] = mse[2][0];
		tmp_mse[3] = (mse[1][0] + mse[1][1])/2.0;

//  printf("%.6lf %.6lf \n\n", tmp_mse[2], tmp_mse[3]);

		if (tmp_mse[2] <= tmp_mse[3]) {	// GOP == 2^N-2			(4)	(8)
			seltype[pos++] = gop_size-2;
		}
		else {							// GOP == 2^N-3			(4)
			seltype[pos++] = gop_size-3;
			seltype[pos++] = gop_size-3;
		}

		tmp_mse[2] = mse[2][1];
		tmp_mse[3] = (mse[1][2] + mse[1][3])/2.0;

//		printf("%.6lf %.6lf \n\n", tmp_mse[2], tmp_mse[3]);


		if (tmp_mse[2] <= tmp_mse[3]) {	// GOP == 2^N-2			(4)	(8)
			seltype[pos++] = gop_size-2;
		}
		else {							// GOP == 2^N-3			(4)

			seltype[pos++] = gop_size-3;
			seltype[pos++] = gop_size-3;
		}
	}
	else {	// GOP == 2^N-1
		seltype[pos++] = gop_size-1;				// GOP == 2^N-1			(8)
	}


	// GOP 2^N-1 size (1)
	tmp_mse[1] = mse[3][1];
	tmp_mse[2] = (mse[2][2] + mse[2][3])/2.0;
	tmp_mse[3] = (mse[1][4] + mse[1][5] + mse[1][6] + mse[1][7])/4.0;

//	printf("%.6lf %.6lf %.6lf \n\n", tmp_mse[1], tmp_mse[2], tmp_mse[3]);


	tmp_min = 0xffffffff;
	for(i = 1; i < 4; i++) {
		if (tmp_mse[i] < tmp_min) {
			mode = i;
			tmp_min = tmp_mse[i];
		}
	}

	if (mode != 1) { // !=2^N-1

		tmp_mse[2] = mse[2][2];
		tmp_mse[3] = (mse[1][4] + mse[1][5])/2.0;

//		printf("%.6lf %.6lf \n\n", tmp_mse[2], tmp_mse[3]);

		if (tmp_mse[2] <= tmp_mse[3]) {	// GOP == 2^N-2			(4)	(8)
			seltype[pos++] = gop_size-2;
		}
		else {							// GOP == 2^N-3			(4)
			seltype[pos++] = gop_size-3;
			seltype[pos++] = gop_size-3;
		}

		tmp_mse[2] = mse[2][3];
		tmp_mse[3] = (mse[1][6] + mse[1][7])/2.0;

//		printf("%.6lf %.6lf \n\n", tmp_mse[2], tmp_mse[3]);

		if (tmp_mse[2] <= tmp_mse[3]) {	// GOP == 2^N-2			(4)	(8)
			seltype[pos++] = gop_size-2;
		}
		else {							// GOP == 2^N-3			(4)
			seltype[pos++] = gop_size-3;
			seltype[pos++] = gop_size-3;
		}
	}
	else {
		seltype[pos++] = gop_size-1;				// GOP == 2^N-3			(8)
	}

	return pos;
}
//}}Adaptive GOP structure

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
  for( uiLevel = 0; uiLevel <= MAX_DSTAGES; uiLevel++ )
  {
    m_adSeqBitsBase[uiLevel] += (Double)m_auiCurrGOPBitsBase[uiLevel];
    m_adSeqBitsFGS [uiLevel] += (Double)m_auiCurrGOPBitsFGS [uiLevel];
  }
	for( uiLevel = 0; uiLevel < MAX_SCALABLE_LAYERS; uiLevel++ )
	{
		m_adSeqBits		 [uiLevel] += (Double)m_auiCurrGOPBits		[uiLevel];
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
MCTFEncoder::xCalculateAndAddPSNR( PicBufferList& rcPicBufferInputList,
                                   UInt           uiStage,
                                   Bool           bOutput )
{
  //{{Adaptive GOP structure
  // --ETRI & KHU
  if (!m_uiUseAGS) 
  {
  //}}Adaptive GOP structure

  ROT ( uiStage > m_uiDecompositionStages  );
  
  //{{Adaptive GOP structure
  // --ETRI & KHU
  }
  else 
  {
    ROT ( uiStage > (UInt)( log10((double)m_uiMaxGOPSize)/log10(2.0) )  );
  }
  //}}Adaptive GOP structure

  RNOK( m_pcYuvFullPelBufferCtrl->initMb() );

  if( bOutput ) printf("\n");

  const YuvBufferCtrl::YuvBufferParameter&  cBufferParam  = m_pcYuvFullPelBufferCtrl->getBufferParameter();
  UInt                                      uiSkip        = 1 << ( m_uiDecompositionStages - uiStage );

  //{{Adaptive GOP structure
  // --ETRI & KHU
  if (m_uiUseAGS) 
  {
    uiSkip        = 1 << ( (UInt)( log10((double)m_uiMaxGOPSize)/log10(2.0) ) - uiStage );
  }
  //}}Adaptive GOP structure

  UInt                                      uiIndex;
  PicBufferList::iterator                   cIter;

  for( UInt uiFrame = ( m_bFirstGOPCoded ? uiSkip : 0 ); uiFrame <= m_uiGOPSize; uiFrame += uiSkip )
  {
    //===== get correct pic buffer ====
    for( uiIndex = uiFrame - ( m_bFirstGOPCoded ? 1 : 0 ), cIter = rcPicBufferInputList.begin(); uiIndex; uiIndex--, cIter++ )
    {
      ROT( cIter == rcPicBufferInputList.end() );
    }


    Int         iPoc        = m_pacControlData[uiFrame].getSliceHeader()->getPoc();
    IntFrame*   pcFrame     = m_papcFrame     [uiFrame];
    PicBuffer*  pcPicBuffer = *cIter;
    Double      dYPSNR      = 0.0;
    Double      dUPSNR      = 0.0;
    Double      dVPSNR      = 0.0;

    if( m_papcCLRecFrame )
    {
      pcFrame = m_papcCLRecFrame[uiFrame];
    }

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
    m_adPSNRSumY[ uiStage ] += dYPSNR;
    m_adPSNRSumU[ uiStage ] += dUPSNR;
    // heiko.schwarz@hhi.fhg.de: correct usage of the V-PSNR
    //m_adPSNRSumV[ uiStage ] += dUPSNR;
    m_adPSNRSumV[ uiStage ] += dVPSNR;

    //===== output PSNR =====
    if( bOutput )
    {
      // heiko.schwarz@hhi.fhg.de: correct usage of the V-PSNR
      printf( "  Frame%5d:    Y %6.4lf dB    U %6.4lf dB    V %6.4lf dB\n", iPoc, dYPSNR, dUPSNR, dVPSNR );
    }
  }


  return Err::m_nOK;
}


Void
print_with_comma( FILE* pFile, Double d )
{
  Int iBeforeComma  = (Int)floor( d );
  Int iAfterComma   = (Int)floor( 1000.0 * ( d - (Double)iBeforeComma ) + 0.5 );
  fprintf( pFile, "%d,%03d", iBeforeComma, iAfterComma );
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
  UInt  uiMinStage        = ( !m_bH264AVCCompatible || m_bWriteSubSequenceSei ? 0 : max( 0, (Int)uiMaxStage - 1 ) );
  Char  acResolution[10];

  sprintf( acResolution, "%dx%d", 16*m_uiFrameWidthInMb, 16*m_uiFrameHeightInMb );

  //===== set final sum of bits and average PSNR's =====
  for( uiStage = 0; uiStage <= MAX_DSTAGES; uiStage++ )
  {
    m_adSeqBitsFGS          [uiStage] += m_adSeqBitsBase            [uiStage];

    if( uiStage  )
    {
      m_adSeqBitsBase       [uiStage] += m_adSeqBitsBase            [uiStage-1];
      m_adSeqBitsFGS        [uiStage] += m_adSeqBitsFGS             [uiStage-1];
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
		m_adSeqBits	[uiStage]	+= m_adSeqBits	[uiStage-1];
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
					dBits = aaadCurrBits[m_uiBaseLayerId][uiLevel][MAX_QUALITY_LEVELS-1];
				}
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
				  dBits = aaadCurrBits[m_uiBaseLayerId][uiLevel][MAX_QUALITY_LEVELS-1];
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

	if( m_uiLayerId == 0 )
	{
		printf( " \n\n\nSUMMARY:\n" );
		printf( "                       " " SNR Level" " bitrate " "   Y-PSNR" "   U-PSNR" "   V-PSNR\n" );
		printf( "                       " " ---------" " --------" " --------" " --------" " --------\n" );
	}
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

      if( uiFGS == m_dNumFGSLayers )
      {
			  printf( " %9s @ %7.4lf" " %10.4lf" " %10.4lf" " %8.4lf" " %8.4lf" " %8.4lf" "\n",
				  acResolution,
				  dFps,
				  (Double)uiFGS,
				  dBitrate,
				  m_adPSNRSumY	[uiStage],
				  m_adPSNRSumU	[uiStage],
				  m_adPSNRSumV	[uiStage] );
      }
      else
      {
			  printf( " %9s @ %7.4lf" " %10.4lf" " %10.4lf" "\n",
				  acResolution,
				  dFps,
				  (Double)uiFGS,
				  dBitrate );
      }
		}
	}

  ruiNumCodedFrames = m_auiNumFramesCoded[uiMaxStage];
  rdOutputRate      = m_fOutputFrameRate;


  return Err::m_nOK;
}



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
  const Int   iDiff             = m_cLPFrameNumList.front() - uiCurrFrameNr;
  UInt        uiDiff            = ( uiMaxFrameNumber - iDiff ) % uiMaxFrameNumber;

  // generate mmco commands for inter b frames
  UInt uiPos = 0;
  while( --uiDiff )
  {
    rcSH.getMmcoBuffer().set( uiPos++, Mmco( MMCO_SHORT_TERM_UNUSED, uiDiff-1 ) );
  }

  // generate mmco command for high-pass frame
  UInt uiNeedLowPassBefore = max( 1, rcSH.getNumRefIdxActive( LIST_0 ) );
  if( m_cLPFrameNumList.size() > uiNeedLowPassBefore )
  {
    const Int iDiff   = m_cLPFrameNumList.popBack() - uiCurrFrameNr;
    UInt      uiDiff  = ( uiMaxFrameNumber - iDiff ) % uiMaxFrameNumber;
    rcSH.getMmcoBuffer().set( uiPos++, Mmco( MMCO_SHORT_TERM_UNUSED, uiDiff-1 ) );
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
}


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
      if( m_pacControlData[i].getSliceHeader()->getTemporalLevel() < uiLevel )
      {
        rcFrameNumList.push_back( m_pacControlData[i].getSliceHeader()->getFrameNum() );
        if( rcFrameNumList.size() == uiMaxSize )
        {
          break;
        }
      }
    }
  }
  else
  {
    for( Int i = uiCurrBasePos-1; i >= 0; i-- )
    {
      if( m_pacControlData[i].getSliceHeader()->getTemporalLevel() < uiLevel )
      {
        rcFrameNumList.push_back( m_pacControlData[i].getSliceHeader()->getFrameNum() );
        if( rcFrameNumList.size() == uiMaxSize )
        {
          break;
        }
      }
    }
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
    for ( UInt uiSimplePriId = 0; uiSimplePriId < (1 << PRI_ID_BITS); uiSimplePriId++ )
    {
        UInt uiLayer, uiTempLevel, uiQualLevel;
        m_pcSPS->getSimplePriorityMap( uiSimplePriId, uiTempLevel, uiLayer, uiQualLevel );
        if ( pcSliceHeader->getTemporalLevel() == uiTempLevel && m_uiLayerId == uiLayer && pcSliceHeader->getQualityLevel() == uiQualLevel )
        {
            pcSliceHeader->setSimplePriorityId ( uiSimplePriId );
            bFound = true;
            break;
        }
    }
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
MCTFEncoder::xUpdateCompensation( IntFrame*        pcMCFrame,
                                  RefFrameList*    pcRefFrameList,
                                  CtrlDataList*    pcCtrlDataList,
                                  ListIdx          eListUpd)
{
  ListIdx eListPrd = ListIdx( 1-eListUpd );
  Int     iRefIdx;
  MbEncoder*  pcMbEncoder     = m_pcSliceEncoder->getMbEncoder();


  if( pcCtrlDataList->getActive() < 1 )
    return Err::m_nDataNotAvailable;


  for( iRefIdx = 1; iRefIdx <= (Int)pcCtrlDataList->getActive(); iRefIdx++ )    
  {

    for( UInt uiMbIndex = 0; uiMbIndex < m_uiMbNumber; uiMbIndex++ )
    {
      UInt          uiMbY           = uiMbIndex / m_uiFrameWidthInMb;
      UInt          uiMbX           = uiMbIndex % m_uiFrameWidthInMb;

      MbDataCtrl*   pcMbDataCtrlPrd = (*pcCtrlDataList)[ iRefIdx ]->getMbDataCtrl();

      MbDataAccess* pcMbDataAccess  = 0;

      RNOK( pcMbDataCtrlPrd         ->initMb( pcMbDataAccess, uiMbY, uiMbX                  ) );
      RNOK( m_pcYuvFullPelBufferCtrl->initMb(                 uiMbY, uiMbX ) );
      RNOK( m_pcYuvHalfPelBufferCtrl->initMb(                 uiMbY, uiMbX ) );
      RNOK( m_pcMotionEstimation    ->initMb(                 uiMbY, uiMbX, *pcMbDataAccess ) );

      RNOK( pcMbEncoder->compensateUpdate( *pcMbDataAccess, pcMCFrame,
                                            iRefIdx, eListPrd, (*pcRefFrameList)[ iRefIdx ] ) );
    }
  }  // iRefIdx

  return Err::m_nOK;
}

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



H264AVC_NAMESPACE_END


