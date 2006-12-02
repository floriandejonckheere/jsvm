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

, m_bFirstFGS          ( true )
{
  m_pcCoefMap = NULL;

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

  m_bFirstFGS      = true;

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
RQFGSDecoder::xDecodingFGS( SliceHeader*                pcSliceHeader   )
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
          const UInt uiMbYIdx = uiMbAddress / m_uiWidthInMB;
          const UInt uiMbXIdx = uiMbAddress % m_uiWidthInMB;

          MbDataAccess* pcMbDataAccessEL = NULL, *pcMbDataAccessBL = NULL;
          RNOK( m_cMbDataCtrlEL    .initMb( pcMbDataAccessEL, uiMbYIdx, uiMbXIdx ) );
          RNOK( m_pcCurrMbDataCtrl->initMb( pcMbDataAccessBL, uiMbYIdx, uiMbXIdx ) );
          MbFGSCoefMap &rcMbFGSCoefMap = m_pcCoefMap[uiMbAddress];
          if( m_pcCurrSliceHeader->getAdaptivePredictionFlag() &&
            ! m_pcCurrMbDataCtrl->getMbData( uiMbXIdx, uiMbYIdx ).isIntra() && rcMbFGSCoefMap.getNumCoded() == 0 )
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
          RNOK( xDecodeNewCoefficientLumaMb( pcMbDataAccessBL, pcMbDataAccessEL, rcMbFGSCoefMap, uiMbYIdx, uiMbXIdx, iLastQP, iLumaScanIdx, uiMaxPosLuma ) );
          //===== CHROMA DC =====
          if( bAllowChromaDC && ( m_bFgsComponentSep == 0 || iLumaScanIdx == 16 ) )
          {
            for( CPlaneIdx cCPlaneIdx; cCPlaneIdx.isLegal(); ++cCPlaneIdx )
            {
              for( ui=iChromaDCScanIdx; ui<=uiMaxPosChromaDC && ui<4; ui++ )
              {
                if( ui == 0 || ui == rcMbFGSCoefMap.getChromaDCScanPos( cCPlaneIdx ) )
                  RNOK( xDecodeNewCoefficientChromaDC( pcMbDataAccessBL, pcMbDataAccessEL, rcMbFGSCoefMap, cCPlaneIdx, iLastQP, ui ) );
                CoefMap cCoefMap = m_pcCoefMap[uiMbAddress].getCoefMap( CIdx( cCPlaneIdx ) + ui )[0];
                if( (cCoefMap & SIGNIFICANT) && !(cCoefMap & CODED) )
                  RNOK( xDecodeCoefficientChromaDCRef( pcMbDataAccessBL, pcMbDataAccessEL, rcMbFGSCoefMap, cCPlaneIdx, ui ) );
              }
            } // for
          } // if

          //===== CHROMA AC =====
          if( bAllowChromaAC && ( m_bFgsComponentSep == 0 || iLumaScanIdx == 16 ) )
          {
            for( CPlaneIdx cCPlaneIdx; cCPlaneIdx.isLegal(); ++cCPlaneIdx )
            {
              for( UInt uiB8YIdx = 2 * uiMbYIdx; uiB8YIdx < 2 * uiMbYIdx + 2; uiB8YIdx++ )
              {
                for( UInt uiB8XIdx = 2 * uiMbXIdx; uiB8XIdx < 2 * uiMbXIdx + 2; uiB8XIdx++ )
                {
                  CIdx cCIdx = CIdx( cCPlaneIdx ) + ((2*(uiB8YIdx%2) + (uiB8XIdx%2)));
                  CoefMap *pcCoefMap = rcMbFGSCoefMap.getCoefMap( cCIdx );

                  for( ui=iChromaACScanIdx; ui<=uiMaxPosChromaAC && ui<16; ui++ )
                  {
                    if( ui == 1 || ui == rcMbFGSCoefMap.getChromaACScanPos( cCIdx ) )
                      RNOK( xDecodeNewCoefficientChromaAC( pcMbDataAccessBL, pcMbDataAccessEL, rcMbFGSCoefMap, cCIdx, iLastQP, ui ) );
                    if( (pcCoefMap[ui] & SIGNIFICANT) && !(pcCoefMap[ui] & CODED) )
                      RNOK( xDecodeCoefficientChromaACRef( pcMbDataAccessBL, pcMbDataAccessEL, rcMbFGSCoefMap, cCIdx, ui ) );
                  }
                } // for
              }
            }
          } // if
          RNOK( m_pcSymbolReader->RQupdateVlcTable() );

          //--ICU/ETRI FMO Implementation
          uiMbAddress = m_pcCurrSliceHeader->getFMO()->getNextMBNr( uiMbAddress );

        } // macroblock iteration
        RNOK( m_pcSymbolReader->RQvlcFlush() );

        iLumaScanIdx = min(uiMaxPosLuma+1, 16);
        if( m_bFgsComponentSep && iLumaScanIdx == 16 )
        {
          RNOK( m_pcSymbolReader->RQcompSepAlign() );
        }
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
RQFGSDecoder::xDecodeNewCoefficientLumaMb( MbDataAccess *pcMbDataAccessBL,
                                           MbDataAccess *pcMbDataAccessEL,
                                           MbFGSCoefMap &rcMbFGSCoefMap,
                                           UInt          uiMbYIdx,
                                           UInt  uiMbXIdx,
                                           Int&  riLastQp,
                                           Int   iLumaScanIdx,
                                           UInt  uiMaxPosLuma )
{
  ROFRS( iLumaScanIdx < 16, Err::m_nOK );

  for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
  {
    for( S4x4Idx cIdx( c8x8Idx ); cIdx.isLegal( c8x8Idx ); cIdx++ )
    {
      if( iLumaScanIdx == 0 )
      {
        RNOK( xDecodeSigHeadersLuma( pcMbDataAccessBL, pcMbDataAccessEL, rcMbFGSCoefMap, c8x8Idx, riLastQp ) );
      }

      for( UInt ui=iLumaScanIdx; ui<=uiMaxPosLuma && ui<16; ui++ )
      {
        while( rcMbFGSCoefMap.getLumaScanPos( cIdx ) <= ui )
        {
          RNOK( xDecodeNewCoefficientLuma( pcMbDataAccessBL, pcMbDataAccessEL, rcMbFGSCoefMap, cIdx ) );
        }
        RNOK( xDecodeCoefficientLumaRef( pcMbDataAccessBL, pcMbDataAccessEL, rcMbFGSCoefMap, cIdx, ui ) );
      }
    } // 4x4 block iteration
  } // 8x8 block iteration

  return Err::m_nOK;
}


