

// =================================================================================
//   GENERAL
// =================================================================================
#ifndef max
#define max(x, y) ((x)>(y)?(x):(y))
#define min(x, y) ((x)<(y)?(x):(y))
#endif

#define DEFAULTY 0
#define DEFAULTU 0
#define DEFAULTV 0

#define ESS_ARRAY_SIZE   64   //TMM_ESS_UNIFIED

#define DIRECT_INTERP 0  // 1: direct interpolation on; 0: off  - SSUN@SHARP

__inline
void
DownConvert::xInitFilterTmm (int iMaxDim )
{  
  m_paiTmp1dBufferIn = m_paiTmp1dBuffer;
  m_paiTmp1dBufferOut = new int [ iMaxDim  ];

  xInitFilterTmm1(iMaxDim);
  xInitFilterTmm2(iMaxDim);
#ifndef NO_MB_DATA_CTRL
  xInitFilterTmmResidual(iMaxDim);
#endif
}

__inline
void
DownConvert::xDestroyFilterTmm ( )
{
  // TODO
  return;
  
  delete [] m_paiTmp1dBufferOut;
  
  xDestroyFilterTmm1();
  xDestroyFilterTmm2();
#ifndef NO_MB_DATA_CTRL
  xDestroyFilterTmmResidual();
#endif
}

// ===== Upsample Intra ============================================================
__inline
void
DownConvert::upsample_tmm ( unsigned char* pucBufferY, int iStrideY,
                            unsigned char* pucBufferU, int iStrideU,
                            unsigned char* pucBufferV, int iStrideV,
                            ResizeParameters* pcParameters )
{
  //===== Upsampling ======
    switch (pcParameters->m_iSpatialScalabilityType)
    {
    case SST_RATIO_1:
      break;
    case SST_RATIO_2:
      upsample(pucBufferY, iStrideY,
               pucBufferU, iStrideU,
               pucBufferV, iStrideV,
               pcParameters->m_iInWidth, pcParameters->m_iInHeight
               );
      break;
    default:
      xGenericUpsample(pucBufferY, iStrideY,
                       pucBufferU, iStrideU,
                       pucBufferV, iStrideV,
                       pcParameters
                       );
    }

  //===== Cropping =====
  if (pcParameters->m_bCrop)
    xCrop(pucBufferY, iStrideY,
          pucBufferU, iStrideU,
          pucBufferV, iStrideV,
          pcParameters
          );
}

__inline
void
DownConvert::xGenericUpsample ( unsigned char* pucBufferY, int iStrideY,
                                unsigned char* pucBufferU, int iStrideU,
                                unsigned char* pucBufferV, int iStrideV,
                                ResizeParameters* pcParameters )
{
  int iInWidth = pcParameters->m_iInWidth;
  int iInHeight = pcParameters->m_iInHeight;
  int iOutWidth = pcParameters->m_iOutWidth;
  int iOutHeight = pcParameters->m_iOutHeight;
  
  //===== luma =====
  xCopyToImageBuffer  ( pucBufferY, iInWidth,   iInHeight,   iStrideY );
  xUpsampling         ( pcParameters, true );
  xCopyFromImageBuffer( pucBufferY,   iOutWidth,   iOutHeight,  iStrideY, 0, 255 );


  // ===== parameters for chromas =====
  iInWidth    >>= 1;
  iInHeight   >>= 1;
  iOutWidth   >>= 1;
  iOutHeight  >>= 1;
  
  //===== chroma cb =====
  xCopyToImageBuffer  ( pucBufferU, iInWidth, iInHeight, iStrideU );
  xUpsampling         ( pcParameters, false ); 
  xCopyFromImageBuffer( pucBufferU, iOutWidth, iOutHeight, iStrideU, 0, 255 );

  //===== chroma cr =====
  xCopyToImageBuffer  ( pucBufferV, iInWidth, iInHeight, iStrideV );
  xUpsampling         ( pcParameters, false );
  xCopyFromImageBuffer( pucBufferV, iOutWidth, iOutHeight, iStrideV, 0, 255 ); 
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
  xCopyToImageBuffer  ( pucBufferY, iOutWidth,  iOutHeight, iStrideY );
  xSetValue(pucBufferY, iStrideY, iGlobWidth, iGlobHeight, (unsigned char)DEFAULTY);
  xCopyFromImageBuffer( ptr,        iOutWidth,  iOutHeight, iStrideY, 0, 255 );

  // ===== parameters for chromas =====
  iOutWidth   >>= 1;
  iOutHeight  >>= 1;
  iPosX       >>= 1;
  iPosY       >>= 1;
  iGlobWidth  >>= 1;
  iGlobHeight >>= 1;
  
  //===== chroma cb =====
  ptr = &pucBufferU[iPosY * iStrideU + iPosX];
  xCopyToImageBuffer  ( pucBufferU, iOutWidth,  iOutHeight, iStrideU );
  xSetValue(pucBufferU, iStrideU, iGlobWidth, iGlobHeight, (unsigned char)DEFAULTU);
  xCopyFromImageBuffer( ptr,        iOutWidth,  iOutHeight, iStrideU, 0, 255 );

  //===== chroma cr =====
  ptr = &pucBufferV[iPosY * iStrideV + iPosX];
  xCopyToImageBuffer  ( pucBufferV, iOutWidth,  iOutHeight, iStrideV );
  xSetValue(pucBufferV, iStrideV, iGlobWidth, iGlobHeight, (unsigned char)DEFAULTV);
  xCopyFromImageBuffer( ptr,        iOutWidth,  iOutHeight, iStrideV, 0, 255 );
}



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
    case SST_RATIO_2:
      upsample(psBufferY, iStrideY,
               psBufferU, iStrideU,
               psBufferV, iStrideV,
               pcParameters->m_iInWidth, pcParameters->m_iInHeight,
               bClip,
               1
               );
      break;
    default:
#if DIRECT_INTERP // SSUN@SHARP
      xGenericUpsampleEss(psBufferY, iStrideY,
                          psBufferU, iStrideU,
                          psBufferV, iStrideV,
                          pcParameters, bClip
                         );
#else // end of SSUN@SHARP
      xGenericUpsample(psBufferY, iStrideY,
                       psBufferU, iStrideU,
                       psBufferV, iStrideV,
                       pcParameters, bClip
                       );
#endif
    }

  //===== Cropping =====
#if DIRECT_INTERP  // SSUN@SHARP
  if(pcParameters->m_iSpatialScalabilityType <= SST_RATIO_2)
#endif // end of SSUN@SHARP
    if (pcParameters->m_bCrop)
      xCrop(psBufferY, iStrideY,
            psBufferU, iStrideU,
            psBufferV, iStrideV,
            pcParameters, bClip
            );
}


//SSUN@SHARP
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
// end of SSUN@SHARP


__inline
void
DownConvert::xGenericUpsample ( short* psBufferY, int iStrideY,
                                short* psBufferU, int iStrideU,
                                short* psBufferV, int iStrideV,
                                ResizeParameters* pcParameters,
                                bool bClip )
{
  int   min = ( bClip ?   0 : -32768 );
  int   max = ( bClip ? 255 :  32767 );

  int iInWidth = pcParameters->m_iInWidth;
  int iInHeight = pcParameters->m_iInHeight;
  int iOutWidth = pcParameters->m_iOutWidth;
  int iOutHeight = pcParameters->m_iOutHeight;
  
  //===== luma =====
  xCopyToImageBuffer  ( psBufferY, iInWidth,   iInHeight,   iStrideY );
  xUpsampling         ( pcParameters, true);
  xCopyFromImageBuffer( psBufferY, iOutWidth,   iOutHeight,  iStrideY, min, max );


  // ===== parameters for chromas =====
  iInWidth    >>= 1;
  iInHeight   >>= 1;
  iOutWidth   >>= 1;
  iOutHeight  >>= 1;
  
  //===== chroma cb =====
  xCopyToImageBuffer  ( psBufferU, iInWidth, iInHeight, iStrideU );
  xUpsampling         ( pcParameters, false);
  xCopyFromImageBuffer( psBufferU, iOutWidth, iOutHeight, iStrideU, min, max );

  //===== chroma cr =====
  xCopyToImageBuffer  ( psBufferV, iInWidth, iInHeight, iStrideV );
  xUpsampling         ( pcParameters, false);
  xCopyFromImageBuffer( psBufferV, iOutWidth, iOutHeight, iStrideV, min, max ); 
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


// ===== Upsample Inter ============================================================
#ifndef NO_MB_DATA_CTRL
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
    case SST_RATIO_2:
      upsampleResidual(psBufferY, iStrideY,
                       psBufferU, iStrideU,
                       psBufferV, iStrideV,
                       pcParameters->m_iInWidth, pcParameters->m_iInHeight,
                       pcMbDataCtrl,
                       bClip
                       );
      break;
    default:
#if DIRECT_INTERP // SSUN@SHARP
      xGenericUpsampleEss(psBufferY, iStrideY,
                          psBufferU, iStrideU,
                          psBufferV, iStrideV,
                          pcParameters,
                          pcMbDataCtrl,
                          bClip);
#else // end of SSUN@SHARP
      xGenericUpsample(psBufferY, iStrideY,
                       psBufferU, iStrideU,
                       psBufferV, iStrideV,
                       pcParameters,
                       pcMbDataCtrl,
                       bClip
                       );
#endif
    }    

  //===== Cropping =====
