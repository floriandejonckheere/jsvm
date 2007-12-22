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



#include "H264AVCEncoderLib.h"
#include "BitWriteBuffer.h"
#include "NalUnitEncoder.h"

#include "CodingParameter.h"

H264AVC_NAMESPACE_BEGIN


NalUnitEncoder::NalUnitEncoder()
: m_bIsUnitActive         ( false )
, m_pcBitWriteBuffer      ( 0 )
, m_pcHeaderSymbolWriteIf ( 0 )
, m_pcHeaderSymbolTestIf  ( 0 )
, m_pcBinDataAccessor     ( 0 )
, m_pucBuffer             ( 0 )
, m_pucTempBuffer         ( 0 )
, m_pucTempBufferBackup   ( 0 )
, m_uiPacketLength        ( MSYS_UINT_MAX )
, m_eNalUnitType          ( NAL_UNIT_UNSPECIFIED_0 )
, m_eNalRefIdc            ( NAL_REF_IDC_PRIORITY_LOWEST )
{
}


NalUnitEncoder::~NalUnitEncoder()
{
}


ErrVal
NalUnitEncoder::create( NalUnitEncoder*& rpcNalUnitEncoder )
{
  rpcNalUnitEncoder = new NalUnitEncoder;
  ROT( NULL == rpcNalUnitEncoder );
  return Err::m_nOK;
}


ErrVal
NalUnitEncoder::init( BitWriteBuffer*       pcBitWriteBuffer,
                      HeaderSymbolWriteIf*  pcHeaderSymbolWriteIf,
                      HeaderSymbolWriteIf*  pcHeaderSymbolTestIf )
{
  ROT( NULL == pcBitWriteBuffer );
  ROT( NULL == pcHeaderSymbolWriteIf );
  ROT( NULL == pcHeaderSymbolTestIf );

  m_pcBitWriteBuffer      = pcBitWriteBuffer;
  m_pcHeaderSymbolTestIf  = pcHeaderSymbolTestIf;
  m_pcHeaderSymbolWriteIf = pcHeaderSymbolWriteIf;
  m_bIsUnitActive         = false;
  m_pucBuffer             = NULL;
  m_pucTempBuffer         = NULL;
  m_pucTempBufferBackup   = NULL;
  m_uiPacketLength        = MSYS_UINT_MAX;
  m_eNalUnitType          = NAL_UNIT_UNSPECIFIED_0;
  m_eNalRefIdc            = NAL_REF_IDC_PRIORITY_LOWEST;

  return Err::m_nOK;
}


ErrVal
NalUnitEncoder::uninit()
{
  delete [] m_pucTempBuffer;
  delete [] m_pucTempBufferBackup;
  m_pucTempBufferBackup   = NULL;

  m_pcBitWriteBuffer      = NULL;
  m_pcHeaderSymbolWriteIf = NULL;
  m_pcHeaderSymbolTestIf  = NULL;
  m_bIsUnitActive         = false;
  m_pucBuffer             = NULL;
  m_pucTempBuffer         = NULL;
  m_uiPacketLength        = MSYS_UINT_MAX;
  m_eNalUnitType          = NAL_UNIT_UNSPECIFIED_0;
  m_eNalRefIdc            = NAL_REF_IDC_PRIORITY_LOWEST;

  return Err::m_nOK;
}


ErrVal
NalUnitEncoder::destroy()
{
  uninit();
  delete this;
  return Err::m_nOK;
}


ErrVal
NalUnitEncoder::initNalUnit( BinDataAccessor* pcBinDataAccessor )
{
  ROT( m_bIsUnitActive );
  ROF( pcBinDataAccessor );
  ROT( pcBinDataAccessor->size() < 1 );

  m_bIsUnitActive     = true;
  m_pcBinDataAccessor = pcBinDataAccessor;
  m_pucBuffer         = pcBinDataAccessor->data();

  if( m_uiPacketLength != m_pcBinDataAccessor->size() )
  {
    delete [] m_pucTempBuffer;

    m_uiPacketLength = m_pcBinDataAccessor->size();
    m_pucTempBuffer  = new UChar[ m_uiPacketLength ];
    ROF( m_pucTempBuffer );
  }

  RNOK( m_pcBitWriteBuffer->initPacket( (ULong*)(m_pucTempBuffer), m_uiPacketLength-1 ) );

  return Err::m_nOK;
}



