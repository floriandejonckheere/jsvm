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
#include "H264AVCCommonLib/MbDataCtrl.h"
#include "H264AVCCommonLib/FrameUnit.h"

#include "H264AVCCommonLib/CFMO.h"
#include <math.h>

H264AVC_NAMESPACE_BEGIN


#define MEDIAN(a,b,c)  ((a)>(b)?(a)>(c)?(b)>(c)?(b):(c):(a):(b)>(c)?(a)>(c)?(a):(c):(b))


MbDataCtrl::MbDataCtrl():
  m_pcMbTCoeffs     ( NULL ),
  m_pcMbData        ( NULL ),
  m_pcMbDataAccess  ( NULL ),
  m_pcSliceHeader   ( NULL ),
  m_ucLastMbQp      ( 0 ),
  m_uiMbStride      ( 0 ),
  m_uiMbOffset      ( 0 ),
  m_iMbPerLine      ( 0 ),
  m_iMbPerColumn    ( 0 ),
  m_uiSize          ( 0 ),
  m_uiMbProcessed   ( 0 ),
  m_uiSliceId       ( 0 ),
  m_eProcessingState( PRE_PROCESS),
  m_pcMbDataCtrl0L1 ( NULL ),
  m_bUseTopField    ( false ),
  m_bPicCodedField  ( false ),
  m_bInitDone       ( false )
, m_pacFgsBQMbQP    ( 0 )
, m_pauiFgsBQMbCbp  ( 0 )
, m_pauiFgsBQBCBP   ( 0 )
, m_pabFgsBQ8x8Trafo( 0 )
{
  m_apcMbMvdData    [LIST_0]  = NULL;
  m_apcMbMvdData    [LIST_1]  = NULL;
  m_apcMbMotionData [LIST_0]  = NULL;
  m_apcMbMotionData [LIST_1]  = NULL;
  m_apcMbMotionDataBase [LIST_0]  = NULL;
  m_apcMbMotionDataBase [LIST_1]  = NULL;
}

MbDataCtrl::~MbDataCtrl()
{
  AOT( xDeleteData() );
  AOT( m_bInitDone );
}

ErrVal MbDataCtrl::xCreateData( UInt uiSize )
{
  uiSize++;

  ROT( NULL == ( m_pcMbTCoeffs         = new MbTransformCoeffs [ uiSize ] ) );
  ROT( NULL == ( m_apcMbMotionData[0]  = new MbMotionData      [ uiSize ] ) );
  ROT( NULL == ( m_apcMbMotionData[1]  = new MbMotionData      [ uiSize ] ) );
  ROT( NULL == ( m_apcMbMotionDataBase[0]  = new MbMotionData      [ uiSize ] ) );
  ROT( NULL == ( m_apcMbMotionDataBase[1]  = new MbMotionData      [ uiSize ] ) );
  ROT( NULL == ( m_apcMbMvdData[0]     = new MbMvData          [ uiSize ] ) );
  ROT( NULL == ( m_apcMbMvdData[1]     = new MbMvData          [ uiSize ] ) );
  ROT( NULL == ( m_pcMbData            = new MbData            [ uiSize ] ) );

  for( UInt uiIdx = 0; uiIdx < uiSize; uiIdx++ )
  {
    m_pcMbData[ uiIdx ].init( m_pcMbTCoeffs        + uiIdx,
                              m_apcMbMvdData   [0] + uiIdx,
                              m_apcMbMvdData   [1] + uiIdx,
                              m_apcMbMotionData[0] + uiIdx,
                              m_apcMbMotionData[1] + uiIdx,
                              m_apcMbMotionDataBase[0] + uiIdx,
                              m_apcMbMotionDataBase[1] + uiIdx );
  }

  // clear outside mb data
  m_pcMbData[uiSize-1].getMbTCoeffs().setAllCoeffCount( 0 );
  m_pcMbData[uiSize-1].initMbData( 0, MSYS_UINT_MAX );

  return Err::m_nOK;
}

ErrVal MbDataCtrl::xDeleteData()
{
  H264AVC_DELETE_CLASS( m_pcMbDataAccess );

  H264AVC_DELETE( m_pcMbTCoeffs );
  H264AVC_DELETE( m_apcMbMvdData[1] );
  H264AVC_DELETE( m_apcMbMvdData[0] );
  H264AVC_DELETE( m_apcMbMotionData[1] );
  H264AVC_DELETE( m_apcMbMotionData[0] );
  H264AVC_DELETE( m_apcMbMotionDataBase[1] );
  H264AVC_DELETE( m_apcMbMotionDataBase[0] );
  H264AVC_DELETE( m_pcMbData );
  m_uiSize          = 0;
  return Err::m_nOK;
}

ErrVal MbDataCtrl::xResetData()
{
  UInt uiIdx;
  for( uiIdx = 0; uiIdx < m_uiSize; uiIdx++ )
  {
    m_pcMbData[ uiIdx ].reset();
  }
  for( uiIdx = 0; uiIdx < m_uiSize; uiIdx++ )
  {
    m_pcMbTCoeffs[ uiIdx ].clear();
  }
  for( uiIdx = 0; uiIdx < m_uiSize; uiIdx++ )
  {
    m_apcMbMvdData[0][ uiIdx ].clear();
  }
  for( uiIdx = 0; uiIdx < m_uiSize; uiIdx++ )
  {
    m_apcMbMvdData[1][ uiIdx ].clear();
  }
  for( uiIdx = 0; uiIdx < m_uiSize; uiIdx++ )
  {
    m_apcMbMotionData[0][ uiIdx ].reset();
  }
  for( uiIdx = 0; uiIdx < m_uiSize; uiIdx++ )
  {
    m_apcMbMotionData[1][ uiIdx ].reset();
  }
  for( uiIdx = 0; uiIdx < m_uiSize; uiIdx++ )
  {
    m_apcMbMotionDataBase[0][ uiIdx ].reset();
  }
  for( uiIdx = 0; uiIdx < m_uiSize; uiIdx++ )
  {
    m_apcMbMotionDataBase[1][ uiIdx ].reset();
  }
  return Err::m_nOK;
}


