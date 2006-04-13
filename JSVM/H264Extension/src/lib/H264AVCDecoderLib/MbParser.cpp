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
#include "H264AVCCommonLib/Transform.h"


H264AVC_NAMESPACE_BEGIN


MbParser::MbParser()
: m_pcTransform       ( NULL )
, m_pcMbSymbolReadIf  ( NULL )
, m_bInitDone         ( false )
, m_bPrevIsSkipped    ( false )
{
}

MbParser::~MbParser()
{
}


ErrVal MbParser::create( MbParser*& rpcMbParser )
{
  rpcMbParser = new MbParser;

  ROT( NULL == rpcMbParser );

  return Err::m_nOK;
}



ErrVal MbParser::init( Transform* pcTransform )
{
  ROF( pcTransform );
  m_pcTransform = pcTransform;
  
  return Err::m_nOK;
}


ErrVal MbParser::destroy()
{
  delete this;
  return Err::m_nOK;
}


ErrVal MbParser::initSlice( MbSymbolReadIf* pcMbSymbolReadIf )
{
  ROT( NULL == pcMbSymbolReadIf );

  m_pcMbSymbolReadIf = pcMbSymbolReadIf;
  m_bPrevIsSkipped   = false;
  m_bInitDone = true;
  return Err::m_nOK;
}

ErrVal MbParser::uninit()
{
  m_pcTransform       = NULL;
  m_pcMbSymbolReadIf  = NULL;

  m_bInitDone = false;
  return Err::m_nOK;
}




ErrVal MbParser::process( MbDataAccess& rcMbDataAccess, Bool& rbEndOfSlice)
{
  ROF( m_bInitDone );

  Bool bIsCoded       = true;


  //===== parse SKIP flag =====
  if( m_pcMbSymbolReadIf->isMbSkipped( rcMbDataAccess ) )
  {
    bIsCoded = false;
    rcMbDataAccess.getMbTCoeffs().clear();
    rcMbDataAccess.getMbData().clearIntraPredictionModes( true );
    DECRNOK( xSkipMb( rcMbDataAccess ) );
  }

  if( bIsCoded )
  {
    DECRNOK( xReadMbType( rcMbDataAccess, false ) );

    if( ! rcMbDataAccess.getSH().isIntra() )
    {
      MbMode eMbMode = rcMbDataAccess.getMbData().getMbMode();
      if( rcMbDataAccess.getSH().isInterB() )
      {
        DECRNOK( xReadReferenceFrames( rcMbDataAccess, eMbMode, LIST_0 ) );
        DECRNOK( xReadReferenceFrames( rcMbDataAccess, eMbMode, LIST_1 ) );
        DECRNOK( xReadMotionVectors( rcMbDataAccess,   eMbMode, LIST_0 ) );
        DECRNOK( xReadMotionVectors( rcMbDataAccess,   eMbMode, LIST_1 ) );
      }
      else
      {
        DECRNOK( xReadReferenceFrames( rcMbDataAccess, eMbMode, LIST_0 ) );
        DECRNOK( xReadMotionVectors(   rcMbDataAccess, eMbMode, LIST_0 ) );
      }
    }

    if( rcMbDataAccess.getMbData().isPCM() )
    {
      DECRNOK( m_pcMbSymbolReadIf->samplesPCM( rcMbDataAccess  ) );
    }
    else
    {
      DECRNOK( xReadIntraPredModes( rcMbDataAccess ) );

      Bool bTrafo8x8Flag = ( rcMbDataAccess.getSH().getPPS().getTransform8x8ModeFlag() &&
                             rcMbDataAccess.getMbData().is8x8TrafoFlagPresent()        &&
                            !rcMbDataAccess.getMbData().isIntra4x4() );
      //-- JVT-R091
			DECRNOK( xReadTextureInfo( rcMbDataAccess, NULL, bTrafo8x8Flag ) );
			//--
    }
  }
  m_bPrevIsSkipped = ! bIsCoded;

  rcMbDataAccess.getMbData().updateResidualAvailFlags();

  rbEndOfSlice = m_pcMbSymbolReadIf->isEndOfSlice();

  return Err::m_nOK;
}




