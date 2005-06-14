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

#define equal(a,b)  (!stricmp((a),(b)))

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
	m_bDS = false;
	m_bNivQ = false;
	Int iNbInfile = 0;

	static int indexLayer = 0;
	for( Int iArg = 0; iArg < argc; iArg++ )
	{
		
	if( equal( "-numl", argv[iArg] ) )
    {
	  m_uiNumOfLayer       = atoi( argv[ ++iArg ] );
      CodingParameter::setNumberOfLayers(m_uiNumOfLayer);
	}
	if( equal("-infile", argv[iArg]))
	{
		int iLayer = atoi( argv[ ++iArg ] );
		char fileName[256];
        strcpy( fileName, argv[iArg+1]);
		m_acInFile[iLayer] = fileName;
		iNbInfile++;
	}
	if(equal("-ds",argv[iArg]))
	{
		m_bDS = true;
	}
	if(equal("-ql",argv[iArg]))
	{
		m_bNivQ = true;
		iArg++;
		for(Int i = 0; i < m_uiNumOfLayer; i++)
		{
		  char layer[256];
		  //set rd files name
		 char distoFileName[256], rateFileName[256];
         strcpy( distoFileName, argv[iArg]);
		 strcpy( rateFileName, argv[iArg]);
		 strcat( distoFileName, "_disto" );
		 strcat( rateFileName, "_rate" );
		 sprintf(layer,"%d",i); //use of sprintf rather than itoa for portability
		strcat( distoFileName, layer );
		strcat( rateFileName, layer );
		
		m_cDistoFilename[i] = distoFileName;
		m_cFGSRateFilename[i] = rateFileName;
		}
	}

    if(equal("-outfile",argv[iArg]))
    {
        iArg++;
        m_cOutFile = argv[iArg];
    }
    if(equal("-gopsize",argv[iArg]))
    {
        iArg++;
        UInt uiLayer = atoi(argv[iArg]);
        iArg++;
        UInt gop = atoi(argv[iArg]);
        m_uiGOPSize[uiLayer] = atoi(argv[iArg]);
    }

	}
	
	//set output file name
	if(m_bNivQ && m_bDS)
	{
		printf("Error: Quality Levels and Dead Substreams can not be added at the same time\n");
		exit(1);
	}
	if(m_bDS && (iNbInfile!=m_uiNumOfLayer))
	{
		printf("Error: Dead Substream: number of input files not enough compared to number of layer\n");
		exit(1);
	}

}


