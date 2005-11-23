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


#include "QualityLevelAssigner.h"

#include "ReadBitstreamFile.h"
#include "WriteBitstreamToFile.h"
#include "ReadYuvFile.h"
#include "WriteYuvToFile.h"
#include "QualityLevelParameter.h"
#include "H264AVCCommonLib.h"
#include "H264AVCCommonLib/CommonDefs.h"
#include <math.h>
#if WIN32
#include <io.h>
#include <windows.h>
#endif

using namespace h264;



QualityLevelAssigner::QualityLevelAssigner()
: m_pcParameter             ( 0 )
, m_pcH264AVCPacketAnalyzer ( 0 )
, m_pcH264AVCDecoder        ( 0 )
, m_pcBitCounter            ( 0 )
, m_pcBitWriteBuffer        ( 0 )
, m_pcUvlcWriter            ( 0 )
, m_pcUvlcTester            ( 0 )
, m_pcNalUnitEncoder        ( 0 )
, m_uiNumLayers             ( 0 )
, m_bOutputReconstructions  ( false ) // for debugging
{
  ::memset( m_auiNumFGSLayers, 0x00, MAX_LAYERS                   *sizeof(UInt) );
  ::memset( m_auiNumFrames,    0x00, MAX_LAYERS                   *sizeof(UInt) );
  ::memset( m_auiGOPSize,      0x00, MAX_LAYERS                   *sizeof(UInt) );
  ::memset( m_auiNumTempLevel, 0x00, MAX_LAYERS                   *sizeof(UInt) );
  ::memset( m_auiFrameWidth,   0x00, MAX_LAYERS                   *sizeof(UInt) );
  ::memset( m_auiFrameHeight,  0x00, MAX_LAYERS                   *sizeof(UInt) );
  ::memset( m_auiSPSRequired,  0x00, 32                           *sizeof(UInt) );
  ::memset( m_auiPPSRequired,  0x00, 256                          *sizeof(UInt) );
  ::memset( m_aaadDeltaDist,   0x00, MAX_LAYERS*MAX_QUALITY_LEVELS*sizeof(Void*));
  ::memset( m_aaauiPacketSize, 0x00, MAX_LAYERS*MAX_QUALITY_LEVELS*sizeof(Void*));
  ::memset( m_aaauiQualityID,  0x00, MAX_LAYERS*MAX_QUALITY_LEVELS*sizeof(Void*));
}


QualityLevelAssigner::~QualityLevelAssigner()
{
}


ErrVal
QualityLevelAssigner::create( QualityLevelAssigner*& rpcQualityLevelAssigner )
{
  rpcQualityLevelAssigner = new QualityLevelAssigner;
  ROF( rpcQualityLevelAssigner );
  return Err::m_nOK;
}


ErrVal
QualityLevelAssigner::init( QualityLevelParameter* pcParameter )
{
  ROF( pcParameter );
  m_pcParameter = pcParameter;

  //--- create objects ---
  RNOK( H264AVCPacketAnalyzer::create( m_pcH264AVCPacketAnalyzer ) );
  RNOK( CreaterH264AVCDecoder::create( m_pcH264AVCDecoder ) );
  RNOK( BitCounter           ::create( m_pcBitCounter ) );
  RNOK( BitWriteBuffer       ::create( m_pcBitWriteBuffer ) );
  RNOK( UvlcWriter           ::create( m_pcUvlcWriter ) );
  RNOK( UvlcWriter           ::create( m_pcUvlcTester ) );
  RNOK( NalUnitEncoder       ::create( m_pcNalUnitEncoder ) );

  //--- initialize encoder objects ---
  RNOK( m_pcBitWriteBuffer  ->init() );
  RNOK( m_pcBitCounter      ->init() );
  RNOK( m_pcUvlcWriter      ->init( m_pcBitWriteBuffer ) );
  RNOK( m_pcUvlcTester      ->init( m_pcBitCounter ) );
  RNOK( m_pcNalUnitEncoder  ->init( m_pcBitWriteBuffer, m_pcUvlcWriter, m_pcUvlcTester ) );

  //--- get basic stream parameters ---
  RNOK( xInitStreamParameters() );

  //--- create arrays ---
  for( UInt uiLayer = 0; uiLayer <  m_uiNumLayers;              uiLayer++ )
  for( UInt uiFGS   = 0; uiFGS   <= m_auiNumFGSLayers[uiLayer]; uiFGS  ++ )
  {
    ROF( m_auiNumFrames[uiLayer] );
    ROFRS( ( m_aaadDeltaDist  [uiLayer][uiFGS] = new Double[m_auiNumFrames[uiLayer]] ), Err::m_nERR );
    ROFRS( ( m_aaauiPacketSize[uiLayer][uiFGS] = new UInt  [m_auiNumFrames[uiLayer]] ), Err::m_nERR );
    ROFRS( ( m_aaauiQualityID [uiLayer][uiFGS] = new UInt  [m_auiNumFrames[uiLayer]] ), Err::m_nERR );
  }

  //--- init start code ----
  m_aucStartCodeBuffer[0] = 0;
  m_aucStartCodeBuffer[1] = 0;
  m_aucStartCodeBuffer[2] = 0;
  m_aucStartCodeBuffer[3] = 1;
  m_cBinDataStartCode.reset();
  m_cBinDataStartCode.set( m_aucStartCodeBuffer, 4 );

  return Err::m_nOK;
}


ErrVal
QualityLevelAssigner::destroy()
{
  m_cBinDataStartCode.reset();

  if( m_pcH264AVCPacketAnalyzer )
  {
    RNOK( m_pcH264AVCPacketAnalyzer->destroy() );
  }
  if( m_pcH264AVCDecoder )
  {
    RNOK( m_pcH264AVCDecoder->destroy() );
  }
  if( m_pcBitCounter )
  {
    RNOK( m_pcBitCounter->uninit  () );
    RNOK( m_pcBitCounter->destroy () );
  }
  if( m_pcBitWriteBuffer )
  {
    RNOK( m_pcBitWriteBuffer->uninit  () );
    RNOK( m_pcBitWriteBuffer->destroy () );
  }
  if( m_pcUvlcTester )
  {
    RNOK( m_pcUvlcTester->uninit  () );
    RNOK( m_pcUvlcTester->destroy () );
  }
  if( m_pcUvlcWriter )
  {
    RNOK( m_pcUvlcWriter->uninit  () );
    RNOK( m_pcUvlcWriter->destroy () );
  }
  if( m_pcNalUnitEncoder )
  {
    RNOK( m_pcNalUnitEncoder->uninit  () );
    RNOK( m_pcNalUnitEncoder->destroy () )
  }


  for( UInt uiLayer = 0; uiLayer < MAX_LAYERS;          uiLayer++ )
  for( UInt uiFGS   = 0; uiFGS   < MAX_QUALITY_LEVELS;  uiFGS  ++ )
  {
    delete [] m_aaadDeltaDist   [uiLayer][uiFGS];
    delete [] m_aaauiPacketSize [uiLayer][uiFGS];
    delete [] m_aaauiQualityID  [uiLayer][uiFGS];
  }
  
  //===== delete picture buffers =====
  RNOK( xClearPicBufferLists() );

  delete this;
  return Err::m_nOK;
}


ErrVal
QualityLevelAssigner::go()
{
  //===== get rate and distortion values =====
  if( m_pcParameter->readDataFile() )
  {
    RNOK( xReadDataFile( m_pcParameter->getDataFileName() ) );
  }
  else
  {
    RNOK( xInitRateAndDistortion() );

    if( m_pcParameter->writeDataFile() )
    {
      RNOK( xWriteDataFile( m_pcParameter->getDataFileName() ) );
    }
  }
  
  if( ! m_pcParameter->getOutputBitStreamName().empty() )
  {
    //===== determine quality levels =====
    RNOK( xDetermineQualityIDs() );

    //===== write output stream with quality levels =====
    if( m_pcParameter->writeQualityLayerSEI() )
    {
      RNOK( xWriteQualityLayerStreamSEI() );
    }
    else
    {
      RNOK( xWriteQualityLayerStreamPID() );
    }
  }

  printf("\n");
  return Err::m_nOK;
}


