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




#include "H264AVCEncoderLibTest.h"
#include "H264AVCEncoderTest.h"
#include "EncoderCodingParameter.h"



H264AVCEncoderTest::H264AVCEncoderTest() :
  m_pcH264AVCEncoder        ( NULL ),
  m_pcWriteBitstreamToFile  ( NULL ),
  m_pcEncoderCodingParameter( NULL )
{
  ::memset( m_apcReadYuv,   0x00, MAX_LAYERS*sizeof(Void*) );
  ::memset( m_apcWriteYuv,  0x00, MAX_LAYERS*sizeof(Void*) );
  ::memset( m_auiLumOffset, 0x00, MAX_LAYERS*sizeof(UInt) );
  ::memset( m_auiCbOffset,  0x00, MAX_LAYERS*sizeof(UInt) );
  ::memset( m_auiCrOffset,  0x00, MAX_LAYERS*sizeof(UInt) );
  ::memset( m_auiHeight,    0x00, MAX_LAYERS*sizeof(UInt) );
  ::memset( m_auiWidth,     0x00, MAX_LAYERS*sizeof(UInt) );
  ::memset( m_auiStride,    0x00, MAX_LAYERS*sizeof(UInt) );
}


H264AVCEncoderTest::~H264AVCEncoderTest()
{
}


ErrVal
H264AVCEncoderTest::create( H264AVCEncoderTest*& rpcH264AVCEncoderTest )
{
  rpcH264AVCEncoderTest = new H264AVCEncoderTest;

  ROT( NULL == rpcH264AVCEncoderTest );
  
  return Err::m_nOK;
}



ErrVal H264AVCEncoderTest::init( Int    argc,
                                 Char** argv )
{
  //===== create and read encoder parameters =====
  RNOK( EncoderCodingParameter::create( m_pcEncoderCodingParameter ) );
  if( Err::m_nOK != m_pcEncoderCodingParameter->init( argc, argv, m_cEncoderIoParameter.pBitstreamFile ) )
  {
    m_pcEncoderCodingParameter->printHelp();
    return -3;
  }
  m_cEncoderIoParameter.nResult = -1;


  //===== init instances for reading and writing yuv data =====
  for( UInt uiLayer = 0; uiLayer < m_pcEncoderCodingParameter->getNumberOfLayers(); uiLayer++ )
  {
    h264::LayerParameters&  rcLayer = m_pcEncoderCodingParameter->getLayerParameters( uiLayer );

    RNOKS( WriteYuvToFile::create( m_apcWriteYuv[uiLayer], rcLayer.getOutputFilename() ) );
    RNOKS( ReadYuvFile   ::create( m_apcReadYuv [uiLayer] ) );  

    RNOKS( m_apcReadYuv[uiLayer]->init( rcLayer.getInputFilename(),
                                        rcLayer.getFrameHeight  (),
                                        rcLayer.getFrameWidth   () ) );  
  }


  //===== init bitstream writer =====
  RNOKS( WriteBitstreamToFile::create   ( m_pcWriteBitstreamToFile ) )
  RNOKS( m_pcWriteBitstreamToFile->init ( m_cEncoderIoParameter.pBitstreamFile ) );  
  

  //===== create encoder instance =====
  RNOK( h264::CreaterH264AVCEncoder::create( m_pcH264AVCEncoder ) );


  //===== set start code =====
  m_aucStartCodeBuffer[0] = 0;
  m_aucStartCodeBuffer[1] = 0;
  m_aucStartCodeBuffer[2] = 0;
  m_aucStartCodeBuffer[3] = 1;
  m_cBinDataStartCode.reset ();
  m_cBinDataStartCode.set   ( m_aucStartCodeBuffer, 4 );

  return Err::m_nOK;
}




