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





#include "H264AVCDecoderLib.h"
#include "H264AVCDecoder.h"

#include "H264AVCCommonLib/FrameMng.h"
#include "H264AVCCommonLib/LoopFilter.h"
#include "H264AVCCommonLib/TraceFile.h"

#include "SliceReader.h"
#include "SliceDecoder.h"

#include "CreaterH264AVCDecoder.h"
#include "ControlMngH264AVCDecoder.h"
#include "FGSSubbandDecoder.h"

#include "GOPDecoder.h"

#include "H264AVCCommonLib/CFMO.h"


H264AVC_NAMESPACE_BEGIN


H264AVCDecoder::H264AVCDecoder()
: m_pcSliceReader                 ( NULL  )
, m_pcSliceDecoder                ( NULL  )
, m_pcFrameMng                    ( NULL  )
, m_pcNalUnitParser               ( NULL  )
, m_pcControlMng                  ( NULL  )
, m_pcLoopFilter                  ( NULL  )
, m_pcHeaderSymbolReadIf          ( NULL  )
, m_pcParameterSetMng             ( NULL  )
, m_pcPocCalculator               ( NULL  )
, m_pcSliceHeader                 ( NULL  )
, m_pcPrevSliceHeader             ( NULL  )
, m_pcTempSliceHeader			        ( NULL  )//EIDR bug-fix
, m_pcSliceHeader_backup          ( NULL  ) // JVT-Q054 Red. Picture
, m_bFirstSliceHeaderBackup       ( true  ) // JVT-Q054 Red. Picture
, m_bRedundantPic                 ( false ) // JVT-Q054 Red. Picture
, m_bInitDone                     ( false )
, m_bLastFrame                    ( false )
, m_bFrameDone                    ( true  )
, m_bEnhancementLayer             ( false )
, m_bSpatialScalability           ( false )
, m_bBaseLayerIsAVCCompatible     ( false )
, m_bNewSPS                       ( false )
, m_bReconstruct                  ( false )
, m_uiRecLayerId                  ( 0 )
, m_uiLastLayerId                 ( MSYS_UINT_MAX )
, m_pcVeryFirstSPS                ( NULL )
, m_bCheckNextSlice               ( false )
, m_iFirstLayerIdx                ( 0 )
, m_iLastLayerIdx                 ( 0 )
, m_iLastPocChecked               (-1 )
, m_iFirstSlicePoc                ( 0 )
, m_bBaseLayerAvcCompliant        ( false )
, m_bDependencyInitialized        ( false )
, m_uiQualityLevelForPrediction   ( 3 )
, m_pcNonRequiredSei			  ( NULL )
, m_uiNonRequiredSeiReadFlag	  ( 0 )
, m_uiNonRequiredSeiRead    	  ( 0 )
, m_uiPrevPicLayer				  ( 0 )
, m_uiCurrPicLayer				  ( 0 )
//JVT-P031
, m_uiFirstFragmentPPSId          ( 0 )
, m_uiFirstFragmentNumMbsInSlice  ( 0 )
, m_bFirstFragmentFGSCompSep      ( false )
, m_uiLastFragOrder               ( 0 )
, m_uiNumberOfSPS                 ( 0 )
, m_uiDecodedLayer                ( 0 )
, m_uiNumOfNALInAU                ( 0 )
//~JVT-P031
, m_bFGSCodingMode                ( false )
, m_uiGroupingSize                ( 1 )
, m_pcBaseLayerCtrlEL						  ( 0 )		// ICU/ETRI FGS_MOT_USE
, m_iCurNalSpatialLayer		(-1)
, m_iNextNalSpatialLayer	(-1)			   
, m_iCurNalPOC				(-1)
, m_iNextNalPOC				(-1)
, m_bCurNalIsEndOfPic		(false)
, m_iCurNalFirstMb			(-1)
, m_bFirstFGS				(true)
//JVT-T054{
, m_bLastNalInAU                  ( false )
, m_bFGSRefInAU                   ( false )
, m_bCGSSNRInAU                   ( false )
//JVT-T054}
, m_bBaseSVCActive                ( false ) //JVT-T054_FIX
, m_bLastFrameReconstructed       ( false ) //JVT-T054_FIX
{
  ::memset( m_apcMCTFDecoder, 0x00, MAX_LAYERS * sizeof( Void* ) );
  ::memset( m_uiPosVect,      0x00, 16         * sizeof( UInt  ) ); 
  m_pcVeryFirstSliceHeader = NULL;

  UInt uiLayer, uiQualityLevel;
  for(uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++)
  {
    for( uiQualityLevel = 0; uiQualityLevel < MAX_QUALITY_LEVELS; uiQualityLevel ++)
      m_uiNumberOfFragment[uiLayer][uiQualityLevel] = 0;
    
      m_uiSPSId[uiLayer] = 0;
  }

//	TMM	EC {{
	m_uiNextFrameNum	=	0;
	m_uiNextLayerId		=	0;
	m_uiNextPoc				=	0;
	m_uiNumLayers			=	1;
	m_uiMaxGopSize		=	16;
	m_uiMaxDecompositionStages	=	4;
	m_uiMaxLayerId	=	0;
  UInt ui;
	for ( ui=0; ui<MAX_LAYERS; ui++)
	{
		m_pauiPocInGOP         [ui]  =	NULL;
		m_pauiFrameNumInGOP    [ui]  =	NULL;
		m_pauiTempLevelInGOP   [ui]  =	NULL;
		m_uiDecompositionStages[ui]	 =	4;
		m_uiFrameIdx           [ui]  =	0;
		m_uiGopSize            [ui] 	 =	16;
	}
  
  m_eErrorConceal  =	EC_NONE;
	m_bNotSupport	=	false;
  m_baseMode = 1;
//  TMM_EC }}
}



H264AVCDecoder::~H264AVCDecoder()
{
}



ErrVal H264AVCDecoder::destroy()
{
  delete this;
  return Err::m_nOK;
}



ErrVal H264AVCDecoder::init( MCTFDecoder*        apcMCTFDecoder[MAX_LAYERS],
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
                             MotionCompensation* pcMotionCompensation )
{

  ROT( NULL == pcSliceReader );
  ROT( NULL == pcSliceDecoder );
  ROT( NULL == pcFrameMng );
  ROT( NULL == pcNalUnitParser );
  ROT( NULL == pcControlMng );
  ROT( NULL == pcLoopFilter );
  ROT( NULL == pcHeaderSymbolReadIf );
  ROT( NULL == pcParameterSetMng );
  ROT( NULL == pcPocCalculator );
  ROT( NULL == apcMCTFDecoder );
  ROT( NULL == pcRQFGSDecoder );

  m_pcSliceReader             = pcSliceReader;
  m_pcSliceDecoder            = pcSliceDecoder;
  m_pcRQFGSDecoder            = pcRQFGSDecoder;
  m_pcFrameMng                = pcFrameMng;
  m_pcNalUnitParser           = pcNalUnitParser;
  m_pcControlMng              = pcControlMng;
  m_pcLoopFilter              = pcLoopFilter;
  m_pcHeaderSymbolReadIf      = pcHeaderSymbolReadIf;
  m_pcParameterSetMng         = pcParameterSetMng;
  m_pcPocCalculator           = pcPocCalculator;
  m_pcFGSPicBuffer            = 0;
  m_bEnhancementLayer         = false;
  m_bSpatialScalability       = false;
  m_bBaseLayerIsAVCCompatible = false;
	m_bNewSPS                   = false;
  m_uiRecLayerId              = 0;
  m_uiLastLayerId             = MSYS_UINT_MAX;
  m_pcVeryFirstSPS            = 0;
  m_pcMotionCompensation      = pcMotionCompensation;

  m_bActive = false;

  for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
  {
    ROT( NULL == apcMCTFDecoder[uiLayer] );

    m_apcMCTFDecoder[uiLayer] = apcMCTFDecoder[uiLayer];
    for( UInt uiQualityLevel = 0; uiQualityLevel < MAX_QUALITY_LEVELS; uiQualityLevel ++)
      m_uiNumberOfFragment[uiLayer][uiQualityLevel] = 0;


//	TMM EC {{
		m_apcMCTFDecoder[uiLayer]->m_pcFrameMng	=	m_pcFrameMng;
		m_apcMCTFDecoder[uiLayer]->m_eErrorConceal	=	m_eErrorConceal;
//  TMM_EC }}
  }

  RNOK( m_acLastPredWeightTable[LIST_0].init( 64 ) );
  RNOK( m_acLastPredWeightTable[LIST_1].init( 64 ) );

  m_bInitDone = true;

  m_pcBaseLayerCtrlEL               = 0;

  return Err::m_nOK;
}



ErrVal H264AVCDecoder::uninit()
{
  RNOK( m_acLastPredWeightTable[LIST_0].uninit() );
  RNOK( m_acLastPredWeightTable[LIST_1].uninit() );

  m_pcSliceReader         = NULL;
  m_pcSliceDecoder        = NULL;
  m_pcFrameMng            = NULL;
  m_pcNalUnitParser       = NULL;
  m_pcControlMng          = NULL;
  m_pcLoopFilter          = NULL;
  m_pcHeaderSymbolReadIf  = NULL;
  m_pcParameterSetMng     = NULL;
  m_pcPocCalculator       = NULL;
  m_bInitDone             = false;
  m_bLastFrame            = false;
  m_bFrameDone            = true;
  m_pcMotionCompensation  = NULL;

  delete m_pcSliceHeader;
  delete m_pcPrevSliceHeader;
  delete m_pcTempSliceHeader;     //EIDR bug-fix 
  delete m_pcSliceHeader_backup;  // JVT-Q054 Red. Pic
  m_pcSliceHeader         = NULL;
  m_pcPrevSliceHeader     = NULL;
  m_pcTempSliceHeader	  = NULL;   //EIDR bug-fix
  m_pcSliceHeader_backup  = NULL; // JVT-Q054 Red. Pic

  m_pcSliceReader         = NULL;
  m_pcSliceDecoder        = NULL;
  m_pcFrameMng            = NULL;
  m_pcNalUnitParser       = NULL;
  m_pcControlMng          = NULL;
  m_pcLoopFilter          = NULL;
  m_pcHeaderSymbolReadIf  = NULL;
  m_pcParameterSetMng     = NULL;
  m_pcPocCalculator       = NULL;

	m_bNewSPS               = false;
	if( m_pcVeryFirstSliceHeader )
  delete m_pcVeryFirstSliceHeader;
  m_pcVeryFirstSliceHeader = NULL;

  for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
  {
    m_apcMCTFDecoder[uiLayer] = NULL;
  }

  m_bInitDone = false;
  
  return Err::m_nOK;
}



ErrVal H264AVCDecoder::create( H264AVCDecoder*& rpcH264AVCDecoder )
{
  rpcH264AVCDecoder = new H264AVCDecoder;
  ROT( NULL == rpcH264AVCDecoder );
  return Err::m_nOK;
}


ErrVal
H264AVCDecoder::calculatePoc( NalUnitType   eNalUnitType,
                              SliceHeader&  rcSliceHeader,
                              Int&          riSlicePoc )
{
  PocCalculator *pcLocalPocCalculator;

  if( eNalUnitType == NAL_UNIT_CODED_SLICE ||  eNalUnitType == NAL_UNIT_CODED_SLICE_IDR )
  {
    m_pcPocCalculator->copy( pcLocalPocCalculator );
  }
  else
  {
    m_apcMCTFDecoder[m_iFirstLayerIdx]->getPocCalculator()->copy( pcLocalPocCalculator );
  }

   //EIDR bug-fix {
	if(rcSliceHeader.getLayerId() == 0 )
	{
		rcSliceHeader.setInIDRAccess(rcSliceHeader.isIdrNalUnit());
	}
	else
	{
		rcSliceHeader.setInIDRAccess(m_pcTempSliceHeader?m_pcTempSliceHeader->getInIDRAccess():false);
	}
   //EIDR bug-fix }

 pcLocalPocCalculator->calculatePoc( rcSliceHeader );

	riSlicePoc = rcSliceHeader.getPoc();

  pcLocalPocCalculator->destroy();

  return Err::m_nOK;
}


ErrVal H264AVCDecoder::xInitParameters(SliceHeader* pcSliceHeader)
{
  m_uiFirstFragmentPPSId = pcSliceHeader->getPicParameterSetId();
  m_uiFirstFragmentNumMbsInSlice = pcSliceHeader->getNumMbsInSlice();
  m_bFirstFragmentFGSCompSep = pcSliceHeader->getFgsComponentSep();
  //activate corresponding SPS
  PictureParameterSet * rcPPS;
  m_pcParameterSetMng->get(rcPPS,m_uiFirstFragmentPPSId);
  UInt uiSPSId = rcPPS->getSeqParameterSetId();
  Bool bFound = false;
  for(UInt ui = 0; ui < m_uiNumberOfSPS; ui++)
  {
    if(m_uiSPSId[ui] == uiSPSId)
    {
      bFound = true;
      break;
    }
  } 
  if(!bFound)
  {
    m_uiSPSId[m_uiNumberOfSPS] = uiSPSId;
    m_uiNumberOfSPS++;
    SequenceParameterSet * rcSPS;
    m_pcParameterSetMng->get(rcSPS,uiSPSId);
    rcSPS->setLayerId(pcSliceHeader->getLayerId());
  }   
  return Err::m_nOK;
}

