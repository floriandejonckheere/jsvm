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




#ifndef __EXTRACTOR_H_D65BE9B4_A8DA_11D3_AFE7_005004464B79
#define __EXTRACTOR_H_D65BE9B4_A8DA_11D3_AFE7_005004464B79


#define MAX_PACKET_SIZE 1000000


#include "H264AVCCommonLib/Sei.h"

#include "ReadBitstreamFile.h"
#include "WriteBitstreamToFile.h"
#include "ExtractorParameter.h"


enum NalUnitType
{
  NAL_UNIT_EXTERNAL                 = 0,
  NAL_UNIT_CODED_SLICE              = 1,
  NAL_UNIT_CODED_SLICE_DATAPART_A   = 2,
  NAL_UNIT_CODED_SLICE_DATAPART_B   = 3,
  NAL_UNIT_CODED_SLICE_DATAPART_C   = 4,
  NAL_UNIT_CODED_SLICE_IDR          = 5,
  NAL_UNIT_SEI                      = 6,
  NAL_UNIT_SPS                      = 7,
  NAL_UNIT_PPS                      = 8,
  NAL_UNIT_ACCESS_UNIT_DELIMITER    = 9,
  NAL_UNIT_END_OF_SEQUENCE          = 10,
  NAL_UNIT_END_OF_STREAM            = 11,
  NAL_UNIT_FILLER_DATA              = 12,

  NAL_UNIT_CODED_SLICE_SCALABLE     = 20,
  NAL_UNIT_CODED_SLICE_IDR_SCALABLE = 21
};


class ScalableStreamDescription
{
public:
  ScalableStreamDescription   ();
  ~ScalableStreamDescription  ();

  ErrVal  init      ( h264::SEI::ScalableSei* pcScalableSei );
  ErrVal  uninit    ();
  ErrVal  addPacket ( UInt                    uiNumBytes,
                      UInt                    uiLayer,
                      UInt                    uiLevel,
                      UInt                    uiFGSLayer,
                      Bool                    bNewPicture );
  ErrVal  analyse   ();
  Void    output    ( FILE*                   pFile );

  UInt    getNumberOfLayers ()                  const { return m_uiNumLayers; }
  UInt    getNumOfScalableLayers()              const { return m_uiScalableNumLayersMinus1 + 1; }
  UInt    getNumberOfScalableLayers ( UInt uiLayer, UInt uiTL, UInt uiQL ) const { return m_aaauiScalableLayerId[uiLayer][uiTL][uiQL]; }
  UInt    getBitrateOfScalableLayers( UInt uiScalableLayer ) const { return m_auiBitrate[uiScalableLayer]; }
  UInt    getDependencyId           ( UInt uiScalableLayer ) const { return m_auiDependencyId[uiScalableLayer]; }
  UInt    getTempLevel              ( UInt uiScalableLayer ) const { return m_auiTempLevel[uiScalableLayer]; }
  UInt    getFGSLevel               ( UInt uiScalableLayer ) const { return m_auiQualityLevel[uiScalableLayer]; }
	UInt    getFrmWidth               ( UInt uiScalableLayer ) const { return m_auiFrmWidth[uiScalableLayer]; }
	UInt    getFrmHeight              ( UInt uiScalableLayer ) const { return m_auiFrmHeight[uiScalableLayer]; }
	Double  getFrameRate              ( UInt uiScalableLayer ) const { return m_adFramerate[uiScalableLayer]; }
	Bool    getBaseLayerModeAVC       ()                       const { return m_bAVCBaseLayer;                }
#if 1 //BUG_FIX liuhui 0603
  UInt    getStdAVCOffset           ()                       const { return m_uiStdAVCOffset;                }
  UInt    getScalableLayer  ( UInt uiLayer, UInt uiTL, UInt uiQL ) 
	                                            const { return m_aaauiScalableLayerId[uiLayer][uiTL][uiQL]; }
#endif
	Void    setBaseLayerMode          ( Bool bAVCCompatible  )       { m_bAVCBaseLayer = bAVCCompatible;      }
  UInt    getFrameWidth     ( UInt uiLayer )    const { return m_auiFrameWidth  [uiLayer]; }
  UInt    getFrameHeight    ( UInt uiLayer )    const { return m_auiFrameHeight [uiLayer]; }
  
