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



#if !defined(AFX_CODINGPARAMETER_H__8403A680_A65D_466E_A411_05C3A7C0D59F__INCLUDED_)
#define AFX_CODINGPARAMETER_H__8403A680_A65D_466E_A411_05C3A7C0D59F__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


H264AVC_NAMESPACE_BEGIN



#if defined( MSYS_WIN32 )
# pragma warning( disable: 4275 )
#endif

// TMM_ESS 
#include "ResizeParameters.h"

#define MAX_CONFIG_PARAMS 256

class H264AVCENCODERLIB_API EncoderConfigLineBase
{
protected:
  EncoderConfigLineBase(Char* pcTag, UInt uiType ) : m_cTag( pcTag ), m_uiType( uiType ) {}
  EncoderConfigLineBase() {}
public:
  virtual ~EncoderConfigLineBase() {}
  std::string&  getTag () { return m_cTag; }
  virtual Void  setVar ( std::string& rcValue ) = 0;
protected:
  std::string m_cTag;
  UInt m_uiType;
};

class H264AVCENCODERLIB_API MotionVectorSearchParams
{
public:
  MotionVectorSearchParams() :  m_uiSearchMode(FAST_SEARCH),  m_uiFullPelDFunc(DF_SAD), m_uiSubPelDFunc(DF_SAD), m_uiSearchRange(64), m_uiDirectMode(0) {}

  ErrVal check() const;

  const SearchMode getSearchMode()                const { return (SearchMode)m_uiSearchMode; }
  const DFunc getFullPelDFunc()                   const { return (DFunc)m_uiFullPelDFunc; }
  const DFunc getSubPelDFunc()                    const { return (DFunc)m_uiSubPelDFunc; }
  const UInt getSearchRange()                     const { return m_uiSearchRange; }
  UInt        getNumMaxIter     ()                const { return m_uiNumMaxIter; }
  UInt        getIterSearchRange()                const { return m_uiIterSearchRange; }
  const UInt getDirectMode()                      const { return m_uiDirectMode; }

  Void setSearchMode( UInt uiSearchMode )               { m_uiSearchMode   = uiSearchMode; }
  Void setFullPelDFunc( UInt uiFullPelDFunc )           { m_uiFullPelDFunc = uiFullPelDFunc; }
  Void setSubPelDFunc( UInt uiSubPelDFunc )             { m_uiSubPelDFunc  = uiSubPelDFunc; }
  Void setSearchRange ( UInt uiSearchRange)             { m_uiSearchRange = uiSearchRange; }
  Void setNumMaxIter        ( UInt uiNumMaxIter      )  { m_uiNumMaxIter      = uiNumMaxIter;       }
  Void setIterSearchRange   ( UInt uiIterSearchRange )  { m_uiIterSearchRange = uiIterSearchRange;  }
  Void setDirectMode( UInt uiDirectMode)                { m_uiDirectMode = uiDirectMode; }

public:
  UInt        m_uiSearchMode;
  UInt        m_uiFullPelDFunc;
  UInt        m_uiSubPelDFunc;
  UInt        m_uiSearchRange;   // no limit
  UInt        m_uiNumMaxIter;
  UInt        m_uiIterSearchRange;
  UInt        m_uiDirectMode;    // 0 temporal, 1 spatial
};



class H264AVCENCODERLIB_API LoopFilterParams
{
public:
  LoopFilterParams() : m_uiFilterIdc( 0 ),  m_iAlphaOffset( 0 ), m_iBetaOffset( 0 ) {}

  ErrVal check() const ;

  Bool  isDefault()                     const { return m_uiFilterIdc == 0 && m_iAlphaOffset == 0 && m_iBetaOffset == 0;}
  const UInt getFilterIdc()             const { return m_uiFilterIdc; }
  const Int getAlphaOffset()            const { return m_iAlphaOffset; }
  const Int getBetaOffset()             const { return m_iBetaOffset; }
  Void setAlphaOffset( Int iAlphaOffset )     { m_iAlphaOffset = iAlphaOffset; }
  Void setBetaOffset( Int iBetaOffset )       { m_iBetaOffset = iBetaOffset; }
  Void setFilterIdc( UInt uiFilterIdc)        { m_uiFilterIdc = uiFilterIdc; }

public:
  UInt m_uiFilterIdc;   // 0: Filter All Edges, 1: Filter No Edges, 2: Filter All Edges But Slice Boundaries
  Int  m_iAlphaOffset;
  Int  m_iBetaOffset;
};


const unsigned CodParMAXNumSliceGroupsMinus1 =8; // the same as MAXNumSliceGroupsMinus1 in pictureParameter.h






class H264AVCENCODERLIB_API LayerParameters
{
public:
  LayerParameters()
    : m_uiLayerId                         (0)
    , m_uiFrameWidth                      (352)
    , m_uiFrameHeight                     (288)
    , m_dInputFrameRate                   (7.5)
    , m_dOutputFrameRate                  (7.5)
    , m_cInputFilename                    ("none")
    , m_cOutputFilename                   ("none")
    , m_uiEntropyCodingModeFlag           (1)
    , m_uiAdaptiveTransform               (0)
    , m_uiMaxAbsDeltaQP                   (1)
    , m_dBaseQpResidual                   (26.0)
    , m_uiInterLayerPredictionMode        (0)
    , m_uiMotionInfoMode                  (0)
    , m_cMotionInfoFilename               ("none")
    , m_uiDecompositionStages             (0)
    , m_uiNotCodedMCTFStages              (0)
    , m_uiTemporalResolution              (0)
    , m_uiFrameDelay                      (0)
    , m_uiBaseQualityLevel                (15)
    , m_bConstrainedIntraPredForLP        (false)
    , m_uiForceReorderingCommands         (0)
    , m_uiBaseLayerId                     (MSYS_UINT_MAX)
    , m_uiMbAff                           ( 0 )
    , m_uiPaff                            ( 0 )
    , m_uiUseRedundantSlice               (0)   //JVT-Q054 Red. Picture
		, m_uiUseRedundantKeySlice            (0)   //JVT-W049
// JVT-Q065 EIDR{
	  , m_iIDRPeriod						  (0)
	  , m_bBLSkipEnable					  ( false )
  // JVT-Q065 EIDR}
    , m_uiMGSVectorMode                      ( 0 )
    , m_dQpModeDecisionLP ( 0.00 )
    , m_uiNumSliceGroupMapUnitsMinus1 ( 0 ) 
    // JVT-S054 (ADD) ->
    , m_uiNumSliceMinus1 (0)
    , m_bSliceDivisionFlag (false)
    //, m_uiSliceDivisionType (0)//SEI changes update
		, m_bGridFlag (0)//SEI changes update
    , m_puiGridSliceWidthInMbsMinus1 (0)
    , m_puiGridSliceHeightInMbsMinus1 (0)
    , m_puiFirstMbInSlice (0)
    , m_puiLastMbInSlice (0)
    , m_puiSliceId (0)
    // JVT-S054 (ADD) <-
    , m_bAVCRewriteFlag                  ( 0 )   // JVT-V035
    , m_bAVCAdaptiveRewriteFlag          ( 0 )   // JVT-V035
    , m_uiLowComplexMbEnable             ( 0 )   // JVT-V079    

