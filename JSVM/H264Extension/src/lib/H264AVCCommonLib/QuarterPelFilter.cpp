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
#include "H264AVCCommonLib/QuarterPelFilter.h"

H264AVC_NAMESPACE_BEGIN


QuarterPelFilter::QuarterPelFilter()
: m_bClip ( true )
, m_bRCDO ( false )
{
  uninit();
}

QuarterPelFilter::~QuarterPelFilter()
{
}

ErrVal QuarterPelFilter::create( QuarterPelFilter*& rpcQuarterPelFilter )
{
  QuarterPelFilter* pcQuarterPelFilter;

  pcQuarterPelFilter = new QuarterPelFilter;

  rpcQuarterPelFilter = pcQuarterPelFilter;

  ROT( NULL == rpcQuarterPelFilter );

  return Err::m_nOK;
}


ErrVal QuarterPelFilter::destroy()
{
  delete this;

  return Err::m_nOK;
}


ErrVal QuarterPelFilter::init()
{
  m_bClip = true;
  m_afpFilterBlockFunc[0] = QuarterPelFilter::xFilter1;
  m_afpFilterBlockFunc[1] = QuarterPelFilter::xFilter2;
  m_afpFilterBlockFunc[2] = QuarterPelFilter::xFilter3;
  m_afpFilterBlockFunc[3] = QuarterPelFilter::xFilter4;

  m_afpXFilterBlockFunc[0] = QuarterPelFilter::xXFilter1;
  m_afpXFilterBlockFunc[1] = QuarterPelFilter::xXFilter2;
  m_afpXFilterBlockFunc[2] = QuarterPelFilter::xXFilter3;
  m_afpXFilterBlockFunc[3] = QuarterPelFilter::xXFilter4;
  return Err::m_nOK;
}

ErrVal QuarterPelFilter::uninit()
{
  m_afpFilterBlockFunc[0] = NULL;
  m_afpFilterBlockFunc[1] = NULL;
  m_afpFilterBlockFunc[2] = NULL;
  m_afpFilterBlockFunc[3] = NULL;

  m_afpXFilterBlockFunc[0] = NULL;
  m_afpXFilterBlockFunc[1] = NULL;
  m_afpXFilterBlockFunc[2] = NULL;
  m_afpXFilterBlockFunc[3] = NULL;
  return Err::m_nOK;
}


const Int g_aiTapCoeff[6] = { 1, -5,20,20,-5, 1};


extern Int  giInterpolationType;


Void predBlkBilinear( YuvMbBuffer* pcDesBuffer, YuvPicBuffer* pcSrcBuffer, LumaIdx cIdx, Mv cMv, Int iSizeY, Int iSizeX )
{
  Pel* pucDes     = pcDesBuffer->getYBlk( cIdx );
  Pel* pucSrc     = pcSrcBuffer->getYBlk( cIdx );
  Int iDesStride  = pcDesBuffer->getLStride();
  Int iSrcStride  = pcSrcBuffer->getLStride();
  Int iOffset     = (cMv.getHor() >> 2) + (cMv.getVer() >> 2) * iSrcStride;

  pucSrc += iOffset;

  Int iDx = cMv.getHor() & 3;
  Int iDy = cMv.getVer() & 3;

  if( iDx == 0 && iDy == 0 )
  {
    for( Int y = 0; y < iSizeY; y++)
    {
      for( Int x = 0; x < iSizeX; x++ )
        pucDes[x] = pucSrc[x];

      pucDes += iDesStride;
      pucSrc += iSrcStride;
    }
  }
  else
  {
    // normal bilinear interpolation
    for( Int y = 0; y < iSizeY; y++)
    {
      for( Int x = 0; x < iSizeX; x++ )
      {
        Int iTemp;

        iTemp = 
          (pucSrc[x]              * (4 - iDx) + pucSrc[x + 1]              * iDx) * (4 - iDy) + 
          (pucSrc[iSrcStride + x] * (4 - iDx) + pucSrc[iSrcStride + x + 1] * iDx) * iDy;

        pucDes[x] = (iTemp >= 0) ? ( (iTemp + 8) >> 4 ) : -( (-iTemp + 8) >> 4 );
      }

      pucDes += iDesStride;
      pucSrc += iSrcStride;
    }
  }
}


Void QuarterPelFilter::predBlk( YuvMbBuffer* pcDesBuffer, YuvPicBuffer* pcSrcBuffer, LumaIdx cIdx, Mv cMv, Int iSizeY, Int iSizeX)
{
  if( m_bRCDO )
  {
    predBlkRCDO( pcDesBuffer, pcSrcBuffer, cIdx, cMv, iSizeY, iSizeX );
    return;
  }

  Pel* pucDes     = pcDesBuffer->getYBlk( cIdx );
  Pel* pucSrc     = pcSrcBuffer->getYBlk( cIdx );
  Int iDesStride  = pcDesBuffer->getLStride();
  Int iSrcStride  = pcSrcBuffer->getLStride();
  Int iOffset     = (cMv.getHor() >> 2) + (cMv.getVer() >> 2) * iSrcStride;

  pucSrc += iOffset;

  Int iDx = cMv.getHor() & 3;
  Int iDy = cMv.getVer() & 3;
  if( iDy == 0)
  {
    if( iDx == 0 )
    {
      iSizeX>>=2;
      for( Int y = 0; y < iSizeY; y++)
      {
        UInt* puiDes = (UInt*)pucDes;
        UInt* puiSrc = (UInt*)pucSrc;
        for( Int x = 0; x < iSizeX; x++ )
        {
          puiDes[x] = puiSrc[x];
        }
        pucDes += iDesStride;
        pucSrc += iSrcStride;
      }
      return;
    }


    if( iDx == 2 )
    {
      xPredDy0Dx2( pucDes, pucSrc, iDesStride, iSrcStride, iSizeY, iSizeX );
      return;
    }

    // if( iDx == 1 || iDx == 3)
    xPredDy0Dx13( pucDes, pucSrc, iDesStride, iSrcStride, iDx,  iSizeY, iSizeX );
    return;
  }


  if( iDx == 0)
  {
    if( iDy == 2 )
    {
      xPredDx0Dy2( pucDes, pucSrc, iDesStride, iSrcStride, iSizeY, iSizeX );
      return;
    }

    // if( iDx == 1 || iDx == 3)
    xPredDx0Dy13( pucDes, pucSrc, iDesStride, iSrcStride, iDx, iDy, iSizeY, iSizeX );
    return;
  }


  if( iDx == 2)
  {
    if( iDy == 2 )
    {
      xPredDx2Dy2( pucDes, pucSrc, iDesStride, iSrcStride, iDx, iDy, iSizeY, iSizeX );
      return;
    }

    // if( iDy == 1 || iDy == 3 )
    xPredDx2Dy13( pucDes, pucSrc, iDesStride, iSrcStride, iDx, iDy, iSizeY, iSizeX );
    return;
  }



  if( iDy == 2)
  {
    // if( iDx == 2 ) is already done
    // if( iDx == 1 || iDx == 3 )
    xPredDy2Dx13( pucDes, pucSrc, iDesStride, iSrcStride, iDx, iDy, iSizeY, iSizeX );
    return;
  }


  xPredElse( pucDes, pucSrc, iDesStride, iSrcStride, iDx, iDy, iSizeY, iSizeX );
  return;
}



Void QuarterPelFilter::xPredDy0Dx2( Pel* pucDest, Pel* pucSrc, Int iDestStride, Int iSrcStride, UInt uiSizeY, UInt uiSizeX )
{
  for( UInt y = 0; y < uiSizeY; y++)
  {
    for( UInt x = 0; x < uiSizeX; x++)
    {
      Int iTemp;
      iTemp  = pucSrc[x - 0];
      iTemp += pucSrc[x + 1];
      iTemp  = iTemp << 2;
      iTemp -= pucSrc[x - 1];
      iTemp -= pucSrc[x + 2];
      iTemp += iTemp << 2;
      iTemp += pucSrc[x - 2];
      iTemp += pucSrc[x + 3];
      pucDest[x] = gClip( (iTemp + 16) / 32 );
    }
    pucDest += iDestStride;
    pucSrc  += iSrcStride;
  }
}

Void QuarterPelFilter::xPredDy0Dx13( Pel* pucDest, Pel* pucSrc, Int iDestStride, Int iSrcStride, Int iDx, UInt uiSizeY, UInt uiSizeX )
{

  iDx >>= 1;
  for( UInt y = 0; y < uiSizeY; y++)
  {
    for( UInt x = 0; x < uiSizeX; x++)
    {
      Int iTemp;
      iTemp  = pucSrc[x - 0];
      iTemp += pucSrc[x + 1];
      iTemp  = iTemp << 2;
      iTemp -= pucSrc[x - 1];
      iTemp -= pucSrc[x + 2];
      iTemp += iTemp << 2;
      iTemp += pucSrc[x - 2];
      iTemp += pucSrc[x + 3];
      iTemp = gClip( (iTemp + 16) / 32 );
      pucDest[x] = (iTemp + pucSrc[ x + iDx] + 1) / 2;
    }
    pucDest += iDestStride;
    pucSrc  += iSrcStride;
  }
}


Void QuarterPelFilter::xPredDx0Dy2( Pel* pucDest, Pel* pucSrc, Int iDestStride, Int iSrcStride,  UInt uiSizeY, UInt uiSizeX )
{
  for( UInt y = 0; y < uiSizeY; y++)
  {
    for( UInt x = 0; x < uiSizeX; x++)
    {
      Int iTemp;
      iTemp  = pucSrc[x - 0*iSrcStride];
      iTemp += pucSrc[x + 1*iSrcStride];
      iTemp  = iTemp << 2;
      iTemp -= pucSrc[x - 1*iSrcStride];
      iTemp -= pucSrc[x + 2*iSrcStride];
      iTemp += iTemp << 2;
      iTemp += pucSrc[x - 2*iSrcStride];
      iTemp += pucSrc[x + 3*iSrcStride];
      pucDest[x] = gClip( (iTemp + 16) / 32 );
    }
    pucDest += iDestStride;
    pucSrc  += iSrcStride;
  }
}

