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
#include "DownConvert.h"

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
class RQFGSEncoder;



typedef MyList<UInt>        UIntList;


#define TARGET_MOTION_RATE_FACTOR   0.670
#define MAX_MOTION_RATE_FACTOR      0.800
#define MOTION_RATE_INC_FACTOR      1.500



class H264AVCENCODERLIB_API AccessUnit
{
public:
  AccessUnit  ( Int         iPoc )  : m_iPoc( iPoc )        {}
  ~AccessUnit ()                                            {}

  Int                     getPoc          () const          { return m_iPoc; }
  ExtBinDataAccessorList& getNalUnitList  ()                { return m_cNalUnitList; }
  
private:
  Int                     m_iPoc;
  ExtBinDataAccessorList  m_cNalUnitList;
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




class H264AVCENCODERLIB_API MCTFEncoder
{
  enum
  {
    NUM_TMP_FRAMES  = 2
  };
  enum
  {
    LPS = 0x00,   // low pass signal
    HPS = 0x01    // high pass signal
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
                                      RQFGSEncoder*                   pcRQFGSEncoder,
                                      LoopFilter*                     pcLoopFilter,
                                      PocCalculator*                  pcPocCalculator,
                                      NalUnitEncoder*                 pcNalUnitEncoder,
                                      YuvBufferCtrl*                  pcYuvFullPelBufferCtrl,
                                      YuvBufferCtrl*                  pcYuvHalfPelBufferCtrl,
                                      QuarterPelFilter*               pcQuarterPelFilter,
                                      MotionEstimation*               pcMotionEstimation );
  ErrVal        initParameterSets   ( const SequenceParameterSet&     rcSPS,
                                      const PictureParameterSet&      rcPPSLP,
                                      const PictureParameterSet&      rcPPSHP );
  ErrVal        uninit              ();
 
  ErrVal        addParameterSetBits ( UInt                            uiParameterSetBits );
  Bool          firstGOPCoded       ()                                { return m_bFirstGOPCoded; }
  ErrVal        process             ( AccessUnitList&                 rcAccessUnitList,
                                      PicBufferList&                  rcPicBufferInputList,
                                      PicBufferList&                  rcPicBufferOutputList,
                                      PicBufferList&                  rcPicBufferUnusedList );
  ErrVal        finish              ( UInt&                           ruiNumCodedFrames,
                                      Double&                         rdOutputRate );




  Int           getFrameWidth       ()                                { return 16*m_uiFrameWidthInMb; }
  Int           getFrameHeight      ()                                { return 16*m_uiFrameHeightInMb; }
  ErrVal        getBaseLayerStatus  ( Bool&                           bExists,
                                      Bool&                           bMotion,
                                      Int                             iPoc );
  ErrVal        getBaseLayerData    ( IntFrame*&                      pcFrame,
                                      IntFrame*&                      pcResidual,
                                      MbDataCtrl*&                    pcMbDataCtrl,
                                      Bool&                           bForCopyOnly,
                                      Int                             iSpatialScalability,
                                      Int                             iPoc,
                                      Bool                            bMotion );

  UInt*         getGOPBitsBase      ()  { return m_auiCurrGOPBitsBase;  }
  UInt*         getGOPBitsFGS       ()  { return m_auiCurrGOPBitsFGS;   }


  //===== ESS =====
  Int                     getSpatialScalabilityType() { return m_pcResizeParameters->m_iSpatialScalabilityType; }
  ResizeParameters*       getResizeParameters()       { return m_pcResizeParameters; }