	//S051{
	, m_cOutSIPFileName					("none")
	, m_cInSIPFileName					("none")
	, m_uiAnaSIP						(0)
	, m_bEncSIP							(false)
	//S051}
//JVT-T054{
    , m_uiLayerCGSSNR         ( 0 )
    , m_uiQualityLevelCGSSNR  ( 0 )
    , m_uiBaseLayerCGSSNR     ( 0 )
    , m_uiBaseQualityLevelCGSSNR ( 0 )
//DS_FIX_FT_09_2007
    , m_bDiscardable          ( true )
    , m_uiQLDiscardable       ( MAX_QUALITY_LEVELS )
//~DS_FIX_FT_09_2007
//JVT-T054}
    , m_uiExplicitQPCascading( 0 )
  {
    for( UInt ui = 0; ui < MAX_DSTAGES; ui++ ) m_adQpModeDecision[ui] = 0.00;
    ::memset( m_uiMGSVect, 0x00, 16*sizeof(UInt) );

    for( UInt uiTTL = 0; uiTTL < MAX_TEMP_LEVELS; uiTTL++ )
    {
      m_adDeltaQPTLevel[uiTTL] = 0.0;
    }
  }

  virtual ~LayerParameters()
  {
    // JVT-S054 (ADD) ->
    if (m_puiGridSliceWidthInMbsMinus1 != NULL)
    {
      free(m_puiGridSliceWidthInMbsMinus1);
      m_puiGridSliceWidthInMbsMinus1 = NULL;
    }
    if (m_puiGridSliceHeightInMbsMinus1 != NULL)
    {
      free(m_puiGridSliceHeightInMbsMinus1);
      m_puiGridSliceHeightInMbsMinus1 = NULL;
    }
    if (m_puiFirstMbInSlice != NULL)
    {
      free(m_puiFirstMbInSlice);
      m_puiFirstMbInSlice = NULL;
    }
    if (m_puiLastMbInSlice != NULL)
    {
      free(m_puiLastMbInSlice);
      m_puiLastMbInSlice = NULL;
    }
    if (m_puiSliceId != NULL)
    {
      free(m_puiSliceId);
      m_puiSliceId = NULL;
    }
    // JVT-S054 (ADD) <-
  }


  //===== get =====
  UInt                            getLayerId                        () const {return m_uiLayerId; }
  UInt                            getFrameWidth                     () const {return m_uiFrameWidth; }
  UInt                            getFrameHeight                    () const {return m_uiFrameHeight; }
  Double                          getInputFrameRate                 () const {return m_dInputFrameRate; }
  Double                          getOutputFrameRate                () const {return m_dOutputFrameRate; }
  const std::string&              getInputFilename                  () const {return m_cInputFilename; }
  const std::string&              getOutputFilename                 () const {return m_cOutputFilename; }
  Bool                            getEntropyCodingModeFlag          () const {return m_uiEntropyCodingModeFlag == 1; }
  UInt                            getAdaptiveTransform              () const {return m_uiAdaptiveTransform; }
  UInt                            getMaxAbsDeltaQP                  () const {return m_uiMaxAbsDeltaQP; }
  Double                          getBaseQpResidual                 () const {return m_dBaseQpResidual; }
  Double                          getQpModeDecision          (UInt ui) const {return m_adQpModeDecision[ui]; }
  Double                          getQpModeDecisionLP               () const {return m_dQpModeDecisionLP; }
  UInt                            getInterLayerPredictionMode       () const {return m_uiInterLayerPredictionMode; }
  UInt                            getBaseQualityLevel               () const {return m_uiBaseQualityLevel; }
  UInt                            getMotionInfoMode                 () const {return m_uiMotionInfoMode; }
  const std::string&              getMotionInfoFilename             () const {return m_cMotionInfoFilename; }
  
//JVT-V079 Low-complexity MB mode decision {
  UInt                            getLowComplexMbEnable             () const   { return m_uiLowComplexMbEnable; }
//JVT-V079 Low-complexity MB mode decision }
  
  UInt                            getDecompositionStages            () const {return m_uiDecompositionStages; }
  UInt                            getNotCodedMCTFStages             () const {return m_uiNotCodedMCTFStages; }
  UInt                            getTemporalResolution             () const {return m_uiTemporalResolution; }
  UInt                            getFrameDelay                     () const {return m_uiFrameDelay; }

