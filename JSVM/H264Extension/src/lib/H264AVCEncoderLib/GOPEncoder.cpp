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




H264AVC_NAMESPACE_BEGIN



#define FACTOR_22_HP  0.70710678118654752440084436210485  //sqrt( 1.0/ 2.0)
#define FACTOR_22_LP  1.4142135623730950488016887242097   //sqrt( 2.0/ 1.0)
#define FACTOR_53_HP  0.81649658092772603273242802490196  //sqrt( 2.0/ 3.0)
#define FACTOR_53_LP  1.1795356492391770676634011002828   //sqrt(32.0/23.0)


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
, m_uiBaseLayerId                   ( MSYS_UINT_MAX )
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
, m_uiGOPId                         ( 0 )
, m_uiGOPNumber                     ( 0 )
//----- frame memories -----
, m_papcFrame                       ( 0 )
, m_papcResidual                    ( 0 )
, m_papcSubband                     ( 0 )
, m_pcLowPassBaseReconstruction     ( 0 )
, m_pcAnchorFrameOriginal           ( 0 )
, m_pcAnchorFrameReconstructed      ( 0 )
, m_pcBaseLayerFrame                ( 0 )
, m_pcBaseLayerResidual             ( 0 )
//----- control data arrays -----
, m_pacControlData                  ( 0 )
, m_pcBaseLayerCtrl                 ( 0 )
//----- auxiliary buffers -----
, m_uiWriteBufferSize               ( 0 )
, m_pucWriteBuffer                  ( 0 )
, m_pusUpdateWeights                ( 0 )
//----- PSNR & rate -----
, m_fOutputFrameRate                ( 0.0 )
, m_uiParameterSetBits              ( 0 )
//--- FGS ---
, m_uiFGSMode                       ( 0 )
, m_pFGSFile                        ( 0 )
, m_uiFGSCutLayer                   ( 0 )
, m_uiFGSCutPath                    ( 0 )
, m_dFGSCutFactor                   ( 0.0 )
, m_iLastFGSError                   ( 0 )
, m_uiNotYetConsideredBaseLayerBits ( 0 )
{
  ::memset( m_abIsRef,          0x00, sizeof( m_abIsRef           ) );
  ::memset( m_apcFrameTemp,     0x00, sizeof( m_apcFrameTemp      ) );
  ::memset( m_apcLowPassTmpOrg, 0x00, sizeof( m_apcLowPassTmpOrg  ) );

  for( UInt ui = 0; ui <= MAX_DSTAGES; ui++ )
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
  m_uiLayerId               = pcLayerParameters->getLayerId                 ();
  m_uiBaseLayerId           = m_uiLayerId ? m_uiLayerId - 1 : MSYS_UINT_MAX;
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
  m_bUpdate                 = pcLayerParameters->getUpdateStep              ()  > 0  &&
                            ( pcCodingParameter->getBaseLayerMode           () == 0 || m_uiLayerId >  0 );
  m_bH264AVCCompatible      = pcCodingParameter->getBaseLayerMode           ()  > 0 && m_uiLayerId == 0;
  m_bInterLayerPrediction   = pcLayerParameters->getInterLayerPredictionMode()  > 0;
  m_bAdaptivePrediction     = pcLayerParameters->getInterLayerPredictionMode()  > 1;
  m_bHaarFiltering          = false;
  m_bBiPredOnly             = false;
  m_bAdaptiveQP             = pcLayerParameters->getAdaptiveQPSetting       ()  > 0;
  m_bWriteSubSequenceSei    = pcCodingParameter->getBaseLayerMode           ()  > 1 && m_uiLayerId == 0;

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
  
  for( UInt ui = 0; ui <= MAX_DSTAGES; ui++ )
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


  //----- FGS -----
  m_uiFGSMode = pcLayerParameters->getFGSMode();
  m_pFGSFile  = 0;
  
  if( m_uiFGSMode == 1 )
  {
    m_pFGSFile = ::fopen( pcLayerParameters->getFGSFilename().c_str(), "wt" );
    ROF( m_pFGSFile );
  }
  if( m_uiFGSMode == 2 )
  {
    m_pFGSFile = ::fopen( pcLayerParameters->getFGSFilename().c_str(), "rt" );
    ROF( m_pFGSFile );
  }

  m_uiFGSCutLayer                   = 0;
  m_uiFGSCutPath                    = 0;
  m_dFGSCutFactor                   = 0;
  m_iLastFGSError                   = 0;
  m_uiNotYetConsideredBaseLayerBits = 0;

  // analyse and set parameters
  if( m_uiFGSMode == 2 )
  {
    Char  acLine        [1000];
    UInt  uiNumFrames                       =     0;
    UInt  uiBaseBits                        =     0;
    UInt  uiSumBaseBits                     =     0;
    UInt  uiFGSBits     [MAX_FGS_LAYERS]    =   { 0, 0, 0 };
    UInt  uiSumFGSBits  [MAX_FGS_LAYERS]    =   { 0, 0, 0 };
    UInt  uiPBits       [MAX_FGS_LAYERS][3] = { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } };
    UInt  uiSumPBits    [MAX_FGS_LAYERS][3] = { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } };

    for( ; ; )
    {
      Int i, c;
      for( i = 0; ( c = fgetc(m_pFGSFile), ( c != '\n' && c != EOF ) ); acLine[i++] = c );
      acLine[i] = '\0';
      if( feof(m_pFGSFile) )
      {
        break;
      }
  
      sscanf( acLine, "%d %d %d %d %d %d %d %d %d %d %d %d %d ", // MAX_QUALITY_LEVEL must be equal to 4
        &uiBaseBits,
        &uiPBits[0][0], &uiPBits[0][1], &uiPBits[0][2], &uiFGSBits[0],
        &uiPBits[1][0], &uiPBits[1][1], &uiPBits[1][2], &uiFGSBits[1],
        &uiPBits[2][0], &uiPBits[2][1], &uiPBits[2][2], &uiFGSBits[2] );

      uiNumFrames   ++;
      uiSumBaseBits += uiBaseBits;
      for( Int k = 0; k < MAX_FGS_LAYERS; k++ )
      {
        for( Int l = 0; l < 3; l++ )  uiSumPBits[k][l] += uiPBits[k][l];
        uiSumFGSBits[k] += uiFGSBits[k];
      }
    }
    ROF( uiNumFrames );

    Double  dTargetBits   = 1000.0 * pcLayerParameters->getFGSRate() * (Double)uiNumFrames / pcLayerParameters->getOutputFrameRate();
    UInt    uiTargetBits  = (UInt)floor( dTargetBits + 0.5 );


    if( uiTargetBits <= uiSumBaseBits )
    {
      ROT(1);
    }
    uiTargetBits -= uiSumBaseBits;

    for( UInt uiLayer = 0; uiLayer < MAX_FGS_LAYERS; uiLayer++ )
    {
      if( uiTargetBits <= uiSumFGSBits[uiLayer] )
      {
        m_uiFGSCutLayer = uiLayer+1;
        uiTargetBits    = ( uiTargetBits >= (uiSumFGSBits[uiLayer]-uiSumPBits[uiLayer][0]-uiSumPBits[uiLayer][1]-uiSumPBits[uiLayer][2] )
                          ? uiTargetBits  - (uiSumFGSBits[uiLayer]-uiSumPBits[uiLayer][0]-uiSumPBits[uiLayer][1]-uiSumPBits[uiLayer][2] ) : 0 );
        for( UInt uiPath = 0; uiPath < 3; uiPath++ )
        {
          if( uiTargetBits <= uiSumPBits[uiLayer][uiPath] )
          {
            m_uiFGSCutPath  = uiPath;
            m_dFGSCutFactor = (Double)uiTargetBits / (Double)uiSumPBits[uiLayer][uiPath];
            break;
          }
          else
          {
            uiTargetBits -= uiSumPBits[uiLayer][uiPath];
          }
        }
        break;
      }
      else
      {
        uiTargetBits -= uiSumFGSBits[uiLayer];
      }
    }
    ROF( m_uiFGSCutLayer );
    m_dNumFGSLayers = m_uiFGSCutLayer;
    ::fseek( m_pFGSFile, 0, SEEK_SET );
  }

  return Err::m_nOK;
}



