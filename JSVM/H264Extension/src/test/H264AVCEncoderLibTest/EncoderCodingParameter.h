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



#if !defined(AFX_ENCODERCODINGPARAMETER_H__145580A5_E0D6_4E9C_820F_EA4EF1E1B793__INCLUDED_)
#define AFX_ENCODERCODINGPARAMETER_H__145580A5_E0D6_4E9C_820F_EA4EF1E1B793__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
 
 
#include "string.h" 
#include "CodingParameter.h"



 
class EncoderCodingParameter : 
public h264::CodingParameter 
{
protected: 
  EncoderCodingParameter          (){}
  virtual ~EncoderCodingParameter (){}

public:
  static ErrVal create    ( EncoderCodingParameter*& rpcEncoderCodingParameter );
  ErrVal        destroy   ();
  ErrVal        init      ( Int     argc,
                            Char**  argv,
                            Char*   pcBitstreamFile );

  Void          printHelp ();

protected:
  Bool    equals( const Char* str1, const Char* str2, UInt nLetter ) { return 0 == ::strncmp( str1, str2, nLetter); }

  ErrVal  xReadFromFile      ( Char*                   pcFilename,
                               Char*                   pcBitstreamFile  );
  ErrVal  xReadLayerFromFile ( Char*                   pcFilename,
                               h264::LayerParameters&  rcLayer );
  
  ErrVal  xReadLine( FILE* hFile, Char* pcFormat, Void* pvP0 );
  ErrVal  xReadLine( FILE* hFile, Char* pcFormat, Void* pvP0, Void* pvP1 );
};




ErrVal EncoderCodingParameter::create( EncoderCodingParameter*& rpcEncoderCodingParameter )
{
  rpcEncoderCodingParameter = new EncoderCodingParameter;
  
  ROT( NULL == rpcEncoderCodingParameter );

  return Err::m_nOK;
}


ErrVal EncoderCodingParameter::destroy()
{
  delete this;
  return Err::m_nOK;
}


