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
public:
  EncoderConfigLineBase(Char* pcTag) {strcpy(m_pcTag, pcTag); m_uiType = 0;};
  EncoderConfigLineBase() {};
  Char* getTag() {return m_pcTag;};
  virtual void setVar(Char* pvValue) = 0;
protected:
  Char m_pcTag[256];
  UInt m_uiType;
};

class H264AVCENCODERLIB_API MotionVectorSearchParams
{
public:
  MotionVectorSearchParams() :  m_eSearchMode(FAST_SEARCH),  m_eFullPelDFunc(DF_SAD), m_eSubPelDFunc(DF_SAD), m_uiSearchRange(64), m_uiDirectMode(0) {}

  ErrVal check() const;

  const SearchMode getSearchMode()                const { return m_eSearchMode; }
  const DFunc getFullPelDFunc()                   const { return m_eFullPelDFunc; }
  const DFunc getSubPelDFunc()                    const { return m_eSubPelDFunc; }
  const UInt getSearchRange()                     const { return m_uiSearchRange; }
  UInt        getNumMaxIter     ()                const { return m_uiNumMaxIter; }
  UInt        getIterSearchRange()                const { return m_uiIterSearchRange; }
  const UInt getDirectMode()                      const { return m_uiDirectMode; }

  Void setSearchMode( UInt uiSearchMode )               { m_eSearchMode = SearchMode(uiSearchMode); }
  Void setFullPelDFunc( UInt uiFullPelDFunc )           { m_eFullPelDFunc = DFunc(uiFullPelDFunc); }
  Void setSubPelDFunc( UInt uiSubPelDFunc )             { m_eSubPelDFunc = DFunc(uiSubPelDFunc); }
  Void setSearchRange ( UInt uiSearchRange)             { m_uiSearchRange = uiSearchRange; }
  Void setNumMaxIter        ( UInt uiNumMaxIter      )  { m_uiNumMaxIter      = uiNumMaxIter;       }
  Void setIterSearchRange   ( UInt uiIterSearchRange )  { m_uiIterSearchRange = uiIterSearchRange;  }
  Void setDirectMode( UInt uiDirectMode)                { m_uiDirectMode = uiDirectMode; }

public:
  SearchMode  m_eSearchMode;
  DFunc       m_eFullPelDFunc;
  DFunc       m_eSubPelDFunc;
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
  UInt m_iAlphaOffset;
  UInt m_iBetaOffset;
};








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
    , m_uiUpdateStep                      (0)
    , m_uiAdaptiveQPSetting               (1)
    , m_uiMCTFIntraMode                   (1)
    , m_uiAdaptiveTransform               (0)
    , m_uiMaxAbsDeltaQP                   (1)
    , m_dBaseQpResidual                   (26.0)
    , m_dNumFGSLayers                     (0)
    , m_uiInterLayerPredictionMode        (0)
    , m_uiMotionInfoMode                  (0)
    , m_cMotionInfoFilename               ("none")
    , m_uiFGSMode                         (0)
    , m_cFGSRateFilename                  ("none")
    , m_dFGSRate                          (0)
    //{{Quality level estimation and modified truncation- JVTO044 and m12007
    //France Telecom R&D-(nathalie.cammas@francetelecom.com)
    , m_cDistoFilename                    ("none")
    , m_cRateFilename                     ("none")
    //}}Quality level estimation and modified truncation- JVTO044 and m12007
    , m_uiDecompositionStages             (0)
    , m_uiNotCodedMCTFStages              (0)
    , m_uiTemporalResolution              (0)
    , m_uiFrameDelay                      (0)
    , m_uiBaseQualityLevel                (3)
    , m_uiDecodingLoops                   (0)
    , m_bConstrainedIntraPredForLP        (false)
  {
    for( UInt ui = 0; ui < MAX_DSTAGES; ui++ ) m_adQpModeDecision[ui] = 0.00;
  }

  virtual ~LayerParameters()
  {
  }


  //===== get =====
  UInt                            getLayerId                        () const {return m_uiLayerId; }
  UInt                            getFrameWidth                     () const {return m_uiFrameWidth; }
  UInt                            getFrameHeight                    () const {return m_uiFrameHeight; }
  Double                          getInputFrameRate                 () const {return m_dInputFrameRate; }
  Double                          getOutputFrameRate                () const {return m_dOutputFrameRate; }
  const std::string&              getInputFilename                  () const {return m_cInputFilename; }
  const std::string&              getOutputFilename                 () const {return m_cOutputFilename; }
  UInt                            getUpdateStep                     () const {return m_uiUpdateStep; }
  UInt                            getAdaptiveQPSetting              () const {return m_uiAdaptiveQPSetting; }
  UInt                            getMCTFIntraMode                  () const {return m_uiMCTFIntraMode; }
  UInt                            getAdaptiveTransform              () const {return m_uiAdaptiveTransform; }
  UInt                            getMaxAbsDeltaQP                  () const {return m_uiMaxAbsDeltaQP; }
  Double                          getBaseQpResidual                 () const {return m_dBaseQpResidual; }
  Double                          getNumFGSLayers                   () const {return m_dNumFGSLayers; }
  Double                          getQpModeDecision          (UInt ui) const {return m_adQpModeDecision[ui]; }
  UInt                            getInterLayerPredictionMode       () const {return m_uiInterLayerPredictionMode; }
  UInt                            getBaseQualityLevel               () const {return m_uiBaseQualityLevel; }
  UInt                            getMotionInfoMode                 () const {return m_uiMotionInfoMode; }
  const std::string&              getMotionInfoFilename             () const {return m_cMotionInfoFilename; }
  UInt                            getFGSMode                        () const {return m_uiFGSMode; }
  const std::string&              getFGSFilename                    () const {return m_cFGSRateFilename; }
  Double                          getFGSRate                        () const {return m_dFGSRate; }
  //{{Quality level estimation and modified truncation- JVTO044 and m12007
  //France Telecom R&D-(nathalie.cammas@francetelecom.com)
  const std::string&              getRateFilename                    () const {return m_cRateFilename; }
  const std::string&              getDistoFilename                   () const {return m_cDistoFilename; }
  //}}Quality level estimation and modified truncation- JVTO044 and m12007

  UInt                            getDecompositionStages            () const {return m_uiDecompositionStages; }
  UInt                            getNotCodedMCTFStages             () const {return m_uiNotCodedMCTFStages; }
  UInt                            getTemporalResolution             () const {return m_uiTemporalResolution; }
  UInt                            getFrameDelay                     () const {return m_uiFrameDelay; }

  UInt                            getBaseLayerSpatRes               () const {return m_uiBaseLayerSpatRes; }
  UInt                            getBaseLayerTempRes               () const {return m_uiBaseLayerTempRes; }
  UInt                            getDecodingLoops                  () const {return m_uiDecodingLoops; }
  Bool                            getContrainedIntraForLP           () const {return m_bConstrainedIntraPredForLP; }


  //===== set =====
  Void setLayerId                         (UInt   p) { m_uiLayerId                        = p; }
  Void setFrameWidth                      (UInt   p) { m_uiFrameWidth                     = p; }
  Void setFrameHeight                     (UInt   p) { m_uiFrameHeight                    = p; }
  Void setInputFrameRate                  (Double p) { m_dInputFrameRate                  = p; }
  Void setOutputFrameRate                 (Double p) { m_dOutputFrameRate                 = p; }
  Void setInputFilename                   (Char*  p) { m_cInputFilename                   = p; }
  Void setOutputFilename                  (Char*  p) { m_cOutputFilename                  = p; }
  Void setUpdateStep                      (UInt   p) { m_uiUpdateStep                     = p; }
  Void setAdaptiveQPSetting               (UInt   p) { m_uiAdaptiveQPSetting              = p; }
  Void setMCTFIntraMode                   (UInt   p) { m_uiMCTFIntraMode                  = p; }
  Void setAdaptiveTransform               (UInt   p) { m_uiAdaptiveTransform              = p; }
  Void setMaxAbsDeltaQP                   (UInt   p) { m_uiMaxAbsDeltaQP                  = p; }
  Void setBaseQpResidual                  (Double p) { m_dBaseQpResidual                  = p; }
  Void setNumFGSLayers                    (Double p) { m_dNumFGSLayers                    = p; }
  Void setQpModeDecision                  (UInt   n,
                                           Double p) { m_adQpModeDecision             [n] = p; }
  Void setInterLayerPredictionMode        (UInt   p) { m_uiInterLayerPredictionMode       = p; }
  Void setMotionInfoMode                  (UInt   p) { m_uiMotionInfoMode                 = p; }
  Void setMotionInfoFilename              (Char*  p) { m_cMotionInfoFilename              = p; }
  Void setFGSMode                         (UInt   p) { m_uiFGSMode                        = p; }
  Void setFGSFilename                     (Char*  p) { m_cFGSRateFilename                 = p; }
  Void setFGSRate                         (Double p) { m_dFGSRate                         = p; }
  //{{Quality level estimation and modified truncation- JVTO044 and m12007
  //France Telecom R&D-(nathalie.cammas@francetelecom.com)
  Void setDistoFilename                    (Char*  p) { m_cDistoFilename                 = p; }
  Void setRateFilename                     (Char*  p) { m_cRateFilename                 = p; }
  //}}Quality level estimation and modified truncation- JVTO044 and m12007
  
  Void setDecompositionStages             (UInt   p) { m_uiDecompositionStages            = p; }
  Void setNotCodedMCTFStages              (UInt   p) { m_uiNotCodedMCTFStages             = p; }
  Void setTemporalResolution              (UInt   p) { m_uiTemporalResolution             = p; }
  Void setFrameDelay                      (UInt   p) { m_uiFrameDelay                     = p; }

  Void setBaseLayerSpatRes                (UInt   p) { m_uiBaseLayerSpatRes               = p; }
  Void setBaseLayerTempRes                (UInt   p) { m_uiBaseLayerTempRes               = p; }
  Void setBaseQualityLevel                (UInt   p) { m_uiBaseQualityLevel               = p; }
  Void setDecodingLoops                   (UInt   p) { m_uiDecodingLoops                  = p; }
  Void setContrainedIntraForLP            ()         { m_bConstrainedIntraPredForLP       = true; }

