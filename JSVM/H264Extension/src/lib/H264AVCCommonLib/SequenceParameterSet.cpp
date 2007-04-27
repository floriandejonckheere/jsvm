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




#include "H264AVCCommonLib.h"
#include "H264AVCCommonLib/SequenceParameterSet.h"
#include "H264AVCCommonLib/TraceFile.h"
#include <cmath>


H264AVC_NAMESPACE_BEGIN


const SequenceParameterSet::LevelLimit SequenceParameterSet::m_aLevelLimit[52] =
{
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //0
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //1
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //2
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //3
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //4
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //5
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //6
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //7
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //8
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //9
  { 1,   1485,    99,    297*1024,     64,    175,  256, 2, MSYS_UINT_MAX },     //10
  { 1,   3000,   396,    675*1024,    192,    500,  512, 2, MSYS_UINT_MAX },     //11
  { 1,   6000,   396,   1782*1024,    384,   1000,  512, 2, MSYS_UINT_MAX },     //12
  { 1,  11880,   396,   1782*1024,    768,   2000,  512, 2, MSYS_UINT_MAX },     //13
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //14
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //15
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //16
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //17
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //18
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //19
  { 1,  11880,   396,   1782*1024,   2000,   2000,  512, 2, MSYS_UINT_MAX },     //20
  { 1,  19800,   792,   3564*1024,   4000,   4000, 1024, 2, MSYS_UINT_MAX },     //21
  { 1,  20250,  1620,   6075*1024,   4000,   4000, 1024, 2, MSYS_UINT_MAX },     //22
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //23
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //24
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //25
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //26
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //27
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //28
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //29
  { 1,  40500,  1620,   6075*1024,  10000,  10000, 1024, 2, 32 },                //30
  { 1, 108000,  3600,  13500*1024,  14000,  14000, 2048, 4, 16 },                //31
  { 1, 216000,  5120,  15360*1024,  20000,  20000, 2048, 4, 16 },                //32
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //33
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //34
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //35
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //36
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //37
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //38
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //39
  { 1, 245760,  8192,  24576*1024,  20000,  25000, 2048, 4, 16 },                //40
  { 1, 245760,  8192,  24576*1024,  50000,  62500, 2048, 2, 16 },                //41
  { 1, 491520,  8192,  24576*1024,  50000,  62500, 2048, 2, 16 },                //42
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //43
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //44
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //45
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //46
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //47
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //48
  { 0,      0,     0,           0,      0,      0,    0, 0, 0 },                 //49
  { 1, 589824, 22080,  82620*1024, 135000, 135000, 2048, 2, 16 },                //50
  { 1, 983040, 36864, 138240*1024, 240000, 240000, 2048, 2, 16 }                 //51
};





SequenceParameterSet::SequenceParameterSet  ()
: m_bInitDone                               ( false )
, m_eNalUnitType                            ( NAL_UNIT_EXTERNAL )
, m_uiLayerId                               ( 0 )
, m_eProfileIdc                             ( SCALABLE_PROFILE )
, m_bConstrainedSet0Flag                    ( false )
, m_bConstrainedSet1Flag                    ( false )
, m_bConstrainedSet2Flag                    ( false )
, m_bConstrainedSet3Flag                    ( false )
, m_uiLevelIdc                              ( 0 )
, m_uiSeqParameterSetId                     ( MSYS_UINT_MAX )
//, m_bNalUnitExtFlag                         ( true  )  //JVT-S036 lsj
, m_bSeqScalingMatrixPresentFlag            ( false )
, m_uiLog2MaxFrameNum                       ( 0 )
, m_uiPicOrderCntType                       ( 0 )
, m_uiLog2MaxPicOrderCntLsb                 ( 4 )
, m_bDeltaPicOrderAlwaysZeroFlag            ( false )
, m_iOffsetForNonRefPic                     ( 0 )
, m_iOffsetForTopToBottomField              ( 0 )
, m_uiNumRefFramesInPicOrderCntCycle        ( 0 )
, m_uiNumRefFrames                          ( 0 )
, m_bGapsInFrameNumValueAllowedFlag         ( false )
, m_uiFrameWidthInMbs                       ( 0 )
, m_uiFrameHeightInMbs                      ( 0 )
, m_bDirect8x8InferenceFlag                 ( false )
,m_uiExtendedSpatialScalability             ( ESS_NONE ) // TMM_ESS
,m_uiChromaPhaseXPlus1                      ( 0 ) // TMM_ESS
,m_uiChromaPhaseYPlus1                      ( 1 )// TMM_ESS
, m_bFGSInfoPresentFlag                     ( false )
, m_bFGSCycleAlignedFragment                ( false ) 
, m_uiNumFGSVectModes                       ( 1 )
, m_bInterlayerDeblockingPresent            ( 0 )
, m_uiPaff                                  ( 0 )
, m_bFrameMbsOnlyFlag                       ( true )
, m_bMbAdaptiveFrameFieldFlag               ( false )
, m_bRCDOBlockSizes                         ( false )
, m_bRCDOMotionCompensationY                ( false )
, m_bRCDOMotionCompensationC                ( false )
, m_bRCDODeblocking                         ( false )
, m_b4TapMotionCompensationY                ( false )  // V090
, m_bAVCRewriteFlag                         ( false )   // V035
, m_bAVCAdaptiveRewriteFlag                 ( false )
{
	m_auiNumRefIdxUpdateActiveDefault[LIST_0]=1;// VW
	m_auiNumRefIdxUpdateActiveDefault[LIST_1]=1;// VW

  ::memset( m_abFGSCodingMode,     0x00, MAX_NUM_FGS_VECT_MODES*sizeof(Bool)    );
  ::memset( m_auiNumPosVectors,    0x00, MAX_NUM_FGS_VECT_MODES*sizeof(UInt)    );
  ::memset( m_auiPosVect,          0x00, MAX_NUM_FGS_VECT_MODES*16*sizeof(UInt) );

  UInt ui;
  for( ui = 0; ui < MAX_NUM_FGS_VECT_MODES; ui++ )
    m_auiGroupingSize[ ui ] = 1; 
}

