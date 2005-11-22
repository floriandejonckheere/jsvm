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
#include "MbCoder.h"
#include "H264AVCCommonLib/Tables.h"



H264AVC_NAMESPACE_BEGIN


MbCoder::MbCoder():
  m_pcMbSymbolWriteIf( NULL ),
  m_pcRateDistortionIf( NULL ),
  m_bInitDone( false ),
  m_bCabac( false ),
  m_bPrevIsSkipped( false )
{
}


MbCoder::~MbCoder()
{
}


ErrVal MbCoder::create( MbCoder*& rpcMbCoder )
{
  rpcMbCoder = new MbCoder;

  ROT( NULL == rpcMbCoder );

  return Err::m_nOK;
}

ErrVal MbCoder::destroy()
{
  delete this;
  return Err::m_nOK;
}

ErrVal MbCoder::initSlice(  const SliceHeader& rcSH,
                            MbSymbolWriteIf*   pcMbSymbolWriteIf,
                            RateDistortionIf*  pcRateDistortionIf )
{
  ROT( NULL == pcMbSymbolWriteIf );
  ROT( NULL == pcRateDistortionIf );

  m_pcMbSymbolWriteIf = pcMbSymbolWriteIf;
  m_pcRateDistortionIf = pcRateDistortionIf;

  m_bCabac          = rcSH.getPPS().getEntropyCodingModeFlag();
  m_bPrevIsSkipped  = false;

  m_bInitDone = true;

  return Err::m_nOK;
}


ErrVal MbCoder::uninit()
{
  m_pcMbSymbolWriteIf = NULL;
  m_pcRateDistortionIf = NULL;

  m_bInitDone = false;
  return Err::m_nOK;
}