Bool MbDataCtrl::isPicDone( const SliceHeader& rcSH )
{
  return ( m_uiMbProcessed == rcSH.getSPS().getMbInFrame() || m_uiMbProcessed == rcSH.getMbInPic());
}

Bool MbDataCtrl::isFrameDone( const SliceHeader& rcSH )
{
  return ( m_uiMbProcessed == rcSH.getSPS().getMbInFrame());
}


ErrVal MbDataCtrl::init( const SequenceParameterSet& rcSPS )
{
  AOT_DBG( m_bInitDone );

  UInt uiSize = rcSPS.getMbInFrame();

  ROT( 0 == uiSize );
  if( m_uiSize == uiSize )
  {
    RNOK( xResetData() );
  }
  else
  {
    RNOK( xDeleteData() );
    RNOK( xCreateData( uiSize ) );
    m_uiSize = uiSize;
  }

  m_iMbPerLine = rcSPS.getFrameWidthInMbs();

  RNOK( m_cpDFPBuffer.init( uiSize+1 ) );
  m_cpDFPBuffer.clear();

  m_bInitDone     = true;

  return Err::m_nOK;
}


ErrVal
MbDataCtrl::switchMotionRefinement()
{
  for( UInt n = 0; n < m_uiSize; n++ )
    m_pcMbData[n].switchMotionRefinement();
  return Err::m_nOK;
}


ErrVal
MbDataCtrl::copyMotion( MbDataCtrl& rcMbDataCtrl )
{
  for( UInt n = 0; n < m_uiSize; n++ )
  {
    RNOK( m_pcMbData[n].copyMotion( rcMbDataCtrl.m_pcMbData[n], m_uiSliceId ) );
  }
  return Err::m_nOK;
}

// TMM_ESS_UNIFIED {
// motion copying (upsampling with factor=1) with/without cropping (MB aligned)
ErrVal
MbDataCtrl::copyMotionBL( MbDataCtrl& rcBaseMbDataCtrl, ResizeParameters* pcParameters )
{
  Bool bDirect8x8 = rcBaseMbDataCtrl.xGetDirect8x8InferenceFlag();

  Int  iMbOrigX = pcParameters->m_iPosX/16;
  Int  iMbOrigY = pcParameters->m_iPosY/16;
  Int  iMbEndX = iMbOrigX + pcParameters->m_iOutWidth/16;
  Int  iMbEndY = iMbOrigY + pcParameters->m_iOutHeight/16;

 for( Int iMbY = iMbOrigY ; iMbY < iMbEndY; iMbY++)
  for(Int iMbX = iMbOrigX ; iMbX < iMbEndX; iMbX++)
	 {
    MbData& rcMbDes = m_pcMbData[iMbY*rcBaseMbDataCtrl.m_uiMbStride + iMbX];
    RNOK( rcMbDes.copyMotionBL( rcBaseMbDataCtrl.m_pcMbData[(iMbY - iMbOrigY)*rcBaseMbDataCtrl.m_uiMbStride + (iMbX - iMbOrigX)], bDirect8x8, m_uiSliceId ) );
    rcMbDes.setInCropWindowFlag( true );
	 }
  return Err::m_nOK;
}


// motion upsampling (upsampling with factor=2) with/without cropping (MB aligned)
ErrVal
MbDataCtrl::xUpsampleMotionDyad( MbDataCtrl& rcBaseMbDataCtrl, ResizeParameters* pcParameters )
{
	Bool bDirect8x8 = rcBaseMbDataCtrl.xGetDirect8x8InferenceFlag();
	Int  iMbX,iMbY,iBaseMbY,iBaseMbX,iPar;
	UInt uiBaseMbStride=rcBaseMbDataCtrl.m_uiMbStride;

	Int  iMbOrigX = pcParameters->m_iPosX/16;
	Int  iMbOrigY = pcParameters->m_iPosY/16;
	Int  iMbEndX = iMbOrigX + pcParameters->m_iOutWidth/16;
	Int  iMbEndY = iMbOrigY + pcParameters->m_iOutHeight/16;

	//loop on scaled base window MBs
	for( iMbY = iMbOrigY,iBaseMbY = 0 ; iMbY < iMbEndY; iMbY+=2,iBaseMbY++)
		for( iMbX = iMbOrigX,iBaseMbX = 0 ; iMbX < iMbEndX; iMbX+=2,iBaseMbX++)
			for( iPar = 0; iPar < 4;              iPar++  )
			{
				MbData& rcMbDes = m_pcMbData[(iMbY+(iPar/2))*m_uiMbStride+(iMbX+(iPar%2))];
				MbData& rcMbSrc = rcBaseMbDataCtrl.m_pcMbData[iBaseMbY*uiBaseMbStride+iBaseMbX];

				Par8x8  ePar    = Par8x8( iPar );
				rcMbDes.setInCropWindowFlag( true );
				RNOK( rcMbDes.upsampleMotion( rcMbSrc, ePar, bDirect8x8 ) );
			}

			return Err::m_nOK;
}


