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

#include "H264AVCCommonLib/SliceHeaderBase.h"
#include "H264AVCCommonLib/TraceFile.h"



H264AVC_NAMESPACE_BEGIN



SliceHeaderBase::SliceHeaderBase( const SequenceParameterSet& rcSPS,
                                  const PictureParameterSet&  rcPPS )
: m_rcPPS                             ( rcPPS )
, m_rcSPS                             ( rcSPS )
, m_eNalRefIdc                        ( NAL_REF_IDC_PRIORITY_LOWEST )
, m_eNalUnitType                      ( NAL_UNIT_EXTERNAL )
, m_uiLayerId                         ( 0 )
, m_uiTemporalLevel                   ( 0 )
, m_uiQualityLevel                    ( 0 )
, m_uiFirstMbInSlice                  ( 0 )
, m_eSliceType                        ( B_SLICE )
, m_uiPicParameterSetId               ( rcPPS.getPicParameterSetId() )
, m_uiFrameNum                        ( 0 )
, m_uiIdrPicId                        ( 0 )
, m_uiPicOrderCntLsb                  ( 0 )
, m_bDirectSpatialMvPredFlag          ( true )
, m_uiGOPId                           ( 0 )
, m_uiGOPSize                         ( 0 )
, m_uiDecompositionStages             ( 0 )
, m_uiFrameIdInsideGOP                ( 0 )
, m_uiBaseLayerId                     ( MSYS_UINT_MAX )
, m_bAdaptivePredictionFlag           ( false )
, m_bNumRefIdxActiveOverrideFlag      ( false )
, m_bNoOutputOfPriorPicsFlag          ( true  )
, m_bAdaptiveRefPicBufferingModeFlag  ( false )
, m_uiCabacInitIdc                    ( 0 )
, m_iSliceQpDelta                     ( 0 )
{
  ::memset( m_auiNumRefIdxActive        , 0x00, 2*sizeof(UInt) );
  ::memset( m_aauiNumRefIdxActiveUpdate , 0x00, 2*sizeof(UInt)*MAX_TEMP_LEVELS );
}


SliceHeaderBase::~SliceHeaderBase()
{
}


ErrVal
SliceHeaderBase::write( HeaderSymbolWriteIf* pcWriteIf ) const
{
  if( m_eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE ||
      m_eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE        )
  {
    return xWriteScalable         ( pcWriteIf );
  }
  else
  {
    return xWriteH264AVCCompatible( pcWriteIf );
  }
  return Err::m_nERR;
}



