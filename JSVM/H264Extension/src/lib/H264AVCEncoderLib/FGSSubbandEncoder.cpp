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




H264AVC_NAMESPACE_BEGIN




RQFGSEncoder::RQFGSEncoder()
: m_pcMbEncoder               ( 0 )
, m_pcSymbolWriter            ( 0 )
, m_iRemainingTCoeff          ( 0 )
, m_pcSliceHeader             ( 0 )
, m_pcOrgResidual             ( 0 )
, m_bTraceEnable              ( true )
{
  ::memset( m_apaucLumaCoefMap,       0x00,   16*sizeof(UChar*) );
  ::memset( m_aapaucChromaDCCoefMap,  0x00, 2* 4*sizeof(UChar*) );
  ::memset( m_aapaucChromaACCoefMap,  0x00, 2*16*sizeof(UChar*) );
  ::memset( m_apaucChromaDCBlockMap,  0x00,    2*sizeof(UChar*) );
  ::memset( m_apaucChromaACBlockMap,  0x00,    2*sizeof(UChar*) );
  m_paucBlockMap        = 0;
  m_paucSubMbMap        = 0;
  m_pauiMacroblockMap   = 0;
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
                    Transform*      pcTransform,
                    ControlMngH264AVCEncoder* pcControlMng,
                    MbEncoder*      pcMbEncoder )
{
  ROT( m_bInit );
  ROF( apcYuvFullPelBufferCtrl );
  ROF( pcTransform );
  ROF( pcControlMng );
  ROF( pcMbEncoder );

  m_pcMbEncoder               = pcMbEncoder;
  m_pcControlMng              = pcControlMng;
  m_iRemainingTCoeff          = 0;
  m_pcSliceHeader             = 0;
  m_pcOrgResidual             = 0;

  xInit( apcYuvFullPelBufferCtrl, pcTransform );

  return Err::m_nOK;
}
  

ErrVal
RQFGSEncoder::uninit()
{
  ROT( m_bPicInit );
  ROF( m_bInit );
  
  xUninit();

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

  m_uiWidthInMB       = pcSliceHeader->getSPS().getFrameWidthInMbs  ();
  m_uiHeightInMB      = pcSliceHeader->getSPS().getFrameHeightInMbs ();
  m_pcSliceHeader     = pcSliceHeader;
  m_iMaxQpDelta       = iMaxQpDelta;
  m_dLambda           = dLambda;
  m_pcCurrMbDataCtrl  = pcCurrMbDataCtrl;
  m_pcOrgResidual     = pcOrgResidual;

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

  RNOK( xInitializeCodingPath() );

  RNOK( xScaleBaseLayerCoeffs() );

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

  return Err::m_nOK;
}

ErrVal
RQFGSEncoder::encodeNextLayer( Bool&  rbFinished,
                               Bool&  rbCorrupted,
                               UInt   uiMaxBits,
                               UInt uiFrac,
                               Bool bFragmented,
                               FILE*  pFile ) //JVT-P031
{
  ROF( m_bInit );
  ROF( m_bPicInit );
  rbCorrupted = false;
  rbFinished  = false;

  //JVT-P031
  if(!uiFrac) RNOK ( xResidualTransform() );
  RNOK ( xEncodingFGS( rbFinished, rbCorrupted, uiMaxBits, uiFrac, pFile ) );
  ROTRS( rbFinished, Err::m_nOK );

  //FIX_FRAG_CAVLC
  if((bFragmented && (uiFrac != 0 || !rbCorrupted) ) || !bFragmented)
  {
    //~FIX_FRAG_CAVLC
  Int iOldSliceQP     = m_pcSliceHeader->getPicQp();
  Int iNewSliceQP     = max( 0, iOldSliceQP - RQ_QP_DELTA );
  rbFinished          = ( iNewSliceQP == iOldSliceQP );
  if((bFragmented && uiFrac != 0) || !bFragmented)
      m_pcSliceHeader->setQualityLevel ( m_pcSliceHeader->getQualityLevel() + 1 );
  m_pcSliceHeader->setSliceHeaderQp       ( iNewSliceQP );
  m_dLambda          /= pow( 2.0, (Double)(iOldSliceQP-iNewSliceQP) / 3.0 );
   } //FIX_FRAG_CAVLC
  //~JVT-P031
  return Err::m_nOK;
}


