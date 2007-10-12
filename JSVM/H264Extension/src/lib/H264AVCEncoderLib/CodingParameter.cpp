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
#include "SequenceStructure.h"

#include <math.h>

#define ROTREPORT(x,t) {if(x) {::printf("\n%s\n",t); assert(0); return Err::m_nInvalidParameter;} }

// h264 namepace begin
H264AVC_NAMESPACE_BEGIN


ErrVal MotionVectorSearchParams::check() const
{
  ROTREPORT( 4 < m_uiSearchMode,   "No Such Search Mode 0==Block,1==Spiral,2==Log,3==Fast, 4==NewFast" )
  ROTREPORT( 3 < m_uiFullPelDFunc, "No Such Search Func (Full Pel) 0==SAD,1==SSE,2==Hadamard,3==YUV-SAD" )
  ROTREPORT( 3 == m_uiFullPelDFunc && (m_uiSearchMode==2 || m_uiSearchMode==3), "Log and Fast search not possible in comb. with distortion measure 3" )
  ROTREPORT( 2 < m_uiSubPelDFunc,  "No Such Search Func (Sub Pel) 0==SAD,1==SSE,2==Hadamard" )
  ROTREPORT( 1 < m_uiDirectMode,  "Direct Mode Exceeds Supported Range 0==Temporal, 1==Spatial");

  return Err::m_nOK;
}


