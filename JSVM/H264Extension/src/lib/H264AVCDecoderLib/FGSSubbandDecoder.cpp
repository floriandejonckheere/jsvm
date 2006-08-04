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
#include "FGSSubbandDecoder.h"
#include "CabacReader.h"
#include "UvlcReader.h"
#include "BitReadBuffer.h"
#include "MbParser.h"
#include "MbDecoder.h"

#include "H264AVCCommonLib/MbDataCtrl.h"
#include "H264AVCCommonLib/IntFrame.h"
#include "H264AVCCommonLib/IntYuvMbBuffer.h"
#include "H264AVCCommonLib/Transform.h"
#include "H264AVCCommonLib/TraceFile.h"


#include "H264AVCCommonLib/CFMO.h"



H264AVC_NAMESPACE_BEGIN




RQFGSDecoder::RQFGSDecoder()
: m_bPicChanged               ( false )
, m_bPicFinished              ( false )
, m_pcSymbolReader            ( 0 )
, m_pcUvlcReader              ( 0 )
, m_pcCabacReader             ( 0 )
, m_pcCurrSliceHeader         ( 0 )
, m_pcMbParser                ( 0 )
, m_pcMbDecoder               ( 0 )

, m_bFirstFGS				  ( true )
{
  ::memset( m_apaucLumaCoefMap,       0x00,   16*sizeof(UChar*) );
  ::memset( m_aapaucChromaDCCoefMap,  0x00, 2* 4*sizeof(UChar*) );
  ::memset( m_aapaucChromaACCoefMap,  0x00, 2*16*sizeof(UChar*) );
  ::memset( m_apaucChromaDCBlockMap,  0x00,    2*sizeof(UChar*) );
  ::memset( m_apaucChromaACBlockMap,  0x00,    2*sizeof(UChar*) );
  m_paucBlockMap        = 0;
  m_paucSubMbMap        = 0;
  m_pauiMacroblockMap   = 0;

	// ICU/ETRI FGS_MOT_USE
  for (int i = 0; i < 8; ++i) m_bFGSMotionUse[i] = false;
}


RQFGSDecoder::~RQFGSDecoder()
{
  AOT( m_bInit );
}


ErrVal
RQFGSDecoder::create( RQFGSDecoder*& rpcRQFGSDecoder )
{
  rpcRQFGSDecoder = new RQFGSDecoder;
  ROT( NULL == rpcRQFGSDecoder );
  return Err::m_nOK;
}
  

ErrVal
RQFGSDecoder::destroy()
{
  delete this;
  return Err::m_nOK;
}


ErrVal
RQFGSDecoder::init( YuvBufferCtrl** apcYuvFullPelBufferCtrl,
                    Transform*      pcTransform,
                    MbParser*       pcMbParser,
                    MbDecoder*      pcMbDecoder,
                    UvlcReader*     pcUvlcReader,
                    CabacReader*    pcCabacReader )
{
  ROT( m_bInit );
  ROF( apcYuvFullPelBufferCtrl );
  ROF( pcTransform );
  ROF( pcUvlcReader );
  ROF( pcCabacReader );
  ROF( pcMbParser );
  ROF( pcMbDecoder );

  m_pcCabacReader             = pcCabacReader;
  m_bInit                     = true;
  m_pcUvlcReader              = pcUvlcReader;
  m_pcMbParser                = pcMbParser;
  m_pcMbDecoder               = pcMbDecoder;

  m_pcCurrSliceHeader         = 0;
  m_bPicChanged               = false;
  m_bPicFinished              = false;

  xInit( apcYuvFullPelBufferCtrl, pcTransform );

  return Err::m_nOK;
}
  

ErrVal
RQFGSDecoder::uninit()
{
  ROT( m_bPicInit );
  ROF( m_bInit );
  
  xUninit();

  m_pcMbParser                = 0;
  m_pcMbDecoder               = 0;
  m_pcCabacReader             = 0;
  m_pcSymbolReader            = 0;
  m_pcUvlcReader              = 0;
  m_pcCurrSliceHeader         = 0;
  m_bPicChanged               = false;
  m_bPicFinished              = false;

  return Err::m_nOK;
}



ErrVal
RQFGSDecoder::initPicture( SliceHeader* pcSliceHeader,
                           MbDataCtrl*  pcCurrMbDataCtrl )
{
  ROT( m_bPicInit );
  ROF( m_bInit );
  ROF( pcSliceHeader );
  ROF( pcCurrMbDataCtrl );

  RNOK( xInitSPS( pcSliceHeader->getSPS() ) );

  m_uiWidthInMB       = pcSliceHeader->getSPS().getFrameWidthInMbs  ();
  m_uiHeightInMB      = pcSliceHeader->getSPS().getFrameHeightInMbs ();
  m_pcCurrSliceHeader = pcSliceHeader;
  m_pcCurrMbDataCtrl  = pcCurrMbDataCtrl;
  m_bPicInit          = true;
  m_bPicChanged       = false;
  m_bPicFinished      = false;

  Bool bCabac         = pcSliceHeader->getPPS().getEntropyCodingModeFlag();
  m_pcSymbolReader    = ( bCabac ) ? (MbSymbolReadIf*)m_pcCabacReader : (MbSymbolReadIf*)m_pcUvlcReader;

  RNOK( xInitBaseLayerSbb( m_pcCurrSliceHeader->getLayerId() ) );
  RNOK( xInitializeCodingPath(pcSliceHeader) );
  RNOK( xScaleBaseLayerCoeffs() );

  return Err::m_nOK;
}


ErrVal
RQFGSDecoder::finishPicture()
{
  ROF( m_bPicInit );

  m_bPicInit          = false;
  m_bPicChanged       = false;
  m_bPicFinished      = false;
  m_uiWidthInMB       = 0;
  m_uiHeightInMB      = 0;
  m_pcCurrSliceHeader = 0;
  m_pcCurrMbDataCtrl  = 0;

  m_bFirstFGS		  = true;

  return Err::m_nOK;
}



ErrVal
RQFGSDecoder::decodeNextLayer( SliceHeader* pcSliceHeader )
{
  ROF( m_bInit );
  ROF( m_bPicInit );
  
  //===== update slice header =====
  m_pcCurrSliceHeader->setSliceHeaderQp ( pcSliceHeader->getPicQp()          );
  m_pcCurrSliceHeader->setFirstMbInSlice( pcSliceHeader->getFirstMbInSlice() );
  m_pcCurrSliceHeader->setNumMbsInSlice ( pcSliceHeader->getNumMbsInSlice () );
  // JVT-S054 (2) (ADD)
  m_pcCurrSliceHeader->setLastMbInSlice ( pcSliceHeader->getLastMbInSlice () );
  m_pcCurrSliceHeader->setAdaptivePredictionFlag( pcSliceHeader->getAdaptivePredictionFlag() );
  m_bPicChanged = true;

  RNOK( xDecodingFGS(pcSliceHeader) );

  return Err::m_nOK;
}




ErrVal
RQFGSDecoder::reconstruct( IntFrame*  pcRecResidual )
{
  ROF( m_bInit );
  ROF( m_bPicInit );
  ROF( pcRecResidual );

  UInt            uiLayer         = m_pcCurrSliceHeader->getLayerId();
  YuvBufferCtrl*  pcYuvBufferCtrl = m_papcYuvFullPelBufferCtrl[uiLayer];
  IntYuvMbBuffer  cMbBuffer;

  RNOK( m_pcCurrMbDataCtrl->initSlice ( *m_pcCurrSliceHeader, PRE_PROCESS, true, NULL ) );

  for( UInt uiMbY = 0; uiMbY < m_uiHeightInMB; uiMbY++ )
  for( UInt uiMbX = 0; uiMbX < m_uiWidthInMB;  uiMbX++ )
  {
    MbDataAccess* pcMbDataAccess = 0;
    RNOK( m_pcCurrMbDataCtrl->initMb( pcMbDataAccess, uiMbY, uiMbX ) );
    RNOK( pcYuvBufferCtrl   ->initMb( uiMbY, uiMbX ) );
    RNOK( xReconstructMacroblock    ( *pcMbDataAccess, cMbBuffer    ) );
    RNOK( pcRecResidual->getFullPelYuvBuffer()->loadBuffer( &cMbBuffer ) );
  }

  return Err::m_nOK;
}




ErrVal
RQFGSDecoder::xScaleBaseLayerCoeffs()
{
  RNOK( m_pcCurrMbDataCtrl->initSlice( *m_pcCurrSliceHeader, PRE_PROCESS, true, NULL ) );

  for( UInt uiMbY = 0; uiMbY < m_uiHeightInMB; uiMbY++ )
  for( UInt uiMbX = 0; uiMbX < m_uiWidthInMB;  uiMbX++ )
  {
    MbDataAccess* pcMbDataAccess = 0;

    RNOK( m_pcCurrMbDataCtrl->initMb(  pcMbDataAccess, uiMbY, uiMbX ) );
    RNOK( xScaleTCoeffs             ( *pcMbDataAccess, true ) );
  }
  
  return Err::m_nOK;
}

UInt gauiB8x8Mapping[4] = { 0, 2, 3, 1 }; 