  //{{Adaptive GOP structure
  // --ETRI & KHU
  ErrVal        process_ags         ( AccessUnitList&                 rcAccessUnitList,
                                      PicBufferList&                  rcPicBufferInputList,
                                      PicBufferList&                  rcPicBufferOutputList,
                                      PicBufferList&                  rcPicBufferUnusedList );
  UInt			getSelect(int gop, int index) {return m_uiSelect[gop][index];}
  void			setSelect(int gop, int index, UInt p) {m_uiSelect[gop][index] = p;}
  UInt			getSelectPos() {return m_uiSelectPos;}
  void			setSelectPos(UInt p) {m_uiSelectPos = p;}
  //}}Adaptive GOP structure

protected:
  ErrVal  xProcessClosedLoop            ( AccessUnitList&             rcAccessUnitList,
                                          PicBufferList&              rcPicBufferInputList,
                                          PicBufferList&              rcPicBufferOutputList,
                                          PicBufferList&              rcPicBufferUnusedList );
  //===== data management =====
  ErrVal  xCreateData                   ( const SequenceParameterSet& rcSPS );
  ErrVal  xDeleteData                   ();

  
  ErrVal  xInitGOP                      ( PicBufferList&              rcPicBufferInputList );
  ErrVal  xUpdateBitCounts              ();
  ErrVal  xFinishGOP                    ( PicBufferList&              rcPicBufferInputList,
                                          PicBufferList&              rcPicBufferOutputList,
                                          PicBufferList&              rcPicBufferUnusedList );

  ErrVal  xInitExtBinDataAccessor       ( ExtBinDataAccessor&         rcExtBinDataAccessor );
  ErrVal  xAppendNewExtBinDataAccessor  ( ExtBinDataAccessorList&     rcExtBinDataAccessorList,
                                          ExtBinDataAccessor*         pcExtBinDataAccessor,
                                          Bool                        bModifyDataAccessor = true );


  
  //===== decomposition / composition =====
  ErrVal  xMotionEstimationStage        ( UInt                        uiBaseLevel );
  ErrVal  xDecompositionStage           ( UInt                        uiBaseLevel );
  ErrVal  xCompositionStage             ( UInt                        uiBaseLevel,
                                          PicBufferList&              rcPicBufferInputList );
  ErrVal  xStoreReconstruction          ( PicBufferList&              rcPicBufferOutputList );
  ErrVal  xStoreOriginalPictures        ();
  ErrVal  xSwitchOriginalPictures       ();




  //===== control data initialization =====
  ErrVal  xSetScalingFactorsMCTF        ( UInt                        uiBaseLevel );
  //{{Quality level estimation and modified truncation- JVTO044 and m12007
  //France Telecom R&D-(nathalie.cammas@francetelecom.com)
  ErrVal  xSetScalingFactorsMCTF_FT     ( UInt                        uiBaseLevel );
  //}}Quality level estimation and modified truncation- JVTO044 and m12007
  ErrVal  xSetScalingFactorsAVC         ();
  ErrVal  xGetListSizes                 ( UInt                        uiTemporalLevel,
                                          UInt                        uiFrameIdInGOP,
                                          UInt                        auiPredListSize[2],
                                          UInt                        aauiUpdListSize[MAX_DSTAGES][2] );
  ErrVal  xSetBaseLayerData             ( UInt                        uiFrameIdInGOP );
  ErrVal  xInitReordering               ( UInt                        uiFrameIdInGOP );
  ErrVal  xInitSliceHeader              ( UInt                        uiTemporalLevel,
                                          UInt                        uiFrameIdInGOP );
  ErrVal  xClearBufferExtensions        ();
  ErrVal  xGetPredictionLists           ( RefFrameList&               rcRefList0,
                                          RefFrameList&               rcRefList1,
                                          UInt                        uiBaseLevel,
                                          UInt                        uiFrame,
                                          Bool                        bHalfPel = false );
  ErrVal  xGetBQPredictionLists         ( RefFrameList&               rcRefList0,
                                          RefFrameList&               rcRefList1,
                                          UInt                        uiBaseLevel,
                                          UInt                        uiFrame );
  ErrVal  xGetUpdateLists               ( RefFrameList&               rcRefList0,
                                          RefFrameList&               rcRefList1,
                                          CtrlDataList&               rcCtrlList0,
                                          CtrlDataList&               rcCtrlList1,
                                          UInt                        uiBaseLevel,
                                          UInt                        uiFrame );
  ErrVal  xInitBaseLayerData            ( ControlData&                rcControlData, 
                                          UInt                        uiBaseLevel,  //TMM_ESS
                                          UInt                        uiFrame,      //TMM_ESS
                                          Bool                        bMotion = false ); 
  ErrVal  xInitControlDataMotion        ( UInt                        uiBaseLevel,
                                          UInt                        uiFrame,
                                          Bool                        bMotionEstimation );
  ErrVal  xInitControlDataLowPass       ( UInt                        uiFrameIdInGOP,
                                          UInt                        uiBaseLevel,  //TMM_ESS
                                          UInt                        uiFrame  );   //TMM_ESS
  ErrVal  xInitControlDataHighPass      ( UInt                        uiFrameIdInGOP,
                                          UInt                        uiBaseLevel,   //TMM_ESS
                                          UInt                        uiFrame  );    //TMM_ESS
  ErrVal  xGetConnections               ( ControlData&                rcControlData,
                                          Double&                     rdL0Rate,
                                          Double&                     rdL1Rate,
                                          Double&                     rdBiRate );


