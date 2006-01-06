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
#include "H264AVCCommonLib/PictureParameterSet.h"
#include "H264AVCCommonLib/SliceHeader.h"
#include "H264AVCCommonLib/TraceFile.h"



H264AVC_NAMESPACE_BEGIN



PictureParameterSet::PictureParameterSet()
: m_eNalUnitType                            ( NAL_UNIT_EXTERNAL )
, m_uiLayerId                               ( 0 )
, m_uiPicParameterSetId                     ( MSYS_UINT_MAX )
, m_uiSeqParameterSetId                     ( MSYS_UINT_MAX )
, m_bEntropyCodingModeFlag                  ( false )
, m_bPicOrderPresentFlag                    ( false )
, m_uiPicInitQp                             ( 26 )
, m_iChomaQpIndexOffset                     ( 0 )
, m_bDeblockingFilterParametersPresentFlag  ( false )
, m_bConstrainedIntraPredFlag               ( false )
, m_bTransform8x8ModeFlag                   ( false )
, m_bPicScalingMatrixPresentFlag            ( false )
, m_iSecondChromaQpIndexOffset              ( 0 )
{
  m_auiNumRefIdxActive[LIST_0] = 0;
  m_auiNumRefIdxActive[LIST_1] = 0;
}


PictureParameterSet::~PictureParameterSet()
{
}


ErrVal
PictureParameterSet::create( PictureParameterSet*& rpcPPS )
{
  rpcPPS = new PictureParameterSet;
  ROT( NULL == rpcPPS );
  return Err::m_nOK;
}


ErrVal
PictureParameterSet::destroy()
{
  delete this;
  return Err::m_nOK;
}


ErrVal
PictureParameterSet::write( HeaderSymbolWriteIf* pcWriteIf ) const
{
  //===== NAL unit header =====
  Bool m_bTraceEnable = true;
  g_nLayer = m_uiLayerId;
  ETRACE_LAYER(m_uiLayerId);
  ETRACE_HEADER( "PICTURE PARAMETER SET" );
  RNOK  ( pcWriteIf->writeFlag( 0,                                        "NALU HEADER: forbidden_zero_bit" ) );
  RNOK  ( pcWriteIf->writeCode( 3, 2,                                     "NALU HEADER: nal_ref_idc" ) );
  RNOK  ( pcWriteIf->writeCode( m_eNalUnitType, 5,                        "NALU HEADER: nal_unit_type" ) );

  //===== NAL unit payload =====
  RNOK( pcWriteIf->writeUvlc( getPicParameterSetId(),                     "PPS: pic_parameter_set_id" ) );
  RNOK( pcWriteIf->writeUvlc( getSeqParameterSetId(),                     "PPS: seq_parameter_set_id" ) );
  RNOK( pcWriteIf->writeFlag( getEntropyCodingModeFlag(),                 "PPS: entropy_coding_mode_flag" ) );
  RNOK( pcWriteIf->writeFlag( getPicOrderPresentFlag(),                   "PPS: pic_order_present_flag" ) );
   
  //--ICU/ETRI FMO Implementation : FMO stuff start
  Int iNumberBitsPerSliceGroupId;
  RNOK( pcWriteIf->writeUvlc( getNumSliceGroupsMinus1(),                         "PPS: num_slice_groups_minus1" ) );  
    
  if(getNumSliceGroupsMinus1() > 0)
  {	  
	  RNOK( pcWriteIf->writeUvlc( getSliceGroupMapType(),                             "PPS: slice_group_map_type" ) );
	  if(getSliceGroupMapType() ==0)
	  {
		  for(int iSliceGroup=0;iSliceGroup<=getNumSliceGroupsMinus1();iSliceGroup++)
		  {
			  RNOK( pcWriteIf->writeUvlc( getRunLengthMinus1(iSliceGroup),                             "PPS: run_length_minus1 [iSliceGroup]" ) );
		  }
	  }
	  else if (getSliceGroupMapType() ==2)
	  {
		  for(int iSliceGroup=0;iSliceGroup<getNumSliceGroupsMinus1();iSliceGroup++)
		  {
			  RNOK( pcWriteIf->writeUvlc( getTopLeft(iSliceGroup),                             "PPS: top_left [iSliceGroup]" ) );
			  RNOK( pcWriteIf->writeUvlc( getBottomRight(iSliceGroup),                             "PPS: bottom_right [iSliceGroup]" ) );
		  }
	  }
	  else if(getSliceGroupMapType() ==3 || 
		  getSliceGroupMapType() ==4 || 
		  getSliceGroupMapType() ==5)
	  {
		  RNOK( pcWriteIf->writeFlag( getSliceGroupChangeDirection_flag(),                      "PPS: slice_group_change_direction_flag" ) );
		  RNOK( pcWriteIf->writeUvlc( getSliceGroupChangeRateMinus1(),                             "PPS: slice_group_change_rate_minus1" ) );
	  }
	  else if (getSliceGroupMapType() ==6)
	  {
		  if (getNumSliceGroupsMinus1()+1 >4)
			  iNumberBitsPerSliceGroupId = 3;
		  else if (getNumSliceGroupsMinus1() > 2)
			  iNumberBitsPerSliceGroupId = 2;
		  else
			  iNumberBitsPerSliceGroupId = 1;
		  //! JVT-F078, exlicitly signal number of MBs in the map
		  RNOK( pcWriteIf->writeUvlc( getNumSliceGroupMapUnitsMinus1(),                             "PPS: num_slice_group_map_units_minus1" ) );
		  for (int iSliceGroup=0; iSliceGroup<=getNumSliceGroupMapUnitsMinus1(); iSliceGroup++)
			  RNOK( pcWriteIf->writeCode( getSliceGroupId(iSliceGroup), iNumberBitsPerSliceGroupId ,                                    "PPS: slice_group_id[iSliceGroup]" ) );
	  }
	  
  }
  //--ICU/ETRI FMO Implementation : FMO stuff end
    
  RNOK( pcWriteIf->writeUvlc( getNumRefIdxActive(LIST_0)-1,               "PPS: num_ref_idx_l0_active_minus1" ) );
  RNOK( pcWriteIf->writeUvlc( getNumRefIdxActive(LIST_1)-1,               "PPS: num_ref_idx_l1_active_minus1" ) );
  RNOK( pcWriteIf->writeFlag( false,                                      "PPS: weighted_pred_flag" ) );
  RNOK( pcWriteIf->writeCode( 0, 2,                                       "PPS: weighted_bipred_idc" ) );
  RNOK( pcWriteIf->writeSvlc( (Int)getPicInitQp() - 26,                   "PPS: pic_init_qp_minus26" ) );
  RNOK( pcWriteIf->writeSvlc( 0,                                          "PPS: pic_init_qs_minus26" ) );
  RNOK( pcWriteIf->writeSvlc( getChomaQpIndexOffset(),                    "PPS: chroma_qp_index_offset" ) );
  RNOK( pcWriteIf->writeFlag( getDeblockingFilterParametersPresentFlag(), "PPS: deblocking_filter_parameters_present_flag" ) );
  RNOK( pcWriteIf->writeFlag( getConstrainedIntraPredFlag(),              "PPS: constrained_intra_pred_flag" ) );
  RNOK( pcWriteIf->writeFlag( false,                                      "PPS: redundant_pic_cnt_present_flag" ) );

  if( getTransform8x8ModeFlag() || m_bPicScalingMatrixPresentFlag || m_iSecondChromaQpIndexOffset != m_iChomaQpIndexOffset )
  {
    RNOK( xWriteFrext( pcWriteIf ) );
  }

  return Err::m_nOK;
}


