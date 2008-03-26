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


#include "H264AVCDecoderLib.h"
#include "H264AVCDecoder.h"

#include "H264AVCCommonLib/LoopFilter.h"
#include "H264AVCCommonLib/TraceFile.h"

#include "SliceReader.h"
#include "SliceDecoder.h"

#include "CreaterH264AVCDecoder.h"
#include "ControlMngH264AVCDecoder.h"

#include "GOPDecoder.h"

#include "H264AVCCommonLib/CFMO.h"


H264AVC_NAMESPACE_BEGIN





H264AVCDecoder::H264AVCDecoder()
: m_bInitDone               ( false )
, m_pcNalUnitParser         ( 0 )
, m_pcHeaderSymbolReadIf    ( 0 )
, m_pcParameterSetMngAUInit ( 0 )
, m_pcParameterSetMngDecode ( 0 )
{
  ::memset( m_apcLayerDecoder, 0x00, MAX_LAYERS * sizeof( Void* ) );
}

H264AVCDecoder::~H264AVCDecoder()
{
}

ErrVal
H264AVCDecoder::create( H264AVCDecoder*& rpcH264AVCDecoder )
{
  rpcH264AVCDecoder = new H264AVCDecoder;
  ROT( NULL == rpcH264AVCDecoder );
  return Err::m_nOK;
}


ErrVal
H264AVCDecoder::destroy()
{
  delete this;
  return Err::m_nOK;
}

ErrVal
H264AVCDecoder::init( NalUnitParser*      pcNalUnitParser,
                      HeaderSymbolReadIf* pcHeaderSymbolReadIf,
                      ParameterSetMng*    pcParameterSetMngAUInit,
                      ParameterSetMng*    pcParameterSetMngDecode,
                      LayerDecoder*       apcLayerDecoder[MAX_LAYERS] )
{

  ROF( pcNalUnitParser );
  ROF( pcHeaderSymbolReadIf );
  ROF( pcParameterSetMngAUInit );
  ROF( pcParameterSetMngDecode );
  ROF( apcLayerDecoder );

  for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
  {
    ROF( apcLayerDecoder[uiLayer] );
    m_apcLayerDecoder[uiLayer] = apcLayerDecoder[uiLayer];
  }

  m_bInitDone               = true;
  m_pcNalUnitParser         = pcNalUnitParser;
  m_pcHeaderSymbolReadIf    = pcHeaderSymbolReadIf;
  m_pcParameterSetMngAUInit = pcParameterSetMngAUInit;
  m_pcParameterSetMngDecode = pcParameterSetMngDecode;

  return Err::m_nOK;
}

ErrVal
H264AVCDecoder::uninit()
{
  m_bInitDone               = false;
  m_pcNalUnitParser         = 0;
  m_pcHeaderSymbolReadIf    = 0;
  m_pcParameterSetMngAUInit = 0;
  m_pcParameterSetMngDecode = 0;

  for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
  {
    m_apcLayerDecoder[uiLayer] = 0;
  }

  return Err::m_nOK;
}

