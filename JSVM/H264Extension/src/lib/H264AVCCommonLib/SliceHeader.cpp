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

#include "H264AVCCommonLib/SliceHeader.h"
#include "H264AVCCommonLib/PictureParameterSet.h"
#include "H264AVCCommonLib/Frame.h"



H264AVC_NAMESPACE_BEGIN


SliceHeader::SliceHeader()
: m_eErrorConcealMode   ( EC_NONE )
, m_bTrueSlice          ( true )
, m_bInIDRAccess        ( false )
, m_uiNumMbsInSlice     ( 0 )
, m_uiLastMbInSlice     ( 0 )
, m_iTopFieldPoc        ( 0 )
, m_iBotFieldPoc        ( 0 )
, m_bCoeffResidualPred  ( false )
//>>> remove
, m_uiLayerCGSSNR                     ( 0 )
, m_uiQualityLevelCGSSNR              ( 0 )
, m_uiBaseLayerCGSSNR                 ( MSYS_UINT_MAX )
, m_uiBaseQualityLevelCGSSNR          ( 0 )
, m_uiQLDiscardable                   ( MAX_QUALITY_LEVELS )
, m_uiBaseLayerId                     ( MSYS_UINT_MAX )
//<<< remove
, m_bReconstructionLayer( false )
{
  ::memset( m_aapcRefFrameList, 0x00, sizeof( m_aapcRefFrameList ) );
  for(UInt ui=0;ui<MAX_TEMP_LEVELS;ui++)
  {
    ::memset( m_aauiNumRefIdxActiveUpdate[ui] , 0x00, 2*sizeof(UInt));
  }
}

SliceHeader::SliceHeader( const NalUnitHeader& rcNalUnitHeader )
: SliceHeaderSyntax     ( rcNalUnitHeader )
, m_eErrorConcealMode   ( EC_NONE )
, m_bTrueSlice          ( true )
, m_bInIDRAccess        ( false )
, m_uiNumMbsInSlice     ( 0 )
, m_uiLastMbInSlice     ( 0 )
, m_iTopFieldPoc        ( 0 )
, m_iBotFieldPoc        ( 0 )
, m_bCoeffResidualPred  ( false )
//>>> remove
, m_uiLayerCGSSNR                     ( 0 )
, m_uiQualityLevelCGSSNR              ( 0 )
, m_uiBaseLayerCGSSNR                 ( MSYS_UINT_MAX )
, m_uiBaseQualityLevelCGSSNR          ( 0 )
, m_uiQLDiscardable                   ( MAX_QUALITY_LEVELS )
, m_uiBaseLayerId                     ( MSYS_UINT_MAX )
//<<< remove
, m_bReconstructionLayer( false )
{
  ::memset( m_aapcRefFrameList, 0x00, sizeof( m_aapcRefFrameList ) );
  for(UInt ui=0;ui<MAX_TEMP_LEVELS;ui++)
  {
    ::memset( m_aauiNumRefIdxActiveUpdate[ui] , 0x00, 2*sizeof(UInt));
  }
}

SliceHeader::SliceHeader( const PrefixHeader& rcPrefixHeader )
: SliceHeaderSyntax     ( rcPrefixHeader )
, m_eErrorConcealMode   ( EC_NONE )
, m_bTrueSlice          ( true )
, m_bInIDRAccess        ( false )
, m_uiNumMbsInSlice     ( 0 )
, m_uiLastMbInSlice     ( 0 )
, m_iTopFieldPoc        ( 0 )
, m_iBotFieldPoc        ( 0 )
, m_bCoeffResidualPred  ( false )
//>>> remove
, m_uiLayerCGSSNR                     ( 0 )
, m_uiQualityLevelCGSSNR              ( 0 )
, m_uiBaseLayerCGSSNR                 ( MSYS_UINT_MAX )
, m_uiBaseQualityLevelCGSSNR          ( 0 )
, m_uiQLDiscardable                   ( MAX_QUALITY_LEVELS )
, m_uiBaseLayerId                     ( MSYS_UINT_MAX )
//<<< remove
, m_bReconstructionLayer( false )
{
  ::memset( m_aapcRefFrameList, 0x00, sizeof( m_aapcRefFrameList ) );
  for(UInt ui=0;ui<MAX_TEMP_LEVELS;ui++)
  {
    ::memset( m_aauiNumRefIdxActiveUpdate[ui] , 0x00, 2*sizeof(UInt));
  }
}

