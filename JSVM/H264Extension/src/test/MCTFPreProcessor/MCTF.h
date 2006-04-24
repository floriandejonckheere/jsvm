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


#if !defined  _MCTF_H_
#define       _MCTF_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class PreProcessorParameter;

H264AVC_NAMESPACE_BEGIN

class SliceHeader;
class MbDataCtrl;
class MbEncoder;
class MotionEstimation;
class IntFrame;


class MCTF
{
protected:
  MCTF          ();
  virtual ~MCTF ();

public:
  static ErrVal create              ( MCTF*&                      rpcMCTF );
  ErrVal        destroy             ();
  ErrVal        init                ( PreProcessorParameter*      pcParameter,
                                      MbEncoder*                  pcMbEncoder,
                                      YuvBufferCtrl*              pcYuvFullPelBufferCtrl,
                                      YuvBufferCtrl*              pcYuvHalfPelBufferCtrl,
                                      QuarterPelFilter*           pcQuarterPelFilter,
                                      MotionEstimation*           pcMotionEstimation );
  ErrVal        uninit              ();
  ErrVal        process             ( PicBuffer*                  pcOrgPicBuffer,
                                      PicBuffer*                  pcRecPicBuffer,
                                      PicBufferList&              rcPicBufferOutputList,
                                      PicBufferList&              rcPicBufferUnusedList );

protected:
  //===== main functions =====
  ErrVal        xProcessGOP         ( PicBufferList&              rcPicBufferInputList,
                                      PicBufferList&              rcPicBufferOutputList,
                                      PicBufferList&              rcPicBufferUnusedList );

  //===== data management =====
  ErrVal  xCreateData               ( const SequenceParameterSet& rcSPS );
  ErrVal  xDeleteData               ();
  ErrVal  xInitGOP                  ( PicBufferList&              rcPicBufferInputList );
  ErrVal  xFinishGOP                ( PicBufferList&              rcPicBufferInputList,
                                      PicBufferList&              rcPicBufferOutputList,
                                      PicBufferList&              rcPicBufferUnusedList );
  ErrVal  xStoreReconstruction      ( PicBufferList&              rcPicBufferOutputList );

  //===== decomposition / composition =====
  ErrVal  xMotionEstimationStage    ( UInt                        uiBaseLevel );
  ErrVal  xDecompositionStage       ( UInt                        uiBaseLevel );
  ErrVal  xCompositionStage         ( UInt                        uiBaseLevel,
                                      PicBufferList&              rcPicBufferInputList );

  //===== control data initialization =====
  ErrVal  xGetConnections           ( Double&                     rdL0Rate,
                                      Double&                     rdL1Rate,
                                      Double&                     rdBiRate );
  ErrVal  xSetScalingFactors        ( UInt                        uiBaseLevel );
  ErrVal  xGetListSizes             ( UInt                        uiTemporalLevel,
                                      UInt                        uiFrameIdInGOP,
                                      UInt                        auiPredListSize[2],
                                      UInt                        aauiUpdListSize[MAX_DSTAGES][2] );
  ErrVal  xInitSliceHeader          ( UInt                        uiTemporalLevel,
                                      UInt                        uiFrameIdInGOP );
  ErrVal  xClearBufferExtensions    ();
  ErrVal  xGetPredictionLists       ( RefFrameList&               rcRefList0,
                                      RefFrameList&               rcRefList1,
                                      UInt                        uiBaseLevel,
                                      UInt                        uiFrame,
                                      Bool                        bHalfPel );
  ErrVal  xGetUpdateLists           ( RefFrameList&               rcRefList0,
                                      RefFrameList&               rcRefList1,
                                      CtrlDataList&               rcCtrlList0,
                                      CtrlDataList&               rcCtrlList1,
                                      UInt                        uiBaseLevel,
                                      UInt                        uiFrame );
  ErrVal  xInitControlDataMotion    ( UInt                        uiBaseLevel,
                                      UInt                        uiFrame,
                                      Bool                        bMotionEstimation );

  //===== motion estimation / compensation =====
  ErrVal  xMotionCompensation       ( IntFrame*                   pcMCFrame,
                                      RefFrameList*               pcRefFrameList0,
                                      RefFrameList*               pcRefFrameList1,
                                      MbDataCtrl*                 pcMbDataCtrl,
                                      SliceHeader&                rcSH );
  ErrVal  xMotionEstimation         ( RefFrameList*               pcRefFrameList0,
                                      RefFrameList*               pcRefFrameList1,
                                      const IntFrame*             pcOrigFrame,
                                      ControlData&                rcControlData );
  ErrVal  xUpdateCompensation       ( IntFrame*                   pcMCFrame,
                                      RefFrameList*               pcRefFrameList,
                                      CtrlDataList*               pcCtrlDataList,
                                      ListIdx                     eListUpd );

  //===== auxiliary functions =====
  ErrVal  xFillAndUpsampleFrame     ( IntFrame*                   rcFrame );
  ErrVal  xFillAndExtendFrame       ( IntFrame*                   rcFrame );
  ErrVal  xZeroIntraMacroblocks     ( IntFrame*                   pcFrame,
                                      ControlData&                pcCtrlData );

protected:
  SequenceParameterSet*         m_pcSPS;
  PictureParameterSet*          m_pcPPS;
  YuvBufferCtrl*                m_pcYuvFullPelBufferCtrl;
  YuvBufferCtrl*                m_pcYuvHalfPelBufferCtrl;
  MbEncoder*                    m_pcMbEncoder;
  QuarterPelFilter*             m_pcQuarterPelFilter;
  MotionEstimation*             m_pcMotionEstimation;

  Bool                          m_bFirstGOPCoded;                     // true if first GOP of a sequence has been coded
  UInt                          m_uiGOPSize;                          // current GOP size
  UInt                          m_uiDecompositionStages;              // number of decomposition stages
  UInt                          m_uiFrameWidthInMb;                   // frame width in macroblocks
  UInt                          m_uiFrameHeightInMb;                  // frame height in macroblocks
  UInt                          m_uiMbNumber;                         // number of macroblocks in a frame
  Double                        m_adBaseQpLambdaMotion[MAX_DSTAGES];  // base QP's for mode decision and motion estimation
  IntFrame*                     m_pcFrameTemp;                        // auxiliary frame memory
  IntFrame**                    m_papcFrame;                          // frame stores
  IntFrame**                    m_papcResidual;                       // frame stores for residual data
  ControlData*                  m_pacControlData;                     // control data arrays
  PicBufferList                 m_cOrgPicBufferList;
  PicBufferList                 m_cRecPicBufferList;
};

H264AVC_NAMESPACE_END

#endif // _MCTF_H_
