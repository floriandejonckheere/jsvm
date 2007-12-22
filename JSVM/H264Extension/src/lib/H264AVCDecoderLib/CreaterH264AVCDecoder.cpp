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


#include "H264AVCDecoder.h"
#include "GOPDecoder.h"
#include "ControlMngH264AVCDecoder.h"
#include "SliceReader.h"
#include "SliceDecoder.h"
#include "UvlcReader.h"
#include "MbParser.h"
#include "MbDecoder.h"
#include "NalUnitParser.h"
#include "BitReadBuffer.h"
#include "CabacReader.h"
#include "CabaDecoder.h"

#include "H264AVCCommonLib/MbData.h"
#include "H264AVCCommonLib/LoopFilter.h"
#include "H264AVCCommonLib/Transform.h"
#include "H264AVCCommonLib/IntraPrediction.h"
#include "H264AVCCommonLib/MotionCompensation.h"
#include "H264AVCCommonLib/YuvBufferCtrl.h"
#include "H264AVCCommonLib/QuarterPelFilter.h"
#include "H264AVCCommonLib/SampleWeighting.h"
#include "H264AVCCommonLib/ParameterSetMng.h"
#include "H264AVCCommonLib/PocCalculator.h"

#include "CreaterH264AVCDecoder.h"

#include "H264AVCCommonLib/TraceFile.h"
#include "H264AVCCommonLib/ReconstructionBypass.h"



H264AVC_NAMESPACE_BEGIN

SliceDataNALUnit::SliceDataNALUnit( BinData* pcBinData, const PrefixHeader& rcPrefixHeader )
: m_bHeaderExtensionPresent   ( true )
, m_bSliceHeaderPresent       ( false )
, m_pcBinDataPrefix           ( pcBinData )
, m_pcBinData                 ( 0 )
, m_eNalRefIdc                ( rcPrefixHeader.getNalRefIdc() )
, m_eNalUnitType              ( rcPrefixHeader.getNalUnitType() )
, m_bIdrFlag                  ( rcPrefixHeader.getIdrFlag() )
, m_uiPriorityId              ( rcPrefixHeader.getPriorityId() )
, m_bNoInterLayerPredFlag     ( rcPrefixHeader.getNoInterLayerPredFlag() )
, m_uiDependencyId            ( rcPrefixHeader.getDependencyId() )
, m_uiQualityId               ( rcPrefixHeader.getQualityId() )
, m_uiTemporalId              ( rcPrefixHeader.getTemporalId() )
, m_bUseRefBasePicFlag        ( rcPrefixHeader.getUseRefBasePicFlag() )
, m_bDiscardableFlag          ( rcPrefixHeader.getDiscardableFlag() )
, m_bOutputFlag               ( rcPrefixHeader.getOutputFlag() )
, m_bTCoeffLevelPredictionFlag( false )
, m_uiPPSId                   ( MSYS_UINT_MAX )
, m_uiSPSId                   ( MSYS_UINT_MAX )
, m_uiFrameNum                ( MSYS_UINT_MAX )
, m_uiRedundantPicCnt         ( MSYS_UINT_MAX )
, m_uiRefLayerDQId            ( MSYS_UINT_MAX )
, m_uiFrameWidthInMb          ( 0 )
, m_uiFrameHeightInMb         ( 0 )
, m_bLastSliceInAccessUnit    ( false )
, m_bLastAccessUnitInStream   ( false )
, m_bPartOfIDRAccessUnit      ( false )
, m_bHighestRewriteLayer      ( false )
{
  m_auiCroppingRectangle[0] = 0;
  m_auiCroppingRectangle[1] = 0;
  m_auiCroppingRectangle[2] = 0;
  m_auiCroppingRectangle[3] = 0;
}

SliceDataNALUnit::SliceDataNALUnit( BinData* pcBinData, const SliceHeader& rcSliceHeader )
: m_bHeaderExtensionPresent   (!rcSliceHeader.isH264AVCCompatible() )
, m_bSliceHeaderPresent       ( true )
, m_pcBinDataPrefix           ( 0 )
, m_pcBinData                 ( pcBinData )
, m_eNalRefIdc                ( rcSliceHeader.getNalRefIdc() )
, m_eNalUnitType              ( rcSliceHeader.getNalUnitType() )
, m_bIdrFlag                  ( rcSliceHeader.getIdrFlag() )
, m_uiPriorityId              ( rcSliceHeader.getPriorityId() )
, m_bNoInterLayerPredFlag     ( rcSliceHeader.getNoInterLayerPredFlag() )
, m_uiDependencyId            ( rcSliceHeader.getDependencyId() )
, m_uiQualityId               ( rcSliceHeader.getQualityId() )
, m_uiTemporalId              ( rcSliceHeader.getTemporalId() )
, m_bUseRefBasePicFlag        ( rcSliceHeader.getUseRefBasePicFlag() )
, m_bDiscardableFlag          ( rcSliceHeader.getDiscardableFlag() )
, m_bOutputFlag               ( rcSliceHeader.getOutputFlag() )
, m_bTCoeffLevelPredictionFlag( rcSliceHeader.getTCoeffLevelPredictionFlag() )
, m_uiPPSId                   ( rcSliceHeader.getPicParameterSetId() )
, m_uiSPSId                   ( rcSliceHeader.getPPS().getSeqParameterSetId() )
, m_uiFrameNum                ( rcSliceHeader.getFrameNum() )
, m_uiRedundantPicCnt         ( rcSliceHeader.getRedundantPicCnt() )
, m_uiRefLayerDQId            ( rcSliceHeader.getRefLayerDQId() )
, m_uiFrameWidthInMb          ( rcSliceHeader.getSPS().getFrameWidthInMbs() )
, m_uiFrameHeightInMb         ( rcSliceHeader.getSPS().getFrameHeightInMbs() )
, m_bLastSliceInAccessUnit    ( false )
, m_bLastAccessUnitInStream   ( false )
, m_bPartOfIDRAccessUnit      ( false )
, m_bHighestRewriteLayer      ( false )
{
  m_auiCroppingRectangle[0] = rcSliceHeader.getSPS().getFrameCropLeftOffset  () << 1;
  m_auiCroppingRectangle[1] = rcSliceHeader.getSPS().getFrameCropRightOffset () << 1;
  m_auiCroppingRectangle[2] = rcSliceHeader.getSPS().getFrameCropTopOffset   () << ( rcSliceHeader.getSPS().getFrameMbsOnlyFlag() ? 1 : 2 );
  m_auiCroppingRectangle[3] = rcSliceHeader.getSPS().getFrameCropBottomOffset() << ( rcSliceHeader.getSPS().getFrameMbsOnlyFlag() ? 1 : 2 );
}

