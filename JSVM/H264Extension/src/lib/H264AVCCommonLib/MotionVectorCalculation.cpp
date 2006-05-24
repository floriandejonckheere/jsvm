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
#include "H264AVCCommonLib/FrameMng.h"
#include "H264AVCCommonLib/MotionVectorCalculation.h"



H264AVC_NAMESPACE_BEGIN


MotionVectorCalculation::MotionVectorCalculation() :
  m_uiMaxBw           ( 0 ),
  m_bSpatialDirectMode( false )
{
}


MotionVectorCalculation::~MotionVectorCalculation()
{
}


ErrVal MotionVectorCalculation::initSlice( const SliceHeader& rcSH )
{
  m_bSpatialDirectMode  = rcSH.getDirectSpatialMvPredFlag();
  m_uiMaxBw             = rcSH.isInterB() ? 2 : 1;

  return Err::m_nOK;
}


ErrVal MotionVectorCalculation::uninit()
{
  return Err::m_nOK;
}

Void MotionVectorCalculation::xCalcSDirect( MbDataAccess& rcMbDataAccess,
                                            MbDataAccess* pcMbDataAccessBase )
{
  Mv    cMv;
  SChar scRefPic;

  MbDataAccess* pTmp = rcMbDataAccess.getMbDataAccessBase();
  rcMbDataAccess.setMbDataAccessBase( NULL );

  for( UInt uiBw = 0; uiBw < 2; uiBw++ )
  {
    ListIdx       eListIdx        = ListIdx( uiBw );
    MbMotionData& rcMbMotionData  = rcMbDataAccess.getMbMotionData( eListIdx );

    if( 0 < (scRefPic = rcMbMotionData.getRefIdx() ) )
    {
      if( rcMbMotionData.getMotPredFlag() )
      {
        AOF( pcMbDataAccessBase );
        cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv();
      }
      else
      {
        rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx );
      }

      rcMbMotionData.setAllMv( cMv );
    }
  }
  rcMbDataAccess.setMbDataAccessBase( pTmp );
}



Void MotionVectorCalculation::xCalc16x16( MbDataAccess& rcMbDataAccess,
                                          MbDataAccess* pcMbDataAccessBase )
{
  Mv    cMv;
  SChar scRefPic;

  for( UInt uiBw = 0; uiBw < m_uiMaxBw; uiBw++ )
  {
    ListIdx       eListIdx        = ListIdx( uiBw );
    MbMotionData& rcMbMotionData  = rcMbDataAccess.getMbMotionData( eListIdx );
    MbMvData&     rcMbMvdData     = rcMbDataAccess.getMbMvdData   ( eListIdx );

    if( 0 < (scRefPic = rcMbMotionData.getRefIdx() ) )
    {
      if( rcMbMotionData.getMotPredFlag() )
      {
        AOF( pcMbDataAccessBase );
        cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv();
      }
      else
      {
        rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx );
      }
      cMv += rcMbMvdData.getMv();

      rcMbMotionData.setAllMv( cMv );
    }
  }
}


Void MotionVectorCalculation::xCalc16x8( MbDataAccess&  rcMbDataAccess,
                                         MbDataAccess*  pcMbDataAccessBase )
{
  Mv    cMv;
  SChar scRefPic;

  for( UInt uiBw = 0; uiBw < m_uiMaxBw; uiBw++ )
  {
    ListIdx       eListIdx        = ListIdx( uiBw );
    MbMotionData& rcMbMotionData  = rcMbDataAccess.getMbMotionData( eListIdx );
    MbMvData&     rcMbMvdData     = rcMbDataAccess.getMbMvdData   ( eListIdx );

    if( 0 < (scRefPic = rcMbMotionData.getRefIdx( PART_16x8_0 ) ) )
    {
      if( rcMbMotionData.getMotPredFlag( PART_16x8_0 ) )
      {
        AOF( pcMbDataAccessBase );
        cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( PART_16x8_0 );
      }
      else
      {
        rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, PART_16x8_0 );
      }
      cMv += rcMbMvdData.getMv( PART_16x8_0 );

      rcMbMotionData.setAllMv( cMv, PART_16x8_0 );
    }
    if( 0 < (scRefPic = rcMbMotionData.getRefIdx( PART_16x8_1 ) ) )
    {
      if( rcMbMotionData.getMotPredFlag( PART_16x8_1 ) )
      {
        AOF( pcMbDataAccessBase );
        cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( PART_16x8_1 );
      }
      else
      {
        rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, PART_16x8_1 );
      }
      cMv += rcMbMvdData.getMv( PART_16x8_1 );

      rcMbMotionData.setAllMv( cMv, PART_16x8_1 );
    }
  }
}