ErrVal
PictureParameterSet::read( HeaderSymbolReadIf*  pcReadIf,
                           NalUnitType          eNalUnitType )
{
  //===== NAL unit header =====
  setNalUnitType    ( eNalUnitType );


  Bool  bTmp;
  UInt  uiTmp;
  Int   iTmp;

  //--ICU/ETRI FMO Implementation
  Int iNumberBitsPerSliceGroupId;


  RNOK( pcReadIf->getUvlc( m_uiPicParameterSetId,                         "PPS: pic_parameter_set_id" ) );
  ROT ( m_uiPicParameterSetId > 255 );
  RNOK( pcReadIf->getUvlc( m_uiSeqParameterSetId,                         "PPS: seq_parameter_set_id" ) );
  ROT ( m_uiSeqParameterSetId > 31 );
  RNOK( pcReadIf->getFlag( m_bEntropyCodingModeFlag,                      "PPS: entropy_coding_mode_flag" ) );
  RNOK( pcReadIf->getFlag( m_bPicOrderPresentFlag,                        "PPS: pic_order_present_flag" ) );

  //--ICU/ETRI FMO Implementation : FMO stuff start
  RNOK( pcReadIf->getUvlc( m_uiNumSliceGroupsMinus1,                         "PPS: num_slice_groups_minus1" ) );  
  ROT ( m_uiNumSliceGroupsMinus1 > MAXNumSliceGroupsMinus1);
  
  if(m_uiNumSliceGroupsMinus1 > 0)
  {	  
	  RNOK( pcReadIf->getUvlc( m_uiSliceGroupMapType,                             "PPS: slice_group_map_type" ) );
	  if(m_uiSliceGroupMapType ==0)
	  {
		  for(int i=0;i<=m_uiNumSliceGroupsMinus1;i++)
		  {
			  RNOK( pcReadIf->getUvlc( m_uiRunLengthMinus1[i],                             "PPS: run_length_minus1 [i]" ) );
		  }
	  }
	  else if (m_uiSliceGroupMapType ==2)
	  {
		  for(int i=0;i<m_uiNumSliceGroupsMinus1;i++)
		  {
			  RNOK( pcReadIf->getUvlc( m_uiTopLeft[i],                             "PPS: top_left [i]" ) );
			  RNOK( pcReadIf->getUvlc( m_uiBottomRight[i],                             "PPS: bottom_right [i]" ) );
		  }
	  }
	  else if(m_uiSliceGroupMapType ==3 || 
		  m_uiSliceGroupMapType ==4 || 
		  m_uiSliceGroupMapType ==5)
	  {
		  RNOK( pcReadIf->getFlag( m_bSliceGroupChangeDirection_flag,                      "PPS: slice_group_change_direction_flag" ) );
		  RNOK( pcReadIf->getUvlc( m_uiSliceGroupChangeRateMinus1,                             "PPS: slice_group_change_rate_minus1" ) );
	  }
	  else if (m_uiSliceGroupMapType ==6)
	  {
		  if (m_uiNumSliceGroupsMinus1+1 >4)
			  iNumberBitsPerSliceGroupId = 3;
		  else if (m_uiNumSliceGroupsMinus1+1 > 2)
			  iNumberBitsPerSliceGroupId = 2;
		  else
			  iNumberBitsPerSliceGroupId = 1;
		  //! JVT-F078, exlicitly signal number of MBs in the map
		  RNOK( pcReadIf->getUvlc( m_uiNumSliceGroupMapUnitsMinus1,                             "PPS: num_slice_group_map_units_minus1" ) );
		  for (int i=0; i<=m_uiNumSliceGroupMapUnitsMinus1; i++)
			  RNOK( pcReadIf->getCode( m_uiSliceGroupId[i], iNumberBitsPerSliceGroupId ,                                    "PPS: slice_group_id[i]" ) );
	  }
	  
  }
  //--ICU/ETRI FMO Implementation : FMO stuff end  


  RNOK( pcReadIf->getUvlc( uiTmp,                                         "PPS: num_ref_idx_l0_active_minus1" ) );
  ROT ( uiTmp > 14 );
  setNumRefIdxActive( LIST_0, uiTmp + 1 );
  RNOK( pcReadIf->getUvlc( uiTmp,                                         "PPS: num_ref_idx_l1_active_minus1" ) );
  ROT ( uiTmp > 14 );
  setNumRefIdxActive( LIST_1, uiTmp + 1 );
  RNOK( pcReadIf->getFlag( bTmp,                                          "PPS: weighted_pred_flag" ) );
  ROT ( bTmp );
  RNOK( pcReadIf->getCode( uiTmp, 2,                                      "PPS: weighted_bipred_idc" ) );
  ROT ( uiTmp );
  RNOK( pcReadIf->getSvlc( iTmp,                                          "PPS: pic_init_qp_minus26" ) );
  ROT ( iTmp < -26 || iTmp > 25 );
  setPicInitQp( (UInt)( iTmp + 26 ) );
  RNOK( pcReadIf->getSvlc( iTmp,                                          "PPS: pic_init_qs_minus26" ) );
  RNOK( pcReadIf->getSvlc( iTmp,                                          "PPS: chroma_qp_index_offset" ) );
  ROT ( iTmp < -12 || iTmp > 12 );
  setChomaQpIndexOffset( iTmp );
  RNOK( pcReadIf->getFlag( m_bDeblockingFilterParametersPresentFlag,      "PPS: deblocking_filter_parameters_present_flag" ) );
  RNOK( pcReadIf->getFlag( m_bConstrainedIntraPredFlag,                   "PPS: constrained_intra_pred_flag" ) );
  RNOK( pcReadIf->getFlag( bTmp,                                          "PPS: redundant_pic_cnt_present_flag" ) );
  ROT ( bTmp );
  RNOK( xReadFrext( pcReadIf ) );

  return Err::m_nOK;
}


