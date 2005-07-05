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
#include "ControlMngH264AVCDecoder.h"


H264AVC_NAMESPACE_BEGIN

ControlMngH264AVCDecoder::ControlMngH264AVCDecoder():
  m_pcMbDataCtrl          ( NULL ),
  m_pcFrameMng            ( NULL ),
  m_pcParameterSetMng     ( NULL ),
  m_pcSliceReader         ( NULL ),
  m_pcNalUnitParser       ( NULL ),
  m_pcSliceDecoder        ( NULL ),
  m_pcControlMng          ( NULL ),
  m_pcBitReadBuffer       ( NULL ),
  m_pcUvlcReader          ( NULL ),
  m_pcMbParser            ( NULL ),
  m_pcLoopFilter          ( NULL ),
  m_pcMbDecoder           ( NULL ),
  m_pcTransform           ( NULL ),
  m_pcIntraPrediction     ( NULL ),
  m_pcMotionCompensation  ( NULL ),
  m_pcQuarterPelFilter    ( NULL ),
  m_pcCabacReader         ( NULL ),
  m_pcSampleWeighting     ( NULL ),
  m_uiCurrLayer           ( MSYS_UINT_MAX ),
  m_bLayer0IsAVC          ( true )
{
  for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
  {
    m_auiMbXinFrame           [uiLayer] = 0;
    m_auiMbYinFrame           [uiLayer] = 0;
    m_apcMCTFDecoder          [uiLayer] = NULL;
    m_apcPocCalculator        [uiLayer] = NULL;
    m_apcYuvFullPelBufferCtrl [uiLayer] = NULL;
    m_uiInitilized            [uiLayer] = false;
  }
}



ControlMngH264AVCDecoder::~ControlMngH264AVCDecoder()
{
}




ErrVal
ControlMngH264AVCDecoder::init( FrameMng*            pcFrameMng,
                             ParameterSetMng*     pcParameterSetMng,
                             PocCalculator*       apcPocCalculator        [MAX_LAYERS],
                             SliceReader*         pcSliceReader,
                             NalUnitParser*       pcNalUnitParser,
                             SliceDecoder*        pcSliceDecoder,
                             BitReadBuffer*       pcBitReadBuffer,
                             UvlcReader*          pcUvlcReader,
                             MbParser*            pcMbParser,
                             LoopFilter*          pcLoopFilter,
                             MbDecoder*           pcMbDecoder,
                             Transform*           pcTransform,
                             IntraPrediction*     pcIntraPrediction,
                             MotionCompensation*  pcMotionCompensation,
                             YuvBufferCtrl*       apcYuvFullPelBufferCtrl [MAX_LAYERS],
                             QuarterPelFilter*    pcQuarterPelFilter,
                             CabacReader*         pcCabacReader,
                             SampleWeighting*     pcSampleWeighting,
                             MCTFDecoder*         apcMCTFDecoder          [MAX_LAYERS],
                             H264AVCDecoder*         pcH264AVCDecoder )
{ 

  ROT( NULL == pcFrameMng );
  ROT( NULL == pcParameterSetMng );
  ROT( NULL == pcSliceReader );
  ROT( NULL == pcNalUnitParser );
  ROT( NULL == pcSliceDecoder );
  ROT( NULL == pcBitReadBuffer );
  ROT( NULL == pcUvlcReader );
  ROT( NULL == pcMbParser );
  ROT( NULL == pcLoopFilter );
  ROT( NULL == pcMbDecoder );
  ROT( NULL == pcTransform );
  ROT( NULL == pcIntraPrediction );
  ROT( NULL == pcMotionCompensation );
  ROT( NULL == pcQuarterPelFilter );
  ROT( NULL == pcCabacReader );
  ROT( NULL == pcSampleWeighting );
  ROT( NULL == pcH264AVCDecoder );

  m_bLayer0IsAVC          = true;
  m_uiCurrLayer           = MSYS_UINT_MAX;
  m_pcFrameMng            = pcFrameMng; 
  m_pcParameterSetMng     = pcParameterSetMng;
  m_pcSliceReader         = pcSliceReader; 
  m_pcNalUnitParser       = pcNalUnitParser; 
  m_pcSliceDecoder        = pcSliceDecoder; 
  m_pcBitReadBuffer       = pcBitReadBuffer; 
  m_pcUvlcReader          = pcUvlcReader; 
  m_pcMbParser            = pcMbParser; 
  m_pcLoopFilter          = pcLoopFilter; 
  m_pcMbDecoder           = pcMbDecoder; 
  m_pcTransform           = pcTransform; 
  m_pcIntraPrediction     = pcIntraPrediction; 
  m_pcMotionCompensation  = pcMotionCompensation; 
  m_pcQuarterPelFilter    = pcQuarterPelFilter; 
  m_pcCabacReader         = pcCabacReader; 
  m_pcSampleWeighting     = pcSampleWeighting; 
  m_pcH264AVCDecoder         = pcH264AVCDecoder;

  ROT( NULL == apcMCTFDecoder );
  ROT( NULL == apcPocCalculator );
  ROT( NULL == apcYuvFullPelBufferCtrl );

  for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
  {
    ROT( NULL == apcMCTFDecoder         [uiLayer] );
    ROT( NULL == apcPocCalculator       [uiLayer] );
    ROT( NULL == apcYuvFullPelBufferCtrl[uiLayer] );

    m_apcMCTFDecoder          [uiLayer] = apcMCTFDecoder          [uiLayer];
    m_apcPocCalculator        [uiLayer] = apcPocCalculator        [uiLayer];
    m_apcYuvFullPelBufferCtrl [uiLayer] = apcYuvFullPelBufferCtrl [uiLayer];
  }
  
  return Err::m_nOK;
}




