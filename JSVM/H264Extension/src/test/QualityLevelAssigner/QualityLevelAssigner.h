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


#ifndef _QUALITY_LEVEL_ASSIGNER_H
#define _QUALITY_LEVEL_ASSIGNER_H


#include "Typedefs.h"

#include "ReadBitstreamFile.h"  //bug-fix suffix
#include "H264AVCDecoderLib.h"
#include "CreaterH264AVCDecoder.h"
#include "H264AVCEncoderLib.h"
#include "QualityLevelEstimation.h"
#include "../src/lib/H264AVCEncoderLib/BitCounter.h"
#include "../src/lib/H264AVCEncoderLib/BitWriteBuffer.h"
#include "../src/lib/H264AVCEncoderLib/UvlcWriter.h"
#include "../src/lib/H264AVCEncoderLib/NalUnitEncoder.h"

class ReadBitstreamFile;
class WriteBitstreamToFile;
class ReadYuvFile;
class QualityLevelParameter;



class QualityLevelAssigner
{
public:
  QualityLevelAssigner();
  ~QualityLevelAssigner();

  static ErrVal   create  ( QualityLevelAssigner*&  rpcQualityLevelAssigner );
  ErrVal          destroy ();
  ErrVal          init    ( QualityLevelParameter*  pcQualityLevelParameter );
  ErrVal          go      ();

protected:
  //====== initialization ======
  ErrVal          xInitStreamParameters       ();

  //====== picture buffer and NAL unit handling ======
  ErrVal          xGetNewPicBuffer            ( PicBuffer*&           rpcPicBuffer,
                                                UInt                  uiSize );
  ErrVal          xRemovePicBuffer            ( PicBufferList&        rcPicBufferUnusedList );
  ErrVal          xClearPicBufferLists        ();
  ErrVal          xGetNextValidPacket         ( BinData*&             rpcBinData,
                                                ReadBitstreamFile*    pcReadBitStream,
                                                UInt                  uiTopLayer,
                                                UInt                  uiLayer,
                                                UInt                  uiFGSLayer,
                                                UInt                  uiLevel,
                                                Bool                  bIndependent,
                                                Bool&                 rbEOS,
                                                UInt*                 auiFrameNum );

  //====== get rate and distortion ======
  ErrVal          xInitRateAndDistortion      (Bool bMultiLayer);
  ErrVal          xInitRateValues             ();
  ErrVal          xInitDistortion             ( UInt*                 auiDistortion,
                                                UInt                  uiTopLayer,
                                                UInt                  uiLayer,
                                                UInt                  uiFGSLayer,
                                                UInt                  uiLevel      = MSYS_UINT_MAX,
                                                Bool                  bIndependent = false );
  ErrVal          xGetDistortion              ( UInt&                 ruiDistortion,
                                                const UChar*          pucReconstruction,
                                                const UChar*          pucReference,
                                                UInt                  uiHeight,
                                                UInt                  uiWidth,
                                                UInt                  uiStride );

  //====== read from and write to data file =====
  ErrVal          xWriteDataFile              ( const std::string&    cFileName );
  ErrVal          xReadDataFile               ( const std::string&    cFileName );

  //====== determine and write quality id's =====
  ErrVal          xDetermineQualityIDs        ();
  ErrVal          xWriteQualityLayerStreamPID ();
  ErrVal          xWriteQualityLayerStreamSEI ();
	//SEI changes update {
  //ErrVal          xInsertQualityLayerSEI      ( WriteBitstreamToFile* pcWriteBitStream,
  //                                              UInt                  uiLayer,
  //                                              UInt                  uiFrameNum );
	  ErrVal          xInsertPriorityLevelSEI      ( WriteBitstreamToFile* pcWriteBitStream,
                                                UInt                  uiLayer,
                                                UInt                  uiFrameNum );
	//SEI changes update }
  //JVT-S043
  ErrVal          xDetermineMultiLayerQualityIDs        ();
  //SEI changes update {
	//ErrVal          xInsertMultiLayerQualityLayerSEI ( WriteBitstreamToFile* pcWriteBitStream,
 //                                                    UInt                  uiLayer,
 //                                                    UInt                  uiFrameNum );
	ErrVal          xInsertMultiLayerPriorityLevelSEI ( WriteBitstreamToFile* pcWriteBitStream,
                                                     UInt                  uiLayer,
                                                     UInt                  uiFrameNum );
	//SEI changes update }
private:
  QualityLevelParameter*        m_pcParameter;
  h264::H264AVCPacketAnalyzer*  m_pcH264AVCPacketAnalyzer;
  h264::CreaterH264AVCDecoder*  m_pcH264AVCDecoder;
    //bug-fix suffix{{
  ReadBitstreamIf*            m_pcReadBitstream;
  //bug-fix suffix}}
  // for SEI writing
  h264::BitCounter*             m_pcBitCounter;
  h264::BitWriteBuffer*         m_pcBitWriteBuffer;
  h264::UvlcWriter*             m_pcUvlcWriter;
  h264::UvlcWriter*             m_pcUvlcTester;
  h264::NalUnitEncoder*         m_pcNalUnitEncoder;

  Bool                          m_bOutputReconstructions;
  UInt                          m_uiNumLayers;
  UInt                          m_auiNumFGSLayers     [MAX_LAYERS];
  UInt                          m_auiNumFrames        [MAX_LAYERS];
  UInt                          m_auiGOPSize          [MAX_LAYERS];
  UInt                          m_auiNumTempLevel     [MAX_LAYERS];
  UInt                          m_auiFrameWidth       [MAX_LAYERS];
  UInt                          m_auiFrameHeight      [MAX_LAYERS];
  UInt                          m_auiSPSRequired      [32];
  UInt                          m_auiSubsetSPSRequired[32];
  UInt                          m_auiPPSRequired      [256];
  Double*                       m_aaadDeltaDist       [MAX_LAYERS][MAX_QUALITY_LEVELS];
  UInt*                         m_aaauiPacketSize     [MAX_LAYERS][MAX_QUALITY_LEVELS];
  UInt*                         m_aaauiQualityID      [MAX_LAYERS][MAX_QUALITY_LEVELS];

  PicBufferList                 m_cActivePicBufferList;
  PicBufferList                 m_cUnusedPicBufferList;

  UChar                         m_aucStartCodeBuffer[5];
  BinData                       m_cBinDataStartCode;
};



#endif // _QUALITY_LEVEL_ASSIGNER_H
