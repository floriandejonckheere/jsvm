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



#include "H264AVCDecoderLib.h"

#include "MbSymbolReadIf.h"
#include "MbParser.h"
#include "DecError.h"


H264AVC_NAMESPACE_BEGIN


MbParser::MbParser()
: m_bInitDone         ( false )
, m_pcMbSymbolReadIf  ( 0 )
, m_bPrevIsSkipped    ( false )
{
}

MbParser::~MbParser()
{
}

ErrVal
MbParser::create( MbParser*& rpcMbParser )
{
  rpcMbParser = new MbParser;
  ROF( rpcMbParser );
  return Err::m_nOK;
}

ErrVal
MbParser::init()
{
  return Err::m_nOK;
}

ErrVal
MbParser::destroy()
{
  delete this;
  return Err::m_nOK;
}

ErrVal 
MbParser::initSlice( MbSymbolReadIf* pcMbSymbolReadIf )
{
  ROF( pcMbSymbolReadIf );
  m_pcMbSymbolReadIf  = pcMbSymbolReadIf;
  m_bPrevIsSkipped    = false;
  m_bInitDone         = true;
  return Err::m_nOK;
}

ErrVal 
MbParser::uninit()
{
  m_pcMbSymbolReadIf  = 0;
  m_bInitDone         = false;
  return Err::m_nOK;
}


