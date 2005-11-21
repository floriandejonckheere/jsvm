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
#include "EncoderMerger.h"

#include "ReadBitstreamFile.h"
#include "WriteBitstreamToFile.h"
#include "EncoderMergerParameter.h"
#include "H264AVCCommonLib.h"
#include "H264AVCCommonLib/CommonDefs.h"
#include <math.h>

using namespace h264;

#define equal(a,b)  (!stricmp((a),(b)))


EncoderMerger::EncoderMerger()
: m_pcWriteBitstreamToFile      ( 0 )
, m_pcEncoderMergerParameter  ( 0 )
, m_fPID(0)
{
}

EncoderMerger::~EncoderMerger()
{
}

ErrVal
EncoderMerger::create( EncoderMerger*& rpcEncoderMerger )
{
  rpcEncoderMerger = new EncoderMerger;
  ROT( NULL == rpcEncoderMerger );
  return Err::m_nOK;
}

ErrVal
EncoderMerger::init( EncoderMergerParameter *pcEncoderMergerParameter )
{
  ROT( NULL == pcEncoderMergerParameter );

  m_pcEncoderMergerParameter  = pcEncoderMergerParameter;
  
  m_uiNumOfLayer = pcEncoderMergerParameter->m_uiNumOfLayer;
#ifdef QL_CLOSEDLOOP
  if(pcEncoderMergerParameter->m_uiMode == 1)
  {
#endif
  RNOKS( ReadBitstreamFile::create( m_pcReadBitstream ) ); 
  RNOKS( m_pcReadBitstream->init( m_pcEncoderMergerParameter->getInFile() ));

  WriteBitstreamToFile*  pcWriteBitstreamFile;
  RNOKS( WriteBitstreamToFile::create( pcWriteBitstreamFile ) ); 
  RNOKS( pcWriteBitstreamFile->init( m_pcEncoderMergerParameter->getOutFile()/*, MAX_PACKET_SIZE*/ ) );  
  m_pcWriteBitstreamToFile = pcWriteBitstreamFile;
  
  RNOK( h264::H264AVCPacketAnalyzer::create( m_pcH264AVCPacketAnalyzer ) );
  
  RNOK( h264::CreaterH264AVCEncoder::create( m_pcH264AVCEncoder ) );
  RNOK( m_pcH264AVCEncoder->init(pcEncoderMergerParameter));
  m_aucStartCodeBuffer[0] = 0;
  m_aucStartCodeBuffer[1] = 0;
  m_aucStartCodeBuffer[2] = 0;
  m_aucStartCodeBuffer[3] = 1;
  m_cBinDataStartCode.reset();
  m_cBinDataStartCode.set( m_aucStartCodeBuffer, 4 );
#ifdef QL_CLOSEDLOOP
  }
#endif
  return Err::m_nOK;
}



ErrVal EncoderMerger::destroy()
{
	m_cBinDataStartCode.reset();

  if( NULL != m_pcH264AVCPacketAnalyzer )       
  {
	  RNOK( m_pcH264AVCPacketAnalyzer->destroy() );    
	  
  }

  if( NULL != m_pcH264AVCEncoder)
  { 
	 RNOK( m_pcH264AVCEncoder->uninit() );   
     RNOK( m_pcH264AVCEncoder->destroy());
  }

  if( NULL != m_pcReadBitstream )     
  {
	RNOK( m_pcReadBitstream->uninit() );  
	RNOK( m_pcReadBitstream->destroy() ); 
  }
 
  if( NULL != m_pcWriteBitstreamToFile )     
  {
    RNOK( m_pcWriteBitstreamToFile->uninit() );  
    RNOK( m_pcWriteBitstreamToFile->destroy() );  
  }
 
  if(m_fPID != NULL)
      m_fPID = NULL;
  delete this;

  UInt uiLayer, uiPoint, uiFGSLayer;
  for(uiLayer=0; uiLayer<MAX_LAYERS; uiLayer++)
  {
        delete []m_aadWeight[uiLayer];
        delete []m_aadByteForFrame[uiLayer];
     for(uiPoint = 0; uiPoint < MAX_NUM_RD_LEVELS; uiPoint++)
     {
         delete []m_aaadDisto[uiLayer][uiPoint];
         delete []m_aaadFGSRate[uiLayer][uiPoint];
      }
      for(uiFGSLayer=0; uiFGSLayer<MAX_FGS_LAYERS+1; uiFGSLayer++)
      {
         delete []m_aaadByteForFrameFGS[uiLayer][uiFGSLayer];
         delete []m_uiFGSIndex[uiLayer][uiFGSLayer];
         if(uiFGSLayer <MAX_FGS_LAYERS)
         {
            delete []m_uiSavedPID[uiLayer][uiFGSLayer];
         }
      }
  }

  return Err::m_nOK;
}

UInt ReadUChar(FILE *fFile)
{
  UChar ucTemp;
  fread(&ucTemp, sizeof(unsigned char), 1, fFile);
  return ucTemp;
}

void EncoderMerger::ComputeDeltaDisto(std::string& cOrig, UInt uiWidth, UInt uiHeight, 
                       UInt uiNumOfFrames, UInt uiGopSize, 
					   std::string& cRoot, UInt uiFGSLayer, UInt uiLevel, UInt uiLayer,
                       FILE *fOUT)
{
  UInt uiMaxLevel = 0;
  UInt uiN = uiGopSize;
  while(uiN != 1)
  {
      uiN /= 2;
      uiMaxLevel++;
  }
  
  char cPrevLayerFile[256];
  char cNextLayerFile[256];
  UInt uiPrevFGSLayer = (uiLevel>0  ? uiFGSLayer : uiFGSLayer-1);
  UInt uiPrevLevel = (uiLevel>0 ? uiLevel-1 : uiMaxLevel);
  sprintf(cPrevLayerFile, "%s-%d%d%d.yuv", cRoot.c_str(), uiLayer, uiPrevFGSLayer, uiPrevLevel);
  sprintf(cNextLayerFile, "%s-%d%d%d.yuv", cRoot.c_str(), uiLayer, uiFGSLayer, uiLevel);
  printf("Compute relative distortions between\n\t%s and %s\n", cPrevLayerFile, cNextLayerFile);

  FILE *fOrig = fopen(cOrig.c_str(), "rb");
  FILE *fPrevLayer = fopen(cPrevLayerFile, "rb");
  FILE *fNextLayer = fopen(cNextLayerFile, "rb");
  UInt uiChromaSize = (uiWidth*uiHeight)>>1;
  UInt uiLumaSize = uiWidth*uiHeight;

  UInt uiStepFrame = 2 << (uiMaxLevel-uiLevel);
  UInt uiOffsetFrame = uiGopSize>>uiLevel;
  UInt uiResetMod = uiGopSize>>(uiLevel-1);

  UInt uiFrame;
  UInt uiFrameToAddSSE = uiOffsetFrame;
  Bool bFrameToWrite=true;
  Double dCumulatedDeltaSSE = 0;
  for(uiFrame=0; uiFrame<uiNumOfFrames; uiFrame++)
  {
    Double dSSEPrev=0;
    Double dSSENext=0;
    if ((uiFrame!=0) && (uiFrame%uiResetMod == 0))
    {
      //printf("-- Cumulated disto for frame %d: %3.8f\n", uiFrameToAddSSE, dCumulatedDeltaSSE);
      fprintf(fOUT, "%4d %d %d %3.8f\n", uiFrameToAddSSE, uiFGSLayer, uiLayer, dCumulatedDeltaSSE);
      uiFrameToAddSSE += uiStepFrame;
      dCumulatedDeltaSSE = 0;
    }
    
    for(UInt i=0; i<uiLumaSize; i++)
    {
      UInt uiOrig = ReadUChar(fOrig);
      UInt uiPrev = ReadUChar(fPrevLayer);
      UInt uiNext = ReadUChar(fNextLayer);
      Int diff = uiPrev-uiOrig;
      dSSEPrev += diff*diff;
      diff = uiNext-uiOrig;
      dSSENext += diff*diff;
    }
#if 0
    Double dDeltaDisto = dSSENext-dSSEPrev;
#else
    Double dDeltaDisto = 10*(log(dSSEPrev)-log(dSSENext))/log(10.)/uiNumOfFrames;
#endif
    // skip chroma info
    fseek(fOrig, uiChromaSize, SEEK_CUR);
    fseek(fPrevLayer, uiChromaSize, SEEK_CUR);
    fseek(fNextLayer, uiChromaSize, SEEK_CUR);

    //printf("Frame %d: %9.f %9.f %3.8f\n", uiFrame, dSSEPrev, dSSENext, dDeltaDisto); 
    dCumulatedDeltaSSE += dDeltaDisto;
  }
  if (uiFrameToAddSSE < uiNumOfFrames)
  {
    //printf("-- Cumulated disto for frame %d: %3.8f\n", uiFrameToAddSSE, dCumulatedDeltaSSE);
    fprintf(fOUT, "%4d %d %d %3.8f\n", uiFrameToAddSSE, uiFGSLayer, uiLayer, dCumulatedDeltaSSE);
  }

  fclose(fOrig);
  fclose(fPrevLayer);
  fclose(fNextLayer);
}

