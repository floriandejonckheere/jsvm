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
, m_pcMbParser                ( 0 )
, m_pcMbDecoder               ( 0 )

, m_bFirstFGS				  ( true )
{
  m_pcCoefMap = NULL;
  m_pcSliceHeader       = 0;

  for (int i = 0; i < MAX_LAYERS; ++i ) m_bFGSUse[i] = false;
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

  m_pcSliceHeader             = NULL;
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
  
  RNOK( xUninit() );

  m_pcMbParser                = 0;
  m_pcMbDecoder               = 0;
  m_pcCabacReader             = 0;
  m_pcSymbolReader            = 0;
  m_pcUvlcReader              = 0;
  m_pcSliceHeader             = 0;
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
  m_uiHeightInMB      = ( pcSliceHeader->getFieldPicFlag() ) ? m_uiHeightInMB>>1 : m_uiHeightInMB;
  m_pcSliceHeader     = pcSliceHeader;
  m_pcCurrMbDataCtrl  = pcCurrMbDataCtrl;
  m_bPicInit          = true;
  m_bPicChanged       = false;
  m_bPicFinished      = false;

  for(int i = 0; i < MAX_LAYERS; ++i ) m_bFGSUse[i] = false;

  Bool bCabac         = pcSliceHeader->getPPS().getEntropyCodingModeFlag();
  m_pcSymbolReader    = ( bCabac ) ? (MbSymbolReadIf*)m_pcCabacReader : (MbSymbolReadIf*)m_pcUvlcReader;

  RNOK( m_pcCurrMbDataCtrl->initSlice( *pcSliceHeader, PRE_PROCESS, true, NULL ) );
  
  RNOK( xInitBaseLayerSbb( m_pcSliceHeader->getLayerId() ) );
  RNOK( xInitializeCodingPath(pcSliceHeader) );
    RNOK( xScaleBaseLayerCoeffs( true ) );

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
  m_pcSliceHeader     = 0;
  m_pcCurrMbDataCtrl  = 0;

  m_bFirstFGS		  = true;

  return Err::m_nOK;
}

Bool gbParallelDecoding = false;

ErrVal
RQFGSDecoder::decodeNextLayer( SliceHeader* pcSliceHeader )
{
  ROF( m_bInit );
  ROF( m_bPicInit );
  
  //===== update slice header =====
  m_pcSliceHeader->setSliceHeaderQp ( pcSliceHeader->getPicQp()          );
  m_pcSliceHeader->setFirstMbInSlice( pcSliceHeader->getFirstMbInSlice() );
  m_pcSliceHeader->setNumMbsInSlice ( pcSliceHeader->getNumMbsInSlice () );
  // JVT-S054 (2) (ADD)
  m_pcSliceHeader->setLastMbInSlice ( pcSliceHeader->getLastMbInSlice () );
  m_pcSliceHeader->setAdaptivePredictionFlag( pcSliceHeader->getAdaptivePredictionFlag() );
  m_bPicChanged = true;

  // if parallel decoding, return gracefully when the bitstream buffer end is reached; 
  // otherwise, throw an exception when the bitstream buffer end is reached
  gbParallelDecoding = pcSliceHeader->getFGSCycleAlignedFragment(); 


  m_bFGSUse[m_pcSliceHeader->getLayerId()] = true;

  if( pcSliceHeader->getFGSCycleAlignedFragment() )
  {
    RNOK( xDecodingFGSBlock(pcSliceHeader) );
  }
  else
  {
  RNOK( xDecodingFGS(pcSliceHeader) );
  }

  return Err::m_nOK;
}






static UInt gauiB8x8Mapping[4] = { 0, 2, 3, 1 }; 

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
        RNOKS( ((UvlcReader *) m_pcSymbolReader)->getUvlc(m_uiLumaCbpRun, "Luma_CBP_run") );
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
    RNOKS( ((UvlcReader *) m_pcSymbolReader)->getFlag(bTransitionFlag, "Chroma_CBP_transition") );
    m_uiLastChromaCbp = auiNextChromaCbp[m_uiLastChromaCbp][bTransitionFlag];

    RNOKS( ((UvlcReader *) m_pcSymbolReader)->getUvlc(m_uiChromaCbpRun, "Chroma_CBP_run") );
    m_uiChromaCbpRun ++;
  }

  uiMbCbp = pcMbDataAccessEL->getMbData().getMbCbp();
  pcMbDataAccessEL->getMbData().setMbCbp( (uiMbCbp & 0x0F) | (m_uiLastChromaCbp << 4) );
  m_uiChromaCbpRun --;

  return Err::m_nOK;
}

ErrVal
RQFGSDecoder::xDecodeMbHeader( MbDataAccess*      pcMbDataAccessBL,
                               MbDataAccess*      pcMbDataAccessEL,
                               MbFGSCoefMap       &rcMbFGSCoefMap,
                               Int&               riLastQp,
                               SliceHeader*       pcSliceHeader)
{
  UInt    uiMbX = pcMbDataAccessBL->getMbX();
  UInt    uiMbY = pcMbDataAccessBL->getMbY();
  UInt    uiCbpBit;
  MbSymbolReadIf* pcMbHeaderReader;

  pcMbHeaderReader =  m_pcSymbolReader->RQactivateFragment( 0 );

  const Bool    bMbAff   = m_pcSliceHeader->isMbAff   ();// TMM_INTERLACE
  if( m_pcSliceHeader->getAdaptivePredictionFlag() &&
    ! pcMbDataAccessBL->getMbData().isIntra() )
  {
    // the error is not handled if it is termnated early
    RNOKS( m_pcMbParser ->readMotion( *pcMbDataAccessEL, pcMbDataAccessBL ) );
    RNOK ( m_pcMbDecoder->calcMv    ( *pcMbDataAccessEL, pcMbDataAccessBL ) );

    if( ! pcMbDataAccessEL->getMbData().getBLSkipFlag() && ! pcMbDataAccessEL->getMbData().getResidualPredFlag( PART_16x16 ) )
    {
      //----- motion refinement without residual prediction ===> clear base layer coeffs -----
      UInt            uiLayer         = m_pcSliceHeader->getLayerId();
      YuvBufferCtrl*  pcYuvBufferCtrl = m_papcYuvFullPelBufferCtrl[uiLayer];
      RNOK( pcYuvBufferCtrl->initMb( uiMbY, uiMbX, bMbAff ) );
      RNOK( xClearBaseCoeffs( *pcMbDataAccessEL, pcMbDataAccessBL ) );
    }
  }

  //===== CBP =====
  if( ! m_pcSliceHeader->getPPS().getEntropyCodingModeFlag() ) {
    RNOKS( xDecodeLumaCbpVlc   ( uiMbX, uiMbY ) );
    RNOKS( xDecodeChromaCbpVlc ( uiMbX, uiMbY ) );

    // restore pcMbDataAccessBL and pcMbDataAccessEL, may not be necessary
    RNOK( m_pcCurrMbDataCtrl ->initMb( pcMbDataAccessBL, uiMbY, uiMbX ) );
    RNOK( m_cMbDataCtrlEL     .initMb( pcMbDataAccessEL, uiMbY, uiMbX ) );
  }

  if( ! m_pcSliceHeader->getPPS().getEntropyCodingModeFlag() )
  {
//decide the VLC table to be used for refinement coefficients for current MB
  Bool bLowPass=(pcSliceHeader->getTemporalLevel()==0);
  Bool isIntra  = m_pcCurrMbDataCtrl->getMbData( uiMbX, uiMbY ).isIntra();
  ((UvlcReader *)m_pcSymbolReader)->setQLevel(pcSliceHeader->getQualityLevel());
  ((UvlcReader *)m_pcSymbolReader)->setMBMode(bLowPass || isIntra);
  ((UvlcReader *)m_pcSymbolReader)->decideTable();

//Read the codetype here
  rcMbFGSCoefMap.getMbMapH() = -1;		//very imp to initialize code_type to -1
  Bool useCT_Intra = ((UvlcReader *)m_pcSymbolReader)->useCodeTypeIntra;
  Bool useCT_Inter = ((UvlcReader *)m_pcSymbolReader)->useCodeTypeInter;
  if ((useCT_Intra==1 && useCT_Inter==1))
      xReadCodeTypeRef(pcSliceHeader,rcMbFGSCoefMap,uiMbX,uiMbY);
  }

  // Luma CBP in CABAC, need also for CAVLC to update the CBP in buffer
  {
    for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx ++ )
    RNOKS( pcMbHeaderReader->RQdecodeCBP_8x8( *pcMbDataAccessEL, *pcMbDataAccessBL, c8x8Idx ) );
  }

  // CHROMA CBP in CABAC, need also for CAVLC to update the CBP in buffer
  RNOKS( pcMbHeaderReader->RQdecodeCBP_Chroma( *pcMbDataAccessEL, *pcMbDataAccessBL, uiCbpBit ) );
  if( uiCbpBit )
    RNOKS( pcMbHeaderReader->RQdecodeCBP_ChromaAC( *pcMbDataAccessEL, *pcMbDataAccessBL, uiCbpBit ) );

  // transform size
  if( ( pcMbDataAccessBL->getMbData().getMbCbp() & 15 ) &&
    ! ( rcMbFGSCoefMap.getMbMap() & TRANSFORM_SPECIFIED ) ) {
    RNOKS( pcMbHeaderReader->RQdecode8x8Flag( *pcMbDataAccessEL, *pcMbDataAccessBL ) );
    rcMbFGSCoefMap.getMbMap() |= TRANSFORM_SPECIFIED;
  }

  // delta QP and transform flag
  if( pcMbDataAccessBL->getMbData().getMbCbp() != 0 ) {
    if( ! ( rcMbFGSCoefMap.getMbMap() & SIGNIFICANT ) ) {
      pcMbDataAccessEL->setLastQp( riLastQp );
      RNOKS( pcMbHeaderReader->RQdecodeDeltaQp( *pcMbDataAccessEL ) );
      riLastQp = pcMbDataAccessEL->getMbData().getQp();

      rcMbFGSCoefMap.getMbMap() |= SIGNIFICANT;
    }
  }

  // Luma BCBP
  {
    for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx ++ ) {
      Bool bSigBCBP = ( ( pcMbDataAccessEL->getMbData().getMbCbp() >> c8x8Idx.b8x8Index() ) & 1 ? 1 : 0 );
      Bool b8x8     = pcMbDataAccessBL->getMbData().isTransformSize8x8();

      // 8x8 in VLC mode is de-interleaved into 4 4x4 blocks
      // 8x8 in CABAC mode is encoded in native 8x8 zigzag order, and no BCBP is needed
      if( bSigBCBP && (! m_pcSliceHeader->getPPS().getEntropyCodingModeFlag() || ! b8x8 ) ) {
        for( S4x4Idx cIdx( c8x8Idx ); cIdx.isLegal( c8x8Idx ); cIdx ++ )
          RNOKS( pcMbHeaderReader->RQdecodeBCBP_4x4( *pcMbDataAccessEL, *pcMbDataAccessBL, b8x8, cIdx, uiCbpBit ) );
      }
      else
      {
        S4x4Idx cIdx (c8x8Idx);

        rcMbFGSCoefMap.getLumaScanPos( cIdx+1 ) = 64; 
      }
    }
  }

  // do not track the number of coefficients in the decoder
  xUpdateMbMaps( pcMbDataAccessBL, pcMbDataAccessEL, rcMbFGSCoefMap, 0 );

  return Err::m_nOK;
}

