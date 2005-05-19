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

#include "H264AVCCommonLib/SampleWeighting.h"
#include "H264AVCCommonLib/QuarterPelFilter.h"
#include "H264AVCCommonLib/FrameMng.h"
#include "H264AVCCommonLib/Frame.h"
#include "H264AVCCommonLib/IntFrame.h"
#include "H264AVCCommonLib/MotionVectorCalculation.h"
#include "H264AVCCommonLib/MotionCompensation.h"


H264AVC_NAMESPACE_BEGIN


__inline Void MotionCompensation::xPredChromaPel( Pel* pucDest, Int iDestStride, Pel* pucSrc, Int iSrcStride, Mv cMv, Int iSizeY, Int iSizeX )
{
  Int xF1 = cMv.getHor();
  Int yF1 = cMv.getVer();

  xF1 &= 0x7;
  yF1 &= 0x7;

  Int x, y;
  if( 0 == xF1 )
  {
    if( 0 == yF1 )
    {
      if( iSizeX == 2 )
      {
        for( y = 0; y < iSizeY; y++ )
        {
          *((UShort*)pucDest) = *((UShort*)pucSrc);
          pucDest += iDestStride;
          pucSrc  += iSrcStride;
        }
        return;
      }
      if( iSizeX == 4 )
      {
        for( y = 0; y < iSizeY; y++ )
        {
          *((UInt*)pucDest) = *((UInt*)pucSrc);
          pucDest += iDestStride;
          pucSrc  += iSrcStride;
        }
        return;
      }

      for( y = 0; y < iSizeY; y++ )
      {
        ((UInt*)pucDest)[0] = ((UInt*)pucSrc)[0];
        ((UInt*)pucDest)[1] = ((UInt*)pucSrc)[1];
        pucDest += iDestStride;
        pucSrc  += iSrcStride;
      }
      return;
    }

    Int yF0 = 8 - yF1;

    for( y = 0; y < iSizeY; y++ )
    {
      for( x = 0; x < iSizeX; x++ )
      {
        pucDest[x  ] = (yF0 * pucSrc[x  ] + yF1 * pucSrc[x+iSrcStride  ] + 4) >> 3;
      }
      pucDest += iDestStride;
      pucSrc  += iSrcStride;
    }
    return;
  }


  if( 0 == yF1 )
  {
    Int xF0 = 8 - xF1;

    for( y = 0; y < iSizeY; y++ )
    {
      for( x = 0; x < iSizeX; x++ )
      {
        pucDest[x] = (xF0 * pucSrc[x] + xF1 * pucSrc[x+1] + 4) >> 3;
      }
      pucDest += iDestStride;
      pucSrc  += iSrcStride;
    }
    return;
  }

  Int xF0 = 8 - xF1;
  Int yF0 = 8 - yF1;
  for( y = 0; y < iSizeY; y++ )
  {
    Int a = xF0* ( yF0 * pucSrc[0] + yF1 * pucSrc[iSrcStride]);
    for( x = 0; x < iSizeX; x++ )
    {
      Int b = yF0 * pucSrc[x+1] + yF1 * pucSrc[iSrcStride+x+1];
      Int c = xF1 * b;
      pucDest[x]   = (a + c + 0x20) >> 6;
      a = (b<<3) - c;
    }
    pucDest += iDestStride;
    pucSrc  += iSrcStride;
  }
}




MotionCompensation::MotionCompensation():
  m_pcQuarterPelFilter( NULL ),
  m_pcSampleWeighting( NULL ),
  m_uiMbInFrameY( 0 ),
  m_uiMbInFrameX( 0 )
{
}


MotionCompensation::~MotionCompensation()
{
}


ErrVal MotionCompensation::create( MotionCompensation*& rpcMotionCompensation )
{
  rpcMotionCompensation = new MotionCompensation;

  ROT( NULL == rpcMotionCompensation );

  return Err::m_nOK;
}


ErrVal MotionCompensation::destroy()
{
  delete this;

  return Err::m_nOK;
}



ErrVal MotionCompensation::init( QuarterPelFilter* pcQuarterPelFilter,
                                 SampleWeighting* pcSampleWeighting )
{
  ROT( NULL == pcQuarterPelFilter );
  ROT( NULL == pcSampleWeighting );

  setMotCompType(PRED_MC);   // default to PRED_MC

  m_pcQuarterPelFilter  = pcQuarterPelFilter;
  m_pcSampleWeighting   = pcSampleWeighting;

  return Err::m_nOK;
}


ErrVal MotionCompensation::initSlice( const SliceHeader& rcSH )
{
  m_uiMbInFrameY = rcSH.getSPS().getFrameHeightInMbs();
  m_uiMbInFrameX = rcSH.getSPS().getFrameWidthInMbs();

  RNOK( MotionVectorCalculation::initSlice( rcSH ) );

  return Err::m_nOK;
}


ErrVal MotionCompensation::uninit()
{
  RNOK( MotionVectorCalculation::uninit() );

  m_pcQuarterPelFilter  = NULL;
  m_pcSampleWeighting   = NULL;

  return Err::m_nOK;
}


Void MotionCompensation::xGetMbPredData( MbDataAccess& rcMbDataAccess, MC8x8D& rcMC8x8D )
{
  rcMC8x8D.clear();

  const SliceHeader& rcSH = rcMbDataAccess.getSH();

  Int nMax = rcMbDataAccess.getSH().isInterB() ? 2:1;
  Int iPredCount = 0;

  for( Int n = 0; n < nMax; n++)
  {
    ListIdx eLstIdx = ListIdx( n );
    Mv3D& rcMv3D = rcMC8x8D.m_aacMv[eLstIdx][0];
    rcMbDataAccess.getMbMotionData( eLstIdx ).getMv3D( rcMv3D, rcMC8x8D.m_cIdx );

    if( BLOCK_NOT_PREDICTED != rcMv3D.getRef() )
    {
      rcMv3D.limitComponents( m_cMin, m_cMax );

      const Frame* pcRefFrame = rcSH.getRefPic( rcMv3D.getRef(), eLstIdx ).getFrame();

      rcMC8x8D.m_apcRefBuffer[eLstIdx]  = const_cast<Frame*>(pcRefFrame)->getFullPelYuvBuffer();
      iPredCount++;
    }
  }
  rcMC8x8D.m_bBi = ( iPredCount == 2 );
}




__inline Void MotionCompensation::xGetBlkPredData( MbDataAccess& rcMbDataAccess, MC8x8D& rcMC8x8D, BlkMode eBlkMode )
{
  rcMC8x8D.clear();

  const SliceHeader& rcSH = rcMbDataAccess.getSH();

  Int nMax = rcMbDataAccess.getSH().isInterB() ? 2:1;
  Int iPredCount = 0;

  for( Int n = 0; n < nMax; n++)
  {
    ListIdx eLstIdx = ListIdx( n );
    Mv3D& rcMv3D = rcMC8x8D.m_aacMv[eLstIdx][SPART_4x4_0];
    const MbMotionData& rcMbMotionData = rcMbDataAccess.getMbMotionData( eLstIdx );

    rcMbMotionData.getMv3D( rcMv3D, rcMC8x8D.m_cIdx + SPART_4x4_0 );
    rcMv3D.limitComponents( m_cMin, m_cMax );

    if( BLOCK_NOT_PREDICTED != rcMv3D.getRef() )
    {
      iPredCount++;
      const Frame* pcRefFrame = rcSH.getRefPic( rcMv3D.getRef(), eLstIdx ).getFrame();

      rcMC8x8D.m_apcRefBuffer[eLstIdx]  = const_cast<Frame*>(pcRefFrame)->getFullPelYuvBuffer();

      switch( eBlkMode )
      {
      case BLK_8x8:
        break;
      case BLK_8x4:
        {
          rcMC8x8D.m_aacMv[eLstIdx][SPART_4x4_2] = rcMbMotionData.getMv( rcMC8x8D.m_cIdx + SPART_4x4_2 );
          rcMC8x8D.m_aacMv[eLstIdx][SPART_4x4_2].limitComponents( m_cMin, m_cMax );
        }
        break;
      case BLK_4x8:
        {
          rcMC8x8D.m_aacMv[eLstIdx][SPART_4x4_1] = rcMbMotionData.getMv( rcMC8x8D.m_cIdx + SPART_4x4_1 );
          rcMC8x8D.m_aacMv[eLstIdx][SPART_4x4_1].limitComponents( m_cMin, m_cMax );
        }
        break;
      case BLK_SKIP:
      case BLK_4x4:
        {
          rcMC8x8D.m_aacMv[eLstIdx][SPART_4x4_1] = rcMbMotionData.getMv( rcMC8x8D.m_cIdx + SPART_4x4_1 );
          rcMC8x8D.m_aacMv[eLstIdx][SPART_4x4_1].limitComponents( m_cMin, m_cMax );

          rcMC8x8D.m_aacMv[eLstIdx][SPART_4x4_2] = rcMbMotionData.getMv( rcMC8x8D.m_cIdx + SPART_4x4_2 );
          rcMC8x8D.m_aacMv[eLstIdx][SPART_4x4_2].limitComponents( m_cMin, m_cMax );

          rcMC8x8D.m_aacMv[eLstIdx][SPART_4x4_3] = rcMbMotionData.getMv( rcMC8x8D.m_cIdx + SPART_4x4_3 );
          rcMC8x8D.m_aacMv[eLstIdx][SPART_4x4_3].limitComponents( m_cMin, m_cMax );
        }
        break;
      default:
        {
          AOT(1);
        }
      }
    }
  }
  rcMC8x8D.m_bBi = ( iPredCount == 2 );
}



