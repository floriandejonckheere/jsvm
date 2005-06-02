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



#include "BStreamExtractor.h"
#include "Extractor.h"
#include <math.h>





Extractor::Extractor()
: m_pcReadBitstream       ( 0 )
, m_pcWriteBitstream      ( 0 )
, m_pcExtractorParameter  ( 0 )
// HS: packet trace
, m_pcTraceFile           ( 0 )
, m_pcExtractionTraceFile ( 0 )
, m_uiMaxSize             ( 0 )
{
}



Extractor::~Extractor()
{
}



ErrVal
Extractor::create( Extractor*& rpcExtractor )
{
  rpcExtractor = new Extractor;
  ROT( NULL == rpcExtractor );
  return Err::m_nOK;
}



ErrVal
Extractor::init( ExtractorParameter *pcExtractorParameter )
{
  ROT( NULL == pcExtractorParameter );

  m_pcExtractorParameter  = pcExtractorParameter;
  m_pcExtractorParameter->setResult( -1 );

  ReadBitstreamFile*  pcReadBitstreamFile;
  RNOKS( ReadBitstreamFile::create( pcReadBitstreamFile ) );
  RNOKS( pcReadBitstreamFile->init( m_pcExtractorParameter->getInFile() ) );
  m_pcReadBitstream = (ReadBitstreamIf*)pcReadBitstreamFile;

  if( !m_pcExtractorParameter->getAnalysisOnly() )
  {
    WriteBitstreamToFile*  pcWriteBitstreamFile;
    RNOKS( WriteBitstreamToFile::create( pcWriteBitstreamFile ) );
    RNOKS( pcWriteBitstreamFile->init( m_pcExtractorParameter->getOutFile() ) );
    m_pcWriteBitstream = (WriteBitstreamIf*)pcWriteBitstreamFile;
  }
  else
  {
    m_pcWriteBitstream = NULL;
  }

  RNOK( h264::H264AVCPacketAnalyzer::create( m_pcH264AVCPacketAnalyzer ) );

  // HS: packet trace
  if( m_pcExtractorParameter->getTraceEnabled() )
  {
    m_pcTraceFile = ::fopen( m_pcExtractorParameter->getTraceFile().c_str(), "wt" );
    ROF( m_pcTraceFile );
  }
  else
  {
    m_pcTraceFile = 0;
  }
  if( m_pcExtractorParameter->getExtractTrace() )
  {
    m_pcExtractionTraceFile = ::fopen( m_pcExtractorParameter->getExtractTraceFile().c_str(), "rt" );
    ROF( m_pcExtractionTraceFile );

    RNOK( m_cLargeFile.open( m_pcExtractorParameter->getInFile(), LargeFile::OM_READONLY ) );
  }
  else
  {
    m_pcExtractionTraceFile = 0;
  }


  m_aucStartCodeBuffer[0] = 0;
  m_aucStartCodeBuffer[1] = 0;
  m_aucStartCodeBuffer[2] = 0;
  m_aucStartCodeBuffer[3] = 1;
  m_cBinDataStartCode.reset();
  m_cBinDataStartCode.set( m_aucStartCodeBuffer, 4 );

  return Err::m_nOK;
}



ErrVal
Extractor::destroy()
{
  m_cBinDataStartCode.reset();

  if( NULL != m_pcH264AVCPacketAnalyzer )
  {
    RNOK( m_pcH264AVCPacketAnalyzer->destroy() );
  }

  if( NULL != m_pcReadBitstream )
  {
    RNOK( m_pcReadBitstream->uninit() );
    RNOK( m_pcReadBitstream->destroy() );
  }

  if( NULL != m_pcWriteBitstream )
  {
    RNOK( m_pcWriteBitstream->uninit() );
    RNOK( m_pcWriteBitstream->destroy() );
  }

  // HS: packet trace
  if( m_pcTraceFile )
  {
    ::fclose( m_pcTraceFile );
    m_pcTraceFile = 0;
  }
  if( m_pcExtractionTraceFile )
  {
    ::fclose( m_pcExtractionTraceFile );
    m_pcExtractionTraceFile = 0;
  }
  if( m_cLargeFile.is_open() )
  {
    RNOK( m_cLargeFile.close() );
  }

  delete this;

  return Err::m_nOK;
}



