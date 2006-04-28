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



#if !defined(AFX_CABACREADER_H__06F9800B_44E9_4FB9_9BBC_BF5E02AFBBB3__INCLUDED_)
#define AFX_CABACREADER_H__06F9800B_44E9_4FB9_9BBC_BF5E02AFBBB3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MbSymbolReadIf.h"
#include "CabaDecoder.h"
#include "H264AVCCommonLib/CabacContextModel2DBuffer.h"
#include "H264AVCCommonLib/Quantizer.h"


H264AVC_NAMESPACE_BEGIN


class CabacReader :
public MbSymbolReadIf
, private CabaDecoder
, public Quantizer

{
protected:
  CabacReader();
  virtual ~CabacReader();

public:
  static ErrVal create        ( CabacReader*& rpcCabacReader );
  ErrVal        destroy       ();

  ErrVal  startSlice          ( const SliceHeader& rcSliceHeader );
  ErrVal  finishSlice         ( const SliceHeader& rcSliceHeader ) { return Err::m_nOK; }

  ErrVal  init                ( BitReadBuffer* pcBitReadBuffer );
  ErrVal  uninit              ();

  Bool    RQdecodeBCBP_4x4        ( MbDataAccess&   rcMbDataAccess,
                                    MbDataAccess&   rcMbDataAccessBase,
                                    LumaIdx         cIdx );
  Bool    RQdecodeBCBP_ChromaDC   ( MbDataAccess&   rcMbDataAccess,
                                    MbDataAccess&   rcMbDataAccessBase,
                                    ChromaIdx       cIdx );
  Bool    RQdecodeBCBP_ChromaAC   ( MbDataAccess&   rcMbDataAccess,
                                    MbDataAccess&   rcMbDataAccessBase,
                                    ChromaIdx       cIdx );
  Bool    RQdecodeCBP_Chroma      ( MbDataAccess&   rcMbDataAccess,
                                    MbDataAccess&   rcMbDataAccessBase );
  Bool    RQdecodeCBP_ChromaAC    ( MbDataAccess&   rcMbDataAccess,
                                    MbDataAccess&   rcMbDataAccessBase );
  Bool    RQdecodeCBP_8x8         ( MbDataAccess&   rcMbDataAccess,
                                    MbDataAccess&   rcMbDataAccessBase,
                                    B8x8Idx         c8x8Idx );
  ErrVal  RQdecodeDeltaQp         ( MbDataAccess&   rcMbDataAccess,
                                    MbDataAccess&   rcMbDataAccessBase );
  ErrVal  RQdecode8x8Flag         ( MbDataAccess&   rcMbDataAccess,
                                    MbDataAccess&   rcMbDataAccessBase );
  ErrVal  RQdecodeTermBit         ( UInt&           ruiBit );



  ErrVal  RQdecodeNewTCoeff_8x8    ( MbDataAccess&   rcMbDataAccess,
                                     MbDataAccess&   rcMbDataAccessBase,
                                     B8x8Idx         c8x8Idx,
                                     UInt            uiScanIndex,
                                     Bool&           rbLast );
  ErrVal  RQdecodeTCoeffRef_8x8    ( MbDataAccess&   rcMbDataAccess,
                                     MbDataAccess&   rcMbDataAccessBase,
                                     B8x8Idx         c8x8Idx,
                                     UInt            uiScanIndex );
  ErrVal  RQdecodeNewTCoeff_Luma   ( MbDataAccess&   rcMbDataAccess,
                                     MbDataAccess&   rcMbDataAccessBase,
                                     ResidualMode    eResidualMode,
                                     LumaIdx         cIdx,
                                     UInt            uiScanIndex,
                                     Bool&           rbLast,
                                     UInt&           ruiNumCoefRead );
  ErrVal  RQdecodeTCoeffRef_Luma   ( MbDataAccess&   rcMbDataAccess,
                                     MbDataAccess&   rcMbDataAccessBase,
                                     ResidualMode    eResidualMode,
                                     LumaIdx         cIdx,
                                     UInt            uiScanIndex );
  ErrVal  RQdecodeNewTCoeff_Chroma ( MbDataAccess&   rcMbDataAccess,
                                     MbDataAccess&   rcMbDataAccessBase,
                                     ResidualMode    eResidualMode,
                                     ChromaIdx       cIdx,
                                     UInt            uiScanIndex,
                                     Bool&           rbLast,
                                     UInt&           ruiNumCoefRead );
  ErrVal  RQdecodeTCoeffRef_Chroma ( MbDataAccess&   rcMbDataAccess,
                                     MbDataAccess&   rcMbDataAccessBase,
                                     ResidualMode    eResidualMode,
                                     ChromaIdx       cIdx,
                                     UInt            uiScanIndex );
  ErrVal  RQdecodeCycleSymbol      ( UInt&           uiCycle );
  ErrVal  RQdecodeEobOffsets_Luma  () { return Err::m_nOK; };
  ErrVal  RQdecodeEobOffsets_Chroma() { return Err::m_nOK; };
  ErrVal  RQdecodeVlcTableMap      ( UInt            uiMaxH,
                                     UInt            uiMaxV ) { return Err::m_nOK; };
  ErrVal  RQupdateVlcTable         () { return Err::m_nOK; };
  ErrVal  RQvlcFlush               () { return Err::m_nOK; };
  Bool    RQpeekCbp4x4(MbDataAccess& rcMbDataAccess, MbDataAccess&  rcMbDataAccessBase, LumaIdx cIdx);
  
  ErrVal  blFlag              ( MbDataAccess& rcMbDataAccess );
  Bool    isEndOfSlice        ();
  Bool    isMbSkipped         ( MbDataAccess& rcMbDataAccess );
  Bool    isBLSkipped         ( MbDataAccess& rcMbDataAccess );
  Bool    isBLQRef            ( MbDataAccess& rcMbDataAccess );
  ErrVal  blockModes          ( MbDataAccess& rcMbDataAccess );
  ErrVal  mbMode              ( MbDataAccess& rcMbDataAccess );
  ErrVal  resPredFlag         ( MbDataAccess& rcMbDataAccess );
  ErrVal  smoothedRefFlag     ( MbDataAccess& rcMbDataAccess );	// JVT-R091

  ErrVal  mvdQPel             ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx                      );
  ErrVal  mvdQPel             ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  );
  ErrVal  mvdQPel             ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  );
  ErrVal  mvdQPel             ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  );

  ErrVal  mvd                 ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx );
  ErrVal  mvd                 ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  );
  ErrVal  mvd                 ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  );
  ErrVal  mvd                 ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  );
  ErrVal  mvd                 ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx8x4 eSParIdx );
  ErrVal  mvd                 ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx4x8 eSParIdx );
  ErrVal  mvd                 ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx4x4 eSParIdx );
  ErrVal  cbp                 ( MbDataAccess& rcMbDataAccess );
  ErrVal  refFrame            ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx );
  ErrVal  refFrame            ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  );
  ErrVal  refFrame            ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  );
  ErrVal  refFrame            ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  );

  ErrVal  motionPredFlag      ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx );
  ErrVal  motionPredFlag      ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  );
  ErrVal  motionPredFlag      ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  );
  ErrVal  motionPredFlag      ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  );

  ErrVal  residualBlock       ( MbDataAccess& rcMbDataAccess, LumaIdx   cIdx, ResidualMode eResidualMode, UInt& ruiMbExtCbp );
  ErrVal  residualBlock       ( MbDataAccess& rcMbDataAccess, ChromaIdx cIdx, ResidualMode eResidualMode );
  
  ErrVal  deltaQp             ( MbDataAccess& rcMbDataAccess );
  ErrVal  intraPredModeLuma   ( MbDataAccess& rcMbDataAccess, LumaIdx cIdx );
  ErrVal  intraPredModeChroma ( MbDataAccess& rcMbDataAccess );
  ErrVal  samplesPCM          ( MbDataAccess& rcMbDataAccess );

  ErrVal  residualBlock8x8    ( MbDataAccess& rcMbDataAccess, B8x8Idx cIdx, ResidualMode eResidualMode, UInt& ruiMbExtCbp);
  ErrVal  intraPredModeLuma8x8( MbDataAccess& rcMbDataAccess, B8x8Idx cIdx );
  ErrVal  transformSize8x8Flag( MbDataAccess& rcMbDataAccess);