ErrVal MbCoder::encode( MbDataAccess& rcMbDataAccess,
                        MbDataAccess* pcMbDataAccessBase,
                        Int           iSpatialScalabilityType,
                        Bool          bTerminateSlice )
{
  ROF( m_bInitDone );

  Bool  bIsCoded  = true;

  //===== skip flag =====
  if( rcMbDataAccess.getSH().isH264AVCCompatible() )
  {
    bIsCoded  = ! rcMbDataAccess.isSkippedMb();
    RNOK( m_pcMbSymbolWriteIf->skipFlag( rcMbDataAccess, false ) );
  }


  if( bIsCoded )
  {
    //===== base layer mode flag and base layer refinement flag =====
    if( rcMbDataAccess.getSH().getBaseLayerId() != MSYS_UINT_MAX )
    {
      if ( pcMbDataAccessBase->getMbData().getInCropWindowFlag() == true )// TMM_ESS
      {
				if( rcMbDataAccess.getSH().getAdaptivePredictionFlag() )
				{
					RNOK  ( m_pcMbSymbolWriteIf->BLSkipFlag( rcMbDataAccess ) );

					if( ! rcMbDataAccess.getMbData().getBLSkipFlag() && (iSpatialScalabilityType != SST_RATIO_1) && ! pcMbDataAccessBase->getMbData().isIntra() )
					{
						RNOK( m_pcMbSymbolWriteIf->BLQRefFlag( rcMbDataAccess ) );
					}
				}
				else
				{
					ROF( rcMbDataAccess.getMbData().getBLSkipFlag () );
					ROT( rcMbDataAccess.getMbData().getBLQRefFlag () );
				}
// TMM_ESS {
      }
      else  // of if ( rcMbDataAccess.getMbData().getInCropWindowFlag() == true )
      {
          ROT  ( rcMbDataAccess.getMbData().getBLSkipFlag () );
          ROT  ( rcMbDataAccess.getMbData().getBLQRefFlag () );
      }
// TMM_ESS }
    }
    else
    {
      ROT  ( rcMbDataAccess.getMbData().getBLSkipFlag () );
      ROT  ( rcMbDataAccess.getMbData().getBLQRefFlag () );
    }
    
  

    //===== macroblock mode =====
    if( ! rcMbDataAccess.getMbData().getBLSkipFlag() &&
        ! rcMbDataAccess.getMbData().getBLQRefFlag()    )
    {
      MbMode  eMbModeOrg = rcMbDataAccess.getMbData().getMbMode();
      MbMode  eMbModeSet = ( eMbModeOrg == INTRA_BL ? INTRA_4X4 : eMbModeOrg );
      rcMbDataAccess.getMbData().setMbMode( eMbModeSet );
      RNOK( m_pcMbSymbolWriteIf->mbMode( rcMbDataAccess ) );
      rcMbDataAccess.getMbData().setMbMode( eMbModeOrg );

      if( rcMbDataAccess.getSH().getBaseLayerId() != MSYS_UINT_MAX && eMbModeSet == INTRA_4X4 )
      {
	    if( ( pcMbDataAccessBase->getMbData().isIntra() || !rcMbDataAccess.isConstrainedInterLayerPred( pcMbDataAccessBase ) ) &&
			pcMbDataAccessBase->getMbData().getInCropWindowFlag() ) {
          RNOK( m_pcMbSymbolWriteIf->blFlag( rcMbDataAccess ) );    
		}
      }
    }


    //===== prediction info =====
    if( ! rcMbDataAccess.getMbData().getBLSkipFlag() )
    {
      if( rcMbDataAccess.getMbData().getBLQRefFlag() )
      {
        //===== QPEL REFINEMENTS OF MOTION VECTORS =====
        if( rcMbDataAccess.getSH().isInterB() )
        {
          RNOK( xWriteMotionVectorsQPel( rcMbDataAccess, LIST_0 ) );
          RNOK( xWriteMotionVectorsQPel( rcMbDataAccess, LIST_1 ) );
        }
        else
        {
          RNOK( xWriteMotionVectorsQPel( rcMbDataAccess, LIST_0 ) );
        }
      }
      else
      {
        //===== BLOCK MODES =====
        if( rcMbDataAccess.getMbData().isInter8x8() )
        {
          RNOK( m_pcMbSymbolWriteIf->blockModes( rcMbDataAccess ) );
        }

        if( rcMbDataAccess.getMbData().isPCM() )
        {
          //===== PCM SAMPLES =====
          RNOK( m_pcMbSymbolWriteIf->samplesPCM( rcMbDataAccess ) );
        }
        else if( rcMbDataAccess.getMbData().isIntra() )
        {
          //===== INTRA PREDICTION MODES =====
          RNOK( xWriteIntraPredModes( rcMbDataAccess ) );
        }
        else
        {
          //===== MOTION INFORMATION =====
          MbMode eMbMode = rcMbDataAccess.getMbData().getMbMode();
          if( rcMbDataAccess.getSH().isInterB() )
          {
            RNOK( xWriteMotionPredFlags( rcMbDataAccess, pcMbDataAccessBase, eMbMode, LIST_0 ) );
            RNOK( xWriteMotionPredFlags( rcMbDataAccess, pcMbDataAccessBase, eMbMode, LIST_1 ) );
            RNOK( xWriteReferenceFrames( rcMbDataAccess,                     eMbMode, LIST_0 ) );
            RNOK( xWriteReferenceFrames( rcMbDataAccess,                     eMbMode, LIST_1 ) );
            RNOK( xWriteMotionVectors  ( rcMbDataAccess,                     eMbMode, LIST_0 ) );
            RNOK( xWriteMotionVectors  ( rcMbDataAccess,                     eMbMode, LIST_1 ) );
          }
          else
          {
            RNOK( xWriteMotionPredFlags( rcMbDataAccess, pcMbDataAccessBase, eMbMode, LIST_0 ) );
            RNOK( xWriteReferenceFrames( rcMbDataAccess,                     eMbMode, LIST_0 ) );
            RNOK( xWriteMotionVectors  ( rcMbDataAccess,                     eMbMode, LIST_0 ) );
          }
        }
      }
    }


    //===== TEXTURE =====
    if( ! rcMbDataAccess.getMbData().isPCM() )
    {
      Bool bTrafo8x8Flag = ( rcMbDataAccess.getSH().getPPS().getTransform8x8ModeFlag() &&
                             rcMbDataAccess.getMbData().is8x8TrafoFlagPresent()        &&
                            !rcMbDataAccess.getMbData().isIntra4x4() );
      RNOK( xWriteTextureInfo( rcMbDataAccess, rcMbDataAccess.getMbTCoeffs(), bTrafo8x8Flag ) );
    }
  }

  rcMbDataAccess.getMbData().updateResidualAvailFlags();

  //--- write terminating bit ---
  RNOK( m_pcMbSymbolWriteIf->terminatingBit ( bTerminateSlice ? 1:0 ) );

  if( bTerminateSlice )
  {
    RNOK( m_pcMbSymbolWriteIf->finishSlice() );
  }

  return Err::m_nOK;
}