// motion upsampling with any cropping and upsampling factor
ErrVal
MbDataCtrl::xUpsampleMotionESS( MbDataCtrl& rcBaseMbDataCtrl,ResizeParameters* pcParameters )
{
  Bool bDirect8x8 = rcBaseMbDataCtrl.xGetDirect8x8InferenceFlag();
  
  if( pcParameters->m_iExtendedSpatialScalability == ESS_PICT )
  {
  Int index = m_pcSliceHeader->getPoc();
  pcParameters->setPOC(index);
  pcParameters->setCurrentPictureParametersWith(index);
  }

  Int     iScaledBaseOrigX = pcParameters->m_iPosX;
  Int     iScaledBaseOrigY = pcParameters->m_iPosY; 
  Int     iScaledBaseWidth = pcParameters->m_iOutWidth;
  Int     iScaledBaseHeight = pcParameters->m_iOutHeight;

  Int  aiPelOrig[2];  

  // loop on MBs of high res picture
  //--------------------------------
  for( Int iMbY = 0; iMbY < m_iMbPerColumn; iMbY++ )
  {
    for( Int iMbX = 0; iMbX < m_iMbPerLine;   iMbX++ )
    {
      // get current high res MB and upsampling
      MbData& rcMbDes = m_pcMbData[iMbY*m_uiMbStride+iMbX];

      // check if MB is inside cropping window - if not, no upsampling is performed
      if ( (iMbX >= (iScaledBaseOrigX+15) / 16) && (iMbX < (iScaledBaseOrigX+iScaledBaseWidth) / 16) &&
           (iMbY >= (iScaledBaseOrigY+15) / 16) && (iMbY < (iScaledBaseOrigY+iScaledBaseHeight) / 16) )
      {
          aiPelOrig[0]=(Int)16*iMbX-iScaledBaseOrigX;
          aiPelOrig[1]=(Int)16*iMbY-iScaledBaseOrigY;
          RNOK(rcMbDes.upsampleMotionESS(
                                        rcBaseMbDataCtrl.m_pcMbData,
                                         
                                        rcBaseMbDataCtrl.m_uiMbStride,
                                         aiPelOrig,
										 bDirect8x8,
                                         pcParameters));
           rcMbDes.setInCropWindowFlag( true );
		  }
      else
      {
          rcMbDes.noUpsampleMotion();
      }
	 } // end of for( Int iMbX = 0; iMbX < m_iMbPerLine;   iMbX++ )
  } // end of for( Int iMbY = 0; iMbY < m_iMbPerColumn; iMbY++ )

  return Err::m_nOK;
}
ErrVal
MbDataCtrl::upsampleMotion( MbDataCtrl& rcBaseMbDataCtrl, ResizeParameters* pcParameters )
{
	switch (pcParameters->m_iSpatialScalabilityType)
	{
  case SST_RATIO_1:
		return copyMotionBL(rcBaseMbDataCtrl, pcParameters);
    break;
	case SST_RATIO_2:
    return xUpsampleMotionDyad(rcBaseMbDataCtrl, pcParameters);
		break;
	default:
		return xUpsampleMotionESS(rcBaseMbDataCtrl, pcParameters);
		break;
	}
	return Err::m_nOK;
}
// TMM_ESS_UNIFIED }

ErrVal
MbDataCtrl::copyBaseResidualAvailFlags( MbDataCtrl& rcSrcMbDataCtrl )
{
  for( UInt uiMbIdx = 0 ; uiMbIdx < m_uiSize; uiMbIdx ++ )
    m_pcMbData[uiMbIdx].setResidualAvailFlagsBase( rcSrcMbDataCtrl.m_pcMbData[uiMbIdx].getResidualAvailFlagsBase() );

  return Err::m_nOK;
}


ErrVal MbDataCtrl::uninit()
{
  m_ucLastMbQp      = 0;
  m_uiMbStride      = 0;
  m_uiMbOffset      = 0;
  m_iMbPerLine      = 0;
  m_iMbPerColumn    = 0;
  m_uiMbProcessed   = 0;
  m_uiSliceId       = 0;
  m_pcMbDataCtrl0L1 = 0;

  for( UInt n = 0; n < m_cpDFPBuffer.size(); n++ )
  {
    delete m_cpDFPBuffer.get( n );
    m_cpDFPBuffer.set( n, NULL );
  }
  RNOK( m_cpDFPBuffer.uninit() );

  m_bInitDone = false;
  return Err::m_nOK;
}


ErrVal MbDataCtrl::reset()
{
  m_ucLastMbQp      = 0;
  m_uiMbProcessed   = 0;
  m_uiSliceId       = 0;
  m_pcMbDataCtrl0L1 = 0;
  return Err::m_nOK;
}

