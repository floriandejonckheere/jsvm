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
#include "MCTF.h"
#include "PreProcessorParameter.h"
#include "../../lib/H264AVCEncoderLib/SliceEncoder.h"

H264AVC_NAMESPACE_BEGIN

#define FACTOR_22_HP  0.70710678118654752440084436210485  //sqrt( 1.0/ 2.0)
#define FACTOR_22_LP  1.4142135623730950488016887242097   //sqrt( 2.0/ 1.0)
#define FACTOR_53_HP  0.84779124789065851738306954082825  //sqrt(23.0/32.0)
#define FACTOR_53_LP  1.2247448713915890490986420373529   //sqrt( 3.0/ 2.0)


MCTF::MCTF()
: m_pcSPS                   ( 0 )
, m_pcPPS                   ( 0 )
, m_pcYuvFullPelBufferCtrl  ( 0 )
, m_pcYuvHalfPelBufferCtrl  ( 0 )
, m_pcMbEncoder             ( 0 )
, m_pcMotionEstimation      ( 0 )
, m_bFirstGOPCoded          ( false )
, m_uiGOPSize               ( 0 )
, m_uiDecompositionStages   ( 0 )
, m_uiFrameWidthInMb        ( 0 )
, m_uiFrameHeightInMb       ( 0 )
, m_uiMbNumber              ( 0 )
, m_papcFrame               ( 0 )
, m_papcResidual            ( 0 )
, m_pcFrameTemp             ( 0 )
, m_pacControlData          ( 0 )
{
}


MCTF::~MCTF()
{
}


ErrVal
MCTF::create( MCTF*& rpcMCTF )
{
  rpcMCTF = new MCTF;
  ROF( rpcMCTF );
  return Err::m_nOK;
}


ErrVal
MCTF::destroy()
{
  delete this;
  return Err::m_nOK;
}


ErrVal
MCTF::init( PreProcessorParameter*  pcParameter,
            MbEncoder*              pcMbEncoder,
            YuvBufferCtrl*          pcYuvFullPelBufferCtrl,
            YuvBufferCtrl*          pcYuvHalfPelBufferCtrl,
            QuarterPelFilter*       pcQuarterPelFilter,
            MotionEstimation*       pcMotionEstimation )
{
  ROF( pcParameter );
  ROF( pcMbEncoder );
  ROF( pcYuvFullPelBufferCtrl );
  ROF( pcYuvHalfPelBufferCtrl );
  ROF( pcQuarterPelFilter );
  ROF( pcMotionEstimation );

  //----- references -----
  m_pcSPS                   = 0;
  m_pcPPS                   = 0;
  m_pcYuvFullPelBufferCtrl  = pcYuvFullPelBufferCtrl;
  m_pcYuvHalfPelBufferCtrl  = pcYuvHalfPelBufferCtrl;
  m_pcMbEncoder             = pcMbEncoder;
  m_pcQuarterPelFilter      = pcQuarterPelFilter;
  m_pcMotionEstimation      = pcMotionEstimation;

  //----- control parameters -----
  m_uiDecompositionStages   = (UInt)floor( 0.5 + log10( (Double)pcParameter->getGOPSize() ) / log10( 2.0 ) );

  for( UInt uiStage = 0; uiStage < MAX_DSTAGES; uiStage++ )
  {
    m_adBaseQpLambdaMotion[uiStage] = pcParameter->getQP();
  }

  //----- init parameter sets -----
  m_uiFrameWidthInMb      = pcParameter->getFrameWidth  () >> 4;
  m_uiFrameHeightInMb     = pcParameter->getFrameHeight () >> 4;
  m_uiMbNumber            = m_uiFrameWidthInMb * m_uiFrameHeightInMb;
  m_bFirstGOPCoded        = false;
  RNOK( SequenceParameterSet::create( m_pcSPS ) );
  RNOK( PictureParameterSet ::create( m_pcPPS ) );
  m_pcSPS->setFrameWidthInMbs           ( m_uiFrameWidthInMb  );
  m_pcSPS->setFrameHeightInMbs          ( m_uiFrameHeightInMb );
  m_pcSPS->setDirect8x8InferenceFlag    ( true  );
  m_pcPPS->setNumRefIdxActive           ( LIST_0, 1 );
  m_pcPPS->setNumRefIdxActive           ( LIST_1, 1 );
  m_pcPPS->setConstrainedIntraPredFlag  ( true );

  //----- init frame buffer controls -----
  RNOK( m_pcYuvFullPelBufferCtrl->initSPS( m_uiFrameHeightInMb<<4, m_uiFrameWidthInMb<<4, YUV_Y_MARGIN, YUV_X_MARGIN    ) );
  RNOK( m_pcYuvHalfPelBufferCtrl->initSPS( m_uiFrameHeightInMb<<4, m_uiFrameWidthInMb<<4, YUV_Y_MARGIN, YUV_X_MARGIN, 1 ) );

  //------ re-allocate dynamic memory -----
  RNOK( xDeleteData() );
  RNOK( xCreateData( *m_pcSPS ) );

  return Err::m_nOK;
}