ErrVal MbParser::read( MbDataAccess&  rcMbDataAccess,
                       MbDataAccess*  pcMbDataAccessBase,
                       Int            iSpatialScalabilityType,
                       Bool&          rbEndOfSlice)
{
  ROF( m_bInitDone );

  //===== base layer mode flag and base layer refinement flag =====
  if( rcMbDataAccess.getSH().getBaseLayerId() != MSYS_UINT_MAX )
  {
    if ( pcMbDataAccessBase->getMbData().getInCropWindowFlag() == true )// TMM_ESS
    {
			if( rcMbDataAccess.getSH().getAdaptivePredictionFlag() )
			{
	            // TMM_ESS 
				if( ! m_pcMbSymbolReadIf->isBLSkipped( rcMbDataAccess ) && (iSpatialScalabilityType != SST_RATIO_1) && ! pcMbDataAccessBase->getMbData().isIntra() )
	
				{
					m_pcMbSymbolReadIf->isBLQRef( rcMbDataAccess );
				}
				else
				{
					rcMbDataAccess.getMbData().setBLQRefFlag( false );
				}
			}
			else
			{
				rcMbDataAccess.getMbData().setBLSkipFlag( true  );
				rcMbDataAccess.getMbData().setBLQRefFlag( false );
			}
// TMM_ESS {
    }
    else  // ( rcMbDataAccess.getMbData().getInCropWindowFlag() == false )
    {
        rcMbDataAccess.getMbData().setBLSkipFlag( false );
        rcMbDataAccess.getMbData().setBLQRefFlag( false );
    }
// TMM_ESS }
  }
  else
  {
      rcMbDataAccess.getMbData().setBLSkipFlag( false );
      rcMbDataAccess.getMbData().setBLQRefFlag( false );
  }


  //===== macroblock mode =====
  if( ! rcMbDataAccess.getMbData().getBLSkipFlag() &&
      ! rcMbDataAccess.getMbData().getBLQRefFlag()   )
  {
    DECRNOK( m_pcMbSymbolReadIf->mbMode( rcMbDataAccess ) );

    if( rcMbDataAccess.getSH().getBaseLayerId() != MSYS_UINT_MAX && rcMbDataAccess.getMbData().getMbMode() == INTRA_4X4 )
    {
	  if( ( pcMbDataAccessBase->getMbData().isIntra() || !rcMbDataAccess.isConstrainedInterLayerPred( pcMbDataAccessBase ) ) &&
	      pcMbDataAccessBase->getMbData().getInCropWindowFlag() )
      DECRNOK( m_pcMbSymbolReadIf->blFlag( rcMbDataAccess ) );
    }
  }


  if( rcMbDataAccess.getMbData().getBLSkipFlag() )
  {
    //===== copy motion data from base layer ======
    ROF( pcMbDataAccessBase );
    rcMbDataAccess.getMbData().copyMotion( pcMbDataAccessBase->getMbData() );
    rcMbDataAccess.getMbMvdData( LIST_0 ).clear();
    rcMbDataAccess.getMbMvdData( LIST_1 ).clear();
    rcMbDataAccess.getMbData().setBLSkipFlag( true  );
    rcMbDataAccess.getMbData().setBLQRefFlag( false );
    if( rcMbDataAccess.getMbData().isIntra() )
    {
      rcMbDataAccess.getMbData().setMbMode( INTRA_BL );
    }
  }
  else if( rcMbDataAccess.getMbData().getBLQRefFlag() )
  {
    //===== copy base layer motion =====
    rcMbDataAccess.getMbData().copyMotion( pcMbDataAccessBase->getMbData() );
    rcMbDataAccess.getMbMvdData( LIST_0 ).clear();
    rcMbDataAccess.getMbMvdData( LIST_1 ).clear();
    rcMbDataAccess.getMbData().setBLSkipFlag( false );
    rcMbDataAccess.getMbData().setBLQRefFlag( true  );

    //===== QPEL MOTION VECTOR REFINEMENTS =====
    if( rcMbDataAccess.getSH().isInterB() )
    {
      DECRNOK( xReadMotionVectorsQPel( rcMbDataAccess, LIST_0 ) );
      DECRNOK( xReadMotionVectorsQPel( rcMbDataAccess, LIST_1 ) );
    }
    else
    {
      DECRNOK( xReadMotionVectorsQPel( rcMbDataAccess, LIST_0 ) );
    }
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
      if( rcMbDataAccess.getSH().isInterB() )
      {
        rcMbDataAccess.getMbMotionData( LIST_1 ).clear( BLOCK_NOT_PREDICTED );
        rcMbDataAccess.getMbMvdData   ( LIST_1 ).clear();
      }
    }
    else if( eMbMode == MODE_SKIP )
    {
      if( rcMbDataAccess.getSH().isInterB() )
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
      if( rcMbDataAccess.getSH().isInterB() )
      {
        DECRNOK( xReadMotionPredFlags         ( rcMbDataAccess, pcMbDataAccessBase, eMbMode, LIST_0 ) );
        DECRNOK( xReadMotionPredFlags         ( rcMbDataAccess, pcMbDataAccessBase, eMbMode, LIST_1 ) );
        DECRNOK( xReadReferenceFramesNoRefPic ( rcMbDataAccess,                     eMbMode, LIST_0 ) );
        DECRNOK( xReadReferenceFramesNoRefPic ( rcMbDataAccess,                     eMbMode, LIST_1 ) );
        DECRNOK( xReadMotionVectors           ( rcMbDataAccess,                     eMbMode, LIST_0 ) );
        DECRNOK( xReadMotionVectors           ( rcMbDataAccess,                     eMbMode, LIST_1 ) );
      }
      else
      {
        DECRNOK( xReadMotionPredFlags         ( rcMbDataAccess, pcMbDataAccessBase, eMbMode, LIST_0 ) );
        DECRNOK( xReadReferenceFramesNoRefPic ( rcMbDataAccess,                     eMbMode, LIST_0 ) );
        DECRNOK( xReadMotionVectors           ( rcMbDataAccess,                     eMbMode, LIST_0 ) );
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
    DECRNOK( xReadIntraPredModes( rcMbDataAccess ) );


    Bool bTrafo8x8Flag = ( rcMbDataAccess.getSH().getPPS().getTransform8x8ModeFlag() &&
                           rcMbDataAccess.getMbData().is8x8TrafoFlagPresent()        &&
                          !rcMbDataAccess.getMbData().isIntra4x4() );
    //-- JVT-R091
		DECRNOK( xReadTextureInfo( rcMbDataAccess, pcMbDataAccessBase, bTrafo8x8Flag ) );
		//--
  }

  rcMbDataAccess.getMbData().updateResidualAvailFlags();

  //===== terminating bits =====
  rbEndOfSlice = m_pcMbSymbolReadIf->isEndOfSlice();

  return Err::m_nOK;
}

//	TMM_EC {{
ErrVal MbParser::readVirtual( MbDataAccess&  rcMbDataAccess,
                       MbDataAccess*  pcMbDataAccessBase,
                       Int            iSpatialScalabilityType,
                       Bool&          rbEndOfSlice,
											 ERROR_CONCEAL      eErrorConceal)
{
  ROF( m_bInitDone );
	switch ( eErrorConceal)
	{
	case	EC_TEMPORAL_DIRECT:
		{
			rcMbDataAccess.getMbData().setMbMode(MODE_SKIP);
		}
		break;
	case	EC_FRAME_COPY:
		rcMbDataAccess.getMbData().setMbMode(MODE_16x16);
		rcMbDataAccess.getMbData().getMbMotionData(LIST_0).setAllMv( Mv::ZeroMv(), PART_16x16 );
		rcMbDataAccess.getMbData().getMbMotionData(LIST_0).setRefIdx(1);
		rcMbDataAccess.getMbData().getMbMvdData   (LIST_0).clear();
		if(rcMbDataAccess.getSH().getSliceType()==B_SLICE)
		{	
		rcMbDataAccess.getMbData().getMbMotionData(LIST_1).setAllMv( Mv::ZeroMv(), PART_16x16 );
		rcMbDataAccess.getMbData().getMbMotionData(LIST_1).setRefIdx(1);
		rcMbDataAccess.getMbData().getMbMvdData   (LIST_1).clear();
		}

		break;
	case	EC_BLSKIP:
			//===== copy motion data from base layer ======
		ROF( pcMbDataAccessBase );
		rcMbDataAccess.getMbData().copyMotion( pcMbDataAccessBase->getMbData() );
		//      rcMbDataAccess.getMbMvdData( LIST_0 ).clear();
		//      rcMbDataAccess.getMbMvdData( LIST_1 ).clear();
		rcMbDataAccess.getMbData().setBLSkipFlag( true  );
		rcMbDataAccess.getMbData().setBLQRefFlag( false );

		if( rcMbDataAccess.getMbData().isIntra() )
		{
			rcMbDataAccess.getMbData().setMbMode( INTRA_BL );
		}
		else
		{
		rcMbDataAccess.getMbData().setResidualPredFlag( true, PART_16x16);
		}
		break;
	}

  return Err::m_nOK;
}
//	TMM_EC }}


ErrVal MbParser::readMotion(MbDataAccess&  rcMbDataAccess,
                            MbDataAccess*  pcMbDataAccessBase )
{
  ROT( rcMbDataAccess.getMbData().isIntra() );

  UInt uiFwdBwdBase        = pcMbDataAccessBase->getMbData().getFwdBwd();

  try {
    //===== base mode flag =====
    ROTRS( m_pcMbSymbolReadIf->isBLSkipped( rcMbDataAccess ), Err::m_nOK );

    rcMbDataAccess.getMbMotionData( LIST_0 ).reset();
    rcMbDataAccess.getMbMotionData( LIST_0 ).clear( BLOCK_NOT_PREDICTED );
    rcMbDataAccess.getMbMvdData   ( LIST_0 ).clear();
    rcMbDataAccess.getMbMotionData( LIST_1 ).reset();
    rcMbDataAccess.getMbMotionData( LIST_1 ).clear( BLOCK_NOT_PREDICTED );
    rcMbDataAccess.getMbMvdData   ( LIST_1 ).clear();
    rcMbDataAccess.getMbData().setBLSkipFlag( false );
    rcMbDataAccess.getMbData().setBLQRefFlag( false );

    // <<<<<
    // really nasty code: set forward/backward indication of BASE LAYER (sic!)
    // this probably should/could have been done somewhere else...
    if( rcMbDataAccess.getSH().isH264AVCCompatible() )
    {
      if( pcMbDataAccessBase->getMbData().getMbMode() == MODE_SKIP )
      {
        UInt uiFwdBwd = 0;
        if( pcMbDataAccessBase->getSH().isInterB() )
        {
          for( Int n = 3; n >= 0; n--)
          {
            uiFwdBwd <<= 4;
            uiFwdBwd += (0 < pcMbDataAccessBase->getMbMotionData( LIST_0 ).getRefIdx( Par8x8(n) )) ? 1:0;
            uiFwdBwd += (0 < pcMbDataAccessBase->getMbMotionData( LIST_1 ).getRefIdx( Par8x8(n) )) ? 2:0;
          }
        }
        if( pcMbDataAccessBase->getSH().isInterP() )
        {
          for( Int n = 3; n >= 0; n--)
          {
            uiFwdBwd <<= 4;
            uiFwdBwd += (0 < pcMbDataAccessBase->getMbMotionData( LIST_0 ).getRefIdx( Par8x8(n) )) ? 1:0;
          }
        }
        pcMbDataAccessBase->getMbData().setFwdBwd( uiFwdBwd );
      }
      else if( pcMbDataAccessBase->getMbData().getMbMode() == MODE_8x8 )
      {
        UInt uiFwdBwd = 0;
        for( Int n = 3; n >= 0; n-- )
        {
          uiFwdBwd <<= 4;
          if( pcMbDataAccessBase->getMbData().getBlkMode( Par8x8(n) ) == BLK_SKIP )
          {
            uiFwdBwd += (0 < pcMbDataAccessBase->getMbMotionData( LIST_0 ).getRefIdx( Par8x8(n) )) ? 1:0;
            if( pcMbDataAccessBase->getSH().isInterB() ) 
              uiFwdBwd += (0 < pcMbDataAccessBase->getMbMotionData( LIST_1 ).getRefIdx( Par8x8(n) )) ? 2:0;
          }
        }
        pcMbDataAccessBase->getMbData().setFwdBwd( pcMbDataAccessBase->getMbData().getFwdBwd() | uiFwdBwd );
      }
    }
    // >>>>>

    //===== macroblock mode =====
    DECRNOK( m_pcMbSymbolReadIf->mbMode( rcMbDataAccess ) );
    if( rcMbDataAccess.getMbData().isIntra() )
    {
      // NOTE: this may happen only at a truncation point
      // ----- restore base layer forward/backward indication -----
      pcMbDataAccessBase->getMbData().setFwdBwd( uiFwdBwdBase );

      // ----- mark as skipped macroblock -----
      RNOK( rcMbDataAccess.getMbData().copyMotion( pcMbDataAccessBase->getMbData() ) );
      rcMbDataAccess.getMbData().copyFrom  ( pcMbDataAccessBase->getMbData() );
      rcMbDataAccess.getMbData().setBLSkipFlag( true );
      return Err::m_nOK;
    }

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

    //===== MOTION DATA =====
    MbMode eMbMode = rcMbDataAccess.getMbData().getMbMode();

    if( eMbMode == MODE_SKIP )
    {
      if( rcMbDataAccess.getSH().isInterB() )
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
      if( rcMbDataAccess.getSH().isInterB() )
      {
        DECRNOK( xReadMotionPredFlags         ( rcMbDataAccess, pcMbDataAccessBase, eMbMode, LIST_0 ) );
        DECRNOK( xReadMotionPredFlags         ( rcMbDataAccess, pcMbDataAccessBase, eMbMode, LIST_1 ) );
        DECRNOK( xReadReferenceFramesNoRefPic ( rcMbDataAccess,                     eMbMode, LIST_0 ) );
        DECRNOK( xReadReferenceFramesNoRefPic ( rcMbDataAccess,                     eMbMode, LIST_1 ) );
        DECRNOK( xReadMotionVectors           ( rcMbDataAccess,                     eMbMode, LIST_0 ) );
        DECRNOK( xReadMotionVectors           ( rcMbDataAccess,                     eMbMode, LIST_1 ) );
      }
      else
      {
        DECRNOK( xReadMotionPredFlags         ( rcMbDataAccess, pcMbDataAccessBase, eMbMode, LIST_0 ) );
        DECRNOK( xReadReferenceFramesNoRefPic ( rcMbDataAccess,                     eMbMode, LIST_0 ) );
        DECRNOK( xReadMotionVectors           ( rcMbDataAccess,                     eMbMode, LIST_0 ) );
      }
    }
    //===== residual prediction flag =====
    DECRNOK( m_pcMbSymbolReadIf->resPredFlag( rcMbDataAccess ) );

    // nasty again, set reference pics
    // <<<<<
    if( rcMbDataAccess.getSH().isH264AVCCompatible() )
    {
      SliceHeader& rcSH = rcMbDataAccess.getSH();
      for( UInt uiListIdx = 0; uiListIdx < UInt( rcSH.isInterB() ? 2 : 1 ); uiListIdx++ )
      {
        ListIdx eListIdx = ListIdx( uiListIdx );
        MbMotionData& rcMbMotionData = rcMbDataAccess.getMbMotionData( eListIdx );
        for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
        {
          if( rcMbDataAccess.getMbData().isBlockFwdBwd( c8x8Idx.b8x8Index() , eListIdx ) )
          {
            const Frame* pcRefFrame = rcSH.getRefPic( rcMbMotionData.getRefIdx( c8x8Idx.b8x8Index() ), eListIdx ).getFrame();
            rcMbMotionData.setRefPic( pcRefFrame, c8x8Idx.b8x8() );
          }
        }
      }
    }
    // >>>>>
  }
  catch(...)
  {
    // ===== ERROR HANDLING (typically truncated FGS slice) =====
    // ----- restore base layer forward/backward indication -----
    pcMbDataAccessBase->getMbData().setFwdBwd( uiFwdBwdBase );

    // ----- mark as skipped macroblock -----
    RNOK( rcMbDataAccess.getMbData().copyMotion( pcMbDataAccessBase->getMbData() ) );
          rcMbDataAccess.getMbData().copyFrom  ( pcMbDataAccessBase->getMbData() );
          rcMbDataAccess.getMbData().setBLSkipFlag( true );
    throw;
  }

  pcMbDataAccessBase->getMbData().setFwdBwd( uiFwdBwdBase );

  return Err::m_nOK;
}



ErrVal MbParser::xReadMbType( MbDataAccess& rcMbDataAccess, Bool bBaseLayer )
{
  DECRNOK( m_pcMbSymbolReadIf->mbMode( rcMbDataAccess ) );

  if( bBaseLayer && rcMbDataAccess.getMbData().getMbMode() == INTRA_4X4 )
  {
    DECRNOK( m_pcMbSymbolReadIf->blFlag( rcMbDataAccess ) );
  }

  ROFRS( rcMbDataAccess.getMbData().isInter8x8(), Err::m_nOK );

  DECRNOK( m_pcMbSymbolReadIf->blockModes( rcMbDataAccess ) );

  return Err::m_nOK;
}




ErrVal MbParser::xReadIntraPredModes( MbDataAccess& rcMbDataAccess )
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


ErrVal MbParser::xGet8x8BlockMv( MbDataAccess& rcMbDataAccess, B8x8Idx c8x8Idx, ListIdx eLstIdx )
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



ErrVal MbParser::xReadReferenceFrames( MbDataAccess& rcMbDataAccess, MbMode eMbMode, ListIdx eLstIdx )
{
  MbMotionData&       rcMbMotionData = rcMbDataAccess.getMbMotionData( eLstIdx );
  const SliceHeader&  rcSliceHeader  = rcMbDataAccess.getSH();
  const Frame*        pcRefPic;

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
          if( 1 == rcMbDataAccess.getNumActiveRef( eLstIdx ) )
          {
            rcMbMotionData.setRefIdx( 1 );
          }
          else
          {
            DECRNOK( m_pcMbSymbolReadIf->refFrame( rcMbDataAccess, eLstIdx ) );
          }
          pcRefPic = rcSliceHeader.getRefPic( rcMbMotionData.getRefIdx(), eLstIdx ).getFrame();
          rcMbMotionData.setRefPic( pcRefPic );
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
          if( 1 == rcMbDataAccess.getNumActiveRef( eLstIdx ) )
          {
            rcMbMotionData.setRefIdx( 1, PART_16x8_0 );
          }
          else
          {
            DECRNOK( m_pcMbSymbolReadIf->refFrame( rcMbDataAccess, eLstIdx, PART_16x8_0 ) );
          }
          pcRefPic = rcSliceHeader.getRefPic( rcMbMotionData.getRefIdx( PART_16x8_0 ), eLstIdx ).getFrame();
          rcMbMotionData.setRefPic( pcRefPic, PART_16x8_0 );
        }
        else
        {
          rcMbMotionData.setRefIdx( BLOCK_NOT_PREDICTED, PART_16x8_0 );
        }

        if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_2, eLstIdx ) )
        {
          if( 1 == rcMbDataAccess.getNumActiveRef( eLstIdx ) )
          {
            rcMbMotionData.setRefIdx( 1, PART_16x8_1 );
          }
          else
          {
            DECRNOK( m_pcMbSymbolReadIf->refFrame( rcMbDataAccess, eLstIdx, PART_16x8_1 ) );
          }
          pcRefPic = rcSliceHeader.getRefPic( rcMbMotionData.getRefIdx( PART_16x8_1 ), eLstIdx ).getFrame();
          rcMbMotionData.setRefPic( pcRefPic, PART_16x8_1 );
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
          if( 1 == rcMbDataAccess.getNumActiveRef( eLstIdx ) )
          {
            rcMbMotionData.setRefIdx( 1, PART_8x16_0 );
          }
          else
          {
            DECRNOK( m_pcMbSymbolReadIf->refFrame( rcMbDataAccess, eLstIdx, PART_8x16_0 ) );
          }
          pcRefPic = rcSliceHeader.getRefPic( rcMbMotionData.getRefIdx( PART_8x16_0 ), eLstIdx ).getFrame();
          rcMbMotionData.setRefPic( pcRefPic, PART_8x16_0 );
        }
        else
        {
          rcMbMotionData.setRefIdx( BLOCK_NOT_PREDICTED, PART_8x16_0 );
        }

        if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_1, eLstIdx ) )
        {
          if( 1 == rcMbDataAccess.getNumActiveRef( eLstIdx ) )
          {
            rcMbMotionData.setRefIdx( 1, PART_8x16_1 );
          }
          else
          {
            DECRNOK( m_pcMbSymbolReadIf->refFrame( rcMbDataAccess, eLstIdx, PART_8x16_1 ) );
          }
          pcRefPic = rcSliceHeader.getRefPic( rcMbMotionData.getRefIdx( PART_8x16_1 ), eLstIdx ).getFrame();
          rcMbMotionData.setRefPic( pcRefPic, PART_8x16_1 );
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
          if( rcMbDataAccess.getMbData().isBlockFwdBwd( c8x8Idx.b8x8Index(), eLstIdx ) )
          {
            if( 1 == rcMbDataAccess.getNumActiveRef( eLstIdx ) )
            {
              rcMbMotionData.setRefIdx( 1, c8x8Idx.b8x8() );
            }
            else
            {
              DECRNOK( m_pcMbSymbolReadIf->refFrame( rcMbDataAccess, eLstIdx, c8x8Idx.b8x8() ) );
            }
            pcRefPic = rcSliceHeader.getRefPic( rcMbMotionData.getRefIdx( c8x8Idx.b8x8() ), eLstIdx ).getFrame();
            rcMbMotionData.setRefPic( pcRefPic, c8x8Idx.b8x8() );
          }
          else
          {
            rcMbMotionData.setRefIdx( BLOCK_NOT_PREDICTED, c8x8Idx.b8x8() );
          }
        }
        break;
      }
    case MODE_8x8ref0:
      {
        rcMbMotionData.setRefIdx( 1 );
        pcRefPic = rcSliceHeader.getRefPic( rcMbMotionData.getRefIdx(), eLstIdx ).getFrame();
        rcMbMotionData.setRefPic( pcRefPic );
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


ErrVal MbParser::xReadReferenceFramesNoRefPic( MbDataAccess& rcMbDataAccess, MbMode eMbMode, ListIdx eLstIdx )
{
  MbMotionData& rcMbMotionData  = rcMbDataAccess.getMbMotionData( eLstIdx );
  Bool          bPred           = rcMbDataAccess.getSH().getAdaptivePredictionFlag();

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
        AOT(1);
        return Err::m_nERR;
      }
  }

  return Err::m_nOK;
}