ErrVal
SliceHeaderBase::xWriteScalable( HeaderSymbolWriteIf* pcWriteIf ) const
{
  //===== NAL unit header =====
  Bool m_bTraceEnable = true;
  RNOK  ( pcWriteIf->writeFlag( 0,                                              "NALU HEADER: forbidden_zero_bit" ) );
  RNOK  ( pcWriteIf->writeCode( m_eNalRefIdc,   2,                              "NALU HEADER: nal_ref_idc" ) );
  RNOK  ( pcWriteIf->writeCode( m_eNalUnitType, 5,                              "NALU HEADER: nal_unit_type" ) );
  if( m_eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE ||
      m_eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE )
  {
    UChar ucDDI = ( m_uiLayerId       << 5 );
    ucDDI      += ( m_uiTemporalLevel << 2 );
    ucDDI      += ( m_uiQualityLevel       );
    RNOK( pcWriteIf->writeCode( ucDDI,          8,                              "NALU HEADER: decodability_dependency_information" ) );
  }

  
  //===== slice header =====
  RNOK(     pcWriteIf->writeUvlc( m_uiFirstMbInSlice,                           "SH: first_mb_in_slice" ) );
  
  UInt  uiSliceType = ( m_eSliceType == B_SLICE ? 0 : m_eSliceType == P_SLICE ? 1 : UInt(m_eSliceType) );
  RNOK(     pcWriteIf->writeUvlc( uiSliceType,                                  "SH: slice_type" ) );
  
  RNOK(     pcWriteIf->writeUvlc( m_uiPicParameterSetId,                        "SH: pic_parameter_set_id" ) );
  RNOK(     pcWriteIf->writeCode( m_uiFrameNum,
                                  getSPS().getLog2MaxFrameNum(),                "SH: frame_num" ) );
  if( m_eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE )
  {
    RNOK(   pcWriteIf->writeUvlc( m_uiIdrPicId,                                 "SH: idr_pic_id" ) );
  }
  
  RNOK(     pcWriteIf->writeCode( m_uiPicOrderCntLsb,
                                  getSPS().getLog2MaxPicOrderCntLsb(),          "SH: pic_order_cnt_lsb" ) );
  RNOK(     pcWriteIf->writeSvlc( 0,                                            "SH: delta_pic_order_cnt_bottom" ) );
  
  if( m_eSliceType == B_SLICE )
  {
    RNOK(   pcWriteIf->writeFlag( m_bDirectSpatialMvPredFlag,                   "SH: direct_spatial_mv_pred_flag" ) );
  }
  
  if( m_eSliceType != F_SLICE )
  {
    RNOK(   pcWriteIf->writeCode( m_uiGOPId, 4,                                 "SH: gop_id" ) );
    RNOK(   pcWriteIf->writeUvlc( m_uiGOPSize,                                  "SH: gop_size" ) );
    RNOK(   pcWriteIf->writeUvlc( m_uiDecompositionStages,                      "SH: decomposition_stages" ) );

    //----- that's a bit tricky (different parameter for encoder and decoder) -----
    UInt    uiFrameIdInsideGOP  = max( 1, m_uiFrameIdInsideGOP );
    RNOK(   pcWriteIf->writeUvlc( uiFrameIdInsideGOP,                           "SH: pic_id_inside_gop" ) );

    UInt  uiBaseLayerIdPlus1 = m_uiBaseLayerId + 1;
    RNOK(   pcWriteIf->writeUvlc( uiBaseLayerIdPlus1,                           "SH: base_layer_id_plus1" ) );
    if( uiBaseLayerIdPlus1 )
    {
      RNOK( pcWriteIf->writeFlag( m_bAdaptivePredictionFlag,                    "SH: adaptive_prediction_flag" ) );
    }
    
    if( m_eSliceType == P_SLICE || m_eSliceType == B_SLICE )
    {
      RNOK( pcWriteIf->writeFlag( m_bNumRefIdxActiveOverrideFlag,               "SH: num_ref_idx_active_override_flag" ) );
      if( m_bNumRefIdxActiveOverrideFlag )
      {
        RNOK( pcWriteIf->writeUvlc( m_auiNumRefIdxActive[LIST_0]-1,             "SH: num_ref_idx_l0_active_minus1" ) );
        if( m_eSliceType == B_SLICE )
        {
          RNOK( pcWriteIf->writeUvlc( m_auiNumRefIdxActive[LIST_1]-1,           "SH: num_ref_idx_l1_active_minus1" ) );
        }
      }
    }

    if( m_eSliceType == P_SLICE || m_eSliceType == B_SLICE )
    {
      RNOK( getRplrBuffer( LIST_0 ).write( pcWriteIf ) );
    }
    if( m_eSliceType == B_SLICE )
    {
      RNOK( getRplrBuffer( LIST_1 ).write( pcWriteIf ) );
    }

    for( UInt uiIndex = m_uiTemporalLevel; uiIndex < m_uiDecompositionStages; uiIndex++ )
    {
      RNOK( pcWriteIf->writeUvlc( m_aauiNumRefIdxActiveUpdate[uiIndex][LIST_0], "SH: num_ref_idx_update_l0_active" ) );
      RNOK( pcWriteIf->writeUvlc( m_aauiNumRefIdxActiveUpdate[uiIndex][LIST_1], "SH: num_ref_idx_update_l1_active" ) );
    }

    if( getNalRefIdc() )
    {
      if( isIdrNalUnit() )
      {
        RNOK( pcWriteIf->writeFlag( m_bNoOutputOfPriorPicsFlag,                 "DRPM: no_output_of_prior_pics_flag" ) );
        RNOK( pcWriteIf->writeFlag( false,                                      "DRPM: long_term_reference_flag" ) );
      }
      else
      {
        RNOK( pcWriteIf->writeFlag( m_bAdaptiveRefPicBufferingModeFlag,         "DRPM: adaptive_ref_pic_buffering_mode_flag" ) );
        if( m_bAdaptiveRefPicBufferingModeFlag )
        {
          RNOK( getMmcoBuffer().write( pcWriteIf ) );
        }
      }
    }

    if( getPPS().getEntropyCodingModeFlag() && m_eSliceType != I_SLICE )
    {
      RNOK( pcWriteIf->writeUvlc( m_uiCabacInitIdc,                             "SH: cabac_init_idc" ) );
    }
  }


  RNOK( pcWriteIf->writeSvlc( m_iSliceQpDelta,                                  "SH: slice_qp_delta" ) );
  
  if( getPPS().getDeblockingFilterParametersPresentFlag() )
  {
    RNOK( getDeblockingFilterParameter().write( pcWriteIf ) );
  }

  return Err::m_nOK;
}



