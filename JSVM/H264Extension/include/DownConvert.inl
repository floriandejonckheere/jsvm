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

__inline
DownConvert::DownConvert()
: m_iImageStride  ( 0 )
, m_paiImageBuffer		  ( NULL )
, m_paiTmp1dBuffer		  ( NULL )
#ifdef DOWN_CONVERT_STATIC //TMM_JV
, m_padFilter			  ( NULL )     
, m_aiTmp1dBufferInHalfpel( NULL )
, m_aiTmp1dBufferInQ1pel  ( NULL )
, m_aiTmp1dBufferInQ3pel  ( NULL )
, m_paiTmp1dBufferOut	  ( NULL )
#endif //DOWN_CONVERT_STATIC
{

}

__inline
DownConvert::~DownConvert()
{
 xDestroy();

#ifdef DOWN_CONVERT_STATIC //TMM_JV
  xDestroyFilterTmm(); 
#endif 
}

__inline
void 
DownConvert::xDestroy()
{
  if (m_paiImageBuffer) delete [] m_paiImageBuffer;
  if (m_paiTmp1dBuffer) delete [] m_paiTmp1dBuffer;
}

__inline
int
DownConvert::init( int iMaxWidth, int iMaxHeight )
{
  int iPicSize  =   iMaxWidth * iMaxHeight;
  int iMaxDim   = ( iMaxWidth > iMaxHeight ? iMaxWidth : iMaxHeight );

  xDestroy();

  m_iImageStride    = iMaxWidth;
  m_paiImageBuffer  = new int [ iPicSize ];
  m_paiTmp1dBuffer  = new int [ iMaxDim  ];

#ifdef DOWN_CONVERT_STATIC //TMM_JV
  xInitFilterTmm(iMaxDim); 
#endif // DOWN_CONVERT_STATIC 

  return ( m_paiImageBuffer == 0 || m_paiTmp1dBuffer == 0 );
}

__inline
int
DownConvert::xClip( int iValue, int imin, int imax )
{
  return ( iValue < imin ? imin : iValue > imax ? imax : iValue );
}

// =================================================================================
//   INTRA 3 
// =================================================================================
__inline
void
DownConvert::xUpsampling3( ResizeParameters* pcParameters,
                           bool bLuma
                          )
{
  int fact = (bLuma ? 1 : 2);
  int input_width   = pcParameters->m_iInWidth   /fact;
  int input_height  = pcParameters->m_iInHeight  /fact;
  int output_width  = pcParameters->m_iGlobWidth /fact;  
  int output_height = pcParameters->m_iGlobHeight/fact;
  int crop_x0 = pcParameters->m_iPosX /fact;
  int crop_y0 = pcParameters->m_iPosY /fact;
  int crop_w = pcParameters->m_iOutWidth /fact;
  int crop_h = pcParameters->m_iOutHeight/fact;  
  int input_chroma_phase_shift_x = 0;
  int input_chroma_phase_shift_y = 0;
  int output_chroma_phase_shift_x = 0;
  int output_chroma_phase_shift_y = 0;
  
  if ( !bLuma )
  {
    input_chroma_phase_shift_x = pcParameters->m_iBaseChromaPhaseX;
    input_chroma_phase_shift_y = pcParameters->m_iBaseChromaPhaseY;
    output_chroma_phase_shift_x = pcParameters->m_iChromaPhaseX;
    output_chroma_phase_shift_y = pcParameters->m_iChromaPhaseY;
  }

  //cixunzhang
  xUpsampling3(input_width, input_height,
	  output_width, output_height,
	  crop_x0, crop_y0, crop_w, crop_h,
	  input_chroma_phase_shift_x, input_chroma_phase_shift_y,
	  output_chroma_phase_shift_x, output_chroma_phase_shift_y, !bLuma );
  //end

}

//cixunzhang
__inline
void
DownConvert::xUpsampling3  ( int input_width, int input_height,
							   int output_width, int output_height,
							   int crop_x0, int crop_y0, int crop_w, int crop_h,
							   int input_chroma_phase_shift_x, int input_chroma_phase_shift_y,
							   int output_chroma_phase_shift_x, int output_chroma_phase_shift_y, bool uv_flag )

