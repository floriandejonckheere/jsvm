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

#include "H264AVCCommonLib/Tables.h"
#include "H264AVCCommonLib/Transform.h"


H264AVC_NAMESPACE_BEGIN


Transform::Transform()
: m_bClip( true )
{
}

Transform::~Transform()
{
}


ErrVal Transform::create( Transform*& rpcTransform )
{
  rpcTransform = new Transform;

  ROT( NULL == rpcTransform );

  return Err::m_nOK;
}


ErrVal Transform::destroy()
{
  delete this;

  return Err::m_nOK;
}

ErrVal Transform::invTransform4x4Blk( Pel* puc, Int iStride, TCoeff* piCoeff )
{
  xInvTransform4x4Blk( puc, iStride, piCoeff );

  return Err::m_nOK;
}



ErrVal Transform::invTransformDcCoeff( TCoeff* piCoeff, Int iQpScale )
{
  Int aai[4][4];
  Int tmp1, tmp2;
  Int x, y;

  TCoeff *piWCoeff = piCoeff;

  for( x = 0; x < 4; x++, piCoeff += 0x10 )
  {
    tmp1 = piCoeff[0x00] + piCoeff[0x80];
    tmp2 = piCoeff[0xc0] + piCoeff[0x40];

    aai[x][0] = tmp1 + tmp2;
	  aai[x][3] = tmp1 - tmp2;

    tmp1 = piCoeff[0x00] - piCoeff[0x80];
    tmp2 = piCoeff[0x40] - piCoeff[0xc0];

    aai[x][1] = tmp1 + tmp2;
    aai[x][2] = tmp1 - tmp2;
  }

  for( y = 0; y < 4; y++ )
  {
    tmp1 = aai[0][y] + aai[2][y];
    tmp2 = aai[3][y] + aai[1][y];

    piWCoeff[0x00] = (( tmp1 + tmp2 ) * iQpScale + 2 ) >> 2;
	  piWCoeff[0x30] = (( tmp1 - tmp2 ) * iQpScale + 2 ) >> 2;

    tmp1 = aai[0][y] - aai[2][y];
    tmp2 = aai[1][y] - aai[3][y];

    piWCoeff[0x10] = (( tmp1 + tmp2 ) * iQpScale + 2 ) >> 2;
    piWCoeff[0x20] = (( tmp1 - tmp2 ) * iQpScale + 2 ) >> 2;

    piWCoeff += 0x40;
  }

  return Err::m_nOK;
}



Void Transform::xForTransformLumaDc( TCoeff* piCoeff )
{
  Int aai[4][4];
  Int tmp1, tmp2;
  Int x, y;

  TCoeff *piWCoeff = piCoeff;

  for( x = 0; x < 4; x++, piCoeff += 0x10 )
  {
    tmp1 = piCoeff[0x00] + piCoeff[0x80];
    tmp2 = piCoeff[0xc0] + piCoeff[0x40];

    aai[x][0] = tmp1 + tmp2;
	  aai[x][3] = tmp1 - tmp2;

    tmp1 = piCoeff[0x00] - piCoeff[0x80];
    tmp2 = piCoeff[0x40] - piCoeff[0xc0];

    aai[x][1] = tmp1 + tmp2;
    aai[x][2] = tmp1 - tmp2;
  }

  for( y = 0; y < 4; y++ )
  {
    tmp1 = aai[0][y] + aai[2][y];
    tmp2 = aai[3][y] + aai[1][y];

    piWCoeff[0x00] = (tmp1 + tmp2)/2;
	  piWCoeff[0x30] = (tmp1 - tmp2)/2;

    tmp1 = aai[0][y] - aai[2][y];
    tmp2 = aai[1][y] - aai[3][y];

    piWCoeff[0x10] = (tmp1 + tmp2)/2;
    piWCoeff[0x20] = (tmp1 - tmp2)/2;

    piWCoeff += 0x40;
  }
}



Void Transform::xInvTransform4x4Blk( Pel* puc, Int iStride, TCoeff* piCoeff )
{

  Int aai[4][4];
  Int tmp1, tmp2;
  Int x, y;
  Int iStride2=2*iStride;
  Int iStride3=3*iStride;

  for( x = 0; x < 4; x++, piCoeff+=4 )
  {
    tmp1 = piCoeff[0] + piCoeff[2];
    tmp2 = (piCoeff[3]>>1) + piCoeff[1];

    aai[0][x] = tmp1 + tmp2;
	  aai[3][x] = tmp1 - tmp2;

    tmp1 = piCoeff[0] - piCoeff[2];
    tmp2 = (piCoeff[1]>>1) - piCoeff[3];

    aai[1][x] = tmp1 + tmp2;
    aai[2][x] = tmp1 - tmp2;
  }

  for( y = 0; y < 4; y++, puc ++ )
  {
    tmp1 =  aai[y][0] + aai[y][2];
    tmp2 = (aai[y][3]>>1) + aai[y][1];

    puc[0]        = gClip( xRound( tmp1 + tmp2) + puc[0]        );
	  puc[iStride3] = gClip( xRound( tmp1 - tmp2) + puc[iStride3] );

    tmp1 =  aai[y][0] - aai[y][2];
    tmp2 = (aai[y][1]>>1) - aai[y][3];

    puc[iStride]  = gClip( xRound( tmp1 + tmp2) + puc[iStride]  );
	  puc[iStride2] = gClip( xRound( tmp1 - tmp2) + puc[iStride2] );

  }
}



Void Transform::xInvTransform4x4BlkNoAc( Pel* puc, Int iStride, TCoeff* piCoeff )
{
  Int iDc = xRound( piCoeff[0] );

  ROFVS( iDc );

  for( Int y = 0; y < 4; y++ )
  {
    puc[0] = gClip( iDc + puc[0] );
    puc[1] = gClip( iDc + puc[0] );
    puc[2] = gClip( iDc + puc[0] );
    puc[3] = gClip( iDc + puc[0] );
    puc += iStride;
  }
}



Void Transform::xInvTransform4x4BlkNoAc( XPel* puc, Int iStride, TCoeff* piCoeff )
{
  Int iDc = xRound( piCoeff[0] );

  ROFVS( iDc );

  for( Int y = 0; y < 4; y++ )
  {
    puc[0] = xClip( iDc + puc[0] );
    puc[1] = xClip( iDc + puc[0] );
    puc[2] = xClip( iDc + puc[0] );
    puc[3] = xClip( iDc + puc[0] );
    puc += iStride;
  }
}



Void Transform::invTransformChromaDc( TCoeff* piCoeff, Int iQpScale )
{
  Int   tmp1, tmp2;
  Int   d00, d01, d10, d11;
 
  d00 = piCoeff[0];
  d10 = piCoeff[32];
  d01 = piCoeff[16];
  d11 = piCoeff[48];

  tmp1 = d00 + d11;
  tmp2 = d10 + d01;

  piCoeff[ 0] = ( ( tmp1 + tmp2 ) * iQpScale ) >> 5;
  piCoeff[48] = ( ( tmp1 - tmp2 ) * iQpScale ) >> 5;

  tmp1 = d00 - d11;
  tmp2 = d01 - d10;

  piCoeff[32] = ( ( tmp1 + tmp2 ) * iQpScale ) >> 5;
  piCoeff[16] = ( ( tmp1 - tmp2 ) * iQpScale ) >> 5;
}


