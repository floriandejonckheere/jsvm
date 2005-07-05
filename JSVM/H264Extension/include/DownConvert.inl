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





#include <math.h>


__inline
DownConvert::DownConvert()
: m_iImageStride  ( 0 )
, m_paiImageBuffer( 0 )
#ifndef NO_MB_DATA_CTRL
, m_paiImageBuffer2( 0 )
#endif
, m_paiTmp1dBuffer( 0 )
, m_padFilter(0)        //TMM_ESS
, m_paiTmp1dBufferOut(0)//TMM_ESS
{
}


__inline
DownConvert::~DownConvert()
{
#ifndef NO_MB_DATA_CTRL
  delete [] m_paiImageBuffer2;
#endif
  delete [] m_paiImageBuffer;
  delete [] m_paiTmp1dBuffer;
 
  xDestroyFilterTmm(); // TMM_ESS
}


__inline
int
DownConvert::init( int iMaxWidth, int iMaxHeight )
{
  int iPicSize  =   iMaxWidth * iMaxHeight;
  int iMaxDim   = ( iMaxWidth > iMaxHeight ? iMaxWidth : iMaxHeight );

#ifndef NO_MB_DATA_CTRL
  delete [] m_paiImageBuffer2;
#endif
  delete [] m_paiImageBuffer;
  delete [] m_paiTmp1dBuffer;

  m_iImageStride    = iMaxWidth;
  m_paiImageBuffer  = new int [ iPicSize ];
  m_paiTmp1dBuffer  = new int [ iMaxDim  ];
  
  xInitFilterTmm(iMaxDim); // TMM_ESS
  
#ifndef NO_MB_DATA_CTRL
  m_paiImageBuffer2 = new int [ iPicSize ];
  return ( m_paiImageBuffer2 == 0 || m_paiImageBuffer == 0 || m_paiTmp1dBuffer == 0 );
#endif

  return ( m_paiImageBuffer == 0 || m_paiTmp1dBuffer == 0 );
}


__inline
int
DownConvert::xClip( int iValue, int imin, int imax )
{
  return ( iValue < imin ? imin : iValue > imax ? imax : iValue );
}


