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




#define _ABT_FLAG_IN_SLICE_HEADER_
#include "H264AVCDecoderLib.h"

#include "ControlMngH264AVCDecoder.h"
#include "H264AVCCommonLib/TraceFile.h"

#include "MbParser.h"
#include "H264AVCCommonLib/FrameMng.h"
#include "H264AVCCommonLib/ParameterSetMng.h"

#include "SliceReader.h"
#include "DecError.h"

#include "H264AVCCommonLib/CFMO.h"

H264AVC_NAMESPACE_BEGIN


SliceReader::SliceReader():
  m_pcHeaderReadIf( NULL ),
  m_pcParameterSetMng(NULL),
  m_pcMbParser( NULL ),
  m_pcControlMng( NULL ),
  m_bInitDone( false)
{
}


SliceReader::~SliceReader()
{
}


ErrVal SliceReader::create( SliceReader*& rpcSliceReader )
{
  rpcSliceReader = new SliceReader;

  ROT( NULL == rpcSliceReader );

  return Err::m_nOK;
}


ErrVal SliceReader::destroy()
{
  delete this;
  return Err::m_nOK;
}


ErrVal SliceReader::init( HeaderSymbolReadIf* pcHeaderSymbolReadIf,
                          ParameterSetMng* pcParameterSetMng,
                          MbParser* pcMbParser,
                          ControlMngIf* pcControlMng )
{
  ROT( m_bInitDone );
  ROT( NULL == pcHeaderSymbolReadIf );
  ROT( NULL == pcMbParser );
  ROT( NULL == pcControlMng );


  m_pcParameterSetMng = pcParameterSetMng;
  m_pcControlMng      = pcControlMng;
  m_pcHeaderReadIf    = pcHeaderSymbolReadIf;
  m_pcMbParser        = pcMbParser;

  m_bInitDone = true;
  return Err::m_nOK;
}


ErrVal SliceReader::uninit()
{
  ROF( m_bInitDone );
  m_pcHeaderReadIf = NULL;
  m_pcMbParser = NULL;
  m_bInitDone = false;

  return Err::m_nOK;
}


// JVT-S054 (2) (REPLACE)
//ErrVal SliceReader::process( const SliceHeader& rcSH, UInt& ruiMbRead )
ErrVal SliceReader::process( SliceHeader& rcSH, UInt& ruiMbRead )
{
  int sgId = rcSH.getFMO()->getSliceGroupId(rcSH.getFirstMbInSlice());  
  int pocOrder = rcSH.getPicOrderCntLsb();
  rcSH.getFMO()->setCodedSG(sgId, pocOrder);  

  ROF( m_bInitDone );

  //====== initialization ======
  UInt  uiMbAddress       = rcSH.getFirstMbInSlice();
  Bool  bEndOfSlice       = false;

  //===== loop over macroblocks =====
  for(  ruiMbRead = 0; !bEndOfSlice; ruiMbRead++ ) //--ICU/ETRI FMO Implementation
  {
    DTRACE_NEWMB( uiMbAddress );
    MbDataAccess* pcMbDataAccess;

    RNOK( m_pcControlMng->initMbForParsing( pcMbDataAccess, rcSH.getMbIndexFromAddress( uiMbAddress ) ) );
    if( rcSH.isMbAff() && uiMbAddress % 2 == 0 )
    {
      pcMbDataAccess->setFieldMode( pcMbDataAccess->getDefaultFieldFlag() );
    }
    pcMbDataAccess->getMbData().deactivateMotionRefinement();

    DECRNOK( m_pcMbParser->process( *pcMbDataAccess, bEndOfSlice) );

    // JVT-S054 (2) (ADD)
    rcSH.setLastMbInSlice(uiMbAddress);
    //--ICU/ETRI FMO Implementation
    uiMbAddress  = rcSH.getFMO()->getNextMBNr(uiMbAddress ); 

  }
  // JVT-S054 (2) (ADD)
  rcSH.setNumMbsInSlice(ruiMbRead);

  return Err::m_nOK;
}







