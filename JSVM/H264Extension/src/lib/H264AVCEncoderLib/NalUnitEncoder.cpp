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



#include "H264AVCEncoderLib.h"
#include "BitWriteBuffer.h"
#include "NalUnitEncoder.h"


H264AVC_NAMESPACE_BEGIN


NalUnitEncoder::NalUnitEncoder()
: m_bIsUnitActive         ( false )
, m_pcBitWriteBuffer      ( 0 )
, m_pcHeaderSymbolWriteIf ( 0 )
, m_pcHeaderSymbolTestIf  ( 0 )
, m_pcBinDataAccessor     ( 0 )
, m_pucBuffer             ( 0 )
, m_pucTempBuffer         ( 0 )
, m_pucTempBufferBackup   ( 0 )
, m_uiPacketLength        ( MSYS_UINT_MAX )
, m_eNalUnitType          ( NAL_UNIT_EXTERNAL )
{
}


NalUnitEncoder::~NalUnitEncoder()
{
}


ErrVal
NalUnitEncoder::create( NalUnitEncoder*& rpcNalUnitEncoder )
{
  rpcNalUnitEncoder = new NalUnitEncoder;
  ROT( NULL == rpcNalUnitEncoder );
  return Err::m_nOK;
}


ErrVal
NalUnitEncoder::init( BitWriteBuffer*       pcBitWriteBuffer,
                      HeaderSymbolWriteIf*  pcHeaderSymbolWriteIf,
                      HeaderSymbolWriteIf*  pcHeaderSymbolTestIf )
{
  ROT( NULL == pcBitWriteBuffer );
  ROT( NULL == pcHeaderSymbolWriteIf );
  ROT( NULL == pcHeaderSymbolTestIf );

  m_pcBitWriteBuffer      = pcBitWriteBuffer;
  m_pcHeaderSymbolTestIf  = pcHeaderSymbolTestIf;
  m_pcHeaderSymbolWriteIf = pcHeaderSymbolWriteIf;
  m_bIsUnitActive         = false;
  m_pucBuffer             = NULL;
  m_pucTempBuffer         = NULL;
  m_pucTempBufferBackup   = NULL;
  m_uiPacketLength        = MSYS_UINT_MAX;
  m_eNalUnitType          = NAL_UNIT_EXTERNAL;

  return Err::m_nOK;
}


ErrVal
NalUnitEncoder::uninit()
{
  delete [] m_pucTempBuffer;
  delete [] m_pucTempBufferBackup;
  m_pucTempBufferBackup   = NULL;

  m_pcBitWriteBuffer      = NULL;
  m_pcHeaderSymbolWriteIf = NULL;
  m_pcHeaderSymbolTestIf  = NULL;
  m_bIsUnitActive         = false;
  m_pucBuffer             = NULL;
  m_pucTempBuffer         = NULL;
  m_uiPacketLength        = MSYS_UINT_MAX;
  m_eNalUnitType          = NAL_UNIT_EXTERNAL;

  return Err::m_nOK;
}


ErrVal
NalUnitEncoder::destroy()
{
  uninit();
  delete this;
  return Err::m_nOK;
}


ErrVal
NalUnitEncoder::initNalUnit( BinDataAccessor* pcBinDataAccessor )
{
  ROT( m_bIsUnitActive );
  ROF( pcBinDataAccessor );
  ROT( pcBinDataAccessor->size() < 1 );

  m_bIsUnitActive     = true;
  m_pcBinDataAccessor = pcBinDataAccessor;
  m_pucBuffer         = pcBinDataAccessor->data();

  if( m_uiPacketLength != m_pcBinDataAccessor->size() )
  {
    delete [] m_pucTempBuffer;

    m_uiPacketLength = m_pcBinDataAccessor->size();
    m_pucTempBuffer  = new UChar[ m_uiPacketLength ];
    ROF( m_pucTempBuffer );
  }

  RNOK( m_pcBitWriteBuffer->initPacket( (ULong*)(m_pucTempBuffer), m_uiPacketLength-1 ) );

  return Err::m_nOK;
}