{
	int filter16[16][6] = { // Lanczos3
		{0,0,32,0,0,0},
		{0,-2,32,2,0,0},
		{1,-3,31,4,-1,0},
		{1,-4,30,6,-1,0},
		{1,-4,28,9,-2,0},
		{1,-4,27,11,-3,0},
		{1,-5,25,14,-3,0},
		{1,-5,22,17,-4,1},
		{1,-5,20,20,-5,1},
		{1,-4,17,22,-5,1},
		{0,-3,14,25,-5,1},
		{0,-3,11,27,-4,1},
		{0,-2,9,28,-4,1},
		{0,-1,6,30,-4,1},
		{0,-1,4,31,-3,1},
		{0,0,2,32,-2,0}
	};
	int filter16_chroma[16][6] = { // bilinear
		{0,0,32,0,0,0},
		{0,0,30,2,0,0},
		{0,0,28,4,0,0},
		{0,0,26,6,0,0},
		{0,0,24,8,0,0},
		{0,0,22,10,0,0},
		{0,0,20,12,0,0},
		{0,0,18,14,0,0},
		{0,0,16,16,0,0},
		{0,0,14,18,0,0},
		{0,0,12,20,0,0},
		{0,0,10,22,0,0},
		{0,0,8,24,0,0},
		{0,0,6,26,0,0},
		{0,0,4,28,0,0},
		{0,0,2,30,0,0}
	};

	if(uv_flag)
	{
		for (int i=0; i<16; i++)
			for (int j=0; j<6; j++)
				filter16[i][j] = filter16_chroma[i][j];
	}

	//end

//JVT-S067
  int i, j, k, *px, *py;
  int x16, y16, x, y, m;
  bool ratio1_flag = ( input_width == crop_w );

  // initialization
  px = new int[output_width];
  py = new int[output_height];

  //int F=4; 
  int G = 2, J, M, S = 12;
  unsigned short C, C1, D1;
  int D, E, q, w;

  for(i=0; i<crop_x0; i++)  px[i] = -128;
  
  for(i=crop_x0+crop_w; i<output_width; i++)  px[i] = -128;

  if(ratio1_flag)
  {
	  for(i = 0; i < crop_w; i++)
	  {
        px[i+crop_x0] = i*16+4*(2+output_chroma_phase_shift_x)-4*(2+input_chroma_phase_shift_x);
	  }
  }
  else
  {
    J = 1; M = 13; 
    if(uv_flag)
    {
	    J ++;
	    M --;
    };
   
    // S = M + G +  J  - F;

    C = ((1<<(M+G))*input_width + (crop_w>>1))/crop_w;
    //D = ((-1)<<(G-1+J+M)) + (1<<(S-1)) - (input_chroma_phase_shift_x<<(G-2+J+M));
    D = ((-1)<<15) + (1<<11) - (input_chroma_phase_shift_x<<14);

    C1 = C<<J;
    E = 0;

    q = (C<<(J-1)) + D + (C<<(J-2))*output_chroma_phase_shift_x;
    w = q>>S;
    D1 = q - (w<<S);
    E += w;
    px[0+crop_x0] = E;
  
    for(i = 1; i < crop_w; i++)
    {
	  q = C1 + D1;
	  w = q>>S;
	  D1 = q - (w<<S);
	  E += w;	  
	  px[i+crop_x0] = E;
    }
  }

  ratio1_flag = ( input_height == crop_h );

  for(j=0; j<crop_y0; j++)   py[j] = -128;
  
  for(j=crop_y0+crop_h; j<output_height; j++)  py[j] = -128;
  
  if(ratio1_flag)
  {
	  for(j = 0; j < crop_h; j++)
	  {
        py[j+crop_y0] = j*16+4*(2+output_chroma_phase_shift_y)-4*(2+input_chroma_phase_shift_y);
	  }
  }
  else
  {
    J = 1; M = 13; 
    if(uv_flag)
    {
	    J ++;
	    M --;
    };
   // S = M + G +  J  - F;

    C = ((1<<(M+G))*input_height + (crop_h>>1))/crop_h;
    //D = ((-1)<<(G-1+J+M)) + (1<<(S-1)) - (input_chroma_phase_shift_y<<(G-2+J+M));
    D = ((-1)<<15) + (1<<11) - (input_chroma_phase_shift_y<<14);

    C1 = C<<J;
    E = 0;

    q = (C<<(J-1)) + D + (C<<(J-2))*output_chroma_phase_shift_y;
    w = q>>S;
    D1 = q - (w<<S);
    E += w;
    py[0+crop_y0] = E;
  
    for(j = 1; j < crop_h; j++)
    {
      q = C1 + D1;
	  w = q>>S;
	  D1 = q - (w<<S);
	  E += w;	  
	  py[j+crop_y0] = E;
    }
  }

  //========== horizontal upsampling ===========
  for( j = 0; j < input_height; j++ ) 
  {
    int*  piSrc = &m_paiImageBuffer[j*m_iImageStride];
    for( i = 0; i < output_width; i++ ){
      if( px[i]==-128 ) continue;
      x16 = px[i]&0x0f;
      x = px[i]>>4;
      m_paiTmp1dBuffer[i] = 0;
      for( k=0; k<6; k++) {
        m = x - 2 + k;
        if( m<0 ) m = 0;
        else if( m>(input_width-1) ) m=input_width-1;
        m_paiTmp1dBuffer[i] += filter16[x16][k]*piSrc[m];
      }
      m_paiTmp1dBuffer[i] = m_paiTmp1dBuffer[i];
    }
    //----- copy row back to image buffer -----
    ::memcpy( piSrc, m_paiTmp1dBuffer, output_width*sizeof(int) );
  }
  //========== vertical upsampling ===========
  for( i = 0; i < output_width; i++ ) 
  {
    int*  piSrc = &m_paiImageBuffer[i];
    for( j = 0; j < output_height; j++ ){
      if( py[j]==-128 || px[i]==-128)
      {
        m_paiTmp1dBuffer[j] = 128;
        continue;
      }
      y16 = py[j]&0x0f;
      y = py[j]>>4;
      m_paiTmp1dBuffer[j] = 0;
      for( k=0; k<6; k++) {
        m = y - 2 + k;
        if( m<0 ) m = 0;
        else if( m>(input_height-1) ) m=input_height-1;
        m_paiTmp1dBuffer[j] += filter16[y16][k]*piSrc[m*m_iImageStride];
      }
      m_paiTmp1dBuffer[j] = (m_paiTmp1dBuffer[j]+512)/1024;
    }
    //----- scale and copy back to image buffer -----
    for( j = 0; j < output_height; j++ )
    {
      piSrc[j*m_iImageStride] = m_paiTmp1dBuffer[j];
    }
  }
  // free memory
   delete [] px;
   delete [] py;

}