SliceDataNALUnit::~SliceDataNALUnit()
{
  if( m_pcBinDataPrefix )
  {
    m_pcBinDataPrefix->deleteData();
    delete m_pcBinDataPrefix;
  }
  if( m_pcBinData )
  {
    m_pcBinData->deleteData();
    delete m_pcBinData;
  }
}

ErrVal
SliceDataNALUnit::update( BinData* pcBinData, const SliceHeader& rcSliceHeader )
{
  ROT( m_bSliceHeaderPresent );
  ROF( rcSliceHeader.isH264AVCCompatible() );
  ROF( m_bIdrFlag == rcSliceHeader.getIdrFlag() );
  m_bSliceHeaderPresent         = true;
  m_pcBinData                   = pcBinData;
  m_eNalRefIdc                  = rcSliceHeader.getNalRefIdc();
  m_eNalUnitType                = rcSliceHeader.getNalUnitType();
  m_bTCoeffLevelPredictionFlag  = false;
  m_uiPPSId                     = rcSliceHeader.getPicParameterSetId();
  m_uiSPSId                     = rcSliceHeader.getPPS().getSeqParameterSetId();
  m_uiFrameNum                  = rcSliceHeader.getFrameNum();
  m_uiRedundantPicCnt           = rcSliceHeader.getRedundantPicCnt();
  m_uiRefLayerDQId              = rcSliceHeader.getRefLayerDQId();
  m_uiFrameWidthInMb            = rcSliceHeader.getSPS().getFrameWidthInMbs();
  m_uiFrameHeightInMb           = rcSliceHeader.getSPS().getFrameHeightInMbs();
  m_auiCroppingRectangle[0]     = rcSliceHeader.getSPS().getFrameCropLeftOffset  () << 1;
  m_auiCroppingRectangle[1]     = rcSliceHeader.getSPS().getFrameCropRightOffset () << 1;
  m_auiCroppingRectangle[2]     = rcSliceHeader.getSPS().getFrameCropTopOffset   () << ( rcSliceHeader.getSPS().getFrameMbsOnlyFlag() ? 1 : 2 );
  m_auiCroppingRectangle[3]     = rcSliceHeader.getSPS().getFrameCropBottomOffset() << ( rcSliceHeader.getSPS().getFrameMbsOnlyFlag() ? 1 : 2 );
  return Err::m_nOK;
}

AccessUnitSlices::AccessUnitSlices()
: m_bEndOfStream                            ( false )
, m_bComplete                               ( false )
, m_pcFirstSliceDataNALUnitOfNextAccessUnit ( 0 )
, m_pcLastPrefixHeader                      ( 0 )
, m_pcLastSliceHeader                       ( 0 )
{
}

AccessUnitSlices::~AccessUnitSlices()
{
  while( ! m_cSliceDataNalUnitList.empty() )
  {
    SliceDataNALUnit* pcSliceDataNalUnit = m_cSliceDataNalUnitList.popFront();
    delete            pcSliceDataNalUnit;
  }
  delete m_pcFirstSliceDataNALUnitOfNextAccessUnit;
  delete m_pcLastPrefixHeader;
  delete m_pcLastSliceHeader;
}

ErrVal
AccessUnitSlices::updateEndOfStream()
{
  ROT ( m_bComplete );
  ROT ( m_bEndOfStream );
  xSetComplete();
  return Err::m_nOK;
}

ErrVal
AccessUnitSlices::update( BinData* pcBinData, SliceHeader& rcSliceHeader )
{
  ROT( m_bComplete );
  ROT( m_bEndOfStream );
  ROF( pcBinData );

  //===== create or extract and update slice data NAL unit =====
  SliceDataNALUnit* pcSliceDataNalUnit = 0;
  if( m_pcLastPrefixHeader && rcSliceHeader.isH264AVCCompatible() )
  {
    pcSliceDataNalUnit = m_cSliceDataNalUnitList.popBack();
    RNOK( pcSliceDataNalUnit->update( pcBinData, rcSliceHeader ) );
  }
  else
  {
    pcSliceDataNalUnit = new SliceDataNALUnit( pcBinData, rcSliceHeader );
    ROF ( pcSliceDataNalUnit );
  }

  //===== check for new access unit =====
  if( rcSliceHeader.isFirstSliceOfNextAccessUnit( m_pcLastSliceHeader ) )
  {
    xSetComplete( false, pcSliceDataNalUnit, &rcSliceHeader );
    return Err::m_nOK;
  }

  //===== update slice data NAL unit list and header references =====
  m_cSliceDataNalUnitList.push_back( pcSliceDataNalUnit );
  delete m_pcLastPrefixHeader;
  delete m_pcLastSliceHeader;
  m_pcLastPrefixHeader  = 0;
  m_pcLastSliceHeader   = &rcSliceHeader;
  return Err::m_nOK;
}

ErrVal
AccessUnitSlices::update( BinData* pcBinData, PrefixHeader& rcPrefixHeader )
{
  ROT( m_bComplete );
  ROT( m_bEndOfStream );
  ROF( pcBinData );

  //===== create or extract and update slice data NAL unit =====
  SliceDataNALUnit* pcSliceDataNalUnit = new SliceDataNALUnit( pcBinData, rcPrefixHeader );
  ROF ( pcSliceDataNalUnit );

  //===== update slice data NAL unit list and header references =====
  m_cSliceDataNalUnitList.push_back( pcSliceDataNalUnit );
  delete m_pcLastPrefixHeader;
  m_pcLastPrefixHeader  = &rcPrefixHeader;
  return Err::m_nOK;
}