ErrVal
H264AVCDecoder::initNALUnit( BinData*&    rpcBinData,
                             AccessUnit&  rcAccessUnit )
{
  ROT( rcAccessUnit.isComplete    () );
  ROT( rcAccessUnit.isEndOfStream () );

  //===== check for empty NAL unit packet =====
  if( ! rpcBinData || ! rpcBinData->data() || ! rpcBinData->size() )
  {
    RNOK( rcAccessUnit.update() );
    return Err::m_nOK;
  }

  //===== create copy of bin data =====
  UChar*    pucBuffer     = new UChar [ rpcBinData->size() ];
  BinData*  pcBinDataCopy = new BinData;
  ROF( pucBuffer );
  ROF( pcBinDataCopy );
  ::memcpy( pucBuffer, rpcBinData->data(), rpcBinData->size() );
  pcBinDataCopy->set( pucBuffer, rpcBinData->size() );

  //===== parse required part of the NAL unit =====
  PrefixHeader* pcPrefixHeader  = 0;
  SliceHeader*  pcSliceHeader   = 0;
  {
    DTRACE_OFF;
    Bool            bCompletelyParsed = false;
    BinDataAccessor cBinDataAccessor;
    rpcBinData                ->setMemAccessor( cBinDataAccessor );
    RNOK  ( m_pcNalUnitParser ->initNalUnit   ( cBinDataAccessor ) );
    switch( m_pcNalUnitParser ->getNalUnitType() )
    {
    case NAL_UNIT_SPS:
    case NAL_UNIT_SUBSET_SPS:
      {
        //===== required for parsing of slice headers =====
        SequenceParameterSet* pcSPS = NULL;
        RNOK( SequenceParameterSet::create( pcSPS ) );
        RNOK( pcSPS->read( m_pcHeaderSymbolReadIf, m_pcNalUnitParser->getNalUnitType() ) );
        RNOK( m_pcParameterSetMngAUInit->store( pcSPS ) );
        bCompletelyParsed = true;
        break;
      }
    case NAL_UNIT_PPS:
      {
        //===== required for parsing of slice headers =====
        PictureParameterSet* pcPPS = NULL;
        RNOK( PictureParameterSet::create( pcPPS ) );
        RNOK( pcPPS->read( m_pcHeaderSymbolReadIf, m_pcNalUnitParser->getNalUnitType() ) );
        RNOK( m_pcParameterSetMngAUInit->store ( pcPPS ) );
        bCompletelyParsed = true;
        break;
      }
    case NAL_UNIT_PREFIX:
      {
        pcPrefixHeader = new PrefixHeader( *m_pcNalUnitParser );
        ROF ( pcPrefixHeader );
        RNOK( pcPrefixHeader->read( *m_pcHeaderSymbolReadIf ) );
        bCompletelyParsed = true;
        break;
      }
    case NAL_UNIT_CODED_SLICE:
    case NAL_UNIT_CODED_SLICE_IDR:
      {
        if( rcAccessUnit.getLastPrefixHeader() )
        {
          pcSliceHeader = new SliceHeader( *rcAccessUnit.getLastPrefixHeader() );
          ROF( pcSliceHeader );
          pcSliceHeader->NalUnitHeader::copy( *m_pcNalUnitParser, false );
        }
        else
        {
          pcSliceHeader = new SliceHeader( *m_pcNalUnitParser );
          ROF( pcSliceHeader );
        }
        RNOK ( pcSliceHeader->read( *m_pcParameterSetMngAUInit, *m_pcHeaderSymbolReadIf ) );
        break;
      }
    case NAL_UNIT_CODED_SLICE_SCALABLE:
      {
        pcSliceHeader = new SliceHeader( *m_pcNalUnitParser );
        ROF ( pcSliceHeader );
        RNOK( pcSliceHeader->read( *m_pcParameterSetMngAUInit, *m_pcHeaderSymbolReadIf ) );
        break;
      }
    default:
      {
        // no parsing required
        break;
      }
    }
    RNOK( m_pcNalUnitParser->closeNalUnit( bCompletelyParsed ) );
    DTRACE_ON;
  }

  //===== update access unit list =====
  switch( m_pcNalUnitParser->getNalUnitType() )
  {
  case NAL_UNIT_PREFIX:
    {
      RNOK( rcAccessUnit.update( pcBinDataCopy, *pcPrefixHeader ) );
      break;
    }
  case NAL_UNIT_CODED_SLICE:
  case NAL_UNIT_CODED_SLICE_IDR:
  case NAL_UNIT_CODED_SLICE_SCALABLE:
    {
      RNOK( rcAccessUnit.update( pcBinDataCopy, *pcSliceHeader ) );
      break;
    }
  case NAL_UNIT_CODED_SLICE_DATAPART_A:
  case NAL_UNIT_CODED_SLICE_DATAPART_B:
  case NAL_UNIT_CODED_SLICE_DATAPART_C:
    {
      RERR(); // not supported
      break;
    }
  default:
    {
      RNOK( rcAccessUnit.update( pcBinDataCopy ) );
      break;
    }
  }
  return Err::m_nOK;
}