protected:
  ErrVal  xRQdecodeNewTCoeffs ( TCoeff*       piCoeff,
                                TCoeff*       piCoeffBase,
                                UInt          uiStart,
                                UInt          uiStop,
                                ResidualMode  eResidualMode,
                                const UChar*  pucScan,
                                UInt          uiScanIndex,
                                Bool&         rbLast,
                                UInt&         ruiNumCoefRead );
  ErrVal  xRQdecodeTCoeffsRef ( TCoeff*       piCoeff,
                                TCoeff*       piCoeffBase,
                                const UChar*  pucScan,
                                UInt          uiScanIndex );

  ErrVal xGetMvdQPel( MbDataAccess& rcMbDataAccess, Mv& rcMv, LumaIdx cIdx, ListIdx eLstIdx );
  ErrVal xGetMvdComponentQPel( Short& rsMvdComp, UInt uiAbsSum, UInt uiCtx );
  ErrVal xGetMvd( MbDataAccess& rcMbDataAccess, Mv& rcMv, LumaIdx cIdx, ListIdx eLstIdx );

  ErrVal xInitContextModels( const SliceHeader& rcSliceHeader );
  ErrVal xGetMvdComponent( Short& rsMvdComp, UInt uiAbsSum, UInt uiCtx );
  ErrVal xRefFrame      ( MbDataAccess& rcMbDataAccess, UInt& ruiRefFrame, ListIdx eLstIdx, ParIdx8x8 eParIdx );
  ErrVal xMotionPredFlag( MbDataAccess& rcMbDataAccess, Bool& bFlag,       ListIdx eLstIdx, ParIdx8x8 eParIdx );

  ErrVal xReadBCbp( MbDataAccess& rcMbDataAccess, Bool& rbCoded, ResidualMode eResidualMode, LumaIdx cIdx );
  ErrVal xReadBCbp( MbDataAccess& rcMbDataAccess, Bool& rbCoded, ResidualMode eResidualMode, ChromaIdx cIdx );

  ErrVal xReadCoeff( TCoeff*        piCoeff,
                     ResidualMode   eResidualMode,
                     const UChar*   pucScan
                     );

