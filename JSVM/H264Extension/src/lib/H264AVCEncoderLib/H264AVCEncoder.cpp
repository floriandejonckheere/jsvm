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

#include "GOPEncoder.h"
#include "CreaterH264AVCEncoder.h"
#include "ControlMngH264AVCEncoder.h"
#include "H264AVCCommonLib/ParameterSetMng.h"
#include "H264AVCCommonLib/FrameMng.h"

#include <math.h>

// JVT-V068 HRD {
#include "Scheduler.h"
#include "H264AVCCommonLib/Hrd.h"
// JVT-V068 HRD }

H264AVC_NAMESPACE_BEGIN


H264AVCEncoder::H264AVCEncoder():
  m_pcParameterSetMng ( NULL ),
  m_pcPocCalculator   ( NULL ),
  m_pcNalUnitEncoder  ( NULL ),
  m_pcControlMng      ( NULL ),
  m_pcCodingParameter ( NULL ),
  m_pcFrameMng        ( NULL ),
#ifdef SHARP_AVC_REWRITE_OUTPUT
  m_pcMbDataCtrl      ( NULL ),
#endif
  m_bVeryFirstCall    ( true ),
  m_bScalableSeiMessage( false ),
  m_bInitDone         ( false ),
  m_bTraceEnable      ( false ),
  m_bWrteROISEI		  ( true ), 
  m_loop_roi_sei	  ( 0 )
{
  ::memset( m_apcMCTFEncoder, 0x00, MAX_LAYERS*sizeof(Void*) );
  ::memset( m_aaadFinalFramerate, 0x00, MAX_LAYERS*MAX_TEMP_LEVELS*MAX_QUALITY_LEVELS*sizeof(Double) );
  ::memset( m_aaadSeqBits,        0x00, MAX_LAYERS*MAX_TEMP_LEVELS*MAX_QUALITY_LEVELS*sizeof(Double) );
  //JVT-W052
  ::memset( m_uiNumofCGS,     0x00, MAX_LAYERS*sizeof(UInt) );
  ::memset( m_uicrcVal,       0x00, MAX_LAYERS*sizeof(UInt) );
  //JVT-W052
  //JVT-W051 {	
  ::memset( m_uiProfileIdc, 0, MAX_LAYERS*sizeof(UInt) );
  ::memset( m_uiLevelIdc, 0, MAX_LAYERS*sizeof(UInt) );
  ::memset( m_bConstraint0Flag, 0, MAX_LAYERS*sizeof(Bool) );
  ::memset( m_bConstraint1Flag, 0, MAX_LAYERS*sizeof(Bool) );
  ::memset( m_bConstraint2Flag, 0, MAX_LAYERS*sizeof(Bool) );
  ::memset( m_bConstraint3Flag, 0, MAX_LAYERS*sizeof(Bool) );
  ::memset( m_uiFrameNumInGOP, 0x00, MAX_LAYERS*sizeof(UInt) );
  ::memset( m_uiCodeFrameNum, 0x00, MAX_LAYERS*sizeof(UInt) );
  m_aaadFrameInTimeWindowBits = new Double**[300];
  for ( UInt i = 0; i < 300; i++ )
  {
    m_aaadFrameInTimeWindowBits[i] = new Double*[MAX_LAYERS];
    for ( UInt j = 0; j < MAX_LAYERS; j++ )
    {
      m_aaadFrameInTimeWindowBits[i][j] = new Double[MAX_QUALITY_LEVELS];
      for ( UInt k = 0; k < MAX_QUALITY_LEVELS; k++ )
      {
        m_aaadFrameInTimeWindowBits[i][j][k] = 0;
      }
    }
  }
  ::memset( m_aaadFrameInGOPBits, 0x00, 65*MAX_LAYERS*MAX_QUALITY_LEVELS*sizeof(Double) );
  ::memset( m_adAvgBitrate, 0x00, MAX_LAYERS*sizeof(Double) );
  ::memset( m_aadMaxBitrate, 0x00, MAX_LAYERS*MAX_QUALITY_LEVELS*sizeof(Double) );
  //JVT-W051 }
  for( UInt ui = 0; ui < MAX_LAYERS; ui++ )
  {
    for( UInt uj = 0; uj < MAX_TEMP_LEVELS; uj++ )
    {
      for( UInt uk = 0; uk < MAX_QUALITY_LEVELS; uk++ )
      {
        m_aaauidSeqBits[ui][uj][uk] = 0;                   
        m_aaadSingleLayerBitrate[ui][uj][uk] = 0;            // BUG_FIX Shenqiu (06-04-08)
        m_aaauiScalableLayerId[ui][uj][uk] = MSYS_UINT_MAX;  // BUG_FIX Shenqiu (06-04-08)
      }
    }
  }

}

H264AVCEncoder::~H264AVCEncoder()
{
}


ErrVal
H264AVCEncoder::create( H264AVCEncoder*& rpcH264AVCEncoder )
{
  rpcH264AVCEncoder = new H264AVCEncoder;

  ROT( NULL == rpcH264AVCEncoder );

  return Err::m_nOK;
}


ErrVal
H264AVCEncoder::destroy()
{
  delete this;
  return Err::m_nOK;
}


IntFrame*
H264AVCEncoder::getLowPassRec( UInt uiLayerId )
{
  //===== shall not be called in non-CGS mode =====
  IntFrame* pcLPRec = 0;
  for( UInt uiBL = MSYS_UINT_MAX; ( uiBL = m_pcCodingParameter->getLayerParameters( uiLayerId ).getBaseLayerId() ) != MSYS_UINT_MAX; uiLayerId = uiBL )
  {
    if( m_apcMCTFEncoder[uiBL]->getMGSLPRec() )
    {
      pcLPRec = m_apcMCTFEncoder[uiBL]->getMGSLPRec();
    }
    else
    {
      break;
    }
  }
  return pcLPRec;
}


IntFrame*
H264AVCEncoder::getELRefPic( UInt uiLayerId, Int iPoc,
                            UInt uiIdrPicId)//EIDR 0619
{
  IntFrame* pcELRefPic = 0;
  for( UInt uiELId = uiLayerId + 1; uiELId < m_pcCodingParameter->getNumberOfLayers(); uiELId++ )
  {
    //===== check whether higher layer can be used =====
    LayerParameters&  rcCL        = m_pcCodingParameter->getLayerParameters( uiLayerId  );
    LayerParameters&  rcEL        = m_pcCodingParameter->getLayerParameters( uiELId     );
    UInt              uiScalType  = MSYS_UINT_MAX;
    if( ! rcEL.getResizeParameters()->m_bCrop && ( rcCL.getBaseLayerId() == MSYS_UINT_MAX || ! rcCL.getResizeParameters()->m_bCrop ) )
    {
      if(   rcCL.getFrameWidth () == rcEL.getFrameWidth () &&
            rcCL.getFrameHeight() == rcEL.getFrameHeight()   )   uiScalType  = 0;
      if( 2*rcCL.getFrameWidth () == rcEL.getFrameWidth () &&
          2*rcCL.getFrameHeight() == rcEL.getFrameHeight()   )   uiScalType  = 1;
    }

    //===== get frame =====
    if( uiScalType == 0 )
    {
      pcELRefPic = m_apcMCTFEncoder[uiELId]->getRefPic( iPoc, uiIdrPicId); //EIDR 0619
    }
    else
    {
      break;
    }
  }
  return pcELRefPic;
}



ErrVal
H264AVCEncoder::getBaseLayerStatus( UInt&   ruiBaseLayerId,
                                    UInt&   ruiBaseLayerIdMotionOnly,
                                    Int &   riSpatialScalabilityType,
                                    UInt    uiLayerId,
                                    Int     iPoc,
                                    PicType ePicType,
                                    UInt		uiIdrPicId)//EIDR 0619
{
  //===== get spatial resolution ratio =====
 
// TMM_ESS 
  riSpatialScalabilityType = m_apcMCTFEncoder[uiLayerId]->getSpatialScalabilityType();

  //===== check data availability =====
  if( ruiBaseLayerId < m_pcCodingParameter->getNumberOfLayers() )
  {
    Bool  bExists = false;
    Bool  bMotion = false;

    RNOK( m_apcMCTFEncoder[ruiBaseLayerId]->getBaseLayerStatus( bExists, bMotion, iPoc, ePicType
                                                                ,uiIdrPicId) );//EIDR 0619

    ruiBaseLayerIdMotionOnly  = ( bMotion ? ruiBaseLayerId : MSYS_UINT_MAX );
    ruiBaseLayerId            = ( bExists ? ruiBaseLayerId : MSYS_UINT_MAX );
    return Err::m_nOK;
  }

  ruiBaseLayerId              = MSYS_UINT_MAX;
  ruiBaseLayerIdMotionOnly    = MSYS_UINT_MAX;
  return Err::m_nOK;
}

ErrVal
H264AVCEncoder::getBaseLayerDataAvailability  ( IntFrame*&    pcFrame,
                                                IntFrame*&    pcResidual,
                                                MbDataCtrl*&  pcMbDataCtrl,
                                                Bool&         bConstrainedIPredBL,
                                                Bool&         bForCopyOnly,
                                                Int           iSpatialScalability,
                                                UInt          uiBaseLayerId,
                                                Int           iPoc,
                                                Bool          bMotion,
                                                Bool&         bBaseDataAvailable,
                                                PicType       ePicType,
                                                UInt					uiIdrPicId)//EIDR 0619
{
  ROF ( uiBaseLayerId < MAX_LAYERS );

  RNOK( m_apcMCTFEncoder[uiBaseLayerId]->getBaseLayerDataAvailability( pcFrame, 
                                                                       pcResidual, 
                                                                       pcMbDataCtrl, 
                                                                       bConstrainedIPredBL,
                                                                       bForCopyOnly, 
                                                                       iSpatialScalability, 
                                                                       iPoc, 
                                                                       bMotion,
                                                                       ePicType,
                                                                       uiIdrPicId) );//EIDR 0619

  bBaseDataAvailable = pcFrame && pcResidual && pcMbDataCtrl;

  return Err::m_nOK;
}
ErrVal
H264AVCEncoder::getBaseLayerData( IntFrame*&    pcFrame,
                                  IntFrame*&    pcResidual,
                                  MbDataCtrl*&  pcMbDataCtrl,
                                  MbDataCtrl*&  pcMbDataCtrlEL,		// ICU/ETRI FGS_MOT_USE
                                  Bool&         bConstrainedIPredBL,
                                  Bool&         bForCopyOnly,
                                  Int           iSpatialScalability,
                                  UInt          uiBaseLayerId,
                                  Int           iPoc,
                                  Bool				 bMotion,
                                  PicType      ePicType,
                                  UInt				 uiIdrPicId)//EIDR 0619
{
  ROF ( uiBaseLayerId < MAX_LAYERS );

  // ICU/ETRI FGS_MOT_USE
  RNOK( m_apcMCTFEncoder[uiBaseLayerId]->getBaseLayerData( pcFrame, 
                                                           pcResidual, 
                                                           pcMbDataCtrl,
                                                           pcMbDataCtrlEL,
                                                           bConstrainedIPredBL, 
                                                           bForCopyOnly, 
                                                           iSpatialScalability, 
                                                           iPoc, 
                                                           bMotion,
                                                           ePicType,
                                                           uiIdrPicId) );//EIDR 0619

  pcMbDataCtrlEL = pcMbDataCtrl;
  return Err::m_nOK;
}


ErrVal
H264AVCEncoder::getBaseLayerResidual( IntFrame*&      pcResidual,
                                      UInt            uiBaseLayerId)
{
  pcResidual = m_apcMCTFEncoder[uiBaseLayerId]->getBaseLayerResidual();  
  return Err::m_nOK;
}


ErrVal
H264AVCEncoder::getBaseLayerSH( SliceHeader*& rpcSliceHeader,
                                UInt          uiBaseLayerId,
                                Int           iPoc,
                                PicType      ePicType,
                                UInt					uiIdrPicId)//EIDR 0619
{
  ROF( uiBaseLayerId < MAX_LAYERS );

  RNOK( m_apcMCTFEncoder[uiBaseLayerId]->getBaseLayerSH( rpcSliceHeader, iPoc, ePicType,
                                                          uiIdrPicId) ); //EIDR 0619
  return Err::m_nOK;
}


UInt
H264AVCEncoder::getNewBits( UInt uiBaseLayerId )
{
  ROFRS( uiBaseLayerId < MAX_LAYERS, 0 );
  return m_apcMCTFEncoder[uiBaseLayerId]->getNewBits();
}


ErrVal
H264AVCEncoder::init( MCTFEncoder*      apcMCTFEncoder[MAX_LAYERS],
                      ParameterSetMng*  pcParameterSetMng,
                      PocCalculator*    pcPocCalculator,
                      NalUnitEncoder*   pcNalUnitEncoder,
                      ControlMngIf*     pcControlMng,
                      CodingParameter*  pcCodingParameter,
                      FrameMng*         pcFrameMng
                      // JVT-V068 {
                      ,StatBuf<Scheduler*, MAX_SCALABLE_LAYERS>* apcScheduler 
                      // JVT-V068 }
                      )
{
  ROT( NULL == apcMCTFEncoder );
  ROT( NULL == pcFrameMng );
  ROT( NULL == pcParameterSetMng );
  ROT( NULL == pcPocCalculator );
  ROT( NULL == pcNalUnitEncoder );
  ROT( NULL == pcControlMng );
  ROT( NULL == pcCodingParameter );

  m_pcFrameMng        = pcFrameMng;
  m_pcParameterSetMng = pcParameterSetMng;
  m_pcPocCalculator   = pcPocCalculator;
  m_pcNalUnitEncoder  = pcNalUnitEncoder;
  m_pcControlMng      = pcControlMng;
  m_pcCodingParameter = pcCodingParameter;

  // JVT-V068 {
  m_apcScheduler = apcScheduler;
  m_apcScheduler->clear();
  // JVT-V068 }

  UInt uiLayer;
  for( uiLayer = 0; uiLayer < m_pcCodingParameter->getNumberOfLayers(); uiLayer++ )
  {
    ROT( NULL == apcMCTFEncoder[uiLayer] );
    m_apcMCTFEncoder[uiLayer] = apcMCTFEncoder[uiLayer];
  }
  for( ; uiLayer < MAX_LAYERS; uiLayer++ )
  {
    m_apcMCTFEncoder[uiLayer] = 0;
  }

  m_cAccessUnitList.clear();

  return Err::m_nOK;
}