ErrVal
SliceHeaderBase::xWriteH264AVCCompatible( HeaderSymbolWriteIf* pcWriteIf ) const
{
  //===== NAL unit header =====
  Bool m_bTraceEnable = true;
  RNOK  ( pcWriteIf->writeFlag( 0,                                              "NALU HEADER: forbidden_zero_bit" ) );
  RNOK  ( pcWriteIf->writeCode( m_eNalRefIdc,   2,                              "NALU HEADER: nal_ref_idc" ) );
  RNOK  ( pcWriteIf->writeCode( m_eNalUnitType, 5,                              "NALU HEADER: nal_unit_type" ) );

  
  //===== slice header =====
  RNOK(     pcWriteIf->writeUvlc( m_uiFirstMbInSlice,                           "SH: first_mb_in_slice" ) );
  RNOK(     pcWriteIf->writeUvlc( m_eSliceType,                                 "SH: slice_type" ) );
  RNOK(     pcWriteIf->writeUvlc( m_uiPicParameterSetId,                        "SH: pic_parameter_set_id" ) );
  RNOK(     pcWriteIf->writeCode( m_uiFrameNum,
                                  getSPS().getLog2MaxFrameNum(),                "SH: frame_num" ) );
  if( m_eNalUnitType == NAL_UNIT_CODED_SLICE_IDR )
  {
    RNOK(   pcWriteIf->writeUvlc( m_uiIdrPicId,                                 "SH: idr_pic_id" ) );
  }
  
  RNOK(     pcWriteIf->writeCode( m_uiPicOrderCntLsb,
                                  getSPS().getLog2MaxPicOrderCntLsb(),          "SH: pic_order_cnt_lsb" ) );
  RNOK(     pcWriteIf->writeSvlc( 0,                                            "SH: delta_pic_order_cnt_bottom" ) );
  
  if( m_eSliceType == B_SLICE )
  {
    RNOK(   pcWriteIf->writeFlag( m_bDirectSpatialMvPredFlag,                   "SH: direct_spatial_mv_pred_flag" ) );
  }

  if( m_eSliceType == P_SLICE || m_eSliceType == B_SLICE )
  {
    RNOK( pcWriteIf->writeFlag( m_bNumRefIdxActiveOverrideFlag,                 "SH: num_ref_idx_active_override_flag" ) );
    if( m_bNumRefIdxActiveOverrideFlag )
    {
      RNOK( pcWriteIf->writeUvlc( m_auiNumRefIdxActive[LIST_0]-1,               "SH: num_ref_idx_l0_active_minus1" ) );
      if( m_eSliceType == B_SLICE )
      {
        RNOK( pcWriteIf->writeUvlc( m_auiNumRefIdxActive[LIST_1]-1,             "SH: num_ref_idx_l1_active_minus1" ) );
      }
    }
  }

  if( m_eSliceType == P_SLICE || m_eSliceType == B_SLICE )
  {
    RNOK( getRplrBuffer( LIST_0 ).write( pcWriteIf ) );
  }
  if( m_eSliceType == B_SLICE )
  {
    RNOK( getRplrBuffer( LIST_1 ).write( pcWriteIf ) );
  }

  if( getNalRefIdc() )
  {
    if( isIdrNalUnit() )
    {
      RNOK( pcWriteIf->writeFlag( m_bNoOutputOfPriorPicsFlag,                   "DRPM: no_output_of_prior_pics_flag" ) );
      RNOK( pcWriteIf->writeFlag( false,                                        "DRPM: long_term_reference_flag" ) );
    }
    else
    {
      RNOK( pcWriteIf->writeFlag( m_bAdaptiveRefPicBufferingModeFlag,           "DRPM: adaptive_ref_pic_buffering_mode_flag" ) );
      if( m_bAdaptiveRefPicBufferingModeFlag )
      {
        RNOK( getMmcoBuffer().write( pcWriteIf ) );
      }
    }
  }

  if( getPPS().getEntropyCodingModeFlag() && m_eSliceType != I_SLICE )
  {
    RNOK( pcWriteIf->writeUvlc( m_uiCabacInitIdc,                               "SH: cabac_init_idc" ) );
  }


  RNOK( pcWriteIf->writeSvlc( m_iSliceQpDelta,                                  "SH: slice_qp_delta" ) );
  
  if( getPPS().getDeblockingFilterParametersPresentFlag() )
  {
    RNOK( getDeblockingFilterParameter().write( pcWriteIf ) );
  }

  return Err::m_nOK;
}