  UInt                            getBaseLayerSpatRes               () const {return m_uiBaseLayerSpatRes; }
  UInt                            getBaseLayerTempRes               () const {return m_uiBaseLayerTempRes; }
  Bool                            getContrainedIntraForLP           () const {return m_bConstrainedIntraPredForLP; }
  UInt                            getForceReorderingCommands        () const {return m_uiForceReorderingCommands; }
  UInt                            getBaseLayerId                    () const {return m_uiBaseLayerId; }
  UInt                            getMbAff                          () const {return m_uiMbAff;}
  UInt                            getPaff                           () const {return m_uiPaff;}
  Bool                            getUseRedundantSliceFlag          () const {return m_uiUseRedundantSlice == 1; }  //JVT-Q054 Red. Picture
  Bool                            getUseRedundantKeySliceFlag       () const {return m_uiUseRedundantKeySlice == 1; }   //JVT-W049
  //--ICU/ETRI FMO Implementation :  FMO start 
  UInt          getNumSliceGroupsMinus1() const {return m_uiNumSliceGroupsMinus1;}  //for test
  UInt          getSliceGroupMapType() const {return  m_uiSliceGroupMapType;  }
  Bool          getSliceGroupChangeDirection_flag () const {return m_bSliceGroupChangeDirection_flag;}
  UInt          getSliceGroupChangeRateMinus1 () const {return m_uiSliceGroupChangeRateMinus1;}
  UInt          getNumSliceGroupMapUnitsMinus1() const {return m_uiNumSliceGroupMapUnitsMinus1;}
  UInt          getSliceGroupId(Int i) const {return m_uiSliceGroupId[i];}
  UInt          getSliceMode() const {return m_uiSliceMode;}
  UInt          getSliceArgument() const { return m_uiSliceArgument ;}
  const std::string&   getSliceGroupConfigFileName() const{ return m_cSliceGroupConfigFileName;}
  UInt          getUseRedundantSlice() const { return m_uiUseRedundantSlice;}
  UInt*         getArrayRunLengthMinus1 () const {return (UInt*)m_uiRunLengthMinus1;}  
  UInt*         getArrayTopLeft () const {return (UInt*)m_uiTopLeft;}
  UInt*         getArrayBottomRight () const {return (UInt*)m_uiBottomRight;}
  UInt*         getArraySliceGroupId() const {return (UInt*)m_uiSliceGroupId;}
  //--ICU/ETRI FMO Implementation : FMO end

  //<-- consider ROI Extraction ICU/ETRI DS
  const std::string&   getROIConfigFileName() const{ return m_cROIConfigFileName;}
  UInt          getNumROI() const {return m_uiNumROI;}  //for test

  UInt*         getROIID () const {return (UInt*)m_uiROIID;}
  UInt*         getSGID () const {return (UInt*)m_uiSGID;}
  UInt*         getSLID () const {return (UInt*)m_uiSLID;}
  //--> consider ROI Extraction ICU/ETRI DS
  
  UInt getMGSVect                        (UInt uiNum) const { return m_uiMGSVectorMode ? m_uiMGSVect[uiNum] : (uiNum == 0 ? 16 : 0); }
  Bool getAVCRewriteFlag ()               const { return m_bAVCRewriteFlag==1; }
  Bool getAVCAdaptiveRewriteFlag ()       const { return m_bAVCAdaptiveRewriteFlag==1; }

  //===== set =====
  Void setLayerId                         (UInt   p) { m_uiLayerId                        = p; }
  Void setFrameWidth                      (UInt   p) { m_uiFrameWidth                     = p; }
  Void setFrameHeight                     (UInt   p) { m_uiFrameHeight                    = p; }
  Void setInputFrameRate                  (Double p) { m_dInputFrameRate                  = p; }
  Void setOutputFrameRate                 (Double p) { m_dOutputFrameRate                 = p; }
  Void setInputFilename                   (Char*  p) { m_cInputFilename                   = p; }
  Void setOutputFilename                  (Char*  p) { m_cOutputFilename                  = p; }
  Void setEntropyCodingModeFlag           (Bool   p) { m_uiEntropyCodingModeFlag          = p; }
  Void setAdaptiveTransform               (UInt   p) { m_uiAdaptiveTransform              = p; }
  Void setMaxAbsDeltaQP                   (UInt   p) { m_uiMaxAbsDeltaQP                  = p; }
  Void setBaseQpResidual                  (Double p) { m_dBaseQpResidual                  = p; }
  Void setQpModeDecision                  (UInt   n,
                                           Double p) { m_adQpModeDecision             [n] = p; }
  Void setQpModeDecisionLP                (Double p) { m_dQpModeDecisionLP                = p; }
  Void setInterLayerPredictionMode        (UInt   p) { m_uiInterLayerPredictionMode       = p; }
  Void setMotionInfoMode                  (UInt   p) { m_uiMotionInfoMode                 = p; }
  Void setMotionInfoFilename              (Char*  p) { m_cMotionInfoFilename              = p; }
  
  Void setDecompositionStages             (UInt   p) { m_uiDecompositionStages            = p; }
  Void setNotCodedMCTFStages              (UInt   p) { m_uiNotCodedMCTFStages             = p; }
  Void setTemporalResolution              (UInt   p) { m_uiTemporalResolution             = p; }
  Void setFrameDelay                      (UInt   p) { m_uiFrameDelay                     = p; }

  Void setBaseLayerSpatRes                (UInt   p) { m_uiBaseLayerSpatRes               = p; }
  Void setBaseLayerTempRes                (UInt   p) { m_uiBaseLayerTempRes               = p; }
  Void setBaseQualityLevel                (UInt   p) { m_uiBaseQualityLevel               = p; }
  Void setContrainedIntraForLP            ()         { m_bConstrainedIntraPredForLP       = true; }
  Void setForceReorderingCommands         (UInt   p) { m_uiForceReorderingCommands        = p; }
  Void setBaseLayerId                     (UInt   p) { m_uiBaseLayerId                    = p; }
  Void setMbAff                           (UInt   p) { m_uiMbAff                          = p; }
  Void setPaff                            (UInt   p) { m_uiPaff                           = p; }
// TMM_ESS {
  int                 getExtendedSpatialScalability     () { return m_ResizeParameter.m_iExtendedSpatialScalability; }
  int                 getSpatialScalabilityType         () { return m_ResizeParameter.m_iSpatialScalabilityType; }
  Void                setResizeParameters      (ResizeParameters *p) { memcpy(&m_ResizeParameter, p, sizeof(ResizeParameters)); }
  ResizeParameters*   getResizeParameters      () {return &m_ResizeParameter; }
// TMM_ESS }

// JVT-Q065 EIDR{
  Int				  getIDRPeriod			   () { return m_iIDRPeriod; }
  Bool				  getBLSkipEnable		   () { return m_bBLSkipEnable; }
  Void				  setBLSkipEnable( Bool b )   { m_bBLSkipEnable = b; }
// JVT-Q065 EIDR}

