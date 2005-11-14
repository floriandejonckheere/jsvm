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
#include "H264AVCEncoder.h"
#include "H264AVCCommonLib/MbData.h"
#include "H264AVCCommonLib/Frame.h"
#include "H264AVCCommonLib/FrameMng.h"
#include "BitWriteBuffer.h"
#include "H264AVCCommonLib/Transform.h"
#include "H264AVCCommonLib/YuvBufferCtrl.h"
#include "H264AVCCommonLib/QuarterPelFilter.h"
#include "H264AVCCommonLib/ParameterSetMng.h"
#include "H264AVCCommonLib/LoopFilter.h"
#include "H264AVCCommonLib/SampleWeighting.h"
#include "H264AVCCommonLib/PocCalculator.h"
#include "H264AVCCommonLib/ReconstructionBypass.h"
#include "SliceEncoder.h"
#include "UvlcWriter.h"
#include "MbCoder.h"
#include "MbEncoder.h"
#include "IntraPredictionSearch.h"
#include "CodingParameter.h"
#include "CabacWriter.h"
#include "NalUnitEncoder.h"
#include "Distortion.h"
#include "MotionEstimation.h"
#include "MotionEstimationQuarterPel.h"
#include "RateDistortion.h"
#include "GOPEncoder.h"
#include "ControlMngH264AVCEncoder.h"
#include "CreaterH264AVCEncoder.h"
#include "H264AVCCommonLib/TraceFile.h"
#include "FGSSubbandEncoder.h"



H264AVC_NAMESPACE_BEGIN



CreaterH264AVCEncoder::CreaterH264AVCEncoder():
  m_pcH264AVCEncoder      ( NULL ), 
  m_pcFrameMng            ( NULL ),
  m_pcSliceEncoder        ( NULL ),
  m_pcControlMng          ( NULL ),
  m_pcBitWriteBuffer      ( NULL ),
  m_pcNalUnitEncoder      ( NULL ),
  m_pcUvlcWriter          ( NULL ),
  m_pcUvlcTester          ( NULL ),
  m_pcMbCoder             ( NULL ),
  m_pcLoopFilter          ( NULL ),
  m_pcMbEncoder           ( NULL ),
  m_pcQuarterPelFilter    ( NULL ),
  m_pcCodingParameter     ( NULL ),
  m_pcParameterSetMng     ( NULL ),
  m_pcSampleWeighting     ( NULL ),
  m_pcCabacWriter         ( NULL ),
  m_pcXDistortion         ( NULL ),
  m_pcMotionEstimation    ( NULL ),
  m_pcRateDistortion      ( NULL ),
  m_pcHistory             ( NULL ),
  m_pcRQFGSEncoder        ( NULL ),
  m_bTraceEnable          ( true )
{
  ::memset( m_apcYuvFullPelBufferCtrl, 0x00, MAX_LAYERS*sizeof(Void*) );
  ::memset( m_apcYuvHalfPelBufferCtrl, 0x00, MAX_LAYERS*sizeof(Void*) );
  ::memset( m_apcPocCalculator,        0x00, MAX_LAYERS*sizeof(Void*) );
  ::memset( m_apcMCTFEncoder,          0x00, MAX_LAYERS*sizeof(Void*) );
  m_pcReconstructionBypass = NULL;
}


CreaterH264AVCEncoder::~CreaterH264AVCEncoder()
{

}

Bool
CreaterH264AVCEncoder::getScalableSeiMessage()
{
	return m_pcH264AVCEncoder->bGetScalableSeiMessage();
}

Void
CreaterH264AVCEncoder::SetVeryFirstCall()
{ 
	m_pcH264AVCEncoder->SetVeryFirstCall(); 
}

ErrVal
CreaterH264AVCEncoder::writeParameterSets( ExtBinDataAccessor* pcExtBinDataAccessor,
                                           Bool&               rbMoreSets )
{
  RNOK( m_pcH264AVCEncoder->writeParameterSets( pcExtBinDataAccessor, rbMoreSets ) );
  return Err::m_nOK;
}

//{{Quality level estimation and modified truncation- JVTO044 and m12007
//France Telecom R&D-(nathalie.cammas@francetelecom.com)
ErrVal CreaterH264AVCEncoder::writeQualityLevelInfosSEI(ExtBinDataAccessor* pcExtBinDataAccessor,UInt* uiaQualityLevel, UInt *uiaDelta,UInt uiNumLevels, UInt uiLayer)
{
	RNOK( m_pcH264AVCEncoder->writeQualityLevelInfosSEI( pcExtBinDataAccessor, uiaQualityLevel,uiaDelta, uiNumLevels, uiLayer ) );
  return Err::m_nOK;
}
ErrVal CreaterH264AVCEncoder::writeDeadSubstreamSEI(ExtBinDataAccessor* pcExtBinDataAccessor,UInt uiDeltaBytesDS, UInt uiLayer)
{
	RNOK( m_pcH264AVCEncoder->writeDeadSubstreamSEI( pcExtBinDataAccessor, uiDeltaBytesDS,uiLayer ) );
  return Err::m_nOK;
}
//}}Quality level estimation and modified truncation- JVTO044 and m12007