ErrVal
RQFGSDecoder::xDecodeLumaCbpVlc(UInt  uiCurrMbIdxX,
                                UInt  uiCurrMbIdxY)
{
  UInt uiLumaCbpBase, uiLumaCbp;
  MbDataAccess* pcMbDataAccessEL  = 0;
  MbDataAccess* pcMbDataAccessBL  = 0;

  RNOK( m_pcCurrMbDataCtrl ->initMb( pcMbDataAccessBL, uiCurrMbIdxY, uiCurrMbIdxX ) );
  RNOK( m_cMbDataCtrlEL     .initMb( pcMbDataAccessEL, uiCurrMbIdxY, uiCurrMbIdxX ) );

  uiLumaCbpBase = pcMbDataAccessBL->getMbData().getMbCbp() & 0x0F;
  uiLumaCbp     = uiLumaCbpBase;

  for( UInt uiB8x8 = 0; uiB8x8 < 4; uiB8x8 ++ )
  {
    UInt uiCbpFlagBase = (uiLumaCbpBase >> gauiB8x8Mapping[uiB8x8]) & 1;

    if( uiCbpFlagBase == 0 )
    {
      if( m_uiLumaCbpRun == 0 )
      {
        // read next run
        ((UvlcReader *) m_pcSymbolReader)->getUvlc(m_uiLumaCbpRun, "Luma_CBP_run");
        m_uiLumaCbpRun ++;
        m_bLastLumaCbpFlag = ! m_bLastLumaCbpFlag;
      }

      uiLumaCbp |= m_bLastLumaCbpFlag << gauiB8x8Mapping[uiB8x8];
      m_uiLumaCbpRun --;
    }
  }

  pcMbDataAccessEL->getMbData().setMbCbp(uiLumaCbp);

  return Err::m_nOK;
}

// uiLastChromaCbp, bTransitionFlag
UInt auiNextChromaCbp[3][2] = { { 1, 2 }, { 2, 0 }, { 0, 1 } };

ErrVal
RQFGSDecoder::xDecodeChromaCbpVlc(UInt  uiCurrMbIdxX,
                                  UInt  uiCurrMbIdxY)
{
  UInt uiMbCbp;
  Bool bTransitionFlag;
  MbDataAccess* pcMbDataAccessEL  = 0;

  RNOK( m_cMbDataCtrlEL     .initMb( pcMbDataAccessEL, uiCurrMbIdxY, uiCurrMbIdxX ) );

  if( m_uiChromaCbpRun == 0 )
  {
    // read the transition flag
    ((UvlcReader *) m_pcSymbolReader)->getFlag(bTransitionFlag, "Chroma_CBP_transition");
    m_uiLastChromaCbp = auiNextChromaCbp[m_uiLastChromaCbp][bTransitionFlag];

    ((UvlcReader *) m_pcSymbolReader)->getUvlc(m_uiChromaCbpRun, "Chroma_CBP_run");
    m_uiChromaCbpRun ++;
  }

  uiMbCbp = pcMbDataAccessEL->getMbData().getMbCbp();
  pcMbDataAccessEL->getMbData().setMbCbp( (uiMbCbp & 0x0F) | (m_uiLastChromaCbp << 4) );
  m_uiChromaCbpRun --;

  return Err::m_nOK;
}

ErrVal
RQFGSDecoder::xDecodingFGS()
{
  RNOK( m_cMbDataCtrlEL    .reset     () );
  RNOK( m_cMbDataCtrlEL    .clear     () );
  RNOK( m_cMbDataCtrlEL    .initSlice ( *m_pcCurrSliceHeader, PARSE_PROCESS, true, NULL ) );
  RNOK( m_pcCurrMbDataCtrl->initSlice ( *m_pcCurrSliceHeader, PRE_PROCESS, true, NULL ) );
  
  RNOK( xInitializeMacroblockQPs() );

  RNOK( m_cMbDataCtrlEL    .initSlice ( *m_pcCurrSliceHeader, PRE_PROCESS, true, NULL ) );

  Int iLastQP = m_pcCurrSliceHeader->getPicQp();

  UInt uiFirstMbInSlice = m_pcCurrSliceHeader->getFirstMbInSlice ();
  UInt uiNumMbsInSlice  = m_pcCurrSliceHeader->getNumMbsInSlice  ();
  m_bFgsComponentSep    = m_pcCurrSliceHeader->getFgsComponentSep();

  //positions vector for luma (and chromaAC) and chroma DC
  UInt ui;
  for(ui = 0; ui < 4; ui++)
  {
    m_auiScanPosVectChromaDC[ui] = ui;
  }
  if(m_pcCurrSliceHeader->getFGSCodingMode() == false)
  {
    //grouping size mode
    UInt uiGroupingSize = m_pcCurrSliceHeader->getGroupingSize();
    ui = 0;
    m_auiScanPosVectLuma[ui] = uiGroupingSize-1;
    while( m_auiScanPosVectLuma[ui] < 15)
    {
      ui++;
      m_auiScanPosVectLuma[ui] = m_auiScanPosVectLuma[ui-1]+uiGroupingSize;
    }
  }
  else
  {
    //vector specified
    ui = 0;
    m_auiScanPosVectLuma[ui] = m_pcCurrSliceHeader->getPosVect(ui);
    while( m_auiScanPosVectLuma[ui] != 15)
    {
      ui++;
      m_auiScanPosVectLuma[ui] = m_auiScanPosVectLuma[ui-1] + m_pcCurrSliceHeader->getPosVect(ui);
    }
  }
  try
  {
    AOT( m_pcSymbolReader == 0 );
    RNOK( m_pcSymbolReader  ->startSlice( *m_pcCurrSliceHeader ) );
    
    //===== SIGNIFICANCE PATH =====
    {
      UInt iStartCycle = 0, iCycle = 0;
      UInt iLumaScanIdx     = 0;
      UInt iChromaDCScanIdx = 0;
      UInt iChromaACScanIdx = 1;

      UInt uiFirstMbY = (UInt) ( uiFirstMbInSlice / m_uiWidthInMB );
      UInt uiFirstMbX = uiFirstMbInSlice % m_uiWidthInMB;
      UInt uiLastMbY  = (UInt) ( ( uiFirstMbInSlice + uiNumMbsInSlice ) / m_uiWidthInMB );
      UInt uiLastMbX  = ( uiFirstMbInSlice + uiNumMbsInSlice ) % m_uiWidthInMB;

      RNOK( m_pcSymbolReader->RQdecodeEobOffsets_Luma  () );
      RNOK( m_pcSymbolReader->RQdecodeEobOffsets_Chroma() );
      RNOK( m_pcSymbolReader->RQdecodeBestCodeTableMap  ( 16 ) );

      m_uiLumaCbpRun          = 0;
      m_uiChromaCbpRun        = 0;
      Bool bChromaCbpFlag;

      if( ! m_pcCurrSliceHeader->getPPS().getEntropyCodingModeFlag() )
      {
        m_pcUvlcReader->getFlag(m_bLastLumaCbpFlag, "Luma_CBP_first_flag");
        m_pcUvlcReader->getFlag(bChromaCbpFlag, "Chroma_CBP_first");
        m_uiLastChromaCbp = bChromaCbpFlag ? 1 : 0;
      }

      while (iLumaScanIdx < 16 || iChromaDCScanIdx < 4 || iChromaACScanIdx < 16) {
        UInt bAllowChromaDC = (iCycle == 0) || ((iCycle >= iStartCycle) && ((iCycle-iStartCycle) % 2 == 0));
        UInt bAllowChromaAC = (iCycle > 0) && ((iCycle == iStartCycle) || ((iCycle >= iStartCycle) && ((iCycle-iStartCycle) % 3 == 1)));
        if( iLumaScanIdx >= 16 && !bAllowChromaDC && !bAllowChromaAC )
        {
          iCycle++;
          continue;
        }

        UInt uiMaxPosLuma;
        UInt uiMaxPosChromaAC;
        UInt uiMaxPosChromaDC;

        if( iLumaScanIdx == 16 )
          uiMaxPosLuma = 16;
        else {
          for( ui=0; m_auiScanPosVectLuma[ui]<iLumaScanIdx; ui++ ) ;
          uiMaxPosLuma = m_auiScanPosVectLuma[ui];
        }
        if( iChromaACScanIdx == 16 )
          uiMaxPosChromaAC = 16;
        else {
          for( ui=0; 1+m_auiScanPosVectLuma[ui]<iChromaACScanIdx; ui++ ) ;
          uiMaxPosChromaAC = 1+m_auiScanPosVectLuma[ui];
        }
        if( iChromaDCScanIdx == 16 )
          uiMaxPosChromaDC = 16;
        else {
          for( ui=0; m_auiScanPosVectChromaDC[ui]<iChromaDCScanIdx; ui++ ) ;
          uiMaxPosChromaDC = m_auiScanPosVectChromaDC[ui];
        }

          for( UInt uiMbYIdx = uiFirstMbY; uiMbYIdx < uiLastMbY; uiMbYIdx++ )
          {
          for( UInt uiMbXIdx = ( uiMbYIdx == uiFirstMbY ? uiFirstMbX : 0 ); uiMbXIdx < ( uiMbYIdx == uiLastMbY ? uiLastMbX : m_uiWidthInMB );  uiMbXIdx++ )
          {
            if( m_pcCurrSliceHeader->getAdaptivePredictionFlag() &&
                ! m_pcCurrMbDataCtrl->getMbData( uiMbXIdx, uiMbYIdx ).isIntra() &&
                ( m_pauiMacroblockMap[uiMbYIdx * m_uiWidthInMB + uiMbXIdx] >> NUM_COEFF_SHIFT ) == 0 )
            {
              //----- Read motion parameters the first time we visit each inter-coded macroblock -----				
              RNOK( xDecodeMotionData( uiMbYIdx, uiMbXIdx ) );
            }
            //===== Luma =====
              if( ! m_pcCurrSliceHeader->getPPS().getEntropyCodingModeFlag() )
              {
                if( iLumaScanIdx == 0 )
                {
                  xDecodeLumaCbpVlc(uiMbXIdx, uiMbYIdx);
                  xDecodeChromaCbpVlc(uiMbXIdx, uiMbYIdx);
                }
              }
           RNOK( xDecodeNewCoefficientLumaMb( uiMbYIdx, uiMbXIdx, iLastQP, iLumaScanIdx, uiMaxPosLuma ) );

            //===== CHROMA DC =====
            if( bAllowChromaDC && ( m_bFgsComponentSep == 0 || iLumaScanIdx == 16 ) ) {
              for( UInt uiPlane = 0; uiPlane < 2; uiPlane ++ ) {
              for( ui=iChromaDCScanIdx; ui<=uiMaxPosChromaDC && ui<4; ui++ )
              {
                if( ui == 0 || ui == m_apaucScanPosMap[uiPlane+1][uiMbYIdx * m_uiWidthInMB + uiMbXIdx] )
                  RNOK( xDecodeNewCoefficientChromaDC( uiPlane, uiMbYIdx, uiMbXIdx, iLastQP, ui ) );
                if( (m_aapaucChromaDCCoefMap[uiPlane][ui][uiMbYIdx * m_uiWidthInMB + uiMbXIdx] & SIGNIFICANT) &&
                  !(m_aapaucChromaDCCoefMap[uiPlane][ui][uiMbYIdx * m_uiWidthInMB + uiMbXIdx] & CODED) )
                RNOK( xDecodeCoefficientChromaDCRef( uiPlane, uiMbYIdx, uiMbXIdx, ui ) );
              }
              } // for
            } // if

            //===== CHROMA AC =====
            if( bAllowChromaAC && ( m_bFgsComponentSep == 0 || iLumaScanIdx == 16 ) ) {
              for( UInt uiPlane = 0; uiPlane < 2; uiPlane ++ )
              for( UInt uiB8YIdx = 2 * uiMbYIdx; uiB8YIdx < 2 * uiMbYIdx + 2; uiB8YIdx++ )
              for( UInt uiB8XIdx = 2 * uiMbXIdx; uiB8XIdx < 2 * uiMbXIdx + 2; uiB8XIdx++ ) {
              for( ui=iChromaACScanIdx; ui<=uiMaxPosChromaAC && ui<16; ui++ )
              {
                if( ui == 1 || ui == m_apaucScanPosMap[uiPlane+3][uiB8YIdx*2 * m_uiWidthInMB + uiB8XIdx] )
                  RNOK( xDecodeNewCoefficientChromaAC( uiPlane, uiB8YIdx, uiB8XIdx, iLastQP, ui ) );
                if((m_aapaucChromaACCoefMap[uiPlane][ui][uiB8YIdx*2 * m_uiWidthInMB + uiB8XIdx] & SIGNIFICANT) &&
                  !(m_aapaucChromaACCoefMap[uiPlane][ui][uiB8YIdx*2 * m_uiWidthInMB + uiB8XIdx] & CODED) )
                  RNOK( xDecodeCoefficientChromaACRef( uiPlane, uiB8YIdx, uiB8XIdx, ui ) );
              }
              } // for
            } // if
            RNOK( m_pcSymbolReader->RQupdateVlcTable() );
          } // macroblock iteration
          }
          RNOK( m_pcSymbolReader->RQvlcFlush() );

        iLumaScanIdx = min(uiMaxPosLuma+1, 16);
        if (bAllowChromaDC)
          iChromaDCScanIdx = min(uiMaxPosChromaDC+1, 4);
        if (bAllowChromaAC)
          iChromaACScanIdx = min(uiMaxPosChromaAC+1, 16);

        if ( iCycle == 0 && m_bFgsComponentSep == 0 )
        {
          m_pcSymbolReader->RQdecodeCycleSymbol(iStartCycle);
        }

        iCycle++;

      } // while
    }
    // ==
    
    UInt  uiTermBit = 0;
    RNOK( m_pcSymbolReader->RQdecodeTermBit( uiTermBit ) );
    // heiko.schwarz@hhi.fhg.de: decoder could assert when nearly complete FGS slices are decoded
    //ROF ( uiTermBit );
    if( !uiTermBit )
    {
      throw BitReadBuffer::ReadStop();
    }
  }
  catch( BitReadBuffer::ReadStop )
  {
    m_bPicFinished = true;
  }

  if(m_pcSymbolReader == m_pcUvlcReader)
  {
    m_pcUvlcReader->m_bTruncated = m_bPicFinished;
  }

  RNOK( m_pcSymbolReader->finishSlice( ) );

  if( ! m_bPicFinished )
  {
    m_pcCurrSliceHeader->setQualityLevel( m_pcCurrSliceHeader->getQualityLevel() + 1 );
  }


  RNOK( xUpdateCodingPath() );
  RNOK( xClearCodingPath() );
  
  return Err::m_nOK;
}


