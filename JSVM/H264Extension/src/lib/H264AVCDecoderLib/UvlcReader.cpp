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

#include "H264AVCCommonLib/TraceFile.h"
#include "H264AVCCommonLib/Tables.h"
#include "UvlcReader.h"
#include "BitReadBuffer.h"
#include "DecError.h"



H264AVC_NAMESPACE_BEGIN

UInt UvlcReader::m_auiShiftLuma[16];
UInt UvlcReader::m_auiShiftChroma[16];
UInt UvlcReader::m_auiBestCodeTab[16];

#define RUNBEFORE_NUM  7
#define MAX_VALUE  0xdead
#define TOTRUN_NUM    15
const UInt g_auiIncVlc[] = {0,3,6,12,24,48,32768};	// maximum vlc = 6

const UChar COEFF_COST[16] =
{
  3, 2,2,1, 1,1,0,0,0,0,0,0,0,0,0,0
};

const UChar g_aucLenTableTO4[4][5] =
{
  { 2, 6, 6, 6, 6,},
  { 0, 1, 6, 7, 8,},
  { 0, 0, 3, 7, 8,},
  { 0, 0, 0, 6, 7,},
};

const UChar g_aucCodeTableTO4[4][5] =
{
  {1,7,4,3,2},
  {0,1,6,3,3},
  {0,0,1,2,2},
  {0,0,0,5,0},
};

const UChar g_aucLenTableTZ4[3][4] =
{
  { 1, 2, 3, 3,},
  { 1, 2, 2, 0,},
  { 1, 1, 0, 0,},
};

const UChar g_aucCodeTableTZ4[3][4] =
{
  { 1, 1, 1, 0,},
  { 1, 1, 0, 0,},
  { 1, 0, 0, 0,},
};

const UChar g_aucLenTableTZ16[TOTRUN_NUM][16] =
{

  { 1,3,3,4,4,5,5,6,6,7,7,8,8,9,9,9},
  { 3,3,3,3,3,4,4,4,4,5,5,6,6,6,6},
  { 4,3,3,3,4,4,3,3,4,5,5,6,5,6},
  { 5,3,4,4,3,3,3,4,3,4,5,5,5},
  { 4,4,4,3,3,3,3,3,4,5,4,5},
  { 6,5,3,3,3,3,3,3,4,3,6},
  { 6,5,3,3,3,2,3,4,3,6},
  { 6,4,5,3,2,2,3,3,6},
  { 6,6,4,2,2,3,2,5},
  { 5,5,3,2,2,2,4},
  { 4,4,3,3,1,3},
  { 4,4,2,1,3},
  { 3,3,1,2},
  { 2,2,1},
  { 1,1},
};

const UChar g_aucCodeTableTZ16[TOTRUN_NUM][16] =
{
  {1,3,2,3,2,3,2,3,2,3,2,3,2,3,2,1},
  {7,6,5,4,3,5,4,3,2,3,2,3,2,1,0},
  {5,7,6,5,4,3,4,3,2,3,2,1,1,0},
  {3,7,5,4,6,5,4,3,3,2,2,1,0},
  {5,4,3,7,6,5,4,3,2,1,1,0},
  {1,1,7,6,5,4,3,2,1,1,0},
  {1,1,5,4,3,3,2,1,1,0},
  {1,1,1,3,3,2,2,1,0},
  {1,0,1,3,2,1,1,1,},
  {1,0,1,3,2,1,1,},
  {0,1,1,2,1,3},
  {0,1,1,1,1},
  {0,1,1,1},
  {0,1,1},
  {0,1},
};

const UChar g_aucCbpIntra[48] =
{
   47, 31, 15,  0,
   23, 27, 29, 30,
    7, 11, 13, 14,
   39, 43, 45, 46,
   16,  3,  5, 10,
   12, 19, 21, 26,
   28, 35, 37, 42,
   44,  1,  2,  4,
    8, 17, 18, 20,
   24,  6,  9, 22,
   25, 32, 33, 34,
   36, 40, 38, 41
};


const UChar g_aucCbpInter[48] =
{
    0, 16,  1,  2,
    4,  8, 32,  3,
    5, 10, 12, 15,
   47,  7, 11, 13,
   14,  6,  9, 31,
   35, 37, 42, 44,
   33, 34, 36, 40,
   39, 43, 45, 46,
   17, 18, 20, 24,
   19, 21, 26, 28,
   23, 27, 29, 30,
   22, 25, 38, 41
};

const UChar g_aucLenTableTO16[3][4][17] =
{
  {   // 0702
    { 1, 6, 8, 9,10,11,13,13,13,14,14,15,15,16,16,16,16},
    { 0, 2, 6, 8, 9,10,11,13,13,14,14,15,15,15,16,16,16},
    { 0, 0, 3, 7, 8, 9,10,11,13,13,14,14,15,15,16,16,16},
    { 0, 0, 0, 5, 6, 7, 8, 9,10,11,13,14,14,15,15,16,16},
  },
  {
    { 2, 6, 6, 7, 8, 8, 9,11,11,12,12,12,13,13,13,14,14},
    { 0, 2, 5, 6, 6, 7, 8, 9,11,11,12,12,13,13,14,14,14},
    { 0, 0, 3, 6, 6, 7, 8, 9,11,11,12,12,13,13,13,14,14},
    { 0, 0, 0, 4, 4, 5, 6, 6, 7, 9,11,11,12,13,13,13,14},
  },
  {
    { 4, 6, 6, 6, 7, 7, 7, 7, 8, 8, 9, 9, 9,10,10,10,10},
    { 0, 4, 5, 5, 5, 5, 6, 6, 7, 8, 8, 9, 9, 9,10,10,10},
    { 0, 0, 4, 5, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9,10,10,10},
    { 0, 0, 0, 4, 4, 4, 4, 4, 5, 6, 7, 8, 8, 9,10,10,10},
  },
};

const UChar g_aucCodeTableTO16[3][4][17] =
{
  {
    { 1, 5, 7, 7, 7, 7,15,11, 8,15,11,15,11,15,11, 7,4},
    { 0, 1, 4, 6, 6, 6, 6,14,10,14,10,14,10, 1,14,10,6},
    { 0, 0, 1, 5, 5, 5, 5, 5,13, 9,13, 9,13, 9,13, 9,5},
    { 0, 0, 0, 3, 3, 4, 4, 4, 4, 4,12,12, 8,12, 8,12,8},
  },
  {
    { 3,11, 7, 7, 7, 4, 7,15,11,15,11, 8,15,11, 7, 9,7},
    { 0, 2, 7,10, 6, 6, 6, 6,14,10,14,10,14,10,11, 8,6},
    { 0, 0, 3, 9, 5, 5, 5, 5,13, 9,13, 9,13, 9, 6,10,5},
    { 0, 0, 0, 5, 4, 6, 8, 4, 4, 4,12, 8,12,12, 8, 1,4},
  },
  {
    {15,15,11, 8,15,11, 9, 8,15,11,15,11, 8,13, 9, 5,1},
    { 0,14,15,12,10, 8,14,10,14,14,10,14,10, 7,12, 8,4},
    { 0, 0,13,14,11, 9,13, 9,13,10,13, 9,13, 9,11, 7,3},
    { 0, 0, 0,12,11,10, 9, 8,13,12,12,12, 8,12,10, 6,2},
  },
};

const UChar g_aucLenTable3[7][15] =
{
  {1,1},
  {1,2,2},
  {2,2,2,2},
  {2,2,2,3,3},
  {2,2,3,3,3,3},
  {2,3,3,3,3,3,3},
  {3,3,3,3,3,3,3,4,5,6,7,8,9,10,11},
};

const UChar g_aucCodeTable3[7][15] =
{
  {1,0},
  {1,1,0},
  {3,2,1,0},
  {3,2,1,1,0},
  {3,2,3,2,1,0},
  {3,0,1,3,2,5,4},
  {7,6,5,4,3,2,1,1,1,1,1,1,1,1,1},
};

const UInt g_auiISymCode[3][16] =
{ {  0,  1                                                        },
  {  0,  4,  5, 28,  6, 29, 30, 31                                },
  {  0,  4,  5, 28, 12, 29, 60,252, 13, 61,124,253,125,254,510,511}
};
const UInt g_auiISymLen[3][16] =
{ { 1, 1                                          },
  { 1, 3, 3, 5, 3, 5, 5, 5                        },
  { 1, 3, 3, 5, 4, 5, 6, 8, 4, 6, 7, 8, 7, 8, 9, 9}
};

const UInt g_auiRefSymCode[2][27] =
{
  { 
    0x1, 0x3, 0x5, 0x3, 0x5, 0x5, 0x4, 0x5, 0x5, 
    0x2, 0x4, 0x4, 0x3, 0x4, 0x3, 0x4, 0x2, 0x3, 
    0x3, 0x3, 0x3, 0x3, 0x1, 0x2, 0x2, 0x1, 0x0
  },
  {
    0x1, 0x7, 0x6, 0x7,	0x9, 0x8, 0x6, 0x7,	0x9,
    0x5, 0x6,	0x5, 0x8,	0x7, 0x6,	0x7, 0x5,	0x4,
    0x4, 0x4,	0x6, 0x5,	0x3, 0x2,	0x5, 0x1,	0x0
  }
};

const UInt g_auiRefSymLen[2][27] =
{
  {
    1, 4, 5, 3, 6, 8, 5, 7, 9,
    3, 6, 8, 6, 9,10, 7,10,12,
    5, 7, 9, 8,10,12, 9,12,12
  },
  {
    1, 5, 5, 4, 7, 7, 4, 7, 6,
    4, 7, 7, 6, 8, 8, 6, 8, 8,
    4, 7, 6, 6, 8, 8, 5, 8, 8
  }
};

UvlcReader::UvlcReader() :
  m_pcBitReadBuffer( NULL ),
  m_uiBitCounter( 0 ),
  m_uiPosCounter( 0 ),
  m_bRunLengthCoding( false ),
  m_uiRun( 0 )
, m_bParentFlag     ( true )          
, m_bTruncated      ( false )
{
  UInt ui;

  for( ui = 0; ui < MAX_NUM_PD_FRAGMENTS; ui ++ )
  {
    m_apcFragBitBuffers [ui] = 0;
    m_apcFragSymGrps    [ui] = 0;
  }

  m_uiCurrentFragment = 0;
  m_uiNumFragments    = 0;
  m_pSymGrp        = new UcSymGrpReader( this );
}

UvlcReader::~UvlcReader()
{
  delete m_pSymGrp; 
}



__inline ErrVal UvlcReader::xGetCode( UInt& ruiCode, UInt uiLength )
{
  DTRACE_TY( " u(v)" );

  ErrVal retVal = m_pcBitReadBuffer->get( ruiCode, uiLength );

  DTRACE_POS;
  DTRACE_CODE (ruiCode);
  DTRACE_BITS (ruiCode, uiLength );
  DTRACE_COUNT(uiLength);

  return retVal;
}


__inline ErrVal UvlcReader::xGetFlag( UInt& ruiCode )
{
  DTRACE_TY( " u(1)" );

  ErrVal retVal = m_pcBitReadBuffer->get( ruiCode );

  DTRACE_POS;
  DTRACE_CODE (ruiCode);
  DTRACE_BITS (ruiCode, 1 );
  DTRACE_COUNT(1);

  return retVal;
}


Bool UvlcReader::moreRBSPData()
{
  return m_pcBitReadBuffer->isValid();
}


ErrVal UvlcReader::create( UvlcReader*& rpcUvlcReader )
{
  rpcUvlcReader = new UvlcReader;

  ROT( NULL == rpcUvlcReader );

  return Err::m_nOK;
}

ErrVal UvlcReader::destroy()
{
  delete this;
  return Err::m_nOK;
}

ErrVal UvlcReader::init(  BitReadBuffer* pcBitReadBuffer )
{
  ROT( NULL == pcBitReadBuffer );

  m_pcBitReadBuffer = pcBitReadBuffer;

  m_bRunLengthCoding  = false;
  m_uiRun             = 0;

  return Err::m_nOK;
}

ErrVal UvlcReader::uninit()
{
  m_pcBitReadBuffer = NULL;
  return Err::m_nOK;
}