Void MotionVectorCalculation::xCalc8x16( MbDataAccess&  rcMbDataAccess,
                                         MbDataAccess*  pcMbDataAccessBase )
{
  Mv    cMv;
  SChar scRefPic;

  for( UInt uiBw = 0; uiBw < m_uiMaxBw; uiBw++ )
  {
    ListIdx       eListIdx        = ListIdx( uiBw );
    MbMotionData& rcMbMotionData  = rcMbDataAccess.getMbMotionData( eListIdx );
    MbMvData&     rcMbMvdData     = rcMbDataAccess.getMbMvdData   ( eListIdx );

    if( 0 < (scRefPic = rcMbMotionData.getRefIdx( PART_8x16_0 ) ) )
    {
      if( rcMbMotionData.getMotPredFlag( PART_8x16_0 ) )
      {
        AOF( pcMbDataAccessBase );
        cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( PART_8x16_0 );
      }
      else
      {
        rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, PART_8x16_0 );
      }
      cMv += rcMbMvdData.getMv( PART_8x16_0 );

      rcMbMotionData.setAllMv( cMv, PART_8x16_0 );
    }

    if( 0 < (scRefPic = rcMbMotionData.getRefIdx( PART_8x16_1 ) ) )
    {
      if( rcMbMotionData.getMotPredFlag( PART_8x16_1 ) )
      {
        AOF( pcMbDataAccessBase );
        cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( PART_8x16_1 );
      }
      else
      {
        rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, PART_8x16_1 );
      }
      cMv += rcMbMvdData.getMv( PART_8x16_1 );

      rcMbMotionData.setAllMv( cMv, PART_8x16_1 );
    }
  }
}



