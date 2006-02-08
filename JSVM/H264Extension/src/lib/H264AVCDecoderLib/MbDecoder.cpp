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
#include "H264AVCCommonLib/Tables.h"


#include "H264AVCCommonLib/Transform.h"
#include "H264AVCCommonLib/IntraPrediction.h"
#include "H264AVCCommonLib/MotionCompensation.h"
#include "H264AVCCommonLib/FrameMng.h"
#include "MbDecoder.h"



H264AVC_NAMESPACE_BEGIN



MbDecoder::MbDecoder():
  m_pcTransform( NULL ),
  m_pcIntraPrediction( NULL ),
  m_pcMotionCompensation( NULL ),
  m_pcFrameMng( NULL ),
  m_bInitDone( false )
{
}

MbDecoder::~MbDecoder()
{
}


ErrVal MbDecoder::create( MbDecoder*& rpcMbDecoder )
{
  rpcMbDecoder = new MbDecoder;

  ROT( NULL == rpcMbDecoder );

  return Err::m_nOK;
}


ErrVal MbDecoder::destroy()
{
  delete this;

  return Err::m_nOK;
}



ErrVal MbDecoder::init( Transform*          pcTransform,
                        IntraPrediction*    pcIntraPrediction,
                        MotionCompensation* pcMotionCompensation,
                        FrameMng*           pcFrameMng )
{
  ROT( NULL == pcTransform );
  ROT( NULL == pcIntraPrediction );
  ROT( NULL == pcMotionCompensation );
  ROT( NULL == pcFrameMng );

  m_pcFrameMng            = pcFrameMng;
  m_pcTransform           = pcTransform;
  m_pcIntraPrediction     = pcIntraPrediction;
  m_pcMotionCompensation  = pcMotionCompensation;
  m_bInitDone             = true;

  return Err::m_nOK;
}


ErrVal MbDecoder::uninit()
{
  m_pcTransform           = NULL;
  m_pcIntraPrediction     = NULL;
  m_pcMotionCompensation  = NULL;
  m_bInitDone             = false;
  
  return Err::m_nOK;
}



ErrVal
MbDecoder::decode( MbDataAccess&  rcMbDataAccess,
                   MbDataAccess*  pcMbDataAccessBase,
                   IntFrame*      pcFrame,
                   IntFrame*      pcResidual,
                   IntFrame*      pcPredSignal,
                   IntFrame*      pcBaseLayer,
                   IntFrame*      pcBaseLayerResidual,
                   RefFrameList*  pcRefFrameList0,
                   RefFrameList*  pcRefFrameList1,
                   Bool           bReconstructAll )
{
  ROF( m_bInitDone );

  IntYuvMbBuffer  cPredBuffer;  cPredBuffer.setAllSamplesToZero();

  //===== scale coefficients =====
  RNOK( xScaleTCoeffs( rcMbDataAccess ) );

  if( rcMbDataAccess.getMbData().isIntra() )
  {
    //===== clear residual signal for intra macroblocks =====
    if( pcResidual )
    {
      IntYuvMbBuffer  cYuvMbBuffer;  cYuvMbBuffer.setAllSamplesToZero();
      RNOK( pcResidual->getFullPelYuvBuffer()->loadBuffer( &cYuvMbBuffer ) );
    }

    if( rcMbDataAccess.getMbData().isPCM() )
    {
      //===== I_PCM mode =====
      RNOK( xDecodeMbPCM( rcMbDataAccess, pcFrame->getFullPelYuvBuffer() ) );
      cPredBuffer.loadBuffer( pcFrame->getFullPelYuvBuffer() );
    }
    else if( rcMbDataAccess.getMbData().getMbMode() == INTRA_BL )
    {
      //===== I_BL mode =====
      RNOK( xDecodeMbIntraBL( rcMbDataAccess,  pcFrame->getFullPelYuvBuffer(),
                              cPredBuffer, pcBaseLayer->getFullPelYuvBuffer() ) );
    }
    else
    {
      m_pcIntraPrediction->setAvailableMaskMb( rcMbDataAccess.getAvailableMask() );
      IntYuvMbBuffer cRecBuffer;  cRecBuffer.loadIntraPredictors( pcFrame->getFullPelYuvBuffer() );

      if( rcMbDataAccess.getMbData().isIntra16x16() )
      {
        //===== I_16x16 mode ====
        RNOK( xDecodeMbIntra16x16( rcMbDataAccess, cRecBuffer, cPredBuffer ) );
      }
      else if( rcMbDataAccess.getMbData().isTransformSize8x8() )
      {
        //===== I_8x8 mode =====
        RNOK( xDecodeMbIntra8x8( rcMbDataAccess, cRecBuffer, cPredBuffer ) );
      }
      else
      {
        //===== I_4x4 mode =====
        RNOK( xDecodeMbIntra4x4( rcMbDataAccess, cRecBuffer, cPredBuffer ) );
      }
      RNOK( pcFrame->getFullPelYuvBuffer()->loadBuffer( &cRecBuffer ) );
    }
  }
  else
  {
    //===== motion-compensated modes =====
    RNOK( xDecodeMbInter( rcMbDataAccess, pcMbDataAccessBase,
                          cPredBuffer, pcFrame->getFullPelYuvBuffer(),
                          pcResidual, pcBaseLayerResidual,
                          *pcRefFrameList0, *pcRefFrameList1, bReconstructAll ) );
  }


  //===== store prediction signal =====
  if( pcPredSignal )
  {
    RNOK( pcPredSignal->getFullPelYuvBuffer()->loadBuffer( &cPredBuffer ) );
  }

  return Err::m_nOK;
}