#if DIRECT_INTERP  // SSUN@SHARP
  if(pcParameters->m_iSpatialScalabilityType <= SST_RATIO_2)
#endif // end of SSUN@SHARP
    if (pcParameters->m_bCrop)
      xCrop(psBufferY, iStrideY,
            psBufferU, iStrideU,
            psBufferV, iStrideV,
            pcParameters, bClip
            );
}



// SSUN@SHARP
__inline
void
DownConvert::xFilterResidualHor ( short *buf_in, short *buf_out, 
                                  int width, int height, 
                                  int x, int y, int w, int h, 
                                  int wsize_in, int hsize_in, 
                                  h264::MbDataCtrl*  pcMbDataCtrl, 
                                  bool chroma, 
                                  unsigned char *buf_blocksize )
{
  int j, i, k, i1, ii;
  short *ptr1, *ptr2;
  unsigned char *ptr3;
  int p, p2, p3, block = 8;
  int iMbPerRow = wsize_in >> 4;
  int new_div[2];  // for the simplified division operation
  int *x16, *k16, *p16;

  x16 = (int *)malloc( sizeof(int)*w ); 
  k16 = (int *)malloc( sizeof(int)*w ); // for relative phase shift in unit of 1/16 sample
  p16 = (int *)malloc( sizeof(int)*w );

  // initialize the simplified division operator
  new_div[0] = 0;
  while( (1<<( new_div[0] + 1 )) < w ) new_div[0] += 1;
  new_div[0] += 15;
  new_div[1] = ( ( 1<<new_div[0] ) + w/2 ) / w;

  for( i = 0; i < w; i++ ) 
  {
    ii = i * wsize_in;
    i1 = ( ( ( ( ii & 0xffff ) * new_div[1] ) >> 16 ) + ( ii>>16 ) * new_div[1] ) >> ( new_div[0] - 16 );
    k = ( ( ( ( ( ii & 0xffff ) * new_div[1] ) >> 16 ) + ( ii>>16 ) * new_div[1] ) >> ( new_div[0] - 20 ) ) - i1 * 16;
    p = ( k > 7 && (i1 + 1) < wsize_in ) ? ( i1 + 1 ) : i1;
    p = p < 0 ? 0 : p;
    x16[i] = i1; k16[i] = k; p16[i] = p;
  }

  for( j = 0; j < hsize_in; j++ )
  {
    ptr1 = buf_in + j * wsize_in;
    ptr2 = buf_out + j * width + x;
    ptr3 = buf_blocksize + j * width + x;
    for( i = 0; i < w; i++ )
    {
      i1 = x16[i]; k = k16[i]; p = p16[i];
      if( !chroma )
      {
        const h264::MbData& rcMbData = pcMbDataCtrl->getMbData( (p>>4) + (j>>4) * iMbPerRow );
        if( rcMbData.isIntra16x16() ) block = 16;
        else if( rcMbData.isTransformSize8x8() ) block = 8;
        else block = 4;
        ptr3[i] = block;
      }
      p = p / block;
      p2 = ( i1 / block ) == p ? i1 : ( p * block );
      p3 = ( (i1+1) / block ) == p ? (i1+1) : ( p*block + (block-1) );
      ptr2[i] = (16-k) * ptr1[p2] + k * ptr1[p3];
    }
  }
  free( x16 );
  free( k16 );
  free( p16 );
}

__inline
void
DownConvert::xFilterResidualVer ( short *buf_in, short *buf_out, 
                                  int width, int height, 
                                  int x, int y, int w, int h, 
                                  int wsize_in, int hsize_in, 
                                  h264::MbDataCtrl*  pcMbDataCtrl, 
                                  bool chroma, 
                                  unsigned char *buf_blocksize )
{
  int j, i, k, j1, jj;
  short *ptr1, *ptr2;
  unsigned char *ptr3;
  int p, p2, p3, block = 8;
  int new_div[2];  // for the simplified division operation
  int *y16, *k16, *p16;

  y16 = (int *)malloc( sizeof(int) * h );
  k16 = (int *)malloc( sizeof(int) * h ); // for relative phase shift in unit of 1/16 sample
  p16 = (int *)malloc( sizeof(int) * h );

  // initialize the simplified division operator
  new_div[0] = 0;
  while( (1<<( new_div[0] + 1 )) < h ) new_div[0] += 1;
  new_div[0] += 15;
  new_div[1] =( ( 1 << new_div[0] ) + h/2 ) / h;

  for( j = 0; j < h; j++ )
  {
    jj = j * hsize_in;
    j1 = ( ( ( ( jj & 0xffff ) * new_div[1] ) >> 16 ) + ( jj >> 16 ) * new_div[1] ) >> ( new_div[0] - 16 );
    k = ( ( ( ( ( jj & 0xffff ) * new_div[1] ) >> 16 ) + ( jj >> 16 ) * new_div[1] ) >> ( new_div[0] - 20 ) ) - j1 * 16;
    p = ( k > 7 && ( j1+1 ) < hsize_in ) ? ( j1+1 ) : j1;
    p = p < 0 ? 0 : p;
    y16[j] = j1; k16[j] = k; p16[j] = p;
  }

  for( i = 0; i < w; i++ )
  {
    ptr1 = buf_in + i + x;
    ptr3 = buf_blocksize + i + x;
    ptr2 = buf_out + i + x + width * y;
    for( j = 0; j < h; j++ )
    {
      j1 = y16[j]; k = k16[j]; p = p16[j];
      if( !chroma ){
        block = ptr3[ wsize_in * p ];
      }
      p = p / block;
      p2 = ( j1/block ) == p ? j1 : ( p*block );
      p3 = ( (j1+1) / block ) == p ? (j1+1) : ( p*block + (block-1) );
      ptr2[j*width] = ( (16-k) * ptr1[wsize_in*p2] + k * ptr1[wsize_in*p3] + 128 ) >> 8;
    }
  }
  free(y16);
  free(k16);
  free(p16);
}