ErrVal
RQFGSEncoder::reconstruct( IntFrame* pcRecResidual )
{
  ROF( m_bInit );
  ROF( m_bPicInit );
  ROF( pcRecResidual );

  UInt            uiLayer         = m_pcSliceHeader->getSPS().getLayerId();
  YuvBufferCtrl*  pcYuvBufferCtrl = m_papcYuvFullPelBufferCtrl[uiLayer];
  IntYuvMbBuffer  cMbBuffer;

  RNOK( m_pcCurrMbDataCtrl->initSlice ( *m_pcSliceHeader, PRE_PROCESS, false, NULL ) );

  for( UInt uiMbY = 0; uiMbY < m_uiHeightInMB; uiMbY++ )
  for( UInt uiMbX = 0; uiMbX < m_uiWidthInMB;  uiMbX++ )
  {
    MbDataAccess* pcMbDataAccess = 0;

    RNOK( m_pcCurrMbDataCtrl->initMb(  pcMbDataAccess, uiMbY, uiMbX ) );
    RNOK( pcYuvBufferCtrl   ->initMb(                  uiMbY, uiMbX ) );
    RNOK( xReconstructMacroblock    ( *pcMbDataAccess, cMbBuffer    ) );

    RNOK( pcRecResidual->getFullPelYuvBuffer()->loadBuffer( &cMbBuffer ) );
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
                                 const QpParameter&  cQP,
                                 UInt&               uiCoeffCostDC,
                                 UInt&               uiCoeffCostAC,
                                 Bool&               bSigDC,
                                 Bool&               bSigAC,
                                 ChromaIdx           cIdx )
{
  Int iRun  = 0;
  UInt    uiPlane             = cIdx.plane();
  UChar** ppucChromaDCCoefMap = m_aapaucChromaDCCoefMap[uiPlane];
  UChar** ppucChromaACCoefMap = m_aapaucChromaACCoefMap[uiPlane];
  UInt    uiMbIndex           = uiMbY * m_uiWidthInMB + uiMbX;
  UInt    ui8x8Idx = ( 2 * uiMbY + cIdx.y() ) * 2 * m_uiWidthInMB + ( 2 * uiMbX + cIdx.x() );

  for( Int iPos = 0; iPos < 16; iPos++ )
  {
    Int iIndex      = g_aucFrameScan[iPos];
    Int iLevel      = piCoeff       [iIndex];
    UChar ucSigMap = ( iPos == 0 ) ? 
      ppucChromaDCCoefMap[cIdx.y()* 2 + cIdx.x()][uiMbIndex] : ppucChromaACCoefMap[iPos][ui8x8Idx];

    if( ucSigMap & SIGNIFICANT )
    {
      if( abs(iLevel) > 1 )
      {
        iLevel = max( -1, min( 1, iLevel ) );
      }

      piCoeff[iIndex] = iLevel;
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
                              const QpParameter&  cQP,
                              UInt&               uiCoeffCost,
                              UInt&               ruiCbp,
                              LumaIdx             cIdx,
                              UInt                uiStart )
{
  Int   iRun  = 0;
  Bool  bSig  = false;

  for( Int iPos = uiStart; iPos < 16; iPos++ )
  {
    Int iIndex      = g_aucFrameScan[iPos];
    Int iLevel      = piCoeff       [iIndex];

    UInt uiBlockIndex = ( uiMbY * 4 + cIdx.y() ) * m_uiWidthInMB * 4 + uiMbX * 4 + cIdx.x();
    if( m_apaucLumaCoefMap[iPos][uiBlockIndex] & SIGNIFICANT )
    {
      if( abs(iLevel) > 1 )
      {
        iLevel = max( -1, min( 1, iLevel ) );
      }

      piCoeff[iIndex] = iLevel;
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
    ruiCbp |= ( 1 << cIdx );
  }

  return Err::m_nOK;
}






ErrVal
RQFGSEncoder::xSetSymbols8x8( TCoeff*             piCoeff,
                              TCoeff*             piCoeffBase,
                              const QpParameter&  cQP,
                              UInt&               uiCoeffCost,
                              UInt&               ruiCbp,
                              LumaIdx             cIdx )
{
  Int   iRun  = 0;
  Bool  bSig  = false;

  for( Int iPos = 0; iPos < 64; iPos++ )
  {
    Int iIndex      = g_aucFrameScan64[iPos];
    Int iBaseLevel  = piCoeffBase     [iIndex];
    Int iLevel      = piCoeff         [iIndex];

    if( iBaseLevel )
    {
      if( abs(iLevel) > 1 )
      {
        iLevel = max( -1, min( 1, iLevel ) );
      }

      piCoeff[iIndex] = iLevel;
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
                                     MbDataAccess&    rcMbDataAccessBase,
                                     IntYuvMbBuffer&  rcBLRecBuffer )
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
  UInt          uiMbIndex = uiMbY * m_uiWidthInMB + uiMbX;

  rcMbDataAccess.getMbData().setQp( iQp );
  m_pcTransform->setQp( rcMbDataAccess, bLowPass || bIntra );

  IntYuvMbBuffer  cMbBuffer;
  cMbBuffer.loadBuffer( m_pcOrgResidual->getFullPelYuvBuffer() );
  cMbBuffer.subtract  ( rcBLRecBuffer );

  //===== luma =====
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
                                            pucScaleY, uiAbsSum8x8 ) );
      uiAbsSumMb += uiAbsSum8x8;

      RNOK( xSetSymbols8x8( rcMbDataAccess    .getMbTCoeffs().get8x8( c8x8Idx ),
                            rcMbDataAccessBase.getMbTCoeffs().get8x8( c8x8Idx ),
                            m_pcTransform->getLumaQp(),
                            uiCoeffCost8x8, uiCbp, c8x8Idx ) );

      if( uiCbp )
      {
#if COEF_SKIP
        if( uiCoeffCost8x8 <= 4 && ! bIntra && ! bLowPass )
        {
          rcMbDataAccess.getMbTCoeffs().clearNewLumaLevels8x8Block( c8x8Idx, rcMbDataAccessBase.getMbTCoeffs() );
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
      rcMbDataAccess.getMbTCoeffs().clearNewLumaLevels( rcMbDataAccessBase.getMbTCoeffs() );
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
          pucScaleY, 
          bIntra16x16,
          uiAbsSum4x4 ) );

        uiAbsSum8x8 += uiAbsSum4x4;
        if ( bIntra16x16 )
          uiAbsSum8x8 -= abs(rcMbDataAccess.getMbTCoeffs().get( cIdx )[0]);

        RNOK( xSetSymbols4x4( rcMbDataAccess    .getMbTCoeffs().get( cIdx ),
          uiMbX, uiMbY, 
          m_pcTransform->getLumaQp(),
          uiCoeffCost8x8, uiCbp, cIdx, bIntra16x16 ) );
      }
      if( uiCbp )
      {
#if COEF_SKIP
        if( uiCoeffCost8x8 <= 4 && ! bIntra && ! bLowPass && ! bIntra16x16 )
        {
          rcMbDataAccess.getMbTCoeffs().clearNewLumaLevels8x8( c8x8Idx, rcMbDataAccessBase.getMbTCoeffs() );
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
      TCoeff  *piCoeff, *piCoeffBase;

      // put DC coefficients into buffer
      piCoeff     = rcMbDataAccess    .getMbTCoeffs().get( B4x4Idx(0) );
      piCoeffBase = rcMbDataAccessBase.getMbTCoeffs().get( B4x4Idx(0) );

      // transform and quantization on intra16x16 DC coefficients
      m_pcTransform->requantLumaDcCoeffs( piCoeff, piCoeffBase, pucScaleY, uiAbsDc );

      for ( UInt uiDcIdx = 0; uiDcIdx < 16; uiDcIdx ++ )
      {
        Int iLevel      = piCoeff    [16 * uiDcIdx];
        Int uiBlockIndex = (uiMbY * 4 + uiDcIdx/4)* m_uiWidthInMB * 4 + uiMbX * 4 + (uiDcIdx % 4);
        if( m_apaucLumaCoefMap[0][uiBlockIndex] & SIGNIFICANT )

        {
          if( abs(iLevel) > 1 )
          {
            iLevel = max( -1, min( 1, iLevel ) );
          }

          piCoeff[16 * uiDcIdx] = iLevel;
        }
        else
        // WARNING, should have "else" here because CBP is only for new-significant coefficients

        // since the DC coefficients are merged with the AC coefficients
        // update the cbp information
        uiExtCbp |= (iLevel != 0) << uiDcIdx;
      }
    }

#if COEF_SKIP
    if( ! bIntra16x16 )
    if( uiExtCbp && uiCoeffCostMb <= 5 && ! bIntra && ! bLowPass )
    {
      rcMbDataAccess.getMbTCoeffs().clearNewLumaLevels( rcMbDataAccessBase.getMbTCoeffs() );
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

  cMbBuffer.set4x4Block( B4x4Idx(0) );
  RNOK( m_pcTransform->requantChroma( cMbBuffer,
                                      rcMbDataAccess    .getMbTCoeffs().get( CIdx(0) ),
                                      rcMbDataAccessBase.getMbTCoeffs().get( CIdx(0) ),
                                      pucScaleU, pucScaleV, uiDcAbs, uiAcAbs ) );
  
  //===== chrominance U =====
  {
    UInt  uiCoeffCostAC_U = 0;
    
    for( CIdx cIdxU(0); cIdxU.isLegal(4); cIdxU++ )
    {
      RNOK( xSetSymbolsChroma( rcMbDataAccess    .getMbTCoeffs().get( cIdxU ),
                               uiMbX, uiMbY, 
                               m_pcTransform->getChromaQp(),
                               uiCoeffCostDC, uiCoeffCostAC_U, bSigDC, bSigAC_U, cIdxU ) );
    }
#if COEF_SKIP
    if( uiCoeffCostAC_U < 4 )
    {
      for( CIdx cIdxU(0); cIdxU.isLegal(4); cIdxU++ )
      {
        rcMbDataAccess.getMbTCoeffs().clearNewAcBlk( cIdxU, rcMbDataAccessBase.getMbTCoeffs() );
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
      UInt    ui8x8Idx    = ( 2*uiMbY + cIdxV.y() ) * 2 * m_uiWidthInMB + ( 2 * uiMbX + cIdxV.x() );
      RNOK( xSetSymbolsChroma( rcMbDataAccess    .getMbTCoeffs().get( cIdxV ),
                               uiMbX, uiMbY, 
                               m_pcTransform->getChromaQp(),
                               uiCoeffCostDC, uiCoeffCostAC_V, bSigDC, bSigAC_V, cIdxV ) );
    }
#if COEF_SKIP
    if( uiCoeffCostAC_V < 4 )
    {
      for( CIdx cIdxV(4); cIdxV.isLegal(8); cIdxV++ )
      {
        rcMbDataAccess.getMbTCoeffs().clearNewAcBlk( cIdxV, rcMbDataAccessBase.getMbTCoeffs() );
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
RQFGSEncoder::xResidualTransform( /*UInt uiBasePos*/ )
{
  UInt            uiLayer         = m_pcSliceHeader->getSPS().getLayerId();
  YuvBufferCtrl*  pcYuvBufferCtrl = m_papcYuvFullPelBufferCtrl[uiLayer];
  Bool            bLowPass        = m_pcSliceHeader->getTemporalLevel() == 0;
  IntYuvMbBuffer  cBLRecBuffer;

  RNOK( m_cMbDataCtrlEL.clear     () );
  RNOK( m_cMbDataCtrlEL.reset     () );
  
  RNOK( m_cMbDataCtrlEL    .initSlice ( *m_pcSliceHeader, ENCODE_PROCESS, false, NULL ) );
  RNOK( m_pcCurrMbDataCtrl->initSlice ( *m_pcSliceHeader, PRE_PROCESS,    false, NULL ) );

  for( UInt uiMbY = 0; uiMbY < m_uiHeightInMB; uiMbY++ )
  for( UInt uiMbX = 0; uiMbX < m_uiWidthInMB;  uiMbX++ )
  {
    MbDataAccess* pcMbDataAccessCurr  = 0;
    MbDataAccess* pcMbDataAccessEL    = 0;

    RNOK( m_pcCurrMbDataCtrl->initMb( pcMbDataAccessCurr, uiMbY, uiMbX ) );
    RNOK( m_cMbDataCtrlEL    .initMb( pcMbDataAccessEL,   uiMbY, uiMbX ) );
    RNOK( pcYuvBufferCtrl   ->initMb(                     uiMbY, uiMbX ) );

    if( pcMbDataAccessCurr->getMbData().isIntra16x16() ||
      ( m_pauiMacroblockMap[uiMbY * m_uiWidthInMB + uiMbX] & SIGNIFICANT ) != 0 )
    {
      //===== requantization =====
      RNOK( xReconstructMacroblock( *pcMbDataAccessCurr, cBLRecBuffer ) );
      RNOK( xRequantizeMacroblock ( *pcMbDataAccessEL,
                                    *pcMbDataAccessCurr, cBLRecBuffer ) );
    }
    else
    {
      //===== new encoding =====
      m_pcTransform->setClipMode( false );
      pcMbDataAccessEL->getMbData().setQp( m_pcSliceHeader->getPicQp() );
      RNOK( m_pcMbEncoder->encodeResidual( *pcMbDataAccessEL,
                                           *pcMbDataAccessCurr,
                                           m_pcOrgResidual,
                                           m_dLambda,
                                           bLowPass,
                                           m_iMaxQpDelta ) );
      m_pcTransform->setClipMode( true );
    }
  }

 
  return Err::m_nOK;
}




ErrVal
RQFGSEncoder::xScaleBaseLayerCoeffs()
{
  RNOK( m_pcCurrMbDataCtrl->initSlice( *m_pcSliceHeader, PRE_PROCESS, false, NULL ) );

  for( UInt uiMbY = 0; uiMbY < m_uiHeightInMB; uiMbY++ )
  for( UInt uiMbX = 0; uiMbX < m_uiWidthInMB;  uiMbX++ )
  {
    MbDataAccess* pcMbDataAccess = 0;

    RNOK( m_pcCurrMbDataCtrl->initMb(  pcMbDataAccess, uiMbY, uiMbX ) );
    RNOK( xScaleTCoeffs             ( *pcMbDataAccess, true ) );
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
    for( UInt uiMbY = 0; uiMbY < m_uiHeightInMB; uiMbY++ )
  {
    for( UInt uiMbX = 0; uiMbX < m_uiWidthInMB;  uiMbX++ )
    {
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
    }
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
                             UInt iCompleteLuma,
                             UInt iCompleteChromaDC,
                             UInt iCompleteChromaAC,
                             UInt iCycle,
                             UInt itCompleteLuma,
                             UInt itCompleteChromaDC,
                             UInt itCompleteChromaAC,
                             UInt bAllowChromaDC,
                             UInt bAllowChromaAC,
                             UInt uiMbYIdx,
                             UInt uiMbXIdx,
                             UInt uiB8YIdx,
                             UInt uiB8XIdx,
                             UInt uiBlockYIdx,
                             UInt uiBlockXIdx,
                             UInt iLastBitsLuma,
                             UInt uiBitsLast,
                             UInt uiFGSPart,
                             UInt uiPlane,
                             Int iLastQP)
{
  m_iLumaScanIdx = iLumaScanIdx;
  m_iChromaDCScanIdx = iChromaDCScanIdx;
  m_iChromaACScanIdx = iChromaACScanIdx;
  m_iStartCycle = iStartCycle;
  m_iCompleteLuma = iCompleteLuma;
  m_iCompleteChromaDC = iCompleteChromaDC;
  m_iCompleteChromaAC = iCompleteChromaAC;
  m_iCycle = iCycle;
  m_itCompleteLuma = itCompleteLuma;
  m_itCompleteChromaDC = itCompleteChromaDC;
  m_itCompleteChromaAC = itCompleteChromaAC;
  m_bAllowChromaDC = bAllowChromaDC;
  m_bAllowChromaAC = bAllowChromaAC;
  m_uiMbYIdx = uiMbYIdx;
  m_uiMbXIdx = uiMbXIdx;
  m_uiB8YIdx = uiB8YIdx;
  m_uiB8XIdx = uiB8XIdx;
  m_uiBlockYIdx = uiBlockYIdx;
  m_uiBlockXIdx = uiBlockXIdx;
  m_iLastBitsLuma = iLastBitsLuma;
  m_uiBitsLast = uiBitsLast;
  m_uiFGSPart = uiFGSPart;
  m_uiPlane = uiPlane;
  m_iLastQP = iLastQP;

  return Err::m_nOK;
}

ErrVal
RQFGSEncoder::xRestoreFGSState(UInt& riLumaScanIdx,
                             UInt& riChromaDCScanIdx,
                             UInt& riChromaACScanIdx,
                             UInt& riStartCycle,
                             UInt& riCompleteLuma,
                             UInt& riCompleteChromaDC,
                             UInt& riCompleteChromaAC,
                             UInt& riCycle,
                             UInt& ritCompleteLuma,
                             UInt& ritCompleteChromaDC,
                             UInt& ritCompleteChromaAC,
                             UInt& rbAllowChromaDC,
                             UInt& rbAllowChromaAC,
                             UInt& ruiMbYIdx,
                             UInt& ruiMbXIdx,
                             UInt& ruiB8YIdx,
                             UInt& ruiB8XIdx,
                             UInt& ruiBlockYIdx,
                             UInt& ruiBlockXIdx,
                             UInt& riLastBitsLuma,
                             UInt& ruiBitsLast,
                             UInt& ruiFGSPart,
                             UInt& ruiPlane,
                             Int& riLastQP)
{
  riLumaScanIdx = m_iLumaScanIdx;
  riChromaDCScanIdx = m_iChromaDCScanIdx;
  riChromaACScanIdx = m_iChromaACScanIdx;
  riStartCycle = m_iStartCycle;
  riCompleteLuma = m_iCompleteLuma;
  riCompleteChromaDC = m_iCompleteChromaDC;
  riCompleteChromaAC = m_iCompleteChromaAC;
  riCycle = m_iCycle;
  ritCompleteLuma = m_itCompleteLuma;
  ritCompleteChromaDC = m_itCompleteChromaDC;
  ritCompleteChromaAC = m_itCompleteChromaAC;
  rbAllowChromaDC = m_bAllowChromaDC;
  rbAllowChromaAC = m_bAllowChromaAC;
  ruiMbYIdx = m_uiMbYIdx;
  ruiMbXIdx = m_uiMbXIdx;
  ruiB8YIdx = m_uiB8YIdx;
  ruiB8XIdx = m_uiB8XIdx;
  ruiBlockYIdx = m_uiBlockYIdx;
  ruiBlockXIdx = m_uiBlockXIdx;
  riLastBitsLuma = m_iLastBitsLuma;
  ruiBitsLast = m_uiBitsLast;
  ruiFGSPart = m_uiFGSPart;
  ruiPlane = m_uiPlane;
  riLastQP = m_iLastQP;
  return Err::m_nOK;
}
//~JVT-P031

ErrVal
RQFGSEncoder::xEncodingFGS( Bool& rbFinished,
                            Bool& rbCorrupted,
                            UInt uiMaxBits,
                            UInt uiFracNb,
                            FILE* pFile ) //JVT-P031 (modified for fragments)
{
  Bool bRestore = true;
  Bool bFirstPass = true;

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

#if 1 // BUG FIX Martin.Winken@hhi.fhg.de
  if( uiFracNb )
  {
    RNOK( xRestoreCodingPath() );
  }
#else
  if(!uiFracNb) 
  {
    RNOK( xInitializeCodingPath() );
  }
  else
    xRestoreCodingPath();
#endif
  
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
  try
  {
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
      UInt iCompleteLuma     = 0;
      UInt iCompleteChromaDC = 0;
      UInt iCompleteChromaAC = 0;
      UInt uiFirstMbY = (UInt) ( m_uiFirstMbInSlice / m_uiWidthInMB );
      UInt uiFirstMbX = m_uiFirstMbInSlice % m_uiWidthInMB;
      UInt uiLastMbY  = (UInt) ( ( m_uiFirstMbInSlice + m_uiNumMbsInSlice ) / m_uiWidthInMB );
      UInt uiLastMbX  = ( m_uiFirstMbInSlice + m_uiNumMbsInSlice ) % m_uiWidthInMB;
      if(!uiFracNb) //FIX_FRAG_CAVLC
      {
      // Pre-scan frame to find VLC positions
      UInt  auiEobShift[16];
      UInt  auiHighMagHist[16];
      UInt* pauiVlcTab      = new UInt[16*16   ];
      UInt* pauiHistLuma    = new UInt[16*16*16];
      UInt* pauiHistChroma  = new UInt[16*16*16];

      ::memset(pauiHistLuma  , 0x00, 16*16*16*sizeof(UInt));
      ::memset(pauiHistChroma, 0x00, 16*16*16*sizeof(UInt));
      ::memset(auiHighMagHist, 0x00,       16*sizeof(UInt));
      
      for( UInt uiMbYIdx = uiFirstMbY; uiMbYIdx < uiLastMbY; uiMbYIdx++ )
      for( UInt uiMbXIdx = ( uiMbYIdx == uiFirstMbY ? uiFirstMbX : 0 ); uiMbXIdx < ( uiMbYIdx == uiLastMbY ? uiLastMbX : m_uiWidthInMB );  uiMbXIdx++ )
      {
        //===== LUMA =====
        for( UInt uiB8YIdx = 2 * uiMbYIdx; uiB8YIdx < 2 * uiMbYIdx + 2; uiB8YIdx++ )
        for( UInt uiB8XIdx = 2 * uiMbXIdx; uiB8XIdx < 2 * uiMbXIdx + 2; uiB8XIdx++ )
        {
          UInt uiCbp = 0;
          UInt uiNumCb = 0;
          for( UInt uiBlockYIdx = 2 * uiB8YIdx; uiBlockYIdx < 2 * uiB8YIdx + 2; uiBlockYIdx++ )
          for( UInt uiBlockXIdx = 2 * uiB8XIdx; uiBlockXIdx < 2 * uiB8XIdx + 2; uiBlockXIdx++ )
          {
            RNOK( xVLCParseLuma( uiBlockYIdx, uiBlockXIdx, pauiHistLuma, auiHighMagHist ) );
          } // 4x4 block iteration
        } // 8x8 block iteration
        // ===== CHROMA AC =====
        for( UInt uiPlane = 0; uiPlane  < 2; uiPlane ++ )
        for( UInt uiB8YIdx = 2 * uiMbYIdx; uiB8YIdx < 2 * uiMbYIdx + 2; uiB8YIdx++ )
        for( UInt uiB8XIdx = 2 * uiMbXIdx; uiB8XIdx < 2 * uiMbXIdx + 2; uiB8XIdx++ ) {
            RNOK( xVLCParseChromaAC( uiPlane, uiB8YIdx, uiB8XIdx, pauiHistChroma ) );
        } // for
      } // macroblock iteration
      
      RNOK( m_cMbDataCtrlEL    .initSlice ( *m_pcSliceHeader, PRE_PROCESS, false, NULL ) );
      RNOK( m_pcCurrMbDataCtrl->initSlice ( *m_pcSliceHeader, PRE_PROCESS, false, NULL ) );

      // Tally over base pos to find EOB shift
      for (uiScanPos=0; uiScanPos<16; uiScanPos++)
      {
        auiEobShift[uiScanPos] = 0;
        for (UInt uiHpos=1; uiHpos<16; uiHpos++) {
          UInt uiTotal = 0;
          for (int uiBase=0; uiBase<16; uiBase++)
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

      // VLC selection for sig coeffs
      for (     uiScanPos=0; uiScanPos<16; uiScanPos++)
      for (UInt uiBase   =0; uiBase   <16; uiBase++   )
      {
        UInt uiCutoff;
        UInt uiBestCost = UInt(-1);
        for ( uiCutoff = 0; uiCutoff < 16; uiCutoff++ )
        {
          UInt uiCost = 0;
          for (UInt uiHpos=0; uiHpos<16; uiHpos++)
          {
            UInt uiSymbol;
            UInt uiCount;
            if (uiHpos == 0)
            {
              uiSymbol = auiEobShift[uiScanPos];
              uiCount  = auiHighMagHist[uiScanPos];
            } else {
              uiSymbol = (uiHpos <= auiEobShift[uiScanPos]) ? uiHpos-1 : uiHpos;
              uiCount  = pauiHistLuma[(uiScanPos*16+uiBase)*16+uiHpos];
            }
            uiCost += uiCount * (( uiSymbol <= uiCutoff ) ? uiSymbol + 1 : ((int)((uiSymbol-uiCutoff-1)/3)) * 2 + uiCutoff + 3);
            if ( uiCost > uiBestCost )
              break;
          }
          if ( uiCost < uiBestCost )
            uiBestCost = uiCost;
          else
            break;
        }
        pauiVlcTab[uiScanPos*16+uiBase] = max(uiCutoff - 1, (uiBase == 0) ? (uiScanPos == 0 ? 0 : pauiVlcTab[uiScanPos*16-16])
                                                                         : pauiVlcTab[uiScanPos*16+uiBase-1] );
        pauiVlcTab[uiScanPos*16+uiBase] = min(4, pauiVlcTab[uiScanPos*16+uiBase]);
      }

      // Tally over base pos to find EOB shift
      for (uiScanPos=0; uiScanPos<16; uiScanPos++) {
        auiEobShift[uiScanPos] = 0;
        UInt uiEobTotal = 0;
        for (UInt uiBase=0; uiBase<16; uiBase++)
          uiEobTotal += pauiHistChroma[(uiScanPos*16+uiBase)*16];
        for (UInt uiHpos=1; uiHpos<16; uiHpos++) {
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

      RNOK( m_pcSymbolWriter->RQencodeVlcTableMap( pauiVlcTab, 16, 16 ) );

      delete pauiVlcTab;
      delete pauiHistLuma;
      delete pauiHistChroma;
      } //FIX_FRAG_CAVLC

      while (iLumaScanIdx < 16 || iChromaDCScanIdx < 4 || iChromaACScanIdx < 16) {
        UInt bAllowChromaDC = (iCycle == 0) || ((iCycle >= iStartCycle) && ((iCycle-iStartCycle) % 2 == 0));
        UInt bAllowChromaAC = (iCycle > 0) && ((iCycle == iStartCycle) || ((iCycle >= iStartCycle) && ((iCycle-iStartCycle) % 3 == 1)));
        UInt itCompleteLuma     = 1;
        UInt itCompleteChromaDC = bAllowChromaDC;
        UInt itCompleteChromaAC = bAllowChromaAC;

        //if(!uiFracNb) bCheck = true;
        //m_uiMaxBits = 10000;
        if(!uiFracNb)
        {
          // start values for loops
          m_uiMbYIdx = uiFirstMbY;
          m_uiMbXIdx = ( m_uiMbYIdx == uiFirstMbY ? uiFirstMbX : 0 );
          m_uiB8YIdx = 2*m_uiMbYIdx;
          m_uiB8XIdx = 2*m_uiMbXIdx;
          m_uiBlockYIdx = 2*m_uiB8YIdx;
          m_uiBlockXIdx = 2*m_uiB8XIdx;
          m_uiPlane = 0;
          m_uiFGSPart = 0;
        }
        else if(bRestore)
        {
          bCheck = false;
          // restore status
          UInt uiUnused;
          xRestoreFGSState(iLumaScanIdx, iChromaDCScanIdx, iChromaACScanIdx, iStartCycle, iCompleteLuma, iCompleteChromaDC, iCompleteChromaAC, bAllowChromaDC, bAllowChromaAC, iCycle, itCompleteLuma, itCompleteChromaDC, itCompleteChromaAC, uiUnused, uiUnused, uiUnused, uiUnused, uiUnused, uiUnused, iLastBitsLuma, /*uiBitsLast*/uiUnused, uiUnused, uiUnused, iLastQP);
          bRestore = false;
        }

        for( UInt uiMbYIdx = m_uiMbYIdx; uiMbYIdx < uiLastMbY; uiMbYIdx++ )
        {
          for( UInt uiMbXIdx = m_uiMbXIdx; uiMbXIdx < ( uiMbYIdx == uiLastMbY ? uiLastMbX : m_uiWidthInMB );  uiMbXIdx++ )
          {
            if(!bFirstPass)
            {
              m_uiMbYIdx = uiMbYIdx;
              m_uiMbXIdx = uiMbXIdx;
              m_uiB8YIdx = 2*m_uiMbYIdx;
              m_uiB8XIdx = 2*m_uiMbXIdx;
            }
            //===== LUMA =====
            if( (m_uiFGSPart == 0) || (!bFirstPass) )
            {
              for( UInt uiB8YIdx = m_uiB8YIdx; uiB8YIdx < 2 * uiMbYIdx + 2; uiB8YIdx++ )
              {
                for( UInt uiB8XIdx = m_uiB8XIdx; uiB8XIdx < 2 * uiMbXIdx + 2; uiB8XIdx++ )
                {
                  if(!bFirstPass)
                  {
                    m_uiBlockYIdx = 2*uiB8YIdx;
                    m_uiBlockXIdx = 2*uiB8XIdx;
                  }
                  else
                  {
                    bFirstPass = false;
                  }
                  for( UInt uiBlockYIdx = m_uiBlockYIdx; uiBlockYIdx < 2 * uiB8YIdx + 2; uiBlockYIdx++ )
                  {
                    for( UInt uiBlockXIdx = m_uiBlockXIdx; uiBlockXIdx < 2 * uiB8XIdx + 2; uiBlockXIdx++ )
                    {
                      if (!iCompleteLuma) {
                        iLastBitsLuma = m_pcSymbolWriter->getNumberOfWrittenBits();

                        RNOK( xEncodeNewCoefficientLuma( uiBlockYIdx, uiBlockXIdx, itCompleteLuma, iLastQP ) );

                        if (iCycle == 0)
                          iBitsLuma += m_pcSymbolWriter->getNumberOfWrittenBits() - iLastBitsLuma;

                        if( bCheck && m_pcSymbolWriter->getNumberOfWrittenBits() - uiBitsLast >= uiMaxBits )
                        {
                          xStoreFGSState(iLumaScanIdx, iChromaDCScanIdx, iChromaACScanIdx, iStartCycle, iCompleteLuma, iCompleteChromaDC, iCompleteChromaAC, bAllowChromaDC, bAllowChromaAC, iCycle, itCompleteLuma, itCompleteChromaDC, itCompleteChromaAC, uiMbYIdx, uiMbXIdx, uiB8YIdx, uiB8XIdx, uiBlockYIdx, uiBlockXIdx+1, iLastBitsLuma, uiBitsLast, 0, 0, iLastQP);
                          throw WriteStop();
                        }
                      } else if (iLumaScanIdx < 16) {
                        RNOK( xEncodeCoefficientLumaRef( uiBlockYIdx, uiBlockXIdx, iLumaScanIdx ) );
                        if( bCheck && m_pcSymbolWriter->getNumberOfWrittenBits() - uiBitsLast >= uiMaxBits )
                        {
                          xStoreFGSState(iLumaScanIdx, iChromaDCScanIdx, iChromaACScanIdx, iStartCycle, iCompleteLuma, iCompleteChromaDC, iCompleteChromaAC, bAllowChromaDC, bAllowChromaAC, iCycle, itCompleteLuma, itCompleteChromaDC, itCompleteChromaAC, uiMbYIdx, uiMbXIdx, uiB8YIdx, uiB8XIdx, uiBlockYIdx, uiBlockXIdx+1, iLastBitsLuma, uiBitsLast, 0, 0, iLastQP);
                          throw WriteStop();
                        }
                        }
                    } // 4x4 block iteration
                    m_uiBlockXIdx = 2 * uiB8XIdx;
                  }
                } // 8x8 block iteration
                m_uiB8XIdx = 2 * uiMbXIdx;
              }
              m_uiPlane = 0;
              m_uiB8YIdx = 2 * uiMbYIdx;
              m_uiB8XIdx = 2 * uiMbXIdx;
            }

            // ===== CHROMA DC =====
            if( (m_uiFGSPart == 1) || (!bFirstPass) )
            {
              if( bAllowChromaDC && ( m_bFgsComponentSep == 0 || iLumaScanIdx == 16 ) ) {
                bFirstPass = false;
                for( UInt uiPlane = m_uiPlane; uiPlane  < 2; uiPlane ++ ) {

                  if (!iCompleteChromaDC) {
                    iLastBitsChroma = m_pcSymbolWriter->getNumberOfWrittenBits();
                    RNOK( xEncodeNewCoefficientChromaDC( uiPlane, uiMbYIdx, uiMbXIdx, itCompleteChromaDC, iLastQP ) );

                    if (iCycle == 0)
                      iBitsChroma += m_pcSymbolWriter->getNumberOfWrittenBits() - iLastBitsChroma;

                    if( bCheck && m_pcSymbolWriter->getNumberOfWrittenBits() - uiBitsLast >= uiMaxBits )
                    {
                      xStoreFGSState(iLumaScanIdx, iChromaDCScanIdx, iChromaACScanIdx, iStartCycle, iCompleteLuma, iCompleteChromaDC, iCompleteChromaAC, bAllowChromaDC, bAllowChromaAC, iCycle, itCompleteLuma, itCompleteChromaDC, itCompleteChromaAC, uiMbYIdx, uiMbXIdx, 2*uiMbYIdx, 2*uiMbXIdx, 4*uiMbYIdx, 4*uiMbXIdx, iLastBitsLuma, uiBitsLast, 1, uiPlane+1, iLastQP);
                      throw WriteStop();
                    }
                    } else if (iChromaDCScanIdx < 4) {
                    RNOK( xEncodeCoefficientChromaDCRef( uiPlane, uiMbYIdx, uiMbXIdx, iChromaDCScanIdx ) );

                    if( bCheck && m_pcSymbolWriter->getNumberOfWrittenBits() - uiBitsLast >= uiMaxBits )
                    {
                      xStoreFGSState(iLumaScanIdx, iChromaDCScanIdx, iChromaACScanIdx, iStartCycle, iCompleteLuma, iCompleteChromaDC, iCompleteChromaAC, bAllowChromaDC, bAllowChromaAC, iCycle, itCompleteLuma, itCompleteChromaDC, itCompleteChromaAC, uiMbYIdx, uiMbXIdx, 2*uiMbYIdx, 2*uiMbXIdx, 4*uiMbYIdx, 4*uiMbXIdx, iLastBitsLuma, uiBitsLast, 1, uiPlane+1, iLastQP);
                      throw WriteStop();
                    }
                  } // if
                } // for
                m_uiPlane = 0;
              } // if
            }
            m_uiFGSPart = 2;

            // ===== CHROMA AC =====
            if( (m_uiFGSPart == 2) || (!bFirstPass) )
            {
              if( bAllowChromaAC && ( m_bFgsComponentSep == 0 || iLumaScanIdx == 16 ) ) {
                bFirstPass = false;
                for( UInt uiPlane = m_uiPlane; uiPlane  < 2; uiPlane ++ )
                {
                  for( UInt uiB8YIdx = m_uiB8YIdx; uiB8YIdx < 2 * uiMbYIdx + 2; uiB8YIdx++ ) {
                    for( UInt uiB8XIdx = m_uiB8XIdx; uiB8XIdx < 2 * uiMbXIdx + 2; uiB8XIdx++ ) {

                      if ( !iCompleteChromaAC) {
                        RNOK( xEncodeNewCoefficientChromaAC( uiPlane, uiB8YIdx, uiB8XIdx, itCompleteChromaAC, iLastQP ) );

                        if( bCheck && m_pcSymbolWriter->getNumberOfWrittenBits() - uiBitsLast >= uiMaxBits )
                        {
                          xStoreFGSState(iLumaScanIdx, iChromaDCScanIdx, iChromaACScanIdx, iStartCycle, iCompleteLuma, iCompleteChromaDC, iCompleteChromaAC, bAllowChromaDC, bAllowChromaAC, iCycle, itCompleteLuma, itCompleteChromaDC, itCompleteChromaAC, uiMbYIdx, uiMbXIdx, uiB8YIdx, uiB8XIdx+1, 0, 0, iLastBitsLuma, uiBitsLast, 2, uiPlane, iLastQP);
                          throw WriteStop();
                        }
                       } else if (iChromaACScanIdx < 16) {
                        RNOK( xEncodeCoefficientChromaACRef( uiPlane, uiB8YIdx, uiB8XIdx, iChromaACScanIdx ) );
                        if( bCheck && m_pcSymbolWriter->getNumberOfWrittenBits() - uiBitsLast >= uiMaxBits )
                        {
                          xStoreFGSState(iLumaScanIdx, iChromaDCScanIdx, iChromaACScanIdx, iStartCycle, iCompleteLuma, iCompleteChromaDC, iCompleteChromaAC, bAllowChromaDC, bAllowChromaAC, iCycle, itCompleteLuma, itCompleteChromaDC, itCompleteChromaAC, uiMbYIdx, uiMbXIdx, uiB8YIdx, uiB8XIdx+1, 0, 0, iLastBitsLuma, uiBitsLast, 2, uiPlane, iLastQP);
                          throw WriteStop();
                        }
                        } // if
                    } // for
                    m_uiB8XIdx = 2 * uiMbXIdx;
                  }
                  m_uiB8YIdx = 2 * uiMbYIdx;
                }
                m_uiPlane = 0;
              } // if
            }
            m_uiFGSPart = 0;
          } // macroblock iteration
          m_uiMbXIdx = 0;
        }

        m_uiMbYIdx = uiFirstMbY;
        m_uiMbXIdx = ( m_uiMbYIdx == uiFirstMbY ? uiFirstMbX : 0 );
        m_uiB8YIdx = 2*m_uiMbYIdx;
        m_uiB8XIdx = 2*m_uiMbXIdx;
        m_uiBlockYIdx = 2*m_uiB8YIdx;
        m_uiBlockXIdx = 2*m_uiB8XIdx;

        if (iCompleteLuma)
          iLumaScanIdx++;
        if (iCompleteChromaDC && bAllowChromaDC)
          iChromaDCScanIdx++;
        if (iCompleteChromaAC && bAllowChromaAC)
          iChromaACScanIdx++;
        if ( iCycle == 0 && m_bFgsComponentSep == 0 ) {
          UInt ratio = (iBitsChroma > 0) ? (iBitsLuma / iBitsChroma) : 999;
          iStartCycle = 3 - (ratio >= 15) - (ratio >= 30);
          m_pcSymbolWriter->RQencodeCycleSymbol(iStartCycle-1);
        }
        iCompleteLuma     |= itCompleteLuma;
        iCompleteChromaDC |= itCompleteChromaDC;
        iCompleteChromaAC |= itCompleteChromaAC;
        iCycle++;
      } // while
    }
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
        return Err::m_nOK;
    }
  }
  rbFinished      = rbCorrupted || ( m_iRemainingTCoeff == 0 );
  RNOK( m_pcSymbolWriter->finishSlice() );


  RNOK( xUpdateCodingPath() );

  RNOK( xClearCodingPath() );

  return Err::m_nOK;
}

ErrVal
RQFGSEncoder::xEncodeNewCoefficientLuma( UInt   uiBlockYIndex,
                                         UInt   uiBlockXIndex,
                                         UInt&  uiComplete,
                                         Int&   riLastQp )
{
  UInt    uiBlockIndex  =  uiBlockYIndex    * 4 * m_uiWidthInMB +  uiBlockXIndex;
  UInt    uiSubMbIndex  = (uiBlockYIndex/2) * 2 * m_uiWidthInMB + (uiBlockXIndex/2);
  UInt    uiMbIndex     = (uiBlockYIndex/4) * 1 * m_uiWidthInMB + (uiBlockXIndex/4);
  
  UInt    uiB8x8        = ((uiBlockYIndex%4)/2) * 2 + ((uiBlockXIndex%4)/2);
  UInt    uiB4x4        =  (uiBlockYIndex%4)    * 4 +  (uiBlockXIndex%4);
  Par8x8  ePar8x8       = Par8x8(uiB8x8);
  
  B8x8Idx c8x8Idx(ePar8x8);
  B4x4Idx c4x4Idx(uiB4x4);


  MbDataAccess* pcMbDataAccessEL  = 0;
  MbDataAccess* pcMbDataAccessBL  = 0;
  RNOK( m_cMbDataCtrlEL     .initMb( pcMbDataAccessEL, uiBlockYIndex/4, uiBlockXIndex/4 ) );
  RNOK( m_pcCurrMbDataCtrl ->initMb( pcMbDataAccessBL, uiBlockYIndex/4, uiBlockXIndex/4 ) );
  

  if( ! ( m_paucSubMbMap[uiSubMbIndex] & CODED ) )
  {
    //===== CBP =====
    Bool bSigCBP = m_pcSymbolWriter->RQencodeCBP_8x8( *pcMbDataAccessEL, *pcMbDataAccessBL, c8x8Idx );
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

        for( UInt ui = 0; ui < 16; ui++ )
        {
          if( ! ( m_apaucLumaCoefMap[ui][uiBlk] & SIGNIFICANT ) )
          {
            m_apaucLumaCoefMap[ui][uiBlk] |= CODED;
            m_iRemainingTCoeff--;
            m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
            ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );
          }
        }
      }
      TWSOT( m_iRemainingTCoeff < 0 );
      return Err::m_nOK;
    }
    
    //===== DELTA QP & TRANSFORM SIZE =====
    if( ! ( m_pauiMacroblockMap[uiMbIndex] & CODED ) )
    {
      //===== delta QP =====
      if( ! ( m_pauiMacroblockMap[uiMbIndex] & SIGNIFICANT ) )
      {
        pcMbDataAccessEL->setLastQp( riLastQp );
        RNOK( m_pcSymbolWriter->RQencodeDeltaQp( *pcMbDataAccessEL, *pcMbDataAccessBL ) );
        riLastQp = pcMbDataAccessEL->getMbData().getQp();

        m_pauiMacroblockMap[uiMbIndex] |= SIGNIFICANT;
      }

      //===== transform size =====
      if( ! ( m_pauiMacroblockMap[uiMbIndex] & TRANSFORM_SPECIFIED ) )
      {
        RNOK( m_pcSymbolWriter->RQencode8x8Flag( *pcMbDataAccessEL, *pcMbDataAccessBL ) );

        m_pauiMacroblockMap[uiMbIndex] |= TRANSFORM_SPECIFIED;
      }

      m_pauiMacroblockMap[uiMbIndex] |= CODED;
    }
  }


  if( pcMbDataAccessBL->getMbData().isTransformSize8x8() )
  {

    UInt auiBlockIdx[4] = { ( (uiBlockYIndex/4)*4 + c8x8Idx.y()     ) * 4 * m_uiWidthInMB + ( (uiBlockXIndex/4)*4 + c8x8Idx.x()     ),
                            ( (uiBlockYIndex/4)*4 + c8x8Idx.y()     ) * 4 * m_uiWidthInMB + ( (uiBlockXIndex/4)*4 + c8x8Idx.x() + 1 ),
                            ( (uiBlockYIndex/4)*4 + c8x8Idx.y() + 1 ) * 4 * m_uiWidthInMB + ( (uiBlockXIndex/4)*4 + c8x8Idx.x()     ),
                            ( (uiBlockYIndex/4)*4 + c8x8Idx.y() + 1 ) * 4 * m_uiWidthInMB + ( (uiBlockXIndex/4)*4 + c8x8Idx.x() + 1 ) };


    UInt    uiStop          = 64;
    UInt    ui8x8ScanIndex  = uiStop;
    UInt    ui8x8StartIndex = uiStop;
    UInt    ui8x8Index;
    for( ui8x8Index = 0; ui8x8Index < uiStop; ui8x8Index++ )
    {
      UInt  uiS = ui8x8Index / 4;
      UInt  uiB = auiBlockIdx[ui8x8Index%4];
      if( !( m_apaucLumaCoefMap[uiS][uiB] & SIGNIFICANT ) || ( m_apaucLumaCoefMap[uiS][uiB] & CODED ) )
      {
        ui8x8StartIndex = ui8x8Index;
        break;
      }
    }
    for( ui8x8Index = 0; ui8x8Index < uiStop; ui8x8Index++ )
    {
      UInt  uiS = ui8x8Index / 4;
      UInt  uiB = auiBlockIdx[ui8x8Index%4];
      if( !( m_apaucLumaCoefMap[uiS][uiB] & (SIGNIFICANT|CODED) ) )
      {
        ui8x8ScanIndex  = ui8x8Index;
        break;
      }
    }
    uiComplete &= (ui8x8ScanIndex == uiStop);
    ROTRS( ui8x8ScanIndex == uiStop, Err::m_nOK );

    UInt uiNeedEob = ( ui8x8ScanIndex > ui8x8StartIndex );

    while( ui8x8ScanIndex < 64 )
    {
      RNOK( m_pcSymbolWriter->RQencodeNewTCoeff_8x8( *pcMbDataAccessEL, *pcMbDataAccessBL,
                                                    c8x8Idx, ui8x8ScanIndex, uiNeedEob ) );
      if( uiNeedEob )
      {
        //===== end-of-block =====
        for( UInt ui8x8 = ui8x8ScanIndex; ui8x8 < 64; ui8x8++ )
        {
          UInt  uiS = ui8x8/4;
          UInt  uiB = auiBlockIdx[ui8x8%4];
          if( !( m_apaucLumaCoefMap[uiS][uiB] & SIGNIFICANT ) )
          {
            m_apaucLumaCoefMap[uiS][uiB]   |= CODED;
            m_iRemainingTCoeff--;
            m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
            ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );
          }
        }
        break;
      }
      else
      {
        //===== coefficient =====
        uiNeedEob = 0;
        UInt  uiS = ui8x8ScanIndex/4;
        UInt  uiB = auiBlockIdx[ui8x8ScanIndex%4];

        m_apaucLumaCoefMap[uiS][uiB] |= CODED;
        if( pcMbDataAccessEL->getMbTCoeffs().get8x8( c8x8Idx )[ g_aucFrameScan64[ui8x8ScanIndex] ] )
        {
          m_apaucLumaCoefMap[uiS][uiB] |= SIGNIFICANT;
        }
        m_iRemainingTCoeff--;
        m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
        ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );

        if( m_apaucLumaCoefMap[uiS][uiB] & SIGNIFICANT )
        {
          break;
        }

        ui8x8ScanIndex++;
        while( ( ui8x8ScanIndex < 64 ) && ( m_apaucLumaCoefMap[ui8x8ScanIndex/4][auiBlockIdx[ui8x8ScanIndex%4]] & SIGNIFICANT ) )
        {
          ui8x8ScanIndex++;
        }
      }
    }

    TWSOT( m_iRemainingTCoeff < 0 );
  }
  else
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
            Bool bSigBCBP = m_pcSymbolWriter->RQencodeBCBP_4x4( *pcMbDataAccessEL, *pcMbDataAccessBL, c4x4Tmp );
            m_paucBlockMap[uiBlk] |= CODED;
            if(  bSigBCBP )
            {
              m_paucBlockMap[uiBlk] |= SIGNIFICANT;
            }
            if( ! bSigBCBP )
            {
              for( UInt ui = 0; ui < 16; ui++ )
              {
                if( ! ( m_apaucLumaCoefMap[ui][uiBlk] & SIGNIFICANT ) )
                {
                  m_apaucLumaCoefMap[ui][uiBlk] |= CODED;
                  m_iRemainingTCoeff--;
                  m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
                  ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );
                }
              }
              TWSOT( m_iRemainingTCoeff < 0 );
            }
          }
        }
      }
    if (! m_pcSymbolWriter->RQpeekCbp4x4( *pcMbDataAccessEL, *pcMbDataAccessBL, c4x4Idx ) )
    {
      return Err::m_nOK;
    }

    UInt    uiStop       = 16;
    UInt    uiScanIndex  = uiStop;
    UInt    uiStartIndex = uiStop;
    UInt    uiIndex;
    for( uiIndex = 0; uiIndex < uiStop; uiIndex++ )
    {
      if( ! ( m_apaucLumaCoefMap[uiIndex][uiBlockIndex] & SIGNIFICANT ) || ( m_apaucLumaCoefMap[uiIndex][uiBlockIndex] & CODED ) )
      {
        uiStartIndex = uiIndex;
        break;
      }
    }
    for( uiIndex = 0; uiIndex < uiStop; uiIndex++ )
    {
      if( !( m_apaucLumaCoefMap[uiIndex][uiBlockIndex] & (SIGNIFICANT|CODED) ) )
      {
        uiScanIndex = uiIndex;
        break;
      }
    }
    uiComplete &= (uiScanIndex == uiStop);
    ROTRS(uiScanIndex == uiStop, Err::m_nOK);

    Bool bNeedEob = ( uiScanIndex > uiStartIndex );
    UInt uiNumCoefWritten;
    RNOK( m_pcSymbolWriter->RQencodeNewTCoeff_Luma( *pcMbDataAccessEL, *pcMbDataAccessBL,
                                                  LUMA_SCAN, c4x4Idx, uiScanIndex, bNeedEob, uiNumCoefWritten ) );

    for ( UInt ui = 0; uiScanIndex < 16 && ( ui < uiNumCoefWritten || bNeedEob ); uiScanIndex++ )
    {
      if( ! ( m_apaucLumaCoefMap[uiScanIndex][uiBlockIndex] & SIGNIFICANT ) )
      {
        if( pcMbDataAccessEL->getMbTCoeffs().get( c4x4Idx )[g_aucFrameScan[uiScanIndex]] )
        {
          m_apaucLumaCoefMap[uiScanIndex][uiBlockIndex] |= SIGNIFICANT;
        }
        m_apaucLumaCoefMap[uiScanIndex][uiBlockIndex] |= CODED;
        m_iRemainingTCoeff--;
        m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
        ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );
        ui++;
      }
    }
    TWSOT( m_iRemainingTCoeff < 0 );
  }
  
  return Err::m_nOK;
}



ErrVal
RQFGSEncoder::xEncodeNewCoefficientChromaDC ( UInt    uiPlane,
                                              UInt    uiMbYIdx,
                                              UInt    uiMbXIdx,
                                              UInt&   uiComplete,
                                              Int&    riLastQP )
{
  UInt uiMbIndex = uiMbYIdx * m_uiWidthInMB + uiMbXIdx;

  UInt    uiDCIdx    = 4;
  UInt    uiStartIdx = 4;
  UInt    uiIndex;
  for( uiIndex = 0; uiIndex < 4; uiIndex++ )
  {
    if( ! ( m_aapaucChromaDCCoefMap[uiPlane][uiIndex][uiMbIndex] & SIGNIFICANT) || ( m_aapaucChromaDCCoefMap[uiPlane][uiIndex][uiMbIndex] & CODED ) )
    {
      uiStartIdx = uiIndex;
      break;
    }
  }
  for( uiIndex = 0; uiIndex < 4; uiIndex++ )
  {
    if( ! ( m_aapaucChromaDCCoefMap[uiPlane][uiIndex][uiMbIndex] & (SIGNIFICANT|CODED) ) )
    {
      uiDCIdx = uiIndex;
      break;
    }
  }
  uiComplete &= (uiDCIdx == 4);
  ROTRS(uiDCIdx == 4, Err::m_nOK);

  Bool bNeedEob = ( uiDCIdx > uiStartIdx );

  MbDataAccess* pcMbDataAccessEL  = 0;
  MbDataAccess* pcMbDataAccessBL  = 0;
  RNOK( m_cMbDataCtrlEL     .initMb( pcMbDataAccessEL, uiMbYIdx, uiMbXIdx ) );
  RNOK( m_pcCurrMbDataCtrl ->initMb( pcMbDataAccessBL, uiMbYIdx, uiMbXIdx ) );
  

  if( ! ( m_pauiMacroblockMap[uiMbIndex] & CHROMA_CBP_CODED ) )
  {
    //===== CHROMA CBP =====
    Bool  bSigCBP = m_pcSymbolWriter->RQencodeCBP_Chroma( *pcMbDataAccessEL, *pcMbDataAccessBL );
    m_pauiMacroblockMap[uiMbIndex] |= CHROMA_CBP_CODED;
    
    if( !bSigCBP )
    {
      //----- chroma DC -----
      for( UInt uiCP = 0; uiCP < 2; uiCP++ )
      {
        m_apaucChromaDCBlockMap[uiCP][uiMbIndex] |= CODED;

        for( UInt ui = 0; ui < 4; ui++ )
        {
          if( ! ( m_aapaucChromaDCCoefMap[uiCP][ui][uiMbIndex] & SIGNIFICANT ) )
          {
            m_aapaucChromaDCCoefMap[uiCP][ui][uiMbIndex] |= CODED;
            m_iRemainingTCoeff--;
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

        for( UInt ui = 1; ui < 16; ui++ )
        {
          if( ! ( m_aapaucChromaACCoefMap[uiCP][ui][uiBlk] & SIGNIFICANT ) )
          {
            m_aapaucChromaACCoefMap[uiCP][ui][uiBlk] |= CODED;
            m_iRemainingTCoeff--;
            m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
            ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );
          }
        }
      }

      TWSOT( m_iRemainingTCoeff < 0 );
      return Err::m_nOK;
    }
    
    //===== DELTA QP =====
    if( ! ( m_pauiMacroblockMap[uiMbIndex] & CODED ) )
    {
      //===== delta QP =====
      if( ! ( m_pauiMacroblockMap[uiMbIndex] & SIGNIFICANT ) )
      {
        pcMbDataAccessEL->setLastQp( riLastQP );
        RNOK( m_pcSymbolWriter->RQencodeDeltaQp( *pcMbDataAccessEL, *pcMbDataAccessBL ) );
        riLastQP = pcMbDataAccessEL->getMbData().getQp();

        m_pauiMacroblockMap[uiMbIndex] |= SIGNIFICANT;
      }
    }
  }



  if( ! ( m_apaucChromaDCBlockMap[uiPlane][uiMbIndex] & CODED ) )
  {
    Bool bSigBCBP = m_pcSymbolWriter->RQencodeBCBP_ChromaDC( *pcMbDataAccessEL, *pcMbDataAccessBL, CIdx(4*uiPlane) );
    m_apaucChromaDCBlockMap[uiPlane][uiMbIndex] |= CODED;
    if(  bSigBCBP )
    {
      m_apaucChromaDCBlockMap[uiPlane][uiMbIndex] |= SIGNIFICANT;
    }
    if( ! bSigBCBP )
    {
      for( UInt ui = 0; ui < 4; ui++ )
      {
        if( ! ( m_aapaucChromaDCCoefMap[uiPlane][ui][uiMbIndex] & SIGNIFICANT ) )
        {
          m_aapaucChromaDCCoefMap[uiPlane][ui][uiMbIndex] |= CODED;
          m_iRemainingTCoeff--;
          m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
          ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );
        }
      }
      TWSOT( m_iRemainingTCoeff < 0 );
      return Err::m_nOK;
    }
  }


  UInt uiNumCoefWritten;
  RNOK( m_pcSymbolWriter->RQencodeNewTCoeff_Chroma( *pcMbDataAccessEL, *pcMbDataAccessBL,
                                                  CHROMA_DC, CIdx(4*uiPlane), uiDCIdx, bNeedEob, uiNumCoefWritten ) );
  for ( UInt ui = 0; uiDCIdx < 4 && ( ui < uiNumCoefWritten || bNeedEob ); uiDCIdx++ )
  {
    if( ! ( m_aapaucChromaDCCoefMap[uiPlane][uiDCIdx][uiMbIndex] & SIGNIFICANT ) )
    {
      if( pcMbDataAccessEL->getMbTCoeffs().get( CIdx(4*uiPlane) )[g_aucIndexChromaDCScan[uiDCIdx]] )
      {
        m_aapaucChromaDCCoefMap[uiPlane][uiDCIdx][uiMbIndex] |= SIGNIFICANT;
      }
      m_aapaucChromaDCCoefMap[uiPlane][uiDCIdx][uiMbIndex] |= CODED;
      m_iRemainingTCoeff--;
      m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
      ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );
      ui++;
    }
  }
  TWSOT( m_iRemainingTCoeff < 0 );
  
  return Err::m_nOK;
}






ErrVal
RQFGSEncoder::xEncodeNewCoefficientChromaAC ( UInt    uiPlane,
                                              UInt    uiB8YIdx,
                                              UInt    uiB8XIdx,
                                              UInt&   uiComplete,
                                              Int&    riLastQP )
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
    if( !( m_aapaucChromaACCoefMap[uiPlane][uiIndex][uiB8Index] & SIGNIFICANT ) || ( m_aapaucChromaACCoefMap[uiPlane][uiIndex][uiB8Index] & CODED ) )
    {
      uiStartIndex = uiIndex;
      break;
    }
  }
  for( uiIndex = 1; uiIndex < 16; uiIndex++ )
  {
    if( !( m_aapaucChromaACCoefMap[uiPlane][uiIndex][uiB8Index] & (SIGNIFICANT|CODED) ) )
    {
      uiScanIndex = uiIndex;
      break;
    }
  }
  uiComplete &= (uiScanIndex == 16);
  ROTRS(uiScanIndex == 16, Err::m_nOK);

  Bool bNeedEob = (uiScanIndex > uiStartIndex );

  MbDataAccess* pcMbDataAccessEL  = 0;
  MbDataAccess* pcMbDataAccessBL  = 0;
  RNOK( m_cMbDataCtrlEL     .initMb( pcMbDataAccessEL, uiB8YIdx/2, uiB8XIdx/2 ) );
  RNOK( m_pcCurrMbDataCtrl ->initMb( pcMbDataAccessBL, uiB8YIdx/2, uiB8XIdx/2 ) );
  

  if( ! ( m_pauiMacroblockMap[uiMbIndex] & CHROMA_CBP_CODED ) )
  {
    //===== CHROMA CBP =====
    Bool  bSigCBP = m_pcSymbolWriter->RQencodeCBP_Chroma( *pcMbDataAccessEL, *pcMbDataAccessBL );
    m_pauiMacroblockMap[uiMbIndex] |= CHROMA_CBP_CODED;
    
    if( !bSigCBP )
    {
      //----- chroma DC -----
      for( UInt uiCP = 0; uiCP < 2; uiCP++ )
      {
        m_apaucChromaDCBlockMap[uiCP][uiMbIndex] |= CODED;

        for( UInt ui = 0; ui < 4; ui++ )
        {
          if( ! ( m_aapaucChromaDCCoefMap[uiCP][ui][uiMbIndex] & SIGNIFICANT ) )
          {
            m_aapaucChromaDCCoefMap[uiCP][ui][uiMbIndex] |= CODED;
            m_iRemainingTCoeff--;
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

        for( UInt ui = 1; ui < 16; ui++ )
        {
          if( ! ( m_aapaucChromaACCoefMap[uiCP][ui][uiBlk] & SIGNIFICANT ) )
          {
            m_aapaucChromaACCoefMap[uiCP][ui][uiBlk] |= CODED;
            m_iRemainingTCoeff--;
            m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
            ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );
          }
        }
      }

      TWSOT( m_iRemainingTCoeff < 0 );
      return Err::m_nOK;
    }
    
    //===== DELTA QP =====
    if( ! ( m_pauiMacroblockMap[uiMbIndex] & CODED ) )
    {
      //===== delta QP =====
      if( ! ( m_pauiMacroblockMap[uiMbIndex] & SIGNIFICANT ) )
      {
        pcMbDataAccessEL->setLastQp( riLastQP );
        RNOK( m_pcSymbolWriter->RQencodeDeltaQp( *pcMbDataAccessEL, *pcMbDataAccessBL ) );
        riLastQP = pcMbDataAccessEL->getMbData().getQp();

        m_pauiMacroblockMap[uiMbIndex] |= SIGNIFICANT;
      }
    }
  }



  if( ! ( m_pauiMacroblockMap[uiMbIndex] & CHROMA_CBP_AC_CODED ) )
  {
    //===== CHROMA CBP =====
    Bool  bSigCBP = m_pcSymbolWriter->RQencodeCBP_ChromaAC( *pcMbDataAccessEL, *pcMbDataAccessBL );
    m_pauiMacroblockMap[uiMbIndex] |= CHROMA_CBP_AC_CODED;
    
    if( !bSigCBP )
    {
      for( CIdx cCIdx; cCIdx.isLegal(); cCIdx++ )
      {
        UInt  uiCP  = cCIdx.plane();
        UInt  uiBlk = ( (uiB8YIdx/2)*2 + cCIdx.y() ) * 2 * m_uiWidthInMB + ( (uiB8XIdx/2)*2 + cCIdx.x() );

        m_apaucChromaACBlockMap[uiCP][uiBlk] |= CODED;

        for( UInt ui = 1; ui < 16; ui++ )
        {
          if( ! ( m_aapaucChromaACCoefMap[uiCP][ui][uiBlk] & SIGNIFICANT ) )
          {
            m_aapaucChromaACCoefMap[uiCP][ui][uiBlk] |= CODED;
            m_iRemainingTCoeff--;
            m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
            ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );
          }
        }
      }

      TWSOT( m_iRemainingTCoeff < 0 );
      return Err::m_nOK;
    }
  }



  if( ! ( m_apaucChromaACBlockMap[uiPlane][uiB8Index] & CODED ) )
  {
    Bool bSigBCBP = m_pcSymbolWriter->RQencodeBCBP_ChromaAC( *pcMbDataAccessEL, *pcMbDataAccessBL, cChromaIdx );
    m_apaucChromaACBlockMap[uiPlane][uiB8Index] |= CODED;
    if(  bSigBCBP )
    {
      m_apaucChromaACBlockMap[uiPlane][uiB8Index] |= SIGNIFICANT;
    }
    if( ! bSigBCBP )
    {
      for( UInt ui = 1; ui < 16; ui++ )
      {
        if( ! ( m_aapaucChromaACCoefMap[uiPlane][ui][uiB8Index] & SIGNIFICANT ) )
        {
          m_aapaucChromaACCoefMap[uiPlane][ui][uiB8Index] |= CODED;
          m_iRemainingTCoeff--;
          m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
          ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );
        }
      }
      TWSOT( m_iRemainingTCoeff < 0 );
      return Err::m_nOK;
    }
  }


  UInt uiNumCoefWritten;
  RNOK( m_pcSymbolWriter->RQencodeNewTCoeff_Chroma( *pcMbDataAccessEL, *pcMbDataAccessBL,
                                                  CHROMA_AC, cChromaIdx, uiScanIndex, bNeedEob, uiNumCoefWritten ) );

  for ( UInt ui = 0; uiScanIndex < 16 && ( ui < uiNumCoefWritten || bNeedEob ); uiScanIndex++ )
  {
    if( ! ( m_aapaucChromaACCoefMap[uiPlane][uiScanIndex][uiB8Index] & SIGNIFICANT ) )
    {
      if( pcMbDataAccessEL->getMbTCoeffs().get( cChromaIdx )[g_aucFrameScan[uiScanIndex]] )
      {
        m_aapaucChromaACCoefMap[uiPlane][uiScanIndex][uiB8Index] |= SIGNIFICANT;
      }
      m_aapaucChromaACCoefMap[uiPlane][uiScanIndex][uiB8Index] |= CODED;
      m_iRemainingTCoeff--;
      m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
      ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );
      ui++;
    }
  }
  TWSOT( m_iRemainingTCoeff < 0 );
  
  return Err::m_nOK;
}







