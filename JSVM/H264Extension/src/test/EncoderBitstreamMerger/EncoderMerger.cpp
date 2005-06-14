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



EncoderMerger::EncoderMerger()
: m_pcWriteBitstreamToFile      ( 0 )
, m_pcEncoderMergerParameter  ( 0 )
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
	Int iLayer;
	UInt uiLayer;
	UInt uiFrame;
	UInt uiFGSLayer;

  ROT( NULL == pcEncoderMergerParameter );

  m_pcEncoderMergerParameter  = pcEncoderMergerParameter;
  
  m_uiNumOfLayer = pcEncoderMergerParameter->m_uiNumOfLayer;
    
  for(iLayer = 0; iLayer < m_uiNumOfLayer; iLayer++)
  {
	  if((m_pcEncoderMergerParameter->getInsertQL() && iLayer == 0) || m_pcEncoderMergerParameter->getInsertDS())
	 {
         RNOKS( ReadBitstreamFile::create( m_pcReadBitstream[iLayer] ) ); 
		 RNOKS( m_pcReadBitstream[iLayer]->init( m_pcEncoderMergerParameter->getInFile(iLayer) ));
	 }
  }

  WriteBitstreamToFile*  pcWriteBitstreamFile;
  RNOKS( WriteBitstreamToFile::create( pcWriteBitstreamFile ) ); 
  RNOKS( pcWriteBitstreamFile->init( m_pcEncoderMergerParameter->getOutFile()/*, MAX_PACKET_SIZE*/ ) );  
  m_pcWriteBitstreamToFile = pcWriteBitstreamFile;
  
  RNOK( h264::H264AVCPacketAnalyzer::create( m_pcH264AVCPacketAnalyzer ) );
  
  RNOK( h264::CreaterH264AVCEncoder::create( m_pcH264AVCEncoder ) );
  
  m_aucStartCodeBuffer[0] = 0;
  m_aucStartCodeBuffer[1] = 0;
  m_aucStartCodeBuffer[2] = 0;
  m_aucStartCodeBuffer[3] = 1;
  m_cBinDataStartCode.reset();
  m_cBinDataStartCode.set( m_aucStartCodeBuffer, 4 );

  for(iLayer = 0; iLayer < MAX_LAYERS;iLayer++)
  {
	  for(uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer ++)
	  {
		  for(uiFrame = 0; uiFrame < MAX_NBFRAMES; uiFrame++)
		  {
			  for( uiFGSLayer = 0; uiFGSLayer < MAX_FGS_LAYERS; uiFGSLayer ++)
			  {
				  m_aaaauiRate[iLayer][uiLayer][uiFrame][uiFGSLayer] = 0;
			  }
		  }
	  }
	  for(uiFrame = 0; uiFrame < MAX_NBFRAMES; uiFrame++)
	  {
		  m_aauiMaxRate[iLayer][uiFrame] = 0;
	  }
  }

  return Err::m_nOK;
}



ErrVal EncoderMerger::destroy()
{
	Int iLayer;

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

  for(iLayer = 0; iLayer < MAX_LAYERS; iLayer++)
  {

  if( NULL != m_pcReadBitstream[iLayer] )     
  {
	RNOK( m_pcReadBitstream[iLayer]->uninit() );  
	RNOK( m_pcReadBitstream[iLayer]->destroy() ); 
  }
  }

  if( NULL != m_pcWriteBitstreamToFile )     
  {
    RNOK( m_pcWriteBitstreamToFile->uninit() );  
    RNOK( m_pcWriteBitstreamToFile->destroy() );  
  }
 
  delete this;

  return Err::m_nOK;
}

ErrVal EncoderMerger::countNumOfNAL (UInt uiLayer, UInt &uiNumSkip )
{
  UInt  uiSkip = 0;
  Bool  bEOS          = false;
  
  
  RNOK( m_pcH264AVCPacketAnalyzer->init() );
  while( ! bEOS )
      {
       //=========== get packet ===========
	   BinData*  pcBinData;
       RNOK( m_pcReadBitstream[uiLayer]->extractPacket( pcBinData, bEOS ) );
       if( bEOS )
       {
         continue;
       }
	
	UChar       ucFirstByte   = *pcBinData->data();
    NalUnitType eNalUnitType  = NalUnitType ( ucFirstByte  & 0x1F );
	
    if(eNalUnitType == NAL_UNIT_SEI || eNalUnitType == NAL_UNIT_SPS || eNalUnitType == NAL_UNIT_PPS)
	{
	  uiSkip ++;
    }
	RNOK( m_pcReadBitstream[uiLayer]->releasePacket( pcBinData ) ); 
	
  }

  RNOK( m_pcH264AVCPacketAnalyzer->uninit() );

  uiNumSkip = uiSkip;

  return Err::m_nOK;
}