ErrVal MbDataCtrl::initSlice( SliceHeader& rcSH, ProcessingState eProcessingState, Bool bDecoder, MbDataCtrl* pcMbDataCtrl )
{
  AOF_DBG( m_bInitDone );

  m_eProcessingState  = eProcessingState;
  m_pcMbDataCtrl0L1   = NULL;

  if( rcSH.isInterB() )
  {
    if( rcSH.getNalUnitType() != NAL_UNIT_CODED_SLICE_IDR_SCALABLE &&
        rcSH.getNalUnitType() != NAL_UNIT_CODED_SLICE_SCALABLE     && bDecoder )
    {
      const RefPic& rcRefPic0L1 = rcSH.getRefPic( 1, LIST_1 );
      AOF_DBG( rcRefPic0L1.isAvailable() );
      const FrameUnit* pcFU = rcRefPic0L1.getPic().getFrameUnit();

      Int iCurrPoc      = rcSH.getPoc();
      m_pcMbDataCtrl0L1 = pcFU->getMbDataCtrl();
    }

    if( pcMbDataCtrl )
    {
      m_pcMbDataCtrl0L1 = pcMbDataCtrl;
    }
  }

  if( PARSE_PROCESS == m_eProcessingState || ENCODE_PROCESS == m_eProcessingState )
  {
    m_uiSliceId++;

    m_cpDFPBuffer.set( m_uiSliceId, rcSH.getDeblockingFilterParameter().getCopy() );
    m_bDirect8x8InferenceFlag = rcSH.getSPS().getDirect8x8InferenceFlag();
  }
  m_pcSliceHeader = &rcSH;


  Int iMbPerColumn  = rcSH.getSPS().getFrameHeightInMbs ();
  m_iMbPerLine      = rcSH.getSPS().getFrameWidthInMbs  ();
  m_uiMbOffset      = 0;
  m_uiMbStride      = m_iMbPerLine;
  m_iMbPerColumn    = iMbPerColumn;
  m_ucLastMbQp      = rcSH.getPicQp();

  H264AVC_DELETE_CLASS( m_pcMbDataAccess );
  return Err::m_nOK;
}


const MbData& MbDataCtrl::xGetColMbData( UInt uiIndex )
{
  return (( m_pcMbDataCtrl0L1 == NULL ) ? xGetOutMbData() : m_pcMbDataCtrl0L1->getMbData( uiIndex ));
}

const MbData& MbDataCtrl::xGetRefMbData( UInt uiSliceId, Int uiCurrSliceID, Int iMbY, Int iMbX, Bool bLoopFilter )
{
  // check whether ref mb is inside
  ROTRS( iMbX < 0,               xGetOutMbData() );
  ROTRS( iMbY < 0,               xGetOutMbData() );
  ROTRS( iMbX >= m_iMbPerLine,   xGetOutMbData() );
  ROTRS( iMbY >= m_iMbPerColumn, xGetOutMbData() );

  //--ICU/ETRI FMO Implementation
  ROTRS( uiCurrSliceID != getSliceGroupIDofMb(iMbY * m_uiMbStride + iMbX + m_uiMbOffset ) , xGetOutMbData() );

  // get the ref mb data
  const MbData& rcMbData = getMbData( iMbY * m_uiMbStride + iMbX + m_uiMbOffset );
  // test slice id
  return (( rcMbData.getSliceId() == uiSliceId || bLoopFilter ) ? rcMbData : xGetOutMbData() );
}


ErrVal MbDataCtrl::initMb( MbDataAccess*& rpcMbDataAccess, UInt uiMbY, UInt uiMbX, const Int iForceQp )
{
  ROF( m_bInitDone );

  AOT_DBG( uiMbY * m_uiMbStride + uiMbX + m_uiMbOffset >= m_uiSize );

  Bool     bLf          = (m_eProcessingState == POST_PROCESS);
  UInt     uiCurrIdx    = uiMbY * m_uiMbStride + uiMbX + m_uiMbOffset;
  MbData&  rcMbDataCurr = m_pcMbData[ uiCurrIdx ];


  if( m_pcMbDataAccess )
  {
    m_ucLastMbQp = m_pcMbDataAccess->getMbData().getQp();
  }

  UInt uiSliceId = rcMbDataCurr.getSliceId();
  if( PARSE_PROCESS == m_eProcessingState || ENCODE_PROCESS == m_eProcessingState)
  {
    if( 0 == uiSliceId )
    {
      uiSliceId = m_uiSliceId;
      rcMbDataCurr.getMbTCoeffs().clear();
      rcMbDataCurr.initMbData( m_ucLastMbQp, uiSliceId );
      rcMbDataCurr.clear();
      m_uiMbProcessed++;
    }
    else
    {
      //allready assigned;
      if( ENCODE_PROCESS != m_eProcessingState )
      {
        AOT(1);
      }
      else
      {
        if( iForceQp != -1 )
        {
          m_ucLastMbQp = iForceQp;
        }
      }
    }
  }

  Bool bColocatedField = ( m_pcMbDataCtrl0L1 == NULL) ? true : m_pcMbDataCtrl0L1->isPicCodedField();

  Int icurrSliceGroupID = getSliceGroupIDofMb(uiMbY * m_uiMbStride + uiMbX + m_uiMbOffset);

  m_pcMbDataAccess = new (m_pcMbDataAccess) MbDataAccess(
                                       rcMbDataCurr,                                      // current
                                       xGetRefMbData( uiSliceId, icurrSliceGroupID, uiMbY,   uiMbX-1, bLf ), // left        //--ICU/ETRI FMO Implementation
                                       xGetRefMbData( uiSliceId, icurrSliceGroupID, uiMbY-1, uiMbX  , bLf ), // above       //--ICU/ETRI FMO Implementation
                                       xGetRefMbData( uiSliceId, icurrSliceGroupID, uiMbY-1, uiMbX-1, bLf ), // above left  //--ICU/ETRI FMO Implementation
                                       xGetRefMbData( uiSliceId, icurrSliceGroupID, uiMbY-1, uiMbX+1, bLf ), // above right //--ICU/ETRI FMO Implementation
                                       xGetOutMbData(),                                   // unvalid
                                       xGetColMbData( uiCurrIdx ),
                                       *m_pcSliceHeader,
                                       *m_cpDFPBuffer.get( uiSliceId ),
                                       uiMbX,
                                       uiMbY,
                                       m_ucLastMbQp );


  ROT( NULL == m_pcMbDataAccess );

  rpcMbDataAccess = m_pcMbDataAccess;

  return Err::m_nOK;
}