ErrVal
QualityLevelAssigner::xGetNewPicBuffer( PicBuffer*& rpcPicBuffer,
                                        UInt        uiSize )
{
  if( m_cUnusedPicBufferList.empty() )
  {
    rpcPicBuffer = new PicBuffer( new UChar[ uiSize ] );
  }
  else
  {
    rpcPicBuffer = m_cUnusedPicBufferList.popFront();
  }

  m_cActivePicBufferList.push_back( rpcPicBuffer );
  return Err::m_nOK;
}


ErrVal
QualityLevelAssigner::xRemovePicBuffer( PicBufferList& rcPicBufferUnusedList )
{
  while( ! rcPicBufferUnusedList.empty() )
  {
    PicBuffer* pcBuffer = rcPicBufferUnusedList.popFront();

    if( NULL != pcBuffer )
    {
      PicBufferList::iterator  begin = m_cActivePicBufferList.begin();
      PicBufferList::iterator  end   = m_cActivePicBufferList.end  ();
      PicBufferList::iterator  iter  = std::find( begin, end, pcBuffer );

      ROT( iter == end ); // there is something wrong if the address is not in the active list

      AOT( pcBuffer->isUsed() )
      m_cUnusedPicBufferList.push_back( pcBuffer );
      m_cActivePicBufferList.erase    (  iter );
    }
  }
  return Err::m_nOK;
}


ErrVal
QualityLevelAssigner::xClearPicBufferLists()
{
  ROF( m_cActivePicBufferList.empty() );
  
  PicBufferList::iterator iter;
  for( iter = m_cUnusedPicBufferList.begin(); iter != m_cUnusedPicBufferList.end(); iter++ )
  {
    delete (*iter)->getBuffer();
    delete (*iter);
  }
  m_cUnusedPicBufferList.clear();
  
  for( iter = m_cActivePicBufferList.begin(); iter != m_cActivePicBufferList.end(); iter++ )
  {
    delete (*iter)->getBuffer();
    delete (*iter);
  }
  m_cActivePicBufferList.clear();

  return Err::m_nOK;
}


ErrVal
QualityLevelAssigner::xInitStreamParameters()
{
  printf( "analyse stream content ..." );

  Bool              bFirstPacket  = true;
  BinData*          pcBinData     = 0;
  SEI::SEIMessage*  pcScalableSEI = 0;
  PacketDescription cPacketDescription;

  m_uiNumLayers = 0;
  ::memset( m_auiNumFGSLayers,  0x00, MAX_LAYERS*sizeof(UInt) );
  ::memset( m_auiNumFrames,     0x00, MAX_LAYERS*sizeof(UInt) );
  ::memset( m_auiGOPSize,       0x00, MAX_LAYERS*sizeof(UInt) );
  ::memset( m_auiNumTempLevel,  0x00, MAX_LAYERS*sizeof(UInt) );
  ::memset( m_auiFrameWidth,    0x00, MAX_LAYERS*sizeof(UInt) );
  ::memset( m_auiFrameHeight,   0x00, MAX_LAYERS*sizeof(UInt) );
  ::memset( m_auiSPSRequired,   0x00, 32        *sizeof(UInt) );
  ::memset( m_auiPPSRequired,   0x00, 256       *sizeof(UInt) );

  //===== init =====
  RNOK( m_pcH264AVCPacketAnalyzer->init() );
  ReadBitstreamFile* pcReadBitStream = 0;
  RNOK( ReadBitstreamFile::create( pcReadBitStream ) );
  RNOK( pcReadBitStream->init( m_pcParameter->getInputBitStreamName() ) );


  //===== loop over packets =====
  while( true )
  {
    //----- read packet -----
    Bool bEOS = false;
    RNOK( pcReadBitStream->extractPacket( pcBinData, bEOS ) );
    if( bEOS )
    {
      break;
    }

    //----- get packet description -----
    RNOK( m_pcH264AVCPacketAnalyzer->process( pcBinData, cPacketDescription, pcScalableSEI ) );
    ROT( bFirstPacket && !pcScalableSEI );
    if( pcScalableSEI )
    {
      Bool              bUncompleteInfo     = false;
      SEI::ScalableSei* pcScalSEI           = (SEI::ScalableSei*)pcScalableSEI;
      UInt              uiNumLayersInSEI    = pcScalSEI->getNumLayersMinus1() + 1;
      Bool              abUsed[MAX_LAYERS]  = { false, false, false, false, false, false, false, false };

      for( UInt ui = 0; ui < uiNumLayersInSEI; ui++ )
      {
        if( ! pcScalSEI->getDecodingDependencyInfoPresentFlag ( ui ) ||
            ! pcScalSEI->getFrmSizeInfoPresentFlag            ( ui )   )
        {
          bUncompleteInfo = true;
          break;
        }

        UInt uiLayerId = pcScalSEI->getDependencyId( ui );
        if( abUsed[uiLayerId] )
        { // update information
          m_auiNumTempLevel [uiLayerId] = max( m_auiNumTempLevel[uiLayerId], pcScalSEI->getTemporalLevel( ui ) );
          m_auiGOPSize      [uiLayerId] = ( 1 << m_auiNumTempLevel[uiLayerId] );
        }
        else
        { // init information
          abUsed            [uiLayerId] = true;
          m_auiNumTempLevel [uiLayerId] = pcScalSEI->getTemporalLevel( ui );
          m_auiGOPSize      [uiLayerId] = ( 1 << m_auiNumTempLevel[uiLayerId] );
          m_auiFrameWidth   [uiLayerId] = ( pcScalSEI->getFrmWidthInMbsMinus1 ( ui ) + 1 ) << 4;
          m_auiFrameHeight  [uiLayerId] = ( pcScalSEI->getFrmHeightInMbsMinus1( ui ) + 1 ) << 4;
        }
      }

      ROT( bUncompleteInfo );

      delete pcScalableSEI;
      pcScalableSEI = 0;
      bFirstPacket  = false;
    }

    //----- analyse packets -----
    if( cPacketDescription.FGSLayer )
    {
      if( cPacketDescription.Layer+1 > m_uiNumLayers)
      {
        m_uiNumLayers = cPacketDescription.Layer+1;
      }
      if( cPacketDescription.FGSLayer > m_auiNumFGSLayers[cPacketDescription.Layer] )
      {
        m_auiNumFGSLayers[cPacketDescription.Layer] = cPacketDescription.FGSLayer;
      }
    }
    else if( ! cPacketDescription.ParameterSet && cPacketDescription.NalUnitType != NAL_UNIT_SEI &&
             ! cPacketDescription.FGSLayer )
    {
      m_auiNumFrames[cPacketDescription.Layer]++;
      
      m_auiSPSRequired[cPacketDescription.SPSid] |= (1 << cPacketDescription.Layer);
      m_auiPPSRequired[cPacketDescription.PPSid] |= (1 << cPacketDescription.Layer);
    }

    //----- delete bin data -----
    RNOK( pcReadBitStream->releasePacket( pcBinData ) );
  }

  
  //===== uninit =====
  RNOK( m_pcH264AVCPacketAnalyzer->uninit() );
  RNOK( pcReadBitStream->uninit() );
  RNOK( pcReadBitStream->destroy() );

  printf("\n");
  return Err::m_nOK;
}


