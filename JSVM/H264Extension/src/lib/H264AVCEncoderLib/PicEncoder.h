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


#if !defined  _PIC_ENCODER_INCLUDED_
#define       _PIC_ENCODER_INCLUDED_

#include "InputPicBuffer.h"
#include "SequenceStructure.h"
#include "RecPicBuffer.h"

H264AVC_NAMESPACE_BEGIN

class CodingParameter;
class ParameterSetMng;
class SliceEncoder;
class LoopFilter;
class PocCalculator;
class NalUnitEncoder;
class YuvBufferCtrl;
class QuarterPelFilter;
class MotionEstimation;
class ControlMngIf;


class PicEncoder
{
public:
  PicEncoder                                    ();
  virtual ~PicEncoder                           ();

  static ErrVal   create                        ( PicEncoder*&                rpcPicEncoder );
  ErrVal          destroy                       ();

  ErrVal          init                          ( CodingParameter*            pcCodingParameter,
                                                  ControlMngIf*               pcControlMng,
                                                  SliceEncoder*               pcSliceEncoder,
                                                  LoopFilter*                 pcLoopFilter,
                                                  PocCalculator*              pcPocCalculator,
                                                  NalUnitEncoder*             pcNalUnitEncoder,
                                                  YuvBufferCtrl*              pcYuvBufferCtrlFullPel,
                                                  YuvBufferCtrl*              pcYuvBufferCtrlHalfPel,
                                                  QuarterPelFilter*           pcQuarterPelFilter,
                                                  MotionEstimation*           pcMotionEstimation );
  ErrVal          uninit                        ();

  ErrVal          writeAndInitParameterSets     ( ExtBinDataAccessor*         pcExtBinDataAccessor,
                                                  Bool&                       rbMoreSets );
  ErrVal          process                       ( PicBuffer*                  pcInputPicBuffer,
                                                  PicBufferList&              rcOutputList,
                                                  PicBufferList&              rcUnusedList,
                                                  ExtBinDataAccessorList&     rcExtBinDataAccessorList );
  ErrVal          finish                        ( PicBufferList&              rcOutputList,
                                                  PicBufferList&              rcUnusedList );

private:
  //===== initializations =====
  ErrVal          xInitSPS                      ();
  ErrVal          xInitPPS                      ();
  ErrVal          xInitParameterSets            ();
  ErrVal          xInitSliceHeader              ( SliceHeader*&               rpcSliceHeader,
                                                  const FrameSpec&            rcFrameSpec,
                                                  Double&                     dLambda );
  ErrVal          xInitPredWeights              ( SliceHeader&                rcSliceHeader );

  //===== create and delete memory =====
  ErrVal          xCreateData                   ();
  ErrVal          xDeleteData                   ();

  //===== packet management =====
  ErrVal          xInitExtBinDataAccessor       ( ExtBinDataAccessor&         rcExtBinDataAccessor );
  ErrVal          xAppendNewExtBinDataAccessor  ( ExtBinDataAccessorList&     rcExtBinDataAccessorList,
                                                  ExtBinDataAccessor*         pcExtBinDataAccessor );

  //===== encoding =====
  ErrVal          xStartPicture                 ( RecPicBufUnit&              rcRecPicBufUnit,
                                                  SliceHeader&                rcSliceHeader,
                                                  RefFrameList&               rcList0,
                                                  RefFrameList&               rcList1 );
  ErrVal          xEncodePicture                ( ExtBinDataAccessorList&     rcExtBinDataAccessorList,
                                                  RecPicBufUnit&              rcRecPicBufUnit,
                                                  SliceHeader&                rcSliceHeader,
                                                  Double                      dLambda,
                                                  UInt&                       ruiBits );
  ErrVal          xFinishPicture                ( RecPicBufUnit&              rcRecPicBufUnit,
                                                  SliceHeader&                rcSliceHeader,
                                                  RefFrameList&               rcList0,
                                                  RefFrameList&               rcList1,
                                                  UInt                        uiBits );
  ErrVal          xGetPSNR                      ( RecPicBufUnit&              rcRecPicBufUnit,
                                                  Double*                     adPSNR );

private:
  Bool                        m_bInit;
  Bool                        m_bInitParameterSets;

  //===== members =====
  BinData                     m_cBinData;
  ExtBinDataAccessor          m_cExtBinDataAccessor;
  FrameSpec                   m_cFrameSpecification;
  SequenceStructure*          m_pcSequenceStructure;
  InputPicBuffer*             m_pcInputPicBuffer;
  SequenceParameterSet*       m_pcSPS;
  PictureParameterSet*        m_pcPPS;
  RecPicBuffer*               m_pcRecPicBuffer;

  //===== references =====
  CodingParameter*            m_pcCodingParameter;
  ControlMngIf*               m_pcControlMng;
  SliceEncoder*               m_pcSliceEncoder;
  LoopFilter*                 m_pcLoopFilter;
  PocCalculator*              m_pcPocCalculator;
  NalUnitEncoder*             m_pcNalUnitEncoder;
  YuvBufferCtrl*              m_pcYuvBufferCtrlFullPel;
  YuvBufferCtrl*              m_pcYuvBufferCtrlHalfPel;
  QuarterPelFilter*           m_pcQuarterPelFilter;
  MotionEstimation*           m_pcMotionEstimation;

  //===== fixed coding parameters =====
  UInt                        m_uiFrameWidthInMb;
  UInt                        m_uiFrameHeightInMb;
  UInt                        m_uiMbNumber;

  //===== variable parameters =====
  UInt                        m_uiFrameNum;
  UInt                        m_uiIdrPicId;
  UInt                        m_uiWrittenBytes;
  UInt                        m_uiCodedFrames;
  Double                      m_dSumYPSNR;
  Double                      m_dSumUPSNR;
  Double                      m_dSumVPSNR;

  //===== auxiliary buffers =====
  UInt                        m_uiWriteBufferSize;                  // size of temporary write buffer
  UChar*                      m_pucWriteBuffer;                     // write buffer

  Bool                        m_bTraceEnable;
};


H264AVC_NAMESPACE_END


#endif // _PIC_ENCODER_INCLUDED_