ErrVal
RQFGSDecoder::xDecodingFGS( SliceHeader*                pcSliceHeader 	)
{
	RNOK( m_cMbDataCtrlEL    .reset     () );
  RNOK( m_cMbDataCtrlEL    .clear     () );
  RNOK( m_cMbDataCtrlEL    .initSlice ( *m_pcCurrSliceHeader, PARSE_PROCESS, true, NULL ) );
  RNOK( m_pcCurrMbDataCtrl->initSlice ( *m_pcCurrSliceHeader, PRE_PROCESS, true, NULL ) );
  
  RNOK( xInitializeMacroblockQPs() );

  RNOK( m_cMbDataCtrlEL    .initSlice ( *m_pcCurrSliceHeader, PRE_PROCESS, true, NULL ) );

  Int iLastQP = m_pcCurrSliceHeader->getPicQp();

  UInt uiFirstMbInSlice = m_pcCurrSliceHeader->getFirstMbInSlice ();
  // JVT-S054 (2) (ADD)
  UInt uiLastMbInSlice  = m_pcCurrSliceHeader->getLastMbInSlice();
  m_bFgsComponentSep    = m_pcCurrSliceHeader->getFgsComponentSep();

  Bool isTruncated =false;

  //positions vector for luma (and chromaAC) and chroma DC
  UInt ui;
  for(ui = 0; ui < 4; ui++)
  {
    m_auiScanPosVectChromaDC[ui] = ui;
  }
  if(m_pcCurrSliceHeader->getFGSCodingMode() == false)
  {
    //grouping size mode
    UInt uiGroupingSize = m_pcCurrSliceHeader->getGroupingSize();
    ui = 0;
    m_auiScanPosVectLuma[ui] = uiGroupingSize-1;
    while( m_auiScanPosVectLuma[ui] < 15)
    {
      ui++;
      m_auiScanPosVectLuma[ui] = m_auiScanPosVectLuma[ui-1]+uiGroupingSize;
    }
  }
  else
  {
    //vector specified
    ui = 0;
    m_auiScanPosVectLuma[ui] = m_pcCurrSliceHeader->getPosVect(ui);
    while( m_auiScanPosVectLuma[ui] != 15)
    {
      ui++;
      m_auiScanPosVectLuma[ui] = m_auiScanPosVectLuma[ui-1] + m_pcCurrSliceHeader->getPosVect(ui);
    }
  }
  try
  {
    AOT( m_pcSymbolReader == 0 );
    RNOK( m_pcSymbolReader  ->startSlice( *m_pcCurrSliceHeader ) );
    
    //===== SIGNIFICANCE PATH =====
    {
      UInt iStartCycle = 0, iCycle = 0;
      UInt iLumaScanIdx     = 0;
      UInt iChromaDCScanIdx = 0;
      UInt iChromaACScanIdx = 1;

      RNOK( m_pcSymbolReader->RQdecodeEobOffsets_Luma  () );
      RNOK( m_pcSymbolReader->RQdecodeEobOffsets_Chroma() );
      RNOK( m_pcSymbolReader->RQdecodeBestCodeTableMap  ( 16 ) );

      m_uiLumaCbpRun          = 0;
      m_uiChromaCbpRun        = 0;
      Bool bChromaCbpFlag;

      if( ! m_pcCurrSliceHeader->getPPS().getEntropyCodingModeFlag() )
      {
        m_pcUvlcReader->getFlag(m_bLastLumaCbpFlag, "Luma_CBP_first_flag");
        m_pcUvlcReader->getFlag(bChromaCbpFlag, "Chroma_CBP_first");
        m_uiLastChromaCbp = bChromaCbpFlag ? 1 : 0;
      }

      while (iLumaScanIdx < 16 || iChromaDCScanIdx < 4 || iChromaACScanIdx < 16) {
        UInt bAllowChromaDC = (iCycle == 0) || ((iCycle >= iStartCycle) && ((iCycle-iStartCycle) % 2 == 0));
        UInt bAllowChromaAC = (iCycle > 0) && ((iCycle == iStartCycle) || ((iCycle >= iStartCycle) && ((iCycle-iStartCycle) % 3 == 1)));
        if( iLumaScanIdx >= 16 && !bAllowChromaDC && !bAllowChromaAC )
        {
          iCycle++;
          continue;
        }

        UInt uiMaxPosLuma;
        UInt uiMaxPosChromaAC;
        UInt uiMaxPosChromaDC;

        if( iLumaScanIdx == 16 )
          uiMaxPosLuma = 16;
        else {
          for( ui=0; m_auiScanPosVectLuma[ui]<iLumaScanIdx; ui++ ) ;
          uiMaxPosLuma = m_auiScanPosVectLuma[ui];
        }
        if( iChromaACScanIdx == 16 )
          uiMaxPosChromaAC = 16;
        else {
          for( ui=0; 1+m_auiScanPosVectLuma[ui]<iChromaACScanIdx; ui++ ) ;
          uiMaxPosChromaAC = 1+m_auiScanPosVectLuma[ui];
        }
        if( iChromaDCScanIdx == 16 )
          uiMaxPosChromaDC = 16;
        else {
          for( ui=0; m_auiScanPosVectChromaDC[ui]<iChromaDCScanIdx; ui++ ) ;
          uiMaxPosChromaDC = m_auiScanPosVectChromaDC[ui];
        }

        UInt uiMbAddress = 0;
        for(uiMbAddress=uiFirstMbInSlice ;uiMbAddress<=uiLastMbInSlice ;)
        {
          UInt uiMbYIdx = uiMbAddress / m_uiWidthInMB;
          UInt uiMbXIdx = uiMbAddress % m_uiWidthInMB;

            if( m_pcCurrSliceHeader->getAdaptivePredictionFlag() &&
                ! m_pcCurrMbDataCtrl->getMbData( uiMbXIdx, uiMbYIdx ).isIntra() &&
                ( m_pauiMacroblockMap[uiMbYIdx * m_uiWidthInMB + uiMbXIdx] >> NUM_COEFF_SHIFT ) == 0 )
            {
              //----- Read motion parameters the first time we visit each inter-coded macroblock -----
              RNOK( xDecodeMotionData( uiMbYIdx, uiMbXIdx ) );
            }
            //===== Luma =====
              if( ! m_pcCurrSliceHeader->getPPS().getEntropyCodingModeFlag() )
              {
                if( iLumaScanIdx == 0 )
                {
                  xDecodeLumaCbpVlc(uiMbXIdx, uiMbYIdx);
                  xDecodeChromaCbpVlc(uiMbXIdx, uiMbYIdx);
                }
              }
           RNOK( xDecodeNewCoefficientLumaMb( uiMbYIdx, uiMbXIdx, iLastQP, iLumaScanIdx, uiMaxPosLuma ) );

            //===== CHROMA DC =====
            if( bAllowChromaDC && ( m_bFgsComponentSep == 0 || iLumaScanIdx == 16 ) ) {
              for( UInt uiPlane = 0; uiPlane < 2; uiPlane ++ ) {
              for( ui=iChromaDCScanIdx; ui<=uiMaxPosChromaDC && ui<4; ui++ )
              {
                if( ui == 0 || ui == m_apaucScanPosMap[uiPlane+1][uiMbYIdx * m_uiWidthInMB + uiMbXIdx] )
                  RNOK( xDecodeNewCoefficientChromaDC( uiPlane, uiMbYIdx, uiMbXIdx, iLastQP, ui ) );
                if( (m_aapaucChromaDCCoefMap[uiPlane][ui][uiMbYIdx * m_uiWidthInMB + uiMbXIdx] & SIGNIFICANT) &&
                  !(m_aapaucChromaDCCoefMap[uiPlane][ui][uiMbYIdx * m_uiWidthInMB + uiMbXIdx] & CODED) )
                RNOK( xDecodeCoefficientChromaDCRef( uiPlane, uiMbYIdx, uiMbXIdx, ui ) );
              }
              } // for
            } // if

            //===== CHROMA AC =====
            if( bAllowChromaAC && ( m_bFgsComponentSep == 0 || iLumaScanIdx == 16 ) ) {
              for( UInt uiPlane = 0; uiPlane < 2; uiPlane ++ )
              for( UInt uiB8YIdx = 2 * uiMbYIdx; uiB8YIdx < 2 * uiMbYIdx + 2; uiB8YIdx++ )
              for( UInt uiB8XIdx = 2 * uiMbXIdx; uiB8XIdx < 2 * uiMbXIdx + 2; uiB8XIdx++ ) {
              for( ui=iChromaACScanIdx; ui<=uiMaxPosChromaAC && ui<16; ui++ )
              {
                if( ui == 1 || ui == m_apaucScanPosMap[uiPlane+3][uiB8YIdx*2 * m_uiWidthInMB + uiB8XIdx] )
                  RNOK( xDecodeNewCoefficientChromaAC( uiPlane, uiB8YIdx, uiB8XIdx, iLastQP, ui ) );
                if((m_aapaucChromaACCoefMap[uiPlane][ui][uiB8YIdx*2 * m_uiWidthInMB + uiB8XIdx] & SIGNIFICANT) &&
                  !(m_aapaucChromaACCoefMap[uiPlane][ui][uiB8YIdx*2 * m_uiWidthInMB + uiB8XIdx] & CODED) )
                  RNOK( xDecodeCoefficientChromaACRef( uiPlane, uiB8YIdx, uiB8XIdx, ui ) );
              }
              } // for
            } // if
            RNOK( m_pcSymbolReader->RQupdateVlcTable() );

			//--ICU/ETRI FMO Implementation
			uiMbAddress = m_pcCurrSliceHeader->getFMO()->getNextMBNr(uiMbAddress );

          } // macroblock iteration
          RNOK( m_pcSymbolReader->RQvlcFlush() );

        iLumaScanIdx = min(uiMaxPosLuma+1, 16);
        if (bAllowChromaDC)
          iChromaDCScanIdx = min(uiMaxPosChromaDC+1, 4);
        if (bAllowChromaAC)
          iChromaACScanIdx = min(uiMaxPosChromaAC+1, 16);

        if ( iCycle == 0 && m_bFgsComponentSep == 0 )
        {
          m_pcSymbolReader->RQdecodeCycleSymbol(iStartCycle);
        }

        iCycle++;

      } // while
    }
    // ==
    
    UInt  uiTermBit = 0;
    RNOK( m_pcSymbolReader->RQdecodeTermBit( uiTermBit ) );
    // heiko.schwarz@hhi.fhg.de: decoder could assert when nearly complete FGS slices are decoded
    //ROF ( uiTermBit );
    if( !uiTermBit )
    {
      throw BitReadBuffer::ReadStop();
    }
  }
  catch( BitReadBuffer::ReadStop )
  {
	  // FGS ROI DECODE ICU/ETRI
	  isTruncated =true;	
  }

  if(m_pcSymbolReader == m_pcUvlcReader)
  {
	  // FGS ROI DECODE ICU/ETRI
	  m_pcUvlcReader->m_bTruncated = isTruncated ;
  }

  RNOK( m_pcSymbolReader->finishSlice( ) );

  if( ! m_bPicFinished )
  {	  
	  m_pcCurrSliceHeader->setQualityLevel( pcSliceHeader->getQualityLevel());
  }


  //--ICU/ETRI 1206  
  RNOK( xUpdateCodingPath(pcSliceHeader) );  
  RNOK( xClearCodingPath() );
  
  return Err::m_nOK;
}


