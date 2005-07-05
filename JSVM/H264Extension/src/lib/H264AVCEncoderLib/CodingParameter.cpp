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
#include "H264AVCCommonLib.h"

#include "CodingParameter.h"

#include <math.h>

#define ROTREPORT(x,t) {if(x) {::printf("\n%s\n",t); assert(0); return Err::m_nInvalidParameter;} }

// h264 namepace begin
H264AVC_NAMESPACE_BEGIN



ErrVal MotionVectorSearchParams::check() const
{
  ROTREPORT( 4 < m_eSearchMode,   "No Such Search Mode 0==Block,1==Spiral,2==Log,3==Fast, 4==NewFast" )
  ROTREPORT( 3 < m_eFullPelDFunc, "No Such Search Func (Full Pel) 0==SAD,1==SSE,2==Hadamard" )
  ROTREPORT( 3 == m_eFullPelDFunc && (m_eSearchMode==2 || m_eSearchMode==3), "Log and Fast search not possible in comb. with distortion measure 3" )
  ROTREPORT( 2 < m_eSubPelDFunc,  "No Such Search Func (Sub Pel) 0==SAD,1==SSE,2==Hadamard" )
  ROTREPORT( 1 < m_uiDirectMode,  "Direct Mode Exceeds Supported Range 0==Temporal, 1==Spatial");

  return Err::m_nOK;
}


ErrVal LoopFilterParams::check() const
{
  ROTREPORT( 2 < getFilterIdc(),        "Loop Filter Idc exceeds supported range 0..2");  if( 69 != getAlphaOffset() )

  if( 69 != getAlphaOffset() )
  {
    ROTREPORT( 26 < getAlphaOffset(),       "Loop Filter Alpha Offset exceeds supported range -26..26");
    ROTREPORT( -26 > getAlphaOffset(),      "Loop Filter Alpha Offset exceeds supported range -26..26");
  }

  if( 69 != getBetaOffset() )
  {
    ROTREPORT( 26 < getBetaOffset(),        "Loop Filter Beta Offset exceeds supported range -26..26");
    ROTREPORT( -26 > getBetaOffset(),       "Loop Filter Beta Offset exceeds supported range -26..26");
  }

  return Err::m_nOK;
}



ErrVal LayerParameters::check()
{
  ROTREPORT( getFrameWidth              () % 16,        "Frame Width must be a multiple of 16" );
  ROTREPORT( getFrameHeight             () % 16,        "Frame Height must be a multiple of 16" );
  ROTREPORT( getInputFrameRate          () < 
             getOutputFrameRate         (),             "Output frame rate must be less than or equal to input frame rate" );
  ROTREPORT( getAdaptiveTransform       () > 2,         "FRExt mode not supported" );
  ROTREPORT( getMaxAbsDeltaQP           () > 7,         "MaxAbsDeltaQP not supported" );
  ROTREPORT( getNumFGSLayers            () > 3,         "Number of FGS layers not supported" );
  ROTREPORT( getInterLayerPredictionMode() < 0 ||
             getInterLayerPredictionMode() > 2,         "Unsupported inter-layer prediction mode" );
  ROTREPORT( getMotionInfoMode          () > 2,         "Motion info mode not supported" );
#if MULTIPLE_LOOP_DECODING
  ROTREPORT( getDecodingLoops           () > 2,         "Unsupported mode for decoding loops" );
#else
  ROTREPORT( getDecodingLoops           () > 1,         "Unsupported mode for decoding loops" );
#endif
  
  return Err::m_nOK;
}


UInt CodingParameter::getLogFactor( Double r0, Double r1 )
{
  Double dLog2Factor  = log10( r1 / r0 ) / log10( 2.0 );
  Double dRound       = floor( dLog2Factor + 0.5 );
  Double dEpsilon     = 0.0001;

  if( dLog2Factor-dEpsilon < dRound && dRound < dLog2Factor+dEpsilon )
  {
    return (UInt)(dRound);
  }
  return MSYS_UINT_MAX;
}



