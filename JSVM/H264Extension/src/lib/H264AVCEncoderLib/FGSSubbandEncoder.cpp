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
#include "MbEncoder.h"
#include "FGSSubbandEncoder.h"


#include "H264AVCCommonLib/MbDataCtrl.h"
#include "H264AVCCommonLib/IntFrame.h"
#include "H264AVCCommonLib/IntYuvMbBuffer.h"
#include "H264AVCCommonLib/Transform.h"
#include "H264AVCCommonLib/TraceFile.h"


#include "H264AVCCommonLib/CFMO.h"



H264AVC_NAMESPACE_BEGIN


RQFGSEncoder::RQFGSEncoder()
: m_pcMbEncoder               ( 0 )
, m_pcSymbolWriter            ( 0 )
, m_iRemainingTCoeff          ( 0 )
, m_pcOrgResidual             ( 0 )
, m_bTraceEnable              ( true )
, m_pcFGSPredFrame            ( 0 )
, m_pcRefFrameListDiff        ( 0 )
{
  m_pcCoefMap = NULL;
  m_pcSliceHeader       = 0;
  m_bUseDiscardableUnit =  false; //JVT-P031
}


RQFGSEncoder::~RQFGSEncoder()
{
  AOT( m_bInit );
}


ErrVal
RQFGSEncoder::create( RQFGSEncoder*& rpcRQFGSEncoder )
{
  rpcRQFGSEncoder = new RQFGSEncoder;
  ROT( NULL == rpcRQFGSEncoder );
  return Err::m_nOK;
}
  

ErrVal
RQFGSEncoder::destroy()
{
  delete this;
  return Err::m_nOK;
}


ErrVal
RQFGSEncoder::init( YuvBufferCtrl** apcYuvFullPelBufferCtrl,
                    YuvBufferCtrl**   apcYuvHalfPelBufferCtrl,
                    QuarterPelFilter* pcQuarterPelFilter,
                    MotionEstimation* pcMotionEstimation,
                    MbCoder*          pcMbCoder,
                    Transform*      pcTransform,
                    ControlMngH264AVCEncoder* pcControlMng,
                    MbEncoder*      pcMbEncoder )
{
  ROT( m_bInit );
  ROF( apcYuvFullPelBufferCtrl );
  ROF( apcYuvHalfPelBufferCtrl );
  ROF( pcQuarterPelFilter );
  ROF( pcMotionEstimation );
  ROF( pcMbCoder );
  ROF( pcTransform );
  ROF( pcControlMng );
  ROF( pcMbEncoder );

  m_pcMbEncoder               = pcMbEncoder;
  m_pcControlMng              = pcControlMng;
  m_iRemainingTCoeff          = 0;
  m_pcSliceHeader             = 0;
  m_pcOrgResidual             = 0;
  m_papcYuvHalfPelBufferCtrl  = apcYuvHalfPelBufferCtrl;
  m_pcQuarterPelFilter        = pcQuarterPelFilter;
  m_pcMotionEstimation        = pcMotionEstimation;
  m_pcMbCoder                 = pcMbCoder;

  xInit( apcYuvFullPelBufferCtrl, pcTransform );

  return Err::m_nOK;
}
  

ErrVal
RQFGSEncoder::uninit()
{
  ROT( m_bPicInit );
  ROF( m_bInit );
  
  xUninit();
  m_pcFGSPredFrame = 0;
  m_pcRefFrameListDiff = 0;

  m_pcMbEncoder               = 0;
  m_pcSymbolWriter            = 0;
  m_iRemainingTCoeff          = 0;
  m_pcSliceHeader             = 0;
  m_pcOrgResidual             = 0;
  return Err::m_nOK;
}


ErrVal
RQFGSEncoder::initSPS( const SequenceParameterSet& rcSPS )
{
  return xInitSPS( rcSPS );
}


ErrVal
RQFGSEncoder::initPicture( SliceHeader* pcSliceHeader,
                           MbDataCtrl*  pcCurrMbDataCtrl,
                           IntFrame*    pcOrgResidual,
                           IntFrame*     pcOrgFrame,
                           IntFrame*     pcPredSignal,
                           RefFrameList* pcRefFrameList0,
                           RefFrameList* pcRefFrameList1,
                           UInt          uiNumMaxIter,
                           UInt          uiIterSearchRange,
                           Double       dNumFGSLayers,
                           Double       dLambda,
                           Int          iMaxQpDelta,
                           Bool&        rbFinished,
                           Bool         bTruncate,
                           Bool         bUseDiscardableUnit) //JVT-P031
/*ErrVal
RQFGSEncoder::initPicture( SliceHeader* pcSliceHeader,
                           MbDataCtrl*  pcCurrMbDataCtrl,
                           IntFrame*    pcOrgResidual,
                           Double       dNumFGSLayers,
                           Double       dLambda,
                           Int          iMaxQpDelta,
                           Bool&        rbFinished,
                           Bool         bTruncate )*/
{
  ROT( m_bPicInit );
  ROF( m_bInit );
  ROF( pcSliceHeader );
  ROF( pcCurrMbDataCtrl );
  ROF( pcOrgResidual );
  ROF( dNumFGSLayers >= 0.0 );
  ROF( pcOrgFrame );
  ROF( pcPredSignal );

  m_uiWidthInMB       = pcSliceHeader->getSPS().getFrameWidthInMbs  ();
  m_uiHeightInMB      = pcSliceHeader->getSPS().getFrameHeightInMbs ();
	m_uiHeightInMB     = ( pcSliceHeader->getFieldPicFlag() ) ? m_uiHeightInMB>>1 : m_uiHeightInMB;
  m_pcSliceHeader     = pcSliceHeader;
  m_iMaxQpDelta       = iMaxQpDelta;
  m_dLambda           = dLambda;
  m_pcCurrMbDataCtrl  = pcCurrMbDataCtrl;
  m_pcOrgResidual     = pcOrgResidual;
  m_pcOrgFrame        = pcOrgFrame;
  m_pcPredSignal      = pcPredSignal;
  m_pcRefFrameList0   = pcRefFrameList0;
  m_pcRefFrameList1   = pcRefFrameList1;
  m_uiNumMaxIter      = uiNumMaxIter;
  m_uiIterSearchRange = uiIterSearchRange;

  ROF( m_uiWidthInMB*m_uiHeightInMB <= m_cMbDataCtrlEL.getSize() );

  UInt  uiNumTCoeff   = m_uiWidthInMB*m_uiHeightInMB*(16*16+2*8*8);
  m_iRemainingTCoeff  = (Int)floor( (Double)uiNumTCoeff * dNumFGSLayers + 0.5 );
  m_bPicInit          = true;
  m_bUseDiscardableUnit   = bUseDiscardableUnit; //JVT-P031
  if( bTruncate )
  {
    m_iRemainingTCoeff = MSYS_INT_MAX;
  }

  m_eSliceType        = m_pcSliceHeader->getSliceType ();
  Int iOldSliceQP     = m_pcSliceHeader->getPicQp     ();
  Int iNewSliceQP     = max( 0, iOldSliceQP - RQ_QP_DELTA );
  rbFinished          = ( iNewSliceQP == iOldSliceQP );

  m_uiFirstMbInSlice = m_pcSliceHeader->getFirstMbInSlice ();
  m_uiNumMbsInSlice  = m_pcSliceHeader->getNumMbsInSlice  ();
  m_bFgsComponentSep = m_pcSliceHeader->getFgsComponentSep();

  m_pcSliceHeader->setSliceType   ( F_SLICE );

  if( m_pcSliceHeader->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR )
  {
    m_pcSliceHeader->setNalUnitType ( NAL_UNIT_CODED_SLICE_IDR_SCALABLE );
  }
  else if( m_pcSliceHeader->getNalUnitType() == NAL_UNIT_CODED_SLICE )
  {
    m_pcSliceHeader->setNalUnitType ( NAL_UNIT_CODED_SLICE_SCALABLE );
  }

  Bool bCabac = m_pcSliceHeader->getPPS().getEntropyCodingModeFlag();
  m_pcSymbolWriter = ( bCabac ) ? (MbSymbolWriteIf*)m_pcControlMng->getCabacWriter() : (MbSymbolWriteIf*)m_pcControlMng->getUvlcWriter();

  m_pcSliceHeader->setQualityLevel( 1 );
  m_pcSliceHeader->setSliceHeaderQp       ( iNewSliceQP );
  m_dLambda          /= pow( 2.0, (Double)(iOldSliceQP-iNewSliceQP) / 3.0 );

  RNOK( xInitBaseLayerSbb( m_pcSliceHeader->getSPS().getLayerId() ) );
  RNOK( xInitializeCodingPath() );

  RNOK( xScaleBaseLayerCoeffs(false) );

  return Err::m_nOK;
}


ErrVal
RQFGSEncoder::initArFgs( IntFrame*     pcPredSignal,
                         RefFrameList* pcRefFrameListDiff )
{
  if( m_pcSliceHeader->getArFgsUsageFlag() )
  {
    m_pcFGSPredFrame     = pcPredSignal;
    m_pcRefFrameListDiff = pcRefFrameListDiff;
  }
  else
  {
    m_pcFGSPredFrame     = 0;
    m_pcRefFrameListDiff = 0;
  }
  return Err::m_nOK;
}


ErrVal
RQFGSEncoder::finishPicture()
{
  ROF( m_bPicInit );

  m_pcSliceHeader->setSliceType( m_eSliceType );
  
  m_bPicInit          = false;
  m_uiWidthInMB       = 0;
  m_uiHeightInMB      = 0;
  m_pcSliceHeader     = 0;
  m_pcCurrMbDataCtrl  = 0;
  m_pcOrgResidual     = 0;
  m_pcOrgFrame        = 0;
  m_pcPredSignal      = 0;

  return Err::m_nOK;
}

ErrVal
RQFGSEncoder::encodeNextLayer( Bool&  rbFinished,
                               Bool&  rbCorrupted,
                               UInt   uiMaxBits,
                               UInt*  puiPDFragBits,
                               UInt&  ruiNumPDFrags,
                               UInt uiFrac,
                               Bool bFragmented,
                               FILE*  pFile ) //JVT-P031
{
  ROF( m_bInit );
  ROF( m_bPicInit );
  rbCorrupted = false;
  rbFinished  = false;

  RNOK ( xEncodingFGS( rbFinished, rbCorrupted, uiMaxBits, puiPDFragBits, ruiNumPDFrags,uiFrac, pFile ) );

  return Err::m_nOK;
}


// FGS FMO ICU/ETRI
ErrVal
RQFGSEncoder::updateQP(Bool&  rbCorrupted, Bool& rbFinished, Bool bFragmented, UInt uiFrac,Bool isLastSlice)
{
	
  if(isLastSlice == false)
	  return Err::m_nOK;

	//FIX_FRAG_CAVLC
  if((bFragmented && (uiFrac != 0 || !rbCorrupted) ) || !bFragmented)
  {
	  Int iOldSliceQP     = m_pcSliceHeader->getPicQp();
	  Int iNewSliceQP     = max( 0, iOldSliceQP - RQ_QP_DELTA );
	  rbFinished          = ( iNewSliceQP == iOldSliceQP );

    if( m_pcSliceHeader->getFGSCycleAlignedFragment() || (bFragmented && uiFrac != 0) || !bFragmented)
		  m_pcSliceHeader->setQualityLevel ( m_pcSliceHeader->getQualityLevel() + 1 );
  
	  m_pcSliceHeader->setSliceHeaderQp       ( iNewSliceQP );
	  m_dLambda          /= pow( 2.0, (Double)(iOldSliceQP-iNewSliceQP) / 3.0 );
  }  //FIX_FRAG_CAVLC
 


  return Err::m_nOK;
}

// FGS FMO ICU/ETRI
ErrVal
RQFGSEncoder::setSliceGroup(Int iSliceGroupID)
{
  FMO* pcFMO = m_pcSliceHeader->getFMO();
  m_pcSliceHeader->setFirstMbInSlice(pcFMO->getFirstMacroblockInSlice(iSliceGroupID));
  m_pcSliceHeader->setLastMbInSlice(pcFMO->getLastMBInSliceGroup(iSliceGroupID));
  // JVT-S054 (2) (ADD)
  m_pcSliceHeader->setNumMbsInSlice(pcFMO->getNumMbsInSlice(m_pcSliceHeader->getFirstMbInSlice(), m_pcSliceHeader->getLastMbInSlice()));
  return Err::m_nOK;
}


// FGS FMO ICU/ETRI
ErrVal
RQFGSEncoder::prepareEncode(UInt uiFrac, UInt uiFracNb)
{
	if( ! uiFrac )
	{
		RNOK ( xMotionEstimation() );
		RNOK ( xResidualTransform() );
	}
	
	if( uiFracNb )
	{
		RNOK( xRestoreCodingPath() );
	}
	return Err::m_nOK;
}







const UChar COEFF_COST[16] =
{
  3, 2,2,1, 1,1,0,0,0,0,0,0,0,0,0,0
};

