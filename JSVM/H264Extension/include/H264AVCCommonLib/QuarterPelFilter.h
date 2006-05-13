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






#if !defined(AFX_INTERPOLATIONFILTER_H__79B49DF9_A41E_4043_AD0E_CAAC3A0A9DA1__INCLUDED_)
#define AFX_INTERPOLATIONFILTER_H__79B49DF9_A41E_4043_AD0E_CAAC3A0A9DA1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "H264AVCCommonLib/YuvMbBuffer.h"
#include "H264AVCCommonLib/YuvPicBuffer.h"
#include "H264AVCCommonLib/IntYuvMbBuffer.h"
#include "H264AVCCommonLib/IntYuvPicBuffer.h"

H264AVC_NAMESPACE_BEGIN

typedef Void (*FilterBlockFunc)( Pel* pDes, Pel* pSrc, Int iSrcStride, UInt uiXSize, UInt uiYSize );
typedef Void (*XFilterBlockFunc)( XPel* pDes, XPel* pSrc, Int iSrcStride, UInt uiXSize, UInt uiYSize );

class H264AVCCOMMONLIB_API QuarterPelFilter
{
protected:
	QuarterPelFilter();
	virtual ~QuarterPelFilter();

public:
  static ErrVal create( QuarterPelFilter*& rpcQuarterPelFilter );
  ErrVal destroy();
  virtual ErrVal init();
  ErrVal uninit();

  Void predBlkBilinear( IntYuvMbBuffer*     pcDesBuffer, IntYuvPicBuffer*     pcSrcBuffer, LumaIdx cIdx, Mv cMv, Int iSizeY, Int iSizeX);
  Void predBlk4Tap    ( IntYuvMbBuffer*     pcDesBuffer, IntYuvPicBuffer*     pcSrcBuffer, LumaIdx cIdx, Mv cMv, Int iSizeY, Int iSizeX);
  Bool getClipMode    ()        { return m_bClip; }
  Void setClipMode( Bool bEnableClip ) { m_bClip = bEnableClip; }

  virtual ErrVal filterFrame( IntYuvPicBuffer* pcPelBuffer, IntYuvPicBuffer* pcHalfPelBuffer );
  Void filterBlock( Pel* pDes, Pel* pSrc, Int iSrcStride, UInt uiXSize, UInt uiYSize, UInt uiFilter )
  {
    m_afpFilterBlockFunc[uiFilter]( pDes, pSrc, iSrcStride, uiXSize, uiYSize );
  }
  Void filterBlock( XPel* pDes, XPel* pSrc, Int iSrcStride, UInt uiXSize, UInt uiYSize, UInt uiFilter )
  {
    m_afpXFilterBlockFunc[uiFilter]( pDes, pSrc, iSrcStride, uiXSize, uiYSize );
  }


  Void predBlk( YuvMbBuffer* pcDesBuffer, YuvPicBuffer* pcSrcBuffer, LumaIdx cIdx, Mv cMv, Int iSizeY, Int iSizeX);
  Void predBlk( IntYuvMbBuffer*     pcDesBuffer, IntYuvPicBuffer*     pcSrcBuffer, LumaIdx cIdx, Mv cMv, Int iSizeY, Int iSizeX);

  Void weightOnEnergy(UShort *usWeight, XPel* pucSrc, Int iSrcStride, Int iSizeY, Int iSizeX );
  Void xUpdInterpBlnr(Int* pucDest, XPel* pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, 
                                    UInt uiSizeY, UInt uiSizeX );
  Void xUpdInterp4Tap(Int* pucDest, XPel* pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, 
                                    UInt uiSizeY, UInt uiSizeX );
  Void xUpdInterpChroma( Int* pucDest, Int iDestStride, XPel* pucSrc, Int iSrcStride, Mv cMv, Int iSizeY, Int iSizeX );

protected:
  virtual Void xPredElse( Pel*  pucDest, Pel*  pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX );
  virtual Void xPredDy2Dx13( Pel*  pucDest, Pel*  pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX );
  virtual Void xPredDx2Dy13( Pel*  pucDest, Pel*  pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX );
  virtual Void xPredDx2Dy2( Pel*  pucDest, Pel*  pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX );
  virtual Void xPredDx0Dy13( Pel*  pucDest, Pel*  pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX );
  virtual Void xPredDx0Dy2( Pel*  pucDest, Pel*  pucSrc, Int iDestStride, Int iSrcStride, UInt uiSizeY, UInt uiSizeX );
  virtual Void xPredDy0Dx13( Pel*  pucDest, Pel*  pucSrc, Int iDestStride, Int iSrcStride, Int iDx, UInt uiSizeY, UInt uiSizeX );
  virtual Void xPredDy0Dx2( Pel*  pucDest, Pel*  pucSrc, Int iDestStride, Int iSrcStride, UInt uiSizeY, UInt uiSizeX );
  
