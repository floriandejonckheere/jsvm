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




#if !defined(AFX_GOPENCODER_H__75F41F36_C28D_41F9_AB5E_4C90D66D160C__INCLUDED_)
#define AFX_GOPENCODER_H__75F41F36_C28D_41F9_AB5E_4C90D66D160C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000





#include "H264AVCCommonLib/TraceFile.h"
#include "H264AVCCommonLib/MbDataCtrl.h"
// JVT-V068 HRD {
#include "H264AVCCommonLib/ParameterSetMng.h"
// JVT-V068 HRD }
#include "DownConvert.h"
#include "H264AVCCommonLib/Sei.h"  //NonRequired JVT-Q066 (06-04-08)

#include <algorithm>
#include <list>

#if defined( WIN32 )
# pragma warning( disable: 4251 )
#endif


H264AVC_NAMESPACE_BEGIN


class H264AVCEncoder;
class SliceHeader;
class SliceEncoder;
class FrameMng;
class PocCalculator;
class LoopFilter;
class CodingParameter;
class LayerParameters;
class RateDistortionIf;
class HeaderSymbolWriteIf;
class NalUnitEncoder;
class ControlMngIf;
class ParameterSetMng;
class ControlMngH264AVCEncoder;
class MotionEstimation;
class IntFrame;
//JVT-U106 Behaviour at slice boundaries{
class ReconstructionBypass;
//JVT-U106 Behaviour at slice boundaries}

// JVT-V068 {
class Scheduler;
// JVT-V068 }

typedef MyList<UInt>        UIntList;


class H264AVCENCODERLIB_API AccessUnit
{
public:
  AccessUnit  ( Int         iPoc )  : m_iPoc( iPoc )   
  , m_pcNonRequiredSei ( NULL )  //NonRequired JVT-Q066 (06-04-08)
  {}
  ~AccessUnit ()                                            {}

  Int                     getPoc          () const          { return m_iPoc; }
  ExtBinDataAccessorList& getNalUnitList  ()                { return m_cNalUnitList; }
  //NonRequired JVT-Q066 (06-04-08){{
  ErrVal				  CreatNonRequiredSei()				{ RNOK(SEI::NonRequiredSei::create( m_pcNonRequiredSei)) return Err::m_nOK;}
  SEI::NonRequiredSei*	  getNonRequiredSei()				{ return m_pcNonRequiredSei; }
  //NonRequired JVT-Q066 (06-04-08)}}

	//JVT-W052 wxwan
	ErrVal					CreatIntegrityCheckSei()    { RNOK(SEI::IntegrityCheckSEI::create( m_pcIntegrityCheckSei)) return Err::m_nOK; }
	SEI::IntegrityCheckSEI* getIntegrityCheckSei()    { return m_pcIntegrityCheckSei; }
	//JVT-W052 wxwan

private:
  Int                     m_iPoc;
  ExtBinDataAccessorList  m_cNalUnitList;
  SEI::NonRequiredSei*	  m_pcNonRequiredSei; //NonRequired JVT-Q066 (06-04-08)
	SEI::IntegrityCheckSEI* m_pcIntegrityCheckSei;//JVT-W052
};


class H264AVCENCODERLIB_API AccessUnitList
{
public:
  AccessUnitList  ()  {}
  ~AccessUnitList ()  {}

  Void        clear           ()            { m_cAccessUnitList.clear(); }
  AccessUnit& getAccessUnit   ( Int iPoc )
  {
    std::list<AccessUnit>::iterator  iter = m_cAccessUnitList.begin();
    std::list<AccessUnit>::iterator  end  = m_cAccessUnitList.end  ();
    for( ; iter != end; iter++ )
    {
      if( (*iter).getPoc() == iPoc )
      {
        return (*iter);
      }
    }
    AccessUnit cAU( iPoc );
    m_cAccessUnitList.push_back( cAU );
    return m_cAccessUnitList.back();
  }
  Void        emptyNALULists  ( ExtBinDataAccessorList& rcOutputList )
  {
    while( ! m_cAccessUnitList.empty() )
    {
      ExtBinDataAccessorList& rcNaluList = m_cAccessUnitList.front().getNalUnitList();
      rcOutputList += rcNaluList;
      rcNaluList.clear();
      m_cAccessUnitList.pop_front();
    }
  }

private:
  std::list<AccessUnit>  m_cAccessUnitList;
};




class PicOutputData
{
public:
  Bool    FirstPicInAU;
  Int     Poc;
  Char    FrameType[3];
  Int     DependencyId;
  Int     QualityId;
  Int     TemporalId;
  Int     Qp;
  Int     Bits;
  Double  YPSNR;
  Double  UPSNR;
  Double  VPSNR;
// JVT-V068 {
  Int     iPicType;
  UInt    uiBaseQualityLevel;
  UInt    uiBaseLayerId;

  PicOutputData(): uiBaseQualityLevel(MSYS_UINT_MAX), uiBaseLayerId(MSYS_UINT_MAX) {};
// JVT-V068 }
};

typedef MyList<PicOutputData> PicOutputDataList;