void EncoderMerger::ComputeAllDisto(std::string& cOrig, UInt uiWidth, UInt uiHeight, 
                     UInt uiNumOfFrames, UInt uiGopSize, 
					 std::string& cRoot, UInt uiFGSLayer, UInt uiLayer, 
					 std::string& cOutFilename)
{
  UInt uiMaxLevel = 0;
  UInt uiN = uiGopSize;
  while(uiN != 1)
  {
      uiN /= 2;
      uiMaxLevel++;
  }

  FILE *fOut = fopen(cOutFilename.c_str(), "at");

  UInt uiLevel;
  // Question: start at level 0 or 1? 
  //  start at level 1, since we will consider that all level 0 should have same pid... (?)
  for(UInt uiFGS = 1; uiFGS <= uiFGSLayer; uiFGS++)
  {
      for(uiLevel=1; uiLevel<=uiMaxLevel; uiLevel++)
          {
             ComputeDeltaDisto(cOrig, uiWidth, uiHeight, uiNumOfFrames, uiGopSize, cRoot, 
                 uiFGS, uiLevel, uiLayer, fOut);
    }
  }
  fclose(fOut);
}

ErrVal EncoderMerger::EstimateQL()
{
	// parameter
  UInt uiGopSize;
  UInt uiNumOfFrames;
  UInt uiMaxLayer;
  UInt uiLayer;
  UInt uiMaxFGS;

  uiNumOfFrames = m_pcEncoderMergerParameter->m_uiNumOfFrames;
  uiGopSize = m_pcEncoderMergerParameter->m_uiGopSize;
  uiMaxLayer = m_pcEncoderMergerParameter->m_uiNumOfLayer;
  uiMaxFGS = m_pcEncoderMergerParameter->m_uiMaxFGS;
  m_uiMaxFGS = uiMaxFGS;
  UInt *auiExtraNumOfFrames = new UInt[uiMaxLayer+1];
  UInt *auiNumOfFrames = new UInt[uiMaxLayer+1];
  UInt *auiGopSize = new UInt[uiMaxLayer+1];
  auiNumOfFrames[uiMaxLayer] = uiNumOfFrames;
  UInt uiNumOfGop = ((uiNumOfFrames-2)/uiGopSize) + 1;
  auiExtraNumOfFrames[uiMaxLayer] = (uiNumOfGop*uiGopSize)+1;
  auiGopSize[uiMaxLayer] = uiGopSize;
  Int iLayer;
  if(uiMaxLayer!= 0)
  {
    for(iLayer = uiMaxLayer-1; iLayer>=0; iLayer--)
    {
        auiNumOfFrames[iLayer] = auiNumOfFrames[iLayer+1]/2;
        auiGopSize[iLayer] = auiGopSize[iLayer+1]/2;
        auiExtraNumOfFrames[iLayer] = (uiNumOfGop*auiGopSize[iLayer])+1;
    }
  }
  ppppRDTree aaaapcRDTrees;
  AllocateRDTrees(aaaapcRDTrees, uiMaxLayer, uiMaxFGS, auiExtraNumOfFrames);
  for(uiLayer = 0; uiLayer <= uiMaxLayer; uiLayer++)
  { 
      BuildRDHierarchy(aaaapcRDTrees, auiExtraNumOfFrames[uiLayer], auiGopSize[uiLayer], uiLayer, uiMaxFGS);
  }
  UInt **aaFrameOrder;
  typedef UInt * pUInt;
  aaFrameOrder = new pUInt[uiMaxLayer+1];
  for(uiLayer=0; uiLayer<=uiMaxLayer;uiLayer++)
  {
      aaFrameOrder[uiLayer] = new UInt[auiNumOfFrames[uiLayer]];
      GetFrameOrder(auiNumOfFrames[uiLayer], auiGopSize[uiLayer], aaFrameOrder[uiLayer]);
  }
    
  ReadDistoFromFile(m_pcEncoderMergerParameter->m_cDistoFilename, aaaapcRDTrees);
  ReadRateFromFile(m_pcEncoderMergerParameter->m_cFGSRateFilename, aaaapcRDTrees, aaFrameOrder,uiMaxLayer);

  // output information that have been read for debug
  UInt uiFGS, uiFrame;
  for(uiLayer=0; uiLayer<=uiMaxLayer;uiLayer++)
  {
      for(uiFrame=0; uiFrame<auiNumOfFrames[uiLayer]; uiFrame++)
      {
          for(uiFGS = 1; uiFGS <= uiMaxFGS; uiFGS++)
          {
              printf("Frame %4d DeltaRate: %6d DeltaDisto: %9.5f\n", uiFrame,
                  aaaapcRDTrees[uiLayer][uiFGS][uiFrame]->GetDeltaRate(),
                  aaaapcRDTrees[uiLayer][uiFGS][uiFrame]->GetDeltaDisto() );
          }
      }
  }

  UInt uiGop;
  for(uiGop=0; uiGop<uiNumOfGop; uiGop++)
  {
      for(uiLayer=0; uiLayer<=uiMaxLayer;uiLayer++)
      {
          UInt uiDeltaPos=auiGopSize[uiLayer];
          uiFrame = uiGop*auiGopSize[uiLayer]+uiDeltaPos;
          for(uiFGS = 1; uiFGS <= uiMaxFGS; uiFGS++)
          {
              aaaapcRDTrees[uiLayer][uiFGS][uiFrame]->RDOptim();
              aaaapcRDTrees[uiLayer][uiFGS][uiFrame]->HierarchyPrint();
              // merge in node associated to firt frame
              aaaapcRDTrees[uiLayer][uiFGS][0]->MergeRDLists(aaaapcRDTrees[uiLayer][uiFGS][uiFrame]->GetNextRDNode(), aaaapcRDTrees[uiLayer][uiFGS][0]->GetNextRDNode());
#ifdef DO_GLOBAL_SLOPE_UPDATE
              aaaapcRDTrees[uiLayer][uiFGS][0]->GetNextRDNode()->UpdateGlobalRDSlopeOfList();
#endif
              printf("Global RD list:\n");
              aaaapcRDTrees[uiLayer][uiFGS][0]->PrintRDList();
          }
      }
  }
    
  typedef UInt **ppUInt;
  typedef UInt *pUInt;
  UInt ***auiQLForFrames = new ppUInt[uiMaxLayer+1];
  UInt ***auiQLForFramesOrder = new ppUInt[uiMaxLayer+1];
  for(uiLayer = 0; uiLayer <= uiMaxLayer; uiLayer++)
  {
      auiQLForFrames[uiLayer] = new pUInt[uiMaxFGS+1];
	  auiQLForFramesOrder[uiLayer] = new pUInt[uiMaxFGS+1];
      for(uiFGS = 0; uiFGS <= uiMaxFGS; uiFGS++)
      {
          auiQLForFrames[uiLayer][uiFGS] = new UInt[auiExtraNumOfFrames[uiLayer]+1];
		  auiQLForFramesOrder[uiLayer][uiFGS] = new UInt[auiExtraNumOfFrames[uiLayer]+1];
          for(uiFrame=0; uiFrame<=auiExtraNumOfFrames[uiLayer]; uiFrame++)
              auiQLForFrames[uiLayer][uiFGS][uiFrame] = 123456;
      }
  }

  UInt uiMinQL=0;
  UInt uiMaxQL = 62; // 63 is kept for BL
  UInt uiStep = (62/((uiMaxLayer+1)*uiMaxFGS)+1);
  UInt uiPrevMinQL;
  uiMinQL = uiMaxQL-uiStep+1;
  for(uiLayer=0;uiLayer<=uiMaxLayer;uiLayer++)
  {
      for(uiFGS = 1; uiFGS <= uiMaxFGS; uiFGS++)
      {
          uiPrevMinQL = uiMinQL;
          SetQualityLayerForFrames(auiQLForFrames[uiLayer][uiFGS], auiExtraNumOfFrames[uiLayer], auiGopSize[uiLayer], 
              aaaapcRDTrees[uiLayer][uiFGS][0]->GetNextRDNode(), aaaapcRDTrees[uiLayer][uiFGS], 
              uiMinQL, uiMaxQL);
          printf("--- QL generated for frames:\n");
          for(uiFrame=0; uiFrame<auiNumOfFrames[uiLayer]; uiFrame++)
              printf("QL[%3d] = %3d\n", uiFrame, auiQLForFrames[uiLayer][uiFGS][uiFrame]);        
          uiMaxQL = uiPrevMinQL-1;
          Int iMin = uiMaxQL-uiStep+1;
          uiMinQL = (iMin >0 ? uiMaxQL-uiStep+1 : 0);
      }
  }

  UInt uiIndex;
  for(uiLayer=0; uiLayer<=uiMaxLayer; uiLayer++)
  {
	for(uiIndex=0; uiIndex<auiNumOfFrames[uiLayer]; uiIndex++)
      {
        auiQLForFramesOrder[uiLayer][0][uiIndex] = 63;
        for(uiFGS = 1; uiFGS <= uiMaxFGS; uiFGS++)
        {
            UInt uiFrame = aaFrameOrder[uiLayer][uiIndex];
            auiQLForFramesOrder[uiLayer][uiFGS][uiIndex] = auiQLForFrames[uiLayer][uiFGS][uiFrame];
        }
	  }
  }

  if(m_pcEncoderMergerParameter->getQLInSEI() == 1)
  {
      //QualityLayers are put in SEI messages
#ifdef QL_CLOSEDLOOP
      RNOK(addQualityLevel_SEI(auiQLForFramesOrder));
#endif
  }
  else
  {
      //Quality Layers are put in Priority_Id
#ifdef QL_CLOSEDLOOP
      RNOK(addQualityLevel_PID(auiQLForFramesOrder));
#endif
  }
  return Err::m_nOK;
}


