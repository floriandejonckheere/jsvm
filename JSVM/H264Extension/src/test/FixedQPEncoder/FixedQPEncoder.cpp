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
#include <string>


#define MAX_LAYERS  10
#define USAGE(X)    {if(X) print_usage();}
#define MIN_QP       0.0
#define MAX_QP      51.0
#define DELTA_QP     6.0
#define MIN_DQP      1.0
#define MAX_NUM_ITER  15

void
print_usage()
{
  printf("\nusage:\n\n");
  printf("FixedQPEncoder <ConfigFile>\n");
  printf("\n");
  exit(1);
}



typedef struct
{
  double        dTargetRate;                      // set from config file (never changed)
  double        dMinMismatch;                     // set from config file (never changed)
  double        dMaxMismatch;                     // set from config file (never changed)
  double        dStartBaseQpResidual;             // set from config file (never changed)
	double        dStartQpModeDecision;             // set from config file (never changed)

	double        dQpModeDecision;                  // needed for encode
  double        dBaseQpResidual;                  // needed for encode
  unsigned int  uiMotionFileMode;                 // needed for encode
  std::string   cMotionFile;                      // needed for encode

  double        dRate;

  unsigned int  uiNumIter;
}
LayerParameters;



typedef struct
{
  std::string     cLabel;                         //                   (never changed) - config file
  std::string     cEncoderBinary;                 // needed for encode (never changed) - config file
  std::string     cParameterFile;                 // needed for encode (never changed) - config file
  std::string     cBitStreamFile;                 // needed for encode (never changed) - config file
  std::string     cMotionFolder;                  // needed for encode (never changed) - config file
  unsigned int    uiNumberOfLayers;               // needed for encode                 - config file
  unsigned int    uiNumberOfFrames;               //                   (never changed) - config file
  double          dFramesPerSecond;               //                   (never changed) - config file
  unsigned int    uiConstrainedIntraBL;
  LayerParameters acLayerParameters[MAX_LAYERS];  // needed for encode                 - config file
}
EncoderParameters;







//////////////////////////////////////////////////////////////////////////
//
//   encode once and return the file size
//
//////////////////////////////////////////////////////////////////////////
unsigned int
encode( EncoderParameters& rcEncoderParameters )
{
  char        acTempString[1024];
  std::string cCommandLineString;
	std::string cEchoLineString = "echo ";

  //----- general settings -----
  cCommandLineString    += rcEncoderParameters.cEncoderBinary;
  cCommandLineString    += " -pf ";
  cCommandLineString    += rcEncoderParameters.cParameterFile;
  cCommandLineString    += " -bf ";
  cCommandLineString    += rcEncoderParameters.cBitStreamFile;
  cCommandLineString    += " -numl ";
  sprintf( acTempString, "%d", rcEncoderParameters.uiNumberOfLayers );
  cCommandLineString    += acTempString;
  if( rcEncoderParameters.uiConstrainedIntraBL )
  {
    cCommandLineString  += " -bcip";
  }
	cCommandLineString    += " -frms ";
	sprintf( acTempString, "%d", rcEncoderParameters.uiNumberOfFrames );
	cCommandLineString    += acTempString;

  //----- layer settings -----
  for( unsigned int uiLayer = 0; uiLayer < rcEncoderParameters.uiNumberOfLayers; uiLayer++ )
  {
    LayerParameters& rcLayer = rcEncoderParameters.acLayerParameters[uiLayer];
    sprintf( acTempString, " -lqp %d %lf -rqp %d %lf -mfile %d %d %s ",
      uiLayer, rcLayer.dQpModeDecision, uiLayer, rcLayer.dBaseQpResidual,
      uiLayer, rcLayer.uiMotionFileMode, rcLayer.cMotionFile.c_str() );
    cCommandLineString  += acTempString;
  }

	cEchoLineString += cCommandLineString;
	cEchoLineString += "\n";
	system( cEchoLineString.c_str() );

  //----- run encoder -----
  int iResult = system( cCommandLineString.c_str() );
  if( iResult )
  {
    printf("\n\nERROR while executing \"...\"\n\n%s\n\n", cCommandLineString.c_str() );
    exit(2);
  }

  //----- get file size -----
  unsigned int uiFileSize = 0;
  FILE* pFile = fopen( rcEncoderParameters.cBitStreamFile.c_str(), "rb" );
  if( ! pFile )
  {
    printf( "\n\nUNEXPECTED ERROR: Cannot open file\"%s\"\n\n", rcEncoderParameters.cBitStreamFile.c_str() );
    exit(1);
  }
  fseek( pFile, 0, SEEK_END );
  uiFileSize += ftell( pFile );
  fclose( pFile );

  return uiFileSize;
}

