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
#include "H264AVCCommonLib/CabacTables.h"
#include "H264AVCCommonLib/TraceFile.h"
#include "H264AVCCommonLib/CabacContextModel.h"
#include "BitReadBuffer.h"
#include "CabaDecoder.h"
#include "DecError.h"


#if 0 // FAST_CABAC
#define RNOKCABAC( exp ) exp
#define ROTRSCABAC( exp, err ) ROTVS(exp)
#define ROFRSCABAC( exp, err ) ROFVS(exp)
#else
#define RNOKCABAC( exp )       RNOK(exp)
#define ROTRSCABAC( exp, err ) ROTRS(exp,err)
#define ROFRSCABAC( exp, err ) ROFRS(exp,err)
#endif


H264AVC_NAMESPACE_BEGIN


CabaDecoder::CabaDecoder() :
  m_pcBitReadBuffer( NULL ),
  m_uiRange( 0 ),
  m_uiValue( 0 ),
  m_uiWord( 0 ),
  m_uiBitsLeft( 0 )
{
}


CabaDecoder::~CabaDecoder()
{

}


ErrVal CabaDecoder::init( BitReadBuffer* pcBitReadBuffer )
{
  ROT( NULL == pcBitReadBuffer )

  m_pcBitReadBuffer = pcBitReadBuffer;
  return Err::m_nOK;
}



__inline Void CabaDecoder::xReadBit( UInt& ruiValue )
{
  if( 0 == m_uiBitsLeft-- )
  {
    m_pcBitReadBuffer->get( m_uiWord, 8 );
    m_uiBitsLeft = 7;
  }
  ruiValue += ruiValue + ((m_uiWord >> 7)&1);
  m_uiWord <<= 1;
}



ErrVal CabaDecoder::finish()
{
  return Err::m_nOK;
}

ErrVal CabaDecoder::start()
{
  m_uiRange     = HALF-2;
  m_uiValue     = 0;
  m_uiWord      = 0;
  m_uiBitsLeft  = 0;


  RNOK( m_pcBitReadBuffer->flush( m_pcBitReadBuffer->getBitsUntilByteAligned() ) );
  m_pcBitReadBuffer->setModeCabac();

  while( ! m_pcBitReadBuffer->isWordAligned() && ( 8 > m_uiBitsLeft) )
  {
    UInt uiByte;
    m_pcBitReadBuffer->get( uiByte, 8 );
    m_uiWord <<= 8;
    m_uiWord += uiByte;
    m_uiBitsLeft += 8;
  }

  m_uiWord <<= 8-m_uiBitsLeft;

  for( UInt n = 0; n < B_BITS-1; n++ )
  {
    xReadBit( m_uiValue );
  }

  return Err::m_nOK;
}



ErrVal CabaDecoder::getTerminateBufferBit( UInt& ruiBit )
{
  UInt uiRange = m_uiRange-2;
  UInt uiValue = m_uiValue;

  DTRACE_V (g_nSymbolCounter[g_nLayer]++);
  DTRACE_T ("  ");
  DTRACE_X (m_uiRange);


  if( uiValue >= uiRange )
  {
    ruiBit = 1;
  }
  else
  {
    ruiBit = 0;

	  while( uiRange < QUARTER )
	  {
		  uiRange += uiRange;
      xReadBit( uiValue );
	  }

    m_uiRange = uiRange;
    m_uiValue = uiValue;
  }

  DTRACE_T ("  -  ");
  DTRACE_V (ruiBit);
  DTRACE_N;
  return Err::m_nOK;
}


ErrVal CabaDecoder::uninit()
{
  m_pcBitReadBuffer = NULL;
  m_uiRange = 0;
  m_uiValue = 0;
  return Err::m_nOK;
}




ErrVal CabaDecoder::getSymbol( UInt& ruiSymbol, CabacContextModel& rcCCModel )
{
  UInt uiRange = m_uiRange;
  UInt uiValue = m_uiValue;

  DTRACE_V (g_nSymbolCounter[g_nLayer]++);
  DTRACE_T ("  ");
  DTRACE_X (m_uiRange);
  DTRACE_T ("  ");
  DTRACE_V (rcCCModel.getState());
  DTRACE_T ("  ");
  DTRACE_V (rcCCModel.getMps());

  {

    UInt uiLPS;

    uiLPS = g_aucLPSTable64x4[rcCCModel.getState()][(uiRange>>6) & 0x03];
		uiRange -= uiLPS;

		if( uiValue < uiRange )
    {
			ruiSymbol = rcCCModel.getMps();
  		rcCCModel.setState( g_aucACNextStateMPS64[ rcCCModel.getState() ] );
    }
    else
    {
      uiValue -= uiRange;
      uiRange  = uiLPS;

			ruiSymbol = 1 - rcCCModel.getMps();

      if( ! rcCCModel.getState() )
      {
				rcCCModel.toggleMps();
      }

			rcCCModel.setState( g_aucACNextStateLPS64[ rcCCModel.getState() ] );
    }
  }

  DTRACE_T ("  -  ");
  DTRACE_V (ruiSymbol);
  DTRACE_N;

  while( uiRange < QUARTER )
  {
    uiRange += uiRange;
    xReadBit( uiValue );
  }

  m_uiRange = uiRange;
  m_uiValue = uiValue;

  return Err::m_nOK;
}