ErrVal
RQFGSDecoder::xDecodeMotionData( UInt uiMbYIdx, UInt uiMbXIdx )
{
	// ICU/ETRI FGS_MOT_USE
	m_bFGSMotionUse[m_pcCurrSliceHeader->getLayerId()] = true;

  DTRACE_DO( UInt          uiMbIndex         = uiMbYIdx * m_uiWidthInMB + uiMbXIdx );
  MbDataAccess* pcMbDataAccessEL  = 0;
  MbDataAccess* pcMbDataAccessBL  = 0;

  RNOK( m_cMbDataCtrlEL     .initMb( pcMbDataAccessEL, uiMbYIdx, uiMbXIdx ) );
  RNOK( m_pcCurrMbDataCtrl ->initMb( pcMbDataAccessBL, uiMbYIdx, uiMbXIdx ) );
  ROT ( pcMbDataAccessBL->getMbData().isIntra() );

  DTRACE_NEWMB( uiMbIndex );
  RNOK( m_pcMbParser ->readMotion( *pcMbDataAccessEL, pcMbDataAccessBL ) );
  RNOK( m_pcMbDecoder->calcMv    ( *pcMbDataAccessEL, pcMbDataAccessBL ) );

  if( ! pcMbDataAccessEL->getMbData().getBLSkipFlag() && ! pcMbDataAccessEL->getMbData().getResidualPredFlag( PART_16x16 ) )
  {
    //----- motion refinement without residual prediction ===> clear base layer coeffs -----
    UInt            uiLayer         = m_pcCurrSliceHeader->getLayerId();
    YuvBufferCtrl*  pcYuvBufferCtrl = m_papcYuvFullPelBufferCtrl[uiLayer];
    RNOK( pcYuvBufferCtrl->initMb( uiMbYIdx, uiMbXIdx ) );
    RNOK( xClearBaseCoeffs( *pcMbDataAccessEL, pcMbDataAccessBL ) );
  }
  return Err::m_nOK;
}


ErrVal
RQFGSDecoder::xInitializeMacroblockQPs()
{
  for( UInt uiMbY = 0; uiMbY < m_uiHeightInMB; uiMbY++ )
  for( UInt uiMbX = 0; uiMbX < m_uiWidthInMB;  uiMbX++ )
  {
    MbDataAccess* pcMbDataAccess    = 0;
    RNOK( m_pcCurrMbDataCtrl->initMb( pcMbDataAccess,   uiMbY, uiMbX ) );

    MbDataAccess* pcMbDataAccessEL  = 0;
    RNOK( m_cMbDataCtrlEL    .initMb( pcMbDataAccessEL, uiMbY, uiMbX ) );

    //===== set QP for enhancement layer =====
    Int iQpEL = max( 0, pcMbDataAccess->getMbData().getQp() - RQ_QP_DELTA );
    pcMbDataAccessEL->getMbData().setQp( iQpEL );
    RNOK( pcMbDataAccessEL->getMbData().copyMotion( pcMbDataAccess->getMbData() ) );
    if( ! m_pcCurrSliceHeader->getAdaptivePredictionFlag() && ! pcMbDataAccess->getMbData().isIntra() )
      pcMbDataAccessEL->getMbData().setBLSkipFlag( true );
  }

  return Err::m_nOK;
}