__inline UInt downround2powerof2( UInt i ) { UInt r = 1; for( ; ( 1 << r ) <= i; r++ ); return ( 1 << ( r - 1 ) ); }

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
  m_uiGOPId               = 0;
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
  ROFRS   ( ( m_papcResidual                  = new IntFrame* [ m_uiMaxGOPSize + 1 ]      ), Err::m_nERR );
  ROFRS   ( ( m_papcSubband                   = new IntFrame* [ m_uiMaxGOPSize + 1 ]      ), Err::m_nERR );
  
  for( uiIndex = 0; uiIndex <= m_uiMaxGOPSize; uiIndex++ )
  {
    ROFRS ( ( m_papcFrame         [ uiIndex ] = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                              *m_pcYuvHalfPelBufferCtrl ) ), Err::m_nERR );
    ROFRS ( ( m_papcResidual      [ uiIndex ] = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                              *m_pcYuvHalfPelBufferCtrl ) ), Err::m_nERR );
    ROFRS ( ( m_papcSubband       [ uiIndex ] = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                              *m_pcYuvHalfPelBufferCtrl ) ), Err::m_nERR );
    RNOK  (   m_papcFrame         [ uiIndex ] ->init        () );
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

    Bool          bLowPass                    = ( ( uiIndex % ( 1 << m_uiDecompositionStages ) ) == 0 );
    SliceHeader*  pcSliceHeader               = 0;
    ROFRS ( (     pcSliceHeader               = new SliceHeader ( *m_pcSPS, bLowPass ? *m_pcPPSLP : *m_pcPPSHP ) ), Err::m_nERR );
    RNOK  (       m_pacControlData[ uiIndex ] . setSliceHeader  (  pcSliceHeader ) );
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
  ROFRS( ( m_pusUpdateWeights = new UShort[ uiNum4x4Blocks      ] ), Err::m_nERR );


  
  //========== RE-INITIALIZE OBJECTS ==========
  RNOK( m_cConnectionArray.init   ( rcSPS ) );
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

  {
    MbDataCtrl*   pcMbDataCtrl  = m_cControlDataUpd.getMbDataCtrl  ();
    SliceHeader*  pcSliceHeader = m_cControlDataUpd.getSliceHeader ();
    if( pcMbDataCtrl )
    {
      RNOK( pcMbDataCtrl->uninit() );
    }
    delete pcMbDataCtrl;
    delete pcSliceHeader;
    m_cControlDataUpd.setMbDataCtrl ( 0 );
    m_cControlDataUpd.setSliceHeader( 0 );
  }

  if( m_pcBaseLayerCtrl )
  {
    RNOK( m_pcBaseLayerCtrl->uninit() );
    delete m_pcBaseLayerCtrl;
    m_pcBaseLayerCtrl = 0;
  }
  

  //========== DELETE UPDATE WEIGHTS ARRAY and WRITE BUFFER ==========
  delete [] m_pucWriteBuffer;
  delete [] m_pusUpdateWeights;
  m_pusUpdateWeights  = 0;
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
  Bool          bHalfResBaseLayer     =  rcControlData.isHalfResolutionBaseLayer();
  Bool          bEstimateBase         =  rcSliceHeader.getBaseLayerId           () == MSYS_UINT_MAX && ! pcBaseLayerCtrl;
  Bool          bEstimateMotion       =  rcSliceHeader.getAdaptivePredictionFlag() || bEstimateBase;


  //===== copy motion if non-adaptive prediction =====
  if( ! bEstimateMotion )
  {
    ROF ( pcBaseLayerCtrl )
    RNOK( pcMbDataCtrl->copyMotion( *pcBaseLayerCtrl ) );
    return Err::m_nOK;
  }

  
  //===== initialization =====
  if( ! m_bLoadMotionInfo )
  {
    RNOK( m_pcMotionEstimation->initSlice( rcSliceHeader ) );
    RNOK( pcMbEncoder         ->initSlice( rcSliceHeader ) );
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
                                               bHalfResBaseLayer,
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
                              UInt&                   ruiBits )
{
  Bool          bFinished     = false;
  SliceHeader*  pcSliceHeader = rcControlData.getSliceHeader();
  MbDataCtrl*   pcMbDataCtrl  = rcControlData.getMbDataCtrl ();
  IntFrame*     pcOrgResidual = pcTempFrame;

  //===== set original residual signal =====
  RNOK( pcOrgResidual->subtract( pcFrame, pcPredSignal ) );
  RNOK( xAddBaseLayerResidual( rcControlData, pcOrgResidual, true ) );

  UInt uiTarget;
  UInt uiFGSMaxBits = 0;
  
  if( m_uiFGSMode == 2 )
  {
    Char  acLine    [1000];
    UInt  uiBaseBits                    =     0;
    UInt  uiFGSBits [MAX_FGS_LAYERS]    =   { 0, 0, 0 };
    UInt  uiPBits   [MAX_FGS_LAYERS][3] = { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } };
    Int   i, c;
    for(  i = 0; ( c = fgetc(m_pFGSFile), ( c != '\n' && c != EOF ) ); acLine[i++] = c );
    acLine[i] = '\0';
    ROT( feof(m_pFGSFile) );

    sscanf( acLine, "%d %d %d %d %d %d %d %d %d %d %d %d %d ",
      &uiBaseBits,
      &uiPBits[0][0], &uiPBits[0][1], &uiPBits[0][2], &uiFGSBits[0],
      &uiPBits[1][0], &uiPBits[1][1], &uiPBits[1][2], &uiFGSBits[1],
      &uiPBits[2][0], &uiPBits[2][1], &uiPBits[2][2], &uiFGSBits[2] );

    if( m_uiFGSCutLayer )
    {
      uiFGSMaxBits  = (UInt)floor( (Double)uiPBits[m_uiFGSCutLayer-1][m_uiFGSCutPath]*m_dFGSCutFactor );
      if( (Int)uiFGSMaxBits >= m_iLastFGSError )
      {
        uiFGSMaxBits   -= m_iLastFGSError;
        m_iLastFGSError = 0;
      }
      
      uiTarget      = uiFGSBits[m_uiFGSCutLayer-1] + uiFGSMaxBits;
      for( UInt ui = m_uiFGSCutPath; ui < 3; ui++ )
        uiTarget -= uiPBits[m_uiFGSCutLayer-1][ui];
    }
  }

  //===== initialize FGS encoder =====
  RNOK( m_pcRQFGSEncoder->initPicture( rcControlData.getSliceHeader(),
                                       rcControlData.getMbDataCtrl(),
                                       pcOrgResidual,
                                       m_dNumFGSLayers,
                                       rcControlData.getLambda(),
                                       m_iMaxDeltaQp,
                                       bFinished,
                                       m_uiFGSCutLayer,
                                       m_uiFGSCutPath,
                                       uiFGSMaxBits ) );


  //===== encoding of FGS packets =====
  for( UInt uiRecLayer = 1; !bFinished; uiRecLayer++ )
  {
    UInt  uiPacketBits  = 0;

    //----- init NAL UNIT -----
    RNOK( xInitExtBinDataAccessor               (  m_cExtBinDataAccessor ) );
    RNOK( m_pcNalUnitEncoder->initNalUnit       ( &m_cExtBinDataAccessor ) );

    //---- write Slice Header -----
    ETRACE_NEWSLICE;
    //pcSliceHeader->setQualityLevel       ( uiRecLayer );
    RNOK( m_pcNalUnitEncoder->write  ( *pcSliceHeader ) );
    Int iQp = pcSliceHeader->getPicQp();

    //---- encode next bit-plane for current NAL unit ----
    Bool bCorrupted = false;
    RNOK( m_pcRQFGSEncoder->encodeNextLayer     ( bFinished, bCorrupted, ( m_uiFGSMode == 1 ? m_pFGSFile : 0 ) ) );

    //----- close NAL UNIT -----
    RNOK( m_pcNalUnitEncoder->closeNalUnit      ( uiPacketBits ) );
    RNOK( xAppendNewExtBinDataAccessor          ( rcOutExtBinDataAccessorList, &m_cExtBinDataAccessor ) );
    uiPacketBits += 4*8;

    if( uiRecLayer == m_uiFGSCutLayer )
    {
      m_iLastFGSError += (Int)uiPacketBits - (Int)uiTarget;
    }

    if( m_uiFGSMode == 1 && m_pFGSFile )
    {
      fprintf( m_pFGSFile, "\t%d", uiPacketBits );
    }

    printf("  Frame %4d ( LId%2d, TL%2d, QL%2d, PrRef,              QP%3d ) %10d bits\n",
      rcControlData.getSliceHeader()->getPoc                    (),
      rcControlData.getSliceHeader()->getLayerId                (),
      rcControlData.getSliceHeader()->getTemporalLevel          (),
      uiRecLayer, iQp, uiPacketBits );

    ruiBits += uiPacketBits;
  }
  

  //===== reconstruction =====
  RNOK( m_pcRQFGSEncoder->reconstruct   ( pcFrame ) );
  RNOK( m_pcRQFGSEncoder->finishPicture () );

  RNOK( xAddBaseLayerResidual           ( rcControlData, pcFrame, false ) );
  RNOK( pcResidual      ->copy          ( pcFrame ) );
  RNOK( xZeroIntraMacroblocks           ( pcResidual, rcControlData ) );
  RNOK( pcFrame         ->add           ( pcPredSignal ) );
  RNOK( xClipIntraMacroblocks   ( pcFrame, rcControlData, pcSliceHeader->getTemporalLevel() == 0 ) );

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

  //----- init NAL UNIT -----
  RNOK( xInitExtBinDataAccessor                 (  m_cExtBinDataAccessor ) );
  RNOK( m_pcNalUnitEncoder->initNalUnit         ( &m_cExtBinDataAccessor ) );

  //---- write Slice Header -----
  ETRACE_NEWSLICE;
  RNOK( m_pcNalUnitEncoder->write               ( *pcSliceHeader ) );



  rcControlData.getDPCMRefFrameList( LIST_0 ).reset();
  rcControlData.getDPCMRefFrameList( LIST_1 ).reset();

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
    RNOK( rcControlData.getDPCMRefFrameList ( LIST_0 ).add  ( m_pcLowPassBaseReconstruction ) );
    RNOK( xFillAndUpsampleFrame                             ( m_pcLowPassBaseReconstruction ) );

    RNOK( m_pcSliceEncoder->encodeInterPictureP ( uiBits,
                                                  pcFrame,
                                                  pcResidual,
                                                  pcPredSignal,
                                                  rcControlData,
                                                  m_uiFrameWidthInMb,
                                                  rcControlData.getDPCMRefFrameList( LIST_0 ) ) );

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

  ruiBits += uiBits + uiBitsSEI;

  return Err::m_nOK;
}