ErrVal UvlcReader::xGetUvlcCode( UInt& ruiVal)
{
  UInt uiVal = 0;
  UInt uiCode = 0;
  UInt uiLength;

  DTRACE_DO( m_uiBitCounter = 1 );
  DTRACE_TY( "ue(v)" );

  DECRNOK( m_pcBitReadBuffer->get( uiCode, 1 ) );
  DTRACE_BITS(uiCode, 1);

  if( 0 == uiCode )
  {
    uiLength = 0;

    while( ! ( uiCode & 1 ))
    {
      DECRNOK( m_pcBitReadBuffer->get( uiCode, 1 ) );
      DTRACE_BITS(uiCode, 1);
      uiLength++;
    }

    DTRACE_DO( m_uiBitCounter += 2*uiLength );

    DECRNOK( m_pcBitReadBuffer->get( uiVal, uiLength ) );
    DTRACE_BITS(uiVal, uiLength);

    uiVal += (1 << uiLength)-1;
  }

  ruiVal = uiVal;

  DTRACE_POS;
  DTRACE_CODE(uiVal);
  DTRACE_COUNT(m_uiBitCounter);

  return Err::m_nOK;
}


ErrVal UvlcReader::xGetSvlcCode( Int& riVal)
{
  UInt uiBits = 0;

  DTRACE_DO( m_uiBitCounter = 1 );
  DTRACE_TY( "se(v)" );

  DECRNOK( m_pcBitReadBuffer->get( uiBits, 1 ) );
  DTRACE_BITS(uiBits, 1);
  if( 0 == uiBits )
  {
    UInt uiLength = 0;

    while( ! ( uiBits & 1 ))
    {
      DECRNOK( m_pcBitReadBuffer->get( uiBits, 1 ) );
      DTRACE_BITS(uiBits, 1);
      uiLength++;
    }

    DTRACE_DO( m_uiBitCounter += 2*uiLength );

    DECRNOK( m_pcBitReadBuffer->get( uiBits, uiLength ) );
    DTRACE_BITS(uiBits, uiLength);

    uiBits += (1 << uiLength);
    riVal = ( uiBits & 1) ? -(Int)(uiBits>>1) : (Int)(uiBits>>1);
  }
  else
  {
    riVal = 0;
  }

  DTRACE_POS;
  DTRACE_CODE(riVal);
  DTRACE_COUNT(m_uiBitCounter);

  return Err::m_nOK;
}




ErrVal UvlcReader::getFlag( Bool& rbFlag, Char* pcTraceString )
{
  DTRACE_TH( pcTraceString );

  UInt uiCode;
  DECRNOK( xGetFlag( uiCode ) );

  rbFlag = (1 == uiCode);
  DTRACE_N;

  return Err::m_nOK;
}


ErrVal UvlcReader::getCode( UInt& ruiCode, UInt uiLength, Char* pcTraceString )
{
  DTRACE_TH( pcTraceString );
  RNOK( xGetCode( ruiCode, uiLength) );
  DTRACE_N;

  return Err::m_nOK;
}


ErrVal UvlcReader::getUvlc( UInt& ruiCode, Char* pcTraceString )
{
  DTRACE_TH( pcTraceString );

  DECRNOK( xGetUvlcCode( ruiCode ) );

  DTRACE_N;

  return Err::m_nOK;
}


ErrVal UvlcReader::getSvlc( Int& riCode, Char* pcTraceString )
{
  DTRACE_TH( pcTraceString );

  DECRNOK( xGetSvlcCode( riCode ) );

  DTRACE_N;

  return Err::m_nOK;
}

ErrVal UvlcReader::readByteAlign()
{
  UInt uiCode;
  UInt uiLength = m_pcBitReadBuffer->getBitsUntilByteAligned();

  ROTRS( 0 == uiLength, Err::m_nOK );

  DECRNOK( m_pcBitReadBuffer->get( uiCode, uiLength ) );

  DECROF( (UInt(1<<uiLength)>>1) == uiCode );

  DTRACE_POS;
  DTRACE_T( "SEI: alignment_bits" );
  DTRACE_TY( " u(v)" );
  DTRACE_CODE( uiCode );
  DTRACE_BITS( uiCode, uiLength );
  DTRACE_N;
  DTRACE_COUNT( uiLength );

  return Err::m_nOK;
}

//JVT-T073 {
ErrVal UvlcReader::readZeroByteAlign()
{
  UInt uiCode;
  UInt uiLength = m_pcBitReadBuffer->getBitsUntilByteAligned();

  ROTRS( 0 == uiLength, Err::m_nOK );

  DECRNOK( m_pcBitReadBuffer->get( uiCode, uiLength ) );


  DTRACE_POS;
  DTRACE_T( "NestingSEI: zero_alignment_bits" );
  DTRACE_TY( " u(v)" );
  DTRACE_CODE( uiCode );
  DTRACE_BITS( uiCode, uiLength );
  DTRACE_N;
  DTRACE_COUNT( uiLength );

  return Err::m_nOK;
}
//JVT-T073 }


ErrVal UvlcReader::startSlice( const SliceHeader& rcSliceHeader )
{
  m_bRunLengthCoding  = ! rcSliceHeader.isIntra();
  m_uiRun             = 0;

  // for FGS refinement coding
  RQreset( rcSliceHeader );

  return Err::m_nOK;
}

ErrVal UvlcReader::refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx )
{
  UInt uiRefFrame = 0;
	RNOK( xGetRefFrame( 2 == rcMbDataAccess.getNumActiveRef( eLstIdx ), uiRefFrame, eLstIdx ) );
  rcMbDataAccess.getMbMotionData( eLstIdx ).setRefIdx( ++uiRefFrame );
  return Err::m_nOK;
}

ErrVal UvlcReader::refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  )
{
  UInt uiRefFrame = 0;
	RNOK( xGetRefFrame( 2 == rcMbDataAccess.getNumActiveRef( eLstIdx ), uiRefFrame, eLstIdx ) );
  rcMbDataAccess.getMbMotionData( eLstIdx ).setRefIdx( ++uiRefFrame, eParIdx );
  return Err::m_nOK;
}

ErrVal UvlcReader::refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  )
{
  UInt uiRefFrame = 0;
	RNOK( xGetRefFrame( 2 == rcMbDataAccess.getNumActiveRef( eLstIdx ), uiRefFrame, eLstIdx ) );
  rcMbDataAccess.getMbMotionData( eLstIdx ).setRefIdx( ++uiRefFrame, eParIdx );
  return Err::m_nOK;
}

ErrVal UvlcReader::refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8 eParIdx  )
{
  UInt uiRefFrame = 0;
	RNOK( xGetRefFrame( 2 == rcMbDataAccess.getNumActiveRef( eLstIdx ), uiRefFrame, eLstIdx ) );
  rcMbDataAccess.getMbMotionData( eLstIdx ).setRefIdx( ++uiRefFrame, eParIdx );
  return Err::m_nOK;
}

ErrVal  UvlcReader::motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx )
{
  Bool bFlag;
  RNOK( xGetMotionPredFlag( bFlag ) );
  rcMbDataAccess.getMbMotionData( eLstIdx ).setMotPredFlag( bFlag );
  return Err::m_nOK;
}
ErrVal  UvlcReader::motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx   )
{
  Bool bFlag;
  RNOK( xGetMotionPredFlag( bFlag ) );
  rcMbDataAccess.getMbMotionData( eLstIdx ).setMotPredFlag( bFlag, eParIdx );
  return Err::m_nOK;
}
ErrVal  UvlcReader::motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx   )
{
  Bool bFlag;
  RNOK( xGetMotionPredFlag( bFlag ) );
  rcMbDataAccess.getMbMotionData( eLstIdx ).setMotPredFlag( bFlag, eParIdx );
  return Err::m_nOK;
}
ErrVal  UvlcReader::motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8 eParIdx   )
{
  Bool bFlag;
  RNOK( xGetMotionPredFlag( bFlag ) );
  rcMbDataAccess.getMbMotionData( eLstIdx ).setMotPredFlag( bFlag, eParIdx );
  return Err::m_nOK;
}

ErrVal UvlcReader::blockModes( MbDataAccess& rcMbDataAccess )
{
  for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
  {
    DTRACE_T( "BlockMode" );

    UInt uiBlockMode = 0;
    RNOK( xGetUvlcCode( uiBlockMode ) );

    rcMbDataAccess.setConvertBlkMode( c8x8Idx.b8x8Index(), uiBlockMode );

    DTRACE_N;
  }
  return Err::m_nOK;
}

Bool UvlcReader::isMbSkipped( MbDataAccess& rcMbDataAccess )
{
  rcMbDataAccess.getMbTCoeffs().setAllCoeffCount( 0 );

  ROFRS( m_bRunLengthCoding, false );

  if ( m_uiRun > 0 )
  {
    m_uiRun--;
  } else {
    DTRACE_T( "Run" );
    ANOK( xGetUvlcCode( m_uiRun ) );
    DTRACE_N;
  }
  rcMbDataAccess.getMbData().setSkipFlag( m_uiRun != 0 );
  
  return ( rcMbDataAccess.getMbData().getSkipFlag() );
}

Bool UvlcReader::isBLSkipped( MbDataAccess& rcMbDataAccess )
{
  UInt uiCode;
  ANOK( xGetFlag( uiCode ) );

  DTRACE_T( "BLSkipFlag" );
  DTRACE_TY( "ae(v)" );
  DTRACE_CODE( uiCode );
  DTRACE_N;

  rcMbDataAccess.getMbTCoeffs().setAllCoeffCount( 0 );

  rcMbDataAccess.getMbData().setBLSkipFlag( ( uiCode != 0 ) );
  return rcMbDataAccess.getMbData().getBLSkipFlag();
}

ErrVal UvlcReader::mbMode( MbDataAccess& rcMbDataAccess )
{
  UInt uiMbMode = 0;
  rcMbDataAccess.getMbTCoeffs().setAllCoeffCount( 0 );
  rcMbDataAccess.getMbData().setBCBPAll( 0 );

  DTRACE_T( "MbMode" );
  RNOK( xGetUvlcCode( uiMbMode ) );
  DTRACE_N;

  rcMbDataAccess.setConvertMbType( uiMbMode );

  return Err::m_nOK;
}

ErrVal UvlcReader::resPredFlag( MbDataAccess& rcMbDataAccess )
{
  UInt uiCode;
  DTRACE_T( "ResidualPredFlag" );
  RNOK( xGetFlag( uiCode ) );
  DTRACE_N;
  rcMbDataAccess.getMbData().setResidualPredFlag( uiCode?true:false );

  return Err::m_nOK;
}

ErrVal UvlcReader::resPredFlag_FGS( MbDataAccess& rcMbDataAccess, Bool bBaseCoeff )
{
  UInt uiCode;
  DTRACE_T( "ResidualPredFlag" );
  RNOK( xGetFlag( uiCode ) );
  DTRACE_N;
  rcMbDataAccess.getMbData().setResidualPredFlag( uiCode?true:false );

  return Err::m_nOK;
}

//-- JVT-R091
ErrVal UvlcReader::smoothedRefFlag( MbDataAccess& rcMbDataAccess )
{
  UInt uiCode;
  DTRACE_T( "SmoothedRefFlag" );
  RNOK( xGetFlag( uiCode ) );
  DTRACE_N;
  rcMbDataAccess.getMbData().setSmoothedRefFlag( uiCode?true:false );

  return Err::m_nOK;
}
//--

ErrVal UvlcReader::mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx )
{
  Mv cMv;
  RNOK( xGetMvd( cMv ) );
  rcMbDataAccess.getMbMvdData( eLstIdx ).setAllMv( cMv );
  return Err::m_nOK;
}

ErrVal UvlcReader::mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  )
{
  Mv cMv;
  RNOK( xGetMvd( cMv ) );
  rcMbDataAccess.getMbMvdData( eLstIdx ).setAllMv( cMv, eParIdx );
  return Err::m_nOK;
}

ErrVal UvlcReader::mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  )
{
  Mv cMv;
  RNOK( xGetMvd( cMv ) );
  rcMbDataAccess.getMbMvdData( eLstIdx ).setAllMv( cMv, eParIdx );
  return Err::m_nOK;
}

ErrVal UvlcReader::mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  )
{
  Mv cMv;
  RNOK( xGetMvd( cMv ) );
  rcMbDataAccess.getMbMvdData( eLstIdx ).setAllMv( cMv, eParIdx );
  return Err::m_nOK;
}

ErrVal UvlcReader::mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx8x4 eSParIdx )
{
  Mv cMv;
  RNOK( xGetMvd( cMv ) );
  rcMbDataAccess.getMbMvdData( eLstIdx ).setAllMv( cMv, eParIdx, eSParIdx );
  return Err::m_nOK;
}

ErrVal UvlcReader::mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx4x8 eSParIdx )
{
  Mv cMv;
  RNOK( xGetMvd( cMv ) );
  rcMbDataAccess.getMbMvdData( eLstIdx ).setAllMv( cMv, eParIdx, eSParIdx );
  return Err::m_nOK;
}

ErrVal UvlcReader::mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx4x4 eSParIdx )
{
  Mv cMv;
  RNOK( xGetMvd( cMv ) );
  rcMbDataAccess.getMbMvdData( eLstIdx ).setAllMv( cMv, eParIdx, eSParIdx );
  return Err::m_nOK;
}

