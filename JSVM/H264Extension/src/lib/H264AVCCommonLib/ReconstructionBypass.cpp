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


#include "H264AVCCommonLib.h"
#include "H264AVCCommonLib/ReconstructionBypass.h"


#include "H264AVCCommonLib/Frame.h"
#include "H264AVCCommonLib/YuvMbBuffer.h"
#include "H264AVCCommonLib/MbDataCtrl.h"
#include "H264AVCCommonLib/YuvBufferCtrl.h"

H264AVC_NAMESPACE_BEGIN

ReconstructionBypass::ReconstructionBypass() 
{
}

ErrVal ReconstructionBypass::create( ReconstructionBypass*& rpcReconstructionBypass )
{
  rpcReconstructionBypass = new ReconstructionBypass;
  ROT( NULL == rpcReconstructionBypass ) ;
  return Err::m_nOK;
}

ErrVal ReconstructionBypass::destroy()
{
  delete this;
  return Err::m_nOK;
}

ErrVal ReconstructionBypass::init()
{
  return Err::m_nOK;
}

ErrVal ReconstructionBypass::uninit()
{
  return Err::m_nOK;
}
 
ErrVal
ReconstructionBypass::padRecFrame( Frame*             pcFrame, 
                                   const MbDataCtrl*  pcMbDataCtrl,
                                   ResizeParameters*  pcResizeParameters,
                                   UInt               uiSliceId /* = MSYS_UINT_MAX */ )
{
  ROF( pcFrame );
  ROF( pcMbDataCtrl );
  ROF( pcResizeParameters );

  RNOK( pcFrame->addFrameFieldBuffer() );

  UInt    uiFrmWidth  =   pcResizeParameters->m_iRefLayerFrmWidth  >> 4;
  UInt    uiFrmHeight =   pcResizeParameters->m_iRefLayerFrmHeight >> 4;
  Bool    bMbAffFrame =   pcResizeParameters->m_bRefLayerIsMbAffFrame;
  PicType ePicType    = ( pcResizeParameters->m_bRefLayerFieldPicFlag ? ( pcResizeParameters->m_bRefLayerBotFieldFlag ? BOT_FIELD : TOP_FIELD ) : FRAME );

  for( UInt uiMbY = 0; uiMbY < uiFrmHeight; uiMbY++ )
  for( UInt uiMbX = 0; uiMbX < uiFrmWidth;  uiMbX++ )
  {
    UInt  uiMask  = 0;
    Bool  bIntra  = false;
    RNOK( pcFrame->getFullPelYuvBuffer()->getYuvBufferCtrl().initMb( uiMbY, uiMbX, bMbAffFrame ) );

    if( ! bMbAffFrame )
    {
      RNOK( pcMbDataCtrl->getBoundaryMask( uiMbY, uiMbX, bIntra, uiMask, uiSliceId ) );
      if  ( ! bIntra )
      {
        YuvMbBufferExtension  cBuffer;
        YuvPicBuffer*         pcPicBuffer = pcFrame->getPic( ePicType )->getFullPelYuvBuffer();
        cBuffer.setAllSamplesToZero ();
        if( uiMask )
        {
          cBuffer.loadSurrounding   ( pcPicBuffer );
          RNOK( xPadRecMb           ( &cBuffer, uiMask ) );
        }
        pcPicBuffer->loadBuffer     ( &cBuffer );
      }
    }
    else
    {
      RNOK( pcMbDataCtrl->getBoundaryMask_MbAff( uiMbY, uiMbX, bIntra, uiMask, uiSliceId ) );
      if  ( ! bIntra )
      {
        YuvMbBufferExtension  cBuffer;
        PicType               eMbPicType  = ( uiMbY % 2 ? BOT_FIELD : TOP_FIELD );
        YuvPicBuffer*         pcPicBuffer = pcFrame->getPic( eMbPicType )->getFullPelYuvBuffer();
        cBuffer.setAllSamplesToZero     ();
        if( uiMask )
        {
          cBuffer.loadSurrounding_MbAff ( pcPicBuffer,  uiMask );
          RNOK( xPadRecMb_MbAff         ( &cBuffer,     uiMask ) );
        }
        pcPicBuffer->loadBuffer_MbAff   ( &cBuffer,     uiMask );
      }
    }
  }

  return Err::m_nOK;
}


