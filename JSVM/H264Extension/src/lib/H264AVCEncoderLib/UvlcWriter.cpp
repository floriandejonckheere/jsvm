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
#include "UvlcWriter.h"
#include "H264AVCCommonLib/Tables.h"
#include "H264AVCCommonLib/TraceFile.h"

#define MAX_VALUE  0xdead
#define TOTRUN_NUM    15
#define RUNBEFORE_NUM  7

// h264 namepace begin
H264AVC_NAMESPACE_BEGIN

const UInt g_auiIncVlc[] = {0,3,6,12,24,48,32768};	// maximum vlc = 6

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


const UChar COEFF_COST[16] =
{
  3, 2,2,1, 1,1,0,0,0,0,0,0,0,0,0,0
};

const UChar COEFF_COST8x8[64] =
{
  3,3,3,3,2,2,2,2,2,2,2,2,1,1,1,1,
  1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

const UChar g_aucCbpIntra[48] =
{
   3, 29, 30, 17,
  31, 18, 37,  8,
  32, 38, 19,  9,
  20, 10, 11,  2,
  16, 33, 34, 21,
  35, 22, 39,  4,
  36, 40, 23,  5,
  24,  6,  7,  1,
  41, 42, 43, 25,
  44, 26, 46, 12,
  45, 47, 27, 13,
  28, 14, 15,  0
};


const UChar g_aucCbpInter[48] =
{
   0,  2,  3,  7,
   4,  8, 17, 13,
   5, 18,  9, 14,
  10, 15, 16, 11,
   1, 32, 33, 36,
  34, 37, 44, 40,
  35, 45, 38, 41,
  39, 42, 43, 19,
   6, 24, 25, 20,
  26, 21, 46, 28,
  27, 47, 22, 29,
  23, 30, 31, 12
};

const UInt g_auiISymCode[3][16] =
{ {  0,  1,  2,  3,  4,  5,  6,  7                                },
  {  0,  4,  5, 28,  6, 29, 30, 31                                },
  {  0,  4,  5, 28, 12, 29, 60,252, 13, 61,124,253,125,254,510,511}
};
const UInt g_auiISymLen[3][16] =
{ { 3, 3, 3, 3, 3, 3, 3, 3                        },
  { 1, 3, 3, 5, 3, 5, 5, 5                        },
  { 1, 3, 3, 5, 4, 5, 6, 8, 4, 6, 7, 8, 7, 8, 9, 9}
};


UvlcWriter::UvlcWriter( Bool bTraceEnable ) :
  m_pcBitWriteBufferIf( NULL ),
  m_uiBitCounter( 0 ),
  m_uiPosCounter( 0 ),
  m_uiCoeffCost( 0 ),
  m_bTraceEnable( bTraceEnable ),
  m_bRunLengthCoding( false ),
  m_uiRun( 0 )
{
  m_pBitGrpRef = new UcBitGrpWriter( this, 1, 15, 1024, 16 );
  m_pBitGrpSgn = new UcBitGrpWriter( this, 1, 15, 1024, 16 );
}


UvlcWriter::~UvlcWriter()
{
  delete m_pBitGrpRef;
  delete m_pBitGrpSgn;
}


ErrVal UvlcWriter::create( UvlcWriter*& rpcUvlcWriter, Bool bTraceEnable )
{
  rpcUvlcWriter = new UvlcWriter( bTraceEnable );

  ROT( NULL == rpcUvlcWriter );

  return Err::m_nOK;
}


ErrVal UvlcWriter::destroy()
{
  delete this;
  return Err::m_nOK;
}

__inline ErrVal UvlcWriter::xWriteCode( UInt uiCode, UInt uiLength )
{
  AOT_DBG(uiLength<1);

  ErrVal retVal = m_pcBitWriteBufferIf->write( uiCode, uiLength );

  ETRACE_TY( " u(v)" );
  ETRACE_BITS( uiCode, uiLength );
  ETRACE_POS;

  ETRACE_CODE( uiCode );
  ETRACE_COUNT (uiLength);
  return retVal;
}


__inline ErrVal UvlcWriter::xWriteFlag( UInt uiCode )
{
  ErrVal retVal = m_pcBitWriteBufferIf->write( uiCode, 1 );

  ETRACE_TY( " u(1)" );
  ETRACE_BITS( uiCode, 1 );
  ETRACE_POS;

  ETRACE_CODE( uiCode );
  ETRACE_COUNT (1);
  return retVal;
}

ErrVal UvlcWriter::init(  BitWriteBufferIf* pcBitWriteBufferIf )
{
  ROT( NULL == pcBitWriteBufferIf );

  m_pcBitWriteBufferIf= pcBitWriteBufferIf;
  m_bRunLengthCoding  = false;
  m_uiRun             = 0;

  return Err::m_nOK;
}

ErrVal UvlcWriter::uninit()
{
  m_pcBitWriteBufferIf = NULL;
  return Err::m_nOK;
}

UInt UvlcWriter::getNumberOfWrittenBits()
{
  return m_pcBitWriteBufferIf->getNumberOfWrittenBits();
}


ErrVal UvlcWriter::startSlice( const SliceHeader& rcSliceHeader )
{
  m_bRunLengthCoding  = ! rcSliceHeader.isIntra();
  m_uiRun             = 0;
  ::memset( m_auiSigMap, 0, 16*sizeof(UInt) );
  return Err::m_nOK;
}

ErrVal UvlcWriter::startFragment() //JVT-P031
{
    RNOK( m_pcBitWriteBufferIf->writeAlignOne() ); //FIX_FRAG_CAVLC
    return Err::m_nOK;
}
//FIX_FRAG_CAVLC
ErrVal UvlcWriter::getLastByte(UChar &uiLastByte, UInt &uiLastBitPos)
{
  RNOK(m_pcBitWriteBufferIf->getLastByte(uiLastByte, uiLastBitPos));
  return Err::m_nOK;
}
ErrVal UvlcWriter::setFirstBits(UChar ucByte,UInt uiLastBitPos)
{
  RNOK( m_pcBitWriteBufferIf->write(ucByte,uiLastBitPos));
  return Err::m_nOK;
}
//~FIX_FRAG_CAVLC
ErrVal UvlcWriter::xWriteUvlcCode( UInt uiVal)
{
  UInt uiLength = 1;
  UInt uiTemp = ++uiVal;

  AOF_DBG( uiTemp );

  while( 1 != uiTemp )
  {
    uiTemp >>= 1;
    uiLength += 2;
  }

  RNOK( m_pcBitWriteBufferIf->write( uiVal, uiLength ) );

  ETRACE_TY( "ue(v)" );
  ETRACE_BITS( uiVal, uiLength );
  ETRACE_POS;

  ETRACE_DO( uiVal-- );

  ETRACE_CODE( uiVal );
  ETRACE_COUNT (uiLength);

  return Err::m_nOK;
}


ErrVal UvlcWriter::xWriteSvlcCode( Int iVal)
{
  UInt uiVal = xConvertToUInt( iVal );
  UInt uiLength = 1;
  UInt uiTemp = ++uiVal;

  AOF_DBG( uiTemp );

  while( 1 != uiTemp )
  {
    uiTemp >>= 1;
    uiLength += 2;
  }

  RNOK( m_pcBitWriteBufferIf->write( uiVal, uiLength ) );

  ETRACE_TY( "ue(v)" );
  ETRACE_BITS( uiVal, uiLength );
  ETRACE_POS;

  ETRACE_CODE( iVal );
  ETRACE_COUNT (uiLength);

  return Err::m_nOK;
}

ErrVal UvlcWriter::writeUvlc( UInt uiCode, Char* pcTraceString )
{
  ETRACE_TH( pcTraceString );

  RNOK( xWriteUvlcCode( uiCode ) );

  ETRACE_N;
  return Err::m_nOK;
}


ErrVal UvlcWriter::writeSvlc( Int iCode, Char* pcTraceString )
{
  UInt uiCode;

  ETRACE_TH( pcTraceString );

  uiCode = xConvertToUInt( iCode );
  RNOK( xWriteUvlcCode( uiCode ) );

  ETRACE_TY( "se(v)" );
  ETRACE_CODE( iCode );

  ETRACE_N;
  return Err::m_nOK;
}

ErrVal UvlcWriter::writeFlag( Bool bFlag, Char* pcTraceString )
{
  ETRACE_TH( pcTraceString );
  ETRACE_TY( " u(1)" );

  RNOK( xWriteFlag( bFlag ? 1:0) );

  ETRACE_N;
  return Err::m_nOK;
}


ErrVal UvlcWriter::writeSCode( Int iCode, UInt uiLength, Char* pcTraceString )
{
  AOT_DBG(uiLength<1);
  ETRACE_TH( pcTraceString );
  ETRACE_TY( " i(v)" );

  UInt uiShift = 32 - uiLength;
  UInt uiCode = ((UInt)(iCode << uiShift)) >> uiShift;
  RNOK( m_pcBitWriteBufferIf->write( uiCode, uiLength ) );

  ETRACE_POS;
  ETRACE_CODE (iCode);
  ETRACE_BITS (uiCode, uiLength );
  ETRACE_COUNT(uiLength);
  ETRACE_N;
  return Err::m_nOK;
}

ErrVal UvlcWriter::writeCode( UInt uiCode, UInt uiLength, Char* pcTraceString )
{
  AOT_DBG(uiLength<1);
  ETRACE_TH( pcTraceString );
  ETRACE_TY( " u(v)" );

  RNOK( m_pcBitWriteBufferIf->write( uiCode, uiLength ) );

  ETRACE_POS;
  ETRACE_CODE (uiCode);
  ETRACE_BITS (uiCode, uiLength );
  ETRACE_COUNT(uiLength);
  ETRACE_N;
  return Err::m_nOK;
}



ErrVal UvlcWriter::refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx )
{
  UInt uiRefFrame = rcMbDataAccess.getMbMotionData( eLstIdx ).getRefIdx();
  RNOK( xWriteRefFrame( 2 == rcMbDataAccess.getNumActiveRef( eLstIdx ), --uiRefFrame ) )
  return Err::m_nOK;
}

ErrVal UvlcWriter::refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  )
{
  UInt uiRefFrame = rcMbDataAccess.getMbMotionData( eLstIdx ).getRefIdx( eParIdx );
  RNOK( xWriteRefFrame( 2 == rcMbDataAccess.getNumActiveRef( eLstIdx ), --uiRefFrame ) )
  return Err::m_nOK;
}

ErrVal UvlcWriter::refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  )
{
  UInt uiRefFrame = rcMbDataAccess.getMbMotionData( eLstIdx ).getRefIdx( eParIdx );
  RNOK( xWriteRefFrame( 2 == rcMbDataAccess.getNumActiveRef( eLstIdx ), --uiRefFrame ) )
  return Err::m_nOK;
}

ErrVal UvlcWriter::refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8 eParIdx  )
{
  UInt uiRefFrame = rcMbDataAccess.getMbMotionData( eLstIdx ).getRefIdx( eParIdx );
  RNOK( xWriteRefFrame( 2 == rcMbDataAccess.getNumActiveRef( eLstIdx ), --uiRefFrame ) )
  return Err::m_nOK;
}