SliceHeader::SliceHeader( const SequenceParameterSet& rcSPS, const PictureParameterSet& rcPPS )
: SliceHeaderSyntax     ( rcSPS, rcPPS )
, m_eErrorConcealMode   ( EC_NONE )
, m_bTrueSlice          ( true )
, m_bInIDRAccess        ( false )
, m_uiNumMbsInSlice     ( 0 )
, m_uiLastMbInSlice     ( 0 )
, m_iTopFieldPoc        ( 0 )
, m_iBotFieldPoc        ( 0 )
, m_bCoeffResidualPred  ( false )
//>>> remove
, m_uiLayerCGSSNR                     ( 0 )
, m_uiQualityLevelCGSSNR              ( 0 )
, m_uiBaseLayerCGSSNR                 ( MSYS_UINT_MAX )
, m_uiBaseQualityLevelCGSSNR          ( 0 )
, m_uiQLDiscardable                   ( MAX_QUALITY_LEVELS )
, m_uiBaseLayerId                     ( MSYS_UINT_MAX )
//<<< remove
, m_bReconstructionLayer( false )
{
  ::memset( m_aapcRefFrameList, 0x00, sizeof( m_aapcRefFrameList ) );
  for(UInt ui=0;ui<MAX_TEMP_LEVELS;ui++)
  {
    ::memset( m_aauiNumRefIdxActiveUpdate[ui] , 0x00, 2*sizeof(UInt));
  }
}
 
SliceHeader::SliceHeader( const SliceHeader& rcSliceHeader )
: SliceHeaderSyntax         ( rcSliceHeader )
, m_eErrorConcealMode       ( EC_NONE )
, m_bTrueSlice              ( true )
, m_bInIDRAccess            ( false )
, m_uiNumMbsInSlice         ( 0 )
, m_uiLastMbInSlice         ( rcSliceHeader.m_uiLastMbInSlice )
, m_iTopFieldPoc            ( rcSliceHeader.m_iTopFieldPoc )
, m_iBotFieldPoc            ( rcSliceHeader.m_iBotFieldPoc )
, m_iSpatialScalabilityType ( rcSliceHeader.m_iSpatialScalabilityType )
, m_bCoeffResidualPred      ( rcSliceHeader.m_bCoeffResidualPred )
//>>> remove
, m_uiLayerCGSSNR                     ( 0 )
, m_uiQualityLevelCGSSNR              ( 0 )
, m_uiBaseLayerCGSSNR                 ( MSYS_UINT_MAX )
, m_uiBaseQualityLevelCGSSNR          ( 0 )
, m_uiQLDiscardable                   ( MAX_QUALITY_LEVELS )
, m_uiBaseLayerId                     ( MSYS_UINT_MAX )
//<<< remove
, m_bReconstructionLayer( false )
{
  ::memcpy( m_aapcRefFrameList, rcSliceHeader.m_aapcRefFrameList, sizeof( m_aapcRefFrameList ) );
  ::memcpy( m_aauiNumRefIdxActiveUpdate, rcSliceHeader.m_aauiNumRefIdxActiveUpdate, sizeof( m_aauiNumRefIdxActiveUpdate ) );
  m_cFMO = rcSliceHeader.m_cFMO;
}

SliceHeader::~SliceHeader()
{
}

ErrVal
SliceHeader::init( const SequenceParameterSet& rcSPS, const PictureParameterSet& rcPPS )
{
  RNOK( SliceHeaderSyntax::init( rcSPS, rcPPS ) );
  m_eErrorConcealMode   = EC_NONE;
  m_bTrueSlice          = true;
  m_bInIDRAccess        = false;
  m_uiNumMbsInSlice     = 0;
  m_uiLastMbInSlice     = 0;
  m_iTopFieldPoc        = 0;
  m_iBotFieldPoc        = 0;
  m_bCoeffResidualPred  = false;
  m_bReconstructionLayer= false;
  ::memset( m_aapcRefFrameList, 0x00, sizeof( m_aapcRefFrameList ) );
  for(UInt ui=0;ui<MAX_TEMP_LEVELS;ui++)
  {
    ::memset( m_aauiNumRefIdxActiveUpdate[ui] , 0x00, 2*sizeof(UInt));
  }
  return Err::m_nOK;
}