//TMM_EC {{
ErrVal MbDataCtrl::initMbTDEnhance( MbDataAccess*& rpcMbDataAccess, MbDataCtrl *pcMbDataCtrl, MbDataCtrl *pcMbDataCtrlRef, UInt uiMbY, UInt uiMbX, const Int iForceQp )
{
  ROF( m_bInitDone );

  AOT_DBG( uiMbY * m_uiMbStride + uiMbX + m_uiMbOffset >= m_uiSize );

  Bool     bLf          = (m_eProcessingState == POST_PROCESS);
  UInt     uiCurrIdx    = uiMbY * m_uiMbStride + uiMbX + m_uiMbOffset;
  MbData&  rcMbDataCurr = m_pcMbData[ uiCurrIdx ];


  if( m_pcMbDataAccess )
  {
    m_ucLastMbQp = m_pcMbDataAccess->getMbData().getQp();
  }

  UInt uiSliceId = rcMbDataCurr.getSliceId();
  if( PARSE_PROCESS == m_eProcessingState || ENCODE_PROCESS == m_eProcessingState)
  {
    if( 0 == uiSliceId )
    {
      uiSliceId = m_uiSliceId;
      rcMbDataCurr.getMbTCoeffs().clear();
      rcMbDataCurr.initMbData( m_ucLastMbQp, uiSliceId );
      rcMbDataCurr.clear();
      m_uiMbProcessed++;
    }
    else
    {
      //allready assigned;
      if( ENCODE_PROCESS != m_eProcessingState )
      {
        AOT(1);
      }
      else
      {
        if( iForceQp != -1 )
        {
          m_ucLastMbQp = iForceQp;
        }
      }
    }
  }

  Bool bColocatedField = ( m_pcMbDataCtrl0L1 == NULL) ? true : m_pcMbDataCtrl0L1->isPicCodedField();

  Int icurrSliceGroupID = getSliceGroupIDofMb(uiMbY * m_uiMbStride + uiMbX + m_uiMbOffset);

  m_pcMbDataAccess = new (m_pcMbDataAccess) MbDataAccess(
                                       rcMbDataCurr,                                      // current
                                       xGetRefMbData( uiSliceId, icurrSliceGroupID, uiMbY,   uiMbX-1, bLf ), // left
                                       xGetRefMbData( uiSliceId, icurrSliceGroupID, uiMbY-1, uiMbX  , bLf ), // above
                                       xGetRefMbData( uiSliceId, icurrSliceGroupID, uiMbY-1, uiMbX-1, bLf ), // above left
                                       xGetRefMbData( uiSliceId, icurrSliceGroupID, uiMbY-1, uiMbX+1, bLf ), // above right
                                       xGetOutMbData(),                                   // unvalid
																			 pcMbDataCtrlRef->getMbData( uiMbX, uiMbY),
                                       *m_pcSliceHeader,
                                       *m_cpDFPBuffer.get( uiSliceId ),
                                       uiMbX,
                                       uiMbY,
                                       m_ucLastMbQp );


  ROT( NULL == m_pcMbDataAccess );

  rpcMbDataAccess = m_pcMbDataAccess;

  return Err::m_nOK;
}
//TMM_EC }}

ErrVal
MbDataCtrl::storeFgsBQLayerQpAndCbp()
{
  ROF( m_pacFgsBQMbQP );
  ROF( m_pauiFgsBQMbCbp );
  ROF( m_pauiFgsBQBCBP );
  ROF( m_pabFgsBQ8x8Trafo );
  for( UInt uiMbIndex = 0; uiMbIndex < getSize(); uiMbIndex++ )
  {
    m_pacFgsBQMbQP     [uiMbIndex] = getMbData( uiMbIndex ).getQp();
    m_pauiFgsBQMbCbp   [uiMbIndex] = getMbData( uiMbIndex ).getMbExtCbp();
    m_pauiFgsBQBCBP    [uiMbIndex] = getMbData( uiMbIndex ).getBCBP();
    m_pabFgsBQ8x8Trafo [uiMbIndex] = getMbData( uiMbIndex ).isTransformSize8x8();
    
  }
  return Err::m_nOK;
}

ErrVal
MbDataCtrl::switchFgsBQLayerQpAndCbp()
{
  ROF( m_pacFgsBQMbQP );
  ROF( m_pauiFgsBQMbCbp );
  ROF( m_pauiFgsBQBCBP );
  ROF( m_pabFgsBQ8x8Trafo );
  for( UInt uiMbIndex = 0; uiMbIndex < getSize(); uiMbIndex++ )
  {
    UChar ucQP  = getMbData( uiMbIndex ).getQp();
    UInt  uiCbp = getMbData( uiMbIndex ).getMbExtCbp();
    UInt  uiBCBP= getMbData( uiMbIndex ).getBCBP();
    Bool  bT8x8 = getMbData( uiMbIndex ).isTransformSize8x8();

    getMbDataByIndex( uiMbIndex ).setQp               ( m_pacFgsBQMbQP     [uiMbIndex] );
    getMbDataByIndex( uiMbIndex ).setMbExtCbp         ( m_pauiFgsBQMbCbp   [uiMbIndex] );
    getMbDataByIndex( uiMbIndex ).setBCBP             ( m_pauiFgsBQBCBP    [uiMbIndex] );
    getMbDataByIndex( uiMbIndex ).setTransformSize8x8 ( m_pabFgsBQ8x8Trafo [uiMbIndex] );

    m_pacFgsBQMbQP     [uiMbIndex] = ucQP;
    m_pauiFgsBQMbCbp   [uiMbIndex] = uiCbp;
    m_pauiFgsBQBCBP    [uiMbIndex] = uiBCBP;
    m_pabFgsBQ8x8Trafo [uiMbIndex] = bT8x8;
  }
  return Err::m_nOK;
}