class H264AVCENCODERLIB_API MCTFEncoder
{
  enum
  {
    //NUM_TMP_FRAMES  = 6//RPIC bug fix
    NUM_TMP_FRAMES  = 11
  };
  enum RefListUsage
  {
    RLU_UNDEFINED         = 0,
    RLU_MOTION_ESTIMATION = 1,
    RLU_GET_RESIDUAL      = 2,
    RLU_RECONSTRUCTION    = 3
  };

protected:
	MCTFEncoder          ();
	virtual ~MCTFEncoder ();

public:
  static ErrVal create              ( MCTFEncoder*&                   rpcMCTFEncoder );
  ErrVal        destroy             ();
  ErrVal        init                ( CodingParameter*                pcCodingParameter,
                                      LayerParameters*                pcLayerParameters,
                                      H264AVCEncoder*                 pcH264AVCEncoder,
                                      SliceEncoder*                   pcSliceEncoder,
                                      LoopFilter*                     pcLoopFilter,
                                      PocCalculator*                  pcPocCalculator,
                                      NalUnitEncoder*                 pcNalUnitEncoder,
                                      YuvBufferCtrl*                  pcYuvFullPelBufferCtrl,
                                      YuvBufferCtrl*                  pcYuvHalfPelBufferCtrl,
                                      QuarterPelFilter*               pcQuarterPelFilter,
                                      MotionEstimation*               pcMotionEstimation
									  //JVT-U106 Behaviour at slice boundaries{
                                      ,ReconstructionBypass*           pcReconstructionBypass
									  //JVT-U106 Behaviour at slice boundaries}
                                      // JVT-V068 {
                                      ,StatBuf<Scheduler*, MAX_SCALABLE_LAYERS>* apcScheduler
                                      // JVT-V068 }
									  );
  ErrVal        initParameterSets   ( const SequenceParameterSet&     rcSPS,
                                      const PictureParameterSet&      rcPPSLP,
                                      const PictureParameterSet&      rcPPSHP );

ErrVal          initParameterSetsForFGS( const SequenceParameterSet& rcSPS,
                                          const PictureParameterSet&  rcPPSLP,
                                          const PictureParameterSet&  rcPPSHP );

  ErrVal        uninit              ();
 
  ErrVal        addParameterSetBits ( UInt                            uiParameterSetBits );
  Bool          firstGOPCoded       ()                                { return m_bFirstGOPCoded; }
  ErrVal        initGOP             ( AccessUnitList&                 rcAccessUnitList,
                                      PicBufferList&                  rcPicBufferInputList );
  ErrVal        process             ( UInt                            uiAUIndex,
                                      AccessUnitList&                 rcAccessUnitList,
                                      PicBufferList&                  rcPicBufferInputList,
                                      PicBufferList&                  rcPicBufferOutputList,
                                      PicBufferList&                  rcPicBufferUnusedList,
                                      Double                          aaauidSeqBits[MAX_LAYERS][MAX_TEMP_LEVELS][MAX_QUALITY_LEVELS]
                                      // JVT-V068 HRD {
									  ,ParameterSetMng*                pcParameterSetMng
                                      // JVT-V068 HRD {
									                  );
  ErrVal        finish              ( UInt&                           ruiNumCodedFrames,
                                      Double&                         rdOutputRate,
                                      Double                          aaadOutputFramerate[MAX_LAYERS][MAX_TEMP_LEVELS][MAX_QUALITY_LEVELS],
                                      Double                          aaadBits[MAX_LAYERS][MAX_TEMP_LEVELS][MAX_QUALITY_LEVELS] );

// BUG_FIX liuhui{
  ErrVal        SingleLayerFinish(   Double                           aaadBits[MAX_LAYERS][MAX_TEMP_LEVELS][MAX_QUALITY_LEVELS],
                                     Double                           aaadSingleBitrate[MAX_LAYERS][MAX_TEMP_LEVELS][MAX_QUALITY_LEVELS] );
// BUG_FIX liuhui}



  Int           getFrameWidth       ()                                { return 16*m_uiFrameWidthInMb; }
  Int           getFrameHeight      ()                                { return 16*m_uiFrameHeightInMb; }
  ErrVal        getBaseLayerStatus  ( Bool&                           bExists,
																			Bool&                           bMotion,
																			Int                             iPoc,
																			PicType                         ePicType,
																			UInt														uiIdrPicId);//EIDR 0619

	ErrVal       getBaseLayerDataAvailability( IntFrame*&                      pcFrame,
																							IntFrame*&                      pcResidual,
																							MbDataCtrl*&                    pcMbDataCtrl,
																							Bool&                           bConstrainedIPredBL,
																							Bool&                           bForCopyOnly,
																							Int                             iSpatialScalability,
																							Int                             iPoc,
																							Bool                            bMotion,
																							PicType                         ePicType,
																							UInt														uiIdrPicId);//EIDR 0619


		ErrVal        getBaseLayerData    ( IntFrame*&                      pcFrame,
																				IntFrame*&                      pcResidual,
																				MbDataCtrl*&                    pcMbDataCtrl,
																				MbDataCtrl*&                    pcMbDataCtrlEL,			// ICU/ETRI FGS_MOT_USE
																				Bool&                           bConstrainedIPredBL,
																				Bool&                           bForCopyOnly,
																				Int                             iSpatialScalability,
																				Int                             iPoc,
																				Bool                            bMotion,
																				PicType                         ePicType,
																				UInt														uiIdrPicId);//EIDR 0619