ErrVal EncoderMerger::go()
{
#ifdef QL_CLOSEDLOOP
	if(m_pcEncoderMergerParameter->m_uiMode == 0)
	{
  UInt uiWidth = m_pcEncoderMergerParameter->m_uiWidth;
  UInt uiHeight = m_pcEncoderMergerParameter->m_uiHeight;
  UInt uiNumOfFrames = m_pcEncoderMergerParameter->m_uiNumOfFrames;
  UInt uiGopSize = m_pcEncoderMergerParameter->m_uiGopSize;
  UInt uiFGSLayer = m_pcEncoderMergerParameter->m_uiMaxFGS;
  UInt uiLayer = m_pcEncoderMergerParameter->m_uiLayer;
  
  ComputeAllDisto(m_pcEncoderMergerParameter->m_cOrig,uiWidth,uiHeight,uiNumOfFrames,uiGopSize,
	  m_pcEncoderMergerParameter->m_cRoot,uiFGSLayer,uiLayer,m_pcEncoderMergerParameter->m_cDistoFilename);

	}
	if(m_pcEncoderMergerParameter->m_uiMode == 1)
	{
        PrimaryAnalyse();
        EstimateQL();
	}
#else
	RNOK(go_QL());
#endif
	return Err::m_nOK;
}

void EncoderMerger::ReadDistoFromFile(std::string& cFile, ppppRDTree aaaapcRDTrees)
{
	FILE *fFile = fopen(cFile.c_str(), "rt");

  UInt uiFrame, uiFGSLayer, uiLayer;
  Double dDisto;
  while (!feof(fFile))
  {
    fscanf(fFile, "%d %d %d %lf", &uiFrame, &uiFGSLayer, &uiLayer, &dDisto);
    printf("reading Frame: %4d FGSLayer: %2d Layer:%d DeltaDisto: %f\n", uiFrame, uiFGSLayer, uiLayer, dDisto);
    aaaapcRDTrees[uiLayer][uiFGSLayer][uiFrame]->SetDeltaDisto(dDisto);
  }
  fclose(fFile);
}

Bool EncoderMerger::IsNewFrame(UInt uiLayerId, UInt uiLevelId, UInt uiFGSId, 
                UInt &ruiLastLayerId, UInt &ruiLastLevelId, UInt &ruiLastFGSId)
{
  Bool bNewFrame=false;

  if (uiLayerId<ruiLastLayerId)
    bNewFrame = true;
  else if (uiLevelId<ruiLastLevelId)
    bNewFrame = true;
  else if (uiFGSId<ruiLastFGSId)
    bNewFrame = true;

  ruiLastLayerId = uiLayerId;
  ruiLastLevelId = uiLevelId;
  ruiLastFGSId = uiFGSId;
  return bNewFrame;
}
void EncoderMerger::ReadRateFromFile(std::string& cFile, ppppRDTree aaaapcRDTrees, UInt **aaFrameOrder, UInt uiMaxLayer)
{
	FILE *fFile = fopen(cFile.c_str(), "rt");
  char cBuffLine[512];
  // skip first two lines
  fgets(cBuffLine, 511, fFile);
  fgets(cBuffLine, 511, fFile);
  UInt uiTimeFrame=0;
  Int* iIndexFrame = new Int[uiMaxLayer];
  UInt uiLayer,uiFGSLayer,uiFrame;
  for(uiLayer=0; uiLayer<=uiMaxLayer; uiLayer++)
  {
      iIndexFrame[uiLayer]= -1;
      for(uiFGSLayer = 0; uiFGSLayer < MAX_FGS_LAYERS; uiFGSLayer++)
      {
          m_uiRateForFrame[uiLayer][uiFGSLayer] = new UInt[m_auiNumFrameAtLayer[uiLayer]];
          for(uiFrame = 0; uiFrame < m_auiNumFrameAtLayer[uiLayer]; uiFrame++)
          {
              m_uiRateForFrame[uiLayer][uiFGSLayer][uiFrame] = 0;
          }
      }
  }
  UInt uiLastLayerId=1;
  UInt uiLastLevelId=0;
  UInt uiLastFGSId=0;
  
  while (!feof(fFile))
  {
    char cAdress[30], cPacketType[30], cDiscardable[30], cTruncatable[30];
    UInt uiLength, uiLayerId, uiLevelId, uiFGSId;
    fscanf(fFile, "%s %d %d %d %d %s %s %s", cAdress, &uiLength, &uiLayerId, &uiLevelId, &uiFGSId,
      cPacketType, cDiscardable, cTruncatable);
    if (equal(cPacketType, "SliceData"))
    {
      if (IsNewFrame(uiLayerId, uiLevelId, uiFGSId, uiLastLayerId, uiLastLevelId, uiLastFGSId))
      {
        iIndexFrame[uiLayerId]++;
        uiTimeFrame = aaFrameOrder[uiLayerId][iIndexFrame[uiLayerId]];
      }
      //printf("reading Length: %6d LayerId: %2d LevelId: %2d FGSId: %2d TimeFrame: %4d \n",
        //uiLength, uiLayerId, uiLevelId, uiFGSId, uiTimeFrame);

      aaaapcRDTrees[uiLayerId][uiFGSId][uiTimeFrame]->SetDeltaRate(uiLength);
      m_uiRateForFrame[uiLayerId][uiFGSId][iIndexFrame[uiLayerId]] = uiLength;
    }
  }
}

void EncoderMerger::AllocateRDTrees(ppppRDTree &raaaapcRDTrees, UInt uiMaxLayer, UInt uiMaxFGS, UInt* auiNumOfFrames)
{
  UInt uiLayer, uiFGS, uiFrame;
  raaaapcRDTrees = new pppRDTree[uiMaxLayer+1];
  for(uiLayer=0; uiLayer<=uiMaxLayer; uiLayer++)
  {
    raaaapcRDTrees[uiLayer] = new ppRDTree[uiMaxFGS+1];
    for(uiFGS=0; uiFGS<=uiMaxFGS; uiFGS++)
    {
      raaaapcRDTrees[uiLayer][uiFGS] = new pRDTree[auiNumOfFrames[uiLayer]];
      for(uiFrame=0; uiFrame<auiNumOfFrames[uiLayer]; uiFrame++)
      {
        raaaapcRDTrees[uiLayer][uiFGS][uiFrame] = new RDTree;
        raaaapcRDTrees[uiLayer][uiFGS][uiFrame]->SetFrameNum(uiFrame);
      }
    }
  }
}

void EncoderMerger::GetFrameOrder(UInt uiNumOfFrames, UInt uiGopSize, UInt *aFrameOrder)
{
  UInt auiNumOfFrameForLevel[6] = { 1, 1, 2, 4, 8, 16};
  UInt uiMaxLevel, uiLevel;
  UInt uiNFrames;
  UInt uiOrder;
  uiMaxLevel = 0;
  UInt uiN = uiGopSize;
  while(uiN != 1)
  {
      uiN /= 2;
      uiMaxLevel++;
  }
  
  // first intra frame
  uiOrder=0;
  uiNFrames=0;
  aFrameOrder[uiOrder++] = 0;

  while (uiNFrames<uiNumOfFrames)
  { // deal with a GOP
    UInt uiNGoodFrames=0;
    for(uiLevel=0; uiLevel<=uiMaxLevel; uiLevel++)
    {
      UInt uiStep = 2 << (uiMaxLevel-uiLevel);
      UInt uiOffset = uiGopSize>>uiLevel;
      UInt uiFrameAtLevel;
      for(uiFrameAtLevel=0; uiFrameAtLevel<auiNumOfFrameForLevel[uiLevel]; uiFrameAtLevel++)
      {
        UInt uiTrueTimeIndex = uiNFrames + uiOffset + uiFrameAtLevel*uiStep;
        if (uiTrueTimeIndex<uiNumOfFrames)
        {
          uiNGoodFrames++;
          aFrameOrder[uiOrder++] = uiTrueTimeIndex;
        }
      }
    }
    uiNFrames += uiGopSize;
  }
}

void EncoderMerger::BuildRDHierarchy(ppppRDTree aaaapcRDTrees, UInt uiNumOfFrames, UInt uiGopSize, UInt uiLayer, UInt uiMaxFGS)
{
  UInt auiNumOfFrameForLevel[7] = { 1, 1, 2, 4, 8, 16, 32};
  UInt uiMaxLevel, uiLevel;
  UInt uiNFrames=0;
  uiMaxLevel = 0;
  UInt uiN = uiGopSize;
  while(uiN != 1)
  {
      uiN /= 2;
      uiMaxLevel++;
  }

  // first intra frame: nothing to do
  while (uiNFrames<uiNumOfFrames)
  { // deal with a GOP
    UInt uiNGoodFrames=0;
    for(uiLevel=0; uiLevel<uiMaxLevel; uiLevel++)
    {
      UInt uiStep = 2 << (uiMaxLevel-uiLevel);
      UInt uiOffset = uiGopSize>>uiLevel;
      UInt uiFrameAtLevel;
      for(uiFrameAtLevel=0; uiFrameAtLevel<auiNumOfFrameForLevel[uiLevel]; uiFrameAtLevel++)
      {
        UInt uiTrueTimeIndex = uiNFrames + uiOffset + uiFrameAtLevel*uiStep;
        if (uiTrueTimeIndex<uiNumOfFrames)
        {
          UInt uiTimeChild1 = uiTrueTimeIndex - (uiStep>>2);
          UInt uiTimeChild2 = uiTrueTimeIndex + (uiStep>>2);
          uiNGoodFrames++;
          //UInt uiLayer=0;
          for(UInt uiFGS = 1; uiFGS <= uiMaxFGS; uiFGS++)
          {
              aaaapcRDTrees[uiLayer][uiFGS][uiTrueTimeIndex]->SetChild1(aaaapcRDTrees[uiLayer][uiFGS][uiTimeChild1]);
              if (uiLevel!=0) 
                aaaapcRDTrees[uiLayer][uiFGS][uiTrueTimeIndex]->SetChild2(aaaapcRDTrees[uiLayer][uiFGS][uiTimeChild2]);
          }
        }
      }
    }
    uiNFrames += uiGopSize;
  }

}