ErrVal
MCTF::xCreateData( const SequenceParameterSet& rcSPS )
{
  UInt uiIndex;

  ROFS   ( ( m_papcFrame       = new IntFrame*   [ ( 1 << m_uiDecompositionStages ) + 1 ] ) );
  ROFS   ( ( m_papcResidual    = new IntFrame*   [ ( 1 << m_uiDecompositionStages ) + 1 ] ) );
  ROFS   ( ( m_pacControlData  = new ControlData [ ( 1 << m_uiDecompositionStages ) + 1 ] ) );

  for( uiIndex = 0; uiIndex <= ( 1U << m_uiDecompositionStages ); uiIndex++ )
  {
    ROFS ( ( m_papcFrame         [ uiIndex ] = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                              *m_pcYuvHalfPelBufferCtrl ) ) );
    ROFS ( ( m_papcResidual      [ uiIndex ] = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                              *m_pcYuvHalfPelBufferCtrl ) ) );
    RNOK  (   m_papcFrame         [ uiIndex ] ->init        () );
    RNOK  (   m_papcResidual      [ uiIndex ] ->init        () );

    MbDataCtrl*   pcMbDataCtrl                = 0;
    ROFS ( (     pcMbDataCtrl                = new MbDataCtrl  () ) );
    RNOK  (       pcMbDataCtrl                ->init            ( rcSPS ) );
    RNOK  (       m_pacControlData[ uiIndex ] . setMbDataCtrl   ( pcMbDataCtrl ) );

    SliceHeader*  pcSliceHeader               = 0;
    ROFS ( (     pcSliceHeader               = new SliceHeader ( *m_pcSPS, *m_pcPPS ) ) );
    RNOK  (       m_pacControlData[ uiIndex ] . setSliceHeader  (  pcSliceHeader ) );
  }

  ROFS   ( ( m_pcFrameTemp                   = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                              *m_pcYuvHalfPelBufferCtrl ) ) );
  RNOK    (   m_pcFrameTemp                   ->init        () );

  return Err::m_nOK;
}


ErrVal
MCTF::xDeleteData()
{
  UInt uiIndex;

  if( m_papcFrame )
  {
    for( uiIndex = 0; uiIndex <= ( 1U << m_uiDecompositionStages ); uiIndex++ )
    {
      if( m_papcFrame[ uiIndex ] )
      {
        RNOK(   m_papcFrame[ uiIndex ]->uninit() );
        delete  m_papcFrame[ uiIndex ];
        m_papcFrame[ uiIndex ] = 0;
      }
    }
    delete [] m_papcFrame;
    m_papcFrame = 0;
  }

  if( m_papcResidual )
  {
    for( uiIndex = 0; uiIndex <= ( 1U << m_uiDecompositionStages ); uiIndex++ )
    {
      if( m_papcResidual[ uiIndex ] )
      {
        RNOK(   m_papcResidual[ uiIndex ]->uninit() );
        delete  m_papcResidual[ uiIndex ];
        m_papcResidual[ uiIndex ] = 0;
      }
    }
    delete [] m_papcResidual;
    m_papcResidual = 0;
  }

  if( m_pcFrameTemp )
  {
    RNOK(   m_pcFrameTemp->uninit() );
    delete  m_pcFrameTemp;
    m_pcFrameTemp = 0;
  }

  if( m_pacControlData )
  {
    for( uiIndex = 0; uiIndex <= ( 1U << m_uiDecompositionStages ); uiIndex++ )
    {
      MbDataCtrl*   pcMbDataCtrl  = m_pacControlData[ uiIndex ].getMbDataCtrl  ();
      SliceHeader*  pcSliceHeader = m_pacControlData[ uiIndex ].getSliceHeader ();
      if( pcMbDataCtrl )
      {
        RNOK( pcMbDataCtrl->uninit() );
      }
      delete pcMbDataCtrl;
      delete pcSliceHeader;
    }
    delete [] m_pacControlData;
    m_pacControlData = 0;
  }

  return Err::m_nOK;
}


ErrVal
MCTF::uninit()
{
  xDeleteData();

  return Err::m_nOK;
}


ErrVal
MCTF::xFillAndUpsampleFrame( IntFrame* rcFrame )
{
  RNOK( m_pcYuvFullPelBufferCtrl->initMb() );
  RNOK( m_pcYuvHalfPelBufferCtrl->initMb() );

  if( ! rcFrame->isHalfPel() )
  {
    RNOK( rcFrame->initHalfPel() );
  }
  RNOK( rcFrame->extendFrame( m_pcQuarterPelFilter ) );

  return Err::m_nOK;
}


ErrVal
MCTF::xFillAndExtendFrame( IntFrame* rcFrame )
{
  RNOK( m_pcYuvFullPelBufferCtrl->initMb() );
  RNOK( rcFrame->extendFrame( NULL ) );
  return Err::m_nOK;
}


ErrVal
MCTF::xMotionEstimation( RefFrameList*    pcRefFrameList0,
                         RefFrameList*    pcRefFrameList1,
                         const IntFrame*  pcOrigFrame,
                         ControlData&     rcControlData )
{
  SliceHeader&  rcSliceHeader = *rcControlData.getSliceHeader();
  MbDataCtrl*   pcMbDataCtrl  =  rcControlData.getMbDataCtrl ();

  //===== initialization =====
  RNOK( m_pcMotionEstimation->initSlice( rcSliceHeader ) );
  RNOK( m_pcMbEncoder       ->initSlice( rcSliceHeader ) );

  //===== loop over macroblocks =====
  for( UInt uiMbIndex = 0; uiMbIndex < m_uiMbNumber; uiMbIndex++ )
  {
    UInt          uiMbY               = uiMbIndex / m_uiFrameWidthInMb;
    UInt          uiMbX               = uiMbIndex % m_uiFrameWidthInMb;
    MbDataAccess* pcMbDataAccess      = 0;

    RNOK( pcMbDataCtrl            ->initMb  ( pcMbDataAccess, uiMbY, uiMbX ) );
    RNOK( m_pcYuvFullPelBufferCtrl->initMb  ( uiMbY, uiMbX ) );
    RNOK( m_pcYuvHalfPelBufferCtrl->initMb  ( uiMbY, uiMbX ) );
    RNOK( m_pcMotionEstimation    ->initMb  ( uiMbY, uiMbX, *pcMbDataAccess ) );

    RNOK( m_pcMbEncoder->estimatePrediction ( *pcMbDataAccess, NULL, 0, 
                                              *pcRefFrameList0, *pcRefFrameList1, NULL, NULL,
                                              *pcOrigFrame, *m_pcFrameTemp,
                                              false, 4, 8, false, rcControlData.getLambda() ) );
  }

  return Err::m_nOK;
}