// not tested with multiple slices
ErrVal
H264AVCDecoder::checkSliceLayerDependency( BinDataAccessor*  pcBinDataAccessor,
                                           Bool&             bFinishChecking
										 )
{
  Bool bEos;
  NalUnitType eNalUnitType;
  SliceHeader* pcSliceHeader = NULL;
  Int slicePoc;
  bFinishChecking = false;

  ROT( NULL == pcBinDataAccessor );

  bEos = ( NULL == pcBinDataAccessor->data() ) || ( 0 == pcBinDataAccessor->size() );
  slicePoc = 0;

  if(m_uiNumOfNALInAU == 0)
  {
    //new AU: initialization
    m_bDependencyInitialized = false;
    m_bFGSRefInAU = false;
    //if(!bEos)                     MGS fix by Heiko Schwarz
    //  m_bCGSSNRInAU = false;      MSG fix by Heiko Schwarz

    initNumberOfFragment(); 
  }
  else
  {
    if(m_bDependencyInitialized)
    {
       bFinishChecking = true;
      return Err::m_nOK;
    }
  }
  if(bEos)
  {
	if (-1 == m_iNextNalSpatialLayer)
		m_bCurNalIsEndOfPic = true;
    m_bDependencyInitialized = true; //JVT-T054
    bFinishChecking = true;
    return Err::m_nOK;
  }

  if( ! bEos )
  {
    m_pcNalUnitParser->setCheckAllNALUs(true);
    UInt uiNumBytesRemoved;
    RNOK( m_pcNalUnitParser->initNalUnit( pcBinDataAccessor,  uiNumBytesRemoved ) );
    m_pcNalUnitParser->setCheckAllNALUs(false);

    eNalUnitType = m_pcNalUnitParser->getNalUnitType();
    if(!m_bDependencyInitialized)
      m_uiNumOfNALInAU++;//JVT-P031
    if( eNalUnitType != NAL_UNIT_CODED_SLICE        &&
        eNalUnitType != NAL_UNIT_CODED_SLICE_IDR      && 
        eNalUnitType != NAL_UNIT_CODED_SLICE_SCALABLE && 
        eNalUnitType != NAL_UNIT_CODED_SLICE_IDR_SCALABLE )
    {
      if(m_uiNumOfNALInAU > 1)
      {
        m_uiNumOfNALInAU--;
        m_bDependencyInitialized = true;
      }
       m_bCheckNextSlice = false;
       bFinishChecking = true;

       return Err::m_nOK;
    }
    else
    {
      // read the slice header
      //JVT-P031
      RNOK( m_pcSliceReader->readSliceHeader( m_pcNalUnitParser,
                                              pcSliceHeader,
                                              m_uiFirstFragmentNumMbsInSlice,
                                              m_bFirstFragmentFGSCompSep
											                        ) );
      //To detect if fragments have been lost or previously extracted
      if(pcSliceHeader->getFragmentedFlag())
      {
          m_uiNumberOfFragment[pcSliceHeader->getLayerId()][pcSliceHeader->getQualityLevel()] ++;
      }

      if(pcSliceHeader->getFragmentOrder() == 0)
      {
          xInitParameters(pcSliceHeader);
      }
      else
      {
        //if next fragments, skip
        if( pcSliceHeader != NULL )
            delete pcSliceHeader;
          
        return Err::m_nOK;      
      }
      //~JVT-P031

      // F slices are also ignored
      if( pcSliceHeader->getSliceType() == F_SLICE )
      {
        m_bFGSRefInAU = true;
        
        //manu.mathew@samsung : memory leak fix
        if( pcSliceHeader )
          delete pcSliceHeader;
        //--
        return Err::m_nOK;
      }
      m_bCGSSNRInAU = (m_bCGSSNRInAU || pcSliceHeader->getQualityLevel() != 0);

      calculatePoc( eNalUnitType, *pcSliceHeader, slicePoc );

	  // <-- ROI DECODE ICU/ETRI
	  // first NAL check
	  if (-1 == m_iCurNalSpatialLayer)
	  {
		  m_iCurNalSpatialLayer 	= m_pcNalUnitParser->getLayerId();
		  m_iCurNalPOC  			= slicePoc;
		  m_iCurNalFirstMb			= pcSliceHeader->getFirstMbInSlice();
		  
	  }
	  
	  // second NAL check
	  else if (-1 == m_iNextNalSpatialLayer)
	  {
		  m_iNextNalSpatialLayer	= m_pcNalUnitParser->getLayerId();
		  m_iNextNalPOC			= slicePoc;
		  
		  if (m_iCurNalSpatialLayer != m_iNextNalSpatialLayer)
			  m_bCurNalIsEndOfPic = true;
		  
		  if (m_iCurNalPOC != m_iNextNalPOC)
			  m_bCurNalIsEndOfPic = true;
		  
		  if( pcSliceHeader->getSliceType() == F_SLICE )
			  m_bCurNalIsEndOfPic = true;

		  Bool bNewFrame =false;
		  RNOK( pcSliceHeader->compareRedPic ( m_pcSliceHeader_backup, bNewFrame ) );
		  if(m_iCurNalFirstMb ==pcSliceHeader->getFirstMbInSlice())
			  m_bCurNalIsEndOfPic = true;	  
	  }
	  // --> ROI DECODE ICU/ETRI

      if( ! m_bCheckNextSlice )
      { 
      m_iFirstLayerIdx = m_pcNalUnitParser->getLayerId();
      m_iFirstSlicePoc = slicePoc;
      if( eNalUnitType == NAL_UNIT_CODED_SLICE ||  eNalUnitType == NAL_UNIT_CODED_SLICE_IDR )
      {
        m_bBaseLayerAvcCompliant = true;
        m_iFirstLayerIdx = m_pcNalUnitParser->getLayerId();
      }
      else
      {
          m_bBaseLayerAvcCompliant = false;
      }
      }

      if( slicePoc == m_iFirstSlicePoc )
      {
      m_iLastLayerIdx = m_pcNalUnitParser->getLayerId();
      if (m_iLastLayerIdx == 0)
      {
        m_auiBaseLayerId[m_iLastLayerIdx]      = MSYS_UINT_MAX;
        m_auiBaseQualityLevel[m_iLastLayerIdx] = 0;
      }
      else
      {
        m_auiBaseLayerId[m_iLastLayerIdx]      = pcSliceHeader->getBaseLayerId();
        m_auiBaseQualityLevel[m_iLastLayerIdx] = pcSliceHeader->getBaseQualityLevel();
      }
      }

      m_bCheckNextSlice = true;
    }
  }
  if( bEos || slicePoc != m_iFirstSlicePoc)  
  {
	// ROI DECODE ICU/ETRI
	if (-1 != m_iCurNalSpatialLayer && -1 != m_iNextNalSpatialLayer)
		bFinishChecking   = true;
    // setup the state information for the previous slices	
    m_bCheckNextSlice = false;
//JVT-T054{
    if(!bEos)
      decreaseNumOfNALInAU();
//JVT-T054}
    m_apcMCTFDecoder[m_iLastLayerIdx]->setQualityLevelForPrediction( 3 );
    if( m_iFirstLayerIdx < m_iLastLayerIdx )
    {
      // set the base layer dependency
      if( m_iFirstLayerIdx == 0 )
      {
        if( m_bBaseLayerAvcCompliant )
        {
          setQualityLevelForPrediction( m_auiBaseQualityLevel[1] );
        }
        else
        {
          m_apcMCTFDecoder[0]->setQualityLevelForPrediction( m_auiBaseQualityLevel[1] );
        }
      }

      for( Int iLayer = (m_iFirstLayerIdx == 0) ? 1 : m_iFirstLayerIdx; 
        iLayer <= m_iLastLayerIdx - 1; iLayer ++ )
      {
        m_apcMCTFDecoder[iLayer]->setQualityLevelForPrediction( m_auiBaseQualityLevel[iLayer + 1] );
      }
    }

    m_bDependencyInitialized = true;
  }

  if( pcSliceHeader != NULL )
 {
  delete m_pcTempSliceHeader;  //EIDR bug-fix
	m_pcTempSliceHeader = pcSliceHeader;//EIDR bug-fix
  }

  return Err::m_nOK;
}

//JVT-P031
Void H264AVCDecoder::initNumberOfFragment()
{
  UInt uiLayer, uiQualityLevel;
  for( uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer ++ )
  for( uiQualityLevel = 0; uiQualityLevel < MAX_QUALITY_LEVELS; uiQualityLevel ++)
  {
      m_uiNumberOfFragment[uiLayer][uiQualityLevel] = 0;
  }

}
Void H264AVCDecoder::getDecodedResolution(UInt &uiLayerId)
{
 UInt uiSPSId = 0;
 UInt uiX     = 0;
 UInt uiY     = 0;
 UInt uiMBX   = 0;
 UInt uiMBY   = 0;
 SequenceParameterSet *rcSPS;

 uiSPSId = 0;
 uiMBX = 0;
 uiMBY = 0;
 UInt uiSPS = 0;
 while(uiSPS < m_uiNumberOfSPS)
 {
   if(m_pcParameterSetMng->isValidSPS(uiSPSId))
   {
    m_pcParameterSetMng->get(rcSPS,uiSPSId);
    uiX = rcSPS->getFrameWidthInMbs();
    uiY = rcSPS->getFrameHeightInMbs();
    if(uiX >= uiMBX && uiY >= uiMBY) //FIX_FRAG_CAVLC
    {
       uiMBX = uiX;
       uiMBY = uiY;
       uiLayerId = rcSPS->getLayerId();
    }
    uiSPS++;
   }
   uiSPSId++;
 }

}
//~JVT-P031
//TMM_EC{{
Bool
H264AVCDecoder::checkSEIForErrorConceal()
{
	Bool	ret	=	true;
	//UInt	i	=	0;
	if ( m_bNotSupport)
	{
		return	false;
	}
	if ( m_uiNumLayers > 2 || m_uiNumLayers == 0 )
	{
		return	false;
	}
	return	ret;
}