Void
SliceHeader::copy( const SliceHeader& rcSliceHeader )
{
  SliceHeaderSyntax::copy ( rcSliceHeader );
  m_eErrorConcealMode     = rcSliceHeader.m_eErrorConcealMode;
  m_bTrueSlice            = rcSliceHeader.m_bTrueSlice;
  m_bInIDRAccess          = rcSliceHeader.m_bInIDRAccess;
  m_uiNumMbsInSlice       = rcSliceHeader.m_uiNumMbsInSlice;
  m_uiLastMbInSlice       = rcSliceHeader.m_uiLastMbInSlice;
  m_iTopFieldPoc          = rcSliceHeader.m_iTopFieldPoc;
  m_iBotFieldPoc          = rcSliceHeader.m_iBotFieldPoc;
  m_bCoeffResidualPred    = rcSliceHeader.m_bCoeffResidualPred;
  m_bReconstructionLayer  = rcSliceHeader.m_bReconstructionLayer;
  m_cFMO                  = rcSliceHeader.m_cFMO;
  ::memcpy( m_aapcRefFrameList, rcSliceHeader.m_aapcRefFrameList, sizeof( m_aapcRefFrameList ) );
  for(UInt ui=0;ui<MAX_TEMP_LEVELS;ui++)
  {
    ::memcpy( m_aauiNumRefIdxActiveUpdate[ui] , rcSliceHeader.m_aauiNumRefIdxActiveUpdate, 2*sizeof(UInt));
  }
}

ErrVal
SliceHeader::copyPrefix( const PrefixHeader& rcPrefixHeader )
{
  ROF( rcPrefixHeader.getNoInterLayerPredFlag() );
  ROT( rcPrefixHeader.getDependencyId() );
  ROT( rcPrefixHeader.getQualityId() );
  ROF( rcPrefixHeader.getOutputFlag() );
  ROF( rcPrefixHeader.getIdrFlag() == ( getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR ) );

  setIdrFlag                ( rcPrefixHeader.getIdrFlag             () );
  setPriorityId             ( rcPrefixHeader.getPriorityId          () );
  setNoInterLayerPredFlag   ( rcPrefixHeader.getNoInterLayerPredFlag() );
  setDependencyId           ( rcPrefixHeader.getDependencyId        () );
  setQualityId              ( rcPrefixHeader.getQualityId           () );
  setTemporalId             ( rcPrefixHeader.getTemporalId          () );
  setUseRefBasePicFlag      ( rcPrefixHeader.getUseRefBasePicFlag   () );
  setDiscardableFlag        ( rcPrefixHeader.getDiscardableFlag     () );
  setOutputFlag             ( rcPrefixHeader.getOutputFlag          () );
  setStoreRefBasePicFlag    ( rcPrefixHeader.getStoreRefBasePicFlag () );
  getDecRefBasePicMarking() = rcPrefixHeader.getDecRefBasePicMarking();
  return Err::m_nOK;
}

UInt
SliceHeader::getMbAddressFromPosition( UInt uiMbY, UInt uiMbX ) const
{
  AOF( parameterSetsInitialized() );

  UInt uiMbAddress  = 0;
  UInt uiMbsInRow   = getSPS().getFrameWidthInMbs();
  if( isMbaffFrame() )
  {
    uiMbAddress = ( ( ( uiMbY >> 1 ) * uiMbsInRow + uiMbX ) << 1 ) + ( uiMbY & 2 );
  }
  else
  {
    uiMbAddress = uiMbY * uiMbsInRow + uiMbX;
  }
  return uiMbAddress;
}