  //===== stage encoding =====
  ErrVal  xEncodeLowPassPictures        ( AccessUnitList&             rcAccessUnitList );
  ErrVal  xEncodeHighPassPictures       ( AccessUnitList&             rcAccessUnitList,
                                          UInt                        uiBaseLevel );

  //===== basic encoding =====
  ErrVal  xEncodeLowPassSignal          ( ExtBinDataAccessorList&     rcOutExtBinDataAccessorList,
                                          ControlData&                rcResidualControlData,
                                          IntFrame*                   pcFrame,
                                          IntFrame*                   pcRecSubband,
                                          IntFrame*                   pcPredSignal,
                                          UInt&                       ruiBits);
  ErrVal  xEncodeHighPassSignal         ( ExtBinDataAccessorList&     rcOutExtBinDataAccessorList,
                                          ControlData&                rcControlData,
                                          IntFrame*                   pcFrame,
                                          IntFrame*                   pcResidual,
                                          IntFrame*                   pcPredSignal,
                                          UInt&                       ruiBits,
                                          UInt&                       ruiBitsRes );
  ErrVal  xEncodeFGSLayer               ( ExtBinDataAccessorList&     rcOutExtBinDataAccessorList,
                                          ControlData&                rcControlData,
                                          IntFrame*                   pcFrame,
                                          IntFrame*                   pcResidual,
                                          IntFrame*                   pcPredSignal,
                                          IntFrame*                   pcTempFrame,
                                          IntFrame*                   pcSubband,
                                          UInt&                       ruiBits );


  //===== motion estimation / compensation =====
  ErrVal  xMotionCompensation           ( IntFrame*                   pcMCFrame,
                                          RefFrameList*               pcRefFrameList0,
                                          RefFrameList*               pcRefFrameList1,
                                          MbDataCtrl*                 pcMbDataCtrl,
                                          SliceHeader&                rcSH );
  ErrVal  xMotionEstimation             ( RefFrameList*               pcRefFrameList0,
                                          RefFrameList*               pcRefFrameList1,
                                          const IntFrame*             pcOrigFrame,
                                          IntFrame*                   pcIntraRec,
                                          ControlData&                rcControlData,
                                          Bool                        bBiPredOnly,
                                          UInt                        uiNumMaxIter,
                                          UInt                        uiIterSearchRange,
                                          UInt                        uiIntraMode );


  //===== auxiliary functions =====
  ErrVal  xCalculateAndAddPSNR          ( PicBufferList&              rcPicBufferInputList,
                                          UInt                        uiStage,
                                          Bool                        bOutput );

  ErrVal  xFillAndUpsampleFrame         ( IntFrame*                   rcFrame );
  ErrVal  xFillAndExtendFrame           ( IntFrame*                   rcFrame );
  ErrVal  xZeroIntraMacroblocks         ( IntFrame*                   pcFrame,
                                          ControlData&                pcCtrlData );
  ErrVal  xClipIntraMacroblocks         ( IntFrame*                   pcFrame,
                                          ControlData&                rcCtrlData,
                                          Bool                        bClipAll );
  ErrVal  xAddBaseLayerResidual         ( ControlData&                rcControlData,
                                          IntFrame*                   pcFrame,
                                          Bool                        bSubtract );


