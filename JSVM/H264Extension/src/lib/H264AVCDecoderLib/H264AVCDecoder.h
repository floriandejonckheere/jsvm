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





#if !defined(AFX_H264AVCDECODER_H__FBF0345F_A5E5_4D18_8BEC_4A68790901F7__INCLUDED_)
#define AFX_H264AVCDECODER_H__FBF0345F_A5E5_4D18_8BEC_4A68790901F7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "GOPDecoder.h"
#include "H264AVCCommonLib/Sei.h" 


H264AVC_NAMESPACE_BEGIN

class SliceReader;
class SliceDecoder;
class FrameMng;
class PocCalculator;
class LoopFilter;
class HeaderSymbolReadIf;
class ParameterSetMng;
class NalUnitParser;
class ControlMngIf;
class RQFGSDecoder;


class H264AVCDECODERLIB_API H264AVCDecoder
{ 
protected:
	H264AVCDecoder         ();
  virtual ~H264AVCDecoder();

public:
  static  ErrVal create ( H264AVCDecoder*& rpcH264AVCDecoder );
  ErrVal destroy();
  ErrVal init   ( MCTFDecoder*        apcMCTFDecoder[MAX_LAYERS],
                  SliceReader*        pcSliceReader,
                  SliceDecoder*       pcSliceDecoder,
                  RQFGSDecoder*       pcRQFGSDecoder,
                  FrameMng*           pcFrameMng,
                  NalUnitParser*      pcNalUnitParser,
                  ControlMngIf*       pcControlMng,
                  LoopFilter*         pcLoopFilter,
                  HeaderSymbolReadIf* pcHeaderSymbolReadIf,
                  ParameterSetMng*    pcParameterSetMng,
                  PocCalculator*      pcPocCalculator,
                  MotionCompensation* pcMotionCompensation );
  ErrVal uninit ();

  ErrVal  initPacket( BinDataAccessor*  pcBinDataAccessor,
	                    UInt&             ruiNalUnitType,
	                    UInt&             ruiMbX,
	                    UInt&             ruiMbY,
	                    UInt&             ruiSize
						//,UInt&				ruiNonRequiredPic  //NonRequired JVT-Q066
                        //JVT-P031
	                      ,Bool              bPreParseHeader //FRAG_FIX
		                  , Bool			bConcatenated //FRAG_FIX_3
                        ,Bool&             rbStartDecoding,
                        UInt&             ruiStartPos,
                        UInt&             ruiEndPos,
                        Bool&             bFragmented,
                        Bool&             bDiscardable
                        //~JVT-P031
                        ); 
  //JVT-P031
  ErrVal  initPacket( BinDataAccessor*  pcBinDataAccessor);
  Void    getDecodedResolution(UInt &uiLayerId);
  UInt    getNumOfNALInAU() {return m_uiNumOfNALInAU;}
  Void    decreaseNumOfNALInAU(){ m_uiNumOfNALInAU--;}
  Void    setDependencyInitialized(Bool b) { m_bDependencyInitialized = b;}
  Void    initNumberOfFragment();
  //~JVT-P031
  ErrVal  process   ( PicBuffer*        pcPicBuffer,
                      PicBufferList&    rcPicBufferOutputList,
                      PicBufferList&    rcPicBufferUnusedList,
                      PicBufferList&    rcPicBufferReleaseList );


  ErrVal  getBaseLayerData              ( IntFrame*&      pcFrame,
                                          IntFrame*&      pcResidual,
                                          MbDataCtrl*&    pcMbDataCtrl,
                                          Bool&           rbConstrainedIPred,
                                          Bool&           rbSpatialScalability,
                                          UInt            uiLayerId,
                                          UInt            uiBaseLayerId,
                                          Int             iPoc );
  ErrVal  getBaseLayerPWTable          ( SliceHeader::PredWeightTable*& rpcPredWeightTable,
                                         UInt                           uiBaseLayerId,
                                         ListIdx                        eListIdx,
                                         Int                            iPoc );