__inline
void
DownConvert::xGenericUpsampleEss( short* psBufferY, int iStrideY,
                                  short* psBufferU, int iStrideU,
                                  short* psBufferV, int iStrideV,
                                  ResizeParameters* pcParameters,
                                  h264::MbDataCtrl*    pcMbDataCtrl,
                                  bool bClip )
{
  int iWidth=pcParameters->m_iInWidth;
  int iHeight=pcParameters->m_iInHeight;
  int width=pcParameters->m_iGlobWidth;
  int height=pcParameters->m_iGlobHeight;
  int x, y, w, h, j, i;
  short *buf1, *ptr1, *buf2, *ptr2, *tmp_buf1, *tmp_buf2;
  unsigned char *tmp_buf3;

  tmp_buf1=(short *)malloc(iWidth*iHeight*sizeof(short));
  tmp_buf2=(short *)malloc(width*iHeight*sizeof(short));
  tmp_buf3=(unsigned char *)malloc(width*iHeight);
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
  xFilterResidualHor(tmp_buf1, tmp_buf2, width, height, x, y, w, h, iWidth, iHeight, pcMbDataCtrl, 0, tmp_buf3);
  for(j=0;j<height;j++){
    for(i=0;i<width;i++)buf1[i]=0;
    buf1+=iStrideY;
  }
  xFilterResidualVer(tmp_buf2, buf2, iStrideY, height, x, y, w, h, width, iHeight, pcMbDataCtrl, 0, tmp_buf3);

  // chroma
  width>>=1; height>>=1; x>>=1; y>>=1; w>>=1; h>>=1; iWidth>>=1; iHeight>>=1;
  // U
  buf1=buf2=psBufferU;
  ptr1=buf1;
  ptr2=tmp_buf1;
  for(j=0;j<iHeight;j++){
    for(i=0;i<iWidth;i++)ptr2[i]=ptr1[i];
    ptr2+=iWidth+2;
    ptr1+=iStrideU;
  }
  xFilterResidualHor(tmp_buf1, tmp_buf2, width, height, x, y, w, h, iWidth, iHeight, pcMbDataCtrl, 1, tmp_buf3);
  for(j=0;j<height;j++){
    for(i=0;i<width;i++)buf1[i]=0;
    buf1+=iStrideU;
  }
  xFilterResidualVer(tmp_buf2, buf2, iStrideU, height, x, y, w, h, width, iHeight, pcMbDataCtrl, 1, tmp_buf3);

  // V
  buf1=buf2=psBufferV;
  ptr1=buf1;
  ptr2=tmp_buf1;
  for(j=0;j<iHeight;j++){
    for(i=0;i<iWidth;i++)ptr2[i]=ptr1[i];
    ptr2+=iWidth+2;
    ptr1+=iStrideV;
  }
  xFilterResidualHor(tmp_buf1, tmp_buf2, width, height, x, y, w, h, iWidth, iHeight, pcMbDataCtrl, 1, tmp_buf3);
  for(j=0;j<height;j++){
    for(i=0;i<width;i++)buf1[i]=0;
    buf1+=iStrideU;
  }
  xFilterResidualVer(tmp_buf2, buf2, iStrideU, height, x, y, w, h, width, iHeight, pcMbDataCtrl, 1, tmp_buf3);
  
  free(tmp_buf1);
  free(tmp_buf2);
  free(tmp_buf3);
} 
// end of SSUN@SHARP


__inline
void
DownConvert::xGenericUpsample ( short* psBufferY, int iStrideY,
                                short* psBufferU, int iStrideU,
                                short* psBufferV, int iStrideV,
                                ResizeParameters* pcParameters,
                                h264::MbDataCtrl*    pcMbDataCtrl,
                                bool bClip )
{
  int   min = ( bClip ?   0 : -32768 );
  int   max = ( bClip ? 255 :  32767 );
  
  int iInWidth = pcParameters->m_iInWidth;
  int iInHeight = pcParameters->m_iInHeight;
  int iOutWidth = pcParameters->m_iOutWidth;
  int iOutHeight = pcParameters->m_iOutHeight;
  
  //===== luma =====
  xCopyToImageBuffer  ( psBufferY, iInWidth,   iInHeight,   iStrideY );
  xUpsamplingFrame    ( pcParameters, m_paiImageBuffer, m_paiImageBuffer2, true, pcMbDataCtrl );
  xCopyFromImageBuffer2( psBufferY,                        iOutWidth,   iOutHeight,  iStrideY, min, max );


  // ===== parameters for chromas =====
  iInWidth    >>= 1;
  iInHeight   >>= 1;
  iOutWidth   >>= 1;
  iOutHeight  >>= 1;
  
  //===== chroma cb =====
  xCopyToImageBuffer  ( psBufferU, iInWidth, iInHeight, iStrideU );
  xUpsamplingFrame    ( pcParameters, m_paiImageBuffer, m_paiImageBuffer2, false, pcMbDataCtrl );
  xCopyFromImageBuffer2( psBufferU,                     iOutWidth, iOutHeight, iStrideU, min, max );

  //===== chroma cr =====
  xCopyToImageBuffer  ( psBufferV, iInWidth, iInHeight, iStrideV );
  xUpsamplingFrame    ( pcParameters, m_paiImageBuffer, m_paiImageBuffer2, false, pcMbDataCtrl );
  xCopyFromImageBuffer2( psBufferV,                     iOutWidth, iOutHeight, iStrideV, min, max );
}

__inline
void
DownConvert::xInitFilterTmmResidual ( int iMaxDim )
{
//TMM_ESS_UNIFIED {
 	m_pitmpDes4 = new int [ESS_ARRAY_SIZE*ESS_ARRAY_SIZE];
	m_pitmpDes8 = new int [ESS_ARRAY_SIZE*ESS_ARRAY_SIZE];
	m_pitmpDes16 = new int [ESS_ARRAY_SIZE*ESS_ARRAY_SIZE];
//TMM_ESS_UNIFIED }
}

__inline
void
DownConvert::xDestroyFilterTmmResidual ( )
{
  delete [] m_pitmpDes4;
  delete [] m_pitmpDes8;
  delete [] m_pitmpDes16;
}

__inline
void
DownConvert::xUpsamplingFrame( ResizeParameters* pcParameters,
                               int* piSrcBlock,   // low-resolution src-addr
                               int* piDesBlock,   // high-resolution src-addr
                               bool bLuma,
                               h264::MbDataCtrl* pcMbDataCtrl)
{
//TMM_ESS_UNIFIED {
  int iInWidth    = pcParameters->m_iInWidth;
  int iInHeight   = pcParameters->m_iInHeight;
  int iOutWidth   = pcParameters->m_iOutWidth;
  int iOutHeight  = pcParameters->m_iOutHeight;
  if (!bLuma)
  {
    iInWidth    >>= 1;
    iInHeight   >>= 1;
    iOutWidth   >>= 1;
    iOutHeight  >>= 1;
  }
	
	int iPelPerMb = bLuma?16:8;
	int iMbPerRow = iInWidth/iPelPerMb;
	
  int numCase=0;
  int nbOfBlksPerLine[3]={1,2,4};
	int iBlkWidthIn[3] ;
	int iBlkWidthOut[3];
	int iBlkHeightIn[3] ;
	int iBlkHeightOut[3];
	int Numerator[2];
	int Denominator[2];

  xComputeNumeratorDenominator(iInWidth, iOutWidth, &(Numerator[0]), &(Denominator[0]));
  xComputeNumeratorDenominator(iInHeight, iOutHeight, &(Numerator[1]), &(Denominator[1]));

  for (int cas=0; cas<3; cas++)
  {
   	iBlkWidthIn[cas] = Denominator[0] * (( (16/nbOfBlksPerLine[cas]) - 1)/Denominator[0] +1);
  	iBlkWidthOut[cas] = (Numerator[0] * iBlkWidthIn[cas]) / Denominator[0];
   	iBlkHeightIn[cas] = Denominator[1] * (( (16/nbOfBlksPerLine[cas]) - 1)/Denominator[1] +1);
  	iBlkHeightOut[cas] = (Numerator[1] * iBlkHeightIn[cas]) / Denominator[1];
  }
		
	int xoutCOMMON=0, youtCOMMON=0;
	int xNumphiCOMMON = 0, yNumphiCOMMON = 0;
	int youtSlice =0, xoutSlice=0;
	int yNumphiSlice=0, xNumphiSlice=0;

	for( int yin=0 ; yin < iInHeight; yin+= iPelPerMb )
	{
		int xoutMB=xoutSlice;
		int xNumphiMB=xNumphiSlice;
		for( int xin=0; xin < iInWidth;  xin+= iPelPerMb )
		{
			int youtMB=youtSlice;
			int yNumphiMB=yNumphiSlice;
			int* piSrc = piSrcBlock + xin+ yin*m_iImageStride;
			
			int* piDes = piDesBlock ;

			const h264::MbData& rcMbData = pcMbDataCtrl->getMbData((xin+yin*iMbPerRow)/iPelPerMb);
			
			if( bLuma )
			{
				if(rcMbData.isIntra16x16() )              numCase = 0;
        else if(rcMbData.isTransformSize8x8() )   numCase = 1;
        else                                      numCase = 2;
      }
      else                                        numCase = 1;

      int *pitmpDes = (numCase==0) ? m_pitmpDes16 : ( (numCase==1) ? m_pitmpDes8 : m_pitmpDes4 );
      int nbBlks = nbOfBlksPerLine[numCase];
			int iSize = iPelPerMb/nbBlks;

			int youtLine = youtMB, xoutLine = xoutMB;
			int yNumphiLine = yNumphiMB,xNumphiLine = xNumphiMB;

			for( int y = 0; y < nbBlks; y++)
			{
				int xoutCol=xoutLine;
				int xNumphiCol=xNumphiLine;

				for( int x=0; x < nbBlks; x++)
				{
					int youtCol=youtLine;
					int yNumphiCol=yNumphiLine;

					int iSrcOffset = iPelPerMb/nbBlks *(x+y*m_iImageStride);
					xUpsamplingBlock(iBlkWidthIn[numCase], iBlkHeightIn[numCase], iBlkWidthOut[numCase], iBlkHeightOut[numCase], 
                           piSrc + iSrcOffset, pitmpDes, xNumphiCol,yNumphiCol,Numerator,16/nbBlks);
					
					// xoutCol and xphi8x8 Update and Output affectation
					int ixlastsample,iylastsample;
					xComputeLastSamplePosition(xNumphiCol,yNumphiCol,iPelPerMb/nbBlks,Numerator,Denominator,&ixlastsample,&iylastsample);
					
					xCopyBuffer(pitmpDes,piDes,ixlastsample,iylastsample,xoutCol,youtCol,iBlkWidthOut[numCase]);

					
					xoutCol += ixlastsample;
					xNumphiCol = ( (xNumphiCol + ixlastsample*Denominator[0]) % Numerator[0] ) % ESS_ARRAY_SIZE; 

					youtCol += iylastsample;
					yNumphiCol = ( (yNumphiCol + iylastsample*Denominator[1]) % Numerator[1] ) % ESS_ARRAY_SIZE; 

					xNumphiCOMMON = xNumphiCol;
					yNumphiCOMMON = yNumphiCol;

					xoutCOMMON = xoutCol;
					youtCOMMON = youtCol;
				}
				yNumphiLine = yNumphiCOMMON;
				youtLine = youtCOMMON;
			}
			xNumphiMB = xNumphiCOMMON;
			xoutMB = xoutCOMMON;

    } // end of for( int xin=0; xin < iInWidth;  xin+= iPelPerMb )

		yNumphiSlice = yNumphiCOMMON;
		youtSlice = youtCOMMON;

  } // end of for( int yin=0 ; yin < iInHeight; yin+= iPelPerMb )
	
//TMM_ESS_UNIFIED }
}