ErrVal
EncoderMerger::AnalyseBitstream(UInt uiAnalyzedLayer)
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
 
  UInt uiNumFrames[MAX_LAYERS];
  UInt uiFrame;
  for(uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++)
  {
	  uiNumFrames[uiLayer] = 0;
  }
  //========== initialize (scalable SEI message shall be the first packet of the stream) ===========
  {
    RNOK( m_pcH264AVCPacketAnalyzer->init() );
    //--- get first packet ---
    RNOK( m_pcReadBitstream[uiAnalyzedLayer]->extractPacket( pcBinData, bEOS ) );
    ROT ( bEOS );
    //--- analyse packet ---
    RNOK( m_pcH264AVCPacketAnalyzer ->process( pcBinData, cPacketDescription, pcScalableSei ) );
    ROF ( pcScalableSei );
    
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
    RNOK( m_pcReadBitstream[uiAnalyzedLayer]->extractPacket( pcBinData, bEOS ) );
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
    }
    bApplyToNext  = cPacketDescription.ApplyToNext;
    bNewPicture   = ( ! cPacketDescription.ParameterSet && ! cPacketDescription.ApplyToNext );
 
	if(! cPacketDescription.ParameterSet)
	{
		//Data NAL unit
		if(uiFGSLayer == 0 && !bApplyToNext)
			uiNumFrames[uiLayer] ++;
		uiFrame = uiNumFrames[uiLayer] - 1;
		m_aaaauiRate[uiAnalyzedLayer][uiLayer][uiFrame][uiFGSLayer] = uiPacketSize;
		printf(" Add to layer %d Frame %d FGS %d Rate %d \n",uiLayer,uiFrame,uiFGSLayer,uiPacketSize);
	}
  }

  //----- reset input file -----
  RNOKS( static_cast<ReadBitstreamFile*>(m_pcReadBitstream[uiAnalyzedLayer])->uninit() );
  RNOKS( static_cast<ReadBitstreamFile*>(m_pcReadBitstream[uiAnalyzedLayer])->init  ( m_pcEncoderMergerParameter->getInFile(uiAnalyzedLayer) ) );  

  for(uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++)
  {
	  m_auiNumFrameAtLayer[uiLayer] = uiNumFrames[uiLayer];
  }
  return Err::m_nOK;
}

Void EncoderMerger::calculateMaxRate()
{
	UInt uiLayer;
	UInt uiCurrLayer;
	UInt uiFrame;
	UInt uiFGSLayer;
	UInt uiRate = 0;
	for(uiCurrLayer = 1; uiCurrLayer < m_uiNumOfLayer;uiCurrLayer++)
	{
		for(uiLayer = uiCurrLayer-1; uiLayer < uiCurrLayer; uiLayer++)
		{
			for(uiFrame = 0; uiFrame < m_auiNumFrameAtLayer[uiLayer]; uiFrame++)
			{
				uiRate = 0;
				for(uiFGSLayer = 0; uiFGSLayer < MAX_FGS_LAYERS; uiFGSLayer++)
				{
					uiRate += m_aaaauiRate[uiCurrLayer][uiLayer][uiFrame][uiFGSLayer];
					printf(" For Frame %d, Rate1 %d %d Rate2 %d %d \n",uiFrame,uiCurrLayer-1, m_aaaauiRate[uiCurrLayer-1][uiLayer][uiFrame][uiFGSLayer],
						uiCurrLayer, m_aaaauiRate[uiCurrLayer][uiLayer][uiFrame][uiFGSLayer]);

					if(m_aaaauiRate[uiCurrLayer-1][uiLayer][uiFrame][uiFGSLayer] 
						!= m_aaaauiRate[uiCurrLayer][uiLayer][uiFrame][uiFGSLayer] || 
                            (m_aaaauiRate[uiCurrLayer-1][uiLayer][uiFrame][uiFGSLayer] == 0 && 
                            m_aaaauiRate[uiCurrLayer][uiLayer][uiFrame][uiFGSLayer] == 0))
						{
							m_aauiMaxRate[uiLayer][uiFrame] = uiRate;
							printf("MaxRate: %d \n", uiRate);
							break;
						}
				}
			}
		}
	}

}