ErrVal MbDecoder::calcMv( MbDataAccess& rcMbDataAccess,
                          MbDataAccess* pcMbDataAccessBaseMotion )
{
  if( rcMbDataAccess.getMbData().getBLSkipFlag() )
  {
    return Err::m_nOK;
  }
  else if( rcMbDataAccess.getMbData().getBLQRefFlag() )
  {
    for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
    {
      for( UInt uiFwdBwd = 1; uiFwdBwd < 3; uiFwdBwd++ )
      {
        if( ( rcMbDataAccess.getMbData().getBlockFwdBwd( c8x8Idx.b8x8Index() ) & uiFwdBwd ) != 0 )
        {
          Mv cMv = rcMbDataAccess.getMbMotionData( ListIdx(uiFwdBwd-1) ).getMv( c8x8Idx.b8x8() );
// TMM_ESS {
          // mv enforced to have 1/2 pel accuracy
          if ( cMv.getHor()%2 != 0 )
            cMv.setHor( 2*(Int)(cMv.getHor()/2) );
          if ( cMv.getVer()%2 != 0 )
            cMv.setVer( 2*(Int)(cMv.getVer()/2) );
// TMM_ESS }
          cMv   += rcMbDataAccess.getMbMvdData   ( ListIdx(uiFwdBwd-1) ).getMv( c8x8Idx.b8x8() );
          rcMbDataAccess.getMbMotionData( ListIdx(uiFwdBwd-1) ).setAllMv( cMv,  c8x8Idx.b8x8() );
        }
      }
    }
    return Err::m_nOK;
  }

  if( rcMbDataAccess.getMbData().getMbMode() == INTRA_4X4 )
  {
    //----- intra prediction -----
    rcMbDataAccess.getMbMotionData( LIST_0 ).setRefIdx( BLOCK_NOT_PREDICTED );
    rcMbDataAccess.getMbMotionData( LIST_0 ).setAllMv ( Mv::ZeroMv() );
    rcMbDataAccess.getMbMotionData( LIST_1 ).setRefIdx( BLOCK_NOT_PREDICTED );
    rcMbDataAccess.getMbMotionData( LIST_1 ).setAllMv ( Mv::ZeroMv() );
  }
  else
  {
    if( rcMbDataAccess.getMbData().getMbMode() == MODE_8x8 || rcMbDataAccess.getMbData().getMbMode() == MODE_8x8ref0 )
    {
      for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
      {
        //----- motion compensated prediction -----
        RNOK( m_pcMotionCompensation->calcMvSubMb( c8x8Idx, rcMbDataAccess, pcMbDataAccessBaseMotion ) );
      }
    }
    else
    {
      //----- motion compensated prediction -----
      RNOK( m_pcMotionCompensation->calcMvMb( rcMbDataAccess, pcMbDataAccessBaseMotion ) );
    }
  }

  return Err::m_nOK;
}