void EncoderMerger::GetRDProgress(RDTree *pcRDTree, char *cOutfile)
{
  RDTree *pcPos;
  FILE *fOUT = fopen(cOutfile, "wt");

  pcPos = pcRDTree;
  UInt uiSumDeltaRate = 0;
  Double dSumDeltaDisto = 0;
  while (pcPos)
  {
    uiSumDeltaRate += pcPos->GetDeltaRate();
    dSumDeltaDisto += pcPos->GetDeltaDisto();
    fprintf(fOUT, "%9d %9.5f\n", uiSumDeltaRate, dSumDeltaDisto);
    pcPos = pcPos->GetNextRDNode();
  }

  fclose(fOUT);
}

UInt EncoderMerger::GetQL(UInt uiCurrentRate, UInt uiMinRate, UInt uiMaxRate, UInt uiQLRate0, UInt uiQLRateMax)
{
//#define USE_LINEAR_QL
  UInt uiQL;
#ifdef USE_LINEAR_QL
  Double dScale = ((Double)uiQLRateMax-uiQLRate0)/uiMaxRate;
  uiQL = (UInt) (uiQLRate0 + uiCurrentRate*dScale + 0.5) ;
#else
  Double dx = log((Double)uiCurrentRate)-log((Double)uiMinRate);
  Double maxDx = log((Double)uiMaxRate)-log((Double)uiMinRate);
  Double dScale = ((Double)uiQLRateMax - uiQLRate0)/maxDx;
  uiQL = uiQLRate0 + ((UInt)  (dx*dScale + 0.5));
#endif
  return uiQL;
}

void EncoderMerger::SetQualityLayerForFrames(UInt *auiQLForFrames, UInt uiNumOfFrames, UInt uiGopSize, 
                              RDTree *pcRDTreeList, ppRDTree apcRDTree,
                              UInt uiMinQL, UInt uiMaxQL)
{
  UInt uiFrame;
  // Set Max QL to level 0 frame
  for(uiFrame=0; uiFrame<uiNumOfFrames; uiFrame+=uiGopSize)
  {
    auiQLForFrames[uiFrame] = uiMaxQL;
  }
  // compute  max rate
  UInt uiMaxRate=0;
  UInt uiMinRate = pcRDTreeList->GetDeltaRate();
  for(uiFrame=0; uiFrame<uiNumOfFrames; uiFrame++)
  {
    if (uiFrame%uiGopSize != 0)
      uiMaxRate += apcRDTree[uiFrame]->GetDeltaRate();
  }
  // set QL for RDTreeList elements
  UInt uiCurrentRate=0;
  RDTree *pcNode;
  for(pcNode=pcRDTreeList; pcNode!=0; pcNode=pcNode->GetNextRDNode())
  {
    uiCurrentRate+=pcNode->GetDeltaRate();
    UInt uiQLRate0 = uiMaxQL-1;
    UInt uiQLRateMax = uiMinQL;
    UInt uiQL = GetQL(uiCurrentRate, uiMinRate, uiMaxRate, uiQLRate0, uiQLRateMax);
    uiFrame = pcNode->GetFrameNum();
    auiQLForFrames[uiFrame] = uiQL;
    Double dPercentRate = (100.0*uiCurrentRate)/uiMaxRate;
    printf("setting QL=%3d for frame %3d (rate=%2.2f%%\n", uiQL, uiFrame, dPercentRate);
  }
}


ErrVal EncoderMerger::go_QL()
{
  PrimaryAnalyse();
  RNOK ( AnalyseBitstream_QL() );
  UInt uiExtLayer = m_pcEncoderMergerParameter->m_uiNumOfLayer;
  if(m_pcEncoderMergerParameter->m_bReadPID == true)
  {
	  //read PID already calculated
      ReadPID(m_pcEncoderMergerParameter->getPIDFilename());
  }
  else
  {
      //read disto in file to calculate PID
    ReadFGSRateAndDistoFile(m_pcEncoderMergerParameter->getDistoFilename());
    CalculateQualityLevel(uiExtLayer);
  }

  //RNOK( m_pcH264AVCEncoder->init( m_pcEncoderMergerParameter )); 

  if(m_pcEncoderMergerParameter->m_bWritePID == true)
  {
      m_fPID = fopen(m_pcEncoderMergerParameter->getPIDFilename().c_str(),"wt");
  }
  else
  {
      m_fPID = 0;
  }

  if(m_pcEncoderMergerParameter->getQLInSEI() == 1)
  {
      //QualityLayers are put in SEI messages
#ifndef QL_CLOSEDLOOP
      RNOK(addQualityLevel_SEI());
#endif
  }
  else
  {
      //Quality Layers are put in Priority_Id
#ifndef QL_CLOSEDLOOP
      RNOK(addQualityLevel_PID());
#endif
  }

  if(m_pcEncoderMergerParameter->m_bWritePID == true)
  { 
      fclose(m_fPID);
  }
  return Err::m_nOK;
}

ErrVal
EncoderMerger::PrimaryAnalyse()
{
  UInt                    uiLayer       = 0;
  UInt                    uiLevel       = 0;
  UInt                    uiFGSLayer    = 0;
  Bool                    bNewPicture   = false;
  Bool                    bApplyToNext  = false;
  Bool                    bEOS          = false;
  Bool                    bFirstPacket  = true;
  BinData*                pcBinData     = 0;
  h264::SEI::SEIMessage*  pcScalableSei = 0;
  h264::PacketDescription cPacketDescription;
  m_uiMaxLayer = 0;
  UInt uiNumFrames[MAX_LAYERS];

  for(uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++)
  {
	  uiNumFrames[uiLayer] = 0;
  }
 
  //========== initialize (scalable SEI message shall be the first packet of the stream) ===========
  {
    RNOK( m_pcH264AVCPacketAnalyzer->init() );
    //--- get first packet ---
    RNOK( m_pcReadBitstream->extractPacket( pcBinData, bEOS ) );
    ROT ( bEOS );
    //--- analyse packet ---
    RNOK( m_pcH264AVCPacketAnalyzer ->process( pcBinData, cPacketDescription, pcScalableSei ) );
    ROF ( pcScalableSei );
    //--- initialize stream description ----
    delete pcScalableSei;
    //---- set packet length ----
    while( pcBinData->data()[ pcBinData->size() - 1 ] == 0x00 )
    {
      RNOK( pcBinData->decreaseEndPos( 1 ) ); // remove zero at end
    }
  }


  while( ! bEOS )
  {
    //===== get packet =====
    RNOK( m_pcReadBitstream->extractPacket( pcBinData, bEOS ) );
    if( bEOS )
    {
      continue;
    }

    //===== get packet description =====
    RNOK( m_pcH264AVCPacketAnalyzer->process( pcBinData, cPacketDescription, pcScalableSei ) );
    ROT ( pcScalableSei );

    //===== set packet length =====
    while( pcBinData->data()[ pcBinData->size() - 1 ] == 0x00 )
    {
      RNOK( pcBinData->decreaseEndPos( 1 ) ); // remove zero at end
    }
    
    //==== get parameters =====
    UInt  uiPacketSize  = 4 + pcBinData->size();
    if( ! bApplyToNext )
    {
      uiLayer     = cPacketDescription.Layer;
      uiLevel     = cPacketDescription.Level;
      uiFGSLayer  = cPacketDescription.FGSLayer;
	  if(uiLayer > m_uiMaxLayer)
          m_uiMaxLayer = uiLayer;
    }
    bApplyToNext  = cPacketDescription.ApplyToNext;
    bNewPicture   = ( ! cPacketDescription.ParameterSet && ! cPacketDescription.ApplyToNext );
 
    //==== update stream description =====
   if(! cPacketDescription.ParameterSet)
	{
		//Data NAL unit
		if(uiFGSLayer == 0 && !bApplyToNext)
			uiNumFrames[uiLayer] ++;
	}
  }

  //----- reset input file -----
  RNOKS( static_cast<ReadBitstreamFile*>(m_pcReadBitstream)->uninit() );
  RNOKS( static_cast<ReadBitstreamFile*>(m_pcReadBitstream)->init  ( m_pcEncoderMergerParameter->getInFile() ) );  

  for(uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++)
  {
	  m_auiNumFrameAtLayer[uiLayer] = uiNumFrames[uiLayer];
  }

  return Err::m_nOK;
}