		UInt          getNewBits          ()  { UInt ui = m_uiNewlyCodedBits; m_uiNewlyCodedBits = 0; return ui; }
		UInt*         getGOPBits          ()  { return m_auiCurrGOPBits;			}
		UInt          getScalableLayer    ()  const { return m_uiScalableLayerId; }

		IntFrame*     getMGSLPRec         ();
		IntFrame*     getRefPic           ( Int iPoc, 
																				UInt uiIdrPicId	);//EIDR 0619


  //===== ESS =====
  Int                     getSpatialScalabilityType() { return m_pcResizeParameters->m_iSpatialScalabilityType; }
  ResizeParameters*       getResizeParameters()       { return m_pcResizeParameters; }

  Void			setNonRequiredWrite ( UInt ui ) {m_uiNonRequiredWrite = ui;} //NonRequired JVT-Q066 (06-04-08)
  //Bug_Fix JVT-R057{
  Bool              getLARDOEnable( ){ return m_bLARDOEnable; }
  Void              setLARDOEnable(Bool bEnable){ m_bLARDOEnable= bEnable; }
  //Bug_Fix JVT-R057{
  MbDataCtrl*   getBaseMbDataCtrl() {return m_pcBaseMbDataCtrl;}
  Void          setBaseMbDataCtrl(MbDataCtrl* pcMbDataCtrl) {m_pcBaseMbDataCtrl = pcMbDataCtrl;}
//JVT-T054{
  Void          setLayerCGSSNR(UInt ui) { m_uiLayerCGSSNR = ui;}
  Void          setQualityLevelCGSSNR(UInt ui) { m_uiQualityLevelCGSSNR = ui;}
  UInt          getLayerCGSSNR() { return m_uiLayerCGSSNR;}
  UInt          getQualityLevelCGSSNR() { return m_uiQualityLevelCGSSNR;}
  Void          setBaseLayerCGSSNR(UInt ui) { m_uiBaseLayerCGSSNR = ui;}
  Void          setBaseQualityLevelCGSSNR(UInt ui) { m_uiBaseQualityLevelCGSSNR = ui;}
  UInt          getBaseLayerCGSSNR() { return m_uiBaseLayerCGSSNR;}
  UInt          getBaseQualityLevelCGSSNR() { return m_uiBaseQualityLevelCGSSNR;}
//JVT-T054}

  IntFrame*     getBaseLayerResidual() {return m_pcBaseLayerResidual;} // this one is upsampled base layer's residual
  UInt          getIdrPicId         () { return m_uiIdrPicId; } //JVT-W062
 //EIDR bug-fix
  Void		      setIDRAccessPeriod(Int i)		{ m_iIDRAccessPeriod = i;}	
	//JVT-W051 {
	UInt			xGetParameterSetBits()	{ return m_uiParameterSetBits; }
	//JVT-W051 }
	//JVT-X046 {
  ErrVal xEncodeNonKeyPictureSlices(  UInt                        uiBaseLevel,
								  UInt                        uiFrame, 
								  AccessUnitList&             rcAccessUnitList,
								PicBufferList&				  rcPicBufferInputList,
								PicOutputDataList&            rcPicOutputDataList );

  ErrVal  xEncodeHighPassSignalSlices         ( ExtBinDataAccessorList&  rcOutExtBinDataAccessorList,
											RefFrameList* pcRefFrameList0,
											RefFrameList* pcRefFrameList1,
											IntFrame*        pcOrigFrame,
											IntFrame*        pcIntraRecFrame,
											IntFrame*        pcMCFrame,
											IntFrame*        pcResidual,
											IntFrame*        pcPredSignal,
											ControlData&     rcControlData,
											Bool             bBiPredOnly,
											UInt             uiNumMaxIter,
											UInt             uiIterSearchRange,
											UInt             uiFrameIdInGOP,
											PicType          ePicType,
											UInt&            ruiBits,
											UInt&            ruiBitsRes,
											PicOutputDataList&       rcPicOutputDataList,
											IntFrame*		 pcFrame,
											IntFrame*		 pcBLRecFrame,
											UInt         uiBaseLevel);
	//JVT-X046 }
protected:
  //===== data management =====
  ErrVal  xCreateData                   ( const SequenceParameterSet& rcSPS );
  ErrVal  xDeleteData                   ();

  
  ErrVal  xInitBitCounts                ();
  ErrVal  xInitGOP                      ( PicBufferList&              rcPicBufferInputList );
  ErrVal  xFinishGOP                    ( PicBufferList&              rcPicBufferInputList,
                                          PicBufferList&              rcPicBufferOutputList,
                                          PicBufferList&              rcPicBufferUnusedList,
                                          Double                      aaauidSeqBits[MAX_LAYERS][MAX_TEMP_LEVELS][MAX_QUALITY_LEVELS] );

  ErrVal  xInitExtBinDataAccessor       ( ExtBinDataAccessor&         rcExtBinDataAccessor );
  ErrVal  xAppendNewExtBinDataAccessor  ( ExtBinDataAccessorList&     rcExtBinDataAccessorList,
                                          ExtBinDataAccessor*         pcExtBinDataAccessor );
  
