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

#include "H264AVCCommonLib/SliceHeaderBase.h"
#include "H264AVCCommonLib/TraceFile.h"

#include "H264AVCCommonLib/CFMO.h"


H264AVC_NAMESPACE_BEGIN


Int iRandom( Int iMin, Int iMax )
{
  Int iRange = iMax - iMin + 1;
  Int iValue = rand() % iRange;
  return iMin + iValue;
}

ErrVal
SliceHeaderBase::PredWeight::createRandomParameters()
{
  setLumaWeightFlag  ( 0 == ( rand() & 1 ) );
  setChromaWeightFlag( 0 == ( rand() & 1 ) );

  if( getLumaWeightFlag() )
  {
    setLumaOffset   (     iRandom( -64, 63 ) );
    setLumaWeight   (     iRandom( -64, 63 ) );
  }
  if( getChromaWeightFlag() )
  {
    setChromaOffset ( 0,  iRandom( -64, 63 ) );
    setChromaWeight ( 0,  iRandom( -64, 63 ) );
    setChromaOffset ( 1,  iRandom( -64, 63 ) );
    setChromaWeight ( 1,  iRandom( -64, 63 ) );
  }
  return Err::m_nOK;
}

ErrVal
SliceHeaderBase::PredWeight::write( HeaderSymbolWriteIf* pcWriteIf ) const
{
  RNOK(   pcWriteIf->writeFlag( getLumaWeightFlag(),    "PWT: luma_weight_flag" ) );
  if( getLumaWeightFlag() )
  {
    RNOK( pcWriteIf->writeSvlc( getLumaWeight(),        "PWT: luma_weight" ) );
    RNOK( pcWriteIf->writeSvlc( getLumaOffset(),        "PWT: luma_offset" ) );
  }

  RNOK(   pcWriteIf->writeFlag( getChromaWeightFlag(),  "PWT: chroma_weight_flag" ) );
  if( getChromaWeightFlag() )
  {
    RNOK( pcWriteIf->writeSvlc( getChromaWeight( 0 ),   "PWT: cr_weight" ) );
    RNOK( pcWriteIf->writeSvlc( getChromaOffset( 0 ),   "PWT: cr_offset" ) );
    RNOK( pcWriteIf->writeSvlc( getChromaWeight( 1 ),   "PWT: cb_weight" ) );
    RNOK( pcWriteIf->writeSvlc( getChromaOffset( 1 ),   "PWT: cb_offset" ) );
  }

  return Err::m_nOK;
}


ErrVal
SliceHeaderBase::PredWeight::read( HeaderSymbolReadIf* pcReadIf )
{
  RNOK(   pcReadIf->getFlag( m_bLumaWeightFlag,     "PWT: luma_weight_flag" ) );
  if( getLumaWeightFlag() )
  {
    RNOK( pcReadIf->getSvlc( m_iLumaWeight,         "PWT: luma_weight" ) );
    RNOK( pcReadIf->getSvlc( m_iLumaOffset,         "PWT: luma_offset" ) );
    ROTR( (-128 > m_iLumaWeight) || (127 < m_iLumaWeight), Err::m_nInvalidParameter );
    ROTR( (-128 > m_iLumaOffset) || (127 < m_iLumaOffset), Err::m_nInvalidParameter );
  }

  RNOK(   pcReadIf->getFlag( m_bChromaWeightFlag,   "PWT: chroma_weight_flag" ) );
  if( getChromaWeightFlag() )
  {
    RNOK( pcReadIf->getSvlc( m_aiChromaWeight[0],   "PWT: cr_weight" ) );
    RNOK( pcReadIf->getSvlc( m_aiChromaOffset[0],   "PWT: cr_offset" ) );
    RNOK( pcReadIf->getSvlc( m_aiChromaWeight[1],   "PWT: cb_weight" ) );
    RNOK( pcReadIf->getSvlc( m_aiChromaOffset[1],   "PWT: cb_offset" ) );
    ROTR( (-128 > m_aiChromaWeight[0]) || (127 < m_aiChromaWeight[0]), Err::m_nInvalidParameter );
    ROTR( (-128 > m_aiChromaOffset[0]) || (127 < m_aiChromaOffset[0]), Err::m_nInvalidParameter );
    ROTR( (-128 > m_aiChromaWeight[1]) || (127 < m_aiChromaWeight[1]), Err::m_nInvalidParameter );
    ROTR( (-128 > m_aiChromaOffset[1]) || (127 < m_aiChromaOffset[1]), Err::m_nInvalidParameter );
  }
  return Err::m_nOK;
}


ErrVal
SliceHeaderBase::PredWeightTable::initDefaults( UInt uiLumaWeightDenom, UInt uiChromaWeightDenom )
{
  const Int iLumaWeight   = 1 << uiLumaWeightDenom;
  const Int iChromaWeight = 1 << uiChromaWeightDenom;

//TMM_WP
  const Int iLumaOffset = 0;
  const Int iChromaOffset = 0;
//TMM_WP

  for( UInt ui = 0; ui < size(); ui++ )
  {
    RNOK( get( ui ).init( iLumaWeight, iChromaWeight, iChromaWeight ) );

//TMM_WP
    RNOK( get( ui ).initOffsets( iLumaOffset, iChromaOffset, iChromaOffset ) );
//TMM_WP
  }
  return Err::m_nOK;
}

ErrVal SliceHeaderBase::PredWeightTable::createRandomParameters()
{
  ROT( 0 == size() );

  for( UInt n = 0; n < size(); n++ )
  {
    RNOK( get(n).createRandomParameters( ) );
  }
  return Err::m_nOK;
}


ErrVal
SliceHeaderBase::PredWeightTable::write( HeaderSymbolWriteIf* pcWriteIf, UInt uiNumber ) const
{
  for( UInt ui = 0; ui < uiNumber; ui++ )
  {
    RNOK( get( ui ).write( pcWriteIf ) );
  }
  return Err::m_nOK;
}

ErrVal SliceHeaderBase::PredWeightTable::read( HeaderSymbolReadIf* pcReadIf, UInt uiNumber )
{
  for( UInt ui = 0; ui < uiNumber; ui++ )
  {
    RNOK( get( ui ).read( pcReadIf ) );
  }
  return Err::m_nOK;
}

ErrVal
SliceHeaderBase::PredWeightTable::copy( const PredWeightTable& rcPredWeightTable )
{
  UInt uiCopySize = min( m_uiBufferSize, rcPredWeightTable.m_uiBufferSize );
  for( UInt ui = 0; ui < uiCopySize; ui++ )
  {
    m_pT[ui].copy( rcPredWeightTable.m_pT[ui] );
  }
  return Err::m_nOK;
}





SliceHeaderBase::SliceHeaderBase( const SequenceParameterSet& rcSPS,
                                  const PictureParameterSet&  rcPPS )
: m_rcPPS                             ( rcPPS )
, m_rcSPS                             ( rcSPS )
, m_eNalRefIdc                        ( NAL_REF_IDC_PRIORITY_LOWEST )
, m_eNalUnitType                      ( NAL_UNIT_EXTERNAL )
, m_uiLayerId                         ( 0 )
, m_uiTemporalLevel                   ( 0 )
, m_uiQualityLevelSH                  ( 0 )
, m_uiFirstMbInSlice                  ( 0 )
, m_eSliceType                        ( B_SLICE )
, m_uiPicParameterSetId               ( rcPPS.getPicParameterSetId() )
, m_uiFrameNum                        ( 0 )
, m_uiNumMbsInSlice                   ( 0 )
, m_uiIdrPicId                        ( 0 )
, m_uiPicOrderCntLsb                  ( 0 )
, m_iDeltaPicOrderCntBottom           ( 0 )
, m_bFieldPicFlag                     ( false )
, m_bBottomFieldFlag                  ( false )
, m_bBasePredWeightTableFlag          ( false )
, m_uiLumaLog2WeightDenom             ( 5 )
, m_uiChromaLog2WeightDenom           ( 5 )
, m_bDirectSpatialMvPredFlag          ( true )
, m_bUseBasePredictionFlag            ( false )
, m_bStoreBaseRepresentationFlag      ( false )
, m_uiBaseLayerId                     ( MSYS_UINT_MAX )
, m_uiBaseQualityLevel                ( 0 )
, m_bAdaptivePredictionFlag           ( false )
, m_bSliceSkipFlag                    ( false )//JVT-W046
//JVT-U160 LMI {
, m_bDefaultBaseModeFlag              ( true )
, m_bAdaptiveMotPredictionFlag        ( true )
, m_bDefaultMotPredictionFlag         ( false )
, m_bAdaptiveResPredictionFlag        ( false )
//JVT-U160 LMI }
, m_bNumRefIdxActiveOverrideFlag      ( false )
, m_bNoOutputOfPriorPicsFlag          ( false ) //EIDR bug-fix
, m_bAdaptiveRefPicBufferingModeFlag  ( false )
, m_bAdaptiveRefPicMarkingModeFlag	  ( false )    //JVT-S036 lsj
, m_uiCabacInitIdc                    ( 0 )
, m_iSliceQpDelta                     ( 0 )
, m_pcFMO                             ( 0 ) //--ICU/ETRI FMO Implementation
, m_bBaseLayerUsesConstrainedIntraPred( false ) 
//, m_uiPrevTl0PicIdx                 ( 0 ) //JVT-W062
//, m_uiNumTl0PicIdxUpdate            ( 0 ) //JVT-W062
//TMM_ESS_UNIFIED {
, m_iScaledBaseLeftOffset             ( 0 ) 
, m_iScaledBaseTopOffset              ( 0 ) 
, m_iScaledBaseRightOffset            ( 0 ) 
, m_iScaledBaseBottomOffset           ( 0 ) 
//TMM_ESS_UNIFIED }
, m_uiBaseChromaPhaseXPlus1           ( 0 ) // TMM_ESS
, m_uiBaseChromaPhaseYPlus1           ( 1 ) // TMM_ESS

, m_bBaseFrameMbsOnlyFlag             ( true  )
, m_bBaseFieldPicFlag                 ( false )
, m_bBaseBotFieldFlag                 ( false )