ErrVal
CreaterH264AVCEncoder::process( ExtBinDataAccessorList&  rcExtBinDataAccessorList, 
                                PicBuffer*               apcOriginalPicBuffer     [MAX_LAYERS],
                                PicBuffer*               apcReconstructPicBuffer  [MAX_LAYERS],
                                PicBufferList*           apcPicBufferOutputList,
                                PicBufferList*           apcPicBufferUnusedList )
{
  RNOK( m_pcH264AVCEncoder->process( rcExtBinDataAccessorList, 
                                     apcOriginalPicBuffer, 
                                     apcReconstructPicBuffer, 
                                     apcPicBufferOutputList, 
                                     apcPicBufferUnusedList ) );
  return Err::m_nOK;
}


ErrVal
CreaterH264AVCEncoder::finish ( ExtBinDataAccessorList&  rcExtBinDataAccessorList, 
                                PicBufferList*           apcPicBufferOutputList,
                                PicBufferList*           apcPicBufferUnusedList,
                                UInt&                    ruiNumCodedFrames,
                                Double&                  rdHighestLayerOutputRate )
{
  RNOK( m_pcH264AVCEncoder->finish( rcExtBinDataAccessorList, 
                                    apcPicBufferOutputList, 
                                    apcPicBufferUnusedList,
                                    ruiNumCodedFrames,
                                    rdHighestLayerOutputRate ) );
  return Err::m_nOK;
}


ErrVal
CreaterH264AVCEncoder::create( CreaterH264AVCEncoder*& rpcCreaterH264AVCEncoder )
{
  rpcCreaterH264AVCEncoder = new CreaterH264AVCEncoder;
  ROT( NULL == rpcCreaterH264AVCEncoder );

  RNOK( rpcCreaterH264AVCEncoder->xCreateEncoder() );

  return Err::m_nOK;
}


ErrVal
CreaterH264AVCEncoder::xCreateEncoder()
{
  RNOK( ParameterSetMng             ::create( m_pcParameterSetMng ) );
  RNOK( FrameMng                    ::create( m_pcFrameMng ) );
  RNOK( BitWriteBuffer              ::create( m_pcBitWriteBuffer ) );
  RNOK( BitCounter                  ::create( m_pcBitCounter ) );
  RNOK( NalUnitEncoder              ::create( m_pcNalUnitEncoder) );
  RNOK( SliceEncoder                ::create( m_pcSliceEncoder ) );
  RNOK( UvlcWriter                  ::create( m_pcUvlcWriter ) );
  RNOK( UvlcWriter                  ::create( m_pcUvlcTester, false ) );
  RNOK( CabacWriter                 ::create( m_pcCabacWriter ) );
  RNOK( MbCoder                     ::create( m_pcMbCoder ) );
  RNOK( MbEncoder                   ::create( m_pcMbEncoder ) );
  RNOK( LoopFilter                  ::create( m_pcLoopFilter ) );
  RNOK( IntraPredictionSearch       ::create( m_pcIntraPrediction ) );
  RNOK( MotionEstimationQuarterPel  ::create( m_pcMotionEstimation ) );
  RNOK( H264AVCEncoder              ::create( m_pcH264AVCEncoder ) );
  RNOK( ControlMngH264AVCEncoder    ::create( m_pcControlMng ) );
  RNOK( ReconstructionBypass        ::create( m_pcReconstructionBypass ) );
  RNOK( QuarterPelFilter            ::create( m_pcQuarterPelFilter ) );
  RNOK( Transform                   ::create( m_pcTransform ) );
  RNOK( SampleWeighting             ::create( m_pcSampleWeighting ) );
  RNOK( XDistortion                 ::create( m_pcXDistortion ) );
  RNOK( RQFGSEncoder                ::create( m_pcRQFGSEncoder ) );

  for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
  {
    RNOK( MCTFEncoder   ::create( m_apcMCTFEncoder         [uiLayer] ) );
    RNOK( PocCalculator ::create( m_apcPocCalculator       [uiLayer] ) );
    RNOK( YuvBufferCtrl ::create( m_apcYuvFullPelBufferCtrl[uiLayer] ) );
    RNOK( YuvBufferCtrl ::create( m_apcYuvHalfPelBufferCtrl[uiLayer] ) );
  }

  return Err::m_nOK;
}