  //===== decomposition / composition =====
  ErrVal  xMotionEstimationFrame        ( UInt                        uiBaseLevel,
                                          UInt                        uiFrame
                                          , PicType                   ePicType //TMM

                                          );
  ErrVal  xDecompositionFrame           ( UInt                        uiBaseLevel,
                                          UInt                        uiFrame
                                          , PicType                   ePicType //TMM
                                          );
  ErrVal  xCompositionFrame             ( UInt                        uiBaseLevel,
                                          UInt                        uiFrame,
                                          PicBufferList&              rcPicBufferInputList
                                          , PicType                   ePicType //TMM
                                          );
  ErrVal  xStoreReconstruction          ( PicBufferList&              rcPicBufferOutputList );

//TMM_INTERLACE{
ErrVal xMotionCompensationMbAff(        IntFrame*                   pcMCFrame,
                                          RefFrameList*               pcRefFrameList0,
                                          RefFrameList*               pcRefFrameList1,
                                          MbDataCtrl*                 pcMbDataCtrl,
                                          SliceHeader&                rcSH );

  ErrVal xMotionEstimationMbAff(          RefFrameList*               pcRefFrameList0,
                                          RefFrameList*               pcRefFrameList1,
                                          IntFrame*                   pcOrigFrame,
                                          IntFrame*                   pcIntraRecFrame,
                                          ControlData&                rcControlData,
                                          Bool                        bBiPredOnly,
                                          UInt                        uiNumMaxIter,
                                          UInt                        uiIterSearchRange,
                                          UInt                        uiIntraMode );
//TMM_INTERLACE}


  //===== control data initialization =====
  ErrVal  xSetScalingFactors            ( UInt                        uiBaseLevel );
  ErrVal  xSetScalingFactors            ();
  ErrVal  xGetListSizes                 ( UInt                        uiTemporalLevel,
                                          UInt                        uiFrameIdInGOP,
                                          UInt                        auiPredListSize[2] );
 	ErrVal  xSetBaseLayerData             ( UInt                        uiFrameIdInGOP,
		                                      PicType                     ePicType );
  ErrVal  xInitReordering               ( UInt                        uiFrameIdInGOP,
		                                      PicType                     ePicType
                                          , Bool                      bUncompleteCOP = false // TMM
                                          );   
  
  Void    xPAffDecision                 ( UInt                        uiFrame );              


 ErrVal  xInitSliceHeader              ( UInt                        uiTemporalLevel,
                                          UInt                        uiFrameIdInGOP,
																					PicType                     ePicType );

  ErrVal  xClearBufferExtensions        ();
  ErrVal  xGetPredictionLists           ( RefFrameList&               rcRefList0,
                                          RefFrameList&               rcRefList1,
                                          UInt                        uiBaseLevel,
                                          UInt                        uiFrame,
                                          RefListUsage                eRefListUsage,
                                          Bool                        bHalfPel = false );

  ErrVal  xGetCLRecPredictionLists      ( RefFrameList&               rcRefList0,
                                          RefFrameList&               rcRefList1,
                                          UInt                        uiBaseLevel,
                                          UInt                        uiFrame,
                                          RefListUsage                eRefListUsage,
                                          Bool                        bHalfPel = false );
  ErrVal  xInitBaseLayerData            ( ControlData&                rcControlData, 
                                          UInt                        uiBaseLevel,  //TMM_ESS
                                          UInt                        uiFrame,      //TMM_ESS
                                          Bool                        bMotion,
																					PicType                     ePicType ); 

  ErrVal  xGetPredictionListsFieldKey  (  RefFrameList&               rcRefList0,
		                                      UInt                        uiList0Size,
                                          PicType                     ePicType );

  ErrVal  xGetPredictionListsField     (  RefFrameList&               rcRefList0,
                                          RefFrameList&               rcRefList1,
                                          UInt                        uiBaseLevel,
                                          UInt                        uiFrame,
                                          RefListUsage                eRefListUsage,
																					Bool                        bHalfPel,
                                          PicType                     ePicType );

  ErrVal  xInitControlDataMotion        ( UInt                        uiBaseLevel,
                                          UInt                        uiFrame,
                                          Bool                        bMotionEstimation,
                                          PicType                     ePicType );

  ErrVal  xInitControlDataLowPass       ( UInt                        uiFrameIdInGOP,
                                          UInt                        uiBaseLevel,  //TMM_ESS
                                          UInt                        uiFrame,
                                          PicType                     ePicType );  

  ErrVal  xInitControlDataHighPass      ( UInt                        uiFrameIdInGOP,
                                          UInt                        uiBaseLevel,   //TMM_ESS
                                          UInt                        uiFrame,
                                          PicType                     ePicType );

  ErrVal  xGetConnections               ( Double&                     rdL0Rate,
                                          Double&                     rdL1Rate,
                                          Double&                     rdBiRate );


  //===== stage encoding =====
  ErrVal  xEncodeKeyPicture             ( Bool&                       rbKeyPicCoded,                         
                                          UInt                        uiFrame,
                                          AccessUnitList&             rcAccessUnitList,
                                          PicOutputDataList&          rcPicOutputDataList );
  ErrVal  xEncodeNonKeyPicture          ( UInt                        uiBaseLevel,
                                          UInt                        uiFrame,
                                          AccessUnitList&             rcAccessUnitList,
                                          PicOutputDataList&          rcPicOutputDataList
                                          , PicType                   ePicType  //TMM
                                          );
  ErrVal  xOutputPicData                ( PicOutputDataList&          rcPicOutputDataList );

