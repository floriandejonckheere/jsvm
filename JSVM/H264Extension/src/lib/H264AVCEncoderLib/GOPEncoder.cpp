
#include "H264AVCEncoderLib.h"
#include "GOPEncoder.h"

#include "SliceEncoder.h"
#include "CreaterH264AVCEncoder.h"
#include "ControlMngH264AVCEncoder.h"
#include "RateDistortionIf.h"
#include "H264AVCCommonLib/ParameterSetMng.h"
#include "H264AVCCommonLib/Sei.h"


#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "H264AVCCommonLib/CFMO.h"

//JVT-U106 Behaviour at slice boundaries{
#include "H264AVCCommonLib/ReconstructionBypass.h"
//JVT-U106 Behaviour at slice boundaries}
// JVT-V068 {
#include "Scheduler.h"
// JVT-V068 }

// JVT-W043 {
#include "RateCtlBase.h"
#include "RateCtlQuadratic.h"
// JVT-W043 }

H264AVC_NAMESPACE_BEGIN



#define FACTOR_22_HP  0.70710678118654752440084436210485  //sqrt( 1.0/ 2.0)
#define FACTOR_22_LP  1.4142135623730950488016887242097   //sqrt( 2.0/ 1.0)
#define FACTOR_53_HP  0.84779124789065851738306954082825  //sqrt(23.0/32.0)
#define FACTOR_53_LP  1.2247448713915890490986420373529   //sqrt( 3.0/ 2.0)


LayerEncoder::LayerEncoder()
//----- references -----
: m_pcSPS                           ( 0 )
, m_pcPPSLP                         ( 0 )
, m_pcPPSHP                         ( 0 )
, m_pcYuvFullPelBufferCtrl          ( 0 )
, m_pcYuvHalfPelBufferCtrl          ( 0 )
, m_pcPocCalculator                 ( 0 )
, m_pcH264AVCEncoder                ( 0 )
, m_pcSliceEncoder                  ( 0 )
, m_pcNalUnitEncoder                ( 0 )
, m_pcLoopFilter                    ( 0 )
, m_pcQuarterPelFilter              ( 0 )
, m_pcMotionEstimation              ( 0 )
//----- fixed control parameters -----
, m_bTraceEnable                    ( true )
, m_bFrameMbsOnlyFlag               ( true )
, m_uiDependencyId                       ( 0 )
, m_uiScalableLayerId								( 0 )
, m_uiBaseLayerId                   ( MSYS_UINT_MAX )
, m_uiBaseQualityLevel              ( 15 )
, m_uiFrameWidthInMb                ( 0 )
, m_uiFrameHeightInMb               ( 0 )
, m_uiMaxGOPSize                    ( 0 )
, m_uiDecompositionStages           ( 0 )
, m_uiTemporalResolution            ( 0 )
, m_uiNotCodedStages                ( 0 )
, m_uiFrameDelay                    ( 0 )
, m_uiMaxNumRefFrames               ( 0 )
, m_uiLowPassIntraPeriod            ( 0 )
, m_uiNumMaxIter                    ( 0 )
, m_uiIterSearchRange               ( 0 )
, m_iMaxDeltaQp                     ( 0 )
, m_bH264AVCCompatible              ( true  )
, m_bInterLayerPrediction           ( true  )
, m_bAdaptivePrediction             ( true  )
, m_bWriteSubSequenceSei            ( false )
, m_dBaseQPResidual                 ( 0.0 )
, m_uiFilterIdc                     ( 0 )
, m_iAlphaOffset                    ( 0 )
, m_iBetaOffset                     ( 0 )
, m_bLoadMotionInfo                 ( false )
, m_bSaveMotionInfo                 ( false )
, m_pMotionInfoFile                 ( 0 )
//----- variable control parameters -----
, m_bInitDone                       ( false )
, m_bFirstGOPCoded                  ( false )
, m_uiGOPSize                       ( 0 )
, m_uiFrameCounter                  ( 0 )
, m_uiFrameNum                      ( 0 )
, m_uiIdrPicId											( 0 ) //EIDR 0619
, m_uiGOPNumber                     ( 0 )
//----- frame memories -----
, m_papcFrame                       ( 0 )
, m_papcELFrame                     ( 0 )
, m_pcResidual                      ( 0 )
, m_pcSubband                       ( 0 )
//TMM_WP
, m_bBaseLayerWp                    (false)
//TMM_WP
, m_pcAnchorFrameOriginal           ( 0 )
, m_pcAnchorFrameReconstructed      ( 0 )
, m_pcBaseLayerFrame                ( 0 )
, m_pcBaseLayerResidual             ( 0 )
//----- control data arrays -----
, m_pacControlData                  ( 0 )
, m_pcBaseLayerCtrl                 ( 0 )
, m_pcBaseLayerCtrlField            ( 0 )
//----- auxiliary buffers -----
, m_uiWriteBufferSize               ( 0 )
, m_pucWriteBuffer                  ( 0 )
//----- PSNR & rate -----
, m_fOutputFrameRate                ( 0.0 )
, m_uiParameterSetBits              ( 0 )
//--- FGS ---
, m_pcResizeParameters              ( 0 )//TMM_ESS
, m_pESSFile                        ( 0 )
// JVT-Q065 EIDR{
, m_iIDRPeriod					        	  ( 0 )
// JVT-Q065 EIDR}
, m_iLayerIntraPeriod               ( 0 )
, m_bLARDOEnable                    ( false ) //JVT-R057 LA-RDO
, m_uiEssRPChkEnable								( 0 )
, m_uiMVThres												( 20 )
, m_uiNonRequiredWrite				      ( 0 )  //NonRequired JVT-Q066 (06-04-08)
, m_uiPreAndSuffixUnitEnable				      ( 0 ) //JVT-S036 lsj
, m_uiMMCOBaseEnable						    ( 0 ) //JVT-S036 lsj
// JVT-S054 (ADD) ->
, m_puiFirstMbInSlice               ( 0 )
// JVT-S054 (ADD) <-
//S051{
, m_uiTotalFrame					          ( 0 )
, m_auiFrameBits				          	( NULL )
, m_uiAnaSIP						            ( 0 )
, m_bEncSIP						            	( false )
, m_cInSIPFileName					        ( "none" )
, m_cOutSIPFileName					        ( "none" )
//S051}
//JVT-T054{
, m_uiLayerCGSSNR                   ( 0 )
, m_uiQualityLevelCGSSNR            ( 0 )
, m_uiBaseLayerCGSSNR               ( MSYS_UINT_MAX )
, m_uiBaseQualityLevelCGSSNR        ( 0 )
//JVT-T054}
// JVT-U085 LMI
, m_bTlevelNestingFlag              (true)
// JVT-U116 W062 LMI
, m_bTl0DepRepIdxEnable            (false)
//JVT-U106 Behaviour at slice boundaries{
, m_bCIUFlag                       (false)
//JVT-U106 Behaviour at slice boundaries}
, m_bGOPInitialized                ( false )
, m_uiMGSKeyPictureControl( 0 )
, m_bHighestMGSLayer( false )
, m_pcRedundantCtrl (0)//RPIC bug fix
, m_pcRedundant1Ctrl (0)//RPIC bug fix
, m_uiLastCodedFrameIdInGOP ( MSYS_UINT_MAX )
, m_uiLastCodedTemporalId   ( MSYS_UINT_MAX )
, m_bBotFieldFirst          ( false )
, m_bUseLongTermPics        ( false )
, m_iMaxLongTermFrmIdx      ( -1 )
, m_uiNextTL0LongTermIdx    (  0 )
, m_uiPicCodingType         ( 0 )
, m_uiFrameIdWithSmallestTId( 0 )
, m_uiNumFramesLeftInGOP    ( 0 )
{
  ::memset( m_abIsRef,                      0x00, sizeof( m_abIsRef                     ) );
  ::memset( m_apcFrameTemp,                 0x00, sizeof( m_apcFrameTemp                ) );
  ::memset( m_auiPicCodingType,             0x00, sizeof( m_auiPicCodingType            ) );
  ::memset( m_auiFrameIdToTemporalId,       0x00, sizeof( m_auiFrameIdToTemporalId      ) );
  ::memset( m_auiCodingIndexToFrameId,      0x00, sizeof( m_auiCodingIndexToFrameId     ) );
  ::memset( m_abMaxLongTermIdxSend,         0x00, sizeof( m_abMaxLongTermIdxSend        ) );
  ::memset( m_auiFrameIdToFrameNumOrLTIdx,  0x00, sizeof( m_auiFrameIdToFrameNumOrLTIdx ) );
  ::memset( m_apcBaseFrame,                 0x00, sizeof( m_apcBaseFrame                ) );

  m_apabBaseModeFlagAllowedArrays[0] = 0;
  m_apabBaseModeFlagAllowedArrays[1] = 0;

  UInt ui;
  for( ui = 0; ui <= MAX_DSTAGES; ui++ )
  {
    m_auiNumFramesCoded [ui]  = 0;
    m_adPSNRSumY        [ui]  = 0.0;
    m_adPSNRSumU        [ui]  = 0.0;
    m_adPSNRSumV        [ui]  = 0.0;
  }
  m_uiNewlyCodedBits      = 0;
}





LayerEncoder::~LayerEncoder()
{
}





ErrVal
LayerEncoder::create( LayerEncoder*& rpcLayerEncoder )
{
  rpcLayerEncoder = new LayerEncoder;
  ROT( NULL == rpcLayerEncoder );
  return Err::m_nOK;
}





ErrVal
LayerEncoder::destroy()
{
  delete this;
  return Err::m_nOK;
}





ErrVal
LayerEncoder::init( CodingParameter*   pcCodingParameter,
                   LayerParameters*   pcLayerParameters,
                   H264AVCEncoder*    pcH264AVCEncoder,
                   SliceEncoder*      pcSliceEncoder,
                   LoopFilter*        pcLoopFilter,
                   PocCalculator*     pcPocCalculator,
                   NalUnitEncoder*    pcNalUnitEncoder,
                   YuvBufferCtrl*     pcYuvFullPelBufferCtrl,
                   YuvBufferCtrl*     pcYuvHalfPelBufferCtrl,
                   QuarterPelFilter*  pcQuarterPelFilter,
                   MotionEstimation*  pcMotionEstimation
           //JVT-U106 Behaviour at slice boundaries{
           ,ReconstructionBypass* pcReconstructionBypass
           //JVT-U106 Behaviour at slice boundaries}
                  // JVT-V068 {
                  ,StatBuf<Scheduler*, MAX_SCALABLE_LAYERS>* apcScheduler
                  // JVT-V068 }
           )
{
  ROF( pcCodingParameter );
  ROF( pcLayerParameters );
  ROF( pcH264AVCEncoder );
  ROF( pcSliceEncoder );
  ROF( pcLoopFilter );
  ROF( pcPocCalculator );
  ROF( pcNalUnitEncoder );
  ROF( pcYuvFullPelBufferCtrl );
  ROF( pcYuvHalfPelBufferCtrl );
  ROF( pcQuarterPelFilter );
  ROF( pcMotionEstimation );
  //JVT-U106 Behaviour at slice boundaries{
  ROF( pcReconstructionBypass );
  //JVT-U106 Behaviour at slice boundaries}

  //----- references -----
  m_pcSPS                   = 0;
  m_pcPPSLP                 = 0;
  m_pcPPSHP                 = 0;
  m_pcYuvFullPelBufferCtrl  = pcYuvFullPelBufferCtrl;
  m_pcYuvHalfPelBufferCtrl  = pcYuvHalfPelBufferCtrl;
  m_pcPocCalculator         = pcPocCalculator;
  m_pcH264AVCEncoder        = pcH264AVCEncoder;
  m_pcSliceEncoder          = pcSliceEncoder;
  m_pcNalUnitEncoder        = pcNalUnitEncoder;
  m_pcLoopFilter            = pcLoopFilter;
  m_pcQuarterPelFilter      = pcQuarterPelFilter;
  m_pcMotionEstimation      = pcMotionEstimation;
  //JVT-U106 Behaviour at slice boundaries{
  m_pcReconstructionBypass  = pcReconstructionBypass;
  //JVT-U106 Behaviour at slice boundaries}
  m_pcLayerParameters       = pcLayerParameters;

  //----- fixed control parameters -----
  m_bInterlaced             = pcLayerParameters->isInterlaced();
  // JVT-V068 {
  m_bEnableHrd              = pcCodingParameter->getEnableNalHRD() || pcCodingParameter->getEnableVclHRD();
  m_apcScheduler            = apcScheduler;
  // JVT-V068 }
  // JVT-W049 {
  m_uiNumberLayersCnt       = pcCodingParameter->getNumberOfLayers();
  // JVT-W049 }
  m_uiDependencyId               = pcLayerParameters->getDependencyId                 ();
  m_uiBaseLayerId           = pcLayerParameters->getBaseLayerId             ();
  m_uiBaseQualityLevel      = pcLayerParameters->getBaseQualityLevel        ();

// JVT-Q065 EIDR{
  m_iIDRPeriod				= pcLayerParameters->getIDRPeriod				();
// JVT-Q065 EIDR}
  m_iLayerIntraPeriod = pcLayerParameters->getLayerIntraPeriod();
// JVT-U085 LMI
  m_bTlevelNestingFlag      = pcCodingParameter->getTlevelNestingFlag();
// JVT-U116 W062 LMI
  m_bTl0DepRepIdxEnable = pcCodingParameter->getTl0DepRepIdxSeiEnable();

  m_bDiscardable = pcLayerParameters->isDiscardable(); //DS_FIX_FT_09_2007
  m_uiQLDiscardable = pcLayerParameters->getQLDiscardable(); //DS_FIX_FT_09_2007

	m_uiEssRPChkEnable = pcCodingParameter->getEssRPChkEnable();
	m_uiMVThres = pcCodingParameter->getMVThres();

  m_uiDecompositionStages   = pcLayerParameters->getDecompositionStages     ();
  m_uiTemporalResolution    = pcLayerParameters->getTemporalResolution      ();
  m_uiNotCodedStages        = pcLayerParameters->getNotCodedStages          ();
  m_uiFrameDelay            = pcLayerParameters->getFrameDelay              ();
  m_uiMaxNumRefFrames       = pcCodingParameter->getNumRefFrames            ();
  m_uiLowPassIntraPeriod    = pcCodingParameter->getIntraPeriodLowPass      ();
  m_uiNumMaxIter            = pcCodingParameter->getMotionVectorSearchParams().getNumMaxIter      ();
  m_uiIterSearchRange       = pcCodingParameter->getMotionVectorSearchParams().getIterSearchRange ();
  m_iMaxDeltaQp             = pcLayerParameters->getMaxAbsDeltaQP           ();
  m_bH264AVCCompatible      = m_uiDependencyId == 0; //bug-fix suffix
  m_bInterLayerPrediction   = pcLayerParameters->getInterLayerPredictionMode()  > 0;
  m_bAdaptivePrediction     = pcLayerParameters->getInterLayerPredictionMode()  > 1;
  m_bBiPred8x8Disable       = pcLayerParameters->getBiPred8x8Disable() > 0;
  m_bMCBlks8x8Disable       = pcLayerParameters->getMCBlks8x8Disable() > 0;
  m_bBotFieldFirst          = pcLayerParameters->getBotFieldFirst() > 0;
  m_bUseLongTermPics        = pcLayerParameters->getUseLongTerm() > 0;
  m_iMaxLongTermFrmIdx      = -1;
  m_uiNextTL0LongTermIdx    = 0;
  if( m_bUseLongTermPics )
  {
    m_iMaxLongTermFrmIdx    = m_uiDecompositionStages - m_uiNotCodedStages + ( pcLayerParameters->getPAff() ? 1 : 0 );
  }
  m_uiPicCodingType         = pcLayerParameters->getPicCodingType();
  m_bForceReOrderingCommands= pcLayerParameters->getForceReorderingCommands ()  > 0;
  m_bWriteSubSequenceSei    = pcCodingParameter->getBaseLayerMode           ()  > 1 && m_uiDependencyId == 0;

  m_bSameResBL              = ( m_uiBaseLayerId != MSYS_UINT_MAX &&
                                pcCodingParameter->getLayerParameters( m_uiBaseLayerId ).getFrameWidthInSamples () == pcLayerParameters->getFrameWidthInSamples () &&
                                pcCodingParameter->getLayerParameters( m_uiBaseLayerId ).getFrameHeightInSamples() == pcLayerParameters->getFrameHeightInSamples() &&
                                pcLayerParameters->getResizeParameters().m_iExtendedSpatialScalability == ESS_NONE );

  m_bSameResEL              = false;
  {
    UInt uiLId;
    for( uiLId = m_uiDependencyId + 1; uiLId < pcCodingParameter->getNumberOfLayers() && ! m_bSameResEL; uiLId++ )
    {
      LayerParameters& rcEL = pcCodingParameter->getLayerParameters( uiLId );
      if( rcEL.getBaseLayerId() == m_uiDependencyId )
      {
        m_bSameResEL        = ( pcLayerParameters->getFrameWidthInSamples () == rcEL.getFrameWidthInSamples () &&
                                pcLayerParameters->getFrameHeightInSamples() == rcEL.getFrameHeightInSamples() &&
                                rcEL.getResizeParameters().m_iExtendedSpatialScalability == ESS_NONE );
      }
    }
  }
  m_bMGS                    = pcCodingParameter->getCGSSNRRefinement    () == 1 && ( m_bSameResBL || m_bSameResEL );
  m_uiEncodeKeyPictures     = ( pcLayerParameters->isIntraOnly() ? 0 : pcCodingParameter->getEncodeKeyPictures() );
  m_uiMGSKeyPictureControl  = pcCodingParameter->getMGSKeyPictureControl();
  m_bHighestMGSLayer        = m_bMGS && !m_bSameResEL;
  m_uiLastCodedFrameIdInGOP = MSYS_UINT_MAX;
  m_uiLastCodedTemporalId   = MSYS_UINT_MAX;

  RNOK( xInitCodingOrder( m_uiDecompositionStages, m_uiTemporalResolution, m_uiFrameDelay ) );
  for( UInt uiTId = 0; uiTId <= MAX_DSTAGES; uiTId++ )
  {
    m_acActiveRefListFrameNum[uiTId].clear();
  }
  m_cActiveRefBasePicListFrameNum.clear();

  m_bExplicitQPCascading      = pcLayerParameters->getExplicitQPCascading() != 0;
  for( UInt uiTTL = 0; uiTTL < MAX_TEMP_LEVELS; uiTTL++ )
  {
    m_adDeltaQPTLevel[uiTTL]  = pcLayerParameters->getDeltaQPTLevel( uiTTL );
  }

  //JVT-V079 Low-complexity MB mode decision {
  if ( m_uiDependencyId==0 )
    m_pcSliceEncoder->getMbEncoder()->setLowComplexMbEnable ( m_uiDependencyId, (pcLayerParameters->getLowComplexMbEnable ()==1) );
  else
    m_pcSliceEncoder->getMbEncoder()->setLowComplexMbEnable ( m_uiDependencyId, false );
  //JVT-V079 Low-complexity MB mode decision }

  //JVT-R057 LA-RDO{
  if(pcCodingParameter->getLARDOEnable()!=0)
  {
    static UInt auiPLR[5];
    static UInt aauiSize[5][2];
    static Double dRatio[5][2];
    auiPLR[m_uiDependencyId]      = pcLayerParameters->getPLR                     ();
    m_bLARDOEnable                = pcCodingParameter->getLARDOEnable()==0? false:true;
    aauiSize[m_uiDependencyId][0] = pcLayerParameters->getFrameWidthInSamples ();
    aauiSize[m_uiDependencyId][1] = pcLayerParameters->getFrameHeightInSamples();
    if(m_uiDependencyId==0||pcLayerParameters->getBaseLayerId()==MSYS_UINT_MAX)
    {
      dRatio[m_uiDependencyId][0]=1;
      dRatio[m_uiDependencyId][1]=1;
    }
    else
    {
      dRatio[m_uiDependencyId][0]=(Double)aauiSize[m_uiDependencyId][0]/aauiSize[pcLayerParameters->getBaseLayerId()][0];
      dRatio[m_uiDependencyId][1]=(Double)aauiSize[m_uiDependencyId][1]/aauiSize[pcLayerParameters->getBaseLayerId()][1];
    }
    m_pcSliceEncoder->getMbEncoder()->setRatio(dRatio);
    m_pcSliceEncoder->getMbEncoder()->setPLR(auiPLR);
    pcLayerParameters->setContrainedIntraForLP();
  }
  //JVT-R057 LA-RDO}

  m_uiPreAndSuffixUnitEnable = pcCodingParameter->getPreAndSuffixUnitEnable();//JVT-S036 lsj
  m_uiMMCOBaseEnable   = pcCodingParameter->getMMCOBaseEnable();  //JVT-S036 lsj

  // TMM_ESS
  m_pcResizeParameters  = &pcLayerParameters->getResizeParameters();
  m_pESSFile            = 0;
  if( m_pcResizeParameters->m_iExtendedSpatialScalability == ESS_PICT )
  {
    m_pESSFile = fopen( pcLayerParameters->getESSFilename().c_str(), "r" );
    if( !m_pESSFile )
    {
      printf( "failed to open resize parameter file %s\n", pcLayerParameters->getESSFilename().c_str() );
      ROT(1);
    }
  }

  for( UInt uiStage = 0; uiStage < MAX_DSTAGES; uiStage++ )
  {
    m_adBaseQpLambdaMotion[uiStage] = pcLayerParameters->getQpModeDecision( uiStage );
  }
  m_dBaseQpLambdaMotionLP   = pcLayerParameters->getQpModeDecisionLP          ();
  m_dBaseQPResidual         = pcLayerParameters->getBaseQpResidual            ();

  m_uiFilterIdc             = pcCodingParameter->getLoopFilterParams          ().getFilterIdc       ();
  m_iAlphaOffset            = pcCodingParameter->getLoopFilterParams          ().getAlphaOffset     ();
  m_iBetaOffset             = pcCodingParameter->getLoopFilterParams          ().getBetaOffset      ();
  m_uiInterLayerFilterIdc   = pcCodingParameter->getInterLayerLoopFilterParams().getFilterIdc       ();
  m_iInterLayerAlphaOffset  = pcCodingParameter->getInterLayerLoopFilterParams().getAlphaOffset     ();
  m_iInterLayerBetaOffset   = pcCodingParameter->getInterLayerLoopFilterParams().getBetaOffset      ();

  m_bLoadMotionInfo         = pcLayerParameters->getMotionInfoMode            () == 1;
  m_bSaveMotionInfo         = pcLayerParameters->getMotionInfoMode            () == 2;
  m_pMotionInfoFile         = 0;

  if( m_bLoadMotionInfo )
  {
    m_pMotionInfoFile = ::fopen( pcLayerParameters->getMotionInfoFilename().c_str(), "rb" );
    if( ! m_pMotionInfoFile )
    {
      fprintf( stderr, "\nCANNOT OPEN MOTION INFO FILE \"%s\"\n\n", pcLayerParameters->getMotionInfoFilename().c_str() );
      return Err::m_nERR;
    }
  }
  else if( m_bSaveMotionInfo )
  {
    m_pMotionInfoFile = ::fopen( pcLayerParameters->getMotionInfoFilename().c_str(), "wb" );
    if( ! m_pMotionInfoFile )
    {
      fprintf( stderr, "\nCANNOT OPEN MOTION INFO FILE \"%s\"\n\n", pcLayerParameters->getMotionInfoFilename().c_str() );
      return Err::m_nERR;
    }
  }


  //----- PSNR and rate -----
  m_fOutputFrameRate          = pcLayerParameters->getOutputFrameRate();
  m_uiParameterSetBits        = 0;

  UInt ui;
  for( ui = 0; ui <= MAX_DSTAGES; ui++ )
  {
    m_auiNumFramesCoded [ui]  = 0;
    m_adPSNRSumY        [ui]  = 0.0;
    m_adPSNRSumU        [ui]  = 0.0;
    m_adPSNRSumV        [ui]  = 0.0;
  }
  m_uiNewlyCodedBits  = 0;

  // JVT-S054 (ADD) ->
  if( pcLayerParameters->m_bSliceDivisionFlag )
  {
    if( m_puiFirstMbInSlice != NULL )
      free(m_puiFirstMbInSlice);
    m_puiFirstMbInSlice = (UInt*)malloc((pcLayerParameters->m_uiNumSliceMinus1+1)*sizeof(UInt));
    memcpy( m_puiFirstMbInSlice, pcLayerParameters->m_puiFirstMbInSlice, (pcLayerParameters->m_uiNumSliceMinus1+1)*sizeof(UInt) );
  }
  // JVT-S054 (ADD) <-

  //S051{
  m_uiTotalFrame	= pcCodingParameter->getTotalFrames();
  m_uiAnaSIP		= pcLayerParameters->getAnaSIP();
  m_cOutSIPFileName	= pcLayerParameters->getOutSIPFileName();
  if(m_uiAnaSIP==1)
    m_bInterLayerPrediction=true;
  if(m_uiAnaSIP==2)
    m_bInterLayerPrediction=m_bAdaptivePrediction=false;

  if(pcCodingParameter->getNumberOfLayers() > m_uiDependencyId+1)
  {
    m_bEncSIP			= pcCodingParameter->getLayerParameters( m_uiDependencyId+1).getEncSIP();
    m_cInSIPFileName	= pcCodingParameter->getLayerParameters( m_uiDependencyId+1).getInSIPFileName();
  }
  //S051}

  //JVT-U106 Behaviour at slice boundaries{
  m_bCIUFlag=pcCodingParameter->getCIUFlag()!=0;
  //JVT-U106 Behaviour at slice boundaries}
  {
    Int iMaxNumMb = pcLayerParameters->getFrameWidthInMbs() * pcLayerParameters->getFrameHeightInMbs();
    m_apabBaseModeFlagAllowedArrays[0] = new Bool [iMaxNumMb];
    m_apabBaseModeFlagAllowedArrays[1] = new Bool [iMaxNumMb];
    ROF( m_apabBaseModeFlagAllowedArrays[0] );
    ROF( m_apabBaseModeFlagAllowedArrays[1] );
  }

  m_uiFramesInCompleteGOPsProcessed = 0;
  m_uiMinScalableLayer              = 0;
  for( UInt uiBaseLayerId = 0; uiBaseLayerId < m_uiDependencyId; uiBaseLayerId++ )
  {
    m_uiMinScalableLayer += pcCodingParameter->getLayerParameters( uiBaseLayerId ).getDecompositionStages() -
      pcCodingParameter->getLayerParameters( uiBaseLayerId ).getNotCodedStages     () + 1U;
  }
	//JVT-X046 {
  m_uiSliceMode     = pcLayerParameters->getSliceMode();
  m_uiSliceArgument = pcLayerParameters->getSliceArgument();
	if( m_uiSliceMode == 2 )
	{
		m_uiSliceArgument = m_uiSliceArgument*8;
	}
  else if( m_uiSliceMode == 0 || m_uiSliceMode > 2 )
  {
    m_uiSliceMode     = 0;
    m_uiSliceArgument = MSYS_UINT_MAX;
  }
  //JVT-X046 }

  return Err::m_nOK;
}


ErrVal
LayerEncoder::xInitCodingOrder( UInt uiDecompositionStages, UInt uiTemporalSubSampling, UInt uiMaxFrameDelay )
{
  ROF( uiDecompositionStages  + uiTemporalSubSampling <= MAX_DSTAGES );
  uiDecompositionStages      += uiTemporalSubSampling;
  const UInt  uiMaxSize       = ( 1U << MAX_DSTAGES ) + 1U;
  UInt        uiMaxGOPSize    = ( 1U << uiDecompositionStages );
  UInt        auiTemporalId  [ uiMaxSize ];
  UInt        auiRequiredFId [ uiMaxSize ];
  Bool        abIndexAssigned[ uiMaxSize ];
  //===== restrict input delay and determine period parameters =====
  uiMaxFrameDelay = min( uiMaxFrameDelay, uiMaxGOPSize - 1 );
  UInt uiPeriod   = 0;
  for( uiPeriod   = 1; uiMaxFrameDelay >= uiPeriod; uiPeriod <<= 1 );
  UInt uiP0Size   = uiPeriod - ( uiMaxFrameDelay + 1 );
  //===== init arrays =====
  for( UInt uiIdx = 0; uiIdx < uiMaxSize; uiIdx++ )
  {
    m_auiFrameIdToTemporalId[uiIdx] = 0;
    auiTemporalId           [uiIdx] = 0;
    auiRequiredFId          [uiIdx] = 0;
    abIndexAssigned         [uiIdx] = false;
  }
  //===== set temporal level (for the inside of a virtual GOP) =====
  for( UInt uiTemporalId = 1; uiTemporalId <= uiDecompositionStages; uiTemporalId++ )
  {
    UInt uiOffset = ( 1U << ( uiDecompositionStages - uiTemporalId ) );
    for( UInt uiFrameIdInGOP = uiOffset; uiFrameIdInGOP <= uiMaxGOPSize; uiFrameIdInGOP += ( uiOffset << 1 ) )
    {
      auiTemporalId[ uiFrameIdInGOP ] = uiTemporalId;
    }
  }
  //===== initialize the array required =====
  for( UInt uiFrameId = 1; uiFrameId < uiMaxGOPSize; uiFrameId++ )
  {
    Bool bSet = false;
    for( UInt uiNextId = uiFrameId + 1; !bSet; uiNextId++ )
    {
      if( auiTemporalId[uiNextId] < auiTemporalId[uiFrameId] )
      {
        auiRequiredFId[uiFrameId] = uiNextId;
        bSet                      = true;
      }
    }
  }
  //===== update the array required taking into account the delay =====
  for( UInt uiPeriodOffset = 1; uiPeriodOffset <= uiMaxGOPSize; uiPeriodOffset += uiPeriod )
  {
    for( UInt uiCnt = 0; uiCnt < 2; uiCnt++ )
    {
      UInt uiMinFrameIdInGOP = uiPeriodOffset + ( uiCnt ? uiP0Size : 0 );
      UInt uiMaxFrameIdInGOP = uiPeriodOffset + ( uiCnt ? uiPeriod : uiP0Size );
      for( UInt uiFrameId = uiMinFrameIdInGOP; uiFrameId < uiMaxFrameIdInGOP; uiFrameId++ )
      {
        if( auiRequiredFId[uiFrameId] >= uiMaxFrameIdInGOP )
        {
          auiRequiredFId[uiFrameId] = 0;
        }
      }
    }
  }
  //===== determine coding order =====
  UInt      uiCodingNumber = 0;
  for( UInt uiFrameIdInGOP = 0; uiFrameIdInGOP <= uiMaxGOPSize; uiFrameIdInGOP++ )
  {
    if( ! abIndexAssigned[ uiFrameIdInGOP ] )
    {
      // get list of required pictures
      UInt auiRequiredList[MAX_DSTAGES];
      UInt uiRequiredListSize = 0;
      for( UInt uiNextRequired = auiRequiredFId[uiFrameIdInGOP]; uiNextRequired && !abIndexAssigned[uiNextRequired]; uiNextRequired = auiRequiredFId[uiNextRequired] )
      {
        auiRequiredList[uiRequiredListSize++] = uiNextRequired;
      }
      // set required list elements
      while( uiRequiredListSize-- )
      {
        UInt uiNextId                                 = auiRequiredList[uiRequiredListSize];
        m_auiCodingIndexToFrameId [uiCodingNumber++]  = uiNextId;
        abIndexAssigned           [uiNextId]          = true;
      }
      // set current
      m_auiCodingIndexToFrameId   [uiCodingNumber++]  = uiFrameIdInGOP;
      abIndexAssigned             [uiFrameIdInGOP]    = true;
    }
  }
  ROF( uiCodingNumber == uiMaxGOPSize + 1 );
  //===== modify according to temporal subsampling =====
  UInt  uiTempResStep = ( 1U << uiTemporalSubSampling );
  UInt  uiNotCodeStep = ( 1U << m_uiNotCodedStages );
  UInt  uiModIdx      = 0;
  for( ; uiModIdx <= uiMaxGOPSize; uiModIdx++ )
  {
    UInt uiVirtFrameIdInGOP = m_auiCodingIndexToFrameId[ uiModIdx ];
    if(  uiVirtFrameIdInGOP % uiTempResStep )
    {
      m_auiCodingIndexToFrameId[ uiModIdx ] = MSYS_UINT_MAX;
    }
    else
    {
      UInt  uiFrameIdInGOP                          = uiVirtFrameIdInGOP >> uiTemporalSubSampling;
      m_auiFrameIdToTemporalId  [ uiFrameIdInGOP ]  = auiTemporalId[ uiVirtFrameIdInGOP ];
      Bool  bNotCoded                               = ( ( uiFrameIdInGOP % uiNotCodeStep ) != 0 );
      m_auiCodingIndexToFrameId [ uiModIdx ]        = ( bNotCoded ? MSYS_UINT_MAX : uiFrameIdInGOP );
    }
  }
  for( ; uiModIdx < uiMaxSize; uiModIdx++ )
  {
    m_auiCodingIndexToFrameId[ uiModIdx ] = MSYS_UINT_MAX;
  }
  return Err::m_nOK;
}


ErrVal
LayerEncoder::initParameterSets( const SequenceParameterSet& rcSPS,
                                const PictureParameterSet&  rcPPSLP,
                                const PictureParameterSet&  rcPPSHP )
{
  //===== set references =====
  m_pcSPS                 = &rcSPS;
  m_pcPPSLP               = &rcPPSLP;
  m_pcPPSHP               = &rcPPSHP;

  //===== get and set relevant parameters =====
  m_bFrameMbsOnlyFlag     = rcSPS.getFrameMbsOnlyFlag ();
  m_uiFrameWidthInMb      = rcSPS.getFrameWidthInMbs  ();
  m_uiFrameHeightInMb     = rcSPS.getFrameHeightInMbs ();
  m_uiMaxGOPSize          = ( 1 << m_uiDecompositionStages );

  //===== re-allocate dynamic memory =====
  RNOK( xDeleteData()        );
  RNOK( xCreateData( rcSPS ) );

  //===== initialize some parameters =====
  m_bInitDone             = true;
  m_bFirstGOPCoded        = false;
  m_uiFrameCounter        = 0;
  m_uiFrameNum            = 0;
  m_uiGOPNumber           = 0;
  ::memset( m_abIsRef, 0x00, sizeof( m_abIsRef ) );

  return Err::m_nOK;
}




ErrVal
LayerEncoder::addParameterSetBits( UInt uiParameterSetBits )
{
  m_uiParameterSetBits += uiParameterSetBits;
  return Err::m_nOK;
}




ErrVal
LayerEncoder::xCreateData( const SequenceParameterSet& rcSPS )
{
  UInt uiIndex;

  //========== CREATE FRAME MEMORIES ==========
  ROFS   ( ( m_papcFrame                     = new Frame* [ m_uiMaxGOPSize + 1 ]      ) );
  if( m_uiMGSKeyPictureControl && ! m_bHighestMGSLayer )
  {
    ROFS ( ( m_papcELFrame                   = new Frame* [ m_uiMaxGOPSize + 1 ]      ) );
  }

  for( uiIndex = 0; uiIndex <= m_uiMaxGOPSize; uiIndex++ )
  {
    RNOK( Frame::create( m_papcFrame   [ uiIndex ], *m_pcYuvFullPelBufferCtrl, *m_pcYuvHalfPelBufferCtrl, FRAME, 0 ) );
    RNOK  (   m_papcFrame         [ uiIndex ] ->init        () );

    if( m_papcELFrame )
    {
      RNOK( Frame::create( m_papcELFrame[ uiIndex ], *m_pcYuvFullPelBufferCtrl, *m_pcYuvHalfPelBufferCtrl, FRAME, 0 ) );
      RNOK(   m_papcELFrame    [ uiIndex ] ->init        () );
    }
  }

  for( uiIndex = 0; uiIndex < NUM_TMP_FRAMES;  uiIndex++ )
  {
    RNOK( Frame::create( m_apcFrameTemp      [ uiIndex ], *m_pcYuvFullPelBufferCtrl, *m_pcYuvHalfPelBufferCtrl, FRAME, 0 ) );
    RNOK  (   m_apcFrameTemp      [ uiIndex ] ->init        () );
  }

  RNOK( Frame::create( m_pcResidual, *m_pcYuvFullPelBufferCtrl, *m_pcYuvHalfPelBufferCtrl, FRAME, 0 ) );
  RNOK  (   m_pcResidual->init        () );
  RNOK( Frame::create( m_pcSubband, *m_pcYuvFullPelBufferCtrl, *m_pcYuvHalfPelBufferCtrl, FRAME, 0 ) );
  RNOK  (   m_pcSubband->init        () );
  RNOK( Frame::create( m_apcBaseFrame[0], *m_pcYuvFullPelBufferCtrl, *m_pcYuvHalfPelBufferCtrl, FRAME, 0 ) );
  RNOK( Frame::create( m_apcBaseFrame[1], *m_pcYuvFullPelBufferCtrl, *m_pcYuvHalfPelBufferCtrl, FRAME, 0 ) );
  RNOK    (   m_apcBaseFrame[0]   ->init        () );
  RNOK    (   m_apcBaseFrame[1]   ->init        () );
  RNOK( Frame::create( m_pcAnchorFrameOriginal      , *m_pcYuvFullPelBufferCtrl, *m_pcYuvHalfPelBufferCtrl, FRAME, 0 ) );
  RNOK    (   m_pcAnchorFrameOriginal         ->init        () );
  RNOK( Frame::create( m_pcAnchorFrameReconstructed , *m_pcYuvFullPelBufferCtrl, *m_pcYuvHalfPelBufferCtrl, FRAME, 0 ) );
  RNOK    (   m_pcAnchorFrameReconstructed    ->init        () );
  RNOK( Frame::create( m_pcBaseLayerFrame           , *m_pcYuvFullPelBufferCtrl, *m_pcYuvHalfPelBufferCtrl, FRAME, 0 ) );
  RNOK    (   m_pcBaseLayerFrame              ->init        () );
  RNOK( Frame::create( m_pcBaseLayerResidual        , *m_pcYuvFullPelBufferCtrl, *m_pcYuvHalfPelBufferCtrl, FRAME, 0 ) );
  RNOK    (   m_pcBaseLayerResidual           ->init        () );

  ROFS ( ( m_pbFieldPicFlag = new Bool[ m_uiMaxGOPSize + 1 ] ));

  //========== CREATE MACROBLOCK DATA MEMORIES ==========
  ROFS   ( ( m_pacControlData  = new ControlData[ m_uiMaxGOPSize + 1 ] ) );

  for( uiIndex = 0; uiIndex <= m_uiMaxGOPSize; uiIndex++ )
  {
    MbDataCtrl*  pcMbDataCtrl                = 0;
    ROFS ( (     pcMbDataCtrl                = new MbDataCtrl  () ) );
    RNOK  (       pcMbDataCtrl                ->init            ( rcSPS ) );
    RNOK  (       m_pacControlData[ uiIndex ] . setMbDataCtrl   ( pcMbDataCtrl ) );

    Bool          bLowPass                    = ( ( uiIndex % ( 1 << m_uiDecompositionStages ) ) == 0 );
    SliceHeader*  pcSliceHeader               = 0;
    ROFS ( (     pcSliceHeader               = new SliceHeader ( *m_pcSPS, bLowPass ? *m_pcPPSLP : *m_pcPPSHP ) ) );
    RNOK  (       m_pacControlData[ uiIndex ] . setSliceHeader  (  pcSliceHeader, FRAME ) );

    if( m_pcLayerParameters->getPAff() )
    {
      ROFRS ( (   pcSliceHeader               = new SliceHeader ( *m_pcSPS, bLowPass ? *m_pcPPSLP : *m_pcPPSHP ) ), Err::m_nERR );
      RNOK  (     m_pacControlData[ uiIndex ] . setSliceHeader  ( pcSliceHeader, BOT_FIELD ) );
    }
  }

  ROFS   ( ( m_pcBaseLayerCtrl = new MbDataCtrl() ) );
  RNOK    (   m_pcBaseLayerCtrl ->init          ( rcSPS ) );

  ROFS   ( ( m_pcBaseLayerCtrlField = new MbDataCtrl() ) );
  RNOK    (   m_pcBaseLayerCtrlField ->init          ( rcSPS ) );

//RPIC bug fix
  ROFS ( (     m_pcRedundantCtrl                = new MbDataCtrl  () ) );
  RNOK  (       m_pcRedundantCtrl                ->init            ( rcSPS ) );
  ROFS ( (     m_pcRedundant1Ctrl                = new MbDataCtrl  () ) );
  RNOK  (       m_pcRedundant1Ctrl                ->init            ( rcSPS ) );
//RPIC bug fix

  //========== CREATE UPDATE WEIGHTS ARRAY and WRITE BUFFER ==========
  UInt  uiNum4x4Blocks      = m_uiFrameWidthInMb * m_uiFrameHeightInMb * 4 * 4;
  m_uiWriteBufferSize       = 3 * ( uiNum4x4Blocks * 4 * 4 );
  ROFS( ( m_pucWriteBuffer  = new UChar [ m_uiWriteBufferSize ] ) );
  ROT ( m_cDownConvert.init( m_uiFrameWidthInMb<<4, m_uiFrameHeightInMb<<4, 16 ) );

  //S051{
  ROFRS( m_auiFrameBits	=	new UInt[m_uiTotalFrame], Err::m_nERR );
  memset( m_auiFrameBits,0,sizeof(UInt)*m_uiTotalFrame);
  if(m_bEncSIP)
  {
    FILE* file=fopen(m_cInSIPFileName.c_str(),"rt");
    if(file==NULL)
    {
      printf("\nCan't open SIP file %s",m_cInSIPFileName.c_str());
      return Err::m_nOK;
    }
    while(!feof(file))
  {
    UInt tmp;
    fscanf(file,"%d",&tmp);
    m_cPocList.push_back(tmp);
  }
    fclose(file);
  }
  //S051}

  return Err::m_nOK;
}





ErrVal
LayerEncoder::xDeleteData()
{
  UInt uiIndex;

  //========== DELETE FRAME MEMORIES ==========
  if( m_papcFrame )
  {
    for( uiIndex = 0; uiIndex <= m_uiMaxGOPSize; uiIndex++ )
    {
      if( m_papcFrame[ uiIndex ] )
      {
    //JVT-R057 LA-RDO{
    if(m_bLARDOEnable)
       m_papcFrame[uiIndex]->uninitChannelDistortion();
    //JVT-R057 LA-RDO}
        RNOK(   m_papcFrame[ uiIndex ]->uninit() );
        RNOK( m_papcFrame[ uiIndex ]->destroy() );
        m_papcFrame[ uiIndex ] = 0;
      }
    }
    delete [] m_papcFrame;
    m_papcFrame = 0;
  }

  if( m_papcELFrame )
  {
    for( uiIndex = 0; uiIndex <= m_uiMaxGOPSize; uiIndex++ )
    {
      if( m_papcELFrame[ uiIndex ] )
      {
        RNOK(   m_papcELFrame[ uiIndex ]->uninit() );
        RNOK(   m_papcELFrame[ uiIndex ]->destroy() );
        m_papcELFrame[ uiIndex ] = 0;
      }
    }
    delete [] m_papcELFrame;
    m_papcELFrame = 0;
  }

  if( m_pcResidual )
  {
    RNOK( m_pcResidual->uninit() );
    RNOK( m_pcResidual->destroy() );
    m_pcResidual = 0;
  }
  if( m_pcSubband )
  {
    RNOK( m_pcSubband->uninit() );
    RNOK( m_pcSubband->destroy() );
    m_pcSubband = 0;
  }

  for( uiIndex = 0; uiIndex < NUM_TMP_FRAMES; uiIndex++ )
  {
    if( m_apcFrameTemp[ uiIndex ] )
    {
      RNOK(   m_apcFrameTemp[ uiIndex ]->uninit() );
      RNOK( m_apcFrameTemp[ uiIndex ]->destroy() );
      m_apcFrameTemp[ uiIndex ] = 0;
    }
  }

  for( UInt uiBaseIdx = 0; uiBaseIdx < 2; uiBaseIdx++ )
  {
    if( m_apcBaseFrame[uiBaseIdx] )
    {
      if(m_bLARDOEnable)
        m_apcBaseFrame[uiBaseIdx]->uninitChannelDistortion();
      // JVT-R057 LA-RDO}
      RNOK( m_apcBaseFrame[uiBaseIdx]->uninit() );
      RNOK( m_apcBaseFrame[uiBaseIdx]->destroy() );
      m_apcBaseFrame[uiBaseIdx] = 0;
    }
  }

  if( m_pcAnchorFrameOriginal )
  {
    RNOK(   m_pcAnchorFrameOriginal->uninit() );
    RNOK( m_pcAnchorFrameOriginal->destroy() );
    m_pcAnchorFrameOriginal = 0;
  }

  if( m_pcAnchorFrameReconstructed )
  {
    RNOK(   m_pcAnchorFrameReconstructed->uninit() );
    RNOK( m_pcAnchorFrameReconstructed->destroy() );
    m_pcAnchorFrameReconstructed = 0;
  }

  if( m_pcBaseLayerFrame )
  {
    RNOK(   m_pcBaseLayerFrame->uninit() );
    RNOK( m_pcBaseLayerFrame->destroy() );
    m_pcBaseLayerFrame = 0;
  }

  if( m_pcBaseLayerResidual )
  {
    RNOK(   m_pcBaseLayerResidual->uninit() );
    RNOK( m_pcBaseLayerResidual->destroy() );
    m_pcBaseLayerResidual = 0;
  }


  //========== DELETE MACROBLOCK DATA MEMORIES (and SLICE HEADER) ==========
  if( m_pacControlData )
  {
    for( uiIndex = 0; uiIndex <= m_uiMaxGOPSize; uiIndex++ )
    {
      MbDataCtrl*   pcMbDataCtrl  = m_pacControlData[ uiIndex ].getMbDataCtrl  ();
      if( pcMbDataCtrl )
      {
        RNOK( pcMbDataCtrl->uninit() );
      }
      delete pcMbDataCtrl;
      pcMbDataCtrl = 0 ;
      SliceHeader*  pcSliceHeader = m_pacControlData[ uiIndex ].getSliceHeader( FRAME );
      delete pcSliceHeader;
      pcSliceHeader = 0 ;

      if( m_pcLayerParameters->getPAff() )
      {
        pcSliceHeader = m_pacControlData[ uiIndex ].getSliceHeader( BOT_FIELD );
      delete pcSliceHeader;
        pcSliceHeader = 0;
      }
      if( m_pbFieldPicFlag )
      {
        delete [] m_pbFieldPicFlag;
        m_pbFieldPicFlag = 0;
      }
    }
    delete [] m_pacControlData;
    m_pacControlData = 0;
  }
//RPIC bug fix
	  if(m_pcRedundantCtrl)
  {
	  RNOK( m_pcRedundantCtrl->uninit() );
	  delete m_pcRedundantCtrl;
	  m_pcRedundantCtrl = 0 ;
  }
		  if(m_pcRedundant1Ctrl)
  {
	  RNOK( m_pcRedundant1Ctrl->uninit() );
	  delete m_pcRedundant1Ctrl;
	  m_pcRedundant1Ctrl = 0 ;
  }
//RPIC bug fix

  if( m_pcBaseLayerCtrl )
  {
    RNOK( m_pcBaseLayerCtrl->uninit() );
    delete m_pcBaseLayerCtrl;
    m_pcBaseLayerCtrl = 0;
  }

  if( m_pcBaseLayerCtrlField )
  {
    RNOK( m_pcBaseLayerCtrlField->uninit() );
    delete m_pcBaseLayerCtrlField;
    m_pcBaseLayerCtrlField = 0;
  }

  //========== DELETE UPDATE WEIGHTS ARRAY and WRITE BUFFER ==========
  delete [] m_pucWriteBuffer;
  m_pucWriteBuffer    = 0;
  m_uiWriteBufferSize = 0;

  //S051{
  delete[]	m_auiFrameBits;
  //S051}

  return Err::m_nOK;
}





ErrVal
LayerEncoder::uninit()
{
  m_bInitDone  = false;

  xDeleteData();

  if( m_pMotionInfoFile )
  {
    ::fclose( m_pMotionInfoFile );
  }
  if( m_pESSFile )
  {
    ::fclose( m_pESSFile );
  }

  // JVT-S054 (ADD) ->
  if( m_puiFirstMbInSlice )
  {
    free(m_puiFirstMbInSlice);
    m_puiFirstMbInSlice = 0;
  }
  // JVT-S054 (ADD) <-

  delete [] m_apabBaseModeFlagAllowedArrays[0];
  delete [] m_apabBaseModeFlagAllowedArrays[1];
  m_apabBaseModeFlagAllowedArrays[0] = 0;
  m_apabBaseModeFlagAllowedArrays[1] = 0;

  return Err::m_nOK;
}





ErrVal
LayerEncoder::xFillAndUpsampleFrame( Frame* pcFrame, PicType ePicType, Bool bFrameMbsOnlyFlag )
{
  RNOK( m_pcYuvFullPelBufferCtrl->initMb() );
  RNOK( m_pcYuvHalfPelBufferCtrl->initMb() );

  if( ! pcFrame->isHalfPel() )
  {
    XPel* pHPData = NULL;
    RNOK( pcFrame->initHalfPel( pHPData ) );
  }

  RNOK( pcFrame->extendFrame( m_pcQuarterPelFilter, ePicType, bFrameMbsOnlyFlag ) );

  return Err::m_nOK;
}





ErrVal
LayerEncoder::xFillAndExtendFrame( Frame* pcFrame, PicType ePicType, Bool bFrameMbsOnlyFlag )
{
  RNOK( m_pcYuvFullPelBufferCtrl->initMb() );

  RNOK( pcFrame->extendFrame( NULL, ePicType, bFrameMbsOnlyFlag ) );

  return Err::m_nOK;
}





ErrVal
LayerEncoder::xMotionEstimation( RefFrameList* pcRefFrameList0,
                                 RefFrameList* pcRefFrameList1,
                                 MbDataCtrl*   pcMbDataCtrlCol,
                                 Frame*        pcOrigFrame,
                                 Frame*        pcIntraRecFrame,
                                 ControlData&  rcControlData,
                                 UInt          uiNumMaxIter,
                                 UInt          uiIterSearchRange,
                                 UInt          uiFrameIdInGOP,
                                 PicType       ePicType )
{
  MbEncoder*    pcMbEncoder           =  m_pcSliceEncoder->getMbEncoder         ();
  SliceHeader&  rcSliceHeader         = *rcControlData.getSliceHeader           ( ePicType );
  MbDataCtrl*   pcMbDataCtrl          =  rcControlData.getMbDataCtrl            ();
  Frame*        pcBaseLayerFrame      =  rcControlData.getBaseLayerRec          ();
  Frame*        pcBaseLayerResidual   =  rcControlData.getBaseLayerSbb          ();
  MbDataCtrl*   pcBaseLayerCtrl       =  rcControlData.getBaseLayerCtrl         ();
  UInt          uiMaxMvPerMb          = rcSliceHeader.getSPS().getMaxMVsPer2Mb () >> 1; // hard limit (don't take into account last macroblock)

  if( ePicType!=FRAME )
  {
    if( pcOrigFrame )         RNOK( pcOrigFrame        ->addFieldBuffer( ePicType ) );
    if( pcIntraRecFrame )     RNOK( pcIntraRecFrame    ->addFieldBuffer( ePicType ) );
    if( pcBaseLayerFrame )    RNOK( pcBaseLayerFrame   ->addFieldBuffer( ePicType ) );
    if( pcBaseLayerResidual ) RNOK( pcBaseLayerResidual->addFieldBuffer( ePicType ) );
  }

  FMO* pcFMO = rcControlData.getSliceHeader( ePicType )->getFMO(); //TMM

  for (UInt iSliceGroupID=0;!pcFMO->SliceGroupCompletelyCoded(iSliceGroupID);iSliceGroupID++)
  {
    rcSliceHeader.setFirstMbInSlice(pcFMO->getFirstMacroblockInSlice(iSliceGroupID));
    rcSliceHeader.setLastMbInSlice(pcFMO->getLastMBInSliceGroup(iSliceGroupID));
    // JVT-S054 (2) (ADD)
    rcSliceHeader.setNumMbsInSlice(rcSliceHeader.getFMO()->getNumMbsInSlice(rcSliceHeader.getFirstMbInSlice(), rcSliceHeader.getLastMbInSlice()));
    UInt uiMbAddress       = rcSliceHeader.getFirstMbInSlice();
    UInt uiLastMbAddress   = rcSliceHeader.getLastMbInSlice();
    UInt uiNumMBInSlice;


    //===== initialization =====
    RefFrameList rcRefFrameList1 = rcControlData.getPrdFrameList( LIST_1 );
    if( rcSliceHeader.isMbaffFrame() )
    {
      RNOK( pcMbDataCtrl->initUsedField( rcSliceHeader, rcRefFrameList1 ) );
    }

    //===== initialization =====
    RNOK( pcMbDataCtrl->initSlice( rcSliceHeader, ENCODE_PROCESS, false, pcMbDataCtrlCol ) );
    if( ! m_bLoadMotionInfo )
    {
      RNOK( m_pcMotionEstimation->initSlice( rcSliceHeader ) );
      RNOK( pcMbEncoder         ->initSlice( rcSliceHeader ) );
      RNOK( m_pcMotionEstimation->getSW()->initSlice( rcSliceHeader ) );
    }

    //===== loop over macroblocks =====
    for(  uiNumMBInSlice = 0; uiMbAddress <= uiLastMbAddress;  )
    {
      MbDataAccess* pcMbDataAccess      = 0;
      MbDataAccess* pcMbDataAccessBase  = 0;

      UInt          uiMbY, uiMbX;
      Double        dCost              = 0;

      rcSliceHeader.getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddress     );

      //===== init macroblock =====
      RNOK  ( pcMbDataCtrl            ->initMb( pcMbDataAccess,     uiMbY, uiMbX ) );
      if    ( pcBaseLayerCtrl )
      {
        RNOK( pcBaseLayerCtrl         ->initMb( pcMbDataAccessBase, uiMbY, uiMbX ) );
        if (rcSliceHeader.getTCoeffLevelPredictionFlag())
        {
          pcMbDataAccess->setMbDataAccessBase( pcMbDataAccessBase );
        }
      }

      if( ! m_bLoadMotionInfo )
      {
        //===== initialisation =====
        RNOK( m_pcYuvFullPelBufferCtrl->initMb( uiMbY, uiMbX, false           ) );
        RNOK( m_pcYuvHalfPelBufferCtrl->initMb( uiMbY, uiMbX, false           ) );
        RNOK( m_pcMotionEstimation    ->initMb( uiMbY, uiMbX, *pcMbDataAccess ) );

        if( !rcSliceHeader.getNoInterLayerPredFlag() )
        {
          pcMbEncoder->setBaseModeAllowedFlag( m_apabBaseModeFlagAllowedArrays[0][uiMbAddress] );
        }

        RNOK( pcMbEncoder->estimatePrediction ( *pcMbDataAccess,
                                                pcMbDataAccessBase,
                                                *pcRefFrameList0,
                                                *pcRefFrameList1,
                                                pcBaseLayerFrame    ? pcBaseLayerFrame   ->getPic( ePicType ) : NULL,
                                                pcBaseLayerResidual ? pcBaseLayerResidual->getPic( ePicType ) : NULL,
                                                *pcOrigFrame                              ->getPic( ePicType ),
                                                *pcIntraRecFrame                          ->getPic( ePicType ),
                                                uiMaxMvPerMb,
                                                m_bMCBlks8x8Disable,
                                                m_bBiPred8x8Disable,
                                                uiNumMaxIter,
                                                uiIterSearchRange,
                                                rcControlData.getLambda(),
                                                dCost,
                                                true ) );
// TMM_INTERLACE{
       /* if( m_bSaveMotionInfo )
        {
          //===== save prediction data =====
         // saveAll is displaced because the Mvs are Ok but the other data could be modified (mode,...).
         // Do it after m_pcSliceEncoder->encodeHighPassPicture
         //            RNOK( pcMbDataAccess->getMbData().saveAll( m_pMotionInfoFile ) );
        }*/
// TMM_INTERLACE}
      }
      else
      {
        //===== load prediction data =====
        RNOK( pcMbDataAccess->getMbData().loadAll( m_pMotionInfoFile ) );
      }
      uiMbAddress = rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress);
      uiNumMBInSlice++;
    }
  }

  if( ePicType!=FRAME )
  {
    if( pcOrigFrame )         RNOK( pcOrigFrame        ->removeFieldBuffer( ePicType ) );
    if( pcIntraRecFrame )     RNOK( pcIntraRecFrame    ->removeFieldBuffer( ePicType ) );
    if( pcBaseLayerFrame )    RNOK( pcBaseLayerFrame   ->removeFieldBuffer( ePicType ) );
    if( pcBaseLayerResidual ) RNOK( pcBaseLayerResidual->removeFieldBuffer( ePicType ) );
  }

  return Err::m_nOK;
}




ErrVal
LayerEncoder::xMotionCompensation( Frame*        pcMCFrame,
                                  RefFrameList*    pcRefFrameList0,
                                  RefFrameList*    pcRefFrameList1,
                                  MbDataCtrl*      pcMbDataCtrl,
                                  SliceHeader&     rcSH )
{
  Bool        bCalcMv         = false;
  Bool        bFaultTolerant  = false;
  MbEncoder*  pcMbEncoder     = m_pcSliceEncoder->getMbEncoder();

  RNOK( pcMbDataCtrl        ->initSlice( rcSH, PRE_PROCESS, false, NULL ) );
  RNOK( m_pcMotionEstimation->initSlice( rcSH              ) );

  MbDataCtrl*      pcBaseMbDataCtrl = getBaseMbDataCtrl();

  RefFrameList* apcRefFrameList0[4] = { NULL, NULL, NULL, NULL };
  RefFrameList* apcRefFrameList1[4] = { NULL, NULL, NULL, NULL };
  const PicType ePicType = rcSH.getPicType();
  const Bool    bMbAff   = rcSH.isMbaffFrame   ();
  Frame* apcBLFrame[3] = { 0, 0, m_pcBaseLayerFrame };

  if( bMbAff )
  {
    RefFrameList acRefFrameList0[2];
    RefFrameList acRefFrameList1[2];

    RNOK( gSetFrameFieldLists( acRefFrameList0[0], acRefFrameList0[1], *pcRefFrameList0 ) );
    RNOK( gSetFrameFieldLists( acRefFrameList1[0], acRefFrameList1[1], *pcRefFrameList1 ) );

    apcRefFrameList0[ TOP_FIELD ] = ( NULL == pcRefFrameList0 ) ? NULL : &acRefFrameList0[0];
    apcRefFrameList0[ BOT_FIELD ] = ( NULL == pcRefFrameList0 ) ? NULL : &acRefFrameList0[1];
    apcRefFrameList1[ TOP_FIELD ] = ( NULL == pcRefFrameList1 ) ? NULL : &acRefFrameList1[0];
    apcRefFrameList1[ BOT_FIELD ] = ( NULL == pcRefFrameList1 ) ? NULL : &acRefFrameList1[1];
    apcRefFrameList0[     FRAME ] = pcRefFrameList0;
    apcRefFrameList1[     FRAME ] = pcRefFrameList1;

    if( m_pcBaseLayerFrame )
    {
      m_pcBaseLayerFrame->addFrameFieldBuffer();
      apcBLFrame[0] = m_pcBaseLayerFrame->getPic( TOP_FIELD );
      apcBLFrame[1] = m_pcBaseLayerFrame->getPic( BOT_FIELD );
    }
  }
  else
  {
    RNOK( pcMCFrame->addFieldBuffer( ePicType ) );
    apcRefFrameList0[ ePicType ] = pcRefFrameList0;
    apcRefFrameList1[ ePicType ] = pcRefFrameList1;
  }

  //===== loop over macroblocks =====
  const UInt uiMbNumber = rcSH.getMbInPic(); //TMM
  for( UInt uiMbAddress = 0 ; uiMbAddress < uiMbNumber; uiMbAddress++ ) //TMM
  {
    MbDataAccess* pcMbDataAccess = NULL;
    MbDataAccess* pcMbDataAccessBase  = 0;
    UInt          uiMbY, uiMbX;

    rcSH.getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddress                                );

    RNOK( pcMbDataCtrl            ->initMb( pcMbDataAccess, uiMbY, uiMbX                  ) );
    if    ( pcBaseMbDataCtrl )
    {
      RNOK( pcBaseMbDataCtrl         ->initMb( pcMbDataAccessBase, uiMbY, uiMbX ) );
    }
    RNOK( m_pcYuvFullPelBufferCtrl->initMb(                 uiMbY, uiMbX, bMbAff          ) );
    RNOK( m_pcYuvHalfPelBufferCtrl->initMb(                 uiMbY, uiMbX, bMbAff          ) );
    RNOK( m_pcMotionEstimation    ->initMb(                 uiMbY, uiMbX, *pcMbDataAccess ) );

    const PicType eMbPicType = pcMbDataAccess->getMbPicType();
    pcMbDataAccess->setMbDataAccessBase(pcMbDataAccessBase);
    pcMbEncoder->setBaseLayerRec( apcBLFrame[ eMbPicType - 1 ] );

    RNOK( pcMbEncoder->compensatePrediction( *pcMbDataAccess,
                                              pcMCFrame->getPic( eMbPicType ),
                                             *apcRefFrameList0 [ eMbPicType ],
                                             *apcRefFrameList1 [ eMbPicType ],
                                             bCalcMv, bFaultTolerant) );

  }

  return Err::m_nOK;
}


ErrVal
LayerEncoder::xZeroIntraMacroblocks( Frame*    pcFrame,
                                    ControlData& rcCtrlData,
                                    PicType      ePicType )
{
  MbDataCtrl*       pcMbDataCtrl  = rcCtrlData.getMbDataCtrl       ();
  SliceHeader*      pcSliceHeader = rcCtrlData.getSliceHeader      (ePicType);


  RNOK( pcMbDataCtrl->initSlice( *pcSliceHeader, PRE_PROCESS, false, NULL ) );

  YuvMbBuffer cZeroMbBuffer;
  cZeroMbBuffer.setAllSamplesToZero();

  const Bool bMbAff = pcSliceHeader->isMbaffFrame();
  if( ePicType!=FRAME )
  {
    RNOK( pcFrame->addFieldBuffer     ( ePicType ) );
  }
  else if( bMbAff )
  {
    RNOK( pcFrame->addFrameFieldBuffer() );
  }

  //===== loop over macroblocks =====
  const UInt uiMbNumber = pcSliceHeader->getMbInPic(); //TMM
  for( UInt uiMbAddress = 0 ; uiMbAddress < uiMbNumber; uiMbAddress++ ) //TMM
  {
    MbDataAccess* pcMbDataAccess = NULL;
    UInt          uiMbY, uiMbX;

    pcSliceHeader->getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddress              );

    RNOK( pcMbDataCtrl            ->initMb( pcMbDataAccess, uiMbY, uiMbX ) );
    RNOK( m_pcYuvFullPelBufferCtrl->initMb (                 uiMbY, uiMbX, bMbAff ) );

    if( pcMbDataAccess->getMbData().isIntra() )
    {
      const PicType eMbPicType = pcMbDataAccess->getMbPicType();
      pcFrame->getPic( eMbPicType )->getFullPelYuvBuffer()->loadBuffer( &cZeroMbBuffer );
    }
  }

  if( ePicType!=FRAME )
  {
    RNOK( pcFrame->removeFieldBuffer     ( ePicType ) );
  }
  else if( bMbAff )
  {
    RNOK( pcFrame->removeFrameFieldBuffer()           );
  }
  return Err::m_nOK;
}



ErrVal
LayerEncoder::xClipIntraMacroblocks( Frame*    pcFrame,
                                    ControlData& rcCtrlData,
                                    Bool         bClipAll,
                                    PicType      ePicType )
{
  MbDataCtrl*       pcMbDataCtrl  = rcCtrlData.getMbDataCtrl       ();
  SliceHeader* pcSliceHeader = rcCtrlData.getSliceHeader( ePicType );
  ROF( pcSliceHeader );
  YuvPicBuffer* pcPicBuffer;
  YuvMbBuffer    cMbBuffer;

  RNOK( pcMbDataCtrl->initSlice( *pcSliceHeader, PRE_PROCESS, false, NULL ) );

  const Bool bMbAff = pcSliceHeader->isMbaffFrame();
  if( ePicType!=FRAME )
  {
    RNOK( pcFrame->addFieldBuffer     ( ePicType ) );
  }
  else if( bMbAff )
  {
    RNOK( pcFrame->addFrameFieldBuffer() );
  }

  //===== loop over macroblocks =====
  const UInt uiMbNumber = pcSliceHeader->getMbInPic();//TMM
  for( UInt uiMbAddress = 0 ; uiMbAddress < uiMbNumber; uiMbAddress++ )//TMM
  {
    MbDataAccess* pcMbDataAccess = 0;
    UInt          uiMbY, uiMbX;

    pcSliceHeader->getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddress              );

    RNOK( pcMbDataCtrl            ->initMb( pcMbDataAccess, uiMbY, uiMbX ) );
    RNOK( m_pcYuvFullPelBufferCtrl->initMb (                 uiMbY, uiMbX, bMbAff ) );

    if( bClipAll || pcMbDataAccess->getMbData().isIntra() )
    {
      const PicType eMbPicType = pcMbDataAccess->getMbPicType();
      pcPicBuffer = pcFrame->getPic( eMbPicType )->getFullPelYuvBuffer();
      cMbBuffer   .loadBuffer( pcPicBuffer );
      cMbBuffer   .clip      ();
      pcPicBuffer->loadBuffer( &cMbBuffer );
    }
  }
  if( ePicType!=FRAME )
  {
    RNOK( pcFrame->removeFieldBuffer     ( ePicType ) );
  }
  else if( bMbAff )
  {
    RNOK( pcFrame->removeFrameFieldBuffer()           );
  }
  return Err::m_nOK;
}


ErrVal
LayerEncoder::xInitExtBinDataAccessor( ExtBinDataAccessor& rcExtBinDataAccessor )
{
  ROF( m_pucWriteBuffer );
  m_cBinData.reset          ();
  m_cBinData.set            ( m_pucWriteBuffer, m_uiWriteBufferSize );
  m_cBinData.setMemAccessor ( rcExtBinDataAccessor );

  return Err::m_nOK;
}


ErrVal
LayerEncoder::xAppendNewExtBinDataAccessor( ExtBinDataAccessorList& rcExtBinDataAccessorList,
                                           ExtBinDataAccessor*     pcExtBinDataAccessor )
{
  ROF( pcExtBinDataAccessor );
  ROF( pcExtBinDataAccessor->data() );

  UInt    uiNewSize     = pcExtBinDataAccessor->size();
  UChar*  pucNewBuffer  = new UChar [ uiNewSize ];
  ROF( pucNewBuffer );
  ::memcpy( pucNewBuffer, pcExtBinDataAccessor->data(), uiNewSize * sizeof( UChar ) );

  ExtBinDataAccessor* pcNewExtBinDataAccessor = new ExtBinDataAccessor;
  ROF( pcNewExtBinDataAccessor );

  m_cBinData              .reset          ();
  m_cBinData              .set            (  pucNewBuffer, uiNewSize );
  m_cBinData              .setMemAccessor ( *pcNewExtBinDataAccessor );
  rcExtBinDataAccessorList.push_back      (  pcNewExtBinDataAccessor );

  m_cBinData              .reset          ();
  m_cBinData              .setMemAccessor ( *pcExtBinDataAccessor );

  return Err::m_nOK;
}



ErrVal
LayerEncoder::xEncodeLowPassSignal( ExtBinDataAccessorList& rcOutExtBinDataAccessorList,
                                    ControlData&            rcControlData,
                                    Frame*                  pcOrgFrame,
                                    Frame*                  pcFrame,
                                    Frame*                  pcResidual,
                                    Frame*                  pcPredSignal,
                                    UInt&                   ruiBits,
                                    PicOutputDataList&      rcPicOutputDataList,
                                    UInt                    uiFrameIdInGOP,
                                    PicType                 ePicType )
{
  UInt         uiBits              = 0;
  UInt         uiBitsSEI           = 0;
  Frame*       pcBaseLayerFrame    = rcControlData.getBaseLayerRec ();
  SliceHeader* pcSliceHeader       = rcControlData.getSliceHeader ( ePicType );
  ROF( pcSliceHeader );

  ExtBinDataAccessorList  cTmpExtBinDataAccessorList;
  ExtBinDataAccessorList  acExtBinDataAccessorList[16];
  PicOutputDataList       acPicOutputDataList     [16];

  //----- subsequence SEI -----
  if( m_bWriteSubSequenceSei && m_bH264AVCCompatible )
  {
    RNOK( xWriteSEI( rcOutExtBinDataAccessorList, *pcSliceHeader, uiBitsSEI ) );
  }

  FMO* pcFMO = pcSliceHeader->getFMO();
  for( UInt iSliceGroupID = 0; !pcFMO->SliceGroupCompletelyCoded( iSliceGroupID ); iSliceGroupID++ )
  {
		//JVT-X046 {
		rcControlData.m_bSliceGroupAllCoded = false;
		rcControlData.m_uiCurrentFirstMB    = pcFMO->getFirstMacroblockInSlice( iSliceGroupID );
		while( !rcControlData.m_bSliceGroupAllCoded )
		{
		  if (!m_uiSliceMode)
      {
        rcControlData.m_bSliceGroupAllCoded = true;
      }
		  //JVT-X046 }
      pcSliceHeader->setFirstMbInSlice( rcControlData.m_uiCurrentFirstMB );
      pcSliceHeader->setLastMbInSlice ( pcFMO->getLastMBInSliceGroup( iSliceGroupID ) );
      // JVT-S054 (2) (ADD)
      pcSliceHeader->setNumMbsInSlice ( pcFMO->getNumMbsInSlice( pcSliceHeader->getFirstMbInSlice(), pcSliceHeader->getLastMbInSlice() ) );

      //prefix unit{{
      if( m_uiPreAndSuffixUnitEnable )
      {
        // TMM {
        if ( rcControlData.getSliceHeader( ePicType )->getNalUnitType() == NAL_UNIT_CODED_SLICE|| rcControlData.getSliceHeader( ePicType )->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR )
        {
          RNOK( xWritePrefixUnit( acExtBinDataAccessorList[0], *rcControlData.getSliceHeader( ePicType ), uiBits ) );
        }
        // TMM }
      }
      //prefix unit}}

      //----- init NAL UNIT -----
      RNOK( xInitExtBinDataAccessor        (  m_cExtBinDataAccessor ) );
      RNOK( m_pcNalUnitEncoder->initNalUnit( &m_cExtBinDataAccessor ) );

      //---- write Slice Header -----
      ETRACE_NEWSLICE;
      xAssignSimplePriorityId( pcSliceHeader );
      if( pcSliceHeader->getSliceSkipFlag() )
      {
        UInt uiMaxMbsInSlice  = ( m_uiSliceMode == 1 ? m_uiSliceArgument : MSYS_UINT_MAX );
        UInt uiNumSkipSlices  = min( uiMaxMbsInSlice, pcSliceHeader->getNumMbsInSlice() );
        ROF( uiNumSkipSlices  > 0 );
        pcSliceHeader->setNumMbsInSliceMinus1( uiNumSkipSlices - 1 );
      }

		  //JVT-X046 {
		  m_pcSliceEncoder->m_pcMbCoder->m_uiSliceMode     = m_uiSliceMode;
		  m_pcSliceEncoder->m_pcMbCoder->m_uiSliceArgument = m_uiSliceArgument;
		  if( m_pcSliceEncoder->m_pcMbCoder->m_uiSliceMode == 2 )
		  {
			  //subtract slice header size and 4*8 bytes from Slice Argument
			  m_pcSliceEncoder->m_pcMbCoder->m_uiSliceArgument -= m_pcNalUnitEncoder->xGetBitsWriteBuffer()->getBitsWritten() + 4*8 +20;
		  }
		  //JVT-X046 }

      RNOK( m_pcNalUnitEncoder->write( *pcSliceHeader ) );

      rcControlData.getPrdFrameList( LIST_0 ).reset();
      rcControlData.getPrdFrameList( LIST_1 ).reset();

      //----- encode slice data -----
      if( pcSliceHeader->isIntraSlice() )
      {
        if( pcSliceHeader->isMbaffFrame() )
        {
          RNOK( m_pcSliceEncoder->encodeIntraPictureMbAff( uiBits,
                                                           rcControlData,
                                                           pcOrgFrame,
                                                           pcFrame,
                                                           pcResidual,
                                                           pcBaseLayerFrame,
                                                           pcPredSignal,
                                                           m_uiFrameWidthInMb,
                                                           rcControlData.getLambda() ) );
        }
        else
        {
          RNOK( m_pcSliceEncoder->encodeIntraPicture( uiBits,
                                                      rcControlData,
                                                      pcFrame,
                                                      pcResidual,
                                                      pcBaseLayerFrame,
                                                      pcPredSignal,
                                                      m_uiFrameWidthInMb,
                                                      rcControlData.getLambda(),
                                                      ePicType ) );
        }
      }
      else
      {
        //----- initialize reference lists -----
        ROF ( pcSliceHeader->getNumRefIdxActive ( LIST_1 ) == 0 );

        //===== get reference frame lists =====
        RefFrameList& rcRefFrameList0 = rcControlData.getPrdFrameList( LIST_0 );
        RefFrameList& rcRefFrameList1 = rcControlData.getPrdFrameList( LIST_1 );
        UInt          uiFrameIdCol    = MSYS_UINT_MAX;

        RNOK( xGetPredictionLists( rcRefFrameList0, rcRefFrameList1, uiFrameIdCol, uiFrameIdInGOP, ePicType, RLU_REF_BASE_PIC, true ) );
        RefFrameList acRefFieldList0[2];
        RefFrameList acRefFieldList1[2];
        pcSliceHeader->setRefFrameList( &rcRefFrameList0, ePicType, LIST_0 );
        pcSliceHeader->setRefFrameList( &rcRefFrameList1, ePicType, LIST_1 );
        if( pcSliceHeader->isMbaffFrame() )
        {
          RNOK( gSetFrameFieldLists( acRefFieldList0[0], acRefFieldList0[1], rcRefFrameList0 ) );
          RNOK( gSetFrameFieldLists( acRefFieldList1[0], acRefFieldList1[1], rcRefFrameList1 ) );
          pcSliceHeader->setRefFrameList( &(acRefFieldList0[0]), TOP_FIELD, LIST_0 );
          pcSliceHeader->setRefFrameList( &(acRefFieldList0[1]), BOT_FIELD, LIST_0 );
          pcSliceHeader->setRefFrameList( &(acRefFieldList1[0]), TOP_FIELD, LIST_1 );
          pcSliceHeader->setRefFrameList( &(acRefFieldList1[1]), BOT_FIELD, LIST_1 );
        }

        if( pcSliceHeader->isMbaffFrame() )
        {
          RNOK( m_pcSliceEncoder->encodeInterPicturePMbAff( uiBits,
                                                            pcOrgFrame,
                                                            pcFrame,
                                                            pcResidual,
                                                            pcPredSignal,
                                                            rcControlData,
                                                            m_bMCBlks8x8Disable,
                                                            m_uiFrameWidthInMb,
                                                            rcRefFrameList0 ) );
        }
        else
        {
          RNOK( m_pcSliceEncoder->encodeInterPictureP ( uiBits,
                                                        pcFrame,
                                                        pcResidual,
                                                        pcPredSignal,
                                                        rcControlData,
                                                        m_bMCBlks8x8Disable,
                                                        m_uiFrameWidthInMb,
                                                        rcRefFrameList0,
                                                        ePicType ) );

        }
        RNOK( m_apcBaseFrame[0]->uninitHalfPel() );
        RNOK( m_apcBaseFrame[1]->uninitHalfPel() );
      }

      //----- close NAL UNIT -----
      UInt auiBits[16];
      RNOK( m_pcNalUnitEncoder->closeAndAppendNalUnits( auiBits,
                                                        cTmpExtBinDataAccessorList,
                                                        &m_cExtBinDataAccessor,
                                                        m_cBinData,
                                                        m_pcH264AVCEncoder,
                                                        m_uiQualityLevelCGSSNR,
                                                        m_uiLayerCGSSNR ) );
      for( Int iMGSIdx = 0; cTmpExtBinDataAccessorList.size(); iMGSIdx++ )
      {
        acExtBinDataAccessorList[iMGSIdx].push_back( cTmpExtBinDataAccessorList.popFront() );
        auiBits[iMGSIdx] += 4*8; // start code
      }
      uiBits = auiBits[0];

      //JVT-W052
      if(m_pcH264AVCEncoder->getCodingParameter()->getIntegrityCheckSEIEnable()&&m_pcH264AVCEncoder->getCodingParameter()->getCGSSNRRefinement() )
      {
        if(m_pcH264AVCEncoder->getCodingParameter()->getNumberOfLayers()-1 == pcSliceHeader->getDependencyId() )
        {
          xWriteIntegrityCheckSEI( rcOutExtBinDataAccessorList, m_pcH264AVCEncoder->m_pcIntegrityCheckSEI, uiBits );
        }
      }
      //JVT-W052

      PicOutputData cPicOutputData;
      cPicOutputData.FirstPicInAU  = pcSliceHeader->getNoInterLayerPredFlag() && pcSliceHeader->getFirstMbInSlice() == 0;
      cPicOutputData.iPicType      = pcSliceHeader->getPicType();
      cPicOutputData.Poc           = pcSliceHeader->getPoc                   ();
      cPicOutputData.FrameType[0]  = pcSliceHeader->getSliceType             () == B_SLICE ? 'B' :
                                     pcSliceHeader->getSliceType             () == P_SLICE ? 'P' : 'I';
      cPicOutputData.FrameType[1]  = pcSliceHeader->getUseRefBasePicFlag     ()            ? 'K' : ' ';
      cPicOutputData.FrameType[2]  = '\0';
      cPicOutputData.DependencyId  = pcSliceHeader->getLayerCGSSNR           ();
      cPicOutputData.QualityId     = pcSliceHeader->getQualityLevelCGSSNR    ();
      cPicOutputData.TemporalId    = pcSliceHeader->getTemporalId            ();
      cPicOutputData.Qp            = pcSliceHeader->getSliceQp               ();
      cPicOutputData.Bits          = uiBits + uiBitsSEI;
      cPicOutputData.YPSNR         = 0.0;
      cPicOutputData.UPSNR         = 0.0;
      cPicOutputData.VPSNR         = 0.0;

      // JVT-V068 {
      cPicOutputData.iPicType      = (Int)ePicType;
      if (  m_uiQualityLevelCGSSNR != 0 || m_uiLayerCGSSNR != rcControlData.getSliceHeader()->getDependencyId()  )
      {
        cPicOutputData.uiBaseLayerId = m_uiBaseLayerCGSSNR;
        cPicOutputData.uiBaseQualityLevel = m_uiBaseQualityLevel;
      }
      else
      {
        cPicOutputData.uiBaseLayerId = m_uiBaseLayerId;
        cPicOutputData.uiBaseQualityLevel = m_uiBaseQualityLevel;
      }
      // JVT-V068 }

      acPicOutputDataList[0].push_back( cPicOutputData );

      for( UInt uiFrag = 0; true; )
      {
        if( pcSliceHeader->getSPS().getMGSCoeffStop( uiFrag ) == 16 )
        {
          break;
        }
        uiFrag++;
        cPicOutputData.QualityId++;
        cPicOutputData.uiBaseLayerId      = pcSliceHeader->getLayerCGSSNR();
        cPicOutputData.uiBaseQualityLevel = cPicOutputData.QualityId - 1;
        cPicOutputData.Bits               = auiBits[uiFrag];
        acPicOutputDataList[uiFrag].push_back( cPicOutputData );
        if( m_pcH264AVCEncoder->getCodingParameter()->getCGSSNRRefinement() )
        {
          m_pcH264AVCEncoder->m_aaadSeqBits[getLayerCGSSNR()][pcSliceHeader->getTemporalId()][getQualityLevelCGSSNR()+uiFrag] += auiBits[uiFrag];
        }
        else
        {
          m_pcH264AVCEncoder->m_aaadSeqBits[m_uiDependencyId][pcSliceHeader->getTemporalId()][0] += auiBits[uiFrag];
        }
        ruiBits += auiBits[uiFrag];
      }

      if( ePicType != ( m_bBotFieldFirst ? BOT_FIELD : TOP_FIELD ) )
      {
        ETRACE_NEWFRAME;
      }
      if( m_pcH264AVCEncoder->getCodingParameter()->getCGSSNRRefinement() )
      {
        m_pcH264AVCEncoder->m_aaadSeqBits[getLayerCGSSNR()][pcSliceHeader->getTemporalId()][getQualityLevelCGSSNR()] += uiBits + uiBitsSEI;
      }
      else
      {
        m_pcH264AVCEncoder->m_aaadSeqBits[m_uiDependencyId][pcSliceHeader->getTemporalId()][0] += uiBits + uiBitsSEI;
      }
      ruiBits = ruiBits + uiBits + uiBitsSEI;
      uiBitsSEI=0;
    }

	}//JVT-X046

  for( UInt uiMGSIdx = 0; uiMGSIdx < 16; uiMGSIdx++ )
  {
    rcOutExtBinDataAccessorList += acExtBinDataAccessorList [uiMGSIdx];
    rcPicOutputDataList         += acPicOutputDataList      [uiMGSIdx];
    acExtBinDataAccessorList[uiMGSIdx].clear();
    acPicOutputDataList     [uiMGSIdx].clear();
  }

  if( ! pcSliceHeader->getNoInterLayerPredFlag() && pcSliceHeader->getSCoeffResidualPredFlag() )
  {
    m_pcSliceEncoder->updatePictureResTransform( rcControlData,m_uiFrameWidthInMb);
  }

  if(pcSliceHeader->getTCoeffLevelPredictionFlag())
  {
    //===== update state prior to deblocking
    m_pcSliceEncoder->updatePictureAVCRewrite( rcControlData, m_uiFrameWidthInMb );
  }

  return Err::m_nOK;
}







ErrVal
LayerEncoder::xEncodeHighPassSignal( ExtBinDataAccessorList&  rcOutExtBinDataAccessorList,
                                    ControlData&             rcControlData,
                                    Frame*                pcOrgFrame,
                                    Frame*                pcFrame,
                                    Frame*                pcResidual,
                                    Frame*                pcPredSignal,
                                    UInt&                    ruiBits,
                                    UInt&                    ruiBitsRes,
                                    PicOutputDataList&       rcPicOutputDataList,
                                    PicType                  ePicType )
{
  UInt  uiBitsSEI   = 0;
  UInt  uiBits      = 0;//prefix unit
  SliceHeader* pcSliceHeader = rcControlData.getSliceHeader( ePicType );
  ROF( pcSliceHeader );

  ExtBinDataAccessorList  cTmpExtBinDataAccessorList;
  ExtBinDataAccessorList  acExtBinDataAccessorList[16];
  PicOutputDataList       acPicOutputDataList     [16];

  //----- Subsequence SEI -----
  if( m_bWriteSubSequenceSei &&   m_bH264AVCCompatible )
  {
    RNOK( xWriteSEI( rcOutExtBinDataAccessorList, *pcSliceHeader, uiBitsSEI ) );
  }

  FMO* pcFMO = rcControlData.getSliceHeader( ePicType )->getFMO();

  for (UInt iSliceGroupID=0;!pcFMO->SliceGroupCompletelyCoded(iSliceGroupID);iSliceGroupID++)
  {
    uiBits      = 0;
    UInt  uiBitsRes   = 0;
    UInt  uiMbCoded   = 0;

    pcSliceHeader->setFirstMbInSlice(pcFMO->getFirstMacroblockInSlice(iSliceGroupID));
    pcSliceHeader->setLastMbInSlice(pcFMO->getLastMBInSliceGroup(iSliceGroupID));
    pcSliceHeader->setNumMbsInSlice (pcFMO->getNumMbsInSlice(rcControlData.getSliceHeader( ePicType )->getFirstMbInSlice(), rcControlData.getSliceHeader( ePicType )->getLastMbInSlice()));

    //prefix unit{{
    if( m_uiPreAndSuffixUnitEnable )
    {
      if ( pcSliceHeader->getNalUnitType() == NAL_UNIT_CODED_SLICE|| pcSliceHeader->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR )
      {
        RNOK( xWritePrefixUnit( acExtBinDataAccessorList[0], *pcSliceHeader, uiBits ) );
      }
    }
    //prefix unit}}

    //----- init NAL UNIT -----
    RNOK( xInitExtBinDataAccessor        (  m_cExtBinDataAccessor ) );
    RNOK( m_pcNalUnitEncoder->initNalUnit( &m_cExtBinDataAccessor ) );

    //---- write Slice Header -----
    ETRACE_NEWSLICE;
    xAssignSimplePriorityId( pcSliceHeader );
    if( pcSliceHeader->getSliceSkipFlag() )
    {
      UInt uiNumSkipSlices  = pcSliceHeader->getNumMbsInSlice();
      ROF( uiNumSkipSlices  > 0 );
      pcSliceHeader->setNumMbsInSliceMinus1( uiNumSkipSlices - 1 );
    }

    RNOK( m_pcNalUnitEncoder->write( *pcSliceHeader ) );

    //----- write slice data -----
    if( pcSliceHeader->isMbaffFrame() )
    {
      RNOK( m_pcSliceEncoder->encodeHighPassPictureMbAff( uiMbCoded,
                                                          uiBits,
                                                          *rcControlData.getSliceHeader            (),
                                                          pcOrgFrame,
                                                          pcFrame,
                                                          pcResidual,
                                                          pcPredSignal,
                                                          rcControlData.getBaseLayerSbb           (),
                                                          rcControlData.getBaseLayerRec           (),
                                                          rcControlData.getMbDataCtrl             (),
                                                          rcControlData.getBaseLayerCtrl          (),
                                                          rcControlData.getBaseLayerCtrlField     (),
                                                          m_uiFrameWidthInMb,
                                                          rcControlData.getLambda                 (),
                                                          m_iMaxDeltaQp ) );
    }
    else
    {
      RNOK( m_pcSliceEncoder->encodeHighPassPicture ( uiMbCoded,
                                                      uiBits,
                                                     *pcSliceHeader,
                                                      pcOrgFrame,
                                                      pcFrame,
                                                      pcResidual,
                                                      pcPredSignal,
                                                      rcControlData.getBaseLayerSbb(),
                                                      rcControlData.getBaseLayerRec(),
                                                      rcControlData.getMbDataCtrl(),
                                                      rcControlData.getBaseLayerCtrl(),
                                                      m_uiFrameWidthInMb,
                                                      rcControlData.getLambda(),
                                                      m_iMaxDeltaQp,
                                                      ePicType ) );
    }

    // TMM_INTERLACE {
    // saveAll is deplaced here. All data are correct.
    // save prediction data for B pictures
    if( m_bSaveMotionInfo )
    {
      MbDataCtrl*   pcMbDataCtrl          =  rcControlData.getMbDataCtrl            ();
      //===== loop over macroblocks =====
      UInt uiMbAddress       = pcSliceHeader->getFirstMbInSlice();
      UInt uiLastMbAddress   = pcSliceHeader->getLastMbInSlice();
      UInt uiNumMBInSlice;
      for(  uiNumMBInSlice = 0; uiMbAddress <= uiLastMbAddress;  )
      {
        MbDataAccess* pcMbDataAccess     = NULL;
        UInt          uiMbY, uiMbX;

        pcSliceHeader->getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddress            );

        //===== init macroblock =====
        RNOK  ( pcMbDataCtrl            ->initMb( pcMbDataAccess,     uiMbY, uiMbX ) );

        //===== save prediction data =====
        RNOK( pcMbDataAccess->getMbData().saveAll( m_pMotionInfoFile ) );
        uiMbAddress = pcSliceHeader->getFMO()->getNextMBNr(uiMbAddress);
        uiNumMBInSlice++;
      }
    }
    // TMM_INTERLACE }

    //----- close NAL UNIT -----
    UInt auiBits[16];
    RNOK( m_pcNalUnitEncoder->closeAndAppendNalUnits( auiBits, cTmpExtBinDataAccessorList, &m_cExtBinDataAccessor, m_cBinData, NULL, 0, 0 ) );
    for( Int iMGSIdx = 0; cTmpExtBinDataAccessorList.size(); iMGSIdx++ )
    {
      acExtBinDataAccessorList[iMGSIdx].push_back( cTmpExtBinDataAccessorList.popFront() );
      auiBits[iMGSIdx] += 4*8; // start code
    }
    uiBits = auiBits[0];

    PicOutputData cPicOutputData;
    cPicOutputData.FirstPicInAU  = rcControlData.getSliceHeader( ePicType )->getNoInterLayerPredFlag() && rcControlData.getSliceHeader( ePicType )->getFirstMbInSlice() == 0;
    cPicOutputData.iPicType      = ePicType;
    cPicOutputData.Poc           = rcControlData.getSliceHeader( ePicType )->getPoc                  ();
    cPicOutputData.FrameType[0]  = rcControlData.getSliceHeader( ePicType )->getSliceType            () == B_SLICE ? 'B' :
    rcControlData.getSliceHeader( ePicType )->getSliceType                                           () == P_SLICE ? 'P' : 'I';
    cPicOutputData.FrameType[1]  = rcControlData.getSliceHeader( ePicType )->getUseRefBasePicFlag()            ? 'K' : ' ';
    cPicOutputData.FrameType[2]  = '\0';
    cPicOutputData.DependencyId  = rcControlData.getSliceHeader( ePicType )->getLayerCGSSNR          ();
    cPicOutputData.QualityId     = rcControlData.getSliceHeader( ePicType )->getQualityLevelCGSSNR   ();
    cPicOutputData.TemporalId    = rcControlData.getSliceHeader( ePicType )->getTemporalId        ();
    cPicOutputData.Qp            = rcControlData.getSliceHeader( ePicType )->getSliceQp                ();
    cPicOutputData.Bits          = uiBits + uiBitsSEI;
    cPicOutputData.YPSNR         = 0.0;
    cPicOutputData.UPSNR         = 0.0;
    cPicOutputData.VPSNR         = 0.0;
    // JVT-V068 {
    cPicOutputData.iPicType      = (Int)ePicType;
    if (  m_uiQualityLevelCGSSNR != 0 || m_uiLayerCGSSNR != rcControlData.getSliceHeader()->getDependencyId()  )
    {
      cPicOutputData.uiBaseLayerId = m_uiBaseLayerCGSSNR;
      cPicOutputData.uiBaseQualityLevel = m_uiBaseQualityLevel;
    }
    else
    {
      cPicOutputData.uiBaseLayerId = m_uiBaseLayerId;
      cPicOutputData.uiBaseQualityLevel = m_uiBaseQualityLevel;
    }
    // JVT-V068 }
    acPicOutputDataList[0].push_back( cPicOutputData );

    for( UInt uiFrag = 0; true; )
    {
      if( pcSliceHeader->getSPS().getMGSCoeffStop( uiFrag ) == 16 )
      {
        break;
      }
      uiFrag++;
      cPicOutputData.QualityId++;
      cPicOutputData.uiBaseLayerId      = pcSliceHeader->getLayerCGSSNR();
      cPicOutputData.uiBaseQualityLevel = cPicOutputData.QualityId - 1;
      cPicOutputData.Bits               = auiBits[uiFrag];
      acPicOutputDataList[uiFrag].push_back( cPicOutputData );
      if( m_pcH264AVCEncoder->getCodingParameter()->getCGSSNRRefinement() )
      {
        m_pcH264AVCEncoder->m_aaadSeqBits[getLayerCGSSNR()][pcSliceHeader->getTemporalId()][getQualityLevelCGSSNR()+uiFrag] += auiBits[uiFrag];
      }
      else
      {
        m_pcH264AVCEncoder->m_aaadSeqBits[m_uiDependencyId][pcSliceHeader->getTemporalId()][0] += auiBits[uiFrag];
      }
      ruiBits += auiBits[uiFrag];
    }

    if( m_pcH264AVCEncoder->getCodingParameter()->getCGSSNRRefinement() )
    {
      m_pcH264AVCEncoder->m_aaadSeqBits[getLayerCGSSNR()][pcSliceHeader->getTemporalId()][getQualityLevelCGSSNR()] += uiBits+uiBitsSEI;
    }
    else
    {
      m_pcH264AVCEncoder->m_aaadSeqBits[m_uiDependencyId][pcSliceHeader->getTemporalId()][0] += uiBits+uiBitsSEI;
    }

    ruiBits     += uiBits+uiBitsSEI;
    ruiBitsRes  += uiBitsRes;
    uiBitsSEI =0;
  }

  for( UInt uiMGSIdx = 0; uiMGSIdx < 16; uiMGSIdx++ )
  {
    rcOutExtBinDataAccessorList += acExtBinDataAccessorList [uiMGSIdx];
    rcPicOutputDataList         += acPicOutputDataList      [uiMGSIdx];
    acExtBinDataAccessorList[uiMGSIdx].clear();
    acPicOutputDataList     [uiMGSIdx].clear();
  }

  if( !pcSliceHeader->getNoInterLayerPredFlag() && pcSliceHeader->getSCoeffResidualPredFlag() )
  {
    m_pcSliceEncoder->updatePictureResTransform( rcControlData,m_uiFrameWidthInMb);
  }

  if( pcSliceHeader->getTCoeffLevelPredictionFlag() )
  {
    //===== update state prior to deblocking
    m_pcSliceEncoder->updatePictureAVCRewrite( rcControlData, m_uiFrameWidthInMb );
  }

  ETRACE_NEWFRAME;

  return Err::m_nOK;
}


ErrVal
LayerEncoder::xInitRefFrameLists( UInt uiFrameIdInGOP, UInt uiTemporalId )
{
  ROF( m_uiDecompositionStages - uiTemporalId >= m_uiNotCodedStages );
  ROF( m_acRefPicListFrameId_L0[ uiFrameIdInGOP ].empty() );
  ROF( m_acRefPicListFrameId_L1[ uiFrameIdInGOP ].empty() );

  //===== check for intra =====
  Bool bIntra  = ( uiFrameIdInGOP == 0 );
  if( !bIntra && ( m_uiLowPassIntraPeriod != MSYS_UINT_MAX || m_iIDRPeriod != 0 || m_iLayerIntraPeriod != 0 ) )
  {
    UInt  uiCurrFrame     = (   m_uiGOPNumber                 << m_uiDecompositionStages ) + uiFrameIdInGOP;
    UInt  uiCurrFrameIDR  = (   uiCurrFrame                   << m_uiTemporalResolution  );
    UInt  uiIntraPeriod   = ( ( m_uiLowPassIntraPeriod + 1  ) << m_uiDecompositionStages );
    Bool  bNormalIntra    = ( m_uiLowPassIntraPeriod != MSYS_UINT_MAX && ( uiCurrFrame    % uiIntraPeriod       ) == 0 );
    Bool  bLayerIntra     = ( m_iLayerIntraPeriod    != 0             && ( uiCurrFrameIDR % m_iLayerIntraPeriod ) == 0 );
    Bool  bIDR            = ( m_iIDRPeriod           != 0             && ( uiCurrFrameIDR % m_iIDRPeriod        ) == 0 );
    bIntra                = ( bNormalIntra || bLayerIntra || bIDR );
  }
  ROT( bIntra && uiTemporalId > 0 );

  //===== set backward and forward list =====
  UInt      uiMaxTId  = ( uiTemporalId ? uiTemporalId - 1 : uiTemporalId );
  UIntList  cFwdList;
  UIntList  cBwdList;
  if( !bIntra )
  {
    for( Int iBwdFrameId = (Int)uiFrameIdInGOP - 1; iBwdFrameId >= 0 && cBwdList.size() < m_uiMaxNumRefFrames; iBwdFrameId-- )
    {
      if( m_auiFrameIdToFrameNumOrLTIdx[ iBwdFrameId ] != MSYS_UINT_MAX && m_auiFrameIdToTemporalId[ iBwdFrameId ] <= uiMaxTId )
      {
        cBwdList.push_back( (UInt)iBwdFrameId );
      }
    }
    for( Int iFwdFrameId = (Int)uiFrameIdInGOP + 1; iFwdFrameId <= (Int)m_uiGOPSize && cFwdList.size() < m_uiMaxNumRefFrames; iFwdFrameId++ )
    {
      if( m_auiFrameIdToFrameNumOrLTIdx[ iFwdFrameId ] != MSYS_UINT_MAX && m_auiFrameIdToTemporalId[ iFwdFrameId ] <= uiMaxTId )
      {
        cFwdList.push_back( (UInt)iFwdFrameId );
      }
    }
    ROT( cBwdList.empty() && cFwdList.empty() );
  }
  if( m_uiPicCodingType == 0 && cBwdList.empty() )
  {
    while( cFwdList.size() )
    {
      cBwdList.push_back( cFwdList.popFront() );
    }
  }


  //===== set picture coding type =====
  m_auiPicCodingType    [ uiFrameIdInGOP ]  = m_uiPicCodingType;
  if( m_auiPicCodingType[ uiFrameIdInGOP ] == 2 && cBwdList.size() && cFwdList.size() && m_uiBaseLayerId != MSYS_UINT_MAX )
  {
    UInt uiShift             = min( m_uiDecompositionStages, m_uiDecompositionStages + 1 - uiTemporalId );
    UInt uiFrmIdInTLayer     = uiFrameIdInGOP >> uiShift;
    UInt uiBasePicCodingType = m_pcH264AVCEncoder->getPicCodingType( m_uiBaseLayerId, uiTemporalId, uiFrmIdInTLayer );
    if(  uiBasePicCodingType == MSYS_UINT_MAX || uiBasePicCodingType == 0 )
    {
      m_auiPicCodingType[ uiFrameIdInGOP ] = 0;
    }
  }

  //===== set reference list sizes =====
  UInt  uiL0Size  = cBwdList.size();
  UInt  uiL1Size  = cFwdList.size();
  if( m_auiPicCodingType[ uiFrameIdInGOP ] == 1 )
  {
    uiL0Size += uiL1Size;
    uiL1Size  = 0;
  }
  else if( m_auiPicCodingType[ uiFrameIdInGOP ] == 2 && uiL1Size )
  {
    uiL0Size += uiL1Size;
    uiL1Size  = uiL0Size;
  }

  //===== set prediction lists =====
  UIntList& rcList0 = m_acRefPicListFrameId_L0[ uiFrameIdInGOP ];
  UIntList& rcList1 = m_acRefPicListFrameId_L1[ uiFrameIdInGOP ];
  if( m_auiPicCodingType[ uiFrameIdInGOP ] == 0 ) // standard mode
  {
    //--- list 0 ---
    for( UIntList::iterator cBIter = cBwdList.begin(); cBIter != cBwdList.end() && rcList0.size() < uiL0Size; cBIter++ )
    {
      rcList0.push_back( *cBIter );
    }
    //--- list 0 ---
    for( UIntList::iterator cFIter = cFwdList.begin(); cFIter != cFwdList.end() && rcList1.size() < uiL1Size; cFIter++ )
    {
      rcList1.push_back( *cFIter );
    }
  }
  else // ( m_uiPicCodingMode == 1 || m_uiPicCodingMode == 2 )
  {
    UIntList::iterator cBIter = cBwdList.begin();
    UIntList::iterator cFIter = cFwdList.begin();
    for( ; cBIter != cBwdList.end() && cFIter != cFwdList.end(); cBIter++, cFIter++ )
    {
      if( m_auiFrameIdToTemporalId[*cFIter] < m_auiFrameIdToTemporalId[*cBIter] )
      {
        rcList0.push_back   ( *cFIter );
        rcList0.push_back   ( *cBIter );
        if( m_auiPicCodingType[ uiFrameIdInGOP ] == 2 )
        {
          rcList1.push_back ( *cBIter );
          rcList1.push_back ( *cFIter );
        }
      }
      else
      {
        rcList0.push_back   ( *cBIter );
        rcList0.push_back   ( *cFIter );
        if( m_auiPicCodingType[ uiFrameIdInGOP ] == 2 )
        {
          rcList1.push_back ( *cFIter );
          rcList1.push_back ( *cBIter );
        }
      }
    }
    for( ; cBIter != cBwdList.end(); cBIter++ )
    {
      rcList0.push_back   ( *cBIter );
      if( m_auiPicCodingType[ uiFrameIdInGOP ] == 2 && uiL1Size  )
      {
        rcList1.push_back ( *cBIter );
      }
    }
    for( ; cFIter != cFwdList.end(); cFIter++ )
    {
      rcList0.push_back   ( *cFIter );
      if( m_auiPicCodingType[ uiFrameIdInGOP ] == 2 )
      {
        rcList1.push_back ( *cFIter );
      }
    }
  }
  //--- final check ---
  ROF( rcList0.size() == uiL0Size );
  ROF( rcList1.size() == uiL1Size );

  return Err::m_nOK;
}

ErrVal
LayerEncoder::xSetReorderingCommandsST( RefPicListReOrdering& rcReOrdering,
                                        UIntList&             cPicNumList,
                                        UInt                  uiCurrPicNum,
                                        Bool                  bNoReordering )
{
  rcReOrdering.clear();
  ROTRS( bNoReordering, Err::m_nOK );
  UInt   uiPred = uiCurrPicNum;
  UInt   uiCnt  = 0;
  while( cPicNumList.size() )
  {
    UInt uiPicNum = cPicNumList.popFront();
    Int  iDiff    = Int( uiPicNum - uiPred );
    AOF( iDiff );
    if ( iDiff < 0 )
    {
      rcReOrdering.set( uiCnt++, RplrCommand( RPLR_NEG, -iDiff - 1 ) );
    }
    else
    {
      rcReOrdering.set( uiCnt++, RplrCommand( RPLR_POS,  iDiff - 1 ) );
    }
    uiPred = uiPicNum;
  }
  if( uiCnt )
  {
    rcReOrdering.set( uiCnt, RplrCommand( RPLR_END ) );
    rcReOrdering.setRefPicListReorderingFlag( true );
  }
  return Err::m_nOK;
}

ErrVal
LayerEncoder::xSetReorderingCommandsLT( RefPicListReOrdering& rcReOrdering,
                                        UIntList&             cLTPicNumList )
{
  rcReOrdering.clear();
  UInt   uiCnt = 0;
  while( cLTPicNumList.size() )
  {
    UInt uiLTPicNum = cLTPicNumList.popFront();
    rcReOrdering.set( uiCnt++, RplrCommand( RPLR_LONG, uiLTPicNum ) );
  }
  if( uiCnt )
  {
    rcReOrdering.set( uiCnt,   RplrCommand( RPLR_END ) );
    rcReOrdering.setRefPicListReorderingFlag( true );
  }
  return Err::m_nOK;
}

ErrVal
LayerEncoder::xSetDecRefPicMarkingST( DecRefPicMarking& rcDecRefPicMarking,
                                      UIntList&         cPicNumList,
                                      UInt              uiCurrPicNum,
                                      UInt              uiMaxPicNum )
{
  UInt uiCnt = 0;
  ROTRS( cPicNumList.empty(), Err::m_nOK );
  while( cPicNumList.size () )
  {
    UInt  uiPicNum  = cPicNumList.popFront();
    UInt  uiDiff    = ( ( uiCurrPicNum + uiMaxPicNum - uiPicNum ) % uiMaxPicNum );
    ROF(  uiDiff );
    rcDecRefPicMarking.set( uiCnt++, MmcoCommand( MMCO_SHORT_TERM_UNUSED, uiDiff - 1 ) );
  }
  rcDecRefPicMarking.set( uiCnt, MmcoCommand( MMCO_END ) );
  rcDecRefPicMarking.setAdaptiveRefPicMarkingModeFlag( true );
  return Err::m_nOK;
}

ErrVal
LayerEncoder::xSetDecRefPicMarkingLT( DecRefPicMarking& rcDecRefPicMarking,
                                      UInt              uiLongTermFrameNum,
                                      Bool              bSendMaxLongTermIdx,
                                      Int               iMaxLongTermIdx )
{
  ROF( iMaxLongTermIdx >= (Int)uiLongTermFrameNum );
  UInt uiCnt = 0;
  if ( bSendMaxLongTermIdx )
  {
    rcDecRefPicMarking.set( uiCnt++, MmcoCommand( MMCO_MAX_LONG_TERM_IDX, UInt( 1 + iMaxLongTermIdx ) ) );
  }
  rcDecRefPicMarking.set  ( uiCnt++, MmcoCommand( MMCO_SET_LONG_TERM, uiLongTermFrameNum ) );
  rcDecRefPicMarking.set  ( uiCnt,   MmcoCommand( MMCO_END ) );
  rcDecRefPicMarking.setAdaptiveRefPicMarkingModeFlag( true );
  return Err::m_nOK;
}

ErrVal
LayerEncoder::xSetSliceTypeAndRefLists( SliceHeader& rcSliceHeader, UInt uiFrameIdInGOP, PicType ePicType )
{
  Bool      bFieldPic       = ( ePicType != FRAME );
  Bool      b2ndField       = ( ePicType == ( m_bBotFieldFirst ? TOP_FIELD : BOT_FIELD ) );
  UInt      uiCurrFrameNum  = ( m_bUseLongTermPics ? rcSliceHeader.getLongTermFrameIdx() : rcSliceHeader.getFrameNum() );
  UInt      uiCurrPicNum    = ( bFieldPic ? 2 * uiCurrFrameNum + 1 : uiCurrFrameNum );
  UIntList& rcFrameIdList0  = m_acRefPicListFrameId_L0[ uiFrameIdInGOP ];
  UIntList& rcFrameIdList1  = m_acRefPicListFrameId_L1[ uiFrameIdInGOP ];

  //===== generate pic num lists =====
  UIntList  cPicNumList0;
  UIntList  cPicNumList1;
  if( b2ndField )
  {
    ROT( uiCurrFrameNum == MSYS_UINT_MAX );
    cPicNumList0.push_back( uiCurrPicNum - 1 );
  }
  for( UIntList::iterator iter0 = rcFrameIdList0.begin(); iter0 != rcFrameIdList0.end(); iter0++ )
  {
    UInt uiFrameNum =  m_auiFrameIdToFrameNumOrLTIdx[ *iter0 ];
    ROT( uiFrameNum == MSYS_UINT_MAX );
    if ( bFieldPic )
    {
      cPicNumList0.push_back( 2 * uiFrameNum + 1 );
      cPicNumList0.push_back( 2 * uiFrameNum     );
    }
    else
    {
      cPicNumList0.push_back( uiFrameNum );
    }
  }
  for( UIntList::iterator iter1 = rcFrameIdList1.begin(); iter1 != rcFrameIdList1.end(); iter1++ )
  {
    UInt uiFrameNum =  m_auiFrameIdToFrameNumOrLTIdx[ *iter1 ];
    ROT( uiFrameNum == MSYS_UINT_MAX );
    if ( bFieldPic )
    {
      cPicNumList1.push_back( 2 * uiFrameNum + 1 );
      cPicNumList1.push_back( 2 * uiFrameNum     );
    }
    else
    {
      cPicNumList1.push_back( uiFrameNum );
    }
  }

  //===== set active entries =====
  Bool  bOverride = false;
  UInt  uiL0Size  = cPicNumList0.size ();
  UInt  uiL1Size  = cPicNumList1.size ();
  if( uiL0Size )
  {
    bOverride     = ( uiL0Size != rcSliceHeader.getPPS().getNumRefIdxActive( LIST_0 ) );
  }
  if( !bOverride && uiL1Size )
  {
    bOverride     = ( uiL1Size != rcSliceHeader.getPPS().getNumRefIdxActive( LIST_1 ) );
  }
  rcSliceHeader.setNumRefIdxActiveOverrideFlag( bOverride );
  rcSliceHeader.setNumRefIdxL0Active          ( uiL0Size );
  rcSliceHeader.setNumRefIdxL1Active          ( uiL1Size );

  //===== set slice type ====
  SliceType eSliceType    = ( cPicNumList0.empty() ? I_SLICE : cPicNumList1.empty() ? P_SLICE : B_SLICE );
  rcSliceHeader.setSliceType( eSliceType );

  //===== set re-ordering commands =====
  if( m_bUseLongTermPics )
  {
    RNOK( xSetReorderingCommandsLT( rcSliceHeader.getRefPicListReorderingL0(), cPicNumList0 ) );
    RNOK( xSetReorderingCommandsLT( rcSliceHeader.getRefPicListReorderingL1(), cPicNumList1 ) );
  }
  else
  {
    Bool  bSingleRefBFrame  = ( eSliceType == B_SLICE && !bFieldPic && m_auiPicCodingType[ uiFrameIdInGOP ] == 0 && m_uiMaxNumRefFrames == 1 && rcFrameIdList0.front() < uiFrameIdInGOP );
    Bool  bDelayRestriction = ( m_uiFrameDelay < ( 1U << m_uiDecompositionStages ) - 1 );
    Bool  bSendReOrdering   = ( m_bForceReOrderingCommands || !bSingleRefBFrame || bDelayRestriction );
    RNOK( xSetReorderingCommandsST( rcSliceHeader.getRefPicListReorderingL0(), cPicNumList0, uiCurrPicNum, !bSendReOrdering ) );
    RNOK( xSetReorderingCommandsST( rcSliceHeader.getRefPicListReorderingL1(), cPicNumList1, uiCurrPicNum, !bSendReOrdering ) );
  }
  return Err::m_nOK;
}

ErrVal
LayerEncoder::xSetSliceHeaderBase( SliceHeader& rcSliceHeader, UInt uiFrameIdInGOP, PicType ePicType )
{
  ROF( m_auiFrameIdToFrameNumOrLTIdx[ uiFrameIdInGOP ] == MSYS_UINT_MAX ); // frame_num must not be set
  UInt        uiTemporalId    = m_auiFrameIdToTemporalId[ uiFrameIdInGOP ];
  Bool        bFieldPic       = ( ePicType != FRAME );
  Bool        b2ndField       = ( ePicType == ( m_bBotFieldFirst ? TOP_FIELD : BOT_FIELD ) );
  Bool        bRefPic         = ( bFieldPic || uiTemporalId == 0 || m_uiDecompositionStages - uiTemporalId > m_uiNotCodedStages );
  NalRefIdc   eNalRefIdc      = ( !bRefPic ? NAL_REF_IDC_PRIORITY_LOWEST : uiTemporalId == 0 ? NAL_REF_IDC_PRIORITY_HIGHEST : uiTemporalId == 1 ? NAL_REF_IDC_PRIORITY_HIGH : NAL_REF_IDC_PRIORITY_LOW );
  Bool        bKeyPictures    = ( m_uiEncodeKeyPictures == 2 || ( m_uiEncodeKeyPictures == 1 && m_bMGS ) );
  Bool        bUseRefBasePic  = ( bKeyPictures && uiTemporalId == 0 );
  Bool        bIdrFlag        = ( uiFrameIdInGOP == 0 && !b2ndField );
  if( ! bIdrFlag && !b2ndField && m_iIDRPeriod != 0 )
  {
    UInt      uiCurrFrameIdx  = ( ( m_uiGOPNumber << m_uiDecompositionStages ) + uiFrameIdInGOP ) << m_uiTemporalResolution;
    bIdrFlag                  = ( ( uiCurrFrameIdx % m_iIDRPeriod  ) == 0 );
    ROT( bIdrFlag && uiTemporalId > 0 );
  }
  if( bIdrFlag )
  {
    m_uiFrameNum              = 0;
    m_uiNextTL0LongTermIdx    = 0;
    for( UInt uiFId = 0; uiFId <= ( 1U << m_uiDecompositionStages ); uiFId++ )
    {
      m_auiFrameIdToFrameNumOrLTIdx[ uiFId ] = MSYS_UINT_MAX;
    }
    for( UInt uiTId = 0; uiTId <= MAX_DSTAGES; uiTId++ )
    {
      m_acActiveRefListFrameNum[ uiTId ].clear();
      m_abMaxLongTermIdxSend   [ uiTId ] = false;
    }
    m_cActiveRefBasePicListFrameNum.clear();
  }
  NalUnitType eNalUnitType    = ( m_bH264AVCCompatible ? ( bIdrFlag ? NAL_UNIT_CODED_SLICE_IDR : NAL_UNIT_CODED_SLICE ) : NAL_UNIT_CODED_SLICE_SCALABLE );
  UInt        uiMbNumber      = ( m_uiFrameWidthInMb * m_uiFrameHeightInMb ) >> ( bFieldPic ? 1 : 0 );
  UInt        uiCurrFrameNum  = m_uiFrameNum;
  Int         iLongTermIdx    = ( m_bUseLongTermPics && bRefPic ? ( uiTemporalId ? (Int)uiTemporalId + 1 : (Int)m_uiNextTL0LongTermIdx ) : -1 );

  //===== set simple slice header parameters =====
  rcSliceHeader.setNalRefIdc                      ( eNalRefIdc );
  rcSliceHeader.setNalUnitType                    ( eNalUnitType );
  rcSliceHeader.setIdrFlag                        ( bIdrFlag );
  rcSliceHeader.setPriorityId                     ( 0 );
  rcSliceHeader.setNoInterLayerPredFlag           ( ! m_bInterLayerPrediction );
  rcSliceHeader.setDependencyId                   ( m_uiDependencyId );
  rcSliceHeader.setQualityId                      ( 0 );
  rcSliceHeader.setTemporalId                     ( uiTemporalId );
  rcSliceHeader.setUseRefBasePicFlag              ( bUseRefBasePic );
  rcSliceHeader.setDiscardableFlag                ( m_bDiscardable );
  rcSliceHeader.setOutputFlag                     ( true );

  rcSliceHeader.setFieldPicFlag                   ( ePicType != FRAME );
  rcSliceHeader.setBottomFieldFlag                ( ePicType == BOT_FIELD );
  rcSliceHeader.setFrameNum                       ( uiCurrFrameNum );
  rcSliceHeader.setIdrPicId                       ( m_uiIdrPicId );
  rcSliceHeader.setFirstMbInSlice                 ( 0 );
  rcSliceHeader.setLastMbInSlice                  ( uiMbNumber - 1 );
  rcSliceHeader.setNumMbsInSlice                  ( uiMbNumber );
  rcSliceHeader.setConstrainedIntraResamplingFlag ( m_bCIUFlag );
  rcSliceHeader.setDirectSpatialMvPredFlag        ( true );
  rcSliceHeader.setCabacInitIdc                   ( 0 );
  rcSliceHeader.setSliceHeaderQp                  ( 0 );
  rcSliceHeader.setRedundantPicCnt                ( 0 );
  rcSliceHeader.setStoreRefBasePicFlag            ( bUseRefBasePic );
  rcSliceHeader.setQLDiscardable                  ( m_uiQLDiscardable );
  rcSliceHeader.setLayerCGSSNR                    ( m_uiLayerCGSSNR );
  rcSliceHeader.setQualityLevelCGSSNR             ( m_uiQualityLevelCGSSNR );
  rcSliceHeader.setBaseLayerCGSSNR                ( m_uiBaseLayerCGSSNR );
  rcSliceHeader.setBaseQualityLevelCGSSNR         ( m_uiBaseQualityLevelCGSSNR );
  rcSliceHeader.setNoOutputOfPriorPicsFlag        ( uiFrameIdInGOP == 0 );
  rcSliceHeader.setLongTermReferenceFlag          ( m_bUseLongTermPics && bIdrFlag );
  rcSliceHeader.setLongTermFrameIdx               ( iLongTermIdx );
  return Err::m_nOK;
}

ErrVal
LayerEncoder::xSetMMCOAndUpdateParameters( SliceHeader& rcSliceHeader, UInt uiFrameIdInGOP, PicType ePicType )
{
  ROF( m_auiFrameIdToFrameNumOrLTIdx[ uiFrameIdInGOP ] == MSYS_UINT_MAX ); // frame_num must not be set
  UInt        uiTemporalId    = m_auiFrameIdToTemporalId[ uiFrameIdInGOP ];
  Bool        bFieldPic       = ( ePicType != FRAME );
  Bool        b2ndField       = ( ePicType == ( m_bBotFieldFirst ? TOP_FIELD : BOT_FIELD ) );
  Bool        bRefPic         = ( bFieldPic || uiTemporalId == 0 || m_uiDecompositionStages - uiTemporalId > m_uiNotCodedStages );
  Bool        bRefFrmOr2ndFld = ( bRefPic && ( !bFieldPic || b2ndField ) );
  Bool        bKeyPictures    = ( m_uiEncodeKeyPictures == 2 || ( m_uiEncodeKeyPictures == 1 && m_bMGS ) );
  Bool        bUseRefBasePic  = ( bKeyPictures && uiTemporalId == 0 );
  Bool        bIdrFlag        = ( uiFrameIdInGOP == 0 && !b2ndField );
  if( ! bIdrFlag && !b2ndField && m_iIDRPeriod != 0 )
  {
    UInt      uiCurrFrameIdx  = ( ( m_uiGOPNumber << m_uiDecompositionStages ) + uiFrameIdInGOP ) << m_uiTemporalResolution;
    bIdrFlag                  = ( ( uiCurrFrameIdx % m_iIDRPeriod  ) == 0 );
    ROT( bIdrFlag && uiTemporalId > 0 );
  }
  UInt        uiMaxFrameNum   = ( 1U << rcSliceHeader.getSPS().getLog2MaxFrameNum() );
  UInt        uiMaxPicNum     = ( bFieldPic ? 2 * uiMaxFrameNum : uiMaxFrameNum );
  UInt        uiCurrFrameNum  = rcSliceHeader.getFrameNum();
  UInt        uiCurrPicNum    = ( bFieldPic ? 2 * uiCurrFrameNum + 1 : uiCurrFrameNum );
  Int         iLongTermFrmIdx = rcSliceHeader.getLongTermFrameIdx();

  //===== clear MMCO buffer =====
  DecRefPicMarking& rcDecRefPicMarking      = rcSliceHeader.getDecRefPicMarking     ();
  DecRefPicMarking& rcDecRefBasePicMarking  = rcSliceHeader.getDecRefBasePicMarking ();
  rcDecRefPicMarking    .clear( false );
  rcDecRefBasePicMarking.clear( true );

  //===== MMCO for short-term configurations =====
  if( ! m_bUseLongTermPics && uiFrameIdInGOP == m_uiFrameIdWithSmallestTId && bRefFrmOr2ndFld )
  {
    if( bUseRefBasePic && m_uiMMCOBaseEnable )
    {
      UIntList cBasePicNumList;
      while( m_cActiveRefBasePicListFrameNum.size() )
      {
        UInt  uiFrameNumX = m_cActiveRefBasePicListFrameNum.popFront();
        if( bFieldPic )
        {
          cBasePicNumList.push_back( 2 * uiFrameNumX );
          cBasePicNumList.push_back( 2 * uiFrameNumX + 1 );
        }
        else
        {
          cBasePicNumList.push_back( uiFrameNumX );
        }
      }
      RNOK( xSetDecRefPicMarkingST( rcDecRefBasePicMarking, cBasePicNumList, uiCurrPicNum, uiMaxPicNum ) );
    }
    UIntList cPicNumList;
    for( UInt uiTId = 0; uiTId <= MAX_DSTAGES; uiTId++ )
    {
      UInt uiMaxListSize = ( m_uiDecompositionStages - uiTId > m_uiNotCodedStages ? 1 : 0 );
      while( m_acActiveRefListFrameNum[ uiTId ].size() > uiMaxListSize )
      {
        UInt  uiFrameNumX = m_acActiveRefListFrameNum[ uiTId ].popFront();
        if( bFieldPic )
        {
          cPicNumList.push_back( 2 * uiFrameNumX );
          cPicNumList.push_back( 2 * uiFrameNumX + 1 );
        }
        else
        {
          cPicNumList.push_back( uiFrameNumX );
        }
        for( UInt uiFId = 0; uiFId <= m_uiGOPSize; uiFId++ )
        {
          if( m_auiFrameIdToFrameNumOrLTIdx[ uiFId ] == uiFrameNumX )
          {
            m_auiFrameIdToFrameNumOrLTIdx[ uiFId ] = MSYS_UINT_MAX;
          }
        }
      }
    }
    RNOK( xSetDecRefPicMarkingST( rcDecRefPicMarking, cPicNumList, uiCurrPicNum, uiMaxPicNum ) );
  }

  //===== MMCO for long-term configurations =====
  if( m_bUseLongTermPics && bRefPic && !bIdrFlag )
  {
    Bool bSendMaxLongTerm = ( m_iMaxLongTermFrmIdx > 0 && ! m_abMaxLongTermIdxSend[ uiTemporalId ] );
    if(  bSendMaxLongTerm )
    {
      for( UInt uiTId = uiTemporalId; uiTId <= MAX_DSTAGES; uiTId++ )
      {
        m_abMaxLongTermIdxSend[ uiTId ] = true;
      }
    }
    ROF ( iLongTermFrmIdx >= 0 );
    RNOK( xSetDecRefPicMarkingLT( rcDecRefPicMarking, (UInt)iLongTermFrmIdx, bSendMaxLongTerm, m_iMaxLongTermFrmIdx ) );
  }

  //===== update =====
  if( bRefFrmOr2ndFld )
  {
    m_uiFrameNum  = ( m_uiFrameNum + 1 ) % uiMaxFrameNum;
    if( m_bUseLongTermPics )
    {
      ROF( iLongTermFrmIdx >= 0 && iLongTermFrmIdx <= m_iMaxLongTermFrmIdx );
      //----- set frame indication and update next LTI for TL0 (when required) ----
      m_auiFrameIdToFrameNumOrLTIdx[ uiFrameIdInGOP ] = (UInt)iLongTermFrmIdx;
      if( uiTemporalId == 0 && m_iMaxLongTermFrmIdx > 0 )
      {
        m_uiNextTL0LongTermIdx  = ( 1 - m_uiNextTL0LongTermIdx );
      }
    }
    else
    {
      //----- set frame indication and update active reference lists -----
      m_auiFrameIdToFrameNumOrLTIdx[ uiFrameIdInGOP ] = uiCurrFrameNum;
      m_acActiveRefListFrameNum [ uiTemporalId ].push_back( uiCurrFrameNum );
      if( bUseRefBasePic )
      {
        m_cActiveRefBasePicListFrameNum.push_back( uiCurrFrameNum );
      }
    }
  }
  if( bIdrFlag )
  {
    m_uiIdrPicId  = ( m_uiIdrPicId + 1 ) % 3;
  }
  return Err::m_nOK;
}

ErrVal
LayerEncoder::xInitSliceHeader( UInt uiFrameIdInGOP, PicType ePicType )
{
  SliceHeader* pcSliceHeader = m_pacControlData[ uiFrameIdInGOP ].getSliceHeader( ePicType );
  ROF( pcSliceHeader );

  //===== set basic parameters, MMCO commands, and reordering commands =====
  RNOK( xSetSliceHeaderBase         ( *pcSliceHeader, uiFrameIdInGOP, ePicType ) );
  RNOK( xSetSliceTypeAndRefLists    ( *pcSliceHeader, uiFrameIdInGOP, ePicType ) );
  RNOK( xSetMMCOAndUpdateParameters ( *pcSliceHeader, uiFrameIdInGOP, ePicType ) );

  //===== de-blocking filter parameters =====
  if( pcSliceHeader->getPPS().getDeblockingFilterParametersPresentFlag() )
  {
    NalUnitType eNalUnitType  = pcSliceHeader->getNalUnitType();
    UInt        uiFilterIdc   = m_uiFilterIdc;
    if( ( eNalUnitType == NAL_UNIT_CODED_SLICE || eNalUnitType == NAL_UNIT_CODED_SLICE_IDR ) && uiFilterIdc > 2 )
    {
      if( uiFilterIdc == 5 )  uiFilterIdc = 2;
      else                    uiFilterIdc = 0;
    }
    pcSliceHeader->getDeblockingFilterParameter().setDisableDeblockingFilterIdc ( uiFilterIdc         );
    pcSliceHeader->getDeblockingFilterParameter().setSliceAlphaC0Offset         ( 2 * m_iAlphaOffset  );
    pcSliceHeader->getDeblockingFilterParameter().setSliceBetaOffset            ( 2 * m_iBetaOffset   );
  }
  if( pcSliceHeader->getSPS().getInterlayerDeblockingPresent() )
  {
    pcSliceHeader->getInterLayerDeblockingFilterParameter().setDisableDeblockingFilterIdc( m_uiInterLayerFilterIdc );
    pcSliceHeader->getInterLayerDeblockingFilterParameter().setSliceAlphaC0Offset        ( 2 * m_iInterLayerAlphaOffset );
    pcSliceHeader->getInterLayerDeblockingFilterParameter().setSliceBetaOffset           ( 2 * m_iInterLayerBetaOffset );
  }

  //===== set POC =====
  Bool  b2ndField       = ( ePicType == ( m_bBotFieldFirst ? TOP_FIELD : BOT_FIELD ) );
  Int   i2ndFieldOffset = ( b2ndField ? 1 : 0 );
  RNOK( m_pcPocCalculator->setPoc( *pcSliceHeader, m_papcFrame[uiFrameIdInGOP]->getPoc() + i2ndFieldOffset ) );
  m_papcFrame[uiFrameIdInGOP]->setPoc( *pcSliceHeader );
  if( m_papcELFrame )
  {
    m_papcELFrame[ uiFrameIdInGOP ]->setPoc( *pcSliceHeader );
  }

  //===== inter-layer prediction parameters =====
  if( ! pcSliceHeader->getNoInterLayerPredFlag() )
  {
    m_pcResizeParameters->updatePicParameters   ( m_papcFrame[uiFrameIdInGOP]->getPicParameters( ePicType ) );
    pcSliceHeader->setScaledRefLayerLeftOffset  ( m_pcResizeParameters->getLeftFrmOffset  () / 2 );
    pcSliceHeader->setScaledRefLayerTopOffset   ( m_pcResizeParameters->getTopFrmOffset   () / 2 );
    pcSliceHeader->setScaledRefLayerRightOffset ( m_pcResizeParameters->getRightFrmOffset () / 2 );
    pcSliceHeader->setScaledRefLayerBottomOffset( m_pcResizeParameters->getBotFrmOffset   () / 2 );
    pcSliceHeader->setRefLayerChromaPhaseX      ( m_pcResizeParameters->m_iRefLayerChromaPhaseX );
    pcSliceHeader->setRefLayerChromaPhaseY      ( m_pcResizeParameters->m_iRefLayerChromaPhaseY );
  }

  //===== set slice group parameters =====
  pcSliceHeader->setSliceGroupChangeCycle(1);
  pcSliceHeader->FMOInit();

  //S051{
  if( m_bEncSIP && xSIPCheck( pcSliceHeader->getPoc() ) )
  {
    pcSliceHeader->setDiscardableFlag( true );
  }
  //S051}

  return Err::m_nOK;
}


ErrVal
LayerEncoder::xInitSliceHeadersAndRefLists()
{
  //===== set number of frames to encode =====
  m_uiNumFramesLeftInGOP = ( m_uiGOPSize >> m_uiNotCodedStages ) + ( m_bFirstGOPCoded ? 0 : 1 );
  //===== reset parameters =====
  {
    for( UInt uiFrameId = 0; uiFrameId <= ( 1U << MAX_DSTAGES ); uiFrameId++ )
    {
      m_auiFrameIdToFrameNumOrLTIdx [ uiFrameId ] = MSYS_UINT_MAX;
      m_auiPicCodingType            [ uiFrameId ] = MSYS_UINT_MAX;
      m_acRefPicListFrameId_L0      [ uiFrameId ].clear();
      m_acRefPicListFrameId_L1      [ uiFrameId ].clear();
    }
  }
  //===== determine frame id with lowest temporal level (first occurence) =====
  {
    UInt  uiSmallestTId         = MSYS_UINT_MAX;
    m_uiFrameIdWithSmallestTId  = 0;
    for( UInt uiFrameId = 1; uiFrameId <= m_uiGOPSize; uiFrameId++ )
    {
      if( m_auiFrameIdToTemporalId[ uiFrameId ] < uiSmallestTId )
      {
        uiSmallestTId               = m_auiFrameIdToTemporalId[ uiFrameId ];
        m_uiFrameIdWithSmallestTId  = uiFrameId;
      }
    }
  }

  UInt uiCodingIndex = 0;
  //==== set frame number for temporal level zero picture of last GOP =====
  if ( m_bFirstGOPCoded )
  {
    if( m_bUseLongTermPics )
    {
      ROF( m_uiNextTL0LongTermIdx <= 1 );
      m_auiFrameIdToFrameNumOrLTIdx[ uiCodingIndex++ ] = ( m_iMaxLongTermFrmIdx ? 1 - m_uiNextTL0LongTermIdx : m_uiNextTL0LongTermIdx );
    }
    else
    {
      m_auiFrameIdToFrameNumOrLTIdx[ uiCodingIndex++ ] = m_acActiveRefListFrameNum[ 0 ].back();
    }
  }
  //==== determine reference lists and set slice header parameters =====
  for( ; uiCodingIndex <= ( 1U << MAX_DSTAGES ); uiCodingIndex++ )
  {
    UInt uiFrameIdInGOP  = m_auiCodingIndexToFrameId [ uiCodingIndex  ];
    if(  uiFrameIdInGOP <= m_uiGOPSize )
    {
      UInt uiTemporalId  = m_auiFrameIdToTemporalId  [ uiFrameIdInGOP ];
      //----- reference lists -----
      RNOK( xInitRefFrameLists( uiFrameIdInGOP, uiTemporalId ) );
      //----- slice headers -----
      xPAffDecision( uiFrameIdInGOP );
      if( m_pbFieldPicFlag[ uiFrameIdInGOP ] )
      {
        RNOK( xInitSliceHeader( uiFrameIdInGOP, ( m_bBotFieldFirst ? BOT_FIELD : TOP_FIELD ) ) );
        RNOK( xInitSliceHeader( uiFrameIdInGOP, ( m_bBotFieldFirst ? TOP_FIELD : BOT_FIELD ) ) );
      }
      else
      {
        RNOK( xInitSliceHeader( uiFrameIdInGOP, FRAME ) );
      }
    }
  }

  return Err::m_nOK;
}


ErrVal
LayerEncoder::xInitGOP( PicBufferList&  rcPicBufferInputList )
{
  //========== INITIALIZE DECOMPOSITION STRUCTURES ==========
  m_uiGOPSize                         = rcPicBufferInputList.size();
  PicBufferList::iterator cInputIter  = rcPicBufferInputList.begin();
  UInt                    uiFrame     = 0;

  if( m_pbFieldPicFlag )
  {
    ::memset( m_pbFieldPicFlag, false, sizeof(Bool)*m_uiGOPSize );
  }

  if( m_bFirstGOPCoded )
  {
    ASSERT( uiFrame==0 );
    m_papcFrame					[ uiFrame ]->copyAll          ( m_pcAnchorFrameOriginal );
    m_papcFrame         [ uiFrame ]->copyPicParameters(*m_pcAnchorFrameOriginal );
    if( m_papcELFrame )
    {
      m_papcELFrame     [ uiFrame ]->copyPicParameters(*m_pcAnchorFrameOriginal );
    }
    uiFrame++;
  }
  else
  {
    m_uiGOPSize--;
  }
  for( ; uiFrame <= m_uiGOPSize; uiFrame++, cInputIter++ )
  {
    if( m_pcResizeParameters->m_iExtendedSpatialScalability == ESS_PICT )
    {
      RNOK( m_pcResizeParameters->readPictureParameters( m_pESSFile, ! m_bInterlaced ) );
    }

    Int         iPoc        = ( m_bInterlaced ? m_uiFrameCounter++ << ( m_uiTemporalResolution+1 ) : m_uiFrameCounter++ << ( m_uiTemporalResolution ) );
    PicBuffer*  pcPicBuffer = *cInputIter;

    m_papcFrame					[ uiFrame ]->load             ( pcPicBuffer );
    m_papcFrame					[ uiFrame ]->clearPoc         ();
    m_papcFrame					[ uiFrame ]->setPoc           ( iPoc );
    m_papcFrame         [ uiFrame ]->setPicParameters ( *m_pcResizeParameters );
    if( m_papcELFrame )
    {
      m_papcELFrame			[ uiFrame ]->clearPoc         ();
      m_papcELFrame     [ uiFrame ]->setPoc           ( iPoc );
      m_papcELFrame     [ uiFrame ]->setPicParameters ( *m_pcResizeParameters );
    }

    m_pacControlData[ uiFrame ].setSpatialScalability( false );
  }
  if( m_uiGOPSize == ( 1 << m_uiDecompositionStages ) ) // store original anchor frame
  {
    RNOK( m_pcAnchorFrameOriginal ->copyAll           ( m_papcFrame[ m_uiGOPSize ] ) );
    m_pcAnchorFrameOriginal       ->copyPicParameters (*m_papcFrame[ m_uiGOPSize ] );
    m_pcAnchorFrameReconstructed  ->copyPicParameters (*m_papcFrame[ m_uiGOPSize ] );
  }

  //========== INITIALIZE SCALING FACTORS ==========
  for( uiFrame = 0; uiFrame <= m_uiGOPSize; uiFrame++ )
  {
    m_pacControlData[ uiFrame ].clear();
    m_pacControlData[ uiFrame ].setScalingFactor( 1.0 );
    MbDataCtrl* pcMbDataCtrl = m_pacControlData[ uiFrame ].getMbDataCtrl();
    RNOK( pcMbDataCtrl->reset () );
    RNOK( pcMbDataCtrl->clear () );
  }

  RNOK( xInitSliceHeadersAndRefLists() );

  m_uiNewlyCodedBits += m_uiParameterSetBits;
  if( m_pcH264AVCEncoder->getCodingParameter()->getCGSSNRRefinement() )
  {
    m_pcH264AVCEncoder->m_aaadSeqBits[getLayerCGSSNR()][0][0] += m_uiParameterSetBits;
  }
  else
  {
    m_pcH264AVCEncoder->m_aaadSeqBits[m_uiDependencyId][0][0] += m_uiParameterSetBits;
  }

  m_uiLastCodedFrameIdInGOP = MSYS_UINT_MAX;
  m_uiLastCodedTemporalId   = MSYS_UINT_MAX;

  return Err::m_nOK;
}


ErrVal
LayerEncoder::xInitBitCounts()
{
  UInt  uiLowerLayerBits             = m_pcH264AVCEncoder->getNewBits( m_uiBaseLayerId );
  m_uiNewlyCodedBits                += uiLowerLayerBits;
  return Err::m_nOK;
}

UInt
LayerEncoder::getPicCodingType( UInt uiTemporalId, UInt uiFrmIdInTLayer )
{
  ROTRS( uiTemporalId   >  m_uiDecompositionStages, MSYS_UINT_MAX );
  UInt   uiFrameIdInGOP  = ( uiTemporalId ? ( uiFrmIdInTLayer << 1 ) + 1 : uiFrmIdInTLayer ) << ( m_uiDecompositionStages - uiTemporalId );
  ROTRS( uiFrameIdInGOP >= m_uiGOPSize,             MSYS_UINT_MAX );
  return m_auiPicCodingType[ uiFrameIdInGOP ];
}

ErrVal
LayerEncoder::updateMaxSliceSize( UInt uiAUIndex, UInt& ruiMaxSliceSize )
{
  ROF( m_bGOPInitialized );
  UInt  uiCodingIndex       = ( m_bFirstGOPCoded ? uiAUIndex + 1 : uiAUIndex );
  UInt  uiFrameIdInGOP      = m_auiCodingIndexToFrameId[ uiCodingIndex ];
  ROT(  uiFrameIdInGOP      > m_uiGOPSize );
  UInt  uiMaxSliceSizeLayer = m_pacControlData[ uiFrameIdInGOP ].getSliceHeader()->getSPS().getMaxSliceSize( m_pbFieldPicFlag[ uiFrameIdInGOP ] );
  if(   uiMaxSliceSizeLayer < ruiMaxSliceSize )
  {
    ruiMaxSliceSize = uiMaxSliceSizeLayer;
  }
  return Err::m_nOK;
}

ErrVal
LayerEncoder::getBaseLayerStatus( Bool&   bExists,
                                  Bool&   bMotion,
                                  PicType ePicType,
                                  UInt		uiTemporalId )
{
  bExists = false;
  bMotion = false;

  if( m_uiLastCodedFrameIdInGOP != MSYS_UINT_MAX && m_uiLastCodedTemporalId == uiTemporalId )
  {
    bExists = true;
    bMotion = bExists || !m_bH264AVCCompatible;
  }

  //S051{
  if(m_bEncSIP && m_uiLastCodedFrameIdInGOP != MSYS_UINT_MAX && m_uiLastCodedTemporalId == uiTemporalId )
  {
    Int iPoc = m_pacControlData[m_uiLastCodedTemporalId].getSliceHeader( ePicType )->getPoc( ePicType );
    if(xSIPCheck(iPoc))
    {
      bExists = false;
      bMotion = bExists || !m_bH264AVCCompatible;
    }
  }
  //S051}

  return Err::m_nOK;
}

ErrVal
LayerEncoder::getBaseLayerDataAvailability( Frame*&       pcFrame,
                                            Frame*&       pcResidual,
                                            MbDataCtrl*&  pcMbDataCtrl,
                                            Bool          bMotion,
                                            PicType       ePicType,
                                            UInt		      uiTemporalId )
{
  pcFrame      = NULL;
  pcResidual   = NULL;
  pcMbDataCtrl = NULL;

  if( m_uiLastCodedFrameIdInGOP != MSYS_UINT_MAX && m_uiLastCodedTemporalId == uiTemporalId )
  {
    SliceHeader* pcSliceHeader = m_pacControlData[ m_uiLastCodedFrameIdInGOP ].getSliceHeader( ePicType );
    ROF( pcSliceHeader );

    pcFrame       = m_pcSubband;
    pcResidual    = m_pcResidual;
    pcMbDataCtrl  = m_pacControlData[m_uiLastCodedFrameIdInGOP].getMbDataCtrl();

    if( pcMbDataCtrl )
    {
      pcMbDataCtrl->setSliceHeader( pcSliceHeader );
    }
  }

  return Err::m_nOK;
}


ErrVal
LayerEncoder::getBaseLayerData( SliceHeader&  rcELSH,
                                Frame*&       pcFrame,
                                Frame*&       pcResidual,
                                MbDataCtrl*&  pcMbDataCtrl,
                                MbDataCtrl*&  pcMbDataCtrlEL,		// ICU/ETRI FGS_MOT_USE
                                Bool          bSpatialScalability,
                                Bool          bMotion,
                                PicType       ePicType,
                                UInt	      	uiTemporalId )
{
  UInt  uiPos   = MSYS_UINT_MAX;
  pcFrame       = 0;
  pcResidual    = 0;
  pcMbDataCtrl  = 0;

  if( m_uiLastCodedFrameIdInGOP != MSYS_UINT_MAX && m_uiLastCodedTemporalId == uiTemporalId )
  {
    SliceHeader* pcSliceHeader = m_pacControlData[ m_uiLastCodedFrameIdInGOP ].getSliceHeader( ePicType );
    ROF( pcSliceHeader );

    pcFrame       = m_pcSubband;
    pcResidual    = m_pcResidual;
    pcMbDataCtrl  = m_pacControlData[m_uiLastCodedFrameIdInGOP].getMbDataCtrl();
    uiPos         = m_uiLastCodedFrameIdInGOP;

    if( pcMbDataCtrl )
    {
      pcMbDataCtrl->setSliceHeader( pcSliceHeader );
    }
  }


  if( bSpatialScalability )
  {
    RNOK( m_apcFrameTemp[0]->copy( pcFrame, ePicType ) );
  //JVT-R057 LA-RDO{
  if(m_bLARDOEnable)
    m_apcFrameTemp[0]->setChannelDistortion(pcFrame);
  //JVT-R057 LA-RDO}
    pcFrame = m_apcFrameTemp[0];

    SliceHeader* pcSliceHeader = m_pacControlData[ uiPos ].getSliceHeader( ePicType );
    ROF( pcSliceHeader );
    RNOK( m_pcLoopFilter->process(*pcSliceHeader, pcFrame, pcResidual, m_pacControlData[uiPos].getMbDataCtrl(),
                                   &rcELSH.getInterLayerDeblockingFilterParameter(),
                                   m_pacControlData[uiPos].getSpatialScalability() ) );
  }

  pcMbDataCtrlEL = pcMbDataCtrl;

  return Err::m_nOK;
}


ErrVal
LayerEncoder::xSetBaseLayerData( UInt    uiFrameIdInGOP,
                                 PicType ePicType )
{
  ControlData&  rcControlData       = m_pacControlData[ uiFrameIdInGOP ];
  SliceHeader*  pcSliceHeader       = rcControlData.getSliceHeader( ePicType );
  ROF( pcSliceHeader );
  UInt          uiTemporalId        = pcSliceHeader->getTemporalId();
  UInt          uiBaseLayerId       = m_uiBaseLayerId;
  UInt          uiBaseLayerIdMotion = m_uiBaseLayerId;

  if( ! m_bInterLayerPrediction )
  {
    pcSliceHeader->setBaseLayerId           ( MSYS_UINT_MAX );
    pcSliceHeader->setNoInterLayerPredFlag( true );
    //pcSliceHeader->setBaseQualityLevel      ( 0 );
    pcSliceHeader->setAdaptiveBaseModeFlag( false );
    // JVT-U160 LMI
    pcSliceHeader->setAdaptiveResidualPredictionFlag (false);
    rcControlData .setBaseLayer             ( MSYS_UINT_MAX, MSYS_UINT_MAX );
    return Err::m_nOK;
  }

  RNOK( m_pcH264AVCEncoder->getBaseLayerStatus( uiBaseLayerId, uiBaseLayerIdMotion, m_uiDependencyId, ePicType, uiTemporalId ) );

  Bool  bAdaptive = m_bAdaptivePrediction;
  if(   bAdaptive )
  {
    if( uiBaseLayerId != MSYS_UINT_MAX         && // when base layer available
        pcSliceHeader->getTemporalId() == 0 && // only for TL=0 pictures
        m_bMGS                                 && // only when MGS
        m_bSameResBL                           && // only when not lowest MGS layer (QL=0)
        m_uiEncodeKeyPictures   >  0            ) // only when MGS key pictures
    {
      bAdaptive = false;
    }
  }
  else if( m_uiGOPSize == ( 1U << m_uiDecompositionStages ) )
  {
    PicType eTestPicType = ( m_pbFieldPicFlag[ m_uiGOPSize ] ? ( m_bBotFieldFirst ? BOT_FIELD : TOP_FIELD ) : FRAME );
    if( m_pacControlData[ m_uiGOPSize ].getSliceHeader( eTestPicType )->getIdrFlag() )
    {
      UInt uiCodNumIDR  = 0;
      UInt uiCodNumCurr = 0;
      while( m_auiCodingIndexToFrameId[ uiCodNumIDR  ] != m_uiGOPSize )
      {
        uiCodNumIDR++;
        ROF( uiCodNumIDR <= 64 );
      }
      while( m_auiCodingIndexToFrameId[ uiCodNumCurr ] != uiFrameIdInGOP )
      {
        uiCodNumCurr++;
        ROF( uiCodNumCurr <= 64 );
      }
      bAdaptive = ( uiCodNumIDR <= uiCodNumCurr );
    }
  }

  m_pcResizeParameters->updatePicParameters( m_papcFrame[ uiFrameIdInGOP ]->getPicParameters( ePicType ) );
  if( uiBaseLayerId != uiBaseLayerIdMotion && ( bAdaptive || m_pcResizeParameters->getSpatialResolutionChangeFlag() ) )
  {
    uiBaseLayerIdMotion = uiBaseLayerId;
  }

  pcSliceHeader->setBaseLayerId( uiBaseLayerId );
  pcSliceHeader->setRefLayer( m_uiBaseLayerCGSSNR, m_uiBaseQualityLevelCGSSNR );
  pcSliceHeader->setNoInterLayerPredFlag( uiBaseLayerId == MSYS_UINT_MAX );
  Bool bSendSkipSlices = ( uiBaseLayerId != MSYS_UINT_MAX && m_pcLayerParameters->getSliceSkip() != 0 && pcSliceHeader->getTemporalId() >= m_pcLayerParameters->getSliceSkipTLevelStart() );
  pcSliceHeader->setSliceSkipFlag( bSendSkipSlices );
  Bool bAdaptivePredFlag = ( uiBaseLayerId != MSYS_UINT_MAX ? bAdaptive : false );
  pcSliceHeader->setAdaptiveBaseModeFlag          ( bAdaptivePredFlag );
  pcSliceHeader->setAdaptiveMotionPredictionFlag  ( bAdaptivePredFlag );
  pcSliceHeader->setAdaptiveResidualPredictionFlag( bAdaptivePredFlag );
  pcSliceHeader->setAdaptiveILPred                ( bAdaptivePredFlag );
  if( ! bAdaptivePredFlag )
  {
    pcSliceHeader->setDefaultBaseModeFlag           ( ! pcSliceHeader->getNoInterLayerPredFlag() );
    pcSliceHeader->setDefaultMotionPredictionFlag   ( ! pcSliceHeader->getNoInterLayerPredFlag() );
    pcSliceHeader->setDefaultResidualPredictionFlag ( ! pcSliceHeader->getNoInterLayerPredFlag() );
  }
  else
  {
    pcSliceHeader->setDefaultBaseModeFlag           ( false );
    pcSliceHeader->setDefaultMotionPredictionFlag   ( false );
    pcSliceHeader->setDefaultResidualPredictionFlag ( false );
  }
  if( ! pcSliceHeader->getNoInterLayerPredFlag() &&
      ! pcSliceHeader->getAdaptiveBaseModeFlag() &&
        pcSliceHeader->getSliceType() == P_SLICE &&
        pcSliceHeader->getSPS().getConvertedLevelIdc() >= 30 &&
      ! m_pcResizeParameters->getRestrictedSpatialResolutionChangeFlag() )
  {
    pcSliceHeader->setAdaptiveBaseModeFlag          ( true );
    pcSliceHeader->setAdaptiveMotionPredictionFlag  ( true );
    pcSliceHeader->setDefaultBaseModeFlag           ( false );
    pcSliceHeader->setDefaultMotionPredictionFlag   ( false );
  }
  rcControlData.setBaseLayer( uiBaseLayerId, uiBaseLayerIdMotion );

  return Err::m_nOK;
}


Frame*
LayerEncoder::getMGSLPRec( UInt uiLowPassIndex )
{
  ROFRS( uiLowPassIndex <= 1,     0 )
  ROFRS( m_bMGS && m_bSameResEL,  0 );
  return m_apcBaseFrame[ uiLowPassIndex ];
}


Frame*
LayerEncoder::xGetRefFrame( UInt         uiRefIndex,
                            RefListUsage eRefListUsage )
{
  if( eRefListUsage == RLU_REF_BASE_PIC )
  {
    UInt   uiMaxGOPSize = ( 1U << m_uiDecompositionStages );
    UInt   uiBaseIdx    = uiRefIndex / uiMaxGOPSize;
    ROFRS( ( uiRefIndex % uiMaxGOPSize ) == 0, 0 );
    ROFRS( uiBaseIdx <= 1, 0 );
    return m_apcBaseFrame[ uiBaseIdx ];
  }
  if( eRefListUsage <= (Int)m_uiMGSKeyPictureControl &&
      m_papcELFrame                                  &&
     !m_papcELFrame[ uiRefIndex ]->isUnvalid()         )
  {
    return  m_papcELFrame [ uiRefIndex ];
  }
  return    m_papcFrame   [ uiRefIndex ];
}


ErrVal
LayerEncoder::xClearELPics()
{
  ROFRS( m_papcELFrame, Err::m_nOK );

  for( UInt uiIndex = 0; uiIndex <= m_uiMaxGOPSize; uiIndex++ )
  {
    m_papcELFrame[uiIndex]->setUnvalid();
  }
  return Err::m_nOK;
}

ErrVal
LayerEncoder::xUpdateELPics()
{
  ROFRS( m_papcELFrame, Err::m_nOK );

  for( UInt uiIndex = 0; uiIndex <= m_uiGOPSize; uiIndex++ )
  {
    if( m_papcELFrame[uiIndex]->isUnvalid() )
    {
      UInt          uiTemporalId  = m_auiFrameIdToTemporalId[ uiIndex ];
      UInt          uiShift       = ( uiTemporalId ? m_uiDecompositionStages + 1 - uiTemporalId : m_uiDecompositionStages );
      UInt          uiFIdInTId    = uiIndex >> uiShift;
      const Frame*  pcELPic       = m_pcH264AVCEncoder->getELRefPic( m_uiDependencyId, uiTemporalId, uiFIdInTId );
      if( pcELPic )
      {
        RNOK( m_papcELFrame[uiIndex]->copy( const_cast<Frame*>( pcELPic ), FRAME ) );
        RNOK( xFillAndUpsampleFrame( m_papcELFrame[uiIndex], FRAME, m_bFrameMbsOnlyFlag ) );
        m_papcELFrame[uiIndex]->setValid();
      }
    }
  }
  return Err::m_nOK;
}

Frame*
LayerEncoder::getRefPic( UInt uiTemporalId, UInt uiFrameIdInTId )
{
  ROTRS( uiTemporalId   > m_uiDecompositionStages, 0 );
  UInt   uiFrameIdInGOP = ( uiTemporalId ? ( uiFrameIdInTId << 1 ) + 1 : uiFrameIdInTId ) << ( m_uiDecompositionStages - uiTemporalId );
  ROTRS( uiFrameIdInGOP > m_uiGOPSize, 0 );
  Frame* pcRefPic       = 0;
  if( m_abCoded[ uiFrameIdInGOP ] && m_pacControlData[ uiFrameIdInGOP ].getSliceHeader()->getNalRefIdc() )
  {
    pcRefPic = m_papcFrame[ uiFrameIdInGOP ];
  }
  return pcRefPic;
}



ErrVal
LayerEncoder::xSetScalingFactors()
{
  for( UInt uiLevel = 0; uiLevel < m_uiDecompositionStages; uiLevel++ )
  {
    RNOK( xSetScalingFactors( uiLevel ) );
  }
  return Err::m_nOK;
}


ErrVal
LayerEncoder::xSetScalingFactors( UInt uiBaseLevel )
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
    adRateL0[iFrame] = 0.2;
    adRateL1[iFrame] = 0.2;
    adRateBi[iFrame] = 0.6;
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

  //===== get high-pass scaling and set scaling factors =====
  for( iFrame = 0; iFrame <= iLowPassSize; iFrame++ )
  {
    Double dScal = dScalingBase;

    if( iFrame % 2 )
    {
      //===== high-pass pictures =====
      dScal *= ( adRateBi[iFrame]                    ) * ( FACTOR_53_HP - 1.0 ) +
               ( adRateL0[iFrame] + adRateL1[iFrame] ) * ( FACTOR_22_HP - 1.0 ) + 1.0;
    }
    else
    {
      //===== low-pass pictures =====
      dScal *= dScalingLowPass;
    }
    m_pacControlData[ iFrame << uiBaseLevel ].setScalingFactor( dScal );
  }

  return Err::m_nOK;
}



ErrVal
LayerEncoder::xClearBufferExtensions()
{
  for( UInt uiFrame = 0; uiFrame <= m_uiGOPSize; uiFrame++ )
  {
    RNOK( m_papcFrame   [uiFrame]->uninitHalfPel() );
    RNOK( m_papcFrame   [ uiFrame ]->removeFrameFieldBuffer() );
    RNOK( m_pcSubband->uninitHalfPel() );
    RNOK( m_pcSubband->removeFrameFieldBuffer() );
    RNOK( m_pcResidual->uninitHalfPel() );
    RNOK( m_pcResidual->removeFrameFieldBuffer() );
  }
  return Err::m_nOK;
}


ErrVal
LayerEncoder::xGetPredictionLists( RefFrameList&  rcRefList0,
                                   RefFrameList&  rcRefList1,
                                   UInt&          ruiFrameIdCol,
                                   UInt           uiFrameIdInGOP,
                                   PicType        ePicType,
                                   RefListUsage   eRefListUsage,
                                   Bool           bHalfPel )
{
  rcRefList0.reset();
  rcRefList1.reset();
  ruiFrameIdCol                   = MSYS_UINT_MAX;
  SliceHeader*  pcSliceHeader     = m_pacControlData[uiFrameIdInGOP].getSliceHeader( ePicType );
  ROF( pcSliceHeader );
  UInt          uiList0Size       = pcSliceHeader->getNumRefIdxL0Active();
  UInt          uiList1Size       = pcSliceHeader->getNumRefIdxL1Active();
  Bool          bFrameMbsOnlyFlag = pcSliceHeader->getSPS().getFrameMbsOnlyFlag();
  Bool          bFieldPic         = ( ePicType != FRAME );
  Bool          b2ndField         = ( ePicType == ( m_bBotFieldFirst ? TOP_FIELD : BOT_FIELD ) );
  PicType       eOppositePicType  = ( ePicType == TOP_FIELD ? BOT_FIELD : TOP_FIELD );
  UIntList&     rcFrameIdList0    = m_acRefPicListFrameId_L0[ uiFrameIdInGOP ];
  UIntList&     rcFrameIdList1    = m_acRefPicListFrameId_L1[ uiFrameIdInGOP ];
  //===== first field of current frame =====
  if( b2ndField )
  {
    Frame* pcFrame = xGetRefFrame( uiFrameIdInGOP, eRefListUsage );
    if( !  pcFrame->isExtended() )
    {
      if( bHalfPel ) { RNOK( xFillAndUpsampleFrame( pcFrame, FRAME, bFrameMbsOnlyFlag ) ); }
      else           { RNOK( xFillAndExtendFrame  ( pcFrame, FRAME, bFrameMbsOnlyFlag ) ); }
    }
    RNOK( rcRefList0.add( pcFrame->getPic( eOppositePicType ) ) );
    pcFrame->setLongTerm( m_bUseLongTermPics );
  }
  //===== L0 list =====
  for( UIntList::iterator iter0 = rcFrameIdList0.begin(); iter0 != rcFrameIdList0.end(); iter0++ )
  {
    UInt   uiFrmId = *iter0;
    Frame* pcFrame = xGetRefFrame( uiFrmId, eRefListUsage );
    if( !  pcFrame->isExtended() )
    {
      if( bHalfPel ) { RNOK( xFillAndUpsampleFrame( pcFrame, FRAME, bFrameMbsOnlyFlag ) ); }
      else           { RNOK( xFillAndExtendFrame  ( pcFrame, FRAME, bFrameMbsOnlyFlag ) ); }
    }
    if( bFieldPic )
    {
      RNOK( rcRefList0.add( pcFrame->getPic( ePicType ) ) );
      RNOK( rcRefList0.add( pcFrame->getPic( eOppositePicType ) ) );
    }
    else
    {
      RNOK( rcRefList0.add( pcFrame->getPic( FRAME ) ) );
    }
    pcFrame->setLongTerm( m_bUseLongTermPics );
  }
  //===== L1 list =====
  for( UIntList::iterator iter1 = rcFrameIdList1.begin(); iter1 != rcFrameIdList1.end(); iter1++ )
  {
    UInt   uiFrmId = *iter1;
    Frame* pcFrame = xGetRefFrame( uiFrmId, eRefListUsage );
    if( !  pcFrame->isExtended() )
    {
      if( bHalfPel ) { RNOK( xFillAndUpsampleFrame( pcFrame, FRAME, bFrameMbsOnlyFlag ) ); }
      else           { RNOK( xFillAndExtendFrame  ( pcFrame, FRAME, bFrameMbsOnlyFlag ) ); }
    }
    if( bFieldPic )
    {
      RNOK( rcRefList1.add( pcFrame->getPic(  ePicType ) ) );
      RNOK( rcRefList1.add( pcFrame->getPic(  eOppositePicType ) ) );
    }
    else
    {
      RNOK( rcRefList1.add( pcFrame->getPic( FRAME ) ) );
    }
    pcFrame->setLongTerm( m_bUseLongTermPics );
    if( ruiFrameIdCol == MSYS_UINT_MAX )
    {
      ruiFrameIdCol = uiFrmId;
    }
  }
  //===== check =====
  ROF( rcRefList0.getSize() == uiList0Size );
  ROF( rcRefList1.getSize() == uiList1Size );
  return Err::m_nOK;
}



ErrVal
LayerEncoder::xInitBaseLayerData( ControlData&  rcControlData,
                                  UInt          uiBaseLevel,
                                  UInt          uiFrame,
                                  Bool          bMotion,
                                  RefFrameList* pcRefFrameList0,
                                  RefFrameList* pcRefFrameList1,
                                  PicType       ePicType )
{
  //===== init =====
  rcControlData.setBaseLayerRec ( 0 );
  rcControlData.setBaseLayerSbb ( 0 );
  rcControlData.setBaseLayerCtrl( 0 );
  rcControlData.setBaseLayerCtrlField( 0 );

  Frame*     pcBaseFrame         = 0;
  Frame*     pcBaseResidual      = 0;
  MbDataCtrl*   pcBaseDataCtrl      = 0;
  MbDataCtrl*   pcBaseDataCtrlEL	  = 0;

  Bool          bBaseDataAvailable  = false;

  SliceHeader*  pcSliceHeader = rcControlData.getSliceHeader( ePicType );
  ROF( pcSliceHeader );

  if( rcControlData.getBaseLayerIdMotion() != MSYS_UINT_MAX )
  {
     RNOK( m_pcH264AVCEncoder->getBaseLayerDataAvailability( pcBaseFrame,
                                                             pcBaseResidual,
                                                             pcBaseDataCtrl,
                                                             rcControlData.getBaseLayerIdMotion      (),
                                                             bMotion,
                                                             bBaseDataAvailable,
                                                             ePicType,
                                                             pcSliceHeader->getTemporalId() ) );

        // Get resampling mode & obtain again with same resolution interlaced-to-progressive check
    if( bBaseDataAvailable )
    {
      m_pcResizeParameters->m_iLevelIdc                  = m_uiLevelIdc;//jzxu, 02Nov2007
      m_pcResizeParameters->m_bFrameMbsOnlyFlag          = pcSliceHeader->getSPS().getFrameMbsOnlyFlag();
      m_pcResizeParameters->m_bFieldPicFlag              = pcSliceHeader->getFieldPicFlag();
      m_pcResizeParameters->m_bBotFieldFlag              = pcSliceHeader->getBottomFieldFlag();
      m_pcResizeParameters->m_bIsMbAffFrame              = pcSliceHeader->isMbaffFrame();
      m_pcResizeParameters->m_bRefLayerFrameMbsOnlyFlag  = pcBaseDataCtrl->getSliceHeader()->getSPS().getFrameMbsOnlyFlag();
      m_pcResizeParameters->m_bRefLayerFieldPicFlag      = pcBaseDataCtrl->getSliceHeader()->getFieldPicFlag();;
      m_pcResizeParameters->m_bRefLayerIsMbAffFrame      = pcBaseDataCtrl->getSliceHeader()->isMbaffFrame();
      m_pcResizeParameters->m_bRefLayerBotFieldFlag      = pcBaseDataCtrl->getSliceHeader()->getBottomFieldFlag();
      rcControlData.setSpatialScalability( m_pcResizeParameters->getSpatialResolutionChangeFlag() );

      RNOK( m_pcH264AVCEncoder->getBaseLayerData( *pcSliceHeader,
                                                  pcBaseFrame,
                                                  pcBaseResidual,
                                                  pcBaseDataCtrl,
                                                  pcBaseDataCtrlEL,
                                                  m_pcResizeParameters->getSpatialResolutionChangeFlag(),
                                                  rcControlData.getBaseLayerIdMotion(),
                                                  bMotion,
                                                  ePicType,
                                                  pcSliceHeader->getTemporalId() ) );
    }
  }


  //===== motion data =====
  if( pcBaseDataCtrl )
  {
    if( pcBaseDataCtrlEL )
    {
      pcBaseDataCtrlEL->setSliceHeader( pcBaseDataCtrl->getSliceHeader() );
      pcBaseDataCtrl = pcBaseDataCtrlEL;
    }
    pcSliceHeader->setSCoeffResidualPredFlag( m_pcResizeParameters );
    RNOK( m_pcBaseLayerCtrl->initSlice( *pcSliceHeader, PRE_PROCESS, false, NULL ) );
    RNOK( m_pcBaseLayerCtrl->upsampleMotion( pcSliceHeader, m_pcResizeParameters, pcBaseDataCtrl, pcRefFrameList0, pcRefFrameList1, m_pcResizeParameters->m_bFieldPicFlag, (m_uiEssRPChkEnable!=0), m_uiMVThres ) );
    rcControlData.setBaseLayerCtrl( m_pcBaseLayerCtrl );

    if( m_pcResizeParameters->m_bIsMbAffFrame )
    {
      RNOK( m_pcBaseLayerCtrlField->initSlice( *pcSliceHeader, PRE_PROCESS, false, NULL ) );
      RNOK( m_pcBaseLayerCtrlField->upsampleMotion( pcSliceHeader, m_pcResizeParameters, pcBaseDataCtrl, pcRefFrameList0, pcRefFrameList1, true, (m_uiEssRPChkEnable!=0), m_uiMVThres ) );
      rcControlData.setBaseLayerCtrlField( m_pcBaseLayerCtrlField );
    }

//TMM_WP
    SliceHeader *pcSliceHeaderCurr, *pcSliceHeaderBase;
    pcSliceHeaderCurr = rcControlData.getSliceHeader(ePicType);
    pcSliceHeaderBase = pcBaseDataCtrl->getSliceHeader();

    //indicates whether the base layer use wp or not
    m_bBaseLayerWp = pcSliceHeaderBase->getPPS().getWeightedPredFlag();

    pcSliceHeaderCurr->setLumaLog2WeightDenom( pcSliceHeaderBase->getLumaLog2WeightDenom() );
    pcSliceHeaderCurr->setChromaLog2WeightDenom( pcSliceHeaderBase->getChromaLog2WeightDenom() );
    pcSliceHeaderCurr->getPredWeightTable( LIST_0 ).copy( pcSliceHeaderBase->getPredWeightTable( LIST_0 ) );
    pcSliceHeaderCurr->getPredWeightTable( LIST_1 ).copy( pcSliceHeaderBase->getPredWeightTable( LIST_1 ) );
//TMM_WP
  }


  //===== residual data =====
  if( bBaseDataAvailable )
  {
    RNOK( m_pcBaseLayerResidual->residualUpsampling( pcBaseResidual, m_cDownConvert, m_pcResizeParameters, pcBaseDataCtrl ) );
    rcControlData.setBaseLayerSbb( m_pcBaseLayerResidual );
  }


  //==== reconstructed (intra) data =====
  if( bBaseDataAvailable )
  {
    Frame*  pcTempBaseFrame = m_apcFrameTemp[0];
    Frame*  pcTempFrame     = m_apcFrameTemp[1];
    RNOK( m_pcBaseLayerFrame->intraUpsampling( pcBaseFrame, pcTempBaseFrame, pcTempFrame, m_cDownConvert, m_pcResizeParameters,
                                               pcBaseDataCtrl, m_pcBaseLayerCtrl, m_pcBaseLayerCtrlField,
                                               m_pcReconstructionBypass, m_bCIUFlag, m_apabBaseModeFlagAllowedArrays[0], m_apabBaseModeFlagAllowedArrays[1] ) );
    m_pcSliceEncoder->setIntraBLFlagArrays( m_apabBaseModeFlagAllowedArrays );
    rcControlData.setBaseLayerRec( m_pcBaseLayerFrame );

  //JVT-R057 LA-RDO{
  if(m_bLARDOEnable)
    m_pcBaseLayerFrame->setChannelDistortion(pcBaseFrame);
  //JVT-R057 LA-RDO}
  }

  setMCResizeParameters(m_pcResizeParameters);

  return Err::m_nOK;
}

Void LayerEncoder::setMCResizeParameters   (ResizeParameters*				resizeParameters)
{
  m_pcMotionEstimation->setResizeParameters(resizeParameters);
}


ErrVal
LayerEncoder::xInitControlDataMotion( UInt          uiBaseLevel,
                                      UInt          uiFrame,
                                      Bool          bMotionEstimation,
                                      PicType       ePicType )
{
  UInt            uiFrameIdInGOP  = uiFrame << uiBaseLevel;
  ControlData&    rcControlData   = m_pacControlData[uiFrameIdInGOP];
  SliceHeader*    pcSliceHeader   = rcControlData.getSliceHeader  ( ePicType );
  ROF( pcSliceHeader );
  Double          dScalFactor     = rcControlData.getScalingFactor();
  Double          dQpPredData     = m_adBaseQpLambdaMotion[ uiBaseLevel ] - 6.0 * log10( dScalFactor ) / log10( 2.0 );
  // JVT-W043 {
  if ( bRateControlEnable && !pcJSVMParams->m_uiLayerId )
  {
    dQpPredData = m_adBaseQpLambdaMotion[ uiBaseLevel ] = pcGenericRC->m_pcJSVMParams->qp;
  }
  // JVT-W043 }
  if( m_bExplicitQPCascading )
  {
    UInt  uiTLevel               = m_uiDecompositionStages - uiBaseLevel;
    dQpPredData                  = m_adBaseQpLambdaMotion[ uiBaseLevel ] + m_adDeltaQPTLevel[ uiTLevel ];
  }
  Double          dLambda           = 0.85 * pow( 2.0, min( 52.0, dQpPredData ) / 3.0 - 4.0 );
  Int             iQp               = max( MIN_QP, min( MAX_QP, (Int)floor( dQpPredData + 0.5 ) ) );

  pcSliceHeader->setSliceHeaderQp( iQp );
  rcControlData. setLambda       ( dLambda );

  if( bMotionEstimation )
  {
    RefFrameList* pcRefFrameList0 = &rcControlData.getPrdFrameList( LIST_0 );
    RefFrameList* pcRefFrameList1 = &rcControlData.getPrdFrameList( LIST_1 );
    m_pcResizeParameters->updatePicParameters( m_papcFrame[ uiFrameIdInGOP ]->getPicParameters( ePicType ) );
    RNOK( xInitBaseLayerData( rcControlData,uiBaseLevel, uiFrame, true, pcRefFrameList0, pcRefFrameList1, ePicType) );
  }

  return Err::m_nOK;
}


ErrVal
LayerEncoder::xInitControlDataLowPass( UInt           uiFrameIdInGOP,
                                       UInt           uiBaseLevel,
                                       UInt           uiFrame,
                                       PicType        ePicType )
{
  ControlData&  rcControlData = m_pacControlData[ uiFrameIdInGOP ];
  SliceHeader* pcSliceHeader  = rcControlData.getSliceHeader( ePicType );
  ROF( pcSliceHeader );

  Double        dScalFactor   = rcControlData.getScalingFactor();
  Double        dQP           = m_dBaseQpLambdaMotionLP - 6.0 * log10( dScalFactor ) / log10( 2.0 );
  if( m_bExplicitQPCascading )
  {
    dQP                       = m_dBaseQpLambdaMotionLP + m_adDeltaQPTLevel[ 0 ];
  }
  // JVT-W043 {
  if ( bRateControlEnable && !pcJSVMParams->m_uiLayerId )
  {
    dQP = m_dBaseQpLambdaMotionLP = pcGenericRC->m_pcJSVMParams->qp;
  }
  // JVT-W043 }
  Double        dLambda         = 0.85 * pow( 2.0, min( 52.0, dQP ) / 3.0 - 4.0 );
  dQP                           = m_dBaseQPResidual - 6.0 * log10( dScalFactor ) / log10( 2.0 );
  if( m_bExplicitQPCascading )
  {
    dQP                         = m_dBaseQPResidual + m_adDeltaQPTLevel[ 0 ];
  }
  // JVT-W043 {
  if ( bRateControlEnable && !pcJSVMParams->m_uiLayerId )
  {
    dQP = m_dBaseQPResidual = pcGenericRC->m_pcJSVMParams->qp;
  }
  // JVT-W043 }
  Int           iQP             = max( MIN_QP, min( MAX_QP, (Int)floor( dQP + 0.5 ) ) );

  pcSliceHeader->setSliceHeaderQp ( iQP );
  rcControlData. setLambda        ( dLambda );

  RefFrameList& rcRefFrameList0 = rcControlData.getPrdFrameList( LIST_0 );
  RefFrameList& rcRefFrameList1 = rcControlData.getPrdFrameList( LIST_1 );
  UInt          uiFrameIdCol    = MSYS_UINT_MAX;
  RNOK( xGetPredictionLists( rcRefFrameList0, rcRefFrameList1, uiFrameIdCol, uiFrameIdInGOP, ePicType, RLU_REF_BASE_PIC, false ) );
  m_apcBaseFrame[0]->clearExtended();
  m_apcBaseFrame[1]->clearExtended();

  m_pcResizeParameters->updatePicParameters( m_papcFrame[ uiFrameIdInGOP ]->getPicParameters( ePicType ) );
  RNOK( xInitBaseLayerData( rcControlData,uiBaseLevel,uiFrame, false, &rcRefFrameList0, &rcRefFrameList1, ePicType ) );

  /* el & has correspongding bl frame & bl does wp then use bl wts */
  if((m_uiDependencyId > 0 && m_bBaseLayerWp &&
      rcControlData.getBaseLayerIdMotion() != MSYS_UINT_MAX) )
  {
      /* use same wts as bl */
      pcSliceHeader->setBasePredWeightTableFlag(true);
  }
  else
  {
      /* call function to calculate the weights */
      m_pcSliceEncoder->xSetPredWeights( *pcSliceHeader,
                                         m_papcFrame[uiFrameIdInGOP],
                                         rcRefFrameList0,
                                         rcRefFrameList1 );
      pcSliceHeader->setBasePredWeightTableFlag(false);
  }

  rcControlData.getPrdFrameList ( LIST_0 ).reset();
//TMM_WP

  return Err::m_nOK;
}


ErrVal
LayerEncoder::xInitControlDataHighPass( UInt          uiFrameIdInGOP,
                                        UInt          uiBaseLevel,
                                        UInt          uiFrame,
                                        PicType       ePicType )
{
  ControlData&  rcControlData = m_pacControlData[ uiFrameIdInGOP ];
  SliceHeader*  pcSliceHeader = rcControlData.getSliceHeader( ePicType );
  ROF( pcSliceHeader );

  Double        dScalFactor   = rcControlData.getScalingFactor();
  Double        dQP           = m_dBaseQPResidual - 6.0 * log10( dScalFactor ) / log10( 2.0 );
  if( m_bExplicitQPCascading )
  {
    UInt  uiTLevel            = m_uiDecompositionStages - uiBaseLevel;
    dQP                       = m_dBaseQPResidual + m_adDeltaQPTLevel[ uiTLevel ];
  }
  // JVT-W043 {
  if ( bRateControlEnable && !pcJSVMParams->m_uiLayerId )
  {
    dQP = m_dBaseQPResidual = pcGenericRC->m_pcJSVMParams->qp;
  }
  // JVT-W043 }
  Double        dLambda         = 0.85 * pow( 2.0, min( 52.0, dQP ) / 3.0 - 4.0 );
  Int           iQP             = max( MIN_QP, min( MAX_QP, (Int)floor( dQP + 0.5 ) ) );

  pcSliceHeader->setSliceHeaderQp ( iQP );
  rcControlData. setLambda        ( dLambda );

  RefFrameList* pcRefFrameList0 = &rcControlData.getPrdFrameList( LIST_0 );
  RefFrameList* pcRefFrameList1 = &rcControlData.getPrdFrameList( LIST_1 );
  m_pcResizeParameters->updatePicParameters( m_papcFrame[ uiFrameIdInGOP ]->getPicParameters( ePicType ) );
  RNOK( xInitBaseLayerData( rcControlData, uiBaseLevel, uiFrame, false, pcRefFrameList0, pcRefFrameList1, ePicType ) );

  return Err::m_nOK;
}

ErrVal
LayerEncoder::xMotionEstimationFrame( UInt uiBaseLevel, UInt uiFrame, PicType ePicType ) //TMM
{
  UInt          uiFrameIdInGOP  = uiFrame << uiBaseLevel;
  ControlData&  rcControlData   = m_pacControlData[uiFrameIdInGOP];
  Frame*     pcFrame         = m_papcFrame     [uiFrameIdInGOP];
  Frame*     pcIntraRecFrame = m_apcFrameTemp  [0];

  SliceHeader*  pcSliceHeader = rcControlData.getSliceHeader( ePicType );
  ROF( pcSliceHeader );

  RefFrameList& rcRefFrameList0 = rcControlData.getPrdFrameList( LIST_0 );
  RefFrameList& rcRefFrameList1 = rcControlData.getPrdFrameList( LIST_1 );
  UInt          uiFrameIdCol    = MSYS_UINT_MAX;

    //===== get reference frame lists =====
  RNOK( xGetPredictionLists( rcRefFrameList0, rcRefFrameList1, uiFrameIdCol, uiFrameIdInGOP, ePicType, RLU_MOTION_ESTIMATION, true ) );
  RefFrameList acRefFieldList0[2];
  RefFrameList acRefFieldList1[2];
  pcSliceHeader->setRefFrameList( &rcRefFrameList0, ePicType, LIST_0 );
  pcSliceHeader->setRefFrameList( &rcRefFrameList1, ePicType, LIST_1 );
  if( pcSliceHeader->isMbaffFrame() )
  {
    RNOK( gSetFrameFieldLists( acRefFieldList0[0], acRefFieldList0[1], rcRefFrameList0 ) );
    RNOK( gSetFrameFieldLists( acRefFieldList1[0], acRefFieldList1[1], rcRefFrameList1 ) );
    pcSliceHeader->setRefFrameList( &(acRefFieldList0[0]), TOP_FIELD, LIST_0 );
    pcSliceHeader->setRefFrameList( &(acRefFieldList0[1]), BOT_FIELD, LIST_0 );
    pcSliceHeader->setRefFrameList( &(acRefFieldList1[0]), TOP_FIELD, LIST_1 );
    pcSliceHeader->setRefFrameList( &(acRefFieldList1[1]), BOT_FIELD, LIST_1 );
  }

    // JVT-W043 {
    if ( bRateControlEnable && !pcJSVMParams->m_uiLayerId )
    {
      pcGenericRC->m_pcJSVMParams->current_mb_nr = 0;
      pcGenericRC->m_pcJSVMParams->current_frame_number = m_uiGOPSize * m_uiGOPNumber + uiFrameIdInGOP;
      pcGenericRC->m_pcJSVMParams->type = B_SLICE;
      pcJSVMParams->CurrGopLevel = pcGenericRC->getCurrGopLevel( pcGenericRC->m_pcJSVMParams->current_frame_number );
      pcGenericRC->m_pcJSVMParams->nal_reference_idc = (pcJSVMParams->CurrGopLevel == 0) ? 0 : 1;
      pcGenericRC->m_pcJSVMParams->qp = pcQuadraticRC->updateQPRC2(0);
    }
    // JVT-W043 }

    //===== set lambda and QP =====
    RNOK( xInitControlDataMotion  ( uiBaseLevel, uiFrame, true, ePicType ) );

  //S051{
  m_pcSliceEncoder->setUseBDir(true);
  if(m_bEncSIP)
  {
    if(m_bH264AVCCompatible||!rcControlData.getSliceHeader( ePicType )->getDirectSpatialMvPredFlag())
    {
      if( uiFrameIdCol != MSYS_UINT_MAX)
      {
        SliceHeader* pcSliceHeaderL1 = m_pacControlData[uiFrameIdCol].getSliceHeader( ePicType );
        if( xSIPCheck( pcSliceHeaderL1->getPoc() ) )
        {
          m_pcSliceEncoder->setUseBDir(false);
        }
      }
    }
  }
//S051}

//TMM_WP
  /* el & has correspongding bl frame & bl uses wp then use bl wts */
  if((m_uiDependencyId > 0 && m_bBaseLayerWp &&
      rcControlData.getBaseLayerIdMotion() != MSYS_UINT_MAX) )
  {
      /* use same wts as bl */
      pcSliceHeader->setBasePredWeightTableFlag(true);
  }
  else
  {
      /* call function to calculate the weights */
      m_pcSliceEncoder->xSetPredWeights( *pcSliceHeader,
                                         pcFrame,
                                         rcRefFrameList0,
                                         rcRefFrameList1
                                         );
      pcSliceHeader->setBasePredWeightTableFlag(false);
  }
//TMM_WP
  // JVT-R057 LA-RDO{
  if(m_bLARDOEnable)
  {
    pcFrame->initChannelDistortion();
    m_pcSliceEncoder->getMbEncoder()->setFrameEcEp(m_papcFrame[(uiFrame-1)<<uiBaseLevel]);
  }
// JVT-R057 LA-RDO}

  //===== motion estimation =====
  if( pcSliceHeader->isMbaffFrame() )
  {
    MbDataCtrl* pcMbDataCtrlCol = ( uiFrameIdCol == MSYS_UINT_MAX ? 0 : m_pacControlData[uiFrameIdCol].getMbDataCtrl() );
    RNOK( xMotionEstimationMbAff( &rcRefFrameList0,
                                  &rcRefFrameList1,
                                  pcMbDataCtrlCol,
                                  pcFrame,
                                  pcIntraRecFrame,
                                  rcControlData,
                                  m_uiNumMaxIter,
                                  m_uiIterSearchRange,
                                  uiFrameIdInGOP ) );
  }
  else
  {
    MbDataCtrl* pcMbDataCtrlCol = ( uiFrameIdCol == MSYS_UINT_MAX ? 0 : m_pacControlData[uiFrameIdCol].getMbDataCtrl() );
    RNOK( xMotionEstimation ( &rcRefFrameList0,
                              &rcRefFrameList1,
                              pcMbDataCtrlCol,
                              pcFrame,
                              pcIntraRecFrame,
                              rcControlData,
                              m_uiNumMaxIter,
                              m_uiIterSearchRange,
                              uiFrameIdInGOP ,
                              ePicType ) );
  }
  return Err::m_nOK;
}


ErrVal
LayerEncoder::xDecompositionFrame( UInt uiBaseLevel, UInt uiFrame, PicType ePicType ) //TMM
{
  UInt          uiFrameIdInGOP  = uiFrame << uiBaseLevel;
  ControlData&  rcControlData   = m_pacControlData[uiFrameIdInGOP];
  Frame*     pcFrame         = m_papcFrame     [uiFrameIdInGOP];
  Frame*     pcResidual      = m_pcResidual;
  Frame*     pcMCFrame       = m_apcFrameTemp  [0];

  SliceHeader*  pcSliceHeader = rcControlData.getSliceHeader( ePicType );
  ROF( pcSliceHeader );

  RefFrameList& rcRefFrameList0 = rcControlData.getPrdFrameList( LIST_0 );
  RefFrameList& rcRefFrameList1 = rcControlData.getPrdFrameList( LIST_1 );
  UInt          uiFrameIdCol    = MSYS_UINT_MAX;

  //===== get reference frame lists =====
  RNOK( xGetPredictionLists( rcRefFrameList0, rcRefFrameList1, uiFrameIdCol, uiFrameIdInGOP, ePicType, RLU_GET_RESIDUAL, false ) );
  RefFrameList acRefFieldList0[2];
  RefFrameList acRefFieldList1[2];
  pcSliceHeader->setRefFrameList( &rcRefFrameList0, ePicType, LIST_0 );
  pcSliceHeader->setRefFrameList( &rcRefFrameList1, ePicType, LIST_1 );
  if( pcSliceHeader->isMbaffFrame() )
  {
    RNOK( gSetFrameFieldLists( acRefFieldList0[0], acRefFieldList0[1], rcRefFrameList0 ) );
    RNOK( gSetFrameFieldLists( acRefFieldList1[0], acRefFieldList1[1], rcRefFrameList1 ) );
    pcSliceHeader->setRefFrameList( &(acRefFieldList0[0]), TOP_FIELD, LIST_0 );
    pcSliceHeader->setRefFrameList( &(acRefFieldList0[1]), BOT_FIELD, LIST_0 );
    pcSliceHeader->setRefFrameList( &(acRefFieldList1[0]), TOP_FIELD, LIST_1 );
    pcSliceHeader->setRefFrameList( &(acRefFieldList1[1]), BOT_FIELD, LIST_1 );
  }

  // JVT-W043 {
  if ( bRateControlEnable && !pcJSVMParams->m_uiLayerId )
  {
    pcGenericRC->m_pcJSVMParams->current_mb_nr = 0;
    pcGenericRC->m_pcJSVMParams->current_frame_number = m_uiGOPSize * m_uiGOPNumber + uiFrameIdInGOP;
    pcGenericRC->m_pcJSVMParams->type = B_SLICE;
    pcJSVMParams->CurrGopLevel = pcGenericRC->getCurrGopLevel( pcGenericRC->m_pcJSVMParams->current_frame_number );
    pcGenericRC->m_pcJSVMParams->nal_reference_idc = (pcJSVMParams->CurrGopLevel == 0) ? 0 : 1;
    pcGenericRC->m_pcJSVMParams->qp = pcQuadraticRC->updateQPRC2(0);
  }
  // JVT-W043 }

  //===== set lambda and QP =====// warnnig false in 7.8
    RNOK( xInitControlDataMotion( uiBaseLevel, uiFrame, true, ePicType ) );

  setBaseMbDataCtrl(rcControlData.getBaseLayerCtrl());

  //===== prediction =====
    RNOK( xMotionCompensation  ( pcMCFrame, &rcRefFrameList0, &rcRefFrameList1, rcControlData.getMbDataCtrl(), *pcSliceHeader ) );
    RNOK( pcFrame->prediction  ( pcMCFrame, pcFrame,        ePicType ) );

  //===== set residual =====
    RNOK( pcResidual->copy     ( pcFrame                  , ePicType ) );
    RNOK( xZeroIntraMacroblocks( pcResidual, rcControlData, ePicType ) );

    return Err::m_nOK;
}


ErrVal
LayerEncoder::xCompositionFrame( UInt uiBaseLevel, UInt uiFrame, PicBufferList& rcPicBufferInputList, PicType ePicType ) //TMM
{
  UInt          uiFrameIdInGOP  = uiFrame << uiBaseLevel;
  ControlData&  rcControlData   = m_pacControlData[uiFrameIdInGOP];
  Frame*     pcFrame         = m_papcFrame     [uiFrameIdInGOP];
  Frame*     pcResidual     = m_pcResidual;
  Frame*     pcMCFrame       = m_apcFrameTemp  [0];

  setBaseMbDataCtrl(rcControlData.getBaseLayerCtrl());

  SliceHeader*  pcSliceHeader = m_pacControlData[ uiFrameIdInGOP ].getSliceHeader( ePicType );
  ROF( pcSliceHeader );

  // JVT-W043 {
  if ( bRateControlEnable && !pcJSVMParams->m_uiLayerId )
  {
    pcGenericRC->m_pcJSVMParams->current_mb_nr = 0;
    pcGenericRC->m_pcJSVMParams->current_frame_number = m_uiGOPSize * m_uiGOPNumber + uiFrameIdInGOP;
    pcGenericRC->m_pcJSVMParams->type = B_SLICE;
    pcJSVMParams->CurrGopLevel = pcGenericRC->getCurrGopLevel( pcGenericRC->m_pcJSVMParams->current_frame_number );
    pcGenericRC->m_pcJSVMParams->nal_reference_idc = (pcJSVMParams->CurrGopLevel == 0) ? 0 : 1;
    pcGenericRC->m_pcJSVMParams->qp = pcQuadraticRC->updateQPRC2(0);
  }
  // JVT-W043 }

    //===== get reference frame lists =====
    RefFrameList& rcRefFrameList0 = rcControlData.getPrdFrameList( LIST_0 );
    RefFrameList& rcRefFrameList1 = rcControlData.getPrdFrameList( LIST_1 );
    UInt          uiFrameIdCol    = MSYS_UINT_MAX;

    RNOK( xGetPredictionLists( rcRefFrameList0, rcRefFrameList1, uiFrameIdCol, uiFrameIdInGOP, ePicType, RLU_RECONSTRUCTION, false ) );
    RefFrameList acRefFieldList0[2];
    RefFrameList acRefFieldList1[2];
    pcSliceHeader->setRefFrameList( &rcRefFrameList0, ePicType, LIST_0 );
    pcSliceHeader->setRefFrameList( &rcRefFrameList1, ePicType, LIST_1 );
    if( pcSliceHeader->isMbaffFrame() )
    {
      RNOK( gSetFrameFieldLists( acRefFieldList0[0], acRefFieldList0[1], rcRefFrameList0 ) );
      RNOK( gSetFrameFieldLists( acRefFieldList1[0], acRefFieldList1[1], rcRefFrameList1 ) );
      pcSliceHeader->setRefFrameList( &(acRefFieldList0[0]), TOP_FIELD, LIST_0 );
      pcSliceHeader->setRefFrameList( &(acRefFieldList0[1]), BOT_FIELD, LIST_0 );
      pcSliceHeader->setRefFrameList( &(acRefFieldList1[0]), TOP_FIELD, LIST_1 );
      pcSliceHeader->setRefFrameList( &(acRefFieldList1[1]), BOT_FIELD, LIST_1 );
    }

    //===== prediction =====
    RNOK( xMotionCompensation         ( pcMCFrame, &rcRefFrameList0, &rcRefFrameList1,
                                      rcControlData.getMbDataCtrl(), *pcSliceHeader ) );

    RNOK( pcFrame ->inversePrediction ( pcMCFrame, pcFrame, ePicType ) );
   //--
    //----- store non-deblocked signal for inter-layer prediction -----
    RNOK( m_pcSubband->copy( pcFrame, ePicType ) );

    // JVT-R057 LA-RDO{
    if(m_bLARDOEnable)
    {
      m_pcSubband->setChannelDistortion(pcFrame);
    }
    // JVT-R057 LA-RDO}

    if( ! pcSliceHeader->getNoInterLayerPredFlag() )
    {
      m_pcSliceEncoder->updateBaseLayerResidual( rcControlData, m_uiFrameWidthInMb );
    }

    //===== de-blocking =====
    RNOK( m_pcLoopFilter->process( *pcSliceHeader, pcFrame, pcResidual, rcControlData.getMbDataCtrl(), 0, rcControlData.getSpatialScalability() ) );

    if( ePicType == ( m_bBotFieldFirst ? BOT_FIELD : TOP_FIELD ) )
    {
      RNOK( m_apcFrameTemp[ 5 ]->uninitHalfPel    ()                    );
      RNOK( m_apcFrameTemp[ 5 ]->removeFieldBuffer( ePicType )          );
      RNOK( m_apcFrameTemp[ 5 ]->copy             ( pcFrame, ePicType ) );
    }

  return Err::m_nOK;
}

//--


ErrVal
LayerEncoder::xEncodeKeyPicture( Bool&               rbKeyPicCoded,
                                 UInt                uiFrame,
                                 AccessUnitData&     rcAccessUnitData,
                                 PicOutputDataList&  rcPicOutputDataList )
{
  rbKeyPicCoded         = false;
  UInt   uiFrameIdInGOP = uiFrame << m_uiDecompositionStages;
  ROTRS( uiFrameIdInGOP > m_uiGOPSize, Err::m_nOK );

  PicType eFirstPic = ( m_pbFieldPicFlag[uiFrameIdInGOP] ? ( m_bBotFieldFirst ? BOT_FIELD : TOP_FIELD ) : FRAME );
  if( m_pacControlData[uiFrameIdInGOP].getSliceHeader( eFirstPic )->getIdrFlag() )
  {
    ::memset( m_abCoded, 0x00, sizeof( m_abCoded ) );
  }
  m_abCoded[uiFrameIdInGOP] = true;

  //===== check for first GOP =====
  if( uiFrame == 0 && m_uiGOPNumber )
  {
    //====== don't code first anchor picture if it was coded within the last GOP =====
    RNOK( m_papcFrame[ uiFrameIdInGOP ]->copyAll( m_pcAnchorFrameReconstructed ) );
    RNOK( m_apcBaseFrame[0]->copyAll          (  m_apcBaseFrame[1] ) );
    RNOK( m_apcBaseFrame[0]->copyPicParameters( *m_apcBaseFrame[1] ) );
    // JVT-R057 LA-RDO{
    if(m_bLARDOEnable)
    {
      m_apcBaseFrame[0]->copyChannelDistortion( m_apcBaseFrame[1] );
      m_papcFrame   [0]->copyChannelDistortion( m_apcBaseFrame[1] );
      m_pcSubband      ->setChannelDistortion ( m_papcFrame   [0] );
    }
    // JVT-R057 LA-RDO}
    return Err::m_nOK;
  }
  //JVT-W051 {
  if ( rbKeyPicCoded == 0 )
  {
    Int iPicType = ( m_pbFieldPicFlag[ uiFrameIdInGOP ] ? ( m_bBotFieldFirst ? BOT_FIELD : TOP_FIELD ) : FRAME );
    PicType ePicType               = PicType( iPicType );
    ControlData&  rcControlData   = m_pacControlData[ uiFrameIdInGOP ];
    SliceHeader* pcSliceHeader           = rcControlData.getSliceHeader( ePicType );
    m_uiProfileIdc      = pcSliceHeader->getSPS().getProfileIdc();
    m_uiLevelIdc        = pcSliceHeader->getSPS().getLevelIdc();
    m_bConstraint0Flag  = pcSliceHeader->getSPS().getConstrainedSet0Flag();
    m_bConstraint1Flag  = pcSliceHeader->getSPS().getConstrainedSet1Flag();
    m_bConstraint2Flag  = pcSliceHeader->getSPS().getConstrainedSet2Flag();
    m_bConstraint3Flag  = pcSliceHeader->getSPS().getConstrainedSet3Flag();
  }
  //JVT-W051 }

  rbKeyPicCoded                         = true;
  UInt          uiBits          = 0;
  ControlData&  rcControlData   = m_pacControlData[ uiFrameIdInGOP ];
  MbDataCtrl*   pcMbDataCtrl    = rcControlData.getMbDataCtrl ();
  Frame*     pcFrame         = m_papcFrame     [ uiFrameIdInGOP ];
  Frame*     pcResidual      = m_pcResidual;
  Frame*     pcPredSignal    = m_apcFrameTemp  [ 0 ];
  Frame*     pcBLRecFrame    = m_apcFrameTemp  [ 1 ];

  const Bool bFieldCoded =  m_pbFieldPicFlag[ uiFrameIdInGOP ] ;

  UInt    uiNumPics     = ( bFieldCoded ? 2 : 1 );
  PicType eFirstPicType = ( bFieldCoded ? ( m_bBotFieldFirst ? BOT_FIELD : TOP_FIELD ) : FRAME );
  PicType eLastPicType  = ( bFieldCoded ? ( m_bBotFieldFirst ? TOP_FIELD : BOT_FIELD ) : FRAME );
  for( UInt uiPicType = 0; uiPicType < uiNumPics; uiPicType++ )
  {
    PicType ePicType    = ( uiPicType ? eLastPicType : eFirstPicType );
    RNOK( xSetBaseLayerData( uiFrameIdInGOP, ePicType ) );

    SliceHeader* pcSliceHeader           = rcControlData.getSliceHeader( ePicType );
    //JVT-W047
    m_bOutputFlag = pcSliceHeader->getOutputFlag();
    //JVT-W047
    ROF( pcSliceHeader );

    ExtBinDataAccessorList& rcOutputList    = rcAccessUnitData.getNalUnitList();
    ExtBinDataAccessorList& rcRedOutputList = rcAccessUnitData.getRedNalUnitList();

    // JVT-W043 {
    if ( bRateControlEnable && !pcJSVMParams->m_uiLayerId )
    {
      pcGenericRC->m_pcJSVMParams->current_mb_nr = 0;
      pcGenericRC->m_pcJSVMParams->current_frame_number = m_uiGOPSize * m_uiGOPNumber + uiFrameIdInGOP;
      pcGenericRC->m_pcJSVMParams->type =
        ( uiFrame > 0 && (uiFrame % pcJSVMParams->m_uiIntraPeriod) != 0 ) ?
        ( ( pcGenericRC->m_pcJSVMParams->current_frame_number % m_uiGOPSize == 0 ) ? P_SLICE : B_SLICE ) : I_SLICE;
      pcGenericRC->m_pcJSVMParams->nal_reference_idc = 1;
      pcGenericRC->m_pcJSVMParams->number = pcGenericRC->m_pcJSVMParams->current_frame_number;

      pcQuadraticRC->rc_init_pict(1, 0, 1, 1.0F);
      if ( pcJSVMParams->m_uiIntraPeriod != 1 )
        pcGenericRC->m_pcJSVMParams->qp = pcQuadraticRC->updateQPRC2(0);
      else
        pcGenericRC->m_pcJSVMParams->qp = pcQuadraticRC->updateQPRC1(0);
    }
    // JVT-W043 }

    //===== initialize =====
    RNOK( xInitControlDataLowPass ( uiFrameIdInGOP, m_uiDecompositionStages-1, uiFrame, ePicType ) );

    //NonRequired JVT-Q066 (06-04-08){{
    if(m_uiDependencyId != 0 && m_uiNonRequiredWrite != 0)
    {
      if( pcSliceHeader->getNoInterLayerPredFlag() || m_uiDependencyId - pcSliceHeader->getRefLayerDependencyId() > 1 || pcSliceHeader->getRefLayerQualityId() != 3 )
      {
        rcAccessUnitData.CreatNonRequiredSei();
      }
      xSetNonRequiredSEI(pcSliceHeader, rcAccessUnitData.getNonRequiredSei());
      if(m_uiNonRequiredWrite == 2 && rcAccessUnitData.getNonRequiredSei() != NULL)
      {
        xWriteNonRequiredSEI(rcOutputList, rcAccessUnitData.getNonRequiredSei(), uiBits);
      }
    }
    //NonRequired JVT-Q066 (06-04-08)}}

    // JVT-V068 HRD {
    if ( pcSliceHeader->getIdrFlag() && m_bEnableHrd )
    {
      SEI::BufferingPeriod *pcBufferingPeriodSei = NULL;
      for (UInt uiLayer = 0; uiLayer < m_pcSPS->getVUI()->getNumLayers(); uiLayer++)
      {
        UInt uiDependencyId = m_pcSPS->getVUI()->getLayerInfo(uiLayer).getDependencyID();
        UInt uiTemporalLevel = m_pcSPS->getVUI()->getLayerInfo(uiLayer).getTemporalId();
        UInt uiQualityLevel = m_pcSPS->getVUI()->getLayerInfo(uiLayer).getQualityId();
        UInt uiIndex = (uiDependencyId<<7)+(uiTemporalLevel<<4)+uiQualityLevel;
        RNOK( m_apcScheduler->get(uiIndex)->createBufferingSei( pcBufferingPeriodSei, m_pcParameterSetMng, (uiDependencyId<<4)+uiQualityLevel ) );
        if ( uiTemporalLevel == m_pcSPS->getVUI()->getNumTemporalLevels() - 1 && uiDependencyId == 0 && uiQualityLevel == 0 )
        {
          // AVC compatible layer
          SEI::MessageList  cSEIMessageList;
          cSEIMessageList.push_back(pcBufferingPeriodSei);
          RNOK( xWriteSEI(rcOutputList,cSEIMessageList,uiBits) );
        }
        else
        {
          RNOK( xWriteNestingSEIforHrd(rcOutputList, pcBufferingPeriodSei, uiDependencyId, uiQualityLevel, uiTemporalLevel, uiBits) );
        }
      }
      ETRACE_LAYER(0);
    }

    if ( m_bEnableHrd )
    {
      SEI::PicTiming *pcPicTimingSei = NULL;
      for (UInt uiLayer = 0; uiLayer < m_pcSPS->getVUI()->getNumLayers(); uiLayer++)
      {
        UInt uiDependencyId = m_pcSPS->getVUI()->getLayerInfo(uiLayer).getDependencyID();
        UInt uiTemporalLevel = m_pcSPS->getVUI()->getLayerInfo(uiLayer).getTemporalId();
        UInt uiQualityLevel = m_pcSPS->getVUI()->getLayerInfo(uiLayer).getQualityId();
        UInt uiIndex = (uiDependencyId<<7)+(uiTemporalLevel<<4)+uiQualityLevel;
        RNOK( m_apcScheduler->get(uiIndex)->createTimingSei( pcPicTimingSei, m_pcSPS->getVUI(), 0, *pcSliceHeader, bFieldCoded, uiLayer ) );
        if ( uiTemporalLevel == m_pcSPS->getVUI()->getNumTemporalLevels() - 1 && uiDependencyId == 0 && uiQualityLevel == 0 )
        {
          // AVC compatible layer
          SEI::MessageList  cSEIMessageList;
          cSEIMessageList.push_back(pcPicTimingSei);
          RNOK( xWriteSEI(rcOutputList,cSEIMessageList,uiBits) );
        }
        else
        {
          RNOK( xWriteNestingSEIforHrd(rcOutputList, pcPicTimingSei, uiDependencyId, uiQualityLevel, uiTemporalLevel, uiBits ) );
        }
      }
      ETRACE_LAYER(0);
    }
    // JVT-V068 HRD }
    // JVT-W049 {
    if((pcSliceHeader->getPPS().getEnableRedundantKeyPicCntPresentFlag())&&(pcSliceHeader->getDependencyId()==0))
    {
      RNOK(xWriteRedundantKeyPicSEI ( rcOutputList, uiBits ));
    }
    // JVT-W049 }

    //===== base layer encoding =====
    // JVT-Q054 Red. Picture {
    //===== primary picture coding =====
    if ( pcSliceHeader->getPPS().getRedundantPicCntPresentFlag() )
    {
      pcSliceHeader->setRedundantPicCnt( 0 );	// set redundant_pic_cnt to 0 for primary coded picture
      if( rcControlData.getBaseLayerSbb() )
  			m_apcFrameTemp[9]->copy((rcControlData.getBaseLayerSbb()),ePicType);//RPIC bug fix
    }
    // JVT-Q054 Red. Picture }
    RNOK( pcBLRecFrame->copy      ( pcFrame, ePicType ) );

    // JVT-R057 LA-RDO{
    if(m_bLARDOEnable)
    {
      pcFrame->initChannelDistortion();
      m_apcBaseFrame[uiFrame]->initChannelDistortion();
      if( uiFrame == 0 )
      {
        ROT( m_uiGOPNumber );
        pcFrame->zeroChannelDistortion();
        m_pcSliceEncoder->getMbEncoder()->setFrameEcEp(NULL);
      }
      else
      {
        m_pcSliceEncoder->getMbEncoder()->setFrameEcEp( m_apcBaseFrame[0] );
      }
      pcBLRecFrame->setChannelDistortion(pcFrame);

    }
    // JVT-R057 LA-RDO}

    if( m_pcH264AVCEncoder->getCodingParameter()->getCGSSNRRefinement() )
    {
      m_pcH264AVCEncoder->m_aaadSeqBits[getLayerCGSSNR()][pcSliceHeader->getTemporalId()][getQualityLevelCGSSNR()] += uiBits;
    }
    else
    {
      m_pcH264AVCEncoder->m_aaadSeqBits[m_uiDependencyId][pcSliceHeader->getTemporalId()][0] += uiBits;
    }

    RNOK( xEncodeLowPassSignal( rcOutputList, rcControlData,
                                pcFrame, pcBLRecFrame, pcResidual, pcPredSignal,
                                uiBits, rcPicOutputDataList, uiFrameIdInGOP, ePicType ) );
    // JVT-W043 {
    if ( bRateControlEnable && !pcJSVMParams->m_uiLayerId )
    {
      pcQuadraticRC->rc_update_pict_frame( uiBits );
      pcQuadraticRC->rc_update_pict( uiBits );
      /* update quadratic R-D model */
      if ( (pcGenericRC->m_pcJSVMParams->type == P_SLICE ||
        (pcGenericRC->m_pcJSVMParams->RCUpdateMode == RC_MODE_1 && pcGenericRC->m_pcJSVMParams->current_frame_number) )
        && pcGenericRC->m_pcJSVMParams->frame_mbs_only_flag )
        pcQuadraticRC->updateRCModel();
      else if ( (pcGenericRC->m_pcJSVMParams->type == P_SLICE ||
        (pcGenericRC->m_pcJSVMParams->RCUpdateMode == RC_MODE_1 && pcGenericRC->m_pcJSVMParams->current_frame_number) )
        && !(pcGenericRC->m_pcJSVMParams->frame_mbs_only_flag)
        && pcGenericRC->m_iNoGranularFieldRC == 0 )
        pcQuadraticRC->updateRCModel();
    }
    // JVT-W043 }
    // JVT-Q054 Red. Picture {
    if ( pcSliceHeader->getPPS().getRedundantPicCntPresentFlag() )
    {
      //RPIC bug fix {
			MbDataCtrl* pcMbDataCtrlTemp=m_pacControlData[uiFrameIdInGOP].getMbDataCtrl();
			m_pcRedundantCtrl->reset();
      m_pacControlData[uiFrameIdInGOP].setMbDataCtrl(m_pcRedundantCtrl);
			MbDataCtrl* pcBaseLayerCtrlTemp=m_pacControlData[uiFrameIdInGOP].getBaseLayerCtrl();
			m_pcRedundant1Ctrl->reset();
      m_pacControlData[uiFrameIdInGOP].setMbDataCtrl(m_pcRedundant1Ctrl);
			m_apcFrameTemp[6]->copy(pcBLRecFrame,ePicType);
			m_apcFrameTemp[7]->copy(pcResidual,ePicType);
			m_apcFrameTemp[8]->copy(pcPredSignal,ePicType);
      if( rcControlData.getBaseLayerSbb() )
      {
        m_apcFrameTemp[10]->copy((rcControlData.getBaseLayerSbb()),ePicType);
        rcControlData.getBaseLayerSbb()->copy(m_apcFrameTemp[9],ePicType);
      }
      //RPIC bug fix }
      // in current version, slice repetition is supported for each primary coded slice
      UInt  uiRedundantPicNum = 1;  // number of redundant pictures for each primary coded picture
      UInt  uiRedundantPicCnt = 0;
      for ( uiRedundantPicCnt = 1; uiRedundantPicCnt <= uiRedundantPicNum; uiRedundantPicCnt++)
      {
        pcSliceHeader->setRedundantPicCnt( uiRedundantPicCnt );

        RNOK( pcBLRecFrame->copy  ( pcFrame,ePicType ) );
        RNOK( xEncodeLowPassSignal( rcRedOutputList, rcControlData,
                                    pcFrame, pcBLRecFrame, pcResidual, pcPredSignal,
                                    uiBits, rcPicOutputDataList, uiFrameIdInGOP, ePicType ) );
      }
      pcSliceHeader->setRedundantPicCnt( 0 );
      //RPIC bug fix {
      m_pacControlData[uiFrameIdInGOP].setMbDataCtrl(pcMbDataCtrlTemp);
			m_pacControlData[uiFrameIdInGOP].setBaseLayerCtrl(pcBaseLayerCtrlTemp);
			pcBLRecFrame->copy(m_apcFrameTemp[6],ePicType);
			pcResidual->copy(m_apcFrameTemp[7],ePicType);
			pcPredSignal->copy(m_apcFrameTemp[8],ePicType);
      if( rcControlData.getBaseLayerSbb() )
  			rcControlData.getBaseLayerSbb()->copy(m_apcFrameTemp[10],ePicType);
      //RPIC bug fix }
    }
    // JVT-Q054 Red. Picture }


    // JVT-R057 LA-RDO{
    if(m_bLARDOEnable)
    {
      m_pcSliceEncoder->getMbEncoder()->setFrameEcEp(NULL);
      pcBLRecFrame->setChannelDistortion(NULL);
    }
    // JVT-R057 LA-RDO}

    m_uiNewlyCodedBits += uiBits;
    m_auiNumFramesCoded [ pcSliceHeader->getTemporalId() ] ++;
    if( ePicType==(m_bBotFieldFirst?TOP_FIELD:BOT_FIELD) ) // counted twice
    m_auiNumFramesCoded [ pcSliceHeader->getTemporalId() ] --;

    if( ! pcSliceHeader->getNoInterLayerPredFlag() )
    {
      m_pcSliceEncoder->updateBaseLayerResidual( rcControlData, m_uiFrameWidthInMb );
    }

    //===== deblock and store picture for prediction of following low-pass frames =====
    //ROF( pcSliceHeader->getNumRefIdxActive( LIST_0 ) == ( pcSliceHeader->isIntraSlice() ? 0 : 1 ) );
    ROF( pcSliceHeader->getNumRefIdxActive( LIST_1 ) == 0 );

    //----- store for inter-layer prediction (non-deblocked version) -----
    RNOK( m_pcSubband->copy( pcBLRecFrame, ePicType ) );

    // JVT-R057 LA-RDO{
    if(m_bLARDOEnable)
    {
      m_apcBaseFrame[uiFrame]->copyChannelDistortion( pcFrame );
      m_pcSubband->setChannelDistortion(pcFrame);
    }
    // JVT-R057 LA-RDO}

    //----- de-blocking -----
    RNOK( m_pcLoopFilter->process( *pcSliceHeader, pcBLRecFrame, pcResidual, pcMbDataCtrl, 0, rcControlData.getSpatialScalability() ) );

    //----- store for prediction of following low-pass pictures -----
    ROF( pcSliceHeader->getNalRefIdc() );
    RNOK( m_apcBaseFrame[uiFrame]->copy( pcBLRecFrame, ePicType ) );
    m_apcBaseFrame[uiFrame]->setPoc( *pcSliceHeader );
    m_apcBaseFrame[uiFrame]->copyPicParameters( *m_papcFrame[uiFrameIdInGOP] );

    RNOK( pcFrame->copy( pcBLRecFrame, ePicType ) );
  }

  RNOK( xUpdateLowPassRec( uiFrame ) );

  return Err::m_nOK;
}

ErrVal
LayerEncoder::xUpdateLowPassRec( UInt uiLowPassIndex )
{
  ROF  ( uiLowPassIndex <= 1 );
  ROFRS( m_bMGS && m_bSameResBL,  Err::m_nOK );
  ROFRS( m_uiEncodeKeyPictures,   Err::m_nOK );

  Frame* pcLPRec = m_pcH264AVCEncoder->getLowPassRec( m_uiDependencyId, uiLowPassIndex );
  ROF  ( pcLPRec );
  RNOK ( m_apcBaseFrame[uiLowPassIndex]->copy( pcLPRec, FRAME ) );
  RNOK ( xFillAndExtendFrame( m_apcBaseFrame[uiLowPassIndex], FRAME, m_pcSPS->getFrameMbsOnlyFlag() ) );
  return Err::m_nOK;
}


ErrVal
LayerEncoder::xEncodeNonKeyPicture( UInt               uiBaseLevel,
                                    UInt               uiFrame,
                                    AccessUnitData&    rcAccessUnitData,
                                    PicOutputDataList& rcPicOutputDataList,
                                    PicType            ePicType )
{
  UInt          uiFrameIdInGOP  = uiFrame << uiBaseLevel;
  UInt          uiBitsRes       = 0;

  Frame*     pcFrame         = m_papcFrame     [uiFrameIdInGOP];
  Frame*     pcOrgFrame      = m_papcFrame     [uiFrameIdInGOP];
  Frame*     pcResidual      = m_pcResidual;
  Frame*     pcPredSignal    = m_apcFrameTemp  [0];
  Frame*     pcBLRecFrame    = m_apcFrameTemp  [1];
  ControlData&  rcControlData   = m_pacControlData[uiFrameIdInGOP];

  const Bool bFieldCoded    =  m_pbFieldPicFlag[ uiFrameIdInGOP ] ;
  m_abCoded[uiFrameIdInGOP] = true;
  UInt uiBits          = 0;

  // JVT-W043 {
  if ( bRateControlEnable && !pcJSVMParams->m_uiLayerId )
  {
    pcGenericRC->m_pcJSVMParams->current_frame_number = m_uiGOPSize * m_uiGOPNumber + uiFrameIdInGOP;
    pcJSVMParams->CurrGopLevel = pcGenericRC->getCurrGopLevel( pcGenericRC->m_pcJSVMParams->current_frame_number );
    pcGenericRC->m_pcJSVMParams->current_mb_nr = 0;
    pcGenericRC->m_pcJSVMParams->type =
        ( uiFrame > 0 && (uiFrame % pcJSVMParams->m_uiIntraPeriod) != 0 ) ?
        ( ( pcGenericRC->m_pcJSVMParams->current_frame_number % m_uiGOPSize == 0 ) ? P_SLICE : B_SLICE ) : I_SLICE;
    pcGenericRC->m_pcJSVMParams->nal_reference_idc = (pcJSVMParams->CurrGopLevel == 0) ? 0 : 1;
    pcGenericRC->m_pcJSVMParams->number = pcGenericRC->m_pcJSVMParams->current_frame_number;

    pcQuadraticRC->rc_init_pict(1, 0, 1, 1.0F);
    pcGenericRC->m_pcJSVMParams->qp = pcQuadraticRC->updateQPRC2(0);
  }
  // JVT-W043 }
  SliceHeader*  pcSliceHeader          = rcControlData.getSliceHeader( ePicType );
  //JVT-W047
  m_bOutputFlag = pcSliceHeader->getOutputFlag();
  //JVT-W047
  ROF( pcSliceHeader );

  ExtBinDataAccessorList& rcOutputList    = rcAccessUnitData.getNalUnitList();
  ExtBinDataAccessorList& rcRedOutputList = rcAccessUnitData.getRedNalUnitList();

  RNOK( xInitControlDataHighPass( uiFrameIdInGOP,uiBaseLevel,uiFrame, ePicType  ) );

  //NonRequired JVT-Q066 (06-04-08){{
  if(m_uiDependencyId != 0 && m_uiNonRequiredWrite != 0)
  {
      if( pcSliceHeader->getNoInterLayerPredFlag() || m_uiDependencyId - pcSliceHeader->getRefLayerDependencyId() > 1 || pcSliceHeader->getRefLayerQualityId() != 3 )
      {
        rcAccessUnitData.CreatNonRequiredSei();
      }
      xSetNonRequiredSEI(pcSliceHeader, rcAccessUnitData.getNonRequiredSei());
      if(m_uiNonRequiredWrite == 2 && rcAccessUnitData.getNonRequiredSei() != NULL)
      {
        xWriteNonRequiredSEI(rcOutputList, rcAccessUnitData.getNonRequiredSei(), uiBits);
      }
  }
    //NonRequired JVT-Q066 (06-04-08)}}

  // JVT-V068 HRD {
    if ( m_bEnableHrd )
    {
      SEI::PicTiming *pcPicTimingSei = NULL;
      for (UInt uiLayer = 0; uiLayer < m_pcSPS->getVUI()->getNumLayers(); uiLayer++)
      {
        UInt uiTemporalLevel = m_pcSPS->getVUI()->getLayerInfo(uiLayer).getTemporalId();
        UInt uiDependencyId = m_pcSPS->getVUI()->getLayerInfo(uiLayer).getDependencyID();
        UInt uiQualityLevel = m_pcSPS->getVUI()->getLayerInfo(uiLayer).getQualityId();
        UInt uiIndex = (uiDependencyId<<7)+(uiTemporalLevel<<4)+uiQualityLevel;
        if (uiTemporalLevel >= m_pcSPS->getVUI()->getNumTemporalLevels() - uiBaseLevel - 1 )
        {
          RNOK( m_apcScheduler->get(uiIndex)->createTimingSei( pcPicTimingSei, m_pcSPS->getVUI(), 0, *pcSliceHeader, bFieldCoded, uiLayer ) );
          if ( uiTemporalLevel == m_pcSPS->getVUI()->getNumTemporalLevels() - 1 && uiDependencyId == 0 && uiQualityLevel == 0 )
          {
            // AVC compatible layer
            SEI::MessageList  cSEIMessageList;
            cSEIMessageList.push_back(pcPicTimingSei);
            RNOK( xWriteSEI(rcOutputList,cSEIMessageList,uiBits) );
          }
          else
          {
            RNOK( xWriteNestingSEIforHrd(rcOutputList, pcPicTimingSei, uiDependencyId, uiQualityLevel, uiTemporalLevel, uiBits ) );
          }
          RNOK( addParameterSetBits ( uiBits ) );
        }
      }
      ETRACE_LAYER(0);
    }
  //JVT-V068 HRD }

    //===== base layer encoding =====
    // JVT-Q054 Red. Picture {
    if ( pcSliceHeader->getPPS().getRedundantPicCntPresentFlag() )
    {
      pcSliceHeader->setRedundantPicCnt( 0 );	// set redundant_pic_cnt to 0 for primary coded picture
    }

    if( m_pcH264AVCEncoder->getCodingParameter()->getCGSSNRRefinement() )
    {
      m_pcH264AVCEncoder->m_aaadSeqBits[getLayerCGSSNR()][pcSliceHeader->getTemporalId()][getQualityLevelCGSSNR()] += uiBits;
    }
    else
    {
      m_pcH264AVCEncoder->m_aaadSeqBits[m_uiDependencyId][pcSliceHeader->getTemporalId()][0] += uiBits;
    }

    // JVT-Q054 Red. Picture }
    {
      RNOK( pcBLRecFrame->copy      ( pcFrame,   ePicType  ) );

      //JVT-R057 LA-RDO{
      //Bug_Fix JVT-R057 0806{
      if(m_bLARDOEnable)
      {
        pcBLRecFrame->setChannelDistortion(pcFrame);
        m_pcSliceEncoder->getMbEncoder()->setFrameEcEp(m_papcFrame[(uiFrame-1)<<uiBaseLevel]);
      }
      //Bug_Fix JVT-R057 0806}
      //JVT-R057 LA-RDO}
      RNOK( xEncodeHighPassSignal   ( rcOutputList,
                                      rcControlData,
                                      pcOrgFrame,
                                      pcBLRecFrame,
                                      pcResidual,
                                      pcPredSignal,
                                      uiBits,
                                      uiBitsRes, rcPicOutputDataList,
                                      ePicType ) );
    }
    // JVT-W043 {
    if ( bRateControlEnable && !pcJSVMParams->m_uiLayerId )
    {
      pcQuadraticRC->rc_update_pict_frame( uiBits );
      pcQuadraticRC->rc_update_pict( uiBits );
      if ( pcGenericRC->m_pcJSVMParams->type == P_SLICE && pcGenericRC->m_pcJSVMParams->frame_mbs_only_flag )
        pcQuadraticRC->updateRCModel();
      else if ( pcGenericRC->m_pcJSVMParams->type == P_SLICE && !(pcGenericRC->m_pcJSVMParams->frame_mbs_only_flag)
        && pcGenericRC->m_iNoGranularFieldRC == 0 )
        pcQuadraticRC->updateRCModel();
    }
    // JVT-W043 }
    //JVT-Q054 Red. Picture {
    //JVT-W049 {
    if ( (pcSliceHeader->getPPS().getRedundantPicCntPresentFlag())&& (!pcSliceHeader->getPPS().getRedundantKeyPicCntPresentFlag()))
    //if ( pcSliceHeader->getPPS().getRedundantPicCntPresentFlag() )
    //JVT-W049 }
    {
      // currently only slice repetition is supported for each primary coded slice
      UInt  uiRedundantPicNum = 1;  // number of redundant picture for each primary coded picture
      UInt  uiRedundantPicCnt = 0;
      for ( uiRedundantPicCnt = 1; uiRedundantPicCnt <= uiRedundantPicNum; uiRedundantPicCnt++)
      {
          pcSliceHeader->setRedundantPicCnt( uiRedundantPicCnt );

          RNOK( pcBLRecFrame->copy      ( pcFrame,      ePicType  ) );

          //JVT-R057 LA-RDO{
          if(m_bLARDOEnable)
          {
            pcBLRecFrame->setChannelDistortion(pcFrame);
            m_pcSliceEncoder->getMbEncoder()->setFrameEcEp( m_papcFrame[(uiFrame-1)<<uiBaseLevel] );
          }
          //JVT-R057 LA-RDO}
          RNOK( xEncodeHighPassSignal   ( rcRedOutputList,
                                          rcControlData,
                                          pcOrgFrame,
                                          pcBLRecFrame,
                                          pcResidual,
                                          pcPredSignal,
                                          uiBits,
                                          uiBitsRes, rcPicOutputDataList,
                                          ePicType  ) );

      }
      pcSliceHeader->setRedundantPicCnt( 0 );
    }
    // JVT-Q054 Red. Picture }

    m_uiNewlyCodedBits += uiBits;
    m_auiNumFramesCoded [ pcSliceHeader->getTemporalId() ] ++;
    if( ePicType==(m_bBotFieldFirst?TOP_FIELD:BOT_FIELD) ) // counted twice
    m_auiNumFramesCoded [ pcSliceHeader->getTemporalId() ] --;

    RNOK( pcFrame->copy( pcBLRecFrame, ePicType ) );

    RNOK( m_pcSubband->copy( pcFrame, ePicType ) );


    // JVT-R057 LA-RDO{
    if(m_bLARDOEnable)
    {
      m_pcSubband->setChannelDistortion(pcFrame);
    }
    // JVT-R057 LA-RDO}

  return Err::m_nOK;
}



ErrVal
LayerEncoder::xOutputPicData( PicOutputDataList& rcPicOutputDataList )
{
  while( rcPicOutputDataList.size() )
  {
    PicOutputData cPicOutputData = rcPicOutputDataList.popFront();
    if( cPicOutputData.YPSNR )
    {
      printf("%2s %5d: %2s   T%1d L%1d Q%-2d  QP%3d   Y%8.4lf  U%8.4lf  V%8.4lf  %8d bit\n",
        cPicOutputData.FirstPicInAU ? "AU" : "  ",
        cPicOutputData.Poc,
        cPicOutputData.FrameType,
        cPicOutputData.TemporalId,
        cPicOutputData.DependencyId,
        cPicOutputData.QualityId,
        cPicOutputData.Qp,
        cPicOutputData.YPSNR,
        cPicOutputData.UPSNR,
        cPicOutputData.VPSNR,
        cPicOutputData.Bits );
    }
    else
    {
      printf("%2s %5d: %2s   T%1d L%1d Q%-2d  QP%3d                                    %8d bit\n",
        cPicOutputData.FirstPicInAU ? "AU" : "  ",
        cPicOutputData.Poc,
        cPicOutputData.FrameType,
        cPicOutputData.TemporalId,
        cPicOutputData.DependencyId,
        cPicOutputData.QualityId,
        cPicOutputData.Qp,
        cPicOutputData.Bits );
    }
  }
  return Err::m_nOK;
}



ErrVal
LayerEncoder::xStoreReconstruction( PicBufferList& rcPicBufferOutputList )
{
  PicBufferList::iterator cOutputIter = rcPicBufferOutputList.begin();
  for( UInt uiIndex = (m_bFirstGOPCoded?1:0); uiIndex <= ( m_uiGOPSize >> m_uiNotCodedStages ); uiIndex++, cOutputIter++ )
  {
    PicBuffer*  pcPicBuffer = *cOutputIter;
    RNOK( m_papcFrame[uiIndex<<m_uiNotCodedStages]->store( pcPicBuffer ) );
  }
  return Err::m_nOK;
}




ErrVal
LayerEncoder::initGOP( AccessUnitData& rcAccessUnitData,
                       PicBufferList&  rcPicBufferInputList )
{
  ROT   ( m_bGOPInitialized );
  ROTRS ( rcPicBufferInputList.empty(), Err::m_nOK );
  RNOK  ( xInitGOP              ( rcPicBufferInputList ) );
  RNOK  ( xSetScalingFactors    () );
  RNOK  ( xClearELPics          () );
  ::memset( m_abCoded, 0x00, sizeof( m_abCoded ) );
  if( m_bFirstGOPCoded )
  {
    //==== copy picture zero =====
    PicOutputDataList cPicOutputDataList;
    Bool              bPicCoded = false;
    RNOK( xEncodeKeyPicture ( bPicCoded, 0, rcAccessUnitData, cPicOutputDataList ) );
    ROT ( bPicCoded );
  }
  m_bGOPInitialized = true;
  return Err::m_nOK;
}



//JVT-X046 {
ErrVal
LayerEncoder::xEncodeHighPassSignalSlices         ( ExtBinDataAccessorList&  rcOutExtBinDataAccessorList,
													  RefFrameList*    pcRefFrameList0,
													  RefFrameList*    pcRefFrameList1,
                            MbDataCtrl*     pcMbDataCtrlCol,
													  Frame*        pcOrigFrame,
													  Frame*        pcIntraRecFrame,
													  Frame*        pcMCFrame,
													  Frame*        pcResidual,
													  Frame*        pcPredSignal,
													  ControlData&     rcControlData,
													  UInt             uiNumMaxIter,
													  UInt             uiIterSearchRange,
													  UInt             uiFrameIdInGOP,
													  PicType          ePicType,
													  UInt&            ruiBits,
													  UInt&            ruiBitsRes,
													  PicOutputDataList&       rcPicOutputDataList,
													  Frame*		   pcFrame,
													  Frame*		   pcBLRecFrame,
														UInt           uiBaseLevel )
{
	Bool          bCalcMv               = false;
	Bool          bFaultTolerant        = false;
	MbEncoder*    pcMbEncoder           =  m_pcSliceEncoder->getMbEncoder         ();
	SliceHeader&  rcSliceHeader         = *rcControlData.getSliceHeader           ( ePicType );
	SliceHeader&  rcSH					        = rcSliceHeader;
	MbDataCtrl*   pcMbDataCtrl          =  rcControlData.getMbDataCtrl            ();
	Frame*        pcBaseLayerFrame      =  rcControlData.getBaseLayerRec          ();
	Frame*        pcBaseLayerResidual   =  rcControlData.getBaseLayerSbb          ();
	MbDataCtrl*   pcBaseLayerCtrl       =  rcControlData.getBaseLayerCtrl         ();
  UInt          uiMaxMvPerMb          = rcSliceHeader.getSPS().getMaxMVsPer2Mb () >> 1; // hard limit (don't take into account last macroblock)

  ExtBinDataAccessorList  cTmpExtBinDataAccessorList;
  ExtBinDataAccessorList  acExtBinDataAccessorList[16];
  PicOutputDataList       acPicOutputDataList     [16];

	RNOK( pcMbDataCtrl        ->initSlice( rcSH, PRE_PROCESS, false, NULL ) );
	RNOK( m_pcMotionEstimation->initSlice( rcSH              ) );

	RefFrameList* apcRefFrameList0[4] = { NULL, NULL, NULL, NULL };
	RefFrameList* apcRefFrameList1[4] = { NULL, NULL, NULL, NULL };
	const Bool    bMbAff   = rcSH.isMbaffFrame   ();


	UInt  uiBitsSEI   = 0;
	SliceHeader* pcSliceHeader = rcControlData.getSliceHeader( ePicType );
	ROF( pcSliceHeader );

	//----- Subsequence SEI -----
	if( m_bWriteSubSequenceSei &&   m_bH264AVCCompatible )
	{
		RNOK( xWriteSEI( rcOutExtBinDataAccessorList, *pcSliceHeader, uiBitsSEI ) );
	}


	if( bMbAff )
	{
    RefFrameList acRefFrameList0[2];
		RefFrameList acRefFrameList1[2];

		RNOK( gSetFrameFieldLists( acRefFrameList0[0], acRefFrameList0[1], *pcRefFrameList0 ) );
		RNOK( gSetFrameFieldLists( acRefFrameList1[0], acRefFrameList1[1], *pcRefFrameList1 ) );

		apcRefFrameList0[ TOP_FIELD ] = ( NULL == pcRefFrameList0 ) ? NULL : &acRefFrameList0[0];
		apcRefFrameList0[ BOT_FIELD ] = ( NULL == pcRefFrameList0 ) ? NULL : &acRefFrameList0[1];
		apcRefFrameList1[ TOP_FIELD ] = ( NULL == pcRefFrameList1 ) ? NULL : &acRefFrameList1[0];
		apcRefFrameList1[ BOT_FIELD ] = ( NULL == pcRefFrameList1 ) ? NULL : &acRefFrameList1[1];
		apcRefFrameList0[     FRAME ] = pcRefFrameList0;
		apcRefFrameList1[     FRAME ] = pcRefFrameList1;
	}
	else
	{
		RNOK( pcMCFrame->addFieldBuffer( ePicType ) );
		apcRefFrameList0[ ePicType ] = pcRefFrameList0;
		apcRefFrameList1[ ePicType ] = pcRefFrameList1;
	}

	if( ePicType!=FRAME )
  {
		if( pcOrigFrame )         RNOK( pcOrigFrame        ->addFieldBuffer( ePicType ) );
		if( pcIntraRecFrame )     RNOK( pcIntraRecFrame    ->addFieldBuffer( ePicType ) );
		if( pcBaseLayerFrame )    RNOK( pcBaseLayerFrame   ->addFieldBuffer( ePicType ) );
		if( pcBaseLayerResidual ) RNOK( pcBaseLayerResidual->addFieldBuffer( ePicType ) );
	}


	FMO* pcFMO = rcControlData.getSliceHeader()->getFMO();
	rcControlData.m_uiCurrentFirstMB=pcFMO->getFirstMacroblockInSlice(0);

	for (UInt iSliceGroupID=0;!pcFMO->SliceGroupCompletelyCoded(iSliceGroupID);iSliceGroupID++)
	{
		rcControlData.m_bSliceGroupAllCoded=false;
		rcControlData.m_uiCurrentFirstMB = pcFMO->getFirstMacroblockInSlice(iSliceGroupID);
		while(!rcControlData.m_bSliceGroupAllCoded)
		{
			if (!m_uiSliceMode)
			{
				rcControlData.m_bSliceGroupAllCoded = true;
			}
			UInt  muiBits      = 0;
			UInt  muiBitsRes   = 0;
			UInt  muiMbCoded   = 0;
			UInt  uiBits       = 0;

			rcSliceHeader.setFirstMbInSlice(rcControlData.m_uiCurrentFirstMB);
			rcSliceHeader.setLastMbInSlice(pcFMO->getLastMBInSliceGroup(iSliceGroupID));
			// JVT-S054 (2) (ADD)
			rcSliceHeader.setNumMbsInSlice(rcSliceHeader.getFMO()->getNumMbsInSlice(rcSliceHeader.getFirstMbInSlice(), rcSliceHeader.getLastMbInSlice()));
			UInt uiMbAddress       = rcSliceHeader.getFirstMbInSlice();
			UInt uiLastMbAddress   = rcSliceHeader.getLastMbInSlice();
			UInt uiNumMBInSlice;


			//prefix unit{{
			if( m_uiPreAndSuffixUnitEnable )
			{
				if ( pcSliceHeader->getNalUnitType() == NAL_UNIT_CODED_SLICE|| pcSliceHeader->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR )
				{
          RNOK( xWritePrefixUnit( acExtBinDataAccessorList[0], *pcSliceHeader, uiBits ) );
				}
			}
			//prefix unit}}

			//----- init NAL UNIT -----
			RNOK( xInitExtBinDataAccessor        (  m_cExtBinDataAccessor ) );
			RNOK( m_pcNalUnitEncoder->initNalUnit( &m_cExtBinDataAccessor ) );

			//---- write Slice Header -----
			ETRACE_NEWSLICE;
			xAssignSimplePriorityId( pcSliceHeader );
      if( pcSliceHeader->getSliceSkipFlag() )
      {
        UInt uiMaxMbsInSlice  = ( m_uiSliceMode == 1 ? m_uiSliceArgument : MSYS_UINT_MAX );
        UInt uiNumSkipSlices  = min( uiMaxMbsInSlice, pcSliceHeader->getNumMbsInSlice() );
        ROF( uiNumSkipSlices  > 0 );
        pcSliceHeader->setNumMbsInSliceMinus1( uiNumSkipSlices - 1 );
      }

			RNOK( m_pcNalUnitEncoder->write( *pcSliceHeader ) );

			m_pcSliceEncoder->m_pcMbCoder->m_uiSliceMode = m_uiSliceMode;
			m_pcSliceEncoder->m_pcMbCoder->m_uiSliceArgument = m_uiSliceArgument;
			if (m_pcSliceEncoder->m_pcMbCoder->m_uiSliceMode==2)
			{
				m_pcSliceEncoder->m_pcMbCoder->m_uiSliceArgument -=
					m_pcNalUnitEncoder->xGetBitsWriteBuffer()->getBitsWritten() + 4*8 +20;
			}

			//===== initialization =====
      RNOK( pcMbDataCtrl->initSlice( rcSliceHeader, ENCODE_PROCESS, false, pcMbDataCtrlCol ) );
			if( ! m_bLoadMotionInfo )
			{
				RNOK( m_pcMotionEstimation->initSlice( rcSliceHeader ) );
				RNOK( pcMbEncoder         ->initSlice( rcSliceHeader ) );
				RNOK( m_pcMotionEstimation->getSW()->initSlice( rcSliceHeader ) );
      }


			ROF( m_pcSliceEncoder->m_bInitDone );

			RNOK( m_pcSliceEncoder->m_pcControlMng->initSliceForCoding( *pcSliceHeader ) );

			//====== initialization ======
			YuvMbBuffer  cZeroBuffer;
			cZeroBuffer.setAllSamplesToZero();
			uiBits = m_pcSliceEncoder->m_pcMbCoder->getBitCount();

			m_pcSliceEncoder->m_pcMbCoder->bSliceCodedDone=false;
			//===== loop over macroblocks =====
			for(  uiNumMBInSlice = 0; uiMbAddress <= uiLastMbAddress;  )
			{
				ETRACE_NEWMB( uiMbAddress );
				bool bCoded;

				MbDataAccess* pcMbDataAccess      = 0;
				MbDataAccess* pcMbDataAccessBase  = 0;

				UInt          uiMbY, uiMbX;
				Double        dCost              = 0;

				rcSliceHeader.getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddress     );

				//===== init macroblock =====
				RNOK  ( pcMbDataCtrl            ->initMb( pcMbDataAccess,     uiMbY, uiMbX ) );
				if    ( pcBaseLayerCtrl )
				{
					RNOK( pcBaseLayerCtrl         ->initMb( pcMbDataAccessBase, uiMbY, uiMbX ) );
				}

				if( ! m_bLoadMotionInfo )
				{
					//===== initialisation =====
					RNOK( m_pcYuvFullPelBufferCtrl->initMb( uiMbY, uiMbX, false           ) );
					RNOK( m_pcYuvHalfPelBufferCtrl->initMb( uiMbY, uiMbX, false           ) );
					RNOK( m_pcMotionEstimation    ->initMb( uiMbY, uiMbX, *pcMbDataAccess ) );

          if( ! pcSliceHeader->getNoInterLayerPredFlag() )
          {
            pcMbEncoder->setBaseModeAllowedFlag( m_apabBaseModeFlagAllowedArrays[0][uiMbAddress] );
          }

          RNOK( pcMbEncoder->estimatePrediction ( *pcMbDataAccess,
						                                      pcMbDataAccessBase,
						                                      *pcRefFrameList0,
						                                      *pcRefFrameList1,
						                                      pcBaseLayerFrame    ? pcBaseLayerFrame   ->getPic( ePicType ) : NULL,
						                                      pcBaseLayerResidual ? pcBaseLayerResidual->getPic( ePicType ) : NULL,
						                                      *pcOrigFrame                              ->getPic( ePicType ),
						                                      *pcIntraRecFrame                          ->getPic( ePicType ),
                                                  uiMaxMvPerMb,
                                                  m_bMCBlks8x8Disable,
                                                  m_bBiPred8x8Disable,
						                                      uiNumMaxIter,
						                                      uiIterSearchRange,
						                                      rcControlData.getLambda(),
						                                      dCost,
						                                      true ) );

					const PicType eMbPicType = pcMbDataAccess->getMbPicType();
					pcMbDataAccess->setMbDataAccessBase(pcMbDataAccessBase);

					pcMCFrame->getFullPelYuvBuffer()->getYuvBufferCtrl().initMb(uiMbY,uiMbX,0);

          pcMbEncoder->setBaseLayerRec( pcBaseLayerFrame ? pcBaseLayerFrame->getPic( ePicType ) : NULL );

					RNOK( pcMbEncoder->compensatePrediction( *pcMbDataAccess,
						                                        pcMCFrame->getPic( eMbPicType ),
						                                        *apcRefFrameList0 [ eMbPicType ],
						                                        *apcRefFrameList1 [ eMbPicType ],
						                                        bCalcMv, bFaultTolerant ) );

					RNOK( pcOrigFrame->predictionSlices  ( pcOrigFrame,pcMCFrame,  uiMbY, uiMbX) );

					// store the new prediction frame that is obtained by invoking the special MC
					RNOK( pcResidual->copyMb(pcOrigFrame, uiMbY, uiMbX) );
					RNOK( pcBLRecFrame->copyMb(pcOrigFrame,uiMbY,uiMbX) );
					if( pcMbDataAccess->getMbData().isIntra() )
					{
						YuvMbBuffer cZeroMbBuffer;
						cZeroMbBuffer.setAllSamplesToZero();
						pcResidual->getPic( pcMbDataAccess->getMbPicType() )->getFullPelYuvBuffer()->loadBuffer( &cZeroMbBuffer );
					}

					RNOK( m_pcSliceEncoder->m_pcControlMng  ->initMbForCoding ( *pcMbDataAccess,    uiMbY, uiMbX, false, false ) );

					if (rcControlData.getBaseLayerCtrl())
					{
						RNOK( rcControlData.getBaseLayerCtrl()->initMb  ( pcMbDataAccessBase, uiMbY, uiMbX ) );
					}
					pcMbDataAccess->setMbDataAccessBase( pcMbDataAccessBase );


					if( pcMbDataAccess->getMbData().isIntra() )
					{
						dCost = 0;
            if( ! pcSliceHeader->getNoInterLayerPredFlag() )
            {
              m_pcSliceEncoder->m_pcMbEncoder->setBaseModeAllowedFlag( m_apabBaseModeFlagAllowedArrays[0][uiMbAddress] );
            }

            pcMbDataAccess->getMbData().setQp( pcMbDataAccess->getSH().getSliceQp() );

						RNOK( m_pcSliceEncoder->m_pcMbEncoder ->encodeIntra   ( *pcMbDataAccess,
							                                                      pcMbDataAccessBase,
							                                                      pcBLRecFrame                  ->getPic( ePicType ),
							                                                      pcBLRecFrame                  ->getPic( ePicType ),
							                                                      pcResidual               ->getPic( ePicType ),
							                                                      rcControlData.getBaseLayerRec() ? rcControlData.getBaseLayerRec()->getPic( ePicType ) : NULL,
							                                                      pcPredSignal             ->getPic( ePicType ),
							                                                      rcControlData.getLambda(),
							                                                      dCost ) );

						uiNumMBInSlice++;
						muiMbCoded++;
						if (m_uiSliceMode==1)
						{
							if (uiNumMBInSlice >= m_uiSliceArgument)
							{
								RNOK( m_pcSliceEncoder->m_pcMbCoder->encode( *pcMbDataAccess, pcMbDataAccessBase, true, true ) );
								rcControlData.m_uiCurrentFirstMB = rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress );
								rcSliceHeader.setNumMbsInSlice(uiNumMBInSlice);
								if (rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress)==-1)
									rcControlData.m_bSliceGroupAllCoded = true;
								break;
							}
							else
              {
								RNOK( m_pcSliceEncoder->m_pcMbCoder->encode( *pcMbDataAccess, pcMbDataAccessBase, (uiMbAddress == uiLastMbAddress ), true ) );
							}
						}
						else if (m_uiSliceMode==2)
						{
							RNOK( m_pcSliceEncoder->m_pcMbCoder->encode( *pcMbDataAccess, pcMbDataAccessBase, (uiMbAddress == uiLastMbAddress ), true ) );
							if ((rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress)==-1) && (!m_pcSliceEncoder->m_pcMbCoder->bSliceCodedDone))
								rcControlData.m_bSliceGroupAllCoded = true;
							if (m_pcSliceEncoder->m_pcMbCoder->bSliceCodedDone)
							{
								rcControlData.m_uiCurrentFirstMB = uiMbAddress;
								MbData* tempMbData;
								tempMbData=pcMbDataCtrl->xGetMbData(rcControlData.m_uiCurrentFirstMB);
								tempMbData->setSliceId( 0 );
								break;
							}
						}
						else
						{
							RNOK( m_pcSliceEncoder->m_pcMbCoder->encode( *pcMbDataAccess, pcMbDataAccessBase, (uiMbAddress == uiLastMbAddress ), true ) );
						}
					}
					else
					{
            pcMbDataAccess->getMbData().setQp( pcMbDataAccess->getSH().getSliceQp() );

						m_pcSliceEncoder->m_pcTransform->setClipMode( false );
						RNOK( m_pcSliceEncoder->m_pcMbEncoder ->encodeResidual  ( *pcMbDataAccess,
					                                                            pcFrame->getPic( ePicType ),
					                                                            pcBLRecFrame   ->getPic( ePicType ),
					                                                            pcResidual->getPic( ePicType ),
					                                                            rcControlData.getBaseLayerSbb() ? rcControlData.getBaseLayerSbb()->getPic( ePicType ) : NULL,
					                                                            bCoded,
					                                                            rcControlData.getLambda(),
					                                                            m_iMaxDeltaQp ) );

						m_pcSliceEncoder->m_pcTransform->setClipMode( true );

						uiNumMBInSlice++;

						if (m_uiSliceMode==1)
						{
							if (uiNumMBInSlice >= m_uiSliceArgument)
							{
								RNOK( m_pcSliceEncoder->m_pcMbCoder->encode( *pcMbDataAccess, pcMbDataAccessBase, true, true ) );
								rcControlData.m_uiCurrentFirstMB = rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress );
								rcSliceHeader.setNumMbsInSlice(uiNumMBInSlice);
								if (rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress)==-1)
									rcControlData.m_bSliceGroupAllCoded = true;
								break;
							}
							else
							{
								RNOK( m_pcSliceEncoder->m_pcMbCoder->encode( *pcMbDataAccess, pcMbDataAccessBase, (uiMbAddress == uiLastMbAddress ), true ) );
							}
						}
						else if (m_uiSliceMode==2)
						{
							RNOK( m_pcSliceEncoder->m_pcMbCoder->encode( *pcMbDataAccess, pcMbDataAccessBase, (uiMbAddress == uiLastMbAddress ), true ) );
							if ((rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress)==-1) && (!m_pcSliceEncoder->m_pcMbCoder->bSliceCodedDone))
								rcControlData.m_bSliceGroupAllCoded = true;
							if (m_pcSliceEncoder->m_pcMbCoder->bSliceCodedDone)
							{
								rcControlData.m_uiCurrentFirstMB = uiMbAddress;
								MbData* tempMbData;
								tempMbData=pcMbDataCtrl->xGetMbData(rcControlData.m_uiCurrentFirstMB);
								tempMbData->setSliceId( 0 );
								RNOK( pcOrigFrame->inversepredictionSlices  ( pcOrigFrame,pcMCFrame,  uiMbY, uiMbX) );
								break;
							}
						}
						else
						{
							RNOK( m_pcSliceEncoder->m_pcMbCoder->encode( *pcMbDataAccess, pcMbDataAccessBase, (uiMbAddress == uiLastMbAddress ), true ) );
						}
						if( bCoded )
						{
							muiMbCoded++;
						}

						RNOK( pcPredSignal->getFullPelYuvBuffer()->loadBuffer( &cZeroBuffer ) );
					}
				}
				else
				{
					//===== load prediction data =====
					RNOK( pcMbDataAccess->getMbData().loadAll( m_pMotionInfoFile ) );
				}

				uiMbAddress = rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress);

				if (uiMbAddress==-1)
				{
					rcControlData.m_bSliceGroupAllCoded=true;
					break;
				}
			}

			muiBits += m_pcSliceEncoder->m_pcMbCoder->getBitCount() - uiBits;

			//----- close NAL UNIT -----
			UInt auiBits[16];
      RNOK( m_pcNalUnitEncoder->closeAndAppendNalUnits( auiBits, cTmpExtBinDataAccessorList, &m_cExtBinDataAccessor, m_cBinData, NULL, 0, 0 ) );
      for( Int iMGSIdx = 0; cTmpExtBinDataAccessorList.size(); iMGSIdx++ )
      {
        acExtBinDataAccessorList[iMGSIdx].push_back( cTmpExtBinDataAccessorList.popFront() );
        auiBits[iMGSIdx] += 4*8; // start code
      }
      muiBits = auiBits[0];

			PicOutputData cPicOutputData;
      cPicOutputData.FirstPicInAU  = rcControlData.getSliceHeader()->getNoInterLayerPredFlag() && rcControlData.getSliceHeader()->getFirstMbInSlice() == 0;
      cPicOutputData.iPicType      = rcControlData.getSliceHeader()->getPicType();
			cPicOutputData.Poc           = rcControlData.getSliceHeader()->getPoc                  ();
			cPicOutputData.FrameType[0]  = rcControlData.getSliceHeader()->getSliceType            () == B_SLICE ? 'B' :
			                               rcControlData.getSliceHeader()->getSliceType            () == P_SLICE ? 'P' : 'I';
			cPicOutputData.FrameType[1]  = rcControlData.getSliceHeader()->getUseRefBasePicFlag()            ? 'K' : ' ';
			cPicOutputData.FrameType[2]  = '\0';
			cPicOutputData.DependencyId  = rcControlData.getSliceHeader()->getLayerCGSSNR          ();
			cPicOutputData.QualityId     = rcControlData.getSliceHeader()->getQualityLevelCGSSNR   ();
			cPicOutputData.TemporalId    = rcControlData.getSliceHeader()->getTemporalId        ();
			cPicOutputData.Qp            = rcControlData.getSliceHeader()->getSliceQp                ();
			cPicOutputData.Bits          = muiBits + uiBitsSEI;
			cPicOutputData.YPSNR         = 0.0;
			cPicOutputData.UPSNR         = 0.0;
			cPicOutputData.VPSNR         = 0.0;
      acPicOutputDataList[0].push_back( cPicOutputData );

      for( UInt uiFrag = 0; true; )
      {
        if( pcSliceHeader->getSPS().getMGSCoeffStop( uiFrag ) == 16 )
        {
          break;
        }
        uiFrag++;
        cPicOutputData.QualityId++;
        cPicOutputData.uiBaseLayerId      = pcSliceHeader->getLayerCGSSNR();
        cPicOutputData.uiBaseQualityLevel = cPicOutputData.QualityId - 1;
        cPicOutputData.Bits               = auiBits[uiFrag];
        acPicOutputDataList[uiFrag].push_back( cPicOutputData );
        if( m_pcH264AVCEncoder->getCodingParameter()->getCGSSNRRefinement() )
        {
          m_pcH264AVCEncoder->m_aaadSeqBits[getLayerCGSSNR()][pcSliceHeader->getTemporalId()][getQualityLevelCGSSNR()+uiFrag] += auiBits[uiFrag];
        }
        else
        {
          m_pcH264AVCEncoder->m_aaadSeqBits[m_uiDependencyId][pcSliceHeader->getTemporalId()][0] += auiBits[uiFrag];
        }
        ruiBits += auiBits[uiFrag];
      }

      if( m_pcH264AVCEncoder->getCodingParameter()->getCGSSNRRefinement() )
      {
        m_pcH264AVCEncoder->m_aaadSeqBits[getLayerCGSSNR()][pcSliceHeader->getTemporalId()][getQualityLevelCGSSNR()] += muiBits+uiBitsSEI;
      }
      else
      {
        m_pcH264AVCEncoder->m_aaadSeqBits[m_uiDependencyId][pcSliceHeader->getTemporalId()][0] += muiBits+uiBitsSEI;
      }

      ruiBits     += muiBits+uiBitsSEI;
			ruiBitsRes  += muiBitsRes;
			uiBitsSEI =0;
		}
	}

  for( UInt uiMGSIdx = 0; uiMGSIdx < 16; uiMGSIdx++ )
  {
    rcOutExtBinDataAccessorList += acExtBinDataAccessorList [uiMGSIdx];
    rcPicOutputDataList         += acPicOutputDataList      [uiMGSIdx];
    acExtBinDataAccessorList[uiMGSIdx].clear();
    acPicOutputDataList     [uiMGSIdx].clear();
  }

	if( ePicType!=FRAME )
	{
		if( pcOrigFrame )         RNOK( pcOrigFrame        ->removeFieldBuffer( ePicType ) );
		if( pcIntraRecFrame )     RNOK( pcIntraRecFrame    ->removeFieldBuffer( ePicType ) );
		if( pcBaseLayerFrame )    RNOK( pcBaseLayerFrame   ->removeFieldBuffer( ePicType ) );
		if( pcBaseLayerResidual ) RNOK( pcBaseLayerResidual->removeFieldBuffer( ePicType ) );
	}

  if( !pcSliceHeader->getNoInterLayerPredFlag() && pcSliceHeader->getSCoeffResidualPredFlag() )
  {
    m_pcSliceEncoder->updatePictureResTransform( rcControlData,m_uiFrameWidthInMb);
  }

  if( pcSliceHeader->getTCoeffLevelPredictionFlag() )
  {
    //===== update state prior to deblocking
    m_pcSliceEncoder->updatePictureAVCRewrite( rcControlData, m_uiFrameWidthInMb );
  }

	ETRACE_NEWFRAME;
	return Err::m_nOK;
}

ErrVal
LayerEncoder::xEncodeNonKeyPictureSlices( UInt                uiBaseLevel,
                                          UInt                uiFrame,
                                          AccessUnitData&     rcAccessUnitData,
                                          PicBufferList&			rcPicBufferInputList,
                                          PicOutputDataList&  rcPicOutputDataList,
                                          PicType             ePicType )
{
	UInt          uiFrameIdInGOP  = uiFrame << uiBaseLevel;
	UInt          uiBits          = 0;
	UInt          uiBitsRes       = 0;
	ControlData&  rcControlData   = m_pacControlData[uiFrameIdInGOP];
	Frame*     pcFrame         = m_papcFrame     [uiFrameIdInGOP];
	Frame*     pcIntraRecFrame = m_apcFrameTemp  [0];
  Frame*     pcResidual      = m_pcResidual;
	Frame*     pcMCFrame       = m_apcFrameTemp  [1];
  Frame*     pcOrgFrame      = m_papcFrame     [uiFrameIdInGOP];
	Frame*     pcPredSignal    = m_apcFrameTemp  [2];
	Frame*     pcBLRecFrame    = m_apcFrameTemp  [3];

	m_abCoded[uiFrameIdInGOP] = true;

	SliceHeader*  pcSliceHeader = rcControlData.getSliceHeader( ePicType );
	ROF( pcSliceHeader );

	ExtBinDataAccessorList& rcOutputList  = rcAccessUnitData.getNalUnitList ();

	RefFrameList& rcRefFrameList0 = rcControlData.getPrdFrameList( LIST_0 );
	RefFrameList& rcRefFrameList1 = rcControlData.getPrdFrameList( LIST_1 );
  UInt          uiFrameIdCol    = MSYS_UINT_MAX;

	//===== get reference frame lists =====
  RNOK( xGetPredictionLists     ( rcRefFrameList0, rcRefFrameList1, uiFrameIdCol, uiFrameIdInGOP, ePicType, RLU_MOTION_ESTIMATION, true ) );
  RefFrameList acRefFieldList0[2];
  RefFrameList acRefFieldList1[2];
  pcSliceHeader->setRefFrameList( &rcRefFrameList0, ePicType, LIST_0 );
  pcSliceHeader->setRefFrameList( &rcRefFrameList1, ePicType, LIST_1 );
  if( pcSliceHeader->isMbaffFrame() )
  {
    RNOK( gSetFrameFieldLists( acRefFieldList0[0], acRefFieldList0[1], rcRefFrameList0 ) );
    RNOK( gSetFrameFieldLists( acRefFieldList1[0], acRefFieldList1[1], rcRefFrameList1 ) );
    pcSliceHeader->setRefFrameList( &(acRefFieldList0[0]), TOP_FIELD, LIST_0 );
    pcSliceHeader->setRefFrameList( &(acRefFieldList0[1]), BOT_FIELD, LIST_0 );
    pcSliceHeader->setRefFrameList( &(acRefFieldList1[0]), TOP_FIELD, LIST_1 );
    pcSliceHeader->setRefFrameList( &(acRefFieldList1[1]), BOT_FIELD, LIST_1 );
  }

  //===== set lambda and QP =====
	RNOK( xInitControlDataMotion  ( uiBaseLevel, uiFrame, true, ePicType ) );

  {
    //----- set slice header QP (residual QP) ----
    Double  dQP = m_dBaseQPResidual - 6.0 * log10( rcControlData.getScalingFactor() ) / log10( 2.0 );
    if( m_bExplicitQPCascading )
    {
      dQP = m_dBaseQPResidual + m_adDeltaQPTLevel[ m_uiDecompositionStages - uiBaseLevel ];
    }
    Int iQP = max( MIN_QP, min( MAX_QP, (Int)floor( dQP + 0.5 ) ) );
    pcSliceHeader->setSliceHeaderQp( iQP );
  }

	//NonRequired JVT-Q066 (06-04-08){{
	if(m_uiDependencyId != 0 && m_uiNonRequiredWrite != 0)
	{
		if( pcSliceHeader->getNoInterLayerPredFlag() || m_uiDependencyId - pcSliceHeader->getRefLayerDependencyId() > 1 || pcSliceHeader->getRefLayerQualityId() != 3 )
		{
			rcAccessUnitData.CreatNonRequiredSei();
		}
		xSetNonRequiredSEI(pcSliceHeader, rcAccessUnitData.getNonRequiredSei());
		if(m_uiNonRequiredWrite == 2 && rcAccessUnitData.getNonRequiredSei() != NULL)
		{
			xWriteNonRequiredSEI(rcOutputList, rcAccessUnitData.getNonRequiredSei(), uiBits);
		}
	}
	//NonRequired JVT-Q066 (06-04-08)}}

	//===== base layer encoding =====
	// JVT-Q054 Red. Picture {
	if ( pcSliceHeader->getPPS().getRedundantPicCntPresentFlag() )
	{
		pcSliceHeader->setRedundantPicCnt( 0 );	// set redundant_pic_cnt to 0 for primary coded picture
	}
	// JVT-Q054 Red. Picture }

	//S051{
	m_pcSliceEncoder->setUseBDir(true);
	if(m_bEncSIP)
	{
		if(m_bH264AVCCompatible||!rcControlData.getSliceHeader()->getDirectSpatialMvPredFlag())
		{
      if( uiFrameIdCol != MSYS_UINT_MAX)
      {
        SliceHeader* pcSliceHeaderL1 = m_pacControlData[uiFrameIdCol].getSliceHeader( ePicType );
        if( xSIPCheck( pcSliceHeaderL1->getPoc() ) )
        {
          m_pcSliceEncoder->setUseBDir(false);
        }
      }
		}
	}
	//S051}

	//TMM_WP
	/* el & has correspongding bl frame & bl uses wp then use bl wts */
	if((m_uiDependencyId > 0 && m_bBaseLayerWp &&
		rcControlData.getBaseLayerIdMotion() != MSYS_UINT_MAX) )
	{
		/* use same wts as bl */
		pcSliceHeader->setBasePredWeightTableFlag(true);
	}
	else
	{
		/* call function to calculate the weights */
		m_pcSliceEncoder->xSetPredWeights( *pcSliceHeader,
			pcFrame,
			rcRefFrameList0,
			rcRefFrameList1
			//rcRefFrameList0.getEntry(0),
			//rcRefFrameList1.getEntry(0),
			);
		pcSliceHeader->setBasePredWeightTableFlag(false);
	}
	//TMM_WP
	// JVT-R057 LA-RDO{
	if(m_bLARDOEnable)
	{
		pcFrame->initChannelDistortion();
		m_pcSliceEncoder->getMbEncoder()->setFrameEcEp(m_papcFrame[(uiFrame-1)<<uiBaseLevel]);
	}
	// JVT-R057 LA-RDO}

	//JVT-R057 LA-RDO{
	//Bug_Fix JVT-R057 0806{
	if(m_bLARDOEnable)
	{
		pcBLRecFrame->setChannelDistortion(pcFrame);
		m_pcSliceEncoder->getMbEncoder()->setFrameEcEp(m_papcFrame[(uiFrame-1)<<uiBaseLevel]);
	}
	//Bug_Fix JVT-R057 0806}
	//JVT-R057 LA-RDO}

  MbDataCtrl* pcMbDataCtrlCol = ( uiFrameIdCol == MSYS_UINT_MAX ? 0 : m_pacControlData[uiFrameIdCol].getMbDataCtrl() );
  RNOK(xEncodeHighPassSignalSlices(rcOutputList,&rcRefFrameList0,&rcRefFrameList1,pcMbDataCtrlCol,pcFrame,
    pcIntraRecFrame,pcMCFrame,pcResidual,pcPredSignal,rcControlData,
    m_uiNumMaxIter,m_uiIterSearchRange,uiFrameIdInGOP,ePicType,
    uiBits,uiBitsRes,rcPicOutputDataList,pcOrgFrame,pcBLRecFrame,uiBaseLevel) );

	m_uiNewlyCodedBits += uiBits;
	m_auiNumFramesCoded [ pcSliceHeader->getTemporalId() ] ++;
  if( ePicType==(m_bBotFieldFirst?TOP_FIELD:BOT_FIELD) ) // counted twice
  m_auiNumFramesCoded [ pcSliceHeader->getTemporalId() ] --;

	RNOK( pcFrame->copy( pcBLRecFrame, ePicType ) );
  RNOK( m_pcSubband->copy( pcFrame, ePicType ) );
  // JVT-R057 LA-RDO{
  if(m_bLARDOEnable)
    m_pcSubband->setChannelDistortion(pcFrame);
  // JVT-R057 LA-RDO}

	return Err::m_nOK;
}
//JVT-X046 }
ErrVal
LayerEncoder::process( UInt             uiAUIndex,
                       AccessUnitData&  rcAccessUnitData,
                       PicBufferList&   rcPicBufferInputList,
                       PicBufferList&   rcPicBufferOutputList,
                       PicBufferList&   rcPicBufferUnusedList,
                       ParameterSetMng* pcParameterSetMng )
{
  ROFRS ( m_bGOPInitialized, Err::m_nOK );
  ROF   ( m_uiNumFramesLeftInGOP );
// JVT-V068 HRD {
  m_pcParameterSetMng = pcParameterSetMng;
// JVT-V068 HRD }

  //===== init some parameters =====
  g_nLayer        = m_uiQualityLevelCGSSNR;
  ETRACE_LAYER(     g_nLayer );
  if( m_bLARDOEnable )
  {
    m_pcSliceEncoder->getMbEncoder()->setLARDOEnable( m_bLARDOEnable );
    m_pcSliceEncoder->getMbEncoder()->setLayerID    ( m_uiDependencyId    );
  }
  m_pcSliceEncoder->getMbEncoder()->setIPCMRate( m_pcLayerParameters->getIPCMRate() );
  // JVT-W043 {
  if ( bRateControlEnable )
  {
    pcJSVMParams->m_uiLayerId = m_uiDependencyId;
  }
  // JVT-W043 }
  RNOK( xInitBitCounts() );

  //===== update higher layer pictures =====
  RNOK( xUpdateELPics() );

	//JVT-X046 {
  m_pcSliceEncoder->m_uiSliceMode = m_uiSliceMode;
  m_pcSliceEncoder->m_uiSliceArgument = m_uiSliceArgument;
  //JVT-X046 }

  UInt   uiCodingIndex   = ( m_bFirstGOPCoded ? uiAUIndex + 1 : uiAUIndex );
  UInt   uiFrameIdInGOP  = m_auiCodingIndexToFrameId[ uiCodingIndex ];
  ROTRS( uiFrameIdInGOP  > m_uiGOPSize, Err::m_nOK );
  UInt   uiSliceMode     = m_uiSliceMode;
  UInt   uiSliceArgument = m_uiSliceArgument;
  if(    uiSliceMode != 2 )
  {
    UInt uiMaxSliceSize  = m_pcH264AVCEncoder->getMaxSliceSize( m_uiDependencyId, uiAUIndex );
    if(  uiMaxSliceSize  < m_uiSliceArgument )
    {
      m_uiSliceMode      = 1;
      m_uiSliceArgument  = uiMaxSliceSize;
      m_pcSliceEncoder->m_uiSliceMode     = m_uiSliceMode;
      m_pcSliceEncoder->m_uiSliceArgument = m_uiSliceArgument;
    }
  }
  Bool   bKeyPicture     = ( ( uiFrameIdInGOP % ( 1U << m_uiDecompositionStages ) ) == 0 );
  if(    bKeyPicture )
  {
    //===== KEY PICTURE =====
    Bool              bPicCoded   = false;
    UInt              uiKeyFrmIdx = ( uiFrameIdInGOP ? 1 : 0 );
    PicOutputDataList cPicOutputDataList;
    RNOK( xEncodeKeyPicture     ( bPicCoded,               uiKeyFrmIdx, rcAccessUnitData,      cPicOutputDataList ) );
    ROF ( bPicCoded );
    RNOK( xCalculateAndAddPSNR  ( m_uiDecompositionStages, uiKeyFrmIdx, rcPicBufferInputList,  cPicOutputDataList ) );
    RNOK( xCalculateTiming      ( cPicOutputDataList,      uiKeyFrmIdx ) );
    RNOK( xOutputPicData        (                                                              cPicOutputDataList ) );
    if  ( uiKeyFrmIdx == 1 )
    {
      RNOK( m_pcAnchorFrameReconstructed->copyAll( m_papcFrame[ m_uiGOPSize ] ) );
    }
    m_uiLastCodedFrameIdInGOP = uiKeyFrmIdx << m_uiDecompositionStages;
    m_uiLastCodedTemporalId   = 0;
  }
  else
  {
    //===== NON-KEY PICTURE =====
    PicOutputDataList cPicOutputDataList;
    UInt    uiLevel         = m_uiDecompositionStages - m_auiFrameIdToTemporalId[ uiFrameIdInGOP ];
    UInt    uiFrame         = uiFrameIdInGOP >> uiLevel;
    Bool    bFieldCoded     = m_pbFieldPicFlag[ uiFrameIdInGOP ];
    UInt    uiNumPics       = ( bFieldCoded ? 2 : 1 );
    PicType eFirstPicType   = ( bFieldCoded ? ( m_bBotFieldFirst ? BOT_FIELD : TOP_FIELD ) : FRAME );
    PicType eLastPicType    = ( bFieldCoded ? ( m_bBotFieldFirst ? TOP_FIELD : BOT_FIELD ) : FRAME );
    for( UInt uiPicType = 0; uiPicType < uiNumPics; uiPicType++ )
    {
      PicType ePicType      = ( uiPicType ? eLastPicType : eFirstPicType );
      RNOK( xSetBaseLayerData           ( uiFrameIdInGOP, ePicType ) );
      if ( m_uiSliceMode )
      {
        RNOK( xEncodeNonKeyPictureSlices( uiLevel, uiFrame, rcAccessUnitData, rcPicBufferInputList, cPicOutputDataList, ePicType ) );
        RNOK( xCompositionFrame         ( uiLevel, uiFrame, rcPicBufferInputList,                     ePicType ) );
      }
      else
      {
        RNOK( xMotionEstimationFrame    ( uiLevel, uiFrame,                                           ePicType ) );
        RNOK( xDecompositionFrame       ( uiLevel, uiFrame,                                           ePicType ) );
        RNOK( xEncodeNonKeyPicture      ( uiLevel, uiFrame, rcAccessUnitData,     cPicOutputDataList, ePicType ) );
        RNOK( xCompositionFrame         ( uiLevel, uiFrame, rcPicBufferInputList,                     ePicType ) );
      }
    }

    RNOK( xCalculateAndAddPSNR          ( uiLevel, uiFrame, rcPicBufferInputList, cPicOutputDataList ) );
    RNOK( xCalculateTiming              ( cPicOutputDataList, uiFrame ) );
    RNOK( xOutputPicData                ( cPicOutputDataList ) );
    RNOK( xClearBufferExtensions        () );
    m_uiLastCodedFrameIdInGOP = uiFrameIdInGOP;
    m_uiLastCodedTemporalId   = m_uiDecompositionStages - uiLevel;
  }
  m_uiSliceMode     = uiSliceMode;
  m_uiSliceArgument = uiSliceArgument;

  //===== finish GOP =====
  if( --m_uiNumFramesLeftInGOP == 0 )
  {
    m_bGOPInitialized = false;
    RNOK( xStoreReconstruction( rcPicBufferOutputList ) );
    RNOK( xFinishGOP          ( rcPicBufferInputList,
                                rcPicBufferOutputList,
                                rcPicBufferUnusedList ) );
  }

  return Err::m_nOK;
}



ErrVal
LayerEncoder::xFinishGOP( PicBufferList& rcPicBufferInputList,
                         PicBufferList& rcPicBufferOutputList,
                         PicBufferList& rcPicBufferUnusedList )
{
  UInt  uiLowPassSize = m_uiGOPSize >> m_uiNotCodedStages;

  while( rcPicBufferOutputList.size() > uiLowPassSize + ( m_bFirstGOPCoded ? 0 : 1 ) )
  {
    PicBuffer*  pcPicBuffer = rcPicBufferOutputList.popBack();
    rcPicBufferUnusedList.push_back( pcPicBuffer );
  }

  //===== update parameters =====
  m_uiParameterSetBits  = 0;
  m_bFirstGOPCoded      = true;
  m_uiGOPNumber        ++;

  return Err::m_nOK;
}



ErrVal
LayerEncoder::xCalculateAndAddPSNR( UInt               uiStage,
                                   UInt               uiFrame,
                                   PicBufferList&     rcPicBufferInputList,
                                   PicOutputDataList& rcPicOutputDataList )
{
  //===== initialize buffer control =====
  RNOK( m_pcYuvFullPelBufferCtrl->initMb() );

  const YuvBufferCtrl::YuvBufferParameter&  cBufferParam  = m_pcYuvFullPelBufferCtrl->getBufferParameter( FRAME );
  PicBufferList::iterator                   cIter           = rcPicBufferInputList.begin();
  UInt                                      uiFrameIdInGOP  = uiFrame << uiStage;
  PicBuffer*                                pcPicBuffer     = 0;
  Frame*                                    pcFrame         = m_papcFrame[uiFrameIdInGOP];
  PicType                                   ePicType        = ( m_pbFieldPicFlag[ uiFrameIdInGOP ] ? ( m_bBotFieldFirst ? TOP_FIELD : BOT_FIELD ) : FRAME );
  Int                                       iPoc            = m_pacControlData[ uiFrameIdInGOP ].getSliceHeader(ePicType)->getPoc();
  Double                                    dYPSNR          = 0.0;
  Double                                    dUPSNR          = 0.0;
  Double                                    dVPSNR          = 0.0;

  //===== get correct picture buffer =====
  for( UInt uiIndex = uiFrameIdInGOP - ( m_bFirstGOPCoded ? 1 : 0 ); uiIndex; uiIndex--, cIter++ )
  {
    ROT( cIter == rcPicBufferInputList.end() );
  }
  pcPicBuffer = *cIter;
  ROF  ( pcPicBuffer );

  //===== calculate PSNR =====
  {
    Pel*  pPelOrig  = pcPicBuffer->getBuffer() + cBufferParam.getMbLum();
    XPel* pPelRec   = pcFrame->getFullPelYuvBuffer()->getMbLumAddr();
    Int   iStride   = cBufferParam.getStride();
    Int   iWidth    = cBufferParam.getWidth ();
    Int   iHeight   = cBufferParam.getHeight();
    UInt  uiSSDY    = 0;
    UInt  uiSSDU    = 0;
    UInt  uiSSDV    = 0;
    Int   x, y;


    for( y = 0; y < iHeight; y++ )
    {
      for( x = 0; x < iWidth; x++ )
      {
        Int iDiff = (Int)pPelOrig[x] - (Int)pPelRec[x];
        uiSSDY   += iDiff * iDiff;
      }
      pPelOrig += iStride;
      pPelRec  += iStride;
    }

    iHeight >>= 1;
    iWidth  >>= 1;
    iStride >>= 1;
    pPelOrig  = pcPicBuffer->getBuffer() + cBufferParam.getMbCb();
    pPelRec   = pcFrame->getFullPelYuvBuffer()->getMbCbAddr();

    for( y = 0; y < iHeight; y++ )
    {
      for( x = 0; x < iWidth; x++ )
      {
        Int iDiff = (Int)pPelOrig[x] - (Int)pPelRec[x];
        uiSSDU   += iDiff * iDiff;
      }
      pPelOrig += iStride;
      pPelRec  += iStride;
    }

    pPelOrig  = pcPicBuffer->getBuffer() + cBufferParam.getMbCr();
    pPelRec   = pcFrame->getFullPelYuvBuffer()->getMbCrAddr();

    for( y = 0; y < iHeight; y++ )
    {
      for( x = 0; x < iWidth; x++ )
      {
        Int iDiff = (Int)pPelOrig[x] - (Int)pPelRec[x];
        uiSSDV   += iDiff * iDiff;
      }
      pPelOrig += iStride;
      pPelRec  += iStride;
    }


    UInt uiMbNumber = cBufferParam.getWidth () * cBufferParam.getHeight() / 16 / 16 ;
    Double fRefValueY = 255.0 * 255.0 * 16.0 * 16.0 * (Double)uiMbNumber;
    Double fRefValueC = fRefValueY / 4.0;
    dYPSNR            = ( uiSSDY ? 10.0 * log10( fRefValueY / (Double)uiSSDY ) : 99.99 );
    dUPSNR            = ( uiSSDU ? 10.0 * log10( fRefValueC / (Double)uiSSDU ) : 99.99 );
    dVPSNR            = ( uiSSDV ? 10.0 * log10( fRefValueC / (Double)uiSSDV ) : 99.99 );
  }

  //===== add PSNR =====
  for( UInt uiLevel = m_uiDecompositionStages - uiStage; uiLevel <= MAX_DSTAGES; uiLevel++ )
  {
    m_adPSNRSumY[ uiLevel ] += dYPSNR;
    m_adPSNRSumU[ uiLevel ] += dUPSNR;
    m_adPSNRSumV[ uiLevel ] += dVPSNR;
  }

  //===== output PSNR =====
  PicOutputData& rcPicOutputData = rcPicOutputDataList.back();
// TMM
  ROF( rcPicOutputData.Poc == iPoc );
  rcPicOutputData.YPSNR = dYPSNR;
  rcPicOutputData.UPSNR = dUPSNR;
  rcPicOutputData.VPSNR = dVPSNR;

  return Err::m_nOK;
}


ErrVal
LayerEncoder::finish( UInt&    ruiNumCodedFrames,
                     Double&  rdOutputRate,
                     Double   aaadOutputFramerate[MAX_LAYERS][MAX_TEMP_LEVELS][MAX_QUALITY_LEVELS],
                     Double   aaauidSeqBits[MAX_LAYERS][MAX_TEMP_LEVELS][MAX_QUALITY_LEVELS] )
{
  ROFRS( m_auiNumFramesCoded[0], Err::m_nOK );

  UInt  uiStage;
  UInt  uiMaxStage = m_uiDecompositionStages - m_uiNotCodedStages;
  // Bug fix: yiliang.bao@nokia.com
  // uiMaxStage is unsigned, it has a problem when uiMaxStage == 0,
  // uiMaxStage - 1 will result in a large number
  UInt  uiMinStage        = 0; //bugfix replace

  Char  acResolution[10];
//bugfix delete

  sprintf( acResolution, "%dx%d", 16*m_uiFrameWidthInMb, 16*m_uiFrameHeightInMb );

  //===== set final sum of bits and average PSNR's =====
  for( uiStage = 0; uiStage <= MAX_DSTAGES; uiStage++ )
  {
    if( uiStage  )
    {
      m_auiNumFramesCoded   [uiStage] += m_auiNumFramesCoded        [uiStage-1];
    }
    if( m_auiNumFramesCoded [uiStage] )
    {
      m_adPSNRSumY          [uiStage] /= (Double)m_auiNumFramesCoded[uiStage];
      m_adPSNRSumU          [uiStage] /= (Double)m_auiNumFramesCoded[uiStage];
      m_adPSNRSumV          [uiStage] /= (Double)m_auiNumFramesCoded[uiStage];
    }
  }

  static Double aaadCurrBits[MAX_LAYERS][MAX_TEMP_LEVELS][MAX_QUALITY_LEVELS];

  if( m_pcH264AVCEncoder->getCodingParameter()->getCGSSNRRefinement() == 1 )
  {
    UInt uiLayerIdCGSSNR = getLayerCGSSNR();
    UInt uiBaseLayerIdCGSSNR = MSYS_UINT_MAX;
    UInt uiBaseQualityLevelCGSSNR = 0;
    if( uiLayerIdCGSSNR ) //may have dependent layer
    for( Int iLayer = (Int)m_uiDependencyId; iLayer >= 0; iLayer-- )
    {
      UInt uiLayer = iLayer;
      if( m_pcH264AVCEncoder->getCodingParameter()->getLayerParameters( uiLayer ).getLayerCGSSNR() == uiLayerIdCGSSNR
        && m_pcH264AVCEncoder->getCodingParameter()->getLayerParameters( uiLayer ).getQualityLevelCGSSNR() == 0 )
      {
        uiBaseLayerIdCGSSNR = m_pcH264AVCEncoder->getCodingParameter()->getLayerParameters( uiLayer ).getBaseLayerCGSSNR();
        uiBaseQualityLevelCGSSNR = m_pcH264AVCEncoder->getCodingParameter()->getLayerParameters( uiLayer ).getBaseQualityLevelCGSSNR();
        break;
      }
    }
    for( UInt uiLevel = 0; uiLevel < MAX_TEMP_LEVELS; uiLevel++ )
    {
      Double dBits = 0;
      if( uiLevel == 0 ) //TL = 0
      {
        for( UInt uiQualityLevel = 0; uiQualityLevel < MAX_QUALITY_LEVELS; uiQualityLevel++ )
        {
          if( uiBaseLayerIdCGSSNR < MAX_LAYERS ) //not base layer
          {
            if( uiQualityLevel ) // D > 0, T = 0, Q != 0
              dBits = aaadCurrBits[uiLayerIdCGSSNR][uiLevel][uiQualityLevel-1];
            else // D > 0, T = 0, Q = 0
            {
              ROT( m_uiBaseQualityLevel >= MAX_QUALITY_LEVELS );
              dBits = aaadCurrBits[uiBaseLayerIdCGSSNR][uiLevel][uiBaseQualityLevelCGSSNR];
            }
          }
          dBits += aaauidSeqBits[uiLayerIdCGSSNR][uiLevel][uiQualityLevel];
          aaadCurrBits[uiLayerIdCGSSNR][uiLevel][uiQualityLevel] = dBits;
        }
      }
      else //TL != 0
      {
        for( UInt uiQualityLevel = 0; uiQualityLevel < MAX_QUALITY_LEVELS; uiQualityLevel++ )
        {
          dBits = 0;
          if( uiBaseLayerIdCGSSNR < MAX_LAYERS ) //not base layer
          {
            ROT( uiBaseQualityLevelCGSSNR >= MAX_QUALITY_LEVELS );
            dBits = aaadCurrBits[uiBaseLayerIdCGSSNR][uiLevel][uiBaseQualityLevelCGSSNR];
          }
          for( UInt uiTIndex = 0; uiTIndex <= uiLevel; uiTIndex++ )
          {
            for( UInt uiFIndex = 0; uiFIndex <= uiQualityLevel; uiFIndex++ )
            {
              dBits += aaauidSeqBits[uiLayerIdCGSSNR][uiTIndex][uiFIndex];
            }
          }
          aaadCurrBits[uiLayerIdCGSSNR][uiLevel][uiQualityLevel] = dBits;
        }
      }
    }
    if( getLayerCGSSNR() == 0 )
    {
      printf( " \n\n\nSUMMARY:\n" );
      printf( "                      " "  bitrate " "   Min-bitr" "   Y-PSNR" "   U-PSNR" "   V-PSNR\n" );
      printf( "                      " " ---------" " ----------" " --------" " --------" " --------\n" );
    }
  }
  else
  {
  for( UInt uiLevel = 0; uiLevel < MAX_TEMP_LEVELS; uiLevel++ )
  {
    Double dBits = 0;
    if( uiLevel == 0 ) //TL = 0
    {
      for( UInt uiFGS = 0; uiFGS < MAX_QUALITY_LEVELS; uiFGS++ )
      {
        if( m_uiBaseLayerId < MAX_LAYERS ) //not base layer
        {
// BUG_FIX liuhui{
          if( uiFGS ) // D > 0, T = 0, Q != 0
            dBits = aaadCurrBits[m_uiDependencyId][uiLevel][uiFGS-1];
          else // D > 0, T = 0, Q = 0
          {
            ROT( m_uiBaseQualityLevel >= MAX_QUALITY_LEVELS );
            dBits = aaadCurrBits[m_uiBaseLayerId][uiLevel][m_uiBaseQualityLevel];
//bugfix delete
          }
// BUG_FIX liuhui}
        }
//bugfix delete
        dBits += aaauidSeqBits[m_uiDependencyId][uiLevel][uiFGS];
        aaadCurrBits[m_uiDependencyId][uiLevel][uiFGS] = dBits;
      }
    }
    else //TL != 0
    {
      for( UInt uiFGS = 0; uiFGS < MAX_QUALITY_LEVELS; uiFGS++ )
      {
        dBits = 0;
        if( m_uiBaseLayerId < MAX_LAYERS ) //not base layer
        {
          ROT( m_uiBaseQualityLevel >= MAX_QUALITY_LEVELS );
// BUG_FIX liuhui{
          dBits = aaadCurrBits[m_uiBaseLayerId][uiLevel][m_uiBaseQualityLevel];
//bugfix delete
// BUG_FIX liuhui}
        }
        for( UInt uiTIndex = 0; uiTIndex <= uiLevel; uiTIndex++ )
        {
          for( UInt uiFIndex = 0; uiFIndex <= uiFGS; uiFIndex++ )
          {
            dBits += aaauidSeqBits[m_uiDependencyId][uiTIndex][uiFIndex];
          }
        }
        aaadCurrBits[m_uiDependencyId][uiLevel][uiFGS] = dBits;
      }
    }
  }

// BUG_FIX liuhui{
  if( m_uiDependencyId == 0 )
  {
    printf( " \n\n\nSUMMARY:\n" );
    printf( "                      " "  bitrate " "   Min-bitr" "   Y-PSNR" "   U-PSNR" "   V-PSNR\n" );
    printf( "                      " " ---------" " ----------" " --------" " --------" " --------\n" );
  }
// BUG_FIX liuhui}
  }

  for( uiStage = uiMinStage; uiStage <= uiMaxStage; uiStage++ )
  {
    Double  dFps    = m_fOutputFrameRate / (Double)( 1 << ( uiMaxStage - uiStage ) );
    Double  dScale  = dFps / (Double)m_auiNumFramesCoded[uiStage];

    Double dBitrate = aaadCurrBits[m_uiDependencyId][uiStage][0] * dScale;

    if( m_pcH264AVCEncoder->getCodingParameter()->getCGSSNRRefinement() == 1 )
    {
      UInt uiStart = 0;
      UInt uiStop  = getQualityLevelCGSSNR() + m_pcH264AVCEncoder->getCodingParameter()->getLayerParameters( m_uiDependencyId).getNumberOfQualityLevelsCGSSNR();
      for( UInt uiQLL = uiStart; uiQLL < uiStop; uiQLL++ )
      {
        aaadOutputFramerate[getLayerCGSSNR()][uiStage][uiQLL] = dFps;
      }
      dBitrate = aaadCurrBits[getLayerCGSSNR()][uiStage][uiStop-1] * dScale; // using highest quality layer as output
      for( UInt uiQualityLevel = 0; uiQualityLevel < MAX_QUALITY_LEVELS; uiQualityLevel ++ )
        m_pcH264AVCEncoder->setBitrateRep( getLayerCGSSNR(), uiStage, uiQualityLevel, aaadCurrBits[getLayerCGSSNR()][uiStage][uiQualityLevel]*dScale );
    }
    else
    {
      aaadOutputFramerate[m_uiDependencyId][uiStage][0] = dFps;
      for( UInt uiQualityLevel = 0; uiQualityLevel < MAX_QUALITY_LEVELS; uiQualityLevel ++ )
      {
        m_pcH264AVCEncoder->setBitrateRep( m_uiDependencyId, uiStage, uiQualityLevel, aaadCurrBits[m_uiDependencyId][uiStage][uiQualityLevel]*dScale );
      }
    }

    static Double adMinBitrate[MAX_LAYERS][MAX_TEMP_LEVELS];
    if( m_pcH264AVCEncoder->getCodingParameter()->getCGSSNRRefinement() == 1 )
    {
      UInt uiLayerIdCGSSNR = getLayerCGSSNR();
      UInt uiBaseLayerIdCGSSNR = MSYS_UINT_MAX;
      if( uiLayerIdCGSSNR ) //may have dependent layer
      for( Int iLayer = (Int)m_uiDependencyId; iLayer >= 0; iLayer-- )
      {
        UInt uiLayer = iLayer;
        if( m_pcH264AVCEncoder->getCodingParameter()->getLayerParameters( uiLayer ).getLayerCGSSNR() == uiLayerIdCGSSNR
          && m_pcH264AVCEncoder->getCodingParameter()->getLayerParameters( uiLayer ).getQualityLevelCGSSNR() == 0 )
        {
          uiBaseLayerIdCGSSNR = m_pcH264AVCEncoder->getCodingParameter()->getLayerParameters( uiLayer ).getBaseLayerCGSSNR();
          break;
        }
      }

      if( uiLayerIdCGSSNR == 0 )
      {
        adMinBitrate[uiLayerIdCGSSNR][uiStage] = aaadCurrBits[uiLayerIdCGSSNR][uiStage][0] * dScale;
      }
      else //D!=0
      {
        if( adMinBitrate[uiBaseLayerIdCGSSNR][uiStage] ) //base layer with the same TL exists
        {
          adMinBitrate[uiLayerIdCGSSNR][uiStage] = adMinBitrate[uiBaseLayerIdCGSSNR][uiStage];
          for(UInt uiTIndex = 0; uiTIndex <= uiStage; uiTIndex++)
          {
            adMinBitrate[uiLayerIdCGSSNR][uiStage] += aaauidSeqBits[uiLayerIdCGSSNR][uiTIndex][0] * dScale;
          }
        }
        else // base layer non-exists
        {
          if(adMinBitrate[uiLayerIdCGSSNR][0] == 0.0) // first time for layer, uiStage = 0
          {
            if( uiBaseLayerIdCGSSNR == 0 && adMinBitrate[uiBaseLayerIdCGSSNR][0] == 0.0 ) //AVC-COMPATIBLE
            {
              UInt uiTL;
              for( uiTL = 0; uiTL < MAX_TEMP_LEVELS; uiTL++) //search minimum base layer bitrate
                if( adMinBitrate[uiBaseLayerIdCGSSNR][uiTL] )
                  break;
              adMinBitrate[uiLayerIdCGSSNR][uiStage] = aaauidSeqBits[uiLayerIdCGSSNR][uiStage][0]*dScale +
                ( adMinBitrate[uiBaseLayerIdCGSSNR][uiTL]*m_auiNumFramesCoded[uiTL]/m_auiNumFramesCoded[uiStage] ) / ( 1 << ( uiTL - uiStage ) );
            }
          }
          else //high layer without corresponding TL in base layer
          {
            adMinBitrate[uiLayerIdCGSSNR][uiStage] = aaauidSeqBits[uiLayerIdCGSSNR][uiStage][0]*dScale +
              adMinBitrate[uiLayerIdCGSSNR][uiStage-1]*m_auiNumFramesCoded[uiStage-1]/m_auiNumFramesCoded[uiStage]*2;
          }//if(adMinBitrate[m_uiBaseLayerId][uiStage]) //base layer exist for same TL
        }
      }
      printf( " %9s @ %7.4lf" " %10.4lf" " %10.4lf" " %8.4lf" " %8.4lf" " %8.4lf" "\n",
      acResolution,
      dFps,
      dBitrate / 1000.0,
      adMinBitrate[uiLayerIdCGSSNR][uiStage] / 1000.0,
      m_adPSNRSumY    [uiStage],
      m_adPSNRSumU    [uiStage],
      m_adPSNRSumV    [uiStage] );
    }
    else
    {
    if( m_uiDependencyId == 0 )
    {
      adMinBitrate[m_uiDependencyId][uiStage] = aaadCurrBits[m_uiDependencyId][uiStage][0] * dScale;
    }
    else //D!=0
    {
      if( adMinBitrate[m_uiBaseLayerId][uiStage] ) //base layer with the same TL exists
      {
        adMinBitrate[m_uiDependencyId][uiStage] = adMinBitrate[m_uiBaseLayerId][uiStage];
        for(UInt uiTIndex = 0; uiTIndex <= uiStage; uiTIndex++)
        {
          adMinBitrate[m_uiDependencyId][uiStage] += aaauidSeqBits[m_uiDependencyId][uiTIndex][0] * dScale;
        }
      }
      else // base layer non-exists
      {
        if(adMinBitrate[m_uiDependencyId][0] == 0.0) // first time for layer, uiStage = 0
        {
          if( m_uiBaseLayerId == 0 && adMinBitrate[m_uiBaseLayerId][0] == 0.0 ) //AVC-COMPATIBLE
          {
            UInt uiTL;
            for( uiTL = 0; uiTL < MAX_TEMP_LEVELS; uiTL++) //search minimum base layer bitrate
              if( adMinBitrate[m_uiBaseLayerId][uiTL] )
                break;
            adMinBitrate[m_uiDependencyId][uiStage] = aaauidSeqBits[m_uiDependencyId][uiStage][0]*dScale +
              ( adMinBitrate[m_uiBaseLayerId][uiTL]*m_auiNumFramesCoded[uiTL]/m_auiNumFramesCoded[uiStage] ) / ( 1 << ( uiTL - uiStage ) );
          }
        }
        else //high layer without corresponding TL in base layer
        {
          adMinBitrate[m_uiDependencyId][uiStage] = aaauidSeqBits[m_uiDependencyId][uiStage][0]*dScale +
            adMinBitrate[m_uiDependencyId][uiStage-1]*m_auiNumFramesCoded[uiStage-1]/m_auiNumFramesCoded[uiStage]*2;
        }//if(adMinBitrate[m_uiBaseLayerId][uiStage]) //base layer exist for same TL
      }
    }

    printf( " %9s @ %7.4lf" " %10.4lf" " %10.4lf" " %8.4lf" " %8.4lf" " %8.4lf" "\n",
      acResolution,
      dFps,
      dBitrate / 1000.0,
      adMinBitrate[m_uiDependencyId][uiStage] / 1000.0,
      m_adPSNRSumY    [uiStage],
      m_adPSNRSumU    [uiStage],
      m_adPSNRSumV    [uiStage] );

    }
  }

  ruiNumCodedFrames = m_auiNumFramesCoded[uiMaxStage];
  rdOutputRate      = m_fOutputFrameRate;

  //S051{
  if(m_uiAnaSIP>0&&m_cOutSIPFileName.length())
  {
    FILE* file=fopen(m_cOutSIPFileName.c_str(),"wt");

    if(file==NULL)
      return Err::m_nOK;

    for(UInt poc=0;poc<m_uiTotalFrame;poc++)
    {
      if(m_auiFrameBits[poc]!=0)
        fprintf(file,"%d ",m_auiFrameBits[poc]);
    }
    fclose(file);
  }
  //S051}

  return Err::m_nOK;
}


ErrVal
LayerEncoder::xWriteSEI( ExtBinDataAccessorList& rcOutExtBinDataAccessorList, SliceHeader& rcSH, UInt& ruiBit )
{
  UInt uiBit = 0;
  Bool m_bWriteSEI = true;
  if( m_bWriteSEI )
  {
    RNOK( xInitExtBinDataAccessor        (  m_cExtBinDataAccessor ) );
    RNOK( m_pcNalUnitEncoder->initNalUnit( &m_cExtBinDataAccessor ) );

    SEI::MessageList cSEIMessageList;
    SEI::SubSeqInfo* pcSubSeqInfo;
    RNOK( SEI::SubSeqInfo::create( pcSubSeqInfo ) );

    const UInt uiSubSeqLayer         = /*m_uiDecompositionStages+1-*/rcSH.getTemporalId();
    const Bool bFirstRefPicFlag      = rcSH.getNalRefIdc() && ! m_abIsRef[uiSubSeqLayer];
    const Bool bLeadingNonRefPicFlag = (0 == rcSH.getNalRefIdc());

    m_abIsRef[uiSubSeqLayer] |= (0 != rcSH.getNalRefIdc());

    cSEIMessageList.push_back( pcSubSeqInfo );
    RNOK( pcSubSeqInfo->init( uiSubSeqLayer, 0, bFirstRefPicFlag, bLeadingNonRefPicFlag ) );

    RNOK( m_pcNalUnitEncoder->write( cSEIMessageList ) );

    RNOK( m_pcNalUnitEncoder->closeNalUnit( uiBit ) );
    RNOK( xAppendNewExtBinDataAccessor( rcOutExtBinDataAccessorList, &m_cExtBinDataAccessor ) );
    uiBit += 4*8;
    ruiBit += uiBit;
  }

  return Err::m_nOK;
}


ErrVal
LayerEncoder::xWritePrefixUnit( ExtBinDataAccessorList& rcOutExtBinDataAccessorList, SliceHeader& rcSH, UInt& ruiBit )
{
  UInt uiBit = 0;
  Bool m_bWriteSuffixUnit = true;
  if( m_bWriteSuffixUnit )
  {
    RNOK( xInitExtBinDataAccessor        (  m_cExtBinDataAccessor ) );
    RNOK( m_pcNalUnitEncoder->initNalUnit( &m_cExtBinDataAccessor ) );

    NalUnitType eNalUnitType = rcSH.getNalUnitType();
    UInt eLayerId = rcSH.getDependencyId();

    rcSH.setDependencyId( 0 );

    if( eNalUnitType == NAL_UNIT_CODED_SLICE || eNalUnitType == NAL_UNIT_CODED_SLICE_IDR )
    {
      rcSH.setNalUnitType( NAL_UNIT_PREFIX );
    }
    else
    {
      return Err::m_nERR;
    }
    ETRACE_HEADER( "PREFIX UNIT" );

    RNOK( m_pcNalUnitEncoder->writePrefix( rcSH ) );

    RNOK( m_pcNalUnitEncoder->closeNalUnit( uiBit ) );
    RNOK( xAppendNewExtBinDataAccessor( rcOutExtBinDataAccessorList, &m_cExtBinDataAccessor ) );
    uiBit += 4*8;
    ruiBit += uiBit;

    rcSH.setNalUnitType( eNalUnitType );
    rcSH.setDependencyId( eLayerId );
  }

  return Err::m_nOK;
}
//prefix unit}}
//JVT-S036 lsj end

//NonRequired JVT-Q066 (06-04-08){{
ErrVal
LayerEncoder::xSetNonRequiredSEI(SliceHeader* pcSliceHeader, SEI::NonRequiredSei* pcNonRequiredSei)
{
  if( pcSliceHeader->getNoInterLayerPredFlag() || m_uiDependencyId - pcSliceHeader->getRefLayerDependencyId() > 1 )
  {
    if(pcNonRequiredSei->getNumInfoEntriesMinus1() != MSYS_UINT_MAX)
      pcNonRequiredSei->setNumInfoEntriesMinus1(pcNonRequiredSei->getNumInfoEntriesMinus1()+1);
    else
      pcNonRequiredSei->setNumInfoEntriesMinus1(0);
    pcNonRequiredSei->setEntryDependencyId(pcNonRequiredSei->getNumInfoEntriesMinus1(), m_uiDependencyId);

    UInt temp = 0;
    UInt i = pcNonRequiredSei->getNumInfoEntriesMinus1();
    UInt j = 0;

    if(pcSliceHeader->getNoInterLayerPredFlag())
      temp = m_uiDependencyId + 1;
    else
      temp = m_uiDependencyId - pcSliceHeader->getRefLayerDependencyId();

    while(temp > 1)
    {
      if(pcNonRequiredSei->getNumNonRequiredPicsMinus1(i) != MSYS_UINT_MAX)
        pcNonRequiredSei->setNumNonRequiredPicsMinus1(i,pcNonRequiredSei->getNumNonRequiredPicsMinus1(i)+16);
      else
        pcNonRequiredSei->setNumNonRequiredPicsMinus1(i, 15);

      for(UInt k = 0; j <= pcNonRequiredSei->getNumNonRequiredPicsMinus1(i) ; k++, j++)
      {
        pcNonRequiredSei->setNonNonRequiredPicDependencyId(i, j, m_uiDependencyId + 1 - temp);
        pcNonRequiredSei->setNonNonRequiredPicQulityLevel(i, j, k);
        pcNonRequiredSei->setNonNonRequiredPicFragmentOrder(i, j, 0);
      }
      temp--;
    }
  }
  else if(pcSliceHeader->getRefLayerQualityId() != 15)
  {
    if(pcNonRequiredSei->getNumInfoEntriesMinus1() != MSYS_UINT_MAX)
      pcNonRequiredSei->setNumInfoEntriesMinus1(pcNonRequiredSei->getNumInfoEntriesMinus1()+1);
    else
      pcNonRequiredSei->setNumInfoEntriesMinus1(0);
    pcNonRequiredSei->setEntryDependencyId(pcNonRequiredSei->getNumInfoEntriesMinus1(), m_uiDependencyId);

    UInt i = pcNonRequiredSei->getNumInfoEntriesMinus1();

    pcNonRequiredSei->setNumNonRequiredPicsMinus1(i, 15 - pcSliceHeader->getRefLayerQualityId() - 1);

    for(UInt j = 0; j <= pcNonRequiredSei->getNumNonRequiredPicsMinus1(i) ; j++)
    {
      pcNonRequiredSei->setNonNonRequiredPicDependencyId(i, j, pcSliceHeader->getRefLayerDependencyId());
      pcNonRequiredSei->setNonNonRequiredPicQulityLevel(i, j, pcSliceHeader->getRefLayerQualityId() + j + 1);
      pcNonRequiredSei->setNonNonRequiredPicFragmentOrder(i, j, 0);
    }
  }
  return Err::m_nOK;
}


//JVT-W052
ErrVal
LayerEncoder::xWriteIntegrityCheckSEI(ExtBinDataAccessorList &rcOutExtBinDataAccessorList, SEI::SEIMessage *pcSEIMessage, UInt &ruiBits)
{
  UInt uiBits = 0;
  RNOK( xInitExtBinDataAccessor      ( m_cExtBinDataAccessor ) );
  RNOK( m_pcNalUnitEncoder->initNalUnit(&m_cExtBinDataAccessor) );

  SEI::MessageList cSEIMessageList;
  cSEIMessageList.push_back(pcSEIMessage);
  RNOK( m_pcNalUnitEncoder->write(cSEIMessageList) );
  RNOK( m_pcNalUnitEncoder->closeNalUnit(uiBits) );

  ROF( &m_cExtBinDataAccessor);
  ROF( m_cExtBinDataAccessor.data());
  UInt uiNewSize = m_cExtBinDataAccessor.size();
  UChar* pucNewBuffer = new UChar[ uiNewSize];
  ROF ( pucNewBuffer);
  ::memcpy(pucNewBuffer, m_cExtBinDataAccessor.data(), uiNewSize * sizeof(UChar) );

  ExtBinDataAccessor* pcNewExtBinDataAccessor = new ExtBinDataAccessor;
  ROF( pcNewExtBinDataAccessor);
  m_cBinData.reset();
  m_cBinData.set(pucNewBuffer, uiNewSize);
  m_cBinData.setMemAccessor( *pcNewExtBinDataAccessor );
  rcOutExtBinDataAccessorList.push_front( pcNewExtBinDataAccessor);
  m_cBinData.reset();
  m_cBinData.setMemAccessor(m_cExtBinDataAccessor );

  uiBits += 4*8;
  ruiBits += uiBits;
  return Err::m_nOK;
}
//JVT-W052

ErrVal
LayerEncoder::xWriteNonRequiredSEI( ExtBinDataAccessorList& rcOutExtBinDataAccessorList, SEI::NonRequiredSei* pcNonRequiredSei, UInt& ruiBit )
{
  UInt uiBit = 0;

  RNOK( xInitExtBinDataAccessor        (  m_cExtBinDataAccessor ) );
  RNOK( m_pcNalUnitEncoder->initNalUnit( &m_cExtBinDataAccessor ) );

  SEI::MessageList cSEIMessageList;
  cSEIMessageList.push_back( pcNonRequiredSei );

  RNOK( m_pcNalUnitEncoder->write( cSEIMessageList ) );
  RNOK( m_pcNalUnitEncoder->closeNalUnit( uiBit ) );

  ROF( &m_cExtBinDataAccessor );
  ROF( m_cExtBinDataAccessor.data() );
  UInt    uiNewSize     = m_cExtBinDataAccessor.size();
  UChar*  pucNewBuffer  = new UChar [ uiNewSize ];
  ROF( pucNewBuffer );
  ::memcpy( pucNewBuffer, m_cExtBinDataAccessor.data(), uiNewSize * sizeof( UChar ) );

  ExtBinDataAccessor* pcNewExtBinDataAccessor = new ExtBinDataAccessor;
  ROF( pcNewExtBinDataAccessor );
  m_cBinData              .reset          ();
  m_cBinData              .set            (  pucNewBuffer, uiNewSize );
  m_cBinData              .setMemAccessor ( *pcNewExtBinDataAccessor );
  rcOutExtBinDataAccessorList.push_front     (  pcNewExtBinDataAccessor );
  m_cBinData              .reset          ();
  m_cBinData              .setMemAccessor ( m_cExtBinDataAccessor );

  uiBit += 4*8;
  ruiBit += uiBit;

  return Err::m_nOK;
}


// JVT-V068 HRD {
ErrVal
LayerEncoder::xWriteSEI(ExtBinDataAccessorList &rcOutExtBinDataAccessorList, SEI::MessageList& rcSEIMessageList, UInt &ruiBits)
{
  UInt uiBits = 0;
  RNOK( xInitExtBinDataAccessor      ( m_cExtBinDataAccessor ) );

  RNOK( m_pcNalUnitEncoder->initNalUnit(&m_cExtBinDataAccessor) );
  RNOK( m_pcNalUnitEncoder->write(rcSEIMessageList) );
  RNOK( m_pcNalUnitEncoder->closeNalUnit(uiBits) );

  ROF( &m_cExtBinDataAccessor);
  ROF( m_cExtBinDataAccessor.data());
  UInt uiNewSize = m_cExtBinDataAccessor.size();
  UChar* pucNewBuffer = new UChar[ uiNewSize];
  ROF ( pucNewBuffer);
  ::memcpy(pucNewBuffer, m_cExtBinDataAccessor.data(), uiNewSize * sizeof(UChar) );

  ExtBinDataAccessor* pcNewExtBinDataAccessor = new ExtBinDataAccessor;
  ROF( pcNewExtBinDataAccessor);
  m_cBinData.reset();
  m_cBinData.set(pucNewBuffer, uiNewSize);
  m_cBinData.setMemAccessor( *pcNewExtBinDataAccessor );
  rcOutExtBinDataAccessorList.push_front( pcNewExtBinDataAccessor);
  m_cBinData.reset();
  m_cBinData.setMemAccessor(m_cExtBinDataAccessor );

  uiBits += 4*8;
  ruiBits += uiBits;

  return Err::m_nOK;
}

ErrVal
LayerEncoder::xWriteNestingSEIforHrd(ExtBinDataAccessorList &rcOutExtBinDataAccessorList, SEI::SEIMessage *pcSEIMessage, UInt uiDependencyId, UInt uiQualityLevel, UInt uiTemporalLevel, UInt &ruiBits)
{
  UInt uiBits = 0;
  RNOK( xInitExtBinDataAccessor      ( m_cExtBinDataAccessor ) );

  RNOK( m_pcNalUnitEncoder->initNalUnit(&m_cExtBinDataAccessor) );

  SEI::ScalableNestingSei* pcScalableNestingSei;
  RNOK( SEI::ScalableNestingSei::create(pcScalableNestingSei) );

  //===== set message =====
  pcScalableNestingSei->setAllPicturesInAuFlag( 0 );
  pcScalableNestingSei->setNumPictures( 1 );
  pcScalableNestingSei->setDependencyId( 0, uiDependencyId );
  pcScalableNestingSei->setQualityLevel( 0, uiQualityLevel );
  pcScalableNestingSei->setTemporalId( uiTemporalLevel );

  SEI::MessageList  cSEIMessageList;
  cSEIMessageList.push_back( pcScalableNestingSei );
  cSEIMessageList.push_back( pcSEIMessage );
  RNOK( m_pcNalUnitEncoder->writeScalableNestingSei(cSEIMessageList) );
  RNOK( m_pcNalUnitEncoder->closeNalUnit(uiBits) );

  ROF( &m_cExtBinDataAccessor);
  ROF( m_cExtBinDataAccessor.data());
  UInt uiNewSize = m_cExtBinDataAccessor.size();
  UChar* pucNewBuffer = new UChar[ uiNewSize];
  ROF ( pucNewBuffer);
  ::memcpy(pucNewBuffer, m_cExtBinDataAccessor.data(), uiNewSize * sizeof(UChar) );

  ExtBinDataAccessor* pcNewExtBinDataAccessor = new ExtBinDataAccessor;
  ROF( pcNewExtBinDataAccessor);
  m_cBinData.reset();
  m_cBinData.set(pucNewBuffer, uiNewSize);
  m_cBinData.setMemAccessor( *pcNewExtBinDataAccessor );
  rcOutExtBinDataAccessorList.push_front( pcNewExtBinDataAccessor);
  m_cBinData.reset();
  m_cBinData.setMemAccessor(m_cExtBinDataAccessor );

  uiBits += 4*8;
  ruiBits += uiBits;

  return Err::m_nOK;
}

ErrVal
LayerEncoder::xWriteSEIforAVCCompatibleHrd(ExtBinDataAccessorList &rcOutExtBinDataAccessorList, SEI::SEIMessage* pcSEIMessage, UInt &ruiBits)
{
  UInt uiBits = 0;
  RNOK( xInitExtBinDataAccessor      ( m_cExtBinDataAccessor ) );
  RNOK( m_pcNalUnitEncoder->initNalUnit(&m_cExtBinDataAccessor) );

  SEI::MessageList cSEIMessageList;
  cSEIMessageList.push_back(pcSEIMessage);
  RNOK( m_pcNalUnitEncoder->write(cSEIMessageList) );
  RNOK( m_pcNalUnitEncoder->closeNalUnit(uiBits) );

  ROF( &m_cExtBinDataAccessor);
  ROF( m_cExtBinDataAccessor.data());
  UInt uiNewSize = m_cExtBinDataAccessor.size();
  UChar* pucNewBuffer = new UChar[ uiNewSize];
  ROF ( pucNewBuffer);
  ::memcpy(pucNewBuffer, m_cExtBinDataAccessor.data(), uiNewSize * sizeof(UChar) );

  ExtBinDataAccessor* pcNewExtBinDataAccessor = new ExtBinDataAccessor;
  ROF( pcNewExtBinDataAccessor);
  m_cBinData.reset();
  m_cBinData.set(pucNewBuffer, uiNewSize);
  m_cBinData.setMemAccessor( *pcNewExtBinDataAccessor );
  rcOutExtBinDataAccessorList.push_front( pcNewExtBinDataAccessor);
  m_cBinData.reset();
  m_cBinData.setMemAccessor(m_cExtBinDataAccessor );

  uiBits += 4*8;
  ruiBits += uiBits;

  return Err::m_nOK;
}

ErrVal
LayerEncoder::xCalculateTiming( PicOutputDataList&  rcPicOutputDataList, UInt uiFrame )
{
  if ( !m_bEnableHrd ) return Err::m_nOK;

  UInt uiFrameIdInGOP = uiFrame >> m_uiDecompositionStages;
  ControlData&  rcControlData   = m_pacControlData[uiFrameIdInGOP];
  PicOutputDataList::iterator iter;
  for (iter = rcPicOutputDataList.begin(); iter != rcPicOutputDataList.end(); iter++)
  {
    UInt uiBaseQualityLevel;
    UInt uiBaseLayerId;
    UInt uiBitCounts = iter->Bits;

    SliceHeader* pcSH = rcControlData.getSliceHeader( PicType( iter->iPicType ) );

    uiBaseQualityLevel = iter->uiBaseQualityLevel;
    uiBaseLayerId = iter->uiBaseLayerId;

    if ( iter->DependencyId == uiBaseLayerId )
    {
      uiBaseQualityLevel = iter->QualityId > (Int) uiBaseQualityLevel ? uiBaseQualityLevel : iter->QualityId - 1;
    }
    else if ( uiBaseLayerId!=MSYS_UINT_MAX && uiBaseQualityLevel!=MSYS_UINT_MAX )
    {
      while(!m_apcScheduler->get((uiBaseLayerId<<7)+ ((iter->TemporalId)<<4) + uiBaseQualityLevel) && (uiBaseQualityLevel--) );
    }

    if ( uiBaseLayerId!=MSYS_UINT_MAX && uiBaseQualityLevel!=MSYS_UINT_MAX )
    {
      for (UInt uiTmp = 0; uiTmp <= uiBaseQualityLevel; uiTmp++)
        uiBitCounts += m_apcScheduler->get((uiBaseLayerId<<7)+(iter->TemporalId<<4)+uiTmp)->getLayerBits();
    }

    for (UInt uiTemporalId=iter->TemporalId; uiTemporalId<=m_uiDecompositionStages; uiTemporalId++)
    {
      m_apcScheduler->get(((iter->DependencyId)<<7)+(uiTemporalId<<4)+(iter->QualityId))->setLayerBits(iter->QualityId == 0 ? uiBitCounts : iter->Bits); // if layer's quality_id == 0, store the sum of bits in all reference layers, otherwise, only store the bits of current layer.
      m_apcScheduler->get(((iter->DependencyId)<<7)+(uiTemporalId<<4)+(iter->QualityId))->calculateTiming(uiBitCounts, uiBitCounts, pcSH->getIdrFlag(), pcSH->getFieldPicFlag());
    }
  }
  return Err::m_nOK;
}
// JVT-V068 HRD }

//JVT-W049 {
ErrVal
LayerEncoder::xWriteRedundantKeyPicSEI ( ExtBinDataAccessorList& rcOutExtBinDataAccessorList, UInt &ruiBits )
{
  //===== create message =====
  UInt uiBits = 0;
  RNOK( xInitExtBinDataAccessor      ( m_cExtBinDataAccessor ) );
  RNOK( m_pcNalUnitEncoder->initNalUnit(&m_cExtBinDataAccessor) );
  SEI::RedundantPicSei* pcRedundantPicSEI;
  RNOK( SEI::RedundantPicSei::create( pcRedundantPicSEI ) );
  //===== set message =====
  UInt uiInputLayers  = m_uiNumberLayersCnt;
  UInt uiNumdIdMinus1 = uiInputLayers - 1;
  UInt uiNumqIdMinus1 = 0;//may be changed
  UInt uiRedundantPicCntMinus1 = 0;//may be changed
  pcRedundantPicSEI->setNumDIdMinus1( uiNumdIdMinus1 );
  for( UInt i = 0; i <= uiNumdIdMinus1; i++ )
  {
    pcRedundantPicSEI->setDependencyId ( i, i );
    pcRedundantPicSEI->setNumQIdMinus1 ( i, uiNumqIdMinus1 );
    for( UInt j = 0; j <= uiNumqIdMinus1; j++ )
    {
      pcRedundantPicSEI->setQualityId ( i, j, j );
      pcRedundantPicSEI->setNumRedundantPicsMinus1 ( i, j, uiRedundantPicCntMinus1 );
      for( UInt k = 0; k <= uiRedundantPicCntMinus1; k++ )
      {
        pcRedundantPicSEI->setRedundantPicCntMinus1 ( i, j, k, k );
        Bool bPicMatchFlag = true;//may be changed
        pcRedundantPicSEI->setPicMatchFlag ( i, j, k, bPicMatchFlag );
        if ( bPicMatchFlag == false )
        {
           Bool bMbTypeMatchFlag = true;//may be changed
           Bool bMotionMatchFlag = true;//may be changed
           Bool bResidualMatchFlag = true;//may be changed
           Bool bIntraSamplesMatchFlag = true;//may be changed
           pcRedundantPicSEI->setMbTypeMatchFlag ( i, j, k, bMbTypeMatchFlag );
                 pcRedundantPicSEI->setMotionMatchFlag ( i, j, k, bMotionMatchFlag );
                   pcRedundantPicSEI->setResidualMatchFlag ( i, j, k, bResidualMatchFlag );
                   pcRedundantPicSEI->setIntraSamplesMatchFlag ( i, j, k, bIntraSamplesMatchFlag );
                }
      }
    }
  }

  //===== write message =====
  SEI::MessageList  cSEIMessageList;
  cSEIMessageList.push_back( pcRedundantPicSEI );
  RNOK( m_pcNalUnitEncoder->write(cSEIMessageList) );
  RNOK( m_pcNalUnitEncoder->closeNalUnit(uiBits) );

  ROF( &m_cExtBinDataAccessor);
  ROF( m_cExtBinDataAccessor.data());
  UInt uiNewSize = m_cExtBinDataAccessor.size();
  UChar* pucNewBuffer = new UChar[ uiNewSize];
  ROF ( pucNewBuffer);
  ::memcpy(pucNewBuffer, m_cExtBinDataAccessor.data(), uiNewSize * sizeof(UChar) );

  ExtBinDataAccessor* pcNewExtBinDataAccessor = new ExtBinDataAccessor;
  ROF( pcNewExtBinDataAccessor);
  m_cBinData.reset();
  m_cBinData.set(pucNewBuffer, uiNewSize);
  m_cBinData.setMemAccessor( *pcNewExtBinDataAccessor );
  rcOutExtBinDataAccessorList.push_front( pcNewExtBinDataAccessor);
  m_cBinData.reset();
  m_cBinData.setMemAccessor(m_cExtBinDataAccessor );

  uiBits += 4*8;
  ruiBits += uiBits;

  return Err::m_nOK;
}
//JVT-W049 }


Void
LayerEncoder::xAssignSimplePriorityId( SliceHeader* pcSliceHeader )
{
    // Lookup simple priority ID from mapping table (J. Ridge, Y-K. Wang @ Nokia)
    Bool bFound = false;
//JVT-S036 lsj start
 //   for ( UInt uiSimplePriId = 0; uiSimplePriId < (1 << PRI_ID_BITS); uiSimplePriId++ )
  //  {
   //     UInt uiLayer, uiTempLevel, uiQualLevel;
   //     m_pcSPS->getSimplePriorityMap( uiSimplePriId, uiTempLevel, uiLayer, uiQualLevel );
  //      if ( pcSliceHeader->getTemporalId() == uiTempLevel && m_uiDependencyId == uiLayer && pcSliceHeader->getQualityId() == uiQualLevel )
    //    {
            pcSliceHeader->setPriorityId ( 0 ); //lsj The syntax element is not used by the decoding process specified in this Recommendation
            bFound = true;
      //      break;
    //    }
   // }
//JVT-S036 lsj end
    //AOF( bFound );
}


Void
LayerEncoder::xPAffDecision( UInt uiFrame )
{
  switch( m_pcLayerParameters->getPAff() )
  {
  case 0:
    m_pbFieldPicFlag[ uiFrame ] = false;

    break;

  case 1:
    m_pbFieldPicFlag[ uiFrame ] = true;
    break;

  case 2:
    m_pbFieldPicFlag[ uiFrame ] = ( rand()%2 ) ? true : false;
    break;

  default:
    AOT( 1 );
    break;
  }
}


//S051{
Bool LayerEncoder:: xSIPCheck	(UInt Poc)
{
  if(Poc==0)               //There seems to be  a bug in decoder if we can
    return false;        //discard picture with Poc=0. So here I forbid Poc=0
  if(std::find(m_cPocList.begin(),m_cPocList.end(),Poc)!=m_cPocList.end())
    return true;
  return false;
}
//S051}


// TMM_INTERLACE{
ErrVal
LayerEncoder::xMotionEstimationMbAff( RefFrameList*    pcRefFrameList0,
                                     RefFrameList*    pcRefFrameList1,
                                     MbDataCtrl*      pcMbDataCtrlCol,
                                     Frame*        pcOrigFrame,
                                     Frame*        pcIntraRecFrame,
                                     ControlData&     rcControlData,
                                     UInt             uiNumMaxIter,
                                     UInt             uiIterSearchRange,
                                     UInt             uiFrameIdInGOP )
{
  MbEncoder*    pcMbEncoder							=  m_pcSliceEncoder->getMbEncoder         ();
  SliceHeader&  rcSliceHeader						= *rcControlData.getSliceHeader           ( FRAME );
  MbDataCtrl*   pcMbDataCtrl						=  rcControlData.getMbDataCtrl            ();
  Frame*     pcBaseLayerFrame				=  rcControlData.getBaseLayerRec          ();
  Frame*     pcBaseLayerResidual	    =  rcControlData.getBaseLayerSbb          ();
  MbDataCtrl*   pcBaseLayerCtrl					=  rcControlData.getBaseLayerCtrl         ();
  MbDataCtrl*   pcBaseLayerCtrlField    =  rcControlData.getBaseLayerCtrlField    ();
  UInt          uiMaxMvPerMb            = rcSliceHeader.getSPS().getMaxMVsPer2Mb () >> 1; // hard limit (don't take into account last macroblock)

  FMO* pcFMO = rcControlData.getSliceHeader( FRAME )->getFMO(); //TMM

  for (UInt iSliceGroupID=0;!pcFMO->SliceGroupCompletelyCoded(iSliceGroupID);iSliceGroupID++)
  {
    rcSliceHeader.setFirstMbInSlice(pcFMO->getFirstMacroblockInSlice(iSliceGroupID));
    rcSliceHeader.setLastMbInSlice(pcFMO->getLastMBInSliceGroup(iSliceGroupID));
    // JVT-S054 (2) (ADD)
    rcSliceHeader.setNumMbsInSlice(rcSliceHeader.getFMO()->getNumMbsInSlice(rcSliceHeader.getFirstMbInSlice(), rcSliceHeader.getLastMbInSlice()));
    UInt uiMbAddress       = rcSliceHeader.getFirstMbInSlice();
    UInt uiLastMbAddress   = rcSliceHeader.getLastMbInSlice();
    UInt uiNumMBInSlice;
    //===== initialization =====
    RNOK( pcMbDataCtrl->initSlice( rcSliceHeader, ENCODE_PROCESS, false, pcMbDataCtrlCol ) );
    if( ! m_bLoadMotionInfo )
    {
      RNOK( m_pcMotionEstimation->initSlice( rcSliceHeader ) );
      RNOK( pcMbEncoder         ->initSlice( rcSliceHeader ) );
    }
    RefFrameList acRefFrameList0    [2];
    RefFrameList acRefFrameList1    [2];

    Frame* apcOrigFrame        [4] = { NULL, NULL, NULL, NULL };
    Frame* apcIntraRecFrame    [4] = { NULL, NULL, NULL, NULL };
    Frame* apcBaseLayerFrame   [4] = { NULL, NULL, NULL, NULL };
    Frame* apcBaseLayerResidual[4] = { NULL, NULL, NULL, NULL };

    RNOK( gSetFrameFieldArrays( apcOrigFrame,         pcOrigFrame         ) );
    RNOK( gSetFrameFieldArrays( apcIntraRecFrame,     pcIntraRecFrame     ) );
    RNOK( gSetFrameFieldArrays( apcBaseLayerFrame,    pcBaseLayerFrame    ) );
    RNOK( gSetFrameFieldArrays( apcBaseLayerResidual, pcBaseLayerResidual ) );

    RNOK( gSetFrameFieldLists( acRefFrameList0[0], acRefFrameList0[1], *pcRefFrameList0 ) );
    RNOK( gSetFrameFieldLists( acRefFrameList1[0], acRefFrameList1[1], *pcRefFrameList1 ) );

    RefFrameList* apcRefFrameList0[4];
    RefFrameList* apcRefFrameList1[4];

    apcRefFrameList0[0] = ( NULL == pcRefFrameList0 ) ? NULL : &acRefFrameList0[0];
    apcRefFrameList0[1] = ( NULL == pcRefFrameList0 ) ? NULL : &acRefFrameList0[1];
    apcRefFrameList1[0] = ( NULL == pcRefFrameList1 ) ? NULL : &acRefFrameList1[0];
    apcRefFrameList1[1] = ( NULL == pcRefFrameList1 ) ? NULL : &acRefFrameList1[1];
    apcRefFrameList0[2] = apcRefFrameList0[3] = pcRefFrameList0;
    apcRefFrameList1[2] = apcRefFrameList1[3] = pcRefFrameList1;

    YuvMbBuffer acIntYuvMbBuffer[2];

    MbDataBuffer acMbData[2];
    Bool   abSkipModeAllowed[4] = {true,true,true,true};
    IntMbTempData cMbTempDataBase;

    //===== loop over macroblocks =====
    for(  uiNumMBInSlice = 0; uiMbAddress <= uiLastMbAddress; uiMbAddress+=2 )
    {
      Double adCost[2]  = {0,0};
      for( Int eP = 0; eP < 4; eP++ )
      {
        MbDataAccess* pcMbDataAccess     = NULL;
        MbDataAccess* pcMbDataAccessBase = NULL;
        Double        dCost = 0;
        UInt          uiMbY, uiMbX;

        const Bool    bField = (eP < 2);
        const UInt    uiMbAddressMbAff = uiMbAddress+(eP%2);

        rcSliceHeader.getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddressMbAff );

        //===== init macroblock =====
        RNOK  ( pcMbDataCtrl            ->initMb( pcMbDataAccess,     uiMbY, uiMbX ) );

        if  (eP < 2 && pcBaseLayerCtrlField)  // field case
        {
          RNOK( pcBaseLayerCtrlField         ->initMb( pcMbDataAccessBase, uiMbY, uiMbX ) );
        }
        else if ( eP >= 2 && pcBaseLayerCtrl)  //frame case
        {
          RNOK( pcBaseLayerCtrl         ->initMb( pcMbDataAccessBase, uiMbY, uiMbX ) );
        }

        if( 0 == eP)
        {
          abSkipModeAllowed[1] = pcMbDataAccess->getDefaultFieldFlag(); // do not move
          abSkipModeAllowed[3] = ! abSkipModeAllowed[1];
        }
        else if( eP%2 == 1 )
        {
          const MbData&	rcTopMb = pcMbDataAccess->getMbDataComplementary();
          if ( rcTopMb.getSkipFlag() && ( pcMbDataAccess->getDefaultFieldFlag() != rcTopMb.getFieldFlag() ) )
            abSkipModeAllowed[eP] = false;
        }

        if( ! m_bLoadMotionInfo )
        {
          pcMbDataAccess->setFieldMode( eP < 2 );
          //===== initialisation =====
          RNOK( m_pcYuvFullPelBufferCtrl->initMb( uiMbY, uiMbX, true ) );
          RNOK( m_pcYuvHalfPelBufferCtrl->initMb( uiMbY, uiMbX, true ) );
          RNOK( m_pcMotionEstimation    ->initMb( uiMbY, uiMbX, *pcMbDataAccess ) );

          if( ! rcSliceHeader.getNoInterLayerPredFlag() )
          {
            Int iFieldMode = ( eP < 2 ? 1 : 0 );
            m_pcSliceEncoder->m_pcMbEncoder->setBaseModeAllowedFlag( m_apabBaseModeFlagAllowedArrays[iFieldMode][uiMbAddressMbAff] );
          }

          //===== estimate prediction data =====
          RNOK( pcMbEncoder->estimatePrediction ( *pcMbDataAccess,
                                                  pcMbDataAccessBase,
                                                  *apcRefFrameList0    [eP],
                                                  *apcRefFrameList1    [eP],
                                                  apcBaseLayerFrame   [eP],
                                                  apcBaseLayerResidual[eP],
                                                  *apcOrigFrame        [eP],
                                                  *apcIntraRecFrame    [eP],
                                                  uiMaxMvPerMb,
                                                  m_bMCBlks8x8Disable,
                                                  m_bBiPred8x8Disable,
                                                  uiNumMaxIter,
                                                  uiIterSearchRange,
                                                  rcControlData.getLambda(),
                                                  dCost,
                                                  abSkipModeAllowed   [eP]) );

// TMM_INTERLACE {
          /*if( m_bSaveMotionInfo )
          {
            //===== save prediction data =====
       // saveAll is displaced because the Mvs are Ok but the other data could be modified (mode,...).
       // Do it after m_pcSliceEncoder->encodeHighPassPicture
       //            RNOK( pcMbDataAccess->getMbData().saveAll( m_pMotionInfoFile ) );
          }*/
// TMM_INTERLACE }
        }
        else
        {
          //===== load prediction data =====
          RNOK( pcMbDataAccess->getMbData().loadAll( m_pMotionInfoFile ) );
        }

        if( adCost[eP>>1] != DOUBLE_MAX )
        {
          adCost [eP>>1] += dCost;
        }
        if( bField )
        {
          acMbData[eP].copy( pcMbDataAccess->getMbData() );
          //TMM_INTERLACE
          (&acIntYuvMbBuffer[eP])->loadBuffer( apcIntraRecFrame[eP]->getFullPelYuvBuffer() );
        }
      }
      Bool bFieldMode = adCost[0] < adCost[1];

      if ( m_bLoadMotionInfo )
      {
        char ch;
        fread( &ch, sizeof(char), 1, m_pMotionInfoFile );
        bFieldMode = ( ch == 1 );
      }
      if ( m_bSaveMotionInfo )
      {
        char ch = ( bFieldMode == true );
        fwrite( &ch, sizeof(char), 1, m_pMotionInfoFile );
      }

  #ifdef RANDOM_MBAFF
      bFieldMode = gBoolRandom();
  #endif

      if( bFieldMode )
      {
        // copy field modes back
        UInt          uiMbY, uiMbX;
        MbDataAccess* pcMbDataAccess = NULL;
        rcSliceHeader.getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddress          );
        RNOK( pcMbDataCtrl      ->initMb      ( pcMbDataAccess,     uiMbY, uiMbX ) );
        pcMbDataAccess->getMbData().copy( acMbData[0] );
        apcIntraRecFrame[0]->getFullPelYuvBuffer()->loadBuffer( &acIntYuvMbBuffer[0] );

        pcMbDataAccess = NULL;
        rcSliceHeader.getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddress+1        );
        RNOK( pcMbDataCtrl      ->initMb      ( pcMbDataAccess,     uiMbY, uiMbX ) );
        pcMbDataAccess->getMbData().copy( acMbData[1] );
        apcIntraRecFrame[1]->getFullPelYuvBuffer()->loadBuffer( &acIntYuvMbBuffer[1] );
      }
       //uiMbAddress = rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress);
       uiNumMBInSlice++;
    }
  }
  return Err::m_nOK;
}

// TMM_INTERLACE}
//JVT-U106 Behaviour at slice boundaries}
H264AVC_NAMESPACE_END