ErrVal
H264AVCDecoder::checkSliceGap( BinDataAccessor*  pcBinDataAccessor,
                               MyList<BinData*>& cVirtualSliceList )
{
  static  Bool  bFinished=false;//TMM_EC
  Bool bEos;
  NalUnitType eNalUnitType;
  SliceHeader* pcSliceHeader = NULL;
  Int slicePoc;

  ROT( NULL == pcBinDataAccessor );

  bEos = ( NULL == pcBinDataAccessor->data() ) || ( 0 == pcBinDataAccessor->size() );
  slicePoc = 0;

	UInt	frame_num;
	UInt	uiLayerId;
	UInt	uiPocLsb;
	UInt	uiMaxFrameNum;
	UInt	uiMaxPocLsb;

  
//TMM_EC {{
  if ( bEos && !bFinished) 
  {
    bFinished = true;
    goto bEOS;
  }
//TMM_EC }}  
  
  if( ! bEos )
  {
    UInt uiNumBytesRemoved; //FIX_FRAG_CAVLC
//bug-fix suffix shenqiu{{
  //RNOK( m_pcNalUnitParser->initNalUnit( pcBinDataAccessor, NULL, uiNumBytesRemoved, true, false, true ) );
  RNOK( m_pcNalUnitParser->initNalUnit( pcBinDataAccessor, uiNumBytesRemoved, true, false, true ) );
//bug-fix suffix shenqiu}} 

    if ( m_pcNalUnitParser->getQualityLevel() != 0)
		{

			BinData	*pcBinData = new BinData;
			pcBinData->set( new UChar[pcBinDataAccessor->size()], pcBinDataAccessor->size());
			memcpy( pcBinData->data(), pcBinDataAccessor->data(), pcBinDataAccessor->size());
			cVirtualSliceList.pushBack( pcBinData);
//*///xk
			return  Err::m_nERR;
		}

    if ( ( m_pcNalUnitParser->getNalUnitType() == NAL_UNIT_CODED_SLICE_SCALABLE
          || m_pcNalUnitParser->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR_SCALABLE )
          && m_pcNalUnitParser->getLayerId() == 0)
		{
			return  Err::m_nERR;
		}

		if ( !checkSEIForErrorConceal())
		{
			return	Err::m_nInvalidParameter;
		}
    eNalUnitType = m_pcNalUnitParser->getNalUnitType();

    if( eNalUnitType != NAL_UNIT_CODED_SLICE        &&
//        eNalUnitType != NAL_UNIT_CODED_SLICE_IDR      && 
        eNalUnitType != NAL_UNIT_CODED_SLICE_SCALABLE && 
//        eNalUnitType != NAL_UNIT_CODED_SLICE_IDR_SCALABLE &&
        eNalUnitType != NAL_UNIT_END_OF_STREAM &&
		1)
    {
			if(eNalUnitType==NAL_UNIT_CODED_SLICE_IDR||eNalUnitType==NAL_UNIT_CODED_SLICE_IDR_SCALABLE)
      {
        RNOK( m_pcSliceReader->readSliceHeader( m_pcNalUnitParser,
                                                pcSliceHeader,
                                                m_uiFirstFragmentNumMbsInSlice,
                                                m_bFirstFragmentFGSCompSep 
		        ) );
        if(pcSliceHeader->getFrameNum()==0)
					m_uiMaxLayerId=pcSliceHeader->getLayerId()>m_uiMaxLayerId ? pcSliceHeader->getLayerId(): m_uiMaxLayerId;
      }
      return Err::m_nOK;
    }
    else
    {
      if ( eNalUnitType != NAL_UNIT_END_OF_STREAM)
			{
				// read the slice header
	      RNOK( m_pcSliceReader->readSliceHeader( m_pcNalUnitParser,
                                              pcSliceHeader,
                                              m_uiFirstFragmentNumMbsInSlice,
                                              m_bFirstFragmentFGSCompSep 
											                         ) );
				if(pcSliceHeader->getFrameNum()==0)
					m_uiMaxLayerId=pcSliceHeader->getLayerId()>m_uiMaxLayerId ? pcSliceHeader->getLayerId(): m_uiMaxLayerId;

	      calculatePoc( eNalUnitType, *pcSliceHeader, slicePoc );
//        if(pcSliceHeader->getFrameNum()==1&&m_uiMaxLayerId!=(m_uiNumLayers-1))
//					m_bNotSupport=true;
       	if ( pcSliceHeader->getFrameNum() == 1 && pcSliceHeader->getLayerId() == m_uiNumLayers-1)
				{
					if ( pcSliceHeader->getPicOrderCntLsb() != m_uiMaxGopSize)
					{
						m_bNotSupport	=	true;
					}
					if ( pcSliceHeader->getFirstMbInSlice() != 0)
					{
						m_bNotSupport	=	true;
					}
					if ( pcSliceHeader->getFrameNum() == 1)
					{
						UInt	i=pcSliceHeader->getPicOrderCntLsb();
						while ( i % 2 == 0)
						{
							i /=	2;
						}
						if ( i!= 1)
						{
							m_bNotSupport	=	true;
						}
					}
				}
				// F slices are also ignored
				if( pcSliceHeader->getSliceType() == F_SLICE || pcSliceHeader->getFrameNum() == 1 && pcSliceHeader->getLayerId() < m_uiMaxLayerId)
//				if( pcSliceHeader->getSliceType() == F_SLICE)
				{
					if( pcSliceHeader != NULL )
						delete pcSliceHeader;

					return Err::m_nOK;
				}
//	detection the gap of slice
				frame_num	=	pcSliceHeader->getFrameNum();
				uiLayerId	=	pcSliceHeader->getLayerId();
				uiPocLsb	=	pcSliceHeader->getPicOrderCntLsb();
//				uiGopSize	=	pcSliceHeader->getSPS().getNumRefFrames() + 1;	//??
				UInt uiGopSize	=	m_uiGopSize[uiLayerId];
				if ( frame_num == 1 &&m_uiMaxLayerId==uiLayerId&& uiPocLsb > uiGopSize)
				{
					m_bNotSupport = true;
					if( pcSliceHeader != NULL )
						delete pcSliceHeader;
					return	Err::m_nOK;
				}
				uiMaxFrameNum	=	1 << pcSliceHeader->getSPS().getLog2MaxFrameNum();
				uiMaxPocLsb		=	1 << pcSliceHeader->getSPS().getLog2MaxPicOrderCntLsb();
			}
			else
			{
        bEOS://xk
        SliceHeader *pcSH;
        if ( m_pcVeryFirstSliceHeader == NULL)
        {
          pcSH  = m_apcMCTFDecoder[m_uiNextLayerId]->m_pcVeryFirstSliceHeader;
        }
        else
        {
          pcSH  = m_pcVeryFirstSliceHeader;
        }
				uiMaxFrameNum	=	1 << pcSH->getSPS().getLog2MaxFrameNum();
				uiMaxPocLsb		=	1 << pcSH->getSPS().getLog2MaxPicOrderCntLsb();
				uiLayerId	=	m_uiNextLayerId;
				UInt uiGopSize	=	1 << m_uiDecompositionStages[uiLayerId];
//				uiGopSize	=	m_uiGopSize[uiLayerId];
				if ( m_uiFrameIdx[uiLayerId] % uiGopSize != 1)
				{
					frame_num	=	m_pauiFrameNumInGOP[uiLayerId][m_uiGopSize[uiLayerId]-1] % uiMaxFrameNum;
					uiPocLsb	=	m_pauiPocInGOP[uiLayerId][m_uiGopSize[uiLayerId]-1] % uiMaxPocLsb;
				}
				else
				{
					frame_num	=	m_pauiFrameNumInGOP[uiLayerId][0] % uiMaxFrameNum;
					uiPocLsb	=	m_pauiPocInGOP[uiLayerId][0] % uiMaxPocLsb;
				}
//				if ( m_uiFrameIdx[uiLayerId] % uiGopSize == 1)
				{
					frame_num	-=	(uiGopSize >> 1);
//xk
          if ( uiGopSize == 1)
            frame_num -=  1;
					uiPocLsb	-=	m_uiMaxGopSize;
				}
			}
		}

		if ( frame_num != m_uiNextFrameNum || uiLayerId != m_uiNextLayerId || uiPocLsb != (m_uiNextPoc % uiMaxPocLsb))
		{
//	judge if the uncompleted GOP
			UInt	uiFrameIdx	=	0;
			UInt	uiGopSize	=	1 << m_uiDecompositionStages[uiLayerId];
//			UInt	uiGopSize	=	m_uiGopSize[uiLayerId];

			for ( ;uiFrameIdx < uiGopSize; uiFrameIdx++)
			{
				if ( m_pauiPocInGOP[uiLayerId][uiFrameIdx]	% uiMaxPocLsb ==	uiPocLsb)
					break;
			}
			if ( ( m_pauiFrameNumInGOP[uiLayerId][uiFrameIdx] % uiMaxFrameNum ) == frame_num || (uiPocLsb-1) / m_uiMaxGopSize != ((m_uiNextPoc-1) % uiMaxPocLsb) / m_uiMaxGopSize)
			{
				if ( m_uiNextLayerId == 0 && m_baseMode == 1) 
				{
					BinData	*pcBinData = new BinData;
					pcBinData->set( new UChar[11], 11);
//					*(pcBinData->data()+0)	=	NAL_UNIT_VIRTUAL_BASELAYER;
					*(pcBinData->data()+0)	=	NAL_UNIT_CODED_SLICE;
					*(Int*)(pcBinData->data()+1)	=	0xdeadface;
					*(Short*)(pcBinData->data()+5)	=	(Short) m_uiNextFrameNum;
					*(Short*)(pcBinData->data()+7)	=	(Short) m_uiNextPoc;
					*(pcBinData->data()+9)	=	(UChar) m_uiNextTempLevel;
					*(pcBinData->data()+pcBinData->size()-1)	=	0;
					cVirtualSliceList.pushFront( pcBinData);

				}
				else
				{
					BinData	*pcBinData = new BinData;
					pcBinData->set( new UChar[11], 11);
//					*(pcBinData->data()+0)	=	NAL_UNIT_VIRTUAL_ENHANCELAYER;
					*(pcBinData->data()+0)	=	NAL_UNIT_CODED_SLICE_SCALABLE;
					*(Int*)(pcBinData->data()+1)	=	0xdeadface;
					*(Short*)(pcBinData->data()+5)	=	(Short)m_uiNextFrameNum;
					*(Short*)(pcBinData->data()+7)	=	(Short)m_uiNextPoc;
					*(pcBinData->data()+9)	=	(UChar) m_uiNextTempLevel;
					*(pcBinData->data()+pcBinData->size()-1)	=	0;
					cVirtualSliceList.pushFront( pcBinData);

          m_uiNumOfNALInAU++;  //TMM_EC
				}
			}
			else
			{
//	uncomplete GOP structure
//	calculate the gop size of the layer current packet belongs to
				if ( uiPocLsb % ( 2<<(m_uiMaxDecompositionStages-m_uiDecompositionStages[uiLayerId])) != 0)
				{
					m_uiGopSize[uiLayerId]	=	(((frame_num-1) % (uiGopSize>>1))<< 1) + 1;
				}
				else
				{
					UInt	uiFrameDiff	=	( m_pauiFrameNumInGOP[uiLayerId][uiFrameIdx] % uiMaxFrameNum)  - frame_num;
					UInt	uiTL	=	m_uiDecompositionStages[uiLayerId];
					UInt	uiTmp	=	uiPocLsb >> (m_uiMaxDecompositionStages - m_uiDecompositionStages[uiLayerId]);
					while( uiTmp % 2 == 0)
					{
						uiTmp	>>=1;
						uiTL--;
					}
          UInt ui = 0;
					for ( ui=m_uiGopSize[uiLayerId]&(-2); ui>0; ui-=2)
					{
						UInt	uiTempLevel	=	m_uiDecompositionStages[uiLayerId];
						uiTmp	=	ui;
						while( uiTmp % 2 == 0)
						{
							uiTmp	>>=1;
							uiTempLevel--;
						}
						if ( uiTL <= uiTempLevel)
						{
							continue;
						}
						uiFrameDiff--;
						if ( uiFrameDiff == 0)
						{
							break;
						}
					}
					m_uiGopSize[uiLayerId]	=	ui - 1;
				}
        //	calculate the gop size of other layers
        UInt  ui=0; 
				for ( ui=0; ui<uiLayerId; ui++)
				{
					m_uiGopSize[ui]	=	m_uiGopSize[uiLayerId] >> (m_uiDecompositionStages[uiLayerId] - m_uiDecompositionStages[ui]);
				}
				for ( ui=uiLayerId+1; ui<m_uiNumLayers; ui++)
				{
					m_uiGopSize[ui]	=	( m_uiGopSize[uiLayerId] << (m_uiDecompositionStages[ui] - m_uiDecompositionStages[uiLayerId])) + ((1<<(m_uiDecompositionStages[ui] - m_uiDecompositionStages[uiLayerId])) -1);
				}
//	calculate the correct frame_num and poc of every packet in this uncompleted gop each layer
				m_uiFrameIdx[m_uiNextLayerId]--;
				for ( ui=0; ui<m_uiNumLayers; ui++)
				{
					uiFrameIdx      	=	0;
					UInt	uiFrameNum	=	m_pauiFrameNumInGOP[ui][0];
					UInt	uiPoc	=	( m_uiNextPoc - 1 ) & -(1<<m_uiMaxDecompositionStages);
					if ( ui == 0 || m_uiFrameIdx[ui] % (1<<m_uiDecompositionStages[ui]) != 0)
						uiFrameNum	-=	( 1 << ( m_uiDecompositionStages[ui] - 1 ) );
					UInt	uiDecompositionStagesSub	=	m_uiMaxDecompositionStages - m_uiDecompositionStages[ui];
					for( UInt uiTemporalLevel = 0; uiTemporalLevel <= m_uiDecompositionStages[ui]; uiTemporalLevel++ )
					{
						UInt      uiStep    = ( 1 << ( m_uiDecompositionStages[ui] - uiTemporalLevel ) );
						for( UInt uiFrameId = uiStep; uiFrameId <= m_uiGopSize[ui]; uiFrameId += ( uiStep << 1 ) )
						{
							m_pauiPocInGOP[ui][uiFrameIdx]	=	(uiFrameId << uiDecompositionStagesSub ) + uiPoc;
							m_pauiFrameNumInGOP[ui][uiFrameIdx]	=	uiFrameNum;
							m_pauiTempLevelInGOP[ui][uiFrameIdx]	=	uiTemporalLevel;
							uiFrameIdx++;
							if ( uiFrameId % 2 == 0)
								uiFrameNum++;
						}
					}
					for ( uiFrameIdx=0; uiFrameIdx<m_uiFrameIdx[ui] % (1<<m_uiDecompositionStages[ui]); uiFrameIdx++)
					{
						m_pauiPocInGOP[ui][uiFrameIdx]	+=	m_uiMaxGopSize;
						m_pauiFrameNumInGOP[ui][uiFrameIdx]	+=	(1<<(m_uiDecompositionStages[ui]-1));
					}
				}
			  do
			  {
				  if (m_uiFrameIdx[m_uiNextLayerId] % m_uiMaxGopSize < m_uiGopSize[m_uiNextLayerId])
					  break;
				  m_uiNextLayerId		=	(m_uiNextLayerId + 1) % m_uiNumLayers;
			  }
				while( true);
				if (m_uiFrameIdx[m_uiNextLayerId] % m_uiMaxGopSize < m_uiGopSize[m_uiNextLayerId])
				{
					uiMaxFrameNum				=	1 << pcSliceHeader->getSPS().getLog2MaxFrameNum();
					m_uiNextFrameNum		=	m_pauiFrameNumInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize] % uiMaxFrameNum;
					m_uiNextPoc					=	m_pauiPocInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize];
					m_uiNextTempLevel		=	m_pauiTempLevelInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize];
          // TMM_EC
          if ( uiGopSize == 1)
					  m_pauiFrameNumInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize]	+=	1 ;
          else
            m_pauiFrameNumInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize]	+=	1 << ( m_uiDecompositionStages[m_uiNextLayerId] - 1);

					m_pauiPocInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize]	+=	m_uiMaxGopSize;
					m_uiFrameIdx[m_uiNextLayerId]++;
				}
				if ( frame_num != m_uiNextFrameNum || uiLayerId != m_uiNextLayerId || uiPocLsb != (m_uiNextPoc % uiMaxPocLsb))
				{
					if ( m_uiNextLayerId == 0)
					{
						BinData	*pcBinData = new BinData;
						pcBinData->set( new UChar[11], 11);
//						*(pcBinData->data()+0)	=	NAL_UNIT_VIRTUAL_BASELAYER;
						*(pcBinData->data()+0)	=	NAL_UNIT_CODED_SLICE;
						*(Int*)(pcBinData->data()+1)	=	0xdeadface;
						*(Short*)(pcBinData->data()+5)	=	(Short)m_uiNextFrameNum;
						*(Short*)(pcBinData->data()+7)	=	(Short)m_uiNextPoc;
						*(pcBinData->data()+9)	=	(UChar)m_uiNextTempLevel;
						*(pcBinData->data()+pcBinData->size()-1)	=	0;
						cVirtualSliceList.pushFront( pcBinData);

					}
					else
					{
						BinData	*pcBinData = new BinData;
						pcBinData->set( new UChar[11], 11);
//						*(pcBinData->data()+0)	=	NAL_UNIT_VIRTUAL_ENHANCELAYER;
						*(pcBinData->data()+0)	=	NAL_UNIT_CODED_SLICE_SCALABLE;
						*(Int*)(pcBinData->data()+1)	=	0xdeadface;
						*(Short*)(pcBinData->data()+5)	=	(Short)m_uiNextFrameNum;
						*(Short*)(pcBinData->data()+7)	=	 (Short)m_uiNextPoc;
						*(pcBinData->data()+9)	=	(UChar)m_uiNextTempLevel;
						*(pcBinData->data()+pcBinData->size()-1)	=	0;
						cVirtualSliceList.pushFront( pcBinData);

            m_uiNumOfNALInAU++;  //TMM_EC
					}
				}
			}
		}
	}

  if( pcSliceHeader != NULL )
    delete pcSliceHeader;

  return Err::m_nOK;
}
//TMM_EC }}
ErrVal
H264AVCDecoder::initPacket( BinDataAccessor*  pcBinDataAccessor,
													  UInt&             ruiNalUnitType,
														UInt&             ruiMbX,
														UInt&             ruiMbY,
														UInt&             ruiSize
														//,UInt&             ruiNonRequiredPic  //NonRequired JVT-Q066
														//JVT-P031
														, Bool            bPreParseHeader //FRAG_FIX
														, Bool			      bConcatenated //FRAG_FIX_3
														, Bool&           rbStartDecoding,
														UInt&             ruiStartPos,
														UInt&             ruiEndPos,
														Bool&             bFragmented,
														Bool&             bDiscardable
                            //~JVT-P031
                            , Bool*           pbFgsParallelDecoding
                            , UInt*           puiNumFragments
                            , UChar**         ppucFragBuffers
                            )
{
  ROF( m_bInitDone );
  UInt uiLayerId;

  ROT( NULL == pcBinDataAccessor );
  if ( NULL == pcBinDataAccessor->data() || 0 == pcBinDataAccessor->size() )
  {
    // switch 
    SliceHeader* pTmp = m_pcSliceHeader;
    m_pcSliceHeader   = m_pcPrevSliceHeader;
    m_pcPrevSliceHeader = pTmp;


    m_bLastFrame = true;
    delete m_pcSliceHeader;
    m_pcSliceHeader = 0;
    rbStartDecoding = true; //JVT-P031
    return Err::m_nOK;
  }

//  Bool KeyPicFlag = false; //bug-fix suffix shenqiu
  static Bool bSuffixUnit = false;  //JVT-S036 lsj

  Bool bFirstFragment;

  //JVT-P031
  Bool bLastFragment;
  UInt uiHeaderBits;
  getDecodedResolution(m_uiDecodedLayer);
  m_pcNalUnitParser->setDecodedLayer(m_uiDecodedLayer);
  ruiStartPos = m_pcNalUnitParser->getNalHeaderSize(pcBinDataAccessor);
  ruiStartPos = 0; //FRAG_FIX
  //~JVT-P031
  UInt uiNumBytesRemoved; //FIX_FRAG_CAVLC
//bug-fix suffix shenqiu{{
  //RNOK( m_pcNalUnitParser->initNalUnit( pcBinDataAccessor, &KeyPicFlag,uiNumBytesRemoved, bPreParseHeader , bConcatenated) ); //BUG_FIX_FT_01_2006_2 //FIX_FRAG_CAVLC

  RNOK( m_pcNalUnitParser->initNalUnit( 
    pcBinDataAccessor, uiNumBytesRemoved, bPreParseHeader, 
    bConcatenated, false, puiNumFragments, ppucFragBuffers) ); //BUG_FIX_FT_01_2006_2 //FIX_FRAG_CAVLC

//bug-fix suffix shenqiu}} 
  UInt uiBitsLeft = m_pcNalUnitParser->getBitsLeft(); //JVT-P031

  ruiNalUnitType = m_pcNalUnitParser->getNalUnitType();
  
  //TMM_EC {{
  if(!bPreParseHeader && ruiNalUnitType==NAL_UNIT_END_OF_STREAM)
  {
	  SliceHeader* pTmp = m_pcSliceHeader;
    m_pcSliceHeader   = m_pcPrevSliceHeader;
    m_pcPrevSliceHeader = pTmp;

		for ( UInt i=0; i< m_uiNumLayers; i++)
		{
			if (m_pauiPocInGOP[i])       delete	[] m_pauiPocInGOP[i];
      if (m_pauiFrameNumInGOP[i])  delete	[] m_pauiFrameNumInGOP[i];
			if (m_pauiTempLevelInGOP[i]) delete	[] m_pauiTempLevelInGOP[i];
		}
    m_bLastFrame = true;
    delete m_pcSliceHeader;
    m_pcSliceHeader = NULL;
    return Err::m_nOK;
  }
//TMM_EC }}
  switch ( m_pcNalUnitParser->getNalUnitType() )
  {
  case NAL_UNIT_CODED_SLICE:
  case NAL_UNIT_CODED_SLICE_IDR:
		{
    //JVT-P031
    RNOK( xStartSlice(bPreParseHeader,bFirstFragment, bLastFragment, pbFgsParallelDecoding, bDiscardable) ); //FRAG_FIX //TMM_EC //JVT-S036 lsj

    ruiEndPos = pcBinDataAccessor->size();
    bDiscardable = false;
    uiHeaderBits = uiBitsLeft - m_pcNalUnitParser->getBitsLeft();
    ruiStartPos += (uiHeaderBits+7)>>3; 
  	ruiStartPos = 0; //FRAG_FIX
    //~JVT-P031
    RNOK( m_pcControlMng      ->initSlice0(m_pcSliceHeader) );
    m_bActive = true;
    rbStartDecoding = true; //JVT-P031
	bSuffixUnit = true; //JVT-S036 lsj

//TMM_EC {{
		if (!m_bNotSupport && !bPreParseHeader)
		{
				UInt	uiMaxGopSize	=	m_uiMaxGopSize;
			  UInt	uiGopSize;
			  m_uiNextLayerId		=	m_pcSliceHeader->getLayerId();
			  do
			  {
				  m_uiNextLayerId		=	(m_uiNextLayerId + 1) % m_uiNumLayers;
				  uiGopSize	=	1 << m_uiDecompositionStages[m_uiNextLayerId];

				  if (m_uiFrameIdx[m_uiNextLayerId] % uiMaxGopSize < m_uiGopSize[m_uiNextLayerId])
					  break;
				  m_uiFrameIdx[m_uiNextLayerId]++;
			  }
			  while( true);

			  if ( m_pcNalUnitParser->getNalUnitType() != NAL_UNIT_CODED_SLICE_IDR || m_uiNextLayerId == 0)
			  {
				  UInt	uiMaxFrameNum	=	1 << m_pcSliceHeader->getSPS().getLog2MaxFrameNum();
  				//UInt	uiMaxPocLsb		=	1 << m_pcSliceHeader->getSPS().getLog2MaxPicOrderCntLsb();
				  m_uiNextFrameNum		=	m_pauiFrameNumInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize] % uiMaxFrameNum;
				  m_uiNextPoc					=	m_pauiPocInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize];
				  m_uiNextTempLevel		=	m_pauiTempLevelInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize];
				  if ( uiGopSize == 1)
					  m_pauiFrameNumInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize]	+=	1;
				  else
					  m_pauiFrameNumInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize]	+=	uiGopSize >> 1;
				  m_pauiPocInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize]	+=	uiMaxGopSize;
				  m_uiFrameIdx[m_uiNextLayerId]++;
			  }
			  else
			  {
				  m_uiNextFrameNum	=	0;
				  m_uiNextPoc			=	0;
			  }
		}
