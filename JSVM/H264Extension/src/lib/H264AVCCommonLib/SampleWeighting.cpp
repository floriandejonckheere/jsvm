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


Void SampleWeighting::getTargetBuffers( YuvMbBuffer* apcTarBuffer[2], YuvMbBuffer* pcRecBuffer, Bool bBi )
{
  if( bBi )
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

Void SampleWeighting::getTargetBuffers( IntYuvMbBuffer* apcTarBuffer[2], IntYuvMbBuffer* pcRecBuffer, Bool bBi )
{
  if( bBi )
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


Void SampleWeighting::weightLumaSamples( YuvMbBuffer* pcRecBuffer, Int iSizeX, Int iSizeY, LumaIdx cIdx, Bool bBi )
{
  AOT_DBG( iSizeY < 8 );
  AOT_DBG( iSizeX < 8 );

  if( bBi )
  {
    xMixB(  pcRecBuffer->getYBlk( cIdx ),
            pcRecBuffer->getLStride(),
            m_cYuvBiBuffer.getYBlk( cIdx ),
            m_cYuvBiBuffer.getLStride(),
            iSizeY,
            iSizeX );
  }
}


Void SampleWeighting::weightLumaSamples( IntYuvMbBuffer* pcRecBuffer, Int iSizeX, Int iSizeY, LumaIdx cIdx, Bool bBi )
{
  AOT_DBG( iSizeY < 8 );
  AOT_DBG( iSizeX < 8 );

  if( bBi )
  {
    xMixB(  pcRecBuffer->getYBlk( cIdx ),
            pcRecBuffer->getLStride(),
            m_cIntYuvBiBuffer.getYBlk( cIdx ),
            m_cIntYuvBiBuffer.getLStride(),
            iSizeY,
            iSizeX );
  }
}




Void SampleWeighting::weightChromaSamples( YuvMbBuffer* pcRecBuffer, Int iSizeX, Int iSizeY, LumaIdx cIdx, Bool bBi )
{
  AOT_DBG( iSizeY < 4 );
  AOT_DBG( iSizeX < 4 );

  if( bBi )
  {
    Pel* pURec      = pcRecBuffer->getUBlk( cIdx );
    Pel* pVRec      = pcRecBuffer->getVBlk( cIdx );
    Int iStrideRec  = pcRecBuffer->getCStride();

    xMixB(  pURec, iStrideRec,
            m_cYuvBiBuffer.getUBlk( cIdx ),
            m_cYuvBiBuffer.getCStride(),
            iSizeY,
            iSizeX );

    xMixB(  pVRec, iStrideRec,
            m_cYuvBiBuffer.getVBlk( cIdx ),
            m_cYuvBiBuffer.getCStride(),
            iSizeY,
            iSizeX );
  }
}



Void SampleWeighting::weightChromaSamples( IntYuvMbBuffer* pcRecBuffer, Int iSizeX, Int iSizeY, LumaIdx cIdx, Bool bBi )
{
  AOT_DBG( iSizeY < 4 );
  AOT_DBG( iSizeX < 4 );

  if( bBi )
  {
    XPel* pURec       = pcRecBuffer->getUBlk( cIdx );
    XPel* pVRec       = pcRecBuffer->getVBlk( cIdx );
    Int   iStrideRec  = pcRecBuffer->getCStride();

    xMixB(  pURec, iStrideRec,
            m_cIntYuvBiBuffer.getUBlk( cIdx ),
            m_cIntYuvBiBuffer.getCStride(),
            iSizeY,
            iSizeX );

    xMixB(  pVRec, iStrideRec,
            m_cIntYuvBiBuffer.getVBlk( cIdx ),
            m_cIntYuvBiBuffer.getCStride(),
            iSizeY,
            iSizeX );
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



Void SampleWeighting::inverseChromaSamples( IntYuvMbBuffer* pcDesBuffer,
                                            IntYuvMbBuffer* pcOrgBuffer,
                                            IntYuvMbBuffer* pcFixBuffer,
                                            Int             iYSize,
                                            Int             iXSize )
{
  iYSize >>= 1;
  iXSize >>= 1;

  {
    XPel* pFix         = pcFixBuffer->getCbBlk();
    XPel* pOrg         = pcOrgBuffer->getCbBlk();
    XPel* pDes         = pcDesBuffer->getCbBlk();
    const Int iStride  = pcDesBuffer->getCStride();

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
  {
    XPel* pFix         = pcFixBuffer->getCrBlk();
    XPel* pOrg         = pcOrgBuffer->getCrBlk();
    XPel* pDes         = pcDesBuffer->getCrBlk();
    const Int iStride  = pcDesBuffer->getCStride();

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


__inline Void SampleWeighting::xMixB(XPel* pucDest, Int iDestStride, XPel* pucSrc, Int iSrcStride, Int iSizeY, Int iSizeX)
{
  m_afpXMixSampleFunc[iSizeX>>2]( pucDest, iDestStride, pucSrc, iSrcStride, iSizeY );
}


H264AVC_NAMESPACE_END

