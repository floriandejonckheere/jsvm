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
#include "H264AVCCommonLib/CommonTypes.h"
#include "H264AVCCommonLib/MbDataStruct.h"

#include<stdio.h>


H264AVC_NAMESPACE_BEGIN



ErrVal
MbDataStruct::save( FILE* pFile )
{
  ROF( pFile );

  UInt uiSave  = ::fwrite( this, sizeof(MbDataStruct), 1, pFile );

  ROF( uiSave == 1 );

  return Err::m_nOK;
}


ErrVal
MbDataStruct::load( FILE* pFile )
{
  ROF( pFile );

  UInt uiRead  = ::fread( this, sizeof(MbDataStruct), 1, pFile );

  ROF( uiRead == 1 );

  return Err::m_nOK;
}




const UChar MbDataStruct::m_aucACTab[6] =
{
  0,1,2,0,1,2
};

MbDataStruct::MbDataStruct()
: m_uiSliceId           ( 0 )
, m_bBLSkipFlag         ( false )
, m_eMbMode             ( MODE_SKIP )
, m_uiMbCbp             ( 0 )
, m_uiBCBP              ( 0 )
, m_usFwdBwd            ( 0 )
, m_ucChromaPredMode    ( 0 )
, m_ucQp                ( 0 )
, m_usResidualPredFlags ( 0 )
, m_bTransformSize8x8   ( false )
, m_bSkipFlag       ( true )
, m_bInCropWindowFlag ( false ) //TMM_ESS	
, m_bSmoothedRefFlag		( false )	// JVT-R091
{
  DO_DBG( clearIntraPredictionModes( true ) );
  m_aBlkMode[0] = m_aBlkMode[1] = m_aBlkMode[2] = m_aBlkMode[3] = BLK_8x8;  //TMM_ESS
}


Void MbDataStruct::reset() 
{
  m_uiBCBP              = 0;
  m_usFwdBwd            = 0;
  m_uiSliceId           = 0;
  m_bBLSkipFlag         = false;
  m_eMbMode             = MODE_SKIP;
  m_uiMbCbp             = 0;
  m_ucChromaPredMode    = 0;
  m_ucQp                = 0;
  m_usResidualPredFlags = 0;
  m_bTransformSize8x8   = 0;
  m_bInCropWindowFlag   = false; //TMM_ESS	
	m_bSmoothedRefFlag		= false; // JVT-R091
  DO_DBG( clearIntraPredictionModes( true ) );
  m_aBlkMode[0] = m_aBlkMode[1] = m_aBlkMode[2] = m_aBlkMode[3] = BLK_8x8;  //TMM_ESS  
}


Void MbDataStruct::clear()
{
  m_usFwdBwd            = 0;
  m_bBLSkipFlag         = false;
  m_eMbMode             = MODE_SKIP;
  m_uiMbCbp             = 0;
  m_ucChromaPredMode    = 0;
  m_uiBCBP              = 0;
  m_usResidualPredFlags = 0;
  m_bTransformSize8x8   = 0;
  m_bInCropWindowFlag   = false; //TMM_ESS	
	m_bSmoothedRefFlag		= false; // JVT-R091
  clearIntraPredictionModes( true );
  m_aBlkMode[0] = m_aBlkMode[1] = m_aBlkMode[2] = m_aBlkMode[3] = BLK_8x8;  //TMM_ESS
}


Void MbDataStruct::clearIntraPredictionModes( Bool bAll )
{
  ::memset( m_ascIPredMode, DC_PRED, sizeof(UChar)* 16 );
  ROFVS( bAll );
  m_ucChromaPredMode = 0;
}



Void MbDataStruct::setMbCbp( UInt uiCbp )
{
  UInt uiExtMbCbp = 0;
  UInt uiMbCbpTmp = uiCbp;

  uiExtMbCbp += (uiCbp & 0x4) ? 0x33 : 0x00;
  uiExtMbCbp += (uiCbp & 0x8) ? 0xcc : 0x00;
  uiExtMbCbp <<= 8;
  uiExtMbCbp += (uiCbp & 0x1) ? 0x33 : 0x00;
  uiExtMbCbp += (uiCbp & 0x2) ? 0xcc : 0x00;
  uiExtMbCbp |= uiMbCbpTmp << 24;

  m_uiMbCbp = uiExtMbCbp;
}



Void MbDataStruct::setAndConvertMbExtCbp( UInt uiExtCbp )
{
  UInt uiMbCbp;
  {
    UInt uiCbp = uiExtCbp;
    uiMbCbp  = (0 != (uiCbp & 0x33)) ? 1 : 0;
    uiMbCbp += (0 != (uiCbp & 0xcc)) ? 2 : 0;
    uiCbp >>= 8;
    uiMbCbp += (0 != (uiCbp & 0x33)) ? 4 : 0;
    uiMbCbp += (0 != (uiCbp & 0xcc)) ? 8 : 0;
  }
  uiMbCbp += (uiExtCbp >> 16) << 4;

  m_uiMbCbp = (uiMbCbp<<24) | uiExtCbp;
}


