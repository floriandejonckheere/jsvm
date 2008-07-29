
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