ErrVal
AccessUnitSlices::getNextSliceDataNalUnit( SliceDataNALUnit*& rpcSliceDataNalUnit )
{
  ROF( m_bComplete );
  rpcSliceDataNalUnit = m_cSliceDataNalUnitList.popFront();
  if( m_cSliceDataNalUnitList.empty() )
  {
    xReInit();
  }
  return Err::m_nOK;
}

ErrVal
AccessUnitSlices::getRefToTargetLayerSliceData( const SliceDataNALUnit*& rpcSliceDataNalUnit ) const
{
  ROF( m_bComplete );
  rpcSliceDataNalUnit = *m_cSliceDataNalUnitList.rbegin();
  return Err::m_nOK;
}

Void
AccessUnitSlices::xSetComplete( Bool              bEndOfStream,
                                SliceDataNALUnit* pcFirstSliceDataNALUnitOfNextAccessUnit,
                                SliceHeader*      pcSliceHeader )
{
  delete m_pcLastPrefixHeader;
  delete m_pcLastSliceHeader;
  m_bEndOfStream                            = bEndOfStream;
  m_bComplete                               = true;
  m_pcFirstSliceDataNALUnitOfNextAccessUnit = pcFirstSliceDataNALUnitOfNextAccessUnit;
  m_pcLastPrefixHeader                      = 0;
  m_pcLastSliceHeader                       = pcSliceHeader;

  //===== remove slice data structures that only contain prefix headers (could be used for error detection in later version) =====
  {
    MyList<SliceDataNALUnit*>::iterator iIter =  m_cSliceDataNalUnitList.begin  ();
    MyList<SliceDataNALUnit*>::iterator iEnd  =  m_cSliceDataNalUnitList.end    ();
    while( iIter != iEnd )
    {
      if( ! (*iIter)->isSliceHeaderPresent() )
      {
        delete *iIter;
        iIter = m_cSliceDataNalUnitList.erase( iIter );
      }
      else
      {
        iIter++;
      }
    }
  }

  //===== remove redundant slices (could be used for error concealment in later version) =====
  {
    MyList<SliceDataNALUnit*>::iterator iIter =  m_cSliceDataNalUnitList.begin  ();
    MyList<SliceDataNALUnit*>::iterator iEnd  =  m_cSliceDataNalUnitList.end    ();
    while( iIter != iEnd )
    {
      if( (*iIter)->getRedundantPicCnt() )
      {
        delete *iIter;
        iIter = m_cSliceDataNalUnitList.erase( iIter );
      }
      else
      {
        iIter++;
      }
    }
  }

  //===== remove non-required NAL units =====
  {
    MyList<SliceDataNALUnit*>::reverse_iterator iIter       = m_cSliceDataNalUnitList.rbegin();
    MyList<SliceDataNALUnit*>::reverse_iterator iEnd        = m_cSliceDataNalUnitList.rend  ();
    UInt                                        uiCurrDQId  = MSYS_UINT_MAX;
    UInt                                        uiNextDQId  = (*iIter)->getDQId();
    while( iIter != iEnd )
    {
      if( (*iIter)->getDQId() != uiCurrDQId && (*iIter)->getDQId() != uiNextDQId )
      {
        MyList<SliceDataNALUnit*>::reverse_iterator iNext     = iIter; iNext++;
        MyList<SliceDataNALUnit*>::iterator         iToDelete = iNext.base();
        delete *iToDelete;
        iToDelete = m_cSliceDataNalUnitList.erase( iToDelete );
        iIter     = static_cast<MyList<SliceDataNALUnit*>::reverse_iterator>( iToDelete );
        iEnd      = m_cSliceDataNalUnitList.rend();
      }
      else
      {
        if( (*iIter)->getDQId() == uiNextDQId )
        {
          uiCurrDQId  = uiNextDQId;
          uiNextDQId  = (*iIter)->getRefLayerDQId();
        }
        iIter++;
      }
    }
    AOF( uiNextDQId == MSYS_UINT_MAX ); // we are missing required packets (replace with error concealment in later version)
  }

  //===== set highest DQId for which AVC rewriting is possible =====
  UInt  uiHighestRewriteDQId = MSYS_UINT_MAX;
  {
    MyList<SliceDataNALUnit*>::iterator iIter = m_cSliceDataNalUnitList.begin();
    MyList<SliceDataNALUnit*>::iterator iEnd  = m_cSliceDataNalUnitList.end  ();
    while( iIter != iEnd )
    {
      if( (*iIter)->getNalUnitType() == NAL_UNIT_CODED_SLICE      ||
          (*iIter)->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR  ||
          (*iIter)->getTCoeffLevelPredictionFlag() )
      {
        uiHighestRewriteDQId = (*iIter)->getDQId();
      }
      iIter++;
    }
  }

  //===== set slice data properties =====
  MyList<SliceDataNALUnit*>::iterator iIter = m_cSliceDataNalUnitList.begin();
  MyList<SliceDataNALUnit*>::iterator iEnd  = m_cSliceDataNalUnitList.end  ();
  MyList<SliceDataNALUnit*>::iterator iNext = iIter; iNext++;
  while( iIter != iEnd )
  {
    //----- check whether slice is the last slice in a layer or dependency representation ----
    Bool  bLastSliceInLayerRepresentation       = false;
    Bool  bLastSliceInDependencyRepresentation  = false;
    if( iNext == iEnd )
    {
      bLastSliceInLayerRepresentation       = true;
      bLastSliceInDependencyRepresentation  = true;
    }
    else
    {
      bLastSliceInLayerRepresentation       = ( (*iIter)->getDQId         () != (*iNext)->getDQId         () );
      bLastSliceInDependencyRepresentation  = ( (*iIter)->getDependencyId () != (*iNext)->getDependencyId () );
    }
    //----- check whether slices is part of an IDR access unit -----
    Bool  bIsPartOfIDRAccessUnit  = false;
    for( MyList<SliceDataNALUnit*>::iterator iNIter = iIter; iNIter != iEnd && ! bIsPartOfIDRAccessUnit; iNIter++ )
    {
      bIsPartOfIDRAccessUnit = (*iNIter)->getIdrFlag();
    }
    //----- set parameters -----
    (*iIter)->setDQIdMax                            ( (*iIter)->getDQId         () == (*m_cSliceDataNalUnitList.rbegin())->getDQId        () );
    (*iIter)->setDependencyIdMax                    ( (*iIter)->getDependencyId () == (*m_cSliceDataNalUnitList.rbegin())->getDependencyId() );
    (*iIter)->setLastSliceInLayerRepresentation     ( bLastSliceInLayerRepresentation );
    (*iIter)->setLastSliceInDependencyRepresentation( bLastSliceInDependencyRepresentation );
    (*iIter)->setLastSliceInAccessUnit              ( iNext == iEnd );
    (*iIter)->setLastAccessUnitInStream             ( m_bEndOfStream );
    (*iIter)->setPartOfIDRAccessUnit                ( bIsPartOfIDRAccessUnit );
    (*iIter)->setHighestRewriteLayer                ( (*iIter)->getDQId         () == uiHighestRewriteDQId );
    //----- update iterators ----
    iIter = iNext;
    if( iNext != iEnd )
    {
      iNext++;
    }
  }
}