ErrVal MbDecoder::process( MbDataAccess& rcMbDataAccess,
                           Bool          bReconstructAll )
{
  ROF( m_bInitDone );

  RNOK( xScaleTCoeffs( rcMbDataAccess ) );

  YuvPicBuffer *pcRecYuvBuffer;
  RNOK( m_pcFrameMng->getRecYuvBuffer( pcRecYuvBuffer ) );

  IntYuvMbBuffer  cPredIntYuvMbBuffer;
  IntYuvMbBuffer  cResIntYuvMbBuffer;
  YuvMbBuffer     cYuvMbBuffer;

  if( rcMbDataAccess.getMbData().isPCM() )
  {
    RNOK( xDecodeMbPCM( rcMbDataAccess, cYuvMbBuffer ) );
    cResIntYuvMbBuffer .setAllSamplesToZero();
    cPredIntYuvMbBuffer.loadBuffer( &cYuvMbBuffer );
  }
  else
  {
    if( rcMbDataAccess.getMbData().isIntra() )
    {
      m_pcIntraPrediction->setAvailableMaskMb( rcMbDataAccess.getAvailableMask() );
      cResIntYuvMbBuffer.loadIntraPredictors( pcRecYuvBuffer );
  
      if( rcMbDataAccess.getMbData().isIntra4x4() )
      {
        if( rcMbDataAccess.getMbData().isTransformSize8x8() )
        {
          RNOK( xDecodeMbIntra8x8( rcMbDataAccess, cResIntYuvMbBuffer, cPredIntYuvMbBuffer ) );
        }
        else
        {
          RNOK( xDecodeMbIntra4x4( rcMbDataAccess, cResIntYuvMbBuffer, cPredIntYuvMbBuffer ) );
        }
      }
      else
      {
        RNOK( xDecodeMbIntra16x16( rcMbDataAccess, cResIntYuvMbBuffer, cPredIntYuvMbBuffer ) );
      }
      cYuvMbBuffer.loadBuffer( &cResIntYuvMbBuffer );
    }
    else
    {
      RNOK( xDecodeMbInter( rcMbDataAccess, cYuvMbBuffer, cPredIntYuvMbBuffer, cResIntYuvMbBuffer, bReconstructAll ) );
    }
  }

  pcRecYuvBuffer->loadBuffer( &cYuvMbBuffer );
  RNOK( m_pcFrameMng->getCurrentFrameUnit()->getResidual()->getFullPelYuvBuffer()->loadBuffer( &cResIntYuvMbBuffer ) );
  RNOK( m_pcFrameMng->getCurrentFrameUnit()->getFGSIntFrame()->getFullPelYuvBuffer()->loadBuffer( &cPredIntYuvMbBuffer ) );

  return Err::m_nOK;
  
}


ErrVal MbDecoder::xDecodeMbPCM( MbDataAccess& rcMbDataAccess, YuvMbBuffer& rcRecYuvBuffer )
{
  Pel* pucSrc  = rcMbDataAccess.getMbTCoeffs().getPelBuffer();
  Pel* pucDest = rcRecYuvBuffer.getMbLumAddr();
  Int  iStride = rcRecYuvBuffer.getLStride();
  Int n;

  for( n = 0; n < 16; n++ )
  {
    ::memcpy( pucDest, pucSrc, 16 );
    pucSrc  += 16;
    pucDest += iStride;
  }

  pucDest = rcRecYuvBuffer.getMbCbAddr();
  iStride = rcRecYuvBuffer.getCStride();

  for( n = 0; n < 8; n++ )
  {
    ::memcpy( pucDest, pucSrc, 8 );
    pucSrc  += 8;
    pucDest += iStride;
  }

  pucDest = rcRecYuvBuffer.getMbCrAddr();

  for( n = 0; n < 8; n++ )
  {
    ::memcpy( pucDest, pucSrc, 8 );
    pucSrc  += 8;
    pucDest += iStride;
  }

  return Err::m_nOK;
}



ErrVal MbDecoder::xDecodeMbPCM( MbDataAccess& rcMbDataAccess, IntYuvPicBuffer *pcRecYuvBuffer )
{
  Pel*  pucSrc  = rcMbDataAccess.getMbTCoeffs().getPelBuffer();
  XPel* pucDest = pcRecYuvBuffer->getMbLumAddr();
  Int   iStride = pcRecYuvBuffer->getLStride();
  UInt  uiDelta = 1;
  Int   n, m, n1, m1, dest;

  for( n = 0; n < 16; n+=uiDelta )
  {
    for( m = 0; m < 16; m+=uiDelta )
    {
      dest = *pucSrc++;

      for( n1=0; n1< +(Int)uiDelta; n1++ )
      for( m1=m; m1<m+(Int)uiDelta; m1++ )
      {
        pucDest[m1+n1*iStride] = dest;
      }
    }
    pucDest += uiDelta*iStride;
  }

  pucDest = pcRecYuvBuffer->getMbCbAddr();
  iStride = pcRecYuvBuffer->getCStride();

  for( n = 0; n < 8; n+=uiDelta )
  {
    for( m = 0; m < 8; m+=uiDelta )
    {
      dest = *pucSrc++;

      for( n1=0; n1< +(Int)uiDelta; n1++ )
      for( m1=m; m1<m+(Int)uiDelta; m1++ )
      {
        pucDest[m1+n1*iStride] = dest;
      }
    }
    pucDest += uiDelta*iStride;
  }

  pucDest = pcRecYuvBuffer->getMbCrAddr();

  for( n = 0; n < 8; n+=uiDelta )
  {
    for( m = 0; m < 8; m+=uiDelta )
    {
      dest = *pucSrc++;

      for( n1=0; n1< +(Int)uiDelta; n1++ )
      for( m1=m; m1<m+(Int)uiDelta; m1++ )
      {
        pucDest[m1+n1*iStride] = dest;
      }
    }
    pucDest += uiDelta*iStride;
  }


  return Err::m_nOK;
}




