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



#if !defined(AFX_GOPDECODER_H__339878FC_BA98_4ABE_8530_E1676196576F__INCLUDED_)
#define AFX_GOPDECODER_H__339878FC_BA98_4ABE_8530_E1676196576F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "H264AVCCommonLib/MbDataCtrl.h"
#include "DownConvert.h"
#ifdef SHARP_AVC_REWRITE_OUTPUT
#include "../H264AVCEncoderLib/H264AVCEncoder.h"
#endif

// TMM_ESS 
#include "ResizeParameters.h"


H264AVC_NAMESPACE_BEGIN

class H264AVCDecoder;
class SliceReader;
class SliceDecoder;
class PocCalculator;
class LoopFilter;
class HeaderSymbolReadIf;
class ParameterSetMng;
class NalUnitParser;
class ControlMngIf;
class MotionCompensation;
class Frame; 
class ReconstructionBypass;
class YuvBufferCtrl; 
class SliceDataNALUnit;



class H264AVCDECODERLIB_API DPBUnit
{
protected:
  DPBUnit           ();
  virtual ~DPBUnit  ();



public:
  static ErrVal create        ( DPBUnit*&                   rpcDPBUnit,
                                YuvBufferCtrl&              rcYuvBufferCtrl,
                                const SequenceParameterSet& rcSPS,
                                UInt                        uiDependencyId );
  ErrVal        destroy       ();

  ErrVal        init          ( const SliceHeader&          rcSH );
  ErrVal        initNonEx     ( Int                         iPoc,
                                UInt                        uiFrameNum );
  ErrVal        initBase      ( DPBUnit&                    rcDPBUnit,
                                Frame*                      pcFrameBaseRep );
  ErrVal        uninit        ();

  ErrVal        markNonRef      ();
  ErrVal        markOutputted   ();
  Void          setPicType      ( PicType     picType       )   { m_ePicType = picType; }
  Void          setNalRefIdc    ( NalRefIdc   nalRefIdc     )   { m_eNalRefIdc = nalRefIdc; }
  Void          setMbDataCtrl   ( MbDataCtrl* pcMbDataCtrl  )   { m_cControlData.setMbDataCtrl( pcMbDataCtrl ); }

  UInt          getFrameNum     ()  const { return m_uiFrameNum; }
  UInt          getTLevel       ()  const { return m_uiTemporalId; }
  Bool          useBasePred     ()  const { return m_bUseBasePred; }
  Bool          isExisting      ()  const { return m_bExisting; }
  Bool          isNeededForRef  ()  const { return m_bNeededForReference; }
  Bool          isOutputted     ()  const { return m_bOutputted; }
  Bool          isBaseRep       ()  const { return m_bBaseRepresentation; }
  Bool          isConstrIPred   ()  const { return m_bConstrainedIntraPred; }
  Bool          isNalRefIdc     ()  const { return m_eNalRefIdc != NAL_REF_IDC_PRIORITY_LOWEST; }
  PicType       getPicType      ()  const { return m_ePicType; }
  Frame*        getFrame        ()        { return m_pcFrame; }
  ControlData&  getCtrlData     ()        { return m_cControlData; }
  MbDataCtrl*   getMbDataCtrlBL ()        { return m_pcMbDataCtrlBL; }
  UInt          getQualityId    ()  const { return m_uiQualityId;}

  Int getPicNum ( UInt uiCurrFrameNum, UInt uiMaxFrameNum ) const
  {
    if( m_uiFrameNum > uiCurrFrameNum )
    {
      return (Int)m_uiFrameNum - (Int)uiMaxFrameNum;
    }
    return (Int)m_uiFrameNum;
  }

  ErrVal storeMbDataCtrlBL( MbDataCtrl* pcMbDataCtrl )
  {
    ROF( pcMbDataCtrl );
    RNOK( m_pcMbDataCtrlBL->copyMotion( *pcMbDataCtrl ) );
    return Err::m_nOK;
  }
  ErrVal switchMbDataCtrlBL( DPBUnit* pcDPBUnit )
  {
    ROF( pcDPBUnit );
    MbDataCtrl* pTmp = m_pcMbDataCtrlBL;
    m_pcMbDataCtrlBL = pcDPBUnit->m_pcMbDataCtrlBL;
    pcDPBUnit->m_pcMbDataCtrlBL = pTmp;
    return Err::m_nOK;
  }

private:
  Int         m_iPoc;
  UInt        m_uiFrameNum;
  UInt        m_uiTemporalId;
  Bool        m_bUseBasePred;
  Bool        m_bExisting;
  Bool        m_bNeededForReference;
  Bool        m_bOutputted;
  Bool        m_bBaseRepresentation;
  Frame*      m_pcFrame;
  ControlData m_cControlData;
  MbDataCtrl* m_pcMbDataCtrlBL;
  Bool        m_bConstrainedIntraPred;
  UInt        m_uiQualityId;
  NalRefIdc   m_eNalRefIdc;
  PicType     m_ePicType;
};