ErrVal
EncoderMerger::AnalyseBitstream_QL()
{
  UInt                    uiLayer       = 0;
  UInt                    uiLevel       = 0;
  UInt                    uiFGSLayer    = 0;
  Bool                    bNewPicture   = false;
  Bool                    bApplyToNext  = false;
  Bool                    bEOS          = false;
  Bool                    bFirstPacket  = true;
  BinData*                pcBinData     = 0;
  h264::SEI::SEIMessage*  pcScalableSei = 0;
  h264::PacketDescription cPacketDescription;
  m_uiMaxLayer = 0;
  UInt uiNumFrames[MAX_LAYERS];
  UInt uiFrame, uiPoint;
  for(uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++)
  {
	  uiNumFrames[uiLayer] = 0;
  }
  
    for(uiLayer=0; uiLayer<MAX_LAYERS; uiLayer++)
    {
        m_aadWeight[uiLayer] = new Double[m_auiNumFrameAtLayer[uiLayer]];
        m_aadByteForFrame[uiLayer] = new Double[m_auiNumFrameAtLayer[uiLayer]];
        for(uiPoint = 0; uiPoint < MAX_NUM_RD_LEVELS; uiPoint++)
        {
            m_aaadDisto[uiLayer][uiPoint] = new Double[m_auiNumFrameAtLayer[uiLayer]];
            m_aaadFGSRate[uiLayer][uiPoint] = new Double[m_auiNumFrameAtLayer[uiLayer]];
        }
        for(uiFGSLayer=0; uiFGSLayer<MAX_FGS_LAYERS+1; uiFGSLayer++)
        {
            m_aaadByteForFrameFGS[uiLayer][uiFGSLayer] = new Double[m_auiNumFrameAtLayer[uiLayer]];
            m_uiFGSIndex[uiLayer][uiFGSLayer] = new UInt[m_auiNumFrameAtLayer[uiLayer]];
            if(uiFGSLayer <MAX_FGS_LAYERS)
            {
                m_uiSavedPID[uiLayer][uiFGSLayer] = new UInt[m_auiNumFrameAtLayer[uiLayer]];
            }
        }

        for(uiFrame=0; uiFrame<m_auiNumFrameAtLayer[uiLayer]; uiFrame++)
        {
            for(uiFGSLayer=0; uiFGSLayer<MAX_FGS_LAYERS+1; uiFGSLayer++)
            {
                m_aaadByteForFrameFGS[uiLayer][uiFGSLayer][uiFrame] = 0;
            }
            m_aadByteForFrame[uiLayer][uiFrame] = 0;
        }
    }

  uiLayer = 0;
  uiFGSLayer = 0;

  //========== initialize (scalable SEI message shall be the first packet of the stream) ===========
  {
    RNOK( m_pcH264AVCPacketAnalyzer->init() );
    //--- get first packet ---
    RNOK( m_pcReadBitstream->extractPacket( pcBinData, bEOS ) );
    ROT ( bEOS );
    //--- analyse packet ---
    RNOK( m_pcH264AVCPacketAnalyzer ->process( pcBinData, cPacketDescription, pcScalableSei ) );
    ROF ( pcScalableSei );
    //--- initialize stream description ----
    delete pcScalableSei;
    //---- set packet length ----
    while( pcBinData->data()[ pcBinData->size() - 1 ] == 0x00 )
    {
      RNOK( pcBinData->decreaseEndPos( 1 ) ); // remove zero at end
    }
    //---- update stream description -----
    
  addPacket(4+pcBinData->size(), 0, 0, 0, false );
  }


  while( ! bEOS )
  {
    //===== get packet =====
    RNOK( m_pcReadBitstream->extractPacket( pcBinData, bEOS ) );
    if( bEOS )
    {
      continue;
    }

    //===== get packet description =====
    RNOK( m_pcH264AVCPacketAnalyzer->process( pcBinData, cPacketDescription, pcScalableSei ) );
    ROT ( pcScalableSei );

    //===== set packet length =====
    while( pcBinData->data()[ pcBinData->size() - 1 ] == 0x00 )
    {
      RNOK( pcBinData->decreaseEndPos( 1 ) ); // remove zero at end
    }
    
    //==== get parameters =====
    UInt  uiPacketSize  = 4 + pcBinData->size();
    if( ! bApplyToNext )
    {
      uiLayer     = cPacketDescription.Layer;
      uiLevel     = cPacketDescription.Level;
      uiFGSLayer  = cPacketDescription.FGSLayer;
	  if(uiLayer > m_uiMaxLayer)
          m_uiMaxLayer = uiLayer;
    }
    bApplyToNext  = cPacketDescription.ApplyToNext;
    bNewPicture   = ( ! cPacketDescription.ParameterSet && ! cPacketDescription.ApplyToNext );
 
    //==== update stream description =====
   if(! cPacketDescription.ParameterSet)
	{
		//Data NAL unit
		if(uiFGSLayer == 0 && !bApplyToNext)
			uiNumFrames[uiLayer] ++;
	}
    addPacket( uiPacketSize, uiLayer, uiLevel, uiFGSLayer, bNewPicture );
  }

  //----- reset input file -----
  RNOKS( static_cast<ReadBitstreamFile*>(m_pcReadBitstream)->uninit() );
  RNOKS( static_cast<ReadBitstreamFile*>(m_pcReadBitstream)->init  ( m_pcEncoderMergerParameter->getInFile() ) );  

  for(uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++)
  {
	  m_auiNumFrameAtLayer[uiLayer] = uiNumFrames[uiLayer];
  }

  return Err::m_nOK;
}


Void EncoderMerger::ReadFGSRateAndDistoFile(std::string & DistoFilename)
{

  FILE *fileDisto = ::fopen( DistoFilename.c_str(), "rt" );

  UInt uiNumFrame;
  UInt uiLayer;
  UInt uiPoint;

  UInt uiNbFrames[MAX_LAYERS];
  for(uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++)
      uiNbFrames[uiLayer] = 0;
    
	Int temp;
	Int temp2;
	Int temp3;

	while(!feof(fileDisto))
	{
      fscanf(fileDisto, "%d %d %d", &temp,&temp2,&temp3);
	  if(!feof(fileDisto))
	  {
	  uiNumFrame = temp;
	  uiLayer = temp2;
	  uiPoint = temp3;
      
	  fscanf(fileDisto, "%d", &temp2);
	  
	  m_aaadDisto[uiLayer][uiPoint][uiNumFrame] = temp2;
      if(temp2 == 0 && uiPoint != 0)
      {
        m_aaadDisto[uiLayer][uiPoint][uiNumFrame] = m_aaadDisto[uiLayer][uiPoint-1][uiNumFrame];
      }
	  //calculate refinement
	  Double dTargetFGS = 0.0;
	  
      Double dSum = 0.0;
      UInt uiFGS;
      for(uiFGS = 0; uiFGS <= MAX_FGS_LAYERS; uiFGS++)
      {
          if(uiPoint >= 4*uiFGS )
              dSum += m_aaadByteForFrameFGS[uiLayer][uiFGS][uiNumFrame];
          m_uiFGSIndex[uiLayer][uiFGS][uiNumFrame] = uiFGS*4;
      }
      Double dMod = uiPoint % 4;
      if(dMod != 0)
      {
          dSum += dMod/4*m_aaadByteForFrameFGS[uiLayer][uiPoint/4+1][uiNumFrame];
      }

      dTargetFGS = dSum;
      
    if(temp2 == 0 && uiPoint != 0)
    {
       m_aaadFGSRate[uiLayer][uiPoint][uiNumFrame] = m_aaadFGSRate[uiLayer][uiPoint-1][uiNumFrame]; 
    }
    else
        m_aaadFGSRate[uiLayer][uiPoint][uiNumFrame] = dTargetFGS;
        
	m_aadWeight[uiLayer][uiNumFrame] = sqrt(1/pow(2,uiLayer));
	
	printf("Frame %d Layer %d Point %d Disto %d\n",uiNumFrame,uiLayer,uiPoint,temp2);
    if(uiPoint == 1)
    {
        uiNbFrames[uiLayer] ++;
    }
      
	  }
	}
    
    FILE *saveDisto = fopen("saveDisto.txt","wt");
    UInt uiFrame;
    for(uiFrame = 0; uiFrame < uiNbFrames[0]; uiFrame++)
    {
        Int iTemp = 0;
        fprintf(saveDisto,"%d ", uiFrame);
        for(uiPoint = 1; uiPoint < 9; uiPoint++)
        {
            iTemp = m_aaadDisto[0][uiPoint][uiFrame]-m_aaadDisto[0][uiPoint-1][uiFrame];
            fprintf(saveDisto,"%d ", iTemp);
        }
        fprintf(saveDisto, "\n");
    }
}

Void EncoderMerger::ReadPID(std::string & PIDFilename)
{
  Char  acLine    [1000];
  FILE *file = ::fopen( PIDFilename.c_str(), "rt" );

  UInt uiNumFrame;
  UInt uiLayer;
  
  Int i,c;
	
  UInt uiPID, uiFGSLayer;
  uiNumFrame = 0;
  while(!feof(file))
	{
        for( i = 0; ( c = fgetc(file), ( c != '\t' && c != '\n' && c != EOF ) ); acLine[i++] = c );
      acLine[i] = '\0';
	  sscanf(acLine, "%d %d %d %d", &uiLayer,&uiNumFrame,&uiFGSLayer, &uiPID);
      m_uiSavedPID[uiLayer][uiFGSLayer][uiNumFrame] = uiPID;
   }

    fclose(file);
}

Void EncoderMerger::addPacket(  UInt                    uiNumBytes,
                      UInt                    uiLayer,
                      UInt                    uiLevel,
                      UInt                    uiFGSLayer,
                      Bool                    bNewPicture )
{
  static UInt auiNumImage[MAX_LAYERS][MAX_FGS_LAYERS] = 
  { 
    {0, 0, 0}, 
    {0, 0, 0}, 
    {0, 0, 0}
  };

  m_aadByteForFrame[uiLayer][auiNumImage[uiLayer][uiFGSLayer]] += uiNumBytes; 
  m_aaadByteForFrameFGS[uiLayer][uiFGSLayer][auiNumImage[uiLayer][uiFGSLayer]] += uiNumBytes; 
  if (bNewPicture)
    auiNumImage[uiLayer][uiFGSLayer] ++;
}