  //===== basic encoding =====
  ErrVal  xEncodeLowPassSignal          ( ExtBinDataAccessorList&     rcOutExtBinDataAccessorList,
                                          ControlData&                rcResidualControlData,
																					IntFrame*                   pcOrgFrame,
                                          IntFrame*                   pcFrame,
                                          IntFrame*                   pcRecSubband,
                                          IntFrame*                   pcPredSignal,
                                          UInt&                       ruiBits,
                                          PicOutputDataList&          rcPicOutputDataList,
																					PicType                     ePicType );


  ErrVal  xEncodeHighPassSignal         ( ExtBinDataAccessorList&     rcOutExtBinDataAccessorList,
                                          ControlData&                rcControlData,
                                          IntFrame*                   pcOrgFrame, 
                                          IntFrame*                   pcFrame,
                                          IntFrame*                   pcResidual,
                                          IntFrame*                   pcPredSignal,
																					UInt&                       ruiBits,
                                          UInt&                       ruiBitsRes,
                                          PicOutputDataList&          rcPicOutputDataList,
																					PicType                     ePicType );


  //===== motion estimation / compensation =====
  ErrVal  xMotionCompensation           ( IntFrame*                   pcMCFrame,
                                          RefFrameList*               pcRefFrameList0,
                                          RefFrameList*               pcRefFrameList1,
                                          MbDataCtrl*                 pcMbDataCtrl,
                                          SliceHeader&                rcSH);
  
  ErrVal  xMotionEstimation             ( RefFrameList*               pcRefFrameList0,
                                          RefFrameList*               pcRefFrameList1,
                                          IntFrame*                   pcOrigFrame,
                                          IntFrame*                   pcIntraRec,
                                          ControlData&                rcControlData,
                                          Bool                        bBiPredOnly,
                                          UInt                        uiNumMaxIter,
                                          // JVT-S054 (REPLACE) ->
                                          //UInt                        uiIterSearchRange );
                                          UInt                        uiIterSearchRange,
                                          UInt                        uiFrameIdInGOP ,
																					PicType                     ePicType  );
                                          // JVT-S054 (REPLACE) <-
  	//--

  //===== auxiliary functions =====
  ErrVal  xCalculateAndAddPSNR          ( UInt                        uiStage,
                                          UInt                        uiFrame,
                                          PicBufferList&              rcPicBufferInputList,
                                          PicOutputDataList&          rcPicOutputDataList );

  ErrVal  xFillAndUpsampleFrame         ( IntFrame*                   pcFrame,
		                                      PicType                     ePicType,
																					Bool                        bFrameMbsOnlyFlag );

  ErrVal  xFillAndExtendFrame           ( IntFrame*                   pcFrame,
		                                      PicType                     ePicType,
																					Bool                        bFrameMbsOnlyFlag );

  ErrVal  xZeroIntraMacroblocks         ( IntFrame*                   pcFrame,
                                          ControlData&                pcCtrlData,
																					PicType                     ePicType );

  ErrVal  xClipIntraMacroblocks         ( IntFrame*                   pcFrame,
                                          ControlData&                rcCtrlData,
                                          Bool                        bClipAll,
																					PicType                     ePicType );

  ErrVal  xAddBaseLayerResidual         ( ControlData&                rcControlData,
                                          IntFrame*                   pcFrame,
                                          Bool                        bSubtract,
																					PicType                     ePicType );
  ErrVal  xUpdateLowPassRec             ();
  IntFrame* xGetRefFrame                ( IntFrame**                  papcRefFrameList,
                                          UInt                        uiRefIndex,
                                          RefListUsage                eRefListUsage );
  ErrVal  xClearELPics                  ();
  ErrVal  xUpdateELPics                 ();


  //===== slice header =====
  ErrVal        xSetRplr            ( RplrBuffer& rcRplrBuffer, UIntList cPicNumList, UInt uiCurrPicNr, PicType ePicType );

  ErrVal        xSetRplrAndMmco     ( SliceHeader& rcSH );

  ErrVal        xSetRplrAndMmcoFld  ( SliceHeader& rcSH );

  ErrVal        xSetMmcoFld         ( SliceHeader& rcSH ); //TMM

  ErrVal        xWriteSEI           ( ExtBinDataAccessorList& rcOutExtBinDataAccessorList, SliceHeader& rcSH, UInt& ruiBit );
  ErrVal		xWriteSuffixUnit    ( ExtBinDataAccessorList& rcOutExtBinDataAccessorList, SliceHeader& rcSH, UInt& ruiBit );//JVT-S036 lsj 
   ErrVal		xWritePrefixUnit    ( ExtBinDataAccessorList& rcOutExtBinDataAccessorList, SliceHeader& rcSH, UInt& ruiBit );//prefix unit
	ErrVal		xSetMmcoBase		( SliceHeader& rcSH, UInt iNum ); //JVT-S036 lsj
  //NonRequired JVT-Q066 (06-04-08){{
  ErrVal		xWriteNonRequiredSEI( ExtBinDataAccessorList& rcOutExtBinDataAccessorList, SEI::NonRequiredSei* pcNonRequiredSei, UInt& ruiBit ); 
  ErrVal		xSetNonRequiredSEI  ( SliceHeader* pcSliceHeader, SEI::NonRequiredSei* pcNonRequiredSei);
  //ErrVal		xWriteNonRequiredSEI( ExtBinDataAccessorList& rcOutExtBinDataAccessorList, UInt& ruiBit ); 
  //NonRequired JVT-Q066 (06-04-08)}}