ErrVal
Extractor::xAnalyse()
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

  // HS: packet trace
  Int64                   i64StartPos   = 0;
  Int64                   i64EndPos     = 0;
  Int                     iLastTempLevel= 0;
  m_uiMaxSize                           = 0;

  //========== initialize (scalable SEI message shall be the first packet of the stream) ===========
  {
    RNOK( m_pcH264AVCPacketAnalyzer->init() );
    //--- get first packet ---
    RNOK( m_pcReadBitstream->extractPacket( pcBinData, bEOS ) );
    ROT ( bEOS );
    //--- analyse packet ---
    RNOK( m_pcH264AVCPacketAnalyzer ->process( pcBinData, cPacketDescription, pcScalableSei ) );
    ROF ( pcScalableSei );

    // HS: packet trace
    if( ! cPacketDescription.ApplyToNext )
    {
      i64EndPos     = static_cast<ReadBitstreamFile*>(m_pcReadBitstream)->getFilePos();
      UInt  uiStart = (UInt)( i64StartPos & 0xFFFFFFFF  );
      UInt  uiSize  = (UInt)( i64EndPos   - i64StartPos );
      i64StartPos   = i64EndPos;
      if( m_pcTraceFile )
      {
        ::fprintf( m_pcTraceFile, "Start-Pos.  Length  LId  TId  QId   Packet-Type  Discardable  Truncatable""\n" );
        ::fprintf( m_pcTraceFile, "==========  ======  ===  ===  ===  ============  ===========  ===========""\n" );
        ::fprintf( m_pcTraceFile, "0x%08x"   "%8d"   "%5d""%5d""%5d""  %s"        "         %s""         %s" "\n",
          uiStart,
          uiSize,
          cPacketDescription.Layer,
          cPacketDescription.Level,
          cPacketDescription.FGSLayer,
          "StreamHeader",
          " No", " No" );
      }
      m_uiMaxSize = max( m_uiMaxSize, uiSize );
    }

    //--- initialize stream description ----
    RNOK( m_cScalableStreamDescription.init( (h264::SEI::ScalableSei*)pcScalableSei ) );
    delete pcScalableSei;
    //---- set packet length ----
    while( pcBinData->data()[ pcBinData->size() - 1 ] == 0x00 )
    {
      RNOK( pcBinData->decreaseEndPos( 1 ) ); // remove zero at end
    }
    //---- update stream description -----
    RNOK( m_cScalableStreamDescription.addPacket( 4+pcBinData->size(), 0, 0, 0, false ) );
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

    // HS: packet trace
    if( ! cPacketDescription.ApplyToNext )
    {
      if( iLastTempLevel )
      {
        cPacketDescription.Level  = iLastTempLevel;
        iLastTempLevel            = 0;
      }
      i64EndPos     = static_cast<ReadBitstreamFile*>(m_pcReadBitstream)->getFilePos();
      UInt  uiStart = (UInt)( i64StartPos & 0xFFFFFFFF  );
      UInt  uiSize  = (UInt)( i64EndPos   - i64StartPos );
      i64StartPos   = i64EndPos;
      if( m_pcTraceFile )
      {
        ::fprintf( m_pcTraceFile, "0x%08x"   "%8d"   "%5d""%5d""%5d""  %s"        "         %s""         %s" "\n",
          uiStart,
          uiSize,
          cPacketDescription.Layer,
          cPacketDescription.Level,
          cPacketDescription.FGSLayer,
          cPacketDescription.ParameterSet ? "ParameterSet" : "   SliceData",
          cPacketDescription.ParameterSet || ( cPacketDescription.Level == 0 && cPacketDescription.FGSLayer == 0 )  ? " No" : "Yes",
          cPacketDescription.FGSLayer ? "Yes" : " No" );
      }
      m_uiMaxSize = max( m_uiMaxSize, uiSize );
    }
    else
    {
      iLastTempLevel = cPacketDescription.Level;
    }

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
    RNOK( m_cScalableStreamDescription.addPacket( uiPacketSize, uiLayer, uiLevel, uiFGSLayer, bNewPicture ) );

    UInt eNalUnitType = cPacketDescription.NalUnitType;
    if(  eNalUnitType == NAL_UNIT_CODED_SLICE              ||
         eNalUnitType == NAL_UNIT_CODED_SLICE_IDR          ||
         eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE     ||
         eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE   )
    {
      m_cScalableStreamDescription.m_bSPSRequired[uiLayer][cPacketDescription.SPSid] = true;
      m_cScalableStreamDescription.m_bPPSRequired[uiLayer][cPacketDescription.PPSid] = true;
    }
  }

  RNOK( m_cScalableStreamDescription.analyse() );
  m_cScalableStreamDescription.output( stdout );

  //----- reset input file -----
  RNOKS( static_cast<ReadBitstreamFile*>(m_pcReadBitstream)->uninit() );
  RNOKS( static_cast<ReadBitstreamFile*>(m_pcReadBitstream)->init  ( m_pcExtractorParameter->getInFile() ) );

  return Err::m_nOK;
}






