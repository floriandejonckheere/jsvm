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

void readColorComponent( ColorComponent* cc, FILE* file )
{
  unsigned int size   = cc->width*cc->height;
  unsigned int rsize;

  rsize = fread( cc->data, sizeof(unsigned char), size, file );

  if( size != rsize )
  {
    fprintf(stderr, "\nERROR: while reading from input file!\n\n");
    exit(1);
  }
}

void writeColorComponent( ColorComponent* cc, FILE* file, int downScale )
{
  int outwidth  = cc->width   >> downScale;
  int outheight = cc->height  >> downScale;
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

double psnr( ColorComponent& rec, ColorComponent& org, int rec_x, int rec_y )
{
  unsigned char*  pOrg  = org.data + (rec_y * org.width) + rec_x;
  unsigned char*  pRec  = rec.data;
  double          ssd   = 0;
  int             diff;

  for  ( int r = 0; r < rec.height; r++ )
  {
    for( int c = 0; c < rec.width;  c++ )
    {
      diff  = pRec[c] - pOrg[c];
      ssd  += (double)( diff * diff );
    }
    pRec   += rec.width;
    pOrg   += org.width;
  }

  return ( 10.0 * log10( (double)rec.width * (double)rec.height * 65025.0 / ssd ) );
}

void getPSNR( double& psnrY, double& psnrU, double& psnrV, YuvFrame& rcFrameOrg, YuvFrame& rcFrameRec, int rec_x, int rec_y )
{
  psnrY = psnr( rcFrameRec.lum, rcFrameOrg.lum, rec_x, rec_y );
  psnrU = psnr( rcFrameRec.cb,  rcFrameOrg.cb,  rec_x, rec_y );
  psnrV = psnr( rcFrameRec.cr,  rcFrameOrg.cr,  rec_x, rec_y );
}

void readFrame( YuvFrame* f, FILE* file )
{
  readColorComponent( &f->lum, file );
  readColorComponent( &f->cb,  file );
  readColorComponent( &f->cr,  file );
}

void writeFrame( YuvFrame* f, FILE* file, int downscale )
{
  writeColorComponent( &f->lum, file, downscale );
  writeColorComponent( &f->cb,  file, downscale );
  writeColorComponent( &f->cr,  file, downscale );
}

void downsampleFrame( YuvFrame* pcFrame, DownConvert& rcDownConvert, int iStages )
{
  rcDownConvert.downsample( pcFrame->lum.data,  pcFrame->lum.width,
                            pcFrame->cb .data,  pcFrame->cb .width,
                            pcFrame->cr .data,  pcFrame->cr .width,
                            pcFrame->lum.width, pcFrame->lum.height, iStages );
}

void print_usage_and_exit( int test, char* name, char* message = 0 )
{
  if( test )
  {
    if( message )
    {
      fprintf ( stderr, "\nERROR: %s\n", message );
    }
    fprintf (   stderr, "\nUsage: %s <w> <h> <org> <rec> [<s> [<t> [<skip> [<strm> <fps>]]]]\n\n", name );
    fprintf (   stderr, "\t    w: original width  (luma samples)\n" );
    fprintf (   stderr, "\t    h: original height (luma samples)\n" );
    fprintf (   stderr, "\t  org: original file\n" );
    fprintf (   stderr, "\t  rec: reconstructed file\n" );
    fprintf (   stderr, "\t    s: number of spatial  downsampling stages (default: 0)\n" );
    fprintf (   stderr, "\t    t: number of temporal downsampling stages (default: 0)\n" );
    fprintf (   stderr, "\t skip: number of frames to skip at start      (default: 0)\n" );
    fprintf (   stderr, "\t strm: coded stream\n" );
    fprintf (   stderr, "\t frms: frames per second\n" );
    fprintf (   stderr, "\n" );
    fprintf (   stderr, "\nUsage: %s -tmm <orig> <w_orig> <h_orig> <rec> <w_rec> <h_rec> <x_rec> <y_rec> [<t> [<skip> [<strm> <fps>]]]\n\n", name );
    exit    (   1 );
  }
}





int main(int argc, char *argv[])
{
  int     acc = 10000;
#define   OUT "%d,%04d"

  //===== input parameters =====
  int           stream          = 0;
  unsigned int  org_width       = 0;
  unsigned int  org_height      = 0;
  unsigned int  rec_width       = 0;
  unsigned int  rec_height      = 0;
  unsigned int  rec_x           = 0;
  unsigned int  rec_y           = 0;
  unsigned int  spatial_stages  = 0;
  unsigned int  temporal_stages = 0;
  unsigned int  skip_at_start   = 0;
  double        fps             = 0.0;
  FILE*         org_file        = 0;
  FILE*         rec_file        = 0;
  FILE*         str_file        = 0;

  //===== variables =====
  unsigned int  index, sidx, skip, skip_between, sequence_length;
  int           py, pu, pv, br;
  double        bitrate, psnrY, psnrU, psnrV;
  YuvFrame      cOrgFrame, cRecFrame;
  DownConvert   cDownConvert;
  double        AveragePSNR_Y = 0.0;
  double        AveragePSNR_U = 0.0;
  double        AveragePSNR_V = 0.0;


  //===== read input parameters =====
  print_usage_and_exit(argc < 5, argv[0]);
  if( strcmp(argv[1], "-tmm") != 0 )
    {
      print_usage_and_exit      ( argc > 10 || argc == 9, argv[0] );
      org_width         = atoi  ( argv[1] );
      org_height        = atoi  ( argv[2] );
      org_file          = fopen ( argv[3], "rb" );
      rec_file          = fopen ( argv[4], "rb" );
      if( argc >=  6 )
        {
          spatial_stages  = atoi  ( argv[5] );
        }
      if( argc >=  7 )
        {
          temporal_stages = atoi  ( argv[6] );
        }
      if( argc >=  8 )
        {
          skip_at_start   = atoi  ( argv[7] );
        }
      if( argc >= 10 )
        {
          str_file        = fopen ( argv[8], "rb" );
          fps             = atof  ( argv[9] );
          stream          = 1;
        }
      rec_width       = org_width  >> spatial_stages;
      rec_height      = org_height >> spatial_stages;
    }
 else
   {
      print_usage_and_exit      ( argc < 10, argv[0] );
      int ind = 2;
      org_file          = fopen ( argv[ind++], "rb" );
      org_width         = atoi  ( argv[ind++] );
      org_height        = atoi  ( argv[ind++] );
      rec_file          = fopen ( argv[ind++], "rb" );
      rec_width         = atoi  ( argv[ind++] );
      rec_height        = atoi  ( argv[ind++] );
      rec_x             = atoi  ( argv[ind++] );
      rec_y             = atoi  ( argv[ind++] );
      spatial_stages    = 0;
      if( argc >=  11 )
        {
          temporal_stages = atoi  ( argv[ind++] );
        }
      if( argc >=  12 )
        {
          skip_at_start   = atoi  ( argv[ind++] );
        }
      if( argc >= 14 )
        {
          str_file        = fopen ( argv[ind++], "rb" );
          fps             = atof  ( argv[ind++] );
          stream          = 1;
        }
   }

  //===== check input parameters =====
  print_usage_and_exit  ( ! org_width  || (org_width %(2<<spatial_stages)), argv[0], "Unvalid input width or spatial stages!" );
  print_usage_and_exit  ( ! org_height || (org_height%(2<<spatial_stages)), argv[0], "Unvalid input height or spatial stages!" );
  print_usage_and_exit  ( ! org_file,                                       argv[0], "Cannot open original file!" );
  print_usage_and_exit  ( ! rec_file,                                       argv[0], "Cannot open reconstructed file!" );
  print_usage_and_exit  ( ! str_file && stream,                             argv[0], "Cannot open stream!" );
  print_usage_and_exit  ( fps <= 0.0 && stream,                             argv[0], "Unvalid frames per second!" );

  //======= get number of frames and stream size =======
  fseek(    rec_file, 0, SEEK_END );
  fseek(    org_file, 0, SEEK_END );
  size_t rsize = ftell( rec_file );
  size_t osize = ftell( org_file );
  fseek(    rec_file, 0, SEEK_SET );
  fseek(    org_file, 0, SEEK_SET );
  if (rsize < osize)
    sequence_length = rsize*4/(6*rec_width)/rec_height;
  else
    sequence_length = osize*4/(6*org_width)/org_height;

  if( stream )
  {
    fseek(  str_file, 0, SEEK_END );
    bitrate       = (double)ftell(str_file) * 8.0 / 1000.0 / ( (double)(sequence_length << temporal_stages) / fps );
    fseek(  str_file, 0, SEEK_SET );
  }
  skip_between    = ( 1 << temporal_stages ) - 1;

  //===== initialization ======
  createFrame( &cOrgFrame, org_width, org_height );
  createFrame( &cRecFrame, rec_width, rec_height );
  cDownConvert.init(       org_width, org_height );


  //===== loop over frames =====
  for( skip = skip_at_start, index = 0; index < sequence_length; index++, skip = skip_between )
  {
    for( sidx = 0; sidx < skip; sidx++ )
    {
      readFrame     ( &cOrgFrame, org_file );
    }
    readFrame       ( &cOrgFrame, org_file );
    downsampleFrame ( &cOrgFrame, cDownConvert, spatial_stages );
    readFrame       ( &cRecFrame, rec_file );

    getPSNR         ( psnrY, psnrU, psnrV, cOrgFrame, cRecFrame, rec_x, rec_y );
    AveragePSNR_Y +=  psnrY;
    AveragePSNR_U +=  psnrU;
    AveragePSNR_V +=  psnrV;

    py = (int)floor( acc * psnrY + 0.5 );
    pu = (int)floor( acc * psnrU + 0.5 );
    pv = (int)floor( acc * psnrV + 0.5 );
    fprintf(stdout,"%d\t"OUT"\t"OUT"\t"OUT"\n",index,py/acc,py%acc,pu/acc,pu%acc,pv/acc,pv%acc);
  }
  fprintf(stdout,"\n");

  py = (int)floor( acc * AveragePSNR_Y / (double)sequence_length + 0.5 );
  pu = (int)floor( acc * AveragePSNR_U / (double)sequence_length + 0.5 );
  pv = (int)floor( acc * AveragePSNR_V / (double)sequence_length + 0.5 );
  br = (int)floor( acc * bitrate                                 + 0.5 );
  if( stream )
  {
    fprintf(stderr,OUT"\t"OUT"\t"OUT"\t"OUT"\n",br/acc,br%acc,py/acc,py%acc,pu/acc,pu%acc,pv/acc,pv%acc);
  }
  else
  {
    fprintf(stderr,"total\t"OUT"\t"OUT"\t"OUT"\n",py/acc,py%acc,pu/acc,pu%acc,pv/acc,pv%acc);
  }
  fprintf(stdout, "\n");


  //===== finish =====
  deleteFrame( &cOrgFrame  );
  deleteFrame( &cRecFrame  );
  fclose     ( org_file    );
  fclose     ( rec_file    );
  if( stream )
  {
    fclose   ( str_file    );
  }

  return 0;
}