, m_uiRedundantPicCnt                 ( 0 ) //JVT-Q054 Red. Picture
, m_bOutputFlag                       ( true )//JVT-W047 wxwan
, m_uiSliceGroupChangeCycle           ( 0 ) 
, m_eErrorConceal                     ( EC_NONE ) 
, m_uiMGSCoeffStart                   ( 0 )
, m_uiMGSCoeffStop                    ( 16 )
//JVT-T054{
, m_uiLayerCGSSNR                     ( 0 )
, m_uiQualityLevelCGSSNR              ( 0 )
, m_uiBaseLayerCGSSNR                 ( MSYS_UINT_MAX )
, m_uiBaseQualityLevelCGSSNR          ( 0 )
//JVT-T054}
//JVT-U106 Behaviour at slice boundaries{
, m_bCIUFlag                          ( false )
//JVT-U106 Behaviour at slice boundaries}
, m_bInIDRAccess					            ( false ) //EIDR bug-fix
, m_bAVCRewriteFlag                   ( rcSPS.getAVCRewriteFlag() )  // V-035
, m_iSpatialScalabilityType           (-1)
, m_bRecosntructionLayer              (false)
, m_uiQLDiscardable                   ( MAX_QUALITY_LEVELS ) //DS_FIX_FT_09_2007
{
  ::memset(  m_auiNumRefIdxActive        , 0x00, 2*sizeof(UInt));
  ::memset(  m_aiDeltaPicOrderCnt,         0x00, 2*sizeof(Int) );
  //TMM_INTERLACE 
   for(UInt ui=0;ui<MAX_TEMP_LEVELS;ui++)
     ::memset( m_aauiNumRefIdxActiveUpdate[ui] , 0x00, 2*sizeof(UInt));
    
}


SliceHeaderBase::~SliceHeaderBase()
{
  FMOUninit(); //TMM_INTERLACE 

  //--
  ANOK( m_acPredWeightTable[LIST_0].uninit() );
  ANOK( m_acPredWeightTable[LIST_1].uninit() );
}

SliceHeaderBase& SliceHeaderBase::operator = ( const SliceHeaderBase& rcSHB )
{
  m_eNalRefIdc													= rcSHB.m_eNalRefIdc;
  m_eNalUnitType												= rcSHB.m_eNalUnitType;
  m_uiLayerId														= rcSHB.m_uiLayerId;
  m_uiTemporalLevel											= rcSHB.m_uiTemporalLevel;
  m_uiQualityLevelSH										= rcSHB.m_uiQualityLevelSH;
  m_uiFirstMbInSlice										= rcSHB.m_uiFirstMbInSlice;
  m_eSliceType													= rcSHB.m_eSliceType;
  m_uiPicParameterSetId									= rcSHB.m_uiPicParameterSetId;
  m_uiFrameNum													= rcSHB.m_uiFrameNum;
  m_uiNumMbsInSlice											= rcSHB.m_uiNumMbsInSlice;
  m_uiIdrPicId													= rcSHB.m_uiIdrPicId;
  m_uiPicOrderCntLsb										= rcSHB.m_uiPicOrderCntLsb;
  m_bDirectSpatialMvPredFlag						= rcSHB.m_bDirectSpatialMvPredFlag;

  m_uiBaseLayerId												= rcSHB.m_uiBaseLayerId;
  m_uiBaseQualityLevel									= rcSHB.m_uiBaseQualityLevel;
  m_bAdaptivePredictionFlag							= rcSHB.m_bAdaptivePredictionFlag;
  m_bNumRefIdxActiveOverrideFlag				= false; //rcSHB.m_bNumRefIdxActiveOverrideFlag;
  m_auiNumRefIdxActive[ LIST_0 ]				= rcSHB.m_auiNumRefIdxActive[ LIST_0 ];
  m_auiNumRefIdxActive[ LIST_1 ]				= rcSHB.m_auiNumRefIdxActive[ LIST_1 ];
  m_acRplrBuffer      [ LIST_0 ]        = rcSHB.m_acRplrBuffer      [ LIST_0 ];
  m_acRplrBuffer      [ LIST_1 ]        = rcSHB.m_acRplrBuffer      [ LIST_1 ];
  //m_aauiNumRefIdxActiveUpdate[MAX_TEMP_LEVELS][2];
  m_bNoOutputOfPriorPicsFlag						= rcSHB.m_bNoOutputOfPriorPicsFlag;
  m_bAdaptiveRefPicBufferingModeFlag		= rcSHB.m_bAdaptiveRefPicBufferingModeFlag;
  m_cMmmcoBuffer												= rcSHB.m_cMmmcoBuffer;
  m_uiCabacInitIdc											= rcSHB.m_uiCabacInitIdc;
  m_iSliceQpDelta												= rcSHB.m_iSliceQpDelta;

  m_cDeblockingFilterParameterScalable					= rcSHB.m_cDeblockingFilterParameterScalable;

  m_bBaseLayerUsesConstrainedIntraPred	= rcSHB.m_bBaseLayerUsesConstrainedIntraPred;
  m_uiPriorityId									      = rcSHB.m_uiPriorityId;
  m_bDiscardableFlag										= rcSHB.m_bDiscardableFlag;
  m_bOutputFlag                         = rcSHB.m_bOutputFlag;//JVT-W047
  m_bFieldPicFlag												= rcSHB.m_bFieldPicFlag;
  m_bBottomFieldFlag										= rcSHB.m_bBottomFieldFlag;
  m_iDeltaPicOrderCntBottom							= rcSHB.m_iDeltaPicOrderCntBottom;
  m_aiDeltaPicOrderCnt[0]								= rcSHB.m_aiDeltaPicOrderCnt[0];
  m_aiDeltaPicOrderCnt[1]								= rcSHB.m_aiDeltaPicOrderCnt[1];

  m_uiBaseChromaPhaseXPlus1							= rcSHB.m_uiBaseChromaPhaseXPlus1;
  m_uiBaseChromaPhaseYPlus1							= rcSHB.m_uiBaseChromaPhaseYPlus1;

  m_iScaledBaseLeftOffset								= rcSHB.m_iScaledBaseLeftOffset;
  m_iScaledBaseTopOffset								= rcSHB.m_iScaledBaseTopOffset;
  m_iScaledBaseRightOffset							= rcSHB.m_iScaledBaseRightOffset;
  m_iScaledBaseBottomOffset							= rcSHB.m_iScaledBaseBottomOffset;
  m_bBasePredWeightTableFlag            = rcSHB.m_bBasePredWeightTableFlag;
  m_uiLumaLog2WeightDenom               = rcSHB.m_uiLumaLog2WeightDenom;
  m_uiChromaLog2WeightDenom             = rcSHB.m_uiChromaLog2WeightDenom; 
 
  m_bCIUFlag                            = rcSHB.m_bCIUFlag;
  m_iSpatialScalabilityType             = rcSHB.m_iSpatialScalabilityType;
  m_bRecosntructionLayer                = rcSHB.m_bRecosntructionLayer;

  if(m_pcFMO == NULL)
    m_pcFMO = new FMO();
  else
  {
     FMOUninit(); //TMM_INTERLACE 
    m_pcFMO = new FMO();
  }
  
  *m_pcFMO                               = *(rcSHB.m_pcFMO);
  
  
  return *this;
}

ErrVal
SliceHeaderBase::write( HeaderSymbolWriteIf* pcWriteIf ) const
{
  if( m_eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE ||
      m_eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE     ||
      m_eNalUnitType == NAL_UNIT_PREFIX ) //prefix unit
  {
    HeaderSymbolWriteIf *pcCurrentWriter = pcWriteIf;
    UInt uiSourceLayer = g_nLayer;

    for( UInt uiMGSFragments = 0; true; uiMGSFragments++ )
    {
      ETRACE_DECLARE( Bool m_bTraceEnable = true; );
      RNOK( xWriteScalable( pcCurrentWriter,
                            m_uiQualityLevelCGSSNR + uiMGSFragments,
                            uiMGSFragments ? m_uiLayerCGSSNR                             : m_uiBaseLayerCGSSNR,
                            uiMGSFragments ? m_uiQualityLevelCGSSNR + uiMGSFragments - 1 : m_uiBaseQualityLevelCGSSNR,
                            uiMGSFragments ? false                                       : m_bAdaptivePredictionFlag,
                            uiMGSFragments ? true                                        : m_bDefaultBaseModeFlag,
                            uiMGSFragments ? false                                       : m_bAdaptiveResPredictionFlag,
                            getSPS().getMGSCoeffStart( uiMGSFragments ),
                            getSPS().getMGSCoeffStop ( uiMGSFragments ) ) );

      if( getSPS().getMGSCoeffStop( uiMGSFragments ) == 16 )
      {
        break;
      }

      g_nLayer++;
      ETRACE_LAYER( g_nLayer );
      pcCurrentWriter = pcCurrentWriter->getHeaderSymbolWriteIfNextSlice( true );
    }

    ETRACE_DECLARE( Bool m_bTraceEnable = true; )
    g_nLayer = uiSourceLayer;
    ETRACE_LAYER( g_nLayer );
    return Err::m_nOK;
  }
  else
  {
    return xWriteH264AVCCompatible( pcWriteIf );
  }
}