ErrVal MbCoder::xWriteIntraPredModes( MbDataAccess& rcMbDataAccess )
{
  ROFRS( rcMbDataAccess.getMbData().isIntra(), Err::m_nOK );

  if( rcMbDataAccess.getMbData().isIntra4x4() )
  {
    if( rcMbDataAccess.getSH().getPPS().getTransform8x8ModeFlag() )
    {
      RNOK( m_pcMbSymbolWriteIf->transformSize8x8Flag( rcMbDataAccess ) );
    }

    if( rcMbDataAccess.getMbData().isTransformSize8x8() )
    {
      for( B8x8Idx cIdx; cIdx.isLegal(); cIdx++ )
      {
        RNOK( m_pcMbSymbolWriteIf->intraPredModeLuma( rcMbDataAccess, cIdx ) );
      }
    }
    else
    {
      for( S4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
      {
        RNOK( m_pcMbSymbolWriteIf->intraPredModeLuma( rcMbDataAccess, cIdx ) );
      }
    }
  }

  if( rcMbDataAccess.getMbData().isIntra4x4() || rcMbDataAccess.getMbData().isIntra16x16() )
  {
    RNOK( m_pcMbSymbolWriteIf->intraPredModeChroma( rcMbDataAccess ) );
  }

  return Err::m_nOK;
}




ErrVal MbCoder::xWriteBlockMv( MbDataAccess& rcMbDataAccess, B8x8Idx c8x8Idx, ListIdx eLstIdx )
{
  BlkMode eBlkMode = rcMbDataAccess.getMbData().getBlkMode( c8x8Idx.b8x8Index() );

  ParIdx8x8 eParIdx = c8x8Idx.b8x8();
  switch( eBlkMode )
  {
    case BLK_8x8:
    {
      RNOK( m_pcMbSymbolWriteIf->mvd( rcMbDataAccess, eLstIdx, eParIdx ) );
      break;
    }
    case BLK_8x4:
    {
      RNOK( m_pcMbSymbolWriteIf->mvd( rcMbDataAccess, eLstIdx, eParIdx, SPART_8x4_0 ) );
      RNOK( m_pcMbSymbolWriteIf->mvd( rcMbDataAccess, eLstIdx, eParIdx, SPART_8x4_1 ) );
      break;
    }
    case BLK_4x8:
    {
      RNOK( m_pcMbSymbolWriteIf->mvd( rcMbDataAccess, eLstIdx, eParIdx, SPART_4x8_0 ) );
      RNOK( m_pcMbSymbolWriteIf->mvd( rcMbDataAccess, eLstIdx, eParIdx, SPART_4x8_1 ) );
      break;
    }
    case BLK_4x4:
    {
      RNOK( m_pcMbSymbolWriteIf->mvd( rcMbDataAccess, eLstIdx, eParIdx, SPART_4x4_0 ) );
      RNOK( m_pcMbSymbolWriteIf->mvd( rcMbDataAccess, eLstIdx, eParIdx, SPART_4x4_1 ) );
      RNOK( m_pcMbSymbolWriteIf->mvd( rcMbDataAccess, eLstIdx, eParIdx, SPART_4x4_2 ) );
      RNOK( m_pcMbSymbolWriteIf->mvd( rcMbDataAccess, eLstIdx, eParIdx, SPART_4x4_3 ) );
      break;
    }
    case BLK_SKIP:
    {
      break;
    }
    default:
    {
      AOT(1);
      return Err::m_nERR;
    }
  }
  return Err::m_nOK;
}




ErrVal
MbCoder::xWriteMotionPredFlags( MbDataAccess&  rcMbDataAccess,
                                MbDataAccess*  pcMbDataAccessBase, 
                                MbMode         eMbMode,
                                ListIdx        eLstIdx )
{
  AOT_DBG( rcMbDataAccess.getMbData().isIntra() );

  ROFRS  ( rcMbDataAccess.getSH().getAdaptivePredictionFlag (), Err::m_nOK );
  ROF    ( pcMbDataAccessBase );

  switch( eMbMode )
  {
  case MODE_SKIP:
    {
      break;
    }

  case MODE_16x16:
    {
      if( rcMbDataAccess     .getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx) &&
          pcMbDataAccessBase->getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx)   )
      {
        RNOK( m_pcMbSymbolWriteIf->motionPredFlag( rcMbDataAccess, eLstIdx ) );
      }
      break;
    }
    
  case MODE_16x8:
    {
      if( rcMbDataAccess     .getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx) &&
          pcMbDataAccessBase->getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx)   )
      {
        RNOK( m_pcMbSymbolWriteIf->motionPredFlag( rcMbDataAccess, eLstIdx, PART_16x8_0 ) );
      }

      if( rcMbDataAccess     .getMbData().isBlockFwdBwd( B_8x8_2, eLstIdx) &&
          pcMbDataAccessBase->getMbData().isBlockFwdBwd( B_8x8_2, eLstIdx)   )
      {
        RNOK( m_pcMbSymbolWriteIf->motionPredFlag( rcMbDataAccess, eLstIdx, PART_16x8_1 ) );
      }
      break;
    }
    
  case MODE_8x16:
    {
      if( rcMbDataAccess     .getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx) &&
          pcMbDataAccessBase->getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx)   )
      {
        RNOK( m_pcMbSymbolWriteIf->motionPredFlag( rcMbDataAccess, eLstIdx, PART_8x16_0 ) );
      }

      if( rcMbDataAccess     .getMbData().isBlockFwdBwd( B_8x8_1, eLstIdx) &&
          pcMbDataAccessBase->getMbData().isBlockFwdBwd( B_8x8_1, eLstIdx)   )
      {
        RNOK( m_pcMbSymbolWriteIf->motionPredFlag( rcMbDataAccess, eLstIdx, PART_8x16_1 ) );
      }
      break;
    }
    
  case MODE_8x8:
  case MODE_8x8ref0:
    {
      for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
      {
        if( BLK_SKIP != rcMbDataAccess.getMbData().getBlkMode   ( c8x8Idx.b8x8Index() ) &&
            rcMbDataAccess            .getMbData().isBlockFwdBwd( c8x8Idx.b8x8Index(), eLstIdx) &&
            pcMbDataAccessBase       ->getMbData().isBlockFwdBwd( c8x8Idx.b8x8Index(), eLstIdx)   )
        {
          RNOK( m_pcMbSymbolWriteIf->motionPredFlag( rcMbDataAccess, eLstIdx, c8x8Idx.b8x8() ) );
        }
      }
      break;
    }
    
  default:
    {
      AOT(1);
      return Err::m_nERR;
    }
  }

  return Err::m_nOK;
}