  UInt                getPLR                   () { return m_uiPLR; } //JVT-R057 LA-RDO
  Void                setPLR                   (UInt ui){m_uiPLR=ui;} //JVT-W049
  //===== check =====
  ErrVal  check();

//--ICU/ETRI FMO Implementation
  Void setSliceGroupId(int i, UInt value) { m_uiSliceGroupId[i] = value;}

  Void                            setUseRedundantSliceFlag(Bool   b) { m_uiUseRedundantSlice = b; }  // JVT-Q054 Red. Picture
  Void                            setUseRedundantKeySliceFlag(UInt   b) { m_uiUseRedundantKeySlice = b; }  //JVT-W049
  //S051{
  const std::string&              getInSIPFileName             () const { return m_cInSIPFileName; }
  const std::string&              getOutSIPFileName            () const { return m_cOutSIPFileName; }
  Void							  setInSIPFileName			   (Char* p) { m_cInSIPFileName=p; }
  Void							  setOutSIPFileName			   (Char* p) { m_cOutSIPFileName=p; }
  Void							  setAnaSIP					   (UInt	uiAnaSIP){ m_uiAnaSIP = uiAnaSIP;}
  Void						      setEncSIP					   (Bool	bEncSIP){ m_bEncSIP = bEncSIP;}
  UInt							  getAnaSIP					   (){ return m_uiAnaSIP; }
  Bool							  getEncSIP					   (){ return m_bEncSIP; }
  //S051}
//JVT-T054{
  UInt getLayerCGSSNR                    ()    { return m_uiLayerCGSSNR;}
  UInt getQualityLevelCGSSNR             ()    { return m_uiQualityLevelCGSSNR;}
  UInt getNumberOfQualityLevelsCGSSNR    ()   const;
  UInt getBaseLayerCGSSNR                    ()    { return m_uiBaseLayerCGSSNR;}
  UInt getBaseQualityLevelCGSSNR             ()    { return m_uiBaseQualityLevelCGSSNR;}
  Void setLayerCGSSNR                    (UInt ui)    { m_uiLayerCGSSNR                   = ui;}
  Void setQualityLevelCGSSNR             (UInt ui)    { m_uiQualityLevelCGSSNR            = ui;}
  Void setBaseLayerCGSSNR                    (UInt ui)    { m_uiBaseLayerCGSSNR                   = ui;}
  Void setBaseQualityLevelCGSSNR             (UInt ui)    { m_uiBaseQualityLevelCGSSNR            = ui;}
//DS_FIX_FT_09_2007
   Void setNonDiscardable                  ()       { m_bDiscardable = false;}
   Void setQLDiscardable                   (UInt ui)  { m_uiQLDiscardable = ui;}
   UInt getQLDiscardable                   ()       { return m_uiQLDiscardable;}
//~DS_FIX_FT_09_2007
  Bool isDiscardable                      ()          { return m_bDiscardable;}
//JVT-T054}

  UInt    getExplicitQPCascading  ()               const   { return m_uiExplicitQPCascading; }
  Double  getDeltaQPTLevel        ( UInt    ui )   const   { return m_adDeltaQPTLevel[ui]; }

  Void    setExplicitQPCascading  ( UInt    ui )           { m_uiExplicitQPCascading = ui; }
  Void    setDeltaQPTLevel        ( UInt    tl,
                                    Double  d  )           { m_adDeltaQPTLevel[tl] = d; }

public:
  UInt                      m_uiLayerId;
  UInt                      m_uiFrameWidth;
  UInt                      m_uiFrameHeight;
  Double                    m_dInputFrameRate;
  Double                    m_dOutputFrameRate;
  std::string               m_cInputFilename;
  std::string               m_cOutputFilename;

  UInt                      m_uiEntropyCodingModeFlag;
  UInt                      m_uiAdaptiveTransform;

  UInt                      m_uiMaxAbsDeltaQP;
  Double                    m_dBaseQpResidual;
  
  Double                    m_adQpModeDecision[MAX_DSTAGES];
  Double                    m_dQpModeDecisionLP;
  UInt                      m_uiInterLayerPredictionMode;
  Bool                      m_bConstrainedIntraPredForLP;
  UInt                      m_uiForceReorderingCommands;
  UInt                      m_uiBaseLayerId;

  UInt                      m_uiBaseQualityLevel;

  UInt                      m_uiMotionInfoMode;
  std::string               m_cMotionInfoFilename;
  
  //JVT-V079 Low-complexity MB mode decision {
  Int                     m_uiLowComplexMbEnable;
  //JVT-V079 Low-complexity MB mode decision }
  
  //----- derived parameters -----
  UInt                      m_uiDecompositionStages;
  UInt                      m_uiNotCodedMCTFStages;
  UInt                      m_uiTemporalResolution;
  UInt                      m_uiFrameDelay;

  //----- for scalable SEI ----
  UInt                      m_uiBaseLayerSpatRes;
  UInt                      m_uiBaseLayerTempRes;

  //----- ESS ---- 
  ResizeParameters          m_ResizeParameter;

  UInt                      m_uiMbAff;
  UInt                      m_uiPaff;

  //--ICU/ETRI FMO Implementation : FMO start
  UInt         m_uiNumSliceGroupsMinus1;  
  UInt         m_uiSliceGroupMapType;  
  UInt         m_uiRunLengthMinus1[CodParMAXNumSliceGroupsMinus1];  
  UInt         m_uiTopLeft[CodParMAXNumSliceGroupsMinus1];
  UInt         m_uiBottomRight[CodParMAXNumSliceGroupsMinus1];
  Bool         m_bSliceGroupChangeDirection_flag;
  UInt         m_uiSliceGroupChangeRateMinus1;
  UInt         m_uiNumSliceGroupMapUnitsMinus1;
  UInt         m_uiSliceGroupId[CodParMAXNumSliceGroupsMinus1];
  UInt         m_uiSliceMode;
  UInt         m_uiSliceArgument;
  std::string  m_cSliceGroupConfigFileName;