Void
SliceHeader::getMbPositionFromAddress( UInt& ruiMbY, UInt& ruiMbX, UInt uiMbAddress ) const
{
  AOF( parameterSetsInitialized() );
  
  UInt uiMbsInRow = getSPS().getFrameWidthInMbs();
  if( isMbaffFrame() )
  {
    ruiMbY = ( 2 * ( uiMbAddress / 2 ) / uiMbsInRow ) + ( uiMbAddress % 2 );
    ruiMbX = (     ( uiMbAddress / 2 ) % uiMbsInRow );
  }
  else
  {
    ruiMbY = ( uiMbAddress / uiMbsInRow );
    ruiMbX = ( uiMbAddress % uiMbsInRow );
  }
}

Void
SliceHeader::getMbPosAndIndexFromAddress( UInt& ruiMbY, UInt& ruiMbX, UInt& ruiMbIndex, UInt uiMbAddress ) const
{
  AOF( parameterSetsInitialized() );

  UInt uiMbsInRow = getSPS().getFrameWidthInMbs();
  if( isMbaffFrame() )
  {
    ruiMbY      = ( 2 * ( uiMbAddress / 2 ) / uiMbsInRow ) + ( uiMbAddress % 2 );
    ruiMbX      = (     ( uiMbAddress / 2 ) % uiMbsInRow );
    ruiMbIndex  = uiMbsInRow * ruiMbY + ruiMbX;
  }
  else
  {
    ruiMbY      = ( uiMbAddress / uiMbsInRow );
    ruiMbX      = ( uiMbAddress % uiMbsInRow );
    ruiMbIndex  =   uiMbAddress;
  }
}

UInt
SliceHeader::getMbIndexFromAddress( UInt uiMbAddress ) const
{
  if( isMbaffFrame() )
  {
    UInt    uiMbsInRow  = getSPS().getFrameWidthInMbs();
    UInt    uiMbY       = ( 2 * ( uiMbAddress / 2 ) / uiMbsInRow ) + ( uiMbAddress % 2 );
    UInt    uiMbX       = (     ( uiMbAddress / 2 ) % uiMbsInRow );
    UInt    uiMbIndex   = uiMbsInRow * uiMbY + uiMbX;
    return  uiMbIndex;
  }
  return    uiMbAddress;
}

Bool
SliceHeader::isFieldPair( UInt uiFrameNum, PicType ePicType, Bool bIsRefPic ) const
{
  ROTRS(  getFrameNum ()  !=  uiFrameNum, false );
  ROTRS(  isRefPic    ()  !=  bIsRefPic,  false );
  ROTRS(  getPicType  ()  &   ePicType,   false );
  return true;
}

Int
SliceHeader::getDistScaleFactorWP( const Frame* pcFrameL0, const Frame* pcFrameL1 ) const
{
  Int iDiffPocD = pcFrameL1->getPoc() - pcFrameL0->getPoc();
  if( iDiffPocD == 0 )
  {
    return 1024;
  }
  Int iDiffPocB = getPoc() - pcFrameL0->getPoc();
  Int iTDB      = gClipMinMax( iDiffPocB, -128, 127 );
  Int iTDD      = gClipMinMax( iDiffPocD, -128, 127 );
  Int iX        = ( 0x4000 + abs( iTDD / 2 ) ) / iTDD;
  Int iScale    = gClipMinMax( ( iTDB * iX + 32 ) >> 6, -1024, 1023 );
  return iScale;
}

const PredWeight&
SliceHeader::getPredWeight( ListIdx eListIdx, UInt uiRefIdx, Bool bFieldFlag ) const
{
  if( bFieldFlag )  return getPredWeightTable( eListIdx ).get( ( uiRefIdx - 1 ) / 2 );
  else              return getPredWeightTable( eListIdx ).get(   uiRefIdx - 1 );
};

PredWeight&
SliceHeader::getPredWeight( ListIdx eListIdx, UInt uiRefIdx, Bool bFieldFlag )
{
  if( bFieldFlag )  return getPredWeightTable( eListIdx ).get( ( uiRefIdx - 1 ) / 2 );
  else              return getPredWeightTable( eListIdx ).get(   uiRefIdx - 1 );
};

PicType
SliceHeader::getPicType() const
{
  ROFRS(  getFieldPicFlag (), FRAME );
  ROFRS(  getBotFieldPoc  (), TOP_FIELD );
  return                      BOT_FIELD;
}