Void Transform::xForTransformChromaDc( TCoeff* piCoeff )
{
  Int   tmp1, tmp2;
  Int   d00, d01, d10, d11;

  d00 = piCoeff[0];
  d10 = piCoeff[16];
  d01 = piCoeff[32];
  d11 = piCoeff[48];

  tmp1 = d00 + d11;
  tmp2 = d10 + d01;

  piCoeff[ 0] = (tmp1 + tmp2);
  piCoeff[48] = (tmp1 - tmp2);

  tmp1 = d00 - d11;
  tmp2 = d01 - d10;

  piCoeff[16] = (tmp1 + tmp2);
  piCoeff[32] = (tmp1 - tmp2);
}




Void Transform::xQuantDequantNonUniformLuma( TCoeff* piQCoeff, TCoeff* piCoeff, const QpParameter& rcQp, const UChar* pucScale, UInt& ruiDcAbs, UInt& ruiAcAbs )
{
  Int   iLevel = piCoeff[0];
  UInt  uiSign = ((UInt)iLevel)>>31;
  Int   iAdd   = ( 1 << 3 ) >> rcQp.per();

  iLevel       = abs( iLevel ) * g_aaiQuantCoef[ rcQp.rem() ][0];
  if( pucScale )
  {
    iLevel     = ( iLevel << 4 ) / pucScale[0];
  }
  iLevel       = ( iLevel + 2 * rcQp.add() ) >> ( rcQp.bits() + 1 );
  
  ruiDcAbs    += iLevel;
  iLevel       = ( uiSign ? -iLevel : iLevel );
  piQCoeff[0]  = iLevel;
    

  UInt uiAcAbs = 0;
  for( Int n = 1; n < 16; n++ )
  {
    iLevel      = piCoeff[n];
    Int iSign   = iLevel;

    iLevel      = abs( iLevel ) * g_aaiQuantCoef[rcQp.rem()][n];
    if( pucScale )
    {
      iLevel    = ( iLevel << 4 ) / pucScale[n];
    }
    iLevel      = ( iLevel + rcQp.add() ) >> rcQp.bits();

    if( 0 != iLevel )
    {
      iSign       >>= 31;
      Int iDeScale  = g_aaiDequantCoef[rcQp.rem()][n];
      uiAcAbs      += iLevel;
      iLevel       ^= iSign;
      iLevel       -= iSign;
      piQCoeff[n]   = iLevel;
      if( pucScale )
      {
        piCoeff[n] = ( ( iLevel*iDeScale*pucScale[n] + iAdd ) << rcQp.per() ) >> 4;
      }
      else
      {
        piCoeff[n]  = iLevel*iDeScale << rcQp.per();
      }
    }
    else
    {
      piQCoeff[n] = 0;
      piCoeff [n] = 0;
    }
  }

  ruiAcAbs += uiAcAbs;
  return;
}



Void Transform::xQuantDequantNonUniformChroma( TCoeff* piQCoeff, TCoeff* piCoeff, const QpParameter& rcQp, const UChar* pucScale, UInt& ruiDcAbs, UInt& ruiAcAbs )
{
  Int   iAdd    = ( 1 << 3 ) >> rcQp.per();
  {
    Int   iLevel  = piCoeff[0];
    UInt  uiSign  = ((UInt)iLevel)>>31;

    iLevel        = ( abs( iLevel ) * g_aaiQuantCoef[ rcQp.rem() ][0] );
    if( pucScale )
    {
      iLevel      = ( iLevel << 4 ) / pucScale[0];
    }
    iLevel        = ( iLevel + 2*rcQp.add() ) >> ( rcQp.bits() + 1 );
  
    ruiDcAbs   += iLevel;
    iLevel      = ( uiSign ? -iLevel : iLevel );
    piQCoeff[0] = iLevel;
    piCoeff [0] = iLevel;
  }
  

  UInt uiAcAbs = 0;
  for( int n = 1; n < 16; n++ )
  {
    Int iLevel  = piCoeff[n];
    Int iSign   = iLevel;

    iLevel      = ( abs( iLevel ) * g_aaiQuantCoef[rcQp.rem()][n] );
    if( pucScale )
    {
      iLevel    = ( iLevel << 4 ) / pucScale[n];
    }
    iLevel = ( iLevel + rcQp.add() ) >> rcQp.bits();

    if( 0 != iLevel )
    {
      iSign      >>= 31;
      Int iDeScale = g_aaiDequantCoef[rcQp.rem()][n];
      uiAcAbs     += iLevel;
      iLevel      ^= iSign;
      iLevel      -= iSign;
      piQCoeff[n]  = iLevel;
      if( pucScale )
      {
        piCoeff[n] = ( ( iLevel*iDeScale*pucScale[n] + iAdd ) << rcQp.per() ) >> 4;
      }
      else
      {
        piCoeff[n]  = iLevel*iDeScale << rcQp.per();
      }
    }
    else
    {
      piQCoeff[n] = 0;
      piCoeff[n] = 0;
    }
  }

  ruiAcAbs += uiAcAbs;
  return;
}





Void Transform::xQuantDequantUniform4x4( TCoeff* piQCoeff, TCoeff* piCoeff, const QpParameter& rcQp, const UChar* pucScale, UInt& ruiAbsSum )
{
  Int n     = 0;
  ruiAbsSum = 0;
  Int iAdd  = ( 1 << 3 ) >> rcQp.per();

  for( ; n < 16; n++ )
  {
    Int iLevel  = piCoeff[n];
    Int iSign   = iLevel;
    
    iLevel      = abs( iLevel ) * g_aaiQuantCoef[rcQp.rem()][n];
    if( pucScale )
    {
      iLevel    = ( iLevel << 4 ) / pucScale[n];
    }
    iLevel      = ( iLevel + rcQp.add() ) >> rcQp.bits();

    if( 0 != iLevel )
    {
      iSign      >>= 31;
      Int iDeScale = g_aaiDequantCoef[ rcQp.rem() ][ n ];
      ruiAbsSum   += iLevel;
      iLevel      ^= iSign;
      iLevel      -= iSign;
      piQCoeff[n]  = iLevel;
      if( pucScale )
      {
        piCoeff[n]   = ( ( iLevel*iDeScale*pucScale[n] + iAdd ) << rcQp.per() ) >> 4;
      }
      else
      {
        piCoeff[n]   = iLevel*iDeScale << rcQp.per();
      }
    }
    else
    {
      piQCoeff[n] = 0;
      piCoeff [n] = 0;
    }
  }
}




