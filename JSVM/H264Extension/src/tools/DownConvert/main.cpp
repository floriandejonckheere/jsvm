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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DOWN_CONVERT_STATIC

#include "DownConvert.h"


typedef struct
{
  int             width;
  int             height;
  unsigned char*  data;
} ColorComponent;

typedef struct
{
  ColorComponent lum;
  ColorComponent cb;
  ColorComponent cr;
} YuvFrame;



void createColorComponent( ColorComponent* cc )
{
 if( ! ( cc->data = new unsigned char[cc->width * cc->height]))
  {
   fprintf(stderr, "\nERROR: memory allocation failed!\n\n");
    exit(1);
  }
}

void deleteColorComponent( ColorComponent* cc )
{
  delete[] cc->data;
  cc->data = NULL;
}


void setColorComponent ( ColorComponent* cc, unsigned char value )
{
  memset(cc->data, value, cc->width*cc->height);
}


void createFrame( YuvFrame* f, int width, int height )
{
  f->lum.width = width;    f->lum.height  = height;     createColorComponent( &f->lum );
  f->cb .width = width/2;  f->cb .height  = height/2;   createColorComponent( &f->cb  );
  f->cr .width = width/2;  f->cr .height  = height/2;   createColorComponent( &f->cr  );
}

void deleteFrame( YuvFrame* f )
{
  deleteColorComponent( &f->lum );
  deleteColorComponent( &f->cb  );
  deleteColorComponent( &f->cr  );
}

void setFrame ( YuvFrame* f, unsigned char y_value,  unsigned char u_value, unsigned char v_value)
{
  setColorComponent(&f->lum, y_value);
  setColorComponent(&f->cb,  u_value);
  setColorComponent(&f->cr,  v_value);
}

void clearFrame ( YuvFrame* f)
{
  setFrame(f, 0, 0, 0);
}

void readColorComponent( ColorComponent* cc, FILE* file, int inwidth, int inheight )
{
  int rsize;

  for( int i = 0; i < inheight; i++ )
  {
    rsize = fread( cc->data+i*cc->width, sizeof(unsigned char), inwidth, file );

    if( rsize != inwidth )
    {
      fprintf(stderr, "\nERROR: while reading from input file!\n\n");
      exit(1);
    }
  }
}

void writeColorComponent( ColorComponent* cc, FILE* file, int outwidth, int outheight )
{
  int wsize;

  for( int i = 0; i < outheight; i++ )
  {
    wsize = fwrite( cc->data+i*cc->width, sizeof(unsigned char), outwidth, file );

    if( outwidth != wsize )
    {
      fprintf(stderr, "\nERROR: while writing to output file!\n\n");
      exit(1);
    }
  }
}

void readFrame( YuvFrame* f, FILE* file, int width, int height )
{
  readColorComponent( &f->lum, file, width, height );
  readColorComponent( &f->cb,  file, width/2, height/2 );
  readColorComponent( &f->cr,  file, width/2, height/2 );
}

void writeFrame( YuvFrame* f, FILE* file, int width, int height )
{
  writeColorComponent( &f->lum, file, width, height );
  writeColorComponent( &f->cb,  file, width/2, height/2 );
  writeColorComponent( &f->cr,  file, width/2, height/2 );
}