ErrVal UvlcReader::fieldFlag( MbDataAccess& rcMbDataAccess )
{
  DTRACE_T( "MbFieldFlag" );

  UInt uiBit;

  DECRNOK( xGetFlag( uiBit ) );

  rcMbDataAccess.setFieldMode(uiBit != 0);

  DTRACE_N;

  return Err::m_nOK;
}

ErrVal UvlcReader::intraPredModeChroma( MbDataAccess& rcMbDataAccess )
{
  DTRACE_T( "IntraPredModeChroma" );

  UInt uiCode = 0;
  RNOK( xGetUvlcCode( uiCode ) );
  rcMbDataAccess.getMbData().setChromaPredMode( uiCode );

  DTRACE_N;

  return Err::m_nOK;
}

ErrVal UvlcReader::intraPredModeLuma( MbDataAccess& rcMbDataAccess, LumaIdx cIdx )
{
  DTRACE_T( "IntraPredModeLuma" );
  DTRACE_POS;

  UInt uiBits;
  RNOK( m_pcBitReadBuffer->get( uiBits, 1 ) );
  DTRACE_BITS( uiBits,1 );
  DTRACE_DO( m_uiBitCounter = 1 );

  if( ! uiBits )
  {
    UInt uiCode;
    RNOK( m_pcBitReadBuffer->get( uiCode, 3 ) );
    rcMbDataAccess.getMbData().intraPredMode( cIdx ) = uiCode;
    DTRACE_BITS( uiCode, 3 );
    DTRACE_DO( m_uiBitCounter = 4 );
  }
  else
  {
    rcMbDataAccess.getMbData().intraPredMode( cIdx ) = -1;
  }
  
  DTRACE_COUNT(m_uiBitCounter);
  DTRACE_CODE(rcMbDataAccess.getMbData().intraPredMode( cIdx ));
  DTRACE_N;

  return Err::m_nOK;
}


ErrVal UvlcReader::intraPredModeLuma8x8( MbDataAccess& rcMbDataAccess, B8x8Idx cIdx ) // HS: bug fix by Nokia
{
  DTRACE_T( "IntraPredModeLuma" );
  DTRACE_POS;

  UInt uiBits;
  RNOK( m_pcBitReadBuffer->get( uiBits, 1 ) );
  DTRACE_BITS( uiBits,1 );
  DTRACE_DO( m_uiBitCounter = 1 );

  if( ! uiBits )
  {
    UInt uiCode;
    RNOK( m_pcBitReadBuffer->get( uiCode, 3 ) );
    rcMbDataAccess.getMbData().intraPredMode( cIdx ) = uiCode;
    DTRACE_BITS( uiCode, 3 );
    DTRACE_DO( m_uiBitCounter = 4 );
  }
  else
  {
    rcMbDataAccess.getMbData().intraPredMode( cIdx ) = -1;
  }
  
  DTRACE_COUNT(m_uiBitCounter);
  DTRACE_CODE(rcMbDataAccess.getMbData().intraPredMode( cIdx ));
  DTRACE_N;
  
  return Err::m_nOK;
}


ErrVal UvlcReader::cbp( MbDataAccess& rcMbDataAccess )
{
  UInt uiTemp = 0;
  DTRACE_T( "Cbp: " );
  RNOK( xGetUvlcCode( uiTemp ) );

  Bool bIntra = ( !rcMbDataAccess.getMbData().getBLSkipFlag() && rcMbDataAccess.getMbData().isIntra() );
  UInt uiCbp  = ( bIntra ? g_aucCbpIntra[uiTemp]: g_aucCbpInter[uiTemp] );
  DTRACE_X ( uiCbp );
  DTRACE_N;

  rcMbDataAccess.getMbData().setMbCbp( uiCbp );

  return Err::m_nOK;
}

ErrVal UvlcReader::deltaQp( MbDataAccess& rcMbDataAccess )
{
  DTRACE_T ("DQp");

  Int uiCode = 0;
  RNOK( xGetSvlcCode( uiCode ) );
  rcMbDataAccess.addDeltaQp( uiCode );

  DTRACE_TY ("se(v)");
  DTRACE_N;
  return Err::m_nOK;
}

ErrVal UvlcReader::samplesPCM( MbDataAccess& rcMbDataAccess )
{
  DTRACE_POS;
  DTRACE_T( "  PCM SAMPLES: " );

  RNOK( m_pcBitReadBuffer->flush( m_pcBitReadBuffer->getBitsUntilByteAligned() ) );

  AOF_DBG( rcMbDataAccess.getMbData().isPCM() );

  rcMbDataAccess.getMbTCoeffs().setAllCoeffCount( 16 );
  Pel* pSrc = rcMbDataAccess.getMbTCoeffs().getPelBuffer();

  const UInt uiFactor = 8*8;
  const UInt uiSize   = uiFactor*2*3;
  RNOK( m_pcBitReadBuffer->samples( pSrc, uiSize ) );

  DTRACE_N;
  DTRACE_COUNT( uiFactor*6 );

  return Err::m_nOK;
}

ErrVal UvlcReader::residualBlock( MbDataAccess& rcMbDataAccess,
                                  LumaIdx       cIdx,
                                  ResidualMode  eResidualMode,
                                  UInt&         ruiMbExtCbp)
{
  const UChar*  pucScan;
	const Bool bFrame = ( FRAME == rcMbDataAccess.getMbPicType());
  TCoeff* piCoeff = rcMbDataAccess.getMbTCoeffs().get( cIdx );

  UInt  uiPos;
  UInt  uiMaxPos  = 16;
  Int   aiLevelRun[32];
  UInt  uiTrailingOnes = 0;
  UInt  uiTotalRun     = 0;
  UInt  uiCoeffCnt     = 0;

  for ( uiPos = 0; uiPos < 32; uiPos++ )
  {
    aiLevelRun[uiPos] = 0;
  }
  uiPos = 0;

  switch( eResidualMode )
  {
  case LUMA_I16_DC:
    {
			pucScan = (bFrame) ? g_aucLumaFrameDCScan : g_aucLumaFieldDCScan;
      uiMaxPos = 16;
      DTRACE_T( "Luma:" );
      DTRACE_V( cIdx );
      DTRACE_N;
      xPredictNonZeroCnt( rcMbDataAccess, cIdx, uiCoeffCnt, uiTrailingOnes );
      xGetRunLevel( aiLevelRun, uiCoeffCnt, uiTrailingOnes, 16, uiTotalRun );
      break;
    }
  case LUMA_I16_AC:
    {
			pucScan = (bFrame) ? g_aucFrameScan : g_aucFieldScan;
      uiPos=1;
      DTRACE_T( "Luma:" );
      DTRACE_V( cIdx );
      DTRACE_N;
      xPredictNonZeroCnt( rcMbDataAccess, cIdx, uiCoeffCnt, uiTrailingOnes );
      xGetRunLevel( aiLevelRun, uiCoeffCnt, uiTrailingOnes, 15, uiTotalRun );
      break;
    }
  case LUMA_SCAN:
    {
			pucScan = (bFrame) ? g_aucFrameScan : g_aucFieldScan;
      DTRACE_T( "Luma:" );
      DTRACE_V( cIdx );
      DTRACE_N;
      xPredictNonZeroCnt( rcMbDataAccess, cIdx, uiCoeffCnt, uiTrailingOnes );
      xGetRunLevel( aiLevelRun, uiCoeffCnt, uiTrailingOnes, 16, uiTotalRun );
      // this is useful only in AR_FGS
      rcMbDataAccess.getMbData().setBCBP( cIdx, uiCoeffCnt != 0);
      break;
    }
  default:
    return Err::m_nERR;
  }

  uiPos += uiTotalRun + uiCoeffCnt - 1;
  for ( Int i = (Int)uiCoeffCnt; i > 0; i-- )
  {
    piCoeff[ pucScan [uiPos--] ] = aiLevelRun[i-1];
    for ( Int j = 0; j < aiLevelRun[i-1+0x10]; j++ )
    {
      piCoeff[ pucScan [uiPos--] ] = 0;
    }
  }

  Bool bCoded = (uiCoeffCnt > 0);
  if( ! bCoded )
  {
    ruiMbExtCbp &= ~(1 << cIdx.b4x4() );
  }

  return Err::m_nOK;
}

ErrVal UvlcReader::residualBlock( MbDataAccess& rcMbDataAccess,
                                  ChromaIdx     cIdx,
                                  ResidualMode  eResidualMode )
{
	const Bool bFrame = ( FRAME == rcMbDataAccess.getMbPicType());
  TCoeff* piCoeff = rcMbDataAccess.getMbTCoeffs().get( cIdx );
  const UChar*  pucScan;
  UInt          uiPos, uiMaxPos;
  Int aiLevelRun[32];

  UInt uiTrailingOnes = 0;
  UInt uiTotalRun     = 0;
  UInt uiCoeffCnt     = 0;

  for ( uiPos = 0; uiPos < 32; uiPos++ )
  {
    aiLevelRun[uiPos] = 0;
  }
  uiPos = 0;

  switch( eResidualMode )
  {
  case CHROMA_DC:
    {
      pucScan = g_aucIndexChromaDCScan;
      uiPos=0;  uiMaxPos= 4;
      DTRACE_T( "CHROMA_DC:" );
      DTRACE_V( cIdx );
      DTRACE_N;
      xGetTrailingOnes4( uiCoeffCnt, uiTrailingOnes );
      xGetRunLevel( aiLevelRun, uiCoeffCnt, uiTrailingOnes, 4, uiTotalRun );
      // this is useful only in AR_FGS
      rcMbDataAccess.getMbData().setBCBP( 24 + cIdx.plane(), uiCoeffCnt != 0);
      break;
    }
  case CHROMA_AC:
    {
			pucScan = (bFrame) ? g_aucFrameScan : g_aucFieldScan;
      uiPos=1;  uiMaxPos=16;
      DTRACE_T( "CHROMA_AC:" );
      DTRACE_V( cIdx );
      DTRACE_N;
      xPredictNonZeroCnt( rcMbDataAccess, cIdx, uiCoeffCnt, uiTrailingOnes );
      xGetRunLevel( aiLevelRun, uiCoeffCnt, uiTrailingOnes, 15, uiTotalRun );
      // this is useful only in AR_FGS
      rcMbDataAccess.getMbData().setBCBP( 16 + cIdx, uiCoeffCnt != 0);
      break;
    }
  default:
    return Err::m_nERR;
  }

  uiPos += uiTotalRun + uiCoeffCnt - 1;
  for ( Int i = (Int)uiCoeffCnt; i > 0; i-- )
  {
    piCoeff[ pucScan [uiPos--] ] = aiLevelRun[i-1];
    for ( Int j = 0; j < aiLevelRun[i-1+0x10]; j++ )
    {
      piCoeff[ pucScan [uiPos--] ] = 0;
    }
  }

  return Err::m_nOK;
}

ErrVal UvlcReader::transformSize8x8Flag( MbDataAccess& rcMbDataAccess ) 
{
  DTRACE_T( "transformSize8x8Flag:" );

  UInt  uiCode;
  RNOK( xGetFlag( uiCode) );
  rcMbDataAccess.getMbData().setTransformSize8x8( uiCode?true:false );

  DTRACE_V( uiCode );
  DTRACE_N;
  return Err::m_nOK;
}


ErrVal UvlcReader::xGetRefFrame( Bool bWriteBit, UInt& uiRefFrame, ListIdx eLstIdx )
{
  DTRACE_T( "RefFrame" );
  UInt uiCode;

  if( bWriteBit )
  {
    RNOK( xGetFlag( uiCode ) );
    uiRefFrame = 1-uiCode;
  }
  else
  {
    RNOK( xGetUvlcCode( uiRefFrame ) );
  }

  DTRACE_V( uiRefFrame+1 );
  DTRACE_N;
  return Err::m_nOK;
}

ErrVal UvlcReader::xGetMotionPredFlag( Bool& rbFlag )
{
  DTRACE_T( "MotionPredFlag" );

  UInt uiCode;
  RNOK( xGetFlag( uiCode ) );

  DTRACE_V( uiCode );
  DTRACE_N;
  rbFlag = (uiCode == 1);
  return Err::m_nOK;
}