  Void    setBaseAVCCompatible        ( Bool                        bAVCCompatible )    { m_bBaseLayerIsAVCCompatible = bAVCCompatible; }
  Void    setReconstructionLayerId    ( UInt                        uiLayerId )         { m_uiRecLayerId = uiLayerId; }
  Void    setVeryFirstSPS             ( const SequenceParameterSet* pcSPS )             { m_pcVeryFirstSPS = pcSPS; }

  ErrVal  calculatePoc                ( NalUnitType       eNalUnitType,
                                        SliceHeader&      rcSliceHeader,
                                        Int&              slicePoc  );
  ErrVal  checkSliceLayerDependency   ( BinDataAccessor*  pcBinDataAccessor,
                                        Bool&             bFinishChecking );
//	TMM_EC {{
	Bool		checkSEIForErrorConceal();
  ErrVal  checkSliceGap   ( BinDataAccessor*  pcBinDataAccessor,
                            MyList<BinData*>&	cVirtualSliceList );
	ErrVal	setec( UInt uiErrorConceal) { m_eErrorConceal = (ERROR_CONCEAL)(EC_NONE + uiErrorConceal); if ( m_eErrorConceal == 0) m_bNotSupport = true; return	Err::m_nOK;}
	UInt	m_uiNextFrameNum;
	UInt	m_uiNextLayerId;
	UInt	m_uiNextPoc;
	UInt	m_uiNextTempLevel;
	UInt	*m_pauiPocInGOP[MAX_LAYERS];
	UInt	*m_pauiFrameNumInGOP[MAX_LAYERS];
	UInt	*m_pauiTempLevelInGOP[MAX_LAYERS];
	UInt	m_uiDecompositionStages[MAX_LAYERS];
	UInt	m_uiNumLayers;
	UInt	m_uiFrameIdx[MAX_LAYERS];
	ERROR_CONCEAL	m_eErrorConceal;
	UInt	m_uiDefNumLayers;
	UInt	m_uiDefDecompositionStages[MAX_LAYERS];
	UInt	m_uiMaxDecompositionStages;
	UInt	m_uiMaxGopSize;
	UInt	m_uiGopSize[MAX_LAYERS];
	Bool	m_bNotSupport;
	UInt	m_uiMaxLayerId;
//  TMM_EC }}
  Void    setQualityLevelForPrediction( UInt ui ) { m_uiQualityLevelForPrediction = ui; }
#if MULTIPLE_LOOP_DECODING
  Void    setCompletelyDecodeLayer    ( Bool b )  { m_bCompletelyDecodeLayer = b; }
#endif

  UInt isNonRequiredPic()						  { return m_uiNonRequiredPic;  } //NonRequired JVT-Q066
  Bool isRedundantPic()             { return m_bRedundantPic; }  // JVT-Q054 Red. Picture
  ErrVal  checkRedundantPic();  // JVT-Q054 Red. Picture

protected:

  ErrVal  xInitSlice                ( SliceHeader*    pcSliceHeader );
  ErrVal  xStartSlice               ( Bool& bPreParseHeader, Bool& bLastFragment, Bool& bDiscardable); //FRAG_FIX //TMM_EC
  // TMM_EC {{
  ErrVal  xProcessSliceVirtual      ( SliceHeader&    rcSH,
	                                    SliceHeader* pcPrevSH,
									                    PicBuffer* &    rpcPicBuffer);
  // TMM_EC }}
  ErrVal  xProcessSlice             ( SliceHeader&    rcSH,
                                      SliceHeader*    pcPrevSH,
                                      PicBuffer*&     rpcPicBuffer );
  ErrVal  xReconstructLastFGS       ();
  ErrVal  xDecodeFGSRefinement      ( SliceHeader*&   rpcSliceHeader,
                                      PicBuffer*&     rpcPicBuffer );

  ErrVal  xZeroIntraMacroblocks     ( IntFrame*       pcFrame,
                                      MbDataCtrl*     pcMbDataCtrl,
                                      SliceHeader*    pcSliceHeader );