  // JVT-V068 HRD {
  ErrVal xWriteSEI(ExtBinDataAccessorList &rcOutExtBinDataAccessorList, SEI::MessageList& rcSEIMessageList, UInt &ruiBits);
  ErrVal xWriteNestingSEIforHrd(ExtBinDataAccessorList &rcOutExtBinDataAccessorList, SEI::SEIMessage *pcSEIMessage, UInt uiuiDependencyId, UInt uiQualityLevel, UInt uiTemporalLevel, UInt &ruiBits);
  ErrVal xWriteSEIforAVCCompatibleHrd(ExtBinDataAccessorList &rcOutExtBinDataAccessorList, SEI::SEIMessage* pcSEIMessage, UInt &ruiBits);
  ErrVal xCalculateTiming( PicOutputDataList&  rcPicOutputDataList, UInt uiFrame );
  // JVT-V068 HRD }
	ErrVal xWriteRedundantKeyPicSEI  ( ExtBinDataAccessorList&  rcOutExtBinDataAccessorList, UInt &ruiBits ); //JVT-W049

	//JVT-W052 wxwan
	ErrVal xWriteIntegrityCheckSEI(ExtBinDataAccessorList &rcOutExtBinDataAccessorList, SEI::SEIMessage *pcSEIMessage, UInt &ruiBits);
	//JVT-W052 wxwan

  ErrVal        xGetFrameNumList    ( SliceHeader& rcSH, UIntList& rcFrameNumList, ListIdx eLstIdx, UInt uiCurrBasePos );

  ErrVal        xGetFieldNumList    ( SliceHeader& rcSH, UIntList& rcFrameNumList, ListIdx eLstIdx, UInt uiCurrBasePos );
 
  MbDataCtrl*   xGetMbDataCtrlL1    ( SliceHeader& rcSH, UInt uiCurrBasePos );
  Void          xAssignSimplePriorityId ( SliceHeader *pcSliceHeader );
  
   //===== ESS =====
   ErrVal		xFillPredictionLists_ESS( UInt uiBaseLevel , UInt uiFrame );

  UInt				getPreAndSuffixUnitEnable()	{return m_uiPreAndSuffixUnitEnable;} //JVT-S036 lsj 
	UInt							  getMMCOBaseEnable		  ()			  const	  { return m_uiMMCOBaseEnable; } //JVT-S036 lsj

  //S051{
  Bool	xSIPCheck	(UInt POC);
  int	xGetMbDataCtrlL1Pos( const SliceHeader& rcSH, UInt uiCurrBasePos );
  //S051}
  Void setMCResizeParameters   (ResizeParameters*				resizeParameters);

  //JVT-U106 Behaviour at slice boundaries{
  ErrVal xConstrainedIntraUpsampling(IntFrame*pcFrame,
									 IntFrame*pcUpsampling, 
									 IntFrame*pcTemp,
									 MbDataCtrl* pcBaseDataCtrl,
									 ReconstructionBypass* pcReconstructionBypass,
									 ResizeParameters* pcResizeParameters
                   , PicType ePicType);
  void  xGetPosition(ResizeParameters* pcResizeParameters,Int*px,Int*py,bool uv_flag);
  //JVT-U106 Behaviour at slice boundaries{
protected:
  //----- instances -----
  ExtBinDataAccessor            m_cExtBinDataAccessor;
  BinData                       m_cBinData;
  DownConvert                   m_cDownConvert;

  //----- references -----
  const SequenceParameterSet*   m_pcSPS;
  const PictureParameterSet*    m_pcPPSLP;
  const PictureParameterSet*    m_pcPPSHP;

  const SequenceParameterSet*   m_pcSPS_FGS;
  const PictureParameterSet*    m_pcPPSLP_FGS;
  const PictureParameterSet*    m_pcPPSHP_FGS;

  YuvBufferCtrl*                m_pcYuvFullPelBufferCtrl;
  YuvBufferCtrl*                m_pcYuvHalfPelBufferCtrl;
  PocCalculator*                m_pcPocCalculator;
  H264AVCEncoder*               m_pcH264AVCEncoder;
  SliceEncoder*                 m_pcSliceEncoder;
  NalUnitEncoder*               m_pcNalUnitEncoder;
  LoopFilter*                   m_pcLoopFilter;
  QuarterPelFilter*             m_pcQuarterPelFilter;
  MotionEstimation*             m_pcMotionEstimation;
  LayerParameters*              m_pcLayerParameters;