ErrVal
NalUnitEncoder::terminateMultiFragments ( UInt& ruiBits )
{
  RNOK ( xWriteTrailingBits() );
  RNOK( m_pcBitWriteBuffer->flushBuffer() );

  ruiBits = m_pcBitWriteBuffer->getNumberOfWrittenBits();
  ruiBits = ( ruiBits >> 3 ) + ( 0 != ( ruiBits & 0x07 ) );

  ruiBits <<= 3;

  if( m_pucTempBufferBackup == 0 )
  {
    m_pucTempBufferBackup  = new UChar[ m_uiPacketLength ];
  }

  memcpy( m_pucTempBufferBackup, m_pucTempBuffer, m_uiPacketLength );

  return Err::m_nOK;
}


// called this function after fragment header is written
// and only for closing FGS fragment
ErrVal
NalUnitEncoder::attachFragmentData( UInt& ruiBits,
                                    UInt  uiStartPos,
                                    UInt  uiEndPos )
{
  UInt  uiBits        = uiEndPos - uiStartPos;
  ROF( m_bIsUnitActive );

  if( uiStartPos > 0 )
  {
    // not the first segment, copy the data from the backup buffer
    RNOK( m_pcBitWriteBuffer->writeAlignOne() );
    RNOK( m_pcBitWriteBuffer->flushBuffer()   );
    
    uiBits  = m_pcBitWriteBuffer->getNumberOfWrittenBits();
    uiBits  = ( uiBits >> 3 ) + ( 0 != ( uiBits & 0x07 ) );
    memcpy( m_pucTempBuffer + uiBits, m_pucTempBufferBackup + uiStartPos, uiEndPos - uiStartPos );

    uiBits += uiEndPos - uiStartPos;
  }
  else 
    uiBits  = uiEndPos;

  //===== convert to payload and add header =====
  UInt  uiHeaderBytes = 2;
  RNOK( xConvertRBSPToPayload( uiBits, uiHeaderBytes ) );
  RNOK( m_pcBinDataAccessor->decreaseEndPos( m_pcBinDataAccessor->size() - uiBits ) );
  ruiBits             = 8*uiBits;

  //==== reset parameters =====
  m_bIsUnitActive     = false;
  m_pucBuffer         = NULL;
  m_pcBinDataAccessor = NULL;
  m_eNalUnitType      = NAL_UNIT_EXTERNAL;

  return Err::m_nOK;
}

ErrVal
NalUnitEncoder::closeNalUnit( UInt& ruiBits )
{
  ROF( m_bIsUnitActive );

  //===== write trailing bits =====
  if( NAL_UNIT_END_OF_SEQUENCE != m_eNalUnitType &&
      NAL_UNIT_END_OF_STREAM   != m_eNalUnitType )
  {
    RNOK ( xWriteTrailingBits() );
  }
  RNOK( m_pcBitWriteBuffer->flushBuffer() );

  //===== convert to payload and add header =====
  Bool  bDDIPresent   = ( m_eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE ||
                          m_eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE );
  UInt  uiHeaderBytes = ( bDDIPresent ? 2 : 1 );
  UInt  uiBits        = m_pcBitWriteBuffer->getNumberOfWrittenBits();
  uiBits              = ( uiBits >> 3 ) + ( 0 != ( uiBits & 0x07 ) );
  RNOK( xConvertRBSPToPayload( uiBits, uiHeaderBytes ) );
  RNOK( m_pcBinDataAccessor->decreaseEndPos( m_pcBinDataAccessor->size() - uiBits ) );
  ruiBits             = 8*uiBits;

  //==== reset parameters =====
  m_bIsUnitActive     = false;
  m_pucBuffer         = NULL;
  m_pcBinDataAccessor = NULL;
  m_eNalUnitType      = NAL_UNIT_EXTERNAL;

  return Err::m_nOK;
}