typedef MyList<DPBUnit*>  DPBUnitList;



/* HS:  This is a very first implementation of the DPB. It shall
        be extended in future in way that it performs exactly the
        same operations as the DPB in standard AVC (including 
        MMCO and RPLR commands). Currently, it simulates the 
        operations of the previous JSVM software version. 
        Furthermore, it is probably necessary that a single DPB is
        used for all "layers" (same dependency_id). With the
        current implementation, a DPB per "layer" is used.
 */
class H264AVCDECODERLIB_API DecodedPicBuffer
{
protected:
  DecodedPicBuffer          ();
  virtual ~DecodedPicBuffer ();

public:
  static ErrVal       create              ( DecodedPicBuffer*&          rpcDecodedPicBuffer );
  ErrVal              destroy             ();
  ErrVal              init                ( YuvBufferCtrl*              pcFullPelBufferCtrl,
                                            UInt                        uiLayer ); // just for dump output
  ErrVal              initSPS             ( const SequenceParameterSet& rcSPS,
                                            UInt                        uiDependencyId );
  ErrVal              uninit              ();


  ErrVal              initCurrDPBUnit     ( DPBUnit*&                   rpcCurrDPBUnit,
                                            SliceHeader*                pcSliceHeader,
                                            PicBufferList&              rcOutputList,
                                            PicBufferList&              rcUnusedList,
                                            Bool                        bFirstSliceInLayerRepresentation );
  ErrVal              initPicCurrDPBUnit  ( PicBuffer*&                 rpcPicBuffer,
                                            Bool                        bRefinement );

  DPBUnit*            getLastUnit         ();
  ErrVal              setPrdRefLists      ( DPBUnit*                    pcCurrDPBUnit );
  ErrVal              store               ( DPBUnit*&                   rpcDPBUnit,
                                            PicBufferList&              rcOutputList,
                                            PicBufferList&              rcUnusedList,
                                            Frame*                      pcFrameBaseRep = 0 );
  ErrVal              update              ( DPBUnit*                    pcDPBUnit );
  ErrVal              clear               ( PicBufferList&              rcOutputList,
                                            PicBufferList&              rcUnusedList );

  ErrVal              slidingWindowBase   ( UInt                        mCurrFrameNum );
  ErrVal              MMCOBase            ( SliceHeader*                pcSliceHeader,
                                            UInt                        mCurrFrameNum );

protected:
  Void                xSetIdentifier          ( UInt&                       uiNum, 
		                                            PicType&                    rePicType, 
																						    const PicType               eCurrentPicType );
  ErrVal              xCreateData             ( UInt                        uiMaxPicsInDPB,
                                                const SequenceParameterSet& rcSPS,
                                                UInt                        uiDependencyId );
  ErrVal              xDeleteData             ();
  
  ErrVal              xClearBuffer            (); // remove non-ref frames that are not output pictures
  ErrVal              xUpdateMemory           ( SliceHeader*                pcSliceHeader );
  ErrVal              xSlidingWindow          ();
  ErrVal              xMMCO                   ( SliceHeader*                pcSliceHeader );

  ErrVal              xMarkShortTermUnusedBase( const PicType               ePicType,
                                                UInt						            mCurrFrameNum,  
                                                UInt                        uiDiffOfPicNums );
  ErrVal              xMarkShortTermUnused    ( const PicType               ePicType,
                                                const DPBUnit*              pcCurrentDPBUnit,
                                                UInt                        uiDiffOfPicNums );
  
  ErrVal              xOutput                 ( PicBufferList&              rcOutputList,
                                                PicBufferList&              rcUnusedList );
  ErrVal              xClearOutputAll         ( PicBufferList&              rcOutputList,
                                                PicBufferList&              rcUnusedList,
                                                Bool                        bFinal );            


