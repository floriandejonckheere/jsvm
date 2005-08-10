/*
********************************************************************************

NOTE - One of the two copyright statements below may be chosen
       that applies for the software.

********************************************************************************
This software module was originally developed by

CAMMAS Nathalie (France Télécom)

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

To the extent that France Télécom owns patent rights that would be required to
make, use, or sell the originally developed software module or portions thereof
included in the ISO/IEC 14496-10:2005 Amd.1 (Scalable Video Coding) in a
conforming product, France Télécom will assure the ISO/IEC that it is willing
to negotiate licenses under reasonable and non-discriminatory terms and
conditions with applicants throughout the world.

France Télécom retains full right to modify and use the code for its own
purpose, assign or donate the code to a third party and to inhibit third
parties from using the code for products that do not conform to MPEG-related
ITU Recommendations and/or ISO/IEC International Standards.

This copyright notice must be included in all copies or derivative works.
Copyright (c) ISO/IEC 2005.

********************************************************************************

COPYRIGHT AND WARRANTY INFORMATION

Copyright 2005, International Telecommunications Union, Geneva

The France Télécom hereby donate this source code to the ITU, with the following
understanding:
    1. France Télécom retain the right to do whatever they wish with the
       contributed source code, without limit.
    2. France Télécom retain full patent rights (if any exist) in the technical
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
#include "Typedefs.h"

#include "H264AVCDecoderLib.h"
#include "CreaterH264AVCDecoder.h"
#include "H264AVCEncoderLib.h"
#include "CreaterH264AVCEncoder.h"
#include "RatePointsManager.h"

class ReadBitstreamFile;
class WriteBitstreamToFile;
class EncoderMergerParameter;
class NalUnitEncoder;

class EncoderMerger
{
protected:
	EncoderMerger();
	virtual ~EncoderMerger();

public:
  static ErrVal create              ( EncoderMerger*&         rpcEncoderMerger );
  ErrVal        init                ( EncoderMergerParameter* pcEncoderMergerParameter );
  ErrVal        destroy             ();
  ErrVal        go();

  ErrVal        go_DS();
  ErrVal        go_QL();

  //Dead substream insertion
  ErrVal countNumOfNAL (UInt uiLayer, UInt &uiNumSkip);
  ErrVal WriteDeadSubstream(UInt uiLayer, UInt uiNFrames);
  ErrVal AnalyseBitstream(UInt uiAnalyzedLayer);
  Void	calculateMaxRate();
  Void WriteDiscardableFlag(BinData*  pcBinData);
  ErrVal MergeAndAddDSInfo(UInt *uiNumSkip );
  

  //Quality levels insertion
  ErrVal        AnalyseBitstream_QL            ();
  Void ReadFGSRateAndDistoFile(UInt uiLayer, std::string & FGSRateFilename,
	  std::string & DistoFilename, UInt uiExtLevel);
  Void addPacket( UInt                    uiNumBytes,
                      UInt                    uiLayer,
                      UInt                    uiLevel,
                      UInt                    uiFGSLayer,
                      Bool                    bNewPicture );
  Void PrintSizeFrames(UInt uiLayer, UInt uiNbFrames);
  Void CalculateQualityLevel(UInt uiExtLayer);
  ErrVal addQualityLevel();
  ErrVal writeQualityLevel(UInt uiLayer, UInt uiNFrames);
  
protected:
  ReadBitstreamFile*              m_pcReadBitstream[MAX_LAYERS];
  EncoderMergerParameter*           m_pcEncoderMergerParameter;
  h264::H264AVCPacketAnalyzer*  m_pcH264AVCPacketAnalyzer;
  h264::CreaterH264AVCEncoder*  m_pcH264AVCEncoder;
  Int m_uiNumOfLayer;
  WriteBitstreamToFile*     m_pcWriteBitstreamToFile;
  
  UChar                         m_aucStartCodeBuffer[5];
  BinData                       m_cBinDataStartCode;

  UInt						m_auiNumFrameAtLayer[MAX_LAYERS];

  //Dead substreams insertion
  UInt						m_aaaauiRate[MAX_LAYERS][MAX_LAYERS][MAX_NBFRAMES][MAX_FGS_LAYERS];
  UInt						m_aauiMaxRate[MAX_LAYERS][MAX_NBFRAMES];
  
  //Quality levels insertion
  Double  m_aaadDisto[MAX_LAYERS][MAX_NBFRAMES][MAX_NUM_RD_LEVELS]; // distorsion points for each frame and each layer
  Double  m_aaadFGSRate[MAX_LAYERS][MAX_NBFRAMES][MAX_NUM_RD_LEVELS];//corresponding rate points 
  Double m_aadWeight[MAX_LAYERS][MAX_NBFRAMES];
  Double m_aadByteForFrame[MAX_NBFRAMES][MAX_LAYERS];
  Double m_aaadByteForFrameFGS[MAX_NBFRAMES][MAX_LAYERS][MAX_FGS_LAYERS+1];
  Double m_aadTargetByteForFrame[MAX_NBFRAMES][MAX_LAYERS];
  RatePointManager  *rpm[MAX_NBFRAMES][MAX_LAYERS];
  UInt m_aauiNbPoints[MAX_LAYERS][MAX_NBFRAMES]; //number of R/D distorsion points per each spatial frame
  Int m_aaiLevel[MAX_LAYERS][MAX_NBFRAMES]; //temporal level of each frames
  Double m_dQualityLevelMax[MAX_LAYERS];
  Double m_dQualityLevelMin[MAX_LAYERS];

  Double m_dQualityLevelMinGlobal;
  Double m_dQualityLevelMaxGlobal;


};