ErrVal
MCTF::xMotionCompensation( IntFrame*        pcMCFrame,
                           RefFrameList*    pcRefFrameList0,
                           RefFrameList*    pcRefFrameList1,
                           MbDataCtrl*      pcMbDataCtrl,
                           SliceHeader&     rcSH )
{
  RNOK( pcMbDataCtrl        ->initSlice( rcSH, PRE_PROCESS, false, NULL ) );
  RNOK( m_pcMotionEstimation->initSlice( rcSH ) );

  for( UInt uiMbIndex = 0; uiMbIndex < m_uiMbNumber; uiMbIndex++ )
  {
    UInt          uiMbY           = uiMbIndex / m_uiFrameWidthInMb;
    UInt          uiMbX           = uiMbIndex % m_uiFrameWidthInMb;
    MbDataAccess* pcMbDataAccess  = 0;

    RNOK( pcMbDataCtrl            ->initMb    ( pcMbDataAccess, uiMbY, uiMbX ) );
    RNOK( m_pcYuvFullPelBufferCtrl->initMb    (                 uiMbY, uiMbX ) );
    RNOK( m_pcYuvHalfPelBufferCtrl->initMb    (                 uiMbY, uiMbX ) );
    RNOK( m_pcMotionEstimation    ->initMb    (                 uiMbY, uiMbX, *pcMbDataAccess ) );

    RNOK( m_pcMbEncoder->compensatePrediction ( *pcMbDataAccess, pcMCFrame,
                                                *pcRefFrameList0, *pcRefFrameList1,
                                                false, false ) );
  }

  return Err::m_nOK;
}


ErrVal
MCTF::xGetConnections( Double&  rdL0Rate,
                       Double&  rdL1Rate,
                       Double&  rdBiRate )
{
  //=== just a guess ===
  rdL0Rate = 0.2;
  rdL1Rate = 0.2;
  rdBiRate = 0.6;
  return Err::m_nOK;
}


ErrVal
MCTF::xZeroIntraMacroblocks( IntFrame*    pcFrame,
                             ControlData& rcCtrlData )
{
  MbDataCtrl*       pcMbDataCtrl  = rcCtrlData.getMbDataCtrl       ();
  SliceHeader*      pcSliceHeader = rcCtrlData.getSliceHeader      ();
  IntYuvPicBuffer*  pcPicBuffer   = pcFrame  ->getFullPelYuvBuffer ();

  RNOK( pcMbDataCtrl->initSlice( *pcSliceHeader, PRE_PROCESS, false, NULL ) );

  IntYuvMbBuffer cZeroMbBuffer;
  cZeroMbBuffer.setAllSamplesToZero();

  for( UInt uiMbIndex = 0; uiMbIndex < m_uiMbNumber; uiMbIndex++ )
  {
    UInt          uiMbY           = uiMbIndex / m_uiFrameWidthInMb;
    UInt          uiMbX           = uiMbIndex % m_uiFrameWidthInMb;
    MbDataAccess* pcMbDataAccess  = 0;

    RNOK( pcMbDataCtrl            ->initMb( pcMbDataAccess, uiMbY, uiMbX ) );
    RNOK( m_pcYuvFullPelBufferCtrl->initMb(                 uiMbY, uiMbX ) );

    if( pcMbDataAccess->getMbData().isIntra() )
    {
      pcPicBuffer->loadBuffer( &cZeroMbBuffer );
    }
  }

  return Err::m_nOK;
}


ErrVal
MCTF::xInitGOP( PicBufferList&  rcPicBufferInputList )
{
  //========== INITIALIZE DECOMPOSITION STRUCTURES ==========
  PicBufferList::iterator cInputIter    = rcPicBufferInputList.begin();
  UInt                    uiFrame       = 0;
  UInt                    uiOldGOPSize  = m_uiGOPSize;
  m_uiGOPSize                           = rcPicBufferInputList.size () - ( m_bFirstGOPCoded ? 0 : 1 );
  if( m_bFirstGOPCoded )
  {
    m_papcFrame[ uiFrame++ ]->copyAll( m_papcFrame[ uiOldGOPSize ] );
  }
  for( ; uiFrame <= m_uiGOPSize; uiFrame++, cInputIter++ )
  {
    m_papcFrame[ uiFrame   ]->load   ( *cInputIter );
  }

  //========== INITIALIZE SLICE HEADERS (the decomposition structure is determined at this point) ==========
  if( ! m_bFirstGOPCoded )
  {
    RNOK  ( xInitSliceHeader( 0, 0 ) );
  }
  UInt uiTemporalLevel;
  for( uiTemporalLevel = 0; uiTemporalLevel <= m_uiDecompositionStages; uiTemporalLevel++ )
  {
    UInt      uiStep    = ( 1 << ( m_uiDecompositionStages - uiTemporalLevel ) );
    for( UInt uiFrameId = uiStep; uiFrameId <= m_uiGOPSize; uiFrameId += ( uiStep << 1 ) )
    {
      RNOK( xInitSliceHeader( uiTemporalLevel, uiFrameId ) );
    }
  }

  //========== INITIALIZE SCALING FACTORS ==========
  for( uiFrame = 0; uiFrame <= m_uiGOPSize; uiFrame++ )
  {
    m_pacControlData      [ uiFrame ].clear();
    m_pacControlData      [ uiFrame ].setScalingFactor( 1.0 );
    RNOK( m_pacControlData[ uiFrame ].getMbDataCtrl()->reset () );
    RNOK( m_pacControlData[ uiFrame ].getMbDataCtrl()->clear () );
  }

  return Err::m_nOK;
}