ErrVal
RQFGSDecoder::xDecodeNewCoefficientLumaMb( UInt  uiMbYIdx,
                                           UInt  uiMbXIdx,
                                           Int&  riLastQp,
                                           Int   iLumaScanIdx,
                                           UInt  uiMaxPosLuma )
{
  ROFRS( iLumaScanIdx < 16, Err::m_nOK );

  MbDataAccess* pcMbDataAccessEL  = 0;
  MbDataAccess* pcMbDataAccessBL  = 0;

  for( UInt uiB8YIdx = 2 * uiMbYIdx; uiB8YIdx < 2 * uiMbYIdx + 2; uiB8YIdx++ )
  for( UInt uiB8XIdx = 2 * uiMbXIdx; uiB8XIdx < 2 * uiMbXIdx + 2; uiB8XIdx++ )
  {
    //UInt uiBaseBlock = 2 * uiB8YIdx * 4 * m_uiWidthInMB + 2 * uiB8XIdx; // unused variable. mwi
    for( UInt uiBlockYIdx = 2 * uiB8YIdx; uiBlockYIdx < 2 * uiB8YIdx + 2; uiBlockYIdx++ )
    for( UInt uiBlockXIdx = 2 * uiB8XIdx; uiBlockXIdx < 2 * uiB8XIdx + 2; uiBlockXIdx++ )
    {
      UInt uiBlockIndex = uiBlockYIdx * 4 * m_uiWidthInMB + uiBlockXIdx;
      if( iLumaScanIdx == 0 )
      {
        if ( !pcMbDataAccessBL )
        {
          RNOK( m_cMbDataCtrlEL     .initMb( pcMbDataAccessEL, uiMbYIdx, uiMbXIdx ) );
          RNOK( m_pcCurrMbDataCtrl ->initMb( pcMbDataAccessBL, uiMbYIdx, uiMbXIdx ) );
        }
        RNOK( xDecodeSigHeadersLuma( pcMbDataAccessBL, pcMbDataAccessEL, uiBlockYIdx, uiBlockXIdx, riLastQp ) );
      }

      for( UInt ui=iLumaScanIdx; ui<=uiMaxPosLuma && ui<16; ui++ )
      {
        if ( !pcMbDataAccessBL )
        {
          RNOK( m_cMbDataCtrlEL     .initMb( pcMbDataAccessEL, uiMbYIdx, uiMbXIdx ) );
          RNOK( m_pcCurrMbDataCtrl ->initMb( pcMbDataAccessBL, uiMbYIdx, uiMbXIdx ) );
        }
        while( m_apaucScanPosMap[0][uiBlockIndex] <= ui ) 
        {
          RNOK( xDecodeNewCoefficientLuma( pcMbDataAccessBL, pcMbDataAccessEL, uiBlockYIdx, uiBlockXIdx ) );
        }
        RNOK( xDecodeCoefficientLumaRef( uiBlockYIdx, uiBlockXIdx, ui ) );
      }
    } // 4x4 block iteration
  } // 8x8 block iteration

  return Err::m_nOK;
}


ErrVal
RQFGSDecoder::xDecodeSigHeadersLuma( MbDataAccess* pcMbDataAccessBL,
                                     MbDataAccess* pcMbDataAccessEL,
                                     UInt          uiBlockYIndex,
                                     UInt          uiBlockXIndex,
                                     Int&          riLastQp )
{
  UInt    uiSubMbIndex  = (uiBlockYIndex/2) * 2 * m_uiWidthInMB + (uiBlockXIndex/2);
  UInt    uiMbIndex     = (uiBlockYIndex/4) * 1 * m_uiWidthInMB + (uiBlockXIndex/4);
  
  UInt    uiB8x8        = ((uiBlockYIndex%4)/2) * 2 + ((uiBlockXIndex%4)/2);
  Par8x8  ePar8x8       = Par8x8(uiB8x8);
  
  B8x8Idx c8x8Idx(ePar8x8);

  if( ! ( m_paucSubMbMap[uiSubMbIndex] & CODED ) )
  {
    //===== CBP =====
    Bool bSigCBP = m_pcSymbolReader->RQdecodeCBP_8x8( *pcMbDataAccessEL, *pcMbDataAccessBL, c8x8Idx );
    m_paucSubMbMap[uiSubMbIndex] |= CODED;
    if(  bSigCBP )
    {
      m_paucSubMbMap[uiSubMbIndex] |= SIGNIFICANT;
    }
    
    if( !bSigCBP )
    {
      //===== set coefficient and block map =====
      for( S4x4Idx cIdx(c8x8Idx); cIdx.isLegal(c8x8Idx); cIdx++ )
      {
        UInt uiBlk = ( (uiBlockYIndex/4)*4 + cIdx.y() ) * 4 * m_uiWidthInMB + ( (uiBlockXIndex/4)*4 + cIdx.x() );

        m_paucBlockMap[uiBlk] |= CODED;
        m_apaucScanPosMap[0][uiBlk] = 16;

        for( UInt ui = 0; ui < 16; ui++ )
        {
          if( ! ( m_apaucLumaCoefMap[ui][uiBlk] & SIGNIFICANT ) )
          {
            m_apaucLumaCoefMap[ui][uiBlk] |= CODED;
            m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
            ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );
          }
        }
      }
      return Err::m_nOK;
    }
    
    //===== DELTA QP & TRANSFORM SIZE =====
    if( ! ( m_pauiMacroblockMap[uiMbIndex] & CODED ) )
    {
      //===== delta QP =====
      if( ! ( m_pauiMacroblockMap[uiMbIndex] & SIGNIFICANT ) )
      {
        pcMbDataAccessEL->setLastQp( riLastQp );
        RNOK( m_pcSymbolReader->RQdecodeDeltaQp( *pcMbDataAccessEL ) );
        riLastQp = pcMbDataAccessEL->getMbData().getQp();

        m_pauiMacroblockMap[uiMbIndex] |= SIGNIFICANT;
      }

      //===== transform size =====
      if( ! ( m_pauiMacroblockMap[uiMbIndex] & TRANSFORM_SPECIFIED ) )
      {
        RNOK( m_pcSymbolReader->RQdecode8x8Flag( *pcMbDataAccessEL, *pcMbDataAccessBL ) );

        m_pauiMacroblockMap[uiMbIndex] |= TRANSFORM_SPECIFIED;
      }

      m_pauiMacroblockMap[uiMbIndex] |= CODED;
    }
  }


  if( ! pcMbDataAccessBL->getMbData().isTransformSize8x8() )
  {
    for (UInt uiY=(uiBlockYIndex/2)*2; uiY<(uiBlockYIndex/2)*2+2; uiY++)
    {
      for (UInt uiX=(uiBlockXIndex/2)*2; uiX<(uiBlockXIndex/2)*2+2; uiX++)
      {
        UInt uiBlk = uiY * 4 * m_uiWidthInMB + uiX;
        UInt uiB4x4        =  (uiY%4)    * 4 +  (uiX%4);
        B4x4Idx c4x4Tmp(uiB4x4);
        if( ! ( m_paucBlockMap[uiBlk] & CODED ) )
        {
          Bool bSigBCBP = m_pcSymbolReader->RQdecodeBCBP_4x4( *pcMbDataAccessBL, c4x4Tmp );
          m_paucBlockMap[uiBlk] |= CODED;
          if(  bSigBCBP )
          {
            m_paucBlockMap[uiBlk] |= SIGNIFICANT;
          }
          if( ! bSigBCBP )
          {
            m_apaucScanPosMap[0][uiBlk] = 16;
            for( UInt ui = 0; ui < 16; ui++ )
            {
              if( ! ( m_apaucLumaCoefMap[ui][uiBlk] & SIGNIFICANT ) )
              {
                m_apaucLumaCoefMap[ui][uiBlk] |= CODED;
                m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
                ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );
              }
            }
          }
        }
      }
    } 
  }
  
  return Err::m_nOK;
}