ErrVal
MbParser::read( MbDataAccess&  rcMbDataAccess,
                UInt           uiNumMbRead,
                Bool&          rbEndOfSlice )
{
  ROF( m_bInitDone );

  ROTRS( xCheckSkipSliceMb( rcMbDataAccess, uiNumMbRead, rbEndOfSlice ), Err::m_nOK );

  Bool bIsCoded = true;
  if( m_pcMbSymbolReadIf->isMbSkipped( rcMbDataAccess ) )
  {
    bIsCoded = false;
    rcMbDataAccess.getMbTCoeffs().clear();
    rcMbDataAccess.getMbData().clearIntraPredictionModes( true );
    RNOK( xSkipMb( rcMbDataAccess ) );
    rcMbDataAccess.getMbData().setBLSkipFlag( false );
    rcMbDataAccess.getMbData().setResidualPredFlag( false );
    if( rcMbDataAccess.getSH().isBSlice() )
    {
      rcMbDataAccess.getMbData().setFwdBwd( 0x3333 );
      rcMbDataAccess.getMbMotionData( LIST_0 ).clear( RefIdxValues(1) );
      rcMbDataAccess.getMbMvdData   ( LIST_0 ).clear();
      rcMbDataAccess.getMbMotionData( LIST_1 ).clear( RefIdxValues(1) );
      rcMbDataAccess.getMbMvdData   ( LIST_1 ).clear();
    }
    else
    {
      rcMbDataAccess.getMbData().setFwdBwd( 0x1111 );
      rcMbDataAccess.getMbMotionData( LIST_0 ).clear( RefIdxValues(1) );
      rcMbDataAccess.getMbMvdData   ( LIST_0 ).clear();
    }
    rcMbDataAccess.resetQp(); // set QP to that of the last macroblock 
  }

  if( bIsCoded )
  {
    if( rcMbDataAccess.getSH().isMbaffFrame() && ( rcMbDataAccess.isTopMb() || m_bPrevIsSkipped ) )
    {
      RNOK( m_pcMbSymbolReadIf->fieldFlag( rcMbDataAccess) );
    }

    Bool bBaseLayerAvailable = ! rcMbDataAccess.getSH().getNoInterLayerPredFlag();

      //===== base layer mode flag and base layer refinement flag =====
    if( bBaseLayerAvailable )
    {
      if ( rcMbDataAccess.getMbData().getInCropWindowFlag() == true )
      {
			  if( rcMbDataAccess.getSH().getAdaptiveBaseModeFlag() )
			  {
          m_pcMbSymbolReadIf->isBLSkipped( rcMbDataAccess );
			  }
			  else
			  {
          rcMbDataAccess.getMbData().setBLSkipFlag( rcMbDataAccess.getSH().getDefaultBaseModeFlag() );
			  }
      }
      else
      {
          rcMbDataAccess.getMbData().setBLSkipFlag( false );
      }
    }
    else
    {
        rcMbDataAccess.getMbData().setBLSkipFlag( false );
    }

    //===== macroblock mode =====
    if( ! rcMbDataAccess.getMbData().getBLSkipFlag() )
    {
      DECRNOK( m_pcMbSymbolReadIf->mbMode( rcMbDataAccess ) );
    }

    if( rcMbDataAccess.getMbData().getBLSkipFlag() )
    {
      //===== copy motion data from base layer ======
      rcMbDataAccess.getMbMvdData( LIST_0 ).clear();
      rcMbDataAccess.getMbMvdData( LIST_1 ).clear();
      rcMbDataAccess.getMbData().setBLSkipFlag( true  );
    }
    else
    {
      //===== BLOCK MODES =====
      if( rcMbDataAccess.getMbData().isInter8x8() )
      {
        DECRNOK( m_pcMbSymbolReadIf->blockModes( rcMbDataAccess ) );

        //===== set motion data for skip block mode =====
        UInt  uiFwdBwd = 0;

        for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
        {
          UInt  uiBlkFwdBwd = rcMbDataAccess.getMbData().getBlockFwdBwd( c8x8Idx.b8x8Index() );

          if( rcMbDataAccess.getMbData().getBlkMode( c8x8Idx.b8x8Index() ) == BLK_SKIP )
          {
            uiBlkFwdBwd = 3;
            rcMbDataAccess.getMbMotionData( LIST_0 ).setRefIdx( 1,            c8x8Idx.b8x8() );
            rcMbDataAccess.getMbMotionData( LIST_1 ).setRefIdx( 1,            c8x8Idx.b8x8() );
            rcMbDataAccess.getMbMvdData   ( LIST_0 ).setAllMv ( Mv::ZeroMv(), c8x8Idx.b8x8() );
            rcMbDataAccess.getMbMvdData   ( LIST_1 ).setAllMv ( Mv::ZeroMv(), c8x8Idx.b8x8() );
          }

          uiFwdBwd |= ( uiBlkFwdBwd << ( c8x8Idx.b8x8Index() * 4 ) );
        }

        rcMbDataAccess.getMbData().setFwdBwd( uiFwdBwd );
      }
      rcMbDataAccess.resetQp(); // set QP to that of the last macroblock 


      //===== MOTION DATA =====
      MbMode eMbMode = rcMbDataAccess.getMbData().getMbMode();
      if( rcMbDataAccess.getMbData().isIntra() )
      {
        //===== clear mtoion data for intra blocks =====
        rcMbDataAccess.getMbMotionData( LIST_0 ).clear( BLOCK_NOT_PREDICTED );
        rcMbDataAccess.getMbMvdData   ( LIST_0 ).clear();
        if( rcMbDataAccess.getSH().isBSlice() )
        {
          rcMbDataAccess.getMbMotionData( LIST_1 ).clear( BLOCK_NOT_PREDICTED );
          rcMbDataAccess.getMbMvdData   ( LIST_1 ).clear();
        }
      }
      else if( eMbMode == MODE_SKIP )
      {
        if( rcMbDataAccess.getSH().isBSlice() )
        {
          rcMbDataAccess.getMbData().setFwdBwd( 0x3333 );
          rcMbDataAccess.getMbMotionData( LIST_0 ).clear( RefIdxValues(1) );
          rcMbDataAccess.getMbMvdData   ( LIST_0 ).clear();
          rcMbDataAccess.getMbMotionData( LIST_1 ).clear( RefIdxValues(1) );
          rcMbDataAccess.getMbMvdData   ( LIST_1 ).clear();
        }
        else
        {
          rcMbDataAccess.getMbData().setFwdBwd( 0x1111 );
          rcMbDataAccess.getMbMotionData( LIST_0 ).clear( RefIdxValues(1) );
          rcMbDataAccess.getMbMvdData   ( LIST_0 ).clear();
          rcMbDataAccess.getMbMotionData( LIST_1 ).clear( BLOCK_NOT_PREDICTED );
          rcMbDataAccess.getMbMvdData   ( LIST_1 ).clear();
        }
      }
      else
      {
        if( rcMbDataAccess.getSH().isBSlice() )
        {
          DECRNOK( xReadMotionPredFlags         ( rcMbDataAccess, eMbMode, LIST_0 ) );
          DECRNOK( xReadMotionPredFlags         ( rcMbDataAccess, eMbMode, LIST_1 ) );
          DECRNOK( xReadReferenceFramesNoRefPic ( rcMbDataAccess, eMbMode, LIST_0 ) );
          DECRNOK( xReadReferenceFramesNoRefPic ( rcMbDataAccess, eMbMode, LIST_1 ) );
          DECRNOK( xReadMotionVectors           ( rcMbDataAccess, eMbMode, LIST_0 ) );
          DECRNOK( xReadMotionVectors           ( rcMbDataAccess, eMbMode, LIST_1 ) );
        }
        else
        {
          DECRNOK( xReadMotionPredFlags         ( rcMbDataAccess, eMbMode, LIST_0 ) );
          DECRNOK( xReadReferenceFramesNoRefPic ( rcMbDataAccess, eMbMode, LIST_0 ) );
          DECRNOK( xReadMotionVectors           ( rcMbDataAccess, eMbMode, LIST_0 ) );
        }
      }
    }

    //===== TEXTURE INFO =====
    if( rcMbDataAccess.getMbData().isPCM() )
    {
      DECRNOK( m_pcMbSymbolReadIf->samplesPCM( rcMbDataAccess ) );
    }
    else
    {
      if( ! rcMbDataAccess.getMbData().getBLSkipFlag() )
      {
        DECRNOK( xReadIntraPredModes( rcMbDataAccess ) );
      }

      Bool bTrafo8x8Flag =  ( rcMbDataAccess.getSH().getPPS().getTransform8x8ModeFlag() &&
                            ( rcMbDataAccess.getMbData().getBLSkipFlag() ||
                              ( rcMbDataAccess.getMbData().is8x8TrafoFlagPresent(rcMbDataAccess.getSH().getSPS().getDirect8x8InferenceFlag()) &&
                               !rcMbDataAccess.getMbData().isIntra4x4() ) ) );
      bBaseLayerAvailable = ! rcMbDataAccess.getSH().getNoInterLayerPredFlag();

      DECRNOK( xReadTextureInfo( rcMbDataAccess,
                                 bTrafo8x8Flag,
                                 bBaseLayerAvailable,
                                 rcMbDataAccess.getSH().getScanIdxStart(),
                                 rcMbDataAccess.getSH().getScanIdxStop() ) );
    }
  }
  m_bPrevIsSkipped = ! bIsCoded;

  if( rcMbDataAccess.getSH().isMbaffFrame() && ( rcMbDataAccess.isTopMb() ) )
  {
    rbEndOfSlice = false;
    return Err::m_nOK;
  }

  //===== terminating bits =====
  rbEndOfSlice = m_pcMbSymbolReadIf->isEndOfSlice();

  return Err::m_nOK;
}


