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






#if !defined(AFX_UVLCWRITER_H__EA98D347_89D5_4D2D_B6D5_FB3A374CD295__INCLUDED_)
#define AFX_UVLCWRITER_H__EA98D347_89D5_4D2D_B6D5_FB3A374CD295__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MbSymbolWriteIf.h"
#include "H264AVCCommonLib/HeaderSymbolWriteIf.h"
#include "BitWriteBufferIf.h"



H264AVC_NAMESPACE_BEGIN

#define REFSYM_MB                                     384

class UvlcWriter :
public MbSymbolWriteIf
, public HeaderSymbolWriteIf

{
public://
//protected://JVT-X046
	UvlcWriter( Bool bTraceEnable = false );
	virtual ~UvlcWriter();

  ErrVal xWriteSigRunCode ( UInt uiSymbol, UInt uiTableIdx );
  ErrVal xWriteUnaryCode (UInt uiSymbol );
  ErrVal xWriteCodeCB1 (UInt uiSymbol );
  ErrVal xWriteCodeCB2 (UInt uiSymbol );
  ErrVal xWriteGolomb(UInt uiSymbol, UInt uiK);
  ErrVal xEncodeMonSeq ( UInt* auiSeq, UInt uiStartVal, UInt uiLen );

public:
  static ErrVal create( UvlcWriter*& rpcUvlcWriter, Bool bTraceEnable = true );
  ErrVal destroy();

  ErrVal init(  BitWriteBufferIf* pcBitWriteBufferIf );
  ErrVal uninit();

  HeaderSymbolWriteIf* getHeaderSymbolWriteIfNextSlice( Bool bStartNewBitstream ) { return xGetUvlcWriterNextSlice( bStartNewBitstream ); }
  MbSymbolWriteIf*     getSymbolWriteIfNextSlice()                                { return xGetUvlcWriterNextSlice( false ); }
  Void                 setTraceEnableBit( Bool bActive )                          { m_bTraceEnable = bActive; }

  ErrVal  startSlice( const SliceHeader& rcSliceHeader );
  ErrVal  getLastByte(UChar &uiLastByte, UInt &uiLastBitPos); //FIX_FRAG_CAVLC
  ErrVal  setFirstBits(UChar ucByte,UInt uiLastBitPos); //FIX_FRAG_CAVLC
  ErrVal  finishSlice();

  ErrVal closeSlice();

  ErrVal  blockModes( MbDataAccess& rcMbDataAccess );
  ErrVal  mbMode     ( MbDataAccess& rcMbDataAccess );
  ErrVal  resPredFlag( MbDataAccess& rcMbDataAccess );
  ErrVal  fieldFlag  ( MbDataAccess& rcMbDataAccess );

  ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx );
  ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  );
  ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  );
  ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  );
  ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx8x4 eSParIdx );
  ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx4x8 eSParIdx );
  ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx4x4 eSParIdx );

  ErrVal  cbp( MbDataAccess& rcMbDataAccess, UInt uiStart, UInt uiStop );

  ErrVal  refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx );
  ErrVal  refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  );
  ErrVal  refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  );
  ErrVal  refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  );

  ErrVal  motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx );
  ErrVal  motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  );
  ErrVal  motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  );
  ErrVal  motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  );

  ErrVal  residualBlock( MbDataAccess& rcMbDataAccess, LumaIdx cIdx, ResidualMode eResidualMode, UInt uiStart = 0, UInt uiStop = 16 );
  ErrVal  residualBlock( MbDataAccess& rcMbDataAccess, ChromaIdx cIdx, ResidualMode eResidualMode, UInt uiStart = 0, UInt uiStop = 16 );
  ErrVal  transformSize8x8Flag( MbDataAccess& rcMbDataAccess, UInt uiStart, UInt uiStop );
  ErrVal  residualBlock8x8    ( MbDataAccess& rcMbDataAccess, B8x8Idx cIdx, ResidualMode eResidualMode, UInt uiStart = 0, UInt uiStop = 16 );

  ErrVal  deltaQp( MbDataAccess& rcMbDataAccess );
  ErrVal  intraPredModeLuma( MbDataAccess& rcMbDataAccess, LumaIdx cIdx );
  ErrVal  intraPredModeChroma( MbDataAccess& rcMbDataAccess );
  ErrVal  samplesPCM( MbDataAccess& rcMbDataAccess );
  ErrVal  skipFlag( MbDataAccess& rcMbDataAccess, Bool bNotAllowed );
  ErrVal  BLSkipFlag( MbDataAccess& rcMbDataAccess );
  ErrVal  terminatingBit ( UInt uiIsLast ) { return Err::m_nOK;}

  ErrVal writeUvlc( UInt uiCode, Char* pcTraceString );
  ErrVal writeSvlc( Int iCode, Char* pcTraceString );
  ErrVal writeCode( UInt uiCode, UInt uiLength, Char* pcTraceString );
  ErrVal writeSCode( Int iCode, UInt uiLength, Char* pcTraceString );
  ErrVal writeFlag( Bool bFlag, Char* pcTraceString );

  UInt getNumberOfWrittenBits();

