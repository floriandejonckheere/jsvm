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
#include "H264AVCCommonLib/Sei.h"
#include "H264AVCCommonLib/ParameterSetMng.h"
#include "H264AVCCommonLib/TraceFile.h"



H264AVC_NAMESPACE_BEGIN




ErrVal
SEI::read( HeaderSymbolReadIf* pcReadIf,
           MessageList&        rcSEIMessageList ) 
{
  ROT( NULL == pcReadIf);

  while( pcReadIf->moreRBSPData() )
  {
    SEIMessage* pcActualSEIMessage = NULL;

    RNOK( xRead( pcReadIf, pcActualSEIMessage ));

    rcSEIMessageList.push_back( pcActualSEIMessage );
  }
  return Err::m_nOK;
}


ErrVal
SEI::write( HeaderSymbolWriteIf*  pcWriteIf,
            HeaderSymbolWriteIf*  pcWriteTestIf,
            MessageList*          rpcSEIMessageList )
{
  ROT( NULL == pcWriteIf);
  ROT( NULL == pcWriteTestIf);
  ROT( NULL == rpcSEIMessageList);

  //===== NAL unit header =====
  ETRACE_DECLARE( Bool m_bTraceEnable = true );
  g_nLayer = 0;
  ETRACE_LAYER(0);
  ETRACE_HEADER( "SEI MESSAGE" );
  RNOK  ( pcWriteIf->writeFlag( 0,                "NALU HEADER: forbidden_zero_bit" ) );
  RNOK  ( pcWriteIf->writeCode( 0, 2,             "NALU HEADER: nal_ref_idc" ) );
  RNOK  ( pcWriteIf->writeCode( NAL_UNIT_SEI, 5,  "NALU HEADER: nal_unit_type" ) );

  while( rpcSEIMessageList->size() )
  {
    RNOK( xWrite( pcWriteIf, pcWriteTestIf, rpcSEIMessageList->front() ) );
    SEIMessage* pcTop = rpcSEIMessageList->front();
    rpcSEIMessageList->pop_front();
    delete pcTop;
  }
  return Err::m_nOK;
}


ErrVal
SEI::xRead( HeaderSymbolReadIf* pcReadIf,
            SEIMessage*&        rpcSEIMessage) 
{
  MessageType eMessageType = RESERVED_SEI;
  UInt        uiSize       = 0;

  RNOK( xReadPayloadHeader( pcReadIf, eMessageType, uiSize) );

  RNOK( xCreate( rpcSEIMessage, eMessageType, uiSize ) )

  RNOK( rpcSEIMessage->read( pcReadIf ) );
  RNOK( pcReadIf->readByteAlign() );
  return Err::m_nOK;
}


ErrVal
SEI::xWrite( HeaderSymbolWriteIf* pcWriteIf,
             HeaderSymbolWriteIf* pcWriteTestIf,
             SEIMessage*          pcSEIMessage )
{

  UInt uiStart  = pcWriteTestIf->getNumberOfWrittenBits();
  RNOK( pcSEIMessage->write( pcWriteTestIf ) );
  UInt uiBits = pcWriteTestIf->getNumberOfWrittenBits() - uiStart;

  UInt uiSize = (uiBits+7)/8;

  RNOK( xWritePayloadHeader( pcWriteIf, pcSEIMessage->getMessageType(), uiSize ) );
  RNOK( pcSEIMessage->write( pcWriteIf ) );
  UInt uiAlignedBits = 8 - (uiBits&7);
  if( uiAlignedBits != 0 && uiAlignedBits != 8)
  {
    RNOK( pcWriteIf->writeCode( 1<<(uiAlignedBits-1), uiAlignedBits, "SEI: alignment_bits" ) );
  }
  return Err::m_nOK;
}



ErrVal
SEI::xReadPayloadHeader( HeaderSymbolReadIf*  pcReadIf,
                         MessageType&         reMessageType,
                         UInt&                ruiSize)
{
  { // type
    UInt uiTemp = 0xFF;
    UInt uiSum  = 0;
    while( 0xFF == uiTemp )
    {
      RNOK( pcReadIf->getCode( uiTemp, 8, "SEI: payload type") );
      uiSum += uiTemp;
    }
    reMessageType = (RESERVED_SEI <= uiSum ) ? RESERVED_SEI : MessageType( uiSum );
  }

  { // size
    UInt uiTemp  = 0xFF;
    UInt uiSum  = 0;

    while( 0xFF == uiTemp )
    {
      RNOK( pcReadIf->getCode( uiTemp, 8, "SEI: payload size") );
      uiSum += uiTemp;
    }
    ruiSize = uiSum;
  }
  return Err::m_nOK;
}



ErrVal
SEI::xCreate( SEIMessage*&  rpcSEIMessage,
              MessageType   eMessageType,
              UInt          uiSize ) 
{
  switch( eMessageType )
  {
    case SUB_SEQ_INFO:  return SubSeqInfo ::create( (SubSeqInfo*&)  rpcSEIMessage );
    case SCALABLE_SEI:  return ScalableSei::create( (ScalableSei*&) rpcSEIMessage );
    case SUB_PIC_SEI:   return SubPicSei::create	( (SubPicSei*&)		rpcSEIMessage );
    //{{Quality level estimation and modified truncation- JVTO044 and m12007
    //France Telecom R&D-(nathalie.cammas@francetelecom.com)
    case QUALITYLEVEL_SEI: return QualityLevelSEI::create((QualityLevelSEI*&) rpcSEIMessage);
    //}}Quality level estimation and modified truncation- JVTO044 and m12007
  	case NON_REQUIRED_SEI: return NonRequiredSei::create((NonRequiredSei*&) rpcSEIMessage); 
	// JVT-S080 LMI {
    case SCALABLE_SEI_LAYERS_NOT_PRESENT:  return ScalableSeiLayersNotPresent::create( (ScalableSeiLayersNotPresent*&) rpcSEIMessage );
	case SCALABLE_SEI_DEPENDENCY_CHANGE:   return ScalableSeiDependencyChange::create( (ScalableSeiDependencyChange*&) rpcSEIMessage );
	// JVT-S080 LMI }
    default :           return ReservedSei::create( (ReservedSei*&) rpcSEIMessage, uiSize );
  }
  //return Err::m_nOK;
}


ErrVal
SEI::xWritePayloadHeader( HeaderSymbolWriteIf*  pcWriteIf,
                          MessageType           eMessageType,
                          UInt                  uiSize )
{
  { // type
    UInt uiTemp = eMessageType;
    UInt uiByte = 0xFF;

    while( 0xFF == uiByte )
    {
      uiByte  = (0xFF > uiTemp) ? uiTemp : 0xff;
      uiTemp -= 0xFF;
      RNOK( pcWriteIf->writeCode( uiByte, 8, "SEI: payload type") );
    }
  }

  { // size
    UInt uiTemp = uiSize;
    UInt uiByte = 0xFF;

    while( 0xFF == uiByte )
    {
      uiByte  = (0xFF > uiTemp) ? uiTemp : 0xff;
      uiTemp -= 0xFF;
      RNOK( pcWriteIf->writeCode( uiByte, 8, "SEI: payload size") );
    }
  }

  return Err::m_nOK;
}