ErrVal
RQFGSEncoder::xEncodeCoefficientLumaRef( UInt   uiBlockYIndex,
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
    RNOK( m_pcSymbolWriter->RQencodeTCoeffRef_8x8( *pcMbDataAccessEL, *pcMbDataAccessBL,
                                                  c8x8Idx, ui8x8ScanIndex ) );    
  }
  else
  {
    // Count number of remaining values
    UInt uiNumSig = 0;
    for( UInt ui=uiScanIndex; ui<16; ui++ )
    {
      if( (m_apaucLumaCoefMap[ui][uiBlockIndex] & (SIGNIFICANT|CODED)) == SIGNIFICANT )
        uiNumSig++;
    }
    RNOK( m_pcSymbolWriter->RQencodeTCoeffRef_Luma( *pcMbDataAccessEL, *pcMbDataAccessBL,
                                                   LUMA_SCAN, c4x4Idx, uiScanIndex, uiNumSig ) );
  }

  m_apaucLumaCoefMap[uiScanIndex][uiBlockIndex] |= CODED;
  m_iRemainingTCoeff--;
  m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
  ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );
  TWSOT( m_iRemainingTCoeff < 0 );

  if( m_pcSymbolWriter == m_pcControlMng->getUvlcWriter() )
    for( UInt ui=uiScanIndex+1; ui<16; ui++ )
    {
      if( (m_apaucLumaCoefMap[ui][uiBlockIndex] & (SIGNIFICANT|CODED)) == SIGNIFICANT )
      {
        RNOK( xEncodeCoefficientLumaRef( uiBlockYIndex, uiBlockXIndex, ui ) );
        break;
      }
    }

  return Err::m_nOK;
}