ErrVal UvlcReader::xGetMvd( Mv& cMv )
{
  DTRACE_T( "Mvd: x" );

  UInt  uiTemp = 0;

  RNOK( xGetUvlcCode( uiTemp ) );

  Short sHor = ( uiTemp & 1) ? (Int)((uiTemp+1)>>1) : -(Int)(uiTemp>>1);
  DTRACE_CODE( sHor );
  DTRACE_TY("se(v)");
  DTRACE_N;

  DTRACE_T( "Mvd: y" );

  RNOK( xGetUvlcCode( uiTemp ) );

  Short sVer = ( uiTemp & 1) ? (Int)((uiTemp+1)>>1) : -(Int)(uiTemp>>1);

  DTRACE_CODE( sVer );
  DTRACE_TY("se(v)");
  DTRACE_N;

  cMv.setHor( sHor );
  cMv.setVer( sVer );

  return Err::m_nOK;
}

ErrVal UvlcReader::xPredictNonZeroCnt( MbDataAccess& rcMbDataAccess, LumaIdx cIdx, UInt& uiCoeffCount, UInt& uiTrailingOnes )
{
  UInt uiCoeffCountCtx = rcMbDataAccess.getCtxCoeffCount( cIdx );

  xGetTrailingOnes16( uiCoeffCountCtx, uiCoeffCount, uiTrailingOnes );

  rcMbDataAccess.getMbTCoeffs().setCoeffCount( cIdx, uiCoeffCount );

  return Err::m_nOK;
}

ErrVal UvlcReader::xPredictNonZeroCnt( MbDataAccess& rcMbDataAccess, ChromaIdx cIdx, UInt& uiCoeffCount, UInt& uiTrailingOnes )
{
  UInt uiCoeffCountCtx = rcMbDataAccess.getCtxCoeffCount( cIdx );

  xGetTrailingOnes16( uiCoeffCountCtx, uiCoeffCount, uiTrailingOnes );

  rcMbDataAccess.getMbTCoeffs().setCoeffCount( cIdx, uiCoeffCount );

  return Err::m_nOK;
}

ErrVal UvlcReader::xGetTrailingOnes16( UInt uiLastCoeffCount, UInt& uiCoeffCount, UInt& uiTrailingOnes )
{
  DTRACE_POS;
  if( 3 == uiLastCoeffCount )
  {
    UInt uiBits;
    RNOK( m_pcBitReadBuffer->get( uiBits, 6 ) );
    DTRACE_DO( m_uiBitCounter = 6 );

    uiTrailingOnes = ( uiBits & 0x3 );
    uiCoeffCount   = ( uiBits >>  2 );
    if ( !uiCoeffCount && uiTrailingOnes == 3 )
    {
      uiTrailingOnes = 0;
    }
    else
    {
      uiCoeffCount++;
    }
  }
  else
  {
    assert (uiLastCoeffCount < 3);

    RNOK( xCodeFromBitstream2D( &g_aucCodeTableTO16[uiLastCoeffCount][0][0], &g_aucLenTableTO16[uiLastCoeffCount][0][0], 17, 4, uiCoeffCount, uiTrailingOnes ) );
    DTRACE_DO( m_uiBitCounter = g_aucLenTableTO16[uiLastCoeffCount][uiTrailingOnes][uiCoeffCount] );
  }

  DTRACE_T( "  TrailingOnes16: Vlc: " );
  DTRACE_V( uiLastCoeffCount );
  DTRACE_T( " CoeffCnt: " );
  DTRACE_V( uiCoeffCount );
  DTRACE_T( " TraiOnes: " );
  DTRACE_V( uiTrailingOnes );
  DTRACE_N;
  DTRACE_COUNT(m_uiBitCounter);

  return Err::m_nOK;
}

ErrVal UvlcReader::xCodeFromBitstream2D( const UChar* aucCod, const UChar* aucLen, UInt uiWidth, UInt uiHeight, UInt& uiVal1, UInt& uiVal2 )
{
  const UChar *paucLenTab;
  const UChar *paucCodTab;
  UChar  uiLenRead = 0;
  UChar  uiCode    = 0;
  UChar  uiMaxLen  = 0;

  // Find maximum number of bits to read before generating error
  paucLenTab = aucLen;
  paucCodTab = aucCod;
  for (UInt j = 0; j < uiHeight; j++, paucLenTab += uiWidth, paucCodTab += uiWidth)
  {
    for ( UInt i = 0; i < uiWidth; i++ )
    {
      if ( paucLenTab[i] > uiMaxLen )
      {
        uiMaxLen = paucLenTab[i];
      }
    }
  }

  while ( uiLenRead < uiMaxLen )
  {
    // Read next bit
    UInt uiBit;
    RNOKS( m_pcBitReadBuffer->get( uiBit, 1 ) );
    uiCode = ( uiCode << 1 ) + uiBit;
    uiLenRead++;

    // Check for matches
    paucLenTab = aucLen;
    paucCodTab = aucCod;
    for (UInt j = 0; j < uiHeight; j++, paucLenTab += uiWidth, paucCodTab += uiWidth)
    {
      for (UInt i = 0; i < uiWidth; i++)
      {
        if ( (paucLenTab[i] == uiLenRead) && (paucCodTab[i] == uiCode) )
        {
          uiVal1 = i;
          uiVal2 = j;
          return Err::m_nOK;
        }
      }
    }

  }
  return Err::m_nERR;
}

ErrVal UvlcReader::xCodeFromBitstream2Di( const UInt* auiCod, const UInt* auiLen, UInt uiWidth, UInt uiHeight, UInt& uiVal1, UInt& uiVal2 )
{
  const UInt *pauiLenTab;
  const UInt *pauiCodTab;
  UChar  uiLenRead = 0;
  UInt   uiCode    = 0;
  UChar  uiMaxLen  = 0;

  // Find maximum number of bits to read before generating error
  pauiLenTab = auiLen;
  pauiCodTab = auiCod;
  for (UInt j = 0; j < uiHeight; j++, pauiLenTab += uiWidth, pauiCodTab += uiWidth)
  {
    for ( UInt i = 0; i < uiWidth; i++ )
    {
      if ( pauiLenTab[i] > uiMaxLen )
      {
        uiMaxLen = pauiLenTab[i];
      }
    }
  }

  while ( uiLenRead < uiMaxLen )
  {
    // Read next bit
    UInt uiBit;
    
    RNOKS( m_pcBitReadBuffer->get( uiBit, 1 ) );

    uiCode = ( uiCode << 1 ) + uiBit;
    uiLenRead++;

    // Check for matches
    pauiLenTab = auiLen;
    pauiCodTab = auiCod;
    for (UInt j = 0; j < uiHeight; j++, pauiLenTab += uiWidth, pauiCodTab += uiWidth)
    {
      for (UInt i = 0; i < uiWidth; i++)
      {
        if ( (pauiLenTab[i] == uiLenRead) && (pauiCodTab[i] == uiCode) )
        {
          uiVal1 = i;
          uiVal2 = j;
          return Err::m_nOK;
        }
      }
    }

  }
  return Err::m_nERR;
}

ErrVal UvlcReader::xGetRunLevel( Int* aiLevelRun, UInt uiCoeffCnt, UInt uiTrailingOnes, UInt uiMaxCoeffs, UInt& uiTotalRun )
{

  ROTRS( 0 == uiCoeffCnt, Err::m_nOK );

  if( uiTrailingOnes )
  {
    UInt uiBits;
    RNOK( m_pcBitReadBuffer->get( uiBits, uiTrailingOnes ));

    Int n = uiTrailingOnes-1;
    for( UInt k = uiCoeffCnt; k > uiCoeffCnt-uiTrailingOnes; k--, n--)
    {
      aiLevelRun[k-1] = (uiBits & (1<<n)) ? -1 : 1;
    }

    DTRACE_POS;
    DTRACE_T( "  TrailingOnesSigns: " );
    DTRACE_V( uiBits );
    DTRACE_N;
    DTRACE_COUNT(uiTrailingOnes);
  }

  UInt uiHighLevel = ( uiCoeffCnt > 3 && uiTrailingOnes == 3) ? 0 : 1;
  UInt uiVlcTable  = ( uiCoeffCnt > 10 && uiTrailingOnes < 3) ? 1 : 0;

  for( Int k = uiCoeffCnt - 1 - uiTrailingOnes; k >= 0; k--)
  {
    Int iLevel;

    if( uiVlcTable == 0 )
    {
	    xGetLevelVLC0( iLevel );
    }
    else
    {
	    xGetLevelVLCN( iLevel, uiVlcTable );
    }

    if( uiHighLevel )
    {
      iLevel += ( iLevel > 0 ) ? 1 : -1;
	    uiHighLevel = 0;
    }
    aiLevelRun[k] = iLevel;

    UInt uiAbsLevel = (UInt)abs(iLevel);

    // update VLC table
    if( uiAbsLevel > g_auiIncVlc[ uiVlcTable ] )
    {
      uiVlcTable++;
    }

    if( k == Int(uiCoeffCnt - 1 - uiTrailingOnes) && uiAbsLevel > 3)
    {
      uiVlcTable = 2;
    }

  }

  ROFRS( uiCoeffCnt < uiMaxCoeffs, Err::m_nOK );


  uiVlcTable = uiCoeffCnt-1;
  if( uiMaxCoeffs <= 4 )
  {
    xGetTotalRun4( uiVlcTable, uiTotalRun );
  }
  else
  {
    xGetTotalRun16( uiVlcTable, uiTotalRun );
  }

  // decode run before each coefficient
  for ( UInt i = 0; i < uiCoeffCnt; i++ )
  {
    aiLevelRun[i + 0x10] = 0;
  }
  uiCoeffCnt--;
  UInt uiRunCount = uiTotalRun;
  if( uiRunCount > 0 && uiCoeffCnt > 0)
  {
    do
    {
      uiVlcTable = (( uiRunCount > RUNBEFORE_NUM) ? RUNBEFORE_NUM : uiRunCount) - 1;
      UInt uiRun = 0;
      
      xGetRun( uiVlcTable, uiRun );
      aiLevelRun[uiCoeffCnt+0x10] = uiRun;

      uiRunCount -= uiRun;
      uiCoeffCnt--;
    } while( uiRunCount != 0 && uiCoeffCnt != 0);
  }

  return Err::m_nOK;
}

ErrVal UvlcReader::xGetTrailingOnes4( UInt& uiCoeffCount, UInt& uiTrailingOnes )
{
  RNOK( xCodeFromBitstream2D( &g_aucCodeTableTO4[0][0], &g_aucLenTableTO4[0][0], 5, 4, uiCoeffCount, uiTrailingOnes ) );

  DTRACE_POS;
  DTRACE_T( "  TrailingOnes4: CoeffCnt: " );
  DTRACE_V( uiCoeffCount );
  DTRACE_T( " TraiOnes: " );
  DTRACE_V( uiTrailingOnes );
  DTRACE_N;
  DTRACE_COUNT(g_aucLenTableTO4[uiTrailingOnes][uiCoeffCount]);

  return Err::m_nOK;
}


ErrVal UvlcReader::xGetTotalRun4( UInt& uiVlcPos, UInt& uiTotalRun )
{
  UInt uiTemp;
  RNOK( xCodeFromBitstream2D( &g_aucCodeTableTZ4[uiVlcPos][0], &g_aucLenTableTZ4[uiVlcPos][0], 4, 1, uiTotalRun, uiTemp ) );

  DTRACE_POS;
  DTRACE_T( "  TotalZeros4 vlc: " );
  DTRACE_V( uiVlcPos );
  DTRACE_T( " TotalRun: " );
  DTRACE_V( uiTotalRun );
  DTRACE_N;
  DTRACE_COUNT(g_aucLenTableTZ4[uiVlcPos][uiTotalRun]);

  return Err::m_nOK;
}


ErrVal UvlcReader::xGetTotalRun16( UInt uiVlcPos, UInt& uiTotalRun )
{
  UInt uiTemp;
  RNOK( xCodeFromBitstream2D( &g_aucCodeTableTZ16[uiVlcPos][0], &g_aucLenTableTZ16[uiVlcPos][0], 16, 1, uiTotalRun, uiTemp ) );

  DTRACE_POS;
  DTRACE_T( "  TotalRun16 vlc: " );
  DTRACE_V( uiVlcPos );
  DTRACE_T( " TotalRun: " );
  DTRACE_V( uiTotalRun );
  DTRACE_N;
  DTRACE_COUNT(g_aucLenTableTZ16[uiVlcPos][uiTotalRun]);

  return Err::m_nOK;
}


ErrVal UvlcReader::xGetRun( UInt uiVlcPos, UInt& uiRun  )
{
  UInt uiTemp;

  RNOK( xCodeFromBitstream2D( &g_aucCodeTable3[uiVlcPos][0], &g_aucLenTable3[uiVlcPos][0], 15, 1, uiRun, uiTemp ) );

  DTRACE_POS;
  DTRACE_T( "  Run" );
  DTRACE_CODE( uiRun );
  DTRACE_COUNT (g_aucLenTable3[uiVlcPos][uiRun]);
  DTRACE_N;

  return Err::m_nOK;
}

