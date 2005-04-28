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



#if !defined(AFX_GOPDECODER_H__339878FC_BA98_4ABE_8530_E1676196576F__INCLUDED_)
#define AFX_GOPDECODER_H__339878FC_BA98_4ABE_8530_E1676196576F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "H264AVCCommonLib/MbDataCtrl.h"
#include "DownConvert.h"


H264AVC_NAMESPACE_BEGIN

class H264AVCDecoder;
class SliceReader;
class SliceDecoder;
class FrameMng;
class PocCalculator;
class LoopFilter;
class HeaderSymbolReadIf;
class ParameterSetMng;
class NalUnitParser;
class ControlMngIf;
class MotionCompensation;
class IntFrame; 
class RQFGSDecoder;

class ReconstructionBypass;
class YuvBufferCtrl; 




class H264AVCDECODERLIB_API MCTFDecoder
{ 
  enum
  {
    NUM_TMP_FRAMES = 2
  };
  enum
  {
    LPS = 0x00,   // low pass signal
    HPS = 0x01    // high pass signal
  };
  enum NextNalType
  {
    ALL           = 0x00,
    LOW_PASS      = 0x01,
    LOW_PASS_IDR  = 0x02
  };



protected:
	MCTFDecoder         ();
	virtual ~MCTFDecoder();

public:
  //===== general functions ======
  static  ErrVal  create        ( MCTFDecoder*&               rpcMCTFDecoder );
  ErrVal          destroy       ();
  ErrVal          init          ( H264AVCDecoder*             pcH264AVCDecoder,
                                  SliceReader*                pcSliceReader,
                                  SliceDecoder*               pcSliceDecoder,
                                  RQFGSDecoder*               pcRQFGSDecoder,
                                  NalUnitParser*              pcNalUnitParser,
                                  ControlMngIf*               pcControlMng,
                                  LoopFilter*                 pcLoopFilter,
                                  HeaderSymbolReadIf*         pcHeaderSymbolReadIf,
                                  ParameterSetMng*            pcParameterSetMng,
                                  PocCalculator*              pcPocCalculator,
                                  YuvBufferCtrl*              pcYuvFullPelBufferCtrl,
                                  MotionCompensation*         pcMotionCompensation,
                                  QuarterPelFilter*           pcQuarterPelFilter );
  ErrVal          uninit        ();
  ErrVal          initSPS       ( const SequenceParameterSet& rcSPS );
  
  ErrVal          process       ( SliceHeader*&               rpcSliceHeader,
                                  PicBuffer*                  pcPicBuffer,
                                  PicBufferList&              rcPicBufferOutputList,
                                  PicBufferList&              rcPicBufferUnusedList,
                                  PicBufferList&              rcPicBufferReleaseList );
  ErrVal          finishProcess ( PicBufferList&              rcPicBufferOutputList,
                                  PicBufferList&              rcPicBufferUnusedList,
                                  PicBufferList&              rcPicBufferReleaseList,
                                  Int&                        riMaxPoc );
  ErrVal          reconstruct   ( PicBufferList&              rcPicBufferOutputList,
                                  PicBufferList&              rcPicBufferUnusedList,
                                  PicBufferList&              rcPicBufferReleaseList,
                                  Int&                        riMaxPoc );
  ErrVal          initSlice     ( SliceHeader* pcSliceHeader, UInt uiLastLayer );



  ErrVal          clearReconstructionStatus( PicBufferList&   rcPicBufferUnusedList );


  Bool            isActive        () { return m_bActive; }
  UInt            getFrameWidth   () { return m_uiFrameWidthInMb*16; }
  UInt            getFrameHeight  () { return m_uiFrameHeightInMb*16; }

  ErrVal          getBaseLayerData( IntFrame*&   pcFrame,
                                    IntFrame*&   pcResidual,
                                    MbDataCtrl*& pcMbDataCtrl,
                                    Bool         bHighPass,
                                    Bool         bSpatialScalability,
                                    Int          iPoc );



protected:
  //===== create and initialize data arrays =====
  ErrVal      xCreateData                     ( const SequenceParameterSet&   rcSPS );
  ErrVal      xDeleteData                     ();
  Bool        xIsNewGOP                       ( SliceHeader*                  pcSliceHeader );
  ErrVal      xInitGOP                        ( SliceHeader*                  pcSliceHeader );