  UInt    getMaxLevel       ( UInt uiLayer )    const { return m_auiDecStages   [uiLayer]; }
  Double  getFrameRateUnit  ()                  const { return (Double)m_uiFrameRateUnitNom/(Double)m_uiFrameRateUnitDenom; }
  UInt    getNumPictures    ( UInt uiLayer,
                              UInt uiLevel )    const { return m_aauiNumPictures[uiLayer][uiLevel]; }
  UInt64  getNALUBytes      ( UInt uiLayer,
                              UInt uiLevel,
                              UInt uiFGS   )    const { return m_aaaui64NumNALUBytes[uiLayer][uiLevel][uiFGS]; }
  //add France Telecom
  Double getFrameRate(UInt uiExtLayer, UInt uiLevel) { return m_aaadFramerate[uiExtLayer][uiLevel][0];}
  //~add France Telecom
  Bool    m_bSPSRequired[MAX_LAYERS][32];
  Bool    m_bPPSRequired[MAX_LAYERS][256];



private:
  Bool    m_bInit;
  Bool    m_bAnalyzed;
  
  UInt    m_uiNumLayers;
  Bool    m_bAVCBaseLayer;
  UInt    m_uiAVCTempResStages;
  UInt    m_uiFrameRateUnitDenom;
  UInt    m_uiFrameRateUnitNom;
  UInt    m_uiMaxDecStages;
  UInt    m_auiFrameWidth       [MAX_LAYERS];
  UInt    m_auiFrameHeight      [MAX_LAYERS];
  UInt    m_auiDecStages        [MAX_LAYERS];

  UInt64  m_aaaui64NumNALUBytes [MAX_LAYERS][MAX_DSTAGES+1][MAX_QUALITY_LEVELS];
  UInt64  m_aaui64BaseLayerBytes[MAX_LAYERS][MAX_DSTAGES+1];
  UInt64  m_aaui64FGSLayerBytes [MAX_LAYERS][MAX_DSTAGES+1];
  UInt    m_aauiNumPictures     [MAX_LAYERS][MAX_DSTAGES+1];
#if 1 //BUG_FIX liuhui 0603
  UInt    m_uiStdAVCOffset;
#endif

  UInt    m_uiScalableNumLayersMinus1;
  UInt    m_auiBitrate              [MAX_LAYERS*MAX_TEMP_LEVELS*MAX_QUALITY_LEVELS];
  UInt    m_auiTempLevel            [MAX_LAYERS*MAX_TEMP_LEVELS*MAX_QUALITY_LEVELS];
  UInt    m_auiDependencyId         [MAX_LAYERS*MAX_TEMP_LEVELS*MAX_QUALITY_LEVELS];
  UInt    m_auiQualityLevel         [MAX_LAYERS*MAX_TEMP_LEVELS*MAX_QUALITY_LEVELS];
  Double  m_adFramerate             [MAX_LAYERS*MAX_TEMP_LEVELS*MAX_QUALITY_LEVELS];
  UInt    m_auiFrmWidth             [MAX_LAYERS*MAX_TEMP_LEVELS*MAX_QUALITY_LEVELS];
  UInt    m_auiFrmHeight            [MAX_LAYERS*MAX_TEMP_LEVELS*MAX_QUALITY_LEVELS];
  UInt    m_aaauiScalableLayerId    [MAX_LAYERS][MAX_TEMP_LEVELS][MAX_QUALITY_LEVELS];
  UInt    m_aaauiBitrate            [MAX_LAYERS][MAX_TEMP_LEVELS][MAX_QUALITY_LEVELS];
  UInt    m_aaauiTempLevel          [MAX_LAYERS][MAX_TEMP_LEVELS][MAX_QUALITY_LEVELS];
  UInt    m_aaauiDependencyId       [MAX_LAYERS][MAX_TEMP_LEVELS][MAX_QUALITY_LEVELS];
  UInt    m_aaauiQualityLevel       [MAX_LAYERS][MAX_TEMP_LEVELS][MAX_QUALITY_LEVELS];
  Double  m_aaadFramerate           [MAX_LAYERS][MAX_TEMP_LEVELS][MAX_QUALITY_LEVELS];
  UInt    m_aaauiFrmWidth           [MAX_LAYERS][MAX_TEMP_LEVELS][MAX_QUALITY_LEVELS];
  UInt    m_aaauiFrmHeight          [MAX_LAYERS][MAX_TEMP_LEVELS][MAX_QUALITY_LEVELS];
};



class Extractor  
{
protected:
	Extractor();
	virtual ~Extractor();

public:
  static ErrVal create              ( Extractor*&         rpcExtractor );
  ErrVal        init                ( ExtractorParameter* pcExtractorParameter );
  ErrVal        go                  ();
  ErrVal        destroy             ();
  ErrVal        xWriteScalableSEIToBuffer( h264::SEI::ScalableSei* pcScalableSei, BinData* pcBinData );