ErrVal
H264AVCEncoderTest::destroy()
{
  m_cBinDataStartCode.reset();

  if( m_pcH264AVCEncoder )       
  {
    RNOK( m_pcH264AVCEncoder->uninit() );       
    RNOK( m_pcH264AVCEncoder->destroy() );       
  }

  for( UInt ui = 0; ui < MAX_LAYERS; ui++ )
  {
    if( m_apcWriteYuv[ui] )              
    {
      RNOK( m_apcWriteYuv[ui]->destroy() );  
    }

    if( m_apcReadYuv[ui] )              
    {
      RNOK( m_apcReadYuv[ui]->uninit() );  
      RNOK( m_apcReadYuv[ui]->destroy() );  
    }
  }

  if( m_pcWriteBitstreamToFile )     
  {
    RNOK( m_pcWriteBitstreamToFile->uninit() );  
    RNOK( m_pcWriteBitstreamToFile->destroy() );  
  }

  RNOK( m_pcEncoderCodingParameter->destroy());

  for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
  {
    AOF( m_acActivePicBufferList[uiLayer].empty() );
    
    //===== delete picture buffer =====
    PicBufferList::iterator iter;
    for( iter = m_acUnusedPicBufferList[uiLayer].begin(); iter != m_acUnusedPicBufferList[uiLayer].end(); iter++ )
    {
      delete (*iter)->getBuffer();
      delete (*iter);
    }
    for( iter = m_acActivePicBufferList[uiLayer].begin(); iter != m_acActivePicBufferList[uiLayer].end(); iter++ )
    {
      delete (*iter)->getBuffer();
      delete (*iter);
    }
  }

  delete this;
  return Err::m_nOK;
}


ErrVal
H264AVCEncoderTest::xGetNewPicBuffer ( PicBuffer*&  rpcPicBuffer,
                                       UInt         uiLayer,
                                       UInt         uiSize )
{
  if( m_acUnusedPicBufferList[uiLayer].empty() )
  {
    rpcPicBuffer = new PicBuffer( new UChar[ uiSize ] );
  }
  else
  {
    rpcPicBuffer = m_acUnusedPicBufferList[uiLayer].popFront();
  }

  m_acActivePicBufferList[uiLayer].push_back( rpcPicBuffer );

  return Err::m_nOK;
}


ErrVal
H264AVCEncoderTest::xRemovePicBuffer( PicBufferList&  rcPicBufferUnusedList,
                                      UInt            uiLayer )
{
  while( ! rcPicBufferUnusedList.empty() )
  {
    PicBuffer* pcBuffer = rcPicBufferUnusedList.popFront();

    if( NULL != pcBuffer )
    {
      PicBufferList::iterator begin = m_acActivePicBufferList[uiLayer].begin();
      PicBufferList::iterator end   = m_acActivePicBufferList[uiLayer].end  ();
      PicBufferList::iterator iter  = std::find( begin, end, pcBuffer );

      ROT( iter == end ); // there is something wrong if the address is not in the active list

      AOT_DBG( (*iter)->isUsed() );
      m_acUnusedPicBufferList[uiLayer].push_back( *iter );
      m_acActivePicBufferList[uiLayer].erase    (  iter );
    }
  }
  return Err::m_nOK;
}


ErrVal
H264AVCEncoderTest::xWrite( PicBufferList&  rcPicBufferList,
                            UInt            uiLayer )
{
  while( ! rcPicBufferList.empty() )
  {
    PicBuffer* pcBuffer = rcPicBufferList.popFront();

    Pel* pcBuf = pcBuffer->getBuffer();
    RNOK( m_apcWriteYuv[uiLayer]->writeFrame( pcBuf + m_auiLumOffset[uiLayer], 
                                              pcBuf + m_auiCbOffset [uiLayer],
                                              pcBuf + m_auiCrOffset [uiLayer],
                                              m_auiHeight           [uiLayer],
                                              m_auiWidth            [uiLayer],
                                              m_auiStride           [uiLayer] ) );
  }
  return Err::m_nOK;
}


ErrVal
H264AVCEncoderTest::xRelease( PicBufferList&  rcPicBufferList,
                              UInt            uiLayer )
{
  RNOK( xRemovePicBuffer( rcPicBufferList, uiLayer ) );
  return Err::m_nOK;
}


ErrVal
H264AVCEncoderTest::xWrite( ExtBinDataAccessorList& rcList,
                            UInt&                   ruiBytesInFrame )
{
  while( rcList.size() )
  {
    ruiBytesInFrame += rcList.front()->size() + 4;
    RNOK( m_pcWriteBitstreamToFile->writePacket( &m_cBinDataStartCode ) );
    RNOK( m_pcWriteBitstreamToFile->writePacket( rcList.front() ) );
    delete[] rcList.front()->data();
    delete   rcList.front();
    rcList.pop_front();
  }
  return Err::m_nOK;
}


