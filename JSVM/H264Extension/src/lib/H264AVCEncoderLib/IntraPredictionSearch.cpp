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





#include "H264AVCEncoderLib.h"
#include "IntraPredictionSearch.h"


H264AVC_NAMESPACE_BEGIN

IntraPredictionSearch::IntraPredictionSearch()
{
}

IntraPredictionSearch::~IntraPredictionSearch()
{
}


ErrVal IntraPredictionSearch::create( IntraPredictionSearch*& rpcIntraPredictionSearch )
{
  rpcIntraPredictionSearch = new IntraPredictionSearch;

  ROT( NULL == rpcIntraPredictionSearch );

  return Err::m_nOK;
}


ErrVal IntraPredictionSearch::destroy()
{
  delete this;

  return Err::m_nOK;
}




ErrVal IntraPredictionSearch::predictSLumaMb( IntYuvMbBuffer *pcYuvBuffer, UInt uiPredMode, Bool& rbValid )
{
  rbValid = false;
  m_uiAvailable = ( m_uiAvailableMaskMb >> 4 ) | m_uiAvailableMaskMb;

  XPel *pPel    = pcYuvBuffer->getMbLumAddr();
  Int   iStride = pcYuvBuffer->getLStride();

  switch( uiPredMode )
  {
  case 0:
    {
      if( xIsAboveRef() )
      {
        xPred16x16IMode0Vert( pPel, iStride );
        rbValid = true;
      }
      break;
    }
  case 1:
    {
      if( xIsLeftRef() )
      {
        xPred16x16IMode1Hori( pPel, iStride );
        rbValid = true;
      }
      break;
    }
  case 2:
    {
      xPred16x16IMode2DC( pPel, iStride );
      rbValid = true;
      break;
    }
  case 3:
    {
      if( xIsAllLeftAboveRef() )
      {
        xPred16x16IMode3Plane( pPel, iStride );
        rbValid = true;
      }
      break;
    }
  default:
    {
      AF();
      return Err::m_nERR;
    }
  }
  return Err::m_nOK;
}



ErrVal IntraPredictionSearch::predictSLumaBlock( IntYuvMbBuffer *pcYuvBuffer, UInt uiPredMode, LumaIdx cIdx, Bool &rbValid )
{
  rbValid       = false;
  XPel* pPel    = pcYuvBuffer->getLumBlk();
  Int   iStride = pcYuvBuffer->getLStride();

  m_uiAvailable = xGetAvailableMask( cIdx );

  switch( uiPredMode & 0xf )
  {
  case 0:
    {
      if( xIsAboveRef() )
      {
        xPredMode0Vert( pPel, iStride );
        rbValid = true;
      }
      break;
    }
  case 1:
    {
      if( xIsLeftRef() )
      {
        xPredMode1Horiz( pPel, iStride );
        rbValid = true;
      }
      break;
    }
  case 2:
    {
      xPredMode2Dc( pPel, iStride );
      rbValid = true;
      break;
    }
  case 3:
    {
      if( xIsAboveRef() )
      {
        xPredMode3DiagDownLeft( pPel, iStride );
        rbValid = true;
      }
      break;
    }
  case 4:
    {
      if( xIsAllLeftAboveRef() )
      {
        xPredMode4DiagDownRight( pPel, iStride );
        rbValid = true;
      }
      break;
    }
  case 5:
    {
      if( xIsAllLeftAboveRef() )
      {
        xPredMode5VertRight( pPel, iStride );
        rbValid = true;
      }
      break;
    }
  case 6:
    {
      if( xIsAllLeftAboveRef() )
      {
        xPredMode6HorizDown( pPel, iStride );
        rbValid = true;
      }
      break;
    }
  case 7:
    {
      if( xIsAboveRef() )
      {
        xPredMode7VertLeft( pPel, iStride );
        rbValid = true;
      }
      break;
    }
  case 8:
    {
      if( xIsLeftRef() )
      {
        xPredMode8HorizUp( pPel, iStride );
        rbValid = true;
      }
      break;
    }
  default:
    {
      assert( 0 );
      return Err::m_nERR;
    }
  }

  return Err::m_nOK;
}