Void QuarterPelFilter::xPredDx0Dy13( Pel* pucDest, Pel* pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX )
{
  iDy = (iDy>>1) * iSrcStride;
  for( UInt y = 0; y < uiSizeY; y++)
  {
    for( UInt x = 0; x < uiSizeX; x++)
    {
      Int iTemp;
      iTemp  = pucSrc[x - 0*iSrcStride];
      iTemp += pucSrc[x + 1*iSrcStride];
      iTemp  = iTemp << 2;
      iTemp -= pucSrc[x - 1*iSrcStride];
      iTemp -= pucSrc[x + 2*iSrcStride];
      iTemp += iTemp << 2;
      iTemp += pucSrc[x - 2*iSrcStride];
      iTemp += pucSrc[x + 3*iSrcStride];
      iTemp = gClip( (iTemp + 16) / 32 );
      pucDest[x] = (iTemp + pucSrc[ x + iDy] + 1)/2;
    }
    pucDest += iDestStride;
    pucSrc  += iSrcStride;
  }
}


Void QuarterPelFilter::xPredDx2( Short*  psDest, Pel*  pucSrc, Int iSrcStride, UInt uiSizeY, UInt uiSizeX )
{
  pucSrc -= 2*iSrcStride;

  for( UInt y = 0; y < uiSizeY + 5; y++)
  {
    for( UInt x = 0; x < uiSizeX; x++)
    {
      Pel* puc = pucSrc + x;
      Int iTemp;
      iTemp  = puc[0];
      iTemp += puc[1];
      iTemp  = iTemp << 2;
      iTemp -= puc[-1];
      iTemp -= puc[2];
      iTemp += iTemp << 2;
      iTemp += puc[-2];
      iTemp += puc[3];
      psDest[x] = iTemp;

    }
    psDest += 16;
    pucSrc += iSrcStride;
  }
}


Void QuarterPelFilter::xPredDx2Dy2( Pel* pucDest, Pel* pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX )
{
  Short asTemp[16*(16+6)];
  Short* psTemp = asTemp;
  xPredDx2( asTemp, pucSrc, iSrcStride, uiSizeY, uiSizeX );

  for( UInt y = 0; y < uiSizeY; y++)
  {
    for( UInt x = 0; x < uiSizeX; x++)
    {
      Int iIndex = x + 0x20;
      Int iTemp;
      iTemp  = psTemp[ 0x00 + iIndex];
      iTemp += psTemp[ 0x10 + iIndex];
      iTemp  = iTemp << 2;
      iTemp -= psTemp[-0x10 + iIndex];
      iTemp -= psTemp[ 0x20 + iIndex];
      iTemp += iTemp << 2;
      iTemp += psTemp[-0x20 + iIndex];
      iTemp += psTemp[ 0x30 + iIndex];

      pucDest[x] = gClip( (iTemp + 512) / 1024 );
    }
    psTemp  += 0x10;
    pucDest += iDestStride;
  }

}


Void QuarterPelFilter::xPredDx2Dy13( Pel* pucDest, Pel* pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX )
{
  Short asTemp[16*(16+6)];
  Short* psTemp = asTemp;
  xPredDx2( asTemp, pucSrc, iSrcStride, uiSizeY, uiSizeX );

  iDy = (iDy == 1) ? 0 : 0x10;

  for( UInt y = 0; y < uiSizeY; y++)
  {
    for( UInt x = 0; x < uiSizeX; x++)
    {
      Int iIndex = x + 0x20;
      Int iTemp;
      iTemp  = psTemp[ 0x00 + iIndex];
      iTemp += psTemp[ 0x10 + iIndex];
      iTemp  = iTemp << 2;
      iTemp -= psTemp[-0x10 + iIndex];
      iTemp -= psTemp[ 0x20 + iIndex];
      iTemp += iTemp << 2;
      iTemp += psTemp[-0x20 + iIndex];
      iTemp += psTemp[ 0x30 + iIndex];
      iTemp = gClip( (iTemp + 512) / 1024 );
      pucDest[x] = (iTemp + gClip( (psTemp[iDy + iIndex] + 16) / 32 ) + 1) / 2;
    }
    psTemp  += 0x10;
    pucDest += iDestStride;
  }
}


Void QuarterPelFilter::xPredDy2Dx13( Pel* pucDest, Pel* pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX )
{
  iDx = (iDx == 1) ? 2 : 3;
  for( UInt y = 0; y < uiSizeY; y++)
  {
    Int aiTemp[6+16];
    for( UInt n = 0; n < 6+uiSizeX; n++)
    {
      Pel* puc = pucSrc + n - 2;
      aiTemp[n]  = puc[ 0*iSrcStride];
      aiTemp[n] += puc[ 1*iSrcStride];
      aiTemp[n]  = aiTemp[n] << 2;
      aiTemp[n] -= puc[-1*iSrcStride];
      aiTemp[n] -= puc[ 2*iSrcStride];
      aiTemp[n] += aiTemp[n] << 2;
      aiTemp[n] += puc[-2*iSrcStride];
      aiTemp[n] += puc[ 3*iSrcStride];
    }

    for( UInt x = 0; x < uiSizeX; x++)
    {
      Int iTemp = 0;
      for( Int n = 0; n < 6; n++ )
      {
        iTemp += aiTemp[n+x]*g_aiTapCoeff[n];
      }
      iTemp = gClip( (iTemp + 512) / 1024 );
      pucDest[x] = (iTemp + gClip( (aiTemp[x+iDx] + 16) / 32 ) + 1) / 2;
    }
    pucDest += iDestStride;
    pucSrc  += iSrcStride;
  }
}



Void QuarterPelFilter::xPredElse( Pel* pucDest, Pel* pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX )
{
  Pel* pucSrcX = pucSrc;
  Pel* pucSrcY = pucSrc;

  pucSrcY += (iDx == 1) ? 0 : 1;
  pucSrcX += (iDy == 1) ? 0 : iSrcStride;

  for( UInt y = 0; y < uiSizeY; y++)
  {
    for( UInt x = 0; x < uiSizeX; x++)
    {
      Int iTempX;
      iTempX  = pucSrcX[x - 0];
      iTempX += pucSrcX[x + 1];
      iTempX  = iTempX << 2;
      iTempX -= pucSrcX[x - 1];
      iTempX -= pucSrcX[x + 2];
      iTempX += iTempX << 2;
      iTempX += pucSrcX[x - 2];
      iTempX += pucSrcX[x + 3];
      iTempX = gClip( (iTempX + 16) / 32 );

      Int iTempY;
      iTempY  = pucSrcY[x - 0*iSrcStride];
      iTempY += pucSrcY[x + 1*iSrcStride];
      iTempY  = iTempY << 2;
      iTempY -= pucSrcY[x - 1*iSrcStride];
      iTempY -= pucSrcY[x + 2*iSrcStride];
      iTempY += iTempY << 2;
      iTempY += pucSrcY[x - 2*iSrcStride];
      iTempY += pucSrcY[x + 3*iSrcStride];
      iTempY = gClip( (iTempY + 16) / 32 );

      pucDest[x] = (iTempX + iTempY + 1) >> 1;
    }
    pucDest += iDestStride;
    pucSrcX += iSrcStride;
    pucSrcY += iSrcStride;
  }
}

