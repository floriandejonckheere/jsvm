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



#if !defined(AFX_CREATERH264AVCDECODER_H__0366BFA9_45D9_4834_B404_8DE3914C1E58__INCLUDED_)
#define AFX_CREATERH264AVCDECODER_H__0366BFA9_45D9_4834_B404_8DE3914C1E58__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "H264AVCCommonLib/Sei.h"


class H264AVCDecoder;
class ControlMngH264AVCDecoder;
class SliceReader;
class SliceDecoder;
class UvlcReader;
class MbParser;
class MbDecoder;
class NalUnitParser;
class BitReadBuffer;
class CabacReader;
class CabaDecoder;

class MbData;
class Frame;
class FrameMng;
class LoopFilter;
class Transform;
class IntraPrediction;
class MotionCompensation;
class YuvBufferCtrl;
class QuarterPelFilter;
class SampleWeighting;
class ParameterSetMng;
class PocCalculator;

class DecodedPicBuffer;
class MCTFDecoder;
class ReconstructionBypass;
class RQFGSDecoder;


H264AVC_NAMESPACE_BEGIN

class H264AVCDECODERLIB_API CreaterH264AVCDecoder
{
protected:
	CreaterH264AVCDecoder();
	virtual ~CreaterH264AVCDecoder();

public:
  static ErrVal create( CreaterH264AVCDecoder*& rpcCreaterH264AVCDecoder );

  ErrVal destroy    ();
  ErrVal init       ();
  ErrVal uninit     ();
  ErrVal process    ( PicBuffer*        pcPicBuffer,
                      PicBufferList&    rcPicBufferOutputList,
                      PicBufferList&    rcPicBufferUnusedList,
                      PicBufferList&    rcPicBufferReleaseList );
  ErrVal initPacket ( BinDataAccessor*  pcBinDataAccessor,
                      UInt&             ruiNalUnitType,
                      UInt&             uiMbX,
                      UInt&             uiMbY,
                      UInt&             uiSize );
  ErrVal  checkSliceLayerDependency ( BinDataAccessor*  pcBinDataAccessor,
                                      Bool&             bFinishChecking );

protected:
  ErrVal xCreateDecoder();

protected:
  H264AVCDecoder*         m_pcH264AVCDecoder;
  RQFGSDecoder*           m_pcRQFGSDecoder;
  DecodedPicBuffer*       m_apcDecodedPicBuffer     [MAX_LAYERS];
  MCTFDecoder*            m_apcMCTFDecoder          [MAX_LAYERS];
  FrameMng*               m_pcFrameMng;
  ParameterSetMng*        m_pcParameterSetMng;
  PocCalculator*          m_apcPocCalculator        [MAX_LAYERS];
  SliceReader*            m_pcSliceReader;
  NalUnitParser*          m_pcNalUnitParser;
  SliceDecoder*           m_pcSliceDecoder;
  ControlMngH264AVCDecoder*  m_pcControlMng;
  BitReadBuffer*          m_pcBitReadBuffer;
  UvlcReader*             m_pcUvlcReader;
  MbParser*               m_pcMbParser;
  LoopFilter*             m_pcLoopFilter;
  MbDecoder*              m_pcMbDecoder;
  Transform*              m_pcTransform;
  IntraPrediction*        m_pcIntraPrediction;
  MotionCompensation*     m_pcMotionCompensation;
  YuvBufferCtrl*          m_apcYuvFullPelBufferCtrl [MAX_LAYERS];
  QuarterPelFilter*       m_pcQuarterPelFilter;
  CabacReader*            m_pcCabacReader;
  SampleWeighting*        m_pcSampleWeighting;
  ReconstructionBypass*   m_pcReconstructionBypass;
};







struct PacketDescription
{
  Bool  ParameterSet;
  Bool  Scalable;
  UInt  Layer;
  UInt  Level;
  UInt  FGSLayer;
  Bool  ApplyToNext;
  UInt  NalUnitType; 
  UInt  SPSid;
  UInt  PPSid;
  UInt  SPSidRefByPPS[256];
  //{{Quality level estimation and modified truncation- JVTO044 and m12007
  //France Telecom R&D-(nathalie.cammas@francetelecom.com)
  UInt auiDeltaBytesRateOfLevelQL[MAX_NUM_RD_LEVELS];
  UInt auiQualityLevelQL[MAX_NUM_RD_LEVELS];
  UInt uiNumLevelsQL;
  UInt  MaxRateDS;
  //}}Quality level estimation and modified truncation- JVTO044 and m12007
};



class H264AVCDECODERLIB_API H264AVCPacketAnalyzer
{
protected:
	H264AVCPacketAnalyzer();
	virtual ~H264AVCPacketAnalyzer();

public:
  static ErrVal create  ( H264AVCPacketAnalyzer*&  rpcH264AVCPacketAnalyzer );
  ErrVal        destroy ();
  ErrVal        init    ();
  ErrVal        uninit  ();
  ErrVal        process ( BinData*              pcBinData,
                          PacketDescription&    rcPacketDescription,
                          SEI::SEIMessage*&     pcScalableSEIMessage );

protected:
  ErrVal        xCreate ();

protected:
  BitReadBuffer*    m_pcBitReadBuffer;
  UvlcReader*       m_pcUvlcReader;
  NalUnitParser*    m_pcNalUnitParser;
  UInt              m_auiDecompositionStages[MAX_LAYERS];
  UInt              m_uiStdAVCOffset;

  UInt              m_uiTemporalLevelList[1 << PRI_ID_BITS];
  UInt              m_uiDependencyIdList [1 << PRI_ID_BITS];
  UInt              m_uiQualityLevelList [1 << PRI_ID_BITS];

};



H264AVC_NAMESPACE_END


#endif // !defined(AFX_CREATERH264AVCDECODER_H__0366BFA9_45D9_4834_B404_8DE3914C1E58__INCLUDED_)