ErrVal UvlcReader::xGetLevelVLC0( Int& iLevel )
{
  UInt uiLength = 0;
  UInt uiCode   = 0;
  UInt uiTemp   = 0;
  UInt uiSign   = 0;
  UInt uiLevel  = 0;

  do
  {
    RNOK( m_pcBitReadBuffer->get( uiTemp, 1 ) );
    uiLength++;
    uiCode = ( uiCode << 1 ) + uiTemp;
  } while ( uiCode == 0 );

  if ( uiLength < 15 )
  {
    uiSign  = (uiLength - 1) & 1;
    uiLevel = (uiLength - 1) / 2 + 1;
  }
  else if (uiLength == 15)
  {
    // escape code
    RNOK( m_pcBitReadBuffer->get( uiTemp, 4 ) );
    uiCode = (uiCode << 4) | uiTemp;
    uiLength += 4;
    uiSign  = (uiCode & 1);
    uiLevel = ((uiCode >> 1) & 0x7) + 8;
  }
  else if (uiLength >= 16)
  {
    // escape code
    UInt uiAddBit = uiLength - 16;
    RNOK( m_pcBitReadBuffer->get( uiCode, uiLength-4 ) );
    uiLength -= 4;
    uiSign    = (uiCode & 1);
    uiLevel   = (uiCode >> 1) + (2048<<uiAddBit)+16-2048;
    uiCode   |= (1 << (uiLength)); // for display purpose only
    uiLength += uiAddBit + 16;
 }

  iLevel = uiSign ? -(Int)uiLevel : (Int)uiLevel;

  DTRACE_POS;
  DTRACE_T( "  VLC0 lev " );
  DTRACE_CODE( iLevel );
  DTRACE_N;
  DTRACE_COUNT( uiLength );

  return Err::m_nOK;

}

ErrVal UvlcReader::xGetLevelVLCN( Int& iLevel, UInt uiVlcLength )
{  
  UInt uiTemp;
  UInt uiLength;
  UInt uiCode;
  UInt uiLevAbs;
  UInt uiSb;
  UInt uiSign;
  UInt uiAddBit;
  UInt uiOffset;
  
  UInt uiNumPrefix = 0;
  UInt uiShift     = uiVlcLength - 1;
  UInt uiEscape    = (15<<uiShift)+1;
  
  // read pre zeros
  do
  {
    RNOK( m_pcBitReadBuffer->get( uiTemp, 1 ) );
    uiNumPrefix++;
  } while ( uiTemp == 0 );

  uiLength = uiNumPrefix;
  uiCode   = 1;
  uiNumPrefix--;
  
  if (uiNumPrefix < 15)
  {
    uiLevAbs = (uiNumPrefix<<uiShift) + 1;
    
    if ( uiVlcLength-1 )
    {
      RNOK( m_pcBitReadBuffer->get( uiSb, uiVlcLength-1 ) );
      uiCode = (uiCode << (uiVlcLength-1) )| uiSb;
      uiLevAbs += uiSb;
      uiLength += (uiVlcLength-1);

    }

    // read 1 bit -> sign
    RNOK( m_pcBitReadBuffer->get( uiSign, 1 ) );
    uiCode = (uiCode << 1)| uiSign;
    uiLength++;
  }
  else // escape
  {
    uiAddBit = uiNumPrefix - 15;

    RNOK( m_pcBitReadBuffer->get( uiSb, (11+uiAddBit) ) );
    uiCode = (uiCode << (11+uiAddBit) )| uiSb;

    uiLength += (11+uiAddBit);
    uiOffset  = (2048<<uiAddBit)+uiEscape-2048;
    uiLevAbs  = uiSb + uiOffset;

    // read 1 bit -> sign
    RNOK( m_pcBitReadBuffer->get( uiSign, 1 ) );
    uiCode = (uiCode << 1)| uiSign;
    uiLength++;
  }
  
  iLevel = (uiSign) ? -(Int)uiLevAbs : (Int)uiLevAbs;

  DTRACE_POS;
  DTRACE_T( "  VLCN lev: " );
  DTRACE_CODE( iLevel );
  DTRACE_N;
  DTRACE_COUNT( uiLength );

  return Err::m_nOK;
}

Bool UvlcReader::isEndOfSlice()
{
  UInt uiEOS = ( m_uiRun > 1 ) ? 0 : !( moreRBSPData() );
  return (uiEOS == 1);
}

ErrVal UvlcReader::finishSlice( )
{
  if( m_bRunLengthCoding && m_uiRun )
  {
    DTRACE_T( "Run" );
    RNOK( xGetUvlcCode( m_uiRun ) );
    DTRACE_N;
  }

  return Err::m_nOK;
}

ErrVal UvlcReader::residualBlock8x8( MbDataAccess&  rcMbDataAccess,
                                     B8x8Idx        c8x8Idx )
{
  const Bool bFrame = ( FRAME == rcMbDataAccess.getMbPicType());
  const UChar* pucScan = (bFrame) ? g_aucFrameScan64 : g_aucFieldScan64;
  TCoeff* piCoeff = rcMbDataAccess.getMbTCoeffs().get8x8( c8x8Idx );

  UInt  uiBlk;
  UInt  uiPos;

  Int   aaiLevelRun     [4][32];
  UInt  auiTrailingOnes [4]     = { 0, 0, 0, 0 };
  UInt  auiTotalRun     [4]     = { 0, 0, 0, 0 };
  UInt  auiCoeffCnt     [4]     = { 0, 0, 0, 0 };

  //Heiner Kirchhoffer fix  
  for ( uiBlk  = 0; uiBlk < 4; uiBlk++ )
   for ( uiPos = 0; uiPos < 32; uiPos++ )
     aaiLevelRun[uiBlk][uiPos] = 0;
  //
  
  {
    UInt uiBitPos = c8x8Idx;
    rcMbDataAccess.getMbData().setBCBP( uiBitPos,   1);
    rcMbDataAccess.getMbData().setBCBP( uiBitPos+1, 1);
    rcMbDataAccess.getMbData().setBCBP( uiBitPos+4, 1);
    rcMbDataAccess.getMbData().setBCBP( uiBitPos+5, 1);
  }
  //===== loop over 4x4 blocks =====
  for( uiBlk = 0; uiBlk < 4; uiBlk++ )
  {
    B4x4Idx cIdx( c8x8Idx.b4x4() + 4*(uiBlk/2) + (uiBlk%2) );

    xPredictNonZeroCnt( rcMbDataAccess, cIdx, auiCoeffCnt[uiBlk], auiTrailingOnes[uiBlk] );
    xGetRunLevel      ( aaiLevelRun[uiBlk],   auiCoeffCnt[uiBlk], auiTrailingOnes[uiBlk], 16, auiTotalRun[uiBlk] );

    uiPos = ((auiTotalRun[uiBlk] + auiCoeffCnt[uiBlk] - 1) << 2) + uiBlk;
    for ( Int i = (Int)auiCoeffCnt[uiBlk]; i > 0; i-- )
    {
      piCoeff[ pucScan [uiPos] ] = aaiLevelRun[uiBlk][i-1];
      uiPos -= 4;
      for ( Int j = 0; j < aaiLevelRun[uiBlk][i-1+0x10]; j++ )
      {
        piCoeff[ pucScan [uiPos] ] = 0;
        uiPos -= 4;
      }
    }

  }


  return Err::m_nOK;
}

Bool
UvlcReader::RQpeekCbp4x4(      MbDataAccess&  rcMbDataAccessBase,
                               LumaIdx        cIdx )
{
  UInt    uiSymbol  = 0;

  uiSymbol = rcMbDataAccessBase.getMbData().getBCBP( cIdx );
  
  return ( uiSymbol == 1 );
}


ErrVal
UvlcReader::RQdecodeNewTCoeff_8x8( MbDataAccess&   rcMbDataAccess,
                                   MbDataAccess&   rcMbDataAccessBase,
                                   B8x8Idx         c8x8Idx,
                                   UInt            uiScanIndex,
                                   Bool&           rbLast,
                                   UInt&           ruiNumCoefRead )
{
  TCoeff*       piCoeff     = rcMbDataAccess    .getMbTCoeffs().get8x8( c8x8Idx );
  TCoeff*       piCoeffBase = rcMbDataAccessBase.getMbTCoeffs().get8x8( c8x8Idx );
  const Bool    bFrame      = ( FRAME == rcMbDataAccess.getMbPicType());
  const UChar*  pucScan     = (bFrame) ? g_aucFrameScan64 : g_aucFieldScan64;


  ROT( piCoeffBase[pucScan[uiScanIndex]] );

  DTRACE_T( "LUMA_8x8_NEW" );
  DTRACE_V( c8x8Idx.b8x8Index() );
  DTRACE_V( uiScanIndex );
  DTRACE_N;

  rcMbDataAccessBase.getMbData().setBCBP( c8x8Idx.b4x4(),   1 );
  rcMbDataAccessBase.getMbData().setBCBP( c8x8Idx.b4x4()+1, 1 );
  rcMbDataAccessBase.getMbData().setBCBP( c8x8Idx.b4x4()+4, 1 );
  rcMbDataAccessBase.getMbData().setBCBP( c8x8Idx.b4x4()+5, 1 );

  UInt auiEobShift[16];
  memset(auiEobShift, 0, sizeof(UInt)*16);

  RNOK( xRQdecodeNewTCoeffs( piCoeff, piCoeffBase, uiScanIndex%4, 64, 4, pucScan, uiScanIndex, auiEobShift, rbLast, ruiNumCoefRead ) );

  return Err::m_nOK;
}



ErrVal
UvlcReader::RQdecodeTCoeffRef_8x8( MbDataAccess&   rcMbDataAccess,
                                    MbDataAccess&   rcMbDataAccessBase,
                                    B8x8Idx         c8x8Idx,
                                  UInt            uiScanIndex,
                                  UInt            uiCtx )
{
  TCoeff*       piCoeff     = rcMbDataAccess    .getMbTCoeffs().get8x8( c8x8Idx );
  TCoeff*       piCoeffBase = rcMbDataAccessBase.getMbTCoeffs().get8x8( c8x8Idx );
  const Bool    bFrame      = ( FRAME == rcMbDataAccess.getMbPicType());
  const UChar*  pucScan     = (bFrame) ? g_aucFrameScan64 : g_aucFieldScan64;

  DTRACE_T( "LUMA_8x8_REF" );
  DTRACE_V( c8x8Idx.b8x8Index() );
  DTRACE_V( uiScanIndex );
  DTRACE_N;

  RNOKS( xRQdecodeTCoeffsRef( piCoeff, piCoeffBase, pucScan, uiScanIndex ) );

  return Err::m_nOK;
}

ErrVal
UvlcReader::RQdecodeNewTCoeff_Luma ( MbDataAccess&   rcMbDataAccess,
                                      MbDataAccess&   rcMbDataAccessBase,
                                      ResidualMode    eResidualMode,
                                      Bool            b8x8,
                                      LumaIdx         cIdx,
                                      UInt            uiScanIndex,
                                      Bool&           rbLast,
                                      UInt&           ruiNumCoefRead )
{
  TCoeff*       piCoeff;
  TCoeff*       piCoeffBase;
  const Bool    bFrame      = ( FRAME == rcMbDataAccess.getMbPicType());
  const UChar*  pucScan     = (bFrame) ? g_aucFrameScan : g_aucFieldScan;
  UInt          uiStart;
  UInt          uiStop;
  UInt          uiStride;

  if( b8x8 )
  {
    UInt    uiOffset8x8   = (cIdx.y() % 2) * 2 + (cIdx.x() % 2);
    B8x8Idx c8x8Idx(cIdx);

    piCoeff     = rcMbDataAccess    .getMbTCoeffs().get8x8( c8x8Idx );
    piCoeffBase = rcMbDataAccessBase.getMbTCoeffs().get8x8( c8x8Idx );
    pucScan     = g_aucFrameScan64;
    uiStart     = uiOffset8x8;
    uiStop      = 64;
    uiStride    = 4;
    uiScanIndex = uiScanIndex * 4 + uiOffset8x8;
  }
  else
  {
    piCoeff     = rcMbDataAccess    .getMbTCoeffs().get( cIdx );
    piCoeffBase = rcMbDataAccessBase.getMbTCoeffs().get( cIdx );
    pucScan     = g_aucFrameScan;
    uiStart     = 0;
    uiStop      = 16;
    uiStride    = 1;
  }

  DTRACE_T( "LUMA_4x4_NEW" );
  DTRACE_V( cIdx.b4x4() );
  DTRACE_V( uiScanIndex );
  DTRACE_N;

  RNOKS( xRQdecodeNewTCoeffs( piCoeff, piCoeffBase, uiStart, uiStop, uiStride, pucScan, uiScanIndex, m_auiShiftLuma, rbLast, ruiNumCoefRead ) );

  return Err::m_nOK;
}