ErrVal MbDecoder::xDecodeMbInter( MbDataAccess&   rcMbDataAccess,
                                  YuvMbBuffer&    rcRecYuvBuffer,
                                  IntYuvMbBuffer& rcPredIntYuvMbBuffer,
                                  IntYuvMbBuffer& rcResIntYuvMbBuffer,
                                  Bool            bReconstruct )
{
  rcResIntYuvMbBuffer.setAllSamplesToZero();

  if( ! bReconstruct )
  {
    RNOK( m_pcMotionCompensation->calculateMb ( rcMbDataAccess, &rcRecYuvBuffer, true ) );
    rcRecYuvBuffer.setZero();
  }
  else
  {
    RNOK( m_pcMotionCompensation->compensateMb( rcMbDataAccess, &rcRecYuvBuffer, true ) );
  }

  rcPredIntYuvMbBuffer.loadBuffer( &rcRecYuvBuffer );
    
  MbTransformCoeffs& rcCoeffs = m_cTCoeffs;

  m_pcTransform->setClipMode( false );
  
  if( rcMbDataAccess.getMbData().isTransformSize8x8() )
  {
    for( B8x8Idx cIdx8x8; cIdx8x8.isLegal(); cIdx8x8++ )
    {
      if( rcMbDataAccess.getMbData().is4x4BlkCoded( cIdx8x8 ) )
      {
        RNOK( m_pcTransform->invTransform8x8Blk( rcResIntYuvMbBuffer.getYBlk( cIdx8x8 ),
                                                 rcResIntYuvMbBuffer.getLStride(),
                                                 rcCoeffs.get8x8(cIdx8x8) ) );
      }
    }
  }
  else
  {
    for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
    {
      if( rcMbDataAccess.getMbData().is4x4BlkCoded( cIdx ) )
      {
        RNOK( m_pcTransform->invTransform4x4Blk( rcResIntYuvMbBuffer.getYBlk( cIdx ),
                                                 rcResIntYuvMbBuffer.getLStride(),
                                                 rcCoeffs.get(cIdx) ) );
      }
    }
  }

  UInt uiChromaCbp = rcMbDataAccess.getMbData().getCbpChroma4x4();
  IntYuvMbBuffer cPredBuffer;
  RNOK( xDecodeChroma( rcMbDataAccess, rcResIntYuvMbBuffer, cPredBuffer, uiChromaCbp, false ) );
  m_pcTransform->setClipMode( true );

 
  IntYuvMbBuffer  cIntYuvMbBuffer;
  cIntYuvMbBuffer. loadLuma      ( rcPredIntYuvMbBuffer );
  cIntYuvMbBuffer. loadChroma    ( rcPredIntYuvMbBuffer );
  cIntYuvMbBuffer. add           ( rcResIntYuvMbBuffer );
  rcRecYuvBuffer.  loadBufferClip( &cIntYuvMbBuffer );

  return Err::m_nOK;
}