  std::string  m_cROIConfigFileName;
  UInt		   m_uiNumROI;
  UInt		   m_uiROIID[CodParMAXNumSliceGroupsMinus1];
  UInt		   m_uiSGID[CodParMAXNumSliceGroupsMinus1];
  UInt		   m_uiSLID[CodParMAXNumSliceGroupsMinus1];

  //--ICU/ETRI FMO Implementation : FMO end
  UInt         m_uiUseRedundantSlice;   // JVT-Q054 Red. Picture
  UInt         m_uiUseRedundantKeySlice;   //JVT-W049
  // JVT-S054 (ADD) ->
  Bool         m_bSliceDivisionFlag;
  UInt         m_uiNumSliceMinus1;
  //UInt         m_uiSliceDivisionType;//SEI changes update
	Bool         m_bGridFlag;//SEI changes update
  UInt*        m_puiGridSliceWidthInMbsMinus1;
  UInt*        m_puiGridSliceHeightInMbsMinus1;
  UInt*        m_puiFirstMbInSlice;
  UInt*        m_puiLastMbInSlice;
  UInt*        m_puiSliceId;
  // JVT-S054 (ADD) <-

  // JVT-V035
  UInt         m_bAVCRewriteFlag;
  UInt         m_bAVCAdaptiveRewriteFlag;

// JVT-Q065 EIDR{
  Int						m_iIDRPeriod;
  Bool						m_bBLSkipEnable;
// JVT-Q065 EIDR}

  UInt               m_uiPLR; //JVT-R057 LA-RDO
  UInt       m_uiMGSVectorMode;
  UInt       m_uiMGSVect[16];

  //S051{
  std::string    m_cOutSIPFileName;
  std::string	 m_cInSIPFileName;
  UInt			 m_uiAnaSIP;
  Bool			 m_bEncSIP;
  //S051}
//JVT-T054{
  UInt                      m_uiLayerCGSSNR;
  UInt                      m_uiQualityLevelCGSSNR;
  UInt                      m_uiBaseLayerCGSSNR;
  UInt                      m_uiBaseQualityLevelCGSSNR;
  Bool                      m_bDiscardable;
  UInt                      m_uiQLDiscardable; //DS_FIX_FT_09_2007
//JVT-T054}

  UInt    m_uiExplicitQPCascading;
  Double  m_adDeltaQPTLevel[MAX_TEMP_LEVELS];

};


//TMM_WP
class H264AVCENCODERLIB_API SampleWeightingParams
{
  public:
    SampleWeightingParams() : m_uiIPMode(0), m_uiBMode(0), m_uiLumaDenom(5), m_uiChromaDenom(5), m_fDiscardThr(1) { }
        ErrVal check() const ;
        ErrVal checkForValidChanges( const SampleWeightingParams& rcSW )const;

        Bool operator == ( const SampleWeightingParams& rcSWP ) const ;
        Bool operator != ( const SampleWeightingParams& rcSWP ) const { return !((*this) == rcSWP); }
        ErrVal writeBinary( BinDataAccessor& rcBinDataAccessor )  const;
        ErrVal readBinary( BinDataAccessor& rcBinDataAccessor );

        UInt getIPMode()                  const { return m_uiIPMode; }
        UInt getBMode()                   const { return m_uiBMode; }
        UInt getLumaDenom()               const { return m_uiLumaDenom; }
        UInt getChromaDenom()             const { return m_uiChromaDenom; }
        Float getDiscardThr()             const { return m_fDiscardThr; }
        Void setIPMode( UInt uiIPMode )         { m_uiIPMode      = uiIPMode; }
        Void setBMode( UInt uiBMode )           { m_uiBMode       = uiBMode; }
        Void setLumaDenom( UInt uiDenom )       { m_uiLumaDenom   = uiDenom; }
        Void setChromaDenom( UInt uiDenom )     { m_uiChromaDenom = uiDenom; }
        Void setDiscardThr( Float fDiscardThr ) { m_fDiscardThr = fDiscardThr; }

  protected:
        UInt m_uiIPMode;      // 0 off, 1 on, 2 random
        UInt m_uiBMode;       // 0 off, 1 explicit, 2 implicit, 3 random
        UInt m_uiLumaDenom;   // 0-7
        UInt m_uiChromaDenom; // 0-7
        Float m_fDiscardThr;
};
//TMM_WP



class H264AVCENCODERLIB_API CodingParameter
{
public:
  CodingParameter()
    : m_dMaximumFrameRate                 ( 0.0 )
    , m_dMaximumDelay                     ( 1e6 )
    , m_uiTotalFrames                     ( 0 )
    , m_uiGOPSize                         ( 0 )
    , m_uiDecompositionStages             ( 0 )
    , m_uiIntraPeriod                     ( 0 )
    , m_uiIntraPeriodLowPass              ( 0 )
    , m_uiNumRefFrames                    ( 0 )
    , m_uiBaseLayerMode                   ( 0 )
    , m_uiNumberOfLayers                  ( 0 )
    , m_uiAVCmode                         ( 0 )
    , m_uiFrameWidth                      ( 0 )
    , m_uiFrameHeight                     ( 0 )
    , m_uiSymbolMode                      ( 0 )
    , m_ui8x8Mode                         ( 0 )
    , m_dBasisQp                          ( 0 )
    , m_uiDPBSize                         ( 0 )
    , m_uiNumDPBRefFrames                 ( 0 )
    , m_uiLog2MaxFrameNum                 ( 0 )
    , m_uiLog2MaxPocLsb                   ( 0 )
    , m_cSequenceFormatString             ()
    , m_uiMaxRefIdxActiveBL0              ( 0 )
    , m_uiMaxRefIdxActiveBL1              ( 0 )
    , m_uiMaxRefIdxActiveP                ( 0 )
//TMM_WP
      , m_uiIPMode                        ( 0 )
      , m_uiBMode                         ( 0 )
//TMM_WP
	  , m_bNonRequiredEnable		      ( 0 ) //NonRequired JVT-Q066
	  , m_uiLARDOEnable                   ( 0 )      //JVT-R057 LA-RDO
		, m_uiEssRPChkEnable									( 0 )
		, m_uiMVThres													( 20 )
	  , m_uiPreAndSuffixUnitEnable		      ( 0 )  //JVT-S036 lsj 
	  , m_uiMMCOBaseEnable			      ( 0 ) //JVT-S036 lsj
//JVT-T073 {
	  , m_uiNestingSEIEnable              ( 0 ) 
	  , m_uiSceneInfoEnable               ( 0 )
//JVT-T073 }
		, m_uiIntegrityCheckSEIEnable       ( 0 )//JVT-W052 wxwan
//JVT-T054{
    , m_uiCGSSNRRefinementFlag            ( 0 )
//JVT-T054}
// JVT-U085 LMI
    , m_uiTlevelNestingFlag               ( 1 )
// JVT-U116 LMI
    , m_uiTl0PicIdxPresentFlag            ( 0 )
    , m_uiCIUFlag                         ( 0 ) //JV
    , m_uiRCDOBlockSizes                  ( 0 )
    , m_uiRCDOMotionCompensationY         ( 0 )
    , m_uiRCDOMotionCompensationC         ( 0 )
    , m_uiRCDODeblocking                  ( 0 )
    , m_ui4TapMotionCompensationY         ( 0 ) // V090
    , m_uiEncodeKeyPictures               ( 0 )
    , m_uiMGSKeyPictureControl            ( 0 )
    , m_uiMGSKeyPictureMotionRefinement   ( 1 )
  {
    for( UInt uiLayer = 0; uiLayer < 6; uiLayer++ )
    {
      m_adDeltaQpLayer[uiLayer] = 0;
    }
  }
	virtual ~CodingParameter()
  {
  }

public:
  const MotionVectorSearchParams& getMotionVectorSearchParams       () const {return m_cMotionVectorSearchParams; }
  const LoopFilterParams&         getLoopFilterParams               () const {return m_cLoopFilterParams; }

