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




#include "BStreamExtractor.h"
#include "ExtractorParameter.h"


#ifndef MSYS_WIN32
#define stricmp strcasecmp
#endif

#define equal(a,b)  (!stricmp((a),(b)))




ExtractorParameter::ExtractorParameter()
: m_cInFile       ()
, m_cOutFile      ()
, m_iResult       ( -10 )
, m_uiLayer       ( MSYS_UINT_MAX )
, m_uiLevel       ( MSYS_UINT_MAX )
, m_dFGSLayer     ( 10.0 )
, m_dBitrate			( MSYS_UINT_MAX )
, m_bAnalysisOnly ( true )
// HS: packet trace
, m_bTraceFile    ( false )
, m_bTraceExtract ( false )
, m_cTraceFile    ()
, m_cExtractTrace ()
{
    //{{Quality level estimation and modified truncation- JVTO044 and m12007
    //France Telecom R&D-(nathalie.cammas@francetelecom.com)
    for(UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++)
    {
	    m_bExtractDeadSubstream[uiLayer] = false;
    }
    //}}Quality level estimation and modified truncation- JVTO044 and m12007
}



ExtractorParameter::~ExtractorParameter()
{
}



ErrVal
ExtractorParameter::xParseFormatString( Char*   pFormatString,
                                        Point&  rcPoint )
{
  Char  acSearch  [4] = "x@:";
  Char* pSubString[4] = { 0, 0, 0, 0 };
  UInt  uiPos         = 0;

  //===== set sub-strings =====
  for( UInt uiIndex = 0; uiIndex < 3; uiIndex++ )
  {
    while( pFormatString[uiPos] != '\0' )
    {
      if ( pFormatString[uiPos++] == acSearch[uiIndex] )
      {
        pFormatString [uiPos-1] =  '\0';
        pSubString    [uiIndex] =  pFormatString;
        pFormatString           = &pFormatString[uiPos];
        uiPos                   =  0;
        break;
      }
    }
  }
  pSubString[3] = pFormatString;

  ROFS( pSubString[0] );
  ROFS( pSubString[1] );
  ROFS( pSubString[2] );
  ROFS( pSubString[3] );

  rcPoint.uiWidth    = atoi( pSubString[0] );
  rcPoint.uiHeight   = atoi( pSubString[1] );
  rcPoint.dFrameRate = atof( pSubString[2] );
  rcPoint.dBitRate   = atof( pSubString[3] );

  ROFS( rcPoint.uiWidth    > 0   );
  ROFS( rcPoint.uiHeight   > 0   );
  ROFS( rcPoint.dFrameRate > 0.0 );
  ROFS( rcPoint.dBitRate   > 0.0 );

  return Err::m_nOK;
}

ErrVal
ExtractorParameter::init( Int     argc,
                          Char**  argv )	
{
  m_cExtractionList.clear();
  m_uiLayer = MSYS_UINT_MAX;
  m_uiLevel = MSYS_UINT_MAX;

  Bool  bLayerSpecified           = false;
  Bool  bLevelSpecified           = false;
  Bool  bFGSSpecified             = false;
	Bool	bBitrateSpecified					= false;
  Point cPoint;

#define EXIT(x,m) {if(x){printf("\n%s\n",m);RNOKS(xPrintUsage(argv))}}

  //===== get file names and set parameter "AnalysisOnly" =====
  EXIT( argc < 2, "No arguments specified" );
  m_iResult       = 0;
  m_bAnalysisOnly = ( argc == 2 ? true : false );
  m_cInFile       = argv[1];
  ROTRS( m_bAnalysisOnly, Err::m_nOK );
  m_cOutFile      = argv[2];

  //===== process arguments =====
  for( Int iArg = 3; iArg < argc; iArg++ )
  {
    if( equal( "-l", argv[iArg] ) )
    {
      EXIT( iArg + 1 == argc,           "Option \"-l\" without argument specified" );
      EXIT( bLayerSpecified,            "Multiple options \"-l\"" );
			EXIT( bLevelSpecified,						"Option \"-l\" used in connection with option \"-t\"" );
			EXIT( bFGSSpecified,							"Option \"-l\" used in connection with option \"-f\"" );
			EXIT( bBitrateSpecified,					"Option \"-l\" used in connection with option \"-b\"" );
      m_uiLayer       = atoi( argv[ ++iArg ] );
      bLayerSpecified = true;
      continue;
    }

    if( equal( "-t", argv[iArg] ) )
    {
      EXIT( iArg + 1 == argc,           "Option \"-t\" without argument specified" );
      EXIT( bLevelSpecified,            "Multiple options \"-t\"" );
			EXIT( bLayerSpecified,						"Option \"-l\" used in connection with option \"-l\"" );
			EXIT( bBitrateSpecified,					"Option \"-l\" used in connection with option \"-b\"" );
      m_uiLevel       = atoi( argv[ ++iArg ] );
      bLevelSpecified = true;
      continue;
    }

    if( equal( "-f", argv[iArg] ) )
    {
      EXIT( iArg + 1 == argc,           "Option \"-f\" without argument specified" );
      EXIT( bFGSSpecified,              "Multiple options \"-f\"" );
			EXIT( bLayerSpecified,						"Option \"-l\" used in connection with option \"-l\"" );
			EXIT( bFGSSpecified,							"Option \"-l\" used in connection with option \"-b\"" );
      m_dFGSLayer     = atof( argv[ ++iArg ] );
      bFGSSpecified   = true;
      continue;
    }

		if( equal( "-b", argv[iArg] ) )
		{
			EXIT( iArg + 1 == argc,						"Option \"-b\" without argument specified" );
			EXIT( bBitrateSpecified,					"Multiple options \"-b\"" );
			EXIT( bLayerSpecified,						"Option \"-l\" used in connection with option \"-l\"" );
			EXIT( bLevelSpecified,						"Option \"-l\" used in connection with option \"-t\"" );
			EXIT( bFGSSpecified,							"Option \"-l\" used in connection with option \"-f\"" );
			m_dBitrate				= atof( argv[ ++iArg ] );
			bBitrateSpecified = true;
			continue;
		}

#if NON_REQUIRED_SEI_ENABLE   //shenqiu 05-10-09
		if (equal( "-enp",argv[iArg] ))  //extract non-required pictures 
		{
			EXIT( iArg + 1 == argc,			 "Option \"-enp\" without argument specified" );
			m_uiExtractNonRequiredPics = atoi(argv[++iArg]);
			continue;
		}
#endif
    EXIT( true, "Unknown option specified" );
  }
  return Err::m_nOK;
#undef EXIT
}



ErrVal
ExtractorParameter::xPrintUsage( Char **argv )
{
  printf("\nUsage: %s InputStream [OutputStream [-l] | [-t] [-f] | [-b]]", argv[0] );
  printf("\noptions:\n");
  printf("\t-l L       -> extract specific scalable layer ID = L\n");
  printf("\t-t T       -> extract all temporal levels <= T\n");
  printf("\t-f F       -> extract all FGS layers <= F (0 - base layer only)\n");
  printf("\t-b B       -> extract all scalable layers with the biggest bitrate <= B\n");
  printf("\n");
  ROTS(1);
}
