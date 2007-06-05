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




#if !defined(AFX_H264AVCENCODER_H__FBF0345F_A5E5_4D18_8BEC_4A68790901F7__INCLUDED_)
#define AFX_H264AVCENCODER_H__FBF0345F_A5E5_4D18_8BEC_4A68790901F7__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



#include "H264AVCEncoderLib.h"
#include "H264AVCCommonLib/TraceFile.h"
#include "GOPEncoder.h"

#ifdef SHARP_AVC_REWRITE_OUTPUT
#include "MbCoder.h"
#include "CreaterH264AVCEncoder.h"
#endif

H264AVC_NAMESPACE_BEGIN



class PocCalculator;
class CodingParameter;
class NalUnitEncoder;
class ControlMngIf;
class ParameterSetMng;
class MCTFEncoder;
class FrameMng;


#if defined( WIN32 )
# pragma warning( disable: 4251 )
#endif




class H264AVCENCODERLIB_API H264AVCEncoder
{
protected:
	H264AVCEncoder();
	virtual ~H264AVCEncoder();

public:
  static  ErrVal create ( H264AVCEncoder*&  rpcH264AVCEncoder );
  virtual ErrVal destroy();
  virtual ErrVal init   ( MCTFEncoder*      apcMCTFEncoder[MAX_LAYERS],
                          ParameterSetMng*  pcParameterSetMng,
                          PocCalculator*    pcPocCalculator,
                          NalUnitEncoder*   pcNalUnitEncoder,
                          ControlMngIf*     pcControlMng,
                          CodingParameter*  pcCodingParameter,
                          FrameMng*         pcFrameMng 
                          // JVT-V068 {
                          ,StatBuf<Scheduler*, MAX_SCALABLE_LAYERS>* apcScheduler 
                          // JVT-V068 }
                        );
  virtual ErrVal uninit ();

  ErrVal writeParameterSets ( ExtBinDataAccessor*       pcExtBinDataAccessor,
                              // JVT-V068 {
                              SequenceParameterSet *pcAVCSPS, 
                              // JVT-V068 }
                              Bool&                     rbMoreSets );
  ErrVal process            ( ExtBinDataAccessorList&   rcExtBinDataAccessorList, 
                              PicBuffer*                apcOriginalPicBuffer   [MAX_LAYERS],
                              PicBuffer*                apcReconstructPicBuffer[MAX_LAYERS],
                              PicBufferList*            apcPicBufferOutputList,
                              PicBufferList*            apcPicBufferUnusedList );
  ErrVal finish             ( ExtBinDataAccessorList&   rcExtBinDataAccessorList, 
                              PicBufferList*            apcPicBufferOutputList,
                              PicBufferList*            apcPicBufferUnusedList,
                              UInt&                     ruiNumCodedFrames,
                              Double&                   rdHighestLayerOutputRate );


  ErrVal  getBaseLayerStatus  ( UInt&         ruiBaseLayerId,
                                UInt&         ruiBaseLayerIdMotionOnly,
                                Int&          riSpatialScalabilityType,
                                UInt          uiLayerId,
                                Int           iPoc,
																PicType       ePicType );

  ErrVal  getBaseLayerDataAvailability  ( IntFrame*&    pcFrame,
                                          IntFrame*&    pcResidual,
                                          MbDataCtrl*&  pcMbDataCtrl,
                                          Bool&         bConstrainedIPredBL,
                                          Bool&         bForCopyOnly,
                                          Int                             iSpatialScalability,
                                          UInt          uiBaseLayerId,
                                          Int           iPoc,
                                          Bool          bMotion,
                                          Bool&         bBaseDataAvailable,
                                          PicType       ePicType);
                                          
  ErrVal  getBaseLayerData    ( IntFrame*&    pcFrame,
                                IntFrame*&    pcResidual,
                                MbDataCtrl*&  pcMbDataCtrl,
																MbDataCtrl*&  pcMbDataCtrlEL,
                                Bool&         bConstrainedIPredBL,
                                Bool&         bForCopyOnly,
                                Int                             iSpatialScalability,
                                UInt          uiBaseLayerId,
                                Int           iPoc,
                                Bool					bMotion,
																PicType       ePicType );

  ErrVal getBaseLayerResidual( IntFrame*&      pcResidual, UInt            uiBaseLayerId);