ErrVal
MbCoder::xWriteReferenceFrames( MbDataAccess& rcMbDataAccess,
                                MbMode        eMbMode,
                                ListIdx       eLstIdx )
{
  AOT_DBG( rcMbDataAccess.getMbData().isIntra() );

  if( 1 == rcMbDataAccess.getNumActiveRef( eLstIdx ) )
  {
    return Err::m_nOK;
  }

  Bool          bPred = rcMbDataAccess.getSH().getAdaptivePredictionFlag();
  MbMotionData& rcMot = rcMbDataAccess.getMbMotionData( eLstIdx );

  switch( eMbMode )
  {
  case MODE_SKIP:
    {
      break;
    }
    
  case MODE_16x16:
    {
      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx) && ( ! bPred || ! rcMot.getMotPredFlag() ) )
      {
        RNOK( m_pcMbSymbolWriteIf->refFrame( rcMbDataAccess, eLstIdx ) );
      }
      break;
    }
    
  case MODE_16x8:
    {
      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx) && ( ! bPred || ! rcMot.getMotPredFlag( PART_16x8_0 ) ) )
      {
        RNOK( m_pcMbSymbolWriteIf->refFrame( rcMbDataAccess, eLstIdx, PART_16x8_0 ) );
      }

      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_2, eLstIdx) && ( ! bPred || ! rcMot.getMotPredFlag( PART_16x8_1 ) ) )
      {
        RNOK( m_pcMbSymbolWriteIf->refFrame( rcMbDataAccess, eLstIdx, PART_16x8_1 ) );
      }
      break;
    }
    
  case MODE_8x16:
    {
      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx) && ( ! bPred || ! rcMot.getMotPredFlag( PART_8x16_0 ) ) )
      {
        RNOK( m_pcMbSymbolWriteIf->refFrame( rcMbDataAccess, eLstIdx, PART_8x16_0 ) );
      }

      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_1, eLstIdx) && ( ! bPred || ! rcMot.getMotPredFlag( PART_8x16_1 ) ) )
      {
        RNOK( m_pcMbSymbolWriteIf->refFrame( rcMbDataAccess, eLstIdx, PART_8x16_1 ) );
      }
      break;
    }
    
  case MODE_8x8:
  case MODE_8x8ref0:
    {
      for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
      {
        if( BLK_SKIP != rcMbDataAccess.getMbData().getBlkMode( c8x8Idx.b8x8Index() ) &&
            rcMbDataAccess.getMbData().isBlockFwdBwd( c8x8Idx.b8x8Index(), eLstIdx ) && ( ! bPred || ! rcMot.getMotPredFlag( c8x8Idx.b8x8() ) ) )
        {
          RNOK( m_pcMbSymbolWriteIf->refFrame( rcMbDataAccess, eLstIdx, c8x8Idx.b8x8() ) );
        }
      }
      break;
    }
    
  default:
    {
      AOT(1);
      return Err::m_nERR;
    }
  }

  return Err::m_nOK;
}