ErrVal
ReconstructionBypass::xPadRecMb( YuvMbBufferExtension* pcBuffer, UInt uiMask )
{
  Bool bAboveIntra      = 0 != (uiMask & 0x01);
  Bool bBelowIntra      = 0 != (uiMask & 0x10);
  Bool bLeftIntra       = 0 != (uiMask & 0x40);
  Bool bRightIntra      = 0 != (uiMask & 0x04);
  Bool bLeftAboveIntra  = 0 != (uiMask & 0x80);
  Bool bRightAboveIntra = 0 != (uiMask & 0x02);
  Bool bLeftBelowIntra  = 0 != (uiMask & 0x20);
  Bool bRightBelowIntra = 0 != (uiMask & 0x08);

  for( B8x8Idx cIdx; cIdx.isLegal(); cIdx++ )
  {
    switch( cIdx.b8x8Index() )
    {
    case 0:
      {
        if( bAboveIntra )
        {
          if( bLeftIntra )
          {
            pcBuffer->mergeFromLeftAbove( cIdx, bLeftAboveIntra );
          }
          else
          {
            pcBuffer->copyFromAbove( cIdx );
          }
        }
        else
        {
          if( bLeftIntra )
          {
            pcBuffer->copyFromLeft( cIdx );
          }
          else if( bLeftAboveIntra )
          {
            pcBuffer->copyFromLeftAbove( cIdx );
          }
        }
      }
      break;
    case 1:
      {
        if( bAboveIntra )
        {
          if( bRightIntra )
          {
            pcBuffer->mergeFromRightAbove( cIdx, bRightAboveIntra );
          }
          else
          {
            pcBuffer->copyFromAbove( cIdx );
          }
        }
        else
        {
          if( bRightIntra )
          {
            pcBuffer->copyFromRight( cIdx );
          }
          else if( bRightAboveIntra )
          {
            pcBuffer->copyFromRightAbove( cIdx );
          }
        }
      }
      break;
    case 2:
      {
        if( bBelowIntra )
        {
          if( bLeftIntra )
          {
            pcBuffer->mergeFromLeftBelow( cIdx, bLeftBelowIntra );
          }
          else
          {
            pcBuffer->copyFromBelow( cIdx );
          }
        }
        else
        {
          if( bLeftIntra )
          {
            pcBuffer->copyFromLeft( cIdx );
          }
          else if( bLeftBelowIntra )
          {
            pcBuffer->copyFromLeftBelow( cIdx );
          }
        }
      }
      break;
    case 3:
      {
        if( bBelowIntra )
        {
          if( bRightIntra )
          {
            pcBuffer->mergeFromRightBelow( cIdx, bRightBelowIntra );
          }
          else
          {
            pcBuffer->copyFromBelow( cIdx );
          }
        }
        else
        {
          if( bRightIntra )
          {
            pcBuffer->copyFromRight( cIdx );
          }
          else if( bRightBelowIntra )
          {
            pcBuffer->copyFromRightBelow( cIdx );
          }
        }
      }
      break;
    default:
      break;
    }
  }

  return Err::m_nOK;
}


ErrVal
ReconstructionBypass::xPadRecMb_MbAff( YuvMbBufferExtension* pcBuffer, UInt uiMask )
{
  Bool bAvailableTopLeft  = ( ( uiMask & 0x001 ) != 0 );
  Bool bAvailableTop      = ( ( uiMask & 0x002 ) != 0 );
  Bool bAvailableTopRight = ( ( uiMask & 0x004 ) != 0 );
  Bool bAvailableLeftTop  = ( ( uiMask & 0x008 ) != 0 );
  Bool bAvailableLeftBot  = ( ( uiMask & 0x010 ) != 0 );
  Bool bAvailableCurrTop  = ( ( uiMask & 0x020 ) != 0 );
  Bool bAvailableCurrBot  = ( ( uiMask & 0x040 ) != 0 );
  Bool bAvailableRightTop = ( ( uiMask & 0x080 ) != 0 );
  Bool bAvailableRightBot = ( ( uiMask & 0x100 ) != 0 );
  Bool bAvailableBotLeft  = ( ( uiMask & 0x200 ) != 0 );
  Bool bAvailableBot      = ( ( uiMask & 0x400 ) != 0 );
  Bool bAvailableBotRight = ( ( uiMask & 0x800 ) != 0 );

  //===== TOP-LEFT 8x8 block =====
  if( ! bAvailableCurrTop )
  {
    Bool  bV0 = bAvailableTop;
    Bool  bV1 = bAvailableCurrBot;
    Bool  bH  = bAvailableLeftTop;
    Bool  bC0 = bAvailableTopLeft;
    Bool  bC1 = bAvailableLeftBot;
    RNOK( xPad8x8Blk_MbAff( pcBuffer, 0, bV0, bV1, bH, bC0, bC1 ) );
  }

  //===== TOP-RIGHT 8x8 block =====
  if( ! bAvailableCurrTop )
  {
    Bool  bV0 = bAvailableTop;
    Bool  bV1 = bAvailableCurrBot;
    Bool  bH  = bAvailableRightTop;
    Bool  bC0 = bAvailableTopRight;
    Bool  bC1 = bAvailableRightBot;
    RNOK( xPad8x8Blk_MbAff( pcBuffer, 1, bV0, bV1, bH, bC0, bC1 ) );
  }

  //===== BOTTOM-LEFT 8x8 block =====
  if( ! bAvailableCurrBot )
  {
    Bool  bV0 = bAvailableBot;
    Bool  bV1 = bAvailableCurrTop;
    Bool  bH  = bAvailableLeftBot;
    Bool  bC0 = bAvailableBotLeft;
    Bool  bC1 = bAvailableLeftTop;
    RNOK( xPad8x8Blk_MbAff( pcBuffer, 2, bV0, bV1, bH, bC0, bC1 ) );
  }

  //===== BOTTOM-RIGHT 8x8 block =====
  if( ! bAvailableCurrBot )
  {
    Bool  bV0 = bAvailableBot;
    Bool  bV1 = bAvailableCurrTop;
    Bool  bH  = bAvailableRightBot;
    Bool  bC0 = bAvailableBotRight;
    Bool  bC1 = bAvailableRightTop;
    RNOK( xPad8x8Blk_MbAff( pcBuffer, 3, bV0, bV1, bH, bC0, bC1 ) );
  }

  return Err::m_nOK;
}