ErrVal
MbDataCtrl::initFgsBQData( UInt uiNumMb )
{
  ROT( m_pacFgsBQMbQP );
  ROT( m_pauiFgsBQMbCbp );
  ROT( m_pauiFgsBQBCBP );
  ROT( m_pabFgsBQ8x8Trafo );
  ROFRS( ( m_pacFgsBQMbQP      = new UChar [uiNumMb] ), Err::m_nERR );
  ROFRS( ( m_pauiFgsBQMbCbp    = new UInt  [uiNumMb] ), Err::m_nERR );
  ROFRS( ( m_pauiFgsBQBCBP     = new UInt  [uiNumMb] ), Err::m_nERR );
  ROFRS( ( m_pabFgsBQ8x8Trafo  = new Bool  [uiNumMb] ), Err::m_nERR );
  return Err::m_nOK;
}

ErrVal
MbDataCtrl::uninitFgsBQData()
{
  delete [] m_pacFgsBQMbQP;
  delete [] m_pauiFgsBQMbCbp;
  delete [] m_pauiFgsBQBCBP;
  delete [] m_pabFgsBQ8x8Trafo;
  m_pacFgsBQMbQP      = 0;
  m_pauiFgsBQMbCbp    = 0;
  m_pauiFgsBQBCBP    = 0;
  m_pabFgsBQ8x8Trafo  = 0;
  return Err::m_nOK;
}









ControlData::ControlData()
: m_pcMbDataCtrl        ( 0   )
, m_pcSliceHeader       ( 0   )
, m_dLambda             ( 0   )
, m_pcBaseLayerRec      ( 0   )
, m_pcBaseLayerSbb      ( 0   )
, m_pcBaseLayerCtrl     ( 0   )
, m_pcBaseCtrlData      ( 0   )
, m_uiUseBLMotion       ( 0   )
, m_dScalingFactor      ( 1.0 )
, m_pacFGSMbQP          ( 0 )
, m_pauiFGSMbCbp        ( 0 )
, m_pabFGS8x8Trafo      ( 0 )
, m_bIsNormalMbDataCtrl ( true )
, m_pacBQMbQP           ( 0 )
, m_pauiBQMbCbp         ( 0 )
, m_pabBQ8x8Trafo       ( 0 )
, m_paeBQMbMode         ( 0 )
, m_pusBQFwdBwd         ( 0 )
, m_bSpatialScalability ( false)//SSUN@SHARP
{
  m_paacBQMotionData[0] = m_paacBQMotionData[1] = 0;
}

ControlData::~ControlData()
{
  AOT( m_pacBQMbQP );
  AOT( m_pauiBQMbCbp );
  AOT( m_pabBQ8x8Trafo );
  AOT( m_pacFGSMbQP );
  AOT( m_pauiFGSMbCbp );
  AOT( m_pabFGS8x8Trafo );
}

Void
ControlData::clear()
{
  m_pcBaseLayerRec      = 0;
  m_pcBaseLayerSbb      = 0;
  m_pcBaseLayerCtrl     = 0;
  m_uiUseBLMotion       = 0;
  m_dScalingFactor      = 1.0;

  m_bIsNormalMbDataCtrl = true;
}

ErrVal
ControlData::init( SliceHeader*  pcSliceHeader,
                   MbDataCtrl*   pcMbDataCtrl,
                   Double        dLambda )
{
  ROF( pcSliceHeader );
  ROF( pcMbDataCtrl  );

  m_pcSliceHeader = pcSliceHeader;
  m_pcMbDataCtrl  = pcMbDataCtrl;
  m_dLambda       = dLambda;
  
  m_pcBaseLayerRec      = 0;
  m_pcBaseLayerSbb      = 0;
  m_pcBaseLayerCtrl     = 0;
  m_uiUseBLMotion       = 0;

  return Err::m_nOK;
}

ErrVal
ControlData::init( SliceHeader*  pcSliceHeader )
{
  ROF( pcSliceHeader );
  ROF( m_pcMbDataCtrl  );

  m_pcSliceHeader = pcSliceHeader;
  
  m_pcBaseLayerRec      = 0;
  m_pcBaseLayerSbb      = 0;
  m_pcBaseLayerCtrl     = 0;
  m_uiUseBLMotion       = 0;

  return Err::m_nOK;
}

ErrVal
ControlData::initBQData( UInt uiNumMb )
{
  ROT( m_pacBQMbQP );
  ROT( m_pauiBQMbCbp );
  ROT( m_pabBQ8x8Trafo );
  ROFRS( ( m_pacBQMbQP      = new UChar [uiNumMb] ), Err::m_nERR );
  ROFRS( ( m_pauiBQMbCbp    = new UInt  [uiNumMb] ), Err::m_nERR );
  ROFRS( ( m_pabBQ8x8Trafo  = new Bool  [uiNumMb] ), Err::m_nERR );
  ROFRS( ( m_paeBQMbMode    = new MbMode[uiNumMb] ), Err::m_nERR );
  ROFRS( ( m_pusBQFwdBwd    = new UShort[uiNumMb] ), Err::m_nERR );
  ROFRS( ( m_paacBQMotionData[0] = new MbMotionData[uiNumMb] ), Err::m_nERR );
  ROFRS( ( m_paacBQMotionData[1] = new MbMotionData[uiNumMb] ), Err::m_nERR );
  return Err::m_nOK;
}