ErrVal QuarterPelFilter::filterFrame( YuvPicBuffer *pcPelBuffer, YuvPicBuffer *pcHalfPelBuffer )
{
  ROT( NULL == pcPelBuffer );
  ROT( NULL == pcHalfPelBuffer );

  Pel*    pucSrc      = pcPelBuffer->getMbLumAddr ();
  Int     iHeight     = pcPelBuffer->getLHeight   ();
  Int     iWidth      = pcPelBuffer->getLWidth    ();
  Int     iStride     = pcPelBuffer->getLStride   ();
  Int     iMargin     = pcPelBuffer->getLXMargin  ();
  UInt    uiTmpXSize  = (iMargin + iWidth + iMargin) * 2;
  UInt    uiTmpYSize  = iMargin + iHeight + iMargin;
  Int     iMarginNew  = iMargin-4;
  Int     x, y;

  XPel*  psTemp      = new XPel[uiTmpXSize * uiTmpYSize];
  ::memset( psTemp, 0x00, (uiTmpXSize * uiTmpYSize)*sizeof(XPel) );

  ROT( NULL == psTemp )

  XPel*  ps          = &psTemp[ iMargin * uiTmpXSize + 2*iMargin ];

  if( m_bRCDO )
  {
    for( y = 0; y < iHeight; y++ )
    {
      for( x = -iMarginNew; x < iWidth+iMarginNew; x++ )
      {
        Int iTemp;
        iTemp  = pucSrc[x - 0];
        iTemp += pucSrc[x + 1];
        iTemp  = iTemp << 2;
        iTemp -= pucSrc[x - 1];
        iTemp -= pucSrc[x + 2];
        iTemp += iTemp << 2;
        iTemp += pucSrc[x - 2];
        iTemp += pucSrc[x + 3];
        ps[2*x]    = pucSrc[x];
        ps[2*x+1]  = (XPel)xClip((iTemp + 6) >> 5);
      }

      ps     += uiTmpXSize;
      pucSrc += iStride;
    }
  }
  else
  {
    for( y = 0; y < iHeight; y++ )
    {
      for( x = -iMarginNew; x < iWidth+iMarginNew; x++ )
      {
        Int iTemp;
        iTemp  = pucSrc[x - 0];
        iTemp += pucSrc[x + 1];
        iTemp  = iTemp << 2;
        iTemp -= pucSrc[x - 1];
        iTemp -= pucSrc[x + 2];
        iTemp += iTemp << 2;
        iTemp += pucSrc[x - 2];
        iTemp += pucSrc[x + 3];
        ps[2*x]    = pucSrc[x]<<5;
        ps[2*x+1]  = iTemp;
      }

      ps     += uiTmpXSize;
      pucSrc += iStride;
    }
  }

  // bot
  ps -= iMargin*2;
  for( y = 0; y < iMargin; y++ )
  {
    ::memcpy( &ps[y*uiTmpXSize], &ps[(y-1)*uiTmpXSize], uiTmpXSize*sizeof(XPel) );
  }

  //top
  ps = &psTemp[ iMargin * uiTmpXSize ];
  for( y = 0; y < iMargin; y++ )
  {
    ::memcpy( &ps[-(y+1)*uiTmpXSize], &ps[-y*uiTmpXSize], uiTmpXSize*sizeof(XPel) );
  }

  ps = &psTemp[ 4*uiTmpXSize + 2*iMargin ];
  iStride = uiTmpXSize;

  Int   iDesStrideHP  = pcHalfPelBuffer->getLStride();
  Pel* pucDesHP       = pcHalfPelBuffer->getMbLumAddr();
  pucDesHP -= (iMarginNew*iDesStrideHP)<<1;

  if( m_bRCDO )
  {
    for( y = -iMarginNew; y < iHeight+iMarginNew; y++ )
    {
      for( x = -2*iMarginNew; x < 2*(iWidth+iMarginNew); x++ )
      {
        pucDesHP[x]                = (Pel)ps[x];
        if( ( x & 1 ) == 0 )
          pucDesHP[x+iDesStrideHP] = (Pel)xClip( ( (ps[x+iStride]+ps[x])*5 - ps[x+2*iStride] - ps[x-iStride] + 1 ) >> 3 );
        else
          pucDesHP[x+iDesStrideHP] = (Pel)( ps[x+iStride-1] + ps[x+1] ) >> 1;
      }
      pucDesHP += iDesStrideHP<<1;
      ps       += iStride;
    }
  }
  else
  {
    for( y = -iMarginNew; y < iHeight+iMarginNew; y++ )
    {
      for( x = -2*iMarginNew; x < 2*(iWidth+iMarginNew); x++ )
      {
        Int iTemp;
        iTemp  = ps[x - 0*iStride];
        iTemp += ps[x + 1*iStride];
        iTemp  = iTemp << 2;
        iTemp -= ps[x - 1*iStride];
        iTemp -= ps[x + 2*iStride];
        iTemp += iTemp << 2;
        iTemp += ps[x - 2*iStride];
        iTemp += ps[x + 3*iStride];

        pucDesHP[x]              = xClip( ( ps[x] + 16) / 32);
        pucDesHP[x+iDesStrideHP] = xClip( ( iTemp + 512) / 1024);
      }
      pucDesHP += iDesStrideHP<<1;
      ps     += iStride;
    }
  }

  delete [] psTemp;
  psTemp = NULL;
  return Err::m_nOK;
}

ErrVal QuarterPelFilter::filterFrame( IntYuvPicBuffer *pcPelBuffer, IntYuvPicBuffer *pcHalfPelBuffer )
{
  ROT( NULL == pcPelBuffer );
  ROT( NULL == pcHalfPelBuffer );

  XPel*   pucSrc      = pcPelBuffer->getMbLumAddr ();
  Int     iHeight     = pcPelBuffer->getLHeight   ();
  Int     iWidth      = pcPelBuffer->getLWidth    ();
  Int     iStride     = pcPelBuffer->getLStride   ();
  Int     iMargin     = pcPelBuffer->getLXMargin  ();
  UInt    uiTmpXSize  = (iMargin + iWidth + iMargin) * 2;
  UInt    uiTmpYSize  = iMargin + iHeight + iMargin;
  Int     iMarginNew  = iMargin-4;
  Int     x, y;

  XXPel*  psTemp      = new XXPel[uiTmpXSize * uiTmpYSize];
  ::memset( psTemp, 0x00, (uiTmpXSize * uiTmpYSize)*sizeof(XXPel) );

  ROT( NULL == psTemp )

  XXPel*  ps          = &psTemp[ iMargin * uiTmpXSize + 2*iMargin ]; // fix provided by Shijun Sun

  if( m_bRCDO )
  {
    for( y = 0; y < iHeight; y++ )
    {
      for( x = -iMarginNew; x < iWidth+iMarginNew; x++ )
      {
        Int iTemp;
        iTemp  = pucSrc[x - 0];
        iTemp += pucSrc[x + 1];
        iTemp  = iTemp << 2;
        iTemp -= pucSrc[x - 1];
        iTemp -= pucSrc[x + 2];
        iTemp += iTemp << 2;
        iTemp += pucSrc[x - 2];
        iTemp += pucSrc[x + 3];
        ps[2*x]    = pucSrc[x];
        ps[2*x+1]  = (XPel)xClip((iTemp + 6) >> 5);
      }

      ps     += uiTmpXSize;
      pucSrc += iStride;
    }
  }
  else
  {
    for( y = 0; y < iHeight; y++ )
    {
      for( x = -iMarginNew; x < iWidth+iMarginNew; x++ )
      {
        Int iTemp;
        iTemp  = pucSrc[x - 0];
        iTemp += pucSrc[x + 1];
        iTemp  = iTemp << 2;
        iTemp -= pucSrc[x - 1];
        iTemp -= pucSrc[x + 2];
        iTemp += iTemp << 2;
        iTemp += pucSrc[x - 2];
        iTemp += pucSrc[x + 3];
        ps[2*x]    = pucSrc[x]<<5;
        ps[2*x+1]  = iTemp;
      }

      ps     += uiTmpXSize;
      pucSrc += iStride;
    }
  }

  // bot
  ps -= iMargin*2;               // fix provided by Shijun Sun
  for( y = 0; y < iMargin; y++ ) // fix provided by Shijun Sun
  {
    ::memcpy( &ps[y*uiTmpXSize], &ps[(y-1)*uiTmpXSize], uiTmpXSize*sizeof(XXPel) );
  }

  //top
  ps = &psTemp[ iMargin * uiTmpXSize ]; // fix provided by Shijun Sun
  for( y = 0; y < iMargin; y++ )        // fix provided by Shijun Sun
  {
    ::memcpy( &ps[-(y+1)*uiTmpXSize], &ps[-y*uiTmpXSize], uiTmpXSize*sizeof(XXPel) );
  }

  ps = &psTemp[ 4*uiTmpXSize + 2*iMargin ]; // fix provided by Shijun Sun
  iStride = uiTmpXSize;

  Int   iDesStrideHP  = pcHalfPelBuffer->getLStride();
  XPel* pucDesHP      = pcHalfPelBuffer->getMbLumAddr();
  pucDesHP -= (iMarginNew*iDesStrideHP)<<1;

  if( m_bRCDO )
  {
    for( y = -iMarginNew; y < iHeight+iMarginNew; y++ )
    {
      for( x = -2*iMarginNew; x < 2*(iWidth+iMarginNew); x++ )
      {
        pucDesHP[x]                = (XPel)ps[x];
        if( ( x & 1 ) == 0 )
          pucDesHP[x+iDesStrideHP] = (XPel)xClip( ( (ps[x+iStride]+ps[x])*5 - ps[x+2*iStride] - ps[x-iStride] + 1 ) >> 3 );
        else
          pucDesHP[x+iDesStrideHP] = (XPel)( ps[x+iStride-1] + ps[x+1] ) >> 1;
      }
      pucDesHP += iDesStrideHP<<1;
      ps       += iStride;
    }
  }
  else
  {
    for( y = -iMarginNew; y < iHeight+iMarginNew; y++ )
    {
      for( x = -2*iMarginNew; x < 2*(iWidth+iMarginNew); x++ )
      {
        Int iTemp;
        iTemp  = ps[x - 0*iStride];
        iTemp += ps[x + 1*iStride];
        iTemp  = iTemp << 2;
        iTemp -= ps[x - 1*iStride];
        iTemp -= ps[x + 2*iStride];
        iTemp += iTemp << 2;
        iTemp += ps[x - 2*iStride];
        iTemp += ps[x + 3*iStride];

        pucDesHP[x]              = xClip( ( ps[x] + 16) / 32);
        pucDesHP[x+iDesStrideHP] = xClip( ( iTemp + 512) / 1024);
      }
      pucDesHP += iDesStrideHP<<1;
      ps     += iStride;
    }
  }

  delete [] psTemp;
  psTemp = NULL;
  return Err::m_nOK;
}