ErrVal CodingParameter::check()
{
  ROTS( m_cLoopFilterParams         .check() );
  ROTS( m_cMotionVectorSearchParams .check() );

  ROTREPORT( getMaximumFrameRate() <= 0.0,              "Maximum frame rate not supported" );
  ROTREPORT( getMaximumDelay    ()  < 0.0,              "Maximum delay must be greater than or equal to 0" );
  ROTREPORT( getTotalFrames     () == 0,                "Total Number Of Frames must be greater than 0" );

  ROTREPORT( getGOPSize         ()  < 1  ||
             getGOPSize         ()  > 64,               "GOP Size not supported" );
  UInt uiDecStages = getLogFactor( 1.0, getGOPSize() );
  ROTREPORT( uiDecStages == MSYS_UINT_MAX,              "GOP Size must be a multiple of 2" );
  setDecompositionStages( uiDecStages );

  ROTREPORT( getIntraPeriod     ()  <
             getGOPSize         (),                     "Intra period must be greater or equal to GOP size" );
  if( getIntraPeriod() == MSYS_UINT_MAX )
  {
    setIntraPeriodLowPass( MSYS_UINT_MAX );
  }
  else
  {
    UInt uiIntraPeriod = getIntraPeriod() / getGOPSize() - 1;
    ROTREPORT( getIntraPeriod() % getGOPSize(),         "Intra period must be a power of 2 of GOP size (or -1)" );
    setIntraPeriodLowPass( uiIntraPeriod );
  }

  ROTREPORT( getNumRefFrames    ()  < 1  ||
             getNumRefFrames    ()  > 15,               "Number of reference frames not supported" );
  ROTREPORT( getBaseLayerMode   ()  > 2,                "Base layer mode not supported" );
  ROTREPORT( getNumberOfLayers  ()  > MAX_LAYERS,       "Number of layers not supported" );



  Double  dMaxFrameDelay  = max( 0, m_dMaximumFrameRate * m_dMaximumDelay / 1000.0 );
  UInt    uiMaxFrameDelay = (UInt)floor( dMaxFrameDelay );

  for( UInt uiLayer = 0; uiLayer < getNumberOfLayers(); uiLayer++ )
  {
    LayerParameters*  pcLayer               = &m_acLayerParameters[uiLayer];

	  RNOK( pcLayer->check() );

    LayerParameters*  pcBaseLayer           = uiLayer ? &m_acLayerParameters[uiLayer-1] : 0;
    UInt              uiLogFactorInOutRate  = getLogFactor( pcLayer->getOutputFrameRate (), pcLayer->getInputFrameRate() );
    UInt              uiLogFactorMaxInRate  = getLogFactor( pcLayer->getInputFrameRate  (), getMaximumFrameRate       () );

    // heiko.schwarz@hhi.fhg.de: add some additional check for input/output frame rates
    ROTREPORT( pcLayer->getInputFrameRate() < pcLayer->getOutputFrameRate(),  "Input frame rate must not be less than output frame rate" );
    ROTREPORT( pcLayer->getInputFrameRate() > getMaximumFrameRate(),          "Input frame rate must not be greater than maximum frame rate" );
    ROTREPORT( getDecompositionStages() < uiLogFactorMaxInRate + uiLogFactorInOutRate, "Number of decomposition stages is too small for the specified output rate" );

    ROTREPORT( uiLogFactorInOutRate == MSYS_UINT_MAX,   "Input frame rate must be a power of 2 of output frame rate" );
    ROTREPORT( uiLogFactorMaxInRate == MSYS_UINT_MAX,   "Maximum frame rate must be a power of 2 of input frame rate" );

    pcLayer->setNotCodedMCTFStages  ( uiLogFactorInOutRate );
    pcLayer->setTemporalResolution  ( uiLogFactorMaxInRate );
    pcLayer->setDecompositionStages ( getDecompositionStages() - uiLogFactorMaxInRate );
    pcLayer->setFrameDelay          ( uiMaxFrameDelay  /  ( 1 << uiLogFactorMaxInRate ) );

    if( pcBaseLayer ) // for sub-sequence SEI
    {
      ROTREPORT( pcLayer->getInputFrameRate() < pcBaseLayer->getInputFrameRate(), "Input frame rate less than base layer output frame rate" );
      UInt uiLogFactorRate = getLogFactor( pcBaseLayer->getInputFrameRate(), pcLayer->getInputFrameRate() );
      ROTREPORT( uiLogFactorRate == MSYS_UINT_MAX, "Input Frame rate must be a power of 2 from layer to layer" );
      pcLayer->setBaseLayerTempRes( uiLogFactorRate );


      ROTREPORT( pcLayer->getFrameWidth ()  < pcBaseLayer->getFrameWidth (), "Frame width  less than base layer frame width" );
      ROTREPORT( pcLayer->getFrameHeight()  < pcBaseLayer->getFrameHeight(), "Frame height less than base layer frame height" );
      UInt uiLogFactorWidth  = getLogFactor( pcBaseLayer->getFrameWidth (), pcLayer->getFrameWidth () );
      UInt uiLogFactorHeight = getLogFactor( pcBaseLayer->getFrameHeight(), pcLayer->getFrameHeight() );
     
      pcLayer->setBaseLayerSpatRes( uiLogFactorWidth );
			
// TMM_ESS {
      ResizeParameters * resize = pcLayer->getResizeParameters();
      if (resize->m_iExtendedSpatialScalability != ESS_NONE)
        {
          ROTREPORT(resize->m_iInWidth  % 16,   "Base layer width must be a multiple of 16" );
          ROTREPORT(resize->m_iInHeight % 16,   "Base layer height must be a multiple of 16" );
          if (resize->m_bCrop)
            {
              ROTREPORT( resize->m_iPosX % 2 , "Cropping Window must be even aligned" );
              ROTREPORT( resize->m_iPosY % 2 , "Cropping Window must be even aligned" );  
              ROTREPORT(resize->m_iGlobWidth  % 16, "Enhancement layer width must be a multiple of 16" );
              ROTREPORT(resize->m_iGlobHeight % 16, "Enhancement layer height must be a multiple of 16" );
            }
          else
            {
              resize->m_iGlobWidth = resize->m_iOutWidth;
              resize->m_iGlobHeight = resize->m_iOutHeight;
            }
          printf("\n\n*************************\n%dx%d  -> %dx%d %s -> %dx%d\n",
                 resize->m_iInWidth, resize->m_iInHeight,
                 resize->m_iOutWidth, resize->m_iOutHeight,
                 (resize->m_bCrop ? "Crop" : "No_Crop"),
                 resize->m_iGlobWidth, resize->m_iGlobHeight);
          printf("ExtendedSpatialScalability: %d    SpatialScalabilityType: %d\n",
            resize->m_iExtendedSpatialScalability,
            resize->m_iSpatialScalabilityType
            );

          
          if ( resize->m_iSpatialScalabilityType <= 1 ) {
            printf("INTRA UPSAMPL - 1/2 FILTER INTERP\n");
            printf("INTER UPSAMPL - BILINEAR\n");
            printf("UPSAMPLE MOTION - AUTOMATIC\n");
          }
          else {
            switch (resize->m_iIntraUpsamplingType)
              {
              case 1:  printf("INTRA UPSAMPL - LANCZOS\n"); break;
              case 2:  printf("INTRA UPSAMPL - 1/2 FILTER INTERP + 1/4 PEL LINEAR INTERP + CHOICE AT 1/4 LEVEL\n"); break;
              case 3:  printf("INTRA UPSAMPL - KAISER\n"); break;
              default: printf("INTRA UPSAMPL - UNKNOWN ????????\n");
              }
            }
        }
      else
        {
          printf("\n\n*************************\n D_Upsampling - No_Crop\n");
        }
// TMM_ESS }

      if( pcLayer->getDecodingLoops() == 0 ) // single-loop decoding also for low-pass
      {
        pcBaseLayer->setContrainedIntraForLP();
      }
    }

    if( pcLayer->getBaseQualityLevel() > 3 )
      pcLayer->setBaseQualityLevel(3);

    if( uiLayer == 0 && pcLayer->getBaseQualityLevel() != 0 )
      pcLayer->setBaseQualityLevel(0);
  }

 return Err::m_nOK;
}



H264AVC_NAMESPACE_END