Void MotionCompensation::xPredLuma( YuvMbBuffer* apcTarBuffer[2], Int iSizeX, Int iSizeY, MC8x8D& rcMc8x8D, SParIdx4x4 eSParIdx )
{
  B4x4Idx cIdx( rcMc8x8D.m_cIdx + eSParIdx );

  for( Int n = 0; n < 2; n++ )
  {
    YuvPicBuffer* pcRefBuffer = rcMc8x8D.m_apcRefBuffer[n];
    if( NULL != pcRefBuffer )
    {
      const Mv cMv = rcMc8x8D.m_aacMv[n][eSParIdx];
      m_pcQuarterPelFilter->predBlk( apcTarBuffer[n], pcRefBuffer, cIdx, cMv, iSizeY, iSizeX );
    }
  }
}



Void MotionCompensation::xPredChroma( YuvMbBuffer* apcTarBuffer[2], Int iSizeX, Int iSizeY, MC8x8D& rcMc8x8D, SParIdx4x4 eSParIdx )
{
  B4x4Idx cIdx( rcMc8x8D.m_cIdx + eSParIdx );

  for( Int n = 0; n < 2; n++ )
  {
    YuvPicBuffer* pcRefBuffer = rcMc8x8D.m_apcRefBuffer[n];
    if( NULL != pcRefBuffer )
    {
      Mv cMv = rcMc8x8D.m_aacMv[n][eSParIdx];

      xPredChroma( apcTarBuffer[n], pcRefBuffer, cIdx, cMv, iSizeY, iSizeX );
    }
  }
}


Void MotionCompensation::xPredLuma( YuvMbBuffer* pcRecBuffer, Int iSizeX, Int iSizeY, MC8x8D& rcMc8x8D )
{
  YuvMbBuffer* apcTarBuffer[2];
  m_pcSampleWeighting->getTargetBuffers( apcTarBuffer, pcRecBuffer, rcMc8x8D.m_bBi );

  for( Int n = 0; n < 2; n++ )
  {
    YuvPicBuffer* pcRefBuffer = rcMc8x8D.m_apcRefBuffer[n];
    if( NULL != pcRefBuffer )
    {
      const Mv cMv = rcMc8x8D.m_aacMv[n][0];
      m_pcQuarterPelFilter->predBlk( apcTarBuffer[n], pcRefBuffer, rcMc8x8D.m_cIdx, cMv, iSizeY, iSizeX );
    }
  }
  m_pcSampleWeighting->weightLumaSamples( pcRecBuffer, iSizeX, iSizeY, rcMc8x8D.m_cIdx, rcMc8x8D.m_bBi );
}




Void MotionCompensation::xPredChroma( YuvMbBuffer* pcRecBuffer, Int iSizeX, Int iSizeY, MC8x8D& rcMc8x8D )
{
  YuvMbBuffer* apcTarBuffer[2];
  m_pcSampleWeighting->getTargetBuffers( apcTarBuffer, pcRecBuffer, rcMc8x8D.m_bBi );

  for( Int n = 0; n < 2; n++ )
  {
    YuvPicBuffer* pcRefBuffer = rcMc8x8D.m_apcRefBuffer[n];
    if( NULL != pcRefBuffer )
    {
      Mv cMv = rcMc8x8D.m_aacMv[n][0];
      xPredChroma( apcTarBuffer[n], pcRefBuffer, rcMc8x8D.m_cIdx, cMv, iSizeY, iSizeX );
    }
  }
  m_pcSampleWeighting->weightChromaSamples( pcRecBuffer, iSizeX, iSizeY, rcMc8x8D.m_cIdx, rcMc8x8D.m_bBi );
}


ErrVal MotionCompensation::compensateMb( MbDataAccess& rcMbDataAccess, YuvMbBuffer* pcRecBuffer, Bool bFaultTolerant )
{
  MbMode eMbMode  = rcMbDataAccess.getMbData().getMbMode();

  switch( eMbMode )
  {
  case MODE_16x16:
    {
      xCalc16x16( rcMbDataAccess, NULL );

      MC8x8D cMC8x8D( B_8x8_0 );
      xGetMbPredData( rcMbDataAccess, cMC8x8D );
      xPredLuma(   pcRecBuffer, 16, 16, cMC8x8D );
      xPredChroma( pcRecBuffer,  8,  8, cMC8x8D );
    }
    break;
  case MODE_16x8:
    {
      xCalc16x8( rcMbDataAccess, NULL );

      MC8x8D cMC8x8D0( B_8x8_0 );
      MC8x8D cMC8x8D1( B_8x8_2 );

      xGetMbPredData( rcMbDataAccess, cMC8x8D0 );
      xGetMbPredData( rcMbDataAccess, cMC8x8D1 );

      xPredLuma(   pcRecBuffer, 16, 8, cMC8x8D0 );
      xPredLuma(   pcRecBuffer, 16, 8, cMC8x8D1 );
      xPredChroma( pcRecBuffer,  8, 4, cMC8x8D0 );
      xPredChroma( pcRecBuffer,  8, 4, cMC8x8D1 );
    }
    break;
  case MODE_8x16:
    {
      xCalc8x16( rcMbDataAccess, NULL );

      MC8x8D cMC8x8D0( B_8x8_0 );
      MC8x8D cMC8x8D1( B_8x8_1 );

      xGetMbPredData( rcMbDataAccess, cMC8x8D0 );
      xGetMbPredData( rcMbDataAccess, cMC8x8D1 );

      xPredLuma(   pcRecBuffer, 8, 16, cMC8x8D0 );
      xPredLuma(   pcRecBuffer, 8, 16, cMC8x8D1 );
      xPredChroma( pcRecBuffer, 4,  8, cMC8x8D0 );
      xPredChroma( pcRecBuffer, 4,  8, cMC8x8D1 );
    }
    break;
  case MODE_SKIP:
    {
      if( rcMbDataAccess.getSH().isInterB() )
      {
        Bool bValid;
        B8x8Idx c8x8Idx;
        RNOK( compensateDirectBlock( rcMbDataAccess, pcRecBuffer, c8x8Idx, bValid, bFaultTolerant ) ); c8x8Idx++;
        ROF ( bValid )
        RNOK( compensateDirectBlock( rcMbDataAccess, pcRecBuffer, c8x8Idx, bValid, bFaultTolerant ) ); c8x8Idx++;
        ROF ( bValid )
        RNOK( compensateDirectBlock( rcMbDataAccess, pcRecBuffer, c8x8Idx, bValid, bFaultTolerant ) ); c8x8Idx++;
        ROF ( bValid )
        RNOK( compensateDirectBlock( rcMbDataAccess, pcRecBuffer, c8x8Idx, bValid, bFaultTolerant ) );
        ROF ( bValid )
      }
      else
      {
        rcMbDataAccess.getMvPredictorSkipMode();

        MC8x8D cMC8x8D( B_8x8_0 );
        xGetMbPredData( rcMbDataAccess, cMC8x8D );

        xPredLuma(   pcRecBuffer, 16, 16, cMC8x8D );
        xPredChroma( pcRecBuffer,  8,  8, cMC8x8D );
        return Err::m_nOK;
      }
    }
    break;
  case MODE_8x8:
    {
      xCalc8x8( rcMbDataAccess, NULL, bFaultTolerant );
      xPredMb8x8Mode( rcMbDataAccess, pcRecBuffer );
    }
    break;
  default:
    break;
  }

  return Err::m_nOK;
}