ErrVal
RQFGSDecoder::xDecodeSigHeadersLuma( MbDataAccess  *pcMbDataAccessBL,
                                     MbDataAccess  *pcMbDataAccessEL,
                                     MbFGSCoefMap  &rcMbFGSCoefMap,
                                     const B8x8Idx &rc8x8Idx,
                                     Int&          riLastQp )
{
  if( !(rcMbFGSCoefMap.getB8x8Map( rc8x8Idx ) & CODED ) )
  {
    //===== CBP =====
    Bool bSigCBP = m_pcSymbolReader->RQdecodeCBP_8x8( *pcMbDataAccessEL, *pcMbDataAccessBL, rc8x8Idx );
    rcMbFGSCoefMap.getB8x8Map( rc8x8Idx ) |= CODED;
    if(  bSigCBP )
    {
      rcMbFGSCoefMap.getB8x8Map( rc8x8Idx ) |= SIGNIFICANT;
    }
    if( !bSigCBP )
    {
      //===== set coefficient and block map =====
      for( S4x4Idx cIdx( rc8x8Idx ); cIdx.isLegal( rc8x8Idx ); cIdx++ )
      {
        rcMbFGSCoefMap.getB4x4Map( cIdx ) |= CODED;
        rcMbFGSCoefMap.getLumaScanPos( cIdx ) = 16;
        CoefMap* pcCoefMap = rcMbFGSCoefMap.getCoefMap( cIdx );
        for( UInt ui = 0; ui < 16; ui++ )
        {
          if( ! ( pcCoefMap[ui] & SIGNIFICANT ) )
          {
            pcCoefMap[ui] |= CODED;
            RNOK( rcMbFGSCoefMap.increaseAndCheckNumCoded( 1 ) );
          }
        }
      }
      return Err::m_nOK;
    }

    //===== DELTA QP & TRANSFORM SIZE =====
    if( ! ( rcMbFGSCoefMap.getMbMap() & CODED ) )
    {
      //===== delta QP =====
      if( ! ( rcMbFGSCoefMap.getMbMap() & SIGNIFICANT ) )
      {
        pcMbDataAccessEL->setLastQp( riLastQp );
        RNOK( m_pcSymbolReader->RQdecodeDeltaQp( *pcMbDataAccessEL ) );
        riLastQp = pcMbDataAccessEL->getMbData().getQp();
        rcMbFGSCoefMap.getMbMap() |= SIGNIFICANT;
      }

      //===== transform size =====
      if( ! ( rcMbFGSCoefMap.getMbMap() & TRANSFORM_SPECIFIED ) )
      {
        RNOK( m_pcSymbolReader->RQdecode8x8Flag( *pcMbDataAccessEL, *pcMbDataAccessBL ) );

        rcMbFGSCoefMap.getMbMap() |= TRANSFORM_SPECIFIED;
      }
      rcMbFGSCoefMap.getMbMap() |= CODED;
    }
  }


  if( ! pcMbDataAccessBL->getMbData().isTransformSize8x8() )
  {
    for( S4x4Idx cIdx( rc8x8Idx ); cIdx.isLegal( rc8x8Idx ); cIdx++ )
    {
      UChar &rucB4x4Map = rcMbFGSCoefMap.getB4x4Map( cIdx );
      if( !(rucB4x4Map & CODED ) )
      {
        Bool bSigBCBP = m_pcSymbolReader->RQdecodeBCBP_4x4( *pcMbDataAccessBL, cIdx );
        rucB4x4Map |= CODED;
        if(  bSigBCBP )
          rucB4x4Map |= SIGNIFICANT;
        if( ! bSigBCBP )
        {
          rcMbFGSCoefMap.getLumaScanPos( cIdx ) = 16;
          CoefMap *pcCoefMap = rcMbFGSCoefMap.getCoefMap( cIdx );
          for( UInt ui = 0; ui < 16; ui++ )
          {
            if( ! ( pcCoefMap[ui] & SIGNIFICANT ) )
            {
              pcCoefMap[ui] |= CODED;
              RNOK( rcMbFGSCoefMap.increaseAndCheckNumCoded( 1 ) );
            }
          }
        }
      }
    }
  }

  return Err::m_nOK;
}