  ErrVal  getBaseLayerSH      ( SliceHeader*& rpcSliceHeader,
                                UInt          uiBaseLayerId,
                                Int           iPoc,
                                PicType      ePicType );

  UInt    getNewBits          ( UInt          uiBaseLayerId );


  IntFrame* getLowPassRec     ( UInt uiLayerId );
  IntFrame* getELRefPic       ( UInt uiLayerId, Int iPoc );

  //{{Quality level estimation and modified truncation- JVTO044 and m12007
  //France Telecom R&D-(nathalie.cammas@francetelecom.com)
  ErrVal writeQualityLevelInfosSEI( ExtBinDataAccessor* pcExtBinDataAccessor, 
                                    UInt*               uiaQualityLevel, 
                                    UInt *              uiaDelta, 
                                    UInt                uiNumLevels, 
                                    UInt                uiLayer ) ;
    //}}Quality level estimation and modified truncation- JVTO044 and m12007
// JVT-T073 {
  ErrVal writeNestingSEIMessage( ExtBinDataAccessor* pcExtBinDataAccessor );
// JVT-T073 }

  Void setScalableSEIMessage  ()       { m_bScalableSeiMessage = true; }
// JVT-V068 HRD {
  Void setBufferPeriodSEIMessage()     { m_bWriteBufferingPeriodSEI = true; }
  ErrVal writeAVCCompatibleHRDSEI( ExtBinDataAccessor* pcExtBinDataAccessor, SequenceParameterSet& rcSPS );
// JVT-V068 HRD }
	Bool bGetScalableSeiMessage	() const { return m_bScalableSeiMessage; }
	Void SetVeryFirstCall				()			 { m_bVeryFirstCall = true; }
	Double* dGetFramerate				()			 { return m_dFinalFramerate; }
	Double* dGetBitrate					()			 { return m_dFinalBitrate; }
	Double m_aaauidSeqBits [MAX_LAYERS][MAX_TEMP_LEVELS][MAX_QUALITY_LEVELS];
// BUG_FIX liuhui{
	UInt   getScalableLayerId( UInt uiLayer, UInt uiTempLevel, UInt uiFGS ) const { return m_aaauiScalableLayerId[uiLayer][uiTempLevel][uiFGS]; }
	Double m_aaadSingleLayerBitrate[MAX_LAYERS][MAX_TEMP_LEVELS][MAX_QUALITY_LEVELS];
	UInt   m_aaauiScalableLayerId[MAX_LAYERS][MAX_TEMP_LEVELS][MAX_QUALITY_LEVELS];
// BUG_FIX liuhui}
// JVT-S080 LMI {
  ErrVal xWriteScalableSEILayersNotPresent( ExtBinDataAccessor* pcExtBinDataAccessor, UInt uiInputLayers, UInt* m_layer_id);
  ErrVal xWriteScalableSEIDependencyChange( ExtBinDataAccessor* pcExtBinDataAccessor, UInt uiNumLayers, UInt* uiLayerId, Bool* pbLayerDependencyInfoPresentFlag, 
												  UInt* uiNumDirectDependentLayers, UInt** puiDirectDependentLayerIdDeltaMinus1, UInt* puiLayerDependencyInfoSrcLayerIdDeltaMinus1);
// JVT-S080 LMI }

#ifdef SHARP_AVC_REWRITE_OUTPUT
  ErrVal  xCreateAvcRewriteEncoder();
  ErrVal  xInitAvcRewriteEncoder();
  ErrVal  xInitSliceForAvcRewriteCoding(const SliceHeader& rcSH);
  ErrVal  xAvcRewriteParameterSets ( ExtBinDataAccessor* pcExtBinDataAccessor,const SliceHeader& rcSH, const NalUnitType eNalUnitType);  
  ErrVal  xAvcRewriteParameterSets ( ExtBinDataAccessor* pcExtBinDataAccessor,      SequenceParameterSet& rcSps);
  ErrVal  xAvcRewriteParameterSets ( ExtBinDataAccessor* pcExtBinDataAccessor,      PictureParameterSet& rcPps);

  ErrVal  xAvcRewriteSliceHeader ( ExtBinDataAccessor* pcExtBinDataAccessor,SliceHeader& rcSH);
  ErrVal  xCloseAvcRewriteEncoder();