ErrVal  SliceReader::read( SliceHeader&   rcSH,
                           MbDataCtrl*    pcMbDataCtrl,
                           Int            iSpatialScalabilityType,
                           UInt           uiMbInRow,
                           UInt&          ruiMbRead )
{
  ROF( m_bInitDone );

	//====== initialization ======
  UInt  uiMbAddress   = rcSH.getFirstMbInSlice();
  UInt  uiNumMbInPic  = rcSH.getSPS().getMbInFrame();
  Bool  bEndOfSlice   = false;

  RNOK( pcMbDataCtrl->initSlice( rcSH, PARSE_PROCESS, true, NULL ) );

  //===== loop over macroblocks =====
  for( ruiMbRead = 0; !bEndOfSlice; ruiMbRead++ ) //--ICU/ETRI FMO Implementation  
  {
    DTRACE_NEWMB( uiMbAddress );

    MbDataAccess* pcMbDataAccess      = 0;
    UInt          uiMbY, uiMbX;

    rcSH.getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddress );
    Bool          bCropWindowFlag     = pcMbDataCtrl->getMbData( uiMbX, uiMbY ).getInCropWindowFlag();

    RNOK( pcMbDataCtrl        ->initMb    ( pcMbDataAccess,     uiMbY, uiMbX ) );
    pcMbDataAccess->getMbData().deactivateMotionRefinement();
    pcMbDataAccess->getMbData().setInCropWindowFlag( bCropWindowFlag );


//TMM_INTERLACE{
	if( rcSH.isMbAff() && uiMbAddress % 2 == 0 )
    {
      pcMbDataAccess->setFieldMode( pcMbDataAccess->getDefaultFieldFlag() );
    }
//TMM_INTERLACE}

    RNOK( m_pcMbParser        ->read      ( *pcMbDataAccess,
                                            iSpatialScalabilityType,
                                            bEndOfSlice  ) );

    if( bEndOfSlice && ruiMbRead < uiNumMbInPic - 1 )
    {
      ROF( rcSH.getMGSCoeffStart() == rcSH.getMGSCoeffStop() );
      bEndOfSlice = false;
    }

	  if(ruiMbRead == uiNumMbInPic) bEndOfSlice = true; //FRAG_FIX
    // JVT-S054 (2) (ADD)
    if( bEndOfSlice )
    {
      rcSH.setLastMbInSlice(uiMbAddress);
    }
    //--ICU/ETRI FMO Implementation
    uiMbAddress  = rcSH.getFMO()->getNextMBNr(uiMbAddress ); 

  }

  // JVT-S054 (2) (ADD)
  rcSH.setNumMbsInSlice(ruiMbRead);

  //ICU/ETRI FGS FMO
  int sgId = rcSH.getFMO()->getSliceGroupId(rcSH.getFirstMbInSlice());  
  int pocOrder = rcSH.getPicOrderCntLsb();

  rcSH.getFMO()->setCodedSG(sgId, pocOrder);  

  //--ICU/ETRI FMO Implementation
  // JVT-S054 (REMOVE)
  //ROF( ruiMbRead == rcSH.getNumMbInSlice());

  return Err::m_nOK;
}