ErrVal MbParser::xReadMotionPredFlags( MbDataAccess&  rcMbDataAccess,
                                       MbDataAccess*  pcMbDataAccessBaseMotion,
                                       MbMode         eMbMode,
                                       ListIdx        eLstIdx )
{
  ROT   ( rcMbDataAccess.getSH().getAdaptivePredictionFlag() && pcMbDataAccessBaseMotion == NULL );
  ROFRS ( rcMbDataAccess.getSH().getAdaptivePredictionFlag(), Err::m_nOK );

  MbMotionData& rcMbMotionData      = rcMbDataAccess           .getMbMotionData( eLstIdx );
  MbMotionData& rcMbMotionDataBase  = pcMbDataAccessBaseMotion->getMbMotionData( eLstIdx );

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
        if( rcMbDataAccess           .getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx ) &&
            pcMbDataAccessBaseMotion->getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx )    )
        {
          DECRNOK( m_pcMbSymbolReadIf->motionPredFlag( rcMbDataAccess, eLstIdx ) );

          if( rcMbMotionData.getMotPredFlag() )
          {
            rcMbMotionData.setRefIdx( rcMbMotionDataBase.getRefIdx() );
          }
        }
        break;
      }
    case MODE_16x8:
      {
        if( rcMbDataAccess           .getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx ) &&
            pcMbDataAccessBaseMotion->getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx )    )
        {
          DECRNOK( m_pcMbSymbolReadIf->motionPredFlag( rcMbDataAccess, eLstIdx, PART_16x8_0 ) );

          if( rcMbMotionData.getMotPredFlag( PART_16x8_0 ) )
          {
            rcMbMotionData.setRefIdx( rcMbMotionDataBase.getRefIdx( PART_16x8_0 ), PART_16x8_0 );
          }
        }

        if( rcMbDataAccess           .getMbData().isBlockFwdBwd( B_8x8_2, eLstIdx ) &&
            pcMbDataAccessBaseMotion->getMbData().isBlockFwdBwd( B_8x8_2, eLstIdx )    )
        {
          DECRNOK( m_pcMbSymbolReadIf->motionPredFlag( rcMbDataAccess, eLstIdx, PART_16x8_1 ) );

          if( rcMbMotionData.getMotPredFlag( PART_16x8_1 ) )
          {
            rcMbMotionData.setRefIdx( rcMbMotionDataBase.getRefIdx( PART_16x8_1 ), PART_16x8_1 );
          }
        }
        break;
      }
    case MODE_8x16:
      {
        if( rcMbDataAccess           .getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx ) &&
            pcMbDataAccessBaseMotion->getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx )    )
        {
          DECRNOK( m_pcMbSymbolReadIf->motionPredFlag( rcMbDataAccess, eLstIdx, PART_8x16_0 ) );

          if( rcMbMotionData.getMotPredFlag( PART_8x16_0 ) )
          {
            rcMbMotionData.setRefIdx( rcMbMotionDataBase.getRefIdx( PART_8x16_0 ), PART_8x16_0 );
          }
        }

        if( rcMbDataAccess           .getMbData().isBlockFwdBwd( B_8x8_1, eLstIdx ) &&
            pcMbDataAccessBaseMotion->getMbData().isBlockFwdBwd( B_8x8_1, eLstIdx )    )
        {
          DECRNOK( m_pcMbSymbolReadIf->motionPredFlag( rcMbDataAccess, eLstIdx, PART_8x16_1 ) );

          if( rcMbMotionData.getMotPredFlag( PART_8x16_1 ) )
          {
            rcMbMotionData.setRefIdx( rcMbMotionDataBase.getRefIdx( PART_8x16_1 ), PART_8x16_1 );
          }
        }
        break;
      }
    case MODE_8x8:
    case MODE_8x8ref0:
      {
        for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
        {
          if( BLK_SKIP != rcMbDataAccess.getMbData().getBlkMode( c8x8Idx.b8x8Index() ) &&
              rcMbDataAccess           .getMbData().isBlockFwdBwd( c8x8Idx.b8x8Index(), eLstIdx ) &&
              pcMbDataAccessBaseMotion->getMbData().isBlockFwdBwd( c8x8Idx.b8x8Index(), eLstIdx )   )
          {
            DECRNOK( m_pcMbSymbolReadIf->motionPredFlag( rcMbDataAccess, eLstIdx, c8x8Idx.b8x8() ) );

            if( rcMbMotionData.getMotPredFlag( c8x8Idx.b8x8() ) )
            {
              rcMbMotionData.setRefIdx( rcMbMotionDataBase.getRefIdx( c8x8Idx.b8x8() ), c8x8Idx.b8x8() );
            }
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





ErrVal MbParser::xReadMotionVectors( MbDataAccess& rcMbDataAccess, MbMode eMbMode, ListIdx eLstIdx )
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
      AOT(1);
      return Err::m_nERR;
    }
  }
  return Err::m_nERR;
}