ErrVal
MCTFEncoder::xEncodeHighPassSignal( ExtBinDataAccessorList&  rcOutExtBinDataAccessorList,
                                    ControlData&             rcControlData,
                                    IntFrame*                pcFrame,
                                    IntFrame*                pcResidual,
                                    IntFrame*                pcPredSignal,
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

  //----- init NAL UNIT -----
  RNOK( xInitExtBinDataAccessor        (  m_cExtBinDataAccessor ) );
  RNOK( m_pcNalUnitEncoder->initNalUnit( &m_cExtBinDataAccessor ) );

  //---- write Slice Header -----
  ETRACE_NEWSLICE;
  RNOK( m_pcNalUnitEncoder->write( *rcControlData.getSliceHeader() ) );


  //----- write slice data -----
  RNOK( m_pcSliceEncoder->encodeHighPassPicture ( uiMbCoded, uiBits, uiBitsRes,
                                                 *rcControlData.getSliceHeader            (),
                                                  pcFrame,
                                                  pcResidual,
                                                  pcPredSignal,
                                                  rcControlData.getBaseLayerSbb           (),
                                                  rcControlData.getBaseLayerRec           (),
                                                  rcControlData.getMbDataCtrl             (),
                                                  rcControlData.getBaseLayerCtrl          (),
                                                  m_uiFrameWidthInMb,
                                                  rcControlData.getLambda                 (),
                                                  m_iMaxDeltaQp,
                                                  rcControlData.isHalfResolutionBaseLayer () ) );
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
    rcControlData.getSliceHeader()->isH264AVCCompatible       () ? "AVC" : " LP",
    rcControlData.getSliceHeader()->getSliceType              () == B_SLICE ? 'B' : 'P',
    rcControlData.getSliceHeader()->getBaseLayerId            (),
    rcControlData.getSliceHeader()->getAdaptivePredictionFlag () ? 1 : 0,
    rcControlData.getSliceHeader()->getPicQp                  (),
    uiBits + uiBitsSEI );

  ruiBits     += uiBits + uiBitsSEI;
  ruiBitsRes  += uiBitsRes;

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
    m_papcFrame   [ uiFrame ]->copyAll      ( m_pcAnchorFrameOriginal );
    m_papcFrame   [ uiFrame ]->setBandType  ( LPS );
    m_papcFrame   [ uiFrame ]->initRecLayer ();
    m_papcFrame   [ uiFrame ]->setValid     ();

    m_papcResidual[ uiFrame ]->setBandType  ( HPS );
    m_papcResidual[ uiFrame ]->initRecLayer ();
    m_papcSubband [ uiFrame ]->setBandType  ( LPS );
    m_papcSubband [ uiFrame ]->initRecLayer ();
    uiFrame    ++;
  }
  else
  {
    m_uiGOPSize--;
  }
  for( ; uiFrame <= m_uiGOPSize; uiFrame++, cInputIter++ )
  {
    PicBuffer*  pcPicBuffer = *cInputIter;
    m_papcFrame   [ uiFrame ]->load         ( pcPicBuffer );
    m_papcFrame   [ uiFrame ]->setBandType  ( LPS );
    m_papcFrame   [ uiFrame ]->initRecLayer ();
    m_papcFrame   [ uiFrame ]->setPOC       ( m_uiFrameCounter++ << m_uiTemporalResolution );
    m_papcFrame   [ uiFrame ]->setValid     ();

    m_papcResidual[ uiFrame ]->setBandType  ( HPS );
    m_papcResidual[ uiFrame ]->initRecLayer ();
    m_papcSubband [ uiFrame ]->setBandType  ( uiFrame == 0 || uiFrame == m_uiGOPSize ? LPS : HPS );
    m_papcSubband [ uiFrame ]->initRecLayer ();
  }



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
  if( m_bH264AVCCompatible )
  {
    //===== RPLR and MMCO commands =====
    for( uiFrame = m_bFirstGOPCoded ? 1 : 0; uiFrame <= m_uiGOPSize; uiFrame++ )
    {
      RNOK( xInitReordering( uiFrame ) );
    }
  }



  //========== INITIALIZE SCALING FACTORS ==========
  for( uiFrame = 0; uiFrame <= m_uiGOPSize; uiFrame++ )
  {
    m_pacControlData[ uiFrame ].setScalingFactor( 1.0 );

    MbDataCtrl* pcMbDataCtrl = m_pacControlData[ uiFrame ].getMbDataCtrl();
    RNOK( pcMbDataCtrl->reset () );
    RNOK( pcMbDataCtrl->clear () );
  }
  

  m_uiNotYetConsideredBaseLayerBits = 0;
  UInt* pauiBLGopBitsBase           = m_pcH264AVCEncoder->getGOPBitsBase( m_uiBaseLayerId );
  UInt* pauiBLGopBitsFGS            = m_pcH264AVCEncoder->getGOPBitsFGS ( m_uiBaseLayerId );
  for( UInt uiStage = 0; uiStage <= MAX_DSTAGES; uiStage++ )
  {
    m_auiCurrGOPBitsBase[uiStage]      = ( pauiBLGopBitsBase ? pauiBLGopBitsBase [uiStage] : 0 );
    m_auiCurrGOPBitsFGS [uiStage]      = ( pauiBLGopBitsFGS  ? pauiBLGopBitsFGS  [uiStage] : 0 );

    m_uiNotYetConsideredBaseLayerBits += m_auiCurrGOPBitsBase[uiStage];
    m_uiNotYetConsideredBaseLayerBits += m_auiCurrGOPBitsFGS [uiStage];
  }
  m_auiCurrGOPBitsBase  [0]           += m_uiParameterSetBits;
  m_uiNotYetConsideredBaseLayerBits   += m_uiParameterSetBits;

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
MCTFEncoder::getBaseLayerData( IntFrame*&   pcFrame,
                               IntFrame*&   pcResidual,
                               MbDataCtrl*& pcMbDataCtrl,
                               Bool&        bForCopyOnly,
                               Bool         bSpatialScalability,
                               Int          iPoc,
                               Bool         bMotion )
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
        pcResidual    = m_papcResidual  [uiFrame];
#if UNRESTRICTED_INTER_LAYER_PREDICTION
        pcFrame       = m_papcFrame     [uiFrame];
#else
        pcFrame       = m_papcSubband   [uiFrame];