Void MotionVectorCalculation::xCalc8x8( B8x8Idx       c8x8Idx,
                                        MbDataAccess& rcMbDataAccess,
                                        MbDataAccess* pcMbDataAccessBase,
                                        Bool          bFaultTolerant )
{
  Mv    cMv;
  SChar scRefPic;
  UInt  uiBw;

  ParIdx8x8 eParIdx = c8x8Idx.b8x8();

  switch( rcMbDataAccess.getMbData().getBlkMode( c8x8Idx.b8x8Index() ) )
  {
    case BLK_SKIP:
    {
      MbDataAccess* pTmp = rcMbDataAccess.getMbDataAccessBase();
      rcMbDataAccess.setMbDataAccessBase( NULL );
      for( uiBw = 0; uiBw < m_uiMaxBw; uiBw++ )
      {
        ListIdx       eListIdx        = ListIdx( uiBw );
        MbMotionData& rcMbMotionData  = rcMbDataAccess.getMbMotionData( eListIdx );

        if( 0 < (scRefPic = rcMbMotionData.getRefIdx( eParIdx ) ) )
        {
          if( rcMbMotionData.getMotPredFlag( eParIdx ) )
          {
            AOF( pcMbDataAccessBase );
            cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( eParIdx );
          }
          else
          {
            rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, eParIdx );
          }
          rcMbMotionData.setAllMv( cMv, eParIdx );
        }
      }
      rcMbDataAccess.setMbDataAccessBase( pTmp );
      break;
    }
    case BLK_8x8:
    {
      for( uiBw = 0; uiBw < m_uiMaxBw; uiBw++ )
      {
        ListIdx       eListIdx        = ListIdx( uiBw );
        MbMotionData& rcMbMotionData  = rcMbDataAccess.getMbMotionData( eListIdx );
        MbMvData&     rcMbMvdData     = rcMbDataAccess.getMbMvdData   ( eListIdx );

        if( 0 < (scRefPic = rcMbMotionData.getRefIdx( eParIdx ) ) )
        {
          if( rcMbMotionData.getMotPredFlag( eParIdx ) )
          {
            AOF( pcMbDataAccessBase );
            cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( eParIdx );
          }
          else
          {
            rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, eParIdx );
          }
          cMv += rcMbMvdData.getMv( eParIdx );
          rcMbMotionData.setAllMv( cMv, eParIdx );
        }
      }
      break;
    }
    case BLK_8x4:
    {
      for( uiBw = 0; uiBw < m_uiMaxBw; uiBw++ )
      {
        ListIdx       eListIdx        = ListIdx( uiBw );
        MbMotionData& rcMbMotionData  = rcMbDataAccess.getMbMotionData( eListIdx );
        MbMvData&     rcMbMvdData     = rcMbDataAccess.getMbMvdData   ( eListIdx );

        if( 0 < (scRefPic = rcMbMotionData.getRefIdx( eParIdx ) ) )
        {
          if( rcMbMotionData.getMotPredFlag( eParIdx ) )
          {
            AOF( pcMbDataAccessBase );
            cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( eParIdx, SPART_8x4_0 );
          }
          else
          {
            rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, eParIdx, SPART_8x4_0 );
          }
          cMv +=  rcMbMvdData.getMv( eParIdx, SPART_8x4_0 );
          rcMbMotionData.setAllMv( cMv, eParIdx, SPART_8x4_0 );

          if( rcMbMotionData.getMotPredFlag( eParIdx ) )
          {
            AOF( pcMbDataAccessBase );
            cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( eParIdx, SPART_8x4_1 );
          }
          else
          {
            rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, eParIdx, SPART_8x4_1 );
          }
          cMv +=  rcMbMvdData.getMv( eParIdx, SPART_8x4_1 );
          rcMbMotionData.setAllMv( cMv, eParIdx, SPART_8x4_1 );
        }
      }
      break;
    }
    case BLK_4x8:
    {
      for( uiBw = 0; uiBw < m_uiMaxBw; uiBw++ )
      {
        ListIdx       eListIdx        = ListIdx( uiBw );
        MbMotionData& rcMbMotionData  = rcMbDataAccess.getMbMotionData( eListIdx );
        MbMvData&     rcMbMvdData     = rcMbDataAccess.getMbMvdData   ( eListIdx );

        if( 0 < (scRefPic = rcMbMotionData.getRefIdx( eParIdx ) ) )
        {
          if( rcMbMotionData.getMotPredFlag( eParIdx ) )
          {
            AOF( pcMbDataAccessBase );
            cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( eParIdx, SPART_4x8_0 );
          }
          else
          {
            rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, eParIdx, SPART_4x8_0 );
          }
          cMv +=  rcMbMvdData.getMv( eParIdx, SPART_4x8_0 );
          rcMbMotionData.setAllMv( cMv, eParIdx, SPART_4x8_0 );

          if( rcMbMotionData.getMotPredFlag( eParIdx ) )
          {
            AOF( pcMbDataAccessBase );
            cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( eParIdx, SPART_4x8_1 );
          }
          else
          {
            rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, eParIdx, SPART_4x8_1 );
          }
          cMv +=  rcMbMvdData.getMv( eParIdx, SPART_4x8_1 );
          rcMbMotionData.setAllMv( cMv, eParIdx, SPART_4x8_1 );
        }
      }
      break;
    }
    case BLK_4x4:
    {
      for( uiBw = 0; uiBw < m_uiMaxBw; uiBw++ )
      {
        ListIdx       eListIdx        = ListIdx( uiBw );
        MbMotionData& rcMbMotionData  = rcMbDataAccess.getMbMotionData( eListIdx );
        MbMvData&     rcMbMvdData     = rcMbDataAccess.getMbMvdData   ( eListIdx );

        if( 0 < (scRefPic = rcMbMotionData.getRefIdx( eParIdx ) ) )
        {
          if( rcMbMotionData.getMotPredFlag( eParIdx ) )
          {
            AOF( pcMbDataAccessBase );
            cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( eParIdx, SPART_4x4_0 );
          }
          else
          {
            rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, eParIdx, SPART_4x4_0 );
          }
          cMv +=  rcMbMvdData.getMv( eParIdx, SPART_4x4_0 );
          rcMbMotionData.setAllMv( cMv, eParIdx, SPART_4x4_0 );

          if( rcMbMotionData.getMotPredFlag( eParIdx ) )
          {
            AOF( pcMbDataAccessBase );
            cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( eParIdx, SPART_4x4_1 );
          }
          else
          {
            rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, eParIdx, SPART_4x4_1 );
          }
          cMv +=  rcMbMvdData.getMv( eParIdx, SPART_4x4_1 );
          rcMbMotionData.setAllMv( cMv, eParIdx, SPART_4x4_1 );

          if( rcMbMotionData.getMotPredFlag( eParIdx ) )
          {
            AOF( pcMbDataAccessBase );
            cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( eParIdx, SPART_4x4_2 );
          }
          else
          {
            rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, eParIdx, SPART_4x4_2 );
          }
          cMv +=  rcMbMvdData.getMv( eParIdx, SPART_4x4_2 );
          rcMbMotionData.setAllMv( cMv, eParIdx, SPART_4x4_2 );

          if( rcMbMotionData.getMotPredFlag( eParIdx ) )
          {
            AOF( pcMbDataAccessBase );
            cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( eParIdx, SPART_4x4_3 );
          }
          else
          {
            rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, eParIdx, SPART_4x4_3 );
          }
          cMv +=  rcMbMvdData.getMv( eParIdx, SPART_4x4_3 );
          rcMbMotionData.setAllMv( cMv, eParIdx, SPART_4x4_3 );
        }
      }
      break;
    }
    default:
    {
      AF();
      break;
    }
  }
}




