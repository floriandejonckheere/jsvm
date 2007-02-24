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

#define DEFAULTY 0
#define DEFAULTU 0
#define DEFAULTV 0

//#define OLD_DYADIC_UPSAMPLE //to be outcommented to use the old MPEG-4 (dyadic) upsampling filter for both luma and chroma

#define FILTER_UP   int piFilter[16] = {  0,  0,  1,  0, -5,  0, 20,   32,   20,    0, -5,  0,  1,  0,  0,   64 };
#define FILTER_UP_4 int piFilter[16] = {  0,  0,  0,  0, -3,  0, 19,   32,   19,    0, -3,  0,  0,  0,  0,   64 };
#ifndef OLD_DYADIC_UPSAMPLE
//cixunzhang add
#define FILTER_UP_CHROMA   int piFilter_chroma[16] = {  0,  0,  0,  0, 0,  0, 16,   32,   16,    0, 0,  0,  0,  0,  0,   64 };
#endif
#define UPS4TAP 0
#define FILTER_DOWN int piFilter[16] = {  0,  2,  0, -4, -3,  5, 19,   26,   19,    5, -3, -4,  0,  2,  0,   64 };


#ifndef DOWN_CONVERT_STATIC //TMM_JV
#include "H264AVCCommonLib/MbDataCtrl.h"

#define RESIDUAL_B8_BASED 0   // 1: residual upsampling is 8x8 based; 0: transform-block based
#endif // DOWN_CONVERT_STATIC

#include "ResizeParameters.h"

class DownConvert
{
 public:

 DownConvert ();
  ~DownConvert();
  
  int   init                    ( int            iMaxWidth,
                                  int            iMaxHeight );
  
#ifndef DOWN_CONVERT_STATIC //TMM_JV
//-------------------------------------------------
//JSVM upsampling methods (encoder + decoder) only
//-------------------------------------------------

  void  upsample                ( short*            psBufferY, int iStrideY,
                                  short*            psBufferU, int iStrideU,
                                  short*            psBufferV, int iStrideV,
                                  ResizeParameters* pcParameters,
                                  bool              bClip = true );

  void upsampleResidual         ( short*            psBufferY,  int iStrideY,
                                  short*            psBufferU,  int iStrideU,
                                  short*            psBufferV,  int iStrideV,
                                  ResizeParameters* pcParameters,
                                  h264::MbDataCtrl* pcMbDataCtrl,
                                  bool              bClip );
                                  
#else // DOWN_CONVERT_STATIC is defined
//------------------------
//DownConvert Tool only
//------------------------

  void upsample                 ( unsigned char* pucBufferY, int iStrideY,
                                  unsigned char* pucBufferU, int iStrideU,
                                  unsigned char* pucBufferV, int iStrideV,
                                  ResizeParameters* pcParameters, int iStages, int* piFilter, int* piFilter_chroma);
                                  
  void upsample_non_dyadic      ( unsigned char* pucBufferY, int iStrideY,
                                  unsigned char* pucBufferU, int iStrideU,
                                  unsigned char* pucBufferV, int iStrideV,
                                  ResizeParameters* pcParameters, int type = 0);
  
  void downsample               ( unsigned char* pucBufferY, int iStrideY,
                                  unsigned char* pucBufferU, int iStrideU,
                                  unsigned char* pucBufferV, int iStrideV,
                                  ResizeParameters* pcParameters, int iStages, int* piFilter);
                                  
  void  upsample_ver            ( short*         psBufferY,  int iStrideY,
                                  short*         psBufferU,  int iStrideU,
                                  short*         psBufferV,  int iStrideV,
                                  int            iWidth,     int iHeight,
                                  int            iPosY,      int iCropY,
                                  bool           bClip,      bool top_flg );

  void  downsample3             ( unsigned char* pucBufferY, unsigned char* pucBufferU, unsigned char* pucBufferV,
                                  int input_width, int input_height, int output_width, int output_height,
                                  int crop_x0, int crop_y0, int crop_w, int crop_h, 
                                  int input_chroma_phase_shift_x, int input_chroma_phase_shift_y,
                                  int output_chroma_phase_shift_x, int output_chroma_phase_shift_y,
                                  int resample_mode, int buf_size );

  void  downsample_ver          ( unsigned char* pucBufferY, int iStrideY,
                                  unsigned char* pucBufferU, int iStrideU,
                                  unsigned char* pucBufferV, int iStrideV,
                                  int            iWidth,     int iHeight, bool top_flg );

