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





#include "H264AVCCommonLib.h"

#include "H264AVCCommonLib/SampleWeighting.h"
#include  "H264AVCCommonLib/YuvMbBuffer.h"


H264AVC_NAMESPACE_BEGIN


SampleWeighting::SampleWeighting()
: m_uiLumaLogWeightDenom    ( 5 )
, m_uiChromaLogWeightDenom  ( 5 )
, m_bExplicit               ( false )
, m_bWeightedPredDisableP   ( true )
, m_bWeightedPredDisableB   ( true )
{
  m_afpMixSampleFunc[0] = NULL;
  m_afpMixSampleFunc[1] = NULL;
  m_afpMixSampleFunc[2] = NULL;
  m_afpMixSampleFunc[3] = NULL;
  m_afpMixSampleFunc[4] = NULL;

  m_afpXMixSampleFunc[0] = NULL;
  m_afpXMixSampleFunc[1] = NULL;
  m_afpXMixSampleFunc[2] = NULL;
  m_afpXMixSampleFunc[3] = NULL;
  m_afpXMixSampleFunc[4] = NULL;
}


ErrVal SampleWeighting::create( SampleWeighting*& rpcSampleWeighting )
{
  rpcSampleWeighting = new SampleWeighting;

  ROT( NULL == rpcSampleWeighting );

  return Err::m_nOK;
}


ErrVal SampleWeighting::destroy()
{
  delete this;

  return Err::m_nOK;
}

ErrVal SampleWeighting::init()
{
  m_afpMixSampleFunc[1] = SampleWeighting::xMixB4x;
  m_afpMixSampleFunc[2] = SampleWeighting::xMixB8x;
  m_afpMixSampleFunc[4] = SampleWeighting::xMixB16x;

  m_afpXMixSampleFunc[1] = SampleWeighting::xXMixB4x;
  m_afpXMixSampleFunc[2] = SampleWeighting::xXMixB8x;
  m_afpXMixSampleFunc[4] = SampleWeighting::xXMixB16x;
  return Err::m_nOK;
}


ErrVal SampleWeighting::uninit()
{
  m_afpMixSampleFunc[0] = NULL;
  m_afpMixSampleFunc[1] = NULL;
  m_afpMixSampleFunc[2] = NULL;
  m_afpMixSampleFunc[3] = NULL;
  m_afpMixSampleFunc[4] = NULL;

  m_afpXMixSampleFunc[0] = NULL;
  m_afpXMixSampleFunc[1] = NULL;
  m_afpXMixSampleFunc[2] = NULL;
  m_afpXMixSampleFunc[3] = NULL;
  m_afpXMixSampleFunc[4] = NULL;

  return Err::m_nOK;
}


ErrVal
SampleWeighting::initSlice( const SliceHeader& rcSliceHeader )
{
  if( rcSliceHeader.isIntra() )
  {
    m_bWeightedPredDisableP = true;
    m_bWeightedPredDisableB = true;
    m_bExplicit             = false;
    return Err::m_nOK;
  }
  if( rcSliceHeader.isInterP() )
  {
    m_bExplicit             = rcSliceHeader.getPPS().getWeightedPredFlag();
    m_bWeightedPredDisableP = ! m_bExplicit;
    m_bWeightedPredDisableB = true;
    if( m_bExplicit )
    {
      m_uiLumaLogWeightDenom   = rcSliceHeader.getLumaLog2WeightDenom();
      m_uiChromaLogWeightDenom = rcSliceHeader.getChromaLog2WeightDenom();
    }
    return Err::m_nOK;
  }
  if( rcSliceHeader.isInterB() )
  {
    switch( rcSliceHeader.getPPS().getWeightedBiPredIdc() )
    {
    case 0:
      {
        m_bExplicit               = false;
        m_bWeightedPredDisableP   = true;
        m_bWeightedPredDisableB   = true;
        m_uiLumaLogWeightDenom    = 0;
        m_uiChromaLogWeightDenom  = 0;
      }
      break;
    case 1:
      {
        m_bExplicit               = true;
        m_bWeightedPredDisableP   = false;
        m_bWeightedPredDisableB   = false;
        m_uiLumaLogWeightDenom    = rcSliceHeader.getLumaLog2WeightDenom();
        m_uiChromaLogWeightDenom  = rcSliceHeader.getChromaLog2WeightDenom();
      }
      break;
    case 2:
      {
        m_bExplicit               = false;
        m_bWeightedPredDisableP   = true;
        m_bWeightedPredDisableB   = false;
        m_uiLumaLogWeightDenom    = 5;
        m_uiChromaLogWeightDenom  = 5;
      }
      break;
    default:
      {
        AF();
      }
      break;
    }
  }
  return Err::m_nOK;
}


