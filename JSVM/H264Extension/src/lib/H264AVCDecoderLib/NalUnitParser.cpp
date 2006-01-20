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





#include "H264AVCDecoderLib.h"
#include "BitReadBuffer.h"
#include "NalUnitParser.h"

#include "H264AVCCommonLib/TraceFile.h"


H264AVC_NAMESPACE_BEGIN


NalUnitParser::NalUnitParser()
: m_pcBitReadBuffer     ( 0 )
, m_pucBuffer           ( 0 )
, m_eNalUnitType        ( NAL_UNIT_EXTERNAL )
, m_eNalRefIdc          ( NAL_REF_IDC_PRIORITY_LOWEST )
, m_uiLayerId           ( 0 )
, m_uiTemporalLevel     ( 0 )
, m_uiQualityLevel      ( 0 )
, m_bCheckAllNALUs      ( false ) //JVT-P031
, m_uiDecodedLayer      ( 0 ) //JVT-P031
{
  for ( UInt uiLoop = 0; uiLoop < (1 << PRI_ID_BITS); uiLoop++ )
  {
    m_uiTemporalLevelList[uiLoop] = 0;
    m_uiDependencyIdList [uiLoop] = 0;
    m_uiQualityLevelList [uiLoop] = 0;
  }
}


NalUnitParser::~NalUnitParser()
{
}


ErrVal
NalUnitParser::create( NalUnitParser*& rpcNalUnitParser )
{
  rpcNalUnitParser = new NalUnitParser;

  ROT( NULL == rpcNalUnitParser );

  return Err::m_nOK;
}


ErrVal
NalUnitParser::init( BitReadBuffer *pcBitReadBuffer )
{
  ROT(NULL == pcBitReadBuffer);

  m_pcBitReadBuffer = pcBitReadBuffer;

  return Err::m_nOK;
}


ErrVal
NalUnitParser::destroy()
{
  delete this;
  return Err::m_nOK;
}


Void
NalUnitParser::xTrace( Bool bDDIPresent )
{
  g_nLayer = m_uiLayerId;
  DTRACE_LAYER(m_uiLayerId);

  //===== head line =====
  switch( m_eNalUnitType )
  {
  case NAL_UNIT_SPS:
    DTRACE_HEADER( "SEQUENCE PARAMETER SET" );
    break;

  case NAL_UNIT_PPS:
    DTRACE_HEADER( "PICTURE PARAMETER SET" );
    break;

  case NAL_UNIT_SEI:
    DTRACE_HEADER( "SEI MESSAGE" );
    break;

  case NAL_UNIT_CODED_SLICE:
  case NAL_UNIT_CODED_SLICE_IDR:
  case NAL_UNIT_CODED_SLICE_SCALABLE:
  case NAL_UNIT_CODED_SLICE_IDR_SCALABLE:
    DTRACE_NEWSLICE;
    break;

  default:
    break;
  }

  //===== forbidden zero bit =====
  DTRACE_TH   ( "NALU HEADER: forbidden_zero_bit" );
  DTRACE_TY   ( " u(1)" );
  DTRACE_BITS ( 0, 1 );
  DTRACE_POS;
  DTRACE_CODE ( 0 );
  DTRACE_COUNT( 1 );
  DTRACE_N;

  //===== nal ref idc =====
  DTRACE_TH   ( "NALU HEADER: nal_ref_idc" );
  DTRACE_TY   ( " u(v)" );
  DTRACE_POS;
  DTRACE_CODE ( m_eNalRefIdc );
  DTRACE_BITS ( m_eNalRefIdc, 2 );
  DTRACE_COUNT( 2 );
  DTRACE_N;

  //===== nal unit type =====
  DTRACE_TH   ( "NALU HEADER: nal_unit_type" );
  DTRACE_TY   ( " u(v)" );
  DTRACE_POS;
  DTRACE_CODE ( m_eNalUnitType );
  DTRACE_BITS ( m_eNalUnitType, 5 );
  DTRACE_COUNT( 5 );
  DTRACE_N;

  ROFVS( bDDIPresent );

  //===== nal unit type =====
  DTRACE_TH   ( "NALU HEADER: temporal_level" );
  DTRACE_TY   ( " u(v)" );
  DTRACE_POS;
  DTRACE_CODE ( m_uiTemporalLevel );
  DTRACE_BITS ( m_uiTemporalLevel, 3 );
  DTRACE_COUNT( 3 );
  DTRACE_N;
  DTRACE_TH   ( "NALU HEADER: dependency_id" );
  DTRACE_TY   ( " u(v)" );
  DTRACE_POS;
  DTRACE_CODE ( m_uiLayerId );
  DTRACE_BITS ( m_uiLayerId, 3 );
  DTRACE_COUNT( 3 );
  DTRACE_N;
  DTRACE_TH   ( "NALU HEADER: quality_level" );
  DTRACE_TY   ( " u(v)" );
  DTRACE_POS;
  DTRACE_CODE ( m_uiQualityLevel );
  DTRACE_BITS ( m_uiQualityLevel, 2 );
  DTRACE_COUNT( 2 );
  DTRACE_N;
}

