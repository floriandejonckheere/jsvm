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

// TMM_ESS 
#include "ResizeParameters.h"

H264AVC_NAMESPACE_BEGIN

class H264AVCDecoder;
class SliceReader;
class SliceDecoder;
class FrameMng;
class PocCalculator;
class LoopFilter;
class HeaderSymbolReadIf;
class ParameterSetMng;
class NalUnitParser;
class ControlMngIf;
class MotionCompensation;
class IntFrame; 
class RQFGSDecoder;

class ReconstructionBypass;
class YuvBufferCtrl; 



class H264AVCDECODERLIB_API DPBUnit
{
protected:
  DPBUnit           ();
  virtual ~DPBUnit  ();

public:
  static ErrVal create        ( DPBUnit*&                   rpcDPBUnit,
                                YuvBufferCtrl&              rcYuvBufferCtrl,
                                const SequenceParameterSet& rcSPS );
  ErrVal        destroy       ();


  ErrVal        init          ( Int       iPoc,
                                UInt      uiFrameNum,
                                UInt      uiTemporalLevel,
                                Bool      bKeyPicture,
                                Bool      bNeededForReference,
                                Bool      bConstrainedIPred );
  ErrVal        initNonEx     ( Int       iPoc,
                                UInt      uiFrameNum );
  ErrVal        initBase      ( DPBUnit&  rcDPBUnit,
                                IntFrame* pcFrameBaseRep );
  ErrVal        uninit        ();


  ErrVal        markNonRef    ();
  ErrVal        markOutputted ();


  Int           getPoc        ()  const { return m_iPoc; }
  UInt          getFrameNum   ()  const { return m_uiFrameNum; }
  UInt          getTLevel     ()  const { return m_uiTemporalLevel; }
  Bool          isKeyPic      ()  const { return m_bKeyPicture; }
  Bool          isExisting    ()  const { return m_bExisting; }
  Bool          isNeededForRef()  const { return m_bNeededForReference; }
  Bool          isOutputted   ()  const { return m_bOutputted; }
  Bool          isBaseRep     ()  const { return m_bBaseRepresentation; }
  Bool          isConstrIPred ()  const { return m_bConstrainedIntraPred; }
  IntFrame*     getFrame      ()        { return m_pcFrame; }
  ControlData&  getCtrlData   ()        { return m_cControlData; }

  Int           getPicNum     ( UInt uiCurrFrameNum, 
                                UInt uiMaxFrameNum ) const
  {
    if( m_uiFrameNum > uiCurrFrameNum )
    {
      return (Int)m_uiFrameNum - (Int)uiMaxFrameNum;
    }
    return (Int)m_uiFrameNum;
  }

private:
  Int         m_iPoc;
  UInt        m_uiFrameNum;
  UInt        m_uiTemporalLevel;
  Bool        m_bKeyPicture;
  Bool        m_bExisting;
  Bool        m_bNeededForReference;
  Bool        m_bOutputted;
  Bool        m_bBaseRepresentation;
  IntFrame*   m_pcFrame;
  ControlData m_cControlData;
  Bool        m_bConstrainedIntraPred;
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
  ErrVal              initSPS             ( const SequenceParameterSet& rcSPS );
  ErrVal              uninit              ();