ErrVal
MCTF::xGetListSizes( UInt  uiTemporalLevel,
                     UInt  uiFrameIdInGOP,
                     UInt  auiPredListSize[2],
                     UInt  aauiUpdListSize[MAX_DSTAGES][2] )
{
  //----- clear update list sizes -----
  ::memset( aauiUpdListSize, 0x00, 2 * MAX_DSTAGES * sizeof( UInt ) );

  //----- loop over prediction and update steps -----
  for( UInt uiLevel = uiTemporalLevel; uiLevel <= m_uiDecompositionStages; uiLevel++ )
  {
    //----- get parameters base GOP size and cut-off frame id -----
    UInt  uiBaseLevel       = m_uiDecompositionStages - uiLevel;
    UInt  uiFrameIdLevel    = uiFrameIdInGOP >> uiBaseLevel;
    UInt  uiBaseGOPSize     = ( 1 << m_uiDecompositionStages ) >> uiBaseLevel;

    if( uiLevel == uiTemporalLevel )
    {
      //=========== PREDICTION LIST SIZES ==========
      auiPredListSize[0]    = ( uiFrameIdLevel + 1 ) >> 1;
      UInt  uiFrameIdWrap   = ( uiFrameIdLevel % uiBaseGOPSize );
      if( uiFrameIdWrap > 0 )
      {
        auiPredListSize[1]  = ( uiBaseGOPSize - uiFrameIdWrap + 1 ) >> 1;
      }
      else
      {
        auiPredListSize[1]  = ( uiFrameIdWrap + 1 ) >> 1;
      }
      auiPredListSize[0]    = min( 1, auiPredListSize[0] );
      auiPredListSize[1]    = min( 1, auiPredListSize[1] );
      UInt  uiMaxL1Size     = ( ( m_uiGOPSize >> uiBaseLevel ) + 1 - uiFrameIdLevel ) >> 1;
      auiPredListSize[1]    = min( uiMaxL1Size, auiPredListSize[1] );
    }
    else
    {
      //========== UPDATE LIST SIZES ==========
      UInt* pauiUpdListSize = aauiUpdListSize[uiLevel-1];
      UInt  uiFrameIdWrap   = ( uiFrameIdLevel == 0 ? 0 : ( ( uiFrameIdLevel - 1 ) % uiBaseGOPSize ) + 1 );
      if( uiFrameIdWrap > 0 )
      {
        pauiUpdListSize[0]  = uiFrameIdWrap >> 1;
        pauiUpdListSize[1]  = ( uiBaseGOPSize - uiFrameIdWrap ) >> 1;
      }
      else
      {
        pauiUpdListSize[0]  = uiFrameIdWrap >> 1;
        pauiUpdListSize[1]  = ( uiFrameIdLevel == 0 ? 0 : ( 1 - uiFrameIdWrap ) >> 1 );
      }
      pauiUpdListSize[0]    = min( 1, pauiUpdListSize[0] );
      pauiUpdListSize[1]    = min( 1, pauiUpdListSize[1] );
      UInt  uiMaxL1Size     = ( ( m_uiGOPSize >> uiBaseLevel ) + 1 - uiFrameIdLevel ) >> 1;
      pauiUpdListSize[1]    = min( uiMaxL1Size, pauiUpdListSize[1] );
    }
  }

  return Err::m_nOK;
}


ErrVal
MCTF::xInitSliceHeader( UInt uiTemporalLevel,
                        UInt uiFrameIdInGOP )
{
  SliceHeader*  pcSliceHeader = m_pacControlData[ uiFrameIdInGOP ].getSliceHeader();
  ROF( pcSliceHeader );

  //===== get maximum sizes of prediction and update lists ( decomposition structure ) =====
  UInt  auiPredListSize             [2];
  UInt  aauiUpdListSize[MAX_DSTAGES][2];
  RNOK( xGetListSizes( uiTemporalLevel, uiFrameIdInGOP, auiPredListSize, aauiUpdListSize ) );

  //===== get slice header parameters =====
  SliceType     eSliceType      = ( auiPredListSize[1] ? B_SLICE : auiPredListSize[0] ? P_SLICE : I_SLICE );

  //===== set simple slice header parameters =====
  pcSliceHeader->setNalUnitType                 ( NAL_UNIT_CODED_SLICE_SCALABLE );
  pcSliceHeader->setFirstMbInSlice              ( 0                     );
  pcSliceHeader->setLastMbInSlice               ( m_uiMbNumber - 1      );
  pcSliceHeader->setNumMbsInSlice               ( m_uiMbNumber          );
  pcSliceHeader->setSliceType                   ( eSliceType            );
  pcSliceHeader->setDirectSpatialMvPredFlag     ( true                  );

  //===== set prediction and update list sizes =====
  pcSliceHeader->setNumRefIdxActive( LIST_0, 0 );
  pcSliceHeader->setNumRefIdxActive( LIST_1, 0 );

  UInt  uiMaxLists = ( eSliceType == B_SLICE ? 2 : eSliceType == P_SLICE ? 1 : 0 );
  for( UInt uiList = 0; uiList < uiMaxLists; uiList++ )
  {
    ListIdx eListIdx  = ListIdx( uiList );
    ROF( auiPredListSize[ uiList ] );

    pcSliceHeader->setNumRefIdxActive( eListIdx, auiPredListSize[ uiList ] );
    if( pcSliceHeader->getPPS().getNumRefIdxActive( eListIdx ) != auiPredListSize[ uiList ] )
    {
      pcSliceHeader->setNumRefIdxActiveOverrideFlag( true );
    }
  }
  for( UInt uiLevel = uiTemporalLevel; uiLevel < m_uiDecompositionStages; uiLevel++ )
  {
    pcSliceHeader->setNumRefIdxUpdate( uiLevel, LIST_0, aauiUpdListSize[uiLevel][0] );
    pcSliceHeader->setNumRefIdxUpdate( uiLevel, LIST_1, aauiUpdListSize[uiLevel][1] );
  }

  //===== pred weights =====
  RNOK( pcSliceHeader->getPredWeightTable(LIST_0).uninit() );
  RNOK( pcSliceHeader->getPredWeightTable(LIST_1).uninit() );
  RNOK( pcSliceHeader->getPredWeightTable(LIST_0).init( pcSliceHeader->getNumRefIdxActive( LIST_0) ) );
  RNOK( pcSliceHeader->getPredWeightTable(LIST_1).init( pcSliceHeader->getNumRefIdxActive( LIST_1) ) );
  RNOK( pcSliceHeader->getPredWeightTable(LIST_1).initDefaults( pcSliceHeader->getLumaLog2WeightDenom(), pcSliceHeader->getChromaLog2WeightDenom() ) );
  RNOK( pcSliceHeader->getPredWeightTable(LIST_0).initDefaults( pcSliceHeader->getLumaLog2WeightDenom(), pcSliceHeader->getChromaLog2WeightDenom() ) );

  return Err::m_nOK;
}