ErrVal
SliceHeaderBase::xWriteScalable( HeaderSymbolWriteIf* pcWriteIf,
                                 UInt uiQualityLevelCGSSNR,
                                 UInt uiBaseLayerCGSSNR,
                                 UInt uiBaseQualityLevelCGSSNR,
                                 Bool bAdaptivePredictionFlag,
                                 Bool bDefaultBaseModeFlag,
                                 Bool bAdaptiveResPredictionFlag,
                                 UInt uiWriteStart,
                                 UInt uiWriteStop ) const
{
  UInt  uiDependencyId = m_uiLayerCGSSNR;
  UInt  uiQualityLevel = uiQualityLevelCGSSNR;
  Bool bDiscardableFlag = (m_bDiscardableFlag || uiQualityLevel >= m_uiQLDiscardable); //DS_FIX_FT_09_2007
  ROT( uiQualityLevel > 15 );

  Bool  bLayerBaseFlag = ( (m_uiBaseLayerId == MSYS_UINT_MAX) && (uiQualityLevel==0) );

  //===== NAL unit header =====
  RNOK  ( pcWriteIf->writeFlag( 0,                                              "NALU HEADER: forbidden_zero_bit" ) );
  RNOK  ( pcWriteIf->writeCode( m_eNalRefIdc,   2,                              "NALU HEADER: nal_ref_idc" ) );
  RNOK  ( pcWriteIf->writeCode( m_eNalUnitType, 5,                              "NALU HEADER: nal_unit_type" ) );

  if( m_eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE ||
      m_eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE	  ||
      m_eNalUnitType == NAL_UNIT_PREFIX			)//prefix unit
  {
    // byte 0
    RNOK (pcWriteIf->writeFlag( 1,                                              "NALU HEADER SVC: reserved_one_bit" ) );
    RNOK (pcWriteIf->writeFlag( 0 /*dummy*/,                                    "NALU HEADER SVC: idr_flag" ) );
    RNOK (pcWriteIf->writeCode( m_uiPriorityId,           6,                    "NALU HEADER SVC: priority_id"));
    // byte 1
    RNOK (pcWriteIf->writeFlag( bLayerBaseFlag,                                 "NALU HEADER SVC: no_inter_layer_pred_flag" ) );
    RNOK( pcWriteIf->writeCode( uiDependencyId,           3,                    "NALU HEADER SVC: dependency_id" ) );
    RNOK( pcWriteIf->writeCode( uiQualityLevel,           4,                    "NALU HEADER SVC: quality_id" ) );
    // byte 2
    RNOK (pcWriteIf->writeCode( m_uiTemporalLevel,        3,                    "NALU HEADER SVC: temporal_id" ) );
    RNOK (pcWriteIf->writeFlag( m_bUseBasePredictionFlag,                       "NALU HEADER SVC: use_ref_base_pic_flag"));

    RNOK (pcWriteIf->writeFlag( bDiscardableFlag,                             "NALU HEADER SVC: discardable_flag")); //DS_FIX_FT_09_2007

    RNOK (pcWriteIf->writeFlag( m_bOutputFlag,                                  "NALU HEADER SVC: output_flag"));
    RNOK (pcWriteIf->writeCode( 3,                        2,                    "NALU HEADER SVC: reserved_three_2bits" ) );

//     RNOK (pcWriteIf->writeFlag( m_bTl0PicIdxPresentFlag,                        "NALU HEADER: tl0_pic_idx_present_flag"));
//     if ( m_bTl0PicIdxPresentFlag )
//       RNOK( pcWriteIf->writeCode( m_uiTl0PicIdx,    8,                          "SH: tl0_pic_idx" ) );
  }

//JVT-S036 lsj start
  if(m_uiLayerId == 0 )
  {
    if( getNalRefIdc() )
    {
      if(m_bStoreBaseRepresentationFlag|| m_bUseBasePredictionFlag && m_eNalUnitType != NAL_UNIT_CODED_SLICE_IDR_SCALABLE)//JVT-W048
      {
         RNOK(pcWriteIf->writeFlag( m_bAdaptiveRefPicMarkingModeFlag,			"DRPM: adaptive_ref_pic_marking_mode_flag"));
         if(m_bAdaptiveRefPicMarkingModeFlag)
         {
           RNOK( getMmcoBaseBuffer().write( pcWriteIf ) );
         }
      }
    }
  }
  else
  {
//JVT-S036 lsj end
    UInt uiFirstMbInSlice = getFirstMbInSlice();
    if ( !getSPS().getFrameMbsOnlyFlag() && getSPS().getMbAdaptiveFrameFieldFlag() && !getFieldPicFlag())
    {
      uiFirstMbInSlice >>=1;
    }

    RNOK(     pcWriteIf->writeUvlc( uiFirstMbInSlice,                             "SH: first_mb_in_slice" ) );
    RNOK(     pcWriteIf->writeUvlc( m_eSliceType,                                 "SH: slice_type" ) );
    RNOK(     pcWriteIf->writeUvlc( m_uiPicParameterSetId,                        "SH: pic_parameter_set_id" ) );
    RNOK(     pcWriteIf->writeCode( m_uiFrameNum,getSPS().getLog2MaxFrameNum(),   "SH: frame_num" ) );

    if( ! getSPS().getFrameMbsOnlyFlag() )
    {
      RNOK(   pcWriteIf->writeFlag( getFieldPicFlag(),                            "SH: field_pic_flag" ) );
      if( getFieldPicFlag() )
      {
        RNOK( pcWriteIf->writeFlag( getBottomFieldFlag(),                         "SH: bottom_field_flag" ) );
      }
    }

    if( m_eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE )
    {
      RNOK(   pcWriteIf->writeUvlc( m_uiIdrPicId,                                 "SH: idr_pic_id" ) );
    }
    if( getSPS().getPicOrderCntType() == 0 )
    {
      RNOK(     pcWriteIf->writeCode( m_uiPicOrderCntLsb,
                    getSPS().getLog2MaxPicOrderCntLsb(),          "SH: pic_order_cnt_lsb" ) );
      if( getPPS().getPicOrderPresentFlag() && ! m_bFieldPicFlag )
      {
        RNOK( pcWriteIf->writeSvlc( m_iDeltaPicOrderCntBottom,                    "SH: delta_pic_order_cnt_bottom" ) );
      }
    }
    if( getSPS().getPicOrderCntType() == 1 && ! getSPS().getDeltaPicOrderAlwaysZeroFlag() )
    {
      RNOK(   pcWriteIf->writeSvlc( m_aiDeltaPicOrderCnt[0],                      "SH: delta_pic_order_cnt[0]" ) );
      if( getPPS().getPicOrderPresentFlag() && ! m_bFieldPicFlag )
      {
      RNOK( pcWriteIf->writeSvlc( m_aiDeltaPicOrderCnt[1],                      "SH: delta_pic_order_cnt[1]" ) );
      }
    }
    if ( getPPS().getRedundantPicCntPresentFlag() )
    {
      RNOK( pcWriteIf->writeUvlc( m_uiRedundantPicCnt,                          "SH: redundant_pic_cnt") );
    }

#if TO_BE_REMOVED
//    if( uiQualityLevel == 0 || ( uiQualityLevel!=0 && m_eSliceType!=F_SLICE) ) <-- old
    if( true )
#else
    if( uiQualityLevel == 0 )
#endif
    { 	
      if( m_eSliceType == EB_SLICE|| m_eSliceType == (EB_SLICE - 5) )
      {
        RNOK(   pcWriteIf->writeFlag( m_bDirectSpatialMvPredFlag,                   "SH: direct_spatial_mv_pred_flag" ) );
      }   
      //{{JVT-T083
      if( m_eSliceType == EP_SLICE || m_eSliceType == EB_SLICE || m_eSliceType == (EP_SLICE - 5)|| m_eSliceType == (EB_SLICE - 5))
      {
        RNOK( pcWriteIf->writeFlag( m_bNumRefIdxActiveOverrideFlag,               "SH: num_ref_idx_active_override_flag" ) );
        if( m_bNumRefIdxActiveOverrideFlag )
        {
          RNOK( pcWriteIf->writeUvlc( m_auiNumRefIdxActive[LIST_0]-1,             "SH: num_ref_idx_l0_active_minus1" ) );
          if( m_eSliceType == B_SLICE )
          {
            RNOK( pcWriteIf->writeUvlc( m_auiNumRefIdxActive[LIST_1]-1,           "SH: num_ref_idx_l1_active_minus1" ) );
          }
        }
      }
      if( m_eSliceType == EP_SLICE || m_eSliceType == EB_SLICE || m_eSliceType == (EP_SLICE - 5) || m_eSliceType == (EB_SLICE - 5) )
      {
        RNOK( getRplrBuffer( LIST_0 ).write( pcWriteIf ) );
      }
      if( m_eSliceType == EB_SLICE || m_eSliceType == (EB_SLICE - 5))
      {
      RNOK( getRplrBuffer( LIST_1 ).write( pcWriteIf ) );
      }
      if( ( getPPS().getWeightedPredFlag ()      && (( m_eSliceType == EP_SLICE ) || m_eSliceType == (EP_SLICE - 5))) ||
        ( getPPS().getWeightedBiPredIdc() == 1 && (( m_eSliceType == EB_SLICE|| m_eSliceType == (EB_SLICE - 5)) ) ) )
      {
        if( !bLayerBaseFlag ) 
        {
          RNOK( pcWriteIf->writeFlag( m_bBasePredWeightTableFlag,                "PWT: base_pred_weight_table_flag" ) );
        }
        if( bLayerBaseFlag||! m_bBasePredWeightTableFlag )
        {
          RNOK( pcWriteIf->writeUvlc( m_uiLumaLog2WeightDenom,                   "PWT: luma_log_weight_denom" ) );
          RNOK( pcWriteIf->writeUvlc( m_uiChromaLog2WeightDenom,                 "PWT: chroma_log_weight_denom" ) );

          RNOK( m_acPredWeightTable[LIST_0].write( pcWriteIf, getNumRefIdxActive( LIST_0 ) ) );
          if( m_eSliceType == EB_SLICE || m_eSliceType == (EB_SLICE - 5))
          {
            RNOK( m_acPredWeightTable[LIST_1].write( pcWriteIf, getNumRefIdxActive( LIST_1) ) );
          }
        }
      }
      if( getNalRefIdc() )
      {
        if( isIdrNalUnit() )
        {
          RNOK( pcWriteIf->writeFlag( m_bNoOutputOfPriorPicsFlag,                 "DRPM: no_output_of_prior_pics_flag" ) );
          RNOK( pcWriteIf->writeFlag( false,                                      "DRPM: long_term_reference_flag" ) );
        }
        else
        {
          RNOK( pcWriteIf->writeFlag( m_bAdaptiveRefPicBufferingModeFlag,         "DRPM: adaptive_ref_pic_buffering_mode_flag" ) );
          if( m_bAdaptiveRefPicBufferingModeFlag )
          {
            RNOK( getMmcoBuffer().write( pcWriteIf ) );
          }
        }		
        if(!getSPS().getAVCHeaderRewriteFlag())
        {
          RNOK( pcWriteIf->writeFlag( m_bStoreBaseRepresentationFlag,             "SH: store_ref_base_pic_flag" ) );
          if( ( m_bStoreBaseRepresentationFlag||m_bUseBasePredictionFlag||m_bStoreBaseRepresentationFlag ) && m_eNalUnitType != NAL_UNIT_CODED_SLICE_IDR_SCALABLE)//JVT-W048
          {
            RNOK(pcWriteIf->writeFlag( m_bAdaptiveRefPicMarkingModeFlag,	        "DRPM: adaptive_ref_pic_marking_mode_flag"));
            if(m_bAdaptiveRefPicMarkingModeFlag)
            {		
              RNOK( getMmcoBaseBuffer().write( pcWriteIf ) );
            }			  
          }
    //JVT-S036 lsj end
        }
      }
    }
    if( getPPS().getEntropyCodingModeFlag() && m_eSliceType != EI_SLICE && m_eSliceType != (EI_SLICE - 5) )
    {
      RNOK( pcWriteIf->writeUvlc( m_uiCabacInitIdc,                               "SH: cabac_init_idc" ) );
    }
    RNOK( pcWriteIf->writeSvlc( m_iSliceQpDelta,                                  "SH: slice_qp_delta" ) );
    if( getPPS().getDeblockingFilterParametersPresentFlag() )
    {
      RNOK( getDeblockingFilterParameterScalable().getDeblockingFilterParameter().write( pcWriteIf, true, false ) ); //V032, added enhanced layer indicator "true"
    }
    if( uiQualityLevel == 0 )
    {
      if(getPPS().getNumSliceGroupsMinus1()>0 && getPPS().getSliceGroupMapType() >=3 && getPPS().getSliceGroupMapType() <= 5)
      {    
        RNOK( pcWriteIf->writeCode( m_uiSliceGroupChangeCycle,
          getPPS().getLog2MaxSliceGroupChangeCycle(getSPS().getMbInFrame()) ,     "SH: slice_group_change_cycle" ) );
      }
    }
    if ( !bLayerBaseFlag ) 
    {
#if TO_BE_REMOVED
//      if( uiQualityLevel == 0 ||  m_eSliceType != F_SLICE ) <-- old
      if( true )
#else
      if( uiQualityLevel == 0 )
#endif
      {
        UInt  uiBaseLayerId;
        if( uiQualityLevelCGSSNR != 0 || m_uiLayerCGSSNR != m_uiLayerId )
        {
          ROF( uiBaseQualityLevelCGSSNR < 16 );
          uiBaseLayerId = (uiBaseLayerCGSSNR << 4) + uiBaseQualityLevelCGSSNR;
        }
        else
        {
          ROF( m_uiBaseQualityLevel < 16 );
          uiBaseLayerId = (m_uiBaseLayerId << 4) + m_uiBaseQualityLevel;
        }
        RNOK( pcWriteIf->writeUvlc( uiBaseLayerId,                                "SH: ref_layer_dq_id" ) );
        if ( getSPS().getInterlayerDeblockingPresent() )
        {
          RNOK( getDeblockingFilterParameterScalable().getInterlayerDeblockingFilterParameter().write( pcWriteIf, true, true ) ); //V032, added enhanced layer indicator "true"
        }
        //JVT-U106 Behaviour at slice boundaries{
        RNOK (pcWriteIf->writeFlag( m_bCIUFlag,                                   "SH: constrained_intra_upsampling_flag"));
        //JVT-U106 Behaviour at slice boundaries}
        // TMM_ESS {
        if ( getSPS().getExtendedSpatialScalability() == ESS_PICT )
        {
          RNOK( pcWriteIf->writeCode( m_uiBaseChromaPhaseXPlus1, 2,                  "SH: BaseChromaPhaseXPlus1" ) );
          RNOK( pcWriteIf->writeCode( m_uiBaseChromaPhaseYPlus1, 2,                  "SH: BaseChromaPhaseXPlus1" ) );				
          Int iYScale = 2 - getSPS().getFrameMbsOnlyFlag();
          RNOK( pcWriteIf->writeSvlc( m_iScaledBaseLeftOffset,                       "SH: ScaledBaseLeftOffset" ) );
          RNOK( pcWriteIf->writeSvlc( m_iScaledBaseTopOffset/iYScale,                "SH: ScaledBaseTopOffset" ) );
          RNOK( pcWriteIf->writeSvlc( m_iScaledBaseRightOffset,                      "SH: ScaledBaseRightOffset" ) );
          RNOK( pcWriteIf->writeSvlc( m_iScaledBaseBottomOffset/iYScale,             "SH: ScaledBaseBottomOffset" ) );
        }
      }
      else
      {
        ROT( 1 );
      }
    }
    if ( !bLayerBaseFlag )
    {
      RNOK( pcWriteIf->writeFlag( m_bSliceSkipFlag,                             "SH: slice_skip_flag" ) );//JVT-W046
      if( m_bSliceSkipFlag )
      {
        RNOK( pcWriteIf->writeUvlc( m_uiNumMbsInSlice - 1,                          "SH: num_mbs_in_slice_minus1" ) );
      }
      else
      {
        RNOK( pcWriteIf->writeFlag( bAdaptivePredictionFlag,                    "SH: adaptive_prediction_flag" ) );
        // JVT-U160 LMI {
        if ( !bAdaptivePredictionFlag ) 
        {
          RNOK( pcWriteIf->writeFlag( bDefaultBaseModeFlag,                    "SH: default_base_mode_flag" ) );
          if( !bDefaultBaseModeFlag) 
          {
            RNOK( pcWriteIf->writeFlag( m_bAdaptiveMotPredictionFlag,                    "SH: adaptive_motion_prediction_flag" ) );
            if ( !m_bAdaptiveMotPredictionFlag )
            {
              RNOK( pcWriteIf->writeFlag( m_bDefaultMotPredictionFlag,                    "SH: default_motion_prediction_flag" ) );
            }
          }
        }
        RNOK( pcWriteIf->writeFlag( bAdaptiveResPredictionFlag,                    "SH: adaptive_residual_prediction_flag" ) );
        // JVT-U160 LMI }
      }
      if( getSPS().getAVCAdaptiveRewriteFlag() == true )
      {
        RNOK( pcWriteIf->writeFlag( m_bAVCRewriteFlag,                        "SH: tcoeff_level_prediction_flag" ) );	
      }
    }
    if( !getSPS().getAVCHeaderRewriteFlag() ) 
    {
      ROT( uiWriteStart > uiWriteStop );
      ROT( uiWriteStop > 16 );
      if( uiWriteStart == uiWriteStop )
      {
        uiWriteStart = 1;
        uiWriteStop  = 0;
      }
      else
      {
        uiWriteStop--;
      }
      RNOK( pcWriteIf->writeCode( uiWriteStart,      4,                      "SH: scan_idx_start" ) );
      RNOK( pcWriteIf->writeCode( uiWriteStop,       4,                      "SH: scan_idx_end" ) );
    }

  }//JVT-S036 lsj
  return Err::m_nOK;
}