ErrVal MotionCompensation::calculateMb( MbDataAccess& rcMbDataAccess, YuvMbBuffer* pcRecBuffer, Bool bFaultTolerant )
{
  MbMode eMbMode  = rcMbDataAccess.getMbData().getMbMode();

  switch( eMbMode )
  {
  case MODE_16x16:
    {
      xCalc16x16( rcMbDataAccess, NULL );
    }
    break;
  case MODE_16x8:
    {
      xCalc16x8( rcMbDataAccess, NULL );
    }
    break;
  case MODE_8x16:
    {
      xCalc8x16( rcMbDataAccess, NULL );
    }
    break;
  case MODE_SKIP:
    {
      if( rcMbDataAccess.getSH().isInterB() )
      {
        B8x8Idx c8x8Idx;

        Bool bOneMv;
        AOF( rcMbDataAccess.getMvPredictorDirect( c8x8Idx.b8x8(), bOneMv, bFaultTolerant ) ); c8x8Idx++;
        AOF( rcMbDataAccess.getMvPredictorDirect( c8x8Idx.b8x8(), bOneMv, bFaultTolerant ) ); c8x8Idx++;
        AOF( rcMbDataAccess.getMvPredictorDirect( c8x8Idx.b8x8(), bOneMv, bFaultTolerant ) ); c8x8Idx++;
        AOF( rcMbDataAccess.getMvPredictorDirect( c8x8Idx.b8x8(), bOneMv, bFaultTolerant ) ); 
      }
      else
      {
        rcMbDataAccess.getMvPredictorSkipMode();

        return Err::m_nOK;
      }
    }
    break;
  case MODE_8x8:
    {
      xCalc8x8( rcMbDataAccess, NULL, bFaultTolerant );
    }
    break;
  default:
    break;
  }

  return Err::m_nOK;
}


Void MotionCompensation::xPredMb8x8Mode( MbDataAccess& rcMbDataAccess, YuvMbBuffer* pcRecBuffer )
{
  for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++)
  {
    YuvMbBuffer* apcTarBuffer[2];
    Par8x8  ePar8x8   = c8x8Idx.b8x8Index();
    BlkMode eBlkMode  = rcMbDataAccess.getMbData().getBlkMode( ePar8x8 );

    MC8x8D cMC8x8D( ePar8x8 );
    xGetBlkPredData( rcMbDataAccess, cMC8x8D, eBlkMode );

    m_pcSampleWeighting->getTargetBuffers( apcTarBuffer, pcRecBuffer, cMC8x8D.m_bBi );

    switch( eBlkMode )
    {
      case BLK_SKIP:
      {
        if( rcMbDataAccess.getSH().getSPS().getDirect8x8InferenceFlag() )
        {
          xPredLuma(   apcTarBuffer, 8, 8, cMC8x8D, SPART_4x4_0 );
          xPredChroma( apcTarBuffer, 4, 4, cMC8x8D, SPART_4x4_0 );
        }
        else
        {
          xPredLuma(   apcTarBuffer, 4, 4, cMC8x8D, SPART_4x4_0 );
          xPredLuma(   apcTarBuffer, 4, 4, cMC8x8D, SPART_4x4_1 );
          xPredLuma(   apcTarBuffer, 4, 4, cMC8x8D, SPART_4x4_2 );
          xPredLuma(   apcTarBuffer, 4, 4, cMC8x8D, SPART_4x4_3 );
          xPredChroma( apcTarBuffer, 2, 2, cMC8x8D, SPART_4x4_0 );
          xPredChroma( apcTarBuffer, 2, 2, cMC8x8D, SPART_4x4_1 );
          xPredChroma( apcTarBuffer, 2, 2, cMC8x8D, SPART_4x4_2 );
          xPredChroma( apcTarBuffer, 2, 2, cMC8x8D, SPART_4x4_3 );
        }
        break;
      }
      case BLK_8x8:
      {
        xPredLuma(   apcTarBuffer, 8, 8, cMC8x8D, SPART_4x4_0 );
        xPredChroma( apcTarBuffer, 4, 4, cMC8x8D, SPART_4x4_0 );
        break;
      }
      case BLK_8x4:
      {
        xPredLuma(   apcTarBuffer, 8, 4, cMC8x8D, SPART_4x4_0 );
        xPredLuma(   apcTarBuffer, 8, 4, cMC8x8D, SPART_4x4_2 );
        xPredChroma( apcTarBuffer, 4, 2, cMC8x8D, SPART_4x4_0 );
        xPredChroma( apcTarBuffer, 4, 2, cMC8x8D, SPART_4x4_2 );

        break;
      }
      case BLK_4x8:
      {
        xPredLuma(   apcTarBuffer, 4, 8, cMC8x8D, SPART_4x4_0 );
        xPredLuma(   apcTarBuffer, 4, 8, cMC8x8D, SPART_4x4_1 );
        xPredChroma( apcTarBuffer, 2, 4, cMC8x8D, SPART_4x4_0 );
        xPredChroma( apcTarBuffer, 2, 4, cMC8x8D, SPART_4x4_1 );

        break;
      }
      case BLK_4x4:
      {
        xPredLuma(   apcTarBuffer, 4, 4, cMC8x8D, SPART_4x4_0 );
        xPredLuma(   apcTarBuffer, 4, 4, cMC8x8D, SPART_4x4_1 );
        xPredLuma(   apcTarBuffer, 4, 4, cMC8x8D, SPART_4x4_2 );
        xPredLuma(   apcTarBuffer, 4, 4, cMC8x8D, SPART_4x4_3 );
        xPredChroma( apcTarBuffer, 2, 2, cMC8x8D, SPART_4x4_0 );
        xPredChroma( apcTarBuffer, 2, 2, cMC8x8D, SPART_4x4_1 );
        xPredChroma( apcTarBuffer, 2, 2, cMC8x8D, SPART_4x4_2 );
        xPredChroma( apcTarBuffer, 2, 2, cMC8x8D, SPART_4x4_3 );
        break;
      }
      default:
      {
        AOT(1);
        break;
      }
    }

    m_pcSampleWeighting->weightChromaSamples( pcRecBuffer, 4, 4, c8x8Idx, cMC8x8D.m_bBi );
    m_pcSampleWeighting->weightLumaSamples  ( pcRecBuffer, 8, 8, c8x8Idx, cMC8x8D.m_bBi );
  }
}