ErrVal
MbCoder::xWriteMotionVectors( MbDataAccess& rcMbDataAccess,
                              MbMode        eMbMode,
                              ListIdx       eLstIdx )
{
  AOT_DBG( rcMbDataAccess.getMbData().isIntra() );

  switch( eMbMode )
  {
  case MODE_SKIP:
    {
      return Err::m_nOK;
    }
  
  case MODE_16x16:
    {
      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx) )
      {
        RNOK( m_pcMbSymbolWriteIf->mvd( rcMbDataAccess, eLstIdx ) );
      }
      return Err::m_nOK;
    }

  case MODE_16x8:
    {
      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx) )
      {
        RNOK( m_pcMbSymbolWriteIf->mvd( rcMbDataAccess, eLstIdx, PART_16x8_0 ) );
      }

      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_2, eLstIdx) )
      {
        RNOK( m_pcMbSymbolWriteIf->mvd( rcMbDataAccess, eLstIdx, PART_16x8_1 ) );
      }
      return Err::m_nOK;
    }
  
  case MODE_8x16:
    {
      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx) )
      {
        RNOK( m_pcMbSymbolWriteIf->mvd( rcMbDataAccess, eLstIdx, PART_8x16_0 ) );
      }

      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_1, eLstIdx) )
      {
        RNOK( m_pcMbSymbolWriteIf->mvd( rcMbDataAccess, eLstIdx, PART_8x16_1 ) );
      }
      return Err::m_nOK;
    }
  
  case MODE_8x8:
  case MODE_8x8ref0:
    {
      for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
      {
        if( rcMbDataAccess.getMbData().isBlockFwdBwd( c8x8Idx.b8x8Index(), eLstIdx ) )
        {
          RNOK( xWriteBlockMv( rcMbDataAccess, c8x8Idx, eLstIdx ) );
        }
      }
      return Err::m_nOK;
    }
   
  default:
    {
      AOT(1);
      return Err::m_nERR;
    }
  }

  return Err::m_nERR;
}



