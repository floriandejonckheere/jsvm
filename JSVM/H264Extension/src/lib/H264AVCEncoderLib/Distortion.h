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




#if !defined(AFX_DISTORTION_H__7052DDA3_6AD5_4BD5_88D0_E34F8BF08D45__INCLUDED_)
#define AFX_DISTORTION_H__7052DDA3_6AD5_4BD5_88D0_E34F8BF08D45__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DistortionIf.h"
#include "H264AVCCommonLib/YuvMbBuffer.h"
#include "H264AVCCommonLib/IntYuvMbBuffer.h"

#define Abs(x) abs(x)



H264AVC_NAMESPACE_BEGIN


#if defined( MSYS_WIN32 )
#pragma warning( disable: 4275 )
#endif


class  XDistortion : public XDistortionIf
{
protected:
	XDistortion();
	virtual ~XDistortion();

public:

  IntYuvMbBuffer* getYuvMbBuffer() { return &m_cOrgData; }

  static  ErrVal  create ( XDistortion*& rpcDistortion );
  virtual ErrVal  destroy();

  virtual ErrVal  init   ();
  virtual ErrVal  uninit () { return Err::m_nOK; }


  Int getBlockHeight( UInt uiMode )   { return  m_aiRows[ uiMode ]; }
  Int getBlockWidth( UInt uiMode )    { return  m_aiCols[ uiMode ]; }

  UInt    get8x8Cb   ( XPel *pPel, Int iStride, DFunc eDFunc = DF_SSD );
  UInt    get8x8Cr   ( XPel *pPel, Int iStride, DFunc eDFunc = DF_SSD );
  UInt    getLum4x4  ( XPel *pPel, Int iStride, DFunc eDFunc = DF_SSD );
  UInt    getLum16x16( XPel *pPel, Int iStride, DFunc eDFunc = DF_SSD );
  UInt    getLum8x8  ( XPel *pPel, Int iStride, DFunc eDFunc = DF_SSD );


  Void    loadOrgMbPelData( IntYuvPicBuffer* pcOrgYuvBuffer, IntYuvMbBuffer*& rpcOrgMbBuffer );

  Void set4x4Block( LumaIdx cIdx )
  {
    m_cOrgData.set4x4Block( cIdx );
  }

  Void getDistStruct( UInt uiBlkMode, DFunc eDFunc, Bool bBiDirectional, XDistSearchStruct& rDistSearchStruct )
  {
    rDistSearchStruct.Func    = m_aaafpDistortionFunc[(bBiDirectional?1:0)][eDFunc][uiBlkMode];
    rDistSearchStruct.iRows   = m_aiRows[uiBlkMode];
    rDistSearchStruct.pYOrg   = m_cOrgData.getLumBlk();
    rDistSearchStruct.pUOrg   = m_cOrgData.getCbBlk ();
    rDistSearchStruct.pVOrg   = m_cOrgData.getCrBlk ();
    DO_DBG( rDistSearchStruct.pYSearch = NULL );
    DO_DBG( rDistSearchStruct.iYStride = 0    );
    DO_DBG( rDistSearchStruct.pUSearch = NULL );
    DO_DBG( rDistSearchStruct.pVSearch = NULL );
    DO_DBG( rDistSearchStruct.iCStride = 0    );
  }

//TMM_WP
  ErrVal getLumaWeight( IntYuvPicBuffer* pcOrgPicBuffer, IntYuvPicBuffer* pcRefPicBuffer, Double& rfWeight, UInt uiLumaLog2WeightDenom );
  ErrVal getChromaWeight( IntYuvPicBuffer* pcOrgPicBuffer, IntYuvPicBuffer* pcRefPicBuffer, Double& rfWeight, UInt uiChromaLog2WeightDenom, Bool bCb );
  ErrVal getLumaOffsets( IntYuvPicBuffer* pcOrgPicBuffer, 
                         IntYuvPicBuffer* pcRefPicBuffer, Double& rfOffset );
  ErrVal getChromaOffsets( IntYuvPicBuffer* pcOrgPicBuffer, 
                           IntYuvPicBuffer* pcRefPicBuffer, 
                           Double& rfOffset, Bool bCb );
//TMM_WP

private:
  static UInt xGetSAD16x          ( XDistSearchStruct* pcDSS );
  static UInt xGetSAD8x           ( XDistSearchStruct* pcDSS );
  static UInt xGetSAD4x           ( XDistSearchStruct* pcDSS );

  static UInt xGetSSE16x          ( XDistSearchStruct* pcDSS );
  static UInt xGetSSE8x           ( XDistSearchStruct* pcDSS );
  static UInt xGetSSE4x           ( XDistSearchStruct* pcDSS );

  static UInt xGetHAD16x          ( XDistSearchStruct* pcDSS );
  static UInt xGetHAD8x           ( XDistSearchStruct* pcDSS );
  static UInt xGetHAD4x           ( XDistSearchStruct* pcDSS );

  static UInt xGetYuvSAD16x       ( XDistSearchStruct* pcDSS );
  static UInt xGetYuvSAD8x        ( XDistSearchStruct* pcDSS );
  static UInt xGetYuvSAD4x        ( XDistSearchStruct* pcDSS );

  static UInt xGetBiSAD16x        ( XDistSearchStruct* pcDSS );
  static UInt xGetBiSAD8x         ( XDistSearchStruct* pcDSS );
  static UInt xGetBiSAD4x         ( XDistSearchStruct* pcDSS );

  static UInt xGetBiSSE16x        ( XDistSearchStruct* pcDSS );
  static UInt xGetBiSSE8x         ( XDistSearchStruct* pcDSS );
  static UInt xGetBiSSE4x         ( XDistSearchStruct* pcDSS );

  static UInt xGetBiHAD16x        ( XDistSearchStruct* pcDSS );
  static UInt xGetBiHAD8x         ( XDistSearchStruct* pcDSS );
  static UInt xGetBiHAD4x         ( XDistSearchStruct* pcDSS );

  static UInt xGetBiYuvSAD16x     ( XDistSearchStruct* pcDSS );
  static UInt xGetBiYuvSAD8x      ( XDistSearchStruct* pcDSS );
  static UInt xGetBiYuvSAD4x      ( XDistSearchStruct* pcDSS );

  static UInt xCalcHadamard4x4    ( XPel *pucOrg, XPel *pPel,                Int iStride );
  static UInt xCalcBiHadamard4x4  ( XPel *pucOrg, XPel *pPelFix, XPel *pPel, Int iStride );

//TMM_WP
  Void xGetWeight(XPel *pucRef, XPel *pucOrg, const UInt uiStride,
                  const UInt uiHeight, const UInt uiWidth, 
                  Double &dDCOrg, Double &dDCRef);
//TMM_WP

protected:
  IntYuvMbBuffer  m_cOrgData;
  XDistortionFunc m_aaafpDistortionFunc[2][4][12];
  Int             m_aiRows[12];
  Int             m_aiCols[12];
};





#if defined( MSYS_WIN32 )
#pragma warning( default: 4275 )
#endif


H264AVC_NAMESPACE_END


#endif // !defined(AFX_DISTORTION_H__7052DDA3_6AD5_4BD5_88D0_E34F8BF08D45__INCLUDED_)
