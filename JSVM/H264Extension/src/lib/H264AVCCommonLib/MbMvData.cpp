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
#include "H264AVCCommonLib/MbMvData.h"
#include "H264AVCCommonLib/FrameUnit.h"

#include<stdio.h>

H264AVC_NAMESPACE_BEGIN
 
const UInt MbMotionData::m_auiBlk2Part [16 ]  = { 0, 0, 1, 1,  0, 0, 1, 1,  2, 2, 3, 3,  2, 2, 3, 3 };  // XDIRECT


ErrVal
MbMvData::save( FILE* pFile )
{
  ROF( pFile );

  UInt uiSave = ::fwrite( &m_acMv[0], sizeof(Mv), 16, pFile );
  ROF( uiSave == 16 );

  return Err::m_nOK;
}


ErrVal
MbMvData::load( FILE* pFile )
{
  ROF( pFile );

  UInt uiRead = ::fread( &m_acMv[0], sizeof(Mv), 16, pFile );
  ROF( uiRead == 16 );

  return Err::m_nOK;
}


ErrVal
MbMotionData::save( FILE* pFile )
{
  ROF( pFile );

  RNOK( MbMvData::save( pFile ) );
  
  UInt uiSave  = ::fwrite( &m_ascRefIdx[0],   sizeof(SChar),  4, pFile );
  uiSave      += ::fwrite( &m_usMotPredFlags, sizeof(UShort), 1, pFile );
  
  ROF( uiSave == ( 4 + 1 ) );

  return Err::m_nOK;
}


ErrVal
MbMotionData::load( FILE* pFile )
{
  ROF( pFile );

  RNOK( MbMvData::load( pFile ) );
  
  UInt uiRead  = ::fread( &m_ascRefIdx[0],   sizeof(SChar),  4, pFile );
  uiRead      += ::fread( &m_usMotPredFlags, sizeof(UShort), 1, pFile );
  
  ROF( uiRead == ( 4 + 1 ) );

  return Err::m_nOK;
}


Void MbMvData::copyFrom( const MbMvData& rcMbMvData, const ParIdx8x8 eParIdx )
{
  m_acMv[ eParIdx     ] = rcMbMvData.m_acMv[ eParIdx     ];
  m_acMv[ eParIdx + 1 ] = rcMbMvData.m_acMv[ eParIdx + 1 ];
  m_acMv[ eParIdx + 4 ] = rcMbMvData.m_acMv[ eParIdx + 4 ];
  m_acMv[ eParIdx + 5 ] = rcMbMvData.m_acMv[ eParIdx + 5 ];
}

Void  MbMotionData::copyFrom( const MbMotionData& rcMbMotionData, const ParIdx8x8 eParIdx )
{
  UInt uiOffset = m_auiBlk2Part[ eParIdx ];
  m_ascRefIdx[ uiOffset ] = rcMbMotionData.m_ascRefIdx[ uiOffset ];
  m_acRefPic [ uiOffset ] = rcMbMotionData.m_acRefPic [ uiOffset ];

  setMotPredFlag( rcMbMotionData.getMotPredFlag( eParIdx ), eParIdx );

  MbMvData::copyFrom( rcMbMotionData, eParIdx );
}


Void MbMvData::copyFrom( const MbMvData& rcMbMvData )
{ 
  ::memcpy( m_acMv, rcMbMvData.m_acMv, sizeof(m_acMv) );
}

Void  MbMotionData::copyFrom( const MbMotionData& rcMbMotionData )
{
  ::memcpy( m_acRefPic,   rcMbMotionData.m_acRefPic,  4 * sizeof(RefPic) );
  ::memcpy( m_ascRefIdx,  rcMbMotionData.m_ascRefIdx, 4 * sizeof(SChar) ); 
  m_usMotPredFlags = rcMbMotionData.m_usMotPredFlags;

  MbMvData::copyFrom( rcMbMotionData );
}


ErrVal
MbMvData::upsampleMotion( const MbMvData& rcMbMvData, Par8x8 ePar8x8 )
{
  const Mv* pacMvSrc = &rcMbMvData.m_acMv[4*(ePar8x8&2)+2*(ePar8x8&1)];

  m_acMv[ 0] = m_acMv[ 1] = m_acMv[ 4] = m_acMv[ 5] = ( pacMvSrc[0] << 1 );
  m_acMv[ 2] = m_acMv[ 3] = m_acMv[ 6] = m_acMv[ 7] = ( pacMvSrc[1] << 1 );
  m_acMv[ 8] = m_acMv[ 9] = m_acMv[12] = m_acMv[13] = ( pacMvSrc[4] << 1 );
  m_acMv[10] = m_acMv[11] = m_acMv[14] = m_acMv[15] = ( pacMvSrc[5] << 1 );

  return Err::m_nOK;
}

