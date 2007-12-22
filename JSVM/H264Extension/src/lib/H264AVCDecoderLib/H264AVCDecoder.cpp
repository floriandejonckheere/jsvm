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
: m_bInitDone             ( false )
, m_pcNalUnitParser       ( 0 )
, m_pcHeaderSymbolReadIf  ( 0 )
, m_pcParameterSetMng     ( 0 )
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
                      ParameterSetMng*    pcParameterSetMng,
                      LayerDecoder*       apcLayerDecoder[MAX_LAYERS] )
{

  ROF( pcNalUnitParser );
  ROF( pcHeaderSymbolReadIf );
  ROF( pcParameterSetMng );
  ROF( apcLayerDecoder );

  for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
  {
    ROF( apcLayerDecoder[uiLayer] );
    m_apcLayerDecoder[uiLayer] = apcLayerDecoder[uiLayer];
  }

  m_bInitDone             = true;
  m_pcNalUnitParser       = pcNalUnitParser;
  m_pcHeaderSymbolReadIf  = pcHeaderSymbolReadIf;
  m_pcParameterSetMng     = pcParameterSetMng;

  return Err::m_nOK;
}

ErrVal
H264AVCDecoder::uninit()
{
  m_bInitDone             = false;
  m_pcNalUnitParser       = 0;
  m_pcHeaderSymbolReadIf  = 0;
  m_pcParameterSetMng     = 0;

  for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
  {
    m_apcLayerDecoder[uiLayer] = 0;
  }

  return Err::m_nOK;
}

ErrVal
H264AVCDecoder::initNALUnit( BinData*&          rpcBinData,
                             AccessUnitSlices&  rcAccessUnitSlices )
{
  ROT( rcAccessUnitSlices.isComplete    () );
  ROT( rcAccessUnitSlices.isEndOfStream () );

  //===== check for empty NAL unit packet =====
  if( ! rpcBinData || ! rpcBinData->data() || ! rpcBinData->size() )
  {
    RNOK( rcAccessUnitSlices.updateEndOfStream() );
    return Err::m_nOK;
  }

  //===== create copy of bin data when required =====
  BinData*    pcBinDataCopy = 0;
  NalUnitType eNalUnitType  = NalUnitType( rpcBinData->data()[ 0 ] & 0x1F );
  if( eNalUnitType == NAL_UNIT_PREFIX               ||
      eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE ||
      eNalUnitType == NAL_UNIT_CODED_SLICE          ||
      eNalUnitType == NAL_UNIT_CODED_SLICE_IDR        )
  {
    UChar*  pucBuffer = new UChar [ rpcBinData->size() ];
    pcBinDataCopy     = new BinData;
    ROF( pucBuffer );
    ROF( pcBinDataCopy );
    ::memcpy( pucBuffer, rpcBinData->data(), rpcBinData->size() );
    pcBinDataCopy->set( pucBuffer, rpcBinData->size() );
  }

  //===== parse required part of the NAL unit =====
  PrefixHeader* pcPrefixHeader  = 0;
  SliceHeader*  pcSliceHeader   = 0;
  {
    Bool            bCompletelyParsed = false;
    BinDataAccessor cBinDataAccessor;
    rpcBinData                ->setMemAccessor( cBinDataAccessor );
    RNOK  ( m_pcNalUnitParser ->initNalUnit   ( cBinDataAccessor ) );
    switch( m_pcNalUnitParser ->getNalUnitType() )
    {
    case NAL_UNIT_SPS:
    case NAL_UNIT_SUBSET_SPS:
      {
        SequenceParameterSet* pcSPS = NULL;
        RNOK( SequenceParameterSet::create( pcSPS ) );
        RNOK( pcSPS               ->read  ( m_pcHeaderSymbolReadIf, m_pcNalUnitParser->getNalUnitType() ) );
        RNOK( m_pcParameterSetMng ->store ( pcSPS ) );
        bCompletelyParsed = true;
        break;
      }
    case NAL_UNIT_PPS:
      {
        PictureParameterSet* pcPPS = NULL;
        RNOK( PictureParameterSet::create ( pcPPS ) );
        RNOK( pcPPS               ->read  ( m_pcHeaderSymbolReadIf, m_pcNalUnitParser->getNalUnitType() ) );
        RNOK( m_pcParameterSetMng ->store ( pcPPS ) );
        bCompletelyParsed = true;
        break;
      }
    case NAL_UNIT_SEI:
      {
        SEI::MessageList cSEIMessageList;
        RNOK( SEI::read( m_pcHeaderSymbolReadIf, cSEIMessageList, m_pcParameterSetMng ) );
        bCompletelyParsed = true;
        // ignore SEI messages
        break;
      }
    case NAL_UNIT_ACCESS_UNIT_DELIMITER:
      {
        AUDelimiter cAUDelimiter( *m_pcNalUnitParser );
        RNOK( cAUDelimiter.read ( *m_pcHeaderSymbolReadIf ) );
        bCompletelyParsed = true;
        break;
      }
    case NAL_UNIT_END_OF_SEQUENCE:
      {
        EndOfSequence cEndOfSequence( *m_pcNalUnitParser );
        RNOK( cEndOfSequence.read   ( *m_pcHeaderSymbolReadIf ) );
        bCompletelyParsed = true;
        break;
      }
    case NAL_UNIT_END_OF_STREAM:
      {
        EndOfStream cEndOfStream( *m_pcNalUnitParser );
        RNOK( cEndOfStream.read ( *m_pcHeaderSymbolReadIf ) );
        bCompletelyParsed = true;
        break;
      }
    case NAL_UNIT_FILLER_DATA:
      {
        const NalUnitHeader&  rcNalUnitHeader = ( rcAccessUnitSlices.getLastPrefixHeader() ? *rcAccessUnitSlices.getLastPrefixHeader() : *m_pcNalUnitParser );
        FillerData            cFillerData( rcNalUnitHeader );
        RNOK( cFillerData.read( *m_pcHeaderSymbolReadIf ) );
        cFillerData.NalUnitHeader::copy( *m_pcNalUnitParser, false );
        bCompletelyParsed = true;
        break;
      }
    case NAL_UNIT_CODED_SLICE_DATAPART_A:
    case NAL_UNIT_CODED_SLICE_DATAPART_B:
    case NAL_UNIT_CODED_SLICE_DATAPART_C:
      {
        RERR(); // not supported
        break;
      }
    case NAL_UNIT_SPS_EXTENSION:
    case NAL_UNIT_AUX_CODED_SLICE:
      {
        // not supported -> ignore
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
        if( rcAccessUnitSlices.getLastPrefixHeader() )
        {
          pcSliceHeader = new SliceHeader( *rcAccessUnitSlices.getLastPrefixHeader() );
          ROF( pcSliceHeader );
          pcSliceHeader->NalUnitHeader::copy( *m_pcNalUnitParser, false );
        }
        else
        {
          pcSliceHeader = new SliceHeader( *m_pcNalUnitParser );
          ROF( pcSliceHeader );
        }
        RNOK ( pcSliceHeader->read( *m_pcParameterSetMng, *m_pcHeaderSymbolReadIf ) );
        break;
      }
    case NAL_UNIT_CODED_SLICE_SCALABLE:
      {
        pcSliceHeader = new SliceHeader( *m_pcNalUnitParser );
        ROF ( pcSliceHeader );
        RNOK( pcSliceHeader->read( *m_pcParameterSetMng, *m_pcHeaderSymbolReadIf ) );
        break;
      }
    default:
      {
        // ignore
        break;
      }
    }
    RNOK( m_pcNalUnitParser->closeNalUnit( bCompletelyParsed ) );
  }

  //===== update access unit list =====
  switch( m_pcNalUnitParser->getNalUnitType() )
  {
  case NAL_UNIT_PREFIX:
    {
      RNOK( rcAccessUnitSlices.update( pcBinDataCopy, *pcPrefixHeader ) );
      break;
    }
  case NAL_UNIT_CODED_SLICE:
  case NAL_UNIT_CODED_SLICE_IDR:
  case NAL_UNIT_CODED_SLICE_SCALABLE:
    {
      RNOK( rcAccessUnitSlices.update( pcBinDataCopy, *pcSliceHeader ) );
      break;
    }
  default:
    break;
  }
  return Err::m_nOK;
}