#ifndef DOWN_CONVERT_STATIC //TMM_JV
//-------------------------------------------------
//JSVM upsampling methods (encoder + decoder) only
//-------------------------------------------------

// ===== Upsample Intra Short =======================================================
__inline
void
DownConvert::upsample ( short* psBufferY, int iStrideY,
                        short* psBufferU, int iStrideU,
                        short* psBufferV, int iStrideV,
                        ResizeParameters* pcParameters,
                        bool bClip )
{
  //===== Upsampling ======
  switch (pcParameters->m_iSpatialScalabilityType)
    {
    case SST_RATIO_1:
      break;
    default:
      xGenericUpsampleEss(psBufferY, iStrideY,
                          psBufferU, iStrideU,
                          psBufferV, iStrideV,
                          pcParameters, bClip
                         );
    }

  //===== Cropping =====
  if(pcParameters->m_iSpatialScalabilityType <= SST_RATIO_1 && pcParameters->m_bCrop)
  {
      xCrop(psBufferY, iStrideY,
            psBufferU, iStrideU,
            psBufferV, iStrideV,
            pcParameters, bClip
            );
  }
}

__inline
void
DownConvert::upsampleResidual ( short*         psBufferY,  int iStrideY,
                                short*         psBufferU,  int iStrideU,
                                short*         psBufferV,  int iStrideV,
                                ResizeParameters* pcParameters,
                                h264::MbDataCtrl*    pcMbDataCtrl,
                                bool           bClip )
{
  //===== Upsampling ======
  switch (pcParameters->m_iSpatialScalabilityType)
    {
    case SST_RATIO_1:
      break;
    default:
      xGenericUpsampleEss(psBufferY, iStrideY,
                          psBufferU, iStrideU,
                          psBufferV, iStrideV,
                          pcParameters,
                          pcMbDataCtrl); 
    }    

  //===== Cropping =====
  if(pcParameters->m_iSpatialScalabilityType <= SST_RATIO_1 && pcParameters->m_bCrop)
  {
      xCrop(psBufferY, iStrideY,
            psBufferU, iStrideU,
            psBufferV, iStrideV,
            pcParameters, bClip
            );
  }
}