  //===== slice header =====
  ErrVal        xSetRplr            ( RplrBuffer& rcRplrBuffer, UIntList cFrameNumList, UInt uiCurrFrameNr );
  ErrVal        xSetRplrAndMmco     ( SliceHeader& rcSH );
  ErrVal        xWriteSEI           ( ExtBinDataAccessorList& rcOutExtBinDataAccessorList, SliceHeader& rcSH, UInt& ruiBit );
  ErrVal        xGetFrameNumList    ( SliceHeader& rcSH, UIntList& rcFrameNumList, ListIdx eLstIdx, UInt uiCurrBasePos );
  MbDataCtrl*   xGetMbDataCtrlL1    ( SliceHeader& rcSH, UInt uiCurrBasePos );
  Void          xAssignSimplePriorityId ( SliceHeader *pcSliceHeader );
  
   //===== ESS =====
   ErrVal		xFillPredictionLists_ESS( UInt uiBaseLevel , UInt uiFrame );


  //===== adaptive gop structure =====
  //{{Adaptive GOP structure
  // --ETRI & KHU  
  UInt	xSelectGOPMode	(Double **mse, UInt *seltype, UInt pos, UInt gop_size, UInt gn_pos, UInt gn_1_pos);
  //}}Adaptive GOP structure

protected:
  //----- instances -----
  ExtBinDataAccessor            m_cExtBinDataAccessor;
  BinData                       m_cBinData;
  DownConvert                   m_cDownConvert;
  ConnectionArray               m_cConnectionArray;

  //----- references -----
  const SequenceParameterSet*   m_pcSPS;
  const PictureParameterSet*    m_pcPPSLP;
  const PictureParameterSet*    m_pcPPSHP;
  YuvBufferCtrl*                m_pcYuvFullPelBufferCtrl;
  YuvBufferCtrl*                m_pcYuvHalfPelBufferCtrl;
  PocCalculator*                m_pcPocCalculator;
  H264AVCEncoder*               m_pcH264AVCEncoder;
  SliceEncoder*                 m_pcSliceEncoder;
  NalUnitEncoder*               m_pcNalUnitEncoder;
  LoopFilter*                   m_pcLoopFilter;
  QuarterPelFilter*             m_pcQuarterPelFilter;
  MotionEstimation*             m_pcMotionEstimation;
  RQFGSEncoder*                 m_pcRQFGSEncoder;

  //----- fixed control parameters ----
  Bool                          m_bTraceEnable;                       // trace file
  UInt                          m_uiLayerId;                          // layer id for current layer
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
  UInt                          m_uiIntraMode;                        // intra mode usage for MCTF
  UInt                          m_uiNumMaxIter;                       // maximum number of iteration for bi-directional search
  UInt                          m_uiIterSearchRange;                  // search range for iterative search
  UInt                          m_iMaxDeltaQp;                        // maximum QP changing
  UInt                          m_uiClosedLoopMode;                   // closed-loop coding mode (0:open-loop)
  Bool                          m_bUpdate;                            // usage of update steps
  Bool                          m_bH264AVCCompatible;                 // H.264/AVC compatibility
  Bool                          m_bInterLayerPrediction;              // inter-layer prediction
  Bool                          m_bAdaptivePrediction;                // adaptive inter-layer prediction
  Bool                          m_bHaarFiltering;                     // haar-based decomposition
  Bool                          m_bBiPredOnly;                        // only bi-direktional prediction
  Bool                          m_bAdaptiveQP;                        // QP's are set based on chosen modes
  Bool                          m_bWriteSubSequenceSei;               // Subsequence SEI message (H.264/AVC base layer)
  Double                        m_adBaseQpLambdaMotion[MAX_DSTAGES];  // base QP's for mode decision and motion estimation
  Double                        m_dBaseQPResidual;                    // base residual QP
  Double                        m_dNumFGSLayers;                      // number of FGS layers

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
  Bool                          m_bLowComplxUpdFlag;

