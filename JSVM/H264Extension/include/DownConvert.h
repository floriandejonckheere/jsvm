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

#include "ResizeParameters.h"
#define LANCZOS_OPTIM 1


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
// TMM_ESS }
  void  upsample                ( short*            psBufferY, int iStrideY,
                                  short*            psBufferU, int iStrideU,
                                  short*            psBufferV, int iStrideV,
                                  ResizeParameters* pcParameters,
                                  bool              bClip = true );

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
  // TMM_ESS 
  void upsampleResidual         ( short*            psBufferY,  int iStrideY,
                                  short*            psBufferU,  int iStrideU,
                                  short*            psBufferV,  int iStrideV,
                                  ResizeParameters* pcParameters,
                                  h264::MbDataCtrl* pcMbDataCtrl,
                                  bool              bClip );

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

// TMM_ESS {

// =================================================================================
//   GENERAL
// =================================================================================
public:
  void  upsample_tmm            ( unsigned char*    pucBufferY, int iStrideY,
                                  unsigned char*    pucBufferU, int iStrideU,
                                  unsigned char*    pucBufferV, int iStrideV,
                                  ResizeParameters* pcParameters );

private:
  void   xInitFilterTmm         ( int iMaxDim );
  void   xDestroyFilterTmm      ( );

  void   xGenericUpsample       ( unsigned char*    pucBufferY, int iStrideY,
                                  unsigned char*    pucBufferU, int iStrideU,
                                  unsigned char*    pucBufferV, int iStrideV,
                                  ResizeParameters* pcParameters );
  // SSUN@SHARP
  void   xGenericUpsampleEss    ( short*            psBufferY, int iStrideY,
                                  short*            psBufferU, int iStrideU,
                                  short*            psBufferV, int iStrideV,
                                  ResizeParameters* pcParameters,
                                  bool              bClip = true );
  // end of SSUN@SHARP
  void   xCrop                  ( unsigned char*    pucBufferY, int iStrideY,
                                  unsigned char*    pucBufferU, int iStrideU,
                                  unsigned char*    pucBufferV, int iStrideV,
                                  ResizeParameters* pcParameters );
  void   xGenericUpsample       ( short*            psBufferY, int iStrideY,
                                  short*            psBufferU, int iStrideU,
                                  short*            psBufferV, int iStrideV,
                                  ResizeParameters* pcParameters,
                                  bool              bClip = true );
  void   xCrop                  ( short*            psBufferY, int iStrideY,
                                  short*            psBufferU, int iStrideU,
                                  short*            psBufferV, int iStrideV,
                                  ResizeParameters* pcParameters,
                                  bool              bClip = true );

#ifndef NO_MB_DATA_CTRL
  // SSUN@SHARP
  void   xGenericUpsampleEss    ( short*            psBufferY, int iStrideY,
                                  short*            psBufferU, int iStrideU,
                                  short*            psBufferV, int iStrideV,
                                  ResizeParameters* pcParameters,
                                  h264::MbDataCtrl* pcMbDataCtrl,
                                  bool              bClip = true );
  void   xFilterResidualHor     ( short *buf_in, short *buf_out, 
                                  int width, int height, 
                                  int x, int y, int w, int h,  
                                  int wsize_in, int hsize_in, 
                                  h264::MbDataCtrl*  pcMbDataCtrl, 
                                  bool chroma, int rounding_para,  // SSUN, 27Sept2005
                                  unsigned char *buf_blocksize );
  void   xFilterResidualVer     ( short *buf_in, short *buf_out, 
                                  int width, int height, 
                                  int x, int y, int w, int h, 
                                  int wsize_in, int hsize_in, 
                                  h264::MbDataCtrl*  pcMbDataCtrl, 
                                  bool chroma, int rounding_para,  // SSUN, 27Sept2005
                                  unsigned char *buf_blocksize );
  // end of SSUN@SHARP
  void xCopyBuffer		(int *pitmpDes,
						           int *piDes,
						           int ixlastsample,
						           int iylastsample,
						           int ixout,
						           int iyout,
						           int iSizeOut);