ErrVal
SliceHeaderBase::xWriteH264AVCCompatible( HeaderSymbolWriteIf* pcWriteIf ) const
{
  //===== NAL unit header =====
  RNOK  ( pcWriteIf->writeFlag( 0,                                              "NALU HEADER: forbidden_zero_bit" ) );
  RNOK  ( pcWriteIf->writeCode( m_eNalRefIdc,   2,                              "NALU HEADER: nal_ref_idc" ) );
  RNOK  ( pcWriteIf->writeCode( m_eNalUnitType, 5,                              "NALU HEADER: nal_unit_type" ) );

  //===== slice header =====
  UInt uiFirstMbInSlice = getFirstMbInSlice();
  if ( !getSPS().getFrameMbsOnlyFlag() && getSPS().getMbAdaptiveFrameFieldFlag() && !getFieldPicFlag())
  {
    uiFirstMbInSlice >>=1;
  }
  
  //===== slice header =====
  RNOK(     pcWriteIf->writeUvlc( uiFirstMbInSlice,                           "SH: first_mb_in_slice" ) );
  RNOK(     pcWriteIf->writeUvlc( m_eSliceType,                                 "SH: slice_type" ) );
  RNOK(     pcWriteIf->writeUvlc( m_uiPicParameterSetId,                        "SH: pic_parameter_set_id" ) );
  RNOK(     pcWriteIf->writeCode( m_uiFrameNum, getSPS().getLog2MaxFrameNum(),  "SH: frame_num" ) );

  if( ! getSPS().getFrameMbsOnlyFlag())
  {
    RNOK( pcWriteIf->writeFlag( getFieldPicFlag(),                              "SH: field_pic_flag" ) );
    if( getFieldPicFlag() )
    {
      RNOK( pcWriteIf->writeFlag( getBottomFieldFlag(),                         "SH: bottom_field_flag" ) );
    }
  }

  if( m_eNalUnitType == NAL_UNIT_CODED_SLICE_IDR )
  {
    RNOK(   pcWriteIf->writeUvlc( m_uiIdrPicId,                                 "SH: idr_pic_id" ) );
  }
  
  if( 0 == getSPS().getPicOrderCntType() )
  {
  RNOK(     pcWriteIf->writeCode( m_uiPicOrderCntLsb,
                                  getSPS().getLog2MaxPicOrderCntLsb(),          "SH: pic_order_cnt_lsb" ) );
    if( getPPS().getPicOrderPresentFlag() && ! m_bFieldPicFlag )
    {
      RNOK( pcWriteIf->writeSvlc( m_iDeltaPicOrderCntBottom,                    "SH: delta_pic_order_cnt_bottom" ) );
    }
  }
  if( getSPS().getPicOrderCntType() == 1 && ! getSPS().getDeltaPicOrderAlwaysZeroFlag() )
  {
    RNOK(   pcWriteIf->writeSvlc( m_aiDeltaPicOrderCnt[0],                      "SH: delta_pic_order_cnt[0]" ) );
    if( getPPS().getPicOrderPresentFlag() && ! m_bFieldPicFlag )
    {
      RNOK( pcWriteIf->writeSvlc( m_aiDeltaPicOrderCnt[1],                      "SH: delta_pic_order_cnt[1]" ) );
    }
  }
  //JVT-Q054 Red. Picture {
  if ( getPPS().getRedundantPicCntPresentFlag() )
  {
    RNOK( pcWriteIf->writeUvlc( m_uiRedundantPicCnt,                            "SH: redundant_pic_cnt") );
  }
  //JVT-Q054 Red. Picture }
  
  if( m_eSliceType == B_SLICE )
  {
    RNOK(   pcWriteIf->writeFlag( m_bDirectSpatialMvPredFlag,                   "SH: direct_spatial_mv_pred_flag" ) );
  }

  if( m_eSliceType == P_SLICE || m_eSliceType == B_SLICE )
  {
    RNOK( pcWriteIf->writeFlag( m_bNumRefIdxActiveOverrideFlag,                 "SH: num_ref_idx_active_override_flag" ) );
    if( m_bNumRefIdxActiveOverrideFlag )
    {
      RNOK( pcWriteIf->writeUvlc( m_auiNumRefIdxActive[LIST_0]-1,               "SH: num_ref_idx_l0_active_minus1" ) );
      if( m_eSliceType == B_SLICE )
      {
        RNOK( pcWriteIf->writeUvlc( m_auiNumRefIdxActive[LIST_1]-1,             "SH: num_ref_idx_l1_active_minus1" ) );
      }
    }
  }

  if( m_eSliceType == P_SLICE || m_eSliceType == B_SLICE )
  {
    RNOK( getRplrBuffer( LIST_0 ).write( pcWriteIf ) );
  }
  if( m_eSliceType == B_SLICE )
  {
    RNOK( getRplrBuffer( LIST_1 ).write( pcWriteIf ) );
  }

  if( ( getPPS().getWeightedPredFlag ()      && ( m_eSliceType == P_SLICE ) ) ||
      ( getPPS().getWeightedBiPredIdc() == 1 && ( m_eSliceType == B_SLICE ) ) )
  {
    RNOK( pcWriteIf->writeUvlc( m_uiLumaLog2WeightDenom,                      "PWT: luma_log_weight_denom" ) );
    RNOK( pcWriteIf->writeUvlc( m_uiChromaLog2WeightDenom,                    "PWT: chroma_log_weight_denom" ) );

    RNOK( m_acPredWeightTable[LIST_0].write( pcWriteIf, getNumRefIdxActive( LIST_0 ) ) );
    if( m_eSliceType == B_SLICE )
    {
      RNOK( m_acPredWeightTable[LIST_1].write( pcWriteIf, getNumRefIdxActive( LIST_1) ) );
    }
  }

  if( getNalRefIdc() )
  {
    if( isIdrNalUnit() )
    {
      RNOK( pcWriteIf->writeFlag( m_bNoOutputOfPriorPicsFlag,                   "DRPM: no_output_of_prior_pics_flag" ) );
      RNOK( pcWriteIf->writeFlag( false,                                        "DRPM: long_term_reference_flag" ) );
    }
    else
    {
      RNOK( pcWriteIf->writeFlag( m_bAdaptiveRefPicBufferingModeFlag,           "DRPM: adaptive_ref_pic_buffering_mode_flag" ) );
      if( m_bAdaptiveRefPicBufferingModeFlag )
      {
        RNOK( getMmcoBuffer().write( pcWriteIf ) );
      }
    }
  }

  if( getPPS().getEntropyCodingModeFlag() && m_eSliceType != I_SLICE )
  {
    RNOK( pcWriteIf->writeUvlc( m_uiCabacInitIdc,                               "SH: cabac_init_idc" ) );
  }


  RNOK( pcWriteIf->writeSvlc( m_iSliceQpDelta,                                  "SH: slice_qp_delta" ) );
  
  if( getPPS().getDeblockingFilterParametersPresentFlag() )
  {
      RNOK( getDeblockingFilterParameterScalable().getDeblockingFilterParameter().write( pcWriteIf, false,false ) ); //XXXV032, added enhanced layer indicator "true"
  }

  if(getPPS().getNumSliceGroupsMinus1()>0 && getPPS().getSliceGroupMapType() >=3 && getPPS().getSliceGroupMapType() <= 5)
  {    
    RNOK(     pcWriteIf->writeCode( m_uiSliceGroupChangeCycle, getPPS().getLog2MaxSliceGroupChangeCycle(getSPS().getMbInFrame()) ,                "SH: slice_group_change_cycle" ) );
  }

  return Err::m_nOK;
}