ErrVal
RQFGSDecoder::xDecodeNewCoefficientLuma( MbDataAccess* pcMbDataAccessBL,
                                         MbDataAccess* pcMbDataAccessEL,
                                         UInt          uiBlockYIndex,
                                         UInt          uiBlockXIndex )
{
  UInt    uiBlockIndex  =  uiBlockYIndex    * 4 * m_uiWidthInMB +  uiBlockXIndex;
  UInt    uiMbIndex     = (uiBlockYIndex/4) * 1 * m_uiWidthInMB + (uiBlockXIndex/4);
  
  UInt    uiB8x8        = ((uiBlockYIndex%4)/2) * 2 + ((uiBlockXIndex%4)/2);
  UInt    uiB4x4        =  (uiBlockYIndex%4)    * 4 +  (uiBlockXIndex%4);
  Par8x8  ePar8x8       = Par8x8(uiB8x8);
  
  B8x8Idx c8x8Idx(ePar8x8);
  B4x4Idx c4x4Idx(uiB4x4);


  if( pcMbDataAccessBL->getMbData().isTransformSize8x8() )
  {
    UInt auiBlockIdx[4] = { ( (uiBlockYIndex/4)*4 + c8x8Idx.y()     ) * 4 * m_uiWidthInMB + ( (uiBlockXIndex/4)*4 + c8x8Idx.x()     ),
                            ( (uiBlockYIndex/4)*4 + c8x8Idx.y()     ) * 4 * m_uiWidthInMB + ( (uiBlockXIndex/4)*4 + c8x8Idx.x() + 1 ),
                            ( (uiBlockYIndex/4)*4 + c8x8Idx.y() + 1 ) * 4 * m_uiWidthInMB + ( (uiBlockXIndex/4)*4 + c8x8Idx.x()     ),
                            ( (uiBlockYIndex/4)*4 + c8x8Idx.y() + 1 ) * 4 * m_uiWidthInMB + ( (uiBlockXIndex/4)*4 + c8x8Idx.x() + 1 ) };

    UInt uiBlkIter8x8;
    UInt uiBlockIdx = uiBlockYIndex * 4 * m_uiWidthInMB + uiBlockXIndex;
    for( uiBlkIter8x8 = 0; uiBlkIter8x8 < 4; uiBlkIter8x8++ )
    {
      if( auiBlockIdx[uiBlkIter8x8] == uiBlockIdx )
        break;
    }

    UInt uiStop         = 64;
    UInt ui8x8ScanIndex = m_apaucScanPosMap[0][auiBlockIdx[uiBlkIter8x8]] * 4 + uiBlkIter8x8;
    ROTRS( ui8x8ScanIndex >= uiStop, Err::m_nOK );

    if (uiMbIndex != m_uiLastMbNum)
    {
      UInt uiOffset;
      UInt uiMin    = 16;
      UInt uiRemain = 0;

      m_uiLastMbNum = uiMbIndex;
      for( uiOffset=0; uiOffset<4; uiOffset++ )
      {
        uiMin = min( uiMin, m_apaucScanPosMap[0][auiBlockIdx[uiOffset]] );
        if( m_apaucScanPosMap[0][auiBlockIdx[uiOffset]] < 16 )
          uiRemain++;
      }
      if( ( uiRemain == 2 ) || ( uiMin > 10 && uiRemain == 3 ) )
      {
        Bool bIsEob   = false;
        m_pcSymbolReader->RQeo8b( bIsEob );
        if (bIsEob)
        {
          //UInt uiOffset; // shadowes previous declaration. mwi
          UInt ui8x8Index;
          for( ui8x8Index = 0; ui8x8Index < uiStop; ui8x8Index++ )
          {
            UInt  uiS = ui8x8Index / 4;
            UInt  uiB = auiBlockIdx[ui8x8Index%4];
            if( ! ( m_apaucLumaCoefMap[uiS][uiB] & SIGNIFICANT ) && ! ( m_apaucLumaCoefMap[uiS][uiB] & CODED ) )
            {
              m_apaucLumaCoefMap[uiS][uiB] |= CODED;
              m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
              ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );
            }
          }
          for( uiOffset=0; uiOffset<4; uiOffset++ )
          {
            m_apaucScanPosMap[0][auiBlockIdx[uiOffset]] = 16;
          }
          ui8x8ScanIndex = m_apaucScanPosMap[0][auiBlockIdx[uiBlkIter8x8]] * 4 + uiBlkIter8x8;
          ROTRS( ui8x8ScanIndex >= uiStop, Err::m_nOK );
        }
      }
    }

    Bool bNeedEob      = true;
    UInt uiNumCoefRead = 0;
    UInt uiStride      = 4;

    RNOK( m_pcSymbolReader->RQdecodeNewTCoeff_8x8( *pcMbDataAccessEL, *pcMbDataAccessBL,
                                                    c8x8Idx, ui8x8ScanIndex, bNeedEob, uiNumCoefRead ) );

    for ( UInt ui8x8 = 0; ui8x8ScanIndex < 64 && ( ui8x8 < uiNumCoefRead || bNeedEob ); ui8x8ScanIndex+=uiStride )
    {
      UInt  uiS = ui8x8ScanIndex/4;
      UInt  uiB = auiBlockIdx[ui8x8ScanIndex%4];
      if( ! ( m_apaucLumaCoefMap[uiS][uiB] & SIGNIFICANT ) )
      {
        m_apaucLumaCoefMap[uiS][uiB] |= CODED;
        if( pcMbDataAccessEL->getMbTCoeffs().get8x8( c8x8Idx )[g_aucFrameScan64[ui8x8ScanIndex]] )
        {
          m_apaucLumaCoefMap[uiS][uiB] |= SIGNIFICANT;
        }
        m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
        ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );
        if( m_apaucLumaCoefMap[uiS][uiB] & SIGNIFICANT )
          m_apaucLumaCoefMap[uiS][uiB] |= NEWSIG;
        ui8x8++;
      }
    }
    if( m_apaucScanPosMap[0][auiBlockIdx[uiBlkIter8x8]] * 4 + uiBlkIter8x8 < ui8x8ScanIndex )
      m_apaucScanPosMap[0][auiBlockIdx[uiBlkIter8x8]] = ui8x8ScanIndex / 4;
    while( ui8x8ScanIndex < 64 && ( m_apaucLumaCoefMap[ui8x8ScanIndex/4][auiBlockIdx[ui8x8ScanIndex%4]] & SIGNIFICANT ) )
    {
      ui8x8ScanIndex += 4;
      m_apaucScanPosMap[0][auiBlockIdx[uiBlkIter8x8]] = ui8x8ScanIndex / 4;
    }
  }
  else
  {
    if (! m_pcSymbolReader->RQpeekCbp4x4( *pcMbDataAccessBL, c4x4Idx ) )
    {
      return Err::m_nOK;
    }

    UInt    uiStop       = 16;
    UInt    uiScanIndex  = uiStop;
    UInt    uiStartIndex = uiStop;
    UInt    uiIndex;
    for( uiIndex = 0; uiIndex < uiStop; uiIndex++ )
    {
      if( ! ( m_apaucLumaCoefMap[uiIndex][uiBlockIndex] & SIGNIFICANT ) || ( m_apaucLumaCoefMap[uiIndex][uiBlockIndex] & NEWSIG ) )
      {
        uiStartIndex = uiIndex;
        break;
      }
    }
    uiScanIndex = m_apaucScanPosMap[0][uiBlockIndex];
    ROTRS(uiScanIndex == uiStop, Err::m_nOK);

    Bool bNeedEob = ( uiScanIndex > uiStartIndex );
    UInt uiNumCoefRead;
    RNOK( m_pcSymbolReader->RQdecodeNewTCoeff_Luma( *pcMbDataAccessEL, *pcMbDataAccessBL,
                                                    LUMA_SCAN, c4x4Idx, uiScanIndex, bNeedEob, uiNumCoefRead ) );

    for ( UInt ui = 0; uiScanIndex < 16 && ( ui < uiNumCoefRead || bNeedEob ); uiScanIndex++ )
    {
      if( ! ( m_apaucLumaCoefMap[uiScanIndex][uiBlockIndex] & SIGNIFICANT ) )
      {
        if( pcMbDataAccessEL->getMbTCoeffs().get( c4x4Idx )[g_aucFrameScan[uiScanIndex]] )
        {
          m_apaucLumaCoefMap[uiScanIndex][uiBlockIndex] |= SIGNIFICANT;
          m_apaucLumaCoefMap[uiScanIndex][uiBlockIndex] |= NEWSIG;
        }
        m_apaucLumaCoefMap[uiScanIndex][uiBlockIndex] |= CODED;
        m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
        ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );
        ui++;
      }
    }
    m_apaucScanPosMap[0][uiBlockIndex] = uiScanIndex;
    while( uiScanIndex < 16 && ( m_apaucLumaCoefMap[uiScanIndex][uiBlockIndex] & SIGNIFICANT ) )
      m_apaucScanPosMap[0][uiBlockIndex] = ++uiScanIndex;
  }
  
  return Err::m_nOK;
}