private:
  UvlcWriter* xGetUvlcWriterNextSlice( Bool bStartNewBitstream );
protected:

  UInt xGetCoeffCost() { return m_uiCoeffCost; }
  Void xSetCoeffCost(UInt uiCost) { m_uiCoeffCost = uiCost; }

  ErrVal xPredictNonZeroCnt( MbDataAccess& rcMbDataAccess, LumaIdx cIdx, UInt uiCoeffCount, UInt uiTrailingOnes, UInt uiStart, UInt uiStop );
  ErrVal xPredictNonZeroCnt( MbDataAccess& rcMbDataAccess, ChromaIdx cIdx, UInt uiCoeffCount, UInt uiTrailingOnes, UInt uiStart, UInt uiStop );

  ErrVal xWriteMvd( Mv cMv );

  ErrVal xWriteTrailingOnes4( UInt uiCoeffCount, UInt uiTrailingOnes );
  ErrVal xWriteTrailingOnes16( UInt uiLastCoeffCount, UInt uiCoeffCount, UInt uiTrailingOnes );
  ErrVal xWriteTotalRun4( UInt uiVlcPos, UInt uiTotalRun );
  ErrVal xWriteTotalRun16( UInt uiVlcPos, UInt uiTotalRun );
  ErrVal xWriteLevelVLC0( Int iLevel );
  ErrVal xWriteLevelVLCN( Int iLevel, UInt uiVlcLength );
  ErrVal xWriteRun( UInt uiVlcPos, UInt uiRun );
  ErrVal xWriteRunLevel( Int* aiLevelRun, UInt uiCoeffCnt, UInt uiTrailingOnes, UInt uiMaxCoeffs, UInt uiTotalRun, MbDataAccess &rcMbDataAccess );
  ErrVal xWriteRefFrame( Bool bWriteBit, UInt uiRefFrame );
  ErrVal xWriteMotionPredFlag( Bool bFlag );

  ErrVal xWriteUvlcCode( UInt uiVal);
  ErrVal xWriteSvlcCode( Int iVal);

	//JVT-X046 {
	UInt getBitCounter(void)					{return m_uiBitCounter;				}
	UInt getPosCounter(void)					{return m_uiPosCounter;				}

	UInt getCoeffCost(void)						{return m_uiCoeffCost;				}
	Bool getTraceEnable(void)					{return m_bTraceEnable;				}

	Bool getRunLengthCoding(void)			{return m_bRunLengthCoding;		}
	UInt getRun(void)									{return m_uiRun;							}

	BitWriteBufferIf* getBitWriteBufferIf(void){return m_pcBitWriteBufferIf;}
	void loadCabacWrite(MbSymbolWriteIf *pcMbSymbolWriteIf)	{}
	void loadUvlcWrite(MbSymbolWriteIf *pcMbSymbolWriteIf);
	UInt getBitsWritten(void) {  return m_pcBitWriteBufferIf->getBitsWritten(); }
	//JVT-X046 }

protected:
  UInt xConvertToUInt( Int iValue )  {  return ( iValue <= 0) ? -iValue<<1 : (iValue<<1)-1; }

private:
  __inline ErrVal xWriteCode( UInt uiCode, UInt uiLength );
  __inline ErrVal xWriteFlag( UInt uiCode );

protected:
  BitWriteBufferIf* m_pcBitWriteBufferIf;
  UInt m_uiBitCounter;
  UInt m_uiPosCounter;

  UInt m_uiCoeffCost;
  Bool m_bTraceEnable;

  Bool m_bRunLengthCoding;
  UInt m_uiRun;

  // new variables for switching bitstream inputs
  UvlcWriter       *m_pcNextUvlcWriter;
};


H264AVC_NAMESPACE_END


#endif // !defined(AFX_UVLCWRITER_H__EA98D347_89D5_4D2D_B6D5_FB3A374CD295__INCLUDED_)