Int
SliceHeader::getPoc() const
{
  return ( getFieldPicFlag() ? ( getBottomFieldFlag() ? m_iBotFieldPoc : m_iTopFieldPoc ) : min( m_iTopFieldPoc, m_iBotFieldPoc ) ); 
}

Int
SliceHeader::getPoc( PicType ePicType ) const
{
  return ( ePicType == FRAME ? min( m_iTopFieldPoc, m_iBotFieldPoc ) : ePicType == BOT_FIELD ? m_iBotFieldPoc : m_iTopFieldPoc ); 
}

Void
SliceHeader::setCoeffResidualPredFlag( const SliceHeader* pcRefSliceHeader )
{
  m_bCoeffResidualPred = false;
  if( pcRefSliceHeader )
  {
    ROTVS( getTCoeffLevelPredictionFlag () );
    ROFVS( m_iSpatialScalabilityType == SST_RATIO_1 );
    ROFVS( getSPS().getFrameMbsOnlyFlag() );
    ROFVS( pcRefSliceHeader->getSPS().getFrameMbsOnlyFlag() );
    m_bCoeffResidualPred = true;
  }
}

Void
SliceHeader::setPicType( PicType ePicType )
{
  setFieldPicFlag   ( ePicType != FRAME );
  setBottomFieldFlag( ePicType == BOT_FIELD );
}

Bool
SliceHeader::isFirstSliceOfNextAccessUnit( const SliceHeader* pcLastSliceHeader ) const
{
  ROFRS   ( pcLastSliceHeader,                                                              false );
#if 1 // redundant pictures in not in correct order currently (should be modified in later versions)
#else
  ROTRS   ( pcLastSliceHeader->getRedundantPicCnt       ()  < getRedundantPicCnt        (), false );
  ROTRS   ( pcLastSliceHeader->getRedundantPicCnt       ()  > getRedundantPicCnt        (), true  );
#endif
  ROTRS   ( pcLastSliceHeader->getDependencyId          ()  < getDependencyId           (), false );
  ROTRS   ( pcLastSliceHeader->getDependencyId          ()  > getDependencyId           (), true  );
  ROTRS   ( pcLastSliceHeader->getQualityId             ()  < getQualityId              (), false );
  ROTRS   ( pcLastSliceHeader->getQualityId             ()  > getQualityId              (), true  );
  ROTRS   ( pcLastSliceHeader->getFrameNum              () != getFrameNum               (), true  );
  ROTRS   ( pcLastSliceHeader->getPicParameterSetId     () != getPicParameterSetId      (), true  );
  ROTRS   ( pcLastSliceHeader->getFieldPicFlag          () != getFieldPicFlag           (), true  );
  ROTRS   ( pcLastSliceHeader->getBottomFieldFlag       () != getBottomFieldFlag        (), true  );
  ROTRS   ( pcLastSliceHeader->isRefPic                 () != isRefPic                  (), true  );
  ROTRS   ( pcLastSliceHeader->getIdrFlag               () != getIdrFlag                (), true  );
  if( getIdrFlag() )
  {
    ROTRS ( pcLastSliceHeader->getIdrPicId              () != getIdrPicId               (), true  );
  }
  if( getSPS().getPicOrderCntType() == 0 )
  {
    ROTRS ( pcLastSliceHeader->getPicOrderCntLsb        () != getPicOrderCntLsb         (), true  );
    ROTRS ( pcLastSliceHeader->getDeltaPicOrderCntBottom() != getDeltaPicOrderCntBottom (), true  );
  }
  if( getSPS().getPicOrderCntType() == 1 )
  {
    ROTRS ( pcLastSliceHeader->getDeltaPicOrderCnt0     () != getDeltaPicOrderCnt0      (), true  );
    ROTRS ( pcLastSliceHeader->getDeltaPicOrderCnt1     () != getDeltaPicOrderCnt1      (), true  );
  }
  return false;
}