ErrVal
RQFGSEncoder::xEncodeCoefficientChromaDCRef ( UInt  uiPlane,
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
  

  // Count number of remaining values
  UInt uiNumSig = 0;
  for( UInt ui=uiDCIdx; ui<4; ui++ )
  {
    if( (m_aapaucChromaDCCoefMap[uiPlane][ui][uiMbIndex] & (SIGNIFICANT|CODED)) == SIGNIFICANT )
      uiNumSig++;
  }
  RNOK( m_pcSymbolWriter->RQencodeTCoeffRef_Chroma( *pcMbDataAccessEL, *pcMbDataAccessBL,
                                                   CHROMA_DC, CIdx(4*uiPlane), uiDCIdx, uiNumSig ) );

  m_aapaucChromaDCCoefMap[uiPlane][uiDCIdx][uiMbIndex] |= CODED;
  m_iRemainingTCoeff--;
  m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
  ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );
  TWSOT( m_iRemainingTCoeff < 0 );
  if( m_pcSymbolWriter == m_pcControlMng->getUvlcWriter() )
    for( UInt ui=uiDCIdx+1; ui<4; ui++ )
    {
      if( (m_aapaucChromaDCCoefMap[uiPlane][ui][uiMbIndex] & (SIGNIFICANT|CODED)) == SIGNIFICANT )
      {
        RNOK( xEncodeCoefficientChromaDCRef( uiPlane, uiMbYIdx, uiMbXIdx, ui ) );
        break;
      }
    }

  return Err::m_nOK;
}
 

