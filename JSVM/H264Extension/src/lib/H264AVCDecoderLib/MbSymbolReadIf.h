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




#if !defined(AFX_MBSYMBOLREADIF_H__E5736EAC_0841_4E22_A1F0_ACAD8A7E5490__INCLUDED_)
#define AFX_MBSYMBOLREADIF_H__E5736EAC_0841_4E22_A1F0_ACAD8A7E5490__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

H264AVC_NAMESPACE_BEGIN

enum ResidualMode
{
  LUMA_I16_DC  = 0,
  LUMA_I16_AC     ,
  LUMA_SCAN       ,
  CHROMA_DC       ,
  CHROMA_AC       
  , LUMA_8X8
};


class MbSymbolReadIf
{
protected:
  MbSymbolReadIf() {}
	virtual ~MbSymbolReadIf() {}

public:
  virtual Bool    isMbSkipped ( MbDataAccess& rcMbDataAccess ) = 0;
  virtual Bool    isBLSkipped ( MbDataAccess& rcMbDataAccess ) = 0;
  virtual Bool    isEndOfSlice() = 0;
  virtual ErrVal  blockModes  ( MbDataAccess& rcMbDataAccess ) = 0;
  virtual ErrVal  mbMode      ( MbDataAccess& rcMbDataAccess ) = 0;
  virtual ErrVal  resPredFlag ( MbDataAccess& rcMbDataAccess ) = 0;
  virtual ErrVal  resPredFlag_FGS ( MbDataAccess& rcMbDataAccess, Bool bBaseCoeff ) = 0;
	virtual ErrVal  smoothedRefFlag ( MbDataAccess& rcMbDataAccess ) = 0;	// JVT-R091