void ess_readPic ( YuvFrame* f, FILE* file, int input_width, int input_height, bool ess_downsample_flg, int resample_mode )
{
  if( !ess_downsample_flg && (resample_mode==6 || resample_mode==7) )
  {
    memcpy(f->lum.data, f->lum.data+f->lum.width*f->lum.height/2, input_height*input_width/2 );
    memcpy(f->cb.data, f->cb.data+f->cb.width*f->cb.height/2, input_height*input_width/8 );
    memcpy(f->cr.data, f->cr.data+f->cr.width*f->cr.height/2, input_height*input_width/8 );
    return; 
  }

  // read Y
  int rsize = fread( f->lum.data, input_height, input_width, file );

  if( rsize != input_width )
  {
    fprintf(stderr, "\nERROR: while reading from input file!\n\n");
    exit(1);
  }
  // read Cb
  rsize = fread( f->cb.data, input_height/2, input_width/2, file );

  if( rsize != input_width/2 )
  {
    fprintf(stderr, "\nERROR: while reading from input file!\n\n");
    exit(1);
  }
  // read Cr
  rsize = fread( f->cr.data, input_height/2, input_width/2, file );

  if( rsize != input_width/2 )
  {
    fprintf(stderr, "\nERROR: while reading from input file!\n\n");
    exit(1);
  }

  if( ( ess_downsample_flg && (resample_mode==1 || resample_mode==2 || resample_mode==3) ) ||
      (!ess_downsample_flg && (resample_mode==1 || resample_mode==4 || resample_mode==5) )   )
  {
    int y;
    int top_first = (resample_mode!=3 && resample_mode!=5);
    int buf_siz = f->lum.width*f->lum.height/2;

    for(y=0; y<input_height/2; y++)
    {
      memcpy(f->lum.data+buf_siz+y*input_width, f->lum.data+(2*y+top_first)*input_width, input_width);
      memcpy(f->lum.data+buf_siz+buf_siz/2+y*input_width, f->lum.data+(2*y+1-top_first)*input_width, input_width);
    }
    memcpy(f->lum.data, f->lum.data+buf_siz+buf_siz/2, input_width*input_height/2 );
    buf_siz >>= 2;
    for(y=0; y<input_height/4; y++)
    {
      memcpy(f->cb.data+buf_siz+y*input_width/2, f->cb.data+(2*y+top_first)*input_width/2, input_width/2);
      memcpy(f->cb.data+buf_siz+buf_siz/2+y*input_width/2, f->cb.data+(2*y+1-top_first)*input_width/2, input_width/2);
      memcpy(f->cr.data+buf_siz+y*input_width/2, f->cr.data+(2*y+top_first)*input_width/2, input_width/2);
      memcpy(f->cr.data+buf_siz+buf_siz/2+y*input_width/2, f->cr.data+(2*y+1-top_first)*input_width/2, input_width/2);
    }
    memcpy(f->cb.data, f->cb.data+buf_siz+buf_siz/2, input_width*input_height/8 );
    memcpy(f->cr.data, f->cr.data+buf_siz+buf_siz/2, input_width*input_height/8 );
  }
}


void ess_writePic ( YuvFrame* f, FILE* file, int output_width, int output_height, bool ess_downsample_flg, int resample_mode )
{
  int buf_siz = f->lum.width*f->lum.height/2;
  if((ess_downsample_flg && (resample_mode==4 || resample_mode==5) ) || resample_mode==1)
  {
    memcpy(f->lum.data+buf_siz+buf_siz/2, f->lum.data, output_width*output_height/2 );
    memcpy(f->cb.data+buf_siz/4+buf_siz/8, f->cb.data, output_width*output_height/8 );
    memcpy(f->cr.data+buf_siz/4+buf_siz/8, f->cr.data, output_width*output_height/8 );
    if(resample_mode!=1) return;
  }
  if(ess_downsample_flg && resample_mode>5) 
  {
    memcpy(f->lum.data+buf_siz, f->lum.data, output_width*output_height/2 );
    memcpy(f->cb.data+buf_siz/4, f->cb.data, output_width*output_height/8 );
    memcpy(f->cr.data+buf_siz/4, f->cr.data, output_width*output_height/8 );
  }
  if((ess_downsample_flg && resample_mode>5) || resample_mode==1)
  {
    int y;
    int top_first=(resample_mode==6 || resample_mode==1);
    for(y=0; y<output_height/2; y++)
    {
      memcpy(f->lum.data+(2*y+top_first)*output_width, f->lum.data+buf_siz+y*output_width, output_width);
      memcpy(f->lum.data+(2*y+1-top_first)*output_width, f->lum.data+buf_siz+buf_siz/2+y*output_width, output_width);
    }
    buf_siz >>= 2;
    for(y=0; y<output_height/4; y++)
    {
      memcpy(f->cb.data+(2*y+top_first)*output_width/2, f->cb.data+buf_siz+y*output_width/2, output_width/2);
      memcpy(f->cb.data+(2*y+1-top_first)*output_width/2, f->cb.data+buf_siz+buf_siz/2+y*output_width/2, output_width/2);
      memcpy(f->cr.data+(2*y+top_first)*output_width/2, f->cr.data+buf_siz+y*output_width/2, output_width/2);
      memcpy(f->cr.data+(2*y+1-top_first)*output_width/2, f->cr.data+buf_siz+buf_siz/2+y*output_width/2, output_width/2);
    }
  }
  // write Y
  int rsize = fwrite( f->lum.data, output_height, output_width, file );

  if( rsize != output_width )
  {
    fprintf(stderr, "\nERROR: while reading from input file!\n\n");
    exit(1);
  }
  // write Cb
  rsize = fwrite( f->cb.data, output_height/2, output_width/2, file );

  if( rsize != output_width/2 )
  {
    fprintf(stderr, "\nERROR: while reading from input file!\n\n");
    exit(1);
  }
  // write Cr
  rsize = fwrite( f->cr.data, output_height/2, output_width/2, file );

  if( rsize != output_width/2 )
  {
    fprintf(stderr, "\nERROR: while reading from input file!\n\n");
    exit(1);
  }
}