Void QuarterPelFilter::filterBlockRCDO( XPel* pDes, XPel* pSrc, Int iSrcStride, UInt uiXSize, UInt uiYSize, UInt uiFilter )
{
  UInt y, x;

  if( uiFilter == 0 ) // vertical full-sample, horizontal full-sample
  {
    for( y = 0; y < uiYSize; y++ )
    {
      for( x = 0; x < uiXSize; x++ )
      {
        XPel a = pSrc[-iSrcStride], b = pSrc[-1], o = pSrc[0], c = pSrc[1], d = pSrc[iSrcStride];

        pDes[0x000] = (a + b + 1) >> 1;
        pDes[0x100] = (a + o + 1) >> 1;
        pDes[0x200] = (a + c + 1) >> 1;
        pDes[0x300] = (b + o + 1) >> 1;
        pDes[0x500] = (o + c + 1) >> 1;
        pDes[0x600] = (b + d + 1) >> 1;
        pDes[0x700] = (o + d + 1) >> 1;
        pDes[0x800] = (c + d + 1) >> 1;
        pSrc += 2;
        pDes++;
      }
      pSrc += 2*(iSrcStride - uiXSize);
      pDes += 16 - uiXSize;
    }
  }
  else if( uiFilter == 1 ) // vertical full-sample, horizontal half-sample
  {
    for( y = 0; y < uiYSize; y++ )
    {
      for( x = 0; x < uiXSize; x++ )
      {
        pDes[0x000] = ( pSrc[ 0] + pSrc[-iSrcStride - 1] + 1 ) >> 1;
        pDes[0x100] = ( pSrc[ 1] + pSrc[-iSrcStride - 1] + 1 ) >> 1;
        pDes[0x200] = ( pSrc[ 0] + pSrc[-iSrcStride + 1] + 1 ) >> 1;
        pDes[0x300] = ( pSrc[ 0] + pSrc[            - 1] + 1 ) >> 1;
        pDes[0x500] = ( pSrc[ 0] + pSrc[              1] + 1 ) >> 1;
        pDes[0x600] = ( pSrc[ 0] + pSrc[ iSrcStride - 1] + 1 ) >> 1;
        pDes[0x700] = ( pSrc[-1] + pSrc[ iSrcStride + 1] + 1 ) >> 1;
        pDes[0x800] = ( pSrc[ 0] + pSrc[ iSrcStride + 1] + 1 ) >> 1;
        pSrc += 2;
        pDes++;
      }
      pSrc += 2*(iSrcStride - uiXSize);
      pDes += 16 - uiXSize;
    }
  }
  else if( uiFilter == 2 ) // vertical half-sample, horizontal full-sample
  {
    for( y = 0; y < uiYSize; y++ )
    {
      for( x = 0; x < uiXSize; x++ )
      {
        pDes[0x000] = ( pSrc[0]            + pSrc[-iSrcStride-1] + 1 ) >> 1;
        pDes[0x100] = ( pSrc[0]            + pSrc[-iSrcStride  ] + 1 ) >> 1;
        pDes[0x200] = ( pSrc[0]            + pSrc[-iSrcStride+1] + 1 ) >> 1;
        pDes[0x300] = ( pSrc[iSrcStride-1] + pSrc[-iSrcStride  ] + 1 ) >> 1;
        pDes[0x500] = ( pSrc[iSrcStride]   + pSrc[-iSrcStride+1] + 1 ) >> 1;
        pDes[0x600] = ( pSrc[0]            + pSrc[ iSrcStride-1] + 1 ) >> 1;
        pDes[0x700] = ( pSrc[0]            + pSrc[ iSrcStride  ] + 1 ) >> 1;
        pDes[0x800] = ( pSrc[0]            + pSrc[ iSrcStride+1] + 1 ) >> 1;
        pSrc += 2;
        pDes++;
      }
      pSrc += 2*(iSrcStride - uiXSize);
      pDes += 16 - uiXSize;
    }
  }
  else  // vertical half-sample, horizontal half-sample
  {
    for( y = 0; y < uiYSize; y++ )
    {
      for( x = 0; x < uiXSize; x++ )
      {
        pDes[0x000] = ( pSrc[-1]            + pSrc[-iSrcStride  ] + 1 ) >> 1;
        pDes[0x100] = ( pSrc[ 1]            + pSrc[-iSrcStride-1] + 1 ) >> 1;
        pDes[0x200] = ( pSrc[ 1]            + pSrc[-iSrcStride  ] + 1 ) >> 1;
        pDes[0x300] = ( pSrc[ iSrcStride-1] + pSrc[-iSrcStride  ] + 1 ) >> 1;
        pDes[0x500] = ( pSrc[-iSrcStride+1] + pSrc[ iSrcStride  ] + 1 ) >> 1;
        pDes[0x600] = ( pSrc[-1]            + pSrc[ iSrcStride  ] + 1 ) >> 1;
        pDes[0x700] = ( pSrc[-1]            + pSrc[ iSrcStride+1] + 1 ) >> 1;
        pDes[0x800] = ( pSrc[ 1]            + pSrc[ iSrcStride  ] + 1 ) >> 1;
        pSrc += 2;
        pDes++;
      }
      pSrc += 2*(iSrcStride - uiXSize);
      pDes += 16 - uiXSize;
    }
  }
}

Void QuarterPelFilter::predBlkRCDO( IntYuvMbBuffer* pcDesBuffer, IntYuvPicBuffer* pcSrcBuffer, LumaIdx cIdx, Mv cMv, Int iSizeY, Int iSizeX)
{
  XPel* pucDes    = pcDesBuffer->getYBlk( cIdx );
  XPel* pucSrc    = pcSrcBuffer->getYBlk( cIdx );
  Int iDesStride  = pcDesBuffer->getLStride();
  Int iSrcStride  = pcSrcBuffer->getLStride();
  Int iOffset     = (cMv.getHor() >> 2) + (cMv.getVer() >> 2) * iSrcStride;

  pucSrc += iOffset;

  Int iDx = cMv.getHor() & 3;
  Int iDy = cMv.getVer() & 3;

  for( Int y = 0; y < iSizeY; y++)
  {
    for( Int x = 0; x < iSizeX; x++ )
    {
      XPel  A = pucSrc[x];
      XPel  B = pucSrc[x+1];
      XPel  C = pucSrc[x+iSrcStride];
      XPel  D = pucSrc[x+iSrcStride+1];
      XPel* p = pucSrc+x;
      XPel  b, h, j, m, s;

      j = (B + C) >> 1;
      b = xClip( ( (p[0]+p[1])*20 - (p[-1]+p[2])* 5 +(p[-2]+p[3]) + 6 ) >> 5 );
      h = xClip( ( (p[0]+p[iSrcStride])*5 - (p[-iSrcStride]+p[2*iSrcStride]) + 1 ) >> 3);
      p = pucSrc+x+iSrcStride;
      s = xClip( ( (p[0]+p[1])*20 - (p[-1]+p[2])* 5 +(p[-2]+p[3]) + 6 ) >> 5 );
      p = pucSrc+x+1;
      m = xClip( ( (p[0]+p[iSrcStride])*5 - (p[-iSrcStride]+p[2*iSrcStride]) + 1 ) >> 3);

      if( iDy == 0 )
      {
        pucDes[x] = ( iDx == 0 ?  A               : ( iDx == 1 ? (A + b + 1) >> 1 : ( iDx == 2 ?  b               : (B + b + 1) >> 1 ) ) );
      }
      else if( iDy == 1 )
      {
        pucDes[x] = ( iDx == 0 ? (A + h + 1) >> 1 : ( iDx == 1 ? (b + h + 1) >> 1 : ( iDx == 2 ? (A + m + 1) >> 1 : (b + m + 1) >> 1 ) ) );
      }
      else if( iDy == 2 )
      {
        pucDes[x] = ( iDx == 0 ?  h               : ( iDx == 1 ? (C + b + 1) >> 1 : ( iDx == 2 ?  j               : (B + s + 1) >> 1 ) ) );
      }
      else
      {
        pucDes[x] = ( iDx == 0 ? (C + h + 1) >> 1 : ( iDx == 1 ? (h + s + 1) >> 1 : ( iDx == 2 ? (D + h + 1) >> 1 : (m + s + 1) >> 1 ) ) );
      }
    }

    pucDes += iDesStride;
    pucSrc += iSrcStride;
  }
}

Void QuarterPelFilter::predBlkRCDO( YuvMbBuffer* pcDesBuffer, YuvPicBuffer* pcSrcBuffer, LumaIdx cIdx, Mv cMv, Int iSizeY, Int iSizeX)
{
  Pel* pucDes     = pcDesBuffer->getYBlk( cIdx );
  Pel* pucSrc     = pcSrcBuffer->getYBlk( cIdx );
  Int  iDesStride = pcDesBuffer->getLStride();
  Int  iSrcStride = pcSrcBuffer->getLStride();
  Int  iOffset    = (cMv.getHor() >> 2) + (cMv.getVer() >> 2) * iSrcStride;

  pucSrc += iOffset;

  Int iDx = cMv.getHor() & 3;
  Int iDy = cMv.getVer() & 3;

  for( Int y = 0; y < iSizeY; y++)
  {
    for( Int x = 0; x < iSizeX; x++ )
    {
      Pel  A = pucSrc[x];
      Pel  B = pucSrc[x+1];
      Pel  C = pucSrc[x+iSrcStride];
      Pel  D = pucSrc[x+iSrcStride+1];
      Pel* p = pucSrc+x;
      Pel  b, h, j, m, s;

      j = (B + C) >> 1;
      b = xClip( ( (p[0]+p[1])*20 - (p[-1]+p[2])* 5 +(p[-2]+p[3]) + 6 ) >> 5 );
      h = xClip( ( (p[0]+p[iSrcStride])*5 - (p[-iSrcStride]+p[2*iSrcStride]) + 1 ) >> 3);
      p = pucSrc+x+iSrcStride;
      s = xClip( ( (p[0]+p[1])*20 - (p[-1]+p[2])* 5 +(p[-2]+p[3]) + 6 ) >> 5 );
      p = pucSrc+x+1;
      m = xClip( ( (p[0]+p[iSrcStride])*5 - (p[-iSrcStride]+p[2*iSrcStride]) + 1 ) >> 3);

      if( iDy == 0 )
      {
        pucDes[x] = ( iDx == 0 ?  A               : ( iDx == 1 ? (A + b + 1) >> 1 : ( iDx == 2 ?  b               : (B + b + 1) >> 1 ) ) );
      }
      else if( iDy == 1 )
      {
        pucDes[x] = ( iDx == 0 ? (A + h + 1) >> 1 : ( iDx == 1 ? (b + h + 1) >> 1 : ( iDx == 2 ? (A + m + 1) >> 1 : (b + m + 1) >> 1 ) ) );
      }
      else if( iDy == 2 )
      {
        pucDes[x] = ( iDx == 0 ?  h               : ( iDx == 1 ? (C + b + 1) >> 1 : ( iDx == 2 ?  j               : (B + s + 1) >> 1 ) ) );
      }
      else
      {
        pucDes[x] = ( iDx == 0 ? (C + h + 1) >> 1 : ( iDx == 1 ? (h + s + 1) >> 1 : ( iDx == 2 ? (D + h + 1) >> 1 : (m + s + 1) >> 1 ) ) );
      }
    }

    pucDes += iDesStride;
    pucSrc += iSrcStride;
  }
}