const UChar g_aucLinearScan[16] =
{
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15
};


ErrVal
RQFGSDecoder::xResidualBlock        ( MbDataAccess&   rcMbDataAccess,
                                      MbDataAccess&   rcMbDataAccessBase,
                                      ResidualMode    eResidualMode,
                                      UInt            uiStride,
                                      UInt            uiBlkIdx,
                                      UInt&           uiBcbp,
                                      Bool            bDecodeBcbpInside,
                                      Int*            piMaxPos, 
                                      RefCtx*         pcRefCtx, 
                                      UInt&           ruiNumFrags,
                                      UInt&           ruiCoeffsDecoded, 
									  MbFGSCoefMap    &rcMbFGSCoefMap)
{
  UInt      uiStart, uiStop, uiCycle, uiFragIdx, uiNumCoeffs;
  Bool      bFirstSigRunCode, bEndOfBlock;
  UInt      uiRun;
  TCoeff    iCoeff;
  TCoeff*   piCoeff;
  TCoeff*   piCoeffBase;
  UInt      uiLastScanIdx;
  ErrVal    eStatus;
  MbSymbolReadIf*   pcFragmentReader;
  const UChar*      pucScan;

  TCoeff    aiCoeffTemp[16];
  TCoeff    aiCoeffBaseTemp[16];
  UInt      uiBlkX, uiBlkY, uiB8x8, uiB4x4IdxInB8x8;
  Par8x8    ePar8x8 = Par8x8(0);

  uiB4x4IdxInB8x8 = 0;
  if( eResidualMode == LUMA_SCAN && uiStride == 1 ) {
    // the normal 4x4 block
    piCoeff         = rcMbDataAccess    .getMbTCoeffs().get( B4x4Idx( uiBlkIdx ) );
    piCoeffBase     = rcMbDataAccessBase.getMbTCoeffs().get( B4x4Idx( uiBlkIdx ) );
    pucScan         = g_aucFrameScan;
  }
  else if ( eResidualMode == LUMA_8X8 || uiStride == 4 ) {
    uiBlkX  = uiBlkIdx % 4;
    uiBlkY  = uiBlkIdx / 4;
    uiB8x8  = (uiBlkY / 2) * 2 + uiBlkX / 2;
    ePar8x8 = Par8x8(uiB8x8);

    uiB4x4IdxInB8x8 = (uiBlkY % 2) * 2 + (uiBlkX % 2);
    piCoeffBase     = rcMbDataAccessBase.getMbTCoeffs().get8x8( B8x8Idx( ePar8x8 ) );

    if( uiStride == 4) {
      // 4x4 block within 8x8 block, deinteleave piCoeffBase to aiCoeffBaseTemp
      for( uiCycle = 0; uiCycle < 16; uiCycle ++ )
        aiCoeffBaseTemp[ uiCycle ] = piCoeffBase[ g_aucFrameScan64[uiCycle * 4 + uiB4x4IdxInB8x8] ];

      piCoeffBase   = aiCoeffBaseTemp;

      memset( aiCoeffTemp, 0, 16 * sizeof(TCoeff) );
      piCoeff       = aiCoeffTemp;
      pucScan       = g_aucLinearScan;
    }
    else {
      // cabac uses native 8x8 zigzag scan
      piCoeff       = rcMbDataAccess    .getMbTCoeffs().get8x8( B8x8Idx( ePar8x8 ) );
      pucScan       = g_aucFrameScan64;

      // a dirty fix
      // in "-pd 0" path, it is set in RQdecodeNewTCoeff_8x8, should be in higher level
      if( uiBcbp ) {
        B8x8Idx c8x8Idx( ePar8x8 );
        rcMbDataAccessBase.getMbData().setBCBP( c8x8Idx.b4x4(),   1 );
        rcMbDataAccessBase.getMbData().setBCBP( c8x8Idx.b4x4()+1, 1 );
        rcMbDataAccessBase.getMbData().setBCBP( c8x8Idx.b4x4()+4, 1 );
        rcMbDataAccessBase.getMbData().setBCBP( c8x8Idx.b4x4()+5, 1 );
      }
    }
  }
  else {
    piCoeff         = rcMbDataAccess    .getMbTCoeffs().get( CIdx( uiBlkIdx ) );
    piCoeffBase     = rcMbDataAccessBase.getMbTCoeffs().get( CIdx( uiBlkIdx ) );
    pucScan         = eResidualMode == CHROMA_DC ? g_aucIndexChromaDCScan : g_aucFrameScan;
  }

  uiStart           = ( eResidualMode == CHROMA_AC ) ? 1 : 0;
  uiStop            = ( eResidualMode == CHROMA_DC ) ? 4 : ( ( eResidualMode == LUMA_8X8 ) ? 64 : 16 );

  bFirstSigRunCode  = true;
  bEndOfBlock       = ( uiBcbp == 0 ) ? 1 : 0;
  pcFragmentReader  = m_pcSymbolReader->RQactivateFragment( 0 );

  uiNumCoeffs       = 0;
  uiRun             = 0;
  eStatus           = Err::m_nOK;

  // last possible coefficient in the block
  for( uiLastScanIdx = uiStop - 1; uiLastScanIdx >= uiStart; uiLastScanIdx -- ) {
    if( ! piCoeffBase[pucScan[uiLastScanIdx]] )
      break;
  }

  for( uiFragIdx = 0, uiCycle = uiStart; uiCycle < uiStop; uiCycle ++ ) {
    UInt uiMaxPos;
    
    uiMaxPos = ( eResidualMode == LUMA_8X8 ) ? ( piMaxPos[uiFragIdx] * 4  +3 ) : piMaxPos[uiFragIdx];
    while( uiCycle > uiMaxPos ) {
      if( uiFragIdx == ruiNumFrags - 1 ) {
        pcFragmentReader = 0;
        break;
      }
      else {
        uiFragIdx ++;
        pcFragmentReader = m_pcSymbolReader->RQactivateFragment( uiFragIdx );
        if( ! rcMbDataAccessBase.getSH().getPPS().getEntropyCodingModeFlag() )//JVT-V095
        {      
          if ( eResidualMode == LUMA_SCAN || eResidualMode == LUMA_8X8 )
            ((UvlcReader *)pcFragmentReader)->setCodType(rcMbFGSCoefMap.getMbMapH());
          if ( eResidualMode == CHROMA_AC || eResidualMode == CHROMA_DC)
            ((UvlcReader *)pcFragmentReader)->setCodType(-1);
          for (UInt i=0;i<3;i++)
            ((UvlcReader *)pcFragmentReader)->m_mapLev2[i]=((UvlcReader *)m_pcSymbolReader)->m_mapLev2[i];
          for (UInt i=0;i<7;i++)
            ((UvlcReader *)pcFragmentReader)->m_mapLev3[i]=((UvlcReader *)m_pcSymbolReader)->m_mapLev3[i];
          ((UvlcReader *)pcFragmentReader)->m_qLevel=((UvlcReader *)m_pcSymbolReader)->m_qLevel;
        }
        uiMaxPos = ( eResidualMode == LUMA_8X8 ) ? ( piMaxPos[uiFragIdx] * 4  +3 ) : piMaxPos[uiFragIdx];
      }
    }

    // no more data for this block
    if( pcFragmentReader == 0 && uiRun == 0 )
      break;

    if( piCoeffBase[pucScan[uiCycle]] ) {
      if( pcFragmentReader ) {
        if( ! rcMbDataAccessBase.getSH().getPPS().getEntropyCodingModeFlag() )    //JVT-V095
        { 
          CoefMap* pcCoefMapH;
          if ( eResidualMode != LUMA_8X8 && eResidualMode == LUMA_SCAN && uiStride ==1 )		//luma 4x4
          {
            B4x4Idx cB4x4IdxFromUInt = B4x4Idx(uiBlkIdx);
            LumaIdx cLumaIdxFromB4x4Idx=LumaIdx(cB4x4IdxFromUInt);
            B8x8Idx c8x8IdxFromLumaIdx( cLumaIdxFromB4x4Idx );
            UInt uiOffset = (cLumaIdxFromB4x4Idx.x()%2) + (cLumaIdxFromB4x4Idx.y()%2) * 2; 
            S4x4Idx c4x4IdxFromLumaIdx ( c8x8IdxFromLumaIdx );  
            c4x4IdxFromLumaIdx = c4x4IdxFromLumaIdx + uiOffset;
            pcCoefMapH=&rcMbFGSCoefMap.getCoefMapH(c4x4IdxFromLumaIdx)[uiCycle];
          }
          if ( eResidualMode == LUMA_SCAN && uiStride == 4)									//luma 8x8 FREXT
          {
            UInt ui8X8Cycle;
            ui8X8Cycle = uiCycle * 4 + uiB4x4IdxInB8x8;
            pcCoefMapH=&rcMbFGSCoefMap.getCoefMapH(B8x8Idx( ePar8x8 ))[ui8X8Cycle];
          }
          if (eResidualMode == LUMA_SCAN || eResidualMode == LUMA_8X8)
          {
            UInt Qlevel=rcMbDataAccess.getSH().getQualityLevel()+1;
            Int uiPrevCoeffInd=0;
            if (Qlevel>1)
              uiPrevCoeffInd=*pcCoefMapH;
            ((UvlcReader *)pcFragmentReader)->setPrevLevInd(uiPrevCoeffInd);
          }
        }
        eStatus = pcFragmentReader->RQdecodeTCoeffsRef( piCoeff, piCoeffBase, pucScan, uiCycle, pcRefCtx[uiCycle] );
        if( eStatus != Err::m_nOK )
          break;

        Int iXCoeff; 
        //if( eResidualMode == LUMA_SCAN && uiStride == 1 )
        {
          iXCoeff = piCoeff[pucScan[uiCycle]];
          if( piCoeffBase[pucScan[uiCycle]] < 0 )
            iXCoeff = -iXCoeff;
        }
#if 0
        else if(eRedisualMode == LUMA_8x8 || uiStride == 4) 
        {
          iCoeff = pcMbDataAccessEL->getMbTCoeffs().get8x8( c8x8Idx )[g_aucFrameScan64[uiCycle]];
          if( pcMbDataAccessBL->getMbTCoeffs().get8x8( c8x8Idx )[g_aucFrameScan64[uiCycle]] < 0 )
            iCoeff = -iCoeff;
        }
#endif
        pcRefCtx[uiCycle] <<= 2;
        if( iXCoeff < 0 )
          pcRefCtx[uiCycle]+= 2;
        else if( iXCoeff > 0 )
          pcRefCtx[uiCycle]++;

       uiNumCoeffs ++;
      }
    }
    else {
      if( bDecodeBcbpInside && pcFragmentReader ) {
        // luma BCBP is currently decoded outside
        if( eResidualMode == CHROMA_AC )
          eStatus = pcFragmentReader->RQdecodeBCBP_ChromaAC( rcMbDataAccess, rcMbDataAccessBase, CIdx( uiBlkIdx ), uiBcbp );
        else
          eStatus = pcFragmentReader->RQdecodeBCBP_ChromaDC( rcMbDataAccess, rcMbDataAccessBase, CIdx( uiBlkIdx ), uiBcbp );

        if( eStatus != Err::m_nOK )
          break;

        bDecodeBcbpInside = false;
        bEndOfBlock       = ( uiBcbp == 0 ) ? 1 : 0;
      }

      if( uiRun > 0 )
        uiRun --;
      else if( ! bEndOfBlock && pcFragmentReader ) {
        iCoeff = 0;

        eStatus = pcFragmentReader->RQdecodeSigCoeff
          ( piCoeff, piCoeffBase, eResidualMode, pucScan, bFirstSigRunCode, uiCycle, uiStart, uiLastScanIdx, bEndOfBlock, iCoeff, uiRun );

        if( eStatus != Err::m_nOK )
          break;

        if( iCoeff != 0 )
        {
          uiNumCoeffs += uiRun + 1;
        }

        bFirstSigRunCode = false;
      }

      // bEndOfBlock indicates there is no significant coefficients
      // but there may be refinement coefficients to be decoded
      if( uiRun == 0 && ! bEndOfBlock )
      {
        piCoeff[pucScan[uiCycle]] = iCoeff;
        if(iCoeff)
          pcRefCtx[uiCycle] = 1;
      }
    }
  }
  if( uiStride == 4) {
    // de-interleaved 4x4 block, put decoded coefficients in the interleaved order
    piCoeff         = rcMbDataAccess    .getMbTCoeffs().get8x8( B8x8Idx( ePar8x8 ) );
    for( uiCycle = 0; uiCycle < 16; uiCycle ++ )
      piCoeff[ g_aucFrameScan64[uiCycle * 4 + uiB4x4IdxInB8x8] ] = aiCoeffTemp[ uiCycle ];
  }

  if( eStatus == Err::m_nEndOfStream ) {
    ruiNumFrags = uiFragIdx;
  }
  else {
    if( bEndOfBlock )
      uiNumCoeffs = uiStop - uiStart;
  }

  ruiCoeffsDecoded += uiNumCoeffs;

  return Err::m_nOK;
}