ErrVal MbCoder::xWriteMotionVectorsQPel( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx )
{
  ROTRS( rcMbDataAccess.getMbData().isIntra(), Err::m_nOK );

  switch( rcMbDataAccess.getMbData().getMbMode() )
  {
  case MODE_16x16:
    {
      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx) )
      {
        RNOK( m_pcMbSymbolWriteIf->mvdQPel( rcMbDataAccess, eLstIdx ) );
      }
      return Err::m_nOK;
    }

  case MODE_16x8:
    {
      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx) )
      {
        RNOK( m_pcMbSymbolWriteIf->mvdQPel( rcMbDataAccess, eLstIdx, PART_16x8_0 ) );
      }

      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_2, eLstIdx) )
      {
        RNOK( m_pcMbSymbolWriteIf->mvdQPel( rcMbDataAccess, eLstIdx, PART_16x8_1 ) );
      }
      return Err::m_nOK;
    }
  case MODE_8x16:
    {
      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx) )
      {
        RNOK( m_pcMbSymbolWriteIf->mvdQPel( rcMbDataAccess, eLstIdx, PART_8x16_0 ) );
      }

      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_1, eLstIdx) )
      {
        RNOK( m_pcMbSymbolWriteIf->mvdQPel( rcMbDataAccess, eLstIdx, PART_8x16_1 ) );
      }
      return Err::m_nOK;
    }
  case MODE_8x8:
  case MODE_8x8ref0:
    {
      for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
      {
        AOT( rcMbDataAccess.getMbData().getBlkMode( c8x8Idx.b8x8Index() ) != BLK_8x8 );
        if( rcMbDataAccess.getMbData().isBlockFwdBwd( c8x8Idx.b8x8Index(), eLstIdx ) )
        {
          RNOK( m_pcMbSymbolWriteIf->mvdQPel( rcMbDataAccess, eLstIdx, c8x8Idx.b8x8() ) );
        }
      }
      return Err::m_nOK;
    }
    default:
    {
      AOT(1);
      return Err::m_nERR;
    }
  }

  return Err::m_nERR;
}




ErrVal MbCoder::xWriteTextureInfo( MbDataAccess&            rcMbDataAccess,
                                   const MbTransformCoeffs& rcMbTCoeff
                                   , Bool                   bTrafo8x8Flag
                                   )
{

  Bool bWriteDQp = true;
  UInt uiCbp = rcMbDataAccess.getMbData().getMbCbp();

  if( ! rcMbDataAccess.getMbData().isIntra16x16() )
  {
    RNOK( m_pcMbSymbolWriteIf->cbp( rcMbDataAccess ) );

    bWriteDQp = ( 0 != uiCbp );
  }


  if( bTrafo8x8Flag && ( rcMbDataAccess.getMbData().getMbCbp() & 0x0F ) )
  {
    ROT( rcMbDataAccess.getMbData().isIntra16x16() );
    ROT( rcMbDataAccess.getMbData().isIntra4x4  () );
    RNOK( m_pcMbSymbolWriteIf->transformSize8x8Flag( rcMbDataAccess ) );
  }

  if( bWriteDQp )
  {
    RNOK( m_pcMbSymbolWriteIf->deltaQp( rcMbDataAccess ) );
  }

  
  if( !rcMbDataAccess.getMbData().isIntra() )
  {
    if( rcMbDataAccess.getSH().getAdaptivePredictionFlag() )
    {
       RNOK( m_pcMbSymbolWriteIf->resPredFlag( rcMbDataAccess ) );
    }
  }

  if( rcMbDataAccess.getMbData().isIntra16x16() )
  {
    RNOK( xScanLumaIntra16x16( rcMbDataAccess, rcMbTCoeff, rcMbDataAccess.getMbData().isAcCoded() ) );
    RNOK( xScanChromaBlocks  ( rcMbDataAccess, rcMbTCoeff, rcMbDataAccess.getMbData().getCbpChroma16x16() ) );

    return Err::m_nOK;
  }



  if( rcMbDataAccess.getMbData().isTransformSize8x8() )
  {
    for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
    {
      if( (uiCbp >> c8x8Idx.b8x8Index()) & 1 )
      {
        RNOK( m_pcMbSymbolWriteIf->residualBlock8x8( rcMbDataAccess, c8x8Idx, LUMA_SCAN ) );
      }
    }
  }
  else
  {
    for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
    {
      if( (uiCbp >> c8x8Idx.b8x8Index()) & 1 )
      {
        for( S4x4Idx cIdx(c8x8Idx); cIdx.isLegal(c8x8Idx); cIdx++ )
        {
          RNOK( xScanLumaBlock( rcMbDataAccess, rcMbTCoeff, cIdx ) );
        }
      }
    }
  }


  RNOK( xScanChromaBlocks( rcMbDataAccess, rcMbTCoeff, rcMbDataAccess.getMbData().getCbpChroma4x4() ) );

  return Err::m_nOK;
}






