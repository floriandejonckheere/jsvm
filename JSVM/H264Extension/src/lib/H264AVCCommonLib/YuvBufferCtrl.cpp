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

H264AVC_NAMESPACE_BEGIN

YuvBufferCtrl::YuvBufferCtrl() :
  m_uiChromaSize  ( 0 ),
  m_iResolution   ( 0 ),
  m_bInitDone     ( false )
{
}

YuvBufferCtrl::~YuvBufferCtrl()
{
}

ErrVal YuvBufferCtrl::getChromaSize( UInt& ruiChromaSize )
{
  ROF( m_bInitDone );
  ruiChromaSize = m_uiChromaSize;
  return Err::m_nOK;
}

ErrVal YuvBufferCtrl::create( YuvBufferCtrl*& rpcYuvBufferCtrl )
{
  rpcYuvBufferCtrl = new YuvBufferCtrl;

  ROT( NULL == rpcYuvBufferCtrl );

  return Err::m_nOK;
}


ErrVal YuvBufferCtrl::destroy()
{
  delete this;

  return Err::m_nOK;
}



ErrVal YuvBufferCtrl::initMb( UInt uiMbY, UInt uiMbX )
{
  ROF( m_bInitDone );

  UInt uiXPos     = (uiMbX<<3) << m_iResolution;
  UInt uiYPos     = (uiMbY<<3) << m_iResolution;
  UInt uiYPosFld  = (uiMbY<<3) << m_iResolution;
  Int  iStride    = m_cBufferParam.m_iStride;

  m_cBufferParam.m_uiCbOffset   = m_uiCbBaseOffset + uiXPos + uiYPos    * iStride / 2;
  m_cBufferParam.m_uiCrOffset   = m_cBufferParam.m_uiCbOffset + m_uiChromaSize;

  uiXPos    <<= 1;
  uiYPos    <<= 1;
  uiYPosFld <<= 1;

  m_cBufferParam.m_uiLumOffset  = m_uiLumBaseOffset + uiXPos + uiYPos    * iStride;

  return Err::m_nOK;
}


ErrVal YuvBufferCtrl::initSlice( UInt uiYFrameSize, UInt uiXFrameSize, UInt uiYMarginSize, UInt uiXMarginSize, UInt uiResolution )
{
  ROT( 0 ==  uiYFrameSize );
  ROT( 0 != (uiYFrameSize&0xf) );
  ROT( 0 ==  uiXFrameSize );
  ROT( 0 != (uiXFrameSize&0xf) );
  ROT( 2 < uiResolution );
  ROT( 1 & uiXMarginSize );


  uiYFrameSize  <<= uiResolution;
  uiXFrameSize  <<= uiResolution;
  uiYMarginSize <<= uiResolution;
  uiXMarginSize <<= uiResolution;

  m_uiXMargin = uiXMarginSize;
  m_uiYMargin = uiYMarginSize/2;

  m_cBufferParam.m_iHeight     = uiYFrameSize;
  m_cBufferParam.m_iStride     = uiXFrameSize   + 2*uiXMarginSize;
  m_cBufferParam.m_iWidth      = uiXFrameSize;
  m_cBufferParam.m_iResolution = uiResolution;

  m_iResolution   = uiResolution;
  m_uiChromaSize  = ((uiYFrameSize >> 1) + uiYMarginSize)
                  * ((uiXFrameSize >> 1) + uiXMarginSize);

  m_uiLumBaseOffset = (m_cBufferParam.m_iStride) * uiYMarginSize + uiXMarginSize;
  m_uiCbBaseOffset  = (m_cBufferParam.m_iStride/2) * uiYMarginSize/2 + uiXMarginSize/2;
  m_uiCbBaseOffset += 4*m_uiChromaSize;
  m_bInitDone = true;

  return Err::m_nOK;
}

ErrVal YuvBufferCtrl::initSPS( UInt uiYFrameSize, UInt uiXFrameSize, UInt uiYMarginSize, UInt uiXMarginSize, UInt uiResolution )
{
  ROT( 0 ==  uiYFrameSize );
  ROT( 0 != (uiYFrameSize&0xf) );
  ROT( 0 ==  uiXFrameSize );
  ROT( 0 != (uiXFrameSize&0xf) );
  ROT( 2 < uiResolution );
  ROT( 1 & uiXMarginSize );


  uiYFrameSize  <<= uiResolution;
  uiXFrameSize  <<= uiResolution;
  uiYMarginSize <<= uiResolution;
  uiXMarginSize <<= uiResolution;

  m_uiXMargin = uiXMarginSize;
  m_uiYMargin = uiYMarginSize/2;

  m_cBufferParam.m_iHeight     = uiYFrameSize;
  m_cBufferParam.m_iStride     = uiXFrameSize   + 2*uiXMarginSize;
  m_cBufferParam.m_iWidth      = uiXFrameSize;
  m_cBufferParam.m_iResolution = uiResolution;

  m_iResolution   = uiResolution;
  m_uiChromaSize  = ((uiYFrameSize >> 1) + uiYMarginSize)
                  * ((uiXFrameSize >> 1) + uiXMarginSize);

  m_uiLumBaseOffset = (m_cBufferParam.m_iStride) * uiYMarginSize + uiXMarginSize;
  m_uiCbBaseOffset  = (m_cBufferParam.m_iStride/2) * uiYMarginSize/2 + uiXMarginSize/2;
  m_uiCbBaseOffset += 4*m_uiChromaSize;
  m_bInitDone = true;

  return Err::m_nOK;
}


ErrVal YuvBufferCtrl::uninit()
{
  m_bInitDone = false;
  return Err::m_nOK;
}



H264AVC_NAMESPACE_END