Void Transform::xRequantUniform4x4( TCoeff*             piCoeff,
                                    TCoeff*             piCoeffBase,
                                    const QpParameter&  rcQp,
                                    const UChar*        pucScale,
                                    UInt&               ruiAbsSum )
{
  Int normAdjust[] = { 4, 5, 4, 5 };

  ruiAbsSum  = 0;
  for( Int n = 0; n < 16; n++ )
  {
    Int iLevel  = piCoeff[n];
    Int iSign   = iLevel;
    iLevel     -= ( normAdjust[n/4] * normAdjust[n%4] * (Int)piCoeffBase[n] + ( 1 << 5 ) ) >> 6;
    iSign       = iLevel;
    iLevel      = abs( iLevel ) * g_aaiQuantCoef[rcQp.rem()][n];
    
    if( pucScale )
    {
      iLevel    = ( iLevel << 4 ) / pucScale[n];
    }
    iLevel      = ( iLevel + rcQp.add() ) >> rcQp.bits();

    if( 0 != iLevel )
    {
      iSign      >>= 31;
      ruiAbsSum   += iLevel;
      iLevel      ^= iSign;
      iLevel      -= iSign;
      piCoeff [n]  = iLevel;
    }
    else
    {
      piCoeff [n] = 0;
    }
  }
}





Void Transform::xRequantNonUniformChroma( TCoeff*             piCoeff,
                                          TCoeff*             piCoeffBase,
                                          const QpParameter&  rcQp,
                                          const UChar*        pucScale,
                                          UInt&               ruiDcAbs,
                                          UInt&               ruiAcAbs )
{
  Int normAdjust[] = { 4, 5, 4, 5 };

  Int   iLevel    = piCoeff[0];
  UInt  uiSign    = ((UInt)iLevel)>>31;
  iLevel         -= ( (Int)piCoeffBase[0] + 1 ) >> 1;
  uiSign          = ((UInt)iLevel)>>31;
  iLevel          = ( abs( iLevel ) * g_aaiQuantCoef[ rcQp.rem() ][0] );
  if( pucScale )
  {
    iLevel        = ( iLevel << 4 ) / pucScale[0];
  }
  iLevel          = ( iLevel + 2*rcQp.add() ) >> ( rcQp.bits() + 1 );

  ruiDcAbs       += iLevel;
  iLevel          = ( uiSign ? -iLevel : iLevel );
  piCoeff [0]     = iLevel;
  

  UInt uiAcAbs = 0;
  for( int n = 1; n < 16; n++ )
  {
    iLevel      = piCoeff[n];
    Int iSign   = iLevel;
    iLevel     -= ( normAdjust[n/4] * normAdjust[n%4] * (Int)piCoeffBase[n] + ( 1 << 5 ) ) >> 6;
    iSign       = iLevel;
    iLevel      = ( abs( iLevel ) * g_aaiQuantCoef[rcQp.rem()][n] );
    if( pucScale )
    {
      iLevel    = ( iLevel << 4 ) / pucScale[n];
    }
    iLevel      = ( iLevel + rcQp.add() ) >> rcQp.bits();

    if( 0 != iLevel )
    {
      iSign      >>= 31;
      uiAcAbs     += iLevel;
      iLevel      ^= iSign;
      iLevel      -= iSign;
      piCoeff [n]  = iLevel;
    }
    else
    {
      piCoeff [n]  = 0;
    }
  }

  ruiAcAbs += uiAcAbs;
  return;
}



ErrVal Transform::invTransformChromaBlocks( Pel* puc, Int iStride, TCoeff* piCoeff )
{
  xInvTransform4x4Blk( puc,     iStride, piCoeff + 0x00 );
  xInvTransform4x4Blk( puc + 4, iStride, piCoeff + 0x10 );
  puc += iStride << 2;
  xInvTransform4x4Blk( puc,     iStride, piCoeff + 0x20 );
  xInvTransform4x4Blk( puc + 4, iStride, piCoeff + 0x30 );

  return Err::m_nOK;
}






ErrVal Transform::transform4x4Blk( IntYuvMbBuffer* pcOrgData, IntYuvMbBuffer* pcPelData, TCoeff* piCoeff, const UChar* pucScale, UInt& ruiAbsSum )
{
  TCoeff  aiTemp[64];
  XPel*   pOrg    = pcOrgData->getLumBlk();
  XPel*   pRec    = pcPelData->getLumBlk();
  Int     iStride = pcPelData->getLStride();

  xForTransform4x4Blk( pOrg, pRec, iStride, aiTemp );
  xQuantDequantUniform4x4( piCoeff, aiTemp, m_cLumaQp, pucScale, ruiAbsSum );

  ROTRS( 0 == ruiAbsSum, Err::m_nOK );

  xInvTransform4x4Blk( pRec, iStride, aiTemp );

  return Err::m_nOK;
}



ErrVal
Transform::requant4x4Block( IntYuvMbBuffer& rcResData,
                            TCoeff*         piCoeff,
                            TCoeff*         piCoeffBase,
                            const UChar*    pucScale,
                            Bool            bFirstIsDc,
                            UInt&           ruiAbsSum )
{
  Int iDcCoeff = 0;

  //===== trafo =====
  x4x4Trafo( rcResData.getLumBlk (), rcResData.getLStride(), piCoeff );

  // backup the first coefficient
  if (bFirstIsDc)
  {
    iDcCoeff = piCoeff[0];
    piCoeff[0] = 0;
  }

  //===== quantization =====
  xRequantUniform4x4( piCoeff, piCoeffBase, m_cLumaQp, pucScale, ruiAbsSum );

  // restored the first coefficient
  if (bFirstIsDc)
    piCoeff[0] = iDcCoeff;

  return Err::m_nOK;
}


ErrVal
Transform::requantLumaDcCoeffs( TCoeff*         piCoeff,
                                TCoeff*         piCoeffBase,
                                const UChar*    pucScale,
                                UInt&           ruiAbsSum )
{
  // the transform was already performed
  xForTransformLumaDc( piCoeff );

  ruiAbsSum = 0;
  for( UInt n = 0; n < 16; n ++ )
  {
    Int   iLevel = piCoeff[n<<4];
    UInt  uiSign = ((UInt)iLevel)>>31;
    iLevel      -= ( (Int)piCoeffBase[n<<4] + 1 ) >> 1;
    uiSign       = ((UInt)iLevel)>>31;

    iLevel       = abs( iLevel ) * g_aaiQuantCoef[ m_cLumaQp.rem() ][0];
    if( pucScale )
    {
      iLevel     = ( iLevel << 4 ) / pucScale[0];
    }
    iLevel       = ( iLevel + 2 * m_cLumaQp.add() ) >> ( m_cLumaQp.bits() + 1 );
    
    ruiAbsSum    += iLevel;
    iLevel       = ( uiSign ? -iLevel : iLevel );
    piCoeff[n<<4]  = iLevel;
  }

  return Err::m_nOK;
}