ErrVal MotionCompensation::compensateDirectBlock( MbDataAccess& rcMbDataAccess, YuvMbBuffer *pcRecBuffer, B8x8Idx c8x8Idx, Bool& rbValid, Bool bFaultTolerant )
{
  rbValid = false;
  Bool bOneMv = false;
  ROFRS( rcMbDataAccess.getMvPredictorDirect( c8x8Idx.b8x8(), bOneMv, bFaultTolerant ), Err::m_nOK );

  MC8x8D          cMC8x8D( c8x8Idx.b8x8Index() );
  YuvMbBuffer*  apcTarBuffer[2];

  if( bOneMv )
  {
    xGetBlkPredData( rcMbDataAccess, cMC8x8D, BLK_8x8 );
    m_pcSampleWeighting->getTargetBuffers( apcTarBuffer, pcRecBuffer, cMC8x8D.m_bBi );

    xPredLuma(   apcTarBuffer, 8, 8, cMC8x8D, SPART_4x4_0 );
    xPredChroma( apcTarBuffer, 4, 4, cMC8x8D, SPART_4x4_0 );
  }
  else
  {
    xGetBlkPredData( rcMbDataAccess, cMC8x8D, BLK_4x4 );
    m_pcSampleWeighting->getTargetBuffers( apcTarBuffer, pcRecBuffer, cMC8x8D.m_bBi );

    xPredLuma(   apcTarBuffer, 4, 4, cMC8x8D, SPART_4x4_0 );
    xPredLuma(   apcTarBuffer, 4, 4, cMC8x8D, SPART_4x4_1 );
    xPredLuma(   apcTarBuffer, 4, 4, cMC8x8D, SPART_4x4_2 );
    xPredLuma(   apcTarBuffer, 4, 4, cMC8x8D, SPART_4x4_3 );
    xPredChroma( apcTarBuffer, 2, 2, cMC8x8D, SPART_4x4_0 );
    xPredChroma( apcTarBuffer, 2, 2, cMC8x8D, SPART_4x4_1 );
    xPredChroma( apcTarBuffer, 2, 2, cMC8x8D, SPART_4x4_2 );
    xPredChroma( apcTarBuffer, 2, 2, cMC8x8D, SPART_4x4_3 );

  }

  m_pcSampleWeighting->weightChromaSamples( pcRecBuffer, 4, 4, c8x8Idx, cMC8x8D.m_bBi );
  m_pcSampleWeighting->weightLumaSamples  ( pcRecBuffer, 8, 8, c8x8Idx, cMC8x8D.m_bBi );

  rbValid = true;
  return Err::m_nOK;
}


ErrVal MotionCompensation::compensateDirectBlock( MbDataAccess& rcMbDataAccess, IntYuvMbBuffer *pcRecBuffer, B8x8Idx c8x8Idx, RefFrameList& rcRefFrameListL0, RefFrameList& rcRefFrameListL1 )
{
  Par8x8 ePar8x8 = c8x8Idx.b8x8Index();
  IntMC8x8D          cMC8x8D( ePar8x8 );
  IntYuvMbBuffer*  apcTarBuffer[2];

  Int       iRefIdx0    = rcMbDataAccess.getMbMotionData( LIST_0 ).getRefIdx( ePar8x8 );
  Int       iRefIdx1    = rcMbDataAccess.getMbMotionData( LIST_1 ).getRefIdx( ePar8x8 );
  IntFrame* pcRefFrame0 = ( iRefIdx0 > 0 ? rcRefFrameListL0[ iRefIdx0 ] : NULL );
  IntFrame* pcRefFrame1 = ( iRefIdx1 > 0 ? rcRefFrameListL1[ iRefIdx1 ] : NULL );

  if( rcMbDataAccess.getSH().getSPS().getDirect8x8InferenceFlag() )
  {
    xGetBlkPredData( rcMbDataAccess, pcRefFrame0, pcRefFrame1, cMC8x8D, BLK_8x8 );
    m_pcSampleWeighting->getTargetBuffers( apcTarBuffer, pcRecBuffer, cMC8x8D.m_bBi );

    xPredLuma(   apcTarBuffer, 8, 8, cMC8x8D, SPART_4x4_0 );
    xPredChroma( apcTarBuffer, 4, 4, cMC8x8D, SPART_4x4_0 );
  }
  else
  {
    xGetBlkPredData( rcMbDataAccess, pcRefFrame0, pcRefFrame1, cMC8x8D, BLK_4x4 );
    m_pcSampleWeighting->getTargetBuffers( apcTarBuffer, pcRecBuffer, cMC8x8D.m_bBi );

    xPredLuma(   apcTarBuffer, 4, 4, cMC8x8D, SPART_4x4_0 );
    xPredLuma(   apcTarBuffer, 4, 4, cMC8x8D, SPART_4x4_1 );
    xPredLuma(   apcTarBuffer, 4, 4, cMC8x8D, SPART_4x4_2 );
    xPredLuma(   apcTarBuffer, 4, 4, cMC8x8D, SPART_4x4_3 );
    xPredChroma( apcTarBuffer, 2, 2, cMC8x8D, SPART_4x4_0 );
    xPredChroma( apcTarBuffer, 2, 2, cMC8x8D, SPART_4x4_1 );
    xPredChroma( apcTarBuffer, 2, 2, cMC8x8D, SPART_4x4_2 );
    xPredChroma( apcTarBuffer, 2, 2, cMC8x8D, SPART_4x4_3 );

  }

  m_pcSampleWeighting->weightChromaSamples( pcRecBuffer, 4, 4, c8x8Idx, cMC8x8D.m_bBi );
  m_pcSampleWeighting->weightLumaSamples  ( pcRecBuffer, 8, 8, c8x8Idx, cMC8x8D.m_bBi );

  return Err::m_nOK;
}




ErrVal MotionCompensation::initMb( UInt uiMbY, UInt uiMbX, MbDataAccess& rcMbDataAccess )
{
  UInt uiMbInFrameY = m_uiMbInFrameY;

  m_cMin.setHor( (Short) max( (Int)MSYS_SHORT_MIN, (Int)((-uiMbX << 4) - (16+8) ) << 2 ) );
  m_cMin.setVer( (Short) max( (Int)MSYS_SHORT_MIN, (Int)((-uiMbY << 4) - (16+8) ) << 2 ) );

  m_cMax.setHor( (Short) min( (Int)MSYS_SHORT_MAX, (Int)(((m_uiMbInFrameX - uiMbX) << 4) + 8 ) << 2 ) );
  m_cMax.setVer( (Short) min( (Int)MSYS_SHORT_MAX, (Int)(((  uiMbInFrameY - uiMbY) << 4) + 8 ) << 2 ) );

  return Err::m_nOK;
}


__inline Void MotionCompensation::xPredChroma( YuvMbBuffer* pcDesBuffer, YuvPicBuffer* pcSrcBuffer, LumaIdx cIdx, Mv cMv, Int iSizeY, Int iSizeX)
{
  const Int iDesStride  = pcDesBuffer->getCStride();
  const Int iSrcStride  = pcSrcBuffer->getCStride();

  cMv.limitComponents( m_cMin, m_cMax );

  const Int iOffset = (cMv.getHor() >> 3) + (cMv.getVer() >> 3) * iSrcStride;

  xPredChromaPel( pcDesBuffer->getUBlk( cIdx ),          iDesStride,
                  pcSrcBuffer->getUBlk( cIdx )+ iOffset, iSrcStride,
                  cMv, iSizeY, iSizeX );

  xPredChromaPel( pcDesBuffer->getVBlk( cIdx ),          iDesStride,
                  pcSrcBuffer->getVBlk( cIdx )+ iOffset, iSrcStride,
                  cMv, iSizeY, iSizeX );
}




Void MotionCompensation::xGetMbPredData( MbDataAccess& rcMbDataAccess, const IntFrame* pcRefFrame, IntMC8x8D& rcMC8x8D )
{
  rcMC8x8D.clear();

  Int nMax = rcMbDataAccess.getSH().isInterB() ? 2:1;
  Int iPredCount = 0;

  for( Int n = 0; n < nMax; n++)
  {
    ListIdx eLstIdx = ListIdx( n );
    Mv3D& rcMv3D = rcMC8x8D.m_aacMv[eLstIdx][0];
    rcMbDataAccess.getMbMotionData( eLstIdx ).getMv3D( rcMv3D, rcMC8x8D.m_cIdx );

    if( BLOCK_NOT_PREDICTED != rcMv3D.getRef() )
    {
      rcMv3D.limitComponents( m_cMin, m_cMax );

      rcMC8x8D.m_apcRefBuffer[eLstIdx]  = const_cast<IntFrame*>(pcRefFrame)->getFullPelYuvBuffer();
      iPredCount++;
    }
  }
  rcMC8x8D.m_bBi = ( iPredCount == 2 );
}



