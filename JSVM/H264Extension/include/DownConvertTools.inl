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

********************************************************************************/

#include <math.h>

#ifndef max
#define max(x, y) ((x)>(y)?(x):(y))
#define min(x, y) ((x)<(y)?(x):(y))
#endif

#define TAP_10_FLT 1          // 1: use a set of 10-tap filters for downsampling; 0: a set of 12-taps.
#define KAISER_FLT 0          // 1: use Kaiser filter, 0: use sine-windowed filter
                       
__inline
void
DownConvert::xInitFilterTmm (int iMaxDim)
{  
 xDestroyFilterTmm ( );
 
 m_paiTmp1dBufferOut = new int [ iMaxDim  ];
 xInitFilterTmm1(); 
 xInitFilterTmm2(iMaxDim);
}

__inline
void
DownConvert::xDestroyFilterTmm ( )
{
  if (m_paiTmp1dBufferOut) 
  {
    delete [] m_paiTmp1dBufferOut;
    m_paiTmp1dBufferOut = NULL;
  }
  
  xDestroyFilterTmm1();
  xDestroyFilterTmm2();
}

__inline
void
DownConvert::upsample                 ( unsigned char* pucBufferY, int iStrideY,
                                        unsigned char* pucBufferU, int iStrideU,
                                        unsigned char* pucBufferV, int iStrideV,
                                        ResizeParameters* pcParameters, 
                                        int iStages, 
                                        int* piFilter,
                                        int* piFilter_chroma)
{
  int iWidth = pcParameters->m_iInWidth;
  int iHeight = pcParameters->m_iInHeight;
  
  for( int i = iStages; i > 0; i-- )
  {
    //===== luma =====
    xCopyToImageBuffer  ( pucBufferY, iWidth,   iHeight,   iStrideY );
    xUpsampling         (             iWidth,   iHeight,   piFilter );
    xCopyFromImageBuffer( pucBufferY, iWidth*2, iHeight*2, iStrideY, 0, 255 );

	//===== chroma cb =====
	xCopyToImageBuffer  ( pucBufferU, iWidth/2, iHeight/2, iStrideU );
	xUpsampling         (             iWidth/2, iHeight/2, piFilter_chroma );
	xCopyFromImageBuffer( pucBufferU, iWidth,   iHeight,   iStrideU, 0, 255 );
	//===== chroma cr =====
	xCopyToImageBuffer  ( pucBufferV, iWidth/2, iHeight/2, iStrideV );
	xUpsampling         (             iWidth/2, iHeight/2, piFilter_chroma );
	xCopyFromImageBuffer( pucBufferV, iWidth,   iHeight,   iStrideV, 0, 255 );


    iWidth  <<=1;
    iHeight <<=1;
  }
}

__inline
void
DownConvert::upsample_non_dyadic      ( unsigned char* pucBufferY, int iStrideY,
                                        unsigned char* pucBufferU, int iStrideU,
                                        unsigned char* pucBufferV, int iStrideV,
                                        ResizeParameters* pcParameters, int type)
{
  int iInWidth = pcParameters->m_iInWidth;
  int iInHeight = pcParameters->m_iInHeight;
  int iGlobWidth = pcParameters->m_iGlobWidth;
  int iGlobHeight = pcParameters->m_iGlobHeight;
  
  //===== luma =====
  xCopyToImageBuffer  ( pucBufferY, iInWidth,   iInHeight,   iStrideY );
  xUpsampling         ( pcParameters, true, type );
  xCopyFromImageBuffer( pucBufferY,   iGlobWidth,   iGlobHeight,  iStrideY, 0, 255 );


  // ===== parameters for chromas =====
  iInWidth    >>= 1;
  iInHeight   >>= 1;
  iGlobWidth   >>= 1;
  iGlobHeight  >>= 1;
  
  //===== chroma cb =====
  xCopyToImageBuffer  ( pucBufferU, iInWidth, iInHeight, iStrideU );
  xUpsampling         ( pcParameters, false, type ); 
  xCopyFromImageBuffer( pucBufferU, iGlobWidth, iGlobHeight, iStrideU, 0, 255 );

  //===== chroma cr =====
  xCopyToImageBuffer  ( pucBufferV, iInWidth, iInHeight, iStrideV );
  xUpsampling         ( pcParameters, false, type );
  xCopyFromImageBuffer( pucBufferV, iGlobWidth, iGlobHeight, iStrideV, 0, 255 ); 
}

__inline
void
DownConvert::downsample               ( unsigned char* pucBufferY, int iStrideY,
                                        unsigned char* pucBufferU, int iStrideU,
                                        unsigned char* pucBufferV, int iStrideV,
                                        ResizeParameters* pcParameters, int iStages, int* piFilter)
{
  int iWidth = pcParameters->m_iInWidth;
  int iHeight = pcParameters->m_iInHeight;
  
  for( int i = iStages; i > 0; i-- )
  {
    //===== luma =====
    xCopyToImageBuffer  ( pucBufferY, iWidth,   iHeight,   iStrideY );
    xDownsampling       (             iWidth,   iHeight,   piFilter );
    xCopyFromImageBuffer( pucBufferY, iWidth/2, iHeight/2, iStrideY, 0, 255 );
    //===== chroma cb =====
    xCopyToImageBuffer  ( pucBufferU, iWidth/2, iHeight/2, iStrideU );
    xDownsampling       (             iWidth/2, iHeight/2, piFilter );
    xCopyFromImageBuffer( pucBufferU, iWidth/4, iHeight/4, iStrideU, 0, 255 );
    //===== chroma cr =====
    xCopyToImageBuffer  ( pucBufferV, iWidth/2, iHeight/2, iStrideV );
    xDownsampling       (             iWidth/2, iHeight/2, piFilter );
    xCopyFromImageBuffer( pucBufferV, iWidth/4, iHeight/4, iStrideV, 0, 255 );

    iWidth  >>=1;
    iHeight >>=1;
  }
}