ErrVal
SliceHeaderBase::read( HeaderSymbolReadIf* pcReadIf )
{
  if( m_eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE || 
      m_eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE       )
  {
    if      ( m_eSliceType == B_SLICE ) m_eSliceType = P_SLICE;
    else if ( m_eSliceType == P_SLICE ) m_eSliceType = B_SLICE;

    return xReadScalable          ( pcReadIf );
  }
  else
  {
    return xReadH264AVCCompatible ( pcReadIf );
  }
  return Err::m_nERR;
}



ErrVal
SliceHeaderBase::xReadScalable( HeaderSymbolReadIf* pcReadIf )
{
  Bool  bTmp;
  Int   iTmp;
  UInt  uiTmp;

 
  RNOK(     pcReadIf->getCode( m_uiFrameNum,
                               getSPS().getLog2MaxFrameNum(),                "SH: frame_num" ) );
  if( m_eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE )
  {
    RNOK(   pcReadIf->getUvlc( m_uiIdrPicId,                                 "SH: idr_pic_id" ) );
  }
  
  RNOK(     pcReadIf->getCode( m_uiPicOrderCntLsb,
                               getSPS().getLog2MaxPicOrderCntLsb(),          "SH: pic_order_cnt_lsb" ) );
  RNOK(     pcReadIf->getSvlc( iTmp,                                         "SH: delta_pic_order_cnt_bottom" ) );
  ROT ( iTmp );
  
  if( m_eSliceType == B_SLICE )
  {
    RNOK(   pcReadIf->getFlag( m_bDirectSpatialMvPredFlag,                   "SH: direct_spatial_mv_pred_flag" ) );
  }
  
  if( m_eSliceType != F_SLICE )
  {
    RNOK(   pcReadIf->getCode( m_uiGOPId, 4,                                 "SH: gop_id" ) );
    RNOK(   pcReadIf->getUvlc( m_uiGOPSize,                                  "SH: gop_size" ) );
    RNOK(   pcReadIf->getUvlc( m_uiDecompositionStages,                      "SH: decomposition_stages" ) );
    RNOK(   pcReadIf->getUvlc( m_uiFrameIdInsideGOP,                         "SH: pic_id_inside_gop" ) );

    RNOK(   pcReadIf->getUvlc( uiTmp,                                        "SH: base_layer_id_plus1" ) );
    m_uiBaseLayerId = uiTmp - 1;
    if( m_uiBaseLayerId != MSYS_UINT_MAX )
    {
      RNOK( pcReadIf->getFlag( m_bAdaptivePredictionFlag,                    "SH: adaptive_prediction_flag" ) );
    }
    
    if( m_eSliceType == P_SLICE || m_eSliceType == B_SLICE )
    {
      RNOK( pcReadIf->getFlag( m_bNumRefIdxActiveOverrideFlag,               "SH: num_ref_idx_active_override_flag" ) );
      if( m_bNumRefIdxActiveOverrideFlag )
      {
        RNOK( pcReadIf->getUvlc( m_auiNumRefIdxActive[LIST_0],               "SH: num_ref_idx_l0_active_minus1" ) );
        m_auiNumRefIdxActive[LIST_0]++;
        if( m_eSliceType == B_SLICE )
        {
          RNOK( pcReadIf->getUvlc( m_auiNumRefIdxActive[LIST_1],             "SH: num_ref_idx_l1_active_minus1" ) );
          m_auiNumRefIdxActive[LIST_1]++;
        }
      }
    }
    if( m_eSliceType != B_SLICE )
    {
      m_auiNumRefIdxActive[LIST_1] = 0;
    }

    if( m_eSliceType == P_SLICE || m_eSliceType == B_SLICE )
    {
      RNOK( getRplrBuffer( LIST_0 ).read( pcReadIf, getNumRefIdxActive( LIST_0 ) ) );
    }
    if( m_eSliceType == B_SLICE )
    {
      RNOK( getRplrBuffer( LIST_1 ).read( pcReadIf, getNumRefIdxActive( LIST_1 ) ) );
    }

    for( UInt uiIndex = m_uiTemporalLevel; uiIndex < m_uiDecompositionStages; uiIndex++ )
    {
      RNOK( pcReadIf->getUvlc( m_aauiNumRefIdxActiveUpdate[uiIndex][LIST_0], "SH: num_ref_idx_update_l0_active" ) );
      RNOK( pcReadIf->getUvlc( m_aauiNumRefIdxActiveUpdate[uiIndex][LIST_1], "SH: num_ref_idx_update_l1_active" ) );
    }

    if( getNalRefIdc() )
    {
      if( isIdrNalUnit() )
      {
        RNOK( pcReadIf->getFlag( m_bNoOutputOfPriorPicsFlag,                 "DRPM: no_output_of_prior_pics_flag" ) );
        RNOK( pcReadIf->getFlag( bTmp,                                       "DRPM: long_term_reference_flag" ) );
        ROT ( bTmp );
      }
      else
      {
        RNOK( pcReadIf->getFlag( m_bAdaptiveRefPicBufferingModeFlag,         "DRPM: adaptive_ref_pic_buffering_mode_flag" ) );
        if( m_bAdaptiveRefPicBufferingModeFlag )
        {
          RNOK( getMmcoBuffer().read( pcReadIf ) );
        }
      }
    }

    if( getPPS().getEntropyCodingModeFlag() && m_eSliceType != I_SLICE )
    {
      RNOK( pcReadIf->getUvlc( m_uiCabacInitIdc,                             "SH: cabac_init_idc" ) );
    }
  }


  RNOK( pcReadIf->getSvlc( m_iSliceQpDelta,                                  "SH: slice_qp_delta" ) );
  
  if( getPPS().getDeblockingFilterParametersPresentFlag() )
  {
    RNOK( getDeblockingFilterParameter().read( pcReadIf ) );
  }

  return Err::m_nOK;
}