Void QuarterPelFilter::xFilter1( Pel* pDes, Pel* pSrc, Int iSrcStride, UInt uiXSize, UInt uiYSize )
{
  UInt y, x;
  for( y = 0; y < uiYSize; y++ )
  {
    for( x = 0; x < uiXSize; x++ )
    {
      Int a = pSrc[-1];
      Int b = pSrc[-iSrcStride];
      Int c = pSrc[+1];
      Int d = pSrc[iSrcStride];
      Int o = pSrc[0];

      pDes[0x000] = (a + b + 1) >> 1;
      pDes[0x100] = (b + o + 1) >> 1;
      pDes[0x200] = (b + c + 1) >> 1;
      pDes[0x300] = (a + o + 1) >> 1;
      pDes[0x500] = (c + o + 1) >> 1;
      pDes[0x600] = (a + d + 1) >> 1;
      pDes[0x700] = (d + o + 1) >> 1;
      pDes[0x800] = (d + c + 1) >> 1;
      pSrc += 2;
      pDes++;
    }
    pSrc += 2*(iSrcStride - uiXSize);
    pDes += 16 - uiXSize;
  }
}

Void QuarterPelFilter::xXFilter1( XPel* pDes, XPel* pSrc, Int iSrcStride, UInt uiXSize, UInt uiYSize )
{
  UInt y, x;
  for( y = 0; y < uiYSize; y++ )
  {
    for( x = 0; x < uiXSize; x++ )
    {
      Int a = pSrc[-1];
      Int b = pSrc[-iSrcStride];
      Int c = pSrc[+1];
      Int d = pSrc[iSrcStride];
      Int o = pSrc[0];

      pDes[0x000] = (a + b + 1) >> 1;
      pDes[0x100] = (b + o + 1) >> 1;
      pDes[0x200] = (b + c + 1) >> 1;
      pDes[0x300] = (a + o + 1) >> 1;
      pDes[0x500] = (c + o + 1) >> 1;
      pDes[0x600] = (a + d + 1) >> 1;
      pDes[0x700] = (d + o + 1) >> 1;
      pDes[0x800] = (d + c + 1) >> 1;
      pSrc += 2;
      pDes++;
    }
    pSrc += 2*(iSrcStride - uiXSize);
    pDes += 16 - uiXSize;
  }
}


Void QuarterPelFilter::xFilter2( Pel* pDes, Pel* pSrc, Int iSrcStride, UInt uiXSize, UInt uiYSize )
{
  UInt y, x;

  for( y = 0; y < uiYSize; y++ )
  {
    for( x = 0; x < uiXSize; x++ )
    {
      Int o = pSrc[0];

      pDes[0x000] = (o + pSrc[-iSrcStride-1] + 1) >> 1;
      pDes[0x100] = (o + pSrc[-iSrcStride  ] + 1) >> 1;
      pDes[0x200] = (o + pSrc[-iSrcStride+1] + 1) >> 1;
      pDes[0x300] = (o + pSrc[-1] + 1) >> 1;
      pDes[0x500] = (o + pSrc[+1] + 1) >> 1;
      pDes[0x600] = (o + pSrc[iSrcStride-1] + 1) >> 1;
      pDes[0x700] = (o + pSrc[iSrcStride  ] + 1) >> 1;
      pDes[0x800] = (o + pSrc[iSrcStride+1] + 1) >> 1;
      pSrc += 2;
      pDes++;
    }
    pSrc += 2*(iSrcStride - uiXSize);
    pDes += 16 - uiXSize;
  }
}

Void QuarterPelFilter::xXFilter2( XPel* pDes, XPel* pSrc, Int iSrcStride, UInt uiXSize, UInt uiYSize )
{
  UInt y, x;

  for( y = 0; y < uiYSize; y++ )
  {
    for( x = 0; x < uiXSize; x++ )
    {
      Int o = pSrc[0];

      pDes[0x000] = (o + pSrc[-iSrcStride-1] + 1) >> 1;
      pDes[0x100] = (o + pSrc[-iSrcStride  ] + 1) >> 1;
      pDes[0x200] = (o + pSrc[-iSrcStride+1] + 1) >> 1;
      pDes[0x300] = (o + pSrc[-1] + 1) >> 1;
      pDes[0x500] = (o + pSrc[+1] + 1) >> 1;
      pDes[0x600] = (o + pSrc[iSrcStride-1] + 1) >> 1;
      pDes[0x700] = (o + pSrc[iSrcStride  ] + 1) >> 1;
      pDes[0x800] = (o + pSrc[iSrcStride+1] + 1) >> 1;
      pSrc += 2;
      pDes++;
    }
    pSrc += 2*(iSrcStride - uiXSize);
    pDes += 16 - uiXSize;
  }
}

Void QuarterPelFilter::xFilter3( Pel* pDes, Pel* pSrc, Int iSrcStride, UInt uiXSize, UInt uiYSize )
{
  UInt y, x;

  for( y = 0; y < uiYSize; y++ )
  {
    for( x = 0; x < uiXSize; x++ )
    {
      Int o = pSrc[0];

      pDes[0x000] = (o + pSrc[-iSrcStride-1] + 1) >> 1;
      pDes[0x100] = (o + pSrc[-iSrcStride  ] + 1) >> 1;
      pDes[0x200] = (o + pSrc[-iSrcStride+1] + 1) >> 1;
      pDes[0x300] = (o + pSrc[-1] + 1) >> 1;
      pDes[0x500] = (o + pSrc[+1] + 1) >> 1;
      pDes[0x600] = (o + pSrc[iSrcStride-1] + 1) >> 1;
      pDes[0x700] = (o + pSrc[iSrcStride  ] + 1) >> 1;
      pDes[0x800] = (o + pSrc[iSrcStride+1] + 1) >> 1;
      pSrc += 2;
      pDes++;
    }
    pSrc += 2*(iSrcStride - uiXSize);
    pDes += 16 - uiXSize;
  }
}

Void QuarterPelFilter::xXFilter3( XPel* pDes, XPel* pSrc, Int iSrcStride, UInt uiXSize, UInt uiYSize )
{
  UInt y, x;

  for( y = 0; y < uiYSize; y++ )
  {
    for( x = 0; x < uiXSize; x++ )
    {
      Int o = pSrc[0];

      pDes[0x000] = (o + pSrc[-iSrcStride-1] + 1) >> 1;
      pDes[0x100] = (o + pSrc[-iSrcStride  ] + 1) >> 1;
      pDes[0x200] = (o + pSrc[-iSrcStride+1] + 1) >> 1;
      pDes[0x300] = (o + pSrc[-1] + 1) >> 1;
      pDes[0x500] = (o + pSrc[+1] + 1) >> 1;
      pDes[0x600] = (o + pSrc[iSrcStride-1] + 1) >> 1;
      pDes[0x700] = (o + pSrc[iSrcStride  ] + 1) >> 1;
      pDes[0x800] = (o + pSrc[iSrcStride+1] + 1) >> 1;
      pSrc += 2;
      pDes++;
    }
    pSrc += 2*(iSrcStride - uiXSize);
    pDes += 16 - uiXSize;
  }
}

Void QuarterPelFilter::xFilter4( Pel* pDes, Pel* pSrc, Int iSrcStride, UInt uiXSize, UInt uiYSize )
{
  UInt y, x;

  for( y = 0; y < uiYSize; y++ )
  {
    for( x = 0; x < uiXSize; x++ )
    {
      Int a = pSrc[-1];
      Int b = pSrc[-iSrcStride];
      Int c = pSrc[+1];
      Int d = pSrc[iSrcStride];
      Int o = pSrc[0];

      pDes[0x000] = (a + b + 1) >> 1;
      pDes[0x100] = (b + o + 1) >> 1;
      pDes[0x200] = (b + c + 1) >> 1;
      pDes[0x300] = (a + o + 1) >> 1;
      pDes[0x500] = (c + o + 1) >> 1;
      pDes[0x600] = (a + d + 1) >> 1;
      pDes[0x700] = (d + o + 1) >> 1;
      pDes[0x800] = (c + d + 1) >> 1;
      pSrc += 2;
      pDes++;
    }
    pSrc += 2*(iSrcStride - uiXSize);
    pDes += 16 - uiXSize;
  }
}
Void QuarterPelFilter::xXFilter4( XPel* pDes, XPel* pSrc, Int iSrcStride, UInt uiXSize, UInt uiYSize )
{
  UInt y, x;

  for( y = 0; y < uiYSize; y++ )
  {
    for( x = 0; x < uiXSize; x++ )
    {
      Int a = pSrc[-1];
      Int b = pSrc[-iSrcStride];
      Int c = pSrc[+1];
      Int d = pSrc[iSrcStride];
      Int o = pSrc[0];

      pDes[0x000] = (a + b + 1) >> 1;
      pDes[0x100] = (b + o + 1) >> 1;
      pDes[0x200] = (b + c + 1) >> 1;
      pDes[0x300] = (a + o + 1) >> 1;
      pDes[0x500] = (c + o + 1) >> 1;
      pDes[0x600] = (a + d + 1) >> 1;
      pDes[0x700] = (d + o + 1) >> 1;
      pDes[0x800] = (c + d + 1) >> 1;
      pSrc += 2;
      pDes++;
    }
    pSrc += 2*(iSrcStride - uiXSize);
    pDes += 16 - uiXSize;
  }
}