ErrVal MbParser::xReadMotionVectorsQPel( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx )
{
  ROTRS( rcMbDataAccess.getMbData().isIntra(), Err::m_nOK );

  switch( rcMbDataAccess.getMbData().getMbMode() )
  {
  case MODE_16x16:
    {
      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx ) )
      {
        DECRNOK( m_pcMbSymbolReadIf->mvdQPel( rcMbDataAccess, eLstIdx ) );
      }
      return Err::m_nOK;
    }

  case MODE_16x8:
    {
      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx ) )
      {
        DECRNOK( m_pcMbSymbolReadIf->mvdQPel( rcMbDataAccess, eLstIdx, PART_16x8_0 ) );
      }

      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_2, eLstIdx ) )
      {
        DECRNOK( m_pcMbSymbolReadIf->mvdQPel( rcMbDataAccess, eLstIdx, PART_16x8_1 ) );
      }
      return Err::m_nOK;
    }
  case MODE_8x16:
    {
      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_0, eLstIdx ) )
      {
        DECRNOK( m_pcMbSymbolReadIf->mvdQPel( rcMbDataAccess, eLstIdx, PART_8x16_0 ) );
      }

      if( rcMbDataAccess.getMbData().isBlockFwdBwd( B_8x8_1, eLstIdx ) )
      {
        DECRNOK( m_pcMbSymbolReadIf->mvdQPel( rcMbDataAccess, eLstIdx, PART_8x16_1 ) );
      }
      return Err::m_nOK;
    }
  case MODE_8x8:
  case MODE_8x8ref0:
    {
      for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
      {
        ROT( rcMbDataAccess.getMbData().getBlkMode( c8x8Idx.b8x8Index() ) != BLK_8x8 );
        if( rcMbDataAccess.getMbData().isBlockFwdBwd( c8x8Idx.b8x8Index(), eLstIdx ) )
        {
          DECRNOK( m_pcMbSymbolReadIf->mvdQPel( rcMbDataAccess, eLstIdx, c8x8Idx.b8x8() ) );
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



ErrVal
MbParser::xReadTextureInfo( MbDataAccess&   rcMbDataAccess,
														MbDataAccess*		pcMbDataAccessBase,	// JVT-R091
                            Bool						bTrafo8x8Flag
                           )
{
  Bool bReadDQp = true;

  if( ! rcMbDataAccess.getMbData().isIntra16x16() )
  {
    DECRNOK( m_pcMbSymbolReadIf->cbp( rcMbDataAccess ) );
    bReadDQp = rcMbDataAccess.getMbData().getMbCbp() != 0;
  }

  
  if( bTrafo8x8Flag && ( rcMbDataAccess.getMbData().getMbCbp() & 0x0F ) )
  {
    ROT( rcMbDataAccess.getMbData().isIntra16x16() );
    ROT( rcMbDataAccess.getMbData().isIntra4x4  () );
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


  if( !rcMbDataAccess.getMbData().isIntra() )
  {
    if( rcMbDataAccess.getSH().getAdaptivePredictionFlag() )
    {
      DECRNOK( m_pcMbSymbolReadIf->resPredFlag( rcMbDataAccess ) );

			//-- JVT-R091
			if ( rcMbDataAccess.getMbData().getResidualPredFlag( PART_16x16 ) && 
					 rcMbDataAccess.getMbData().getBLSkipFlag() &&
					 pcMbDataAccessBase &&
					 rcMbDataAccess.isConstrainedInterLayerPred( pcMbDataAccessBase ) )
			{
				DECRNOK( m_pcMbSymbolReadIf->smoothedRefFlag( rcMbDataAccess ) );
			}
			//--
    }
    else if( rcMbDataAccess.getSH().getBaseLayerId() != MSYS_UINT_MAX )
    {
      rcMbDataAccess.getMbData().setResidualPredFlag( true, PART_16x16 );
			rcMbDataAccess.getMbData().setSmoothedRefFlag( false );	// JVT-R091
    }
  }



  UInt uiDummy = 0;
  if( rcMbDataAccess.getMbData().isIntra16x16() )
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
        DECRNOK( m_pcMbSymbolReadIf->residualBlock8x8( rcMbDataAccess, c8x8Idx, LUMA_8X8, uiMbExtCbp ) );
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
          DECRNOK( m_pcMbSymbolReadIf->residualBlock( rcMbDataAccess, cIdx , LUMA_SCAN, uiMbExtCbp ) );
        }
      }
    }

  }
  rcMbDataAccess.getMbData().setMbExtCbp( uiMbExtCbp );

  DECRNOK( xScanChromaBlocks  ( rcMbDataAccess,  rcMbDataAccess.getMbData().getCbpChroma4x4() ) );

  return Err::m_nOK;
}






