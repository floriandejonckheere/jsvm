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
, m_bNalUnitExtFlag                         ( true  )
, m_bSeqScalingMatrixPresentFlag            ( false )
, m_uiLog2MaxFrameNum                       ( 0 )
, m_uiPicOrderCntType                       ( 0 )
, m_uiLog2MaxPicOrderCntLsb                 ( 4 )
, m_bDeltaPicOrderAlwaysZeroFlag            ( false )
, m_iOffsetForNonRefPic                     ( 0 )
, m_iOffsetForTopToBottomField              ( 0 )
, m_uiNumRefFramesInPicOrderCntCycle        ( 0 )
, m_uiNumRefFrames                          ( 0 )
, m_bRequiredFrameNumUpdateBehaviourFlag    ( false )
, m_uiFrameWidthInMbs                       ( 0 )
, m_uiFrameHeightInMbs                      ( 0 )
, m_bDirect8x8InferenceFlag                 ( false )
,m_uiExtendedSpatialScalability             ( ESS_NONE ) // TMM_ESS
,m_uiChromaPhaseXPlus1                      ( 0 ) // TMM_ESS
,m_uiChromaPhaseYPlus1                      ( 1 )// TMM_ESS
, m_uiNumSimplePriIdVals                    ( 0 ) 
#if MULTIPLE_LOOP_DECODING
, m_bAlwaysDecodeBaseLayer                  ( false )
#endif
, m_bFGSCodingMode                          ( false )
, m_uiGroupingSize                          ( 1 )
#if INDEPENDENT_PARSING
, m_bIndepedentParsing                      ( false )
#endif
{
	m_auiNumRefIdxUpdateActiveDefault[LIST_0]=1;// VW
	m_auiNumRefIdxUpdateActiveDefault[LIST_1]=1;// VW
  for ( UInt uiPriId = 0; uiPriId < (1 << PRI_ID_BITS); uiPriId++ )
  {
      m_uiTemporalLevelList[uiPriId] = 0;
      m_uiDependencyIdList [uiPriId] = 0;
      m_uiQualityLevelList [uiPriId] = 0;
  }
  ::memset( m_aiOffsetForRefFrame, 0x00, 64*sizeof(Int) );
  ::memset( m_uiPosVect,           0x00, 16*sizeof(UInt));
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
  ETRACE_DO( Bool m_bTraceEnable = true );
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
  
  if( m_eProfileIdc == SCALABLE_PROFILE ) // bug-fix (HS)
  {
    RNOK( pcWriteIf->writeFlag( m_bNalUnitExtFlag,                        "SPS: nal_unit_extension_flag" ) );
    if ( m_bNalUnitExtFlag == 0 )
    {
        RNOK ( pcWriteIf->writeUvlc( m_uiNumSimplePriIdVals - 1,          "SPS: number_of_simple_priority_id_values_minus1" ) );
        for ( UInt uiPriCount = 0; uiPriCount < m_uiNumSimplePriIdVals; uiPriCount++ )
        {
            RNOK ( pcWriteIf->writeCode( uiPriCount,              PRI_ID_BITS, "SPS: priority_id" ) );
            RNOK ( pcWriteIf->writeCode( m_uiTemporalLevelList[uiPriCount], 3, "SPS: temporal_level_list[priority_id]" ) );
            RNOK ( pcWriteIf->writeCode( m_uiDependencyIdList [uiPriCount], 3, "SPS: dependency_id_list[priority_id]" ) );
            RNOK ( pcWriteIf->writeCode( m_uiQualityLevelList [uiPriCount], 2, "SPS: quality_level_list[priority_id]" ) );
        }
    }

    RNOK( pcWriteIf->writeCode( getExtendedSpatialScalability(), 2,       "SPS: ExtendedSpatialScalability" ) );
//    if ( 1 /* chroma_format_idc */ > 0 )
    {
      RNOK( pcWriteIf->writeCode( m_uiChromaPhaseXPlus1, 2,             "SPS: ChromaPhaseXPlus1" ) );
      RNOK( pcWriteIf->writeCode( m_uiChromaPhaseYPlus1, 2,             "SPS: ChromaPhaseYPlus1" ) );
    }
    if (getExtendedSpatialScalability() == ESS_SEQ)
    {
      RNOK( pcWriteIf->writeSvlc( m_iScaledBaseLeftOffset,              "SPS: ScaledBaseLeftOffset" ) );
      RNOK( pcWriteIf->writeSvlc( m_iScaledBaseTopOffset,               "SPS: ScaledBaseTopOffset" ) );
      RNOK( pcWriteIf->writeSvlc( m_iScaledBaseRightOffset,             "SPS: ScaledBaseRightOffset" ) );
      RNOK( pcWriteIf->writeSvlc( m_iScaledBaseBottomOffset,            "SPS: ScaledBaseBottomOffset" ) );
    }

    RNOK  ( pcWriteIf->writeFlag( m_bFGSCodingMode,                       "SPS: FGSCodingMode") );
    if(m_bFGSCodingMode == false)
    {
      RNOK  ( pcWriteIf->writeUvlc(m_uiGroupingSize-1,                    "SPS: GroupingSizeMinus1") );
    }
    else
    {
      UInt uiNumPosVector = 0;
      UInt uiIndex = 0;
      while(uiNumPosVector != 15)
      {
        if(uiIndex == 0)
        {
          RNOK( pcWriteIf->writeUvlc(m_uiPosVect[uiIndex],               "SPS: PosVect[0]") );
        }
        else
        {
          RNOK( pcWriteIf->writeUvlc(m_uiPosVect[uiIndex]-m_uiPosVect[uiIndex-1]-1, "SPS: PosVect") );
        }
        uiNumPosVector = m_uiPosVect[uiIndex];
        uiIndex++;
      }
    }

#if MULTIPLE_LOOP_DECODING
    RNOK( pcWriteIf->writeFlag( m_bAlwaysDecodeBaseLayer,                 "SPS: always_decode_base_layer" ) );
#endif
#if INDEPENDENT_PARSING
    RNOK( pcWriteIf->writeFlag( m_bIndepedentParsing,                     "SPS: independent_parsing" ) );
#endif
  }
  //--- fidelity range extension syntax ---
  RNOK  ( xWriteFrext( pcWriteIf ) );
  
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
  RNOK  ( pcWriteIf->writeFlag( getRequiredFrameNumUpdateBehaviourFlag(), "SPS: required_frame_num_update_behaviour_flag" ) );
  RNOK  ( pcWriteIf->writeUvlc( getFrameWidthInMbs  () - 1,               "SPS: pic_width_in_mbs_minus_1" ) );
  RNOK  ( pcWriteIf->writeUvlc( getFrameHeightInMbs () - 1,               "SPS: pic_height_in_map_units_minus_1" ) );
  RNOK  ( pcWriteIf->writeFlag( true,                                     "SPS: frame_mbs_only_flag" ) );
  RNOK  ( pcWriteIf->writeFlag( getDirect8x8InferenceFlag(),              "SPS: direct_8x8_inference_flag" ) );
  RNOK  ( pcWriteIf->writeFlag( false,                                    "SPS: frame_cropping_flag" ) );

  RNOK  ( pcWriteIf->writeFlag( false,                                  "SPS: vui_parameters_present_flag" ) );

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
          m_eProfileIdc != MULTI_VIEW_PROFILE &&
          m_eProfileIdc != SCALABLE_PROFILE );
  RNOK  ( pcReadIf->getFlag( m_bConstrainedSet0Flag,                      "SPS: constrained_set0_flag" ) );
  RNOK  ( pcReadIf->getFlag( m_bConstrainedSet1Flag,                      "SPS: constrained_set1_flag" ) );
  RNOK  ( pcReadIf->getFlag( m_bConstrainedSet2Flag,                      "SPS: constrained_set2_flag" ) );
  RNOK  ( pcReadIf->getFlag( m_bConstrainedSet3Flag,                      "SPS: constrained_set3_flag" ) );
  RNOK  ( pcReadIf->getCode( uiTmp,                               4,      "SPS: reserved_zero_4bits" ) );
  ROT   ( uiTmp );
  RNOK  ( pcReadIf->getCode( m_uiLevelIdc,                        8,      "SPS: level_idc" ) );
  RNOK  ( pcReadIf->getUvlc( m_uiSeqParameterSetId,                       "SPS: seq_parameter_set_id" ) );

  if( m_eProfileIdc == SCALABLE_PROFILE ) // bug-fix (HS)
  {

    RNOK( pcReadIf->getFlag( m_bNalUnitExtFlag,                           "SPS: nal_unit_extension_flag" ) );
    if ( m_bNalUnitExtFlag == 0 )
    {
        RNOK ( pcReadIf->getUvlc( m_uiNumSimplePriIdVals,                 "SPS: number_of_simple_priority_id_values_minus1" ) );
        m_uiNumSimplePriIdVals++;
        for ( UInt uiPriCount = 0; uiPriCount < m_uiNumSimplePriIdVals; uiPriCount++ )
        {
            RNOK ( pcReadIf->getCode( uiTmp,              PRI_ID_BITS,    "SPS: priority_id" ) );
            RNOK ( pcReadIf->getCode( m_uiTemporalLevelList[uiTmp], 3,    "SPS: temporal_level_list[priority_id]" ) );
            RNOK ( pcReadIf->getCode( m_uiDependencyIdList [uiTmp], 3,    "SPS: dependency_id_list[priority_id]" ) );
            RNOK ( pcReadIf->getCode( m_uiQualityLevelList [uiTmp], 2,    "SPS: quality_level_list[priority_id]" ) );
        }
    }

    RNOK( pcReadIf->getCode( m_uiExtendedSpatialScalability, 2,           "SPS: ExtendedSpatialScalability" ) );
//    if ( 1 /* chroma_format_idc */ > 0 )
    {
      RNOK( pcReadIf->getCode( m_uiChromaPhaseXPlus1, 2,                  "SPS: ChromaPhaseXPlus1" ) );
      RNOK( pcReadIf->getCode( m_uiChromaPhaseYPlus1, 2,                  "SPS: ChromaPhaseYPlus1" ) );
    }
    if (m_uiExtendedSpatialScalability == ESS_SEQ)
    {
      RNOK( pcReadIf->getSvlc( m_iScaledBaseLeftOffset,                   "SPS: ScaledBaseLeftOffset" ) );
      RNOK( pcReadIf->getSvlc( m_iScaledBaseTopOffset,                    "SPS: ScaledBaseTopOffset" ) );
      RNOK( pcReadIf->getSvlc( m_iScaledBaseRightOffset,                  "SPS: ScaledBaseRightOffset" ) );
      RNOK( pcReadIf->getSvlc( m_iScaledBaseBottomOffset,                 "SPS: ScaledBaseBottomOffset" ) );
    }
    RNOK  ( pcReadIf->getFlag( m_bFGSCodingMode,                            "SPS: FGSCodingMode") );
    if(m_bFGSCodingMode == false)
    {
      RNOK  ( pcReadIf->getUvlc(m_uiGroupingSize,                           "SPS: GroupingSizeMinus1") );
      m_uiGroupingSize++;
    }
    else
    {
      UInt uiNumPosVector = 0;
      UInt uiIndex = 0;
      while(uiNumPosVector != 15)
      {
        if(uiIndex == 0)
        {
          RNOK( pcReadIf->getUvlc(m_uiPosVect[uiIndex],                     "SPS: PosVect[0]") );
        }
        else
        {
          RNOK( pcReadIf->getUvlc(m_uiPosVect[uiIndex], "SPS: PosVect") );
          m_uiPosVect[uiIndex] = m_uiPosVect[uiIndex] + m_uiPosVect[uiIndex-1] + 1;
        }
        uiNumPosVector = m_uiPosVect[uiIndex];
        uiIndex++;
      }
    }

#if MULTIPLE_LOOP_DECODING
    RNOK( pcReadIf->getFlag( m_bAlwaysDecodeBaseLayer,                    "SPS: always_decode_base_layer" ) );
#endif
#if INDEPENDENT_PARSING
    RNOK( pcReadIf->getFlag( m_bIndepedentParsing,                        "SPS: independent_parsing" ) );
#endif
  }
  //--- fidelity range extension syntax ---
  RNOK  ( xReadFrext( pcReadIf ) );

  RNOK  ( pcReadIf->getUvlc( uiTmp,                                       "SPS: log2_max_frame_num_minus_4" ) );
  ROT   ( uiTmp > 12 );
  setLog2MaxFrameNum( uiTmp + 4 );
  RNOK  ( pcReadIf->getUvlc( m_uiPicOrderCntType,                         "SPS: pic_order_cnt_type" ) );
  if( m_uiPicOrderCntType == 0 )
  {
    RNOK( pcReadIf->getUvlc( m_uiLog2MaxPicOrderCntLsb,                   "SPS: log2_max_pic_order_cnt_lsb_minus4" ) );
    m_uiLog2MaxPicOrderCntLsb += 4;
  }
  else if( getPicOrderCntType() == 1 )
  {
    RNOK( pcReadIf->getFlag( m_bDeltaPicOrderAlwaysZeroFlag,              "SPS: delta_pic_order_always_zero_flag" ) );
    RNOK( pcReadIf->getSvlc( m_iOffsetForNonRefPic,                       "SPS: offset_for_non_ref_pic" ) );
    RNOK( pcReadIf->getSvlc( m_iOffsetForTopToBottomField,                "SPS: offset_for_top_to_bottom_field" ) );
    RNOK( pcReadIf->getUvlc( m_uiNumRefFramesInPicOrderCntCycle,          "SPS: num_ref_frames_in_pic_order_cnt_cycle" ) );
    for( UInt uiIndex = 0; uiIndex < m_uiNumRefFramesInPicOrderCntCycle; uiIndex++ )
    {
      RNOK( pcReadIf->getSvlc( m_aiOffsetForRefFrame[uiIndex],            "SPS: offset_for_ref_frame" ) );
    }
  }
  RNOK( pcReadIf->getUvlc( m_uiNumRefFrames,                              "SPS: num_ref_frames" ) );
  RNOK( pcReadIf->getFlag( m_bRequiredFrameNumUpdateBehaviourFlag,        "SPS: required_frame_num_update_behaviour_flag" ) );
  RNOK( pcReadIf->getUvlc( uiTmp,                                         "SPS: pic_width_in_mbs_minus_1" ) );
  setFrameWidthInMbs ( 1 + uiTmp );
  RNOK( pcReadIf->getUvlc( uiTmp,                                         "SPS: pic_height_in_map_units_minus_1" ) );
  setFrameHeightInMbs( 1 + uiTmp );
  RNOK( pcReadIf->getFlag( bTmp,                                          "SPS: frame_mbs_only_flag" ) );
  ROF ( bTmp );
  RNOK( pcReadIf->getFlag( m_bDirect8x8InferenceFlag,                     "SPS: direct_8x8_inference_flag" ) );
  RNOK( pcReadIf->getFlag( bTmp,                                          "SPS: frame_cropping_flag" ) );
  ROT ( bTmp );
  
  RNOK( pcReadIf->getFlag( bTmp,                                          "SPS: vui_parameters_present_flag" ) );
  ROT ( bTmp );

  return Err::m_nOK;
}


ErrVal
SequenceParameterSet::xWriteFrext( HeaderSymbolWriteIf* pcWriteIf ) const
{
  ROTRS( m_eProfileIdc != HIGH_PROFILE      &&
         m_eProfileIdc != HIGH_10_PROFILE   &&
         m_eProfileIdc != HIGH_422_PROFILE  &&
         m_eProfileIdc != HIGH_444_PROFILE  &&
         m_eProfileIdc != MULTI_VIEW_PROFILE &&
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
         m_eProfileIdc != MULTI_VIEW_PROFILE &&
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
    printf("offset: %d,%d %d,%d\n", m_iScaledBaseLeftOffset<<1, m_iScaledBaseTopOffset<<1, m_iScaledBaseRightOffset<<1, m_iScaledBaseBottomOffset<<1);
  }
  else
  {
    m_iScaledBaseBottomOffset = 0;
    m_iScaledBaseLeftOffset = 0;
    m_iScaledBaseRightOffset = 0;
    m_iScaledBaseTopOffset = 0;
  }
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
}
// TMM_ESS }


H264AVC_NAMESPACE_END