ErrVal
Extractor::xSetParameters()
{
  UInt  uiLayer, uiLevel, uiFGSLayer;

  //=========== clear all ===========
  for( uiLayer = 0; uiLayer <  MAX_LAYERS;  uiLayer++ )
  for( uiLevel = 0; uiLevel <= MAX_DSTAGES; uiLevel++ )
  {
    m_aadTargetSNRLayer[uiLayer][uiLevel] = -1;
  }


#define ERROR(x,t)   {if(x) {::printf("\nERROR:   %s\n",t); assert(0); return Err::m_nERR;} }
#define WARNING(x,t) {if(x) {::printf("\nWARNING: %s\n",t); } }


  const MyList<ExtractorParameter::Point>&          rcExtList   = m_pcExtractorParameter->getExtractionList();
  ROT( rcExtList.size() != 1 );
  MyList<ExtractorParameter::Point>::const_iterator cIter       = rcExtList.begin ();
  MyList<ExtractorParameter::Point>::const_iterator cEnd        = rcExtList.end   ();
  const ExtractorParameter::Point&                  rcExtPoint  = *cIter;
  UInt                                              uiExtLayer  = MSYS_UINT_MAX;
  UInt                                              uiExtLevel  = MSYS_UINT_MAX;
  //----- layer -----
  for( uiLayer = 0; uiLayer <= m_cScalableStreamDescription.getNumberOfLayers(); uiLayer++ )
  {
    if( rcExtPoint.uiWidth  == m_cScalableStreamDescription.getFrameWidth (uiLayer) &&
        rcExtPoint.uiHeight == m_cScalableStreamDescription.getFrameHeight(uiLayer)    )
    {
      uiExtLayer = uiLayer;
      break;
    }
  }
  ERROR( uiExtLayer==MSYS_UINT_MAX, "Spatial resolution of extraction/inclusion point not supported" );
  m_pcExtractorParameter->setLayer(uiExtLayer);
  //--- level ---
  for( uiLevel = 0; uiLevel <= MAX_DSTAGES; uiLevel++ )
  {
    if( rcExtPoint.dFrameRate == (Double)(1<<uiLevel)*m_cScalableStreamDescription.getFrameRateUnit() )
    {
      uiExtLevel = uiLevel;
      break;
    }
  }
  ERROR( uiExtLevel==MSYS_UINT_MAX, "Temporal resolution of extraction/inclusion point not supported" );
  ERROR( uiExtLevel>m_cScalableStreamDescription.getMaxLevel(uiExtLayer), "Spatio-temporal resolution of extraction/inclusion point not supported" );
  //--- target number of bytes -----
  Double  dTargetNumExtBytes  = rcExtPoint.dBitRate / 8.0 * 1000.0 / ((Double)(1<<uiExtLevel)*m_cScalableStreamDescription.getFrameRateUnit() ) * (Double)m_cScalableStreamDescription.getNumPictures(uiExtLayer,uiExtLevel);


  //===== get and set required base layer packets ======
  Double  dRemainingBytes     = dTargetNumExtBytes;
  for( uiLayer = 0; uiLayer <= uiExtLayer; uiLayer++ )
  for( uiLevel = 0; uiLevel <= uiExtLevel; uiLevel++ )
  {
     Int64 i64NALUBytes                    = m_cScalableStreamDescription.getNALUBytes( uiLayer, uiLevel, 0 );
     dRemainingBytes                      -= (Double)i64NALUBytes;
     m_aadTargetSNRLayer[uiLayer][uiLevel] = 0;
  }
  if( dRemainingBytes < 0.0 )
  {
    WARNING( true, "Bit-rate overflow for extraction/inclusion point" );
    return Err::m_nOK;
  }


  //===== set maximum possible bytes for included layers ======
  for( uiLayer = 0; uiLayer <  uiExtLayer; uiLayer++ )
  {
    for( uiLevel = 0; uiLevel <= uiExtLevel; uiLevel++ )
    {
      for( uiFGSLayer = 1; uiFGSLayer < MAX_QUALITY_LEVELS; uiFGSLayer++ )
      {
        Int64 i64NALUBytes = m_cScalableStreamDescription.getNALUBytes( uiLayer, uiLevel, uiFGSLayer );
        if( (Double)i64NALUBytes <= dRemainingBytes )
        {
          dRemainingBytes                      -= (Double)i64NALUBytes;
          m_aadTargetSNRLayer[uiLayer][uiLevel] = (Double)uiFGSLayer;
        }
        else
        {
          //====== set fractional FGS layer and exit =====
          Double  dFGSLayer = dRemainingBytes / (Double)i64NALUBytes;
          m_aadTargetSNRLayer[uiLayer][uiLevel] += dFGSLayer;
          return Err::m_nOK;
        }
      }
    }
  }


  //===== set FGS layer for current layer =====
  for( uiFGSLayer = 1; uiFGSLayer < MAX_QUALITY_LEVELS; uiFGSLayer++ )
  {
    Int64 i64FGSLayerBytes = 0;
    for( uiLevel = 0; uiLevel <= uiExtLevel; uiLevel++ )
    {
      i64FGSLayerBytes += m_cScalableStreamDescription.getNALUBytes( uiExtLayer, uiLevel, uiFGSLayer );
    }
    if( (Double)i64FGSLayerBytes <= dRemainingBytes )
    {
      dRemainingBytes -= (Double)i64FGSLayerBytes;
      for( uiLevel = 0; uiLevel <= uiExtLevel; uiLevel++ )
      {
        m_aadTargetSNRLayer[uiExtLayer][uiLevel] = (Double)uiFGSLayer;
      }
    }
    else
    {
      Double dFGSLayer = dRemainingBytes / (Double)i64FGSLayerBytes;
      for( uiLevel = 0; uiLevel <= uiExtLevel; uiLevel++ )
      {
        m_aadTargetSNRLayer[uiExtLayer][uiLevel] += dFGSLayer;
      }
      return Err::m_nOK;
    }
  }
  WARNING( dRemainingBytes>0.0, "Bit-rate underflow for extraction/inclusion point" );


#undef ERROR
#undef WARNING

  return Err::m_nOK;
}