void ess_resamplePic ( YuvFrame* pcFrame, DownConvert& rcDownConvert, int input_width, int input_height, 
                        int output_width, int output_height, int crop_x0, int crop_y0, int crop_w, int crop_h,
                        int input_chroma_phase_shift_x, int input_chroma_phase_shift_y, 
                       int output_chroma_phase_shift_x, int output_chroma_phase_shift_y, 
                       bool ess_downsample_flg, int resample_mode)
{
  if(ess_downsample_flg)
  {
    if(resample_mode==4 || resample_mode==7)
    {
      rcDownConvert.downsample_ver( pcFrame->lum.data,  pcFrame->lum.width,
                                    pcFrame->cb .data,  pcFrame->cb .width,
                                    pcFrame->cr .data,  pcFrame->cr .width,
                                    input_width, input_height,  1 );
    }
    else if(resample_mode==5 || resample_mode==6)
    {
      rcDownConvert.downsample_ver( pcFrame->lum.data,  pcFrame->lum.width,
                                    pcFrame->cb .data,  pcFrame->cb .width,
                                    pcFrame->cr .data,  pcFrame->cr .width,
                                    input_width, input_height,  0 );
    }
    rcDownConvert.downsample3( pcFrame->lum.data, pcFrame->cb.data, pcFrame->cr.data,
                               input_width, input_height, output_width, output_height,
                               crop_x0, crop_y0, crop_w, crop_h, 
                               input_chroma_phase_shift_x, input_chroma_phase_shift_y,
                               output_chroma_phase_shift_x, output_chroma_phase_shift_y,
                               resample_mode, pcFrame->lum.height*pcFrame->lum.width/2);
  }
  else
  {
    rcDownConvert.upsample3( pcFrame->lum.data, pcFrame->cb.data, pcFrame->cr.data,
                             input_width, input_height, output_width, output_height,
                             crop_x0, crop_y0, crop_w, crop_h, 
                             input_chroma_phase_shift_x, input_chroma_phase_shift_y,
                             output_chroma_phase_shift_x, output_chroma_phase_shift_y,
                             resample_mode, pcFrame->lum.height*pcFrame->lum.width/2);
    if(resample_mode==2 || resample_mode==4 || resample_mode==7)
    {
      rcDownConvert.upsample_ver( pcFrame->lum.data,  pcFrame->lum.width,
                                  pcFrame->cb .data,  pcFrame->cb .width,
                                  pcFrame->cr .data,  pcFrame->cr .width,
                                  output_width, output_height/2,
                                  crop_y0, crop_h, 1 );
    }
    else if(resample_mode==3 || resample_mode==5 || resample_mode==6)
    {
      rcDownConvert.upsample_ver( pcFrame->lum.data,  pcFrame->lum.width,
                                  pcFrame->cb .data,  pcFrame->cb .width,
                                  pcFrame->cr .data,  pcFrame->cr .width,
                                  output_width, output_height/2,  
                                  crop_y0, crop_h, 0 );
    }
  }
}