ErrVal
RQFGSDecoder::xDecodeNewCoefficientLuma( MbDataAccess  *pcMbDataAccessBL,
                                         MbDataAccess  *pcMbDataAccessEL,
                                         MbFGSCoefMap  &rcMbFGSCoefMap,
                                         const S4x4Idx &rcIdx )
{
  B8x8Idx c8x8Idx( rcIdx.getContainingPar8x8() );
  if( pcMbDataAccessBL->getMbData().isTransformSize8x8() )
  {
    UInt uiBlkIter8x8 = rcIdx.s4x4()&3;
    UInt uiStop         = 64;
    UInt ui8x8ScanIndex = rcMbFGSCoefMap.getLumaScanPos( rcIdx ) * 4 + uiBlkIter8x8;
    ROTRS( ui8x8ScanIndex >= uiStop, Err::m_nOK );

    CoefMap* pcCoefMap = rcMbFGSCoefMap.getCoefMap( c8x8Idx );
    UInt uiMbIndex = pcMbDataAccessBL->getMbY() * m_uiWidthInMB + pcMbDataAccessBL->getMbX();

    if (uiMbIndex != m_uiLastMbNum)
    {
      UInt uiMin    = 16;
      UInt uiRemain = 0;

      m_uiLastMbNum = uiMbIndex;
      for( S4x4Idx cIdx( c8x8Idx ); cIdx.isLegal( c8x8Idx ); cIdx++ )
      {
        uiMin = min( uiMin, rcMbFGSCoefMap.getLumaScanPos( cIdx ) );
        if( rcMbFGSCoefMap.getLumaScanPos( cIdx ) < 16 )
          uiRemain++;
      }
      if( ( uiRemain == 2 ) || ( uiMin > 10 && uiRemain == 3 ) )
      {
        Bool bIsEob   = false;
        m_pcSymbolReader->RQeo8b( bIsEob );
        if (bIsEob)
        {
          UInt ui8x8Index;
          for( ui8x8Index = 0; ui8x8Index < uiStop; ui8x8Index++ )
          {
            if( ! ( pcCoefMap[ui8x8Index] & SIGNIFICANT ) && ! ( pcCoefMap[ui8x8Index] & CODED ) )
            {
              pcCoefMap[ui8x8Index] |= CODED;
              RNOK( rcMbFGSCoefMap.increaseAndCheckNumCoded( 1 ) );
            }
          }
          for( S4x4Idx cIdx2( c8x8Idx ); cIdx2.isLegal( c8x8Idx ); cIdx2++ )
            rcMbFGSCoefMap.getLumaScanPos( cIdx2 ) = 16;

          ui8x8ScanIndex = 64 + uiBlkIter8x8;
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
      if( ! ( pcCoefMap[ui8x8ScanIndex] & SIGNIFICANT ) )
      {
        pcCoefMap[ui8x8ScanIndex] |= CODED;
        if( pcMbDataAccessEL->getMbTCoeffs().get8x8( c8x8Idx )[g_aucFrameScan64[ui8x8ScanIndex]] )
        {
          rcMbFGSCoefMap.getRefCtx( c8x8Idx )[ui8x8ScanIndex] = 1;
          pcCoefMap[ui8x8ScanIndex] |= SIGNIFICANT;
        }
        RNOK( rcMbFGSCoefMap.increaseAndCheckNumCoded( 1 ) );
        if( pcCoefMap[ui8x8ScanIndex] & SIGNIFICANT )
          pcCoefMap[ui8x8ScanIndex] |= NEWSIG;
        ui8x8++;
      }
    }
    if( rcMbFGSCoefMap.getLumaScanPos( rcIdx ) * 4 + uiBlkIter8x8 < ui8x8ScanIndex )
      rcMbFGSCoefMap.getLumaScanPos( rcIdx ) = ui8x8ScanIndex / 4;
    while( ui8x8ScanIndex < 64 && ( pcCoefMap[ui8x8ScanIndex] & SIGNIFICANT ) )
    {
      ui8x8ScanIndex += 4;
      rcMbFGSCoefMap.getLumaScanPos( rcIdx ) = ui8x8ScanIndex / 4;
    }
  }
  else
  {
    if (! m_pcSymbolReader->RQpeekCbp4x4( *pcMbDataAccessBL, rcIdx ) )
    {
      return Err::m_nOK;
    }

    UInt    uiStop       = 16;
    UInt    uiScanIndex  = uiStop;
    UInt    uiStartIndex = uiStop;
    UInt    uiIndex;
    CoefMap* pcCoefMap = rcMbFGSCoefMap.getCoefMap( rcIdx );

    for( uiIndex = 0; uiIndex < uiStop; uiIndex++ )
    {
      if( ! ( pcCoefMap[uiIndex] & SIGNIFICANT ) || ( pcCoefMap[uiIndex] & NEWSIG ) )
      {
        uiStartIndex = uiIndex;
        break;
      }
    }
    uiScanIndex = rcMbFGSCoefMap.getLumaScanPos( rcIdx );
    ROTRS(uiScanIndex == uiStop, Err::m_nOK);

    Bool bNeedEob = ( uiScanIndex > uiStartIndex );
    UInt uiNumCoefRead;
    RNOK( m_pcSymbolReader->RQdecodeNewTCoeff_Luma( *pcMbDataAccessEL, *pcMbDataAccessBL,
      LUMA_SCAN, rcIdx, uiScanIndex, bNeedEob, uiNumCoefRead ) );

    for ( UInt ui = 0; uiScanIndex < 16 && ( ui < uiNumCoefRead || bNeedEob ); uiScanIndex++ )
    {
      if( ! ( pcCoefMap[uiScanIndex] & SIGNIFICANT ) )
      {
        if( pcMbDataAccessEL->getMbTCoeffs().get( rcIdx )[g_aucFrameScan[uiScanIndex]] )
        {
          rcMbFGSCoefMap.getRefCtx( rcIdx )[uiScanIndex] = 1;
          pcCoefMap[uiScanIndex] |= SIGNIFICANT | NEWSIG;
        }
        pcCoefMap[uiScanIndex] |= CODED;
        RNOK( rcMbFGSCoefMap.increaseAndCheckNumCoded( 1 ) );
        ui++;
      }
    }
    rcMbFGSCoefMap.getLumaScanPos( rcIdx ) = uiScanIndex;
    while( uiScanIndex < 16 && ( pcCoefMap[uiScanIndex] & SIGNIFICANT ) )
      rcMbFGSCoefMap.getLumaScanPos( rcIdx ) = ++uiScanIndex;
  }

  return Err::m_nOK;
}



ErrVal
RQFGSDecoder::xDecodeNewCoefficientChromaDC ( MbDataAccess* pcMbDataAccessBL,
                                              MbDataAccess* pcMbDataAccessEL,
                                              MbFGSCoefMap& rcMbFGSCoefMap,
                                              const CPlaneIdx &rcCPlaneIdx,
                                              Int&    riLastQP,
                                              UInt    uiChromaScanIndex )
{
  UInt    uiDCIdx    = 4;
  UInt    uiStartIdx = 4;
  UInt    uiIndex;
  for( uiIndex = 0; uiIndex < 4; uiIndex++ )
  {
    CoefMap cCoefMap = rcMbFGSCoefMap.getCoefMap( CIdx( rcCPlaneIdx ) + uiIndex )[0];
    if( ! (cCoefMap & SIGNIFICANT) || (cCoefMap & NEWSIG) )
    {
      uiStartIdx = uiIndex;
      break;
    }
  }
  uiDCIdx = rcMbFGSCoefMap.getChromaDCScanPos( rcCPlaneIdx );
  ROTRS(uiDCIdx == 4, Err::m_nOK);
  ROTRS(uiDCIdx > uiChromaScanIndex, Err::m_nOK);

  Bool bNeedEob = ( uiDCIdx > uiStartIdx );

  if( ! ( rcMbFGSCoefMap.getMbMap() & CHROMA_CBP_CODED ) )
  {
    //===== CHROMA CBP =====
    Bool  bSigCBP = m_pcSymbolReader->RQdecodeCBP_Chroma( *pcMbDataAccessEL, *pcMbDataAccessBL );
    rcMbFGSCoefMap.getMbMap() |= CHROMA_CBP_CODED;

    if( !bSigCBP )
    {
      //----- chroma DC -----
      for( CPlaneIdx cCPlaneIdx2; cCPlaneIdx2.isLegal(); ++cCPlaneIdx2 )
      {
        rcMbFGSCoefMap.getChromaDCMbMap( cCPlaneIdx2 ) |= CODED;
        rcMbFGSCoefMap.getChromaDCScanPos( cCPlaneIdx2 ) = 4;
        for( CIdx cCIdx( cCPlaneIdx2 ); cCIdx.isLegal( cCPlaneIdx2 ); cCIdx++ )
        {
          CoefMap &rcCoefMap = rcMbFGSCoefMap.getCoefMap( cCIdx )[0];
          if( ! (rcCoefMap & SIGNIFICANT) )
          {
            rcCoefMap |= CODED;
            RNOK( rcMbFGSCoefMap.increaseAndCheckNumCoded( 1 ) );
          }
        }
      }
      //----- chroma AC -----
      for( CIdx cCIdx; cCIdx.isLegal(); cCIdx++ )
      {
        rcMbFGSCoefMap.getChromaACBlockMap( cCIdx ) |= CODED;
        rcMbFGSCoefMap.getChromaACScanPos( cCIdx ) = 16;
        CoefMap *pcCoefMap = rcMbFGSCoefMap.getCoefMap( cCIdx );
        for( UInt ui = 1; ui < 16; ui++ )
        {
          if( !(pcCoefMap[ui] & SIGNIFICANT) )
          {
            pcCoefMap[ui] |= CODED;
            RNOK( rcMbFGSCoefMap.increaseAndCheckNumCoded( 1 ) );
          }
        }
      }

      return Err::m_nOK;
    }

    //===== DELTA QP =====
    if( ! ( rcMbFGSCoefMap.getMbMap() & CODED ) )
    {
      //===== delta QP =====
      if( ! ( rcMbFGSCoefMap.getMbMap() & SIGNIFICANT ) )
      {
        pcMbDataAccessEL->setLastQp( riLastQP );
        RNOK( m_pcSymbolReader->RQdecodeDeltaQp( *pcMbDataAccessEL ) );
        riLastQP = pcMbDataAccessEL->getMbData().getQp();

        rcMbFGSCoefMap.getMbMap() |= SIGNIFICANT;
      }
    }
  }

  if( !(rcMbFGSCoefMap.getChromaDCMbMap( rcCPlaneIdx ) & CODED ) )
  {
    Bool bSigBCBP = m_pcSymbolReader->RQdecodeBCBP_ChromaDC( *pcMbDataAccessBL, CIdx( rcCPlaneIdx ) );
    rcMbFGSCoefMap.getChromaDCMbMap( rcCPlaneIdx ) |= CODED;
    if(  bSigBCBP )
      rcMbFGSCoefMap.getChromaDCMbMap( rcCPlaneIdx ) |= SIGNIFICANT;
    if( ! bSigBCBP )
    {
      rcMbFGSCoefMap.getChromaDCScanPos( rcCPlaneIdx ) = 4;
      for( CIdx cCIdx( rcCPlaneIdx ); cCIdx.isLegal( rcCPlaneIdx ); cCIdx++ )
      {
        CoefMap &rcCoefMap = rcMbFGSCoefMap.getCoefMap( cCIdx )[0];
        if( !(rcCoefMap & SIGNIFICANT) )
        {
          rcCoefMap |= CODED;
          RNOK( rcMbFGSCoefMap.increaseAndCheckNumCoded( 1 ) );
        }
      }
      return Err::m_nOK;
    }
  }


  // Encode EOB marker?
  UInt uiNumCoefRead;
  RNOK( m_pcSymbolReader->RQdecodeNewTCoeff_Chroma( *pcMbDataAccessEL, *pcMbDataAccessBL,
                                                    CHROMA_DC, CIdx(rcCPlaneIdx), uiDCIdx, bNeedEob, uiNumCoefRead ) );
  for ( UInt ui = 0; uiDCIdx < 4 && ( ui < uiNumCoefRead || bNeedEob ); uiDCIdx++ )
  {
    CoefMap &rcCoefMap = rcMbFGSCoefMap.getCoefMap( CIdx( rcCPlaneIdx ) + uiDCIdx )[0];
    if( !(rcCoefMap & SIGNIFICANT) )
    {
      if( pcMbDataAccessEL->getMbTCoeffs().get( CIdx( rcCPlaneIdx ) + uiDCIdx )[0] )
      {
        rcMbFGSCoefMap.getRefCtx( CIdx( rcCPlaneIdx ) + uiDCIdx )[0] = 1;
        rcCoefMap |= SIGNIFICANT | NEWSIG;
      }
      rcCoefMap |= CODED;
      RNOK( rcMbFGSCoefMap.increaseAndCheckNumCoded( 1 ) );
      ui++;
    }
  }
  rcMbFGSCoefMap.getChromaDCScanPos( rcCPlaneIdx ) = uiDCIdx;
  while( (rcMbFGSCoefMap.getCoefMap( CIdx( rcCPlaneIdx ) + uiDCIdx )[0] & SIGNIFICANT ) && uiDCIdx < 4 )
    rcMbFGSCoefMap.getChromaDCScanPos( rcCPlaneIdx ) = ++uiDCIdx;

  return Err::m_nOK;
}






ErrVal
RQFGSDecoder::xDecodeNewCoefficientChromaAC ( MbDataAccess* pcMbDataAccessBL,
                                              MbDataAccess* pcMbDataAccessEL,
                                              MbFGSCoefMap& rcMbFGSCoefMap,
                                              const CIdx   &rcCIdx,
                                              Int&    riLastQP,
                                              UInt    uiChromaScanIndex )
{
  UInt    uiScanIndex  = 16;
  UInt    uiStartIndex = 1;
  UInt    uiIndex;
  CoefMap *pcCoefMap = rcMbFGSCoefMap.getCoefMap( rcCIdx );
  for( uiIndex = 1; uiIndex < 16; uiIndex++ )
  {
    if( !(pcCoefMap[uiIndex] & SIGNIFICANT) || (pcCoefMap[uiIndex] & NEWSIG) )
    {
      uiStartIndex = uiIndex;
      break;
    }
  }
  uiScanIndex = rcMbFGSCoefMap.getChromaACScanPos( rcCIdx );
  ROTRS(uiScanIndex == 16, Err::m_nOK);
  ROTRS(uiScanIndex > uiChromaScanIndex, Err::m_nOK);

  Bool bNeedEob = (uiScanIndex > uiStartIndex );

  if( !(rcMbFGSCoefMap.getMbMap() & CHROMA_CBP_CODED) )
  {
    //===== CHROMA CBP =====
    Bool  bSigCBP = m_pcSymbolReader->RQdecodeCBP_Chroma( *pcMbDataAccessEL, *pcMbDataAccessBL );
    rcMbFGSCoefMap.getMbMap() |= CHROMA_CBP_CODED;
    if( !bSigCBP )
    {
      //----- chroma DC -----
      for( CPlaneIdx cCPlaneIdx2; cCPlaneIdx2.isLegal(); ++cCPlaneIdx2 )
      {
        rcMbFGSCoefMap.getChromaDCMbMap( cCPlaneIdx2 ) |= CODED;
        rcMbFGSCoefMap.getChromaDCScanPos( cCPlaneIdx2 ) = 4;

        for( CIdx cCIdx( cCPlaneIdx2 ); cCIdx.isLegal( cCPlaneIdx2 ); cCIdx++ )
        {
          CoefMap &rcCoefMap = rcMbFGSCoefMap.getCoefMap( cCIdx )[0];
          if( !( rcCoefMap & SIGNIFICANT ) )
          {
            rcCoefMap |= CODED;
            RNOK( rcMbFGSCoefMap.increaseAndCheckNumCoded( 1 ) );
          }
        }
      }
      //----- chroma AC -----
      for( CIdx cCIdx; cCIdx.isLegal(); cCIdx++ )
      {
        rcMbFGSCoefMap.getChromaACBlockMap( cCIdx ) |= CODED;
        rcMbFGSCoefMap.getChromaACScanPos( cCIdx ) = 16;
        CoefMap *pcCoefMap2 = rcMbFGSCoefMap.getCoefMap( cCIdx );
        for( UInt ui = 1; ui < 16; ui++ )
        {
          if( !(pcCoefMap2[ui] & SIGNIFICANT) )
          {
            pcCoefMap2[ui] |= CODED;
            RNOK( rcMbFGSCoefMap.increaseAndCheckNumCoded( 1 ) );
          }
        }
      }

      return Err::m_nOK;
    }

    //===== DELTA QP =====
    if( !(rcMbFGSCoefMap.getMbMap() & CODED ) )
    {
      //===== delta QP =====
      if( !(rcMbFGSCoefMap.getMbMap() & SIGNIFICANT ) )
      {
        pcMbDataAccessEL->setLastQp( riLastQP );
        RNOK( m_pcSymbolReader->RQdecodeDeltaQp( *pcMbDataAccessEL ) );
        riLastQP = pcMbDataAccessEL->getMbData().getQp();
        rcMbFGSCoefMap.getMbMap() |= SIGNIFICANT;
      }
    }
  }


  if( !(rcMbFGSCoefMap.getMbMap() & CHROMA_CBP_AC_CODED) )
  {
    //===== CHROMA CBP =====
    Bool  bSigCBP = m_pcSymbolReader->RQdecodeCBP_ChromaAC( *pcMbDataAccessEL, *pcMbDataAccessBL );
    rcMbFGSCoefMap.getMbMap() |= CHROMA_CBP_AC_CODED;
    if( !bSigCBP )
    {
      for( CIdx cCIdx; cCIdx.isLegal(); cCIdx++ )
      {
        rcMbFGSCoefMap.getChromaACBlockMap( cCIdx ) |= CODED;
        rcMbFGSCoefMap.getChromaACScanPos( cCIdx ) = 16;
        CoefMap *pcCoefMap2 = rcMbFGSCoefMap.getCoefMap( cCIdx );
        for( UInt ui = 1; ui < 16; ui++ )
        {
          if( !(pcCoefMap2[ui] & SIGNIFICANT) )
          {
            pcCoefMap2[ui] |= CODED;
            RNOK( rcMbFGSCoefMap.increaseAndCheckNumCoded( 1 ) );
          }
        }
      }

      return Err::m_nOK;
    }
  }

  if( !(rcMbFGSCoefMap.getChromaACBlockMap( rcCIdx ) & CODED ) )
  {
    Bool bSigBCBP = m_pcSymbolReader->RQdecodeBCBP_ChromaAC( *pcMbDataAccessBL, rcCIdx );
    rcMbFGSCoefMap.getChromaACBlockMap( rcCIdx ) |= CODED;
    if(  bSigBCBP )
    {
      rcMbFGSCoefMap.getChromaACBlockMap( rcCIdx ) |= SIGNIFICANT;
    }
    if( ! bSigBCBP )
    {
      rcMbFGSCoefMap.getChromaACScanPos( rcCIdx ) = 16;
      CoefMap *pcCoefMap2 = rcMbFGSCoefMap.getCoefMap( rcCIdx );
      for( UInt ui = 1; ui < 16; ui++ )
      {
        if( !(pcCoefMap2[ui] & SIGNIFICANT) )
        {
          pcCoefMap2[ui] |= CODED;
          RNOK( rcMbFGSCoefMap.increaseAndCheckNumCoded( 1 ) );
        }
      }
      return Err::m_nOK;
    }
  }


  UInt uiNumCoefRead;
  RNOK( m_pcSymbolReader->RQdecodeNewTCoeff_Chroma( *pcMbDataAccessEL, *pcMbDataAccessBL,
    CHROMA_AC, rcCIdx, uiScanIndex, bNeedEob, uiNumCoefRead ) );
  for ( UInt ui = 0; uiScanIndex < 16 && ( ui < uiNumCoefRead || bNeedEob ); uiScanIndex++ )
  {
    if( !(pcCoefMap[uiScanIndex] & SIGNIFICANT) )
    {
      if( pcMbDataAccessEL->getMbTCoeffs().get( rcCIdx )[g_aucFrameScan[uiScanIndex]] )
      {
        rcMbFGSCoefMap.getRefCtx( rcCIdx )[uiScanIndex] = 1;
        pcCoefMap[uiScanIndex] |= SIGNIFICANT | NEWSIG;
      }
      pcCoefMap[uiScanIndex] |= CODED;
      RNOK( rcMbFGSCoefMap.increaseAndCheckNumCoded( 1 ) );
      ui++;
    }
  }
  rcMbFGSCoefMap.getChromaACScanPos( rcCIdx ) = uiScanIndex;
  while( (pcCoefMap[uiScanIndex] & SIGNIFICANT ) && uiScanIndex < 16 )
    rcMbFGSCoefMap.getChromaACScanPos( rcCIdx ) = ++uiScanIndex;

  return Err::m_nOK;
}







ErrVal
RQFGSDecoder::xDecodeCoefficientLumaRef( MbDataAccess* pcMbDataAccessBL,
                                         MbDataAccess* pcMbDataAccessEL,
                                         MbFGSCoefMap  &rcMbFGSCoefMap,
                                         const S4x4Idx &rcIdx,
                                         UInt   uiScanIndex )
{
  B8x8Idx c8x8Idx( rcIdx.getContainingPar8x8() );
  CoefMap* pcCoefMap;
  RefCtx*  pcRefCtx;
  if( pcMbDataAccessBL->getMbData().isTransformSize8x8() )
  {
    uiScanIndex = 4*uiScanIndex + (rcIdx.s4x4() & 3); // convert scan index into 8x8 scan index
    pcCoefMap = &rcMbFGSCoefMap.getCoefMap( c8x8Idx )[uiScanIndex];
    pcRefCtx  = &rcMbFGSCoefMap.getRefCtx( c8x8Idx )[uiScanIndex];
  }
  else
  {
    pcCoefMap = &rcMbFGSCoefMap.getCoefMap( rcIdx )[uiScanIndex];
    pcRefCtx  = &rcMbFGSCoefMap.getRefCtx( rcIdx )[uiScanIndex];
  }

  //===== check if coefficient is not significant or was already coded =====
  ROFRS( *pcCoefMap & SIGNIFICANT, Err::m_nOK );
  ROTRS( *pcCoefMap & CODED,       Err::m_nOK );

  Int iCoeff;

  if( pcMbDataAccessBL->getMbData().isTransformSize8x8() )
  {
    RNOK( m_pcSymbolReader->RQdecodeTCoeffRef_8x8( *pcMbDataAccessEL, *pcMbDataAccessBL, c8x8Idx, uiScanIndex, *pcRefCtx ) );
    iCoeff = pcMbDataAccessEL->getMbTCoeffs().get8x8( c8x8Idx )[g_aucFrameScan64[uiScanIndex]];
    if( pcMbDataAccessBL->getMbTCoeffs().get8x8( c8x8Idx )[g_aucFrameScan64[uiScanIndex]] < 0 )
      iCoeff = -iCoeff;
  }
  else
  {
    RNOK( m_pcSymbolReader->RQdecodeTCoeffRef_Luma( *pcMbDataAccessEL, *pcMbDataAccessBL, rcIdx, uiScanIndex, *pcRefCtx ) );
    iCoeff = pcMbDataAccessEL->getMbTCoeffs().get( rcIdx )[g_aucFrameScan[uiScanIndex]];
    if( pcMbDataAccessBL->getMbTCoeffs().get( rcIdx )[g_aucFrameScan[uiScanIndex]] < 0 )
      iCoeff = - iCoeff;
  }
  (*pcRefCtx) <<= 2;
  if( iCoeff < 0 )
    (*pcRefCtx)+= 2;
  else if( iCoeff > 0 )
    (*pcRefCtx)++;


  *pcCoefMap |= CODED;
  RNOK( rcMbFGSCoefMap.increaseAndCheckNumCoded( 1 ) );

  return Err::m_nOK;
}


ErrVal
RQFGSDecoder::xDecodeCoefficientChromaDCRef ( MbDataAccess    *pcMbDataAccessBL,
                                              MbDataAccess    *pcMbDataAccessEL,
                                              MbFGSCoefMap    &rcMbFGSCoefMap,
                                              const CPlaneIdx &rcCPlaneIdx,
                                              UInt  uiDCIdx )
{
  CIdx cCIdx = CIdx( rcCPlaneIdx ) + uiDCIdx;
  CoefMap &rcCoefMap = rcMbFGSCoefMap.getCoefMap( cCIdx )[0];
  ROF( rcCoefMap & SIGNIFICANT );
  ROT( rcCoefMap & CODED       );

  RefCtx &rcRefCtx = rcMbFGSCoefMap.getRefCtx( cCIdx )[0];

  RNOK( m_pcSymbolReader->RQdecodeTCoeffRef_Chroma( *pcMbDataAccessEL, *pcMbDataAccessBL,
    CHROMA_DC, CIdx(rcCPlaneIdx), uiDCIdx, rcRefCtx ) );
  Int iCoeff = pcMbDataAccessEL->getMbTCoeffs().get( cCIdx )[0];
  if( pcMbDataAccessBL->getMbTCoeffs().get( cCIdx )[0] < 0 )
    iCoeff = -iCoeff;

  rcRefCtx <<= 2;
  if( iCoeff < 0 )
    rcRefCtx += 2;
  else if( iCoeff > 0 )
    rcRefCtx++;

  rcCoefMap |= CODED;
  RNOK( rcMbFGSCoefMap.increaseAndCheckNumCoded( 1 ) );
  return Err::m_nOK;
}


ErrVal
RQFGSDecoder::xDecodeCoefficientChromaACRef ( MbDataAccess  *pcMbDataAccessBL,
                                              MbDataAccess  *pcMbDataAccessEL,
                                              MbFGSCoefMap  &rcMbFGSCoefMap,
                                              const CIdx    &rcCIdx,
                                              UInt  uiScanIdx )
{
  CoefMap &rcCoefMap = rcMbFGSCoefMap.getCoefMap( rcCIdx )[uiScanIdx];
  ROF( rcCoefMap & SIGNIFICANT );
  ROT( rcCoefMap & CODED       );

  RefCtx &rcRefCtx = rcMbFGSCoefMap.getRefCtx( rcCIdx )[uiScanIdx];

  RNOK( m_pcSymbolReader->RQdecodeTCoeffRef_Chroma( *pcMbDataAccessEL, *pcMbDataAccessBL,
    CHROMA_AC, rcCIdx, uiScanIdx, rcRefCtx ) );
  Int iCoeff = pcMbDataAccessEL->getMbTCoeffs().get( rcCIdx )[g_aucFrameScan[uiScanIdx]];
  if( pcMbDataAccessBL->getMbTCoeffs().get( rcCIdx )[g_aucFrameScan[uiScanIdx]] < 0 )
    iCoeff = -iCoeff;

  rcRefCtx <<= 2;
  if( iCoeff < 0 )
    rcRefCtx += 2;
  else if( iCoeff > 0 )
    rcRefCtx++;

  rcCoefMap |= CODED;
  RNOK( rcMbFGSCoefMap.increaseAndCheckNumCoded( 1 ) );

  return Err::m_nOK;
}



H264AVC_NAMESPACE_END