ErrVal
Extractor::go()
{
  RNOK ( xAnalyse() );
  ROTRS( m_pcExtractorParameter->getAnalysisOnly(), Err::m_nOK );

  if( m_pcExtractionTraceFile ) // HS: packet trace
  {
    RNOK( xExtractTrace() ); // HS: packet trace
  }
  else if( m_pcExtractorParameter->getExtractionList().empty() )
  {
    RNOK( xExtractLayerLevel() );
  }
  else
  {
    RNOK( xSetParameters() );
    RNOK( xExtractPoints() );
  }

  return Err::m_nOK;
}





ErrVal
Extractor::xExtractPoints()
{
  UInt  uiNumInput    = 0;
  UInt  uiNumKept     = 0;
  UInt  uiNumCropped  = 0;
  Bool  bKeep         = false;
  Bool  bApplyToNext  = false;
  Bool  bEOS          = false;
  Bool  bCrop         = false;

  UInt  uiLayer       = 0;
  UInt  uiLevel       = 0;
  UInt  uiFGSLayer    = 0;
  UInt  uiPacketSize  = 0;
  UInt  uiShrinkSize  = 0;

  RNOK( m_pcH264AVCPacketAnalyzer->init() );

  while( ! bEOS )
  {
    //=========== get packet ===========
    BinData*  pcBinData;
    RNOK( m_pcReadBitstream->extractPacket( pcBinData, bEOS ) );
    if( bEOS )
    {
      continue;
    }


    //========== get packet description ==========
    h264::SEI::SEIMessage*  pcScalableSEIMessage = 0;
    h264::PacketDescription cPacketDescription;
    RNOK( m_pcH264AVCPacketAnalyzer->process( pcBinData, cPacketDescription, pcScalableSEIMessage ) );
    delete pcScalableSEIMessage;

    //============ get packet size ===========
    while( pcBinData->data()[ pcBinData->size() - 1 ] == 0x00 )
    {
      RNOK( pcBinData->decreaseEndPos( 1 ) ); // remove zero at end
    }
    uiPacketSize  = 4 + pcBinData->size();
    uiShrinkSize  = 0;

    //============ set parameters ===========
    if( ! bApplyToNext  )
    {
      uiLayer    = cPacketDescription.Layer;
      uiLevel    = cPacketDescription.Level;
      uiFGSLayer = cPacketDescription.FGSLayer;
    }
    bApplyToNext = cPacketDescription.ApplyToNext;

    //============ check packet ===========
    Double  dSNRLayerDiff = m_aadTargetSNRLayer[uiLayer][uiLevel] - (Double)uiFGSLayer;
    Double  dUpRound      = ceil  ( dSNRLayerDiff );
    Double  dDownRound    = floor ( dSNRLayerDiff );
    bKeep                 =           ( dUpRound   >= 0.0 );
    bCrop                 = bKeep &&  ( dDownRound <  0.0 );
    if( bCrop && uiFGSLayer == 0 )
    {
      bKeep = bCrop = false;
    }
    if( bCrop )
    {
      Double  dWeight     = -dSNRLayerDiff;
      uiShrinkSize        = (UInt)ceil( (Double)uiPacketSize * dWeight );
      if( uiPacketSize - uiShrinkSize > 25 ) // 25 bytes should be enough for the slice headers
      {
        RNOK( pcBinData->decreaseEndPos( uiShrinkSize ) );
        pcBinData->data()[pcBinData->size()-1]  |= 0x01; // trailing one
      }
      else
      {
        bKeep = bCrop = false;
      }
    }

    UInt eNalUnitType = cPacketDescription.NalUnitType;
    Bool bRequired = false;
    if(  eNalUnitType == NAL_UNIT_SPS )
    {
      for( UInt layer = 0; layer <= m_pcExtractorParameter->getLayer(); layer ++ )
      {
        if( m_cScalableStreamDescription.m_bSPSRequired[layer][cPacketDescription.SPSid] )
        {
          bRequired = true;
          break;
        }
      }
      bKeep = bRequired;
    }
    else if( eNalUnitType == NAL_UNIT_PPS )
    {
      for( UInt layer = 0; layer <= m_pcExtractorParameter->getLayer(); layer ++ )
      {
        if( m_cScalableStreamDescription.m_bPPSRequired[layer][cPacketDescription.PPSid] )
        {
          bRequired = true;
          break;
        }
      }
      bKeep = bRequired;
    }

    uiNumInput++;
    if( bKeep ) uiNumKept   ++;
    if( bCrop ) uiNumCropped++;


    //============ write and release packet ============
    if( bKeep )
    {
      RNOK( m_pcWriteBitstream->writePacket( &m_cBinDataStartCode ) );
      RNOK( m_pcWriteBitstream->writePacket( pcBinData ) );
    }
    RNOK( m_pcReadBitstream->releasePacket( pcBinData ) );
  }

  RNOK( m_pcH264AVCPacketAnalyzer->uninit() );


  printf("\n\nNumber of input packets:  %d\nNumber of output packets: %d (cropped: %d)\n\n", uiNumInput, uiNumKept, uiNumCropped );

  return Err::m_nOK;
}



