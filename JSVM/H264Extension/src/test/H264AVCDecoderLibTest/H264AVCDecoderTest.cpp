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



#include "H264AVCDecoderLibTest.h"
#include "H264AVCDecoderTest.h"



H264AVCDecoderTest::BufferParameters::BufferParameters()
: m_uiLumaOffset  ( 0 )
, m_uiCbOffset    ( 0 )
, m_uiCrOffset    ( 0 )
, m_uiLumaHeight  ( 0 )
, m_uiLumaWidth   ( 0 )
, m_uiLumaStride  ( 0 )
, m_uiBufferSize  ( 0 )
{
  ::memset( m_auiCropping, 0x00, sizeof( m_auiCropping ) );
}

H264AVCDecoderTest::BufferParameters::~BufferParameters()
{
}

ErrVal
H264AVCDecoderTest::BufferParameters::init( const h264::SliceDataNALUnit& rcSliceDataNalUnit )
{
  ROF( rcSliceDataNalUnit.isSliceHeaderPresent() );
  UInt uiMbX        = rcSliceDataNalUnit.getFrameWidthInMb  ();
  UInt uiMbY        = rcSliceDataNalUnit.getFrameHeightInMb ();
  UInt uiChromaSize = ( ( uiMbX << 3 ) + YUV_X_MARGIN     ) * ( ( uiMbY << 3 ) + YUV_Y_MARGIN );
  m_uiLumaOffset    = ( ( uiMbX << 4 ) + YUV_X_MARGIN * 2 ) * YUV_Y_MARGIN     + YUV_X_MARGIN;
  m_uiCbOffset      = ( ( uiMbX << 3 ) + YUV_X_MARGIN     ) * YUV_Y_MARGIN / 2 + YUV_X_MARGIN / 2 + uiChromaSize * 4;
  m_uiCrOffset      = ( ( uiMbX << 3 ) + YUV_X_MARGIN     ) * YUV_Y_MARGIN / 2 + YUV_X_MARGIN / 2 + uiChromaSize * 5;
  m_uiLumaHeight    =   ( uiMbY << 4 );
  m_uiLumaWidth     =   ( uiMbX << 4 );
  m_uiLumaStride    =   ( uiMbX << 4 ) + YUV_X_MARGIN * 2;
  m_uiBufferSize    = 6 * uiChromaSize;
  m_auiCropping[0]  = rcSliceDataNalUnit.getCroppingRectangle ()[0];
  m_auiCropping[1]  = rcSliceDataNalUnit.getCroppingRectangle ()[1];
  m_auiCropping[2]  = rcSliceDataNalUnit.getCroppingRectangle ()[2];
  m_auiCropping[3]  = rcSliceDataNalUnit.getCroppingRectangle ()[3];
  return Err::m_nOK;
}






H264AVCDecoderTest::H264AVCDecoderTest() 
: m_bInitialized          ( false )
, m_pcH264AVCDecoder      ( 0 )
, m_pcParameter           ( 0 )
, m_pcReadBitstream       ( 0 )
#ifdef SHARP_AVC_REWRITE_OUTPUT
, m_pcWriteBitstream      ( 0 )
#else
, m_pcWriteYuv            ( 0 )
, m_iMaxPocDiff           ( MSYS_INT_MAX )
, m_iLastPoc              ( MSYS_UINT_MAX )
, m_pcLastFrame           ( 0 )
#endif
{
#ifdef SHARP_AVC_REWRITE_OUTPUT
  m_aucStartCodeBuffer[ 0 ] = 0;
  m_aucStartCodeBuffer[ 1 ] = 0;
  m_aucStartCodeBuffer[ 2 ] = 0;
  m_aucStartCodeBuffer[ 3 ] = 1;
  m_cBinDataStartCode.reset ();
  m_cBinDataStartCode.set   ( m_aucStartCodeBuffer, 4 );
#endif
}

H264AVCDecoderTest::~H264AVCDecoderTest()
{
}

ErrVal
H264AVCDecoderTest::create( H264AVCDecoderTest*& rpcH264AVCDecoderTest )
{
  rpcH264AVCDecoderTest = new H264AVCDecoderTest;
  ROT( NULL == rpcH264AVCDecoderTest );
  return Err::m_nOK;
}

ErrVal
H264AVCDecoderTest::destroy()
{
  ROT( m_bInitialized );
#ifdef SHARP_AVC_REWRITE_OUTPUT
  m_cBinDataStartCode.reset();
#endif
  delete this;
  return Err::m_nOK;
}

ErrVal
H264AVCDecoderTest::init( DecoderParameter* pcDecoderParameter,
                          ReadBitstreamIf*  pcReadBitstream,
#ifdef SHARP_AVC_REWRITE_OUTPUT
                          WriteBitstreamIf* pcWriteBitStream )
#else
                          WriteYuvIf*       pcWriteYuv )