ErrVal
H264AVCDecoder::processNALUnit( PicBuffer*        pcPicBuffer,
                                PicBufferList&    rcPicBufferOutputList,
                                PicBufferList&    rcPicBufferUnusedList,
                                BinDataList&      rcBinDataList,
                                NALUnit&          rcNALUnit )
{
  if( ! rcNALUnit.isVCLNALUnit() )
  {
    NonVCLNALUnit& rcNonVCLNALUnit = *(NonVCLNALUnit*)rcNALUnit.getInstance();
    RNOK( xProcessNonVCLNALUnit( rcNonVCLNALUnit ) );
    rcPicBufferUnusedList.push_back( pcPicBuffer );
    return Err::m_nOK;
  }

  PicBufferList     cDummyList;
  SliceDataNALUnit& rcSliceDataNALUnit  = *(SliceDataNALUnit*)rcNALUnit.getInstance();
  PicBufferList&    rcOutputList        = ( rcSliceDataNALUnit.isDependencyIdMax() ? rcPicBufferOutputList : cDummyList );
  RNOK  ( m_apcLayerDecoder[ rcSliceDataNALUnit.getDependencyId() ]->processSliceData( pcPicBuffer, rcOutputList, rcPicBufferUnusedList, rcBinDataList, rcSliceDataNALUnit ) );
  ROFRS ( rcSliceDataNALUnit.isLastAccessUnitInStream() && rcSliceDataNALUnit.isLastSliceInAccessUnit(), Err::m_nOK );

  for( UInt uiDependencyId = 0; uiDependencyId < rcSliceDataNALUnit.getDependencyId(); uiDependencyId++ )
  {
    RNOK( m_apcLayerDecoder[ uiDependencyId                       ]->finishProcess( cDummyList,            rcPicBufferUnusedList ) );
  }
  RNOK(   m_apcLayerDecoder[ rcSliceDataNALUnit.getDependencyId() ]->finishProcess( rcPicBufferOutputList, rcPicBufferUnusedList ) );

  return Err::m_nOK;
}