ErrVal
RQFGSDecoder::xResidualBlock    ( MbDataAccess&   rcMbDataAccess,
                                  MbDataAccess&   rcMbDataAccessBase,
                                  LumaIdx         cIdx, 
                                  ResidualMode    eResidualMode,
                                  UInt            uiStride,
                                  Int*            piMaxPos, 
                                  UInt&           ruiNumFrags,
                                  MbFGSCoefMap &  rcMbFGSCoefMap, 
                                  UInt&           ruiCoeffsDecoded )
{
  UInt          uiBcbp;
  RefCtx        *pcRefCtx;  
  B8x8Idx c8x8Idx( cIdx );

  if( eResidualMode == LUMA_8X8 )
  {
    uiBcbp    = ( rcMbDataAccess  .getMbData().getMbCbp() >> ( (cIdx.y()/2)*2 + cIdx.x()/2 ) ) & 1;
    pcRefCtx = rcMbFGSCoefMap.getRefCtx( c8x8Idx );
  }
  else
  { 
    UInt uiOffset = (cIdx.x()%2) + (cIdx.y()%2) * 2; 
    S4x4Idx c4x4Idx ( c8x8Idx );  
    c4x4Idx = c4x4Idx + uiOffset; 
    uiBcbp    = rcMbDataAccess    .getMbData().getBCBP( cIdx.b4x4() );
    pcRefCtx = rcMbFGSCoefMap.getRefCtx( c4x4Idx );
  }

  if( ! rcMbDataAccessBase.getSH().getPPS().getEntropyCodingModeFlag() )
  {
    ((UvlcReader *)m_pcSymbolReader)->setCodType(rcMbFGSCoefMap.getMbMapH());
  }
  RNOKS( xResidualBlock( rcMbDataAccess, rcMbDataAccessBase, eResidualMode, uiStride, cIdx,
    uiBcbp, false, piMaxPos, pcRefCtx, ruiNumFrags, ruiCoeffsDecoded, rcMbFGSCoefMap /*U132*/) );

  return Err::m_nOK;
}