ErrVal
Extractor::xExtractLayerLevel()
{
  UInt    uiMaxLayer    = m_pcExtractorParameter->getLayer();
  UInt    uiMaxLevel    = m_pcExtractorParameter->getLevel();
  Double  dMaxFGSLayer  = m_pcExtractorParameter->getFGSLayer();
  UInt    uiNumInput    = 0;
  UInt    uiNumKept     = 0;
  UInt    uiNumCropped  = 0;
  Bool    bKeep         = false;
  Bool    bApplyToNext  = false;
  Bool    bEOS          = false;
  Bool    bCrop         = false;
  UInt    uiLayer       = 0;
  UInt    uiLevel       = 0;
  UInt    uiFGSLayer    = 0;
  UInt    uiPacketSize  = 0;
  UInt    uiShrinkSize  = 0;

  RNOK( m_pcH264AVCPacketAnalyzer->init() );

  while( ! bEOS )
  {
    //=========== get packet ===========
    BinData*  pcBinData;
    RNOK( m_pcReadBitstream->extractPacket( pcBinData, bEOS ) );
    if( bEOS )
    {
      continue;
    }


    //========== get packet description ==========
    h264::SEI::SEIMessage*  pcScalableSEIMessage = 0;
    h264::PacketDescription cPacketDescription;
    RNOK( m_pcH264AVCPacketAnalyzer->process( pcBinData, cPacketDescription, pcScalableSEIMessage ) );
    delete pcScalableSEIMessage;

    //============ get packet size ===========
    while( pcBinData->data()[ pcBinData->size() - 1 ] == 0x00 )
    {
      RNOK( pcBinData->decreaseEndPos( 1 ) ); // remove zero at end
    }
    uiPacketSize  = 4 + pcBinData->size();
    uiShrinkSize  = 0;

    //============ set parameters ===========
    if( ! bApplyToNext  )
    {
      uiLayer    = cPacketDescription.Layer;
      uiLevel    = cPacketDescription.Level;
      uiFGSLayer = cPacketDescription.FGSLayer;
    }
    bApplyToNext = cPacketDescription.ApplyToNext;

    //============ check packet ===========
    bKeep = ( uiLayer <= uiMaxLayer && uiLevel <= uiMaxLevel );
    bCrop = false;
    if( bKeep )
    {
      Double  dSNRLayerDiff = dMaxFGSLayer - (Double)uiFGSLayer;
      Double  dUpRound      = ceil  ( dSNRLayerDiff );
      Double  dDownRound    = floor ( dSNRLayerDiff );
      bKeep                 =           ( dUpRound    >= 0.0 );
      bCrop                 = bKeep &&  ( dDownRound  <  0.0 );
      if( bCrop )
      {
        Double  dWeight     = -dSNRLayerDiff;
        uiShrinkSize        = (UInt)ceil( (Double)uiPacketSize * dWeight );
        if( uiPacketSize - uiShrinkSize > 25 ) // 25 bytes should be enough for the slice headers
        {
          RNOK( pcBinData->decreaseEndPos( uiShrinkSize ) );
          pcBinData->data()[pcBinData->size()-1]  |= 0x01; // trailing one
        }
        else
        {
          bKeep = bCrop = false;
        }
      }
    }
    UInt eNalUnitType = cPacketDescription.NalUnitType;
    Bool bRequired = false;
    if(  eNalUnitType == NAL_UNIT_SPS )
    {
      for( UInt layer = 0; layer <= uiMaxLayer; layer ++ )
      {
        if( m_cScalableStreamDescription.m_bSPSRequired[layer][cPacketDescription.SPSid] )
        {
          bRequired = true;
          break;
        }
      }
      bKeep = bRequired;
    }
    else if( eNalUnitType == NAL_UNIT_PPS )
    {
      for( UInt layer = 0; layer <= uiMaxLayer; layer ++ )
      {
        if( m_cScalableStreamDescription.m_bPPSRequired[layer][cPacketDescription.PPSid] )
        {
          bRequired = true;
          break;
        }
      }
      bKeep = bRequired;
    }
    uiNumInput++;
    if( bKeep )   uiNumKept++;
    if( bCrop )   uiNumCropped++;


    //============ write and release packet ============
    if( bKeep )
    {
      RNOK( m_pcWriteBitstream->writePacket( &m_cBinDataStartCode ) );
      RNOK( m_pcWriteBitstream->writePacket( pcBinData ) );
    }
    RNOK( m_pcReadBitstream->releasePacket( pcBinData ) );
  }

  RNOK( m_pcH264AVCPacketAnalyzer->uninit() );


  printf("\n\nNumber of input packets:  %d\nNumber of output packets: %d (cropped: %d)\n\n", uiNumInput, uiNumKept, uiNumCropped );

  return Err::m_nOK;
}