ErrVal
MCTF::xSetScalingFactors( UInt uiBaseLevel )
{
  Double  adRateL0 [( 1 << MAX_DSTAGES )];
  Double  adRateL1 [( 1 << MAX_DSTAGES )];
  Double  adRateBi [( 1 << MAX_DSTAGES )];

  Double  dScalingBase    = m_pacControlData[0].getScalingFactor();
  Double  dScalingLowPass = 0.0;
  Int     iLowPassSize    = ( m_uiGOPSize >> uiBaseLevel );
  Int     iFrame;

  //===== get connection data =====
  for( iFrame = 1; iFrame <= iLowPassSize; iFrame += 2 )
  {
    RNOK( xGetConnections( adRateL0[iFrame], adRateL1[iFrame], adRateBi[iFrame] ) );
  }

  //===== get low-pass scaling =====
  for( iFrame = 0; iFrame <= iLowPassSize; iFrame += 2 )
  {
    Double  dScalLPCurr = 1.0;

    if( iFrame > 0 )
    {
      if( ( iFrame + 1 ) < iLowPassSize )
      {
        dScalLPCurr = ( adRateBi[iFrame-1] + adRateBi[iFrame+1] ) * ( FACTOR_53_LP - 1.0 ) / 2.0 +
                      ( adRateL1[iFrame-1] + adRateL0[iFrame+1] ) * ( FACTOR_22_LP - 1.0 ) / 2.0 + 1.0;
      }
      else
      {
        dScalLPCurr = ( adRateBi[iFrame-1] / 2.0 ) * ( FACTOR_53_LP - 1.0 ) +
                      ( adRateL1[iFrame-1]       ) * ( FACTOR_22_LP - 1.0 ) + 1.0;
      }
    }
    else
    {
      if( iLowPassSize )
      {
        dScalLPCurr = ( adRateBi[iFrame+1] / 2.0 ) * ( FACTOR_53_LP - 1.0 ) +
                      ( adRateL0[iFrame+1]       ) * ( FACTOR_22_LP - 1.0 ) + 1.0;
      }
    }

    dScalingLowPass += dScalLPCurr;
  }
  dScalingLowPass /= (Double)( 1 + ( iLowPassSize >> 1 ) );

  Double dFactor53 = FACTOR_53_HP;
  Double dFactor22 = FACTOR_22_HP;

  //===== get high-pass scaling and set scaling factors =====
  for( iFrame = 0; iFrame <= iLowPassSize; iFrame++ )
  {
    Double dScal = dScalingBase;

    if( iFrame % 2 )
    {
      dScal *= ( adRateBi[iFrame]                    ) * ( dFactor53 - 1.0 ) +
               ( adRateL0[iFrame] + adRateL1[iFrame] ) * ( dFactor22 - 1.0 ) + 1.0;
    }
    else
    {
      dScal *= dScalingLowPass;
    }
    m_pacControlData[ iFrame << uiBaseLevel ].setScalingFactor( dScal );
  }

  return Err::m_nOK;
}


ErrVal
MCTF::xClearBufferExtensions()
{
  for( UInt uiFrame = 0; uiFrame <= m_uiGOPSize; uiFrame++ )
  {
    RNOK( m_papcFrame   [uiFrame]->uninitHalfPel() );
    RNOK( m_papcResidual[uiFrame]->uninitHalfPel() );
  }

  return Err::m_nOK;
}