__inline Void MotionCompensation::xGetBlkPredData( MbDataAccess& rcMbDataAccess, const IntFrame* pcRefFrame, IntMC8x8D& rcMC8x8D, BlkMode eBlkMode )
{
  rcMC8x8D.clear();

  Int nMax = rcMbDataAccess.getSH().isInterB() ? 2:1;
  Int iPredCount = 0;

  for( Int n = 0; n < nMax; n++)
  {
    ListIdx eLstIdx = ListIdx( n );
    Mv3D& rcMv3D = rcMC8x8D.m_aacMv[eLstIdx][SPART_4x4_0];
    const MbMotionData& rcMbMotionData = rcMbDataAccess.getMbMotionData( eLstIdx );

    rcMbMotionData.getMv3D( rcMv3D, rcMC8x8D.m_cIdx + SPART_4x4_0 );
    rcMv3D.limitComponents( m_cMin, m_cMax );

    if( BLOCK_NOT_PREDICTED != rcMv3D.getRef() )
    {
      iPredCount++;

      rcMC8x8D.m_apcRefBuffer[eLstIdx]  = const_cast<IntFrame*>(pcRefFrame)->getFullPelYuvBuffer();

      switch( eBlkMode )
      {
      case BLK_8x8:
        break;
      case BLK_8x4:
        {
          rcMC8x8D.m_aacMv[eLstIdx][SPART_4x4_2] = rcMbMotionData.getMv( rcMC8x8D.m_cIdx + SPART_4x4_2 );
          rcMC8x8D.m_aacMv[eLstIdx][SPART_4x4_2].limitComponents( m_cMin, m_cMax );
        }
        break;
      case BLK_4x8:
        {
          rcMC8x8D.m_aacMv[eLstIdx][SPART_4x4_1] = rcMbMotionData.getMv( rcMC8x8D.m_cIdx + SPART_4x4_1 );
          rcMC8x8D.m_aacMv[eLstIdx][SPART_4x4_1].limitComponents( m_cMin, m_cMax );
        }
        break;
      case BLK_SKIP:
      case BLK_4x4:
        {
          rcMC8x8D.m_aacMv[eLstIdx][SPART_4x4_1] = rcMbMotionData.getMv( rcMC8x8D.m_cIdx + SPART_4x4_1 );
          rcMC8x8D.m_aacMv[eLstIdx][SPART_4x4_1].limitComponents( m_cMin, m_cMax );

          rcMC8x8D.m_aacMv[eLstIdx][SPART_4x4_2] = rcMbMotionData.getMv( rcMC8x8D.m_cIdx + SPART_4x4_2 );
          rcMC8x8D.m_aacMv[eLstIdx][SPART_4x4_2].limitComponents( m_cMin, m_cMax );

          rcMC8x8D.m_aacMv[eLstIdx][SPART_4x4_3] = rcMbMotionData.getMv( rcMC8x8D.m_cIdx + SPART_4x4_3 );
          rcMC8x8D.m_aacMv[eLstIdx][SPART_4x4_3].limitComponents( m_cMin, m_cMax );
        }
        break;
      default:
        {
          AOT(1);
        }
      }
    }
  }
  rcMC8x8D.m_bBi = ( iPredCount == 2 );
}


Void MotionCompensation::xPredLuma( IntYuvMbBuffer* pcRecBuffer, Int iSizeX, Int iSizeY, IntMC8x8D& rcMc8x8D )
{
  IntYuvMbBuffer* apcTarBuffer[2];
  m_pcSampleWeighting->getTargetBuffers( apcTarBuffer, pcRecBuffer, rcMc8x8D.m_bBi );

  for( Int n = 0; n < 2; n++ )
  {
    IntYuvPicBuffer* pcRefBuffer = rcMc8x8D.m_apcRefBuffer[n];
    if( NULL != pcRefBuffer )
    {
      const Mv cMv = rcMc8x8D.m_aacMv[n][0];
      if(getMotCompType() == UPDT_MC)
      {
        int index = (m_curMbY*4 + rcMc8x8D.m_cIdx.y())*m_uiMbInFrameX*4 + m_curMbX*4 + rcMc8x8D.m_cIdx.x();
        m_pcQuarterPelFilter->predBlkAdapt( apcTarBuffer[n], pcRefBuffer, rcMc8x8D.m_cIdx, cMv, iSizeY, iSizeX, 
          (UShort*)(m_pusUpdateWeights + index));
      }
      else
        m_pcQuarterPelFilter->predBlk( apcTarBuffer[n], pcRefBuffer, rcMc8x8D.m_cIdx, cMv, iSizeY, iSizeX );
    }
  }
  m_pcSampleWeighting->weightLumaSamples( pcRecBuffer, iSizeX, iSizeY, rcMc8x8D.m_cIdx, rcMc8x8D.m_bBi );
}

Void MotionCompensation::xPredLuma( IntYuvMbBuffer* apcTarBuffer[2], Int iSizeX, Int iSizeY, IntMC8x8D& rcMc8x8D, SParIdx4x4 eSParIdx )
{
  B4x4Idx cIdx( rcMc8x8D.m_cIdx + eSParIdx );

  for( Int n = 0; n < 2; n++ )
  {
    IntYuvPicBuffer* pcRefBuffer = rcMc8x8D.m_apcRefBuffer[n];
    if( NULL != pcRefBuffer )
    {
      const Mv cMv = rcMc8x8D.m_aacMv[n][eSParIdx];
      if(getMotCompType() == UPDT_MC)
      {
        int index = (m_curMbY*4 + cIdx.y())*m_uiMbInFrameX*4 + m_curMbX*4 + cIdx.x();
        m_pcQuarterPelFilter->predBlkAdapt( apcTarBuffer[n], pcRefBuffer, cIdx, cMv, iSizeY, iSizeX, 
          (UShort*)(m_pusUpdateWeights + index) );
      }
      else
        m_pcQuarterPelFilter->predBlk( apcTarBuffer[n], pcRefBuffer, cIdx, cMv, iSizeY, iSizeX  );
    }
  }
}

__inline Void MotionCompensation::xPredChromaPel( XPel* pucDest, Int iDestStride, XPel* pucSrc, Int iSrcStride, Mv cMv, Int iSizeY, Int iSizeX )
{
  Int xF1 = cMv.getHor();
  Int yF1 = cMv.getVer();

  xF1 &= 0x7;
  yF1 &= 0x7;

  Int x, y;
  if( 0 == xF1 )
  {
    if( 0 == yF1 )
    {
      for( y = 0; y < iSizeY; y++ )
      {
        for( x = 0; x < iSizeX; x++ )
        {
          pucDest[x  ] = pucSrc[x  ];
        }
        pucDest += iDestStride;
        pucSrc  += iSrcStride;
      }
      return;
    }

    Int yF0 = 8 - yF1;

    for( y = 0; y < iSizeY; y++ )
    {
      for( x = 0; x < iSizeX; x++ )
      {
        pucDest[x  ] = (yF0 * pucSrc[x  ] + yF1 * pucSrc[x+iSrcStride  ] + 4) >> 3;
      }
      pucDest += iDestStride;
      pucSrc  += iSrcStride;
    }
    return;
  }


  if( 0 == yF1 )
  {
    Int xF0 = 8 - xF1;

    for( y = 0; y < iSizeY; y++ )
    {
      for( x = 0; x < iSizeX; x++ )
      {
        pucDest[x] = (xF0 * pucSrc[x] + xF1 * pucSrc[x+1] + 4) >> 3;
      }
      pucDest += iDestStride;
      pucSrc  += iSrcStride;
    }
    return;
  }

  Int xF0 = 8 - xF1;
  Int yF0 = 8 - yF1;
  for( y = 0; y < iSizeY; y++ )
  {
    Int a = xF0* ( yF0 * pucSrc[0] + yF1 * pucSrc[iSrcStride]);
    for( x = 0; x < iSizeX; x++ )
    {
      Int b = yF0 * pucSrc[x+1] + yF1 * pucSrc[iSrcStride+x+1];
      Int c = xF1 * b;
      pucDest[x]   = (a + c + 0x20) >> 6;
      a = (b<<3) - c;
    }
    pucDest += iDestStride;
    pucSrc  += iSrcStride;
  }
}