ErrVal
PictureParameterSet::xWriteFrext( HeaderSymbolWriteIf* pcWriteIf ) const
{
  RNOK  ( pcWriteIf->writeFlag( m_bTransform8x8ModeFlag,                  "PPS: transform_8x8_mode_flag" ) );
  RNOK  ( pcWriteIf->writeFlag( m_bPicScalingMatrixPresentFlag,           "PPS: pic_scaling_matrix_present_flag" ) );
  if( m_bPicScalingMatrixPresentFlag )
  {
    RNOK( m_cPicScalingMatrix.write( pcWriteIf, m_bTransform8x8ModeFlag ) );
  }
  RNOK  ( pcWriteIf->writeSvlc( m_iSecondChromaQpIndexOffset,             "PPS: second_chroma_qp_index_offset" ) );

  return Err::m_nOK;
}


ErrVal
PictureParameterSet::xReadFrext( HeaderSymbolReadIf* pcReadIf )
{
  ROTRS( ! pcReadIf->moreRBSPData(), Err::m_nOK );

  RNOK  ( pcReadIf->getFlag ( m_bTransform8x8ModeFlag,                    "PPS: transform_8x8_mode_flag" ) );
  RNOK  ( pcReadIf->getFlag ( m_bPicScalingMatrixPresentFlag,             "PPS: pic_scaling_matrix_present_flag" ) );
  if( m_bPicScalingMatrixPresentFlag )
  {
    RNOK( m_cPicScalingMatrix.read( pcReadIf, m_bTransform8x8ModeFlag ) );
  }
  RNOK  ( pcReadIf->getSvlc ( m_iSecondChromaQpIndexOffset,               "PPS: second_chroma_qp_index_offset" ) );

  return Err::m_nOK;
}


H264AVC_NAMESPACE_END
