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






#if !defined(AFX_INTYUVMBBUFFER_H__C9AFC3F1_5EEF_43A6_8105_C3BCD7B098FA__INCLUDED_)
#define AFX_INTYUVMBBUFFER_H__C9AFC3F1_5EEF_43A6_8105_C3BCD7B098FA__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


H264AVC_NAMESPACE_BEGIN


class YuvPicBuffer;


#define OFFSET 19
class H264AVCCOMMONLIB_API YuvMbBuffer
{
public:
	YuvMbBuffer();
	virtual ~YuvMbBuffer();

  XPel*     getLumBlk     ()                      { return m_pPelCurrY; }
  XPel*     getCbBlk      ()                      { return m_pPelCurrU; }
  XPel*     getCrBlk      ()                      { return m_pPelCurrV; }

  Void      set4x4Block   ( LumaIdx cIdx )
  {
    m_pPelCurrY = &m_aucYuvBuffer[   MB_BUFFER_WIDTH +  4 + ((cIdx.x() + cIdx.y()* MB_BUFFER_WIDTH)<<2)];
    m_pPelCurrU = &m_aucYuvBuffer[OFFSET*MB_BUFFER_WIDTH +  4 + ((cIdx.x() + cIdx.y()* MB_BUFFER_WIDTH)<<1)];
    m_pPelCurrV = &m_aucYuvBuffer[OFFSET*MB_BUFFER_WIDTH + 16 + ((cIdx.x() + cIdx.y()* MB_BUFFER_WIDTH)<<1)];
  }

  const Int getLStride    ()                const { return MB_BUFFER_WIDTH;}
  const Int getCStride    ()                const { return MB_BUFFER_WIDTH;}

  XPel*     getYBlk       ( LumaIdx cIdx )        { return &m_aucYuvBuffer[   MB_BUFFER_WIDTH +  4 + ((cIdx.x() + cIdx.y()* MB_BUFFER_WIDTH)<<2)]; }
  XPel*     getUBlk       ( LumaIdx cIdx )        { return &m_aucYuvBuffer[OFFSET*MB_BUFFER_WIDTH +  4 + ((cIdx.x() + cIdx.y()* MB_BUFFER_WIDTH)<<1)]; }
  XPel*     getVBlk       ( LumaIdx cIdx )        { return &m_aucYuvBuffer[OFFSET*MB_BUFFER_WIDTH + 16 + ((cIdx.x() + cIdx.y()* MB_BUFFER_WIDTH)<<1)]; }
  XPel*     getCBlk       ( ChromaIdx cIdx )      { return &m_aucYuvBuffer[OFFSET*MB_BUFFER_WIDTH +  4 + ((cIdx.x() + cIdx.y()* MB_BUFFER_WIDTH)<<2) + 12*cIdx.plane()]; }

  const XPel*     getMbLumAddr  ()          const { return &m_aucYuvBuffer[   MB_BUFFER_WIDTH +  4]; }
  const XPel*     getMbCbAddr   ()          const { return &m_aucYuvBuffer[OFFSET*MB_BUFFER_WIDTH +  4]; }
  const XPel*     getMbCrAddr   ()          const { return &m_aucYuvBuffer[OFFSET*MB_BUFFER_WIDTH + 16];  }

  XPel*     getMbLumAddr  ()                      { return &m_aucYuvBuffer[   MB_BUFFER_WIDTH +  4]; }
  XPel*     getMbCbAddr   ()                      { return &m_aucYuvBuffer[OFFSET*MB_BUFFER_WIDTH +  4]; }
  XPel*     getMbCrAddr   ()                      { return &m_aucYuvBuffer[OFFSET*MB_BUFFER_WIDTH + 16];  }

  const Int getLWidth     ()                const { return 16; }
  const Int getLHeight    ()                const { return 16; }
  const Int getCWidth     ()                const { return 8; }
  const Int getCHeight    ()                const { return 8; }

  Void loadBuffer           ( YuvPicBuffer*  pcSrcBuffer );

  Void loadLuma             ( YuvMbBuffer&   rcSrcBuffer, LumaIdx c4x4Idx);
  Void loadLuma             ( YuvMbBuffer&   rcSrcBuffer, B8x8Idx c8x8Idx);
  Void loadLuma             ( YuvMbBuffer&   rcSrcBuffer );
  Void loadChroma           ( YuvMbBuffer&   rcSrcBuffer );

  Void loadIntraPredictors  ( YuvPicBuffer*  pcSrcBuffer );

  Void setAllSamplesToZero  ();

  Void  add         ( YuvMbBuffer& rcIntYuvMbBuffer );
  Void  subtract    ( YuvMbBuffer& rcIntYuvMbBuffer );
  Void  clip        ();
  Bool  isZero      ();
	Void  setZero     ();

protected:
  XPel* m_pPelCurrY;
  XPel* m_pPelCurrU;
  XPel* m_pPelCurrV;
  XPel  m_aucYuvBuffer[MB_BUFFER_WIDTH*(29+1)];
};

class H264AVCCOMMONLIB_API YuvMbBufferExtension : public YuvMbBuffer
{
public:
  Void loadSurrounding      ( YuvPicBuffer* pcSrcBuffer );
  Void loadSurrounding_MbAff( YuvPicBuffer* pcSrcBuffer, UInt uiMask );//TMM_INTERLACE

  Void mergeFromLeftAbove ( LumaIdx cIdx, Bool bCornerMbPresent, Bool bHalfYSize = false );
  Void mergeFromRightBelow( LumaIdx cIdx, Bool bCornerMbPresent, Bool bHalfYSize = false );
  Void mergeFromRightAbove( LumaIdx cIdx, Bool bCornerMbPresent, Bool bHalfYSize = false );
  Void mergeFromLeftBelow ( LumaIdx cIdx, Bool bCornerMbPresent, Bool bHalfYSize = false );

  Void copyFromBelow      ( LumaIdx cIdx, Bool bHalfYSize = false );
  Void copyFromLeft       ( LumaIdx cIdx );
  Void copyFromAbove      ( LumaIdx cIdx, Bool bHalfYSize = false );
  Void copyFromRight      ( LumaIdx cIdx );

  Void copyFromLeftAbove  ( LumaIdx cIdx, Bool bHalfYSize = false );
  Void copyFromRightAbove ( LumaIdx cIdx, Bool bHalfYSize = false );
  Void copyFromLeftBelow  ( LumaIdx cIdx, Bool bHalfYSize = false );
  Void copyFromRightBelow ( LumaIdx cIdx, Bool bHalfYSize = false );

  Void xFill  ( LumaIdx cIdx, XPel cY, XPel cU, XPel cV, Bool bHalfYSize, Bool bLowerHalf );
  Void xMerge ( Int xDir, Int yDir, Int iSize, XPel* puc, Int iStride, Bool bCornerMbPresent, Bool bHalfYSize );
};


H264AVC_NAMESPACE_END


#endif // !defined(AFX_INTYUVMBBUFFER_H__C9AFC3F1_5EEF_43A6_8105_C3BCD7B098FA__INCLUDED_)