ErrVal
RQFGSDecoder::xDecodeNewCoefficientChromaDC ( UInt    uiPlane,
                                              UInt    uiMbYIdx,
                                              UInt    uiMbXIdx,
                                              Int&    riLastQP,
                                              UInt    uiChromaScanIndex )
{
  UInt uiMbIndex = uiMbYIdx * m_uiWidthInMB + uiMbXIdx;


  UInt    uiDCIdx    = 4;
  UInt    uiStartIdx = 4;
  UInt    uiIndex;
  for( uiIndex = 0; uiIndex < 4; uiIndex++ )
  {
    if( ! ( m_aapaucChromaDCCoefMap[uiPlane][uiIndex][uiMbIndex] & SIGNIFICANT) || ( m_aapaucChromaDCCoefMap[uiPlane][uiIndex][uiMbIndex] & NEWSIG ) )
    {
      uiStartIdx = uiIndex;
      break;
    }
  }
  uiDCIdx = m_apaucScanPosMap[uiPlane+1][uiMbIndex];
  ROTRS(uiDCIdx == 4, Err::m_nOK);
  ROTRS(uiDCIdx > uiChromaScanIndex, Err::m_nOK);

  Bool bNeedEob = ( uiDCIdx > uiStartIdx );

  MbDataAccess* pcMbDataAccessEL  = 0;
  MbDataAccess* pcMbDataAccessBL  = 0;
  RNOK( m_cMbDataCtrlEL     .initMb( pcMbDataAccessEL, uiMbYIdx, uiMbXIdx ) );
  RNOK( m_pcCurrMbDataCtrl ->initMb( pcMbDataAccessBL, uiMbYIdx, uiMbXIdx ) );
  

  if( ! ( m_pauiMacroblockMap[uiMbIndex] & CHROMA_CBP_CODED ) )
  {
    //===== CHROMA CBP =====
    Bool  bSigCBP = m_pcSymbolReader->RQdecodeCBP_Chroma( *pcMbDataAccessEL, *pcMbDataAccessBL );
    m_pauiMacroblockMap[uiMbIndex] |= CHROMA_CBP_CODED;
    
    if( !bSigCBP )
    {
      //----- chroma DC -----
      for( UInt uiCP = 0; uiCP < 2; uiCP++ )
      {
        m_apaucChromaDCBlockMap[uiCP][uiMbIndex] |= CODED;
        m_apaucScanPosMap[uiCP + 1][uiMbIndex] = 4;

        for( UInt ui = 0; ui < 4; ui++ )
        {
          if( ! ( m_aapaucChromaDCCoefMap[uiCP][ui][uiMbIndex] & SIGNIFICANT ) )
          {
            m_aapaucChromaDCCoefMap[uiCP][ui][uiMbIndex] |= CODED;
            m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
            ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );
          }
        }
      }

      //----- chroma AC -----
      for( CIdx cCIdx; cCIdx.isLegal(); cCIdx++ )
      {
        UInt  uiCP  = cCIdx.plane();
        UInt  uiBlk = ( uiMbYIdx*2 + cCIdx.y() ) * 2 * m_uiWidthInMB + ( uiMbXIdx*2 + cCIdx.x() );

        m_apaucChromaACBlockMap[uiCP][uiBlk] |= CODED;
        m_apaucScanPosMap[uiCP + 3][uiBlk] = 16;

        for( UInt ui = 1; ui < 16; ui++ )
        {
          if( ! ( m_aapaucChromaACCoefMap[uiCP][ui][uiBlk] & SIGNIFICANT ) )
          {
            m_aapaucChromaACCoefMap[uiCP][ui][uiBlk] |= CODED;
            m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
            ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );
          }
        }
      }

      return Err::m_nOK;
    }
    
    //===== DELTA QP =====
    if( ! ( m_pauiMacroblockMap[uiMbIndex] & CODED ) )
    {
      //===== delta QP =====
      if( ! ( m_pauiMacroblockMap[uiMbIndex] & SIGNIFICANT ) )
      {
        pcMbDataAccessEL->setLastQp( riLastQP );
        RNOK( m_pcSymbolReader->RQdecodeDeltaQp( *pcMbDataAccessEL ) );
        riLastQP = pcMbDataAccessEL->getMbData().getQp();

        m_pauiMacroblockMap[uiMbIndex] |= SIGNIFICANT;
      }
    }
  }



  if( ! ( m_apaucChromaDCBlockMap[uiPlane][uiMbIndex] & CODED ) )
  {
    Bool bSigBCBP = m_pcSymbolReader->RQdecodeBCBP_ChromaDC( *pcMbDataAccessBL, CIdx(4*uiPlane) );
    m_apaucChromaDCBlockMap[uiPlane][uiMbIndex] |= CODED;
    if(  bSigBCBP )
    {
      m_apaucChromaDCBlockMap[uiPlane][uiMbIndex] |= SIGNIFICANT;
    }
    if( ! bSigBCBP )
    {
      m_apaucScanPosMap[uiPlane + 1][uiMbIndex] = 4;
      for( UInt ui = 0; ui < 4; ui++ )
      {
        if( ! ( m_aapaucChromaDCCoefMap[uiPlane][ui][uiMbIndex] & SIGNIFICANT ) )
        {
          m_aapaucChromaDCCoefMap[uiPlane][ui][uiMbIndex] |= CODED;
          m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
          ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );
        }
      }
      return Err::m_nOK;
    }
  }


  // Encode EOB marker?
  UInt uiNumCoefRead;
  RNOK( m_pcSymbolReader->RQdecodeNewTCoeff_Chroma( *pcMbDataAccessEL, *pcMbDataAccessBL,
                                                    CHROMA_DC, CIdx(4*uiPlane), uiDCIdx, bNeedEob, uiNumCoefRead ) );

  for ( UInt ui = 0; uiDCIdx < 4 && ( ui < uiNumCoefRead || bNeedEob ); uiDCIdx++ )
  {
    if( ! ( m_aapaucChromaDCCoefMap[uiPlane][uiDCIdx][uiMbIndex] & SIGNIFICANT ) )
    {
      if( pcMbDataAccessEL->getMbTCoeffs().get( CIdx(4*uiPlane) )[g_aucIndexChromaDCScan[uiDCIdx]] )
      {
        m_aapaucChromaDCCoefMap[uiPlane][uiDCIdx][uiMbIndex] |= SIGNIFICANT;
        m_aapaucChromaDCCoefMap[uiPlane][uiDCIdx][uiMbIndex] |= NEWSIG;
      }
      m_aapaucChromaDCCoefMap[uiPlane][uiDCIdx][uiMbIndex] |= CODED;
      m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
      ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );
      ui++;
    }
  }
  m_apaucScanPosMap[uiPlane+1][uiMbIndex] = uiDCIdx;
  while( ( m_aapaucChromaDCCoefMap[uiPlane][uiDCIdx][uiMbIndex] & SIGNIFICANT ) && uiDCIdx < 4 )
    m_apaucScanPosMap[uiPlane+1][uiMbIndex] = ++uiDCIdx;
  
  return Err::m_nOK;
}






ErrVal
RQFGSDecoder::xDecodeNewCoefficientChromaAC ( UInt    uiPlane,
                                              UInt    uiB8YIdx,
                                              UInt    uiB8XIdx,
                                              Int&    riLastQP,
                                              UInt    uiChromaScanIndex )
{
  UInt uiB8Index  = (uiB8YIdx  ) * 2 * m_uiWidthInMB + (uiB8XIdx  );
  UInt uiMbIndex  = (uiB8YIdx/2) * 1 * m_uiWidthInMB + (uiB8XIdx/2);
  UInt uiCIdx     = 4*uiPlane + 2*(uiB8YIdx%2) + (uiB8XIdx%2);

  CIdx cChromaIdx(uiCIdx);


  UInt    uiScanIndex  = 16;
  UInt    uiStartIndex = 1;
  UInt    uiIndex;
  for( uiIndex = 1; uiIndex < 16; uiIndex++ )
  {
    if( !( m_aapaucChromaACCoefMap[uiPlane][uiIndex][uiB8Index] & SIGNIFICANT ) || ( m_aapaucChromaACCoefMap[uiPlane][uiIndex][uiB8Index] & NEWSIG ) )
    {
      uiStartIndex = uiIndex;
      break;
    }
  }
  uiScanIndex = m_apaucScanPosMap[uiPlane+3][uiB8Index];
  ROTRS(uiScanIndex == 16, Err::m_nOK);
  ROTRS(uiScanIndex > uiChromaScanIndex, Err::m_nOK);

  Bool bNeedEob = (uiScanIndex > uiStartIndex );

  MbDataAccess* pcMbDataAccessEL  = 0;
  MbDataAccess* pcMbDataAccessBL  = 0;
  RNOK( m_cMbDataCtrlEL     .initMb( pcMbDataAccessEL, uiB8YIdx/2, uiB8XIdx/2 ) );
  RNOK( m_pcCurrMbDataCtrl ->initMb( pcMbDataAccessBL, uiB8YIdx/2, uiB8XIdx/2 ) );
  

  if( ! ( m_pauiMacroblockMap[uiMbIndex] & CHROMA_CBP_CODED ) )
  {
    //===== CHROMA CBP =====
    Bool  bSigCBP = m_pcSymbolReader->RQdecodeCBP_Chroma( *pcMbDataAccessEL, *pcMbDataAccessBL );
    m_pauiMacroblockMap[uiMbIndex] |= CHROMA_CBP_CODED;
    
    if( !bSigCBP )
    {
      //----- chroma DC -----
      for( UInt uiCP = 0; uiCP < 2; uiCP++ )
      {
        m_apaucChromaDCBlockMap[uiCP][uiMbIndex] |= CODED;
        m_apaucScanPosMap[uiCP+1][uiMbIndex] = 4;

        for( UInt ui = 0; ui < 4; ui++ )
        {
          if( ! ( m_aapaucChromaDCCoefMap[uiCP][ui][uiMbIndex] & SIGNIFICANT ) )
          {
            m_aapaucChromaDCCoefMap[uiCP][ui][uiMbIndex] |= CODED;
            m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
            ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );
          }
        }
      }

      //----- chroma AC -----
      for( CIdx cCIdx; cCIdx.isLegal(); cCIdx++ )
      {
        UInt  uiCP  = cCIdx.plane();
        UInt  uiBlk = ( (uiB8YIdx/2)*2 + cCIdx.y() ) * 2 * m_uiWidthInMB + ( (uiB8XIdx/2)*2 + cCIdx.x() );

        m_apaucChromaACBlockMap[uiCP][uiBlk] |= CODED;
        m_apaucScanPosMap[uiCP+3][uiBlk] = 16;

        for( UInt ui = 1; ui < 16; ui++ )
        {
          if( ! ( m_aapaucChromaACCoefMap[uiCP][ui][uiBlk] & SIGNIFICANT ) )
          {
            m_aapaucChromaACCoefMap[uiCP][ui][uiBlk] |= CODED;
            m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
            ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );
          }
        }
      }

      return Err::m_nOK;
    }
    
    //===== DELTA QP =====
    if( ! ( m_pauiMacroblockMap[uiMbIndex] & CODED ) )
    {
      //===== delta QP =====
      if( ! ( m_pauiMacroblockMap[uiMbIndex] & SIGNIFICANT ) )
      {
        pcMbDataAccessEL->setLastQp( riLastQP );
        RNOK( m_pcSymbolReader->RQdecodeDeltaQp( *pcMbDataAccessEL ) );
        riLastQP = pcMbDataAccessEL->getMbData().getQp();

        m_pauiMacroblockMap[uiMbIndex] |= SIGNIFICANT;
      }
    }
  }



  if( ! ( m_pauiMacroblockMap[uiMbIndex] & CHROMA_CBP_AC_CODED ) )
  {
    //===== CHROMA CBP =====
    Bool  bSigCBP = m_pcSymbolReader->RQdecodeCBP_ChromaAC( *pcMbDataAccessEL, *pcMbDataAccessBL );
    m_pauiMacroblockMap[uiMbIndex] |= CHROMA_CBP_AC_CODED;
    
    if( !bSigCBP )
    {
      for( CIdx cCIdx; cCIdx.isLegal(); cCIdx++ )
      {
        UInt  uiCP  = cCIdx.plane();
        UInt  uiBlk = ( (uiB8YIdx/2)*2 + cCIdx.y() ) * 2 * m_uiWidthInMB + ( (uiB8XIdx/2)*2 + cCIdx.x() );

        m_apaucChromaACBlockMap[uiCP][uiBlk] |= CODED;
        m_apaucScanPosMap[uiCP+3][uiBlk] = 16;

        for( UInt ui = 1; ui < 16; ui++ )
        {
          if( ! ( m_aapaucChromaACCoefMap[uiCP][ui][uiBlk] & SIGNIFICANT ) )
          {
            m_aapaucChromaACCoefMap[uiCP][ui][uiBlk] |= CODED;
            m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
            ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );
          }
        }
      }

      return Err::m_nOK;
    }
  }



  if( ! ( m_apaucChromaACBlockMap[uiPlane][uiB8Index] & CODED ) )
  {
    Bool bSigBCBP = m_pcSymbolReader->RQdecodeBCBP_ChromaAC( *pcMbDataAccessBL, cChromaIdx );
    m_apaucChromaACBlockMap[uiPlane][uiB8Index] |= CODED;
    if(  bSigBCBP )
    {
      m_apaucChromaACBlockMap[uiPlane][uiB8Index] |= SIGNIFICANT;
    }
    if( ! bSigBCBP )
    {
      m_apaucScanPosMap[uiPlane+3][uiB8Index] = 16;
      for( UInt ui = 1; ui < 16; ui++ )
      {
        if( ! ( m_aapaucChromaACCoefMap[uiPlane][ui][uiB8Index] & SIGNIFICANT ) )
        {
          m_aapaucChromaACCoefMap[uiPlane][ui][uiB8Index] |= CODED;
          m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
          ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );
        }
      }
      return Err::m_nOK;
    }
  }


  UInt uiNumCoefRead;
  RNOK( m_pcSymbolReader->RQdecodeNewTCoeff_Chroma( *pcMbDataAccessEL, *pcMbDataAccessBL,
                                                    CHROMA_AC, cChromaIdx, uiScanIndex, bNeedEob, uiNumCoefRead ) );

  for ( UInt ui = 0; uiScanIndex < 16 && ( ui < uiNumCoefRead || bNeedEob ); uiScanIndex++ )
  {
    if( ! ( m_aapaucChromaACCoefMap[uiPlane][uiScanIndex][uiB8Index] & SIGNIFICANT ) )
    {
      if( pcMbDataAccessEL->getMbTCoeffs().get( cChromaIdx )[g_aucFrameScan[uiScanIndex]] )
      {
        m_aapaucChromaACCoefMap[uiPlane][uiScanIndex][uiB8Index] |= SIGNIFICANT;
        m_aapaucChromaACCoefMap[uiPlane][uiScanIndex][uiB8Index] |= NEWSIG;
      }
      m_aapaucChromaACCoefMap[uiPlane][uiScanIndex][uiB8Index] |= CODED;
      m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
      ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );
      ui++;
    }
  }
  m_apaucScanPosMap[uiPlane+3][uiB8Index] = uiScanIndex;
  while( ( m_aapaucChromaACCoefMap[uiPlane][uiScanIndex][uiB8Index] & SIGNIFICANT ) && uiScanIndex < 16 )
    m_apaucScanPosMap[uiPlane+3][uiB8Index] = ++uiScanIndex;
  
  return Err::m_nOK;
}