SequenceParameterSet::~SequenceParameterSet()
{
}



ErrVal
SequenceParameterSet::create( SequenceParameterSet*& rpcSPS )
{
  rpcSPS = new SequenceParameterSet;
  ROT( NULL == rpcSPS);
  return Err::m_nOK;
}


ErrVal
SequenceParameterSet::destroy()
{
  delete this;
  return Err::m_nOK;
}

SequenceParameterSet& SequenceParameterSet::operator = ( const SequenceParameterSet& rcSPS )
{
	m_eNalUnitType                      = rcSPS.m_eNalUnitType;
  m_uiLayerId                         = rcSPS.m_uiLayerId;
  m_eProfileIdc                       = rcSPS.m_eProfileIdc;
  m_bConstrainedSet0Flag              = rcSPS.m_bConstrainedSet0Flag;
  m_bConstrainedSet1Flag              = rcSPS.m_bConstrainedSet1Flag;
  m_bConstrainedSet2Flag              = rcSPS.m_bConstrainedSet2Flag;
	m_bConstrainedSet3Flag              = rcSPS.m_bConstrainedSet3Flag;
  m_uiLevelIdc                        = rcSPS.m_uiLevelIdc;
  m_uiSeqParameterSetId               = rcSPS.m_uiSeqParameterSetId;
  m_uiLog2MaxFrameNum                 = rcSPS.m_uiLog2MaxFrameNum;
  m_uiPicOrderCntType                 = rcSPS.m_uiPicOrderCntType;
  m_uiLog2MaxPicOrderCntLsb           = rcSPS.m_uiLog2MaxPicOrderCntLsb;
  m_bDeltaPicOrderAlwaysZeroFlag      = rcSPS.m_bDeltaPicOrderAlwaysZeroFlag;
  m_iOffsetForNonRefPic               = rcSPS.m_iOffsetForNonRefPic;
  m_iOffsetForTopToBottomField        = rcSPS.m_iOffsetForTopToBottomField;
  m_uiNumRefFramesInPicOrderCntCycle  = rcSPS.m_uiNumRefFramesInPicOrderCntCycle;
  m_piOffsetForRefFrame               = rcSPS.m_piOffsetForRefFrame;
  m_uiNumRefFrames                    = rcSPS.m_uiNumRefFrames;
  m_bGapsInFrameNumValueAllowedFlag   = rcSPS.m_bGapsInFrameNumValueAllowedFlag;
  m_uiFrameWidthInMbs                 = rcSPS.m_uiFrameWidthInMbs;
  m_uiFrameHeightInMbs                = rcSPS.m_uiFrameHeightInMbs;
  m_uiPaff                            = rcSPS.m_uiPaff;
  m_bFrameMbsOnlyFlag                 = rcSPS.m_bFrameMbsOnlyFlag;
  m_bMbAdaptiveFrameFieldFlag         = rcSPS.m_bMbAdaptiveFrameFieldFlag;
  m_bDirect8x8InferenceFlag           = rcSPS.m_bDirect8x8InferenceFlag;
  m_bSeqScalingMatrixPresentFlag      = rcSPS.m_bSeqScalingMatrixPresentFlag;
  m_uiExtendedSpatialScalability      = rcSPS.m_uiExtendedSpatialScalability;
  m_uiChromaPhaseXPlus1               = rcSPS.m_uiChromaPhaseXPlus1;
  m_uiChromaPhaseYPlus1               = rcSPS.m_uiChromaPhaseYPlus1;
  m_iScaledBaseLeftOffset             = rcSPS.m_iScaledBaseLeftOffset;
  m_iScaledBaseTopOffset              = rcSPS.m_iScaledBaseTopOffset;
  m_iScaledBaseRightOffset            = rcSPS.m_iScaledBaseRightOffset;
  m_iScaledBaseBottomOffset           = rcSPS.m_iScaledBaseBottomOffset;
  m_bInitDone                         = rcSPS.m_bInitDone;

  return *this;
}

UInt
SequenceParameterSet::getMaxDPBSize() const
{
  const LevelLimit* pcLevelLimit = 0;
  UInt              uiFrameSize = 384*getMbInFrame();
  ANOK( xGetLevelLimit( pcLevelLimit, getLevelIdc() ) );
  return pcLevelLimit->uiMaxDPBSizeX2 / ( 2*uiFrameSize );
}


ErrVal
SequenceParameterSet::xGetLevelLimit( const LevelLimit*& rpcLevelLimit, Int iLevelIdc )
{
  ROT ( iLevelIdc > 51 )
  rpcLevelLimit = &m_aLevelLimit[iLevelIdc];
  ROFS( rpcLevelLimit->bValid )
  return Err::m_nOK;
}


UInt
SequenceParameterSet::getLevelIdc( UInt uiMbY, UInt uiMbX, UInt uiOutFreq, UInt uiMvRange, UInt uiNumRefPic )
{
  UInt uiFrameSize = uiMbY * uiMbX;
  UInt uiMbPerSec  = uiFrameSize * uiOutFreq;
  UInt uiDPBSizeX2 = (uiFrameSize*16*16*3/2) * uiNumRefPic * 2;

  for( Int n = 0; n < 52; n++ )
  {
    const LevelLimit* pcLevelLimit;
    
    if( Err::m_nOK == xGetLevelLimit( pcLevelLimit, n ) )
    {
      UInt  uiMbPerLine  = (UInt)sqrt( (Double) pcLevelLimit->uiMaxFrameSize * 8 );
      if( ( uiMbPerLine                   >= uiMbX        ) &&
          ( uiMbPerLine                   >= uiMbY        ) &&
          ( pcLevelLimit->uiMaxMbPerSec   >= uiMbPerSec   ) &&
          ( pcLevelLimit->uiMaxFrameSize  >= uiFrameSize  ) &&
          ( pcLevelLimit->uiMaxDPBSizeX2  >= uiDPBSizeX2  ) &&

          ( pcLevelLimit->uiMaxVMvRange   >= uiMvRange    )    )
      {
        return n;
      }
    }
  }
  return MSYS_UINT_MAX;
}