// HS: packet trace
ErrVal
Extractor::xReadLineExtractTrace( Char* pcFormatString,
                                  UInt* puiStart,
                                  UInt* puiLength )
{
  if( NULL != puiStart && NULL != puiLength )
  {
    //--- don't ask me why ----
    ROTR( 0 == fscanf( m_pcExtractionTraceFile, pcFormatString, puiStart, puiLength ), Err::m_nInvalidParameter );
  }

  for( Int n = 0; n < 0x400; n++ )
  {
    ROTRS( '\n' == fgetc( m_pcExtractionTraceFile ), Err::m_nOK );
  }

  return Err::m_nERR;
}


// HS: packet trace
ErrVal
Extractor::xExtractTrace()
{
  Bool    bEOS            = false;
  Int64   i64StartPos     = 0;
  Int64   i64EndPos       = 0;
  Int     iLastTempLevel  = 0;
  UInt    uiNextStart     = 0;
  UInt    uiNextLength    = 0;

  UInt    uiNumDiscarded  = 0;
  UInt    uiNumTruncated  = 0;
  UInt    uiNumKept       = 0;

  UChar*  pucPacketBuffer = new UChar[ m_uiMaxSize + 1 ];
  ROF( pucPacketBuffer );

  RNOK( m_pcH264AVCPacketAnalyzer->init() );

  RNOK( xReadLineExtractTrace( "",      NULL,          NULL ) );  // skip first line
  RNOK( xReadLineExtractTrace( "",      NULL,          NULL ) );  // skip second line
  RNOK( xReadLineExtractTrace( "%x %d", &uiNextStart,  &uiNextLength ) );

  while( ! bEOS )
  {
    //=========== get packet ===========
    BinData*  pcBinData;
    RNOK( m_pcReadBitstream->extractPacket( pcBinData, bEOS ) );
    if( bEOS )
    {
      continue;
    }

    //========== get packet description ==========
    h264::SEI::SEIMessage*  pcScalableSEIMessage = 0;
    h264::PacketDescription cPacketDescription;
    RNOK( m_pcH264AVCPacketAnalyzer->process( pcBinData, cPacketDescription, pcScalableSEIMessage ) );
    delete pcScalableSEIMessage;

    if( ! cPacketDescription.ApplyToNext )
    {
      i64EndPos       = static_cast<ReadBitstreamFile*>( m_pcReadBitstream )->getFilePos();
      UInt  uiStart   = (UInt)( i64StartPos & 0xFFFFFFFF  );
      UInt  uiSize    = (UInt)( i64EndPos   - i64StartPos );
      i64StartPos     = i64EndPos;

      printf("PACKET 0x%08x (%6d)     ", uiStart, uiSize );

      //////////////////////////////////////////////////////////////////////////
      if( uiStart == uiNextStart )
      {
        //===== read packet =====
        ROT ( uiSize > m_uiMaxSize );
        RNOK( m_cLargeFile.seek( uiStart, SEEK_SET ) );
        UInt  uiReadSize = 0;
        RNOK( m_cLargeFile.read( pucPacketBuffer, uiSize, uiReadSize ) );
        ROF ( uiSize == uiReadSize );

        //===== modify last bytes if necessary ====
        if( uiSize < uiNextLength )
        {
          fprintf( stderr, "\nERROR: The packet at start pos. 0x%08x is shorter than %d bytes\n", uiNextStart, uiNextLength );
          ROT( true );
        }
        else if( uiSize > uiNextLength )
        {
          if( cPacketDescription.FGSLayer == 0 )
          {
            fprintf( stderr, "\nERROR: The packet at start pos. 0x%08x is not truncatable\n", uiNextStart, uiNextLength );
            ROT( true );
          }

          //===== truncate packet =====
          if( pcBinData->size() - uiSize + uiNextLength < 25 )
          {
            uiNextLength = 25 + uiSize - pcBinData->size();

            fprintf( stderr, "\nWARNING: The size of the packet at start pos. 0x%08x was increased to %d bytes\n", uiNextStart, uiNextLength );
          }

          pucPacketBuffer[uiNextLength-1] |= 0x01; // trailing one

          printf("truncated to %d bytes\n", uiNextLength );
          uiNumTruncated++;
        }
        else
        {
          printf("kept\n");
          uiNumKept++;
        }

        //===== write packet =====
        static_cast<WriteBitstreamToFile*>( m_pcWriteBitstream )->writePacket( pucPacketBuffer, uiNextLength );

        //===== get next traget packet ====
        if( xReadLineExtractTrace( "%x %d", &uiNextStart,  &uiNextLength ) != Err::m_nOK )
        {
          uiNextStart   = 0xFFFFFFFF;
          uiNextLength  = 1;
        }
      }
      else if( uiStart > uiNextStart )
      {
        fprintf( stderr, "\nERROR: It exists no packet with start pos. 0x%08x\n", uiNextStart );
        ROT( true );
      }
      else
      {
        printf("discarded\n");
        uiNumDiscarded++;
      }
    }
    else
    {
      iLastTempLevel  = cPacketDescription.Level;
    }

    RNOK( m_pcReadBitstream->releasePacket( pcBinData ) );
  }

  RNOK( m_pcH264AVCPacketAnalyzer->uninit() );

  delete pucPacketBuffer;

  printf("\n\n\n");
  printf("%d packets kept (%d truncated)\n", uiNumKept+uiNumTruncated, uiNumTruncated );
  printf("%d packets discarded\n", uiNumDiscarded );
  printf("\n");

  return Err::m_nOK;
}