__inline
void
DownConvert::downsample3              ( unsigned char* pucBufferY, int iStrideY,
                                        unsigned char* pucBufferU, int iStrideU,
                                        unsigned char* pucBufferV, int iStrideV,
                                        ResizeParameters* pcParameters)
{
  int input_width   = pcParameters->m_iInWidth;
  int input_height  = pcParameters->m_iInHeight;
  int output_width  = pcParameters->m_iGlobWidth;  
  int output_height = pcParameters->m_iGlobHeight;
  int crop_x0 = pcParameters->m_iPosX;
  int crop_y0 = pcParameters->m_iPosY;
  int crop_w = pcParameters->m_iOutWidth;
  int crop_h = pcParameters->m_iOutHeight;  
  int input_chroma_phase_shift_x = pcParameters->m_iBaseChromaPhaseX;
  int input_chroma_phase_shift_y = pcParameters->m_iBaseChromaPhaseY;
  int output_chroma_phase_shift_x = pcParameters->m_iChromaPhaseX;
  int output_chroma_phase_shift_y = pcParameters->m_iChromaPhaseY;
  
  xCopyToImageBuffer  ( pucBufferY, input_width,   input_height,   iStrideY );
  xDownsampling3      ( input_width, input_height, output_width, output_height,
                        crop_x0, crop_y0, crop_w, crop_h, 0, 0, 0, 0);
  xCopyFromImageBuffer( pucBufferY, output_width, output_height, iStrideY, 0, 255 );
  
  input_width >>= 1;
  input_height >>= 1;
  output_width >>= 1;
  output_height >>= 1;
  crop_x0 >>= 1;
  crop_y0 >>= 1;
  crop_w >>= 1;
  crop_h >>= 1;
  xCopyToImageBuffer  ( pucBufferU, input_width, input_height, iStrideU );
  xDownsampling3      ( input_width, input_height, output_width, output_height,
                        crop_x0, crop_y0, crop_w, crop_h, 
                        input_chroma_phase_shift_x, input_chroma_phase_shift_y,
                        output_chroma_phase_shift_x, output_chroma_phase_shift_y); 
  xCopyFromImageBuffer( pucBufferU, output_width, output_height, iStrideU, 0, 255 );
  
  xCopyToImageBuffer  ( pucBufferV, input_width, input_height, iStrideV );
  xDownsampling3      ( input_width, input_height, output_width, output_height,
                        crop_x0, crop_y0, crop_w, crop_h, 
                        input_chroma_phase_shift_x, input_chroma_phase_shift_y,
                        output_chroma_phase_shift_x, output_chroma_phase_shift_y); 
  xCopyFromImageBuffer( pucBufferV, output_width, output_height, iStrideV, 0, 255 );
  
}
__inline
void
DownConvert::crop                  ( unsigned char*    pucBufferY, int iStrideY,
                                     unsigned char*    pucBufferU, int iStrideU,
                                     unsigned char*    pucBufferV, int iStrideV,
                                     ResizeParameters* pcParameters )
{
 xCrop ( pucBufferY, iStrideY, pucBufferU, iStrideU, pucBufferV, iStrideV, pcParameters );
}

__inline
void
DownConvert::xCopyFromImageBuffer( unsigned char* pucDes,
                                   int            iWidth,
                                   int            iHeight,
                                   int            iStride,
                                   int            imin,
                                   int            imax )
{
  int* piSrc = m_paiImageBuffer;

  for( int j = 0; j < iHeight; j++ )
  {
    for( int i = 0; i < iWidth;  i++ )
    {
      pucDes[i] = (unsigned char)xClip( piSrc[i], imin, imax );
    }
    pucDes  += iStride;
    piSrc   += m_iImageStride;
  }
}

__inline
void
DownConvert::xCopyToImageBuffer( unsigned char* pucSrc,
                                 int            iWidth,
                                 int            iHeight,
                                 int            iStride )
{
  int* piDes = m_paiImageBuffer;

  for( int j = 0; j < iHeight; j++ )
  {
    for( int i = 0; i < iWidth;  i++ )
    {
      piDes[i] = (int)pucSrc[i];
    }
    piDes   += m_iImageStride;
    pucSrc  += iStride;
  }
}

__inline
void
DownConvert::xCrop ( unsigned char* pucBufferY, int iStrideY,
                     unsigned char* pucBufferU, int iStrideU,
                     unsigned char* pucBufferV, int iStrideV,
                     ResizeParameters* pcParameters )
{
  int iOutWidth = pcParameters->m_iOutWidth;
  int iOutHeight = pcParameters->m_iOutHeight;
  int iPosX = pcParameters->m_iPosX;
  int iPosY = pcParameters->m_iPosY;
  int iGlobWidth = pcParameters->m_iGlobWidth;
  int iGlobHeight = pcParameters->m_iGlobHeight;

  unsigned char* ptr;

  //===== luma =====
  ptr = &pucBufferY[iPosY * iStrideY + iPosX];
  xCopyToImageBuffer  ( ptr, iOutWidth,  iOutHeight, iStrideY);
  xSetValue(pucBufferY, iStrideY,   iGlobWidth, iGlobHeight, (unsigned char)DEFAULTY);
  xCopyFromImageBuffer( pucBufferY, iOutWidth,  iOutHeight, iStrideY, 0, 255);

  // ===== parameters for chromas =====
  iOutWidth   >>= 1;
  iOutHeight  >>= 1;
  iPosX       >>= 1;
  iPosY       >>= 1;
  iGlobWidth  >>= 1;
  iGlobHeight >>= 1;
  
  //===== chroma cb =====
  ptr = &pucBufferU[iPosY * iStrideU + iPosX];
  xCopyToImageBuffer  ( ptr, iOutWidth,  iOutHeight, iStrideU);
  xSetValue(pucBufferU, iStrideU,   iGlobWidth, iGlobHeight, (unsigned char)DEFAULTU);
  xCopyFromImageBuffer(pucBufferU,  iOutWidth,  iOutHeight, iStrideU, 0, 255);

  //===== chroma cr =====
  ptr = &pucBufferV[iPosY * iStrideV + iPosX];
  xCopyToImageBuffer  ( ptr, iOutWidth,  iOutHeight, iStrideV);
  xSetValue(pucBufferV, iStrideV,   iGlobWidth, iGlobHeight, (unsigned char)DEFAULTV);
  xCopyFromImageBuffer( pucBufferV, iOutWidth,  iOutHeight, iStrideV, 0, 255);
}

__inline
void
DownConvert::xSetValue ( unsigned char* pucBuffer, int iStride, int iWidth, int iHeight, unsigned char value )
{
  for (int y=0; y<iHeight; y++)
    memset(&pucBuffer[y*iStride], value, iWidth);
}

__inline
void
DownConvert::xUpsampling( ResizeParameters* pcParameters,
                          bool bLuma, int type
                          )
{
  switch (type)
    {
    case 3:
      xUpsampling1(pcParameters, bLuma);
      break;
    case 4:
      xUpsampling2(pcParameters, bLuma);
      break;

    case 0:
      xUpsampling3(pcParameters, bLuma);
      break;
    }
}