__inline
void
DownConvert::downsample( unsigned char* pucBufferY, int iStrideY,
                         unsigned char* pucBufferU, int iStrideU,
                         unsigned char* pucBufferV, int iStrideV,
                         int            iWidth,     int iHeight, int iStages )
{
  FILTER_DOWN

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
DownConvert::downsample( short* psBufferY, int iStrideY,
                         short* psBufferU, int iStrideU,
                         short* psBufferV, int iStrideV,
                         int    iWidth,    int iHeight, bool bClip, int iStages )
{
  FILTER_DOWN

  int   imin = ( bClip ?   0 : -32768 );
  int   imax = ( bClip ? 255 :  32767 );

  for( int i = iStages; i > 0; i-- )
  {
    //===== luma =====
    xCopyToImageBuffer  ( psBufferY, iWidth,   iHeight,   iStrideY );
    xDownsampling       (            iWidth,   iHeight,   piFilter );
    xCopyFromImageBuffer( psBufferY, iWidth/2, iHeight/2, iStrideY, imin, imax );
    //===== chroma cb =====
    xCopyToImageBuffer  ( psBufferU, iWidth/2, iHeight/2, iStrideU );
    xDownsampling       (            iWidth/2, iHeight/2, piFilter );
    xCopyFromImageBuffer( psBufferU, iWidth/4, iHeight/4, iStrideU, imin, imax );
    //===== chroma cr =====
    xCopyToImageBuffer  ( psBufferV, iWidth/2, iHeight/2, iStrideV );
    xDownsampling       (            iWidth/2, iHeight/2, piFilter );
    xCopyFromImageBuffer( psBufferV, iWidth/4, iHeight/4, iStrideV, imin, imax );

    iWidth  >>=1;
    iHeight >>=1;
  }
}




__inline
void
DownConvert::upsample( unsigned char* pucBufferY, int iStrideY,
                       unsigned char* pucBufferU, int iStrideU,
                       unsigned char* pucBufferV, int iStrideV,
                       int            iWidth,     int iHeight, int iStages )
{
  FILTER_UP

  for( int i = iStages; i > 0; i-- )
  {
    //===== luma =====
    xCopyToImageBuffer  ( pucBufferY, iWidth,   iHeight,   iStrideY );
    xUpsampling         (             iWidth,   iHeight,   piFilter );
    xCopyFromImageBuffer( pucBufferY, iWidth*2, iHeight*2, iStrideY, 0, 255 );
    //===== chroma cb =====
    xCopyToImageBuffer  ( pucBufferU, iWidth/2, iHeight/2, iStrideU );
    xUpsampling         (             iWidth/2, iHeight/2, piFilter );
    xCopyFromImageBuffer( pucBufferU, iWidth,   iHeight,   iStrideU, 0, 255 );
    //===== chroma cr =====
    xCopyToImageBuffer  ( pucBufferV, iWidth/2, iHeight/2, iStrideV );
    xUpsampling         (             iWidth/2, iHeight/2, piFilter );
    xCopyFromImageBuffer( pucBufferV, iWidth,   iHeight,   iStrideV, 0, 255 );

    iWidth  <<=1;
    iHeight <<=1;
  }
}


__inline
void
DownConvert::upsample( short* psBufferY, int iStrideY,
                       short* psBufferU, int iStrideU,
                       short* psBufferV, int iStrideV,
                       int    iWidth,    int iHeight, bool bClip, int iStages )
{
  FILTER_UP

  int   imin = ( bClip ?   0 : -32768 );
  int   imax = ( bClip ? 255 :  32767 );

  for( int i = iStages; i > 0; i-- )
  {
    //===== luma =====
    xCopyToImageBuffer  ( psBufferY, iWidth,   iHeight,   iStrideY );
    xUpsampling         (            iWidth,   iHeight,   piFilter );
    xCopyFromImageBuffer( psBufferY, iWidth*2, iHeight*2, iStrideY, imin, imax );
    //===== chroma cb =====
    xCopyToImageBuffer  ( psBufferU, iWidth/2, iHeight/2, iStrideU );
    xUpsampling         (            iWidth/2, iHeight/2, piFilter );
    xCopyFromImageBuffer( psBufferU, iWidth,   iHeight,   iStrideU, imin, imax );
    //===== chroma cr =====
    xCopyToImageBuffer  ( psBufferV, iWidth/2, iHeight/2, iStrideV );
    xUpsampling         (            iWidth/2, iHeight/2, piFilter );
    xCopyFromImageBuffer( psBufferV, iWidth,   iHeight,   iStrideV, imin, imax );

    iWidth  <<=1;
    iHeight <<=1;
  }
}



__inline
void
DownConvert::upsample( unsigned char* pucBufferY, int iStrideY,
                       unsigned char* pucBufferU, int iStrideU,
                       unsigned char* pucBufferV, int iStrideV,
                       int            iWidth,     int iHeight, int* piFilter, int iStages )
{
  for( int i = iStages; i > 0; i-- )
  {
    //===== luma =====
    xCopyToImageBuffer  ( pucBufferY, iWidth,   iHeight,   iStrideY );
    xUpsampling         (             iWidth,   iHeight,   piFilter );
    xCopyFromImageBuffer( pucBufferY, iWidth*2, iHeight*2, iStrideY, 0, 255 );
    //===== chroma cb =====
    xCopyToImageBuffer  ( pucBufferU, iWidth/2, iHeight/2, iStrideU );
    xUpsampling         (             iWidth/2, iHeight/2, piFilter );
    xCopyFromImageBuffer( pucBufferU, iWidth,   iHeight,   iStrideU, 0, 255 );
    //===== chroma cr =====
    xCopyToImageBuffer  ( pucBufferV, iWidth/2, iHeight/2, iStrideV );
    xUpsampling         (             iWidth/2, iHeight/2, piFilter );
    xCopyFromImageBuffer( pucBufferV, iWidth,   iHeight,   iStrideV, 0, 255 );

    iWidth  <<=1;
    iHeight <<=1;
  }
}


__inline
void
DownConvert::upsample( short* psBufferY, int iStrideY,
                       short* psBufferU, int iStrideU,
                       short* psBufferV, int iStrideV,
                       int    iWidth,    int iHeight, int* piFilter, bool bClip, int iStages )
{
  int   imin = ( bClip ?   0 : -32768 );
  int   imax = ( bClip ? 255 :  32767 );

  for( int i = iStages; i > 0; i-- )
  {
    //===== luma =====
    xCopyToImageBuffer  ( psBufferY, iWidth,   iHeight,   iStrideY );
    xUpsampling         (            iWidth,   iHeight,   piFilter );
    xCopyFromImageBuffer( psBufferY, iWidth*2, iHeight*2, iStrideY, imin, imax );
    //===== chroma cb =====
    xCopyToImageBuffer  ( psBufferU, iWidth/2, iHeight/2, iStrideU );
    xUpsampling         (            iWidth/2, iHeight/2, piFilter );
    xCopyFromImageBuffer( psBufferU, iWidth,   iHeight,   iStrideU, imin, imax );
    //===== chroma cr =====
    xCopyToImageBuffer  ( psBufferV, iWidth/2, iHeight/2, iStrideV );
    xUpsampling         (            iWidth/2, iHeight/2, piFilter );
    xCopyFromImageBuffer( psBufferV, iWidth,   iHeight,   iStrideV, imin, imax );

    iWidth  <<=1;
    iHeight <<=1;
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


#ifndef NO_MB_DATA_CTRL

__inline
void
DownConvert::xCopyFromImageBuffer2( short       * psDes,
                                   int            iWidth,
                                   int            iHeight,
                                   int            iStride,
                                   int            imin,
                                   int            imax )
{
  int* piSrc = m_paiImageBuffer2;

  for( int j = 0; j < iHeight; j++ )
  {
    for( int i = 0; i < iWidth;  i++ )
    {
      psDes[i] = (short)xClip( piSrc[i], imin, imax );
    }
    psDes   += iStride;
    piSrc   += m_iImageStride;
  }
}


__inline
void
DownConvert::upsampleResidual( short*         psBufferY,  int iStrideY,
                               short*         psBufferU,  int iStrideU,
                               short*         psBufferV,  int iStrideV,
                               int            iWidth,     int iHeight, 
                               h264::MbDataCtrl* pcMbDataCtrl, Bool bClip, int iStages )
{
  int piFilter[16] =   {  0,  0,  0,  0, 0,  0, 16,   32,   16,    0, 0,  0,  0,  0,  0,   64 };

  int   imin = ( bClip ?   0 : -32768 );
  int   imax = ( bClip ? 255 :  32767 );

  for( int i = iStages; i > 0; i-- )
  {
    //===== luma =====
    xCopyToImageBuffer  ( psBufferY, iWidth,   iHeight,   iStrideY );
    xUpsamplingFrame    (            iWidth,   iHeight,   m_paiImageBuffer, m_paiImageBuffer2, true, pcMbDataCtrl, piFilter );
    xCopyFromImageBuffer2( psBufferY, iWidth*2, iHeight*2, iStrideY, imin, imax );
    //===== chroma cb =====
    xCopyToImageBuffer  ( psBufferU, iWidth/2, iHeight/2, iStrideU );
    xUpsamplingFrame    (            iWidth/2, iHeight/2, m_paiImageBuffer, m_paiImageBuffer2, false, pcMbDataCtrl, piFilter );
    xCopyFromImageBuffer2( psBufferU, iWidth,   iHeight,   iStrideU, imin, imax );
    //===== chroma cr =====
    xCopyToImageBuffer  ( psBufferV, iWidth/2, iHeight/2, iStrideV );
    xUpsamplingFrame    (            iWidth/2, iHeight/2, m_paiImageBuffer, m_paiImageBuffer2, false, pcMbDataCtrl, piFilter );
    xCopyFromImageBuffer2( psBufferV, iWidth,   iHeight,   iStrideV, imin, imax );

    iWidth  <<=1;
    iHeight <<=1;
  }
}


__inline
void
DownConvert::xUpsamplingSubMb( int*           piSrc,  // low-resolution src-addr
                               int*           piDes,  // high-resolution src-addr
                               h264::BlkMode  eBlkMode,
                               int            aiFilter[])  // downsampling filter [15coeff+sum]
{
    switch( eBlkMode )
    {
    case h264::BLK_8x8:
      {
        xUpsamplingBlock( 8, 8, piSrc, piDes, aiFilter );
      }
      break;
    case h264::BLK_8x4:
      {
        xUpsamplingBlock( 8, 4, piSrc, piDes, aiFilter );
        piSrc += 4*m_iImageStride;
        piDes += 8*m_iImageStride;
        xUpsamplingBlock( 8, 4, piSrc, piDes, aiFilter );
      }
      break;
    case h264::BLK_4x8:
      {
        xUpsamplingBlock( 4, 8, piSrc, piDes, aiFilter );
        xUpsamplingBlock( 4, 8, piSrc + 4, piDes + 8, aiFilter );
      }
      break;
    case h264::BLK_SKIP:
    case h264::BLK_4x4:
      {
        xUpsamplingBlock( 4, 4, piSrc, piDes, aiFilter );
        xUpsamplingBlock( 4, 4, piSrc + 4, piDes + 8, aiFilter );
        piSrc += 4*m_iImageStride;
        piDes += 8*m_iImageStride;
        xUpsamplingBlock( 4, 4, piSrc, piDes, aiFilter );
        xUpsamplingBlock( 4, 4, piSrc + 4, piDes + 8, aiFilter );
      }
      break;
    default:
      AOT(1);
      break;
    }

}
__inline
void
DownConvert::xUpsamplingFrame( int                iWidth,       // low-resolution width
                               int                iHeight,      // low-resolution height
                               int*               piSrcBlock,   // low-resolution src-addr
                               int*               piDesBlock,   // high-resolution src-addr
                               bool               bLuma,
                               h264::MbDataCtrl*  pcMbDataCtrl,
                               int                aiFilter[] )  // downsampling filter [15coeff+sum]
{
  Int iPelPerMb = bLuma?16:8;
  Int iMbPerRow = iWidth/iPelPerMb;

  for( int j = 0; j < iHeight; j+= iPelPerMb )
  {
    for( int i = 0; i < iWidth;  i+= iPelPerMb )
    {
      int* piSrc = piSrcBlock + i+j*m_iImageStride;
      int* piDes = piDesBlock + 2*(i+j*m_iImageStride);

      const h264::MbData& rcMbData = pcMbDataCtrl->getMbData((i+j*iMbPerRow)/iPelPerMb);

      if( bLuma )
      {
        if( rcMbData.isIntra16x16() )
        {
          xUpsamplingBlock( iPelPerMb, iPelPerMb, piSrc, piDes, aiFilter );
        }
        else if( rcMbData.isTransformSize8x8() )
        {
          int iSize = iPelPerMb/2;
          xUpsamplingBlock( iSize, iSize, piSrc, piDes, aiFilter );
          piSrc += iPelPerMb/2;  
          piDes += iPelPerMb;
          xUpsamplingBlock( iSize, iSize, piSrc, piDes, aiFilter );
          piSrc += (iPelPerMb/2)*(m_iImageStride-1);  
          piDes += (iPelPerMb)  *(m_iImageStride-1);
          xUpsamplingBlock( iSize, iSize, piSrc, piDes, aiFilter );
          piSrc += iPelPerMb/2;  
          piDes += iPelPerMb;
          xUpsamplingBlock( iSize, iSize, piSrc, piDes, aiFilter );
        }
        else 
        {
          int iSize = iPelPerMb/4;
          for( int y = 0; y < 4; y++)
          {
            for( int x = 0; x < 4; x++)
            {
              Int iSrcOffset = iPelPerMb/4 *(x+y*m_iImageStride);
              xUpsamplingBlock( iSize, iSize, piSrc + iSrcOffset, piDes + 2*iSrcOffset, aiFilter );
            }
          }
        }

      }
      else
      {
        xUpsamplingBlock( 8, 8, piSrc, piDes, aiFilter );
      }
    }
  }
}


__inline
void
DownConvert::xUpsamplingBlock( int  iWidth,       // low-resolution width
                               int  iHeight,      // low-resolution height
                               int* piSrcBlock,   // low-resolution src-addr
                               int* piDesBlock,   // high-resolution src-addr
                               int  aiFilter[] )  // downsampling filter [15coeff+sum]
{
  int i, j, im3, im2, im1, i0, ip1, ip2, ip3, ip4;
  int div = ( aiFilter[15]*aiFilter[15] ) / 4;
  int add = div / 2;


  //========== vertical upsampling ===========
  for( j = 0; j < iWidth; j++ ) 
  {
    int*  piSrc = &piSrcBlock[j];
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

    int*  piDes = &piDesBlock[2*j];
    //----- copy back to image buffer -----
    for( i = 0; i < iHeight*2; i++ )
    {
      piDes[i*m_iImageStride] = m_paiTmp1dBuffer[i];
    }
  }


  //========== horizontal upsampling ==========
  for( j = 0; j < iHeight*2; j++ ) 
  {
    int*  piSrc = &piDesBlock[j*m_iImageStride];
    //----- upsample row -----
    for ( i = 0; i < iWidth; i++ )
    {
      im3     =2*( (i<       3) ? 0   : i     -3);
      im2     =2*( (i<       2) ? 0   : i     -2);
      im1     =2*( (i<       1) ? 0   : i     -1);
      i0      =2*( (i<iWidth  ) ? i   : iWidth-1);
      ip1     =2*( (i<iWidth-1) ? i+1 : iWidth-1);
      ip2     =2*( (i<iWidth-2) ? i+2 : iWidth-1);
      ip3     =2*( (i<iWidth-3) ? i+3 : iWidth-1);
      ip4     =2*( (i<iWidth-4) ? i+4 : iWidth-1);

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
    int*  piDes = &piDesBlock[j*m_iImageStride];
    //----- copy back to image buffer -----
    for( i = 0; i < iWidth*2; i++ )
    {
      piDes[i] = ( m_paiTmp1dBuffer[i] + add) / div;
    }
  }
}


#endif

__inline
void
DownConvert::xCopyToImageBuffer( short*   psSrc,
                                 int      iWidth,
                                 int      iHeight,
                                 int      iStride )
{
  int* piDes = m_paiImageBuffer;

  for( int j = 0; j < iHeight; j++ )
  {
    for( int i = 0; i < iWidth;  i++ )
    {
      piDes[i] = (int)psSrc[i];
    }
    piDes += m_iImageStride;
    psSrc += iStride;
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
DownConvert::xCopyFromImageBuffer( short*   psDes,
                                   int      iWidth,
                                   int      iHeight,
                                   int      iStride,
                                   int      imin,
                                   int      imax )
{
  int* piSrc = m_paiImageBuffer;

  for( int j = 0; j < iHeight; j++ )
  {
    for( int i = 0; i < iWidth;  i++ )
    {
      psDes[i] = (short)xClip( piSrc[i], imin, imax );
    }
    psDes += iStride;
    piSrc += m_iImageStride;
  }
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