//TMM_EC {{
ErrVal  SliceReader::readVirtual( SliceHeader&   rcSH,
																	MbDataCtrl*    pcMbDataCtrl,
																	MbDataCtrl*    pcMbDataCtrlRef,
																	MbDataCtrl*    pcMbDataCtrlBase,
                                  MbDataCtrl*    pcMbDataCtrlBaseField, 
																	Int            iSpatialScalabilityType,
																	UInt           uiMbInRow,
																	UInt&          ruiMbRead,
																	ERROR_CONCEAL			m_eErrorConceal)
{
  ROF( m_bInitDone );

  UInt  uiMbAddress   = rcSH.getFirstMbInSlice();
  UInt  uiNumMbInPic  = rcSH.getSPS().getMbInFrame();
  Bool  bEndOfSlice   = false;

  RNOK( pcMbDataCtrl->initSlice( rcSH, PARSE_PROCESS, true, NULL ) );

  //===== loop over macroblocks =====
	for( ruiMbRead = 0; ruiMbRead < uiNumMbInPic; uiMbAddress++, ruiMbRead++  )
  {
    DTRACE_NEWMB( uiMbAddress );

    MbDataAccess* pcMbDataAccess      = 0;
    MbDataAccess* pcMbDataAccessBase  = 0;
    MbDataAccess* pcMbDataAccessBaseField  = 0;// TMM_INTERLACE
    UInt          uiMbY, uiMbX;

    rcSH.getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddress );
		if ( rcSH.getTrueSlice() || rcSH.m_eErrorConceal != EC_TEMPORAL_DIRECT)
		{
			RNOK( pcMbDataCtrl        ->initMb    ( pcMbDataAccess,     uiMbY, uiMbX ) );
		}
		else
		{
			RNOK( pcMbDataCtrl        ->initMbTDEnhance( pcMbDataAccess, pcMbDataCtrl, pcMbDataCtrlRef, uiMbY, uiMbX ) );
		}
    pcMbDataAccess->getMbData().deactivateMotionRefinement();
		if  ( pcMbDataCtrlBase )
    {
      RNOK( pcMbDataCtrlBase  ->initMb    ( pcMbDataAccessBase, uiMbY, uiMbX ) );
    }
    if  ( pcMbDataCtrlBaseField )// TMM_INTERLACE
    {
      RNOK( pcMbDataCtrlBaseField  ->initMb    ( pcMbDataAccessBaseField, uiMbY, uiMbX ) );
    }
		RNOK( m_pcMbParser        ->readVirtual( *pcMbDataAccess,
                                            pcMbDataAccessBase,
                                            pcMbDataAccessBaseField,// TMM_INTERLACE
                                            iSpatialScalabilityType,
                                            bEndOfSlice,
																						m_eErrorConceal) );
  }
  ROF( ruiMbRead == uiNumMbInPic );

  return Err::m_nOK;
}
//TMM_EC }}
//TMM_EC {{
ErrVal 
SliceReader::readSliceHeaderVirtual(	NalUnitType   eNalUnitType,
																			SliceHeader	*rpcVeryFirstSliceHeader,
																			UInt	uiDecompositionStages,
																			UInt  uiMaxDecompositionStages,
																			UInt	uiGopSize,
																			UInt	uiMaxGopSize,
																			UInt	uiFrameNum,
																			UInt	uiPoc,
																			UInt	uiTemporalLevel,
                                      UInt  uiLayerID      ,
																			SliceHeader*& rpcSH)
{
  SequenceParameterSet* pcSPS;
  PictureParameterSet*  pcPPS;

	UInt	uiPPSId	=	rpcVeryFirstSliceHeader->getPPS().getPicParameterSetId();

  RNOK( m_pcParameterSetMng ->get    ( pcPPS, uiPPSId) );
  RNOK( m_pcParameterSetMng ->get    ( pcSPS, pcPPS->getSeqParameterSetId() ) );

  rpcSH = new SliceHeader ( *pcSPS, *pcPPS );
  ROF( rpcSH );

  rpcSH->setNalUnitType   ( eNalUnitType    );

	UInt m_uiNextLayerId = 0;
	m_uiNextLayerId		=	uiLayerID;

//  if(eNalUnitType==NAL_UNIT_CODED_SLICE_SCALABLE )
  if(eNalUnitType==NAL_UNIT_CODED_SLICE_SCALABLE && m_uiNextLayerId != 0)
  {
		rpcSH->setLayerId       ( 1       );
    rpcSH->setBaseLayerId(MSYS_UINT_MAX); // will be modified later
  }
  else
  {
    rpcSH->setLayerId(0);
		rpcSH->setBaseLayerId   ( MSYS_UINT_MAX       );
  }
  rpcSH->setTemporalLevel ( uiTemporalLevel );
  rpcSH->setQualityLevel  ( 0       );
  rpcSH->setFirstMbInSlice( 0       );

	UInt	uiMaxPocLsb		=	1 << rpcSH->getSPS().getLog2MaxPicOrderCntLsb();
	rpcSH->setFrameNum( uiFrameNum);
	rpcSH->setPicOrderCntLsb( uiPoc % uiMaxPocLsb);
// TMM_INTERLACE{    
// rpcSH->setPoc( uiPoc);
	if( rpcSH->getPicType() == FRAME )
	{
		rpcSH->setTopFieldPoc( uiPoc );
		rpcSH->setBotFieldPoc( uiPoc );
	}
	else if ( rpcSH->getPicType() == TOP_FIELD )
	{
		rpcSH->setTopFieldPoc( uiPoc );
	}
	else
	{
		rpcSH->setBotFieldPoc( uiPoc );
	}
// TMM_INTERLACE}
  
  rpcSH->setAdaptivePredictionFlag(1);
	rpcSH->setDirectSpatialMvPredFlag(false);
	rpcSH->setNumRefIdxActiveOverrideFlag( true);
	rpcSH->setNumRefIdxActive( LIST_0, 1);

	if ( rpcSH->getPicOrderCntLsb() % uiMaxGopSize == 0 || (uiGopSize - ((rpcSH->getPicOrderCntLsb() % uiMaxGopSize) >> (uiMaxDecompositionStages-uiDecompositionStages)) < (unsigned)( 1<<(uiDecompositionStages-uiTemporalLevel) ) ) )
	{
		rpcSH->setSliceType     ( P_SLICE );
		if( rpcSH->getPicOrderCntLsb() % (1<<(uiMaxDecompositionStages-uiDecompositionStages+1)) == 0)
			rpcSH->setNalRefIdc   ( NAL_REF_IDC_PRIORITY_HIGHEST);
		else
			rpcSH->setNalRefIdc     ( NAL_REF_IDC_PRIORITY_HIGH);
	}
  else
	{
		rpcSH->setSliceType     ( B_SLICE );
		if( rpcSH->getPicOrderCntLsb() % (1<<(uiMaxDecompositionStages-uiDecompositionStages+1)) == 0)
			rpcSH->setNalRefIdc   ( NAL_REF_IDC_PRIORITY_LOW);
		else
			rpcSH->setNalRefIdc     ( NAL_REF_IDC_PRIORITY_LOWEST);
		rpcSH->setNumRefIdxActive( LIST_1, 1);
	}
  //if(eNalUnitType==NAL_UNIT_CODED_SLICE||)
//key picture MMCO for base and enhancement layer
  {
		if(rpcSH->getPoc() % uiMaxGopSize == 0  || (uiGopSize - ((rpcSH->getPicOrderCntLsb() % uiMaxGopSize) >> (uiMaxDecompositionStages-uiDecompositionStages)) < (unsigned)( 1<<(uiDecompositionStages-uiTemporalLevel) ) ) )
	  {
			UInt index=rpcSH->getPoc() / uiMaxGopSize;
		  if( index>0 )rpcSH->setAdaptiveRefPicBufferingFlag(true);
      else    	   rpcSH->setAdaptiveRefPicBufferingFlag(false);

		  if(index>1)
			{
        Bool bNumber2Gop= index >2 ? true : false;
				rpcSH->setDefualtMmcoBuffer(uiDecompositionStages,bNumber2Gop);
		  }
  	  rpcSH->setSliceType(P_SLICE);
  	  rpcSH->setNalRefIdc(NAL_REF_IDC_PRIORITY_HIGHEST);
	  }

		if(rpcSH->getPoc() % uiMaxGopSize == 0  || (uiGopSize - ((rpcSH->getPicOrderCntLsb() % uiMaxGopSize) >> (uiMaxDecompositionStages-uiDecompositionStages)) < (unsigned)( 1<<(uiDecompositionStages-uiTemporalLevel) ) ) )
	  {
			UInt index=rpcSH->getPoc() / uiMaxGopSize;
		  if( index>1 )
		  {
				rpcSH->getRplrBuffer(LIST_0).setRefPicListReorderingFlag(true);
				rpcSH->getRplrBuffer(LIST_0).clear();
//TMM_EC		
				if ( uiGopSize == 1)
					rpcSH->getRplrBuffer(LIST_0).set(0,Rplr(RPLR_NEG,0));
				else
					rpcSH->getRplrBuffer(LIST_0).set(0,Rplr(RPLR_NEG,uiGopSize/2-1));
		  }
		  else 
		  {
			  rpcSH->getRplrBuffer(LIST_0).setRefPicListReorderingFlag(false);
		  }
  	
	  }
  }

	//weighted prediction
	RNOK( rpcSH->getPredWeightTable(LIST_0).init( 64 ) );
  RNOK( rpcSH->getPredWeightTable(LIST_1).init( 64 ) );
  
  return Err::m_nOK;
}
//TMM_EC }}