ErrVal  UvlcWriter::motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx )
{
  return xWriteMotionPredFlag( rcMbDataAccess.getMbMotionData( eLstIdx ).getMotPredFlag() );
}
ErrVal  UvlcWriter::motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx   )
{
  return xWriteMotionPredFlag( rcMbDataAccess.getMbMotionData( eLstIdx ).getMotPredFlag( eParIdx ) );
}
ErrVal  UvlcWriter::motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx   )
{
  return xWriteMotionPredFlag( rcMbDataAccess.getMbMotionData( eLstIdx ).getMotPredFlag( eParIdx ) );
}
ErrVal  UvlcWriter::motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8 eParIdx   )
{
  return xWriteMotionPredFlag( rcMbDataAccess.getMbMotionData( eLstIdx ).getMotPredFlag( eParIdx ) );
}



ErrVal UvlcWriter::blockModes( MbDataAccess& rcMbDataAccess )
{
  for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
  {
    ETRACE_T( "BlockMode" );

    UInt uiBlockMode = rcMbDataAccess.getConvertBlkMode( c8x8Idx.b8x8Index() );

    AOT_DBG( uiBlockMode > 12);

    RNOK( xWriteUvlcCode( uiBlockMode ) );

    ETRACE_N;
  }
  return Err::m_nOK;
}

ErrVal UvlcWriter::blFlag( MbDataAccess& rcMbDataAccess )
{
  ETRACE_T( "BLFlag" );

  UInt uiBit = ( rcMbDataAccess.getMbData().getMbMode() == INTRA_BL ? 1 : 0 );

  RNOK( xWriteFlag( uiBit ) );

  ETRACE_N;

  return Err::m_nOK;
}


ErrVal UvlcWriter::skipFlag( MbDataAccess& rcMbDataAccess, Bool bNotAllowed )
{
  rcMbDataAccess.getMbTCoeffs().setAllCoeffCount( 0 );

  ROFRS( m_bRunLengthCoding, Err::m_nOK );

  if( ! bNotAllowed && rcMbDataAccess.isSkippedMb() )
  {
    m_uiRun++;
  }
  else
  {
    ETRACE_T( "Run" );
    RNOK( xWriteUvlcCode( m_uiRun ) );
    ETRACE_N;

    m_uiRun = 0;
  }

  return Err::m_nOK;
}


ErrVal UvlcWriter::BLSkipFlag( MbDataAccess& rcMbDataAccess )
{
  UInt  uiCode = ( rcMbDataAccess.getMbData().getBLSkipFlag() ? 1 : 0 );

  ETRACE_T( "BLSkipFlag" );
  RNOK( xWriteFlag( uiCode ) );
  ETRACE_N;

  rcMbDataAccess.getMbTCoeffs().setAllCoeffCount( 0 );

  return Err::m_nOK;
}
ErrVal UvlcWriter::BLQRefFlag( MbDataAccess& rcMbDataAccess )
{
  UInt  uiCode = ( rcMbDataAccess.getMbData().getBLQRefFlag() ? 1 : 0 );

  ETRACE_T( "BLQRefFlag" );
  RNOK( xWriteFlag( uiCode ) );
  ETRACE_N;

  return Err::m_nOK;
}


ErrVal UvlcWriter::mbMode( MbDataAccess& rcMbDataAccess )
{
  UInt uiMbMode = rcMbDataAccess.getConvertMbType( );

  if( m_bRunLengthCoding )
  {
    uiMbMode--;
  }
  rcMbDataAccess.getMbTCoeffs().setAllCoeffCount( 0 );
	 
	ETRACE_T( "MbMode" );
  RNOK( xWriteUvlcCode( uiMbMode ) );
  ETRACE_N;

  return Err::m_nOK;
}


ErrVal UvlcWriter::resPredFlag( MbDataAccess& rcMbDataAccess )
{
  UInt uiCode = ( rcMbDataAccess.getMbData().getResidualPredFlag( PART_16x16 ) ? 1 : 0 );

  ETRACE_T( "ResidualPredFlag" );
  RNOK( xWriteFlag( uiCode ) );
  ETRACE_N;

  return Err::m_nOK;
}

ErrVal UvlcWriter::mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx )
{
  Mv cMv = rcMbDataAccess.getMbMvdData( eLstIdx ).getMv();
  RNOK( xWriteMvd( cMv ) );
  return Err::m_nOK;
}

ErrVal UvlcWriter::mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  )
{
  Mv cMv = rcMbDataAccess.getMbMvdData( eLstIdx ).getMv( eParIdx );
  RNOK( xWriteMvd( cMv ) );
  return Err::m_nOK;
}

ErrVal UvlcWriter::mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  )
{
  Mv cMv = rcMbDataAccess.getMbMvdData( eLstIdx ).getMv( eParIdx );
  RNOK( xWriteMvd( cMv ) );
  return Err::m_nOK;
}

ErrVal UvlcWriter::mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  )
{
  Mv cMv = rcMbDataAccess.getMbMvdData( eLstIdx ).getMv( eParIdx );
  RNOK( xWriteMvd( cMv ) );
  return Err::m_nOK;
}

ErrVal UvlcWriter::mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx8x4 eSParIdx )
{
  Mv cMv = rcMbDataAccess.getMbMvdData( eLstIdx ).getMv( eParIdx, eSParIdx );
  RNOK( xWriteMvd( cMv ) );
  return Err::m_nOK;
}

ErrVal UvlcWriter::mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx4x8 eSParIdx )
{
  Mv cMv = rcMbDataAccess.getMbMvdData( eLstIdx ).getMv( eParIdx, eSParIdx );
  RNOK( xWriteMvd( cMv ) );
  return Err::m_nOK;
}

ErrVal UvlcWriter::mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx4x4 eSParIdx )
{
  Mv cMv = rcMbDataAccess.getMbMvdData( eLstIdx ).getMv( eParIdx, eSParIdx );
  RNOK( xWriteMvd( cMv ) );
  return Err::m_nOK;
}


ErrVal UvlcWriter::xWriteMvd( Mv cMv )
{
  ETRACE_T( "Mvd: x" );

  UInt  uiTemp;
  Short sHor   = cMv.getHor();
  Short sVer   = cMv.getVer();

  uiTemp = xConvertToUInt( sHor );
  RNOK( xWriteUvlcCode( uiTemp ) );

  ETRACE_CODE( sHor );
  ETRACE_TY("se(v)");
  ETRACE_N;

  ETRACE_T( "Mvd: y" );

  uiTemp = xConvertToUInt( sVer );
  RNOK( xWriteUvlcCode( uiTemp ) );

  ETRACE_CODE( sVer );
  ETRACE_TY("se(v)");
  ETRACE_N;
  return Err::m_nOK;
}

ErrVal  UvlcWriter::mvdQPel ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx                      )
{
  Mv cMv = rcMbDataAccess.getMbMvdData( eLstIdx ).getMv();
  RNOK( xWriteMvdQPel( rcMbDataAccess, cMv, B4x4Idx(0), eLstIdx ) );
  return Err::m_nOK;
}
ErrVal  UvlcWriter::mvdQPel ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  )
{
  Mv cMv = rcMbDataAccess.getMbMvdData( eLstIdx ).getMv( eParIdx );
  RNOK( xWriteMvdQPel( rcMbDataAccess, cMv, B4x4Idx(eParIdx), eLstIdx ) );
  return Err::m_nOK;
}
ErrVal  UvlcWriter::mvdQPel ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  )
{
  Mv cMv = rcMbDataAccess.getMbMvdData( eLstIdx ).getMv( eParIdx );
  RNOK( xWriteMvdQPel( rcMbDataAccess, cMv, B4x4Idx(eParIdx), eLstIdx ) );
  return Err::m_nOK;
}
ErrVal  UvlcWriter::mvdQPel ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  )
{
  Mv cMv = rcMbDataAccess.getMbMvdData( eLstIdx ).getMv( eParIdx );
  RNOK( xWriteMvdQPel( rcMbDataAccess, cMv, B4x4Idx(eParIdx), eLstIdx ) );
  return Err::m_nOK;
}

ErrVal UvlcWriter::xWriteMvdQPel( MbDataAccess& rcMbDataAccess, Mv cMv, LumaIdx cIdx, ListIdx eLstIdx )
{
  Short sHor = cMv.getHor();
  Short sVer = cMv.getVer();

  RNOK( xWriteMvdComponentQPel( sHor, 0, 0 ) );
  RNOK( xWriteMvdComponentQPel( sVer, 0, 0 ) );

  return Err::m_nOK;
}

ErrVal UvlcWriter::xWriteMvdComponentQPel( Short sMvdComp, UInt uiAbsSum, UInt uiCtx )
{
  UInt  uiSymbol  = ( sMvdComp == 0 ? 0 : 1 );
  RNOK( xWriteFlag( uiSymbol ) );
  ROTRS( uiSymbol == 0 , Err::m_nOK );

  uiSymbol = ( sMvdComp < 0 ? 0 : 1 );
  RNOK( xWriteFlag( uiSymbol ) );

  return Err::m_nOK;
}


ErrVal UvlcWriter::intraPredModeChroma( MbDataAccess& rcMbDataAccess )
{
  ETRACE_T( "IntraPredModeChroma" );

  AOT_DBG( 4 < rcMbDataAccess.getMbData().getChromaPredMode() );
  RNOK( xWriteUvlcCode( rcMbDataAccess.getMbData().getChromaPredMode() ) );

  ETRACE_N;

  return Err::m_nOK;
}

ErrVal UvlcWriter::intraPredModeLuma( MbDataAccess& rcMbDataAccess, LumaIdx cIdx )
{
  ETRACE_T( "IntraPredModeLuma" );
  ETRACE_POS;

  Int iIntraPredModeLuma = rcMbDataAccess.encodeIntraPredMode(cIdx);
  ROT( iIntraPredModeLuma > 7);

  UInt uiBits = (iIntraPredModeLuma < 0) ? 1 : 0;

  RNOK( m_pcBitWriteBufferIf->write( uiBits, 1 ) );
  ETRACE_BITS( uiBits,1 );
  ETRACE_DO( m_uiBitCounter = 1 );

  if( ! uiBits )
  {
    RNOK( m_pcBitWriteBufferIf->write( iIntraPredModeLuma, 3 ) );
    ETRACE_BITS( iIntraPredModeLuma, 3 );
    ETRACE_DO( m_uiBitCounter = 4 );
  }

  ETRACE_COUNT(m_uiBitCounter);
  ETRACE_CODE(iIntraPredModeLuma);
  ETRACE_N;

  return Err::m_nOK;
}


ErrVal UvlcWriter::cbp( MbDataAccess& rcMbDataAccess )
{
  UInt uiCbp = rcMbDataAccess.getMbData().getMbCbp();
  ETRACE_T( "Cbp: " );
  ETRACE_X ( uiCbp );

  AOT_DBG( 48 < uiCbp );

  UInt uiTemp = ( rcMbDataAccess.getMbData().isIntra() ) ? g_aucCbpIntra[uiCbp]: g_aucCbpInter[uiCbp];

  RNOK( xWriteUvlcCode( uiTemp ) );

  ETRACE_N;

  return Err::m_nOK;
}