ErrVal
CreaterH264AVCEncoder::destroy()
{
  RNOK( m_pcFrameMng              ->destroy() );
  RNOK( m_pcSliceEncoder          ->destroy() );
  RNOK( m_pcBitWriteBuffer        ->destroy() );
  RNOK( m_pcBitCounter            ->destroy() );
  RNOK( m_pcNalUnitEncoder        ->destroy() );
  RNOK( m_pcUvlcWriter            ->destroy() );
  RNOK( m_pcUvlcTester            ->destroy() );
  RNOK( m_pcMbCoder               ->destroy() );
  RNOK( m_pcLoopFilter            ->destroy() );
  RNOK( m_pcMbEncoder             ->destroy() );
  RNOK( m_pcTransform             ->destroy() );
  RNOK( m_pcIntraPrediction       ->destroy() );
  RNOK( m_pcQuarterPelFilter      ->destroy() );
  RNOK( m_pcCabacWriter           ->destroy() );
  RNOK( m_pcXDistortion           ->destroy() );
  RNOK( m_pcMotionEstimation      ->destroy() );
  RNOK( m_pcSampleWeighting       ->destroy() );
  RNOK( m_pcParameterSetMng       ->destroy() );
  RNOK( m_pcH264AVCEncoder        ->destroy() );
  RNOK( m_pcControlMng            ->destroy() );
  RNOK( m_pcReconstructionBypass  ->destroy() );
  RNOK( m_pcRQFGSEncoder          ->destroy() );

  if( NULL != m_pcRateDistortion )
  {
    RNOK( m_pcRateDistortion      ->destroy() );
  }

  for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
  {
    RNOK( m_apcMCTFEncoder          [uiLayer] ->destroy() );
    RNOK( m_apcYuvFullPelBufferCtrl [uiLayer] ->destroy() );
    RNOK( m_apcYuvHalfPelBufferCtrl [uiLayer] ->destroy() );
    RNOK( m_apcPocCalculator        [uiLayer] ->destroy() );
  }

  delete this;

  return Err::m_nOK;
}