Void
AccessUnitSlices::xReInit()
{
  m_cSliceDataNalUnitList.push_back( m_pcFirstSliceDataNALUnitOfNextAccessUnit );
  m_bComplete                               = false;
  m_pcFirstSliceDataNALUnitOfNextAccessUnit = 0;
}





CreaterH264AVCDecoder::CreaterH264AVCDecoder()
: m_pcH264AVCDecoder      ( 0 )
, m_pcParameterSetMng     ( 0 )
, m_pcSliceReader         ( 0 )
, m_pcNalUnitParser       ( 0 )
, m_pcSliceDecoder        ( 0 )
, m_pcControlMng          ( 0 )
, m_pcBitReadBuffer       ( 0 )
, m_pcUvlcReader          ( 0 )
, m_pcMbParser            ( 0 )
, m_pcLoopFilter          ( 0 )
, m_pcMbDecoder           ( 0 )
, m_pcTransform           ( 0 )
, m_pcIntraPrediction     ( 0 )
, m_pcMotionCompensation  ( 0 )
, m_pcQuarterPelFilter    ( 0 )
, m_pcCabacReader         ( 0 )
, m_pcSampleWeighting     ( 0 )
#ifdef SHARP_AVC_REWRITE_OUTPUT
, m_pcRewriteEncoder      ( 0 )
#endif
{
  ::memset( m_apcDecodedPicBuffer,     0x00, MAX_LAYERS * sizeof( Void* ) );
  ::memset( m_apcLayerDecoder,         0x00, MAX_LAYERS * sizeof( Void* ) );
  ::memset( m_apcPocCalculator,        0x00, MAX_LAYERS * sizeof( Void* ) );
  ::memset( m_apcYuvFullPelBufferCtrl, 0x00, MAX_LAYERS * sizeof( Void* ) );
}

CreaterH264AVCDecoder::~CreaterH264AVCDecoder()
{
}

ErrVal
CreaterH264AVCDecoder::create( CreaterH264AVCDecoder*& rpcCreaterH264AVCDecoder )
{
  rpcCreaterH264AVCDecoder = new CreaterH264AVCDecoder;
  ROT( NULL == rpcCreaterH264AVCDecoder );
  RNOK( rpcCreaterH264AVCDecoder->xCreateDecoder() )
  return Err::m_nOK;
}

ErrVal
CreaterH264AVCDecoder::xCreateDecoder()
{
  RNOK( ParameterSetMng         ::create( m_pcParameterSetMng ) );
  RNOK( BitReadBuffer           ::create( m_pcBitReadBuffer ) );
  RNOK( NalUnitParser           ::create( m_pcNalUnitParser) );
  RNOK( SliceReader             ::create( m_pcSliceReader ) );
  RNOK( SliceDecoder            ::create( m_pcSliceDecoder ) );
  RNOK( UvlcReader              ::create( m_pcUvlcReader ) );
  RNOK( CabacReader             ::create( m_pcCabacReader ) );
  RNOK( MbParser                ::create( m_pcMbParser ) );
  RNOK( MbDecoder               ::create( m_pcMbDecoder ) );
  RNOK( LoopFilter              ::create( m_pcLoopFilter ) );
  RNOK( IntraPrediction         ::create( m_pcIntraPrediction ) );
  RNOK( MotionCompensation      ::create( m_pcMotionCompensation ) );
  RNOK( H264AVCDecoder          ::create( m_pcH264AVCDecoder ) );
  RNOK( ControlMngH264AVCDecoder::create( m_pcControlMng ) );
  RNOK( ReconstructionBypass    ::create( m_pcReconstructionBypass ) );
  RNOK( SampleWeighting         ::create( m_pcSampleWeighting ) );
  RNOK( QuarterPelFilter        ::create( m_pcQuarterPelFilter ) );
  RNOK( Transform               ::create( m_pcTransform ) );
#ifdef SHARP_AVC_REWRITE_OUTPUT
  RNOK( RewriteEncoder          ::create( m_pcRewriteEncoder ) );
#endif

  for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
  {
    RNOK( DecodedPicBuffer      ::create( m_apcDecodedPicBuffer     [uiLayer] ) );
    RNOK( LayerDecoder          ::create( m_apcLayerDecoder         [uiLayer] ) );
    RNOK( PocCalculator         ::create( m_apcPocCalculator        [uiLayer] ) );
    RNOK( YuvBufferCtrl         ::create( m_apcYuvFullPelBufferCtrl [uiLayer] ) );
  }

  return Err::m_nOK;
}