__inline
void
//TMM_ESS_UNIFIED {
DownConvert::xUpsamplingBlock( int iInWidth,       // low-resolution width
                               int iInHeight,      // low-resolution height
                               int iOutWidth,       // high-resolution width
                               int iOutHeight,      // high-resolution height
                               int* piSrcBlock,  // low-resolution src-addr
                               int* piDesBlock,  // high-resolution src-addr
                               int xNumphi,
                               int yNumphi,
                               int* Numerator,
                               int iInBlockSize)
{
  iInWidth = iInWidth % ESS_ARRAY_SIZE;
  iInHeight = iInHeight % ESS_ARRAY_SIZE;
  iOutWidth = iOutWidth % ESS_ARRAY_SIZE;
  iOutHeight = iOutHeight % ESS_ARRAY_SIZE;


  // ===== vertical upsampling =====
	int xin,yin;
   for (xin=0; xin<iInBlockSize; xin++)
    {
      int* piSrc = &piSrcBlock[xin];
      for (yin=0; yin<iInBlockSize; yin++)
        m_paiTmp1dBufferIn[yin] = piSrc[yin * m_iImageStride];
      for (yin=iInBlockSize; yin<iInHeight; yin++)
        m_paiTmp1dBufferIn[yin] = piSrc[(iInBlockSize-1) * m_iImageStride];

      xUpsamplingDataResidual(iInHeight, iOutHeight, (double)xNumphi / Numerator[0]);
      
      for(int yout = 0; yout < iOutHeight; yout++ )
      {
        piDesBlock[yout*iOutWidth+xin] = m_paiTmp1dBufferOut[yout];
      }
    }

    for (xin=iInBlockSize; xin<iInWidth; xin++)
    {
      int* piSrc = &piSrcBlock[iInBlockSize-1];
      for (yin=0; yin<iInBlockSize; yin++)
        m_paiTmp1dBufferIn[yin] = piSrc[yin * m_iImageStride];
      for (yin=iInBlockSize; yin<iInHeight; yin++)
        m_paiTmp1dBufferIn[yin] = piSrcBlock[ iInBlockSize-1+(iInBlockSize-1)* m_iImageStride];

      xUpsamplingDataResidual(iInHeight, iOutHeight, (double)xNumphi / Numerator[0]);
      
      for(int yout = 0; yout < iOutHeight; yout++ )
      {
        piDesBlock[yout*iOutWidth+xin] = m_paiTmp1dBufferOut[yout];
      }
    }

  // ===== horizontal upsampling =====
  for (int yout=0; yout<iOutHeight; yout++)
    {
      int* piSrc = &piDesBlock[yout * iOutWidth];
      for (int xin=0; xin<iInWidth; xin++)
        m_paiTmp1dBufferIn[xin] = piSrc[xin];

      xUpsamplingDataResidual(iInWidth, iOutWidth, (double)yNumphi / Numerator[0]);

      memcpy(&piDesBlock[yout * iOutWidth], m_paiTmp1dBufferOut, iOutWidth*sizeof(int));
    }
}
//TMM_ESS_UNIFIED }

//TMM_ESS_UNIFIED {
__inline
void
DownConvert::xComputeLastSamplePosition( int xNumphi,
									                       int yNumphi,
									                       int iPelPerMb,
									                       int *Numerator,
									                       int *Denominator,
									                       int *pixlastsample,
									                       int *piylastsample)
{
	int ixtmp = (Numerator[0]*iPelPerMb - xNumphi);
	*pixlastsample = 1+(ixtmp-1)/Denominator[0];
	int iytmp = (Numerator[1]*iPelPerMb - yNumphi);
	*piylastsample = 1+(iytmp-1)/Denominator[1];

}
//TMM_ESS_UNIFIED }

__inline
void
DownConvert::xCopyBuffer(int *pitmpDes,
						 int *piDes,
						 int ixlastsample,
						 int iylastsample,
						 int ixout,
						 int iyout,
						 int iBlkWidthOut)
{
  iBlkWidthOut = iBlkWidthOut % ESS_ARRAY_SIZE; //TMM_ESS_UNIFIED

	int iDesOffset = ixout + iyout*m_iImageStride;
	for  (int iycopy = 0 ; iycopy<iylastsample ; iycopy++) 		
	{
	    memcpy(piDes+iDesOffset, pitmpDes +iBlkWidthOut*iycopy, ixlastsample*sizeof(int));
		iDesOffset += m_iImageStride; 
	}
}

#endif  // of #ifndef NO_MB_DATA_CTRL


// ===== Tools =====================================================================
__inline
void
DownConvert::xSetValue ( unsigned char* pucBuffer, int iStride, int iWidth, int iHeight, unsigned char value )
{
  for (int y=0; y<iHeight; y++)
    memset(&pucBuffer[y*iStride], value, iWidth);
}

__inline
void
DownConvert::xSetValue ( short* psBuffer, int iStride, int iWidth, int iHeight, short value )
{
  for (int y=0; y<iHeight; y++)
    for (int x=0; x<iWidth; x++)
      psBuffer[y*iStride + x] = value;
}

__inline
void
DownConvert::xUpsampling( ResizeParameters* pcParameters,
                          bool bLuma
                          )