ErrVal
QualityLevelAssigner::xInitRateAndDistortion()
{
  RNOK( xInitRateValues() );

  //----- create temporarily distortion arrays -----
  UInt  uiLayer, uiFGS, uiTLevel, uiFrame;
  UInt* aaaauiDistortionDep[MAX_LAYERS][MAX_QUALITY_LEVELS][MAX_DSTAGES+1];
  UInt* aaaauiDistortionInd[MAX_LAYERS][MAX_QUALITY_LEVELS][MAX_DSTAGES+1];
  for( uiLayer  = 0;  uiLayer   <  m_uiNumLayers;              uiLayer  ++ )
  for( uiFGS    = 0;  uiFGS     <= m_auiNumFGSLayers[uiLayer]; uiFGS    ++ )
  for( uiTLevel = 0;  uiTLevel  <= m_auiNumTempLevel[uiLayer]; uiTLevel ++ )
  {
    ROFRS( ( aaaauiDistortionDep[uiLayer][uiFGS][uiTLevel] = new UInt [m_auiNumFrames[uiLayer]] ), Err::m_nOK );
    ROFRS( ( aaaauiDistortionInd[uiLayer][uiFGS][uiTLevel] = new UInt [m_auiNumFrames[uiLayer]] ), Err::m_nOK );
  }
  for( uiLayer  = 0;  uiLayer   <  m_uiNumLayers;              uiLayer  ++ )
  for( uiFrame  = 0;  uiFrame   <  m_auiNumFrames[uiLayer];    uiFrame  ++ )
  {
    m_aaadDeltaDist[uiLayer][0][uiFrame] = 100.0; // dummy value
  }


  Bool bDep = m_pcParameter->useDependentDistCalc   ();
  Bool bInd = m_pcParameter->useIndependentDistCalc ();
  ROF( bDep || bInd );


  //----- determine picture distortions -----
  for( uiLayer = 0; uiLayer < m_uiNumLayers; uiLayer++ )
  {
    //----- get base layer distortion -----
    RNOK( xInitDistortion( aaaauiDistortionDep[uiLayer][0][0], uiLayer, 0 ) );
    ::memcpy(              aaaauiDistortionInd[uiLayer][0][0], aaaauiDistortionDep[uiLayer][0][0], m_auiNumFrames[uiLayer]*sizeof(UInt) );
    for( uiTLevel = 1; uiTLevel <= m_auiNumTempLevel[uiLayer]; uiTLevel++ )
    {
      ::memcpy(     aaaauiDistortionDep[uiLayer][0][uiTLevel], aaaauiDistortionDep[uiLayer][0][0], m_auiNumFrames[uiLayer]*sizeof(UInt) );
      ::memcpy(     aaaauiDistortionInd[uiLayer][0][uiTLevel], aaaauiDistortionInd[uiLayer][0][0], m_auiNumFrames[uiLayer]*sizeof(UInt) );
    }
    //----- get enhancement distortions -----
    for( uiFGS    = 1; uiFGS    <= m_auiNumFGSLayers[uiLayer]; uiFGS   ++ )
    for( uiTLevel = 0; uiTLevel <= m_auiNumTempLevel[uiLayer]; uiTLevel++ )
    {
      if( bDep )
      {
        RNOK( xInitDistortion( aaaauiDistortionDep[uiLayer][uiFGS][uiTLevel], uiLayer, uiFGS, uiTLevel, false ) );
      }
      if( bInd )
      {
        RNOK( xInitDistortion( aaaauiDistortionInd[uiLayer][uiFGS][uiTLevel], uiLayer, uiFGS, uiTLevel, true  ) );
      }
    }
    RNOK( xClearPicBufferLists() ); // spatial resolution can be changed
  }


  //----- init delta distortion -----
  printf( "determine delta distortions ..." );
  for( uiLayer = 0; uiLayer < m_uiNumLayers; uiLayer++ )
  {
    //----- create arrays for pic number to frame number mapping -----
    UInt* puiPic2FNum = new UInt[ m_auiNumFrames[uiLayer] ];
    ROF(  puiPic2FNum);
    Bool  bLastGOP    = false;
    UInt  uiFrameNum  = 0;
    UInt  uiGOPPos    = 0;
    for( puiPic2FNum[0] = uiFrameNum++; ! bLastGOP; uiGOPPos += m_auiGOPSize[uiLayer] )
    {
      for( uiTLevel = 0; uiTLevel <= m_auiNumTempLevel[uiLayer]; uiTLevel++ )
      {
        UInt uiStep = ( 1 << ( m_auiNumTempLevel[uiLayer] - uiTLevel ) );
        for( UInt uiDeltaPos = uiStep; uiDeltaPos <= m_auiGOPSize[uiLayer]; uiDeltaPos += (uiStep<<1) )
        {
          if( uiGOPPos + uiDeltaPos < m_auiNumFrames[uiLayer] )
          {
            puiPic2FNum[uiGOPPos+uiDeltaPos] = uiFrameNum++;
          }
          else
          {
            bLastGOP = true;
          }
        }
      }
    }
  
    //----- determine delta distortions -----
    for( uiFGS = 1; uiFGS <= m_auiNumFGSLayers[uiLayer]; uiFGS++ )
    {
      //----- key pictures (that's a bit tricky) -----
      {
        UInt  uiMaxTLevel     = m_auiNumTempLevel[uiLayer];
        Bool  bLastKeyPicture = false;
        for( UInt uiKeyPicCount = 0; ! bLastKeyPicture; uiKeyPicCount++ )
        {
          Double dDistortionBaseDep  = 0;
          Double dDistortionBaseInd  = 0;
          Double dDistortionEnhDep   = 0;
          Double dDistortionEnhInd   = 0;
          bLastKeyPicture         = ( ( ( m_auiNumFrames[uiLayer] - 1 ) / m_auiGOPSize[uiLayer] ) == uiKeyPicCount );
          UInt   uiPicNum         = uiKeyPicCount * m_auiGOPSize[uiLayer];
          UInt   uiStepSize2      = m_auiGOPSize[uiLayer] >> 1;
          //---- preceding level 1 picture -----
          if( uiKeyPicCount )
          {
            dDistortionBaseDep += log10( (Double)aaaauiDistortionDep[uiLayer][uiFGS-1][uiMaxTLevel][uiPicNum-uiStepSize2] ) / 2;
            dDistortionEnhDep  += log10( (Double)aaaauiDistortionDep[uiLayer][uiFGS  ][0          ][uiPicNum-uiStepSize2] ) / 2;

            dDistortionBaseInd += log10( (Double)aaaauiDistortionInd[uiLayer][uiFGS-1][0          ][uiPicNum-uiStepSize2] ) / 2;
            dDistortionEnhInd  += log10( (Double)aaaauiDistortionInd[uiLayer][uiFGS  ][0          ][uiPicNum-uiStepSize2] ) / 2;
          }
          //---- normal pictures -----
          UInt uiStartPicNum  = ( uiKeyPicCount   ?  uiPicNum - uiStepSize2 + 1 : 0 );
          UInt uiEndPicNum    = ( bLastKeyPicture ? m_auiNumFrames[uiLayer] - 1 : uiPicNum + uiStepSize2 - 1 );
          for( UInt uiCheckPicNum = uiStartPicNum; uiCheckPicNum <= uiEndPicNum; uiCheckPicNum++ )
          {
            dDistortionBaseDep += log10( (Double)aaaauiDistortionDep[uiLayer][uiFGS-1][uiMaxTLevel][uiCheckPicNum] );
            dDistortionEnhDep  += log10( (Double)aaaauiDistortionDep[uiLayer][uiFGS  ][0          ][uiCheckPicNum] );

            dDistortionBaseInd += log10( (Double)aaaauiDistortionInd[uiLayer][uiFGS-1][0          ][uiCheckPicNum] );
            dDistortionEnhInd  += log10( (Double)aaaauiDistortionInd[uiLayer][uiFGS  ][0          ][uiCheckPicNum] );
          }
          //---- following level 1 picture -----
          if( ! bLastKeyPicture )
          {
            dDistortionBaseDep += log10( (Double)aaaauiDistortionDep[uiLayer][uiFGS-1][uiMaxTLevel][uiPicNum+uiStepSize2] ) / 2;
            dDistortionEnhDep  += log10( (Double)aaaauiDistortionDep[uiLayer][uiFGS  ][0          ][uiPicNum+uiStepSize2] ) / 2;

            dDistortionBaseInd += log10( (Double)aaaauiDistortionInd[uiLayer][uiFGS-1][0          ][uiPicNum+uiStepSize2] ) / 2;
            dDistortionEnhInd  += log10( (Double)aaaauiDistortionInd[uiLayer][uiFGS  ][0          ][uiPicNum+uiStepSize2] ) / 2;
          }
          m_aaadDeltaDist[uiLayer][uiFGS][puiPic2FNum[uiPicNum]] = 0;
          m_aaadDeltaDist[uiLayer][uiFGS][puiPic2FNum[uiPicNum]]+= ( bDep ? dDistortionBaseDep - dDistortionEnhDep : 0 );
          m_aaadDeltaDist[uiLayer][uiFGS][puiPic2FNum[uiPicNum]]+= ( bInd ? dDistortionBaseInd - dDistortionEnhInd : 0 );
        }
      }

      //----- non-key pictures -----
      for( uiTLevel = 1; uiTLevel <= m_auiNumTempLevel[uiLayer]; uiTLevel++ )
      {
        UInt uiStepSize2   = ( 1 << ( m_auiNumTempLevel[uiLayer] - uiTLevel ) );
        for( UInt uiPicNum = uiStepSize2; uiPicNum < m_auiNumFrames[uiLayer]; uiPicNum += (uiStepSize2<<1) )
        {
          Double dDistortionBaseDep  = 0;
          Double dDistortionBaseInd  = 0;
          Double dDistortionEnhDep   = 0;
          Double dDistortionEnhInd   = 0;
          UInt    uiStartPicNum   = uiPicNum - uiStepSize2 + 1;
          UInt    uiEndPicNum     = min( uiPicNum + uiStepSize2, m_auiNumFrames[uiLayer] ) - 1;
          for( UInt uiCheckPicNum = uiStartPicNum; uiCheckPicNum <= uiEndPicNum; uiCheckPicNum++ )
          {
            dDistortionBaseDep += log10( (Double)aaaauiDistortionDep[uiLayer][uiFGS  ][uiTLevel-1][uiCheckPicNum] );
            dDistortionEnhDep  += log10( (Double)aaaauiDistortionDep[uiLayer][uiFGS  ][uiTLevel  ][uiCheckPicNum] );

            dDistortionBaseInd += log10( (Double)aaaauiDistortionInd[uiLayer][uiFGS-1][uiTLevel  ][uiCheckPicNum] );
            dDistortionEnhInd  += log10( (Double)aaaauiDistortionInd[uiLayer][uiFGS  ][uiTLevel  ][uiCheckPicNum] );
          }
          m_aaadDeltaDist[uiLayer][uiFGS][puiPic2FNum[uiPicNum]] = 0;
          m_aaadDeltaDist[uiLayer][uiFGS][puiPic2FNum[uiPicNum]]+= ( bDep ? dDistortionBaseDep - dDistortionEnhDep : 0 );
          m_aaadDeltaDist[uiLayer][uiFGS][puiPic2FNum[uiPicNum]]+= ( bInd ? dDistortionBaseInd - dDistortionEnhInd : 0 );
        }
      }
    }

    //----- delete auxiliary array -----
    delete [] puiPic2FNum;
  }
  printf("\n");

  
  //----- delete temporarily distortion arrays -----
  for( uiLayer  = 0;  uiLayer   <  m_uiNumLayers;              uiLayer  ++ )
  for( uiFGS    = 0;  uiFGS     <= m_auiNumFGSLayers[uiLayer]; uiFGS    ++ )
  for( uiTLevel = 0;  uiTLevel  <= m_auiNumTempLevel[uiLayer]; uiTLevel ++ )
  {
    delete [] aaaauiDistortionDep[uiLayer][uiFGS][uiTLevel];
    delete [] aaaauiDistortionInd[uiLayer][uiFGS][uiTLevel];
  }

  return Err::m_nOK;
}