  //----- fixed control parameters ----
  Bool                          m_bTraceEnable;                       // trace file
  Bool                          m_bFrameMbsOnlyFlag;                  // frame macroblocks only block
  UInt                          m_uiLayerId;                          // layer id for current layer
  UInt                          m_uiScalableLayerId;                  // scalable layer id for current layer
  UInt                          m_uiBaseLayerId;                      // layer if of base layer
  UInt                          m_uiBaseQualityLevel;                 // quality level of the base layer
  UInt                          m_uiQualityLevelForPrediction;        // quality level for prediction
  UInt                          m_uiFrameWidthInMb;                   // frame width in macroblocks
  UInt                          m_uiFrameHeightInMb;                  // frame height in macroblocks
  UInt                          m_uiMbNumber;                         // number of macroblocks in a frame
  UInt                          m_uiMaxGOPSize;                       // maximum possible GOP size (specified by the level)
  UInt                          m_uiDecompositionStages;              // number of decomposition stages
  UInt                          m_uiTemporalResolution;               // temporal subsampling in comparison to highest layer
  UInt                          m_uiNotCodedMCTFStages;               // number of MCTF stages that are only used for temporal downsampling
  UInt                          m_uiFrameDelay;                       // maximum possible delay in frames
  UInt                          m_uiMaxNumRefFrames;                  // maximum number of active reference pictures in a list
  UInt                          m_uiLowPassIntraPeriod;               // intra period for lowest temporal resolution
  UInt                          m_uiNumMaxIter;                       // maximum number of iteration for bi-directional search
  UInt                          m_uiIterSearchRange;                  // search range for iterative search
  UInt                          m_iMaxDeltaQp;                        // maximum QP changing
  Bool                          m_bH264AVCCompatible;                 // H.264/AVC compatibility
  Bool                          m_bInterLayerPrediction;              // inter-layer prediction
  Bool                          m_bAdaptivePrediction;                // adaptive inter-layer prediction
  Bool                          m_bHaarFiltering;                     // haar-based decomposition
  Bool                          m_bBiPredOnly;                        // only bi-direktional prediction
  Bool                          m_bForceReOrderingCommands;           // always write re-ordering commands (error robustness)
  Bool                          m_bWriteSubSequenceSei;               // Subsequence SEI message (H.264/AVC base layer)
  Double                        m_adBaseQpLambdaMotion[MAX_DSTAGES];  // base QP's for mode decision and motion estimation
  Double                        m_dBaseQpLambdaMotionLP;
  Double                        m_dBaseQPResidual;                    // base residual QP

  UInt                          m_uiFilterIdc;                        // de-blocking filter idc
  Int                           m_iAlphaOffset;                       // alpha offset for de-blocking filter
  Int                           m_iBetaOffset;                        // beta offset for de-blocking filter

  Bool                          m_bLoadMotionInfo;                    // load motion data from file
  Bool                          m_bSaveMotionInfo;                    // save motion data to file
  FILE*                         m_pMotionInfoFile;                    // motion data file

  //----- variable control parameters -----
  Bool                          m_bInitDone;                          // initilisation
  Bool                          m_bFirstGOPCoded;                     // true if first GOP of a sequence has been coded
  UInt                          m_uiGOPSize;                          // current GOP size
  UInt                          m_uiFrameCounter;                     // current frame counter
  UInt                          m_uiFrameNum;                         // current value of syntax element frame_num
  UInt                          m_uiGOPNumber;                        // number of coded GOP's
  Bool                          m_abIsRef[MAX_DSTAGES];               // state of temporal layer (H.264/AVC base layer)
  UIntList                      m_cLPFrameNumList;                    // list of frame_num for low-pass frames
	UInt													m_uiIdrPicId;	//EIDR 0619 

  //----- frame memories -----
  IntFrame*                     m_apcFrameTemp[NUM_TMP_FRAMES];       // auxiliary frame memories
  IntFrame**                    m_papcFrame;                          // frame stores
  IntFrame**                    m_papcOrgFrame;                       // original (highpass) frames
  IntFrame**                    m_papcCLRecFrame;                     // closed-loop rec. (needed when m_uiQualityLevelForPrediction < NumFGS)
  IntFrame**                    m_papcELFrame;                        // higher layer reference frames
  IntFrame**                    m_papcResidual;                       // frame stores for residual data
  IntFrame**                    m_papcSubband;                        // reconstructed subband pictures
  IntFrame*                     m_pcLowPassBaseReconstruction;        // base reconstruction of last low-pass picture
//TMM_WP
  Bool                          m_bBaseLayerWp;
//TMM_WP
  IntFrame*                     m_pcAnchorFrameOriginal;              // original anchor frame
  IntFrame*                     m_pcAnchorFrameReconstructed;         // reconstructed anchor frame
  IntFrame*                     m_pcBaseLayerFrame;                   // base layer frame
  IntFrame*                     m_pcBaseLayerResidual;                // base layer residual
	  
  //----- control data arrays -----
  ControlData*                  m_pacControlData;                     // control data arrays
  MbDataCtrl*                   m_pcBaseLayerCtrl;                    // macroblock data of the base layer pictures
  MbDataCtrl*                   m_pcBaseLayerCtrlField; 
	ControlData*                  m_pacControlDataEL;                     // control data arrays
	MbDataCtrl*                   m_pcBaseLayerCtrlEL;                    // macroblock data of the base layer pictures
  MbDataCtrl*                   m_pcRedundantCtrl;//RPIC bug fix
  MbDataCtrl*                   m_pcRedundant1Ctrl;//RPIC bug fix