Void EncoderMerger::PrintSizeFrames(UInt uiLayer, UInt uiNbFrames)
{
  Double sum=0;
  UInt uiFrame;
  for(uiFrame=0; uiFrame<uiNbFrames; uiFrame ++)
  {
    sum += m_aadByteForFrame[uiLayer][uiFrame];
  }
}

Void EncoderMerger::CalculateQualityLevel(UInt uiExtLayer)
{
  UInt uiNFrames;
  UInt uiNumPictures;
  UInt uiLayer;
  Int uiPoint;
  UInt uiMaxLayers = m_pcEncoderMergerParameter->m_uiNumOfLayer;

  Double QualityLevelMax[MAX_LAYERS] = {0,0,0};
  Double QualityLevelMin[MAX_LAYERS] = {100000000,100000000,100000000};
  typedef RatePointManager  * pRatePointManager;
  // initializing RD infos
  for(uiLayer = 0; uiLayer < uiMaxLayers; uiLayer++)
  {
	uiNumPictures = m_auiNumFrameAtLayer[uiLayer];
    rpm[uiLayer] = new pRatePointManager[uiNumPictures];
    printf("Rate used for frame at layer %d\n", uiLayer);
	for(uiNFrames = 0; uiNFrames < uiNumPictures; uiNFrames++)
    {
		rpm[uiLayer][uiNFrames] = new RatePointManager();
        rpm[uiLayer][uiNFrames]->Reset();
        printf("RD points for Frame %d, uiLayer %d\n", uiNFrames, uiLayer);
        for(uiPoint = 0; uiPoint < MAX_NUM_RD_LEVELS; uiPoint++)
        {
          rpm[uiLayer][uiNFrames]->PushPoint( m_aaadFGSRate[uiLayer][uiPoint][uiNFrames], 
			  m_aadWeight[uiLayer][uiNFrames]*m_aaadDisto[uiLayer][uiPoint][uiNFrames]);;
        }

        rpm[uiLayer][uiNFrames]->SetValidPoints(m_uiFGSIndex[uiLayer], uiNFrames);

        rpm[uiLayer][uiNFrames]->PrintValidPoints();
		rpm[uiLayer][uiNFrames]->SetMaxRate(m_aadByteForFrame[uiLayer][uiNFrames]);

		for(UInt ui = 1; ui < rpm[uiLayer][uiNFrames]->getNbValidPoints(); ui++)
		{
			Double QualityLevel = rpm[uiLayer][uiNFrames]->getValidQualityLevel(ui);
			if(QualityLevel > QualityLevelMax[uiLayer]) QualityLevelMax[uiLayer] = QualityLevel;
			if(QualityLevel < QualityLevelMin[uiLayer] && QualityLevel != 0) QualityLevelMin[uiLayer] = QualityLevel;
		}
        printf("------\n");
	}
  }
  
  m_dQualityLevelMaxGlobal = 0;
  m_dQualityLevelMinGlobal = 10000000;
  for(uiLayer = 0; uiLayer < uiMaxLayers; uiLayer++)
  {
	printf("QualityLevelMin %f QualityLevelMax %f \n ", QualityLevelMin[uiLayer], QualityLevelMax[uiLayer]);
	m_dQualityLevelMax[uiLayer] = log(QualityLevelMax[uiLayer]);
	m_dQualityLevelMin[uiLayer] = log(QualityLevelMin[uiLayer]);
	if(m_dQualityLevelMax[uiLayer] > m_dQualityLevelMaxGlobal) m_dQualityLevelMaxGlobal = m_dQualityLevelMax[uiLayer];
    if(m_dQualityLevelMin[uiLayer] < m_dQualityLevelMinGlobal) m_dQualityLevelMinGlobal = m_dQualityLevelMin[uiLayer];
    
  }

}
#ifdef QL_CLOSEDLOOP
ErrVal EncoderMerger::addQualityLevel_SEI(UInt ***auiQLForFrames)
{

  UInt                    uiLayer       = 0;
  UInt                    uiLevel       = 0;
  UInt                    uiFGSLayer    = 0;
  Bool                    bNewPicture   = false;
  Bool                    bApplyToNext  = false;
  Bool                    bEOS          = false;
  Bool                    bFirstPacket  = true;
  BinData*                pcBinData     = 0;
  h264::SEI::SEIMessage*  pcScalableSei = 0;
  h264::PacketDescription cPacketDescription;

  UInt                    uiLayerForSEI = 0;
  UInt                    uiLevelPrev   = 0;
  UInt                    uiLayerPrev   = 0;

  UInt uiNumFrame[MAX_LAYERS];
  for(uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++)
  {
	  uiNumFrame[uiLayer] = 0;
  }
  uiLayer = 0;
  Bool bSEIPacket = true; 

  RNOK( m_pcH264AVCPacketAnalyzer->init() );

  while( ! bEOS )
  {
    //===== get packet =====
    RNOK( m_pcReadBitstream->extractPacket( pcBinData, bEOS ) );
    if( bEOS )
    {
      continue;
    }

    //===== get packet description =====
    RNOK( m_pcH264AVCPacketAnalyzer->process( pcBinData, cPacketDescription, pcScalableSei ) );
    
    //===== set packet length =====
    while( pcBinData->data()[ pcBinData->size() - 1 ] == 0x00 )
    {
      RNOK( pcBinData->decreaseEndPos( 1 ) ); // remove zero at end
    }
    
    //==== get parameters =====
    UInt  uiPacketSize  = 4 + pcBinData->size();
    if( ! bApplyToNext )
    {
      uiLayer     = cPacketDescription.Layer;
      uiLevel     = cPacketDescription.Level;
      uiFGSLayer  = cPacketDescription.FGSLayer;
    }
    bApplyToNext  = cPacketDescription.ApplyToNext;
    bNewPicture   = ( ! cPacketDescription.ParameterSet && ! cPacketDescription.ApplyToNext );
    
	if(bNewPicture && !bSEIPacket && uiFGSLayer == 0)
	{
        if(uiLayer == 0 ||
            (uiLayerPrev <= uiLayer && uiLevelPrev<uiLevel) ||
            (uiLayerPrev == uiLayer && uiLevelPrev == uiLevel) ||
			 (uiLayer < uiLayerPrev))
        {
            for(uiLayerForSEI = uiLayer; uiLayerForSEI <= m_pcEncoderMergerParameter->m_uiNumOfLayer; uiLayerForSEI++)
            {
                writeQualityLevel_SEI(auiQLForFrames, uiLayerForSEI,uiNumFrame[uiLayerForSEI]);
                uiNumFrame[uiLayerForSEI] ++;
            }
        }
	}

	if(bSEIPacket)
		bSEIPacket = false;

    RNOK( m_pcWriteBitstreamToFile->writePacket( &m_cBinDataStartCode ) );
	RNOK( m_pcWriteBitstreamToFile->writePacket( pcBinData ) );
	RNOK( m_pcReadBitstream->releasePacket( pcBinData ) ); 

    uiLayerPrev = uiLayer;
    uiLevelPrev = uiLevel;
  }

  //----- reset input file -----
  RNOKS( static_cast<ReadBitstreamFile*>(m_pcReadBitstream)->uninit() );
  RNOKS( static_cast<ReadBitstreamFile*>(m_pcReadBitstream)->init  ( m_pcEncoderMergerParameter->getInFile() ) );  

  return Err::m_nOK;
}
#else
ErrVal EncoderMerger::addQualityLevel_SEI()
{

  UInt                    uiLayer       = 0;
  UInt                    uiLevel       = 0;
  UInt                    uiFGSLayer    = 0;
  Bool                    bNewPicture   = false;
  Bool                    bApplyToNext  = false;
  Bool                    bEOS          = false;
  Bool                    bFirstPacket  = true;
  BinData*                pcBinData     = 0;
  h264::SEI::SEIMessage*  pcScalableSei = 0;
  h264::PacketDescription cPacketDescription;

  UInt                    uiLayerForSEI = 0;
  UInt                    uiLevelPrev   = 0;
  UInt                    uiLayerPrev   = 0;

  UInt uiNumFrame[MAX_LAYERS];
  for(uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++)
  {
	  uiNumFrame[uiLayer] = 0;
  }
  uiLayer = 0;
  Bool bSEIPacket = true; 

  RNOK( m_pcH264AVCPacketAnalyzer->init() );

  while( ! bEOS )
  {
    //===== get packet =====
    RNOK( m_pcReadBitstream->extractPacket( pcBinData, bEOS ) );
    if( bEOS )
    {
      continue;
    }

    //===== get packet description =====
    RNOK( m_pcH264AVCPacketAnalyzer->process( pcBinData, cPacketDescription, pcScalableSei ) );
    
    //===== set packet length =====
    while( pcBinData->data()[ pcBinData->size() - 1 ] == 0x00 )
    {
      RNOK( pcBinData->decreaseEndPos( 1 ) ); // remove zero at end
    }
    
    //==== get parameters =====
    UInt  uiPacketSize  = 4 + pcBinData->size();
    if( ! bApplyToNext )
    {
      uiLayer     = cPacketDescription.Layer;
      uiLevel     = cPacketDescription.Level;
      uiFGSLayer  = cPacketDescription.FGSLayer;
    }
    bApplyToNext  = cPacketDescription.ApplyToNext;
    bNewPicture   = ( ! cPacketDescription.ParameterSet && ! cPacketDescription.ApplyToNext );
    
	if(bNewPicture && !bSEIPacket && uiFGSLayer == 0)
	{
        if(uiLayer == 0 ||
            (uiLayerPrev <= uiLayer && uiLevelPrev<uiLevel) ||
            (uiLayerPrev == uiLayer && uiLevelPrev == uiLevel) ||
			 (uiLayer < uiLayerPrev))
        {
            for(uiLayerForSEI = uiLayer; uiLayerForSEI < m_pcEncoderMergerParameter->m_uiNumOfLayer; uiLayerForSEI++)
            {
                writeQualityLevel_SEI(uiLayerForSEI,uiNumFrame[uiLayerForSEI]);
                uiNumFrame[uiLayerForSEI] ++;
            }
        }
	}

	if(bSEIPacket)
		bSEIPacket = false;

    RNOK( m_pcWriteBitstreamToFile->writePacket( &m_cBinDataStartCode ) );
	RNOK( m_pcWriteBitstreamToFile->writePacket( pcBinData ) );
	RNOK( m_pcReadBitstream->releasePacket( pcBinData ) ); 

    uiLayerPrev = uiLayer;
    uiLevelPrev = uiLevel;
  }

  //----- reset input file -----
  RNOKS( static_cast<ReadBitstreamFile*>(m_pcReadBitstream)->uninit() );
  RNOKS( static_cast<ReadBitstreamFile*>(m_pcReadBitstream)->init  ( m_pcEncoderMergerParameter->getInFile() ) );  

  return Err::m_nOK;
}
#endif
#ifdef QL_CLOSEDLOOP
ErrVal EncoderMerger::addQualityLevel_PID(UInt ***auiQLForFrames)
{

  UInt                    uiLayer       = 0;
  UInt                    uiLevel       = 0;
  UInt                    uiFGSLayer    = 0;
  Bool                    bNewPicture   = false;
  Bool                    bApplyToNext  = false;
  Bool                    bEOS          = false;
  Bool                    bFirstPacket  = true;
  BinData*                pcBinData     = 0;
  h264::SEI::SEIMessage*  pcScalableSei = 0;
  h264::PacketDescription cPacketDescription;

  UInt                    uiLayerForSEI = 0;
  UInt                    uiLevelPrev   = 0;
  UInt                    uiLayerPrev   = 0;

  UInt uiNumFrame[MAX_LAYERS];
  for(uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++)
  {
	  uiNumFrame[uiLayer] = 0;
  }
  uiLayer = 0;
  Bool bSEIPacket = true; 

  RNOK( m_pcH264AVCPacketAnalyzer->init() );

  while( ! bEOS )
  {
    //===== get packet =====
    RNOK( m_pcReadBitstream->extractPacket( pcBinData, bEOS ) );
    if( bEOS )
    {
      continue;
    }

    //===== get packet description =====
    RNOK( m_pcH264AVCPacketAnalyzer->process( pcBinData, cPacketDescription, pcScalableSei ) );
    
    //===== set packet length =====
    while( pcBinData->data()[ pcBinData->size() - 1 ] == 0x00 )
    {
      RNOK( pcBinData->decreaseEndPos( 1 ) ); // remove zero at end
    }
    
    //==== get parameters =====
    UInt  uiPacketSize  = 4 + pcBinData->size();
    if( ! bApplyToNext )
    {
      uiLayer     = cPacketDescription.Layer;
      uiLevel     = cPacketDescription.Level;
      uiFGSLayer  = cPacketDescription.FGSLayer;
    }
    bApplyToNext  = cPacketDescription.ApplyToNext;
    bNewPicture   = ( ! cPacketDescription.ParameterSet && ! cPacketDescription.ApplyToNext );

    if(bNewPicture && !bSEIPacket && uiFGSLayer != 0)
    {
        writeQualityLevel_PID(auiQLForFrames, uiLayer,uiNumFrame[uiLayer]-1,uiFGSLayer,uiLevel,pcBinData); 
    }

    if(bNewPicture && !bSEIPacket && uiFGSLayer == 0)
    {   
      uiNumFrame[uiLayer]++;
    }

	if(bSEIPacket)
		bSEIPacket = false;

    RNOK( m_pcWriteBitstreamToFile->writePacket( &m_cBinDataStartCode ) );
	RNOK( m_pcWriteBitstreamToFile->writePacket( pcBinData ) );
	RNOK( m_pcReadBitstream->releasePacket( pcBinData ) ); 

    uiLayerPrev = uiLayer;
    uiLevelPrev = uiLevel;
  }

  //----- reset input file -----
  RNOKS( static_cast<ReadBitstreamFile*>(m_pcReadBitstream)->uninit() );
  RNOKS( static_cast<ReadBitstreamFile*>(m_pcReadBitstream)->init  ( m_pcEncoderMergerParameter->getInFile() ) );  

  return Err::m_nOK;
}
#else
ErrVal EncoderMerger::addQualityLevel_PID()
{

  UInt                    uiLayer       = 0;
  UInt                    uiLevel       = 0;
  UInt                    uiFGSLayer    = 0;
  Bool                    bNewPicture   = false;
  Bool                    bApplyToNext  = false;
  Bool                    bEOS          = false;
  Bool                    bFirstPacket  = true;
  BinData*                pcBinData     = 0;
  h264::SEI::SEIMessage*  pcScalableSei = 0;
  h264::PacketDescription cPacketDescription;

  UInt                    uiLayerForSEI = 0;
  UInt                    uiLevelPrev   = 0;
  UInt                    uiLayerPrev   = 0;

  UInt uiNumFrame[MAX_LAYERS];
  for(uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++)
  {
	  uiNumFrame[uiLayer] = 0;
  }
  uiLayer = 0;
  Bool bSEIPacket = true; 

  RNOK( m_pcH264AVCPacketAnalyzer->init() );

  while( ! bEOS )
  {
    //===== get packet =====
    RNOK( m_pcReadBitstream->extractPacket( pcBinData, bEOS ) );
    if( bEOS )
    {
      continue;
    }

    //===== get packet description =====
    RNOK( m_pcH264AVCPacketAnalyzer->process( pcBinData, cPacketDescription, pcScalableSei ) );
    
    //===== set packet length =====
    while( pcBinData->data()[ pcBinData->size() - 1 ] == 0x00 )
    {
      RNOK( pcBinData->decreaseEndPos( 1 ) ); // remove zero at end
    }
    
    //==== get parameters =====
    UInt  uiPacketSize  = 4 + pcBinData->size();
    if( ! bApplyToNext )
    {
      uiLayer     = cPacketDescription.Layer;
      uiLevel     = cPacketDescription.Level;
      uiFGSLayer  = cPacketDescription.FGSLayer;
    }
    bApplyToNext  = cPacketDescription.ApplyToNext;
    bNewPicture   = ( ! cPacketDescription.ParameterSet && ! cPacketDescription.ApplyToNext );

    if(bNewPicture && !bSEIPacket && uiFGSLayer != 0)
    {
        writeQualityLevel_PID(uiLayer,uiNumFrame[uiLayer]-1,uiFGSLayer,uiLevel,pcBinData); 
    }

    if(bNewPicture && !bSEIPacket && uiFGSLayer == 0)
    {   
      uiNumFrame[uiLayer]++;
    }

	if(bSEIPacket)
		bSEIPacket = false;

    RNOK( m_pcWriteBitstreamToFile->writePacket( &m_cBinDataStartCode ) );
	RNOK( m_pcWriteBitstreamToFile->writePacket( pcBinData ) );
	RNOK( m_pcReadBitstream->releasePacket( pcBinData ) ); 

    uiLayerPrev = uiLayer;
    uiLevelPrev = uiLevel;
  }

  //----- reset input file -----
  RNOKS( static_cast<ReadBitstreamFile*>(m_pcReadBitstream)->uninit() );
  RNOKS( static_cast<ReadBitstreamFile*>(m_pcReadBitstream)->init  ( m_pcEncoderMergerParameter->getInFile() ) );  

  return Err::m_nOK;
}
#endif
#ifdef QL_CLOSEDLOOP
ErrVal EncoderMerger::writeQualityLevel_PID(UInt *** auiQLForFrames, UInt uiLayer, UInt uiNFrames, UInt uiFGSLayer, 
                                        UInt uiLevel,BinData*  pcBinData)
{
	UChar ucByte = 1 + (auiQLForFrames[uiLayer][uiFGSLayer][uiNFrames]<<2);
	pcBinData->data()[1] = ucByte;
    
	return Err::m_nOK;
}