ErrVal CabaDecoder::getEpSymbol( UInt& ruiSymbol )
{
  DTRACE_V (g_nSymbolCounter[g_nLayer]++);
  DTRACE_T ("  ");
  DTRACE_X (m_uiRange);

  UInt uiValue = m_uiValue;

  xReadBit( uiValue );

	if( uiValue >= m_uiRange )
	{
		ruiSymbol = 1;
		uiValue -= m_uiRange;
	}
	else
  {
		ruiSymbol = 0;
  }

  DTRACE_T ("  -  ");
  DTRACE_V (ruiSymbol);
  DTRACE_N;

  m_uiValue = uiValue;

  return Err::m_nOK;
}




ErrVal CabaDecoder::getExGolombLevel( UInt& ruiSymbol, CabacContextModel& rcCCModel  )
{
  UInt uiSymbol;
  UInt uiCount = 0;
  do
  {
    RNOKCABAC( getSymbol( uiSymbol, rcCCModel ) );
    uiCount++;
  }
  while( uiSymbol && (uiCount != 13));

  ruiSymbol = uiCount-1;

	if( uiSymbol )
  {
    RNOKCABAC( getEpExGolomb( uiSymbol, 0 ) );
    ruiSymbol += uiSymbol+1;
  }

  return Err::m_nOK;
}



ErrVal CabaDecoder::getExGolombMvd( UInt& ruiSymbol, CabacContextModel* pcCCModel, UInt uiMaxBin )
{
  UInt uiSymbol;

  RNOKCABAC( getSymbol( ruiSymbol, pcCCModel[0] ) );

  ROTRSCABAC( 0 == ruiSymbol, Err::m_nOK );

  RNOKCABAC( getSymbol( uiSymbol, pcCCModel[1] ) );

  ruiSymbol = 1;

  ROTRSCABAC( 0 == uiSymbol, Err::m_nOK );

  pcCCModel += 2;
  UInt uiCount = 2;

  do
  {
    if( uiMaxBin == uiCount )
    {
      pcCCModel++;
    }
    RNOKCABAC( getSymbol( uiSymbol, *pcCCModel ) );
    uiCount++;
  }
  while( uiSymbol && (uiCount != 8));

  ruiSymbol = uiCount-1;

	if( uiSymbol )
  {
    RNOKCABAC( getEpExGolomb( uiSymbol, 3 ) );
    ruiSymbol += uiSymbol+1;
  }

  return Err::m_nOK;
}


ErrVal CabaDecoder::getEpExGolomb( UInt& ruiSymbol, UInt uiCount )
{
  UInt uiSymbol = 0;
  UInt uiBit = 1;


  while( uiBit )
  {
    RNOKCABAC( getEpSymbol( uiBit ) );
    uiSymbol += uiBit << uiCount++;
  }

  uiCount--;
	while( uiCount-- )
  {
    RNOKCABAC( getEpSymbol( uiBit ) );
  	uiSymbol += uiBit << uiCount;
  }

  ruiSymbol = uiSymbol;
  return Err::m_nOK;
}


ErrVal CabaDecoder::getUnaryMaxSymbol( UInt& ruiSymbol, CabacContextModel* pcCCModel, Int iOffset, UInt uiMaxSymbol )
{
  RNOKCABAC( getSymbol( ruiSymbol, pcCCModel[0] ) );

  ROTRSCABAC( 0 == ruiSymbol, Err::m_nOK );
  ROTRSCABAC( 1 == uiMaxSymbol, Err::m_nOK );

  UInt uiSymbol = 0;
  UInt uiCont;

  do
  {
    RNOKCABAC( getSymbol( uiCont, pcCCModel[ iOffset ] ) );
    uiSymbol++;
  }
  while( uiCont && (uiSymbol < uiMaxSymbol-1) );

  if( uiCont && (uiSymbol == uiMaxSymbol-1) )
  {
    uiSymbol++;
  }

  ruiSymbol = uiSymbol;
  return Err::m_nOK;
}


ErrVal CabaDecoder::getUnarySymbol( UInt& ruiSymbol, CabacContextModel* pcCCModel, Int iOffset )
{
  RNOKCABAC( getSymbol( ruiSymbol, pcCCModel[0] ) );

  ROTRSCABAC( 0 == ruiSymbol, Err::m_nOK );

  UInt uiSymbol = 0;
  UInt uiCont;

  do
  {
    RNOKCABAC( getSymbol( uiCont, pcCCModel[ iOffset ] ) );
    uiSymbol++;
  }
  while( uiCont );

  ruiSymbol = uiSymbol;
  return Err::m_nOK;
}

H264AVC_NAMESPACE_END