ErrVal
UvlcReader::RQdecodeTCoeffRef_Luma ( MbDataAccess&   rcMbDataAccess,
                                     MbDataAccess&   rcMbDataAccessBase,
                                     LumaIdx         cIdx,
                                     UInt            uiScanIndex,
                                     UInt            uiCtx )
{
  TCoeff*       piCoeff     = rcMbDataAccess    .getMbTCoeffs().get( cIdx );
  TCoeff*       piCoeffBase = rcMbDataAccessBase.getMbTCoeffs().get( cIdx );
  const Bool    bFrame      = ( FRAME == rcMbDataAccess.getMbPicType());
  const UChar*  pucScan     = (bFrame) ? g_aucFrameScan : g_aucFieldScan;

  DTRACE_T( "LUMA_4x4_REF" );
  DTRACE_V( cIdx.b4x4() );
  DTRACE_V( uiScanIndex );
  DTRACE_N;

  RNOKS( xRQdecodeTCoeffsRef( piCoeff, piCoeffBase, pucScan, uiScanIndex ) );

  return Err::m_nOK;
}

ErrVal
UvlcReader::RQdecodeNewTCoeff_Chroma( MbDataAccess&   rcMbDataAccess,
                                        MbDataAccess&   rcMbDataAccessBase,
                                        ResidualMode    eResidualMode,
                                        ChromaIdx       cIdx,
                                        UInt            uiScanIndex,
                                        Bool&           rbLast,
                                        UInt&           ruiNumCoefRead )
{
  TCoeff*       piCoeff     = rcMbDataAccess    .getMbTCoeffs().get( cIdx );
  TCoeff*       piCoeffBase = rcMbDataAccessBase.getMbTCoeffs().get( cIdx );
  const Bool    bFrame      = ( FRAME == rcMbDataAccess.getMbPicType());
  const UChar*  pucScan     = ( eResidualMode == CHROMA_DC ? g_aucIndexChromaDCScan : (bFrame) ? g_aucFrameScan : g_aucFieldScan );
  UInt          uiStart     = ( eResidualMode == CHROMA_AC ? 1 : 0  );
  UInt          uiStop      = ( eResidualMode == CHROMA_DC ? 4 : 16 );

  ROT( piCoeffBase[pucScan[uiScanIndex]] );

  DTRACE_T( "CHROMA_4x4_NEW" );
  DTRACE_V( cIdx );
  DTRACE_V( uiScanIndex );
  DTRACE_N;

  RNOKS( xRQdecodeNewTCoeffs( piCoeff, piCoeffBase, uiStart, uiStop, 1, pucScan, uiScanIndex, m_auiShiftChroma, rbLast, ruiNumCoefRead ) );

  return Err::m_nOK;
}

ErrVal
UvlcReader::RQdecodeTCoeffRef_Chroma ( MbDataAccess&   rcMbDataAccess,
                                       MbDataAccess&   rcMbDataAccessBase,
                                       ResidualMode    eResidualMode,
                                       ChromaIdx       cIdx,
                                       UInt            uiScanIndex,
                                       UInt            uiCtx )
{
  TCoeff*       piCoeff     = rcMbDataAccess    .getMbTCoeffs().get( cIdx );
  TCoeff*       piCoeffBase = rcMbDataAccessBase.getMbTCoeffs().get( cIdx );
  const Bool    bFrame      = ( FRAME == rcMbDataAccess.getMbPicType());
  const UChar*  pucScan     = ( eResidualMode == CHROMA_DC ? g_aucIndexChromaDCScan : (bFrame) ? g_aucFrameScan : g_aucFieldScan );


  DTRACE_T( "CHROMA_4x4_REF" );
  DTRACE_V( cIdx );
  DTRACE_V( uiScanIndex );
  DTRACE_N;

  RNOKS( xRQdecodeTCoeffsRef( piCoeff, piCoeffBase, pucScan, uiScanIndex ) );

  return Err::m_nOK;
}

ErrVal
UvlcReader::xRQdecodeNewTCoeffs( TCoeff*       piCoeff,
                                  TCoeff*       piCoeffBase,
                                  UInt          uiStart,
                                  UInt          uiStop,
                                  UInt          uiStride,
                                  const UChar*  pucScan,
                                  UInt          uiScanIndex,
                                  UInt*         pauiEobShift,
                                  Bool&         rbLast,
                                  UInt&         ruiNumCoefRead )
{
  UInt ui=0;

  UInt uiCycle = uiStart/uiStride;
  for (ui=uiStart; ui<uiScanIndex; ui+=uiStride )
  {
    if ( piCoeffBase[pucScan[ui]] || piCoeff[pucScan[ui]] )
      uiCycle = ui/uiStride + 1;
  }

  AOF( uiCycle < uiStop );
  DTRACE_T("NewTCoeffs-uiCycle: ");
  DTRACE_V(uiCycle);
  DTRACE_N;
  ruiNumCoefRead = 0;

  Bool bSkipEob = !rbLast;
  UInt uiSymbol;
  RNOKS( xGetSigRunCode( uiSymbol, m_auiBestCodeTab[uiCycle] ) );

  if( rbLast )
  {
    // Determine "overshoot" symbol
    UInt uiOvershoot = 1;
    for( ui = uiStart; ui < uiStop; ui+=uiStride )
    {
      if ( ! piCoeffBase[pucScan[ui]] )
        uiOvershoot++;
      if( ui < uiScanIndex && piCoeff[pucScan[ui]] && ! piCoeffBase[pucScan[ui]])
      {
        uiOvershoot = 0;
      }
    }
    if ( uiSymbol > uiOvershoot )
    {
      RNOKS( xRQdecodeSigMagGreater1( piCoeff, piCoeffBase, pucScan, uiSymbol-uiOvershoot-1, uiStart, uiStop, uiStride ) );
      rbLast = true;
    } else {
      rbLast = uiSymbol == min( pauiEobShift[uiCycle], uiOvershoot );
    }
    ROTRS(rbLast, Err::m_nOK);
  } else
    rbLast = false;

  //===== SIGNIFICANCE BIT ======
  UInt uiNumCoef = uiSymbol + ((bSkipEob || uiSymbol <= pauiEobShift[uiCycle]) ? 1 : 0);
  AOT( uiNumCoef > uiStop );
  do
  {
    ruiNumCoefRead++;

    if ( --uiNumCoef == 0 )
    {
      break;
    }

    uiScanIndex += uiStride;
    while (uiScanIndex < uiStop && piCoeffBase[pucScan[uiScanIndex]])
      uiScanIndex += uiStride;
  }
  while ( true );
  RNOKS( xGetFlag( uiSymbol ) );
  piCoeff[pucScan[uiScanIndex]] = uiSymbol ? -1 : 1;

  // Check whether any more nonzero values
  Bool bFinished = true;
  for( ui=uiScanIndex+uiStride; ui<uiStop; ui+=uiStride )
  {
    bFinished &= ( piCoeffBase[pucScan[ui]] != 0 );
    if( !bFinished )
      break;
  }
  if( bFinished )
  {
    uiSymbol = 0;
    RNOKS( xGetSigRunCode( uiSymbol, m_auiBestCodeTab[uiCycle] ) );
    if( uiSymbol > 0 )
    {
      RNOKS( xRQdecodeSigMagGreater1( piCoeff, piCoeffBase, pucScan, uiSymbol-1, uiStart, uiStop, uiStride ) );
    }
  }
  return Err::m_nOK;
}

ErrVal
UvlcReader::xRQdecodeSigMagGreater1( TCoeff* piCoeff,
                                     TCoeff* piCoeffBase,
                                     const UChar* pucScan,
                                     UInt    uiTermSym,
                                     UInt    uiStart,
                                     UInt    uiStop,
                                     UInt    uiStride )
{
  // Find optimal terminating code
  UInt ui;
  UInt uiCountMag1 = 0;
  for (ui=uiStart; ui<uiStop; ui+=uiStride )
  {
    if ( !piCoeffBase[pucScan[ui]] && piCoeff[pucScan[ui]] )
    {
      uiCountMag1++;
    }
  }
  UInt uiMaxMag;
  UInt uiCountMag2;
  if ( uiTermSym < uiCountMag1*2 )
  {
    uiMaxMag    = (uiTermSym % 2) + 2;
    uiCountMag2 = (uiTermSym / 2) + 1;
  } else {
    uiMaxMag    = (uiTermSym / uiCountMag1) + 2;
    uiCountMag2 = (uiTermSym % uiCountMag1) + 1;
  }
  UInt auiRemMag[16];
  ::memset(auiRemMag, 0x0, 16*sizeof(UInt));
  UInt uiFlip      = 0;
  UInt uiRemaining = uiCountMag2;
  UInt uiEnd       = uiCountMag1;
  UInt uiCount     = 0;
  for (ui=uiStart; ui<uiStop; ui+=uiStride )
  {
    if( piCoeff[pucScan[ui]] && ! piCoeffBase[pucScan[ui]])
    {
      if( uiRemaining+uiCount == uiEnd )
      {
        auiRemMag[ui/uiStride] = 1-uiFlip;
        uiRemaining--;
      } else {
        RNOKS( xGetFlag( auiRemMag[ui/uiStride] ) );
        uiRemaining -= auiRemMag[ui/uiStride] ? 1-uiFlip : uiFlip;
      }
      if( uiRemaining == 0 )
        break;
      uiCount++;
    }
  }

  UInt uiOutstanding = uiCountMag2;
  Bool bSeenMaxMag   = false;
  for(ui = uiStart; ui < uiStop; ui+=uiStride )
  {
    if( !bSeenMaxMag && uiOutstanding == 1 )
      break;
    if( auiRemMag[ui/uiStride] )
    {
      UInt uiBit;
      UInt uiSymbol = 0;
      for ( UInt uiCutoff=1; uiCutoff<uiMaxMag; uiCutoff++ )
      {
        auiRemMag[ui/uiStride] = 0;
        RNOKS( xGetFlag( uiBit ) );
        if( uiBit )
          uiSymbol++;
        else
          break;
      }
      uiSymbol += 2;
      piCoeff[pucScan[ui]] = (piCoeff[pucScan[ui]] > 0) ? (Int)uiSymbol : -(Int)uiSymbol;
      bSeenMaxMag |= ( uiSymbol == uiMaxMag );
      uiOutstanding--;
      if( uiOutstanding == 0 )
        break;
    }
  }
  for (ui=uiStart; ui<uiStop; ui+=uiStride )
  {
    if ( auiRemMag[ui/uiStride] )
    {
      piCoeff[pucScan[ui]] = ( piCoeff[pucScan[ui]] > 0 ) ? (Int)uiMaxMag : -(Int)uiMaxMag;
    }
  }

  return Err::m_nOK;
}

ErrVal
UvlcReader::xRQdecodeTCoeffsRef( TCoeff*       piCoeff,
                                 TCoeff*       piCoeffBase,
                                 const UChar*  pucScan,
                                 UInt          uiScanIndex )
{
  UInt    uiBaseSign;
  TCoeff* piCoeffPtr;

  uiBaseSign = (piCoeffBase[pucScan[uiScanIndex]] < 0) ? 1 : 0;
  piCoeffPtr = & piCoeff[pucScan[uiScanIndex]];
  RNOKS( m_pSymGrp->xFetchSymbol( this, uiBaseSign, piCoeffPtr, "REF_COEFF" ) );

  return Err::m_nOK;
}


ErrVal
UvlcReader::xGetGolomb(UInt& uiSymbol, UInt uiK)
{
  UInt uiCode;
  UInt uiR;
  UInt uiQ = 0;
  UInt uiC = 0;
  UInt uiT = uiK >> 1;

  while ( uiT > 0 )
  {
    uiC++;
    uiT >>= 1;
  }

  // Unary part
  do {
    RNOKS( xGetFlag( uiCode ) );
    uiQ++;
  } while ( uiCode != 0 );
  uiQ--;

  uiSymbol = uiQ * uiK;

  if ( uiC == 0 )
  {
    return Err::m_nOK;
  }

  // Binary part
  RNOKS( xGetFlag( uiCode ) );
  if ( uiCode == 0 )
  {
    if ( uiC > 1 )
    {
      RNOKS( xGetCode( uiR, uiC-1 ) );
    } else {
      uiR = 0;
    }
  } else {
    RNOKS( xGetCode( uiCode, uiC ) );
    uiR = uiCode + uiC;
  }
  DTRACE_N;

  uiSymbol += uiR;

  return Err::m_nOK;
}