  MotionVectorSearchParams&       getMotionVectorSearchParams       ()       {return m_cMotionVectorSearchParams; }
  LoopFilterParams&               getLoopFilterParams               ()       {return m_cLoopFilterParams; }

  const LayerParameters&          getLayerParameters  ( UInt    n )   const   { return m_acLayerParameters[n]; }
  LayerParameters&                getLayerParameters  ( UInt    n )           { return m_acLayerParameters[n]; }

//TMM_WP
  SampleWeightingParams&           getSampleWeightingParams(UInt uiLayerId)  {return m_cSampleWeightingParams[uiLayerId];}
//TMM_WP
  
  
  const std::string&              getInputFile            ()              const   { return m_cInputFile; }
  Double                          getMaximumFrameRate     ()              const   { return m_dMaximumFrameRate; }
  Double                          getMaximumDelay         ()              const   { return m_dMaximumDelay; }
  UInt                            getTotalFrames          ()              const   { return m_uiTotalFrames; }
  UInt                            getGOPSize              ()              const   { return m_uiGOPSize; }  
  UInt                            getDecompositionStages  ()              const   { return m_uiDecompositionStages; }
  UInt                            getIntraPeriod          ()              const   { return m_uiIntraPeriod; }
  UInt                            getIntraPeriodLowPass   ()              const   { return m_uiIntraPeriodLowPass; }
  UInt                            getNumRefFrames         ()              const   { return m_uiNumRefFrames; }
  UInt                            getBaseLayerMode        ()              const   { return m_uiBaseLayerMode; }
  UInt                            getNumberOfLayers       ()              const   { return m_uiNumberOfLayers; }
/*  Void                            getSimplePriorityMap    ( UInt uiSimplePri, UInt& uiTemporalLevel, UInt& uiLayer, UInt& uiQualityLevel )
                                                                          { uiTemporalLevel = m_uiTemporalLevelList[uiSimplePri];
                                                                            uiLayer         = m_uiDependencyIdList [uiSimplePri];
                                                                            uiQualityLevel  = m_uiQualityLevelList [uiSimplePri];
                                                                          }
 JVT-S036 lsj */
//TMM_WP
  UInt getIPMode()                  const { return m_uiIPMode; }
  UInt getBMode()                   const { return m_uiBMode; }
//TMM_WP

  UInt                            getAVCmode              ()              const   { return m_uiAVCmode; }
  UInt                            getFrameWidth           ()              const   { return m_uiFrameWidth; }
  UInt                            getFrameHeight          ()              const   { return m_uiFrameHeight; }
  UInt                            getSymbolMode           ()              const   { return m_uiSymbolMode; }
  UInt                            get8x8Mode              ()              const   { return m_ui8x8Mode; }
  Double                          getBasisQp              ()              const   { return m_dBasisQp; }
  UInt                            getDPBSize              ()              const   { return m_uiDPBSize; }
  UInt                            getNumDPBRefFrames      ()              const   { return m_uiNumDPBRefFrames; }
  UInt                            getLog2MaxFrameNum      ()              const   { return m_uiLog2MaxFrameNum; }
  UInt                            getLog2MaxPocLsb        ()              const   { return m_uiLog2MaxPocLsb; }
  std::string                     getSequenceFormatString ()              const   { return m_cSequenceFormatString; }
  Double                          getDeltaQpLayer         ( UInt ui )     const   { return m_adDeltaQpLayer[ui]; }
  UInt                            getMaxRefIdxActiveBL0   ()              const   { return m_uiMaxRefIdxActiveBL0; }
  UInt                            getMaxRefIdxActiveBL1   ()              const   { return m_uiMaxRefIdxActiveBL1; }
  UInt                            getMaxRefIdxActiveP     ()              const   { return m_uiMaxRefIdxActiveP; }

  Void                            setInputFile            ( Char*   p )   { m_cInputFile            = p; }

  UInt                            getLARDOEnable          ()              const   { return m_uiLARDOEnable;} //JVT-R057 LA-RDO
  UInt														getEssRPChkEnable				()							const		{	return m_uiEssRPChkEnable;}
  UInt														getMVThres							()							const		{	return m_uiMVThres;}

  UInt							              getPreAndSuffixUnitEnable	  ()		      const	  { return m_uiPreAndSuffixUnitEnable;} //prefix unit
	UInt							              getMMCOBaseEnable		  ()			  const	  { return m_uiMMCOBaseEnable; } //JVT-S036 lsj
  // JVT-T073 {
  UInt                            getNestingSEIEnable     ()              const   { return m_uiNestingSEIEnable; }
  UInt                            getSceneInfoEnable      ()              const   { return m_uiSceneInfoEnable; }
  // JVT-T073 }