__inline
void
DownConvert::xGenericUpsampleEss( short* psBufferY, int iStrideY,
                                  short* psBufferU, int iStrideU,
                                  short* psBufferV, int iStrideV,
                                  ResizeParameters* pcParameters,
                                  h264::MbDataCtrl*    pcMbDataCtrl)
{
  int iWidth=pcParameters->m_iInWidth;
  int iHeight=pcParameters->m_iInHeight;
  int width=pcParameters->m_iGlobWidth;
  int height=pcParameters->m_iGlobHeight;
  int x, y, w, h, j, i;
  short *buf1, *ptr1, *buf2, *ptr2, *tmp_buf1, *tmp_buf2;
  unsigned char *tmp_buf3;

  tmp_buf1=new short[iWidth*iHeight];
  tmp_buf2=new short[width*iHeight];
  tmp_buf3=new unsigned char[width*iHeight];
  
  memset(tmp_buf3, 4, width*iHeight);

  x=pcParameters->m_iPosX;
  y=pcParameters->m_iPosY;
  w=pcParameters->m_iOutWidth;
  h=pcParameters->m_iOutHeight;

  // luma
  buf1=buf2=psBufferY;
  ptr1=buf1;
  ptr2=tmp_buf1;
  for(j=0;j<iHeight;j++){
    for(i=0;i<iWidth;i++)ptr2[i]=ptr1[i];
    ptr2+=iWidth;
    ptr1+=iStrideY;
  }
  
  xFilterResidualHor(tmp_buf1, tmp_buf2, width,  x,  w,  iWidth, iHeight, pcMbDataCtrl, 0, 0, 0, tmp_buf3);
  for(j=0;j<height;j++){
    for(i=0;i<width;i++)buf1[i]=0;
    buf1+=iStrideY;
  }
  xFilterResidualVer(tmp_buf2, buf2, iStrideY,  x, y, w, h, width, iHeight, 0, 0, 0, tmp_buf3);

  // chroma
  width>>=1; height>>=1; x>>=1; y>>=1; w>>=1; h>>=1; iWidth>>=1; iHeight>>=1;
  // U
  buf1=buf2=psBufferU;
  ptr1=buf1;
  ptr2=tmp_buf1;
  for(j=0;j<iHeight;j++){
    for(i=0;i<iWidth;i++)ptr2[i]=ptr1[i];
    ptr2+=iWidth;
    ptr1+=iStrideU;
  }
  xFilterResidualHor(tmp_buf1, tmp_buf2, width,  x,  w, iWidth, iHeight, pcMbDataCtrl, 1, pcParameters->m_iChromaPhaseX, pcParameters->m_iBaseChromaPhaseX, tmp_buf3);
  for(j=0;j<height;j++){
    for(i=0;i<width;i++)buf1[i]=0;
    buf1+=iStrideU;
  }
  xFilterResidualVer(tmp_buf2, buf2, iStrideU,  x, y, w, h, width, iHeight, 1, pcParameters->m_iChromaPhaseY, pcParameters->m_iBaseChromaPhaseY, tmp_buf3);

  // V
  buf1=buf2=psBufferV;
  ptr1=buf1;
  ptr2=tmp_buf1;
  for(j=0;j<iHeight;j++){
    for(i=0;i<iWidth;i++)ptr2[i]=ptr1[i];
    ptr2+=iWidth;
    ptr1+=iStrideV;
  }
  xFilterResidualHor(tmp_buf1, tmp_buf2, width,  x,  w,  iWidth, iHeight, pcMbDataCtrl, 1, pcParameters->m_iChromaPhaseX, pcParameters->m_iBaseChromaPhaseX, tmp_buf3);
  for(j=0;j<height;j++){
    for(i=0;i<width;i++)buf1[i]=0;
    buf1+=iStrideU;
  }
  xFilterResidualVer(tmp_buf2, buf2, iStrideU,  x, y, w, h, width, iHeight,  1, pcParameters->m_iChromaPhaseY, pcParameters->m_iBaseChromaPhaseY, tmp_buf3);
  
  delete [] tmp_buf1;
  delete [] tmp_buf2;
  delete [] tmp_buf3;
} 