ErrVal
RQFGSEncoder::xEncodeCoefficientChromaACRef ( UInt  uiPlane,
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
  

  // Count number of remaining values
  UInt uiNumSig = 0;
  for( UInt ui=uiScanIdx; ui<16; ui++ )
  {
    if( (m_aapaucChromaACCoefMap[uiPlane][ui][uiB8Index] & (SIGNIFICANT|CODED)) == SIGNIFICANT )
      uiNumSig++;
  }
  RNOK( m_pcSymbolWriter->RQencodeTCoeffRef_Chroma( *pcMbDataAccessEL, *pcMbDataAccessBL,
                                                   CHROMA_AC, cChromaIdx, uiScanIdx, uiNumSig ) );

  m_aapaucChromaACCoefMap[uiPlane][uiScanIdx][uiB8Index] |= CODED;
  m_iRemainingTCoeff--;
  m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
  ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );
  TWSOT( m_iRemainingTCoeff < 0 );

  if( m_pcSymbolWriter == m_pcControlMng->getUvlcWriter() )
    for( UInt ui=uiScanIdx+1; ui<16; ui++ )
    {
      if( (m_aapaucChromaACCoefMap[uiPlane][ui][uiB8Index] & (SIGNIFICANT|CODED)) == SIGNIFICANT )
      {
        RNOK( xEncodeCoefficientChromaACRef( uiPlane, uiB8YIdx, uiB8XIdx, ui ) );
        break;
      }
    }

  return Err::m_nOK;
}