  //{{Quality level estimation and modified truncation- JVTO044 and m12007
  //France Telecom R&D-(nathalie.cammas@francetelecom.com)
  //if there is dead substream in the input bitstream but no R/D information
  ErrVal        go_DS                  ();
  //if there is R/D information (with or without Dead substreams)
  ErrVal        go_QL                  ();
  //}}Quality level estimation and modified truncation- JVTO044 and m12007

protected:
  ErrVal        xAnalyse            ();
  ErrVal        xPrimaryAnalyse            ();
  ErrVal        xSetParameters      ();
  ErrVal        xExtractPoints      ();
  ErrVal        xExtractLayerLevel  ();
#if 1 //BUG_FIX liuhui 0603
  Void          setBaseLayerAVCCompatible( Bool bAVCCompatible ) { m_bAVCCompatible = bAVCCompatible; }
  Bool          getBaseLayerAVCCompatible() const { return m_bAVCCompatible; }
  ErrVal        xChangeScalableSEIMesssage( BinData *pcBinData, h264::SEI::SEIMessage* pcScalableSEIMessage,
				  UInt uiKeepScalableLayer, UInt& uiWantedScalableLayer, UInt& uiMaxLayer, UInt& uiMaxTempLevel, Double& dMaxFGSLayer, UInt uiMaxBitrate);
#else
	ErrVal        xChangeScalableSEIMesssage( BinData *pcBinData, h264::SEI::SEIMessage* pcScalableSEIMessage, 
                  UInt uiKeepScalableLayer,UInt& uiWantedScalableLayer, UInt& uiMaxLayer, UInt& uiMaxTempLevel, UInt& uiMaxFGSLayer, UInt uiMaxBitrate);
#endif
  // HS: packet trace
  ErrVal        xReadLineExtractTrace ( Char*               pcFormatString,
                                        UInt*               puiStart,
                                        UInt*               puiLength );
  ErrVal        xExtractTrace         ();
  //{{Quality level estimation and modified truncation- JVTO044 and m12007
  //France Telecom R&D-(nathalie.cammas@francetelecom.com)
  ErrVal        xSetParameters_DS      ();
  ErrVal        xExtractPoints_DS      ();
  //initialize temporal level of a frame
  Void setLevel(UInt                    uiLayer,
                      UInt                    uiLevel,
					  UInt					  uiNumImage);
  //intialize max rate for a frame from SEI dead substream information
  Void setMaxRateDS(UInt                    uiNumBytes,
						 UInt				  uiMaxRate,
                      UInt                    uiLayer,
                      UInt                    uiFGSLayer,
					  UInt					  uiNumImage,
					  Bool					  bMaxRate );
  //count size of packets for each frame
  Void addPacket(UInt                    uiLayer,
                      UInt                    uiLevel,
                      UInt                    uiFGSLayer,
                      UInt                    uiNumBytes);
  //set if dead substream must be kept or thrown away (from command line argument)
  Void setExtractDeadSubstream(Bool b, UInt ui) { m_bExtractDeadSubstream[ui] = b;}
  //calculate the size of the dead substream
  Void CalculateSizeDeadSubstream();
  //determine layer, level and target rate for output stream
  Void GetExtParameters();
  //search optimal quality for target rate
  ErrVal QualityLevelSearch();
  //extract NALs given optimal quality
  ErrVal ExtractPointsFromRate();
  //get total rate for a given quality 
  Double GetRateForQualityLevel(double QualityLevel, UInt uiMaxLayers, UInt uiExtLevel, UInt uiExtLayer);
  //intialize R/D arrays from SEI information
  Void setQualityLevel();
  //get image rate for a given quality
  Double GetRateForQualityLevel(UInt uiLayer, UInt uiNumImage, Double QualityLevel,UInt uiExtLayer);
  //Calculate max rate for each frame of a layer (in case of dead substreams use of SEI information
  // from dead substreams)
  Void CalculateMaxRate(UInt uiLayer);
  //}}Quality level estimation and modified truncation- JVTO044 and m12007

  UInt getPIDIndex(UInt uiPID);
  UInt addPIDToTable(UInt uiPID);
  Double GetTruncatedRate(Double dQL, UInt uilevel, UInt uiLayer);
  UInt GetNearestPIDForQualityLevel(UInt uiLayer, UInt uiNumImage, Double QualityLevel);
  Double GetRateForQualityLevel(UInt uiLayer, UInt uiNumImage, Double QualityLevel,UInt uiExtLayer,Double dRatio);
  Double Extractor::CalculateSizeOfIncludedLayers(UInt uiExtLayer, UInt uiExtLevel);
  Bool IsFrameToCut(UInt uiFrame);
  Void AllocateAndInitializeDatas();

protected:
  ReadBitstreamIf*              m_pcReadBitstream;
  WriteBitstreamIf*             m_pcWriteBitstream;
  ExtractorParameter*           m_pcExtractorParameter;
  h264::H264AVCPacketAnalyzer*  m_pcH264AVCPacketAnalyzer;