ErrVal
ControlData::uninitBQData()
{
  delete [] m_pacBQMbQP;
  delete [] m_pauiBQMbCbp;
  delete [] m_pabBQ8x8Trafo;
  delete [] m_paeBQMbMode;
  delete [] m_pusBQFwdBwd;
  delete [] m_paacBQMotionData[0];
  delete [] m_paacBQMotionData[1];
  m_pacBQMbQP     = 0;
  m_pauiBQMbCbp   = 0;
  m_pabBQ8x8Trafo = 0;
  m_paeBQMbMode   = 0;
  m_pusBQFwdBwd   = 0;
  m_paacBQMotionData[0] = 0;
  m_paacBQMotionData[1] = 0;
  return Err::m_nOK;
}


ErrVal
ControlData::storeBQLayerQpAndCbp()
{
  ROF( m_pacBQMbQP );
  ROF( m_pauiBQMbCbp );
  ROF( m_pabBQ8x8Trafo );
  for( UInt uiMbIndex = 0; uiMbIndex < m_pcMbDataCtrl->getSize(); uiMbIndex++ )
  {
    m_pacBQMbQP     [uiMbIndex] = m_pcMbDataCtrl->getMbData( uiMbIndex ).getQp();
    m_pauiBQMbCbp   [uiMbIndex] = m_pcMbDataCtrl->getMbData( uiMbIndex ).getMbExtCbp();
    m_pabBQ8x8Trafo [uiMbIndex] = m_pcMbDataCtrl->getMbData( uiMbIndex ).isTransformSize8x8();
    m_paeBQMbMode   [uiMbIndex] = m_pcMbDataCtrl->getMbData( uiMbIndex ).getMbMode();
    m_pusBQFwdBwd   [uiMbIndex] = m_pcMbDataCtrl->getMbData( uiMbIndex ).getFwdBwd();
    m_paacBQMotionData[0][uiMbIndex].copyFrom( m_pcMbDataCtrl->getMbData( uiMbIndex ).getMbMotionData( ListIdx( 0 ) ) );
    m_paacBQMotionData[1][uiMbIndex].copyFrom( m_pcMbDataCtrl->getMbData( uiMbIndex ).getMbMotionData( ListIdx( 1 ) ) );
  }
  return Err::m_nOK;
}

ErrVal
ControlData::switchBQLayerQpAndCbp()
{
  ROF( m_pacBQMbQP );
  ROF( m_pauiBQMbCbp );
  ROF( m_pabBQ8x8Trafo );
  for( UInt uiMbIndex = 0; uiMbIndex < m_pcMbDataCtrl->getSize(); uiMbIndex++ )
  {
    UChar ucQP  = m_pcMbDataCtrl->getMbData( uiMbIndex ).getQp();
    UInt  uiCbp = m_pcMbDataCtrl->getMbData( uiMbIndex ).getMbExtCbp();
    Bool  bT8x8 = m_pcMbDataCtrl->getMbData( uiMbIndex ).isTransformSize8x8();

    m_pcMbDataCtrl->getMbDataByIndex( uiMbIndex ).setQp               ( m_pacBQMbQP     [uiMbIndex] );
    m_pcMbDataCtrl->getMbDataByIndex( uiMbIndex ).setMbExtCbp         ( m_pauiBQMbCbp   [uiMbIndex] );
    m_pcMbDataCtrl->getMbDataByIndex( uiMbIndex ).setTransformSize8x8 ( m_pabBQ8x8Trafo [uiMbIndex] );

    m_pacBQMbQP     [uiMbIndex] = ucQP;
    m_pauiBQMbCbp   [uiMbIndex] = uiCbp;
    m_pabBQ8x8Trafo [uiMbIndex] = bT8x8;

    MbMode       eMbMode  = m_pcMbDataCtrl->getMbData( uiMbIndex ).getMbMode();
    UShort       usFwdBwd = m_pcMbDataCtrl->getMbData( uiMbIndex ).getFwdBwd();

    m_pcMbDataCtrl->getMbDataByIndex( uiMbIndex ).setMbMode           ( m_paeBQMbMode [uiMbIndex] );
    m_pcMbDataCtrl->getMbDataByIndex( uiMbIndex ).setFwdBwd           ( m_pusBQFwdBwd [uiMbIndex] );

    m_paeBQMbMode   [uiMbIndex] = eMbMode;
    m_pusBQFwdBwd   [uiMbIndex] = usFwdBwd;

    for( UInt ui = 0; ui < 2; ui++ )
    {
      MbMotionData cMbMotionData;
      cMbMotionData.copyFrom( m_pcMbDataCtrl->getMbData( uiMbIndex ).getMbMotionData( ListIdx( ui ) ) );
      m_pcMbDataCtrl->getMbDataByIndex( uiMbIndex ).getMbMotionData( ListIdx( ui ) ).copyFrom( m_paacBQMotionData[ui][uiMbIndex] );
      m_paacBQMotionData[ui][uiMbIndex].copyFrom( cMbMotionData );
    }
  }
  return Err::m_nOK;
}




ErrVal
ControlData::initFGSData( UInt uiNumMb )
{
  ROT( m_pacFGSMbQP );
  ROT( m_pauiFGSMbCbp );
  ROT( m_pabFGS8x8Trafo );
  ROFRS( ( m_pacFGSMbQP      = new UChar [uiNumMb] ), Err::m_nERR );
  ROFRS( ( m_pauiFGSMbCbp    = new UInt  [uiNumMb] ), Err::m_nERR );
  ROFRS( ( m_pabFGS8x8Trafo  = new Bool  [uiNumMb] ), Err::m_nERR );
  return Err::m_nOK;
}

