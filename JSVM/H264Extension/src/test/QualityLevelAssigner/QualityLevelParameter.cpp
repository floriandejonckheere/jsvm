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


#include "QualityLevelParameter.h"

#ifndef MSYS_WIN32
#define equal(a,b)  (!strcasecmp((a),(b)))
#else
#define equal(a,b)  (!stricmp((a),(b)))
#endif


QualityLevelParameter::QualityLevelParameter()
: m_uiDataFileMode              ( 0 )
, m_uiDistortionEstimationMode  ( 3 )
, m_bQualityLayerSEI            ( false )
, m_eQLAssignerMode             ( QLASSIGNERMODE_QL )
{
}


QualityLevelParameter::~QualityLevelParameter()
{
}


ErrVal
QualityLevelParameter::create( QualityLevelParameter*& rpcQualityLevelParameter )
{
  rpcQualityLevelParameter = new QualityLevelParameter;
  ROT( NULL == rpcQualityLevelParameter );
  return Err::m_nOK;
}

//manu.mathew@samsung : memory leak fix
ErrVal
QualityLevelParameter::destroy()
{
  delete this;
  return Err::m_nOK;
}
//--

ErrVal
QualityLevelParameter::init( Int argc, Char** argv )
{
  Bool bError = false;
  
  for( Int iArg = 1; iArg < argc; iArg++ )
  {
    if( !strcmp( argv[iArg], "-in" ) )
    {
      if( !(iArg+1<argc) || ! m_cInputBitStreamName.empty() )
      {
        bError  = true;
        break;
      }
      m_cInputBitStreamName = argv[++iArg];
    }
    else if( !strcmp( argv[iArg], "-out" ) )
    {
      if( !(iArg+1<argc) || ! m_cOutputBitStreamName.empty() )
      {
        bError  = true;
        break;
      }
      m_cOutputBitStreamName = argv[++iArg];
    }
    else if( !strcmp( argv[iArg], "-org" ) )
    {
      if( !(iArg+2<argc) )
      {
        bError = true;
        break;
      }
      UInt  uiLayer = atoi( argv[++iArg] );
      if( !(uiLayer<MAX_LAYERS) || ! m_acOriginalFileName[uiLayer].empty() )
      {
        bError  = true;
        break;
      }
      m_acOriginalFileName[uiLayer] = argv[++iArg];
    }
    else if( !strcmp( argv[iArg], "-wp" ) )
    {
      if( !(iArg+1<argc) || m_uiDataFileMode )
      {
        bError  = true;
        break;
      }
      m_uiDataFileMode  = 2;
      m_cDataFileName   = argv[++iArg];
    }
    else if( !strcmp( argv[iArg], "-rp" ) )
    {
      if( !(iArg+1<argc) || m_uiDataFileMode )
      {
        bError  = true;
        break;
      }
      m_uiDataFileMode  = 1;
      m_cDataFileName   = argv[++iArg];
    }
    else if( !strcmp( argv[iArg], "-ind" ) )
    {
      if( m_uiDistortionEstimationMode != 3 )
      {
        bError = true;
        break;
      }
      m_uiDistortionEstimationMode  = 1;
    }
    else if( !strcmp( argv[iArg], "-dep" ) )
    {
      if( m_uiDistortionEstimationMode != 3 )
      {
        bError = true;
        break;
      }
      m_uiDistortionEstimationMode  = 2;
    }
    else if( !strcmp( argv[iArg], "-sei" ) )
    {
      m_bQualityLayerSEI = true;
    }
    //JVT-S043
    else if( !strcmp( argv[iArg], "-mlql" ) )
    {
      m_eQLAssignerMode   = QLASSIGNERMODE_MLQL;
    }
    else
    {
      bError = true;
    }
  }


  //===== consistency check =====
  if( !bError )
  {
    bError  = ( m_cInputBitStreamName.empty() );
  }
  if( !bError )
  {
    bError  = ( m_acOriginalFileName[0].empty() && m_cOutputBitStreamName.empty() );
  }
  if( !bError )
  {
    if( !m_cOutputBitStreamName.empty() )
    {
      bError  = ( m_acOriginalFileName[0].empty() && m_uiDataFileMode != 1 );
    }
    else
    {
      bError  = ( m_acOriginalFileName[0].empty() || m_uiDataFileMode != 2 );
    }
  }


  //==== output when error ====
  if( bError )
  {
    RNOKS( xPrintUsage( argv ) );
  }

  return Err::m_nOK;
}


ErrVal
QualityLevelParameter::xPrintUsage( Char** argv )
{
  printf("Usage: QualityLevelAssigner -in Input -org L Original [-org L Original]\n"
         "                           [-out Output [-sei] | -wp DatFile] [-dep | -ind] [-mlql]\n" );
  printf("or     QualityLevelAssigner -in Input -out Output -rp DatFile [-sei]\n\n" );
  printf("  -in  Input      - input bit-stream\n");
  printf("  -out Output     - output bit-stream with determined quality layer id's\n");
  printf("  -org L Original - original image sequence for layer L\n");
  printf("  -wp  DatFile    - data file for storing rate and distortion values\n");
  printf("  -rp  DatFile    - data file with previously computed rate and\n"
         "                    distortion values\n");
  printf("  -sei            - provide quality layer info using SEI mesages\n");
  printf("  -dep            - determine only dependent distortions\n"
         "                    (speed-up by factor of 2, slight coding eff. losses)\n");
  printf("  -ind            - determine only independent distortions\n"
         "                    (speed-up by factor of 2, slight coding eff. losses)\n\n");
  //JVT-S043
  printf("  -mlql           - determine Multi Layer quality layer id's\n");
  RERRS();
}