UInt
RQFGSEncoder::xGetSigCtxLuma( UInt  uiScanIdx,
                              UInt  uiBlockY,
                              UInt  uiBlockX )
{
  UInt  uiNumSigDiag  = 0;
  UInt  uiNumSigNext  = 0;
  if( uiBlockX > 0 && uiBlockY > 0 )
  {
    uiNumSigDiag += ( m_apaucLumaCoefMap[uiScanIdx][(uiBlockY-1)*4*m_uiWidthInMB+(uiBlockX-1)] & SIGNIFICANT ? 1 : 0 );
  }
  if( uiBlockX > 0 && uiBlockY < 4*m_uiHeightInMB-1 )
  {
    uiNumSigDiag += ( m_apaucLumaCoefMap[uiScanIdx][(uiBlockY+1)*4*m_uiWidthInMB+(uiBlockX-1)] & SIGNIFICANT ? 1 : 0 );
  }
  if( uiBlockX < 4*m_uiWidthInMB-1 && uiBlockY < 4*m_uiHeightInMB-1 )
  {
    uiNumSigDiag += ( m_apaucLumaCoefMap[uiScanIdx][(uiBlockY+1)*4*m_uiWidthInMB+(uiBlockX+1)] & SIGNIFICANT ? 1 : 0 );
  }
  if( uiBlockX < 4*m_uiWidthInMB-1 && uiBlockY > 0 )
  {
    uiNumSigDiag += ( m_apaucLumaCoefMap[uiScanIdx][(uiBlockY-1)*4*m_uiWidthInMB+(uiBlockX+1)] & SIGNIFICANT ? 1 : 0 );
  }
  if( uiBlockX > 0 )
  {
    uiNumSigNext += ( m_apaucLumaCoefMap[uiScanIdx][(uiBlockY)*4*m_uiWidthInMB+(uiBlockX-1)] & SIGNIFICANT ? 1 : 0 );
  }
  if( uiBlockX < 4*m_uiWidthInMB-1 )
  {
    uiNumSigNext += ( m_apaucLumaCoefMap[uiScanIdx][(uiBlockY)*4*m_uiWidthInMB+(uiBlockX+1)] & SIGNIFICANT ? 1 : 0 );
  }
  if( uiBlockY > 0 )
  {
    uiNumSigNext += ( m_apaucLumaCoefMap[uiScanIdx][(uiBlockY-1)*4*m_uiWidthInMB+(uiBlockX)] & SIGNIFICANT ? 1 : 0 );
  }
  if( uiBlockY < 4*m_uiHeightInMB-1 )
  {
    uiNumSigNext += ( m_apaucLumaCoefMap[uiScanIdx][(uiBlockY+1)*4*m_uiWidthInMB+(uiBlockX)] & SIGNIFICANT ? 1 : 0 );
  }
  return ( 2*uiNumSigDiag + 3*uiNumSigNext + 9 ) / 10;
}
 