ErrVal
SliceHeader::compare( const SliceHeader* pcSH,
                      Bool&              rbNewPic,
                      Bool&              rbNewFrame ) const
{
  rbNewPic = rbNewFrame = true;

  if( getIdrFlag() )
  {
    ROTRS( NULL == pcSH,                                    Err::m_nOK ); //very first frame
    ROTRS( ! pcSH->getIdrFlag(),                          Err::m_nOK ); //previous no idr
    ROTRS( getIdrPicId() != pcSH->getIdrPicId(),            Err::m_nOK );
  }

  ROTRS( NULL == pcSH,                                      Err::m_nOK );
  ROTRS( isRefPic() != pcSH->isRefPic(),              Err::m_nOK );
  ROTRS( getFrameNum() != pcSH->getFrameNum(),              Err::m_nOK );

  ROTRS( getFieldPicFlag() != pcSH->getFieldPicFlag(),      Err::m_nOK );

  const Bool bSameParity = ( ! getFieldPicFlag() || ( getBottomFieldFlag() == pcSH->getBottomFieldFlag() ) );
  rbNewFrame = bSameParity;

  if( getSPS().getPicOrderCntType() == 0 )
  {
    ROTRS( getPicOrderCntLsb() != pcSH->getPicOrderCntLsb(),  Err::m_nOK );
    ROTRS( ! getFieldPicFlag() && (getDeltaPicOrderCntBottom() != pcSH->getDeltaPicOrderCntBottom()), Err::m_nOK );
  }

  if( getSPS().getPicOrderCntType() == 1 )
  {
    ROTRS( getDeltaPicOrderCnt0() != pcSH->getDeltaPicOrderCnt0(), Err::m_nOK );
    ROTRS( ! getFieldPicFlag() && (getDeltaPicOrderCnt1() != pcSH->getDeltaPicOrderCnt1()), Err::m_nOK );
  }

  ROTRS( ! getFieldPicFlag() && (getDeltaPicOrderCntBottom() != pcSH->getDeltaPicOrderCntBottom()), Err::m_nOK );

  rbNewFrame = false;

  if( getIdrFlag() )
  {
    ROTRS( ! pcSH->getIdrFlag(),                          Err::m_nOK ); //prev no idr
  }
  ROTRS( ! bSameParity,  Err::m_nOK ); // differ

  rbNewPic = false;

  return Err::m_nOK;
}

ErrVal
SliceHeader::compareRedPic( const SliceHeader* pcSH,
                           Bool&              rbNewFrame ) const
{
  rbNewFrame = true;

  ROTRS( NULL == pcSH,                                          Err::m_nOK );
  ROTRS( getIdrPicId() != pcSH->getIdrPicId(),                  Err::m_nOK );
  ROTRS( getFrameNum() != pcSH->getFrameNum(),                  Err::m_nOK );
  ROTRS( getDependencyId() != pcSH->getDependencyId(),                    Err::m_nOK );
  ROTRS( getQualityId() != pcSH->getQualityId(),          Err::m_nOK );
  ROTRS( getFirstMbInSlice() != pcSH->getFirstMbInSlice(),      Err::m_nOK );
  ROTRS( (getNalRefIdc() == 0 )&&(pcSH->getNalRefIdc() != 0),   Err::m_nOK );
  ROTRS( (getNalRefIdc() != 0 )&&(pcSH->getNalRefIdc() == 0),   Err::m_nOK );
  ROTRS( getPicOrderCntLsb() != pcSH->getPicOrderCntLsb(),      Err::m_nOK );

  rbNewFrame = false;

  return Err::m_nOK;
}

ErrVal
SliceHeader::sliceHeaderBackup ( SliceHeader*                pcSH )
{
  pcSH->setIdrPicId       ( getIdrPicId()       );
  pcSH->setFrameNum       ( getFrameNum()       );
  pcSH->setDependencyId        ( getDependencyId()        );
  pcSH->setQualityId   ( getQualityId()   );
  pcSH->setFirstMbInSlice ( getFirstMbInSlice() );
  pcSH->setNalRefIdc      ( getNalRefIdc()      );
  pcSH->setPicOrderCntLsb ( getPicOrderCntLsb() );

  return Err::m_nOK;
}

