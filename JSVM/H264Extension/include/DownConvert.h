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





#ifndef _DOWN_CONVERT_
#define _DOWN_CONVERT_


#define FILTER_UP   int piFilter[16] = {  0,  0,  1,  0, -5,  0, 20,   32,   20,    0, -5,  0,  1,  0,  0,   64 };
#define FILTER_DOWN int piFilter[16] = {  0,  2,  0, -4, -3,  5, 19,   26,   19,    5, -3, -4,  0,  2,  0,   64 };


#ifndef NO_MB_DATA_CTRL
#include "H264AVCCommonLib/MbDataCtrl.h"
#endif


class DownConvert
{
public:
  DownConvert ();
  ~DownConvert();
  
  int   init                    ( int            iMaxWidth,
                                  int            iMaxHeight );
  void  downsample              ( unsigned char* pucBufferY, int iStrideY,
                                  unsigned char* pucBufferU, int iStrideU,
                                  unsigned char* pucBufferV, int iStrideV,
                                  int            iWidth,     int iHeight, int iStages = 1 );                // high-resolution
  void  downsample              ( short*         psBufferY,  int iStrideY,
                                  short*         psBufferU,  int iStrideU,
                                  short*         psBufferV,  int iStrideV,
                                  int            iWidth,     int iHeight, bool bClip, int iStages = 1 );    // high-resolution
  void  upsample                ( unsigned char* pucBufferY, int iStrideY,
                                  unsigned char* pucBufferU, int iStrideU,
                                  unsigned char* pucBufferV, int iStrideV,
                                  int            iWidth,     int iHeight, int iStages = 1 );                // low-resolution
  void  upsample                ( short*         psBufferY,  int iStrideY,
                                  short*         psBufferU,  int iStrideU,
                                  short*         psBufferV,  int iStrideV,
                                  int            iWidth,     int iHeight, bool bClip, int iStages = 1 );    // low-resolution

  // upsampling with given filter coefficients
  void  upsample                ( unsigned char* pucBufferY, int iStrideY,
                                  unsigned char* pucBufferU, int iStrideU,
                                  unsigned char* pucBufferV, int iStrideV,
                                  int            iWidth,     int iHeight, int* piFilter, int iStages = 1 );                // low-resolution
  void  upsample                ( short*         psBufferY,  int iStrideY,
                                  short*         psBufferU,  int iStrideU,
                                  short*         psBufferV,  int iStrideV,
                                  int            iWidth,     int iHeight, int* piFilter, bool bClip, int iStages = 1 );    // low-resolution

#ifndef NO_MB_DATA_CTRL
  void  upsampleResidual        ( short*         psBufferY,  int iStrideY,
                                  short*         psBufferU,  int iStrideU,
                                  short*         psBufferV,  int iStrideV,
                                  int            iWidth,     int iHeight, 
                                  h264::MbDataCtrl* pcMbDataCtrl, bool bClip, int iStages = 1 );    // low-resolution
#endif
  
private:
  int   xClip                   ( int               iValue,
                                  int               imin,
                                  int               imax );
  void  xDownsampling           ( int               iWidth,         // high-resolution width
                                  int               iHeight,        // high-resolution height
                                  int               aiFilter[] );   // downsampling filter [15coeff+sum]
  void  xUpsampling             ( int               iWidth,         // low-resolution width
                                  int               iHeight,        // low-resolution height
                                  int               aiFilter[] );   // downsampling filter [15coeff+sum]
  void  xCopyToImageBuffer      ( short*            psSrc,
                                  int               iWidth,
                                  int               iHeight,
                                  int               iStride );
  void  xCopyToImageBuffer      ( unsigned char*    pucSrc,
                                  int               iWidth,
                                  int               iHeight,
                                  int               iStride );
  void  xCopyFromImageBuffer    ( short*            psDes,
                                  int               iWidth,
                                  int               iHeight,
                                  int               iStride,
                                  int               imin,
                                  int               imax );
  void  xCopyFromImageBuffer    ( unsigned char*    pucDes,
                                  int               iWidth,
                                  int               iHeight,
                                  int               iStride,
                                  int               imin,
                                  int               imax );

#ifndef NO_MB_DATA_CTRL
  void xUpsamplingSubMb         ( int*              piSrcBlock,     // low-resolution src-addr
                                  int*              piDesBlock,     // high-resolution src-addr
                                  h264::BlkMode     eBlkMode,
                                  int               aiFilter[] );   // downsampling filter [15coeff+sum]

  void  xCopyFromImageBuffer2   ( short*            psDes,
                                  int               iWidth,
                                  int               iHeight,
                                  int               iStride,
                                  int               imin,
                                  int               imax );

  void xUpsamplingBlock         ( int               iWidth,         // low-resolution width
                                  int               iHeight,        // low-resolution height
                                  int*              piSrcBlock,     // low-resolution src-addr
                                  int*              piDesBlock,     // high-resolution src-addr
                                  int               aiFilter[] );   // downsampling filter [15coeff+sum]

  void xUpsamplingFrame         ( int               iWidth,         // low-resolution width
                                  int               iHeight,        // low-resolution height
                                  int*              piSrcBlock,     // low-resolution src-addr
                                  int*              piDesBlock,     // high-resolution src-addr
                                  bool              bLuma,
                                  h264::MbDataCtrl* pcMbDataCtrl,
                                  int               aiFilter[] );   // downsampling filter [15coeff+sum]

#endif

private:
  int   m_iImageStride;
  int*  m_paiImageBuffer;
#ifndef NO_MB_DATA_CTRL
  int*  m_paiImageBuffer2;
#endif
  int*  m_paiTmp1dBuffer;
};





#include "DownConvert.inl"




#endif // _DOWN_CONVERT_