ErrVal
SliceReader::readSliceHeader( NalUnitParser* pcNalUnitParser,
                              SliceHeader*& rpcSH )
{
  NalUnitType   eNalUnitType=pcNalUnitParser->getNalUnitType();
  NalRefIdc     eNalRefIdc=pcNalUnitParser->getNalRefIdc();
  UInt          uiLayerId=pcNalUnitParser->getLayerId();
  UInt          uiTemporalLevel=pcNalUnitParser->getTemporalLevel();
  UInt          uiQualityLevel=pcNalUnitParser->getQualityLevel();

  Bool          bUseBasePredFlag=pcNalUnitParser->getUseBasePredFlag();

  Bool                  bScalable         = ( eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE ||
                                              eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE );
  UInt                  uiFirstMbInSlice;
  UInt                  uiSliceType;
  UInt                  uiPPSId;
  SequenceParameterSet* pcSPS;
  PictureParameterSet*  pcPPS;


  //===== read first parameters =====
  //JVT-P031

  //JVT-V088 LMI {
  UInt                uiTl0PicIdx;
  if( bScalable && pcNalUnitParser->getTl0PicIdxPresentFlag() ) 
  {
    RNOK(m_pcHeaderReadIf->getCode(uiTl0PicIdx, 8, "SH: tl0_pic_idx"));
  }
//JVT-V088 LMI }
  if(  uiLayerId == 0 && uiQualityLevel == 0 && 
    (eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE || eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE ) )
  {
    Bool eAdaptiveRefPicMarkingModeFlag = false;
    MmcoBuffer eMmcoBaseBuffer;

    RNOK( m_pcParameterSetMng ->get    ( pcPPS, m_uiPPSId_AVC ) );
    RNOK( m_pcParameterSetMng ->get    ( pcSPS, m_uiSPSId_AVC ) );

    rpcSH = new SliceHeader ( *pcSPS, *pcPPS );
    ROF( rpcSH );
    //JVT-V088 LMI
    if( pcNalUnitParser->getTl0PicIdxPresentFlag() ) 
      rpcSH->setTl0PicIdx  ( uiTl0PicIdx  );
    if( eNalRefIdc != 0)
    {
			 if( bUseBasePredFlag && eNalUnitType != NAL_UNIT_CODED_SLICE_IDR_SCALABLE)
			 {
				 RNOK(m_pcHeaderReadIf->getFlag( eAdaptiveRefPicMarkingModeFlag,			"DRPM: adaptive_ref_pic_marking_mode_flag"));
				 if( eAdaptiveRefPicMarkingModeFlag )
				 {  
					 RNOK( rpcSH->getMmcoBaseBuffer().read( m_pcHeaderReadIf ) );
				 }		
			 }
		 }
		 rpcSH->setNalUnitType   ( eNalUnitType    );
		 rpcSH->setNalRefIdc     ( eNalRefIdc      );
		 rpcSH->setLayerId       ( uiLayerId       );
		 rpcSH->setTemporalLevel ( uiTemporalLevel );
		 rpcSH->setQualityLevel  ( uiQualityLevel  );

     rpcSH->setLayerBaseFlag    ( pcNalUnitParser->getLayerBaseFlag()     );
     rpcSH->setDiscardableFlag  ( pcNalUnitParser->getDiscardableFlag()   );

		 rpcSH->setUseBaseRepresentationFlag( bUseBasePredFlag      );   
		 rpcSH->setAdaptiveRefPicMarkingFlag( eAdaptiveRefPicMarkingModeFlag ); 
		 rpcSH->setPicOrderCntLsb( m_uiPOC_AVC );

		 return Err::m_nOK;
  }
	else
  {
      RNOK( m_pcHeaderReadIf    ->getUvlc( uiFirstMbInSlice,  "SH: first_mb_in_slice" ) );
      RNOK( m_pcHeaderReadIf    ->getUvlc( uiSliceType,       "SH: slice_type" ) );
      ROF( uiSliceType == 0 || uiSliceType == 1 || uiSliceType == 2 ||
           uiSliceType == 5 || uiSliceType == 6 || uiSliceType == 7 );
      if( uiSliceType > 4 && ! bScalable )
      {
          uiSliceType -= 5;
      }
      RNOK( m_pcHeaderReadIf    ->getUvlc( uiPPSId,           "SH: pic_parameter_set_id" ) );
      RNOK( m_pcParameterSetMng ->get    ( pcPPS, uiPPSId) );
      RNOK( m_pcParameterSetMng ->get    ( pcSPS, pcPPS->getSeqParameterSetId() ) );
  }


  //===== create and initialize slice header =====
  rpcSH = new SliceHeader ( *pcSPS, *pcPPS );
  ROF( rpcSH );
  rpcSH->setNalUnitType   ( eNalUnitType    );
  rpcSH->setNalRefIdc     ( eNalRefIdc      );
  rpcSH->setLayerId       ( uiLayerId       );
  rpcSH->setTemporalLevel ( uiTemporalLevel );
  rpcSH->setQualityLevel  ( uiQualityLevel  );

  rpcSH->setLayerBaseFlag   ( pcNalUnitParser->getLayerBaseFlag()     );
  rpcSH->setUseBaseRepresentationFlag  ( bUseBasePredFlag                        );
  rpcSH->setDiscardableFlag ( pcNalUnitParser->getDiscardableFlag()     );

  //JVT-V088 LMI
  if( bScalable && pcNalUnitParser->getTl0PicIdxPresentFlag() ) 
    rpcSH->setTl0PicIdx  ( uiTl0PicIdx  );

  rpcSH->setFirstMbInSlice( uiFirstMbInSlice);
  rpcSH->setSliceType     ( SliceType( uiSliceType ) );
  
  //===== read remaining parameters =====
  RNOK( rpcSH->read( m_pcHeaderReadIf ) ); 

  if ( eNalUnitType == NAL_UNIT_CODED_SLICE ||
	   eNalUnitType == NAL_UNIT_CODED_SLICE_IDR )
  {
	  m_uiPPSId_AVC = uiPPSId;
	  m_uiSPSId_AVC = pcPPS->getSeqParameterSetId();
	  m_uiPOC_AVC = rpcSH->getPicOrderCntLsb();
  }
//JVT-S036 lsj end

  //--ICU/ETRI FMO Implementation 
  rpcSH->FMOInit();
  // JVT-S054 (2) (ADD) ->
  if (rpcSH->getNumMbsInSlice() != 0)
    rpcSH->setLastMbInSlice(rpcSH->getFMO()->getLastMbInSlice(uiFirstMbInSlice, rpcSH->getNumMbsInSlice()));
  else
  // JVT-S054 (2) (ADD) <-
    rpcSH->setLastMbInSlice(rpcSH->getFMO()->getLastMBInSliceGroup(rpcSH->getFMO()->getSliceGroupId(uiFirstMbInSlice)));
   
  if ( !rpcSH->getSPS().getFrameMbsOnlyFlag() )
  {
    if ( (!rpcSH->getFieldPicFlag()) && (rpcSH->getSPS().getMbAdaptiveFrameFieldFlag()) )
    {
      rpcSH->setFirstMbInSlice  ( uiFirstMbInSlice << 1 );
    }
   }

  return Err::m_nOK;
}