ErrVal ControlMngH264AVCDecoder::uninit()
{
  return Err::m_nOK;
}



ErrVal ControlMngH264AVCDecoder::create( ControlMngH264AVCDecoder*& rpcControlMngH264AVCDecoder )
{
  rpcControlMngH264AVCDecoder = new ControlMngH264AVCDecoder;

  ROT( NULL == rpcControlMngH264AVCDecoder );
  return Err::m_nOK;
}




ErrVal ControlMngH264AVCDecoder::destroy()
{
  delete this;
  return Err::m_nOK;
}



ErrVal ControlMngH264AVCDecoder::initMbForParsing( MbDataAccess*& rpcMbDataAccess, UInt uiMbIndex )
{
  ROF( m_uiCurrLayer < MAX_LAYERS );
  
  UInt uiMbY, uiMbX;

  uiMbY = uiMbIndex         / m_auiMbXinFrame[m_uiCurrLayer];
  uiMbX = uiMbIndex - uiMbY * m_auiMbXinFrame[m_uiCurrLayer];

  RNOK( m_pcMbDataCtrl                          ->initMb( rpcMbDataAccess, uiMbY, uiMbX ) );
  RNOK( m_apcYuvFullPelBufferCtrl[m_uiCurrLayer]->initMb(                  uiMbY, uiMbX ) );

  return Err::m_nOK;
}


ErrVal ControlMngH264AVCDecoder::initMbForDecoding( MbDataAccess*& rpcMbDataAccess, UInt uiMbIndex )
{
  ROF( m_uiCurrLayer < MAX_LAYERS );
  
  UInt uiMbY, uiMbX;

  uiMbY = uiMbIndex         / m_auiMbXinFrame[m_uiCurrLayer];
  uiMbX = uiMbIndex - uiMbY * m_auiMbXinFrame[m_uiCurrLayer];

  RNOK( m_pcMbDataCtrl                          ->initMb( rpcMbDataAccess, uiMbY, uiMbX                   ) );
  RNOK( m_apcYuvFullPelBufferCtrl[m_uiCurrLayer]->initMb(                  uiMbY, uiMbX ) );
  RNOK( m_pcMotionCompensation                  ->initMb(                  uiMbY, uiMbX, *rpcMbDataAccess ) ) ;

  return Err::m_nOK;
}



ErrVal ControlMngH264AVCDecoder::initMbForDecoding( MbDataAccess& rcMbDataAccess, UInt uiMbIndex )
{
  ROF( m_uiCurrLayer < MAX_LAYERS );
  
  UInt uiMbY, uiMbX;

  uiMbY = uiMbIndex         / m_auiMbXinFrame[m_uiCurrLayer];
  uiMbX = uiMbIndex - uiMbY * m_auiMbXinFrame[m_uiCurrLayer];

  RNOK( m_apcYuvFullPelBufferCtrl[m_uiCurrLayer]->initMb( uiMbY, uiMbX ) );
  RNOK( m_pcMotionCompensation                  ->initMb( uiMbY, uiMbX, rcMbDataAccess ) ) ;

  return Err::m_nOK;
}


