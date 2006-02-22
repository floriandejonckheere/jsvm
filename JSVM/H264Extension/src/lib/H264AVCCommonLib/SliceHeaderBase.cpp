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

#include "H264AVCCommonLib/CFMO.h"


H264AVC_NAMESPACE_BEGIN


Int iRandom( Int iMin, Int iMax )
{
  Int iRange = iMax - iMin + 1;
  Int iValue = rand() % iRange;
  return iMin + iValue;
}

ErrVal
SliceHeaderBase::PredWeight::createRandomParameters()
{
  setLumaWeightFlag  ( 0 == ( rand() & 1 ) );
  setChromaWeightFlag( 0 == ( rand() & 1 ) );

  if( getLumaWeightFlag() )
  {
    setLumaOffset   (     iRandom( -64, 63 ) );
    setLumaWeight   (     iRandom( -64, 63 ) );
  }
  if( getChromaWeightFlag() )
  {
    setChromaOffset ( 0,  iRandom( -64, 63 ) );
    setChromaWeight ( 0,  iRandom( -64, 63 ) );
    setChromaOffset ( 1,  iRandom( -64, 63 ) );
    setChromaWeight ( 1,  iRandom( -64, 63 ) );
  }
  return Err::m_nOK;
}

ErrVal
SliceHeaderBase::PredWeight::write( HeaderSymbolWriteIf* pcWriteIf ) const
{
  RNOK(   pcWriteIf->writeFlag( getLumaWeightFlag(),    "PWT: luma_weight_flag" ) );
  if( getLumaWeightFlag() )
  {
    RNOK( pcWriteIf->writeSvlc( getLumaWeight(),        "PWT: luma_weight" ) );
    RNOK( pcWriteIf->writeSvlc( getLumaOffset(),        "PWT: luma_offset" ) );
  }

  RNOK(   pcWriteIf->writeFlag( getChromaWeightFlag(),  "PWT: chroma_weight_flag" ) );
  if( getChromaWeightFlag() )
  {
    RNOK( pcWriteIf->writeSvlc( getChromaWeight( 0 ),   "PWT: cr_weight" ) );
    RNOK( pcWriteIf->writeSvlc( getChromaOffset( 0 ),   "PWT: cr_offset" ) );
    RNOK( pcWriteIf->writeSvlc( getChromaWeight( 1 ),   "PWT: cb_weight" ) );
    RNOK( pcWriteIf->writeSvlc( getChromaOffset( 1 ),   "PWT: cb_offset" ) );
  }

  return Err::m_nOK;
}


ErrVal
SliceHeaderBase::PredWeight::read( HeaderSymbolReadIf* pcReadIf )
{
  RNOK(   pcReadIf->getFlag( m_bLumaWeightFlag,     "PWT: luma_weight_flag" ) );
  if( getLumaWeightFlag() )
  {
    RNOK( pcReadIf->getSvlc( m_iLumaWeight,         "PWT: luma_weight" ) );
    RNOK( pcReadIf->getSvlc( m_iLumaOffset,         "PWT: luma_offset" ) );
    ROTR( (-128 > m_iLumaWeight) || (127 < m_iLumaWeight), Err::m_nInvalidParameter );
    ROTR( (-128 > m_iLumaOffset) || (127 < m_iLumaOffset), Err::m_nInvalidParameter );
  }

  RNOK(   pcReadIf->getFlag( m_bChromaWeightFlag,   "PWT: chroma_weight_flag" ) );
  if( getChromaWeightFlag() )
  {
    RNOK( pcReadIf->getSvlc( m_aiChromaWeight[0],   "PWT: cr_weight" ) );
    RNOK( pcReadIf->getSvlc( m_aiChromaOffset[0],   "PWT: cr_offset" ) );
    RNOK( pcReadIf->getSvlc( m_aiChromaWeight[1],   "PWT: cb_weight" ) );
    RNOK( pcReadIf->getSvlc( m_aiChromaOffset[1],   "PWT: cb_offset" ) );
    ROTR( (-128 > m_aiChromaWeight[0]) || (127 < m_aiChromaWeight[0]), Err::m_nInvalidParameter );
    ROTR( (-128 > m_aiChromaOffset[0]) || (127 < m_aiChromaOffset[0]), Err::m_nInvalidParameter );
    ROTR( (-128 > m_aiChromaWeight[1]) || (127 < m_aiChromaWeight[1]), Err::m_nInvalidParameter );
    ROTR( (-128 > m_aiChromaOffset[1]) || (127 < m_aiChromaOffset[1]), Err::m_nInvalidParameter );
  }
  return Err::m_nOK;
}