  virtual ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx ) = 0;
  virtual ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  ) = 0;
  virtual ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  ) = 0;
  virtual ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  ) = 0;
  virtual ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx8x4 eSParIdx ) = 0;
  virtual ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx4x8 eSParIdx ) = 0;
  virtual ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx4x4 eSParIdx ) = 0;

  virtual ErrVal  cbp( MbDataAccess& rcMbDataAccess ) = 0;
  virtual ErrVal  refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx ) = 0;
  virtual ErrVal  refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  ) = 0;
  virtual ErrVal  refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  ) = 0;
  virtual ErrVal  refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  ) = 0;

  virtual ErrVal  motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx ) = 0;
  virtual ErrVal  motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  ) = 0;
  virtual ErrVal  motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  ) = 0;
  virtual ErrVal  motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  ) = 0;

  virtual ErrVal  residualBlock( MbDataAccess& rcMbDataAccess, LumaIdx   cIdx, ResidualMode eResidualMode, UInt& ruiMbExtCbp) = 0;
  virtual ErrVal  residualBlock( MbDataAccess& rcMbDataAccess, ChromaIdx cIdx, ResidualMode eResidualMode ) = 0;

  virtual ErrVal  deltaQp             ( MbDataAccess& rcMbDataAccess ) = 0;
  virtual ErrVal  intraPredModeLuma   ( MbDataAccess& rcMbDataAccess, LumaIdx cIdx ) = 0;
  virtual ErrVal  intraPredModeChroma ( MbDataAccess& rcMbDataAccess ) = 0;
  virtual ErrVal  samplesPCM          ( MbDataAccess& rcMbDataAccess ) = 0;

  virtual ErrVal  startSlice          ( const SliceHeader& rcSliceHeader ) = 0;
  virtual ErrVal  finishSlice         ( ) = 0;

  virtual ErrVal  transformSize8x8Flag( MbDataAccess& rcMbDataAccess) = 0;
  virtual ErrVal  residualBlock8x8    ( MbDataAccess& rcMbDataAccess, B8x8Idx cIdx ) = 0;
  virtual ErrVal  intraPredModeLuma8x8( MbDataAccess& rcMbDataAccess, B8x8Idx cIdx ) = 0;
  virtual ErrVal  RQdecodeCycleSymbol ( UInt& uiCycle ) = 0;
  virtual ErrVal  RQdecodeDeltaQp     ( MbDataAccess&   rcMbDataAccess ) = 0;
  virtual ErrVal  RQdecode8x8Flag     ( MbDataAccess&   rcMbDataAccess,
                                        MbDataAccess&   rcMbDataAccessBase ) = 0;
  virtual Bool    RQdecodeBCBP_4x4     ( MbDataAccess&   rcMbDataAccessBase,
                                         LumaIdx         cIdx ) = 0;
  virtual Bool    RQdecodeBCBP_ChromaDC( MbDataAccess&   rcMbDataAccessBase,
                                         ChromaIdx       cIdx ) = 0;
  virtual Bool    RQdecodeBCBP_ChromaAC( MbDataAccess&   rcMbDataAccessBase,
                                         ChromaIdx       cIdx ) = 0;
  virtual Bool    RQdecodeCBP_Chroma   ( MbDataAccess&   rcMbDataAccess,
                                         MbDataAccess&   rcMbDataAccessBase ) = 0;
  virtual Bool    RQdecodeCBP_ChromaAC ( MbDataAccess&   rcMbDataAccess,
                                         MbDataAccess&   rcMbDataAccessBase ) = 0;
  virtual Bool    RQdecodeCBP_8x8      ( MbDataAccess&   rcMbDataAccess,
                                         MbDataAccess&   rcMbDataAccessBase,
                                         B8x8Idx         c8x8Idx ) = 0;
  virtual ErrVal  RQdecodeTermBit      ( UInt&           ruiBit ) = 0;
  virtual ErrVal  RQdecodeNewTCoeff_8x8    ( MbDataAccess&   rcMbDataAccess,
                                             MbDataAccess&   rcMbDataAccessBase,
                                             B8x8Idx         c8x8Idx,
                                             UInt            uiScanIndex,
                                             Bool&           rbLast,
                                             UInt&           ruiNumCoefRead ) = 0;
  virtual ErrVal  RQeo8b( Bool& bEob ) = 0;
  virtual ErrVal  RQdecodeTCoeffRef_8x8    ( MbDataAccess&   rcMbDataAccess,
                                             MbDataAccess&   rcMbDataAccessBase,
                                             B8x8Idx         c8x8Idx,
                                             UInt            uiScanIndex ) = 0;
  virtual ErrVal  RQdecodeNewTCoeff_Luma   ( MbDataAccess&   rcMbDataAccess,
                                             MbDataAccess&   rcMbDataAccessBase,
                                             ResidualMode    eResidualMode,
                                             LumaIdx         cIdx,
                                             UInt            uiScanIndex,
                                             Bool&           rbLast,
                                             UInt&           ruiNumCoefRead ) = 0;
  virtual ErrVal  RQdecodeTCoeffRef_Luma   ( MbDataAccess&   rcMbDataAccess,
                                             MbDataAccess&   rcMbDataAccessBase,
                                             LumaIdx         cIdx,
                                             UInt            uiScanIndex ) = 0;
  virtual ErrVal  RQdecodeNewTCoeff_Chroma ( MbDataAccess&   rcMbDataAccess,
                                             MbDataAccess&   rcMbDataAccessBase,
                                             ResidualMode    eResidualMode,
                                             ChromaIdx       cIdx,
                                             UInt            uiScanIndex,
                                             Bool&           rbLast,
                                             UInt&           ruiNumCoefRead ) = 0;
  virtual ErrVal  RQdecodeTCoeffRef_Chroma ( MbDataAccess&   rcMbDataAccess,
                                             MbDataAccess&   rcMbDataAccessBase,
                                             ResidualMode    eResidualMode,
                                             ChromaIdx       cIdx,
                                             UInt            uiScanIndex ) = 0;
  virtual ErrVal  RQdecodeEobOffsets_Luma  () = 0;
  virtual ErrVal  RQdecodeEobOffsets_Chroma() = 0;
  virtual ErrVal  RQdecodeBestCodeTableMap ( UInt            uiMaxH ) = 0;
  virtual Bool RQpeekCbp4x4( MbDataAccess& rcMbDataAccessBase, LumaIdx cIdx) = 0;
  virtual ErrVal  RQupdateVlcTable         () = 0;
  virtual ErrVal  RQvlcFlush               () = 0;
};


H264AVC_NAMESPACE_END


#endif // !defined(AFX_MBSYMBOLREADIF_H__E5736EAC_0841_4E22_A1F0_ACAD8A7E5490__INCLUDED_)