ErrVal
UvlcReader::RQdecodeEobOffsets_Luma()
{
  RNOK( m_pSymGrp->Init() );
  m_uiCbpStat4x4[0] = m_uiCbpStat4x4[1] = 0;
  m_uiCbpStats[0][0] = m_uiCbpStats[0][1] = m_uiCbpStats[1][0] = m_uiCbpStats[1][1] = 0;

  RNOKS( xRQdecodeEobOffsets( m_auiShiftLuma, 16 ) );
  return Err::m_nOK;
}

ErrVal
UvlcReader::RQdecodeEobOffsets_Chroma()
{
  m_auiShiftChroma[0] = 15;
  RNOK( xRQdecodeEobOffsets( m_auiShiftChroma+1, 15 ) );

  return Err::m_nOK;
}

ErrVal
UvlcReader::xRQdecodeEobOffsets( UInt* pauiShift, UInt uiMax )
{
  UInt uiNumEnd = 1;
  DTRACE_T("num_end_vals");
  for (UInt uiEc=0; uiEc<3; uiEc++)
  {
    UInt uiCode;
    RNOKS( xGetFlag( uiCode ) );
    if (uiCode)
    {
      uiNumEnd++;
    } else {
      break;
    }
  }
  DTRACE_V(uiNumEnd-1);
  DTRACE_N;

  if (uiNumEnd == 4)
  {
    uiNumEnd = 0;
  }
  for (UInt ui=0; ui<uiNumEnd; ui++)
  {
    pauiShift[ui] = uiMax - 1;
  }
  UInt uiLevel;
  DTRACE_T("eobShiftXXX[ num_end_vals ]");
  RNOKS( xGetGolomb( uiLevel, 2 ) );
  pauiShift[uiNumEnd] = uiLevel;
  
  RNOKS( xDecodeMonSeq( pauiShift+uiNumEnd+1, uiLevel, uiMax-uiNumEnd-1 ) );

  return Err::m_nOK;
}


ErrVal
UvlcReader::RQdecodeBestCodeTableMap( UInt uiMaxH )
{
  UInt uiW = uiMaxH-1;

  memset(m_auiBestCodeTab, 0, sizeof(UInt)*uiMaxH);

  DTRACE_T("num_sig_vlc_selectors");
  RNOKS( xGetCode(uiW, 4) );

  DTRACE_TY("u(4) ");
  DTRACE_V(uiW);
  DTRACE_N;
  DTRACE_T("sig_vlc_selector[i]");
  for(UInt uiH = 0; uiH <= uiW; uiH++)
  {
    RNOKS(xGetSigRunTabCode(m_auiBestCodeTab[uiH]));
  }
  DTRACE_N;

  return Err::m_nOK;
}

ErrVal
UvlcReader::RQvlcFlush()
{
  return Err::m_nOK;
}

ErrVal
UvlcReader::RQupdateVlcTable()
{
  m_pSymGrp->UpdateVlc();

  return Err::m_nOK;
}


ErrVal
UvlcReader::xDecodeMonSeq ( UInt* auiSeq, UInt uiStart, UInt uiLen )
{
  UInt uiPos   = 0;
  UInt uiLevel = uiStart;
  while ( uiLevel > 0 && uiPos < uiLen )
  {
    UInt uiRun;
    DTRACE_T("eob_run");
    RNOKS( xGetGolomb( uiRun, 1 ) );
    for (UInt ui=0; ui<uiRun; ui++,uiPos++)
      auiSeq[uiPos] = uiLevel;
    uiLevel--;
  }
  for (; uiPos < uiLen; uiPos++)
  {
    auiSeq[uiPos] = 0;
  }
  
  return Err::m_nOK;
}

ErrVal 
UvlcReader::xGetSigRunCode( UInt& uiVal, UInt uiCodeTab )
{
  if( uiCodeTab == 0)
  {
    RNOKS(xGetUnaryCode( uiVal ));
  }
  else if( uiCodeTab == 1)
  {
    RNOKS(xGetCodeCB1( uiVal ));
  }
  else if( uiCodeTab == 2)
  {
    RNOKS(xGetCodeCB2( uiVal ));
  }
  else if( uiCodeTab == 3)
  {
    UInt uiCode; 

    RNOKS( xGetFlag( uiCode ) );

    if(uiCode == 1)
    {
      uiVal = 0;
    }
    else
    {
      RNOKS( xGetCodeCB2( uiCode ) );
      uiVal = uiCode+1;
    }
  }
  else
  {
    UInt uiCode; 

    RNOKS( xGetFlag( uiCode ) );

    if(uiCode == 1)
    {
      uiVal = 0;
    }
    else
    {
      RNOKS( xGetCodeCB1( uiCode ) );
      uiVal = uiCode+1;
    }
  }

  return Err::m_nOK;
}


ErrVal 
UvlcReader::xGetCodeCB1 ( UInt& uiVal )
{
  UInt uiPrefixLen = 0;
  UInt uiFlag;

  do
  {
    RNOKS(xGetFlag(uiFlag));
    uiPrefixLen++;
  }
  while (uiFlag == 0);
  RNOKS( xGetFlag( uiFlag) );
  
  uiVal = (2*(uiPrefixLen-1))+(1-uiFlag);
  return Err::m_nOK;
}

ErrVal 
UvlcReader::xGetCodeCB2( UInt& uiVal )
{
  UInt uiPrefixLen = 0;
  UInt uiCode;

  do
  {
    RNOKS( xGetCode( uiCode, 2) );
    uiPrefixLen++;
  }
  while (uiCode == 0);

  uiVal = (3*(uiPrefixLen-1))+(3-uiCode);
  return Err::m_nOK;
}

ErrVal 
UvlcReader::xGetUnaryCode( UInt& uiVal )
{
  UInt uiCode;
  UInt uiSymbol = 0;

  do
  {
    RNOKS( xGetFlag( uiCode ) );
    if(uiCode == 1)
    {
      uiVal = uiSymbol;
      break;
    }
    else
    {
      uiSymbol++;
    }
  }
  while (true);
  return Err::m_nOK;
}

ErrVal
UvlcReader::xGetSigRunTabCode(UInt &uiTab)
{
  UInt uiFlag;

  RNOKS(xGetFlag(uiFlag));
  if(uiFlag == 1)
  {
    uiTab = 0;
  }
  else
  {
    RNOKS(xGetFlag(uiFlag));
    if(uiFlag == 1)
    {
      uiTab = 1;
    }
    else 
    {
      RNOKS(xGetFlag(uiFlag));
      if(uiFlag == 1)
      {
        uiTab  = 2;
      }
      else
      {
        RNOKS(xGetFlag(uiFlag));
        uiTab = 4-uiFlag;
      }
    }
  }
  return Err::m_nOK;
}

ErrVal
UvlcReader::RQcompSepAlign()
{
  RNOKS( m_pcBitReadBuffer->getBitsUntilByteAligned() );

  return Err::m_nOK;
}


ErrVal
UvlcReader::RQreset( const SliceHeader& rcSliceHeader )
{
  m_pSymGrp->Init();
  return Err::m_nOK;
}


UcSymGrpReader::UcSymGrpReader( UvlcReader* pParent )
{
  Init();
}

ErrVal
UcSymGrpReader::Init()
{
  m_uiCode         = 0;
  m_uiLen          = CAVLC_SYMGRP_SIZE;
  m_auiSymCount[0] = m_auiSymCount[1] = m_auiSymCount[2] = 0;
  m_uiTable = 0;
  m_uiCodedFlag    = false;

  return Err::m_nOK;
}

ErrVal
// actually used only for refinement coefficients
UcSymGrpReader::xFetchSymbol( UvlcReader* pcUvlcReader,
                              UInt        uiBaseSign,
                              TCoeff*     piCoeffPtr,
                              Char*       pcTraceString )
{
  if( m_uiLen == CAVLC_SYMGRP_SIZE )
  {
    UInt uiTemp;
   
    m_uiCodedFlag = true;
    RNOKS( pcUvlcReader->codeFromBitstream2Di( g_auiRefSymCode[m_uiTable], g_auiRefSymLen[m_uiTable], 27, 1, m_uiCode, uiTemp ) );

    UInt uiCode = m_uiCode;
    UInt uiDenom = 9;

    for(UInt ui = 0; ui < CAVLC_SYMGRP_SIZE; ui++)
    {
      UChar ucSym;
      ucSym = uiCode/uiDenom;
      m_auiSymbolBuf[ui] = ucSym;
      m_auiSymCount[ucSym]++;

      uiCode = (uiCode % uiDenom);
      uiDenom /= 3;
    }
  
    m_uiLen   = 0;
    m_uiCode  = 0;
  }

  *piCoeffPtr = 0;
  if (m_auiSymbolBuf[m_uiLen] > 0) {
    UInt uiSymbol = m_auiSymbolBuf[m_uiLen] - 1;
    UInt uiSignEL = ( uiBaseSign ^ uiSymbol );
    *piCoeffPtr   = ( uiSignEL ? -1 : 1 );
  }

  m_uiLen ++;

  return Err::m_nOK;
}

Bool
UcSymGrpReader::UpdateVlc()
{
  UInt uiFlag = m_uiCodedFlag;
  if (uiFlag) {
    // updating
    m_uiTable = 0;
    if (m_auiSymCount[0] < 2 *(m_auiSymCount[1] + m_auiSymCount[2]) ||
      m_auiSymCount[1] < 2 * m_auiSymCount[2]) {
      m_uiTable = 1;
    }

    // scaling
    m_auiSymCount[0] = (m_auiSymCount[0] >> 1);
    m_auiSymCount[1] = (m_auiSymCount[1] >> 1);
    m_auiSymCount[2] = (m_auiSymCount[2] >> 1);
    m_uiCodedFlag = false;
    
    m_uiLen = CAVLC_SYMGRP_SIZE;
  }
  return (uiFlag != 0);
}

ErrVal
UcSymGrpReader::Flush()
{
  m_uiCode         = 0;

  m_uiLen          = CAVLC_SYMGRP_SIZE;

  return Err::m_nOK;
}
ErrVal
UvlcReader::RQdecodeCycleSymbol( UInt& uiCycle )
{
  RNOKS( xGetFlag( uiCycle ) );
  if ( uiCycle > 0 )
  {
    UInt uiTemp;
    RNOKS( xGetFlag( uiTemp ) );
    uiCycle += uiTemp;
  }
  uiCycle++;
  return Err::m_nOK;
}


ErrVal
UvlcReader::RQdecodeDeltaQp( MbDataAccess& rcMbDataAccess)
{
  Int   iDQp = 0;

  DTRACE_T ("DQp");

  RNOKS( xGetSvlcCode( iDQp ) );

  DTRACE_TY ("se(v)");
  DTRACE_N;

  rcMbDataAccess.addDeltaQp( iDQp );

  return Err::m_nOK;
}


ErrVal
UvlcReader::RQdecode8x8Flag( MbDataAccess& rcMbDataAccess,
                             MbDataAccess& rcMbDataAccessBase ) 
{
  UInt uiSymbol = 0;
  
  RNOKS( xGetFlag( uiSymbol ) );
  DTRACE_T( "TRAFO_8x8" );
  DTRACE_V( uiSymbol );
  DTRACE_N;

  rcMbDataAccess    .getMbData().setTransformSize8x8( uiSymbol == 1 );
  rcMbDataAccessBase.getMbData().setTransformSize8x8( uiSymbol == 1 );

  return Err::m_nOK;
}


ErrVal
UvlcReader::RQdecodeCBP_8x8( MbDataAccess& rcMbDataAccess,
                             MbDataAccess& rcMbDataAccessBase,
                             B8x8Idx       c8x8Idx )
{
  UInt  uiSymbol        = 0;
  m_uiCbp8x8 = rcMbDataAccess.getMbData().getMbCbp() & 0x0F;

  uiSymbol = (m_uiCbp8x8 >> c8x8Idx.b8x8Index()) & 0x1;

  if ( uiSymbol )
  {
    rcMbDataAccessBase.getMbData().setMbCbp( rcMbDataAccessBase.getMbData().getMbCbp() | ( 1 << c8x8Idx.b8x8Index() ) );
  }

  DTRACE_T( "CBP_Luma" );
  DTRACE_V( uiSymbol );
  DTRACE_N;

  return Err::m_nOK;
}