//TMM_EC }}    
    m_pcSliceHeader->setFGSCodingMode( m_bFGSCodingMode );
    m_pcSliceHeader->setGroupingSize ( m_uiGroupingSize );
    UInt ui;
    for( ui = 0; ui < 16; ui++ )
    {
      m_pcSliceHeader->setPosVect( ui, m_uiPosVect[ui] );
    }
    RNOK( m_pcSliceHeader->checkPosVectors() );
	}
	break;
  case NAL_UNIT_SPS:
    {
      SequenceParameterSet* pcSPS = NULL;
      RNOK( SequenceParameterSet::create  ( pcSPS   ) );
      RNOK( pcSPS               ->read    ( m_pcHeaderSymbolReadIf,
                                            m_pcNalUnitParser->getNalUnitType() ) );
      // It is assumed that layer 0 and layer 1 use the first two SPSs, respectively.
      if( NULL == m_pcVeryFirstSPS )
      {
        setVeryFirstSPS( pcSPS );
      }
//TMM_EC {{
			for ( UInt i=0; i<m_uiNumLayers; i++)
			{
				UInt	uiDecompositionStagesSub	=	m_uiMaxDecompositionStages - m_uiDecompositionStages[i];
				UInt	uiGopSize	=	m_uiGopSize[i];
				m_pauiPocInGOP[i]				=	new	UInt[uiGopSize];
				m_pauiFrameNumInGOP[i]		=	new	UInt[uiGopSize];
				m_pauiTempLevelInGOP[i]	=	new	UInt[uiGopSize];
				UInt	uiFrameIdx	=	0;
				UInt	uiFrameNum	=	1;
				for( UInt uiTemporalLevel = 0; uiTemporalLevel <= m_uiDecompositionStages[i]; uiTemporalLevel++ )
				{
					UInt      uiStep    = ( 1 << ( m_uiDecompositionStages[i] - uiTemporalLevel ) );
					for( UInt uiFrameId = uiStep; uiFrameId <= uiGopSize; uiFrameId += ( uiStep << 1 ) )
					{
						m_pauiPocInGOP[i][uiFrameIdx]	=	uiFrameId << uiDecompositionStagesSub;
						m_pauiFrameNumInGOP[i][uiFrameIdx]	=	uiFrameNum;
						m_pauiTempLevelInGOP[i][uiFrameIdx]	=	uiTemporalLevel;
						uiFrameIdx++;
						if ( uiFrameId % 2 == 0)
							uiFrameNum++;
					}
				}
			}
//TMM_EC }}
			if ( pcSPS->getProfileIdc()==SCALABLE_PROFILE )
      {
//TMM_EC {{
				if ( pcSPS->getSeqParameterSetId() == 0)
				{
					 // m_bNotSupport	=	true;
					m_baseMode = 0;
				}
//TMM_EC }}
				m_bEnhancementLayer = true;
        m_bSpatialScalability = pcSPS->getFrameHeightInMbs() != m_pcVeryFirstSPS->getFrameHeightInMbs();
      }
			m_bNewSPS = true;
      RNOK( m_pcParameterSetMng ->store   ( pcSPS   ) );

      // Copy simple priority ID mapping from SPS to NAL unit parser
    /*  if ( !pcSPS->getNalUnitExtFlag() )
      {
        for ( UInt uiPriId = 0; uiPriId < pcSPS->getNumSimplePriIdVals(); uiPriId++)
        {
            UInt uiLayer, uiTempLevel, uiQualLevel;
            pcSPS->getSimplePriorityMap( uiPriId, uiTempLevel, uiLayer, uiQualLevel );
            m_pcNalUnitParser->setSimplePriorityMap( uiPriId, uiTempLevel, uiLayer, uiQualLevel );
        }

      }
 JVT-S036 lsj */

      ruiMbX  = pcSPS->getFrameWidthInMbs ();
      ruiMbY  = pcSPS->getFrameHeightInMbs();
      ruiSize = max( ruiSize, ( (ruiMbX << 3 ) + YUV_X_MARGIN ) * ( ( ruiMbY << 3 ) + YUV_Y_MARGIN ) * 6 );
			m_pcControlMng->initSPS( *pcSPS, m_uiRecLayerId );
      ruiEndPos = pcBinDataAccessor->size(); //JVT-P031
      bDiscardable = false;//JVT-P031
      rbStartDecoding = true;//JVT-P031

      if( pcSPS->getNumFGSVectModes() == 1)
      {
        m_bFGSCodingMode = pcSPS->getFGSCodingMode( 0 );
        m_uiGroupingSize = pcSPS->getGroupingSize ( 0 );
        UInt ui;
        for(ui = 0; ui < 16; ui++)
        {
          m_uiPosVect[ui] = pcSPS->getPosVect( 0, ui );
        }
      }
      else if( m_pcSliceHeader ) 
      {
        m_bFGSCodingMode = m_pcSliceHeader->getFGSCodingMode();
        m_uiGroupingSize = m_pcSliceHeader->getGroupingSize ();
        UInt ui;
        for(ui = 0; ui < 16; ui++)
        {
          m_uiPosVect[ui] = m_pcSliceHeader->getPosVect(ui);
        }
      }
    }
    break;

  case NAL_UNIT_PPS:
    {
      PictureParameterSet* pcPPS = NULL;
      RNOK( PictureParameterSet::create( pcPPS  ) );
      RNOK( pcPPS->read( m_pcHeaderSymbolReadIf,
                         m_pcNalUnitParser->getNalUnitType() ) );
      RNOK( m_pcParameterSetMng->store( pcPPS   ) );
      ruiEndPos = pcBinDataAccessor->size();//JVT-P031
      bDiscardable = false;//JVT-P031
      rbStartDecoding = true;//JVT-P031
    }
    break;

  case NAL_UNIT_CODED_SLICE_SCALABLE:
  case NAL_UNIT_CODED_SLICE_IDR_SCALABLE:
    {
      bFragmented = m_pcNalUnitParser->getFragmentedFlag() && (m_pcNalUnitParser->getFragmentOrder()!=0 || !m_pcNalUnitParser->getLastFragmentFlag());
      //JVT-P031
      getDecodedResolution(m_uiDecodedLayer);
      if(m_pcNalUnitParser->getLayerId() < m_uiDecodedLayer && m_pcNalUnitParser->getDiscardableFlag())
        bDiscardable = true;
      else
        bDiscardable = false;

      RNOK( xStartSlice(bPreParseHeader, bFirstFragment, bLastFragment, pbFgsParallelDecoding, bDiscardable) );

      if(bDiscardable)
        ruiEndPos = 0;
      else
        ruiEndPos = pcBinDataAccessor->size();

      uiHeaderBits = uiBitsLeft - m_pcNalUnitParser->getBitsLeft();
      // JVT-U116 LMI {
      UChar ucByte = pcBinDataAccessor->data()[3];
      if ( ucByte & 1 )
        uiHeaderBits += 8;
      // JVT-U116 LMI }

      if( (bDiscardable) || (!bFragmented) || (bFragmented && m_pcNalUnitParser->getFragmentOrder()==0) ) 
      {
        ruiStartPos = 0;
      }
      else
      {
        ruiStartPos += (uiHeaderBits+(1+NAL_UNIT_HEADER_SVC_EXTENSION_BYTES)*8+7)>>3;//BUG_FIX_FT_01_2006_2
        //(1+NAL_UNIT_HEADER_SVC_EXTENSION_BYTES)*8 is used to take into account the nal header which has already been read
      }
      if(m_pcNalUnitParser->getFragmentedFlag() && ! m_pcNalUnitParser->getLastFragmentFlag())
      { //FIX_FRAG_CAVLC
        if(bPreParseHeader)
        {
          ruiEndPos -= uiNumBytesRemoved;
        }//~FIX_FRAG_CAVLC
        while( pcBinDataAccessor->data()[ruiEndPos - 1] == 0 )
          ruiEndPos --;
        if( pbFgsParallelDecoding != 0 && (! (*pbFgsParallelDecoding) ) )
        {
          ruiEndPos --; 
        }

      }//FIX_FRAG_CAVLC
      if(!bDiscardable)//~JVT-P031
        RNOK( m_pcControlMng      ->initSlice0(m_pcSliceHeader) );
      //bug-fix suffix shenqiu{{  
      // if(m_pcSliceHeader) //JVT-P031
      
      //bug-fix suffix shenqiu
      //JVT-P031
      
      // the variable m_pcSliceHeader->m_uiLastFragOrder is not updated
      if( (bLastFragment) || (!bFragmented) || 
        ( bFragmented && m_uiLastFragOrder+1 == m_uiNumberOfFragment[m_pcSliceHeader->getLayerId()][m_pcSliceHeader->getQualityLevel()])) 

        rbStartDecoding = true;
      //~JVT-P031
      //TMM_EC {{
      if ( !m_bNotSupport && !bPreParseHeader && m_pcNalUnitParser->getQualityLevel() == 0)
      {
        UInt	uiMaxGopSize	=	m_uiMaxGopSize;
			  UInt	uiGopSize;
			  m_uiNextLayerId		=	m_pcSliceHeader->getLayerId();
			  do
			  {
				  m_uiNextLayerId		=	(m_uiNextLayerId + 1) % m_uiNumLayers;
				  uiGopSize	=	1 << m_uiDecompositionStages[m_uiNextLayerId];
				  if (m_uiFrameIdx[m_uiNextLayerId] % uiMaxGopSize < m_uiGopSize[m_uiNextLayerId])
					  break;
				  m_uiFrameIdx[m_uiNextLayerId]++;
			  }
			  while( true);
//EC bug fix
				if ( m_pcNalUnitParser->getNalUnitType() != NAL_UNIT_CODED_SLICE_IDR_SCALABLE || m_uiNextLayerId == 0)
				{
				  UInt	uiMaxFrameNum	=	1 << m_pcSliceHeader->getSPS().getLog2MaxFrameNum();
 				  //UInt	uiMaxPocLsb		=	1 << m_pcSliceHeader->getSPS().getLog2MaxPicOrderCntLsb();
				  m_uiNextFrameNum		=	m_pauiFrameNumInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize] % uiMaxFrameNum;
				  m_uiNextPoc					=	m_pauiPocInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize];
				  m_uiNextTempLevel		=	m_pauiTempLevelInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize];
				  if ( uiGopSize == 1) 
				      m_pauiFrameNumInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize]	+=	1;
				  else
					  m_pauiFrameNumInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize]	+=	uiGopSize >> 1;
				  m_pauiPocInGOP[m_uiNextLayerId][m_uiFrameIdx[m_uiNextLayerId] % uiGopSize]	+=	uiMaxGopSize;
				  m_uiFrameIdx[m_uiNextLayerId]++;
			  }
			  else
			  {
				  m_uiNextFrameNum	=	0;
				  m_uiNextPoc			=	0;
			  }
		  }
//TMM_EC }}
  if(m_pcSliceHeader)
  {
    m_pcSliceHeader->setFGSCodingMode( m_bFGSCodingMode );
    m_pcSliceHeader->setGroupingSize ( m_uiGroupingSize );
    UInt ui;
    for(ui = 0; ui < 16; ui++)
    {
      m_pcSliceHeader->setPosVect( ui, m_uiPosVect[ui] );
    }
    RNOK( m_pcSliceHeader->checkPosVectors() );
  }
  }
  break;

  case NAL_UNIT_SEI:
    {
      //===== just for trace file =====
      SEI::MessageList  cMessageList;
			UInt	i;
      RNOK( SEI::read( m_pcHeaderSymbolReadIf, cMessageList ) );
      while( ! cMessageList.empty() )
      {
        SEI::SEIMessage*  pcSEIMessage = cMessageList.popBack();
		if(pcSEIMessage->getMessageType() == SEI::NON_REQUIRED_SEI)
		{
			m_pcNonRequiredSei = (SEI::NonRequiredSei*) pcSEIMessage;
			m_uiNonRequiredSeiReadFlag = 1;
		}
		else
		{
				  if ( pcSEIMessage->getMessageType() == SEI::SCALABLE_SEI)
				  {
//	trick
            m_uiNumLayers						=	((SEI::ScalableSei*)pcSEIMessage)->getDependencyId( ((SEI::ScalableSei*)pcSEIMessage)->getNumLayersMinus1()) + 1;
					  for ( uiLayerId=0; uiLayerId<((SEI::ScalableSei*)pcSEIMessage)->getNumLayersMinus1()+1; uiLayerId++)
            {
              if ( ((SEI::ScalableSei*)pcSEIMessage)->getDependencyId( uiLayerId) != 0)
                break;
            }
            uiLayerId--;
            m_uiDecompositionStages[0]	=	((SEI::ScalableSei*)pcSEIMessage)->getTemporalLevel( uiLayerId);
            m_uiDecompositionStages[m_uiNumLayers-1]	=	((SEI::ScalableSei*)pcSEIMessage)->getTemporalLevel( ((SEI::ScalableSei*)pcSEIMessage)->getNumLayersMinus1());
					  m_uiMaxDecompositionStages = m_uiDecompositionStages[m_uiNumLayers-1];
					  m_uiMaxGopSize	=	1 << m_uiMaxDecompositionStages;
						for ( i=0; i< m_uiNumLayers; i++)
						  m_uiGopSize[i]	=	1 << m_uiDecompositionStages[i];
				  }
// JVT-T073 {
				  else if( pcSEIMessage->getMessageType() == SEI::SCALABLE_NESTING_SEI )
				  {
				      //do nothing, or add your code here
				  }
// JVT-T073 }
			    delete pcSEIMessage;
          pcSEIMessage = NULL;
		}
      }
      ruiEndPos = (uiBitsLeft+7)/8; //FRAG_FIX
      bDiscardable = false;//JVT-P031
      rbStartDecoding = true;//JVT-P031
    }
    break;
  case NAL_UNIT_ACCESS_UNIT_DELIMITER:
    {
      RNOK ( m_pcNalUnitParser->readAUDelimiter());
      ruiEndPos = pcBinDataAccessor->size();//JVT-P031
      bDiscardable = false;//JVT-P031
      rbStartDecoding = true;//JVT-P031
    }
    break;
  case NAL_UNIT_END_OF_SEQUENCE:
    {
      RNOK ( m_pcNalUnitParser->readEndOfSeqence());
      ruiEndPos = pcBinDataAccessor->size();//JVT-P031
      bDiscardable = false;//JVT-P031
      rbStartDecoding = true;//JVT-P031
    }
  case NAL_UNIT_END_OF_STREAM:
    {
      RNOK ( m_pcNalUnitParser->readEndOfStream());
      ruiEndPos = pcBinDataAccessor->size();//JVT-P031
      bDiscardable = false;//JVT-P031
      rbStartDecoding = true;//JVT-P031
    }
    break;
  default:
    return Err::m_nERR;
    break;
  }

  m_uiNonRequiredPic = 0; //NonRequired JVT-Q066
  //ruiNonRequiredPic = 0;

  if(m_pcSliceHeader)
  {
	  m_uiCurrPicLayer = (m_pcSliceHeader->getLayerId() << 4) + m_pcSliceHeader->getQualityLevel();
	  if(m_uiCurrPicLayer == 0 || m_uiCurrPicLayer <= m_uiPrevPicLayer)
	  {
		  if(m_uiNonRequiredSeiReadFlag == 0 && m_pcNonRequiredSei)
		  {
			  m_pcNonRequiredSei->destroy();
			  m_pcNonRequiredSei = NULL;
		  }
		  m_uiNonRequiredSeiRead = m_uiNonRequiredSeiReadFlag;
		  m_uiNonRequiredSeiReadFlag = 0;
	  }
	  m_uiPrevPicLayer = m_uiCurrPicLayer;

	  if(m_uiNonRequiredSeiRead == 1)
	  {
		  for(UInt i = 0; i <= m_pcNonRequiredSei->getNumInfoEntriesMinus1(); i++)
		  {
			  if(m_pcNonRequiredSei->getEntryDependencyId(i))  // it should be changed to if(DenpendencyId == LayerId of the shown picture) 
			  {
				  for(UInt j = 0; j <= m_pcNonRequiredSei->getNumNonRequiredPicsMinus1(i); j++)
				  {
					  if(m_pcSliceHeader->getLayerId() == m_pcNonRequiredSei->getNonRequiredPicDependencyId(i,j) &&
						  m_pcSliceHeader->getQualityLevel() == m_pcNonRequiredSei->getNonRequiredPicQulityLevel(i,j))  // it should be add something about FragmentFlag
					  {
						  m_uiNonRequiredPic = 1;  //NonRequired JVT-Q066
						//  ruiNonRequiredPic = 1;
						  ROTRS( m_apcMCTFDecoder[m_pcSliceHeader->getLayerId()]->getWaitForIdr() && !m_pcSliceHeader->isIdrNalUnit(), Err::m_nOK );
						  m_apcMCTFDecoder[m_pcSliceHeader->getLayerId()]->setWaitForIdr(false);
						  return Err::m_nOK;
					  }
				  }
			  }
		  }
	  }
  }
  return Err::m_nOK;
}