ErrVal
CreaterH264AVCDecoder::destroy()
{
  RNOK( m_pcSliceDecoder          ->destroy() );
  RNOK( m_pcSliceReader           ->destroy() );
  RNOK( m_pcBitReadBuffer         ->destroy() );
  RNOK( m_pcUvlcReader            ->destroy() );
  RNOK( m_pcMbParser              ->destroy() );
  RNOK( m_pcLoopFilter            ->destroy() );
  RNOK( m_pcMbDecoder             ->destroy() );
  RNOK( m_pcTransform             ->destroy() );
  RNOK( m_pcIntraPrediction       ->destroy() );
  RNOK( m_pcMotionCompensation    ->destroy() );
  RNOK( m_pcQuarterPelFilter      ->destroy() );
  RNOK( m_pcCabacReader           ->destroy() );
  RNOK( m_pcNalUnitParser         ->destroy() );
  RNOK( m_pcParameterSetMng       ->destroy() );
  RNOK( m_pcSampleWeighting       ->destroy() );
  RNOK( m_pcH264AVCDecoder        ->destroy() );
  RNOK( m_pcControlMng            ->destroy() );
  RNOK( m_pcReconstructionBypass  ->destroy() );
#ifdef SHARP_AVC_REWRITE_OUTPUT
  RNOK( m_pcRewriteEncoder        ->destroy() );
#endif

  for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
  {
    RNOK( m_apcDecodedPicBuffer    [uiLayer]->destroy() );
    RNOK( m_apcLayerDecoder        [uiLayer]->destroy() );
    RNOK( m_apcPocCalculator       [uiLayer]->destroy() );
    RNOK( m_apcYuvFullPelBufferCtrl[uiLayer]->destroy() );
  }
  delete this;
  return Err::m_nOK;
}

ErrVal
CreaterH264AVCDecoder::init( Bool bOpenTrace )
{
  if( bOpenTrace )
  {
    INIT_DTRACE;
    OPEN_DTRACE;
  }

  RNOK( m_pcBitReadBuffer         ->init() );
  RNOK( m_pcNalUnitParser         ->init( m_pcBitReadBuffer, m_pcUvlcReader ) );
  RNOK( m_pcUvlcReader            ->init( m_pcBitReadBuffer ) );
  RNOK( m_pcCabacReader           ->init( m_pcBitReadBuffer ) );
  RNOK( m_pcQuarterPelFilter      ->init() );
  RNOK( m_pcParameterSetMng       ->init() );
  RNOK( m_pcSampleWeighting       ->init() );
#ifdef SHARP_AVC_REWRITE_OUTPUT
  RNOK( m_pcSliceDecoder          ->init( m_pcMbDecoder, m_pcControlMng, m_pcRewriteEncoder ) );
#else
  RNOK( m_pcSliceDecoder          ->init( m_pcMbDecoder, m_pcControlMng ) );
#endif
  RNOK( m_pcSliceReader           ->init( m_pcMbParser ) );
  RNOK( m_pcMbParser              ->init() );
  RNOK( m_pcLoopFilter            ->init( m_pcControlMng, m_pcReconstructionBypass, false ) );
  RNOK( m_pcIntraPrediction       ->init() );
  RNOK( m_pcMotionCompensation    ->init( m_pcQuarterPelFilter, m_pcTransform, m_pcSampleWeighting ) );
  RNOK( m_pcMbDecoder             ->init( m_pcTransform, m_pcIntraPrediction, m_pcMotionCompensation ) );
  RNOK( m_pcH264AVCDecoder        ->init( m_pcNalUnitParser,
                                          m_pcUvlcReader,
                                          m_pcParameterSetMng,
                                          m_apcLayerDecoder ) );
  RNOK( m_pcReconstructionBypass  ->init() );
#ifdef SHARP_AVC_REWRITE_OUTPUT
  RNOK( m_pcRewriteEncoder        ->init() );
#endif

  for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
  {
    RNOK( m_apcDecodedPicBuffer[uiLayer]->init ( m_apcYuvFullPelBufferCtrl [uiLayer],
                                                 uiLayer ) );
#ifdef SHARP_AVC_REWRITE_OUTPUT
    RNOK( m_apcLayerDecoder    [uiLayer]->init ( uiLayer,
                                                 m_pcH264AVCDecoder,
                                                 m_pcNalUnitParser,
                                                 m_pcSliceReader,
                                                 m_pcSliceDecoder,
                                                 m_pcControlMng,
                                                 m_pcLoopFilter,
                                                 m_pcUvlcReader,
                                                 m_pcParameterSetMng,
                                                 m_apcPocCalculator        [uiLayer],
                                                 m_apcYuvFullPelBufferCtrl [uiLayer],
                                                 m_apcDecodedPicBuffer     [uiLayer],
                                                 m_pcMotionCompensation,
												                         m_pcReconstructionBypass,
                                                 m_pcRewriteEncoder ) );
#else
    RNOK( m_apcLayerDecoder    [uiLayer]->init ( uiLayer,
                                                 m_pcH264AVCDecoder,
                                                 m_pcNalUnitParser,
                                                 m_pcSliceReader,
                                                 m_pcSliceDecoder,
                                                 m_pcControlMng,
                                                 m_pcLoopFilter,
                                                 m_pcUvlcReader,
                                                 m_pcParameterSetMng,
                                                 m_apcPocCalculator        [uiLayer],
                                                 m_apcYuvFullPelBufferCtrl [uiLayer],
                                                 m_apcDecodedPicBuffer     [uiLayer],
                                                 m_pcMotionCompensation,
												                         m_pcReconstructionBypass ) );
#endif
  }

  RNOK( m_pcControlMng            ->init( m_pcUvlcReader, 
                                          m_pcMbParser, 
                                          m_pcMotionCompensation, 
                                          m_apcYuvFullPelBufferCtrl, 
                                          m_pcCabacReader, 
                                          m_pcSampleWeighting, 
                                          m_apcLayerDecoder ) );

  return Err::m_nOK;
}

ErrVal
CreaterH264AVCDecoder::uninit( Bool bCloseTrace )
{
  RNOK( m_pcSampleWeighting       ->uninit() );
  RNOK( m_pcQuarterPelFilter      ->uninit() );
  RNOK( m_pcParameterSetMng       ->uninit() );
  RNOK( m_pcSliceDecoder          ->uninit() );
  RNOK( m_pcSliceReader           ->uninit() );
  RNOK( m_pcNalUnitParser         ->uninit() );
  RNOK( m_pcBitReadBuffer         ->uninit() );
  RNOK( m_pcUvlcReader            ->uninit() );
  RNOK( m_pcMbParser              ->uninit() );
  RNOK( m_pcLoopFilter            ->uninit() );
  RNOK( m_pcMbDecoder             ->uninit() );
  RNOK( m_pcIntraPrediction       ->uninit() );
  RNOK( m_pcMotionCompensation    ->uninit() );
  RNOK( m_pcCabacReader           ->uninit() );
  RNOK( m_pcH264AVCDecoder        ->uninit() );
  RNOK( m_pcControlMng            ->uninit() );
  RNOK( m_pcReconstructionBypass  ->uninit() );
#ifdef SHARP_AVC_REWRITE_OUTPUT
  RNOK( m_pcRewriteEncoder        ->uninit() );
#endif

  for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
  {
    RNOK( m_apcDecodedPicBuffer    [uiLayer] ->uninit() );
    RNOK( m_apcLayerDecoder        [uiLayer] ->uninit() );
    RNOK( m_apcYuvFullPelBufferCtrl[uiLayer] ->uninit() );
  }

  if( bCloseTrace )
  {
    CLOSE_DTRACE;
  }

  return Err::m_nOK;
}