  virtual Void xPredDx2( Short* psDest, Pel*  pucSrc, Int iSrcStride, UInt uiSizeY, UInt uiSizeX );

  Void xPredElse    ( XPel*  pucDest, XPel*  pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX );
  Void xPredDy2Dx13 ( XPel*  pucDest, XPel*  pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX );
  Void xPredDx2Dy13 ( XPel*  pucDest, XPel*  pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX );
  Void xPredDx2Dy2  ( XPel*  pucDest, XPel*  pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX );
  Void xPredDx0Dy13 ( XPel*  pucDest, XPel*  pucSrc, Int iDestStride, Int iSrcStride, Int iDx, Int iDy, UInt uiSizeY, UInt uiSizeX );
  Void xPredDx0Dy2  ( XPel*  pucDest, XPel*  pucSrc, Int iDestStride, Int iSrcStride, UInt uiSizeY, UInt uiSizeX );
  Void xPredDy0Dx13 ( XPel*  pucDest, XPel*  pucSrc, Int iDestStride, Int iSrcStride, Int iDx, UInt uiSizeY, UInt uiSizeX );
  Void xPredDy0Dx2  ( XPel*  pucDest, XPel*  pucSrc, Int iDestStride, Int iSrcStride, UInt uiSizeY, UInt uiSizeX );
  Void xPredDx2     ( XXPel* psDest,  XPel*  pucSrc, Int iSrcStride, UInt uiSizeY,   UInt uiSizeX );


  static Void xFilter1( Pel* pDes, Pel* pSrc, Int iSrcStride, UInt uiXSize, UInt uiYSize );
  static Void xFilter2( Pel* pDes, Pel* pSrc, Int iSrcStride, UInt uiXSize, UInt uiYSize );
  static Void xFilter3( Pel* pDes, Pel* pSrc, Int iSrcStride, UInt uiXSize, UInt uiYSize );
  static Void xFilter4( Pel* pDes, Pel* pSrc, Int iSrcStride, UInt uiXSize, UInt uiYSize );

  static Void xXFilter1( XPel* pDes, XPel* pSrc, Int iSrcStride, UInt uiXSize, UInt uiYSize );
  static Void xXFilter2( XPel* pDes, XPel* pSrc, Int iSrcStride, UInt uiXSize, UInt uiYSize );
  static Void xXFilter3( XPel* pDes, XPel* pSrc, Int iSrcStride, UInt uiXSize, UInt uiYSize );
  static Void xXFilter4( XPel* pDes, XPel* pSrc, Int iSrcStride, UInt uiXSize, UInt uiYSize );

  Int xClip( Int iPel ) { return ( m_bClip ? gClip( iPel ) : iPel); }

protected:
  Bool m_bClip;
  FilterBlockFunc m_afpFilterBlockFunc[4];
  XFilterBlockFunc m_afpXFilterBlockFunc[4];
};

#if AR_FGS_COMPENSATE_SIGNED_FRAME
#define SIGNED_ROUNDING(x, o, s)    ( ( (x) >= 0 ) ? ( ( (x) + (o) ) >> s ) : -( ( -(x) + (o) ) >> s ) )
#endif

H264AVC_NAMESPACE_END

#endif // !defined(AFX_INTERPOLATIONFILTER_H__79B49DF9_A41E_4043_AD0E_CAAC3A0A9DA1__INCLUDED_)