ErrVal EncoderMerger::writeQualityLevel_SEI(UInt ***auiQLForFrames, UInt uiLayer, UInt uiNFrames)
{
    UChar aucParameterSetBuffer[1000];
    UInt uiFGS;
    UInt uiRate[MAX_NUM_RD_LEVELS];
	UInt uiQualityLevel[MAX_NUM_RD_LEVELS];
	BinData cBinData;
    cBinData.reset();
    cBinData.set( aucParameterSetBuffer, 1000 );

    ExtBinDataAccessor cExtBinDataAccessor;
    cBinData.setMemAccessor( cExtBinDataAccessor );

    UInt uiNumLevels = m_uiMaxFGS+1;
    for(uiFGS = 0; uiFGS <= m_uiMaxFGS; uiFGS++)
    {
        uiRate[uiFGS] = m_uiRateForFrame[uiLayer][uiFGS][uiNFrames] ;
        uiQualityLevel[uiFGS] = auiQLForFrames[uiLayer][uiFGS][uiNFrames];
        printf("Layer %d Frame %d Rate %d QL %d\n",uiLayer,uiNFrames,uiRate[uiFGS],uiQualityLevel[uiFGS]);
    }

    m_pcH264AVCEncoder->writeQualityLevelInfosSEI(&cExtBinDataAccessor, uiQualityLevel,uiRate,uiNumLevels, uiLayer);
	
    RNOK( m_pcWriteBitstreamToFile->writePacket( &m_cBinDataStartCode ) );
    RNOK( m_pcWriteBitstreamToFile->writePacket( &cExtBinDataAccessor ) );
     
	cBinData.reset();

	return Err::m_nOK;
}
#else
ErrVal EncoderMerger::writeQualityLevel_PID(UInt uiLayer, UInt uiNFrames, UInt uiFGSLayer, 
                                        UInt uiLevel,BinData*  pcBinData)
{
	UChar ucByte = pcBinData->data()[1];

    if(m_pcEncoderMergerParameter->m_bReadPID == true)
    {
        ucByte = m_uiSavedPID[uiLayer][uiFGSLayer][uiNFrames];
        pcBinData->data()[1] = ucByte;
		printf("Write Simple PriorityId Layer %d Frame %d Level %d FGS %d PriorityId %d\n", uiLayer,
        uiNFrames, uiLevel, uiFGSLayer, ucByte);
    }
    else
    {
    Double QualityLevel = log(rpm[uiLayer][uiNFrames]->getValidQualityLevel(uiFGSLayer));
    Int iQualityLevelToCode;
    if(uiFGSLayer == 0)
    {
        iQualityLevelToCode = 63;
    }
    else
    {
        if(uiLayer ==0)
        {
            if(m_uiMaxLayer >1)
            {
                iQualityLevelToCode = (Int)(20*(QualityLevel-m_dQualityLevelMin[uiLayer]) / (m_dQualityLevelMax[uiLayer] - m_dQualityLevelMin[uiLayer])+42);
    	        iQualityLevelToCode = (iQualityLevelToCode < 42 ? 42: iQualityLevelToCode > 62 ? 62 : iQualityLevelToCode);
            }
            else
            {
                iQualityLevelToCode = (Int)(31*(QualityLevel-m_dQualityLevelMin[uiLayer]) / (m_dQualityLevelMax[uiLayer] - m_dQualityLevelMin[uiLayer])+31);
    	        iQualityLevelToCode = (iQualityLevelToCode < 31 ? 31: iQualityLevelToCode > 62 ? 62 : iQualityLevelToCode);
            }
        }
        if(uiLayer ==1)
        {
            if(m_uiMaxLayer >1)
            {
                iQualityLevelToCode = (Int)(20*(QualityLevel-m_dQualityLevelMin[uiLayer]) / (m_dQualityLevelMax[uiLayer] - m_dQualityLevelMin[uiLayer])+21);
    	        iQualityLevelToCode = (iQualityLevelToCode < 21 ? 21: iQualityLevelToCode > 41 ? 41 : iQualityLevelToCode);
            }
            else
            {
                iQualityLevelToCode = (Int)(30*(QualityLevel-m_dQualityLevelMin[uiLayer]) / (m_dQualityLevelMax[uiLayer] - m_dQualityLevelMin[uiLayer]));
        	    iQualityLevelToCode = (iQualityLevelToCode < 0 ? 0: iQualityLevelToCode > 30 ? 30 : iQualityLevelToCode);
            }
        }
        if(uiLayer ==2)
        {
            iQualityLevelToCode = (Int)(20*(QualityLevel-m_dQualityLevelMin[uiLayer]) / (m_dQualityLevelMax[uiLayer] - m_dQualityLevelMin[uiLayer]));
    	    iQualityLevelToCode = (iQualityLevelToCode < 0 ? 0: iQualityLevelToCode > 20 ? 20 : iQualityLevelToCode);
        }

    }
		

    UInt uiQL = iQualityLevelToCode;

    uiQL <<= 2;
    ucByte = uiQL | ucByte;
	pcBinData->data()[1] = ucByte;
	ucByte = 0;
	ucByte = pcBinData->data()[1];
	printf("Write Simple PriorityId Layer %d Frame %d Level %d FGS %d QL %d PriorityId %d\n", uiLayer,
        uiNFrames, uiLevel, uiFGSLayer,iQualityLevelToCode, ucByte);

}

    if(m_pcEncoderMergerParameter->m_bWritePID == true)
    {
        fprintf(m_fPID, "%d %d %d %d\n", uiLayer, uiNFrames, uiFGSLayer, ucByte);
    }

	return Err::m_nOK;
}