ErrVal ControlMngH264AVCDecoder::initMbForFiltering( MbDataAccess*& rpcMbDataAccess, UInt uiMbIndex )
{
  ROF( m_uiCurrLayer < MAX_LAYERS );
  
  UInt uiMbY, uiMbX;

  uiMbY = uiMbIndex         / m_auiMbXinFrame[m_uiCurrLayer];
  uiMbX = uiMbIndex - uiMbY * m_auiMbXinFrame[m_uiCurrLayer];

  RNOK( m_pcMbDataCtrl                          ->initMb( rpcMbDataAccess, uiMbY, uiMbX           ) );
  RNOK( m_apcYuvFullPelBufferCtrl[m_uiCurrLayer]->initMb(                  uiMbY, uiMbX ) );

  return Err::m_nOK;
}



ErrVal ControlMngH264AVCDecoder::initMbForFiltering( MbDataAccess& rcMbDataAccess, UInt uiMbIndex )
{
  ROF( m_uiCurrLayer < MAX_LAYERS );
  
  UInt uiMbY, uiMbX;

  uiMbY = uiMbIndex         / m_auiMbXinFrame[m_uiCurrLayer];
  uiMbX = uiMbIndex - uiMbY * m_auiMbXinFrame[m_uiCurrLayer];

  RNOK( m_apcYuvFullPelBufferCtrl[m_uiCurrLayer]->initMb( uiMbY, uiMbX ) );

  return Err::m_nOK;
}


ErrVal ControlMngH264AVCDecoder::initSlice0(SliceHeader *rcSH)
{
  UInt  uiLayer             = rcSH->getLayerId                    ();
  ROTRS( m_uiInitilized[uiLayer], Err::m_nOK );
  m_auiMbXinFrame[uiLayer]  = rcSH->getSPS().getFrameWidthInMbs   ();
  m_auiMbYinFrame[uiLayer]  = rcSH->getSPS().getFrameHeightInMbs  ();

  UInt uiSizeX = m_auiMbXinFrame  [uiLayer] << 4;
  UInt uiSizeY = m_auiMbYinFrame  [uiLayer] << 4;

  RNOK( m_apcYuvFullPelBufferCtrl [uiLayer]->initSlice( uiSizeY, uiSizeX, YUV_Y_MARGIN, YUV_X_MARGIN ) );

  if( uiLayer == 0 )
  {
    m_bLayer0IsAVC  = ( rcSH->getNalUnitType() == NAL_UNIT_CODED_SLICE || 
                        rcSH->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR);
    
    RNOK( m_pcFrameMng->initSlice               ( rcSH ) );
    m_pcH264AVCDecoder->setReconstructionLayerId( uiLayer );
    m_pcH264AVCDecoder->setBaseAVCCompatible    ( m_bLayer0IsAVC );
  }
  else
  {
    m_pcH264AVCDecoder->setReconstructionLayerId( uiLayer );
  }

  if( rcSH->getNalUnitType() == NAL_UNIT_CODED_SLICE_SCALABLE ||  
      rcSH->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR_SCALABLE)
  {
      m_apcMCTFDecoder[uiLayer]->initSlice0( rcSH );
  }

  m_uiInitilized[uiLayer] = true;

  return Err::m_nOK;
}