{
  //printf("\nxUpsampling %dx%d -> %dx%d\n", iInWidth, iInHeight, iOutWidth, iOutHeight);

  switch (pcParameters->m_iIntraUpsamplingType)
    {
    case 1:
      xUpsampling1(pcParameters, bLuma);
      break;
    case 2:
      xUpsampling2(pcParameters, bLuma);
      break;

    case 3:
      xUpsampling3(pcParameters, bLuma);
      break;
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



// =================================================================================
//   INTRA 1 Lanczos
// =================================================================================
#define TMM_TABLE_SIZE 512
#define TMM_FILTER_WINDOW_SIZE 3

#if LANCZOS_OPTIM
#define NFACT    12
#define VALFACT  (1l<<NFACT)
#define MASKFACT (VALFACT-1)
#endif

//*************************************************************************
// lanczos filter coeffs computation
__inline
void
DownConvert::xInitFilterTmm1 (int iMaxDim )
{  
  const double pi = 3.14159265359;
#if LANCZOS_OPTIM
  m_padFilter = new long[TMM_TABLE_SIZE];
  m_padFilter[0] = VALFACT;
#else
  m_padFilter = new double[TMM_TABLE_SIZE];
  m_padFilter[0] = 1.0;
#endif
  for (int i=1; i<TMM_TABLE_SIZE; i++)
    {
      double x = ((double)i/TMM_TABLE_SIZE) * TMM_FILTER_WINDOW_SIZE;
      double pix = pi*x;
      double pixw = pix/TMM_FILTER_WINDOW_SIZE;
#if LANCZOS_OPTIM
      m_padFilter[i] = (long)(sin(pix)/pix * sin(pixw)/pixw * VALFACT);
#else
      m_padFilter[i] = sin(pix)/pix * sin(pixw)/pixw;
#endif
    } 
}

__inline
void
DownConvert::xDestroyFilterTmm1 ( )
{
  delete [] m_padFilter;
}


//*************************************************************************
__inline
void
DownConvert::xUpsampling1( ResizeParameters* pcParameters,
                           bool bLuma
                          )

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
#if LANCZOS_OPTIM
  long spos;
#endif

  // ===== vertical upsampling =====
  xComputeNumeratorDenominator(iInHeight,iOutHeight,&iNumerator,&iDenominator);
#if LANCZOS_OPTIM
  spos = (1<<NFACT) * iDenominator / iNumerator;
#endif
  for (int xin=0; xin<iInWidth; xin++)
    {
      int* piSrc = &m_paiImageBuffer[xin];
      for (int yin=0; yin<iInHeight; yin++)
        m_paiTmp1dBufferIn[yin] = piSrc[yin * m_iImageStride];

#if LANCZOS_OPTIM
      xUpsamplingData1(iInHeight, iOutHeight, spos);
#else
      xUpsamplingData1(iInHeight, iOutHeight, iNumerator, iDenominator);
#endif
      
      for(int yout = 0; yout < iOutHeight; yout++ )
        piSrc[yout*m_iImageStride] = m_paiTmp1dBufferOut[yout];
    }
  
  // ===== horizontal upsampling =====
  xComputeNumeratorDenominator(iInWidth,iOutWidth,&iNumerator,&iDenominator);
#if LANCZOS_OPTIM
  spos = (1<<NFACT) * iDenominator / iNumerator;
#endif
  for (int yout=0; yout<iOutHeight; yout++)
    {
      int* piSrc = &m_paiImageBuffer[yout * m_iImageStride];
      for (int xin=0; xin<iInWidth; xin++)
        m_paiTmp1dBufferIn[xin] = piSrc[xin];

#if LANCZOS_OPTIM
      xUpsamplingData1(iInWidth, iOutWidth, spos);
#else
      xUpsamplingData1(iInWidth, iOutWidth, iNumerator, iDenominator);
#endif

      memcpy(piSrc, m_paiTmp1dBufferOut, iOutWidth*sizeof(int));
    }
}

#if LANCZOS_OPTIM
__inline
void
DownConvert::xUpsamplingData1 ( int iInLength , int iOutLength , long spos )
#else
__inline
void
DownConvert::xUpsamplingData1 ( int iInLength , int iOutLength , int iNumerator , int iDenominator )
#endif
{
#if LANCZOS_OPTIM
  long dpos0 = 0;
  for (int iout=0; iout<iOutLength; iout++)
    {
      dpos0 += spos;
      //long dpos0 = (iout<<NFACT) * iDenominator / iNumerator;
      long rpos0 = dpos0 & MASKFACT;
      int ipos0 = dpos0 >> NFACT;
      if (rpos0 == 0) {
        m_paiTmp1dBufferOut[iout] = m_paiTmp1dBufferIn[ipos0];
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
          if (i<0)               val = m_paiTmp1dBufferIn[0];
          else if (i>=iInLength) val = m_paiTmp1dBufferIn[iInLength-1];
          else                   val = m_paiTmp1dBufferIn[i];
          sval += val * fact;
        }
      m_paiTmp1dBufferOut[iout] = sval>>NFACT;
    }
#else
  for (int iout=0; iout<iOutLength; iout++)
    {
      double dpos0 = ((double)iout * iDenominator / iNumerator);
      int ipos0 = (int)dpos0;
      double rpos0 = dpos0 - ipos0;
      if ((int)(rpos0 * 1000) == 0) {
        m_paiTmp1dBufferOut[iout] = m_paiTmp1dBufferIn[ipos0];
        continue;
      }
      
      int begin=(int)ceil(dpos0-TMM_FILTER_WINDOW_SIZE);
      int end = (int)(dpos0+TMM_FILTER_WINDOW_SIZE);
          
      double sval = 0;
      for (int i=begin; i<=end; i++)
        {
          double posi = (i-ipos0) - rpos0;
          double fact = xGetFilter(posi);
          int val;
          if (i<0)               val = m_paiTmp1dBufferIn[0];
          else if (i>=iInLength) val = m_paiTmp1dBufferIn[iInLength-1];
          else                   val = m_paiTmp1dBufferIn[i];
          sval += val * fact;
        }
      m_paiTmp1dBufferOut[iout] = (int)sval;
    }
#endif
}
#if LANCZOS_OPTIM
__inline
long
DownConvert::xGetFilter ( long x )
{
  x = abs(x);
  int ind = (int)((x / TMM_FILTER_WINDOW_SIZE) * TMM_TABLE_SIZE) >> NFACT;
  if (ind >= TMM_TABLE_SIZE) return 0;
  return m_padFilter[ind];
}
#else
__inline
double
DownConvert::xGetFilter ( double x )
{
  x = fabs(x);
  int ind = (int)((x / TMM_FILTER_WINDOW_SIZE) * TMM_TABLE_SIZE);
  if (ind >= TMM_TABLE_SIZE) return 0.0;
  return m_padFilter[ind];
}
#endif

#undef NFACT
#undef VALFACT
#undef MASKFACT
#undef TMM_TABLE_SIZE
#undef TMM_FILTER_WINDOW_SIZE



// =================================================================================
//   INTRA 2
// =================================================================================
__inline
void
DownConvert::xInitFilterTmm2 (int iMaxDim )
{
  m_Tmp1dBufferInHalfpel = new int [iMaxDim];
  m_Tmp1dBufferInQ1pel   = new int [iMaxDim];
  m_Tmp1dBufferInQ3pel   = new int [iMaxDim];
}

__inline
void
DownConvert::xDestroyFilterTmm2 ( )
{
  delete [] m_Tmp1dBufferInHalfpel;
	delete [] m_Tmp1dBufferInQ1pel;
	delete [] m_Tmp1dBufferInQ3pel;
}

__inline
void
DownConvert::xUpsampling2( ResizeParameters* pcParameters, bool bLuma
                          )

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

  // ===== vertical upsampling =====
  xComputeNumeratorDenominator(iInHeight,iOutHeight,&iNumerator,&iDenominator);
  for (int xin=0; xin<iInWidth; xin++)
    {
      int* piSrc = &m_paiImageBuffer[xin];
      for (int yin=0; yin<iInHeight; yin++)
        m_paiTmp1dBufferIn[yin] = piSrc[yin * m_iImageStride];

      xUpsamplingData2(iInHeight, iOutHeight, iNumerator , iDenominator);
      
      for(int yout = 0; yout < iOutHeight; yout++ )
        piSrc[yout*m_iImageStride] = m_paiTmp1dBufferOut[yout];
    }
  
  // ===== horizontal upsampling =====
  xComputeNumeratorDenominator(iInWidth, iOutWidth, &iNumerator, &iDenominator);
  for (int yout=0; yout<iOutHeight; yout++)
    {
      int* piSrc = &m_paiImageBuffer[yout * m_iImageStride];
      for (int xin=0; xin<iInWidth; xin++)
        m_paiTmp1dBufferIn[xin] = piSrc[xin];

      xUpsamplingData2(iInWidth, iOutWidth, iNumerator , iDenominator);

//TMM_ESS_UNIFIED {
      for( int i = 0; i < iOutWidth; i++ )
        piSrc[i] = ( m_paiTmp1dBufferOut[i] + 512 ) >> 10;
//TMM_ESS_UNIFIED }
  }
}