  //----- auxiliary buffers -----
  UInt                          m_uiWriteBufferSize;                  // size of temporary write buffer
  UChar*                        m_pucWriteBuffer;                     // write buffer

  //----- PSNR & rate  -----
  Double                        m_fOutputFrameRate;
  UInt                          m_uiParameterSetBits;
  UInt                          m_auiNumFramesCoded [MAX_DSTAGES+1];
  UInt                          m_uiNewlyCodedBits;
  Double                        m_adPSNRSumY        [MAX_DSTAGES+1];
  Double                        m_adPSNRSumU        [MAX_DSTAGES+1];
  Double                        m_adPSNRSumV        [MAX_DSTAGES+1];
  UInt m_auiCurrGOPBits     [ MAX_SCALABLE_LAYERS ];
  Double m_adSeqBits        [ MAX_SCALABLE_LAYERS ];

  //----- ESS -----
  ResizeParameters*				m_pcResizeParameters; 

  Bool*                         m_pbFieldPicFlag;

// JVT-Q065 EIDR{
  Int           						  	m_iIDRPeriod;
  Int				                    m_iIDRAccessPeriod;  //EIDR bug-fix
  Bool					            		m_bBLSkipEnable;
// JVT-Q065 EIDR}
  //JVT-R057 LA-RDO{
  Bool                          m_bLARDOEnable;     
  //JVT-R057 LA-RD}
  UInt                          m_uiEssRPChkEnable;
  UInt                          m_uiMVThres;

  UInt							            m_uiNonRequiredWrite; //NonRequired JVT-Q066 (06-04-08)

  UInt							            m_uiPreAndSuffixUnitEnable; //JVT-S036 lsj 
  UInt							            m_uiMMCOBaseEnable;  //JVT-S036 lsj

  //S051{
  UInt							            m_uiTotalFrame;
  UInt*							            m_auiFrameBits;
  UIntList					            m_cPocList;
  UInt						            	m_uiAnaSIP;
  Bool						            	m_bEncSIP;
  std::string				            m_cInSIPFileName;
  std::string				            m_cOutSIPFileName;
  //S051}

  // JVT-S054 (ADD) ->
  Bool                          m_bIroiSliceDivisionFlag;
  UInt                          m_uiNumSliceMinus1;
  UInt*                         m_puiFirstMbInSlice;
  UInt*                         m_puiLastMbInSlice;
  // JVT-S054 (ADD) <-
//JVT-T054{
  UInt                          m_uiLayerCGSSNR;
  UInt                          m_uiQualityLevelCGSSNR;
  UInt                          m_uiBaseLayerCGSSNR;
  UInt                          m_uiBaseQualityLevelCGSSNR;
//JVT-T054}
//DS_FIX_FT_09_2007
  Bool                          m_bDiscardable;
  UInt                          m_uiQLDiscardable;
//~DS_FIX_FT_09_2007
// JVT-U085 LMI 
  Bool                          m_bTlevelNestingFlag;
// JVT-U116 W062 LMI 
  Bool                          m_bTl0DepRepIdxEnable;
  //JVT-U106 Behaviour at slice boundaries{
  Bool                          m_bCIUFlag;
  ReconstructionBypass*         m_pcReconstructionBypass;
  Bool*                         m_pbIntraBLFlag; 
  //JVT-U106 Behaviour at slice boundaries}
  UInt                          m_uiMinScalableLayer;
  UInt                          m_uiFramesInCompleteGOPsProcessed;
  Bool                          m_bGOPInitialized;
  MbDataCtrl*                   m_pcBaseMbDataCtrl;

  UInt    m_uiEncodeKeyPictures;
  Bool    m_bSameResBL;
  Bool    m_bSameResEL;
  Bool    m_bMGS;
  UInt    m_uiMGSKeyPictureControl;
  Bool    m_bHighestMGSLayer;
  Bool    m_abCoded[(1<<MAX_DSTAGES)+1];
  UInt    m_uiMGSKeyPictureMotRef;

  Bool    m_bExplicitQPCascading;
  Double  m_adDeltaQPTLevel[MAX_TEMP_LEVELS];

// JVT-V068 HRD {
  StatBuf<Scheduler*, MAX_SCALABLE_LAYERS>* m_apcScheduler;
  ParameterSetMng* m_pcParameterSetMng;
  Bool    m_bEnableHrd;
// JVT-V068 HRD }
	
	// JVT-W049 {
  UInt m_uiNumberLayersCnt;
  // JVT-W049 }
	
	//JVT-W051 {
public:
	Double	m_dFrameBits;
	Double	m_dAvgBitrate;
	UInt		m_uiProfileIdc;
	UInt		m_uiLevelIdc;
	Bool		m_bConstraint0Flag;
	Bool		m_bConstraint1Flag;
	Bool		m_bConstraint2Flag;
	Bool		m_bConstraint3Flag;
	//JVT-W051 }
public:
	Bool    m_bOutputFlag;//JVT-W047
	//JVT-X046 {
  UInt    m_uiSliceMode;
  UInt    m_uiSliceArgument;
  //JVT-X046 }
  
};

#if defined( WIN32 )
# pragma warning( default: 4251 )
#endif

H264AVC_NAMESPACE_END

#endif // !defined(AFX_GOPENCODER_H__75F41F36_C28D_41F9_AB5E_4C90D66D160C__INCLUDED_)