  ErrVal              initCurrDPBUnit     ( DPBUnit*&                   rpcCurrDPBUnit,
                                            PicBuffer*&                 rpcPicBuffer,
                                            SliceHeader*                pcSliceHeader,
                                            PicBufferList&              rcOutputList,
                                            PicBufferList&              rcUnusedList );
  DPBUnit*            getLastUnit         ();
  DPBUnit*            getDPBUnit          ( Int                         iPoc );
  ErrVal              setPrdRefLists      ( DPBUnit*                    pcCurrDPBUnit );
//TMM_EC {{
  ErrVal							getPrdRefListsFromBase( DPBUnit*   pcCurrDPBUnit, SliceHeader* pBaseMbDataCtrl );
//TMM_EC }}
  ErrVal              store               ( DPBUnit*&                   rpcDPBUnit,
                                            PicBufferList&              rcOutputList,
                                            PicBufferList&              rcUnusedList,
                                            IntFrame*                   pcFrameBaseRep = NULL );
  ErrVal              update              ( DPBUnit*                    pcDPBUnit );
  ErrVal              clear               ( PicBufferList&              rcOutputList,
                                            PicBufferList&              rcUnusedList,
                                            Int&                        riMaxPoc );

  
  DPBUnit*            getCurrDPBUnit(){return        m_pcCurrDPBUnit;};
  ErrVal                 initPicBuffer(PicBuffer*&    rpcPicBuffer);
  ErrVal              initPicCurrDPBUnit( DPBUnit*&                   rpcCurrDPBUnit,
                                          PicBuffer*&                 rpcPicBuffer,
                                          Bool                        bResidual,
                                          SliceHeader*                pcSliceHeader,
                                          PicBufferList&              rcOutputList,
                                          PicBufferList&              rcUnusedList );

protected:
  ErrVal              xCreateData         ( UInt                        uiMaxPicsInDPB,
                                            const SequenceParameterSet& rcSPS );
  ErrVal              xDeleteData         ();
  

  ErrVal              xClearBuffer        (); // remove non-ref frames that are not output pictures
  ErrVal              xUpdateMemory       ( SliceHeader*                pcSliceHeader );
  ErrVal              xSlidingWindow      ();
  ErrVal              xMMCO               ( SliceHeader*                pcSliceHeader );
  ErrVal              xMarkShortTermUnused( DPBUnit*                    pcCurrentDPBUnit,
                                            UInt                        uiDiffOfPicNums );
  
  ErrVal              xOutput             ( PicBufferList&              rcOutputList,
                                            PicBufferList&              rcUnusedList );
  ErrVal              xClearOutputAll     ( PicBufferList&              rcOutputList,
                                            PicBufferList&              rcUnusedList,
                                            Int&                        riMaxPoc );

  
  ErrVal              xStorePicture       ( DPBUnit*                    pcDPBUnit, // just for checking
                                            PicBufferList&              rcOutputList,
                                            PicBufferList&              rcUnusedList,
                                            Bool                        bTreatAsIdr );
  ErrVal              xCheckMissingPics   ( SliceHeader*                pcSliceHeader,
                                            PicBufferList&              rcOutputList,
                                            PicBufferList&              rcUnusedList );

  ErrVal              xInitPrdListPSlice  ( RefFrameList&               rcList );
  ErrVal              xInitPrdListsBSlice ( RefFrameList&               rcList0,
                                            RefFrameList&               rcList1 );

  ErrVal              xPrdListRemapping   ( RefFrameList&               rcList,
                                            ListIdx                     eListIdx,
                                            SliceHeader*                pcSliceHeader );

  //===== debugging ======
  ErrVal              xDumpDPB            ();
  ErrVal              xDumpRefList        ( ListIdx                     eListIdx,
                                            RefFrameList&               rcList );


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
  PicBufferList       m_cPicBufferList;

};













enum
{
  LPS = 0x00,   // low pass signal
  HPS = 0x01    // high pass signal
};