void print_usage_and_exit( int test, char* name, char* message = 0 )
{
  if( test )
  {
    if( message )
    {
      fprintf ( stderr, "\nERROR: %s\n", message );
    }

    fprintf (   stderr, "\nUsage: %s <win> <hin> <in> <wout> <hout> <out> [<method> [<t> [<skip> [<frms>]]]] [[-crop <args>] [-phase <args>] [-resample_mode <arg>]]\n\n", name );
    fprintf (   stderr, "  win     : input width  (luma samples)\n" );
    fprintf (   stderr, "  hin     : input height (luma samples)\n" );
    fprintf (   stderr, "  in      : input file\n" );
	  fprintf (   stderr, "  wout    : output width  (luma samples)\n" );
    fprintf (   stderr, "  hout    : output height (luma samples)\n" );
    fprintf (   stderr, "  out     : output file\n" );
    fprintf (   stderr, "\n--------------------------- OPTIONAL ---------------------------\n\n" );
	  fprintf (   stderr, "  method  : rescaling methods (default: 0)\n" );
	  fprintf (   stderr, "            0: normative upsampling\n" );
 	  fprintf (   stderr, "               non-normative downsampling (JVT-R006)\n" );
    fprintf (   stderr, "            1: dyadic upsampling (AVC 6-tap (1/2 pel) on odd samples\n" );
    fprintf (   stderr, "               dyadic downsampling (MPEG-4 downsampling filter)\n" );
    fprintf (   stderr, "            2: crop only\n" );
    fprintf (   stderr, "            3: upsampling (Three-lobed Lanczos-windowed sinc)\n" );
    fprintf (   stderr, "                 4: upsampling (JVT-O041: AVC 6-tap 1/2 pel + bilinear 1/4 pel)\n" );
    fprintf (   stderr, "  t       : number of temporal downsampling stages (default: 0)\n" );
    fprintf (   stderr, "  skip    : number of frames to skip at start (default: 0)\n" );
    fprintf (   stderr, "  frms    : number of frames wanted in output file (default: max)\n" );
    fprintf (   stderr, "\n-------------------------- OVERLOADED --------------------------\n\n" );
    fprintf (   stderr, " -crop  <type> <params>\n");
    fprintf (   stderr, "   type   : 0: Sequence level,    1: Picture level\n");
    fprintf (   stderr, "   params : IF Sequence level: <x_orig> <y_orig> <crop_width> <crop_height>\n");
    fprintf (   stderr, "               cropping window origin (x,y) and dimensions (width and height)\n");
    fprintf (   stderr, "            IF Picture level: <crop_file>\n");
    fprintf (   stderr, "                 input file containing cropping window parameters.\n" );
    fprintf (   stderr, "                 each line has four integer numbers separated by a comma\n" );
    fprintf (   stderr, "                 as following: \"x_orig, y_orig, crop_width, crop_height\"\n");
    fprintf (   stderr, "                 for each picture to be resampled;\n" );
    fprintf (   stderr, "\n");
    fprintf (   stderr, " -phase <in_uv_ph_x> <in_uv_ph_y> <out_uv_ph_x> <out_uv_ph_y>\n");
    fprintf (   stderr, "   in_uv_ph_x : input  chroma phase shift in horizontal direction (default:-1)\n" );
    fprintf (   stderr, "   in_uv_ph_y : input  chroma phase shift in vertical   direction (default: 0)\n" );
    fprintf (   stderr, "   out_uv_ph_x: output chroma phase shift in horizontal direction (default:-1)\n" );
    fprintf (   stderr, "   out_uv_ph_y: output chroma phase shift in vertical   direction (default: 0)\n" );
    fprintf (   stderr, "\n");
    fprintf (   stderr, " -resample_mode <resample_mode>\n");
    fprintf (   stderr, "   resample_mode : resampling modes, present when method==0 (default: 0)\n" );
    fprintf (   stderr, "                 0: low-res-frm  = progressive, high-res-frm = progressive\n" );
    fprintf (   stderr, "                 1: low-res-frm  = interlaced,  high-res-frm = interlaced\n" );
    fprintf (   stderr, "                 2: low-res-frm  = progressive  (top-coincided)\n" );
    fprintf (   stderr, "                    high-res-frm = interlaced\n" );
    fprintf (   stderr, "                 3: low-res-frm  = progressive  (bot-coincided)\n" );
    fprintf (   stderr, "                    high-res-frm = interlaced\n" );
    fprintf (   stderr, "                 4: low-res-frm  = interlaced   (top-first)\n" );
    fprintf (   stderr, "                    high-res-frm = progressive  (double frm rate)\n" );
    fprintf (   stderr, "                 5: low-res-frm  = interlaced   (bot-first)\n" );
    fprintf (   stderr, "                    high-res-frm = progressive  (double frm rate)\n" );
    fprintf (   stderr, "\n\n");
    exit    (   1 );
  }
}

void updateCropParametersFromFile(ResizeParameters * rp, FILE * crop_file, int method, char* name)
{
  int crop_x0;
  int crop_y0;
  int crop_w;
  int crop_h;
  if ((fscanf(crop_file,"%d,%d,%d,%d\n", &crop_x0, &crop_y0, &crop_w,&crop_h))!=EOF)
  {
    rp->m_iPosX      = crop_x0;
    rp->m_iPosY      = crop_y0;
    rp->m_iOutWidth  = crop_w;
    rp->m_iOutHeight = crop_h;
  }
  
  print_usage_and_exit ((rp->m_iPosX&1||rp->m_iPosY&1||rp->m_iOutWidth&1||rp->m_iOutHeight&1), name, "Crop parameters must be event values");
  print_usage_and_exit (((method==2)&&((rp->m_iOutWidth != min(rp->m_iInWidth, rp->m_iGlobWidth))||(rp->m_iOutHeight != min(rp->m_iInHeight, rp->m_iGlobHeight)))), name, "Crop dimensions must be the same as the minimal dimensions");
  print_usage_and_exit ((rp->m_iOutWidth>max(rp->m_iInWidth,rp->m_iGlobWidth)||rp->m_iOutHeight>max(rp->m_iInHeight,rp->m_iGlobHeight)||rp->m_iOutWidth<min(rp->m_iInWidth,rp->m_iGlobWidth)||rp->m_iOutHeight<min(rp->m_iInHeight,rp->m_iGlobHeight)),name,"wrong crop window size");
  print_usage_and_exit (!((rp->m_iPosX+rp->m_iOutWidth)<=max(rp->m_iInWidth,rp->m_iGlobWidth)&&(rp->m_iPosY+rp->m_iOutHeight)<=max(rp->m_iInHeight,rp->m_iGlobHeight)),name,"wrong crop window size and origin");
}