ErrVal
SequenceParameterSet::write( HeaderSymbolWriteIf* pcWriteIf ) const
{
  //===== NAL unit header =====
  ETRACE_DECLARE( Bool m_bTraceEnable = true );
	g_nLayer = m_uiLayerId;
  ETRACE_LAYER(m_uiLayerId);
  ETRACE_HEADER( "SEQUENCE PARAMETER SET" );
  RNOK  ( pcWriteIf->writeFlag( 0,                                        "NALU HEADER: forbidden_zero_bit" ) );
  RNOK  ( pcWriteIf->writeCode( 3, 2,                                     "NALU HEADER: nal_ref_idc" ) );
  RNOK  ( pcWriteIf->writeCode( m_eNalUnitType, 5,                        "NALU HEADER: nal_unit_type" ) );

  //===== Sequence parameter set =====
  RNOK  ( pcWriteIf->writeCode( getProfileIdc(),                  8,      "SPS: profile_idc" ) );
  RNOK  ( pcWriteIf->writeFlag( m_bConstrainedSet0Flag,                   "SPS: constrained_set0_flag" ) );
  RNOK  ( pcWriteIf->writeFlag( m_bConstrainedSet1Flag,                   "SPS: constrained_set1_flag" ) );
  RNOK  ( pcWriteIf->writeFlag( m_bConstrainedSet2Flag,                   "SPS: constrained_set2_flag" ) );
  RNOK  ( pcWriteIf->writeFlag( m_bConstrainedSet3Flag,                   "SPS: constrained_set3_flag" ) );
  RNOK  ( pcWriteIf->writeCode( 0,                                4,      "SPS: reserved_zero_4bits" ) );
  RNOK  ( pcWriteIf->writeCode( getLevelIdc(),                    8,      "SPS: level_idc" ) );
  RNOK  ( pcWriteIf->writeUvlc( getSeqParameterSetId(),                   "SPS: seq_parameter_set_id" ) );
  
  //--- fidelity range extension syntax ---
  RNOK  ( xWriteFrext( pcWriteIf ) ); //bug fix JV 07/11/06
  
  if( m_eProfileIdc == SCALABLE_PROFILE ) // bug-fix (HS)
  {

    RNOK( pcWriteIf->writeFlag( getInterlayerDeblockingPresent(),       "SPS: interlayer_deblocking_filter_control_present_flag" ) );

    RNOK( pcWriteIf->writeCode( getExtendedSpatialScalability(), 2,     "SPS: extended_spatial_scalability" ) );
//    if ( 1 /* chroma_format_idc */ > 0 )
    {
      RNOK( pcWriteIf->writeCode( m_uiChromaPhaseXPlus1, 2,             "SPS: chroma_phase_x_plus1" ) );
      RNOK( pcWriteIf->writeCode( m_uiChromaPhaseYPlus1, 2,             "SPS: chroma_phase_y_plus1" ) );
    }
    if (getExtendedSpatialScalability() == ESS_SEQ)
    {
      RNOK( pcWriteIf->writeSvlc( m_iScaledBaseLeftOffset,              "SPS: scaled_base_left_offset" ) );
      RNOK( pcWriteIf->writeSvlc( m_iScaledBaseTopOffset,               "SPS: scaled_base_top_offset" ) );
      RNOK( pcWriteIf->writeSvlc( m_iScaledBaseRightOffset,             "SPS: scaled_base_right_offset" ) );
      RNOK( pcWriteIf->writeSvlc( m_iScaledBaseBottomOffset,            "SPS: scaled_base_bottom_offset" ) );
    }

#ifdef _JVTV074_
    Int k, kmin;
    UInt uiResampleFilterIdx;
    Int iResampleFilterParamA, iResampleFilterParamB;


    RNOK( pcWriteIf->writeUvlc(m_uiNumResampleFiltersMinus1,         "SPS: NumResampleFiltersMinus1" ) );
    for (uiResampleFilterIdx = 0; uiResampleFilterIdx <= m_uiNumResampleFiltersMinus1; uiResampleFilterIdx++)
    {
        RNOK( pcWriteIf->writeFlag( m_bIntegerPosFilterPresentFlag[uiResampleFilterIdx],  "SPS: IntegerPosFilterPresentFlag" ) );
        kmin = (!m_bIntegerPosFilterPresentFlag[uiResampleFilterIdx]);
        for (k = 7; k >= kmin; k--)
        {
            if (k == 6)
                iResampleFilterParamA = m_iResampleFilterParamA[uiResampleFilterIdx][6] - m_iResampleFilterParamA[uiResampleFilterIdx][7];
            else if (k < 6)
                iResampleFilterParamA = m_iResampleFilterParamA[uiResampleFilterIdx][k] - 2 * m_iResampleFilterParamA[uiResampleFilterIdx][k+1] + m_iResampleFilterParamA[uiResampleFilterIdx][k+2];
            else //k == 7
                iResampleFilterParamA = m_iResampleFilterParamA[uiResampleFilterIdx][7];
            RNOK( pcWriteIf->writeSvlc( iResampleFilterParamA, "SPS: ResampleFilterParamA" ) );
        }
        for (k = 1; k <= 8; k++) {
            if (k == 2)
                iResampleFilterParamB = m_iResampleFilterParamB[uiResampleFilterIdx][2] - m_iResampleFilterParamB[uiResampleFilterIdx][1];
            else if (k > 2)
                iResampleFilterParamB = m_iResampleFilterParamB[uiResampleFilterIdx][k] - 2 * m_iResampleFilterParamB[uiResampleFilterIdx][k-1] + m_iResampleFilterParamB[uiResampleFilterIdx][k-2];
            else //k == 1;
                iResampleFilterParamB = m_iResampleFilterParamB[uiResampleFilterIdx][1];
            RNOK( pcWriteIf->writeSvlc( iResampleFilterParamB, "SPS: ResampleFilterParamB" ) );
        }

    }
#endif // _JVTV074_

    RNOK  ( pcWriteIf->writeFlag( m_bFGSInfoPresentFlag,                       "SPS: fgs_info_present") );
    if( m_bFGSInfoPresentFlag ) 
    {
      RNOK  ( pcWriteIf->writeFlag( m_bFGSCycleAlignedFragment,                "SPS: fgs_cycle_aligned_fragment") );
      RNOK  ( pcWriteIf->writeUvlc( m_uiNumFGSVectModes-1,                     "SPS: fgs_number_vector_modes") );

      UInt ui;
      for( ui = 0; ui < m_uiNumFGSVectModes; ui++ )
      {
        RNOK  ( pcWriteIf->writeFlag( m_abFGSCodingMode[ui],                   "SPS: fgs_coding_mode") );
        if(m_abFGSCodingMode[ ui ] == false)
        {
          RNOK  ( pcWriteIf->writeUvlc((m_auiGroupingSize[ui]-1),              "SPS: GroupingSizeMinus1") );
        }
        else
        {
          UInt uiIndex = 0;
          UInt uiRemainingVectLen = 16; 
          UInt auiReverseVectLen[16]; 
          
          for( uiIndex = 0; uiIndex < m_auiNumPosVectors[ui]; uiIndex ++ )
            auiReverseVectLen [ m_auiNumPosVectors[ ui ] -1 - uiIndex ] = m_auiPosVect[ui][uiIndex];

          uiIndex = 0; 
          do 
          {
            UInt uiCodeLen = ( uiRemainingVectLen <= 4 ) ? ( ( uiRemainingVectLen <= 2 ) ? 1 : 2 ) : ( uiRemainingVectLen <= 8 ) ? 3 : 4;

            if( uiRemainingVectLen > 1 )
              RNOK( pcWriteIf->writeCode( auiReverseVectLen[uiIndex] - 1, uiCodeLen,  "SPS: ReverseVectLen") );
            uiRemainingVectLen -= auiReverseVectLen [ uiIndex ];
            uiIndex++;
          } while( uiRemainingVectLen > 0 );
        }
      }
    }

    if( getExtendedSpatialScalability() == ESS_NONE )
    {
      RNOK( pcWriteIf->writeFlag( m_bAVCRewriteFlag,                    "SPS: AVC_rewrite_flag" ) );
      if( m_bAVCRewriteFlag )
        RNOK( pcWriteIf->writeFlag( m_bAVCAdaptiveRewriteFlag,        "SPS: AVC_adaptive_rewrite_flag" ) );
    }
  }
  
  UInt    uiTmp = getLog2MaxFrameNum();
  ROF   ( uiTmp >= 4 );
  RNOK  ( pcWriteIf->writeUvlc( uiTmp - 4,                                "SPS: log2_max_frame_num_minus_4" ) );
  RNOK  ( pcWriteIf->writeUvlc( getPicOrderCntType(),                     "SPS: pic_order_cnt_type" ) );
  if( getPicOrderCntType() == 0 )
  {
  RNOK  ( pcWriteIf->writeUvlc( getLog2MaxPicOrderCntLsb() - 4,           "SPS: log2_max_pic_order_cnt_lsb_minus4" ) );
  }
  else if( getPicOrderCntType() == 1 )
  {
    RNOK( pcWriteIf->writeFlag( getDeltaPicOrderAlwaysZeroFlag(),         "SPS: delta_pic_order_always_zero_flag" ) );
    RNOK( pcWriteIf->writeSvlc( getOffsetForNonRefPic(),                  "SPS: offset_for_non_ref_pic" ) );
    RNOK( pcWriteIf->writeSvlc( getOffsetForTopToBottomField(),           "SPS: offset_for_top_to_bottom_field" ) );
    RNOK( pcWriteIf->writeUvlc( getNumRefFramesInPicOrderCntCycle(),      "SPS: num_ref_frames_in_pic_order_cnt_cycle" ) );
    for( UInt uiIndex = 0; uiIndex < getNumRefFramesInPicOrderCntCycle(); uiIndex++ )
    {
      RNOK( pcWriteIf->writeSvlc( getOffsetForRefFrame( uiIndex ),        "SPS: offset_for_ref_frame" ) );
    }
  }
  RNOK  ( pcWriteIf->writeUvlc( getNumRefFrames(),                        "SPS: num_ref_frames" ) );
  RNOK  ( pcWriteIf->writeFlag( getGapsInFrameNumValueAllowedFlag(),      "SPS: gaps_in_frame_num_value_allowed_flag" ) );
  
  RNOK  ( pcWriteIf->writeUvlc( getFrameWidthInMbs  () - 1,               "SPS: pic_width_in_mbs_minus_1" ) );
  UInt uiHeight = ( getFrameHeightInMbs()-1) >> (1- getFrameMbsOnlyFlag() );

  RNOK  ( pcWriteIf->writeUvlc( uiHeight,                                 "SPS: pic_height_in_map_units_minus1" ) );
  RNOK  ( pcWriteIf->writeFlag( getFrameMbsOnlyFlag(),                    "SPS: frame_mbs_only_flag" ) );

  if( !getFrameMbsOnlyFlag() )
  {
    RNOK( pcWriteIf->writeFlag( getMbAdaptiveFrameFieldFlag(),            "SPS: mb_adaptive_frame_field_flag" ) );
  }


  RNOK  ( pcWriteIf->writeFlag( getDirect8x8InferenceFlag(),              "SPS: direct_8x8_inference_flag" ) );
  RNOK  ( pcWriteIf->writeFlag( false,                                    "SPS: frame_cropping_flag" ) );

  RNOK  ( pcWriteIf->writeFlag( false,                                  "SPS: vui_parameters_present_flag" ) );

  Bool bRCDO  = ( m_bRCDOBlockSizes ||
                  m_bRCDOMotionCompensationY ||
                  m_bRCDOMotionCompensationC ||
                  m_bRCDODeblocking );
  if(  bRCDO )
  {
    RNOK( pcWriteIf->writeFlag( m_b4TapMotionCompensationY,  "4TAPMC: 4tap_motion_compensation_y" ) );  // V090
    RNOK( pcWriteIf->writeFlag( m_bRCDOBlockSizes,           "RCDO: rdco_block_sizes"           ) ); // not really required by decoder
    RNOK( pcWriteIf->writeFlag( m_bRCDOMotionCompensationY,  "RCDO: rdco_motion_compensation_y" ) );
    RNOK( pcWriteIf->writeFlag( m_bRCDOMotionCompensationC,  "RCDO: rdco_motion_compensation_c" ) );
    RNOK( pcWriteIf->writeFlag( m_bRCDODeblocking,           "RCDO: rdco_deblocking"            ) );
  }
  else if( m_b4TapMotionCompensationY)  // V090
	  RNOK( pcWriteIf->writeFlag( m_b4TapMotionCompensationY,    "4TAPMC: 4tap_motion_compensation_y" ) );  // V090


  return Err::m_nOK;
}