Void SampleWeighting::getTargetBuffers( YuvMbBuffer* apcTarBuffer[2], YuvMbBuffer* pcRecBuffer, const PW* pcPW0, const PW* pcPW1 )
{
  if( pcPW0 != 0 && pcPW1 != 0 )
  {
    apcTarBuffer[0] = pcRecBuffer;
    apcTarBuffer[1] = &m_cYuvBiBuffer;
  }
  else
  {
    apcTarBuffer[0] = pcRecBuffer;
    apcTarBuffer[1] = pcRecBuffer;
  }
}

Void SampleWeighting::getTargetBuffers( IntYuvMbBuffer* apcTarBuffer[2], IntYuvMbBuffer* pcRecBuffer, const PW* pcPW0, const PW* pcPW1 )
{
  if( pcPW0 != 0 && pcPW1 != 0 )
  {
    apcTarBuffer[0] = pcRecBuffer;
    apcTarBuffer[1] = &m_cIntYuvBiBuffer;
  }
  else
  {
    apcTarBuffer[0] = pcRecBuffer;
    apcTarBuffer[1] = pcRecBuffer;
  }
}


Void SampleWeighting::weightLumaSamples( YuvMbBuffer* pcRecBuffer, Int iSizeX, Int iSizeY, LumaIdx cIdx, const PW* pcPW0, const PW* pcPW1 )
{
  AOT_DBG( iSizeY < 8 );
  AOT_DBG( iSizeX < 8 );

  if( pcPW0 != NULL && pcPW1 != NULL )
  {
    // bidirectional prediction
    if( m_bWeightedPredDisableB )
    {
      xMixB( pcRecBuffer  ->getYBlk( cIdx ),  pcRecBuffer  ->getLStride(),
             m_cYuvBiBuffer.getYBlk( cIdx ),  m_cYuvBiBuffer.getLStride(),
             iSizeY, iSizeX );
    }
    else
    {
      xMixBWeight( pcRecBuffer  ->getYBlk( cIdx ), pcRecBuffer  ->getLStride(),
                   m_cYuvBiBuffer.getYBlk( cIdx ), m_cYuvBiBuffer.getLStride(),
                   iSizeY, iSizeX,
                   pcPW0->getLumaWeight(),
                   pcPW1->getLumaWeight(),
                   pcPW0->getLumaOffset() + pcPW1->getLumaOffset(),
                   m_uiLumaLogWeightDenom );
    }
  }
  else
  {
    ROTVS( m_bWeightedPredDisableP );
   
    // unidirectionl prediction
    const PW* pcPredWeight = (pcPW0 != NULL) ? pcPW0 : pcPW1;
    AOT_DBG( NULL == pcPredWeight );

    if( pcPredWeight->getLumaWeightFlag() )
    {
      xWeight( pcRecBuffer->getYBlk( cIdx ), pcRecBuffer->getLStride(),
               iSizeY, iSizeX,
               pcPredWeight->getLumaWeight(),
               pcPredWeight->getLumaOffset(),
               m_uiLumaLogWeightDenom );
    }
  }
}