ScalableStreamDescription::ScalableStreamDescription()
: m_bInit     ( false )
, m_bAnalyzed ( false )
{
}

ScalableStreamDescription::~ScalableStreamDescription()
{
}

ErrVal
ScalableStreamDescription::init( h264::SEI::ScalableSei* pcScalableSei )
{
  ROT( m_bInit );

  ::memset( m_aaaui64NumNALUBytes, 0x00, MAX_LAYERS*(MAX_DSTAGES+1)*MAX_QUALITY_LEVELS*sizeof(UInt64) );
  ::memset( m_aauiNumPictures,     0x00, MAX_LAYERS*(MAX_DSTAGES+1)                   *sizeof(UInt)   );

  m_uiNumLayers           = pcScalableSei->getNumLayers           ();
  m_bAVCBaseLayer         = pcScalableSei->getBaseLayerIsAVC      ();
  m_uiAVCTempResStages    = pcScalableSei->getAVCTempResStages    ();
  m_uiFrameRateUnitDenom  = pcScalableSei->getFrameRateUnitDenom  ();
  m_uiFrameRateUnitNom    = pcScalableSei->getFrameRateUnitNom    ();
  m_uiMaxDecStages        = pcScalableSei->getMaxDecStages        ();
  UInt  m_uiMaxWidth      = pcScalableSei->getMaxHorFrameDimInMB  () << 4;
  UInt  m_uiMaxHeight     = pcScalableSei->getMaxVerFrameDimInMB  () << 4;

  for( UInt uiLayer = 0; uiLayer < m_uiNumLayers; uiLayer++ )
  {
    m_auiFrameWidth [uiLayer] = m_uiMaxWidth    >> pcScalableSei->getSpatialResolutionFactor  ( uiLayer );
    m_auiFrameHeight[uiLayer] = m_uiMaxHeight   >> pcScalableSei->getSpatialResolutionFactor  ( uiLayer );
    m_auiDecStages  [uiLayer] = m_uiMaxDecStages - pcScalableSei->getTemporalResolutionFactor ( uiLayer );
    UInt uiNum;
    for( uiNum = 0; uiNum < 32; uiNum ++ )
      m_bSPSRequired[uiLayer][uiNum] = false;
    for( uiNum = 0; uiNum < 256; uiNum ++ )
      m_bPPSRequired[uiLayer][uiNum] = false;
  }

  m_bInit     = true;
  m_bAnalyzed = false;

  return Err::m_nOK;
}

ErrVal
ScalableStreamDescription::uninit()
{
  m_bInit     = false;
  m_bAnalyzed = false;

  return Err::m_nOK;
}

ErrVal
ScalableStreamDescription::addPacket( UInt  uiNumBytes,
                                      UInt  uiLayer,
                                      UInt  uiLevel,
                                      UInt  uiFGSLayer,
                                      Bool  bNewPicture )
{
  ROF( m_bInit      );
  ROT( m_bAnalyzed  );
  ROF( uiLayer    <  MAX_LAYERS         );
  ROF( uiLevel    <= MAX_DSTAGES        );
  ROF( uiFGSLayer <  MAX_QUALITY_LEVELS );

  m_aaaui64NumNALUBytes[uiLayer][uiLevel][uiFGSLayer] += uiNumBytes;

  if( bNewPicture && uiFGSLayer == 0 )
  {
    m_aauiNumPictures[uiLayer][uiLevel]++;
  }

  return Err::m_nOK;
}


