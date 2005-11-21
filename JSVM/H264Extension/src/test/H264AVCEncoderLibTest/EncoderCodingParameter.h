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



#define ROTREPORT(x,t) {if(x) {::printf("\n%s\n",t); assert(0); return Err::m_nInvalidParameter;} }


class EncoderConfigLineCStr : public h264::EncoderConfigLineBase
{
public:
  EncoderConfigLineCStr(Char* pcTag, Char** pcPar, Char* pcDefault)
  {
    strcpy(m_pcTag, pcTag);
    m_pcPar  = *pcPar;
    strcpy(m_pcPar, pcDefault);
    m_uiType = 1;
  };
  void setVar(Char* pvValue) {
    strcpy(m_pcPar, pvValue);
  };
protected:
  Char* m_pcPar;
};

class EncoderConfigLineStr : public h264::EncoderConfigLineBase
{
public:
  EncoderConfigLineStr(Char* pcTag, std::string* pcPar, Char* pcDefault)
  {
    strcpy(m_pcTag, pcTag);
    m_pcPar  = pcPar;
    *m_pcPar = pcDefault;
    m_uiType = 1;
  };
  void setVar(Char* pvValue) {
    *m_pcPar = pvValue;
  };
protected:
  std::string* m_pcPar;
};

class EncoderConfigLineDbl : public h264::EncoderConfigLineBase
{
public:
  EncoderConfigLineDbl(Char* pcTag, Double* pdPar, Double pdDefault)
  {
    strcpy(m_pcTag, pcTag);
    m_pdPar  = pdPar;
    *m_pdPar = pdDefault;
    m_uiType = 2;
  };
  void setVar(Char* pvValue) {
    *m_pdPar = atof(pvValue);
  };
protected:
  Double* m_pdPar;
};

class EncoderConfigLineInt : public h264::EncoderConfigLineBase
{
public:
  EncoderConfigLineInt(Char* pcTag, Int* piPar, Int piDefault)
  {
    strcpy(m_pcTag, pcTag);
    m_piPar  = piPar;
    *m_piPar = piDefault;
    m_uiType = 3;
  };
  void setVar(Char* pvValue) {
    *m_piPar = atoi(pvValue);
  };
protected:
  Int* m_piPar;
};

class EncoderConfigLineUInt : public h264::EncoderConfigLineBase
{
public:
  EncoderConfigLineUInt(Char* pcTag, UInt* puiPar, UInt puiDefault)
  {
    strcpy(m_pcTag, pcTag);
    m_puiPar  = puiPar;
    *m_puiPar = puiDefault;
    m_uiType  = 4;
  };
  void setVar(Char* pvValue) {
    *m_puiPar = atoi(pvValue);
  };
protected:
  UInt* m_puiPar;
};