//JVT-P031
ErrVal
H264AVCDecoder::initPacket( BinDataAccessor*  pcBinDataAccessor)
{
  return m_pcNalUnitParser->initSODBNalUnit(pcBinDataAccessor);
}
//~JVT-P031

//JVT-S036 lsj start
ErrVal
H264AVCDecoder::initPacketSuffix( BinDataAccessor*  pcBinDataAccessor,
											UInt&             ruiNalUnitType
											, Bool            bPreParseHeader 
											, Bool			      bConcatenated 
											, Bool&           rbStartDecoding
											 ,SliceHeader     *pcSliceHeader
											  ,Bool&		  SuffixEnable
								)
{
  ROF( m_bInitDone );
//  UInt uiLayerId;

  ROT( NULL == pcBinDataAccessor );
  if ( NULL == pcBinDataAccessor->data() || 0 == pcBinDataAccessor->size() )
  {
    // switch 
    SliceHeader* pTmp = m_pcSliceHeader;
    m_pcSliceHeader   = m_pcPrevSliceHeader;
    m_pcPrevSliceHeader = pTmp;


    m_bLastFrame = true;
    delete m_pcSliceHeader;
    m_pcSliceHeader = NULL;
    rbStartDecoding = true; //JVT-P031

	if((m_pcNalUnitParser->getNalUnitType() == NAL_UNIT_CODED_SLICE_SCALABLE || m_pcNalUnitParser->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR_SCALABLE)
		 && m_pcNalUnitParser->getLayerId() == 0 && m_pcNalUnitParser->getQualityLevel() == 0)
	{
		SuffixEnable = true;
	}
	else
	{
		SuffixEnable = false;
	}
    return Err::m_nOK;
  }


  //JVT-P031
//  Bool KeyPicFlag = false; //bug-fix suffix shenqiu
  getDecodedResolution(m_uiDecodedLayer);
  m_pcNalUnitParser->setDecodedLayer(m_uiDecodedLayer);
  //~JVT-P031
  UInt uiNumBytesRemoved; //FIX_FRAG_CAVLC
//bug-fix suffix shenqiu{{
  //RNOK( m_pcNalUnitParser->initNalUnit( pcBinDataAccessor, &KeyPicFlag,uiNumBytesRemoved, bPreParseHeader , bConcatenated) ); //BUG_FIX_FT_01_2006_2 //FIX_FRAG_CAVLC
  RNOK( m_pcNalUnitParser->initNalUnit( pcBinDataAccessor, uiNumBytesRemoved, bPreParseHeader , bConcatenated) ); //BUG_FIX_FT_01_2006_2 //FIX_FRAG_CAVLC
//bug-fix suffix shenqiu}} 
  ruiNalUnitType = m_pcNalUnitParser->getNalUnitType();
  
  
//TMM_EC }}
  switch ( m_pcNalUnitParser->getNalUnitType() )
  {

  case NAL_UNIT_CODED_SLICE_SCALABLE:
  case NAL_UNIT_CODED_SLICE_IDR_SCALABLE:
    {
		if( m_pcNalUnitParser->getLayerId() == 0 && m_pcNalUnitParser->getQualityLevel() == 0)
		{
			RNOK( m_pcSliceReader->readSliceHeaderSuffix( m_pcNalUnitParser->getNalUnitType   (),
														  m_pcNalUnitParser->getNalRefIdc     (),
														  m_pcNalUnitParser->getLayerId		  (),
														  m_pcNalUnitParser->getQualityLevel  (),
                              m_pcNalUnitParser->getUseBasePredFlag(),
														  pcSliceHeader
														  ) 
											);
			SuffixEnable = true;
			return Err::m_nOK;	
		}
		else
		{
			SuffixEnable = false;
			return Err::m_nOK;
		}
     
    }
    break;

  default:
	  {
		  SuffixEnable = false;
			return Err::m_nOK;
	  }
    break;
  }

  return Err::m_nOK;
}

//JVT-S036 lsj end

ErrVal
H264AVCDecoder::getBaseLayerPWTable( SliceHeader::PredWeightTable*& rpcPredWeightTable,
                                     UInt                           uiBaseLayerId,
                                     ListIdx                        eListIdx,
                                     Int                            iPoc )
{
  if( uiBaseLayerId || m_apcMCTFDecoder[uiBaseLayerId]->isActive() )
  {
    RNOK( m_apcMCTFDecoder[uiBaseLayerId]->getBaseLayerPWTable( rpcPredWeightTable, eListIdx, iPoc ) );
    return Err::m_nOK;
  }
  rpcPredWeightTable = &m_acLastPredWeightTable[eListIdx];
  return Err::m_nOK;
}

ErrVal
H264AVCDecoder::getBaseLayerUnit(UInt            uiBaseLayerId, 
                                 Int             iPoc,
                                 DPBUnit*      &pcBaseDPBUnit)
{
  RNOK( m_apcMCTFDecoder[uiBaseLayerId]->getBaseLayerUnit( iPoc , pcBaseDPBUnit) );
  return Err::m_nOK;
}

ErrVal
H264AVCDecoder::getBaseLayerData( IntFrame*&      pcFrame,
                                  IntFrame*&      pcResidual,
                                  MbDataCtrl*&    pcMbDataCtrl,
                                  MbDataCtrl*&    pcMbDataCtrlEL,
                                  Bool&           rbConstrainedIPred,
                                  Bool&           rbSpatialScalability,
                                  UInt            uiLayerId,
                                  UInt            uiBaseLayerId,
                                  Int             iPoc,
                                  UInt            uiBaseQualityLevel) //JVT-T054
{
  if(uiBaseLayerId != 0 || !m_apcMCTFDecoder[uiBaseLayerId]->getAVCBased() ||
    (uiBaseQualityLevel != 0 && m_bCGSSNRInAU))

  {
    //===== base layer is scalable extension =====
      //--- get data ---
	  RNOK( m_apcMCTFDecoder[uiBaseLayerId]->getBaseLayerData( pcFrame, 
	                                                         pcResidual, 
	                                                         pcMbDataCtrl, 
	                                                         pcMbDataCtrlEL, 
	                                                         rbConstrainedIPred, 
	                                                         rbSpatialScalability, 
	                                                         iPoc ) );
  }
  else
  {
    //===== base layer is standard H.264/AVC =====
    FrameUnit*  pcFrameUnit = m_pcFrameMng->getReconstructedFrameUnit( iPoc );
    ROF( pcFrameUnit );

    pcFrame           = rbSpatialScalability ? m_pcFrameMng->getRefinementIntFrame2(): 
                                                 m_pcFrameMng->getRefinementIntFrame();

    pcResidual          = pcFrameUnit ->getResidual();
    pcMbDataCtrl        = pcFrameUnit ->getMbDataCtrl();
		pcMbDataCtrlEL      = m_pcBaseLayerCtrlEL;
    rbConstrainedIPred  = pcFrameUnit ->getContrainedIntraPred();
  }
  return Err::m_nOK;
}

ErrVal
H264AVCDecoder::getBaseLayerDataAvailability( IntFrame*&      pcFrame,
                                              IntFrame*&      pcResidual,
                                              MbDataCtrl*&    pcMbDataCtrl,
                                              Bool&           rbBaseDataAvailable,
                                              Bool&           rbSpatialScalability,
                                              UInt            uiLayerId,
                                              UInt            uiBaseLayerId,
                                              Int             iPoc,
                                              UInt            uiBaseQualityLevel) //JVT-T054
{
  if(uiBaseLayerId != 0 || !m_apcMCTFDecoder[uiBaseLayerId]->getAVCBased() ||
    (uiBaseQualityLevel != 0 && m_bCGSSNRInAU))
  {
    //===== base layer is scalable extension =====
    //--- get spatial resolution ratio ---
    // TMM_ESS 
	  if( m_apcMCTFDecoder[uiLayerId]->getFrameHeight() != m_apcMCTFDecoder[uiBaseLayerId]->getFrameHeight() &&
        m_apcMCTFDecoder[uiLayerId]->getFrameWidth () != m_apcMCTFDecoder[uiBaseLayerId]->getFrameWidth ()   )
    {
      rbSpatialScalability = true;
    }
    else
    {
      rbSpatialScalability = false;
    }
	  // TMM_ESS 

    //--- get data ---
	  RNOK( m_apcMCTFDecoder[uiBaseLayerId]->getBaseLayerDataAvailability( pcFrame, pcResidual, pcMbDataCtrl, rbBaseDataAvailable, rbSpatialScalability, iPoc ) );
  }
  else
  {
    //===== base layer is standard H.264/AVC =====
    //--- get spatial resolution ratio ---
    // TMM_ESS 
	  if( m_apcMCTFDecoder[uiLayerId]->getFrameHeight() == 16 * m_pcVeryFirstSliceHeader->getSPS().getFrameHeightInMbs () &&
        m_apcMCTFDecoder[uiLayerId]->getFrameWidth () == 16 * m_pcVeryFirstSliceHeader->getSPS().getFrameWidthInMbs  ()   )
    {
      rbSpatialScalability = false;
    }
    else
    {
      rbSpatialScalability = true;
    }
	  // TMM_ESS 

    FrameUnit*  pcFrameUnit = m_pcFrameMng->getReconstructedFrameUnit( iPoc );
    ROF( pcFrameUnit );

    pcMbDataCtrl        = pcFrameUnit ->getMbDataCtrl();
    if(uiLayerId == uiBaseLayerId) 
      m_apcMCTFDecoder[0]->setILPrediction(pcFrameUnit->getFGSIntFrame(), FRAME); // TMM_INTERLACE

    rbBaseDataAvailable = pcMbDataCtrl>0;
  }
  return Err::m_nOK;
}