Void MbDataStruct::copyFrom( const MbDataStruct& rcMbDataStruct )
{
  m_usFwdBwd            = rcMbDataStruct.m_usFwdBwd;
  m_uiSliceId           = rcMbDataStruct.m_uiSliceId;
  m_bBLSkipFlag         = rcMbDataStruct.m_bBLSkipFlag;
  m_eMbMode             = rcMbDataStruct.m_eMbMode;
  m_ucQp                = rcMbDataStruct.m_ucQp;
  m_uiMbCbp             = rcMbDataStruct.m_uiMbCbp;
  m_ucChromaPredMode    = rcMbDataStruct.m_ucChromaPredMode;
  m_uiBCBP              = rcMbDataStruct.m_uiBCBP;
  m_usResidualPredFlags = rcMbDataStruct.m_usResidualPredFlags;
  m_bTransformSize8x8   = rcMbDataStruct.m_bTransformSize8x8;
	m_bSmoothedRefFlag		= rcMbDataStruct.m_bSmoothedRefFlag; // JVT-R091

  ::memcpy( m_aBlkMode,     rcMbDataStruct.m_aBlkMode,      sizeof(m_aBlkMode) );
  ::memcpy( m_ascIPredMode, rcMbDataStruct.m_ascIPredMode,  sizeof(m_ascIPredMode) );
}



ErrVal
MbDataStruct::upsampleMotion( const MbDataStruct& rcMbDataStruct, Par8x8 ePar8x8, Bool bDirect8x8 )
{
  //--- get & set fwd/bwd ---
  UShort  usFwdBwd8x8 = ( ( rcMbDataStruct.m_usFwdBwd >> ( ePar8x8 << 2 ) ) & 0x03 );
  m_usFwdBwd          = usFwdBwd8x8 + (usFwdBwd8x8<<4) + (usFwdBwd8x8<<8) + (usFwdBwd8x8<<12);

  //--- set block modes ---
  m_aBlkMode[0] = m_aBlkMode[1] = m_aBlkMode[2] = m_aBlkMode[3] = BLK_8x8;
  
  B8x8Idx c8x8Idx(ePar8x8);
  UInt uiCbp = rcMbDataStruct.getMbExtCbp() >> c8x8Idx.b4x4();

  UInt uiMbCbp = 0;
  uiMbCbp |= ((uiCbp&0x10) ? 0x33 : 0);
  uiMbCbp |= ((uiCbp&0x20) ? 0xcc : 0);
  uiMbCbp <<= 8; 
  uiMbCbp |= ((uiCbp&0x01) ? 0x33 : 0);
  uiMbCbp |= ((uiCbp&0x02) ? 0xcc : 0);

  setMbExtCbp( uiMbCbp );

  //--- set macroblock mode ---
  if      ( rcMbDataStruct.m_eMbMode >= INTRA_4X4 )
  {
    m_eMbMode = INTRA_4X4;
  }
  else if ( rcMbDataStruct.m_eMbMode == MODE_8x8 || rcMbDataStruct.m_eMbMode == MODE_8x8ref0 )
  {
    switch( rcMbDataStruct.m_aBlkMode[ePar8x8] )
    {
    case BLK_SKIP:  m_eMbMode = ( bDirect8x8 ? MODE_16x16 : MODE_8x8 );     break;
    case BLK_8x8:   m_eMbMode = MODE_16x16;   break;
    case BLK_8x4:   m_eMbMode = MODE_16x8;    break;
    case BLK_4x8:   m_eMbMode = MODE_8x16;    break;
    case BLK_4x4:   m_eMbMode = MODE_8x8;     break;
    }
  }
  else if ( rcMbDataStruct.m_eMbMode == MODE_SKIP )
  {
    m_eMbMode = ( bDirect8x8 ? MODE_16x16 : MODE_8x8 );
  }
  else
  {
    m_eMbMode = MODE_16x16;
  }

  return Err::m_nOK;
}



Bool
MbDataStruct::is8x8TrafoFlagPresent() const // only for MCTF case (skip mode)
{
  ROTRS( m_eMbMode == INTRA_BL, true  );
  ROTRS( m_eMbMode > INTRA_4X4, false );

  if( m_eMbMode == MODE_8x8 || m_eMbMode == MODE_8x8ref0 )
  {
    for( UInt n = 0; n < 4; n++ )
    {
      ROTRS( m_aBlkMode[n] > BLK_8x8, false );
    }
  }

  return true;
}

H264AVC_NAMESPACE_END