//JVT-P031
UInt
NalUnitParser::getNalHeaderSize( BinDataAccessor* pcBinDataAccessor )
{
  ROF( pcBinDataAccessor->size() );
  ROF( pcBinDataAccessor->data() );

  NalUnitType   eNalUnitType;
  NalRefIdc     eNalRefIdc;
  Bool			bExtensionFlag;

  UInt  uiHeaderLength  = 1;
  UChar ucByte          = pcBinDataAccessor->data()[0];


  //===== NAL unit header =====
  ROT( ucByte & 0x80 );                                     // forbidden_zero_bit ( &10000000b)
  eNalRefIdc          = NalRefIdc   ( ucByte >> 5     );  // nal_ref_idc        ( &01100000b)
  eNalUnitType        = NalUnitType ( ucByte &  0x1F  );  // nal_unit_type      ( &00011111b)
  
  //{{Variable Lengh NAL unit header data with priority and dead substream flag
  //France Telecom R&D- (nathalie.cammas@francetelecom.com)
  m_bDiscardableFlag = false;
  //}}Variable Lengh NAL unit header data with priority and dead substream flag

  if( eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE ||
      eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE )
  {
    ROF( pcBinDataAccessor->size() > 1 );

    ucByte              = pcBinDataAccessor->data()[1];
	bExtensionFlag     = ( ucByte     ) & 1;
	if(bExtensionFlag)
	{
	  uiHeaderLength      ++;
	}
    uiHeaderLength      ++;
  }

  return uiHeaderLength;
}
ErrVal
NalUnitParser::initSODBNalUnit( BinDataAccessor* pcBinDataAccessor )
{
  m_pucBuffer = pcBinDataAccessor->data();
  UInt uiPacketLength = pcBinDataAccessor->size();

  UInt uiBits;
  xConvertRBSPToSODB(uiPacketLength, uiBits);

  RNOK( m_pcBitReadBuffer->initPacket( (ULong*)(m_pucBuffer), uiBits) );
  return Err::m_nOK;
}

UInt
NalUnitParser::getBytesLeft()               
{
  return(m_pcBitReadBuffer->getBytesLeft());
}

UInt
NalUnitParser::getBitsLeft()               
{
  return(m_pcBitReadBuffer->getBitsLeft());
}
//~JVT-P031

ErrVal
NalUnitParser::initNalUnit( BinDataAccessor* pcBinDataAccessor, Bool* KeyPicFlag, Bool bPreParseHeader, Bool bConcatenated  ) //FRAG_FIX
{
  ROF( pcBinDataAccessor->size() );
  ROF( pcBinDataAccessor->data() );


  UInt  uiHeaderLength  = 1;
  UChar ucByte          = pcBinDataAccessor->data()[0];


  //===== NAL unit header =====
  ROT( ucByte & 0x80 );                                     // forbidden_zero_bit ( &10000000b)
  m_eNalRefIdc          = NalRefIdc   ( ucByte >> 5     );  // nal_ref_idc        ( &01100000b)
  if ( m_eNalRefIdc == NAL_REF_IDC_PRIORITY_HIGHEST && KeyPicFlag != NULL )
	*KeyPicFlag = true;
  m_eNalUnitType        = NalUnitType ( ucByte &  0x1F  );  // nal_unit_type      ( &00011111b)
  
  if( m_eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE ||
      m_eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE )
  {
    ROF( pcBinDataAccessor->size() > 1 );

    ucByte              = pcBinDataAccessor->data()[1];
    //{{Variable Lengh NAL unit header data with priority and dead substream flag
    //France Telecom R&D- (nathalie.cammas@francetelecom.com)
    m_uiSimplePriorityId = ( ucByte >> 2);
	  m_bDiscardableFlag	 = ( ucByte >> 1) & 1;
	  m_bExtensionFlag     = ( ucByte     ) & 1;
	  if(m_bExtensionFlag)
	  {
		    ucByte              = pcBinDataAccessor->data()[2];
		    m_uiTemporalLevel   = ( ucByte >> 5 );
		    m_uiLayerId         = ( ucByte >> 2 ) & 7;
		    m_uiQualityLevel    = ( ucByte      ) & 3;
		    uiHeaderLength      ++;
	  }
    else
    {
        // Look up simple priority ID in mapping table (J. Ridge, Y-K. Wang @ Nokia)
        m_uiTemporalLevel = m_uiTemporalLevelList[m_uiSimplePriorityId];
        m_uiLayerId       = m_uiDependencyIdList [m_uiSimplePriorityId];
        m_uiQualityLevel  = m_uiQualityLevelList [m_uiSimplePriorityId];
	  }
    //}}Variable Lengh NAL unit header data with priority and dead substream flag

    uiHeaderLength      ++;
  }
  else
  {
    m_uiTemporalLevel = ( m_eNalRefIdc > 0 ? 0 : 1 );
    m_uiLayerId       = 0;
    m_uiQualityLevel  = 0;
  }


  //===== TRACE output =====
  xTrace( uiHeaderLength > 1 );

  //===== NAL unit payload =====
  m_pucBuffer         = pcBinDataAccessor->data() + uiHeaderLength;
  UInt uiPacketLength = pcBinDataAccessor->size() - uiHeaderLength;

  //JVT-P031
  if(m_bDiscardableFlag == true && m_uiDecodedLayer > m_uiLayerId && !m_bCheckAllNALUs)
  {
		//Nal unit or fragment must be discarded
        uiPacketLength = 0;
  }
  //~JVT-P031

  // nothing more to do
  ROTRS( NAL_UNIT_END_OF_STREAM   == m_eNalUnitType ||
         NAL_UNIT_END_OF_SEQUENCE == m_eNalUnitType,    Err::m_nOK );

  // Unit->RBSP
  if(bPreParseHeader) //FRAG_FIX
      RNOK( xConvertPayloadToRBSP ( uiPacketLength ) );

  UInt uiBitsInPacket;
  // RBSP->SODB
  RNOK( xConvertRBSPToSODB    ( uiPacketLength, uiBitsInPacket ) );

  //FRAG_FIX
  if(!(m_bDiscardableFlag && m_uiDecodedLayer > m_uiLayerId)) //FRAG_FIX_3
  {
  if(bPreParseHeader)
      m_uiBitsInPacketSaved = uiBitsInPacket;
  if(!bPreParseHeader && !bConcatenated) //FRAG_FIX_3
      uiBitsInPacket = m_uiBitsInPacketSaved;
  } //FRAG_FIX_3

  if(!m_bDiscardableFlag || (m_bDiscardableFlag && m_uiDecodedLayer == m_uiLayerId) || m_bCheckAllNALUs) //JVT-P031
  {
      RNOK( m_pcBitReadBuffer->initPacket( (ULong*)(m_pucBuffer), uiBitsInPacket) );
  }
  return Err::m_nOK;
}