#endif
{
  ROT( m_bInitialized );
  ROF( pcDecoderParameter );
  ROF( pcReadBitstream );
#ifdef SHARP_AVC_REWRITE_OUTPUT
  ROF( pcWriteBitStream );
#else
  ROF( pcWriteYuv );
#endif

  h264::CreaterH264AVCDecoder* pcH264AVCDecoder = 0;
  RNOK( h264::CreaterH264AVCDecoder::create( pcH264AVCDecoder ) );

  m_bInitialized          = true;
  m_pcH264AVCDecoder      = pcH264AVCDecoder;
  m_pcParameter           = pcDecoderParameter;
  m_pcReadBitstream       = pcReadBitstream;
#ifdef SHARP_AVC_REWRITE_OUTPUT
  m_pcWriteBitstream      = pcWriteBitStream;
#else
  m_pcWriteYuv            = pcWriteYuv;
  m_iMaxPocDiff           = (Int)m_pcParameter->uiMaxPocDiff;
  m_iLastPoc              = MSYS_UINT_MAX;
  m_pcLastFrame           = 0;
#endif

  m_pcParameter->nResult  = -1;

  RNOK( m_pcH264AVCDecoder->init( true ) );

  return Err::m_nOK;
}

ErrVal
H264AVCDecoderTest::uninit()
{
  ROF( m_bInitialized );
  ROF( m_cActivePicBufferList.empty() );

  if( m_pcH264AVCDecoder )       
  {
    RNOK( m_pcH264AVCDecoder->uninit  ( true ) );
    RNOK( m_pcH264AVCDecoder->destroy () );       
  }

  //===== delete picture buffer =====
  PicBufferList::iterator iter;
  for( iter = m_cUnusedPicBufferList.begin(); iter != m_cUnusedPicBufferList.end(); iter++ )
  {
    delete (*iter)->getBuffer();
    delete (*iter);
  }
  for( iter = m_cActivePicBufferList.begin(); iter != m_cActivePicBufferList.end(); iter++ )
  {
    delete (*iter)->getBuffer();
    delete (*iter);
  }

#ifdef SHARP_AVC_REWRITE_OUTPUT
#else
  delete m_pcLastFrame;
#endif

  m_bInitialized = false;
  return Err::m_nOK;
}

ErrVal
H264AVCDecoderTest::go()
{
  UInt                    uiNumProcessed    = 0;
  Bool                    bFirstAccessUnit  = true;
  h264::AccessUnitSlices  cAccessUnitSlices;
  while( ! cAccessUnitSlices.isEndOfStream() )
  {
    RNOK( xProcessAccessUnit( cAccessUnitSlices, bFirstAccessUnit, uiNumProcessed ) );
  }
#ifdef SHARP_AVC_REWRITE_OUTPUT
  printf("\n%d NAL units rewritten\n\n",  uiNumProcessed );
#else
  printf("\n%d frames decoded\n\n",       uiNumProcessed );
#endif
  m_pcParameter->nFrames = uiNumProcessed;
  m_pcParameter->nResult = 0;

  return Err::m_nOK;
}

ErrVal  
H264AVCDecoderTest::xProcessAccessUnit( h264::AccessUnitSlices& rcAccessUnitSlices, Bool& rbFirstAccessUnit, UInt& ruiNumProcessed )
{
  ROT( rcAccessUnitSlices.isEndOfStream() );
  ROT( rcAccessUnitSlices.isComplete   () );

  //===== read next access unit ====
  while( !rcAccessUnitSlices.isComplete() )
  {
    BinData*  pcBinData     = 0;
    Bool      bEndOfStream  = false; // dummy
    RNOK( m_pcReadBitstream ->extractPacket ( pcBinData, bEndOfStream ) );
    RNOK( m_pcH264AVCDecoder->initNALUnit   ( pcBinData, rcAccessUnitSlices ) );
    RNOK( m_pcReadBitstream ->releasePacket ( pcBinData ) );
  }

  //===== get buffer dimensions =====
  if( rbFirstAccessUnit )
  {
    const h264::SliceDataNALUnit*  pcSliceDataNalUnit = 0;
    RNOK( rcAccessUnitSlices.getRefToTargetLayerSliceData( pcSliceDataNalUnit ) );
    RNOK( m_cBufferParameters.init( *pcSliceDataNalUnit ) );
    rbFirstAccessUnit = false;
#ifdef SHARP_AVC_REWRITE_OUTPUT
#else
    m_pcLastFrame     = new UChar [ m_cBufferParameters.getBufferSize() ];
    ROF ( m_pcLastFrame );
#endif
  }

  //===== decode access unit =====
  while( rcAccessUnitSlices.isComplete() )
  {
    h264::SliceDataNALUnit* pcSliceDataNalUnit = 0;
    PicBuffer*              pcPicBuffer        = 0;
    PicBufferList           cPicBufferOutputList;
    PicBufferList           cPicBufferUnusedList;
    BinDataList             cBinDataList;
    RNOK( rcAccessUnitSlices.getNextSliceDataNalUnit( pcSliceDataNalUnit ) );
    RNOK( xGetNewPicBuffer( pcPicBuffer, m_cBufferParameters.getBufferSize() ) );
    RNOK( m_pcH264AVCDecoder->processSliceData( pcPicBuffer, cPicBufferOutputList, cPicBufferUnusedList, cBinDataList, *pcSliceDataNalUnit ) );
    RNOK( xOutputNALUnits ( cBinDataList,         ruiNumProcessed ) );
    RNOK( xOutputPicBuffer( cPicBufferOutputList, ruiNumProcessed ) );
    RNOK( xRemovePicBuffer( cPicBufferUnusedList ) );
    delete pcSliceDataNalUnit;    
  }
  return Err::m_nOK;
}

