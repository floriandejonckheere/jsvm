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
#include "H264AVCCommonLib/YuvBufferCtrl.h"
#include "H264AVCCommonLib/YuvPicBuffer.h"
#include "H264AVCCommonLib/YuvMbBuffer.h"
#include "H264AVCCommonLib/IntYuvMbBuffer.h"

H264AVC_NAMESPACE_BEGIN

YuvPicBuffer::YuvPicBuffer( YuvBufferCtrl& rcYuvBufferCtrl ):
  m_rcBufferParam   ( rcYuvBufferCtrl.getBufferParameter() ),
  m_rcYuvBufferCtrl ( rcYuvBufferCtrl ),
  m_iStride         ( 0 ),
  m_pPelCurr        ( NULL ),
  m_pucYuvBuffer    ( NULL ),
  m_pucOwnYuvBuffer ( NULL )
{
}


YuvPicBuffer::~YuvPicBuffer()
{
}



ErrVal YuvPicBuffer::init( Pel*& rpucYuvBuffer )
{
  ROT( NULL != m_pucYuvBuffer );
  ROF( m_rcYuvBufferCtrl.isInitDone() )
  m_iStride = m_rcBufferParam.getStride();

  UInt uiSize;
  RNOK( m_rcYuvBufferCtrl.getChromaSize( uiSize ) );

  if( NULL == rpucYuvBuffer )
  {
    m_pucOwnYuvBuffer = new Pel[ 6 * uiSize ];
    ROT( NULL == m_pucOwnYuvBuffer );
    rpucYuvBuffer = m_pucYuvBuffer = m_pucOwnYuvBuffer;
  }
  else
  {
    m_pucOwnYuvBuffer = NULL;
    m_pucYuvBuffer = rpucYuvBuffer;
  }

  m_pucYuvBuffer[(6 * uiSize)-1] = 0xde;

  m_rcYuvBufferCtrl.initMb();
  return Err::m_nOK;
}



ErrVal YuvPicBuffer::uninit()
{
  if( m_pucOwnYuvBuffer )
  {
    delete [] m_pucOwnYuvBuffer;
  }
  m_pucYuvBuffer    = NULL;
  m_pucOwnYuvBuffer = NULL;
  m_pPelCurr = NULL;
  m_iStride = 0;

  return Err::m_nOK;
}

ErrVal YuvPicBuffer::loadBuffer( YuvMbBuffer *pcYuvMbBuffer )
{
  Pel   *pDes       = getMbLumAddr();
  Pel   *pScr       = pcYuvMbBuffer->getMbLumAddr();
  Int   iSrcStride  = pcYuvMbBuffer->getLStride();
  Int   iDesStride  = getLStride();
  UInt  y;

  for( y = 0; y < 16; y++ )
  {
    ::memcpy( pDes, pScr, 16* sizeof(Pel) );
    pDes += iDesStride,
    pScr += iSrcStride;
  }

  iDesStride  >>= 1;
  iSrcStride  = pcYuvMbBuffer->getCStride();
  pScr = pcYuvMbBuffer->getMbCbAddr();
  pDes = getMbCbAddr();

  for( y = 0; y < 8; y++ )
  {
    ::memcpy( pDes, pScr, 8* sizeof(Pel) );
    pDes += iDesStride,
    pScr += iSrcStride;
  }

  pScr = pcYuvMbBuffer->getMbCrAddr();
  pDes = getMbCrAddr();

  for( y = 0; y < 8; y++ )
  {
    ::memcpy( pDes, pScr, 8* sizeof(Pel) );
    pDes += iDesStride,
    pScr += iSrcStride;
  }

  return Err::m_nOK;
}