ErrVal EncoderMerger::writeQualityLevel_SEI(UInt uiLayer, UInt uiNFrames)
{
	UChar aucParameterSetBuffer[1000];

	BinData cBinData;
    cBinData.reset();
    cBinData.set( aucParameterSetBuffer, 1000 );

    ExtBinDataAccessor cExtBinDataAccessor;
    cBinData.setMemAccessor( cExtBinDataAccessor );

	UInt uiNumLevels = rpm[uiLayer][uiNFrames]->getNbValidPoints();
	printf("NumLevels %d: \n",uiNumLevels);
	UInt uiRate[MAX_NUM_RD_LEVELS];
	UInt uiQualityLevel[MAX_NUM_RD_LEVELS];
	UInt uiRateOld = (UInt)floor(rpm[uiLayer][uiNFrames]->getValidRate(0));
	UInt ui;
	
	UInt uiMaxLayers = m_pcEncoderMergerParameter->m_uiNumOfLayer;
	for(ui = 0; ui < uiNumLevels; ui++)
	{	
		if(ui == 0)
		{
			uiRate[ui] = (UInt)floor(rpm[uiLayer][uiNFrames]->getValidRate(ui));
			uiRateOld = (UInt)floor(rpm[uiLayer][uiNFrames]->getValidRate(ui));
		}
		else
		{
            uiRate[ui] = (UInt)floor(rpm[uiLayer][uiNFrames]->getValidRate(ui) ) - uiRateOld;
            uiRateOld = (UInt)floor(rpm[uiLayer][uiNFrames]->getValidRate(ui) );
		}
		
		Double QualityLevel;
		Int iQualityLevelToCode = 0;
		if(ui == 0)
		{
			//first slope is infinite
			iQualityLevelToCode = 63;//255;
		}
		else
		{
		QualityLevel = log(rpm[uiLayer][uiNFrames]->getValidQualityLevel(ui));
		if(uiLayer ==0)
        {
            if(m_uiMaxLayer >1)
            {
                iQualityLevelToCode = (Int)(20*(QualityLevel-m_dQualityLevelMin[uiLayer]) / (m_dQualityLevelMax[uiLayer] - m_dQualityLevelMin[uiLayer])+42);
    	        iQualityLevelToCode = (iQualityLevelToCode < 42 ? 42: iQualityLevelToCode > 62 ? 62 : iQualityLevelToCode);
            }
            else
            {
                iQualityLevelToCode = (Int)(31*(QualityLevel-m_dQualityLevelMin[uiLayer]) / (m_dQualityLevelMax[uiLayer] - m_dQualityLevelMin[uiLayer])+31);
    	        iQualityLevelToCode = (iQualityLevelToCode < 31 ? 31: iQualityLevelToCode > 62 ? 62 : iQualityLevelToCode);
            }
        }
        if(uiLayer ==1)
        {
            if(m_uiMaxLayer >1)
            {
                iQualityLevelToCode = (Int)(20*(QualityLevel-m_dQualityLevelMin[uiLayer]) / (m_dQualityLevelMax[uiLayer] - m_dQualityLevelMin[uiLayer])+21);
    	        iQualityLevelToCode = (iQualityLevelToCode < 21 ? 21: iQualityLevelToCode > 41 ? 41 : iQualityLevelToCode);
            }
            else
            {
                iQualityLevelToCode = (Int)(30*(QualityLevel-m_dQualityLevelMin[uiLayer]) / (m_dQualityLevelMax[uiLayer] - m_dQualityLevelMin[uiLayer]));
        	    iQualityLevelToCode = (iQualityLevelToCode < 0 ? 0: iQualityLevelToCode > 30 ? 30 : iQualityLevelToCode);
            }
        }
        if(uiLayer ==2)
        {
            iQualityLevelToCode = (Int)(20*(QualityLevel-m_dQualityLevelMin[uiLayer]) / (m_dQualityLevelMax[uiLayer] - m_dQualityLevelMin[uiLayer]));
    	    iQualityLevelToCode = (iQualityLevelToCode < 0 ? 0: iQualityLevelToCode > 20 ? 20 : iQualityLevelToCode);
        }
		}
		uiQualityLevel[ui] = iQualityLevelToCode;
		
	}
	m_pcH264AVCEncoder->writeQualityLevelInfosSEI(&cExtBinDataAccessor, uiQualityLevel,uiRate,uiNumLevels, uiLayer);
	
    RNOK( m_pcWriteBitstreamToFile->writePacket( &m_cBinDataStartCode ) );
    RNOK( m_pcWriteBitstreamToFile->writePacket( &cExtBinDataAccessor ) );
     
	cBinData.reset();

	return Err::m_nOK;
}
#endif
