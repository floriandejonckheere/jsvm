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
#include "GOPDecoder.h"
#include "SliceReader.h"
#include "SliceDecoder.h"
#include "FGSSubbandDecoder.h"
#include "CreaterH264AVCDecoder.h"
#include "ControlMngH264AVCDecoder.h"
#include "H264AVCCommonLib/TraceFile.h"
#include <math.h>



H264AVC_NAMESPACE_BEGIN


__inline Void printSpaces( UInt uiNum )
{
  while( uiNum-- ) printf(" ");
}


MCTFDecoder::MCTFDecoder()
: m_pcH264AVCDecoder              ( 0 )
, m_pcSliceReader                 ( 0 )
, m_pcSliceDecoder                ( 0 )
, m_pcNalUnitParser               ( 0 )
, m_pcControlMng                  ( 0 )
, m_pcLoopFilter                  ( 0 )
, m_pcHeaderSymbolReadIf          ( 0 )
, m_pcParameterSetMng             ( 0 )
, m_pcPocCalculator               ( 0 )
, m_pcYuvFullPelBufferCtrl        ( 0 )
, m_pcMotionCompensation          ( 0 )
, m_pcQuarterPelFilter            ( 0 )
, m_bInitDone                     ( false )
, m_bCreateDone                   ( false )
, m_eNextNalType                  ( LOW_PASS_IDR )
, m_uiMaxDecompositionStages      ( 0 )
, m_uiMaxGOPSize                  ( 0 )
, m_uiFrameWidthInMb              ( 0 )
, m_uiFrameHeightInMb             ( 0 )
, m_uiMbNumber                    ( 0 )
, m_uiGOPId                       ( 0 )
, m_uiFrameNumber                 ( 0 )
, m_uiGOPSize                     ( 0 )
, m_uiDecompositionStages         ( 0 )
, m_papcFrame                     ( 0 )
, m_papcResidual                  ( 0 )
, m_pcBaseLayerFrame              ( 0 )
, m_pcBaseLayerResidual           ( 0 )
, m_pcAnchorFrame                 ( 0 )
, m_pcPredSignal                  ( 0 )
, m_pcBaseLayerCtrl               ( 0 )
, m_pacControlData                ( 0 )
, m_uiLayerId                     ( 0 )
, m_pusUpdateWeights              ( 0 )
, m_pcLowPassBaseReconstruction   ( 0 )
, m_bActive( false )
, m_bReconstructed( true )
{
  ::memset( m_apcFrameTemp, 0x00, sizeof( m_apcFrameTemp ) );
}



MCTFDecoder::~MCTFDecoder()
{
}



ErrVal
MCTFDecoder::create( MCTFDecoder*& rpcMCTFDecoder )
{
  rpcMCTFDecoder = new MCTFDecoder;
  ROT( NULL == rpcMCTFDecoder );
  return Err::m_nOK;
}



ErrVal
MCTFDecoder::destroy()
{
  ROT( m_bInitDone );

  delete this;
  return Err::m_nOK;
}



ErrVal
MCTFDecoder::init( H264AVCDecoder*      pcH264AVCDecoder,
                   SliceReader*         pcSliceReader,
                   SliceDecoder*        pcSliceDecoder,
                   RQFGSDecoder*        pcRQFGSDecoder,
                   NalUnitParser*       pcNalUnitParser,
                   ControlMngIf*        pcControlMng,
                   LoopFilter*          pcLoopFilter,
                   HeaderSymbolReadIf*  pcHeaderSymbolReadIf,
                   ParameterSetMng*     pcParameterSetMng,
                   PocCalculator*       pcPocCalculator,
                   YuvBufferCtrl*       pcYuvFullPelBufferCtrl,
                   MotionCompensation*  pcMotionCompensation,
                   QuarterPelFilter*    pcQuarterPelFilter )
{
  ROT( NULL == pcH264AVCDecoder );
  ROT( NULL == pcSliceReader );
  ROT( NULL == pcSliceDecoder );
  ROT( NULL == pcNalUnitParser );
  ROT( NULL == pcControlMng );
  ROT( NULL == pcLoopFilter );
  ROT( NULL == pcHeaderSymbolReadIf );
  ROT( NULL == pcParameterSetMng );
  ROT( NULL == pcPocCalculator );
  ROT( NULL == pcQuarterPelFilter );
  ROT( NULL == pcYuvFullPelBufferCtrl );
  ROT( NULL == pcMotionCompensation );
  ROT( NULL == pcRQFGSDecoder );

  m_pcH264AVCDecoder              = pcH264AVCDecoder;
  m_pcSliceReader                 = pcSliceReader;
  m_pcSliceDecoder                = pcSliceDecoder ;
  m_pcNalUnitParser               = pcNalUnitParser;
  m_pcControlMng                  = pcControlMng;
  m_pcLoopFilter                  = pcLoopFilter;
  m_pcHeaderSymbolReadIf          = pcHeaderSymbolReadIf;
  m_pcParameterSetMng             = pcParameterSetMng;
  m_pcPocCalculator               = pcPocCalculator;
  m_pcYuvFullPelBufferCtrl        = pcYuvFullPelBufferCtrl;
  m_pcMotionCompensation          = pcMotionCompensation;
  m_pcQuarterPelFilter            = pcQuarterPelFilter;
  m_pcRQFGSDecoder                = pcRQFGSDecoder;

  m_bActive = false;
  
  m_bInitDone                     = true;
  m_bCreateDone                   = false;
  m_eNextNalType                  = LOW_PASS_IDR;
  m_uiMaxDecompositionStages      = 0;
  m_uiMaxGOPSize                  = 0;
  m_uiFrameWidthInMb              = 0;
  m_uiFrameHeightInMb             = 0;
  m_uiMbNumber                    = 0;
  m_uiGOPId                       = 0;
  
  m_uiFrameNumber                 = 0;
  m_uiGOPSize                     = 0;
  m_uiDecompositionStages         = 0;

  m_papcFrame                     = 0;
  m_papcResidual                  = 0;
  m_pcBaseLayerFrame              = 0;
  m_pcBaseLayerResidual           = 0;
  m_pcAnchorFrame                 = 0;
  m_pcPredSignal                  = 0;

  m_pcBaseLayerCtrl               = 0;
  m_pacControlData                = 0;
  m_pusUpdateWeights              = 0;

  m_uiLayerId                     = 0;


  return Err::m_nOK;
}



ErrVal
MCTFDecoder::uninit()
{
  m_pcSliceReader             = NULL;
  m_pcSliceDecoder            = NULL;
  m_pcNalUnitParser           = NULL;
  m_pcControlMng              = NULL;
  m_pcLoopFilter              = NULL;
  m_pcHeaderSymbolReadIf      = NULL;
  m_pcParameterSetMng         = NULL;
  m_pcPocCalculator           = NULL;
  m_pcYuvFullPelBufferCtrl    = NULL;
  m_pcMotionCompensation      = NULL;
  m_uiMaxDecompositionStages  = 0;
  m_uiMaxGOPSize              = 0;
  m_uiFrameWidthInMb          = 0;
  m_uiFrameHeightInMb         = 0;

  RNOK( xDeleteData() );

  m_bInitDone                 = false;

  return Err::m_nOK;
}



__inline UInt downround2powerof2( UInt i ) { UInt r = 1; for( ; ( 1 << r ) <= i; r++ ); return ( 1 << ( r - 1 ) ); }