ErrVal
MCTF::xGetPredictionLists( RefFrameList& rcRefList0,
                           RefFrameList& rcRefList1,
                           UInt          uiBaseLevel,
                           UInt          uiFrame,
                           Bool          bHalfPel )
{
  rcRefList0.reset();
  rcRefList1.reset();

  UInt          uiFrameIdInGOP  = ( uiFrame << uiBaseLevel );
  SliceHeader*  pcSliceHeader   = m_pacControlData[uiFrameIdInGOP].getSliceHeader();
  UInt          uiList0Size     = pcSliceHeader->getNumRefIdxActive( LIST_0 );
  UInt          uiList1Size     = pcSliceHeader->getNumRefIdxActive( LIST_1 );

  //===== list 0 =====
  {
    Int iFrameId;
    for( iFrameId = Int( uiFrame - 1 ); iFrameId >= 0 && uiList0Size; iFrameId -= 2 )
    {
      IntFrame* pcFrame = m_papcFrame[ iFrameId << uiBaseLevel ];
      if( ! pcFrame->isExtended() )
      {
        if( bHalfPel )
        {
          RNOK( xFillAndUpsampleFrame ( pcFrame ) );
        }
        else
        {
          RNOK( xFillAndExtendFrame   ( pcFrame ) );
        }
      }
      RNOK( rcRefList0.add( pcFrame ) );
      uiList0Size--;
    }
    ROT( uiList0Size );
  }

  //===== list 1 =====
  {
    Int iFrameId;
    for( iFrameId = Int( uiFrame + 1 ); iFrameId <= (Int)( m_uiGOPSize >> uiBaseLevel ) && uiList1Size; iFrameId += 2 )
    {
      IntFrame* pcFrame = m_papcFrame[ iFrameId << uiBaseLevel ];
      if( ! pcFrame->isExtended() )
      {
        if( bHalfPel )
        {
          RNOK( xFillAndUpsampleFrame ( pcFrame ) );
        }
        else
        {
          RNOK( xFillAndExtendFrame   ( pcFrame ) );
        }
      }
      RNOK( rcRefList1.add( pcFrame ) );
      uiList1Size--;
    }
    ROT( uiList1Size );
  }

  return Err::m_nOK;
}


ErrVal  
MCTF::xGetUpdateLists( RefFrameList& rcRefList0,
                       RefFrameList& rcRefList1,
                       CtrlDataList& rcCtrlList0,
                       CtrlDataList& rcCtrlList1,
                       UInt          uiBaseLevel,
                       UInt          uiFrame )
{
  rcRefList0  .reset();
  rcRefList1  .reset();
  rcCtrlList0 .reset();
  rcCtrlList1 .reset();

  UInt          uiFrameIdInGOP  = ( uiFrame << uiBaseLevel );
  SliceHeader*  pcSliceHeader   = m_pacControlData[uiFrameIdInGOP].getSliceHeader();
  UInt          uiUpdateLevel   = m_uiDecompositionStages - uiBaseLevel - 1;
  UInt          uiList0Size     = pcSliceHeader->getNumRefIdxUpdate( uiUpdateLevel, LIST_0 );
  UInt          uiList1Size     = pcSliceHeader->getNumRefIdxUpdate( uiUpdateLevel, LIST_1 );

  //===== list 0 =====
  {
    for( Int iFrameId = Int( uiFrame - 1 ); iFrameId >= 0 && uiList0Size; iFrameId -= 2 )
    {
      IntFrame*     pcFrame       =  m_papcResidual   [ iFrameId << uiBaseLevel ];
      ControlData*  pcControlData = &m_pacControlData [ iFrameId << uiBaseLevel ];
      if( ! pcFrame->isExtended() )
      {
        RNOK( xFillAndExtendFrame( pcFrame ) );
      }
      RNOK( rcRefList0  .add( pcFrame ) );
      RNOK( rcCtrlList0 .add( pcControlData ) );
      uiList0Size--;
    }
    ROT( uiList0Size );
  }

  //===== list 1 =====
  {
    for( Int iFrameId = Int( uiFrame + 1 ); iFrameId <= (Int)( m_uiGOPSize >> uiBaseLevel ) && uiList1Size; iFrameId += 2 )
    {
      IntFrame*     pcFrame       =  m_papcResidual   [ iFrameId << uiBaseLevel ];
      ControlData*  pcControlData = &m_pacControlData [ iFrameId << uiBaseLevel ];
      if( ! pcFrame->isExtended() )
      {
        RNOK( xFillAndExtendFrame( pcFrame ) );
      }
      RNOK( rcRefList1  .add( pcFrame ) );
      RNOK( rcCtrlList1 .add( pcControlData ) );
      uiList1Size--;
    }
    ROT( uiList1Size );
  }

  return Err::m_nOK;
}


ErrVal
MCTF::xInitControlDataMotion( UInt uiBaseLevel,
                              UInt uiFrame,
                              Bool bMotionEstimation )
{
  UInt            uiFrameIdInGOP    = uiFrame << uiBaseLevel;
  ControlData&    rcControlData     = m_pacControlData[uiFrameIdInGOP];
  SliceHeader*    pcSliceHeader     = rcControlData.getSliceHeader  ();
  MbDataCtrl*     pcMbDataCtrl      = rcControlData.getMbDataCtrl   ();
  Double          dScalFactor       = rcControlData.getScalingFactor() * FACTOR_53_HP;
  Double          dQpPredData       = m_adBaseQpLambdaMotion[ uiBaseLevel ] - 6.0 * log10( dScalFactor ) / log10( 2.0 );
  Double          dLambda           = 0.85 * pow( 2.0, min( 52.0, dQpPredData ) / 3.0 - 4.0 );
  Int             iQp               = max( MIN_QP, min( MAX_QP, (Int)floor( dQpPredData + 0.5 ) ) );

  pcSliceHeader->setSliceHeaderQp( iQp );
  rcControlData. setLambda       ( dLambda );

  if( bMotionEstimation )
  {
    RNOK( pcMbDataCtrl->initSlice( *pcSliceHeader, ENCODE_PROCESS, false, NULL ) );
  }

  return Err::m_nOK;
}