ErrVal
NalUnitEncoder::xConvertRBSPToPayload( UInt& ruiBytesWritten,
                                       UInt  uiHeaderBytes )
{
  UInt uiZeroCount    = 0;
  UInt uiReadOffset   = uiHeaderBytes;
  UInt uiWriteOffset  = uiHeaderBytes;

  //===== NAL unit header =====
  for( UInt uiIndex = 0; uiIndex < uiHeaderBytes; uiIndex++ )
  {
    m_pucBuffer[uiIndex] = m_pucTempBuffer[uiIndex];
  }

  //===== NAL unit payload =====
  for( ; uiReadOffset < ruiBytesWritten ; uiReadOffset++, uiWriteOffset++ )
  {
    ROT( uiWriteOffset >= m_uiPacketLength );

    if( 2 == uiZeroCount && 0 == ( m_pucTempBuffer[uiReadOffset] & 0xfc ) )
    {
      uiZeroCount                   = 0;
      m_pucBuffer[uiWriteOffset++]  = 0x03;
    }

    m_pucBuffer[uiWriteOffset] = m_pucTempBuffer[uiReadOffset];

    if( 0 == m_pucTempBuffer[uiReadOffset] )
    {
      uiZeroCount++;
    }
    else
    {
      uiZeroCount = 0;
    }
  }
  if( ( 0x00 == m_pucBuffer[uiWriteOffset-1] ) && ( 0x00 == m_pucBuffer[uiWriteOffset-2] ) )
  {
    ROT( uiWriteOffset >= m_uiPacketLength );
    m_pucBuffer[uiWriteOffset++] = 0x03;
  }
  ruiBytesWritten = uiWriteOffset;

  return Err::m_nOK;
}


ErrVal
NalUnitEncoder::xWriteTrailingBits( UInt uiFixedNumberOfBits )
{
  if( uiFixedNumberOfBits )
  {
    RNOK( m_pcBitWriteBuffer->write( 1 << ( uiFixedNumberOfBits - 1 ), uiFixedNumberOfBits ) );
    return Err::m_nOK;
  }

  RNOK( m_pcBitWriteBuffer->write( 1 ) );
  RNOK( m_pcBitWriteBuffer->writeAlignZero() );

  return Err::m_nOK;
}


ErrVal
NalUnitEncoder::write( const SequenceParameterSet& rcSPS )
{
  RNOK( rcSPS.write( m_pcHeaderSymbolWriteIf ) );

  m_eNalUnitType  = rcSPS.getNalUnitType();
  return Err::m_nOK;
}


ErrVal
NalUnitEncoder::write( const PictureParameterSet& rcPPS )
{
  RNOK( rcPPS.write( m_pcHeaderSymbolWriteIf ) );

  m_eNalUnitType  = rcPPS.getNalUnitType();
  return Err::m_nOK;
}


ErrVal
NalUnitEncoder::write( const SliceHeader& rcSH )
{
  RNOK( rcSH.write( m_pcHeaderSymbolWriteIf ) );

  m_eNalUnitType  = rcSH.getNalUnitType();
  return Err::m_nOK;
}


ErrVal
NalUnitEncoder::write( SEI::MessageList& rcSEIMessageList )
{
  RNOK( SEI::write( m_pcHeaderSymbolWriteIf, m_pcHeaderSymbolTestIf, &rcSEIMessageList ) );

  m_eNalUnitType  = NAL_UNIT_SEI;
  return Err::m_nOK;
}

// JVT-T073 {
ErrVal
NalUnitEncoder::writeNesting( SEI::MessageList& rcSEIMessageList )
{
  RNOK( SEI::writeNesting( m_pcHeaderSymbolWriteIf, m_pcHeaderSymbolTestIf, &rcSEIMessageList ) );
  m_eNalUnitType = NAL_UNIT_SEI;
  return Err::m_nOK;
}
// JVT-T073 }


H264AVC_NAMESPACE_END