ErrVal
H264AVCDecoder::xProcessNonVCLNALUnit( NonVCLNALUnit& rcNonVCLNALUnit )
{
  //===== parse prefix header when available =====
  PrefixHeader* pcPrefixHeader = 0;
  if( rcNonVCLNALUnit.getBinDataPrefix() )
  {
    BinDataAccessor cBinDataAccessorPrefix;
    rcNonVCLNALUnit.getBinDataPrefix()->setMemAccessor( cBinDataAccessorPrefix );
    RNOK( m_pcNalUnitParser ->initNalUnit   ( cBinDataAccessorPrefix )  );
    ROF ( m_pcNalUnitParser ->getNalUnitType() == NAL_UNIT_PREFIX );
    pcPrefixHeader = new PrefixHeader( *m_pcNalUnitParser );
    ROF ( pcPrefixHeader );
    RNOK( pcPrefixHeader    ->read          ( *m_pcHeaderSymbolReadIf ) );
    RNOK( m_pcNalUnitParser ->closeNalUnit  () );
  }

  //===== parse NAL unit =====
  BinDataAccessor cBinDataAccessor;
  rcNonVCLNALUnit.getBinData()->setMemAccessor( cBinDataAccessor );
  RNOK  ( m_pcNalUnitParser   ->initNalUnit   ( cBinDataAccessor )  );
  switch( m_pcNalUnitParser   ->getNalUnitType() )
  {
  case NAL_UNIT_SPS:
    {
      SequenceParameterSet* pcSPS = NULL;
      RNOK( SequenceParameterSet::create( pcSPS ) );
      RNOK( pcSPS->read( m_pcHeaderSymbolReadIf, m_pcNalUnitParser->getNalUnitType() ) );
      RNOK( m_pcParameterSetMngDecode->store( pcSPS ) );
      printf("  NON-VCL: SEQUENCE PARAMETER SET (ID=%d)\n", pcSPS->getSeqParameterSetId() );
      break;
    }
  case NAL_UNIT_SUBSET_SPS:
    {
      SequenceParameterSet* pcSPS = NULL;
      RNOK( SequenceParameterSet::create( pcSPS ) );
      RNOK( pcSPS->read( m_pcHeaderSymbolReadIf, m_pcNalUnitParser->getNalUnitType() ) );
      RNOK( m_pcParameterSetMngDecode->store( pcSPS ) );
      printf("  NON-VCL: SUBSET SEQUENCE PARAMETER SET (ID=%d)\n", pcSPS->getSeqParameterSetId() );
      break;
    }
  case NAL_UNIT_PPS:
    {
      PictureParameterSet* pcPPS = NULL;
      RNOK( PictureParameterSet::create( pcPPS ) );
      RNOK( pcPPS->read( m_pcHeaderSymbolReadIf, m_pcNalUnitParser->getNalUnitType() ) );
      RNOK( m_pcParameterSetMngDecode->store ( pcPPS ) );
      printf("  NON-VCL: PICTURE PARAMETER SET (ID=%d)\n", pcPPS->getPicParameterSetId() );
      break;
    }
  case NAL_UNIT_SEI: // just read, but ignore
    {
      SEI::MessageList cSEIMessageList;
      RNOK( SEI::read( m_pcHeaderSymbolReadIf, cSEIMessageList, m_pcParameterSetMngDecode ) );
      printf("  NON-VCL: SEI NAL UNIT\n" );
      break;
    }
  case NAL_UNIT_ACCESS_UNIT_DELIMITER: // just read, but ignore
    {
      AUDelimiter cAUDelimiter( *m_pcNalUnitParser );
      RNOK( cAUDelimiter.read ( *m_pcHeaderSymbolReadIf ) );
      printf("  NON-VCL: ACCESS UNIT DELIMITER\n" );
      break;
    }
  case NAL_UNIT_END_OF_SEQUENCE: // just read, but ignore
    {
      EndOfSequence cEndOfSequence( *m_pcNalUnitParser );
      RNOK( cEndOfSequence.read   ( *m_pcHeaderSymbolReadIf ) );
      printf("  NON-VCL: END OF SEQUENCE\n" );
      break;
    }
  case NAL_UNIT_END_OF_STREAM: // just read, but ignore
    {
      EndOfStream cEndOfStream( *m_pcNalUnitParser );
      RNOK( cEndOfStream.read ( *m_pcHeaderSymbolReadIf ) );
      printf("  NON-VCL: END OF STREAM\n" );
      break;
    }
  case NAL_UNIT_FILLER_DATA: // just read, but ignore
    {
      const NalUnitHeader&  rcNalUnitHeader = ( pcPrefixHeader ? *pcPrefixHeader : *m_pcNalUnitParser );
      FillerData            cFillerData( rcNalUnitHeader );
      RNOK( cFillerData.read( *m_pcHeaderSymbolReadIf ) );
      cFillerData.NalUnitHeader::copy( *m_pcNalUnitParser, false );
      printf("  NON-VCL: FILLER DATA (D=%d,Q=%d)\n", cFillerData.getDependencyId(), cFillerData.getQualityId() );
      break;
    }
  default:
    {
      // ignore
      break;
    }
  }
  RNOK( m_pcNalUnitParser->closeNalUnit() );
  return Err::m_nOK;
}

ErrVal
H264AVCDecoder::getBaseLayerData( SliceHeader&      rcELSH,
                                  Frame*&           pcFrame,
                                  Frame*&           pcResidual,
                                  MbDataCtrl*&      pcMbDataCtrl,
                                  ResizeParameters& rcResizeParameters,
                                  UInt              uiBaseLayerId )
{
  RNOK( m_apcLayerDecoder[uiBaseLayerId]->getBaseLayerData( rcELSH, pcFrame, pcResidual, pcMbDataCtrl, rcResizeParameters ) );
  return Err::m_nOK;
}

ErrVal
H264AVCDecoder::getBaseSliceHeader( SliceHeader*& rpcSliceHeader, UInt uiRefLayerDependencyId )
{
  RNOK( m_apcLayerDecoder[ uiRefLayerDependencyId ]->getBaseSliceHeader( rpcSliceHeader ) );
  return Err::m_nOK;
}


H264AVC_NAMESPACE_END