ErrVal
MCTF::xMotionEstimationStage( UInt uiBaseLevel )
{
  for( UInt uiFrame = 1; uiFrame <= ( m_uiGOPSize >> uiBaseLevel ); uiFrame += 2 )
  {
    printf(".");
    UInt          uiFrameIdInGOP  = uiFrame << uiBaseLevel;
    ControlData&  rcControlData   = m_pacControlData[uiFrameIdInGOP];
    IntFrame*     pcFrame         = m_papcFrame     [uiFrameIdInGOP];

    //===== get reference frame lists =====
    RefFrameList& rcRefFrameList0 = rcControlData.getPrdFrameList( LIST_0 );
    RefFrameList& rcRefFrameList1 = rcControlData.getPrdFrameList( LIST_1 );
    RNOK( xGetPredictionLists   ( rcRefFrameList0, rcRefFrameList1, uiBaseLevel, uiFrame, true ) );
    RNOK( xInitControlDataMotion( uiBaseLevel, uiFrame, true ) );
    RNOK( xMotionEstimation     ( &rcRefFrameList0, &rcRefFrameList1, pcFrame, rcControlData ) );
  }
  RNOK( xClearBufferExtensions() );

  return Err::m_nOK;
}


ErrVal
MCTF::xDecompositionStage( UInt uiBaseLevel )
{
  //===== PREDICTION =====
  for( UInt uiFramePrd  = 1; uiFramePrd <= ( m_uiGOPSize >> uiBaseLevel ); uiFramePrd += 2 )
  {
    UInt          uiFrameIdInGOP  = uiFramePrd << uiBaseLevel;
    ControlData&  rcControlData   = m_pacControlData[uiFrameIdInGOP];
    IntFrame*     pcFrame         = m_papcFrame     [uiFrameIdInGOP];
    IntFrame*     pcResidual      = m_papcResidual  [uiFrameIdInGOP];
    IntFrame*     pcMCFrame       = m_pcFrameTemp;

    RefFrameList& rcRefFrameList0 = rcControlData.getPrdFrameList( LIST_0 );
    RefFrameList& rcRefFrameList1 = rcControlData.getPrdFrameList( LIST_1 );
    RNOK( xGetPredictionLists   ( rcRefFrameList0, rcRefFrameList1, uiBaseLevel, uiFramePrd, false ) );
    RNOK( xInitControlDataMotion( uiBaseLevel, uiFramePrd, false ) );
    RNOK( xMotionCompensation   ( pcMCFrame, &rcRefFrameList0, &rcRefFrameList1,
                                  rcControlData.getMbDataCtrl(), *rcControlData.getSliceHeader() ) );
    RNOK( pcFrame ->prediction  ( pcMCFrame, pcFrame ) );
    RNOK( pcResidual->copy      ( pcFrame ) );
    RNOK( xZeroIntraMacroblocks ( pcResidual, rcControlData ) );
  }
  RNOK  ( xClearBufferExtensions() );

  //===== UPDATE =====
  for( UInt uiFrameUpd = 2; uiFrameUpd <= ( m_uiGOPSize >> uiBaseLevel ); uiFrameUpd += 2 )
  {
    UInt          uiFrameIdInGOP  = uiFrameUpd << uiBaseLevel;
    IntFrame*     pcFrame         = m_papcFrame     [uiFrameIdInGOP];

    RefFrameList  acRefFrameListUpd[2];
    CtrlDataList  acCtrlDataList[2];
    RNOK( xGetUpdateLists       ( acRefFrameListUpd[0], acRefFrameListUpd[1],
                                  acCtrlDataList[0], acCtrlDataList[1], uiBaseLevel, uiFrameUpd ) );
    RNOK( xUpdateCompensation   ( pcFrame, &acRefFrameListUpd[0], &acCtrlDataList[0], LIST_0 ) );
    RNOK( xUpdateCompensation   ( pcFrame, &acRefFrameListUpd[1], &acCtrlDataList[1], LIST_1 ) );
  }
  RNOK  ( xClearBufferExtensions() );

  return Err::m_nOK;
}


ErrVal
MCTF::xCompositionStage( UInt           uiBaseLevel,
                         PicBufferList& rcPicBufferInputList )
{
  //===== PREDICTION =====
  for( UInt uiFramePrd  = 1; uiFramePrd <= ( m_uiGOPSize >> uiBaseLevel ); uiFramePrd += 2 )
  {
    UInt          uiFrameIdInGOP  = uiFramePrd << uiBaseLevel;
    ControlData&  rcControlData   = m_pacControlData[uiFrameIdInGOP];
    IntFrame*     pcFrame         = m_papcFrame     [uiFrameIdInGOP];
    IntFrame*     pcMCFrame       = m_pcFrameTemp;

    RefFrameList& rcRefFrameList0 = rcControlData.getPrdFrameList( LIST_0 );
    RefFrameList& rcRefFrameList1 = rcControlData.getPrdFrameList( LIST_1 );
    RNOK( xGetPredictionLists         ( rcRefFrameList0, rcRefFrameList1,
                                        uiBaseLevel, uiFramePrd, false ) );
    RNOK( xMotionCompensation         ( pcMCFrame, &rcRefFrameList0, &rcRefFrameList1,
                                        rcControlData.getMbDataCtrl(), *rcControlData.getSliceHeader() ) );
    RNOK( pcFrame ->inversePrediction ( pcMCFrame, pcFrame ) );
  }
  RNOK( xClearBufferExtensions() );

  return Err::m_nOK;
}


ErrVal
MCTF::xStoreReconstruction( PicBufferList&  rcPicBufferOutputList )
{
  PicBufferList::iterator cOutputIter = rcPicBufferOutputList.begin();
  for( UInt uiIndex = (m_bFirstGOPCoded?1:0); uiIndex <= m_uiGOPSize; uiIndex++, cOutputIter++ )
  {
    RNOK( m_papcFrame[uiIndex]->store( *cOutputIter ) );
  }
  return Err::m_nOK;
}


