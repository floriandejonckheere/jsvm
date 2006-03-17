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






#if !defined(AFX_SAMPLEWEIGHTING_H__6B0A73D2_EAF3_497F_B114_913D68E1C1C0__INCLUDED_)
#define AFX_SAMPLEWEIGHTING_H__6B0A73D2_EAF3_497F_B114_913D68E1C1C0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include  "H264AVCCommonLib/YuvMbBuffer.h"
#include  "H264AVCCommonLib/IntYuvMbBuffer.h"

H264AVC_NAMESPACE_BEGIN

class H264AVCCOMMONLIB_API SampleWeighting 
{
  typedef Void (*MixSampleFunc) ( Pel* pucDest,  Int iDestStride, Pel*  pucSrc, Int iSrcStride, Int iSizeY );
  typedef Void (*XMixSampleFunc)( XPel* pucDest, Int iDestStride, XPel* pucSrc, Int iSrcStride, Int iSizeY );

protected:
  SampleWeighting();
  virtual ~SampleWeighting()  {}

public:
  static ErrVal create( SampleWeighting*& rpcSampleWeighting );
  ErrVal destroy();
  virtual ErrVal init();
  ErrVal uninit();

  ErrVal initSlice( const SliceHeader& rcSliceHeader );

  Void getTargetBuffers( YuvMbBuffer*    apcTarBuffer[2], YuvMbBuffer*    pcRecBuffer, const PW* pcPW0, const PW* pcPW1 );
  Void getTargetBuffers( IntYuvMbBuffer* apcTarBuffer[2], IntYuvMbBuffer* pcRecBuffer, const PW* pcPW0, const PW* pcPW1 );

  Void weightLumaSamples(   YuvMbBuffer*    pcRecBuffer, Int iSizeX, Int iSizeY, LumaIdx cIdx, const PW* pcPW0, const PW* pcPW1 );
  Void weightChromaSamples( YuvMbBuffer*    pcRecBuffer, Int iSizeX, Int iSizeY, LumaIdx cIdx, const PW* pcPW0, const PW* pcPW1 );

  Void weightLumaSamples(   IntYuvMbBuffer* pcRecBuffer, Int iSizeX, Int iSizeY, LumaIdx cIdx, const PW* pcPW0, const PW* pcPW1 );
  Void weightChromaSamples( IntYuvMbBuffer* pcRecBuffer, Int iSizeX, Int iSizeY, LumaIdx cIdx, const PW* pcPW0, const PW* pcPW1 );

  //===== for motion estimation of bi-predicted blocks with standard weights =====
  Void inverseLumaSamples  ( IntYuvMbBuffer* pcDesBuffer, IntYuvMbBuffer* pcOrgBuffer, IntYuvMbBuffer* pcFixBuffer, Int iYSize, Int iXSize );

  //===== for motion estimation of bi-predicted blocks with non-standard weights =====
  Void weightInverseLumaSamples  ( IntYuvMbBuffer* pcDesBuffer, IntYuvMbBuffer* pcOrgBuffer, IntYuvMbBuffer* pcFixBuffer, const PW* pcSearchPW, const PW* pcFixPW, Double&  rdWeight, Int iYSize, Int iXSize );

  //===== for motion estimation of unidirectional predicted blocks with non-standard weights =====
  Void weightInverseLumaSamples  ( IntYuvMbBuffer* pcDesBuffer, IntYuvMbBuffer* pcOrgBuffer, const PW* pcPW, Double&  rdWeight, Int iYSize, Int iXSize );
  Void weightInverseChromaSamples( IntYuvMbBuffer* pcDesBuffer, IntYuvMbBuffer* pcOrgBuffer, const PW* pcPW, Double* padWeight, Int iYSize, Int iXSize );

//TMM_WP
  ErrVal initSliceForWeighting( const SliceHeader& rcSliceHeader);

//TMM_WP
  
protected:
  Void xMixB      ( Pel*  pucDest, Int iDestStride, Pel*  pucSrc, Int iSrcStride, Int iSizeY, Int iSizeX );
  Void xMixB      ( XPel* pucDest, Int iDestStride, XPel* pucSrc, Int iSrcStride, Int iSizeY, Int iSizeX );
  Void xMixBWeight( Pel*  pucDest, Int iDestStride, Pel*  pucSrc, Int iSrcStride, Int iSizeY, Int iSizeX, Int iWD, Int iWS, Int iOffset, UInt uiDenom );
  Void xMixBWeight( XPel* pucDest, Int iDestStride, XPel* pucSrc, Int iSrcStride, Int iSizeY, Int iSizeX, Int iWD, Int iWS, Int iOffset, UInt uiDenom );
  Void xWeight    ( Pel*  pucDest, Int iDestStride,                               Int iSizeY, Int iSizeX, Int iWeight,      Int iOffset, UInt uiDenom );
  Void xWeight    ( XPel* pucDest, Int iDestStride,                               Int iSizeY, Int iSizeX, Int iWeight,      Int iOffset, UInt uiDenom );

private:
  static Void xMixB16x( Pel* pucDest, Int iDestStride, Pel* pucSrc, Int iSrcStride, Int iSizeY );
  static Void xMixB8x ( Pel* pucDest, Int iDestStride, Pel* pucSrc, Int iSrcStride, Int iSizeY );
  static Void xMixB4x ( Pel* pucDest, Int iDestStride, Pel* pucSrc, Int iSrcStride, Int iSizeY );
  static Void xXMixB16x( XPel* pucDest, Int iDestStride, XPel* pucSrc, Int iSrcStride, Int iSizeY );
  static Void xXMixB8x ( XPel* pucDest, Int iDestStride, XPel* pucSrc, Int iSrcStride, Int iSizeY );
  static Void xXMixB4x ( XPel* pucDest, Int iDestStride, XPel* pucSrc, Int iSrcStride, Int iSizeY );

protected:
  MixSampleFunc m_afpMixSampleFunc[5];
  XMixSampleFunc m_afpXMixSampleFunc[5];

private:
  YuvMbBuffer     m_cYuvBiBuffer;
  IntYuvMbBuffer  m_cIntYuvBiBuffer;
  UInt            m_uiLumaLogWeightDenom;
  UInt            m_uiChromaLogWeightDenom;
  Bool            m_bExplicit;
  Bool            m_bWeightedPredDisableP;
  Bool            m_bWeightedPredDisableB;
};


H264AVC_NAMESPACE_END


#endif // !defined(AFX_SAMPLEWEIGHTING_H__6B0A73D2_EAF3_497F_B114_913D68E1C1C0__INCLUDED_)