__inline Void MotionCompensation::xPredChroma( IntYuvMbBuffer* pcDesBuffer, IntYuvPicBuffer* pcSrcBuffer, LumaIdx cIdx, Mv cMv, Int iSizeY, Int iSizeX)
{
  const Int iDesStride  = pcDesBuffer->getCStride();
  const Int iSrcStride  = pcSrcBuffer->getCStride();

  cMv.limitComponents( m_cMin, m_cMax );

  const Int iOffset = (cMv.getHor() >> 3) + (cMv.getVer() >> 3) * iSrcStride;

  xPredChromaPel( pcDesBuffer->getUBlk( cIdx ),          iDesStride,
                  pcSrcBuffer->getUBlk( cIdx )+ iOffset, iSrcStride,
                  cMv, iSizeY, iSizeX );

  xPredChromaPel( pcDesBuffer->getVBlk( cIdx ),          iDesStride,
                  pcSrcBuffer->getVBlk( cIdx )+ iOffset, iSrcStride,
                  cMv, iSizeY, iSizeX );
}

Void MotionCompensation::xPredChroma( IntYuvMbBuffer* pcRecBuffer, Int iSizeX, Int iSizeY, IntMC8x8D& rcMc8x8D )
{
  IntYuvMbBuffer* apcTarBuffer[2];
  m_pcSampleWeighting->getTargetBuffers( apcTarBuffer, pcRecBuffer, rcMc8x8D.m_bBi );

  for( Int n = 0; n < 2; n++ )
  {
    IntYuvPicBuffer* pcRefBuffer = rcMc8x8D.m_apcRefBuffer[n];
    if( NULL != pcRefBuffer )
    {
      Mv cMv = rcMc8x8D.m_aacMv[n][0];
      xPredChroma( apcTarBuffer[n], pcRefBuffer, rcMc8x8D.m_cIdx, cMv, iSizeY, iSizeX );
    }
  }
  m_pcSampleWeighting->weightChromaSamples( pcRecBuffer, iSizeX, iSizeY, rcMc8x8D.m_cIdx, rcMc8x8D.m_bBi );
}

Void MotionCompensation::xPredChroma( IntYuvMbBuffer* apcTarBuffer[2], Int iSizeX, Int iSizeY, IntMC8x8D& rcMc8x8D, SParIdx4x4 eSParIdx )
{
  B4x4Idx cIdx( rcMc8x8D.m_cIdx + eSParIdx );

  for( Int n = 0; n < 2; n++ )
  {
    IntYuvPicBuffer* pcRefBuffer = rcMc8x8D.m_apcRefBuffer[n];
    if( NULL != pcRefBuffer )
    {
      Mv cMv = rcMc8x8D.m_aacMv[n][eSParIdx];
      xPredChroma( apcTarBuffer[n], pcRefBuffer, cIdx, cMv, iSizeY, iSizeX );
    }
  }
}






Void MotionCompensation::setMotCompType(MCType type)
{
  m_isUpdateComp = type;
}

MCType  MotionCompensation::getMotCompType()
{
  return m_isUpdateComp;
}


Void MotionCompensation::setUpdateWeightsBuf(UShort* updateWeightsBuf)
{
  m_pusUpdateWeights = updateWeightsBuf;
}


ErrVal MotionCompensation::calcMvMb( MbDataAccess& rcMbDataAccess, MbDataAccess* pcMbDataAccessBase )
{
  MbMode eMbMode  = rcMbDataAccess.getMbData().getMbMode();

  switch( eMbMode )
  {
  case MODE_16x16:
    xCalc16x16( rcMbDataAccess, pcMbDataAccessBase );
    break;
  case MODE_16x8:
    xCalc16x8( rcMbDataAccess, pcMbDataAccessBase );
    break;
  case MODE_8x16:
    xCalc8x16( rcMbDataAccess, pcMbDataAccessBase );
    break;
  case MODE_SKIP:
    if( rcMbDataAccess.getSH().isInterB() )
    {
      xCalcSDirect( rcMbDataAccess, pcMbDataAccessBase );
    }
    else
    {
      Mv cMvSkip;
      rcMbDataAccess.getMvPredictorSkipMode( cMvSkip );
      rcMbDataAccess.getMbMotionData( LIST_0 ).setAllMv( cMvSkip );
    }
    break;
  case MODE_8x8:
    xCalc8x8( rcMbDataAccess, pcMbDataAccessBase, false );
    break;
  default:
    break;
  }
  return Err::m_nOK;
}

ErrVal MotionCompensation::calcMvSubMb( B8x8Idx c8x8Idx, MbDataAccess& rcMbDataAccess, MbDataAccess* pcMbDataAccessBase )
{
  xCalc8x8( c8x8Idx, rcMbDataAccess, pcMbDataAccessBase, false );
  return Err::m_nOK;
}









ErrVal MotionCompensation::compensateSubMb( B8x8Idx         c8x8Idx,
                                            MbDataAccess&   rcMbDataAccess,
                                            RefFrameList&   rcRefFrameList0,
                                            RefFrameList&   rcRefFrameList1,
                                            IntYuvMbBuffer* pcRecBuffer,
                                            Bool            bCalcMv,
                                            Bool            bFaultTolerant )
{
  if(getMotCompType() == UPDT_MC)
  {
    m_curMbX = rcMbDataAccess.getMbX();
    m_curMbY = rcMbDataAccess.getMbY();
  }
  if( bCalcMv )
  {
    xCalc8x8( c8x8Idx, rcMbDataAccess, NULL, bFaultTolerant );
  }
  
  Int       iRefIdx0    = rcMbDataAccess.getMbMotionData( LIST_0 ).getRefIdx( c8x8Idx.b8x8() );
  Int       iRefIdx1    = rcMbDataAccess.getMbMotionData( LIST_1 ).getRefIdx( c8x8Idx.b8x8() );
  IntFrame* pcRefFrame0 = ( iRefIdx0 > 0 ? rcRefFrameList0[ iRefIdx0 ] : NULL );
  IntFrame* pcRefFrame1 = ( iRefIdx1 > 0 ? rcRefFrameList1[ iRefIdx1 ] : NULL );
  
  xPredMb8x8Mode( c8x8Idx, rcMbDataAccess, pcRefFrame0, pcRefFrame1, pcRecBuffer );

  return Err::m_nOK;
}