ErrVal
MbMotionData::upsampleMotion( const MbMotionData& rcMbMotionData, Par8x8 ePar8x8 )
{
  m_ascRefIdx[0] = m_ascRefIdx[1] = m_ascRefIdx[2] = m_ascRefIdx[3] = rcMbMotionData.m_ascRefIdx[ePar8x8];
  
  m_acRefPic [0].setFrame( NULL );
  m_acRefPic [1].setFrame( NULL );
  m_acRefPic [2].setFrame( NULL );
  m_acRefPic [3].setFrame( NULL );

  RNOK( MbMvData::upsampleMotion( rcMbMotionData, ePar8x8 ) );

  return Err::m_nOK;
}

// TMM_ESS {
ErrVal
MbMotionData::upsampleMotionNonDyad( SChar* pscBl4x4RefIdx  , Mv* acBl4x4Mv , ResizeParameters* pcParameters )
{
  int iScaledBaseWidth  = pcParameters->m_iOutWidth;
  int iScaledBaseHeight = pcParameters->m_iOutHeight;
  int iBaseWidth        = pcParameters->m_iInWidth;
  int iBaseHeight       = pcParameters->m_iInHeight;
  
  for (UInt uiB8x8Idx=0 ; uiB8x8Idx<4 ; uiB8x8Idx++)
  {	
	m_acRefPic[uiB8x8Idx].setFrame(NULL);
	m_ascRefIdx[uiB8x8Idx] = pscBl4x4RefIdx[g_aucConvertTo4x4Idx[uiB8x8Idx]];
	m_ascRefIdx[uiB8x8Idx] = ((m_ascRefIdx[uiB8x8Idx]<=0)?BLOCK_NOT_PREDICTED:m_ascRefIdx[uiB8x8Idx]); 
  }

  Int   dx , dy;

  for (Int iB4x4Idx=0 ; iB4x4Idx<16 ; iB4x4Idx++ )
  {
    m_acMv[iB4x4Idx] = acBl4x4Mv[iB4x4Idx];

    dx = (Int) m_acMv[iB4x4Idx].getHor();
    dy = (Int) m_acMv[iB4x4Idx].getVer();

    Int sign;
    sign = (dx>0) ? 1 : (-1);
    dx = sign * (sign * dx * iScaledBaseWidth + iBaseWidth/2 ) / iBaseWidth;
    sign = (dy>0)?1:(-1);
    dy = sign * (sign * dy * iScaledBaseHeight + iBaseHeight/2) / iBaseHeight;

    m_acMv[iB4x4Idx].set( dx , dy );
  }

  return Err::m_nOK;
}


ErrVal
MbMotionData::upsampleMotionNonDyad(SChar*              scBl8x8RefIdx , 
                                    Mv*                 acBl4x4Mv , 
                                    ResizeParameters*   pcParameters , 
                                    Mv                  deltaMv[4] ) 
{
  upsampleMotionNonDyad( scBl8x8RefIdx , acBl4x4Mv , pcParameters );

  // PICTURE LEVEL ESS 
  for (UInt uiB8x8Idx=0 ; uiB8x8Idx<4 ; uiB8x8Idx++)
  {	
  	const UChar *b4Idx = &(g_aucConvertBlockOrder[uiB8x8Idx*4]);
    if (m_ascRefIdx[uiB8x8Idx] > 0)
    {
      m_acMv[*(b4Idx++)] += deltaMv[uiB8x8Idx];
      m_acMv[*(b4Idx++)] += deltaMv[uiB8x8Idx];
      m_acMv[*(b4Idx++)] += deltaMv[uiB8x8Idx];
      m_acMv[*b4Idx] += deltaMv[uiB8x8Idx];
     }
    else
    {
      m_acMv[*(b4Idx++)].set(0,0);
      m_acMv[*(b4Idx++)].set(0,0);
      m_acMv[*(b4Idx++)].set(0,0);
      m_acMv[*(b4Idx)].set(0,0);
    }
  }
  return Err::m_nOK;
}

// TMM_ESS }

H264AVC_NAMESPACE_END