  //----- frame memories -----
  IntFrame*                     m_apcFrameTemp[NUM_TMP_FRAMES];       // auxiliary frame memories
  IntFrame**                    m_papcFrame;                          // frame stores
  IntFrame**                    m_papcBQFrame;                        // base quality frames
#if MULTIPLE_LOOP_DECODING
  IntFrame**                    m_papcFrameILPred;
#endif
  IntFrame**                    m_papcResidual;                       // frame stores for residual data
  IntFrame**                    m_papcSubband;                        // reconstructed subband pictures
  IntFrame*                     m_apcLowPassTmpOrg  [2];              // temporal storage for original low-pass pcitures
  IntFrame*                     m_pcLowPassBaseReconstruction;        // base reconstruction of last low-pass picture
  IntFrame*                     m_pcAnchorFrameOriginal;              // original anchor frame
  IntFrame*                     m_pcAnchorFrameReconstructed;         // reconstructed anchor frame
  IntFrame*                     m_pcBaseLayerFrame;                   // base layer frame
  IntFrame*                     m_pcBaseLayerResidual;                // base layer residual

  //----- control data arrays -----
  ControlData*                  m_pacControlData;                     // control data arrays
  ControlData                   m_cControlDataUpd;                    // control data for update steps
  MbDataCtrl*                   m_pcBaseLayerCtrl;                    // macroblock data of the base layer pictures

  //----- auxiliary buffers -----
  UInt                          m_uiWriteBufferSize;                  // size of temporary write buffer
  UChar*                        m_pucWriteBuffer;                     // write buffer
  UShort*                       m_pusUpdateWeights;                   // array for storing update weights

  //----- PSNR & rate  -----
  Double                        m_fOutputFrameRate;
  UInt                          m_uiParameterSetBits;
  UInt                          m_auiNumFramesCoded [MAX_DSTAGES+1];
  UInt                          m_auiCurrGOPBitsBase[MAX_DSTAGES+1];
  UInt                          m_auiCurrGOPBitsFGS [MAX_DSTAGES+1];
  Double                        m_adSeqBitsBase     [MAX_DSTAGES+1];
  Double                        m_adSeqBitsFGS      [MAX_DSTAGES+1];
  Double                        m_adPSNRSumY        [MAX_DSTAGES+1];
  Double                        m_adPSNRSumU        [MAX_DSTAGES+1];
  Double                        m_adPSNRSumV        [MAX_DSTAGES+1];

  //----- FGS -----
  UInt                          m_uiFGSMode;
  FILE*                         m_pFGSFile;
  Double                        m_dFGSBitRateFactor;
  Double                        m_dFGSRoundingOffset;
  Int                           m_iLastFGSError;
  UInt                          m_uiNotYetConsideredBaseLayerBits;
  //{{Quality level estimation and modified truncation- JVTO044 and m12007
  //France Telecom R&D-(nathalie.cammas@francetelecom.com)
  FILE*                         m_pRateFile;
  FILE*                         m_pDistoFile;
  Bool							m_bQualityLevelsEstimation;
  //}}Quality level estimation and modified truncation- JVTO044 and m12007
  Bool                          m_bExtendedPriorityId;

  //{{Adaptive GOP structure
  // --ETRI & KHU
  UInt			m_uiWriteGOPMode;
  UInt			m_uiUseAGS;
  UInt			m_uiSelectPos;
  UInt*			m_puiGOPMode;
  UInt**		m_uiSelect;
  Double		m_dMSETemp;
  Bool      m_bFinish;
  std::string	m_cGOPModeFilename;
  //}}Adaptive GOP structure

#if MULTIPLE_LOOP_DECODING
  Bool                          m_bCompletelyDecodeLayer;
  Bool                          m_bLayerReconstructionNotPossible;
  Bool                          m_bHighestLayer;
#endif

 //----- ESS -----
  ResizeParameters*				m_pcResizeParameters; 
};

#if defined( WIN32 )
# pragma warning( default: 4251 )
#endif

H264AVC_NAMESPACE_END

#endif // !defined(AFX_GOPENCODER_H__75F41F36_C28D_41F9_AB5E_4C90D66D160C__INCLUDED_)