ErrVal IntraPredictionSearch::predictSChromaBlock( IntYuvMbBuffer *pcYuvBuffer, UInt uiPredMode, Bool &rbValid )
{
  m_uiAvailable = ( m_uiAvailableMaskMb >> 4 ) | m_uiAvailableMaskMb;

  rbValid = false;
  XPel *pucCb = pcYuvBuffer->getMbCbAddr();
  XPel *pucCr = pcYuvBuffer->getMbCrAddr();
  Int iStride = pcYuvBuffer->getLStride();

  switch( uiPredMode )
  {
  case 0:
    {
      xPred8x8IMode0DC( pucCb, iStride );
      xPred8x8IMode0DC( pucCr, iStride );
      rbValid = true;
      break;
    }
  case 1:
    {
      ROFRS( xIsLeftRef(), Err::m_nOK );

      xPred8x8IMode1Hori( pucCb, iStride );
      xPred8x8IMode1Hori( pucCr, iStride );
      rbValid = true;
      break;
    }
  case 2:
    {
      ROFRS( xIsAboveRef(), Err::m_nOK );

      xPred8x8IMode2Vert( pucCb, iStride );
      xPred8x8IMode2Vert( pucCr, iStride );
      rbValid = true;
      break;
    }
  case 3:
    {
      ROFRS( xIsAllLeftAboveRef(), Err::m_nOK );

      xPred8x8IMode3Plane( pucCb, iStride );
      xPred8x8IMode3Plane( pucCr, iStride );
      rbValid = true;
      break;
    }
  default:
    {
      assert( 0 );
      return Err::m_nERR;
    }
  }

  return Err::m_nOK;
}





ErrVal IntraPredictionSearch::predictSLumaBlock8x8( IntYuvMbBuffer* pcYuvBuffer,
                                                    UInt            uiPredMode, 
                                                    B8x8Idx         c8x8Idx,
                                                    Bool&           rbValid )
{
  rbValid       = false;
  XPel* pPel    = pcYuvBuffer->getLumBlk  ();
  Int   iStride = pcYuvBuffer->getLStride ();

  xSet8x8AvailableMask( c8x8Idx );

  switch( uiPredMode & 0xf )
  {
  case 0:
    {
      if( xIsAboveRef() )
      {
        xPredLum8x8Mode0Vert( pPel, iStride );
        rbValid = true;
      }
      break;
    }
  case 1:
    {
      if( xIsLeftRef() )
      {
        xPredLum8x8Mode1Horiz( pPel, iStride );
        rbValid = true;
      }
      break;
    }
  case 2:
    {
      xPredLum8x8Mode2Dc( pPel, iStride );
      rbValid = true;
      break;
    }
  case 3:
    {
      if( xIsAboveRef() )
      {
        xPredLum8x8Mode3DiagDownLeft( pPel, iStride );
        rbValid = true;
      }
      break;
    }
  case 4:
    {
      if( xIsAllLeftAboveRef() )
      {
        xPredLum8x8Mode4DiagDownRight( pPel, iStride );
        rbValid = true;
      }
      break;
    }
  case 5:
    {
      if( xIsAllLeftAboveRef() )
      {
        xPredLum8x8Mode5VertRight( pPel, iStride );
        rbValid = true;
      }
      break;
    }
  case 6:
    {
      if( xIsAllLeftAboveRef() )
      {
        xPredLum8x8Mode6HorizDown( pPel, iStride );
        rbValid = true;
      }
      break;
    }
  case 7:
    {
      if( xIsAboveRef() )
      {
        xPredLum8x8Mode7VertLeft( pPel, iStride );
        rbValid = true;
      }
      break;
    }
  case 8:
    {
      if( xIsLeftRef() )
      {
        xPredLum8x8Mode8HorizUp( pPel, iStride );
        rbValid = true;
      }
      break;
    }
  default:
    {
      assert( 0 );
      return Err::m_nERR;
    }
  }

  return Err::m_nOK;
}


H264AVC_NAMESPACE_END