//JVT-S036 lsj start
ErrVal
SliceReader::readSliceHeaderSuffix( NalUnitType   eNalUnitType,
									NalRefIdc     eNalRefIdc,
									UInt		  uiLayerId,
									UInt		  uiQualityLevel,
                  Bool          bUseBasePredFlag,
									SliceHeader*  pcSliceHeader
								  )
{
  
  //===== read first parameters =====
  
  Bool eAdaptiveRefPicMarkingModeFlag = false;
 
  if( eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE || 
      eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE       )
  {
	  
	 if( uiLayerId == 0 && uiQualityLevel == 0 ) 
     {

		 if( eNalRefIdc != 0)
		 {
			 if( bUseBasePredFlag && eNalUnitType != NAL_UNIT_CODED_SLICE_IDR_SCALABLE )
			 {
				 RNOK(m_pcHeaderReadIf->getFlag( eAdaptiveRefPicMarkingModeFlag,			"DRPM: adaptive_ref_pic_marking_mode_flag"));
				 if( eAdaptiveRefPicMarkingModeFlag )
				 {  
					 RNOK( pcSliceHeader->getMmcoBaseBuffer().read( m_pcHeaderReadIf ) );
				 }		
			 }
		 }

		 pcSliceHeader->setUseBaseRepresentationFlag( bUseBasePredFlag      );   //bug-fix suffix shenqiu
		 pcSliceHeader->setAdaptiveRefPicMarkingFlag( eAdaptiveRefPicMarkingModeFlag );

		 return Err::m_nOK;
	 }
	 else
		 return Err::m_nERR;
  }
  else
	  return Err::m_nERR;
}
//prefix unit{{
ErrVal
SliceReader::readSliceHeaderPrefix( NalUnitType   eNalUnitType,
									NalRefIdc     eNalRefIdc,
									UInt		  uiLayerId,
									UInt		  uiQualityLevel,
									Bool          bUseBasePredFlag,
									SliceHeader*  pcSliceHeader
								  )
{
  
  //===== read first parameters =====
  
  Bool eAdaptiveRefPicMarkingModeFlag = false;
 
	if( uiLayerId == 0 && uiQualityLevel == 0 ) 
	{
		if( eNalRefIdc != 0)
		{
			if( bUseBasePredFlag && eNalUnitType != NAL_UNIT_CODED_SLICE_IDR_SCALABLE )
			{
				RNOK(m_pcHeaderReadIf->getFlag( eAdaptiveRefPicMarkingModeFlag,			"DRPM: adaptive_ref_pic_marking_mode_flag"));
				if( eAdaptiveRefPicMarkingModeFlag )
				{  
					RNOK( pcSliceHeader->getMmcoBaseBuffer().read( m_pcHeaderReadIf ) );
				}		
			}
		}

		pcSliceHeader->setUseBaseRepresentationFlag( bUseBasePredFlag      );   
		pcSliceHeader->setAdaptiveRefPicMarkingFlag( eAdaptiveRefPicMarkingModeFlag );

		return Err::m_nOK;
	}
	else
		return Err::m_nERR;
}
//prefix unit}}
//JVT-S036 lsj end

H264AVC_NAMESPACE_END