ErrVal
SliceHeaderBase::read( HeaderSymbolReadIf* pcReadIf )
{
  if( m_eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE || 
      m_eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE       )
  {
      m_pcReadIf = pcReadIf; // TMM_INTERLACE

    return xReadScalable          ( pcReadIf );
  }
  else
  {
    return xReadH264AVCCompatible ( pcReadIf );
  }
}

ErrVal
SliceHeaderBase::xReadScalable( HeaderSymbolReadIf* pcReadIf )
{
  Bool  bTmp;
  //JVT-W046 {
  UInt  uiQualityLevel = getQualityLevel();
  RNOK(     pcReadIf->getCode( m_uiFrameNum,
                               getSPS().getLog2MaxFrameNum(),                "SH: frame_num" ) );

  if( ! getSPS().getFrameMbsOnlyFlag() )
  {
    RNOKS(  pcReadIf->getFlag( m_bFieldPicFlag,                              "SH: field_pic_flag" ) );
    if( m_bFieldPicFlag )
    {
      RNOKS(pcReadIf->getFlag( m_bBottomFieldFlag,                           "SH: bottom_field_flag" ) );
    }
  }

  if( m_eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE )
  {
    RNOK(   pcReadIf->getUvlc( m_uiIdrPicId,                                 "SH: idr_pic_id" ) );
  }
  
  if( 0 == getSPS().getPicOrderCntType() )
  {
    RNOK(     pcReadIf->getCode( m_uiPicOrderCntLsb,
                               getSPS().getLog2MaxPicOrderCntLsb(),          "SH: pic_order_cnt_lsb" ) );
    if( getPPS().getPicOrderPresentFlag() && ! m_bFieldPicFlag )
    {
      RNOK( pcReadIf->getSvlc( m_iDeltaPicOrderCntBottom,                    "SH: delta_pic_order_cnt_bottom" ) );
    }
  }

  if( (1 == getSPS().getPicOrderCntType()) && ( ! getSPS().getDeltaPicOrderAlwaysZeroFlag()))
  {
    RNOKS( pcReadIf->getSvlc( m_aiDeltaPicOrderCnt[LIST_0],                 "SH: delta_pic_order_cnt[0]" ) );
    if (getPPS().getPicOrderPresentFlag() && (!m_bFieldPicFlag))
    {
      RNOK( pcReadIf->getSvlc( m_aiDeltaPicOrderCnt[1],                      "SH: delta_pic_order_cnt[1]" ) );
    }
  }

  if( getPPS().getRedundantPicCntPresentFlag() )
  {
    RNOK( pcReadIf->getUvlc( m_uiRedundantPicCnt,                            "SH: redundant_pic_cnt") );
  }

#if TO_BE_REMOVED
//  if( uiQualityLevel == 0 || ( uiQualityLevel!=0 && m_eSliceType!=F_SLICE) )   <-- old
  if( true )
#else
  if( uiQualityLevel == 0 )
#endif
  {
    if( m_eSliceType == EB_SLICE || m_eSliceType == (EB_SLICE - 5))
    {
      RNOK( pcReadIf->getFlag( m_bDirectSpatialMvPredFlag,                   "SH: direct_spatial_mv_pred_flag" ) );
    }
    if( m_eSliceType == EP_SLICE || m_eSliceType == EB_SLICE || m_eSliceType == (EP_SLICE - 5)|| m_eSliceType == (EB_SLICE - 5))
    {
      RNOK( pcReadIf->getFlag( m_bNumRefIdxActiveOverrideFlag,               "SH: num_ref_idx_active_override_flag" ) );
      if( m_bNumRefIdxActiveOverrideFlag )
      {
        RNOK( pcReadIf->getUvlc( m_auiNumRefIdxActive[LIST_0],               "SH: num_ref_idx_l0_active_minus1" ) );
        m_auiNumRefIdxActive[LIST_0]++;
        if( m_eSliceType == EB_SLICE || m_eSliceType == (EB_SLICE - 5))
        {
          RNOK( pcReadIf->getUvlc( m_auiNumRefIdxActive[LIST_1],             "SH: num_ref_idx_l1_active_minus1" ) );
          m_auiNumRefIdxActive[LIST_1]++;
        }
      }
    }
    if( m_eSliceType != EB_SLICE && m_eSliceType != (EB_SLICE - 5))
    {
      m_auiNumRefIdxActive[LIST_1] = 0;
    }

    if( m_eSliceType == EP_SLICE || m_eSliceType == EB_SLICE || m_eSliceType == (EP_SLICE - 5)|| m_eSliceType == (EB_SLICE - 5))
    {
      RNOK( getRplrBuffer( LIST_0 ).read( pcReadIf, getNumRefIdxActive( LIST_0 ) ) );
    }
    if( m_eSliceType == EB_SLICE || m_eSliceType == (EB_SLICE - 5))
    {
      RNOK( getRplrBuffer( LIST_1 ).read( pcReadIf, getNumRefIdxActive( LIST_1 ) ) );
    }
    RNOK( m_acPredWeightTable[LIST_0].init( 64 ) );
    RNOK( m_acPredWeightTable[LIST_1].init( 64 ) );
    if( ( getPPS().getWeightedPredFlag ()      && ( m_eSliceType == EP_SLICE || m_eSliceType == (EP_SLICE - 5)) ) ||
        ( getPPS().getWeightedBiPredIdc() == 1 && ( m_eSliceType == EB_SLICE || m_eSliceType == (EB_SLICE - 5)) ) )
    {
      if( !m_bLayerBaseFlag )
      {
        RNOK( pcReadIf->getFlag( m_bBasePredWeightTableFlag,                "PWT: base_pred_weight_table_flag" ) );
      }

      if( m_bLayerBaseFlag || ! m_bBasePredWeightTableFlag )
      {
        RNOK( pcReadIf->getUvlc( m_uiLumaLog2WeightDenom,                   "PWT: luma_log_weight_denom" ) );
        RNOK( pcReadIf->getUvlc( m_uiChromaLog2WeightDenom,                 "PWT: chroma_log_weight_denom" ) );
        ROTR( m_uiLumaLog2WeightDenom   > 7, Err::m_nInvalidParameter );
        ROTR( m_uiChromaLog2WeightDenom > 7, Err::m_nInvalidParameter );

        RNOK( m_acPredWeightTable[LIST_0].initDefaults( m_uiLumaLog2WeightDenom, m_uiChromaLog2WeightDenom ) );
        RNOK( m_acPredWeightTable[LIST_0].read( pcReadIf, getNumRefIdxActive( LIST_0 ) ) );
        if( m_eSliceType == EB_SLICE || m_eSliceType == (EB_SLICE - 5))
        {
          RNOK( m_acPredWeightTable[LIST_1].initDefaults( m_uiLumaLog2WeightDenom, m_uiChromaLog2WeightDenom ) );
          RNOK( m_acPredWeightTable[LIST_1].read( pcReadIf, getNumRefIdxActive( LIST_1) ) );
        }
      }
    }
    if( getNalRefIdc() )
    {
      if( isIdrNalUnit() )
      {
        RNOK( pcReadIf->getFlag( m_bNoOutputOfPriorPicsFlag,                 "DRPM: no_output_of_prior_pics_flag" ) );
        RNOK( pcReadIf->getFlag( bTmp,                                       "DRPM: long_term_reference_flag" ) );
        ROT ( bTmp );
      }
      else
      {
        RNOK( pcReadIf->getFlag( m_bAdaptiveRefPicBufferingModeFlag,         "DRPM: adaptive_ref_pic_buffering_mode_flag" ) );
        if( m_bAdaptiveRefPicBufferingModeFlag )
        {
          RNOK( getMmcoBuffer().read( pcReadIf ) );
        }
      }
      if(!getSPS().getAVCHeaderRewriteFlag())
      {
        RNOK( pcReadIf->getFlag( m_bStoreBaseRepresentationFlag,              "SH: store_ref_base_pic_flag" ) );

        if ( (m_bUseBasePredictionFlag||m_bStoreBaseRepresentationFlag) &&  getNalUnitType() != NAL_UNIT_CODED_SLICE_IDR_SCALABLE ) //HC
        {
          RNOK(pcReadIf->getFlag( m_bAdaptiveRefPicMarkingModeFlag,		        "DRPM: adaptive_ref_base_pic_marking_mode_flag"));
          if(m_bAdaptiveRefPicMarkingModeFlag)
          {
            RNOK( getMmcoBaseBuffer().read( pcReadIf ) );
          }		  
        }
        //JVT-S036 lsj end
      }
    }
  }
  else
  {
    ROT( 1 ); //Remove
  }
 
  if( getPPS().getEntropyCodingModeFlag() && m_eSliceType != EI_SLICE && m_eSliceType != (EI_SLICE-5))
  {
    RNOK( pcReadIf->getUvlc( m_uiCabacInitIdc,                             "SH: cabac_init_idc" ) );
  }
  RNOK( pcReadIf->getSvlc( m_iSliceQpDelta,                                  "SH: slice_qp_delta" ) );
  if( getPPS().getDeblockingFilterParametersPresentFlag() )
  {
    RNOK( getDeblockingFilterParameterScalable().getDeblockingFilterParameter().read( pcReadIf, true, false ) ); //V032, added enhanced layer indicator "true"
  }
  if( uiQualityLevel == 0 )
  {
    UInt uiSliceGroupChangeCycle;
    if( getPPS().getNumSliceGroupsMinus1()> 0  && getPPS().getSliceGroupMapType() >= 3  &&  getPPS().getSliceGroupMapType() <= 5)
    {
      UInt pictureSizeInMB = getSPS().getFrameHeightInMbs()*getSPS().getFrameWidthInMbs();
      RNOK( pcReadIf->getCode( uiSliceGroupChangeCycle, getLog2MaxSliceGroupChangeCycle(pictureSizeInMB), "SH: slice_group_change_cycle" ) ); 
      setSliceGroupChangeCycle(uiSliceGroupChangeCycle);
    }
  }

  if( !m_bLayerBaseFlag )
  {
#if TO_BE_REMOVED
//    if( uiQualityLevel == 0 || m_eSliceType != F_SLICE ) // old
    if( true )
#else
    if( uiQualityLevel == 0 )
#endif
    {
      RNOK( pcReadIf->getUvlc( m_uiBaseLayerId,                            "SH: ref_layer_dq_id" ) );

      m_uiBaseQualityLevel = m_uiBaseLayerId & 15;
      m_uiBaseLayerId      = m_uiBaseLayerId >> 4;

      if ( getSPS().getInterlayerDeblockingPresent() )
      {
        RNOK( getDeblockingFilterParameterScalable().getDeblockingFilterParameter().read( pcReadIf, true ,true) ); //V032, added enhanced layer indicator "true"
      }
      //JVT-U106 Behaviour at slice boundaries{
      RNOK (pcReadIf->getFlag( m_bCIUFlag, "SH: constrained_intra_upsampling_flag"));
      //JVT-U106 Behaviour at slice boundaries}      
      if (getSPS().getExtendedSpatialScalability() == ESS_PICT)
      { 
        RNOK( pcReadIf->getCode( m_uiBaseChromaPhaseXPlus1, 2,                 "SH: BaseChromaPhaseXPlus1" ) );
        RNOK( pcReadIf->getCode( m_uiBaseChromaPhaseYPlus1, 2,                 "SH: BaseChromaPhaseYPlus1" ) );
        Int iYScale = 2 - getSPS().getFrameMbsOnlyFlag();
        RNOK( pcReadIf->getSvlc( m_iScaledBaseLeftOffset,                          "SH: ScaledBaseLeftOffset" ) );
        RNOK( pcReadIf->getSvlc( m_iScaledBaseTopOffset,                           "SH: ScaledBaseTopOffset" ) );
        RNOK( pcReadIf->getSvlc( m_iScaledBaseRightOffset,                         "SH: ScaledBaseRightOffset" ) );
        RNOK( pcReadIf->getSvlc( m_iScaledBaseBottomOffset,                        "SH: ScaledBaseBottomOffset" ) );
        m_iScaledBaseTopOffset    *= iYScale;
        m_iScaledBaseBottomOffset *= iYScale;
      }
      //JVT-W046 {
      else if( getSPS().getExtendedSpatialScalability() > 0 )
      {
        m_uiBaseChromaPhaseXPlus1 = getSPS().getBaseChromaPhaseXPlus1();
        m_uiBaseChromaPhaseYPlus1 = getSPS().getBaseChromaPhaseYPlus1();
      }
      //JVT-W046 }
    }
  }
  if ( !m_bLayerBaseFlag )
  {
    RNOK( m_pcReadIf->getFlag( m_bSliceSkipFlag,                      "SH: slice_skip_flag" ) );
    if( m_bSliceSkipFlag )
    {
      RNOK( pcReadIf    ->getUvlc( m_uiNumMbsInSlice,                           "SH: num_mbs_in_slice_minus1" ) );
      m_uiNumMbsInSlice++;
    }
    else
    {
      RNOK( pcReadIf->getFlag( m_bAdaptivePredictionFlag,                    "SH: adaptive_prediction_flag" ) );
      // JVT-U160 LMI {
      if ( !m_bAdaptivePredictionFlag ) 
      {
        RNOK( pcReadIf->getFlag( m_bDefaultBaseModeFlag,                    "SH: default_base_mode_flag" ) );
        if( !m_bDefaultBaseModeFlag ) 
        {
          RNOK( pcReadIf->getFlag( m_bAdaptiveMotPredictionFlag,                    "SH: adaptive_motion_prediction_flag" ) );
          if( !m_bAdaptiveMotPredictionFlag )
          {
            RNOK( pcReadIf->getFlag( m_bDefaultMotPredictionFlag,                    "SH: default_motion_prediction_flag" ) );
          }
        }
      }
      RNOK( pcReadIf->getFlag( m_bAdaptiveResPredictionFlag,                    "SH: adaptive_residual_prediction_flag" ) );
    }
    if( getSPS().getAVCAdaptiveRewriteFlag() == true )
    {
      RNOK( pcReadIf->getFlag( m_bAVCRewriteFlag,                            "SH: tcoeff_level_prediction_flag" ) );
    }
  }
  else
  {
    m_uiBaseQualityLevel  = 0;
    m_uiBaseLayerId       = MSYS_UINT_MAX;
    m_bAdaptivePredictionFlag = 0; 
  }
  if( !getSPS().getAVCHeaderRewriteFlag() ) 
  {
    RNOK( pcReadIf->getCode( m_uiMGSCoeffStart,  4,                      "SH: scan_idx_start" ) );
    RNOK( pcReadIf->getCode( m_uiMGSCoeffStop,   4,                      "SH: scan_idx_end" ) );

    // interpret read values for coeff start and stop
    if( m_uiMGSCoeffStart > m_uiMGSCoeffStop )
    {
      m_uiMGSCoeffStart = 0;
      m_uiMGSCoeffStop  = 0;
    }
    else
    {
      m_uiMGSCoeffStop += 1;
    }
  }
  
  return Err::m_nOK;
}



