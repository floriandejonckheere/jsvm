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

// JVT-V068 HRD {
#include "H264AVCCommonLib/Vui.h"
// JVT-V068 HRD }

H264AVC_NAMESPACE_BEGIN




ErrVal
SEI::read( HeaderSymbolReadIf* pcReadIf,
           MessageList&        rcSEIMessageList
           // JVT-V068 {
           ,ParameterSetMng*    pcParameterSetMng
           // JVT-V068 }
         ) 
{
  ROT( NULL == pcReadIf);

  while( pcReadIf->moreRBSPData() )
  {
    SEIMessage* pcActualSEIMessage = NULL;

    RNOK( xRead( pcReadIf, pcActualSEIMessage
    // JVT-V068 {
				, pcParameterSetMng
    // JVT-V068 }
		 ));

    // JVT-V068 {
    if (pcActualSEIMessage)
    // JVT-V068 }
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
// JVT-T073 {
ErrVal
SEI::writeNesting( HeaderSymbolWriteIf*  pcWriteIf,
                   HeaderSymbolWriteIf*  pcWriteTestIf,
                   MessageList*          rpcSEIMessageList )
{
    ROT( NULL == pcWriteIf);
	ROT( NULL == pcWriteTestIf);
	ROT( NULL == rpcSEIMessageList);
    
    SEIMessage* pcTop = rpcSEIMessageList->front();
    rpcSEIMessageList->pop_front();
	SEIMessage* pcBottom = rpcSEIMessageList->front();
	rpcSEIMessageList->pop_front();

	//===== NAL unit header =====
	ETRACE_DECLARE( Bool m_bTraceEnable = true );
	g_nLayer = 0;
	ETRACE_LAYER(0);
	ETRACE_HEADER( "SEI MESSAGE" );
	RNOK  ( pcWriteIf->writeFlag( 0,                "NALU HEADER: forbidden_zero_bit"  ) );
	RNOK  ( pcWriteIf->writeCode( 0, 2,             "NALU HEADER: nal_ref_idc" ) );
	RNOK  ( pcWriteIf->writeCode( NAL_UNIT_SEI, 5,  "NALU HEADER: nal_unit_type" ) );

	//first write testing SEI message to get payload size
	UInt uiBits = 0;
	UInt uiSecondSEILength = 0;
	//take scene info as example, 	//can be changed here
  //JVT-W062
  UInt uiStart = 0;
	switch( pcBottom->getMessageType() )
	{
		case SCENE_INFO_SEI:
	    {
			SEI::SceneInfoSei* pcSceneInfoSei = (SEI::SceneInfoSei*) pcBottom;
			Bool bScenePresentFlag = 1;
			UInt uiSceneId = 1;
			UInt uiSceneTransitionType = 0;
			pcSceneInfoSei->setSceneInfoPresentFlag( bScenePresentFlag );
			pcSceneInfoSei->setSceneId( uiSceneId );
			pcSceneInfoSei->setSceneTransitionType( uiSceneTransitionType );

			RNOK( pcWriteTestIf->writeCode( SCENE_INFO_SEI, 8, "SEI type" ) );
			RNOK( pcWriteTestIf->writeCode( 1, 8, "SEI payload size " ) ); //currently size equals to 1
			RNOK( xWriteNesting( pcWriteIf, pcWriteTestIf, pcBottom, uiBits ) );
			uiSecondSEILength = (uiBits-16+7)/8;
			break;
	    }
    // JVT-W062 {
    case TL0_DEP_REP_IDX_SEI:
    {
     // SEI::Tl0DepRepIdxSei* pcTl0DepRepIdxSEI = (SEI::Tl0DepRepIdxSei*) pcBottom;
	    RNOK( pcWriteTestIf->writeCode( TL0_DEP_REP_IDX_SEI, 8, "SEI type" ) );
  	  RNOK( pcWriteTestIf->writeCode( 3, 8, "SEI payload size " ) ); //currently size equals to 3
      uiStart = pcWriteTestIf->getNumberOfWrittenBits();
	    RNOK( xWriteNesting( pcWriteIf, pcWriteTestIf, pcBottom, uiBits ) );
      uiBits -= uiStart;
	    uiSecondSEILength = (uiBits+7)/8;
      break;
    }
    // JVT-W062 }

		//more case: added here
		default: break;
	}
    RNOK  ( xWriteNesting( pcWriteIf, pcWriteTestIf, pcTop, uiBits ) );

	//Then write actual SEI message
	UInt uiSize = (uiBits+7)/8;
    RNOK( xWritePayloadHeader( pcWriteIf, SCALABLE_NESTING_SEI, uiSize ) );
    RNOK( pcTop->write( pcWriteIf ) );
    // JVT-W062 {
	//RNOK( xWritePayloadHeader( pcWriteIf, SCENE_INFO_SEI, uiSecondSEILength ) );
    RNOK( xWritePayloadHeader( pcWriteIf, pcBottom->getMessageType(), uiSecondSEILength ) );
    // JVT-W062 }

	RNOK( pcBottom->write( pcWriteIf ) );
    UInt uiAlignedBits = 8 - (uiBits&7);
    if( uiAlignedBits != 0 && uiAlignedBits != 8)
    {
        RNOK( pcWriteIf->writeCode( 1<<(uiAlignedBits-1), uiAlignedBits, "SEI: alignment_bits" ) );
    }
  return Err::m_nOK;
}
// JVT-T073 }

// JVT-V068 {
ErrVal
SEI::writeScalableNestingSei( HeaderSymbolWriteIf*  pcWriteIf,
                              HeaderSymbolWriteIf*  pcWriteTestIf,
                              MessageList*          rpcSEIMessageList )
{
  ROT( NULL == pcWriteIf);
	ROT( NULL == pcWriteTestIf);
	ROT( NULL == rpcSEIMessageList);
  ROT( rpcSEIMessageList->size() != 2 );

  SEIMessage* pcTop = rpcSEIMessageList->front();
  rpcSEIMessageList->pop_front();
  ROT( pcTop->getMessageType() != SCALABLE_NESTING_SEI );

	SEIMessage* pcBottom = rpcSEIMessageList->front();
	rpcSEIMessageList->pop_front();

	//===== NAL unit header =====
	ETRACE_DECLARE( Bool m_bTraceEnable = true );
	g_nLayer = 0;
	ETRACE_LAYER(0);
	ETRACE_HEADER( "SEI MESSAGE" );
	RNOK  ( pcWriteIf->writeFlag( 0,                "NALU HEADER: forbidden_zero_bit"  ) );
	RNOK  ( pcWriteIf->writeCode( 0, 2,             "NALU HEADER: nal_ref_idc" ) );
	RNOK  ( pcWriteIf->writeCode( NAL_UNIT_SEI, 5,  "NALU HEADER: nal_unit_type" ) );

	//first write testing SEI message to get payload size
	UInt uiBits = 0;
	UInt uiNestedSeiSize = 0;
	//take scene info as example, 	//can be changed here

  // First Test the payload size 
  UInt uiStart  = pcWriteTestIf->getNumberOfWrittenBits();
  pcBottom->write(pcWriteTestIf);
  uiBits = pcWriteTestIf->getNumberOfWrittenBits() - uiStart;
  UInt uiSize = (uiBits+7)>>3;

  uiNestedSeiSize = uiSize;
  
  uiStart  = pcWriteTestIf->getNumberOfWrittenBits();
  RNOK( xWritePayloadHeader( pcWriteTestIf, pcBottom->getMessageType(), uiSize ) );
  uiBits = pcWriteTestIf->getNumberOfWrittenBits() - uiStart;
  AOT( (uiBits & 7) > 0 );
  uiSize += (uiBits>>3);

  uiStart  = pcWriteTestIf->getNumberOfWrittenBits();
  RNOK( pcTop->write( pcWriteTestIf ) );
  uiBits = pcWriteTestIf->getNumberOfWrittenBits() - uiStart;
  AOT( (uiBits & 7) > 0 );
  uiSize += (uiBits>>3); // Scalable Nesting Sei Payload Size

  //Then write actual SEI message
  uiStart  = pcWriteIf->getNumberOfWrittenBits();
  RNOK( xWritePayloadHeader( pcWriteIf, SCALABLE_NESTING_SEI, uiSize ) );
  RNOK( pcTop->write( pcWriteIf ) );
	RNOK( xWritePayloadHeader( pcWriteIf, pcBottom->getMessageType(), uiNestedSeiSize ) );
	RNOK( pcBottom->write( pcWriteIf ) );
  uiBits = pcWriteIf->getNumberOfWrittenBits() - uiStart;
  UInt uiAlignedBits = 8 - (uiBits&7);
  if( uiAlignedBits != 0 && uiAlignedBits != 8)
  {
      RNOK( pcWriteIf->writeCode( 1<<(uiAlignedBits-1), uiAlignedBits, "SEI: alignment_bits" ) );
  }
  delete pcTop;
  delete pcBottom;

  return Err::m_nOK;
}
// JVT-V068 }

ErrVal
SEI::xRead( HeaderSymbolReadIf* pcReadIf,
            SEIMessage*&        rpcSEIMessage
            // JVT-V068 {
            ,ParameterSetMng* pcParameterSetMng
            // JVT-V068 }
            ) 
{
  MessageType eMessageType = RESERVED_SEI;
  UInt        uiSize       = 0;

  RNOK( xReadPayloadHeader( pcReadIf, eMessageType, uiSize) );

  // JVT-V068 { 
  // Dump HRD SEIs
  if (eMessageType==BUFFERING_PERIOD||eMessageType==PIC_TIMING||eMessageType==AVC_COMPATIBLE_HRD_SEI)
  {
    RNOK( xCreate( rpcSEIMessage, eMessageType, pcParameterSetMng, uiSize ) )
    
    UInt uiDummy;
    while (uiSize--)
    {
      pcReadIf->getCode(uiDummy, 8, "SEI: Byte ignored.");
    }
    RNOK( pcReadIf->readByteAlign() );

    return Err::m_nOK;
  }
  // JVT-V068 }
  RNOK( xCreate( rpcSEIMessage, eMessageType, pcParameterSetMng, uiSize ) )

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

// JVT-T073 {
ErrVal
SEI::xWriteNesting( HeaderSymbolWriteIf* pcWriteIf,
				   HeaderSymbolWriteIf*  pcWriteTestIf,
				   SEIMessage*           pcSEIMessage,
				   UInt&                 uiBits )
{
	UInt uiStart = uiBits;
	if( uiBits == 0 ) // write the following SEI message
	{
		uiBits += 16; //including SEI_type and SEI_payload_size bits, can be changed here for type>0xff
		RNOK( pcSEIMessage->write( pcWriteTestIf ) );
		uiBits = pcWriteTestIf->getNumberOfWrittenBits() - uiStart;
		if( uiBits/8 >0xff )
			uiBits += 8;
		return Err::m_nOK;
	}
	else
	{
		RNOK( pcSEIMessage->write( pcWriteTestIf ) );
		uiBits = pcWriteTestIf->getNumberOfWrittenBits();
		return Err::m_nOK;
	}
}
// JVT-T073 }


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
              // JVT-V068 {
              ParameterSetMng*& rpcParameterSetMng,
              // JVT-V068 }
              UInt          uiSize ) 
{
  switch( eMessageType )
  {
    case SUB_SEQ_INFO:  return SubSeqInfo ::create( (SubSeqInfo*&)  rpcSEIMessage );
    case SCALABLE_SEI:  return ScalableSei::create( (ScalableSei*&) rpcSEIMessage );
    case SUB_PIC_SEI:   return SubPicSei::create	( (SubPicSei*&)		rpcSEIMessage );
	case MOTION_SEI:	return MotionSEI::create( (MotionSEI*&) rpcSEIMessage );
    //{{Quality level estimation and modified truncation- JVTO044 and m12007
    //France Telecom R&D-(nathalie.cammas@francetelecom.com)
    //case QUALITYLEVEL_SEI: return QualityLevelSEI::create((QualityLevelSEI*&) rpcSEIMessage);//SEI changes update
		case PRIORITYLEVEL_SEI: return PriorityLevelSEI::create((PriorityLevelSEI*&) rpcSEIMessage);//SEI changes update
    //}}Quality level estimation and modified truncation- JVTO044 and m12007
  	case NON_REQUIRED_SEI: return NonRequiredSei::create((NonRequiredSei*&) rpcSEIMessage); 
	// JVT-S080 LMI {
    case SCALABLE_SEI_LAYERS_NOT_PRESENT:  return ScalableSeiLayersNotPresent::create( (ScalableSeiLayersNotPresent*&) rpcSEIMessage );
	case SCALABLE_SEI_DEPENDENCY_CHANGE:   return ScalableSeiDependencyChange::create( (ScalableSeiDependencyChange*&) rpcSEIMessage );
	// JVT-S080 LMI }
    // JVT-W062
    case TL0_DEP_REP_IDX_SEI: return Tl0DepRepIdxSei::create( (Tl0DepRepIdxSei*&) rpcSEIMessage);
    // JVT-T073 {
	case SCALABLE_NESTING_SEI: return ScalableNestingSei::create( (ScalableNestingSei*&) rpcSEIMessage );
    // JVT-T073 }
    case PR_COMPONENT_INFO_SEI: return PRComponentInfoSei::create( (PRComponentInfoSei*&) rpcSEIMessage );
// JVT-V068 {
    case AVC_COMPATIBLE_HRD_SEI: 
      return AVCCompatibleHRD::create( (AVCCompatibleHRD*&)rpcSEIMessage, NULL );
    case PIC_TIMING:       
      return PicTiming::create( (PicTiming*&)rpcSEIMessage, NULL, 0 );
    case BUFFERING_PERIOD: 
      return BufferingPeriod::create( (BufferingPeriod*&) rpcSEIMessage, rpcParameterSetMng );
// JVT-V068 }
		// JVT-W049 {
	  case REDUNDANT_PIC_SEI:  
	    return RedundantPicSei::create( (RedundantPicSei*&) rpcSEIMessage );
    // JVT-W049 }
			//JVT-W052 wxwan
		case INTEGRITY_CHECK_SEI: return IntegrityCheckSEI::create((IntegrityCheckSEI*&) rpcSEIMessage );
			//JVT-W052 wxwan
		//JVT-X032 {
		case TL_SWITCHING_POINT_SEI:
			return TLSwitchingPointSei::create((TLSwitchingPointSei*&) rpcSEIMessage );
    //JVT-X032 }
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
//SEI changes update {
//SEI::ScalableSei::ScalableSei	() 
//: SEIMessage									( SCALABLE_SEI )
//// JVT-U085 LMI
//, m_temporal_level_nesting_flag( false )
////JVT-W051 {
//, m_quality_layer_info_present_flag( false )
//, m_ql_num_dId_minus1( 0 )
////JVT-W051 }
//, m_priority_id_setting_flag	 ( true )//JVT-W053
//, m_num_layers_minus1					( 0	)
//{	
//	::memset( m_layer_id, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
////	::memset( m_fgs_layer_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );  //JVT-S036 lsj
//	::memset( m_sub_pic_layer_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
//	::memset( m_sub_region_layer_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
//	//::memset( m_iroi_slice_division_info_present_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) ); //JVT-S036 lsj
//	::memset( m_iroi_division_info_present_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) ); //JVT-W051
//	::memset( m_profile_level_info_present_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
//	::memset( m_bitrate_info_present_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
//	::memset( m_frm_rate_info_present_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
//	::memset( m_frm_size_info_present_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
//	::memset( m_layer_dependency_info_present_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
//	::memset( m_init_parameter_sets_info_present_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
//	::memset( m_exact_sample_value_match_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );  //JVT-S036 lsj
//	JVT-W046 {
//	::memset( m_avc_layer_conversion_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
//  ::memset( m_avc_conversion_type_idc  , 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
//	::memset( m_avc_info_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool*) );
//	::memset( m_avc_profile_level_idc, 0x00, MAX_SCALABLE_LAYERS*sizeof(Int32*) );
//	::memset( m_avc_avg_bitrate, 0x00, MAX_SCALABLE_LAYERS*sizeof(Double*) );
//	::memset( m_avc_max_bitrate, 0x00, MAX_SCALABLE_LAYERS*sizeof(Double*) );
//	JVT-W046 }
//	//::memset( m_layer_profile_idc, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
//	::memset( m_layer_profile_level_idc, 0x00, MAX_SCALABLE_LAYERS*sizeof(Int32) );//JVT-W051
//	::memset( m_layer_constraint_set0_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
//	::memset( m_layer_constraint_set1_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
//	::memset( m_layer_constraint_set2_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
//	::memset( m_layer_constraint_set3_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
//	::memset( m_layer_level_idc, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
//
////JVT-S036 lsj start
//	::memset( m_profile_level_info_src_layer_id_delta, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );//
//
//	::memset( m_simple_priority_id, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );//
//    ::memset( m_discardable_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );//
//	::memset( m_temporal_level, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
//	::memset( m_dependency_id, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
//	::memset( m_quality_level, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
//
//	::memset( m_avg_bitrate, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
//	::memset( m_max_bitrate_layer, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );//
//	//::memset( m_max_bitrate_decoded_picture, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );//
//	::memset( m_max_bitrate_layer_representation, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );//JVT-W051
//	::memset( m_max_bitrate_calc_window, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );//
//
//	::memset( m_constant_frm_rate_idc, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
//	::memset( m_avg_frm_rate, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
//
//	::memset( m_frm_rate_info_src_layer_id_delta, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );//
//
//	::memset( m_frm_width_in_mbs_minus1, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
//	::memset( m_frm_height_in_mbs_minus1, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
//
//	::memset( m_frm_size_info_src_layer_id_delta, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );//
//
//	::memset( m_base_region_layer_id, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
//	::memset( m_dynamic_rect_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
//	::memset( m_horizontal_offset, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
//	::memset( m_vertical_offset, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
//	::memset( m_region_width, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
//	::memset( m_region_height, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
//
//	::memset( m_sub_region_info_src_layer_id_delta, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) ); //
//
//	::memset( m_roi_id, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
//
//	::memset( m_iroi_division_type, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
//	//::memset( m_grid_slice_width_in_mbs_minus1, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
//	::memset( m_grid_width_in_mbs_minus1, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );//JVT-W051
//	//::memset( m_grid_slice_height_in_mbs_minus1, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
//	::memset( m_grid_height_in_mbs_minus1, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );//JVT-W051
//	::memset( m_num_rois_minus1, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
//	::memset( m_first_mb_in_roi, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt*) );
//	::memset( m_roi_width_in_mbs_minus1, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt*) );
//	::memset( m_roi_height_in_mbs_minus1, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt*) );
//	::memset( m_slice_id, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt*) );
//
//	::memset( m_num_directly_dependent_layers, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
//	::memset( m_directly_dependent_layer_id_delta_minus1, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt*) ); //
//
//	::memset( m_layer_dependency_info_src_layer_id_delta, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) ); //
////JVT-S036 lsj end
//
//	::memset( m_num_init_seq_parameter_set_minus1, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
//	::memset( m_init_seq_parameter_set_id_delta, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt*) );
//	::memset( m_num_init_pic_parameter_set_minus1, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
//	::memset( m_init_pic_parameter_set_id_delta, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt*) );
//
//	::memset( m_init_parameter_sets_info_src_layer_id_delta, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) ); //JVT-S036 lsj
//	//JVT-W051 {
//	::memset( m_bitstream_restriction_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool));
//	::memset( m_motion_vectors_over_pic_boundaries_flag, 0x01, MAX_SCALABLE_LAYERS*sizeof(Bool));
//	::memset( m_max_bytes_per_pic_denom, 0x02, MAX_SCALABLE_LAYERS*sizeof(UInt));
//	::memset( m_max_bits_per_mb_denom, 0x01, MAX_SCALABLE_LAYERS*sizeof(UInt));
//	::memset( m_log2_max_mv_length_horizontal, 0x10, MAX_SCALABLE_LAYERS*sizeof(UInt));
//	::memset( m_log2_max_mv_length_vertical, 0x10, MAX_SCALABLE_LAYERS*sizeof(UInt));
//	::memset( m_max_dec_frame_buffering, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt));
//	::memset( m_num_reorder_frames, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt));
//	::memset( m_ql_dependency_id, 0x00, MAX_LAYERS*sizeof(UInt));
//	::memset( m_ql_num_minus1, 0x00, MAX_LAYERS*sizeof(UInt));
//	::memset( m_ql_id, 0x00, MAX_LAYERS*MAX_QUALITY_LEVELS*sizeof(UInt));
//	::memset( m_ql_profile_level_idc, 0x00, MAX_LAYERS*MAX_QUALITY_LEVELS*sizeof(Int32));
//	::memset( m_ql_avg_bitrate, 0x00, MAX_LAYERS*MAX_QUALITY_LEVELS*sizeof(UInt));
//	::memset( m_ql_max_bitrate, 0x00, MAX_LAYERS*MAX_QUALITY_LEVELS*sizeof(UInt));
//	//JVT-W051 }
//
//}

SEI::ScalableSei::ScalableSei	() 
: SEIMessage									( SCALABLE_SEI )
// JVT-U085 LMI
, m_temporal_id_nesting_flag( false )
//JVT-W051 {
, m_priority_layer_info_present_flag( false )
, m_pr_num_dId_minus1( 0 )
//JVT-W051 }
, m_priority_id_setting_flag	 ( true )//JVT-W053
, m_num_layers_minus1					( 0	)
{	
	::memset( m_layer_id, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_priority_id, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
  ::memset( m_discardable_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
	::memset( m_temporal_level, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_dependency_id, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_quality_level, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_sub_pic_layer_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
	::memset( m_sub_region_layer_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
	::memset( m_iroi_division_info_present_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) ); //JVT-W051
	::memset( m_profile_level_info_present_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
	::memset( m_bitrate_info_present_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
	::memset( m_frm_rate_info_present_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
	::memset( m_frm_size_info_present_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
	::memset( m_layer_dependency_info_present_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
  ::memset( m_parameter_sets_info_present_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
	::memset( m_bitstream_restriction_info_present_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
	::memset( m_exact_interlayer_pred_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
	::memset( m_exact_sample_value_match_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );  //JVT-S036
  ::memset( m_layer_conversion_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
	::memset( m_layer_output_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
	::memset( m_layer_profile_level_idc, 0x00, MAX_SCALABLE_LAYERS*sizeof(Int32) );//JVT-W051
	::memset( m_avg_bitrate, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_max_bitrate_layer, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_max_bitrate_layer_representation, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );//JVT-W051
	::memset( m_max_bitrate_calc_window, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_constant_frm_rate_idc, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_avg_frm_rate, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_frm_width_in_mbs_minus1, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_frm_height_in_mbs_minus1, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_base_region_layer_id, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_dynamic_rect_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
	::memset( m_horizontal_offset, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_vertical_offset, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_region_width, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_region_height, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_roi_id, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_iroi_grid_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool) );
	::memset( m_grid_width_in_mbs_minus1, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );//JVT-W051
	::memset( m_grid_height_in_mbs_minus1, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );//JVT-W051
	::memset( m_num_rois_minus1, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_first_mb_in_roi, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt*) );
	::memset( m_roi_width_in_mbs_minus1, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt*) );
	::memset( m_roi_height_in_mbs_minus1, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt*) );
	::memset( m_num_directly_dependent_layers, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_directly_dependent_layer_id_delta_minus1, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt*) );
	::memset( m_layer_dependency_info_src_layer_id_delta, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_num_seq_parameter_set_minus1, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_seq_parameter_set_id_delta, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt*) );
	::memset( m_num_subset_seq_parameter_set_minus1, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
  ::memset( m_subset_seq_parameter_set_id_delta, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt*) );
	::memset( m_num_pic_parameter_set_minus1, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_pic_parameter_set_id_delta, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt*) );
	::memset( m_parameter_sets_info_src_layer_id_delta, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) ); //JVT-S036
	//JVT-W051 {	
	::memset( m_motion_vectors_over_pic_boundaries_flag, 0x01, MAX_SCALABLE_LAYERS*sizeof(Bool));
	::memset( m_max_bytes_per_pic_denom, 0x02, MAX_SCALABLE_LAYERS*sizeof(UInt));
	::memset( m_max_bits_per_mb_denom, 0x01, MAX_SCALABLE_LAYERS*sizeof(UInt));
	::memset( m_log2_max_mv_length_horizontal, 0x10, MAX_SCALABLE_LAYERS*sizeof(UInt));
	::memset( m_log2_max_mv_length_vertical, 0x10, MAX_SCALABLE_LAYERS*sizeof(UInt));
	::memset( m_num_reorder_frames, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt));
	::memset( m_max_dec_frame_buffering, 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt));	
	//JVT-W051 }
	//JVT-W046 {
  ::memset( m_conversion_type_idc  , 0x00, MAX_SCALABLE_LAYERS*sizeof(UInt) );
	::memset( m_rewriting_info_flag, 0x00, MAX_SCALABLE_LAYERS*sizeof(Bool*) );
	::memset( m_rewriting_profile_level_idc, 0x00, MAX_SCALABLE_LAYERS*sizeof(Int32*) );
	::memset( m_rewriting_avg_bitrate, 0x00, MAX_SCALABLE_LAYERS*sizeof(Double*) );
	::memset( m_rewriting_max_bitrate, 0x00, MAX_SCALABLE_LAYERS*sizeof(Double*) );
	//JVT-W046 }
	//JVT-W051 {
	::memset( m_pr_dependency_id, 0x00, MAX_LAYERS*sizeof(UInt));
	::memset( m_pr_num_minus1, 0x00, MAX_LAYERS*sizeof(UInt));
	::memset( m_pr_id, 0x00, MAX_LAYERS*MAX_QUALITY_LEVELS*sizeof(UInt));
	::memset( m_pr_profile_level_idc, 0x00, MAX_LAYERS*MAX_QUALITY_LEVELS*sizeof(Int32));
	::memset( m_pr_avg_bitrate, 0x00, MAX_LAYERS*MAX_QUALITY_LEVELS*sizeof(UInt));
	::memset( m_pr_max_bitrate, 0x00, MAX_LAYERS*MAX_QUALITY_LEVELS*sizeof(UInt));
	//JVT-W051 }
}

SEI::ScalableSei::~ScalableSei()
{
	// JVT-S054 (ADD) ->
	UInt i;
	for( i = 0; i < MAX_SCALABLE_LAYERS; i++ )
	{
		if ( m_first_mb_in_roi[i] != NULL )
		{
			free( m_first_mb_in_roi[i] );
			m_first_mb_in_roi[i] = NULL;
		}
		if ( m_roi_width_in_mbs_minus1[i] != NULL )
		{
			free( m_roi_width_in_mbs_minus1[i] );
			m_roi_width_in_mbs_minus1[i] = NULL;
		}
		if ( m_roi_height_in_mbs_minus1[i] != NULL )
		{
			free( m_roi_height_in_mbs_minus1[i] );
			m_roi_height_in_mbs_minus1[i] = NULL;
		}
    //SEI changes update
		//if ( m_slice_id[i] != NULL )
		//{
		//	free( m_slice_id[i] );
		//	m_slice_id[i] = NULL;
		//}
	}
	// JVT-S054 (ADD) <-
}

ErrVal
SEI::ScalableSei::create( ScalableSei*& rpcSeiMessage )
{
	rpcSeiMessage = new ScalableSei();
	ROT( NULL == rpcSeiMessage )
		return Err::m_nOK;
}

//TMM_FIX
ErrVal
SEI::ScalableSei::destroy() 
{
	delete this;
	return Err::m_nOK;
}
//TMM_FIX

//ErrVal
//SEI::ScalableSei::write( HeaderSymbolWriteIf *pcWriteIf )
//{
//  UInt i=0, j=0;
//  // JVT-U085 LMI
//  RNOK    ( pcWriteIf->writeFlag( m_temporal_level_nesting_flag,                "ScalableSEI: temporal_level_nesting_flag"                      ) );
//	RNOK(pcWriteIf->writeFlag(m_quality_layer_info_present_flag,	"ScalableSEI: quality_layer_info_present_flag"));//JVT-W051 
//	RNOK    ( pcWriteIf->writeFlag( m_priority_id_setting_flag,                   "ScalableSEI: priority_id_setting_flag" ) );//JVT-W053
//	ROF( m_num_layers_minus1+1 );
//	RNOK		( pcWriteIf->writeUvlc(m_num_layers_minus1,													"ScalableSEI: num_layers_minus1"											) );
//
//	for( i = 0; i <= m_num_layers_minus1; i++ )
//	{
//		if (0 < m_aiNumRoi[m_dependency_id[i]])
//		{
//			m_sub_pic_layer_flag[i] = true;
//			m_roi_id[i]				= m_aaiRoiID[m_dependency_id[i]][0];
//		}	
//
//		//JVT-W051 {
//		//RNOK	( pcWriteIf->writeCode( m_layer_id[i],												8,		"ScalableSEI: layer_id"															) );
//		RNOK	( pcWriteIf->writeUvlc( m_layer_id[i],	"ScalableSEI: layer_id"	) );
//		//JVT-W051 }
//	//JVT-S036 lsj start
//		//RNOK	( pcWriteIf->writeFlag( m_fgs_layer_flag[i],												"ScalableSEI: fgs_layer_flag"												) );
//		RNOK	( pcWriteIf->writeCode( m_simple_priority_id[i],					6,		"ScalableSEI: simple_priority_id"										) ); 
//		RNOK    ( pcWriteIf->writeFlag( m_discardable_flag[i],								"ScalableSEI: discardable_flag"											) );  
//		RNOK	( pcWriteIf->writeCode( m_temporal_level[i],								3,		"ScalableSEI: temporal_level"												) );
//		RNOK	( pcWriteIf->writeCode( m_dependency_id[i],							3,		"ScalableSEI: dependency_level"											) );
//    RNOK	( pcWriteIf->writeCode( m_quality_level[i],									4,		"ScalableSEI: quality_level"													) );
//		RNOK	( pcWriteIf->writeFlag( m_sub_pic_layer_flag[i],										"ScalableSEI: sub_pic_layer_flag"										) );
//		RNOK	( pcWriteIf->writeFlag( m_sub_region_layer_flag[i],									"ScalableSEI: sub_region_layer_flag"									) );
//		//RNOK	( pcWriteIf->writeFlag( m_iroi_slice_division_info_present_flag[i],					"ScalableSEI: iroi_slice_division_info_present_flag"					) ); 
//		RNOK	( pcWriteIf->writeFlag( m_iroi_division_info_present_flag[i],					"ScalableSEI: iroi_division_info_present_flag"					) ); //JVT-W051
//		RNOK	( pcWriteIf->writeFlag( m_profile_level_info_present_flag[i],				"ScalableSEI: profile_level_info_present_flag"				) );
//	//JVT-S036 lsj end
//		RNOK	( pcWriteIf->writeFlag( m_bitrate_info_present_flag[i],							"ScalableSEI: bitrate_info_present_flag"							) );
//		RNOK	( pcWriteIf->writeFlag( m_frm_rate_info_present_flag[i],						"ScalableSEI: frm_rate_info_present_flag"						) );
//		RNOK	( pcWriteIf->writeFlag( m_frm_size_info_present_flag[i],						"ScalableSEI: frm_size_info_present_flag"						) );
//		RNOK	( pcWriteIf->writeFlag( m_layer_dependency_info_present_flag[i],		"ScalableSEI: layer_dependency_info_present_flag"		) );
//		RNOK	( pcWriteIf->writeFlag( m_init_parameter_sets_info_present_flag[i],	"ScalableSEI: init_parameter_sets_info_present_flag" ) );
//		RNOK	( pcWriteIf->writeFlag(	m_bitstream_restriction_flag[i],					  "ScalableSEI: bitstream_restriction_info_present_flag" ));  //JVT-051 & JVT-W064
//		RNOK	( pcWriteIf->writeFlag( m_exact_interlayer_pred_flag[i],						"ScalableSEI: exact_interlayer_pred_flag"                      ) );//JVT-S036 lsj 
//    RNOK	( pcWriteIf->writeFlag( m_avc_layer_conversion_flag[i],					   	"ScalableSEI: avc_layer_conversion_flag"                      ) );//JVT-W046
//		RNOK	( pcWriteIf->writeFlag( m_layer_output_flag[i],											"ScalableSEI: layer_output_flag		" ) );//JVT-W047 wxwan
//		if ( m_profile_level_info_present_flag[i] )
//		{
//			//JVT-W051 {
//			RNOK	( pcWriteIf->writeCode( m_layer_profile_level_idc[i],							24,		"ScalableSEI: layer_profile_idc"								) );
//			//RNOK	( pcWriteIf->writeCode( m_layer_profile_level_idc[i],							8,		"ScalableSEI: layer_profile_idc"											) );
//			//RNOK	( pcWriteIf->writeFlag( m_layer_constraint_set0_flag[i],					"ScalableSEI: layer_constraint_set0_flag"						) );
//			//RNOK	( pcWriteIf->writeFlag( m_layer_constraint_set1_flag[i],					"ScalableSEI: layer_constraint_set1_flag"						) );
//			//RNOK	( pcWriteIf->writeFlag( m_layer_constraint_set2_flag[i],					"ScalableSEI: layer_constraint_set2_flag"						) );
//			//RNOK	( pcWriteIf->writeFlag( m_layer_constraint_set3_flag[i],					"ScalableSEI: layer_constraint_set3_flag"						) );
//			//RNOK	( pcWriteIf->writeCode( 0,																	4,		"ScalableSEI: reserved_zero_4bits"										) );
//			//RNOK	( pcWriteIf->writeCode( m_layer_level_idc[i],								8,		"ScalableSEI: layer_level_idc"												) );
//			//JVT-W051 }
//		}
//
//		else
//		{//JVT-S036 lsj
//			RNOK	( pcWriteIf->writeUvlc( m_profile_level_info_src_layer_id_delta[i], "ScalableSEI: profile_level_info_src_layer_id_delta"      ) ); 
//		}
//
///*		if ( m_decoding_dependency_info_present_flag[i] )
//		{
//			RNOK	( pcWriteIf->writeCode( m_simple_priority_id[i],					6,		"ScalableSEI: simple_priority_id"										) ); 
//			RNOK    ( pcWriteIf->writeFlag( m_discardable_flag[i],								"ScalableSEI: discardable_flag"											) );  
//			RNOK	( pcWriteIf->writeCode( m_temporal_level[i],								3,		"ScalableSEI: temporal_level"												) );
//			RNOK	( pcWriteIf->writeCode( m_dependency_id[i],							3,		"ScalableSEI: dependency_level"											) );
//			RNOK	( pcWriteIf->writeCode( m_quality_level[i],									2,		"ScalableSEI: quality_level"													) );
//		}
//JVT-S036 lsj */
//		if ( m_bitrate_info_present_flag[i] )
//		{
//			RNOK	( pcWriteIf->writeCode( m_avg_bitrate[i],										16,		"ScalableSEI: avg_bitrate"														) );
////JVT-S036 lsj start
//			RNOK	( pcWriteIf->writeCode( m_max_bitrate_layer[i],										16,		"ScalableSEI: max_bitrate_layer"											) );
//			//RNOK	( pcWriteIf->writeCode( m_max_bitrate_decoded_picture[i],										16,		"ScalableSEI: max_bitrate_decoded_picture"						) );
//			RNOK	( pcWriteIf->writeCode( m_max_bitrate_layer_representation[i],										16,		"ScalableSEI: max_bitrate_layer_representation"						) );//JVT-W051
//			RNOK	( pcWriteIf->writeCode( m_max_bitrate_calc_window[i],							16,		"ScalableSEI: max_bitrate_calc_window"											) );
////JVT-S036 lsj end
//		}
//		if ( m_frm_rate_info_present_flag[i] )
//		{
//			RNOK	( pcWriteIf->writeCode( m_constant_frm_rate_idc[i],			2,		"ScalableSEI: constant_frm_bitrate_idc"							) );
//			RNOK	( pcWriteIf->writeCode( m_avg_frm_rate[i],									16,		"ScalableSEI: avg_frm_rate"													) );
//		}
//		else
//		{
//	//JVT-S036 lsj
//			RNOK	(pcWriteIf->writeUvlc( m_frm_rate_info_src_layer_id_delta[i],	"ScalableSEI: frm_rate_info_src_layer_id_delta"			) );
//		}
//
//		if ( m_frm_size_info_present_flag[i] )
//		{
//			RNOK	( pcWriteIf->writeUvlc( m_frm_width_in_mbs_minus1[i],							"ScalableSEI: frm_width_in_mbs_minus1"								) );
//			RNOK	( pcWriteIf->writeUvlc( m_frm_height_in_mbs_minus1[i],						"ScalableSEI: frm_height_in_mbs_minus1"							) );
//		}
//		else
//		{//JVT-S036 lsj
//			RNOK	(pcWriteIf->writeUvlc( m_frm_size_info_src_layer_id_delta[i],	"ScalableSEI: frm_size_info_src_layer_id_delta"			) );
//		}
//
//		if ( m_sub_region_layer_flag[i] )
//		{
//			//JVT-W051 {
//			//RNOK	( pcWriteIf->writeCode ( m_base_region_layer_id[i],					8,		"ScalableSEI: base_region_layer_id"									) );
//			RNOK	( pcWriteIf->writeUvlc ( m_base_region_layer_id[i],	"ScalableSEI: base_region_layer_id"	));
//			//JVT-W051 }
//			RNOK	( pcWriteIf->writeFlag ( m_dynamic_rect_flag[i],									"ScalableSEI: dynamic_rect_flag"											) );
//			//JVT-W051 {
//			//if ( m_dynamic_rect_flag[i] )			
//			if ( !m_dynamic_rect_flag[i] )
//			//JVT-W051 }
//			{
//				RNOK	( pcWriteIf->writeCode ( m_horizontal_offset[i],					16,		"ScalableSEI: horizontal_offset"											) );
//				RNOK	( pcWriteIf->writeCode ( m_vertical_offset[i],						16,		"ScalableSEI: vertical_offset"												) );
//				RNOK	( pcWriteIf->writeCode ( m_region_width[i],								16,		"ScalableSEI: region_width"													) );
//				RNOK	( pcWriteIf->writeCode ( m_region_height[i],							16,		"ScalableSEI: region_height"													) );
//			}
//		}
//		else
//		{//JVT-S036 lsj
//			RNOK	( pcWriteIf->writeUvlc( m_sub_region_info_src_layer_id_delta[i],		"ScalableSEI: sub_region_info_src_layer_id_delta"				) );
//		}
//
//		if( m_sub_pic_layer_flag[i] )
//		{//JVT-S036 lsj
//			//JVT-W051 {
//			//RNOK	( pcWriteIf->writeCode( m_roi_id[i],		3,								"Scalable: roi_id"					) );
//			RNOK	( pcWriteIf->writeUvlc( m_roi_id[i],	"Scalable: roi_id" ) );
//			//JVT-W051 }
//		}
//
//	//JVT-S036 lsj start
//		//if ( m_iroi_slice_division_info_present_flag[i] )
//		if ( m_iroi_division_info_present_flag[i] )//JVT-W051
//		{
//			//JVT-W051 {
//			//RNOK	( pcWriteIf->writeCode( m_iroi_division_type[i],		2,		"ScalableSEI:iroi_slice_division_type" ) );
//			RNOK	( pcWriteIf->writeCode( m_iroi_division_type[i],		1,		"ScalableSEI:iroi_slice_division_type" ) );
//			//JVT-W051 }
//			if( m_iroi_division_type[i] == 0 )
//			{
//				//RNOK	( pcWriteIf->writeUvlc( m_grid_slice_width_in_mbs_minus1[i],    "ScalableSEI:grid_slice_width_in_mbs_minus1" ) );
//				//RNOK	( pcWriteIf->writeUvlc( m_grid_slice_height_in_mbs_minus1[i],    "ScalableSEI:grid_slice_height_in_mbs_minus1" ) );
//				RNOK	( pcWriteIf->writeUvlc( m_grid_width_in_mbs_minus1[i],    "ScalableSEI:grid_width_in_mbs_minus1" ) );//JVT-W051
//				RNOK	( pcWriteIf->writeUvlc( m_grid_height_in_mbs_minus1[i],    "ScalableSEI:grid_height_in_mbs_minus1" ) );//JVT-W051
//			}
//			else if( m_iroi_division_type[i] == 1 )
//			{
//				RNOK	( pcWriteIf->writeUvlc( m_num_rois_minus1[i],		"ScalableSEI:num_slice_minus1" ) );
//				for ( j = 0; j <= m_num_rois_minus1[i]; j++ )
//				{
//					RNOK	( pcWriteIf->writeUvlc( m_first_mb_in_roi[i][j],				"ScalableSEI: first_mb_in_slice" ) );
//					RNOK	( pcWriteIf->writeUvlc( m_roi_width_in_mbs_minus1[i][j],		"ScalableSEI:slice_width_in_mbs_minus1" ) );
//					RNOK	( pcWriteIf->writeUvlc( m_roi_height_in_mbs_minus1[i][j],		"ScalableSEI:slice_height_in_mbs_minus1" ) );
//				}
//			}
//      // JVT-S054 (REPLACE): Typo error
//			//else if ( m_iroi_division_type[1] == 2 )
//			else if ( m_iroi_division_type[i] == 2 )
//			{
//				RNOK	( pcWriteIf->writeUvlc( m_num_rois_minus1[i],		"ScalableSEI:num_slice_minus1" ) );
//    		// JVT-S054 (REPLACE) ->
//        /*
//				UInt uiFrameHeightInMb = m_roi_height_in_mbs_minus1[i][j] + 1;
//				UInt uiFrameWidthInMb  = m_roi_width_in_mbs_minus1[i][j] + 1;
//				UInt uiPicSizeInMbs = uiFrameHeightInMb * uiFrameWidthInMb;
//				for ( j = 0; j < uiPicSizeInMbs; j++ )
//				{
//					RNOK	( pcWriteIf->writeUvlc( m_slice_id[i][j],		"ScalableSEI:slice_id"		   ) );
//				}
//        */
//				UInt uiFrameHeightInMb = m_roi_height_in_mbs_minus1[i][j] + 1;
//				UInt uiFrameWidthInMb  = m_roi_width_in_mbs_minus1[i][j] + 1;
//				UInt uiPicSizeInMbs = uiFrameHeightInMb * uiFrameWidthInMb;
//			  UInt uiWriteBits = (UInt) ceil( log( (double) (m_num_rois_minus1[i] + 1) ) / log(2.) );
//				if (uiWriteBits == 0)
//					uiWriteBits = 1;
//				for ( j = 0; j < uiPicSizeInMbs; j++ )
//				{
//					RNOK	( pcWriteIf->writeCode ( m_slice_id[i][j],	uiWriteBits,		"ScalableSEI: slice_id") );
//				}
//    		// JVT-S054 (REPLACE) <-
//			}
//		}
//	//JVT-S036 lsj end
//		if ( m_layer_dependency_info_present_flag[i] )
//		{
//			RNOK	( pcWriteIf->writeUvlc ( m_num_directly_dependent_layers[i],			"ScalableSEI: num_directly_dependent_layers"					) );
//// BUG_FIX liuhui{
//		    for( j = 0; j < m_num_directly_dependent_layers[i]; j++ )
//		    {
//		      RNOK( pcWriteIf->writeUvlc (m_directly_dependent_layer_id_delta_minus1[i][j],      "ScalableSEI: directly_dependent_layers_id_delta"		) );  //JVT-S036 lsj
//		    }
//// BUG_FIX liuhui}
//		}
//		else
//		{//JVT-S036 lsj
//			RNOK( pcWriteIf->writeUvlc(m_layer_dependency_info_src_layer_id_delta[i],	 "ScalableSEI: layer_dependency_info_src_layer_id_delta"     ) );
//		}
//
//		if ( m_init_parameter_sets_info_present_flag[i] ) 
//		{
//
//// BUG_FIX liuhui{
//			RNOK	( pcWriteIf->writeUvlc ( m_num_init_seq_parameter_set_minus1[i],	"ScalableSEI: num_init_seq_parameter_set_minus1"			) );
//			for( j = 0; j <= m_num_init_seq_parameter_set_minus1[i]; j++ )
//			{
//			  RNOK( pcWriteIf->writeUvlc( m_init_seq_parameter_set_id_delta[i][j],  "ScalableSEI: init_seq_parameter_set_id_delta"				    ) );
//			}
//
//			RNOK    ( pcWriteIf->writeUvlc ( m_num_init_pic_parameter_set_minus1[i],	"ScalableSEI: num_init_pic_parameter_set_minus1"			) );
//			for( j = 0; j <= m_num_init_pic_parameter_set_minus1[i]; j++ )
//			{
//			  RNOK ( pcWriteIf->writeUvlc ( m_init_pic_parameter_set_id_delta[i][j],"ScalableSEI: init_pic_parameter_set_id_delta"				    ) );
//			}
//// BUG_FIX liuhui}
//		}
//		else
//		{//JVT-S036 lsj 
//			RNOK	(pcWriteIf->writeUvlc( m_init_parameter_sets_info_src_layer_id_delta[i], "ScalableSEI: init_parameter_sets_info_src_layer_id_delta"	) );
//		}
//		//JVT-W051 & JVT-W064 {
//		if (m_bitstream_restriction_flag[i])
//		{
//			RNOK	(pcWriteIf->writeFlag(m_motion_vectors_over_pic_boundaries_flag[i], "ScalableSEI: motion_vectors_over_pic_boundaries_flag") );
//			RNOK	(pcWriteIf->writeUvlc(m_max_bytes_per_pic_denom[i], "ScalableSEI: max_bytes_per_pic_denom") );
//			RNOK	(pcWriteIf->writeUvlc(m_max_bits_per_mb_denom[i], "ScalableSEI: max_bits_per_mb_denom") );
//			RNOK	(pcWriteIf->writeUvlc(m_log2_max_mv_length_horizontal[i], "ScalableSEI: log2_max_mv_length_horizontal") );
//			RNOK	(pcWriteIf->writeUvlc(m_log2_max_mv_length_vertical[i], "ScalableSEI: log2_max_mv_length_vertical") );
//			RNOK	(pcWriteIf->writeUvlc(m_num_reorder_frames[i], "ScalableSEI: num_reorder_frames"));
//			RNOK	(pcWriteIf->writeUvlc(m_max_dec_frame_buffering[i],                   "ScalableSEI: max_dec_frame_buffering") );
//		}
//		//JVT-W051 & JVT-W064  }
//		//JVT-W046 {
//		if( m_avc_layer_conversion_flag[i] )
//		{
//		  RNOK ( pcWriteIf->writeUvlc ( m_avc_conversion_type_idc[i],                   "ScalableSEI: m_avc_conversion_type_idc"				    ) );
//		  for ( j=0; j<2; j++ )
//		  {
//		    RNOK	( pcWriteIf->writeFlag ( m_avc_info_flag[i][j],						"ScalableSEI: m_avc_info_flag"								) );
//			if( m_avc_info_flag[i][j] )
//			{
//			  RNOK	( pcWriteIf->writeCode( m_avc_profile_level_idc[i][j],		24,		"ScalableSEI:m_avc_profile_level_idc" ) );
//			  RNOK	( pcWriteIf->writeCode( m_avc_avg_bitrate[i][j],		16,		"ScalableSEI:m_avc_avg_bitrate" ) );
//			  RNOK	( pcWriteIf->writeCode( m_avc_max_bitrate[i][j],		16,		"ScalableSEI:m_avc_max_bitrate" ) );
//			} 
//		  }
//		}
//		//JVT-W046 }
//	}
//	//JVT-W051 {
//	if (m_quality_layer_info_present_flag)
//	{
//		RNOK	(pcWriteIf->writeUvlc(m_ql_num_dId_minus1, "ScalableSEI: ql_num_dId_minus1"));
//		for (i=0; i<=m_ql_num_dId_minus1; i++)
//		{
//			RNOK	(pcWriteIf->writeCode(m_ql_dependency_id[i], 3, "ScalableSEI: ql_dependency_id"));
//			RNOK	(pcWriteIf->writeUvlc(m_ql_num_minus1[i], "ScalableSEI: ql_num_minus"));
//			for (j=0; j<=m_ql_num_minus1[i]; j++)
//			{
//				RNOK	(pcWriteIf->writeUvlc(m_ql_id[i][j], "ScalableSEI: ql_id"));
//				RNOK	(pcWriteIf->writeCode(m_ql_profile_level_idc[i][j], 24, "ScalableSEI: ql_profile_level_idc"));
//				RNOK	(pcWriteIf->writeCode(m_ql_avg_bitrate[i][j], 16, "ScalableSEI: ql_avg_bitrate"));
//				RNOK	(pcWriteIf->writeCode(m_ql_max_bitrate[i][j], 16, "ScalableSEI: ql_max_bitrate"));
//			}
//		}
//	}
//	//JVT-W051 }
//	//JVT-W053 wxwan
//	if(m_priority_id_setting_flag)
//	{
//		UInt PriorityIdSettingUriIdx = 0;
//		do{
//			UInt uiTemp = priority_id_setting_uri[PriorityIdSettingUriIdx];
//			RNOK( pcWriteIf->writeCode( uiTemp,   8,    "ScalableSEI: priority_id_setting_uri" ) );
//		}while( priority_id_setting_uri[ PriorityIdSettingUriIdx++ ]  !=  0 );
//	}
//	//JVT-W053 wxwan
//
//	return Err::m_nOK;
//}
//ErrVal
//SEI::ScalableSei::read ( HeaderSymbolReadIf *pcReadIf )
//{
//  UInt i, j=0;
//  UInt rl;//JVT-S036 lsj 
//  // JVT-U085 LMI
//  RNOK  ( pcReadIf->getFlag( m_temporal_level_nesting_flag,                          "" ) );
//	RNOK	(pcReadIf->getFlag( m_quality_layer_info_present_flag,					""));//JVT-W051
//	RNOK  ( pcReadIf->getFlag( m_priority_id_setting_flag,                          "" ) );//JVT-W053
//	RNOK	( pcReadIf->getUvlc( m_num_layers_minus1 ,																""	) );
//
//	for ( i = 0; i <= m_num_layers_minus1; i++ )
//	{
//		//JVT-W051 {
//		//RNOK	( pcReadIf->getCode( m_layer_id[i],																	8,			""	) );
//		RNOK	(pcReadIf->getUvlc( m_layer_id[i],	""));
//		//JVT-W051 }
//	//JVT-S036 lsj start
////		RNOK	( pcReadIf->getFlag( m_fgs_layer_flag[i],																""	) ); 
//		RNOK	( pcReadIf->getCode( m_simple_priority_id[i],													6,		"" ) );	
//		RNOK	( pcReadIf->getFlag( m_discardable_flag[i],													"" ) );     
//		RNOK	( pcReadIf->getCode( m_temporal_level[i],													3,		""	) );
//		RNOK	( pcReadIf->getCode( m_dependency_id[i],												3,		""	) );
//    RNOK	( pcReadIf->getCode( m_quality_level[i],													4,		""	) );
//		RNOK	( pcReadIf->getFlag( m_sub_pic_layer_flag[i],														""	) );
//		RNOK	( pcReadIf->getFlag( m_sub_region_layer_flag[i],													""	) );
//		//RNOK	( pcReadIf->getFlag( m_iroi_slice_division_info_present_flag[i],						""  ) ); 
//		RNOK	( pcReadIf->getFlag( m_iroi_division_info_present_flag[i],						""  ) );//JVT-W051
//		RNOK	( pcReadIf->getFlag( m_profile_level_info_present_flag[i],								""	) );
//   //JVT-S036 lsj end	
//		RNOK	( pcReadIf->getFlag( m_bitrate_info_present_flag[i],											""	) );
//		RNOK	( pcReadIf->getFlag( m_frm_rate_info_present_flag[i],											""	) );
//		RNOK	( pcReadIf->getFlag( m_frm_size_info_present_flag[i],											""	) );
//		RNOK	( pcReadIf->getFlag( m_layer_dependency_info_present_flag[i],								""	) );
//		RNOK	( pcReadIf->getFlag( m_init_parameter_sets_info_present_flag[i],						""	) );
//		RNOK	( pcReadIf->getFlag( m_bitstream_restriction_flag[i], ""));//JVT-W051
//		RNOK	( pcReadIf->getFlag( m_exact_interlayer_pred_flag[i],											""  ) );	//JVT-S036 lsj
//    RNOK	( pcReadIf->getFlag( m_avc_layer_conversion_flag[i],											""  ) );//JVT-W046
//		RNOK	( pcReadIf->getFlag( m_layer_output_flag[i],											""  ) );	//JVT-W047 wxwan
//		if( m_profile_level_info_present_flag[i] )
//		{
//			//JVT-W051 {
//			//RNOK	( pcReadIf->getCode( m_layer_profile_level_idc[i],											8,		""	) );
//			UInt uiTmp;
//			m_layer_profile_level_idc[i] = 0;
//			RNOK	( pcReadIf->getCode( uiTmp, 16, ""));
//			m_layer_profile_level_idc[i] += uiTmp;
//			RNOK	( pcReadIf->getCode( uiTmp, 8, ""));
//			m_layer_profile_level_idc[i] = m_layer_profile_level_idc[i] << 8;
//			m_layer_profile_level_idc[i] += uiTmp;
//			//RNOK	( pcReadIf->getFlag( m_layer_constraint_set0_flag[i],										""	) );
//			//RNOK	( pcReadIf->getFlag( m_layer_constraint_set1_flag[i],										""	) );
//			//RNOK	( pcReadIf->getFlag( m_layer_constraint_set2_flag[i],										""	) );
//			//RNOK	( pcReadIf->getFlag( m_layer_constraint_set3_flag[i],										""	) );
//			//UInt uiReserved;
//			//RNOK	( pcReadIf->getCode( uiReserved,																	4,		""	) );
//			//RNOK	( pcReadIf->getCode( m_layer_level_idc[i],												8,		""	) );
//			//JVT-W051 }
//		}
//		else
//		{//JVT-S036 lsj
//			RNOK	( pcReadIf->getUvlc( m_profile_level_info_src_layer_id_delta[i],						""  ) );
//			rl = m_layer_id[i] - m_profile_level_info_src_layer_id_delta[i];
//			//m_layer_profile_idc[i] = m_layer_profile_idc[rl];
//			m_layer_profile_level_idc[i] = m_layer_profile_level_idc[rl];//JVT-W051
//			m_layer_constraint_set0_flag[i] = m_layer_constraint_set0_flag[rl];
//			m_layer_constraint_set1_flag[i] = m_layer_constraint_set1_flag[rl];
//			m_layer_constraint_set2_flag[i] = m_layer_constraint_set2_flag[rl];
//			m_layer_constraint_set3_flag[i] = m_layer_constraint_set3_flag[rl];
//			m_layer_level_idc[i] = m_layer_level_idc[rl];
//		}
//
//	/*	if( m_decoding_dependency_info_present_flag[i] )
//		{
//			RNOK	( pcReadIf->getCode( m_simple_priority_id[i],													6,		"" ) );	
//			RNOK	( pcReadIf->getFlag( m_discardable_flag[i],													"" ) );     
//			RNOK	( pcReadIf->getCode( m_temporal_level[i],													3,		""	) );
//			RNOK	( pcReadIf->getCode( m_dependency_id[i],												3,		""	) );
//			RNOK	( pcReadIf->getCode( m_quality_level[i],													2,		""	) );
//		}
//JVT-S036 lsj */
//		if( m_bitrate_info_present_flag[i] )
//		{
//			RNOK	( pcReadIf->getCode( m_avg_bitrate[i],														16,		""	) );
//	//JVT-S036 lsj start
//			RNOK	( pcReadIf->getCode( m_max_bitrate_layer[i],														16,		""	) );
//			//RNOK	( pcReadIf->getCode( m_max_bitrate_decoded_picture[i],														16,		""	) );
//			RNOK	( pcReadIf->getCode( m_max_bitrate_layer_representation[i],														16,		""	) );//JVT-W051
//			RNOK	( pcReadIf->getCode( m_max_bitrate_calc_window[i],											16,		""	) );
//	//JVT-S036 lsj end
//		}
//
//		if( m_frm_rate_info_present_flag[i] )
//		{
//			RNOK	( pcReadIf->getCode( m_constant_frm_rate_idc[i],									2,		""	) );
//			RNOK	( pcReadIf->getCode( m_avg_frm_rate[i],														16,		""	) );
//		}
//		else
//		{//JVT-S036 lsj
//			RNOK	( pcReadIf->getUvlc( m_frm_rate_info_src_layer_id_delta[i],							"" ) );
//			rl = m_layer_id[i] - m_frm_rate_info_src_layer_id_delta[i];
//			m_constant_frm_rate_idc[i] = m_constant_frm_rate_idc[rl];
//			m_avg_frm_rate[i] = m_avg_frm_rate[rl];
//		}
//
//		if( m_frm_size_info_present_flag[i] )
//		{
//			RNOK	( pcReadIf->getUvlc( m_frm_width_in_mbs_minus1[i],											""	) );
//			RNOK	( pcReadIf->getUvlc( m_frm_height_in_mbs_minus1[i],											""	) );
//		}
//		else
//		{//JVT-S036 lsj 
//			RNOK	( pcReadIf->getUvlc( m_frm_size_info_src_layer_id_delta[i],							""  ) );
//			rl = m_layer_id[i] - m_frm_size_info_src_layer_id_delta[i];
//			m_frm_width_in_mbs_minus1[i] = m_frm_width_in_mbs_minus1[rl];
//			m_frm_width_in_mbs_minus1[i] = m_frm_width_in_mbs_minus1[rl];
//		}
//
//		if( m_sub_region_layer_flag[i] )
//		{
//			//JVT-W051 {
//			//RNOK	( pcReadIf->getCode( m_base_region_layer_id[i],										8,		""	) );
//			RNOK	(pcReadIf->getUvlc( m_base_region_layer_id[i], ""));
//			//JVT-W051 }
//			RNOK	( pcReadIf->getFlag( m_dynamic_rect_flag[i],														""	) );
//			//JVT-W051 {
//			//if( m_dynamic_rect_flag[i] )
//			if ( !m_dynamic_rect_flag[i] )
//			//JVT-W051 }
//			{
//				RNOK( pcReadIf->getCode( m_horizontal_offset[i],											16,		""	) );
//				RNOK( pcReadIf->getCode( m_vertical_offset[i],												16,		""	) );
//				RNOK( pcReadIf->getCode( m_region_width[i],														16,		""	) );
//				RNOK( pcReadIf->getCode( m_region_height[i],													16,		""	) );
//			}
//		}
//		else
//		{//JVT-S036 lsj 
//			RNOK	( pcReadIf->getUvlc( m_sub_region_info_src_layer_id_delta[i],						""  ) );
//			rl = m_layer_id[i] - m_sub_region_info_src_layer_id_delta[i];
//			m_base_region_layer_id[i] = m_base_region_layer_id[rl];
//			m_dynamic_rect_flag[i] = m_dynamic_rect_flag[rl];
//			if( m_dynamic_rect_flag[i] )
//			{
//				 m_horizontal_offset[i] = m_horizontal_offset[rl];
//				 m_vertical_offset[i] = m_vertical_offset[rl];
//				 m_region_width[i] = m_region_width[rl];
//				 m_region_height[i] = m_region_height[rl];
//			}
//
//		}
//
//	//JVT-S036 lsj start
//		if( m_sub_pic_layer_flag[i] )
//		{
//			//JVT-W051 {
//			//RNOK	( pcReadIf->getCode( m_roi_id[i],		3,								""					) );
//			RNOK	( pcReadIf->getUvlc( m_roi_id[i], ""));
//			//JVT-W051 }
//		}
//		//if ( m_iroi_slice_division_info_present_flag[i] )
//		if ( m_iroi_division_info_present_flag[i] )//JVT-W051
//		{
//			//JVT-W051 {
//			//RNOK	( pcReadIf->getCode( m_iroi_division_type[i],		2,		"ScalableSEI:iroi_slice_division_type" ) );
//			RNOK	( pcReadIf->getCode( m_iroi_division_type[i],		1,		"ScalableSEI:iroi_slice_division_type" ) );
//			//JVT-W051 }
//			if( m_iroi_division_type[i] == 0 )
//			{
//				//RNOK	( pcReadIf->getUvlc( m_grid_slice_width_in_mbs_minus1[i],    "ScalableSEI:grid_slice_width_in_mbs_minus1" ) );
//				//RNOK	( pcReadIf->getUvlc( m_grid_slice_height_in_mbs_minus1[i],    "ScalableSEI:grid_slice_height_in_mbs_minus1" ) );
//				RNOK	( pcReadIf->getUvlc( m_grid_width_in_mbs_minus1[i],    "ScalableSEI:grid_width_in_mbs_minus1" ) );//JVT-W051
//				RNOK	( pcReadIf->getUvlc( m_grid_height_in_mbs_minus1[i],    "ScalableSEI:grid_height_in_mbs_minus1" ) );//JVT-W051
//			}
//			else if( m_iroi_division_type[i] == 1 )
//			{
//				RNOK	( pcReadIf->getUvlc( m_num_rois_minus1[i],		"ScalableSEI:num_slice_minus1" ) );
//    		// JVT-S054 (ADD) ->
//				if ( m_first_mb_in_roi[i] != NULL )
//					free( m_first_mb_in_roi[i] );
//				m_first_mb_in_roi[i] = (UInt*)malloc( m_num_rois_minus1[i]*sizeof(UInt) );
//				if ( m_roi_width_in_mbs_minus1[i] != NULL )
//					free( m_roi_width_in_mbs_minus1[i] );
//				m_roi_width_in_mbs_minus1[i] = (UInt*)malloc( m_num_rois_minus1[i]*sizeof(UInt) );
//				if ( m_roi_height_in_mbs_minus1[i] != NULL )
//					free( m_roi_height_in_mbs_minus1[i] );
//				m_roi_height_in_mbs_minus1[i] = (UInt*)malloc( m_num_rois_minus1[i]*sizeof(UInt) );
//    		// JVT-S054 (ADD) <-
//				for ( j = 0; j <= m_num_rois_minus1[i]; j++ )
//				{
//					RNOK	( pcReadIf->getUvlc( m_first_mb_in_roi[i][j],				"ScalableSEI: first_mb_in_slice" ) );
//					RNOK	( pcReadIf->getUvlc( m_roi_width_in_mbs_minus1[i][j],		"ScalableSEI:slice_width_in_mbs_minus1" ) );
//					RNOK	( pcReadIf->getUvlc( m_roi_height_in_mbs_minus1[i][j],		"ScalableSEI:slice_height_in_mbs_minus1" ) );
//				}
//			}
//			else if ( m_iroi_division_type[i] == 2 )
//			{
//    		// JVT-S054 (REPLACE) ->
//				RNOK	( pcReadIf->getUvlc( m_num_rois_minus1[i],		"ScalableSEI:num_slice_minus1" ) );
//        /*
//				UInt uiFrameHeightInMb = m_roi_height_in_mbs_minus1[i][j] + 1;
//				UInt uiFrameWidthInMb  = m_roi_width_in_mbs_minus1[i][j] + 1;
//				UInt uiPicSizeInMbs = uiFrameHeightInMb * uiFrameWidthInMb;
//				for ( j = 0; j < uiPicSizeInMbs; j++ )
//				{
//					RNOK	( pcReadIf->getUvlc( m_slice_id[i][j],		"ScalableSEI:slice_id"		   ) );
//				}
//        */
//				UInt uiFrameHeightInMb = m_roi_height_in_mbs_minus1[i][j] + 1;
//				UInt uiFrameWidthInMb  = m_roi_width_in_mbs_minus1[i][j] + 1;
//				UInt uiPicSizeInMbs = uiFrameHeightInMb * uiFrameWidthInMb;
//			  UInt uiReadBits = (UInt)ceil( log( (double) (m_num_rois_minus1[i] + 1) ) / log(2.) );
//				if (uiReadBits == 0)
//					uiReadBits = 1;
//				if ( m_slice_id[i] != NULL )
//					free( m_slice_id[i] );
//				m_slice_id[i] = (UInt*)malloc( uiPicSizeInMbs*sizeof(UInt) );
//				for ( j = 0; j < uiPicSizeInMbs; j++ )
//				{
//					RNOK	( pcReadIf->getCode( m_slice_id[i][j],	uiReadBits,		""	) );
//				}
//    		// JVT-S054 (REPLACE) <-
//			}
//		}
//   //JVT-S036 lsj end
//
//		if( m_layer_dependency_info_present_flag[i] )
//		{
//			RNOK	( pcReadIf->getUvlc( m_num_directly_dependent_layers[i],								""	) );
//// BUG_FIX liuhui{
//			for( j = 0; j < m_num_directly_dependent_layers[i]; j++ )
//			{
//				RNOK  ( pcReadIf->getUvlc( m_directly_dependent_layer_id_delta_minus1[i][j],				""  ) );//JVT-S036 lsj
//			}
//// BUG_FIX liuhui}
//		}
//		else
//		{//JVT-S036 lsj
//			RNOK	( pcReadIf->getUvlc( m_layer_dependency_info_src_layer_id_delta[i],				""  ) );
//			rl = m_layer_id[i] - m_layer_dependency_info_src_layer_id_delta[i];
//			m_num_directly_dependent_layers[i] = m_num_directly_dependent_layers[rl];
//			for( j = 0; j < m_num_directly_dependent_layers[i]; j++ )
//			{
//				m_directly_dependent_layer_id_delta_minus1[i][j] = m_directly_dependent_layer_id_delta_minus1[rl][j];
//			}
//		}
//
//		if( m_init_parameter_sets_info_present_flag[i] )
//		{
//// BUG_FIX liuhui{
//			RNOK    ( pcReadIf->getUvlc( m_num_init_seq_parameter_set_minus1[i],                        ""  ) );
//			for( j = 0; j <= m_num_init_seq_parameter_set_minus1[i]; j++ )
//			{
//			    RNOK	( pcReadIf->getUvlc( m_init_seq_parameter_set_id_delta[i][j],					""	) );
//			}
//			RNOK	( pcReadIf->getUvlc( m_num_init_pic_parameter_set_minus1[i],						""	) );
//			for( j = 0; j <= m_num_init_pic_parameter_set_minus1[i]; j++ )
//			{
//				RNOK	( pcReadIf->getUvlc( m_init_pic_parameter_set_id_delta[i][j],					""	) );
//			}
//// BUG_FIX liuhui}
//		}
//		else
//		{//JVT-S036 lsj
//			RNOK	( pcReadIf->getUvlc( m_init_parameter_sets_info_src_layer_id_delta[i],		"" ) );
//			rl = m_layer_id[i] - m_init_parameter_sets_info_src_layer_id_delta[i];
//			m_num_init_seq_parameter_set_minus1[i] = m_num_init_seq_parameter_set_minus1[rl];
//			for( j = 0; j <= m_num_init_seq_parameter_set_minus1[i]; j++ )
//			{
//			    m_init_seq_parameter_set_id_delta[i][j] = m_init_seq_parameter_set_id_delta[rl][j];
//			}
//			m_num_init_pic_parameter_set_minus1[i] = m_num_init_pic_parameter_set_minus1[rl];
//			for( j = 0; j <= m_num_init_pic_parameter_set_minus1[i]; j++ )
//			{
//				m_init_pic_parameter_set_id_delta[i][j] = m_init_pic_parameter_set_id_delta[rl][j];
//			}
//		}
//		//JVT-W051 {
//		if (m_bitstream_restriction_flag[i])
//		{
//			RNOK	(pcReadIf->getFlag( m_motion_vectors_over_pic_boundaries_flag[i], "") );
//			RNOK	(pcReadIf->getUvlc( m_max_bytes_per_pic_denom[i], "") );
//			RNOK	(pcReadIf->getUvlc( m_max_bits_per_mb_denom[i], "") );
//			RNOK	(pcReadIf->getUvlc( m_log2_max_mv_length_horizontal[i], "l") );
//			RNOK	(pcReadIf->getUvlc( m_log2_max_mv_length_vertical[i], "") );
//			RNOK	(pcReadIf->getUvlc( m_num_reorder_frames[i], ""));
//			RNOK	(pcReadIf->getUvlc( m_max_dec_frame_buffering[i], ""));
//		}
//		//JVT-W051 }
//		//JVT-W046 {
//		if( m_avc_layer_conversion_flag[i] )
//		{
//		  UInt uiTmp;
//      RNOK ( pcReadIf->getUvlc ( m_avc_conversion_type_idc[i],                      "ScalableSEI: m_avc_conversion_type_idc"		   ) );
//		  for ( j=0; j<2; j++ )
//		  {
//		    RNOK	( pcReadIf->getFlag ( m_avc_info_flag[i][j],						"ScalableSEI: m_avc_info_flag"					   ) );
//			  if( m_avc_info_flag[i][j] )
//			  {
//			    m_avc_profile_level_idc[i][j] = 0;
//          RNOK	( pcReadIf->getCode ( uiTmp,		16,		"ScalableSEI:m_avc_profile_level_idc"              ) ); 
//          m_avc_profile_level_idc[i][j] += uiTmp;
//          RNOK	( pcReadIf->getCode ( uiTmp,		8,		"ScalableSEI:m_avc_profile_level_idc"              ) );
//          m_avc_profile_level_idc[i][j]  = m_avc_profile_level_idc[i][j] << 8;
//          m_avc_profile_level_idc[i][j] += uiTmp;
//			    RNOK	( pcReadIf->getCode ( m_avc_avg_bitrate[i][j],		        16,		"ScalableSEI:m_avc_avg_bitrate"                    ) );
//			    RNOK	( pcReadIf->getCode ( m_avc_max_bitrate[i][j],		        16,		"ScalableSEI:m_avc_max_bitrate"                    ) );
//			  } 
//		  }
//		}
//		//JVT-W046 }
//	}	
//
//	//JVT-W051 {
//	if (m_quality_layer_info_present_flag)
//	{
//		UInt uiTmp;
//		RNOK	(pcReadIf->getUvlc( m_ql_num_dId_minus1, ""));
//		for (i=0; i<=m_ql_num_dId_minus1; i++)
//		{
//			RNOK	(pcReadIf->getCode(m_ql_dependency_id[i], 3, ""));
//			RNOK	(pcReadIf->getUvlc(m_ql_num_minus1[i], ""));
//			for (j=0; j<=m_ql_num_minus1[i]; j++)
//			{
//				RNOK	(pcReadIf->getUvlc(m_ql_id[i][j], ""));
//				m_ql_profile_level_idc[i][j] = 0;
//				RNOK	( pcReadIf->getCode( uiTmp, 16, ""));
//				m_ql_profile_level_idc[i][j] += uiTmp;
//				RNOK	( pcReadIf->getCode( uiTmp, 8, ""));
//				m_ql_profile_level_idc[i][j]= m_ql_profile_level_idc[i][j] << 8;
//				m_ql_profile_level_idc[i][j] += uiTmp;
//				//RNOK	(pcReadIf->getCode(uiTmp, 16, ""));
//				//RNOK	(pcReadIf->getCode(uiTmp, 16, ""));
//				//RNOK	(pcReadIf->getCode(m_ql_profile_level_idc[i][j], 24, ""));
//				RNOK	(pcReadIf->getCode(m_ql_avg_bitrate[i][j], 16, ""));
//				RNOK	(pcReadIf->getCode(m_ql_max_bitrate[i][j], 16, ""));
//			}
//		}
//	}
//	//JVT-W051 }
//	//JVT-W053 wxwan
//	if(m_priority_id_setting_flag)
//	{
//		UInt PriorityIdSettingUriIdx = 0;
//		do{
//			UInt uiTemp;
//			RNOK( pcReadIf->getCode( uiTemp,  8,  "" ) );
//			priority_id_setting_uri[PriorityIdSettingUriIdx] = (char) uiTemp;
//			PriorityIdSettingUriIdx++;
//		}while( priority_id_setting_uri[ PriorityIdSettingUriIdx-1 ]  !=  0 );
//	}
//	//JVT-W053 wxwan
//	return Err::m_nOK;
//}
ErrVal
SEI::ScalableSei::write( HeaderSymbolWriteIf *pcWriteIf )
{
  UInt i=0, j=0;
  // JVT-U085 LMI
  RNOK    ( pcWriteIf->writeFlag( m_temporal_id_nesting_flag,                "ScalableSEI: temporal_id_nesting_flag"                         ) );
	RNOK    ( pcWriteIf->writeFlag( m_priority_layer_info_present_flag,	       "ScalableSEI: priority_layer_info_present_flag"                 ) );//JVT-W051 
	RNOK    ( pcWriteIf->writeFlag( m_priority_id_setting_flag,                "ScalableSEI: priority_id_setting_flag"                         ) );//JVT-W053
	ROF     ( m_num_layers_minus1+1 );
	RNOK		( pcWriteIf->writeUvlc( m_num_layers_minus1,											 "ScalableSEI: num_layers_minus1"										             ) );

	for( i = 0; i <= m_num_layers_minus1; i++ )
	{
		if (0 < m_aiNumRoi[m_dependency_id[i]])
		{
			m_sub_pic_layer_flag[i] = true;
			m_roi_id[i]				= m_aaiRoiID[m_dependency_id[i]][0];
		}	

		//JVT-W051 {
		RNOK	( pcWriteIf->writeUvlc( m_layer_id[i],	                                                   "ScalableSEI: layer_id"	                              ) );
		//JVT-W051 }
	//JVT-S036 start
		RNOK	( pcWriteIf->writeCode( m_priority_id[i],					                            6,		       "ScalableSEI: priority_id"										          ) ); 
		RNOK  ( pcWriteIf->writeFlag( m_discardable_flag[i],								                             "ScalableSEI: discardable_flag"											  ) );  		
		RNOK	( pcWriteIf->writeCode( m_dependency_id[i],							                      3,		       "ScalableSEI: dependency_id"											      ) );
    RNOK	( pcWriteIf->writeCode( m_quality_level[i],									                  4,		       "ScalableSEI: quality_id"	   												  ) );
		RNOK	( pcWriteIf->writeCode( m_temporal_level[i],								                  3,		       "ScalableSEI: temporal_id"												      ) );
		RNOK	( pcWriteIf->writeFlag( m_sub_pic_layer_flag[i],										                       "ScalableSEI: sub_pic_layer_flag"										  ) );
		RNOK	( pcWriteIf->writeFlag( m_sub_region_layer_flag[i],									                       "ScalableSEI: sub_region_layer_flag"									  ) );
		RNOK	( pcWriteIf->writeFlag( m_iroi_division_info_present_flag[i],					                     "ScalableSEI: iroi_division_info_present_flag"				  ) ); //JVT-W051
		RNOK	( pcWriteIf->writeFlag( m_profile_level_info_present_flag[i],				                       "ScalableSEI: profile_level_info_present_flag"				  ) );
	//JVT-S036 end
		RNOK	( pcWriteIf->writeFlag( m_bitrate_info_present_flag[i],							                       "ScalableSEI: bitrate_info_present_flag"							  ) );
		RNOK	( pcWriteIf->writeFlag( m_frm_rate_info_present_flag[i],						                       "ScalableSEI: frm_rate_info_present_flag"						  ) );
		RNOK	( pcWriteIf->writeFlag( m_frm_size_info_present_flag[i],						                       "ScalableSEI: frm_size_info_present_flag"						  ) );
		RNOK	( pcWriteIf->writeFlag( m_layer_dependency_info_present_flag[i],		                       "ScalableSEI: layer_dependency_info_present_flag"		  ) );
		//RNOK	( pcWriteIf->writeFlag( m_init_parameter_sets_info_present_flag[i],	                       "ScalableSEI: parameter_sets_info_present_flag"        ) );//SEI changes update
		RNOK	( pcWriteIf->writeFlag( m_parameter_sets_info_present_flag[i],	                           "ScalableSEI: parameter_sets_info_present_flag"        ) );//SEI changes update
		RNOK	( pcWriteIf->writeFlag(	m_bitstream_restriction_info_present_flag[i],					             "ScalableSEI: bitstream_restriction_info_present_flag" ) );  //JVT-051 & JVT-W064
		RNOK	( pcWriteIf->writeFlag( m_exact_interlayer_pred_flag[i],						                       "ScalableSEI: exact_interlayer_pred_flag"              ) );//JVT-S036 
		if( m_sub_pic_layer_flag[i] || m_iroi_division_info_present_flag[i] )
			RNOK	( pcWriteIf->writeFlag( m_exact_sample_value_match_flag[i],						                       "ScalableSEI: exact_sample_value_match_flag"              ) );
		RNOK	( pcWriteIf->writeFlag( m_layer_conversion_flag[i],					   	                           "ScalableSEI: layer_conversion_flag"                   ) );//JVT-W046
		RNOK	( pcWriteIf->writeFlag( m_layer_output_flag[i],											                       "ScalableSEI: layer_output_flag		"                   ) );//JVT-W047 wxwan
		if ( m_profile_level_info_present_flag[i] )
		{
			//JVT-W051 {
			RNOK	( pcWriteIf->writeCode( m_layer_profile_level_idc[i],							           24,		        "ScalableSEI: layer_profile_level_idc"								) );
			//JVT-W051 }
		}

		if ( m_bitrate_info_present_flag[i] )
		{
			RNOK	( pcWriteIf->writeCode( m_avg_bitrate[i],										                 16,		        "ScalableSEI: avg_bitrate"														) );
//JVT-S036 lsj start
			RNOK	( pcWriteIf->writeCode( m_max_bitrate_layer[i],										           16,		        "ScalableSEI: max_bitrate_layer"											) );
			RNOK	( pcWriteIf->writeCode( m_max_bitrate_layer_representation[i],							 16,		        "ScalableSEI: max_bitrate_layer_representation"						) );//JVT-W051
			RNOK	( pcWriteIf->writeCode( m_max_bitrate_calc_window[i],							           16,		        "ScalableSEI: max_bitrate_calc_window"											) );
//JVT-S036 lsj end
		}
		if ( m_frm_rate_info_present_flag[i] )
		{
			RNOK	( pcWriteIf->writeCode( m_constant_frm_rate_idc[i],			                      2,		        "ScalableSEI: constant_frm_bitrate_idc"							) );
			RNOK	( pcWriteIf->writeCode( m_avg_frm_rate[i],									                 16,		        "ScalableSEI: avg_frm_rate"													) );
		}

		if ( m_frm_size_info_present_flag[i] || m_iroi_division_info_present_flag[i] )
		{
			RNOK	( pcWriteIf->writeUvlc( m_frm_width_in_mbs_minus1[i],							                          "ScalableSEI: frm_width_in_mbs_minus1"							) );
			RNOK	( pcWriteIf->writeUvlc( m_frm_height_in_mbs_minus1[i],						                          "ScalableSEI: frm_height_in_mbs_minus1"							) );
		}

		if ( m_sub_region_layer_flag[i] )
		{
			//JVT-W051 {
			RNOK	( pcWriteIf->writeUvlc ( m_base_region_layer_id[i],	                                        "ScalableSEI: base_region_layer_id"	                ) );
			//JVT-W051 }
			RNOK	( pcWriteIf->writeFlag ( m_dynamic_rect_flag[i],									                          "ScalableSEI: dynamic_rect_flag"										) );
			//JVT-W051 {	
			if ( !m_dynamic_rect_flag[i] )
			//JVT-W051 }
			{
				RNOK	( pcWriteIf->writeCode ( m_horizontal_offset[i],					                16,		          "ScalableSEI: horizontal_offset"										) );
				RNOK	( pcWriteIf->writeCode ( m_vertical_offset[i],						                16,		          "ScalableSEI: vertical_offset"											) );
				RNOK	( pcWriteIf->writeCode ( m_region_width[i],								                16,		          "ScalableSEI: region_width"													) );
				RNOK	( pcWriteIf->writeCode ( m_region_height[i],							                16,		          "ScalableSEI: region_height"												) );
			}
		}

		if( m_sub_pic_layer_flag[i] )
		{//JVT-S036 lsj
			//JVT-W051 {
			RNOK	( pcWriteIf->writeUvlc( m_roi_id[i],	                                                      "Scalable: roi_id"                                  ) );
			//JVT-W051 }
		}

	//JVT-S036 lsj start
		if ( m_iroi_division_info_present_flag[i] )//JVT-W051
		{
			//JVT-W051 {
			RNOK	( pcWriteIf->writeFlag( m_iroi_grid_flag[i],				                                        "ScalableSEI: iroi_grid_flag"                       ) );
			//JVT-W051 }
			if( m_iroi_grid_flag[i] )
			{
				RNOK	( pcWriteIf->writeUvlc( m_grid_width_in_mbs_minus1[i],                                    "ScalableSEI: grid_width_in_mbs_minus1"             ) );//JVT-W051
				RNOK	( pcWriteIf->writeUvlc( m_grid_height_in_mbs_minus1[i],                                   "ScalableSEI: grid_height_in_mbs_minus1"            ) );//JVT-W051
			}
			else
			{
				RNOK	( pcWriteIf->writeUvlc( m_num_rois_minus1[i],		                                          "ScalableSEI: num_rois_minus1"                      ) );
				for ( j = 0; j <= m_num_rois_minus1[i]; j++ )
				{
					RNOK	( pcWriteIf->writeUvlc( m_first_mb_in_roi[i][j],				                                "ScalableSEI: first_mb_in_roi"                      ) );
					RNOK	( pcWriteIf->writeUvlc( m_roi_width_in_mbs_minus1[i][j],		                            "ScalableSEI: roi_width_in_mbs_minus1"              ) );
					RNOK	( pcWriteIf->writeUvlc( m_roi_height_in_mbs_minus1[i][j],		                            "ScalableSEI: roi_height_in_mbs_minus1"             ) );
				}
      }
    }
      
	//JVT-S036 lsj end
		if ( m_layer_dependency_info_present_flag[i] )
		{
			RNOK	( pcWriteIf->writeUvlc ( m_num_directly_dependent_layers[i],			                          "ScalableSEI: num_directly_dependent_layers"			  ) );
// BUG_FIX liuhui{
		    for( j = 0; j < m_num_directly_dependent_layers[i]; j++ )
		    {
		      RNOK( pcWriteIf->writeUvlc (m_directly_dependent_layer_id_delta_minus1[i][j],                 "ScalableSEI: directly_dependent_layers_id_delta_minus1"		) );  //JVT-S036 lsj
		    }
// BUG_FIX liuhui}
		}
		else
		{//JVT-S036 lsj
			RNOK( pcWriteIf->writeUvlc(m_layer_dependency_info_src_layer_id_delta[i],	                        "ScalableSEI: layer_dependency_info_src_layer_id_delta"     ) );
		}

		if ( m_parameter_sets_info_present_flag[i] ) 
		{

// BUG_FIX liuhui{
			RNOK	( pcWriteIf->writeUvlc ( m_num_seq_parameter_set_minus1[i],	                                "ScalableSEI: num_seq_parameter_set_minus1"			    ) );
			for( j = 0; j <= m_num_seq_parameter_set_minus1[i]; j++ )
			{
			  RNOK( pcWriteIf->writeUvlc ( m_seq_parameter_set_id_delta[i][j],                                "ScalableSEI: seq_parameter_set_id_delta"				    ) );
			}
      RNOK	( pcWriteIf->writeUvlc ( m_num_subset_seq_parameter_set_minus1[i],	                        "ScalableSEI: num_subset_seq_parameter_set_minus1"	) );
			for( j = 0; j <= m_num_subset_seq_parameter_set_minus1[i]; j++ )
			{
			  RNOK( pcWriteIf->writeUvlc ( m_subset_seq_parameter_set_id_delta[i][j],                         "ScalableSEI: subset_seq_parameter_set_id_delta"		) );
			}
			RNOK( pcWriteIf->writeUvlc ( m_num_pic_parameter_set_minus1[i],	                                  "ScalableSEI: num_pic_parameter_set_minus1"			    ) );
			for( j = 0; j <= m_num_pic_parameter_set_minus1[i]; j++ )
			{
			  RNOK ( pcWriteIf->writeUvlc ( m_pic_parameter_set_id_delta[i][j],                               "ScalableSEI: pic_parameter_set_id_delta"				    ) );
			}
// BUG_FIX liuhui}
		}
		else
		{//JVT-S036 lsj 
			RNOK	(pcWriteIf->writeUvlc( m_parameter_sets_info_src_layer_id_delta[i],                         "ScalableSEI: parameter_sets_info_src_layer_id_delta"	) );
		}
		//JVT-W051 & JVT-W064 {
		if (m_bitstream_restriction_info_present_flag[i])
		{
			RNOK	(pcWriteIf->writeFlag(m_motion_vectors_over_pic_boundaries_flag[i],                         "ScalableSEI: motion_vectors_over_pic_boundaries_flag") );
			RNOK	(pcWriteIf->writeUvlc(m_max_bytes_per_pic_denom[i],                                         "ScalableSEI: max_bytes_per_pic_denom"              ) );
			RNOK	(pcWriteIf->writeUvlc(m_max_bits_per_mb_denom[i],                                           "ScalableSEI: max_bits_per_mb_denom"                ) );
			RNOK	(pcWriteIf->writeUvlc(m_log2_max_mv_length_horizontal[i],                                   "ScalableSEI: log2_max_mv_length_horizontal"        ) );
			RNOK	(pcWriteIf->writeUvlc(m_log2_max_mv_length_vertical[i],                                     "ScalableSEI: log2_max_mv_length_vertical"          ) );
			RNOK	(pcWriteIf->writeUvlc(m_num_reorder_frames[i],                                              "ScalableSEI: num_reorder_frames"                   ) );
			RNOK	(pcWriteIf->writeUvlc(m_max_dec_frame_buffering[i],                                         "ScalableSEI: max_dec_frame_buffering"              ) );
		}
		//JVT-W051 & JVT-W064  }
		//JVT-W046 {
		if( m_layer_conversion_flag[i] )
		{
		  RNOK ( pcWriteIf->writeUvlc ( m_conversion_type_idc[i],                                           "ScalableSEI: m_conversion_type_idc"				        ) );
		  for ( j=0; j<2; j++ )
		  {
		    RNOK	( pcWriteIf->writeFlag ( m_rewriting_info_flag[i][j],						                          "ScalableSEI: m_rewriting_info_flag"								) );
			if( m_rewriting_info_flag[i][j] )
			{
			  RNOK	( pcWriteIf->writeCode( m_rewriting_profile_level_idc[i][j],		             24,		      "ScalableSEI: m_rewriting_profile_level_idc"        ) );
			  RNOK	( pcWriteIf->writeCode( m_rewriting_avg_bitrate[i][j],		                   16,		      "ScalableSEI: m_rewriting_avg_bitrate"              ) );
			  RNOK	( pcWriteIf->writeCode( m_rewriting_max_bitrate[i][j],		                   16,		      "ScalableSEI: m_rewriting_max_bitrate"              ) );
			} 
		  }
		}
		//JVT-W046 }
	}
	//JVT-W051 {
	if (m_priority_layer_info_present_flag)
	{
		RNOK	(pcWriteIf->writeUvlc(m_pr_num_dId_minus1,                                                    "ScalableSEI: pr_num_dId_minus1"                    ) );
		for (i=0; i<=m_pr_num_dId_minus1; i++)
		{
			RNOK	(pcWriteIf->writeCode(m_pr_dependency_id[i],                                    3,          "ScalableSEI: pr_dependency_id"                     ) );
			RNOK	(pcWriteIf->writeUvlc(m_pr_num_minus1[i],                                                   "ScalableSEI: pr_num_minus"                         ) );
			for (j=0; j<=m_pr_num_minus1[i]; j++)
			{
				RNOK	(pcWriteIf->writeUvlc(m_pr_id[i][j],                                                      "ScalableSEI: pr_id"                                ) );
				RNOK	(pcWriteIf->writeCode(m_pr_profile_level_idc[i][j],                          24,          "ScalableSEI: pr_profile_level_idc"                 ) );
				RNOK	(pcWriteIf->writeCode(m_pr_avg_bitrate[i][j],                                16,          "ScalableSEI: pr_avg_bitrate"                       ) );
				RNOK	(pcWriteIf->writeCode(m_pr_max_bitrate[i][j],                                16,          "ScalableSEI: pr_max_bitrate"                       ) );
			}
		}
	}
	//JVT-W051 }
	//JVT-W053 wxwan
	if(m_priority_id_setting_flag)
	{
		UInt PriorityIdSettingUriIdx = 0;
		do
		{
			UInt uiTemp = priority_id_setting_uri[PriorityIdSettingUriIdx];
			RNOK( pcWriteIf->writeCode( uiTemp,                                                   8,          "ScalableSEI: priority_id_setting_uri"              ) );
		}while( priority_id_setting_uri[ PriorityIdSettingUriIdx++ ]  !=  0 );
	}
	//JVT-W053 wxwan

	return Err::m_nOK;
}
ErrVal
SEI::ScalableSei::read ( HeaderSymbolReadIf *pcReadIf )
{
  UInt i, j=0;
  UInt rl;//JVT-S036 lsj 
  // JVT-U085 LMI
  RNOK  ( pcReadIf->getFlag( m_temporal_id_nesting_flag,                               "" ) );
	RNOK	( pcReadIf->getFlag( m_priority_layer_info_present_flag,					             "" ) );//JVT-W051
	RNOK  ( pcReadIf->getFlag( m_priority_id_setting_flag,                               "" ) );//JVT-W053
	RNOK	( pcReadIf->getUvlc( m_num_layers_minus1 ,																     ""	) );

	for ( i = 0; i <= m_num_layers_minus1; i++ )
	{
		//JVT-W051 {
		RNOK	( pcReadIf->getUvlc( m_layer_id[i],	                                         "" ) );
		//JVT-W051 }
	//JVT-S036 lsj start
		RNOK	( pcReadIf->getCode( m_priority_id[i],													        6,	 "" ) );	
		RNOK	( pcReadIf->getFlag( m_discardable_flag[i],													         "" ) );     	
		RNOK	( pcReadIf->getCode( m_dependency_id[i],												        3,	 ""	) );
    RNOK	( pcReadIf->getCode( m_quality_level[i],													      4,	 ""	) );
    RNOK	( pcReadIf->getCode( m_temporal_level[i],													      3,	 ""	) );
		RNOK	( pcReadIf->getFlag( m_sub_pic_layer_flag[i],														     ""	) );
		RNOK	( pcReadIf->getFlag( m_sub_region_layer_flag[i],													   ""	) );
		RNOK	( pcReadIf->getFlag( m_iroi_division_info_present_flag[i],						       "" ) );//JVT-W051
		RNOK	( pcReadIf->getFlag( m_profile_level_info_present_flag[i],								   ""	) );
   //JVT-S036 lsj end	
		RNOK	( pcReadIf->getFlag( m_bitrate_info_present_flag[i],											  ""	) );
		RNOK	( pcReadIf->getFlag( m_frm_rate_info_present_flag[i],											  ""	) );
		RNOK	( pcReadIf->getFlag( m_frm_size_info_present_flag[i],											  ""	) );
		RNOK	( pcReadIf->getFlag( m_layer_dependency_info_present_flag[i],								""	) );
		RNOK	( pcReadIf->getFlag( m_parameter_sets_info_present_flag[i],						      ""	) );
		RNOK	( pcReadIf->getFlag( m_bitstream_restriction_info_present_flag[i],          ""  ) );//JVT-W051
		RNOK	( pcReadIf->getFlag( m_exact_interlayer_pred_flag[i],											  ""  ) );//JVT-S036 lsj
		if( m_sub_pic_layer_flag[ i ] || m_iroi_division_info_present_flag[ i ] )
	    RNOK	( pcReadIf->getFlag( m_exact_sample_value_match_flag[i],									""  ) );
    RNOK	( pcReadIf->getFlag( m_layer_conversion_flag[i],											      ""  ) );//JVT-W046
		RNOK	( pcReadIf->getFlag( m_layer_output_flag[i],											          ""  ) );//JVT-W047 wxwan
		if( m_profile_level_info_present_flag[i] )
		{
			//JVT-W051 {
			UInt uiTmp;
			m_layer_profile_level_idc[i] = 0;
			RNOK	( pcReadIf->getCode( uiTmp, 16, ""));
			m_layer_profile_level_idc[i] += uiTmp;
			RNOK	( pcReadIf->getCode( uiTmp, 8, ""));
			m_layer_profile_level_idc[i] = m_layer_profile_level_idc[i] << 8;
			m_layer_profile_level_idc[i] += uiTmp;
			//JVT-W051 }
		}

		if( m_bitrate_info_present_flag[i] )
		{
			RNOK	( pcReadIf->getCode( m_avg_bitrate[i],														  16,		""	) );
	//JVT-S036 lsj start
			RNOK	( pcReadIf->getCode( m_max_bitrate_layer[i],												16,		""	) );
			RNOK	( pcReadIf->getCode( m_max_bitrate_layer_representation[i],					16,		""	) );//JVT-W051
			RNOK	( pcReadIf->getCode( m_max_bitrate_calc_window[i],									16,		""	) );
	//JVT-S036 lsj end
		}

		if( m_frm_rate_info_present_flag[i] )
		{
			RNOK	( pcReadIf->getCode( m_constant_frm_rate_idc[i],									   2,		""	) );
			RNOK	( pcReadIf->getCode( m_avg_frm_rate[i],														  16,		""	) );
		}

		if( m_frm_size_info_present_flag[i] ||  m_iroi_division_info_present_flag[i] )
		{
			RNOK	( pcReadIf->getUvlc( m_frm_width_in_mbs_minus1[i],											  ""	) );
			RNOK	( pcReadIf->getUvlc( m_frm_height_in_mbs_minus1[i],											  ""	) );
		}

		if( m_sub_region_layer_flag[i] )
		{
			//JVT-W051 { 
			RNOK	( pcReadIf->getUvlc( m_base_region_layer_id[i],                          ""  ) );
			//JVT-W051 }
			RNOK	( pcReadIf->getFlag( m_dynamic_rect_flag[i],														 ""	 ) );
			//JVT-W051 {
			if ( !m_dynamic_rect_flag[i] )
			//JVT-W051 }
			{
				RNOK( pcReadIf->getCode( m_horizontal_offset[i],											  16,  ""	 ) );
				RNOK( pcReadIf->getCode( m_vertical_offset[i],												  16,	 ""	 ) );
				RNOK( pcReadIf->getCode( m_region_width[i],														  16,	 ""	 ) );
				RNOK( pcReadIf->getCode( m_region_height[i],													  16,	 ""	 ) );
			}
		}

	//JVT-S036 lsj start
		if( m_sub_pic_layer_flag[i] )
		{
			//JVT-W051 {
			RNOK	( pcReadIf->getUvlc( m_roi_id[i],                                       ""   ) );
			//JVT-W051 }
		}
		if( m_iroi_division_info_present_flag[i] )//JVT-W051
		{
			//JVT-W051 {
			RNOK	( pcReadIf->getFlag( m_iroi_grid_flag[i],			                   "ScalableSEI:iroi_grid_flag" ) );
			//JVT-W051 }
			if( m_iroi_grid_flag[i] )
			{
				RNOK	( pcReadIf->getUvlc( m_grid_width_in_mbs_minus1[i],            "ScalableSEI:grid_width_in_mbs_minus1"  ) );//JVT-W051
				RNOK	( pcReadIf->getUvlc( m_grid_height_in_mbs_minus1[i],           "ScalableSEI:grid_height_in_mbs_minus1" ) );//JVT-W051
			}
			else
			{
				RNOK	( pcReadIf->getUvlc( m_num_rois_minus1[i],		                 "ScalableSEI:num_rois_minus1"           ) );
    		// JVT-S054 (ADD) ->
				if ( m_first_mb_in_roi[i] != NULL )
					free( m_first_mb_in_roi[i] );
				m_first_mb_in_roi[i] = (UInt*)malloc( m_num_rois_minus1[i]*sizeof(UInt) );
				if ( m_roi_width_in_mbs_minus1[i] != NULL )
					free( m_roi_width_in_mbs_minus1[i] );
				m_roi_width_in_mbs_minus1[i] = (UInt*)malloc( m_num_rois_minus1[i]*sizeof(UInt) );
				if ( m_roi_height_in_mbs_minus1[i] != NULL )
					free( m_roi_height_in_mbs_minus1[i] );
				m_roi_height_in_mbs_minus1[i] = (UInt*)malloc( m_num_rois_minus1[i]*sizeof(UInt) );
    		// JVT-S054 (ADD) <-
				for ( j = 0; j <= m_num_rois_minus1[i]; j++ )
				{
					RNOK	( pcReadIf->getUvlc( m_first_mb_in_roi[i][j],				        "ScalableSEI:first_mb_in_slice" ) );
					RNOK	( pcReadIf->getUvlc( m_roi_width_in_mbs_minus1[i][j],		    "ScalableSEI:slice_width_in_mbs_minus1" ) );
					RNOK	( pcReadIf->getUvlc( m_roi_height_in_mbs_minus1[i][j],		  "ScalableSEI:slice_height_in_mbs_minus1" ) );
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
			RNOK	( pcReadIf->getUvlc( m_layer_dependency_info_src_layer_id_delta[i],			""  ) );
			rl = m_layer_id[i] - m_layer_dependency_info_src_layer_id_delta[i];
			m_num_directly_dependent_layers[i] = m_num_directly_dependent_layers[rl];
			for( j = 0; j < m_num_directly_dependent_layers[i]; j++ )
			{
				m_directly_dependent_layer_id_delta_minus1[i][j] = m_directly_dependent_layer_id_delta_minus1[rl][j];
			}
		}

		if( m_parameter_sets_info_present_flag[i] )
		{
// BUG_FIX liuhui{
			RNOK    ( pcReadIf->getUvlc( m_num_seq_parameter_set_minus1[i],              ""  ) );
			for( j = 0; j <= m_num_seq_parameter_set_minus1[i]; j++ )
			{
			  RNOK	( pcReadIf->getUvlc( m_seq_parameter_set_id_delta[i][j],					     ""	) );
			}
			RNOK    ( pcReadIf->getUvlc( m_num_seq_parameter_set_minus1[i],              ""  ) );
			for( j = 0; j <= m_num_subset_seq_parameter_set_minus1[i]; j++ )
			{
			  RNOK	( pcReadIf->getUvlc( m_subset_seq_parameter_set_id_delta[i][j],				""	) );
			}
			RNOK	( pcReadIf->getUvlc( m_num_pic_parameter_set_minus1[i],						     ""	 ) );
			for( j = 0; j <= m_num_pic_parameter_set_minus1[i]; j++ )
			{
				RNOK	( pcReadIf->getUvlc( m_pic_parameter_set_id_delta[i][j],					     ""	) );
			}
// BUG_FIX liuhui}
		}
		else
		{//JVT-S036 lsj 
			RNOK	( pcReadIf->getUvlc( m_parameter_sets_info_src_layer_id_delta[i],     ""  ) );
			rl = m_layer_id[i] - m_parameter_sets_info_src_layer_id_delta[i];
			m_num_seq_parameter_set_minus1[i] = m_num_seq_parameter_set_minus1[rl];
			for( j = 0; j <= m_num_seq_parameter_set_minus1[i]; j++ )
			{
			  m_seq_parameter_set_id_delta[i][j] = m_seq_parameter_set_id_delta[rl][j];
			}
      m_num_subset_seq_parameter_set_minus1[i] = m_num_subset_seq_parameter_set_minus1[rl];
			for( j = 0; j <= m_num_seq_parameter_set_minus1[i]; j++ )
			{
			  m_subset_seq_parameter_set_id_delta[i][j] = m_subset_seq_parameter_set_id_delta[rl][j];
			}
			m_num_pic_parameter_set_minus1[i] = m_num_pic_parameter_set_minus1[rl];
			for( j = 0; j <= m_num_pic_parameter_set_minus1[i]; j++ )
			{
				m_pic_parameter_set_id_delta[i][j] = m_pic_parameter_set_id_delta[rl][j];
			}
		}
		//JVT-W051 {
		if (m_bitstream_restriction_info_present_flag[i])
		{
			RNOK	(pcReadIf->getFlag( m_motion_vectors_over_pic_boundaries_flag[i],   ""   ) );
			RNOK	(pcReadIf->getUvlc( m_max_bytes_per_pic_denom[i],                   ""   ) );
			RNOK	(pcReadIf->getUvlc( m_max_bits_per_mb_denom[i],                     ""   ) );
			RNOK	(pcReadIf->getUvlc( m_log2_max_mv_length_horizontal[i],             ""   ) );
			RNOK	(pcReadIf->getUvlc( m_log2_max_mv_length_vertical[i],               ""   ) );
			RNOK	(pcReadIf->getUvlc( m_num_reorder_frames[i],                        ""   ) );
			RNOK	(pcReadIf->getUvlc( m_max_dec_frame_buffering[i],                   ""   ) );
		}
		//JVT-W051 }
		//JVT-W046 {
		if( m_layer_conversion_flag[i] )
		{
		  UInt uiTmp;
      RNOK ( pcReadIf->getUvlc ( m_conversion_type_idc[i],                            ""		     ) );
		  for ( j=0; j<2; j++ )
		  {
		    RNOK	( pcReadIf->getFlag ( m_rewriting_info_flag[i][j],						          ""			   ) );
			  if( m_rewriting_info_flag[i][j] )
			  {
			    m_rewriting_profile_level_idc[i][j] = 0;
          RNOK	( pcReadIf->getCode ( uiTmp,		16,		""              ) ); 
          m_rewriting_profile_level_idc[i][j] += uiTmp;
          RNOK	( pcReadIf->getCode ( uiTmp,		8,		""              ) );
          m_rewriting_profile_level_idc[i][j]  = m_rewriting_profile_level_idc[i][j] << 8;
          m_rewriting_profile_level_idc[i][j] += uiTmp;
			    RNOK	( pcReadIf->getCode ( m_rewriting_avg_bitrate[i][j],		        16,		""         ) );
			    RNOK	( pcReadIf->getCode ( m_rewriting_max_bitrate[i][j],		        16,		""         ) );
			  } 
		  }
		}
		//JVT-W046 }
	}	

	//JVT-W051 {
	if (m_priority_layer_info_present_flag)
	{
		UInt uiTmp;
		RNOK	(pcReadIf->getUvlc( m_pr_num_dId_minus1, ""));
		for (i=0; i<=m_pr_num_dId_minus1; i++)
		{
			RNOK	(pcReadIf->getCode(m_pr_dependency_id[i], 3, ""));
			RNOK	(pcReadIf->getUvlc(m_pr_num_minus1[i], ""));
			for (j=0; j<=m_pr_num_minus1[i]; j++)
			{
				RNOK	(pcReadIf->getUvlc(m_pr_id[i][j], ""));
				m_pr_profile_level_idc[i][j] = 0;
				RNOK	( pcReadIf->getCode( uiTmp, 16, ""));
				m_pr_profile_level_idc[i][j] += uiTmp;
				RNOK	( pcReadIf->getCode( uiTmp, 8, ""));
				m_pr_profile_level_idc[i][j]= m_pr_profile_level_idc[i][j] << 8;
				m_pr_profile_level_idc[i][j] += uiTmp;
				RNOK	(pcReadIf->getCode(m_pr_avg_bitrate[i][j], 16, ""));
				RNOK	(pcReadIf->getCode(m_pr_max_bitrate[i][j], 16, ""));
			}
		}
	}
	//JVT-W051 }
	//JVT-W053 wxwan
	if(m_priority_id_setting_flag)
	{
		UInt PriorityIdSettingUriIdx = 0;
		do{
			UInt uiTemp;
			RNOK( pcReadIf->getCode( uiTemp,  8,  "" ) );
			priority_id_setting_uri[PriorityIdSettingUriIdx] = (char) uiTemp;
			PriorityIdSettingUriIdx++;
		}while( priority_id_setting_uri[ PriorityIdSettingUriIdx-1 ]  !=  0 );
	}
	//JVT-W053 wxwan
	return Err::m_nOK;
}
//SEI changes update }
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



//////////////////////////////////////////////////////////////////////////
// 
//      MOTION     S E I  FOR  ROI
//
//////////////////////////////////////////////////////////////////////////

SEI::MotionSEI::MotionSEI     ()
 : SEIMessage                     ( MOTION_SEI ),
 m_num_slice_groups_in_set_minus1(0),
 m_exact_sample_value_match_flag(true),
 m_pan_scan_rect_flag(false)
{
}

SEI::MotionSEI::~MotionSEI()
{
}

ErrVal
SEI::MotionSEI::create( MotionSEI*& rpcSeiMessage )
{
  rpcSeiMessage = new MotionSEI();
  ROT( NULL == rpcSeiMessage )
  return Err::m_nOK;
}

ErrVal
SEI::MotionSEI::write( HeaderSymbolWriteIf* pcWriteIf )
{

  RNOK  ( pcWriteIf->writeUvlc( m_num_slice_groups_in_set_minus1,               "Motion Constrainted SEI: Num_slice_groups_in_set_minus1"   ) );

  for(UInt i = 0; i <= m_num_slice_groups_in_set_minus1; i++)
  {    
    RNOK  ( pcWriteIf->writeUvlc( m_slice_group_id[ i ],               "Motion Constrainted SEI: slice_group_id[ i ]"   ) );
  }
  
    
  RNOK  ( pcWriteIf->writeFlag(m_exact_sample_value_match_flag           ,     "Motion Constrainted SEI: exact_sample_value_match_flag"            ) );
  RNOK  ( pcWriteIf->writeFlag(m_pan_scan_rect_flag                      ,     "Motion Constrainted SEI: frm_rate_info_present_flag"           ) );

  return Err::m_nOK;
}

ErrVal
SEI::MotionSEI::read ( HeaderSymbolReadIf* pcReadIf )
{
  RNOK  ( pcReadIf->getUvlc( m_num_slice_groups_in_set_minus1,               "Motion Constrainted SEI: Num_slice_groups_in_set_minus1"   ) );

  for(UInt i = 0; i <= m_num_slice_groups_in_set_minus1; i++)
  {    
    RNOK  ( pcReadIf->getUvlc( m_slice_group_id[ i ],               "Motion Constrainted SEI: slice_group_id[ i ]"   ) );
  }
    
  RNOK  ( pcReadIf->getFlag(m_exact_sample_value_match_flag           ,     "Motion Constrainted SEI: exact_sample_value_match_flag"            ) );
  RNOK  ( pcReadIf->getFlag(m_pan_scan_rect_flag                      ,     "Motion Constrainted SEI: frm_rate_info_present_flag"           ) );

  assert(m_exact_sample_value_match_flag==true);
  assert(m_pan_scan_rect_flag ==false);

  return Err::m_nOK;
}

ErrVal        
SEI::MotionSEI::setSliceGroupId(UInt id)
{
  m_slice_group_id[0] = id;    
  return Err::m_nOK;
};

//SEI changes update {
////{{Quality level estimation and modified truncation- JVTO044 and m12007
////France Telecom R&D-(nathalie.cammas@francetelecom.com)
////////////////////////////////////////////////////////////////////////////
//// 
////      QUALITY LEVEL     S E I
////
////////////////////////////////////////////////////////////////////////////
//
//SEI::QualityLevelSEI::QualityLevelSEI     ()
// : SEIMessage                     ( QUALITYLEVEL_SEI ),
// m_uiNumLevels         ( 0 ),
// m_uiDependencyId      ( 0 )
//{
//  ::memset( m_auiQualityLevel,  0x00, MAX_NUM_RD_LEVELS*sizeof(UInt) );
////  ::memset( m_auiDeltaBytesRateOfLevel, 0x00, MAX_NUM_RD_LEVELS*sizeof(UInt) ); //JVT-W137
//}
//
//
//SEI::QualityLevelSEI::~QualityLevelSEI()
//{
//}
//
//
//ErrVal
//SEI::QualityLevelSEI::create( QualityLevelSEI*& rpcSeiMessage )
//{
//  rpcSeiMessage = new QualityLevelSEI();
//  ROT( NULL == rpcSeiMessage )
//  return Err::m_nOK;
//}
//
//
//ErrVal
//SEI::QualityLevelSEI::write( HeaderSymbolWriteIf* pcWriteIf )
//{
//  RNOK  ( pcWriteIf->writeCode( m_uiDependencyId, 3,"QualityLevelSEI: DependencyId"   ) );
//  RNOK  ( pcWriteIf->writeCode( m_uiNumLevels, 4,"QualityLevelSEI: NumLevels"   ) );
//  for(UInt ui = 0; ui < m_uiNumLevels; ui++)
//  {
//	RNOK  ( pcWriteIf->writeCode( m_auiQualityLevel[ui], 8,"QualityLevelSEI: QualityLevel"   ) );
//  //JVT-W137: remove m_auiDeltaBytesRateOfLevel
//	//RNOK  ( pcWriteIf->writeUvlc( m_auiDeltaBytesRateOfLevel[ui],"QualityLevelSEI: DeDeltaBytesRateOfLevellta"   ) );//~JVT-W137
//  }
//
//  return Err::m_nOK;
//}
//
//
//ErrVal
//SEI::QualityLevelSEI::read ( HeaderSymbolReadIf* pcReadIf )
//{
//  RNOK  ( pcReadIf->getCode( m_uiDependencyId, 3,"QualityLevelSEI: DependencyId"   ) );
//  RNOK  ( pcReadIf->getCode( m_uiNumLevels, 4,"QualityLevelSEI: NumLevels"   ) );
//  for(UInt ui = 0; ui < m_uiNumLevels; ui++)
//  {
//	RNOK  ( pcReadIf->getCode( m_auiQualityLevel[ui], 8,"QualityLevelSEI: QualityLevel"   ) );
//  //JVT-W137
//	//RNOK  ( pcReadIf->getUvlc( m_auiDeltaBytesRateOfLevel[ui],"QualityLevelSEI: DeltaBytesRateOfLevel"   ) );//~JVT-W137
//  }
//  return Err::m_nOK;
//}
//
////}}Quality level estimation and modified truncation- JVTO044 and m12007
//{{Quality level estimation and modified truncation- JVTO044 and m12007
//France Telecom R&D-(nathalie.cammas@francetelecom.com)
//////////////////////////////////////////////////////////////////////////
// 
//      PRIORITY LAYER INFORMATION  S E I
//
//////////////////////////////////////////////////////////////////////////

SEI::PriorityLevelSEI::PriorityLevelSEI     ()
 : SEIMessage                     ( PRIORITYLEVEL_SEI ),
 m_uiNumPriorityIds         ( 0 ),
 m_uiPrDependencyId      ( 0 )
{
  ::memset( m_auiAltPriorityId,  0x00, MAX_NUM_RD_LEVELS*sizeof(UInt) );
}


SEI::PriorityLevelSEI::~PriorityLevelSEI()
{
}


ErrVal
SEI::PriorityLevelSEI::create( PriorityLevelSEI*& rpcSeiMessage )
{
  rpcSeiMessage = new PriorityLevelSEI();
  ROT( NULL == rpcSeiMessage )
  return Err::m_nOK;
}


ErrVal
SEI::PriorityLevelSEI::write( HeaderSymbolWriteIf* pcWriteIf )
{
  RNOK  ( pcWriteIf->writeCode( m_uiPrDependencyId, 3,"PriorityLevelSEI: DependencyId"   ) );
  RNOK  ( pcWriteIf->writeCode( m_uiNumPriorityIds, 4,"PriorityLevelSEI: NumPriorityIds"   ) );
  for(UInt ui = 0; ui < m_uiNumPriorityIds; ui++)
  {
	RNOK  ( pcWriteIf->writeCode( m_auiAltPriorityId[ui], 6,"PriorityLevelSEI: AltPriorityId"   ) );
  }

  return Err::m_nOK;
}


ErrVal
SEI::PriorityLevelSEI::read ( HeaderSymbolReadIf* pcReadIf )
{
  RNOK  ( pcReadIf->getCode( m_uiPrDependencyId, 3,"PriorityLevelSEI: DependencyId"   ) );
  RNOK  ( pcReadIf->getCode( m_uiNumPriorityIds, 4,"PriorityLevelSEI: NumLevels"   ) );
  for(UInt ui = 0; ui < m_uiNumPriorityIds; ui++)
  {
	RNOK  ( pcReadIf->getCode( m_auiAltPriorityId[ui], 6,"PriorityLevelSEI: AltPriorityId"   ) );
  }
  return Err::m_nOK;
}

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

//ErrVal
//SEI::NonRequiredSei::write( HeaderSymbolWriteIf* pcWriteIf )
//{
//	RNOK	(pcWriteIf->writeUvlc( m_uiNumInfoEntriesMinus1,	"NonRequiredSEI: NumInfoEntriesMinus1"	));
//	for( UInt uiLayer = 0; uiLayer <= m_uiNumInfoEntriesMinus1; uiLayer++)
//	{
//		RNOK(pcWriteIf->writeCode( m_uiEntryDependencyId[uiLayer],	3,	"NonRequiredSEI: EntryDependencyId"		));
//		RNOK(pcWriteIf->writeUvlc( m_uiNumNonRequiredPicsMinus1[uiLayer],	"NonRequiredSEI: NumNonRequiredPicsMinus1"	));
//		for( UInt NonRequiredLayer = 0; NonRequiredLayer <= m_uiNumNonRequiredPicsMinus1[uiLayer]; NonRequiredLayer++)
//		{
//			RNOK(pcWriteIf->writeCode( m_uiNonRequiredPicDependencyId[uiLayer][NonRequiredLayer],	3,	"NonRequiredSEI: NonRequiredPicDependencyId"));
//			RNOK(pcWriteIf->writeCode( m_uiNonRequiredPicQulityLevel[uiLayer][NonRequiredLayer],	2,	"NonRequiredSEI: NonRequiredPicQulityLevel"	));
//			RNOK(pcWriteIf->writeCode( m_uiNonRequiredPicFragmentOrder[uiLayer][NonRequiredLayer],	2,	"NonRequiredSEI: NonRequiredFragmentOrder"	));
//		}
//	}
//	return Err::m_nOK;
//}
//
//ErrVal
//SEI::NonRequiredSei::read( HeaderSymbolReadIf* pcReadIf )
//{
//	RNOK	(pcReadIf->getUvlc( m_uiNumInfoEntriesMinus1,	"NonRequiredSEI: NumInfoEntriesMinus1"	));
//	for( UInt uiLayer = 0; uiLayer <= m_uiNumInfoEntriesMinus1; uiLayer++)
//	{
//		RNOK(pcReadIf->getCode( m_uiEntryDependencyId[uiLayer],	3,	"NonRequiredSEI: EntryDependencyId"		));
//		RNOK(pcReadIf->getUvlc( m_uiNumNonRequiredPicsMinus1[uiLayer],	"NonRequiredSEI: NumNonRequiredPicsMinus1"	));
//		for( UInt NonRequiredLayer = 0; NonRequiredLayer <= m_uiNumNonRequiredPicsMinus1[uiLayer]; NonRequiredLayer++)
//		{
//			RNOK(pcReadIf->getCode( m_uiNonRequiredPicDependencyId[uiLayer][NonRequiredLayer],	3,	"NonRequiredSEI: NonRequiredPicDependencyId"));
//			RNOK(pcReadIf->getCode( m_uiNonRequiredPicQulityLevel[uiLayer][NonRequiredLayer],	2,	"NonRequiredSEI: NonRequiredPicQulityLevel"	));
//			RNOK(pcReadIf->getCode( m_uiNonRequiredPicFragmentOrder[uiLayer][NonRequiredLayer],	2,	"NonRequiredSEI: NonRequiredFragmentOrder"	));
//		}
//	}
//	return Err::m_nOK;
//}
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
			RNOK(pcWriteIf->writeCode( m_uiNonRequiredPicQulityLevel[uiLayer][NonRequiredLayer],	4,	"NonRequiredSEI: NonRequiredPicQulityLevel"	));
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
			RNOK(pcReadIf->getCode( m_uiNonRequiredPicQulityLevel[uiLayer][NonRequiredLayer],	4,	"NonRequiredSEI: NonRequiredPicQulityLevel"	));
		}
	}
	return Err::m_nOK;
}
//SEI changes update }
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
  //TMM_FIX
ErrVal
SEI::ScalableSeiLayersNotPresent::destroy()
{
	delete this ;
	return Err::m_nOK;
}
  //TMM_FIX
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
// JVT-T073 {
//////////////////////////////////////////////////////////////////////////
//
//			SCALABLE NESTING SEI
//
//////////////////////////////////////////////////////////////////////////
ErrVal
SEI::ScalableNestingSei::create( ScalableNestingSei* &rpcSeiMessage )
{
    rpcSeiMessage = new ScalableNestingSei();
	ROT( NULL == rpcSeiMessage );
	return Err::m_nOK;
}
ErrVal
SEI::ScalableNestingSei::destroy()
{
    delete this;
	return Err::m_nOK;
}

ErrVal
SEI::ScalableNestingSei::write( HeaderSymbolWriteIf *pcWriteIf )
{
    UInt uiStartBits  = pcWriteIf->getNumberOfWrittenBits();
	UInt uiPayloadSize = 0;
	UInt uiIndex;
	RNOK( pcWriteIf->writeFlag( m_bAllPicturesInAuFlag, " ScalableNestingSei:AllPicturesInAuFlag " ) );
	if( m_bAllPicturesInAuFlag == 0 )
	{
		RNOK( pcWriteIf->writeUvlc( m_uiNumPictures, "ScalableNestingSei:NumPictures" ) );
		ROT( m_uiNumPictures == 0 );
		for( uiIndex = 0; uiIndex < m_uiNumPictures; uiIndex++ )
		{
			RNOK( pcWriteIf->writeCode( m_auiDependencyId[uiIndex],3, " ScalableNestingSei:uiDependencyId " ) );
			RNOK( pcWriteIf->writeCode( m_auiQualityLevel[uiIndex],4, " ScalableNestingSei:uiQualityLevel " ) );
		}	    
		// JVT-V068 HRD {
    	RNOK( pcWriteIf->writeCode( m_uiTemporalLevel,3, " ScalableNestingSei:uiTemporalLevel " ) );
		// JVT-V068 HRD }
	}
	UInt uiBits = pcWriteIf->getNumberOfWrittenBits()-uiStartBits;
	UInt uiBitsMod8 = uiBits%8;
	if( uiBitsMod8 )
	{
		RNOK( pcWriteIf->writeCode(0, 8-uiBitsMod8, "SeiNestingZeroBits" ) );
	}
	uiBits = pcWriteIf->getNumberOfWrittenBits();
	uiPayloadSize = (uiBits+7)/8;

	return Err::m_nOK;
}

ErrVal
SEI::ScalableNestingSei::read( HeaderSymbolReadIf *pcReadIf )
{
	RNOK( pcReadIf->getFlag( m_bAllPicturesInAuFlag, " ScalableNestingSei:AllPicturesInAuFlag " ) );
	if( m_bAllPicturesInAuFlag == 0 )
	{
		RNOK( pcReadIf->getUvlc( m_uiNumPictures, "ScalableNestingSei:NumPictures" ) );
		ROT( m_uiNumPictures == 0 );
		for( UInt uiIndex = 0; uiIndex < m_uiNumPictures; uiIndex++ )
		{
			RNOK( pcReadIf->getCode( m_auiDependencyId[uiIndex],3, " ScalableNestingSei:uiDependencyId " ) );
			RNOK( pcReadIf->getCode( m_auiQualityLevel[uiIndex],4, " ScalableNestingSei:uiQualityLevel " ) );
		}
    // JVT-V068 HRD {
    RNOK( pcReadIf->getCode( m_uiTemporalLevel,3, " ScalableNestingSei:uiTemporalLevel " ) );
    // JVT-V068 HRD }
	}
	RNOK( pcReadIf->readZeroByteAlign() ); //nesting_zero_bit

	//Read the following SEI message
	UInt uiType, uiPayloadSize;
	while(1)
	{
		RNOK( pcReadIf->getCode( uiType, 8, " ScalableNestingSei:SEI type" ) );
		if( uiType != 0xff )
			break;
	}
	while(1)
	{
		RNOK( pcReadIf->getCode( uiPayloadSize, 8, " ScalableNestingSei:SEI payloadSize" ) );
		if( uiPayloadSize != 0xff )
			break;
	}
	switch( uiType )
	{
	case SCENE_INFO_SEI:
		{
			SEI::SceneInfoSei* pcSceneInfoSei;
			RNOK( SEI::SceneInfoSei::create(pcSceneInfoSei) );
			RNOK( pcSceneInfoSei->read(pcReadIf) );
			//add some control

			RNOK( pcSceneInfoSei->destroy() );  
			break;
		}
  //JVT-W062 {
  case SEI::TL0_DEP_REP_IDX_SEI:
    {
      SEI::Tl0DepRepIdxSei* pcTl0DepRepIdxSei;
  		RNOK( SEI::Tl0DepRepIdxSei::create(pcTl0DepRepIdxSei) );
	    RNOK( pcTl0DepRepIdxSei->read(pcReadIf) );
	    RNOK( pcTl0DepRepIdxSei->destroy() );  
      break;
    }
//JVT-W062 }

	//more case can be added here
	default:
// JVT-V068 {
    {
      for (UInt ui=0; ui<uiPayloadSize; ui++)
      {
        UInt uiDummy;
        pcReadIf->getCode(uiDummy, 8, "SEI: Byte ignored" );
      }
    }
// JVT-V068 }
		break;
	}

	return Err::m_nOK;
}

//Scene Info, simplified
ErrVal
SEI::SceneInfoSei::create( SceneInfoSei* &rpcSeiMessage )
{
    rpcSeiMessage = new SceneInfoSei();
	ROT( NULL == rpcSeiMessage );
	return Err::m_nOK;
}

ErrVal
SEI::SceneInfoSei::destroy()
{
    delete this;
	return Err::m_nOK;
}

ErrVal
SEI::SceneInfoSei::write( HeaderSymbolWriteIf *pcWriteIf )
{
	UInt uiStart = pcWriteIf->getNumberOfWrittenBits();
	UInt uiPayloadSize = 0;
	Bool bSceneInfoPresentFlag = getSceneInfoPresentFlag();
	RNOK( pcWriteIf->writeFlag( bSceneInfoPresentFlag, "SceneInfo: SceneInfoPresentFlag" ) );
	if( bSceneInfoPresentFlag )
	{
		RNOK( pcWriteIf->writeUvlc( getSceneId(), "SceneInfo: SceneId" ) );
		RNOK( pcWriteIf->writeUvlc( getSceneTransitionType(), "SceneInfo: SceneTransitionType" ) );
		if( getSceneTransitionType() > 3 )
		{
			RNOK( pcWriteIf->writeUvlc( getSecondSceneId(), "SceneInfo: SecondSceneId" ) );
		}
	}
	uiPayloadSize = ( pcWriteIf->getNumberOfWrittenBits() - uiStart + 7 )/8;

	return Err::m_nOK;
}

ErrVal
SEI::SceneInfoSei::read( HeaderSymbolReadIf *pcReadIf )
{
	RNOK( pcReadIf->getFlag( m_bSceneInfoPresentFlag, "SceneInfo: bSceneInfoPresentFlag" ) );
	if( m_bSceneInfoPresentFlag )
	{
		RNOK( pcReadIf->getUvlc( m_uiSceneId,  "SceneInfo: SceneId" ) );
		RNOK( pcReadIf->getUvlc( m_uiSceneTransitionType,  "SceneInfo: SceneTransitionType " ) );
		if( m_uiSceneTransitionType > 3 )
		{
			RNOK( pcReadIf->getUvlc( m_uiSecondSceneId, "SceneInfo: SecondSceneId" ) );
		}
	}
	return Err::m_nOK;
}
// JVT-T073 }


// JVT-W052 wxwan
//////////////////////////////////////////////////////////////////////////
//
//			Quality layer integrity check SEI 
//
//////////////////////////////////////////////////////////////////////////
ErrVal
SEI::IntegrityCheckSEI::create( IntegrityCheckSEI* &rpcSeiMessage )
{
	rpcSeiMessage = new IntegrityCheckSEI();
	ROT( NULL == rpcSeiMessage );
	return Err::m_nOK;
}
ErrVal
SEI::IntegrityCheckSEI::destroy()
{
	delete this;
	return Err::m_nOK;
}
ErrVal
SEI::IntegrityCheckSEI::write( HeaderSymbolWriteIf *pcWriteIf )
{
	RNOK( pcWriteIf->writeUvlc( m_uinuminfoentriesminus1, " IntegrityCheckSEI:num_info_entries_minus1 " ) );
	for( UInt i = 0; i<= m_uinuminfoentriesminus1; i++ )
	{
		RNOK( pcWriteIf->writeCode( m_uientrydependency_id[i],3,  " IntegrityCheckSEI:entry_dependency_id[ i ] " ) );
		RNOK( pcWriteIf->writeCode( m_uiquality_layer_crc [i],16, " IntegrityCheckSEI:quality_layer_crc  [ i ] " ) );
	}
	return Err::m_nOK;
}
ErrVal
SEI::IntegrityCheckSEI::read( HeaderSymbolReadIf* pcReadIf )
{
	RNOK( pcReadIf->getUvlc( m_uinuminfoentriesminus1, " IntegrityCheckSEI:num_info_entries_minus1 ") );
	for( UInt i = 0; i<= m_uinuminfoentriesminus1; i++ )
	{
		RNOK( pcReadIf->getCode( m_uientrydependency_id[i],3,  " IntegrityCheckSEI:entry_dependency_id[ i ] " ) );
		RNOK( pcReadIf->getCode( m_uiquality_layer_crc [i],16, " IntegrityCheckSEI:quality_layer_crc  [ i ] " ) );
	}
	return Err::m_nOK;
}
// JVT-W052 wxwan

// PR slice component info
ErrVal
SEI::PRComponentInfoSei::create( PRComponentInfoSei* &rpcSeiMessage )
{
  rpcSeiMessage = new PRComponentInfoSei();
  ROT( NULL == rpcSeiMessage );
  return Err::m_nOK;
}

ErrVal
SEI::PRComponentInfoSei::destroy()
{
  delete this;
  return Err::m_nOK;
}

ErrVal
SEI::PRComponentInfoSei::write( HeaderSymbolWriteIf *pcWriteIf )
{
  UInt ui_i, ui_j, ui_k;
  UInt uiStart = pcWriteIf->getNumberOfWrittenBits();
  UInt uiPayloadSize = 0;
  RNOK( pcWriteIf->writeUvlc( m_uiNumDependencyIdMinus1, "PRComponentInfo: NumDependencyIdMinus1" ) );
  for( ui_i=0; ui_i<=m_uiNumDependencyIdMinus1; ui_i++ )
  {
    RNOK( pcWriteIf->writeCode( m_uiPrDependencyId[ui_i], 3, "PRComponentInfo: PrDependencyId" ) );
    RNOK( pcWriteIf->writeUvlc( m_uiNumQualityLevelMinus1[ui_i], "PRComponentInfo: NumQualityLevelsMinus1" ) );
    for( ui_j=0; ui_j<=m_uiNumQualityLevelMinus1[ui_i]; ui_j++ )
    {
      RNOK( pcWriteIf->writeCode( m_uiPrQualityLevel[ui_i][ui_j], 2, "PRComponentInfo: PrQualityLevel" ) );
      RNOK( pcWriteIf->writeUvlc( m_uiNumPrSliceMinus1[ui_i][ui_j], "PRComponentInfo: NumPrSliceMinus1" ) );
      for( ui_k=0; ui_k<=m_uiNumPrSliceMinus1[ui_i][ui_j]; ui_k++ )
      {
        RNOK( pcWriteIf->writeUvlc( m_uiChromaOffset[ui_i][ui_j][ui_k], "PRComponentInfo: ChromaOffset" ) );
      }
    }
  }
  uiPayloadSize = ( pcWriteIf->getNumberOfWrittenBits() - uiStart + 7 )/8;

  return Err::m_nOK;
}

ErrVal
SEI::PRComponentInfoSei::read( HeaderSymbolReadIf *pcReadIf )
{
  UInt ui_i, ui_j, ui_k;
  RNOK( pcReadIf->getUvlc( m_uiNumDependencyIdMinus1, "PRComponentInfo: NumDependencyIdMinus1" ) );
  for( ui_i=0; ui_i<=m_uiNumDependencyIdMinus1; ui_i++ )
  {
    RNOK( pcReadIf->getCode( m_uiPrDependencyId[ui_i], 3, "PRComponentInfo: PrDependencyId" ) );
    RNOK( pcReadIf->getUvlc( m_uiNumQualityLevelMinus1[ui_i], "PRComponentInfo: NumQualityLevelsMinus1" ) );
    for( ui_j=0; ui_j<=m_uiNumQualityLevelMinus1[ui_i]; ui_j++ )
    {
      RNOK( pcReadIf->getCode( m_uiPrQualityLevel[ui_i][ui_j], 2, "PRComponentInfo: PrQualityLevel" ) );
      RNOK( pcReadIf->getUvlc( m_uiNumPrSliceMinus1[ui_i][ui_j], "PRComponentInfo: NumPrSliceMinus1" ) );
      for( ui_k=0; ui_k<=m_uiNumPrSliceMinus1[ui_i][ui_j]; ui_k++ )
      {
        RNOK( pcReadIf->getUvlc( m_uiChromaOffset[ui_i][ui_j][ui_k], "PRComponentInfo: ChromaOffset" ) );
      }
    }
  }
  return Err::m_nOK;
}

// JVT-V068 HRD {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  BufferingPeriod
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrVal SEI::BufferingPeriod::create( BufferingPeriod*& rpcBufferingPeriod, BufferingPeriod* pcBufferingPeriod )
{
  rpcBufferingPeriod = new BufferingPeriod( pcBufferingPeriod->m_pcParameterSetMng );
  ROT( NULL == rpcBufferingPeriod );
  rpcBufferingPeriod->m_uiSeqParameterSetId       = pcBufferingPeriod->m_uiSeqParameterSetId;
  rpcBufferingPeriod->m_aacSchedSel[HRD::NAL_HRD] = pcBufferingPeriod->m_aacSchedSel[HRD::NAL_HRD];
  rpcBufferingPeriod->m_aacSchedSel[HRD::VCL_HRD] = pcBufferingPeriod->m_aacSchedSel[HRD::VCL_HRD];
  return Err::m_nOK;
}

SEI::BufferingPeriod::~BufferingPeriod()
{
  if (m_bHrdParametersPresentFlag[HRD::NAL_HRD])
  {
    m_aacSchedSel[HRD::NAL_HRD].uninit();
  }
  if (m_bHrdParametersPresentFlag[HRD::VCL_HRD])
  {
    m_aacSchedSel[HRD::VCL_HRD].uninit();
  } 
}

ErrVal SEI::BufferingPeriod::create( BufferingPeriod*& rpcBufferingPeriod, ParameterSetMng*& rpcParameterSetMng )
{
  rpcBufferingPeriod = new BufferingPeriod( rpcParameterSetMng );
  ROT( NULL == rpcBufferingPeriod );
  return Err::m_nOK;
}

ErrVal SEI::BufferingPeriod::setHRD( UInt uiSPSId, const HRD* apcHrd[] )
{
  ROF( m_pcParameterSetMng->isValidSPS(uiSPSId));
  m_uiSeqParameterSetId = uiSPSId;

  SequenceParameterSet *pcSPS = NULL;
  m_pcParameterSetMng->get( pcSPS, m_uiSeqParameterSetId);  

  m_apcHrd[HRD::NAL_HRD] = apcHrd[HRD::NAL_HRD];
  m_apcHrd[HRD::VCL_HRD] = apcHrd[HRD::VCL_HRD];

  if (m_apcHrd[HRD::NAL_HRD]->getHrdParametersPresentFlag()) 
  {
    m_aacSchedSel[HRD::NAL_HRD].init( m_apcHrd[HRD::NAL_HRD]->getCpbCnt() );
  }

  if (m_apcHrd[HRD::VCL_HRD]->getHrdParametersPresentFlag()) 
  {
    m_aacSchedSel[HRD::VCL_HRD].init( m_apcHrd[HRD::VCL_HRD]->getCpbCnt() );
  }

  return Err::m_nOK;
}

ErrVal SEI::BufferingPeriod::write( HeaderSymbolWriteIf* pcWriteIf )
{
  RNOK( pcWriteIf->writeUvlc( m_uiSeqParameterSetId, "SEI: seq_parameter_set_id" ) );

  if ( m_apcHrd[HRD::NAL_HRD]->getHrdParametersPresentFlag() )
  {
    for( UInt n = 0; n < m_apcHrd[HRD::NAL_HRD]->getCpbCnt(); n++ )
    {
      RNOK( getSchedSel( HRD::NAL_HRD, n ).write( pcWriteIf, *(m_apcHrd[HRD::NAL_HRD]) ) );
    }
  }

  if (m_apcHrd[HRD::VCL_HRD]->getHrdParametersPresentFlag())
  {
    for( UInt n = 0; n < m_apcHrd[HRD::VCL_HRD]->getCpbCnt(); n++ )
    {
      RNOK( getSchedSel( HRD::VCL_HRD, n ).write( pcWriteIf, *(m_apcHrd[HRD::VCL_HRD]) ) );
    }
  }
  return Err::m_nOK;
}



ErrVal SEI::BufferingPeriod::read ( HeaderSymbolReadIf* pcReadIf )
{
//  RNOKS( pcReadIf->getUvlc( m_uiSeqParameterSetId, "SEI: seq_parameter_set_id" ) );
//
//  ROF( m_pcParameterSetMng->isValidSPS(m_uiSeqParameterSetId));
//
//  SequenceParameterSet *pcSPS = NULL;
//  m_pcParameterSetMng->get( pcSPS,m_uiSeqParameterSetId);  
//
//  m_pcVUI = pcSPS->getVUI ();// get a pointer to VUI of the SPS with m_uiSeqParameterSetId
//
//  Bool bNalHrdBpPresentFlag = m_pcVUI ? m_pcVUI->getNalHrd(m_uiLayerIndex).getHrdParametersPresentFlag() : false;
//  Bool bVclHrdBpPresentFlag = m_pcVUI ? m_pcVUI->getVclHrd(m_uiLayerIndex).getHrdParametersPresentFlag() : false;
//
//  if (bNalHrdBpPresentFlag) 
//  {
//    m_aacSchedSel[HRD::NAL_HRD].init( m_pcVUI->getNalHrd(m_uiLayerIndex).getCpbCnt() + 1 );
//  }
//  if (bVclHrdBpPresentFlag) 
//  {
//    m_aacSchedSel[HRD::VCL_HRD].init( m_pcVUI->getVclHrd(m_uiLayerIndex).getCpbCnt() + 1 );
//  }
//
//  if (bNalHrdBpPresentFlag)
//  {
//    const HRD& rcNalHRD = m_pcVUI->getNalHrd(m_uiLayerIndex);
//    for( UInt n = 0; n < rcNalHRD.getCpbCnt(); n++ )
//    {
//      RNOKS( getSchedSel( HRD::NAL_HRD, n ).read( pcReadIf, rcNalHRD ) );
//    }
//  }
//
//  if (bVclHrdBpPresentFlag)
//  {
//    const HRD& rcVlcHRD = m_pcVUI->getVclHrd(m_uiLayerIndex);
//    for( UInt n = 0; n < rcVlcHRD.getCpbCnt(); n++ )
//    {
//      RNOKS( getSchedSel( HRD::VCL_HRD, n ).read( pcReadIf, rcVlcHRD ) );
//    }
//  }
  return Err::m_nOK;
}



ErrVal SEI::BufferingPeriod::SchedSel::write( HeaderSymbolWriteIf* pcWriteIf, const HRD& rcHrd )
{
  const UInt uiLength = rcHrd.getInitialCpbRemovalDelayLength();
  RNOK( pcWriteIf->writeCode( m_uiInitialCpbRemovalDelay,       uiLength, "SEI: initial_cpb_removal_delay ") );
  RNOK( pcWriteIf->writeCode( m_uiInitialCpbRemovalDelayOffset, uiLength, "SEI: initial_cpb_removal_delay_offset ") );
  return Err::m_nOK;
}


ErrVal SEI::BufferingPeriod::SchedSel::read ( HeaderSymbolReadIf* pcReadIf, const HRD& rcHrd )
{
//  const UInt uiLength = rcHrd.getInitialCpbRemovalDelayLength();
//  RNOKS( pcReadIf->getCode( m_uiInitialCpbRemovalDelay ,       uiLength, "SEI: initial_cpb_removal_delay ") );
//  RNOKS( pcReadIf->getCode( m_uiInitialCpbRemovalDelayOffset,  uiLength, "SEI: initial_cpb_removal_delay_offset ") );
  return Err::m_nOK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  PicTimiming
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

UInt SEI::PicTiming::getNumClockTs()
{
  switch ( m_ePicStruct )
  {
  case PS_FRAME:
  case PS_TOP:
  case PS_BOT:
    return 1;
  case PS_TOP_BOT:
  case PS_BOT_TOP:
    return 2;
  default:  return 0; // reserved
  }
}

ErrVal SEI::PicTiming::create( PicTiming*& rpcPicTiming, const VUI* pcVUI, UInt uiLayerIndex )
{
  //ROT( NULL == pcVUI );
  rpcPicTiming = new PicTiming( *pcVUI, uiLayerIndex );
  ROT( NULL == rpcPicTiming );
  return Err::m_nOK;
}

ErrVal SEI::PicTiming::create(PicTiming*& rpcPicTiming, ParameterSetMng* parameterSetMng, UInt uiSPSId, UInt uiLayerIndex)
{
	ROF( parameterSetMng->isValidSPS(uiSPSId));

	SequenceParameterSet *pcSPS = NULL;
	parameterSetMng->get( pcSPS, uiSPSId );  

	VUI& rcVUI = *(pcSPS->getVUI ()); // get a pointer to VUI of SPS with m_uiSeqParameterSetId

	rpcPicTiming = new PicTiming( rcVUI, uiLayerIndex );
	ROT( NULL == rpcPicTiming );

	return Err::m_nOK;
}

ErrVal SEI::PicTiming::write( HeaderSymbolWriteIf* pcWriteIf )
{
  const HRD* pcHRD = NULL;
  if( m_rcVUI.getNalHrd(m_uiLayerIndex).getHrdParametersPresentFlag() )
  {
    pcHRD = &m_rcVUI.getNalHrd(m_uiLayerIndex);
  }
  else if( m_rcVUI.getVclHrd(m_uiLayerIndex).getHrdParametersPresentFlag() )
  {
    pcHRD = &m_rcVUI.getVclHrd(m_uiLayerIndex);
  }

  if( NULL != pcHRD )
  {
    RNOK( pcWriteIf->writeCode( m_uiCpbRemovalDelay,  pcHRD->getCpbRemovalDelayLength(), "SEI: cpb_removal_delay"  ) );
    RNOK( pcWriteIf->writeCode( m_uiDpbOutputDelay,   pcHRD->getDpbOutputDelayLength(),  "SEI: dpb_output_delay"   ) );
  }

  if( m_rcVUI.getPicStructPresentFlag(m_uiLayerIndex) )
  {
    RNOK( pcWriteIf->writeCode( m_ePicStruct, 4,      "SEI: pic_struct"         ) );
    AOT( NULL == pcHRD );
    UInt uiNumClockTs = getNumClockTs(); 
    for( UInt n = 0; n < uiNumClockTs ; n++ ) 
    {
      RNOK( m_acClockTimestampBuf.get( n ).write( pcWriteIf, *pcHRD ) );
    }
  }

  return Err::m_nOK;
}

Int  SEI::PicTiming::getTimestamp( UInt uiNum, UInt uiLayerIndex )
{
  return m_acClockTimestampBuf.get( uiNum ).get( m_rcVUI, uiLayerIndex );
}

ErrVal  SEI::PicTiming::setDpbOutputDelay( UInt uiDpbOutputDelay)
{
  m_uiDpbOutputDelay = uiDpbOutputDelay;
  return Err::m_nOK;
}


ErrVal SEI::PicTiming::setTimestamp( UInt uiNum, UInt uiLayerIndex, Int iTimestamp )
{
  m_acClockTimestampBuf.get( uiNum ).set( m_rcVUI, uiLayerIndex, iTimestamp );
  return Err::m_nOK;
}

ErrVal  SEI::PicTiming::setCpbRemovalDelay(UInt uiCpbRemovalDelay)
{
  m_uiCpbRemovalDelay = uiCpbRemovalDelay;
  return Err::m_nOK;
}


ErrVal SEI::PicTiming::read( HeaderSymbolReadIf* pcReadIf )
{
  const HRD* pcHRD = NULL;
  if( m_rcVUI.getNalHrd(m_uiLayerIndex).getHrdParametersPresentFlag() )
  {
    pcHRD = &m_rcVUI.getNalHrd(m_uiLayerIndex);
  }
  else if( m_rcVUI.getVclHrd(m_uiLayerIndex).getHrdParametersPresentFlag() )
  {
    pcHRD = &m_rcVUI.getVclHrd(m_uiLayerIndex);
  }

  if( NULL != pcHRD )
  {
    RNOKS( pcReadIf->getCode( m_uiCpbRemovalDelay,  pcHRD->getCpbRemovalDelayLength(), "SEI: cpb_removal_delay"  ) );
    RNOKS( pcReadIf->getCode( m_uiDpbOutputDelay,   pcHRD->getDpbOutputDelayLength(),  "SEI: dpb_output_delay"   ) );
  }

  if( m_rcVUI.getPicStructPresentFlag(m_uiLayerIndex) )
  {
    UInt uiCode = 0;
    RNOKS( pcReadIf->getCode( uiCode, 4, "SEI: pic_struct" ) );
    m_ePicStruct = (PicStruct)uiCode;
    AOT( NULL == pcHRD );
    UInt uiNumClockTs = getNumClockTs(); 
    for( UInt n = 0; n < uiNumClockTs ; n++ ) 
    {
      RNOKS( m_acClockTimestampBuf.get( n ).read( pcReadIf, *pcHRD ) );
    }
  }

  return Err::m_nOK;
}

ErrVal SEI::PicTiming::ClockTimestamp::write( HeaderSymbolWriteIf* pcWriteIf, const HRD& rcHRD )
{
  RNOK( pcWriteIf->writeFlag( m_bClockTimestampFlag, "SEI: clock_timestamp_flag"   ) );

  ROFRS( m_bClockTimestampFlag, Err::m_nOK );

  m_bFullTimestampFlag = (0 != m_uiHours);

  RNOK( pcWriteIf->writeCode( m_uiCtType, 2,         "SEI: ct_type"                ) );
  RNOK( pcWriteIf->writeFlag( m_bNuitFieldBasedFlag, "SEI: nuit_field_based_flag"  ) );
  RNOK( pcWriteIf->writeCode( m_uiCountingType, 5,   "SEI: counting_type"          ) );
  RNOK( pcWriteIf->writeFlag( m_bFullTimestampFlag,  "SEI: full_timestamp_flag"    ) );
  RNOK( pcWriteIf->writeFlag( m_bDiscontinuityFlag,  "SEI: discontinuity_flag"     ) );
  RNOK( pcWriteIf->writeFlag( m_bCntDroppedFlag,     "SEI: cnt_dropped_flag"       ) );
  RNOK( pcWriteIf->writeCode( m_uiNFrames, 8,        "SEI: n_frames"               ) );

  if( m_bFullTimestampFlag )
  {
    RNOK( pcWriteIf->writeCode( m_uiSeconds, 6,      "SEI: seconds_value"          ) );
    RNOK( pcWriteIf->writeCode( m_uiMinutes, 6,      "SEI: minutes_value"          ) );
    RNOK( pcWriteIf->writeCode( m_uiHours,   5,      "SEI: hours_value"            ) );
  }
  else
  {
    Bool bHoursFlag   = ( 0 != m_uiHours );
    Bool bMinutesFlag = ( 0 != m_uiMinutes ) || bHoursFlag;
    Bool bSecondFlag  = ( 0 != m_uiSeconds ) || bMinutesFlag;

    RNOK( pcWriteIf->writeFlag( bSecondFlag,         "SEI: seconds_flag"           ) );
    if( bSecondFlag )
    {
      RNOK( pcWriteIf->writeCode( m_uiSeconds, 6,    "SEI: seconds_value"          ) );
      RNOK( pcWriteIf->writeFlag( bMinutesFlag,      "SEI: minutes_flag"           ) );
      if( bMinutesFlag )
      {
        RNOK( pcWriteIf->writeCode( m_uiMinutes, 6,  "SEI: minutes_value"          ) );
        RNOK( pcWriteIf->writeFlag( bHoursFlag,      "SEI: hours_flag"             ) );
        if( bHoursFlag )
        {
          RNOK( pcWriteIf->writeCode( m_uiHours, 5,  "SEI: hours_value"            ) );
        }
      }
    }
  }

  if( rcHRD.getTimeOffsetLength() > 0 )
  {
    RNOK( pcWriteIf->writeSCode( m_iTimeOffset, rcHRD.getTimeOffsetLength(), "SEI: time_offset" ) );
  }
  return Err::m_nOK;
}

ErrVal SEI::PicTiming::ClockTimestamp::read( HeaderSymbolReadIf* pcReadIf, const HRD& rcHRD )
{
  RNOKS( pcReadIf->getFlag( m_bClockTimestampFlag, "SEI: clock_timestamp_flag"   ) );

  ROFRS( m_bClockTimestampFlag, Err::m_nOK );

  RNOKS( pcReadIf->getCode( m_uiCtType, 2,         "SEI: ct_type"                ) );
  RNOKS( pcReadIf->getFlag( m_bNuitFieldBasedFlag, "SEI: nuit_field_based_flag"  ) );
  RNOKS( pcReadIf->getCode( m_uiCountingType, 5,   "SEI: counting_type"          ) );
  RNOKS( pcReadIf->getFlag( m_bFullTimestampFlag,  "SEI: full_timestamp_flag"    ) );
  RNOKS( pcReadIf->getFlag( m_bDiscontinuityFlag,  "SEI: discontinuity_flag"     ) );
  RNOKS( pcReadIf->getFlag( m_bCntDroppedFlag,     "SEI: cnt_dropped_flag"       ) );
  RNOKS( pcReadIf->getCode( m_uiNFrames, 8,        "SEI: n_frames"               ) );

  if( m_bFullTimestampFlag )
  {
    RNOKS( pcReadIf->getCode( m_uiSeconds, 6,      "SEI: seconds_value"          ) );
    RNOKS( pcReadIf->getCode( m_uiMinutes, 6,      "SEI: minutes_value"          ) );
    RNOKS( pcReadIf->getCode( m_uiHours,   5,      "SEI: hours_value"            ) );
  }
  else
  {
    Bool bSecondFlag;
    RNOKS( pcReadIf->getFlag( bSecondFlag,         "SEI: seconds_flag"           ) );
    if( bSecondFlag )
    {
      RNOKS( pcReadIf->getCode( m_uiSeconds, 6,    "SEI: seconds_value"          ) );
      Bool bMinutesFlag;
      RNOKS( pcReadIf->getFlag( bMinutesFlag,      "SEI: minutes_flag"           ) );
      if( bMinutesFlag )
      {
        RNOKS( pcReadIf->getCode( m_uiMinutes, 6,  "SEI: minutes_value"          ) );
        Bool bHoursFlag;
        RNOKS( pcReadIf->getFlag( bHoursFlag,      "SEI: hours_flag"             ) );
        if( bHoursFlag )
        {
          RNOKS( pcReadIf->getCode( m_uiHours, 5,  "SEI: hours_value"            ) );
        }
      }
    }
  }

  if( rcHRD.getTimeOffsetLength() > 0 )
  {
    RNOKS( pcReadIf->getSCode( m_iTimeOffset, rcHRD.getTimeOffsetLength(), "SEI: time_offset" ) );
  }
  return Err::m_nOK;
}

Int SEI::PicTiming::ClockTimestamp::get( const VUI& rcVUI, UInt uiLayerIndex )
{
  const VUI::TimingInfo& rcTimingInfo = rcVUI.getTimingInfo( uiLayerIndex );
  Int iTimeOffset  = (((m_uiHours * 60 + m_uiMinutes) * 60) + m_uiSeconds ) * rcTimingInfo.getTimeScale();
  Int iFrameOffset = m_uiNFrames * ( rcTimingInfo.getNumUnitsInTick() * (m_bNuitFieldBasedFlag?2:1));

  return iTimeOffset + iFrameOffset + m_iTimeOffset;
}

Void SEI::PicTiming::ClockTimestamp::set( const VUI& rcVUI, UInt uiLayerIndex, Int iTimestamp )
{
  const VUI::TimingInfo& rcTimingInfo = rcVUI.getTimingInfo(uiLayerIndex);
  Int iTime     = iTimestamp / rcTimingInfo.getTimeScale();
  Int iTimeSub  = 0;
  m_uiHours     = iTime / 360;
  iTimeSub      = m_uiHours * 360;
  m_uiMinutes   = (iTime - iTimeSub) / 60;
  iTimeSub     += m_uiMinutes * 60;
  m_uiSeconds   = iTime - iTimeSub;
  iTimeSub     += m_uiSeconds;

  iTime         = iTimestamp - iTimeSub * rcTimingInfo.getTimeScale();

  Int iScale    = ( rcTimingInfo.getNumUnitsInTick() * (m_bNuitFieldBasedFlag?2:1));
  m_uiNFrames   = iTime / iScale;
  m_iTimeOffset = iTime - m_uiNFrames * iScale;
}

ErrVal SEI::AVCCompatibleHRD::create( AVCCompatibleHRD*& rpcAVCCompatibleHRD, VUI* pcVUI )
{
  ROT( NULL == pcVUI );
  rpcAVCCompatibleHRD = new AVCCompatibleHRD( *pcVUI );
  ROT( NULL == rpcAVCCompatibleHRD );
  return Err::m_nOK;
}

ErrVal SEI::AVCCompatibleHRD::write( HeaderSymbolWriteIf* pcWriteIf )
{
  AOF( m_rcVUI.getNumTemporalLevels() > 1 );

  RNOK( pcWriteIf->writeUvlc( m_rcVUI.getNumLayers() - 1,  "SEI: num_of_temporal_layers_in_base_layer_minus1"  ) );

  for ( UInt ui = 0; ui < m_rcVUI.getNumLayers(); ui++ )
  {
    VUI::LayerInfo& rcLayerInfo = m_rcVUI.getLayerInfo(ui);
    if ( rcLayerInfo.getTemporalLevel() < m_rcVUI.getNumTemporalLevels() - 1 )
    {
      RNOK( pcWriteIf->writeCode( rcLayerInfo.getTemporalLevel(), 3,  "SEI: temporal_level"  ) );
      RNOK( m_rcVUI.getTimingInfo(ui).write( pcWriteIf) );
      RNOK( m_rcVUI.getNalHrd(ui).write( pcWriteIf ) );
      RNOK( m_rcVUI.getVclHrd(ui).write( pcWriteIf ) );
      if( m_rcVUI.getNalHrd(ui).getHrdParametersPresentFlag() || m_rcVUI.getVclHrd(ui).getHrdParametersPresentFlag() )
      {
        RNOK( pcWriteIf->writeFlag( m_rcVUI.getLowDelayHrdFlag(ui),           "VUI: low_delay_hrd_flag[i]"));
      }
      RNOK( pcWriteIf->writeFlag( m_rcVUI.getPicStructPresentFlag(ui),        "VUI: pic_struct_present_flag[i]"));
    }
  }

  return Err::m_nOK;
}

ErrVal
SEI::AVCCompatibleHRD::read( HeaderSymbolReadIf* pcReadIf )
{
  AF();
  return Err::m_nOK;
}

// JVT-V068 HRD }
//JVT-W049 {
//////////////////////////////////////////////////////////////////////////
// 
//      REDUNDANT PIC       S E I
//
//////////////////////////////////////////////////////////////////////////
SEI::RedundantPicSei::RedundantPicSei	() 
: SEIMessage									( REDUNDANT_PIC_SEI )
{
    UInt i=0, j=0, k=0;
	m_num_dId_minus1=0; 
	::memset(  m_dependency_id, 0x00, MAX_LAYERS*sizeof(UInt)  );                                            
    ::memset(  m_num_qId_minus1, 0x00, MAX_LAYERS*sizeof(UInt)  );                                           
	::memset(  m_quality_id, 0x00, MAX_LAYERS*MAX_QUALITY_LEVELS*sizeof(UInt*)  );                                        
	::memset(  m_num_redundant_pics_minus1, 0x00, MAX_LAYERS*MAX_QUALITY_LEVELS*sizeof(UInt*)  );                    
	for( i = 0; i < MAX_LAYERS; i++ ) 
	{
		for( j = 0; j < MAX_QUALITY_LEVELS; j++ ) 
		{
			for( k = 0; k < MAX_REDUNDANT_PICTURES_NUM; k++ ) 
			{
				m_redundant_pic_cnt_minus1[ i ][ j ][ k ]     = 0;
				m_pic_match_flag[ i ][ j ][ k ]               = 0;
                m_mb_type_match_flag[ i ][ j ][ k ]           = 0;
				m_motion_match_flag[ i ][ j ][ k ]            = 0;
				m_residual_match_flag[ i ][ j ][ k ]          = 0;
				m_intra_samples_match_flag[ i ][ j ][ k ]     = 0;
				
			}
		}
	}              
}

SEI::RedundantPicSei::~RedundantPicSei()
{
}

ErrVal
SEI::RedundantPicSei::create( RedundantPicSei*& rpcSeiMessage )
{
	rpcSeiMessage = new RedundantPicSei();
	ROT( NULL == rpcSeiMessage )
		return Err::m_nOK;
}

ErrVal
SEI::RedundantPicSei::write(HeaderSymbolWriteIf* pcWriteIf)
{
	UInt i=0, j=0, k=0;
	ROF  ( m_num_dId_minus1+1 );	
	RNOK ( pcWriteIf->writeUvlc(m_num_dId_minus1,						                           "RedundantPicSEI: num_layers_minus1"		           ) );
	for( i = 0; i <= m_num_dId_minus1; i++ ) 
	{
		RNOK	( pcWriteIf->writeCode( m_dependency_id[ i ] ,                              3 ,    "RedundantPicSEI: dependency_id"                    ) );
		RNOK    ( pcWriteIf->writeUvlc( m_num_qId_minus1[ i ],                                     "RedundantPicSEI: m_num_qId_minus1"                 ) );
		for( j = 0; j <= m_num_qId_minus1[ i ]; j++ ) 
		{
			RNOK	( pcWriteIf->writeCode( m_quality_id[ i ][ j ],                         2 ,    "RedundantPicSEI: m_quality_id"                     ) );
			RNOK    ( pcWriteIf->writeUvlc( m_num_redundant_pics_minus1[ i ][ j ],                 "RedundantPicSEI: m_num_redundant_pics_minus1"      ) );
			for( k = 0; k <= m_num_redundant_pics_minus1[ i ][ j ]; k++ ) 
			{
				RNOK    ( pcWriteIf->writeUvlc( m_redundant_pic_cnt_minus1[ i ][ j ][ k ],         "RedundantPicSEI: m_redundant_pic_cnt_minus1"       ) );
				RNOK    ( pcWriteIf->writeFlag( m_pic_match_flag[ i ][ j ][ k ],                   "RedundantPicSEI: m_pic_match_flag"                 ) );
				if( !m_pic_match_flag[ i ][ j ][ k ]) 
				{
					RNOK    ( pcWriteIf->writeFlag( m_mb_type_match_flag[ i ][ j ][ k ],           "RedundantPicSEI: m_mb_type_match_flag"             ) );
					RNOK    ( pcWriteIf->writeFlag( m_motion_match_flag[ i ][ j ][ k ],            "RedundantPicSEI: m_motion_match_flag"              ) );
					RNOK    ( pcWriteIf->writeFlag( m_residual_match_flag[ i ][ j ][ k ],          "RedundantPicSEI: m_residual_match_flag"            ) );
					RNOK    ( pcWriteIf->writeFlag( m_intra_samples_match_flag[ i ][ j ][ k ],     "RedundantPicSEI: m_intra_samples_match_flag"       ) );
				}
			}
		}
	}
 return Err::m_nOK;
}

ErrVal
SEI::RedundantPicSei::read(HeaderSymbolReadIf* pcReadIf)
{
	UInt i=0, j=0, k=0;	
	RNOK ( pcReadIf->getUvlc(m_num_dId_minus1,						                            ""		            ) );
	for( i = 0; i <= m_num_dId_minus1; i++ ) 
	{
		RNOK	( pcReadIf->getCode( m_dependency_id[ i ] ,                              3 ,    ""                  ) );
		RNOK    ( pcReadIf->getUvlc( m_num_qId_minus1[ i ],                                     ""                  ) );
		for( j = 0; j <= m_num_qId_minus1[ i ]; j++ ) 
		{
			RNOK	( pcReadIf->getCode( m_quality_id[ i ][ j ],                         2 ,    ""                  ) );
			RNOK    ( pcReadIf->getUvlc( m_num_redundant_pics_minus1[ i ][ j ],                 ""                  ) );
			for( k = 0; k <= m_num_redundant_pics_minus1[ i ][ j ]; k++ ) 
			{
				RNOK    ( pcReadIf->getUvlc( m_redundant_pic_cnt_minus1[ i ][ j ][ k ],         ""                  ) );
				RNOK    ( pcReadIf->getFlag( m_pic_match_flag[ i ][ j ][ k ],                   ""                  ) );
				if( !m_pic_match_flag[ i ][ j ][ k ]) 
				{
					RNOK    ( pcReadIf->getFlag( m_mb_type_match_flag[ i ][ j ][ k ],           ""                  ) );
					RNOK    ( pcReadIf->getFlag( m_motion_match_flag[ i ][ j ][ k ],            ""                  ) );
					RNOK    ( pcReadIf->getFlag( m_residual_match_flag[ i ][ j ][ k ],          ""                  ) );
					RNOK    ( pcReadIf->getFlag( m_intra_samples_match_flag[ i ][ j ][ k ],     ""                  ) );
				}
			}
		}
	}
	return Err::m_nOK;
}
//JVT-W049}
//JVT-X032{
//////////////////////////////////////////////////////////////////////////
// 
//      TEMPORAL LEVEL SWITCHING POINT      S E I
//
//////////////////////////////////////////////////////////////////////////
SEI::TLSwitchingPointSei::TLSwitchingPointSei	() 
: SEIMessage									( TL_SWITCHING_POINT_SEI )
{
   m_delta_frame_num = 0;              
}

SEI::TLSwitchingPointSei::~TLSwitchingPointSei()
{
}

ErrVal
SEI::TLSwitchingPointSei::create( TLSwitchingPointSei*& rpcSeiMessage )
{
	rpcSeiMessage = new TLSwitchingPointSei();
	ROT( NULL == rpcSeiMessage )
		return Err::m_nOK;
}

ErrVal
SEI::TLSwitchingPointSei::write(HeaderSymbolWriteIf* pcWriteIf)
{
	RNOK ( pcWriteIf->writeSvlc(m_delta_frame_num,						                           "TLSwitchingPointSEI: m_delta_frame_num"		           ) );
    return Err::m_nOK;
}

ErrVal
SEI::TLSwitchingPointSei::read(HeaderSymbolReadIf* pcReadIf)
{
	RNOK ( pcReadIf->getSvlc(m_delta_frame_num,						                            ""		            ) );
	return Err::m_nOK;
}
//JVT-X032}

//JVT-W062 {
//////////////////////////////////////////////////////////////////////////
//
//			TL0 DEP REP INDEX SEI
//
//////////////////////////////////////////////////////////////////////////

SEI::Tl0DepRepIdxSei::Tl0DepRepIdxSei (): SEIMessage		( TL0_DEP_REP_IDX_SEI )
{
}

SEI::Tl0DepRepIdxSei::~Tl0DepRepIdxSei ()
{
}

ErrVal
SEI::Tl0DepRepIdxSei::create ( Tl0DepRepIdxSei*& rpcSeiMessage)
{
  rpcSeiMessage = new Tl0DepRepIdxSei();
  ROT( NULL == rpcSeiMessage );
  return Err::m_nOK;
}

ErrVal
SEI::Tl0DepRepIdxSei::destroy()
{
    delete this;
	return Err::m_nOK;
}

ErrVal
SEI::Tl0DepRepIdxSei::read( HeaderSymbolReadIf *pcReadIf )
{
  RNOK( pcReadIf->getCode( m_uiTl0DepRepIdx, 8, "Tl0DepRepIdxSei: tl0_dep_rep_idx" ) );
  RNOK( pcReadIf->getCode( m_uiEfIdrPicId, 16, "Tl0DepRepIdxSei: effective_idr_pic_id" ) );
  return Err::m_nOK;
}

ErrVal
SEI::Tl0DepRepIdxSei::write( HeaderSymbolWriteIf *pcWriteIf )
{
  RNOK( pcWriteIf->writeCode( m_uiTl0DepRepIdx, 8, "Tl0DepRepIdxSei: tl0_dep_rep_idx" ) );
  RNOK( pcWriteIf->writeCode( m_uiEfIdrPicId, 16, "Tl0DepRepIdxSei: effective_idr_pic_id" ) );
  return Err::m_nOK;
}
//JVT-W062 }

H264AVC_NAMESPACE_END
