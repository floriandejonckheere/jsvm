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




H264AVC_NAMESPACE_BEGIN


H264AVCEncoder::H264AVCEncoder():
  m_pcParameterSetMng ( NULL ),
  m_pcPocCalculator   ( NULL ),
  m_pcNalUnitEncoder  ( NULL ),
  m_pcControlMng      ( NULL ),
  m_pcCodingParameter ( NULL ),
  m_pcFrameMng        ( NULL ),
  m_bVeryFirstCall    ( true ),
  m_bScalableSeiMessage( false ),
  m_bInitDone         ( false ),
  m_bTraceEnable      ( false )  
{
  ::memset( m_apcMCTFEncoder, 0x00, MAX_LAYERS*sizeof(Void*) );
  ::memset( m_dFinalFramerate, 0x00,MAX_LAYERS*MAX_DSTAGES*MAX_QUALITY_LEVELS*sizeof(Double) );
	::memset( m_dFinalBitrate,	0x00, MAX_LAYERS*MAX_DSTAGES*MAX_QUALITY_LEVELS*sizeof(Double) );
	for( UInt ui = 0; ui < MAX_LAYERS; ui++ )
	for( UInt uj = 0; uj < MAX_TEMP_LEVELS; uj++ )
		for( UInt uk = 0; uk < MAX_QUALITY_LEVELS; uk++ ){

		  m_aaauidSeqBits[ui][uj][uk] = 0;                   
		  m_aaadSingleLayerBitrate[ui][uj][uk] = 0;            // BUG_FIX Shenqiu (06-04-08)
		  m_aaauiScalableLayerId[ui][uj][uk] = MSYS_UINT_MAX;  // BUG_FIX Shenqiu (06-04-08)
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



ErrVal
H264AVCEncoder::getBaseLayerStatus( UInt&   ruiBaseLayerId,
                                    UInt&   ruiBaseLayerIdMotionOnly,
                                    Int &   riSpatialScalabilityType,
                                    UInt    uiLayerId,
                                    Int     iPoc )
{
  //===== get spatial resolution ratio =====
 
// TMM_ESS 
  riSpatialScalabilityType = m_apcMCTFEncoder[uiLayerId]->getSpatialScalabilityType();

  //===== check data availability =====
  if( ruiBaseLayerId < m_pcCodingParameter->getNumberOfLayers() )
  {
    Bool  bExists = false;
    Bool  bMotion = false;

    RNOK( m_apcMCTFEncoder[ruiBaseLayerId]->getBaseLayerStatus( bExists, bMotion, iPoc ) );

    ruiBaseLayerIdMotionOnly  = ( bMotion ? ruiBaseLayerId : MSYS_UINT_MAX );
    ruiBaseLayerId            = ( bExists ? ruiBaseLayerId : MSYS_UINT_MAX );
    return Err::m_nOK;
  }

  ruiBaseLayerId              = MSYS_UINT_MAX;
  ruiBaseLayerIdMotionOnly    = MSYS_UINT_MAX;
  return Err::m_nOK;
}


ErrVal
H264AVCEncoder::getBaseLayerData( IntFrame*&    pcFrame,
                                  IntFrame*&    pcResidual,
                                  MbDataCtrl*&  pcMbDataCtrl,
                                  Bool&         bConstrainedIPredBL,
                                  Bool&         bForCopyOnly,
                                  Int           iSpatialScalability,
                                  UInt          uiBaseLayerId,
                                  Int           iPoc,
                                  Bool          bMotion )
{
  ROF ( uiBaseLayerId < MAX_LAYERS );

  RNOK( m_apcMCTFEncoder[uiBaseLayerId]->getBaseLayerData( pcFrame, pcResidual, pcMbDataCtrl, bConstrainedIPredBL,
                                                           bForCopyOnly, iSpatialScalability, iPoc, bMotion ) );

  return Err::m_nOK;
}

ErrVal
H264AVCEncoder::getBaseLayerSH( SliceHeader*& rpcSliceHeader,
                                UInt          uiBaseLayerId,
                                Int           iPoc )
{
  ROF( uiBaseLayerId < MAX_LAYERS );

  RNOK( m_apcMCTFEncoder[uiBaseLayerId]->getBaseLayerSH( rpcSliceHeader, iPoc ) );
  return Err::m_nOK;
}


UInt*
H264AVCEncoder::getGOPBitsBase( UInt uiBaseLayerId )
{
  ROFRS( uiBaseLayerId < MAX_LAYERS, 0 );
  return m_apcMCTFEncoder[uiBaseLayerId]->getGOPBitsBase();
}

UInt*
H264AVCEncoder::getGOPBitsFGS( UInt uiBaseLayerId )
{
  ROFRS( uiBaseLayerId < MAX_LAYERS, 0 );
  return m_apcMCTFEncoder[uiBaseLayerId]->getGOPBitsFGS();
}

UInt*
H264AVCEncoder::getGOPBits( UInt uiBaseLayerId )
{
	ROFRS( uiBaseLayerId < MAX_LAYERS , 0 );
	return m_apcMCTFEncoder[uiBaseLayerId]->getGOPBits();
}


ErrVal
H264AVCEncoder::init( MCTFEncoder*      apcMCTFEncoder[MAX_LAYERS],
                      ParameterSetMng*  pcParameterSetMng,
                      PocCalculator*    pcPocCalculator,
                      NalUnitEncoder*   pcNalUnitEncoder,
                      ControlMngIf*     pcControlMng,
                      CodingParameter*  pcCodingParameter,
                      FrameMng*         pcFrameMng)
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

  //{{Adaptive GOP structure
  // --ETRI & KHU
  m_uiGOPOrder = 0;
  m_uiTarget = 0xffffffff;
  //}}Adaptive GOP structure

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


ErrVal
H264AVCEncoder::xWriteScalableSEI( ExtBinDataAccessor* pcExtBinDataAccessor )
{
	//===== create message =====
	SEI::ScalableSei* pcScalableSEI;
	RNOK(SEI::ScalableSei::create(pcScalableSEI) );


	//===== set message =====
	UInt j; //JVT-S036 lsj
	UInt uiInputLayers = m_pcCodingParameter->getNumberOfLayers ();
	UInt uiLayerNum = 0;	//total scalable layer numbers
	for ( UInt i = 0; i < uiInputLayers; i++ )	//calculate total scalable layer numbers
	{
		Bool bH264AVCCompatible = ( i == 0 && m_pcCodingParameter->getBaseLayerMode() > 0 );
		Bool bSubSeq            = ( i == 0 && m_pcCodingParameter->getBaseLayerMode() > 1 );

		LayerParameters& rcLayer = m_pcCodingParameter->getLayerParameters ( i );
		UInt uiTotalTempLevel = rcLayer.getDecompositionStages () - rcLayer.getNotCodedMCTFStages();
// *LMH(20060203): Fix Bug due to underflow (Replace)
		//UInt uiMinTempLevel   = ( !bH264AVCCompatible ||bSubSeq ) ? 0: max( 0, uiTotalTempLevel - 1 );
		UInt uiMinTempLevel   = ( !bH264AVCCompatible ||bSubSeq ) ? 0: max( 0, (Int)uiTotalTempLevel - 1 );
		UInt uiActTempLevel   = uiTotalTempLevel - uiMinTempLevel + 1;
		UInt uiTotalFGSLevel  = (UInt)rcLayer.getNumFGSLayers () + 1;
		uiLayerNum += uiActTempLevel * uiTotalFGSLevel;
	}
	UInt uiTotalScalableLayer = 0;

	//===== get framerate information ===
	Double *dFramerate = dGetFramerate();
  
	UInt uiNumLayersMinus1 = uiLayerNum - 1;

	pcScalableSEI->setNumLayersMinus1 ( uiNumLayersMinus1 );

	UInt uiNumScalableLayer = 0;
	for ( UInt uiCurrLayer = 0; uiCurrLayer < uiInputLayers; uiCurrLayer++)
	{
		LayerParameters& rcLayer = m_pcCodingParameter->getLayerParameters ( uiCurrLayer );
		UInt uiTotalTempLevel = rcLayer.getDecompositionStages () - rcLayer.getNotCodedMCTFStages() + 1;
		UInt uiTotalFGSLevel = (UInt)rcLayer.getNumFGSLayers () + 1;
		//Bool bFGSLayerFlag = uiTotalFGSLevel > 1; //JVT-S036 lsj
		Bool bH264AVCCompatible = ( uiCurrLayer == 0 && m_pcCodingParameter->getBaseLayerMode() > 0 );
		Bool bSubSeq            = ( uiCurrLayer == 0 && m_pcCodingParameter->getBaseLayerMode() > 1 );
// *LMH(20060203): Fix Bug due to underflow (Replace)
		//UInt uiMinTempLevel     = ( !bH264AVCCompatible ||bSubSeq ) ? 0: max(0,uiTotalTempLevel - 2);
		UInt uiMinTempLevel     = ( !bH264AVCCompatible ||bSubSeq ) ? 0: max(0, (Int)uiTotalTempLevel - 2);

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
				  //Bool bInitParameterSetsInfoPresentFlag = false;		//may be changed //JVT-S036 lsj
				  Bool bExactInterayerPredFlag = true;			//JVT-S036 lsj may be changed
  				  Bool bIroiSliceDivisionFlag = false;  //JVT-S036 lsj
				  pcScalableSEI->setLayerId(uiNumScalableLayer, uiNumScalableLayer);
	//JVT-S036 lsj start
				  //pcScalableSEI->setFGSlayerFlag(uiNumScalableLayer, bFGSLayerFlag); 
				  //pcScalableSEI->setSubPicLayerFlag(uiNumScalableLayer,0);				  
					UInt uiTempLevel = uiCurrTempLevel; //BUG_FIX_FT_01_2006
					UInt uiDependencyID = uiCurrLayer;
					UInt uiQualityLevel = uiCurrFGSLevel;
	// BUG_FIX liuhui{
					m_aaauiScalableLayerId[uiCurrLayer][uiCurrTempLevel][uiCurrFGSLevel] = uiNumScalableLayer;
	// BUG_FIX liuhui}					
					UInt uiSimplePriorityId = 0;
					Bool bDiscardableFlag  = false;
					if( uiCurrFGSLevel > rcLayer.getNumFGSLayers() )
						bDiscardableFlag = true;
					pcScalableSEI->setSimplePriorityId(uiNumScalableLayer, uiSimplePriorityId);
					pcScalableSEI->setDiscardableFlag(uiNumScalableLayer, bDiscardableFlag);
					pcScalableSEI->setTemporalLevel(uiNumScalableLayer, uiTempLevel);
					pcScalableSEI->setDependencyId(uiNumScalableLayer, uiDependencyID);
					pcScalableSEI->setQualityLevel(uiNumScalableLayer, uiQualityLevel);				
	 //JVT-S036 lsj end
				  pcScalableSEI->setSubRegionLayerFlag(uiNumScalableLayer, bSubRegionLayerFlag);
				  pcScalableSEI->setIroiSliceDivisionInfoPresentFlag(uiNumScalableLayer, bIroiSliceDivisionFlag); //JVT-S036 lsj
				  pcScalableSEI->setProfileLevelInfoPresentFlag(uiNumScalableLayer, bProfileLevelInfoPresentFlag);
				  pcScalableSEI->setBitrateInfoPresentFlag(uiNumScalableLayer, bBitrateInfoPresentFlag);
				  pcScalableSEI->setFrmRateInfoPresentFlag(uiNumScalableLayer, bFrmRateInfoPresentFlag);
				  pcScalableSEI->setFrmSizeInfoPresentFlag(uiNumScalableLayer, bFrmSizeInfoPresentFlag);
				  pcScalableSEI->setLayerDependencyInfoPresentFlag(uiNumScalableLayer, bLayerDependencyInfoPresentFlag);
				  pcScalableSEI->setInitParameterSetsInfoPresentFlag(uiNumScalableLayer, bInitParameterSetsInfoPresentFlag);

				  pcScalableSEI->setExactInterlayerPredFlag(uiNumScalableLayer, bExactInterayerPredFlag);//JVT-S036 lsj

				  if(pcScalableSEI->getProfileLevelInfoPresentFlag(uiNumScalableLayer))
				  {
					  UInt uilayerProfileIdc = 0;	//may be changed
					  Bool bLayerConstraintSet0Flag = false;	//may be changed
					  Bool bH264AVCCompatibleTmp  = m_pcCodingParameter->getBaseLayerMode() > 0 && uiCurrLayer == 0;
					  Bool bLayerConstraintSet1Flag = ( bH264AVCCompatibleTmp ? 1 : 0 );	//may be changed
					  Bool bLayerConstraintSet2Flag = false;	//may be changed
					  Bool bLayerConstraintSet3Flag = false;	//may be changed
					  UInt uiLayerLevelIdc = 0;		//may be changed

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


	/*			  if(pcScalableSEI->getDecodingDependencyInfoPresentFlag(uiNumScalableLayer))
				  {
					  //UInt uiTempLevel = uiCurrTempLevel - uiMinTempLevel;
					  UInt uiTempLevel = uiCurrTempLevel; //BUG_FIX_FT_01_2006
					  UInt uiDependencyID = uiCurrLayer;
					  UInt uiQualityLevel = uiCurrFGSLevel;
// BUG_FIX liuhui{
					  m_aaauiScalableLayerId[uiCurrLayer][uiCurrTempLevel][uiCurrFGSLevel] = uiNumScalableLayer;
// BUG_FIX liuhui}
					 
					  UInt uiSimplePriorityId = 0;
					  Bool uiDiscardableFlag  = false;

					  pcScalableSEI->setSimplePriorityId(uiNumScalableLayer, uiSimplePriorityId);
					  pcScalableSEI->setDiscardableFlag(uiNumScalableLayer, uiDiscardableFlag);
			
					  pcScalableSEI->setTemporalLevel(uiNumScalableLayer, uiTempLevel);
					  pcScalableSEI->setDependencyId(uiNumScalableLayer, uiDependencyID);
					  pcScalableSEI->setQualityLevel(uiNumScalableLayer, uiQualityLevel);
				  }
 JVT-S036 lsj */
				  if(pcScalableSEI->getBitrateInfoPresentFlag(uiNumScalableLayer))
				  {
// BUG_FIX liuhui{
					  UInt uiAvgBitrate = (UInt)( m_aaadSingleLayerBitrate[uiCurrLayer][uiCurrTempLevel][uiCurrFGSLevel]+0.5 );
// BUG_FIX liuhui}
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
					  UInt uiAvgFrmRate = (UInt)( 256*dFramerate[uiTotalScalableLayer] + 0.5 );

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
					  UInt bIroiSliceDivisionType = 0; //may be changed
					  UInt bNumSliceMinus1 = 1;
					  pcScalableSEI->setIroiSliceDivisionType( uiNumScalableLayer, bIroiSliceDivisionType );
					  if( bIroiSliceDivisionType == 0 )
					  {
						  UInt bGridSliceWidthInMbsMinus1 = 0; //should be changed
						  UInt bGridSliceHeightInMbsMinus1 = 0; //should be changed
						  pcScalableSEI->setGridSliceWidthInMbsMinus1( uiNumScalableLayer, bGridSliceWidthInMbsMinus1 );
						  pcScalableSEI->setGridSliceHeightInMbsMinus1( uiNumScalableLayer, bGridSliceHeightInMbsMinus1 );
					  }
					  else if( bIroiSliceDivisionType == 1 )
					  {
						  bNumSliceMinus1 = 1; //should be changed
						  pcScalableSEI->setNumSliceMinus1( uiNumScalableLayer, bNumSliceMinus1);
						  for ( j = 0; j <= bNumSliceMinus1; j++ )
						  {
							  UInt bFirstMbInSlice = 1;//should be changed
							  UInt bSliceWidthInMbsMinus1 = 1;//should be changed
							  UInt bSliceHeightInMbsMinus1 = 1;//should be changed
							  pcScalableSEI->setFirstMbInSlice( uiNumScalableLayer, j, bFirstMbInSlice );
							  pcScalableSEI->setSliceWidthInMbsMinus1( uiNumScalableLayer, j, bSliceWidthInMbsMinus1 );
							  pcScalableSEI->setSliceHeightInMbsMinus1( uiNumScalableLayer, j, bSliceHeightInMbsMinus1 );
						  }
					  }
					  else if( bIroiSliceDivisionType == 2 )
					  {
						  pcScalableSEI->setNumSliceMinus1( uiNumScalableLayer, bNumSliceMinus1 );
						  UInt uiFrameHeightInMb = pcScalableSEI->getFrmHeightInMbsMinus1( uiNumScalableLayer ) + 1;
						  UInt uiFrameWidthInMb  = pcScalableSEI->getFrmWidthInMbsMinus1(uiNumScalableLayer ) + 1;
						  UInt uiPicSizeInMbs = uiFrameHeightInMb * uiFrameWidthInMb;
						  for ( j = 0; j < uiPicSizeInMbs; j++)
						  {
							  UInt bSliceId = 1; //should be changed
							  pcScalableSEI->setSliceId( uiNumScalableLayer, j, bSliceId );
						  }
					  }
				  }
			  //JVT-S036 lsj end

				  if(pcScalableSEI->getLayerDependencyInfoPresentFlag(uiNumScalableLayer))
				  {
// BUG_FIX liuhui{
					{
					  UInt uiDelta;
					  if( uiCurrFGSLevel ) // FGS layer, Q != 0
					  {
					    uiDelta = uiNumScalableLayer - getScalableLayerId( uiCurrLayer, uiCurrTempLevel, uiCurrFGSLevel-1 );
						pcScalableSEI->setDirectlyDependentLayerIdDeltaMinus1( uiNumScalableLayer, 0, uiDelta );//JVT-S036 lsj
						pcScalableSEI->setNumDirectlyDependentLayers(uiNumScalableLayer, 1 );
						if( uiCurrTempLevel- uiMinTempLevel ) // T != 0
						{
						  uiDelta = uiNumScalableLayer - getScalableLayerId( uiCurrLayer, uiCurrTempLevel-1, uiCurrFGSLevel );
						  pcScalableSEI->setDirectlyDependentLayerIdDeltaMinus1( uiNumScalableLayer, 1, uiDelta );//JVT-S036 lsj
						  pcScalableSEI->setNumDirectlyDependentLayers(uiNumScalableLayer, 2 );
						}
					  }
					  else if( ( uiCurrTempLevel- uiMinTempLevel ) ) // Q = 0, T != 0					    
					  {
					    uiDelta = uiNumScalableLayer - getScalableLayerId( uiCurrLayer, uiCurrTempLevel-1, uiCurrFGSLevel );
						pcScalableSEI->setDirectlyDependentLayerIdDeltaMinus1( uiNumScalableLayer, 0, uiDelta ); //JVT-S036 lsj
						pcScalableSEI->setNumDirectlyDependentLayers( uiNumScalableLayer, 1 );
						if( uiCurrLayer ) // D != 0, T != 0, Q = 0
						{
						  UInt uiBaseLayerId = rcLayer.getBaseLayerId();
						  LayerParameters& rcBaseLayer = m_pcCodingParameter->getLayerParameters ( uiBaseLayerId );
						  UInt uiBaseFGSLayers = (UInt)( rcBaseLayer.getNumFGSLayers() );
						  UInt uiBaseQualityLevel = rcLayer.getBaseQualityLevel();
						  uiBaseQualityLevel = min( uiBaseQualityLevel, uiBaseFGSLayers );
						  if( uiBaseLayerId == 0 && m_pcCodingParameter->getBaseLayerMode() == 1 ) // AVC-COMPATIBLE
						  {
						    UInt uiBaseTempLevel = max( 0, rcBaseLayer.getDecompositionStages() - rcBaseLayer.getNotCodedMCTFStages() - 1 );
							if( uiCurrTempLevel-uiMinTempLevel >= uiBaseTempLevel )
							{
							  if( MSYS_UINT_MAX != getScalableLayerId( uiBaseLayerId, uiCurrTempLevel, uiBaseQualityLevel ) )
							  {
							  uiDelta = uiNumScalableLayer - getScalableLayerId( uiBaseLayerId, uiCurrTempLevel, uiBaseQualityLevel );
							   pcScalableSEI->setDirectlyDependentLayerIdDeltaMinus1( uiNumScalableLayer, 1, uiDelta );//JVT-S036 lsj
							  pcScalableSEI->setNumDirectlyDependentLayers( uiNumScalableLayer, 2 );
							  }
							}
							else 
							{
							  if( MSYS_UINT_MAX != getScalableLayerId( uiBaseLayerId, uiBaseTempLevel, uiBaseQualityLevel ) )
							  { //this should always be true
							    uiDelta = uiNumScalableLayer - getScalableLayerId( uiBaseLayerId, uiBaseTempLevel, uiBaseQualityLevel );
							    pcScalableSEI->setDirectlyDependentLayerIdDeltaMinus1( uiNumScalableLayer, 1, uiDelta ); //JVT-S036 lsj
							    pcScalableSEI->setNumDirectlyDependentLayers( uiNumScalableLayer, 2 );
							  }
							}
						  }
						  else //non-AVC mode
						  {
						    if( MSYS_UINT_MAX != getScalableLayerId( uiBaseLayerId, uiCurrTempLevel, uiBaseQualityLevel ) )
							{
						      uiDelta = uiNumScalableLayer - getScalableLayerId( uiBaseLayerId, uiCurrTempLevel, uiBaseQualityLevel );
						      pcScalableSEI->setDirectlyDependentLayerIdDeltaMinus1( uiNumScalableLayer, 1, uiDelta ); //JVT-S036 lsj
						      pcScalableSEI->setNumDirectlyDependentLayers( uiNumScalableLayer, 2 );
							}
						  }
						}
					  }
					  else if ( uiCurrLayer ) // D != 0,T = 0, Q = 0
					  {
						UInt uiBaseLayerId = rcLayer.getBaseLayerId();
						LayerParameters& rcBaseLayer = m_pcCodingParameter->getLayerParameters ( uiBaseLayerId );
						UInt uiBaseFGSLayers = (UInt)( rcBaseLayer.getNumFGSLayers() );
						UInt uiBaseQualityLevel = rcLayer.getBaseQualityLevel();
						uiBaseQualityLevel = min( uiBaseQualityLevel, uiBaseFGSLayers );
						if( uiBaseLayerId == 0 && m_pcCodingParameter->getBaseLayerMode() == 1 ) //AVC-COMPATIBLE
						{
						  Int iBaseTempLevel = max( 0, (Int)( rcBaseLayer.getDecompositionStages() - rcBaseLayer.getNotCodedMCTFStages() ) - 1 );
						  uiDelta = uiNumScalableLayer - getScalableLayerId( uiBaseLayerId, (UInt)iBaseTempLevel, (UInt)uiBaseQualityLevel );
						}
						else
						  uiDelta = uiNumScalableLayer - getScalableLayerId( uiBaseLayerId, uiCurrTempLevel, uiBaseQualityLevel );
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

				  uiNumScalableLayer++;
				}
				uiTotalScalableLayer++;
			}
		}

	}

	UInt              uiBits = 0;
	SEI::MessageList  cSEIMessageList;
	cSEIMessageList.push_back                       ( pcScalableSEI );
	RNOK( m_pcNalUnitEncoder  ->initNalUnit         ( pcExtBinDataAccessor ) );
	RNOK( m_pcNalUnitEncoder  ->write               ( cSEIMessageList ) );
	RNOK( m_pcNalUnitEncoder  ->closeNalUnit        ( uiBits ) );
	RNOK( m_apcMCTFEncoder[0] ->addParameterSetBits ( uiBits+4*8 ) );

	return Err::m_nOK;

}
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
H264AVCEncoder::writeParameterSets( ExtBinDataAccessor* pcExtBinDataAccessor, Bool &rbMoreSets )
{
  if( m_bVeryFirstCall )
  {
    m_bVeryFirstCall = false;

    RNOK( xInitParameterSets() );
    if( m_bScalableSeiMessage )
    RNOK( xWriteScalableSEI( pcExtBinDataAccessor ) );

    return Err::m_nOK;
  }
  else
    m_bScalableSeiMessage = true;
    
  UInt uiBits;

  if( ! m_cUnWrittenSPS.empty() )
  {
    RNOK( m_pcNalUnitEncoder->initNalUnit( pcExtBinDataAccessor ) );
    SequenceParameterSet& rcSPS = *m_cUnWrittenSPS.front();
    // Copy simple priority ID mapping into SPS
   /* rcSPS.setNalUnitExtFlag ( m_pcCodingParameter->getExtendedPriorityId() );
    for ( UInt uiPriId = 0; uiPriId < m_pcCodingParameter->getNumSimplePris(); uiPriId++)
    {
        UInt uiTempLevel, uiLayer, uiQualLevel;
        m_pcCodingParameter->getSimplePriorityMap ( uiPriId, uiTempLevel, uiLayer, uiQualLevel );
        rcSPS.setSimplePriorityMap( uiPriId, uiTempLevel, uiLayer, uiQualLevel );
    }
    rcSPS.setNumSimplePriIdVals( m_pcCodingParameter->getNumSimplePris() );
 JVT-S036 lsj */
    RNOK( m_pcNalUnitEncoder->write( rcSPS ) );
    RNOK( m_pcNalUnitEncoder->closeNalUnit( uiBits ) );

    if( m_pcCodingParameter->getNumberOfLayers() )
    {
      UInt  uiLayer = rcSPS.getLayerId();
      UInt  uiSize  = pcExtBinDataAccessor->size();
      RNOK( m_apcMCTFEncoder[uiLayer]->addParameterSetBits( 8*(uiSize+4) ) );
    }
    
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
  UInt  uiLayer;
	UInt	uiScalableLayer = 0;
  UInt  uiNumLayers = m_pcCodingParameter->getNumberOfLayers();

  //{{Adaptive GOP structure
  // --ETRI & KHU
  if ( !m_pcCodingParameter->getUseAGS() ) {
  //}}Adaptive GOP structure

  for( uiLayer = 0; uiLayer < uiNumLayers; uiLayer++ )
  {
    if( apcOriginalPicBuffer[uiLayer] )
    {
      RNOK( xProcessGOP( uiLayer,
 												 uiScalableLayer,
                         apcOriginalPicBuffer   [uiLayer],
                         apcReconstructPicBuffer[uiLayer],
                         apcPicBufferOutputList [uiLayer],
                         apcPicBufferUnusedList [uiLayer] ) );
    }
    else
    {
      if( apcReconstructPicBuffer[uiLayer] )
      {
        apcPicBufferUnusedList[uiLayer].push_back( apcReconstructPicBuffer[uiLayer] );
      }
    }
  }

  m_cAccessUnitList.emptyNALULists( rcExtBinDataAccessorList );

  //{{Adaptive GOP structure
  // --ETRI & KHU
  }
  else 
  {
    if ( m_pcCodingParameter->getWriteGOPMode() ) 
    {
      m_uiGOPOrder = 0xffffffff;	
      for( uiLayer = 0; uiLayer < uiNumLayers; uiLayer++ )
      {
        if( apcOriginalPicBuffer[uiLayer] )
        {
          RNOK( xProcessGOP( uiLayer,
														 uiScalableLayer,
                             apcOriginalPicBuffer   [uiLayer],
                             apcReconstructPicBuffer[uiLayer],
                             apcPicBufferOutputList [uiLayer],
                             apcPicBufferUnusedList [uiLayer] ) );
        }
        else
        {
          if( apcReconstructPicBuffer[uiLayer] )
          {
            apcPicBufferUnusedList[uiLayer].push_back( apcReconstructPicBuffer[uiLayer] );
          }
        }
      }

      m_cAccessUnitList.emptyNALULists( rcExtBinDataAccessorList );
    }
    else 
    {
      if ((1 == (m_uiTarget - m_acOrgPicBufferList[uiNumLayers-1].size()) 
          && apcOriginalPicBuffer[uiNumLayers-1])|| (m_uiTarget == m_acOrgPicBufferList[uiNumLayers-1].size()))
      {

				int order = 0;
				int i;

				for (i = 0; i < 8 && (m_apcMCTFEncoder[0]->getSelect(m_uiGOPOrder,i ) ); i++) {
					if ( m_apcMCTFEncoder[0]->getSelect(m_uiGOPOrder,i) )
						order++;
				}

				m_bGOPDone = false;
			
			  for (i = 0; i < order; i++) 
        {
				
					if (i == order-1 ) {
						m_bGOPDone = true;
					}
					for( uiLayer = 0; uiLayer < uiNumLayers; uiLayer++ )
          {
            m_apcMCTFEncoder[uiLayer]->setSelectPos(i);
            if( apcOriginalPicBuffer[uiLayer] )
            {
              RNOK( xProcessGOP( uiLayer,
 																 uiScalableLayer,
                                 apcOriginalPicBuffer   [uiLayer],
                                 apcReconstructPicBuffer[uiLayer],
                                 apcPicBufferOutputList [uiLayer],
                                 apcPicBufferUnusedList [uiLayer] ) );
            }
            else
            {
              if( apcReconstructPicBuffer[uiLayer] )
              {
                apcPicBufferUnusedList[uiLayer].push_back( apcReconstructPicBuffer[uiLayer] );
              }
            }
          }
          m_cAccessUnitList.emptyNALULists( rcExtBinDataAccessorList ); //!! ??
				}
			  m_uiTarget = 0xffffffff;
				m_uiGOPOrder++;
			}
			else {
        for( uiLayer = 0; uiLayer < uiNumLayers; uiLayer++ )
        {
          if( apcOriginalPicBuffer[uiLayer] )
          {
            RNOK( xProcessGOP( uiLayer,
															 uiScalableLayer,
                               apcOriginalPicBuffer   [uiLayer],
                               apcReconstructPicBuffer[uiLayer],
                               apcPicBufferOutputList [uiLayer],
                               apcPicBufferUnusedList [uiLayer] ) );
          }
          else
          {
            if( apcReconstructPicBuffer[uiLayer] )
            {
              apcPicBufferUnusedList[uiLayer].push_back( apcReconstructPicBuffer[uiLayer] );
            }
          }
        }
        m_cAccessUnitList.emptyNALULists( rcExtBinDataAccessorList ); //!! ??
			}
    }

  }
  //}}Adaptive GOP structure

  return Err::m_nOK;
}


ErrVal
H264AVCEncoder::finish( ExtBinDataAccessorList&  rcExtBinDataAccessorList,
                        PicBufferList*           apcPicBufferOutputList,
                        PicBufferList*           apcPicBufferUnusedList,
                        UInt&                    ruiNumCodedFrames,
                        Double&                  rdHighestLayerOutputRate )
{
  UInt  uiLayer;
	UInt	uiScalableLayer = 0;
  UInt  uiNumLayers = m_pcCodingParameter->getNumberOfLayers();

  //{{Adaptive GOP structure
  // --ETRI & KHU
  if ( !m_pcCodingParameter->getUseAGS() ) {
  //}}Adaptive GOP structure

  for( uiLayer = 0; uiLayer < uiNumLayers; uiLayer++ )
  {
    RNOK( xProcessGOP( uiLayer,
 											 uiScalableLayer,
                       NULL,
                       NULL,
                       apcPicBufferOutputList[uiLayer],
                       apcPicBufferUnusedList[uiLayer] ) );
  }

  m_cAccessUnitList.emptyNALULists( rcExtBinDataAccessorList );
  
  for( uiLayer = 0; uiLayer < m_pcCodingParameter->getNumberOfLayers(); uiLayer++ )
  {
    RNOK( m_apcMCTFEncoder[uiLayer]->finish( ruiNumCodedFrames, rdHighestLayerOutputRate ,
																						 dGetFramerate(), dGetBitrate(), m_aaauidSeqBits ) );
// BUG_FIX liuhui{
	RNOK( m_apcMCTFEncoder[uiLayer]->SingleLayerFinish( m_aaauidSeqBits, m_aaadSingleLayerBitrate) );
// BUG_FIX liuhui}
 }
  printf("\n");

  //{{Adaptive GOP structure
  // --ETRI & KHU
  }
  else
  {
    if (m_pcCodingParameter->getWriteGOPMode())
    {
      for( uiLayer = 0; uiLayer < uiNumLayers; uiLayer++ )
      {
        RNOK( xProcessGOP( uiLayer,
													 uiScalableLayer,
                           NULL,
                           NULL,
                           apcPicBufferOutputList[uiLayer],
                           apcPicBufferUnusedList[uiLayer] ) );
      }

      m_cAccessUnitList.emptyNALULists( rcExtBinDataAccessorList );
  
      for( uiLayer = 0; uiLayer < m_pcCodingParameter->getNumberOfLayers(); uiLayer++ )
      {
        RNOK( m_apcMCTFEncoder[uiLayer]->finish( ruiNumCodedFrames, rdHighestLayerOutputRate,
																								 dGetFramerate(), dGetBitrate(), m_aaauidSeqBits ) );
// BUG_FIX liuhui{
	    RNOK( m_apcMCTFEncoder[uiLayer]->SingleLayerFinish( m_aaauidSeqBits, m_aaadSingleLayerBitrate) );
// BUG_FIX liuhui}
      }
      printf("\n");
    }
    else
    {
      int order = 0;
		  int i;
		  
		  for (i = 0; i < 8 && (m_apcMCTFEncoder[0]->getSelect(m_uiGOPOrder,i ) ); i++) {
			  if ( m_apcMCTFEncoder[0]->getSelect(m_uiGOPOrder,i) )
				  order++;
		  }

		  m_bGOPDone = false;
		  
		  for (i = 0; i < order; i++) {
			  
			  if (i == order-1 ) {
				  m_bGOPDone = true;
			  }
        for( uiLayer = 0; uiLayer < uiNumLayers; uiLayer++ )
        {
          m_apcMCTFEncoder[uiLayer]->setSelectPos(i);
          RNOK( xProcessGOP( uiLayer,
 														 uiScalableLayer,
                             NULL,
                             NULL,
                             apcPicBufferOutputList[uiLayer],
                             apcPicBufferUnusedList[uiLayer] ) );
        }
        
        m_cAccessUnitList.emptyNALULists( rcExtBinDataAccessorList );
      }  
      for( uiLayer = 0; uiLayer < m_pcCodingParameter->getNumberOfLayers(); uiLayer++ )
      {
        RNOK( m_apcMCTFEncoder[uiLayer]->finish( ruiNumCodedFrames, rdHighestLayerOutputRate,
																								 dGetFramerate(), dGetBitrate(), m_aaauidSeqBits ) );
// BUG_FIX liuhui{
	    RNOK( m_apcMCTFEncoder[uiLayer]->SingleLayerFinish( m_aaauidSeqBits, m_aaadSingleLayerBitrate) );
// BUG_FIX liuhui}     
     }
      printf("\n");
    }
  }
  //}}Adaptive GOP structure

  return Err::m_nOK;
}



ErrVal
H264AVCEncoder::xProcessGOP( UInt                     uiLayer,
														 UInt&										uiScalableLayer,
                             PicBuffer*               pcOriginalPicBuffer,
                             PicBuffer*               pcReconstructPicBuffer,
                             PicBufferList&           rcPicBufferOutputList,
                             PicBufferList&           rcPicBufferUnusedList )
{ 
	//NonRequired JVT-Q066 (06-04-08){{
	if(m_pcCodingParameter->getNonRequiredEnable())
	{
		if(uiLayer == m_pcCodingParameter->getNumberOfLayers() - 1)
			m_apcMCTFEncoder[uiLayer]->setNonRequiredWrite(2);
		else
			m_apcMCTFEncoder[uiLayer]->setNonRequiredWrite(1);
	}
	//NonRequired JVT-Q066 (06-04-08)}}

  //{{Adaptive GOP structure
  // --ETRI & KHU
  if ( !m_pcCodingParameter->getUseAGS() ) {
  //}}Adaptive GOP structure

  if( pcOriginalPicBuffer == NULL )
  {
    //===== FINISH ENCODING =====
    if( pcReconstructPicBuffer )
    {
      rcPicBufferUnusedList.push_back( pcReconstructPicBuffer );
    }

    if( m_acOrgPicBufferList[uiLayer].size() )
    {
// BUG_FIX liuhui{
		if( uiLayer && uiScalableLayer == 0 )
			m_apcMCTFEncoder[uiLayer]->setScalableLayer( m_apcMCTFEncoder[uiLayer-1]->getScalableLayer() );
		else
// BUG_FIX liuhui}
			m_apcMCTFEncoder[uiLayer]->setScalableLayer( uiScalableLayer );
      RNOK( m_apcMCTFEncoder[uiLayer]->process ( m_cAccessUnitList,
                                                 m_acOrgPicBufferList[uiLayer],
                                                 m_acRecPicBufferList[uiLayer],
                                                 rcPicBufferUnusedList,
																								 m_aaauidSeqBits ) );
   		uiScalableLayer = m_apcMCTFEncoder[uiLayer]->getScalableLayer();
      rcPicBufferOutputList += m_acRecPicBufferList[uiLayer];
    }

    rcPicBufferUnusedList += m_acOrgPicBufferList[uiLayer];
    rcPicBufferUnusedList += m_acRecPicBufferList[uiLayer];

    m_acOrgPicBufferList[uiLayer].clear();
    m_acRecPicBufferList[uiLayer].clear();

    return Err::m_nOK;
  }

  
  //===== INSERT BUFFERS =====
  m_acOrgPicBufferList[uiLayer].push_back( pcOriginalPicBuffer );
  m_acRecPicBufferList[uiLayer].push_back( pcReconstructPicBuffer );

  
  UInt  uiTargetBufferSize = ( 1 << m_pcCodingParameter->getLayerParameters(uiLayer).getDecompositionStages() );
  if( ! m_apcMCTFEncoder[uiLayer]->firstGOPCoded() )
  {
    uiTargetBufferSize++;
  }
  ROT( m_acOrgPicBufferList[uiLayer].size() > uiTargetBufferSize );


  if( m_acOrgPicBufferList[uiLayer].size() == uiTargetBufferSize )
  {
    //===== ENCODE GROUP OF PICTURES =====
    m_apcMCTFEncoder[uiLayer]->setScalableLayer( uiScalableLayer );
    RNOK( m_apcMCTFEncoder[uiLayer]->process( m_cAccessUnitList,
                                              m_acOrgPicBufferList[uiLayer],
                                              m_acRecPicBufferList[uiLayer],
                                              rcPicBufferUnusedList,
																							m_aaauidSeqBits ) );
		uiScalableLayer = m_apcMCTFEncoder[uiLayer]->getScalableLayer();

    //----- set output list -----
    rcPicBufferOutputList += m_acRecPicBufferList[uiLayer];

    //----- update unused list -----
    rcPicBufferUnusedList += m_acOrgPicBufferList[uiLayer];
    rcPicBufferUnusedList += m_acRecPicBufferList[uiLayer];

    //----- reset orig and reconstructed list -----
    m_acOrgPicBufferList[uiLayer].clear();
    m_acRecPicBufferList[uiLayer].clear();
  }

  //{{Adaptive GOP structure
  // --ETRI & KHU
  }
  else
  {
		if( pcOriginalPicBuffer == NULL )
		{
			//===== FINISH ENCODING =====
			if( pcReconstructPicBuffer )
			{
				rcPicBufferUnusedList.push_back( pcReconstructPicBuffer );
			}

      if ( m_pcCodingParameter->getWriteGOPMode() ) { // mode decision

				if( m_acOrgPicBufferList[uiLayer].size() )
				{
					RNOK( m_apcMCTFEncoder[uiLayer]->process_ags ( m_cAccessUnitList,
                                                         m_acOrgPicBufferList[uiLayer],
                                                         m_acRecPicBufferList[uiLayer],
                                                         rcPicBufferUnusedList,
																												 m_aaauidSeqBits ) );
					rcPicBufferOutputList += m_acRecPicBufferList[uiLayer];
				}
				
				rcPicBufferUnusedList += m_acOrgPicBufferList[uiLayer];
				rcPicBufferUnusedList += m_acRecPicBufferList[uiLayer];
				
				m_acOrgPicBufferList[uiLayer].clear();
				m_acRecPicBufferList[uiLayer].clear();
			}
			else {
				if ( !m_bGOPDone ) 
        {
					PicBufferList templist;
					templist +=	rcPicBufferUnusedList;
					rcPicBufferUnusedList.clear();
					
					if( m_acOrgPicBufferList[uiLayer].size() ) { // !!!!!
						RNOK( m_apcMCTFEncoder[uiLayer]->process_ags ( m_cAccessUnitList,
                                                           m_acOrgPicBufferList[uiLayer],
                                                           m_acRecPicBufferList[uiLayer],
                                                           rcPicBufferUnusedList,
																													 m_aaauidSeqBits ) );
						rcPicBufferOutputList += m_acRecPicBufferList[uiLayer];
				  }
          m_acRecPicBufferList[uiLayer] += rcPicBufferUnusedList;
				  rcPicBufferUnusedList.clear();
					rcPicBufferUnusedList += templist;
					
				}
				else 
        {
					PicBufferList templist;
					PicBufferList templist2;
					templist +=	rcPicBufferUnusedList;
					rcPicBufferUnusedList.clear();

					if( m_acOrgPicBufferList[uiLayer].size() ) {	// !!!!!
					//===== ENCODE GROUP OF PICTURES =====
						RNOK( m_apcMCTFEncoder[uiLayer]->process_ags ( m_cAccessUnitList,
                                                           m_acOrgPicBufferList[uiLayer],
                                                           m_acRecPicBufferList[uiLayer],
                                                           rcPicBufferUnusedList,
																													 m_aaauidSeqBits ) );
						
						//----- set output list -----
						rcPicBufferOutputList += m_acRecPicBufferList[uiLayer];
						templist2 += rcPicBufferUnusedList;
						rcPicBufferUnusedList.clear();
						rcPicBufferUnusedList += templist;
						
						
					}
          //----- update unused list -----
          rcPicBufferUnusedList += m_acOrgPicBufferList[uiLayer];
					rcPicBufferUnusedList += templist2;
					rcPicBufferUnusedList += m_acRecPicBufferList[uiLayer];
					
					//----- reset orig and reconstructed list -----
					m_acOrgPicBufferList[uiLayer].clear();
					m_acRecPicBufferList[uiLayer].clear();
				}
			}
			
			return Err::m_nOK;
		}
		
		UInt  uiTargetBufferSize = ( 1 << m_pcCodingParameter->getLayerParameters(uiLayer).getDecompositionStages() );

		if( ! m_apcMCTFEncoder[uiLayer]->firstGOPCoded() )
		{
			uiTargetBufferSize++;
		}
		
		UInt add_size = 0;
		if (!m_uiGOPOrder && !(uiTargetBufferSize%2))
			add_size = 1;
		
		if (uiTargetBufferSize + add_size != m_acOrgPicBufferList[uiLayer].size()) {
			//===== INSERT BUFFERS =====
			m_acOrgPicBufferList[uiLayer].push_back( pcOriginalPicBuffer );
			m_acRecPicBufferList[uiLayer].push_back( pcReconstructPicBuffer );
		
			ROT( m_acOrgPicBufferList[uiLayer].size() > uiTargetBufferSize );
			
			if (uiLayer == (m_pcCodingParameter->getNumberOfLayers()-1))
				m_uiTarget = uiTargetBufferSize;
		}

		if( m_acOrgPicBufferList[uiLayer].size() == uiTargetBufferSize + add_size)
		{
			
			if (m_pcCodingParameter->getWriteGOPMode()) {
				
				//===== ENCODE GROUP OF PICTURES =====
				RNOK( m_apcMCTFEncoder[uiLayer]->process_ags ( m_cAccessUnitList,
                                                       m_acOrgPicBufferList[uiLayer],
                                                       m_acRecPicBufferList[uiLayer],
                                                       rcPicBufferUnusedList,
																											 m_aaauidSeqBits ) );
				
				//----- set output list -----
				rcPicBufferOutputList += m_acRecPicBufferList[uiLayer];
				
				//----- update unused list -----
				rcPicBufferUnusedList += m_acOrgPicBufferList[uiLayer];
				rcPicBufferUnusedList += m_acRecPicBufferList[uiLayer];
				
				//----- reset orig and reconstructed list -----
				m_acOrgPicBufferList[uiLayer].clear();
				m_acRecPicBufferList[uiLayer].clear();
				
				
			}
			else {
				if ( !m_bGOPDone ) {
					PicBufferList templist;
					templist +=	rcPicBufferUnusedList;
					rcPicBufferUnusedList.clear();
					
					RNOK( m_apcMCTFEncoder[uiLayer]->process_ags ( m_cAccessUnitList,
                                                         m_acOrgPicBufferList[uiLayer],
                                                         m_acRecPicBufferList[uiLayer],
                                                         rcPicBufferUnusedList,
																												 m_aaauidSeqBits ) );
					rcPicBufferOutputList += m_acRecPicBufferList[uiLayer];
					m_acRecPicBufferList[uiLayer] += rcPicBufferUnusedList;
					rcPicBufferUnusedList.clear();
					rcPicBufferUnusedList += templist;
					
				}
				else {
					PicBufferList templist;
					PicBufferList templist2;
					templist +=	rcPicBufferUnusedList;
					rcPicBufferUnusedList.clear();
					//===== ENCODE GROUP OF PICTURES =====
					RNOK( m_apcMCTFEncoder[uiLayer]->process_ags ( m_cAccessUnitList,
                                                         m_acOrgPicBufferList[uiLayer],
                                                         m_acRecPicBufferList[uiLayer],
                                                         rcPicBufferUnusedList,
																												 m_aaauidSeqBits ) );
					
					//----- set output list -----
					rcPicBufferOutputList += m_acRecPicBufferList[uiLayer];
					templist2 += rcPicBufferUnusedList;
					rcPicBufferUnusedList.clear();
					rcPicBufferUnusedList += templist;
					//----- update unused list -----
					rcPicBufferUnusedList += m_acOrgPicBufferList[uiLayer];
					rcPicBufferUnusedList += templist2;
					rcPicBufferUnusedList += m_acRecPicBufferList[uiLayer];
					
					//----- reset orig and reconstructed list -----
					m_acOrgPicBufferList[uiLayer].clear();
					m_acRecPicBufferList[uiLayer].clear();
				}
			}
		}
  }
  //}}Adaptive GOP structure

  return Err::m_nOK;
}


ErrVal
H264AVCEncoder::xInitParameterSets()
{
  UInt uiSPSId = 0;
  UInt uiPPSId = 0;
  UInt uiIndex;


  //===== determine values for POC calculation =====
  UInt uiMaxResolutionStages  = m_pcCodingParameter->getDecompositionStages();  
  UInt uiRequiredPocBits      = max( 4, 1 + (Int)ceil( log10( 1.0 + ( 1 << uiMaxResolutionStages ) ) / log10( 2.0 ) ) );


  //===== loop over layers =====
  for( uiIndex = 0; uiIndex < m_pcCodingParameter->getNumberOfLayers(); uiIndex++ )
  {
    //===== get configuration parameters =====
    LayerParameters&  rcLayerParameters   = m_pcCodingParameter->getLayerParameters( uiIndex );
    Bool              bH264AVCCompatible  = m_pcCodingParameter->getBaseLayerMode() > 0 && uiIndex == 0;
    UInt              uiMbY               = rcLayerParameters.getFrameHeight() / 16;
    UInt              uiMbX               = rcLayerParameters.getFrameWidth () / 16;
    UInt              uiOutFreq           = (UInt)ceil( rcLayerParameters.getOutputFrameRate() );
    UInt              uiMvRange           = m_pcCodingParameter->getMotionVectorSearchParams().getSearchRange() / 4;
    UInt              uiDPBSize           = ( 1 << max( 1, rcLayerParameters.getDecompositionStages() ) ) 

											                      //{{Adaptive GOP structure -- 10.18.2005
                                            // --ETRI & KHU
                                            + (m_pcCodingParameter->getUseAGS() && !bH264AVCCompatible? 20: 0);
                                            //}}Adaptive GOP structure -- 10.18.2005
    UInt              uiNumRefPic         = uiDPBSize; 
    UInt              uiLevelIdc          = SequenceParameterSet::getLevelIdc( uiMbY, uiMbX, uiOutFreq, uiMvRange, uiDPBSize );
    ROT( bH264AVCCompatible && uiDPBSize > 16 );
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
    pcSPS->setRequiredFrameNumUpdateBehaviourFlag ( true );
    pcSPS->setFrameWidthInMbs                     ( uiMbX );
    pcSPS->setFrameHeightInMbs                    ( uiMbY );
    pcSPS->setDirect8x8InferenceFlag              ( true  );
    // TMM_ESS 
    pcSPS->setResizeParameters                    (rcLayerParameters.getResizeParameters());

#if MULTIPLE_LOOP_DECODING
    pcSPS->setAlwaysDecodeBaseLayer               ( rcLayerParameters.getInterLayerPredictionMode() > 0 && 
                                                    rcLayerParameters.getDecodingLoops() > 1 );
#endif
    if(rcLayerParameters.getFGSCodingMode() == 0)
    {
      pcSPS->setFGSCodingMode                     ( false );
    }
    else
    {
      pcSPS->setFGSCodingMode                     ( true );
    }
    pcSPS->setGroupingSize                        ( rcLayerParameters.getGroupingSize() );
    for( UInt ui = 0; ui < 16; ui++ )
    {
      pcSPS->setPosVect                           ( ui, rcLayerParameters.getPosVect(ui) );
    }
#if INDEPENDENT_PARSING
    pcSPS->setIndependentParsing                  ( rcLayerParameters.getIndependentParsing() > 0 );
#endif

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
  LayerParameters&  rcLayerParameters   = m_pcCodingParameter->getLayerParameters( uiIndex );
  Bool              bH264AVCCompatible  = m_pcCodingParameter->getBaseLayerMode() > 0 && uiIndex == 0;
  if(bH264AVCCompatible && m_pcCodingParameter->getNumberOfLayers() == 1 && rcLayerParameters.getNumFGSLayers() > 0)
  {
    UInt              uiMbY               = rcLayerParameters.getFrameHeight() / 16;
    UInt              uiMbX               = rcLayerParameters.getFrameWidth () / 16;
    UInt              uiOutFreq           = (UInt)ceil( rcLayerParameters.getOutputFrameRate() );
    UInt              uiMvRange           = m_pcCodingParameter->getMotionVectorSearchParams().getSearchRange() / 4;
    UInt              uiDPBSize           = ( 1 << max( 1, rcLayerParameters.getDecompositionStages() ) ) 

											                      //{{Adaptive GOP structure -- 10.18.2005
                                            // --ETRI & KHU
                                            + (m_pcCodingParameter->getUseAGS() && !bH264AVCCompatible? 20: 0);
                                            //}}Adaptive GOP structure -- 10.18.2005
    UInt              uiNumRefPic         = uiDPBSize; 
    UInt              uiLevelIdc          = SequenceParameterSet::getLevelIdc( uiMbY, uiMbX, uiOutFreq, uiMvRange, uiDPBSize );
    ROT( bH264AVCCompatible && uiDPBSize > 16 );
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
    pcSPS->setProfileIdc                          ( SCALABLE_PROFILE );
    pcSPS->setConstrainedSet0Flag                 ( false );
    pcSPS->setConstrainedSet1Flag                 ( bH264AVCCompatible ? 1 : 0 );
    pcSPS->setConstrainedSet2Flag                 ( false );
    pcSPS->setConstrainedSet3Flag                 ( false );
    pcSPS->setLevelIdc                            ( uiLevelIdc );
    pcSPS->setSeqScalingMatrixPresentFlag         ( rcLayerParameters.getAdaptiveTransform() > 1 );
    pcSPS->setLog2MaxFrameNum                     ( MAX_FRAME_NUM_LOG2 );
    pcSPS->setLog2MaxPicOrderCntLsb               ( min( 15, uiRequiredPocBits + 2 ) );  // HS: decoder robustness -> value increased by 2
    pcSPS->setNumRefFrames                        ( uiNumRefPic );
    pcSPS->setRequiredFrameNumUpdateBehaviourFlag ( true );
    pcSPS->setFrameWidthInMbs                     ( uiMbX );
    pcSPS->setFrameHeightInMbs                    ( uiMbY );
    pcSPS->setDirect8x8InferenceFlag              ( true  );
    // TMM_ESS 
    pcSPS->setResizeParameters                    (rcLayerParameters.getResizeParameters());

#if MULTIPLE_LOOP_DECODING
    pcSPS->setAlwaysDecodeBaseLayer               ( rcLayerParameters.getInterLayerPredictionMode() > 0 && 
                                                    rcLayerParameters.getDecodingLoops() > 1 );
#endif
    if(rcLayerParameters.getFGSCodingMode() == 0)
    {
      pcSPS->setFGSCodingMode                     ( false );
    }
    else
    {
      pcSPS->setFGSCodingMode                     ( true );
    }
    pcSPS->setGroupingSize                        ( rcLayerParameters.getGroupingSize() );
    for( UInt ui = 0; ui < 16; ui++ )
    {
      pcSPS->setPosVect                           ( ui, rcLayerParameters.getPosVect(ui) );
    }
#if INDEPENDENT_PARSING
    pcSPS->setIndependentParsing                  ( rcLayerParameters.getIndependentParsing() > 0 );
#endif

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
    RNOK( m_pcControlMng->initParameterSetsForFGS( *pcSPS, *pcPPSLP, *pcPPSHP ) );
  }

  //===== set unwritten parameter lists =====
  RNOK( m_pcParameterSetMng->setParamterSetList( m_cUnWrittenSPS, m_cUnWrittenPPS ) );

  return Err::m_nOK;
}



H264AVC_NAMESPACE_END