Void SampleWeighting::weightLumaSamples( IntYuvMbBuffer* pcRecBuffer, Int iSizeX, Int iSizeY, LumaIdx cIdx, const PW* pcPW0, const PW* pcPW1 )
{
  AOT_DBG( iSizeY < 8 );
  AOT_DBG( iSizeX < 8 );

  if( pcPW0 != NULL && pcPW1 != NULL )
  {
    // bidirectional prediction
    if( m_bWeightedPredDisableB )
    {
      xMixB( pcRecBuffer     ->getYBlk( cIdx ),  pcRecBuffer     ->getLStride(),
             m_cIntYuvBiBuffer.getYBlk( cIdx ),  m_cIntYuvBiBuffer.getLStride(),
             iSizeY, iSizeX );
    }
    else
  {
      xMixBWeight( pcRecBuffer     ->getYBlk( cIdx ), pcRecBuffer     ->getLStride(),
                   m_cIntYuvBiBuffer.getYBlk( cIdx ), m_cIntYuvBiBuffer.getLStride(),
                   iSizeY, iSizeX,
                   pcPW0->getLumaWeight(),
                   pcPW1->getLumaWeight(),
                   pcPW0->getLumaOffset() + pcPW1->getLumaOffset(),
                   m_uiLumaLogWeightDenom );
  }
}
  else
  {
    ROTVS( m_bWeightedPredDisableP );

    // unidirectionl prediction
    const PW* pcPredWeight = (pcPW0 != NULL) ? pcPW0 : pcPW1;
    AOT_DBG( NULL == pcPredWeight );

    if( pcPredWeight->getLumaWeightFlag() )
    {
      xWeight( pcRecBuffer->getYBlk( cIdx ), pcRecBuffer->getLStride(),
               iSizeY, iSizeX,
               pcPredWeight->getLumaWeight(),
               pcPredWeight->getLumaOffset(),
               m_uiLumaLogWeightDenom );
    }
  }
}


Void SampleWeighting::weightChromaSamples( YuvMbBuffer* pcRecBuffer, Int iSizeX, Int iSizeY, LumaIdx cIdx, const PW* pcPW0, const PW* pcPW1 )
{
  AOT_DBG( iSizeY < 4 );
  AOT_DBG( iSizeX < 4 );

  if( pcPW0 != NULL && pcPW1 != NULL )
  {
    // bidirectional prediction
    if( m_bWeightedPredDisableB )
    {
      xMixB( pcRecBuffer  ->getUBlk( cIdx ), pcRecBuffer  ->getCStride(),
             m_cYuvBiBuffer.getUBlk( cIdx ), m_cYuvBiBuffer.getCStride(),
             iSizeY, iSizeX );
      xMixB( pcRecBuffer  ->getVBlk( cIdx ), pcRecBuffer  ->getCStride(),
             m_cYuvBiBuffer.getVBlk( cIdx ), m_cYuvBiBuffer.getCStride(),
             iSizeY, iSizeX );
    }
    else
  {
      xMixBWeight( pcRecBuffer  ->getUBlk( cIdx ), pcRecBuffer  ->getCStride(),
                   m_cYuvBiBuffer.getUBlk( cIdx ), m_cYuvBiBuffer.getCStride(),
                   iSizeY, iSizeX,
                   pcPW0->getChromaWeight( 0 ),
                   pcPW1->getChromaWeight( 0 ),
                   pcPW0->getChromaOffset( 0 ) + pcPW1->getChromaOffset( 0 ),
                   m_uiChromaLogWeightDenom );
      xMixBWeight( pcRecBuffer  ->getVBlk( cIdx ), pcRecBuffer  ->getCStride(),
                   m_cYuvBiBuffer.getVBlk( cIdx ), m_cYuvBiBuffer.getCStride(),
                   iSizeY, iSizeX,
                   pcPW0->getChromaWeight( 1 ),
                   pcPW1->getChromaWeight( 1 ),
                   pcPW0->getChromaOffset( 1 ) + pcPW1->getChromaOffset( 1 ),
                   m_uiChromaLogWeightDenom );
    }
  }
  else
  {
    ROTVS( m_bWeightedPredDisableP );

    // unidirectionl prediction
    const PW* pcPredWeight = (pcPW0 != NULL) ? pcPW0 : pcPW1;
    AOT_DBG( NULL == pcPredWeight );

    if( pcPredWeight->getChromaWeightFlag() )
    {
      xWeight( pcRecBuffer->getUBlk( cIdx ), pcRecBuffer->getCStride(),
               iSizeY, iSizeX,
               pcPredWeight->getChromaWeight( 0 ),
               pcPredWeight->getChromaOffset( 0 ),
               m_uiChromaLogWeightDenom );
      xWeight( pcRecBuffer->getVBlk( cIdx ), pcRecBuffer->getCStride(),
               iSizeY, iSizeX,
               pcPredWeight->getChromaWeight( 1 ),
               pcPredWeight->getChromaOffset( 1 ),
               m_uiChromaLogWeightDenom );
    }
  }
}