ErrVal MbDecoder::xDecodeMbInter( MbDataAccess&     rcMbDataAccess,
                                  MbDataAccess*     pcMbDataAccessBase,
                                  IntYuvMbBuffer&   rcPredBuffer,
                                  IntYuvPicBuffer*  pcRecYuvBuffer,
                                  IntFrame*         pcResidual,
                                  IntFrame*         pcBaseResidual,
                                  RefFrameList&     rcRefFrameList0,
                                  RefFrameList&     rcRefFrameList1,
                                  Bool              bReconstruct )
{
  IntYuvMbBuffer      cYuvMbBuffer;         cYuvMbBuffer        .setAllSamplesToZero();
  IntYuvMbBuffer      cYuvMbBufferResidual; cYuvMbBufferResidual.setAllSamplesToZero();
  MbTransformCoeffs&  rcCoeffs        = m_cTCoeffs;
  Bool                bCalcMv         = false;
  Bool                bFaultTolerant  = true;

  //===== derive motion vectors =====
  calcMv( rcMbDataAccess, pcMbDataAccessBase );

  //===== get prediction signal when full reconstruction is requested =====
  if( bReconstruct )
  {
    if( rcMbDataAccess.getMbData().getMbMode() == MODE_8x8 || rcMbDataAccess.getMbData().getMbMode() == MODE_8x8ref0 )
    {
      for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
      {
        //----- motion compensated prediction -----
        RNOK( m_pcMotionCompensation->compensateSubMb ( c8x8Idx,
                                                        rcMbDataAccess, rcRefFrameList0, rcRefFrameList1,
                                                        &cYuvMbBuffer, bCalcMv, bFaultTolerant ) );
      }
    }
    else
    {
      //----- motion compensated prediction -----
      RNOK(   m_pcMotionCompensation->compensateMb    ( rcMbDataAccess, rcRefFrameList0, rcRefFrameList1,
                                                        &cYuvMbBuffer, bCalcMv, bFaultTolerant ) );
    }

    rcPredBuffer.loadLuma   ( cYuvMbBuffer );
    rcPredBuffer.loadChroma ( cYuvMbBuffer );
  }


  //===== reconstruct residual signal by using transform coefficients ======
  m_pcTransform->setClipMode( false );
  if( rcMbDataAccess.getMbData().isTransformSize8x8() )
  {
    for( B8x8Idx cIdx; cIdx.isLegal(); cIdx++ )
    {
      if( rcMbDataAccess.getMbData().is4x4BlkCoded( cIdx ) )
      {
        RNOK( m_pcTransform->invTransform8x8Blk( cYuvMbBufferResidual.getYBlk( cIdx ),
                                                 cYuvMbBufferResidual.getLStride(),
                                                 rcCoeffs.get8x8(cIdx) ) );
      }
    }
  }
  else
  {
    for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
    {
      if( rcMbDataAccess.getMbData().is4x4BlkCoded( cIdx ) )
      {
        RNOK( m_pcTransform->invTransform4x4Blk( cYuvMbBufferResidual.getYBlk( cIdx ),
                                                 cYuvMbBufferResidual.getLStride(),
                                                 rcCoeffs.get(cIdx) ) );
      }
    }
  }

  UInt uiChromaCbp = rcMbDataAccess.getMbData().getCbpChroma4x4();
  RNOK( xDecodeChroma( rcMbDataAccess, cYuvMbBufferResidual, rcPredBuffer, uiChromaCbp, false ) );
  m_pcTransform->setClipMode( true );


  //===== add base layer residual =====
  if( rcMbDataAccess.getMbData().getResidualPredFlag( PART_16x16 ) )
  {
    IntYuvMbBuffer cBaseLayerBuffer;
    cBaseLayerBuffer.loadBuffer( pcBaseResidual->getFullPelYuvBuffer() );
    cYuvMbBufferResidual.add( cBaseLayerBuffer );
    //--- set CBP ---
    rcMbDataAccess.getMbData().setMbExtCbp( rcMbDataAccess.getMbData().getMbExtCbp() | pcMbDataAccessBase->getMbData().getMbExtCbp() );
  }


  //===== reconstruct signal =====
  if( bReconstruct )
  {
    cYuvMbBuffer.add( cYuvMbBufferResidual );
    cYuvMbBuffer.clip();
  }


  //===== store reconstructed residual =====
  if( pcResidual )
  {
    RNOK( pcResidual->getFullPelYuvBuffer()->loadBuffer( &cYuvMbBufferResidual ) );
  }

  //===== store reconstructed signal =====
  if( pcRecYuvBuffer )
  {
    RNOK( pcRecYuvBuffer->loadBuffer( &cYuvMbBuffer ) );
  }

  return Err::m_nOK;
}



ErrVal MbDecoder::xDecodeChroma( MbDataAccess& rcMbDataAccess, YuvMbBuffer& rcRecYuvBuffer, UInt uiChromaCbp, Bool bPredChroma )
{
  MbTransformCoeffs& rcCoeffs = m_cTCoeffs;

  Pel*  pucCb   = rcRecYuvBuffer.getMbCbAddr();
  Pel*  pucCr   = rcRecYuvBuffer.getMbCrAddr();
  Int   iStride = rcRecYuvBuffer.getCStride();

  if( bPredChroma )
  {
    RNOK( m_pcIntraPrediction->predictChromaBlock( pucCb, pucCr, iStride, rcMbDataAccess.getMbData().getChromaPredMode() ) );
  }

  ROTRS( 0 == uiChromaCbp, Err::m_nOK );

  Int       iShift, iScale;
  Quantizer cQuantizer;
  cQuantizer.setQp( rcMbDataAccess, false );

  const QpParameter&  cCQp      = cQuantizer.getChromaQp();
  Bool                bIntra    = rcMbDataAccess.getMbData().isIntra();
  UInt                uiUScalId = ( bIntra ? 1 : 4 );
  UInt                uiVScalId = ( bIntra ? 2 : 5 );
  const UChar*        pucScaleU = rcMbDataAccess.getSH().getScalingMatrix( uiUScalId );
  const UChar*        pucScaleV = rcMbDataAccess.getSH().getScalingMatrix( uiVScalId );

  // scaling has already been performed on DC coefficients
  /* HS: old scaling modified:
     (It did not work for scaling matrices, when QpPer became less than 5 in an FGS enhancement) */
  iScale = ( pucScaleU ? pucScaleU[0] : 1 );
  iShift = ( pucScaleU ? 5 : 1 );
  m_pcTransform->invTransformChromaDc( rcCoeffs.get( CIdx(0) ), iScale, iShift );     
  iScale = ( pucScaleV ? pucScaleV[0] : 1 );
  iShift = ( pucScaleV ? 5 : 1 );
  m_pcTransform->invTransformChromaDc( rcCoeffs.get( CIdx(4) ), iScale, iShift );

  RNOK( m_pcTransform->invTransformChromaBlocks( pucCb, iStride, rcCoeffs.get( CIdx(0) ) ) );
  RNOK( m_pcTransform->invTransformChromaBlocks( pucCr, iStride, rcCoeffs.get( CIdx(4) ) ) );

  return Err::m_nOK;
}