ErrVal
ReconstructionBypass::xPad8x8Blk_MbAff( YuvMbBufferExtension* pcBuffer, UInt ui8x8Blk, Bool bV0, Bool bV1, Bool bH, Bool bC0, Bool bC1 )
{
  Bool    bSwitch     = ( !bV0 && bV1 && bH ) || ( !bV0 && !bH && !bC0 && ( bV1 || bC1 ) );
  Bool    bDouble     = ( bV0 && bV1 ) || ( ( bV0 || bC0 ) && !bH && ( bV1 || bC1 ) );
  ROT( bSwitch && bDouble );
  Bool    bFromAbove  = ( ui8x8Blk < 2 );
  Bool    bFromLeft   = ( ui8x8Blk % 2 == 0 );
  B8x8Idx cIdx( (Par8x8)ui8x8Blk );

  if( bDouble )
  {
    RNOK( xPadBlock_MbAff( pcBuffer, cIdx, bV0, bH, bC0, true,   bFromAbove, bFromLeft ) );
    RNOK( xPadBlock_MbAff( pcBuffer, cIdx, bV1, bH, bC1, true,  !bFromAbove, bFromLeft ) );
  }
  else if( bSwitch )
  {
    RNOK( xPadBlock_MbAff( pcBuffer, cIdx, bV1, bH, bC1, false, !bFromAbove, bFromLeft ) );
  }
  else
  {
    RNOK( xPadBlock_MbAff( pcBuffer, cIdx, bV0, bH, bC0, false,  bFromAbove, bFromLeft ) );
  }

  return Err::m_nOK;
}

ErrVal
ReconstructionBypass::xPadBlock_MbAff( YuvMbBufferExtension* pcBuffer, LumaIdx cIdx, Bool bVer, Bool bHor, Bool bCorner, Bool bHalfYSize, Bool bFromAbove, Bool bFromLeft )
{
  if( bVer && bHor )
  {
    if( bFromAbove && bFromLeft )
    {
      pcBuffer->mergeFromLeftAbove ( cIdx, bCorner, bHalfYSize );
    }
    else if( bFromAbove )
    {
      pcBuffer->mergeFromRightAbove( cIdx, bCorner, bHalfYSize );
    }
    else if( bFromLeft )
    {
      pcBuffer->mergeFromLeftBelow ( cIdx, bCorner, bHalfYSize );
    }
    else
    {
      pcBuffer->mergeFromRightBelow( cIdx, bCorner, bHalfYSize );
    }
  }
  else if( bVer )
  {
    if( bFromAbove )
    {
      pcBuffer->copyFromAbove( cIdx, bHalfYSize );
    }
    else
    {
      pcBuffer->copyFromBelow( cIdx, bHalfYSize );
    }
  }
  else if( bHor )
  {
    ROT( bHalfYSize );
    if( bFromLeft )
    {
      pcBuffer->copyFromLeft ( cIdx );
    }
    else
    {
      pcBuffer->copyFromRight( cIdx );
    }
  }
  else if( bCorner )
  {
    if( bFromAbove && bFromLeft )
    {
      pcBuffer->copyFromLeftAbove ( cIdx, bHalfYSize );
    }
    else if( bFromAbove )
    {
      pcBuffer->copyFromRightAbove( cIdx, bHalfYSize );
    }
    else if( bFromLeft )
    {
      pcBuffer->copyFromLeftBelow ( cIdx, bHalfYSize );
    }
    else
    {
      pcBuffer->copyFromRightBelow( cIdx, bHalfYSize );
    }
  }
  else
  {
    ROT( bHalfYSize );
  }
  return Err::m_nOK;
}


H264AVC_NAMESPACE_END