Void SampleWeighting::weightChromaSamples( IntYuvMbBuffer* pcRecBuffer, Int iSizeX, Int iSizeY, LumaIdx cIdx, const PW* pcPW0, const PW* pcPW1 )
{
  AOT_DBG( iSizeY < 4 );
  AOT_DBG( iSizeX < 4 );

  if( pcPW0 != NULL && pcPW1 != NULL )
  {
    // bidirectional prediction
    if( m_bWeightedPredDisableB )
    {
      xMixB( pcRecBuffer     ->getUBlk( cIdx ), pcRecBuffer     ->getCStride(),
             m_cIntYuvBiBuffer.getUBlk( cIdx ), m_cIntYuvBiBuffer.getCStride(),
             iSizeY, iSizeX );
      xMixB( pcRecBuffer     ->getVBlk( cIdx ), pcRecBuffer     ->getCStride(),
             m_cIntYuvBiBuffer.getVBlk( cIdx ), m_cIntYuvBiBuffer.getCStride(),
             iSizeY, iSizeX );
  }
    else
    {
      xMixBWeight( pcRecBuffer     ->getUBlk( cIdx ), pcRecBuffer     ->getCStride(),
                   m_cIntYuvBiBuffer.getUBlk( cIdx ), m_cIntYuvBiBuffer.getCStride(),
                   iSizeY, iSizeX,
                   pcPW0->getChromaWeight( 0 ),
                   pcPW1->getChromaWeight( 0 ),
                   pcPW0->getChromaOffset( 0 ) + pcPW1->getChromaOffset( 0 ),
                   m_uiChromaLogWeightDenom );
      xMixBWeight( pcRecBuffer     ->getVBlk( cIdx ), pcRecBuffer     ->getCStride(),
                   m_cIntYuvBiBuffer.getVBlk( cIdx ), m_cIntYuvBiBuffer.getCStride(),
                   iSizeY, iSizeX,
                   pcPW0->getChromaWeight( 1 ),
                   pcPW1->getChromaWeight( 1 ),
                   pcPW0->getChromaOffset( 1 ) + pcPW1->getChromaOffset( 1 ),
                   m_uiChromaLogWeightDenom );
}
  }
  else
  {
    ROTVS( m_bWeightedPredDisableP );

    // unidirectionl prediction
    const PW* pcPredWeight = (pcPW0 != NULL) ? pcPW0 : pcPW1;
    AOT_DBG( NULL == pcPredWeight );

    if( pcPredWeight->getChromaWeightFlag() )
    {
      xWeight( pcRecBuffer->getUBlk( cIdx ), pcRecBuffer->getCStride(),
               iSizeY, iSizeX,
               pcPredWeight->getChromaWeight( 0 ),
               pcPredWeight->getChromaOffset( 0 ),
               m_uiChromaLogWeightDenom );
      xWeight( pcRecBuffer->getVBlk( cIdx ), pcRecBuffer->getCStride(),
               iSizeY, iSizeX,
               pcPredWeight->getChromaWeight( 1 ),
               pcPredWeight->getChromaOffset( 1 ),
               m_uiChromaLogWeightDenom );
    }
  }
}