ErrVal MbCoder::xScanLumaIntra16x16( MbDataAccess& rcMbDataAccess, const MbTransformCoeffs& rcTCoeff, Bool bAC )
{
  RNOK( m_pcMbSymbolWriteIf->residualBlock( rcMbDataAccess, B4x4Idx(0), LUMA_I16_DC ) );
  ROFRS( bAC, Err::m_nOK );

  for( S4x4Idx cIdx; cIdx.isLegal(); cIdx++)
  {
    RNOK( m_pcMbSymbolWriteIf->residualBlock( rcMbDataAccess, cIdx, LUMA_I16_AC ) );
  }

  return Err::m_nOK;
}


ErrVal MbCoder::xScanLumaBlock( MbDataAccess& rcMbDataAccess, const MbTransformCoeffs& rcTCoeff, LumaIdx cIdx )
{
  RNOK( m_pcMbSymbolWriteIf->residualBlock( rcMbDataAccess, cIdx, LUMA_SCAN ) );
  return Err::m_nOK;
}


ErrVal MbCoder::xScanChromaDc( MbDataAccess& rcMbDataAccess, const MbTransformCoeffs& rcTCoeff )
{
  RNOK( m_pcMbSymbolWriteIf->residualBlock( rcMbDataAccess, CIdx(0), CHROMA_DC ) );
  RNOK( m_pcMbSymbolWriteIf->residualBlock( rcMbDataAccess, CIdx(4), CHROMA_DC ) );

  return Err::m_nOK;
}


ErrVal MbCoder::xScanChromaAcU( MbDataAccess& rcMbDataAccess, const MbTransformCoeffs& rcTCoeff )
{
  RNOK( m_pcMbSymbolWriteIf->residualBlock( rcMbDataAccess, CIdx(0), CHROMA_AC ) );
  RNOK( m_pcMbSymbolWriteIf->residualBlock( rcMbDataAccess, CIdx(1), CHROMA_AC ) );
  RNOK( m_pcMbSymbolWriteIf->residualBlock( rcMbDataAccess, CIdx(2), CHROMA_AC ) );
  RNOK( m_pcMbSymbolWriteIf->residualBlock( rcMbDataAccess, CIdx(3), CHROMA_AC ) );
  return Err::m_nOK;
}


ErrVal MbCoder::xScanChromaAcV( MbDataAccess& rcMbDataAccess, const MbTransformCoeffs& rcTCoeff )
{
  RNOK( m_pcMbSymbolWriteIf->residualBlock( rcMbDataAccess, CIdx(4), CHROMA_AC ) );
  RNOK( m_pcMbSymbolWriteIf->residualBlock( rcMbDataAccess, CIdx(5), CHROMA_AC ) );
  RNOK( m_pcMbSymbolWriteIf->residualBlock( rcMbDataAccess, CIdx(6), CHROMA_AC ) );
  RNOK( m_pcMbSymbolWriteIf->residualBlock( rcMbDataAccess, CIdx(7), CHROMA_AC ) );
  return Err::m_nOK;
}


ErrVal MbCoder::xScanChromaBlocks( MbDataAccess& rcMbDataAccess, const MbTransformCoeffs& rcTCoeff, UInt uiChromCbp )
{
  ROTRS( 1 > uiChromCbp, Err::m_nOK );

  RNOK( xScanChromaDc ( rcMbDataAccess, rcTCoeff ) );

  ROTRS( 2 > uiChromCbp, Err::m_nOK );

  RNOK( xScanChromaAcU( rcMbDataAccess, rcTCoeff ) );
  RNOK( xScanChromaAcV( rcMbDataAccess, rcTCoeff ) );
  return Err::m_nOK;
}



H264AVC_NAMESPACE_END