ErrVal
NalUnitEncoder::closeAndAppendNalUnits( UInt                    *pauiBits,
                                        ExtBinDataAccessorList  &rcExtBinDataAccessorList,
                                        ExtBinDataAccessor      *pcExtBinDataAccessor,
                                        BinData                 &rcBinData,
                                        H264AVCEncoder          *pcH264AVCEncoder,
                                        UInt                    uiQualityLevelCGSSNR,
                                        UInt                    uiLayerCGSSNR )
{
  ROF( m_bIsUnitActive );
  ROF( pcExtBinDataAccessor );
  ROF( pcExtBinDataAccessor->data() );

  ROF( m_pcBinDataAccessor == pcExtBinDataAccessor );

  //===== write trailing bits =====
  if( NAL_UNIT_END_OF_SEQUENCE != m_eNalUnitType &&
      NAL_UNIT_END_OF_STREAM   != m_eNalUnitType &&
     (NAL_UNIT_PREFIX          != m_eNalUnitType || m_eNalRefIdc != NAL_REF_IDC_PRIORITY_LOWEST) )
  {
    RNOK( xWriteTrailingBits() );
  }
  RNOK( m_pcBitWriteBuffer->flushBuffer() );

  //===== convert to payload and add header =====
  UInt  uiHeaderBytes = 1;
  if( m_eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE || m_eNalUnitType == NAL_UNIT_PREFIX )
  {
    uiHeaderBytes += NAL_UNIT_HEADER_SVC_EXTENSION_BYTES;
  }

  BitWriteBufferIf *pcCurrentWriteBuffer = m_pcBitWriteBuffer;
  UChar            *pucPayload           = m_pucBuffer;
  const UChar      *pucRBSP              = m_pucTempBuffer;
  UInt              uiPayloadBufferSize  = m_uiPacketLength;

  ROF( pcExtBinDataAccessor->data() == pucPayload          );
  ROF( pcExtBinDataAccessor->size() == uiPayloadBufferSize );

  UInt uiFragment = 0;
  while( true )
  {
    UInt uiBits  = pcCurrentWriteBuffer->getNumberOfWrittenBits();
    UInt uiBytes = ( uiBits + 7 ) >> 3;
    RNOK( convertRBSPToPayload( uiBytes, uiHeaderBytes, pucPayload, pucRBSP, uiPayloadBufferSize ) );
    pauiBits[uiFragment] = 8 * uiBytes;

    UChar* pucNewBuffer = new UChar [ uiBytes ];
    ROF( pucNewBuffer );
    ::memcpy( pucNewBuffer, pucPayload, uiBytes * sizeof( UChar ) );

    if( pcH264AVCEncoder )
    {
      //JVT-W052
      if(pcH264AVCEncoder->getCodingParameter()->getIntegrityCheckSEIEnable() && pcH264AVCEncoder->getCodingParameter()->getCGSSNRRefinement() )
      {
        if( uiQualityLevelCGSSNR + uiFragment > 0 )
        {
          UInt uicrcMsb,uicrcVal;
          uicrcVal = pcH264AVCEncoder->m_uicrcVal[uiLayerCGSSNR];
          uicrcMsb = 0;
          Bool BitVal = false;
          for ( ULong uiBitIdx = 0; uiBitIdx< uiBytes*8; uiBitIdx++ )
          {
            uicrcMsb = ( uicrcVal >> 15 ) & 1;
            BitVal = ( pucNewBuffer[uiBitIdx>>3] >> (7-(uiBitIdx&7)) )&1;
            uicrcVal = (((uicrcVal<<1) + BitVal ) & 0xffff)^(uicrcMsb*0x1021);
          }
          pcH264AVCEncoder->m_uicrcVal[uiLayerCGSSNR] = uicrcVal;
          if( pcH264AVCEncoder->m_uiNumofCGS[uiLayerCGSSNR] == uiQualityLevelCGSSNR + uiFragment )
          {
            ROT( pcCurrentWriteBuffer->nextBitWriteBufferActive() );
            for(ULong uiBitIdx = 0; uiBitIdx< 16; uiBitIdx++)
            {
              uicrcMsb = ( uicrcVal >> 15 ) & 1;
              BitVal = 0;
              uicrcVal = (((uicrcVal<<1) + BitVal ) & 0xffff)^(uicrcMsb*0x1021);
            }
            pcH264AVCEncoder->m_uicrcVal[uiLayerCGSSNR] = uicrcVal;
            pcH264AVCEncoder->m_pcIntegrityCheckSEI->setNumInfoEntriesMinus1(uiLayerCGSSNR);
            pcH264AVCEncoder->m_pcIntegrityCheckSEI->setEntryDependencyId(uiLayerCGSSNR,uiLayerCGSSNR);
            pcH264AVCEncoder->m_pcIntegrityCheckSEI->setQualityLayerCRC(uiLayerCGSSNR,uicrcVal);
          }
        }

        //JVT-W052 bug_fixed
        if(pcH264AVCEncoder->getCodingParameter()->getNumberOfLayers() == 1)
        {
          pcH264AVCEncoder->m_pcIntegrityCheckSEI->setNumInfoEntriesMinus1(0);
          pcH264AVCEncoder->m_pcIntegrityCheckSEI->setEntryDependencyId(0,0);
          pcH264AVCEncoder->m_pcIntegrityCheckSEI->setQualityLayerCRC(0,0);
        }
        //JVT-W052 bug_fixed
      }
      //JVT-W052
    }

    ExtBinDataAccessor* pcNewExtBinDataAccessor = new ExtBinDataAccessor;
    ROF( pcNewExtBinDataAccessor );

    rcBinData               .reset          ();
    rcBinData               .set            (  pucNewBuffer, uiBytes );
    rcBinData               .setMemAccessor ( *pcNewExtBinDataAccessor );
    rcExtBinDataAccessorList.push_back      (  pcNewExtBinDataAccessor );

    rcBinData               .reset          ();
    rcBinData               .setMemAccessor ( *pcExtBinDataAccessor );

    if( !pcCurrentWriteBuffer->nextBitWriteBufferActive() )
    {
      break;
    }
    pucRBSP              = pcCurrentWriteBuffer->getNextBuffersPacket();
    pcCurrentWriteBuffer = pcCurrentWriteBuffer->getNextBitWriteBuffer();
    uiFragment++;
  }

  RNOK( m_pcBitWriteBuffer->uninit() );

  //==== reset parameters =====
  m_bIsUnitActive     = false;
  m_pucBuffer         = NULL;
  m_pcBinDataAccessor = NULL;
  m_eNalUnitType      = NAL_UNIT_UNSPECIFIED_0;
  m_eNalRefIdc        = NAL_REF_IDC_PRIORITY_LOWEST;
  return Err::m_nOK;
}