ErrVal
H264AVCEncoder::uninit()
{
  m_cUnWrittenSPS.clear();
  m_cUnWrittenPPS.clear();
  m_pcParameterSetMng           = NULL;
  m_pcPocCalculator             = NULL;
  m_pcNalUnitEncoder            = NULL;
  m_pcControlMng                = NULL;
  m_pcCodingParameter           = NULL;
  m_pcFrameMng                  = NULL;
  m_bInitDone                   = false;
  m_bVeryFirstCall              = true;
  m_bScalableSeiMessage         = true;
  m_bTraceEnable                = false;  

  for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
  {
    m_apcMCTFEncoder    [uiLayer] = NULL;
    m_acOrgPicBufferList[uiLayer]   .clear();
    m_acRecPicBufferList[uiLayer]   .clear();
  }

  m_cAccessUnitList.clear();

  return Err::m_nOK;
}



//{{Quality level estimation and modified truncation- JVTO044 and m12007
//France Telecom R&D-(nathalie.cammas@francetelecom.com)
ErrVal H264AVCEncoder::writeQualityLevelInfosSEI(ExtBinDataAccessor* pcExtBinDataAccessor, UInt* uiaQualityLevel, UInt *uiaDelta, UInt uiNumLevels, UInt uiLayer ) 
{
  //===== create message =====
  SEI::QualityLevelSEI* pcQualityLevelSEI;
  RNOK( SEI::QualityLevelSEI::create( pcQualityLevelSEI ) );

  //===== set message =====
  pcQualityLevelSEI->setNumLevel(uiNumLevels);
  pcQualityLevelSEI->setDependencyId(uiLayer);

  UInt ui;
  for(ui= 0; ui < uiNumLevels; ui++)
  {
    pcQualityLevelSEI->setQualityLevel(ui,uiaQualityLevel[ui]);
    pcQualityLevelSEI->setDeltaBytesRateOfLevel(ui,uiaDelta[ui]);
  }
  
  //===== write message =====
  UInt              uiBits = 0;
  SEI::MessageList  cSEIMessageList;
  cSEIMessageList.push_back                       ( pcQualityLevelSEI );
  RNOK( m_pcNalUnitEncoder  ->initNalUnit         ( pcExtBinDataAccessor ) );
  RNOK( m_pcNalUnitEncoder  ->write               ( cSEIMessageList ) );
  RNOK( m_pcNalUnitEncoder  ->closeNalUnit        ( uiBits ) );

  return Err::m_nOK;
}
//}}Quality level estimation and modified truncation- JVTO044 and m12007

// JVT-T073 {
ErrVal H264AVCEncoder::writeNestingSEIMessage( ExtBinDataAccessor* pcExtBinDataAccessor ) 
{
  SEI::ScalableNestingSei* pcScalableNestingSei;
  RNOK( SEI::ScalableNestingSei::create(pcScalableNestingSei) );

  //===== set message =====
  //may be changed here
  Bool bAllPicturesInAuFlag = 0;
    pcScalableNestingSei->setAllPicturesInAuFlag( bAllPicturesInAuFlag );
  if( bAllPicturesInAuFlag  == 0 )
  {
    UInt uiNumPictures;

    // assign value, may be changed here
    uiNumPictures = 1;
    ROT( uiNumPictures == 0 );
    UInt *uiDependencyId = new UInt[uiNumPictures];
    UInt *uiQualityLevel = new UInt[uiNumPictures];
        uiDependencyId[0] = 0;
        uiQualityLevel[0] = 0;

    pcScalableNestingSei->setNumPictures( uiNumPictures );
    for( UInt uiIndex = 0; uiIndex < uiNumPictures; uiIndex++ )
    {
      pcScalableNestingSei->setDependencyId( uiIndex, uiDependencyId[uiIndex] );
      pcScalableNestingSei->setQualityLevel( uiIndex, uiQualityLevel[uiIndex] );
    }
    delete uiDependencyId;
    delete uiQualityLevel;
  }
  //deal with the following SEI message in nesting SEI message
  //may be changed here, here take scene_info_sei as an example
  SEI::SceneInfoSei *pcSceneInfoSEI;
  RNOK( SEI::SceneInfoSei::create( pcSceneInfoSEI ) );

  UInt              uiBits = 0;
  SEI::MessageList  cSEIMessageList;
  cSEIMessageList.push_back                 ( pcScalableNestingSei );
  cSEIMessageList.push_back                 ( pcSceneInfoSEI );
  RNOK ( m_pcNalUnitEncoder->initNalUnit    ( pcExtBinDataAccessor ) );
  RNOK ( m_pcNalUnitEncoder->writeNesting   ( cSEIMessageList ) );
  RNOK ( m_pcNalUnitEncoder->closeNalUnit   ( uiBits ) );
  RNOK( m_apcMCTFEncoder[0]->addParameterSetBits ( uiBits+4*8 ) );

  RNOK( pcSceneInfoSEI->destroy() );
  RNOK( pcScalableNestingSei->destroy() );
  return Err::m_nOK;
}
// JVT-T073 }