//////////////////////////////////////////////////////////////////////////
//
//   encode layer until rate is reached and return file size
//
//////////////////////////////////////////////////////////////////////////
void
encode_layer( EncoderParameters& rcEncoderParameters )
{
  //----- initialize -----
  char              acTempString[1024];
  unsigned int      uiLayer          = rcEncoderParameters.uiNumberOfLayers - 1;
  LayerParameters&  rcLayer          = rcEncoderParameters.acLayerParameters[uiLayer];
  double            dSeqLength       = (double)rcEncoderParameters.uiNumberOfFrames / rcEncoderParameters.dFramesPerSecond;
  double            dTargetSize      = 1000.0 / 8.0 * rcLayer.dTargetRate * dSeqLength;
  unsigned int      uiMinTarget      = (unsigned int)(int)ceil ( (100.0 - rcLayer.dMinMismatch) / 100.0 * dTargetSize );
  unsigned int      uiMaxTarget      = (unsigned int)(int)floor( (100.0 + rcLayer.dMaxMismatch) / 100.0 * dTargetSize );
  unsigned int      uiTargetSize     = ( uiMinTarget + uiMaxTarget + 1 ) / 2;
  unsigned int      uiMinSize        = 0;
  unsigned int      uiMaxSize        = (unsigned int)-1;
  double            dMinSizeQP       =  1000;
  double            dMaxSizeQP       = -1000;
  int               iMinSizeQP       =  1000;
  int               iMaxSizeQP       = -1000;
  double            dCurrQP          = rcLayer.dStartBaseQpResidual;
 
  //----- run until rate matches -----
	rcLayer.uiNumIter = 0;
	rcLayer.dBaseQpResidual = rcLayer.dStartBaseQpResidual;
	rcLayer.dQpModeDecision = rcLayer.dStartQpModeDecision;
  while( true )
  {
    //----- set parameters -----
    const int iCurrQP       = (int)floor( dCurrQP + 0.5 );
    rcLayer.dBaseQpResidual = dCurrQP;
		if( (rcLayer.dStartBaseQpResidual == rcLayer.dStartQpModeDecision) && (rcLayer.uiNumIter < 5) )
		{
			// update QpModeDecision
			rcLayer.dQpModeDecision = rcLayer.dBaseQpResidual;
		}

		printf( "\n##################################" );
		printf( "\n### Layer %u - iteration %u      ###", uiLayer, rcLayer.uiNumIter );
		printf( "\n### BaseQpResidual = %lf ###", rcLayer.dBaseQpResidual );
		printf( "\n### QpModeDecision = %lf ###", rcLayer.dQpModeDecision );
		printf( "\n##################################\n" );

		sprintf( acTempString, "%s\\%s_layer%d.mot",
      rcEncoderParameters.cMotionFolder.c_str(),
      rcEncoderParameters.cLabel.c_str(),
      uiLayer );
    rcLayer.cMotionFile         = acTempString;
      rcLayer.uiMotionFileMode = 2;
		rcLayer.uiNumIter++;

    //--- run ---
    unsigned int uiCurrSize = encode( rcEncoderParameters );

    //--- check ---
    if( ( uiCurrSize >= uiMinTarget && uiCurrSize <= uiMaxTarget ) || dCurrQP >= MAX_QP || dCurrQP <= MIN_QP || rcLayer.uiNumIter >= MAX_NUM_ITER )
    {
      rcLayer.uiMotionFileMode  = 1;
      rcLayer.dRate             = (double)uiCurrSize / dSeqLength * 8.0 / 1000.0;
      return;
    }

    //--- update intervall borders ---
    if( uiCurrSize < uiTargetSize )
    {
      //--- lower border ---
      uiMinSize   = uiCurrSize;
      dMinSizeQP  = dCurrQP;
      iMinSizeQP  = iCurrQP;
    }
    else
    {
      //--- upper border ---
      uiMaxSize   = uiCurrSize;
      dMaxSizeQP  = dCurrQP;
      iMaxSizeQP  = iCurrQP;
    }

    //--- get new BaseQpResidual ---
    if( dMinSizeQP != 1000.0 && dMaxSizeQP != -1000.0 )
    {
      //--- two borders ---
      dCurrQP  = dMinSizeQP - dMaxSizeQP;
      dCurrQP *= log( (double)uiMaxSize / (double)uiTargetSize );
      dCurrQP /= log( (double)uiMaxSize / (double)uiMinSize    );
      dCurrQP += dMaxSizeQP;
    }
    else if( dMinSizeQP != 1000.0 )
    {
      //--- lower border: decrease BaseQpResidual ---
      double dDQP = dCurrQP - ( dMinSizeQP + DELTA_QP * log( (double)uiMinSize / (double)uiTargetSize ) );
      dDQP        = ( dDQP < MIN_DQP ? MIN_DQP : dDQP );
      dCurrQP    -= dDQP;
      if( dCurrQP < MIN_QP )
      {
        dCurrQP   = MIN_QP;
      }
    }
    else
    {
      //--- upper border: increase BaseQpResidual ---
      double dDQP = ( dMaxSizeQP + DELTA_QP * log( (double)uiMaxSize / (double)uiTargetSize ) ) - dCurrQP;
      dDQP        = ( dDQP < MIN_DQP ? MIN_DQP : dDQP );
      dCurrQP    += dDQP;
      if( dCurrQP > MAX_QP )
      {
        dCurrQP   = MAX_QP;
      }
    }
  }
}