ErrVal EncoderCodingParameter::init( Int     argc,
                                     Char**  argv,
                                     Char*   pcBitstreamFile  )
{
  Char* pcCom;

  pcBitstreamFile [0] = (Char) 0;

  ROTS( argc < 2 )

  for( Int n = 1; n < argc; n++ )
  {
    pcCom = argv[n++];

    if( equals( pcCom, "-anafgs", 7 ) )
    {
      ROTS( NULL == argv[n  ] );
      ROTS( NULL == argv[n+1] );
      ROTS( NULL == argv[n+2] );
      UInt  uiLayer       = atoi( argv[n++] );
      UInt  uiNumLayers   = atoi( argv[n++] );
      ROT( CodingParameter::getLayerParameters( uiLayer ).getFGSMode() );
      CodingParameter::getLayerParameters( uiLayer ).setNumFGSLayers( uiNumLayers );
      CodingParameter::getLayerParameters( uiLayer ).setFGSFilename ( argv[n]     );
      CodingParameter::getLayerParameters( uiLayer ).setFGSMode     ( 1           );
      continue;
    }
    if( equals( pcCom, "-encfgs", 7 ) )
    {
      ROTS( NULL == argv[n  ] );
      ROTS( NULL == argv[n+1] );
      ROTS( NULL == argv[n+2] );
      UInt    uiLayer   = atoi( argv[n++] );
      Double  dFGSRate  = atof( argv[n++] );
      ROT( CodingParameter::getLayerParameters( uiLayer ).getFGSMode() );
      CodingParameter::getLayerParameters( uiLayer ).setFGSRate     ( dFGSRate    );
      CodingParameter::getLayerParameters( uiLayer ).setFGSFilename ( argv[n]     );
      CodingParameter::getLayerParameters( uiLayer ).setFGSMode     ( 2           );
      continue;
    }

    if( equals( pcCom, "-bf", 3 ) )
    {
      ROTS( NULL == argv[n] );
      strcpy( pcBitstreamFile, argv[n] );
      continue;
    }
    if( equals( pcCom, "-numl", 5 ) )
    {
      ROTS( NULL == argv[n] );
      UInt  uiNumLayers = atoi( argv[n] );
      CodingParameter::setNumberOfLayers( uiNumLayers );
      continue;
    }
    if( equals( pcCom, "-rqp", 4 ) )
    {
      ROTS( NULL == argv[n  ] );
      ROTS( NULL == argv[n+1] );
      UInt    uiLayer = atoi( argv[n  ] );
      Double  dResQp  = atof( argv[n+1] );
      CodingParameter::getLayerParameters( uiLayer ).setBaseQpResidual( dResQp );
      n += 1;
      continue;      
    }
    if( equals( pcCom, "-mqp", 4 ) )
    {
      ROTS( NULL == argv[n  ] );
      ROTS( NULL == argv[n+1] );
      ROTS( NULL == argv[n+2] );
      UInt    uiLayer = atoi( argv[n  ] );
      UInt    uiStage = atoi( argv[n+1] );
      Double  dMotQp  = atof( argv[n+2] );
      CodingParameter::getLayerParameters( uiLayer ).setQpModeDecision( uiStage, dMotQp );
      n += 2;
      continue;      
    }
    if( equals( pcCom, "-lqp", 4 ) )
    {
      ROTS( NULL == argv[n  ] );
      ROTS( NULL == argv[n+1] );
      UInt    uiLayer = atoi( argv[n  ] );
      Double  dQp     = atof( argv[n+1] );
      CodingParameter::getLayerParameters( uiLayer ).setBaseQpResidual( dQp );
      for( UInt uiStage = 0; uiStage < MAX_DSTAGES; uiStage++ )
      {
        CodingParameter::getLayerParameters( uiLayer ).setQpModeDecision( uiStage, dQp );
      }
      n += 1;
      continue;      
    }
    if( equals( pcCom, "-ilpred", 7 ) )
    {
      ROTS( NULL == argv[n  ] );
      ROTS( NULL == argv[n+1] );
      UInt    uiLayer = atoi( argv[n  ] );
      UInt    uiBLRes = atoi( argv[n+1] );
      CodingParameter::getLayerParameters( uiLayer ).setInterLayerPredictionMode( uiBLRes );
      n += 1;
      continue;
    }
    if( equals( pcCom, "-mfile", 6 ) )
    {
      ROTS( NULL == argv[n  ] );
      ROTS( NULL == argv[n+1] );
      ROTS( NULL == argv[n+2] );
      UInt    uiLayer = atoi( argv[n  ] );
      UInt    uiMode  = atoi( argv[n+1] );
      CodingParameter::getLayerParameters( uiLayer ).setMotionInfoMode( uiMode );
      CodingParameter::getLayerParameters( uiLayer ).setMotionInfoFilename( argv[n+2] );
      n += 2;
      continue;
    }
    if( equals( pcCom, "-frms", 5 ) )
    {
      ROTS( NULL == argv[n] );
      UInt uiFrms = atoi( argv[n] );
      CodingParameter::setTotalFrames( uiFrms );
      continue;
    }

    if( equals( pcCom, "-pf", 3) )
    {
      ROTS( NULL == argv[n] );
      RNOKS( xReadFromFile( argv[n],
                            pcBitstreamFile ) );
      continue;
    }

    if( equals( pcCom, "-h", 2) )
    {
      printHelp();
      return Err::m_nOK;
    }

    return Err::m_nERR;
  }


  RNOKS( check() );
  
  return Err::m_nOK;
}


Void EncoderCodingParameter::printHelp()
{
  printf("\n supported options:\n\n");
  printf("  -pf     Parameter File Name\n\n");

  printf("  -bf     BitStreamFile\n");
  printf("  -frms   Number of total frames\n");
  printf("  -numl   Number Of Layers\n");
  printf("  -rqp    (Layer) (ResidualQP)\n");
  printf("  -mqp    (Layer) (Stage) (MotionQP)\n");
  printf("  -lqp    (Layer) (ResidualAndMotionQP)\n");
  printf("  -ilpred (Layer) (InterLayerPredictionMode)\n");
  printf("  -mfile  (Layer) (Mode) (MotionInfoFile)\n");
  printf("  -h      Print Option List \n");
  printf("\n");
}