protected:
  CabacContextModel2DBuffer m_cBCbpCCModel;
  CabacContextModel2DBuffer m_cMapCCModel;
  CabacContextModel2DBuffer m_cLastCCModel;
  CabacContextModel2DBuffer m_cRefCCModel;
  CabacContextModel2DBuffer m_cSigCCModel;
  CabacContextModel2DBuffer m_cOneCCModel;
  CabacContextModel2DBuffer m_cAbsCCModel;
  CabacContextModel2DBuffer m_cChromaPredCCModel;
  CabacContextModel2DBuffer m_cBLFlagCCModel;
  CabacContextModel2DBuffer m_cBLSkipCCModel;
  CabacContextModel2DBuffer m_cBLQRefCCModel;
  CabacContextModel2DBuffer m_cMbTypeCCModel;
  CabacContextModel2DBuffer m_cBlockTypeCCModel;
  CabacContextModel2DBuffer m_cMvdCCModel;
  CabacContextModel2DBuffer m_cRefPicCCModel;
  CabacContextModel2DBuffer m_cBLPredFlagCCModel;
  CabacContextModel2DBuffer m_cResPredFlagCCModel;
  CabacContextModel2DBuffer m_cDeltaQpCCModel;
  CabacContextModel2DBuffer m_cIntraPredCCModel;
  CabacContextModel2DBuffer m_cCbpCCModel;
  CabacContextModel2DBuffer m_cBCbpEnhanceCCModel;
  CabacContextModel2DBuffer m_cCbpEnhanceCCModel;
  CabacContextModel2DBuffer m_cTransSizeCCModel;
	CabacContextModel2DBuffer m_cSRFlagCCModel;	// JVT-R091

  UInt m_uiBitCounter;
  UInt m_uiPosCounter;
  UInt m_uiLastDQpNonZero;
};

H264AVC_NAMESPACE_END


#endif // !defined(AFX_CABACREADER_H__06F9800B_44E9_4FB9_9BBC_BF5E02AFBBB3__INCLUDED_)