Void SampleWeighting::inverseLumaSamples( IntYuvMbBuffer* pcDesBuffer,
                                          IntYuvMbBuffer* pcOrgBuffer,
                                          IntYuvMbBuffer* pcFixBuffer,
                                          Int             iYSize,
                                          Int             iXSize )
{
  XPel* pFix         = pcFixBuffer->getLumBlk();
  XPel* pOrg         = pcOrgBuffer->getLumBlk();
  XPel* pDes         = pcDesBuffer->getLumBlk();
  const Int iStride  = pcDesBuffer->getLStride();

  Int iLine = 0;
  for( Int y = 0; y < iYSize; y++)
  {
    for( Int x = 0; x < iXSize; x++)
    {
      pDes[x+iLine] = gClip((Int)(2*pOrg[x+iLine] - pFix[x+iLine])) ;
    }
    iLine += iStride;
  }
}



Void SampleWeighting::xMixB16x( Pel* pucDest, Int iDestStride, Pel* pucSrc, Int iSrcStride, Int iSizeY )
  {
  for( Int y = 0; y < iSizeY; y++)
    {
    for( Int x = 0; x < 16; x++)
      {
      pucDest[x] = (pucDest[x] + pucSrc[x] + 1) >> 1;
      }
    pucDest += iDestStride;
    pucSrc  += iSrcStride;
    }
  }

Void SampleWeighting::xMixB8x( Pel* pucDest, Int iDestStride, Pel* pucSrc, Int iSrcStride, Int iSizeY)
    {
  for( Int y = 0; y < iSizeY; y++)
      {
    for( Int x = 0; x < 8; x++)
    {
      pucDest[x] = (pucDest[x] + pucSrc[x] + 1) >> 1;
    }
    pucDest += iDestStride;
    pucSrc  += iSrcStride;
  }
}


Void SampleWeighting::xMixB4x( Pel* pucDest, Int iDestStride, Pel* pucSrc, Int iSrcStride, Int iSizeY )
{
  for( Int y = 0; y < iSizeY; y++)
  {
    for( Int x = 0; x < 4; x++)
    {
      pucDest[x] = (pucDest[x] + pucSrc[x] + 1) >> 1;
    }
    pucDest += iDestStride;
    pucSrc  += iSrcStride;
  }
}


Void SampleWeighting::xWeight( Pel* pucDest, Int iDestStride, Int iSizeY, Int iSizeX, Int iWeight, Int iOffset, UInt uiDenom )
{
  Int iAdd = ((1+iOffset*2)<<uiDenom)>>1;

  AOT_DBG( iWeight >  128 );
  AOT_DBG( iWeight < -128 );

  for( Int y = 0; y < iSizeY; y++)
  {
    for( Int x = 0; x < iSizeX; x++)
    {
      Int iTemp  = (( iWeight * pucDest[x] + iAdd) >> uiDenom);
      pucDest[x] = gClip( iTemp );
    }
    pucDest += iDestStride;
  }
}


Void SampleWeighting::xMixBWeight( Pel* pucDest, Int iDestStride, Pel* pucSrc, Int iSrcStride, Int iSizeY, Int iSizeX, Int iWD, Int iWS, Int iOffset, UInt uiDenom )
{
  Int iAdd = (1<<uiDenom);

  AOT_DBG( (iWD + iWS) > ((uiDenom == 7) ? 127 : 128));
  AOT_DBG( iWD + iWS < -128 );

  uiDenom++;
  iOffset = (iOffset+1) >> 1;
  for( Int y = 0; y < iSizeY; y++)
  {
    for( Int x = 0; x < iSizeX; x++)
    {
      Int iTemp = (( iWD * pucDest[x] + iWS * pucSrc[x] + iAdd) >> uiDenom) + iOffset;
      pucDest[x] = gClip( iTemp );
    }
    pucDest += iDestStride;
    pucSrc  += iSrcStride;
  }
}



__inline Void SampleWeighting::xMixB(Pel* pucDest, Int iDestStride, Pel* pucSrc, Int iSrcStride, Int iSizeY, Int iSizeX)
{
  m_afpMixSampleFunc[iSizeX>>2]( pucDest, iDestStride, pucSrc, iSrcStride, iSizeY );
}