#endif
        pcMbDataCtrl  = m_pacControlData[uiFrame].getMbDataCtrl();
        bForCopyOnly  = false;
      }
      else if( ! m_bH264AVCCompatible )
      {
        pcResidual    = 0;
        pcFrame       = 0;
        pcMbDataCtrl  = m_pacControlData[uiFrame].getMbDataCtrl();
        bForCopyOnly  = true;
      }
      uiPos = uiFrame;
      break;
    }
  }

#if ! UNRESTRICTED_INTER_LAYER_PREDICTION
  if( pcFrame )
  {
    Bool bHighPass = m_pacControlData[uiPos].getSliceHeader()->getTemporalLevel() > 0;
    if ( bHighPass && bSpatialScalability )
    {
      RNOK( m_apcFrameTemp[0]->copy( pcFrame ) );
      pcFrame = m_apcFrameTemp[0];

      m_pcLoopFilter->setFilterMode( LoopFilter::LFMode( LoopFilter::LFM_NO_INTER_FILTER + LoopFilter::LFM_EXTEND_INTRA_SUR ) );
      RNOK( m_pcLoopFilter->process(*m_pacControlData[uiPos].getSliceHeader (),
                                     pcFrame,
                                     m_pacControlData[uiPos].getMbDataCtrl  (),
                                     m_pacControlData[uiPos].getMbDataCtrl  (),
                                     m_uiFrameWidthInMb,
                                     NULL,
                                     NULL ) );
      m_pcLoopFilter->setFilterMode();
    }
    else if( bSpatialScalability )
    {
      RNOK( m_pcLoopFilter->process(*m_pacControlData[uiPos].getSliceHeader (),
                                     pcFrame,
                                     m_pacControlData[uiPos].getMbDataCtrl  (),
                                     m_pacControlData[uiPos].getMbDataCtrl  (),
                                     m_uiFrameWidthInMb,
                                    &m_pacControlData[uiPos].getDPCMRefFrameList( LIST_0 ),
                                    &m_pacControlData[uiPos].getDPCMRefFrameList( LIST_1 ) ) );
    }
  }
#endif
  
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
  Bool          bHalfResolution     = false;

  if( ! m_bInterLayerPrediction )
  {
    pcSliceHeader->setBaseLayerId           ( MSYS_UINT_MAX );
    pcSliceHeader->setAdaptivePredictionFlag( false );
    rcControlData .setBaseLayer             ( MSYS_UINT_MAX, MSYS_UINT_MAX );
    rcControlData .setBaseLayerResolution   ( false );
    return Err::m_nOK;
  }

  RNOK( m_pcH264AVCEncoder->getBaseLayerStatus( uiBaseLayerId, uiBaseLayerIdMotion, bHalfResolution, m_uiLayerId, iPoc ) );

  Bool  bAdaptive = ( ! pcSliceHeader->getTemporalLevel() && ( ! pcSliceHeader->isIntra() || bHalfResolution ) ? true : m_bAdaptivePrediction );

  if( uiBaseLayerId != uiBaseLayerIdMotion && ( bAdaptive || bHalfResolution ) )
  {
    uiBaseLayerIdMotion = uiBaseLayerId;
  }

  pcSliceHeader->setBaseLayerId             ( uiBaseLayerId );
  pcSliceHeader->setAdaptivePredictionFlag  ( uiBaseLayerId != MSYS_UINT_MAX ? bAdaptive : false );
  rcControlData .setBaseLayer               ( uiBaseLayerId, uiBaseLayerIdMotion );
  rcControlData .setBaseLayerResolution     ( bHalfResolution );

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
  NalRefIdc     eNalRefIdc      = NalRefIdc( min( 3, max( 0, (Int)( m_uiDecompositionStages - m_uiNotCodedMCTFStages - uiTemporalLevel ) ) ) );

  // Bug fix: yiliang.bao@nokia.com
  // encoder crashes if GOP size is 1 (m_uiDecompositionStages == 0), 
  // because a low-pass frame becomes a non-reference frame
  if (uiTemporalLevel == 0 && eNalRefIdc == NAL_REF_IDC_PRIORITY_LOWEST)
    eNalRefIdc = NAL_REF_IDC_PRIORITY_HIGHEST;

  NalUnitType   eNalUnitType    = ( m_bH264AVCCompatible
                                    ? ( uiFrameIdInGOP ? NAL_UNIT_CODED_SLICE          : NAL_UNIT_CODED_SLICE_IDR          )
                                    : ( uiFrameIdInGOP ? NAL_UNIT_CODED_SLICE_SCALABLE : NAL_UNIT_CODED_SLICE_IDR_SCALABLE ) );
  SliceType     eSliceType      = ( auiPredListSize[1] ? B_SLICE : auiPredListSize[0] ? P_SLICE : I_SLICE );
  UInt          uiGOPSize       = uiFrameIdInGOP ? m_uiGOPSize             : 1;
  UInt          uiDecStages     = uiFrameIdInGOP ? m_uiDecompositionStages : 0;



  //===== set simple slice header parameters =====
  pcSliceHeader->setNalRefIdc                   ( eNalRefIdc        );
  pcSliceHeader->setNalUnitType                 ( eNalUnitType      );
  pcSliceHeader->setLayerId                     ( m_uiLayerId       );
  pcSliceHeader->setTemporalLevel               ( uiTemporalLevel   );
  pcSliceHeader->setQualityLevel                ( 0                 );

  pcSliceHeader->setFirstMbInSlice              ( 0                 );
  pcSliceHeader->setLastMbInSlice               ( m_uiMbNumber - 1  );
  pcSliceHeader->setSliceType                   ( eSliceType        );
  pcSliceHeader->setFrameNum                    ( m_uiFrameNum      );
  pcSliceHeader->setIdrPicId                    ( 0                 );
  pcSliceHeader->setDirectSpatialMvPredFlag     ( true              );
  pcSliceHeader->setGOPId                       ( m_uiGOPId         );
  pcSliceHeader->setGOPSize                     ( uiGOPSize         );
  pcSliceHeader->setDecompositionStages         ( uiDecStages       );
  pcSliceHeader->setFrameIdInsideGOP            ( uiFrameIdInGOP    );
  pcSliceHeader->setNumRefIdxActiveOverrideFlag ( false             );
  pcSliceHeader->setCabacInitIdc                ( 0                 );
  pcSliceHeader->setSliceHeaderQp               ( 0                 );

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


  //===== update some parameters =====
  if( eNalRefIdc )
  {
    m_uiFrameNum = ( m_uiFrameNum + 1 ) % ( 1 << pcSliceHeader->getSPS().getLog2MaxFrameNum() );
  }
  if( uiFrameIdInGOP == 0 )
  {
    m_uiGOPId++;
  }

  return Err::m_nOK;
}