ErrVal
RQFGSDecoder::xDecodeCoefficientLumaRef( UInt   uiBlockYIndex,
                                         UInt   uiBlockXIndex,
                                         UInt   uiScanIndex )
{
  UInt    uiMbIndex     = (uiBlockYIndex/4) * 1 * m_uiWidthInMB + (uiBlockXIndex/4);
  UInt    uiBlockIndex  =  uiBlockYIndex    * 4 * m_uiWidthInMB +  uiBlockXIndex;

  UInt    uiB8x8        = ((uiBlockYIndex%4)/2) * 2 + ((uiBlockXIndex%4)/2);
  UInt    uiB4x4        =  (uiBlockYIndex%4)    * 4 +  (uiBlockXIndex%4);
  Par8x8  ePar8x8       = Par8x8(uiB8x8);
  
  B8x8Idx c8x8Idx(ePar8x8);
  B4x4Idx c4x4Idx(uiB4x4);

  //===== check if coefficient is not significant or was already coded =====
  ROFRS( m_apaucLumaCoefMap[uiScanIndex][uiBlockIndex] & SIGNIFICANT, Err::m_nOK );
  ROTRS( m_apaucLumaCoefMap[uiScanIndex][uiBlockIndex] & CODED,       Err::m_nOK );

  MbDataAccess* pcMbDataAccessEL  = 0;
  MbDataAccess* pcMbDataAccessBL  = 0;
  RNOK( m_cMbDataCtrlEL     .initMb( pcMbDataAccessEL, uiBlockYIndex/4, uiBlockXIndex/4 ) );
  RNOK( m_pcCurrMbDataCtrl ->initMb( pcMbDataAccessBL, uiBlockYIndex/4, uiBlockXIndex/4 ) );
  
  if( pcMbDataAccessBL->getMbData().isTransformSize8x8() )
  {
    UInt  ui8x8ScanIndex  = 4*uiScanIndex + 2*( uiBlockYIndex % 2 ) + ( uiBlockXIndex % 2 );
    RNOK( m_pcSymbolReader->RQdecodeTCoeffRef_8x8( *pcMbDataAccessEL, *pcMbDataAccessBL,
                                                  c8x8Idx, ui8x8ScanIndex ) );    
  }
  else
  {
    RNOK( m_pcSymbolReader->RQdecodeTCoeffRef_Luma( *pcMbDataAccessEL, *pcMbDataAccessBL, c4x4Idx, uiScanIndex ) );
  }

  m_apaucLumaCoefMap[uiScanIndex][uiBlockIndex] |= CODED;
  m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
  ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );


  return Err::m_nOK;
}


ErrVal
RQFGSDecoder::xDecodeCoefficientChromaDCRef ( UInt  uiPlane,
                                              UInt  uiMbYIdx,
                                              UInt  uiMbXIdx,
                                              UInt  uiDCIdx )
{
  UInt  uiMbIndex = uiMbYIdx * m_uiWidthInMB + uiMbXIdx;

  //===== check if coefficient is not significant or was already coded =====
  ROFRS( m_aapaucChromaDCCoefMap[uiPlane][uiDCIdx][uiMbIndex] & SIGNIFICANT, Err::m_nOK );
  ROTRS( m_aapaucChromaDCCoefMap[uiPlane][uiDCIdx][uiMbIndex] & CODED,       Err::m_nOK );

  MbDataAccess* pcMbDataAccessEL  = 0;
  MbDataAccess* pcMbDataAccessBL  = 0;
  RNOK( m_cMbDataCtrlEL     .initMb( pcMbDataAccessEL, uiMbYIdx, uiMbXIdx ) );
  RNOK( m_pcCurrMbDataCtrl ->initMb( pcMbDataAccessBL, uiMbYIdx, uiMbXIdx ) );
  
  RNOK( m_pcSymbolReader->RQdecodeTCoeffRef_Chroma( *pcMbDataAccessEL, *pcMbDataAccessBL,
                                                   CHROMA_DC, CIdx(4*uiPlane), uiDCIdx ) );

  m_aapaucChromaDCCoefMap[uiPlane][uiDCIdx][uiMbIndex] |= CODED;
  m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
  ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );

  return Err::m_nOK;
}
 

ErrVal
RQFGSDecoder::xDecodeCoefficientChromaACRef ( UInt  uiPlane,
                                              UInt  uiB8YIdx,
                                              UInt  uiB8XIdx,
                                              UInt  uiScanIdx )
{
  UInt  uiB8Index   = (uiB8YIdx  ) * 2 * m_uiWidthInMB + (uiB8XIdx  );
  UInt  uiMbIndex   = (uiB8YIdx/2) * 1 * m_uiWidthInMB + (uiB8XIdx/2);
  UInt  uiChromaIdx = 4*uiPlane + 2*(uiB8YIdx%2) + (uiB8XIdx%2);
  
  CIdx  cChromaIdx(uiChromaIdx);

  //===== check if coefficient is not significant or was already coded =====
  ROFRS( m_aapaucChromaACCoefMap[uiPlane][uiScanIdx][uiB8Index] & SIGNIFICANT, Err::m_nOK );
  ROTRS( m_aapaucChromaACCoefMap[uiPlane][uiScanIdx][uiB8Index] & CODED,       Err::m_nOK );

  MbDataAccess* pcMbDataAccessEL  = 0;
  MbDataAccess* pcMbDataAccessBL  = 0;
  RNOK( m_cMbDataCtrlEL     .initMb( pcMbDataAccessEL, uiB8YIdx/2, uiB8XIdx/2 ) );
  RNOK( m_pcCurrMbDataCtrl ->initMb( pcMbDataAccessBL, uiB8YIdx/2, uiB8XIdx/2 ) );
  
  RNOK( m_pcSymbolReader->RQdecodeTCoeffRef_Chroma( *pcMbDataAccessEL, *pcMbDataAccessBL,
                                                   CHROMA_AC, cChromaIdx, uiScanIdx ) );

  m_aapaucChromaACCoefMap[uiPlane][uiScanIdx][uiB8Index] |= CODED;
  m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
  ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );

  return Err::m_nOK;
}



H264AVC_NAMESPACE_END