ErrVal EncoderCodingParameter::xReadLine( FILE* hFile, Char* pcFormat, Void* pvP0 )
{
  if( NULL != pvP0 )
  {
    ROTR( 0 == fscanf( hFile, pcFormat, pvP0 ), Err::m_nInvalidParameter ); 
  }

  for( Int n = 0; n < 256; n++ )
  {
    ROTRS( '\n' == fgetc( hFile ), Err::m_nOK );
  }

  return Err::m_nERR;
}


ErrVal EncoderCodingParameter::xReadLine( FILE* hFile, Char* pcFormat, Void* pvP0, Void* pvP1 )
{
  if( NULL != pvP0 && NULL != pvP1 )
  {
    ROTR( 0 == fscanf( hFile, pcFormat, pvP0, pvP1 ), Err::m_nInvalidParameter );
  }

  for( Int n = 0; n < 256; n++ )
  {
    ROTRS( '\n' == fgetc( hFile ), Err::m_nOK );
  }

  return Err::m_nERR;
}


ErrVal EncoderCodingParameter::xReadFromFile( Char* pcFilename,
                                              Char* pcBitstreamFile  )
{
  UInt  ui;
  Char  aacLayerConfigName[MAX_LAYERS][256];

  FILE *f = fopen( pcFilename, "r");
  if( NULL == f )
  { 
    printf( "failed to open %s parameter file\n",pcFilename);
    return Err::m_nERR;
  } 

  //=== GENERAL ===
  RNOK( xReadLine( f, "", NULL ) );// skip line
  RNOK( xReadLine( f, "", NULL ) );// skip line 
  RNOK( xReadLine( f, "", NULL ) );// skip line
  RNOK( xReadLine( f, "%s",  pcBitstreamFile ) );
  RNOK( xReadLine( f, "%lf", &m_dMaximumFrameRate ) );
  RNOK( xReadLine( f, "%lf", &m_dMaximumDelay ) );
  RNOK( xReadLine( f, "%d",  &m_uiTotalFrames ) );

  //=== MCTF ===
  RNOK( xReadLine( f, "", NULL ) );// skip line
  RNOK( xReadLine( f, "", NULL ) );// skip line
  RNOK( xReadLine( f, "%d",  &m_uiGOPSize ) );
  RNOK( xReadLine( f, "%d",  &m_uiIntraPeriod ) );
  RNOK( xReadLine( f, "%d",  &m_uiNumRefFrames ) );
  RNOK( xReadLine( f, "%d",  &m_uiBaseLayerMode ) );

  //=== MOTION SEARCH ===
  RNOK( xReadLine( f, "", NULL ) );// skip line
  RNOK( xReadLine( f, "", NULL ) );// skip line
  RNOK( xReadLine( f, "%d", &m_cMotionVectorSearchParams.m_eSearchMode ) );
  RNOK( xReadLine( f, "%d", &m_cMotionVectorSearchParams.m_eFullPelDFunc ) );
  RNOK( xReadLine( f, "%d", &m_cMotionVectorSearchParams.m_eSubPelDFunc ) );
  RNOK( xReadLine( f, "%d", &m_cMotionVectorSearchParams.m_uiSearchRange ) );
  RNOK( xReadLine( f, "%d", &m_cMotionVectorSearchParams.m_uiNumMaxIter ) );
  RNOK( xReadLine( f, "%d", &m_cMotionVectorSearchParams.m_uiIterSearchRange ) );
  
  //=== LOOP FILTER ===
  RNOK( xReadLine( f, "", NULL ) );// skip line
  RNOK( xReadLine( f, "", NULL ) );// skip line
  RNOK( xReadLine( f,"%d", &m_cLoopFilterParams.m_uiFilterIdc ) );
  RNOK( xReadLine( f,"%d", &m_cLoopFilterParams.m_iAlphaOffset ) );
  RNOK( xReadLine( f,"%d", &m_cLoopFilterParams.m_iBetaOffset ) );
  
  //=== LAYER DEFINITION ===
  RNOK( xReadLine( f, "", NULL ) );// skip line
  RNOK( xReadLine( f, "", NULL ) );// skip line
  RNOK( xReadLine( f, "%d", &m_uiNumberOfLayers ) );

  for( ui = 0; ui < m_uiNumberOfLayers; ui++ )
  {
    RNOK( xReadLine( f, "%s", aacLayerConfigName[ui] ) );
  }

  fclose( f );

  for( ui = 0; ui < m_uiNumberOfLayers; ui++ )
  {
    getLayerParameters(ui).setLayerId(ui);
    RNOK( xReadLayerFromFile( aacLayerConfigName[ui], getLayerParameters(ui) ) );
  }

  return Err::m_nOK;
}




