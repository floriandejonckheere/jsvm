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
  Bool m_bTraceEnable = true;
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
  MessageType eMessageType;
  UInt        uiSize;

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
  AOT( uiSize > 255 );

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
    default :           return ReservedSei::create( (ReservedSei*&) rpcSEIMessage, uiSize );
  }
  return Err::m_nOK;
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
  AOT(1);
  return Err::m_nOK;
}


ErrVal
SEI::ReservedSei::read( HeaderSymbolReadIf* pcReadIf )
{
  AOT(1);
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

SEI::ScalableSei::ScalableSei     ()
 : SEIMessage                     ( SCALABLE_SEI )
 , m_uiMaxHorFrameDimInMB         ( 0 )
 , m_uiMaxVerFrameDimInMB         ( 0 )
 , m_uiFrameRateUnitNom           ( 0 )
 , m_uiFrameRateUnitDenom         ( 0 )
 , m_uiMaxDecStages               ( 0 )
 , m_uiNumLayers                  ( 0 )
 , m_bBaseLayerIsAVC              ( false )
 , m_uiAVCTempResStages           ( 0 )
{
  ::memset( m_uiSpatialResolutionFactor,  0x00, MAX_LAYERS*sizeof(UInt) );
  ::memset( m_uiTemporalResolutionFactor, 0x00, MAX_LAYERS*sizeof(UInt) );
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
SEI::ScalableSei::write( HeaderSymbolWriteIf* pcWriteIf )
{
  ROF( m_uiNumLayers ); // check if initialized

  RNOK  ( pcWriteIf->writeUvlc( m_uiMaxHorFrameDimInMB-1,               "ScalableSEI: MaxHorFrameDimInMB"   ) );
  RNOK  ( pcWriteIf->writeUvlc( m_uiMaxVerFrameDimInMB-1,               "ScalableSEI: MaxVerFrameDimInMB"   ) );
  RNOK  ( pcWriteIf->writeUvlc( m_uiFrameRateUnitNom-1,                 "ScalableSEI: FrameRateUnitsNom"    ) );
  RNOK  ( pcWriteIf->writeUvlc( m_uiFrameRateUnitDenom-1,               "ScalableSEI: FrameRateUnitsDenom"  ) );
  RNOK  ( pcWriteIf->writeUvlc( m_uiMaxDecStages,                       "ScalableSEI: MaxDecStages"         ) );
  RNOK  ( pcWriteIf->writeUvlc( m_uiNumLayers-1,                        "ScalableSEI: NumLayers"            ) );
  RNOK  ( pcWriteIf->writeFlag( m_bBaseLayerIsAVC,                      "ScalableSEI: BaseLayerIsAVC"       ) );
  if( m_bBaseLayerIsAVC )
  {
    RNOK( pcWriteIf->writeUvlc( m_uiAVCTempResStages,                   "ScalableSEI: Log2AVCTempResFactor" ) );
  }
  for( UInt uiLayer = 0; uiLayer < m_uiNumLayers; uiLayer++ )
  {
    RNOK( pcWriteIf->writeUvlc( m_uiSpatialResolutionFactor [uiLayer],  "ScalableSEI: Log2SpatResFactor"    ) );
    RNOK( pcWriteIf->writeUvlc( m_uiTemporalResolutionFactor[uiLayer],  "ScalableSEI: Log2TempResFactor"    ) );
  }

  return Err::m_nOK;
}


ErrVal
SEI::ScalableSei::read ( HeaderSymbolReadIf* pcReadIf )
{
  RNOK  ( pcReadIf->getUvlc( m_uiMaxHorFrameDimInMB,                 "ScalableSEI: MaxHorFrameDimInMB"   ) );
  RNOK  ( pcReadIf->getUvlc( m_uiMaxVerFrameDimInMB,                 "ScalableSEI: MaxVerFrameDimInMB"   ) );
  RNOK  ( pcReadIf->getUvlc( m_uiFrameRateUnitNom,                   "ScalableSEI: FrameRateUnitsNom"    ) );
  RNOK  ( pcReadIf->getUvlc( m_uiFrameRateUnitDenom,                 "ScalableSEI: FrameRateUnitsDenom"  ) );
  RNOK  ( pcReadIf->getUvlc( m_uiMaxDecStages,                       "ScalableSEI: MaxDecStages"  ) );
  RNOK  ( pcReadIf->getUvlc( m_uiNumLayers,                          "ScalableSEI: NumLayers"            ) );
  m_uiMaxHorFrameDimInMB  ++;
  m_uiMaxVerFrameDimInMB  ++;
  m_uiFrameRateUnitNom    ++;
  m_uiFrameRateUnitDenom  ++;
  m_uiNumLayers           ++;
  
  RNOK  ( pcReadIf->getFlag( m_bBaseLayerIsAVC,                      "ScalableSEI: BaseLayerIsAVC"       ) );
  if( m_bBaseLayerIsAVC )
  {
    RNOK( pcReadIf->getUvlc( m_uiAVCTempResStages,                   "ScalableSEI: Log2AVCTempResFactor" ) );
  }
  for( UInt uiLayer = 0; uiLayer < m_uiNumLayers; uiLayer++ )
  {
    RNOK( pcReadIf->getUvlc( m_uiSpatialResolutionFactor [uiLayer],  "ScalableSEI: Log2SpatResFactor"    ) );
    RNOK( pcReadIf->getUvlc( m_uiTemporalResolutionFactor[uiLayer],  "ScalableSEI: Log2TempResFactor"    ) );
  }

  return Err::m_nOK;
}



H264AVC_NAMESPACE_END