ErrVal
H264AVCDecoder::process( PicBuffer*       pcPicBuffer,
                         PicBufferList&   rcPicBufferOutputList,
                         PicBufferList&   rcPicBufferUnusedList,
                         PicBufferList&   rcPicBufferReleaseList )
 {
  ROF( m_bInitDone );

   //EIDR bug-fix {
	if(m_pcSliceHeader )
	{
		if(m_pcSliceHeader->getLayerId() == 0 )
		{
			m_pcSliceHeader->setInIDRAccess(m_pcSliceHeader->isIdrNalUnit());
		}
		else
		{
			m_pcSliceHeader->setInIDRAccess(m_pcPrevSliceHeader?m_pcPrevSliceHeader->getInIDRAccess():false);
		}
	}
 //EIDR bug-fix }
  
  //JVT-T054_FIX{
  if(m_bBaseSVCActive)
  {
    RNOK( xInitSlice( m_pcSliceHeader ) );
    m_apcMCTFDecoder[0]->GetAVCFrameForDPB(m_pcSliceHeader,pcPicBuffer, rcPicBufferOutputList, rcPicBufferUnusedList);
    m_bBaseSVCActive = false;
    m_bLastFrameReconstructed = true;
    return Err::m_nOK;
  }
  if(!m_bLastFrameReconstructed)
  {
   //JVT-T054}
    RNOK( xInitSlice( m_pcSliceHeader ) );
    //JVT-T054_FIX{
  }
  else
  {
    if( m_pcSliceHeader && m_pcSliceHeader->getLayerId() == 0 )
    {
      RNOK( m_pcPocCalculator->calculatePoc( *m_pcSliceHeader ) );
    }
  }

  m_bLastFrameReconstructed = false; //JVT-T054}


  if( m_bLastFrame )
  {
    if( m_uiRecLayerId > 0 || !m_bBaseLayerIsAVCCompatible ) // we have an MCTF reconstruction layer
    {
      PicBufferList cDummyList;
      Int           iMaxPoc;
      for( UInt uiLayer = 0; uiLayer < m_uiRecLayerId; uiLayer++ )
      {
        if( uiLayer == 0 && m_bBaseLayerIsAVCCompatible )
        {
          RNOK( m_pcFrameMng->outputAll() );
          RNOK( m_pcFrameMng->setPicBufferLists( cDummyList, rcPicBufferReleaseList ) );
        }
        RNOK( m_apcMCTFDecoder[uiLayer]       ->finishProcess( cDummyList,
                                                               rcPicBufferUnusedList,
                                                               iMaxPoc ) );
      }
      RNOK  ( m_apcMCTFDecoder[m_uiRecLayerId]->finishProcess( rcPicBufferOutputList,
                                                               rcPicBufferUnusedList,
                                                               iMaxPoc = MSYS_INT_MIN ) );
    }
    else
    {
      //===== output all remaining frames in decoded picture buffer =====

      PicBufferList cPicBufferList;
      Int           iMaxPoc;
      RNOK( m_pcFrameMng->outputAll() );
      RNOK( m_pcFrameMng->setPicBufferLists( cPicBufferList, rcPicBufferReleaseList ) );
      if(m_bCGSSNRInAU)
      {
        RNOK  ( m_apcMCTFDecoder[m_uiRecLayerId]->finishProcess( rcPicBufferOutputList,
                                                               rcPicBufferUnusedList,
                                                               iMaxPoc = MSYS_INT_MIN ) );
      }
      else
      {
        rcPicBufferOutputList += cPicBufferList;
      }
    }

    rcPicBufferUnusedList.pushBack( pcPicBuffer );
    return Err::m_nOK;
  }

  const NalUnitType eNalUnitType  = m_pcNalUnitParser->getNalUnitType();

  //====== check for AVC baselayer FGS refinement =====
  if( ( eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE || eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE ) &&
      m_bActive &&
      m_pcSliceHeader->getLayerId()   == 0 &&
      m_pcSliceHeader->getSliceType() == F_SLICE ) 
  {
//TMM_EC {{
    if ( m_pcSliceHeader->getTrueSlice())
    {
//      RNOK( xDecodeFGSRefinement( m_pcSliceHeader, pcPicBuffer ) );

			if ( Err::m_nOK != xDecodeFGSRefinement( m_pcSliceHeader, pcPicBuffer ) )
			{
				//===== switch slice headers and update =====
				SliceHeader*  pcTemp  = m_pcSliceHeader;
				m_pcSliceHeader       = m_pcPrevSliceHeader;
				m_pcPrevSliceHeader   = pcTemp;
			}
		}
    else
    {
      //===== switch slice headers and update =====
      SliceHeader*  pcTemp  = m_pcSliceHeader;
      m_pcSliceHeader       = m_pcPrevSliceHeader;
      m_pcPrevSliceHeader   = pcTemp;
    }
//TMM_EC }}
    rcPicBufferUnusedList.pushBack( pcPicBuffer );
    RNOK( m_pcNalUnitParser->closeNalUnit() );
    return Err::m_nOK;
  }


  //===== decode NAL unit =====
  switch( eNalUnitType )
  {
  case NAL_UNIT_CODED_SLICE_SCALABLE:
  case NAL_UNIT_CODED_SLICE_IDR_SCALABLE:
    {
      ROT( m_pcSliceHeader == NULL );
      m_uiLastLayerId = m_pcSliceHeader->getLayerId();
      m_bLastNalInAU = m_uiNumOfNALInAU == 0; //JVT-T054
// JVT-Q054 Red. Picture {
      if ( NULL != m_pcSliceHeader_backup )
      {
        RNOK( m_pcSliceHeader->sliceHeaderBackup( m_pcSliceHeader_backup ) );
      }
// JVT-Q054 Red. Picture }
      
      PicBufferList   cDummyList;
      PicBufferList&  rcOutputList  = ( m_uiLastLayerId == m_uiRecLayerId ? rcPicBufferOutputList : cDummyList );
//JVT-T054{
      Bool bHighestLayer;
      if(m_bFGSRefInAU)
        bHighestLayer = ( m_uiLastLayerId == m_uiRecLayerId );
      else
        bHighestLayer = ( m_uiLastLayerId == m_uiRecLayerId && m_bLastNalInAU);
//JVT-T054}
//	TMM EC {{
      if ( m_pcNalUnitParser->getQualityLevel() == 0)
      {
			  m_apcMCTFDecoder[m_uiLastLayerId]->m_uiDecompositionStages	=	m_uiDecompositionStages[m_uiLastLayerId];
			  m_apcMCTFDecoder[m_uiLastLayerId]->m_uiDecompositionStagesBase	=	m_uiDecompositionStages[0];
      }
//TMM_EC }}
//JVT-T054{
       if(m_bBaseLayerAvcCompliant && m_uiLastLayerId == 0 && m_pcSliceHeader->getQualityLevel() == 1)

      {
         m_apcMCTFDecoder[m_uiLastLayerId]->setAVCBased(true);
      }
      else
      {
        m_apcMCTFDecoder[m_uiLastLayerId]->setAVCBased(false);
      }
//JVT-T054}
      RNOK( m_apcMCTFDecoder[m_uiLastLayerId] ->process     ( m_pcSliceHeader,
                                                              pcPicBuffer,
                                                              rcOutputList,
                                                              rcPicBufferUnusedList,
                                                              bHighestLayer ) );
      RNOK( m_pcNalUnitParser                 ->closeNalUnit() );
      return Err::m_nOK;
    }
    break;

  case NAL_UNIT_CODED_SLICE:
  case NAL_UNIT_CODED_SLICE_IDR:
    {
      ROF( m_pcSliceHeader );
      ROF( m_pcSliceHeader->getLayerId() == 0 );
      m_uiLastLayerId    = m_pcSliceHeader->getLayerId();
//JVT-T054{
      Bool bHighestLayer;
      m_bLastNalInAU = (m_uiNumOfNALInAU == 0);
      if(m_bFGSRefInAU)
        bHighestLayer = ( m_uiLastLayerId == m_uiRecLayerId );
      else
        bHighestLayer = ( m_uiLastLayerId == m_uiRecLayerId && m_bLastNalInAU);
//JVT-T054}
// JVT-Q054 Red. Picture {
      if ( NULL != m_pcSliceHeader_backup )
      {
        RNOK( m_pcSliceHeader->sliceHeaderBackup( m_pcSliceHeader_backup ) );
      }
// JVT-Q054 Red. Picture }

//	TMM EC {{
			m_apcMCTFDecoder[m_uiLastLayerId+1]->m_bBaseLayerLost	=	!m_pcSliceHeader->getTrueSlice();

//			if(eNalUnitType==NAL_UNIT_VIRTUAL_BASELAYER)
			if ( !m_pcSliceHeader->getTrueSlice())
			{
				RNOK( xProcessSliceVirtual( *m_pcSliceHeader, m_pcPrevSliceHeader, pcPicBuffer ) );				
			}
			else
			{
      RNOK( xProcessSlice( *m_pcSliceHeader, 
                          m_pcPrevSliceHeader, 
                          pcPicBuffer, 
                          bHighestLayer ) ); //JVT-T054
			}
//TMM_EC }}

      PicBufferList   cDummyList;
      PicBufferList&  rcOutputList  = ( (!m_bCGSSNRInAU && m_uiRecLayerId == 0 && bHighestLayer) ? rcPicBufferOutputList : cDummyList ); //JVT-T054

      RNOK( m_pcFrameMng->setPicBufferLists( rcOutputList, rcPicBufferReleaseList ) );
//JVT-T054_FIX{
      m_apcMCTFDecoder[m_uiLastLayerId]->setAVCBased(true);
      m_bBaseSVCActive = (bHighestLayer && m_apcMCTFDecoder[0]->isActive());

//JVT-T054}
    }
    break;
  
  case NAL_UNIT_SPS:
    {
      break;
    }


  case NAL_UNIT_PPS:
  case NAL_UNIT_SEI:
	case NAL_UNIT_ACCESS_UNIT_DELIMITER:
	case NAL_UNIT_END_OF_SEQUENCE:
	case NAL_UNIT_END_OF_STREAM:
    {
    }
    break;
  
  default:
    {
      AF();
      return Err::m_nERR;
    }
    break;
  }

  rcPicBufferUnusedList.pushBack( pcPicBuffer );
  RNOK( m_pcNalUnitParser->closeNalUnit() );

  return Err::m_nOK;
}



ErrVal H264AVCDecoder::xStartSlice(Bool& bPreParseHeader, Bool& bFirstFragment, Bool& bLastFragment, Bool* pbFgsParallelDecoding, Bool& bDiscardable) //FRAG_FIX //TMM_EC  //JVT-S036 lsj
{
  //JVT-P031
  SliceHeader * pSliceHeader = NULL;

  bFirstFragment = true;
  bLastFragment = false;

  if(m_pcNalUnitParser->getDiscardableFlag() == false || m_pcNalUnitParser->getLayerId() == m_uiDecodedLayer)
  {
    //TMM_EC {{
      //UInt uiPPSId = (m_pcSliceHeader != NULL) ? m_pcSliceHeader->getPicParameterSetId() : 0;
    UInt uiNumMbsInSlice = (m_pcSliceHeader != NULL) ? m_pcSliceHeader->getNumMbsInSlice() : 0;
    Bool bFGSCompSep = (m_pcSliceHeader != NULL) ? m_pcSliceHeader->getFgsComponentSep() : 0;
		if ( !m_pcNalUnitParser->isTrueNalUnit())
		{
      
      SliceHeader*    rpcSliceHeader=NULL;
      if(m_pcNalUnitParser->getNalUnitType()==NAL_UNIT_CODED_SLICE_SCALABLE && m_uiNextLayerId==0)
      {
        rpcSliceHeader=m_apcMCTFDecoder[0]->m_pcVeryFirstSliceHeader;
      }
      else if(m_pcNalUnitParser->getNalUnitType()==NAL_UNIT_CODED_SLICE_SCALABLE)
      {
        rpcSliceHeader=m_apcMCTFDecoder[1]->m_pcVeryFirstSliceHeader;
      }
      else
      {
        rpcSliceHeader=m_pcVeryFirstSliceHeader;
      }
      
			RNOK( m_pcSliceReader->readSliceHeaderVirtual(m_pcNalUnitParser->getNalUnitType(),
																										rpcSliceHeader,
																										m_uiDecompositionStages[m_uiNextLayerId],
																										m_uiMaxDecompositionStages,
																										m_uiGopSize[m_uiNextLayerId],
																										m_uiMaxGopSize,
																										*(Short*)(&m_pcNalUnitParser->m_pucBuffer[4]),
																										*(Short*)(&m_pcNalUnitParser->m_pucBuffer[6]),
																										m_pcNalUnitParser->m_pucBuffer[8],
                                                    m_uiNextLayerId,   
																										pSliceHeader) );
			pSliceHeader->setTrueSlice( false);
		}
		else
		{
				RNOK( m_pcSliceReader->readSliceHeader( m_pcNalUnitParser,
																						    pSliceHeader,
																						    uiNumMbsInSlice,
																						    bFGSCompSep
																						));
				pSliceHeader->setTrueSlice( true);
		}
//TMM_EC {{
    
    SliceHeader* pSH = NULL;
   // if(m_pcNalUnitParser->getNalUnitType()==NAL_UNIT_CODED_SLICE_SCALABLE && m_uiNextLayerId==1 && !m_bBaseLayerIsAVCCompatible && (m_apcMCTFDecoder[0]->m_bBaseLayerLost || m_apcMCTFDecoder[1]->m_bBaseLayerLost))
   if(m_pcNalUnitParser->getNalUnitType()==NAL_UNIT_CODED_SLICE_SCALABLE && m_uiNextLayerId==1 && !m_bBaseLayerIsAVCCompatible)
    {
      pSH=m_apcMCTFDecoder[0]->m_pcVeryFirstSliceHeader;
    }
    else
    {
      pSH = m_pcSliceHeader;
    }
//    if (bPreParseHeader && pSH != NULL && (pSH->getPicOrderCntLsb() != pSliceHeader->getPicOrderCntLsb() || !pSH->getTrueSlice()) && pSliceHeader->getSliceType() == F_SLICE && pSH->getSliceType() != F_SLICE)
 
		if (bPreParseHeader && m_pcSliceHeader != NULL && (m_pcSliceHeader->getPicOrderCntLsb() != pSliceHeader->getPicOrderCntLsb() || !m_pcSliceHeader->getTrueSlice()) && pSliceHeader->getSliceType() == F_SLICE && m_pcSliceHeader->getSliceType() != F_SLICE)
		{
			delete pSliceHeader; 
			pSliceHeader = NULL;
			bLastFragment = true;
			bPreParseHeader	=	false;
			bDiscardable	=	true;
			return Err::m_nOK;
		}
//TMM_EC }}
    if( (!pSliceHeader->getFragmentedFlag()) || ( (pSliceHeader->getFragmentedFlag()) && (pSliceHeader->getFragmentOrder() == 0) ) )
    {
      if( pSliceHeader->getFragmentedFlag() && pSliceHeader->getFragmentOrder() == 0 
        && pbFgsParallelDecoding != 0 )
        *pbFgsParallelDecoding  = pSliceHeader->getFGSCycleAlignedFragment();

      if(bPreParseHeader) //FRAG_FIX
      {
// JVT-Q054 Red. Picture {
        if ( isRedundantPic() )
        {
          delete m_pcSliceHeader;
          m_pcSliceHeader = pSliceHeader;
        }
        else
        {
           //EIDR bug-fix
		   if(m_pcSliceHeader)
         {
          //delete m_pcPrevSliceHeader; //TMM_INTERLACE
          m_pcPrevSliceHeader = m_pcSliceHeader;
         }
          m_pcSliceHeader     = pSliceHeader;
        }
        
        m_uiLastFragOrder = 0;
// JVT-Q054 Red. Picture }
      } // FRAG_FIX
      else // memory leak fix provided by Nathalie
      {
        delete pSliceHeader; 
        pSliceHeader = NULL;
      }
    }
    else
    {
      // just ensure that fragmented information are correct
      if(pSliceHeader->getFragmentOrder() != m_uiLastFragOrder+1)
      {
        printf("pb with fragment ordering information\n");
      }

      m_uiLastFragOrder = pSliceHeader->getFragmentOrder();

      // set current slice header with last fragment info, in order to start the decoding process
      bFirstFragment = false;
      if( pSliceHeader->getLastFragmentFlag() )
      bLastFragment = true;
      else
        bLastFragment = false;

      delete pSliceHeader;
      pSliceHeader = NULL;
    }
  }
  else
  {
    // set current slice header with last fragment info, in order to start the decoding process
    bLastFragment = true;
  }
  //~JVT-P031
 
  return Err::m_nOK;
}

//JVT-T054_FIX{
ErrVal
H264AVCDecoder::getAVCFrame( IntFrame*&      pcFrame,
                                  IntFrame*&      pcResidual,
                                  MbDataCtrl*&    pcMbDataCtrl,
                                  Int             iPoc)
{
  FrameUnit*  pcFrameUnit = m_pcFrameMng->getReconstructedFrameUnit( iPoc );
  ROF( pcFrameUnit );
  pcFrame             = m_pcFrameMng->getRefinementIntFrame();
  pcResidual          = pcFrameUnit ->getResidual();
  pcMbDataCtrl        = m_pcRQFGSDecoder->getMbDataCtrl();
  
  return Err::m_nOK;
}
//JVT-T054}

ErrVal  
H264AVCDecoder::replaceSNRCGSBaseFrame( IntFrame* pcELFrame ) // MGS fix by Heiko Schwarz
{
  ROFRS ( m_bCGSSNRInAU, Err::m_nOK );
  RNOK  ( m_pcFrameMng->updateLastFrame( pcELFrame ) );
  return Err::m_nOK;
}