ErrVal EncoderMerger::go()
{

	if(m_pcEncoderMergerParameter->getInsertDS() == true)
	{
		RNOK(go_DS());
	}
	if(m_pcEncoderMergerParameter->getInsertQL() == true)
	{
		RNOK(go_QL());
	}
	return Err::m_nOK;
}
ErrVal EncoderMerger::go_DS()
{
	UInt uiLayer;
	UInt uiTotal = 0;
	
	//Analyze all input files
	for(uiLayer = 0; uiLayer < m_uiNumOfLayer; uiLayer++)
	{
		AnalyseBitstream(uiLayer);
	}

	//Calculate MaxRate
	calculateMaxRate();

	//Write first NALs
	Bool bMoreSets=true;

	RNOK( m_pcH264AVCEncoder->init( m_pcEncoderMergerParameter )); 

	UInt *uiNumSkip = new UInt[m_uiNumOfLayer];
	for(uiLayer = 0; uiLayer < m_uiNumOfLayer; uiLayer++)
	{
		countNumOfNAL(uiLayer,uiNumSkip[uiLayer]);
	}
	
	MergeAndAddDSInfo(uiNumSkip);

    delete []uiNumSkip;
	return Err::m_nOK;
}

ErrVal EncoderMerger::go_QL()
{

  RNOK ( AnalyseBitstream_QL() );
  UInt uiLayer;
  UInt uiExtLayer = m_pcEncoderMergerParameter->m_uiNumOfLayer;
  
  for(uiLayer = 0; uiLayer < uiExtLayer; uiLayer++)
  {
	ReadFGSRateAndDistoFile(uiLayer,m_pcEncoderMergerParameter->getFGSRateFilename(uiLayer),
		m_pcEncoderMergerParameter->getDistoFilename(uiLayer), 0);
  }

  CalculateQualityLevel(uiExtLayer);

  RNOK( m_pcH264AVCEncoder->init( m_pcEncoderMergerParameter )); 

  RNOK(addQualityLevel());
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
 
  UInt uiNumFrames[MAX_LAYERS];
  UInt uiFrame;
  for(uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++)
  {
	  uiNumFrames[uiLayer] = 0;
  }
  for(uiFrame=0; uiFrame<MAX_NBFRAMES; uiFrame++)
  {
    for(uiLayer=0; uiLayer<MAX_LAYERS; uiLayer++)
    {
      for(uiFGSLayer=0; uiFGSLayer<MAX_FGS_LAYERS+1; uiFGSLayer++)
        m_aaadByteForFrameFGS[uiFrame][uiLayer][uiFGSLayer] = 0;
      m_aadByteForFrame[uiFrame][uiLayer] = 0;
    }
  }
  uiLayer = 0;
  uiFGSLayer = 0;

  //========== initialize (scalable SEI message shall be the first packet of the stream) ===========
  {
    RNOK( m_pcH264AVCPacketAnalyzer->init() );
    //--- get first packet ---
    RNOK( m_pcReadBitstream[0]->extractPacket( pcBinData, bEOS ) );
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
    RNOK( m_pcReadBitstream[0]->extractPacket( pcBinData, bEOS ) );
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
  RNOKS( static_cast<ReadBitstreamFile*>(m_pcReadBitstream[0])->uninit() );
  RNOKS( static_cast<ReadBitstreamFile*>(m_pcReadBitstream[0])->init  ( m_pcEncoderMergerParameter->getInFile(0) ) );  

  for(uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++)
  {
	  m_auiNumFrameAtLayer[uiLayer] = uiNumFrames[uiLayer];
  }

  return Err::m_nOK;
}

ErrVal EncoderMerger::WriteDeadSubstream(UInt uiLayer, UInt uiNFrames)
{
	UChar aucParameterSetBuffer[1000];

	BinData cBinData;
    cBinData.reset();
    cBinData.set( aucParameterSetBuffer, 1000 );

    ExtBinDataAccessor cExtBinDataAccessor;
    cBinData.setMemAccessor( cExtBinDataAccessor );
	printf("Writing deadsubstream Layer %d Image %d size %d  \n", uiLayer, uiNFrames, m_aauiMaxRate[uiLayer][uiNFrames]);
	m_pcH264AVCEncoder->writeDeadSubstreamSEI(&cExtBinDataAccessor, m_aauiMaxRate[uiLayer][uiNFrames], uiLayer);
	RNOK( m_pcWriteBitstreamToFile->writePacket( &m_cBinDataStartCode ) );
    RNOK( m_pcWriteBitstreamToFile->writePacket( &cExtBinDataAccessor ) );
     
	cBinData.reset();
    return Err::m_nOK;
}

Void EncoderMerger::WriteDiscardableFlag(BinData*  pcBinData)
{
	UChar ucByte = pcBinData->data()[1];
	ucByte = (ucByte | 2);
	pcBinData->data()[1] = ucByte;
	ucByte = 0;
	ucByte = pcBinData->data()[1];
	printf("Write discardable flag \n");
}


Void EncoderMerger::ReadFGSRateAndDistoFile(UInt uiLayer, std::string & FGSRateFilename,std::string & DistoFilename, UInt uiExtLevel)
{
  Char  acLine    [1000];
  Char  acLineDisto    [1000];
  Char  acLine3    [1000];
  FILE *file = ::fopen( FGSRateFilename.c_str(), "rt" );
  FILE *fileDisto = ::fopen( DistoFilename.c_str(), "rt" );
  UInt uiNFrames = 0;
  UInt uiNumFrame;
  Int uiLevel;
  Double dWLayer = (uiLayer == 0 ? 1 : (uiLayer == 1 ? 4: 16));
  Int index = 0;
  UInt uiCutLayer;
  Double dDistoCut;
  Double div = 8.0;
  Double acc = 0.0;

	Int i,c;
	Int i2,c2;
	Int i3,c3;
	Int temp;
	Int temp2;
	Int temp3;
	UInt ui, uiN;

	for(ui = 0; ui < MAX_LAYERS;ui++)
	{
		for(uiN = 0; uiN < MAX_NBFRAMES; uiN++)
		{
			m_aauiNbPoints[ui][uiN] = 0;
		}
	}
	
	while(!feof(file))
	{
	  acc = 0.0;
      //read num frame in GOP
	  for( i2 = 0; ( c2 = fgetc(fileDisto), ( c2 != '\t' && c2 != '\n' && c2 != EOF ) ); acLineDisto[i2++] = c2 );
      acLineDisto[i2] = '\0';
	  sscanf(acLineDisto, "%d ", &temp2);
	  uiNumFrame = temp2;

	  //read temporal level of current frame
	  for( i2 = 0; ( c2 = fgetc(fileDisto), ( c2 != '\t' && c2 != '\n' && c2 != EOF ) ); acLineDisto[i2++] = c2 );
      acLineDisto[i2] = '\0';
	  sscanf(acLineDisto, "%d ", &temp2);
	  uiLevel = temp2;
	  m_aaiLevel[uiLayer][uiNFrames] = uiLevel;

	  //set spatial layer weight
	  m_aadWeight[uiLayer][uiNFrames] = 1/dWLayer;

	  //read BL rate
	  for( i = 0; ( c = fgetc(file), ( c != '\t' && c != '\n' && c != EOF ) ); acLine[i++] = c );
      acLine[i] = '\0';
	  sscanf(acLine, "%d ", &temp);
		
	  //read BL disto
	  for( i2 = 0; ( c2 = fgetc(fileDisto), ( c2 != '\t' && c2 != '\n' && c2 != EOF ) ); acLineDisto[i2++] = c2 );
      acLineDisto[i2] = '\0';
	  sscanf(acLineDisto, "%d ", &temp2);

	  //set BL rate and disto
	  UInt uiIndex = 0;
	  uiNumFrame = uiNFrames;
	  m_aaadFGSRate[uiLayer][uiNFrames][uiIndex] = m_aaadByteForFrameFGS[uiNumFrame][uiLayer][0];
	  m_aaadDisto[uiLayer][uiNFrames][uiIndex] = temp2;
	  //read and set rate and disto of other points
	  while(c != '\n' && c != EOF)
	  {
        uiIndex++;
		//read rate
 		for( i = 0; ( c = fgetc(file), ( c != '\t' && c != '\n' && c != EOF ) ); acLine[i++] = c );
		acLine[i] = '\0';
		sscanf(acLine, "%d ", &temp);
		div = 8.0;
		if(temp == 1 || temp == 2 || temp == 3)
		{
			acc+= m_aaadByteForFrameFGS[uiNumFrame][uiLayer][temp];
			temp = acc;
			div = 1.0;
		}
		//read disto
		for( i2 = 0; ( c2 = fgetc(fileDisto), ( c2 != '\t' && c2 != '\n' && c2 != EOF ) ); acLineDisto[i2++] = c2 );
		acLineDisto[i2] = '\0';
		sscanf(acLineDisto, "%d ", &temp2);
		
		m_aaadFGSRate[uiLayer][uiNFrames][uiIndex] = m_aaadFGSRate[uiLayer][uiNFrames][0]+(Double)temp/div;
		m_aaadDisto[uiLayer][uiNFrames][uiIndex] = (uiNumFrame != -1) ? temp2 : 0;
	  }

	  //set FGS layers rate 
	  m_aauiNbPoints[uiLayer][uiNFrames] = uiIndex;
	  uiNFrames++;
	}
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

  m_aadByteForFrame[auiNumImage[uiLayer][uiFGSLayer]][uiLayer] += uiNumBytes; 
  m_aaadByteForFrameFGS[auiNumImage[uiLayer][uiFGSLayer]][uiLayer][uiFGSLayer] += uiNumBytes; 
  if (bNewPicture)
    auiNumImage[uiLayer][uiFGSLayer] ++;
}

Void EncoderMerger::PrintSizeFrames(UInt uiLayer, UInt uiNbFrames)
{
  Double sum=0;
  UInt uiFrame;
  for(uiFrame=0; uiFrame<uiNbFrames; uiFrame ++)
  {
    sum += m_aadByteForFrame[uiFrame][uiLayer];
  }
}

Void EncoderMerger::CalculateQualityLevel(UInt uiExtLayer)
{
  Double R,D;
  UInt uiNFrames;
  UInt uiDecStages;
  UInt uiNumPictures;
  UInt uiLayer;
  Int uiPoint;
  UInt uiMaxLayers = m_pcEncoderMergerParameter->m_uiNumOfLayer;

  Double QualityLevelMax[MAX_LAYERS] = {0,0,0};
  Double QualityLevelMin[MAX_LAYERS] = {100000000,100000000,100000000};

  // initializing RD infos
  for(uiLayer = 0; uiLayer < uiMaxLayers; uiLayer++)
  {
	uiNumPictures = m_auiNumFrameAtLayer[uiLayer];
    printf("Rate used for frame at layer %d\n", uiLayer);
	for(uiNFrames = 0; uiNFrames < uiNumPictures; uiNFrames++)
    {
		rpm[uiNFrames][uiLayer] = new RatePointManager();
        rpm[uiNFrames][uiLayer]->Reset();
        printf("RD points for Frame %d, uiLayer %d\n", uiNFrames, uiLayer);
        for(uiPoint = 0; uiPoint < MAX_NUM_RD_LEVELS; uiPoint++)
        {
          rpm[uiNFrames][uiLayer]->PushPoint( m_aaadFGSRate[uiLayer][uiNFrames][uiPoint], 
			  m_aadWeight[uiLayer][uiNFrames]*m_aaadDisto[uiLayer][uiNFrames][uiPoint]);;
        }
        
        rpm[uiNFrames][uiLayer]->SetValidPoints();
        rpm[uiNFrames][uiLayer]->PrintValidPoints();
		rpm[uiNFrames][uiLayer]->SetMaxRate(m_aadByteForFrame[uiNFrames][uiLayer]);

		for(UInt ui = 1; ui < rpm[uiNFrames][uiLayer]->getNbValidPoints(); ui++)
		{
			Double QualityLevel = rpm[uiNFrames][uiLayer]->getValidQualityLevel(ui);
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

ErrVal EncoderMerger::MergeAndAddDSInfo(UInt *uiNumSkip )
{

  UInt                    uiLayer       = 0;
  UInt                    auiLayer[MAX_LAYERS] = {0,0,0,0,0,0,0,0};
  UInt                    auiLevel[MAX_LAYERS] = {0,0,0,0,0,0,0,0};
  UInt                    auiFGSLayer[MAX_LAYERS] = {0,0,0,0,0,0,0,0};
  Bool                    bNewPicture   = false;
  Bool                    bApplyToNext  = false;
  Bool                    abEOS[MAX_LAYERS]  = {false,false,false,false,false,false,false,false};
  Bool                    bEOS          = false;
  Bool                    bFirstPacket  = true;
  typedef BinData *       pBinData;
  BinData**               pcBinData     = new pBinData[m_uiNumOfLayer];
  h264::SEI::SEIMessage*  pcScalableSei = 0;
  h264::PacketDescription cPacketDescription;

  UInt                    auiLevelPrev[MAX_LAYERS] = {0,0,0,0,0,0,0,0};
  UInt                    auiLayerPrev[MAX_LAYERS] = {0,0,0,0,0,0,0,0};

  UInt                    uiNal         = 0;
  Bool                    bFinish       = false;
  Bool                    bSet          = false;
  Bool                    bFirstTime    = true;
  UInt                    auiLayerBegin[MAX_LAYERS]  = {0,0,0,0,0,0,0,0};
  UInt                    uiLayerBegin  = 0;
  
  Int                     iRemainBytes[MAX_LAYERS] = {0,0,0,0,0,0,0,0};
  UInt                    uiPacketSize[MAX_LAYERS] = {0,0,0,0,0,0,0,0};

  UInt uiNumFrame[MAX_LAYERS];
  for(uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++)
  {
	  uiNumFrame[uiLayer] = 0;
  }
  uiLayer = 0;
  Bool bSEIPacket = true; 

  RNOK( m_pcH264AVCPacketAnalyzer->init() );

  for(uiLayer = 0; uiLayer < m_uiNumOfLayer; uiLayer++)
  {
	  RNOKS( ReadBitstreamFile::create( m_pcReadBitstream[uiLayer] ) ); 
	  RNOKS( m_pcReadBitstream[uiLayer]->init( m_pcEncoderMergerParameter->getInFile(uiLayer) ));
  }

  //skip first parameters NAL for all layers but the last
  for(uiLayer = 0; uiLayer < m_uiNumOfLayer-1; uiLayer++)
  {
	  for(uiNal = 0; uiNal < uiNumSkip[uiLayer]; uiNal++)
	  {
          RNOK( m_pcReadBitstream[uiLayer]->extractPacket( pcBinData[uiLayer], bEOS ) );
		  RNOK( m_pcReadBitstream[uiLayer]->releasePacket( pcBinData[uiLayer] ) ); 
	  }
  }

  //write parameter sets information from the last layer
  for(uiNal = 0; uiNal < uiNumSkip[m_uiNumOfLayer-1]; uiNal++)
  {
      RNOK( m_pcReadBitstream[m_uiNumOfLayer-1]->extractPacket( pcBinData[m_uiNumOfLayer-1], bEOS ) );
      RNOK( m_pcWriteBitstreamToFile->writePacket( &m_cBinDataStartCode ) );
	  RNOK( m_pcWriteBitstreamToFile->writePacket( pcBinData[m_uiNumOfLayer-1] ) );
	  RNOK( m_pcReadBitstream[m_uiNumOfLayer-1]->releasePacket( pcBinData[m_uiNumOfLayer-1] ) ); 
  }

  //get First NAL of all input files
  for(uiLayer = 0; uiLayer < m_uiNumOfLayer; uiLayer++)
  {
      if(!abEOS[uiLayer])
      {
          RNOK( m_pcReadBitstream[uiLayer]->extractPacket( pcBinData[uiLayer], bEOS ) );
      }
      else
        bEOS = true;
      if( bEOS )
      {
        abEOS[uiLayer] = bEOS;
        continue;
      }
      //===== get packet description =====
      RNOK( m_pcH264AVCPacketAnalyzer->process( pcBinData[uiLayer], cPacketDescription, pcScalableSei ) );
      //===== set packet length =====
      while( pcBinData[uiLayer]->data()[ pcBinData[uiLayer]->size() - 1 ] == 0x00 )
      {
        RNOK( pcBinData[uiLayer]->decreaseEndPos( 1 ) ); // remove zero at end
      }
    
      //==== get parameters =====
      uiPacketSize[uiLayer]  = 4 + pcBinData[uiLayer]->size();
      auiLayer[uiLayer]     = cPacketDescription.Layer;
      auiLevel[uiLayer]     = cPacketDescription.Level;
      auiFGSLayer[uiLayer] = cPacketDescription.FGSLayer;
  }


  while( ! bFinish )
  {
    
    //set cursor of reading bitstream to corresponding nal layer
    for(uiLayer = 0; uiLayer < m_uiNumOfLayer; uiLayer++)
    {
        bSet = false;
        while(!bSet)
        {
            if(auiLayer[uiLayer] != uiLayer)
            {
                //layer of NAL different from current layer: release packet
                if(!abEOS[uiLayer])
                {
                    RNOK( m_pcReadBitstream[uiLayer]->releasePacket( pcBinData[uiLayer] ) );
                }
                else
                {
                    bSet = true;
                    continue;
                }
            }
            else
            {
                //layer of NAL same as current layer: cursor is set for this frame
                bSet = true;
                continue;
            }

            if(!abEOS[uiLayer])
            {
                RNOK( m_pcReadBitstream[uiLayer]->extractPacket( pcBinData[uiLayer], bEOS ) );
            }
            else
                bEOS = true;
            if( bEOS )
            {
                abEOS[uiLayer] = bEOS;
                continue;
            }
            //===== get packet description =====
            RNOK( m_pcH264AVCPacketAnalyzer->process( pcBinData[uiLayer], cPacketDescription, pcScalableSei ) );
            //===== set packet length =====
            while( pcBinData[uiLayer]->data()[ pcBinData[uiLayer]->size() - 1 ] == 0x00 )
            {
                RNOK( pcBinData[uiLayer]->decreaseEndPos( 1 ) ); // remove zero at end
            }
    
            //==== get parameters =====
            uiPacketSize[uiLayer]  = 4 + pcBinData[uiLayer]->size();
            auiLayer[uiLayer]     = cPacketDescription.Layer;
            auiLevel[uiLayer]     = cPacketDescription.Level;
            auiFGSLayer[uiLayer] = cPacketDescription.FGSLayer;
        }
    }

    //write DS info for all layers but the last
    for(uiLayer = uiLayerBegin; uiLayer < m_pcEncoderMergerParameter->m_uiNumOfLayer-1; uiLayer++)
    {
		if(!abEOS[uiLayer])
        {
			iRemainBytes[uiLayer] = m_aauiMaxRate[uiLayer][uiNumFrame[uiLayer]];
			WriteDeadSubstream(uiLayer,uiNumFrame[uiLayer]);
			uiNumFrame[uiLayer] ++;
		}
    }
    
    //write all datas info for all layers 
    for(uiLayer = uiLayerBegin; uiLayer < m_pcEncoderMergerParameter->m_uiNumOfLayer; uiLayer++)
    {
        bSet = false;
        while(!bSet)
        {
        if(uiLayer == auiLayer[uiLayer])
        {
            if(!abEOS[uiLayer])
            {
				if(auiFGSLayer[uiLayer] == 0)
				{
					if(uiLayer == m_pcEncoderMergerParameter->m_uiNumOfLayer-1)
						uiNumFrame[uiLayer]++;
				}

                if(iRemainBytes[uiLayer] <= 0 && auiFGSLayer[uiLayer] != 0 && uiLayer != m_pcEncoderMergerParameter->m_uiNumOfLayer-1)
                {
                    WriteDiscardableFlag(pcBinData[uiLayer]);
                }
                iRemainBytes[uiLayer] -= uiPacketSize[uiLayer];
                RNOK( m_pcWriteBitstreamToFile->writePacket( &m_cBinDataStartCode ) );
	            RNOK( m_pcWriteBitstreamToFile->writePacket( pcBinData[uiLayer] ) );
            }
            else
            {
                bSet = true;
                continue;
            }
        }
        if(!abEOS[uiLayer])
        {
            RNOK( m_pcReadBitstream[uiLayer]->releasePacket( pcBinData[uiLayer] ) );
            RNOK( m_pcReadBitstream[uiLayer]->extractPacket( pcBinData[uiLayer], bEOS ) );
        }
        else
            bEOS = true;
        if( bEOS )
        {
           abEOS[uiLayer] = bEOS;
           continue;
        }
        //===== get packet description =====
        RNOK( m_pcH264AVCPacketAnalyzer->process( pcBinData[uiLayer], cPacketDescription, pcScalableSei ) );
        //===== set packet length =====
        while( pcBinData[uiLayer]->data()[ pcBinData[uiLayer]->size() - 1 ] == 0x00 )
        {
           RNOK( pcBinData[uiLayer]->decreaseEndPos( 1 ) ); // remove zero at end
        }
    
        //==== get parameters =====
        uiPacketSize[uiLayer] = 4 + pcBinData[uiLayer]->size();
        auiLayer[uiLayer]     = cPacketDescription.Layer;
        auiLevel[uiLayer]     = cPacketDescription.Level;
        auiFGSLayer[uiLayer] = cPacketDescription.FGSLayer;
        if(auiFGSLayer[uiLayer] == 0 && auiLayer[uiLayer] == 0)
        {
            auiLayerBegin[uiLayer] = 0;
            bSet = true;
        }
        else
        {
            if(auiFGSLayer[uiLayer] == 0 && 
            ((auiLayerPrev[uiLayer] <= auiLayer[uiLayer] && auiLevelPrev[uiLayer]<auiLevel[uiLayer]) ||
            (auiLayerPrev[uiLayer] == auiLayer[uiLayer] && auiLevelPrev[uiLayer] == auiLevel[uiLayer] ||
			(auiLayerPrev[uiLayer] > auiLayer[uiLayer]))))
            {
              auiLayerBegin[uiLayer] = auiLayer[uiLayer];
              bSet = true;
            }
        }

        auiLayerPrev[uiLayer] = auiLayer[uiLayer];
        auiLevelPrev[uiLayer] = auiLevel[uiLayer];
        }
    }

    //set uiLayerBegin
    uiLayerBegin = 0;
    for(uiLayer = 0; uiLayer < m_uiNumOfLayer; uiLayer++)
    {
       if(auiLayerBegin[uiLayer] > uiLayerBegin)
       {
           uiLayerBegin = auiLayerBegin[uiLayer];
       }
    }

    //set bFinish
    bFinish = true;
    for(uiLayer = 0; uiLayer < m_uiNumOfLayer; uiLayer++)
    {
        if(!abEOS[uiLayer])
            bFinish = false;
    }
  }

  //----- reset input file -----
  for(uiLayer = 0; uiLayer < m_uiNumOfLayer; uiLayer++)
  {
    RNOKS( static_cast<ReadBitstreamFile*>(m_pcReadBitstream[uiLayer])->uninit() );
    RNOKS( static_cast<ReadBitstreamFile*>(m_pcReadBitstream[uiLayer])->init  ( m_pcEncoderMergerParameter->getInFile(uiLayer) ) );  
  }

  return Err::m_nOK;
}

ErrVal EncoderMerger::addQualityLevel()
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
    RNOK( m_pcReadBitstream[0]->extractPacket( pcBinData, bEOS ) );
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
                writeQualityLevel(uiLayerForSEI,uiNumFrame[uiLayerForSEI]);
                uiNumFrame[uiLayerForSEI] ++;
            }
        }
	}

	if(bSEIPacket)
		bSEIPacket = false;

    RNOK( m_pcWriteBitstreamToFile->writePacket( &m_cBinDataStartCode ) );
	RNOK( m_pcWriteBitstreamToFile->writePacket( pcBinData ) );
	RNOK( m_pcReadBitstream[0]->releasePacket( pcBinData ) ); 

    uiLayerPrev = uiLayer;
    uiLevelPrev = uiLevel;
  }

  //----- reset input file -----
  RNOKS( static_cast<ReadBitstreamFile*>(m_pcReadBitstream[0])->uninit() );
  RNOKS( static_cast<ReadBitstreamFile*>(m_pcReadBitstream[0])->init  ( m_pcEncoderMergerParameter->getInFile(0) ) );  

  return Err::m_nOK;
}