ErrVal
Transform::requant8x8Block( IntYuvMbBuffer& rcResData,
                            TCoeff*         piCoeff,
                            TCoeff*         piCoeffBase,
                            const UChar*    pucScale,
                            UInt&           ruiAbsSum )
{
  //===== trafo =====
  x8x8Trafo( rcResData.getLumBlk (), rcResData.getLStride(), piCoeff );

  //===== quantization =====
  xRequantUniform8x8( piCoeff, piCoeffBase, m_cLumaQp, pucScale, ruiAbsSum );

  return Err::m_nOK;
}


ErrVal
Transform::requantChroma( IntYuvMbBuffer& rcResData,
                          TCoeff*         piCoeff,
                          TCoeff*         piCoeffBase,
                          const UChar*    pucScaleU,
                          const UChar*    pucScaleV,
                          UInt&           ruiDcAbs,
                          UInt&           ruiAcAbs )
{
  x4x4Trafo( rcResData.getMbCbAddr(),                                rcResData.getCStride(), piCoeff + 0x00 );
  x4x4Trafo( rcResData.getMbCbAddr() + 4,                            rcResData.getCStride(), piCoeff + 0x10 );
  x4x4Trafo( rcResData.getMbCbAddr() +     4*rcResData.getCStride(), rcResData.getCStride(), piCoeff + 0x20 );
  x4x4Trafo( rcResData.getMbCbAddr() + 4 + 4*rcResData.getCStride(), rcResData.getCStride(), piCoeff + 0x30 );
  x4x4Trafo( rcResData.getMbCrAddr(),                                rcResData.getCStride(), piCoeff + 0x40 );
  x4x4Trafo( rcResData.getMbCrAddr() + 4,                            rcResData.getCStride(), piCoeff + 0x50 );
  x4x4Trafo( rcResData.getMbCrAddr() +     4*rcResData.getCStride(), rcResData.getCStride(), piCoeff + 0x60 );
  x4x4Trafo( rcResData.getMbCrAddr() + 4 + 4*rcResData.getCStride(), rcResData.getCStride(), piCoeff + 0x70 );
  xForTransformChromaDc( piCoeff + 0x00 );
  xForTransformChromaDc( piCoeff + 0x40 );

  xRequantNonUniformChroma( piCoeff+0x00, piCoeffBase+0x00, m_cChromaQp, pucScaleU, ruiDcAbs, ruiAcAbs );
  xRequantNonUniformChroma( piCoeff+0x10, piCoeffBase+0x10, m_cChromaQp, pucScaleU, ruiDcAbs, ruiAcAbs );
  xRequantNonUniformChroma( piCoeff+0x20, piCoeffBase+0x20, m_cChromaQp, pucScaleU, ruiDcAbs, ruiAcAbs );
  xRequantNonUniformChroma( piCoeff+0x30, piCoeffBase+0x30, m_cChromaQp, pucScaleU, ruiDcAbs, ruiAcAbs );

  xRequantNonUniformChroma( piCoeff+0x40, piCoeffBase+0x40, m_cChromaQp, pucScaleV, ruiDcAbs, ruiAcAbs );
  xRequantNonUniformChroma( piCoeff+0x50, piCoeffBase+0x50, m_cChromaQp, pucScaleV, ruiDcAbs, ruiAcAbs );
  xRequantNonUniformChroma( piCoeff+0x60, piCoeffBase+0x60, m_cChromaQp, pucScaleV, ruiDcAbs, ruiAcAbs );
  xRequantNonUniformChroma( piCoeff+0x70, piCoeffBase+0x70, m_cChromaQp, pucScaleV, ruiDcAbs, ruiAcAbs );

  return Err::m_nOK;
}






Void Transform::xForTransform4x4Blk( XPel* pucOrg, XPel* pucRec, Int iStride, TCoeff* piPredCoeff )
{
  Int aai[4][4];
  Int tmp1, tmp2;

  for( Int y = 0; y < 4; y++ )
  {
    Int ai[4];

    ai[0] = pucOrg[0] - pucRec[0];
    ai[1] = pucOrg[1] - pucRec[1];
    ai[2] = pucOrg[2] - pucRec[2];
    ai[3] = pucOrg[3] - pucRec[3];

    tmp1 = ai[0] + ai[3];
    tmp2 = ai[1] + ai[2];

    aai[0][y] = tmp1 + tmp2;
    aai[2][y] = tmp1 - tmp2;

    tmp1 = ai[0] - ai[3];
    tmp2 = ai[1] - ai[2];

    aai[1][y] = tmp1 * 2 + tmp2 ;
    aai[3][y] = tmp1  - tmp2 * 2;
    pucRec += iStride;
    pucOrg += iStride;
  }


  for( Int x = 0; x < 4; x++, piPredCoeff++ )
  {
    tmp1 = aai[x][0] + aai[x][3];
    tmp2 = aai[x][1] + aai[x][2];

    piPredCoeff[0] = tmp1 + tmp2;
    piPredCoeff[8] = tmp1 - tmp2;

    tmp1 = aai[x][0] - aai[x][3];
    tmp2 = aai[x][1] - aai[x][2];

    piPredCoeff[4]  = tmp1 * 2 + tmp2;
    piPredCoeff[12] = tmp1 - tmp2 * 2;
  }
}




Void Transform::xInvTransform4x4Blk( XPel* puc, Int iStride, TCoeff* piCoeff )
{

  Int aai[4][4];
  Int tmp1, tmp2;
  Int x, y;
  Int iStride2=2*iStride;
  Int iStride3=3*iStride;

  for( x = 0; x < 4; x++, piCoeff+=4 )
  {
    tmp1 = piCoeff[0] + piCoeff[2];
    tmp2 = (piCoeff[3]>>1) + piCoeff[1];

    aai[0][x] = tmp1 + tmp2;
	  aai[3][x] = tmp1 - tmp2;

    tmp1 = piCoeff[0] - piCoeff[2];
    tmp2 = (piCoeff[1]>>1) - piCoeff[3];

    aai[1][x] = tmp1 + tmp2;
    aai[2][x] = tmp1 - tmp2;
  }

  for( y = 0; y < 4; y++, puc ++ )
  {
    tmp1 =  aai[y][0] + aai[y][2];
    tmp2 = (aai[y][3]>>1) + aai[y][1];

    puc[0]        = xClip( xRound( tmp1 + tmp2) + puc[0]        );
	  puc[iStride3] = xClip( xRound( tmp1 - tmp2) + puc[iStride3] );

    tmp1 =  aai[y][0] - aai[y][2];
    tmp2 = (aai[y][1]>>1) - aai[y][3];

    puc[iStride]  = xClip( xRound( tmp1 + tmp2) + puc[iStride]  );
	  puc[iStride2] = xClip( xRound( tmp1 - tmp2) + puc[iStride2] );

  }
}





