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

H264AVC_NAMESPACE_BEGIN


BitWriteBuffer::BitWriteBuffer():
  m_uiDWordsLeft    ( 0 ),
  m_uiBitsWritten   ( 0 ),
  m_iValidBits      ( 0 ),
  m_ulCurrentBits   ( 0 ),
  m_pulStreamPacket ( NULL )
{
}

BitWriteBuffer::~BitWriteBuffer()
{
}


ErrVal BitWriteBuffer::init()
{
  m_uiDWordsLeft    = 0;
  m_uiBitsWritten   = 0;
  m_iValidBits      = 0;
  m_ulCurrentBits   = 0;
  m_pulStreamPacket = NULL;
  return Err::m_nOK;
}

ErrVal BitWriteBuffer::loadContent( BitWriteBuffer& rcBitWriteBuffer )
{
  m_uiDWordsLeft    = rcBitWriteBuffer.m_uiDWordsLeft;
  m_uiBitsWritten   = rcBitWriteBuffer.m_uiBitsWritten;
  m_iValidBits      = rcBitWriteBuffer.m_iValidBits;
  m_ulCurrentBits   = rcBitWriteBuffer.m_ulCurrentBits;
  m_pulStreamPacket = rcBitWriteBuffer.m_pulStreamPacket;
  return Err::m_nOK;
}


ErrVal BitWriteBuffer::create( BitWriteBuffer*& rpcBitWriteBuffer )
{
  rpcBitWriteBuffer = new BitWriteBuffer;

  ROT( NULL == rpcBitWriteBuffer );

  return Err::m_nOK;
}

ErrVal BitWriteBuffer::destroy()
{
  delete this;

  return Err::m_nOK;
}


ErrVal BitWriteBuffer::initPacket( ULong* pulBits, UInt uiPacketLength )
{
  // invalidate all members if something is wrong
  uninit();

  // check the parameter
  ROT( uiPacketLength < 4);
  ROT( NULL == pulBits );

  // now init the Bitstream object
  m_pulStreamPacket = pulBits;
  m_uiDWordsLeft = (uiPacketLength + 3) / 4;
  m_iValidBits    = 32;
  return Err::m_nOK;

}
//FIX_FRAG_CAVLC
ErrVal BitWriteBuffer::getLastByte(UChar &uiLastByte, UInt &uiLastBitPos)
{
  UInt uiBytePos = m_iValidBits/8;
  if(m_iValidBits%8 != 0)
  {
    uiBytePos = uiBytePos*8;
    uiLastByte = (UChar)(m_ulCurrentBits >> uiBytePos);
    uiLastBitPos = (m_iValidBits/8+1)*8-m_iValidBits;
    uiLastByte = (uiLastByte >> (8-uiLastBitPos));
  }
  else
  {
    uiLastByte = 0;
    uiLastBitPos = 0;
  }
  return Err::m_nOK;
}
//~FIX_FRAG_CAVLC
ErrVal BitWriteBuffer::write( UInt uiBits, UInt uiNumberOfBits )
{
  AOF_DBG( uiNumberOfBits <= 32 );

  AOF_DBG( ! ( (uiBits >> 1) >> (uiNumberOfBits - 1)) ); // because shift with 32 has no effect

  m_uiBitsWritten += uiNumberOfBits;

  if( (Int)uiNumberOfBits < m_iValidBits)  // one word
  {
    m_iValidBits -= uiNumberOfBits;

    m_ulCurrentBits |= uiBits << m_iValidBits;

    return Err::m_nOK;
  }


  ROT( 0 == m_uiDWordsLeft );
  m_uiDWordsLeft--;

  UInt uiShift = uiNumberOfBits - m_iValidBits;

  // add the last bits
  m_ulCurrentBits |= uiBits >> uiShift;

  *m_pulStreamPacket++ = xSwap( m_ulCurrentBits );


  // note: there is a problem with left shift with 32
  m_iValidBits = 32 - uiShift;

  m_ulCurrentBits = uiBits << m_iValidBits;

  if( 0 == uiShift )
  {
    m_ulCurrentBits = 0;
  }


  return Err::m_nOK;
}


ErrVal BitWriteBuffer::writeAlignZero()
{
  return write( 0, m_iValidBits & 0x7 );
}

ErrVal BitWriteBuffer::writeAlignOne()
{
  return write( ( 1 << (m_iValidBits & 0x7) ) - 1, m_iValidBits & 0x7 );
}


ErrVal BitWriteBuffer::flushBuffer()
{
  ROT( 0 == m_uiDWordsLeft );

  *m_pulStreamPacket = xSwap( m_ulCurrentBits );

  m_uiBitsWritten = (m_uiBitsWritten+7)/8;

  m_uiBitsWritten *= 8;

  return Err::m_nOK;
}



ErrVal BitWriteBuffer::samples( const Pel* pPel, UInt uiNumberOfSamples )
{
  // can be done in a faster way
  for( UInt n = 0; n < uiNumberOfSamples; n++)
  {
    RNOK( write( pPel[n], 8) );
  }
  return Err::m_nOK;
}


H264AVC_NAMESPACE_END