ErrVal
SliceHeaderBase::PredWeightTable::initDefaults( UInt uiLumaWeightDenom, UInt uiChromaWeightDenom )
{
  const Int iLumaWeight   = 1 << uiLumaWeightDenom;
  const Int iChromaWeight = 1 << uiChromaWeightDenom;

  for( UInt ui = 0; ui < size(); ui++ )
  {
    RNOK( get( ui ).init( iLumaWeight, iChromaWeight, iChromaWeight ) );
  }
  return Err::m_nOK;
}

ErrVal SliceHeaderBase::PredWeightTable::createRandomParameters()
{
  ROT( 0 == size() );

  for( UInt n = 0; n < size(); n++ )
  {
    RNOK( get(n).createRandomParameters( ) );
  }
  return Err::m_nOK;
}


ErrVal
SliceHeaderBase::PredWeightTable::write( HeaderSymbolWriteIf* pcWriteIf, UInt uiNumber ) const
{
  for( UInt ui = 0; ui < uiNumber; ui++ )
  {
    RNOK( get( ui ).write( pcWriteIf ) );
  }
  return Err::m_nOK;
}

ErrVal SliceHeaderBase::PredWeightTable::read( HeaderSymbolReadIf* pcReadIf, UInt uiNumber )
{
  for( UInt ui = 0; ui < uiNumber; ui++ )
  {
    RNOK( get( ui ).read( pcReadIf ) );
  }
  return Err::m_nOK;
}

ErrVal
SliceHeaderBase::PredWeightTable::copy( const PredWeightTable& rcPredWeightTable )
{
  UInt uiCopySize = min( m_uiBufferSize, rcPredWeightTable.m_uiBufferSize );
  for( UInt ui = 0; ui < uiCopySize; ui++ )
  {
    m_pT[ui].copy( rcPredWeightTable.m_pT[ui] );
  }
  return Err::m_nOK;
}





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
, m_uiNumMbsInSlice                   ( 0 )
, m_bFgsComponentSep                  ( 0 )
, m_uiIdrPicId                        ( 0 )
, m_uiPicOrderCntLsb                  ( 0 )
, m_iDeltaPicOrderCntBottom           ( 0 )
, m_bBasePredWeightTableFlag          ( false )
, m_uiLumaLog2WeightDenom             ( 0 )
, m_uiChromaLog2WeightDenom           ( 0 )
, m_bDirectSpatialMvPredFlag          ( true )
, m_bKeyPictureFlag                   ( false )
, m_uiBaseLayerId                     ( MSYS_UINT_MAX )
, m_uiBaseQualityLevel                ( 0 )
, m_uiBaseFragmentOrder				        ( 0 )
, m_bAdaptivePredictionFlag           ( false )
, m_bNumRefIdxActiveOverrideFlag      ( false )
, m_bNoOutputOfPriorPicsFlag          ( true  )
, m_bAdaptiveRefPicBufferingModeFlag  ( false )
, m_uiCabacInitIdc                    ( 0 )
, m_iSliceQpDelta                     ( 0 )
, m_pcFMO                             ( 0 ) //--ICU/ETRI FMO Implementation
//TMM_ESS_UNIFIED {
, m_iScaledBaseLeftOffset             ( 0 ) 
, m_iScaledBaseTopOffset              ( 0 ) 
, m_iScaledBaseRightOffset            ( 0 ) 
, m_iScaledBaseBottomOffset           ( 0 ) 
//TMM_ESS_UNIFIED }
, m_uiBaseChromaPhaseXPlus1           ( 0 ) // TMM_ESS
, m_uiBaseChromaPhaseYPlus1           ( 1 ) // TMM_ESS
, m_bArFgsUsageFlag                   ( false )
, m_uiLowPassFgsMcFilter              ( AR_FGS_DEFAULT_FILTER )
, m_uiBaseWeightZeroBaseBlock         ( AR_FGS_DEFAULT_BASE_WEIGHT_ZERO_BLOCK )
, m_uiBaseWeightZeroBaseCoeff         ( AR_FGS_DEFAULT_BASE_WEIGHT_ZERO_COEFF )
, m_uiFragmentOrder                   ( 0 )
{
  ::memset( m_auiNumRefIdxActive        , 0x00, 2*sizeof(UInt) );
  ::memset( m_aauiNumRefIdxActiveUpdate , 0x00, 2*sizeof(UInt)*MAX_TEMP_LEVELS );
  ::memset( m_aiDeltaPicOrderCnt,         0x00, 2*sizeof(Int) );
}