__inline
void
DownConvert::xGenericUpsampleEss( short* psBufferY, int iStrideY,
                                  short* psBufferU, int iStrideU,
                                  short* psBufferV, int iStrideV,
                                  ResizeParameters* pcParameters,
                                  bool bClip )
{
  int   min = ( bClip ?   0 : -32768 );
  int   max = ( bClip ? 255 :  32767 );

  int iInWidth = pcParameters->m_iInWidth;
  int iInHeight = pcParameters->m_iInHeight;
  int iGlobWidth = pcParameters->m_iGlobWidth;
  int iGlobHeight = pcParameters->m_iGlobHeight;
  
  //===== luma =====
  xCopyToImageBuffer  ( psBufferY, iInWidth,   iInHeight,   iStrideY );
  xUpsampling3        ( pcParameters, true);
  xCopyFromImageBuffer( psBufferY, iGlobWidth,   iGlobHeight,  iStrideY, min, max );


  // ===== parameters for chromas =====
  iInWidth    >>= 1;
  iInHeight   >>= 1;
  iGlobWidth   >>= 1;
  iGlobHeight  >>= 1;
  
  //===== chroma cb =====
  xCopyToImageBuffer  ( psBufferU, iInWidth, iInHeight, iStrideU );
  xUpsampling3        ( pcParameters, false);
  xCopyFromImageBuffer( psBufferU, iGlobWidth, iGlobHeight, iStrideU, min, max );

  //===== chroma cr =====
  xCopyToImageBuffer  ( psBufferV, iInWidth, iInHeight, iStrideV );
  xUpsampling3        ( pcParameters, false);
  xCopyFromImageBuffer( psBufferV, iGlobWidth, iGlobHeight, iStrideV, min, max ); 
} 

//JVT-S067
__inline
void
DownConvert::xFilterResidualHor ( short *buf_in, short *buf_out, 
                                  int width, 
                                  int x, int w, 
                                  int wsize_in, int hsize_in, 
                                  h264::MbDataCtrl*  pcMbDataCtrl, 
                                  bool chroma, int output_chroma_phase_shift_x, int input_chroma_phase_shift_x,
                                  unsigned char *buf_blocksize )
{
  int j, i, k, i1;
  short *ptr1, *ptr2;
  unsigned char *ptr3;
  int p, p2, p3, block = ( chroma ? 4 : 8 );
  int iMbPerRow = wsize_in >> 4;

  int *x16 = new int[w]; 
  int* k16 = new int[w]; // for relative phase shift in unit of 1/16 sample
  int* p16 = new int[w];

  if(w == wsize_in)
  {
	  for(i = 0; i < w; i++)
	  {
		  k = i*16+4*(2+output_chroma_phase_shift_x)-4*(2+input_chroma_phase_shift_x);
		  if(k<0)
		    k = 0;
		  i1 = k >> 4;
	      k -= i1 * 16;
	      p = ( k > 7 && (i1 + 1) < wsize_in ) ? ( i1 + 1 ) : i1;
	      p = p < 0 ? 0 : p;
		  x16[i] = i1; k16[i] = k; p16[i] = p;
	  }
  }
  else
  {
    //int F = 4; 
    int G = 2, J = 1, M = 13, S = 12;
    if(chroma)
    {
  	  J ++;
  	  M --;
    };
    //S = M + G +  J  - F;
    unsigned short C, C1, D1;
    int D, E, q, w1;

    C = ((1<<(M+G))*wsize_in + (w>>1))/w;
    //D = ((-1)<<(G-1+J+M)) + (1<<(S-1)) - (input_chroma_phase_shift_x<<(G-2+J+M));
    D = ((-1)<<15) + (1<<11) - (input_chroma_phase_shift_x<<14);

    C1 = C<<J;
    E = 0;

    q = (C<<(J-1)) + D + (C<<(J-2))*output_chroma_phase_shift_x;
    w1 = q>>S;
    D1 = q - (w1<<S);
    E += w1;
    k = E;
    if(k<0)
	    k = 0;
    i1 = k >> 4;
    k -= i1 * 16;
    p = ( k > 7 && (i1 + 1) < wsize_in ) ? ( i1 + 1 ) : i1;
    p = p < 0 ? 0 : p;
    x16[0] = i1; k16[0] = k; p16[0] = p;
  
    for(i = 1; i < w; i++)
    {
	  q = C1 + D1;
	  w1 = q>>S;
	  D1 = q - (w1<<S);
	  E += w1;
	  k = E;
	  if(k<0)
		k = 0;
	  i1 = k >> 4;
	  k -= i1 * 16;
	  p = ( k > 7 && (i1 + 1) < wsize_in ) ? ( i1 + 1 ) : i1;
	  p = p < 0 ? 0 : p;
	  x16[i] = i1; k16[i] = k; p16[i] = p;
    }
  }

  for( j = 0; j < hsize_in; j++ )
  {
    ptr1 = buf_in + j * wsize_in;
    ptr2 = buf_out + j * width + x;
    ptr3 = buf_blocksize + j * width + x;
    for( i = 0; i < w; i++ )
    {
      i1 = x16[i]; k = k16[i]; p = p16[i];
#if !RESIDUAL_B8_BASED
      if( !chroma )
      {
        const h264::MbData& rcMbData = pcMbDataCtrl->getMbData( (p>>4) + (j>>4) * iMbPerRow );
        //if( rcMbData.isIntra16x16() ) block = 16;
        //else 
        if( rcMbData.isTransformSize8x8() ) block = 8;
        else block = 4;
        ptr3[i] = (unsigned char) block;
      }
#endif
      p = p / block;
      p2 = ( i1 / block ) == p ? i1 : ( p * block );
      p3 = ( (i1+1) / block ) == p ? (i1+1) : ( p*block + (block-1) );
      ptr2[i] = (short) ( (16-k) * ptr1[p2] + k * ptr1[p3]);
    }
  }
 
  delete [] x16;
  delete [] k16;
  delete [] p16;

}