  UChar                         m_aucStartCodeBuffer[5];
  BinData                       m_cBinDataStartCode;

  ScalableStreamDescription     m_cScalableStreamDescription;
  Double                        m_aadTargetSNRLayer[MAX_LAYERS][MAX_DSTAGES+1];

  // HS: packet trace
  FILE*                         m_pcTraceFile;
  FILE*                         m_pcExtractionTraceFile;
  LargeFile                     m_cLargeFile;
  UInt                          m_uiMaxSize;

#if 1 //BUG_FIX liuhui 0603
  Void    xOutput          ( ScalableStreamDescription& rcScalableStreamDescription, FILE* pFile);
  UInt    getScalableLayer ( UInt uiLayer, UInt uiTL, UInt uiQL ) const { return m_aaauiScalableLayerId[uiLayer][uiTL][uiQL]; }
  Bool                          m_bAVCCompatible;
  UInt                          m_uiScalableNumLayersMinus1;
  UInt                          m_aaauiScalableLayerId[MAX_LAYERS][MAX_TEMP_LEVELS][MAX_QUALITY_LEVELS];
  UInt                          m_auiDependencyId[MAX_SCALABLE_LAYERS];
  UInt                          m_auiTempLevel[MAX_SCALABLE_LAYERS];
  UInt                          m_auiQualityLevel[MAX_SCALABLE_LAYERS];
  UInt                          m_auiFrmWidth[MAX_SCALABLE_LAYERS];
  UInt                          m_auiFrmHeight[MAX_SCALABLE_LAYERS];
  Double                        m_adFramerate[MAX_SCALABLE_LAYERS];
  UInt                          m_auiDirDepLayer[MAX_SCALABLE_LAYERS];
  Double                        m_adFrameRate[MAX_TEMP_LEVELS];
  Double                        m_aadMinBitrate[MAX_LAYERS][MAX_TEMP_LEVELS]; 
  Double                        m_aaadSingleBitrate[MAX_LAYERS][MAX_TEMP_LEVELS][MAX_QUALITY_LEVELS];
  Double                        m_adTotalBitrate[MAX_SCALABLE_LAYERS];
#endif

  //{{Quality level estimation and modified truncation- JVTO044 and m12007
  //France Telecom R&D-(nathalie.cammas@francetelecom.com)
  Double*						m_aaadMaxRate[MAX_LAYERS]; //size of each frame for each layer without deadsubstream
  Double*                       m_aaadTargetBytesFGS[MAX_LAYERS][MAX_FGS_LAYERS+1]; //bytes to be extracted for each FGS layer for each frame 																							// at each layer		
  Int*							m_aaiLevelForFrame[MAX_LAYERS];//temporal level of each frame
  Double*                       m_aaadBytesForFrameFGS[MAX_LAYERS][MAX_FGS_LAYERS+1]; //size of each FGS layer for each frame at each layer
  Double						m_aaadMaxRateForLevel[MAX_LAYERS][MAX_DSTAGES+1]; //size of layer for each level without deadsubstream
  Bool							m_bExtractDeadSubstream[MAX_LAYERS]; //indicate if deadsubstream has to be removed (command line)
  UInt							m_aSizeDeadSubstream[MAX_LAYERS]; //size of deadsubstream for each layer
  Bool							m_bInInputStreamDS; //indicate if deadsubstream is in the input bitstream
  Bool							m_bInInputStreamQL;// indicate if RD informations are in the input bitstream
  Double*                       m_aadTargetByteForFrame[MAX_LAYERS];
  UInt*                         m_aaauiBytesForQualityLevel[MAX_LAYERS][MAX_NUM_RD_LEVELS];
  Double*                       m_aaadQualityLevel[MAX_LAYERS][MAX_NUM_RD_LEVELS];
  Int*                          m_aaiNumLevels[MAX_LAYERS];
  UInt                          m_auiNbImages[MAX_LAYERS];
  //}}Quality level estimation and modified truncation- JVTO044 and m12007
  UInt							m_uiExtractNonRequiredPics;
  UInt m_uiQualityLevel;
  UInt m_auiPID[64];
  UInt m_uiNbPID;
  Bool m_bQualityLevelInSEI; //indicates if QualityLayers are in SEI messages
};
class ExtractStop{};
#endif //__EXTRACTOR_H_D65BE9B4_A8DA_11D3_AFE7_005004464B79
