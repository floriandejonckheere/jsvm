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

#define NO_MB_DATA_CTRL
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

void readColorComponent( ColorComponent* cc, FILE* file, int upscale )
{
  int inwidth   = ( cc->width  >> upscale );
  int inheight  = ( cc->height >> upscale );
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

void writeColorComponent( ColorComponent* cc, FILE* file, int downScale )
{
  int outwidth  = cc->width  >> downScale;
  int outheight = cc->height >> downScale; 
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


void readFrame( YuvFrame* f, FILE* file, int upscale )
{
  readColorComponent( &f->lum, file, upscale );
  readColorComponent( &f->cb,  file, upscale );
  readColorComponent( &f->cr,  file, upscale );
}

void writeFrame( YuvFrame* f, FILE* file, int downscale )
{
  writeColorComponent( &f->lum, file, downscale );
  writeColorComponent( &f->cb,  file, downscale );
  writeColorComponent( &f->cr,  file, downscale );
}

void resampleFrame( YuvFrame* pcFrame, DownConvert& rcDownConvert, int iStagesUp, int iStagesDown, int* piFilter ) 
{
  if( iStagesUp )
  {
    if( piFilter )
    {
      rcDownConvert.upsample  ( pcFrame->lum.data,   pcFrame->lum.width,
                                pcFrame->cb .data,   pcFrame->cb .width,
                                pcFrame->cr .data,   pcFrame->cr .width,
                                pcFrame->lum.width/(1<<iStagesUp),pcFrame->lum.height/(1<<iStagesUp), piFilter, iStagesUp );
      return;
    }

    rcDownConvert.upsample  ( pcFrame->lum.data,   pcFrame->lum.width,
                              pcFrame->cb .data,   pcFrame->cb .width,
                              pcFrame->cr .data,   pcFrame->cr .width,
                              pcFrame->lum.width/(1<<iStagesUp),pcFrame->lum.height/(1<<iStagesUp), iStagesUp );
  }
  else
  {
    rcDownConvert.downsample( pcFrame->lum.data,  pcFrame->lum.width,
                              pcFrame->cb .data,  pcFrame->cb .width,
                              pcFrame->cr .data,  pcFrame->cr .width,
                              pcFrame->lum.width, pcFrame->lum.height, iStagesDown );
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
    exit    (   1 );
  }
}





int main(int argc, char *argv[])
{
  DownConvert   cDownConvert;
  
  //===== input parameters =====
  int   input_width         = 0;
  int   input_height        = 0;
  int   spatial_stages_down = 0;
  int   spatial_stages_up   = 0;
  int   temporal_stages     = 0;
  int   skip_at_start       = 0;
  int   number_frames       = (1<<30);
  FILE* input_file          = 0;
  FILE* output_file         = 0;
  int   aiFilter[16]        = { 0,0,1,0,-5,0,20,32,20,0,-5,0,1,0,0,64};
  int*  piFilter            = 0;
  
  //===== variables =====
  int           written, index, sidx, skip, skip_between, sequence_length;
  YuvFrame      cFrame;


  //===== read input parameters =====
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
  }
  else
  {
    spatial_stages_up   = 0;
  }

  //===== check input parameters =====
  print_usage_and_exit( ! input_width  || (input_width %(2<<spatial_stages_down)), argv[0], "Unvalid input width or spatial stages!" );
  print_usage_and_exit( ! input_height || (input_height%(2<<spatial_stages_down)), argv[0], "Unvalid input height or spatial stages!" );
  print_usage_and_exit( ! number_frames,                                           argv[0], "Unvalid input frames!" );
  print_usage_and_exit( ! input_file,                                              argv[0], "Cannot open input file!" );
  print_usage_and_exit( ! output_file,                                             argv[0], "Cannot open output file!" );

  //===== set filter if given =====
  if( argc > 9 )
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
  sequence_length = ftell(input_file)*4/(6*input_width*input_height);
  number_frames   = ( number_frames < sequence_length ? number_frames : sequence_length );
  fseek(  input_file, 0, SEEK_SET );
  skip_between    = ( 1 << temporal_stages ) - 1;

  input_width   <<= spatial_stages_up;
  input_height  <<= spatial_stages_up;

  //===== initialization ======
  createFrame( &cFrame, input_width, input_height );
  cDownConvert.init(    input_width, input_height );

  //===== loop over frames =====
  for( skip = skip_at_start, index = 0, written = 0; index < number_frames; index++, skip = skip_between )
  {
    for( sidx = 0; sidx < skip && index < number_frames; sidx++, index++ )
    {
      readFrame       ( &cFrame, input_file, spatial_stages_up );
    }
    if( index < number_frames )
    {
      readFrame       ( &cFrame, input_file,   spatial_stages_up );
      resampleFrame   ( &cFrame, cDownConvert, spatial_stages_up, spatial_stages_down, piFilter );
      writeFrame      ( &cFrame, output_file,  spatial_stages_down );
      fprintf( stderr, "\r%6d frames converted", ++written );
    }
  }


  //===== finish =====
  deleteFrame( &cFrame     );
  fclose     ( input_file  );
  fclose     ( output_file );


  fprintf(stderr, "\n" );

  return 0;
}