  ErrVal              xStorePicture           ( DPBUnit*                    pcDPBUnit, // just for checking
                                                PicBufferList&              rcOutputList,
                                                PicBufferList&              rcUnusedList,
                                                Bool                        bTreatAsIdr,
                                                Bool                        bFrameMbsOnlyFlag,
                                                Bool                        bRefinement );
  ErrVal              xCheckMissingPics       ( const SliceHeader*          pcSliceHeader,
                                                PicBufferList&              rcOutputList,
                                                PicBufferList&              rcUnusedList );


  ErrVal              xInitPrdListPSlice      ( RefFrameList&               rcList0,
                                                Frame*                      pcCurrentFrame,
                                                PicType                     eCurrentPicType,
																						    SliceType                   eSliceType );
  ErrVal              xInitPrdListsBSlice     ( RefFrameList&               rcList0,
                                                RefFrameList&               rcList1,
                                                Frame*                      pcCurrentFrame,
                                                PicType                     eCurrentPicType,
																						    SliceType                   eSliceType );

  ErrVal              xSetInitialRefFieldList ( RefFrameList&               rcList,
                                                Frame*                      pcCurrentFrame,
                                                PicType                     eCurrentPicType,
																						    SliceType                   eSliceType );
  ErrVal              xPrdListRemapping       ( RefFrameList&               rcList,
                                                ListIdx                     eListIdx,
                                                SliceHeader*                pcSliceHeader );
  ErrVal              xUpdateDPBUnitList      ( DPBUnit                     *pcDPBUNit  );
  
  //===== debugging ======
  ErrVal              xDumpDPB                ();
  ErrVal              xDumpRefList            ( RefFrameList&               rcList,
		                                            ListIdx                     eListIdx );
private:
  Bool                m_bInitDone;
  YuvBufferCtrl*      m_pcYuvBufferCtrl;

  UInt                m_uiLayer;
  UInt                m_uiNumRefFrames;
  UInt                m_uiMaxFrameNum;
  UInt                m_uiLastRefFrameNum;
  DPBUnitList         m_cUsedDPBUnitList;
  DPBUnitList         m_cFreeDPBUnitList;
  DPBUnit*            m_pcCurrDPBUnit;
  DPBUnit*            m_pcLastDPBUnit;
  PicBufferList       m_cPicBufferList;
};



class H264AVCDECODERLIB_API MbStatus
{
public:
  MbStatus();
  virtual ~MbStatus();

  Void    reset           ();
  Bool    canBeUpdated    ( const SliceHeader*  pcSliceHeader );
  ErrVal  update          ( SliceHeader*        pcSliceHeader );

  SliceHeader*        getSliceHeader    ()        { return    m_pcSliceHeader; }
  const SliceHeader*  getSliceHeader    ()  const { return    m_pcSliceHeader; }
  UInt                getSliceIdc       ()  const { return    m_uiSliceIdc; }
  UInt                getFirstMbInSlice ()  const { return    m_uiSliceIdc >> 7; }
  UInt                getDQId           ()  const { return    m_uiSliceIdc        & 0x7F; }
  UInt                getDependencyId   ()  const { return  ( m_uiSliceIdc >> 4 ) & 0x7; }
  UInt                getQualityId      ()  const { return    m_uiSliceIdc        & 0xF; }
  Bool                isCoded           ()  const { return    m_bIsCoded; }

private:
  UInt          m_uiSliceIdc;
  Bool          m_bIsCoded;
  SliceHeader*  m_pcSliceHeader;
};



class H264AVCDECODERLIB_API LayerDecoder
{ 
  enum { NUM_TMP_FRAMES = 2 };

protected:
	LayerDecoder         ();
	virtual ~LayerDecoder();

public:
  //===== general functions ======
  static  ErrVal  create        ( LayerDecoder*&          rpcLayerDecoder );
  ErrVal          destroy       ();
  ErrVal          init          ( UInt                    uiDependencyId,
                                  H264AVCDecoder*         pcH264AVCDecoder,
                                  NalUnitParser*          pcNalUnitParser,
                                  SliceReader*            pcSliceReader,
                                  SliceDecoder*           pcSliceDecoder,
                                  ControlMngIf*           pcControlMng,
                                  LoopFilter*             pcLoopFilter,
                                  HeaderSymbolReadIf*     pcHeaderSymbolReadIf,
                                  ParameterSetMng*        pcParameterSetMng,
                                  PocCalculator*          pcPocCalculator,
                                  YuvBufferCtrl*          pcYuvFullPelBufferCtrl,
                                  DecodedPicBuffer*       pcDecodedPictureBuffer,
                                  MotionCompensation*     pcMotionCompensation,
								                  ReconstructionBypass*   pcReconstructionBypass
#ifdef SHARP_AVC_REWRITE_OUTPUT
                                  ,RewriteEncoder*        pcRewriteEncoder
#endif
                                  );
  ErrVal          uninit        ();