  //===== inverse MCTF & motion compensation =====
  ErrVal      xReconstruct                    ( PicBufferList&                rcPicBufferOutputList,
                                                PicBufferList&                rcPicBufferUnusedList,
                                                Int&                          riMaxPoc );  
  ErrVal      xFillAndExtendFrame             ( IntFrame*                     rcFrame );
  ErrVal      xMotionCompensation             ( IntFrame*                     pcMCFrame,
                                                RefFrameList*                 pcRefFrameList0,
                                                RefFrameList*                 pcRefFrameList1,
                                                MbDataCtrl*                   pcMbDataCtrl,
                                                SliceHeader&                  rcSH );
  ErrVal      xZeroIntraMacroblocks           ( IntFrame*                     pcFrame,
                                                ControlData&                  rcCtrlData );
  ErrVal      xClipIntraMacroblocks           ( IntFrame*                     pcFrame,
                                                ControlData&                  rcCtrlData,
                                                Bool                          bClipAll );
  ErrVal      xAddBaseLayerResidual           ( ControlData&                  rcControlData,
                                                IntFrame*                     pcFrame );
  ErrVal      xInitBaseLayerData              ( ControlData&                  rcControlData );
  //===== check for inverse MCTF =====
  ErrVal      xInvokeMCTF                     ( UInt                          uiFrameIdInGOP );
  ErrVal      xCheckForInversePrediction      (  Int                          iPrdPicIdInGOP,
                                                 Int                          iUpdPicIdInGOP );
  ErrVal      xCheckForInverseUpdate          (  Int                          iUpdPicIdInGOP,
                                                 Int                          iPrdPicIdInGOP );
  ErrVal      xInversePrediction              ( UInt                          uiFrameIdInGOP );
  ErrVal      xInverseUpdate                  ( UInt                          uiFrameIdInGOP );
  
  //===== decode pictures / subbands =====
  ErrVal      xDecodeLowPassSignal            ( SliceHeader*&                 rpcSliceHeader );
  ErrVal      xDecodeHighPassSignal           ( SliceHeader*&                 rpcSliceHeader );
  ErrVal      xDecodeFGSRefinement            ( SliceHeader*&                 rpcSliceHeader );
  ErrVal      xReconstructLastFGS             ();
  ErrVal      xCalcMv                         ( SliceHeader*                  pcSliceHeader,
                                                MbDataCtrl*                   pcMbDataCtrl,
                                                MbDataCtrl*                   pcMbDataCtrlBase );

  //===== reference list handling ======
  ErrVal      xClearBufferExtensions          ();
  ErrVal      xGetPredictionLists             ( RefFrameList& rcRefList0,
                                                RefFrameList& rcRefList1,
                                                UInt          uiBaseLevel,
                                                UInt          uiFrame );
  ErrVal      xGetUpdateLists                 ( RefFrameList& rcRefList0,
                                                RefFrameList& rcRefList1,
                                                CtrlDataList& rcCtrlList0,
                                                CtrlDataList& rcCtrlList1,
                                                UInt          uiBaseLevel,
                                                UInt          uiFrame );
  ErrVal      xCompositionStage               ( UInt          uiBaseLevel );




  ErrVal xDumpFrames( Char* pFilename, UInt uiStage = MSYS_UINT_MAX );


protected:
  //----- references -----
  H264AVCDecoder*     m_pcH264AVCDecoder;
  SliceReader*        m_pcSliceReader;
  SliceDecoder*       m_pcSliceDecoder;
  RQFGSDecoder*       m_pcRQFGSDecoder;
  NalUnitParser*      m_pcNalUnitParser;
  ControlMngIf*       m_pcControlMng;
  LoopFilter*         m_pcLoopFilter;
  HeaderSymbolReadIf* m_pcHeaderSymbolReadIf;
  ParameterSetMng*    m_pcParameterSetMng;
  PocCalculator*      m_pcPocCalculator;
  YuvBufferCtrl*      m_pcYuvFullPelBufferCtrl;
  MotionCompensation* m_pcMotionCompensation;
  QuarterPelFilter*   m_pcQuarterPelFilter;
  DownConvert         m_cDownConvert;


  //----- general parameters -----
  Bool                m_bInitDone;
  Bool                m_bCreateDone;
  NextNalType         m_eNextNalType;
  UInt                m_uiMaxDecompositionStages;
  UInt                m_uiMaxGOPSize;
  UInt                m_uiFrameWidthInMb;
  UInt                m_uiFrameHeightInMb;
  UInt                m_uiMbNumber;
  UInt                m_uiGOPId;
  PicBufferList       m_cPicBufferList;

  Bool                m_bActive;
  Bool                m_bReconstructed;
  UInt                m_uiLayerId;
  
  //----- decomposition structure ----
  UInt                m_uiFrameNumber;
  UInt                m_uiGOPSize;
  UInt                m_uiDecompositionStages;

  //----- frame memories -----
  IntFrame*           m_apcFrameTemp    [NUM_TMP_FRAMES];
  IntFrame**          m_papcFrame;
  IntFrame**          m_papcResidual;
  IntFrame*           m_pcLowPassBaseReconstruction;
  IntFrame*           m_pcAnchorFrame;
  IntFrame*           m_pcPredSignal;
  IntFrame*           m_pcBaseLayerFrame;
  IntFrame*           m_pcBaseLayerResidual;

  //----- control data memories -----
  ControlData*        m_pacControlData;
  ControlData         m_cControlDataUpd;
  MbDataCtrl*         m_pcBaseLayerCtrl;

  SliceHeader*        m_pcCurrSliceHeader;
  
  ConnectionArray     m_cConnectionArray;
  UShort*             m_pusUpdateWeights;
};


H264AVC_NAMESPACE_END


#endif // !defined(AFX_GOPDECODER_H__339878FC_BA98_4ABE_8530_E1676196576F__INCLUDED_)