ErrVal
MCTF::xProcessGOP( PicBufferList&    rcPicBufferInputList,
                   PicBufferList&    rcPicBufferOutputList,
                   PicBufferList&    rcPicBufferUnusedList )
{
  RNOK( xInitGOP( rcPicBufferInputList ) );

  for( Int iLevel = 0; iLevel < (Int)m_uiDecompositionStages; iLevel++ )
  {
    RNOK( xMotionEstimationStage  ( iLevel ) );
    RNOK( xDecompositionStage     ( iLevel ) );
    RNOK( xSetScalingFactors      ( iLevel ) );
  }

  UInt   uiStage    = m_uiDecompositionStages;
  while( uiStage--  > 0 )
  {
    RNOK( xCompositionStage( uiStage, rcPicBufferInputList ) );
  }

  RNOK( xStoreReconstruction( rcPicBufferOutputList ) );
  RNOK( xFinishGOP          ( rcPicBufferInputList,
                              rcPicBufferOutputList,
                              rcPicBufferUnusedList ) );

  return Err::m_nOK;
}


ErrVal
MCTF::xFinishGOP( PicBufferList& rcPicBufferInputList,
                  PicBufferList& rcPicBufferOutputList,
                  PicBufferList& rcPicBufferUnusedList )
{
  while( rcPicBufferOutputList.size() > m_uiGOPSize + ( m_bFirstGOPCoded ? 0 : 1 ) )
  {
    PicBuffer*  pcPicBuffer = rcPicBufferOutputList.popBack();
    rcPicBufferUnusedList.push_back( pcPicBuffer );
  }
  m_bFirstGOPCoded = true;

  return Err::m_nOK;
}


ErrVal
MCTF::xUpdateCompensation( IntFrame*        pcMCFrame,
                           RefFrameList*    pcRefFrameList,
                           CtrlDataList*    pcCtrlDataList,
                           ListIdx          eListUpd)
{
  ROFRS( pcCtrlDataList->getActive(), Err::m_nOK );

  ListIdx eListPrd = ListIdx( 1-eListUpd );
  for( Int iRefIdx = 1; iRefIdx <= (Int)pcCtrlDataList->getActive(); iRefIdx++ )    
  {
    for( UInt uiMbIndex = 0; uiMbIndex < m_uiMbNumber; uiMbIndex++ )
    {
      UInt          uiMbY           = uiMbIndex / m_uiFrameWidthInMb;
      UInt          uiMbX           = uiMbIndex % m_uiFrameWidthInMb;
      MbDataCtrl*   pcMbDataCtrlPrd = (*pcCtrlDataList)[ iRefIdx ]->getMbDataCtrl();
      MbDataAccess* pcMbDataAccess  = 0;

      RNOK( pcMbDataCtrlPrd         ->initMb( pcMbDataAccess, uiMbY, uiMbX ) );
      RNOK( m_pcYuvFullPelBufferCtrl->initMb(                 uiMbY, uiMbX ) );
      RNOK( m_pcYuvHalfPelBufferCtrl->initMb(                 uiMbY, uiMbX ) );
      RNOK( m_pcMotionEstimation    ->initMb(                 uiMbY, uiMbX, *pcMbDataAccess ) );
      RNOK( m_pcMbEncoder->compensateUpdate ( *pcMbDataAccess, pcMCFrame,
                                              iRefIdx, eListPrd, (*pcRefFrameList)[ iRefIdx ] ) );
    }
  }

  return Err::m_nOK;
}


ErrVal
MCTF::process( PicBuffer*     pcOrgPicBuffer,
               PicBuffer*     pcRecPicBuffer,
               PicBufferList& rcPicBufferOutputList,
               PicBufferList& rcPicBufferUnusedList )
{
  if( pcOrgPicBuffer == NULL )
  {
    //===== finish =====
    if( pcRecPicBuffer )
    {
      rcPicBufferUnusedList.push_back( pcRecPicBuffer );
    }
    if( m_cOrgPicBufferList.size() )
    {
      RNOK( xProcessGOP( m_cOrgPicBufferList, m_cRecPicBufferList, rcPicBufferUnusedList ) );
      rcPicBufferOutputList += m_cRecPicBufferList;
    }
    rcPicBufferUnusedList   += m_cOrgPicBufferList;
    rcPicBufferUnusedList   += m_cRecPicBufferList;
    m_cOrgPicBufferList.clear();
    m_cRecPicBufferList.clear();
    return Err::m_nOK;
  }

  //===== insert buffers =====
  m_cOrgPicBufferList.push_back( pcOrgPicBuffer );
  m_cRecPicBufferList.push_back( pcRecPicBuffer );

  UInt uiTargetBufferSize = ( 1 << m_uiDecompositionStages ) + ( m_bFirstGOPCoded ? 0 : 1 );
  ROT( m_cOrgPicBufferList.size() > uiTargetBufferSize );

  if( m_cOrgPicBufferList.size() == uiTargetBufferSize )
  {
    RNOK( xProcessGOP( m_cOrgPicBufferList, m_cRecPicBufferList, rcPicBufferUnusedList ) );
    rcPicBufferOutputList += m_cRecPicBufferList;
    rcPicBufferUnusedList += m_cOrgPicBufferList;
    rcPicBufferUnusedList += m_cRecPicBufferList;
    m_cOrgPicBufferList.clear();
    m_cRecPicBufferList.clear();
  }

  return Err::m_nOK;
}


H264AVC_NAMESPACE_END