	UInt														getIntegrityCheckSEIEnable()						const   { return m_uiIntegrityCheckSEIEnable; }//JVT-W052
  Void                            setIntegrityCheckSEIEnable( UInt  ui )  { m_uiIntegrityCheckSEIEnable = ui;}           //JVT-W052 bug_fixed
  Void                            setMaximumFrameRate     ( Double  d )   { m_dMaximumFrameRate     = d; }
  Void                            setMaximumDelay         ( Double  d )   { m_dMaximumDelay         = d; }
  Void                            setTotalFrames          ( UInt    n )   { m_uiTotalFrames         = n; }
  Void                            setGOPSize              ( UInt    n )   { m_uiGOPSize             = n; }
  Void                            setDecompositionStages  ( UInt    n )   { m_uiDecompositionStages = n; }
  Void                            setIntraPeriod          ( UInt    n )   { m_uiIntraPeriod         = n; }
  Void                            setIntraPeriodLowPass   ( UInt    n )   { m_uiIntraPeriodLowPass  = n; }
  Void                            setNumRefFrames         ( UInt    n )   { m_uiNumRefFrames        = n; }
  Void                            setBaseLayerMode        ( UInt    n )   { m_uiBaseLayerMode       = n; }
  Void                            setNumberOfLayers       ( UInt    n )   { m_uiNumberOfLayers      = n; }
 /* Void                            setSimplePriorityMap ( UInt uiSimplePri, UInt uiTemporalLevel, UInt uiLayer, UInt uiQualityLevel )
                                                                          { m_uiTemporalLevelList[uiSimplePri] = uiTemporalLevel;
                                                                            m_uiDependencyIdList [uiSimplePri] = uiLayer;
                                                                            m_uiQualityLevelList [uiSimplePri] = uiQualityLevel;
                                                                          }
 JVT-S036 lsj */
  Void                            setFrameWidth           ( UInt    p )   { m_uiFrameWidth          = p; }
  Void                            setFrameHeight          ( UInt    p )   { m_uiFrameHeight         = p; }
  Void                            setSymbolMode           ( UInt    p )   { m_uiSymbolMode          = p; }
  Void                            set8x8Mode              ( UInt    p )   { m_ui8x8Mode             = p; }
  Void                            setBasisQp              ( Double  p )   { m_dBasisQp              = p; }
  Void                            setDPBSize              ( UInt    p )   { m_uiDPBSize             = p; }
  Void                            setNumDPBRefFrames      ( UInt    p )   { m_uiNumDPBRefFrames     = p; }
  Void                            setLog2MaxFrameNum      ( UInt    p )   { m_uiLog2MaxFrameNum     = p; }
  Void                            setLog2MaxPocLsb        ( UInt    p )   { m_uiLog2MaxPocLsb       = p; }
  Void                            setSequenceFormatString ( Char*   p )   { m_cSequenceFormatString = p; }
  Void                            setDeltaQpLayer         ( UInt    n,
                                                            Double  p )   { m_adDeltaQpLayer[n]     = p; }
  Void                            setMaxRefIdxActiveBL0   ( UInt    p )   { m_uiMaxRefIdxActiveBL0  = p; }
  Void                            setMaxRefIdxActiveBL1   ( UInt    p )   { m_uiMaxRefIdxActiveBL1  = p; }
  Void                            setMaxRefIdxActiveP     ( UInt    p )   { m_uiMaxRefIdxActiveP    = p; }

  ErrVal                          check                   ();
  
  // TMM_ESS 
  ResizeParameters*               getResizeParameters  ( UInt    n )    { return m_acLayerParameters[n].getResizeParameters(); }

  Int					              		  getNonRequiredEnable    ()			{ return m_bNonRequiredEnable; }  //NonRequired JVT-Q066 (06-04-08)

//JVT-T054{
  UInt                            getCGSSNRRefinement     ()              const   { return m_uiCGSSNRRefinementFlag;}
  Void                            setCGSSNRRefinement     ( UInt    b )   { m_uiCGSSNRRefinementFlag = b; }
//JVT-T054}
// JVT-U085 LMI {
  Bool                            getTlevelNestingFlag    ()              const   { return m_uiTlevelNestingFlag > 0 ? true : false; }
  Void                            setTlevelNestingFlag    ( UInt  ui )    { m_uiTlevelNestingFlag = ui; }
// JVT-U085 LMI }
// JVT-U116 LMI {
  Bool                            getTl0PicIdxPresentFlag        ()              const   { return m_uiTl0PicIdxPresentFlag > 0 ? true : false; }
  Void                            setTl0PicIdxPresentFlag        ( UInt  ui )    { m_uiTl0PicIdxPresentFlag = ui; }
// JVT-U116 LMI }

  //JVT-U106 Behaviour at slice boundaries{
  void   setCIUFlag(UInt  flag)
  {
	  m_uiCIUFlag=flag;
  }
  UInt   getCIUFlag()
  {
	  return m_uiCIUFlag;
  }
  //JVT-U106 Behaviour at slice boundaries}

  UInt  getRCDOBlockSizes         ()  const { return m_uiRCDOBlockSizes; }
  UInt  getRCDOMotionCompensationY()  const { return m_uiRCDOMotionCompensationY; }
  UInt  getRCDOMotionCompensationC()  const { return m_uiRCDOMotionCompensationC; }
  UInt  getRCDODeblocking         ()  const { return m_uiRCDODeblocking; }

  UInt  get4TapMotionCompensationY()  const { return m_ui4TapMotionCompensationY; }  // V090
  Void  set4TapMotionCompensationY( UInt ui )  { m_ui4TapMotionCompensationY = ui; } // V090

  Void  setRCDOBlockSizes         ( UInt ui )  { m_uiRCDOBlockSizes          = ui; }
  Void  setRCDOMotionCompensationY( UInt ui )  { m_uiRCDOMotionCompensationY = ui; }
  Void  setRCDOMotionCompensationC( UInt ui )  { m_uiRCDOMotionCompensationC = ui; }
  Void  setRCDODeblocking         ( UInt ui )  { m_uiRCDODeblocking          = ui; }