ErrVal
CreaterH264AVCDecoder::initNALUnit( BinData*& rpcBinData, AccessUnitSlices& rcAccessUnitSlices )
{
  RNOK( m_pcH264AVCDecoder->initNALUnit( rpcBinData, rcAccessUnitSlices ) );
  return Err::m_nOK;
}

ErrVal
CreaterH264AVCDecoder::processSliceData( PicBuffer*         pcPicBuffer,
                                        PicBufferList&     rcPicBufferOutputList,
                                        PicBufferList&     rcPicBufferUnusedList,
                                        BinDataList&       rcBinDataList,
                                        SliceDataNALUnit&  rcSliceDataNALUnit )
{
  RNOK( m_pcH264AVCDecoder->processSliceData( pcPicBuffer, rcPicBufferOutputList, rcPicBufferUnusedList, rcBinDataList, rcSliceDataNALUnit ) );
  return Err::m_nOK;
}








H264AVCPacketAnalyzer::H264AVCPacketAnalyzer()
: m_pcBitReadBuffer       ( 0 )
, m_pcUvlcReader          ( 0 )
, m_pcNalUnitParser       ( 0 )
, m_uiStdAVCOffset        ( 0 )
, m_pcNonRequiredSEI      ( 0 )
, m_uiNonRequiredSeiFlag  ( 0 )
, m_uiPrevPicLayer        ( 0 )
, m_uiCurrPicLayer        ( 0 )
{
  for( Int iLayer = 0; iLayer < MAX_SCALABLE_LAYERS; iLayer++ )
  {
    m_silceIDOfSubPicLayer[iLayer] = -1;
  }
}


H264AVCPacketAnalyzer::~H264AVCPacketAnalyzer()
{
}


ErrVal
H264AVCPacketAnalyzer::create( H264AVCPacketAnalyzer*& rpcH264AVCPacketAnalyzer )
{
  rpcH264AVCPacketAnalyzer = new H264AVCPacketAnalyzer;
  ROT ( NULL == rpcH264AVCPacketAnalyzer );
  RNOK( rpcH264AVCPacketAnalyzer->xCreate() );
  return Err::m_nOK;
}



ErrVal
H264AVCPacketAnalyzer::xCreate()
{
  RNOK( BitReadBuffer::create( m_pcBitReadBuffer ) );
  RNOK( UvlcReader   ::create( m_pcUvlcReader    ) );
  RNOK( NalUnitParser::create( m_pcNalUnitParser  ) );

  return Err::m_nOK;
}



ErrVal
H264AVCPacketAnalyzer::destroy()
{
  RNOK( m_pcBitReadBuffer ->destroy() );
  RNOK( m_pcUvlcReader    ->destroy() );
  RNOK( m_pcNalUnitParser ->destroy() );
  delete this;
  return Err::m_nOK;
}



ErrVal
H264AVCPacketAnalyzer::init()
{
  RNOK( m_pcBitReadBuffer ->init() );
  RNOK( m_pcUvlcReader    ->init( m_pcBitReadBuffer ) );
  RNOK( m_pcNalUnitParser ->init( m_pcBitReadBuffer, m_pcUvlcReader ) );
  return Err::m_nOK;
}


ErrVal
H264AVCPacketAnalyzer::uninit()
{
  RNOK( m_pcBitReadBuffer ->uninit() );
  RNOK( m_pcUvlcReader    ->uninit() );
  RNOK( m_pcNalUnitParser ->uninit() );
  return Err::m_nOK;
}