ErrVal
QualityLevelAssigner::xInitRateValues()
{
  printf( "determine packet sizes ..." );

  Int64             i64StartPos   = 0;
  BinData*          pcBinData     = 0;
  SEI::SEIMessage*  pcScalableSEI = 0;
  PacketDescription cPacketDescription;
  UInt              auiFrameNum[MAX_LAYERS] = { MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX };

  //===== init =====
  RNOK( m_pcH264AVCPacketAnalyzer->init() );
  ReadBitstreamFile* pcReadBitStream = 0;
  RNOK( ReadBitstreamFile::create( pcReadBitStream ) );
  RNOK( pcReadBitStream->init( m_pcParameter->getInputBitStreamName() ) );


  //===== init values ======
  for( UInt uiLayer = 0; uiLayer <  m_uiNumLayers;               uiLayer ++ )
  for( UInt uiFGS   = 0; uiFGS   <= m_auiNumFGSLayers[uiLayer];  uiFGS   ++ )
  for( UInt uiFrame = 0; uiFrame <  m_auiNumFrames   [uiLayer];  uiFrame ++ )
  {
    m_aaauiPacketSize[uiLayer][uiFGS][uiFrame] = 0;
  }

  //===== loop over packets =====
  while( true )
  {
    //----- read packet -----
    Bool bEOS = false;
    RNOK( pcReadBitStream->extractPacket( pcBinData, bEOS ) );
    if( bEOS )
    {
      break;
    }

    //----- get packet description -----
    RNOK( m_pcH264AVCPacketAnalyzer->process( pcBinData, cPacketDescription, pcScalableSEI ) );
    delete pcScalableSEI; pcScalableSEI = 0;

    //----- get packet size -----
    Int64 i64EndPos     = pcReadBitStream->getFilePos();
    UInt  uiPacketSize  = (UInt)( i64EndPos - i64StartPos );
    i64StartPos         = i64EndPos;

    //----- analyse packets -----
    if( ! cPacketDescription.ParameterSet && cPacketDescription.NalUnitType != NAL_UNIT_SEI )
    {
      if( cPacketDescription.FGSLayer == 0 )
      {
        auiFrameNum[cPacketDescription.Layer]++;
      }
      m_aaauiPacketSize[cPacketDescription.Layer][cPacketDescription.FGSLayer][auiFrameNum[cPacketDescription.Layer]] += uiPacketSize;
    }

    //----- delete bin data -----
    RNOK( pcReadBitStream->releasePacket( pcBinData ) );
  }

  
  //===== uninit =====
  RNOK( m_pcH264AVCPacketAnalyzer->uninit() );
  RNOK( pcReadBitStream->uninit() );
  RNOK( pcReadBitStream->destroy() );

  printf("\n");
  return Err::m_nOK;
}