//TMM_ESS_UNIFIED {
__inline
void
DownConvert::xUpsamplingData2 ( int iInLength , int iOutLength , int iNumerator , int iDenominator )
{
  int  *Tmp1dBufferInHalfpel = m_Tmp1dBufferInHalfpel;
  int  *Tmp1dBufferInQ1pel = m_Tmp1dBufferInQ1pel;
  int  *Tmp1dBufferInQ3pel = m_Tmp1dBufferInQ3pel;

  int x,y;
  int iTemp;

  // half pel samples (6 taps 1 -5 20 20 -5 1)
  for(  x = 0; x < iInLength ; x++)
    {
      y=x;
      iTemp  = m_paiTmp1dBufferIn[y];
      y = (x+1 < iInLength ? x+1 : iInLength -1);
      iTemp += m_paiTmp1dBufferIn[y];
      iTemp  = iTemp << 2;
      y = (x-1 >= 0  ? x-1 : 0);
      iTemp -= m_paiTmp1dBufferIn[y];
      y = (x+2 < iInLength ? x+2 : iInLength -1);
      iTemp -= m_paiTmp1dBufferIn[y];
      iTemp += iTemp << 2;
      y = (x-2 >= 0  ? x-2 : 0);
      iTemp += m_paiTmp1dBufferIn[y];
      y = (x+3 < iInLength ? x+3 : iInLength -1);
      iTemp += m_paiTmp1dBufferIn[y];
      Tmp1dBufferInHalfpel[x] = iTemp;
    }

  // 1/4 pel samples
  for( x = 0; x < iInLength-1 ; x++)
    {
      Tmp1dBufferInQ1pel[x] = ( (m_paiTmp1dBufferIn[x]<<5) + Tmp1dBufferInHalfpel[x] + 1) >> 1;
      Tmp1dBufferInQ3pel[x] = ( (m_paiTmp1dBufferIn[x+1]<<5) + Tmp1dBufferInHalfpel[x] + 1) >> 1;
    }
  Tmp1dBufferInQ1pel[iInLength-1] = ( (m_paiTmp1dBufferIn[iInLength-1]<<5) + Tmp1dBufferInHalfpel[iInLength-1] + 1) >> 1;
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
          m_paiTmp1dBufferOut[iout] =  m_paiTmp1dBufferIn[ipos0] << 5; // original pel value
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
          m_paiTmp1dBufferOut[iout] =  m_paiTmp1dBufferIn[ipos1] << 5; // original pel value
          break;

        }
    }  
}
//TMM_ESS_UNIFIED }


// =================================================================================
//   INTRA 3 
// =================================================================================

__inline
void
DownConvert::upsample3( unsigned char* pucBufferY, unsigned char* pucBufferU, unsigned char* pucBufferV,
                        int input_width, int input_height, int output_width, int output_height,
                        int crop_x0, int crop_y0, int crop_w, int crop_h, 
                        int input_chroma_phase_shift_x, int input_chroma_phase_shift_y,
                        int output_chroma_phase_shift_x, int output_chroma_phase_shift_y)
{
  //===== luma =====
  xCopyToImageBuffer  ( pucBufferY, input_width,   input_height,   input_width );
  xUpsampling3        ( input_width, input_height, output_width, output_height,
                        crop_x0, crop_y0, crop_w, crop_h, 0, 0, 0, 0, 0 );
  xCopyFromImageBuffer( pucBufferY, output_width, output_height, output_width, 0, 255 );
  //===== chroma cb =====
  xCopyToImageBuffer  ( pucBufferU, input_width/2, input_height/2, input_width/2 );
  xUpsampling3        ( input_width/2, input_height/2, output_width/2, output_height/2,
                        crop_x0/2, crop_y0/2, crop_w/2, crop_h/2, 
                        input_chroma_phase_shift_x, input_chroma_phase_shift_y,
                        output_chroma_phase_shift_x, output_chroma_phase_shift_y, 1 );
  xCopyFromImageBuffer( pucBufferU, output_width/2, output_height/2, output_width/2, 0, 255 );
  //===== chroma cr =====
  xCopyToImageBuffer  ( pucBufferV, input_width/2, input_height/2, input_width/2 );
  xUpsampling3        ( input_width/2, input_height/2, output_width/2, output_height/2,
                        crop_x0/2, crop_y0/2, crop_w/2, crop_h/2, 
                        input_chroma_phase_shift_x, input_chroma_phase_shift_y,
                        output_chroma_phase_shift_x, output_chroma_phase_shift_y, 1 );
  xCopyFromImageBuffer( pucBufferV, output_width/2, output_height/2, output_width/2, 0, 255 );  
}

__inline
void
DownConvert::xUpsampling3( ResizeParameters* pcParameters,
                           bool bLuma
                          )
{
  int fact = (bLuma ? 1 : 2);
  int input_width   = pcParameters->m_iInWidth   /fact;
  int input_height  = pcParameters->m_iInHeight  /fact;
  // SSUN@SHARP
  int output_width  = pcParameters->m_iGlobWidth /fact;  
  int output_height = pcParameters->m_iGlobHeight/fact;
  int crop_x0 = pcParameters->m_iPosX /fact;
  int crop_y0 = pcParameters->m_iPosY /fact;
  int crop_w = pcParameters->m_iOutWidth /fact;
  int crop_h = pcParameters->m_iOutHeight/fact;  
  // End of SSUN@SHARP
  int input_chroma_phase_shift_x = pcParameters->m_iBaseChromaPhaseX;
  int input_chroma_phase_shift_y = pcParameters->m_iBaseChromaPhaseY;
  int output_chroma_phase_shift_x = pcParameters->m_iChromaPhaseX;
  int output_chroma_phase_shift_y = pcParameters->m_iChromaPhaseY;
  bool uv_flag = !bLuma;

  xUpsampling3(input_width, input_height,
    output_width, output_height,
    crop_x0, crop_y0, crop_w, crop_h,
    input_chroma_phase_shift_x, input_chroma_phase_shift_y,
    output_chroma_phase_shift_x, output_chroma_phase_shift_y,
    uv_flag );
}