ErrVal
NalUnitEncoder::closeNalUnit( UInt& ruiBits )
{
  ROF( m_bIsUnitActive );

  //===== write trailing bits =====
  if( NAL_UNIT_END_OF_SEQUENCE != m_eNalUnitType &&
      NAL_UNIT_END_OF_STREAM   != m_eNalUnitType &&
     (NAL_UNIT_PREFIX          != m_eNalUnitType || m_eNalRefIdc != NAL_REF_IDC_PRIORITY_LOWEST) )
  {
    RNOK ( xWriteTrailingBits() );
  }
  RNOK( m_pcBitWriteBuffer->flushBuffer() );

  //===== convert to payload and add header =====
  UInt  uiHeaderBytes = 1;
  if( m_eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE || m_eNalUnitType == NAL_UNIT_PREFIX )
  {
    uiHeaderBytes += NAL_UNIT_HEADER_SVC_EXTENSION_BYTES;
  }

  UInt  uiBits = ( m_pcBitWriteBuffer->getNumberOfWrittenBits() + 7 ) >> 3;

  RNOK( convertRBSPToPayload( uiBits, uiHeaderBytes, m_pucBuffer, m_pucTempBuffer, m_uiPacketLength ) );
  RNOK( m_pcBinDataAccessor->decreaseEndPos( m_pcBinDataAccessor->size() - uiBits ) );
  ruiBits             = 8*uiBits;

  //==== reset parameters =====
  m_bIsUnitActive     = false;
  m_pucBuffer         = NULL;
  m_pcBinDataAccessor = NULL;
  m_eNalUnitType      = NAL_UNIT_UNSPECIFIED_0;
  m_eNalRefIdc        = NAL_REF_IDC_PRIORITY_LOWEST;
  return Err::m_nOK;
}