ErrVal
ScalableStreamDescription::analyse()
{
  ROF( m_bInit );

  UInt  uiLayer, uiLevel, uiFGSLayer;

  ::memset( m_aaui64BaseLayerBytes, 0x00, MAX_LAYERS*(MAX_DSTAGES+1)*sizeof(UInt64) );
  ::memset( m_aaui64FGSLayerBytes,  0x00, MAX_LAYERS*(MAX_DSTAGES+1)*sizeof(UInt64) );

  UInt auiMaxUsedDecStages[MAX_LAYERS];
  ::memset( auiMaxUsedDecStages, 0x00, MAX_LAYERS*sizeof(UInt) );
  for( uiLayer = 0; uiLayer < m_uiNumLayers; uiLayer++ )
  {
    for( uiLevel = MAX_DSTAGES; uiLevel > 0; uiLevel-- )
    {
      if( m_aauiNumPictures[uiLayer][uiLevel] )
      {
        auiMaxUsedDecStages[uiLayer] = uiLevel;
        break;
      }
    }
  }
  if( m_bAVCBaseLayer )
  {
    auiMaxUsedDecStages[0] = max( auiMaxUsedDecStages[0], m_auiDecStages[0]-m_uiAVCTempResStages );
  }


  //===== adding NAL unit packet bytes =====
  for( uiLayer = 0; uiLayer < m_uiNumLayers; uiLayer++ )
  {
    for( uiLevel = 0; uiLevel <= MAX_DSTAGES; uiLevel++ )
    {
      m_aaui64BaseLayerBytes[uiLayer][uiLevel] = m_aaaui64NumNALUBytes[uiLayer][uiLevel][0];
      m_aaui64FGSLayerBytes [uiLayer][uiLevel] = 0;

      for( uiFGSLayer = 1; uiFGSLayer < MAX_QUALITY_LEVELS; uiFGSLayer++ )
      {
        m_aaui64FGSLayerBytes   [uiLayer][uiLevel] += m_aaaui64NumNALUBytes[uiLayer][uiLevel][uiFGSLayer];
      }

      if( uiLevel )
      {
        m_aaui64BaseLayerBytes  [uiLayer][uiLevel] += m_aaui64BaseLayerBytes[uiLayer][uiLevel-1];
        m_aaui64FGSLayerBytes   [uiLayer][uiLevel] += m_aaui64FGSLayerBytes [uiLayer][uiLevel-1];
      }
      if( uiLayer )
      {
        m_aaui64BaseLayerBytes  [uiLayer][uiLevel] += m_aaui64BaseLayerBytes[uiLayer-1][uiLevel];
        m_aaui64FGSLayerBytes   [uiLayer][uiLevel] += m_aaui64FGSLayerBytes [uiLayer-1][uiLevel];

        if( uiLevel )
        {
          m_aaui64BaseLayerBytes[uiLayer][uiLevel] -= m_aaui64BaseLayerBytes[uiLayer-1][uiLevel-1];
          m_aaui64FGSLayerBytes [uiLayer][uiLevel] -= m_aaui64FGSLayerBytes [uiLayer-1][uiLevel-1];
        }
      }
      if( uiLevel && uiLevel <= auiMaxUsedDecStages[uiLayer] )
      {
        m_aauiNumPictures[uiLayer][uiLevel] += m_aauiNumPictures[uiLayer][uiLevel-1];
      }
    }
  }

  m_bAnalyzed = true;
  return Err::m_nOK;
}



Void
ScalableStreamDescription::output( FILE* pFile )
{
  printf("\nsupported bit-rates:");
  printf("\n====================\n\n");

  for( UInt uiLayer = 0; uiLayer < m_uiNumLayers; uiLayer++ )
  {
    Char  acFormat[20];
    UInt  uiFrameWidth  = m_auiFrameWidth   [uiLayer];
    UInt  uiFrameHeight = m_auiFrameHeight  [uiLayer];
    sprintf( acFormat, "L%d %dx%d", uiLayer, uiFrameWidth, uiFrameHeight );

    for( UInt uiLevel = 0; uiLevel <= m_auiDecStages[uiLayer]; uiLevel++ )
    {
      if( m_aauiNumPictures[uiLayer][uiLevel] && ( uiLayer || uiLevel >= m_auiDecStages[0]-m_uiAVCTempResStages || !m_bAVCBaseLayer ) )
      {
        Double  dFrameRate  = (Double)( ( 1 << uiLevel ) * m_uiFrameRateUnitNom ) / (Double)m_uiFrameRateUnitDenom;
        Double  dMinBitRate = (Double)(Int64) m_aaui64BaseLayerBytes[uiLayer][uiLevel];
        Double  dMaxBitRate = (Double)(Int64)(m_aaui64BaseLayerBytes[uiLayer][uiLevel]+m_aaui64FGSLayerBytes [uiLayer][uiLevel]);
        Double  dRateFactor = 8.0 / 1000.0 * dFrameRate / (Double)m_aauiNumPictures[uiLayer][uiLevel];

        printf("%12s @ %7.4lf Hz   ->  %8.2lf kbit/s  - %8.2lf kbit/s\n",
          acFormat, dFrameRate,
          dRateFactor*dMinBitRate,
          dRateFactor*dMaxBitRate );
      }
    }
    printf("\n");
  }

  printf("\n\n");
}