ErrVal MbDecoder::xDecodeMbIntra4x4( MbDataAccess& rcMbDataAccess, 
                                     IntYuvMbBuffer& cYuvMbBuffer,
                                     IntYuvMbBuffer&  rcPredBuffer )
{
  Int  iStride = cYuvMbBuffer.getLStride();

  MbTransformCoeffs& rcCoeffs = m_cTCoeffs;

  for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
  {
    XPel* puc = cYuvMbBuffer.getYBlk( cIdx );

    UInt uiPredMode = rcMbDataAccess.getMbData().intraPredMode( cIdx );
    RNOK( m_pcIntraPrediction->predictLumaBlock( puc, iStride, uiPredMode, cIdx ) );

    rcPredBuffer.loadLuma( cYuvMbBuffer, cIdx );

    if( rcMbDataAccess.getMbData().is4x4BlkCoded( cIdx ) )
    {
      RNOK( m_pcTransform->invTransform4x4Blk( puc, iStride, rcCoeffs.get( cIdx ) ) );
    }
  }

  UInt uiChromaCbp = rcMbDataAccess.getMbData().getCbpChroma4x4();
  RNOK( xDecodeChroma( rcMbDataAccess, cYuvMbBuffer, rcPredBuffer, uiChromaCbp, true ) );

  return Err::m_nOK;
}




ErrVal MbDecoder::xDecodeMbIntra8x8( MbDataAccess&   rcMbDataAccess,
                                     IntYuvMbBuffer& cYuvMbBuffer,
                                     IntYuvMbBuffer& rcPredBuffer )
{
  Int  iStride = cYuvMbBuffer.getLStride();

  MbTransformCoeffs& rcCoeffs = m_cTCoeffs;

  for( B8x8Idx cIdx; cIdx.isLegal(); cIdx++ )
  {
    XPel* puc = cYuvMbBuffer.getYBlk( cIdx );

    const UInt uiPredMode = rcMbDataAccess.getMbData().intraPredMode( cIdx );

    RNOK( m_pcIntraPrediction->predictLuma8x8Block( puc, iStride, uiPredMode, cIdx ) );

    rcPredBuffer.loadLuma( cYuvMbBuffer, cIdx );

    if( rcMbDataAccess.getMbData().is4x4BlkCoded( cIdx ) )
    {
      RNOK( m_pcTransform->invTransform8x8Blk( puc, iStride, rcCoeffs.get8x8( cIdx ) ) );
    }

  }

  UInt uiChromaCbp = rcMbDataAccess.getMbData().getCbpChroma4x4();
  RNOK( xDecodeChroma( rcMbDataAccess, cYuvMbBuffer, rcPredBuffer, uiChromaCbp, true ) );

  return Err::m_nOK;
}



ErrVal MbDecoder::xDecodeMbIntraBL( MbDataAccess&     rcMbDataAccess,
                                    IntYuvPicBuffer*  pcRecYuvBuffer,
                                    IntYuvMbBuffer&   rcPredBuffer,
                                    IntYuvPicBuffer*  pcBaseYuvBuffer )
{
  IntYuvMbBuffer      cYuvMbBuffer;
  MbTransformCoeffs&  rcCoeffs = m_cTCoeffs;
  Int                 iStride  = cYuvMbBuffer.getLStride();

  cYuvMbBuffer.loadBuffer ( pcBaseYuvBuffer );
  rcPredBuffer.loadLuma   ( cYuvMbBuffer );
  rcPredBuffer.loadChroma ( cYuvMbBuffer );

  if( rcMbDataAccess.getMbData().isTransformSize8x8() )
  {
    for( B8x8Idx cIdx; cIdx.isLegal(); cIdx++ )
    {
      if( rcMbDataAccess.getMbData().is4x4BlkCoded( cIdx ) )
      {
        RNOK( m_pcTransform->invTransform8x8Blk( cYuvMbBuffer.getYBlk( cIdx ),
                                                 cYuvMbBuffer.getLStride(),
                                                 rcCoeffs.get8x8(cIdx) ) );
      }
    }
  }
  else
  {
    for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
    {
      if( rcMbDataAccess.getMbData().is4x4BlkCoded( cIdx ) )
      {
        RNOK( m_pcTransform->invTransform4x4Blk( cYuvMbBuffer.getYBlk( cIdx ),
                                                 cYuvMbBuffer.getLStride(),
                                                 rcCoeffs.get(cIdx) ) );
      }
    }
  }

  UInt uiChromaCbp = rcMbDataAccess.getMbData().getCbpChroma4x4();
  RNOK( xDecodeChroma( rcMbDataAccess, cYuvMbBuffer, rcPredBuffer, uiChromaCbp, false ) );

  pcRecYuvBuffer->loadBuffer( &cYuvMbBuffer );

  return Err::m_nOK;
}