//JVT-S067
__inline
void
DownConvert::xFilterResidualVer ( short *buf_in, short *buf_out, 
                                  int width, 
                                  int x, int y, int w, int h, 
                                  int wsize_in, int hsize_in, 
                                  bool chroma, int output_chroma_phase_shift_y, int input_chroma_phase_shift_y,
                                  unsigned char *buf_blocksize )
{
  int j, i, k, j1;
  short *ptr1, *ptr2;
  unsigned char *ptr3;
  int p, p2, p3, block = ( chroma ? 4 : 8 );

  int* y16 = new int[h]; 
  int* k16 = new int[h]; // for relative phase shift in unit of 1/16 sample
  int* p16 = new int[h];

  if(h == hsize_in)
  {
	  for(j = 0; j < h; j++)
	  {
		  k = j*16+4*(2+output_chroma_phase_shift_y)-4*(2+input_chroma_phase_shift_y);
		  if(k<0)
		    k = 0;
	      j1 = k >> 4;
	      k -= j1 * 16;
	      p = ( k > 7 && ( j1+1 ) < hsize_in ) ? ( j1+1 ) : j1;
    	  p = p < 0 ? 0 : p;
	      y16[j] = j1; k16[j] = k; p16[j] = p;
	  }
  }
  else
  {
    //int F = 4;
    int G = 2, J = 1, M = 13, S = 12;
    if(chroma)
    {
	  J ++;
	  M --;
    };
    //S = M + G + J - F;
    unsigned short C, C1, D1;
    int D, E, q, w1;

    C = ((1<<(M+G))*hsize_in + (h>>1))/h;
    //D = ((-1)<<(G-1+J+M)) + (1<<(S-1)) - (input_chroma_phase_shift_y<<(G-2+J+M));
    D = ((-1)<<15) + (1<<11) - (input_chroma_phase_shift_y<<14);

    C1 = C<<J;
    E = 0;

    q = (C<<(J-1)) + D + (C<<(J-2))*output_chroma_phase_shift_y;
    w1 = q>>S;
    D1 = q - (w1<<S);
    E += w1;
    k = E;
    if(k<0)
	  k = 0;
    j1 = k >> 4;
    k -= j1 * 16;
    p = ( k > 7 && ( j1+1 ) < hsize_in ) ? ( j1+1 ) : j1;
    p = p < 0 ? 0 : p;
    y16[0] = j1; k16[0] = k; p16[0] = p;
  
    for(j = 1; j < h; j++)
    {
	  q = C1 + D1;
	  w1 = q>>S;
	  D1 = q - (w1<<S);
	  E += w1;
	  k = E;
	  if(k<0)
		k = 0;
	  j1 = k >> 4;
	  k -= j1 * 16;
	  p = ( k > 7 && ( j1+1 ) < hsize_in ) ? ( j1+1 ) : j1;
	  p = p < 0 ? 0 : p;
	  y16[j] = j1; k16[j] = k; p16[j] = p;
    }
  }

  for( i = 0; i < w; i++ )
  {
    ptr1 = buf_in + i + x;
    ptr3 = buf_blocksize + i + x;
    ptr2 = buf_out + i + x + width * y;
    for( j = 0; j < h; j++ )
    {
      j1 = y16[j]; k = k16[j]; p = p16[j];
#if !RESIDUAL_B8_BASED
      if( !chroma ){
        block = ptr3[ wsize_in * p ];
      }
#endif
      p = p / block;
      p2 = ( j1/block ) == p ? j1 : ( p*block );
      p3 = ( (j1+1) / block ) == p ? (j1+1) : ( p*block + (block-1) );
      ptr2[j*width] = (Short) (( (16-k) * ptr1[wsize_in*p2] + k * ptr1[wsize_in*p3] + 128 ) >> 8);
    }
  }
  delete [] y16;
  delete [] k16;
  delete [] p16;
}