ErrVal Transform::transformMb16x16( IntYuvMbBuffer* pcOrgData, IntYuvMbBuffer* pcPelData, TCoeff* piCoeff, const UChar* pucScale, UInt& ruiDcAbs, UInt& ruiAcAbs )
{
  XPel* pucOrg  = pcOrgData->getMbLumAddr();
  XPel* pucRec  = pcPelData->getMbLumAddr();
  Int   iStride = pcPelData->getLStride();

  TCoeff aiCoeff[256];

  Int x, n;
  Int iOffset = 0;

  for( n = 0; n < 16; n+=4 )
  {
    for( x = 0; x < 4; x++ )
    {
      UInt uiBlk = x+n;
      Int iOffsetBlk = iOffset + (x << 2);
      xForTransform4x4Blk( pucOrg + iOffsetBlk, pucRec + iOffsetBlk, iStride, &aiCoeff[uiBlk<<4] );
    }
    iOffset += iStride << 2;
  }

  xForTransformLumaDc( aiCoeff );

  for( n = 0; n < 16; n ++ )
  {
    xQuantDequantNonUniformLuma( &piCoeff[n<<4], &aiCoeff[n<<4], m_cLumaQp, pucScale, ruiDcAbs, ruiAcAbs );
  }
  
  for( n = 0; n < 16; n ++ )
  {
    aiCoeff[n<<4] = piCoeff[n<<4];
  }

  Int iQpScale = ( g_aaiDequantCoef[m_cLumaQp.rem()][0] << m_cLumaQp.per() );
  if( pucScale )
  {
    iQpScale = ( iQpScale * pucScale[0] ) >> 4;
  }

  invTransformDcCoeff( aiCoeff, iQpScale );

  iOffset = 0;
  for( n = 0; n < 16; n += 4 )
  {
    for( x = 0; x < 4; x++ )
    {
      UInt uiBlk = x+n;
      Int iOffsetBlk = iOffset + (x << 2);
      xInvTransform4x4Blk( pucRec + iOffsetBlk, iStride, &aiCoeff[uiBlk<<4] );
    }
    iOffset += iStride << 2;
  }

  return Err::m_nOK;
}


ErrVal Transform::transformChromaBlocks( XPel* pucOrg, XPel* pucRec, Int iStride, TCoeff* piCoeff, TCoeff* piQuantCoeff, const UChar* pucScale, UInt& ruiDcAbs, UInt& ruiAcAbs )
{
  Int iOffset = 0;

  xForTransform4x4Blk( pucOrg + iOffset, pucRec + iOffset, iStride, piQuantCoeff + 0x00);
  iOffset += 4;
  xForTransform4x4Blk( pucOrg + iOffset, pucRec + iOffset, iStride, piQuantCoeff + 0x10);
  iOffset  = 4*iStride;
  xForTransform4x4Blk( pucOrg + iOffset, pucRec + iOffset, iStride, piQuantCoeff + 0x20);
  iOffset += 4;
  xForTransform4x4Blk( pucOrg + iOffset, pucRec + iOffset, iStride, piQuantCoeff + 0x30);

  xForTransformChromaDc( piQuantCoeff );

  xQuantDequantNonUniformChroma( piCoeff + 0x00, piQuantCoeff + 0x00, m_cChromaQp, pucScale, ruiDcAbs, ruiAcAbs );
  xQuantDequantNonUniformChroma( piCoeff + 0x10, piQuantCoeff + 0x10, m_cChromaQp, pucScale, ruiDcAbs, ruiAcAbs );
  xQuantDequantNonUniformChroma( piCoeff + 0x20, piQuantCoeff + 0x20, m_cChromaQp, pucScale, ruiDcAbs, ruiAcAbs );
  xQuantDequantNonUniformChroma( piCoeff + 0x30, piQuantCoeff + 0x30, m_cChromaQp, pucScale, ruiDcAbs, ruiAcAbs );

  return Err::m_nOK;
}


ErrVal Transform::invTransformChromaBlocks( XPel* puc, Int iStride, TCoeff* piCoeff )
{
  xInvTransform4x4Blk( puc,     iStride, piCoeff + 0x00 );
  xInvTransform4x4Blk( puc + 4, iStride, piCoeff + 0x10 );
  puc += iStride << 2;
  xInvTransform4x4Blk( puc,     iStride, piCoeff + 0x20 );
  xInvTransform4x4Blk( puc + 4, iStride, piCoeff + 0x30 );

  return Err::m_nOK;
}


ErrVal Transform::invTransform4x4Blk( XPel* puc, Int iStride, TCoeff* piCoeff )
{
  xInvTransform4x4Blk( puc, iStride, piCoeff );

  return Err::m_nOK;
}





ErrVal
Transform::transform8x8Blk( IntYuvMbBuffer* pcOrgData,
                            IntYuvMbBuffer* pcPelData,
                            TCoeff*         piCoeff,
                            const UChar*    pucScale,
                            UInt&           ruiAbsSum )
{
  TCoeff  aiTemp[64];
  XPel*   pOrg    = pcOrgData->getLumBlk();
  XPel*   pRec    = pcPelData->getLumBlk();
  Int     iStride = pcPelData->getLStride();

  xForTransform8x8Blk     ( pOrg, pRec, iStride, aiTemp );
  xQuantDequantUniform8x8 ( piCoeff, aiTemp, m_cLumaQp, pucScale, ruiAbsSum );
  invTransform8x8Blk      ( pRec, iStride, aiTemp );

  return Err::m_nOK;
}