ErrVal
QualityLevelAssigner::xGetNextValidPacket( BinData*&          rpcBinData,
                                           ReadBitstreamFile* pcReadBitStream,
                                           UInt               uiLayer,
                                           UInt               uiFGSLayer,
                                           UInt               uiLevel,
                                           Bool               bIndependent,
                                           Bool&              rbEOS,
                                           UInt*              auiFrameNum )
{
  Bool              bValid        = false;
  SEI::SEIMessage*  pcScalableSEI = 0;
  PacketDescription cPacketDescription;

  while( !bValid )
  {
    //===== get next packet =====
    RNOK( pcReadBitStream->extractPacket( rpcBinData, rbEOS ) );
    if( rbEOS )
    {
      break;
    }

    //===== analyze packet =====
    RNOK( m_pcH264AVCPacketAnalyzer->process( rpcBinData, cPacketDescription, pcScalableSEI ) );
    delete pcScalableSEI; pcScalableSEI = 0;


    //===== check whether packet is required =====
    if( cPacketDescription.NalUnitType == NAL_UNIT_SEI )
    {
      bValid      = true;
    }
    else if( cPacketDescription.NalUnitType == NAL_UNIT_SPS )
    {
      bValid      = false;
      for( UInt ui = 0; ui <= uiLayer; ui++ )
      {
        if( m_auiSPSRequired[cPacketDescription.SPSid] & (1<<ui) )
        {
          bValid  = true;
          break;
        }
      }
    }
    else if( cPacketDescription.NalUnitType == NAL_UNIT_PPS )
    {
      bValid      = false;
      for( UInt ui = 0; ui <= uiLayer; ui++ )
      {
        if( m_auiPPSRequired[cPacketDescription.PPSid] & (1<<ui) )
        {
          bValid  = true;
          break;
        }
      }
    }
    else // slice data
    {
      //===== update frame num =====
      if( ! cPacketDescription.FGSLayer )
      {
        auiFrameNum[cPacketDescription.Layer]++;
      }

      //===== check temporal level =====
      {
        UInt uiTL                 = 0;
        UInt uiFN                 = auiFrameNum[cPacketDescription.Layer];
        UInt uiNumFramesComplete  = ( ( m_auiNumFrames[cPacketDescription.Layer] - 1 ) / m_auiGOPSize[cPacketDescription.Layer] ) * m_auiGOPSize[cPacketDescription.Layer] + 1;
        UInt uiRemainingFrames    = m_auiNumFrames[cPacketDescription.Layer] - uiNumFramesComplete;
        UInt uiFNMod              = ( uiFN - 1 ) % m_auiGOPSize[cPacketDescription.Layer];
        if( uiFN )
        {
          if( uiFN < uiNumFramesComplete )
          {
            for( ; uiFNMod > 0; uiFNMod >>= 1, uiTL++ );
          }
          else
          {
            UInt auiTLevel[128];
            UInt uiEntry = 0;
            ::memset( auiTLevel, 0xFF, 128*sizeof(UInt) );
            for( UInt   uiTempLevel = 0;      uiTempLevel <= m_auiNumTempLevel[cPacketDescription.Layer]; uiTempLevel++ )
            {
              UInt      uiStep      = ( 1 << ( m_auiNumTempLevel[cPacketDescription.Layer] - uiTempLevel ) );
              for( UInt uiPos       = uiStep; uiPos      <= m_auiGOPSize[cPacketDescription.Layer];      uiPos += (uiStep<<1) )
              {
                if( uiPos - 1 < uiRemainingFrames )
                {
                  auiTLevel[uiEntry++] = uiTempLevel;
                }
              }
            }
            uiTL = auiTLevel[uiFNMod];
            ROT( uiTL == MSYS_UINT_MAX );
          }
        }
        ROT( cPacketDescription.Scalable && cPacketDescription.Level != uiTL );
        cPacketDescription.Level = uiTL;
      }

      //===== get valid status =====
      if( bIndependent )
      {
        bValid      = ( cPacketDescription.Layer    <= uiLayer );
        if( cPacketDescription.Layer == uiLayer )
        {
          bValid    = ( cPacketDescription.Level    == uiLevel &&
                        cPacketDescription.FGSLayer <= uiFGSLayer ) || ( cPacketDescription.FGSLayer == 0 );
        }
      }
      else
      {
        bValid      = ( cPacketDescription.Layer <= uiLayer );
        if( cPacketDescription.Layer == uiLayer )
        {
          bValid    = ( cPacketDescription.FGSLayer <= uiFGSLayer );
          if( cPacketDescription.FGSLayer == uiFGSLayer )
          {
            bValid  = ( cPacketDescription.Level <= uiLevel );
          }
        }
      }
    }

    if( !bValid )
    {
      RNOK( pcReadBitStream->releasePacket( rpcBinData ) );
    }
  }

  return Err::m_nOK;
}



ErrVal
QualityLevelAssigner::xGetDistortion( UInt&         ruiDistortion,
                                      const UChar*  pucReconstruction,
                                      const UChar*  pucReference,
                                      UInt          uiHeight,
                                      UInt          uiWidth,
                                      UInt          uiStride )
{
  ruiDistortion = 0;
  for( UInt y = 0; y < uiHeight; y++ )
  {
    for( UInt x = 0; x < uiWidth; x++ )
    {
      Int iDiff      = ( pucReconstruction[x] - pucReference[x] );
      ruiDistortion += (UInt)( iDiff * iDiff );
    }
    pucReconstruction += uiStride;
    pucReference      += uiStride;
  }
  return Err::m_nOK;
}