ErrVal MbDecoder::xDecodeMbIntra16x16( MbDataAccess&    rcMbDataAccess,
                                       IntYuvMbBuffer&  cYuvMbBuffer,
                                       IntYuvMbBuffer& rcPredBuffer )
{
  Int  iStride = cYuvMbBuffer.getLStride();

  RNOK( m_pcIntraPrediction->predictLumaMb( cYuvMbBuffer.getMbLumAddr(), iStride, rcMbDataAccess.getMbData().intraPredMode() ) );

  rcPredBuffer.loadLuma( cYuvMbBuffer );

  MbTransformCoeffs& rcCoeffs = m_cTCoeffs;

  Quantizer           cQuantizer;   cQuantizer.setQp    ( rcMbDataAccess, false );
  const QpParameter   rcLQp       = cQuantizer.getLumaQp();
  Int                 iScaleY     = g_aaiDequantCoef[rcLQp.rem()][0] << rcLQp.per();
  const UChar*        pucScaleY   = rcMbDataAccess.getSH().getScalingMatrix( 0 );

  if( pucScaleY )
  {
    iScaleY  *= pucScaleY[0];
    iScaleY >>= 4;
  }

  RNOK( m_pcTransform->invTransformDcCoeff( rcCoeffs.get( B4x4Idx(0) ), iScaleY ) );

  for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
  {
    RNOK( m_pcTransform->invTransform4x4Blk( cYuvMbBuffer.getYBlk( cIdx ), iStride, rcCoeffs.get( cIdx ) ) );
  }

  UInt uiChromaCbp = rcMbDataAccess.getMbData().getCbpChroma16x16();
  RNOK( xDecodeChroma( rcMbDataAccess, cYuvMbBuffer, rcPredBuffer, uiChromaCbp, true ) );

  return Err::m_nOK;
}


ErrVal MbDecoder::xDecodeChroma( MbDataAccess&    rcMbDataAccess,
                                 IntYuvMbBuffer&  rcRecYuvBuffer,
                                 IntYuvMbBuffer&  rcPredMbBuffer,
                                 UInt             uiChromaCbp,
                                 Bool             bPredChroma )
{
  MbTransformCoeffs& rcCoeffs = m_cTCoeffs;

  XPel* pucCb   = rcRecYuvBuffer.getMbCbAddr();
  XPel* pucCr   = rcRecYuvBuffer.getMbCrAddr();
  Int   iStride = rcRecYuvBuffer.getCStride();

  if( bPredChroma )
  {
    RNOK( m_pcIntraPrediction->predictChromaBlock( pucCb, pucCr, iStride, rcMbDataAccess.getMbData().getChromaPredMode() ) );
    rcPredMbBuffer.loadChroma( rcRecYuvBuffer );
  }

  ROTRS( 0 == uiChromaCbp, Err::m_nOK );
  
  Int       iShift, iScale;
  Quantizer cQuantizer;
  cQuantizer.setQp( rcMbDataAccess, false );

  const QpParameter&  cCQp      = cQuantizer.getChromaQp();
  Bool                bIntra    = rcMbDataAccess.getMbData().isIntra();
  UInt                uiUScalId = ( bIntra ? 1 : 4 );
  UInt                uiVScalId = ( bIntra ? 2 : 5 );
  const UChar*        pucScaleU = rcMbDataAccess.getSH().getScalingMatrix( uiUScalId );
  const UChar*        pucScaleV = rcMbDataAccess.getSH().getScalingMatrix( uiVScalId );

  // scaling has already been performed on DC coefficients
  /* HS: old scaling modified:
     (It did not work for scaling matrices, when QpPer became less than 5 in an FGS enhancement) */
  iScale = ( pucScaleU ? pucScaleU[0] : 1 );
  iShift = ( pucScaleU ? 5 : 1 );
  m_pcTransform->invTransformChromaDc( rcCoeffs.get( CIdx(0) ), iScale, iShift );     
  iScale = ( pucScaleV ? pucScaleV[0] : 1 );
  iShift = ( pucScaleV ? 5 : 1 );
  m_pcTransform->invTransformChromaDc( rcCoeffs.get( CIdx(4) ), iScale, iShift );

  RNOK( m_pcTransform->invTransformChromaBlocks( pucCb, iStride, rcCoeffs.get( CIdx(0) ) ) );
  RNOK( m_pcTransform->invTransformChromaBlocks( pucCr, iStride, rcCoeffs.get( CIdx(4) ) ) );

  return Err::m_nOK;
}




ErrVal
MbDecoder::xScale4x4Block( TCoeff*            piCoeff,
                           const UChar*       pucScale,
                           UInt               uiStart,
                           const QpParameter& rcQP )
{
  if( pucScale )
  {
    Int iAdd = ( rcQP.per() <= 3 ? ( 1 << ( 3 - rcQP.per() ) ) : 0 );
    
    for( UInt ui = uiStart; ui < 16; ui++ )
    {
      piCoeff[ui] = ( ( piCoeff[ui] * g_aaiDequantCoef[rcQP.rem()][ui] * pucScale[ui] + iAdd ) << rcQP.per() ) >> 4;
    }
  }
  else
  {
    for( UInt ui = uiStart; ui < 16; ui++ )
    {
      piCoeff[ui] *= ( g_aaiDequantCoef[rcQP.rem()][ui] << rcQP.per() );
    }
  }

  return Err::m_nOK;
}