class H264AVCDECODERLIB_API MCTFDecoder
{ 
  enum
  {
    NUM_TMP_FRAMES = 3 // Hanke@RWTH
  };
  enum NextNalType
  {
    ALL           = 0x00,
    LOW_PASS      = 0x01,
    LOW_PASS_IDR  = 0x02
  };

public:
//TMM_EC {{
	ErrVal			getECMethod( SliceHeader *rpcSliceHeader, ERROR_CONCEAL &m_eErrorConceal);
	UInt	m_bBaseLayerLost;
	SliceHeader	*m_pcVeryFirstSliceHeader;
	FrameMng	*m_pcFrameMng;
	ERROR_CONCEAL	m_eErrorConcealTemp;
	UInt			m_uiDecompositionStages;
	UInt			m_uiDecompositionStagesBase;
	ERROR_CONCEAL	m_eErrorConceal;
//TMM_EC }}
protected:
	MCTFDecoder         ();
	virtual ~MCTFDecoder();

public:
  //===== general functions ======
  static  ErrVal  create        ( MCTFDecoder*&               rpcMCTFDecoder );
  ErrVal          destroy       ();
  ErrVal          init          ( H264AVCDecoder*             pcH264AVCDecoder,
                                  SliceReader*                pcSliceReader,
                                  SliceDecoder*               pcSliceDecoder,
                                  RQFGSDecoder*               pcRQFGSDecoder,
                                  NalUnitParser*              pcNalUnitParser,
                                  ControlMngIf*               pcControlMng,
                                  LoopFilter*                 pcLoopFilter,
                                  HeaderSymbolReadIf*         pcHeaderSymbolReadIf,
                                  ParameterSetMng*            pcParameterSetMng,
                                  PocCalculator*              pcPocCalculator,
                                  YuvBufferCtrl*              pcYuvFullPelBufferCtrl,
                                  DecodedPicBuffer*           pcDecodedPictureBuffer,
                                  MotionCompensation*         pcMotionCompensation,
                                  QuarterPelFilter*           pcQuarterPelFilter );
  ErrVal          uninit        ();
  ErrVal          initSlice0    ( SliceHeader*                pcSliceHeader );
  ErrVal          initSlice     ( SliceHeader*                pcSliceHeader,
                                  UInt                        uiLastLayer );
  
  ErrVal          process       ( SliceHeader*&               rpcSliceHeader,
                                  PicBuffer*                  pcPicBuffer,
                                  PicBufferList&              rcPicBufferOutputList,
                                  PicBufferList&              rcPicBufferUnusedList,
                                  Bool                        bReconstructionLayer );
  ErrVal          finishProcess ( PicBufferList&              rcPicBufferOutputList,
                                  PicBufferList&              rcPicBufferUnusedList,
                                  Int&                        riMaxPoc );



  Bool            isActive        () { return m_bActive; }
  UInt            getFrameWidth   () { return m_uiFrameWidthInMb*16; }
  UInt            getFrameHeight  () { return m_uiFrameHeightInMb*16; }

  ErrVal          getBaseLayerData              ( IntFrame*&    pcFrame,
                                                  IntFrame*&    pcResidual,
                                                  MbDataCtrl*&  pcMbDataCtrl,
                                                  Bool&         rbConstrainedIPred,
                                                  Bool          bSpatialScalability,
                                                  Int           iPoc );
  ErrVal          getBaseLayerPWTable           ( SliceHeader::PredWeightTable*& rpcPredWeightTable,
                                                  ListIdx                        eListIdx,
                                                  Int                            iPoc );

  Void            setQualityLevelForPrediction  ( UInt ui )         { m_uiQualityLevelForPrediction = ui; }
#if MULTIPLE_LOOP_DECODING
  Void            setCompletelyDecodeLayer      ( Bool b )          { m_bCompletelyDecodeLayer = b; }
#endif
  PocCalculator*  getPocCalculator              ()                  { return m_pcPocCalculator; }

// TMM_ESS {
  Void              setResizeParameters ( ResizeParameters* params ) { m_pcResizeParameter = params; }
  ResizeParameters* getResizeParameters ()                           { return m_pcResizeParameter; }
  Int               getSpatialScalabilityType()                      { return m_pcResizeParameter->m_iSpatialScalabilityType; }
// TMM_ESS }
  void				setWaitForIdr(Bool b)	{ m_bWaitForIdr = b;}
  Bool				getWaitForIdr()			{ return m_bWaitForIdr;}

  ErrVal            setDiffPrdRefLists  ( RefFrameList&               diffPrdRefList,
                                          YuvBufferCtrl*              pcYuvFullPelBufferCtrl);
  ErrVal            freeDiffPrdRefLists ( RefFrameList& diffPrdRefList);

protected:
  //===== create and initialize data arrays =====
  ErrVal      xCreateData                     ( const SequenceParameterSet&   rcSPS );
  ErrVal      xDeleteData                     ();