ErrVal
SliceHeaderBase::xReadH264AVCCompatible( HeaderSymbolReadIf* pcReadIf )
{
  Bool  bTmp;
  Int   iTmp;

 
  RNOK(     pcReadIf->getCode( m_uiFrameNum,
                               getSPS().getLog2MaxFrameNum(),                "SH: frame_num" ) );
  if( m_eNalUnitType == NAL_UNIT_CODED_SLICE_IDR )
  {
    RNOK(   pcReadIf->getUvlc( m_uiIdrPicId,                                 "SH: idr_pic_id" ) );
  }
  
  RNOK(     pcReadIf->getCode( m_uiPicOrderCntLsb,
                               getSPS().getLog2MaxPicOrderCntLsb(),          "SH: pic_order_cnt_lsb" ) );
  RNOK(     pcReadIf->getSvlc( iTmp,                                         "SH: delta_pic_order_cnt_bottom" ) );
  ROT ( iTmp );
  
  if( m_eSliceType == B_SLICE )
  {
    RNOK(   pcReadIf->getFlag( m_bDirectSpatialMvPredFlag,                   "SH: direct_spatial_mv_pred_flag" ) );
  }

  if( m_eSliceType == P_SLICE || m_eSliceType == B_SLICE )
  {
    RNOK( pcReadIf->getFlag( m_bNumRefIdxActiveOverrideFlag,                 "SH: num_ref_idx_active_override_flag" ) );
    if( m_bNumRefIdxActiveOverrideFlag )
    {
      RNOK( pcReadIf->getUvlc( m_auiNumRefIdxActive[LIST_0],                 "SH: num_ref_idx_l0_active_minus1" ) );
      m_auiNumRefIdxActive[LIST_0]++;
      if( m_eSliceType == B_SLICE )
      {
        RNOK( pcReadIf->getUvlc( m_auiNumRefIdxActive[LIST_1],               "SH: num_ref_idx_l1_active_minus1" ) );
        m_auiNumRefIdxActive[LIST_1]++;
      }
    }
  }

  if( m_eSliceType == P_SLICE || m_eSliceType == B_SLICE )
  {
    RNOK( getRplrBuffer( LIST_0 ).read( pcReadIf, getNumRefIdxActive( LIST_0 ) ) );
  }
  if( m_eSliceType == B_SLICE )
  {
    RNOK( getRplrBuffer( LIST_1 ).read( pcReadIf, getNumRefIdxActive( LIST_1 ) ) );
  }

  if( getNalRefIdc() )
  {
    if( isIdrNalUnit() )
    {
      RNOK( pcReadIf->getFlag( m_bNoOutputOfPriorPicsFlag,                   "DRPM: no_output_of_prior_pics_flag" ) );
      RNOK( pcReadIf->getFlag( bTmp,                                         "DRPM: long_term_reference_flag" ) );
      ROT ( bTmp );
    }
    else
    {
      RNOK( pcReadIf->getFlag( m_bAdaptiveRefPicBufferingModeFlag,           "DRPM: adaptive_ref_pic_buffering_mode_flag" ) );
      if( m_bAdaptiveRefPicBufferingModeFlag )
      {
        RNOK( getMmcoBuffer().read( pcReadIf ) );
      }
    }
  }

  if( getPPS().getEntropyCodingModeFlag() && m_eSliceType != I_SLICE )
  {
    RNOK( pcReadIf->getUvlc( m_uiCabacInitIdc,                               "SH: cabac_init_idc" ) );
  }


  RNOK( pcReadIf->getSvlc( m_iSliceQpDelta,                                  "SH: slice_qp_delta" ) );
  
  if( getPPS().getDeblockingFilterParametersPresentFlag() )
  {
    RNOK( getDeblockingFilterParameter().read( pcReadIf ) );
  }

  return Err::m_nOK;
}