ErrVal
H264AVCEncoder::xWriteScalableSEI( ExtBinDataAccessor* pcExtBinDataAccessor )
{
  //===== create message =====
  SEI::ScalableSei* pcScalableSEI;
  RNOK(SEI::ScalableSei::create(pcScalableSEI) );


  //===== set message =====
  UInt j = 0; //JVT-S036 lsj
  UInt i = 0;
  UInt uiInputLayers = m_pcCodingParameter->getNumberOfLayers ();
  UInt uiLayerNum = 0;	//total scalable layer numbers
  for ( i = 0; i < uiInputLayers; i++ )	//calculate total scalable layer numbers
  {
//bug-fix suffix{{
    //Bool bH264AVCCompatible = ( i == 0 && m_pcCodingParameter->getBaseLayerMode() > 0 );
    //Bool bH264AVCCompatible = ( i == 0 );
//bug-fix suffix}}
    //Bool bSubSeq            = ( i == 0 && m_pcCodingParameter->getBaseLayerMode() > 1 );

    LayerParameters& rcLayer = m_pcCodingParameter->getLayerParameters ( i );
    UInt uiTotalTempLevel = rcLayer.getDecompositionStages () - rcLayer.getNotCodedMCTFStages();
// *LMH(20060203): Fix Bug due to underflow (Replace)
    //UInt uiMinTempLevel   = ( !bH264AVCCompatible ||bSubSeq ) ? 0: max( 0, uiTotalTempLevel - 1 );
    UInt uiMinTempLevel   = 0; //bugfix replace
    UInt uiActTempLevel   = uiTotalTempLevel - uiMinTempLevel + 1;
    uiLayerNum += uiActTempLevel * rcLayer.getNumberOfQualityLevelsCGSSNR();

    pcScalableSEI->setROINum ( i, rcLayer.getNumROI() );
    pcScalableSEI->setROIID  ( i, rcLayer.getROIID() );
    pcScalableSEI->setSGID  ( i, rcLayer.getSGID() );
    pcScalableSEI->setSLID  ( i, rcLayer.getSLID() );
  }

  UInt uiNumLayersMinus1 = uiLayerNum - 1;
  pcScalableSEI->setNumLayersMinus1 ( uiNumLayersMinus1 );

  UInt uiNumScalableLayer = 0;
  for ( UInt uiCurrLayer = 0; uiCurrLayer < uiInputLayers; uiCurrLayer++)
  {
    LayerParameters& rcLayer = m_pcCodingParameter->getLayerParameters ( uiCurrLayer );
    UInt uiTotalTempLevel = rcLayer.getDecompositionStages () - rcLayer.getNotCodedMCTFStages() + 1;
    UInt uiTotalFGSLevel = rcLayer.getNumberOfQualityLevelsCGSSNR();
    UInt uiMinTempLevel     = 0; //bugfix replace

    for ( UInt uiCurrTempLevel = 0; uiCurrTempLevel < uiTotalTempLevel; uiCurrTempLevel++ )
    {
      for ( UInt uiCurrFGSLevel = 0; uiCurrFGSLevel < uiTotalFGSLevel; uiCurrFGSLevel++ )
      {
        if( uiCurrTempLevel >= uiMinTempLevel )
        {
          //Bool bSubPicLayerFlag = false;
          Bool bSubRegionLayerFlag = false;
          Bool bProfileLevelInfoPresentFlag = false;
          Bool bInitParameterSetsInfoPresentFlag = false;		//may be changed  //JVT-S036 lsj
          if( uiNumScalableLayer == 0 )
          {//JVT-S036 lsj
            bSubRegionLayerFlag = true;
            bProfileLevelInfoPresentFlag = true;
            bInitParameterSetsInfoPresentFlag = true;		
          }
          Bool bBitrateInfoPresentFlag = true;
          Bool bFrmRateInfoPresentFlag = true;//rcLayer.getInputFrameRate () > 0;
          Bool bFrmSizeInfoPresentFlag = true;
// BUG_FIX liuhui{
          Bool bLayerDependencyInfoPresentFlag = true;			//may be changed
// BUG_FIX liuhui}
          Bool bExactInterayerPredFlag = true;			//JVT-S036 lsj may be changed
          pcScalableSEI->setLayerId(uiNumScalableLayer, uiNumScalableLayer);

          UInt uiTempLevel = uiCurrTempLevel; //BUG_FIX_FT_01_2006
          UInt uiDependencyID = (m_pcCodingParameter->getCGSSNRRefinement() == 1 ? rcLayer.getLayerCGSSNR() : uiCurrLayer);
          UInt uiQualityLevel = (m_pcCodingParameter->getCGSSNRRefinement() == 1 ? rcLayer.getQualityLevelCGSSNR() + uiCurrFGSLevel : uiCurrFGSLevel);
          m_aaauiScalableLayerId[uiDependencyID][uiCurrTempLevel][uiQualityLevel] = uiNumScalableLayer;
      
          UInt uiSimplePriorityId = 0;
          Bool bDiscardableFlag  = false;
          if( uiCurrFGSLevel > 0 || rcLayer.isDiscardable() ) // bugfix replace
            bDiscardableFlag = true;
          pcScalableSEI->setSimplePriorityId(uiNumScalableLayer, uiSimplePriorityId);
          pcScalableSEI->setDiscardableFlag(uiNumScalableLayer, bDiscardableFlag);
          pcScalableSEI->setTemporalLevel(uiNumScalableLayer, uiTempLevel);
          pcScalableSEI->setDependencyId(uiNumScalableLayer, uiDependencyID);
          pcScalableSEI->setQualityLevel(uiNumScalableLayer, uiQualityLevel);				
   //JVT-S036 lsj end
          pcScalableSEI->setSubRegionLayerFlag(uiNumScalableLayer, bSubRegionLayerFlag);
          // JVT-S054 (REPLACE)
          //pcScalableSEI->setIroiSliceDivisionInfoPresentFlag(uiNumScalableLayer, bIroiSliceDivisionFlag); //JVT-S036 lsj
          pcScalableSEI->setIroiSliceDivisionInfoPresentFlag(uiNumScalableLayer, rcLayer.m_bSliceDivisionFlag);
          pcScalableSEI->setProfileLevelInfoPresentFlag(uiNumScalableLayer, bProfileLevelInfoPresentFlag);
          pcScalableSEI->setBitrateInfoPresentFlag(uiNumScalableLayer, bBitrateInfoPresentFlag);
          pcScalableSEI->setFrmRateInfoPresentFlag(uiNumScalableLayer, bFrmRateInfoPresentFlag);
          pcScalableSEI->setFrmSizeInfoPresentFlag(uiNumScalableLayer, bFrmSizeInfoPresentFlag);
          pcScalableSEI->setLayerDependencyInfoPresentFlag(uiNumScalableLayer, bLayerDependencyInfoPresentFlag);
          pcScalableSEI->setInitParameterSetsInfoPresentFlag(uiNumScalableLayer, bInitParameterSetsInfoPresentFlag);

          pcScalableSEI->setExactInterlayerPredFlag(uiNumScalableLayer, bExactInterayerPredFlag);//JVT-S036 lsj
          //JVT-W046 {
          pcScalableSEI->setAvcLayerConversionFlag(uiNumScalableLayer, 0); //default: 0, could be changed
          //JVT-W046 }
          //JVT-W047
          if(m_apcMCTFEncoder[0]->m_bOutputFlag)
            pcScalableSEI->setLayerOutputFlag(uiNumScalableLayer,true);
          else
            pcScalableSEI->setLayerOutputFlag(uiNumScalableLayer,false);
          //JVT-W047

          if(pcScalableSEI->getProfileLevelInfoPresentFlag(uiNumScalableLayer))
          {
            UInt uilayerProfileIdc = 0;	//may be changed
            Bool bLayerConstraintSet0Flag = false;	//may be changed
//bug-fix suffix{{
//					  Bool bH264AVCCompatibleTmp  = m_pcCodingParameter->getBaseLayerMode() > 0 && uiCurrLayer == 0;
            Bool bH264AVCCompatibleTmp  = ( uiCurrLayer == 0 );
//bug-fix suffix}}
            Bool bLayerConstraintSet1Flag = ( bH264AVCCompatibleTmp ? 1 : 0 );	//may be changed
            Bool bLayerConstraintSet2Flag = false;	//may be changed
            Bool bLayerConstraintSet3Flag = false;	//may be changed
            UInt uiLayerLevelIdc = 0;		//may be changed
            //JVT-W051 {
            uilayerProfileIdc = m_uiProfileIdc[uiCurrLayer];
            uiLayerLevelIdc = m_uiLevelIdc[uiCurrLayer];
            bLayerConstraintSet0Flag = m_bConstraint0Flag[uiCurrLayer];
            bLayerConstraintSet1Flag = m_bConstraint1Flag[uiCurrLayer];
            bLayerConstraintSet2Flag = m_bConstraint2Flag[uiCurrLayer];
            bLayerConstraintSet3Flag = m_bConstraint3Flag[uiCurrLayer];
            Int32 uilayerProfileLevelIdc = 0;
            uilayerProfileLevelIdc += uilayerProfileIdc;
            uilayerProfileLevelIdc = uilayerProfileLevelIdc << 1;
            uilayerProfileLevelIdc += bLayerConstraintSet0Flag;
            uilayerProfileLevelIdc = uilayerProfileLevelIdc << 1;
            uilayerProfileLevelIdc += bLayerConstraintSet1Flag;
            uilayerProfileLevelIdc = uilayerProfileLevelIdc << 1;
            uilayerProfileLevelIdc += bLayerConstraintSet2Flag;
            uilayerProfileLevelIdc = uilayerProfileLevelIdc << 1;
            uilayerProfileLevelIdc += bLayerConstraintSet3Flag;
            uilayerProfileLevelIdc = uilayerProfileLevelIdc << 4;
            uilayerProfileLevelIdc = uilayerProfileLevelIdc << 8;
            uilayerProfileLevelIdc += uiLayerLevelIdc;
            //JVT-W051 }
            pcScalableSEI->setLayerProfileIdc(uiNumScalableLayer, uilayerProfileIdc);
            pcScalableSEI->setLayerConstraintSet0Flag(uiNumScalableLayer, bLayerConstraintSet0Flag);
            pcScalableSEI->setLayerConstraintSet1Flag(uiNumScalableLayer, bLayerConstraintSet1Flag);
            pcScalableSEI->setLayerConstraintSet2Flag(uiNumScalableLayer, bLayerConstraintSet2Flag);
            pcScalableSEI->setLayerConstraintSet3Flag(uiNumScalableLayer, bLayerConstraintSet3Flag);
            pcScalableSEI->setLayerLevelIdc(uiNumScalableLayer, uiLayerLevelIdc);
          }
          else
          {//JVT-S036 lsj
            UInt bProfileLevelInfoSrcLayerIdDelta = 0;  //may be changed

            pcScalableSEI->setProfileLevelInfoSrcLayerIdDelta(uiNumScalableLayer, bProfileLevelInfoSrcLayerIdDelta);
          }

          if(pcScalableSEI->getBitrateInfoPresentFlag(uiNumScalableLayer))
          {
            UInt uiAvgBitrate = (UInt)( m_aaadSingleLayerBitrate[uiDependencyID][uiCurrTempLevel][uiQualityLevel]+0.5 );
          //JVT-S036 lsj start
            UInt uiMaxBitrateLayer = 0;	//should be changed
            UInt uiMaxBitrateDecodedPicture = 0;	//should be changed
            UInt uiMaxBitrateCalcWindow = 0; //should be changed

            pcScalableSEI->setAvgBitrate(uiNumScalableLayer, uiAvgBitrate);
            pcScalableSEI->setMaxBitrateLayer(uiNumScalableLayer, uiMaxBitrateLayer);
            pcScalableSEI->setMaxBitrateDecodedPicture(uiNumScalableLayer, uiMaxBitrateDecodedPicture);
            pcScalableSEI->setMaxBitrateCalcWindow(uiNumScalableLayer, uiMaxBitrateCalcWindow);
            //JVT-S036 lsj end
          }

          if(pcScalableSEI->getFrmRateInfoPresentFlag(uiNumScalableLayer))
          {
            UInt uiConstantFrmRateIdc = 0;
            ROT( m_aaadFinalFramerate[uiDependencyID][uiCurrTempLevel][uiQualityLevel] == 0.0 );
            UInt uiAvgFrmRate = (UInt)floor( 256.0 * m_aaadFinalFramerate[uiDependencyID][uiCurrTempLevel][uiQualityLevel] + 0.5 );

            pcScalableSEI->setConstantFrmRateIdc(uiNumScalableLayer, uiConstantFrmRateIdc);
            pcScalableSEI->setAvgFrmRate(uiNumScalableLayer, uiAvgFrmRate);
          }
          else
          {//JVT-S036 lsj
            UInt  bFrmRateInfoSrcLayerIdDelta = 0;  //may be changed

            pcScalableSEI->setFrmRateInfoSrcLayerIdDelta(uiNumScalableLayer, bFrmRateInfoSrcLayerIdDelta);
          }

          if(pcScalableSEI->getFrmSizeInfoPresentFlag(uiNumScalableLayer))
          {
            UInt uiFrmWidthInMbsMinus1 = rcLayer.getFrameWidth()/16 - 1;
            UInt uiFrmHeightInMbsMinus1 = rcLayer.getFrameHeight()/16 - 1;

            pcScalableSEI->setFrmWidthInMbsMinus1(uiNumScalableLayer, uiFrmWidthInMbsMinus1);
            pcScalableSEI->setFrmHeightInMbsMinus1(uiNumScalableLayer, uiFrmHeightInMbsMinus1);
          }
          else
          {//JVT-S036 lsj
            UInt  bFrmSizeInfoSrcLayerIdDelta = 0;  //may be changed

            pcScalableSEI->setFrmSizeInfoSrcLayerIdDelta(uiNumScalableLayer, bFrmSizeInfoSrcLayerIdDelta);
          }

          if(pcScalableSEI->getSubRegionLayerFlag(uiNumScalableLayer))
          {
            UInt uiBaseRegionLayerId = 0;
            Bool bDynamicRectFlag = false;

            pcScalableSEI->setBaseRegionLayerId(uiNumScalableLayer, uiBaseRegionLayerId);
            pcScalableSEI->setDynamicRectFlag(uiNumScalableLayer, bDynamicRectFlag);
            if(pcScalableSEI->getDynamicRectFlag(uiNumScalableLayer))
            {
              UInt uiHorizontalOffset = 0;
              UInt uiVerticalOffset = 0;
              UInt uiRegionWidth = 0;
              UInt uiRegionHeight = 0;
              pcScalableSEI->setHorizontalOffset(uiNumScalableLayer, uiHorizontalOffset);
              pcScalableSEI->setVerticalOffset(uiNumScalableLayer, uiVerticalOffset);
              pcScalableSEI->setRegionWidth(uiNumScalableLayer, uiRegionWidth);
              pcScalableSEI->setRegionHeight(uiNumScalableLayer, uiRegionHeight);
            }
          }
          else
          {//JVT-S036 lsj
            UInt  bSubRegionInfoSrcLayerIdDelta = 0; //may be changed

            pcScalableSEI->setSubRegionInfoSrcLayerIdDelta(uiNumScalableLayer, bSubRegionInfoSrcLayerIdDelta);
          }

        //JVT-S036 lsj start
          if( pcScalableSEI->getSubPicLayerFlag( uiNumScalableLayer ) )
          {
            UInt RoiId = 1;//should be changed
            pcScalableSEI->setRoiId( uiNumScalableLayer, RoiId );
          }
          if( pcScalableSEI->getIroiSliceDivisionInfoPresentFlag( uiNumScalableLayer ) )
          {
            pcScalableSEI->setIroiSliceDivisionType( uiNumScalableLayer, rcLayer.m_uiSliceDivisionType );
            if (rcLayer.m_uiSliceDivisionType == 0)
            {
              pcScalableSEI->setGridSliceWidthInMbsMinus1( uiNumScalableLayer, rcLayer.m_puiGridSliceWidthInMbsMinus1[0] );
              pcScalableSEI->setGridSliceHeightInMbsMinus1( uiNumScalableLayer, rcLayer.m_puiGridSliceHeightInMbsMinus1[0] );
            }
            else if (rcLayer.m_uiSliceDivisionType == 1)
            {
              pcScalableSEI->setNumSliceMinus1( uiNumScalableLayer, rcLayer.m_uiNumSliceMinus1 );
              for ( j = 0; j <= rcLayer.m_uiNumSliceMinus1; j++ )
              {
                pcScalableSEI->setFirstMbInSlice( uiNumScalableLayer, j, rcLayer.m_puiFirstMbInSlice[j] );
                pcScalableSEI->setSliceWidthInMbsMinus1( uiNumScalableLayer, j, rcLayer.m_puiGridSliceWidthInMbsMinus1[j] );
                pcScalableSEI->setSliceHeightInMbsMinus1( uiNumScalableLayer, j, rcLayer.m_puiGridSliceHeightInMbsMinus1[j] );
              }
            }
            else if (rcLayer.m_uiSliceDivisionType == 2)
            {
              pcScalableSEI->setNumSliceMinus1( uiNumScalableLayer, rcLayer.m_uiNumSliceMinus1 );
              UInt uiFrameHeightInMb = pcScalableSEI->getFrmHeightInMbsMinus1( uiNumScalableLayer ) + 1;
              UInt uiFrameWidthInMb  = pcScalableSEI->getFrmWidthInMbsMinus1(uiNumScalableLayer ) + 1;
              UInt uiPicSizeInMbs = uiFrameHeightInMb * uiFrameWidthInMb;
              for ( j = 0; j < uiPicSizeInMbs; j++)
              {
                pcScalableSEI->setSliceId( uiNumScalableLayer, j, rcLayer.m_puiSliceId[j] );
              }
            }
          }
        //JVT-S036 lsj end

          if(pcScalableSEI->getLayerDependencyInfoPresentFlag(uiNumScalableLayer))
          {
// BUG_FIX liuhui{
          {
            UInt uiDelta;
            UInt uiQL = (m_pcCodingParameter->getCGSSNRRefinement() == 1 ? rcLayer.getQualityLevelCGSSNR() + uiCurrFGSLevel : uiCurrFGSLevel);
            UInt uiL = (m_pcCodingParameter->getCGSSNRRefinement() == 1 ? rcLayer.getLayerCGSSNR() : uiCurrLayer);
            if( uiQL ) // FGS layer, Q != 0
            {
              uiDelta = uiNumScalableLayer - getScalableLayerId( uiL, uiCurrTempLevel, uiQL-1 );
              pcScalableSEI->setDirectlyDependentLayerIdDeltaMinus1( uiNumScalableLayer, 0, uiDelta );//JVT-S036 lsj
              pcScalableSEI->setNumDirectlyDependentLayers(uiNumScalableLayer, 1 );
              if( uiCurrTempLevel- uiMinTempLevel ) // T != 0
              {
                uiDelta = uiNumScalableLayer - getScalableLayerId( uiL, uiCurrTempLevel-1, uiQL );
                pcScalableSEI->setDirectlyDependentLayerIdDeltaMinus1( uiNumScalableLayer, 1, uiDelta );//JVT-S036 lsj
                pcScalableSEI->setNumDirectlyDependentLayers(uiNumScalableLayer, 2 );
              }
            }
            else if( ( uiCurrTempLevel- uiMinTempLevel ) ) // Q = 0, T != 0					    
            {
              uiDelta = uiNumScalableLayer - getScalableLayerId( uiL, uiCurrTempLevel-1, uiQL );
              pcScalableSEI->setDirectlyDependentLayerIdDeltaMinus1( uiNumScalableLayer, 0, uiDelta ); //JVT-S036 lsj
              pcScalableSEI->setNumDirectlyDependentLayers( uiNumScalableLayer, 1 );
              if( uiL ) // D != 0, T != 0, Q = 0
              {
                UInt uiBaseLayerCGSSNRId = (m_pcCodingParameter->getCGSSNRRefinement() == 1 ? rcLayer.getBaseLayerCGSSNR() : rcLayer.getBaseLayerId());
                UInt uiBaseQualityLevel = (m_pcCodingParameter->getCGSSNRRefinement() == 1 ? rcLayer.getBaseQualityLevelCGSSNR() : rcLayer.getBaseQualityLevel());
                uiBaseQualityLevel = (m_pcCodingParameter->getCGSSNRRefinement() == 1 ? uiBaseQualityLevel : 0 );
  //bugfix delete
                {
                  if( MSYS_UINT_MAX != getScalableLayerId( uiBaseLayerCGSSNRId, uiCurrTempLevel, uiBaseQualityLevel ) )
                  {
                    uiDelta = uiNumScalableLayer - getScalableLayerId( uiBaseLayerCGSSNRId, uiCurrTempLevel, uiBaseQualityLevel );
                    pcScalableSEI->setDirectlyDependentLayerIdDeltaMinus1( uiNumScalableLayer, 1, uiDelta ); //JVT-S036 lsj
                    pcScalableSEI->setNumDirectlyDependentLayers( uiNumScalableLayer, 2 );
                  }
                }
              }
            }
            else if ( uiL ) // D != 0,T = 0, Q = 0
            {
              UInt uiBaseLayerCGSSNRId = (m_pcCodingParameter->getCGSSNRRefinement() == 1 ? rcLayer.getBaseLayerCGSSNR() : rcLayer.getBaseLayerId());
              UInt uiBaseQualityLevel = (m_pcCodingParameter->getCGSSNRRefinement() == 1 ? rcLayer.getBaseQualityLevelCGSSNR() : rcLayer.getBaseQualityLevel());
              uiBaseQualityLevel = (m_pcCodingParameter->getCGSSNRRefinement() == 1 ? uiBaseQualityLevel : 0 );
//bugfix delete
              uiDelta = uiNumScalableLayer - getScalableLayerId( uiBaseLayerCGSSNRId, uiCurrTempLevel, uiBaseQualityLevel );
              pcScalableSEI->setDirectlyDependentLayerIdDeltaMinus1( uiNumScalableLayer, 0, uiDelta ); //JVT-S036 lsj
              pcScalableSEI->setNumDirectlyDependentLayers( uiNumScalableLayer, 1 );
            }
            else // base layer, no dependency layers
            {
              pcScalableSEI->setNumDirectlyDependentLayers( uiNumScalableLayer, 0 );
            }
          } 

// BUG_FIX liuhui}
          }
          else
          {//JVT-S036 lsj
            UInt uiLayerDependencyInfoSrcLayerIdDelta = 0; //may be changed

            pcScalableSEI->setLayerDependencyInfoSrcLayerIdDelta( uiNumScalableLayer, uiLayerDependencyInfoSrcLayerIdDelta);
          }

          if(pcScalableSEI->getInitParameterSetsInfoPresentFlag(uiNumScalableLayer))
          {
            UInt uiNumInitSPSMinus1 = 0;	//should be changed
            UInt uiNumInitPPSMinus1 = 0;	//should be changed
            pcScalableSEI->setNumInitSeqParameterSetMinus1(uiNumScalableLayer, uiNumInitSPSMinus1);
            pcScalableSEI->setNumInitPicParameterSetMinus1(uiNumScalableLayer, uiNumInitPPSMinus1);
            for( j = 0; j <= pcScalableSEI->getNumInitSPSMinus1(uiNumScalableLayer); j++)
            {
              UInt uiDelta = 0; //should be changed
              pcScalableSEI->setInitSeqParameterSetIdDelta( uiNumScalableLayer, j, uiDelta );
            }
            for( j = 0; j <= pcScalableSEI->getNumInitPPSMinus1(uiNumScalableLayer); j++)
            {
              UInt uiDelta = 0; //should be changed
              pcScalableSEI->setInitPicParameterSetIdDelta( uiNumScalableLayer, j, uiDelta );
            }
          }
          else
          {//JVT-S036 lsj
            UInt bInitParameterSetsInfoSrcLayerIdDelta = 0;  //may be changed

            pcScalableSEI->setInitParameterSetsInfoSrcLayerIdDelta( uiNumScalableLayer, bInitParameterSetsInfoSrcLayerIdDelta );
          }

          //JVT-W051 {
          if ( pcScalableSEI->getBitstreamRestrictionFlag ( uiNumScalableLayer ) )
          {
            UInt uiMaxDecFrameBuffering = m_pcCodingParameter->getDPBSize();//should be changed
            UInt uiNumReorderFrames = pcScalableSEI->getMaxDecFrameBuffering( uiNumScalableLayer );//should be changed
            pcScalableSEI->setMaxDecFrameBuffering( uiNumScalableLayer, uiMaxDecFrameBuffering );					  
            pcScalableSEI->setNumReorderFrames( uiNumScalableLayer, uiNumReorderFrames );
          }
          else
          {
            UInt uiDpbInfoSrcLayerIdDelta = 0;//should be changed
            pcScalableSEI->setBitstreamRestrictionSrcLayerIdDelta( uiNumScalableLayer, uiDpbInfoSrcLayerIdDelta );
          }
          //JVT-W051 }
          uiNumScalableLayer++;
        }
      }
    }

  }
  //JVT-W051 {
  pcScalableSEI->setQualityLayerInfoPresentFlag(true);
  if ( pcScalableSEI->getQualityLayerInfoPresentFlag() )
  {
    UInt uiQlNumdIdMinus1 = m_pcCodingParameter->getNumberOfLayers ()-1;
    pcScalableSEI->setQlNumdIdMinus1( uiQlNumdIdMinus1 );
    for ( i=0; i <= pcScalableSEI->getQlNumdIdMinus1(); i++ )
    {
      UInt uiQlDependencyId = i;//could be changed
      pcScalableSEI->setQlDependencyId(i, uiQlDependencyId);
      pcScalableSEI->setQlNumMinus1(i, 0);

      for (j=0; j <= pcScalableSEI->getQlNumMinus1(i); j++)
      {
        UInt uiQlId = j;//could be changed
        pcScalableSEI->setQlId(i,j,uiQlId);
        Int32 uilayerProfileLevelIdc = 0;
        {
          UInt uilayerProfileIdc = m_uiProfileIdc[uiQlDependencyId];
          UInt uiLayerLevelIdc = m_uiLevelIdc[uiQlDependencyId];
          Bool bLayerConstraintSet0Flag = m_bConstraint0Flag[uiQlDependencyId];
          Bool bLayerConstraintSet1Flag = m_bConstraint1Flag[uiQlDependencyId];
          Bool bLayerConstraintSet2Flag = m_bConstraint2Flag[uiQlDependencyId];
          Bool bLayerConstraintSet3Flag = m_bConstraint3Flag[uiQlDependencyId];					
          uilayerProfileLevelIdc += uilayerProfileIdc;
          uilayerProfileLevelIdc = uilayerProfileLevelIdc << 1;
          uilayerProfileLevelIdc += bLayerConstraintSet0Flag;
          uilayerProfileLevelIdc = uilayerProfileLevelIdc << 1;
          uilayerProfileLevelIdc += bLayerConstraintSet1Flag;
          uilayerProfileLevelIdc = uilayerProfileLevelIdc << 1;
          uilayerProfileLevelIdc += bLayerConstraintSet2Flag;
          uilayerProfileLevelIdc = uilayerProfileLevelIdc << 1;
          uilayerProfileLevelIdc += bLayerConstraintSet3Flag;
          uilayerProfileLevelIdc = uilayerProfileLevelIdc << 4;
          uilayerProfileLevelIdc = uilayerProfileLevelIdc << 8;
          uilayerProfileLevelIdc += uiLayerLevelIdc;
        }
        pcScalableSEI->setQlProfileLevelIdx(i,j,uilayerProfileLevelIdc);


        pcScalableSEI->setQlAvgBitrate(i,j,(UInt)(m_adAvgBitrate[uiQlDependencyId]+0.5));
        pcScalableSEI->setQlMaxBitrate(i,j,(UInt)(m_aadMaxBitrate[uiQlDependencyId][uiQlId]+0.5));
      }
    }
  }
  //JVT-W051 }
  //JVT-W053
  const char *ucTemp = "http://svc.com";
  strcpy( pcScalableSEI->getPriorityIdSetUri(), ucTemp);
  //JVT-W053

  UInt              uiBits = 0;
  SEI::MessageList  cSEIMessageList;
  cSEIMessageList.push_back                       ( pcScalableSEI );
  RNOK( m_pcNalUnitEncoder  ->initNalUnit         ( pcExtBinDataAccessor ) );
  RNOK( m_pcNalUnitEncoder  ->write               ( cSEIMessageList ) );
  RNOK( m_pcNalUnitEncoder  ->closeNalUnit        ( uiBits ) );
  RNOK( m_apcMCTFEncoder[0] ->addParameterSetBits ( uiBits+4*8 ) );

  return Err::m_nOK;

}