  void  xDownsampling_ver       ( int iWidth,       // high-resolution width
                                  int iHeight,      // high-resolution height
                                  int aiFilter[],   // downsampling filter [15coeff+sum]
                                  bool top_flg );

  void  upsample3               ( unsigned char* pucBufferY, unsigned char* pucBufferU, unsigned char* pucBufferV,
                                  int input_width, int input_height, int output_width, int output_height,
                                  int crop_x0, int crop_y0, int crop_w, int crop_h, 
                                  int input_chroma_phase_shift_x, int input_chroma_phase_shift_y,
                                  int output_chroma_phase_shift_x, int output_chroma_phase_shift_y,
                                  int resample_mode, int buf_size );

  void  upsample_ver            ( unsigned char* pucBufferY, int iStrideY,
                                  unsigned char* pucBufferU, int iStrideU,
                                  unsigned char* pucBufferV, int iStrideV,
                                  int            iWidth,     int iHeight, 
                                  int            iPosY,      int iCropY,
                                  bool           top_flg );

                                  
  void   crop                  ( unsigned char*    pucBufferY, int iStrideY,
                                  unsigned char*    pucBufferU, int iStrideU,
                                  unsigned char*    pucBufferV, int iStrideV,
                                  ResizeParameters* pcParameters );
#endif // DOWN_CONVERT_STATIC 

private:
 
  int   m_iImageStride;
  int*  m_paiImageBuffer;
  int*  m_paiTmp1dBuffer;
 
#ifdef DOWN_CONVERT_STATIC //TMM_JV
//------------------------
//DownConvert Tool only
//------------------------
  long* m_padFilter;
  int* m_aiTmp1dBufferInHalfpel;
  int* m_aiTmp1dBufferInQ1pel;
  int* m_aiTmp1dBufferInQ3pel;
  int*  m_paiTmp1dBufferOut;
#endif //DOWN_CONVERT_STATIC 
  
  void  xDestroy                ( );
  int   xClip                   ( int               iValue,
                                  int               imin,
                                  int               imax );

  void   xUpsampling3           ( ResizeParameters* pcParameters,
                                  bool bLuma, int resample_mode );

  void  xUpsampling_ver         ( int               iWidth,         // low-resolution width
                                  int               iHeight,        // low-resolution height
                                  int               iPosY,     
                                  int               iCropY,
                                  int               *aiFilter,
                                  bool              top_flg    );
  
  void  xUpsampling_ver_res     ( int               iWidth,         // low-resolution width
                                  int               iHeight,        // low-resolution height
                                  int               iPosY,     
                                  int               iCropY,
                                  int               chroma,
                                  bool              top_flg    );

  //cixunzhang
  void   xUpsampling3           ( int input_width, int input_height,
	                                int output_width, int output_height,
	                                int crop_x0, int crop_y0, int crop_w, int crop_h,
	                                int input_chroma_phase_shift_x, int input_chroma_phase_shift_y,
	                                int output_chroma_phase_shift_x, int output_chroma_phase_shift_y, bool uv_flag );
#ifdef _JVTV074_
  //cixunzhang
  void   xUpsampling4           ( int input_width, int input_height,
	                                int output_width, int output_height,
	                                int crop_x0, int crop_y0, int crop_w, int crop_h,
	                                int input_chroma_phase_shift_x, int input_chroma_phase_shift_y,
	                                int output_chroma_phase_shift_x, int output_chroma_phase_shift_y, bool uv_flag, int iResampleFilter[16][4]);
#endif // _JVTV074_
                                  
#ifndef DOWN_CONVERT_STATIC // TMM_JV
//-------------------------------------------------
//JSVM upsampling methods (encoder + decoder) only
//-------------------------------------------------
  

  void   xGenericUpsampleEss    ( short*            psBufferY, int iStrideY,
                                  short*            psBufferU, int iStrideU,
                                  short*            psBufferV, int iStrideV,
                                  ResizeParameters* pcParameters,
                                  h264::MbDataCtrl* pcMbDataCtrl); 
  void   xGenericUpsampleEss    ( short*            psBufferY, int iStrideY,
                                  short*            psBufferU, int iStrideU,
                                  short*            psBufferV, int iStrideV,
                                  ResizeParameters* pcParameters,
                                  bool              bClip = true );
  //jzxu
  void   xFilterResidualHor     ( short *buf_in, short *buf_out, 
                                  int width, int x, int w,
                                  int wsize_in, int hsize_in, 
                                  h264::MbDataCtrl*  pcMbDataCtrl, 
                                  bool chroma, bool interlace,
                                  int output_chroma_phase_shift_x, int input_chroma_phase_shift_x,
                                  unsigned char *buf_blocksize );
  void   xFilterResidualVer     ( short *buf_in, short *buf_out, 
                                  int width, 
                                  int x, int y, int w, int h, 
                                  int wsize_in, int hsize_in, 
                                  bool chroma,  bool interlace,
                                  int output_chroma_phase_shift_y, int input_chroma_phase_shift_y,
                                  unsigned char *buf_blocksize );
 						           
