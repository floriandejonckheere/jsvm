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





#if !defined(AFX_LOOPFILTER_H__1860BB4C_C677_487A_A81F_0BD39DA40284__INCLUDED_)
#define AFX_LOOPFILTER_H__1860BB4C_C677_487A_A81F_0BD39DA40284__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


H264AVC_NAMESPACE_BEGIN

class ControlMngIf;
class YuvPicBuffer;
class Frame;

class ReconstructionBypass;

class H264AVCCOMMONLIB_API LoopFilter
{
  enum LFPass
  {
    FIRST_PASS  = 0,
    SECOND_PASS = 1,
    ONE_PASS    = 1,
    TWO_PASSES  = 2
  };
  enum Dir
  {
    VER = 0,
    HOR = 1
  };
  typedef struct
  {
    UChar ucAlpha;
    UChar aucClip[5];
  } AlphaClip;
  static const UChar      g_aucBetaTab  [52];
  static const AlphaClip  g_acAlphaClip [52];

protected:
	LoopFilter();
	virtual ~LoopFilter();

public:
  static ErrVal create  ( LoopFilter*&           rpcLoopFilter );
  ErrVal        destroy ();
  ErrVal        init    ( ControlMngIf*          pcControlMngIf,
                          ReconstructionBypass*  pcReconstructionBypass,
                          Bool                   bEncoder );
  ErrVal        uninit  ();

  ErrVal        process ( SliceHeader&              rcSH,
                          Frame*                    pcFrame,
                          Frame*                    pcResidual,
                          MbDataCtrl*               pcMbDataCtrl,
                          const DBFilterParameter*  pcInterLayerDBParameter,
                          Bool                      bSpatialScalabilityFlag );

private:
  ErrVal        xFilterMb             ( MbDataAccess&             rcMbDataAccess,
                                        YuvPicBuffer*             pcYuvBuffer,
                                        YuvPicBuffer*             pcResidual,
                                        const DBFilterParameter*  pcInterLayerDBParameter, 
                                        Bool                      bSpatialScalableFlag,
                                        LFPass                    eLFPass );            //VB-JV 04/08

  ErrVal        xRecalcCBP            ( MbDataAccess&             rcMbDataAccess );

  //===== determination of filter strength =====
  UInt          xGetHorFilterStrength ( const MbDataAccess&       rcMbDataAccess,
                                        LumaIdx                   cIdx,
                                        Int                       iFilterIdc,
                                        Bool                      bInterLayerFlag,
                                        Bool                      bSpatialScalableFlag,
                                        LFPass                    eLFPass );
  UInt          xGetVerFilterStrength ( const MbDataAccess&       rcMbDataAccess,
                                        LumaIdx                   cIdx,
                                        Int                       iFilterIdc,
                                        Bool                      bInterLayerFlag,
                                        Bool                      bSpatialScalableFlag,
                                        LFPass                    eLFPass );
  const MbData& xGetMbDataLeft        ( const MbDataAccess&       rcMbDataAccess,
                                        LumaIdx&                  rcIdx );
  const MbData& xGetMbDataAbove       ( const MbDataAccess&       rcMbDataAccess );
  Bool          xFilterInsideEdges    ( const MbDataAccess&       rcMbDataAccess,
                                        Int                       iFilterIdc,
                                        Bool                      bInterLayerFlag,
                                        LFPass                    eLFPass );
  Bool          xFilterLeftEdge       ( const MbDataAccess&       rcMbDataAccess,
                                        Int                       iFilterIdc,
                                        Bool                      bInterLayerFlag,
                                        LFPass                    eLFPass );
  Bool          xFilterTopEdge        ( const MbDataAccess&       rcMbDataAccess,
                                        Int                       iFilterIdc,
                                        Bool                      bInterLayerFlag,
                                        LFPass                    eLFPass );
  UChar         xCheckMvDataP         ( const MbData&             rcQMbData,
                                        const LumaIdx             cQIdx,
                                        const MbData&             rcPMbData,
                                        const LumaIdx             cPIdx,
                                        const Short               sHorMvThr,
                                        const Short               sVerMvThr  );
  UChar         xCheckMvDataB         ( const MbData&             rcQMbData,
                                        const LumaIdx             cQIdx,
                                        const MbData&             rcPMbData,
                                        const LumaIdx             cPIdx,
                                        const Short               sHorMvThr,
                                        const Short               sVerMvThr );

  //===== filtering =====
  ErrVal        xLumaHorFiltering     ( const MbDataAccess&       rcMbDataAccess,
                                        const DBFilterParameter&  rcDFP,
                                        YuvPicBuffer*             pcYuvBuffer );
  ErrVal        xLumaVerFiltering     ( const MbDataAccess&       rcMbDataAccess,
                                        const DBFilterParameter&  rcDFP,
                                        YuvPicBuffer*             pcYuvBuffer );
  ErrVal        xChromaHorFiltering   ( const MbDataAccess&       rcMbDataAccess,
                                        const DBFilterParameter&  rcDFP,
                                        YuvPicBuffer*             pcYuvBuffer );
  ErrVal        xChromaVerFiltering   ( const MbDataAccess&       rcMbDataAccess,
                                        const DBFilterParameter&  rcDFP,
                                        YuvPicBuffer*             pcYuvBuffer );
  Void          xFilter               ( XPel*                     pFlt,
                                        const Int&                iOffset,
                                        const Int&                iIndexA,
                                        const Int&                iIndexB,
                                        const UChar&              ucBs,
                                        const Bool&               bLum );


protected:
  ControlMngIf*           m_pcControlMngIf;
  ReconstructionBypass*   m_pcReconstructionBypass;
  UChar                   m_aaaucBs[2][4][4];
  UChar                   m_aucBsHorTop[4];
  UChar                   m_aucBsVerBot[4];
  Bool                    m_bVerMixedMode;
  Bool                    m_bHorMixedMode;
  Bool                    m_bAddEdge;
  Bool                    m_bEncoder;
};



H264AVC_NAMESPACE_END


#endif // !defined(AFX_LOOPFILTER_H__1860BB4C_C677_487A_A81F_0BD39DA40284__INCLUDED_)