Void SampleWeighting::xXMixB16x( XPel* pucDest, Int iDestStride, XPel* pucSrc, Int iSrcStride, Int iSizeY )
{
  for( Int y = 0; y < iSizeY; y++)
  {
    for( Int x = 0; x < 16; x++)
    {
      pucDest[x] = (pucDest[x] + pucSrc[x] + 1) >> 1;
    }
    pucDest += iDestStride;
    pucSrc  += iSrcStride;
  }
}

Void SampleWeighting::xXMixB8x( XPel* pucDest, Int iDestStride, XPel* pucSrc, Int iSrcStride, Int iSizeY)
{
  for( Int y = 0; y < iSizeY; y++)
  {
    for( Int x = 0; x < 8; x++)
    {
      pucDest[x] = (pucDest[x] + pucSrc[x] + 1) >> 1;
    }
    pucDest += iDestStride;
    pucSrc  += iSrcStride;
  }
}


Void SampleWeighting::xXMixB4x( XPel* pucDest, Int iDestStride, XPel* pucSrc, Int iSrcStride, Int iSizeY )
{
  for( Int y = 0; y < iSizeY; y++)
  {
    for( Int x = 0; x < 4; x++)
    {
      pucDest[x] = (pucDest[x] + pucSrc[x] + 1) >> 1;
    }
    pucDest += iDestStride;
    pucSrc  += iSrcStride;
  }
}

Void SampleWeighting::xWeight( XPel* pucDest, Int iDestStride, Int iSizeY, Int iSizeX, Int iWeight, Int iOffset, UInt uiDenom )
{
  Int iAdd = ((1+iOffset*2)<<uiDenom)>>1;

  AOT_DBG( iWeight >  128 );
  AOT_DBG( iWeight < -128 );

  for( Int y = 0; y < iSizeY; y++)
  {
    for( Int x = 0; x < iSizeX; x++)
    {
      Int iTemp  = (( iWeight * pucDest[x] + iAdd) >> uiDenom);
      pucDest[x] = gClip( iTemp );
    }
    pucDest += iDestStride;
  }
}


Void SampleWeighting::xMixBWeight( XPel* pucDest, Int iDestStride, XPel* pucSrc, Int iSrcStride, Int iSizeY, Int iSizeX, Int iWD, Int iWS, Int iOffset, UInt uiDenom )
{
  Int iAdd = (1<<uiDenom);

  AOT_DBG( (iWD + iWS) > ((uiDenom == 7) ? 127 : 128));
  AOT_DBG( iWD + iWS < -128 );

  uiDenom++;
  iOffset = (iOffset+1) >> 1;
  for( Int y = 0; y < iSizeY; y++)
  {
    for( Int x = 0; x < iSizeX; x++)
    {
      Int iTemp = (( iWD * pucDest[x] + iWS * pucSrc[x] + iAdd) >> uiDenom) + iOffset;
      pucDest[x] = gClip( iTemp );
    }
    pucDest += iDestStride;
    pucSrc  += iSrcStride;
  }
}


__inline Void SampleWeighting::xMixB(XPel* pucDest, Int iDestStride, XPel* pucSrc, Int iSrcStride, Int iSizeY, Int iSizeX)
{
  m_afpXMixSampleFunc[iSizeX>>2]( pucDest, iDestStride, pucSrc, iSrcStride, iSizeY );
}