ErrVal
MCTFDecoder::initSPS( const SequenceParameterSet& rcSPS )
{
  //===== get and set relevant parameters =====
  UInt  uiMaxDPBSize  = rcSPS.getMaxDPBSize();
  m_uiLayerId         = rcSPS.getLayerId();
  m_uiFrameWidthInMb  = rcSPS.getFrameWidthInMbs();
  m_uiFrameHeightInMb = rcSPS.getFrameHeightInMbs();
  m_uiMbNumber        = m_uiFrameWidthInMb * m_uiFrameHeightInMb;
  m_uiMaxGOPSize      = downround2powerof2( uiMaxDPBSize );

  //===== re-allocate dynamic memory =====
  RNOK( xDeleteData() );
  RNOK( xCreateData( rcSPS ) );

  //===== initialize some parameters =====
  m_bActive         = true;
  m_bReconstructed  = true;
  m_bInitDone       = true;

  return Err::m_nOK;
}



ErrVal
MCTFDecoder::process( SliceHeader*&  rpcSliceHeader,
                      PicBuffer*     pcPicBuffer,
                      PicBufferList& rcPicBufferOutputList,
                      PicBufferList& rcPicBufferUnusedList,
                      PicBufferList& rcPicBufferReleaseList )
{
  ROF( m_bInitDone );



  //===== initialize GOP =====
  if( xIsNewGOP( rpcSliceHeader ) )
  {
    Int iMaxPoc;
    RNOK( xReconstruct( rcPicBufferOutputList, rcPicBufferUnusedList, iMaxPoc ) );
  }

  //===== store picture buffers =====
  if( pcPicBuffer && m_cPicBufferList.size() <= m_uiMaxGOPSize )
  {
    m_cPicBufferList      .push_back( pcPicBuffer );
  }
  else
  {
    rcPicBufferUnusedList .push_back( pcPicBuffer );
  }


  //===== check NAL unit for the start of the next GOP =====
  switch( m_eNextNalType )
  {
  case LOW_PASS_IDR:
    ROFRS( rpcSliceHeader->getTemporalLevel   () == 0,                                  Err::m_nOK );
    ROFRS( rpcSliceHeader->getNalUnitType     () == NAL_UNIT_CODED_SLICE_IDR_SCALABLE,  Err::m_nOK );
    RNOK ( xInitGOP( rpcSliceHeader ) );
    break;
  
  case LOW_PASS:
    RNOK ( xInitGOP( rpcSliceHeader ) );
    break;
  }

  //===== decoding =====
  if( rpcSliceHeader->getSliceType() == F_SLICE )
  {
    RNOK( xDecodeFGSRefinement  ( rpcSliceHeader ) );
  }
  else if( rpcSliceHeader->getTemporalLevel() )
  {
    RNOK( xDecodeHighPassSignal ( rpcSliceHeader ) );
  }
  else
  {
    RNOK( xDecodeLowPassSignal  ( rpcSliceHeader ) );
  }


  //===== delete slice header (if not stored) =====
  delete rpcSliceHeader;
  rpcSliceHeader = NULL;


  return Err::m_nOK;
}



ErrVal
MCTFDecoder::finishProcess( PicBufferList&  rcPicBufferOutputList,
                            PicBufferList&  rcPicBufferUnusedList,
                            PicBufferList&  rcPicBufferReleaseList,
                            Int&            riMaxPoc )
{
  RNOK( xReconstruct( rcPicBufferOutputList, rcPicBufferUnusedList, riMaxPoc ) );

  rcPicBufferUnusedList += m_cPicBufferList;
  m_cPicBufferList      .clear();

  return Err::m_nOK;
}



ErrVal
MCTFDecoder::reconstruct( PicBufferList&  rcPicBufferOutputList,
                          PicBufferList&  rcPicBufferUnusedList,
                          PicBufferList&  rcPicBufferReleaseList,
                          Int&            riMaxPoc )
{
  RNOK( xReconstruct( rcPicBufferOutputList, rcPicBufferUnusedList, riMaxPoc ) );

  rcPicBufferUnusedList += m_cPicBufferList;
  m_cPicBufferList.clear();

  return Err::m_nOK;
}



ErrVal
MCTFDecoder::getBaseLayerData( IntFrame*&   pcFrame,
                               IntFrame*&   pcResidual,
                               MbDataCtrl*& pcMbDataCtrl,
                               Bool         bHighPass,
                               Bool         bSpatialScalability,
                               Int          iPoc )
{
  UInt  uiPos   = MSYS_UINT_MAX;
  pcFrame       = 0;
  pcResidual    = 0;
  pcMbDataCtrl  = 0;

  for( UInt uiFrame = 0; uiFrame <= m_uiGOPSize; uiFrame++ )
  {
    if( m_pacControlData[uiFrame].getSliceHeader() &&
        m_pacControlData[uiFrame].getSliceHeader()->getPoc() == iPoc )
    {
      pcResidual    = m_papcResidual  [uiFrame];
      pcFrame       = m_papcFrame     [uiFrame];
      pcMbDataCtrl  = m_pacControlData[uiFrame].getMbDataCtrl();
      uiPos         = uiFrame;
      break;
    }
  }

#if ! UNRESTRICTED_INTER_LAYER_PREDICTION
  if( pcFrame )
  {
    if ( bHighPass && bSpatialScalability )
    {
      RNOK( m_apcFrameTemp[0]->copy( pcFrame ) );
      pcFrame = m_apcFrameTemp[0];

      m_pcLoopFilter->setFilterMode( LoopFilter::LFMode( LoopFilter::LFM_NO_INTER_FILTER + LoopFilter::LFM_EXTEND_INTRA_SUR ) );
      RNOK( m_pcLoopFilter->process(*m_pacControlData[uiPos].getSliceHeader (),
                                     pcFrame,
                                     m_pacControlData[uiPos].getMbDataCtrl  (),
                                     m_pacControlData[uiPos].getMbDataCtrl  (),
                                     m_uiFrameWidthInMb,
                                     NULL,
                                     NULL ) );
      m_pcLoopFilter->setFilterMode();
    }
    else if( bSpatialScalability )
    {
      RNOK( m_pcLoopFilter->process(*m_pacControlData[uiPos].getSliceHeader (),
                                     pcFrame,
                                     m_pacControlData[uiPos].getMbDataCtrl  (),
                                     m_pacControlData[uiPos].getMbDataCtrl  (),
                                     m_uiFrameWidthInMb,
                                    &m_pacControlData[uiPos].getDPCMRefFrameList( LIST_0 ),
                                    &m_pacControlData[uiPos].getDPCMRefFrameList( LIST_1 ) ) );
    }
  }
#endif
  
  return Err::m_nOK;
}