ErrVal
Transform::invTransform8x8Blk( XPel*    puc,
                               Int      iStride, 
                               TCoeff*  piCoeff )
{
  Int aai[8][8];
  Int n;

  for( n = 0; n < 8; n++ )
  {
    TCoeff* pi = piCoeff + n*8;
    Int     ai1[8];
    Int     ai2[8];
    
    ai1[0] = pi[0] + pi[4];
    ai1[2] = pi[0] - pi[4];

    ai1[4] = (pi[2]>>1) -  pi[6];
    ai1[6] =  pi[2]     + (pi[6]>>1);

    ai1[1] = pi[5] - pi[3] - pi[7] - (pi[7]>>1);
    ai1[3] = pi[1] + pi[7] - pi[3] - (pi[3]>>1);;
    ai1[5] = pi[7] - pi[1] + pi[5] + (pi[5]>>1);
    ai1[7] = pi[3] + pi[5] + pi[1] + (pi[1]>>1);

    ai2[0] = ai1[0] + ai1[6];
    ai2[6] = ai1[0] - ai1[6];

    ai2[2] = ai1[2] + ai1[4];
    ai2[4] = ai1[2] - ai1[4];

    ai2[1] = ai1[1] + (ai1[7]>>2);
    ai2[7] = ai1[7] - (ai1[1]>>2);

    ai2[3] =  ai1[3]     + (ai1[5]>>2);
    ai2[5] = (ai1[3]>>2) -  ai1[5];

    aai[n][0] = ai2[0] + ai2[7];
    aai[n][1] = ai2[2] + ai2[5];
    aai[n][2] = ai2[4] + ai2[3];
    aai[n][3] = ai2[6] + ai2[1];
    aai[n][4] = ai2[6] - ai2[1];
    aai[n][5] = ai2[4] - ai2[3];
    aai[n][6] = ai2[2] - ai2[5];
    aai[n][7] = ai2[0] - ai2[7];
  }

  for( n = 0; n < 8; n++, puc++ )
  {
    Int ai1[8];
    Int ai2[8];

    ai1[0] =  aai[0][n]     +  aai[4][n];
    ai1[1] =  aai[5][n]     -  aai[3][n]     - aai[7][n] - (aai[7][n]>>1);
    ai1[2] =  aai[0][n]     -  aai[4][n];
    ai1[3] =  aai[1][n]     +  aai[7][n]     - aai[3][n] - (aai[3][n]>>1);
    ai1[4] = (aai[2][n]>>1) -  aai[6][n];
    ai1[5] =  aai[7][n]     -  aai[1][n]     + aai[5][n] + (aai[5][n]>>1);
    ai1[6] =  aai[2][n]     + (aai[6][n]>>1);
    ai1[7] =  aai[3][n]     +  aai[5][n]     + aai[1][n] + (aai[1][n]>>1);

    ai2[2] = ai1[2] + ai1[4];
    ai2[4] = ai1[2] - ai1[4];

    ai2[0] = ai1[0] + ai1[6];
    ai2[6] = ai1[0] - ai1[6];

    ai2[1] = ai1[1] + (ai1[7]>>2);
    ai2[7] = ai1[7] - (ai1[1]>>2);

    ai2[3] =  ai1[3]     + (ai1[5]>>2);
    ai2[5] = (ai1[3]>>2) -  ai1[5];

    puc[0*iStride] = xClip( xRound( ai2[0] + ai2[7] ) + puc[0*iStride] );
    puc[1*iStride] = xClip( xRound( ai2[2] + ai2[5] ) + puc[1*iStride] );
    puc[2*iStride] = xClip( xRound( ai2[4] + ai2[3] ) + puc[2*iStride] );
    puc[3*iStride] = xClip( xRound( ai2[6] + ai2[1] ) + puc[3*iStride] );
    puc[4*iStride] = xClip( xRound( ai2[6] - ai2[1] ) + puc[4*iStride] );
    puc[5*iStride] = xClip( xRound( ai2[4] - ai2[3] ) + puc[5*iStride] );
    puc[6*iStride] = xClip( xRound( ai2[2] - ai2[5] ) + puc[6*iStride] );
    puc[7*iStride] = xClip( xRound( ai2[0] - ai2[7] ) + puc[7*iStride] );
  }

  return Err::m_nOK;
}




ErrVal
Transform::transformMb( TCoeff*          piCoeff,           // -> array of 16*16+2*4*16 = 384 entries
                        IntYuvMbBuffer&  rcYuvMbBuffer,     // <- current macroblock buffer
                        Int              iQpLuma,           // <- luma QP
                        Int              iQpChroma,         // <- chroma QP
                        Int              iQuantOffsetDiv )  // <- determines quantization offset
{
  ROF( piCoeff );

  TCoeff      aiOrgCoeff[16];
  QpParameter cLumaQp, cChromaQp;
  Int         iLStride  = rcYuvMbBuffer.getLStride();
  Int         iCStride  = rcYuvMbBuffer.getCStride();

  //===== set quantization parameters =====
  cLumaQp   .setQp( iQpLuma,   iQuantOffsetDiv );
  cChromaQp .setQp( iQpChroma, iQuantOffsetDiv );

  //===== luma =====
  for( B4x4Idx cYIdx; cYIdx.isLegal(); cYIdx++, piCoeff+=16 )
  {
    x4x4Trafo( rcYuvMbBuffer.getYBlk( cYIdx ), iLStride, aiOrgCoeff );
    x4x4Quant(                                 piCoeff,  aiOrgCoeff, cLumaQp );
  }

  //===== chroma =====
  for( CIdx    cCIdx; cCIdx.isLegal(); cCIdx++, piCoeff+=16 )
  {
    x4x4Trafo( rcYuvMbBuffer.getCBlk( cCIdx ), iCStride, aiOrgCoeff );
    x4x4Quant(                                 piCoeff,  aiOrgCoeff, cChromaQp );
  }

  return Err::m_nOK;
}



ErrVal
Transform::inverseTransformMb( IntYuvMbBuffer&  rcYuvMbBuffer,    // -> current macroblock buffer
                               TCoeff*          piCoeff,          // <- array of 16*16+2*4*16 = 384 entries
                               Int              iQpLuma,          // <- luma QP
                               Int              iQpChroma )       // <- chroma QP
{
  ROF( piCoeff );

  TCoeff      aiRecCoeff[16];
  QpParameter cLumaQp, cChromaQp;
  Int         iLStride  = rcYuvMbBuffer.getLStride();
  Int         iCStride  = rcYuvMbBuffer.getCStride();

  //===== set quantization parameters =====
  cLumaQp   .setQp( iQpLuma   );
  cChromaQp .setQp( iQpChroma );

  //===== luma =====
  for( B4x4Idx cYIdx; cYIdx.isLegal(); cYIdx++, piCoeff+=16 )
  {
    x4x4Dequant(                                      piCoeff,  aiRecCoeff, cLumaQp );
    x4x4InverseTrafo( rcYuvMbBuffer.getYBlk( cYIdx ), iLStride, aiRecCoeff );
  }

  //===== chroma =====
  for( CIdx    cCIdx; cCIdx.isLegal(); cCIdx++, piCoeff+=16 )
  {
    x4x4Dequant(                                      piCoeff,  aiRecCoeff, cChromaQp );
    x4x4InverseTrafo( rcYuvMbBuffer.getCBlk( cCIdx ), iCStride, aiRecCoeff );
  }

  return Err::m_nOK;
}


Void
Transform::x4x4Trafo( XPel*   pOrg,
                      Int     iStride,
                      TCoeff* piCoeff )
{
  Int aai[4][4];
  Int tmp1, tmp2;

  for( Int y = 0; y < 4; y++ )
  {
    tmp1 = pOrg[0] + pOrg[3];
    tmp2 = pOrg[1] + pOrg[2];

    aai[0][y] = tmp1 + tmp2;
    aai[2][y] = tmp1 - tmp2;

    tmp1 = pOrg[0] - pOrg[3];
    tmp2 = pOrg[1] - pOrg[2];

    aai[1][y] = tmp1 * 2 + tmp2 ;
    aai[3][y] = tmp1  - tmp2 * 2;
    
    pOrg += iStride;
  }


  for( Int x = 0; x < 4; x++, piCoeff++ )
  {
    tmp1 = aai[x][0] + aai[x][3];
    tmp2 = aai[x][1] + aai[x][2];

    piCoeff[0] = tmp1 + tmp2;
    piCoeff[8] = tmp1 - tmp2;

    tmp1 = aai[x][0] - aai[x][3];
    tmp2 = aai[x][1] - aai[x][2];

    piCoeff[4]  = tmp1 * 2 + tmp2;
    piCoeff[12] = tmp1 - tmp2 * 2;
  }
}


