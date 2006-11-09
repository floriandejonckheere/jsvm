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

#if !defined  _REC_PIC_BUFFER_INCLUDED_
#define       _REC_PIC_BUFFER_INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "H264AVCCommonLib/MbDataCtrl.h"
#include "H264AVCCommonLib/IntFrame.h"


H264AVC_NAMESPACE_BEGIN


class RecPicBufUnit
{
protected:
  RecPicBufUnit             ();
  virtual ~RecPicBufUnit    ();

public:
  static ErrVal create          ( RecPicBufUnit*&             rpcRecPicBufUnit,
                                  YuvBufferCtrl&              rcYuvBufferCtrlFullPel,
                                  YuvBufferCtrl&              rcYuvBufferCtrlHalfPel,
                                  const SequenceParameterSet& rcSPS );
  ErrVal        destroy         ();

  ErrVal        init            ( SliceHeader*                pcSliceHeader,
                                  PicBuffer*                  pcPicBuffer );
  ErrVal        initNonEx       ( Int                         iPoc,
                                  UInt                        uiFrameNum );
  ErrVal        uninit          ();

  ErrVal        markNonRef      ();
  ErrVal        markOutputted   ();

  Int           getPoc          ()  const { return m_iPoc; }
  UInt          getFrameNum     ()  const { return m_uiFrameNum; }
  Bool          isExisting      ()  const { return m_bExisting; }
  Bool          isNeededForRef  ()  const { return m_bNeededForReference; }
  Bool          isOutputted     ()  const { return m_bOutputted; }
  IntFrame*     getRecFrame     ()        { return m_pcReconstructedFrame; }
  MbDataCtrl*   getMbDataCtrl   ()        { return m_pcMbDataCtrl; }
  PicBuffer*    getPicBuffer    ()        { return m_pcPicBuffer; }

  Int           getPicNum       ( UInt uiCurrFrameNum,
                                  UInt uiMaxFrameNum )  const
  {
    if( m_uiFrameNum > uiCurrFrameNum )
    {
      return (Int)m_uiFrameNum - (Int)uiMaxFrameNum;
    }
    return (Int)m_uiFrameNum;
  }


private:
  Int           m_iPoc;
  UInt          m_uiFrameNum;
  Bool          m_bExisting;
  Bool          m_bNeededForReference;
  Bool          m_bOutputted;
  IntFrame*     m_pcReconstructedFrame;
  MbDataCtrl*   m_pcMbDataCtrl;
  PicBuffer*    m_pcPicBuffer;
};


typedef MyList<RecPicBufUnit*>  RecPicBufUnitList;



class RecPicBuffer
{
protected:
  RecPicBuffer          ();
  virtual ~RecPicBuffer ();

public:
  static ErrVal   create                ( RecPicBuffer*&              rpcRecPicBuffer );
  ErrVal          destroy               ();
  ErrVal          init                  ( YuvBufferCtrl*              pcYuvBufferCtrlFullPel,
                                          YuvBufferCtrl*              pcYuvBufferCtrlHalfPel );
  ErrVal          initSPS               ( const SequenceParameterSet& rcSPS );
  ErrVal          uninit                ();
  ErrVal          clear                 ( PicBufferList&              rcOutputList,
                                          PicBufferList&              rcUnusedList );

  ErrVal          initCurrRecPicBufUnit ( RecPicBufUnit*&             rpcCurrRecPicBufUnit,
                                          PicBuffer*                  pcPicBuffer,
                                          SliceHeader*                pcSliceHeader,
                                          PicBufferList&              rcOutputList,
                                          PicBufferList&              rcUnusedList );
  ErrVal          store                 ( RecPicBufUnit*              pcRecPicBufUnit,
                                          SliceHeader*                pcSliceHeader,
                                          PicBufferList&              rcOutputList,
                                          PicBufferList&              rcUnusedList );
  ErrVal          getRefLists           ( RefFrameList&               rcList0,
                                          RefFrameList&               rcList1,
                                          SliceHeader&                rcSliceHeader );

  RecPicBufUnit*  getLastUnit           ();
  RecPicBufUnit*  getCurrUnit           ();
  RecPicBufUnit*  getRecPicBufUnit      ( Int                         iPoc );

private:
  ErrVal          xCreateData           ( UInt                        uiMaxFramesInDPB,
                                          const SequenceParameterSet& rcSPS );
  ErrVal          xDeleteData           ();


  //===== memory management =====
  ErrVal          xCheckMissingPics     ( SliceHeader*                pcSliceHeader,
                                          PicBufferList&              rcOutputList,
                                          PicBufferList&              rcUnusedList );
  ErrVal          xStorePicture         ( RecPicBufUnit*              pcRecPicBufUnit,
                                          PicBufferList&              rcOutputList,
                                          PicBufferList&              rcUnusedList,
                                          SliceHeader*                pcSliceHeader,
                                          Bool                        bTreatAsIdr );
  ErrVal          xOutput               ( PicBufferList&              rcOutputList,
                                          PicBufferList&              rcUnusedList );
  ErrVal          xClearOutputAll       ( PicBufferList&              rcOutputList,
                                          PicBufferList&              rcUnusedList );
  ErrVal          xUpdateMemory         ( SliceHeader*                pcSliceHeader );
  ErrVal          xClearBuffer          ();
  ErrVal          xSlidingWindow        ();
  ErrVal          xMMCO                 ( SliceHeader*                pcSliceHeader );
  ErrVal          xMarkShortTermUnused  ( RecPicBufUnit*              pcCurrentRecPicBufUnit,
                                          UInt                        uiDiffOfPicNums );

  //===== reference picture lists =====
  ErrVal          xInitRefListPSlice    ( RefFrameList&               rcList );
  ErrVal          xInitRefListsBSlice   ( RefFrameList&               rcList0,
                                          RefFrameList&               rcList1 );
  ErrVal          xRefListRemapping     ( RefFrameList&               rcList,
                                          ListIdx                     eListIdx,
                                          SliceHeader*                pcSliceHeader );
  ErrVal          xAdaptListSize        ( RefFrameList&               rcList,
                                          ListIdx                     eListIdx,
                                          SliceHeader&                rcSliceHeader );

  //--- debug ---
  ErrVal          xDumpRecPicBuffer     ();
  ErrVal          xDumpRefList          ( RefFrameList&               rcList,
                                          ListIdx                     eListIdx );

private:
  Bool                m_bInitDone;
  YuvBufferCtrl*      m_pcYuvBufferCtrlFullPel;
  YuvBufferCtrl*      m_pcYuvBufferCtrlHalfPel;

  UInt                m_uiNumRefFrames;
  UInt                m_uiMaxFrameNum;
  UInt                m_uiLastRefFrameNum;
  RecPicBufUnitList   m_cUsedRecPicBufUnitList;
  RecPicBufUnitList   m_cFreeRecPicBufUnitList;
  RecPicBufUnit*      m_pcCurrRecPicBufUnit;
};


H264AVC_NAMESPACE_END


#endif // _REC_PIC_BUFFER_INCLUDED_

