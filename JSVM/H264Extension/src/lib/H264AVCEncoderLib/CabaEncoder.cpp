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
#include "H264AVCCommonLib/CabacTables.h"
#include "H264AVCCommonLib/CabacContextModel.h"
#include "H264AVCCommonLib/TraceFile.h"
#include "CabaEncoder.h"



H264AVC_NAMESPACE_BEGIN


CabaEncoder::CabaEncoder() :
  m_pcBitWriteBufferIf( NULL ),
  m_uiRange( 0 ),
  m_uiLow( 0 ),
  m_uiByte( 0 ),
  m_uiBitsLeft( 0 ),
  m_uiBitsToFollow( 0 ),
  m_bTraceEnable(true)
{
}

CabaEncoder::~CabaEncoder()
{
}


__inline ErrVal CabaEncoder::xWriteBit( UInt uiBit)
{
  m_uiByte += m_uiByte + uiBit;
  if( ! --m_uiBitsLeft )
  {
    const UInt uiByte = m_uiByte;
    m_uiBitsLeft = 8;
    m_uiByte     = 0;
    return m_pcBitWriteBufferIf->write( uiByte, 8);
  }
  return Err::m_nOK;
}


__inline ErrVal CabaEncoder::xWriteBitAndBitsToFollow( UInt uiBit)
{
  RNOK( xWriteBit( uiBit ) );
  // invert bit
  uiBit = 1-uiBit;

	while( m_uiBitsToFollow > 0)
	{
		m_uiBitsToFollow--;
    RNOK( xWriteBit( uiBit ) );
	}
  return Err::m_nOK;
}


ErrVal CabaEncoder::init( BitWriteBufferIf* pcBitWriteBufferIf )
{
  ROT( NULL == pcBitWriteBufferIf )

  m_pcBitWriteBufferIf = pcBitWriteBufferIf;

  return Err::m_nOK;
}


ErrVal CabaEncoder::start()
{
  m_uiRange = HALF-2;
  m_uiLow = 0;
  m_uiBitsToFollow = 0;
  m_uiByte = 0;
  m_uiBitsLeft = 9;

  RNOK( m_pcBitWriteBufferIf->writeAlignOne() );
  return Err::m_nOK;
}

//JVT-P031
ErrVal CabaEncoder::startFragment()
{
  RNOK( m_pcBitWriteBufferIf->writeAlignOne() );
  return Err::m_nOK;
}
//~JVT-P031
//FIX_FRAG_CAVLC
ErrVal CabaEncoder::getLastByte(UChar &uiLastByte, UInt &uiLastBitPos)
{
  RNOK(m_pcBitWriteBufferIf->getLastByte(uiLastByte, uiLastBitPos));
  return Err::m_nOK;
}
ErrVal CabaEncoder::setFirstBits(UChar ucByte,UInt uiLastBitPos)
{
  RNOK( m_pcBitWriteBufferIf->write(ucByte,uiLastBitPos));
  return Err::m_nOK;
}
//~FIX_FRAG_CAVLC
ErrVal CabaEncoder::uninit()
{
  m_pcBitWriteBufferIf = NULL;
  m_uiRange = 0;
  return Err::m_nOK;
}


ErrVal CabaEncoder::writeUnaryMaxSymbol( UInt uiSymbol, CabacContextModel* pcCCModel, Int iOffset, UInt uiMaxSymbol )
{
  RNOK( writeSymbol( uiSymbol ? 1 : 0, pcCCModel[ 0 ] ) );

  ROTRS( 0 == uiSymbol, Err::m_nOK );

  Bool bCodeLast = ( uiMaxSymbol > uiSymbol );

  while( --uiSymbol )
  {
    RNOK( writeSymbol( 1, pcCCModel[ iOffset ] ) );
  }
  if( bCodeLast )
  {
    RNOK( writeSymbol( 0, pcCCModel[ iOffset ] ) );
  }

  return Err::m_nOK;
}


ErrVal CabaEncoder::writeExGolombLevel( UInt uiSymbol, CabacContextModel& rcCCModel  )
{
  if( uiSymbol )
  {
    RNOK( writeSymbol( 1, rcCCModel ) );
    UInt uiCount = 0;
    Bool bNoExGo = (uiSymbol < 13);

    while( --uiSymbol && ++uiCount < 13 )
    {
      RNOK( writeSymbol( 1, rcCCModel ) );
    }
    if( bNoExGo )
    {
      RNOK( writeSymbol( 0, rcCCModel ) );
    }
    else
    {
      RNOK( writeEpExGolomb( uiSymbol, 0 ) );
    }
  }
  else
  {
    RNOK( writeSymbol( 0, rcCCModel ) );
  }

  return Err::m_nOK;
}


ErrVal CabaEncoder::writeEpExGolomb( UInt uiSymbol, UInt uiCount )
{
  while( uiSymbol >= (UInt)(1<<uiCount) )
  {
    writeEPSymbol( 1 );
    uiSymbol -= 1<<uiCount;
    uiCount  ++;
  }
  writeEPSymbol( 0 );
  while( uiCount-- )
  {
    writeEPSymbol( (uiSymbol>>uiCount) & 1 );
  }

  return Err::m_nOK;
}