__inline
void
DownConvert::xDownsampling( int iWidth,       // high-resolution width
                            int iHeight,      // high-resolution height
                            int aiFilter[] )  // downsampling filter [15coeff+sum]
{
  int i, j, k, im7, im6, im5, im4, im3, im2, im1, i0, ip1, ip2, ip3, ip4, ip5, ip6, ip7;
  int div = aiFilter[15]*aiFilter[15];
  int add = div / 2;


  //========== horizontal downsampling ===========
  for( j = 0; j < iHeight; j++ ) 
  {
    int*  piSrc = &m_paiImageBuffer[j*m_iImageStride];
    //----- down sample row -----
    for ( i = 0; i < iWidth/2; i++ )
    {
    	k   = i*2;
      im7 = (k<       7) ? 0   : k     -7;
      im6 = (k<       6) ? 0   : k     -6;
      im5 = (k<       5) ? 0   : k     -5;
      im4 = (k<       4) ? 0   : k     -4;
      im3 = (k<       3) ? 0   : k     -3;
      im2 = (k<       2) ? 0   : k     -2;
      im1 = (k<       1) ? 0   : k     -1;
      i0  = (k<iWidth  ) ? k   : iWidth-1;
      ip1 = (k<iWidth-1) ? k+1 : iWidth-1;
      ip2 = (k<iWidth-2) ? k+2 : iWidth-1;
      ip3 = (k<iWidth-3) ? k+3 : iWidth-1;
      ip4 = (k<iWidth-4) ? k+4 : iWidth-1;
      ip5 = (k<iWidth-5) ? k+5 : iWidth-1;
      ip6 = (k<iWidth-6) ? k+6 : iWidth-1;
      ip7 = (k<iWidth-7) ? k+7 : iWidth-1;

      m_paiTmp1dBuffer[i] = aiFilter[ 0]*piSrc[im7]
			                    + aiFilter[ 1]*piSrc[im6]
			                    + aiFilter[ 2]*piSrc[im5]
			                    + aiFilter[ 3]*piSrc[im4]
			                    + aiFilter[ 4]*piSrc[im3]
			                    + aiFilter[ 5]*piSrc[im2]
			                    + aiFilter[ 6]*piSrc[im1]
			                    + aiFilter[ 7]*piSrc[i0 ]
			                    + aiFilter[ 8]*piSrc[ip1]
			                    + aiFilter[ 9]*piSrc[ip2]
			                    + aiFilter[10]*piSrc[ip3]
			                    + aiFilter[11]*piSrc[ip4]
			                    + aiFilter[12]*piSrc[ip5]
			                    + aiFilter[13]*piSrc[ip6]				
			                    + aiFilter[14]*piSrc[ip7];
    }
    //----- copy row back to image buffer -----
    ::memcpy( piSrc, m_paiTmp1dBuffer, (iWidth/2)*sizeof(int) );
  }

  //=========== vertical downsampling ===========
  for( j = 0; j < iWidth/2; j++ )
  {
    int*  piSrc = &m_paiImageBuffer[j];
    //----- down sample column -----
    for( i = 0; i < iHeight/2; i++)
    {
	    k   = i*2;         
      im7 = ( (k<        7) ? 0   : k      -7 ) * m_iImageStride;
      im6 = ( (k<        6) ? 0   : k      -6 ) * m_iImageStride;
      im5 = ( (k<        5) ? 0   : k      -5 ) * m_iImageStride;
      im4 = ( (k<        4) ? 0   : k      -4 ) * m_iImageStride;
      im3 = ( (k<        3) ? 0   : k      -3 ) * m_iImageStride;
      im2 = ( (k<        2) ? 0   : k      -2 ) * m_iImageStride;
      im1 = ( (k<        1) ? 0   : k      -1 ) * m_iImageStride;
    	i0  = ( (k<iHeight  ) ? k   : iHeight-1 ) * m_iImageStride;
      ip1 = ( (k<iHeight-1) ? k+1 : iHeight-1 ) * m_iImageStride;
      ip2 = ( (k<iHeight-2) ? k+2 : iHeight-1 ) * m_iImageStride;
      ip3 = ( (k<iHeight-3) ? k+3 : iHeight-1 ) * m_iImageStride;
      ip4 = ( (k<iHeight-4) ? k+4 : iHeight-1 ) * m_iImageStride;
      ip5 = ( (k<iHeight-5) ? k+5 : iHeight-1 ) * m_iImageStride;
      ip6 = ( (k<iHeight-6) ? k+6 : iHeight-1 ) * m_iImageStride;
      ip7 = ( (k<iHeight-7) ? k+7 : iHeight-1 ) * m_iImageStride;

      m_paiTmp1dBuffer[i] = aiFilter[ 0]*piSrc[im7]
			                    + aiFilter[ 1]*piSrc[im6]
			                    + aiFilter[ 2]*piSrc[im5]
			                    + aiFilter[ 3]*piSrc[im4]
			                    + aiFilter[ 4]*piSrc[im3]
			                    + aiFilter[ 5]*piSrc[im2]
			                    + aiFilter[ 6]*piSrc[im1]
			                    + aiFilter[ 7]*piSrc[i0 ]
			                    + aiFilter[ 8]*piSrc[ip1]
			                    + aiFilter[ 9]*piSrc[ip2]
			                    + aiFilter[10]*piSrc[ip3]
			                    + aiFilter[11]*piSrc[ip4]
			                    + aiFilter[12]*piSrc[ip5]
			                    + aiFilter[13]*piSrc[ip6]				
			                    + aiFilter[14]*piSrc[ip7];
    }
    //----- scale and copy back to image buffer -----
    for( i = 0; i < iHeight/2; i++ )
    {
      piSrc[i*m_iImageStride] = ( m_paiTmp1dBuffer[i] + add ) / div;
    }
  }
}