Void
SampleWeighting::weightInverseLumaSamples( IntYuvMbBuffer*  pcDesBuffer,
                                           IntYuvMbBuffer*  pcOrgBuffer,
                                           IntYuvMbBuffer*  pcFixBuffer,
                                           const PW*        pcSearchPW,
                                           const PW*        pcFixPW,
                                           Double&          rdWeight,
                                           Int              iYSize,
                                           Int              iXSize )
{
  XPel* pFix        = pcFixBuffer->getLumBlk();
  XPel* pOrg        = pcOrgBuffer->getLumBlk();
  XPel* pDes        = pcDesBuffer->getLumBlk();
  Int   iStride     = pcDesBuffer->getLStride();

  Int   iFixWeight  = pcFixPW   ->getLumaWeight();
  Int   iWeight     = pcSearchPW->getLumaWeight();
  Int   iOffset     = ( pcFixPW ->getLumaOffset() + pcSearchPW->getLumaOffset() + 1 ) >> 1;
  Int   iLWD        = m_uiLumaLogWeightDenom;
  Int   iAdd        = ( 1 << iLWD );

  AOT_DBG( m_bExplicit && iWeight >  128 );
  AOT_DBG( m_bExplicit && iWeight < -128 );

  if( iWeight == 0 ) // doesn't make sense to transmit a motion vector for that case
  {
    for( Int iLine = 0, y = 0; y < iYSize; y++, iLine += iStride )
    for( Int            x = 0; x < iXSize; x++ )
    {
      pDes[x+iLine] = 128;
    }
    rdWeight        = 1.0;
  }
  else
  {
    Int iShift      = ( iLWD + 8 );
    Int iInvWeight  = ( 1    << iShift ) / iWeight;
    Int iInvAdd     = ( iAdd << iShift ) / iWeight;

    for( Int iLine = 0, y = 0; y < iYSize; y++, iLine += iStride )
    for( Int            x = 0; x < iXSize; x++ )
    {
      Int iTemp     = ( iInvWeight * ( ( ( pOrg[x+iLine] - iOffset ) << (iLWD+1) ) - iFixWeight * pFix[x+iLine] ) - iInvAdd ) >> iShift;
      pDes[x+iLine] = gClip( iTemp );
    }
    rdWeight = 128.0 / abs(iInvWeight);
  }
}




Void
SampleWeighting::weightInverseLumaSamples( IntYuvMbBuffer* pcDesBuffer,
                                           IntYuvMbBuffer* pcOrgBuffer,
                                           const PW*       pcPW,
                                           Double&         rdWeight,
                                           Int             iYSize,
                                           Int             iXSize )
{
  XPel* pOrg      = pcOrgBuffer->getLumBlk();
  XPel* pDes      = pcDesBuffer->getLumBlk();
  Int   iStride   = pcDesBuffer->getLStride();

  Int   iWeight   = pcPW->getLumaWeight();
  Int   iOffset   = pcPW->getLumaOffset();
  Int   iAdd      = ( 1 << m_uiLumaLogWeightDenom ) >> 1;

  AOT_DBG( iWeight >  127 );
  AOT_DBG( iWeight < -128 );

  if( iWeight == 0 ) // motion vector doesn't make sense
  {
    for( Int y = 0; y < iYSize; y++, pDes+=iStride )
    for( Int x = 0; x < iXSize; x++)
    {
      pDes[x]       = 128;
    }
    rdWeight        = 1.0;
  }
  else if( ! pcPW->getLumaWeightFlag() )
  {
    //===== unweighted copy =====
    for( Int y = 0; y < iYSize; y++, pDes+=iStride )
    for( Int x = 0; x < iXSize; x++)
    {
      pDes[x]       = pOrg[x];
    }
    rdWeight        = 1.0;
  }
  else
  {
    Int iInvWeight  = ( 1    << ( m_uiLumaLogWeightDenom + 8 ) ) / iWeight;
    Int iInvAdd     = ( iAdd <<                            8   ) / iWeight;

    for( Int y = 0; y < iYSize; y++, pDes+=iStride, pOrg+=iStride )
    for( Int x = 0; x < iXSize; x++)
    {
      Int iTemp     = ( iInvWeight * ( pOrg[x] - iOffset ) - iInvAdd ) >> 8;
      pDes[x]       = gClip( iTemp );
    }
    rdWeight        = 256.0 / abs(iInvWeight);
  }
}