ErrVal
H264AVCDecoder::xZeroIntraMacroblocks( IntFrame*    pcFrame,
                                       MbDataCtrl*  pcMbDataCtrl,
                                       SliceHeader* pcSliceHeader )
{
	SliceHeader    *  pcSliceHeaderV    = pcMbDataCtrl->getSliceHeader();//TMM_EC
  RNOK( pcMbDataCtrl->initSlice( *pcSliceHeader, PRE_PROCESS, false, NULL ) );

  IntYuvMbBuffer cZeroMbBuffer;
  cZeroMbBuffer.setAllSamplesToZero();

  	const PicType ePicType = pcSliceHeader->getPicType();
	const Bool    bMbAff   = pcSliceHeader->isMbAff   ();
	if( ePicType!=FRAME )
	{
		RNOK( pcFrame->addFieldBuffer     ( ePicType ) );
	}
	else if( bMbAff )
	{
		RNOK( pcFrame->addFrameFieldBuffer() );
	}

	//===== loop over macroblocks =====
  const UInt uiMbNumber = pcSliceHeader->getMbInPic();
	for(UInt uiMbAddress = 0 ; uiMbAddress < uiMbNumber; uiMbAddress++ )
  {
		MbDataAccess* pcMbDataAccess = NULL;
		UInt          uiMbY, uiMbX;

		pcSliceHeader->getMbPositionFromAddress ( uiMbY, uiMbX, uiMbAddress               );

		RNOK( m_pcControlMng->initMbForFiltering( *pcMbDataAccess, uiMbY, uiMbX, bMbAff ) );
    RNOK( pcMbDataCtrl->initMb( pcMbDataAccess, uiMbY, uiMbX ) );


    if( pcMbDataAccess->getMbData().isIntra() )
    {
			const PicType eMbPicType = pcMbDataAccess->getMbPicType();
      pcFrame->getPic( eMbPicType )->getFullPelYuvBuffer()->loadBuffer( &cZeroMbBuffer );
    }

  }
//TMM_EC{{ bug fix
	if(!pcSliceHeaderV->getTrueSlice()){
		RNOK(pcMbDataCtrl->initSlice(*pcSliceHeaderV,POST_PROCESS,false, NULL));}
//TMM_EC}} bug fix

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
H264AVCDecoder::xProcessSlice( SliceHeader& rcSH,
                               SliceHeader* pcPrevSH,
                               PicBuffer*&  rpcPicBuffer,
                               Bool         bHighestLayer) //JVT-T054
{
  UInt  uiMbRead;
  Bool  bNewFrame = false;
  Bool  bNewPic   = false; 
  Bool  bVeryFirstSlice=false;  

  //===== calculate Poc =====
  if( rcSH.getLayerId() == 0 )
  {
    RNOK( m_pcPocCalculator->calculatePoc( rcSH ) );
  }

  //===== store prediction weights table for inter-layer prediction =====
  m_acLastPredWeightTable[LIST_0].copy( rcSH.getPredWeightTable( LIST_0 ) );
  m_acLastPredWeightTable[LIST_1].copy( rcSH.getPredWeightTable( LIST_1 ) );

  //===== set some basic parameters =====
  if( NULL == m_pcVeryFirstSliceHeader )
  {
     m_pcVeryFirstSliceHeader = new SliceHeader( rcSH );
    //--ICU/ETRI FMO Implementation
    m_pcVeryFirstSliceHeader->setSliceGroupChangeCycle( rcSH.getSliceGroupChangeCycle() ); // fix HS
    m_pcVeryFirstSliceHeader->FMOInit();  
    bVeryFirstSlice= true;

		// ICU/ETRI FGS_MOT_USE
    m_pcBaseLayerCtrlEL = new MbDataCtrl();
    m_pcBaseLayerCtrlEL->init(rcSH.getSPS());
  }
	else if ( m_bNewSPS && (rcSH.getSPS().getProfileIdc() != SCALABLE_PROFILE) )
	{
		delete m_pcVeryFirstSliceHeader;
		m_pcVeryFirstSliceHeader  = new SliceHeader( rcSH );
    m_pcVeryFirstSliceHeader->setSliceGroupChangeCycle( rcSH.getSliceGroupChangeCycle() ); // fix HS
    m_pcVeryFirstSliceHeader->FMOInit();  // fix HS
	}
	m_bNewSPS = false;
  

  //===== check if new pictures and initialization =====
	rcSH.compare( pcPrevSH, bNewPic, bNewFrame );

	if( rcSH.getBottomFieldFlag())
	{
 	  bNewFrame = false;
	  bNewPic = true;
	}

  if( bNewFrame || m_bFrameDone )
  {
    RNOK( m_pcFrameMng->initFrame( rcSH, rpcPicBuffer ) );
    rpcPicBuffer = NULL;
  }
  if( bNewPic )
  {
    RNOK( m_pcFrameMng->initPic  ( rcSH ) );
  }
  else
  {
    rcSH.setFrameUnit( pcPrevSH->getFrameUnit() );
  }


  //===== set reference lists =====
  RNOK( m_pcFrameMng->setRefPicLists( rcSH, false ) );


  //===== parse slice =====
  RNOK( m_pcControlMng  ->initSlice ( rcSH, PARSE_PROCESS ) );
  RNOK( m_pcSliceReader ->process   ( rcSH, uiMbRead ) );
  rcSH.setNumMbsInSlice( uiMbRead );

  //===== decode slice =====
  Bool  bUseBaseRep     = rcSH.getUseBasePredictionFlag();
  Bool  bConstrainedIP  = rcSH.getPPS().getConstrainedIntraPredFlag();
  Bool  bReconstruct    = (m_uiRecLayerId == 0) || ! bConstrainedIP || bHighestLayer; //JVT-T054
  m_bReconstruct  = bReconstruct;
  //***** NOTE: Motion-compensated prediction for non-key pictures is done in xReconstructLastFGS()
  bReconstruct    = bReconstruct && bUseBaseRep || ! bConstrainedIP;

  RNOK( m_pcControlMng  ->initSlice ( rcSH, DECODE_PROCESS ) );
  RNOK( m_pcSliceDecoder->process   ( rcSH, bReconstruct, uiMbRead ) );
	const PicType ePicType = rcSH.getPicType();

  Bool bPicDone;
  RNOK( m_pcControlMng->finishSlice( rcSH, bPicDone, m_bFrameDone ) );

  if( bPicDone )
  {
    // copy intra and inter prediction signal
		RNOK( m_pcFrameMng->getPredictionIntFrame()->copy( rcSH.getFrameUnit()->getFGSIntFrame(), ePicType                              ) );
    // delete intra prediction
    RNOK( m_pcControlMng->initSlice( rcSH, POST_PROCESS));
    RNOK( xZeroIntraMacroblocks( rcSH.getFrameUnit()->getFGSIntFrame(), rcSH.getFrameUnit()->getMbDataCtrl(), &rcSH ) );
    // add residual and intra signal
		rcSH.getFrameUnit()->getFGSIntFrame()->add ( rcSH.getFrameUnit()->getResidual   (), ePicType                                      );
    rcSH.getFrameUnit()->getFGSIntFrame()->clip();

    RNOK( xZeroIntraMacroblocks( rcSH.getFrameUnit()->getResidual(),
                                 rcSH.getFrameUnit()->getMbDataCtrl(), m_pcVeryFirstSliceHeader ) );

    //===== deblocking of base representation =====
    RNOK( m_pcLoopFilter->process( rcSH) );
    RNOK( m_pcFrameMng->storePicture( rcSH ) );

    //===== init FGS decoder =====
    RNOK( m_pcRQFGSDecoder->initPicture( &rcSH, rcSH.getFrameUnit()->getMbDataCtrl() ) );
   }
  
//TMM_EF_01{
//	  printf("  %s POC %4d ( LId 0, TL X, QL 0, AVC-%c, BId-1, AP 0, QP%3d )\n",
//		(ePicType==FRAME) ?  "FRAME" : ( (ePicType==TOP_FIELD) ? "TOP_FIELD" : "BOT_FIELD"),
	  printf("  %s %4d ( LId 0, TL X, QL 0, AVC-%c, BId 1, AP 0, QP%3d )\n",
		(ePicType==FRAME) ?  "Frame" : ( (ePicType==TOP_FIELD) ? "TopFd" : "BotFd"),
//TMM_EF_01}
		rcSH.getPoc                (),
		rcSH.getSliceType              () == B_SLICE ? 'B' : 
	  rcSH.getSliceType              () == P_SLICE ? 'P' : 'I',
		rcSH.getPicQp                  () );

  if( m_bFrameDone )
  {
    DTRACE_NEWFRAME;
  }

  return Err::m_nOK;
}


//TMM_EC {{
ErrVal
H264AVCDecoder::xProcessSliceVirtual( SliceHeader&    rcSH,
                                      SliceHeader*    pcPrevSH,
                                      PicBuffer* &    rpcPicBuffer)
{
  UInt  uiMbRead;
  Bool  bNewFrame = false;
  Bool  bNewPic   = false; 

   //===== calculate Poc =====
  if( rcSH.getLayerId() == 0 )
  {
    RNOK( m_pcPocCalculator->calculatePoc( rcSH ) );
  }

  //===== store prediction weights table for inter-layer prediction =====
  m_acLastPredWeightTable[LIST_0].copy( rcSH.getPredWeightTable( LIST_0 ) );
  m_acLastPredWeightTable[LIST_1].copy( rcSH.getPredWeightTable( LIST_1 ) );

  m_uiNumOfNALInAU++;  //TMM_EC
 
  //===== set some basic parameters =====
  if( NULL == m_pcVeryFirstSliceHeader )
  {
    m_pcVeryFirstSliceHeader  = new SliceHeader( rcSH);
  }

 


  //===== check if new pictures and initialization =====
  rcSH.compare( pcPrevSH, bNewPic, bNewFrame );
  
 if( rcSH.getBottomFieldFlag())
	{
 	  bNewFrame = false;
	  bNewPic = true;
	}

  if( bNewFrame || m_bFrameDone )
  {
    RNOK( m_pcFrameMng->initFrame( rcSH, rpcPicBuffer ) );
    rpcPicBuffer = NULL;
  }
  if( bNewPic )
  {
    RNOK( m_pcFrameMng->initPic  ( rcSH ) );
  }
  else
  {
    rcSH.setFrameUnit( pcPrevSH->getFrameUnit() );
  }


  //===== set reference lists =====
  RNOK( m_pcFrameMng->setRefPicLists( rcSH, false ) );


  //===== parse slice =====
  RNOK( m_pcControlMng  ->initSlice ( rcSH, PARSE_PROCESS ) );
  uiMbRead	=	rcSH.getMbInPic();

  //===== decode slice =====
//   Bool  bUseBaseRep     = rcSH.getUseBasePredictionFlag();
  Bool  bUseBaseRep     = 1;
  Bool  bConstrainedIP  = rcSH.getPPS().getConstrainedIntraPredFlag();

  Bool  bReconstruct    = (m_uiRecLayerId == 0) || ! bConstrainedIP;

  m_bReconstruct = bReconstruct;
  //***** NOTE: Motion-compensated prediction for non-key pictures is done in xReconstructLastFGS()
  bReconstruct   = bReconstruct && bUseBaseRep || ! bConstrainedIP;

  RNOK( m_pcControlMng  ->initSlice ( rcSH, DECODE_PROCESS ) );

  const PicType ePicType = rcSH.getPicType();
	if ( m_eErrorConceal == EC_BLSKIP || m_eErrorConceal == EC_TEMPORAL_DIRECT)

	{
		RNOK( m_pcSliceDecoder->processVirtual( rcSH, bReconstruct, uiMbRead ) );
	}
	else
	{
		Frame	*frame	=	(Frame*)(rcSH.getRefPicList( ePicType, LIST_0 ).get(0).getFrame());
//		m_pcFrameMng->getCurrentFrameUnit()->getFrame().getFullPelYuvBuffer()->loadBuffer( frame->getFullPelYuvBuffer());
		  m_pcFrameMng->getCurrentFrameUnit()->getPic(ePicType)->getFullPelYuvBuffer()->loadBuffer( frame->getFullPelYuvBuffer());
	}

  
  Bool bPicDone;
  RNOK( m_pcControlMng->finishSlice( rcSH, bPicDone, m_bFrameDone ) );

  bPicDone	=	true;
  m_bFrameDone	=	true;
// TMM_EC
  if( bPicDone )
  {
		if (  m_eErrorConceal == EC_FRAME_COPY)
		{
			rcSH.getFrameUnit()->getFGSIntFrame()->copy( m_pcFrameMng->getCurrentFrameUnit()->getPic(ePicType),ePicType);
		}
    // copy intra and inter prediction signal
    m_pcFrameMng->getPredictionIntFrame()->copy( rcSH.getFrameUnit()->getFGSIntFrame(),ePicType);
    // delete intra prediction
    RNOK( m_pcControlMng->initSlice( rcSH, POST_PROCESS));
    RNOK( xZeroIntraMacroblocks( rcSH.getFrameUnit()->getFGSIntFrame(), rcSH.getFrameUnit()->getMbDataCtrl(), &rcSH ) );
    // add residual and intra signal
    rcSH.getFrameUnit()->getFGSIntFrame()->add( rcSH.getFrameUnit()->getResidual(),ePicType );
    rcSH.getFrameUnit()->getFGSIntFrame()->clip();

    RNOK( xZeroIntraMacroblocks( rcSH.getFrameUnit()->getResidual(),
                                 rcSH.getFrameUnit()->getMbDataCtrl(), m_pcVeryFirstSliceHeader ) );

    //===== deblocking of base representation =====
  
  
    RNOK( m_pcFrameMng->storePicture( rcSH ) );
    //===== init FGS decoder =====
//    RNOK( m_pcRQFGSDecoder->initPicture( &rcSH, rcSH.getFrameUnit()->getMbDataCtrl() ) );
  }

 printf("  Frame %4d ( LId 0, TL X, QL 0, AVC-%c, BId-1, AP 0, QP%3d, V )\n",
    rcSH.getPoc                    (),
    rcSH.getSliceType              () == B_SLICE ? 'B' : 
    rcSH.getSliceType              () == P_SLICE ? 'P' : 'I',
    rcSH.getPicQp                  () );


  if( m_bFrameDone )
  {
    DTRACE_NEWFRAME;
  }

  return Err::m_nOK;
}
//TMM_EC }}




ErrVal
H264AVCDecoder::xInitSlice( SliceHeader* pcSliceHeader )
{
  for( UInt uiLayer = 0; uiLayer < MAX_LAYERS; uiLayer++ )
  {
// *LMH: Inverse MCTF
    UInt  uiLastLayer;
    if( m_uiLastLayerId != MSYS_UINT_MAX && m_uiLastLayerId == m_uiRecLayerId )
      uiLastLayer = m_uiRecLayerId;
    else
      uiLastLayer = MAX_LAYERS;
    if( !( m_bBaseLayerAvcCompliant && (m_pcRQFGSDecoder->getSliceHeader() && 
      m_pcRQFGSDecoder->getSliceHeader()->getQualityLevel() == 0 &&
      m_pcRQFGSDecoder->getSliceHeader()->getLayerId() == 0 )) || (pcSliceHeader && pcSliceHeader->getLayerId() != 0) ) //JVT-T054
   

      RNOK( m_apcMCTFDecoder[uiLayer]->initSlice( pcSliceHeader, uiLastLayer, m_bLastNalInAU, m_bCGSSNRInAU ) );
  }
  ROFRS( m_bActive, Err::m_nOK );

  //===== calculate POC =====
  if( pcSliceHeader && pcSliceHeader->getLayerId() == 0 )
  {
    RNOK( m_pcPocCalculator->calculatePoc( *pcSliceHeader ) );
  }

  //===== check if an FGS enhancement needs to be reconstructed =====
  if( m_pcRQFGSDecoder->isInitialized  ()                                                               &&
      m_pcRQFGSDecoder->getSliceHeader ()->getLayerId() == 0                                            &&
    (!pcSliceHeader                                                                                     ||
      pcSliceHeader->getLayerId       () != m_pcRQFGSDecoder->getSliceHeader()->getLayerId      ()      ||
      pcSliceHeader->getQualityLevel  () != m_pcRQFGSDecoder->getSliceHeader()->getQualityLevel () + 1  ||
      pcSliceHeader->getPoc           () != m_pcRQFGSDecoder->getSliceHeader()->getPoc          ()        
      || pcSliceHeader->getSliceType() != F_SLICE //JVT-T054
      ))
  {
    Bool bHighestLayer = ( m_bLastNalInAU ); //JVT-T054
	
    if (NULL == pcSliceHeader || pcSliceHeader->getQualityLevel  () == m_pcRQFGSDecoder->getSliceHeader()->getQualityLevel () + 1 )
    {
      if(bHighestLayer && m_bCGSSNRInAU)
      {
        RNOK( m_apcMCTFDecoder[0]->ReconstructLastFGS(bHighestLayer, m_bCGSSNRInAU ) );
        m_apcMCTFDecoder[0]->getLastDPBUnit()->setMbDataCtrl(0);
      }
      else
      {
        RNOK( xReconstructLastFGS(bHighestLayer) );
      }
    }
    else
    {
      if(pcSliceHeader->getQualityLevel  () != m_pcRQFGSDecoder->getSliceHeader()->getQualityLevel () ||
          0 == pcSliceHeader->getQualityLevel  ())
      {
        RNOK( xReconstructLastFGS(bHighestLayer) );
      }
  	}
  }

  return Err::m_nOK;
}

//JVT-T054{
Void  H264AVCDecoder::setFGSRefInAU(Bool &b)
{ 
  m_bFGSRefInAU = b;
}
//JVT-T054}

ErrVal
H264AVCDecoder::xReconstructLastFGS(Bool bHighestLayer) //JVT-T054
{
  MbDataCtrl*   pcMbDataCtrl        = m_pcRQFGSDecoder->getMbDataCtrl   ();
  SliceHeader*  pcSliceHeader       = m_pcRQFGSDecoder->getSliceHeader  ();
  ROF( pcSliceHeader );
  const PicType ePicType         = pcSliceHeader->getPicType                    ();
  IntFrame* pcResidual           = pcSliceHeader->getFrameUnit()->getResidual   ();
  IntFrame* pcRecFrame           = pcSliceHeader->getFrameUnit()->getFGSIntFrame();
  IntFrame*     pcILPredFrame       = m_pcFrameMng    ->getRefinementIntFrame();
  IntFrame*     pcILPredFrameSpatial= m_pcFrameMng    ->getRefinementIntFrame2();
  Bool          bReconstructFGS     = m_pcRQFGSDecoder->changed();
  Bool          bUseBaseRep         = pcSliceHeader   ->getUseBasePredictionFlag(); // HS: fix by Nokia
  Bool          bConstrainedIP      = pcSliceHeader   ->getPPS().getConstrainedIntraPredFlag();

  //===== reconstruct FGS =====
  if( bReconstructFGS || ! bUseBaseRep && bConstrainedIP )
  {
		RNOK( pcRecFrame->removeFrameFieldBuffer() );
    RNOK( m_pcRQFGSDecoder            ->reconstruct( pcRecFrame, true                                ) );
    RNOK( pcResidual                  ->copy       ( pcRecFrame,                            ePicType ) );
    RNOK( xZeroIntraMacroblocks           ( pcResidual, pcMbDataCtrl, pcSliceHeader ) );

    if( m_bReconstruct )
    {
      if( bUseBaseRep && pcSliceHeader->isInterP() )
      {
        RefFrameList  cRefListDiff;

        setDiffPrdRefLists(cRefListDiff, m_pcFrameMng->getYuvFullPelBufferCtrl() );

        //----- key frames: adaptive motion-compensated prediction -----
        m_pcMotionCompensation->loadAdaptiveRefPredictors(
                                              m_pcFrameMng->getYuvFullPelBufferCtrl(), 
                                              m_pcFrameMng->getPredictionIntFrame(), 
                                              m_pcFrameMng->getPredictionIntFrame(), 
                                              &cRefListDiff, 
                                              pcMbDataCtrl, m_pcRQFGSDecoder, 
                                              m_pcRQFGSDecoder->getSliceHeader());

        freeDiffPrdRefLists(cRefListDiff);
      }
      else if( ! pcSliceHeader->isIntra() )
      {
        //----- "normal" motion-compensated prediction -----
        RNOK( m_pcSliceDecoder->compensatePrediction( *pcSliceHeader ) );
      }
    }

    RNOK( pcRecFrame      ->add           ( m_pcFrameMng->getPredictionIntFrame(), ePicType ) );
    RNOK( pcRecFrame      ->clip          () );
  }
//JVT-S036 lsj start

  if( pcSliceHeader->getUseBasePredictionFlag() )  //bug-fix suffix shenqiu
	{
		if( pcSliceHeader->getAdaptiveRefPicMarkingFlag() )
		{
			RNOK( m_pcFrameMng->xMMCOUpdateBase(pcSliceHeader) );
		}
		else
		{
			RNOK( m_pcFrameMng->xSlidingWindowUpdateBase( pcSliceHeader->getFrameNum() ) );
		}
	}
//JVT-S036 lsj end

  RNOK  ( m_pcRQFGSDecoder->finishPicture () );

  //===== store intra signal for inter-layer prediction =====
  if( m_bEnhancementLayer )
  {
    RNOK( pcILPredFrame->copy( pcRecFrame, ePicType ) );
  }
//TMM_EC {{
  if ( pcSliceHeader->getTrueSlice())
  {
  //===== loop filter =====
  if( (m_uiRecLayerId == 0) || bUseBaseRep || bHighestLayer ) // HS: fix by Nokia //JVT-T054
  {
		RNOK( m_pcLoopFilter->process( *pcSliceHeader, pcRecFrame ) );
  }
  }
//TMM_EC }}
  //===== store in FGS pic buffer =====
  if( bReconstructFGS )
  {
    //===== update DPB =====
		RNOK( m_pcFrameMng->storeFGSPicture( m_pcFGSPicBuffer ) );
    m_pcFGSPicBuffer = NULL;
//JVT-S036 lsj{
	if( pcSliceHeader->getUseBasePredictionFlag() )  //bug-fix suffix shenqiu
		m_pcFrameMng->getCurrentFrameUnit()->uninitBase();
//JVT-S036 lsj}
  }
  else if( ! bUseBaseRep && bConstrainedIP )
  {
    RNOK( m_pcFrameMng->storeFGSPicture( pcSliceHeader->getFrameUnit()->getPicBuffer() ) );
  }

  m_pcFrameMng->RefreshOrderedPocList();  //JVT-S036 lsj



  //===== loop-filter for spatial scalable coding =====
  if( m_bEnhancementLayer )
  {
   RNOK( pcILPredFrameSpatial->copy( pcILPredFrame, ePicType ) );
//TMM_EC {{
    if ( pcSliceHeader->getTrueSlice())
    {
    if( pcSliceHeader->getFrameUnit()->getContrainedIntraPred() )
    {
      m_pcLoopFilter->setFilterMode( LoopFilter::LFMode( LoopFilter::LFM_NO_INTER_FILTER + LoopFilter::LFM_EXTEND_INTRA_SUR ) );
      RNOK( m_pcLoopFilter->process( *m_pcVeryFirstSliceHeader,
                                     pcILPredFrameSpatial,
                                     pcMbDataCtrl,
                                     pcMbDataCtrl,
                                     m_pcVeryFirstSliceHeader->getSPS().getFrameWidthInMbs(),
                                     NULL,
                                     NULL,
									 false,
                                     false ) );  // SSUN@SHARP

      m_pcLoopFilter->setFilterMode();
//  bug fix TMM_EC
    RNOK(pcMbDataCtrl->initSlice(*pcSliceHeader, POST_PROCESS, false, NULL));
// end bug fix TMM_EC
      }
      else
      {
      RNOK( m_pcLoopFilter->process( *pcSliceHeader, pcILPredFrameSpatial ) );
      }
    }
//  TMM_EC }}
  }

  return Err::m_nOK;
}


ErrVal
H264AVCDecoder::xDecodeFGSRefinement( SliceHeader*& rpcSliceHeader, PicBuffer*& rpcPicBuffer )
{

  ROFS( m_pcRQFGSDecoder->isInitialized() );

  //===== check slice header =====
	if(!m_pcRQFGSDecoder->isFinished    ()                                                                 &&
		m_pcRQFGSDecoder->getSliceHeader()->getPoc          ()      == rpcSliceHeader->getPoc           () &&
		(  m_pcRQFGSDecoder->getSliceHeader()->getQualityLevel () + 1  == rpcSliceHeader->getQualityLevel  ()  ||
		   m_pcRQFGSDecoder->getSliceHeader()->getQualityLevel () == rpcSliceHeader->getQualityLevel  ()
	    )
	  )
  {
		const PicType ePicType = rpcSliceHeader->getPicType();
    if( rpcSliceHeader->getQualityLevel() <= m_uiQualityLevelForPrediction )
    {
      printf("  FGS  POC %4d ( LId%2d, TL%2d, QL%2d, PrRef,        MR %d, QP%3d, %s )\n",
        rpcSliceHeader->getPoc                    (),
        rpcSliceHeader->getLayerId                (),
        rpcSliceHeader->getTemporalLevel          (),
        rpcSliceHeader->getQualityLevel           (),
        rpcSliceHeader->getAdaptivePredictionFlag (),
        rpcSliceHeader->getPicQp                  (),
        (ePicType==FRAME) ?  "FRAME" : ( (ePicType==TOP_FIELD) ? "TOP_FIELD" : "BOT_FIELD") );

      //===== set mem when first FGS layer ====
      if( rpcSliceHeader->getQualityLevel() == 1 )
      {
		if (m_pcRQFGSDecoder->isFirstFGS())
		{
          ROT( m_pcFGSPicBuffer );
          ROF( rpcPicBuffer );

          m_pcFGSPicBuffer  = rpcPicBuffer;
          rpcPicBuffer      = 0;
		  m_pcRQFGSDecoder->SetIsFirstFGS(false);

		  if( rpcSliceHeader->getTemporalLevel()  == 0  &&
			  m_pcRQFGSDecoder->getSliceHeader()->isInterP() )
			    m_pcRQFGSDecoder->SetIsFirstFGS(true);
		}
      }

      if( rpcSliceHeader->getTemporalLevel()  == 0      &&
        m_pcRQFGSDecoder->getSliceHeader()->isInterP())
      {
        // m_pcRQFGSDecoder->getSliceHeader has the slice header of the base layer
        m_pcRQFGSDecoder->getSliceHeader()->setBaseWeightZeroBaseBlock(rpcSliceHeader->getBaseWeightZeroBaseBlock() );
        m_pcRQFGSDecoder->getSliceHeader()->setBaseWeightZeroBaseCoeff(rpcSliceHeader->getBaseWeightZeroBaseCoeff() );
        m_pcRQFGSDecoder->getSliceHeader()->setLowPassFgsMcFilter     (rpcSliceHeader->getLowPassFgsMcFilter() );
        m_pcRQFGSDecoder->getSliceHeader()->setArFgsUsageFlag         (rpcSliceHeader->getArFgsUsageFlag() );

        if( rpcSliceHeader->getQualityLevel()   == 1 )
        {
		  if (m_pcRQFGSDecoder->isFirstFGS())
		  {
            m_pcRQFGSDecoder->getMbDataCtrl()->storeFgsBQLayerQpAndCbp();
            m_pcRQFGSDecoder->xStoreBQLayerSigMap();
		    m_pcRQFGSDecoder->SetIsFirstFGS(false);	
		  }	// end if isFirstFGS
        }
      }

      RNOK( m_pcRQFGSDecoder->decodeNextLayer( rpcSliceHeader ) );
    }

		// ICU/ETRI FGS_MOT_USE
		// 2006.10.02 ICU/ETRI FGS_MOT_USE Bug Fix
	  if (!m_pcRQFGSDecoder->getSliceHeader()->isIntra() && m_pcRQFGSDecoder->getSliceHeader()->getQualityLevel() == m_uiQualityLevelForPrediction)
	  {		  
		  m_pcBaseLayerCtrlEL->copyMotion(*(m_pcRQFGSDecoder->getMbDataCtrlEL()));
	    m_pcBaseLayerCtrlEL->SetMbStride(m_pcRQFGSDecoder->getMbDataCtrlEL()->GetMbStride());
	    m_pcBaseLayerCtrlEL->xSetDirect8x8InferenceFlag(m_pcRQFGSDecoder->getMbDataCtrlEL()->xGetDirect8x8InferenceFlagPublic());	
	  }


    //===== switch slice headers and update =====
    SliceHeader*  pcTemp  = m_pcSliceHeader;
    m_pcSliceHeader       = m_pcPrevSliceHeader;
    m_pcPrevSliceHeader   = pcTemp;
  }

  return Err::m_nOK;
}


ErrVal
H264AVCDecoder::setDiffPrdRefLists( RefFrameList& diffPrdRefList, 
                                    YuvBufferCtrl* pcYuvFullPelBufferCtrl )
{
  SliceHeader* pcSH = m_pcRQFGSDecoder->getSliceHeader();
  ROTRS( pcSH->isIntra(),   Err::m_nOK );
  const PicType ePicType = pcSH->getPicType();

  RefPicList<RefPic>& rcBaseList = pcSH->getRefPicList(ePicType,  LIST_0 );

  for(UInt i=0; i< rcBaseList.size(); i++)
  {
    Int iRefPoc;

    IntFrame* pcDiffFrame;
    ROFS   ( ( pcDiffFrame                  = new IntFrame( *pcYuvFullPelBufferCtrl,
                                                           *pcYuvFullPelBufferCtrl ) ) );
    pcDiffFrame->init();

    iRefPoc = rcBaseList.get(i).getFrame()->getPoc();

    FrameUnit* pcRefFrameUnit     = m_pcFrameMng->getReconstructedFrameUnit( iRefPoc );

// JVT-Q065 EIDR{
	if(!pcRefFrameUnit)
	{
		break;
	}
// JVT-Q065 EIDR}

    IntFrame* baseFrame = pcRefFrameUnit->getFGSReconstruction(0);
    IntFrame* enhFrame = pcRefFrameUnit->getFGSIntFrame();

    pcDiffFrame->subtract(enhFrame, baseFrame);
    RNOK( pcDiffFrame->extendFrame( NULL ) );
    RNOK( diffPrdRefList.add( pcDiffFrame ) );
  }

  return Err::m_nOK;
}


ErrVal
H264AVCDecoder::freeDiffPrdRefLists( RefFrameList& diffPrdRefList)
{
  for(UInt i=0; i< diffPrdRefList.getSize(); i++)
  {
    diffPrdRefList.getEntry(i)->uninit();
    free(diffPrdRefList.getEntry(i));
  }

  return Err::m_nOK;
}

// JVT-Q054 Red. Picture {
ErrVal
H264AVCDecoder::checkRedundantPic()
{
  m_bRedundantPic = false;
  if ( m_bFirstSliceHeaderBackup && (NULL != m_pcSliceHeader) )
  {
    ROF( ( m_pcSliceHeader_backup = new SliceHeader ( m_pcSliceHeader->getSPS(), m_pcSliceHeader->getPPS()) ) );
    m_bFirstSliceHeaderBackup = false;
  }
  else
  {
    if ( ( NULL != m_pcSliceHeader ) && ( NULL != m_pcSliceHeader_backup ) )
    {
      const NalUnitType eNalUnitType  = m_pcNalUnitParser->getNalUnitType();
      if ( ( eNalUnitType == NAL_UNIT_CODED_SLICE_SCALABLE ) || ( eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE )
        || ( eNalUnitType == NAL_UNIT_CODED_SLICE ) || ( eNalUnitType == NAL_UNIT_CODED_SLICE_IDR ) )
      {
        Bool  bNewFrame  = true;
        RNOK( m_pcSliceHeader->compareRedPic ( m_pcSliceHeader_backup, bNewFrame ) );
        if (!bNewFrame)
        {
          m_bRedundantPic = true;
        }
      }
    }
  }
  return Err::m_nOK;
}


// JVT-Q054 Red. Picture }

H264AVC_NAMESPACE_END