ErrVal
QualityLevelAssigner::xInitDistortion( UInt*  auiDistortion,
                                       UInt   uiLayer,
                                       UInt   uiFGSLayer,
                                       UInt   uiLevel,
                                       Bool   bIndependent )
{
  ROT( m_pcParameter->getOriginalFileName( uiLayer ).empty() );
  
  if( uiLevel == MSYS_UINT_MAX )
    printf( "determine distortion (layer %d - FGS %d - base layer  ) ...", uiLayer, uiFGSLayer );
  else
    printf( "determine distortion (layer %d - FGS %d - lev%2d - %s ) ...", uiLayer, uiFGSLayer, uiLevel, bIndependent?"ind":"dep" );

#if WIN32
  Char              tmp_file_name[]   = "decout.tmp";
#endif
  Bool              bEOS              = false;
  Bool              bToDecode         = false;
  SEI::SEIMessage*  pcScalableSEI     = 0;
  UInt              uiFrame           = 0;
  UInt              uiNalUnitType     = 0;
  UInt              uiMbX             = 0;
  UInt              uiMbY             = 0;
  UInt              uiSize            = 0;
#if NON_REQUIRED_SEI_ENABLE
  UInt              uiNonRequiredPic  = 0;
#endif
  UInt              uiLumOffset       = 0;
  UInt              uiCbOffset        = 0;
  UInt              uiCrOffset        = 0;
  Bool              bYuvDimSet        = false;
  PicBuffer*        pcPicBuffer       = 0;
  PicBuffer*        pcPicBufferOrig   = 0;
  WriteYuvIf*       pcWriteYuv        = 0;
  PicBufferList     cPicBufferOutputList;
  PicBufferList     cPicBufferUnusedList;
  PicBufferList     cPicBufferReleaseList;
  UInt              auiFrameNumAnalysis[MAX_LAYERS] = { MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX };
  UInt              auiFrameNumDecoding[MAX_LAYERS] = { MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX };

  //===== init =====
  RNOK( m_pcH264AVCPacketAnalyzer->init() );
  RNOK( m_pcH264AVCDecoder       ->init() );
  ReadBitstreamFile*  pcReadBitStream = 0;
  ReadYuvFile*        pcReadYuv       = 0;
  RNOK( ReadBitstreamFile ::create( pcReadBitStream ) );
  RNOK( ReadYuvFile       ::create( pcReadYuv       ) );
  RNOK( pcReadBitStream ->init( m_pcParameter->getInputBitStreamName() ) );
  RNOK( pcReadYuv       ->init( m_pcParameter->getOriginalFileName  ( uiLayer ), m_auiFrameHeight[uiLayer], m_auiFrameWidth[uiLayer] ) );
  
  if( m_bOutputReconstructions )
  {
    Char  acName[1024];
    sprintf( acName, "rec_Layer%d_FGS%d_Level%d_Mode%d", uiLayer, uiFGSLayer, uiLevel, (bIndependent?0:1) );
    RNOK( WriteYuvToFile::create( pcWriteYuv, std::string( acName ) ) );
  }


  //===== loop over packets =====
  while( ! bEOS )
  {
    Int             iPos;
    Bool            bFinishChecking;
    BinData*        pcBinData;
    BinDataAccessor cBinDataAccessor;

    //----- analyse access unit -----
    RNOK( pcReadBitStream->getPosition( iPos ) );
    do
    {
      RNOK( xGetNextValidPacket( pcBinData, pcReadBitStream, uiLayer, uiFGSLayer, uiLevel, bIndependent, bEOS, auiFrameNumAnalysis ) );
      pcBinData->setMemAccessor( cBinDataAccessor );

      bFinishChecking = false;
      if( m_pcH264AVCDecoder->getNumOfNALInAU() == 0 )
      {
        m_pcH264AVCDecoder->setDependencyInitialized( false );
        m_pcH264AVCDecoder->initNumberOfFragment    ();
      }
      RNOK( m_pcH264AVCDecoder->checkSliceLayerDependency( &cBinDataAccessor, bFinishChecking ) );
      RNOK( pcReadBitStream->releasePacket( pcBinData ) );
    	
    } while( !bFinishChecking );
    
    
#define MAX_FRAGMENTS 10 // see H264AVCDecoderTest::go()

    Bool            bFragmented       = false;
    Bool            bDiscardable      = false;
    Bool            bStart            = false;
    Bool            bFirst            = true;
    UInt            uiTotalLength     = 0;
    UInt            uiFragmentNumber  = 0;
    BinData*        apcBinDataTmp       [MAX_FRAGMENTS];
    BinDataAccessor acBinDataAccessorTmp[MAX_FRAGMENTS];
    UInt            auiStartPos         [MAX_FRAGMENTS];
    UInt            auiEndPos           [MAX_FRAGMENTS];
    
    for( pcBinData = 0, bEOS = false; !bStart && !bEOS; )
    {
      if( bFirst )
      {
        RNOK( pcReadBitStream->setPosition( iPos ) );
        bFirst = false;
      }
      RNOK( xGetNextValidPacket( apcBinDataTmp[uiFragmentNumber], pcReadBitStream, uiLayer, uiFGSLayer, uiLevel, bIndependent, bEOS, auiFrameNumDecoding ) );
      ::memcpy( auiFrameNumAnalysis, auiFrameNumDecoding, MAX_LAYERS*sizeof(UInt) );
      apcBinDataTmp[uiFragmentNumber]->setMemAccessor( acBinDataAccessorTmp[uiFragmentNumber] );

#if NON_REQUIRED_SEI_ENABLE
      RNOK( m_pcH264AVCDecoder->initPacket( &acBinDataAccessorTmp[uiFragmentNumber],
                                            uiNalUnitType, uiMbX, uiMbY, uiSize, uiNonRequiredPic,
                                            bStart, auiStartPos[uiFragmentNumber], auiEndPos[uiFragmentNumber],
                                            bFragmented, bDiscardable ) );
#else
      RNOK( m_pcH264AVCDecoder->initPacket( &acBinDataAccessorTmp[uiFragmentNumber],
                                            uiNalUnitType, uiMbX, uiMbY, uiSize,
                                            bStart, auiStartPos[uiFragmentNumber], auiEndPos[uiFragmentNumber],
                                            bFragmented, bDiscardable ) );
#endif

      uiTotalLength += ( auiEndPos[uiFragmentNumber] - auiStartPos[uiFragmentNumber] );

      if( !bStart )
      {
        uiFragmentNumber++;
      }
      else
      {
        if( apcBinDataTmp[0]->size() != 0 )
        {
          pcBinData = new BinData();
          pcBinData->set( new UChar[uiTotalLength], uiTotalLength );
          UInt uiOffset = 0;
          for( UInt uiFragment = 0; uiFragment <= uiFragmentNumber; uiFragment++ )
          {
            ::memcpy  ( pcBinData->data()+uiOffset,
                        apcBinDataTmp[uiFragment]->data() + auiStartPos[uiFragment],
                        auiEndPos[uiFragment] - auiStartPos[uiFragment] );
            uiOffset += auiEndPos[uiFragment] - auiStartPos[uiFragment];

            RNOK( pcReadBitStream->releasePacket( apcBinDataTmp[uiFragment] ) );
            apcBinDataTmp[uiFragment] = 0;
            m_pcH264AVCDecoder->decreaseNumOfNALInAU();
          }

          pcBinData->setMemAccessor( cBinDataAccessor );
          bToDecode = false;
          if( ( uiTotalLength != 0 ) && ( !bDiscardable || bFragmented ) )
          {
            m_pcH264AVCDecoder->initPacket( &cBinDataAccessor );
            bToDecode = true;
          }
        }
      }
    }
    
    if( bToDecode )
    {
      //----- get pic buffer -----
      pcPicBuffer = 0;
      if( uiNalUnitType == 1 || uiNalUnitType == 5 || uiNalUnitType == 20 || uiNalUnitType == 21 )
      {
        RNOK( xGetNewPicBuffer( pcPicBuffer, uiSize ) );
        if( !bYuvDimSet )
        {
          RNOK( xGetNewPicBuffer( pcPicBufferOrig, uiSize ) );
          UInt uiLumSize  = ((uiMbX<<3)+  YUV_X_MARGIN) * ((uiMbY<<3)    + YUV_Y_MARGIN ) * 4;
          uiLumOffset     = ((uiMbX<<4)+2*YUV_X_MARGIN) * YUV_Y_MARGIN   + YUV_X_MARGIN;  
          uiCbOffset      = ((uiMbX<<3)+  YUV_X_MARGIN) * YUV_Y_MARGIN/2 + YUV_X_MARGIN/2 +   uiLumSize; 
          uiCrOffset      = ((uiMbX<<3)+  YUV_X_MARGIN) * YUV_Y_MARGIN/2 + YUV_X_MARGIN/2 + 5*uiLumSize/4;
          bYuvDimSet      = true;
        }
      }

      //----- decode packet -----
      {
        //----- re-direct stdout -----
#if WIN32 // for linux, this have to be slightly re-formulated
        Int   orig_stdout     = _dup(1);
        FILE* stdout_copy     = freopen( tmp_file_name, "wt", stdout );
#endif
        //----- decode -----
        RNOK( m_pcH264AVCDecoder->process( pcPicBuffer,
                                           cPicBufferOutputList,
                                           cPicBufferUnusedList,
                                           cPicBufferReleaseList ) );
        //---- restore stdout -----
#if WIN32 // for linux, this have to be slightly re-formulated
        fclose( stdout );
        _dup2( orig_stdout, 1 );
        _iob[1] = *fdopen( 1, "wt" );
        fclose(  fdopen( orig_stdout, "w" ) );
#endif
      }

      //----- determine distortion (and output for debugging) -----
      while( ! cPicBufferOutputList.empty() )
      {
        PicBuffer* pcPicBuffer = cPicBufferOutputList.front();
        cPicBufferOutputList.pop_front();
        if( pcPicBuffer )
        {
          //----- output (for debugging) -----
          if( pcWriteYuv )
          {
            RNOK( pcWriteYuv->writeFrame( *pcPicBuffer + uiLumOffset,
                                          *pcPicBuffer + uiCbOffset,
                                          *pcPicBuffer + uiCrOffset,
                                           uiMbY << 4,
                                           uiMbX << 4,
                                          (uiMbX << 4) + YUV_X_MARGIN*2 ) );
          }

          //----- read in reference picture ------
          RNOK( pcReadYuv->readFrame    ( *pcPicBufferOrig + uiLumOffset,
                                          *pcPicBufferOrig + uiCbOffset,
                                          *pcPicBufferOrig + uiCrOffset,
                                           uiMbY << 4,
                                           uiMbX << 4,
                                          (uiMbX << 4) + YUV_X_MARGIN*2 ) );

          //----- get distortion -----
          RNOK( xGetDistortion          (  auiDistortion[uiFrame],
                                          *pcPicBuffer     + uiLumOffset,
                                          *pcPicBufferOrig + uiLumOffset,
                                           uiMbY << 4,
                                           uiMbX << 4,
                                          (uiMbX << 4) + YUV_X_MARGIN*2 ) );

          //----- increment output picture number -----
          uiFrame++;
          if( uiLevel == MSYS_UINT_MAX )
            printf( "\rdetermine distortion (layer %d - FGS %d - base layer  ) --> frame %d completed", uiLayer, uiFGSLayer,                                    uiFrame );
          else
            printf( "\rdetermine distortion (layer %d - FGS %d - lev%2d - %s ) --> frame %d completed", uiLayer, uiFGSLayer, uiLevel, bIndependent?"ind":"dep", uiFrame );
        }
      }
    } // if( bToDecode )

    //----- free buffers -----
    RNOK( xRemovePicBuffer( cPicBufferReleaseList ) );
    RNOK( xRemovePicBuffer( cPicBufferUnusedList  ) );

    //----- delete bin data -----
    if( pcBinData )
    {
      RNOK( pcReadBitStream->releasePacket( pcBinData ) );
      pcBinData = 0;
    }
  }
  //----- remove original pic buffer -----
  PicBufferList cPicBufferListOrig; cPicBufferListOrig.push_back( pcPicBufferOrig );
  RNOK( xRemovePicBuffer( cPicBufferListOrig ) );
#if WIN32
  remove( tmp_file_name );
#endif

  
  //===== uninit =====
  RNOK( m_pcH264AVCPacketAnalyzer ->uninit  () );
  RNOK( m_pcH264AVCDecoder        ->uninit  () );
  RNOK( pcReadBitStream           ->uninit  () );
  RNOK( pcReadYuv                 ->uninit  () );
  RNOK( pcReadBitStream           ->destroy () );
  RNOK( pcReadYuv                 ->destroy () );
  if( pcWriteYuv )
  {
    RNOK( pcWriteYuv->destroy() );
  }

  //---- re-create decoder (there's something wrong) -----
  RNOK( m_pcH264AVCDecoder->destroy() );
  RNOK( CreaterH264AVCDecoder::create( m_pcH264AVCDecoder ) );

  printf("\n");

  return Err::m_nOK;
}