  ErrVal  xAvcRewriteInitNalUnit ( ExtBinDataAccessor* pcExtBinDataAccessor);
  ErrVal  xAvcRewriteCloseNalUnit ();
  MbCoder* xGetPcMbCoder();

  ErrVal initMb( MbDataAccess*& pcMbDataAccessRewrite, UInt uiMbY, UInt uiMbX );
  Void xStoreEstimation( MbDataAccess& rcMbDataAccess, MbDataAccess&  rcMbBestData );
  ErrVal initSlice( SliceHeader& rcSH, ProcessingState eProcessingState, Bool bDecoder, MbDataCtrl* pcMbDataCtrl );
#endif

protected:
  ErrVal xInitParameterSets ();
// JVT-V068 HRD {
	ErrVal xInitLayerInfoForHrd(SequenceParameterSet* pcSPS, UInt uiLayer );
// JVT-V068 HRD }
  ErrVal xWriteScalableSEI  ( ExtBinDataAccessor*       pcExtBinDataAccessor );
	ErrVal xWriteSubPicSEI		( ExtBinDataAccessor*				pcExtBinDataAccessor );
	ErrVal xWriteSubPicSEI( ExtBinDataAccessor* pcExtBinDataAccessor, UInt layer_id ) ;
	ErrVal xWriteMotionSEI( ExtBinDataAccessor* pcExtBinDataAccessor, UInt sg_id ) ;
// JVT-V068 HRD {
  ErrVal xWriteBufferingPeriodSEI ( ExtBinDataAccessor*  pcExtBinDataAccessor );
// JVT-V068 HRD }

  ErrVal xProcessGOP        ( PicBufferList*            apcPicBufferOutputList, 
                              PicBufferList*            apcPicBufferUnusedList );

#ifdef SHARP_AVC_REWRITE_OUTPUT
  MbCoder*                      m_pcMbCoder;
  BitWriteBuffer*				m_pcBitWriteBuffer;
  BitCounter*                   m_pcBitCounter;
  UvlcWriter*                   m_pcUvlcTester;
  UvlcWriter*                   m_pcUvlcWriter;
  CabacWriter*                  m_pcCabacWriter;
  RateDistortion*               m_pcRateDistortion;
  MbDataCtrl*                   m_pcMbDataCtrl;
#endif

protected:
  std::list<SequenceParameterSet*>  m_cUnWrittenSPS;
  std::list<PictureParameterSet*>   m_cUnWrittenPPS;
  PicBufferList                     m_acOrgPicBufferList[MAX_LAYERS];
  PicBufferList                     m_acRecPicBufferList[MAX_LAYERS];
  ParameterSetMng*                  m_pcParameterSetMng;
  PocCalculator*                    m_pcPocCalculator;
  NalUnitEncoder*                   m_pcNalUnitEncoder;
  ControlMngIf*                     m_pcControlMng;
  CodingParameter*                  m_pcCodingParameter;
  FrameMng*                         m_pcFrameMng;
  Bool                              m_bVeryFirstCall;
  Bool                              m_bInitDone;
  Bool                              m_bTraceEnable;

	Bool															m_bScalableSeiMessage;
  Double														m_dFinalBitrate[MAX_LAYERS * MAX_DSTAGES * MAX_QUALITY_LEVELS];
	Double														m_dFinalFramerate[MAX_LAYERS * MAX_DSTAGES * MAX_QUALITY_LEVELS];
  MCTFEncoder*                      m_apcMCTFEncoder    [MAX_LAYERS];
  AccessUnitList                    m_cAccessUnitList;

  // ICU / ETRI ROI 
  Bool    m_bWrteROISEI;
// JVT-V068 HRD {
  Bool    m_bWriteBufferingPeriodSEI;
  StatBuf<Scheduler*, MAX_SCALABLE_LAYERS>* m_apcScheduler;
// JVT-V068 HRD }
  UInt    m_loop_roi_sei;
};


#if defined( WIN32 )
# pragma warning( default: 4251 )
#endif


H264AVC_NAMESPACE_END


#endif // !defined(AFX_H264AVCENCODER_H__FBF0345F_A5E5_4D18_8BEC_4A68790901F7__INCLUDED_)