  void   xCrop                  ( short*            psBufferY, int iStrideY,
                                  short*            psBufferU, int iStrideU,
                                  short*            psBufferV, int iStrideV,
                                  ResizeParameters* pcParameters,
                                  bool              bClip = true );
                                  
  void  xCopyToImageBuffer      ( short*            psSrc,
                                  int               iWidth,
                                  int               iHeight,
                                  int               iStride );

  void  xCopyFromImageBuffer    ( short*            psDes,
                                  int               iWidth,
                                  int               iHeight,
                                  int               iStride,
                                  int               imin,
                                  int               imax );

  void   xSetValue              ( short* psBuffer, int iStride,
                                  int iWidth, int iHeight,
                                  short value );
                                  
                                  
#else // DOWN_CONVERT_STATIC is defined
//------------------------
//DownConvert Tool only
//------------------------

  void  xCopyToImageBuffer      ( short*            psSrc,
                                  int               iWidth,
                                  int               iHeight,
                                  int               iStride );

  void  xCopyFromImageBuffer    ( short*            psDes,
                                  int               iWidth,
                                  int               iHeight,
                                  int               iStride,
                                  int               imin,
                                  int               imax );

  void  xCopyToImageBuffer      ( unsigned char*    pucSrc,
                                  int               iWidth,
                                  int               iHeight,
                                  int               iStride );

  void  xCopyFromImageBuffer    ( unsigned char*    pucDes,
                                  int               iWidth,
                                  int               iHeight,
                                  int               iStride,
                                  int               imin,
                                  int               imax );
                                  
  void   xSetValue              ( unsigned char* pucBuffer, int iStride,
                                  int iWidth, int iHeight,
                                  unsigned char value );
                                  
  void  xDownsampling           ( int               iWidth,         // high-resolution width
                                  int               iHeight,        // high-resolution height
                                  int               aiFilter[] );   // downsampling filter [15coeff+sum]

  void  xUpsampling             ( int               iWidth,         // low-resolution width
                                  int               iHeight,        // low-resolution height
                                  int               aiFilter[] );   // downsampling filter [15coeff+sum]

  void   xUpsampling            ( ResizeParameters* pcParameters,
                                  bool bLuma, int type );

// =================================================================================
//   INTRA 1 Lanczos
// =================================================================================
  void   xInitFilterTmm1        ( );
  void   xDestroyFilterTmm1     ( );
  void   xUpsampling1           ( ResizeParameters* pcParameters,
                                  bool bLuma );
  void   xUpsamplingData1       ( int iInLength , int iOutLength , long spos );
  long   xGetFilter             ( long x );
  
// =================================================================================
//   INTRA 2
// =================================================================================
  void   xInitFilterTmm2        ( int iMaxDim );
  void   xDestroyFilterTmm2     ( );
  void   xUpsampling2           ( ResizeParameters* pcParameters,
                                  bool bLuma );
  void   xUpsamplingData2       ( int iInLength , int iOutLength );

// =================================================================================

  void   xInitFilterTmm         ( int iMaxDim );
  void   xDestroyFilterTmm      ( );


  void   xCrop                  ( unsigned char*    pucBufferY, int iStrideY,
                                  unsigned char*    pucBufferU, int iStrideU,
                                  unsigned char*    pucBufferV, int iStrideV,
                                  ResizeParameters* pcParameters );
                                  
  void   xComputeNumeratorDenominator ( int iInWidth , int iOutWidth ,
                                        int* iNumerator, int *iDenominator);
                                        
  void   xDownsampling3         ( int input_width, int input_height, int output_width, int output_height,
                                  int crop_x0, int crop_y0, int crop_w, int crop_h,
                                  int input_chroma_phase_shift_x, int input_chroma_phase_shift_y,
                                  int output_chroma_phase_shift_x, int output_chroma_phase_shift_y );   

#endif //DOWN_CONVERT_STATIC 

};

#include "DownConvert.inl"

#ifdef DOWN_CONVERT_STATIC //TMM_JV
#include "DownConvertTools.inl"
#endif // DOWN_CONVERT_STATIC 

#undef DEFAULTY
#undef DEFAULTU
#undef DEFAULTV

#endif // _DOWN_CONVERT_