ErrVal
MbDecoder::xScale8x8Block( TCoeff*            piCoeff,
                           const UChar*       pucScale,
                           const QpParameter& rcQP )
{
  Int iAdd = ( rcQP.per() <= 5 ? ( 1 << ( 5 - rcQP.per() ) ) : 0 );

  if( pucScale )
  {
    for( UInt ui = 0; ui < 64; ui++ )
    {
      piCoeff[ui] = ( ( piCoeff[ui] * g_aaiDequantCoef64[rcQP.rem()][ui] * pucScale[ui] + iAdd ) << rcQP.per() ) >> 6;
    }
  }
  else
  {
    for( UInt ui = 0; ui < 64; ui++ )
    {
      piCoeff[ui] = ( ( piCoeff[ui] * g_aaiDequantCoef64[rcQP.rem()][ui] * 16 + iAdd ) << rcQP.per() ) >> 6;
    }
  }

  return Err::m_nOK;
}


ErrVal
MbDecoder::xScaleTCoeffs( MbDataAccess& rcMbDataAccess )
{
  const Int aaiDequantDcCoef[6] = {  10, 11, 13, 14, 16, 18 };
  Quantizer cQuantizer;
  cQuantizer.setQp( rcMbDataAccess, false );
  
  const QpParameter&  cLQp      = cQuantizer.getLumaQp  ();
  const QpParameter&  cCQp      = cQuantizer.getChromaQp();
  Bool                bIntra    = rcMbDataAccess.getMbData().isIntra();
  Bool                b8x8      = rcMbDataAccess.getMbData().isTransformSize8x8();
  Bool                b16x16    = rcMbDataAccess.getMbData().isIntra16x16();
  UInt                uiYScalId = ( bIntra ? ( b8x8 && !b16x16 ? 6 : 0 ) : ( b8x8 ? 7 : 3 ) );
  UInt                uiUScalId = ( bIntra ? 1 : 4 );
  UInt                uiVScalId = ( bIntra ? 2 : 5 );
  const UChar*        pucScaleY = rcMbDataAccess.getSH().getScalingMatrix( uiYScalId );
  const UChar*        pucScaleU = rcMbDataAccess.getSH().getScalingMatrix( uiUScalId );
  const UChar*        pucScaleV = rcMbDataAccess.getSH().getScalingMatrix( uiVScalId );

  //===== copy all coefficients =====
  MbTransformCoeffs& rcTCoeffs = m_cTCoeffs;
  rcTCoeffs.copyFrom( rcMbDataAccess.getMbTCoeffs() );

  //===== luma =====
  if( b16x16 )
  {
    //===== INTRA_16x16 =====
    for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
    {
      RNOK( xScale4x4Block( rcTCoeffs.get( cIdx ), pucScaleY, 1, cLQp ) );
    }
  }
  else if( b8x8 )
  {
    //===== 8x8 BLOCKS =====
    for( B8x8Idx cIdx; cIdx.isLegal(); cIdx++ )
    {
      RNOK( xScale8x8Block( rcTCoeffs.get8x8( cIdx ), pucScaleY, cLQp ) );
    }
  }
  else
  {
    //===== 4x4 BLOCKS =====
    for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
    {
      RNOK( xScale4x4Block( rcTCoeffs.get( cIdx ), pucScaleY, 0, cLQp ) );
    }
  }

  //===== chroma =====
  Int iScaleU  = g_aaiDequantCoef[cCQp.rem()][0] << cCQp.per();
  Int iScaleV  = g_aaiDequantCoef[cCQp.rem()][0] << cCQp.per();
  /* HS: old scaling modified:
     (I did not work for scaling matrices, when QpPer became less than 5 in an FGS enhancement) */
  for( CIdx cIdx; cIdx.isLegal(); cIdx++ )
  {
    RNOK( xScale4x4Block( rcTCoeffs.get( cIdx ), ( cIdx.plane() ? pucScaleV : pucScaleU ), 1, cCQp ) );
  }
  UInt    uiDCIdx;
  TCoeff* piCoeff = rcTCoeffs.get( CIdx(0) );
  for( uiDCIdx = 0; uiDCIdx < 4; uiDCIdx++ )
  {
    piCoeff[16*uiDCIdx] *= iScaleU;
  }
  piCoeff = rcTCoeffs.get( CIdx(4) );
  for( uiDCIdx = 0; uiDCIdx < 4; uiDCIdx++ )
  {
    piCoeff[16*uiDCIdx] *= iScaleV;
  }


  return Err::m_nOK;
}



H264AVC_NAMESPACE_END