ErrVal
H264AVCEncoderTest::xRelease( ExtBinDataAccessorList& rcList )
{
  while( rcList.size() )
  {
    delete[] rcList.front()->data();
    delete   rcList.front();
    rcList.pop_front();
  }
  return Err::m_nOK;
}


ErrVal
H264AVCEncoderTest::go()
{
  UInt                    uiWrittenBytes          = 0;
  const UInt              uiMaxFrame              = m_pcEncoderCodingParameter->getTotalFrames();
  UInt                    uiFrame;
  UInt                    uiLayer;
  UInt                    auiMbX                  [MAX_LAYERS];
  UInt                    auiMbY                  [MAX_LAYERS];
  UInt                    auiPicSize              [MAX_LAYERS];
  PicBuffer*              apcOriginalPicBuffer    [MAX_LAYERS];
  PicBuffer*              apcReconstructPicBuffer [MAX_LAYERS];
  PicBufferList           acPicBufferOutputList   [MAX_LAYERS];
  PicBufferList           acPicBufferUnusedList   [MAX_LAYERS];
  ExtBinDataAccessorList  cInExtBinDataAccessorList;
  ExtBinDataAccessorList  cOutExtBinDataAccessorList;
  ExtBinDataAccessorList  cUnusedExtBinDataAccessorList;


  
  //===== initialization =====
  RNOK( m_pcH264AVCEncoder->init( m_pcEncoderCodingParameter ) ); 


  //===== write parameter sets =====
  for( Bool bMoreSets = true; bMoreSets;  )
  {
    UChar   aucParameterSetBuffer[1000];
    BinData cBinData;
    cBinData.reset();
    cBinData.set( aucParameterSetBuffer, 1000 );

    ExtBinDataAccessor cExtBinDataAccessor;
    cBinData.setMemAccessor( cExtBinDataAccessor );

    RNOK( m_pcH264AVCEncoder      ->writeParameterSets( &cExtBinDataAccessor, bMoreSets) );
    RNOK( m_pcWriteBitstreamToFile->writePacket       ( &m_cBinDataStartCode ) );
    RNOK( m_pcWriteBitstreamToFile->writePacket       ( &cExtBinDataAccessor ) );
    
    uiWrittenBytes += 4 + cExtBinDataAccessor.size();
    cBinData.reset();
  }


  //===== determine parameters for required frame buffers =====
  for( uiLayer = 0; uiLayer < m_pcEncoderCodingParameter->getNumberOfLayers(); uiLayer++ )
  {
    auiMbX        [uiLayer] = m_pcEncoderCodingParameter->getLayerParameters( uiLayer ).getFrameWidth () >> 4;
    auiMbY        [uiLayer] = m_pcEncoderCodingParameter->getLayerParameters( uiLayer ).getFrameHeight() >> 4;
    UInt  uiSize            = ((auiMbY[uiLayer]<<4)+2*YUV_Y_MARGIN)*((auiMbX[uiLayer]<<4)+2*YUV_X_MARGIN);
    auiPicSize    [uiLayer] = ((auiMbX[uiLayer]<<4)+2*YUV_X_MARGIN)*((auiMbY[uiLayer]<<4)+2*YUV_Y_MARGIN)*3/2;
    m_auiLumOffset[uiLayer] = ((auiMbX[uiLayer]<<4)+2*YUV_X_MARGIN)* YUV_Y_MARGIN   + YUV_X_MARGIN;  
    m_auiCbOffset [uiLayer] = ((auiMbX[uiLayer]<<3)+  YUV_X_MARGIN)* YUV_Y_MARGIN/2 + YUV_X_MARGIN/2 + uiSize; 
    m_auiCrOffset [uiLayer] = ((auiMbX[uiLayer]<<3)+  YUV_X_MARGIN)* YUV_Y_MARGIN/2 + YUV_X_MARGIN/2 + 5*uiSize/4;
    m_auiHeight   [uiLayer] =   auiMbY[uiLayer]<<4;
    m_auiWidth    [uiLayer] =   auiMbX[uiLayer]<<4;
    m_auiStride   [uiLayer] =  (auiMbX[uiLayer]<<4)+ 2*YUV_X_MARGIN;
  }

  
  //===== loop over frames =====
  for( uiFrame = 0; uiFrame < uiMaxFrame; uiFrame++ )
  {
    //===== get picture buffers and read original pictures =====
    for( uiLayer = 0; uiLayer < m_pcEncoderCodingParameter->getNumberOfLayers(); uiLayer++ )
    {
      UInt  uiSkip = ( 1 << m_pcEncoderCodingParameter->getLayerParameters( uiLayer ).getTemporalResolution() );

      if( uiFrame % uiSkip == 0 )
      {
        RNOK( xGetNewPicBuffer( apcReconstructPicBuffer [uiLayer], uiLayer, auiPicSize[uiLayer] ) );
        RNOK( xGetNewPicBuffer( apcOriginalPicBuffer    [uiLayer], uiLayer, auiPicSize[uiLayer] ) );

        RNOK( m_apcReadYuv[uiLayer]->readFrame( *apcOriginalPicBuffer[uiLayer] + m_auiLumOffset[uiLayer],
                                                *apcOriginalPicBuffer[uiLayer] + m_auiCbOffset [uiLayer],
                                                *apcOriginalPicBuffer[uiLayer] + m_auiCrOffset [uiLayer],
                                                m_auiHeight [uiLayer],
                                                m_auiWidth  [uiLayer],
                                                m_auiStride [uiLayer] ) );
      }
      else
      {
        apcReconstructPicBuffer [uiLayer] = 0;
        apcOriginalPicBuffer    [uiLayer] = 0;
      }
    }

    //===== call encoder =====
    RNOK( m_pcH264AVCEncoder->process( cInExtBinDataAccessorList,
                                       cOutExtBinDataAccessorList,
                                       cUnusedExtBinDataAccessorList,
                                       apcOriginalPicBuffer,
                                       apcReconstructPicBuffer,
                                       acPicBufferOutputList,
                                       acPicBufferUnusedList ) );

    //===== write and release NAL unit buffers =====
    UInt  uiBytesUsed = 0;
    RNOK( xWrite  ( cOutExtBinDataAccessorList, uiBytesUsed ) );
    RNOK( xRelease( cUnusedExtBinDataAccessorList ) );
    uiWrittenBytes   += uiBytesUsed;
    
    //===== write and release reconstructed pictures =====
    for( uiLayer = 0; uiLayer < m_pcEncoderCodingParameter->getNumberOfLayers(); uiLayer++ )
    {
      RNOK( xWrite  ( acPicBufferOutputList[uiLayer], uiLayer ) );
      RNOK( xRelease( acPicBufferUnusedList[uiLayer], uiLayer ) );
    }
  }

  
  //===== finish encoding =====
  UInt  uiNumCodedFrames = 0;
  Double  dHighestLayerOutputRate = 0.0;
  RNOK( m_pcH264AVCEncoder->finish( cInExtBinDataAccessorList,
                                    cOutExtBinDataAccessorList,
                                    cUnusedExtBinDataAccessorList,
                                    acPicBufferOutputList,
                                    acPicBufferUnusedList,
                                    uiNumCodedFrames,
                                    dHighestLayerOutputRate ) );

  //===== write and release NAL unit buffers =====
  RNOK( xWrite  ( cOutExtBinDataAccessorList, uiWrittenBytes ) );
  RNOK( xRelease( cUnusedExtBinDataAccessorList ) );
  RNOK( xRelease( cInExtBinDataAccessorList ) );

  //===== write and release reconstructed pictures =====
  for( uiLayer = 0; uiLayer < m_pcEncoderCodingParameter->getNumberOfLayers(); uiLayer++ )
  {
    RNOK( xWrite  ( acPicBufferOutputList[uiLayer], uiLayer ) );
    RNOK( xRelease( acPicBufferUnusedList[uiLayer], uiLayer ) );
  }


  //===== set parameters and output summary =====
  m_cEncoderIoParameter.nFrames = uiFrame;
  m_cEncoderIoParameter.nResult = 0;

  printf( "\n\n%d bit [%d byte]   rate: %.4f kbit/s  (%d frames @ %.2f fps)\n",
    uiWrittenBytes*8,
    uiWrittenBytes,
    0.008f*(Double)uiWrittenBytes*dHighestLayerOutputRate/(Double)uiNumCodedFrames,
    uiNumCodedFrames,
    dHighestLayerOutputRate );

  return Err::m_nOK;
}