ErrVal
H264AVCDecoderTest::xGetNewPicBuffer( PicBuffer*& rpcPicBuffer, UInt uiSize )
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
H264AVCDecoderTest::xOutputNALUnits( BinDataList& rcBinDataList, UInt& ruiNumNALUnits )
{
#ifdef SHARP_AVC_REWRITE_OUTPUT
  while( ! rcBinDataList.empty() )
  {
    BinData* pcBinData = rcBinDataList.popFront();
    ROF( pcBinData );
    RNOK( m_pcWriteBitstream->writePacket( &m_cBinDataStartCode ) );
    RNOK( m_pcWriteBitstream->writePacket( pcBinData ) );
    ruiNumNALUnits++;
    pcBinData->deleteData();
    delete pcBinData;
  }
#else
  ROF( rcBinDataList.empty() );
#endif
  return Err::m_nOK;
}

ErrVal
H264AVCDecoderTest::xOutputPicBuffer( PicBufferList& rcPicBufferOutputList, UInt& ruiNumFrames )
{
#ifdef SHARP_AVC_REWRITE_OUTPUT
  rcPicBufferOutputList.clear();
#else
  while( ! rcPicBufferOutputList.empty() )
  {
    PicBuffer* pcPicBuffer = rcPicBufferOutputList.popFront();
    ROF( pcPicBuffer );

    while( abs( m_iLastPoc + m_iMaxPocDiff ) < abs( (Int)pcPicBuffer->getCts() ) )
    {
      ROF ( m_pcLastFrame );
      RNOK( m_pcWriteYuv->writeFrame( m_pcLastFrame + m_cBufferParameters.getLumaOffset (),
                                      m_pcLastFrame + m_cBufferParameters.getCbOffset   (),
                                      m_pcLastFrame + m_cBufferParameters.getCrOffset (),
                                      m_cBufferParameters.getLumaHeight (),
                                      m_cBufferParameters.getLumaWidth  (),
                                      m_cBufferParameters.getLumaStride (),
                                      m_cBufferParameters.getCropping   () ) );
      printf("REPEAT FRAME\n");
      ruiNumFrames++;
      m_iLastPoc  += m_iMaxPocDiff;
    }

    m_pcLastFrame = pcPicBuffer->switchBuffer( m_pcLastFrame );
    ROF ( m_pcLastFrame );
    RNOK( m_pcWriteYuv->writeFrame( m_pcLastFrame + m_cBufferParameters.getLumaOffset (),
                                    m_pcLastFrame + m_cBufferParameters.getCbOffset   (),
                                    m_pcLastFrame + m_cBufferParameters.getCrOffset (),
                                    m_cBufferParameters.getLumaHeight (),
                                    m_cBufferParameters.getLumaWidth  (),
                                    m_cBufferParameters.getLumaStride (),
                                    m_cBufferParameters.getCropping   () ) );
    ruiNumFrames++;
    m_iLastPoc  += (Int)pcPicBuffer->getCts();
  }
#endif
  return Err::m_nOK;
}

ErrVal
H264AVCDecoderTest::xRemovePicBuffer( PicBufferList& rcPicBufferUnusedList )
{
  while( ! rcPicBufferUnusedList.empty() )
  {
    PicBuffer* pcBuffer = rcPicBufferUnusedList.popFront();
    if( pcBuffer )
    {
      PicBufferList::iterator  begin = m_cActivePicBufferList.begin();
      PicBufferList::iterator  end   = m_cActivePicBufferList.end  ();
      PicBufferList::iterator  iter  = std::find( begin, end, pcBuffer );

      ROT( iter == end ); // there is something wrong if the address is not in the active list
      ROT( pcBuffer->isUsed() );

      m_cUnusedPicBufferList.push_back( pcBuffer );
      m_cActivePicBufferList.erase    (  iter );
    }
  }
  return Err::m_nOK;
}