Void QuarterPelFilter::predBlkSR( IntYuvMbBuffer* pcDesBuffer, IntYuvPicBuffer* pcSrcBuffer, LumaIdx cIdx, Mv cMv, Int iSizeY, Int iSizeX)
{
  XPel* pucDes    = pcDesBuffer->getYBlk( cIdx );
  XPel* pucSrc    = pcSrcBuffer->getYBlk( cIdx );
  Int   iDesStride  = pcDesBuffer->getLStride();
  Int   iSrcStride  = pcSrcBuffer->getLStride();
  Int   iOffset     = (cMv.getHor() >> 2) + (cMv.getVer() >> 2) * iSrcStride;
  XPel  ucTmpBlk[16];
  XPel  ucExtBlk[18*16];    // max size of internal block buffer 
  XPel* pucExtBlk;
  Int   x, y;

  // TO BE FIXED: currently do not smooth across MB boundary, need to fix 
  pucSrc += iOffset;

  Int iDx = cMv.getHor() & 3;
  Int iDy = cMv.getVer() & 3;

  if( iDx == 0 && iDy == 0 )
  {
    pucExtBlk  = ucExtBlk;
    pucSrc    -= iSrcStride;
    for( y = -1; y < iSizeY+1; y++)
    {
      for( x = 0; x < iSizeX; x++ )
        pucExtBlk[x] = pucSrc[x-1] + pucSrc[x]*2 + pucSrc[x+1];
      
      pucExtBlk += 16;
      pucSrc    += iSrcStride;
    }
    
    pucDes     = pcDesBuffer->getYBlk( cIdx );
    pucExtBlk  = ucExtBlk + 16;
    for( x = 0; x < iSizeX; x++)
    {
      for( y = 0; y < iSizeY; y++ )
      {
        ucTmpBlk[y] = (pucExtBlk[x+(y-1)*16] + pucExtBlk[x+y*16]*2 + pucExtBlk[x+(y+1)*16] + 8) >> 4;
      }

      for( y = 0; y < iSizeY; y++ )
        pucDes[x + y*iDesStride] = ucTmpBlk[y];
    }
  }
  else if ( iDx == 0 ) // iDy != 0
  {
    pucExtBlk  = ucExtBlk;
    for( y = 0; y < iSizeY+1; y++)
    {
      for( x = 0; x < iSizeX; x++ )
        pucExtBlk[x] = pucSrc[x-1] + pucSrc[x]*2 + pucSrc[x+1];
      
      pucExtBlk += 16;
      pucSrc    += iSrcStride;
    }

    pucDes    = pcDesBuffer->getYBlk( cIdx );
    pucExtBlk = ucExtBlk;
    for( y = 0; y < iSizeY; y++)
    {
      for( x = 0; x < iSizeX; x++ )
        pucDes[x] = (pucExtBlk[x] * (4 - iDy) + pucExtBlk[x + 16] * iDy + 8 ) >> 4;

      pucExtBlk += 16;
      pucDes    += iDesStride;
    }
  }
  else if ( iDy == 0 ) // iDx != 0
  {
    pucExtBlk  = ucExtBlk;
    pucSrc    -= iSrcStride; 
    for( y = -1; y < iSizeY+1; y++)
    {
      for( x = 0; x < iSizeX; x++ )
      {
        pucExtBlk[x] = (pucSrc[x] * (4 - iDx) + pucSrc[x + 1] * iDx );
      }

      pucExtBlk += 16;
      pucSrc    += iSrcStride;
    }

    pucDes     = pcDesBuffer->getYBlk( cIdx );
    pucExtBlk  = ucExtBlk+16;
    for( y = 0; y < iSizeY; y++ )
    {
      for( x = 0; x < iSizeX; x++)
      {
        pucDes[x] = (pucExtBlk[x - 16] + pucExtBlk[x]*2 + pucExtBlk[x + 16] + 8) >> 4;
      }

      pucExtBlk += 16;
      pucDes    += iDesStride;
    }
  }
  else // if (iDx != 0 && iDy != 0) 
  {
    pucExtBlk  = ucExtBlk;
    for( y = 0; y < iSizeY+1; y++)
    {
      for( x = 0; x < iSizeX; x++ )
      {
        pucExtBlk[x] = (pucSrc[x] * (4 - iDx) + pucSrc[x + 1] * iDx );
      }

      pucExtBlk += 16;
      pucSrc    += iSrcStride;
    }

    pucDes    = pcDesBuffer->getYBlk( cIdx );
    pucExtBlk = ucExtBlk;
    for( y = 0; y < iSizeY; y++)
    {
      for( x = 0; x < iSizeX; x++ )
        pucDes[x] = (pucExtBlk[x] * (4 - iDy) + pucExtBlk[x + 16] * iDy + 8 ) >> 4;

      pucExtBlk += 16;
      pucDes    += iDesStride;
    }
  }
}


Void QuarterPelFilter::predBlkBilinear( IntYuvMbBuffer* pcDesBuffer, IntYuvPicBuffer* pcSrcBuffer, LumaIdx cIdx, Mv cMv, Int iSizeY, Int iSizeX)
{
  XPel* pucDes    = pcDesBuffer->getYBlk( cIdx );
  XPel* pucSrc    = pcSrcBuffer->getYBlk( cIdx );
  Int iDesStride  = pcDesBuffer->getLStride();
  Int iSrcStride  = pcSrcBuffer->getLStride();
  Int iOffset     = (cMv.getHor() >> 2) + (cMv.getVer() >> 2) * iSrcStride;

  pucSrc += iOffset;

  Int iDx = cMv.getHor() & 3;
  Int iDy = cMv.getVer() & 3;

  if( iDx == 0 && iDy == 0 )
  {
    for( Int y = 0; y < iSizeY; y++)
    {
      for( Int x = 0; x < iSizeX; x++ )
        pucDes[x] = pucSrc[x];

      pucDes += iDesStride;
      pucSrc += iSrcStride;
    }
  }
  else
  {
    for( Int y = 0; y < iSizeY; y++)
    {
      for( Int x = 0; x < iSizeX; x++ )
      {
        Int iSum = 
          (pucSrc[x]              * (4 - iDx) + pucSrc[x + 1]              * iDx) * (4 - iDy) + 
          (pucSrc[iSrcStride + x] * (4 - iDx) + pucSrc[iSrcStride + x + 1] * iDx) * iDy;

        pucDes[x] = ( iSum >= 0 ) ? ( ( iSum + 8 ) >> 4 ) : -( ( -iSum + 8 ) >> 4 );
      }

      pucDes += iDesStride;
      pucSrc += iSrcStride;
    }
  }
}


Void QuarterPelFilter::predBlk4Tap( IntYuvMbBuffer* pcDesBuffer, IntYuvPicBuffer* pcSrcBuffer, LumaIdx cIdx, Mv cMv, Int iSizeY, Int iSizeX)
{
  XPel* pucDes    = pcDesBuffer->getYBlk( cIdx );
  XPel* pucSrc    = pcSrcBuffer->getYBlk( cIdx );
  Int iDesStride  = pcDesBuffer->getLStride();
  Int iSrcStride  = pcSrcBuffer->getLStride();
  Int iOffset     = (cMv.getHor() >> 2) + (cMv.getVer() >> 2) * iSrcStride;

  pucSrc += iOffset;

  Int iDx = cMv.getHor() & 3;
  Int iDy = cMv.getVer() & 3;
  static int f4tap[4][4] = {
    { 0, 16,  0,  0}, 
    {-2, 14,  5, -1},
    {-2, 10, 10, -2},
    {-1,  5, 14, -2}
  };

  for( Int y = 0; y < iSizeY; y++)
  {
    for( Int x = 0; x < iSizeX; x++ )
    {
      Int iTemp1[4], iTemp2;
      int i, j;

      for( i = 0; i < 4; i++ )
      {
        iTemp1[i] = 0;
        for( j = 0; j < 4; j++ )
          iTemp1[i] += pucSrc[x + (i - 1) * iSrcStride + j - 1] * f4tap[iDx][j];
      }

      iTemp2 = 0;
      for(j=0;j<4;j++)
        iTemp2 += iTemp1[j] * f4tap[iDy][j];

      if( m_bClip )
        pucDes[x] = xClip( (iTemp2 + 128) >> 8 );
      else
        pucDes[x] = (iTemp2 >= 0) ? ( (iTemp2 + 128) >> 8 ) : -( (-iTemp2 + 128) >> 8 );
    }

    pucDes += iDesStride;
    pucSrc += iSrcStride;
  }
}


Void QuarterPelFilter::predBlk( IntYuvMbBuffer* pcDesBuffer, IntYuvPicBuffer* pcSrcBuffer, LumaIdx cIdx, Mv cMv, Int iSizeY, Int iSizeX)
{
  if( m_bRCDO )
  {
    predBlkRCDO( pcDesBuffer, pcSrcBuffer, cIdx, cMv, iSizeY, iSizeX );
    return;
  }

  if( giInterpolationType == AR_FGS_MC_INTERP_BILINEAR )
  {
    predBlkBilinear(pcDesBuffer, pcSrcBuffer, cIdx, cMv, iSizeY, iSizeX);
    return;
  }
  else if( giInterpolationType == AR_FGS_MC_INTERP_4_TAP )
  {
    predBlk4Tap(pcDesBuffer, pcSrcBuffer, cIdx, cMv, iSizeY, iSizeX);
    return;
  }

  XPel* pucDes    = pcDesBuffer->getYBlk( cIdx );
  XPel* pucSrc    = pcSrcBuffer->getYBlk( cIdx );
  Int iDesStride  = pcDesBuffer->getLStride();
  Int iSrcStride  = pcSrcBuffer->getLStride();
  Int iOffset     = (cMv.getHor() >> 2) + (cMv.getVer() >> 2) * iSrcStride;

  pucSrc += iOffset;

  Int iDx = cMv.getHor() & 3;
  Int iDy = cMv.getVer() & 3;
  if( iDy == 0)
  {
    if( iDx == 0 )
    {
      for( Int y = 0; y < iSizeY; y++)
      {
        for( Int x = 0; x < iSizeX; x++ )
        {
          pucDes[x] = pucSrc[x];
        }
        pucDes += iDesStride;
        pucSrc += iSrcStride;
      }
      return;
    }


    if( iDx == 2 )
    {
      xPredDy0Dx2( pucDes, pucSrc, iDesStride, iSrcStride, iSizeY, iSizeX );
      return;
    }

    // if( iDx == 1 || iDx == 3)
    xPredDy0Dx13( pucDes, pucSrc, iDesStride, iSrcStride, iDx, iSizeY, iSizeX );
    return;
  }


  if( iDx == 0)
  {
    if( iDy == 2 )
    {
      xPredDx0Dy2( pucDes, pucSrc, iDesStride, iSrcStride, iSizeY, iSizeX );
      return;
    }

    // if( iDx == 1 || iDx == 3)
    xPredDx0Dy13( pucDes, pucSrc, iDesStride, iSrcStride, iDx, iDy, iSizeY, iSizeX );
    return;
  }


  if( iDx == 2)
  {
    if( iDy == 2 )
    {
      xPredDx2Dy2( pucDes, pucSrc, iDesStride, iSrcStride, iDx, iDy, iSizeY, iSizeX );
      return;
    }

    // if( iDy == 1 || iDy == 3 )
    xPredDx2Dy13( pucDes, pucSrc, iDesStride, iSrcStride, iDx, iDy, iSizeY, iSizeX );
    return;
  }



  if( iDy == 2)
  {
    // if( iDx == 2 ) is already done
    // if( iDx == 1 || iDx == 3 )
    xPredDy2Dx13( pucDes, pucSrc, iDesStride, iSrcStride, iDx, iDy, iSizeY, iSizeX );
    return;
  }


  xPredElse( pucDes, pucSrc, iDesStride, iSrcStride, iDx, iDy, iSizeY, iSizeX );
  return;
}




