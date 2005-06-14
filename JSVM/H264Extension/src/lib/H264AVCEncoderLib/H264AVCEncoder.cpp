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
  m_bInitDone         ( false ),
  m_bTraceEnable      ( false )
{
  ::memset( m_apcMCTFEncoder, 0x00, MAX_LAYERS*sizeof(Void*) );
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
                                    Bool&   rbHalfResolutionBaseLayer,
                                    UInt    uiLayerId,
                                    Int     iPoc )
{
  //===== get spatial resolution ratio =====
  if( m_apcMCTFEncoder[uiLayerId]->getFrameHeight() == 2 * m_apcMCTFEncoder[ruiBaseLayerId]->getFrameHeight() &&
      m_apcMCTFEncoder[uiLayerId]->getFrameWidth () == 2 * m_apcMCTFEncoder[ruiBaseLayerId]->getFrameWidth ()   )
  {
    rbHalfResolutionBaseLayer = true;
  }
  else
  {
    ROF( m_apcMCTFEncoder[uiLayerId]->getFrameHeight() == m_apcMCTFEncoder[ruiBaseLayerId]->getFrameHeight() &&
         m_apcMCTFEncoder[uiLayerId]->getFrameWidth () == m_apcMCTFEncoder[ruiBaseLayerId]->getFrameWidth ()   )

    rbHalfResolutionBaseLayer = false;
  }

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
                                  Bool&         bForCopyOnly,
                                  Bool          bSpatialScalability,
                                  UInt          uiBaseLayerId,
                                  Int           iPoc,
                                  Bool          bMotion )
{
  ROF ( uiBaseLayerId < MAX_LAYERS );
  RNOK( m_apcMCTFEncoder[uiBaseLayerId]->getBaseLayerData( pcFrame, pcResidual, pcMbDataCtrl,
                                                           bForCopyOnly, bSpatialScalability, iPoc, bMotion ) );
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

  ROF( m_pcCodingParameter->getNumberOfLayers() );
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


ErrVal getRateSpecifiers( UInt&   ruiFrameRateUnitNom,
                          UInt&   ruiFrameRateUnitDenom,
                          Double  dFrameRate )
{
  ROT( dFrameRate <= 0.0 );

  UInt  uiMaxDenom      = 30000;
  ruiFrameRateUnitNom   = 0;

  for( ruiFrameRateUnitDenom = 1; ruiFrameRateUnitDenom <= uiMaxDenom; ruiFrameRateUnitDenom++ )
  {
    Double  dNom = dFrameRate * (Double)ruiFrameRateUnitDenom;
    Int     iL   = (Int)floor( dNom );
    Int     iH   = (Int)ceil ( dNom );
    
    if( iL == iH )
    {
      ruiFrameRateUnitNom = iL;
      break;
    }
  }
  ROF( ruiFrameRateUnitNom );
  
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

ErrVal H264AVCEncoder::writeDeadSubstreamSEI(ExtBinDataAccessor* pcExtBinDataAccessor, UInt uiDeltaBytesDS, UInt uiLayer ) 
{
	//===== create message =====
  SEI::DeadSubstreamSEI* pcDeadSubstreamSEI;
  RNOK( SEI::DeadSubstreamSEI::create( pcDeadSubstreamSEI ) );

  //===== set message =====
  pcDeadSubstreamSEI->setDeltaBytesDeadSubstream(uiDeltaBytesDS);
  pcDeadSubstreamSEI->setDependencyId(uiLayer);

  //===== write message =====
  UInt              uiBits = 0;
  SEI::MessageList  cSEIMessageList;
  cSEIMessageList.push_back                       ( pcDeadSubstreamSEI );
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
  RNOK( SEI::ScalableSei::create( pcScalableSEI ) );

  //===== set message =====
  UInt              uiLayer;
  UInt              uiNumLayers           = m_pcCodingParameter->getNumberOfLayers  ();
  LayerParameters&  rcLayer0              = m_pcCodingParameter->getLayerParameters ( 0 );
  LayerParameters&  rcLayerN              = m_pcCodingParameter->getLayerParameters ( uiNumLayers - 1 );
  Bool              bBaseLayerIsAVC       = m_pcCodingParameter->getBaseLayerMode   () > 0;
  UInt              uiMaxHorFrameDimInMB  = ( rcLayerN.getFrameWidth  () + 15 ) / 16;
  UInt              uiMaxVerFrameDimInMB  = ( rcLayerN.getFrameHeight () + 15 ) / 16;
  Double            dInputFrameRate       = rcLayerN.getInputFrameRate();
  UInt              uiMaxDecStages        = rcLayerN.getDecompositionStages();
  UInt              uiAVCTempResStages    = ( m_pcCodingParameter->getBaseLayerMode() > 1 ? rcLayer0.getDecompositionStages() - rcLayer0.getNotCodedMCTFStages() : 1 );
  UInt              uiFrameRateUnitNom;
  UInt              uiFrameRateUnitDenom;
  UInt              auiSpatialResolutionFactors [MAX_LAYERS];
  UInt              auiTemporalResolutionFactors[MAX_LAYERS];
  ::memset( auiSpatialResolutionFactors,  0x00, MAX_LAYERS*sizeof(UInt) );
  ::memset( auiTemporalResolutionFactors, 0x00, MAX_LAYERS*sizeof(UInt) );
  for( uiLayer = uiNumLayers - 1; uiLayer > 0; uiLayer-- )
  {
    UInt uiSpatResDiff = m_pcCodingParameter->getLayerParameters( uiLayer ).getBaseLayerSpatRes();
    UInt uiTempResDiff = m_pcCodingParameter->getLayerParameters( uiLayer ).getBaseLayerTempRes();
    
    for( UInt uiBase = 0; uiBase < uiLayer; uiBase++ )
    {
      auiSpatialResolutionFactors  [uiBase] += uiSpatResDiff;
      auiTemporalResolutionFactors [uiBase] += uiTempResDiff;
    }
  }
  if( bBaseLayerIsAVC )
  {
    auiTemporalResolutionFactors[0] += rcLayer0.getNotCodedMCTFStages();
  }
  RNOK( getRateSpecifiers( uiFrameRateUnitNom, uiFrameRateUnitDenom,
                           dInputFrameRate/(Double)(1<<uiMaxDecStages) ) );

  pcScalableSEI->setMaxHorFrameDimInMB       ( uiMaxHorFrameDimInMB );
  pcScalableSEI->setMaxVerFrameDimInMB       ( uiMaxVerFrameDimInMB );
  pcScalableSEI->setFrameRateUnitNom         ( uiFrameRateUnitNom );
  pcScalableSEI->setFrameRateUnitDenom       ( uiFrameRateUnitDenom );
  pcScalableSEI->setMaxDecStages             ( uiMaxDecStages );
  pcScalableSEI->setNumLayers                ( uiNumLayers );
  pcScalableSEI->setBaseLayerIsAVC           ( bBaseLayerIsAVC );
  pcScalableSEI->setAVCTempResStages         ( uiAVCTempResStages );
  for( uiLayer = 0; uiLayer < uiNumLayers; uiLayer++ )
  {
    pcScalableSEI->setSpatialResolutionFactor  ( uiLayer, auiSpatialResolutionFactors  [uiLayer] );
    pcScalableSEI->setTemporalResolutionFactor ( uiLayer, auiTemporalResolutionFactors [uiLayer] );
  }
  
  //===== write message =====
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
H264AVCEncoder::writeParameterSets( ExtBinDataAccessor* pcExtBinDataAccessor, Bool &rbMoreSets )
{
  if( m_bVeryFirstCall )
  {
    m_bVeryFirstCall = false;

    RNOK( xInitParameterSets() );
    RNOK( xWriteScalableSEI( pcExtBinDataAccessor ) );

    return Err::m_nOK;
  }

    
  UInt uiBits;

  if( ! m_cUnWrittenSPS.empty() )
  {
    RNOK( m_pcNalUnitEncoder->initNalUnit( pcExtBinDataAccessor ) );
    SequenceParameterSet& rcSPS = *m_cUnWrittenSPS.front();
    // Copy simple priority ID mapping into SPS
    rcSPS.setNalUnitExtFlag ( m_pcCodingParameter->getExtendedPriorityId() );
    for ( UInt uiPriId = 0; uiPriId < m_pcCodingParameter->getNumSimplePris(); uiPriId++)
    {
        UInt uiTempLevel, uiLayer, uiQualLevel;
        m_pcCodingParameter->getSimplePriorityMap ( uiPriId, uiTempLevel, uiLayer, uiQualLevel );
        rcSPS.setSimplePriorityMap( uiPriId, uiTempLevel, uiLayer, uiQualLevel );
    }
    rcSPS.setNumSimplePriIdVals( m_pcCodingParameter->getNumSimplePris() );
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
      AOT(1);
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
  UInt  uiNumLayers = m_pcCodingParameter->getNumberOfLayers();

  for( uiLayer = 0; uiLayer < uiNumLayers; uiLayer++ )
  {
    if( apcOriginalPicBuffer[uiLayer] )
    {
      RNOK( xProcessGOP( uiLayer,
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
  UInt  uiNumLayers = m_pcCodingParameter->getNumberOfLayers();

  for( uiLayer = 0; uiLayer < uiNumLayers; uiLayer++ )
  {
    RNOK( xProcessGOP( uiLayer,
                       NULL,
                       NULL,
                       apcPicBufferOutputList[uiLayer],
                       apcPicBufferUnusedList[uiLayer] ) );
  }

  m_cAccessUnitList.emptyNALULists( rcExtBinDataAccessorList );
  
  for( uiLayer = 0; uiLayer < m_pcCodingParameter->getNumberOfLayers(); uiLayer++ )
  {
    RNOK( m_apcMCTFEncoder[uiLayer]->finish( ruiNumCodedFrames, rdHighestLayerOutputRate ) );
  }
  printf("                                   ----------------------------------------\n");
  printf("                                   (*): These PSNR values are not accurate!\n\n");

  return Err::m_nOK;
}



ErrVal
H264AVCEncoder::xProcessGOP( UInt                     uiLayer,
                             PicBuffer*               pcOriginalPicBuffer,
                             PicBuffer*               pcReconstructPicBuffer,
                             PicBufferList&           rcPicBufferOutputList,
                             PicBufferList&           rcPicBufferUnusedList )
{ 
  if( pcOriginalPicBuffer == NULL )
  {
    //===== FINISH ENCODING =====
    if( pcReconstructPicBuffer )
    {
      rcPicBufferUnusedList.push_back( pcReconstructPicBuffer );
    }

    if( m_acOrgPicBufferList[uiLayer].size() )
    {
      RNOK( m_apcMCTFEncoder[uiLayer]->process ( m_cAccessUnitList,
                                                 m_acOrgPicBufferList[uiLayer],
                                                 m_acRecPicBufferList[uiLayer],
                                                 rcPicBufferUnusedList ) );
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
    RNOK( m_apcMCTFEncoder[uiLayer]->process( m_cAccessUnitList,
                                              m_acOrgPicBufferList[uiLayer],
                                              m_acRecPicBufferList[uiLayer],
                                              rcPicBufferUnusedList ) );

    //----- set output list -----
    rcPicBufferOutputList += m_acRecPicBufferList[uiLayer];

    //----- update unused list -----
    rcPicBufferUnusedList += m_acOrgPicBufferList[uiLayer];
    rcPicBufferUnusedList += m_acRecPicBufferList[uiLayer];

    //----- reset orig and reconstructed list -----
    m_acOrgPicBufferList[uiLayer].clear();
    m_acRecPicBufferList[uiLayer].clear();
  }

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
    UInt              uiNumMb             = uiMbY * uiMbX;
    UInt              uiOutFreq           = (UInt)ceil( rcLayerParameters.getOutputFrameRate() );
    UInt              uiMvRange           = m_pcCodingParameter->getMotionVectorSearchParams().getSearchRange() / 4;
    UInt              uiDPBSize           = ( 1 << rcLayerParameters.getDecompositionStages() );
    UInt              uiNumRefPic         = uiDPBSize - ( uiDPBSize > 1 ? 1 : 0 );
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
    pcSPS->setLog2MaxFrameNum                     ( MAX_FRAME_NUM_LOG2 - 4 );
    pcSPS->setLog2MaxPicOrderCntLsb               ( min( 15, uiRequiredPocBits + 2 ) );  // HS: decoder robustness -> value increased by 2
    pcSPS->setNumRefFrames                        ( uiNumRefPic );
    pcSPS->setRequiredFrameNumUpdateBehaviourFlag ( true );
    pcSPS->setFrameWidthInMbs                     ( uiMbX );
    pcSPS->setFrameHeightInMbs                    ( uiMbY );
    pcSPS->setDirect8x8InferenceFlag              ( true  );
    pcSPS->setLowComplxUpdFlag                    ( m_pcCodingParameter->getLowComplxUpdFlag() );
#if MULTIPLE_LOOP_DECODING
    pcSPS->setAlwaysDecodeBaseLayer               ( rcLayerParameters.getInterLayerPredictionMode() > 0 && 
                                                    rcLayerParameters.getDecodingLoops() > 1 );
#endif

    //===== set picture parameter set parameters =====
    pcPPSHP->setNalUnitType                           ( NAL_UNIT_PPS );
    pcPPSHP->setLayerId                               ( rcLayerParameters.getLayerId() );
    pcPPSHP->setEntropyCodingModeFlag                 ( true );
    pcPPSHP->setPicOrderPresentFlag                   ( true );
    pcPPSHP->setNumRefIdxActive( LIST_0               , m_pcCodingParameter->getNumRefFrames() );
    pcPPSHP->setNumRefIdxActive( LIST_1               , m_pcCodingParameter->getNumRefFrames() );
    // heiko.schwarz@hhi.fhg.de: ensures that the PPS QP will be in the valid range (specified QP can be outside that range to force smaller/higher lambdas)
    //pcPPSHP->setPicInitQp                             ( (Int)rcLayerParameters.getBaseQpResidual() );
    pcPPSHP->setPicInitQp                             ( min( 51, max( 0, (Int)rcLayerParameters.getBaseQpResidual() ) ) );
    pcPPSHP->setChomaQpIndexOffset                    ( 0 );
    pcPPSHP->setDeblockingFilterParametersPresentFlag ( ! m_pcCodingParameter->getLoopFilterParams().isDefault() );
    pcPPSHP->setConstrainedIntraPredFlag              ( true );
    pcPPSHP->setTransform8x8ModeFlag                  ( rcLayerParameters.getAdaptiveTransform() > 0 );
    pcPPSHP->setPicScalingMatrixPresentFlag           ( false );
    pcPPSHP->set2ndChromaQpIndexOffset                ( 0 );

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
      pcPPSLP->setTransform8x8ModeFlag                  ( pcPPSHP->getTransform8x8ModeFlag                  ()  );
      pcPPSLP->setPicScalingMatrixPresentFlag           ( pcPPSHP->getPicScalingMatrixPresentFlag           ()  );
      pcPPSLP->set2ndChromaQpIndexOffset                ( pcPPSHP->get2ndChromaQpIndexOffset                ()  );
    }


    //===== initialization using parameter sets =====
    RNOK( m_pcControlMng->initParameterSets( *pcSPS, *pcPPSLP, *pcPPSHP ) );
  }


  //===== set unwritten parameter lists =====
  RNOK( m_pcParameterSetMng->setParamterSetList( m_cUnWrittenSPS, m_cUnWrittenPPS ) );

  return Err::m_nOK;
}



H264AVC_NAMESPACE_END