ErrVal 
SliceHeader::FMOInit()
{
  m_cFMO.sps_.frame_mbs_only_flag            = getSPS().getFrameMbsOnlyFlag(); 
  m_cFMO.sps_.mb_adaptive_frame_field_flag   = getSPS().getMbAdaptiveFrameFieldFlag(); 
  m_cFMO.sps_.pic_height_in_map_units_minus1 = ( getSPS().getFrameMbsOnlyFlag() ? getSPS().getFrameHeightInMbs() : getSPS().getFrameHeightInMbs() >> 1 ) - 1;
  m_cFMO.sps_.pic_width_in_mbs_minus1        = getSPS().getFrameWidthInMbs() - 1;

  m_cFMO.pps_.num_slice_groups_minus1 = getPPS().getNumSliceGroupsMinus1();
  m_cFMO.pps_.slice_group_map_type    = getPPS().getSliceGroupMapType();
  m_cFMO.pps_.num_slice_group_map_units_minus1 = getPPS().getNumSliceGroupMapUnitsMinus1();	  
  m_cFMO.pps_.copy_run_length_minus1(getPPS().getArrayRunLengthMinus1());
  m_cFMO.pps_.copy_top_left(getPPS().getArrayTopLeft());
  m_cFMO.pps_.copy_bottom_right(getPPS().getArrayBottomRight());
  m_cFMO.pps_.slice_group_change_direction_flag = getPPS().getSliceGroupChangeDirection_flag();
  m_cFMO.pps_.slice_group_change_rate_minus1    = getPPS().getSliceGroupChangeRateMinus1();
  m_cFMO.pps_.copy_slice_group_id(getPPS().getArraySliceGroupId());

  m_cFMO.img_.field_pic_flag      = getFieldPicFlag();
  m_cFMO.img_.PicHeightInMapUnits = m_cFMO.sps_.pic_height_in_map_units_minus1 + 1; 
  m_cFMO.img_.PicWidthInMbs       = getSPS().getFrameWidthInMbs();
  m_cFMO.img_.PicSizeInMbs        = (getSPS().getFrameHeightInMbs()>> (UChar)getFieldPicFlag())*getSPS().getFrameWidthInMbs();
  m_cFMO.img_.slice_group_change_cycle = getSliceGroupChangeCycle();

  m_cFMO.init();
  m_cFMO.StartPicture();
  return Err::m_nOK;
}

ErrVal 
SliceHeader::FMOUninit()
{
  m_cFMO.finit();
  return Err::m_nOK;
}

Int
SliceHeader::getNumMbInSlice()
{  
  Int SliceID = m_cFMO.getSliceGroupId(getFirstMbInSlice());
  return m_cFMO.getNumMbInSliceGroup(SliceID);
}

ErrVal
SliceHeader::xInitScalingMatrix()
{
  ROF( parameterSetsInitialized() );

  if( ! getSPS().getSeqScalingMatrixPresentFlag() && ! getPPS().getPicScalingMatrixPresentFlag() )
  {
    SliceHeaderSyntax::getScalingMatrix().setAll( 0 );
    return Err::m_nOK;
  }
  for( UInt uiIndex = 0; uiIndex < SliceHeaderSyntax::getScalingMatrix().size(); uiIndex++ )
  {
    const UChar* puc = getPPS().getPicScalingMatrix().get( uiIndex );
    if( ! puc )
    {
      puc = getSPS().getSeqScalingMatrix().get( uiIndex );
    }
    if( ! puc && ( uiIndex == 1 || uiIndex == 2 || uiIndex == 4 || uiIndex == 5 ) )
    {
      puc = SliceHeaderSyntax::getScalingMatrix().get( uiIndex - 1 );
    }
    if( ! puc )
    {
      switch( uiIndex )
      {
      case 0 : puc = g_aucScalingMatrixDefault4x4Intra; break;
      case 3 : puc = g_aucScalingMatrixDefault4x4Inter; break;
      case 6 : puc = g_aucScalingMatrixDefault8x8Intra; break;
      case 7 : puc = g_aucScalingMatrixDefault8x8Inter; break;
      default:
        RERR();
      }
    }
    SliceHeaderSyntax::getScalingMatrix().set( uiIndex, puc );
  }
  return Err::m_nOK;
}




H264AVC_NAMESPACE_END