Void QuarterPelFilter::xPredDy0Dx2( XPel* pucDest, XPel* pucSrc, Int iDestStride, Int iSrcStride,  UInt uiSizeY, UInt uiSizeX )
{
  for( UInt y = 0; y < uiSizeY; y++)
  {
    for( UInt x = 0; x < uiSizeX; x++)
    {
      Int iTemp;
      iTemp  = pucSrc[x - 0];
      iTemp += pucSrc[x + 1];
      iTemp  = iTemp << 2;
      iTemp -= pucSrc[x - 1];
      iTemp -= pucSrc[x + 2];
      iTemp += iTemp << 2;
      iTemp += pucSrc[x - 2];
      iTemp += pucSrc[x + 3];
#if AR_FGS_COMPENSATE_SIGNED_FRAME
      if( ! m_bClip )
        pucDest[x] =  SIGNED_ROUNDING( iTemp, 16, 5 );
      else
#endif
      pucDest[x] = xClip( (iTemp + 16) / 32 );
    }
    pucDest += iDestStride;
    pucSrc  += iSrcStride;
  }
}


Void QuarterPelFilter::xPredDy0Dx13( XPel* pucDest, XPel* pucSrc, Int iDestStride, Int iSrcStride, Int iDx, UInt uiSizeY, UInt uiSizeX )
{

  iDx >>= 1;
  for( UInt y = 0; y < uiSizeY; y++)
  {
    for( UInt x = 0; x < uiSizeX; x++)
    {
      Int iTemp;
      iTemp  = pucSrc[x - 0];
      iTemp += pucSrc[x + 1];
      iTemp  = iTemp << 2;
      iTemp -= pucSrc[x - 1];
      iTemp -= pucSrc[x + 2];
      iTemp += iTemp << 2;
      iTemp += pucSrc[x - 2];
      iTemp += pucSrc[x + 3];

#if AR_FGS_COMPENSATE_SIGNED_FRAME
      if( ! m_bClip )
      {
        iTemp     += pucSrc[ x + iDx] << 5;
        pucDest[x] = SIGNED_ROUNDING( iTemp,  32, 6 );
      }
      else
      {
        iTemp = xClip( (iTemp + 16) / 32 );
        pucDest[x] = (iTemp + pucSrc[ x + iDx] + 1) / 2;
      }
#else
      iTemp = xClip( (iTemp + 16) / 32 );
      pucDest[x] = (iTemp + pucSrc[ x + iDx] + 1) / 2;
#endif
    }
    pucDest += iDestStride;
    pucSrc  += iSrcStride;
  }
}


Void QuarterPelFilter::xPredDx0Dy2( XPel* pucDest, XPel* pucSrc, Int iDestStride, Int iSrcStride, UInt uiSizeY, UInt uiSizeX )
{
  for( UInt y = 0; y < uiSizeY; y++)
  {
    for( UInt x = 0; x < uiSizeX; x++)
    {
      Int iTemp;
      iTemp  = pucSrc[x - 0*iSrcStride];
      iTemp += pucSrc[x + 1*iSrcStride];
      iTemp  = iTemp << 2;
      iTemp -= pucSrc[x - 1*iSrcStride];
      iTemp -= pucSrc[x + 2*iSrcStride];
      iTemp += iTemp << 2;
      iTemp += pucSrc[x - 2*iSrcStride];
      iTemp += pucSrc[x + 3*iSrcStride];
#if AR_FGS_COMPENSATE_SIGNED_FRAME
      if( ! m_bClip )
        pucDest[x] = SIGNED_ROUNDING( iTemp, 16, 5 );
      else
#endif
      pucDest[x] = xClip( (iTemp + 16) / 32 );
    }
    pucDest += iDestStride;
    pucSrc  += iSrcStride;
  }
}


Void QuarterPelFilter::xPredDx0Dy13( XPel* pucDest, XPel* pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX )
{
  iDy = (iDy>>1) * iSrcStride;
  for( UInt y = 0; y < uiSizeY; y++)
  {
    for( UInt x = 0; x < uiSizeX; x++)
    {
      Int iTemp;
      iTemp  = pucSrc[x - 0*iSrcStride];
      iTemp += pucSrc[x + 1*iSrcStride];
      iTemp  = iTemp << 2;
      iTemp -= pucSrc[x - 1*iSrcStride];
      iTemp -= pucSrc[x + 2*iSrcStride];
      iTemp += iTemp << 2;
      iTemp += pucSrc[x - 2*iSrcStride];
      iTemp += pucSrc[x + 3*iSrcStride];
#if AR_FGS_COMPENSATE_SIGNED_FRAME
      if( ! m_bClip )
      {
        iTemp     += pucSrc[ x + iDy] << 5;
        pucDest[x] = SIGNED_ROUNDING( iTemp, 32, 6 );
      }
      else
      {
        iTemp = xClip( (iTemp + 16) / 32 );
        pucDest[x] = (iTemp + pucSrc[ x + iDy] + 1)/2;
      }
#else
      iTemp = xClip( (iTemp + 16) / 32 );
      pucDest[x] = (iTemp + pucSrc[ x + iDy] + 1)/2;
#endif
    }
    pucDest += iDestStride;
    pucSrc  += iSrcStride;
  }
}



Void QuarterPelFilter::xPredDx2( XXPel*  psDest, XPel*  pucSrc, Int iSrcStride, UInt uiSizeY, UInt uiSizeX )
{
  pucSrc -= 2*iSrcStride;

  for( UInt y = 0; y < uiSizeY + 5; y++)
  {
    for( UInt x = 0; x < uiSizeX; x++)
    {
      XPel* puc = pucSrc + x;
      Int iTemp;
      iTemp  = puc[0];
      iTemp += puc[1];
      iTemp  = iTemp << 2;
      iTemp -= puc[-1];
      iTemp -= puc[2];
      iTemp += iTemp << 2;
      iTemp += puc[-2];
      iTemp += puc[3];
      psDest[x] = iTemp;

    }
    psDest += 16;
    pucSrc += iSrcStride;
  }
}



Void QuarterPelFilter::xPredDx2Dy2( XPel* pucDest, XPel* pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX )
{
  XXPel   asTemp[16*(16+6)];
  XXPel*  psTemp = asTemp;
  xPredDx2( asTemp, pucSrc, iSrcStride, uiSizeY, uiSizeX );

  for( UInt y = 0; y < uiSizeY; y++)
  {
    for( UInt x = 0; x < uiSizeX; x++)
    {
      Int iIndex = x + 0x20;
      Int iTemp;
      iTemp  = psTemp[ 0x00 + iIndex];
      iTemp += psTemp[ 0x10 + iIndex];
      iTemp  = iTemp << 2;
      iTemp -= psTemp[-0x10 + iIndex];
      iTemp -= psTemp[ 0x20 + iIndex];
      iTemp += iTemp << 2;
      iTemp += psTemp[-0x20 + iIndex];
      iTemp += psTemp[ 0x30 + iIndex];

#if AR_FGS_COMPENSATE_SIGNED_FRAME
      if( ! m_bClip )
        pucDest[x] = SIGNED_ROUNDING( iTemp, 512, 10 );
      else
#endif
      pucDest[x] = xClip( (iTemp + 512) / 1024 );
    }
    psTemp  += 0x10;
    pucDest += iDestStride;
  }

}


Void QuarterPelFilter::xPredDx2Dy13( XPel* pucDest, XPel* pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX )
{
  XXPel   asTemp[16*(16+6)];
  XXPel*  psTemp = asTemp;
  xPredDx2( asTemp, pucSrc, iSrcStride, uiSizeY, uiSizeX );

  iDy = (iDy == 1) ? 0 : 0x10;

  for( UInt y = 0; y < uiSizeY; y++)
  {
    for( UInt x = 0; x < uiSizeX; x++)
    {
      Int iIndex = x + 0x20;
      Int iTemp;
      iTemp  = psTemp[ 0x00 + iIndex];
      iTemp += psTemp[ 0x10 + iIndex];
      iTemp  = iTemp << 2;
      iTemp -= psTemp[-0x10 + iIndex];
      iTemp -= psTemp[ 0x20 + iIndex];
      iTemp += iTemp << 2;
      iTemp += psTemp[-0x20 + iIndex];
      iTemp += psTemp[ 0x30 + iIndex];
#if AR_FGS_COMPENSATE_SIGNED_FRAME
      if( ! m_bClip )
      {
        iTemp     += psTemp[iDy + iIndex] << 5;
        pucDest[x] = SIGNED_ROUNDING( iTemp, 1024, 11 );
      }
      else
      {
        iTemp = xClip( (iTemp + 512) / 1024 );
        pucDest[x] = (iTemp + xClip( (psTemp[iDy + iIndex] + 16) / 32 ) + 1) / 2;
      }
#else
      iTemp = xClip( (iTemp + 512) / 1024 );
      pucDest[x] = (iTemp + xClip( (psTemp[iDy + iIndex] + 16) / 32 ) + 1) / 2;
#endif
    }
    psTemp  += 0x10;
    pucDest += iDestStride;
  }
}