  ErrVal      xZeroIntraMacroblocks           ( IntFrame*                     pcFrame,
                                                ControlData&                  rcCtrlData );
  ErrVal      xAddBaseLayerResidual           ( ControlData&                  rcControlData,
                                                IntFrame*                     pcFrame );
//TMM_EC 
  ErrVal      xInitBaseLayer                  ( ControlData&                  rcControlData, SliceHeader *&rcSliceHeaderBase);
  
  //===== decode pictures / subbands =====
  ErrVal      xDecodeBaseRepresentation       ( SliceHeader*&                 rpcSliceHeader,
                                                PicBuffer*&                   rpcPicBuffer,
                                                PicBufferList&                rcOutputList,
                                                PicBufferList&                rcUnusedList,
                                                Bool                          bReconstructionLayer );
  ErrVal      xDecodeFGSRefinement            ( SliceHeader*&                 rpcSliceHeader );
  ErrVal      xReconstructLastFGS             ( Bool                          bHighestLayer );

  ErrVal      xMotionCompensation             ( IntFrame*                     pcMCFrame,
                                                RefFrameList&                 rcRefFrameList0,
                                                RefFrameList&                 rcRefFrameList1,
                                                MbDataCtrl*                   pcMbDataCtrl,
                                                SliceHeader&                  rcSH );
  ErrVal      xFixMCPrediction                ( IntFrame*                     pcMCFrame,
                                                ControlData&                  rcControlData );
                                                                             

  Bool isPictureDecComplete(SliceHeader* rpcSliceHeader);
  const Bool isNewPictureStart(SliceHeader* rpcSliceHeader);
  ErrVal InitWhenNewPictureStart(SliceHeader* pcSliceHeader, MbDataCtrl*   pcMbDataCtrl);

protected:
  //----- references -----
  H264AVCDecoder*     m_pcH264AVCDecoder;
  SliceReader*        m_pcSliceReader;
  SliceDecoder*       m_pcSliceDecoder;
  RQFGSDecoder*       m_pcRQFGSDecoder;
  NalUnitParser*      m_pcNalUnitParser;
  ControlMngIf*       m_pcControlMng;
  LoopFilter*         m_pcLoopFilter;
  HeaderSymbolReadIf* m_pcHeaderSymbolReadIf;
  ParameterSetMng*    m_pcParameterSetMng;
  PocCalculator*      m_pcPocCalculator;
  YuvBufferCtrl*      m_pcYuvFullPelBufferCtrl;
  DecodedPicBuffer*   m_pcDecodedPictureBuffer;
  MotionCompensation* m_pcMotionCompensation;
  QuarterPelFilter*   m_pcQuarterPelFilter;
  DownConvert         m_cDownConvert;


  //----- general parameters -----
  Bool                m_bInitDone;
  Bool                m_bCreateDone;
  Bool                m_bWaitForIdr;
  UInt                m_uiFrameWidthInMb;
  UInt                m_uiFrameHeightInMb;
  UInt                m_uiMbNumber;

  Bool                m_bActive;
  UInt                m_uiLayerId;
  

  //----- frame memories and control data -----
  IntFrame*           m_apcFrameTemp    [NUM_TMP_FRAMES];
  IntFrame*           m_pcResidual;
  IntFrame*           m_pcILPrediction;
  IntFrame*           m_pcPredSignal;
  IntFrame*           m_pcBaseLayerFrame;
  IntFrame*           m_pcBaseLayerResidual;
  MbDataCtrl*         m_pcBaseLayerCtrl;
  DPBUnit*            m_pcCurrDPBUnit;

  UInt                m_uiNumLayers[2];
  

  // TMM_ESS 
  ResizeParameters*   m_pcResizeParameter;
 
  // should this layer be decoded at all, and up to which FGS layer should be decoded
  Int                 m_uiQualityLevelForPrediction;
#if MULTIPLE_LOOP_DECODING
  Bool                m_bCompletelyDecodeLayer;
#endif
  
  Int m_iMbProcessed;
};

H264AVC_NAMESPACE_END


#endif // !defined(AFX_GOPDECODER_H__339878FC_BA98_4ABE_8530_E1676196576F__INCLUDED_)