Void
Transform::x4x4InverseTrafo( XPel*   pRec,
                             Int     iStride,
                             TCoeff* piCoeff )
{

  Int aai[4][4];
  Int tmp1, tmp2;
  Int x, y;
  Int iStride2 = 2*iStride;
  Int iStride3 = 3*iStride;

  for( x = 0; x < 4; x++, piCoeff+=4 )
  {
    tmp1 =  piCoeff[0]     + piCoeff[2];
    tmp2 = (piCoeff[3]>>1) + piCoeff[1];

    aai[0][x] = tmp1 + tmp2;
	  aai[3][x] = tmp1 - tmp2;

    tmp1 =  piCoeff[0]     - piCoeff[2];
    tmp2 = (piCoeff[1]>>1) - piCoeff[3];

    aai[1][x] = tmp1 + tmp2;
    aai[2][x] = tmp1 - tmp2;
  }

  for( y = 0; y < 4; y++, pRec++ )
  {
    tmp1 =  aai[y][0]     + aai[y][2];
    tmp2 = (aai[y][3]>>1) + aai[y][1];

    pRec[0]        = /*gClip*/( xRound( tmp1 + tmp2) );
	  pRec[iStride3] = /*gClip*/( xRound( tmp1 - tmp2) );

    tmp1 =  aai[y][0]     - aai[y][2];
    tmp2 = (aai[y][1]>>1) - aai[y][3];

    pRec[iStride]  = /*gClip*/( xRound( tmp1 + tmp2) );
	  pRec[iStride2] = /*gClip*/( xRound( tmp1 - tmp2) );
  }
}


Void
Transform::x4x4Quant( TCoeff*             piQCoeff,
                      TCoeff*             piCoeff,
                      const QpParameter&  rcQp )
{
  for( Int n = 0; n < 16; n++ )
  {
    Int iLevel  = ( abs( piCoeff[n] ) * g_aaiQuantCoef[ rcQp.rem() ][n] + rcQp.add() ) >> rcQp.bits();
    Int iSign   = (  0 < piCoeff[n] ? 1 : -1 );
    
    if( 0 != iLevel )
    {
      piQCoeff[n]  = iLevel * iSign;
    }
    else
    {
      piQCoeff[n] = 0;
    }
  }
}


Void
Transform::x4x4Dequant( TCoeff*             piQCoeff,
                        TCoeff*             piCoeff,
                        const QpParameter&  rcQp )
{
  for( Int n = 0; n < 16; n++ )
  {
    if( piQCoeff[n] != 0 )
    {
      Int iScale  = g_aaiDequantCoef[rcQp.rem()][n];
      piCoeff[n]  = ( piQCoeff[n] * iScale ) << rcQp.per();
    }
    else
    {
      piCoeff[n]  = 0;
    }
  }
}





Void
Transform::xForTransform8x8Blk( XPel* pucOrg, XPel* pucRec, Int iStride, TCoeff* piPredCoeff )
{
  Int aai[8][8];

  for( Int i = 0; i < 8; i++, pucOrg += iStride, pucRec += iStride)
  {
    Int ai  [8];
    Int ai1 [8];
    Int ai2 [8];

    ai[0] = pucOrg[0] - pucRec[0];
    ai[1] = pucOrg[1] - pucRec[1];
    ai[2] = pucOrg[2] - pucRec[2];
    ai[3] = pucOrg[3] - pucRec[3];
    ai[4] = pucOrg[4] - pucRec[4];
    ai[5] = pucOrg[5] - pucRec[5];
    ai[6] = pucOrg[6] - pucRec[6];
    ai[7] = pucOrg[7] - pucRec[7];
    
    ai1[0] = ai[0] + ai[7];
    ai1[1] = ai[1] + ai[6];
    ai1[2] = ai[2] + ai[5];
    ai1[3] = ai[3] + ai[4];

    ai1[4] = ai[0] - ai[7];
    ai1[5] = ai[1] - ai[6];
    ai1[6] = ai[2] - ai[5];
    ai1[7] = ai[3] - ai[4];

    ai2[0] = ai1[0] + ai1[3];
    ai2[1] = ai1[1] + ai1[2];
    ai2[2] = ai1[0] - ai1[3];
    ai2[3] = ai1[1] - ai1[2];
    ai2[4] = ai1[5] + ai1[6] + ((ai1[4]>>1) + ai1[4]);
    ai2[5] = ai1[4] - ai1[7] - ((ai1[6]>>1) + ai1[6]);
    ai2[6] = ai1[4] + ai1[7] - ((ai1[5]>>1) + ai1[5]);
    ai2[7] = ai1[5] - ai1[6] + ((ai1[7]>>1) + ai1[7]);

    aai[0][i] =  ai2[0]     +  ai2[1];
    aai[2][i] =  ai2[2]     + (ai2[3]>>1);
    aai[4][i] =  ai2[0]     -  ai2[1];
    aai[6][i] = (ai2[2]>>1) -  ai2[3];

    aai[1][i] =  ai2[4]     + (ai2[7]>>2);
    aai[3][i] =  ai2[5]     + (ai2[6]>>2);
    aai[5][i] =  ai2[6]     - (ai2[5]>>2);
    aai[7][i] = (ai2[4]>>2) -  ai2[7];
  }

  // vertical transform
  for( Int n = 0; n < 8; n++, piPredCoeff++)
  {
    Int ai1[8];
    Int ai2[8];

    ai1[0] = aai[n][0] + aai[n][7];
    ai1[1] = aai[n][1] + aai[n][6];
    ai1[2] = aai[n][2] + aai[n][5];
    ai1[3] = aai[n][3] + aai[n][4];
    ai1[4] = aai[n][0] - aai[n][7];
    ai1[5] = aai[n][1] - aai[n][6];
    ai1[6] = aai[n][2] - aai[n][5];
    ai1[7] = aai[n][3] - aai[n][4];

    ai2[0] = ai1[0] + ai1[3];
    ai2[1] = ai1[1] + ai1[2];
    ai2[2] = ai1[0] - ai1[3];
    ai2[3] = ai1[1] - ai1[2];
    ai2[4] = ai1[5] + ai1[6] + ((ai1[4]>>1) + ai1[4]);
    ai2[5] = ai1[4] - ai1[7] - ((ai1[6]>>1) + ai1[6]);
    ai2[6] = ai1[4] + ai1[7] - ((ai1[5]>>1) + ai1[5]);
    ai2[7] = ai1[5] - ai1[6] + ((ai1[7]>>1) + ai1[7]);

    piPredCoeff[ 0] =  ai2[0]     +  ai2[1];
    piPredCoeff[16] =  ai2[2]     + (ai2[3]>>1);
    piPredCoeff[32] =  ai2[0]     -  ai2[1];
    piPredCoeff[48] = (ai2[2]>>1) -  ai2[3];

    piPredCoeff[ 8] =  ai2[4]     + (ai2[7]>>2);
    piPredCoeff[24] =  ai2[5]     + (ai2[6]>>2);
    piPredCoeff[40] =  ai2[6]     - (ai2[5]>>2);
    piPredCoeff[56] = (ai2[4]>>2) -  ai2[7];
  }
}