ErrVal
SequenceParameterSet::read( HeaderSymbolReadIf* pcReadIf,
                            NalUnitType         eNalUnitType )
{
  m_uiExtendedSpatialScalability = ESS_NONE;

  //===== NAL unit header =====
  setNalUnitType    ( eNalUnitType );

  Bool  bTmp;
  UInt  uiTmp;

  //===== Sequence parameter set =====
  RNOK  ( pcReadIf->getCode( uiTmp,                               8,      "SPS: profile_idc" ) );
  m_eProfileIdc  = Profile ( uiTmp );
  ROT   ( m_eProfileIdc != BASELINE_PROFILE &&
          m_eProfileIdc != MAIN_PROFILE  &&
          m_eProfileIdc != EXTENDED_PROFILE  &&
          m_eProfileIdc != HIGH_PROFILE  &&
          m_eProfileIdc != SCALABLE_PROFILE );
  RNOK  ( pcReadIf->getFlag( m_bConstrainedSet0Flag,                      "SPS: constrained_set0_flag" ) );
  RNOK  ( pcReadIf->getFlag( m_bConstrainedSet1Flag,                      "SPS: constrained_set1_flag" ) );
  RNOK  ( pcReadIf->getFlag( m_bConstrainedSet2Flag,                      "SPS: constrained_set2_flag" ) );
  RNOK  ( pcReadIf->getFlag( m_bConstrainedSet3Flag,                      "SPS: constrained_set3_flag" ) );
  RNOK  ( pcReadIf->getCode( uiTmp,                               4,      "SPS: reserved_zero_4bits" ) );
  ROT   ( uiTmp );
  RNOK  ( pcReadIf->getCode( m_uiLevelIdc,                        8,      "SPS: level_idc" ) );
  RNOK  ( pcReadIf->getUvlc( m_uiSeqParameterSetId,                       "SPS: seq_parameter_set_id" ) );

   //--- fidelity range extension syntax ---
  RNOK  ( xReadFrext( pcReadIf ) );//bug fix JV 07/11/06

  if( m_eProfileIdc == SCALABLE_PROFILE ) // bug-fix (HS)
  {

    RNOK( pcReadIf->getFlag( m_bInterlayerDeblockingPresent,              "SPS: interlayer_deblocking_filter_control_present_flag" ) );

    RNOK( pcReadIf->getCode( m_uiExtendedSpatialScalability, 2,           "SPS: extended_spatial_scalability" ) );
//    if ( 1 /* chroma_format_idc */ > 0 )
    {
      RNOK( pcReadIf->getCode( m_uiChromaPhaseXPlus1, 2,                  "SPS: chroma_phase_x_plus1" ) );
      RNOK( pcReadIf->getCode( m_uiChromaPhaseYPlus1, 2,                  "SPS: chroma_phase_y_plus1" ) );
    }
    if (m_uiExtendedSpatialScalability == ESS_SEQ)
    {
      RNOK( pcReadIf->getSvlc( m_iScaledBaseLeftOffset,                   "SPS: scaled_base_left_offset" ) );
      RNOK( pcReadIf->getSvlc( m_iScaledBaseTopOffset,                    "SPS: scaled_base_top_offset" ) );
      RNOK( pcReadIf->getSvlc( m_iScaledBaseRightOffset,                  "SPS: scaled_base_right_offset" ) );
      RNOK( pcReadIf->getSvlc( m_iScaledBaseBottomOffset,                 "SPS: scaled_base_bottom_offset" ) );
    }

#ifdef _JVTV074_
    UInt k, uiResampleFilterIdx, kmin;
    Int iResampleFilterParamA, iResampleFilterParamB; 
	Bool bIntegerPosFilterPresentFlag;


    RNOK( pcReadIf->getUvlc( m_uiNumResampleFiltersMinus1,         "SPS: NumResampleFiltersMinus1" ) );
    for (uiResampleFilterIdx = 0; uiResampleFilterIdx <= m_uiNumResampleFiltersMinus1; uiResampleFilterIdx++)
    {
        RNOK( pcReadIf->getFlag( bIntegerPosFilterPresentFlag,  "SPS: IntegerPosFilterPresentFlag" ) );
        m_bIntegerPosFilterPresentFlag[uiResampleFilterIdx] = bIntegerPosFilterPresentFlag;
        if (!m_bIntegerPosFilterPresentFlag[uiResampleFilterIdx])
        {
            m_iResampleFilterParamA[uiResampleFilterIdx][0] = 0;
        }
        m_iResampleFilterParamA[uiResampleFilterIdx][8] = 0;
        m_iResampleFilterParamB[uiResampleFilterIdx][0] = 0;
        kmin = (!m_bIntegerPosFilterPresentFlag[uiResampleFilterIdx]);
        for (k = 7; k >= kmin; k--)
        {
            RNOK( pcReadIf->getSvlc( iResampleFilterParamA, "SPS: ResampleFilterParamA" ) );
            if (k == 6)
                m_iResampleFilterParamA[uiResampleFilterIdx][6] = iResampleFilterParamA +  m_iResampleFilterParamA[uiResampleFilterIdx][7];
            else if (k < 6)
                m_iResampleFilterParamA[uiResampleFilterIdx][k] = iResampleFilterParamA + 2 * m_iResampleFilterParamA[uiResampleFilterIdx][k+1] - m_iResampleFilterParamA[uiResampleFilterIdx][k+2];
            else //k == 7
                m_iResampleFilterParamA[uiResampleFilterIdx][7] = iResampleFilterParamA;
        }
        for (k = 1; k <= 8; k++) 
        {
            RNOK( pcReadIf->getSvlc( iResampleFilterParamB, "SPS: ResampleFilterParamB" ) );
            if (k == 2)
                m_iResampleFilterParamB[uiResampleFilterIdx][2] = iResampleFilterParamB + m_iResampleFilterParamB[uiResampleFilterIdx][1];
            else if (k > 2)
                m_iResampleFilterParamB[uiResampleFilterIdx][k] = iResampleFilterParamB + 2 * m_iResampleFilterParamB[uiResampleFilterIdx][k-1] - m_iResampleFilterParamB[uiResampleFilterIdx][k-2];
            else //k == 1;
                m_iResampleFilterParamB[uiResampleFilterIdx][1] = iResampleFilterParamB;
        }
    }
#endif // _JVTV074_

    RNOK  ( pcReadIf->getFlag( m_bFGSInfoPresentFlag,                       "SPS: fgs_info_present") );
    if( m_bFGSInfoPresentFlag ) 
    {
      RNOK  ( pcReadIf->getFlag( m_bFGSCycleAlignedFragment,                "SPS: fgs_cycle_aligned_fragment") );
      RNOK  ( pcReadIf->getUvlc( m_uiNumFGSVectModes,                       "SPS: fgs_number_fgs_vector_modes") );
      m_uiNumFGSVectModes++;
      ROF ( m_uiNumFGSVectModes <= MAX_NUM_FGS_VECT_MODES ); 

      UInt ui;
      for( ui = 0; ui < m_uiNumFGSVectModes; ui++ )
      {
        RNOK  ( pcReadIf->getFlag( m_abFGSCodingMode[ui],                   "SPS: fgs_coding_mode") );
        if(m_abFGSCodingMode[ ui ] == false)
        {
          RNOK  ( pcReadIf->getUvlc(m_auiGroupingSize[ui],                  "SPS: GroupingSizeMinus1") );
          m_auiGroupingSize[ ui ]++;
        }
        else
        {
          UInt uiIndex = 0;
          UInt uiRemainingVectLen = 16; 
          UInt auiReverseVectLen[16]; 

          do 
          {
            UInt uiCodeLen = ( uiRemainingVectLen <= 4 ) ? ( ( uiRemainingVectLen <= 2 ) ? 1 : 2 ) : ( uiRemainingVectLen <= 8 ) ? 3 : 4;

            auiReverseVectLen[ uiIndex ] = 0;
            if( uiRemainingVectLen > 1 )
            {
              RNOK( pcReadIf->getCode( auiReverseVectLen[ uiIndex ], uiCodeLen, "SPS: ReverseVectLen") );
            }
            else
              auiReverseVectLen[ uiIndex ] = 0;
            auiReverseVectLen[ uiIndex ] ++;
            uiRemainingVectLen -= auiReverseVectLen [ uiIndex ];
            uiIndex++;
          } while( uiRemainingVectLen > 0 );

          m_auiNumPosVectors[ui] = uiIndex;
          
          for( uiIndex = 0; uiIndex < m_auiNumPosVectors[ui]; uiIndex ++ )
            m_auiPosVect[ui][uiIndex] = auiReverseVectLen [ m_auiNumPosVectors[ ui ] - 1 - uiIndex ];

        }
      }
    }
    else
    {
      m_uiNumFGSVectModes = 0; 
    }

    if( getExtendedSpatialScalability() == ESS_NONE )
    {
      RNOK( pcReadIf->getFlag( m_bAVCRewriteFlag,                       "SPS: AVC_rewrite_flag" ) );
      if( m_bAVCRewriteFlag )
        RNOK( pcReadIf->getFlag( m_bAVCAdaptiveRewriteFlag,           "SPS: AVC_adaptive_rewrite_flag" ) );
    }
  }

  RNOK  ( pcReadIf->getUvlc( uiTmp,                                       "SPS: log2_max_frame_num_minus_4" ) );
  ROT   ( uiTmp > 12 );
  setLog2MaxFrameNum( uiTmp + 4 );
	RNOK  ( xReadPicOrderCntInfo( pcReadIf ) );
  RNOK( pcReadIf->getUvlc( m_uiNumRefFrames,                              "SPS: num_ref_frames" ) );
	RNOK( pcReadIf->getFlag( m_bGapsInFrameNumValueAllowedFlag,             "SPS: gaps_in_frame_num_value_allowed_flag" ) );

	RNOK( pcReadIf->getUvlc( uiTmp,                                         "SPS: pic_width_in_mbs_minus1" ) );
  setFrameWidthInMbs ( 1 + uiTmp );
  RNOK( pcReadIf->getUvlc( uiTmp,                                         "SPS: pic_height_in_map_units_minus1" ) );
  RNOK( pcReadIf->getFlag( m_bFrameMbsOnlyFlag,                           "SPS: frame_mbs_only_flag" ) );
	if( getFrameMbsOnlyFlag() )
  {
    setFrameHeightInMbs( uiTmp+1 );
		setMbAdaptiveFrameFieldFlag( false );
  }
  else
    {
    setFrameHeightInMbs( (uiTmp+1)<<1 );
		RNOK( pcReadIf->getFlag( m_bMbAdaptiveFrameFieldFlag,                 "SPS: mb_adaptive_frame_field_flag"));
    }
  RNOK( pcReadIf->getFlag( m_bDirect8x8InferenceFlag,                     "SPS: direct_8x8_inference_flag" ) );
  RNOK( pcReadIf->getFlag( bTmp,                                          "SPS: frame_cropping_flag" ) );
  ROT ( bTmp );
  
  RNOK( pcReadIf->getFlag( bTmp,                                          "SPS: vui_parameters_present_flag" ) );
  ROT ( bTmp );

  m_b4TapMotionCompensationY = false;                      // V090
  ROFRS( pcReadIf->moreRBSPData(), Err::m_nOK );           // V090
  RNOK ( pcReadIf->getFlag( m_b4TapMotionCompensationY,  "4TAPMC: 4tap_motion_compensation_y" ) );   // V090

  m_bRCDOBlockSizes          = false;
  m_bRCDOMotionCompensationY = false;
  m_bRCDOMotionCompensationC = false;
  m_bRCDODeblocking          = false;
  ROFRS( pcReadIf->moreRBSPData(), Err::m_nOK );
  RNOK ( pcReadIf->getFlag( m_bRCDOBlockSizes,           "RCDO: rdco_block_sizes"           ) ); // not really required by decoder
  RNOK ( pcReadIf->getFlag( m_bRCDOMotionCompensationY,  "RCDO: rdco_motion_compensation_y" ) );
  RNOK ( pcReadIf->getFlag( m_bRCDOMotionCompensationC,  "RCDO: rdco_motion_compensation_c" ) );
  RNOK ( pcReadIf->getFlag( m_bRCDODeblocking,           "RCDO: rdco_deblocking"            ) );

  return Err::m_nOK;
}