// JVT-S080 LMI {
ErrVal
H264AVCEncoder::xWriteScalableSEILayersNotPresent( ExtBinDataAccessor* pcExtBinDataAccessor, UInt uiNumLayers, UInt* uiLayerId)
{
  UInt i, uiBits = 0;
  SEI::MessageList  cSEIMessageList;
  SEI::ScalableSeiLayersNotPresent* pcScalableSeiLayersNotPresent;
  RNOK(SEI::ScalableSeiLayersNotPresent::create(pcScalableSeiLayersNotPresent) );
  pcScalableSeiLayersNotPresent->setNumLayers( uiNumLayers );
  for (i=0; i < uiNumLayers; i++)
  pcScalableSeiLayersNotPresent->setLayerId( i, uiLayerId[i] );
  cSEIMessageList.push_back                       ( pcScalableSeiLayersNotPresent );
  RNOK( m_pcNalUnitEncoder  ->initNalUnit         ( pcExtBinDataAccessor ) );
  RNOK( m_pcNalUnitEncoder  ->write               ( cSEIMessageList ) );
  RNOK( m_pcNalUnitEncoder  ->closeNalUnit        ( uiBits ) );
  RNOK( m_apcMCTFEncoder[0] ->addParameterSetBits ( uiBits+4*8 ) );

  return Err::m_nOK;
}

ErrVal
H264AVCEncoder::xWriteScalableSEIDependencyChange( ExtBinDataAccessor* pcExtBinDataAccessor, UInt uiNumLayers, UInt* uiLayerId, Bool* pbLayerDependencyInfoPresentFlag, 
                          UInt* uiNumDirectDependentLayers, UInt** puiDirectDependentLayerIdDeltaMinus1, UInt* puiLayerDependencyInfoSrcLayerIdDeltaMinus1)
{
  UInt uiBits = 0;
  SEI::MessageList  cSEIMessageList;
  SEI::ScalableSeiDependencyChange* pcScalableSeiDependencyChange;
  RNOK(SEI::ScalableSeiDependencyChange::create(pcScalableSeiDependencyChange) );
  pcScalableSeiDependencyChange->setNumLayersMinus1(uiNumLayers-1);
    UInt uiLayer, uiDirectLayer;

  for( uiLayer = 0; uiLayer < uiNumLayers; uiLayer++ )
  {
    pcScalableSeiDependencyChange->setLayerId( uiLayer, uiLayerId[uiLayer]);
    pcScalableSeiDependencyChange->setLayerDependencyInfoPresentFlag( uiLayer, pbLayerDependencyInfoPresentFlag[uiLayer] );
    if ( pcScalableSeiDependencyChange->getLayerDependencyInfoPresentFlag( uiLayer ) )
    {
          pcScalableSeiDependencyChange->setNumDirectDependentLayers( uiLayer, uiNumDirectDependentLayers[uiLayer] );
      for ( uiDirectLayer = 0; uiDirectLayer < pcScalableSeiDependencyChange->getNumDirectDependentLayers( uiLayer ); uiDirectLayer++)
              pcScalableSeiDependencyChange->setDirectDependentLayerIdDeltaMinus1( uiLayer, uiDirectLayer,  puiDirectDependentLayerIdDeltaMinus1[uiLayer][uiDirectLayer] );
    }
    else
            pcScalableSeiDependencyChange->setLayerDependencyInfoSrcLayerIdDeltaMinus1( uiLayer, puiLayerDependencyInfoSrcLayerIdDeltaMinus1[uiLayer] );
  }


  cSEIMessageList.push_back                       ( pcScalableSeiDependencyChange );
  RNOK( m_pcNalUnitEncoder  ->initNalUnit         ( pcExtBinDataAccessor ) );
  RNOK( m_pcNalUnitEncoder  ->write               ( cSEIMessageList ) );
  RNOK( m_pcNalUnitEncoder  ->closeNalUnit        ( uiBits ) );
  RNOK( m_apcMCTFEncoder[0] ->addParameterSetBits ( uiBits+4*8 ) );

  return Err::m_nOK;
}
//  JVT-S080 LMI }

ErrVal
H264AVCEncoder::xWriteSubPicSEI ( ExtBinDataAccessor* pcExtBinDataAccessor )
{
  SEI::SubPicSei* pcSubPicSEI;
  RNOK( SEI::SubPicSei::create( pcSubPicSEI ) );

  //===== set message =====
  UInt uiScalableLayerId = 0;	//should be changed
  pcSubPicSEI->setLayerId( uiScalableLayerId );
  
  //===== write message =====
  UInt              uiBits = 0;
  SEI::MessageList  cSEIMessageList;
  cSEIMessageList.push_back                       ( pcSubPicSEI );
  RNOK( m_pcNalUnitEncoder  ->initNalUnit         ( pcExtBinDataAccessor ) );
  RNOK( m_pcNalUnitEncoder  ->write               ( cSEIMessageList ) );
  RNOK( m_pcNalUnitEncoder  ->closeNalUnit        ( uiBits ) );

  return Err::m_nOK;
}


ErrVal
H264AVCEncoder::xWriteSubPicSEI ( ExtBinDataAccessor* pcExtBinDataAccessor, UInt layer_id )
{
  SEI::SubPicSei* pcSubPicSEI;
  RNOK( SEI::SubPicSei::create( pcSubPicSEI ) );

  //===== set message =====
  pcSubPicSEI->setLayerId( layer_id );
  
  //===== write message =====
  UInt              uiBits = 0;
  SEI::MessageList  cSEIMessageList;
  cSEIMessageList.push_back                       ( pcSubPicSEI );
  RNOK( m_pcNalUnitEncoder  ->initNalUnit         ( pcExtBinDataAccessor ) );
  RNOK( m_pcNalUnitEncoder  ->write               ( cSEIMessageList ) );
  RNOK( m_pcNalUnitEncoder  ->closeNalUnit        ( uiBits ) );

  return Err::m_nOK;
}

// Scalable SEI for ROI ICU/ETRI
ErrVal
H264AVCEncoder::xWriteMotionSEI( ExtBinDataAccessor* pcExtBinDataAccessor, UInt sg_id ) 
{
  //===== create message =====
  SEI::MotionSEI* pcMotionSEI;
  RNOK( SEI::MotionSEI::create( pcMotionSEI) );

  pcMotionSEI->setSliceGroupId(sg_id);


  //===== write message =====
  UInt              uiBits = 0;
  SEI::MessageList  cSEIMessageList;
  cSEIMessageList.push_back                       ( pcMotionSEI);
  RNOK( m_pcNalUnitEncoder  ->initNalUnit         ( pcExtBinDataAccessor ) );
  RNOK( m_pcNalUnitEncoder  ->write               ( cSEIMessageList ) );
  RNOK( m_pcNalUnitEncoder  ->closeNalUnit        ( uiBits ) );

  return Err::m_nOK;
}