__inline
void
DownConvert::xCrop ( short* psBufferY, int iStrideY,
                     short* psBufferU, int iStrideU,
                     short* psBufferV, int iStrideV,
                     ResizeParameters* pcParameters,
                     bool bClip )
{
  int iOutWidth = pcParameters->m_iOutWidth;
  int iOutHeight = pcParameters->m_iOutHeight;
  int iPosX = pcParameters->m_iPosX;
  int iPosY = pcParameters->m_iPosY;
  int iGlobWidth = pcParameters->m_iGlobWidth;
  int iGlobHeight = pcParameters->m_iGlobHeight;

  int   min = ( bClip ?   0 : -32768 );
  int   max = ( bClip ? 255 :  32767 );
  short* ptr;

  //===== luma =====
  ptr = &psBufferY[iPosY * iStrideY + iPosX];
  xCopyToImageBuffer  ( psBufferY, iOutWidth,  iOutHeight,   iStrideY );
  xSetValue(psBufferY, iStrideY, iGlobWidth, iGlobHeight, (short)DEFAULTY);
  xCopyFromImageBuffer( ptr,                                iOutWidth,   iOutHeight,  iStrideY, min, max );

  // ===== parameters for chromas =====
  iOutWidth   >>= 1;
  iOutHeight  >>= 1;
  iPosX       >>= 1;
  iPosY       >>= 1;
  iGlobWidth  >>= 1;
  iGlobHeight >>= 1;
  
  //===== chroma cb =====
  ptr = &psBufferU[iPosY * iStrideU + iPosX];
  xCopyToImageBuffer  ( psBufferU, iOutWidth,  iOutHeight,   iStrideU );
  xSetValue(psBufferU, iStrideU, iGlobWidth, iGlobHeight, (short)DEFAULTU);
  xCopyFromImageBuffer( ptr,                                 iOutWidth, iOutHeight, iStrideU, min, max );

  //===== chroma cr =====
  ptr = &psBufferV[iPosY * iStrideV + iPosX];
  xCopyToImageBuffer  ( psBufferV, iOutWidth,  iOutHeight,   iStrideV );
  xSetValue(psBufferV, iStrideV, iGlobWidth, iGlobHeight, (short)DEFAULTV);
  xCopyFromImageBuffer( ptr,                                 iOutWidth, iOutHeight, iStrideV, min, max );
}

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
DownConvert::xSetValue ( short* psBuffer, int iStride, int iWidth, int iHeight, short value )
{
  for (int y=0; y<iHeight; y++)
    for (int x=0; x<iWidth; x++)
      psBuffer[y*iStride + x] = value;
}

#endif // DOWN_CONVERT_STATIC 