ErrVal 
MCTFDecoder::xCreateData( const SequenceParameterSet& rcSPS )
{
  UInt uiIndex;


  //========== CREATE FRAME MEMORIES ==========
  ROFRS   ( ( m_papcFrame                   = new IntFrame* [ m_uiMaxGOPSize + 1 ]      ), Err::m_nERR );
  ROFRS   ( ( m_papcResidual                = new IntFrame* [ m_uiMaxGOPSize + 1 ]      ), Err::m_nERR );
  
  for( uiIndex = 0; uiIndex <= m_uiMaxGOPSize; uiIndex++ )
  {
    ROFRS ( ( m_papcFrame     [ uiIndex ]   = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                            *m_pcYuvFullPelBufferCtrl ) ), Err::m_nERR );
    ROFRS ( ( m_papcResidual  [ uiIndex ]   = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                            *m_pcYuvFullPelBufferCtrl ) ), Err::m_nERR );
    RNOK  (   m_papcFrame     [ uiIndex ]   ->init        () );
    RNOK  (   m_papcResidual  [ uiIndex ]   ->init        () );
  }
  
  for( uiIndex = 0; uiIndex < NUM_TMP_FRAMES;  uiIndex++ )
  {
    ROFRS ( ( m_apcFrameTemp  [ uiIndex ]   = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                            *m_pcYuvFullPelBufferCtrl ) ), Err::m_nERR );
    RNOK  (   m_apcFrameTemp  [ uiIndex ]   ->init        () );
  }
  
  ROFRS   ( ( m_pcLowPassBaseReconstruction = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                            *m_pcYuvFullPelBufferCtrl ) ), Err::m_nERR );
  RNOK    (   m_pcLowPassBaseReconstruction ->init        () );
  
  ROFRS   ( ( m_pcAnchorFrame               = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                            *m_pcYuvFullPelBufferCtrl ) ), Err::m_nERR );
  RNOK    (   m_pcAnchorFrame               ->init        () );
  
  ROFRS   ( ( m_pcPredSignal                = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                            *m_pcYuvFullPelBufferCtrl ) ), Err::m_nERR );
  RNOK    (   m_pcPredSignal                ->init        () );
  
  ROFRS   ( ( m_pcBaseLayerFrame            = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                            *m_pcYuvFullPelBufferCtrl ) ), Err::m_nERR );
  RNOK    (   m_pcBaseLayerFrame            ->init        () );
  
  ROFRS   ( ( m_pcBaseLayerResidual         = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                            *m_pcYuvFullPelBufferCtrl ) ), Err::m_nERR );
  RNOK    (   m_pcBaseLayerResidual         ->init        () );



  //========== CREATE MACROBLOCK DATA MEMORIES ==========
  ROFRS   ( ( m_pacControlData  = new ControlData[ m_uiMaxGOPSize + 1 ] ), Err::m_nERR );

  for( uiIndex = 0; uiIndex <= m_uiMaxGOPSize; uiIndex++ )
  {
    MbDataCtrl*   pcMbDataCtrl                = 0;
    ROFRS ( (     pcMbDataCtrl                = new MbDataCtrl  () ), Err::m_nERR );
    RNOK  (       pcMbDataCtrl                ->init            ( rcSPS ) );
    RNOK  (       m_pacControlData[ uiIndex ] . setMbDataCtrl   ( pcMbDataCtrl ) );
  }
  
  MbDataCtrl*   pcMbDataCtrl    = 0;
  ROFRS   ( (   pcMbDataCtrl    = new MbDataCtrl() ), Err::m_nERR );
  RNOK    (     pcMbDataCtrl    ->init          ( rcSPS ) );
  RNOK    (     m_cControlDataUpd.setMbDataCtrl ( pcMbDataCtrl ) );

  ROFRS   ( ( m_pcBaseLayerCtrl = new MbDataCtrl() ), Err::m_nERR );
  RNOK    (   m_pcBaseLayerCtrl ->init          ( rcSPS ) );



  //========== CREATE UPDATE WEIGHTS ARRAY and WRITE BUFFER ==========
  UInt  uiNum4x4Blocks        = m_uiFrameWidthInMb * m_uiFrameHeightInMb * 4 * 4;
  ROFRS( ( m_pusUpdateWeights = new UShort[ uiNum4x4Blocks      ] ), Err::m_nERR );


  
  //========== RE-INITIALIZE OBJECTS ==========
  RNOK( m_cConnectionArray.init   ( rcSPS ) );
  ROT ( m_cDownConvert    .init   ( m_uiFrameWidthInMb<<4, m_uiFrameHeightInMb<<4 ) );

  return Err::m_nOK;
}