ErrVal
H264AVCEncoder::writeParameterSets( ExtBinDataAccessor* pcExtBinDataAccessor, 
                                    // JVT-V068 {
                                    SequenceParameterSet *pcAVCSPS, 
                                    // JVT-V068 }
                                    Bool &rbMoreSets )
{
  if( m_bVeryFirstCall )
  {
    m_bVeryFirstCall = false;

    RNOK( xInitParameterSets() );
    if( m_bScalableSeiMessage )
      RNOK( xWriteScalableSEI( pcExtBinDataAccessor ) );

    LayerParameters& rcLayer = m_pcCodingParameter->getLayerParameters ( 0 );
    if (0 < rcLayer.getNumROI())
      m_bWrteROISEI = true;
    else
      m_bWrteROISEI = false;
    m_loop_roi_sei=0;

    return Err::m_nOK;
  }
  else
    m_bScalableSeiMessage = true;

  UInt uiNumLayer = m_pcCodingParameter->getNumberOfLayers();
  
  if(m_bWrteROISEI)
  {
  LayerParameters& rcLayer = m_pcCodingParameter->getLayerParameters ( m_loop_roi_sei/2 );
  {
    if(((m_loop_roi_sei+1)/2) >= uiNumLayer )
    {
    m_bWrteROISEI = false;
    }

    if(m_loop_roi_sei%2)
    {			
    RNOK( xWriteMotionSEI( pcExtBinDataAccessor,rcLayer.getSGID()[0] ) );    m_loop_roi_sei++; return Err::m_nOK;
    }
    else
    {
      RNOK( xWriteSubPicSEI( pcExtBinDataAccessor, rcLayer.getSLID()[0] ) );    m_loop_roi_sei++; return Err::m_nOK;
    }
    }		
  }
    
  UInt uiBits;

  if( ! m_cUnWrittenSPS.empty() )
  {
    RNOK( m_pcNalUnitEncoder->initNalUnit( pcExtBinDataAccessor ) );
    SequenceParameterSet& rcSPS = *m_cUnWrittenSPS.front();
    
    if( rcSPS.getMbAdaptiveFrameFieldFlag() )
    {
      rcSPS.setFrameMbsOnlyFlag( false );
    }
    
    RNOK( m_pcNalUnitEncoder->write( rcSPS ) );
    RNOK( m_pcNalUnitEncoder->closeNalUnit( uiBits ) );

    if( m_pcCodingParameter->getNumberOfLayers() )
    {
      UInt  uiLayer = rcSPS.getLayerId();
      UInt  uiSize  = pcExtBinDataAccessor->size();
      RNOK( m_apcMCTFEncoder[uiLayer]->addParameterSetBits( 8*(uiSize+4) ) );
    }
    
// JVT-V068 {
    if ( (m_pcCodingParameter->getEnableNalHRD() || m_pcCodingParameter->getEnableVclHRD()) && rcSPS.getProfileIdc() != SCALABLE_PROFILE )
    {
      pcAVCSPS = m_cUnWrittenSPS.front();
    }
// JVT-V068 }

    m_cUnWrittenSPS.pop_front();
  }
  else
  {
    if( ! m_cUnWrittenPPS.empty() )
    {
      RNOK( m_pcNalUnitEncoder->initNalUnit( pcExtBinDataAccessor ) );
      PictureParameterSet& rcPPS = *m_cUnWrittenPPS.front();
      RNOK( m_pcNalUnitEncoder->write( rcPPS ) )
      RNOK( m_pcNalUnitEncoder->closeNalUnit( uiBits ) );

      if( m_pcCodingParameter->getNumberOfLayers() )
      {
        UInt  uiSPSId = rcPPS.getSeqParameterSetId();
        SequenceParameterSet* pcSPS;
        RNOK( m_pcParameterSetMng->get( pcSPS, uiSPSId ) );

        UInt  uiLayer = pcSPS->getLayerId();
        UInt  uiSize  = pcExtBinDataAccessor->size();
        RNOK( m_apcMCTFEncoder[uiLayer]->addParameterSetBits( 8*(uiSize+4) ) );
      }
      
      m_cUnWrittenPPS.pop_front();
    }
    else
    {
      AF();
      rbMoreSets = false;
      return Err::m_nERR;
    }
  }

  rbMoreSets = !(m_cUnWrittenSPS.empty() && m_cUnWrittenPPS.empty());
  return Err::m_nOK;
}



ErrVal
H264AVCEncoder::process( ExtBinDataAccessorList&  rcExtBinDataAccessorList,
                         PicBuffer*               apcOriginalPicBuffer    [MAX_LAYERS],
                         PicBuffer*               apcReconstructPicBuffer [MAX_LAYERS],
                         PicBufferList*           apcPicBufferOutputList,
                         PicBufferList*           apcPicBufferUnusedList )
{
  UInt  uiHighestLayer  = m_pcCodingParameter->getNumberOfLayers() - 1;
  UInt  uiTargetSize    = ( 1 << m_pcCodingParameter->getLayerParameters(uiHighestLayer).getDecompositionStages() ) + ( m_apcMCTFEncoder[uiHighestLayer]->firstGOPCoded() ? 0 : 1 );

  //===== fill lists =====
  for( UInt uiLayer = 0; uiLayer <= uiHighestLayer; uiLayer++ )
  {
    if( apcOriginalPicBuffer[ uiLayer ] )
    {
      ROF( apcReconstructPicBuffer[ uiLayer ] );
      m_acOrgPicBufferList  [ uiLayer ].push_back( apcOriginalPicBuffer   [ uiLayer ] );
      m_acRecPicBufferList  [ uiLayer ].push_back( apcReconstructPicBuffer[ uiLayer ] );
    }
    else if( apcReconstructPicBuffer[ uiLayer ] )
    {
      apcPicBufferUnusedList[ uiLayer ].push_back( apcReconstructPicBuffer[ uiLayer ] );
    }
  }

  //===== encoding of GOP =====
  ROT( m_acOrgPicBufferList[uiHighestLayer].size() >  uiTargetSize );
  if ( m_acOrgPicBufferList[uiHighestLayer].size() == uiTargetSize )
  {
    RNOK( xProcessGOP( apcPicBufferOutputList, apcPicBufferUnusedList ) );
  }

  //===== update data accessor list =====
  m_cAccessUnitList.emptyNALULists( rcExtBinDataAccessorList );

  return Err::m_nOK;
}


ErrVal
H264AVCEncoder::finish( ExtBinDataAccessorList&  rcExtBinDataAccessorList,
                        PicBufferList*           apcPicBufferOutputList,
                        PicBufferList*           apcPicBufferUnusedList,
                        UInt&                    ruiNumCodedFrames,
                        Double&                  rdHighestLayerOutputRate )
{
  //===== encode GOP =====
  RNOK( xProcessGOP( apcPicBufferOutputList, apcPicBufferUnusedList ) );

  //===== update data accessor list =====
  m_cAccessUnitList.emptyNALULists( rcExtBinDataAccessorList );

  //===== finish encoding =====
  for( UInt uiLayer = 0; uiLayer < m_pcCodingParameter->getNumberOfLayers(); uiLayer++ )
  {
    RNOK( m_apcMCTFEncoder[uiLayer]->finish           ( ruiNumCodedFrames, rdHighestLayerOutputRate, m_aaadFinalFramerate, m_aaauidSeqBits ) );
    RNOK( m_apcMCTFEncoder[uiLayer]->SingleLayerFinish( m_aaauidSeqBits, m_aaadSingleLayerBitrate ) );
    //JVT-W051 {
    m_adAvgBitrate[uiLayer] = m_apcMCTFEncoder[uiLayer]->m_dAvgBitrate;
    //JVT-W051 }
  }
  printf("\n");

  return Err::m_nOK;
}

//JVT-W051 {
ErrVal 
H264AVCEncoder::xCalMaxBitrate(UInt uiLayer)
{	
  //UInt uiDecompositionStages = m_pcCodingParameter->getLayerParameters(uiLayer).getDecompositionStages();
  UInt uiCurrIdx = 0;
  UInt uiTimeWindowSize =  (UInt)(m_pcCodingParameter->getLayerParameters(uiLayer).m_dOutputFrameRate+0.5);
  UInt uiFrameNumInGOP = m_uiFrameNumInGOP[uiLayer];
  UInt uiBaseLayerId = m_pcCodingParameter->getLayerParameters(uiLayer).getBaseLayerId();
  UInt uiBaseQualityLevel = m_pcCodingParameter->getLayerParameters(uiLayer).getBaseQualityLevel();
  if ( uiBaseQualityLevel > 3 )
  {
    uiBaseQualityLevel = 3;
  }
  for ( uiCurrIdx = 0; uiCurrIdx < uiFrameNumInGOP; uiCurrIdx++ )
  {
    if ( uiLayer )
    {
      m_aaadFrameInGOPBits[uiCurrIdx][uiLayer][0] += m_aaadFrameInGOPBits[uiCurrIdx][uiBaseLayerId][uiBaseQualityLevel];
    }

    for ( UInt uiFGSLayer = 1; uiFGSLayer < MAX_QUALITY_LEVELS; uiFGSLayer++ )
    {
      m_aaadFrameInGOPBits[uiCurrIdx][uiLayer][uiFGSLayer] += m_aaadFrameInGOPBits[uiCurrIdx][uiLayer][uiFGSLayer-1];
    }
  }
  if ( ( m_uiCodeFrameNum[uiLayer] - uiFrameNumInGOP ) < uiTimeWindowSize )
  {
    for ( uiCurrIdx = 0; (uiCurrIdx < uiFrameNumInGOP) && (m_uiCodeFrameNum[uiLayer]-uiFrameNumInGOP+uiCurrIdx < uiTimeWindowSize); uiCurrIdx ++ )
    {
      for ( UInt uiFGSLayer = 0; uiFGSLayer < MAX_QUALITY_LEVELS; uiFGSLayer++ )
      {
        m_aaadFrameInTimeWindowBits[m_uiCodeFrameNum[uiLayer]-uiFrameNumInGOP+uiCurrIdx][uiLayer][uiFGSLayer] = 
          m_aaadFrameInGOPBits[uiCurrIdx][uiLayer][uiFGSLayer];
      }
    }
    //add
    for ( UInt uiFGSLayer = 0; uiFGSLayer < MAX_QUALITY_LEVELS; uiFGSLayer++ )
    {
      m_aadMaxBitrate[uiLayer][uiFGSLayer] = 0;
      for ( UInt i = 0; i < uiTimeWindowSize; i++ )
      {
        m_aadMaxBitrate[uiLayer][uiFGSLayer] += m_aaadFrameInTimeWindowBits[i][uiLayer][uiFGSLayer];
      }
      m_aadMaxBitrate[uiLayer][uiFGSLayer] = m_aadMaxBitrate[uiLayer][uiFGSLayer]*uiTimeWindowSize/(m_uiCodeFrameNum[uiLayer]-uiFrameNumInGOP+uiCurrIdx);
    }
    //add

    if ((uiCurrIdx < uiFrameNumInGOP) || (m_uiCodeFrameNum[uiLayer]-uiFrameNumInGOP+uiCurrIdx == uiTimeWindowSize))
    {
      for ( UInt uiFGSLayer = 0; uiFGSLayer < MAX_QUALITY_LEVELS; uiFGSLayer++ )
      {
        m_aadMaxBitrate[uiLayer][uiFGSLayer] = 0;
        for ( UInt i = 0; i < uiTimeWindowSize; i++ )
        {
          m_aadMaxBitrate[uiLayer][uiFGSLayer] += m_aaadFrameInTimeWindowBits[i][uiLayer][uiFGSLayer];
        }
        //m_aadMaxBitrate[uiLayer][uiFGSLayer] = m_aadMaxBitrate[uiLayer][uiFGSLayer];
      }
      for ( UInt uiCurrAUIdx = uiCurrIdx; uiCurrAUIdx < uiFrameNumInGOP; uiCurrAUIdx++ )
      {
        for ( UInt uiFGSLayer = 0; uiFGSLayer < MAX_QUALITY_LEVELS; uiFGSLayer++ )
        {
          UInt i = 0;
          for (i = 0; i < uiTimeWindowSize-1; i++)
          {
            m_aaadFrameInTimeWindowBits[i][uiLayer][uiFGSLayer] = m_aaadFrameInTimeWindowBits[i+1][uiLayer][uiFGSLayer];
          }
          m_aaadFrameInTimeWindowBits[uiTimeWindowSize-1][uiLayer][uiFGSLayer] = m_aaadFrameInGOPBits[uiCurrAUIdx][uiLayer][uiFGSLayer];
          Double dTmpMaxBitrate = 0;
          for (i = 0; i < uiTimeWindowSize; i++)
          {
            dTmpMaxBitrate += m_aaadFrameInTimeWindowBits[i][uiLayer][uiFGSLayer];
          }
          dTmpMaxBitrate = dTmpMaxBitrate;
          if (dTmpMaxBitrate > m_aadMaxBitrate[uiLayer][uiFGSLayer])
          {
            m_aadMaxBitrate[uiLayer][uiFGSLayer] = dTmpMaxBitrate;
          }
        }				
      }
    }
  }
  else
  {
    for ( uiCurrIdx = 0; uiCurrIdx < uiFrameNumInGOP; uiCurrIdx ++ )
    {
      for ( UInt uiFGSLayer = 0; uiFGSLayer < MAX_QUALITY_LEVELS; uiFGSLayer++ )
      {
        UInt i = 0;
        for (i = 0; i < uiTimeWindowSize-1; i++)
        {
          m_aaadFrameInTimeWindowBits[i][uiLayer][uiFGSLayer] = m_aaadFrameInTimeWindowBits[i+1][uiLayer][uiFGSLayer];
        }
        m_aaadFrameInTimeWindowBits[uiTimeWindowSize-1][uiLayer][uiFGSLayer] = m_aaadFrameInGOPBits[uiCurrIdx][uiLayer][uiFGSLayer];
        Double dTmpMaxBitrate = 0;
        for (i = 0; i < uiTimeWindowSize; i++)
        {
          dTmpMaxBitrate += m_aaadFrameInTimeWindowBits[i][uiLayer][uiFGSLayer];
        }
        dTmpMaxBitrate = dTmpMaxBitrate;
        if (dTmpMaxBitrate > m_aadMaxBitrate[uiLayer][uiFGSLayer])
        {
          m_aadMaxBitrate[uiLayer][uiFGSLayer] = dTmpMaxBitrate;
        }
      }
    }

  }
  return Err::m_nOK;
}
//JVT-W051 }