#endif
  
  void   xSetValue              ( unsigned char* pucBuffer, int iStride,
                                  int iWidth, int iHeight,
                                  unsigned char value );
  void   xSetValue              ( short* psBuffer, int iStride,
                                  int iWidth, int iHeight,
                                  short value );
  void   xUpsampling            ( ResizeParameters* pcParameters,
                                  bool bLuma );
  void   xComputeNumeratorDenominator ( int iInWidth , int iOutWidth ,
                                        int* iNumerator, int *iDenominator);

// =================================================================================
//   INTRA 1 Lanczos
// =================================================================================
private:
  void   xInitFilterTmm1        ( int iMaxDim );
  void   xDestroyFilterTmm1     ( );
  void   xUpsampling1           ( ResizeParameters* pcParameters,
                                  bool bLuma );
#if LANCZOS_OPTIM
  void   xUpsamplingData1       ( int iInLength , int iOutLength , long spos );
  long   xGetFilter             ( long x );
#else
  void   xUpsamplingData1       ( int iInLength , int iOutLength , int iNumerator , int iDenominator );
  double xGetFilter             ( double x );
#endif

private:
#if LANCZOS_OPTIM
  long* m_padFilter;
#else
  double* m_padFilter;
#endif

  
  
// =================================================================================
//   INTRA 2
// =================================================================================
private:
  void   xInitFilterTmm2        ( int iMaxDim );
  void   xDestroyFilterTmm2     ( );
  void   xUpsampling2           ( ResizeParameters* pcParameters,
                                  bool bLuma );
  void   xUpsamplingData2       ( int iInLength , int iOutLength , int iNumerator , int iDenominator );

// =================================================================================
  
// INTRA 2 / INTER 2 
private:
 
  int* m_aiTmp1dBufferInHalfpel;
  int* m_aiTmp1dBufferInQ1pel;
  int* m_aiTmp1dBufferInQ3pel;
  
  
// =================================================================================
  
// INTRA 3
public:
  void upsample3            ( unsigned char* pucBufferY, unsigned char* pucBufferU, unsigned char* pucBufferV,
                              int input_width, int input_height, int output_width, int output_height,
                              int crop_x0, int crop_y0, int crop_w, int crop_h, 
                              int input_chroma_phase_shift_x, int input_chroma_phase_shift_y,
                              int output_chroma_phase_shift_x, int output_chroma_phase_shift_y);
  
  void downsample3          ( unsigned char* pucBufferY, unsigned char* pucBufferU, unsigned char* pucBufferV,
                              int input_width, int input_height, int output_width, int output_height,
                              int crop_x0, int crop_y0, int crop_w, int crop_h, 
                              int input_chroma_phase_shift_x, int input_chroma_phase_shift_y,
                              int output_chroma_phase_shift_x, int output_chroma_phase_shift_y );

  void UnifiedOneForthPix ( unsigned char **out4Y, int img_width, int img_height );

  
private:
  void   xUpsampling3           ( ResizeParameters* pcParameters,
                                  bool bLuma );
  void   xUpsampling3           ( int input_width, int input_height,
                                  int output_width, int output_height,
                                  int crop_x0, int crop_y0, int crop_w, int crop_h,
                                  int input_chroma_phase_shift_x, int input_chroma_phase_shift_y,
                                  int output_chroma_phase_shift_x, int output_chroma_phase_shift_y,
                                  bool uv_flag );
  void   xDownsampling3         ( int input_width, int input_height, int output_width, int output_height,
                                  int crop_x0, int crop_y0, int crop_w, int crop_h,
                                  int input_chroma_phase_shift_x, int input_chroma_phase_shift_y,
                                  int output_chroma_phase_shift_x, int output_chroma_phase_shift_y, bool uv_flg );


// =================================================================================
// =================================================================================
private:
  int*  m_paiTmp1dBufferIn;
  int*  m_paiTmp1dBufferOut;

// TMM_ESS }
  
};





#include "DownConvert.inl"
#include "DownConvertTmm.inl"



#endif // _DOWN_CONVERT_