ErrVal
SequenceParameterSet::xWriteFrext( HeaderSymbolWriteIf* pcWriteIf ) const
{
  ROTRS( m_eProfileIdc != HIGH_PROFILE      &&
         m_eProfileIdc != HIGH_10_PROFILE   &&
         m_eProfileIdc != HIGH_422_PROFILE  &&
         m_eProfileIdc != HIGH_444_PROFILE  &&
         m_eProfileIdc != SCALABLE_PROFILE, Err::m_nOK );

  RNOK  ( pcWriteIf->writeUvlc( 1,                              "SPS: chroma_format_idc" ) );
  RNOK  ( pcWriteIf->writeUvlc( 0,                              "SPS: bit_depth_luma_minus8" ) );
  RNOK  ( pcWriteIf->writeUvlc( 0,                              "SPS: bit_depth_chroma_minus8" ) );
  RNOK  ( pcWriteIf->writeFlag( false,                          "SPS: qpprime_y_zero_transform_bypass_flag" ) );
  RNOK  ( pcWriteIf->writeFlag( m_bSeqScalingMatrixPresentFlag, "SPS: seq_scaling_matrix_present_flag"  ) );
  
  ROTRS ( ! m_bSeqScalingMatrixPresentFlag, Err::m_nOK );
  RNOK  ( m_cSeqScalingMatrix.write( pcWriteIf, true ) );

  return Err::m_nOK;
}