ErrVal
SEI::ReservedSei::create( ReservedSei*& rpcReservedSei,
                          UInt          uiSize )
{
  rpcReservedSei = new ReservedSei (uiSize);
  ROT( NULL == rpcReservedSei );
  return Err::m_nOK;
}


ErrVal
SEI::ReservedSei::write( HeaderSymbolWriteIf* pcWriteIf )
{
  AF();
  return Err::m_nOK;
}


ErrVal
SEI::ReservedSei::read( HeaderSymbolReadIf* pcReadIf )
{
  AF();
  return Err::m_nOK;
}





//////////////////////////////////////////////////////////////////////////
// 
//      S U B S E Q U E N C E     S E I
//
//////////////////////////////////////////////////////////////////////////

ErrVal
SEI::SubSeqInfo::create( SubSeqInfo*& rpcSEIMessage )
{
  SubSeqInfo* pcSubSeqInfo = new SubSeqInfo();
  rpcSEIMessage = pcSubSeqInfo;
  ROT( NULL == rpcSEIMessage )
  return Err::m_nOK;
}


ErrVal
SEI::SubSeqInfo::write( HeaderSymbolWriteIf* pcWriteIf )
{
  RNOK(   pcWriteIf->writeUvlc( m_uiSubSeqLayerNum,       "SubSeqSEI: sub_seq_layer_num") );
  RNOK(   pcWriteIf->writeUvlc( m_uiSubSeqId,             "SubSeqSEI: sub_seq_layer_id") );
  RNOK(   pcWriteIf->writeFlag( m_bFirstRefPicFlag,       "SubSeqSEI: first_ref_pic_flag" ) );
  RNOK(   pcWriteIf->writeFlag( m_bLeadingNonRefPicFlag,  "SubSeqSEI: leading_non_ref_pic_flag" ) );
  RNOK(   pcWriteIf->writeFlag( m_bLastPicFlag,           "SubSeqSEI: last_pic_flag" ) );
  RNOK(   pcWriteIf->writeFlag( m_bSubSeqFrameNumFlag,    "SubSeqSEI: sub_seq_frame_num_flag" ) );
  if( m_bSubSeqFrameNumFlag )
  {
    RNOK( pcWriteIf->writeUvlc( m_uiSubSeqFrameNum,       "SubSeqSEI: sub_seq_frame_num") );
  }
  return Err::m_nOK;
}


ErrVal
SEI::SubSeqInfo::read ( HeaderSymbolReadIf* pcReadIf )
{
  RNOK(   pcReadIf->getUvlc( m_uiSubSeqLayerNum,       "SubSeqSEI: sub_seq_layer_num") );
  RNOK(   pcReadIf->getUvlc( m_uiSubSeqId,             "SubSeqSEI: sub_seq_layer_id") );
  RNOK(   pcReadIf->getFlag( m_bFirstRefPicFlag,       "SubSeqSEI: first_ref_pic_flag" ) );
  RNOK(   pcReadIf->getFlag( m_bLeadingNonRefPicFlag,  "SubSeqSEI: leading_non_ref_pic_flag" ) );
  RNOK(   pcReadIf->getFlag( m_bLastPicFlag,           "SubSeqSEI: last_pic_flag" ) );
  RNOK(   pcReadIf->getFlag( m_bSubSeqFrameNumFlag,    "SubSeqSEI: sub_seq_frame_num_flag" ) );
  if( m_bSubSeqFrameNumFlag )
  {
    RNOK( pcReadIf->getUvlc( m_uiSubSeqFrameNum,       "SubSeqSEI: sub_seq_frame_num") );
  }
  return Err::m_nOK;
}


ErrVal
SEI::SubSeqInfo::init( UInt  uiSubSeqLayerNum,
                       UInt  uiSubSeqId,
                       Bool  bFirstRefPicFlag,
                       Bool  bLeadingNonRefPicFlag,
                       Bool  bLastPicFlag,
                       Bool  bSubSeqFrameNumFlag,
                       UInt  uiSubSeqFrameNum ) 

{
  m_uiSubSeqLayerNum      = uiSubSeqLayerNum;
  m_uiSubSeqId            = uiSubSeqId;
  m_bFirstRefPicFlag      = bFirstRefPicFlag;
  m_bLeadingNonRefPicFlag = bLeadingNonRefPicFlag;
  m_bLastPicFlag          = bLastPicFlag;
  m_bSubSeqFrameNumFlag   = bSubSeqFrameNumFlag;
  m_uiSubSeqFrameNum      = uiSubSeqFrameNum;
  return Err::m_nOK;
}





//////////////////////////////////////////////////////////////////////////
// 
//      S C A L A B L E     S E I
//
//////////////////////////////////////////////////////////////////////////