ErrVal
QualityLevelAssigner::xWriteDataFile( const std::string&  cFileName )
{
  printf( "write data to file \"%s\" ...", cFileName.c_str() );

  FILE* pFile = fopen( cFileName.c_str(), "wt" );
  if( !pFile )
  {
    fprintf( stderr, "\nERROR: Cannot open file \"%s\" for writing!\n\n", cFileName.c_str() );
    return Err::m_nERR;
  }

  for( UInt uiLayer = 0; uiLayer <  m_uiNumLayers;               uiLayer ++ )
  for( UInt uiFGS   = 0; uiFGS   <= m_auiNumFGSLayers[uiLayer];  uiFGS   ++ )
  for( UInt uiFrame = 0; uiFrame <  m_auiNumFrames   [uiLayer];  uiFrame ++ )
  {
    fprintf( pFile,
             "%d  %d  %5d  %6d  %lf\n",
              uiLayer, uiFGS, uiFrame,
              m_aaauiPacketSize [uiLayer][uiFGS][uiFrame],
              m_aaadDeltaDist   [uiLayer][uiFGS][uiFrame] );

  }

  fclose( pFile );

  printf("\n");
  return Err::m_nOK;
}


ErrVal
QualityLevelAssigner::xReadDataFile( const std::string&  cFileName )
{
  printf( "read data from file \"%s\" ...", cFileName.c_str() );

  FILE* pFile = fopen( cFileName.c_str(), "rt" );
  if( !pFile )
  {
    fprintf( stderr, "\nERROR: Cannot open file \"%s\" for reading!\n\n", cFileName.c_str() );
    return Err::m_nERR;
  }

  Bool    bEOS    = false;
  UInt    uiLayer, uiFGS, uiFrame, uiPacketSize, uiNumPackets;
  Double  dDeltaDist;
  for( uiNumPackets = 0; !bEOS; uiNumPackets++ )
  {
    Int iNumRead = fscanf( pFile,
                           " %d %d %d %d %lf",
                           &uiLayer, &uiFGS, &uiFrame, &uiPacketSize, &dDeltaDist );
    if( iNumRead == 5 )
    {
      ROF( uiLayer <  m_uiNumLayers );
      ROF( uiFGS   <= m_auiNumFGSLayers[uiLayer] );
      ROF( uiFrame <  m_auiNumFrames   [uiLayer] );
      m_aaauiPacketSize [uiLayer][uiFGS][uiFrame]  = uiPacketSize;
      m_aaadDeltaDist   [uiLayer][uiFGS][uiFrame]  = dDeltaDist;
    }
    else
    {
      bEOS = true;
      uiNumPackets--;
    }
  }

  //----- check number of elements -----
  UInt uiTargetPackets = 0;
  for( uiLayer = 0; uiLayer < m_uiNumLayers; uiLayer++ )
  {
    uiTargetPackets += ( 1 + m_auiNumFGSLayers[uiLayer] ) * m_auiNumFrames[uiLayer];
  }
  if( uiTargetPackets != uiNumPackets )
  {
    fprintf( stderr, "\nERROR: File \"%s\" contains incomplete data!\n\n", cFileName.c_str() );
    return Err::m_nERR;
  }

  fclose( pFile );

  printf("\n");
  return Err::m_nOK;
}


ErrVal
QualityLevelAssigner::xDetermineQualityIDs()
{
  printf( "determine quality levels ..." );

  //===== determine minimum and maximum quality level id's =====
  UInt  auiMinQualityLevel[MAX_LAYERS];
  UInt  auiMaxQualityLevel[MAX_LAYERS];
  {
    for( Int iLayer = (Int)m_uiNumLayers-1; iLayer >= 0; iLayer-- )
    {
      UInt  uiMinQLLayer          = ( iLayer == (Int)m_uiNumLayers-1 ? 0 : auiMaxQualityLevel[iLayer+1]+1 );
      UInt  uiNumQLInLayer        = ( 63 - uiMinQLLayer ) / ( iLayer + 1 );
      auiMinQualityLevel[iLayer]  = uiMinQLLayer;
      auiMaxQualityLevel[iLayer]  = uiMinQLLayer + uiNumQLInLayer - 1;
    }
  }

  //===== determine optimized quality levels per layer =====
  for( UInt uiLayer = 0; uiLayer < m_uiNumLayers; uiLayer++ )
  {
    //----- create quality estimation object -----
    QualityLevelEstimation  cQualityLevelEstimation;
    RNOK( cQualityLevelEstimation.init( m_auiNumFGSLayers[uiLayer], m_auiNumFrames[uiLayer] ) );

    //----- initialize with packets -----
    {
      for( UInt uiFGSLayer = 1; uiFGSLayer <= m_auiNumFGSLayers[uiLayer]; uiFGSLayer++ )
      for( UInt uiFrame    = 0; uiFrame    <  m_auiNumFrames   [uiLayer]; uiFrame   ++ )
      {
        RNOK( cQualityLevelEstimation.addPacket( uiFGSLayer, uiFrame,
                                                 m_aaauiPacketSize [uiLayer][uiFGSLayer][uiFrame],
                                                 m_aaadDeltaDist   [uiLayer][uiFGSLayer][uiFrame] ) );
      }
    }

    //----- determine quality levels -----
    RNOK( cQualityLevelEstimation.optimizeQualityLevel( auiMinQualityLevel[uiLayer], auiMaxQualityLevel[uiLayer] ) );

    //----- assign quality levels -----
    {
      for( UInt uiFGSLayer = 0; uiFGSLayer <= m_auiNumFGSLayers[uiLayer]; uiFGSLayer++ )
      for( UInt uiFrame    = 0; uiFrame    <  m_auiNumFrames   [uiLayer]; uiFrame   ++ )
      {
        m_aaauiQualityID[uiLayer][uiFGSLayer][uiFrame] = ( uiFGSLayer ? cQualityLevelEstimation.getQualityLevel( uiFGSLayer, uiFrame ) : 63 );
      }
    }
  }

  printf("\n");
  return Err::m_nOK;
}


ErrVal
QualityLevelAssigner::xWriteQualityLayerStreamPID()
{
  printf( "write stream with quality layer (PID) \"%s\" ...", m_pcParameter->getOutputBitStreamName().c_str() );

  BinData*          pcBinData     = 0;
  SEI::SEIMessage*  pcScalableSEI = 0;
  PacketDescription cPacketDescription;
  UInt              auiFrameNum[MAX_LAYERS] = { MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX };

  //===== init =====
  RNOK( m_pcH264AVCPacketAnalyzer->init() );
  ReadBitstreamFile*    pcReadBitStream   = 0;
  WriteBitstreamToFile* pcWriteBitStream  = 0;
  RNOK( ReadBitstreamFile   ::create( pcReadBitStream  ) );
  RNOK( WriteBitstreamToFile::create( pcWriteBitStream ) );
  RNOK( pcReadBitStream ->init( m_pcParameter->getInputBitStreamName  () ) );
  RNOK( pcWriteBitStream->init( m_pcParameter->getOutputBitStreamName () ) );


  //===== loop over packets =====
  while( true )
  {
    //----- read packet -----
    Bool bEOS = false;
    RNOK( pcReadBitStream->extractPacket( pcBinData, bEOS ) );
    if( bEOS )
    {
      break;
    }

    //----- get packet description -----
    RNOK( m_pcH264AVCPacketAnalyzer->process( pcBinData, cPacketDescription, pcScalableSEI ) );
    delete pcScalableSEI; pcScalableSEI = 0;

    //----- set packet size -----
    while( pcBinData->data()[ pcBinData->size() - 1 ] == 0x00 )
    {
      RNOK( pcBinData->decreaseEndPos( 1 ) ); // remove trailing zeros
    }

    //----- analyse packets -----
    if( cPacketDescription.FGSLayer )
    {
      pcBinData->data()[1] |= ( m_aaauiQualityID[cPacketDescription.Layer][cPacketDescription.FGSLayer][auiFrameNum[cPacketDescription.Layer]] << 2 );
    }
    else
    {
      if( ! cPacketDescription.ParameterSet && cPacketDescription.NalUnitType != NAL_UNIT_SEI &&
          ! cPacketDescription.FGSLayer )
      {
        auiFrameNum[cPacketDescription.Layer]++;
      }
    }

    //----- write and delete bin data -----
    RNOK( pcWriteBitStream->writePacket   ( &m_cBinDataStartCode ) );
    RNOK( pcWriteBitStream->writePacket   ( pcBinData ) );
    RNOK( pcReadBitStream ->releasePacket ( pcBinData ) );
  }

  
  //===== uninit =====
  RNOK( m_pcH264AVCPacketAnalyzer->uninit() );
  RNOK( pcReadBitStream ->uninit  () );
  RNOK( pcWriteBitStream->uninit  () );
  RNOK( pcReadBitStream ->destroy () );
  RNOK( pcWriteBitStream->destroy () );

  printf("\n");
  return Err::m_nOK;
}