ErrVal
MCTFEncoder::xInitReordering ( UInt uiFrameIdInGOP )
{
  ROFRS( m_bH264AVCCompatible, Err::m_nOK );

  SliceHeader*  pcSliceHeader = m_pacControlData[ uiFrameIdInGOP ].getSliceHeader();
  ROF( pcSliceHeader );


  //===== set RPLR and MMCO =====
  if( pcSliceHeader->getTemporalLevel() == 0 )
  {
    //===== low-pass frames =====
    RNOK( xSetRplrAndMmco( *pcSliceHeader ) );
  }
  else
  {
    UIntList cFrameNumList;
    pcSliceHeader->getMmcoBuffer().clear();
    pcSliceHeader->setAdaptiveRefPicBufferingFlag( false );

    if( pcSliceHeader->getSliceType() == B_SLICE )
    {
      if( pcSliceHeader->getNumRefIdxActive( LIST_0 ) > 1 && pcSliceHeader->getNalRefIdc() )
      {
        RNOK( xGetFrameNumList( *pcSliceHeader,                       cFrameNumList, LIST_0, uiFrameIdInGOP ) );
        RNOK( xSetRplr        ( pcSliceHeader->getRplrBuffer(LIST_0), cFrameNumList, pcSliceHeader->getFrameNum() ) );
      }
      else
      {
        pcSliceHeader->getRplrBuffer( LIST_0 ).clear();
        pcSliceHeader->getRplrBuffer( LIST_0 ).setRefPicListReorderingFlag( false );
      }
    }
    else
    {
      ROF( pcSliceHeader->getSliceType() == P_SLICE );

      RNOK( xGetFrameNumList( *pcSliceHeader,                       cFrameNumList, LIST_0, uiFrameIdInGOP ) );
      RNOK( xSetRplr        ( pcSliceHeader->getRplrBuffer(LIST_0), cFrameNumList, pcSliceHeader->getFrameNum() ) );
    }

    pcSliceHeader->getRplrBuffer( LIST_1 ).clear();
    pcSliceHeader->getRplrBuffer( LIST_1 ).setRefPicListReorderingFlag( false );
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

  

  //===== get high-pass scaling and set scaling factors =====
  for( iFrame = 0; iFrame <= iLowPassSize; iFrame++ )
  {
    Double dScal = dScalingBase;

    if( iFrame % 2 )
    {
      //===== high-pass pictures =====
      dScal *= ( adRateBi[iFrame]                    ) * ( FACTOR_53_HP - 1.0 ) +
               ( adRateL0[iFrame] + adRateL1[iFrame] ) * ( FACTOR_22_HP - 1.0 ) + 1.0;
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
  }

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
    for( Int iFrameId = Int( uiFrame + 1 ); iFrameId <= ( m_uiGOPSize >> uiBaseLevel ) && uiList1Size; iFrameId += 2 )
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
    for( Int iFrameId = Int( uiFrame + 1 ); iFrameId <= ( m_uiGOPSize >> uiBaseLevel ) && uiList1Size; iFrameId += 2 )
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
MCTFEncoder::xInitBaseLayerData( ControlData& rcControlData, Bool bMotion )
{
  //===== init =====
  rcControlData.setBaseLayerRec ( 0 );
  rcControlData.setBaseLayerSbb ( 0 );
  rcControlData.setBaseLayerCtrl( 0 );

  
  IntFrame*   pcBaseFrame         = 0;
  IntFrame*   pcBaseResidual      = 0;
  MbDataCtrl* pcBaseDataCtrl      = 0;
  Bool        bForCopyOnly        = false;
  Bool        bBaseDataAvailable  = false;

  if( rcControlData.getBaseLayerIdMotion() != MSYS_UINT_MAX )
  {
    RNOK( m_pcH264AVCEncoder->getBaseLayerData( pcBaseFrame,
                                                pcBaseResidual,
                                                pcBaseDataCtrl,
                                                bForCopyOnly,
                                                rcControlData.isHalfResolutionBaseLayer (),
                                                rcControlData.getBaseLayerIdMotion      (),
                                                rcControlData.getSliceHeader()->getPoc  (), bMotion ) );
    bBaseDataAvailable = pcBaseFrame && pcBaseResidual && pcBaseDataCtrl;
  }


  //===== motion data =====
  if( pcBaseDataCtrl )
  {
    RNOK( m_pcBaseLayerCtrl->initSlice( *rcControlData.getSliceHeader(), PRE_PROCESS, false, NULL ) );
    if( ! rcControlData.isHalfResolutionBaseLayer() )
    {
      if( bForCopyOnly )    
      {
        RNOK( m_pcBaseLayerCtrl->copyMotion  ( *pcBaseDataCtrl ) );
      }
      else
      {
        RNOK( m_pcBaseLayerCtrl->copyMotionBL( *pcBaseDataCtrl ) );
      }
    }
    else
    {
      ROT( bForCopyOnly );
      RNOK( m_pcBaseLayerCtrl->upsampleMotion( *pcBaseDataCtrl ) );
    }
    rcControlData.setBaseLayerCtrl( m_pcBaseLayerCtrl );
  }


  //===== residual data =====
  if( bBaseDataAvailable )
  {
    // if RECONSTRUCTION BYPASS ?????  ===> zero intra macroblocks ?????

    RNOK( m_pcBaseLayerResidual->copy( pcBaseResidual ) );
    if( rcControlData.isHalfResolutionBaseLayer() )
    {
      RNOK( m_pcBaseLayerResidual->upsampleResidual( m_cDownConvert, 1,
                                                     pcBaseDataCtrl, false ) );
    }
    rcControlData.setBaseLayerSbb( m_pcBaseLayerResidual );
  }


  //==== reconstructed (intra) data =====
  if( bBaseDataAvailable )
  {
    RNOK( m_pcBaseLayerFrame->copy( pcBaseFrame ) );
    if( rcControlData.isHalfResolutionBaseLayer() )
    {
      RNOK( m_pcBaseLayerFrame->upsample( m_cDownConvert, 1, true ) );
    }
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
  if( ! m_bH264AVCCompatible )
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
    RNOK( xInitBaseLayerData( rcControlData, true ) );
  
    RNOK( pcMbDataCtrl->initSlice( *pcSliceHeader, ENCODE_PROCESS, false, pcMbDataCtrlL1 ) );
  }

  return Err::m_nOK;
}


ErrVal
MCTFEncoder::xInitControlDataLowPass( UInt uiFrameIdInGOP )
{
  ControlData&  rcControlData   = m_pacControlData[ uiFrameIdInGOP ];
  SliceHeader*  pcSliceHeader   = rcControlData.getSliceHeader();
  
  Double        dScalFactor     = rcControlData.getScalingFactor();
  Double        dQP             = m_dBaseQPResidual - 6.0 * log10( dScalFactor ) / log10( 2.0 );
  Double        dLambda         = 0.85 * pow( 2.0, min( 52.0, dQP ) / 3.0 - 4.0 );
  Int           iQP             = max( MIN_QP, min( MAX_QP, (Int)floor( dQP + 0.5 ) ) );

  pcSliceHeader->setSliceHeaderQp ( iQP );
  rcControlData. setLambda        ( dLambda );

  RNOK( xInitBaseLayerData( rcControlData ) );

  return Err::m_nOK;
}


ErrVal
MCTFEncoder::xInitControlDataHighPass( UInt uiFrameIdInGOP )
{
  ControlData&  rcControlData   = m_pacControlData[ uiFrameIdInGOP ];
  SliceHeader*  pcSliceHeader   = rcControlData.getSliceHeader();
  
  Double        dScalFactor     = rcControlData.getScalingFactor();
  Double        dQP             = m_dBaseQPResidual - 6.0 * log10( dScalFactor ) / log10( 2.0 );
  Double        dLambda         = 0.85 * pow( 2.0, min( 52.0, dQP ) / 3.0 - 4.0 );
  Int           iQP             = max( MIN_QP, min( MAX_QP, (Int)floor( dQP + 0.5 ) ) );

  pcSliceHeader->setSliceHeaderQp ( iQP );
  rcControlData. setLambda        ( dLambda );

  RNOK( xInitBaseLayerData( rcControlData ) );

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
    RefFrameList  acRefFrameList[2];
    RNOK( xGetPredictionLists   ( acRefFrameList[0], acRefFrameList[1], uiBaseLevel, uiFrame, true ) );

    //===== set lambda and QP =====
    RNOK( xInitControlDataMotion( uiBaseLevel, uiFrame, true ) );

    //===== motion estimation =====
    RNOK( xMotionEstimation     ( &acRefFrameList[0], &acRefFrameList[1],
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

    //===== get reference frame lists =====
    RefFrameList  acRefFrameList[2];
    RNOK( xGetPredictionLists   ( acRefFrameList[0], acRefFrameList[1], uiBaseLevel, uiFramePrd, false ) );

    //===== set lambda and QP =====
    RNOK( xInitControlDataMotion( uiBaseLevel, uiFramePrd, false ) );

    //===== prediction =====
    RNOK( xMotionCompensation   ( pcMCFrame, &acRefFrameList[0], &acRefFrameList[1],
                                  rcControlData.getMbDataCtrl(), *rcControlData.getSliceHeader() ) );
    RNOK( pcFrame ->prediction  ( pcMCFrame, pcFrame ) );
    pcFrame       ->setBandType ( HPS );

    //===== set residual =====
    RNOK( pcResidual->copy      ( pcFrame ) );
    RNOK( xZeroIntraMacroblocks ( pcResidual, rcControlData ) );
  }
  if( bPrediction ) printf("\n");


  //===== clear half-pel buffers and buffer extension status =====
  RNOK( xClearBufferExtensions() );




  //===== UPDATE =====
  Bool      bUpdate    = false;
  for( UInt uiFrameUpd = 2; uiFrameUpd <= ( m_uiGOPSize >> uiBaseLevel ); uiFrameUpd += 2 )
  {
    bUpdate                       = true;
    UInt          uiFrameIdInGOP  = uiFrameUpd << uiBaseLevel;
    ControlData&  rcControlData   = m_pacControlData[uiFrameIdInGOP];
    SliceHeader*  pcSliceHeader   = rcControlData     .getSliceHeader ();
    MbDataCtrl*   pcMbDataCtrl    = m_cControlDataUpd .getMbDataCtrl  ();
    m_cControlDataUpd.setSliceHeader( pcSliceHeader );
    IntFrame*     pcMCFrame0      = m_apcFrameTemp  [0];
    IntFrame*     pcMCFrame1      = m_apcFrameTemp  [1];
    IntFrame*     pcFrame         = m_papcFrame     [uiFrameIdInGOP];


    //===== get reference lists =====
    RefFrameList  acRefFrameList[2];
    CtrlDataList  acCtrlDataList[2];
    RNOK( xGetUpdateLists       ( acRefFrameList[0], acRefFrameList[1],
                                  acCtrlDataList[0], acCtrlDataList[1], uiBaseLevel, uiFrameUpd ) );

    if( uiFrameUpd == 2 ) printf("              -");
    printSpaces( ( 1 << ( uiBaseLevel + 1 ) ) - 1 );
    printf( acRefFrameList[0].getActive() || acRefFrameList[1].getActive() ? "U" : "-" );


    //===== list 0 motion compensation =====
    RNOK( pcMbDataCtrl->reset() );
    RNOK( pcMbDataCtrl->clear() );
    RNOK( pcMbDataCtrl->deriveUpdateMotionFieldAdaptive ( *pcSliceHeader,
                                                          &acCtrlDataList[0],
                                                          m_cConnectionArray,
                                                          m_pusUpdateWeights,
                                                          false, LIST_0 ) );
    m_pcQuarterPelFilter->setClipMode                   ( false );
    RNOK( xMotionCompensation                           ( pcMCFrame0,
                                                          &acRefFrameList[0],
                                                          &acRefFrameList[1],
                                                          pcMbDataCtrl,
                                                          *pcSliceHeader ) );
    m_pcQuarterPelFilter->setClipMode                   ( true );
    RNOK( pcMCFrame0->adaptiveWeighting                 ( m_pusUpdateWeights ) );

    
    //===== list 1 motion compensation =====
    RNOK( pcMbDataCtrl->reset() );
    RNOK( pcMbDataCtrl->clear() );
    RNOK( pcMbDataCtrl->deriveUpdateMotionFieldAdaptive ( *pcSliceHeader,
                                                          &acCtrlDataList[1],
                                                          m_cConnectionArray,
                                                          m_pusUpdateWeights,
                                                          false, LIST_1 ) );
    m_pcQuarterPelFilter->setClipMode                   ( false );
    RNOK( xMotionCompensation                           ( pcMCFrame1,
                                                          &acRefFrameList[0],
                                                          &acRefFrameList[1],
                                                          pcMbDataCtrl,
                                                          *pcSliceHeader ) );
    m_pcQuarterPelFilter->setClipMode                   ( true );
    RNOK( pcMCFrame1->adaptiveWeighting                 ( m_pusUpdateWeights ) );


    //===== update =====
    RNOK( pcFrame->update                               ( pcMCFrame0, pcMCFrame1, pcFrame ) );


    //----- clear slice header reference -----
    m_cControlDataUpd.setSliceHeader( NULL );
  }
  if( bUpdate ) printf("\n");

  //===== clear half-pel buffers and buffer extension status =====
  RNOK( xClearBufferExtensions() );


  return Err::m_nOK;
}





ErrVal
MCTFEncoder::xCompositionStage( UInt uiBaseLevel, PicBufferList& rcPicBufferInputList )
{
#if DEBUG_OUTPUT
  if( uiBaseLevel + 1 == m_uiDecompositionStages )
  {
    RNOK( xDumpFrames( "enc_rec", false, uiBaseLevel + 1 ) );
  }
#endif

  //===== UPDATE =====
  Bool      bUpdate    = false;
  for( UInt uiFrameUpd = 2; uiFrameUpd <= ( m_uiGOPSize >> uiBaseLevel ); uiFrameUpd += 2 )
  {
    bUpdate                       = true;
    UInt          uiFrameIdInGOP  = uiFrameUpd << uiBaseLevel;
    ControlData&  rcControlData   = m_pacControlData[uiFrameIdInGOP];
    SliceHeader*  pcSliceHeader   = rcControlData     .getSliceHeader ();
    MbDataCtrl*   pcMbDataCtrl    = m_cControlDataUpd .getMbDataCtrl  ();
    m_cControlDataUpd.setSliceHeader( pcSliceHeader );
    IntFrame*     pcMCFrame0      = m_apcFrameTemp  [0];
    IntFrame*     pcMCFrame1      = m_apcFrameTemp  [1];
    IntFrame*     pcFrame         = m_papcFrame     [uiFrameIdInGOP];


    //===== get reference lists =====
    RefFrameList  acRefFrameList[2];
    CtrlDataList  acCtrlDataList[2];
    RNOK( xGetUpdateLists       ( acRefFrameList[0], acRefFrameList[1],
                                  acCtrlDataList[0], acCtrlDataList[1], uiBaseLevel, uiFrameUpd ) );

    if( uiFrameUpd == 2 ) printf("              -");
    printSpaces( ( 1 << ( uiBaseLevel + 1 ) ) - 1 );
    printf( acRefFrameList[0].getActive() || acRefFrameList[1].getActive() ? "U" : "-" );


    //===== list 0 motion compensation =====
    RNOK( pcMbDataCtrl->reset() );
    RNOK( pcMbDataCtrl->clear() );
    RNOK( pcMbDataCtrl->deriveUpdateMotionFieldAdaptive ( *pcSliceHeader,
                                                          &acCtrlDataList[0],
                                                          m_cConnectionArray,
                                                          m_pusUpdateWeights,
                                                          false, LIST_0 ) );
    m_pcQuarterPelFilter->setClipMode                   ( false );
    RNOK( xMotionCompensation                           ( pcMCFrame0,
                                                          &acRefFrameList[0],
                                                          &acRefFrameList[1],
                                                          pcMbDataCtrl,
                                                          *pcSliceHeader ) );
    m_pcQuarterPelFilter->setClipMode                   ( true );
    RNOK( pcMCFrame0->adaptiveWeighting                 ( m_pusUpdateWeights ) );

    
    //===== list 1 motion compensation =====
    RNOK( pcMbDataCtrl->reset() );
    RNOK( pcMbDataCtrl->clear() );
    RNOK( pcMbDataCtrl->deriveUpdateMotionFieldAdaptive ( *pcSliceHeader,
                                                          &acCtrlDataList[1],
                                                          m_cConnectionArray,
                                                          m_pusUpdateWeights,
                                                          false, LIST_1 ) );
    m_pcQuarterPelFilter->setClipMode                   ( false );
    RNOK( xMotionCompensation                           ( pcMCFrame1,
                                                          &acRefFrameList[0],
                                                          &acRefFrameList[1],
                                                          pcMbDataCtrl,
                                                          *pcSliceHeader ) );
    m_pcQuarterPelFilter->setClipMode                   ( true );
    RNOK( pcMCFrame1->adaptiveWeighting                 ( m_pusUpdateWeights ) );


    //===== inverse update =====
    RNOK( pcFrame->inverseUpdate                        ( pcMCFrame0, pcMCFrame1, pcFrame ) );


    //----- clear slice header reference -----
    m_cControlDataUpd.setSliceHeader( NULL );
  }
  if( bUpdate ) printf("\n");

  //===== clear half-pel buffers and buffer extension status =====
  RNOK( xClearBufferExtensions() );



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
    //IntFrame*     pcResidual      = m_papcResidual  [uiFrameIdInGOP];
    IntFrame*     pcMCFrame       = m_apcFrameTemp  [0];

    //===== get reference frame lists =====
    RefFrameList  acRefFrameList[2];
    RNOK( xGetPredictionLists         ( acRefFrameList[0], acRefFrameList[1], uiBaseLevel, uiFramePrd, false ) );

    //===== prediction =====
    RNOK( xMotionCompensation         ( pcMCFrame, &acRefFrameList[0], &acRefFrameList[1],
                                        rcControlData.getMbDataCtrl(), *rcControlData.getSliceHeader() ) );
    RNOK( pcFrame ->inversePrediction ( pcMCFrame, pcFrame ) );
    //pcFrame       ->clip              ();
    pcFrame       ->setBandType       ( LPS );

    //===== de-blocking =====
    RNOK( m_pcLoopFilter->process     ( *rcControlData.getSliceHeader(),
                                         pcFrame,
                                         rcControlData.getMbDataCtrl (),
                                         rcControlData.getMbDataCtrl (),
                                         m_uiFrameWidthInMb,
                                         &acRefFrameList[0],
                                         &acRefFrameList[1] ) );
  }
  if( bPrediction ) printf("\n");

  //===== clear half-pel buffers and buffer extension status =====
  RNOK( xClearBufferExtensions() );


  RNOK( xCalculateAndAddPSNR( rcPicBufferInputList, m_uiDecompositionStages - uiBaseLevel, uiBaseLevel == m_uiNotCodedMCTFStages ) );


#if DEBUG_OUTPUT
  RNOK( xDumpFrames( "enc_rec", false, uiBaseLevel ) );
#endif


  return Err::m_nOK;
}




ErrVal
MCTFEncoder::xEncodeLowPassPictures( ExtBinDataAccessorList*  pacOutputList )
{
  for( UInt uiFrame = 0; uiFrame <= 1; uiFrame++ )
  {
    UInt                    uiFrameIdInGOP  =   uiFrame << m_uiDecompositionStages;
    ExtBinDataAccessorList& rcOutputList    = ( uiFrame ? pacOutputList[1] : pacOutputList[0] );
    if( uiFrameIdInGOP > m_uiGOPSize )
    {
      continue;
    }


    if( uiFrame )
    {
      //===== store original low-pass for decomposition of following GOP's =====
      RNOK( m_pcAnchorFrameOriginal       ->copyAll( m_papcFrame[ uiFrameIdInGOP ] ) );
    }
    else if( m_uiGOPNumber )
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
    ROT( pcFrame   ->getRecLayer() );
    ROT( pcResidual->getRecLayer() );


    //===== initialize =====
    RNOK( xInitControlDataLowPass ( uiFrameIdInGOP ) );

    //===== base layer encoding =====
    RNOK( pcBLRecFrame->copy      ( pcFrame ) );
    RNOK( xEncodeLowPassSignal    ( rcOutputList,
                                    rcControlData,
                                    pcBLRecFrame,
                                    pcResidual,
                                    pcPredSignal,
                                    uiBits ) );
    m_auiCurrGOPBitsBase[ pcSliceHeader->getTemporalLevel() ] += uiBits;
    m_auiNumFramesCoded [ pcSliceHeader->getTemporalLevel() ] ++;


    //===== write FGS info to file =====
    if( m_uiFGSMode == 1 && m_pFGSFile )
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
    RNOK( m_pcLoopFilter->process ( *pcSliceHeader,
                                    pcBLRecFrame,
                                    ( pcSliceHeader->isIntra() ? NULL : pcMbDataCtrl ),
                                    pcMbDataCtrl,
                                    m_uiFrameWidthInMb,
                                    &rcControlData.getDPCMRefFrameList( LIST_0 ),
                                    &rcControlData.getDPCMRefFrameList( LIST_1 ) ) );

    //----- store for prediction of following low-pass pictures -----
    RNOK( m_pcLowPassBaseReconstruction ->copy( pcBLRecFrame ) );



    //===== FGS enhancement layers =====
    if( m_dNumFGSLayers == 0.0 )
    {
      RNOK( pcFrame->copy   ( pcBLRecFrame ) );
    }
    else
    {
      //----- encode FGS enhancement -----
      uiBits = 0;
      RNOK( xEncodeFGSLayer ( rcOutputList,
                              rcControlData,
                              pcFrame,
                              pcResidual,
                              pcPredSignal,
                              pcBLRecFrame,
                              uiBits ) );
      m_auiCurrGOPBitsFGS[ pcSliceHeader->getTemporalLevel() ] += uiBits;

      //----- store for inter-layer prediction (non-deblocked version) -----
      RNOK( m_papcSubband[uiFrameIdInGOP] ->copy( pcFrame ) );

      //----- de-blocking -----
      RNOK( m_pcLoopFilter->process ( *pcSliceHeader,
                                      pcFrame,
                                      ( pcSliceHeader->isIntra() ? NULL : pcMbDataCtrl ),
                                      pcMbDataCtrl,
                                      m_uiFrameWidthInMb,
                                      &rcControlData.getDPCMRefFrameList( LIST_0 ),
                                      &rcControlData.getDPCMRefFrameList( LIST_1 ) ) );
    }


    //===== set reconstructed =====
    pcFrame   ->addRecLayer();
    pcResidual->addRecLayer();

    
    if( m_uiFGSMode == 1 && m_pFGSFile )
    {
      fprintf( m_pFGSFile, "\n" );
    }
  }

  return Err::m_nOK;
}





ErrVal
MCTFEncoder::xEncodeHighPassPictures( ExtBinDataAccessorList* pacOutputList,
                                      UInt                    uiBaseLevel )
{
  ROFRS( m_uiNotCodedMCTFStages <= uiBaseLevel, Err::m_nOK );  // does not belong to this layer


  for( UInt uiFrame = 1; uiFrame <= ( m_uiGOPSize >> uiBaseLevel ); uiFrame += 2 )
  {
    UInt          uiFrameIdInGOP  = uiFrame << uiBaseLevel;
    UInt          uiBits          = 0;
    UInt          uiBitsRes       = 0;
    IntFrame*     pcFrame         = m_papcFrame     [uiFrameIdInGOP];
    IntFrame*     pcResidual      = m_papcResidual  [uiFrameIdInGOP];
    IntFrame*     pcPredSignal    = m_apcFrameTemp  [0];
    IntFrame*     pcBLRecFrame    = m_apcFrameTemp  [1];
    ControlData&  rcControlData   = m_pacControlData[uiFrameIdInGOP];
    SliceHeader*  pcSliceHeader   = rcControlData.getSliceHeader();
    

    RNOK( xInitControlDataHighPass( uiFrameIdInGOP ) );


    //===== base layer encoding =====
    RNOK( pcBLRecFrame->copy      ( pcFrame ) );
    RNOK( xEncodeHighPassSignal   ( pacOutputList[1],
                                    rcControlData,
                                    pcBLRecFrame,
                                    pcResidual,
                                    pcPredSignal,
                                    uiBits, uiBitsRes ) );
    m_auiCurrGOPBitsBase[ pcSliceHeader->getTemporalLevel() ] += uiBits;
    m_auiNumFramesCoded [ pcSliceHeader->getTemporalLevel() ] ++;


    //===== save FGS info =====
    if( m_uiFGSMode == 1 && m_pFGSFile )
    {
      fprintf( m_pFGSFile, "%d", uiBits + m_uiNotYetConsideredBaseLayerBits );
      m_uiNotYetConsideredBaseLayerBits = 0;
    }


    //===== FGS enhancement ====
    if( m_dNumFGSLayers == 0.0 )
    {
      RNOK( pcFrame->copy         ( pcBLRecFrame ) );
    }
    else
    {
      uiBits = 0;
      RNOK( xEncodeFGSLayer       ( pacOutputList[1],
                                    rcControlData,
                                    pcFrame,
                                    pcResidual,
                                    pcPredSignal,
                                    pcBLRecFrame,
                                    uiBits ) );
      m_auiCurrGOPBitsFGS[ pcSliceHeader->getTemporalLevel() ] += uiBits;
    }


    //----- set reconstruction -----
    pcFrame   ->addRecLayer();
    pcResidual->addRecLayer();

    RNOK( m_papcSubband[uiFrameIdInGOP]->copy( pcFrame ) );

    if( m_uiFGSMode == 1 && m_pFGSFile )
    {
      fprintf( m_pFGSFile, "\n" );
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
    RNOK( m_papcFrame[uiIndex<<m_uiNotCodedMCTFStages]->store( pcPicBuffer ) );
  }
  return Err::m_nOK;
}




ErrVal
MCTFEncoder::xDumpFrames( Char* pFilename, Bool bAll, UInt uiStage )
{
  Char     acTemp[1000];
  if( uiStage == MSYS_UINT_MAX )  sprintf( acTemp, "%s_L%d.yuv",      pFilename, m_uiLayerId );
  else                            sprintf( acTemp, "%s_L%d_S%d.yuv",  pFilename, m_uiLayerId, uiStage );
  FILE*    pFile = ::fopen( acTemp, m_bFirstGOPCoded ? "ab" : "wb" );
  ROF    ( pFile );
  
  for( UInt uiFrame = ( m_bFirstGOPCoded ? 1 : 0 ); uiFrame <= m_uiGOPSize; uiFrame++ )
  {
    if( bAll || m_pacControlData[ uiFrame ].getSliceHeader()->getTemporalLevel() <= m_uiDecompositionStages - m_uiNotCodedMCTFStages )
    {
      RNOK( m_papcFrame[ uiFrame ]->dump( pFile, m_pacControlData[ uiFrame ].getMbDataCtrl() ) );
    }
  }

  ::fclose( pFile );
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
MCTFEncoder::process( ExtBinDataAccessorList*  pacOutExtBinDataAccessorList,
                      ExtBinDataAccessorList&  rcUnusedExtBinDataAccessorList,
                      PicBufferList&           rcPicBufferInputList,
                      PicBufferList&           rcPicBufferOutputList,
                      PicBufferList&           rcPicBufferUnusedList )
{
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
    for( iLevel = 0; iLevel < m_uiDecompositionStages; iLevel++ )
    {
      RNOK( xMotionEstimationStage  ( iLevel ) );
      RNOK( xDecompositionStage     ( iLevel ) );
      RNOK( xSetScalingFactorsMCTF  ( iLevel ) );
    }

    printf("\nCODING:\n");

    //===== encode low-pass pictures =====
    RNOK( xEncodeLowPassPictures    ( pacOutExtBinDataAccessorList ) );
  }
  else
  {
    //===== set all scaling factors =====
    RNOK( xSetScalingFactorsAVC     () );

    //===== encode low-pass pictures =====
    RNOK( xStoreOriginalPictures    () );
    printf("\nLOW-PASS CODING:\n");
    RNOK( xEncodeLowPassPictures    ( pacOutExtBinDataAccessorList ) );
    RNOK( xSwitchOriginalPictures   () );

    printf("\nDECOMPOSITION:\n");

    //===== motion estimation =====
    for( iLevel = m_uiDecompositionStages - 1; iLevel >= (Int)m_uiNotCodedMCTFStages; iLevel-- )
    {
      RNOK( xMotionEstimationStage  ( iLevel ) );
    }

    //===== decomposition =====
    for( iLevel = m_uiNotCodedMCTFStages; iLevel < m_uiDecompositionStages; iLevel++ )
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
    RNOK( xEncodeHighPassPictures   ( pacOutExtBinDataAccessorList, iLevel ) );
  }

 

  //===== store anchor frame =====
  if( m_uiGOPSize == ( 1 << m_uiDecompositionStages ) )
  {
    RNOK( m_pcAnchorFrameReconstructed->copyAll( m_papcFrame[ m_uiGOPSize ] ) );
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


  //===== finish GOP =====
  RNOK( xStoreReconstruction( rcPicBufferOutputList ) );
  RNOK( xFinishGOP          ( rcPicBufferInputList,
                              rcPicBufferOutputList,
                              rcPicBufferUnusedList ) );

  return Err::m_nOK;
}



ErrVal
MCTFEncoder::xFinishGOP( PicBufferList& rcPicBufferInputList,
                         PicBufferList& rcPicBufferOutputList,
                         PicBufferList& rcPicBufferUnusedList )
{
  UInt  uiLowPassSize = m_uiGOPSize >> m_uiNotCodedMCTFStages;

  while( rcPicBufferOutputList.size() > uiLowPassSize + ( m_bFirstGOPCoded ? 0 : 1 ) )
  {
    PicBuffer*  pcPicBuffer = rcPicBufferOutputList.popBack();
    rcPicBufferUnusedList.push_back( pcPicBuffer );
  }
  
  //===== update bit counts etc. =====
  for( UInt uiLevel = 0; uiLevel <= MAX_DSTAGES; uiLevel++ )
  {
    m_adSeqBitsBase[uiLevel] += (Double)m_auiCurrGOPBitsBase[uiLevel];
    m_adSeqBitsFGS [uiLevel] += (Double)m_auiCurrGOPBitsFGS [uiLevel];
  }

  //===== update parameters =====
  m_uiParameterSetBits  = 0;
  m_bFirstGOPCoded      = true;
  m_uiGOPId             = ( m_uiGOPId + 1 ) % ( 1 << LOG2_GOP_ID_WRAP );
  m_uiGOPNumber        ++;

  return Err::m_nOK;
}






ErrVal
MCTFEncoder::xCalculateAndAddPSNR( PicBufferList& rcPicBufferInputList,
                                   UInt           uiStage,
                                   Bool           bOutput )
{
  ROT ( uiStage > m_uiDecompositionStages  );
  RNOK( m_pcYuvFullPelBufferCtrl->initMb() );

  if( bOutput ) printf("\n");

  const YuvBufferCtrl::YuvBufferParameter&  cBufferParam  = m_pcYuvFullPelBufferCtrl->getBufferParameter();
  UInt                                      uiSkip        = 1 << ( m_uiDecompositionStages - uiStage );
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

      Float fRefValueY  = 255.0 * 255.0 * 16.0 * 16.0 * (Float)m_uiMbNumber;
      Float fRefValueC  = fRefValueY / 4.0;
      dYPSNR            = 10.0 * log10( fRefValueY / (Float)uiSSDY );
      dUPSNR            = 10.0 * log10( fRefValueC / (Float)uiSSDU );
      dVPSNR            = 10.0 * log10( fRefValueC / (Float)uiSSDV );
    }

    //===== add PSNR =====
    m_adPSNRSumY[ uiStage ] += dYPSNR;
    m_adPSNRSumU[ uiStage ] += dUPSNR;
    m_adPSNRSumV[ uiStage ] += dUPSNR;

    //===== output PSNR =====
    if( bOutput )
    {
      printf( "  Frame%5d:    Y %6.3f dB    U %6.3f dB    V %6.3f dB\n", iPoc, dYPSNR, dUPSNR, dUPSNR );
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
                     Double&  rdOutputRate)
{
  ROFRS( m_auiNumFramesCoded[0], Err::m_nOK );

  UInt  uiStage;
  UInt  uiMaxStage        = m_uiDecompositionStages - m_uiNotCodedMCTFStages;
  // Bug fix: yiliang.bao@nokia.com
  // uiMaxStage is unsigned, it has a problem when uiMaxStage == 0,
  // uiMaxStage - 1 will result in a large number
  UInt  uiMinStage        = ( !m_bH264AVCCompatible || m_bWriteSubSequenceSei ? 0 : max( 0, (Int)uiMaxStage - 1 ) );
  //UInt  uiMinStage        = ( !m_bH264AVCCompatible || m_bWriteSubSequenceSei ? 0 : max( 0, uiMaxStage - 1 ) );
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


  if( m_uiLayerId == 0 )
  {
    printf("\n\n\nSUMMARY:\n");
    printf("                     " "   min. rate" "   max. rate" "    Y-PSNR" "    U-PSNR" "    V-PSNR\n" );
    printf("                     " "  ----------" "  ----------" "  --------" "  --------" "  --------\n" );
  }


  for( uiStage = uiMinStage; uiStage <= uiMaxStage; uiStage++ )
  {
    Double  dFps    = m_fOutputFrameRate / (Double)( 1 << ( uiMaxStage - uiStage ) );
    Double  dScale  = dFps / 1000 / (Double)m_auiNumFramesCoded[uiStage];

    printf(" %9s @ %7.4f" "  %10.4f" "  %10.4f" "  %8.4f" "  %8.4f" "  %8.4f" "\n",
      acResolution,
      dFps,
      m_adSeqBitsBase [uiStage] * dScale,
      m_adSeqBitsFGS  [uiStage] * dScale,
      m_adPSNRSumY    [uiStage],
      m_adPSNRSumU    [uiStage],
      m_adPSNRSumV    [uiStage] );
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
  rcSH.getMmcoBuffer().set( uiPos, Mmco( MMCO_END) );
  rcSH.setAdaptiveRefPicBufferingFlag( true );

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

  if( iSum == 0 && ( bPos == ! bNeg ) )
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
MCTFEncoder::xGetFrameNumList( SliceHeader& rcSH, UIntList& rcFrameNumList, ListIdx eLstIdx, UInt uiCurrBasePos )
{
  rcFrameNumList.clear();

  const UInt uiLevel   = rcSH.getTemporalLevel  ();
  const UInt uiMaxSize = rcSH.getNumRefIdxActive( eLstIdx );
  ROF( uiMaxSize );


  if( eLstIdx == LIST_1 )
  {
    for( Int i = uiCurrBasePos+1; i <= m_uiGOPSize; i++ )
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


H264AVC_NAMESPACE_END


