/*
********************************************************************************

NOTE - One of the two copyright statements below may be chosen
       that applies for the software.

********************************************************************************
This software module was originally developed by

CAMMAS Nathalie (France Télécom)

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

To the extent that France Télécom owns patent rights that would be required to
make, use, or sell the originally developed software module or portions thereof
included in the ISO/IEC 14496-10:2005 Amd.1 (Scalable Video Coding) in a
conforming product, France Télécom will assure the ISO/IEC that it is willing
to negotiate licenses under reasonable and non-discriminatory terms and
conditions with applicants throughout the world.

France Télécom retains full right to modify and use the code for its own
purpose, assign or donate the code to a third party and to inhibit third
parties from using the code for products that do not conform to MPEG-related
ITU Recommendations and/or ISO/IEC International Standards.

This copyright notice must be included in all copies or derivative works.
Copyright (c) ISO/IEC 2005.

********************************************************************************

COPYRIGHT AND WARRANTY INFORMATION

Copyright 2005, International Telecommunications Union, Geneva

The France Télécom hereby donate this source code to the ITU, with the following
understanding:
    1. France Télécom retain the right to do whatever they wish with the
       contributed source code, without limit.
    2. France Télécom retain full patent rights (if any exist) in the technical
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
#include "EncoderMergerParameter.h"

#ifndef MSYS_WIN32
#define equal(a,b)  (!strcasecmp((a),(b)))
#else
#define equal(a,b)  (!stricmp((a),(b)))
#endif


void print_usage_and_exit_1( int test, char* name, char* message = 0 )
{
  if( test )
  {
    if( message )
    {
      fprintf ( stderr, "\nERROR: %s\n", message );
    }
    fprintf (   stderr, "\nUsage: %s <w> <h> <orig> <root> <disto> <nf> <gs> <fgs> <l> \n\n", name );
	  fprintf (   stderr, "\t  w: width\n" );
	  fprintf (   stderr, "\t  h: height\n" );
    fprintf (   stderr, "\t  orig: orig yuv file\n" );
	  fprintf (   stderr, "\t  root: root extracted yuv files\n" );
	  fprintf (   stderr, "\t  disto: disto file\n" );
	  fprintf (   stderr, "\t   nf: number of frames\n" );
    fprintf (   stderr, "\t   gs: gop size\n" );
	  fprintf (   stderr, "\t  fgs: number of FGS layer\n" );
	  fprintf (   stderr, "\t   l: layer\n" );
	  fprintf (   stderr, "\n" );
    exit    (   1 );
  }
}

void print_usage_and_exit_2( int test, char* name, char* message = 0 )
{
  if( test )
  {
    if( message )
    {
      fprintf ( stderr, "\nERROR: %s\n", message );
    }
    fprintf (   stderr, "\nUsage: %s <numl> <in> <out> <disto> <rate> <nf> <gs> <fgs> <SEIm>\n\n", name );
	  fprintf (   stderr, "\t   numl: number of layer\n" );
	  fprintf (   stderr, "\t   in: input file\n" );
	  fprintf (   stderr, "\t  out: output file generated\n" );
	  fprintf (   stderr, "\t  disto: disto file \n" );
	  fprintf (   stderr, "\t  rate: rate file \n" );
    fprintf (   stderr, "\t   nf: number of frames\n" );
    fprintf (   stderr, "\t   gs: gop size\n" );
	  fprintf (   stderr, "\t  fgs: number of FGS layer\n" );
	  fprintf (   stderr, "\t  SEIm: QL in SEI message (1) or priority_id (0) \n" );
	  fprintf (   stderr, "\n" );
    exit    (   1 );
  }
}

EncoderMergerParameter::EncoderMergerParameter()
{
}

EncoderMergerParameter::~EncoderMergerParameter()
{
}

ErrVal EncoderMergerParameter::create( EncoderMergerParameter*& rpcEncoderMergerParameter )
{
  rpcEncoderMergerParameter = new EncoderMergerParameter;
  
  ROT( NULL == rpcEncoderMergerParameter );
 
  return Err::m_nOK;
}
ErrVal EncoderMergerParameter::destroy()
{
  delete this;
  return Err::m_nOK;
}

void EncoderMergerParameter::init(Int argc, char **argv)
{
	EncoderIoParameter m_cEncoderIoParameter;
	Char*   pcBitstreamFile = m_cEncoderIoParameter.pBitstreamFile;
	pcBitstreamFile [0] = (Char) 0;
	char fileName[256];
	m_uiMode = atoi( argv[ 1 ] );
	if(m_uiMode == 0)
	{
		print_usage_and_exit_1( argc<11 , argv[0]);
		m_uiWidth = atoi(argv[2]);
		m_uiHeight = atoi(argv[3]);
		strcpy( fileName, argv[4]);
		m_cOrig = fileName;
		strcpy( fileName, argv[5]);
		m_cRoot = fileName;
		strcpy( fileName, argv[6]);
		m_cDistoFilename = fileName;
		m_uiNumOfFrames = atoi(argv[7]);
		m_uiGopSize = atoi(argv[8]);
		m_uiMaxFGS = atoi(argv[9]);
		m_uiLayer = atoi(argv[10]);
	}
	if(m_uiMode == 1)
	{
		print_usage_and_exit_2( argc<11 , argv[0]);
        m_uiNumOfLayer       = atoi( argv[ 2 ] )-1;
		CodingParameter::setNumberOfLayers(m_uiNumOfLayer);
		strcpy( fileName, argv[3]);
		m_acInFile = fileName;
		strcpy( fileName, argv[4]);
		m_cOutFile = fileName;
		strcpy( fileName, argv[5]);
		m_cDistoFilename = fileName;
		strcpy( fileName, argv[6]);
		m_cFGSRateFilename = fileName;
		m_uiNumOfFrames = atoi(argv[7]);
		m_uiGopSize = atoi(argv[8]);
		m_uiMaxFGS = atoi(argv[9]);
		m_uiQLInSEI = atoi(argv[10]);
    CodingParameter::setNumberOfLayers(m_uiNumOfLayer+1);
	}
        
}