  //===== main processing functions =====
  ErrVal  processSliceData      ( PicBuffer*              pcPicBuffer,
                                  PicBufferList&          rcPicBufferOutputList,
                                  PicBufferList&          rcPicBufferUnusedList,
                                  BinDataList&            rcBinDataList,
                                  SliceDataNALUnit&       rcSliceDataNALUnit );
  ErrVal  finishProcess         ( PicBufferList&          rcPicBufferOutputList,
                                  PicBufferList&          rcPicBufferUnusedList );

  //===== returning data =====
  ErrVal            getBaseLayerDataAvailability  ( Frame*&           pcFrame,
                                                    Frame*&           pcResidual,
                                                    MbDataCtrl*&      pcMbDataCtrl,
                                                    Bool&             bBaseDataAvailable );
  ErrVal            getBaseLayerData              ( Frame*&           pcFrame,
                                                    Frame*&           pcResidual,
                                                    MbDataCtrl*&      pcMbDataCtrl,
                                                    Bool&             rbConstrainedIPred,
                                                    Bool              bSpatialScalability );
  ErrVal            getBaseSliceHeader            ( SliceHeader*&     rpcSliceHeader );

  Void              setResizeParameters           ( ResizeParameters* params )        { m_pcResizeParameter = params; }
  ResizeParameters* getResizeParameters           ()                                  { return m_pcResizeParameter; }
  Int               getSpatialScalabilityType     ()                            const { return m_pcResizeParameter->m_iSpatialScalabilityType; }
  UInt              getFrameWidth                 ()                            const { return m_uiFrameWidthInMb*16; }
  UInt              getFrameHeight                ()                            const { return m_uiFrameHeightInMb*16; }
  Frame*            getBaseLayerResidual          ()                                  { return m_pcBaseLayerResidual; }

private:
  //===== create data arrays =====
  ErrVal  xCreateData                 ( const SequenceParameterSet&   rcSPS );
  ErrVal  xDeleteData                 ();

  //===== initialization =====
  ErrVal  xReadSliceHeader            ( SliceHeader*&           rpcSliceHeader,
                                        SliceDataNALUnit&       rcSliceDataNalUnit );
  ErrVal  xInitSliceHeader            ( SliceHeader&            rcSliceHeader,
                                        const SliceDataNALUnit& rcSliceDataNalUnit );
  ErrVal  xInitSPS                    ( const SliceHeader&      rcSliceHeader );
  ErrVal  xInitDPBUnit                ( SliceHeader&            rcSliceHeader,
                                        PicBuffer*              pcPicBuffer,
                                        PicBufferList&          rcPicBufferOutputList,
                                        PicBufferList&          rcPicBufferUnusedList );

  //===== slice processing =====
  ErrVal  xInitSlice                  ( SliceHeader*&           rpcSliceHeader,
                                        PicBuffer*              pcPicBuffer,
                                        PicBufferList&          rcPicBufferOutputList,
                                        PicBufferList&          rcPicBufferUnusedList,
                                        SliceDataNALUnit&       rcSliceDataNalUnit );
  ErrVal  xParseSlice                 ( SliceHeader&            rcSliceHeader );
  ErrVal  xDecodeSlice                ( SliceHeader&            rcSliceHeader,
                                        const SliceDataNALUnit& rcSliceDataNalUnit,
                                        Bool                    bFirstSliceInLayerRepresentation );
  ErrVal  xFinishLayerRepresentation  ( SliceHeader&            rcSliceHeader,
                                        PicBufferList&          rcPicBufferOutputList,
                                        PicBufferList&          rcPicBufferUnusedList,
                                        const SliceDataNALUnit& rcSliceDataNalUnit,
                                        BinDataList&            rcBinDataList );
  ErrVal  xFinishSlice                ( SliceHeader&            rcSliceHeader,
                                        PicBufferList&          rcPicBufferOutputList,
                                        PicBufferList&          rcPicBufferUnusedList,
                                        const SliceDataNALUnit& rcSliceDataNalUnit,
                                        BinDataList&            rcBinDataList );