class EncoderConfigLineChar : public h264::EncoderConfigLineBase
{
public:
  EncoderConfigLineChar(Char* pcTag, Char* pcPar, Char pcDefault)
  {
    strcpy(m_pcTag, pcTag);
    m_pcPar  = pcPar;
    *m_pcPar = pcDefault;
    m_uiType = 5;
  };
  void setVar(Char* pvValue) {
    *m_pcPar = (Char)atoi(pvValue);
  };
protected:
  Char* m_pcPar;
};


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
  
  ErrVal  xReadLine( FILE* hFile, Char paacTag[4][256] );
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
      printf("\n********** layer %1d - rqp = %f **********\n\n",uiLayer,dResQp);
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
    if( equals( pcCom, "-bcip", 5 ) )
    {
      n--;
      ROTS( NULL == argv[n] );
      CodingParameter::getLayerParameters(0).setContrainedIntraForLP();
      continue;
    }
    if( equals( pcCom, "-cl", 3 ) )
    {
      ROTS( NULL == argv[n] );
      ROTS( NULL == argv[n+1] );
      UInt uiLayer = atoi( argv[n] );
      UInt uiCLoop = atoi( argv[n+1] );
      CodingParameter::getLayerParameters( uiLayer ).setClosedLoop( uiCLoop );
      n += 1;
      continue;
    }
    if( equals( pcCom, "-ref", 4 ) )
    {
      ROTS( NULL == argv[n] );
      Double dLowPassEnhRef = atof( argv[n] );
      CodingParameter::setLowPassEnhRef( dLowPassEnhRef );
      continue;
    }
    if( equals( pcCom, "-ar", 3 ) )
    {
      ROTS( NULL == argv[n] );
      ROTS( NULL == argv[n + 1] );
      UInt uiBaseRefWeightZeroBlock = atoi( argv[n] );
      UInt uiBaseRefWeightZeroCoeff = atoi( argv[n + 1] );
      CodingParameter::setAdaptiveRefFGSWeights( uiBaseRefWeightZeroBlock, uiBaseRefWeightZeroCoeff );
      // skip two
      n += 1;
      continue;
    }

    if( equals( pcCom, "-pf", 3) )
    {
      ROTS( NULL == argv[n] );
      RNOKS( xReadFromFile( argv[n],
                            pcBitstreamFile ) );
      continue;
    }
    //JVT-P031
    if( equals( pcCom, "-ds", 3) )
    {
     ROTS( NULL == argv[n] );
     ROTS( NULL == argv[n+1] );
     UInt uiLayer = atoi(argv[n]);
     CodingParameter::getLayerParameters(uiLayer).setUseDiscardable(true);
     Double dRate = atof(argv[n+1]);
     CodingParameter::getLayerParameters(uiLayer).setPredFGSRate(dRate);
     n+=1;
     continue;
    }
    //~JVT-P031

    if( equals( pcCom, "-h", 2) )
    {
      printHelp();
      return Err::m_nOK;
    }
    //{{Adaptive GOP structure
	  // --ETRI & KHU
	  if (equals( pcCom, "-anaags", 7) )
	  {
	    m_uiWriteGOPMode	= atoi( argv[n] );
        continue;
	  }
	  //}}Adaptive GOP structure

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
  printf("  -anafgs (Layer) (NumFGSLayers) (File for storing FGS parameters)\n");
  printf("  -encfgs (Layer) (bit-rate in kbps) (File with stored FGS parameters)\n");
  printf("  -cl     (Layer) (ClosedLoopParameter)\n");
  printf("  -ds     (Layer) (Rate for inter-layer prediction)\n");
  printf("  -lcupd  Update method [0 - original, 1 - low-complexity (default)]\n");
  printf("  -bcip   Constrained intra prediction for base layer (needed for single-loop) in scripts\n");
  printf("  -anaags  [1 - mode decision for Adaptive GOP Structure]\n");
  printf("  -h      Print Option List \n");
  printf("\n");
}