  Void  setEncodeKeyPictures      ( UInt ui )          { m_uiEncodeKeyPictures = ui; }
  UInt  getEncodeKeyPictures      ()           const   { return m_uiEncodeKeyPictures; }

  Void  setMGSKeyPictureControl   ( UInt ui )          { m_uiMGSKeyPictureControl = ui; }
  UInt  getMGSKeyPictureControl   ()           const   { return m_uiMGSKeyPictureControl; }

  Void  setMGSKeyPictureMotRef    ( UInt ui )          { m_uiMGSKeyPictureMotionRefinement = ui; }
  UInt  getMGSKeyPictureMotRef    ()           const   { return m_uiMGSKeyPictureMotionRefinement; }

  // JVT-V068 HRD {
  UInt getEnableVUI()    { return getEnableNalHRD() || getEnableVclHRD(); }
  Bool getEnableNalHRD() { return m_uiNalHRD == 0 ? false: true; }
  Bool getEnableVclHRD() { return m_uiVclHRD == 0 ? false: true; }
  UInt getEnableVUITimingInfo() { return getEnableVUI(); }
  UInt getEnableSEIBufferingPeriod() { return getEnableNalHRD() || getEnableVclHRD(); }
  UInt getEnableSEIPicTiming() { return getEnableNalHRD() || getEnableVclHRD(); }
  // JVT-V068 HRD }
  //JVT-W049 {
  Bool getEnableRedundantKeyPic() {return m_uiRedundantKeyPic == 0 ? false:true; }
  //JVT-W049 }
	//JVT-X046 {
  UInt getSliceMode               ()                   { return m_uiSliceMode; }
  UInt getSliceArgument           ()                   { return m_uiSliceArgument; }
  //JVT-X046 }
private:
  UInt                            getLogFactor            ( Double  r0,
                                                            Double  r1 );


protected:
  std::string               m_cInputFile;
  Double                    m_dMaximumFrameRate;
  Double                    m_dMaximumDelay;
  UInt                      m_uiTotalFrames;

  UInt                      m_uiGOPSize;
  UInt                      m_uiDecompositionStages;
  UInt                      m_uiIntraPeriod;
  UInt                      m_uiIntraPeriodLowPass;
  UInt                      m_uiNumRefFrames;
  UInt                      m_uiBaseLayerMode;

  MotionVectorSearchParams  m_cMotionVectorSearchParams;
  LoopFilterParams          m_cLoopFilterParams;

  UInt                      m_uiNumberOfLayers;
  LayerParameters           m_acLayerParameters[MAX_LAYERS];

  EncoderConfigLineBase*    m_pEncoderLines[MAX_CONFIG_PARAMS];
  EncoderConfigLineBase*    m_pLayerLines  [MAX_CONFIG_PARAMS];

  UInt                      m_uiAVCmode;
  UInt                      m_uiFrameWidth;
  UInt                      m_uiFrameHeight;
  UInt                      m_uiSymbolMode;
  UInt                      m_ui8x8Mode;
  Double                    m_dBasisQp;
  UInt                      m_uiDPBSize;
  UInt                      m_uiNumDPBRefFrames;
  UInt                      m_uiLog2MaxFrameNum;
  UInt                      m_uiLog2MaxPocLsb;
  std::string               m_cSequenceFormatString;
  Double                    m_adDeltaQpLayer[6];
  UInt                      m_uiMaxRefIdxActiveBL0;
  UInt                      m_uiMaxRefIdxActiveBL1;
  UInt                      m_uiMaxRefIdxActiveP;

//TMM_WP
  UInt m_uiIPMode;
  UInt m_uiBMode;

  SampleWeightingParams m_cSampleWeightingParams[MAX_LAYERS];
//TMM_WP

  Int						m_bNonRequiredEnable; //NonRequired JVT-Q066
  UInt                       m_uiLARDOEnable; //JVT-R057 LA-RDO

	UInt						m_uiEssRPChkEnable;
	UInt						m_uiMVThres;

  UInt						m_uiPreAndSuffixUnitEnable; //JVT-S036 lsj 
  UInt						m_uiMMCOBaseEnable;  //JVT-S036 lsj

//JVT-T054{
  UInt                      m_uiCGSSNRRefinementFlag;
//JVT-T054}
//JVT-T073 {
  UInt                      m_uiNestingSEIEnable;
  UInt                      m_uiSceneInfoEnable;
//JVT-T073 }

	//JVT-W052
	UInt											m_uiIntegrityCheckSEIEnable;
	//JVT-W052

// JVT-U085 LMI 
  UInt                      m_uiTlevelNestingFlag;
// JVT-U116 LMI 
  UInt                      m_uiTl0PicIdxPresentFlag;
  //JVT-U106 Behaviour at slice boundaries{
  UInt                      m_uiCIUFlag;
  //JVT-U106 Behaviour at slice boundaries}

  UInt    m_uiRCDOBlockSizes;
  UInt    m_uiRCDOMotionCompensationY;
  UInt    m_uiRCDOMotionCompensationC;
  UInt    m_uiRCDODeblocking;

  UInt    m_ui4TapMotionCompensationY;  // V090

  UInt    m_uiEncodeKeyPictures;  // 0:only FGS[default], 1:FGS&MGS, 2:always[useless]
  UInt    m_uiMGSKeyPictureControl;
  UInt    m_uiMGSKeyPictureMotionRefinement;

  // JVT-V068 HRD {
  UInt    m_uiNalHRD;
  UInt    m_uiVclHRD;
  // JVT-V068 HRD }
	//JVT-W049 {
  UInt    m_uiRedundantKeyPic;
  //JVT-W049 }
	//JVT-X046 {
  UInt    m_uiSliceMode;
  UInt    m_uiSliceArgument;
  //JVT-X046 }
};

#if defined( MSYS_WIN32 )
# pragma warning( default: 4275 )
#endif


H264AVC_NAMESPACE_END


#endif // !defined(AFX_CODINGPARAMETER_H__8403A680_A65D_466E_A411_05C3A7C0D59F__INCLUDED_)