// TMM_ESS {
ErrVal ControlMngH264AVCDecoder::initSPS( SequenceParameterSet& rcSequenceParameterSet, UInt  uiLayer )
{
  m_auiMbXinFrame[uiLayer]  = rcSequenceParameterSet.getFrameWidthInMbs   ();
  m_auiMbYinFrame[uiLayer]  = rcSequenceParameterSet.getFrameHeightInMbs  ();

  rcSequenceParameterSet.getResizeParameters(&m_ResizeParameter[uiLayer]);

  if (uiLayer != 0)
    {
      ResizeParameters * curr = &m_ResizeParameter[uiLayer];
      curr->m_iInWidth  = m_auiMbXinFrame  [uiLayer-1] << 4;
      curr->m_iInHeight = m_auiMbYinFrame  [uiLayer-1] << 4;

      bool is_crop_aligned = (curr->m_iPosX%16 == 0) && (curr->m_iPosY%16 == 0);
      if      ((curr->m_iInWidth == curr->m_iOutWidth) && (curr->m_iInHeight == curr->m_iOutHeight) &&
               is_crop_aligned && (curr->m_iExtendedSpatialScalability < ESS_PICT) )
        curr->m_iSpatialScalabilityType = SST_RATIO_1;
      else if ((curr->m_iInWidth*2 == curr->m_iOutWidth) && (curr->m_iInHeight*2 == curr->m_iOutHeight) &&
               is_crop_aligned && (curr->m_iExtendedSpatialScalability < ESS_PICT) )
        curr->m_iSpatialScalabilityType = SST_RATIO_2;
      else if ((curr->m_iInWidth*3 == curr->m_iOutWidth*2) && (curr->m_iInHeight*3 == curr->m_iOutHeight*2) &&
               is_crop_aligned && (curr->m_iExtendedSpatialScalability < ESS_PICT) )
        curr->m_iSpatialScalabilityType = SST_RATIO_3_2;
      else
        curr->m_iSpatialScalabilityType = SST_RATIO_X;

      if ( curr->m_iExtendedSpatialScalability == ESS_NONE && curr->m_iSpatialScalabilityType > SST_RATIO_2 )
      {
        printf("\nControlMngH264AVCDecoder::initSPS() - use of Extended Spatial Scalability not signaled\n");
        return Err::m_nERR;
      }
      //end 

      m_apcMCTFDecoder[uiLayer]->setResizeParameters(&m_ResizeParameter[uiLayer]);

      if (curr->m_iExtendedSpatialScalability == ESS_SEQ)
      {
        printf("Extended Spatial Scalability - crop win: origin=(%3d,%3d) - size=(%3d,%3d)\n\n",
               curr->m_iPosX,curr->m_iPosY,curr->m_iOutWidth,curr->m_iOutHeight);
      }
      else if (curr->m_iExtendedSpatialScalability == ESS_PICT)
      {
        printf("Extended Spatial Scalability - crop win by picture\n\n");
      }
      
    }
		return Err::m_nOK;
}
// TMM_ESS }



ErrVal ControlMngH264AVCDecoder::initSlice( SliceHeader& rcSH, ProcessingState eProcessingState )
{
  m_uiCurrLayer   = rcSH.getLayerId();
  m_pcMbDataCtrl  = rcSH.getFrameUnit()->getMbDataCtrl();

  RNOK( m_pcMbDataCtrl->initSlice( rcSH, eProcessingState, true, NULL ) );

  if( PARSE_PROCESS == eProcessingState )
  {
    MbSymbolReadIf* pcMbSymbolReadIf;
    
    if( rcSH.getPPS().getEntropyCodingModeFlag() )
    {
      pcMbSymbolReadIf = m_pcCabacReader;
    }
    else
    {
      ROT(1);
    }

    RNOK( pcMbSymbolReadIf->startSlice( rcSH ) );
    RNOK( m_pcMbParser->initSlice( pcMbSymbolReadIf ) );

  }

  if( DECODE_PROCESS == eProcessingState)
  {
    RNOK( m_pcMotionCompensation->initSlice( rcSH ) );
  }
  
  return Err::m_nOK;
}



ErrVal ControlMngH264AVCDecoder::initSliceForReading( const SliceHeader& rcSH )
{
  m_uiCurrLayer   = rcSH.getLayerId();

  MbSymbolReadIf* pcMbSymbolReadIf;
  
  if( rcSH.getPPS().getEntropyCodingModeFlag() )
  {
    pcMbSymbolReadIf = m_pcCabacReader;
  }
  else
  {
    ROT(1);
  }

  RNOK( pcMbSymbolReadIf->startSlice( rcSH ) );
  RNOK( m_pcMbParser->initSlice( pcMbSymbolReadIf ) );

  return Err::m_nOK;
}


ErrVal ControlMngH264AVCDecoder::initSliceForDecoding( const SliceHeader& rcSH )
{
  m_uiCurrLayer   = rcSH.getLayerId();

  RNOK( m_pcMotionCompensation->initSlice( rcSH ) );

  return Err::m_nOK;
}


ErrVal ControlMngH264AVCDecoder::initSliceForFiltering( const SliceHeader& rcSH )
{
  m_uiCurrLayer   = rcSH.getLayerId();

  return Err::m_nOK;
}


ErrVal ControlMngH264AVCDecoder::finishSlice( const SliceHeader& rcSH, Bool& rbPicDone, Bool& rbFrameDone )
{
  rbPicDone     = m_pcMbDataCtrl->isPicDone( rcSH );
  rbFrameDone   = m_pcMbDataCtrl->isFrameDone( rcSH );
  m_uiCurrLayer = MSYS_UINT_MAX;
  
  return Err::m_nOK;
}

H264AVC_NAMESPACE_END