__inline
void
DownConvert::xUpsampling3( int input_width, int input_height, int output_width, int output_height,
                           int crop_x0, int crop_y0, int crop_w, int crop_h,
                           int input_chroma_phase_shift_x, int input_chroma_phase_shift_y,
                           int output_chroma_phase_shift_x, int output_chroma_phase_shift_y, bool uv_flag )
{
#if DIRECT_INTERP  // SSUN@SHARP
  const int filter16[16][6] = { // Lanczos3
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
#endif
  int i, j, k, *px, *py, div_scale, div_shift;
#if DIRECT_INTERP  // SSUN@SHARP
  int up_res = 16;
  int x16, y16, x, y, m;
#else
  int up_res = 4;
  unsigned char **QPel=0;
#endif

  // initialization
  px = (int *)malloc(sizeof(int)*output_width);
  py = (int *)malloc(sizeof(int)*output_height);
#if !DIRECT_INTERP  // SSUN@SHARP
  QPel = (unsigned char **) malloc (sizeof(unsigned char *)*(input_height+8)*4);
  QPel[0] = (unsigned char *) malloc ((input_width+8)*(input_height+8)*16);
  for( j=1; j<((input_height+8)*4); j++) QPel[j] = QPel[j-1] + (input_width+8)*4;
#endif
  div_shift = 0;
  while( (1<<(div_shift+1)) < crop_w ) div_shift++;
  div_shift += 15;
  div_scale =((1<<div_shift)+crop_w/2)/crop_w;
  for( i = 0; i < output_width; i++ )
  {
    if( i<crop_x0 || i>=(crop_x0+crop_w) )
      px[i]=-128;
    else
    {
      k = (i-crop_x0)*input_width*up_res + up_res/4*(2+input_chroma_phase_shift_x)*input_width - up_res/4*(2+output_chroma_phase_shift_x)*crop_w;
      px[i] = ((((k&0xffff)*div_scale)>>16)+(k>>16)*div_scale)>>(div_shift-16);
    }
  }
  div_shift = 0;
  while( (1<<(div_shift+1)) < crop_h ) div_shift++;
  div_shift += 15;
  div_scale =((1<<div_shift)+crop_h/2)/crop_h;
  for( j = 0; j < output_height; j++ )
  {
    if( j<crop_y0 || j>=(crop_y0+crop_h) )
      py[j]=-128;
    else
    {
      k = (j-crop_y0)*input_height*up_res + up_res/4*(2+input_chroma_phase_shift_y)*input_height - up_res/4*(2+output_chroma_phase_shift_y)*crop_h;
      py[j] = ((((k&0xffff)*div_scale)>>16)+(k>>16)*div_scale)>>(div_shift-16);
    }
  }
#if DIRECT_INTERP  // SSUN@SHARP
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
#else
  // 1/4-pel interpolation
  UnifiedOneForthPix ( QPel, input_width, input_height );

  // extract upsampled data
  for( j = 0; j < output_height; j++ )
  {
    int*  piDes = &m_paiImageBuffer[j*m_iImageStride];
    for( i = 0; i < output_width; i++ )
    {
      if( px[i]==-128 ||  py[j]==-128) piDes[i] = 128;
      else piDes[i] = QPel[py[j]+16][px[i]+16];
    }
  }
#endif
  // free memory
  free(px);
  free(py);
#if !DIRECT_INTERP  // SSUN@SHARP
  free(QPel[0]);
  free(QPel);
#endif
}

__inline
void 
DownConvert::UnifiedOneForthPix ( unsigned char **out4Y, int img_width, int img_height )  // borrowed/modified from JM software
{
  int is;
  int i, j, j4;
  int ie2, je2, jj, maxy;
  int  **imgY;
  int  **img4Y_tmp;

  // initialization
  imgY = (int **)malloc(sizeof(int *)*img_height);
  imgY[0] = &m_paiImageBuffer[0];
  for( j=1; j<img_height; j++ ) imgY[j] = imgY[j-1] + m_iImageStride;
  img4Y_tmp = (int **)malloc(sizeof(int *)*(img_height+8));
  img4Y_tmp[0] = (int *)malloc(sizeof(int)*(img_height+8)*(img_width+8)*2);
  for( j=1; j<(img_height+8); j++ ) img4Y_tmp[j] = img4Y_tmp[j-1] + (img_width+8)*2;

  for (j = -4; j < img_height + 4; j++)
  {
    jj = max (0, min (img_height - 1, j));
    for (i = -4; i < img_width + 4; i++)
    {
      is =
        (20 *
        (imgY[jj][max (0, min (img_width - 1, i))] +
         imgY[jj][max (0, min (img_width - 1, i + 1))]) +
        (-5) *
        (imgY[jj][max (0, min (img_width - 1, i - 1))] +
         imgY[jj][max (0, min (img_width - 1, i + 2))]) +
        1 *
        (imgY[jj][max (0, min (img_width - 1, i - 2))] +
         imgY[jj][max (0, min (img_width - 1, i + 3))]));
      img4Y_tmp[j + 4][(i + 4) * 2] = imgY[jj][max (0, min (img_width - 1, i))] * 1024;    // 1/1 pix pos
      img4Y_tmp[j + 4][(i + 4) * 2 + 1] = is * 32;  // 1/2 pix pos
    }
  }
  
  for (i = 0; i < (img_width + 2 * 4) * 2; i++)
  {
    for (j = 0; j < img_height + 2 * 4; j++)
    {
      j4 = j * 4;
      maxy = img_height + 2 * 4 - 1;
      is =
        (20 *
        (img4Y_tmp[j][i] + img4Y_tmp[min (maxy, j + 1)][i]) +
         (-5) * (img4Y_tmp[max (0, j - 1)][i] +
         img4Y_tmp[min (maxy, j + 2)][i]) +
         1 * (img4Y_tmp[max (0, j - 2)][i] +
         img4Y_tmp[min (maxy, j + 3)][i])) / 32;
      
      out4Y[j*4][i*2] = max (0, min(255, (img4Y_tmp[j][i] + 512) / 1024));  // 1/2 pix
      out4Y[j*4+2][i*2] = max (0, min(255, (is + 512) / 1024));   // 1/2 pix
    }
  }
  
  /* 1/4 pix */
  /* luma */
  ie2 = (img_width + 2 * 4 - 1) * 4;
  je2 = (img_height + 2 * 4 - 1) * 4;
  
  for (j = 0; j < je2 + 4; j += 2)
    for (i = 0; i < ie2 + 3; i += 2)
    {
      /*  '-'  */
      out4Y[j][i+1] = max (0, min (255, ((int)out4Y[j][i] + (int)out4Y[j][min (ie2 + 2, i + 2)]+1) / 2));
    }
  for (i = 0; i < ie2 + 4; i++)
  {
    for (j = 0; j < je2 + 3; j += 2)
    {
      if (i % 2 == 0)
      {
        /*  '|'  */
        out4Y[j+1][i] = max (0, min (255, ((int)out4Y[j][i] + (int)out4Y[min (je2 + 2, j + 2)][i]+1) / 2));
      }
      else if ((j % 4 == 0 && i % 4 == 1) || (j % 4 == 2 && i % 4 == 3))
      {
        /*  '/'  */
        out4Y[j + 1][i] = max (0, min (255, ((int)out4Y[j][min (ie2 + 2, i + 1)] + (int)out4Y[min (je2 + 2, j + 2)][i - 1] + 1) / 2));
      }
      else
      {
        /*  '\'  */
        out4Y[j + 1][i] = max (0, min (255, ((int)out4Y[j][i - 1] + (int)out4Y[min (je2 + 2, j + 2)][min (ie2 + 2, i + 1)] + 1) / 2));
      }
    }
  }
    
  // free memory
  free( imgY );
  free( img4Y_tmp[0] );
  free( img4Y_tmp );
}


__inline
void
DownConvert::downsample3( unsigned char* pucBufferY, unsigned char* pucBufferU, unsigned char* pucBufferV,
                          int input_width, int input_height, int output_width, int output_height,
                          int crop_x0, int crop_y0, int crop_w, int crop_h, 
                          int input_chroma_phase_shift_x, int input_chroma_phase_shift_y,
                          int output_chroma_phase_shift_x, int output_chroma_phase_shift_y)
{
  //===== luma =====
  xCopyToImageBuffer  ( pucBufferY, input_width,   input_height,   input_width );
  xDownsampling3      ( input_width, input_height, output_width, output_height,
                        crop_x0, crop_y0, crop_w, crop_h, 0, 0, 0, 0, 0 );
  xCopyFromImageBuffer( pucBufferY, output_width, output_height, output_width, 0, 255 );
  //===== chroma cb =====
  xCopyToImageBuffer  ( pucBufferU, input_width/2, input_height/2, input_width/2 );
  xDownsampling3      ( input_width/2, input_height/2, output_width/2, output_height/2,
                        crop_x0/2, crop_y0/2, crop_w/2, crop_h/2, 
                        input_chroma_phase_shift_x, input_chroma_phase_shift_y,
                        output_chroma_phase_shift_x, output_chroma_phase_shift_y, 1 );
  xCopyFromImageBuffer( pucBufferU, output_width/2, output_height/2, output_width/2, 0, 255 );
  //===== chroma cr =====
  xCopyToImageBuffer  ( pucBufferV, input_width/2, input_height/2, input_width/2 );
  xDownsampling3      ( input_width/2, input_height/2, output_width/2, output_height/2,
                        crop_x0/2, crop_y0/2, crop_w/2, crop_h/2, 
                        input_chroma_phase_shift_x, input_chroma_phase_shift_y,
                        output_chroma_phase_shift_x, output_chroma_phase_shift_y, 1 );
  xCopyFromImageBuffer( pucBufferV, output_width/2, output_height/2, output_width/2, 0, 255 );
}


__inline
void
DownConvert::xDownsampling3( int input_width, int input_height, int output_width, int output_height,
                             int crop_x0, int crop_y0, int crop_w, int crop_h,
                             int input_chroma_phase_shift_x, int input_chroma_phase_shift_y,
                             int output_chroma_phase_shift_x, int output_chroma_phase_shift_y, bool uv_flg )
{
  const int filter16[3][16][12] = { 
                                    // test
                                    {
                                      {1,-5,-6,10,38,52,38,10,-6,-5,1,0},
                                      {1,-5,-6,9,36,51,39,12,-5,-5,1,0},
                                      {1,-4,-6,8,35,50,40,14,-4,-5,0,0},
                                      {1,-4,-6,7,33,49,41,15,-3,-5,0,0},
                                      {1,-4,-6,6,31,49,42,17,-2,-5,-1,0},
                                      {1,-3,-6,5,29,48,42,19,-1,-5,-1,0},
                                      {1,-3,-6,4,27,47,43,21,0,-5,-1,0},
                                      {1,-2,-6,3,26,46,44,22,1,-5,-2,0},
                                      {1,-2,-6,2,24,45,45,24,2,-6,-2,1},
                                      {0,-2,-5,1,22,44,46,26,3,-6,-2,1},
                                      {0,-1,-5,0,21,43,47,27,4,-6,-3,1},
                                      {0,-1,-5,-1,19,42,48,29,5,-6,-3,1},
                                      {0,-1,-5,-2,17,42,49,31,6,-6,-4,1},
                                      {0,0,-5,-3,15,41,49,33,7,-6,-4,1},
                                      {0,0,-5,-4,14,40,50,35,8,-6,-4,1},
                                      {0,1,-5,-5,12,39,51,36,9,-6,-5,1}
                                    },
                                    // Kaiser, N=3, D=2, beta=4
                                    { 
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
                                    // Kaiser, N=3, D=1, beta=4
                                    { 
                                      {0,0,0,0,0,128,0,0,0,0,0,0},
                                      {0,0,0,2,-6,127,7,-2,0,0,0,0},
                                      {0,0,0,3,-11,124,15,-4,1,0,0,0},
                                      {0,0,0,4,-14,119,24,-6,1,0,0,0},
                                      {0,0,0,4,-17,114,34,-9,2,0,0,0},
                                      {0,0,0,4,-18,106,45,-11,2,0,0,0},
                                      {0,0,0,4,-19,97,56,-13,3,0,0,0},
                                      {0,0,0,4,-18,88,66,-15,3,0,0,0},
                                      {0,0,0,4,-17,77,77,-17,4,0,0,0},
                                      {0,0,0,3,-15,66,88,-18,4,0,0,0},
                                      {0,0,0,3,-13,56,97,-19,4,0,0,0},
                                      {0,0,0,2,-11,45,106,-18,4,0,0,0},
                                      {0,0,0,2,-9,34,114,-17,4,0,0,0},
                                      {0,0,0,1,-6,24,119,-14,4,0,0,0},
                                      {0,0,0,1,-4,15,124,-11,3,0,0,0},
                                      {0,0,0,0,-2,7,127,-6,2,0,0,0}
                                    }
                                  };
  int i, j, k, m, *px, *py, x, y, x16, y16, filter;

  // initialization
  px = (int *)malloc(sizeof(int)*output_width);
  py = (int *)malloc(sizeof(int)*output_height);
  if(crop_w*3 > 5*output_width) filter = 0;  // test
  else if(crop_w*4 > 5*output_width) filter = 1;
  else filter = 2;

  //========== horizontal downsampling ===========
  for( i = 0; i < output_width; i++ )
  {
    px[i] = 16*crop_x0 + ( i*crop_w*16 + 4*(2+input_chroma_phase_shift_x)*crop_w - 4*(2+output_chroma_phase_shift_x)*output_width) / output_width;
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
  for( j = 0; j < output_height; j++ )
  {
    py[j] = 16*crop_y0 + ( j*crop_h*16 + 4*(2+input_chroma_phase_shift_y)*crop_h - 4*(2+output_chroma_phase_shift_y)*output_height) / output_height;
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
  free(px);
  free(py);
}






// =================================================================================
//   INTER 1/2
// =================================================================================
#ifndef NO_MB_DATA_CTRL
__inline
void
DownConvert::xUpsamplingDataResidual ( int iInLength , int iOutLength, double phase_init)
{
  int  *Tmp1dBufferInHalfpel = m_Tmp1dBufferInHalfpel;
  int  *Tmp1dBufferInQ1pel = m_Tmp1dBufferInQ1pel;
  int  *Tmp1dBufferInQ3pel = m_Tmp1dBufferInQ3pel;

  int x,y;
  int iTemp;

   for(  x = 0; x < iInLength ; x++)
     {
       y=x;
       iTemp  = m_paiTmp1dBufferIn[y];
       y = (x+1 < iInLength ? x+1 : iInLength -1);
       iTemp += m_paiTmp1dBufferIn[y];
       iTemp  = iTemp << 2;
       y = (x-1 >= 0  ? x-1 : 0);
       iTemp -= m_paiTmp1dBufferIn[y];
       y = (x+2 < iInLength ? x+2 : iInLength -1);
       iTemp -= m_paiTmp1dBufferIn[y];
       iTemp += iTemp << 2;
       y = (x-2 >= 0  ? x-2 : 0);
       iTemp += m_paiTmp1dBufferIn[y];
       y = (x+3 < iInLength ? x+3 : iInLength -1);
       iTemp += m_paiTmp1dBufferIn[y];
       Tmp1dBufferInHalfpel[x] = ((iTemp + 16) >> 5);
     }

   for( x = 0; x < iInLength -1; x++)
	   {
       Tmp1dBufferInQ1pel[x] = (m_paiTmp1dBufferIn[x] + Tmp1dBufferInHalfpel[x] + 1) >> 1;
       Tmp1dBufferInQ3pel[x] = (m_paiTmp1dBufferIn[x+1] + Tmp1dBufferInHalfpel[x] + 1) >> 1;
     }
   Tmp1dBufferInQ1pel[iInLength-1] = (m_paiTmp1dBufferIn[iInLength-1] + Tmp1dBufferInHalfpel[iInLength-1] + 1) >> 1;
   Tmp1dBufferInQ3pel[iInLength-1] = Tmp1dBufferInHalfpel[iInLength-1] ;

   for (int iout=0; iout<iOutLength; iout++)
     {
       double    dpos0 = ((double)iout * iInLength / iOutLength);
       int       ipos0 = (int)dpos0;
       double    rpos0 = dpos0 - ipos0;

       int iIndex = (int) (8 * rpos0);
       switch (iIndex)
         {
         case 0:
           m_paiTmp1dBufferOut[iout] =  m_paiTmp1dBufferIn[ipos0];
           break;
			
         case 1:
         case 2:
           m_paiTmp1dBufferOut[iout] =  Tmp1dBufferInQ1pel[ipos0];
           break;
			
         case 3:
         case 4:
           m_paiTmp1dBufferOut[iout] =  Tmp1dBufferInHalfpel[ipos0];
           break;
			
         case 5:
         case 6:
           m_paiTmp1dBufferOut[iout] =  Tmp1dBufferInQ3pel[ipos0];
           break;
			
         case 7:
           int ipos1 = (ipos0+1 < iInLength) ? ipos0+1 : ipos0;
           m_paiTmp1dBufferOut[iout] =  m_paiTmp1dBufferIn[ipos1];
           break;

         }
     }
 }

#endif

//*************************************************************************











#undef DEFAULTY
#undef DEFAULTU
#undef DEFAULTV