const UChar COEFF_COST8x8[64] =
{
  3,3,3,3,2,2,2,2,2,2,2,2,1,1,1,1,
  1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

const UChar MAX_VALUE = 100;



ErrVal
RQFGSEncoder::xSetSymbolsChroma( TCoeff*             piCoeff,
                                 UInt                uiMbX,
                                 UInt                uiMbY,
                                 UInt&               uiCoeffCostDC,
                                 UInt&               uiCoeffCostAC,
                                 Bool&               bSigDC,
                                 Bool&               bSigAC,
                                 ChromaIdx           cIdx,
                                 Bool               bFrame )
{
  Int iRun  = 0;
  UInt    uiMbIndex           = uiMbY * m_uiWidthInMB + uiMbX;
  CoefMap *pcCoefMap = m_pcCoefMap[uiMbIndex].getCoefMap( cIdx );
  const UChar*  pucScan = (bFrame) ? g_aucFrameScan : g_aucFieldScan;

  for( Int iPos = 0; iPos < 16; iPos++ )
  {
    Int iIndex      = pucScan[iPos];
    Int iLevel      = piCoeff       [iIndex];

    if( pcCoefMap[iPos] & SIGNIFICANT )
    {
      ROT( abs(iLevel) > 1 );
    }
    else
    {
      if( iPos == 0 )
      {
        if( iLevel )
        {
          if( abs( iLevel) == 1 )   uiCoeffCostDC += COEFF_COST[iRun];
          else                      uiCoeffCostDC += MAX_VALUE;
        
          bSigDC  = true;
        }
      }
      else
      {
        if( iLevel )
        {
          if( abs( iLevel) == 1 )   uiCoeffCostAC += COEFF_COST[iRun];
          else                      uiCoeffCostAC += MAX_VALUE;

          iRun   = 0;
          bSigAC = true;
        }
        else
        {
          iRun++;
        }
      }
    }
  }

  return Err::m_nOK;
}





ErrVal
RQFGSEncoder::xSetSymbols4x4( TCoeff*             piCoeff,
                              UInt                uiMbX,
                              UInt                uiMbY,
                              UInt&               uiCoeffCost,
                              UInt&               ruiCbp,
                              const S4x4Idx&      rcIdx,
                              UInt                uiStart,
                              Bool                bFrame  )
{
  Int   iRun  = 0;
  Bool  bSig  = false;
  CoefMap* pcCoefMap = m_pcCoefMap[uiMbY * m_uiWidthInMB + uiMbX ].getCoefMap( rcIdx );
  const UChar*  pucScan = (bFrame) ? g_aucFrameScan : g_aucFieldScan;
  for( Int iPos = uiStart; iPos < 16; iPos++ )
  {
    Int iIndex      = pucScan[iPos];
    Int iLevel      = piCoeff       [iIndex];

    if( pcCoefMap[iPos] & SIGNIFICANT )
      {
      ROT( abs(iLevel) > 1 );
    }
    else
    {
      if( iLevel )
      {
        if( abs( iLevel) == 1 )   uiCoeffCost += COEFF_COST[iRun];
        else                      uiCoeffCost += MAX_VALUE;

        iRun  = 0;
        bSig  = true;
      }
      else
      {
        iRun++;
      }
    }
  }

  if( bSig ) 
  {
    ruiCbp |= ( 1 << rcIdx );
  }

  return Err::m_nOK;
}






ErrVal
RQFGSEncoder::xSetSymbols8x8( TCoeff*             piCoeff,
                              UInt                uiMbX,
                              UInt                uiMbY,
                              UInt&               uiCoeffCost,
                              UInt&               ruiCbp,
                              B8x8Idx             cIdx,
                              Bool                bFrame )
{
  Int     iRun            = 0;
  Bool    bSig            = false;
  CoefMap* pcCoefMap = m_pcCoefMap[uiMbY * m_uiWidthInMB + uiMbX ].getCoefMap( cIdx );
  const UChar*  pucScan64 = (bFrame) ? g_aucFrameScan64 : g_aucFieldScan64;
  for( Int iPos = 0; iPos < 64; iPos++ )
  {
    Int iIndex = pucScan64[iPos];
    Int iLevel = piCoeff         [iIndex];
    if( pcCoefMap[iPos] & SIGNIFICANT )
      {
      ROT( abs(iLevel) > 1 );
    }
    else
    {
      if( iLevel )
      {
        if( abs( iLevel) == 1 )   uiCoeffCost += COEFF_COST8x8[iRun];
        else                      uiCoeffCost += MAX_VALUE;

        iRun  = 0;
        bSig  = true;
      }
      else
      {
        iRun++;
      }
    }
  }

  if( bSig )
  {
    ruiCbp |= ( 0x33 << cIdx );
  }

  return Err::m_nOK;
}




ErrVal
RQFGSEncoder::xRequantizeMacroblock( MbDataAccess&    rcMbDataAccess,
                                     MbDataAccess&    rcMbDataAccessBase )
{
#define COEF_SKIP 1

  UInt          uiExtCbp  = 0;
  Int           iQp       = max( 0, rcMbDataAccessBase.getMbData().getQp() - RQ_QP_DELTA );
  Bool          bIntra    = rcMbDataAccessBase.getMbData().isIntra();
  Bool          bIntra16x16 = rcMbDataAccessBase.getMbData().isIntra16x16();
  Bool          b8x8      = rcMbDataAccessBase.getMbData().isTransformSize8x8();
  Bool          bLowPass  = m_pcSliceHeader->getTemporalLevel() == 0;
  const UChar*  pucScaleY = rcMbDataAccessBase.getSH().getScalingMatrix( bIntra ? ( b8x8 ? 6 : 0 ) : ( b8x8 ? 7 : 3 ) );
  const UChar*  pucScaleU = rcMbDataAccessBase.getSH().getScalingMatrix( bIntra ? 1 : 4 );
  const UChar*  pucScaleV = rcMbDataAccessBase.getSH().getScalingMatrix( bIntra ? 2 : 5 );
  UInt          uiMbX     = rcMbDataAccess.getMbX();
  UInt          uiMbY     = rcMbDataAccess.getMbY();

  const PicType eMbPicType = rcMbDataAccess.getMbPicType();
  const Bool   bFrame    = (FRAME == rcMbDataAccess.getMbPicType());
  
  rcMbDataAccess.getMbData().setQp( iQp );

	//-- JVT-R091
	// use intra offset for smoothed reference MB
	if ( rcMbDataAccessBase.getMbData().getSmoothedRefFlag() )
	{
		m_pcTransform->setQp( rcMbDataAccess, true );
	}
	else
	{
		m_pcTransform->setQp( rcMbDataAccess, bLowPass || bIntra );
	}
	//--

  IntYuvMbBuffer  cMbBuffer;
  cMbBuffer.loadBuffer( m_pcOrgResidual->getPic( eMbPicType )->getFullPelYuvBuffer() );


  //===== luma =====
  MbFGSCoefMap* pcMbFGSCoefMap = &m_pcCoefMap[uiMbY * m_uiWidthInMB + uiMbX];
  if( b8x8 )
  {
    UInt  uiAbsSumMb    = 0;
    UInt  uiCoeffCostMb = 0;

    for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
    {
      UInt  uiCoeffCost8x8  = 0;
      UInt  uiAbsSum8x8     = 0;
      UInt  uiCbp           = 0;
      
      cMbBuffer.set4x4Block( c8x8Idx );
      RNOK( m_pcTransform->requant8x8Block( cMbBuffer,
                                            rcMbDataAccess    .getMbTCoeffs().get8x8( c8x8Idx ),
                                            rcMbDataAccessBase.getMbTCoeffs().get8x8( c8x8Idx ),
                                            pcMbFGSCoefMap->getRefCtx( c8x8Idx ),
                                            pucScaleY, uiAbsSum8x8 ) );
      uiAbsSumMb += uiAbsSum8x8;

      RNOK( xSetSymbols8x8( rcMbDataAccess    .getMbTCoeffs().get8x8( c8x8Idx ),
                            uiMbX, uiMbY,
                            uiCoeffCost8x8, uiCbp, c8x8Idx, bFrame ) );

      if( uiCbp )
      {
#if COEF_SKIP
        if( uiCoeffCost8x8 <= 4 && ! bIntra && ! bLowPass )
        {
          rcMbDataAccess.getMbTCoeffs().clearNewLumaLevels8x8Block( c8x8Idx, rcMbDataAccessBase.getMbTCoeffs(), pcMbFGSCoefMap );
        }
        else
#endif
        {
          uiCoeffCostMb += uiCoeffCost8x8;
          uiExtCbp      += uiCbp;
        }
      }
    }
#if COEF_SKIP
    if( uiExtCbp && uiCoeffCostMb <= 5 && ! bIntra && ! bLowPass )
    {
      rcMbDataAccess.getMbTCoeffs().clearNewLumaLevels( rcMbDataAccessBase.getMbTCoeffs(), pcMbFGSCoefMap, true );
      uiExtCbp = 0;
    }
#endif
  }
  else
  {
    UInt  uiAbsSumMb    = 0;
    UInt  uiCoeffCostMb = 0;

    for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
    {
      UInt  uiCoeffCost8x8  = 0;
      UInt  uiAbsSum8x8     = 0;
      UInt  uiCbp           = 0;

      for( S4x4Idx cIdx( c8x8Idx ); cIdx.isLegal( c8x8Idx ); cIdx++ )
      {
        UInt  uiAbsSum4x4 = 0;
        cMbBuffer.set4x4Block( cIdx );

        RNOK( m_pcTransform->requant4x4Block( cMbBuffer,
          rcMbDataAccess    .getMbTCoeffs().get( cIdx ),
          rcMbDataAccessBase.getMbTCoeffs().get( cIdx ),
          pcMbFGSCoefMap->getRefCtx( cIdx ),
          pucScaleY, 
          bIntra16x16,
          uiAbsSum4x4 ) );
        uiAbsSum8x8 += uiAbsSum4x4;
        if ( bIntra16x16 )
          uiAbsSum8x8 -= abs(rcMbDataAccess.getMbTCoeffs().get( cIdx )[0]);

        RNOK( xSetSymbols4x4( rcMbDataAccess    .getMbTCoeffs().get( cIdx ),
                              uiMbX, uiMbY, 
          uiCoeffCost8x8, uiCbp, cIdx, bIntra16x16, bFrame ) );
      }
      if( uiCbp )
      {
#if COEF_SKIP
        if( uiCoeffCost8x8 <= 4 && ! bIntra && ! bLowPass && ! bIntra16x16 )
        {
          rcMbDataAccess.getMbTCoeffs().clearNewLumaLevels8x8( c8x8Idx, rcMbDataAccessBase.getMbTCoeffs(), pcMbFGSCoefMap );
        }
        else
#endif
        {
          uiCoeffCostMb += uiCoeffCost8x8;
          uiExtCbp      += uiCbp;
        }
      }
      uiAbsSumMb += uiAbsSum8x8;
    }

    if ( bIntra16x16 )
    {
      UInt    uiAbsDc;
      MbTransformCoeffs& rcMbTCoeffs     = rcMbDataAccess.getMbTCoeffs();
      MbTransformCoeffs& rcMbTCoeffsBase = rcMbDataAccessBase.getMbTCoeffs();
      // transform and quantization on intra16x16 DC coefficients
      m_pcTransform->requantLumaDcCoeffs( rcMbTCoeffs, rcMbTCoeffsBase, *pcMbFGSCoefMap, pucScaleY, uiAbsDc );

      for( S4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
        {
        Int iLevel = rcMbTCoeffs.get( cIdx )[0];
        if( pcMbFGSCoefMap->getCoefMap( cIdx )[0] & SIGNIFICANT )
          {
          AOT( iLevel > 1 || iLevel < -1 );
        }
        else
        {
        // WARNING, should have "else" here because CBP is only for new-significant coefficients

        // since the DC coefficients are merged with the AC coefficients
        // update the cbp information
          uiExtCbp |= (iLevel != 0) << cIdx;
        }
      }
    }

#if COEF_SKIP
    if( ! bIntra16x16 )
    if( uiExtCbp && uiCoeffCostMb <= 5 && ! bIntra && ! bLowPass )
    {
      rcMbDataAccess.getMbTCoeffs().clearNewLumaLevels( rcMbDataAccessBase.getMbTCoeffs(), pcMbFGSCoefMap, false );
      uiExtCbp = 0;
    }
#endif
  }

  //===== chroma =====
  UInt  uiDcAbs       = 0;
  UInt  uiAcAbs       = 0;
  UInt  uiCoeffCostDC = 0;
  UInt  uiCoeffCostAC = 0;
  Bool  bSigDC        = false;
  Bool  bSigAC_U      = false;
  Bool  bSigAC_V      = false;

  RNOK( m_pcTransform->requantChroma( cMbBuffer,
                                      rcMbDataAccess.    getMbTCoeffs(),
                                      rcMbDataAccessBase.getMbTCoeffs(),
                                      *pcMbFGSCoefMap, pucScaleU, pucScaleV, uiDcAbs, uiAcAbs ) );
  //===== chrominance U =====
  {
    UInt  uiCoeffCostAC_U = 0;
    
    for( CIdx cIdxU(0); cIdxU.isLegal(4); cIdxU++ )
    {
      RNOK( xSetSymbolsChroma( rcMbDataAccess    .getMbTCoeffs().get( cIdxU ),
                               uiMbX, uiMbY, 
                               uiCoeffCostDC, uiCoeffCostAC_U, bSigDC, bSigAC_U, cIdxU, bFrame ) );
    }
#if COEF_SKIP
    if( uiCoeffCostAC_U < 4 )
    {
      for( CIdx cIdxU(0); cIdxU.isLegal(4); cIdxU++ )
      {
        rcMbDataAccess.getMbTCoeffs().clearNewAcBlk( cIdxU, *pcMbFGSCoefMap, rcMbDataAccessBase.getMbTCoeffs() );
      }
      bSigAC_U = false;
    }
    else
#endif
    {
      uiCoeffCostAC += uiCoeffCostAC_U;
    }
  }

  //===== chrominance V =====
  {
    UInt  uiCoeffCostAC_V = 0;
    
    for( CIdx cIdxV(4); cIdxV.isLegal(8); cIdxV++ )
    {
      RNOK( xSetSymbolsChroma( rcMbDataAccess    .getMbTCoeffs().get( cIdxV ),
                               uiMbX, uiMbY, 
                               uiCoeffCostDC, uiCoeffCostAC_V, bSigDC, bSigAC_V, cIdxV, bFrame ) );
    }
#if COEF_SKIP
    if( uiCoeffCostAC_V < 4 )
    {
      for( CIdx cIdxV(4); cIdxV.isLegal(8); cIdxV++ )
      {
        rcMbDataAccess.getMbTCoeffs().clearNewAcBlk( cIdxV, *pcMbFGSCoefMap, rcMbDataAccessBase.getMbTCoeffs() );
      }
      bSigAC_V = false;
    }
    else
#endif
    {
      uiCoeffCostAC += uiCoeffCostAC_V;
    }
  }

  //===== set CBP =====
  UInt  uiChromaCBP = ( bSigAC_U || bSigAC_V ? 2 : bSigDC ? 1 : 0 );
  uiExtCbp         |= ( uiChromaCBP << 16 );

  rcMbDataAccess.getMbData().setAndConvertMbExtCbp( uiExtCbp );

  return Err::m_nOK;
#undef COEF_SKIP
}


ErrVal
RQFGSEncoder::xMotionEstimation()
{
  RNOK( m_cMbDataCtrlEL    .clear     () );
  RNOK( m_cMbDataCtrlEL    .reset     () );
  RNOK( m_cMbDataCtrlEL    .initSlice ( *m_pcSliceHeader, ENCODE_PROCESS, false, NULL ) );
  RNOK( m_pcCurrMbDataCtrl->initSlice ( *m_pcSliceHeader, PRE_PROCESS,    false, NULL ) );
  ROTRS( m_eSliceType == I_SLICE, Err::m_nOK );

  UInt            uiLayer             = m_pcSliceHeader->getSPS().getLayerId();
  YuvBufferCtrl*  pcYuvFullBufferCtrl = m_papcYuvFullPelBufferCtrl[uiLayer];
  YuvBufferCtrl*  pcYuvHalfBufferCtrl = m_papcYuvHalfPelBufferCtrl[uiLayer];

  const Bool    bMbAff   = m_pcSliceHeader->isMbAff();// TMM_INTERLACE

  m_pcSliceHeader->setSliceType( m_eSliceType );
  for( UInt uiMbY = 0; uiMbY < m_uiHeightInMB; uiMbY++ )
  for( UInt uiMbX = 0; uiMbX < m_uiWidthInMB;  uiMbX++ )
  {
    MbDataAccess* pcMbDataAccessCurr  = 0;
    MbDataAccess* pcMbDataAccessEL    = 0;

    RNOK( m_pcCurrMbDataCtrl  ->initMb( pcMbDataAccessCurr, uiMbY, uiMbX ) );
    RNOK( m_cMbDataCtrlEL      .initMb( pcMbDataAccessEL,   uiMbY, uiMbX ) );
    RNOK( pcYuvFullBufferCtrl ->initMb(                     uiMbY, uiMbX, bMbAff ) );// TMM_INTERLACE
    RNOK( pcYuvHalfBufferCtrl ->initMb(                     uiMbY, uiMbX, bMbAff  ) );// TMM_INTERLACE
    RNOK( m_pcMotionEstimation->initMb( uiMbY, uiMbX, *pcMbDataAccessEL  ) );

    if( pcMbDataAccessCurr->getMbData().isIntra() || ! m_pcSliceHeader->getAdaptivePredictionFlag() )
    {
      pcMbDataAccessEL->getMbData().copyFrom  ( pcMbDataAccessCurr->getMbData() );
      pcMbDataAccessEL->getMbData().setBCBP   ( 0 );
      RNOK( pcMbDataAccessEL->getMbData().copyMotion( pcMbDataAccessCurr->getMbData() ) );
      if( ! pcMbDataAccessCurr->getMbData().isIntra() )
        pcMbDataAccessEL->getMbData().setBLSkipFlag( true );
    }
    else
    {
      //----- reconstruct base layer -----
      IntYuvMbBuffer cBLRecBuffer;
      RNOK( xReconstructMacroblock( *pcMbDataAccessCurr, cBLRecBuffer ) );
      //----- get new motion data -----
      RNOK( m_pcMbEncoder->encodeFGS( *pcMbDataAccessEL,
                                      pcMbDataAccessCurr,
                                      *m_pcRefFrameList0,
                                      *m_pcRefFrameList1,
                                      *m_pcOrgFrame,
                                      m_pcPredSignal,
                                      m_pcFGSPredFrame,
                                      m_pcRefFrameListDiff,
                                      this,
                                      cBLRecBuffer,
                                      m_uiNumMaxIter,
                                      m_uiIterSearchRange,
                                      m_dLambda,
                                      m_iMaxQpDelta ) );

      if( ! pcMbDataAccessEL->getMbData().getResidualPredFlag( PART_16x16 ) )
      {
        ROT( pcMbDataAccessEL->getMbData().getBLSkipFlag() );
        //----- motion refinement without residual prediction ===> clear base layer coeffs -----
        RNOK( xClearBaseCoeffs( *pcMbDataAccessEL, pcMbDataAccessCurr ) );
      }
    }
  }
  m_pcSliceHeader->setSliceType( F_SLICE );

  return Err::m_nOK;
}


ErrVal
RQFGSEncoder::xEncodeMotionData( UInt uiMbYIdx, UInt uiMbXIdx )
{
  ETRACE_DECLARE( UInt          uiMbIndex         = uiMbYIdx * m_uiWidthInMB + uiMbXIdx );
  MbDataAccess* pcMbDataAccessEL  = 0;
  MbDataAccess* pcMbDataAccessBL  = 0;

  RNOK( m_cMbDataCtrlEL     .initMb( pcMbDataAccessEL, uiMbYIdx, uiMbXIdx ) );
  RNOK( m_pcCurrMbDataCtrl ->initMb( pcMbDataAccessBL, uiMbYIdx, uiMbXIdx ) );
  ROT ( pcMbDataAccessBL->getMbData().isIntra() );

  ETRACE_NEWMB( uiMbIndex );
  m_pcSliceHeader  ->setSliceType( m_eSliceType );
  RNOK( m_pcMbCoder->encodeMotion( *pcMbDataAccessEL, pcMbDataAccessBL ) );
  m_pcSliceHeader  ->setSliceType( F_SLICE );

  return Err::m_nOK;
}


ErrVal
RQFGSEncoder::xResidualTransform(  )
{
  UInt            uiLayer         = m_pcSliceHeader->getSPS().getLayerId();
  YuvBufferCtrl*  pcYuvBufferCtrl = m_papcYuvFullPelBufferCtrl[uiLayer];
  Bool            bLowPass        = m_pcSliceHeader->getTemporalLevel() == 0;
  const PicType ePicType = m_pcSliceHeader->getPicType();
	const Bool    bMbAff   = m_pcSliceHeader->isMbAff   ();
  if( ePicType!=FRAME )
	{
		RNOK( m_pcOrgResidual->addFieldBuffer     ( ePicType ) );
	}
	else if( bMbAff )
	{
	  RNOK( m_pcOrgResidual->addFrameFieldBuffer() );
	}

  RNOK( m_pcOrgResidual->subtract( m_pcOrgFrame,    m_pcPredSignal   ) );
  RNOK( m_pcOrgResidual->subtract( m_pcOrgResidual, m_pcBaseLayerSbb ) );

  for( UInt uiMbY = 0; uiMbY < m_uiHeightInMB; uiMbY++ )
  for( UInt uiMbX = 0; uiMbX < m_uiWidthInMB;  uiMbX++ )
  {
    MbDataAccess* pcMbDataAccessBL  = 0;
    MbDataAccess* pcMbDataAccessEL    = 0;

    RNOK( m_pcCurrMbDataCtrl->initMb( pcMbDataAccessBL, uiMbY, uiMbX ) );
    RNOK( m_cMbDataCtrlEL    .initMb( pcMbDataAccessEL,   uiMbY, uiMbX ) );
      if( bMbAff )
    {
      pcMbDataAccessEL->setFieldMode( pcMbDataAccessBL->getMbData().getFieldFlag()      );
    }
    RNOK( pcYuvBufferCtrl   ->initMb         (                   uiMbY, uiMbX, bMbAff ) );


    UInt uiMbIndex = uiMbY * m_uiWidthInMB + uiMbX;
    MbFGSCoefMap &rcMbFGSCoefMap = m_pcCoefMap[uiMbIndex];
    if( pcMbDataAccessBL->getMbData().isIntra16x16() || (rcMbFGSCoefMap.getMbMap() & SIGNIFICANT) )
    {
      Bool bUseTransformSizeFromRDO = ! ( rcMbFGSCoefMap.getMbMap() & TRANSFORM_SPECIFIED ) && ! pcMbDataAccessBL->getMbData().isIntra();
      if( bUseTransformSizeFromRDO )
      {
        //----- set transform size from RDO (i.e., MbEncoder::encodeFGS()) if possible -----
        pcMbDataAccessBL->getMbData().setTransformSize8x8( pcMbDataAccessEL->getMbData().isTransformSize8x8() );
      }
      //===== requantization =====
      RNOK( xRequantizeMacroblock ( *pcMbDataAccessEL, *pcMbDataAccessBL ) );
      if( bUseTransformSizeFromRDO && ( pcMbDataAccessEL->getMbData().getMbCbp() & 0x0F ) == 0 )
      {
        //----- clear transform size flag if no luma samples coded -----
        pcMbDataAccessBL->getMbData().setTransformSize8x8( false );
      }
    }
    else
    {

      //===== new encoding =====
      m_pcTransform->setClipMode( false );
      pcMbDataAccessEL->getMbData().setQp( m_pcSliceHeader->getPicQp() );
      Bool bResidualPrediction = pcMbDataAccessEL->getMbData().getResidualPredFlag( PART_16x16 );
      RNOK( m_pcMbEncoder->encodeResidual( *pcMbDataAccessEL,
                                           *pcMbDataAccessBL,
                                           rcMbFGSCoefMap,
																					 m_pcOrgResidual->getPic( m_pcSliceHeader->getPicType() ),
                                           m_dLambda,
                                           bLowPass,
                                           m_iMaxQpDelta ) );
      pcMbDataAccessEL->getMbData().setResidualPredFlag( bResidualPrediction );
      m_pcTransform->setClipMode( true );
    }
   pcMbDataAccessEL->getMbData().setBCBP(0);
  }

 	if( ePicType!=FRAME )
	{
		RNOK( m_pcOrgResidual->removeFieldBuffer     ( ePicType ) );
	}
	else if( bMbAff )
	{
	  RNOK( m_pcOrgResidual->removeFrameFieldBuffer()           );
	}
  return Err::m_nOK;
}



//JVT-P031
ErrVal RQFGSEncoder::xSaveCodingPath()
{
    typedef MbTransformCoeffs* pMbTransformCoeffs;
    m_aMyELTransformCoefs = new pMbTransformCoeffs[m_uiHeightInMB*m_uiWidthInMB];
    m_aMyBLTransformCoefs = new pMbTransformCoeffs[m_uiHeightInMB*m_uiWidthInMB];
    m_auiMbCbpStored = new UInt[m_uiHeightInMB*m_uiWidthInMB];
    m_auiBCBPStored = new UInt[m_uiHeightInMB*m_uiWidthInMB];
    m_aiBLQP = new Int[m_uiHeightInMB*m_uiWidthInMB];
    m_abELtransform8x8 = new Bool[m_uiHeightInMB*m_uiWidthInMB];

    for( UInt uiMbY = 0; uiMbY < m_uiHeightInMB; uiMbY++ )
  {
    for( UInt uiMbX = 0; uiMbX < m_uiWidthInMB;  uiMbX++ )
    {
        m_aMyELTransformCoefs[uiMbY+uiMbX*m_uiHeightInMB] = new MbTransformCoeffs();
        m_aMyBLTransformCoefs[uiMbY+uiMbX*m_uiHeightInMB] = new MbTransformCoeffs();

        //--- Get MbData for BL and EL
      MbDataAccess* pcMbDataAccessBL = 0;
      MbDataAccess* pcMbDataAccessEL = 0;

      RNOK( m_pcCurrMbDataCtrl->initMb( pcMbDataAccessBL, uiMbY, uiMbX ) );
      RNOK( m_cMbDataCtrlEL    .initMb( pcMbDataAccessEL, uiMbY, uiMbX ) );

      //--------------------------------------------------------
      //--- Perform some backup of BL and EL 
      // coeffs
      m_aMyBLTransformCoefs[uiMbY+uiMbX*m_uiHeightInMB]->copyFrom(pcMbDataAccessBL->getMbData().getMbTCoeffs());
      m_aMyELTransformCoefs[uiMbY+uiMbX*m_uiHeightInMB]->copyFrom(pcMbDataAccessEL->getMbData().getMbTCoeffs());
      // store CBP before update (but it seems not to have any influence)
      m_auiMbCbpStored[uiMbY+uiMbX*m_uiHeightInMB] = pcMbDataAccessBL->getMbData().getMbExtCbp();
      m_auiBCBPStored[uiMbY+uiMbX*m_uiHeightInMB] = pcMbDataAccessBL->getMbData().getBCBP();
      m_aiBLQP[uiMbY+uiMbX*m_uiHeightInMB] = pcMbDataAccessBL->getMbData().getQp();
      m_abELtransform8x8[uiMbY+uiMbX*m_uiHeightInMB] = pcMbDataAccessEL->getMbData().isTransformSize8x8();
    }
  }

    return Err::m_nOK;
}

ErrVal RQFGSEncoder::xRestoreCodingPath()
{
	//--ICU/ETRI FMO
	UInt uiFirstMbInSlice;
	UInt uiLastMbInSlice;
	
	if(m_pcSliceHeader !=NULL)
	{
		uiFirstMbInSlice  = m_pcSliceHeader->getFirstMbInSlice();
		uiLastMbInSlice  = m_pcSliceHeader->getLastMbInSlice();  
	}
	else
	{
		uiFirstMbInSlice =0;
		uiLastMbInSlice  = (m_uiWidthInMB*m_uiHeightInMB) -1;

	}
	
	for(UInt uiMbAddress= uiFirstMbInSlice ;uiMbAddress<=uiLastMbInSlice ;)
	{
    UInt uiMbY  = uiMbAddress / m_uiWidthInMB;
    UInt uiMbX  = uiMbAddress % m_uiWidthInMB;
    //-------------------------------------------------------
      //--- Get MbData for BL and EL
      MbDataAccess* pcMbDataAccessBL = 0;
      MbDataAccess* pcMbDataAccessEL = 0;

      RNOK( m_pcCurrMbDataCtrl->initMb( pcMbDataAccessBL, uiMbY, uiMbX ) );
      RNOK( m_cMbDataCtrlEL    .initMb( pcMbDataAccessEL, uiMbY, uiMbX ) );

      //----- restore informations on BL and EL
      pcMbDataAccessBL->getMbData().setBCBP(m_auiBCBPStored[uiMbY+uiMbX*m_uiHeightInMB]);
      pcMbDataAccessBL->getMbData().setMbExtCbp(m_auiMbCbpStored[uiMbY+uiMbX*m_uiHeightInMB]);
      pcMbDataAccessBL->getMbData().setQp(m_aiBLQP[uiMbY+uiMbX*m_uiHeightInMB]);
      // restore Coeffs
      m_cMbDataCtrlEL.getMbData(uiMbX,uiMbY).getMbTCoeffs().copyFrom(*m_aMyELTransformCoefs[uiMbY+uiMbX*m_uiHeightInMB]);
      m_pcCurrMbDataCtrl->getMbData(uiMbX,uiMbY).getMbTCoeffs().copyFrom(*m_aMyBLTransformCoefs[uiMbY+uiMbX*m_uiHeightInMB]);
      pcMbDataAccessEL->getMbData().setTransformSize8x8( m_abELtransform8x8[uiMbY+uiMbX*m_uiHeightInMB] );

			//--ICU/ETRI FMO Implementation
	    if(m_pcSliceHeader !=NULL)
	      uiMbAddress = m_pcSliceHeader->getFMO()->getNextMBNr(uiMbAddress );
	     else
	     uiMbAddress ++;
	}	
    delete []m_aMyELTransformCoefs;
    delete []m_aMyBLTransformCoefs;
    delete []m_auiMbCbpStored;
    delete []m_auiBCBPStored;
    delete []m_aiBLQP;
    delete []m_abELtransform8x8;

    return Err::m_nOK;
}

ErrVal
RQFGSEncoder::xStoreFGSState(UInt iLumaScanIdx,
                             UInt iChromaDCScanIdx,
                             UInt iChromaACScanIdx,
                             UInt iStartCycle,
                             UInt iCycle,
                             UInt uiPass,
                             UInt uiFragIdx,
                             UInt uiMbYIdx,
                             UInt uiMbXIdx,
                             B8x8Idx c8x8Idx,
                             S4x4Idx cIdx,
                             UInt iLastBitsLuma,
                             UInt uiBitsLast,
                             UInt uiFGSPart,
                             CPlaneIdx cCPlaneIdx,
                             Int iLastQP)
{
  m_iLumaScanIdx = iLumaScanIdx;
  m_iChromaDCScanIdx = iChromaDCScanIdx;
  m_iChromaACScanIdx = iChromaACScanIdx;
  m_iStartCycle = iStartCycle;
  m_iCycle = iCycle;
  m_uiPass = uiPass;
  m_uiFragIdx = uiFragIdx;
  m_uiMbYIdx = uiMbYIdx;
  m_uiMbXIdx = uiMbXIdx;
  m_c8x8Idx = c8x8Idx;
  m_cIdx    = cIdx;
  m_iLastBitsLuma = iLastBitsLuma;
  m_uiBitsLast = uiBitsLast;
  m_uiFGSPart = uiFGSPart;
  m_cCPlaneIdx = cCPlaneIdx;
  m_iLastQP = iLastQP;

  return Err::m_nOK;
}

ErrVal
RQFGSEncoder::xRestoreFGSState(UInt& riLumaScanIdx,
                             UInt& riChromaDCScanIdx,
                             UInt& riChromaACScanIdx,
                             UInt& riStartCycle,
                             UInt& riCycle,
                             UInt& ruiPass,
                             UInt& ruiFragIdx,
//                              UInt& ruiMbYIdx,
//                              UInt& ruiMbXIdx,
//                              UInt& ruiB8YIdx,
//                              UInt& ruiB8XIdx,
//                              UInt& ruiBlockYIdx,
//                              UInt& ruiBlockXIdx,
                             UInt& riLastBitsLuma,
//                              UInt& ruiBitsLast,
//                              UInt& ruiFGSPart,
//                              UInt& ruiPlane,
                             Int& riLastQP)
{
  riLumaScanIdx = m_iLumaScanIdx;
  riChromaDCScanIdx = m_iChromaDCScanIdx;
  riChromaACScanIdx = m_iChromaACScanIdx;
  riStartCycle = m_iStartCycle;
  riCycle = m_iCycle;
  ruiPass = m_uiPass;
  ruiFragIdx = m_uiFragIdx;
//   ruiMbYIdx = m_uiMbYIdx;
//   ruiMbXIdx = m_uiMbXIdx;
//   ruiB8YIdx = m_uiB8YIdx;
//   ruiB8XIdx = m_uiB8XIdx;
//   ruiBlockYIdx = m_uiBlockYIdx;
//   ruiBlockXIdx = m_uiBlockXIdx;
  riLastBitsLuma = m_iLastBitsLuma;
//   ruiBitsLast = m_uiBitsLast;
//   ruiFGSPart = m_uiFGSPart;
//   ruiPlane = m_uiPlane;
  riLastQP = m_iLastQP;

  return Err::m_nOK;
}
//~JVT-P031

UInt gauiB8x8Mapping[4] = { 0, 2, 3, 1 }; 

static UInt UvlcCodeLength(UInt uiSymbol)
{
  UInt uiCodeLength = 1;

  uiSymbol ++;
  while( 1 != uiSymbol )
  {
    uiSymbol >>= 1;
    uiCodeLength += 2;
  }

  return uiCodeLength;
}

ErrVal
RQFGSEncoder::xEncodeLumaCbpVlcStart(UInt&  uiXLumaCbpNextMbX,
                                     UInt&  uiXLumaCbpNextMbY,
                                     UInt&  uiXLumaCbpNext8x8Idx,
                                     UInt   uiXLastMbX,
                                     UInt   uiXLastMbY,
                                     UInt&  ruiLumaCbpBitCount)
{
  UInt uiMbXIdx = 0, uiMbYIdx = 0, uiB8x8 = 0;
  UInt uiLumaCbpBase, uiLumaCbp;
  MbDataAccess *pcMbDataAccessEL, *pcMbDataAccessBL;
  Bool bLumaCbpCodedFlag = false;

  UInt uiFirstMbInSlice  = uiXLumaCbpNextMbY*m_uiWidthInMB+uiXLumaCbpNextMbX ;
  UInt uiLastMbInSlice   = uiXLastMbY*m_uiWidthInMB+uiXLastMbX ;

  for(UInt uiMbAddress= uiFirstMbInSlice ;uiMbAddress<=uiLastMbInSlice ;)
  {
        uiMbYIdx = uiMbAddress / m_uiWidthInMB;
        uiMbXIdx = uiMbAddress % m_uiWidthInMB;

      RNOK( m_pcCurrMbDataCtrl ->initMb( pcMbDataAccessBL, uiMbYIdx, uiMbXIdx ) );
      RNOK( m_cMbDataCtrlEL     .initMb( pcMbDataAccessEL, uiMbYIdx, uiMbXIdx ) );

      uiLumaCbpBase = pcMbDataAccessBL->getMbData().getMbCbp() & 0x0F;
      uiLumaCbp     = pcMbDataAccessEL->getMbData().getMbCbp() & 0x0F;

      if( uiLumaCbpBase != 15 )
      {
        for( uiB8x8 = 0; uiB8x8 < 4; uiB8x8 ++ )
        {
          Bool bLumaCbpFlagBase = (uiLumaCbpBase >> gauiB8x8Mapping[uiB8x8]) & 1;
          if( ! bLumaCbpFlagBase )
          {
            m_bLastLumaCbpFlag  = (uiLumaCbp     >> gauiB8x8Mapping[uiB8x8]) & 1;
            break;
          }
        }

        bLumaCbpCodedFlag = true;
        break;
      }
      else
        pcMbDataAccessEL->getMbData().setMbCbp(pcMbDataAccessEL->getMbData().getMbCbp() | uiLumaCbpBase);


    if( bLumaCbpCodedFlag )
      break;

    //--ICU/ETRI FMO Implementation
    uiMbAddress = m_pcSliceHeader->getFMO()->getNextMBNr(uiMbAddress );
  }

  uiXLumaCbpNextMbX       = uiMbXIdx;
  uiXLumaCbpNextMbY       = uiMbYIdx;
  uiXLumaCbpNext8x8Idx    = uiB8x8;
  m_uiLumaCbpRun  = 0;

  ((UvlcWriter *) m_pcSymbolWriter)->writeFlag(! m_bLastLumaCbpFlag, "Luma_CBP_first_flag");
  ruiLumaCbpBitCount ++;

  m_uiLumaCbpNextMbX = uiXLumaCbpNextMbX;
  m_uiLumaCbpNextMbY = uiXLumaCbpNextMbY;
  m_uiLumaCbpNext8x8Idx = uiXLumaCbpNext8x8Idx;

  return Err::m_nOK;
}

ErrVal
RQFGSEncoder::xEncodeLumaCbpVlc(UInt  uiCurrMbIdxX,
                                UInt  uiCurrMbIdxY,
                                UInt& uiXLumaCbpNextMbX,
                                UInt& uiXLumaCbpNextMbY,
                                UInt& uiXLumaCbpNext8x8Idx,
                                UInt  uiXLastMbX,
                                UInt  uiXLastMbY,
                                UInt& ruiCbpBitCount)
{
  Bool bFirstMb  = true;
  MbDataAccess *pcMbDataAccessEL, *pcMbDataAccessBL;

  if( uiCurrMbIdxX != uiXLumaCbpNextMbX || uiCurrMbIdxY != uiXLumaCbpNextMbY )
    return Err::m_nOK;

  UInt uiFirstMbInSlice  = uiXLumaCbpNextMbY*m_uiWidthInMB+uiXLumaCbpNextMbX ;
  UInt uiLastMbInSlice   = uiXLastMbY*m_uiWidthInMB+uiXLastMbX ;

  for(UInt uiMbAddress= uiFirstMbInSlice ;uiMbAddress<=uiLastMbInSlice ;)
  {
        UInt uiMbYIdx = uiMbAddress / m_uiWidthInMB;
        UInt uiMbXIdx = uiMbAddress % m_uiWidthInMB;
  {
    {
      RNOK( m_pcCurrMbDataCtrl ->initMb( pcMbDataAccessBL, uiMbYIdx, uiMbXIdx ) );
      RNOK( m_cMbDataCtrlEL     .initMb( pcMbDataAccessEL, uiMbYIdx, uiMbXIdx ) );

      UInt uiLumaCbpBase = pcMbDataAccessBL->getMbData().getMbCbp() & 0x0F;
      UInt uiLumaCbp     = pcMbDataAccessEL->getMbData().getMbCbp() & 0x0F;

      for( UInt uiB8x8 = bFirstMb ? uiXLumaCbpNext8x8Idx : 0; uiB8x8 < 4; uiB8x8 ++ )
      {
        Bool bLumaCbpFlagBase = (uiLumaCbpBase >> gauiB8x8Mapping[uiB8x8]) & 1;
        Bool bLumaCbpFlag     = (uiLumaCbp     >> gauiB8x8Mapping[uiB8x8]) & 1;

        if( ! bLumaCbpFlagBase )
        {
          if( bLumaCbpFlag == m_bLastLumaCbpFlag )
            m_uiLumaCbpRun ++;
          else
          {
            ((UvlcWriter *) m_pcSymbolWriter)->writeUvlc(m_uiLumaCbpRun - 1, "Luma_CBP_run");
            ruiCbpBitCount += UvlcCodeLength(m_uiLumaCbpRun - 1);

            m_bLastLumaCbpFlag = bLumaCbpFlag;

            // is it in the next MB?
            if( uiCurrMbIdxX == uiMbXIdx && uiCurrMbIdxY == uiMbYIdx )
              m_uiLumaCbpRun  = 1;
            else
            {
              uiXLumaCbpNextMbX       = uiMbXIdx;
              uiXLumaCbpNextMbY       = uiMbYIdx;
              uiXLumaCbpNext8x8Idx    = uiB8x8;
              m_uiLumaCbpRun  = 0;

              m_uiLumaCbpNextMbX = uiXLumaCbpNextMbX;
              m_uiLumaCbpNextMbY = uiXLumaCbpNextMbY;
              m_uiLumaCbpNext8x8Idx = uiXLumaCbpNext8x8Idx;

              return Err::m_nOK;
            }
          }
        }
      }

      pcMbDataAccessEL->getMbData().setMbCbp(pcMbDataAccessEL->getMbData().getMbCbp() | uiLumaCbpBase);

      bFirstMb = false;
    }
  }

  uiMbAddress = m_pcSliceHeader->getFMO()->getNextMBNr(uiMbAddress );

  } // end for FGS ROI 1

  if( m_uiLumaCbpRun > 0 )
  {
    ((UvlcWriter *) m_pcSymbolWriter)->writeUvlc(m_uiLumaCbpRun - 1, "Luma_CBP_run");

    ruiCbpBitCount += UvlcCodeLength(m_uiLumaCbpRun - 1);

    m_uiLumaCbpRun = 0;

    uiXLumaCbpNextMbX = uiXLastMbX;
    uiXLumaCbpNextMbY = uiXLastMbY;

    m_uiLumaCbpNextMbX = uiXLumaCbpNextMbX;
    m_uiLumaCbpNextMbY = uiXLumaCbpNextMbY;
  }

  return Err::m_nOK;
}

Bool gaabTransitionFlag[3][3] = { { 0, 0, 1 }, { 1, 0, 0 }, { 0, 1, 0 } };


ErrVal
RQFGSEncoder::xEncodeChromaCbpVlcStart(UInt   uiCurrMbIdxX,
                                       UInt   uiCurrMbIdxY,
                                       UInt&  ruiChromaCbpBitCount)
{
  MbDataAccess *pcMbDataAccessEL;
  UInt uiChromaCbp;
  UInt uiInitialChromaCbp;

  RNOK( m_cMbDataCtrlEL     .initMb( pcMbDataAccessEL, uiCurrMbIdxY, uiCurrMbIdxX ) );
  uiChromaCbp = pcMbDataAccessEL->getMbData().getMbCbp() >> 4;

  uiInitialChromaCbp = uiChromaCbp == 0 ? 1 : 0;
  ((UvlcWriter *) m_pcSymbolWriter)->writeFlag(uiInitialChromaCbp != 0, "Chroma_CBP_first");
  ruiChromaCbpBitCount ++;

  m_bChromaCbpTransition = gaabTransitionFlag[uiInitialChromaCbp][uiChromaCbp];

  m_uiLastChromaCbp = uiChromaCbp;
  m_uiChromaCbpRun  = 0;

  return Err::m_nOK;
}


ErrVal
RQFGSEncoder::xEncodeChromaCbpVlc(UInt  uiCurrMbIdxX,
                                  UInt  uiCurrMbIdxY,
                                  UInt& uiXChromaCbpNextMbX,
                                  UInt& uiXChromaCbpNextMbY,
                                  UInt  uiXLastMbX,
                                  UInt  uiXLastMbY,
                                  UInt& ruiChromaCbpBitCount)
{
  MbDataAccess *pcMbDataAccessEL;

  if( uiCurrMbIdxX != uiXChromaCbpNextMbX || uiCurrMbIdxY != uiXChromaCbpNextMbY )
    return Err::m_nOK;

  UInt uiFirstMbInSlice  = uiXChromaCbpNextMbY*m_uiWidthInMB+uiXChromaCbpNextMbX ;
  UInt uiLastMbInSlice   = uiXLastMbY*m_uiWidthInMB+uiXLastMbX ;


  for(UInt uiMbAddress= uiFirstMbInSlice ;uiMbAddress<=uiLastMbInSlice ;)
  {
  UInt uiMbYIdx = uiMbAddress / m_uiWidthInMB;
    UInt uiMbXIdx = uiMbAddress % m_uiWidthInMB;
  {
    {
      RNOK( m_cMbDataCtrlEL.initMb( pcMbDataAccessEL, uiMbYIdx, uiMbXIdx ) );
      UInt uiChromaCbp = (pcMbDataAccessEL->getMbData().getMbCbp() >> 4);

      if( uiChromaCbp == m_uiLastChromaCbp )
        m_uiChromaCbpRun ++;
      else
      {
        ((UvlcWriter *) m_pcSymbolWriter)->writeFlag(m_bChromaCbpTransition, "Chroma_CBP_transition");
        ruiChromaCbpBitCount ++;

        ((UvlcWriter *) m_pcSymbolWriter)->writeUvlc(m_uiChromaCbpRun - 1, "Chroma_CBP_run");
        ruiChromaCbpBitCount += UvlcCodeLength(m_uiChromaCbpRun - 1);

        m_uiChromaCbpRun = 0;

        m_bChromaCbpTransition = gaabTransitionFlag[m_uiLastChromaCbp][uiChromaCbp];
        m_uiLastChromaCbp   = uiChromaCbp;

        uiXChromaCbpNextMbX  = uiMbXIdx;
        uiXChromaCbpNextMbY  = uiMbYIdx;

        m_uiChromaCbpNextMbX = uiXChromaCbpNextMbX;
        m_uiChromaCbpNextMbY = uiXChromaCbpNextMbY;

        return Err::m_nOK;
      }
    }
  }

  uiMbAddress = m_pcSliceHeader->getFMO()->getNextMBNr(uiMbAddress );
  }

  if( m_uiChromaCbpRun > 0 )
  {
    ((UvlcWriter *) m_pcSymbolWriter)->writeFlag(m_bChromaCbpTransition, "Chroma_CBP_transition");
    ruiChromaCbpBitCount ++;

    ((UvlcWriter *) m_pcSymbolWriter)->writeUvlc(m_uiChromaCbpRun - 1, "Chromama_CBP_run");
    ruiChromaCbpBitCount += UvlcCodeLength(m_uiChromaCbpRun - 1);

    m_uiChromaCbpRun = 0;

    uiXChromaCbpNextMbX = uiXLastMbX;
    uiXChromaCbpNextMbY = uiXLastMbY;

    m_uiChromaCbpNextMbX = uiXChromaCbpNextMbX;
    m_uiChromaCbpNextMbY = uiXChromaCbpNextMbY;
  }

  return Err::m_nOK;
}


ErrVal
RQFGSEncoder::xEncodingFGS( Bool& rbFinished,
                            Bool& rbCorrupted,
                            UInt uiMaxBits,
                            UInt* puiPDFragBits,
                            UInt& ruiNumPDFrags,
                            UInt uiFracNb,
                            FILE* pFile ) //JVT-P031 (modified for fragments)
{
  Bool bRestore = true;
  Bool bFirstPass = true;
  UInt ui;
  UInt uiFragIdx;

  RNOK( m_cMbDataCtrlEL    .initSlice ( *m_pcSliceHeader, PRE_PROCESS, false, NULL ) );
  RNOK( m_pcCurrMbDataCtrl->initSlice ( *m_pcSliceHeader, PRE_PROCESS, false, NULL ) );
  m_pcSliceHeader->setSliceType( m_eSliceType );
  if(!uiFracNb)
  {
    RNOK( m_pcSymbolWriter   ->startSlice( *m_pcSliceHeader ) );
  }
  else
  {
    RNOK( m_pcSymbolWriter   ->startFragment() );
  }

  AOT( m_pcSymbolWriter == 0);
  m_pcSliceHeader->setSliceType( F_SLICE );

  rbCorrupted = false;
  Int iLastQP = m_pcSliceHeader->getPicQp();

  UInt  uiBitsLast = m_pcSymbolWriter->getNumberOfWrittenBits();
  Bool  bCheck     = ( uiMaxBits != 0 );
  //FIX_FRAG_CAVLC
  if( uiFracNb )
  {
    RNOK( m_pcSymbolWriter->setFirstBits(m_ucLastByte, m_uiLastBitPos));
  }
  //~FIX_FRAG_CAVLC
  //positions vector for luma (and chromaAC) and chroma DC

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
    UInt uiScanPos;
    UInt iStartCycle = 0, iCycle = 0;
    UInt iLastBitsLuma   = 0;
    UInt iLastBitsChroma = 0;
    UInt iBitsLuma       = 0;
    UInt iBitsChroma     = 0;
    UInt iLumaScanIdx     = 0;
    UInt iChromaDCScanIdx = 0;
    UInt iChromaACScanIdx = 1;
    UInt uiPass;
    UInt uiPassStart;

    m_uiFirstMbInSlice  = m_pcSliceHeader->getFirstMbInSlice();
    UInt uiLastMbInSlice  = m_pcSliceHeader->getLastMbInSlice();

    //--ICU/ETRI FMO Implementation : start
    UInt uiFirstMbY = (UInt) ( m_uiFirstMbInSlice / m_uiWidthInMB );
    UInt uiFirstMbX = m_uiFirstMbInSlice % m_uiWidthInMB;
    uiLastMbY  = (UInt) ( ( uiLastMbInSlice+1) / m_uiWidthInMB );
    uiLastMbX  = ( uiLastMbInSlice+1) % m_uiWidthInMB;


      if(!uiFracNb) //FIX_FRAG_CAVLC
      {
      // Pre-scan frame to find VLC positions
      UInt  auiEobShift[16];
      UInt  auiHighMagHist[16];
      UInt  auiBestCodeTabMap[16];
      UInt* pauiHistLuma    = new UInt[16*16*16];
      UInt* pauiHistChroma  = new UInt[16*16*16];

      ::memset(pauiHistLuma  , 0x00, 16*16*16*sizeof(UInt));
      ::memset(pauiHistChroma, 0x00, 16*16*16*sizeof(UInt));
      ::memset(auiHighMagHist, 0x00,       16*sizeof(UInt));

      for(UInt uiMbAddress=m_uiFirstMbInSlice ;uiMbAddress<=uiLastMbInSlice ;)
	  {
        UInt uiMbYIdx = uiMbAddress / m_uiWidthInMB;
        UInt uiMbXIdx = uiMbAddress % m_uiWidthInMB;
        MbDataAccess *pcMbDataAccessCurr = NULL;
        RNOK( m_cMbDataCtrlEL    .initMb( pcMbDataAccessCurr, uiMbYIdx, uiMbXIdx ) );
        const Bool bFrame = (FRAME == pcMbDataAccessCurr->getMbPicType());

      {
        //===== LUMA =====
        for( UInt uiB8YIdx = 2 * uiMbYIdx; uiB8YIdx < 2 * uiMbYIdx + 2; uiB8YIdx++ )
        for( UInt uiB8XIdx = 2 * uiMbXIdx; uiB8XIdx < 2 * uiMbXIdx + 2; uiB8XIdx++ )
        {
          for( UInt uiBlockYIdx = 2 * uiB8YIdx; uiBlockYIdx < 2 * uiB8YIdx + 2; uiBlockYIdx++ )
          for( UInt uiBlockXIdx = 2 * uiB8XIdx; uiBlockXIdx < 2 * uiB8XIdx + 2; uiBlockXIdx++ )
          {
          RNOK( xVLCParseLuma( uiBlockYIdx, uiBlockXIdx, pauiHistLuma, auiHighMagHist, bFrame ) );
          } // 4x4 block iteration
        } // 8x8 block iteration
        // ===== CHROMA AC =====
        for( UInt uiPlane = 0; uiPlane  < 2; uiPlane ++ )
        for( UInt uiB8YIdx = 2 * uiMbYIdx; uiB8YIdx < 2 * uiMbYIdx + 2; uiB8YIdx++ )
        for( UInt uiB8XIdx = 2 * uiMbXIdx; uiB8XIdx < 2 * uiMbXIdx + 2; uiB8XIdx++ ) {
          RNOK( xVLCParseChromaAC( uiPlane, uiB8YIdx, uiB8XIdx, pauiHistChroma, bFrame ) );
        } 
      } // macroblock iteration

		//--ICU/ETRI FMO Implementation
        uiMbAddress = m_pcSliceHeader->getFMO()->getNextMBNr(uiMbAddress );
	  }
      
      RNOK( m_cMbDataCtrlEL    .initSlice ( *m_pcSliceHeader, PRE_PROCESS, false, NULL ) );
      RNOK( m_pcCurrMbDataCtrl->initSlice ( *m_pcSliceHeader, PRE_PROCESS, false, NULL ) );

      // Tally over base pos to find EOB shift
      for (uiScanPos=0; uiScanPos<16; uiScanPos++)
      {
        auiEobShift[uiScanPos] = 0;
        for (UInt uiHpos=1; uiHpos<16; uiHpos++) {
          UInt uiTotal = 0;
          for (UInt uiBase=0; uiBase<16; uiBase++)
            uiTotal += pauiHistLuma[(uiScanPos*16+uiBase)*16+uiHpos];
          if (uiTotal < auiHighMagHist[uiScanPos])
            break;
          if (uiScanPos > 0 && auiEobShift[uiScanPos] == auiEobShift[uiScanPos-1])
            break;
          auiEobShift[uiScanPos]++;
        }
      }
      // 'Outlier' removal
      if (auiEobShift[0] > auiEobShift[1] + 8)
        auiEobShift[0] = 15;

      RNOK( m_pcSymbolWriter->RQencodeEobOffsets_Luma( auiEobShift ) );

        UInt uiTempCodeLenCycleBase[][16] = {
        {1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16}, // unary          0
        {2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9}, // flat 2         1
        {2, 2, 2, 4, 4, 4, 6, 6, 6, 8, 8, 8,10,10,10,11}, // flat 3         2
        {1, 3, 3, 3, 5, 5, 5, 7, 7, 7, 9, 9, 9,11,11,11}, // S3 cutoff 0    3
        {1, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8,9, 9, 9}}; //    
        UInt *pauiHistLumaNoBase = new UInt[16*16];
        UInt uiTotalNumCodeTable = 5;
        UInt uiTotalRunCost = 0;
        UInt uiCycle=0;

        // tally histogram without LEBL index
        memset(pauiHistLumaNoBase, 0, sizeof(UInt)*16*16);
        for(uiCycle = 0; uiCycle < 16; uiCycle++)
        {
          for(ui = 0; ui < 16; ui++)
            for(UInt uiBase = 0; uiBase < 16; uiBase++)
              pauiHistLumaNoBase[uiCycle*16+ui] += pauiHistLuma[(uiCycle*16+uiBase)*16+ui];
            }

          uiCycle = 0;
          UInt uiBestTable; 
          UInt uiCost[5], uiBestCost;
          UInt uiTableIdx;

          // find the cost of using each table 
          for(uiTableIdx = 0; uiTableIdx < uiTotalNumCodeTable; uiTableIdx++)
          {
            UInt *uiTable = uiTempCodeLenCycleBase[uiTableIdx];

            uiCost[uiTableIdx] = 0;
            for(ui = 1; ui < 16; ui++)
            {
              UInt uiCodeLen = uiTable[ui-1];

              uiCost[uiTableIdx] += pauiHistLumaNoBase[ui]*uiCodeLen;
            }
          }

          // find the best table with lowest cost 
          uiBestTable = 0;
          uiBestCost = uiCost[0];
          for(uiTableIdx = 1; uiTableIdx < uiTotalNumCodeTable; uiTableIdx++)
          {
            if(uiCost[uiTableIdx] < uiBestCost)
            {
              uiBestTable = uiTableIdx;
              uiBestCost = uiCost[uiTableIdx];
            }
          }

          auiBestCodeTabMap[uiCycle] = uiBestTable;
          uiTotalRunCost += uiBestCost;

        for(uiCycle = 1; uiCycle < 16; uiCycle++)
        {
          // find the cost of using each table 
          for( uiTableIdx = 0; uiTableIdx < uiTotalNumCodeTable; uiTableIdx++)
          {
            UInt *uiTable = uiTempCodeLenCycleBase[uiTableIdx];

            uiCost[uiTableIdx] = 0;
            for(ui = 0; ui < 16; ui++)
            {
              UInt uiSymbol; 
              if(ui == 0)
              {
                uiSymbol = auiEobShift[uiCycle];
              }
              else
              {
                uiSymbol = (ui <= auiEobShift[uiCycle])? ui-1:ui;
              }
              UInt uiCodeLen = uiTable[uiSymbol];
              uiCost[uiTableIdx] += pauiHistLumaNoBase[(uiCycle*16)+ui]*uiCodeLen;
            }
          }

          // find the best table with lowest cost 
          uiBestTable = 0;
          uiBestCost = uiCost[0];
          for(uiTableIdx = 1; uiTableIdx < uiTotalNumCodeTable; uiTableIdx++)
          {
            if(uiCost[uiTableIdx] < uiBestCost)
            {
              uiBestTable = uiTableIdx;
              uiBestCost = uiCost[uiTableIdx];
            }
          }

          auiBestCodeTabMap[uiCycle] = uiBestTable;
          uiTotalRunCost += uiBestCost;
        }
        delete pauiHistLumaNoBase;

      // Tally over base pos to find EOB shift
    for (uiScanPos=0; uiScanPos<16; uiScanPos++)
    {
        auiEobShift[uiScanPos] = 0;
        UInt uiEobTotal = 0;
        for (UInt uiBase=0; uiBase<16; uiBase++)
          uiEobTotal += pauiHistChroma[(uiScanPos*16+uiBase)*16];
      for (UInt uiHpos=1; uiHpos<16; uiHpos++)
      {
          UInt uiTotal = 0;
          for (int uiBase=0; uiBase<16; uiBase++)
            uiTotal += pauiHistChroma[(uiScanPos*16+uiBase)*16+uiHpos];
          if (uiTotal < uiEobTotal)
            break;
          if (uiScanPos > 0 && auiEobShift[uiScanPos] == auiEobShift[uiScanPos-1])
            break;
          auiEobShift[uiScanPos]++;
        }
      }
      // 'Outlier' removal
      if (auiEobShift[0] > auiEobShift[1] + 8)
        auiEobShift[0] = 15;

      RNOK( m_pcSymbolWriter->RQencodeEobOffsets_Chroma( auiEobShift ) );
      RNOK( m_pcSymbolWriter->RQencodeBestCodeTableMap( auiBestCodeTabMap, 16 ) );
      delete pauiHistLuma;
      delete pauiHistChroma;
      } //FIX_FRAG_CAVLC

    uiLumaCbpBitCount    = 0;
    uiLumaCbpNextMbX     = uiFirstMbX;
    uiLumaCbpNextMbY     = uiFirstMbY;
    uiLumaCbpNext8x8Idx  = 0;
    uiChromaCbpBitCount  = 0;
    uiChromaCbpNextMbX   = uiFirstMbX;
    uiChromaCbpNextMbY   = uiFirstMbY;

      if( ! m_pcSliceHeader->getPPS().getEntropyCodingModeFlag() && !uiFracNb)
      {
      xEncodeLumaCbpVlcStart( uiLumaCbpNextMbX, uiLumaCbpNextMbY, uiLumaCbpNext8x8Idx,
          uiLastMbX, uiLastMbY, uiLumaCbpBitCount);

        xEncodeChromaCbpVlcStart(uiFirstMbX, uiFirstMbY, uiChromaCbpBitCount);
      }

    ruiNumPDFrags     = 0;
    puiPDFragBits[0]  = 0;

    // chroma cycle frequency. 
    // hard-coded for now, should this be moved to the slice header
    iStartCycle = ( m_eSliceType == B_SLICE ) ? 2 : 3;
    if( !uiFracNb )
    {
    m_pcSymbolWriter->RQencodeCycleSymbol( iStartCycle-1 );
    }

    UInt  uiNumFrags;
    Int   aiMaxPosLuma[16], aiMaxPosChromaAC[16], aiMaxPosChromaDC[16];

    uiFragIdx = 0;
    uiNumFrags = xDeriveComponentPosVectors( m_auiScanPosVectLuma, 
      aiMaxPosLuma, aiMaxPosChromaAC, aiMaxPosChromaDC, iStartCycle );

    while (iLumaScanIdx < 16 || iChromaDCScanIdx < 4 || iChromaACScanIdx < 16)
    {
      UInt bAllowChromaDC, bAllowChromaAC;

      UInt uiMaxPosLuma;
      UInt uiMaxPosChromaAC;
      UInt uiMaxPosChromaDC;

        if(!uiFracNb)
        {
          // start values for loops
          m_uiMbYIdx    = uiFirstMbY;
          m_uiMbXIdx    = ( m_uiMbYIdx == uiFirstMbY ? uiFirstMbX : 0 );
        m_c8x8Idx     = B8x8Idx();
        m_cIdx        = S4x4Idx();
          m_uiFGSPart   = 0;
          uiPassStart   = 0;
        }
        else
        {
          if(bRestore)
          {
            bCheck = false;
            // restore status
            xRestoreFGSState(iLumaScanIdx, iChromaDCScanIdx, iChromaACScanIdx, iStartCycle,
                            iCycle, uiPassStart, uiFragIdx, 
                            iLastBitsLuma,
                            iLastQP);

            // restore the cbp-related coding states 
            uiLumaCbpNextMbX    = m_uiLumaCbpNextMbX;
            uiLumaCbpNextMbY    = m_uiLumaCbpNextMbY;
            uiLumaCbpNext8x8Idx = m_uiLumaCbpNext8x8Idx;
            uiChromaCbpNextMbX  = m_uiChromaCbpNextMbX;
            uiChromaCbpNextMbY  = m_uiChromaCbpNextMbY;
            bRestore = false;
          }
          else
          {
            uiPassStart = 0;
          }
        }
        uiPass = uiPassStart; 

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

      // flush the bitstream, and send the re-sync marker
      if( m_pcSliceHeader->getFGSCycleAlignedFragment() )
      {
        // any more fragment to follow?
        if( iLumaScanIdx > 0 )
        {
          // do not know why the reader can not have enough bits
          RNOK( m_pcSymbolWriter->RQencodeTermBit( 1 ) );
          RNOK( m_pcSymbolWriter->RQencodeTermBit( 1 ) );
          RNOK( m_pcSymbolWriter->finishSlice() );

          // byte-aligned, same as NalUnitEncoder::xWriteTrailingBits
          RNOK( m_pcSymbolWriter->getWriteBuffer()->write( 1 ) );
          RNOK( m_pcSymbolWriter->getWriteBuffer()->writeAlignZero() );

          // do not have to flush the buffer as more fragments will be coded
          // record the end of the segment
          puiPDFragBits[uiFragIdx] = m_pcSymbolWriter->getWriteBuffer()->getNumberOfWrittenBits();

          m_pcSliceHeader->setSliceType( m_eSliceType );
          m_pcSymbolWriter->RQreset( *m_pcSliceHeader );
          m_pcSliceHeader->setSliceType( F_SLICE );
        }
      }

		    for(UInt uiMbAddress=m_uiFirstMbInSlice ;uiMbAddress<=uiLastMbInSlice;)
		    {
        const UInt uiMbYIdx = uiMbAddress / m_uiWidthInMB;
        const UInt uiMbXIdx = uiMbAddress % m_uiWidthInMB;
			    
			     MbDataAccess *pcMbDataAccess = NULL;
          RNOK( m_pcCurrMbDataCtrl->initMb( pcMbDataAccess, uiMbYIdx, uiMbXIdx ) );
          const Bool bFrame = (FRAME == pcMbDataAccess->getMbPicType());

			    {
            if( ! m_pcSliceHeader->getPPS().getEntropyCodingModeFlag() )
            {
              Bool bFirstPassPrescan = bFirstPass;
              UInt uiB8YInit, uiB8XInit, uiBlockYInit, uiBlockXInit;
            uiB8YInit = m_uiMbYIdx*2 + m_c8x8Idx.y()/2;
            uiB8XInit = m_uiMbXIdx*2 + m_c8x8Idx.x()/2;
              uiBlockYInit = 2*uiB8YInit;
              uiBlockXInit = 2*uiB8XInit;
            if(!bFirstPassPrescan)
            {
                uiB8YInit = 2*uiMbYIdx;
                uiB8XInit = 2*uiMbXIdx;
              }

              /* refinement coefficients pre-scan */
              //===== LUMA =====
            if( (m_uiFGSPart == 0) || (!bFirstPassPrescan) )
            {
              for( UInt uiB8YIdx = uiB8YInit; uiB8YIdx < 2 * uiMbYIdx + 2; uiB8YIdx++ )
              {
                for( UInt uiB8XIdx = uiB8XInit; uiB8XIdx < 2 * uiMbXIdx + 2; uiB8XIdx++ )
                {
                  if(!bFirstPassPrescan)
                  {
                      uiBlockYInit = 2*uiB8YIdx;
                      uiBlockXInit = 2*uiB8XIdx;
                  }
                  else
                  {
                      bFirstPassPrescan = false;
                    }
                  for( UInt uiBlockYIdx = uiBlockYInit; uiBlockYIdx < 2 * uiB8YIdx + 2; uiBlockYIdx++ )
                  {
                    for( UInt uiBlockXIdx = uiBlockXInit; uiBlockXIdx < 2 * uiB8XIdx + 2; uiBlockXIdx++ )
                    {
                      for( ui=iLumaScanIdx; ui<=uiMaxPosLuma; ui++ )
                      {
                        if( ui < 16 )
                        {
                          RNOK( xPrescanCoefficientLumaRef( uiBlockYIdx, uiBlockXIdx, ui, bFrame ) );
                          }
                        }
                      } // 4x4 block iteration
                      uiBlockXInit = 2 * uiB8XIdx;
                    }
                  } // 8x8 block iteration
                  uiB8XInit = 2 * uiMbXIdx;
                }
              m_cCPlaneIdx = CPlaneIdx( 0 );
                uiB8YInit = 2 * uiMbYIdx;
                uiB8XInit = 2 * uiMbXIdx;
              }

              // ===== CHROMA DC =====
            if( (m_uiFGSPart == 1) || (!bFirstPassPrescan) )
            {
              if( bAllowChromaDC && ( m_bFgsComponentSep == 0 || iLumaScanIdx == 16 ) )
              {
                  bFirstPassPrescan = false;
                for( UInt uiPlane = m_cCPlaneIdx; uiPlane < 2; uiPlane ++ )
                {
                  for( ui=iChromaDCScanIdx; ui<=uiMaxPosChromaDC && ui<4; ui++ )
                  {
                    CoefMap cCoefMap = m_pcCoefMap[uiMbYIdx * m_uiWidthInMB + uiMbXIdx].getCoefMap( CIdx( CPlaneIdx(uiPlane) ) + ui )[0];
                    if( (cCoefMap & SIGNIFICANT) && !(cCoefMap & CODED) )
                        RNOK( xPrescanCoefficientChromaDCRef( uiPlane, uiMbYIdx, uiMbXIdx, ui ) );
                    }
                  } // for
                m_cCPlaneIdx = CPlaneIdx( 0 );
                } // if
              }
              m_uiFGSPart = 2;

              // ===== CHROMA AC =====
            if( (m_uiFGSPart == 2) || (!bFirstPassPrescan) )
            {
              if( bAllowChromaAC && ( m_bFgsComponentSep == 0 || iLumaScanIdx == 16 ) )
              {
                  bFirstPassPrescan = false;
                for( CPlaneIdx cCPlaneIdx; cCPlaneIdx.isLegal(); ++cCPlaneIdx )
                {
                  for( UInt uiB8YIdx = uiB8YInit; uiB8YIdx < 2 * uiMbYIdx + 2; uiB8YIdx++ )
                  {
                    for( UInt uiB8XIdx = uiB8XInit; uiB8XIdx < 2 * uiMbXIdx + 2; uiB8XIdx++ )
                    {
                      CoefMap *pcCoefMap = m_pcCoefMap[uiMbYIdx * m_uiWidthInMB + uiMbXIdx].getCoefMap( CIdx( cCPlaneIdx ) + ( ( (uiB8YIdx&1)<<1 ) + (uiB8XIdx&1) ) );
                      for( ui=iChromaACScanIdx; ui<=uiMaxPosChromaAC && ui<16; ui++ )
                      {
                        if( (pcCoefMap[ui] & SIGNIFICANT) && !(pcCoefMap[ui] & CODED) )
                          RNOK( xPrescanCoefficientChromaACRef( cCPlaneIdx, uiB8YIdx, uiB8XIdx, ui ) );
                        }
                      } // for
                      uiB8XInit = 2 * uiMbXIdx;
                    }
                    uiB8YInit = 2 * uiMbYIdx;
                  }
                m_cCPlaneIdx = CPlaneIdx( 0 );
                } // if
              }
              m_uiFGSPart = 0;
            }

          MbFGSCoefMap &rcMbFGSCoefMap = m_pcCoefMap[uiMbAddress];
          MbDataAccess *pcMbDataAccessEL  = NULL, *pcMbDataAccessBL = NULL;
          RNOK( m_cMbDataCtrlEL     .initMb( pcMbDataAccessEL, uiMbYIdx, uiMbXIdx ) );
          RNOK( m_pcCurrMbDataCtrl ->initMb( pcMbDataAccessBL, uiMbYIdx, uiMbXIdx ) );

          if( iLumaScanIdx == 0 && rcMbFGSCoefMap.getNumCoded() == 0 ) {
            xEncodeMbHeader( pcMbDataAccessBL, pcMbDataAccessEL, rcMbFGSCoefMap, iLastQP );
          }

          if(!bFirstPass)
          {
            m_uiMbYIdx = uiMbYIdx;
            m_uiMbXIdx = uiMbXIdx;
            m_c8x8Idx = B8x8Idx();
          }
          //===== LUMA =====
          if( (m_uiFGSPart == 0) || (!bFirstPass) )
          {
            for( B8x8Idx c8x8Idx = m_c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
            {
              if(!bFirstPass)
                m_cIdx = S4x4Idx( c8x8Idx );
              else
                bFirstPass = false;
              S4x4Idx cIdx2 = S4x4Idx( c8x8Idx ); 
              Bool b8x8 = (rcMbFGSCoefMap.getLumaScanPos( cIdx2+1 ) == 64); 
                //pcMbDataAccessBL->getMbData().isTransformSize8x8(); 
              iLastBitsLuma = m_pcSymbolWriter->getNumberOfWrittenBits();
              if( b8x8 ) {
                for( ui = iLumaScanIdx; ui <= uiMaxPosLuma; ui ++ ) {
                  for( S4x4Idx cIdx = m_cIdx; cIdx.isLegal( c8x8Idx ); cIdx++ ) {
                    iLastBitsLuma = m_pcSymbolWriter->getNumberOfWrittenBits();
                    UInt uiOffset = ( cIdx.y() % 2 ) * 2 + ( cIdx.x() % 2);
                    if( rcMbFGSCoefMap.getLumaScanPos( cIdx2 ) == ui * 4 + uiOffset )
                      RNOK( xEncodeNewCoefficientLuma( pcMbDataAccessBL, pcMbDataAccessEL, rcMbFGSCoefMap, cIdx, bFrame ) );
                    RNOK( xEncodeCoefficientLumaRef( pcMbDataAccessBL, pcMbDataAccessEL, rcMbFGSCoefMap, cIdx, ui ) );
                    if( bCheck && m_pcSymbolWriter->getNumberOfWrittenBits() - uiBitsLast >= uiMaxBits )
                    {
                      xStoreFGSState(iLumaScanIdx, iChromaDCScanIdx, iChromaACScanIdx, iStartCycle,
                                      iCycle, uiPass, uiFragIdx, uiMbYIdx, uiMbXIdx,
                                      c8x8Idx, cIdx/*cIdx replaced uiBlockYIdx, uiBlockXIdx+1 - this is not the same but how should it be?*/, iLastBitsLuma, uiBitsLast, 0, 0, iLastQP);                      throw WriteStop();
                    }
                  }
                }
              }
              else {
                  for( S4x4Idx cIdx = m_cIdx; cIdx.isLegal( c8x8Idx ); cIdx++ ) {
                    iLastBitsLuma = m_pcSymbolWriter->getNumberOfWrittenBits();
                    for( ui=iLumaScanIdx; ui<=uiMaxPosLuma; ui++ ) {
                      if( rcMbFGSCoefMap.getLumaScanPos( cIdx )  <= ui )
                        RNOK( xEncodeNewCoefficientLuma( pcMbDataAccessBL, pcMbDataAccessEL, rcMbFGSCoefMap, cIdx, bFrame ) );
                      RNOK( xEncodeCoefficientLumaRef( pcMbDataAccessBL, pcMbDataAccessEL, rcMbFGSCoefMap, cIdx, ui ) );
                    }
                    if( bCheck && m_pcSymbolWriter->getNumberOfWrittenBits() - uiBitsLast >= uiMaxBits )
                    {
                      xStoreFGSState(iLumaScanIdx, iChromaDCScanIdx, iChromaACScanIdx, iStartCycle,
                                      iCycle, uiPass, uiFragIdx, uiMbYIdx, uiMbXIdx,
                                      c8x8Idx, cIdx/*cIdx replaced uiBlockYIdx, uiBlockXIdx+1 - this is not the same but how should it be?*/, iLastBitsLuma, uiBitsLast, 0, 0, iLastQP);
                      throw WriteStop();
                    }
                  }
              }

              if (iCycle == 0)
                iBitsLuma += m_pcSymbolWriter->getNumberOfWrittenBits() - iLastBitsLuma;
            }
            m_cCPlaneIdx = CPlaneIdx( 0 );
            m_c8x8Idx = B8x8Idx();
          }
          // ===== CHROMA DC =====
          if( (m_uiFGSPart == 1) || (!bFirstPass) )
          {
            if( bAllowChromaDC && ( m_bFgsComponentSep == 0 || iLumaScanIdx == 16 ) )
            {
              bFirstPass = false;
              for( CPlaneIdx cCPlaneIdx = m_cCPlaneIdx; cCPlaneIdx.isLegal(); ++cCPlaneIdx )
              {
                iLastBitsChroma = m_pcSymbolWriter->getNumberOfWrittenBits();
                for( ui=iChromaDCScanIdx; ui<=uiMaxPosChromaDC && ui<4; ui++ )
                {
                  if( ui == 0 || ui == rcMbFGSCoefMap.getChromaDCScanPos( cCPlaneIdx ) )
                    RNOK( xEncodeNewCoefficientChromaDC( pcMbDataAccessBL, pcMbDataAccessEL, rcMbFGSCoefMap, cCPlaneIdx, iLastQP, ui ) );
                  if (iCycle == 0)
                    iBitsChroma += m_pcSymbolWriter->getNumberOfWrittenBits() - iLastBitsChroma;
                  CoefMap cCoefMap = rcMbFGSCoefMap.getCoefMap( CIdx( cCPlaneIdx ) + ui )[0];
                  if( (cCoefMap & SIGNIFICANT) && !(cCoefMap & CODED) )
                    RNOK( xEncodeCoefficientChromaDCRef( pcMbDataAccessBL, pcMbDataAccessEL, rcMbFGSCoefMap, cCPlaneIdx, ui ) );
                }
                if( bCheck && m_pcSymbolWriter->getNumberOfWrittenBits() - uiBitsLast >= uiMaxBits )
                {
                  xStoreFGSState(iLumaScanIdx, iChromaDCScanIdx, iChromaACScanIdx, iStartCycle,
                                  iCycle, uiPass, 
                                  uiFragIdx,
                                  uiMbYIdx, uiMbXIdx, B8x8Idx(),S4x4Idx(), iLastBitsLuma, uiBitsLast, 1, cCPlaneIdx/* removed: +1 */, iLastQP);
                  throw WriteStop();
                }
              } // for
              m_cCPlaneIdx = CPlaneIdx( 0 );
            } // if
          }
          m_uiFGSPart = 2;

              // ===== CHROMA AC =====
              if( (m_uiFGSPart == 2) || (!bFirstPass) )
              {
            if( bAllowChromaAC && ( m_bFgsComponentSep == 0 || iLumaScanIdx == 16 ) )
            {
                  bFirstPass = false;
              for( CIdx cCIdx( m_cCPlaneIdx, m_c8x8Idx.b8x8Index() ); cCIdx.isLegal(); cCIdx++ )
                  {
                CoefMap *pcCoefMap = rcMbFGSCoefMap.getCoefMap( cCIdx );
                      for( ui=iChromaACScanIdx; ui<=uiMaxPosChromaAC && ui<16; ui++ )
                      {
                  if( ui == rcMbFGSCoefMap.getChromaACScanPos( cCIdx ) )
                    RNOK( xEncodeNewCoefficientChromaAC( pcMbDataAccessBL, pcMbDataAccessEL, rcMbFGSCoefMap, cCIdx, iLastQP, ui, bFrame ) );
                  if( (pcCoefMap[ui] & SIGNIFICANT) && !(pcCoefMap[ui] & CODED) )
                    RNOK( xEncodeCoefficientChromaACRef( pcMbDataAccessBL, pcMbDataAccessEL, rcMbFGSCoefMap, cCIdx, ui ) );
                        if( bCheck && m_pcSymbolWriter->getNumberOfWrittenBits() - uiBitsLast >= uiMaxBits )
                        {
                    xStoreFGSState(iLumaScanIdx, iChromaDCScanIdx, iChromaACScanIdx, iStartCycle,
                      iCycle, uiPass, 
                      uiFragIdx,
                      uiMbYIdx, uiMbXIdx, B8x8Idx( Par8x8(cCIdx&3) ),/*  uiB8YIdx, uiB8XIdx+1*/ S4x4Idx(), iLastBitsLuma, uiBitsLast, 2, CPlaneIdx( cCIdx.plane() ), iLastQP);
                          throw WriteStop();
                        }
                      }
                  }
              m_cCPlaneIdx = CPlaneIdx();
                } // if
              }
              m_uiFGSPart = 0;
              RNOK( m_pcSymbolWriter->RQupdateVlcTable() );

            } // macroblock iteration

		  //--ICU/ETRI FMO Implementation
		  uiMbAddress = m_pcSliceHeader->getFMO()->getNextMBNr(uiMbAddress );
		  }	

          RNOK( m_pcSymbolWriter->RQvlcFlush() );

          m_uiMbYIdx = uiFirstMbY;
          m_uiMbXIdx = ( m_uiMbYIdx == uiFirstMbY ? uiFirstMbX : 0 );
      m_c8x8Idx = B8x8Idx();
      m_cIdx    = S4x4Idx();

        iLumaScanIdx = min(uiMaxPosLuma+1, 16);
        if( m_bFgsComponentSep && iLumaScanIdx == 16 )
        {
          RNOK( m_pcSymbolWriter->RQcompSepAlign() );
        }
        if (bAllowChromaDC)
          iChromaDCScanIdx = min(uiMaxPosChromaDC+1, 4);
        if (bAllowChromaAC)
          iChromaACScanIdx = min(uiMaxPosChromaAC+1, 16);

        uiFragIdx ++;
        iCycle++;

      } // while

    UInt uiBitsPath = m_pcSymbolWriter->getNumberOfWrittenBits() - uiBitsLast;
    if( pFile )
    {
      fprintf( pFile, "\t%d", uiBitsPath );
    }

    RNOK( m_pcSymbolWriter->RQencodeTermBit( 1 ) );
  }
  catch( WriteStop )
  {
    rbCorrupted = true;
    // if it is fragmented, escape before saying is is finished
    if( m_pcSliceHeader->getFragmentedFlag() && !m_pcSliceHeader->getLastFragmentFlag() ) 
    {
        xSaveCodingPath();
        RNOK( xUpdateCodingPath() );
        //FIX_FRAG_CAVLC
        //save last written byte
        RNOK(m_pcSymbolWriter->getLastByte(m_ucLastByte, m_uiLastBitPos));
        if( !m_pcSliceHeader->getPPS().getEntropyCodingModeFlag() )
        {
          RNOK(((UvlcWriter *)m_pcSymbolWriter)->RQcountFragmentedSymbols());
        }
        return Err::m_nOK;
    }
  }

  if( ! m_pcSliceHeader->getFGSCycleAlignedFragment() )
    ruiNumPDFrags = 1;
  else
    ruiNumPDFrags = uiFragIdx;

  rbFinished      = rbCorrupted || ( m_iRemainingTCoeff == 0 );

  RNOK( m_pcSymbolWriter->finishSlice() );

  RNOK( xUpdateCodingPath(m_pcSliceHeader) );
  RNOK( xClearCodingPath() );

  return Err::m_nOK;
}


ErrVal
RQFGSEncoder::xEncodeMbHeader( MbDataAccess*      pcMbDataAccessBL,
                               MbDataAccess*      pcMbDataAccessEL,
                               MbFGSCoefMap       &rcMbFGSCoefMap,
                               Int&               riLastQp )
{
  UInt    uiMbX = pcMbDataAccessBL->getMbX();
  UInt    uiMbY = pcMbDataAccessBL->getMbY();

  if( m_pcSliceHeader->getAdaptivePredictionFlag() &&
    ! pcMbDataAccessBL->getMbData().isIntra() )
  {
    m_pcSliceHeader  ->setSliceType( m_eSliceType );
    RNOK( m_pcMbCoder->encodeMotion( *pcMbDataAccessEL, pcMbDataAccessBL ) );
    m_pcSliceHeader  ->setSliceType( F_SLICE );
  }


  //===== CBP =====
  if( ! m_pcSliceHeader->getPPS().getEntropyCodingModeFlag() ) {
    xEncodeLumaCbpVlc   (uiMbX, uiMbY, uiLumaCbpNextMbX, uiLumaCbpNextMbY, uiLumaCbpNext8x8Idx, uiLastMbX, uiLastMbY, uiLumaCbpBitCount);
    xEncodeChromaCbpVlc (uiMbX, uiMbY, uiChromaCbpNextMbX, uiChromaCbpNextMbY, uiLastMbX, uiLastMbY, uiChromaCbpBitCount);

    // restore pcMbDataAccessBL and pcMbDataAccessEL
    RNOK( m_pcCurrMbDataCtrl ->initMb( pcMbDataAccessBL, uiMbY, uiMbX ) );
    RNOK( m_cMbDataCtrlEL     .initMb( pcMbDataAccessEL, uiMbY, uiMbX ) );
  }


  // Luma CBP in CABAC, need also for CAVLC to update the CBP in buffer
  {
    for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx ++ ) {
      m_pcSymbolWriter->RQencodeCBP_8x8( *pcMbDataAccessEL, *pcMbDataAccessBL, c8x8Idx );
    }
  }

  // CHROMA CBP in CABAC, need also for CAVLC to update the CBP in buffer
  if( m_pcSymbolWriter->RQencodeCBP_Chroma( *pcMbDataAccessEL, *pcMbDataAccessBL ) )
    m_pcSymbolWriter->RQencodeCBP_ChromaAC( *pcMbDataAccessEL, *pcMbDataAccessBL );

  // transform size
  if( ( pcMbDataAccessBL->getMbData().getMbCbp() & 15 ) &&
    ! ( rcMbFGSCoefMap.getMbMap() & TRANSFORM_SPECIFIED ) ) {
    RNOK( m_pcSymbolWriter->RQencode8x8Flag( *pcMbDataAccessEL, *pcMbDataAccessBL ) );
    rcMbFGSCoefMap.getMbMap() |= TRANSFORM_SPECIFIED;
  }

  // delta QP and transform flag
  if( pcMbDataAccessBL->getMbData().getMbCbp() != 0 ) {
    if( ! ( rcMbFGSCoefMap.getMbMap() & SIGNIFICANT ) ) {
      pcMbDataAccessEL->setLastQp( riLastQp );
      RNOK( m_pcSymbolWriter->RQencodeDeltaQp( *pcMbDataAccessEL ) );
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
          m_pcSymbolWriter->RQencodeBCBP_4x4( *pcMbDataAccessEL, *pcMbDataAccessBL, b8x8, cIdx );
      }
      else
      {
        S4x4Idx cIdx (c8x8Idx);

        rcMbFGSCoefMap.getLumaScanPos( cIdx+1 ) = 64;   
      }
    }
  }

  xUpdateMbMaps( pcMbDataAccessBL, pcMbDataAccessEL, rcMbFGSCoefMap, & m_iRemainingTCoeff );

  return Err::m_nOK;
}