UInt
RQFGSEncoder::xGetSigCtxChromaDC( UInt  uiPlane,
                                  UInt  uiScanIdx,
                                  UInt  uiMbY,
                                  UInt  uiMbX )
{
  UInt  uiNumSigDiag  = 0;
  UInt  uiNumSigNext  = 0;
  if( uiMbX > 0 && uiMbY > 0 )
  {
    uiNumSigDiag += ( m_aapaucChromaDCCoefMap[uiPlane][uiScanIdx][(uiMbY-1)*m_uiWidthInMB+(uiMbX-1)] & SIGNIFICANT ? 1 : 0 );
  }
  if( uiMbX > 0 && uiMbY < m_uiHeightInMB-1 )
  {
    uiNumSigDiag += ( m_aapaucChromaDCCoefMap[uiPlane][uiScanIdx][(uiMbY+1)*m_uiWidthInMB+(uiMbX-1)] & SIGNIFICANT ? 1 : 0 );
  }
  if( uiMbX < m_uiWidthInMB-1 && uiMbY < m_uiHeightInMB-1 )
  {
    uiNumSigDiag += ( m_aapaucChromaDCCoefMap[uiPlane][uiScanIdx][(uiMbY+1)*m_uiWidthInMB+(uiMbX+1)] & SIGNIFICANT ? 1 : 0 );
  }
  if( uiMbX < m_uiWidthInMB-1 && uiMbY > 0 )
  {
    uiNumSigDiag += ( m_aapaucChromaDCCoefMap[uiPlane][uiScanIdx][(uiMbY-1)*m_uiWidthInMB+(uiMbX+1)] & SIGNIFICANT ? 1 : 0 );
  }
  if( uiMbX > 0 )
  {
    uiNumSigNext += ( m_aapaucChromaDCCoefMap[uiPlane][uiScanIdx][(uiMbY)*m_uiWidthInMB+(uiMbX-1)] & SIGNIFICANT ? 1 : 0 );
  }
  if( uiMbX < m_uiWidthInMB-1 )
  {
    uiNumSigNext += ( m_aapaucChromaDCCoefMap[uiPlane][uiScanIdx][(uiMbY)*m_uiWidthInMB+(uiMbX+1)] & SIGNIFICANT ? 1 : 0 );
  }
  if( uiMbY > 0 )
  {
    uiNumSigNext += ( m_aapaucChromaDCCoefMap[uiPlane][uiScanIdx][(uiMbY-1)*m_uiWidthInMB+(uiMbX)] & SIGNIFICANT ? 1 : 0 );
  }
  if( uiMbY < m_uiHeightInMB-1 )
  {
    uiNumSigNext += ( m_aapaucChromaDCCoefMap[uiPlane][uiScanIdx][(uiMbY+1)*m_uiWidthInMB+(uiMbX)] & SIGNIFICANT ? 1 : 0 );
  }
  return 3 + ( 2*uiNumSigDiag + 3*uiNumSigNext + 9 ) / 10;
}
 