Void
SampleWeighting::weightInverseChromaSamples( IntYuvMbBuffer* pcDesBuffer,
                                             IntYuvMbBuffer* pcOrgBuffer,
                                             const PW*       pcPW,
                                             Double*         padWeight,
                                             Int             iYSize,
                                             Int             iXSize )
{
  iYSize >>= 1;
  iXSize >>= 1;

  for( Int C = 0; C < 2; C++ )
  {
    XPel* pOrg      = ( C ? pcOrgBuffer->getCrBlk() : pcOrgBuffer->getCbBlk() );
    XPel* pDes      = ( C ? pcDesBuffer->getCrBlk() : pcDesBuffer->getCbBlk() );
    Int   iStride   = pcDesBuffer->getCStride();

    Int   iWeight   = pcPW->getChromaWeight( C );
    Int   iOffset   = pcPW->getChromaOffset( C );
    Int   iAdd      = ( 1 << m_uiChromaLogWeightDenom ) >> 1;

    AOT_DBG( iWeight >  127 );
    AOT_DBG( iWeight < -128 );

    if( iWeight == 0 ) // motion vector doesn't make sense
    {
      for( Int y = 0; y < iYSize; y++, pDes+=iStride )
      for( Int x = 0; x < iXSize; x++)
      {
        pDes[x]       = 128;
      }
      padWeight[C]    = 1.0;
    }
    else if( ! pcPW->getChromaWeightFlag() )
    {
      //===== unweighted copy =====
      for( Int y = 0; y < iYSize; y++, pDes+=iStride )
      for( Int x = 0; x < iXSize; x++)
      {
        pDes[x]       = pOrg[x];
      }
      padWeight[C]    = 1.0;
    }
    else
    {
      Int iInvWeight  = ( 1    << ( m_uiChromaLogWeightDenom + 8 ) ) / iWeight;
      Int iInvAdd     = ( iAdd <<                              8   ) / iWeight;

      for( Int y = 0; y < iYSize; y++, pDes+=iStride, pOrg+=iStride )
      for( Int x = 0; x < iXSize; x++)
      {
        Int iTemp     = ( iInvWeight * ( pOrg[x] - iOffset ) - iInvAdd ) >> 8;
        pDes[x]       = gClip( iTemp );
      }
      padWeight[C]    = 256.0 / abs(iInvWeight);
    }
  }
}


//TMM_WP
ErrVal SampleWeighting::initSliceForWeighting( const SliceHeader& rcSliceHeader)
{
  if( rcSliceHeader.isIntra() )
  {
    m_bWeightedPredDisableP = true;
    m_bWeightedPredDisableB = true;
    m_bExplicit             = false;
    return Err::m_nOK;
  }

  if( rcSliceHeader.isInterP() )
  {
    m_bExplicit             = rcSliceHeader.getPPS().getWeightedPredFlag();
    m_bWeightedPredDisableP = ! m_bExplicit;
    m_bWeightedPredDisableB = true;

    if( m_bExplicit )
    {
      m_uiLumaLogWeightDenom   = rcSliceHeader.getLumaLog2WeightDenom();
      m_uiChromaLogWeightDenom = rcSliceHeader.getChromaLog2WeightDenom();
    }

    return Err::m_nOK;
  }


  else if( rcSliceHeader.isInterB() )
  {
    switch( rcSliceHeader.getPPS().getWeightedBiPredIdc() )
    {
    case 0:
      {
        m_bExplicit               = false;
        m_bWeightedPredDisableP   = true;
        m_bWeightedPredDisableB   = true;
        m_uiLumaLogWeightDenom    = 0;
        m_uiChromaLogWeightDenom  = 0;
      }
      break;
    case 1:
      {
        m_bExplicit               = true;
        m_bWeightedPredDisableP   = false;
        m_bWeightedPredDisableB   = false;
        m_uiLumaLogWeightDenom    = rcSliceHeader.getLumaLog2WeightDenom();
        m_uiChromaLogWeightDenom  = rcSliceHeader.getChromaLog2WeightDenom();
      }
      break;
    case 2:
      {
        m_bExplicit               = false;
        m_bWeightedPredDisableP   = true;
        m_bWeightedPredDisableB   = false;
        m_uiLumaLogWeightDenom    = 5;
        m_uiChromaLogWeightDenom  = 5;
      }
      break;
    default:
      {
        AF();
      }
      break;
    }
  }

  return Err::m_nOK;
}

//TMM_WP

H264AVC_NAMESPACE_END