ErrVal
ControlData::uninitFGSData()
{
  delete [] m_pacFGSMbQP;
  delete [] m_pauiFGSMbCbp;
  delete [] m_pabFGS8x8Trafo;
  m_pacFGSMbQP      = 0;
  m_pauiFGSMbCbp    = 0;
  m_pabFGS8x8Trafo  = 0;
  return Err::m_nOK;
}

ErrVal
ControlData::storeFGSLayerQpAndCbp()
{
  ROF( m_pacFGSMbQP );
  ROF( m_pauiFGSMbCbp );
  ROF( m_pabFGS8x8Trafo );
  for( UInt uiMbIndex = 0; uiMbIndex < m_pcMbDataCtrl->getSize(); uiMbIndex++ )
  {
    m_pacFGSMbQP     [uiMbIndex] = m_pcMbDataCtrl->getMbData( uiMbIndex ).getQp();
    m_pauiFGSMbCbp   [uiMbIndex] = m_pcMbDataCtrl->getMbData( uiMbIndex ).getMbExtCbp();
    m_pabFGS8x8Trafo [uiMbIndex] = m_pcMbDataCtrl->getMbData( uiMbIndex ).isTransformSize8x8();
  }
  return Err::m_nOK;
}

ErrVal
ControlData::switchFGSLayerQpAndCbp()
{
  ROF( m_pacFGSMbQP );
  ROF( m_pauiFGSMbCbp );
  ROF( m_pabFGS8x8Trafo );
  for( UInt uiMbIndex = 0; uiMbIndex < m_pcMbDataCtrl->getSize(); uiMbIndex++ )
  {
    UChar ucQP  = m_pcMbDataCtrl->getMbData( uiMbIndex ).getQp();
    UInt  uiCbp = m_pcMbDataCtrl->getMbData( uiMbIndex ).getMbExtCbp();
    Bool  bT8x8 = m_pcMbDataCtrl->getMbData( uiMbIndex ).isTransformSize8x8();

    m_pcMbDataCtrl->getMbDataByIndex( uiMbIndex ).setQp               ( m_pacFGSMbQP     [uiMbIndex] );
    m_pcMbDataCtrl->getMbDataByIndex( uiMbIndex ).setMbExtCbp         ( m_pauiFGSMbCbp   [uiMbIndex] );
    m_pcMbDataCtrl->getMbDataByIndex( uiMbIndex ).setTransformSize8x8 ( m_pabFGS8x8Trafo [uiMbIndex] );

    m_pacFGSMbQP     [uiMbIndex] = ucQP;
    m_pauiFGSMbCbp   [uiMbIndex] = uiCbp;
    m_pabFGS8x8Trafo [uiMbIndex] = bT8x8;
  }
  return Err::m_nOK;
}











ErrVal MbDataCtrl::getBoundaryMask( Int iMbY, Int iMbX, UInt& ruiMask ) const 
{
  UInt     uiCurrIdx    = iMbY * m_uiMbStride + iMbX + m_uiMbOffset;
  AOT( uiCurrIdx >= m_uiSize );

  ruiMask               = 0;

  ROTRS( m_pcMbData[uiCurrIdx].isIntra(), Err::m_nOK );

  Bool bLeftAvailable   = ( iMbX > 0 );
  Bool bTopAvailable    = ( iMbY > 0 );
  Bool bRightAvailable  = ( iMbX < m_iMbPerLine-1 );
  Bool bBottomAvailable = ( iMbY < m_iMbPerColumn-1 );

  if( bTopAvailable )
  {
    Int iIndex = uiCurrIdx - m_uiMbStride;
    ruiMask |= m_pcMbData[iIndex].isIntra() ? 0x01 :0;

    if( bLeftAvailable )
    {
      Int iIndex = uiCurrIdx - m_uiMbStride - 1;
      ruiMask |= m_pcMbData[iIndex].isIntra() ? 0x80 :0;
    }

    if( bRightAvailable )
    {
      Int iIndex = uiCurrIdx - m_uiMbStride + 1;
      ruiMask |= m_pcMbData[iIndex].isIntra() ? 0x02 :0;
    }
  }

  if( bBottomAvailable )
  {
    Int iIndex = uiCurrIdx + m_uiMbStride;
    ruiMask |= m_pcMbData[iIndex].isIntra() ? 0x10 :0;

    if( bLeftAvailable )
    {
      Int iIndex = uiCurrIdx  + m_uiMbStride - 1;
      ruiMask |= m_pcMbData[iIndex].isIntra() ? 0x20 :0;
    }

    if( bRightAvailable )
    {
      Int iIndex = uiCurrIdx + m_uiMbStride + 1;
      ruiMask |= m_pcMbData[iIndex].isIntra() ? 0x08 :0;
    }
  }

  if( bLeftAvailable )
  {
    Int iIndex = uiCurrIdx-1;
    ruiMask |= m_pcMbData[iIndex].isIntra() ? 0x40 :0;
  }

  if( bRightAvailable )
  {
    Int iIndex = uiCurrIdx + 1;
    ruiMask |= m_pcMbData[iIndex].isIntra() ? 0x04 :0;
  }
  return Err::m_nOK;
}

const Int MbDataCtrl::getSliceGroupIDofMb(Int mb)
{
  Int iRefSliceID ;
  if(m_pcSliceHeader->getFMO() != NULL)
	iRefSliceID =m_pcSliceHeader->getFMO()->getSliceGroupId(mb );
  else
	iRefSliceID =-1;

  return iRefSliceID ;
}

H264AVC_NAMESPACE_END