  //===== picture processing =====
  ErrVal  xCheckForMissingSlices      ( const SliceDataNALUnit& rcSliceDataNalUnit );
  ErrVal  xSetLoopFilterQPs           ( MbDataCtrl&             rcMbDataCtrl );
#ifdef SHARP_AVC_REWRITE_OUTPUT
  ErrVal  xRewritePicture             ( BinDataList&            rcBinDataList,
                                        MbDataCtrl&             rcMbDataCtrl );
#endif

  //===== base layer processing =====
  ErrVal  xInitESSandCroppingWindow   ( SliceHeader&            rcSliceHeader,
                                        MbDataCtrl&             rcMbDataCtrl,
                                        ControlData&            rcControlData ); 
  ErrVal  xInitBaseLayer              ( ControlData&            rcControlData,
												                SliceHeader*&           rcSliceHeaderBase,
                                        Bool                    bFirstSliceInLayerRepresentation );
  ErrVal  xGetBaseLayerData           ( ControlData&            rcControlData,
                                        Frame*&                 rpcBaseFrame,
                                        Frame*&                 rpcBaseResidual,
                                        MbDataCtrl*&            rpcBaseDataCtrl,
                                        Bool&                   rbBaseDataAvailable,
                                        Bool&                   rbConstrainedIPredBL,
                                        Bool&                   rbSpatialScalability,
                                        ResizeParameters*       pcResizeParameter );
  ErrVal  xConstrainedIntraUpsampling ( Frame*                  pcFrame,
                                        Frame*                  pcUpsampling, 
                                        Frame*                  pcTemp,
                                        MbDataCtrl*             pcBaseDataCtrl,
                                        ReconstructionBypass*   pcReconstructionBypass,
                                        ResizeParameters*       pcResizeParameters,
                                        PicType                 ePicType );
  Void    xGetPosition                ( ResizeParameters*       pcResizeParameters,
                                        Int*                    px,
                                        Int*                    py,
                                        Bool                    uv_flag );
  Void    xSetMCResizeParameters      ( ResizeParameters*				resizeParameters );

protected:
  //----- references -----
  H264AVCDecoder*       m_pcH264AVCDecoder;
  NalUnitParser*        m_pcNalUnitParser;
  SliceReader*          m_pcSliceReader;
  SliceDecoder*         m_pcSliceDecoder;
  ControlMngIf*         m_pcControlMng;
  LoopFilter*           m_pcLoopFilter;
  HeaderSymbolReadIf*   m_pcHeaderSymbolReadIf;
  ParameterSetMng*      m_pcParameterSetMng;
  PocCalculator*        m_pcPocCalculator;
  YuvBufferCtrl*        m_pcYuvFullPelBufferCtrl;
  DecodedPicBuffer*     m_pcDecodedPictureBuffer;
  MotionCompensation*   m_pcMotionCompensation;
  ReconstructionBypass* m_pcReconstructionBypass;
#ifdef SHARP_AVC_REWRITE_OUTPUT
  RewriteEncoder*       m_pcRewriteEncoder;
#endif
  DownConvert           m_cDownConvert;

  //----- general parameters -----
  Bool                  m_bInitialized;
  Bool                  m_bSPSInitialized;
  Bool                  m_bDependencyRepresentationInitialized;
  Bool                  m_bLayerRepresentationInitialized;
  UInt                  m_uiFrameWidthInMb;
  UInt                  m_uiFrameHeightInMb;
  UInt                  m_uiMbNumber;
  UInt                  m_uiDependencyId;
  UInt                  m_uiQualityId;

  //----- macroblock status and slice headers -----
  MbStatus*             m_pacMbStatus;
  MyList<SliceHeader*>  m_cSliceHeaderList;

  //----- frame memories, control data, and references  -----
  ResizeParameters*     m_pcResizeParameter;
  DPBUnit*              m_pcCurrDPBUnit;
  MbDataCtrl*           m_pcBaseLayerCtrl;
  MbDataCtrl*           m_pcBaseLayerCtrlField;
  Frame*                m_pcResidual;
  Frame*                m_pcILPrediction;
  Frame*                m_pcBaseLayerFrame;
  Frame*                m_pcBaseLayerResidual;
  Frame*                m_apcFrameTemp[NUM_TMP_FRAMES];
};

H264AVC_NAMESPACE_END


#endif // !defined(AFX_GOPDECODER_H__339878FC_BA98_4ABE_8530_E1676196576F__INCLUDED_)