ErrVal
RQFGSDecoder::xResidualBlock    ( MbDataAccess&   rcMbDataAccess,
                                  MbDataAccess&   rcMbDataAccessBase,
                                  ChromaIdx       cIdx, 
                                  ResidualMode    eResidualMode,
                                  Int*            piMaxPos, 
                                  UInt&           ruiNumFrags,
                                  MbFGSCoefMap &  rcMbFGSCoefMap, 
                                  UInt&           ruiCoeffsDecoded )
{
  UInt          uiBcbp, uiBaseBcbp;
  Bool          bDecodeBcbpInside;
  ErrVal        eStatus;
  RefCtx*       pcRefCtx; 
  RefCtx        cRefCtxTemp[4]; 

  bDecodeBcbpInside = false;
  uiBcbp = 0;
  if( eResidualMode == CHROMA_DC ) {
    if( ( rcMbDataAccess.getMbData().getMbCbp() >> 4 ) > 0 )
      bDecodeBcbpInside = true;

    uiBaseBcbp  = rcMbDataAccessBase.getMbData().getBCBP( 24 + cIdx.plane() );

    CPlaneIdx cCPlaneIdx ( cIdx.plane() );
    for(UInt uiDCIdx = 0; uiDCIdx < 4; uiDCIdx ++ )
    {
      CIdx cCIdx = CIdx( cCPlaneIdx ) + uiDCIdx;

      cRefCtxTemp[uiDCIdx] = rcMbFGSCoefMap.getRefCtx( cCIdx )[0] ; 
    }
    pcRefCtx = cRefCtxTemp;
  }
  else {
    if( ( rcMbDataAccess.getMbData().getMbCbp() >> 4 ) > 1 )
      bDecodeBcbpInside = true;

    uiBaseBcbp  = rcMbDataAccessBase.getMbData().getBCBP( 16 + cIdx );
    pcRefCtx = rcMbFGSCoefMap.getRefCtx ( cIdx ); 
  }

//  ROTRS( uiBaseBcbp == 0 && ! bDecodeBcbpInside, Err::m_nOK );
  if( ! rcMbDataAccessBase.getSH().getPPS().getEntropyCodingModeFlag() )
  {
    ((UvlcReader *)m_pcSymbolReader)->setCodType(-1);
  }
  eStatus = xResidualBlock( rcMbDataAccess, rcMbDataAccessBase, eResidualMode, 1, cIdx,
    uiBcbp, bDecodeBcbpInside, piMaxPos, pcRefCtx, ruiNumFrags, ruiCoeffsDecoded, rcMbFGSCoefMap /*U132*/ );

  if( eResidualMode == CHROMA_DC ) {
    rcMbDataAccess    .getMbData().setBCBP( 24 + cIdx.plane(), uiBcbp );
    rcMbDataAccessBase.getMbData().setBCBP( 24 + cIdx.plane(), uiBcbp);

    CPlaneIdx cCPlaneIdx ( cIdx.plane() );
    for(UInt uiDCIdx = 0; uiDCIdx < 4; uiDCIdx ++ )
    {
      CIdx cCIdx = CIdx( cCPlaneIdx ) + uiDCIdx;

      rcMbFGSCoefMap.getRefCtx( cCIdx )[0] = cRefCtxTemp[uiDCIdx]; 
    }
  }
  else {
    rcMbDataAccess    .getMbData().setBCBP( 16 + cIdx, uiBcbp );
    rcMbDataAccessBase.getMbData().setBCBP( 16 + cIdx, uiBcbp );
  }

  return eStatus;
}

ErrVal
RQFGSDecoder::xDecodingFGSBlock( SliceHeader*                pcSliceHeader  )
{
  RNOK( m_cMbDataCtrlEL    .reset     () );
  RNOK( m_cMbDataCtrlEL    .clear     () );
  RNOK( m_cMbDataCtrlEL    .initSlice ( *m_pcSliceHeader, PARSE_PROCESS, true, NULL ) );
  RNOK( m_pcCurrMbDataCtrl->initSlice ( *m_pcSliceHeader, PRE_PROCESS, true, NULL ) );
  
  RNOK( xInitializeMacroblockQPs() );

  RNOK( m_cMbDataCtrlEL    .initSlice ( *m_pcSliceHeader, PRE_PROCESS, true, NULL ) );

  Int iLastQP = m_pcSliceHeader->getPicQp();

  UInt uiFirstMbInSlice = m_pcSliceHeader->getFirstMbInSlice ();
  UInt uiNumMbsInSlice  = m_pcSliceHeader->getNumMbsInSlice  ();
  m_bFgsComponentSep    = m_pcSliceHeader->getFgsComponentSep();

  m_eFrameType=getSliceHeader()->getSliceType();

  UInt ui;

  if(m_pcSliceHeader->getFGSCodingMode() == false) {
    //grouping size mode
    UInt uiGroupingSize = m_pcSliceHeader->getGroupingSize();
    ui = 0;
    m_auiScanPosVectLuma[ui] = uiGroupingSize-1;
    while( m_auiScanPosVectLuma[ui] < 15) {
      ui++;
      m_auiScanPosVectLuma[ui] = m_auiScanPosVectLuma[ui-1]+uiGroupingSize;
    }
  }
  else {
    //vector specified
    ui = 0;
    m_auiScanPosVectLuma[ui] = m_pcSliceHeader->getPosVect(ui) - 1;
    while( m_auiScanPosVectLuma[ui] < 15) {
      ui++;
      m_auiScanPosVectLuma[ui] = m_auiScanPosVectLuma[ui-1] + m_pcSliceHeader->getPosVect(ui);
    }
  }

  AOT( m_pcSymbolReader == 0 );
  RNOK( m_pcSymbolReader  ->startSlice( *m_pcSliceHeader ) );
  
  UInt iStartCycle = 0;
  Bool bChromaCbpFlag;
  UInt uiMbX, uiMbY;

  UInt uiFirstMbY = (UInt) ( uiFirstMbInSlice / m_uiWidthInMB );
  UInt uiFirstMbX = uiFirstMbInSlice % m_uiWidthInMB;
  UInt uiLastMbY  = (UInt) ( ( uiFirstMbInSlice + uiNumMbsInSlice ) / m_uiWidthInMB );
  UInt uiLastMbX  = ( uiFirstMbInSlice + uiNumMbsInSlice ) % m_uiWidthInMB;

  UInt uiNumFrags;
  Int  aiMaxPosLuma[16], aiMaxPosChromaAC[16], aiMaxPosChromaDC[16];

  try
  {
    AOT( m_pcSymbolReader == 0 );

    RNOK( m_pcSymbolReader->RQdecodeEobOffsets_Luma  () );
    RNOK( m_pcSymbolReader->RQdecodeEobOffsets_Chroma() );
    RNOK( m_pcSymbolReader->RQdecodeBestCodeTableMap  ( 16 ) );

  if( ! m_pcSliceHeader->getPPS().getEntropyCodingModeFlag() )  //JVT-V095
  {
	xSignaling_Table(pcSliceHeader);	//receive the VLC table to be used for refinement coeff  -V095
	xSignaling(pcSliceHeader);	        //receive codetype and history map -V095
  }
    m_uiLumaCbpRun    = 0;
    m_uiChromaCbpRun  = 0;

    if( ! m_pcSliceHeader->getPPS().getEntropyCodingModeFlag() )
    {
      ((UvlcReader *) m_pcSymbolReader)->getFlag(m_bLastLumaCbpFlag, "Luma_CBP_first_flag");
      ((UvlcReader *) m_pcSymbolReader)->getFlag(bChromaCbpFlag, "Chroma_CBP_first");
      m_uiLastChromaCbp = bChromaCbpFlag ? 1 : 0;
    }

    // initialize the parallel bitstream buffers
    m_pcSymbolReader->RQinitFragments( *m_pcSliceHeader, uiNumFrags, true );

    m_pcSymbolReader->RQsetTruncatedFlag( false );

    // when m_bFgsComponentSep is needed
    if( m_bFgsComponentSep == 0 )
      m_pcSymbolReader->RQdecodeCycleSymbol( iStartCycle );

    xDeriveComponentPosVectors( m_auiScanPosVectLuma, 
      aiMaxPosLuma, aiMaxPosChromaAC, aiMaxPosChromaDC, iStartCycle );

    for( uiMbY = uiFirstMbY; uiMbY < uiLastMbY && uiNumFrags > 0; uiMbY ++ ) {
      for( uiMbX = ( uiMbY == uiFirstMbY ? uiFirstMbX : 0 ); 
        uiMbX < ( uiMbY == uiLastMbY ? uiLastMbX : m_uiWidthInMB ) && uiNumFrags > 0;  uiMbX ++ ) {

        MbDataAccess* pcMbDataAccessEL  = 0;
        MbDataAccess* pcMbDataAccessBL  = 0;
        UInt          uiMbCoeffsDecoded = 0;
        UInt          uiMbAddress = uiMbY * m_uiWidthInMB + uiMbX ;
        MbFGSCoefMap  &rcMbFGSCoefMap = m_pcCoefMap[uiMbAddress];

        RNOK( m_pcCurrMbDataCtrl ->initMb( pcMbDataAccessBL, uiMbY, uiMbX ) );
        RNOK( m_cMbDataCtrlEL     .initMb( pcMbDataAccessEL, uiMbY, uiMbX ) );

        // Read the MB header
       if( xDecodeMbHeader( pcMbDataAccessBL, pcMbDataAccessEL, rcMbFGSCoefMap, iLastQP, pcSliceHeader /*U132*/ ) != Err::m_nOK )
        {
          uiNumFrags = 0;

          UInt uiFwdBwdBase        = pcMbDataAccessBL->getMbData().getFwdBwd();
          // ===== ERROR HANDLING (typically truncated FGS slice) =====
          // ----- restore base layer forward/backward indication -----
          pcMbDataAccessEL->getMbData().setFwdBwd( uiFwdBwdBase );

          // ----- mark as skipped macroblock -----
          RNOK( pcMbDataAccessEL->getMbData().copyMotion( pcMbDataAccessBL->getMbData() ) );
                pcMbDataAccessEL->getMbData().copyFrom  ( pcMbDataAccessBL->getMbData() );
                pcMbDataAccessEL->getMbData().setBLSkipFlag( true );
        }

        if( uiNumFrags > 0 ) {
          //===== LUMA =====
          for( B8x8Idx c8x8Idx; c8x8Idx.isLegal() && uiNumFrags > 0; c8x8Idx ++ ) {
            if( pcMbDataAccessBL->getMbData().isTransformSize8x8() &&
              pcMbDataAccessBL->getSH().getPPS().getEntropyCodingModeFlag() ) {
              xResidualBlock( *pcMbDataAccessEL, *pcMbDataAccessBL, 
                c8x8Idx, LUMA_8X8, 1, aiMaxPosLuma, uiNumFrags, rcMbFGSCoefMap, uiMbCoeffsDecoded );
            }
            else {
              // determine if this is actual 4x4, or deinterleaved 4x4
              UInt uiStride = pcMbDataAccessBL->getMbData().isTransformSize8x8() ? 4 : 1;
              for( S4x4Idx cIdx(c8x8Idx); cIdx.isLegal( c8x8Idx ) && uiNumFrags > 0; cIdx ++ ) {
                xResidualBlock( *pcMbDataAccessEL, *pcMbDataAccessBL, 
                  cIdx, LUMA_SCAN, uiStride, aiMaxPosLuma, uiNumFrags, rcMbFGSCoefMap, uiMbCoeffsDecoded );
              }
            }
          }

          //===== CHROMA DC =====
          if( uiNumFrags > 0 ) {
            xResidualBlock( *pcMbDataAccessEL, *pcMbDataAccessBL, 
              CIdx(0), CHROMA_DC, aiMaxPosChromaDC, uiNumFrags, rcMbFGSCoefMap, uiMbCoeffsDecoded );
          }
          if( uiNumFrags > 0 ) {
            xResidualBlock( *pcMbDataAccessEL, *pcMbDataAccessBL, 
              CIdx(4), CHROMA_DC, aiMaxPosChromaDC, uiNumFrags, rcMbFGSCoefMap, uiMbCoeffsDecoded );
          }

          //===== CHROMA AC =====
          for( CIdx cCIdx; cCIdx.isLegal() && uiNumFrags > 0; cCIdx ++ ) {
            xResidualBlock( *pcMbDataAccessEL, *pcMbDataAccessBL, 
              cCIdx, CHROMA_AC, aiMaxPosChromaAC, uiNumFrags, rcMbFGSCoefMap, uiMbCoeffsDecoded );
          }

          RNOK( m_pcSymbolReader->RQupdateVlcTable( uiNumFrags ) );
          RNOK( xSetNumCoefficients( uiMbX, uiMbY, rcMbFGSCoefMap, uiMbCoeffsDecoded ) );
        }
      }
    }

    RNOK( m_pcSymbolReader->RQvlcFlush() );

    UInt  uiTermBit = 0;
    RNOK( m_pcSymbolReader->RQdecodeTermBit( uiTermBit ) );
  }

  catch( BitReadBuffer::ReadStop )
  {
    m_bPicFinished = true;

    m_pcSymbolReader->RQsetTruncatedFlag( true );
  }

  RNOK( m_pcSymbolReader->finishSlice( ) );

  // initialize the parallel bitstream buffers
  m_pcSymbolReader->RQreleaseFragments();

  if( ! m_bPicFinished )
  {
    m_pcSliceHeader->setQualityLevel( m_pcSliceHeader->getQualityLevel() + 1 );
  }

  m_bUpdateWithoutMap = true;

  RNOK( xUpdateCodingPath( pcSliceHeader ) );
  RNOK( xClearCodingPath() );
 
  return Err::m_nOK;
}