ErrVal
H264AVCDecoder::processSliceData( PicBuffer*        pcPicBuffer,
                                  PicBufferList&    rcPicBufferOutputList,
                                  PicBufferList&    rcPicBufferUnusedList,
                                  BinDataList&      rcBinDataList,
                                  SliceDataNALUnit& rcSliceDataNALUnit )
{
  PicBufferList   cDummyList;
  PicBufferList&  rcOutputList = ( rcSliceDataNALUnit.isDependencyIdMax() ? rcPicBufferOutputList : cDummyList );
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
H264AVCDecoder::getBaseLayerDataAvailability( Frame*&       pcFrame,
                                              Frame*&       pcResidual,
                                              MbDataCtrl*&  pcMbDataCtrl,
                                              Bool&         rbBaseDataAvailable,
                                              Bool&         rbSpatialScalability,
                                              UInt          uiLayerId,
                                              UInt          uiBaseLayerId )
{
  if( m_apcLayerDecoder[uiLayerId]->getFrameHeight() != m_apcLayerDecoder[uiBaseLayerId]->getFrameHeight() &&
      m_apcLayerDecoder[uiLayerId]->getFrameWidth () != m_apcLayerDecoder[uiBaseLayerId]->getFrameWidth ()   )
  {
    rbSpatialScalability = true;
  }
  else
  {
    rbSpatialScalability = false;
  }
  RNOK( m_apcLayerDecoder[uiBaseLayerId]->getBaseLayerDataAvailability( pcFrame, pcResidual, pcMbDataCtrl, rbBaseDataAvailable ) );
  return Err::m_nOK;
}

ErrVal
H264AVCDecoder::getBaseLayerData( Frame*&       pcFrame,
                                  Frame*&       pcResidual,
                                  MbDataCtrl*&  pcMbDataCtrl,
                                  Bool&         rbConstrainedIPred,
                                  Bool&         rbSpatialScalability,
                                  UInt          uiBaseLayerId )
{
  RNOK( m_apcLayerDecoder[uiBaseLayerId]->getBaseLayerData( pcFrame, pcResidual, pcMbDataCtrl, rbConstrainedIPred, rbSpatialScalability ) );
  return Err::m_nOK;
}

ErrVal
H264AVCDecoder::getBaseLayerResidual( Frame*& pcResidual,
                                      UInt    uiBaseLayerId )
{
  pcResidual = m_apcLayerDecoder[uiBaseLayerId]->getBaseLayerResidual();
  return Err::m_nOK;
}

ErrVal
H264AVCDecoder::getBaseLayerPWTable( PredWeightTable*& rpcPredWeightTable,
                                     UInt              uiRefLayerDependencyId,
                                     ListIdx           eListIdx )
{
  RNOK( m_apcLayerDecoder[ uiRefLayerDependencyId ]->getBaseLayerPWTable( rpcPredWeightTable, eListIdx ) );
  return Err::m_nOK;
}


H264AVC_NAMESPACE_END

