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
/**************************************************************************

File:        WriteYuvaToRgb.cpp

Author:      Tobias Hinz, Karsten Suehring

email:       hinz@hhi.de, suehring@hhi.de

Copyright:   2002 Heinrich-Hertz-Institut Berlin GmbH
             The copyright of this software source code
             is the property of Heinrich-Hertz Institut (HHI).
             The program(s) may be used and/or copied only with
             the written permission of Heinrich-Hertz-Institut
             and in accordance with the terms and conditions
             stipulated in the agreement/contract under which
             the software has been supplied.

Description: implementation for the WriteYuvaToRgb class.

Modify list:

**************************************************************************/

#include "H264AVCVideoIoLib.h"
#include "WriteYuvaToRgb.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

__inline
Int Clip( const Int iX )
{
  const Int i2 = (iX & 0xFF);
  if( i2 == iX )  { return iX; }
  if( iX < 0 ) { return 0x00; }
  else { return 0xFF; }
}

#define dematrix R = Clip(((Y *37 + V * 51 ) >> 5) - 223); G = Clip(((Y *37 - U * 13 - V * 26 ) >> 5) + 135); B = Clip(((Y *37 + U * 65) >> 5) - 277);

#define pack32 ((R << 16) | (G << 8) | B)
#define pack15 (((R >> 3) << 10 ) | ((G >> 3) << 5 ) | (B >> 3) )
#define pack16 (((R >> 3) << 11 ) | ((G >> 2) << 5 ) | (B >> 3) )


WriteYuvaToRgb::WriteYuvaToRgb()
{
}

WriteYuvaToRgb::~WriteYuvaToRgb()
{
}



ErrVal WriteYuvaToRgb::create( WriteYuvaToRgb*& rpcWriteYuvaToRgb )
{
  rpcWriteYuvaToRgb = new WriteYuvaToRgb;

  ROT( NULL == rpcWriteYuvaToRgb );
  
  return Err::m_nOK;
}

ErrVal WriteYuvaToRgb::destroy()
{ 
  delete this;

  return Err::m_nOK; 
}

ErrVal WriteYuvaToRgb::setFrameDimension( UInt uiLumHeight, UInt uiLumWidth )
{
  m_uiHeight = uiLumHeight; 
  m_uiWidth =  uiLumWidth;
  return Err::m_nOK; 
}

ErrVal WriteYuvaToRgb::writeFrameRGB( UChar *pucRGB,
                                      UInt uiDestStride,
                                      const UChar *pLum,
                                      const UChar *pCb,
                                      const UChar *pCr,
                                      UInt uiLumHeight,
                                      UInt uiLumWidth,
                                      UInt uiLumStride )

{
  CHECK( pLum );
  CHECK( pCb );
  CHECK( pCr );


	const UChar *pu, *pv, *py;
  UInt  column, row;
	Int   Y, U, V, R, G, B;
  UInt  *argb;
  UChar *pucDest = pucRGB;
  Int   iWidth = uiDestStride; 

  if( uiLumHeight > m_uiHeight)
  {
    uiLumHeight = m_uiHeight;
  }

  if( uiLumWidth > m_uiWidth)
  {
    uiLumWidth = m_uiWidth;
  }


  for( row = 0; row < uiLumHeight; row++)
	{
	  argb = (UInt*)pucDest;
    pucDest += iWidth;
		pu	 = pCb + (row>>1) * (uiLumStride >> 1);
		pv	 = pCr + (row>>1) * (uiLumStride >> 1);
    py   = pLum +  row * uiLumStride;

		for( column = 0; column < uiLumWidth; column+= 2)
		{
			Y = *py++;
			U = *pu++;
			V = *pv++;

  		dematrix
      *argb++ = pack32;

			Y = *py++;

			dematrix
			*argb++ = pack32;
		}
	}

  return Err::m_nOK;
}


ErrVal WriteYuvaToRgb::writeFrameYV12( UChar *pucDest,
                                       UInt uiDestStride,
                                       const UChar *pLum,
                                       const UChar *pCb,
                                       const UChar *pCr,
                                       UInt uiLumHeight,
                                       UInt uiLumWidth,
                                       UInt uiLumStride )

{
  CHECK( pLum );
  CHECK( pCb );
  CHECK( pCr );


  UInt *pDest = (UInt*)pucDest;
  UInt  row;
  Int   iWidth = uiDestStride; 

  if( uiLumHeight > m_uiHeight)
  {
    uiLumHeight = m_uiHeight;
  }

  if( uiLumWidth > m_uiWidth)
  {
    uiLumWidth = m_uiWidth;
  }

  UInt uiStride = uiLumStride;
  UInt uiWidth  = uiLumWidth;
  UInt uiHeight = uiLumHeight;

  {
    const UChar* pSrc = pLum;
    UChar* pDes = (UChar*)pDest;
    for( row = 0; row < uiHeight; row++)
	  {
      memcpy( pDes, pSrc, uiWidth);
      pSrc += uiStride;
      pDes += iWidth;
	  }
  }

  uiStride >>= 1;
  uiWidth  >>= 1;
  uiHeight >>= 1;
  {
    const UChar* pSrc1 = pCb;
    const UChar* pSrc2 = pCr;
    UChar* pDes = (UChar*)pDest + 2*uiHeight * uiDestStride;

    memset( pDes, 0x80, 2*uiHeight * uiDestStride/2 );

    for( row = 0; row < uiHeight; row++)
	  {
      memcpy( pDes, pSrc2, uiWidth);
      pDes += uiDestStride/2;
      pSrc2 += uiStride;
	  }

    for( row = 0; row < uiHeight; row++)
	  {
      memcpy( pDes, pSrc1, uiWidth);
      pDes += uiDestStride/2;
      pSrc1 += uiStride;
    }
  }

  return Err::m_nOK;
}

ErrVal WriteYuvaToRgb::writeFrameYUYV( UChar* pucYUYV,
                                       UInt uiDestStride,
                                       const UChar *pLum,
                                       const UChar *pCb,
                                       const UChar *pCr,
                                       UInt uiLumHeight,
                                       UInt uiLumWidth,
                                       UInt uiLumStride )

{
  CHECK( pLum );
  CHECK( pCb );
  CHECK( pCr );


  UChar *pucDest = pucYUYV;
	const UChar *pu, *pv, *py;
  UInt  column, row;
  Int   iWidth = uiDestStride; 

  if( uiLumHeight > m_uiHeight)
  {
    uiLumHeight = m_uiHeight;
  }

  if( uiLumWidth > m_uiWidth)
  {
    uiLumWidth = m_uiWidth;
  }

  for( row = 0; row < uiLumHeight; row++)
	{
    UChar* Yuvy = pucDest;
    pucDest += iWidth;
		pu	 = pCb + (row>>1) * (uiLumStride >> 1);
		pv	 = pCr + (row>>1) * (uiLumStride >> 1);
    py   = pLum +  row * uiLumStride;

		for( column = 0; column < uiLumWidth; column+= 2)
		{
      Yuvy[0] = *py++;
      Yuvy[2] = *py++;
			Yuvy[1] = *pu++;
			Yuvy[3] = *pv++;
      Yuvy += 4;
		}
	}

  return Err::m_nOK;
}