ErrVal
UvlcReader::RQdecodeBCBP_4x4(  MbDataAccess&  rcMbDataAccess,
                               MbDataAccess&  rcMbDataAccessBase,
                               Bool           b8x8,
                               LumaIdx        cIdx,
                               UInt&          ruiSymbol )
{
  ruiSymbol = 0;

  if ( (cIdx.x() %2) == 0 && (cIdx.y() %2) == 0)
  {
    // Write
    UInt  uiCode  = 0;
    UInt  uiLen   = 4;
    UInt  uiFlip  = (m_uiCbpStat4x4[1] > m_uiCbpStat4x4[0]) ? 1 : 0;
    UInt  uiVlc   = (m_uiCbpStat4x4[uiFlip] < 2*m_uiCbpStat4x4[1-uiFlip]) ? 0 : 2;

    if (uiVlc == 0)
    {
      RNOKS( xGetCode( uiCode, uiLen ) );
    } else {
      UInt uiTemp;
      RNOKS( xCodeFromBitstream2Di( g_auiISymCode[2], g_auiISymLen[2], 1<<uiLen, 1, uiCode, uiTemp ) );
    }
    if (uiFlip)
      uiCode = uiCode ^ ((1<<uiLen)-1);

    m_uiCurrCbp4x4 = 0;
    UInt ui = uiLen;
    for( Int uiY=cIdx.y(); uiY<cIdx.y()+2; uiY++)
      for( Int uiX=cIdx.x(); uiX<cIdx.x()+2; uiX++)
      {
        UInt uiSymbol = 0;
        B4x4Idx cTmp(uiY*4+uiX);
        ui--;
        uiSymbol = (uiCode >> ui) & 0x1;
        rcMbDataAccess.getMbData().setBCBP( cTmp, uiSymbol );
        rcMbDataAccessBase.getMbData().setBCBP( cTmp, uiSymbol );
        m_uiCurrCbp4x4 |= uiSymbol<<cTmp;
        m_uiCbpStat4x4[uiSymbol]++;
      }
    // Scaling
    if (m_uiCbpStat4x4[0]+m_uiCbpStat4x4[1] > 512)
    {
      m_uiCbpStat4x4[0] >>= 1;
      m_uiCbpStat4x4[1] >>= 1;
    }
    DTRACE_T( "BCBP_4x4" );
    DTRACE_V( uiCode );
    DTRACE_N;
  }

  ruiSymbol = (m_uiCurrCbp4x4 >> cIdx) & 0x1;

  return Err::m_nOK;
}


ErrVal
UvlcReader::RQdecodeBCBP_ChromaDC(  MbDataAccess&   rcMbDataAccess,
                                    MbDataAccess&   rcMbDataAccessBase,
                                    ChromaIdx       cIdx,
                                    UInt&           ruiSymbol )
{
  ruiSymbol  = 0;

  RNOKS( xGetFlag( ruiSymbol ) );
  DTRACE_T( "BCBP_ChromaDC" );
  DTRACE_V( ruiSymbol );
  DTRACE_N;

  rcMbDataAccess.getMbData()    .setBCBP( 24 + cIdx.plane(), ruiSymbol );
  rcMbDataAccessBase.getMbData().setBCBP( 24 + cIdx.plane(), ruiSymbol );
  
  return Err::m_nOK;
}


ErrVal
UvlcReader::RQdecodeBCBP_ChromaAC(  MbDataAccess&  rcMbDataAccess,
                                    MbDataAccess&  rcMbDataAccessBase,
                                    ChromaIdx      cIdx,
                                    UInt&          ruiSymbol )
{
  ruiSymbol  = 0;

  RNOKS( xGetFlag( ruiSymbol ) );
  DTRACE_T( "BCBP_ChromaAC" );
  DTRACE_V( ruiSymbol );
  DTRACE_N;

  rcMbDataAccess.getMbData()    .setBCBP( 16 + cIdx, ruiSymbol );
  rcMbDataAccessBase.getMbData().setBCBP( 16 + cIdx, ruiSymbol );
  
  return Err::m_nOK;
}


ErrVal
UvlcReader::RQdecodeCBP_Chroma( MbDataAccess& rcMbDataAccess,
                                MbDataAccess& rcMbDataAccessBase,
                                UInt&         ruiSymbol )
{
  ruiSymbol = ((rcMbDataAccess.getMbData().getMbCbp()>>4) != 0);
  DTRACE_T( "CBP_Chroma" );
  DTRACE_V( ruiSymbol );
  DTRACE_N;

  if( ruiSymbol )
  {
    rcMbDataAccessBase.getMbData().setMbCbp( rcMbDataAccessBase.getMbData().getMbCbp() | 0x10 );
    rcMbDataAccess    .getMbData().setMbCbp( rcMbDataAccess    .getMbData().getMbCbp() | 0x10 );
  }

  return Err::m_nOK;
}


ErrVal
UvlcReader::RQdecodeCBP_ChromaAC( MbDataAccess& rcMbDataAccess,
                                  MbDataAccess& rcMbDataAccessBase,
                                  UInt&         ruiSymbol )
{
  ruiSymbol = ((rcMbDataAccess.getMbData().getMbCbp()>>4) > 1);
  DTRACE_T( "CBP_ChromaAC" );
  DTRACE_V( ruiSymbol );
  DTRACE_N;

  if( ruiSymbol )
  {
    rcMbDataAccessBase.getMbData().setMbCbp( ( rcMbDataAccessBase.getMbData().getMbCbp() & 0xF ) | 0x20 );
    rcMbDataAccess    .getMbData().setMbCbp( ( rcMbDataAccess    .getMbData().getMbCbp() & 0xF ) | 0x20 );
  }
  return Err::m_nOK;
}


ErrVal
UvlcReader::RQdecodeTCoeffsRef ( TCoeff*       piCoeff,
                                 TCoeff*       piCoeffBase,
                                 const UChar*  pucScan,
                                 UInt          uiScanIndex,
                                 UInt          uiCtx )
{
  return xRQdecodeTCoeffsRef( piCoeff, piCoeffBase, pucScan, uiScanIndex );
}


ErrVal
UvlcReader::RQupdateVlcTable  ( UInt    uiNumFrags )
{
  for( UInt uiFragIdx = 0; uiFragIdx < uiNumFrags; uiFragIdx ++ )
    m_apcFragSymGrps[uiFragIdx]->UpdateVlc();

  return Err::m_nOK;
}


ErrVal
UvlcReader::xInitFragments    ( const SliceHeader&  rcSliceHeader,
                                UChar**             ppucFragBuffers, 
                                UInt*               puiFragLengthInBits,
                                UInt                uiNumFragments )
{
  UInt uiFragIdx;

  // the first bitstream has already been initialized
  m_uiCurrentFragment     = 0;
  m_uiNumFragments        = uiNumFragments;

  m_apcFragmentReaders[0] = this;
  m_apcFragBitBuffers [0] = m_pcBitReadBuffer;
  m_apcFragSymGrps    [0] = m_pSymGrp;

  for( uiFragIdx = 1; uiFragIdx < uiNumFragments; uiFragIdx ++ )
  {
    // initialize the bit read buffers
    if( m_apcFragBitBuffers[uiFragIdx] == 0 )
    {
      BitReadBuffer::create( m_apcFragBitBuffers[uiFragIdx] );
      if( m_apcFragBitBuffers[uiFragIdx] == 0 )
        return Err::m_nERR;
    }
    m_apcFragBitBuffers[uiFragIdx]->initPacket
      ( (ULong *) ppucFragBuffers[uiFragIdx], puiFragLengthInBits[uiFragIdx] );

    // initialize the symbole group for refinement coding
    if( m_apcFragSymGrps[uiFragIdx] == 0 )
    {
      m_apcFragSymGrps[uiFragIdx] = new UcSymGrpReader(this);
      if( m_apcFragSymGrps[uiFragIdx] == 0 )
        return Err::m_nERR;
    }

    m_apcFragSymGrps[uiFragIdx]->Init();

    // child SymbolReader for fragments other than 0
    m_apcFragmentReaders[uiFragIdx] = new UvlcReader();
    m_apcFragmentReaders[uiFragIdx]->xSetParentFlag( false );

    m_apcFragmentReaders[uiFragIdx]->m_pcBitReadBuffer = m_apcFragBitBuffers[uiFragIdx];
    m_apcFragmentReaders[uiFragIdx]->m_pSymGrp         = m_apcFragSymGrps[uiFragIdx];
  }

  return Err::m_nOK;
}


ErrVal
UvlcReader::RQinitFragments   ( const SliceHeader&  rcSliceHeader,
                                UInt&               ruiNumFrags,
                                Bool                bCAF)
{
  UChar*  apucFragBuffers     [MAX_NUM_PD_FRAGMENTS];
  UInt    auiFragLengthInBits [MAX_NUM_PD_FRAGMENTS];

  // find the exact length of the first fragment, 
  // and find the start and the length of remaining fragments
  m_pcBitReadBuffer->assignFragments( apucFragBuffers, ruiNumFrags );

 //Ye Yan Frag: Bugfix
  if(!bCAF)
    ruiNumFrags = 1; 
 //
   m_pcBitReadBuffer->separateFragments
      ( apucFragBuffers, auiFragLengthInBits, ruiNumFrags, MAX_NUM_PD_FRAGMENTS - 1 );

  xInitFragments( rcSliceHeader, apucFragBuffers, auiFragLengthInBits, ruiNumFrags );

  return Err::m_nOK;
}


ErrVal
UvlcReader::RQreleaseFragments    ()
{
  UInt uiFragIdx;

  // restore the first bitstream to m_pcBitReadBuffer
  m_pcBitReadBuffer = m_apcFragBitBuffers[0];
  m_pSymGrp         = m_apcFragSymGrps[0];

  for( uiFragIdx = 1; uiFragIdx < m_uiNumFragments; uiFragIdx ++ )
  {
    if( m_apcFragBitBuffers[uiFragIdx] != 0 )
    {
      m_apcFragBitBuffers[uiFragIdx]->destroy();
      m_apcFragBitBuffers[uiFragIdx] = 0;
    }
    if( m_apcFragSymGrps[uiFragIdx] != 0 )
    {
      delete m_apcFragSymGrps[uiFragIdx];
      m_apcFragSymGrps[uiFragIdx] = 0;
    }
  }

  m_uiCurrentFragment = 0;
  m_uiNumFragments    = 0;

  return Err::m_nOK;
}


ErrVal
UvlcReader::RQdecodeSigCoeff        ( TCoeff*         piCoeff,
                                      TCoeff*         piCoeffBase,
                                      ResidualMode    eResidualMode,
                                      const UChar*    pucScan,
                                      Bool            bSkipEob,
                                      UInt            uiCycle,
                                      UInt            uiStartScanIdx,
                                      UInt            uiLastScanIdx,
                                      Bool&           rbEndOfBlock,
                                      TCoeff&         riCoeff,
                                      UInt&           ruiRun )
{
  UInt  ui, uiSymbol, uiOvershoot, uiActualEobIdx;
  UInt *pauiEobShift  = (eResidualMode == LUMA_SCAN) ? m_auiShiftLuma : m_auiShiftChroma;
  UInt *pauiVlcTable = m_auiBestCodeTab;

  // decode significant coefficient run/EOB
  RNOKS( xGetSigRunCode( uiSymbol, pauiVlcTable[uiCycle] ) );

  // Determine "overshoot" symbol, can this be simplified?
  for( uiOvershoot = 1, ui = uiCycle + 1; ui <= uiLastScanIdx; ui++ ) {
    if ( ! piCoeffBase[pucScan[ui]] )
      uiOvershoot ++;
  }

  // uiOvershoot in cludes run = 0, run = 1, and EOB
  if ( uiSymbol > uiOvershoot ) {
    rbEndOfBlock     = true;
    RNOKS( xRQdecodeSigMagGreater1( piCoeff, piCoeffBase, pucScan, 
      uiSymbol - uiOvershoot - 1, uiStartScanIdx, uiLastScanIdx + 1 ) );
  }
  else {
    uiActualEobIdx = min( uiOvershoot, pauiEobShift[uiCycle] );
    if( ! bSkipEob && uiSymbol == uiActualEobIdx ) {
      rbEndOfBlock = true;

    }
    else {
      ruiRun = ( bSkipEob || uiSymbol < uiActualEobIdx ) ? uiSymbol : uiSymbol - 1;

      RNOKS( xGetFlag( uiSymbol ) );
      riCoeff = uiSymbol ? -1 : 1;

      if( ruiRun == uiOvershoot - 1 ) {
        rbEndOfBlock  = true;
        piCoeff[pucScan[uiLastScanIdx]] = riCoeff;

        // decode token to check if any coefficients are larger than one
        RNOKS( xGetSigRunCode( uiSymbol, pauiVlcTable[uiCycle] ) );

        if( uiSymbol > 0 ) {
          RNOKS( xRQdecodeSigMagGreater1( piCoeff, piCoeffBase, pucScan, 
            uiSymbol - 1, uiStartScanIdx, uiLastScanIdx + 1 ) );
        }
      }
    }
  }

  return Err::m_nOK;
}

H264AVC_NAMESPACE_END