ErrVal LoopFilterParams::check() const
{
  //V032 of FSL for extending the Idc value to 3 and 4 w.r.t. 0 and 2 in the enhanced layer 
  //for disabling chroma deblocking in enhanced layer
  ROTREPORT( 4 < getFilterIdc(),        "Loop Filter Idc exceeds supported range 0..4");  if( 69 != getAlphaOffset() )


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


UInt LayerParameters::getNumberOfQualityLevelsCGSSNR() const
{
  UInt uiVectPos = 0;
  UInt uiQLs;
  for( uiQLs = 0; uiVectPos != 16; uiQLs++ )
  {
    uiVectPos += getMGSVect( uiQLs );
  }
  return uiQLs;
}

ErrVal LayerParameters::check()
{
//TMM_INTERLACE{
  ROTREPORT( ( getFrameHeight           () % 32 ) &&
             ( getMbAff() > 0 || getPaff() > 0 ),       "Frame Height must be a multiple of 32 for interlace" );
//TMM_INTERLACE}

  ROTREPORT( getFrameWidth              () % 16,        "Frame Width must be a multiple of 16" );
  ROTREPORT( getFrameHeight             () % 16,        "Frame Height must be a multiple of 16" );
  ROTREPORT( getInputFrameRate          () < 
             getOutputFrameRate         (),             "Output frame rate must be less than or equal to input frame rate" );
  ROTREPORT( getAdaptiveTransform       () > 2,         "FRExt mode not supported" );
  ROTREPORT( getMaxAbsDeltaQP           () > 7,         "MaxAbsDeltaQP not supported" );
  ROTREPORT( getInterLayerPredictionMode() > 2,         "Unsupported inter-layer prediction mode" );
  ROTREPORT( getMotionInfoMode          () > 2,         "Motion info mode not supported" );

  ROTREPORT( getBaseLayerId() != MSYS_UINT_MAX && getBaseLayerId() >= getLayerId(), "BaseLayerId is not possible" );

  UInt uiVectPos = 0;
  UInt ui;
  for( ui = 0; uiVectPos < 16; ui++ )
  {
    uiVectPos += getMGSVect( ui );
    ROTREPORT( uiVectPos > 16 || (ui == 15 && uiVectPos < 16), "Sum over elements of MGSVector does not equal 16." );
  }
  Bool bTrailingZeros = true;
  for( ui = 15; ui > 0; ui-- )
  {
    if( getMGSVect( ui ) != 0 )
      bTrailingZeros = false;
    ROTREPORT( !bTrailingZeros && getMGSVect( ui ) == 0, "Zeros inside of the MGSVector are not allowed (except for the first element and the end of the vector)." );
  }
  Bool bUseMGSVectors = getMGSVect( 0 ) != 16;
  ROTREPORT( ( getAVCRewriteFlag() || getAVCAdaptiveRewriteFlag() ) && bUseMGSVectors, "MGS Vectors are not allowed with AVC rewriting enabled." );

  if ((getBaseLayerId() == MSYS_UINT_MAX) && (getAVCRewriteFlag() == true))
  {
    printf( "AvcRewriteFlag should be false for base layer, reset to 0\n" );
    m_bAVCRewriteFlag = 0;
  }

  if( m_dQpModeDecisionLP == -1.0 )
  {
    m_dQpModeDecisionLP = m_dBaseQpResidual;
  }

  // JVT-S054 (ADD) ->
  if ( m_uiNumSliceGroupsMinus1 > 0 && m_uiSliceGroupMapType == 0 && m_uiSliceMode == 4)
  {
    UInt i, j, uiSliceId, uiTotalRun, uiNumSlicePerHeight, uiMBAddr, uiMBCount, uiFrameWidthInMbs, uiFrameHeightInMbs, uiFrameSizeInMbs;

    m_bSliceDivisionFlag = true;
    //m_uiSliceDivisionType = 0;  // rectangular grid slice of constant size//SEI changes update
    m_bGridFlag = 0;  // rectangular grid slice of constant size//SEI changes update
    uiTotalRun = m_uiRunLengthMinus1[0] + 1;
    for (i=1; i<m_uiNumSliceGroupsMinus1; i++)
    {
      uiTotalRun += m_uiRunLengthMinus1[i] + 1;
      if ( m_uiRunLengthMinus1[i] != m_uiRunLengthMinus1[0] )
        //m_uiSliceDivisionType = 1;//SEI changes update
        m_bGridFlag = 1;//SEI changes update
    }
    uiTotalRun += m_uiRunLengthMinus1[m_uiNumSliceGroupsMinus1] + 1;
    if ( m_uiRunLengthMinus1[m_uiNumSliceGroupsMinus1] > m_uiRunLengthMinus1[0] )
    {
      //m_uiSliceDivisionType = 1;//SEI changes update
			m_bGridFlag = 1;//SEI changes update
    }

    if ( (uiTotalRun<<4) != m_uiFrameWidth )
    {
      printf("Unsupported IROI mode\n");
      m_bSliceDivisionFlag = false;
      return Err::m_nOK;
    }

    uiFrameWidthInMbs = m_uiFrameWidth>>4;
    uiFrameHeightInMbs = m_uiFrameHeight>>4;
    uiFrameSizeInMbs = uiFrameWidthInMbs*uiFrameHeightInMbs;
    uiNumSlicePerHeight = (uiFrameHeightInMbs+m_uiSliceArgument-1)/m_uiSliceArgument;
    m_uiNumSliceMinus1 = ((m_uiNumSliceGroupsMinus1+1) * uiNumSlicePerHeight) - 1;

    // Allocate memory for slice division info
    if (m_puiGridSliceWidthInMbsMinus1 != NULL)
      free(m_puiGridSliceWidthInMbsMinus1);
    m_puiGridSliceWidthInMbsMinus1 = (UInt*)malloc((m_uiNumSliceMinus1+1)*sizeof(UInt));

    if (m_puiGridSliceHeightInMbsMinus1 != NULL)
      free(m_puiGridSliceHeightInMbsMinus1);
    m_puiGridSliceHeightInMbsMinus1 = (UInt*)malloc((m_uiNumSliceMinus1+1)*sizeof(UInt));

    if (m_puiFirstMbInSlice != NULL)
      free(m_puiFirstMbInSlice);
    m_puiFirstMbInSlice = (UInt*)malloc((m_uiNumSliceMinus1+1)*sizeof(UInt));

    if (m_puiLastMbInSlice != NULL)
      free(m_puiLastMbInSlice);
    m_puiLastMbInSlice = (UInt*)malloc((m_uiNumSliceMinus1+1)*sizeof(UInt));

    if (m_puiSliceId != NULL)
      free(m_puiSliceId);
    m_puiSliceId = (UInt*)malloc(uiFrameSizeInMbs*sizeof(UInt));

    // Initialize slice division info
    uiSliceId=0;
    m_puiGridSliceWidthInMbsMinus1[uiSliceId] = m_uiRunLengthMinus1[0];
    m_puiGridSliceHeightInMbsMinus1[uiSliceId] = m_uiSliceArgument-1;
    m_puiFirstMbInSlice[uiSliceId] = 0;
    m_puiLastMbInSlice[uiSliceId] = m_puiFirstMbInSlice[uiSliceId] + m_puiGridSliceHeightInMbsMinus1[uiSliceId]*(m_uiFrameWidth>>4) + m_puiGridSliceWidthInMbsMinus1[uiSliceId];
    uiSliceId += uiNumSlicePerHeight;
    for (i=1; i<=m_uiNumSliceGroupsMinus1; i++)
    {
      m_puiGridSliceWidthInMbsMinus1[uiSliceId] = m_uiRunLengthMinus1[i];
      m_puiGridSliceHeightInMbsMinus1[uiSliceId] = m_uiSliceArgument-1;
      m_puiFirstMbInSlice[uiSliceId] = m_puiFirstMbInSlice[uiSliceId-uiNumSlicePerHeight] + m_uiRunLengthMinus1[i-1] + 1;
      m_puiLastMbInSlice[uiSliceId] = m_puiFirstMbInSlice[uiSliceId] + m_puiGridSliceHeightInMbsMinus1[uiSliceId]*(m_uiFrameWidth>>4) + m_puiGridSliceWidthInMbsMinus1[uiSliceId];
      uiSliceId += uiNumSlicePerHeight;
    }
    uiSliceId = 0;
    for (i=0; i<=m_uiNumSliceGroupsMinus1; i++)
    {
      uiSliceId++;
      for (j=1; j<uiNumSlicePerHeight; j++, uiSliceId++)
      {
        m_puiGridSliceWidthInMbsMinus1[uiSliceId] = m_puiGridSliceWidthInMbsMinus1[uiSliceId-1];
        m_puiGridSliceHeightInMbsMinus1[uiSliceId] = m_uiSliceArgument-1;
        if ( j == (uiNumSlicePerHeight-1) )
          m_puiGridSliceHeightInMbsMinus1[uiSliceId] = (m_uiFrameHeight>>4) - (j*m_uiSliceArgument) - 1;

        m_puiFirstMbInSlice[uiSliceId] = m_puiFirstMbInSlice[uiSliceId-1] + m_uiSliceArgument*(m_uiFrameWidth>>4);
        m_puiLastMbInSlice[uiSliceId] = m_puiFirstMbInSlice[uiSliceId] + m_puiGridSliceHeightInMbsMinus1[uiSliceId]*(m_uiFrameWidth>>4) + m_puiGridSliceWidthInMbsMinus1[uiSliceId];
      }
    }
    // Debug
    if (uiSliceId != (m_uiNumSliceMinus1+1))
    {
      printf("IROI error\n");
      m_bSliceDivisionFlag = false;
      return Err::m_nOK;
    }

    uiMBCount = 0;
    for (uiSliceId = 0; uiSliceId <= m_uiNumSliceMinus1; uiSliceId++)
    {
      uiMBAddr = m_puiFirstMbInSlice[uiSliceId];
      for (i = 0; i <= m_puiGridSliceHeightInMbsMinus1[uiSliceId]; i++, uiMBAddr+=uiFrameWidthInMbs)
      {
        for (j = 0; j <= m_puiGridSliceWidthInMbsMinus1[uiSliceId]; j++, uiMBCount++)
        {
          m_puiSliceId[uiMBAddr+j] = uiSliceId;
        }
      }
    }
    // Debug
    if (uiMBCount != uiFrameSizeInMbs)
    {
      printf("IROI error\n");
      m_bSliceDivisionFlag = false;
      return Err::m_nOK;
    }


    // Display slice division info.
    //printf("IROI: Slice Division Type %d, Num Slice %d\n", m_uiSliceDivisionType, m_uiNumSliceMinus1+1);//SEI changes update
		printf("IROI: Iroi Grid Flag %d, Num Slice %d\n", m_bGridFlag, m_uiNumSliceMinus1+1);//SEI changes update
    //for (i=0; i<=m_uiNumSliceMinus1; i++)
    //{
    //  printf("(%d, %d, %d, %d, %d)\n", i, m_puiGridSliceWidthInMbsMinus1[i], m_puiGridSliceHeightInMbsMinus1[i], m_puiFirstMbInSlice[i], m_puiLastMbInSlice[i]);
    //}
  }
  // JVT-S054 (ADD) <-

  //S051{
  ROTREPORT( getAnaSIP	()>0 && getEncSIP(),			"Unsupported SIP mode\n"); 
  //S051}

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

  if( getAVCmode() )
  {
    Bool bStringNotOk = SequenceStructure::checkString( getSequenceFormatString() ); 

    //===== coder is operated in MVC mode =====
    ROTREPORT( getFrameWidth        () <= 0 ||
               getFrameWidth        ()  % 16,             "Frame Width  must be greater than 0 and a multiple of 16" );
    ROTREPORT( getFrameHeight       () <= 0 ||
               getFrameHeight       ()  % 16,             "Frame Height must be greater than 0 and a multiple of 16" );
    ROTREPORT( getMaximumFrameRate  () <= 0.0,            "Frame rate not supported" );
    ROTREPORT( getTotalFrames       () == 0,              "Total Number Of Frames must be greater than 0" );
    ROTREPORT( getSymbolMode        ()  > 1,              "Symbol mode not supported" );
    ROTREPORT( get8x8Mode           ()  > 2,              "FRExt mode not supported" );
    ROTREPORT( getDPBSize           () == 0,              "DPBSize must be greater than 0" );
    ROTREPORT( getNumDPBRefFrames   () == 0 ||
               getNumDPBRefFrames   ()  > getDPBSize(),   "NumRefFrames must be greater than 0 and must not be greater than DPB size" );
    ROTREPORT( getLog2MaxFrameNum   ()  < 4 ||
               getLog2MaxFrameNum   ()  > 16,             "Log2MaxFrameNum must be in the range of [4..16]" );
    ROTREPORT( getLog2MaxPocLsb     ()  < 4 ||
               getLog2MaxPocLsb     ()  > 15,             "Log2MaxFrameNum must be in the range of [4..15]" );
    ROTREPORT( bStringNotOk,                              "Unvalid SequenceFormatString" );
    ROTREPORT( getMaxRefIdxActiveBL0() <= 0 ||
               getMaxRefIdxActiveBL0()  > 15,             "Unvalid value for MaxRefIdxActiveBL0" );
    ROTREPORT( getMaxRefIdxActiveBL1() <= 0 ||
               getMaxRefIdxActiveBL1()  > 15,             "Unvalid value for MaxRefIdxActiveBL1" );
    ROTREPORT( getMaxRefIdxActiveP  () <= 0 ||
               getMaxRefIdxActiveP  ()  > 15,             "Unvalid value for MaxRefIdxActiveP" );

    return Err::m_nOK;
  }

  ROTREPORT( getMaximumFrameRate() <= 0.0,              "Maximum frame rate not supported" );
  ROTREPORT( getMaximumDelay    ()  < 0.0,              "Maximum delay must be greater than or equal to 0" );
  ROTREPORT( getTotalFrames     () == 0,                "Total Number Of Frames must be greater than 0" );

  ROTREPORT( getGOPSize         ()  < 1  ||
             getGOPSize         ()  > 64,               "GOP Size not supported" );
  UInt uiDecStages = getLogFactor( 1.0, getGOPSize() );
  ROTREPORT( uiDecStages == MSYS_UINT_MAX,              "GOP Size must be a power of 2" );
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

	//JVT-X046 {
  ROTREPORT(   m_uiSliceMode < 0 || m_uiSliceMode > 2 || 
	  (m_uiSliceMode==1)&&(m_uiSliceArgument <= 0) || (m_uiSliceMode==2)&&(m_uiSliceArgument <= 0)
	  , "Unvalid value for SliceMode or SliceArgument" );
  //JVT-X046 }
  ROTREPORT( getNumRefFrames    ()  < 1  ||
             getNumRefFrames    ()  > 15,               "Number of reference frames not supported" );
  ROTREPORT( getBaseLayerMode   ()  > 2,                "Base layer mode not supported" );
  ROTREPORT( getNumberOfLayers  ()  > MAX_LAYERS,       "Number of layers not supported" );


  ROTREPORT( m_uiEncodeKeyPictures    > 2,          "Key picture mode not supported" );
  ROTREPORT( m_uiMGSKeyPictureControl > 2,          "Unsupported value for MGSControl" );
  ROTREPORT( m_uiMGSKeyPictureControl &&
            !m_uiCGSSNRRefinementFlag,              "MGSControl can only be specified in connection with CGSSNRRefinementFlag=1" );
  ROTREPORT( m_uiMGSKeyPictureMotionRefinement > 1, "Unsupported value for MGSKeyPicMotRef" );


  Double  dMaxFrameDelay  = max( 0, m_dMaximumFrameRate * m_dMaximumDelay / 1000.0 );
  UInt    uiMaxFrameDelay = (UInt)floor( dMaxFrameDelay );

  for( UInt uiLayer = 0; uiLayer < getNumberOfLayers(); uiLayer++ )
  {
    LayerParameters*  pcLayer               = &m_acLayerParameters[uiLayer];

	  RNOK( pcLayer->check() );

    UInt              uiBaseLayerId         = uiLayer && pcLayer->getBaseLayerId() != MSYS_UINT_MAX ? pcLayer->getBaseLayerId() : MSYS_UINT_MAX;
    LayerParameters*  pcBaseLayer           = uiBaseLayerId != MSYS_UINT_MAX ? &m_acLayerParameters[uiBaseLayerId] : 0;
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

    Bool bMGSVectorUsed = pcLayer->getMGSVect( 0 ) != 16;
    if( bMGSVectorUsed )
    {
      ROTREPORT( !getCGSSNRRefinement(), "MGS vectors are only supported in MGS." );
      ROTREPORT( !pcBaseLayer,           "MGS vectors are not allowed in the base layer." );
    }

    if( pcBaseLayer ) // for sub-sequence SEI
    {
      Bool bResolutionChange = pcLayer->getFrameWidth () != pcBaseLayer->getFrameWidth() || 
                               pcLayer->getFrameHeight() != pcBaseLayer->getFrameHeight();
      // maybe the following is only necessary when key pictures are used
      ROTREPORT( bResolutionChange && pcLayer->getMGSVect(0) != 16, "Base layer and current layer must have the same resolution when MGS vectors are used in the current layer." );
      ROTREPORT( pcLayer->getInputFrameRate() < pcBaseLayer->getInputFrameRate(), "Input frame rate less than base layer output frame rate" );
      UInt uiLogFactorRate = getLogFactor( pcBaseLayer->getInputFrameRate(), pcLayer->getInputFrameRate() );
      ROTREPORT( uiLogFactorRate == MSYS_UINT_MAX, "Input Frame rate must be a power of 2 from layer to layer" );
      pcLayer->setBaseLayerTempRes( uiLogFactorRate );


      ROTREPORT( pcLayer->getFrameWidth ()  < pcBaseLayer->getFrameWidth (), "Frame width  less than base layer frame width" );
      ROTREPORT( pcLayer->getFrameHeight()  < pcBaseLayer->getFrameHeight(), "Frame height less than base layer frame height" );
      UInt uiLogFactorWidth  = getLogFactor( pcBaseLayer->getFrameWidth (), pcLayer->getFrameWidth () );
     
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
            /*
          printf("\n\n*************************\n%dx%d  -> %dx%d %s -> %dx%d\n",
                 resize->m_iInWidth, resize->m_iInHeight,
                 resize->m_iOutWidth, resize->m_iOutHeight,
                 (resize->m_bCrop ? "Crop" : "No_Crop"),
                 resize->m_iGlobWidth, resize->m_iGlobHeight);
          printf("ExtendedSpatialScalability: %d    SpatialScalabilityType: %d\n",
            resize->m_iExtendedSpatialScalability,
                 resize->m_iSpatialScalabilityType);
                 */
        }
        /*
      else
        {
          printf("\n\n*************************\n No_Crop\n");
        }
        */
// TMM_ESS }

      pcBaseLayer->setContrainedIntraForLP();
    }

    ROTREPORT( pcLayer->getBaseQualityLevel() > 15, "Base quality level may not exceed 15." );

    if( uiLayer == 0 && pcLayer->getBaseQualityLevel() != 0 )
    {
      pcLayer->setBaseQualityLevel(0);
    }
  }

  return Err::m_nOK;
}



H264AVC_NAMESPACE_END
