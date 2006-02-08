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
#include "DecError.h"



H264AVC_NAMESPACE_BEGIN

BitReadBuffer::BitReadBuffer():
  m_uiDWordsLeft( 0 ),
  m_uiBitsLeft( 0 ),
  m_iValidBits( 0 ),
  m_ulCurrentBits( 0xdeaddead ),
  m_uiNextBits( 0xdeaddead ),
  m_pulStreamPacket( 0 )
{
}


BitReadBuffer::~BitReadBuffer()
{
}



ErrVal BitReadBuffer::create( BitReadBuffer*& rpcBitReadBuffer )
{
  rpcBitReadBuffer = new BitReadBuffer;

  ROT( NULL == rpcBitReadBuffer );

  return Err::m_nOK;
}


ErrVal BitReadBuffer::destroy()
{
  delete this;

  return Err::m_nOK;
}




ErrVal BitReadBuffer::initPacket( ULong* puiBits, UInt uiBitsInPacket )
{
  // invalidate all members if something is wrong
  m_pulStreamPacket    = NULL;
  m_ulCurrentBits      = 0xdeaddead;
  m_uiNextBits         = 0xdeaddead;
  m_uiBitsLeft         = 0;
  m_iValidBits         = 0;
  m_uiDWordsLeft     = 0;


  // check the parameter
  ROT( uiBitsInPacket < 1);
  ROT( NULL == puiBits );

  // now init the Bitstream object
  m_pulStreamPacket  = puiBits;

  m_uiBitsLeft = uiBitsInPacket;

  m_uiDWordsLeft = m_uiBitsLeft >> 5;
  m_iValidBits     = -32;

  // preload first two dwords if valid
  xReadNextWord();
  xReadNextWord();

  return Err::m_nOK;
}



Void BitReadBuffer::show( UInt& ruiBits, UInt uiNumberOfBits  )
{
  UInt ui_right_shift;

  // check the number_of_bits parameter matches the range
  AOF_DBG( uiNumberOfBits <= 32 );

  if( (Int32) uiNumberOfBits <= m_iValidBits )
  {
    // calculate the number of bits to extract the desired number of bits
    ui_right_shift = 32 - uiNumberOfBits ;

    // mask out the value
    ruiBits  = m_ulCurrentBits >> ui_right_shift;
  }
  else
  {
    // mask out the value in the current word
    ruiBits = m_ulCurrentBits;

    // mask out the value in the next word
    ruiBits |= m_uiNextBits >> m_iValidBits;

    // mask out the value
    ruiBits >>= 32 - uiNumberOfBits;
  }

}



ErrVal BitReadBuffer::flush( UInt uiNumberOfBits  )
{
  // check the number_of_bits parameter matches the range
  AOF_DBG( uiNumberOfBits <= 32 );

  DECROTR( uiNumberOfBits > m_uiBitsLeft, Err::m_nEndOfBuffer );

  // sub the desired number of bits
  m_uiBitsLeft -= uiNumberOfBits;
  m_iValidBits -= uiNumberOfBits;

  // check the current word for beeing still valid
  if( 0 < m_iValidBits )
  {
    m_ulCurrentBits <<= uiNumberOfBits;
    return Err::m_nOK;
  }

  xReadNextWord();

  // shift to the right position
  m_ulCurrentBits <<= 32 - m_iValidBits;

  return Err::m_nOK;
}



ErrVal BitReadBuffer::get( UInt& ruiBits  )
{
  if( 0 == m_uiBitsLeft )
  {
    throw ReadStop();
  }

  m_uiBitsLeft --;
  m_iValidBits --;

  // mask out the value
  ruiBits  = m_ulCurrentBits >> 31;

  //prepare for next access
  m_ulCurrentBits <<= 1;

  // check the current word for beeing empty
  ROTRS( 0 < m_iValidBits, Err::m_nOK );

  xReadNextWord();

  return Err::m_nOK;
}


ErrVal BitReadBuffer::get( UInt& ruiBits, UInt uiNumberOfBits  )
{
  UInt ui_right_shift;

  // check the number_of_bits parameter matches the range
  AOT_DBG( uiNumberOfBits > 32 );

  if( uiNumberOfBits > m_uiBitsLeft )
  {
    throw ReadStop();
  }

  m_uiBitsLeft  -= uiNumberOfBits;
  m_iValidBits -= uiNumberOfBits;

  if( 0 <= m_iValidBits )
  {
    // calculate the number of bits to extract the desired number of bits
    ui_right_shift = 32 - uiNumberOfBits ;

    // mask out the value
    ruiBits  = m_ulCurrentBits >> ui_right_shift;

    //prepare for next access
    m_ulCurrentBits = m_ulCurrentBits << uiNumberOfBits;
  }
  else
  {
    // mask out the value in the current word
    ruiBits = m_ulCurrentBits;

    // calculate the number of bits to extract the desired number of bits
    ui_right_shift = m_iValidBits + uiNumberOfBits ;

    // mask out the value in the next word
    ruiBits |= m_uiNextBits >> ui_right_shift;

    ruiBits >>= 32 - uiNumberOfBits;

    m_uiNextBits <<=  -m_iValidBits;
  }

  // check the current word for beeing empty
  ROTRS( 0 < m_iValidBits, Err::m_nOK );

  xReadNextWord();

  return Err::m_nOK;
}



ErrVal BitReadBuffer::samples( Pel* pPel, UInt uiNumberOfSamples )
{
  AOF_DBG( isByteAligned() );
  // can be done in a faster way
  for( UInt n = 0; n < uiNumberOfSamples; n++)
  {
    UInt uiTemp;
    DECRNOK( get( uiTemp, 8) );
    pPel[n] = uiTemp;
  }
  return Err::m_nOK;
}




Bool BitReadBuffer::isValid()
{
  return ( m_uiBitsLeft > 1 );
}

__inline Void BitReadBuffer::xReadNextWord()
{
  m_ulCurrentBits = m_uiNextBits;
  m_iValidBits += 32;

  // chech if there are bytes left in the packet
  if( m_uiDWordsLeft )
  {
    // read 32 bit from the packet
    m_uiNextBits = xSwap( *m_pulStreamPacket++ );
    m_uiDWordsLeft--;
  }
  else
  {
    Int iBytesLeft  = ((Int)m_uiBitsLeft - m_iValidBits+7) >> 3;
    UChar* puc      = (UChar*) m_pulStreamPacket;
    m_uiNextBits  = 0;

    if( iBytesLeft > 0)
    {
      for( Int iByte = 0; iByte < iBytesLeft; iByte++ )
      {
        m_uiNextBits <<= 8;
        m_uiNextBits += puc[iByte];
      }
      m_uiNextBits <<= (4-iBytesLeft)<<3;
    }
  }
}


H264AVC_NAMESPACE_END