ErrVal
CreaterH264AVCEncoder::init( CodingParameter* pcCodingParameter )
{
  INIT_ETRACE;
  OPEN_ETRACE;
  ROT( NULL == pcCodingParameter);

  m_pcCodingParameter = pcCodingParameter;

  RNOK( RateDistortion::create( m_pcRateDistortion ) );

  RNOK( m_pcBitWriteBuffer          ->init() );
  RNOK( m_pcBitCounter              ->init() );
  RNOK( m_pcXDistortion             ->init() );
  RNOK( m_pcSampleWeighting         ->init() );
  RNOK( m_pcNalUnitEncoder          ->init( m_pcBitWriteBuffer,
                                            m_pcUvlcWriter,
                                            m_pcUvlcTester ));
  RNOK( m_pcUvlcWriter              ->init( m_pcBitWriteBuffer ) );
  RNOK( m_pcUvlcTester              ->init( m_pcBitCounter ) );
  RNOK( m_pcCabacWriter             ->init( m_pcBitWriteBuffer ) );
  RNOK( m_pcParameterSetMng         ->init() );
  RNOK( m_pcFrameMng                ->init( m_apcYuvFullPelBufferCtrl[0],
                                            m_apcYuvHalfPelBufferCtrl[0],
                                            m_pcQuarterPelFilter ) );

  RNOK( m_pcSliceEncoder            ->init( m_pcMbEncoder,
                                            m_pcMbCoder,
                                            m_pcControlMng,
                                            m_pcCodingParameter,
                                            m_apcPocCalculator[0],
                                            m_pcTransform ) );
  RNOK( m_pcReconstructionBypass    ->init() );
  RNOK( m_pcLoopFilter              ->init( m_pcControlMng,
                                            m_pcReconstructionBypass ) );
  RNOK( m_pcRateDistortion          ->init( pcCodingParameter ) );
  RNOK( m_pcQuarterPelFilter        ->init() );

  RNOK( m_pcMbEncoder               ->init( m_pcTransform,
                                            m_pcIntraPrediction,
                                            m_pcMotionEstimation,
                                            m_pcCodingParameter,
                                            m_pcRateDistortion,
                                            m_pcFrameMng,
                                            m_pcXDistortion ) );
  RNOK( m_pcMotionEstimation        ->init( m_pcXDistortion,
                                            m_pcCodingParameter,
                                            m_pcRateDistortion,
                                            m_pcQuarterPelFilter,
                                            m_pcTransform,
                                            m_pcSampleWeighting) );

  RNOK( m_pcControlMng              ->init( m_pcFrameMng,
                                            m_apcMCTFEncoder,
                                            m_pcSliceEncoder,
                                            m_pcControlMng,
                                            m_pcBitWriteBuffer,
                                            m_pcBitCounter,
                                            m_pcNalUnitEncoder,
                                            m_pcUvlcWriter,
                                            m_pcUvlcTester,
                                            m_pcMbCoder,
                                            m_pcLoopFilter,
                                            m_pcMbEncoder,
                                            m_pcTransform,
                                            m_pcIntraPrediction,
                                            m_apcYuvFullPelBufferCtrl,
                                            m_apcYuvHalfPelBufferCtrl,
                                            m_pcQuarterPelFilter,
                                            m_pcCodingParameter,
                                            m_pcParameterSetMng,
                                            m_apcPocCalculator,
                                            m_pcSampleWeighting,
                                            m_pcCabacWriter,
                                            m_pcXDistortion,
                                            m_pcMotionEstimation,
                                            m_pcRateDistortion ) );
  
  RNOK( m_pcH264AVCEncoder          ->init( m_apcMCTFEncoder,
                                            m_pcParameterSetMng,
                                            m_apcPocCalculator[0],
                                            m_pcNalUnitEncoder,
                                            m_pcControlMng,
                                            pcCodingParameter,
                                            m_pcFrameMng ) );

  RNOK( m_pcRQFGSEncoder            ->init( m_apcYuvFullPelBufferCtrl,
                                            m_pcTransform,
                                            m_pcControlMng,
                                            m_pcMbEncoder ) );

  for( UInt uiLayer = 0; uiLayer < m_pcCodingParameter->getNumberOfLayers(); uiLayer++ )
  {
    RNOK( m_apcMCTFEncoder[uiLayer]->init( m_pcCodingParameter,
                                          &m_pcCodingParameter->getLayerParameters(uiLayer),
                                           m_pcH264AVCEncoder,
                                           m_pcSliceEncoder,
                                           m_pcRQFGSEncoder,
                                           m_pcLoopFilter,
                                           m_apcPocCalculator        [uiLayer],
                                           m_pcNalUnitEncoder,
                                           m_apcYuvFullPelBufferCtrl [uiLayer],
                                           m_apcYuvHalfPelBufferCtrl [uiLayer],
                                           m_pcQuarterPelFilter,
                                           m_pcMotionEstimation ) );
  }

  return Err::m_nOK;
}


ErrVal
CreaterH264AVCEncoder::uninit()
{
  RNOK( m_pcQuarterPelFilter      ->uninit() );
  RNOK( m_pcSampleWeighting       ->uninit() );
  RNOK( m_pcFrameMng              ->uninit() );
  RNOK( m_pcParameterSetMng       ->uninit() );
  RNOK( m_pcSliceEncoder          ->uninit() );
  RNOK( m_pcNalUnitEncoder        ->uninit() );
  RNOK( m_pcBitWriteBuffer        ->uninit() );
  RNOK( m_pcBitCounter            ->uninit() );
  RNOK( m_pcUvlcWriter            ->uninit() );
  RNOK( m_pcUvlcTester            ->uninit() );
  RNOK( m_pcMbCoder               ->uninit() );
  RNOK( m_pcLoopFilter            ->uninit() );
  RNOK( m_pcMbEncoder             ->uninit() );
  RNOK( m_pcIntraPrediction       ->uninit() );
  RNOK( m_pcMotionEstimation      ->uninit() );
  RNOK( m_pcCabacWriter           ->uninit() );
  RNOK( m_pcMotionEstimation      ->uninit() );
  RNOK( m_pcXDistortion           ->uninit() );
  RNOK( m_pcH264AVCEncoder        ->uninit() );
  RNOK( m_pcControlMng            ->uninit() );
  RNOK( m_pcReconstructionBypass  ->uninit() );
  RNOK( m_pcRQFGSEncoder          ->uninit() );

  if( NULL != m_pcRateDistortion )
  {
    RNOK( m_pcRateDistortion      ->uninit() );
  }

  for( UInt uiLayer = 0; uiLayer < m_pcCodingParameter->getNumberOfLayers(); uiLayer++ )
  {
    RNOK( m_apcMCTFEncoder         [uiLayer] ->uninit() );
    RNOK( m_apcYuvFullPelBufferCtrl[uiLayer] ->uninit() );
    RNOK( m_apcYuvHalfPelBufferCtrl[uiLayer] ->uninit() );
  }

  CLOSE_ETRACE;

  return Err::m_nOK;
}




H264AVC_NAMESPACE_END