__inline
void
DownConvert::xUpsampling( int iWidth,       // low-resolution width
                          int iHeight,      // low-resolution height
                          int aiFilter[] )  // downsampling filter [15coeff+sum]
{
  int i, j, im3, im2, im1, i0, ip1, ip2, ip3, ip4;
  int div = ( aiFilter[15]*aiFilter[15] ) / 4;
  int add = div / 2;


  //========== vertical upsampling ===========
  for( j = 0; j < iWidth; j++ ) 
  {
    int*  piSrc = &m_paiImageBuffer[j];
    //----- upsample column -----
    for( i = 0; i < iHeight; i++)
    {
      im3       = ( (i<        3) ? 0   : i      -3 ) * m_iImageStride;
      im2       = ( (i<        2) ? 0   : i      -2 ) * m_iImageStride;
      im1       = ( (i<        1) ? 0   : i      -1 ) * m_iImageStride;
    	i0        = ( (i<iHeight  ) ? i   : iHeight-1 ) * m_iImageStride;
      ip1       = ( (i<iHeight-1) ? i+1 : iHeight-1 ) * m_iImageStride;
      ip2       = ( (i<iHeight-2) ? i+2 : iHeight-1 ) * m_iImageStride;
      ip3       = ( (i<iHeight-3) ? i+3 : iHeight-1 ) * m_iImageStride;
      ip4       = ( (i<iHeight-4) ? i+4 : iHeight-1 ) * m_iImageStride;

      //--- even sample ---
      m_paiTmp1dBuffer[2*i+0] = aiFilter[13]*piSrc[im3]
			                        + aiFilter[11]*piSrc[im2]
				                      + aiFilter[ 9]*piSrc[im1]		   
				                      + aiFilter[ 7]*piSrc[i0 ]
			                        + aiFilter[ 5]*piSrc[ip1]
			                        + aiFilter[ 3]*piSrc[ip2]
			                        + aiFilter[ 1]*piSrc[ip3];
      //--- odd sample ---
      m_paiTmp1dBuffer[2*i+1] = aiFilter[14]*piSrc[im3]
			                        + aiFilter[12]*piSrc[im2]
				                      + aiFilter[10]*piSrc[im1]		   
				                      + aiFilter[ 8]*piSrc[i0 ]
			                        + aiFilter[ 6]*piSrc[ip1]
			                        + aiFilter[ 4]*piSrc[ip2]
			                        + aiFilter[ 2]*piSrc[ip3]
			                        + aiFilter[ 0]*piSrc[ip4];
    }
    //----- copy back to image buffer -----
    for( i = 0; i < iHeight*2; i++ )
    {
      piSrc[i*m_iImageStride] = m_paiTmp1dBuffer[i];
    }
  }


  //========== horizontal upsampling ==========
  for( j = 0; j < iHeight*2; j++ ) 
  {
    int*  piSrc = &m_paiImageBuffer[j*m_iImageStride];
    //----- upsample row -----
    for ( i = 0; i < iWidth; i++ )
    {
      im3     = (i<       3) ? 0   : i     -3;
      im2     = (i<       2) ? 0   : i     -2;
      im1     = (i<       1) ? 0   : i     -1;
      i0      = (i<iWidth  ) ? i   : iWidth-1;
      ip1     = (i<iWidth-1) ? i+1 : iWidth-1;
      ip2     = (i<iWidth-2) ? i+2 : iWidth-1;
      ip3     = (i<iWidth-3) ? i+3 : iWidth-1;
      ip4     = (i<iWidth-4) ? i+4 : iWidth-1;

      //--- even sample ---
      m_paiTmp1dBuffer[2*i+0] = aiFilter[13]*piSrc[im3]
			                        + aiFilter[11]*piSrc[im2]
				                      + aiFilter[ 9]*piSrc[im1]		   
				                      + aiFilter[ 7]*piSrc[i0 ]
			                        + aiFilter[ 5]*piSrc[ip1]
			                        + aiFilter[ 3]*piSrc[ip2]
			                        + aiFilter[ 1]*piSrc[ip3];
      //--- odd sample ---
      m_paiTmp1dBuffer[2*i+1] = aiFilter[14]*piSrc[im3]
			                        + aiFilter[12]*piSrc[im2]
				                      + aiFilter[10]*piSrc[im1]		   
				                      + aiFilter[ 8]*piSrc[i0 ]
			                        + aiFilter[ 6]*piSrc[ip1]
			                        + aiFilter[ 4]*piSrc[ip2]
			                        + aiFilter[ 2]*piSrc[ip3]
			                        + aiFilter[ 0]*piSrc[ip4];
    }
    //----- scale and copy back to image buffer -----
    for( i = 0; i < iWidth*2; i++ )
    {
      piSrc[i] = ( m_paiTmp1dBuffer[i] + add ) / div;
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
//JVT-R006
////////////////////////////////////////////////////////////////////////////////////////
__inline
void
DownConvert::xDownsampling3( int input_width, int input_height, int output_width, int output_height,
                             int crop_x0, int crop_y0, int crop_w, int crop_h,
                             int input_chroma_phase_shift_x, int input_chroma_phase_shift_y,
                             int output_chroma_phase_shift_x, int output_chroma_phase_shift_y ) 
{
  const int filter16[8][16][12] = {   // sine, N = 3
                                    { // D = 1
                                      {0,0,0,0,0,128,0,0,0,0,0,0},
                                      {0,0,0,2,-6,127,7,-2,0,0,0,0},
                                      {0,0,0,3,-12,125,16,-5,1,0,0,0},
                                      {0,0,0,4,-16,120,26,-7,1,0,0,0},
                                      {0,0,0,5,-18,114,36,-10,1,0,0,0},
                                      {0,0,0,5,-20,107,46,-12,2,0,0,0},
                                      {0,0,0,5,-21,99,57,-15,3,0,0,0},
                                      {0,0,0,5,-20,89,68,-18,4,0,0,0},
                                      {0,0,0,4,-19,79,79,-19,4,0,0,0},
                                      {0,0,0,4,-18,68,89,-20,5,0,0,0},
                                      {0,0,0,3,-15,57,99,-21,5,0,0,0},
                                      {0,0,0,2,-12,46,107,-20,5,0,0,0},
                                      {0,0,0,1,-10,36,114,-18,5,0,0,0},
                                      {0,0,0,1,-7,26,120,-16,4,0,0,0},
                                      {0,0,0,1,-5,16,125,-12,3,0,0,0},
                                      {0,0,0,0,-2,7,127,-6,2,0,0,0}
                                    },
                                    { // D = 1.5
                                      {0,2,0,-14,33,86,33,-14,0,2,0,0},
                                      {0,1,1,-14,29,85,38,-13,-1,2,0,0},
                                      {0,1,2,-14,24,84,43,-12,-2,2,0,0},
                                      {0,1,2,-13,19,83,48,-11,-3,2,0,0},
                                      {0,0,3,-13,15,81,53,-10,-4,3,0,0},
                                      {0,0,3,-12,11,79,57,-8,-5,3,0,0},
                                      {0,0,3,-11,7,76,62,-5,-7,3,0,0},
                                      {0,0,3,-10,3,73,65,-2,-7,3,0,0},
                                      {0,0,3,-9,0,70,70,0,-9,3,0,0},
                                      {0,0,3,-7,-2,65,73,3,-10,3,0,0},
                                      {0,0,3,-7,-5,62,76,7,-11,3,0,0},
                                      {0,0,3,-5,-8,57,79,11,-12,3,0,0},
                                      {0,0,3,-4,-10,53,81,15,-13,3,0,0},
                                      {0,0,2,-3,-11,48,83,19,-13,2,1,0},
                                      {0,0,2,-2,-12,43,84,24,-14,2,1,0},
                                      {0,0,2,-1,-13,38,85,29,-14,1,1,0}
                                    },
#if KAISER_FLT
                                    { // Kaiser, N=3, D=2, beta=4
                                      {2,0,-9,0,39,64,39,0,-9,0,2,0},
                                      {2,0,-8,-2,36,64,41,2,-9,0,2,0},
                                      {2,1,-8,-3,33,63,44,4,-9,-1,2,0},
                                      {1,1,-7,-4,31,63,46,5,-9,-1,2,0},
                                      {1,1,-7,-5,28,62,49,8,-9,-2,2,0},
                                      {1,2,-6,-6,25,61,51,10,-9,-3,2,0},
                                      {1,2,-6,-7,22,60,53,12,-9,-3,2,1},
                                      {1,2,-5,-8,20,58,55,15,-9,-4,2,1},
                                      {1,2,-4,-8,17,56,56,17,-8,-4,2,1},
                                      {1,2,-4,-9,15,55,58,20,-8,-5,2,1},
                                      {1,2,-3,-9,12,53,60,22,-7,-6,2,1},
                                      {0,2,-3,-9,10,51,61,25,-6,-6,2,1},
                                      {0,2,-2,-9,8,49,62,28,-5,-7,1,1},
                                      {0,2,-1,-9,5,46,63,31,-4,-7,1,1},
                                      {0,2,-1,-9,4,44,63,33,-3,-8,1,2},
                                      {0,2,0,-9,2,41,64,36,-2,-8,0,2}
                                    },
#else
                                    { // D = 2
                                      {2,0,-10,0,40,64,40,0,-10,0,2,0},
                                      {2,1,-9,-2,37,64,42,2,-10,-1,2,0},
                                      {2,1,-9,-3,34,64,44,4,-10,-1,2,0},
                                      {2,1,-8,-5,31,63,47,6,-10,-2,3,0},
                                      {1,2,-8,-6,29,62,49,8,-10,-2,3,0},
                                      {1,2,-7,-7,26,61,52,10,-10,-3,3,0},
                                      {1,2,-6,-8,23,60,54,13,-10,-4,3,0},
                                      {1,2,-6,-9,20,59,56,15,-10,-4,3,1},
                                      {1,2,-5,-9,18,57,57,18,-9,-5,2,1},
                                      {1,3,-4,-10,15,56,59,20,-9,-6,2,1},
                                      {0,3,-4,-10,13,54,60,23,-8,-6,2,1},
                                      {0,3,-3,-10,10,52,61,26,-7,-7,2,1},
                                      {0,3,-2,-10,8,49,62,29,-6,-8,2,1},
                                      {0,3,-2,-10,6,47,63,31,-5,-8,1,2},
                                      {0,2,-1,-10,4,44,64,34,-3,-9,1,2},
                                      {0,2,-1,-10,2,42,64,37,-2,-9,1,2}
                                    },
#endif
                                    { // D = 2.5
#if TAP_10_FLT
                                      {0,-4,-7,11,38,52,38,11,-7,-4,0,0},
                                      {0,-4,-7,9,37,51,40,13,-6,-7,2,0},
                                      {0,-3,-7,8,35,51,41,14,-5,-7,1,0},
                                      {0,-2,-8,6,33,51,42,16,-5,-7,2,0},
                                      {0,-2,-8,5,32,50,43,18,-4,-8,2,0},
                                      {0,-2,-8,4,30,50,45,19,-3,-8,1,0},
                                      {0,-1,-8,2,28,49,46,21,-2,-8,1,0},
                                      {0,-1,-8,1,26,49,47,23,-1,-8,0,0},
                                      {0,0,-8,0,24,48,48,24,0,-8,0,0},
                                      {0,0,-8,-1,23,47,49,26,1,-8,-1,0},
                                      {0,1,-8,-2,21,46,49,28,2,-8,-1,0},
                                      {0,1,-8,-3,19,45,50,30,4,-8,-2,0},
                                      {0,2,-8,-4,18,43,50,32,5,-8,-2,0},
                                      {0,2,-7,-5,16,42,51,33,6,-8,-2,0},
                                      {0,1,-7,-5,14,41,51,35,8,-7,-3,0},
                                      {0,2,-7,-6,13,40,51,37,9,-7,-4,0}
#else
                                      {3,-7,-7,11,38,52,38,11,-7,-7,0,3},
                                      {3,-6,-7,9,36,51,39,13,-6,-7,0,3},
                                      {3,-6,-7,8,35,51,41,14,-5,-7,-1,2},
                                      {3,-5,-8,6,33,51,42,16,-5,-7,-1,3},
                                      {3,-5,-8,5,32,50,43,18,-4,-8,-1,3},
                                      {3,-4,-8,3,30,50,45,19,-3,-8,-2,3},
                                      {3,-4,-8,2,28,49,46,21,-2,-8,-2,3},
                                      {3,-4,-8,1,26,49,47,23,-1,-8,-3,3},
                                      {3,-3,-8,0,24,48,48,24,0,-8,-3,3},
                                      {3,-3,-8,-1,23,47,49,26,1,-8,-4,3},
                                      {3,-2,-8,-2,21,46,49,28,2,-8,-4,3},
                                      {3,-2,-8,-3,19,45,50,30,3,-8,-4,3},
                                      {3,-1,-8,-4,18,43,50,32,5,-8,-5,3},
                                      {3,-1,-7,-5,16,42,51,33,6,-8,-5,3},
                                      {2,-1,-7,-5,14,41,51,35,8,-7,-6,3},
                                      {3,0,-7,-6,13,39,51,36,9,-7,-6,3}
#endif
#if 0  // N=4 version tested in Nice
                                      {5,-8,-7,11,38,51,38,11,-7,-8,0,4},// 0
                                      {4,-7,-8,10,37,51,40,13,-7,-8,-1,4},
                                      {4,-7,-8,8,35,51,41,15,-6,-8,-1,4},
                                      {4,-7,-8,7,34,51,42,16,-5,-9,-1,4},
                                      {5,-6,-9,5,32,50,44,18,-4,-9,-2,4},// 1/4
                                      {5,-6,-9,4,31,50,45,20,-4,-10,-3,5},
                                      {5,-5,-10,3,29,49,46,22,-3,-10,-3,5},
                                      {5,-5,-9,1,27,48,47,23,-1,-9,-4,5},
                                      {5,-4,-10,0,25,48,48,25,0,-10,-4,5},// 1/2
                                      {5,-4,-9,-1,23,47,48,27,1,-9,-5,5},
                                      {5,-3,-10,-3,22,46,49,29,3,-10,-5,5},
                                      {5,-3,-10,-4,20,45,50,31,4,-9,-6,5},
                                      {4,-2,-9,-4,18,44,50,32,5,-9,-6,5},
                                      {4,-1,-9,-5,16,42,51,34,7,-8,-7,4},
                                      {4,-1,-8,-6,15,41,51,35,8,-8,-7,4},
                                      {4,-1,-8,-7,13,40,51,37,10,-8,-7,4}
#endif
                                    },
                                    { // D = 3
                                      {-2,-7,0,17,35,43,35,17,0,-7,-5,2},
                                      {-2,-7,-1,16,34,43,36,18,1,-7,-5,2},
                                      {-1,-7,-1,14,33,43,36,19,1,-6,-5,2},
                                      {-1,-7,-2,13,32,42,37,20,3,-6,-5,2},
                                      {0,-7,-3,12,31,42,38,21,3,-6,-5,2},
                                      {0,-7,-3,11,30,42,39,23,4,-6,-6,1},
                                      {0,-7,-4,10,29,42,40,24,5,-6,-6,1},
                                      {1,-7,-4,9,27,41,40,25,6,-5,-6,1},
                                      {1,-6,-5,7,26,41,41,26,7,-5,-6,1},
                                      {1,-6,-5,6,25,40,41,27,9,-4,-7,1},
                                      {1,-6,-6,5,24,40,42,29,10,-4,-7,0},
                                      {1,-6,-6,4,23,39,42,30,11,-3,-7,0},
                                      {2,-5,-6,3,21,38,42,31,12,-3,-7,0},
                                      {2,-5,-6,3,20,37,42,32,13,-2,-7,-1},
                                      {2,-5,-6,1,19,36,43,33,14,-1,-7,-1},
                                      {2,-5,-7,1,18,36,43,34,16,-1,-7,-2}
                                    },
                                    { // D = 3.5
                                      {-6,-3,5,19,31,36,31,19,5,-3,-6,0},
                                      {-6,-4,4,18,31,37,32,20,6,-3,-6,-1},
                                      {-6,-4,4,17,30,36,33,21,7,-3,-6,-1},
                                      {-5,-5,3,16,30,36,33,22,8,-2,-6,-2},
                                      {-5,-5,2,15,29,36,34,23,9,-2,-6,-2},
                                      {-5,-5,2,15,28,36,34,24,10,-2,-6,-3},
                                      {-4,-5,1,14,27,36,35,24,10,-1,-6,-3},
                                      {-4,-5,0,13,26,35,35,25,11,0,-5,-3},
                                      {-4,-6,0,12,26,36,36,26,12,0,-6,-4},
                                      {-3,-5,0,11,25,35,35,26,13,0,-5,-4},
                                      {-3,-6,-1,10,24,35,36,27,14,1,-5,-4},
                                      {-3,-6,-2,10,24,34,36,28,15,2,-5,-5},
                                      {-2,-6,-2,9,23,34,36,29,15,2,-5,-5},
                                      {-2,-6,-2,8,22,33,36,30,16,3,-5,-5},
                                      {-1,-6,-3,7,21,33,36,30,17,4,-4,-6},
                                      {-1,-6,-3,6,20,32,37,31,18,4,-4,-6}
                                    },
                                    { // D = 4
                                      {-9,0,9,20,28,32,28,20,9,0,-9,0},
                                      {-9,0,8,19,28,32,29,20,10,0,-4,-5},
                                      {-9,-1,8,18,28,32,29,21,10,1,-4,-5},
                                      {-9,-1,7,18,27,32,30,22,11,1,-4,-6},
                                      {-8,-2,6,17,27,32,30,22,12,2,-4,-6},
                                      {-8,-2,6,16,26,32,31,23,12,2,-4,-6},
                                      {-8,-2,5,16,26,31,31,23,13,3,-3,-7},
                                      {-8,-3,5,15,25,31,31,24,14,4,-3,-7},
                                      {-7,-3,4,14,25,31,31,25,14,4,-3,-7},
                                      {-7,-3,4,14,24,31,31,25,15,5,-3,-8},
                                      {-7,-3,3,13,23,31,31,26,16,5,-2,-8},
                                      {-6,-4,2,12,23,31,32,26,16,6,-2,-8},
                                      {-6,-4,2,12,22,30,32,27,17,6,-2,-8},
                                      {-6,-4,1,11,22,30,32,27,18,7,-1,-9},
                                      {-5,-4,1,10,21,29,32,28,18,8,-1,-9},
                                      {-5,-4,0,10,20,29,32,28,19,8,0,-9}
                                    },
                                    { // D = 6
                                      {-6,8,13,18,20,22,20,18,13,8,4,-10},
                                      {-6,8,13,17,20,21,20,18,13,9,4,-9},
                                      {-6,8,12,17,20,21,20,18,14,9,4,-9},
                                      {-7,7,12,17,20,21,21,18,14,9,5,-9},
                                      {-7,7,12,16,20,21,21,18,14,10,5,-9},
                                      {-7,7,12,16,20,21,21,18,14,10,5,-9},
                                      {-8,7,11,16,20,21,21,19,15,10,5,-9},
                                      {-8,6,11,16,19,21,21,19,15,11,6,-9},
                                      {-8,6,11,15,19,21,21,19,15,11,6,-8},
                                      {-9,6,11,15,19,21,21,19,16,11,6,-8},
                                      {-9,5,10,15,19,21,21,20,16,11,7,-8},
                                      {-9,5,10,14,18,21,21,20,16,12,7,-7},
                                      {-9,5,10,14,18,21,21,20,16,12,7,-7},
                                      {-9,5,9,14,18,21,21,20,17,12,7,-7},
                                      {-9,4,9,14,18,20,21,20,17,12,8,-6},
                                      {-9,4,9,13,18,20,21,20,17,13,8,-6}
                                    }
                                  };
  int i, j, k, m, *px, *py, x, y, x16, y16, filter;

  // initialization
  px = new int[output_width];
  py = new int[output_height];

  //========== horizontal downsampling ===========
 // if(crop_w*2 > 7*output_width) filter = 7;
  //else 
  if(crop_w*7 > 20*output_width) filter = 6;
  else if(crop_w*2 > 5*output_width) filter = 5;
  else if(crop_w*1 > 2*output_width) filter = 4;
  else if(crop_w*3 > 5*output_width) filter = 3;
  else if(crop_w*4 > 5*output_width) filter = 2;
  else if(crop_w*19 > 20*output_width) filter = 1;
  else filter = 0;

  for( i = 0; i < output_width; i++ )
  {
    px[i] = 16*crop_x0 + ( i*crop_w*16 + 4*(2+output_chroma_phase_shift_x)*crop_w - 4*(2+input_chroma_phase_shift_x)*output_width + output_width/2) / output_width;
  }
  for( j = 0; j < input_height; j++ ) 
  {
    int*  piSrc = &m_paiImageBuffer[j*m_iImageStride];
    for( i = 0; i < output_width; i++ ){
      x16 = px[i]&0x0f;
      x = px[i]>>4;
      m_paiTmp1dBuffer[i] = 0;
      for( k=0; k<12; k++) {
        m = x - 5 + k;
        if( m<0 ) m = 0;
        else if( m>(input_width-1) ) m=input_width-1;
        m_paiTmp1dBuffer[i] += filter16[filter][x16][k]*piSrc[m];
      }
    }
    //----- copy row back to image buffer -----
    ::memcpy( piSrc, m_paiTmp1dBuffer, output_width*sizeof(int) );
  }

  //========== vertical downsampling ===========
  //if     (crop_h*2 > 7*output_height) filter = 7;
  //else 
  if(crop_h*7 > 20*output_height) filter = 6;
  else if(crop_h*2 > 5*output_height) filter = 5;
  else if(crop_h*1 > 2*output_height) filter = 4;
  else if(crop_h*3 > 5*output_height) filter = 3;
  else if(crop_h*4 > 5*output_height) filter = 2;
  else if(crop_h*19 > 20*output_height) filter = 1;
  else filter = 0;

  for( j = 0; j < output_height; j++ )
  {
    py[j] = 16*crop_y0 + ( j*crop_h*16 + 4*(2+output_chroma_phase_shift_y)*crop_h - 4*(2+input_chroma_phase_shift_y)*output_height + output_height/2 ) / output_height;
  }
  for( i = 0; i < output_width; i++ ) 
  {
    int*  piSrc = &m_paiImageBuffer[i];
    for( j = 0; j < output_height; j++ ){
      y16 = py[j]&0x0f;
      y = py[j]>>4;
      m_paiTmp1dBuffer[j] = 0;
      for( k=0; k<12; k++) {
        m = y - 5 + k;
        if( m<0 ) m = 0;
        else if( m>(input_height-1) ) m=input_height-1;
        m_paiTmp1dBuffer[j] += filter16[filter][y16][k]*piSrc[m*m_iImageStride];
      }
    }
    //----- scale and copy back to image buffer -----
    for( j = 0; j < output_height; j++ )
    {
      piSrc[j*m_iImageStride] = ( m_paiTmp1dBuffer[j] + (1<<13) ) / (1<<14);
    }
  }
  
  // free memory
  delete [] px;
  delete [] py;
}

// =================================================================================
//   INTRA 2
// =================================================================================
//TMM_ESS_UNIFIED {
__inline
void
DownConvert::xInitFilterTmm2 (int iMaxDim )
{
 if(m_aiTmp1dBufferInHalfpel == NULL)
  {
  m_aiTmp1dBufferInHalfpel = new int [iMaxDim];
  m_aiTmp1dBufferInQ1pel   = new int [iMaxDim];
  m_aiTmp1dBufferInQ3pel   = new int [iMaxDim];
  }
}


__inline
void
DownConvert::xDestroyFilterTmm2 ( )
{
  if  (NULL!=m_aiTmp1dBufferInHalfpel) 
  {
  delete [] m_aiTmp1dBufferInHalfpel; m_aiTmp1dBufferInHalfpel=NULL;
  delete [] m_aiTmp1dBufferInQ1pel;	m_aiTmp1dBufferInQ1pel  =NULL;
  delete [] m_aiTmp1dBufferInQ3pel;	m_aiTmp1dBufferInQ3pel  =NULL;
  }
}
//TMM_ESS_UNIFIED }
__inline
void
DownConvert::xUpsampling2( ResizeParameters* pcParameters, bool bLuma)

{
  int iInWidth = pcParameters->m_iInWidth;
  int iInHeight = pcParameters->m_iInHeight;
  int iOutWidth = pcParameters->m_iOutWidth;
  int iOutHeight = pcParameters->m_iOutHeight;

  if (!bLuma)
  {
    iInWidth    >>= 1;
    iInHeight   >>= 1;
    iOutWidth   >>= 1;
    iOutHeight  >>= 1;
  }

  // ===== vertical upsampling =====
  for (int xin=0; xin<iInWidth; xin++)
    {
      int* piSrc = &m_paiImageBuffer[xin];
      for (int yin=0; yin<iInHeight; yin++)
        m_paiTmp1dBuffer[yin] = piSrc[yin * m_iImageStride];

      xUpsamplingData2(iInHeight, iOutHeight); 
      
      for(int yout = 0; yout < iOutHeight; yout++ )
        piSrc[yout*m_iImageStride] = m_paiTmp1dBufferOut[yout];
    }
  
  // ===== horizontal upsampling =====
  for (int yout=0; yout<iOutHeight; yout++)
    {
      int* piSrc = &m_paiImageBuffer[yout * m_iImageStride];
      for (int xin=0; xin<iInWidth; xin++)
        m_paiTmp1dBuffer[xin] = piSrc[xin];

      xUpsamplingData2(iInWidth, iOutWidth);

      for( int i = 0; i < iOutWidth; i++ )
        piSrc[i] = ( m_paiTmp1dBufferOut[i] + 512 ) >> 10;
  }
}

__inline
void
DownConvert::xUpsamplingData2 ( int iInLength , int iOutLength )
{
  int  *Tmp1dBufferInHalfpel = m_aiTmp1dBufferInHalfpel;
  int  *Tmp1dBufferInQ1pel = m_aiTmp1dBufferInQ1pel;
  int  *Tmp1dBufferInQ3pel = m_aiTmp1dBufferInQ3pel;

  int x,y;
  int iTemp;

  // half pel samples (6 taps 1 -5 20 20 -5 1)
  for(  x = 0; x < iInLength ; x++)
    {
      y=x;
      iTemp  = m_paiTmp1dBuffer[y];
      y = (x+1 < iInLength ? x+1 : iInLength -1);
      iTemp += m_paiTmp1dBuffer[y];
      iTemp  = iTemp << 2;
      y = (x-1 >= 0  ? x-1 : 0);
      iTemp -= m_paiTmp1dBuffer[y];
      y = (x+2 < iInLength ? x+2 : iInLength -1);
      iTemp -= m_paiTmp1dBuffer[y];
      iTemp += iTemp << 2;
      y = (x-2 >= 0  ? x-2 : 0);
      iTemp += m_paiTmp1dBuffer[y];
      y = (x+3 < iInLength ? x+3 : iInLength -1);
      iTemp += m_paiTmp1dBuffer[y];
      Tmp1dBufferInHalfpel[x] = iTemp;
    }

  // 1/4 pel samples
  for( x = 0; x < iInLength-1 ; x++)
    {
      Tmp1dBufferInQ1pel[x] = ( (m_paiTmp1dBuffer[x]<<5) + Tmp1dBufferInHalfpel[x] + 1) >> 1;
      Tmp1dBufferInQ3pel[x] = ( (m_paiTmp1dBuffer[x+1]<<5) + Tmp1dBufferInHalfpel[x] + 1) >> 1;
    }
  Tmp1dBufferInQ1pel[iInLength-1] = ( (m_paiTmp1dBuffer[iInLength-1]<<5) + Tmp1dBufferInHalfpel[iInLength-1] + 1) >> 1;
  Tmp1dBufferInQ3pel[iInLength-1] = Tmp1dBufferInHalfpel[iInLength-1] ;

  // generic interpolation to nearest 1/4 pel position
  for (int iout=0; iout<iOutLength; iout++)
    {
      double    dpos0 = ((double)iout * iInLength / iOutLength);
      int       ipos0 = (int)dpos0;
      double    rpos0 = dpos0 - ipos0;

      int iIndex = (int) (8 * rpos0);
      switch (iIndex)
        {
        case 0:
          m_paiTmp1dBufferOut[iout] =  m_paiTmp1dBuffer[ipos0] << 5; // original pel value
          break;
			
        case 1:
        case 2:
          m_paiTmp1dBufferOut[iout] =  Tmp1dBufferInQ1pel[ipos0]; // 1/4 pel value
          break;
			
        case 3:
        case 4:
          m_paiTmp1dBufferOut[iout] =  Tmp1dBufferInHalfpel[ipos0]; // half pel value
          break;
			
        case 5:
        case 6:
          m_paiTmp1dBufferOut[iout] =  Tmp1dBufferInQ3pel[ipos0]; // 1/4 pel value
          break;
			
        case 7:
          int ipos1 = (ipos0+1 < iInLength) ? ipos0+1 : ipos0;
          m_paiTmp1dBufferOut[iout] =  m_paiTmp1dBuffer[ipos1] << 5; // original pel value
          break;

        }
    }  
}




// =================================================================================
//   INTRA 1 Lanczos
// =================================================================================
#define TMM_TABLE_SIZE 512
#define TMM_FILTER_WINDOW_SIZE 3

#define NFACT    12
#define VALFACT  (1l<<NFACT)
#define MASKFACT (VALFACT-1)

//*************************************************************************
// lanczos filter coeffs computation
__inline
void
DownConvert::xInitFilterTmm1 ()
{  
  xDestroyFilterTmm1();

  const double pi = 3.14159265359;
  m_padFilter = new long[TMM_TABLE_SIZE];
  m_padFilter[0] = VALFACT;

  for (int i=1; i<TMM_TABLE_SIZE; i++)
    {
      double x = ((double)i/TMM_TABLE_SIZE) * TMM_FILTER_WINDOW_SIZE;
      double pix = pi*x;
      double pixw = pix/TMM_FILTER_WINDOW_SIZE;

      m_padFilter[i] = (long)(sin(pix)/pix * sin(pixw)/pixw * VALFACT);
    } 
}

 __inline
void
DownConvert::xDestroyFilterTmm1 ( )
{
 if(m_padFilter) 
  {
  delete [] m_padFilter;
  m_padFilter=NULL; 
}
}



//*************************************************************************
__inline
void
DownConvert::xUpsampling1( ResizeParameters* pcParameters,
                           bool bLuma)

{
  int iInWidth = pcParameters->m_iInWidth;
  int iInHeight = pcParameters->m_iInHeight;
  int iOutWidth = pcParameters->m_iOutWidth;
  int iOutHeight = pcParameters->m_iOutHeight;
  if (!bLuma)
  {
    iInWidth    >>= 1;
    iInHeight   >>= 1;
    iOutWidth   >>= 1;
    iOutHeight  >>= 1;
  }

  int iNumerator = 1;
  int iDenominator = 1;

  long spos;

  // ===== vertical upsampling =====
  xComputeNumeratorDenominator(iInHeight,iOutHeight,&iNumerator,&iDenominator);

  spos = (1<<NFACT) * iDenominator / iNumerator;

  for (int xin=0; xin<iInWidth; xin++)
    {
      int* piSrc = &m_paiImageBuffer[xin];
      for (int yin=0; yin<iInHeight; yin++)
        m_paiTmp1dBuffer[yin] = piSrc[yin * m_iImageStride];

      xUpsamplingData1(iInHeight, iOutHeight, spos);
      
      for(int yout = 0; yout < iOutHeight; yout++ )
        piSrc[yout*m_iImageStride] = m_paiTmp1dBufferOut[yout];
    }
  
  // ===== horizontal upsampling =====
  xComputeNumeratorDenominator(iInWidth,iOutWidth,&iNumerator,&iDenominator);

  spos = (1<<NFACT) * iDenominator / iNumerator;

  for (int yout=0; yout<iOutHeight; yout++)
    {
      int* piSrc = &m_paiImageBuffer[yout * m_iImageStride];
      for (int xin=0; xin<iInWidth; xin++)
        m_paiTmp1dBuffer[xin] = piSrc[xin];

      xUpsamplingData1(iInWidth, iOutWidth, spos);

      memcpy(piSrc, m_paiTmp1dBufferOut, iOutWidth*sizeof(int));
    }
}

__inline
void
DownConvert::xUpsamplingData1 ( int iInLength , int iOutLength , long spos )
{

  long dpos0 = -spos;
  for (int iout=0; iout<iOutLength; iout++)
    {
      dpos0 += spos;
    
      long rpos0 = dpos0 & MASKFACT;
      int ipos0 = dpos0 >> NFACT;
      if (rpos0 == 0) {
        m_paiTmp1dBufferOut[iout] = m_paiTmp1dBuffer[ipos0];
        continue;
      }

      int end = ipos0 + TMM_FILTER_WINDOW_SIZE;
      int begin = end - TMM_FILTER_WINDOW_SIZE*2;

      long sval = 0;
      long posi = ((begin-ipos0)<<NFACT) - rpos0;
      for (int i=begin; i<=end; i++, posi += VALFACT)
        {
          long fact = xGetFilter(posi);
          int val;
          if (i<0)               val = m_paiTmp1dBuffer[0];
          else if (i>=iInLength) val = m_paiTmp1dBuffer[iInLength-1];
          else                   val = m_paiTmp1dBuffer[i];
          sval += val * fact;
        }
      m_paiTmp1dBufferOut[iout] = sval>>NFACT;
    }

}

__inline
void
DownConvert::xComputeNumeratorDenominator ( int iInWidth , int iOutWidth ,
                                           int* iNumerator, int *iDenominator)
{
  int iA = 1;
	int iB = iOutWidth;
  int iC = iInWidth;
	while (iC != 0)
	{
		iA = iB;
		iB = iC;		
		iC = iA % iB;
	}

  *iNumerator = iOutWidth / iB;
  *iDenominator = iInWidth / iB;
}

__inline
long
DownConvert::xGetFilter ( long x )
{
  x = abs(x);
  int ind = (int)((x / TMM_FILTER_WINDOW_SIZE) * TMM_TABLE_SIZE) >> NFACT;
  if (ind >= TMM_TABLE_SIZE) return 0;
  return m_padFilter[ind];
}


#undef NFACT
#undef VALFACT
#undef MASKFACT
#undef TMM_TABLE_SIZE
#undef TMM_FILTER_WINDOW_SIZE