const UChar g_aucTcoeffCDc[3][2]=
{
  {0,0},
  {2,6},
  {4,1}
};
const UChar g_aucRunCDc[4]=
{
  2,1,0,0
};



const UChar g_aucRunSScan[16]=
{
  4,2,2,1,1,1,1,1,1,1,0,0,0,0,0,0
};
const UChar g_aucTcoeffSScan[4][10]=
{
  { 1, 3, 5, 9,11,13,21,23,25,27},
  { 7,17,19, 0, 0, 0, 0, 0, 0, 0},
  {15, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {29, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};




const UChar g_aucRunDScan[8]=
{
  9,3,1,1,1,0,0,0
};
const UChar g_aucTcoeffDScan[9][5] =
{
  { 1, 3, 7,15,17},
  { 5,19, 0, 0, 0},
  { 9,21, 0, 0, 0},
  {11, 0, 0, 0, 0},
  {13, 0, 0, 0, 0},
  {23, 0, 0, 0, 0},
  {25, 0, 0, 0, 0},
  {27, 0, 0, 0, 0},
  {29, 0, 0, 0, 0},
};



ErrVal UvlcWriter::residualBlock( MbDataAccess& rcMbDataAccess,
                                  LumaIdx       cIdx,
                                  ResidualMode  eResidualMode )
{
  const UChar*  pucScan;
  const TCoeff* piCoeff = rcMbDataAccess.getMbTCoeffs().get( cIdx );

  Int   iLevel;
  Int   iRun      = 0;
  UInt  uiPos     = 0;
  UInt  uiMaxPos  = 16;

  switch( eResidualMode )
  {
  case LUMA_I16_DC:
    {
      pucScan  = g_aucLumaFrameDCScan;
      uiMaxPos = 16;
      break;
    }
  case LUMA_I16_AC:
    {
      pucScan  = g_aucFrameScan;
      uiPos=1;
      break;
    }
  case LUMA_SCAN:
    {
      pucScan  = g_aucFrameScan;
      break;
    }
  default:
    return Err::m_nERR;
  }

  Int   aiLevelRun[32];
  UInt  uiTrailingOnes = 0;
  UInt  uiTotalRun     = 0;
  UInt  uiCoeffCnt     = 0;

  while( uiPos < uiMaxPos )
  {
    if( ( iLevel = piCoeff[ pucScan [ uiPos++ ] ]) )
    {
      if( abs(iLevel) == 1 )
      {
        m_uiCoeffCost += COEFF_COST[iRun];
        uiTrailingOnes++;
      }
      else
      {
        m_uiCoeffCost += MAX_VALUE;                // set high cost, shall not be discarded
        uiTrailingOnes = 0;
      }

      aiLevelRun[uiCoeffCnt]      = iLevel;
      aiLevelRun[uiCoeffCnt+0x10] = iRun;
      uiTotalRun += iRun;
      uiCoeffCnt++;
      iRun = 0;
    }
    else
    {
      iRun++;
    }
  }

  if( uiTrailingOnes > 3 )
  {
    uiTrailingOnes = 3;
  }


  switch( eResidualMode )
  {
  case LUMA_I16_DC:
    {
      ETRACE_T( "Luma:" );
      ETRACE_V( cIdx );
      ETRACE_N;
      xPredictNonZeroCnt( rcMbDataAccess, cIdx, uiCoeffCnt, uiTrailingOnes );
      xWriteRunLevel( aiLevelRun, uiCoeffCnt, uiTrailingOnes, 16, uiTotalRun );
      break;
    }
  case LUMA_I16_AC:
    {
      ETRACE_T( "Luma:" );
      ETRACE_V( cIdx );
      ETRACE_N;
      xPredictNonZeroCnt( rcMbDataAccess, cIdx, uiCoeffCnt, uiTrailingOnes );
      xWriteRunLevel( aiLevelRun, uiCoeffCnt, uiTrailingOnes, 15, uiTotalRun );
      break;
    }
  case LUMA_SCAN:
    {
      ETRACE_T( "Luma:" );
      ETRACE_V( cIdx );
      ETRACE_N;
      xPredictNonZeroCnt( rcMbDataAccess, cIdx, uiCoeffCnt, uiTrailingOnes );
      xWriteRunLevel( aiLevelRun, uiCoeffCnt, uiTrailingOnes, 16, uiTotalRun );
      break;
    }
  default:
    {
      AOT(1);
    }
  }

  return Err::m_nOK;
}



ErrVal UvlcWriter::residualBlock( MbDataAccess& rcMbDataAccess,
                                  ChromaIdx     cIdx,
                                  ResidualMode  eResidualMode )
{
  const TCoeff* piCoeff = rcMbDataAccess.getMbTCoeffs().get( cIdx );
  const UChar*  pucScan;
  Int           iRun = 0, iLevel;
  UInt          uiPos, uiMaxPos;

  switch( eResidualMode )
  {
  case CHROMA_DC:
    {
      pucScan = g_aucIndexChromaDCScan;
      uiPos=0;  uiMaxPos= 4;
      break;
    }
  case CHROMA_AC:
    {
      pucScan = g_aucFrameScan;
      uiPos=1;  uiMaxPos=16;
      break;
    }
  default:
    return Err::m_nERR;
  }

  Int aiLevelRun[32];

  UInt uiTrailingOnes = 0;
  UInt uiTotalRun     = 0;
  UInt uiCoeffCnt     = 0;

  while( uiPos < uiMaxPos )
  {
    if( ( iLevel = piCoeff[ pucScan [ uiPos++ ] ]) )
    {
      if( abs(iLevel) == 1 )
      {
        m_uiCoeffCost += COEFF_COST[iRun];
        uiTrailingOnes++;
      }
      else
      {
        m_uiCoeffCost += MAX_VALUE;                // set high cost, shall not be discarded
        uiTrailingOnes = 0;
      }

      aiLevelRun[uiCoeffCnt]      = iLevel;
      aiLevelRun[uiCoeffCnt+0x10] = iRun;
      uiTotalRun += iRun;
      uiCoeffCnt++;
      iRun = 0;
    }
    else
    {
      iRun++;
    }
  }

  if( uiTrailingOnes > 3 )
  {
    uiTrailingOnes = 3;
  }


  switch( eResidualMode )
  {
  case CHROMA_AC:
    {
      ETRACE_T( "CHROMA_AC:" );
      ETRACE_V( cIdx );
      ETRACE_N;
      xPredictNonZeroCnt( rcMbDataAccess, cIdx, uiCoeffCnt, uiTrailingOnes );
      xWriteRunLevel( aiLevelRun, uiCoeffCnt, uiTrailingOnes, 15, uiTotalRun );
      break;
    }
  case CHROMA_DC:
    {
      ETRACE_T( "CHROMA_DC:" );
      ETRACE_V( cIdx );
      ETRACE_N;
      xWriteTrailingOnes4( uiCoeffCnt, uiTrailingOnes );
      xWriteRunLevel( aiLevelRun, uiCoeffCnt, uiTrailingOnes, 4, uiTotalRun );
      break;
    }
  default:
    {
      AOT(1);
    }
  }

  return Err::m_nOK;
}


ErrVal UvlcWriter::deltaQp( MbDataAccess& rcMbDataAccess )
{
  ETRACE_T ("DQp");

  RNOK( xWriteSvlcCode( rcMbDataAccess.getDeltaQp() ) );

  ETRACE_TY ("se(v)");
  ETRACE_N;
  return Err::m_nOK;
}


ErrVal UvlcWriter::finishSlice()
{
  if( m_bRunLengthCoding && m_uiRun )
  {
    ETRACE_T( "Run" );
    RNOK( xWriteUvlcCode( m_uiRun ) );
    ETRACE_N;
  }

  return Err::m_nOK;
}





ErrVal UvlcWriter::xPredictNonZeroCnt( MbDataAccess& rcMbDataAccess, ChromaIdx cIdx, UInt uiCoeffCount, UInt uiTrailingOnes )
{
  UInt uiCoeffCountCtx = rcMbDataAccess.getCtxCoeffCount( cIdx );

  xWriteTrailingOnes16( uiCoeffCountCtx, uiCoeffCount, uiTrailingOnes );

  rcMbDataAccess.getMbTCoeffs().setCoeffCount( cIdx, uiCoeffCount );

  return Err::m_nOK;
}

ErrVal UvlcWriter::xPredictNonZeroCnt( MbDataAccess& rcMbDataAccess, LumaIdx cIdx, UInt uiCoeffCount, UInt uiTrailingOnes )
{
  UInt uiCoeffCountCtx = rcMbDataAccess.getCtxCoeffCount( cIdx );

  xWriteTrailingOnes16( uiCoeffCountCtx, uiCoeffCount, uiTrailingOnes );

  rcMbDataAccess.getMbTCoeffs().setCoeffCount( cIdx, uiCoeffCount );

  return Err::m_nOK;
}

ErrVal UvlcWriter::xWriteRunLevel( Int* aiLevelRun, UInt uiCoeffCnt, UInt uiTrailingOnes, UInt uiMaxCoeffs, UInt uiTotalRun )
{

  ROTRS( 0 == uiCoeffCnt, Err::m_nOK );

  if( uiTrailingOnes )
  {
    UInt uiBits = 0;
    Int n = uiTrailingOnes-1;
    for( UInt k = uiCoeffCnt; k > uiCoeffCnt-uiTrailingOnes; k--, n--)
    {
      if( aiLevelRun[k-1] < 0)
      {
        uiBits |= 1<<n;
      }
    }

    RNOK( m_pcBitWriteBufferIf->write( uiBits, uiTrailingOnes ))
    ETRACE_POS;
    ETRACE_T( "  TrailingOnesSigns: " );
    ETRACE_V( uiBits );
    ETRACE_N;
    ETRACE_COUNT(uiTrailingOnes);
  }


  Int iHighLevel = ( uiCoeffCnt > 3 && uiTrailingOnes == 3) ? 0 : 1;
  Int iVlcTable  = ( uiCoeffCnt > 10 && uiTrailingOnes < 3) ? 1 : 0;

  for( Int k = uiCoeffCnt - 1 - uiTrailingOnes; k >= 0; k--)
  {
    Int iLevel;
    iLevel = aiLevelRun[k];

    UInt uiAbsLevel = (UInt)abs(iLevel);

    if( iHighLevel )
    {
      iLevel -= ( iLevel > 0 ) ? 1 : -1;
	    iHighLevel = 0;
    }

    if( iVlcTable == 0 )
    {
	    xWriteLevelVLC0( iLevel );
    }
    else
    {
	    xWriteLevelVLCN( iLevel, iVlcTable );
    }

    // update VLC table
    if( uiAbsLevel > g_auiIncVlc[ iVlcTable ] )
    {
      iVlcTable++;
    }

    if( k == Int(uiCoeffCnt - 1 - uiTrailingOnes) && uiAbsLevel > 3)
    {
      iVlcTable = 2;
    }

  }

  ROFRS( uiCoeffCnt < uiMaxCoeffs, Err::m_nOK );


  iVlcTable = uiCoeffCnt-1;
  if( uiMaxCoeffs <= 4 )
  {
    xWriteTotalRun4( iVlcTable, uiTotalRun );
  }
  else
  {
    xWriteTotalRun16( iVlcTable, uiTotalRun );
  }

  // decode run before each coefficient
  uiCoeffCnt--;
  if( uiTotalRun > 0 && uiCoeffCnt > 0)
  {
    do
    {
      iVlcTable = (( uiTotalRun > RUNBEFORE_NUM) ? RUNBEFORE_NUM : uiTotalRun) - 1;
      UInt uiRun = aiLevelRun[uiCoeffCnt+0x10];

      xWriteRun( iVlcTable, uiRun );

      uiTotalRun -= uiRun;
      uiCoeffCnt--;
    } while( uiTotalRun != 0 && uiCoeffCnt != 0);
  }

  return Err::m_nOK;
}


ErrVal UvlcWriter::xWriteTrailingOnes16( UInt uiLastCoeffCount, UInt uiCoeffCount, UInt uiTrailingOnes )
{
  UInt uiVal;
  UInt uiSize;

  ETRACE_POS;
  if( 3 == uiLastCoeffCount )
  {
    UInt uiBits = 3;
    if( uiCoeffCount )
    {
      uiBits = (uiCoeffCount-1)<<2 | uiTrailingOnes;
    }
    RNOK( m_pcBitWriteBufferIf->write( uiBits, 6) );
    ETRACE_DO( m_uiBitCounter = 6 );

    uiVal = uiBits;
    uiSize = 6;
  }
  else
  {
    RNOK( m_pcBitWriteBufferIf->write( g_aucCodeTableTO16[uiLastCoeffCount][uiTrailingOnes][uiCoeffCount],
                                  g_aucLenTableTO16[uiLastCoeffCount][uiTrailingOnes][uiCoeffCount] ) );
    ETRACE_DO( m_uiBitCounter = g_aucLenTableTO16[uiLastCoeffCount][uiTrailingOnes][uiCoeffCount] );

    uiVal = g_aucCodeTableTO16[uiLastCoeffCount][uiTrailingOnes][uiCoeffCount];
    uiSize = g_aucLenTableTO16[uiLastCoeffCount][uiTrailingOnes][uiCoeffCount];
  }

  ETRACE_T( "  TrailingOnes16: Vlc: " );
  ETRACE_V( uiLastCoeffCount );
  ETRACE_T( " CoeffCnt: " );
  ETRACE_V( uiCoeffCount );
  ETRACE_T( " TraiOnes: " );
  ETRACE_V( uiTrailingOnes );
  ETRACE_N;
  ETRACE_COUNT(m_uiBitCounter);

  return Err::m_nOK;
}



ErrVal UvlcWriter::xWriteTrailingOnes4( UInt uiCoeffCount, UInt uiTrailingOnes )
{
  RNOK( m_pcBitWriteBufferIf->write( g_aucCodeTableTO4[uiTrailingOnes][uiCoeffCount],
                                g_aucLenTableTO4[uiTrailingOnes][uiCoeffCount] ) );

  ETRACE_POS;
  ETRACE_T( "  TrailingOnes4: CoeffCnt: " );
  ETRACE_V( uiCoeffCount );
  ETRACE_T( " TraiOnes: " );
  ETRACE_V( uiTrailingOnes );
  ETRACE_N;
  ETRACE_COUNT(g_aucLenTableTO4[uiTrailingOnes][uiCoeffCount]);

  return Err::m_nOK;
}


ErrVal UvlcWriter::xWriteTotalRun4( UInt uiVlcPos, UInt uiTotalRun )
{
  RNOK( m_pcBitWriteBufferIf->write( g_aucCodeTableTZ4[uiVlcPos][uiTotalRun],
                                g_aucLenTableTZ4[uiVlcPos][uiTotalRun] ) );

  ETRACE_POS;
  ETRACE_T( "  TotalZeros4 vlc: " );
  ETRACE_V( uiVlcPos );
  ETRACE_T( " TotalRun: " );
  ETRACE_V( uiTotalRun );
  ETRACE_N;
  ETRACE_COUNT(g_aucLenTableTZ4[uiVlcPos][uiTotalRun]);

  return Err::m_nOK;
}


ErrVal UvlcWriter::xWriteTotalRun16( UInt uiVlcPos, UInt uiTotalRun )
{
  RNOK( m_pcBitWriteBufferIf->write( g_aucCodeTableTZ16[uiVlcPos][uiTotalRun],
                                g_aucLenTableTZ16[uiVlcPos][uiTotalRun] ) );

  ETRACE_POS;
  ETRACE_T( "  TotalRun16 vlc: " );
  ETRACE_V( uiVlcPos );
  ETRACE_T( " TotalRun: " );
  ETRACE_V( uiTotalRun );
  ETRACE_N;
  ETRACE_COUNT(g_aucLenTableTZ16[uiVlcPos][uiTotalRun]);

  return Err::m_nOK;
}


ErrVal UvlcWriter::xWriteRun( UInt uiVlcPos, UInt uiRun  )
{
  RNOK( m_pcBitWriteBufferIf->write( g_aucCodeTable3[uiVlcPos][uiRun],
                                g_aucLenTable3[uiVlcPos][uiRun] ) );

  ETRACE_POS;
  ETRACE_T( "  Run" );
  ETRACE_CODE( uiRun );
  ETRACE_COUNT (g_aucLenTable3[uiVlcPos][uiRun]);
  ETRACE_N;

  return Err::m_nOK;
}




ErrVal UvlcWriter::xWriteLevelVLC0( Int iLevel )
{

  UInt uiLength;
  UInt uiLevel = abs( iLevel );
  UInt uiSign = ((UInt)iLevel)>>31;

  UInt uiBits;

  if( 8 > uiLevel )
  {
    uiBits   = 1;
    uiLength = 2 * uiLevel - 1 + uiSign;
  }
  else if( 16 > uiLevel )
  {
    uiBits   = 2*uiLevel + uiSign;
    uiLength = 15 + 4;
  }
  else
  {
    uiBits   = 0x1000-32 + (uiLevel<<1) + uiSign;
    uiLength = 16 + 12;
  }


  RNOK( m_pcBitWriteBufferIf->write( uiBits, uiLength ) );

  ETRACE_POS;
  ETRACE_T( "  VLC0 lev " );
  ETRACE_CODE( iLevel );
  ETRACE_N;
  ETRACE_COUNT( uiLength );

  return Err::m_nOK;

}

ErrVal UvlcWriter::xWriteLevelVLCN( Int iLevel, UInt uiVlcLength )
{
  UInt uiLength;
  UInt uiLevel = abs( iLevel );
  UInt uiSign = ((UInt)iLevel)>>31;
  UInt uiBits;

  UInt uiShift = uiVlcLength-1;
  UInt uiEscapeCode = (0xf<<uiShift)+1;

  if( uiLevel < uiEscapeCode )
  {
    uiLevel--;
	  uiLength = (uiLevel>>uiShift) + uiVlcLength + 1;
    uiLevel &= ~((0xffffffff)<<uiShift);
	  uiBits   = (2<<uiShift) | 2*uiLevel | uiSign;
  }
  else
  {
	  uiLength = 28;
	  uiBits   = 0x1000 + 2*(uiLevel-uiEscapeCode) + uiSign;
  }



  RNOK( m_pcBitWriteBufferIf->write( uiBits, uiLength ) );

  ETRACE_POS;
  ETRACE_T( "  VLCN lev: " );
  ETRACE_CODE( iLevel );
  ETRACE_N;
  ETRACE_COUNT( uiLength );

  return Err::m_nOK;
}



ErrVal UvlcWriter::samplesPCM( MbDataAccess& rcMbDataAccess )
{
  ETRACE_POS;
  ETRACE_T( "  PCM SAMPLES: " );

  RNOK( m_pcBitWriteBufferIf->writeAlignZero() );

  AOF_DBG( rcMbDataAccess.getMbData().isPCM() );

  rcMbDataAccess.getMbTCoeffs().setAllCoeffCount( 16 );
  Pel* pSrc = rcMbDataAccess.getMbTCoeffs().getPelBuffer();

  const UInt uiFactor = 8*8;
  const UInt uiSize   = uiFactor*2*3;
  RNOK( m_pcBitWriteBufferIf->samples( pSrc, uiSize ) );

  ETRACE_N;
  ETRACE_COUNT( uiFactor*6 );

  return Err::m_nOK;
}



ErrVal UvlcWriter::xWriteRefFrame( Bool bWriteBit, UInt uiRefFrame )
{
  ETRACE_T( "RefFrame" );

  if( bWriteBit )
  {
    RNOK( xWriteFlag( 1-uiRefFrame ) );
  }
  else
  {
    RNOK( xWriteUvlcCode( uiRefFrame ) );
  }

  ETRACE_V( uiRefFrame+1 );
  ETRACE_N;
  return Err::m_nOK;
}


ErrVal UvlcWriter::xWriteMotionPredFlag( Bool bFlag )
{
  ETRACE_T( "MotionPredFlag" );

  UInt  uiCode = ( bFlag ? 1 : 0 );
  RNOK( xWriteFlag( uiCode) );

  ETRACE_V( uiCode );
  ETRACE_N;
  return Err::m_nOK;
}


ErrVal UvlcWriter::transformSize8x8Flag( MbDataAccess& rcMbDataAccess ) 
{
  ETRACE_T( "transformSize8x8Flag:" );

  UInt  uiCode = rcMbDataAccess.getMbData().isTransformSize8x8() ? 1 : 0;
  RNOK( xWriteFlag( uiCode) );

  ETRACE_V( uiCode );
  ETRACE_N;
  return Err::m_nOK;
}





ErrVal UvlcWriter::residualBlock8x8( MbDataAccess&  rcMbDataAccess,
                                     B8x8Idx        c8x8Idx,
                                     ResidualMode   eResidualMode )
{
  ROF( eResidualMode == LUMA_SCAN );

  const UChar*  pucScan = g_aucFrameScan64;
  const TCoeff* piCoeff = rcMbDataAccess.getMbTCoeffs().get8x8( c8x8Idx );

  UInt  uiBlk;
  Int   iLevel;
  Int   iOverallRun = 0;
  UInt  uiPos       = 0;
  UInt  uiMaxPos    = 64;

  Int   aaiLevelRun     [4][32];
  Int   aiRun           [4]     = { 0, 0, 0, 0 };
  UInt  auiTrailingOnes [4]     = { 0, 0, 0, 0 };
  UInt  auiTotalRun     [4]     = { 0, 0, 0, 0 };
  UInt  auiCoeffCnt     [4]     = { 0, 0, 0, 0 };

  while( uiPos < uiMaxPos )
  {
    uiBlk = ( uiPos % 4 );

    if( ( iLevel = piCoeff[ pucScan[ uiPos++ ] ] ) )
    {
      if( abs(iLevel) == 1 )
      {
        m_uiCoeffCost         += COEFF_COST8x8[ iOverallRun ];
        auiTrailingOnes[uiBlk]++;
      }
      else
      {
        m_uiCoeffCost         += MAX_VALUE;
        auiTrailingOnes[uiBlk] = 0;
      }

      aaiLevelRun[uiBlk][auiCoeffCnt[uiBlk]]      = iLevel;
      aaiLevelRun[uiBlk][auiCoeffCnt[uiBlk]+0x10] = aiRun[uiBlk];
      auiTotalRun[uiBlk]  += aiRun[uiBlk];
      auiCoeffCnt[uiBlk]  ++;
      aiRun      [uiBlk]  = 0;
      iOverallRun         = 0;
    }
    else
    {
      aiRun[uiBlk]++;
      iOverallRun ++;
    }
  }


  //===== loop over 4x4 blocks =====
  for( uiBlk = 0; uiBlk < 4; uiBlk++ )
  {
    if( auiTrailingOnes[uiBlk] > 3 )
    {
      auiTrailingOnes[uiBlk] = 3;
    }
    B4x4Idx cIdx( c8x8Idx.b4x4() + 4*(uiBlk/2) + (uiBlk%2) );

    xPredictNonZeroCnt( rcMbDataAccess, cIdx, auiCoeffCnt[uiBlk], auiTrailingOnes[uiBlk] );
    xWriteRunLevel    ( aaiLevelRun[uiBlk],   auiCoeffCnt[uiBlk], auiTrailingOnes[uiBlk], 16, auiTotalRun[uiBlk] );
  }

  return Err::m_nOK;
}


Bool
UvlcWriter::RQencodeCBP_8x8( MbDataAccess& rcMbDataAccess,
                              MbDataAccess& rcMbDataAccessBase,
                              B8x8Idx       c8x8Idx )
{
  UInt uiSymbol  = ( ( rcMbDataAccess.getMbData().getMbCbp() >> c8x8Idx.b8x8Index() ) & 1 ? 1 : 0 );
  UInt uiBaseCbp = rcMbDataAccessBase.getMbData().getMbCbp();
  UInt uiCbp     = rcMbDataAccess.getMbData().getMbCbp();
  B8x8Idx uiFirstCbp;
  UInt ui, uiB;

  if (uiFirstCbp == c8x8Idx) {
    // Maintain stats; split CBP by base context
    UInt uiCode[2] = {0,0};
    UInt uiLen[2]  = {0,0};
    UInt uiVlc[2]  = {0,0};
    UInt uiBaseCtx;
    UInt uiCBlk;
    for (ui=0; ui<4; ui++)
    {
      uiBaseCtx = (uiBaseCbp & 0xf & (1<<ui)) ? 1 : 0;
      uiCBlk    = (uiCbp     & 0xf & (1<<ui)) ? 1 : 0;
      uiLen[uiBaseCtx]++;
      uiCode[uiBaseCtx] <<= 1;
      uiCode[uiBaseCtx] |= uiCBlk;
    }
    // Determine optimal VLC for each context, and av symbol size
    for (uiB=0; uiB<2; uiB++)
    {
      if ( uiLen[uiB] > 0 )
      {
        UInt uiFlip = (m_uiCbpStats[uiB][0] <= m_uiCbpStats[uiB][1]) ? 1 : 0;
        if ( uiFlip )
        {
          uiCode[uiB] = uiCode[uiB] ^ ((1 << uiLen[uiB])-1);
        }
        if ( uiLen[uiB] <= 2 )
        {
          uiVlc[uiB] = 0;
        } else if ( uiLen[uiB] == 3 )
        {
          uiVlc[uiB] = (m_uiCbpStats[uiB][uiFlip] < 2*m_uiCbpStats[uiB][1-uiFlip]) ? 0 : 1;
        } else {
          if (m_uiCbpStats[uiB][uiFlip] < 2*m_uiCbpStats[uiB][1-uiFlip])
          {
            uiVlc[uiB] = 0;
          } else {
            uiVlc[uiB] = 2;
          }
        }
      }
    }
    // Decide on the optimal VLC
    for (uiB=0; uiB<2; uiB++)
    {
      if ( uiLen[uiB] > 0 )
      {
        if ( uiVlc[uiB] == 0 )
        {
          ANOK( xWriteCode( uiCode[uiB], uiLen[uiB] ) );
        } else {
          ANOK( xWriteCode(g_auiISymCode[uiVlc[uiB]][uiCode[uiB]], g_auiISymLen[uiVlc[uiB]][uiCode[uiB]]) );
        }
      }
    }
    // Update stats
    for (ui=0; ui<4; ui++)
    {
      uiBaseCtx = (uiBaseCbp & 0xf & (1<<ui)) ? 1 : 0;
      uiCBlk    = (uiCbp     & 0xf & (1<<ui)) ? 1 : 0;
      m_uiCbpStats[uiBaseCtx][uiCBlk]++;
    }
    // Scale counter if necessary
    for (uiB = 0; uiB < 2; uiB++) {
      if ( m_uiCbpStats[uiB][0] + m_uiCbpStats[uiB][1] > 512 )
      {
        m_uiCbpStats[uiB][0] >>= 4;
        m_uiCbpStats[uiB][1] >>= 4;
      }
    }
    ETRACE_T( "CBP_Luma" );
    ETRACE_V( uiCode[0] );
    ETRACE_N;
    ETRACE_T( "CBP_Luma" );
    ETRACE_V( uiCode[1] );
    ETRACE_N;
  }

  if( uiSymbol )
  {
    rcMbDataAccessBase.getMbData().setMbCbp( rcMbDataAccessBase.getMbData().getMbCbp() | ( 1 << c8x8Idx.b8x8Index() ) );
  }

  return ( uiSymbol == 1 );
}

Bool
UvlcWriter::RQpeekCbp4x4( MbDataAccess&  rcMbDataAccess,
                          MbDataAccess&  rcMbDataAccessBase,
                          LumaIdx        cIdx )
{
  UInt    uiSymbol  = 0;
  TCoeff* piCoeff   = rcMbDataAccess.    getMbTCoeffs().get( cIdx );
  TCoeff* piBCoeff  = rcMbDataAccessBase.getMbTCoeffs().get( cIdx );

  for( UInt ui = 0; ui < 16; ui++ )  
  {
    if( piCoeff[ g_aucFrameScan[ui] ] && !piBCoeff[ g_aucFrameScan[ui] ] )
    {
      uiSymbol = 1;
      break;
    }
  }
  return ( uiSymbol == 1 );
}

Bool
UvlcWriter::RQencodeBCBP_4x4( MbDataAccess&  rcMbDataAccess,
                               MbDataAccess&  rcMbDataAccessBase,
                               LumaIdx        cIdx )
{
  if ( (cIdx.x() %2) == 0 && (cIdx.y() %2) == 0)
  {
    // Write
    UInt    uiCode    = 0;
    UInt    uiLen     = 0;
    UInt uiFlip = (m_uiCbpStat4x4[1] > m_uiCbpStat4x4[0]) ? 1 : 0;
    UInt uiVlc = (m_uiCbpStat4x4[uiFlip] < 2*m_uiCbpStat4x4[1-uiFlip]) ? 0 : 2;

    for( Int iY=cIdx.y(); iY<cIdx.y()+2; iY++)
      for ( Int iX=cIdx.x(); iX<cIdx.x()+2; iX++)
      {
        UInt uiSymbol = 0;
        B4x4Idx cTmp(iY*4+iX);
        uiSymbol = RQpeekCbp4x4(rcMbDataAccess, rcMbDataAccessBase, cTmp);
        rcMbDataAccessBase.getMbData().setBCBP( cTmp, uiSymbol );
        uiCode <<= 1;
        uiCode |= uiSymbol;
        uiLen++;
        m_uiCbpStat4x4[uiSymbol]++;
      }

    if (uiFlip)
      uiCode = uiCode ^ ((1<<uiLen)-1);
    if (uiVlc == 0)
    {
      ANOK( xWriteCode( uiCode, uiLen ) );
    } else {
      ANOK( xWriteCode( g_auiISymCode[2][uiCode], g_auiISymLen[2][uiCode] ) );
    }
    // Scaling
    if (m_uiCbpStat4x4[0]+m_uiCbpStat4x4[1] > 512)
    {
      m_uiCbpStat4x4[0] >>= 1;
      m_uiCbpStat4x4[1] >>= 1;
    }
    ETRACE_T( "BCBP_4x4" );
    ETRACE_V( uiCode );
    ETRACE_N;
  }
  return RQpeekCbp4x4(rcMbDataAccess, rcMbDataAccessBase, cIdx);
}

Bool
UvlcWriter::RQencodeCBP_Chroma( MbDataAccess& rcMbDataAccess,
                                 MbDataAccess& rcMbDataAccessBase )
{
  UInt  uiSymbol          = ( ( rcMbDataAccess.getMbData().getMbCbp() >> 4 ) ? 1 : 0 );
  UInt  uiLeftChromaCbp   = rcMbDataAccessBase.getLeftChromaCbpFGS ();
  UInt  uiAboveChromaCbp  = rcMbDataAccessBase.getAboveChromaCbpFGS();
  UInt  uiCtx             = ( uiLeftChromaCbp > 0 ? 1 : 0 ) + ( uiAboveChromaCbp > 0 ? 2 : 0 );

  ANOK( xWriteFlag( uiSymbol ) );
  ETRACE_T( "CBP_Chroma" );
  ETRACE_V( uiSymbol );
  ETRACE_N;

  if( uiSymbol )
  {
    rcMbDataAccessBase.getMbData().setMbCbp( rcMbDataAccessBase.getMbData().getMbCbp() | 0x10 );
  }
  return ( uiSymbol == 1 );
}

Bool
UvlcWriter::RQencodeBCBP_ChromaDC( MbDataAccess&   rcMbDataAccess,
                                    MbDataAccess&   rcMbDataAccessBase,
                                    ChromaIdx       cIdx )
{
  UInt    uiSymbol  = 0;
  TCoeff* piCoeff   = rcMbDataAccess.getMbTCoeffs().get( cIdx );
  TCoeff* piBCoeff  = rcMbDataAccessBase.getMbTCoeffs().get( cIdx );

  for( UInt ui = 0; ui < 4; ui++ )  
  {
    if( piCoeff[ g_aucIndexChromaDCScan[ui] ] && !piBCoeff[ g_aucIndexChromaDCScan[ui] ] )
    {
      uiSymbol = 1;
      break;
    }
  }

  ANOK( xWriteFlag( uiSymbol ) );
  ETRACE_T( "BCBP_ChromaDC" );
  ETRACE_V( uiSymbol );
  ETRACE_N;

  rcMbDataAccessBase.getMbData().setBCBP( 24 + cIdx.plane(), uiSymbol );
  
  return ( uiSymbol == 1 );
}


Bool
UvlcWriter::RQencodeBCBP_ChromaAC( MbDataAccess&  rcMbDataAccess,
                                    MbDataAccess&  rcMbDataAccessBase,
                                    ChromaIdx      cIdx )
{
  UInt    uiSymbol  = 0;
  TCoeff* piCoeff   = rcMbDataAccess.getMbTCoeffs().get( cIdx );
  TCoeff* piBCoeff  = rcMbDataAccessBase.getMbTCoeffs().get( cIdx );

  for( UInt ui = 1; ui < 16; ui++ )  
  {
    if( piCoeff[ g_aucFrameScan[ui] ] && !piBCoeff[ g_aucFrameScan[ui] ] )
    {
      uiSymbol = 1;
      break;
    }
  }

  ANOK( xWriteFlag( uiSymbol ) );
  ETRACE_T( "BCBP_ChromaAC" );
  ETRACE_V( uiSymbol );
  ETRACE_N;

  rcMbDataAccessBase.getMbData().setBCBP( 16 + cIdx, uiSymbol );
  
  return ( uiSymbol == 1 );
}

Bool
UvlcWriter::RQencodeCBP_ChromaAC( MbDataAccess& rcMbDataAccess,
                                   MbDataAccess& rcMbDataAccessBase )
{
  UInt  uiSymbol          = ( ( rcMbDataAccess.getMbData().getMbCbp() >> 5 ) ? 1 : 0 );
  ANOK( xWriteFlag( uiSymbol ) );
  ETRACE_T( "CBP_ChromaAC" );
  ETRACE_V( uiSymbol );
  ETRACE_N;

  if( uiSymbol )
  {
    rcMbDataAccessBase.getMbData().setMbCbp( ( rcMbDataAccessBase.getMbData().getMbCbp() & 0xF ) | 0x20 );
  }
  return ( uiSymbol == 1 );
}

ErrVal
UvlcWriter::RQencodeDeltaQp( MbDataAccess& rcMbDataAccess,
                              MbDataAccess& rcMbDataAccessBase )
{
  ETRACE_T ("DQp");

  RNOK( xWriteSvlcCode( rcMbDataAccess.getDeltaQp() ) );

  ETRACE_TY ("se(v)");
  ETRACE_N;

  return Err::m_nOK;
}

ErrVal
UvlcWriter::RQencode8x8Flag( MbDataAccess& rcMbDataAccess,
                              MbDataAccess& rcMbDataAccessBase ) 
{
  UInt uiSymbol = rcMbDataAccess.getMbData().isTransformSize8x8() ? 1 : 0;
 
  RNOK( xWriteFlag( uiSymbol ) );
  ETRACE_T( "TRAFO_8x8" );
  ETRACE_V( uiSymbol );
  ETRACE_N;

  rcMbDataAccessBase.getMbData().setTransformSize8x8( rcMbDataAccess.getMbData().isTransformSize8x8() );

  return Err::m_nOK;
}

ErrVal
UvlcWriter::RQencodeNewTCoeff_8x8( MbDataAccess&   rcMbDataAccess,
                                    MbDataAccess&   rcMbDataAccessBase,
                                    B8x8Idx         c8x8Idx,
                                    UInt            uiScanIndex,
                                    UInt&           ruiLast )
{
  TCoeff*       piCoeff     = rcMbDataAccess    .getMbTCoeffs().get8x8( c8x8Idx );
  TCoeff*       piCoeffBase = rcMbDataAccessBase.getMbTCoeffs().get8x8( c8x8Idx );
  const UChar*  pucScan     = g_aucFrameScan64;
  const UInt    uiCtxOffset = 2;
  UInt          uiStop      = 64;

  ROT( piCoeffBase[pucScan[uiScanIndex]] );

  ETRACE_T( "LUMA_8x8_NEW" );
  ETRACE_V( c8x8Idx.b8x8Index() );
  ETRACE_V( uiScanIndex );
  ETRACE_N;

  rcMbDataAccessBase.getMbData().setBCBP( c8x8Idx.b4x4(),   1 );
  rcMbDataAccessBase.getMbData().setBCBP( c8x8Idx.b4x4()+1, 1 );
  rcMbDataAccessBase.getMbData().setBCBP( c8x8Idx.b4x4()+4, 1 );
  rcMbDataAccessBase.getMbData().setBCBP( c8x8Idx.b4x4()+5, 1 );

  //===== end-of-block =====
  if( ruiLast )
  {
    for( UInt ui = uiScanIndex; ui < 64; ui++ )
    {
      if( piCoeff[pucScan[ui]] && ! piCoeffBase[pucScan[ui]] )
      {
        ruiLast = 0;
        break;
      }
    }
    RNOK( xWriteFlag( ruiLast ) );
    ROTRS( ruiLast, Err::m_nOK );
  }
  else
  {
    ruiLast = 0;
  }

  //===== SIGNIFICANCE BIT =====
  UInt uiSig = ( piCoeff[pucScan[uiScanIndex]] ? 1 : 0 );
  RNOK( xWriteFlag( uiSig ) );
  
  if( uiSig )
  {
    UInt  uiAbs     = ( piCoeff[pucScan[uiScanIndex]] > 0 ? piCoeff[pucScan[uiScanIndex]] : -piCoeff[pucScan[uiScanIndex]] );
    UInt  uiSign    = ( piCoeff[pucScan[uiScanIndex]] > 0 ?                             0 :                              1 );

    xWriteLevelVLC0( piCoeff[pucScan[uiScanIndex]] );
  }

  return Err::m_nOK;
}

ErrVal
UvlcWriter::RQencodeNewTCoeff_Luma ( MbDataAccess&   rcMbDataAccess,
                                      MbDataAccess&   rcMbDataAccessBase,
                                      ResidualMode    eResidualMode,
                                      LumaIdx         cIdx,
                                      UInt            uiScanIndex,
                                      Bool&           rbLast,
                                      UInt&           ruiNumCoefWritten )
{
  TCoeff*       piCoeff     = rcMbDataAccess    .getMbTCoeffs().get( cIdx );
  TCoeff*       piCoeffBase = rcMbDataAccessBase.getMbTCoeffs().get( cIdx );
  const UChar*  pucScan     = g_aucFrameScan;
  UInt          uiStart     = 0;
  UInt          uiStop      = 16;

  ROT( piCoeffBase[pucScan[uiScanIndex]] );

  ETRACE_T( "LUMA_4x4_NEW" );
  ETRACE_V( cIdx.b4x4() );
  ETRACE_V( uiScanIndex );
  ETRACE_N;

  RNOK( xRQencodeNewTCoeffs( piCoeff, piCoeffBase, uiStart, uiStop, eResidualMode, pucScan, uiScanIndex, m_auiShiftLuma, rbLast, ruiNumCoefWritten ) );
  return Err::m_nOK;
}

ErrVal
UvlcWriter::RQencodeNewTCoeff_Chroma ( MbDataAccess&   rcMbDataAccess,
                                        MbDataAccess&   rcMbDataAccessBase,
                                        ResidualMode    eResidualMode,
                                        ChromaIdx       cIdx,
                                        UInt            uiScanIndex,
                                        Bool&           rbLast,
                                        UInt&           ruiNumCoefWritten )
{
  TCoeff*       piCoeff     = rcMbDataAccess    .getMbTCoeffs().get( cIdx );
  TCoeff*       piCoeffBase = rcMbDataAccessBase.getMbTCoeffs().get( cIdx );
  const UChar*  pucScan     = ( eResidualMode == CHROMA_DC ? g_aucIndexChromaDCScan : g_aucFrameScan );
  UInt          uiStart     = ( eResidualMode == CHROMA_AC ? 1 : 0  );
  UInt          uiStop      = ( eResidualMode == CHROMA_DC ? 4 : 16 );

  ROT( piCoeffBase[pucScan[uiScanIndex]] );

  ETRACE_T( "CHROMA_4x4_NEW" );
  ETRACE_V( cIdx );
  ETRACE_V( uiScanIndex );
  ETRACE_N;

  RNOK( xRQencodeNewTCoeffs( piCoeff, piCoeffBase, uiStart, uiStop, eResidualMode, pucScan, uiScanIndex, m_auiShiftChroma, rbLast, ruiNumCoefWritten ) );

  return Err::m_nOK;
}

ErrVal
UvlcWriter::xRQencodeNewTCoeffs( TCoeff*       piCoeff,
                                 TCoeff*       piCoeffBase,
                                 UInt          uiStart,
                                 UInt          uiStop,
                                 ResidualMode  eResidualMode,
                                 const UChar*  pucScan,
                                 UInt          uiScanIndex,
                                 UInt*         pauiEobShift,
                                 Bool&         rbLast,
                                 UInt&         ruiNumCoefWritten )
{
  UInt ui;
  UInt uiBaseLast = 0;
  for ( ui=0; ui<uiStop; ui++ )
  {
    if ( piCoeffBase[pucScan[ui]] )
    {
      uiBaseLast = ui;
    }
  }
  UInt uiCycle = 0;
  for ( ui=uiStart; ui<uiScanIndex; ui++ )
  {
    if ( !piCoeffBase[pucScan[ui]] && piCoeff[pucScan[ui]] )
    {
      uiCycle = ui + 1;
    }
  }
  AOF( uiCycle < uiStop );
  Bool bSkipEob = !rbLast;
  ruiNumCoefWritten = 0;

  if( rbLast )
  {
    rbLast = true;
    for( ui = uiScanIndex; ui < uiStop; ui++ )
    {
      if( piCoeff[pucScan[ui]] && !piCoeffBase[pucScan[ui]] )
      {
        rbLast = false;
        break;
      }
    }
    if (rbLast) {

      UInt uiCountMag2;
      UInt uiLastPos = 0;
      for( ui = uiStart; ui < uiStop; ui++ )
      {
        if ( ! piCoeffBase[pucScan[ui]] )
          uiLastPos++;
        if( piCoeff[pucScan[ui]] && ! piCoeffBase[pucScan[ui]])
        {
          uiLastPos = 1;
        }
      }
      RNOK( xRQencodeSigMagGreater1( piCoeff, piCoeffBase, uiLastPos, uiStart, uiStop, uiCycle*16+uiBaseLast, pucScan, uiCountMag2 ) );
      if ( uiCountMag2 == 0 )
      {
        RNOK( xWriteS3Code( min(pauiEobShift[uiCycle], uiLastPos), m_auiVlcTabMap[uiCycle*16 + uiBaseLast] ) );
      }
    }
    ROTRS(rbLast, Err::m_nOK);
  } else
    rbLast = false;

  //===== SIGNIFICANCE BIT ======
  UInt uiSig;
  do
  {
    ruiNumCoefWritten++;

    UInt uiLastScanPosition = uiScanIndex + 1;
    while (uiLastScanPosition < uiStop && piCoeffBase[pucScan[uiLastScanPosition]])
      uiLastScanPosition ++;
  
    if (uiLastScanPosition < uiStop)
    {
      uiSig = piCoeff[pucScan[uiScanIndex] ] ? 1 : 0;
    } else {
      uiSig = 1;
    }

    if( uiSig )
    {
      break;
    }

    uiScanIndex++;
    while (uiScanIndex < uiStop && piCoeffBase[pucScan[uiScanIndex]])
      uiScanIndex++;
  }
  while ( true );
  UInt uiSymbol = ruiNumCoefWritten - ((bSkipEob || ruiNumCoefWritten <= pauiEobShift[uiCycle]) ? 1 : 0);
  RNOK( xWriteS3Code( uiSymbol, m_auiVlcTabMap[uiCycle*16+uiBaseLast] ) );
  RNOK( xWriteFlag( piCoeff[pucScan[uiScanIndex]] < 0 ? 1 : 0 ) );

  // Check whether any more nonzero values
  Bool bFinished = true;
  for( ui=uiScanIndex+1; ui<uiStop; ui++ )
  {
    bFinished &= ( piCoeffBase[pucScan[ui]] != 0 );
    if( !bFinished )
      break;
  }
  if( bFinished )
  {
    UInt uiCountMag2;
    RNOK( xRQencodeSigMagGreater1( piCoeff, piCoeffBase, 0, uiStart, uiStop, uiCycle*16+uiBaseLast, pucScan, uiCountMag2 ) );
    if( uiCountMag2 == 0 )
    {
      RNOK( xWriteS3Code( 0, m_auiVlcTabMap[uiCycle*16 + uiBaseLast] ) );
    }
  }
  return Err::m_nOK;
}

ErrVal
UvlcWriter::xRQencodeSigMagGreater1( TCoeff* piCoeff,
                                     TCoeff* piCoeffBase,
                                     UInt    uiBaseCode,
                                     UInt    uiStart,
                                     UInt    uiStop,
                                     UInt    uiVlcTable,
                                     const UChar*  pucScan,
                                     UInt&   ruiNumMagG1 )
{
  // Any magnitudes greater than one?
  ruiNumMagG1      = 0;
  UInt uiCountMag1 = 0;
  UInt uiMaxMag    = 0;
  UInt ui;
  for( ui = uiStart; ui < uiStop; ui++ )
  {
    if( piCoeff[pucScan[ui]] && ! piCoeffBase[pucScan[ui]])
    {
      uiCountMag1++;
      UInt uiAbs = ( piCoeff[pucScan[ui]] < 0 ? -piCoeff[pucScan[ui]] : piCoeff[pucScan[ui]] );
      if ( uiAbs > 1 )
      {
        ruiNumMagG1++;
      }
      if ( uiAbs > uiMaxMag )
      {
        uiMaxMag = uiAbs;
      }
    }
  }

  if( ruiNumMagG1 == 0 )
  {
    return Err::m_nOK;
  }

  // Find optimal terminating code
  UInt uiTermSym;
  if ( uiMaxMag < 4 )
  {
    uiTermSym = 2*(ruiNumMagG1-1) + uiMaxMag%2;
  } else {
    uiTermSym = uiCountMag1*(uiMaxMag-2) + ruiNumMagG1 - 1;
  }
  RNOK( xWriteS3Code( uiBaseCode+uiTermSym+1, m_auiVlcTabMap[uiVlcTable] ) );

  UInt uiFlip      = 0;
  UInt uiRemaining = ruiNumMagG1;
  UInt uiBegin     = 0;
  UInt uiEnd       = uiCountMag1;
  UInt uiCount     = 0;
  for( ui = uiStart; ui < uiStop; ui++ )
  {
    if( piCoeff[pucScan[ui]] && ! piCoeffBase[pucScan[ui]])
    {
      // Coding last value(s) may be unnecessary
      if ( uiRemaining == uiEnd-uiCount )
        break;
      // Range check for interval splitting
      uiCount++;
      if ( uiCount <= uiBegin )
        continue;
      if ( uiCount > uiEnd )
        break;
      UInt uiAbs = ( piCoeff[pucScan[ui]] < 0 ? -piCoeff[pucScan[ui]] : piCoeff[pucScan[ui]] );
      RNOK( xWriteFlag( (uiAbs > 1) ? 1 : 0 ) );
      uiRemaining -= ( uiAbs > 1 ) ? 1-uiFlip : uiFlip;
      if ( uiRemaining == 0 )
        break;
    }
  }
  UInt uiOutstanding = ruiNumMagG1;
  Bool bSeenMaxMag   = false;
  for( ui = uiStart; ui < uiStop; ui++ )
  {
    if( !bSeenMaxMag && uiOutstanding == 1 )
      break;
    if( piCoeff[pucScan[ui]] && ! piCoeffBase[pucScan[ui]])
    {
      UInt uiAbs = ( piCoeff[pucScan[ui]] < 0 ? -piCoeff[pucScan[ui]] : piCoeff[pucScan[ui]] );
      bSeenMaxMag |= ( uiAbs == uiMaxMag );
      for ( UInt uiCutoff=1; uiAbs>uiCutoff && uiCutoff<uiMaxMag; uiCutoff++ )
      {
        RNOK( xWriteFlag( uiAbs > (uiCutoff+1) ) );
      }
      if( uiAbs > 1 )
        uiOutstanding--;
      if( uiOutstanding == 0 )
        break;
    }
  }
  return Err::m_nOK;
}

ErrVal
UvlcWriter::RQencodeTCoeffRef_8x8( MbDataAccess&   rcMbDataAccess,
                                    MbDataAccess&   rcMbDataAccessBase,
                                    B8x8Idx         c8x8Idx,
                                    UInt            uiScanIndex )
{
  TCoeff*       piCoeff     = rcMbDataAccess    .getMbTCoeffs().get8x8( c8x8Idx );
  TCoeff*       piCoeffBase = rcMbDataAccessBase.getMbTCoeffs().get8x8( c8x8Idx );
  const UChar*  pucScan     = g_aucFrameScan64;

  ROF( piCoeffBase[pucScan[uiScanIndex]] );

  ETRACE_T( "LUMA_8x8_REF" );
  ETRACE_V( c8x8Idx.b8x8Index() );
  ETRACE_V( uiScanIndex );
  ETRACE_N;

  UChar ucCode[64];
  UInt uiLen      = 0;
  UInt uiPos;
  UInt uiFirstPos = 64;

  UInt ui;
  for ( ui=0; ui<64; ui++ )
  {
    UInt uiSig = ( piCoeffBase[pucScan[ui]] ? 1 : 0 );
    if ( uiSig )
    {
      ucCode[uiLen] = (piCoeff[pucScan[ui]] ? 1 : 0);
      uiLen++;
      if (ui == uiScanIndex)
        uiPos = uiLen;
      if (ui < uiFirstPos)
        uiFirstPos = ui;
    }
  }
  if (uiScanIndex == uiFirstPos)
  {
    for( ui=0; ui<64; ui++ )
    {
      if ( piCoeffBase[pucScan[ui]] )
        RNOK( m_pBitGrpRef->Write( piCoeff[pucScan[ui]] ? 1 : 0 ));
    }
    RNOK( m_pBitGrpRef->Flush() );
    for ( UInt ui=0; ui<64; ui++ )
    {
      UInt uiSig = ( piCoeffBase[pucScan[ui]] ? 1 : 0 );
      UInt uiSigEL = ( piCoeff[pucScan[ui]] ? 1 : 0 );
      if ( uiSig && uiSigEL )
      {
        UInt uiSignBL = ( piCoeffBase[pucScan[ui]] < 0 ? 1 : 0 );
        UInt uiSignEL = ( piCoeff    [pucScan[ui]] < 0 ? 1 : 0 );
        UInt uiSymbol = ( uiSignBL ^ uiSignEL );
        RNOK( m_pBitGrpSgn->Write( uiSymbol ) );
      }
    }
    RNOK( m_pBitGrpSgn->Flush() );
  }

  return Err::m_nOK;
}

ErrVal
UvlcWriter::RQencodeTCoeffRef_Luma ( MbDataAccess&   rcMbDataAccess,
                                      MbDataAccess&   rcMbDataAccessBase,
                                      ResidualMode    eResidualMode,
                                      LumaIdx         cIdx,
                                      UInt            uiScanIndex,
                                      UInt            uiNumSig )
{
  TCoeff*       piCoeff     = rcMbDataAccess    .getMbTCoeffs().get( cIdx );
  TCoeff*       piCoeffBase = rcMbDataAccessBase.getMbTCoeffs().get( cIdx );
  const UChar*  pucScan     = g_aucFrameScan;
  UInt          uiStart     = 0;
  UInt          uiStop      = 16;

  ROF( piCoeffBase[pucScan[uiScanIndex]] );

  ETRACE_T( "LUMA_4x4_REF" );
  ETRACE_V( cIdx.b4x4() );
  ETRACE_V( uiScanIndex );
  ETRACE_N;

  RNOK( xRQencodeTCoeffsRef( piCoeff, piCoeffBase, pucScan, uiScanIndex, uiStop, uiNumSig ) );
  return Err::m_nOK;
}

ErrVal
UvlcWriter::xRQencodeTCoeffsRef( TCoeff*       piCoeff,
                                 TCoeff*       piCoeffBase,
                                 const UChar*  pucScan,
                                 UInt          uiScanIndex,
                                 UInt          uiStop,
                                 UInt          uiNumSig)
{
  UInt uiSig = ( piCoeff[pucScan[uiScanIndex]] ? 1 : 0 );
  m_pBitGrpRef->Write( uiSig );
  m_auiSigMap[uiScanIndex] = uiSig;

  if( uiNumSig == 1 )
  {
    m_pBitGrpRef->Flush();
    for( UInt ui=0; ui<uiStop; ui++ )
    {
      if( m_auiSigMap[ui] )
      {
        UInt uiSignBL = ( piCoeffBase[pucScan[ui]] < 0 ? 1 : 0 );
        UInt uiSignEL = ( piCoeff    [pucScan[ui]] < 0 ? 1 : 0 );
        UInt uiSymbol = ( uiSignBL ^ uiSignEL );
        RNOK( m_pBitGrpSgn->Write( uiSymbol ) );
      }
    }
    RNOK( m_pBitGrpSgn->Flush() );
    ::memset( m_auiSigMap, 0, 16*sizeof(UInt) );
  }

  return Err::m_nOK;
}

ErrVal
UvlcWriter::RQencodeTCoeffRef_Chroma ( MbDataAccess&   rcMbDataAccess,
                                        MbDataAccess&   rcMbDataAccessBase,
                                        ResidualMode    eResidualMode,
                                        ChromaIdx       cIdx,
                                        UInt            uiScanIndex,
                                        UInt            uiNumSig )
{
  TCoeff*       piCoeff     = rcMbDataAccess    .getMbTCoeffs().get( cIdx );
  TCoeff*       piCoeffBase = rcMbDataAccessBase.getMbTCoeffs().get( cIdx );
  const UChar*  pucScan     = ( eResidualMode == CHROMA_DC ? g_aucIndexChromaDCScan : g_aucFrameScan );
  UInt          uiStart     = ( eResidualMode == CHROMA_AC ? 1 : 0  );
  UInt          uiStop      = ( eResidualMode == CHROMA_DC ? 4 : 16 );


  ETRACE_T( "CHROMA_4x4_REF" );
  ETRACE_V( cIdx );
  ETRACE_V( uiScanIndex );
  ETRACE_N;

  RNOK( xRQencodeTCoeffsRef( piCoeff, piCoeffBase, pucScan, uiScanIndex, uiStop, uiNumSig ) );
  return Err::m_nOK;
}

ErrVal
UvlcWriter::RQencodeCycleSymbol( UInt uiCycle )
{
  RNOK( xWriteFlag( uiCycle > 0 ) );
  if ( uiCycle > 0 )
    RNOK( xWriteFlag( uiCycle - 1 ) );
  return Err::m_nOK;
}

ErrVal
UvlcWriter::xWriteGolomb(UInt uiSymbol, UInt uiK)
{
  UInt uiQ = uiSymbol / uiK;
  UInt uiR = uiSymbol - uiQ * uiK;
  UInt uiC = 0;
  UInt uiT = uiK >> 1;

  while ( uiT > 0 )
  {
    uiC++;
    uiT >>= 1;
  }

  // Unary part
  for ( UInt ui = 0; ui < uiQ; ui++ )
  {
    RNOK( xWriteFlag( 1 ) );
  }
  RNOK( xWriteFlag( 0 ) );

  // Binary part
  if ( uiR < uiC )
  {
    RNOK( xWriteCode( uiR, uiC ) );
  } else if ( uiC > 0 ) {
    RNOK( xWriteFlag( 1 ) );
    RNOK( xWriteCode( uiR - uiC, uiC ) );
  }
  ETRACE_N;

  return Err::m_nOK;
}

UInt
UvlcWriter::peekGolomb(UInt uiSymbol, UInt uiK)
{
  UInt uiQ = uiSymbol / uiK;
  UInt uiR = uiSymbol - uiQ * uiK;
  UInt uiC = 0;
  UInt uiT = uiK >> 1;

  while ( uiT > 0 )
  {
    uiC++;
    uiT >>= 1;
  }

  // Unary part
  uiT = uiQ + 1 + uiC;
  if ( uiR >= uiC && uiC > 0 )
  {
    uiT++;
  }

  return uiT;
}

ErrVal
UvlcWriter::RQencodeEobOffsets_Luma( UInt* pauiSeq )
{
  m_pBitGrpRef    ->Init();
  m_pBitGrpSgn    ->Init();
  m_uiCbpStat4x4[0] = m_uiCbpStat4x4[1] = 0;
  m_uiCbpStats[0][0] = m_uiCbpStats[0][1] = m_uiCbpStats[1][0] = m_uiCbpStats[1][1] = 0;

  memcpy( m_auiShiftLuma, pauiSeq, sizeof(UInt)*16 );
  return xRQencodeEobOffsets(pauiSeq, 16);
}

ErrVal
UvlcWriter::RQencodeEobOffsets_Chroma( UInt* auiSeq )
{
  memcpy( m_auiShiftChroma, auiSeq, sizeof(UInt)*16 );
  m_auiShiftChroma[0] = 15;
  return xRQencodeEobOffsets(auiSeq+1, 15);
}

ErrVal
UvlcWriter::xRQencodeEobOffsets( UInt* auiSeq, UInt uiMax )
{
  UInt uiNumEnd = 0;
  for (UInt uiEc=0; uiEc<uiMax && auiSeq[uiEc] == uiMax-1 && uiNumEnd<3; uiEc++)
  {
    uiNumEnd++;
  }

  if ( uiNumEnd )
  {
    RNOK( xWriteGolomb( uiNumEnd-1, 1 ) );
  } else {
    ETRACE_T("Eob");
    RNOK( xWriteCode( 0x7, 3 ) );
    ETRACE_N;
  }
  RNOK( xWriteGolomb( auiSeq[uiNumEnd], 2 ) );
  RNOK( xEncodeMonSeq( auiSeq+uiNumEnd+1, auiSeq[uiNumEnd], uiMax-uiNumEnd-1 ) );
  ETRACE_N;
  return Err::m_nOK;
}

ErrVal
UvlcWriter::RQencodeVlcTableMap( UInt* pauiTable, UInt uiMaxH, UInt uiMaxV )
{
  memcpy( m_auiVlcTabMap, pauiTable, sizeof(UInt)*uiMaxH*uiMaxV );
  // Form vector for first column
  RNOK( xWriteGolomb( pauiTable[0], 1 ) );
  UInt* auiCol = new UInt[uiMaxV];
  UInt* puiCurrVal = pauiTable;
  UInt  uiV, uiH;
  for (uiV=0; uiV<uiMaxV; uiV++,puiCurrVal+=uiMaxH)
  {
    auiCol[uiV] = 5 - *puiCurrVal;
  }
  for (uiV=uiMaxV-1; uiV>0; uiV--)
  {
    if (auiCol[uiV] == auiCol[uiV-1])
    {
      auiCol[uiV] = 0;
    } else {
      break;
    }
  }
  RNOK( xEncodeMonSeq( auiCol+1, auiCol[0], uiMaxV-1 ) );
  // Get rid of EOB
  for (uiV=1; uiV<uiMaxV; uiV++)
  {
    if (auiCol[uiV] == 0)
    {
      auiCol[uiV] = auiCol[uiV-1];
    }
  }
  puiCurrVal = pauiTable;
  UInt* auiRow = new UInt[uiMaxH];
  for (uiV=0; uiV<uiMaxV; uiV++,puiCurrVal+=uiMaxH)
  {
    auiRow[0] = auiCol[uiV];
    for (uiH=1; uiH<uiMaxH; uiH++)
    {
      auiRow[uiH] = 5 - puiCurrVal[uiH];
    }
    for (uiH=uiMaxH-1; uiH>0; uiH--)
    {
      if (auiRow[uiH] == auiRow[uiH-1])
      {
        auiRow[uiH] = 0;
      } else {
        break;
      }
    }
    RNOK( xEncodeMonSeq( auiRow+1, auiRow[0], uiMaxH-1 ) );
  }
  delete auiRow;
  delete auiCol;
  return Err::m_nOK;
}

ErrVal
UvlcWriter::xEncodeMonSeq ( UInt* auiSeq, UInt uiStartVal, UInt uiLen )
{
  UInt uiRun   = 0;
  UInt uiLevel = uiStartVal;
  for (UInt uiPos=0; uiPos<uiLen && uiLevel > 0; uiPos++)
  {
    if (auiSeq[uiPos] == uiLevel)
    {
      uiRun++;
    } else {
      RNOK( xWriteGolomb( uiRun, 1 ) );
      uiRun = 1;
      uiLevel--;
      while ( uiLevel > auiSeq[uiPos] )
      {
        RNOK( xWriteGolomb( 0, 1 ) );
        uiLevel--;
      }
    }
  }
  if (uiLevel > 0)
  {
    RNOK( xWriteGolomb( uiRun, 1 ) );
  }
  return Err::m_nOK;
}

ErrVal
UvlcWriter::xWriteS3Code ( UInt uiSymbol, UInt uiCutoff )
{
  if ( uiSymbol <= uiCutoff )
  {
    RNOK( xWriteGolomb( uiSymbol, 1 ) );
  }
  else
  {
    UInt uiLen = (uiSymbol - uiCutoff-1)/3 *2 + uiCutoff+1;
    RNOK( xWriteCode( (1 << uiLen) - 1, uiLen ) );
    RNOK( xWriteCode( (uiSymbol - uiCutoff-1)%3, 2) );
  }
  return Err::m_nOK;
}

UcBitGrpWriter::UcBitGrpWriter( UvlcWriter* pParent,
                                UInt uiInitTable,
                                UInt uiScaleFac,
                                UInt uiScaleLimit,
                                UInt uiStabPeriod )
{
  m_pParent      = pParent;
  m_uiScaleFac   = uiScaleFac;
  m_uiScaleLimit = uiScaleLimit;
  m_uiInitTable  = uiInitTable;
  m_uiStabPeriod = uiStabPeriod;
  Init();
}

ErrVal
UcBitGrpWriter::Init()
{
  m_auiSymCount[0] = 0;
  m_auiSymCount[1] = 0;
  m_uiCode         = 0;
  m_uiLen          = 0;
  m_uiFlip         = 0;
  m_uiTable        = m_uiInitTable;

  return Err::m_nOK;
}

ErrVal
UcBitGrpWriter::Write( UChar ucBit )
{
  AOF((ucBit & 0xfe) == 0);

  m_auiSymCount[ucBit]++;

  if ( m_uiTable == 0 )
  {
    RNOK( m_pParent->writeFlag( ( ucBit ^ m_uiFlip ) != 0 , "" ) );
    m_uiCode = 0;
    m_uiLen  = 0;
    RNOK( xUpdate() );
  } else {
    m_uiCode <<= 1;
    m_uiCode += ucBit;
    m_uiLen++;

    if ( m_uiLen == ((m_uiTable == 1 ) ? 3 : 4) )
    {
      if ( m_uiFlip )
      {
        m_uiCode = m_uiCode ^ ((1 << m_uiLen) - 1);
      }
      RNOK( m_pParent->writeCode( g_auiISymCode[m_uiTable][m_uiCode], g_auiISymLen[m_uiTable][m_uiCode], "" ) );
      m_uiCode   = 0;
      m_uiLen    = 0;
      RNOK( xUpdate() );
    }
  }

  return Err::m_nOK;
}

ErrVal
UcBitGrpWriter::Flush()
{
  if ( m_uiLen == 0 )
    return Err::m_nOK;

  if ( m_uiFlip )
  {
    m_uiCode = m_uiCode ^ ((1 << m_uiLen) - 1);
  }
  if ( m_uiLen <= 2 )
  {
    RNOK( m_pParent->writeCode( m_uiCode, m_uiLen, "" ) );
  } else {
    RNOK( m_pParent->writeCode( g_auiISymCode[1][m_uiCode], g_auiISymLen[1][m_uiCode], "" ) );
  }

  RNOK( xUpdate() );

  m_uiCode = 0;
  m_uiLen  = 0;
  return Err::m_nOK;
}

ErrVal
UcBitGrpWriter::xUpdate()
{
  if (m_auiSymCount[0] + m_auiSymCount[1] > m_uiStabPeriod)
  {
    m_uiFlip  = ( m_auiSymCount[0] < m_auiSymCount[1] ) ? 1 : 0;
    m_uiTable = (m_auiSymCount[m_uiFlip] < 2*m_auiSymCount[1-m_uiFlip]) ? 0
                  : ((7*m_auiSymCount[1-m_uiFlip]<=m_auiSymCount[m_uiFlip]) ? 2 : 1);
  }

  if ( m_auiSymCount[0] + m_auiSymCount[1] > m_uiScaleLimit )
  {
    m_auiSymCount[0] >>= m_uiScaleFac;
    m_auiSymCount[1] >>= m_uiScaleFac;
  }

  return Err::m_nOK;
}

H264AVC_NAMESPACE_END
