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

class UcSymGrpWriter; 

class UvlcWriter :
public MbSymbolWriteIf
, public HeaderSymbolWriteIf

{
protected:
	UvlcWriter( Bool bTraceEnable = false );
	virtual ~UvlcWriter();
  ErrVal xWriteMvdComponentQPel ( Short sMvdComp );
  ErrVal xWriteMvdQPel          ( Mv cMv );
  ErrVal xRQencodeNewTCoeffs( TCoeff*       piCoeff,
                                    TCoeff*       piCoeffBase,
                                    UInt          uiStart,
                                    UInt          uiStop,
                                    ResidualMode  eResidualMode,
                                    const UChar*  pucScan,
                                    UInt          uiScanIndex,
                                    UInt*         pauiEobShift,
                                    Bool&         rbLast,
                                    UInt&         ruiNumCoefWritten );
  ErrVal xRQencodeTCoeffsRef( TCoeff*       piCoeff,
                                  TCoeff*       piCoeffBase,
                                  const UChar*  pucScan,
                                  UInt          uiScanIndex );

  ErrVal xRQencodeSigMagGreater1( TCoeff* piCoeff, TCoeff* piCoeffBase, UInt uiBaseCode, UInt uiStart, UInt uiStop, UInt uiVlcTable, const UChar*  pucScan, UInt& ruiNumMagG1 );
  ErrVal xWriteSigRunCode ( UInt uiSymbol, UInt uiTableIdx );
  ErrVal xWriteUnaryCode (UInt uiSymbol );
  ErrVal xWriteCodeCB1 (UInt uiSymbol );
  ErrVal xWriteCodeCB2 (UInt uiSymbol );
  ErrVal xWriteGolomb(UInt uiSymbol, UInt uiK);
  ErrVal xEncodeMonSeq ( UInt* auiSeq, UInt uiStartVal, UInt uiLen );
  ErrVal xRQencodeEobOffsets ( UInt* auiSeq, UInt uiLen );
  UInt m_auiShiftLuma[16];
  UInt m_auiShiftChroma[16];
  UInt m_auiBestCodeTabMap[16];
  UInt m_uiCbpStats[3][2];
  UcSymGrpWriter* m_pSymGrp; 
  UInt m_uiCbpStat4x4[2];

public:
  static ErrVal create( UvlcWriter*& rpcUvlcWriter, Bool bTraceEnable = true );
  ErrVal destroy();

  ErrVal init(  BitWriteBufferIf* pcBitWriteBufferIf );
  ErrVal uninit();

  ErrVal  startSlice( const SliceHeader& rcSliceHeader );
  ErrVal  startFragment(); //JVT-P031
  ErrVal  getLastByte(UChar &uiLastByte, UInt &uiLastBitPos); //FIX_FRAG_CAVLC
  ErrVal  setFirstBits(UChar ucByte,UInt uiLastBitPos); //FIX_FRAG_CAVLC
  ErrVal  finishSlice();

  ErrVal closeSlice();

  ErrVal  blFlag    ( MbDataAccess& rcMbDataAccess );
  ErrVal  blockModes( MbDataAccess& rcMbDataAccess );
  ErrVal  mbMode( MbDataAccess& rcMbDataAccess/*, Bool bBLQRefFlag*/ );
  ErrVal  resPredFlag( MbDataAccess& rcMbDataAccess );
	ErrVal  smoothedRefFlag( MbDataAccess& rcMbDataAccess );	// JVT-R091

  ErrVal  mvdQPel ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx                      );
  ErrVal  mvdQPel ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  );
  ErrVal  mvdQPel ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  );
  ErrVal  mvdQPel ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  );

  ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx );
  ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  );
  ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  );
  ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  );
  ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx8x4 eSParIdx );
  ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx4x8 eSParIdx );
  ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx4x4 eSParIdx );

  ErrVal  cbp( MbDataAccess& rcMbDataAccess );
  ErrVal  refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx );
  ErrVal  refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  );
  ErrVal  refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  );
  ErrVal  refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  );

  ErrVal  motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx );
  ErrVal  motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  );
  ErrVal  motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  );
  ErrVal  motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  );

  ErrVal  residualBlock( MbDataAccess& rcMbDataAccess, LumaIdx cIdx, ResidualMode eResidualMode );
  ErrVal  residualBlock( MbDataAccess& rcMbDataAccess, ChromaIdx cIdx, ResidualMode eResidualMode );

  ErrVal  transformSize8x8Flag( MbDataAccess& rcMbDataAccess );
  ErrVal  residualBlock8x8    ( MbDataAccess& rcMbDataAccess, B8x8Idx cIdx, ResidualMode eResidualMode );

  ErrVal  deltaQp( MbDataAccess& rcMbDataAccess );
  ErrVal  intraPredModeLuma( MbDataAccess& rcMbDataAccess, LumaIdx cIdx );
  ErrVal  intraPredModeChroma( MbDataAccess& rcMbDataAccess );
  ErrVal  samplesPCM( MbDataAccess& rcMbDataAccess );
  ErrVal  skipFlag( MbDataAccess& rcMbDataAccess, Bool bNotAllowed );
  ErrVal  BLSkipFlag( MbDataAccess& rcMbDataAccess );
  ErrVal  BLQRefFlag( MbDataAccess& rcMbDataAccess );
  ErrVal  terminatingBit ( UInt uiIsLast ) { return Err::m_nOK;}

  ErrVal writeUvlc( UInt uiCode, Char* pcTraceString );
  ErrVal writeSvlc( Int iCode, Char* pcTraceString );
  ErrVal writeCode( UInt uiCode, UInt uiLength, Char* pcTraceString );
  ErrVal writeSCode( Int iCode, UInt uiLength, Char* pcTraceString );
  ErrVal writeFlag( Bool bFlag, Char* pcTraceString );

  UInt getNumberOfWrittenBits();

  UInt xGetCoeffCost() { return m_uiCoeffCost; }
  Void xSetCoeffCost(UInt uiCost) { m_uiCoeffCost = uiCost; }

  ErrVal xPredictNonZeroCnt( MbDataAccess& rcMbDataAccess, LumaIdx cIdx, UInt uiCoeffCount, UInt uiTrailingOnes );
  ErrVal xPredictNonZeroCnt( MbDataAccess& rcMbDataAccess, ChromaIdx cIdx, UInt uiCoeffCount, UInt uiTrailingOnes );

  ErrVal xWriteMvd( Mv cMv );

  ErrVal xWriteTrailingOnes4( UInt uiCoeffCount, UInt uiTrailingOnes );
  ErrVal xWriteTrailingOnes16( UInt uiLastCoeffCount, UInt uiCoeffCount, UInt uiTrailingOnes );
  ErrVal xWriteTotalRun4( UInt uiVlcPos, UInt uiTotalRun );
  ErrVal xWriteTotalRun16( UInt uiVlcPos, UInt uiTotalRun );
  ErrVal xWriteLevelVLC0( Int iLevel );
  ErrVal xWriteLevelVLCN( Int iLevel, UInt uiVlcLength );
  ErrVal xWriteRun( UInt uiVlcPos, UInt uiRun );
  ErrVal xWriteRunLevel( Int* aiLevelRun, UInt uiCoeffCnt, UInt uiTrailingOnes, UInt uiMaxCoeffs, UInt uiTotalRun );
  ErrVal xWriteRefFrame( Bool bWriteBit, UInt uiRefFrame );
  ErrVal xWriteMotionPredFlag( Bool bFlag );

  ErrVal xWriteUvlcCode( UInt uiVal);
  ErrVal xWriteSvlcCode( Int iVal);

  UInt xConvertToUInt( Int iValue )  {  return ( iValue <= 0) ? -iValue<<1 : (iValue<<1)-1; }

  Bool    RQencodeCBP_8x8( MbDataAccess& rcMbDataAccess, MbDataAccess& rcMbDataAccessBase, B8x8Idx c8x8Idx );
  Bool    RQencodeBCBP_4x4( MbDataAccess& rcMbDataAccess, MbDataAccess& rcMbDataAccessBase, LumaIdx cIdx );
  Bool    RQencodeCBP_Chroma( MbDataAccess& rcMbDataAccess, MbDataAccess& rcMbDataAccessBase );
  Bool    RQencodeBCBP_ChromaAC( MbDataAccess&  rcMbDataAccess, MbDataAccess&  rcMbDataAccessBase, ChromaIdx cIdx );
  Bool    RQencodeBCBP_ChromaDC( MbDataAccess&   rcMbDataAccess, MbDataAccess&   rcMbDataAccessBase, ChromaIdx cIdx );
  Bool    RQencodeCBP_ChromaAC( MbDataAccess& rcMbDataAccess, MbDataAccess& rcMbDataAccessBase );
  ErrVal  RQencodeDeltaQp( MbDataAccess& rcMbDataAccess );
  ErrVal  RQencode8x8Flag( MbDataAccess& rcMbDataAccess, MbDataAccess& rcMbDataAccessBase );
  ErrVal  RQencodeNewTCoeff_8x8( MbDataAccess&   rcMbDataAccess,
                                      MbDataAccess&   rcMbDataAccessBase,
                                      B8x8Idx         c8x8Idx,
                                      UInt            uiScanIndex,
                                      UInt&           ruiLast );
  ErrVal RQencodeNewTCoeff_Luma ( MbDataAccess&   rcMbDataAccess,
                                        MbDataAccess&   rcMbDataAccessBase,
                                        ResidualMode    eResidualMode,
                                        LumaIdx         cIdx,
                                        UInt            uiScanIndex,
                                        Bool&           rbLast,
                                        UInt&           ruiNumCoefWritten );
  ErrVal RQencodeNewTCoeff_Chroma ( MbDataAccess&   rcMbDataAccess,
                                          MbDataAccess&   rcMbDataAccessBase,
                                          ResidualMode    eResidualMode,
                                          ChromaIdx       cIdx,
                                          UInt            uiScanIndex,
                                          Bool&           rbLast,
                                          UInt&           ruiNumCoefWritten );
  ErrVal RQencodeTCoeffRef_8x8( MbDataAccess&   rcMbDataAccess,
                                      MbDataAccess&   rcMbDataAccessBase,
                                      B8x8Idx         c8x8Idx,
                                      UInt            uiScanIndex );
  ErrVal RQencodeTCoeffRef_Luma ( MbDataAccess&   rcMbDataAccess,
                                        MbDataAccess&   rcMbDataAccessBase,
                                        LumaIdx         cIdx,
                                        UInt            uiScanIndex );
  ErrVal RQencodeTCoeffRef_Chroma ( MbDataAccess&   rcMbDataAccess,
                                        MbDataAccess&   rcMbDataAccessBase,
                                        ResidualMode    eResidualMode,
                                        ChromaIdx       cIdx,
                                        UInt            uiScanIndex );
  ErrVal RQencodeCycleSymbol( UInt uiCycle );
  ErrVal RQencodeTermBit ( UInt uiIsLast ) { return Err::m_nOK;}
  Bool   RQpeekCbp4x4(MbDataAccess& rcMbDataAccess, MbDataAccess&  rcMbDataAccessBase, LumaIdx cIdx);
  ErrVal RQencodeEobOffsets_Luma ( UInt* auiSeq );
  ErrVal RQencodeEobOffsets_Chroma( UInt* auiSeq );
  ErrVal RQencodeBestCodeTableMap( UInt* auiTable, UInt uiMaxH );
  ErrVal RQupdateVlcTable         ();
  ErrVal RQvlcFlush               ();
  static UInt   peekGolomb(UInt uiSymbol, UInt uiK);
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
};

class UcSymGrpWriter
{
public:
  UcSymGrpWriter( UvlcWriter* pParent );
  ErrVal Init();
  ErrVal Flush();
  ErrVal Write( UChar ucBit );
  Bool   UpdateVlc();

protected:
  UvlcWriter* m_pParent;
  UInt m_auiSymCount[3];
  UInt m_uiTable;
  UInt m_uiCodedFlag;
  UInt m_uiCode;
  UInt m_uiLen;
};


H264AVC_NAMESPACE_END


#endif // !defined(AFX_UVLCWRITER_H__EA98D347_89D5_4D2D_B6D5_FB3A374CD295__INCLUDED_)