  ErrVal  setDiffPrdRefLists        ( RefFrameList&               diffPrdRefList,
                                      YuvBufferCtrl*              pcYuvFullPelBufferCtrl);
  ErrVal  freeDiffPrdRefLists       ( RefFrameList& diffPrdRefList);

protected:
  SliceReader*                  m_pcSliceReader;
  SliceDecoder*                 m_pcSliceDecoder;
  FrameMng*                     m_pcFrameMng;
  NalUnitParser*                m_pcNalUnitParser;
  ControlMngIf*                 m_pcControlMng;
  LoopFilter*                   m_pcLoopFilter;
  HeaderSymbolReadIf*           m_pcHeaderSymbolReadIf;
  ParameterSetMng*              m_pcParameterSetMng;
  PocCalculator*                m_pcPocCalculator;
  SliceHeader*                  m_pcSliceHeader;
  SliceHeader*                  m_pcPrevSliceHeader;
  SliceHeader*                  m_pcSliceHeader_backup; //JVT-Q054 Red. Picture
  Bool                          m_bFirstSliceHeaderBackup;  //JVT-Q054 Red. Picture
  Bool                          m_bRedundantPic;  // JVT-Q054 Red. Picture
  Bool                          m_bInitDone;
  Bool                          m_bLastFrame;
  Bool                          m_bFrameDone;
  MotionCompensation*           m_pcMotionCompensation;

  MCTFDecoder*                  m_apcMCTFDecoder[MAX_LAYERS];
  RQFGSDecoder*                 m_pcRQFGSDecoder;
  PicBuffer*                    m_pcFGSPicBuffer;

  Bool                          m_bEnhancementLayer;
  Bool                          m_bActive;
  Bool                          m_bReconstruct;
  Bool                          m_bBaseLayerIsAVCCompatible;
	Bool                          m_bNewSPS;
  UInt                          m_uiRecLayerId;
  UInt                          m_uiLastLayerId;
  const SequenceParameterSet*   m_pcVeryFirstSPS;
  SliceHeader*                  m_pcVeryFirstSliceHeader;

  Bool                          m_bCheckNextSlice;
  Bool                          m_bDependencyInitialized;

  Int                           m_iLastPocChecked;
  Int                           m_iFirstSlicePoc;

  Int                           m_iFirstLayerIdx;
  Int                           m_iLastLayerIdx;
  Bool                          m_bBaseLayerAvcCompliant;

  Int                           m_auiBaseLayerId[MAX_LAYERS];
  Int                           m_auiBaseQualityLevel[MAX_LAYERS];

  // should this layer be decoded at all, and up to which FGS layer should be decoded
  UInt                          m_uiQualityLevelForPrediction;
#if MULTIPLE_LOOP_DECODING
  Bool                          m_bCompletelyDecodeLayer;
  Bool                          m_abCompletlyDecodeBaseLayer[MAX_LAYERS];
#endif

  Bool                          m_bFGSCodingMode;
  UInt                          m_uiGroupingSize;
  UInt                          m_uiPosVect[16];

  SEI::NonRequiredSei*			m_pcNonRequiredSei;
  UInt							m_uiNonRequiredSeiReadFlag;
	UInt							m_uiNonRequiredSeiRead;
	UInt							m_uiNonRequiredPic;	//NonRequired JVT-Q066	
  UInt							m_uiPrevPicLayer;
  UInt							m_uiCurrPicLayer;
  //JVT-P031
  UInt                          m_uiFirstFragmentPPSId;
  UInt                          m_uiFirstFragmentNumMbsInSlice;
  Bool                          m_bFirstFragmentFGSCompSep;
  UInt                          m_uiLastFragOrder;
  UInt                          m_uiNumberOfFragment[MAX_LAYERS];
  UInt                          m_uiNumberOfSPS;
  UInt                          m_uiSPSId[MAX_LAYERS];
  UInt                          m_uiDecodedLayer;
  UInt                          m_uiNumOfNALInAU;
  SliceHeader*                  m_pcSliceHeaderStored;
  Int                           m_iPrevPoc;
  //~JVT-P031

  SliceHeader::PredWeightTable  m_acLastPredWeightTable[2];
};

H264AVC_NAMESPACE_END

#endif // !defined(AFX_H264AVCDECODER_H__FBF0345F_A5E5_4D18_8BEC_4A68790901F7__INCLUDED_)
