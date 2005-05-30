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
#include "H264AVCCommonLib/MbData.h"



H264AVC_NAMESPACE_BEGIN


ErrVal
MbData::saveAll( FILE* pFile )
{
  RNOK( MbDataStruct::          save( pFile ) );
  RNOK( m_pcMbTCoeffs         ->save( pFile ) );
  RNOK( m_apcMbMotionData [0] ->save( pFile ) );
  RNOK( m_apcMbMvdData    [0] ->save( pFile ) );
  RNOK( m_apcMbMotionData [1] ->save( pFile ) );
  RNOK( m_apcMbMvdData    [1] ->save( pFile ) );

  return Err::m_nOK;
}


ErrVal
MbData::loadAll( FILE* pFile )
{
  RNOK( MbDataStruct::          load( pFile ) );
  RNOK( m_pcMbTCoeffs         ->load( pFile ) );
  RNOK( m_apcMbMotionData [0] ->load( pFile ) );
  RNOK( m_apcMbMvdData    [0] ->load( pFile ) );
  RNOK( m_apcMbMotionData [1] ->load( pFile ) );
  RNOK( m_apcMbMvdData    [1] ->load( pFile ) );

  return Err::m_nOK;
}

ErrVal
MbData::copyMotion( MbData& rcMbData,
                    UInt    uiSliceId )
{
  m_uiSliceId   = ( uiSliceId != MSYS_UINT_MAX ? uiSliceId : m_uiSliceId );
  m_bSkipFlag   = rcMbData.m_bSkipFlag;
  m_bBLSkipFlag = rcMbData.m_bBLSkipFlag;
  m_bBLQRefFlag = rcMbData.m_bBLQRefFlag;
  m_eMbMode     = rcMbData.m_eMbMode;
  m_aBlkMode[0] = rcMbData.m_aBlkMode[0];
  m_aBlkMode[1] = rcMbData.m_aBlkMode[1];
  m_aBlkMode[2] = rcMbData.m_aBlkMode[2];
  m_aBlkMode[3] = rcMbData.m_aBlkMode[3]; 
  m_usFwdBwd    = rcMbData.m_usFwdBwd;

  m_apcMbMotionData[0]->copyFrom( *rcMbData.m_apcMbMotionData[0] );
  m_apcMbMotionData[1]->copyFrom( *rcMbData.m_apcMbMotionData[1] );

  m_apcMbMvdData[0]->copyFrom( *rcMbData.m_apcMbMvdData[0] );
  m_apcMbMvdData[1]->copyFrom( *rcMbData.m_apcMbMvdData[1] );

  return Err::m_nOK;
}


ErrVal
MbData::copyMotionBL( MbData& rcMbData,
                      Bool    bDirect8x8,
                      UInt    uiSliceId )
{
  m_uiSliceId   = ( uiSliceId != MSYS_UINT_MAX ? uiSliceId : m_uiSliceId );
  m_bBLSkipFlag = false;
  m_bBLQRefFlag = false;
  m_eMbMode     = ( rcMbData.m_eMbMode     == MODE_SKIP ? MODE_8x8   : rcMbData.m_eMbMode     );
  m_aBlkMode[0] = ( rcMbData.m_aBlkMode[0] == BLK_SKIP || rcMbData.m_eMbMode == MODE_SKIP ? ( bDirect8x8 ? BLK_8x8 : BLK_4x4 ) : rcMbData.m_aBlkMode[0] );
  m_aBlkMode[1] = ( rcMbData.m_aBlkMode[1] == BLK_SKIP || rcMbData.m_eMbMode == MODE_SKIP ? ( bDirect8x8 ? BLK_8x8 : BLK_4x4 ) : rcMbData.m_aBlkMode[1] );
  m_aBlkMode[2] = ( rcMbData.m_aBlkMode[2] == BLK_SKIP || rcMbData.m_eMbMode == MODE_SKIP ? ( bDirect8x8 ? BLK_8x8 : BLK_4x4 ) : rcMbData.m_aBlkMode[2] );
  m_aBlkMode[3] = ( rcMbData.m_aBlkMode[3] == BLK_SKIP || rcMbData.m_eMbMode == MODE_SKIP ? ( bDirect8x8 ? BLK_8x8 : BLK_4x4 ) : rcMbData.m_aBlkMode[3] );

  if( rcMbData.m_eMbMode == MODE_SKIP || rcMbData.m_eMbMode == MODE_8x8)
  {
    UInt uiFwdBwd = 0;
    for( Int n = 3; n >= 0; n--)
    {
      uiFwdBwd <<= 4;
      uiFwdBwd += (0 < rcMbData.m_apcMbMotionData[0]->getRefIdx( Par8x8(n) )) ? 1:0;
      uiFwdBwd += (0 < rcMbData.m_apcMbMotionData[1]->getRefIdx( Par8x8(n) )) ? 2:0;
    }
    m_usFwdBwd = uiFwdBwd;
  }
  else
  {
    m_usFwdBwd    = rcMbData.m_usFwdBwd;
  }


  m_apcMbMotionData[0]->copyFrom( *rcMbData.m_apcMbMotionData[0] );
  m_apcMbMotionData[1]->copyFrom( *rcMbData.m_apcMbMotionData[1] );

  m_apcMbMvdData[0]->copyFrom( *rcMbData.m_apcMbMvdData[0] );
  m_apcMbMvdData[1]->copyFrom( *rcMbData.m_apcMbMvdData[1] );

  return Err::m_nOK;
}


ErrVal
MbData::upsampleMotion( MbData& rcMbData, Par8x8 ePar8x8, Bool bDirect8x8 )
{
  RNOK( MbDataStruct::upsampleMotion( rcMbData, ePar8x8, bDirect8x8 ) );
  m_bBLSkipFlag = false;
  m_bBLQRefFlag = false;
  
  RNOK( m_apcMbMotionData[0]->upsampleMotion( *rcMbData.m_apcMbMotionData[0], ePar8x8 ) );
  RNOK( m_apcMbMotionData[1]->upsampleMotion( *rcMbData.m_apcMbMotionData[1], ePar8x8 ) );

  UInt uiFwdBwd = 0;
  uiFwdBwd += (0 < m_apcMbMotionData[0]->getRefIdx( B_8x8_0 )) ? 1:0;
  uiFwdBwd += (0 < m_apcMbMotionData[1]->getRefIdx( B_8x8_0 )) ? 2:0;
  m_usFwdBwd = (uiFwdBwd<<12)|(uiFwdBwd<<8)|(uiFwdBwd<<4)|(uiFwdBwd);

  return Err::m_nOK;
}


ErrVal
MbData::storeIntraBaseCoeffs( MbTransformCoeffs& rcCoeffs )
{
  ROT( m_pcMbIntraBaseTCoeffs );
  
  ROFRS( ( m_pcMbIntraBaseTCoeffs = new MbTransformCoeffs() ), Err::m_nERR );
  m_pcMbIntraBaseTCoeffs->copyFrom( rcCoeffs );
  return Err::m_nOK;
}



H264AVC_NAMESPACE_END