ErrVal MbParser::xScanChromaBlocks( MbDataAccess& rcMbDataAccess, UInt uiChromCbp )
{
  ROTRS( 1 > uiChromCbp, Err::m_nOK );

  DECRNOK( m_pcMbSymbolReadIf->residualBlock( rcMbDataAccess,  CIdx(0), CHROMA_DC ) );
  DECRNOK( m_pcMbSymbolReadIf->residualBlock( rcMbDataAccess,  CIdx(4), CHROMA_DC ) );

  ROTRS( 2 > uiChromCbp, Err::m_nOK );

  for( CIdx cCIdx; cCIdx.isLegal(); cCIdx++ )
  {
    DECRNOK( m_pcMbSymbolReadIf->residualBlock( rcMbDataAccess,  cCIdx, CHROMA_AC ) );
  }
  return Err::m_nOK;
}


ErrVal MbParser::xSkipMb( MbDataAccess& rcMbDataAccess )
{
  if( rcMbDataAccess.getSH().isInterB() )
  {
    RNOK( rcMbDataAccess.setConvertMbType( 0 ) );
  }
  else
  {
    const Frame* pcRefPic = rcMbDataAccess.getSH().getRefPic( 1, LIST_0 ).getFrame();
    rcMbDataAccess.getMbMotionData( LIST_0 ).setRefIdx( 1 );
    rcMbDataAccess.getMbMotionData( LIST_0 ).setRefPic( pcRefPic );
    RNOK( rcMbDataAccess.setConvertMbType( MSYS_UINT_MAX ) );
  }

  rcMbDataAccess.getMbData().setMbCbp( 0 );
  return Err::m_nOK;
}




H264AVC_NAMESPACE_END