ErrVal
H264AVCEncoder::xProcessGOP( PicBufferList* apcPicBufferOutputList,
                             PicBufferList* apcPicBufferUnusedList )
{
  UInt uiAUIndex, uiLayer; 

  //===== init GOP =====
  for( uiLayer = 0; uiLayer <  m_pcCodingParameter->getNumberOfLayers(); uiLayer++ )
  {
    m_apcMCTFEncoder[uiLayer]->setLayerCGSSNR           ( m_pcCodingParameter->getLayerParameters( uiLayer ).getLayerCGSSNR           () );
    m_apcMCTFEncoder[uiLayer]->setQualityLevelCGSSNR    ( m_pcCodingParameter->getLayerParameters( uiLayer ).getQualityLevelCGSSNR    () );
    m_apcMCTFEncoder[uiLayer]->setBaseLayerCGSSNR       ( m_pcCodingParameter->getLayerParameters( uiLayer ).getBaseLayerCGSSNR       () );
    m_apcMCTFEncoder[uiLayer]->setBaseQualityLevelCGSSNR( m_pcCodingParameter->getLayerParameters( uiLayer ).getBaseQualityLevelCGSSNR() );

    RNOK( m_apcMCTFEncoder[uiLayer]->initGOP( m_cAccessUnitList, m_acOrgPicBufferList[uiLayer] ) );
  }

  //JVT-W052
  if(m_pcCodingParameter->getIntegrityCheckSEIEnable() && m_pcCodingParameter->getCGSSNRRefinement())
  {
    for( uiLayer = 0; uiLayer < m_pcCodingParameter->getNumberOfLayers(); uiLayer++ )
    {
      m_uiNumofCGS[m_apcMCTFEncoder[uiLayer]->getLayerCGSSNR()] = m_pcCodingParameter->getLayerParameters( uiLayer ).getQualityLevelCGSSNR() + m_pcCodingParameter->getLayerParameters( uiLayer ).getNumberOfQualityLevelsCGSSNR() - 1;
      m_uicrcVal  [m_apcMCTFEncoder[uiLayer]->getLayerCGSSNR()] = 0xffff;
    }
  }
  //JVT-W052

  //===== loop over access units in GOP, and layers inside access units =====
  for( uiAUIndex = 0; uiAUIndex <= 64;                                       uiAUIndex++ )
  {																																									//JVT-W052
    if(m_pcCodingParameter->getIntegrityCheckSEIEnable() && m_pcCodingParameter->getCGSSNRRefinement())//JVT-W052
      RNOK( SEI::IntegrityCheckSEI::create( m_pcIntegrityCheckSEI ) );							//JVT-W052
    for( uiLayer   = 0; uiLayer   <  m_pcCodingParameter->getNumberOfLayers(); uiLayer  ++ )
    {
      if( m_pcCodingParameter->getNonRequiredEnable() )
      {
        if( uiLayer == m_pcCodingParameter->getNumberOfLayers() - 1 )   m_apcMCTFEncoder[ uiLayer ]->setNonRequiredWrite( 2 );
        else                                                            m_apcMCTFEncoder[ uiLayer ]->setNonRequiredWrite( 1 );
      }
      //JVT-W051 {
      m_bIsFirstGOP = false;
      if ( m_apcMCTFEncoder[uiLayer]->firstGOPCoded() == false )
      {
        m_bIsFirstGOP = true;
      }
      //JVT-W051 }
      RNOK( m_apcMCTFEncoder[uiLayer]->process( uiAUIndex,
                                                m_cAccessUnitList,
                                                m_acOrgPicBufferList  [uiLayer],
                                                m_acRecPicBufferList  [uiLayer],
                                                apcPicBufferUnusedList[uiLayer],
                                                m_aaauidSeqBits,
                                                // JVT-V068 HRD {
                                                m_pcParameterSetMng
                                                // JVT-V068 HRD }
                                              ) );
      //JVT-W051 {
      m_aaadFrameInGOPBits[uiAUIndex][uiLayer][0] = m_apcMCTFEncoder[uiLayer]->m_dFrameBits/1000;
      if ( uiLayer == 0 && uiAUIndex == 0 )
      {
        m_aaadFrameInGOPBits[uiAUIndex][uiLayer][0] += (Double)m_apcMCTFEncoder[uiLayer]->xGetParameterSetBits()/1000;
      }
      if ( m_bIsFirstGOP )
      {			
        m_uiProfileIdc[uiLayer] = m_apcMCTFEncoder[uiLayer]->m_uiProfileIdc;
        m_uiLevelIdc[uiLayer] = m_apcMCTFEncoder[uiLayer]->m_uiLevelIdc;
        m_bConstraint0Flag[uiLayer] = m_apcMCTFEncoder[uiLayer]->m_bConstraint0Flag;
        m_bConstraint1Flag[uiLayer] = m_apcMCTFEncoder[uiLayer]->m_bConstraint1Flag;
        m_bConstraint2Flag[uiLayer] = m_apcMCTFEncoder[uiLayer]->m_bConstraint2Flag;
        m_bConstraint3Flag[uiLayer] = m_apcMCTFEncoder[uiLayer]->m_bConstraint3Flag;
      }
      //JVT-W051 }
    }		
  }//JVT-W052
  
  //JVT-W051 {
  for( uiLayer = 0; uiLayer < m_pcCodingParameter->getNumberOfLayers(); uiLayer ++ )
  {
    m_uiFrameNumInGOP[uiLayer] = 0;
    for ( UInt i = 0; i < 65; i++ )
    {
      if ( m_aaadFrameInGOPBits[i][uiLayer][0] != 0 )
      {
        m_uiFrameNumInGOP[uiLayer]++;
      }
    }
    m_uiCodeFrameNum[uiLayer] += m_uiFrameNumInGOP[uiLayer];
    xCalMaxBitrate(uiLayer);
  }
  //JVT-W051 

  //===== update pic buffer lists =====
  for( uiLayer = 0; uiLayer < m_pcCodingParameter->getNumberOfLayers(); uiLayer++ )
  {
    //----- set output list -----
    apcPicBufferOutputList[ uiLayer ] += m_acRecPicBufferList[ uiLayer ];
    //----- update unused list -----
    apcPicBufferUnusedList[ uiLayer ] += m_acOrgPicBufferList[ uiLayer ];
    apcPicBufferUnusedList[ uiLayer ] += m_acRecPicBufferList[ uiLayer ];
    //----- reset lists -----
    m_acOrgPicBufferList  [ uiLayer ].clear();
    m_acRecPicBufferList  [ uiLayer ].clear();
  }

  return Err::m_nOK;
}