SliceHeaderBase::~SliceHeaderBase()
{
  if(m_pcFMO !=NULL)
	  m_pcFMO->finit();
  ANOK( m_acPredWeightTable[LIST_0].uninit() );
  ANOK( m_acPredWeightTable[LIST_1].uninit() );
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
    //{{Variable Lengh NAL unit header data with priority and dead substream flag
    //France Telecom R&D- (nathalie.cammas@francetelecom.com)
    RNOK (pcWriteIf->writeCode( m_uiSimplePriorityId,  6,                       "NALU HEADER: simple_priority_id"));
    RNOK (pcWriteIf->writeFlag( m_bDiscardableFlag,                             "NALU HEADER: discardable_flag"));
    RNOK (pcWriteIf->writeFlag( m_bExtensionFlag,                               "NALU HEADER: extension_flag"));
    if( m_bExtensionFlag == true )
    {
      RNOK (pcWriteIf->writeCode( m_uiTemporalLevel,   3,                       "NALU HEADER: temporal_level"));
      RNOK( pcWriteIf->writeCode( m_uiLayerId,         3,                       "NALU HEADER: dependency_id" ) );
      RNOK( pcWriteIf->writeCode( m_uiQualityLevel,    2,                       "NALU HEADER: quality_level" ) );
    } 
    //}}Variable Lengh NAL unit header data with priority and dead substream flag
  }
 
  //===== slice header =====
  RNOK(     pcWriteIf->writeUvlc( m_uiFirstMbInSlice,                           "SH: first_mb_in_slice" ) );
  
  UInt  uiSliceType = ( m_eSliceType == B_SLICE ? 0 : m_eSliceType == P_SLICE ? 1 : UInt(m_eSliceType) );
  RNOK(     pcWriteIf->writeUvlc( uiSliceType,                                  "SH: slice_type" ) );
  
  //JVT-P031
  if(uiSliceType == F_SLICE)
  {
     RNOK( pcWriteIf    ->writeFlag( m_bFragmentedFlag,  "SH: fgs_frag_flag" ) );
     if(m_bFragmentedFlag)
     {
        RNOK( pcWriteIf    ->writeUvlc( m_uiFragmentOrder,  "SH: fgs_frag_order" ) );
        if(m_uiFragmentOrder!=0)
        {
          RNOK( pcWriteIf    ->writeFlag( m_bLastFragmentFlag,  "SH: fgs_last_frag_flag" ) );
        }
     }
  }
  if(m_uiFragmentOrder == 0)
  {
  //~JVT-P031
	if( m_eSliceType == F_SLICE ) 
	{
		RNOK(   pcWriteIf->writeUvlc( m_uiNumMbsInSlice,                            "SH: num_mbs_in_slice" ) );
		RNOK(   pcWriteIf->writeFlag( m_bFgsComponentSep,                           "SH: fgs_comp_sep" ) );
	}
  RNOK(     pcWriteIf->writeUvlc( m_uiPicParameterSetId,                        "SH: pic_parameter_set_id" ) );
  RNOK(     pcWriteIf->writeCode( m_uiFrameNum,
                                  getSPS().getLog2MaxFrameNum(),                "SH: frame_num" ) );
  if( m_eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE )
  {
    RNOK(   pcWriteIf->writeUvlc( m_uiIdrPicId,                                 "SH: idr_pic_id" ) );
  }
  
  if( getSPS().getPicOrderCntType() == 0 )
  {
  RNOK(     pcWriteIf->writeCode( m_uiPicOrderCntLsb,
                                  getSPS().getLog2MaxPicOrderCntLsb(),          "SH: pic_order_cnt_lsb" ) );
    if( getPPS().getPicOrderPresentFlag() && true /* ! field_pic_flag */ )
    {
      RNOK( pcWriteIf->writeSvlc( m_iDeltaPicOrderCntBottom,                    "SH: delta_pic_order_cnt_bottom" ) );
    }
  }
  if( getSPS().getPicOrderCntType() == 1 && ! getSPS().getDeltaPicOrderAlwaysZeroFlag() )
  {
    RNOK(   pcWriteIf->writeSvlc( m_aiDeltaPicOrderCnt[0],                      "SH: delta_pic_order_cnt[0]" ) );
    if( getPPS().getPicOrderPresentFlag() && true /* ! field_pic_flag */ )
    {
      RNOK( pcWriteIf->writeSvlc( m_aiDeltaPicOrderCnt[1],                      "SH: delta_pic_order_cnt[1]" ) );
    }
  }
  
  if( m_eSliceType == B_SLICE )
  {
    RNOK(   pcWriteIf->writeFlag( m_bDirectSpatialMvPredFlag,                   "SH: direct_spatial_mv_pred_flag" ) );
  }
  
  if( m_eSliceType != F_SLICE )
  {
    UInt  uiBaseLayerIdPlus1;
    if( m_uiBaseLayerId == MSYS_UINT_MAX )
      uiBaseLayerIdPlus1 = 0;
    else
      // one example (m_uiBaseLayerId, m_uiBaseQualityLevel) -> uiBaseLayerIdPlus1 mapping
	  uiBaseLayerIdPlus1 = ( (m_uiBaseLayerId << 4) + (m_uiBaseQualityLevel << 2) + m_uiBaseFragmentOrder ) + 1;
    RNOK(   pcWriteIf->writeUvlc( uiBaseLayerIdPlus1,                           "SH: base_id_plus1" ) );
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

    if( ( getPPS().getWeightedPredFlag ()      && ( m_eSliceType == P_SLICE ) ) ||
        ( getPPS().getWeightedBiPredIdc() == 1 && ( m_eSliceType == B_SLICE ) ) )
    {
      if( m_uiBaseLayerId != MSYS_UINT_MAX )
      {
        RNOK( pcWriteIf->writeFlag( m_bBasePredWeightTableFlag,                "PWT: base_pred_weight_table_flag" ) );
      }
      if( ! m_bBasePredWeightTableFlag )
      {
        RNOK( pcWriteIf->writeUvlc( m_uiLumaLog2WeightDenom,                   "PWT: luma_log_weight_denom" ) );
        RNOK( pcWriteIf->writeUvlc( m_uiChromaLog2WeightDenom,                 "PWT: chroma_log_weight_denom" ) );

        RNOK( m_acPredWeightTable[LIST_0].write( pcWriteIf, getNumRefIdxActive( LIST_0 ) ) );
        if( m_eSliceType == B_SLICE )
        {
          RNOK( m_acPredWeightTable[LIST_1].write( pcWriteIf, getNumRefIdxActive( LIST_1) ) );
        }
      }
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

  if(getPPS().getNumSliceGroupsMinus1()>0 && getPPS().getSliceGroupMapType() >=3 && getPPS().getSliceGroupMapType() <= 5)
  {    
    RNOK(     pcWriteIf->writeCode( m_uiSliceGroupChangeCycle, getPPS().getLog2MaxSliceGroupChangeCycle(getSPS().getMbInFrame()) ,                "SH: slice_group_change_cycle" ) );
  }

// TMM_ESS {
  if ((m_eSliceType != F_SLICE) && (getSPS().getExtendedSpatialScalability() > ESS_NONE))
  {
    if ( 1 /* chroma_format_idc */ > 0 )
    {
      RNOK( pcWriteIf->writeCode( m_uiBaseChromaPhaseXPlus1, 2,                  "SH: BaseChromaPhaseXPlus1" ) );
      RNOK( pcWriteIf->writeCode( m_uiBaseChromaPhaseYPlus1, 2,                  "SH: BaseChromaPhaseXPlus1" ) );
    }
    
    if (getSPS().getExtendedSpatialScalability() == ESS_PICT)
    {
      RNOK( pcWriteIf->writeSvlc( m_iScaledBaseLeftOffset,                       "SH: ScaledBaseLeftOffset" ) );
      RNOK( pcWriteIf->writeSvlc( m_iScaledBaseTopOffset,                        "SH: ScaledBaseTopOffset" ) );
      RNOK( pcWriteIf->writeSvlc( m_iScaledBaseRightOffset,                      "SH: ScaledBaseRightOffset" ) );
      RNOK( pcWriteIf->writeSvlc( m_iScaledBaseBottomOffset,                     "SH: ScaledBaseBottomOffset" ) );
    }
  }
// TMM_ESS }

  if( m_eSliceType == F_SLICE )
  {
    RNOK( pcWriteIf->writeFlag( m_bArFgsUsageFlag,                               "SH: base_layer_key_pic_flag" ) );
    if( m_bArFgsUsageFlag )
    {
      // send other information conditionally
      UInt uiWeight;

      // AR_FGS_MAX_BASE_WEIGHT - 1 is not allowed
      uiWeight = ( m_uiBaseWeightZeroBaseBlock >= AR_FGS_MAX_BASE_WEIGHT )
        ? ( AR_FGS_MAX_BASE_WEIGHT - 1 ) : m_uiBaseWeightZeroBaseBlock;
      RNOK( pcWriteIf->writeCode( uiWeight, 5,                                   "SH: base_ref_weight_for_zero_base_block" ) );

      // AR_FGS_MAX_BASE_WEIGHT - 1 is not allowed
      uiWeight = ( m_uiBaseWeightZeroBaseCoeff >= AR_FGS_MAX_BASE_WEIGHT )
        ? ( AR_FGS_MAX_BASE_WEIGHT - 1 ) : m_uiBaseWeightZeroBaseCoeff;
      RNOK( pcWriteIf->writeCode( uiWeight, 5,                                   "SH: base_ref_weight_for_zero_base_coeff" ) );
    }
  }
} //JVT-P031
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
  
  if( getSPS().getPicOrderCntType() == 0 )
  {
  RNOK(     pcWriteIf->writeCode( m_uiPicOrderCntLsb,
                                  getSPS().getLog2MaxPicOrderCntLsb(),          "SH: pic_order_cnt_lsb" ) );
    if( getPPS().getPicOrderPresentFlag() && true /* ! field_pic_flag */ )
    {
      RNOK( pcWriteIf->writeSvlc( m_iDeltaPicOrderCntBottom,                    "SH: delta_pic_order_cnt_bottom" ) );
    }
  }
  if( getSPS().getPicOrderCntType() == 1 && ! getSPS().getDeltaPicOrderAlwaysZeroFlag() )
  {
    RNOK(   pcWriteIf->writeSvlc( m_aiDeltaPicOrderCnt[0],                      "SH: delta_pic_order_cnt[0]" ) );
    if( getPPS().getPicOrderPresentFlag() && true /* ! field_pic_flag */ )
    {
      RNOK( pcWriteIf->writeSvlc( m_aiDeltaPicOrderCnt[1],                      "SH: delta_pic_order_cnt[1]" ) );
    }
  }
  
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

  if( ( getPPS().getWeightedPredFlag ()      && ( m_eSliceType == P_SLICE ) ) ||
      ( getPPS().getWeightedBiPredIdc() == 1 && ( m_eSliceType == B_SLICE ) ) )
  {
    RNOK( pcWriteIf->writeUvlc( m_uiLumaLog2WeightDenom,                      "PWT: luma_log_weight_denom" ) );
    RNOK( pcWriteIf->writeUvlc( m_uiChromaLog2WeightDenom,                    "PWT: chroma_log_weight_denom" ) );

    RNOK( m_acPredWeightTable[LIST_0].write( pcWriteIf, getNumRefIdxActive( LIST_0 ) ) );
    if( m_eSliceType == B_SLICE )
    {
      RNOK( m_acPredWeightTable[LIST_1].write( pcWriteIf, getNumRefIdxActive( LIST_1) ) );
    }
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

  if(getPPS().getNumSliceGroupsMinus1()>0 && getPPS().getSliceGroupMapType() >=3 && getPPS().getSliceGroupMapType() <= 5)
  {    
    RNOK(     pcWriteIf->writeCode( m_uiSliceGroupChangeCycle, getPPS().getLog2MaxSliceGroupChangeCycle(getSPS().getMbInFrame()) ,                "SH: slice_group_change_cycle" ) );
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
  UInt  uiTmp;

  /*if( m_eSliceType == F_SLICE ) // HS: coding order changed to match the text
  {
    RNOK(   pcReadIf->getUvlc( m_uiNumMbsInSlice,                            "SH: num_mbs_in_slice" ) );
    RNOK(   pcReadIf->getFlag( m_bFgsComponentSep,                           "SH: fgs_comp_sep" ) );
  }*/
  RNOK(     pcReadIf->getCode( m_uiFrameNum,
                               getSPS().getLog2MaxFrameNum(),                "SH: frame_num" ) );
  if( m_eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE )
  {
    RNOK(   pcReadIf->getUvlc( m_uiIdrPicId,                                 "SH: idr_pic_id" ) );
  }
  
  if( getSPS().getPicOrderCntType() == 0 )
  {
  RNOK(     pcReadIf->getCode( m_uiPicOrderCntLsb,
                               getSPS().getLog2MaxPicOrderCntLsb(),          "SH: pic_order_cnt_lsb" ) );
    if( getPPS().getPicOrderPresentFlag() && true /* ! field_pic_flag */ )
    {
      RNOK( pcReadIf->getSvlc( m_iDeltaPicOrderCntBottom,                    "SH: delta_pic_order_cnt_bottom" ) );
    }
  }
  if( getSPS().getPicOrderCntType() == 1 && ! getSPS().getDeltaPicOrderAlwaysZeroFlag() )
  {
    RNOK(   pcReadIf->getSvlc( m_aiDeltaPicOrderCnt[0],                      "SH: delta_pic_order_cnt[0]" ) );
    if( getPPS().getPicOrderPresentFlag() && true /* ! field_pic_flag */ )
    {
      RNOK( pcReadIf->getSvlc( m_aiDeltaPicOrderCnt[1],                      "SH: delta_pic_order_cnt[1]" ) );
    }
  }
  
  if( m_eSliceType == B_SLICE )
  {
    RNOK(   pcReadIf->getFlag( m_bDirectSpatialMvPredFlag,                   "SH: direct_spatial_mv_pred_flag" ) );
  }
  
  if( m_eSliceType != F_SLICE )
  {
    RNOK(   pcReadIf->getUvlc( uiTmp,                                        "SH: base_id_plus1" ) );
    m_uiBaseLayerId = uiTmp - 1;
    if( m_uiBaseLayerId != MSYS_UINT_MAX )
    {
	    m_uiBaseFragmentOrder = m_uiBaseLayerId & 0x03;
      m_uiBaseQualityLevel = (m_uiBaseLayerId >> 2) & 0x03;
	    m_uiBaseLayerId = m_uiBaseLayerId >> 4;
    }
    else
    {
      m_uiBaseQualityLevel = 0;
    }

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

    RNOK( m_acPredWeightTable[LIST_0].init( 64 ) );
    RNOK( m_acPredWeightTable[LIST_1].init( 64 ) );

    if( ( getPPS().getWeightedPredFlag ()      && ( m_eSliceType == P_SLICE ) ) ||
        ( getPPS().getWeightedBiPredIdc() == 1 && ( m_eSliceType == B_SLICE ) ) )
    {
      if( m_uiBaseLayerId != MSYS_UINT_MAX )
      {
        RNOK( pcReadIf->getFlag( m_bBasePredWeightTableFlag,                "PWT: base_pred_weight_table_flag" ) );
      }
      if( ! m_bBasePredWeightTableFlag )
      {
        RNOK( pcReadIf->getUvlc( m_uiLumaLog2WeightDenom,                   "PWT: luma_log_weight_denom" ) );
        RNOK( pcReadIf->getUvlc( m_uiChromaLog2WeightDenom,                 "PWT: chroma_log_weight_denom" ) );
        ROTR( m_uiLumaLog2WeightDenom   > 7, Err::m_nInvalidParameter );
        ROTR( m_uiChromaLog2WeightDenom > 7, Err::m_nInvalidParameter );

        RNOK( m_acPredWeightTable[LIST_0].initDefaults( m_uiLumaLog2WeightDenom, m_uiChromaLog2WeightDenom ) );
        RNOK( m_acPredWeightTable[LIST_0].read( pcReadIf, getNumRefIdxActive( LIST_0 ) ) );
        if( m_eSliceType == B_SLICE )
        {
          RNOK( m_acPredWeightTable[LIST_1].initDefaults( m_uiLumaLog2WeightDenom, m_uiChromaLog2WeightDenom ) );
          RNOK( m_acPredWeightTable[LIST_1].read( pcReadIf, getNumRefIdxActive( LIST_1) ) );
        }
      }
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

  UInt uiSliceGroupChangeCycle;
  if( getPPS().getNumSliceGroupsMinus1()> 0  && getPPS().getSliceGroupMapType() >= 3  &&  getPPS().getSliceGroupMapType() <= 5)
  {
	  UInt pictureSizeInMB = getSPS().getFrameHeightInMbs()*getSPS().getFrameWidthInMbs();
	  RNOK(     pcReadIf->getCode( uiSliceGroupChangeCycle, getLog2MaxSliceGroupChangeCycle(pictureSizeInMB), "SH: slice_group_change_cycle" ) ); 
	  setSliceGroupChangeCycle(uiSliceGroupChangeCycle);
  }

// TMM_ESS {
  if ((m_eSliceType != F_SLICE) && (getSPS().getExtendedSpatialScalability() > ESS_NONE))
  {
    if ( 1 /* chroma_format_idc */ > 0 )
    {
      RNOK( pcReadIf->getCode( m_uiBaseChromaPhaseXPlus1, 2,                 "SH: BaseChromaPhaseXPlus1" ) );
      RNOK( pcReadIf->getCode( m_uiBaseChromaPhaseYPlus1, 2,                 "SH: BaseChromaPhaseYPlus1" ) );
    }

    if (getSPS().getExtendedSpatialScalability() == ESS_PICT)
    {
      RNOK( pcReadIf->getSvlc( m_iScaledBaseLeftOffset,                          "SH: ScaledBaseLeftOffset" ) );
      RNOK( pcReadIf->getSvlc( m_iScaledBaseTopOffset,                           "SH: ScaledBaseTopOffset" ) );
      RNOK( pcReadIf->getSvlc( m_iScaledBaseRightOffset,                         "SH: ScaledBaseRightOffset" ) );
      RNOK( pcReadIf->getSvlc( m_iScaledBaseBottomOffset,                        "SH: ScaledBaseBottomOffset" ) );
    }
  }
// TMM_ESS }

  if( m_eSliceType == F_SLICE )
  {
    RNOK( pcReadIf->getFlag( m_bArFgsUsageFlag,                               "SH: base_layer_key_pic_flag" ) );
    if( m_bArFgsUsageFlag ) 
    {
      // send other information conditionally
      RNOK( pcReadIf->getCode( m_uiBaseWeightZeroBaseBlock, 5,                "SH: base_ref_weight_for_zero_base_block" ) );
      if( m_uiBaseWeightZeroBaseBlock == AR_FGS_MAX_BASE_WEIGHT - 1 )
	      m_uiBaseWeightZeroBaseBlock = AR_FGS_MAX_BASE_WEIGHT;

      RNOK( pcReadIf->getCode( m_uiBaseWeightZeroBaseCoeff, 5,                "SH: base_ref_weight_for_zero_base_coeff" ) );
      if( m_uiBaseWeightZeroBaseCoeff == AR_FGS_MAX_BASE_WEIGHT - 1 )
	      m_uiBaseWeightZeroBaseCoeff = AR_FGS_MAX_BASE_WEIGHT;
    }
    else
    {
      m_uiBaseWeightZeroBaseBlock = AR_FGS_MAX_BASE_WEIGHT;
      m_uiBaseWeightZeroBaseCoeff = AR_FGS_MAX_BASE_WEIGHT;
    }
  }

  return Err::m_nOK;
}



ErrVal
SliceHeaderBase::xReadH264AVCCompatible( HeaderSymbolReadIf* pcReadIf )
{
  Bool  bTmp;
 
  RNOK(     pcReadIf->getCode( m_uiFrameNum,
                               getSPS().getLog2MaxFrameNum(),                "SH: frame_num" ) );
  if( m_eNalUnitType == NAL_UNIT_CODED_SLICE_IDR )
  {
    RNOK(   pcReadIf->getUvlc( m_uiIdrPicId,                                 "SH: idr_pic_id" ) );
  }
  
  if( getSPS().getPicOrderCntType() == 0 )
  {
  RNOK(     pcReadIf->getCode( m_uiPicOrderCntLsb,
                               getSPS().getLog2MaxPicOrderCntLsb(),          "SH: pic_order_cnt_lsb" ) );
    if( getPPS().getPicOrderPresentFlag() && true /* ! field_pic_flag */ )
    {
      RNOK( pcReadIf->getSvlc( m_iDeltaPicOrderCntBottom,                    "SH: delta_pic_order_cnt_bottom" ) );
    }
  }
  if( getSPS().getPicOrderCntType() == 1 && ! getSPS().getDeltaPicOrderAlwaysZeroFlag() )
  {
    RNOK(   pcReadIf->getSvlc( m_aiDeltaPicOrderCnt[0],                      "SH: delta_pic_order_cnt[0]" ) );
    if( getPPS().getPicOrderPresentFlag() && true /* ! field_pic_flag */ )
  {
      RNOK( pcReadIf->getSvlc( m_aiDeltaPicOrderCnt[1],                      "SH: delta_pic_order_cnt[1]" ) );
    }
  }
  
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

  RNOK( m_acPredWeightTable[LIST_0].init( 64 ) );
  RNOK( m_acPredWeightTable[LIST_1].init( 64 ) );

  if( ( getPPS().getWeightedPredFlag ()      && ( m_eSliceType == P_SLICE ) ) ||
      ( getPPS().getWeightedBiPredIdc() == 1 && ( m_eSliceType == B_SLICE ) ) )
  {
    RNOK( pcReadIf->getUvlc( m_uiLumaLog2WeightDenom,                     "PWT: luma_log_weight_denom" ) );
    RNOK( pcReadIf->getUvlc( m_uiChromaLog2WeightDenom,                   "PWT: chroma_log_weight_denom" ) );
    ROTR( m_uiLumaLog2WeightDenom   > 7, Err::m_nInvalidParameter );
    ROTR( m_uiChromaLog2WeightDenom > 7, Err::m_nInvalidParameter );

    RNOK( m_acPredWeightTable[LIST_0].initDefaults( m_uiLumaLog2WeightDenom, m_uiChromaLog2WeightDenom ) );
    RNOK( m_acPredWeightTable[LIST_0].read( pcReadIf, getNumRefIdxActive( LIST_0 ) ) );
    if( m_eSliceType == B_SLICE )
    {
      RNOK( m_acPredWeightTable[LIST_1].initDefaults( m_uiLumaLog2WeightDenom, m_uiChromaLog2WeightDenom ) );
      RNOK( m_acPredWeightTable[LIST_1].read( pcReadIf, getNumRefIdxActive( LIST_1) ) );
    }
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

  //--ICU/ETRI FMO Implementation
  UInt uiSliceGroupChangeCycle;
  if( getPPS().getNumSliceGroupsMinus1()> 0  && getPPS().getSliceGroupMapType() >= 3  &&  getPPS().getSliceGroupMapType() <= 5)
  {
	  UInt pictureSizeInMB = getSPS().getFrameHeightInMbs()*getSPS().getFrameWidthInMbs();

	  RNOK(     pcReadIf->getCode( uiSliceGroupChangeCycle, getLog2MaxSliceGroupChangeCycle(pictureSizeInMB), "SH: slice_group_change_cycle" ) );
 
	   
	  setSliceGroupChangeCycle(uiSliceGroupChangeCycle);
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


//--ICU/ETRI FMO Implementation
ErrVal 
SliceHeaderBase::FMOInit()
{
		
	if(m_pcFMO == NULL)
		m_pcFMO = new FMO();
	else
	{
		m_pcFMO->finit();
		m_pcFMO = new FMO();
	}

	const SequenceParameterSet* pcSPS = &(getSPS());
	const PictureParameterSet* pcPPS = &(getPPS());

	m_pcFMO->img_.field_pic_flag = false;  //interlaced TODO

	m_pcFMO->pps_.num_slice_groups_minus1 = pcPPS->getNumSliceGroupsMinus1();
	m_pcFMO->pps_.slice_group_map_type = pcPPS->getSliceGroupMapType();
	m_pcFMO->img_.PicHeightInMapUnits = pcSPS->getFrameHeightInMbs(); 
	m_pcFMO->img_.PicWidthInMbs = pcSPS->getFrameWidthInMbs();
	m_pcFMO->img_.PicSizeInMbs = pcSPS->getFrameHeightInMbs()*pcSPS->getFrameWidthInMbs();
	m_pcFMO->img_.slice_group_change_cycle = getSliceGroupChangeCycle();
	m_pcFMO->pps_.num_slice_group_map_units_minus1 = pcPPS->getNumSliceGroupMapUnitsMinus1();	  
	m_pcFMO->pps_.copy_run_length_minus1(pcPPS->getArrayRunLengthMinus1());
	m_pcFMO->pps_.copy_top_left(pcPPS->getArrayTopLeft());
	m_pcFMO->pps_.copy_bottom_right(pcPPS->getArrayBottomRight());
	m_pcFMO->pps_.slice_group_change_direction_flag = pcPPS->getSliceGroupChangeDirection_flag();
	m_pcFMO->pps_.slice_group_change_rate_minus1 = pcPPS->getSliceGroupChangeRateMinus1();
	m_pcFMO->pps_.copy_slice_group_id(pcPPS->getArraySliceGroupId());

	m_pcFMO->sps_.pic_height_in_map_units_minus1 =pcSPS->getFrameHeightInMbs()-1;
	m_pcFMO->sps_.pic_width_in_mbs_minus1 = pcSPS->getFrameWidthInMbs()-1;
	m_pcFMO->sps_.frame_mbs_only_flag = 1; // interlaced TODO
	m_pcFMO->sps_.mb_adaptive_frame_field_flag = 0; //

	m_pcFMO->init(&(m_pcFMO->pps_),&(m_pcFMO->sps_));

	m_pcFMO->StartPicture();

	return Err::m_nOK;
}

Int SliceHeaderBase::getNumMbInSlice()
{  
	Int SliceID =m_pcFMO->getSliceGroupId(getFirstMbInSlice());
	return m_pcFMO->getNumMbInSliceGroup(SliceID);
}

H264AVC_NAMESPACE_END