Void MotionCompensation::xPredMb8x8Mode(       B8x8Idx         c8x8Idx,
                                               MbDataAccess&   rcMbDataAccess,
                                         const IntFrame*       pcRefFrame0,
                                         const IntFrame*       pcRefFrame1,
                                               IntYuvMbBuffer* pcRecBuffer )
{
  IntYuvMbBuffer* apcTarBuffer[2];
  Par8x8  ePar8x8   = c8x8Idx.b8x8Index();
  BlkMode eBlkMode  = rcMbDataAccess.getMbData().getBlkMode( ePar8x8 );

  IntMC8x8D cMC8x8D( ePar8x8 );
  xGetBlkPredData( rcMbDataAccess, pcRefFrame0, pcRefFrame1, cMC8x8D, eBlkMode );
  m_pcSampleWeighting->getTargetBuffers( apcTarBuffer, pcRecBuffer, cMC8x8D.m_bBi );
  switch( eBlkMode )
  {
    case BLK_SKIP:
    {
      if( rcMbDataAccess.getSH().getSPS().getDirect8x8InferenceFlag() )
      {
        xPredLuma(   apcTarBuffer, 8, 8, cMC8x8D, SPART_4x4_0 );
        xPredChroma( apcTarBuffer, 4, 4, cMC8x8D, SPART_4x4_0 );
      }
      else
      {
        xPredLuma(   apcTarBuffer, 4, 4, cMC8x8D, SPART_4x4_0 );
        xPredLuma(   apcTarBuffer, 4, 4, cMC8x8D, SPART_4x4_1 );
        xPredLuma(   apcTarBuffer, 4, 4, cMC8x8D, SPART_4x4_2 );
        xPredLuma(   apcTarBuffer, 4, 4, cMC8x8D, SPART_4x4_3 );
        xPredChroma( apcTarBuffer, 2, 2, cMC8x8D, SPART_4x4_0 );
        xPredChroma( apcTarBuffer, 2, 2, cMC8x8D, SPART_4x4_1 );
        xPredChroma( apcTarBuffer, 2, 2, cMC8x8D, SPART_4x4_2 );
        xPredChroma( apcTarBuffer, 2, 2, cMC8x8D, SPART_4x4_3 );
      }
      break;
    }
    case BLK_8x8:
    {
      xPredLuma(   apcTarBuffer, 8, 8, cMC8x8D, SPART_4x4_0 );
      xPredChroma( apcTarBuffer, 4, 4, cMC8x8D, SPART_4x4_0 );
      break;
    }
    case BLK_8x4:
    {
      xPredLuma(   apcTarBuffer, 8, 4, cMC8x8D, SPART_4x4_0 );
      xPredLuma(   apcTarBuffer, 8, 4, cMC8x8D, SPART_4x4_2 );
      xPredChroma( apcTarBuffer, 4, 2, cMC8x8D, SPART_4x4_0 );
      xPredChroma( apcTarBuffer, 4, 2, cMC8x8D, SPART_4x4_2 );

      break;
    }
    case BLK_4x8:
    {
      xPredLuma(   apcTarBuffer, 4, 8, cMC8x8D, SPART_4x4_0 );
      xPredLuma(   apcTarBuffer, 4, 8, cMC8x8D, SPART_4x4_1 );
      xPredChroma( apcTarBuffer, 2, 4, cMC8x8D, SPART_4x4_0 );
      xPredChroma( apcTarBuffer, 2, 4, cMC8x8D, SPART_4x4_1 );

      break;
    }
    case BLK_4x4:
    {
      xPredLuma(   apcTarBuffer, 4, 4, cMC8x8D, SPART_4x4_0 );
      xPredLuma(   apcTarBuffer, 4, 4, cMC8x8D, SPART_4x4_1 );
      xPredLuma(   apcTarBuffer, 4, 4, cMC8x8D, SPART_4x4_2 );
      xPredLuma(   apcTarBuffer, 4, 4, cMC8x8D, SPART_4x4_3 );
      xPredChroma( apcTarBuffer, 2, 2, cMC8x8D, SPART_4x4_0 );
      xPredChroma( apcTarBuffer, 2, 2, cMC8x8D, SPART_4x4_1 );
      xPredChroma( apcTarBuffer, 2, 2, cMC8x8D, SPART_4x4_2 );
      xPredChroma( apcTarBuffer, 2, 2, cMC8x8D, SPART_4x4_3 );
      break;
    }
    default:
    {
      AOT(1);
      break;
    }
  }

  m_pcSampleWeighting->weightChromaSamples( pcRecBuffer, 4, 4, c8x8Idx, cMC8x8D.m_bBi );
  m_pcSampleWeighting->weightLumaSamples  ( pcRecBuffer, 8, 8, c8x8Idx, cMC8x8D.m_bBi );
}



ErrVal MotionCompensation::compensateMb( MbDataAccess&    rcMbDataAccess,
                                         RefFrameList&    rcRefFrameList0,
                                         RefFrameList&    rcRefFrameList1,
                                         IntYuvMbBuffer*  pcRecBuffer,
                                         Bool             bCalcMv,
                                         Bool             bFaultTolerant )
{
  MbMode eMbMode  = rcMbDataAccess.getMbData().getMbMode();
  if(getMotCompType() == UPDT_MC)
  {
    m_curMbX = rcMbDataAccess.getMbX();
    m_curMbY = rcMbDataAccess.getMbY();
  }

  switch( eMbMode )
  {
  case MODE_16x16:
    {
      if( bCalcMv )
      {
        xCalc16x16( rcMbDataAccess, NULL );
      }

      IntMC8x8D cMC8x8D( B_8x8_0 );
  
      Int       iRefIdx0    = rcMbDataAccess.getMbMotionData( LIST_0 ).getRefIdx();
      Int       iRefIdx1    = rcMbDataAccess.getMbMotionData( LIST_1 ).getRefIdx();
      IntFrame* pcRefFrame0 = ( iRefIdx0 > 0 ? rcRefFrameList0[ iRefIdx0 ] : NULL );
      IntFrame* pcRefFrame1 = ( iRefIdx1 > 0 ? rcRefFrameList1[ iRefIdx1 ] : NULL );
      xGetMbPredData( rcMbDataAccess, pcRefFrame0, pcRefFrame1, cMC8x8D );
      
      xPredLuma(   pcRecBuffer, 16, 16, cMC8x8D );
      xPredChroma( pcRecBuffer,  8,  8, cMC8x8D );
    }
    break;

  case MODE_16x8:
    {
      if( bCalcMv )
      {
        xCalc16x8( rcMbDataAccess, NULL );
      }

      IntMC8x8D cMC8x8D0( B_8x8_0 );
      IntMC8x8D cMC8x8D1( B_8x8_2 );

      Int       iRefIdx0    = rcMbDataAccess.getMbMotionData( LIST_0 ).getRefIdx( PART_16x8_0 );
      Int       iRefIdx1    = rcMbDataAccess.getMbMotionData( LIST_1 ).getRefIdx( PART_16x8_0 );
      IntFrame* pcRefFrame0 = ( iRefIdx0 > 0 ? rcRefFrameList0[ iRefIdx0 ] : NULL );
      IntFrame* pcRefFrame1 = ( iRefIdx1 > 0 ? rcRefFrameList1[ iRefIdx1 ] : NULL );
      xGetMbPredData( rcMbDataAccess, pcRefFrame0, pcRefFrame1, cMC8x8D0 );

      iRefIdx0    = rcMbDataAccess.getMbMotionData( LIST_0 ).getRefIdx( PART_16x8_1 );
      iRefIdx1    = rcMbDataAccess.getMbMotionData( LIST_1 ).getRefIdx( PART_16x8_1 );
      pcRefFrame0 = ( iRefIdx0 > 0 ? rcRefFrameList0[ iRefIdx0 ] : NULL );
      pcRefFrame1 = ( iRefIdx1 > 0 ? rcRefFrameList1[ iRefIdx1 ] : NULL );
      xGetMbPredData( rcMbDataAccess, pcRefFrame0, pcRefFrame1, cMC8x8D1 );

      xPredLuma(   pcRecBuffer, 16, 8, cMC8x8D0 );
      xPredLuma(   pcRecBuffer, 16, 8, cMC8x8D1 );
      xPredChroma( pcRecBuffer,  8, 4, cMC8x8D0 );
      xPredChroma( pcRecBuffer,  8, 4, cMC8x8D1 );
    }
    break;

  case MODE_8x16:
    {
      if( bCalcMv )
      {
        xCalc8x16( rcMbDataAccess, NULL );
      }

      IntMC8x8D cMC8x8D0( B_8x8_0 );
      IntMC8x8D cMC8x8D1( B_8x8_1 );

      Int       iRefIdx0    = rcMbDataAccess.getMbMotionData( LIST_0 ).getRefIdx( PART_8x16_0 );
      Int       iRefIdx1    = rcMbDataAccess.getMbMotionData( LIST_1 ).getRefIdx( PART_8x16_0 );
      IntFrame* pcRefFrame0 = ( iRefIdx0 > 0 ? rcRefFrameList0[ iRefIdx0 ] : NULL );
      IntFrame* pcRefFrame1 = ( iRefIdx1 > 0 ? rcRefFrameList1[ iRefIdx1 ] : NULL );
      xGetMbPredData( rcMbDataAccess, pcRefFrame0, pcRefFrame1, cMC8x8D0 );

      iRefIdx0    = rcMbDataAccess.getMbMotionData( LIST_0 ).getRefIdx( PART_8x16_1 );
      iRefIdx1    = rcMbDataAccess.getMbMotionData( LIST_1 ).getRefIdx( PART_8x16_1 );
      pcRefFrame0 = ( iRefIdx0 > 0 ? rcRefFrameList0[ iRefIdx0 ] : NULL );
      pcRefFrame1 = ( iRefIdx1 > 0 ? rcRefFrameList1[ iRefIdx1 ] : NULL );
      xGetMbPredData( rcMbDataAccess, pcRefFrame0, pcRefFrame1, cMC8x8D1 );

      xPredLuma(   pcRecBuffer, 8, 16, cMC8x8D0 );
      xPredLuma(   pcRecBuffer, 8, 16, cMC8x8D1 );
      xPredChroma( pcRecBuffer, 4,  8, cMC8x8D0 );
      xPredChroma( pcRecBuffer, 4,  8, cMC8x8D1 );
    }
    break;

  case MODE_SKIP:
    {
      if( rcMbDataAccess.getSH().isInterB() )
      {
        if( bCalcMv )
        {
          xCalcSDirect( rcMbDataAccess, NULL );
        }

        B8x8Idx c8x8Idx;
        RNOK( compensateDirectBlock( rcMbDataAccess, pcRecBuffer, c8x8Idx, rcRefFrameList0, rcRefFrameList1 ) ); c8x8Idx++;
        RNOK( compensateDirectBlock( rcMbDataAccess, pcRecBuffer, c8x8Idx, rcRefFrameList0, rcRefFrameList1 ) ); c8x8Idx++;
        RNOK( compensateDirectBlock( rcMbDataAccess, pcRecBuffer, c8x8Idx, rcRefFrameList0, rcRefFrameList1 ) ); c8x8Idx++;
        RNOK( compensateDirectBlock( rcMbDataAccess, pcRecBuffer, c8x8Idx, rcRefFrameList0, rcRefFrameList1 ) ); ;

      }
      else
      {
        if( bCalcMv )
        {
          Mv cMvPred;
          rcMbDataAccess.getMvPredictorSkipMode ( cMvPred );
          rcMbDataAccess.getMbMotionData( LIST_0 ).setRefIdx   ( 1 );
          rcMbDataAccess.getMbMotionData( LIST_0 ).setAllMv    ( cMvPred );
          rcMbDataAccess.getMbMotionData( LIST_1 ).setRefIdx   ( 0 );
          rcMbDataAccess.getMbMotionData( LIST_1 ).setAllMv    ( Mv::ZeroMv() );
        }

        IntMC8x8D cMC8x8D( B_8x8_0 );
      
        Int       iRefIdx0    = rcMbDataAccess.getMbMotionData( LIST_0 ).getRefIdx( PART_8x16_0 );
        Int       iRefIdx1    = rcMbDataAccess.getMbMotionData( LIST_1 ).getRefIdx( PART_8x16_0 );
        IntFrame* pcRefFrame0 = ( iRefIdx0 > 0 ? rcRefFrameList0[ iRefIdx0 ] : NULL );
        IntFrame* pcRefFrame1 = ( iRefIdx1 > 0 ? rcRefFrameList1[ iRefIdx1 ] : NULL );
        
        xGetMbPredData( rcMbDataAccess, pcRefFrame0, pcRefFrame1, cMC8x8D );

        xPredLuma(   pcRecBuffer, 16, 16, cMC8x8D );
        xPredChroma( pcRecBuffer,  8,  8, cMC8x8D );
        return Err::m_nOK;
      }
    }
    break;

  case MODE_8x8:
    printf("function not defined for the case\n");
    ROT(1);
    break;

  default:
    break;
  }

  return Err::m_nOK;
}