ErrVal
H264AVCEncoder::xInitParameterSets()
{
  UInt uiSPSId = 0;
  UInt uiPPSId = 0;
  UInt uiIndex;

  //===== determine values for Poc calculation =====
  UInt uiMaxResolutionStages  = m_pcCodingParameter->getDecompositionStages();  
  UInt uiRequiredPocBits      = max( 4, 1 + (Int)ceil( log10( 1.0 + ( 1 << uiMaxResolutionStages ) ) / log10( 2.0 ) ) );


  //===== loop over layers =====
  for( uiIndex = 0; uiIndex < m_pcCodingParameter->getNumberOfLayers(); uiIndex++ )
  {
    //===== get configuration parameters =====
    LayerParameters&  rcLayerParameters   = m_pcCodingParameter->getLayerParameters( uiIndex );
//bug-fix suffix{{
  //Bool              bH264AVCCompatible  = m_pcCodingParameter->getBaseLayerMode() > 0 && uiIndex == 0;
  Bool              bH264AVCCompatible  = ( uiIndex == 0 );
//bug-fix suffix}}
  UInt              uiMbY               = rcLayerParameters.getFrameHeight() / 16;
    UInt              uiMbX               = rcLayerParameters.getFrameWidth () / 16;
    UInt              uiOutFreq           = (UInt)ceil( rcLayerParameters.getOutputFrameRate() );
    UInt              uiMvRange           = m_pcCodingParameter->getMotionVectorSearchParams().getSearchRange() / 4;
    UInt              uiDPBSize           = ( 1 << max( 1, rcLayerParameters.getDecompositionStages() ) );
    UInt              uiNumRefPic         = uiDPBSize; 
    UInt              uiLevelIdc          = SequenceParameterSet::getLevelIdc( uiMbY, uiMbX, uiOutFreq, uiMvRange, uiDPBSize );
//bug-fix suffix{{
//	if(m_pcCodingParameter->getBaseLayerMode() > 0)
//bug-fix suffix}}
//	{
//		ROT( bH264AVCCompatible && uiDPBSize > 16 );
//	}
  ROT( uiLevelIdc == MSYS_UINT_MAX );

    
    //===== create parameter sets, set Id's, and store =====
    SequenceParameterSet* pcSPS;
    PictureParameterSet*  pcPPSLP;
    PictureParameterSet*  pcPPSHP;
    
    RNOK( SequenceParameterSet::create( pcSPS   ) );
    RNOK( PictureParameterSet ::create( pcPPSHP ) );
    pcPPSHP->setPicParameterSetId( uiPPSId++ );
    pcPPSHP->setSeqParameterSetId( uiSPSId   );
    RNOK( m_pcParameterSetMng->store( pcPPSHP ) );
    if( rcLayerParameters.getContrainedIntraForLP() )
    {
      pcPPSLP = pcPPSHP;
    }
    else
    {
      RNOK( PictureParameterSet ::create( pcPPSLP ) );
      pcPPSLP->setPicParameterSetId( uiPPSId++ );
      pcPPSLP->setSeqParameterSetId( uiSPSId   );
      RNOK( m_pcParameterSetMng->store( pcPPSLP ) );
    }
    pcSPS->setSeqParameterSetId( uiSPSId++ );
    RNOK( m_pcParameterSetMng->store( pcSPS   ) );


    //===== set sequence parameter set parameters =====
    pcSPS->setNalUnitType                         ( NAL_UNIT_SPS );
    pcSPS->setLayerId                             ( rcLayerParameters.getLayerId() );
    pcSPS->setProfileIdc                          ( bH264AVCCompatible ? ( rcLayerParameters.getAdaptiveTransform() > 0 ? HIGH_PROFILE : MAIN_PROFILE ) : SCALABLE_PROFILE );
    pcSPS->setConstrainedSet0Flag                 ( false );
    pcSPS->setConstrainedSet1Flag                 ( bH264AVCCompatible ? 1 : 0 );
    pcSPS->setConstrainedSet2Flag                 ( false );
    pcSPS->setConstrainedSet3Flag                 ( false );
    pcSPS->setLevelIdc                            ( uiLevelIdc );
    pcSPS->setSeqScalingMatrixPresentFlag         ( rcLayerParameters.getAdaptiveTransform() > 1 );
    pcSPS->setLog2MaxFrameNum                     ( MAX_FRAME_NUM_LOG2 );
    pcSPS->setLog2MaxPicOrderCntLsb               ( min( 15, uiRequiredPocBits + 2 ) );  // HS: decoder robustness -> value increased by 2
    pcSPS->setNumRefFrames                        ( uiNumRefPic );
    pcSPS->setGapsInFrameNumValueAllowedFlag      ( true );
    pcSPS->setFrameWidthInMbs                     ( uiMbX );
    pcSPS->setFrameHeightInMbs                    ( uiMbY );
    pcSPS->setDirect8x8InferenceFlag              ( true  );
    // TMM_ESS 
    pcSPS->setResizeParameters                    (rcLayerParameters.getResizeParameters());

    if (uiIndex == 0)
    {  // base layer AVC rewrite flag should always be false
      pcSPS->setAVCRewriteFlag(false);
      pcSPS->setAVCAdaptiveRewriteFlag(false);
    }
    else
    {
      pcSPS->setAVCRewriteFlag(rcLayerParameters.getAVCRewriteFlag());
      pcSPS->setAVCAdaptiveRewriteFlag(rcLayerParameters.getAVCAdaptiveRewriteFlag());
    }
    
    const Bool  bInterlaced = ( rcLayerParameters.getPaff() || rcLayerParameters.getMbAff() );
    pcSPS->setPaff                                ( bH264AVCCompatible || ! rcLayerParameters.getPaff() );
    pcSPS->setFrameMbsOnlyFlag										( ! bInterlaced );
    pcSPS->setMbAdaptiveFrameFieldFlag            ( (rcLayerParameters.getMbAff() ? true : false ) );
   
    if( pcSPS->getMbAdaptiveFrameFieldFlag() && uiMbY % 2)
    {
      printf(" MBAFF ignored ");
      pcSPS->setMbAdaptiveFrameFieldFlag( false ); //not allowed
    }
    
    for( UInt ui = 0; ui < 16; ui++ )
    {
      pcSPS->setMGSVect                           ( ui, rcLayerParameters.getMGSVect( ui ) );
    }

    pcSPS->setRCDOBlockSizes         ( false );
    pcSPS->setRCDOMotionCompensationY( false );
    pcSPS->setRCDOMotionCompensationC( false );
    pcSPS->setRCDODeblocking         ( false );
    UInt  uiRCDOBS  = m_pcCodingParameter->getRCDOBlockSizes         ();
    UInt  uiRCDOMCY = m_pcCodingParameter->getRCDOMotionCompensationY();
    UInt  uiRCDOMCC = m_pcCodingParameter->getRCDOMotionCompensationC();
    UInt  uiRCDODB  = m_pcCodingParameter->getRCDODeblocking         ();
    pcSPS->setRCDOBlockSizes         ( ( uiIndex && uiRCDOBS  ) || ( uiRCDOBS  > 1 ) );
    pcSPS->setRCDOMotionCompensationY( ( uiIndex && uiRCDOMCY ) || ( uiRCDOMCY > 1 ) );
    pcSPS->setRCDOMotionCompensationC( ( uiIndex && uiRCDOMCC ) || ( uiRCDOMCC > 1 ) );
    pcSPS->setRCDODeblocking         ( ( uiIndex && uiRCDODB  ) || ( uiRCDODB  > 1 ) );

  // V090
  pcSPS->set4TapMotionCompensationY( false );
  UInt  ui4TapMCY = m_pcCodingParameter->get4TapMotionCompensationY();                 // V090
  pcSPS->set4TapMotionCompensationY( ( uiIndex && ui4TapMCY ) || ( ui4TapMCY > 1 ) );  // V090
  // V090

  // JVT-V068 HRD {
    pcSPS->setVUI( pcSPS );
  xInitLayerInfoForHrd( pcSPS, uiIndex );
  // JVT-V068 HRD }

    //===== set picture parameter set parameters =====
    pcPPSHP->setNalUnitType                           ( NAL_UNIT_PPS );
    pcPPSHP->setLayerId                               ( rcLayerParameters.getLayerId() );
    pcPPSHP->setEntropyCodingModeFlag                 ( rcLayerParameters.getEntropyCodingModeFlag() );
    pcPPSHP->setPicOrderPresentFlag                   ( true );
    pcPPSHP->setNumRefIdxActive( LIST_0               , m_pcCodingParameter->getNumRefFrames() );
    pcPPSHP->setNumRefIdxActive( LIST_1               , m_pcCodingParameter->getNumRefFrames() );
    // heiko.schwarz@hhi.fhg.de: ensures that the PPS QP will be in the valid range (specified QP can be outside that range to force smaller/higher lambdas)
    //pcPPSHP->setPicInitQp                             ( (Int)rcLayerParameters.getBaseQpResidual() );
    pcPPSHP->setPicInitQp                             ( min( 51, max( 0, (Int)rcLayerParameters.getBaseQpResidual() ) ) );
    pcPPSHP->setChomaQpIndexOffset                    ( 0 );
    pcPPSHP->setDeblockingFilterParametersPresentFlag ( ! m_pcCodingParameter->getLoopFilterParams().isDefault() );
    pcPPSHP->setConstrainedIntraPredFlag              ( true );
    pcPPSHP->setRedundantPicCntPresentFlag            ( rcLayerParameters.getUseRedundantSliceFlag() ); // JVT-Q054 Red. Picture
    //JVT-W049 {
    pcPPSHP->setRedundantKeyPicCntPresentFlag         ( rcLayerParameters.getUseRedundantKeySliceFlag() );
    if ( m_pcCodingParameter->getEnableRedundantKeyPic() == true ) 
    {	
      pcPPSHP->setEnableRedundantKeyPicCntPresentFlag   ( true );
      if( rcLayerParameters.getLayerId() != 0 )
      {
        pcPPSHP->setRedundantPicCntPresentFlag            ( true );
        pcPPSHP->setRedundantKeyPicCntPresentFlag         ( true );
      }
    }
    //JVT-W049 }
    pcPPSHP->setTransform8x8ModeFlag                  ( rcLayerParameters.getAdaptiveTransform() > 0 );
    pcPPSHP->setPicScalingMatrixPresentFlag           ( false );
    pcPPSHP->set2ndChromaQpIndexOffset                ( 0 );

    pcPPSHP->setWeightedPredFlag                      ( WEIGHTED_PRED_FLAG );
    pcPPSHP->setWeightedBiPredIdc                     ( WEIGHTED_BIPRED_IDC );
//TMM_WP
    pcPPSHP->setWeightedPredFlag                   (m_pcCodingParameter->getIPMode()!=0);
    pcPPSHP->setWeightedBiPredIdc                  (m_pcCodingParameter->getBMode());  
//TMM_WP

    //--ICU/ETRI FMO Implementation : FMO stuff start
    pcPPSHP->setNumSliceGroupsMinus1                  (rcLayerParameters.getNumSliceGroupsMinus1());
    pcPPSHP->setSliceGroupMapType                     (rcLayerParameters.getSliceGroupMapType());
    pcPPSHP->setArrayRunLengthMinus1					      (rcLayerParameters.getArrayRunLengthMinus1());
    pcPPSHP->setArrayTopLeft								  (rcLayerParameters.getArrayTopLeft());
    pcPPSHP->setArrayBottomRight							  (rcLayerParameters.getArrayBottomRight());
    pcPPSHP->setSliceGroupChangeDirection_flag		  (rcLayerParameters.getSliceGroupChangeDirection_flag());
    pcPPSHP->setSliceGroupChangeRateMinus1			  (rcLayerParameters.getSliceGroupChangeRateMinus1());
    pcPPSHP->setNumSliceGroupMapUnitsMinus1			  (rcLayerParameters.getNumSliceGroupMapUnitsMinus1());
    pcPPSHP->setArraySliceGroupId						  (rcLayerParameters.getArraySliceGroupId());
    //--ICU/ETRI FMO Implementation : FMO stuff end

    if( ! rcLayerParameters.getContrainedIntraForLP() )
    {
      pcPPSLP->setNalUnitType                           ( pcPPSHP->getNalUnitType                           ()  );
      pcPPSLP->setLayerId                               ( pcPPSHP->getLayerId                               ()  );
      pcPPSLP->setEntropyCodingModeFlag                 ( pcPPSHP->getEntropyCodingModeFlag                 ()  );
      pcPPSLP->setPicOrderPresentFlag                   ( pcPPSHP->getPicOrderPresentFlag                   ()  );
      pcPPSLP->setNumRefIdxActive( LIST_0               , pcPPSHP->getNumRefIdxActive               ( LIST_0 )  );
      pcPPSLP->setNumRefIdxActive( LIST_1               , pcPPSHP->getNumRefIdxActive               ( LIST_1 )  );
      pcPPSLP->setPicInitQp                             ( pcPPSHP->getPicInitQp                             ()  );
      pcPPSLP->setChomaQpIndexOffset                    ( pcPPSHP->getChomaQpIndexOffset                    ()  );
      pcPPSLP->setDeblockingFilterParametersPresentFlag ( pcPPSHP->getDeblockingFilterParametersPresentFlag ()  );
      pcPPSLP->setConstrainedIntraPredFlag              ( false                                                 );
      pcPPSLP->setRedundantPicCntPresentFlag            ( pcPPSHP->getRedundantPicCntPresentFlag            ()  );  //JVT-Q054 Red. Picture
      pcPPSLP->setRedundantKeyPicCntPresentFlag         ( pcPPSHP->getRedundantKeyPicCntPresentFlag         ()  );  //JVT-W049
      pcPPSLP->setTransform8x8ModeFlag                  ( pcPPSHP->getTransform8x8ModeFlag                  ()  );
      pcPPSLP->setPicScalingMatrixPresentFlag           ( pcPPSHP->getPicScalingMatrixPresentFlag           ()  );
      pcPPSLP->set2ndChromaQpIndexOffset                ( pcPPSHP->get2ndChromaQpIndexOffset                ()  );
      pcPPSLP->setWeightedPredFlag                      ( pcPPSHP->getWeightedPredFlag                      ()  );
      pcPPSLP->setWeightedBiPredIdc                     ( pcPPSHP->getWeightedBiPredIdc                     ()  );
    }

    //--ICU/ETRI FMO Implementation : FMO stuff start
    pcPPSLP->setNumSliceGroupsMinus1                  (rcLayerParameters.getNumSliceGroupsMinus1());
    pcPPSLP->setSliceGroupMapType                     (rcLayerParameters.getSliceGroupMapType());
    pcPPSLP->setArrayRunLengthMinus1					      (rcLayerParameters.getArrayRunLengthMinus1());
    pcPPSLP->setArrayTopLeft								  (rcLayerParameters.getArrayTopLeft());
    pcPPSLP->setArrayBottomRight							  (rcLayerParameters.getArrayBottomRight());
    pcPPSLP->setSliceGroupChangeDirection_flag		  (rcLayerParameters.getSliceGroupChangeDirection_flag());
    pcPPSLP->setSliceGroupChangeRateMinus1			  (rcLayerParameters.getSliceGroupChangeRateMinus1());
    pcPPSLP->setNumSliceGroupMapUnitsMinus1			  (rcLayerParameters.getNumSliceGroupMapUnitsMinus1());
    pcPPSLP->setArraySliceGroupId						  (rcLayerParameters.getArraySliceGroupId());
    //--ICU/ETRI FMO Implementation : FMO stuff end

    //===== initialization using parameter sets =====
    RNOK( m_pcControlMng->initParameterSets( *pcSPS, *pcPPSLP, *pcPPSHP ) );
  }


  uiIndex = 0;
//bug-fix suffix{{
  //Bool              bH264AVCCompatible  = m_pcCodingParameter->getBaseLayerMode() > 0 && uiIndex == 0;
//bug-fix suffix}}

  //===== set unwritten parameter lists =====
  RNOK( m_pcParameterSetMng->setParamterSetList( m_cUnWrittenSPS, m_cUnWrittenPPS ) );

  return Err::m_nOK;
}

#ifdef SHARP_AVC_REWRITE_OUTPUT
MbCoder* H264AVCEncoder::xGetPcMbCoder()
{
  return m_pcMbCoder;
}

ErrVal H264AVCEncoder::initMb( MbDataAccess*& pcMbDataAccessRewrite, UInt uiMbY, UInt uiMbX )
{
  return m_pcMbDataCtrl->initMb( pcMbDataAccessRewrite, uiMbY, uiMbX );
}

ErrVal H264AVCEncoder::initSlice( SliceHeader& rcSH, ProcessingState eProcessingState, Bool bDecoder, MbDataCtrl* pcMbDataCtrl )
{
  return m_pcMbDataCtrl->initSlice( rcSH, eProcessingState, bDecoder, pcMbDataCtrl );
}

Void
H264AVCEncoder::xStoreEstimation( MbDataAccess& rcMbDataAccess, MbDataAccess& rcMbBestData )
{
  if( rcMbBestData.getMbData().isIntra() )
  {
    rcMbBestData.getMbData().getMbMotionData( LIST_0 ).clear( BLOCK_NOT_PREDICTED );
    rcMbBestData.getMbData().getMbMvdData   ( LIST_0 ).clear();

    if( rcMbDataAccess.getSH().isInterB() )
    {
      rcMbBestData.getMbData().getMbMotionData( LIST_1 ).clear( BLOCK_NOT_PREDICTED );
      rcMbBestData.getMbData().getMbMvdData   ( LIST_1 ).clear();
    }
  }
  else if( rcMbBestData.getMbData().isSkiped() )
  {
    rcMbBestData.getMbData().getMbMvdData( LIST_0 ).clear();

    if( rcMbDataAccess.getSH().isInterB() )
    {
      rcMbBestData.getMbData().getMbMvdData( LIST_1 ).clear();
    }
  }  

  UInt uiFwdBwd = 0;
  if( rcMbDataAccess.getSH().isInterB() )
  {
    for( Int n = 3; n >= 0; n--)
    {
      uiFwdBwd <<= 4;
      uiFwdBwd += (0 < rcMbBestData.getMbData().getMbMotionData( LIST_0 ).getRefIdx( Par8x8(n) )) ? 1:0;
      uiFwdBwd += (0 < rcMbBestData.getMbData().getMbMotionData( LIST_1 ).getRefIdx( Par8x8(n) )) ? 2:0;
    }
  }

  if( rcMbDataAccess.getSH().isInterP() )
  {
    for( Int n = 3; n >= 0; n--)
    {
      uiFwdBwd <<= 4;
      uiFwdBwd +=  (0 < rcMbBestData.getMbData().getMbMotionData( LIST_0 ).getRefIdx( Par8x8(n) )) ? 1:0;
    }
  }
  rcMbBestData.getMbData().setFwdBwd  ( uiFwdBwd );
  //rcMbBestData.copyTo     ( rcMbDataAccess );
  
  rcMbDataAccess.getMbData()            .copyFrom( rcMbBestData.getMbData() );
  rcMbDataAccess.getMbTCoeffs()         .copyFrom( rcMbBestData.getMbTCoeffs() );
  rcMbDataAccess.getMbData().copyMotion( rcMbBestData.getMbData(), 0 );  // SET THE SLICE ID (BIT OF A HACK)  

  if( ! rcMbDataAccess.getMbData().isIntra4x4() && ( rcMbDataAccess.getMbData().getMbCbp() & 0x0F ) == 0 
    && !rcMbDataAccess.getMbData().isIntra_BL() 	  
    )
  {
    rcMbDataAccess.getMbData().setTransformSize8x8( false );
  }
}


ErrVal
H264AVCEncoder::xCreateAvcRewriteEncoder() {

  ROT( NULL == this);
  // only create the parts relating to the SVC-AVC rewrite	
  RNOK( BitWriteBuffer              ::create( m_pcBitWriteBuffer ) );
  RNOK( BitCounter                  ::create( m_pcBitCounter ) );
  RNOK( NalUnitEncoder              ::create( m_pcNalUnitEncoder) );

  RNOK( UvlcWriter                  ::create( m_pcUvlcWriter ) );
  RNOK( UvlcWriter                  ::create( m_pcUvlcTester ) );
  RNOK( CabacWriter                 ::create( m_pcCabacWriter ) );
  RNOK( MbCoder                     ::create( m_pcMbCoder ) );
  RNOK( RateDistortion              ::create( m_pcRateDistortion ) ); // maybe used in init m_pcMbCoder

  return Err::m_nOK;
}