ErrVal
NalUnitEncoder::convertRBSPToPayload( UInt         &ruiBytesWritten,
                                       UInt          uiHeaderBytes,
                                       UChar        *pcPayload,
                                       const UChar  *pcRBSP,
                                       UInt          uiPayloadBufferSize )
{
  UInt uiZeroCount    = 0;
  UInt uiReadOffset   = uiHeaderBytes;
  UInt uiWriteOffset  = uiHeaderBytes;

  //===== NAL unit header =====
  for( UInt uiIndex = 0; uiIndex < uiHeaderBytes; uiIndex++ )
  {
    pcPayload[uiIndex] = pcRBSP[uiIndex];
  }

  //===== NAL unit payload =====
  for( ; uiReadOffset < ruiBytesWritten ; uiReadOffset++, uiWriteOffset++ )
  {
    ROT( uiWriteOffset >= uiPayloadBufferSize );

    if( 2 == uiZeroCount && 0 == ( pcRBSP[uiReadOffset] & 0xfc ) )
    {
      uiZeroCount                   = 0;
      pcPayload[uiWriteOffset++]  = 0x03;
    }

    pcPayload[uiWriteOffset] = pcRBSP[uiReadOffset];

    if( 0 == pcRBSP[uiReadOffset] )
    {
      uiZeroCount++;
    }
    else
    {
      uiZeroCount = 0;
    }
  }
  if( ( 0x00 == pcPayload[uiWriteOffset-1] ) && ( 0x00 == pcPayload[uiWriteOffset-2] ) )
  {
    ROT( uiWriteOffset >= uiPayloadBufferSize );
    pcPayload[uiWriteOffset++] = 0x03;
  }
  ruiBytesWritten = uiWriteOffset;

  return Err::m_nOK;
}

ErrVal NalUnitEncoder::xWriteTrailingBits( )
{
  RNOK( m_pcBitWriteBuffer->write( 1 ) );
  RNOK( m_pcBitWriteBuffer->writeAlignZero() );

  BitWriteBufferIf* pcCurrentBitWriter = m_pcBitWriteBuffer;
  while( pcCurrentBitWriter->nextBitWriteBufferActive() )
  {
    pcCurrentBitWriter = pcCurrentBitWriter->getNextBitWriteBuffer();
    RNOK( pcCurrentBitWriter->write( 1 ) );
    RNOK( pcCurrentBitWriter->writeAlignZero() );
  }
  return Err::m_nOK;
}


ErrVal
NalUnitEncoder::write( const SequenceParameterSet& rcSPS )
{
  RNOK( rcSPS.write( m_pcHeaderSymbolWriteIf ) );
  m_eNalUnitType  = rcSPS.getNalUnitType();
  m_eNalRefIdc    = NAL_REF_IDC_PRIORITY_HIGHEST;
  return Err::m_nOK;
}


ErrVal
NalUnitEncoder::write( const PictureParameterSet& rcPPS )
{
  RNOK( rcPPS.write( m_pcHeaderSymbolWriteIf ) );

  m_eNalUnitType  = rcPPS.getNalUnitType();
  m_eNalRefIdc    = NAL_REF_IDC_PRIORITY_HIGHEST;
  return Err::m_nOK;
}


ErrVal
NalUnitEncoder::writePrefix( const SliceHeader& rcSH )
{
  RNOK( rcSH.writePrefix( *m_pcHeaderSymbolWriteIf ) );
  m_eNalUnitType  = NAL_UNIT_PREFIX;
  m_eNalRefIdc    = rcSH.getNalRefIdc();
  return Err::m_nOK;
}


