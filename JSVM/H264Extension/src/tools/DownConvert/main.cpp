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

#define NO_MB_DATA_CTRL
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
  if( ! ( cc->data = (unsigned char*) malloc (cc->width * cc->height * sizeof(unsigned char)) ) )
  {
    fprintf(stderr, "\nERROR: malloc failed!\n\n");
    exit(1);
  }
}

void deleteColorComponent( ColorComponent* cc )
{
  free( cc->data );
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


void ess_readFrame( YuvFrame* f, FILE* file, int input_width, int input_height )
{
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
}
void ess_writeFrame( YuvFrame* f, FILE* file, int output_width, int output_height )
{
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

void ess_resampleFrame( YuvFrame* pcFrame, DownConvert& rcDownConvert, int input_width, int input_height, 
                        int output_width, int output_height, int crop_x0, int crop_y0, int crop_w, int crop_h,
                        int input_chroma_phase_shift_x, int input_chroma_phase_shift_y, 
                        int output_chroma_phase_shift_x, int output_chroma_phase_shift_y, bool ess_downsample_flg)
{
  if(ess_downsample_flg)
  {
    rcDownConvert.downsample3( pcFrame->lum.data, pcFrame->cb.data, pcFrame->cr.data,
                               input_width, input_height, output_width, output_height,
                               crop_x0, crop_y0, crop_w, crop_h, 
                               input_chroma_phase_shift_x, input_chroma_phase_shift_y,
                               output_chroma_phase_shift_x, output_chroma_phase_shift_y);
  }
  else
    {
      rcDownConvert.upsample3( pcFrame->lum.data, pcFrame->cb.data, pcFrame->cr.data,
                               input_width, input_height, output_width, output_height,
                               crop_x0, crop_y0, crop_w, crop_h, 
                               input_chroma_phase_shift_x, input_chroma_phase_shift_y,
                               output_chroma_phase_shift_x, output_chroma_phase_shift_y);
  }
}

void resampleFrame( int do_tmm, YuvFrame* pcFrame, DownConvert& rcDownConvert, int inwidth, int inheight, int outwidth, int outheight, int* piFilter, int method ) 
{
  if (do_tmm)
  {
	  ResizeParameters param;
    param.m_iExtendedSpatialScalability = ESS_SEQ;
    param.m_iSpatialScalabilityType = SST_RATIO_X;
    param.m_iIntraUpsamplingType = method;
    param.m_iInWidth   = inwidth;
    param.m_iInHeight  = inheight;
    param.m_iOutWidth  = outwidth;
    param.m_iOutHeight = outheight;
    param.m_bCrop      = false;
    param.m_iPosX      = 0;
    param.m_iPosY      = 0;
    param.m_iGlobWidth = outwidth;
    param.m_iGlobHeight= outheight;
    
    rcDownConvert.upsample_tmm ( pcFrame->lum.data,   pcFrame->lum.width,
                                 pcFrame->cb .data,   pcFrame->cb .width,
                                 pcFrame->cr .data,   pcFrame->cr .width,
                                 &param );
    return;
  }
    
  if ((outwidth > inwidth) || (outheight > inheight))
  {
    int div = outwidth / inwidth;
    int iStagesUp;
    if      (div == 1) iStagesUp = 0;
    else if (div == 2) iStagesUp = 1;
    else if (div == 4) iStagesUp = 2;
    else if (div == 8) iStagesUp = 3;
    else { fprintf(stderr, "x%d not supported\n", div); exit(1); }
    if( piFilter )
    {
      rcDownConvert.upsample  ( pcFrame->lum.data,   pcFrame->lum.width,
                                pcFrame->cb .data,   pcFrame->cb .width,
                                pcFrame->cr .data,   pcFrame->cr .width,
                                inwidth, inheight, piFilter, iStagesUp );
      return;
    }

    rcDownConvert.upsample  ( pcFrame->lum.data,   pcFrame->lum.width,
                              pcFrame->cb .data,   pcFrame->cb .width,
                              pcFrame->cr .data,   pcFrame->cr .width,
                              inwidth, inheight, iStagesUp );
  }
  else
  {
    if ( (((inwidth / outwidth)*outwidth) != inwidth) ||
         (((inheight / outheight)*outheight) != inheight)
         )
      { fprintf(stderr, "downsample not integer not supported\n"); exit(1); }
    int div = inwidth / outwidth;
    int iStagesDown;
    if      (div == 1) iStagesDown = 0;
    else if (div == 2) iStagesDown = 1;
    else if (div == 4) iStagesDown = 2;
    else if (div == 8) iStagesDown = 3;
    else { fprintf(stderr, "1/%d not supported\n", div); exit(1); }
    rcDownConvert.downsample( pcFrame->lum.data,  pcFrame->lum.width,
                              pcFrame->cb .data,  pcFrame->cb .width,
                              pcFrame->cr .data,  pcFrame->cr .width,
                              inwidth, inheight, iStagesDown );
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
    fprintf (   stderr, "\nUsage: %s <w> <h> <in> <s> <t> <out> [<skip> [<frms> [FILTER_COEFFS]]]\n\n", name );
    fprintf (   stderr, "\t    w: input width  (luma samples)\n" );
    fprintf (   stderr, "\t    h: input height (luma samples)\n" );
    fprintf (   stderr, "\t   in: input file\n" );
    fprintf (   stderr, "\t    s: number of spatial  downsampling stages\n" );
    fprintf (   stderr, "\t    t: number of temporal downsampling stages\n" );
    fprintf (   stderr, "\t  out: output file\n" );
    fprintf (   stderr, "\t skip: number of frames to skip at start (default: 0)\n" );
    fprintf (   stderr, "\t frms: number of maximum input frames (default: max)\n" );
    fprintf (   stderr, "\n" );
    fprintf (   stderr, "\n--------------------------- OR ---------------------------\n\n" );
    fprintf (   stderr, "\nUsage: %s -ess <ess_mode> <in_file> <win> <hin> <out_file> <wout> <hout> [<in_uv_ph_x> <in_uv_ph_y> <out_uv_ph_x> <out_uv_ph_y> [<crop_paras_file> [<t> [<skip> [<frms>]]]]]\n\n", name );
    fprintf (   stderr, "       ess_mode: 1:sequence-level cropping parameters\n\t\t 2:picture-level cropping parameters\n" );
    fprintf (   stderr, "        in_file: input file\n" );
    fprintf (   stderr, "            win: input width  (luma samples)\n" );
    fprintf (   stderr, "            hin: input height (luma samples)\n" );
    fprintf (   stderr, "       out_file: output file\n" );
    fprintf (   stderr, "           wout: output width  (luma samples)\n" );
    fprintf (   stderr, "           hout: output height (luma samples)\n" );
    fprintf (   stderr, "     in_uv_ph_x: input chroma phase shift in horizontal direction (default:-1)\n" );
    fprintf (   stderr, "     in_uv_ph_y: input chroma phase shift in vertical direction (default:-1)\n" );
    fprintf (   stderr, "    out_uv_ph_x: output chroma phase shift in horizontal direction (default:-1)\n" );
    fprintf (   stderr, "    out_uv_ph_y: output chroma phase shift in vertical direction (default:-1)\n" );
    fprintf (   stderr, "crop_paras_file: input file containing cropping window parameters.\n" );
    fprintf (   stderr, "                 data format - each line has three (even) integer numbers\n" );
    fprintf (   stderr, "                 representing x_orig, y_orig, crop_window_width (luma samples)\n" );
    fprintf (   stderr, "                 for each picture to be resampled; when ess_mode is \"1\", the\n" );
    fprintf (   stderr, "                 parameters of the first line will be used for all pictures;\n" );
    fprintf (   stderr, "                 when the file is not available, the default parameters are\n" );
    fprintf (   stderr, "                 \"0, 0, the maximum of win and wout\" for all pictures.\n" );
    fprintf (   stderr, "              t: number of temporal downsampling stages (default: 0)\n" );
    fprintf (   stderr, "           skip: number of frames to skip at start (default: 0)\n" );
    fprintf (   stderr, "           frms: number of maximum input frames (default: max)\n" );
    fprintf (   stderr, "\n" );
    fprintf (   stderr, "\n--------------------------- OR ---------------------------\n\n" );
    fprintf (   stderr, "\nUsage: %s -tmm <in> <win> <hin> <out> <wout> <hout> [<t> [<skip> [<frms>]]]\n\n", name );
    fprintf (   stderr, "\t   in: input file\n" );
    fprintf (   stderr, "\t  win: input width  (luma samples)\n" );
    fprintf (   stderr, "\t  hin: input height (luma samples)\n" );
    fprintf (   stderr, "\t  out: output file\n" );
    fprintf (   stderr, "\t wout: output width  (luma samples)\n" );
    fprintf (   stderr, "\t hout: output height (luma samples)\n" );
    fprintf (   stderr, "\t    t: number of temporal downsampling stages\n" );
    fprintf (   stderr, "\t skip: number of frames to skip at start (default: 0)\n" );
    fprintf (   stderr, "\t frms: number of maximum input frames (default: max)\n" );
    fprintf (   stderr, "\n" );
    fprintf (   stderr, "\n--------------------------- OR ---------------------------\n\n" );
    fprintf (   stderr, "\nUsage: %s -tmm2 <method> <in> <win> <hin> <out> <wout> <hout> [<t> [<skip> [<frms>]]]\n\n", name );
    fprintf (   stderr, "\tmethod: 1:lanczos, 2:1/2 pel + bilin 1/4 pel\n" );
    fprintf (   stderr, "\t   in: input file\n" );
    fprintf (   stderr, "\t  win: input width  (luma samples)\n" );
    fprintf (   stderr, "\t  hin: input height (luma samples)\n" );
    fprintf (   stderr, "\t  out: output file\n" );
    fprintf (   stderr, "\t wout: output width  (luma samples)\n" );
    fprintf (   stderr, "\t hout: output height (luma samples)\n" );
    fprintf (   stderr, "\t    t: number of temporal downsampling stages\n" );
    fprintf (   stderr, "\t skip: number of frames to skip at start (default: 0)\n" );
    fprintf (   stderr, "\t frms: number of maximum input frames (default: max)\n" );
    fprintf (   stderr, "\n" );
    exit    (   1 );
  }
}




int main(int argc, char *argv[])
{
  DownConvert   cDownConvert;
  
  //===== input parameters =====
  int   input_width         = 0;
  int   input_height        = 0;
  int   output_width        = 0;
  int   output_height       = 0;
  int   spatial_stages_down = 0;
  int   spatial_stages_up   = 0;
  int   temporal_stages     = 0;
  int   skip_at_start       = 0;
  int   number_frames       = (1<<30);
  FILE* input_file          = 0;
  FILE* output_file         = 0;
  FILE* crop_paras_file     = 0;
  int   aiFilter[16]        = { 0,0,1,0,-5,0,20,32,20,0,-5,0,1,0,0,64};
  int*  piFilter            = 0;
  
  int   do_tmm              = 0;
  int   method              = 1;

  int   do_ess              = 0;
  int   ess_mode            = 0;
  int   crop_x0             = 0;
  int   crop_y0             = 0;
  int   crop_w              = 0;
  int   crop_h              = 0;
  int   input_chroma_phase_shift_x = -1;
  int   input_chroma_phase_shift_y = -1;
  int   output_chroma_phase_shift_x = -1;
  int   output_chroma_phase_shift_y = -1;
  bool  ess_downsample_flg  = 1;
  
  //===== variables =====
  int           written, index, sidx, skip, skip_between, sequence_length;
  YuvFrame      cFrame;
  int frame_width;
  int frame_height;


  //===== read input parameters =====
  print_usage_and_exit (argc<=1, argv[0]);

  
  if (strcmp(argv[1], "-ess") == 0)  // ESS
    {
      print_usage_and_exit    ( (argc < 9 || argc > 17) || ( argc >= 10 && argc < 13 ), argv[0] );
      do_ess = 1;
      ess_mode            = atoi  ( argv[2] );
      input_file          = fopen ( argv[3], "rb" );
      input_width         = atoi  ( argv[4] );
      input_height        = atoi  ( argv[5] );
      output_file         = fopen ( argv[6], "wb" );
      output_width        = atoi  ( argv[7] );
      output_height       = atoi  ( argv[8] );
      if( argc >= 10 )
        {
          input_chroma_phase_shift_x = atoi ( argv[9] );
          input_chroma_phase_shift_y = atoi ( argv[10] );
          output_chroma_phase_shift_x = atoi ( argv[11] );
          output_chroma_phase_shift_y = atoi ( argv[12] );
        }
      if( argc >= 14 )
        {
          crop_paras_file   = fopen ( argv[13], "rb" );
          print_usage_and_exit( ! crop_paras_file,                                     argv[0], "Cannot open cropping-parameter file!" );
        }
      if( argc >= 15 )
        {
          temporal_stages   = atoi  ( argv[14] );
        }
      if( argc >= 16 )
        {
          skip_at_start     = atoi  ( argv[15] );
        }
      if( argc == 17 )
        {
          number_frames     = atoi  ( argv[16] );
        }
    }
  else if (strstr(argv[1], "-tmm") != argv[1]) // normal
    {
      print_usage_and_exit    ( (argc < 7 || argc > 9) && argc!=13, argv[0] );
      input_width         = atoi  ( argv[1] );
      input_height        = atoi  ( argv[2] );
      input_file          = fopen ( argv[3], "rb" );
      spatial_stages_down = atoi  ( argv[4] );
      temporal_stages     = atoi  ( argv[5] );
      output_file         = fopen ( argv[6], "wb" );
      if( argc >= 8 )
        {
          skip_at_start     = atoi  ( argv[7] );
        }
      if( argc >= 9 )
        {
          number_frames     = atoi  ( argv[8] );
        }

      if( spatial_stages_down < 0 )
        {
          spatial_stages_up   = -spatial_stages_down;
          spatial_stages_down = 0;
          output_width = input_width << spatial_stages_up;
          output_height = input_height << spatial_stages_up;
        }
      else
        {
          spatial_stages_up   = 0;
          output_width = input_width >> spatial_stages_down;
          output_height = input_height >> spatial_stages_down;
        }
      //===== check input parameters =====
      print_usage_and_exit( ! input_width  || (input_width %(2<<spatial_stages_down)), argv[0], "Unvalid input width or spatial stages!" );
      print_usage_and_exit( ! input_height || (input_height%(2<<spatial_stages_down)), argv[0], "Unvalid input height or spatial stages!" );
    }
  else // TMM TMM2
    {
      do_tmm = 1;
      int ind = 2;
      if (strcmp(argv[1], "-tmm2") == 0)
        {
          print_usage_and_exit(!((argc>=9) && (argc<=12)), argv[0]);
          method = atoi( argv[ind++] );
        }
      else
        {
          print_usage_and_exit(!((argc>=8) && (argc<=11)), argv[0]);
          method = 1;
        }
      input_file          = fopen ( argv[ind++], "rb" );
      input_width         = atoi  ( argv[ind++] );
      input_height        = atoi  ( argv[ind++] );
      output_file         = fopen ( argv[ind++], "wb" );
      output_width        = atoi  ( argv[ind++] );
      output_height       = atoi  ( argv[ind++] );
      if (argc > ind) temporal_stages = atoi  ( argv[ind++] );
      if (argc > ind) skip_at_start   = atoi  ( argv[ind++] );
      if (argc > ind) number_frames   = atoi  ( argv[ind++] );
      argc = 0;
    }

  if(input_width < output_width || input_height < output_height) ess_downsample_flg = 0;

  //===== check input parameters =====
  print_usage_and_exit( ! number_frames,                                           argv[0], "Unvalid input frames!" );
  print_usage_and_exit( ! input_file,                                              argv[0], "Cannot open input file!" );
  print_usage_and_exit( ! output_file,                                             argv[0], "Cannot open output file!" );


  if (do_ess)
    {
      print_usage_and_exit( ess_mode < 1 || ess_mode > 2, argv[0], "Invalid ess_mode!" );
      print_usage_and_exit( ! input_width || ! output_width, argv[0], "Invalid input or output width!" );
      print_usage_and_exit( ! input_height || ! output_height, argv[0], "Invalid input or output height!" );
      //print_usage_and_exit( ! ess_downsample_flg, argv[0], "no upsampling support so far." );
      print_usage_and_exit( input_height < output_height && input_height > output_height, argv[0], "no support for mixed down/up sampling." );
      print_usage_and_exit( input_height > output_height && input_height < output_height, argv[0], "no support for mixed down/up sampling." );
      print_usage_and_exit( input_chroma_phase_shift_x > 1 || input_chroma_phase_shift_x < (-1), argv[0], "Invalid input_chroma_phase_shift_x!" );
      print_usage_and_exit( input_chroma_phase_shift_y > 1 || input_chroma_phase_shift_y < (-1), argv[0], "Invalid input_chroma_phase_shift_y!" );
      print_usage_and_exit( output_chroma_phase_shift_x > 1 || output_chroma_phase_shift_x < (-1), argv[0], "Invalid output_chroma_phase_shift_x!" );
      print_usage_and_exit( output_chroma_phase_shift_y > 1 || output_chroma_phase_shift_y < (-1), argv[0], "Invalid output_chroma_phase_shift_y!" );
    }
  
  //===== set filter if given =====
  if ((!do_tmm && !do_ess) && ( argc > 9 ))
  {
    int a = atoi( argv[ 9] );
    int b = atoi( argv[10] );
    int c = atoi( argv[11] );
    int d = atoi( argv[12] );
    int e = 16 - a/2 - c;
    int f = 16 - b   - d;

    if( a%2 )
    {
      fprintf(stderr, "WRONG FILTER COEFFS\n");
      exit(1);
    }

    aiFilter[ 0] = 0;
    aiFilter[ 1] = 0;
    aiFilter[ 2] = f;
    aiFilter[ 3] = e;
    aiFilter[ 4] = d;
    aiFilter[ 5] = c;
    aiFilter[ 6] = b;
    aiFilter[ 7] = a;
    aiFilter[ 8] = b;
    aiFilter[ 9] = c;
    aiFilter[10] = d;
    aiFilter[11] = e;
    aiFilter[12] = f;
    aiFilter[13] = 0;
    aiFilter[14] = 0;
    aiFilter[15] = 64;
    piFilter     = &aiFilter[0];
  }

  //======= get number of input frames =======
  fseek(  input_file, 0, SEEK_END );
  sequence_length = (ftell(input_file)/(3*input_width*input_height/2));
  number_frames   = ( number_frames < sequence_length ? number_frames : sequence_length );
  fseek(  input_file, 0, SEEK_SET );
  skip_between    = ( 1 << temporal_stages ) - 1;

  
  //===== initialization ======
  frame_width = (input_width > output_width) ? input_width : output_width;
  frame_height = (input_height > output_height) ? input_height : output_height;

  frame_width *= 2;
  frame_height *= 2;
  createFrame( &cFrame, frame_width, frame_height );
  cDownConvert.init( frame_width, frame_height );

  long start_time = clock();

  //===== loop over frames =====
  for( skip = skip_at_start, index = 0, written = 0; index < number_frames; index++, skip = skip_between )
  {
    for( sidx = 0; sidx < skip && index < number_frames; sidx++, index++ )
    {
      readFrame       ( &cFrame, input_file, input_width, input_height );
    }
    if( index < number_frames )
    {
      clearFrame      ( &cFrame );
       
      if(ess_mode == 0)
      {
        readFrame       ( &cFrame, input_file,   input_width, input_height );
        resampleFrame   ( do_tmm, &cFrame, cDownConvert, input_width, input_height, output_width, output_height, piFilter, method );
        writeFrame      ( &cFrame, output_file,  output_width, output_height );
      }
      else
      {
        if (!crop_paras_file){
          crop_x0=crop_y0=0;
          crop_w=max(input_width, output_width);
        }
        else if(ess_mode==2) fscanf(crop_paras_file,"%d,%d,%d\n", &crop_x0, &crop_y0, &crop_w);
        else if(index == skip_at_start) fscanf(crop_paras_file,"%d,%d,%d\n", &crop_x0, &crop_y0, &crop_w);

        if( (crop_x0&1) || (crop_y0&1) || (crop_w&1) || crop_w<1 || crop_w>max(input_width, output_width) ){
          fprintf(stderr, "cropping parameters must be even value integers!\n");
          break;
        }


        //SSUN@SHARP crop_h = (crop_w*min(input_height, output_height)+min(input_width, output_width)/2)/min(input_width, output_width);
        //SSUN@SHARP crop_h = crop_h - (crop_h&1);  // must be even!

        ess_readFrame     ( &cFrame, input_file, input_width, input_height );
        ess_resampleFrame ( &cFrame, cDownConvert, input_width, input_height, output_width, output_height,
                            crop_x0, crop_y0, crop_w, crop_h, input_chroma_phase_shift_x, input_chroma_phase_shift_y,
                            output_chroma_phase_shift_x, output_chroma_phase_shift_y, ess_downsample_flg);
        ess_writeFrame    ( &cFrame, output_file, output_width, output_height );
      }


      fprintf( stderr, "\r%6d frames converted", ++written );
    }
  }
  long end_time = clock();
  

  //===== finish =====
  deleteFrame( &cFrame     );
  fclose     ( input_file  );
  fclose     ( output_file );
  if(crop_paras_file) fclose (crop_paras_file);


  fprintf(stderr, "\n" );
  double delta_in_s = (double)(end_time - start_time) / CLOCKS_PER_SEC;
  fprintf(stderr, "in %.2lf seconds => %.0lf ms/frame\n", delta_in_s, delta_in_s/written*1000);

  return 0;
}