ErrVal
H264AVCEncoder::xInitAvcRewriteEncoder() {
  // init
  RNOK( m_pcBitWriteBuffer          ->init() );
  RNOK( m_pcBitCounter              ->init() );
  
  RNOK( m_pcUvlcWriter              ->init( m_pcBitWriteBuffer ) );
  RNOK( m_pcUvlcTester              ->init( m_pcBitCounter ) );
  RNOK( m_pcCabacWriter             ->init( m_pcBitWriteBuffer ) );

  RNOK( m_pcNalUnitEncoder          ->init( m_pcBitWriteBuffer,
                                            m_pcUvlcWriter,
                                            m_pcUvlcTester));   // 
     
  return Err::m_nOK;
}

ErrVal
H264AVCEncoder::xInitSliceForAvcRewriteCoding(const SliceHeader& rcSH)
{
  MbSymbolWriteIf* m_pcMbSymbolWriteIf;
  bool bCabac = rcSH.getPPS().getEntropyCodingModeFlag();

  if(bCabac )
  {
    m_pcMbSymbolWriteIf = m_pcCabacWriter;
  }
  else
  {
    m_pcMbSymbolWriteIf = m_pcUvlcWriter;
  }

  RNOK( m_pcMbSymbolWriteIf   ->startSlice( rcSH ) );
  RNOK( m_pcMbCoder           ->initSlice ( rcSH, m_pcMbSymbolWriteIf, m_pcRateDistortion) ); // 

  if (m_pcMbDataCtrl == NULL)
  {
    m_pcMbDataCtrl = new MbDataCtrl();
    m_pcMbDataCtrl->init( rcSH.getSPS() ); 
  }
    
  return Err::m_nOK;
}

ErrVal
H264AVCEncoder::xCloseAvcRewriteEncoder() {

  RNOK( m_pcBitWriteBuffer        ->destroy() );
  RNOK( m_pcBitCounter            ->destroy() );
  RNOK( m_pcNalUnitEncoder        ->destroy() );
  RNOK( m_pcUvlcWriter            ->destroy() );
  RNOK( m_pcUvlcTester            ->destroy() );
  RNOK( m_pcCabacWriter           ->destroy() );
  RNOK( m_pcMbCoder               ->destroy() );
  RNOK( m_pcRateDistortion        ->destroy() );
  if( m_pcMbDataCtrl )
  {
    m_pcMbDataCtrl->uninit();
    m_pcMbDataCtrl = NULL;
  }
  return Err::m_nOK;
}

// overloaded functions to write parameter sets
ErrVal  
H264AVCEncoder::xAvcRewriteParameterSets ( ExtBinDataAccessor* pcExtBinDataAccessor,const SliceHeader& rcSH, const NalUnitType eNalUnitType) {
  UInt uiBits=0;
  RNOK( m_pcNalUnitEncoder->initNalUnit( pcExtBinDataAccessor ) );
  switch( eNalUnitType )
  {
  case NAL_UNIT_SPS:
    {
      // Todo: disable sending seq_parameter_set_svc_extension if profile_idc == 83			
      RNOK( m_pcNalUnitEncoder->write( rcSH.getSPS()) );		// write SPS	  
      break;
    }
  case NAL_UNIT_PPS:
    {
      RNOK( m_pcNalUnitEncoder->write( rcSH.getPPS()) );		// write PPS	 
      break;
    }
  case NAL_UNIT_SEI:     // SHARP_AVC_REWRITE_OUTPUT: TODO
    break;
  case NAL_UNIT_ACCESS_UNIT_DELIMITER:    
    break;
  case NAL_UNIT_END_OF_SEQUENCE:
    break;
  case NAL_UNIT_END_OF_STREAM:
    break;
  default:
    break;
  }

  RNOK( m_pcNalUnitEncoder->closeNalUnit( uiBits ) );
  return Err::m_nOK;
}

ErrVal  
H264AVCEncoder::xAvcRewriteParameterSets ( ExtBinDataAccessor* pcExtBinDataAccessor, SequenceParameterSet& rcSps) {
  UInt uiBits=0;
  RNOK( m_pcNalUnitEncoder->initNalUnit( pcExtBinDataAccessor ) );
  
  Profile profile_idc_save = rcSps.getProfileIdc();

  if (profile_idc_save ==   SCALABLE_PROFILE)  // SVC extension 
    rcSps.setProfileIdc(HIGH_PROFILE);

  RNOK( m_pcNalUnitEncoder->write( rcSps) );	// write SPS	  

  RNOK( m_pcNalUnitEncoder->closeNalUnit( uiBits ) );

  rcSps.setProfileIdc(profile_idc_save);     // resume the profile

  return Err::m_nOK;
}

ErrVal  
H264AVCEncoder::xAvcRewriteParameterSets ( ExtBinDataAccessor* pcExtBinDataAccessor, PictureParameterSet& rcPps) {
  UInt uiBits=0;
  RNOK( m_pcNalUnitEncoder->initNalUnit( pcExtBinDataAccessor ) );	
  RNOK( m_pcNalUnitEncoder->write( rcPps) );		// write PPS	  

  RNOK( m_pcNalUnitEncoder->closeNalUnit( uiBits ) );

  return Err::m_nOK;
}

ErrVal  
H264AVCEncoder::xAvcRewriteSliceHeader ( ExtBinDataAccessor* pcExtBinDataAccessor, SliceHeader& rcSH) {
  // need to write AVC compatible
  NalUnitType nal_type_save = rcSH.getNalUnitType();

  if (nal_type_save==NAL_UNIT_CODED_SLICE_SCALABLE)
    rcSH.setNalUnitType(NAL_UNIT_CODED_SLICE);
  else if (nal_type_save==NAL_UNIT_CODED_SLICE_IDR_SCALABLE)
    rcSH.setNalUnitType(NAL_UNIT_CODED_SLICE_IDR);

  RNOK( m_pcNalUnitEncoder->write  (rcSH) );

  // resume the NAL slice type
  rcSH.setNalUnitType(nal_type_save);

  return Err::m_nOK;
}

ErrVal  
H264AVCEncoder::xAvcRewriteInitNalUnit ( ExtBinDataAccessor* pcExtBinDataAccessor) {

  RNOK( m_pcNalUnitEncoder->initNalUnit( pcExtBinDataAccessor ) );
  return Err::m_nOK;
}

ErrVal  
H264AVCEncoder::xAvcRewriteCloseNalUnit () {
  UInt uiBits;
  RNOK( m_pcNalUnitEncoder->closeNalUnit( uiBits ) );
  return Err::m_nOK;
}

#endif

// JVT-V068 HRD {
ErrVal H264AVCEncoder::xInitLayerInfoForHrd(SequenceParameterSet* pcSPS, UInt uiLayer )
{
  Scheduler* pcScheduler = NULL;

  VUI* pcVui = pcSPS->getVUI();
  LayerParameters& rcLayer = m_pcCodingParameter->getLayerParameters(uiLayer);

  UInt uiTotalTemporalLevel = rcLayer.getDecompositionStages()- rcLayer.getNotCodedMCTFStages() + 1;
  UInt uiTotalFGSLevel = (m_pcCodingParameter->getCGSSNRRefinement() == 1 ? 1 : MAX_QUALITY_LEVELS);
  UInt uiDependencyId = m_pcCodingParameter->getCGSSNRRefinement() == 1 ? rcLayer.getLayerCGSSNR() : uiLayer;

  pcVui->setVuiParametersPresentFlag( ( m_pcCodingParameter->getEnableVUI()!=0) );

  if (pcVui->getVuiParametersPresentFlag())
  {
    pcVui->setProfileIdc(pcSPS->getProfileIdc());
    pcVui->setBaseLayerProfileIdc( m_pcCodingParameter->getLayerParameters(0).getAdaptiveTransform() > 0 ? HIGH_PROFILE : MAIN_PROFILE );
    pcVui->setLevelIdc(pcSPS->getLevelIdc());

    pcVui->init( pcSPS, uiDependencyId, uiTotalTemporalLevel, uiTotalFGSLevel );

    // initiate Nal HRD pamameters
    UInt uiCpbBrNalFactor = pcVui->getBaseLayerProfileIdc() < HIGH_PROFILE ? 1200 : 
                          ( pcVui->getBaseLayerProfileIdc() < HIGH_10_PROFILE ? 1500 :
                          ( pcVui->getBaseLayerProfileIdc() < HIGH_10_PROFILE ? 3600 : 4800 ) );

    UInt uiCpbBrVclFactor = pcVui->getBaseLayerProfileIdc() < HIGH_PROFILE ? 1000 : 
                          ( pcVui->getBaseLayerProfileIdc() < HIGH_10_PROFILE ? 1250 :
                          ( pcVui->getBaseLayerProfileIdc() < HIGH_10_PROFILE ? 3000 : 4000 ) );

    // handle 'output frequency' case
    Float fOutFreq = (Float)rcLayer.getOutputFrameRate() / ( 1 << (uiTotalTemporalLevel - 1) );
    UInt uiTimeScale = UInt(fOutFreq * 2000000 + 0.5); // integer frame rate (24, 30, 50, 60, etc)
    UInt uiNumUnits = 1000000;

    for (UInt uiTemporalId = 0; uiTemporalId < uiTotalTemporalLevel; uiTemporalId++)
    for (UInt uiFGSLevel = 0; uiFGSLevel < uiTotalFGSLevel; uiFGSLevel++)
    {
      UInt uiQualityLevel = (m_pcCodingParameter->getCGSSNRRefinement() == 1 ? rcLayer.getQualityLevelCGSSNR() : uiFGSLevel);
      UInt uiIndex = (uiDependencyId<<5)+(uiTemporalId<<2)+uiQualityLevel;
      UInt uiHrdIdx = uiTemporalId * uiTotalFGSLevel + uiFGSLevel;

      pcVui->getLayerInfo(uiHrdIdx).setDependencyID(uiDependencyId);
      pcVui->getLayerInfo(uiHrdIdx).setTemporalLevel(uiTemporalId);
      pcVui->getLayerInfo(uiHrdIdx).setQualityLevel(uiQualityLevel);
      pcVui->getTimingInfo(uiHrdIdx).setTimingInfoPresentFlag((m_pcCodingParameter->getEnableVUITimingInfo()!=0));
      if (pcVui->getTimingInfo(uiHrdIdx).getTimingInfoPresentFlag() )
      {
        // write TimingInfo
        pcVui->getTimingInfo(uiHrdIdx).setNumUnitsInTick ( uiNumUnits );
        pcVui->getTimingInfo(uiHrdIdx).setTimeScale      ( uiTimeScale << uiTemporalId );
        pcVui->getTimingInfo(uiHrdIdx).setFixedFrameRateFlag ( true );  // fixed frame rate hard coded
      }
      pcVui->getNalHrd(uiHrdIdx).setHrdParametersPresentFlag(m_pcCodingParameter->getEnableNalHRD());
      pcVui->getVclHrd(uiHrdIdx).setHrdParametersPresentFlag(m_pcCodingParameter->getEnableVclHRD());
      pcVui->getLowDelayHrdFlag(uiHrdIdx) = false;        // we hard coded the low delay mode.
      pcVui->getPicStructPresentFlag(uiHrdIdx) = false;   // we hard coded the pic_struct_present_flag.

      if ( pcVui->getNalHrd(uiHrdIdx).getHrdParametersPresentFlag() )
        pcVui->InitHrd( uiHrdIdx, HRD::NAL_HRD,  pcSPS->getMaxBitRate() * uiCpbBrNalFactor, pcSPS->getMaxCPBSize() * uiCpbBrNalFactor );

      if ( pcVui->getVclHrd(uiHrdIdx).getHrdParametersPresentFlag() )
        pcVui->InitHrd( uiHrdIdx, HRD::VCL_HRD, pcSPS->getMaxBitRate() * uiCpbBrVclFactor, pcSPS->getMaxCPBSize() * uiCpbBrVclFactor );

      if ( m_pcCodingParameter->getEnableNalHRD() || m_pcCodingParameter->getEnableVclHRD() )
      {
        Scheduler::create(pcScheduler);
        pcScheduler->init(m_pcCodingParameter, uiLayer);
        RNOK( pcScheduler->initBuffer (pcVui, uiHrdIdx));
        pcScheduler->setInitialOutputDelay( 2*rcLayer.getDecompositionStages() );
        (*m_apcScheduler)[uiIndex] = pcScheduler;
      }
    }
  }

  return Err::m_nOK;
}

ErrVal H264AVCEncoder::writeAVCCompatibleHRDSEI(ExtBinDataAccessor* pcExtBinDataAccessor, SequenceParameterSet& rcSPS )
{
  VUI* pcVUI = rcSPS.getVUI();
  if (pcVUI->getVuiParametersPresentFlag() && pcVUI->getNumTemporalLevels() > 1)
  {
    UInt uiBitsSei = 0;
    SEI::MessageList cSEIMessageList; 
    SEI::AVCCompatibleHRD* pcAVCCompHRD;
    RNOK( SEI::AVCCompatibleHRD::create(pcAVCCompHRD, pcVUI) );  
    cSEIMessageList.push_back(pcAVCCompHRD);
    RNOK( m_pcNalUnitEncoder->initNalUnit( pcExtBinDataAccessor ) );
    RNOK( m_pcNalUnitEncoder->write( cSEIMessageList ) );
    RNOK( m_pcNalUnitEncoder->closeNalUnit( uiBitsSei ) );
  }
  return Err::m_nOK;
}
// JVT-V068 HRD }

H264AVC_NAMESPACE_END