ErrVal
NalUnitEncoder::write( const SliceHeader& rcSH )
{
  SliceHeader           cSH           = rcSH;
  HeaderSymbolWriteIf*  pcCurrWriteIf = m_pcHeaderSymbolWriteIf;
  UInt                  uiSourceLayer = g_nLayer;

  for( UInt uiMGSFragment = 0; true; uiMGSFragment++ )
  {
    ETRACE_DECLARE( Bool m_bTraceEnable = true );

    //----- modify copy of slice header -----
    cSH.setDependencyId                   ( rcSH.getLayerCGSSNR         () );
    cSH.setQualityId                      ( rcSH.getQualityLevelCGSSNR  () + uiMGSFragment );
    cSH.setDiscardableFlag                ( rcSH.getDiscardableFlag     () || cSH.getQualityId() >= rcSH.getQLDiscardable() );
    cSH.setNoInterLayerPredFlag           ( rcSH.getNoInterLayerPredFlag() && cSH.getQualityId() == 0 );
    cSH.setScanIdxStart                   ( rcSH.getSPS().getMGSCoeffStart( uiMGSFragment ) );
    cSH.setScanIdxStop                    ( rcSH.getSPS().getMGSCoeffStop ( uiMGSFragment ) );
    cSH.setRefLayerDQId                   ( uiMGSFragment == 0 ? rcSH.getRefLayerDQId                   () : ( rcSH.getLayerCGSSNR() << 4 ) + rcSH.getQualityLevelCGSSNR() + uiMGSFragment - 1 );
    cSH.setAdaptiveBaseModeFlag           ( uiMGSFragment == 0 ? rcSH.getAdaptiveBaseModeFlag           () : false  );
    cSH.setAdaptiveMotionPredictionFlag   ( uiMGSFragment == 0 ? rcSH.getAdaptiveMotionPredictionFlag   () : false  );
    cSH.setAdaptiveResidualPredictionFlag ( uiMGSFragment == 0 ? rcSH.getAdaptiveResidualPredictionFlag () : false  );
    cSH.setDefaultBaseModeFlag            ( uiMGSFragment == 0 ? rcSH.getDefaultBaseModeFlag            () : true   );
    cSH.setDefaultMotionPredictionFlag    ( uiMGSFragment == 0 ? rcSH.getDefaultMotionPredictionFlag    () : true   );
    cSH.setDefaultResidualPredictionFlag  ( uiMGSFragment == 0 ? rcSH.getDefaultResidualPredictionFlag  () : true   );

    //----- write copy of slice header -----
    RNOK( cSH.write( *pcCurrWriteIf ) );
    if( rcSH.getSPS().getMGSCoeffStop( uiMGSFragment ) == 16 )
    {
      break;
    }

    //----- update -----
    g_nLayer++;
    ETRACE_LAYER( g_nLayer );
    pcCurrWriteIf = pcCurrWriteIf->getHeaderSymbolWriteIfNextSlice( true );
  }

  ETRACE_DECLARE( Bool m_bTraceEnable = true );
  g_nLayer = uiSourceLayer;
  ETRACE_LAYER( g_nLayer );

  m_eNalUnitType  = rcSH.getNalUnitType ();
  m_eNalRefIdc    = rcSH.getNalRefIdc   ();
  return Err::m_nOK;
}


ErrVal
NalUnitEncoder::write( SEI::MessageList& rcSEIMessageList )
{
  RNOK( SEI::write( m_pcHeaderSymbolWriteIf, m_pcHeaderSymbolTestIf, &rcSEIMessageList ) );

  m_eNalUnitType  = NAL_UNIT_SEI;
  m_eNalRefIdc    = NAL_REF_IDC_PRIORITY_LOWEST;
  return Err::m_nOK;
}

// JVT-T073 {
ErrVal
NalUnitEncoder::writeNesting( SEI::MessageList& rcSEIMessageList )
{
  RNOK( SEI::writeNesting( m_pcHeaderSymbolWriteIf, m_pcHeaderSymbolTestIf, &rcSEIMessageList ) );
  m_eNalUnitType  = NAL_UNIT_SEI;
  m_eNalRefIdc    = NAL_REF_IDC_PRIORITY_LOWEST;
  return Err::m_nOK;
}
// JVT-T073 }

// JVT-V068 {
ErrVal
NalUnitEncoder::writeScalableNestingSei( SEI::MessageList& rcSEIMessageList )
{
  RNOK( SEI::writeScalableNestingSei( m_pcHeaderSymbolWriteIf, m_pcHeaderSymbolTestIf, &rcSEIMessageList ) );
  m_eNalUnitType  = NAL_UNIT_SEI;
  m_eNalRefIdc    = NAL_REF_IDC_PRIORITY_LOWEST;
  return Err::m_nOK;
}
// JVT-V068 }

H264AVC_NAMESPACE_END
