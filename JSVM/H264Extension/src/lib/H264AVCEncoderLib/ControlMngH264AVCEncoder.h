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




#if !defined(AFX_CONTROLMNGH264AVCENCODER_H__CE22E161_6ACE_4E6F_9E1B_08F0A99F9742__INCLUDED_)
#define AFX_CONTROLMNGH264AVCENCODER_H__CE22E161_6ACE_4E6F_9E1B_08F0A99F9742__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "H264AVCCommonLib/ControlMngIf.h"
#include "MbSymbolWriteIf.h"
#include "H264AVCEncoder.h"
#include "H264AVCCommonLib/MbData.h"
#include "H264AVCCommonLib/Frame.h"
#include "H264AVCCommonLib/FrameMng.h"
#include "BitWriteBuffer.h"
#include "H264AVCCommonLib/Transform.h"
#include "H264AVCCommonLib/YuvBufferCtrl.h"
#include "H264AVCCommonLib/QuarterPelFilter.h"
#include "H264AVCCommonLib/ParameterSetMng.h"
#include "H264AVCCommonLib/LoopFilter.h"
#include "H264AVCCommonLib/SampleWeighting.h"
#include "H264AVCCommonLib/PocCalculator.h"

#include "SliceEncoder.h"
#include "UvlcWriter.h"
#include "MbCoder.h"
#include "MbEncoder.h"
#include "IntraPredictionSearch.h"
#include "CodingParameter.h"
#include "CabacWriter.h"
#include "NalUnitEncoder.h"
#include "Distortion.h"
#include "MotionEstimation.h"
#include "MotionEstimationQuarterPel.h"
#include "RateDistortion.h"
#include "GOPEncoder.h"


H264AVC_NAMESPACE_BEGIN

class ControlMngH264AVCEncoder : public ControlMngIf
{
protected:
	ControlMngH264AVCEncoder();
	virtual ~ControlMngH264AVCEncoder();

public:
  static ErrVal create( ControlMngH264AVCEncoder*& rpcControlMngH264AVCEncoder );
  ErrVal init(  FrameMng*               pcFrameMng,
                MCTFEncoder*            apcMCTFEncoder          [MAX_LAYERS],
                SliceEncoder*           pcSliceEncoder,
                ControlMngH264AVCEncoder*  pcControlMng,
                BitWriteBuffer*         pcBitWriteBuffer,
                BitCounter*             pcBitCounter,
                NalUnitEncoder*         pcNalUnitEncoder,
                UvlcWriter*             pcUvlcWriter,
                UvlcWriter*             pcUvlcTester,
                MbCoder*                pcMbCoder,
                LoopFilter*             pcLoopFilter,
                MbEncoder*              pcMbEncoder,
                Transform*              pcTransform,
                IntraPredictionSearch*  pcIntraPrediction,
                YuvBufferCtrl*          apcYuvFullPelBufferCtrl [MAX_LAYERS],
                YuvBufferCtrl*          apcYuvHalfPelBufferCtrl [MAX_LAYERS],
                QuarterPelFilter*       pcQuarterPelFilter,
                CodingParameter*        pcCodingParameter,
                ParameterSetMng*        pcParameterSetMng,
                PocCalculator*          apcPocCalculator        [MAX_LAYERS],
                SampleWeighting*        pcSampleWeighting,
                CabacWriter*            pcCabacWriter,
                XDistortion*            pcXDistortion,
                MotionEstimation*       pcMotionEstimation,
                RateDistortion*         pcRateDistortion );

  ErrVal uninit();
  ErrVal destroy();


  ErrVal initSPS          ( SequenceParameterSet&       rcSPS   ) { return Err::m_nERR; }
  ErrVal initParameterSets( const SequenceParameterSet& rcSPS,
                            const PictureParameterSet&  rcPPSLP,
                            const PictureParameterSet&  rcPPSHP );

  ErrVal initMbForFiltering( MbDataAccess*& rpcMbDataAccess, UInt uiMbIndex );

  ErrVal initSlice( SliceHeader& rcSH, ProcessingState eProcessingState );
  ErrVal finishSlice( const SliceHeader& rcSH, Bool& rbPicDone, Bool& rbFrameDone );

  ErrVal initMbForParsing( MbDataAccess*& rpcMbDataAccess, UInt uiMbIndex ) { return Err::m_nERR; }
  ErrVal initMbForDecoding( MbDataAccess*& rpcMbDataAccess, UInt uiMbIndex ) { return Err::m_nERR; }

  ErrVal initSliceForCoding   ( const SliceHeader& rcSH );
  ErrVal initSliceForReading  ( const SliceHeader& rcSH ) { return Err::m_nERR; }
  ErrVal initSliceForDecoding ( const SliceHeader& rcSH ) { return Err::m_nERR; }
  ErrVal initSliceForFiltering( const SliceHeader& rcSH );

  ErrVal initMbForCoding      ( MbDataAccess& rcMbDataAccess, UInt uiMbIndex );
  ErrVal initMbForDecoding    ( MbDataAccess& rcMbDataAccess, UInt uiMbIndex ) { return Err::m_nERR; };
  ErrVal initMbForFiltering   ( MbDataAccess& rcMbDataAccess, UInt uiMbIndex );

protected:
  FrameMng*               m_pcFrameMng;
  MCTFEncoder*            m_apcMCTFEncoder          [MAX_LAYERS];
  SliceEncoder*           m_pcSliceEncoder;
  ControlMngH264AVCEncoder*  m_pcControlMng;
  BitWriteBuffer*         m_pcBitWriteBuffer;
  BitCounter*             m_pcBitCounter;
  NalUnitEncoder*         m_pcNalUnitEncoder;
  UvlcWriter*             m_pcUvlcWriter;
  UvlcWriter*             m_pcUvlcTester;
  MbCoder*                m_pcMbCoder;
  LoopFilter*             m_pcLoopFilter;
  MbEncoder*              m_pcMbEncoder;
  Transform*              m_pcTransform;
  IntraPredictionSearch*  m_pcIntraPrediction;
  YuvBufferCtrl*          m_apcYuvFullPelBufferCtrl [MAX_LAYERS];
  YuvBufferCtrl*          m_apcYuvHalfPelBufferCtrl [MAX_LAYERS];
  QuarterPelFilter*       m_pcQuarterPelFilter;
  CodingParameter*        m_pcCodingParameter;
  ParameterSetMng*        m_pcParameterSetMng;
  PocCalculator*          m_apcPocCalculator        [MAX_LAYERS];
  SampleWeighting*        m_pcSampleWeighting;
  CabacWriter*            m_pcCabacWriter;
  XDistortion*            m_pcXDistortion;
  MotionEstimation*       m_pcMotionEstimation;
  RateDistortion*         m_pcRateDistortion;

  MbDataCtrl*             m_pcMbDataCtrl;
  MbSymbolWriteIf*        m_pcMbSymbolWriteIf;
  UInt                    m_auiMbXinFrame           [MAX_LAYERS];
  UInt                    m_auiMbYinFrame           [MAX_LAYERS];
  UInt                    m_uiCurrLayer;
  Bool                    m_bLayer0IsAVC;
};


H264AVC_NAMESPACE_END


#endif // !defined(AFX_CONTROLMNGH264AVCENCODER_H__CE22E161_6ACE_4E6F_9E1B_08F0A99F9742__INCLUDED_)
