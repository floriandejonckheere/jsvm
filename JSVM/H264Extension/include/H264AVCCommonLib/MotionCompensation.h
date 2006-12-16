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




#if !defined(AFX_MOTIONCOMPENSATION_H__820D6942_007B_42EA_838B_AC025E866DBE__INCLUDED_)
#define AFX_MOTIONCOMPENSATION_H__820D6942_007B_42EA_838B_AC025E866DBE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "H264AVCCommonLib/MotionVectorCalculation.h"
#include "H264AVCCommonLib/YuvMbBuffer.h"
#include "H264AVCCommonLib/YuvPicBuffer.h"
#include "H264AVCCommonLib/IntYuvMbBuffer.h"
#include "H264AVCCommonLib/IntYuvPicBuffer.h"
#include "H264AVCCommonLib/IntFrame.h"


H264AVC_NAMESPACE_BEGIN


class SampleWeighting;
class QuarterPelFilter;
class FrameMng;
class Frame;

class FGSCoder;
class Transform;

#if defined( WIN32 )
# pragma warning( disable: 4251 )
#endif

class H264AVCCOMMONLIB_API MotionCompensation :
public MotionVectorCalculation
{
protected:

  class MC8x8D
  {
  public:
    MC8x8D( Par8x8 ePar8x8 ) :  m_cIdx( B8x8Idx(ePar8x8) )  { clear(); }
    Void clear()
    {
      m_apcRefBuffer[0] = m_apcRefBuffer[1] = NULL;
      m_apcPW[0]        = m_apcPW[1]        = NULL;
    }

    B4x4Idx         m_cIdx;
    PW              m_acPW[2];
    const PW*       m_apcPW[2];
    YuvPicBuffer*   m_apcRefBuffer[2];
    Mv3D            m_aacMv[2][6];
  };

  class IntMC8x8D
  {
  public:
    IntMC8x8D( Par8x8 ePar8x8 ) :  m_cIdx( B8x8Idx(ePar8x8) )  { clear(); }
    Void clear()
    {
      m_apcRefBuffer[0] = m_apcRefBuffer[1] = NULL;
      m_apcPW[0]        = m_apcPW[1]        = NULL;
    }

    B4x4Idx           m_cIdx;
    PW                m_acPW[2];
    const PW*         m_apcPW[2];
    IntYuvPicBuffer*  m_apcRefBuffer[2];
    Mv3D              m_aacMv[2][6];
    Mv3D              m_aacMvd[2][6];  // differential motion vector
  };

protected:
  MotionCompensation();
  virtual ~MotionCompensation();

public:
  static ErrVal create( MotionCompensation*& rpcMotionCompensation );
  ErrVal destroy();

  ErrVal init( QuarterPelFilter* pcQuarterPelFilter,
               Transform*        pcTransform,
               SampleWeighting* pcSampleWeighting);

  ErrVal initSlice( const SliceHeader& rcSH );
  ErrVal uninit();

  ErrVal compensateMb( MbDataAccess& rcMbDataAccess, YuvMbBuffer* pcRecBuffer, Bool bFaultTolerant, Bool bCalcMv = true );
  ErrVal calculateMb( MbDataAccess& rcMbDataAccess, Bool bFaultTolerant );

  ErrVal compensateMbBLSkipIntra( MbDataAccess&      rcMbDataAccessBase,
                                  IntYuvMbBuffer*    pcRecBuffer,
                                  IntFrame*          pcBaseLayerRec );
  ErrVal copyMbBuffer(  IntYuvMbBuffer*    pcMbBufSrc,
                        IntYuvMbBuffer*    pcMbBufDes,
                        Int sX, Int sY, Int eX, Int eY);

  Void setResizeParameters   (ResizeParameters*				resizeParameters)
  {
    m_pcResizeParameters = resizeParameters;
  }; 

  ErrVal compensateMb     ( MbDataAccess&   rcMbDataAccess,
                            RefFrameList&   rcRefFrameList0,
                            RefFrameList&   rcRefFrameList1,
                            IntYuvMbBuffer* pcRecBuffer,
                            Bool            bCalcMv,
                            Bool            bSR = false );
  ErrVal compensateSubMb  ( B8x8Idx         c8x8Idx,
                            MbDataAccess&   rcMbDataAccess,
                            RefFrameList&   rcRefFrameList0,
                            RefFrameList&   rcRefFrameList1,
                            IntYuvMbBuffer* pcRecBuffer,
                            Bool            bCalcMv,
                            Bool            bFaultTolerant, 
                            Bool            bSR = false );
  Void xAdjustResidualRefBlk          ( XPel*           piResidualRef,
                                        UInt            uiBlkWidth,
                                        UInt            uiBlkHeight,
                                        Int             iStride,
                                        UChar*          pucSigMap,
                                        Bool            bNonzeroBaseBlock,
                                        Int             iBcbpCtx,
                                        UInt            uiWeightZeroBlk,
                                        UInt            uiWeightZeroCoeff);

  Void xAdjustResidualRefBlkSpatial   ( XPel*           piResidualRef,
                                        UInt            uiBlkWidth,
                                        UInt            uiBlkHeight,
                                        Int             iStride,
                                        UInt            uiWeightZeroBlk);

  Void xAdjustResidualRefBlkFrequency ( XPel*           piResidualRef,
                                        UInt            uiBlkWidth,
                                        UInt            uiBlkHeight,
                                        Int             iStride,
                                        UChar*          pucSigMap,
                                        UInt            uiWeightZeroCoeff);

  Void xAdjustChromaResidualRefBlock  ( XPel*           piResidualRef,
                                        Int             iStride,
                                        UChar*          pusSigMap,
                                        UInt            uiWeightZeroCoeff);

  ErrVal xCompensateMbAllModes        ( MbDataAccess&   rcMbDataAccess,
                                        RefFrameList&   rcRefFrameList0,
                                        RefFrameList&   rcRefFrameList1,
                                        IntYuvMbBuffer* pcYuvMbBuffer,
                                        Bool            bSR = false );

  ErrVal xAdjustMbResidual            ( IntYuvMbBuffer& rcMbBufferDiff,
                                        MbDataAccess*   pcMbDataAccess,
                                        FGSCoder*       pcFGSCoder,
                                        SliceHeader*    pcSliceHeader );

  ErrVal adaptiveMotionCompensationMb(  IntYuvMbBuffer* pcMbBufferMC,
                                        RefFrameList*   pcRefFrameListDiff,
                                        MbDataAccess*   pcMbDataAccessMotion,
                                        FGSCoder*       pcFGSCoder );

  ErrVal xAdaptiveMotionCompensation  ( YuvBufferCtrl*  pcYuvFullPelBufferCtrl,
                                        IntFrame*       pcMCFrame,
                                        IntFrame*       pcBaseFrame,
                                        RefFrameList*   pcRefFrameListBase,
                                        MbDataCtrl*     pcMbDataCtrl,
                                        FGSCoder*       pcFGSCoder,
                                        SliceHeader*    pcSliceHeader );

  ErrVal loadAdaptiveRefPredictors    ( YuvBufferCtrl*  pcYuvFullPelBufferCtrl,
                                        IntFrame*       pcPredSignal,
                                        IntFrame*       pcBaseFrame,
                                        RefFrameList*   cRefListDiff,
                                        MbDataCtrl*     pcMbDataCtrl,
                                        FGSCoder*       pcFGSCoder,
                                        SliceHeader*    pcSliceHeader);


  ErrVal updateMb(MbDataAccess&   rcMbDataAccess,
                  IntFrame*       pcMCFrame,
                  IntFrame*       pcPrdFrame,
                  ListIdx         eListPrd,
                  Int             iRefIdx);

  ErrVal updateSubMb( B8x8Idx         c8x8Idx,
                      MbDataAccess&   rcMbDataAccess,
                      IntFrame*       pcMCFrame,
                      IntFrame*       pcPrdFrame,
                      ListIdx         eListPrd );

  Void xUpdateMb8x8Mode(    B8x8Idx         c8x8Idx,
                            MbDataAccess&   rcMbDataAccess,
                            IntFrame*       pcMCFrame,
                            IntFrame*       pcPrdFrame,
                            ListIdx         eListPrd );

  ErrVal updateDirectBlock( MbDataAccess&   rcMbDataAccess,
                            IntFrame*       pcMCFrame,
                            IntFrame*       pcPrdFrame,
                            ListIdx         eListPrd,
                            Int             iRefIdx,
                            B8x8Idx         c8x8Idx );

  Void xUpdateBlk( IntFrame* pcPrdFrame, Int iSizeX, Int iSizeY, IntMC8x8D& rcMc8x8D );
  Void xUpdateBlk( IntFrame* pcPrdFrame, Int iSizeX, Int iSizeY, IntMC8x8D& rcMc8x8D, SParIdx4x4 eSParIdx );

  Void xUpdateLuma( IntFrame* pcPrdFrame, Int iSizeX, Int iSizeY, IntMC8x8D& rcMc8x8D, UShort *usWeight );
  Void xUpdateLuma( IntFrame* pcPrdFrame, Int iSizeX, Int iSizeY, IntMC8x8D& rcMc8x8D, SParIdx4x4 eSParIdx, UShort *usWeight );

  Void updateBlkAdapt( IntYuvPicBuffer* pcSrcBuffer, IntYuvPicBuffer* pcDesBuffer, LumaIdx cIdx, Mv cMv, Int iSizeY, Int iSizeX,
                                      UShort *usWeight);

  Void xUpdAdapt( XPel* pucDest, XPel* pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy,
                                    UInt uiSizeY, UInt uiSizeX, UShort weight, UShort wMax );

  __inline Void xUpdateChroma( IntYuvPicBuffer* pcSrcBuffer, IntYuvPicBuffer* pcDesBuffer,  LumaIdx cIdx, Mv cMv,
    Int iSizeY, Int iSizeX, UShort *usWeight);
  Void xUpdateChroma( IntFrame* pcSrcFrame, Int iSizeX, Int iSizeY, IntMC8x8D& rcMc8x8D, SParIdx4x4 eSParIdx, UShort *usWeight );
  Void xUpdateChroma( IntFrame* pcSrcFrame, Int iSizeX, Int iSizeY, IntMC8x8D& rcMc8x8D, UShort *usWeight );
  __inline Void xUpdateChromaPel( XPel* pucDest, Int iDestStride, XPel* pucSrc, Int iSrcStride, Mv cMv, Int iSizeY, Int iSizeX, UShort weight );

  ErrVal calcMvMb   (                   MbDataAccess& rcMbDataAccess, MbDataAccess* pcMbDataAccessBase );
//TMM_EC {
  ErrVal calcMvMbTD (                   MbDataAccess& rcMbDataAccess, MbDataAccess* pcMbDataAccessBase, RefFrameList& rcRefFrameListL0, RefFrameList& rcRefFrameListL1  );
//TMM_EC }
  ErrVal calcMvSubMb( B8x8Idx c8x8Idx,  MbDataAccess& rcMbDataAccess, MbDataAccess* pcMbDataAccessBase );

  ErrVal compensateDirectBlock( MbDataAccess& rcMbDataAccess, YuvMbBuffer *pcRecBuffer, B8x8Idx c8x8Idx, Bool& rbValid, Bool bFaultTolerant, Bool bCalcMv = true );
  ErrVal compensateDirectBlock( MbDataAccess& rcMbDataAccess, IntYuvMbBuffer *pcRecBuffer, B8x8Idx c8x8Idx, RefFrameList& rcRefFrameListL0, RefFrameList& rcRefFrameListL1, Bool bSR = false );
  ErrVal initMb( UInt uiMbY, UInt uiMbX);


protected:
  Void xPredMb8x8Mode( MbDataAccess& rcMbDataAccess, YuvMbBuffer* pcRecBuffer );
  Void xPredMb8x8Mode( B8x8Idx c8x8Idx, MbDataAccess& rcMbDataAccess, const IntFrame* pcRefFrame0, const IntFrame* pcRefFrame1, IntYuvMbBuffer* pcRecBuffer, Bool bSR = false );

  Void xPredLuma(   YuvMbBuffer* pcRecBuffer, Int iSizeX, Int iSizeY, MC8x8D& rcMc8x8D );
  Void xPredChroma( YuvMbBuffer* pcRecBuffer, Int iSizeX, Int iSizeY, MC8x8D& rcMc8x8D );

  Void xPredLuma(   YuvMbBuffer* apcTarBuffer[2], Int iSizeX, Int iSizeY, MC8x8D& rcMc8x8D, SParIdx4x4 eSParIdx );
  Void xPredChroma( YuvMbBuffer* apcTarBuffer[2], Int iSizeX, Int iSizeY, MC8x8D& rcMc8x8D, SParIdx4x4 eSParIdx );


  Void xPredLuma  ( IntYuvMbBuffer* pcRecBuffer,      Int iSizeX, Int iSizeY, IntMC8x8D& rcMc8x8D, Bool bSR = false );
  Void xPredLuma  ( IntYuvMbBuffer* apcTarBuffer[2],  Int iSizeX, Int iSizeY, IntMC8x8D& rcMc8x8D, SParIdx4x4 eSParIdx, Bool bSR = false );
  Void xPredChroma( IntYuvMbBuffer* pcRecBuffer,      Int iSizeX, Int iSizeY, IntMC8x8D& rcMc8x8D );
  Void xPredChroma( IntYuvMbBuffer* apcTarBuffer[2],  Int iSizeX, Int iSizeY, IntMC8x8D& rcMc8x8D, SParIdx4x4 eSParIdx );

private:
  __inline Void xGetMbPredData( MbDataAccess& rcMbDataAccess, MC8x8D& rcMC8x8D );
  __inline Void xGetBlkPredData( MbDataAccess& rcMbDataAccess, MC8x8D& rcMC8x8D, BlkMode eBlkMode );
  __inline Void xPredChromaPel( Pel* pucDest, Int iDestStride, Pel* pucSrc, Int iSrcStride, Mv cMv, Int iSizeY, Int iSizeX );
  __inline Void xPredChroma( YuvMbBuffer* pcDesBuffer, YuvPicBuffer* pcSrcBuffer, LumaIdx cIdx, Mv cMv, Int iSizeY, Int iSizeX);

  __inline Void xGetMbPredData  ( MbDataAccess& rcMbDataAccess, const IntFrame* pcRefFrame, IntMC8x8D& rcMC8x8D );
  __inline Void xGetBlkPredData ( MbDataAccess& rcMbDataAccess, const IntFrame* pcRefFrame, IntMC8x8D& rcMC8x8D, BlkMode eBlkMode );

  __inline Void xPredChromaPel  ( XPel* pucDest, Int iDestStride, XPel* pucSrc, Int iSrcStride, Mv cMv, Int iSizeY, Int iSizeX );
  __inline Void xPredChroma     ( IntYuvMbBuffer* pcDesBuffer, IntYuvPicBuffer* pcSrcBuffer, LumaIdx cIdx, Mv cMv, Int iSizeY, Int iSizeX);

  __inline Void xGetMbPredData  ( MbDataAccess& rcMbDataAccess, const IntFrame* pcRefFrame0, const IntFrame* pcRefFrame1, IntMC8x8D& rcMC8x8D );
  __inline Void xGetBlkPredData ( MbDataAccess& rcMbDataAccess, const IntFrame* pcRefFrame0, const IntFrame* pcRefFrame1, IntMC8x8D& rcMC8x8D, BlkMode eBlkMode );



protected:
  QuarterPelFilter* m_pcQuarterPelFilter;
  Transform*        m_pcTransform;
  SampleWeighting* m_pcSampleWeighting;
  Mv   m_cMin;
  Mv   m_cMax;
  UInt m_uiMbInFrameY;
  UInt m_uiMbInFrameX;
  int m_curMbX;
  int m_curMbY;

  ResizeParameters*				m_pcResizeParameters; 

};

#if defined( WIN32 )
# pragma warning( default: 4251 )
#endif

#define DMV_THRES   5


H264AVC_NAMESPACE_END


#endif // !defined(AFX_MOTIONCOMPENSATION_H__820D6942_007B_42EA_838B_AC025E866DBE__INCLUDED_)