ErrVal
NalUnitParser::closeNalUnit()
{
  m_pucBuffer         = NULL;
  m_eNalUnitType      = NAL_UNIT_EXTERNAL;
  m_eNalRefIdc        = NAL_REF_IDC_PRIORITY_LOWEST;
  m_uiLayerId         = 0;
  m_uiTemporalLevel   = 0;
  m_uiQualityLevel    = 0;

  return Err::m_nOK;
}



ErrVal
NalUnitParser::xConvertPayloadToRBSP( UInt& ruiPacketLength )
{
  UInt uiZeroCount    = 0;
  UInt uiWriteOffset  = 0;
  UInt uiReadOffset   = 0;

  for( ; uiReadOffset < ruiPacketLength; uiReadOffset++, uiWriteOffset++ )
  {
    if( 2 == uiZeroCount && 0x03 == m_pucBuffer[uiReadOffset] )
    {
      uiReadOffset++;
      uiZeroCount = 0;
    }

    m_pucBuffer[uiWriteOffset] = m_pucBuffer[uiReadOffset];

    if( 0x00 == m_pucBuffer[uiReadOffset] )
    {
      uiZeroCount++;
    }
    else
    {
      uiZeroCount = 0;
    }
  }

  ruiPacketLength = uiWriteOffset;

  return Err::m_nOK;
}


ErrVal
NalUnitParser::xConvertRBSPToSODB( UInt  uiPacketLength,
                                   UInt& ruiBitsInPacket )
{
  uiPacketLength--;
  UChar *puc = m_pucBuffer;

  //remove zero bytes at the end of the stream
  while (puc[uiPacketLength] == 0x00)
  {
    uiPacketLength-=1;
  }

  // find the first non-zero bit
  UChar ucLastByte=puc[uiPacketLength];
  Int   i;
  for(  i = 0; (ucLastByte & 1 ) == 0; i++ )
  {
    ucLastByte >>= 1;
    AOT_DBG( i > 7 );
  }

  ruiBitsInPacket = (uiPacketLength << 3) + 8 - i;
  return Err::m_nOK;
}

#ifdef   CONFORMANCE_BUGFIX
ErrVal NalUnitParser::readAUDelimiter()
{
  DTRACE_HEADER("Access Unit Delimiter");

  UInt uiPicDelimiterType;
  m_pcBitReadBuffer->get(uiPicDelimiterType, 3);

  DTRACE_TH( "AUD: primary_pic_type"  );
  DTRACE_TY( " u(3)" );
  DTRACE_BITS(uiPicDelimiterType, 3);
  DTRACE_POS;
  DTRACE_CODE(uiPicDelimiterType);
  DTRACE_COUNT(3);
  DTRACE_N;

  return Err::m_nOK;
}

ErrVal NalUnitParser::readEndOfSeqence()
{
  DTRACE_HEADER("End of Sequence");
  return Err::m_nOK;
}

ErrVal NalUnitParser::readEndOfStream()
{
  DTRACE_HEADER("End of Stream");
  return Err::m_nOK;
}
#endif //CONFORMANCE_BUGFIX

H264AVC_NAMESPACE_END