int main(int argc, char *argv[])
{
  DownConvert   cDownConvert;
  ResizeParameters * rp = new ResizeParameters;
  
  rp->m_iInWidth                   = 0;
  rp->m_iInHeight                  = 0;
  rp->m_iGlobWidth                 = 0;
  rp->m_iGlobHeight                = 0;
  rp->m_iBaseChromaPhaseX          =-1;
  rp->m_iBaseChromaPhaseY          = 0;
  rp->m_iChromaPhaseX              =-1;
  rp->m_iChromaPhaseY              = 0;
  rp->m_iPosX                      = 0;
  rp->m_iPosY                      = 0;
  rp->m_iOutWidth                  = 0;
  rp->m_iOutHeight                 = 0;
  rp->m_iExtendedSpatialScalability= 0;
  rp->m_iPoc                       = 0;
  rp->m_pParamFile                 = 0;  
  
  int   iStage              = 0;
  int   temporal_stages     = 0;
  int   skip_at_start       = 0;
  int   number_frames       = (1<<30);
  FILE* input_file          = 0;
  FILE* output_file         = 0;
  FILE* crop_file           = 0;
  int   method              = 0;
  bool  resample            = false;
  bool  upsample            = false;
  bool  crop_init           = false;
  bool  phase_init          = false;
  bool  method_init         = false;
  bool  crop_file_init      = false;
  int   sequence_length     = 0;
  int   skip_between        = 0;
  int   i;
  int   frame_width;
  int   frame_height;
  int   resample_mode       = 0;
  bool   downsample_flg      = 1;
  
  int           written, skip;
  YuvFrame      cFrame;
  
  print_usage_and_exit ((argc<7||argc>24), argv[0],"number of arguments");
  rp->m_iInWidth        = atoi  ( argv[1] );
  rp->m_iInHeight       = atoi  ( argv[2] );
  input_file            = fopen ( argv[3], "rb" );
  rp->m_iGlobWidth      = atoi  ( argv[4] );
  rp->m_iGlobHeight     = atoi  ( argv[5] );
  output_file           = fopen ( argv[6], "wb" );
  
  print_usage_and_exit ((input_file == NULL||output_file == NULL),argv[0],"failed to open file");
  print_usage_and_exit(((rp->m_iInWidth>rp->m_iGlobWidth&&rp->m_iInHeight<rp->m_iGlobHeight)||(rp->m_iInWidth<rp->m_iGlobWidth&&rp->m_iInHeight>rp->m_iGlobHeight)),argv[0],"mixed Upsample and Downsample");
  
  fseek(  input_file, 0, SEEK_END );
  sequence_length = ((unsigned int)ftell(input_file)/(3*rp->m_iInWidth*rp->m_iInHeight/2));
  fseek(  input_file, 0, SEEK_SET );
  
  i = 7;
  while (i<argc)
  {
    if (strcmp(argv[i], "-crop")==0)
    {
      print_usage_and_exit ((method == 1), argv[0], "No crop in Dyadic method");
      print_usage_and_exit (((method == 2)&&(rp->m_iInWidth<rp->m_iGlobWidth)), argv[0], "No crop only while upsampling");
      print_usage_and_exit ((crop_init||argc<(i+3)||argc==(i+4)||argc==(i+5)),argv[0],"Error in crop parameters");
      crop_init = true;
      i++;
      print_usage_and_exit (!(atoi(argv[i])==0||atoi(argv[i])==1),argv[0],"Wrong crop type");
      rp->m_iExtendedSpatialScalability = (atoi(argv[i]))+1;
      print_usage_and_exit(((rp->m_iExtendedSpatialScalability!=1)&&(rp->m_iExtendedSpatialScalability!=2)),argv[0],"Wrong crop type");
      i++;
      if (rp->m_iExtendedSpatialScalability==1)
      {
        rp->m_iPosX = atoi  ( argv[i] );
        i++;
        rp->m_iPosY = atoi  ( argv[i] );
        i++;
        rp->m_iOutWidth   = atoi  ( argv[i] );
        i++;
        rp->m_iOutHeight  = atoi  ( argv[i] );
        i++;
        print_usage_and_exit ((rp->m_iPosX&1||rp->m_iPosY&1||rp->m_iOutWidth&1||rp->m_iOutHeight&1), argv[0], "Crop parameters must be event values");
        print_usage_and_exit (((method==2)&&((rp->m_iOutWidth != min(rp->m_iInWidth, rp->m_iGlobWidth))||(rp->m_iOutHeight != min(rp->m_iInHeight, rp->m_iGlobHeight)))), argv[0], "Crop dimensions must be the same as the minimal dimensions");
        print_usage_and_exit ((rp->m_iOutWidth>max(rp->m_iInWidth,rp->m_iGlobWidth)||rp->m_iOutHeight>max(rp->m_iInHeight,rp->m_iGlobHeight)||rp->m_iOutWidth<min(rp->m_iInWidth,rp->m_iGlobWidth)||rp->m_iOutHeight<min(rp->m_iInHeight,rp->m_iGlobHeight)),argv[0],"wrong crop window size");
        print_usage_and_exit (!((rp->m_iPosX+rp->m_iOutWidth)<=max(rp->m_iInWidth,rp->m_iGlobWidth)&&(rp->m_iPosY+rp->m_iOutHeight)<=max(rp->m_iInHeight,rp->m_iGlobHeight)),argv[0],"wrong crop window size and origin");
      }
      else
      {
        crop_file_init = true;
        crop_file = fopen ( argv[i], "rb" );
        i++;
        print_usage_and_exit ((crop_file == NULL),argv[0],"failed to open crop parameters file");
      }
    }
    else if (strcmp(argv[i], "-phase")==0)
    {
      print_usage_and_exit ((method != 0), argv[0], "Phase only in normative resampling");
           
      print_usage_and_exit ((phase_init||argc<(i+5)),argv[0],"wrong number of phase parameters");
      i++;
      phase_init = true;
      rp->m_iBaseChromaPhaseX  = atoi  ( argv[i] );
      i++;
      rp->m_iBaseChromaPhaseY  = atoi  ( argv[i] );
      i++;
      rp->m_iChromaPhaseX = atoi  ( argv[i] );
      i++;
      rp->m_iChromaPhaseY = atoi  ( argv[i] );
      i++;
      print_usage_and_exit ((rp->m_iBaseChromaPhaseX>1||rp->m_iBaseChromaPhaseX<-1||rp->m_iBaseChromaPhaseY>1||rp->m_iBaseChromaPhaseY<-1||rp->m_iChromaPhaseX>1||rp->m_iChromaPhaseX<-1||rp->m_iChromaPhaseY>1||rp->m_iChromaPhaseY<-1),argv[0],"Wrong phase parameters (range : [-1, 1])");
    }

    else if (strcmp(argv[i], "-resample_mode")==0)
    {
      print_usage_and_exit ((method != 0), argv[0], "resample_mode only in normative resampling");
           
      resample = true;
      i++;
      resample_mode     = atoi  ( argv[i] );
      if (resample_mode > 5 || resample_mode < 0) resample_mode = 0;
      i++;
    }      
    else if (i == 7)
    {
      method_init = true;
      method = atoi  ( argv[i] );
      i++;
      print_usage_and_exit ((method<0||method>4),argv[0],"wrong method");
      if (method>2)
      {
        fprintf( stderr, "\nNot normative, nor dyadic resampling or not crop only\n");
        print_usage_and_exit((rp->m_iInWidth>rp->m_iGlobWidth||rp->m_iInHeight>rp->m_iGlobHeight),argv[0],"Wrong method for downsampling");
      }
      if (!(method == 2))
      {
        resample = true;
        if (rp->m_iInWidth < rp->m_iGlobWidth)
        {
          upsample = true;
					downsample_flg = !upsample;//TMM_INTERLACE
        }
      }
      if (method==1)
      {
        if (upsample)
        {
          int div = rp->m_iGlobWidth / rp->m_iInWidth;
          if      (div == 1) iStage = 0;
          else if (div == 2) iStage = 1;
          else if (div == 4) iStage = 2;
          else if (div == 8) iStage = 3;
          else { print_usage_and_exit(true, argv[0], "ratio not supported for dyadic upsampling method"); }
          print_usage_and_exit((((rp->m_iGlobWidth / rp->m_iInWidth)*rp->m_iInWidth)!=rp->m_iGlobWidth), argv[0],"ratio is not dyadic");
          print_usage_and_exit((rp->m_iInHeight*div != rp->m_iGlobHeight), argv[0], "Not the same ratio for Height and Width in dyadic mode");
        }
        else
        {
          int div = rp->m_iInWidth / rp->m_iGlobWidth;
          if      (div == 1) {iStage = 0; fprintf( stderr, "\nNo resampling in dyadic method\n");}
          else if (div == 2)  iStage = 1;
          else if (div == 4)  iStage = 2;
          else if (div == 8)  iStage = 3;
          else { print_usage_and_exit(true, argv[0], "ratio not supported for dyadic upsampling method"); }
          print_usage_and_exit((((rp->m_iInWidth / rp->m_iGlobWidth)*rp->m_iGlobWidth)!=rp->m_iInWidth), argv[0],"ratio is not dyadic");
          print_usage_and_exit((rp->m_iGlobHeight*div != rp->m_iInHeight), argv[0], "Not the same ratio for Height and Width in dyadic mode");
        }
      }
    }
    else if (i == 8)
    {
      temporal_stages = atoi ( argv[i] );
      i++;
      print_usage_and_exit ((temporal_stages<0),argv[0],"Error in temporal stage");
    }
    else if (i == 9)
    {
      skip_at_start = atoi ( argv[i] );
      i++;
      print_usage_and_exit (((skip_at_start<0)||(skip_at_start>=sequence_length)),argv[0],"Error in number of frame to skip at start");
    }
    else if (i == 10)
    {
      number_frames = atoi ( argv[i] );
      i++;
      print_usage_and_exit ((number_frames<0),argv[0],"Error in number of frames");
    }
    else
    {
      print_usage_and_exit (true,argv[0]);
    }
  }
  
  if (!method_init)
  {
    resample = true;
    if (rp->m_iInWidth < rp->m_iGlobWidth)
    {
      upsample = true;
			downsample_flg = !upsample;//TMM_INTERLACE
    }
  }
  
  if (!crop_init)
  {
    rp->m_iOutWidth = max(rp->m_iInWidth,rp->m_iGlobWidth);
    rp->m_iOutHeight = max(rp->m_iInHeight,rp->m_iGlobHeight);
  }
  
  if (method == 2)
  {
    if (!crop_init)
    {
      rp->m_iOutWidth = min(rp->m_iInWidth,rp->m_iGlobWidth);
      rp->m_iOutHeight = min(rp->m_iInHeight,rp->m_iGlobHeight);
      fprintf( stderr, "\nCrop parameters set to default 0,0,min_width,min_height\n");
    }
  }
  
  skip_between    = ( 1 << temporal_stages ) - 1;
  
  if ( number_frames > ((sequence_length - skip_at_start+((1<<temporal_stages)-1))>>temporal_stages) )
  {
    if (number_frames != (1 << 30))
    {
      fprintf( stderr, "\nWrong number of frames\n");
    }
    number_frames   = ((sequence_length - skip_at_start+((1<<temporal_stages)-1))>>temporal_stages);
  }
    
  frame_width = (rp->m_iInWidth > rp->m_iGlobWidth) ? rp->m_iInWidth : rp->m_iGlobWidth;
  frame_height = (rp->m_iInHeight > rp->m_iGlobHeight) ? rp->m_iInHeight : rp->m_iGlobHeight;  
    
  createFrame( &cFrame, frame_width*2, frame_height*2 );
  cDownConvert.init( frame_width*2, frame_height*2 );
  
  long start_time = clock();
  
  for( skip = skip_at_start, rp->m_iPoc = 0, written = 0; ((rp->m_iPoc < sequence_length)&&(written < number_frames)); rp->m_iPoc++, skip = skip_between )
  {
    for( int num_skip = skip; num_skip > 0; num_skip-- )
    {
      fseek( input_file, rp->m_iInWidth*rp->m_iInHeight*3/2, SEEK_CUR);
    }
    rp->m_iPoc += skip;
    
    if ((rp->m_iPoc < sequence_length)&&(written < number_frames))
    {
      clearFrame      ( &cFrame );
      
      if(method)
      readFrame       ( &cFrame, input_file, rp->m_iInWidth, rp->m_iInHeight );      
      else
        ess_readPic   ( &cFrame, input_file, rp->m_iInWidth, rp->m_iInHeight, downsample_flg, resample_mode );
      
      if (crop_file_init&&rp->m_iExtendedSpatialScalability==2)
      {
        updateCropParametersFromFile(rp, crop_file, method, argv[0]);
      }
      if ((rp->m_iOutWidth==min(rp->m_iInWidth, rp->m_iGlobWidth))&&
          (rp->m_iOutHeight==min(rp->m_iInHeight, rp->m_iGlobHeight))&&
           method!=0)
      {
        resample = false;
      }
      else
      {
        resample = true;
      }
      
      if ((!resample) && (!upsample))
      {
        cDownConvert.crop(cFrame.lum.data, cFrame.lum.width, cFrame.cb.data, cFrame.cb.width, cFrame.cr.data, cFrame.cr.width, rp);      
      }
      else
      {
        if (upsample)
        {
          switch (method)
          {
          case 1:
            {
              FILTER_UP
              FILTER_UP_CHROMA
              cDownConvert.upsample(cFrame.lum.data, cFrame.lum.width, cFrame.cb.data, cFrame.cb.width, cFrame.cr.data, cFrame.cr.width, rp, iStage, piFilter, piFilter_chroma);
              break;
            }
          case 0:
            {
              ess_resamplePic ( &cFrame, cDownConvert, rp->m_iInWidth, rp->m_iInHeight, rp->m_iGlobWidth, rp->m_iGlobHeight,
                                rp->m_iPosX, rp->m_iPosY, rp->m_iOutWidth, rp->m_iOutHeight, rp->m_iBaseChromaPhaseX, rp->m_iChromaPhaseY,
                                rp->m_iChromaPhaseX, rp->m_iChromaPhaseY, downsample_flg, resample_mode);
              break;
            }
          case 3:
          case 4:
            {
              cDownConvert.upsample_non_dyadic(cFrame.lum.data, cFrame.lum.width, cFrame.cb.data, cFrame.cb.width, cFrame.cr.data, cFrame.cr.width, rp, method);
              break;
            }
          default:
            {
              print_usage_and_exit (true, argv[0], "Wrong upsample");
            }
          }
        }
        if (!upsample)
        {
          switch (method)
          {
          case 0:
            {
              //cDownConvert.downsample3(cFrame.lum.data, cFrame.lum.width, cFrame.cb.data, cFrame.cb.width, cFrame.cr.data, cFrame.cr.width, rp);
              ess_resamplePic ( &cFrame, cDownConvert, rp->m_iInWidth, rp->m_iInHeight, rp->m_iGlobWidth, rp->m_iGlobHeight,
                                rp->m_iPosX, rp->m_iPosY, rp->m_iOutWidth, rp->m_iOutHeight, rp->m_iBaseChromaPhaseX, rp->m_iChromaPhaseY,
                                rp->m_iChromaPhaseX, rp->m_iChromaPhaseY, downsample_flg, resample_mode);
              break;
            }
          case 1:
            {
              FILTER_DOWN
              cDownConvert.downsample(cFrame.lum.data, cFrame.lum.width, cFrame.cb.data, cFrame.cb.width, cFrame.cr.data, cFrame.cr.width, rp, iStage, piFilter);
              break;
            }
          default:
            {
              print_usage_and_exit (true, argv[0], "Wrong downsample");
            }
          }
        }
      }

      if(method!=0)
      writeFrame ( &cFrame, output_file,  rp->m_iGlobWidth, rp->m_iGlobHeight );
      else
      ess_writePic ( &cFrame, output_file, rp->m_iGlobWidth, rp->m_iGlobHeight, downsample_flg, resample_mode );

      if(resample_mode>3){
        if(downsample_flg){
          fseek( input_file, skip*rp->m_iInWidth*rp->m_iInHeight*3/2, SEEK_CUR);
          rp->m_iPoc += skip;
       //   written++;
        }
        if (crop_file_init&&rp->m_iExtendedSpatialScalability==2)
        {
          updateCropParametersFromFile(rp, crop_file, method, argv[0]);
        }
        ess_readPic   ( &cFrame, input_file, rp->m_iInWidth, rp->m_iInHeight, downsample_flg, resample_mode+2 );
        ess_resamplePic ( &cFrame, cDownConvert, rp->m_iInWidth, rp->m_iInHeight, rp->m_iGlobWidth, rp->m_iGlobHeight,
                          rp->m_iPosX, rp->m_iPosY, rp->m_iOutWidth, rp->m_iOutHeight, rp->m_iBaseChromaPhaseX, rp->m_iChromaPhaseY,
                          rp->m_iChromaPhaseX, rp->m_iChromaPhaseY, downsample_flg, resample_mode+2 );
        ess_writePic ( &cFrame, output_file, rp->m_iGlobWidth, rp->m_iGlobHeight, downsample_flg, resample_mode+2 );
      }

      fprintf( stderr, "\r%6d frames converted", ++written );
    }
  }
  long end_time = clock();
  
  deleteFrame( &cFrame     );
  fclose     ( input_file  );
  fclose     ( output_file );
  
  if (crop_file_init)
  {
    fclose   ( crop_file   );
  }
  
  fprintf(stderr, "\n" );
  double delta_in_s = (double)(end_time - start_time) / CLOCKS_PER_SEC;
  fprintf(stderr, "in %.2lf seconds => %.0lf ms/frame\n", delta_in_s, delta_in_s/written*1000);
  //TMM_FIX
  delete rp;

  return 0;
}