ErrVal
SequenceParameterSet::xReadFrext( HeaderSymbolReadIf* pcReadIf )
{
  ROTRS( m_eProfileIdc != HIGH_PROFILE      &&
         m_eProfileIdc != HIGH_10_PROFILE   &&
         m_eProfileIdc != HIGH_422_PROFILE  &&
         m_eProfileIdc != HIGH_444_PROFILE  &&
         m_eProfileIdc != SCALABLE_PROFILE, Err::m_nOK );

  UInt  uiTmp;
  Bool  bTmp;
  RNOK( pcReadIf->getUvlc( uiTmp,                               "SPS: chroma_format_idc" ) );
  ROF ( uiTmp == 1 );
  RNOK( pcReadIf->getUvlc( uiTmp,                               "SPS: bit_depth_luma_minus8" ) );
  ROF ( uiTmp == 0 );
  RNOK( pcReadIf->getUvlc( uiTmp,                               "SPS: bit_depth_chroma_minus8" ) );
  ROF ( uiTmp == 0 );
  RNOK( pcReadIf->getFlag( bTmp,                                "SPS: qpprime_y_zero_transform_bypass_flag" ) );
  ROT ( bTmp )
  RNOK( pcReadIf->getFlag( m_bSeqScalingMatrixPresentFlag,      "SPS: seq_scaling_matrix_present_flag") );
  
  ROTRS ( ! m_bSeqScalingMatrixPresentFlag, Err::m_nOK );
  RNOK  ( m_cSeqScalingMatrix.read( pcReadIf, true ) );

  return Err::m_nOK;
}