Void QuarterPelFilter::xPredDy2Dx13( XPel* pucDest, XPel* pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX )
{
  iDx = (iDx == 1) ? 2 : 3;
  for( UInt y = 0; y < uiSizeY; y++)
  {
    Int aiTemp[6+16];
    for( UInt n = 0; n < 6+uiSizeX; n++)
    {
      XPel* puc = pucSrc + n - 2;
      aiTemp[n]  = puc[ 0*iSrcStride];
      aiTemp[n] += puc[ 1*iSrcStride];
      aiTemp[n]  = aiTemp[n] << 2;
      aiTemp[n] -= puc[-1*iSrcStride];
      aiTemp[n] -= puc[ 2*iSrcStride];
      aiTemp[n] += aiTemp[n] << 2;
      aiTemp[n] += puc[-2*iSrcStride];
      aiTemp[n] += puc[ 3*iSrcStride];
    }

    for( UInt x = 0; x < uiSizeX; x++)
    {
      Int iTemp = 0;
      for( Int n = 0; n < 6; n++ )
      {
        iTemp += aiTemp[n+x]*g_aiTapCoeff[n];
      }
#if AR_FGS_COMPENSATE_SIGNED_FRAME
      if( ! m_bClip )
      {
        iTemp     += aiTemp[x+iDx] << 5;
        pucDest[x] = SIGNED_ROUNDING( iTemp, 1024, 11 );
      }
      else
      {
        iTemp = xClip( (iTemp + 512) / 1024 );
        pucDest[x] = (iTemp + xClip( (aiTemp[x+iDx] + 16) / 32 ) + 1) / 2;
      }
#else
      iTemp = xClip( (iTemp + 512) / 1024 );
      pucDest[x] = (iTemp + xClip( (aiTemp[x+iDx] + 16) / 32 ) + 1) / 2;
#endif
    }
    pucDest += iDestStride;
    pucSrc  += iSrcStride;
  }
}



Void QuarterPelFilter::xPredElse( XPel* pucDest, XPel* pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX )
{
  XPel* pucSrcX = pucSrc;
  XPel* pucSrcY = pucSrc;

  pucSrcY += (iDx == 1) ? 0 : 1;
  pucSrcX += (iDy == 1) ? 0 : iSrcStride;

  for( UInt y = 0; y < uiSizeY; y++)
  {
    for( UInt x = 0; x < uiSizeX; x++)
    {
      Int iTempX;
      iTempX  = pucSrcX[x - 0];
      iTempX += pucSrcX[x + 1];
      iTempX  = iTempX << 2;
      iTempX -= pucSrcX[x - 1];
      iTempX -= pucSrcX[x + 2];
      iTempX += iTempX << 2;
      iTempX += pucSrcX[x - 2];
      iTempX += pucSrcX[x + 3];
#if ! AR_FGS_COMPENSATE_SIGNED_FRAME
      iTempX = xClip( (iTempX + 16) / 32 );
#endif

      Int iTempY;
      iTempY  = pucSrcY[x - 0*iSrcStride];
      iTempY += pucSrcY[x + 1*iSrcStride];
      iTempY  = iTempY << 2;
      iTempY -= pucSrcY[x - 1*iSrcStride];
      iTempY -= pucSrcY[x + 2*iSrcStride];
      iTempY += iTempY << 2;
      iTempY += pucSrcY[x - 2*iSrcStride];
      iTempY += pucSrcY[x + 3*iSrcStride];
#if AR_FGS_COMPENSATE_SIGNED_FRAME
      if( ! m_bClip )
      {
        pucDest[x] = SIGNED_ROUNDING( iTempX + iTempY, 32, 6 );
      }
      else
      {
        iTempX = xClip( (iTempX + 16) / 32 );
        iTempY = xClip( (iTempY + 16) / 32 );
        pucDest[x] = (iTempX + iTempY + 1) >> 1;
      }
#else
      iTempY = xClip( (iTempY + 16) / 32 );
      pucDest[x] = (iTempX + iTempY + 1) >> 1;
#endif
    }
    pucDest += iDestStride;
    pucSrcX += iSrcStride;
    pucSrcY += iSrcStride;
  }
}


Void QuarterPelFilter::xUpdInterpBlnr(Int* pucDest, XPel* pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, 
                                    Int iSizeY, Int iSizeX )
{
  static int f2tap[4][2] = {
    { 4,  0}, 
    { 3,  1},
    { 2,  2},
    { 1,  3}
  };
  int sx1, sx2, sy1, sy2; 
  for( Int y = 0; y < iSizeY + 1; y++)
  {
    for( Int x = 0; x < iSizeX + 1; x++)
    {
      Int iTemp1[2], iTemp2;
      int i, j;
      sx1 = max(0, x + 1 - iSizeX);
      sx2 = min(2, x + 1);
      sy1 = max(0, y + 1 - iSizeY);
      sy2 = min(2, y + 1);

      for(i = sy1; i < sy2; i++)
      {
        iTemp1[i] = 0;
       
        for(j = sx1; j < sx2; j++ )
          iTemp1[i] += pucSrc[x - i*iSrcStride - j] * f2tap[iDx][j];
      }

      iTemp2 = 0;
      for(i = sy1; i < sy2; i++)
        iTemp2 += iTemp1[i] * f2tap[iDy][i];

      pucDest[x] = iTemp2;
    }
    pucDest += iDestStride;
    pucSrc += iSrcStride; 
  }
}


Void QuarterPelFilter::xUpdInterp4Tap(Int* pucDest, XPel* pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, 
                                    Int iSizeY, Int iSizeX )
{
  static int f4tap[4][4] = {
    { 0, 16,  0,  0}, 
    {-2, 14,  5, -1},
    {-2, 10, 10, -2},
    {-1,  5, 14, -2}
  };

  int sx1, sx2, sy1, sy2; 
  for( Int y = 0; y < iSizeY + 3; y++)
  {
    for( Int x = 0; x < iSizeX + 3; x++)
    {
      Int iTemp1[4], iTemp2;
      int i, j;
      sx1 = max(0, x + 1 - iSizeX);
      sx2 = min(4, x + 1);
      sy1 = max(0, y + 1 - iSizeY);
      sy2 = min(4, y + 1);

      for(i = sy1; i < sy2; i++)
      {
        iTemp1[i] = 0;
       
        for(j = sx1; j < sx2; j++ )
          iTemp1[i] += pucSrc[x - i*iSrcStride - j] * f4tap[iDx][j];
      }

      iTemp2 = 0;
      for(i = sy1; i < sy2; i++)
        iTemp2 += iTemp1[i] * f4tap[iDy][i];

      pucDest[x] = iTemp2;
    }
    pucDest += iDestStride;
    pucSrc += iSrcStride; 
  }
}

Void QuarterPelFilter::xUpdInterpChroma( Int* pucDest, Int iDestStride, XPel* pucSrc, Int iSrcStride, Mv cMv, Int iSizeY, Int iSizeX )
{
  Int iDx = (cMv.getHor() & 0x7);
  Int iDy = (cMv.getVer() & 0x7);

  static int f2tapC[8][2] = {
    { 8,  0}, 
    { 7,  1},
    { 6,  2},
    { 5,  3},
    { 4,  4}, 
    { 3,  5},
    { 2,  6},
    { 1,  7}
  };
  int sx1, sx2, sy1, sy2; 
  for( Int y = 0; y < iSizeY + 1; y++)
  {
    for( Int x = 0; x < iSizeX + 1; x++)
    {
      Int iTemp1[2], iTemp2;
      int i, j;
      sx1 = max(0, x + 1 - (int)iSizeX);
      sx2 = min(2, x + 1);
      sy1 = max(0, y + 1 - (int)iSizeY);
      sy2 = min(2, y + 1);

      for(i = sy1; i < sy2; i++)
      {
        iTemp1[i] = 0;
       
        for(j = sx1; j < sx2; j++ )
          iTemp1[i] += pucSrc[x - i*iSrcStride - j] * f2tapC[iDx][j];
      }

      iTemp2 = 0;
      for(i = sy1; i < sy2; i++)
        iTemp2 += iTemp1[i] * f2tapC[iDy][i];

      pucDest[x] = iTemp2;
    }
    pucDest += iDestStride;
    pucSrc += iSrcStride; 
  }
}

Void QuarterPelFilter::weightOnEnergy(UShort *usWeight, XPel* pucSrc, Int iSrcStride, Int iSizeY, Int iSizeX)
{
  Int iSSD = 0;
  int bitsShift = 8, i;

  for( Int y = 0; y < iSizeY; y++)
  {
    for( Int x = 0; x < iSizeX; x++)
    {
      Int iTemp;
      iTemp  = xClip( pucSrc[iSrcStride*y + x]);
      iSSD += iTemp*iTemp;
    }
  }


  for( i = (iSizeY/4)*(iSizeX/4); i > 1; i >>= 1 )
    bitsShift ++;

  iSSD                                = ( iSSD + (1 << (bitsShift-1)) ) >> bitsShift;
  *usWeight                           = (UShort) max( 0, min( 16, 20 - iSSD ) );
}

H264AVC_NAMESPACE_END