ErrVal EncoderCodingParameter::xReadLine( FILE* hFile, Char paacTag[4][256] )
{
  if( NULL != paacTag )
  {
    Int  n;
    UInt uiTagNum = 0;
    Bool bFlush   = false;
    Char* pacStr  = &paacTag[0][0];
    for( n = 0; n < 4; n++ )
    {
      paacTag[n][0] = 0;
    }
    for( n = 0; n < 1024; n++ )
    {
      Char cChar = fgetc( hFile );
      if ( cChar == '\n' || feof (hFile) )
      {
        cChar = 0;
      } else if ( cChar == '#' )
      {
        bFlush = true;
      }
      if ( !bFlush )
      {
        if ( cChar == ' ' )
        {
          *pacStr = 0;
          ROTR( uiTagNum == 3, Err::m_nERR );
          if ( paacTag[uiTagNum][0] != 0 )
          {
            uiTagNum++;
            pacStr = &paacTag[uiTagNum][0];
          }
        } else {
          *pacStr = cChar;
          pacStr++;

        }
      }
      ROTRS( 0 == cChar, Err::m_nOK );
    }
  }

  return Err::m_nERR;
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

  Char aacTags[4][256];
  UInt uiLayerCnt   = 0;
  UInt uiParLnCount = 0;

  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineCStr("OutputFile",              &pcBitstreamFile,                                      "test.264");
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineDbl ("FrameRate",               &m_dMaximumFrameRate,                                  60.0      );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineDbl ("MaxDelay",                &m_dMaximumDelay,                                      1200.0    );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("FramesToBeEncoded",       &m_uiTotalFrames,                                      1 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("GOPSize",                 &m_uiGOPSize,                                          1 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("IntraPeriod",             &m_uiIntraPeriod,                                      UInt(-1));
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("NumberReferenceFrames",   &m_uiNumRefFrames,                                     1 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("BaseLayerMode",           &m_uiBaseLayerMode,                                    3 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("NumLayers",               &m_uiNumberOfLayers,                                   1 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("SearchRange",             &(m_cMotionVectorSearchParams.m_uiSearchRange),        96);
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("BiPredIter",              &(m_cMotionVectorSearchParams.m_uiNumMaxIter),         4 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("IterSearchRange",         &(m_cMotionVectorSearchParams.m_uiIterSearchRange),    8 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("LoopFilterDisable",       &(m_cLoopFilterParams.m_uiFilterIdc),                  0 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineInt ("LoopFilterAlphaC0Offset", (Int*)&(m_cLoopFilterParams.m_iAlphaOffset),           0 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineInt ("LoopFilterBetaOffset",    (Int*)&(m_cLoopFilterParams.m_iBetaOffset),            0 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineInt ("SearchMode",              (Int*)&(m_cMotionVectorSearchParams.m_eSearchMode),    0 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineInt ("SearchFuncFullPel",       (Int*)&(m_cMotionVectorSearchParams.m_eFullPelDFunc),  0 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineInt ("SearchFuncSubPel",        (Int*)&(m_cMotionVectorSearchParams.m_eSubPelDFunc),   0 );
  //{{Adaptive GOP structure
  // --ETRI & KHU
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("UseAdaptiveGOP",          &m_uiUseAGS,                                           0 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineUInt("AGSModeDecision",         &m_uiWriteGOPMode,                                     0 );
  m_pEncoderLines[uiParLnCount++] = new EncoderConfigLineStr ("AGSGOPModeFile",          &m_cGOPModeFilename,                           "ags.dat" );
  //}}Adaptive GOP structure
  m_pEncoderLines[uiParLnCount] = NULL;

  while (!feof(f))
  {
    RNOK( xReadLine( f, aacTags ) );
    if ( aacTags[0][0] == 0 )
    {
      continue;
    }
    for (UInt ui=0; m_pEncoderLines[ui] != NULL; ui++)
    {
      if (equals( aacTags[0], m_pEncoderLines[ui]->getTag(), strlen(m_pEncoderLines[ui]->getTag())))
      {
        m_pEncoderLines[ui]->setVar(aacTags[1]);
        break;
      }
    }
    if( equals( aacTags[0], "LayerCfg", 8) )
    {
      strcpy (aacLayerConfigName[uiLayerCnt++], aacTags[1]);
      continue;
    }
  }

  uiParLnCount = 0;
  while (m_pEncoderLines[uiParLnCount] != NULL)
  {
    delete m_pEncoderLines[uiParLnCount];
    m_pEncoderLines[uiParLnCount] = NULL;
    uiParLnCount++;
  }

  if ( uiLayerCnt != m_uiNumberOfLayers )
  {
    fprintf(stderr, "Could not locate all layer config files: check config file syntax\n");
    AOT(1);
  }

  fclose( f );

  for( ui = 0; ui < m_uiNumberOfLayers; ui++ )
  {
    getLayerParameters(ui).setLayerId(ui);
    RNOK( xReadLayerFromFile( aacLayerConfigName[ui], getLayerParameters(ui) ) );
// TMM_ESS {
    ResizeParameters * curr;
    curr = getResizeParameters(ui);

    // HS: set base layer id
    UInt uiBaseLayerId = getLayerParameters(ui).getBaseLayerId();
    if( ui && uiBaseLayerId == MSYS_UINT_MAX )
    {
      uiBaseLayerId = ui - 1; // default value
    }
    getLayerParameters(ui).setBaseLayerId(uiBaseLayerId);
    // HS: set base layer id

    if (ui>0)
    {
      ResizeParameters * prev = getResizeParameters(uiBaseLayerId); // HS: use "real" base layer
      curr->m_iInWidth  = prev->m_iOutWidth;
      curr->m_iInHeight = prev->m_iOutHeight;

      bool is_crop_aligned = (curr->m_iPosX%16 == 0) && (curr->m_iPosY%16 == 0);
      if      ((curr->m_iInWidth == curr->m_iOutWidth) && (curr->m_iInHeight == curr->m_iOutHeight) &&
               is_crop_aligned && (curr->m_iExtendedSpatialScalability < ESS_PICT) )
        curr->m_iSpatialScalabilityType = SST_RATIO_1;
      else if ((curr->m_iInWidth*2 == curr->m_iOutWidth) && (curr->m_iInHeight*2 == curr->m_iOutHeight) &&
               is_crop_aligned && (curr->m_iExtendedSpatialScalability < ESS_PICT) )
        curr->m_iSpatialScalabilityType = SST_RATIO_2;
      else if ((curr->m_iInWidth*3 == curr->m_iOutWidth*2) && (curr->m_iInHeight*3 == curr->m_iOutHeight*2) &&
               is_crop_aligned && (curr->m_iExtendedSpatialScalability < ESS_PICT) )
      {
        curr->m_iSpatialScalabilityType = SST_RATIO_3_2;
        if ( curr->m_iExtendedSpatialScalability == ESS_NONE )
          curr->m_iExtendedSpatialScalability = ESS_SEQ;
      }
      else
      {
        curr->m_iSpatialScalabilityType = SST_RATIO_X;
        if ( curr->m_iExtendedSpatialScalability == ESS_NONE )
          curr->m_iExtendedSpatialScalability = ESS_SEQ;
      }
     }
    else
    {
      curr->m_iSpatialScalabilityType = SST_RATIO_1;
      curr->m_iExtendedSpatialScalability = ESS_NONE;
    }
// TMM_ESS }
  }

  return Err::m_nOK;
}




ErrVal EncoderCodingParameter::xReadLayerFromFile ( Char*                   pcFilename,
                                                    h264::LayerParameters&  rcLayer )
{
  Char  acTemp[256];
  Char  acEssFsp[256], *pcEssFsp = acEssFsp;
  FILE *f = fopen( pcFilename, "r");
  if( NULL == f )
  { 
    printf( "failed to open %s layer config file\n",pcFilename);
    return Err::m_nERR;
  } 

  Char aacTags[4][256];
  UInt uiParLnCount = 0;
  Char cInfile[256], cOutfile[256], cMotfile[256];
  Char *pcInfile  = cInfile;
  Char *pcOutfile = cOutfile;
  Char *pcMotfile = cMotfile;

  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("SourceWidth",    &(rcLayer.m_uiFrameWidth),               176       );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("SourceHeight",   &(rcLayer.m_uiFrameHeight),              352       );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineDbl ("FrameRateIn",    &(rcLayer.m_dInputFrameRate),            30        );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineDbl ("FrameRateOut",   &(rcLayer.m_dOutputFrameRate),           30        );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineCStr("InputFile",      &pcInfile,                               "test.yuv");
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineCStr("ReconFile",      &pcOutfile,                              "rec.yuv" );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("SymbolMode",     &(rcLayer.m_uiEntropyCodingModeFlag),    1         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("UpdateStep",     &(rcLayer.m_uiUpdateStep),               1         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("ClosedLoop",     &(rcLayer.m_uiClosedLoop),               0         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("AdaptiveQP",     &(rcLayer.m_uiAdaptiveQPSetting),        1         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("UseIntra",       &(rcLayer.m_uiMCTFIntraMode),            1         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("FRExt",          &(rcLayer.m_uiAdaptiveTransform),        0         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("MaxDeltaQP",     &(rcLayer.m_uiMaxAbsDeltaQP),            1         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineDbl ("QP",             &(rcLayer.m_dBaseQpResidual),            32.0      );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineDbl ("NumFGSLayers",   &(rcLayer.m_dNumFGSLayers),              0         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineDbl ("MeQP0",          &(rcLayer.m_adQpModeDecision[0]),        32.0      );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineDbl ("MeQP1",          &(rcLayer.m_adQpModeDecision[1]),        32.0      );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineDbl ("MeQP2",          &(rcLayer.m_adQpModeDecision[2]),        32.0      );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineDbl ("MeQP3",          &(rcLayer.m_adQpModeDecision[3]),        32.0      );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineDbl ("MeQP4",          &(rcLayer.m_adQpModeDecision[4]),        32.0      );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineDbl ("MeQP5",          &(rcLayer.m_adQpModeDecision[5]),        32.0      );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("InterLayerPred", &(rcLayer.m_uiInterLayerPredictionMode), 0         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("BaseQuality",    &(rcLayer.m_uiBaseQualityLevel),         3         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("DecodeLoops",    &(rcLayer.m_uiDecodingLoops),            1         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("MotionInfoMode", &(rcLayer.m_uiMotionInfoMode),           0         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineCStr("MotionInfoFile", &pcMotfile,                              "test.mot");
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineInt ("UseESS",         &(rcLayer.m_ResizeParameter.m_iExtendedSpatialScalability), 0         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineCStr("ESSPicParamFile",&pcEssFsp,                                                  "ess.dat" );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineInt ("ESSCropWidth",   &(rcLayer.m_ResizeParameter.m_iOutWidth),                   0         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineInt ("ESSCropHeight",  &(rcLayer.m_ResizeParameter.m_iOutHeight),                  0         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineInt ("ESSOriginX",     &(rcLayer.m_ResizeParameter.m_iPosX),                       0         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineInt ("ESSOriginY",     &(rcLayer.m_ResizeParameter.m_iPosY),                       0         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("ForceReOrdering",&(rcLayer.m_uiForceReorderingCommands),  0         );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("BaseLayerId",    &(rcLayer.m_uiBaseLayerId),              MSYS_UINT_MAX );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineDbl ("EnhRefME",       &(rcLayer.m_dLowPassEnhRef),              0.5 );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("WeightZeroBlock",&(rcLayer.m_uiBaseWeightZeroBaseBlock),          2       );
  m_pLayerLines[uiParLnCount++] = new EncoderConfigLineUInt("WeightZeroCoeff",&(rcLayer.m_uiBaseWeightZeroBaseCoeff),          14       );
  m_pLayerLines[uiParLnCount] = NULL;

  // SSUN@SHARP reset ResizeParameters
  if( rcLayer.m_ResizeParameter.m_iExtendedSpatialScalability == 0 )
  {
    rcLayer.m_ResizeParameter.m_iOutWidth = rcLayer.m_uiFrameWidth;
    rcLayer.m_ResizeParameter.m_iOutHeight = rcLayer.m_uiFrameHeight;
  } // end of SSUN@SHARP end of reset ResizeParameters

  while (!feof(f))
  {
    RNOK( xReadLine( f, aacTags ) );
    if ( aacTags[0][0] == 0 )
    {
      continue;
    }
    for (UInt ui=0; m_pLayerLines[ui] != NULL; ui++)
    {
      if (equals( aacTags[0], m_pLayerLines[ui]->getTag(), strlen(m_pLayerLines[ui]->getTag())))
      {
        m_pLayerLines[ui]->setVar(aacTags[1]);
        break;
      }
    }
  }
  rcLayer.setInputFilename     ( cInfile  );
  rcLayer.setOutputFilename    ( cOutfile );
  rcLayer.setMotionInfoFilename( cMotfile );

  uiParLnCount = 0;
  while (m_pLayerLines[uiParLnCount] != NULL)
  {
    delete m_pLayerLines[uiParLnCount];
    m_pLayerLines[uiParLnCount] = NULL;
    uiParLnCount++;
  }

// TMM_ESS {
  // default values
  rcLayer.m_ResizeParameter.m_iInWidth    = rcLayer.m_uiFrameWidth;
  rcLayer.m_ResizeParameter.m_iInHeight   = rcLayer.m_uiFrameHeight;
  rcLayer.m_ResizeParameter.m_iGlobWidth  = rcLayer.m_uiFrameWidth;
  rcLayer.m_ResizeParameter.m_iGlobHeight = rcLayer.m_uiFrameHeight;
  rcLayer.m_ResizeParameter.m_bCrop       = false;
  if(rcLayer.m_ResizeParameter.m_iExtendedSpatialScalability)  
  {
    rcLayer.m_ResizeParameter.m_bCrop = true;        
    if(rcLayer.m_ResizeParameter.m_iExtendedSpatialScalability==2)
    {
      rcLayer.m_ResizeParameter.m_pParamFile = fopen( acEssFsp, "r");
      if( NULL == rcLayer.m_ResizeParameter.m_pParamFile )
      { 
        printf( "failed to open resize parameter file %s\n", acTemp);
        return Err::m_nERR;
      }
      rcLayer.m_ResizeParameter.m_iSpatialScalabilityType = SST_RATIO_X;
      rcLayer.m_ResizeParameter.m_iBaseChromaPhaseX       = -1;
      rcLayer.m_ResizeParameter.m_iBaseChromaPhaseY       = -1;
      rcLayer.m_ResizeParameter.m_iChromaPhaseX           = -1;
      rcLayer.m_ResizeParameter.m_iChromaPhaseY           = -1;
    }
  } else {
    // default values
    rcLayer.m_ResizeParameter.m_iOutWidth   = rcLayer.m_uiFrameWidth;
    rcLayer.m_ResizeParameter.m_iOutHeight  = rcLayer.m_uiFrameHeight;
    rcLayer.m_ResizeParameter.m_iPosX       = 0;
    rcLayer.m_ResizeParameter.m_iPosY       = 0;
  }
// TMM_ESS }

  ::fclose(f);

  return Err::m_nOK;
}



#endif // !defined(AFX_ENCODERCODINGPARAMETER_H__145580A5_E0D6_4E9C_820F_EA4EF1E1B793__INCLUDED_)