Void
Transform::x8x8Trafo( XPel*   pOrg, 
                      Int     iStride,
                      TCoeff* piCoeff )
{
  Int aai[8][8];

  for( Int i = 0; i < 8; i++, pOrg += iStride )
  {
    Int ai1 [8];
    Int ai2 [8];
    
    ai1[0] = pOrg[0] + pOrg[7];
    ai1[1] = pOrg[1] + pOrg[6];
    ai1[2] = pOrg[2] + pOrg[5];
    ai1[3] = pOrg[3] + pOrg[4];

    ai1[4] = pOrg[0] - pOrg[7];
    ai1[5] = pOrg[1] - pOrg[6];
    ai1[6] = pOrg[2] - pOrg[5];
    ai1[7] = pOrg[3] - pOrg[4];

    ai2[0] = ai1[0] + ai1[3];
    ai2[1] = ai1[1] + ai1[2];
    ai2[2] = ai1[0] - ai1[3];
    ai2[3] = ai1[1] - ai1[2];
    ai2[4] = ai1[5] + ai1[6] + ((ai1[4]>>1) + ai1[4]);
    ai2[5] = ai1[4] - ai1[7] - ((ai1[6]>>1) + ai1[6]);
    ai2[6] = ai1[4] + ai1[7] - ((ai1[5]>>1) + ai1[5]);
    ai2[7] = ai1[5] - ai1[6] + ((ai1[7]>>1) + ai1[7]);

    aai[0][i] =  ai2[0]     +  ai2[1];
    aai[2][i] =  ai2[2]     + (ai2[3]>>1);
    aai[4][i] =  ai2[0]     -  ai2[1];
    aai[6][i] = (ai2[2]>>1) -  ai2[3];

    aai[1][i] =  ai2[4]     + (ai2[7]>>2);
    aai[3][i] =  ai2[5]     + (ai2[6]>>2);
    aai[5][i] =  ai2[6]     - (ai2[5]>>2);
    aai[7][i] = (ai2[4]>>2) -  ai2[7];
  }

  // vertical transform
  for( Int n = 0; n < 8; n++, piCoeff++ )
  {
    Int ai1[8];
    Int ai2[8];

    ai1[0] = aai[n][0] + aai[n][7];
    ai1[1] = aai[n][1] + aai[n][6];
    ai1[2] = aai[n][2] + aai[n][5];
    ai1[3] = aai[n][3] + aai[n][4];
    ai1[4] = aai[n][0] - aai[n][7];
    ai1[5] = aai[n][1] - aai[n][6];
    ai1[6] = aai[n][2] - aai[n][5];
    ai1[7] = aai[n][3] - aai[n][4];

    ai2[0] = ai1[0] + ai1[3];
    ai2[1] = ai1[1] + ai1[2];
    ai2[2] = ai1[0] - ai1[3];
    ai2[3] = ai1[1] - ai1[2];
    ai2[4] = ai1[5] + ai1[6] + ((ai1[4]>>1) + ai1[4]);
    ai2[5] = ai1[4] - ai1[7] - ((ai1[6]>>1) + ai1[6]);
    ai2[6] = ai1[4] + ai1[7] - ((ai1[5]>>1) + ai1[5]);
    ai2[7] = ai1[5] - ai1[6] + ((ai1[7]>>1) + ai1[7]);

    piCoeff[ 0] =  ai2[0]     +  ai2[1];
    piCoeff[16] =  ai2[2]     + (ai2[3]>>1);
    piCoeff[32] =  ai2[0]     -  ai2[1];
    piCoeff[48] = (ai2[2]>>1) -  ai2[3];

    piCoeff[ 8] =  ai2[4]     + (ai2[7]>>2);
    piCoeff[24] =  ai2[5]     + (ai2[6]>>2);
    piCoeff[40] =  ai2[6]     - (ai2[5]>>2);
    piCoeff[56] = (ai2[4]>>2) -  ai2[7];
  }
}



Void
Transform::xQuantDequantUniform8x8( TCoeff*             piQCoeff,
                                    TCoeff*             piCoeff,
                                    const QpParameter&  rcQp,
                                    const UChar*        pucScale,
                                    UInt&               ruiAbsSum )
{
  UInt  uiAbsSum  = 0;
  Int   iAdd      = ( 1 << 5 ) >> rcQp.per();

  for( Int n = 0; n < 64; n++ )
  {
    Int iLevel  = piCoeff[n];
    Int iSign   = iLevel;

    iLevel      = abs( iLevel ) * g_aaiQuantCoef64[ rcQp.rem() ][ n ];
    if( pucScale )
    {
      iLevel    = ( iLevel << 4 ) / pucScale[ n ];
    }
    iLevel      = ( iLevel + 2*rcQp.add() ) >> ( rcQp.bits() + 1 );

    if( 0 != iLevel )
    {
      iSign      >>= 31;
      Int iDeScale = g_aaiDequantCoef64[ rcQp.rem() ][ n ];
      uiAbsSum    += iLevel;
      iLevel      ^= iSign;
      iLevel      -= iSign;
      piQCoeff[n]  = iLevel;
      if( pucScale )
      {
        piCoeff[n]   = ( (iLevel*iDeScale*pucScale[n] + iAdd) << rcQp.per() ) >> 6;
      }
      else
      {
        piCoeff[n]   = ( (iLevel*iDeScale*16          + iAdd) << rcQp.per() ) >> 6;
      }
    }
    else
    {
      piQCoeff[n] = 0;
      piCoeff [n] = 0;
    }
  }

  ruiAbsSum   = uiAbsSum;
}



Void
Transform::xRequantUniform8x8( TCoeff*             piCoeff,
                               TCoeff*             piCoeffBase,
                               const QpParameter&  rcQp,
                               const UChar*        pucScale,
                               UInt&               ruiAbsSum )
{
  Int normAdjust[] = { 8, 9, 5, 9,   8, 9, 5, 9 };

  ruiAbsSum = 0;

  for( Int n = 0; n < 64; n++ )
  {
    Int iLevel  = piCoeff[n];
    Int iSign   = iLevel;
    iLevel     -= ( normAdjust[n/8] * normAdjust[n%8] * (Int)piCoeffBase[n] + ( 1 << 5 ) ) >> 6;
    iSign       = iLevel;

    iLevel      = abs( iLevel ) * g_aaiQuantCoef64[ rcQp.rem() ][ n ];
    if( pucScale )
    {
      iLevel    = ( iLevel << 4 ) / pucScale[ n ];
    }
    iLevel      = ( iLevel + 2*rcQp.add() ) >> ( rcQp.bits() + 1 );

    if( 0 != iLevel )
    {
      iSign      >>= 31;
      ruiAbsSum   += iLevel;
      iLevel      ^= iSign;
      iLevel      -= iSign;
      piCoeff [n]  = iLevel;
    }
    else
    {
      piCoeff [n] = 0;
    }
  }
}


// h264 namepace end
H264AVC_NAMESPACE_END