ErrVal CabaEncoder::writeExGolombMvd( UInt uiSymbol, CabacContextModel* pcCCModel, UInt uiMaxBin )
{
  if( ! uiSymbol )
  {
    RNOK( writeSymbol( 0, *pcCCModel ) );
    return Err::m_nOK;
  }

  RNOK( writeSymbol( 1, *pcCCModel ) );

  Bool  bNoExGo = ( uiSymbol < 8 );
  UInt  uiCount = 1;
  pcCCModel++;

  while( --uiSymbol && ++uiCount <= 8 )
  {
    RNOK( writeSymbol( 1, *pcCCModel ) );
    if( uiCount == 2 )
    {
      pcCCModel++;
    }
    if( uiCount == uiMaxBin )
    {
      pcCCModel++;
    }
  }

  if( bNoExGo )
  {
    RNOK( writeSymbol( 0, *pcCCModel ) );
  }
  else
  {
    RNOK( writeEpExGolomb( uiSymbol, 3 ) );
  }

  return Err::m_nOK;
}



ErrVal CabaEncoder::writeUnarySymbol( UInt uiSymbol, CabacContextModel* pcCCModel, Int iOffset )
{
  RNOK( writeSymbol( uiSymbol ? 1 : 0, pcCCModel[0] ) );

  ROTRS( 0 == uiSymbol, Err::m_nOK );

  while( uiSymbol-- )
  {
    RNOK( writeSymbol( uiSymbol ? 1 : 0, pcCCModel[ iOffset ] ) );
  }

  return Err::m_nOK;
}


ErrVal CabaEncoder::finish()
{
  RNOK( xWriteBitAndBitsToFollow( (m_uiLow >> (B_BITS-1)) & 1 ) );
  RNOK( xWriteBit(                (m_uiLow >> (B_BITS-2)) & 1 ) );

  RNOK( m_pcBitWriteBufferIf->write( m_uiByte, 8 - m_uiBitsLeft ) );

  return Err::m_nOK;
}


ErrVal CabaEncoder::writeSymbol( UInt uiSymbol, CabacContextModel& rcCCModel )
{
  ETRACE_V (g_nSymbolCounter[g_nLayer]++);
  ETRACE_T ("  ");
  ETRACE_X (m_uiRange);
  ETRACE_T ("  ");
  ETRACE_V (rcCCModel.getState());
  ETRACE_T ("  ");
  ETRACE_V (rcCCModel.getMps());
  ETRACE_T ("  -  ");
  ETRACE_V (uiSymbol);
  ETRACE_N;

	UInt uiLow    = m_uiLow;
	UInt uiRange  = m_uiRange;
	UInt uiLPS = g_aucLPSTable64x4[rcCCModel.getState()][(uiRange>>6) & 3];

  AOT_DBG( 1 < uiSymbol );

  rcCCModel.incrementCount();

  uiRange -= uiLPS;
  if( uiSymbol != rcCCModel.getMps() )
  {
    uiLow += uiRange;
    uiRange = uiLPS;

    if( ! rcCCModel.getState() )
    {
      rcCCModel.toggleMps();
    }
    rcCCModel.setState( g_aucACNextStateLPS64[rcCCModel.getState()] );
  }
  else
  {
    rcCCModel.setState( g_aucACNextStateMPS64[rcCCModel.getState()] );
  }

	while( uiRange < QUARTER )
  {
		if( uiLow >= HALF )
    {
      RNOK( xWriteBitAndBitsToFollow( 1 ) );
      uiLow -= HALF;
		}
    else if( uiLow < QUARTER )
    {
      RNOK( xWriteBitAndBitsToFollow( 0 ) );
    }
    else
    {
			m_uiBitsToFollow++;
      uiLow -= QUARTER;
    }
    uiLow   <<= 1;
    uiRange <<= 1;
  }

  m_uiLow   = uiLow;
  m_uiRange = uiRange;

  return Err::m_nOK;
}


ErrVal CabaEncoder::writeEPSymbol( UInt uiSymbol )
{
  ETRACE_V(g_nSymbolCounter[g_nLayer]++);
  ETRACE_T ("  ");
  ETRACE_X(m_uiRange);
  ETRACE_T ("  -  ");
  ETRACE_V (uiSymbol);
  ETRACE_N;

  UInt uiLow = m_uiLow<<1;

  if( uiSymbol != 0 )
  {
    uiLow += m_uiRange;
  }

	if (uiLow >= ONE)
	{
    RNOK( xWriteBitAndBitsToFollow( 1 ) );
    uiLow -= ONE;
	}
	else if (uiLow < HALF)
	{
    RNOK( xWriteBitAndBitsToFollow( 0 ) );
	}
	else
	{
		m_uiBitsToFollow++;
		uiLow -= HALF;
	}

  m_uiLow = uiLow;

  return Err::m_nOK;
}


ErrVal CabaEncoder::writeTerminatingBit( UInt uiBit )
{
  ETRACE_V(g_nSymbolCounter[g_nLayer]++);
  ETRACE_T ("  ");
  ETRACE_X(m_uiRange);
  ETRACE_T ("  -  ");
  ETRACE_V (uiBit);
  ETRACE_N;

  UInt uiRange = m_uiRange - 2;
  UInt uiLow   = m_uiLow;

  if( uiBit )
  {
 		uiLow += uiRange;
    uiRange = 2;
  }

	while( uiRange < QUARTER )
  {
		if( uiLow >= HALF )
    {
      RNOK( xWriteBitAndBitsToFollow( 1 ) );
      uiLow -= HALF;
		}
    else if( uiLow < QUARTER )
		{
      RNOK( xWriteBitAndBitsToFollow( 0 ) );
		}
		else
		{
      m_uiBitsToFollow++;
			uiLow -= QUARTER;
		}
    uiLow   <<= 1;
    uiRange <<= 1;
  }

  m_uiRange = uiRange;
	m_uiLow   = uiLow;

  return Err::m_nOK;
}


H264AVC_NAMESPACE_END