// TMM_ESS {
Void SequenceParameterSet::setResizeParameters ( const ResizeParameters * params )
{
  m_uiExtendedSpatialScalability = (UInt)params->m_iExtendedSpatialScalability;

  m_uiChromaPhaseXPlus1 = (UInt)(params->m_iChromaPhaseX+1);
  m_uiChromaPhaseYPlus1 = (UInt)(params->m_iChromaPhaseY+1);

  if (m_uiExtendedSpatialScalability == ESS_SEQ)
  {
    m_iScaledBaseLeftOffset   = params->m_iPosX /2;
    m_iScaledBaseTopOffset    = params->m_iPosY /2;
    m_iScaledBaseRightOffset  = (params->m_iGlobWidth - params->m_iPosX - params->m_iOutWidth) /2;
    m_iScaledBaseBottomOffset = (params->m_iGlobHeight - params->m_iPosY - params->m_iOutHeight) /2;
  }
  else
  {
    m_iScaledBaseBottomOffset = 0;
    m_iScaledBaseLeftOffset = 0;
    m_iScaledBaseRightOffset = 0;
    m_iScaledBaseTopOffset = 0;
  }
#ifdef _JVTV074_
  m_uiNumResampleFiltersMinus1 = params->m_uiNumResampleFiltersMinus1;
  Int k;
  UInt uiResampleFilterIdx;
  for (uiResampleFilterIdx = 0; uiResampleFilterIdx <= m_uiNumResampleFiltersMinus1; uiResampleFilterIdx++)
  {
      m_bIntegerPosFilterPresentFlag[uiResampleFilterIdx] = params->m_bIntegerPosFilterPresentFlag[uiResampleFilterIdx];
      for (k = 0; k < 9; k++) 
      {
          m_iResampleFilterParamA[uiResampleFilterIdx][k] = params->m_iResampleFilterParamA[uiResampleFilterIdx][k];
          m_iResampleFilterParamB[uiResampleFilterIdx][k] = params->m_iResampleFilterParamB[uiResampleFilterIdx][k];
      }
  }
#endif // _JVTV074_
 }