ErrVal
QualityLevelAssigner::xWriteQualityLayerStreamSEI()
{
  printf( "write stream with quality layer (SEI) \"%s\" ...", m_pcParameter->getOutputBitStreamName().c_str() );

  UInt              uiNumAccessUnits  = 0;
  UInt              uiPrevLayer       = 0;
  UInt              uiPrevLevel       = 0;
  BinData*          pcBinData         = 0;
  SEI::SEIMessage*  pcScalableSEI     = 0;
  PacketDescription cPacketDescription;
  UInt              auiFrameNum[MAX_LAYERS] = { MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX, MSYS_UINT_MAX };

  //===== init =====
  RNOK( m_pcH264AVCPacketAnalyzer->init() );
  ReadBitstreamFile*    pcReadBitStream   = 0;
  WriteBitstreamToFile* pcWriteBitStream  = 0;
  RNOK( ReadBitstreamFile   ::create( pcReadBitStream  ) );
  RNOK( WriteBitstreamToFile::create( pcWriteBitStream ) );
  RNOK( pcReadBitStream ->init( m_pcParameter->getInputBitStreamName  () ) );
  RNOK( pcWriteBitStream->init( m_pcParameter->getOutputBitStreamName () ) );


  //===== loop over packets =====
  while( true )
  {
    //----- read packet -----
    Bool bEOS = false;
    RNOK( pcReadBitStream->extractPacket( pcBinData, bEOS ) );
    if( bEOS )
    {
      break;
    }

    //----- get packet description -----
    RNOK( m_pcH264AVCPacketAnalyzer->process( pcBinData, cPacketDescription, pcScalableSEI ) );
    delete pcScalableSEI; pcScalableSEI = 0;

    //----- set packet size -----
    while( pcBinData->data()[ pcBinData->size() - 1 ] == 0x00 )
    {
      RNOK( pcBinData->decreaseEndPos( 1 ) ); // remove trailing zeros
    }

    //----- detect first slice data of access unit -----
    Bool bNewAccessUnit = ( !cPacketDescription.ParameterSet                &&
                            !cPacketDescription.ApplyToNext                 &&
                             cPacketDescription.NalUnitType != NAL_UNIT_SEI &&
                             cPacketDescription.FGSLayer    == 0U );
    if(  bNewAccessUnit )
    {
      bNewAccessUnit  =                   ( cPacketDescription.Layer == 0 );
      bNewAccessUnit  = bNewAccessUnit || ( cPacketDescription.Layer >= uiPrevLayer && cPacketDescription.Level >  uiPrevLevel );
      bNewAccessUnit  = bNewAccessUnit || ( cPacketDescription.Layer == uiPrevLayer && cPacketDescription.Level == uiPrevLevel );
      bNewAccessUnit  = bNewAccessUnit || ( cPacketDescription.Layer <  uiPrevLayer );
    }

    //----- insert quality layer SEI and increase frame number -----
    if( bNewAccessUnit )
    {
      for( UInt uiLayer = cPacketDescription.Layer; uiLayer < m_uiNumLayers; uiLayer++ )
      {
        auiFrameNum[uiLayer]++;
        xInsertQualityLayerSEI( pcWriteBitStream, uiLayer, auiFrameNum[uiLayer] );
      }
      uiNumAccessUnits++;
    }

    //----- write and delete bin data -----
    RNOK( pcWriteBitStream->writePacket   ( &m_cBinDataStartCode ) );
    RNOK( pcWriteBitStream->writePacket   ( pcBinData ) );
    RNOK( pcReadBitStream ->releasePacket ( pcBinData ) );

    //----- update previous layer information -----
    uiPrevLayer = cPacketDescription.Layer;
    uiPrevLevel = cPacketDescription.Level;
  }

  
  //===== uninit =====
  RNOK( m_pcH264AVCPacketAnalyzer->uninit() );
  RNOK( pcReadBitStream ->uninit  () );
  RNOK( pcWriteBitStream->uninit  () );
  RNOK( pcReadBitStream ->destroy () );
  RNOK( pcWriteBitStream->destroy () );


  printf(" (%d AU's)\n", uiNumAccessUnits );
  return Err::m_nOK;
}



ErrVal
QualityLevelAssigner::xInsertQualityLayerSEI( WriteBitstreamToFile* pcWriteBitStream,
                                              UInt                  uiLayer,
                                              UInt                  uiFrameNum )
{
  //===== init binary data =====
  const UInt          uiQLSEIMessageBufferSize = 1024;
  UChar               aucQLSEIMessageBuffer[uiQLSEIMessageBufferSize];
  BinData             cBinData;
  ExtBinDataAccessor  cExtBinDataAccessor;
  cBinData.reset          ();
  cBinData.set            ( aucQLSEIMessageBuffer, uiQLSEIMessageBufferSize );
  cBinData.setMemAccessor ( cExtBinDataAccessor );

  //===== create SEI message =====
  SEI::QualityLevelSEI* pcQualityLevelSEI;
  SEI::MessageList      cSEIMessageList;
  RNOK( SEI::QualityLevelSEI::create( pcQualityLevelSEI ) );
  cSEIMessageList.push_back( pcQualityLevelSEI );

  //===== set content of SEI message =====
  UInt uiNumLayers;
  for( uiNumLayers = 0; uiNumLayers <= m_auiNumFGSLayers[uiLayer] && m_aaauiQualityID  [uiLayer][uiNumLayers][uiFrameNum] != MSYS_UINT_MAX; uiNumLayers++ )
  {
    pcQualityLevelSEI->setQualityLevel          ( uiNumLayers,       m_aaauiQualityID  [uiLayer][uiNumLayers][uiFrameNum] );
    pcQualityLevelSEI->setDeltaBytesRateOfLevel ( uiNumLayers,       m_aaauiPacketSize [uiLayer][uiNumLayers][uiFrameNum] );
  }
  pcQualityLevelSEI->setDependencyId            ( uiLayer );
  pcQualityLevelSEI->setNumLevel                ( uiNumLayers );

  //===== encode SEI message =====
  UInt uiBits = 0;
  RNOK( m_pcNalUnitEncoder->initNalUnit ( &cExtBinDataAccessor ) );
  RNOK( m_pcNalUnitEncoder->write       ( cSEIMessageList ) );
  RNOK( m_pcNalUnitEncoder->closeNalUnit( uiBits ) );

  //===== write SEI message =====
  RNOK( pcWriteBitStream->writePacket( &m_cBinDataStartCode ) );
  RNOK( pcWriteBitStream->writePacket( &cExtBinDataAccessor ) );

  //===== reset =====
  cBinData.reset();

  return Err::m_nOK;
}