ErrVal
SliceHeaderBase::DeblockingFilterParameter::write( HeaderSymbolWriteIf* pcWriteIf ) const
{
  RNOK( pcWriteIf->writeUvlc( getDisableDeblockingFilterIdc(),  "SH: disable_deblocking_filter_idc" ) );
  ROTRS( 1 == getDisableDeblockingFilterIdc(), Err::m_nOK );

  RNOK( pcWriteIf->writeSvlc( getSliceAlphaC0Offset() >> 1,     "SH: slice_alpha_c0_offset_div2" ) );
  RNOK( pcWriteIf->writeSvlc( getSliceBetaOffset() >> 1,        "SH: slice_beta_offset_div2" ) );
  return Err::m_nOK;
}



ErrVal
SliceHeaderBase::DeblockingFilterParameter::read( HeaderSymbolReadIf* pcReadIf )
{
  RNOK( pcReadIf->getUvlc( m_uiDisableDeblockingFilterIdc,      "SH: disable_deblocking_filter_idc" ) );
  ROT ( m_uiDisableDeblockingFilterIdc > 2 );
  ROTRS( 1 == getDisableDeblockingFilterIdc(), Err::m_nOK );

  Int iTmp;
  RNOK( pcReadIf->getSvlc( iTmp,                                "SH: slice_alpha_c0_offset_div2" ) );
  ROT( (iTmp < -6) || (iTmp >  6) );
  setSliceAlphaC0Offset( iTmp << 1);

  RNOK( pcReadIf->getSvlc( iTmp,                                "SH: slice_beta_offset_div2" ) );
  ROT( (iTmp < -6) || (iTmp >  6) );
  setSliceBetaOffset( iTmp << 1 );

  return Err::m_nOK;
}



H264AVC_NAMESPACE_END