ErrVal EncoderMerger::writeQualityLevel(UInt uiLayer, UInt uiNFrames)
{
	UChar aucParameterSetBuffer[1000];

	BinData cBinData;
    cBinData.reset();
    cBinData.set( aucParameterSetBuffer, 1000 );

    ExtBinDataAccessor cExtBinDataAccessor;
    cBinData.setMemAccessor( cExtBinDataAccessor );

	UInt uiNumLevels = rpm[uiNFrames][uiLayer]->getNbValidPoints();
	printf("NumLevels %d: \n",uiNumLevels);
	UInt uiRate[MAX_NUM_RD_LEVELS];
	UInt uiQualityLevel[MAX_NUM_RD_LEVELS];
	Bool abSign[MAX_NUM_RD_LEVELS];
	UInt uiRateOld = rpm[uiNFrames][uiLayer]->getValidRate(0);
	UInt ui;
	
	UInt uiMaxLayers = m_pcEncoderMergerParameter->m_uiNumOfLayer;
	for(ui = 0; ui < uiNumLevels; ui++)
	{	
		if(ui == 0)
		{
			uiRate[ui] = rpm[uiNFrames][uiLayer]->getValidRate(ui);
			uiRateOld = rpm[uiNFrames][uiLayer]->getValidRate(ui);
		}
		else
		{
			uiRate[ui] = rpm[uiNFrames][uiLayer]->getValidRate(ui) - uiRateOld;
            uiRateOld = rpm[uiNFrames][uiLayer]->getValidRate(ui);
		}
		
		Double QualityLevel;
		Int iQualityLevelToCode = 0;
		if(ui == 0)
		{
			//first slope is infinite
			iQualityLevelToCode = 255;
		}
		else
		{
		QualityLevel = log(rpm[uiNFrames][uiLayer]->getValidQualityLevel(ui));
		
		iQualityLevelToCode = (Int)(254*(QualityLevel-m_dQualityLevelMinGlobal) / (m_dQualityLevelMaxGlobal - m_dQualityLevelMinGlobal));
		iQualityLevelToCode = (iQualityLevelToCode < 0 ? 0: iQualityLevelToCode > 254 ? 254 : iQualityLevelToCode);
		
		}
		uiQualityLevel[ui] = iQualityLevelToCode;
		
	}
	m_pcH264AVCEncoder->writeQualityLevelInfosSEI(&cExtBinDataAccessor, uiQualityLevel,uiRate,uiNumLevels, uiLayer);
	
    RNOK( m_pcWriteBitstreamToFile->writePacket( &m_cBinDataStartCode ) );
    RNOK( m_pcWriteBitstreamToFile->writePacket( &cExtBinDataAccessor ) );
     
	cBinData.reset();

	return Err::m_nOK;
}