// TMM_ESS {
  int                 getExtendedSpatialScalability     () { return m_ResizeParameter.m_iExtendedSpatialScalability; }
  int                 getSpatialScalabilityType         () { return m_ResizeParameter.m_iSpatialScalabilityType; }
  Void                setResizeParameters      (ResizeParameters *p) { memcpy(&m_ResizeParameter, p, sizeof(ResizeParameters)); }
  ResizeParameters*   getResizeParameters      () {return &m_ResizeParameter; }
// TMM_ESS }
  //===== check =====
  ErrVal  check();



public:
  UInt                      m_uiLayerId;
  UInt                      m_uiFrameWidth;
  UInt                      m_uiFrameHeight;
  Double                    m_dInputFrameRate;
  Double                    m_dOutputFrameRate;
  std::string               m_cInputFilename;
  std::string               m_cOutputFilename;

  UInt                      m_uiUpdateStep;
  UInt                      m_uiAdaptiveQPSetting;
  UInt                      m_uiMCTFIntraMode;
  UInt                      m_uiAdaptiveTransform;

  UInt                      m_uiMaxAbsDeltaQP;
  Double                    m_dBaseQpResidual;
  Double                    m_dNumFGSLayers;
  
  Double                    m_adQpModeDecision[MAX_DSTAGES];
  UInt                      m_uiInterLayerPredictionMode;
  UInt                      m_uiDecodingLoops;
  Bool                      m_bConstrainedIntraPredForLP;

  UInt                      m_uiBaseQualityLevel;

  UInt                      m_uiMotionInfoMode;
  std::string               m_cMotionInfoFilename;

  UInt                      m_uiFGSMode;
  std::string               m_cFGSRateFilename;
  Double                    m_dFGSRate;
  //{{Quality level estimation and modified truncation- JVTO044 and m12007
  //France Telecom R&D-(nathalie.cammas@francetelecom.com)
  std::string               m_cRateFilename;
  std::string               m_cDistoFilename;
  //}}Quality level estimation and modified truncation- JVTO044 and m12007

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
};





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
    , m_bLowComplxUpdFlag                 ( 1 )
    , m_bExtendedPriorityId               ( 0 )
    , m_uiNumSimplePris                   (0)
    //{{Adaptive GOP structure
    // --ETRI & KHU
    , m_uiUseAGS                          (0)
    , m_uiWriteGOPMode                    (0)
    //}}Adaptive GOP structure
  {
      for ( UInt uiLoop = 0; uiLoop < (1 << PRI_ID_BITS); uiLoop++ )
      {
        m_uiTemporalLevelList[uiLoop] = 0;
        m_uiDependencyIdList [uiLoop] = 0;
        m_uiQualityLevelList [uiLoop] = 0;
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
  Bool                            getLowComplxUpdFlag     ()              const   { return m_bLowComplxUpdFlag; }
  Bool                            getExtendedPriorityId   ()              const   { return m_bExtendedPriorityId; }
  UInt                            getNumSimplePris        ()              const   { return m_uiNumSimplePris; }
  Void                            getSimplePriorityMap    ( UInt uiSimplePri, UInt& uiTemporalLevel, UInt& uiLayer, UInt& uiQualityLevel )
                                                                          { uiTemporalLevel = m_uiTemporalLevelList[uiSimplePri];
                                                                            uiLayer         = m_uiDependencyIdList [uiSimplePri];
                                                                            uiQualityLevel  = m_uiQualityLevelList [uiSimplePri];
                                                                          }

  Void                            setLowComplxUpdFlag     ( UInt  n   )   { m_bLowComplxUpdFlag = (n==1); }
  Void                            setInputFile            ( Char*   p )   { m_cInputFile            = p; }
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
  Void                            setExtendedPriorityId   ( Bool    b )   { m_bExtendedPriorityId   = b; }
  Void                            setNumSimplePris        ( UInt    n )   { m_uiNumSimplePris       = n; }
  Void                            setSimplePriorityMap ( UInt uiSimplePri, UInt uiTemporalLevel, UInt uiLayer, UInt uiQualityLevel )
                                                                          { m_uiTemporalLevelList[uiSimplePri] = uiTemporalLevel;
                                                                            m_uiDependencyIdList [uiSimplePri] = uiLayer;
                                                                            m_uiQualityLevelList [uiSimplePri] = uiQualityLevel;
                                                                          }

  ErrVal                          check                   ();
  //{{Quality level estimation and modified truncation- JVTO044 and m12007
  //France Telecom R&D-(nathalie.cammas@francetelecom.com)
  Bool getQualityLevelsEstimation() { return m_bQualityLevelsEstimation;}
  //}}Quality level estimation and modified truncation- JVTO044 and m12007

  // TMM_ESS 
  ResizeParameters*               getResizeParameters  ( UInt    n )    { return m_acLayerParameters[n].getResizeParameters(); }

  //{{Adaptive GOP structure
  // --ETRI & KHU
  UInt getUseAGS () const { return m_uiUseAGS; }
  Void setUseAGS (UInt p) { m_uiUseAGS = p; }
  UInt getWriteGOPMode () const { return m_uiWriteGOPMode; }
  Void setWriteGOPMode (UInt p) { m_uiWriteGOPMode = p; }
  const std::string& getGOPModeFile () const { return m_cGOPModeFilename; }
  Void setGOPModeFile (Char*   p) { m_cGOPModeFilename = p; }
  //}}Adaptive GOP structure

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
  Bool                      m_bLowComplxUpdFlag;

  MotionVectorSearchParams  m_cMotionVectorSearchParams;
  LoopFilterParams          m_cLoopFilterParams;

  UInt                      m_uiNumberOfLayers;
  LayerParameters           m_acLayerParameters[MAX_LAYERS];
  //{{Quality level estimation and modified truncation- JVTO044 and m12007
  //France Telecom R&D-(nathalie.cammas@francetelecom.com)
  Bool						m_bQualityLevelsEstimation;
  //}}Quality level estimation and modified truncation- JVTO044 and m12007
  Bool                      m_bExtendedPriorityId;
  UInt                      m_uiNumSimplePris;
  UInt                      m_uiTemporalLevelList[1 << PRI_ID_BITS];
  UInt                      m_uiDependencyIdList [1 << PRI_ID_BITS];
  UInt                      m_uiQualityLevelList [1 << PRI_ID_BITS];
  EncoderConfigLineBase*    m_pEncoderLines[MAX_CONFIG_PARAMS];
  EncoderConfigLineBase*    m_pLayerLines  [MAX_CONFIG_PARAMS];

  //{{Adaptive GOP structure
  // --ETRI & KHU
  UInt	m_uiWriteGOPMode;
  UInt	m_uiUseAGS;
  std::string               m_cGOPModeFilename;
  //}}Adaptive GOP structure
};

#if defined( MSYS_WIN32 )
# pragma warning( default: 4275 )
#endif


H264AVC_NAMESPACE_END


#endif // !defined(AFX_CODINGPARAMETER_H__8403A680_A65D_466E_A411_05C3A7C0D59F__INCLUDED_)