#define ROT(x)  {if(x) return 1;}
#define ROF(x)  {if(!x)return 1;}

int
read_line( FILE* pFile, char* pcFormat, void* pPar )
{
  if( pPar )
  {
    int  result = fscanf( pFile, pcFormat, pPar );
    ROF( result );
  }

  for( int n = 0; n < 1024; n++ )
  {
    if( '\n' == fgetc( pFile ) )
    {
      return 0;
    }
  }
  return 1;
}

int
read_config_file( EncoderParameters& cEncoderParameters, FILE* pFile )
{
  char acTempString[1024];
  ROF( pFile );

  ROT( read_line( pFile, "",    NULL ) );
  ROT( read_line( pFile, "",    NULL ) );
  ROT( read_line( pFile, "%s",  acTempString ) );
  cEncoderParameters.cLabel         = acTempString;
  ROT( read_line( pFile, "%s",  acTempString ) );
  cEncoderParameters.cEncoderBinary = acTempString;
  ROT( read_line( pFile, "%s",  acTempString ) );
  cEncoderParameters.cParameterFile = acTempString;
  ROT( read_line( pFile, "%s",  acTempString ) );
  cEncoderParameters.cBitStreamFile = acTempString;
  ROT( read_line( pFile, "%s",  acTempString ) );
  cEncoderParameters.cMotionFolder  = acTempString;
  ROT( read_line( pFile, "%d",  &cEncoderParameters.uiNumberOfFrames ) );
  ROT( read_line( pFile, "%lf", &cEncoderParameters.dFramesPerSecond ) );
  ROT( read_line( pFile, "%d",  &cEncoderParameters.uiNumberOfLayers ) );
  ROT( read_line( pFile, "%d",  &cEncoderParameters.uiConstrainedIntraBL ) );
  ROT( read_line( pFile, "",    NULL ) );

  for( unsigned int uiLayer = 0; uiLayer < cEncoderParameters.uiNumberOfLayers; uiLayer++ )
  {
    LayerParameters& rcLayer = cEncoderParameters.acLayerParameters[uiLayer];

    ROT( read_line( pFile, "",    NULL ) );
    ROT( read_line( pFile, "",    NULL ) );
    ROT( read_line( pFile, "%lf", &rcLayer.dTargetRate ) );
    ROT( read_line( pFile, "%lf", &rcLayer.dMinMismatch ) );
    ROT( read_line( pFile, "%lf", &rcLayer.dMaxMismatch ) );
    ROT( read_line( pFile, "%lf", &rcLayer.dStartBaseQpResidual ) );
		ROT( read_line( pFile, "%lf", &rcLayer.dStartQpModeDecision ) );
  }

  return 0;
}

int main( int argc, char** argv)
{
  EncoderParameters cEncoderParameters;
  
  USAGE( argc < 2 );
  FILE* pFile = fopen( argv[1], "rt" );
  if( ! pFile )
  {
    printf("\n\nCannot open config file \"%s\"\n\n", argv[1] );
    exit(1);
  }
  else
  {
    if( read_config_file( cEncoderParameters, pFile ) )
    {
      printf("\n\nError while reading from config file \"%s\"\n\n", argv[1] );
      exit(1);
    }
    fclose( pFile );
  }
  
  //----- iterative encode -----
  unsigned int uiNumberOfLayers = cEncoderParameters.uiNumberOfLayers;
  for( unsigned int uiNumLayers = 1; uiNumLayers <= uiNumberOfLayers; uiNumLayers++ )
  {
    cEncoderParameters.uiNumberOfLayers = uiNumLayers;
    encode_layer( cEncoderParameters );
  }

  //----- output -----
  fprintf( stderr, "\n\n\n" );
  for( unsigned int uiLayer = 0; uiLayer < uiNumberOfLayers; uiLayer++ )
  {
    fprintf( stderr, "L%d:   QP = %lf    MQP = %lf    RATE =%10.4lf [ %6.3lf %% ] (%u iterations)\n",
      uiLayer,
        cEncoderParameters.acLayerParameters[uiLayer].dBaseQpResidual,
				cEncoderParameters.acLayerParameters[uiLayer].dQpModeDecision,
        cEncoderParameters.acLayerParameters[uiLayer].dRate,
      ( cEncoderParameters.acLayerParameters[uiLayer].dRate - 
        cEncoderParameters.acLayerParameters[uiLayer].dTargetRate ) /
        cEncoderParameters.acLayerParameters[uiLayer].dTargetRate * 100.0,
				cEncoderParameters.acLayerParameters[uiLayer].uiNumIter );
  }
  printf("\n\n\n");

  return 0;
}
