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




#if !defined(AFX_FGSSUBBANDENCODER_H__02F9A3A4_BE1C_4D88_86D9_98AF451F04CD__INCLUDED_)
#define AFX_FGSSUBBANDENCODER_H__02F9A3A4_BE1C_4D88_86D9_98AF451F04CD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "MbSymbolWriteIf.h"
#include "ControlMngH264AVCEncoder.h"
#include "H264AVCCommonLib/MbDataCtrl.h"
#include "H264AVCCommonLib/Transform.h"

#include "H264AVCCommonLib/FGSCoder.h"


H264AVC_NAMESPACE_BEGIN


class YuvBufferCtrl;
class Transform;
class MbDataCtrl;
class MbEncoder;



#define TWSOT(x)  { if( x) { xThrowWriteStop(); } }
#define TWSOF(x)  { if(!x) { xThrowWriteStop(); } }


class RQFGSEncoder  
  : public FGSCoder
{
private:
  class WriteStop
  {
  };

  __inline Void xThrowWriteStop()
  {
    throw WriteStop();
  }

protected:
	RQFGSEncoder         ();
	virtual ~RQFGSEncoder();

public:
  static ErrVal     create                ( RQFGSEncoder*&              rpcFGSSubbandEncoder );
  ErrVal            destroy               ();

  ErrVal            init                  ( YuvBufferCtrl**             apcYuvFullPelBufferCtrl,
                                            YuvBufferCtrl**             apcYuvHalfPelBufferCtrl,
                                            QuarterPelFilter*           pcQuarterPelFilter,
                                            MotionEstimation*           pcMotionEstimation,
                                            MbCoder*                    pcMbCoder,
                                            Transform*                  pcTransform,
                                            ControlMngH264AVCEncoder*   pcControlMng,
                                            MbEncoder*                  pcMbEncoder );
  ErrVal            uninit                ();
  ErrVal            initSPS               ( const SequenceParameterSet& rcSPS );


  ErrVal            initPicture           ( SliceHeader*                pcSliceHeader,
                                            MbDataCtrl*                 pcCurrMbDataCtrl,
                                            IntFrame*                   pcOrgResidual,
                                            IntFrame*                   pcOrgFrame,
                                            IntFrame*                   pcPredSignal,
                                            RefFrameList*               pcRefFrameList0,
                                            RefFrameList*               pcRefFrameList1,
                                            UInt                        uiNumMaxIter,
                                            UInt                        uiIterSearchRange,
                                            Double                      dNumFGSLayers,
                                            Double                      dLambda,
                                            Int                         iMaxQpDelta,
                                            Bool&                       rbFinished,
                                            Bool                        bTruncate,
                                            Bool                        bUseDiscardable); //JVT-P031
  ErrVal            initArFgs             ( IntFrame*                   pcPredSignal,
                                            RefFrameList*               pcRefFrameListDiff );
  ErrVal            encodeNextLayer       ( Bool&                       rbFinished,
                                            Bool&                       rbCorrupted,
                                            UInt                        uiMaxBits,
                                            UInt*                       puiPDFragBits,
                                            UInt&                       ruiNumPDFrags,
                                            UInt                        uiFrac,
                                            Bool                        bFragmented,
                                            FILE*                       pFile ); //JVT-P031
 
  ErrVal            finishPicture         ();
  
  ErrVal            setNewOriginalResidual( IntFrame                    *pcNewOriginalResidual )
  {
    m_pcOrgResidual = pcNewOriginalResidual;

    return Err::m_nOK;
  }

  //-- ICU/ETRI FMO Implementation 1206
  ErrVal            updateQP(Bool&  rbCorrupted, Bool& rbFinished, Bool bFragmented, UInt uiFrac, Bool isLastSlice);
  ErrVal            setSliceGroup(Int iSliceGroupID);
  ErrVal            prepareEncode(UInt uiFrac, UInt uiFracNb);
  
private:
  ErrVal            xVLCParseLuma         ( UInt   uiBlockYIndex,
                                            UInt   uiBlockXIndex,
                                            UInt*  pauiNumCoefHist,
                                            UInt*  pauiHighMagHist,
                                            Bool   bFrame);
  ErrVal            xVLCParseChromaAC     ( UInt   uiPlane,
                                            UInt   uiBlockYIndex,
                                            UInt   uiBlockXIndex,
                                            UInt*  pauiNumCoefHist,
                                            Bool   bFrame);

  ErrVal            xSetSymbolsChroma     ( TCoeff*                     piCoeff,
                                            UInt                        uiMbX,
                                            UInt                        uiMbY,
                                            UInt&                       uiCoeffCostDC,
                                            UInt&                       uiCoeffCostAC,
                                            Bool&                       bSigDC,
                                            Bool&                       bSigAC,
                                            ChromaIdx                   cIdx,
                                            Bool                        bFrame );
  ErrVal            xSetSymbols4x4        ( TCoeff*                     piCoeff,
                                            UInt                        uiMbX,
                                            UInt                        uiMbY,
                                            UInt&                       uiCoeffCost,
                                            UInt&                       ruiCbp,
                                            const S4x4Idx&              cIdx,
                                            UInt                        uiStart,
                                            Bool                        bFrame );
  ErrVal            xSetSymbols8x8        ( TCoeff*                     piCoeff,
                                            UInt                        uiMbX,
                                            UInt                        uiMbY,
                                            UInt&                       uiCoeffCost,
                                            UInt&                       ruiCbp,
                                            B8x8Idx                     cIdx,
                                            Bool                        bFrame );


  ErrVal            xRequantizeMacroblock ( MbDataAccess&               rcMbDataAccess,
                                            MbDataAccess&               rcMbDataAccessBL );
  ErrVal            xResidualTransform    ();

  ErrVal            xMotionEstimation     ();
  ErrVal            xEncodeMotionData             ( UInt uiMbYIdx,
                                                    UInt uiMbXIdx );
  ErrVal            xEncodeLumaCbpVlcStart( UInt&                       uiLumaNextMbX,
                                            UInt&                       uiLumaNextMbY,
                                            UInt&                       uiNext8x8Idx,
                                            UInt                        uiLastMbX,
                                            UInt                        uiLastMbY,
                                            UInt&                       ruiLumaCbpBitCount);

  ErrVal            xEncodeLumaCbpVlc     ( UInt                        uiCurrMbIdxX,
                                            UInt                        uiCurrMbIdxY,
                                            UInt&                       uiNextMbX,
                                            UInt&                       uiNextMbY,
                                            UInt&                       uiNext8x8Idx,
                                            UInt                        uiLastMbX,
                                            UInt                        uiLastMbY,
                                            UInt&                       ruiCbpBitCount);

  ErrVal            xEncodeChromaCbpVlcStart( UInt                      uiCurrMbIdxX,
                                              UInt                      uiCurrMbIdxY,
                                              UInt&                     ruiChromaCbpBitCount);

  ErrVal            xEncodeChromaCbpVlc   ( UInt                        uiCurrMbIdxX,
                                            UInt                        uiCurrMbIdxY,
                                            UInt&                       uiChromaCbpNextMbX,
                                            UInt&                       uiChromaCbpNextMbY,
                                            UInt                        uiLastMbX,
                                            UInt                        uiLastMbY,
                                            UInt&                       ruiChromaCbpBitCount);

  ErrVal            xEncodingFGS                  ( Bool&               rbFinished, 
                                                    Bool&               rbCorrupted, 
                                                    UInt                uiMaxBits, 
                                                    UInt*               puiPDFragBits,
                                                    UInt&               ruiNumPDFrags,
                                                    UInt                uiFracNb, 
                                                    FILE*               pFile ); //JVT-P031
  ErrVal            xEncodeNewCoefficientLuma     ( MbDataAccess        *pcMbDataAccessBL,
                                                   MbDataAccess        *pcMbDataAccessEL,
                                                   MbFGSCoefMap        &rcMbFGSCoefMap,
                                                   const S4x4Idx       &rcIdx,
                                                    Bool                bFrame  );
  ErrVal            xEncodeNewCoefficientChromaDC ( MbDataAccess       *pcMbDataAccessBL,
                                                    MbDataAccess       *pcMbDataAccessEL,
                                                    MbFGSCoefMap       &rcMbFGSCoefMap,
                                                    const CPlaneIdx    &rcCPlaneIdx,
                                                    Int&                riLastQp,
                                                    UInt                uiChromaScanIndex );
  ErrVal            xEncodeNewCoefficientChromaAC ( MbDataAccess       *pcMbDataAccessBL,
                                                    MbDataAccess       *pcMbDataAccessEL,
                                                    MbFGSCoefMap       &rcMbFGSCoefMap,
                                                    const CIdx         &rcCIdx,
                                                    Int&                riLastQp,
                                                    UInt                uiChromaScanIndex,
                                                    Bool                bFrame  );// TMM_INTERLACE
  
  ErrVal            xEncodeCoefficientLumaRef     ( MbDataAccess       *pcMbDataAccessBL,
                                                    MbDataAccess       *pcMbDataAccessEL,
                                                    MbFGSCoefMap       &rcMbFGSCoefMap,
                                                    const S4x4Idx      &rcIdx,
                                                    UInt                uiScanIndex);
  ErrVal            xEncodeCoefficientChromaDCRef ( MbDataAccess       *pcMbDataAccessBL,
                                                    MbDataAccess       *pcMbDataAccessEL,
                                                    MbFGSCoefMap       &rcMbFGSCoefMap,
                                                    const CPlaneIdx    &rcCPlaneIdx,
                                                    UInt                uiDCIdx );
  ErrVal            xEncodeCoefficientChromaACRef ( MbDataAccess       *pcMbDataAccessBL,
                                                    MbDataAccess       *pcMbDataAccessEL,
                                                    MbFGSCoefMap       &rcMbFGSCoefMap,
                                                    const CIdx         &rcCIdx,
                                                    UInt                uiScanIdx );
  ErrVal            xPrescanCoefficientLumaRef    ( UInt                uiBlockYIndex,
                                                    UInt                uiBlockXIndex,
                                                    UInt                uiScanIndex ,
                                                    Bool                bFrame  );// TMM_INTERLACE
  ErrVal            xPrescanCoefficientChromaDCRef( UInt                uiPlane,
                                                    UInt                uiMbYIdx,
                                                    UInt                uiMbXIdx,
                                                    UInt                uiDCIdx );
  ErrVal            xPrescanCoefficientChromaACRef( UInt                uiPlane,
                                                    UInt                uiB8YIdx,
                                                    UInt                uiB8XIdx,
                                                    UInt                uiScanIdx );

  //JVT-P031
ErrVal            xStoreFGSState(UInt iLumaScanIdx,
                             UInt iChromaDCScanIdx,
                             UInt iChromaACScanIdx,
                             UInt iStartCycle,
                             UInt iCycle,
                             UInt uiPass,
                             UInt uiFragIdx,
                             UInt uiMbYIdx,
                             UInt uiMbXIdx,
                             B8x8Idx c8x8Idx,
                             S4x4Idx cIdx,
                             UInt iLastBitsLuma,
                             UInt uiBitsLast,
                             UInt uiFGSPart,
                             CPlaneIdx cCPlaneIdx,
                             Int iLastQP);

  ErrVal            xRestoreFGSState(UInt& riLumaScanIdx,
                             UInt& riChromaDCScanIdx,
                             UInt& riChromaACScanIdx,
                             UInt& riStartCycle,
                             UInt& riCycle,
                             UInt& ruiPass,
                             UInt& ruiFragIdx,
//                              UInt& ruiMbYIdx,
//                              UInt& ruiMbXIdx,
//                              UInt& ruiB8YIdx,
//                              UInt& ruiB8XIdx,
//                              UInt& ruiBlockYIdx,
//                              UInt& ruiBlockXIdx,
                             UInt& riLastBitsLuma,
//                              UInt& ruiBitsLast,
//                              UInt& ruiFGSPart,
//                              UInt& ruiPlane,
                             Int& riLastQP);
  ErrVal        xSaveCodingPath();
  ErrVal        xRestoreCodingPath();
  //~JVT-P031
  ErrVal            xEncodeMbHeader               ( MbDataAccess*       pcMbDataAccessBL,
                                                    MbDataAccess*       pcMbDataAccessEL,
                                                    MbFGSCoefMap        &rcMbFGSCoefMap,
                                                    Int&                riLastQp );

private:
  MbSymbolWriteIf*  m_pcSymbolWriter;
  ControlMngH264AVCEncoder* m_pcControlMng;
  MbEncoder*        m_pcMbEncoder;
  Bool              m_bChromaCbpTransition;

  Int               m_iRemainingTCoeff;
  Double            m_dLambda;
  Int               m_iMaxQpDelta;
  SliceType         m_eSliceType;
  UInt              m_uiFirstMbInSlice;
  UInt              m_uiNumMbsInSlice;

  IntFrame*         m_pcOrgResidual;
  UInt              m_auiScanPosVectLuma    [16];
  UInt              m_auiScanPosVectChromaDC[ 4];
  
  Bool              m_bTraceEnable;

  //JVT-P031
  UInt               m_iLumaScanIdx;
  UInt               m_iChromaDCScanIdx;
  UInt               m_iChromaACScanIdx;
  UInt               m_iStartCycle;
  UInt               m_iCycle;
  UInt              m_uiPass;
  UInt              m_uiFragIdx;
  UInt              m_uiMbYIdx;
  UInt              m_uiMbXIdx;
  B8x8Idx           m_c8x8Idx;
  S4x4Idx           m_cIdx;
  CPlaneIdx         m_cCPlaneIdx;
  UInt               m_iLastBitsLuma;
  UInt              m_uiBitsLast;
  UInt              m_uiFGSPart;
  Int               m_iLastQP;
  Bool              m_bUseDiscardableUnit;   
  MbTransformCoeffs ** m_aMyELTransformCoefs;
  MbTransformCoeffs ** m_aMyBLTransformCoefs;
  UInt              * m_auiMbCbpStored;
  UInt              * m_auiBCBPStored;
  Int               * m_aiBLQP;
  Bool              * m_abELtransform8x8;

  UChar             m_ucLastByte;//FIX_FRAG_CAVLC
  UInt              m_uiLastBitPos; //FIX_FRAG_CAVLC

  YuvBufferCtrl**   m_papcYuvHalfPelBufferCtrl;
  QuarterPelFilter* m_pcQuarterPelFilter;
  MotionEstimation* m_pcMotionEstimation;
  MbCoder*          m_pcMbCoder;

  IntFrame*         m_pcOrgFrame;
  IntFrame*         m_pcPredSignal;
  RefFrameList*     m_pcRefFrameList0;
  RefFrameList*     m_pcRefFrameList1;

  IntFrame*         m_pcFGSPredFrame;
  RefFrameList*     m_pcRefFrameListDiff;

  UInt              m_uiNumMaxIter;
  UInt              m_uiIterSearchRange;
  UInt              m_uiLastMbNum;

  UInt              uiLastMbX;
  UInt              uiLastMbY;
  UInt              uiLumaCbpBitCount;
  UInt              uiLumaCbpNextMbX;
  UInt              uiLumaCbpNextMbY;
  UInt              uiLumaCbpNext8x8Idx;
  UInt              uiChromaCbpBitCount;
  UInt              uiChromaCbpNextMbX;
  UInt              uiChromaCbpNextMbY;
};



H264AVC_NAMESPACE_END


#endif // !defined(AFX_FGSSUBBANDENCODER_H__02F9A3A4_BE1C_4D88_86D9_98AF451F04CD__INCLUDED_)