ErrVal 
MbParser::xReadIntraPredModes( MbDataAccess& rcMbDataAccess )
{
  ROFRS( rcMbDataAccess.getMbData().isIntra(), Err::m_nOK );

  if( rcMbDataAccess.getMbData().isIntra4x4() )
  {
    if( rcMbDataAccess.getSH().getPPS().getTransform8x8ModeFlag() )
    {
      DECRNOK( m_pcMbSymbolReadIf->transformSize8x8Flag( rcMbDataAccess ) );
    }

    if( rcMbDataAccess.getMbData().isTransformSize8x8() )
    {
      for( B8x8Idx cIdx; cIdx.isLegal(); cIdx++ )
      {
        DECRNOK( m_pcMbSymbolReadIf->intraPredModeLuma8x8( rcMbDataAccess, cIdx ) );
      }
    }
    else
    {
      for( S4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
      {
        DECRNOK( m_pcMbSymbolReadIf->intraPredModeLuma( rcMbDataAccess, cIdx ) );
      }
    }
  }

  if( rcMbDataAccess.getMbData().isIntra4x4() || rcMbDataAccess.getMbData().isIntra16x16() )
  {
    DECRNOK( m_pcMbSymbolReadIf->intraPredModeChroma( rcMbDataAccess ) );
  }

  return Err::m_nOK;
}


ErrVal 
MbParser::xGet8x8BlockMv( MbDataAccess& rcMbDataAccess, B8x8Idx c8x8Idx, ListIdx eLstIdx )
{
  ParIdx8x8 eParIdx = c8x8Idx.b8x8();

  switch( rcMbDataAccess.getMbData().getBlkMode( c8x8Idx.b8x8Index() ) )
  {
    case BLK_8x8:
    {
      DECRNOK( m_pcMbSymbolReadIf->mvd( rcMbDataAccess, eLstIdx, eParIdx ) );
      break;
    }
    case BLK_8x4:
    {
      DECRNOK( m_pcMbSymbolReadIf->mvd( rcMbDataAccess, eLstIdx, eParIdx, SPART_8x4_0 ) );
      DECRNOK( m_pcMbSymbolReadIf->mvd( rcMbDataAccess, eLstIdx, eParIdx, SPART_8x4_1 ) );
      break;
    }
    case BLK_4x8:
    {
      DECRNOK( m_pcMbSymbolReadIf->mvd( rcMbDataAccess, eLstIdx, eParIdx, SPART_4x8_0 ) );
      DECRNOK( m_pcMbSymbolReadIf->mvd( rcMbDataAccess, eLstIdx, eParIdx, SPART_4x8_1 ) );
      break;
    }
    case BLK_4x4:
    {
      DECRNOK( m_pcMbSymbolReadIf->mvd( rcMbDataAccess, eLstIdx, eParIdx, SPART_4x4_0 ) );
      DECRNOK( m_pcMbSymbolReadIf->mvd( rcMbDataAccess, eLstIdx, eParIdx, SPART_4x4_1 ) );
      DECRNOK( m_pcMbSymbolReadIf->mvd( rcMbDataAccess, eLstIdx, eParIdx, SPART_4x4_2 ) );
      DECRNOK( m_pcMbSymbolReadIf->mvd( rcMbDataAccess, eLstIdx, eParIdx, SPART_4x4_3 ) );
      break;
    }
    case BLK_SKIP:
    {
      break;
    }
    default:
    {
      return Err::m_nERR;
    }
  }

  return Err::m_nOK;
}


ErrVal 
MbParser::xReadReferenceFramesNoRefPic( MbDataAccess& rcMbDataAccess, MbMode eMbMode, ListIdx eLstIdx )
{
  MbMotionData& rcMbMotionData  = rcMbDataAccess.getMbMotionData( eLstIdx );
  Bool          bPred           = rcMbDataAccess.getSH().getAdaptiveBaseModeFlag();

  if( rcMbDataAccess.getMbData().isIntra() )
  {
    rcMbMotionData.setRefIdx( -1 );
    return Err::m_nOK;
  }

  switch( eMbMode )
  {
    case MODE_SKIP:
      {
        break;
      }
    case MODE_16x16:
      {
        if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx ) )
        {
          if( !bPred || !rcMbMotionData.getMotPredFlag() )
          {
            if( 1 == rcMbDataAccess.getNumActiveRef( eLstIdx ) )
            {
              rcMbMotionData.setRefIdx( 1 );
            }
            else
            {
              DECRNOK( m_pcMbSymbolReadIf->refFrame( rcMbDataAccess, eLstIdx ) );
            }
          }
        }
        else
        {
          rcMbMotionData.setRefIdx( BLOCK_NOT_PREDICTED );
        }
        break;
      }
    case MODE_16x8:
      {
        if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx ) )
        {
          if( !bPred || !rcMbMotionData.getMotPredFlag(PART_16x8_0) )
          {
            if( 1 == rcMbDataAccess.getNumActiveRef( eLstIdx ) )
            {
              rcMbMotionData.setRefIdx( 1, PART_16x8_0 );
            }
            else
            {
              DECRNOK( m_pcMbSymbolReadIf->refFrame( rcMbDataAccess, eLstIdx, PART_16x8_0 ) );
            }
          }
        }
        else
        {
          rcMbMotionData.setRefIdx( BLOCK_NOT_PREDICTED, PART_16x8_0 );
        }

        if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_2, eLstIdx )  )
        {
          if( !bPred || !rcMbMotionData.getMotPredFlag(PART_16x8_1) )
          {
            if( 1 == rcMbDataAccess.getNumActiveRef( eLstIdx ) )
            {
              rcMbMotionData.setRefIdx( 1, PART_16x8_1 );
            }
            else
            {
              DECRNOK( m_pcMbSymbolReadIf->refFrame( rcMbDataAccess, eLstIdx, PART_16x8_1 ) );
            }
          }
        }
        else
        {
          rcMbMotionData.setRefIdx( BLOCK_NOT_PREDICTED, PART_16x8_1 );
        }
        break;
      }
    case MODE_8x16:
      {
        if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx ) )
        {
          if( !bPred || !rcMbMotionData.getMotPredFlag(PART_8x16_0) )
          {
            if( 1 == rcMbDataAccess.getNumActiveRef( eLstIdx ) )
            {
              rcMbMotionData.setRefIdx( 1, PART_8x16_0 );
            }
            else
            {
              DECRNOK( m_pcMbSymbolReadIf->refFrame( rcMbDataAccess, eLstIdx, PART_8x16_0 ) );
            }
          }
        }
        else
        {
          rcMbMotionData.setRefIdx( BLOCK_NOT_PREDICTED, PART_8x16_0 );
        }

        if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_1, eLstIdx ) )
        {
          if( !bPred || !rcMbMotionData.getMotPredFlag(PART_8x16_1) )
          {
            if( 1 == rcMbDataAccess.getNumActiveRef( eLstIdx ) )
            {
              rcMbMotionData.setRefIdx( 1, PART_8x16_1 );
            }
            else
            {
              DECRNOK( m_pcMbSymbolReadIf->refFrame( rcMbDataAccess, eLstIdx, PART_8x16_1 ) );
            }
          }
        }
        else
        {
          rcMbMotionData.setRefIdx( BLOCK_NOT_PREDICTED, PART_8x16_1 );
        }
        break;
      }
    case MODE_8x8:
      {
        for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
        {
          if( BLK_SKIP != rcMbDataAccess.getMbData().getBlkMode( c8x8Idx.b8x8Index() ) )
          {
            if( rcMbDataAccess.getMbData().isBlockFwdBwd( c8x8Idx.b8x8Index(), eLstIdx ) )
            {
              if( !bPred || !rcMbMotionData.getMotPredFlag(c8x8Idx.b8x8()) )
              {
                if( 1 == rcMbDataAccess.getNumActiveRef( eLstIdx ) )
                {
                  rcMbMotionData.setRefIdx( 1, c8x8Idx.b8x8() );
                }
                else
                {
                  DECRNOK( m_pcMbSymbolReadIf->refFrame( rcMbDataAccess, eLstIdx, c8x8Idx.b8x8() ) );
                }
              }
            }
            else
            {
              rcMbMotionData.setRefIdx( BLOCK_NOT_PREDICTED, c8x8Idx.b8x8() );
            }
          }
        }
        break;
      }
    case MODE_8x8ref0:
      {
        rcMbMotionData.setRefIdx( 1 );
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


ErrVal
MbParser::xReadMotionPredFlags( MbDataAccess&  rcMbDataAccess,
                                MbMode         eMbMode,
                                ListIdx        eLstIdx )
{
  ROTRS( rcMbDataAccess.getSH     ().getNoInterLayerPredFlag(),  Err::m_nOK );
  ROFRS( rcMbDataAccess.getMbData ().getInCropWindowFlag(),      Err::m_nOK );

  MbMotionData& rcMbMotionData = rcMbDataAccess.getMbMotionData( eLstIdx );

  rcMbMotionData.setMotPredFlag( rcMbDataAccess.getSH().getDefaultMotionPredictionFlag() );
  ROFRS ( rcMbDataAccess.getSH().getAdaptiveMotionPredictionFlag(), Err::m_nOK );
  
  //--- clear ---
  rcMbMotionData.setMotPredFlag( false );

  if( rcMbDataAccess.getMbData().isIntra() )
  {
    return Err::m_nOK;
  }

  switch( eMbMode )
  {
  case MODE_SKIP:
    {
      break;
    }
  case MODE_16x16:
    {
      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx ) )
      {
        DECRNOK( m_pcMbSymbolReadIf->motionPredFlag( rcMbDataAccess, eLstIdx ) );
      }
      break;
    }
  case MODE_16x8:
    {
      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx ) )
      {
        DECRNOK( m_pcMbSymbolReadIf->motionPredFlag( rcMbDataAccess, eLstIdx, PART_16x8_0 ) );
      }

      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_2, eLstIdx ) )
      {
        DECRNOK( m_pcMbSymbolReadIf->motionPredFlag( rcMbDataAccess, eLstIdx, PART_16x8_1 ) );
      }
      break;
    }
  case MODE_8x16:
    {
      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx ) )
      {
        DECRNOK( m_pcMbSymbolReadIf->motionPredFlag( rcMbDataAccess, eLstIdx, PART_8x16_0 ) );
      }

      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_1, eLstIdx ) )
      {
        DECRNOK( m_pcMbSymbolReadIf->motionPredFlag( rcMbDataAccess, eLstIdx, PART_8x16_1 ) );
      }
      break;
    }
  case MODE_8x8:
  case MODE_8x8ref0:
    {
      for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
      {
        if( BLK_SKIP != rcMbDataAccess.getMbData().getBlkMode   ( c8x8Idx.b8x8Index() ) &&
            rcMbDataAccess            .getMbData().isBlockFwdBwd( c8x8Idx.b8x8Index(), eLstIdx ) )
        {
          DECRNOK( m_pcMbSymbolReadIf->motionPredFlag( rcMbDataAccess, eLstIdx, c8x8Idx.b8x8() ) );
        }
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


ErrVal 
MbParser::xReadMotionVectors( MbDataAccess& rcMbDataAccess, MbMode eMbMode, ListIdx eLstIdx )
{
  ROTRS( rcMbDataAccess.getMbData().isIntra(), Err::m_nOK );

  switch( eMbMode )
  {
  case MODE_SKIP:
    {
      return Err::m_nOK;
    }
  case MODE_16x16:
    {
      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx ) )
      {
        DECRNOK( m_pcMbSymbolReadIf->mvd( rcMbDataAccess, eLstIdx ) );
      }
      return Err::m_nOK;
    }
  case MODE_16x8:
    {
      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx ) )
      {
        DECRNOK( m_pcMbSymbolReadIf->mvd( rcMbDataAccess, eLstIdx, PART_16x8_0 ) );
      }

      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_2, eLstIdx ) )
      {
        DECRNOK( m_pcMbSymbolReadIf->mvd( rcMbDataAccess, eLstIdx, PART_16x8_1 ) );
      }
      return Err::m_nOK;
    }
  case MODE_8x16:
    {
      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx ) )
      {
        DECRNOK( m_pcMbSymbolReadIf->mvd( rcMbDataAccess, eLstIdx, PART_8x16_0 ) );
      }

      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_1, eLstIdx ) )
      {
        DECRNOK( m_pcMbSymbolReadIf->mvd( rcMbDataAccess, eLstIdx, PART_8x16_1 ) );
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
          DECRNOK( xGet8x8BlockMv( rcMbDataAccess, c8x8Idx, eLstIdx ) );
        }
      }
      return Err::m_nOK;
    }
  default:
    {
      AF();
      return Err::m_nERR;
    }
  }
  return Err::m_nERR;
}