Void SequenceParameterSet::getResizeParameters ( ResizeParameters * params ) const
{
  params->m_iExtendedSpatialScalability = m_uiExtendedSpatialScalability;

  params->m_bCrop = (m_uiExtendedSpatialScalability != ESS_NONE);

  int w = m_uiFrameWidthInMbs * 16;
  int h = m_uiFrameHeightInMbs * 16;
  params->m_iGlobWidth  = w;
  params->m_iGlobHeight = h;

  params->m_iChromaPhaseX = (Int)m_uiChromaPhaseXPlus1 - 1;
  params->m_iChromaPhaseY = (Int)m_uiChromaPhaseYPlus1 - 1;


  if (m_uiExtendedSpatialScalability == ESS_SEQ)
  {
    params->m_iPosX       = m_iScaledBaseLeftOffset *2;
    params->m_iPosY       = m_iScaledBaseTopOffset *2;
    params->m_iOutWidth   = w - params->m_iPosX - (m_iScaledBaseRightOffset *2);
    params->m_iOutHeight  = h - params->m_iPosY - (m_iScaledBaseBottomOffset *2);
  }
  else
  {
    params->m_iOutWidth   = w;
    params->m_iOutHeight  = h;
    params->m_iPosX       = 0;
    params->m_iPosY       = 0;
  }
#ifdef _JVTV074_
    params->m_uiNumResampleFiltersMinus1 = m_uiNumResampleFiltersMinus1;
  Int k;
  UInt uiResampleFilterIdx;
  for (uiResampleFilterIdx = 0; uiResampleFilterIdx <= m_uiNumResampleFiltersMinus1; uiResampleFilterIdx++)
  {
      params->m_bIntegerPosFilterPresentFlag[uiResampleFilterIdx] = m_bIntegerPosFilterPresentFlag[uiResampleFilterIdx];
      for (k = 0; k < 9; k++) 
      {

          params->m_iResampleFilterParamA[uiResampleFilterIdx][k] = m_iResampleFilterParamA[uiResampleFilterIdx][k];
          params->m_iResampleFilterParamB[uiResampleFilterIdx][k] = m_iResampleFilterParamB[uiResampleFilterIdx][k];
      }
  }
#endif //_JVTV074_
}
// TMM_ESS }

ErrVal SequenceParameterSet::xReadPicOrderCntInfo( HeaderSymbolReadIf* pcReadIf )
{
  RNOK( pcReadIf->getUvlc( m_uiPicOrderCntType,                  "SPS: pic_order_cnt_type" ) );
  ROT( m_uiPicOrderCntType>2 );

  ROTRS( 2 == m_uiPicOrderCntType, Err::m_nOK );

  if( 0 == m_uiPicOrderCntType )
  {
    UInt uiTmp;
    RNOK( pcReadIf->getUvlc( uiTmp,                              "SPS: log2_max_pic_order_cnt_lsb_minus4" ));
    setLog2MaxPicOrderCntLsb( 4+uiTmp );
  }
	else if( 1 == m_uiPicOrderCntType )
	{
		RNOK( pcReadIf->getFlag( m_bDeltaPicOrderAlwaysZeroFlag,     "SPS: delta_pic_order_always_zero_flag" ));
		RNOK( pcReadIf->getSvlc( m_iOffsetForNonRefPic,              "SPS: offset_for_non_ref_pic" ));
		RNOK( pcReadIf->getSvlc( m_iOffsetForTopToBottomField,       "SPS: offset_for_top_to_bottom_field" ));
		RNOK( pcReadIf->getUvlc( m_uiNumRefFramesInPicOrderCntCycle, "SPS: num_ref_frames_in_pic_order_cnt_cycle" ));
		RNOK( initOffsetForRefFrame( m_uiNumRefFramesInPicOrderCntCycle ) );

		for( UInt i = 0; i < m_uiNumRefFramesInPicOrderCntCycle; i++)
		{
			Int  iTmp;
			RNOK( pcReadIf->getSvlc( iTmp, "SPS: offset_for_ref_frame" ) );
			setOffsetForRefFrame( i, iTmp );
		}
	}

  return Err::m_nOK;
}

H264AVC_NAMESPACE_END