__inline Void MotionCompensation::xGetMbPredData(       MbDataAccess& rcMbDataAccess,
                                                  const IntFrame*     pcRefFrame0,
                                                  const IntFrame*     pcRefFrame1,
                                                        IntMC8x8D&    rcMC8x8D )
{
  rcMC8x8D.clear();

  Int iPredCount = 0;

  for (Int n = 0; n < 2; n++)
  {
    const IntFrame* pcRefFrame = n ? pcRefFrame1 : pcRefFrame0;
    ListIdx eLstIdx            = ListIdx(n);

    Mv3D& rcMv3D               = rcMC8x8D.m_aacMv[eLstIdx][0];
    rcMbDataAccess.getMbMotionData( eLstIdx ).getMv3D( rcMv3D, rcMC8x8D.m_cIdx );

    if( rcMv3D.getRef() > 0 ) // changed
    {
      iPredCount++;
      rcMv3D.limitComponents( m_cMin, m_cMax );

      rcMC8x8D.m_apcRefBuffer [eLstIdx] = const_cast<IntFrame*>(pcRefFrame)->getFullPelYuvBuffer();
    }
  }
  rcMC8x8D.m_bBi = ( iPredCount == 2 );
}

__inline Void MotionCompensation::xGetBlkPredData(       MbDataAccess& rcMbDataAccess,
                                                   const IntFrame*     pcRefFrame0,
                                                   const IntFrame*     pcRefFrame1,
                                                         IntMC8x8D&    rcMC8x8D,
                                                         BlkMode       eBlkMode )
{
  rcMC8x8D.clear();

  Int iPredCount = 0;

  for( Int n = 0; n < 2; n++)
  {
    const IntFrame* pcRefFrame = n ? pcRefFrame1 : pcRefFrame0;
    ListIdx eLstIdx            = ListIdx( n );

    Mv3D& rcMv3D                       = rcMC8x8D.m_aacMv[eLstIdx][SPART_4x4_0];
    const MbMotionData& rcMbMotionData = rcMbDataAccess.getMbMotionData( eLstIdx );

    rcMbMotionData.getMv3D( rcMv3D, rcMC8x8D.m_cIdx + SPART_4x4_0 );
    rcMv3D.limitComponents( m_cMin, m_cMax );

    if( rcMv3D.getRef() > 0 ) //changed
    {
      iPredCount++;
      rcMC8x8D.m_apcRefBuffer[eLstIdx]  = const_cast<IntFrame*>(pcRefFrame)->getFullPelYuvBuffer();

      switch( eBlkMode )
      {
      case BLK_8x8:
        break;
      case BLK_8x4:
        {
          rcMC8x8D.m_aacMv[eLstIdx][SPART_4x4_2] = rcMbMotionData.getMv( rcMC8x8D.m_cIdx + SPART_4x4_2 );
          rcMC8x8D.m_aacMv[eLstIdx][SPART_4x4_2].limitComponents( m_cMin, m_cMax );
        }
        break;
      case BLK_4x8:
        {
          rcMC8x8D.m_aacMv[eLstIdx][SPART_4x4_1] = rcMbMotionData.getMv( rcMC8x8D.m_cIdx + SPART_4x4_1 );
          rcMC8x8D.m_aacMv[eLstIdx][SPART_4x4_1].limitComponents( m_cMin, m_cMax );
        }
        break;
      case BLK_SKIP:
      case BLK_4x4:
        {
          rcMC8x8D.m_aacMv[eLstIdx][SPART_4x4_1] = rcMbMotionData.getMv( rcMC8x8D.m_cIdx + SPART_4x4_1 );
          rcMC8x8D.m_aacMv[eLstIdx][SPART_4x4_1].limitComponents( m_cMin, m_cMax );

          rcMC8x8D.m_aacMv[eLstIdx][SPART_4x4_2] = rcMbMotionData.getMv( rcMC8x8D.m_cIdx + SPART_4x4_2 );
          rcMC8x8D.m_aacMv[eLstIdx][SPART_4x4_2].limitComponents( m_cMin, m_cMax );

          rcMC8x8D.m_aacMv[eLstIdx][SPART_4x4_3] = rcMbMotionData.getMv( rcMC8x8D.m_cIdx + SPART_4x4_3 );
          rcMC8x8D.m_aacMv[eLstIdx][SPART_4x4_3].limitComponents( m_cMin, m_cMax );
        }
        break;
      default:
        {
          AOT(1);
        }
      }
    }
  }
  rcMC8x8D.m_bBi = ( iPredCount == 2 );
}


H264AVC_NAMESPACE_END