ErrVal
SliceHeaderBase::xReadH264AVCCompatible( HeaderSymbolReadIf* pcReadIf )
{
  const Bool bL0InfoPresent = ! ( m_eSliceType == I_SLICE );
  const Bool bL1InfoPresent =  m_eSliceType == B_SLICE;
  Bool  bTmp;
 
  RNOK(     pcReadIf->getCode( m_uiFrameNum,
                               getSPS().getLog2MaxFrameNum(),                "SH: frame_num" ) );
  ROT( isIdrNalUnit() && (0 != m_uiFrameNum) );

  if( ! getSPS().getFrameMbsOnlyFlag() )
  {
    RNOKS(   pcReadIf->getFlag( m_bFieldPicFlag,                             "SH: field_pic_flag" ) );
    if( m_bFieldPicFlag )
    {
      RNOKS( pcReadIf->getFlag( m_bBottomFieldFlag,                          "SH: bottom_field_flag" ) );
    }
  }

  if( m_eNalUnitType == NAL_UNIT_CODED_SLICE_IDR )
  {
    RNOK(   pcReadIf->getUvlc( m_uiIdrPicId,                                 "SH: idr_pic_id" ) );
  }
  
  if( 0 == getSPS().getPicOrderCntType() )
  {
  RNOK(     pcReadIf->getCode( m_uiPicOrderCntLsb,
                               getSPS().getLog2MaxPicOrderCntLsb(),          "SH: pic_order_cnt_lsb" ) );
    if( getPPS().getPicOrderPresentFlag() && ! m_bFieldPicFlag )
    {
      RNOK( pcReadIf->getSvlc( m_iDeltaPicOrderCntBottom,                    "SH: delta_pic_order_cnt_bottom" ) );
    }
  }
  if( getSPS().getPicOrderCntType() == 1 && ! getSPS().getDeltaPicOrderAlwaysZeroFlag() )
  {
    RNOK(   pcReadIf->getSvlc( m_aiDeltaPicOrderCnt[LIST_0],                      "SH: delta_pic_order_cnt[0]" ) );
    if( getPPS().getPicOrderPresentFlag() && ! m_bFieldPicFlag )
  {
      RNOK( pcReadIf->getSvlc( m_aiDeltaPicOrderCnt[LIST_1],                      "SH: delta_pic_order_cnt[1]" ) );
    }
  }
  //JVT-Q054 Red. Picture {
  if ( getPPS().getRedundantPicCntPresentFlag())
  {
    RNOK( pcReadIf->getUvlc( m_uiRedundantPicCnt,                            "SH: redundant_pic_cnt") );
  }
  //JVT-Q054 Red. Picture }
  
  if( bL1InfoPresent )
  {
    RNOK(   pcReadIf->getFlag( m_bDirectSpatialMvPredFlag,                   "SH: direct_spatial_mv_pred_flag" ) );
  }

  if( bL0InfoPresent )
  {
    RNOK( pcReadIf->getFlag( m_bNumRefIdxActiveOverrideFlag,                 "SH: num_ref_idx_active_override_flag" ) );
    if( m_bNumRefIdxActiveOverrideFlag )
    {
      RNOK( pcReadIf->getUvlc( m_auiNumRefIdxActive[LIST_0],                 "SH: num_ref_idx_l0_active_minus1" ) );
      m_auiNumRefIdxActive[LIST_0]++;
      if( bL1InfoPresent )
      {
        RNOK( pcReadIf->getUvlc( m_auiNumRefIdxActive[LIST_1],               "SH: num_ref_idx_l1_active_minus1" ) );
        m_auiNumRefIdxActive[LIST_1]++;
      }
    }
  }

  if( bL0InfoPresent )
  {
    RNOK( getRplrBuffer( LIST_0 ).read( pcReadIf, getNumRefIdxActive( LIST_0 ) ) );
  }

  if( bL1InfoPresent )
  {
    RNOK( getRplrBuffer( LIST_1 ).read( pcReadIf, getNumRefIdxActive( LIST_1 ) ) );
  }

  RNOK( m_acPredWeightTable[LIST_0].init( 64 ) );
  RNOK( m_acPredWeightTable[LIST_1].init( 64 ) );

  if( ( getPPS().getWeightedPredFlag ()      && ( m_eSliceType == P_SLICE ) ) ||
      ( getPPS().getWeightedBiPredIdc() == 1 && ( m_eSliceType == B_SLICE ) ) )
  {
    RNOK( pcReadIf->getUvlc( m_uiLumaLog2WeightDenom,                     "PWT: luma_log_weight_denom" ) );
    RNOK( pcReadIf->getUvlc( m_uiChromaLog2WeightDenom,                   "PWT: chroma_log_weight_denom" ) );
    ROTR( m_uiLumaLog2WeightDenom   > 7, Err::m_nInvalidParameter );
    ROTR( m_uiChromaLog2WeightDenom > 7, Err::m_nInvalidParameter );

    RNOK( m_acPredWeightTable[LIST_0].initDefaults( m_uiLumaLog2WeightDenom, m_uiChromaLog2WeightDenom ) );
    RNOK( m_acPredWeightTable[LIST_0].read( pcReadIf, getNumRefIdxActive( LIST_0 ) ) );
    if( m_eSliceType == B_SLICE )
    {
      RNOK( m_acPredWeightTable[LIST_1].initDefaults( m_uiLumaLog2WeightDenom, m_uiChromaLog2WeightDenom ) );
      RNOK( m_acPredWeightTable[LIST_1].read( pcReadIf, getNumRefIdxActive( LIST_1) ) );
    }
  }

  if( getNalRefIdc() )
  {
    if( isIdrNalUnit() )
    {
      RNOK( pcReadIf->getFlag( m_bNoOutputOfPriorPicsFlag,                   "DRPM: no_output_of_prior_pics_flag" ) );
      RNOK( pcReadIf->getFlag( bTmp,                                         "DRPM: long_term_reference_flag" ) );
      ROT ( bTmp );
    }
    else
    {
      RNOK( pcReadIf->getFlag( m_bAdaptiveRefPicBufferingModeFlag,           "DRPM: adaptive_ref_pic_buffering_mode_flag" ) );
      if( m_bAdaptiveRefPicBufferingModeFlag )
      {
        RNOK( getMmcoBuffer().read( pcReadIf ) );
      }
    }
  }

  if( getPPS().getEntropyCodingModeFlag() && bL0InfoPresent )
  {
    RNOK( pcReadIf->getUvlc( m_uiCabacInitIdc,                               "SH: cabac_init_idc" ) );
  }


  RNOK( pcReadIf->getSvlc( m_iSliceQpDelta,                                  "SH: slice_qp_delta" ) );
  
  if( getPPS().getDeblockingFilterParametersPresentFlag() )
  {
       RNOK( getDeblockingFilterParameterScalable().getDeblockingFilterParameter().read( pcReadIf, false, false) ); //V032, added enhanced layer indicator "false"

  }

  //--ICU/ETRI FMO Implementation
  UInt uiSliceGroupChangeCycle;
  if( getPPS().getNumSliceGroupsMinus1()> 0  && getPPS().getSliceGroupMapType() >= 3  &&  getPPS().getSliceGroupMapType() <= 5)
  {
    UInt pictureSizeInMB = getSPS().getFrameHeightInMbs()*getSPS().getFrameWidthInMbs();

    RNOK(     pcReadIf->getCode( uiSliceGroupChangeCycle, getLog2MaxSliceGroupChangeCycle(pictureSizeInMB), "SH: slice_group_change_cycle" ) );
 
     
    setSliceGroupChangeCycle(uiSliceGroupChangeCycle);
  }

  return Err::m_nOK;
}