ErrVal
MbParser::xReadTextureInfo( MbDataAccess&   rcMbDataAccess,
                            Bool						bTrafo8x8Flag,
                            Bool            bBaseLayerAvailable,
                            UInt            uiStart,
                            UInt            uiStop )
{
  Bool bReadDQp = true;

  if( (rcMbDataAccess.getMbData().getBLSkipFlag() || !rcMbDataAccess.getMbData().isIntra()) &&
      (rcMbDataAccess.getSH().getAdaptiveResidualPredictionFlag() && !rcMbDataAccess.getSH().isIntraSlice()) )
  {
    if( rcMbDataAccess.getMbData().getInCropWindowFlag() )
    {
      DECRNOK( m_pcMbSymbolReadIf->resPredFlag( rcMbDataAccess ) );
    }
    else
    {
      rcMbDataAccess.getMbData().setResidualPredFlag( false );
    }
  }
  else
  {
    rcMbDataAccess.getMbData().setResidualPredFlag( ( rcMbDataAccess.getSH().isIntraSlice() && rcMbDataAccess.getMbData().getBLSkipFlag() ) ||
                                                    (!rcMbDataAccess.getSH().isIntraSlice() && rcMbDataAccess.getMbData().getBLSkipFlag() && !rcMbDataAccess.getSH().getNoInterLayerPredFlag() ) );
  }

  if( rcMbDataAccess.getMbData().getBLSkipFlag() || 
     !rcMbDataAccess.getMbData().isIntra16x16() )
  {
    if( uiStart == uiStop )
    {
      rcMbDataAccess.getMbData().setMbCbp( 0 );
    }
    else
    {
      DECRNOK( m_pcMbSymbolReadIf->cbp( rcMbDataAccess, uiStart, uiStop ) );
    }
    bReadDQp = rcMbDataAccess.getMbData().getMbCbp() != 0;
  }
  
  if( bTrafo8x8Flag && ( rcMbDataAccess.getMbData().getMbCbp() & 0x0F ) )
  {
    DECRNOK( m_pcMbSymbolReadIf->transformSize8x8Flag( rcMbDataAccess ) );
  }

  if( bReadDQp )
  {
    DECRNOK( m_pcMbSymbolReadIf->deltaQp( rcMbDataAccess ) );
  }
  else
  {
    rcMbDataAccess.resetQp();
  }

  UInt uiDummy     = 0;
  Bool bIntra16x16 = ( !rcMbDataAccess.getMbData().getBLSkipFlag() &&
                        rcMbDataAccess.getMbData().isIntra16x16 ()   );
  if( bIntra16x16 )
  {
    DECRNOK( m_pcMbSymbolReadIf->residualBlock( rcMbDataAccess, B4x4Idx(0), LUMA_I16_DC, uiDummy ) );

    if( rcMbDataAccess.getMbData().isAcCoded() )
    {
      for( S4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
      {
        DECRNOK( m_pcMbSymbolReadIf->residualBlock( rcMbDataAccess, cIdx, LUMA_I16_AC, uiDummy ) );
      }
      rcMbDataAccess.getMbData().setMbCbp( 0xf + ( rcMbDataAccess.getMbData().getCbpChroma16x16() << 4) );
    }

    DECRNOK( xScanChromaBlocks( rcMbDataAccess,  rcMbDataAccess.getMbData().getCbpChroma16x16() ) );
    return Err::m_nOK;
  }


  UInt uiMbExtCbp = rcMbDataAccess.getMbData().getMbExtCbp();

  if( rcMbDataAccess.getMbData().isTransformSize8x8() )
  {
    for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
    {
      if( uiMbExtCbp & ( 1 << c8x8Idx.b4x4() ) )
      {
        DECRNOK( m_pcMbSymbolReadIf->residualBlock8x8( rcMbDataAccess, c8x8Idx, uiStart, uiStop ) );
      }
    }
  }
  else
  {
    for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
    {
      if( uiMbExtCbp & ( 1 << c8x8Idx.b4x4() ) )
      {
        for( S4x4Idx cIdx( c8x8Idx ); cIdx.isLegal( c8x8Idx ); cIdx++ )
        {
          DECRNOK( m_pcMbSymbolReadIf->residualBlock( rcMbDataAccess, cIdx , LUMA_SCAN, uiMbExtCbp, uiStart, uiStop ) );
        }
      }
    }

  }
  rcMbDataAccess.getMbData().setMbExtCbp( uiMbExtCbp );

  DECRNOK( xScanChromaBlocks  ( rcMbDataAccess,  rcMbDataAccess.getMbData().getCbpChroma4x4(), uiStart, uiStop ) );
  return Err::m_nOK;
}


ErrVal
MbParser::xScanChromaBlocks( MbDataAccess& rcMbDataAccess, UInt uiChromCbp, UInt uiStart, UInt uiStop )
{
  ROTRS( 1 > uiChromCbp, Err::m_nOK );

  if( uiStart == 0 )
  {
    DECRNOK( m_pcMbSymbolReadIf->residualBlock( rcMbDataAccess,  CIdx(0), CHROMA_DC, uiStart, uiStop ) );
    DECRNOK( m_pcMbSymbolReadIf->residualBlock( rcMbDataAccess,  CIdx(4), CHROMA_DC, uiStart, uiStop ) );
  }

  ROTRS( 2 > uiChromCbp, Err::m_nOK );

  for( CIdx cCIdx; cCIdx.isLegal(); cCIdx++ )
  {
    if( uiStop > 1 )
    {
      DECRNOK( m_pcMbSymbolReadIf->residualBlock( rcMbDataAccess,  cCIdx, CHROMA_AC, uiStart, uiStop ) );
    }
  }
  return Err::m_nOK;
}


Bool
MbParser::xCheckSkipSliceMb( MbDataAccess& rcMbDataAccess, UInt uiNumMbRead, Bool& rbEndOfSlice )
{
  ROFRS( rcMbDataAccess.getSH().getSliceSkipFlag(), false );

  MbData& rcMbData = rcMbDataAccess.getMbData();
  rcMbData.setSkipFlag          ( false );
  rcMbData.setBLSkipFlag        ( true );
  rcMbData.setResidualPredFlag  ( true );
  rcMbData.setMbExtCbp          ( 0 );
  rcMbData.setFwdBwd            ( 0 );
  rcMbDataAccess.resetQp        ();
  rcMbDataAccess.getMbMotionData( LIST_0 ).clear( BLOCK_NOT_PREDICTED );
  rcMbDataAccess.getMbMotionData( LIST_1 ).clear( BLOCK_NOT_PREDICTED );
  rcMbDataAccess.getMbMvdData   ( LIST_0 ).clear();
  rcMbDataAccess.getMbMvdData   ( LIST_1 ).clear();

  rbEndOfSlice = ( uiNumMbRead >= rcMbDataAccess.getSH().getNumMbsInSliceMinus1() );
  return true;
}


ErrVal
MbParser::xSkipMb( MbDataAccess& rcMbDataAccess )
{
  if( rcMbDataAccess.getSH().isBSlice() )
  {
    RNOK( rcMbDataAccess.setConvertMbType( 0 ) );
  }
  else
  {
    rcMbDataAccess.getMbMotionData( LIST_0 ).setRefIdx( 1 );
    RNOK( rcMbDataAccess.setConvertMbType( MSYS_UINT_MAX ) );
  }

  rcMbDataAccess.getMbData().setMbCbp( 0 );
  return Err::m_nOK;
}


H264AVC_NAMESPACE_END