// HS: decoder robustness
ErrVal
YuvPicBuffer::copy( YuvPicBuffer* pcPicBuffer )
{
  Pel   *pDes   = getLumOrigin();
  Pel   *pScr   = pcPicBuffer->getMbLumAddr();
  Int   iStride = getLStride();
  Int   iWidth  = getLWidth();
  Int   iHeight = getLHeight();
  UInt  y;

  for( y = 0; y < (UInt)iHeight; y++ )
  {
    ::memcpy( pDes, pScr, iWidth* sizeof(Pel) );
    pDes += iStride,
    pScr += iStride;
  }

  iWidth >>= 1;
  iHeight >>=1;
  iStride  >>= 1;
  pScr = pcPicBuffer->getCbOrigin();
  pDes = getCbOrigin();

  for( y = 0; y < (UInt)iHeight; y++ )
  {
    ::memcpy( pDes, pScr, iWidth* sizeof(Pel) );
    pDes += iStride,
    pScr += iStride;
  }

  pScr = pcPicBuffer->getCrOrigin();
  pDes = getCrOrigin();

  for( y = 0; y < (UInt)iHeight; y++ )
  {
    ::memcpy( pDes, pScr, iWidth* sizeof(Pel) );
    pDes += iStride,
    pScr += iStride;
  }

  return Err::m_nOK;
}

ErrVal YuvPicBuffer::fillMargin()
{
  m_rcYuvBufferCtrl.initMb();

  xFillPlaneMargin( getMbLumAddr(), getLHeight(), getLWidth(), getLStride(), getLXMargin(), getLYMargin() );
  xFillPlaneMargin( getMbCbAddr(),  getCHeight(), getCWidth(), getCStride(), getCXMargin(), getCYMargin() );
  xFillPlaneMargin( getMbCrAddr(),  getCHeight(), getCWidth(), getCStride(), getCXMargin(), getCYMargin() );

  return Err::m_nOK;
}


Void YuvPicBuffer::xFillPlaneMargin( Pel *pucDest, Int iHeight, Int iWidth, Int iStride, Int iXMargin, Int iYMargin )
{
  Pel* puc;
  Int n;

  // left and right borders at once
  puc = pucDest;
  for( n = 0; n < iHeight; n++)
  {
    // left border lum
    ::memset( puc - iXMargin, puc[0],         iXMargin );
    // right border lum
    ::memset( puc + iWidth,  puc[iWidth - 1], iXMargin );
    puc += iStride;
  }

  // bot border lum
  puc = pucDest - iXMargin + iStride * iHeight;
  UInt uiSize = iWidth + 2*iXMargin;
  for( n = 0; n < iYMargin; n++)
  {
    ::memcpy( puc, puc - iStride, uiSize );
    puc += iStride;
  }

  // top border lum
  puc = pucDest - iXMargin;
  for( n = 0; n < iYMargin; n++)
  {
    ::memcpy( puc - iStride, puc, uiSize );
    puc -= iStride;
  }
}




ErrVal YuvPicBuffer::loadBuffer( YuvPicBuffer *pcSrcYuvPicBuffer )
{
  pcSrcYuvPicBuffer->m_rcYuvBufferCtrl.initMb();
  m_rcYuvBufferCtrl.initMb();

  Pel   *pDes       = getMbLumAddr();
  Pel   *pScr       = pcSrcYuvPicBuffer->getMbLumAddr();
  Int   iSrcStride  = pcSrcYuvPicBuffer->getLStride();
  Int   iDesStride  = getLStride();
  UInt  uiHeight    = getLHeight();
  UInt  uiSize      = getLWidth() * sizeof(Pel);
  UInt  y;

  for( y = 0; y < uiHeight; y++ )
  {
    ::memcpy( pDes, pScr, uiSize );
    pDes += iDesStride,
    pScr += iSrcStride;
  }

  iDesStride  >>= 1;
  iSrcStride  >>= 1;
  uiSize      >>= 1;
  pScr = pcSrcYuvPicBuffer->getMbCbAddr();
  pDes = getMbCbAddr();

  for( y = 0; y < uiHeight; y+=2 )
  {
    ::memcpy( pDes, pScr, uiSize );
    pDes += iDesStride,
    pScr += iSrcStride;
  }

  pScr = pcSrcYuvPicBuffer->getMbCrAddr();
  pDes = getMbCrAddr();

  for( y = 0; y < uiHeight; y+=2 )
  {
    ::memcpy( pDes, pScr, uiSize );
    pDes += iDesStride,
    pScr += iSrcStride;
  }

  return Err::m_nOK;
}
//TMM_EC }}
H264AVC_NAMESPACE_END