ErrVal
RQFGSEncoder::xEncodeNewCoefficientLuma( MbDataAccess  *pcMbDataAccessBL,
                                         MbDataAccess  *pcMbDataAccessEL,
                                         MbFGSCoefMap  &rcMbFGSCoefMap,
                                         const S4x4Idx &rcIdx,
                                         Bool    bFrame  )
{
  B8x8Idx c8x8Idx( rcIdx.getContainingPar8x8() );
  Bool    b8x8 = pcMbDataAccessBL->getMbData().isTransformSize8x8();
  if( b8x8 && m_pcSliceHeader->getPPS().getEntropyCodingModeFlag() )
  {
    UInt    uiStop          = 64;
    UInt    ui8x8ScanIndex  = uiStop;
    UInt    ui8x8StartIndex = uiStop;
    UInt    ui8x8Index;

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
    
    Bool bNeedEob = ( ui8x8ScanIndex > ui8x8StartIndex );

    while( ui8x8ScanIndex < 64 )
    {
      UInt uiNumCoefWritten;

      RNOK( m_pcSymbolWriter->RQencodeNewTCoeff_8x8( *pcMbDataAccessEL, *pcMbDataAccessBL,
                                                    c8x8Idx, ui8x8ScanIndex, bNeedEob, uiNumCoefWritten ) );

      if( bNeedEob )
      {
        //===== end of block =====
        rcMbFGSCoefMap.getLumaScanPos( cIdx2 ) = 64;

        for( UInt ui8x8 = ui8x8ScanIndex; ui8x8 < 64; ui8x8++ )
        {
          if( !(pcCoefMap [ ui8x8 ] & SIGNIFICANT) )
          {
            pcCoefMap[ ui8x8 ]  |= CODED;
            m_iRemainingTCoeff--;
            RNOK( rcMbFGSCoefMap.increaseAndCheckNumCoded( 1 ) );
          }
        }
        break;
      }
      else
      {
        //===== coefficient =====
        bNeedEob = false;

        const UChar*  pucScan64 = (bFrame) ? g_aucFrameScan64 : g_aucFieldScan64;

        pcCoefMap[ ui8x8ScanIndex ] |= CODED;
        RNOK( rcMbFGSCoefMap.increaseAndCheckNumCoded( 1 ) );
        m_iRemainingTCoeff--;
        if( pcMbDataAccessEL->getMbTCoeffs().get8x8( c8x8Idx )[pucScan64[ui8x8ScanIndex]] )
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
      } // end bNeedEob
    }
    if( rcMbFGSCoefMap.getLumaScanPos( cIdx2 ) < ui8x8ScanIndex )
      rcMbFGSCoefMap.getLumaScanPos( cIdx2 ) = ui8x8ScanIndex;
    while( ui8x8ScanIndex < 64 && ( pcCoefMap[ ui8x8ScanIndex ] & SIGNIFICANT ) )
      rcMbFGSCoefMap.getLumaScanPos( cIdx2 ) = ++ui8x8ScanIndex;
    TWSOT( m_iRemainingTCoeff < 0 );
  }
  else
  {
    if (! m_pcSymbolWriter->RQpeekCbp4x4( *pcMbDataAccessEL, *pcMbDataAccessBL, b8x8, rcIdx ) )
    {
      return Err::m_nOK;
    }

    CoefMap* pcCoefMap = b8x8 ? rcMbFGSCoefMap.getCoefMap( c8x8Idx ) : rcMbFGSCoefMap.getCoefMap( rcIdx );
    UInt    uiStop = b8x8 ? 64:16;
    UInt    uiScanIndex = uiStop;
    UInt    uiStartIndex = uiStop;
    UInt    uiIndex = uiStop;

    if(b8x8) 
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
    UInt uiNumCoefWritten;


    RNOK( m_pcSymbolWriter->RQencodeNewTCoeff_Luma( *pcMbDataAccessEL, *pcMbDataAccessBL,
                                                  LUMA_SCAN, b8x8, rcIdx, uiScanIndex, bNeedEob, uiNumCoefWritten ) );

    if(b8x8)
    {
      UInt    uiOffset = (rcIdx.y() % 2) * 2 + (rcIdx.x() % 2); 
      UInt    ui8x8ScanIndex = uiOffset + uiScanIndex * 4; 
      const UChar*  pucScan64 = (bFrame) ? g_aucFrameScan64 : g_aucFieldScan64;

      for ( UInt ui8x8 = 0; ui8x8ScanIndex < 64 && ( ui8x8 < uiNumCoefWritten || bNeedEob ); ui8x8ScanIndex += 4 )
      {
        if( ! ( pcCoefMap[ui8x8ScanIndex] & SIGNIFICANT ) )
        {
          if( pcMbDataAccessEL->getMbTCoeffs().get8x8( c8x8Idx )[ pucScan64[ui8x8ScanIndex] ] )
            pcCoefMap[ui8x8ScanIndex] |= SIGNIFICANT | NEWSIG;

          pcCoefMap[ui8x8ScanIndex] |= CODED;
          m_iRemainingTCoeff--;
          RNOK( rcMbFGSCoefMap.increaseAndCheckNumCoded( 1 ) );
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
      for ( UInt ui = 0; uiScanIndex < 16 && ( ui < uiNumCoefWritten || bNeedEob ); uiScanIndex++ )
      {
        if( ! ( pcCoefMap[uiScanIndex] & SIGNIFICANT ) )
        {
          if( pcMbDataAccessEL->getMbTCoeffs().get( rcIdx )[pucScan[uiScanIndex]] )
            pcCoefMap[uiScanIndex] |= SIGNIFICANT | NEWSIG;
          pcCoefMap[uiScanIndex] |= CODED;
          m_iRemainingTCoeff--;
          RNOK( rcMbFGSCoefMap.increaseAndCheckNumCoded( 1 ) );
          ui++;
        }
      }
      rcMbFGSCoefMap.getLumaScanPos( rcIdx ) = uiScanIndex;
      while( uiScanIndex < 16 && ( pcCoefMap[uiScanIndex] & SIGNIFICANT ) )
        rcMbFGSCoefMap.getLumaScanPos( rcIdx ) = ++uiScanIndex;
    }
    TWSOT( m_iRemainingTCoeff < 0 );
  }

  return Err::m_nOK;
}



ErrVal
RQFGSEncoder::xEncodeNewCoefficientChromaDC ( MbDataAccess    *pcMbDataAccessBL,
                                              MbDataAccess    *pcMbDataAccessEL,
                                              MbFGSCoefMap    &rcMbFGSCoefMap,
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
    if( !(cCoefMap & SIGNIFICANT) || (cCoefMap & NEWSIG ) )
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
    Bool bSigBCBP = m_pcSymbolWriter->RQencodeBCBP_ChromaDC( *pcMbDataAccessEL, *pcMbDataAccessBL, CIdx( rcCPlaneIdx ) );
    rcMbFGSCoefMap.getChromaDCMbMap( rcCPlaneIdx ) |= CODED;
    if(  bSigBCBP )
      rcMbFGSCoefMap.getChromaDCMbMap( rcCPlaneIdx ) |= SIGNIFICANT;
    if( ! bSigBCBP )
    {
      rcMbFGSCoefMap.getChromaDCScanPos( rcCPlaneIdx ) = 4;
      for( CIdx cCIdx( rcCPlaneIdx ); cCIdx.isLegal( rcCPlaneIdx ); cCIdx++ )
      {
        CoefMap &rcCoefMap = rcMbFGSCoefMap.getCoefMap( cCIdx )[0];
        if( ! ( rcCoefMap & SIGNIFICANT ) )
        {
          rcCoefMap |= CODED;
          m_iRemainingTCoeff--;
          RNOK( rcMbFGSCoefMap.increaseAndCheckNumCoded( 1 ) );
        }
      }
      TWSOT( m_iRemainingTCoeff < 0 );
      return Err::m_nOK;
    }
  }


  UInt uiNumCoefWritten;
  RNOK( m_pcSymbolWriter->RQencodeNewTCoeff_Chroma( *pcMbDataAccessEL, *pcMbDataAccessBL,
    CHROMA_DC, CIdx( rcCPlaneIdx ), uiDCIdx, bNeedEob, uiNumCoefWritten ) );
  for ( UInt ui = 0; uiDCIdx < 4 && ( ui < uiNumCoefWritten || bNeedEob ); uiDCIdx++ )
  {
    CoefMap &rcCoefMap = rcMbFGSCoefMap.getCoefMap( CIdx( rcCPlaneIdx ) + uiDCIdx )[0];
    if( !(rcCoefMap & SIGNIFICANT ) )
      {
      if( pcMbDataAccessEL->getMbTCoeffs().get( CIdx( rcCPlaneIdx ) + uiDCIdx )[0] )
        rcCoefMap |= SIGNIFICANT | NEWSIG;
      rcCoefMap |= CODED;
      m_iRemainingTCoeff--;
      RNOK( rcMbFGSCoefMap.increaseAndCheckNumCoded( 1 ) );
      ui++;
    }
  }
  rcMbFGSCoefMap.getChromaDCScanPos( rcCPlaneIdx ) = uiDCIdx;
  while( ( rcMbFGSCoefMap.getCoefMap( CIdx( rcCPlaneIdx ) + uiDCIdx )[0] & SIGNIFICANT ) && uiDCIdx < 4 )
    rcMbFGSCoefMap.getChromaDCScanPos( rcCPlaneIdx ) = ++uiDCIdx;
  TWSOT( m_iRemainingTCoeff < 0 );
  
  return Err::m_nOK;
}






ErrVal
RQFGSEncoder::xEncodeNewCoefficientChromaAC ( MbDataAccess *pcMbDataAccessBL,
                                              MbDataAccess *pcMbDataAccessEL,
                                              MbFGSCoefMap &rcMbFGSCoefMap,
                                              const CIdx   &rcCIdx,
                                              Int&    riLastQP,
                                              UInt    uiChromaScanIndex,
                                              Bool    bFrame  )
{
  CoefMap *pcCoefMap = rcMbFGSCoefMap.getCoefMap( rcCIdx );
  UInt    uiScanIndex  = 16;
  UInt    uiStartIndex = 1;
  UInt    uiIndex;
  for( uiIndex = 1; uiIndex < 16; uiIndex++ )
  {
    if( !(pcCoefMap[uiIndex] & SIGNIFICANT ) || (pcCoefMap[uiIndex] & NEWSIG ) )
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
    Bool bSigBCBP = m_pcSymbolWriter->RQencodeBCBP_ChromaAC( *pcMbDataAccessEL, *pcMbDataAccessBL, rcCIdx );
    rcMbFGSCoefMap.getChromaACBlockMap( rcCIdx ) |= CODED;
    if(  bSigBCBP )
      rcMbFGSCoefMap.getChromaACBlockMap( rcCIdx ) |= SIGNIFICANT;

    if( ! bSigBCBP )
    {
      rcMbFGSCoefMap.getChromaACScanPos( rcCIdx ) = 16;
      for( UInt ui = 1; ui < 16; ui++ )
      {
        if( !(pcCoefMap[ui] & SIGNIFICANT ) )
        {
          pcCoefMap[ui] |= CODED;
          m_iRemainingTCoeff--;
          RNOK( rcMbFGSCoefMap.increaseAndCheckNumCoded( 1 ) );
        }
      }
      TWSOT( m_iRemainingTCoeff < 0 );
      return Err::m_nOK;
    }
  }


  UInt uiNumCoefWritten;
  RNOK( m_pcSymbolWriter->RQencodeNewTCoeff_Chroma( *pcMbDataAccessEL, *pcMbDataAccessBL,
    CHROMA_AC, rcCIdx, uiScanIndex, bNeedEob, uiNumCoefWritten ) );

  const UChar*  pucScan = (bFrame) ? g_aucFrameScan : g_aucFieldScan;
  for ( UInt ui = 0; uiScanIndex < 16 && ( ui < uiNumCoefWritten || bNeedEob ); uiScanIndex++ )
  {
    if( !(pcCoefMap[uiScanIndex] & SIGNIFICANT ) )
    {
      if( pcMbDataAccessEL->getMbTCoeffs().get( rcCIdx )[pucScan[uiScanIndex]] )
        pcCoefMap[uiScanIndex] |= SIGNIFICANT | NEWSIG;
      pcCoefMap[uiScanIndex] |= CODED;
      m_iRemainingTCoeff--;
      RNOK( rcMbFGSCoefMap.increaseAndCheckNumCoded( 1 ) );
      ui++;
    }
  }
  rcMbFGSCoefMap.getChromaACScanPos( rcCIdx ) = uiScanIndex;
  while( ( pcCoefMap[uiScanIndex] & SIGNIFICANT ) && uiScanIndex < 16 )
    rcMbFGSCoefMap.getChromaACScanPos( rcCIdx ) = ++uiScanIndex;
  TWSOT( m_iRemainingTCoeff < 0 );
  
  return Err::m_nOK;
}







ErrVal
RQFGSEncoder::xEncodeCoefficientLumaRef( MbDataAccess  *pcMbDataAccessBL,
                                         MbDataAccess  *pcMbDataAccessEL,
                                         MbFGSCoefMap  &rcMbFGSCoefMap,
                                         const S4x4Idx &rcIdx,
                                         UInt   uiScanIndex)
{
  B8x8Idx c8x8Idx( rcIdx.getContainingPar8x8() );
  CoefMap* pcCoefMap;

  if( pcMbDataAccessBL->getMbData().isTransformSize8x8() )
  {
    uiScanIndex = 4*uiScanIndex + (rcIdx.s4x4() & 3); // convert scan index to 8x8 scan index
    pcCoefMap = &rcMbFGSCoefMap.getCoefMap( c8x8Idx )[uiScanIndex];
  }
  else
    pcCoefMap = &rcMbFGSCoefMap.getCoefMap( rcIdx )[uiScanIndex];

  //===== check if coefficient is not significant or was already coded =====
  ROFRS( *pcCoefMap & SIGNIFICANT, Err::m_nOK );
  ROTRS( *pcCoefMap & CODED,       Err::m_nOK );

  if( pcMbDataAccessBL->getMbData().isTransformSize8x8() )
  {
    RNOK( m_pcSymbolWriter->RQencodeTCoeffRef_8x8( *pcMbDataAccessEL, *pcMbDataAccessBL, c8x8Idx, uiScanIndex, rcMbFGSCoefMap.getRefCtx( c8x8Idx )[uiScanIndex]>>2 ) );
  }
  else
  {
    RNOK( m_pcSymbolWriter->RQencodeTCoeffRef_Luma( *pcMbDataAccessEL, *pcMbDataAccessBL, rcIdx, uiScanIndex, rcMbFGSCoefMap.getRefCtx( rcIdx )[uiScanIndex]>>2 ) );
  }
  *pcCoefMap |= CODED;
  m_iRemainingTCoeff--;
  RNOK( rcMbFGSCoefMap.increaseAndCheckNumCoded( 1 ) );
  TWSOT( m_iRemainingTCoeff < 0 );

  return Err::m_nOK;
}


ErrVal
RQFGSEncoder::xEncodeCoefficientChromaDCRef ( MbDataAccess    *pcMbDataAccessBL,
                                              MbDataAccess    *pcMbDataAccessEL,
                                              MbFGSCoefMap    &rcMbFGSCoefMap,
                                              const CPlaneIdx &rcCPlaneIdx,
                                              UInt  uiDCIdx )
{
  CoefMap &rcCoefMap = rcMbFGSCoefMap.getCoefMap( CIdx( rcCPlaneIdx ) + uiDCIdx )[0];
  //===== check if coefficient is not significant or was already coded =====
  ROF( rcCoefMap & SIGNIFICANT );
  ROT( rcCoefMap & CODED       );
  
  RNOK( m_pcSymbolWriter->RQencodeTCoeffRef_Chroma( *pcMbDataAccessEL, *pcMbDataAccessBL,
    CHROMA_DC, CIdx( rcCPlaneIdx ), uiDCIdx, rcMbFGSCoefMap.getRefCtx( CIdx( rcCPlaneIdx ) + uiDCIdx )[0]>>2 ) );

  rcCoefMap |= CODED;
  m_iRemainingTCoeff--;
  RNOK( rcMbFGSCoefMap.increaseAndCheckNumCoded( 1 ) );
  TWSOT( m_iRemainingTCoeff < 0 );

  return Err::m_nOK;
}
 

ErrVal
RQFGSEncoder::xEncodeCoefficientChromaACRef ( MbDataAccess *pcMbDataAccessBL,
                                              MbDataAccess *pcMbDataAccessEL,
                                              MbFGSCoefMap &rcMbFGSCoefMap,
                                              const CIdx   &rcCIdx,
                                              UInt  uiScanIdx )
{
  CoefMap &rcCoefMap = rcMbFGSCoefMap.getCoefMap( rcCIdx )[uiScanIdx];

  //===== check if coefficient is not significant or was already coded =====
  ROF( rcCoefMap & SIGNIFICANT );
  ROT( rcCoefMap & CODED       );
  
  RNOK( m_pcSymbolWriter->RQencodeTCoeffRef_Chroma( *pcMbDataAccessEL, *pcMbDataAccessBL,
    CHROMA_AC, rcCIdx, uiScanIdx, rcMbFGSCoefMap.getRefCtx( rcCIdx )[uiScanIdx]>>2 ) );

  rcCoefMap |= CODED;
  m_iRemainingTCoeff--;
  RNOK( rcMbFGSCoefMap.increaseAndCheckNumCoded( 1 ) );
  TWSOT( m_iRemainingTCoeff < 0 );

  return Err::m_nOK;
}

ErrVal
RQFGSEncoder::xPrescanCoefficientLumaRef( UInt   uiBlockYIndex,
                                          UInt   uiBlockXIndex,
                                          UInt   uiScanIndex,
                                          Bool   bFrame )
{
  UInt uiB8x8 = ((uiBlockYIndex>>1)%2) * 2 + ((uiBlockXIndex>>1)%2);
  UInt uiB4x4 = ( uiBlockYIndex    %2) * 2 + ( uiBlockXIndex    %2);
  Par8x8  ePar8x8       = Par8x8(uiB8x8);
  
  B8x8Idx c8x8Idx(ePar8x8);
  S4x4Idx c4x4Idx = S4x4Idx( c8x8Idx ) + uiB4x4;

  MbDataAccess* pcMbDataAccessBL  = 0;
  RNOK( m_pcCurrMbDataCtrl ->initMb( pcMbDataAccessBL, uiBlockYIndex/4, uiBlockXIndex/4 ) );
  UInt    uiMbIndex     = (uiBlockYIndex/4) * 1 * m_uiWidthInMB + (uiBlockXIndex/4);
  CoefMap* pcCoefMap;
  UInt uiScanIdxCurr;
  if( pcMbDataAccessBL->getMbData().isTransformSize8x8() )
  {
    pcCoefMap = m_pcCoefMap[uiMbIndex].getCoefMap( c8x8Idx );
    uiScanIdxCurr = 4*uiScanIndex + 2*( uiBlockYIndex % 2 ) + ( uiBlockXIndex % 2 );
  }
  else
  {
    pcCoefMap = m_pcCoefMap[uiMbIndex].getCoefMap( c4x4Idx );
    uiScanIdxCurr = uiScanIndex;
  }

  //===== check if coefficient is not significant or was already coded =====
  ROFRS( pcCoefMap[uiScanIdxCurr] & SIGNIFICANT, Err::m_nOK );
  ROTRS( pcCoefMap[uiScanIdxCurr] & CODED,       Err::m_nOK );
  MbDataAccess* pcMbDataAccessEL  = 0;
  RNOK( m_cMbDataCtrlEL     .initMb( pcMbDataAccessEL, uiBlockYIndex/4, uiBlockXIndex/4 ) );
  
  if( pcMbDataAccessBL->getMbData().isTransformSize8x8() )
  {
    TCoeff*       piCoeff     = pcMbDataAccessEL->getMbTCoeffs().get8x8( c8x8Idx );
    TCoeff*       piCoeffBase = pcMbDataAccessBL->getMbTCoeffs().get8x8( c8x8Idx );
    const UChar*  pucScan64 = (bFrame) ? g_aucFrameScan64 : g_aucFieldScan64;

     UInt  ui8x8ScanIndex  = 4*uiScanIndex + 2*( uiBlockYIndex % 2 ) + ( uiBlockXIndex % 2 );

    RNOK( ((UvlcWriter *)m_pcSymbolWriter)->xRQprescanTCoeffsRef( piCoeff, piCoeffBase, pucScan64, ui8x8ScanIndex ) );
  }
  else
  {
    TCoeff*       piCoeff     = pcMbDataAccessEL->getMbTCoeffs().get( c4x4Idx );
    TCoeff*       piCoeffBase = pcMbDataAccessBL->getMbTCoeffs().get( c4x4Idx );
    const UChar*  pucScan     =(bFrame) ? g_aucFrameScan : g_aucFieldScan;

    RNOK( ((UvlcWriter *)m_pcSymbolWriter)->xRQprescanTCoeffsRef( piCoeff, piCoeffBase, pucScan, uiScanIndex ) );
  }

  return Err::m_nOK;
}


ErrVal
RQFGSEncoder::xPrescanCoefficientChromaDCRef ( UInt  uiPlane,
                                               UInt  uiMbYIdx,
                                               UInt  uiMbXIdx,
                                               UInt  uiDCIdx )
{
  MbDataAccess* pcMbDataAccessEL  = 0;
  MbDataAccess* pcMbDataAccessBL  = 0;
  RNOK( m_cMbDataCtrlEL     .initMb( pcMbDataAccessEL, uiMbYIdx, uiMbXIdx ) );
  RNOK( m_pcCurrMbDataCtrl ->initMb( pcMbDataAccessBL, uiMbYIdx, uiMbXIdx ) );
  
  {
    TCoeff*       piCoeff     = pcMbDataAccessEL->getMbTCoeffs().get( CIdx(4*uiPlane) );
    TCoeff*       piCoeffBase = pcMbDataAccessBL->getMbTCoeffs().get( CIdx(4*uiPlane) );
    const UChar*  pucScan     = g_aucIndexChromaDCScan;

    RNOK( ((UvlcWriter *)m_pcSymbolWriter)->xRQprescanTCoeffsRef( piCoeff, piCoeffBase, pucScan, uiDCIdx ) );
  }

  return Err::m_nOK;
}
 

ErrVal
RQFGSEncoder::xPrescanCoefficientChromaACRef ( UInt  uiPlane,
                                               UInt  uiB8YIdx,
                                               UInt  uiB8XIdx,
                                               UInt  uiScanIdx )
{
  UInt  uiChromaIdx = 4*uiPlane + 2*(uiB8YIdx%2) + (uiB8XIdx%2);
  
  CIdx  cChromaIdx(uiChromaIdx);
  MbDataAccess* pcMbDataAccessEL  = 0;
  MbDataAccess* pcMbDataAccessBL  = 0;
  RNOK( m_cMbDataCtrlEL     .initMb( pcMbDataAccessEL, uiB8YIdx/2, uiB8XIdx/2 ) );
  RNOK( m_pcCurrMbDataCtrl ->initMb( pcMbDataAccessBL, uiB8YIdx/2, uiB8XIdx/2 ) );
  
  // Count number of remaining values
  {
    TCoeff*       piCoeff     = pcMbDataAccessEL->getMbTCoeffs().get( CIdx(cChromaIdx) );
    TCoeff*       piCoeffBase = pcMbDataAccessBL->getMbTCoeffs().get( CIdx(cChromaIdx) );

    RNOK( ((UvlcWriter *)m_pcSymbolWriter)->xRQprescanTCoeffsRef( piCoeff, piCoeffBase, g_aucFrameScan, uiScanIdx ) );
  }

  return Err::m_nOK;
}

ErrVal
RQFGSEncoder::xVLCParseLuma( UInt   uiBlockYIndex,
                             UInt   uiBlockXIndex,
                             UInt*  pauiNumCoefHist,
                             UInt*  pauiHighMagHist,
                             Bool  bFrame)
{
  UInt    uiB8x8        = ((uiBlockYIndex%4)/2) * 2 + ((uiBlockXIndex%4)/2);
  UInt    uiB4x4        =  (uiBlockYIndex%4)    * 4 +  (uiBlockXIndex%4);
  Par8x8  ePar8x8       = Par8x8(uiB8x8);
  
  B8x8Idx c8x8Idx(ePar8x8);
  B4x4Idx c4x4Idx(uiB4x4);

  MbDataAccess* pcMbDataAccessEL  = 0;
  MbDataAccess* pcMbDataAccessBL  = 0;
  RNOK( m_cMbDataCtrlEL     .initMb( pcMbDataAccessEL, uiBlockYIndex/4, uiBlockXIndex/4 ) );
  RNOK( m_pcCurrMbDataCtrl ->initMb( pcMbDataAccessBL, uiBlockYIndex/4, uiBlockXIndex/4 ) );
  
  if( pcMbDataAccessBL->getMbData().isTransformSize8x8() )
  {
    // Not handled (using 4x4 stats is OK)
  }
  else
  {
    UInt          uiIndex;
    UInt          uiBaseLast     = 0;
    UInt          uiHighMagCount = 0;
    UInt          uiCycle        = 0;
    UInt          uiRun          = 0;
    UInt          uiStop         = 16;
    TCoeff*       piCoeff        = pcMbDataAccessEL->getMbTCoeffs().get( c4x4Idx );
    TCoeff*       piCoeffBase    = pcMbDataAccessBL->getMbTCoeffs().get( c4x4Idx );
    const UChar* pucScan        = (bFrame) ? g_aucFrameScan : g_aucFieldScan;

    for ( uiIndex = 0; uiIndex < uiStop; uiIndex++ )
    {
      // Last pos of base?
      if ( piCoeffBase[pucScan[uiIndex]] )
      {
        uiBaseLast = uiIndex;
      }
    }
    for ( uiIndex = 0; uiIndex < uiStop; uiIndex++ )
    {
      // Significant?
      if ( ! piCoeffBase[pucScan[uiIndex]] )
      {
        if ( piCoeff[pucScan[uiIndex]] )
        {
          pauiNumCoefHist[((uiCycle * 16) + uiBaseLast) * 16 + uiRun+1]++;
          UInt uiAbsMag = ( piCoeff[pucScan[uiIndex]] >= 0 ) ? piCoeff[pucScan[uiIndex]] : -piCoeff[pucScan[uiIndex]];
          uiHighMagCount += ( uiAbsMag > 1 ) ? 1 : 0;
          uiCycle = uiIndex + 1;
          uiRun = 0;
        } else {
          uiRun++;
        }
      }
    }
    if ( uiRun > 0 && uiCycle > 0 )
    {
      pauiNumCoefHist[((uiCycle * 16) + uiBaseLast) * 16]++;
      pauiHighMagHist[uiCycle] += ( uiHighMagCount == 0 ) ? 1 : 0;
    }
  }  
  return Err::m_nOK;
}

ErrVal
RQFGSEncoder::xVLCParseChromaAC( UInt   uiPlane,
                                 UInt   uiBlockYIndex,
                                 UInt   uiBlockXIndex,
                                 UInt*  pauiNumCoefHist,
                                 Bool  bFrame)
{
  UInt    uiCIdx        = 4*uiPlane + 2*(uiBlockYIndex%2) + (uiBlockXIndex%2);
  CIdx cChromaIdx(uiCIdx);
  
  MbDataAccess* pcMbDataAccessEL  = 0;
  MbDataAccess* pcMbDataAccessBL  = 0;
  RNOK( m_cMbDataCtrlEL     .initMb( pcMbDataAccessEL, uiBlockYIndex/2, uiBlockXIndex/2 ) );
  RNOK( m_pcCurrMbDataCtrl ->initMb( pcMbDataAccessBL, uiBlockYIndex/2, uiBlockXIndex/2 ) );
  
  UInt          uiIndex;
  UInt          uiBaseLast  = 0;
  UInt          uiCycle     = 1;
  UInt          uiRun       = 0;
  UInt          uiStop      = 16;
  TCoeff*       piCoeff     = pcMbDataAccessEL->getMbTCoeffs().get( cChromaIdx );
  TCoeff*       piCoeffBase = pcMbDataAccessBL->getMbTCoeffs().get( cChromaIdx );
  const UChar* pucScan     = (bFrame) ? g_aucFrameScan : g_aucFieldScan;

  for ( uiIndex = 0; uiIndex < uiStop; uiIndex++ )
  {
    // Last pos of base?
    if ( piCoeffBase[pucScan[uiIndex]] )
    {
      uiBaseLast = uiIndex;
    }
  }
  for ( uiIndex = 1; uiIndex < uiStop; uiIndex++ )
  {
    // Significant?
    if ( ! piCoeffBase[pucScan[uiIndex]] )
    {
      if ( piCoeff[pucScan[uiIndex]] )
      {
        pauiNumCoefHist[((uiCycle * 16) + uiBaseLast) * 16 + uiRun+1]++;
        uiCycle = uiIndex + 1;
        uiRun = 0;
      } else {
        uiRun++;
      }
    }
  }
  if ( uiRun > 0 && uiCycle > 0 )
  {
    pauiNumCoefHist[((uiCycle * 16) + uiBaseLast) * 16]++;
  }
  return Err::m_nOK;
}

H264AVC_NAMESPACE_END