ErrVal
SliceHeaderBase::DeblockingFilterParameter::write( HeaderSymbolWriteIf* pcWriteIf, bool enhancedLayerFlag, bool interlayer ) const //V032, added enhanced layer indicator

{ //JVT-W046
  //RNOK( pcWriteIf->writeUvlc( getDisableDeblockingFilterIdc(),  "SH: disable_deblocking_filter_idc" ) );

  //======The following code were added for V032=============
  //=======Start adding code for V032
  if (!interlayer)
  { //for enhanced layer
    if(enhancedLayerFlag) 
      pcWriteIf->writeUvlc( getDisableDeblockingFilterIdc(),  "SH: disable_deblocking_filter_idc" ) ;
      else  //for base layer
      {
        if (3 == getDisableDeblockingFilterIdc())  //Disable chroma deblocking W.R.T. idc of 0 in base layer
        pcWriteIf->writeUvlc( 0,  "SH: disable_deblocking_filter_idc" );
        else if (4 == getDisableDeblockingFilterIdc()) //Disable chroma deblocking W.R.T. idc of 2 in base layer
        pcWriteIf->writeUvlc( 2,  "SH: disable_deblocking_filter_idc" );
      else  //normal values of idc, 0, 1, and 2
        pcWriteIf->writeUvlc( getDisableDeblockingFilterIdc(),  "SH: disable_deblocking_filter_idc" );
    }
  }
  else
  {
    if(enhancedLayerFlag)//for enhanced layer 
      pcWriteIf->writeUvlc( getDisableInterlayerDeblockingFilterIdc(),  "SH: disable_interlayer_deblocking_filter_idc" ) ;
      else  //for base layer
      {
        if (3 == getDisableInterlayerDeblockingFilterIdc())  //Disable chroma deblocking W.R.T. idc of 0 in base layer
        pcWriteIf->writeUvlc( 0,  "SH: disable_interlayer_deblocking_filter_idc" );
        else if (4 == getDisableInterlayerDeblockingFilterIdc()) //Disable chroma deblocking W.R.T. idc of 2 in base layer
        pcWriteIf->writeUvlc( 2,  "SH: disable_interlayer_deblocking_filter_idc" );
      else  //normal values of idc, 0, 1, and 2
        pcWriteIf->writeUvlc( getDisableInterlayerDeblockingFilterIdc(),  "SH: disable_interlayer_deblocking_filter_idc" );
    }
  }
  //=======End adding code for V032=========================
  if(!interlayer)
  {
      ROTRS( 1 == getDisableDeblockingFilterIdc(), Err::m_nOK );
      RNOK( pcWriteIf->writeSvlc( getSliceAlphaC0Offset() >> 1,     "SH: slice_alpha_c0_offset_div2" ) );
    RNOK( pcWriteIf->writeSvlc( getSliceBetaOffset() >> 1,        "SH: slice_beta_offset_div2" ) );
  }
  else
  {
    ROTRS( 1 == getDisableInterlayerDeblockingFilterIdc(), Err::m_nOK );
      RNOK( pcWriteIf->writeSvlc( getInterlayerSliceAlphaC0Offset() >> 1,     "SH: interlayer_slice_alpha_c0_offset_div2" ) );
    RNOK( pcWriteIf->writeSvlc( getInterlayerSliceBetaOffset() >> 1,        "SH: interlayer_slice_beta_offset_div2" ) );
  }
  //JVT-W046
  return Err::m_nOK;
}