ErrVal EncoderCodingParameter::xReadLayerFromFile ( Char*                   pcFilename,
                                                    h264::LayerParameters&  rcLayer )
{
  Char  acTemp[256];
  FILE *f = fopen( pcFilename, "r");
  if( NULL == f )
  { 
    printf( "failed to open %s layer config file\n",pcFilename);
    return Err::m_nERR;
  } 

  RNOK( xReadLine( f, "", NULL ) );// skip line
  RNOK( xReadLine( f, "", NULL ) );// skip line 
  RNOK( xReadLine( f, "", NULL ) );// skip line
  RNOK( xReadLine( f, "%d",  &rcLayer.m_uiFrameWidth ) );
  RNOK( xReadLine( f, "%d",  &rcLayer.m_uiFrameHeight ) );
  RNOK( xReadLine( f, "%lf", &rcLayer.m_dInputFrameRate ) );
  RNOK( xReadLine( f, "%lf", &rcLayer.m_dOutputFrameRate ) );
  RNOK( xReadLine( f, "%s",  acTemp ) );
  rcLayer.setInputFilename( acTemp );
  RNOK( xReadLine( f, "%s",  acTemp ) );
  rcLayer.setOutputFilename( acTemp );

  
  RNOK( xReadLine( f, "", NULL ) );// skip line 
  RNOK( xReadLine( f, "", NULL ) );// skip line
  RNOK( xReadLine( f, "%d",  &rcLayer.m_uiUpdateStep ) );
  RNOK( xReadLine( f, "%d",  &rcLayer.m_uiAdaptiveQPSetting ) );
  RNOK( xReadLine( f, "%d",  &rcLayer.m_uiMCTFIntraMode ) );
  RNOK( xReadLine( f, "%d",  &rcLayer.m_uiAdaptiveTransform ) );

  
  RNOK( xReadLine( f, "", NULL ) );// skip line 
  RNOK( xReadLine( f, "", NULL ) );// skip line
  RNOK( xReadLine( f, "%d",  &rcLayer.m_uiMaxAbsDeltaQP ) );
  RNOK( xReadLine( f, "%lf", &rcLayer.m_dBaseQpResidual ) );
  RNOK( xReadLine( f, "%lf", &rcLayer.m_dNumFGSLayers ) );

  
  RNOK( xReadLine( f, "", NULL ) );// skip line 
  RNOK( xReadLine( f, "", NULL ) );// skip line
  RNOK( xReadLine( f, "%lf", &rcLayer.m_adQpModeDecision[0] ) );
  RNOK( xReadLine( f, "%lf", &rcLayer.m_adQpModeDecision[1] ) );
  RNOK( xReadLine( f, "%lf", &rcLayer.m_adQpModeDecision[2] ) );
  RNOK( xReadLine( f, "%lf", &rcLayer.m_adQpModeDecision[3] ) );
  RNOK( xReadLine( f, "%lf", &rcLayer.m_adQpModeDecision[4] ) );
  RNOK( xReadLine( f, "%lf", &rcLayer.m_adQpModeDecision[5] ) );
  RNOK( xReadLine( f, "", NULL ) );// skip line
  RNOK( xReadLine( f, "%d",  &rcLayer.m_uiInterLayerPredictionMode ) );
  RNOK( xReadLine( f, "", NULL ) );// skip line
  RNOK( xReadLine( f, "%d",  &rcLayer.m_uiMotionInfoMode ) );
  RNOK( xReadLine( f, "%s",  acTemp ) );
  rcLayer.setMotionInfoFilename( acTemp );

  ::fclose(f);

  return Err::m_nOK;
}



#endif // !defined(AFX_ENCODERCODINGPARAMETER_H__145580A5_E0D6_4E9C_820F_EA4EF1E1B793__INCLUDED_)
