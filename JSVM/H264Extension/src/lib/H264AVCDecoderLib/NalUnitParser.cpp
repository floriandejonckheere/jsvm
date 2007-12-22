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
: m_bInitialized        ( false )
, m_bNalUnitInitialized ( false )
, m_pcBitReadBuffer     ( 0 )
{
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
NalUnitParser::destroy()
{
  ROT( m_bInitialized );
  ROT( m_bNalUnitInitialized );
  delete this;
  return Err::m_nOK;
}

ErrVal
NalUnitParser::init( BitReadBuffer*       pcBitReadBuffer,
                     HeaderSymbolReadIf*  pcHeaderSymbolReadIf )
{
  ROT( m_bInitialized );
  ROT( m_bNalUnitInitialized );
  ROF( pcBitReadBuffer );
  ROF( pcHeaderSymbolReadIf );
  m_bInitialized          = true;
  m_bNalUnitInitialized   = false;
  m_pcBitReadBuffer       = pcBitReadBuffer;
  m_pcHeaderSymbolReadIf  = pcHeaderSymbolReadIf;
  return Err::m_nOK;
}

ErrVal
NalUnitParser::uninit()
{
  ROF( m_bInitialized );
  ROT( m_bNalUnitInitialized );
  m_bInitialized          = false;
  m_bNalUnitInitialized   = false;
  m_pcBitReadBuffer       = 0;
  m_pcHeaderSymbolReadIf  = 0;
  return Err::m_nOK;
}

ErrVal
NalUnitParser::initNalUnit( BinDataAccessor& rcBinDataAccessor )
{
  ROF( m_bInitialized );
  ROT( m_bNalUnitInitialized );
  ROF( rcBinDataAccessor.size() );
  ROF( rcBinDataAccessor.data() );

  //===== determine NAL unit type =====
  UChar       ucFirstByte   = rcBinDataAccessor.data()[ 0 ];
  NalRefIdc   eNalRefIdc    = NalRefIdc   ( ( ucFirstByte & 0x7F ) >> 5 );
  NalUnitType eNalUnitType  = NalUnitType (   ucFirstByte & 0x1F );
  Bool        bTrailingBits = true;
  switch( eNalUnitType )
  {
  case NAL_UNIT_SPS:
    DTRACE_HEADER( "SEQUENCE PARAMETER SET" );
    break;
  case NAL_UNIT_SPS_EXTENSION:
    DTRACE_HEADER( "SEQUENCE PARAMETER SET EXTENSION" );
    break;
  case NAL_UNIT_SUBSET_SPS:
    DTRACE_HEADER( "SUBSET SEQUENCE PARAMETER SET" );
    break;
  case NAL_UNIT_PPS:
    DTRACE_HEADER( "PICTURE PARAMETER SET" );
    break;
  case NAL_UNIT_SEI:
    DTRACE_HEADER( "SEI" );
    break;
  case NAL_UNIT_FILLER_DATA:
    DTRACE_HEADER( "FILLER DATA" );
    break;
  case NAL_UNIT_ACCESS_UNIT_DELIMITER:
    DTRACE_HEADER( "ACCESS UNIT DELIMITER" );
    break;
  case NAL_UNIT_END_OF_SEQUENCE:
    DTRACE_HEADER( "END OF SEQUENCE" );
    bTrailingBits = false;
    break;
  case NAL_UNIT_END_OF_STREAM:
    DTRACE_HEADER( "END OF STREAM" );
    bTrailingBits = false;
    break;
  case NAL_UNIT_PREFIX:
    DTRACE_HEADER( "PREFIX" );
    bTrailingBits = ( eNalRefIdc != NAL_REF_IDC_PRIORITY_LOWEST );
    break;
  case NAL_UNIT_CODED_SLICE:
  case NAL_UNIT_CODED_SLICE_DATAPART_A:
  case NAL_UNIT_CODED_SLICE_DATAPART_B:
  case NAL_UNIT_CODED_SLICE_DATAPART_C:
  case NAL_UNIT_CODED_SLICE_IDR:
  case NAL_UNIT_AUX_CODED_SLICE:
  case NAL_UNIT_CODED_SLICE_SCALABLE:
    DTRACE_NEWSLICE;
    break;
  case NAL_UNIT_RESERVED_16:
  case NAL_UNIT_RESERVED_17:
  case NAL_UNIT_RESERVED_18:
  case NAL_UNIT_RESERVED_21:
  case NAL_UNIT_RESERVED_22:
  case NAL_UNIT_RESERVED_23:
    DTRACE_HEADER( "RESERVED" );
    bTrailingBits = false;
    break;
  default:
    DTRACE_HEADER( "UNSPECIFIED" );
    bTrailingBits = false;
    break;
  }

  //===== init bit read buffer and read NAL unit header =====
  RNOK( xInitSODB( rcBinDataAccessor, bTrailingBits ) );
  RNOK( NalUnitHeader::read( *m_pcHeaderSymbolReadIf ) );
  m_bNalUnitInitialized = true;

  return Err::m_nOK;
}

ErrVal
NalUnitParser::closeNalUnit( Bool bCheckEndOfNalUnit )
{
  ROF( m_bInitialized );
  ROF( m_bNalUnitInitialized );
  ROT( bCheckEndOfNalUnit && m_pcBitReadBuffer->isValid() );
  m_bNalUnitInitialized = false;
  return Err::m_nOK;
}

ErrVal
NalUnitParser::xInitSODB( BinDataAccessor&  rcBinDataAccessor,
                          Bool              bTrailingBits )
{
  UChar*  pucBuffer       = rcBinDataAccessor.data();
  UInt    uiRBSPSize      = 0;
  UInt    uiBitsInPacket  = 0;

  //===== convert payload to RBSP =====
  for( UInt uiNumZeros = 0, uiReadOffset = 0; uiReadOffset < rcBinDataAccessor.size(); uiReadOffset++ )
  {
    if( uiNumZeros == 2 && pucBuffer[ uiReadOffset ] == 0x03 )
    {
      uiReadOffset++;
      uiNumZeros = 0;
    }
    
    pucBuffer[ uiRBSPSize++ ] = pucBuffer[ uiReadOffset ];
    
    if( pucBuffer[ uiReadOffset] == 0x00 )
    {
      uiNumZeros++;
    }
    else
    {
      uiNumZeros = 0;
    }
  }
  
  //===== determine bits in NAL unit except trailing bits =====
  uiBitsInPacket      = ( uiRBSPSize << 3 );
  UInt  uiLastBytePos = uiRBSPSize - 1;

  //----- remove zero bytes at the end -----
  {
    while( pucBuffer[ uiLastBytePos ] == 0x00 )
    {
      uiLastBytePos --;
    }
    uiBitsInPacket  -= ( ( uiRBSPSize - uiLastBytePos - 1 ) << 3 );
  }

  //----- remove trailing bits -----
  if( bTrailingBits )
  {
    UChar ucLastByte    = pucBuffer[ uiLastBytePos ];
    UInt  uiNumZeroBits = 0;
    while( ( ucLastByte & 0x01 ) == 0x00 )
    {
      ucLastByte    >>= 1;
      uiNumZeroBits ++;
    }
    uiBitsInPacket  -= uiNumZeroBits;
  }

  //===== init bit read buffer =====
  ULong*  pulBuffer = (ULong*)pucBuffer;
  RNOK( m_pcBitReadBuffer->initPacket( pulBuffer, uiBitsInPacket ) );
  return Err::m_nOK;
}


H264AVC_NAMESPACE_END