ErrVal
SliceHeaderBase::DeblockingFilterParameter::read( HeaderSymbolReadIf* pcReadIf, bool enhancedLayerFlag, bool interlayer )  //V032, added enhanced layer indicator "false"
{
  //JVT-W046
  if (!interlayer)
  {
    RNOK( pcReadIf->getUvlc( m_uiDisableDeblockingFilterIdc,      "SH: disable_deblocking_filter_idc" ) );
    //ROT ( m_uiDisableDeblockingFilterIdc > 2 );
    ROT ( m_uiDisableDeblockingFilterIdc > 4 );      //V032 for extending Idc value to 3 and 4 w.r.t. 0 and 2 respectively
    ROT (!enhancedLayerFlag && (m_uiDisableDeblockingFilterIdc > 2) ); //V032, Idc should not be >2 for base layer
  }
  else
  {
    RNOK( pcReadIf->getUvlc( m_uiDisableInterlayerDeblockingFilterIdc,      "SH: disable_interlayer_deblocking_filter_idc" ) );
    //ROT ( m_uiDisableDeblockingFilterIdc > 2 );
    ROT ( m_uiDisableInterlayerDeblockingFilterIdc > 4 );      //V032 for extending Idc value to 3 and 4 w.r.t. 0 and 2 respectively
    ROT (!enhancedLayerFlag && (m_uiDisableInterlayerDeblockingFilterIdc > 2) ); //V032, Idc should not be >2 for base layer
  }
  if (!interlayer)
  {
    ROTRS( 1 == getDisableDeblockingFilterIdc(), Err::m_nOK );
 
    Int iTmp;
    RNOK( pcReadIf->getSvlc( iTmp,                                "SH: slice_alpha_c0_offset_div2" ) );
    ROT( (iTmp < -6) || (iTmp >  6) );
    setSliceAlphaC0Offset( iTmp << 1);

    RNOK( pcReadIf->getSvlc( iTmp,                                "SH: slice_beta_offset_div2" ) );
    ROT( (iTmp < -6) || (iTmp >  6) );
  setSliceBetaOffset( iTmp << 1 );
  }
  else
  {
    ROTRS( 1 == getDisableInterlayerDeblockingFilterIdc(), Err::m_nOK );
 
    Int iTmp;
    RNOK( pcReadIf->getSvlc( iTmp,                                "SH: interlayer_slice_alpha_c0_offset_div2" ) );
    ROT( (iTmp < -6) || (iTmp >  6) );
    setInterlayerSliceAlphaC0Offset( iTmp << 1);

    RNOK( pcReadIf->getSvlc( iTmp,                                "SH: interlayer_slice_beta_offset_div2" ) );
    ROT( (iTmp < -6) || (iTmp >  6) );
  setInterlayerSliceBetaOffset( iTmp << 1 );
  }
  //JVT-W046
  return Err::m_nOK;
}


//--ICU/ETRI FMO Implementation
ErrVal 
SliceHeaderBase::FMOInit()
{
    
  if(m_pcFMO == NULL)
    m_pcFMO = new FMO();
  else
  {
    //TMM_INTERLACE 
    FMOUninit(); 
    //--
    m_pcFMO = new FMO();
  }

  const SequenceParameterSet* pcSPS = &(getSPS());
  const PictureParameterSet* pcPPS = &(getPPS());

//TMM_INTERLACE {
  m_pcFMO->sps_.frame_mbs_only_flag            = pcSPS->getFrameMbsOnlyFlag(); 
  m_pcFMO->sps_.mb_adaptive_frame_field_flag   = pcSPS->getMbAdaptiveFrameFieldFlag(); 
  m_pcFMO->sps_.pic_height_in_map_units_minus1 = (pcSPS->getFrameMbsOnlyFlag()?pcSPS->getFrameHeightInMbs(): pcSPS->getFrameHeightInMbs()>>1)  -1;
  m_pcFMO->sps_.pic_width_in_mbs_minus1        =  pcSPS->getFrameWidthInMbs()-1;

  m_pcFMO->pps_.num_slice_groups_minus1 = pcPPS->getNumSliceGroupsMinus1();
  m_pcFMO->pps_.slice_group_map_type    = pcPPS->getSliceGroupMapType();
  m_pcFMO->pps_.num_slice_group_map_units_minus1 = pcPPS->getNumSliceGroupMapUnitsMinus1();	  
  m_pcFMO->pps_.copy_run_length_minus1(pcPPS->getArrayRunLengthMinus1());
  m_pcFMO->pps_.copy_top_left(pcPPS->getArrayTopLeft());
  m_pcFMO->pps_.copy_bottom_right(pcPPS->getArrayBottomRight());
  m_pcFMO->pps_.slice_group_change_direction_flag = pcPPS->getSliceGroupChangeDirection_flag();
  m_pcFMO->pps_.slice_group_change_rate_minus1    = pcPPS->getSliceGroupChangeRateMinus1();
  m_pcFMO->pps_.copy_slice_group_id(pcPPS->getArraySliceGroupId());

  m_pcFMO->img_.field_pic_flag      = getFieldPicFlag();
  m_pcFMO->img_.PicHeightInMapUnits = m_pcFMO->sps_.pic_height_in_map_units_minus1 + 1; 
  m_pcFMO->img_.PicWidthInMbs       = pcSPS->getFrameWidthInMbs();
  m_pcFMO->img_.PicSizeInMbs        = (pcSPS->getFrameHeightInMbs()>> (UChar)getFieldPicFlag())*pcSPS->getFrameWidthInMbs();
  m_pcFMO->img_.slice_group_change_cycle = getSliceGroupChangeCycle();
 //TMM_INTERLACE }

  m_pcFMO->init(&(m_pcFMO->pps_),&(m_pcFMO->sps_));

  m_pcFMO->StartPicture();

  return Err::m_nOK;
}

ErrVal 
SliceHeaderBase::FMOUninit()
{
  if(m_pcFMO)
  {
    m_pcFMO->finit(); // Bug_fix memory leak
    delete m_pcFMO; m_pcFMO = NULL;
  }
    
  return Err::m_nOK;
}

Int SliceHeaderBase::getNumMbInSlice()
{  
  Int SliceID =m_pcFMO->getSliceGroupId(getFirstMbInSlice());
  return m_pcFMO->getNumMbInSliceGroup(SliceID);
}



//TMM_WP
ErrVal SliceHeaderBase::PredWeight::setPredWeightsAndFlags( const Int iLumaScale, 
                                                            const Int iChromaScale, 
                                                            const Double *pfWeight, 
                                                            Double fDiscardThr )
{
  const Double *pW = pfWeight;
  const Int iLScale = iLumaScale;
  const Int iCScale = iChromaScale;
  const Int iLumW = (Int)pW[0];
  const Int iCbW = (Int)pW[1];
  const Int iCrW = (Int)pW[2];

  RNOK( init( iLumW, iCbW, iCrW ) );

  setLumaWeightFlag  ( (iLumW != iLScale) );
  setChromaWeightFlag( (iCbW != iCScale) || (iCrW != iCScale) );

  return Err::m_nOK;
}

ErrVal SliceHeaderBase::PredWeight::getPredWeights( Double *afWeight)
{    
    afWeight[0] = (Double) getLumaWeight();
    afWeight[1] = (Double) getChromaWeight(0);
    afWeight[2] = (Double) getChromaWeight(1);
    
    return Err::m_nOK;
}

ErrVal SliceHeaderBase::PredWeightTable::setPredWeightsAndFlags( const Int iLumaScale, const Int iChromaScale, const Double(*pafWeight)[3], Double fDiscardThr )
{
  ROT( 0 == size() );

  for( UInt n = 0; n < size(); n++ )
  {
    RNOK( get(n).setPredWeightsAndFlags( iLumaScale, iChromaScale, pafWeight[n], fDiscardThr ) );
  }
  return Err::m_nOK;
}

ErrVal SliceHeaderBase::copyWeightedPred(PredWeightTable& pcPredWeightTable, UInt uiLumaLogWeightDenom,
                                         UInt uiChromaWeightDenom, ListIdx eListIdx, Bool bDecoder)
{
    m_uiLumaLog2WeightDenom = uiLumaLogWeightDenom;
    m_uiChromaLog2WeightDenom = uiChromaWeightDenom;
    Int iLumaScale = 1 << uiLumaLogWeightDenom;
    Int iChromaScale = 1 << uiChromaWeightDenom;
    Double afWeights[3];
    /* Disable this for now since offsets are not supported for SVC. Enabling this will result in mismatch*/ 
    //Double afOffsets[3];

    if(!bDecoder)
    {
        RNOK( getPredWeightTable(eListIdx).uninit() );
        RNOK( getPredWeightTable(eListIdx).init( getNumRefIdxActive( eListIdx) ) );
    }
        
    for( UInt n = 0; n < pcPredWeightTable.size(); n++ )
    {
        RNOK( pcPredWeightTable.get(n).getPredWeights( afWeights) );
        m_acPredWeightTable[eListIdx].get(n).setPredWeightsAndFlags( iLumaScale, iChromaScale, afWeights, false );

        /* Disable this for now since offsets are not supported for SVC. Enabling this will result in mismatch*/ 
//        RNOK( pcPredWeightTable.get(n).getOffsets( afOffsets) );
//        m_acPredWeightTable[eListIdx].get(n).setOffsets(afOffsets);
    }

    return Err::m_nOK;
}

ErrVal SliceHeaderBase::PredWeight::setOffsets( const Double *pfOffsets)
{
  const Double *pW = pfOffsets;

  const Int iLumO = (Int)pW[0];
  const Int iCbO = (Int)pW[1];
  const Int iCrO = (Int)pW[2];

  setLumaWeightFlag  ( (iLumO != 0) );
  setChromaWeightFlag( (iCbO != 0) || (iCrO != 0) );

  RNOK( initOffsets( iLumO, iCbO, iCrO ) );

  return Err::m_nOK;
}

ErrVal SliceHeaderBase::PredWeight::getOffsets( Double *afOffset)
{    
    afOffset[0] = (Double) getLumaOffset();
    afOffset[1] = (Double) getChromaOffset(0);
    afOffset[2] = (Double) getChromaOffset(1);
    
    return Err::m_nOK;
}

ErrVal SliceHeaderBase::PredWeightTable::setOffsets(  const Double(*pafOffsets)[3] )
{
  ROT( 0 == size() );

  for( UInt n = 0; n < size(); n++ )
  {
    RNOK( get(n).setOffsets( pafOffsets[n] ) );
  }
  return Err::m_nOK;
}
//TMM_WP

H264AVC_NAMESPACE_END