ErrVal 
MCTFDecoder::xDeleteData()
{
  UInt uiIndex;

  //========== DELETE FRAME MEMORIES ==========
  if( m_papcFrame )
  {
    for( uiIndex = 0; uiIndex <= m_uiMaxGOPSize; uiIndex++ )
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
    for( uiIndex = 0; uiIndex <= m_uiMaxGOPSize; uiIndex++ )
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
  
  for( uiIndex = 0; uiIndex < NUM_TMP_FRAMES; uiIndex++ )
  {
    if( m_apcFrameTemp[ uiIndex ] )
    {
      RNOK(   m_apcFrameTemp[ uiIndex ]->uninit() );
      delete  m_apcFrameTemp[ uiIndex ];
      m_apcFrameTemp[ uiIndex ] = 0;
    }
  }

  if( m_pcLowPassBaseReconstruction )
  {
    RNOK(   m_pcLowPassBaseReconstruction->uninit() );
    delete  m_pcLowPassBaseReconstruction;
    m_pcLowPassBaseReconstruction = 0;
  }

  if( m_pcAnchorFrame )
  {
    RNOK(   m_pcAnchorFrame->uninit() );
    delete  m_pcAnchorFrame;
    m_pcAnchorFrame = 0;
  }

  if( m_pcPredSignal )
  {
    RNOK(   m_pcPredSignal->uninit() );
    delete  m_pcPredSignal;
    m_pcPredSignal = 0;
  }

  if( m_pcBaseLayerFrame )
  {
    RNOK(   m_pcBaseLayerFrame->uninit() );
    delete  m_pcBaseLayerFrame;
    m_pcBaseLayerFrame = 0;
  }

  if( m_pcBaseLayerResidual )
  {
    RNOK(   m_pcBaseLayerResidual->uninit() );
    delete  m_pcBaseLayerResidual;
    m_pcBaseLayerResidual = 0;
  }


  //========== DELETE MACROBLOCK DATA MEMORIES (and SLICE HEADER) ==========
  if( m_pacControlData )
  {
    for( uiIndex = 0; uiIndex <= m_uiMaxGOPSize; uiIndex++ )
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

  {
    MbDataCtrl*   pcMbDataCtrl  = m_cControlDataUpd.getMbDataCtrl  ();
    SliceHeader*  pcSliceHeader = m_cControlDataUpd.getSliceHeader ();
    if( pcMbDataCtrl )
    {
      RNOK( pcMbDataCtrl->uninit() );
    }
    delete pcMbDataCtrl;
    delete pcSliceHeader;
    m_cControlDataUpd.setMbDataCtrl ( 0 );
    m_cControlDataUpd.setSliceHeader( 0 );
  }

  if( m_pcBaseLayerCtrl )
  {
    RNOK( m_pcBaseLayerCtrl->uninit() );
    delete m_pcBaseLayerCtrl;
    m_pcBaseLayerCtrl = 0;
  }
  

  //========== DELETE UPDATE WEIGHTS ARRAY and WRITE BUFFER ==========
  delete [] m_pusUpdateWeights;
  m_pusUpdateWeights  = 0;


  return Err::m_nOK;
}




Bool
MCTFDecoder::xIsNewGOP( SliceHeader* pcSliceHeader )
{
  ROTRS( m_eNextNalType == LOW_PASS_IDR,              true );
  ROTRS( pcSliceHeader->getSliceType() != F_SLICE &&
         pcSliceHeader->getGOPId    () != m_uiGOPId,  true );

  return false;
}



ErrVal
MCTFDecoder::xInitGOP( SliceHeader*  pcSliceHeader )
{
  m_bReconstructed = false;


  UInt uiIndex;
  for( uiIndex = 0; uiIndex <= m_uiMaxGOPSize; uiIndex++ )
  {
    m_papcFrame   [uiIndex]->initRecLayer ();
    m_papcResidual[uiIndex]->initRecLayer ();

    m_papcFrame   [uiIndex]->setBandType  ( LPS );
    m_papcResidual[uiIndex]->setBandType  ( HPS );
  }
  if( m_pcAnchorFrame->isValid() )
  {
    RNOK( m_papcFrame[0]->copyAll( m_pcAnchorFrame ) );
  }


  //============================= CONTROL DATA =============================
  for( uiIndex = 0; uiIndex <= m_uiMaxGOPSize; uiIndex++ )
  {
    SliceHeader*  pcSH = m_pacControlData[uiIndex].getSliceHeader();
    delete pcSH;

    m_pacControlData[uiIndex].clear();
    m_pacControlData[uiIndex].setSliceHeader( 0 );
  }
  m_cControlDataUpd.clear();


  //============================ PARAMETERS ================================
  m_uiGOPId                   = pcSliceHeader->getGOPId               ();
  m_uiGOPSize                 = pcSliceHeader->getGOPSize             ();
  m_uiDecompositionStages     = pcSliceHeader->getDecompositionStages ();
  m_eNextNalType              = ALL;
  
  return Err::m_nOK;
}




ErrVal
MCTFDecoder::xZeroIntraMacroblocks( IntFrame*    pcFrame,
                                    ControlData& rcCtrlData )
{
  MbDataCtrl*       pcMbDataCtrl  = rcCtrlData.getMbDataCtrl       ();
  SliceHeader*      pcSliceHeader = rcCtrlData.getSliceHeader      ();
  IntYuvPicBuffer*  pcPicBuffer   = pcFrame  ->getFullPelYuvBuffer ();

  RNOK( pcMbDataCtrl->initSlice( *pcSliceHeader, PRE_PROCESS, true, NULL ) );

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
MCTFDecoder::xClipIntraMacroblocks( IntFrame*    pcFrame,
                                   ControlData& rcCtrlData, Bool bClipAll )
{
  MbDataCtrl*       pcMbDataCtrl  = rcCtrlData.getMbDataCtrl       ();
  SliceHeader*      pcSliceHeader = rcCtrlData.getSliceHeader      ();
  IntYuvPicBuffer*  pcPicBuffer   = pcFrame  ->getFullPelYuvBuffer ();
  IntYuvMbBuffer    cMbBuffer;

  RNOK( pcMbDataCtrl->initSlice( *pcSliceHeader, PRE_PROCESS, false, NULL ) );

  for( UInt uiMbIndex = 0; uiMbIndex < m_uiMbNumber; uiMbIndex++ )
  {
    UInt          uiMbY           = uiMbIndex / m_uiFrameWidthInMb;
    UInt          uiMbX           = uiMbIndex % m_uiFrameWidthInMb;
    MbDataAccess* pcMbDataAccess  = 0;

    RNOK( pcMbDataCtrl            ->initMb( pcMbDataAccess, uiMbY, uiMbX ) );
    RNOK( m_pcYuvFullPelBufferCtrl->initMb(                 uiMbY, uiMbX ) );

    if( bClipAll || pcMbDataAccess->getMbData().isIntra() )
    {
      cMbBuffer   .loadBuffer( pcPicBuffer );
      cMbBuffer   .clip      ();
      pcPicBuffer->loadBuffer( &cMbBuffer );
    }
  }

  return Err::m_nOK;
}



ErrVal
MCTFDecoder::xAddBaseLayerResidual( ControlData& rcControlData,
                                    IntFrame*    pcFrame )
{
  ROFRS( rcControlData.getBaseLayerSbb(), Err::m_nOK );

  MbDataCtrl*       pcMbDataCtrl  = rcControlData.getMbDataCtrl       ();
  SliceHeader*      pcSliceHeader = rcControlData.getSliceHeader      ();
  IntYuvPicBuffer*  pcBLResidual  = rcControlData.getBaseLayerSbb     ()->getFullPelYuvBuffer();
  IntYuvPicBuffer*  pcPicBuffer   = pcFrame     ->getFullPelYuvBuffer ();
  IntYuvMbBuffer    cBLResBuffer;
  IntYuvMbBuffer    cMbBuffer;

  RNOK( pcMbDataCtrl->initSlice( *pcSliceHeader, PRE_PROCESS, true, NULL ) );

  for( UInt uiMbIndex = 0; uiMbIndex < m_uiMbNumber; uiMbIndex++ )
  {
    UInt          uiMbY           = uiMbIndex / m_uiFrameWidthInMb;
    UInt          uiMbX           = uiMbIndex % m_uiFrameWidthInMb;
    MbDataAccess* pcMbDataAccess  = 0;

    RNOK( pcMbDataCtrl            ->initMb( pcMbDataAccess, uiMbY, uiMbX ) );
    RNOK( m_pcYuvFullPelBufferCtrl->initMb(                 uiMbY, uiMbX ) );

    if( ! pcMbDataAccess->getMbData().isIntra() && pcMbDataAccess->getMbData().getResidualPredFlag( PART_16x16 ) )
    {
      cMbBuffer   .loadBuffer ( pcPicBuffer  );
      cBLResBuffer.loadBuffer ( pcBLResidual );
      cMbBuffer   .add        ( cBLResBuffer );
      pcPicBuffer->loadBuffer ( &cMbBuffer );
    }
  }

  return Err::m_nOK;
}



ErrVal
MCTFDecoder::initSlice( SliceHeader* pcSliceHeader )
{
  ROFRS( m_bActive, Err::m_nOK );

  //===== calculate POC =====
  if( pcSliceHeader && pcSliceHeader->getLayerId() == m_uiLayerId )
  {
    RNOK( m_pcPocCalculator->calculatePoc( *pcSliceHeader ) );
  }

  //===== check if an FGS enhancement needs to be reconstructed =====
  if( m_pcRQFGSDecoder->isInitialized  ()                                                               &&
      m_pcRQFGSDecoder->getSliceHeader ()->getLayerId() == m_uiLayerId                                  &&
    (!pcSliceHeader                                                                                     ||
      pcSliceHeader->getLayerId       () != m_pcRQFGSDecoder->getSliceHeader()->getLayerId      ()      ||
      pcSliceHeader->getTemporalLevel () != m_pcRQFGSDecoder->getSliceHeader()->getTemporalLevel()      ||
      pcSliceHeader->getQualityLevel  () != m_pcRQFGSDecoder->getSliceHeader()->getQualityLevel () + 1  ||
      pcSliceHeader->getPoc           () != m_pcRQFGSDecoder->getSliceHeader()->getPoc          ()        ) )
  {
    RNOK( xReconstructLastFGS() );
  }

  return Err::m_nOK;
}



ErrVal
MCTFDecoder::xReconstructLastFGS()
{
  if( m_pcRQFGSDecoder->changed() )
  {
    SliceHeader*  pcSliceHeader   = m_pcRQFGSDecoder->getSliceHeader      ();
    UInt          uiFrameIdInGOP  = pcSliceHeader   ->getFrameIdInsideGOP ();
    IntFrame*     pcFrame         = m_papcFrame     [uiFrameIdInGOP];
    IntFrame*     pcResidual      = m_papcResidual  [uiFrameIdInGOP];
    ControlData&  rcControlData   = m_pacControlData[uiFrameIdInGOP];

    RNOK( m_pcRQFGSDecoder->reconstruct( pcFrame ) );

    RNOK( xAddBaseLayerResidual   ( rcControlData, pcFrame ) );
    RNOK( pcResidual      ->copy  ( pcFrame ) )
    RNOK( xZeroIntraMacroblocks   ( pcResidual, rcControlData ) );
    RNOK( pcFrame         ->add   ( m_pcPredSignal ) );

    RNOK( xClipIntraMacroblocks( pcFrame, rcControlData, pcSliceHeader->getTemporalLevel() == 0 ) );
  }

  RNOK( m_pcRQFGSDecoder->finishPicture () );


  return Err::m_nOK;
}





ErrVal
MCTFDecoder::xDecodeFGSRefinement( SliceHeader*& rpcSliceHeader )
{
  ROFRS( m_pcRQFGSDecoder->isInitialized(), Err::m_nOK );

  printf("  Frame %4d ( LId%2d, TL%2d, QL%2d, PrRef,              QP%3d )\n",
    rpcSliceHeader->getPoc                    (),
    rpcSliceHeader->getLayerId                (),
    rpcSliceHeader->getTemporalLevel          (),
    rpcSliceHeader->getQualityLevel           (),
    rpcSliceHeader->getPicQp                  () );

  //===== check slice header =====
  if(!m_pcRQFGSDecoder->isFinished    ()                                                                 &&
      m_pcRQFGSDecoder->getSliceHeader()->getTemporalLevel()      == rpcSliceHeader->getTemporalLevel () &&
      m_pcRQFGSDecoder->getSliceHeader()->getPoc          ()      == rpcSliceHeader->getPoc           () &&
      m_pcRQFGSDecoder->getSliceHeader()->getQualityLevel () + 1  == rpcSliceHeader->getQualityLevel  ()  )
  {
    RNOK( m_pcRQFGSDecoder->decodeNextLayer( rpcSliceHeader ) );
  }

  return Err::m_nOK;
}






ErrVal
MCTFDecoder::xInitBaseLayerData( ControlData& rcControlData )
{
  //===== init =====
  rcControlData.setBaseLayerRec ( 0 );
  rcControlData.setBaseLayerSbb ( 0 );
  rcControlData.setBaseLayerCtrl( 0 );

  
  IntFrame*   pcBaseFrame         = 0;
  IntFrame*   pcBaseResidual      = 0;
  MbDataCtrl* pcBaseDataCtrl      = 0;
  Bool        bSpatialScalability = false;
  Bool        bBaseDataAvailable  = false;

  if( rcControlData.getSliceHeader()->getBaseLayerId() != MSYS_UINT_MAX )
  {
    RNOK( m_pcH264AVCDecoder->getBaseLayerData( pcBaseFrame,
                                                pcBaseResidual,
                                                pcBaseDataCtrl,
                                                bSpatialScalability,
                                                m_uiLayerId,
                                                rcControlData.getSliceHeader()->getBaseLayerId(),
                                                rcControlData.getSliceHeader()->getTemporalLevel() > 0,
                                                rcControlData.getSliceHeader()->getPoc() ) );
    bBaseDataAvailable = pcBaseFrame && pcBaseResidual && pcBaseDataCtrl;
    ROF( bBaseDataAvailable );

    rcControlData.setBaseLayerResolution( bSpatialScalability );
  }


  //===== motion data =====
  if( pcBaseDataCtrl )
  {
    RNOK( m_pcBaseLayerCtrl->initSlice( *rcControlData.getSliceHeader(), PRE_PROCESS, false, NULL ) );
    if( ! rcControlData.isHalfResolutionBaseLayer() )
    {
      RNOK( m_pcBaseLayerCtrl->copyMotionBL  ( *pcBaseDataCtrl ) );
    }
    else
    {
      RNOK( m_pcBaseLayerCtrl->upsampleMotion( *pcBaseDataCtrl ) );
    }
    rcControlData.setBaseLayerCtrl( m_pcBaseLayerCtrl );
  }


  //===== residual data =====
  if( bBaseDataAvailable )
  {
    // if RECONSTRUCTION BYPASS ?????  ===> zero intra macroblocks ?????

    RNOK( m_pcBaseLayerResidual->copy( pcBaseResidual ) ); 
    if( rcControlData.isHalfResolutionBaseLayer() )
    {
      RNOK( m_pcBaseLayerResidual->upsampleResidual( m_cDownConvert, 1,
                                                     pcBaseDataCtrl, false ) );
    }
    rcControlData.setBaseLayerSbb( m_pcBaseLayerResidual );
  }


  //==== reconstructed (intra) data =====
  if( bBaseDataAvailable )
  {
    RNOK( m_pcBaseLayerFrame->copy( pcBaseFrame ) );
    if( rcControlData.isHalfResolutionBaseLayer() )
    {
      RNOK( m_pcBaseLayerFrame->upsample( m_cDownConvert, 1, true ) );
    }
    rcControlData.setBaseLayerRec( m_pcBaseLayerFrame );
  }


  return Err::m_nOK;
}






ErrVal
MCTFDecoder::xDecodeLowPassSignal( SliceHeader*& rpcSliceHeader )
{
  printf("  Frame %4d ( LId%2d, TL%2d, QL%2d, %s-%c, BId%2d, AP%2d, QP%3d )\n",
    rpcSliceHeader->getPoc                    (),
    rpcSliceHeader->getLayerId                (),
    rpcSliceHeader->getTemporalLevel          (),
    rpcSliceHeader->getQualityLevel           (),
    rpcSliceHeader->isH264AVCCompatible       () ? "AVC" : " LP",
    rpcSliceHeader->getSliceType              () == I_SLICE ? 'I' : 'P',
    rpcSliceHeader->getBaseLayerId            (),
    rpcSliceHeader->getAdaptivePredictionFlag () ? 1 : 0,
    rpcSliceHeader->getPicQp                  () );


  UInt          uiFrameIdInGOP      = rpcSliceHeader->getFrameIdInsideGOP();
  ControlData&  rcControlData       = m_pacControlData[ uiFrameIdInGOP ];   
  IntFrame*     pcFrame             = m_papcFrame     [ uiFrameIdInGOP ];
  IntFrame*     pcResidual          = m_papcResidual  [ uiFrameIdInGOP ];
  IntFrame*     pcLPFrame           = m_apcFrameTemp  [0];
  MbDataCtrl*   pcMbDataCtrl        = rcControlData.getMbDataCtrl();
  UInt          uiMbRead            = 0;

  ROT ( pcFrame->getRecLayer() );
  
  RNOK( rcControlData      .setSliceHeader(  rpcSliceHeader ) );
  RNOK( pcMbDataCtrl      ->reset         () );
  RNOK( pcMbDataCtrl      ->clear         () );
  pcFrame->setPOC( rpcSliceHeader->getPoc() );

  
  RNOK( xInitBaseLayerData( rcControlData ) );

  
  //----- initialize reference lists -----
  RefFrameList& cRefFrameList0 = rcControlData.getDPCMRefFrameList( 0 );
  RefFrameList& cRefFrameList1 = rcControlData.getDPCMRefFrameList( 1 );
  cRefFrameList0.reset();
  cRefFrameList1.reset();
  RNOK( cRefFrameList0.add  ( m_pcLowPassBaseReconstruction ) );
  RNOK( xFillAndExtendFrame ( m_pcLowPassBaseReconstruction ) );

  //----- decode and reconstruct low-pass picture and subband -----
  if( rpcSliceHeader->isIntra() )
  {
    //----- parsing -----
    RNOK( m_pcControlMng  ->initSliceForReading ( *rpcSliceHeader ) );
    RNOK( m_pcSliceReader ->read                ( *rpcSliceHeader,
                                                  pcMbDataCtrl,
                                                  rcControlData.getBaseLayerCtrl(),
                                                  rcControlData.isHalfResolutionBaseLayer(),
                                                  m_uiFrameWidthInMb,
                                                  uiMbRead ) );

    //----- decoding -----
    RNOK( m_pcControlMng  ->initSliceForDecoding( *rpcSliceHeader ) );
    RNOK( m_pcSliceDecoder->decodeIntra         ( *rpcSliceHeader,
                                                  pcMbDataCtrl,
                                                  pcLPFrame,
                                                  pcResidual,
                                                  m_pcPredSignal,
                                                  rcControlData.getBaseLayerRec(),
                                                  m_uiFrameWidthInMb,
                                                  uiMbRead ) );
  }
  else
  {
    //----- parsing -----
    RNOK( m_pcControlMng  ->initSliceForReading ( *rpcSliceHeader ) );
    RNOK( m_pcSliceReader ->read                ( *rpcSliceHeader,
                                                  pcMbDataCtrl,
                                                  rcControlData.getBaseLayerCtrl(),
                                                  rcControlData.isHalfResolutionBaseLayer(),
                                                  m_uiFrameWidthInMb,
                                                  uiMbRead ) );

    //----- decoding -----
    RNOK( m_pcControlMng  ->initSliceForDecoding( *rpcSliceHeader ) );
    RNOK( m_pcSliceDecoder->decodeInterP        ( *rpcSliceHeader,
                                                  pcMbDataCtrl,
                                                  rcControlData.getBaseLayerCtrl(),
                                                  pcLPFrame,
                                                  pcResidual,
                                                  m_pcPredSignal,
                                                  rcControlData.getBaseLayerRec(),
                                                  rcControlData.getBaseLayerSbb(),
                                                  cRefFrameList0,
                                                  m_uiFrameWidthInMb,
                                                  uiMbRead ) );
  }

  //----- store non-filtered subband and residual -----
  RNOK( pcFrame->copy( pcLPFrame ) );
  pcFrame   ->setBandType( LPS );
  pcFrame   ->addRecLayer();
  pcResidual->addRecLayer();

  //----- loop-filtering and store in DPCM buffer (for following low-pass frames) -----
  RNOK( m_pcLoopFilter->process( *rpcSliceHeader,
                                 pcLPFrame,
                                 ( rpcSliceHeader->isIntra() ? NULL : pcMbDataCtrl ),
                                 pcMbDataCtrl,
                                 m_uiFrameWidthInMb,
                                 &cRefFrameList0,
                                 &cRefFrameList1 ) );
  m_pcLowPassBaseReconstruction->uninitHalfPel();
  RNOK( m_pcLowPassBaseReconstruction->copy( pcLPFrame ) );

  //----- set slice header to zero (slice header is stored in control data) -----
  rpcSliceHeader = 0;

  RNOK( m_pcRQFGSDecoder->initPicture( rcControlData.getSliceHeader(), rcControlData.getMbDataCtrl() ) );

  DTRACE_NEWFRAME;

  return Err::m_nOK;
}





ErrVal
MCTFDecoder::xDecodeHighPassSignal( SliceHeader*& rpcSliceHeader )
{
  printf("  Frame %4d ( LId%2d, TL%2d, QL%2d, %s-%c, BId%2d, AP%2d, QP%3d )\n",
    rpcSliceHeader->getPoc                    (),
    rpcSliceHeader->getLayerId                (),
    rpcSliceHeader->getTemporalLevel          (),
    rpcSliceHeader->getQualityLevel           (),
    rpcSliceHeader->isH264AVCCompatible       () ? "AVC" : " HP",
    rpcSliceHeader->getSliceType              () == B_SLICE ? 'B' : 'P',
    rpcSliceHeader->getBaseLayerId            (),
    rpcSliceHeader->getAdaptivePredictionFlag () ? 1 : 0,
    rpcSliceHeader->getPicQp                  () );


  UInt          uiFrameIdInGOP      = rpcSliceHeader->getFrameIdInsideGOP();
  ControlData&  rcControlData       = m_pacControlData[ uiFrameIdInGOP ];   
  IntFrame*     pcFrame             = m_papcFrame     [ uiFrameIdInGOP ];
  IntFrame*     pcResidual          = m_papcResidual  [ uiFrameIdInGOP ];
  MbDataCtrl*   pcMbDataCtrl        = rcControlData.getMbDataCtrl();
  UInt          uiMbRead            = 0;


  ROT( pcFrame->getRecLayer() );
  
  RNOK( rcControlData      .setSliceHeader(  rpcSliceHeader ) );
  RNOK( pcMbDataCtrl      ->reset         () );
  RNOK( pcMbDataCtrl      ->clear         () );
  pcFrame->setPOC( rpcSliceHeader->getPoc() );

  
  RNOK( xInitBaseLayerData( rcControlData ) );

  
  //----- parsing -----
  RNOK( m_pcControlMng  ->initSliceForReading ( *rpcSliceHeader ) );
  
  RNOK( m_pcSliceReader ->read                ( *rpcSliceHeader,
                                                pcMbDataCtrl,
                                                rcControlData.getBaseLayerCtrl(),
                                                rcControlData.isHalfResolutionBaseLayer(),
                                                m_uiFrameWidthInMb,
                                                uiMbRead ) );

  //----- decode motion vectors -----
  if( rpcSliceHeader->getBaseLayerId() == MSYS_UINT_MAX || rpcSliceHeader->getAdaptivePredictionFlag() )
  {
    RNOK( xCalcMv( rpcSliceHeader, pcMbDataCtrl, rcControlData.getBaseLayerCtrl() ) );
  }

  //----- decode residual signals -----
  RNOK( m_pcControlMng  ->initSliceForDecoding( *rpcSliceHeader ) );
  RNOK( m_pcSliceDecoder->decodeHighPass      ( *rpcSliceHeader,
                                                pcMbDataCtrl,
                                                rcControlData.getBaseLayerCtrl(),
                                                pcFrame,
                                                pcResidual,
                                                m_pcPredSignal,
                                                rcControlData.getBaseLayerSbb(),
                                                rcControlData.getBaseLayerRec(),
                                                m_uiFrameWidthInMb,
                                                uiMbRead ) );

  //----- updating -----
  pcResidual->addRecLayer();
  pcFrame->addRecLayer();
  pcFrame->setBandType( HPS );

  //----- set slice header to zero (slice header is stored in control data) -----
  rpcSliceHeader = 0;

  RNOK( m_pcRQFGSDecoder->initPicture( rcControlData.getSliceHeader(), rcControlData.getMbDataCtrl() ) );

  DTRACE_NEWFRAME;

  return Err::m_nOK;
}



ErrVal
MCTFDecoder::xDumpFrames( Char* pFilename, UInt uiStage )
{
  Char     acTemp[1000];
  if( uiStage == MSYS_UINT_MAX )  sprintf( acTemp, "%s_L%d.yuv",      pFilename, m_uiLayerId );
  else                            sprintf( acTemp, "%s_L%d_S%d.yuv",  pFilename, m_uiLayerId, uiStage );
  FILE*    pFile = ::fopen( acTemp, m_uiGOPId ? "ab" : "wb" );
  
  for( UInt uiFrame = 1; uiFrame <= m_uiGOPSize; uiFrame++ )
  {
    if( m_pacControlData[ uiFrame ].getSliceHeader() )
    {
      RNOK( m_papcFrame[ uiFrame ]->dump( pFile, m_pacControlData[ uiFrame ].getMbDataCtrl() ) );
    }
  }

  ::fclose( pFile );
  return Err::m_nOK;
}



ErrVal
MCTFDecoder::xClearBufferExtensions()
{
  for( UInt uiFrame = 0; uiFrame <= m_uiGOPSize; uiFrame++ )
  {
    RNOK( m_papcFrame   [uiFrame]->uninitHalfPel() );
    RNOK( m_papcResidual[uiFrame]->uninitHalfPel() );
  }

  return Err::m_nOK;
}



ErrVal
MCTFDecoder::xGetPredictionLists( RefFrameList& rcRefList0,
                                  RefFrameList& rcRefList1,
                                  UInt          uiBaseLevel,
                                  UInt          uiFrame )
{
  rcRefList0.reset();
  rcRefList1.reset();

  UInt          uiFrameIdInGOP  = ( uiFrame << uiBaseLevel );
  SliceHeader*  pcSliceHeader   = m_pacControlData[uiFrameIdInGOP].getSliceHeader();
  UInt          uiList0Size     = pcSliceHeader->getNumRefIdxActive( LIST_0 );
  UInt          uiList1Size     = pcSliceHeader->getNumRefIdxActive( LIST_1 );

  //===== list 0 =====
  {
    for( Int iFrameId = Int( uiFrame - 1 ); iFrameId >= 0 && uiList0Size; iFrameId -= 2 )
    {
      IntFrame* pcFrame = m_papcFrame[ iFrameId << uiBaseLevel ];

      if( ! pcFrame->isExtended() )
      {
        RNOK( xFillAndExtendFrame   ( pcFrame ) );
      }

      RNOK( rcRefList0.add( pcFrame ) );
      uiList0Size--;
    }
    ROT( uiList0Size );
  }

  //===== list 1 =====
  {
    for( Int iFrameId = Int( uiFrame + 1 ); iFrameId <= ( m_uiGOPSize >> uiBaseLevel ) && uiList1Size; iFrameId += 2 )
    {
      IntFrame* pcFrame = m_papcFrame[ iFrameId << uiBaseLevel ];

      //----- create half-pel buffer -----
      if( ! pcFrame->isExtended() )
      {
        RNOK( xFillAndExtendFrame   ( pcFrame ) );
      }

      RNOK( rcRefList1.add( pcFrame ) );
      uiList1Size--;
    }
    ROT( uiList1Size );
  }

  return Err::m_nOK;
}




ErrVal  
MCTFDecoder::xGetUpdateLists( RefFrameList& rcRefList0,
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
    for( Int iFrameId = Int( uiFrame + 1 ); iFrameId <= ( m_uiGOPSize >> uiBaseLevel ) && uiList1Size; iFrameId += 2 )
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
MCTFDecoder::xCompositionStage( UInt  uiBaseLevel )
{
#if DEBUG_OUTPUT
  if( uiBaseLevel + 1 == m_uiDecompositionStages )
  {
    RNOK( xDumpFrames( "..//H264AVCEncoderLibTest//dec_rec", uiBaseLevel + 1 ) );
  }
#endif

   

  //===== CHECK IF COMPOSITION IS POSSIBLE / USEFUL =====
  {
    Bool  bOk     = true;
    UInt  uiFrame = 0;
    for(; uiFrame <= ( m_uiGOPSize >> uiBaseLevel ); uiFrame++ )
    {
      if( ! m_papcFrame[ uiFrame << uiBaseLevel ]->getRecLayer() )
      {
        bOk = false;
      }
    }
    ROFRS( bOk, Err::m_nOK );
  }



  
  //===== UPDATE =====
  Bool      bUpdate    = false;
  for( UInt uiFrameUpd = 2; uiFrameUpd <= ( m_uiGOPSize >> uiBaseLevel ); uiFrameUpd += 2 )
  {
    bUpdate                       = true;
    UInt          uiFrameIdInGOP  = uiFrameUpd << uiBaseLevel;
    ControlData&  rcControlData   = m_pacControlData[uiFrameIdInGOP];
    SliceHeader*  pcSliceHeader   = rcControlData     .getSliceHeader ();
    MbDataCtrl*   pcMbDataCtrl    = m_cControlDataUpd .getMbDataCtrl  ();
    m_cControlDataUpd.setSliceHeader( pcSliceHeader );
    IntFrame*     pcMCFrame0      = m_apcFrameTemp  [0];
    IntFrame*     pcMCFrame1      = m_apcFrameTemp  [1];
    IntFrame*     pcFrame         = m_papcFrame     [uiFrameIdInGOP];


    //===== get reference lists =====
    RefFrameList  acRefFrameList[2];
    CtrlDataList  acCtrlDataList[2];
    RNOK( xGetUpdateLists       ( acRefFrameList[0], acRefFrameList[1],
                                  acCtrlDataList[0], acCtrlDataList[1], uiBaseLevel, uiFrameUpd ) );

    if( uiFrameUpd == 2 ) printf("              -");
    printSpaces( ( 1 << ( uiBaseLevel + 1 ) ) - 1 );
    printf( acRefFrameList[0].getActive() || acRefFrameList[1].getActive() ? "U" : "-" );


    //===== list 0 motion compensation =====
    RNOK( pcMbDataCtrl->reset() );
    RNOK( pcMbDataCtrl->clear() );
    RNOK( pcMbDataCtrl->deriveUpdateMotionFieldAdaptive ( *pcSliceHeader,
                                                          &acCtrlDataList[0],
                                                          m_cConnectionArray,
                                                          m_pusUpdateWeights,
                                                          true, LIST_0 ) );
    m_pcQuarterPelFilter->setClipMode                   ( false );
    RNOK( xMotionCompensation                           ( pcMCFrame0,
                                                          &acRefFrameList[0],
                                                          &acRefFrameList[1],
                                                          pcMbDataCtrl,
                                                          *pcSliceHeader ) );
    m_pcQuarterPelFilter->setClipMode                   ( true );
    RNOK( pcMCFrame0->adaptiveWeighting                 ( m_pusUpdateWeights ) );

    
    //===== list 1 motion compensation =====
    RNOK( pcMbDataCtrl->reset() );
    RNOK( pcMbDataCtrl->clear() );
    RNOK( pcMbDataCtrl->deriveUpdateMotionFieldAdaptive ( *pcSliceHeader,
                                                          &acCtrlDataList[1],
                                                          m_cConnectionArray,
                                                          m_pusUpdateWeights,
                                                          false, LIST_1 ) );
    m_pcQuarterPelFilter->setClipMode                   ( false );
    RNOK( xMotionCompensation                           ( pcMCFrame1,
                                                          &acRefFrameList[0],
                                                          &acRefFrameList[1],
                                                          pcMbDataCtrl,
                                                          *pcSliceHeader ) );
    m_pcQuarterPelFilter->setClipMode                   ( true );
    RNOK( pcMCFrame1->adaptiveWeighting                 ( m_pusUpdateWeights ) );


    //===== inverse update =====
    RNOK( pcFrame->inverseUpdate                        ( pcMCFrame0, pcMCFrame1, pcFrame ) );


    //----- clear slice header reference -----
    m_cControlDataUpd.setSliceHeader( NULL );
  }
  if( bUpdate ) printf("\n");

  //===== clear half-pel buffers and buffer extension status =====
  RNOK( xClearBufferExtensions() );



  //===== PREDICTION =====
  Bool      bPrediction = false;
  for( UInt uiFramePrd  = 1; uiFramePrd <= ( m_uiGOPSize >> uiBaseLevel ); uiFramePrd += 2 )
  {
    if( uiFramePrd == 1 )  printf("              ");
    printSpaces( uiFramePrd == 1 ? 1 << uiBaseLevel : ( ( 1 << ( uiBaseLevel + 1 ) ) - 1 ) );
    printf("P");

    bPrediction                   = true;
    UInt          uiFrameIdInGOP  = uiFramePrd << uiBaseLevel;
    ControlData&  rcControlData   = m_pacControlData[uiFrameIdInGOP];
    IntFrame*     pcFrame         = m_papcFrame     [uiFrameIdInGOP];
    IntFrame*     pcMCFrame       = m_apcFrameTemp  [0];

    //===== get reference frame lists =====
    RefFrameList  acRefFrameList[2];
    RNOK( xGetPredictionLists         ( acRefFrameList[0], acRefFrameList[1], uiBaseLevel, uiFramePrd ) );

    //===== prediction =====
    RNOK( xMotionCompensation         ( pcMCFrame, &acRefFrameList[0], &acRefFrameList[1],
                                        rcControlData.getMbDataCtrl(), *rcControlData.getSliceHeader() ) );
    RNOK( pcFrame ->inversePrediction ( pcMCFrame, pcFrame ) );
    //pcFrame       ->clip              ();
    pcFrame       ->setBandType       ( LPS );

    //===== de-blocking =====
    RNOK( m_pcLoopFilter->process     ( *rcControlData.getSliceHeader(),
                                         pcFrame,
                                         rcControlData.getMbDataCtrl (),
                                         rcControlData.getMbDataCtrl (),
                                         m_uiFrameWidthInMb,
                                         &acRefFrameList[0],
                                         &acRefFrameList[1] ) );
  }
  if( bPrediction ) printf("\n");

  //===== clear half-pel buffers and buffer extension status =====
  RNOK( xClearBufferExtensions() );


#if DEBUG_OUTPUT
  RNOK( xDumpFrames( "..//H264AVCEncoderLibTest//dec_rec", uiBaseLevel ) );
#endif


  return Err::m_nOK;
}




ErrVal
MCTFDecoder::clearReconstructionStatus( PicBufferList&  rcPicBufferUnusedList )
{
  m_bReconstructed       = true;
  rcPicBufferUnusedList += m_cPicBufferList;
  m_cPicBufferList.clear();

  return Err::m_nOK;
}




ErrVal
MCTFDecoder::xReconstruct( PicBufferList&  rcPicBufferOutputList,
                           PicBufferList&  rcPicBufferUnusedList,
                           Int&            riMaxPoc )
{
  //===== update parameters =====
  m_eNextNalType  = ( m_eNextNalType == ALL ? LOW_PASS : m_eNextNalType );

  ROTRS( m_bReconstructed, Err::m_nOK );


  //===== loopfilter for low-pass frames / set anchor frame =====
  {
    UInt          uiFrameIdInGOP  = m_uiGOPSize;
    IntFrame*     pcFrame         = m_papcFrame     [ uiFrameIdInGOP ];
    ControlData&  rcControlData   = m_pacControlData[ uiFrameIdInGOP ];
    SliceHeader*  pcSliceHeader   = rcControlData.getSliceHeader();
    MbDataCtrl*   pcMbDataCtrl    = rcControlData.getMbDataCtrl();
    RefFrameList& rcRefFrameList0 = rcControlData.getDPCMRefFrameList( 0 );
    RefFrameList& rcRefFrameList1 = rcControlData.getDPCMRefFrameList( 1 );

    if( pcSliceHeader && pcSliceHeader->getTemporalLevel() == 0 )
    {
      RNOK( m_pcLoopFilter->process( *pcSliceHeader,
                                     pcFrame,
                                     ( pcSliceHeader->isIntra() ? NULL : pcMbDataCtrl ),
                                     pcMbDataCtrl,
                                     m_uiFrameWidthInMb,
                                     &rcRefFrameList0,
                                     &rcRefFrameList1 ) );

      RNOK( m_pcAnchorFrame->copyAll ( pcFrame ) );
      m_pcAnchorFrame->setValid   ();
    }
    else
    { 
      m_pcAnchorFrame->setUnvalid ();
    }
  }


  //===== reconstruction =====
  {
    printf("\nRECONSTRUCTION:\n");
    UInt   uiCurrStage = m_uiDecompositionStages;
    while( uiCurrStage-- > 0 )
    {
      RNOK( xCompositionStage( uiCurrStage ) );
    }
    printf("\n");
  }
#if DEBUG_OUTPUT
  //===== just debug output for first frame =====
  if( ! m_uiDecompositionStages )
  {
    for( UInt uiBaseLevel = 0; uiBaseLevel <= MAX_DSTAGES; uiBaseLevel++ )
    {
      RNOK( xDumpFrames( "..//H264AVCEncoderLibTest//dec_rec", uiBaseLevel ) );
    }
  }
#endif


  //===== copy to output buffer list =====
  riMaxPoc = MSYS_INT_MIN;
  for( UInt uiFrame = 1; uiFrame <= m_uiGOPSize; uiFrame++ )
  {
    if( m_papcFrame[uiFrame]->getRecLayer() && 
        m_papcFrame[uiFrame]->getBandType() == LPS )
    {
      ROT( m_cPicBufferList.empty() );

      PicBuffer*  pcPicBuffer = m_cPicBufferList.popFront();

      m_papcFrame[uiFrame]->store( pcPicBuffer );

      rcPicBufferOutputList.push_back( pcPicBuffer );
      rcPicBufferUnusedList.push_back( pcPicBuffer );

      if( m_pacControlData[uiFrame].getSliceHeader()->getPoc() > riMaxPoc )
      {
        riMaxPoc = m_pacControlData[uiFrame].getSliceHeader()->getPoc();
      }
    }
  }

  //===== clear status =====
#if ! UNRESTRICTED_INTER_LAYER_PREDICTION

  RNOK( m_pcH264AVCDecoder->clearReconstructionStatus( riMaxPoc, rcPicBufferUnusedList ) );

#else

  m_bReconstructed       = true;
  rcPicBufferUnusedList += m_cPicBufferList;
  m_cPicBufferList.clear();

#endif

  return Err::m_nOK;
}





ErrVal MCTFDecoder::xFillAndExtendFrame( IntFrame* rcFrame )
{
  RNOK( m_pcYuvFullPelBufferCtrl->initMb() );

  RNOK( rcFrame->extendFrame( NULL ) );

  return Err::m_nOK;
}





ErrVal
MCTFDecoder::xMotionCompensation( IntFrame*        pcMCFrame,
                                 RefFrameList*    pcRefFrameList0,
                                 RefFrameList*    pcRefFrameList1,
                                 MbDataCtrl*      pcMbDataCtrl,
                                 SliceHeader&     rcSH )
{
  Bool            bCalcMv         = false;
  Bool            bFaultTolerant  = false;
  MbDecoder*      pcMbDecoder     = m_pcSliceDecoder->getMbDecoder();

  RNOK( pcMbDataCtrl          ->initSlice( rcSH, PRE_PROCESS, true, NULL ) );
  RNOK( m_pcMotionCompensation->initSlice( rcSH              ) );

  for( UInt uiMbIndex = 0; uiMbIndex < m_uiMbNumber; uiMbIndex++ )
  {
    UInt          uiMbY           = uiMbIndex / m_uiFrameWidthInMb;
    UInt          uiMbX           = uiMbIndex % m_uiFrameWidthInMb;
    MbDataAccess* pcMbDataAccess  = 0;


    RNOK( pcMbDataCtrl            ->initMb( pcMbDataAccess, uiMbY, uiMbX                  ) );
    RNOK( m_pcYuvFullPelBufferCtrl->initMb(                 uiMbY, uiMbX ) );
    RNOK( m_pcMotionCompensation  ->initMb(                 uiMbY, uiMbX, *pcMbDataAccess ) );

    RNOK( pcMbDecoder->compensatePrediction( *pcMbDataAccess, pcMCFrame,
                                             *pcRefFrameList0, *pcRefFrameList1,
                                             bCalcMv, bFaultTolerant ) );
  }

  return Err::m_nOK;
}






ErrVal
MCTFDecoder::xCalcMv( SliceHeader*  pcSliceHeader,
                      MbDataCtrl*   pcMbDataCtrl,
                      MbDataCtrl*   pcMbDataCtrlBase )
{
  UInt        uiMbNumber  = m_uiFrameWidthInMb * m_uiFrameHeightInMb;
  MbDecoder*  pcMbDecoder = m_pcSliceDecoder->getMbDecoder();

  RNOK( pcMbDataCtrl          ->initSlice( *pcSliceHeader, PRE_PROCESS, true, NULL ) );
  RNOK( m_pcMotionCompensation->initSlice( *pcSliceHeader              ) );

  for( UInt uiMbIndex = 0; uiMbIndex < uiMbNumber; uiMbIndex++ )
  {
    UInt          uiMbY           = uiMbIndex / m_uiFrameWidthInMb;
    UInt          uiMbX           = uiMbIndex % m_uiFrameWidthInMb;
    MbDataAccess* pcMbDataAccess  = 0;
    MbDataAccess* pcMbDataAccessBase  = 0;

    if( pcMbDataCtrlBase && pcSliceHeader->getAdaptivePredictionFlag() )
    {
      RNOK( pcMbDataCtrlBase->initMb( pcMbDataAccessBase, uiMbY, uiMbX ) );
    }

    //===== init macroblock =====
    RNOK( pcMbDataCtrl          ->initMb( pcMbDataAccess, uiMbY, uiMbX                  ) );
    RNOK( m_pcMotionCompensation->initMb(                 uiMbY, uiMbX, *pcMbDataAccess ) );

    //===== motion compensation for macroblock =====
    RNOK( pcMbDecoder->calcMv( *pcMbDataAccess, pcMbDataAccessBase ) );
  }

  return Err::m_nOK;
}




H264AVC_NAMESPACE_END