Void MotionVectorCalculation::xCalc8x8( MbDataAccess& rcMbDataAccess,
                                        MbDataAccess* pcMbDataAccessBase,
                                        Bool          bFaultTolerant )
{
  Mv    cMv;
  SChar scRefPic;
  UInt  uiBw;

  for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
  {
    ParIdx8x8 eParIdx = c8x8Idx.b8x8();

    switch( rcMbDataAccess.getMbData().getBlkMode( c8x8Idx.b8x8Index() ) )
    {
      case BLK_SKIP:
      {
        Bool bOneMv;
        AOF( rcMbDataAccess.getMvPredictorDirect( c8x8Idx.b8x8(), bOneMv, bFaultTolerant ) );
        break;
      }
      case BLK_8x8:
      {
        for( uiBw = 0; uiBw < m_uiMaxBw; uiBw++ )
        {
          ListIdx       eListIdx        = ListIdx( uiBw );
          MbMotionData& rcMbMotionData  = rcMbDataAccess.getMbMotionData( eListIdx );
          MbMvData&     rcMbMvdData     = rcMbDataAccess.getMbMvdData   ( eListIdx );

          if( 0 < (scRefPic = rcMbMotionData.getRefIdx( eParIdx ) ) )
          {
            if( rcMbMotionData.getMotPredFlag( eParIdx ) )
            {
              AOF( pcMbDataAccessBase );
              cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( eParIdx );
            }
            else
            {
              rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, eParIdx );
            }
            cMv += rcMbMvdData.getMv( eParIdx );

            rcMbMotionData.setAllMv( cMv, eParIdx );
          }
        }
        break;
      }
      case BLK_8x4:
      {
        for( uiBw = 0; uiBw < m_uiMaxBw; uiBw++ )
        {
          ListIdx       eListIdx        = ListIdx( uiBw );
          MbMotionData& rcMbMotionData  = rcMbDataAccess.getMbMotionData( eListIdx );
          MbMvData&     rcMbMvdData     = rcMbDataAccess.getMbMvdData   ( eListIdx );

          if( 0 < (scRefPic = rcMbMotionData.getRefIdx( eParIdx ) ) )
          {
            if( rcMbMotionData.getMotPredFlag( eParIdx ) )
            {
              AOF( pcMbDataAccessBase );
              cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( eParIdx, SPART_8x4_0 );
            }
            else
            {
              rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, eParIdx, SPART_8x4_0 );
            }
            cMv +=  rcMbMvdData.getMv( eParIdx, SPART_8x4_0 );
            rcMbMotionData.setAllMv( cMv, eParIdx, SPART_8x4_0 );

            if( rcMbMotionData.getMotPredFlag( eParIdx ) )
            {
              AOF( pcMbDataAccessBase );
              cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( eParIdx, SPART_8x4_1 );
            }
            else
            {
              rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, eParIdx, SPART_8x4_1 );
            }
            cMv +=  rcMbMvdData.getMv( eParIdx, SPART_8x4_1 );
            rcMbMotionData.setAllMv( cMv, eParIdx, SPART_8x4_1 );
          }
        }
        break;
      }
      case BLK_4x8:
      {
        for( uiBw = 0; uiBw < m_uiMaxBw; uiBw++ )
        {
          ListIdx       eListIdx        = ListIdx( uiBw );
          MbMotionData& rcMbMotionData  = rcMbDataAccess.getMbMotionData( eListIdx );
          MbMvData&     rcMbMvdData     = rcMbDataAccess.getMbMvdData   ( eListIdx );

          if( 0 < (scRefPic = rcMbMotionData.getRefIdx( eParIdx ) ) )
          {
            if( rcMbMotionData.getMotPredFlag( eParIdx ) )
            {
              AOF( pcMbDataAccessBase );
              cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( eParIdx, SPART_4x8_0 );
            }
            else
            {
              rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, eParIdx, SPART_4x8_0 );
            }
            cMv +=  rcMbMvdData.getMv( eParIdx, SPART_4x8_0 );
            rcMbMotionData.setAllMv( cMv, eParIdx, SPART_4x8_0 );

            if( rcMbMotionData.getMotPredFlag( eParIdx ) )
            {
              AOF( pcMbDataAccessBase );
              cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( eParIdx, SPART_4x8_1 );
            }
            else
            {
              rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, eParIdx, SPART_4x8_1 );
            }
            cMv +=  rcMbMvdData.getMv( eParIdx, SPART_4x8_1 );
            rcMbMotionData.setAllMv( cMv, eParIdx, SPART_4x8_1 );
          }
        }
        break;
      }
      case BLK_4x4:
      {
        for( uiBw = 0; uiBw < m_uiMaxBw; uiBw++ )
        {
          ListIdx       eListIdx        = ListIdx( uiBw );
          MbMotionData& rcMbMotionData  = rcMbDataAccess.getMbMotionData( eListIdx );
          MbMvData&     rcMbMvdData     = rcMbDataAccess.getMbMvdData   ( eListIdx );

          if( 0 < (scRefPic = rcMbMotionData.getRefIdx( eParIdx ) ) )
          {
            if( rcMbMotionData.getMotPredFlag( eParIdx ) )
            {
              AOF( pcMbDataAccessBase );
              cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( eParIdx, SPART_4x4_0 );
            }
            else
            {
              rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, eParIdx, SPART_4x4_0 );
            }
            cMv +=  rcMbMvdData.getMv( eParIdx, SPART_4x4_0 );
            rcMbMotionData.setAllMv( cMv, eParIdx, SPART_4x4_0 );

            if( rcMbMotionData.getMotPredFlag( eParIdx ) )
            {
              AOF( pcMbDataAccessBase );
              cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( eParIdx, SPART_4x4_1 );
            }
            else
            {
              rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, eParIdx, SPART_4x4_1 );
            }
            cMv +=  rcMbMvdData.getMv( eParIdx, SPART_4x4_1 );
            rcMbMotionData.setAllMv( cMv, eParIdx, SPART_4x4_1 );

            if( rcMbMotionData.getMotPredFlag( eParIdx ) )
            {
              AOF( pcMbDataAccessBase );
              cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( eParIdx, SPART_4x4_2 );
            }
            else
            {
              rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, eParIdx, SPART_4x4_2 );
            }
            cMv +=  rcMbMvdData.getMv( eParIdx, SPART_4x4_2 );
            rcMbMotionData.setAllMv( cMv, eParIdx, SPART_4x4_2 );

            if( rcMbMotionData.getMotPredFlag( eParIdx ) )
            {
              AOF( pcMbDataAccessBase );
              cMv = pcMbDataAccessBase->getMbMotionData( eListIdx ).getMv( eParIdx, SPART_4x4_3 );
            }
            else
            {
              rcMbDataAccess.getMvPredictor( cMv, scRefPic, eListIdx, eParIdx, SPART_4x4_3 );
            }
            cMv +=  rcMbMvdData.getMv( eParIdx, SPART_4x4_3 );
            rcMbMotionData.setAllMv( cMv, eParIdx, SPART_4x4_3 );
          }
        }
        break;
      }
      default:
      {
        AF();
        break;
      }
    }
  }
}


H264AVC_NAMESPACE_END