ErrVal
H264AVCPacketAnalyzer::process( BinData*            pcBinData,
                                PacketDescription&  rcPacketDescription,
                                SEI::SEIMessage*&   pcScalableSEIMessage )
{
  ROF( pcBinData );

  //===== copy bin data and init NAL unit =====
  UChar*  pucBuffer = new UChar [ pcBinData->size() ];
  ROF( pucBuffer );
  ::memcpy( pucBuffer, pcBinData->data(), pcBinData->size() );
  BinData         cBinData( pucBuffer, pcBinData->size() );
  BinDataAccessor cBinDataAccessor;
  cBinData.setMemAccessor( cBinDataAccessor );
  RNOK( m_pcNalUnitParser->initNalUnit( cBinDataAccessor ) );


  pcScalableSEIMessage            = 0;
  NalRefIdc   eNalRefIdc          = m_pcNalUnitParser->getNalRefIdc           ();
  NalUnitType eNalUnitType        = m_pcNalUnitParser->getNalUnitType         ();
  UInt        uiLayer             = m_pcNalUnitParser->getDependencyId        ();
  UInt        uiLevel             = m_pcNalUnitParser->getTemporalId          ();
  UInt        uiFGSLayer          = m_pcNalUnitParser->getQualityId           ();
  UInt        uiSimplePriorityId  = m_pcNalUnitParser->getPriorityId          ();
  Bool        bDiscardableFlag    = m_pcNalUnitParser->getDiscardableFlag     ();
  Bool        bApplyToNext        = false;
  Bool        bParameterSet       = ( eNalUnitType == NAL_UNIT_SPS || eNalUnitType == NAL_UNIT_SUBSET_SPS || eNalUnitType == NAL_UNIT_PPS );
  Bool        bScalable           = ( eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE );
  UInt        uiSPSid             = 0;
  UInt        uiPPSid             = 0;
  
  if( eNalUnitType == NAL_UNIT_CODED_SLICE || eNalUnitType == NAL_UNIT_CODED_SLICE_IDR )
  {
    uiLevel = ( eNalRefIdc > 0 ? 0 : 1 + m_uiStdAVCOffset );
  }
  
  rcPacketDescription.uiNumLevelsQL       = 0;
  rcPacketDescription.bDiscardableHRDSEI  = false;
  for( UInt ui = 0; ui < MAX_NUM_RD_LEVELS; ui++ )
  {
    rcPacketDescription.auiPriorityLevelPR[ui] = 0;
  }

  if( eNalUnitType == NAL_UNIT_SEI )
  {
    SEI::MessageList cMessageList;
    ParameterSetMng* pcParameterSetMng = NULL;
    RNOK( SEI::read( m_pcUvlcReader, cMessageList, pcParameterSetMng ) );

    SEI::MessageList::iterator iter = cMessageList.begin();
    while( ! cMessageList.empty() )
    {
      SEI::SEIMessage* pcSEIMessage = cMessageList.popBack();

      switch( pcSEIMessage->getMessageType() )
      {
      case SEI::SUB_SEQ_INFO:
        {
          SEI::SubSeqInfo* pcSubSeqInfo = (SEI::SubSeqInfo*) pcSEIMessage;
          uiLevel       = pcSubSeqInfo->getSubSeqLayerNum();
          uiLayer       = 0;
          bApplyToNext  = true;
          delete pcSEIMessage;
          break;
        }
      case SEI::SCALABLE_SEI:
        {
          uiLevel = 0;
          uiLayer = 0;
          pcScalableSEIMessage = pcSEIMessage;
          {
            //====set parameters used for further parsing =====
            SEI::ScalableSei* pcSEI    = (SEI::ScalableSei*)pcSEIMessage;
            UInt uiNumScalableLayers  = pcSEI->getNumLayersMinus1() + 1;
            for(UInt uiIndex = 0; uiIndex < uiNumScalableLayers; uiIndex++ )
            {
              if( pcSEI->getDependencyId( uiIndex ) == 0 )
              {
                m_uiStdAVCOffset = pcSEI->getTemporalId( uiIndex );
                pcSEI->setStdAVCOffset( m_uiStdAVCOffset-1 );
              }
              else
              {
                break;
              }
            }
          }

          SEI::ScalableSei* pcSEI    = (SEI::ScalableSei*)pcSEIMessage;
          m_uiNum_layers = pcSEI->getNumLayersMinus1() + 1;
          for(UInt i=0; i < m_uiNum_layers; i++)
          {
            m_ID_ROI[i] = pcSEI->getRoiId(i);
            m_ID_Dependency[i] = pcSEI->getDependencyId(i);
          }
          break;
        }

      case SEI::MOTION_SEI:
        {
          SEI::MotionSEI* pcSEI               = (SEI::MotionSEI*)pcSEIMessage;
          m_silceIDOfSubPicLayer[m_layer_id]  = pcSEI->m_slice_group_id[0];
          break;
        }

      case SEI::SCALABLE_SEI_LAYERS_NOT_PRESENT:
      case SEI::SCALABLE_SEI_DEPENDENCY_CHANGE:
        {
          pcScalableSEIMessage = pcSEIMessage;
          break;
        }

      case SEI::SUB_PIC_SEI:
        {
          SEI::SubPicSei* pcSEI    = (SEI::SubPicSei*)pcSEIMessage;
          m_layer_id          = pcSEI->getDependencyId();
          bApplyToNext  = true;
          break;
        }

      case SEI::PRIORITYLEVEL_SEI:
        {
          UInt uiNum = 0;
          UInt uiPriorityLevel = 0;
          SEI::PriorityLevelSEI* pcSEI           = (SEI::PriorityLevelSEI*)pcSEIMessage;
          uiNum = pcSEI->getNumPriorityIds();
          rcPacketDescription.uiNumLevelsQL = uiNum;
          for(UInt ui = 0; ui < uiNum; ui++)
          {
            uiPriorityLevel = pcSEI->getAltPriorityId(ui);
            rcPacketDescription.auiPriorityLevelPR[ui] = uiPriorityLevel;
          }
          uiLayer = pcSEI->getPrDependencyId();
          bApplyToNext = true;
          break;
        }

      case SEI::NON_REQUIRED_SEI:
        {
          m_pcNonRequiredSEI = (SEI::NonRequiredSei*) pcSEIMessage;
          m_uiNonRequiredSeiFlag = 1;
          break;
        }

      case SEI::SCALABLE_NESTING_SEI:
        {
          Bool bAllPicturesInAuFlag;
          UInt uiNumPictures;
          UInt *puiDependencyId, *puiQualityLevel;
          SEI::ScalableNestingSei* pcSEI = (SEI::ScalableNestingSei*)pcSEIMessage;
          bAllPicturesInAuFlag = pcSEI->getAllPicturesInAuFlag();
          if( bAllPicturesInAuFlag == 0 )
          {
            uiNumPictures = pcSEI->getNumPictures();
            ROT( uiNumPictures == 0 );
            puiDependencyId = new UInt[uiNumPictures];
            puiQualityLevel = new UInt[uiNumPictures];
            for( UInt uiIndex = 0; uiIndex < uiNumPictures; uiIndex++ )
            {
              puiDependencyId[uiIndex] = pcSEI->getDependencyId(uiIndex);
              puiQualityLevel[uiIndex] = pcSEI->getQualityId(uiIndex);
            }
            delete puiDependencyId;
            delete puiQualityLevel;
          }
          bApplyToNext = true;
          rcPacketDescription.bDiscardableHRDSEI = true;
          break;
        }

      case SEI::BUFFERING_PERIOD:
        {
          rcPacketDescription.bDiscardableHRDSEI = true;
          rcPacketDescription.bDiscardable = true;
          break;
        }

      case SEI::PIC_TIMING:
        {
          rcPacketDescription.bDiscardableHRDSEI = true;
          rcPacketDescription.bDiscardable = true;
          break;
        }

      case SEI::INTEGRITY_CHECK_SEI:
        {
          break;
        }

      case SEI::REDUNDANT_PIC_SEI:
        {
          SEI::RedundantPicSei* pcSEI = (SEI::RedundantPicSei*) pcSEIMessage;
          UInt uiNumDIdMinus1;
          UInt uiNumDId;
          UInt *puiNumQIdMinus1, *puiDependencyId;
          UInt **ppuiQualityId, **ppuiNumRedundantPicsMinus1;
          UInt ***pppuiRedundantPicCntMinus1;

          uiNumDIdMinus1 = pcSEI->getNumDIdMinus1( );
          uiNumDId = uiNumDIdMinus1 + 1;
          puiNumQIdMinus1                = new UInt[uiNumDId];
          puiDependencyId                = new UInt[uiNumDId]; 
          ppuiQualityId                  = new UInt*[uiNumDId];
          ppuiNumRedundantPicsMinus1     = new UInt*[uiNumDId];
          pppuiRedundantPicCntMinus1     = new UInt**[uiNumDId];
          for(UInt ui = 0; ui <= uiNumDIdMinus1; ui++)
          {
            puiDependencyId[ui] = pcSEI->getDependencyId ( ui );
            puiNumQIdMinus1[ui] = pcSEI->getNumQIdMinus1 ( ui );
            ppuiQualityId[ui]                = new UInt[puiNumQIdMinus1[ui]+1];
            ppuiNumRedundantPicsMinus1[ui]   = new UInt[puiNumQIdMinus1[ui]+1];
            pppuiRedundantPicCntMinus1[ui]    = new UInt*[puiNumQIdMinus1[ui]+1];
            for(UInt uj = 0; uj <= puiNumQIdMinus1[ui]; uj++)
            {
              ppuiQualityId[ui][uj]  = pcSEI->getQualityId ( ui, uj );
              ppuiNumRedundantPicsMinus1[ui][uj]  = pcSEI->getNumRedundantPicsMinus1 ( ui, uj );
              pppuiRedundantPicCntMinus1[ui][uj] = new UInt[ppuiNumRedundantPicsMinus1[ui][uj] +1];
              for(UInt uk = 0; uk <= ppuiNumRedundantPicsMinus1[ui][uj]; uk++)
              {
                pppuiRedundantPicCntMinus1[ui][uj][uk] = pcSEI->getRedundantPicCntMinus1 ( ui, uj, uk );
              }
            }                                                           			                                               		                                
          }

          delete puiNumQIdMinus1;
          delete puiDependencyId;
          delete ppuiQualityId;
          delete ppuiNumRedundantPicsMinus1;
          delete pppuiRedundantPicCntMinus1;

          break;
        }

      case SEI::TL_SWITCHING_POINT_SEI:
        {
          break;
        }

      case SEI::TL0_DEP_REP_IDX_SEI:
        {
          break;
        }

      default:
        {
          delete pcSEIMessage;
        }
      }
    }
  }
  else if( eNalUnitType == NAL_UNIT_SPS || eNalUnitType == NAL_UNIT_SUBSET_SPS )
  {
    SequenceParameterSet* pcSPS = NULL;
    RNOK( SequenceParameterSet::create  ( pcSPS   ) );
    RNOK( pcSPS->read( m_pcUvlcReader, eNalUnitType ) );
    uiSPSid = pcSPS->getSeqParameterSetId();
    pcSPS->destroy();
  }
  else if( eNalUnitType == NAL_UNIT_PPS )
  {
    PictureParameterSet* pcPPS = NULL;
    RNOK( PictureParameterSet::create  ( pcPPS   ) );
    RNOK( pcPPS->read( m_pcUvlcReader, eNalUnitType ) );
    uiPPSid = pcPPS->getPicParameterSetId();
    uiSPSid = pcPPS->getSeqParameterSetId();

    m_uiNumSliceGroupsMinus1 = pcPPS->getNumSliceGroupsMinus1();
    for(UInt i=0; i<= m_uiNumSliceGroupsMinus1; i++)
    {
      uiaAddrFirstMBofROIs[uiPPSid ][i] = pcPPS->getTopLeft (i);
    }

    pcPPS->destroy();
    rcPacketDescription.SPSidRefByPPS[uiPPSid] = uiSPSid;
  }
  else if( eNalUnitType == NAL_UNIT_CODED_SLICE           ||
           eNalUnitType == NAL_UNIT_CODED_SLICE_IDR       ||
           eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE    )
  {
    if( ! ( uiLayer == 0 && uiFGSLayer == 0 && eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE ) )
    {
      UInt uiTemp;
      RNOK( m_pcUvlcReader->getUvlc( uiTemp,  "SH: first_mb_in_slice" ) );
      rcPacketDescription.uiFirstMb = uiTemp;

      RNOK( m_pcUvlcReader->getUvlc( uiTemp,  "SH: slice_type" ) );
      RNOK( m_pcUvlcReader->getUvlc( uiPPSid, "SH: pic_parameter_set_id" ) );
      uiSPSid = rcPacketDescription.SPSidRefByPPS[uiPPSid];

      m_uiCurrPicLayer = (uiLayer << 4) + uiFGSLayer;
      if((m_uiCurrPicLayer < m_uiPrevPicLayer || (m_uiCurrPicLayer == m_uiPrevPicLayer && m_uiCurrPicLayer == 0))&& m_uiNonRequiredSeiFlag != 1) //prefix unit
      {
        m_pcNonRequiredSEI->destroy();
        m_pcNonRequiredSEI = NULL;
      }
      m_uiNonRequiredSeiFlag = 0;
      m_uiPrevPicLayer = m_uiCurrPicLayer;
    }
  }
  
  m_pcNalUnitParser->closeNalUnit( false );

  rcPacketDescription.NalUnitType   = eNalUnitType;
  rcPacketDescription.SPSid         = uiSPSid;
  rcPacketDescription.PPSid         = uiPPSid;
  rcPacketDescription.Scalable      = bScalable;
  rcPacketDescription.ParameterSet  = bParameterSet;
  rcPacketDescription.Layer         = uiLayer;
  rcPacketDescription.FGSLayer      = uiFGSLayer;
  rcPacketDescription.Level         = uiLevel;
  rcPacketDescription.ApplyToNext   = bApplyToNext;
  rcPacketDescription.uiPId         = uiSimplePriorityId;
  rcPacketDescription.bDiscardable  = bDiscardableFlag;
  return Err::m_nOK;
}


H264AVC_NAMESPACE_END