UInt
RQFGSEncoder::xGetSigCtxChromaAC( UInt  uiPlane,
                                  UInt  uiScanIdx,
                                  UInt  uiB8Y,
                                  UInt  uiB8X )
{
  UInt  uiNumSigDiag  = 0;
  UInt  uiNumSigNext  = 0;
  if( uiB8X > 0 && uiB8Y > 0 )
  {
    uiNumSigDiag += ( m_aapaucChromaACCoefMap[uiPlane][uiScanIdx][(uiB8Y-1)*2*m_uiWidthInMB+(uiB8X-1)] & SIGNIFICANT ? 1 : 0 );
  }
  if( uiB8X > 0 && uiB8Y < 2*m_uiHeightInMB-1 )
  {
    uiNumSigDiag += ( m_aapaucChromaACCoefMap[uiPlane][uiScanIdx][(uiB8Y+1)*2*m_uiWidthInMB+(uiB8X-1)] & SIGNIFICANT ? 1 : 0 );
  }
  if( uiB8X < 2*m_uiWidthInMB-1 && uiB8Y < 2*m_uiHeightInMB-1 )
  {
    uiNumSigDiag += ( m_aapaucChromaACCoefMap[uiPlane][uiScanIdx][(uiB8Y+1)*2*m_uiWidthInMB+(uiB8X+1)] & SIGNIFICANT ? 1 : 0 );
  }
  if( uiB8X < 2*m_uiWidthInMB-1 && uiB8Y > 0 )
  {
    uiNumSigDiag += ( m_aapaucChromaACCoefMap[uiPlane][uiScanIdx][(uiB8Y-1)*2*m_uiWidthInMB+(uiB8X+1)] & SIGNIFICANT ? 1 : 0 );
  }
  if( uiB8X > 0 )
  {
    uiNumSigNext += ( m_aapaucChromaACCoefMap[uiPlane][uiScanIdx][(uiB8Y)*2*m_uiWidthInMB+(uiB8X-1)] & SIGNIFICANT ? 1 : 0 );
  }
  if( uiB8X < 2*m_uiWidthInMB-1 )
  {
    uiNumSigNext += ( m_aapaucChromaACCoefMap[uiPlane][uiScanIdx][(uiB8Y)*2*m_uiWidthInMB+(uiB8X+1)] & SIGNIFICANT ? 1 : 0 );
  }
  if( uiB8Y > 0 )
  {
    uiNumSigNext += ( m_aapaucChromaACCoefMap[uiPlane][uiScanIdx][(uiB8Y-1)*2*m_uiWidthInMB+(uiB8X)] & SIGNIFICANT ? 1 : 0 );
  }
  if( uiB8Y < 2*m_uiHeightInMB-1 )
  {
    uiNumSigNext += ( m_aapaucChromaACCoefMap[uiPlane][uiScanIdx][(uiB8Y+1)*2*m_uiWidthInMB+(uiB8X)] & SIGNIFICANT ? 1 : 0 );
  }
  return 6 + ( 2*uiNumSigDiag + 3*uiNumSigNext + 9 ) / 10;
}

ErrVal
RQFGSEncoder::xVLCParseLuma( UInt   uiBlockYIndex,
                             UInt   uiBlockXIndex,
                             UInt*  pauiNumCoefHist,
                             UInt*  pauiHighMagHist)
{
  UInt    uiBlockIndex  =  uiBlockYIndex    * 4 * m_uiWidthInMB +  uiBlockXIndex;
  UInt    uiSubMbIndex  = (uiBlockYIndex/2) * 2 * m_uiWidthInMB + (uiBlockXIndex/2);
  UInt    uiMbIndex     = (uiBlockYIndex/4) * 1 * m_uiWidthInMB + (uiBlockXIndex/4);
  
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
    const UChar*  pucScan        = g_aucFrameScan;

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
                                 UInt*  pauiNumCoefHist)
{
  UInt    uiBlockIndex  =  uiBlockYIndex    * 2 * m_uiWidthInMB +  uiBlockXIndex;
  UInt    uiMbIndex     = (uiBlockYIndex/2) * 1 * m_uiWidthInMB + (uiBlockXIndex/2);
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
  const UChar*  pucScan     = g_aucFrameScan;

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