// all functions below, except xDecodeMotionData and  
// xInitializeMacroblockQPs are not needed for block-based decoding
ErrVal
RQFGSDecoder::xDecodingFGS( SliceHeader*                pcSliceHeader 	)
{
  RNOK( m_cMbDataCtrlEL    .reset     () );
  RNOK( m_cMbDataCtrlEL    .clear     () );
  RNOK( m_cMbDataCtrlEL    .initSlice ( *m_pcSliceHeader, PARSE_PROCESS, true, NULL ) );
  RNOK( m_pcCurrMbDataCtrl->initSlice ( *m_pcSliceHeader, PRE_PROCESS, true, NULL ) );
  
  RNOK( xInitializeMacroblockQPs() );

  RNOK( m_cMbDataCtrlEL    .initSlice ( *m_pcSliceHeader, PRE_PROCESS, true, NULL ) );

  Int iLastQP = m_pcSliceHeader->getPicQp();

  m_eFrameType=getSliceHeader()->getSliceType();

  UInt uiFirstMbInSlice = m_pcSliceHeader->getFirstMbInSlice ();
  // JVT-S054 (2) (ADD)
  UInt uiLastMbInSlice  = m_pcSliceHeader->getLastMbInSlice();
  m_bFgsComponentSep    = m_pcSliceHeader->getFgsComponentSep();

  Bool isTruncated =false;

  //positions vector for luma (and chromaAC) and chroma DC
  UInt ui;
  for(ui = 0; ui < 4; ui++)
  {
    m_auiScanPosVectChromaDC[ui] = ui;
  }
  if(m_pcSliceHeader->getFGSCodingMode() == false)
  {
    //grouping size mode
    UInt uiGroupingSize = m_pcSliceHeader->getGroupingSize();
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
    m_auiScanPosVectLuma[ui] = m_pcSliceHeader->getPosVect(ui) - 1;
    while( m_auiScanPosVectLuma[ui] < 15)
    {
      ui++;
      m_auiScanPosVectLuma[ui] = m_auiScanPosVectLuma[ui-1] + m_pcSliceHeader->getPosVect(ui);
    }
  }
  try
  {
    AOT( m_pcSymbolReader == 0 );
    RNOK( m_pcSymbolReader  ->startSlice( *m_pcSliceHeader ) );
    
    //===== SIGNIFICANCE PATH =====
    {
      UInt iStartCycle = 0, iCycle = 0;
      UInt iLumaScanIdx     = 0;
      UInt iChromaDCScanIdx = 0;
      UInt iChromaACScanIdx = 1;

      RNOK( m_pcSymbolReader->RQdecodeEobOffsets_Luma  () );
      RNOK( m_pcSymbolReader->RQdecodeEobOffsets_Chroma() );
      RNOK( m_pcSymbolReader->RQdecodeBestCodeTableMap  ( 16 ) );

      if( ! m_pcSliceHeader->getPPS().getEntropyCodingModeFlag() )  //JVT-V095
      {
	    xSignaling_Table(pcSliceHeader);  //receive the VLC table info to be used to decode Refinement coeff -V095
	    xSignaling(pcSliceHeader); //V095
      }

      m_uiLumaCbpRun          = 0;
      m_uiChromaCbpRun        = 0;
      Bool bChromaCbpFlag;

      if( ! m_pcSliceHeader->getPPS().getEntropyCodingModeFlag() )
      {
        m_pcUvlcReader->getFlag(m_bLastLumaCbpFlag, "Luma_CBP_first_flag");
        m_pcUvlcReader->getFlag(bChromaCbpFlag, "Chroma_CBP_first");
        m_uiLastChromaCbp = bChromaCbpFlag ? 1 : 0;
      }

      UInt  uiNumFrags, uiFragIdx;
      Int   aiMaxPosLuma[16], aiMaxPosChromaAC[16], aiMaxPosChromaDC[16];
      
      // initialize the parallel bitstream buffers
      m_pcSymbolReader->RQinitFragments( *m_pcSliceHeader, uiNumFrags, false );
      if( m_bFgsComponentSep == 0 )
        m_pcSymbolReader->RQdecodeCycleSymbol(iStartCycle);

      uiFragIdx = 0;
      uiNumFrags = xDeriveComponentPosVectors( m_auiScanPosVectLuma, 
        aiMaxPosLuma, aiMaxPosChromaAC, aiMaxPosChromaDC, iStartCycle );
      while (iLumaScanIdx < 16 || iChromaDCScanIdx < 4 || iChromaACScanIdx < 16) {
        UInt bAllowChromaDC, bAllowChromaAC;

        UInt uiMaxPosLuma;
        UInt uiMaxPosChromaAC;
        UInt uiMaxPosChromaDC;

        uiMaxPosLuma      = aiMaxPosLuma    [uiFragIdx];
        uiMaxPosChromaAC  = aiMaxPosChromaAC[uiFragIdx];
        uiMaxPosChromaDC  = aiMaxPosChromaDC[uiFragIdx];

        if( uiFragIdx == 0 )
        {
          bAllowChromaDC = true;
          bAllowChromaAC = aiMaxPosChromaAC[0] > 0;
        }
        else
        {
          bAllowChromaDC = (Int) uiMaxPosChromaDC > aiMaxPosChromaDC[uiFragIdx - 1];
          bAllowChromaAC = (Int) uiMaxPosChromaAC > aiMaxPosChromaAC[uiFragIdx - 1];
        }

        uiFragIdx ++;

        if( iLumaScanIdx >= 16 && !bAllowChromaDC && !bAllowChromaAC )
        {
          iCycle++;
          continue;
        }

        UInt uiMbAddress = 0;
        for(uiMbAddress=uiFirstMbInSlice ;uiMbAddress<=uiLastMbInSlice ;)
        {
          const UInt uiMbYIdx = uiMbAddress / m_uiWidthInMB;
          const UInt uiMbXIdx = uiMbAddress % m_uiWidthInMB;

          MbDataAccess* pcMbDataAccessEL = NULL, *pcMbDataAccessBL = NULL;
          RNOK( m_cMbDataCtrlEL    .initMb( pcMbDataAccessEL, uiMbYIdx, uiMbXIdx ) );
          RNOK( m_pcCurrMbDataCtrl->initMb( pcMbDataAccessBL, uiMbYIdx, uiMbXIdx ) );
          const Bool bFrame =  (FRAME == pcMbDataAccessBL->getMbPicType());
          MbFGSCoefMap &rcMbFGSCoefMap = m_pcCoefMap[uiMbAddress];

          if( iLumaScanIdx == 0 && rcMbFGSCoefMap.getNumCoded() == 0 ) {
            if( xDecodeMbHeader( pcMbDataAccessBL, pcMbDataAccessEL, rcMbFGSCoefMap, iLastQP, pcSliceHeader /*U132*/ ) != Err::m_nOK )
              throw BitReadBuffer::ReadStop();
          }
          if( ! m_pcSliceHeader->getPPS().getEntropyCodingModeFlag() )  //JVT-V095
          {
            ((UvlcReader *)m_pcSymbolReader)->setCodType(rcMbFGSCoefMap.getMbMapH());
          }
          if( xDecodeNewCoefficientLumaMb( pcMbDataAccessBL, pcMbDataAccessEL, rcMbFGSCoefMap, uiMbYIdx, uiMbXIdx, iLastQP, iLumaScanIdx, uiMaxPosLuma, bFrame ) != Err::m_nOK )
            throw BitReadBuffer::ReadStop();
          //===== CHROMA DC =====
          if( bAllowChromaDC && ( m_bFgsComponentSep == 0 || iLumaScanIdx == 16 ) )
          {
            for( CPlaneIdx cCPlaneIdx; cCPlaneIdx.isLegal(); ++cCPlaneIdx )
            {
              for( ui=iChromaDCScanIdx; ui<=uiMaxPosChromaDC && ui<4; ui++ )
              {
                if( ui == 0 || ui == rcMbFGSCoefMap.getChromaDCScanPos( cCPlaneIdx ) )
                {
                  if( xDecodeNewCoefficientChromaDC( pcMbDataAccessBL, pcMbDataAccessEL, rcMbFGSCoefMap, cCPlaneIdx, iLastQP, ui ) != Err::m_nOK )
                    throw BitReadBuffer::ReadStop();
                }
                CoefMap cCoefMap = m_pcCoefMap[uiMbAddress].getCoefMap( CIdx( cCPlaneIdx ) + ui )[0];
                if( (cCoefMap & SIGNIFICANT) && !(cCoefMap & CODED) )
				{
                  if( ! m_pcSliceHeader->getPPS().getEntropyCodingModeFlag() )  //JVT-V095
                  {
                    ((UvlcReader *)m_pcSymbolReader)->setCodType(-1);
                  }
                if( xDecodeCoefficientChromaDCRef( pcMbDataAccessBL, pcMbDataAccessEL, rcMbFGSCoefMap, cCPlaneIdx, ui ) != Err::m_nOK )
                  throw BitReadBuffer::ReadStop();
				}
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
                    {
                      if( xDecodeNewCoefficientChromaAC( pcMbDataAccessBL, pcMbDataAccessEL, rcMbFGSCoefMap, cCIdx, iLastQP, ui, bFrame ) != Err::m_nOK )
                        throw BitReadBuffer::ReadStop();
                    }
                    if( (pcCoefMap[ui] & SIGNIFICANT) && !(pcCoefMap[ui] & CODED) )
                    {
                      if( ! m_pcSliceHeader->getPPS().getEntropyCodingModeFlag() )  //JVT-V095
                      {
                       ((UvlcReader *)m_pcSymbolReader)->setCodType(-1);
                      }
                      if( xDecodeCoefficientChromaACRef( pcMbDataAccessBL, pcMbDataAccessEL, rcMbFGSCoefMap, cCIdx, ui ) != Err::m_nOK )
                        throw BitReadBuffer::ReadStop();
                    }
              }
              } // for
              }
            }
            } // if
            RNOK( m_pcSymbolReader->RQupdateVlcTable() );

			//--ICU/ETRI FMO Implementation
          uiMbAddress = m_pcSliceHeader->getFMO()->getNextMBNr( uiMbAddress );

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
    m_pcSymbolReader->RQsetTruncatedFlag( true );
  }

  RNOK( m_pcSymbolReader->finishSlice( ) );

  // initialize the parallel bitstream buffers
  m_pcSymbolReader->RQreleaseFragments(); //TMM_INTERLACE 

  if( ! m_bPicFinished )
  {	  
    m_pcSliceHeader->setQualityLevel( pcSliceHeader->getQualityLevel());
  }

  //--ICU/ETRI 1206  
  RNOK( xUpdateCodingPath(pcSliceHeader) );  
  RNOK( xClearCodingPath() );
  
  return Err::m_nOK;
}


ErrVal
RQFGSDecoder::xDecodeMotionData( UInt uiMbYIdx, UInt uiMbXIdx )
{
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
    UInt            uiLayer         = m_pcSliceHeader->getLayerId();
    YuvBufferCtrl*  pcYuvBufferCtrl = m_papcYuvFullPelBufferCtrl[uiLayer];

    const Bool    bMbAff   = m_pcSliceHeader->isMbAff   (); // TMM_INTERLACE
    RNOK( pcYuvBufferCtrl->initMb( uiMbYIdx, uiMbXIdx, bMbAff ) );
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
    if( ! m_pcSliceHeader->getAdaptivePredictionFlag() && ! pcMbDataAccess->getMbData().isIntra() )
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
                                           UInt  uiMaxPosLuma,
                                           Bool  bFrame )
{
  ROFRS( iLumaScanIdx < 16, Err::m_nOK );

  for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
  {
    S4x4Idx cIdx2 = S4x4Idx( c8x8Idx ); 
    Bool b8x8 = (rcMbFGSCoefMap.getLumaScanPos( cIdx2+1 ) == 64); 

    if( b8x8 )
    {
      for( UInt ui=iLumaScanIdx; ui<=uiMaxPosLuma && ui<16; ui++ )
      {
        for( S4x4Idx cIdx( c8x8Idx ); cIdx.isLegal( c8x8Idx ); cIdx++ )
        {
          UInt uiOffset = ( cIdx.y() % 2 ) * 2 + ( cIdx.x() % 2);
          if( rcMbFGSCoefMap.getLumaScanPos( cIdx2 ) == ui * 4 + uiOffset )
          {
            RNOKS( xDecodeNewCoefficientLuma( pcMbDataAccessBL, pcMbDataAccessEL, rcMbFGSCoefMap, cIdx, bFrame ) );
          }
          RNOKS( xDecodeCoefficientLumaRef( pcMbDataAccessBL, pcMbDataAccessEL, rcMbFGSCoefMap, cIdx, ui ) );
        }
      } // 4x4 block iteration
    }
    else
    {
      for( S4x4Idx cIdx( c8x8Idx ); cIdx.isLegal( c8x8Idx ); cIdx++ )
      {
          for( UInt ui=iLumaScanIdx; ui<=uiMaxPosLuma && ui<16; ui++ )
          {
          if( rcMbFGSCoefMap.getLumaScanPos( cIdx ) == ui )
          {
              RNOK( xDecodeNewCoefficientLuma( pcMbDataAccessBL, pcMbDataAccessEL, rcMbFGSCoefMap, cIdx, bFrame ) );
            }
            RNOKS( xDecodeCoefficientLumaRef( pcMbDataAccessBL, pcMbDataAccessEL, rcMbFGSCoefMap, cIdx, ui ) );
          }
        } // 4x4 block iteration
    }
  } // 8x8 block iteration

  return Err::m_nOK;
}

ErrVal
RQFGSDecoder::xDecodeNewCoefficientLuma( MbDataAccess* pcMbDataAccessBL,
                                         MbDataAccess* pcMbDataAccessEL,
                                         MbFGSCoefMap  &rcMbFGSCoefMap,
                                         const S4x4Idx &rcIdx,
                                         Bool          bFrame )
{
  B8x8Idx c8x8Idx( rcIdx.getContainingPar8x8() );
  Bool    b8x8 = pcMbDataAccessBL->getMbData().isTransformSize8x8();
  if( b8x8 && m_pcSliceHeader->getPPS().getEntropyCodingModeFlag() )
  {
    UInt uiStop           = 64;
    UInt ui8x8ScanIndex   = uiStop;
    UInt ui8x8StartIndex  = uiStop;
    UInt ui8x8Index; 

    CoefMap* pcCoefMap = rcMbFGSCoefMap.getCoefMap( c8x8Idx );
    for( ui8x8Index = 0; ui8x8Index < uiStop; ui8x8Index++ )
    {
      if( ! (pcCoefMap[ ui8x8Index ] & SIGNIFICANT ) || (pcCoefMap [ ui8x8Index ] & NEWSIG ) )
      {
        ui8x8StartIndex = ui8x8Index;
        break; 
      }
    }
    S4x4Idx cIdx2 = S4x4Idx ( c8x8Idx );
    ui8x8ScanIndex = rcMbFGSCoefMap.getLumaScanPos( cIdx2 ) ;
    ROTRS ( ui8x8ScanIndex == uiStop, Err::m_nOK ); 
    
    Bool bNeedEob = (ui8x8ScanIndex > ui8x8StartIndex );
    while ( ui8x8ScanIndex < 64 )
    {
      UInt uiNumCoefRead;

      RNOKS( m_pcSymbolReader->RQdecodeNewTCoeff_8x8( *pcMbDataAccessEL, *pcMbDataAccessBL,
                                                      c8x8Idx, ui8x8ScanIndex, bNeedEob, uiNumCoefRead ) );
      if( bNeedEob )
      {
        //===== end of block =====
        rcMbFGSCoefMap.getLumaScanPos( cIdx2 ) = 64;

        for( UInt ui8x8 = ui8x8ScanIndex; ui8x8 < 64; ui8x8++ )
        {
          if( ! (pcCoefMap [ ui8x8 ] & SIGNIFICANT) )
          {
            pcCoefMap[ ui8x8 ]  |= CODED;
            RNOK( rcMbFGSCoefMap.increaseAndCheckNumCoded( 1 ) );
          }
        }
        break;
      }
      else
      {
        //===== coefficient =====
        bNeedEob = false;

        pcCoefMap[ ui8x8ScanIndex ] |= CODED;
        RNOK( rcMbFGSCoefMap.increaseAndCheckNumCoded( 1 ) );
        if( pcMbDataAccessEL->getMbTCoeffs().get8x8( c8x8Idx )[g_aucFrameScan64[ui8x8ScanIndex]] )
        {
          rcMbFGSCoefMap.getRefCtx( c8x8Idx )[ui8x8ScanIndex] = 1;
          pcCoefMap[ ui8x8ScanIndex ] |= SIGNIFICANT | NEWSIG;
          break;
        }

        ui8x8ScanIndex++;
        while( ( ui8x8ScanIndex < 64 ) && (pcCoefMap[ ui8x8ScanIndex ] & SIGNIFICANT) )
        {
          ui8x8ScanIndex++;
        }
      }
    }
    if( rcMbFGSCoefMap.getLumaScanPos( cIdx2 ) < ui8x8ScanIndex )
      rcMbFGSCoefMap.getLumaScanPos( cIdx2 ) = ui8x8ScanIndex;
    while( ui8x8ScanIndex < 64 && ( pcCoefMap[ ui8x8ScanIndex ] & SIGNIFICANT ) )
      rcMbFGSCoefMap.getLumaScanPos( cIdx2 ) = ++ui8x8ScanIndex;
  }
  else
  {
    if (! m_pcSymbolReader->RQpeekCbp4x4( *pcMbDataAccessBL, rcIdx ) )
    {
      return Err::m_nOK;
    }

    UInt    uiStop       = b8x8 ? 64:16;
    UInt    uiScanIndex  = uiStop;
    UInt    uiStartIndex = uiStop;
    UInt    uiIndex;
    CoefMap* pcCoefMap = b8x8 ? rcMbFGSCoefMap.getCoefMap(c8x8Idx) : rcMbFGSCoefMap.getCoefMap( rcIdx );

    if( b8x8 )
    {
      UInt ui8x8Index;
      UInt uiOffset = (rcIdx.x() % 2) + (rcIdx.y() % 2) *2; 
      for( ui8x8Index = uiOffset; ui8x8Index < uiStop; ui8x8Index+=4 )
      {
        if( ! ( pcCoefMap[ui8x8Index] & SIGNIFICANT ) || ( pcCoefMap[ui8x8Index] & NEWSIG ) )
        {
          uiStartIndex = ui8x8Index;
          break;
        }
      }
      uiStartIndex >>= 2;
    }
    else
    {
      for( uiIndex = 0; uiIndex < uiStop; uiIndex++ )
      {
        if( ! ( pcCoefMap[uiIndex] & SIGNIFICANT ) || ( pcCoefMap[uiIndex] & NEWSIG ) )
        {
          uiStartIndex = uiIndex;
          break;
        }
      }
    }
    uiScanIndex = rcMbFGSCoefMap.getLumaScanPos( rcIdx );
    ROTRS(uiScanIndex == uiStop, Err::m_nOK);

    Bool bNeedEob = ( uiScanIndex > uiStartIndex );
    UInt uiNumCoefRead;

    RNOKS( m_pcSymbolReader->RQdecodeNewTCoeff_Luma( *pcMbDataAccessEL, *pcMbDataAccessBL,
      LUMA_SCAN, b8x8, rcIdx, uiScanIndex, bNeedEob, uiNumCoefRead ) );
    if(b8x8)
    {
      UInt    uiOffset = (rcIdx.y() % 2) * 2 + (rcIdx.x() % 2); 
      UInt    ui8x8ScanIndex = uiOffset + uiScanIndex * 4; 

      for ( UInt ui8x8 = 0; ui8x8ScanIndex < 64 && ( ui8x8 < uiNumCoefRead || bNeedEob ); ui8x8ScanIndex += 4 )
      {
        if( ! ( pcCoefMap[ui8x8ScanIndex] & SIGNIFICANT ) )
        {
          if( pcMbDataAccessEL->getMbTCoeffs().get8x8( c8x8Idx )[ g_aucFrameScan64[ui8x8ScanIndex] ] )
          {
            rcMbFGSCoefMap.getRefCtx( c8x8Idx )[ui8x8ScanIndex] = 1;
            pcCoefMap[ui8x8ScanIndex] |= SIGNIFICANT | NEWSIG;
          }
          pcCoefMap[ui8x8ScanIndex] |= CODED;
          RNOKS( rcMbFGSCoefMap.increaseAndCheckNumCoded( 1 ) );
          ui8x8++;
        }
      }
      rcMbFGSCoefMap.getLumaScanPos( rcIdx ) = ui8x8ScanIndex / 4;
      while( ui8x8ScanIndex < 64 && ( pcCoefMap[ui8x8ScanIndex] & SIGNIFICANT ) )
      {
        ui8x8ScanIndex += 4;
        rcMbFGSCoefMap.getLumaScanPos( rcIdx ) = ui8x8ScanIndex / 4;
      }
    }
    else
    {
      const UChar*  pucScan = (bFrame) ? g_aucFrameScan : g_aucFieldScan;
      for ( UInt ui = 0; uiScanIndex < 16 && ( ui < uiNumCoefRead || bNeedEob ); uiScanIndex++ )
      {
        if( ! ( pcCoefMap[uiScanIndex] & SIGNIFICANT ) )
        {
          if( pcMbDataAccessEL->getMbTCoeffs().get( rcIdx )[pucScan[uiScanIndex]] )
          {
            rcMbFGSCoefMap.getRefCtx( rcIdx )[uiScanIndex] = 1;
            pcCoefMap[uiScanIndex] |= SIGNIFICANT | NEWSIG;
          }
          pcCoefMap[uiScanIndex] |= CODED;
          RNOKS( rcMbFGSCoefMap.increaseAndCheckNumCoded( 1 ) );
          ui++;
        }
      }
      rcMbFGSCoefMap.getLumaScanPos( rcIdx ) = uiScanIndex;
      while( uiScanIndex < 16 && ( pcCoefMap[uiScanIndex] & SIGNIFICANT ) )
        rcMbFGSCoefMap.getLumaScanPos( rcIdx ) = ++uiScanIndex;
    }
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

  if( !(rcMbFGSCoefMap.getChromaDCMbMap( rcCPlaneIdx ) & CODED ) )
  {
    UInt  uiSymbol;
    RNOKS( m_pcSymbolReader->RQdecodeBCBP_ChromaDC( *pcMbDataAccessEL, *pcMbDataAccessBL, CIdx(rcCPlaneIdx), uiSymbol ) );
    Bool  bSigBCBP = uiSymbol != 0;
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
  RNOKS( m_pcSymbolReader->RQdecodeNewTCoeff_Chroma( *pcMbDataAccessEL, *pcMbDataAccessBL,
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
                                              UInt    uiChromaScanIndex,
                                              Bool    bFrame )
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

  if( !(rcMbFGSCoefMap.getChromaACBlockMap( rcCIdx ) & CODED ) )
  {
    UInt  uiSymbol;
    RNOKS( m_pcSymbolReader->RQdecodeBCBP_ChromaAC( *pcMbDataAccessEL, *pcMbDataAccessBL, rcCIdx, uiSymbol ) );
    Bool  bSigBCBP = uiSymbol != 0;
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
  RNOKS( m_pcSymbolReader->RQdecodeNewTCoeff_Chroma( *pcMbDataAccessEL, *pcMbDataAccessBL,
                                                     CHROMA_AC, rcCIdx, uiScanIndex, bNeedEob, uiNumCoefRead ) );

  const UChar*  pucScan = (bFrame) ? g_aucFrameScan : g_aucFieldScan;
  for ( UInt ui = 0; uiScanIndex < 16 && ( ui < uiNumCoefRead || bNeedEob ); uiScanIndex++ )
  {
    if( !(pcCoefMap[uiScanIndex] & SIGNIFICANT) )
    {
      if( pcMbDataAccessEL->getMbTCoeffs().get( rcCIdx )[pucScan[uiScanIndex]] )
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
  CoefMap* pcCoefMapH;

  RefCtx*  pcRefCtx;
  if( pcMbDataAccessBL->getMbData().isTransformSize8x8() )
  {
    uiScanIndex = 4*uiScanIndex + (rcIdx.s4x4() & 3); // convert scan index into 8x8 scan index
    pcCoefMap = &rcMbFGSCoefMap.getCoefMap( c8x8Idx )[uiScanIndex];
    pcCoefMapH = &rcMbFGSCoefMap.getCoefMapH( c8x8Idx )[uiScanIndex];
    pcRefCtx  = &rcMbFGSCoefMap.getRefCtx( c8x8Idx )[uiScanIndex];
  }
  else
  {
    pcCoefMap = &rcMbFGSCoefMap.getCoefMap( rcIdx )[uiScanIndex];
    pcCoefMapH = &rcMbFGSCoefMap.getCoefMapH( rcIdx )[uiScanIndex];
    pcRefCtx  = &rcMbFGSCoefMap.getRefCtx( rcIdx )[uiScanIndex];
  }

  //===== check if coefficient is not significant or was already coded =====
  ROFRS( *pcCoefMap & SIGNIFICANT, Err::m_nOK );
  ROTRS( *pcCoefMap & CODED,       Err::m_nOK );

  if( ! m_pcSliceHeader->getPPS().getEntropyCodingModeFlag() )  //JVT-V095
  {
    UInt Qlevel=pcMbDataAccessEL->getSH().getQualityLevel()+1;
    Int uiPrevCoeffInd=0;
    if (Qlevel>1)
      uiPrevCoeffInd = *pcCoefMapH;
    ((UvlcReader *)m_pcSymbolReader)->setPrevLevInd(uiPrevCoeffInd);
  }
  Int iCoeff;
  
  if( pcMbDataAccessBL->getMbData().isTransformSize8x8() )
  {
    RNOKS( m_pcSymbolReader->RQdecodeTCoeffRef_8x8( *pcMbDataAccessEL, *pcMbDataAccessBL, c8x8Idx, uiScanIndex, *pcRefCtx ) );

    iCoeff = pcMbDataAccessEL->getMbTCoeffs().get8x8( c8x8Idx )[g_aucFrameScan64[uiScanIndex]];
    if( pcMbDataAccessBL->getMbTCoeffs().get8x8( c8x8Idx )[g_aucFrameScan64[uiScanIndex]] < 0 )
      iCoeff = -iCoeff;
  }
  else
  {
    RNOKS( m_pcSymbolReader->RQdecodeTCoeffRef_Luma( *pcMbDataAccessEL, *pcMbDataAccessBL, rcIdx, uiScanIndex, *pcRefCtx ) );

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

  RNOKS( rcMbFGSCoefMap.increaseAndCheckNumCoded( 1 ) );

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


Void 
RQFGSDecoder::xSignaling_Table( SliceHeader* pcSliceHeader)
{
  UInt QL   = pcSliceHeader->getQualityLevel();
  UInt TL   = pcSliceHeader->getTemporalLevel();
  UInt Type = pcSliceHeader->getSliceType();
  Bool sbit;

  m_pcUvlcReader->m_TableRefInit[1]= 0 ;	//Table for Intra
  m_pcUvlcReader->m_TableRefInit[0]= 0 ;	//Table for Inter

  if(QL>0)	
  {
    m_pcUvlcReader->getFlag(sbit,"TABLE_TYPE_FOR_INTRA");
	m_pcUvlcReader->m_TableRefInit[1]=(sbit==0)? 0 : 1 ;	//Table for Intra
	m_pcUvlcReader->getFlag(sbit,"TABLE_TYPE_FOR_INTER");
	m_pcUvlcReader->m_TableRefInit[0]=(sbit==0)? 0 : 1 ;	//Table for Inter
  }
}


Void 
RQFGSDecoder::xSignaling( SliceHeader* pcSliceHeader)
{
	UInt QL   = pcSliceHeader->getQualityLevel();
	UInt TL   = pcSliceHeader->getTemporalLevel();
	UInt Type = pcSliceHeader->getSliceType();
	Bool sbit;

	Bool useCT_Intra = 0;
	Bool useCT_Inter = 0;

	if(QL>1)
	{
		// Read two bits to decide whether to use codetype method for Intra/inter MB
		m_pcUvlcReader->getFlag(sbit,"useCodeTypeIntra"); 
		useCT_Intra = sbit;
		m_pcUvlcReader->getFlag(sbit,"useCodeTypeInter");
		useCT_Inter = sbit;

		if(useCT_Intra==1 && useCT_Inter==1)	//receive HistoryMap if we are using code_type method
		{
			if(QL==2)
			{
				for(UInt i=0;i<3;i++)
				{
					m_pcUvlcReader->getFlag(sbit,"TABLE_MAP_FOR_QL=2");
					m_pcUvlcReader->m_mapLev2[i]=sbit;
				}
			}
			if(QL==3)	
			{
				for(UInt i=0;i<7;i++)
				{
					m_pcUvlcReader->getFlag(sbit,"TABLE_MAP_FOR_QL=3");
					m_pcUvlcReader->m_mapLev3[i]=sbit;
				}
			}
		}
	}
	((UvlcReader *) m_pcSymbolReader)->useCodeTypeIntra = useCT_Intra;
	((UvlcReader *) m_pcSymbolReader)->useCodeTypeInter = useCT_Inter;
}


Void 
RQFGSDecoder::xReadCodeTypeRef	( 		SliceHeader*       pcSliceHeader,
										MbFGSCoefMap       &rcMbFGSCoefMap,
										UInt uiMbX,
										UInt uiMbY)
{

   //=============read codtype from bitstream here=====
   UInt Qlevel= pcSliceHeader->getQualityLevel();
   Bool bLowPass=(pcSliceHeader->getTemporalLevel()==0);
   Bool isIntra = m_pcCurrMbDataCtrl->getMbData( uiMbX, uiMbY ).isIntra();
   ((UvlcReader *)m_pcSymbolReader)->setQLevel(Qlevel);
   ((UvlcReader *)m_pcSymbolReader)->setMBMode(bLowPass || isIntra);
   UInt q = ((UvlcReader *)m_pcSymbolReader)->m_qLevel;
   Bool p = ((UvlcReader *)m_pcSymbolReader)->m_isIntra;
   UInt    uiMbIndex_r  = (uiMbY) * m_uiWidthInMB + (uiMbX);

   rcMbFGSCoefMap.getMbMapH  () = -1;

   if(q>1 && p)
   {
      Bool codtype_decoder;
      m_pcUvlcReader->getFlag(codtype_decoder,"");
      rcMbFGSCoefMap.getMbMapH  () = codtype_decoder;
   }
}

H264AVC_NAMESPACE_END