SEI::ScalableSei::ScalableSei	() 
: SEIMessage									( SCALABLE_SEI )
, m_num_layers_minus1					( 0	)
{	
	::memset( m_layer_id, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
//	::memset( m_fgs_layer_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );  //JVT-S036 lsj
	::memset( m_sub_pic_layer_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
	::memset( m_sub_region_layer_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
	::memset( m_iroi_slice_division_info_present_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) ); //JVT-S036 lsj
	::memset( m_profile_level_info_present_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
	::memset( m_bitrate_info_present_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
	::memset( m_frm_rate_info_present_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
	::memset( m_frm_size_info_present_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
	::memset( m_layer_dependency_info_present_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
	::memset( m_init_parameter_sets_info_present_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );

	::memset( m_exact_interlayer_pred_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );  //JVT-S036 lsj

	::memset( m_layer_profile_idc, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_layer_constraint_set0_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
	::memset( m_layer_constraint_set1_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
	::memset( m_layer_constraint_set2_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
	::memset( m_layer_constraint_set3_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
	::memset( m_layer_level_idc, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );

//JVT-S036 lsj start
	::memset( m_profile_level_info_src_layer_id_delta, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );//

	::memset( m_simple_priority_id, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );//
    ::memset( m_discardable_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );//
	::memset( m_temporal_level, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_dependency_id, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_quality_level, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );

	::memset( m_avg_bitrate, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_max_bitrate_layer, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );//
	::memset( m_max_bitrate_decoded_picture, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );//
	::memset( m_max_bitrate_calc_window, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );//

	::memset( m_constant_frm_rate_idc, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_avg_frm_rate, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );

	::memset( m_frm_rate_info_src_layer_id_delta, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );//

	::memset( m_frm_width_in_mbs_minus1, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_frm_height_in_mbs_minus1, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );

	::memset( m_frm_size_info_src_layer_id_delta, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );//

	::memset( m_base_region_layer_id, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_dynamic_rect_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
	::memset( m_horizontal_offset, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_vertical_offset, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_region_width, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_region_height, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );

	::memset( m_sub_region_info_src_layer_id_delta, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) ); //

	::memset( m_roi_id, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );

	::memset( m_iroi_slice_division_type, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_grid_slice_width_in_mbs_minus1, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_grid_slice_height_in_mbs_minus1, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_num_slice_minus1, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_first_mb_in_slice, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt*) );
	::memset( m_slice_width_in_mbs_minus1, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt*) );
	::memset( m_slice_height_in_mbs_minus1, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt*) );
	::memset( m_slice_id, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt*) );

	::memset( m_num_directly_dependent_layers, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_directly_dependent_layer_id_delta_minus1, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt*) ); //

	::memset( m_layer_dependency_info_src_layer_id_delta, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) ); //
//JVT-S036 lsj end

	::memset( m_num_init_seq_parameter_set_minus1, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_init_seq_parameter_set_id_delta, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt*) );
	::memset( m_num_init_pic_parameter_set_minus1, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_init_pic_parameter_set_id_delta, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt*) );

	::memset( m_init_parameter_sets_info_src_layer_id_delta, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) ); //JVT-S036 lsj

}


SEI::ScalableSei::~ScalableSei()
{
}

ErrVal
SEI::ScalableSei::create( ScalableSei*& rpcSeiMessage )
{
	rpcSeiMessage = new ScalableSei();
	ROT( NULL == rpcSeiMessage )
		return Err::m_nOK;
}


ErrVal
SEI::ScalableSei::write( HeaderSymbolWriteIf *pcWriteIf )
{
  UInt i=0, j=0;

	ROF( m_num_layers_minus1+1 );
	RNOK		( pcWriteIf->writeUvlc(m_num_layers_minus1,													"ScalableSEI: num_layers_minus1"											) );
	for( i = 0; i <= m_num_layers_minus1; i++ )
	{
		RNOK	( pcWriteIf->writeCode( m_layer_id[i],												8,		"ScalableSEI: layer_id"															) );
	//JVT-S036 lsj start
		//RNOK	( pcWriteIf->writeFlag( m_fgs_layer_flag[i],												"ScalableSEI: fgs_layer_flag"												) );
		RNOK	( pcWriteIf->writeCode( m_simple_priority_id[i],					6,		"ScalableSEI: simple_priority_id"										) ); 
		RNOK    ( pcWriteIf->writeFlag( m_discardable_flag[i],								"ScalableSEI: discardable_flag"											) );  
		RNOK	( pcWriteIf->writeCode( m_temporal_level[i],								3,		"ScalableSEI: temporal_level"												) );
		RNOK	( pcWriteIf->writeCode( m_dependency_id[i],							3,		"ScalableSEI: dependency_level"											) );
		RNOK	( pcWriteIf->writeCode( m_quality_level[i],									2,		"ScalableSEI: quality_level"													) );
	
		RNOK	( pcWriteIf->writeFlag( m_sub_pic_layer_flag[i],										"ScalableSEI: sub_pic_layer_flag"										) );
		RNOK	( pcWriteIf->writeFlag( m_sub_region_layer_flag[i],									"ScalableSEI: sub_region_layer_flag"									) );
		RNOK	( pcWriteIf->writeFlag( m_iroi_slice_division_info_present_flag[i],					"ScalableSEI: iroi_slice_division_info_present_flag"					) ); 
		RNOK	( pcWriteIf->writeFlag( m_profile_level_info_present_flag[i],				"ScalableSEI: profile_level_info_present_flag"				) );
	//JVT-S036 lsj end
		RNOK	( pcWriteIf->writeFlag( m_bitrate_info_present_flag[i],							"ScalableSEI: bitrate_info_present_flag"							) );
		RNOK	( pcWriteIf->writeFlag( m_frm_rate_info_present_flag[i],						"ScalableSEI: frm_rate_info_present_flag"						) );
		RNOK	( pcWriteIf->writeFlag( m_frm_size_info_present_flag[i],						"ScalableSEI: frm_size_info_present_flag"						) );
		RNOK	( pcWriteIf->writeFlag( m_layer_dependency_info_present_flag[i],		"ScalableSEI: layer_dependency_info_present_flag"		) );
		RNOK	( pcWriteIf->writeFlag( m_init_parameter_sets_info_present_flag[i],	"ScalableSEI: init_parameter_sets_info_present_flag" ) );
		RNOK	( pcWriteIf->writeFlag( m_exact_interlayer_pred_flag[i],						"ScalableSEI: exact_interlayer_pred_flag"                      ) );//JVT-S036 lsj 

		if ( m_profile_level_info_present_flag[i] )
		{
			RNOK	( pcWriteIf->writeCode( m_layer_profile_idc[i],							8,		"ScalableSEI: layer_profile_idc"											) );
			RNOK	( pcWriteIf->writeFlag( m_layer_constraint_set0_flag[i],					"ScalableSEI: layer_constraint_set0_flag"						) );
			RNOK	( pcWriteIf->writeFlag( m_layer_constraint_set1_flag[i],					"ScalableSEI: layer_constraint_set1_flag"						) );
			RNOK	( pcWriteIf->writeFlag( m_layer_constraint_set2_flag[i],					"ScalableSEI: layer_constraint_set2_flag"						) );
			RNOK	( pcWriteIf->writeFlag( m_layer_constraint_set3_flag[i],					"ScalableSEI: layer_constraint_set3_flag"						) );
			RNOK	( pcWriteIf->writeCode( 0,																	4,		"ScalableSEI: reserved_zero_4bits"										) );
			RNOK	( pcWriteIf->writeCode( m_layer_level_idc[i],								8,		"ScalableSEI: layer_level_idc"												) );
		}

		else
		{//JVT-S036 lsj
			RNOK	( pcWriteIf->writeUvlc( m_profile_level_info_src_layer_id_delta[i], "ScalableSEI: profile_level_info_src_layer_id_delta"      ) ); 
		}

/*		if ( m_decoding_dependency_info_present_flag[i] )
		{
			RNOK	( pcWriteIf->writeCode( m_simple_priority_id[i],					6,		"ScalableSEI: simple_priority_id"										) ); 
			RNOK    ( pcWriteIf->writeFlag( m_discardable_flag[i],								"ScalableSEI: discardable_flag"											) );  
			RNOK	( pcWriteIf->writeCode( m_temporal_level[i],								3,		"ScalableSEI: temporal_level"												) );
			RNOK	( pcWriteIf->writeCode( m_dependency_id[i],							3,		"ScalableSEI: dependency_level"											) );
			RNOK	( pcWriteIf->writeCode( m_quality_level[i],									2,		"ScalableSEI: quality_level"													) );
		}
JVT-S036 lsj */
		if ( m_bitrate_info_present_flag[i] )
		{
			RNOK	( pcWriteIf->writeCode( m_avg_bitrate[i],										16,		"ScalableSEI: avg_bitrate"														) );
//JVT-S036 lsj start
			RNOK	( pcWriteIf->writeCode( m_max_bitrate_layer[i],										16,		"ScalableSEI: max_bitrate_layer"											) );
			RNOK	( pcWriteIf->writeCode( m_max_bitrate_decoded_picture[i],										16,		"ScalableSEI: max_bitrate_decoded_picture"						) );
			RNOK	( pcWriteIf->writeCode( m_max_bitrate_calc_window[i],							16,		"ScalableSEI: max_bitrate_calc_window"											) );
//JVT-S036 lsj end
		}
		if ( m_frm_rate_info_present_flag[i] )
		{
			RNOK	( pcWriteIf->writeCode( m_constant_frm_rate_idc[i],			2,		"ScalableSEI: constant_frm_bitrate_idc"							) );
			RNOK	( pcWriteIf->writeCode( m_avg_frm_rate[i],									16,		"ScalableSEI: avg_frm_rate"													) );
		}
		else
		{
	//JVT-S036 lsj
			RNOK	(pcWriteIf->writeUvlc( m_frm_rate_info_src_layer_id_delta[i],	"ScalableSEI: frm_rate_info_src_layer_id_delta"			) );
		}

		if ( m_frm_size_info_present_flag[i] )
		{
			RNOK	( pcWriteIf->writeUvlc( m_frm_width_in_mbs_minus1[i],							"ScalableSEI: frm_width_in_mbs_minus1"								) );
			RNOK	( pcWriteIf->writeUvlc( m_frm_height_in_mbs_minus1[i],						"ScalableSEI: frm_height_in_mbs_minus1"							) );
		}
		else
		{//JVT-S036 lsj
			RNOK	(pcWriteIf->writeUvlc( m_frm_size_info_src_layer_id_delta[i],	"ScalableSEI: frm_size_info_src_layer_id_delta"			) );
		}

		if ( m_sub_region_layer_flag[i] )
		{
			RNOK	( pcWriteIf->writeCode ( m_base_region_layer_id[i],					8,		"ScalableSEI: base_region_layer_id"									) );
			RNOK	( pcWriteIf->writeFlag ( m_dynamic_rect_flag[i],									"ScalableSEI: dynamic_rect_flag"											) );
			if ( m_dynamic_rect_flag[i] )
			{
				RNOK	( pcWriteIf->writeCode ( m_horizontal_offset[i],					16,		"ScalableSEI: horizontal_offset"											) );
				RNOK	( pcWriteIf->writeCode ( m_vertical_offset[i],						16,		"ScalableSEI: vertical_offset"												) );
				RNOK	( pcWriteIf->writeCode ( m_region_width[i],								16,		"ScalableSEI: region_width"													) );
				RNOK	( pcWriteIf->writeCode ( m_region_height[i],							16,		"ScalableSEI: region_height"													) );
			}
		}
		else
		{//JVT-S036 lsj
			RNOK	( pcWriteIf->writeUvlc( m_sub_region_info_src_layer_id_delta[i],		"ScalableSEI: sub_region_info_src_layer_id_delta"				) );
		}

		if( m_sub_pic_layer_flag[i] )
		{//JVT-S036 lsj
			RNOK	( pcWriteIf->writeCode( m_roi_id[i],		3,								"Scalable: roi_id"					) );
		}

	//JVT-S036 lsj start
		if ( m_iroi_slice_division_info_present_flag[i] )
		{
			RNOK	( pcWriteIf->writeCode( m_iroi_slice_division_type[i],		2,		"ScalableSEI:iroi_slice_division_type" ) );
			if( m_iroi_slice_division_type[i] == 0 )
			{
				RNOK	( pcWriteIf->writeUvlc( m_grid_slice_width_in_mbs_minus1[i],    "ScalableSEI:grid_slice_width_in_mbs_minus1" ) );
				RNOK	( pcWriteIf->writeUvlc( m_grid_slice_height_in_mbs_minus1[i],    "ScalableSEI:grid_slice_height_in_mbs_minus1" ) );
			}
			else if( m_iroi_slice_division_type[i] == 1 )
			{
				RNOK	( pcWriteIf->writeUvlc( m_num_slice_minus1[i],		"ScalableSEI:num_slice_minus1" ) );
				for ( j = 0; j <= m_num_slice_minus1[i]; j++ )
				{
					RNOK	( pcWriteIf->writeUvlc( m_first_mb_in_slice[i][j],				"ScalableSEI: first_mb_in_slice" ) );
					RNOK	( pcWriteIf->writeUvlc( m_slice_width_in_mbs_minus1[i][j],		"ScalableSEI:slice_width_in_mbs_minus1" ) );
					RNOK	( pcWriteIf->writeUvlc( m_slice_height_in_mbs_minus1[i][j],		"ScalableSEI:slice_height_in_mbs_minus1" ) );
				}
			}
			else if ( m_iroi_slice_division_type[1] == 2 )
			{
				RNOK	( pcWriteIf->writeUvlc( m_num_slice_minus1[i],		"ScalableSEI:num_slice_minus1" ) );
				UInt uiFrameHeightInMb = m_slice_height_in_mbs_minus1[i][j] + 1;
				UInt uiFrameWidthInMb  = m_slice_width_in_mbs_minus1[i][j] + 1;
				UInt uiPicSizeInMbs = uiFrameHeightInMb * uiFrameWidthInMb;
				for ( j = 0; j < uiPicSizeInMbs; j++ )
				{
					RNOK	( pcWriteIf->writeUvlc( m_slice_id[i][j],		"ScalableSEI:slice_id"		   ) );
				}
			}
		}
	//JVT-S036 lsj end
		if ( m_layer_dependency_info_present_flag[i] )
		{
			RNOK	( pcWriteIf->writeUvlc ( m_num_directly_dependent_layers[i],			"ScalableSEI: num_directly_dependent_layers"					) );
// BUG_FIX liuhui{
		    for( j = 0; j < m_num_directly_dependent_layers[i]; j++ )
		    {
		      RNOK( pcWriteIf->writeUvlc (m_directly_dependent_layer_id_delta_minus1[i][j],      "ScalableSEI: directly_dependent_layers_id_delta"		) );  //JVT-S036 lsj
		    }
// BUG_FIX liuhui}
		}
		else
		{//JVT-S036 lsj
			RNOK( pcWriteIf->writeUvlc(m_layer_dependency_info_src_layer_id_delta[i],	 "ScalableSEI: layer_dependency_info_src_layer_id_delta"     ) );
		}

		if ( m_init_parameter_sets_info_present_flag[i] ) 
		{

// BUG_FIX liuhui{
			RNOK	( pcWriteIf->writeUvlc ( m_num_init_seq_parameter_set_minus1[i],	"ScalableSEI: num_init_seq_parameter_set_minus1"			) );
			for( j = 0; j <= m_num_init_seq_parameter_set_minus1[i]; j++ )
			{
			  RNOK( pcWriteIf->writeUvlc( m_init_seq_parameter_set_id_delta[i][j],  "ScalableSEI: init_seq_parameter_set_id_delta"				    ) );
			}

			RNOK    ( pcWriteIf->writeUvlc ( m_num_init_pic_parameter_set_minus1[i],	"ScalableSEI: num_init_pic_parameter_set_minus1"			) );
			for( j = 0; j <= m_num_init_pic_parameter_set_minus1[i]; j++ )
			{
			  RNOK ( pcWriteIf->writeUvlc ( m_init_pic_parameter_set_id_delta[i][j],"ScalableSEI: init_pic_parameter_set_id_delta"				    ) );
			}
// BUG_FIX liuhui}
		}
		else
		{//JVT-S036 lsj 
			RNOK	(pcWriteIf->writeUvlc( m_init_parameter_sets_info_src_layer_id_delta[i], "ScalableSEI: init_parameter_sets_info_src_layer_id_delta"	) );
		}
	}

	return Err::m_nOK;
}
ErrVal
SEI::ScalableSei::read ( HeaderSymbolReadIf *pcReadIf )
{
   UInt i, j=0;
  UInt rl;//JVT-S036 lsj 

	RNOK	( pcReadIf->getUvlc( m_num_layers_minus1 ,																""	) );

	for ( i = 0; i <= m_num_layers_minus1; i++ )
	{
		RNOK	( pcReadIf->getCode( m_layer_id[i],																	8,			""	) );
	//JVT-S036 lsj start
//		RNOK	( pcReadIf->getFlag( m_fgs_layer_flag[i],																""	) ); 
		RNOK	( pcReadIf->getCode( m_simple_priority_id[i],													6,		"" ) );	
		RNOK	( pcReadIf->getFlag( m_discardable_flag[i],													"" ) );     
		RNOK	( pcReadIf->getCode( m_temporal_level[i],													3,		""	) );
		RNOK	( pcReadIf->getCode( m_dependency_id[i],												3,		""	) );
		RNOK	( pcReadIf->getCode( m_quality_level[i],													2,		""	) );
	
		RNOK	( pcReadIf->getFlag( m_sub_pic_layer_flag[i],														""	) );
		RNOK	( pcReadIf->getFlag( m_sub_region_layer_flag[i],													""	) );
		RNOK	( pcReadIf->getFlag( m_iroi_slice_division_info_present_flag[i],						""  ) );  
		RNOK	( pcReadIf->getFlag( m_profile_level_info_present_flag[i],								""	) );
   //JVT-S036 lsj end	
		RNOK	( pcReadIf->getFlag( m_bitrate_info_present_flag[i],											""	) );
		RNOK	( pcReadIf->getFlag( m_frm_rate_info_present_flag[i],											""	) );
		RNOK	( pcReadIf->getFlag( m_frm_size_info_present_flag[i],											""	) );
		RNOK	( pcReadIf->getFlag( m_layer_dependency_info_present_flag[i],								""	) );
		RNOK	( pcReadIf->getFlag( m_init_parameter_sets_info_present_flag[i],						""	) );
		RNOK	( pcReadIf->getFlag( m_exact_interlayer_pred_flag[i],											""  ) );	//JVT-S036 lsj

		if( m_profile_level_info_present_flag[i] )
		{
			RNOK	( pcReadIf->getCode( m_layer_profile_idc[i],											8,		""	) );
			RNOK	( pcReadIf->getFlag( m_layer_constraint_set0_flag[i],										""	) );
			RNOK	( pcReadIf->getFlag( m_layer_constraint_set1_flag[i],										""	) );
			RNOK	( pcReadIf->getFlag( m_layer_constraint_set2_flag[i],										""	) );
			RNOK	( pcReadIf->getFlag( m_layer_constraint_set3_flag[i],										""	) );
			UInt uiReserved;
			RNOK	( pcReadIf->getCode( uiReserved,																	4,		""	) );
			RNOK	( pcReadIf->getCode( m_layer_level_idc[i],												8,		""	) );
		}
		else
		{//JVT-S036 lsj
			RNOK	( pcReadIf->getUvlc( m_profile_level_info_src_layer_id_delta[i],						""  ) );
			rl = m_layer_id[i] - m_profile_level_info_src_layer_id_delta[i];
			m_layer_profile_idc[i] = m_layer_profile_idc[rl];
			m_layer_constraint_set0_flag[i] = m_layer_constraint_set0_flag[rl];
			m_layer_constraint_set1_flag[i] = m_layer_constraint_set1_flag[rl];
			m_layer_constraint_set2_flag[i] = m_layer_constraint_set2_flag[rl];
			m_layer_constraint_set3_flag[i] = m_layer_constraint_set3_flag[rl];
			m_layer_level_idc[i] = m_layer_level_idc[rl];
		}

	/*	if( m_decoding_dependency_info_present_flag[i] )
		{
			RNOK	( pcReadIf->getCode( m_simple_priority_id[i],													6,		"" ) );	
			RNOK	( pcReadIf->getFlag( m_discardable_flag[i],													"" ) );     
			RNOK	( pcReadIf->getCode( m_temporal_level[i],													3,		""	) );
			RNOK	( pcReadIf->getCode( m_dependency_id[i],												3,		""	) );
			RNOK	( pcReadIf->getCode( m_quality_level[i],													2,		""	) );
		}
JVT-S036 lsj */
		if( m_bitrate_info_present_flag[i] )
		{
			RNOK	( pcReadIf->getCode( m_avg_bitrate[i],														16,		""	) );
	//JVT-S036 lsj start
			RNOK	( pcReadIf->getCode( m_max_bitrate_layer[i],														16,		""	) );
			RNOK	( pcReadIf->getCode( m_max_bitrate_decoded_picture[i],														16,		""	) );
			RNOK	( pcReadIf->getCode( m_max_bitrate_calc_window[i],											16,		""	) );
	//JVT-S036 lsj end
		}

		if( m_frm_rate_info_present_flag[i] )
		{
			RNOK	( pcReadIf->getCode( m_constant_frm_rate_idc[i],									2,		""	) );
			RNOK	( pcReadIf->getCode( m_avg_frm_rate[i],														16,		""	) );
		}
		else
		{//JVT-S036 lsj
			RNOK	( pcReadIf->getUvlc( m_frm_rate_info_src_layer_id_delta[i],							"" ) );
			rl = m_layer_id[i] - m_frm_rate_info_src_layer_id_delta[i];
			m_constant_frm_rate_idc[i] = m_constant_frm_rate_idc[rl];
			m_avg_frm_rate[i] = m_avg_frm_rate[rl];
		}

		if( m_frm_size_info_present_flag[i] )
		{
			RNOK	( pcReadIf->getUvlc( m_frm_width_in_mbs_minus1[i],											""	) );
			RNOK	( pcReadIf->getUvlc( m_frm_height_in_mbs_minus1[i],											""	) );
		}
		else
		{//JVT-S036 lsj 
			RNOK	( pcReadIf->getUvlc( m_frm_size_info_src_layer_id_delta[i],							""  ) );
			rl = m_layer_id[i] - m_frm_size_info_src_layer_id_delta[i];
			m_frm_width_in_mbs_minus1[i] = m_frm_width_in_mbs_minus1[rl];
			m_frm_width_in_mbs_minus1[i] = m_frm_width_in_mbs_minus1[rl];
		}

		if( m_sub_region_layer_flag[i] )
		{
			RNOK	( pcReadIf->getCode( m_base_region_layer_id[i],										8,		""	) );
			RNOK	( pcReadIf->getFlag( m_dynamic_rect_flag[i],														""	) );
			if( m_dynamic_rect_flag[i] )
			{
				RNOK( pcReadIf->getCode( m_horizontal_offset[i],											16,		""	) );
				RNOK( pcReadIf->getCode( m_vertical_offset[i],												16,		""	) );
				RNOK( pcReadIf->getCode( m_region_width[i],														16,		""	) );
				RNOK( pcReadIf->getCode( m_region_height[i],													16,		""	) );
			}
		}
		else
		{//JVT-S036 lsj 
			RNOK	( pcReadIf->getUvlc( m_sub_region_info_src_layer_id_delta[i],						""  ) );
			rl = m_layer_id[i] - m_sub_region_info_src_layer_id_delta[i];
			m_base_region_layer_id[i] = m_base_region_layer_id[rl];
			m_dynamic_rect_flag[i] = m_dynamic_rect_flag[rl];
			if( m_dynamic_rect_flag[i] )
			{
				 m_horizontal_offset[i] = m_horizontal_offset[rl];
				 m_vertical_offset[i] = m_vertical_offset[rl];
				 m_region_width[i] = m_region_width[rl];
				 m_region_height[i] = m_region_height[rl];
			}

		}

	//JVT-S036 lsj start
		if( m_sub_pic_layer_flag[i] )
		{
			RNOK	( pcReadIf->getCode( m_roi_id[i],		3,								""					) );
		}
		if ( m_iroi_slice_division_info_present_flag[i] )
		{
			RNOK	( pcReadIf->getCode( m_iroi_slice_division_type[i],		2,		"ScalableSEI:iroi_slice_division_type" ) );
			if( m_iroi_slice_division_type[i] == 0 )
			{
				RNOK	( pcReadIf->getUvlc( m_grid_slice_width_in_mbs_minus1[i],    "ScalableSEI:grid_slice_width_in_mbs_minus1" ) );
				RNOK	( pcReadIf->getUvlc( m_grid_slice_height_in_mbs_minus1[i],    "ScalableSEI:grid_slice_height_in_mbs_minus1" ) );
			}
			else if( m_iroi_slice_division_type[i] == 1 )
			{
				RNOK	( pcReadIf->getUvlc( m_num_slice_minus1[i],		"ScalableSEI:num_slice_minus1" ) );
				for ( j = 0; j <= m_num_slice_minus1[i]; j++ )
				{
					RNOK	( pcReadIf->getUvlc( m_first_mb_in_slice[i][j],				"ScalableSEI: first_mb_in_slice" ) );
					RNOK	( pcReadIf->getUvlc( m_slice_width_in_mbs_minus1[i][j],		"ScalableSEI:slice_width_in_mbs_minus1" ) );
					RNOK	( pcReadIf->getUvlc( m_slice_height_in_mbs_minus1[i][j],		"ScalableSEI:slice_height_in_mbs_minus1" ) );
				}
			}
			else if ( m_iroi_slice_division_type[i] == 2 )
			{
				RNOK	( pcReadIf->getUvlc( m_num_slice_minus1[i],		"ScalableSEI:num_slice_minus1" ) );
				UInt uiFrameHeightInMb = m_slice_height_in_mbs_minus1[i][j] + 1;
				UInt uiFrameWidthInMb  = m_slice_width_in_mbs_minus1[i][j] + 1;
				UInt uiPicSizeInMbs = uiFrameHeightInMb * uiFrameWidthInMb;
				for ( j = 0; j < uiPicSizeInMbs; j++ )
				{
					RNOK	( pcReadIf->getUvlc( m_slice_id[i][j],		"ScalableSEI:slice_id"		   ) );
				}
			}
		}
   //JVT-S036 lsj end

		if( m_layer_dependency_info_present_flag[i] )
		{
			RNOK	( pcReadIf->getUvlc( m_num_directly_dependent_layers[i],								""	) );
// BUG_FIX liuhui{
			for( j = 0; j < m_num_directly_dependent_layers[i]; j++ )
			{
				RNOK  ( pcReadIf->getUvlc( m_directly_dependent_layer_id_delta_minus1[i][j],				""  ) );//JVT-S036 lsj
			}
// BUG_FIX liuhui}
		}
		else
		{//JVT-S036 lsj
			RNOK	( pcReadIf->getUvlc( m_layer_dependency_info_src_layer_id_delta[i],				""  ) );
			rl = m_layer_id[i] - m_layer_dependency_info_src_layer_id_delta[i];
			m_num_directly_dependent_layers[i] = m_num_directly_dependent_layers[rl];
			for( j = 0; j < m_num_directly_dependent_layers[i]; j++ )
			{
				m_directly_dependent_layer_id_delta_minus1[i][j] = m_directly_dependent_layer_id_delta_minus1[rl][j];
			}
		}

		if( m_init_parameter_sets_info_present_flag[i] )
		{
// BUG_FIX liuhui{
			RNOK    ( pcReadIf->getUvlc( m_num_init_seq_parameter_set_minus1[i],                        ""  ) );
			for( j = 0; j <= m_num_init_seq_parameter_set_minus1[i]; j++ )
			{
			    RNOK	( pcReadIf->getUvlc( m_init_seq_parameter_set_id_delta[i][j],					""	) );
			}
			RNOK	( pcReadIf->getUvlc( m_num_init_pic_parameter_set_minus1[i],						""	) );
			for( j = 0; j <= m_num_init_pic_parameter_set_minus1[i]; j++ )
			{
				RNOK	( pcReadIf->getUvlc( m_init_pic_parameter_set_id_delta[i][j],					""	) );
			}
// BUG_FIX liuhui}
		}
		else
		{//JVT-S036 lsj
			RNOK	( pcReadIf->getUvlc( m_init_parameter_sets_info_src_layer_id_delta[i],		"" ) );
			rl = m_layer_id[i] - m_init_parameter_sets_info_src_layer_id_delta[i];
			m_num_init_seq_parameter_set_minus1[i] = m_num_init_seq_parameter_set_minus1[rl];
			for( j = 0; j <= m_num_init_seq_parameter_set_minus1[i]; j++ )
			{
			    m_init_seq_parameter_set_id_delta[i][j] = m_init_seq_parameter_set_id_delta[rl][j];
			}
			m_num_init_pic_parameter_set_minus1[i] = m_num_init_pic_parameter_set_minus1[rl];
			for( j = 0; j <= m_num_init_pic_parameter_set_minus1[i]; j++ )
			{
				m_init_pic_parameter_set_id_delta[i][j] = m_init_pic_parameter_set_id_delta[rl][j];
			}
		}
	}	

	return Err::m_nOK;
}

//////////////////////////////////////////////////////////////////////////
//
//			SUB-PICTURE SCALABLE LAYER SEI
//
//////////////////////////////////////////////////////////////////////////

SEI::SubPicSei::SubPicSei ()
: SEIMessage		( SUB_PIC_SEI ),
m_uiLayerId			( 0 )
{
}

SEI::SubPicSei::~SubPicSei ()
{
}

ErrVal
SEI::SubPicSei::create( SubPicSei*& rpcSeiMessage)
{
	rpcSeiMessage = new SubPicSei();
	ROT( NULL == rpcSeiMessage );
	return Err::m_nOK;
}

ErrVal
SEI::SubPicSei::write( HeaderSymbolWriteIf *pcWriteIf )
{
	RNOK	( pcWriteIf->writeUvlc( m_uiLayerId, "Sub-picture scalable SEI: m_uiLayerId" ) );
	return Err::m_nOK;
}

ErrVal
SEI::SubPicSei::read( HeaderSymbolReadIf *pcReadIf )
{
	RNOK	( pcReadIf->getUvlc( m_uiLayerId, "Sub-picture scalable SEI: m_uiLayerd" ) );
	return Err::m_nOK;
}

//{{Quality level estimation and modified truncation- JVTO044 and m12007
//France Telecom R&D-(nathalie.cammas@francetelecom.com)
//////////////////////////////////////////////////////////////////////////
// 
//      QUALITY LEVEL     S E I
//
//////////////////////////////////////////////////////////////////////////

SEI::QualityLevelSEI::QualityLevelSEI     ()
 : SEIMessage                     ( QUALITYLEVEL_SEI ),
 m_uiNumLevels         ( 0 ),
 m_uiDependencyId      ( 0 )
{
  ::memset( m_auiQualityLevel,  0x00, MAX_NUM_RD_LEVELS*sizeof(UInt) );
  ::memset( m_auiDeltaBytesRateOfLevel, 0x00, MAX_NUM_RD_LEVELS*sizeof(UInt) );
}


SEI::QualityLevelSEI::~QualityLevelSEI()
{
}


ErrVal
SEI::QualityLevelSEI::create( QualityLevelSEI*& rpcSeiMessage )
{
  rpcSeiMessage = new QualityLevelSEI();
  ROT( NULL == rpcSeiMessage )
  return Err::m_nOK;
}


ErrVal
SEI::QualityLevelSEI::write( HeaderSymbolWriteIf* pcWriteIf )
{
  RNOK  ( pcWriteIf->writeCode( m_uiDependencyId, 3,"QualityLevelSEI: DependencyId"   ) );
  RNOK  ( pcWriteIf->writeCode( m_uiNumLevels, 4,"QualityLevelSEI: NumLevels"   ) );
  for(UInt ui = 0; ui < m_uiNumLevels; ui++)
  {
	RNOK  ( pcWriteIf->writeCode( m_auiQualityLevel[ui], 8,"QualityLevelSEI: QualityLevel"   ) );
	RNOK  ( pcWriteIf->writeUvlc( m_auiDeltaBytesRateOfLevel[ui],"QualityLevelSEI: DeDeltaBytesRateOfLevellta"   ) );
  }

  return Err::m_nOK;
}


ErrVal
SEI::QualityLevelSEI::read ( HeaderSymbolReadIf* pcReadIf )
{
  RNOK  ( pcReadIf->getCode( m_uiDependencyId, 3,"QualityLevelSEI: DependencyId"   ) );
  RNOK  ( pcReadIf->getCode( m_uiNumLevels, 4,"QualityLevelSEI: NumLevels"   ) );
  for(UInt ui = 0; ui < m_uiNumLevels; ui++)
  {
	RNOK  ( pcReadIf->getCode( m_auiQualityLevel[ui], 8,"QualityLevelSEI: QualityLevel"   ) );
	RNOK  ( pcReadIf->getUvlc( m_auiDeltaBytesRateOfLevel[ui],"QualityLevelSEI: DeltaBytesRateOfLevel"   ) );
  }
  return Err::m_nOK;
}

//}}Quality level estimation and modified truncation- JVTO044 and m12007

//NonRequired JVT-Q066 (06-04-08){{
SEI::NonRequiredSei::NonRequiredSei	()
: SEIMessage						( NON_REQUIRED_SEI )
, m_uiNumInfoEntriesMinus1			(MSYS_UINT_MAX)
{
	::memset( m_uiEntryDependencyId,			MSYS_UINT_MAX, MAX_NUM_INFO_ENTRIES*sizeof(UInt) );
	::memset( m_uiNumNonRequiredPicsMinus1,		MSYS_UINT_MAX, MAX_NUM_INFO_ENTRIES*sizeof(UInt) );
	::memset( m_uiNonRequiredPicDependencyId,	MSYS_UINT_MAX, MAX_NUM_INFO_ENTRIES*MAX_NUM_NON_REQUIRED_PICS*sizeof(UInt) );
	::memset( m_uiNonRequiredPicQulityLevel,	MSYS_UINT_MAX, MAX_NUM_INFO_ENTRIES*MAX_NUM_NON_REQUIRED_PICS*sizeof(UInt) );
	::memset( m_uiNonRequiredPicFragmentOrder,  MSYS_UINT_MAX, MAX_NUM_INFO_ENTRIES*MAX_NUM_NON_REQUIRED_PICS*sizeof(UInt) );
}
/*
SEI::NonRequiredSei::NonRequiredSei	()
: SEIMessage						( NON_REQUIRED_SEI )
, m_uiNumInfoEntriesMinus1			(0)
{
::memset( m_uiEntryDependencyId,			0x00, MAX_NUM_INFO_ENTRIES*sizeof(UInt) );
::memset( m_uiNumNonRequiredPicsMinus1,		0x00, MAX_NUM_INFO_ENTRIES*sizeof(UInt) );
::memset( m_uiNonRequiredPicDependencyId,	0x00, MAX_NUM_INFO_ENTRIES*MAX_NUM_NON_REQUIRED_PICS*sizeof(UInt) );
::memset( m_uiNonRequiredPicQulityLevel,	0x00, MAX_NUM_INFO_ENTRIES*MAX_NUM_NON_REQUIRED_PICS*sizeof(UInt) );
::memset( m_uiNonRequiredPicFragmentOrder,  0x00, MAX_NUM_INFO_ENTRIES*MAX_NUM_NON_REQUIRED_PICS*sizeof(UInt) );
}*/
//NonRequired JVT-Q066 (06-04-08)}}

SEI::NonRequiredSei::~NonRequiredSei ()
{
}

ErrVal
SEI::NonRequiredSei::create ( NonRequiredSei*& rpcSeiMessage )
{
	rpcSeiMessage = new NonRequiredSei();
	ROT( NULL == rpcSeiMessage)
		return Err::m_nOK;
}

ErrVal
SEI::NonRequiredSei::destroy() 
{
	delete this;
	return Err::m_nOK;
}

ErrVal
SEI::NonRequiredSei::write( HeaderSymbolWriteIf* pcWriteIf )
{
	RNOK	(pcWriteIf->writeUvlc( m_uiNumInfoEntriesMinus1,	"NonRequiredSEI: NumInfoEntriesMinus1"	));
	for( UInt uiLayer = 0; uiLayer <= m_uiNumInfoEntriesMinus1; uiLayer++)
	{
		RNOK(pcWriteIf->writeCode( m_uiEntryDependencyId[uiLayer],	3,	"NonRequiredSEI: EntryDependencyId"		));
		RNOK(pcWriteIf->writeUvlc( m_uiNumNonRequiredPicsMinus1[uiLayer],	"NonRequiredSEI: NumNonRequiredPicsMinus1"	));
		for( UInt NonRequiredLayer = 0; NonRequiredLayer <= m_uiNumNonRequiredPicsMinus1[uiLayer]; NonRequiredLayer++)
		{
			RNOK(pcWriteIf->writeCode( m_uiNonRequiredPicDependencyId[uiLayer][NonRequiredLayer],	3,	"NonRequiredSEI: NonRequiredPicDependencyId"));
			RNOK(pcWriteIf->writeCode( m_uiNonRequiredPicQulityLevel[uiLayer][NonRequiredLayer],	2,	"NonRequiredSEI: NonRequiredPicQulityLevel"	));
			RNOK(pcWriteIf->writeCode( m_uiNonRequiredPicFragmentOrder[uiLayer][NonRequiredLayer],	2,	"NonRequiredSEI: NonRequiredFragmentOrder"	));
		}
	}
	return Err::m_nOK;
}

ErrVal
SEI::NonRequiredSei::read( HeaderSymbolReadIf* pcReadIf )
{
	RNOK	(pcReadIf->getUvlc( m_uiNumInfoEntriesMinus1,	"NonRequiredSEI: NumInfoEntriesMinus1"	));
	for( UInt uiLayer = 0; uiLayer <= m_uiNumInfoEntriesMinus1; uiLayer++)
	{
		RNOK(pcReadIf->getCode( m_uiEntryDependencyId[uiLayer],	3,	"NonRequiredSEI: EntryDependencyId"		));
		RNOK(pcReadIf->getUvlc( m_uiNumNonRequiredPicsMinus1[uiLayer],	"NonRequiredSEI: NumNonRequiredPicsMinus1"	));
		for( UInt NonRequiredLayer = 0; NonRequiredLayer <= m_uiNumNonRequiredPicsMinus1[uiLayer]; NonRequiredLayer++)
		{
			RNOK(pcReadIf->getCode( m_uiNonRequiredPicDependencyId[uiLayer][NonRequiredLayer],	3,	"NonRequiredSEI: NonRequiredPicDependencyId"));
			RNOK(pcReadIf->getCode( m_uiNonRequiredPicQulityLevel[uiLayer][NonRequiredLayer],	2,	"NonRequiredSEI: NonRequiredPicQulityLevel"	));
			RNOK(pcReadIf->getCode( m_uiNonRequiredPicFragmentOrder[uiLayer][NonRequiredLayer],	2,	"NonRequiredSEI: NonRequiredFragmentOrder"	));
		}
	}
	return Err::m_nOK;
}
// JVT-S080 LMI {
//////////////////////////////////////////////////////////////////////////
//
//			SCALABLE SEI LAYERS NOT PRESENT
//
//////////////////////////////////////////////////////////////////////////

UInt SEI::ScalableSeiLayersNotPresent::m_uiLeftNumLayers = 0;
UInt SEI::ScalableSeiLayersNotPresent::m_auiLeftLayerId[MAX_SCALABLE_LAYERS];

SEI::ScalableSeiLayersNotPresent::ScalableSeiLayersNotPresent (): SEIMessage		( SCALABLE_SEI_LAYERS_NOT_PRESENT )
{
}

SEI::ScalableSeiLayersNotPresent::~ScalableSeiLayersNotPresent ()
{
}

ErrVal
SEI::ScalableSeiLayersNotPresent::create( ScalableSeiLayersNotPresent*& rpcSeiMessage )
{
	rpcSeiMessage = new ScalableSeiLayersNotPresent();
	ROT( NULL == rpcSeiMessage )
		return Err::m_nOK;
}

ErrVal
SEI::ScalableSeiLayersNotPresent::write( HeaderSymbolWriteIf *pcWriteIf )
{
  UInt i;

	RNOK ( pcWriteIf->writeUvlc(m_uiNumLayers,													"ScalableSEILayersNotPresent: num_layers"											) );
	for( i = 0; i < m_uiNumLayers; i++ )
	{
		RNOK ( pcWriteIf->writeCode( m_auiLayerId[i],												8,		"ScalableSEILayersNotPresent: layer_id"															) );
	}
	return Err::m_nOK;
}

ErrVal
SEI::ScalableSeiLayersNotPresent::read ( HeaderSymbolReadIf *pcReadIf )
{
	UInt i;
	RNOK ( pcReadIf->getUvlc( m_uiNumLayers ,																"ScalableSEILayersNotPresent: num_layers"	) );
	for ( i = 0; i < m_uiNumLayers; i++ )
	{
		RNOK ( pcReadIf->getCode( m_auiLayerId[i],																	8,			"ScalableSEILayersNotPresent: layer_id"	) );
	}
	return Err::m_nOK;
}

//////////////////////////////////////////////////////////////////////////
//
//			SCALABLE SEI DEPENDENCY CHANGE
//
//////////////////////////////////////////////////////////////////////////

SEI::ScalableSeiDependencyChange::ScalableSeiDependencyChange (): SEIMessage		( SCALABLE_SEI_DEPENDENCY_CHANGE )
{
}

SEI::ScalableSeiDependencyChange::~ScalableSeiDependencyChange ()
{
}

ErrVal
SEI::ScalableSeiDependencyChange::create( ScalableSeiDependencyChange*& rpcSeiMessage )
{
	rpcSeiMessage = new ScalableSeiDependencyChange();
	ROT( NULL == rpcSeiMessage )
		return Err::m_nOK;
}

ErrVal
SEI::ScalableSeiDependencyChange::write( HeaderSymbolWriteIf *pcWriteIf )
{
  UInt i, j;

	ROF( m_uiNumLayersMinus1+1 );
	RNOK		( pcWriteIf->writeUvlc(m_uiNumLayersMinus1,													"ScalableSeiDependencyChange: num_layers_minus1"											) );
	for( i = 0; i <= m_uiNumLayersMinus1; i++ )
	{
		RNOK	( pcWriteIf->writeCode( m_auiLayerId[i],												8,		"ScalableSeiDependencyChange: layer_id"															) );
		RNOK	( pcWriteIf->writeFlag( m_abLayerDependencyInfoPresentFlag[i],		"ScalableSeiDependencyChange: layer_dependency_info_present_flag"															) );		
		if( m_abLayerDependencyInfoPresentFlag[i] )
		{
	       RNOK		( pcWriteIf->writeUvlc(m_auiNumDirectDependentLayers[i],													"ScalableSeiDependencyChange: num_directly_dependent_layers"											) );
	       for ( j = 0; j < m_auiNumDirectDependentLayers[i]; j++)
	            RNOK( pcWriteIf->writeUvlc(m_auiDirectDependentLayerIdDeltaMinus1[i][j],													"ScalableSeiDependencyChange: directly_dependent_layer_id_delta_minus1"											) );
		}
		else
	            RNOK	( pcWriteIf->writeUvlc(m_auiLayerDependencyInfoSrcLayerIdDeltaMinus1[i],													"ScalableSeiDependencyChange: layer_dependency_info_src_layer_id_delta_minus1"											) );
	}
	return Err::m_nOK;
}

ErrVal
SEI::ScalableSeiDependencyChange::read ( HeaderSymbolReadIf *pcReadIf )
{
  UInt i, j;

	RNOK		( pcReadIf->getUvlc(m_uiNumLayersMinus1,													"ScalableSeiDependencyChange: num_layers_minus1"											) );
	for( i = 0; i <= m_uiNumLayersMinus1; i++ )
	{
		RNOK	( pcReadIf->getCode( m_auiLayerId[i],												8,		"ScalableSeiDependencyChange: layer_id"															) );
		RNOK	( pcReadIf->getFlag( m_abLayerDependencyInfoPresentFlag[i],		"ScalableSeiDependencyChange: layer_dependency_info_present_flag"															) );		
		if( m_abLayerDependencyInfoPresentFlag[i] )
		{
	       RNOK		( pcReadIf->getUvlc(m_auiNumDirectDependentLayers[i],													"ScalableSeiDependencyChange: num_directly_dependent_layers"											) );
	       for ( j = 0; j < m_auiNumDirectDependentLayers[i]; j++)
	            RNOK		( pcReadIf->getUvlc(m_auiDirectDependentLayerIdDeltaMinus1[i][j],													"ScalableSeiDependencyChange: directly_dependent_layer_id_delta_minus1"											) );
		}
		else
	            RNOK		( pcReadIf->getUvlc(m_auiLayerDependencyInfoSrcLayerIdDeltaMinus1[i],													"ScalableSeiDependencyChange: layer_dependency_info_src_layer_id_delta_minus1"											) );
	}
	return Err::m_nOK;
}
// JVT-S080 LMI }
H264AVC_NAMESPACE_END
