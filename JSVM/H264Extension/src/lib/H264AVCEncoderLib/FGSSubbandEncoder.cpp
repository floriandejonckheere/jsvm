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
: m_bInit                     ( false )
, m_papcYuvFullPelBufferCtrl  ( 0 )
, m_pcTransform               ( 0 )
, m_pcCabacWriter             ( 0 )
, m_pcMbEncoder               ( 0 )
, m_bPicInit                  ( false )
, m_uiWidthInMB               ( 0 )
, m_uiHeightInMB              ( 0 )
, m_iRemainingTCoeff          ( 0 )
, m_pcSliceHeader             ( 0 )
, m_pcCurrMbDataCtrl          ( 0 )
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
                    CabacWriter*    pcCabacWriter,
                    MbEncoder*      pcMbEncoder )
{
  ROT( m_bInit );
  ROF( apcYuvFullPelBufferCtrl );
  ROF( pcTransform );
  ROF( pcCabacWriter );
  ROF( pcMbEncoder );

  m_papcYuvFullPelBufferCtrl  = apcYuvFullPelBufferCtrl;
  m_pcTransform               = pcTransform;
  m_pcCabacWriter             = pcCabacWriter;
  m_pcMbEncoder               = pcMbEncoder;
  m_bInit                     = true;

  m_bPicInit                  = false;
  m_uiWidthInMB               = 0;
  m_uiHeightInMB              = 0;
  m_iRemainingTCoeff          = 0;
  m_pcSliceHeader             = 0;
  m_pcCurrMbDataCtrl          = 0;
  m_pcOrgResidual             = 0;

  ::memset( m_apaucLumaCoefMap,       0x00,   16*sizeof(UChar*) );
  ::memset( m_aapaucChromaDCCoefMap,  0x00, 2* 4*sizeof(UChar*) );
  ::memset( m_aapaucChromaACCoefMap,  0x00, 2*16*sizeof(UChar*) );
  ::memset( m_apaucChromaDCBlockMap,  0x00,    2*sizeof(UChar*) );
  ::memset( m_apaucChromaACBlockMap,  0x00,    2*sizeof(UChar*) );
  m_paucBlockMap        = 0;
  m_paucSubMbMap        = 0;
  m_pauiMacroblockMap   = 0;

  return Err::m_nOK;
}
  

ErrVal
RQFGSEncoder::uninit()
{
  ROT( m_bPicInit );
  ROF( m_bInit );
  
  m_cMbDataCtrlEL.uninit();

  m_bInit                     = false;
  m_papcYuvFullPelBufferCtrl  = 0;
  m_pcTransform               = 0;
  m_pcCabacWriter             = 0;
  m_pcMbEncoder               = 0;

  m_bPicInit                  = false;
  m_uiWidthInMB               = 0;
  m_uiHeightInMB              = 0;
  m_iRemainingTCoeff          = 0;
  m_pcSliceHeader             = 0;
  m_pcCurrMbDataCtrl          = 0;
  m_pcOrgResidual             = 0;

  Int i;
  for( i = 0; i < 16; i++ ) 
  {
    delete  [] m_apaucLumaCoefMap        [i];
    delete  [] m_aapaucChromaACCoefMap[0][i];
    delete  [] m_aapaucChromaACCoefMap[1][i];
  }
  for( i = 0; i < 4; i++ ) 
  {
    delete  [] m_aapaucChromaDCCoefMap[0][i];
    delete  [] m_aapaucChromaDCCoefMap[1][i];
  }
  delete    [] m_apaucChromaDCBlockMap[0];
  delete    [] m_apaucChromaDCBlockMap[1];
  delete    [] m_apaucChromaACBlockMap[0];
  delete    [] m_apaucChromaACBlockMap[1];
  delete    [] m_paucBlockMap;
  delete    [] m_paucSubMbMap;
  delete    [] m_pauiMacroblockMap;

  return Err::m_nOK;
}


ErrVal
RQFGSEncoder::initSPS( const SequenceParameterSet& rcSPS )
{
  Int  i;
  UInt uiSize = rcSPS.getFrameWidthInMbs() * rcSPS.getFrameHeightInMbs();

  if( uiSize > m_cMbDataCtrlEL.getSize() )
  {
    RNOK( m_cMbDataCtrlEL.uninit() );
    RNOK( m_cMbDataCtrlEL.init  ( rcSPS ) );

    for( i = 0; i < 16; i++ ) 
    {
      delete  [] m_apaucLumaCoefMap        [i];
      delete  [] m_aapaucChromaACCoefMap[0][i];
      delete  [] m_aapaucChromaACCoefMap[1][i];
    }
    for( i = 0; i < 4; i++ ) 
    {
      delete  [] m_aapaucChromaDCCoefMap[0][i];
      delete  [] m_aapaucChromaDCCoefMap[1][i];
    }
    delete    [] m_apaucChromaDCBlockMap[0];
    delete    [] m_apaucChromaDCBlockMap[1];
    delete    [] m_apaucChromaACBlockMap[0];
    delete    [] m_apaucChromaACBlockMap[1];
    delete    [] m_paucBlockMap;
    delete    [] m_paucSubMbMap;
    delete    [] m_pauiMacroblockMap;


    for( i = 0; i < 16; i++ )
    {
      ROFRS ( ( m_apaucLumaCoefMap        [i] = new UChar[uiSize*16] ), Err::m_nERR );
      ROFRS ( ( m_aapaucChromaACCoefMap[0][i] = new UChar[uiSize*4 ] ), Err::m_nERR );
      ROFRS ( ( m_aapaucChromaACCoefMap[1][i] = new UChar[uiSize*4 ] ), Err::m_nERR );
    }
    for( i = 0; i < 4; i++ )
    {
      ROFRS ( ( m_aapaucChromaDCCoefMap[0][i] = new UChar[uiSize   ] ), Err::m_nERR );
      ROFRS ( ( m_aapaucChromaDCCoefMap[1][i] = new UChar[uiSize   ] ), Err::m_nERR );
    }
    ROFRS   ( ( m_apaucChromaDCBlockMap[0]    = new UChar[uiSize   ] ), Err::m_nERR );
    ROFRS   ( ( m_apaucChromaDCBlockMap[1]    = new UChar[uiSize   ] ), Err::m_nERR );
    ROFRS   ( ( m_apaucChromaACBlockMap[0]    = new UChar[uiSize*4 ] ), Err::m_nERR );
    ROFRS   ( ( m_apaucChromaACBlockMap[1]    = new UChar[uiSize*4 ] ), Err::m_nERR );
    ROFRS   ( ( m_paucBlockMap                = new UChar[uiSize*16] ), Err::m_nERR );
    ROFRS   ( ( m_paucSubMbMap                = new UChar[uiSize*4 ] ), Err::m_nERR );
    ROFRS   ( ( m_pauiMacroblockMap           = new UInt [uiSize   ] ), Err::m_nERR );
  }

  return Err::m_nOK;
}


ErrVal
RQFGSEncoder::initPicture( SliceHeader* pcSliceHeader,
                           MbDataCtrl*  pcCurrMbDataCtrl,
                           IntFrame*    pcOrgResidual,
                           Double       dNumFGSLayers,
                           Double       dLambda,
                           Int          iMaxQpDelta,
                           Bool&        rbFinished,
                           UInt         uiCutLayer,
                           UInt         uiCutPath,
                           UInt         uiMaxBits )
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

  m_uiCutLayer        = uiCutLayer;
  m_uiCutPath         = uiCutPath;
  m_uiMaxBits         = uiMaxBits;
  if( m_uiCutLayer )
  {
    m_iRemainingTCoeff = MSYS_INT_MAX;
  }

  m_eSliceType        = m_pcSliceHeader->getSliceType ();
  Int iOldSliceQP     = m_pcSliceHeader->getPicQp     ();
  Int iNewSliceQP     = max( 0, iOldSliceQP - RQ_QP_DELTA );
  rbFinished          = ( iNewSliceQP == iOldSliceQP );
  
  m_pcSliceHeader->setSliceType   ( F_SLICE );
  m_pcSliceHeader->setNalUnitType ( NAL_UNIT_CODED_SLICE_SCALABLE );
  m_pcSliceHeader->setQualityLevel( 1 );
  m_pcSliceHeader->setSliceHeaderQp       ( iNewSliceQP );
  m_dLambda          /= pow( 2.0, (Double)(iOldSliceQP-iNewSliceQP) / 3.0 );

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
                               FILE*  pFile )
{
  ROF( m_bInit );
  ROF( m_bPicInit );
  rbCorrupted = false;
  rbFinished  = false;

  RNOK ( xResidualTransform() );
  RNOK ( xEncodingFGS( rbFinished, rbCorrupted, pFile ) );
  ROTRS( rbFinished, Err::m_nOK );

  Int iOldSliceQP     = m_pcSliceHeader->getPicQp();
  Int iNewSliceQP     = max( 0, iOldSliceQP - RQ_QP_DELTA );
  rbFinished          = ( iNewSliceQP == iOldSliceQP );
  m_pcSliceHeader->setQualityLevel ( m_pcSliceHeader->getQualityLevel() + 1 );
  m_pcSliceHeader->setSliceHeaderQp       ( iNewSliceQP );
  m_dLambda          /= pow( 2.0, (Double)(iOldSliceQP-iNewSliceQP) / 3.0 );

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




ErrVal
RQFGSEncoder::xScale4x4Block( TCoeff*            piCoeff,
                              const UChar*       pucScale,
                              UInt               uiStart,
                              const QpParameter& rcQP )
{
  if( pucScale )
  {
    Int iAdd = ( rcQP.per() <= 3 ? ( 1 << ( 3 - rcQP.per() ) ) : 0 );
    
    for( UInt ui = uiStart; ui < 16; ui++ )
    {
      piCoeff[ui] = ( ( piCoeff[ui] * g_aaiDequantCoef[rcQP.rem()][ui] * pucScale[ui] + iAdd ) << rcQP.per() ) >> 4;
    }
  }
  else
  {
    for( UInt ui = uiStart; ui < 16; ui++ )
    {
      piCoeff[ui] *= ( g_aaiDequantCoef[rcQP.rem()][ui] << rcQP.per() );
    }
  }

  return Err::m_nOK;
}


ErrVal
RQFGSEncoder::xScale8x8Block( TCoeff*            piCoeff,
                              const UChar*       pucScale,
                              const QpParameter& rcQP )
{
  Int iAdd = ( rcQP.per() <= 5 ? ( 1 << ( 5 - rcQP.per() ) ) : 0 );

  if( pucScale )
  {
    for( UInt ui = 0; ui < 64; ui++ )
    {
      piCoeff[ui] = ( ( piCoeff[ui] * g_aaiDequantCoef64[rcQP.rem()][ui] * pucScale[ui] + iAdd ) << rcQP.per() ) >> 6;
    }
  }
  else
  {
    for( UInt ui = 0; ui < 64; ui++ )
    {
      piCoeff[ui] = ( ( piCoeff[ui] * g_aaiDequantCoef64[rcQP.rem()][ui] * 16 + iAdd ) << rcQP.per() ) >> 6;
    }
  }

  return Err::m_nOK;
}



ErrVal
RQFGSEncoder::xScaleTCoeffs( MbDataAccess& rcMbDataAccess,
                             Bool          bBaseLayer )
{
  const Int aaiDequantDcCoef[6] = {  10, 11, 13, 14, 16, 18 };
  Quantizer cQuantizer;
  cQuantizer.setQp( rcMbDataAccess, false );
  
  const QpParameter&  cLQp      = cQuantizer.getLumaQp  ();
  const QpParameter&  cCQp      = cQuantizer.getChromaQp();
  Bool                bIntra    = rcMbDataAccess.getMbData().isIntra();
  Bool                b8x8      = rcMbDataAccess.getMbData().isTransformSize8x8();
  Bool                b16x16    = rcMbDataAccess.getMbData().isIntra16x16();
  UInt                uiYScalId = ( bIntra ? ( b8x8 && !b16x16 ? 6 : 0 ) : ( b8x8 ? 7 : 3 ) );
  UInt                uiUScalId = ( bIntra ? 1 : 4 );
  UInt                uiVScalId = ( bIntra ? 2 : 5 );
  const UChar*        pucScaleY = rcMbDataAccess.getSH().getScalingMatrix( uiYScalId );
  const UChar*        pucScaleU = rcMbDataAccess.getSH().getScalingMatrix( uiUScalId );
  const UChar*        pucScaleV = rcMbDataAccess.getSH().getScalingMatrix( uiVScalId );
  Int                 iScale    = 1;
  Int                 iShift    = 1;

  //===== luma =====
  if( b16x16 && bBaseLayer )
  {
    //===== INTRA_16x16 =====
    for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
    {
      RNOK( xScale4x4Block( rcMbDataAccess.getMbTCoeffs().get   ( cIdx ), pucScaleY, 1, cLQp ) );
    }

    iScale  = aaiDequantDcCoef[cLQp.rem()] << cLQp.per();
    if( pucScaleY )
    {
      iScale  *= pucScaleY[0];
      iScale >>= 4;
    }
    RNOK( m_pcTransform->invTransformDcCoeff( rcMbDataAccess.getMbTCoeffs().get( B4x4Idx(0) ), iScale ) );

    //===== correct CBP =====
    rcMbDataAccess.getMbData().setMbCbp( rcMbDataAccess.getAutoCbp() );
  }
  else if( b8x8 )
  {
    //===== 8x8 BLOCKS =====
    for( B8x8Idx cIdx; cIdx.isLegal(); cIdx++ )
    {
      RNOK( xScale8x8Block( rcMbDataAccess.getMbTCoeffs().get8x8( cIdx ), pucScaleY, cLQp ) );
    }
  }
  else
  {
    //===== 4x4 BLOCKS =====
    for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
    {
      RNOK( xScale4x4Block( rcMbDataAccess.getMbTCoeffs().get   ( cIdx ), pucScaleY, 0, cLQp ) );
    }
  }

  //===== chroma =====
  for( CIdx cIdx; cIdx.isLegal(); cIdx++ )
  {
    RNOK( xScale4x4Block( rcMbDataAccess.getMbTCoeffs().get( cIdx ), ( cIdx.plane() ? pucScaleV : pucScaleU ), 1, cCQp ) );
  }

  if( ! bBaseLayer )
  {
    UInt  uiMbIndex = rcMbDataAccess.getMbY()*m_uiWidthInMB+rcMbDataAccess.getMbX();
    for( UInt uiPlane = 0; uiPlane < 2; uiPlane++ )
    {
      TCoeff*   piCoeff = rcMbDataAccess.getMbTCoeffs().get( CIdx(4*uiPlane) );
      for( UInt uiDCIdx = 0; uiDCIdx < 4; uiDCIdx++ )
      {
        if( ! ( m_aapaucChromaDCCoefMap[uiPlane][uiDCIdx][uiMbIndex] & CODED ) )
        {
          piCoeff[16*uiDCIdx] = 0;
        }
      }
    }
  }

  iScale = g_aaiDequantCoef[cCQp.rem()][0] << cCQp.per();
  iShift = 1;  
  if( cCQp.per() < 5 && pucScaleU )
  {
    iScale = pucScaleU[0] * g_aaiDequantCoef[cCQp.rem()][0];
    iShift = 5 - cCQp.per();
  }
  m_pcTransform->invTransformChromaDc( rcMbDataAccess.getMbTCoeffs().get( CIdx(0) ), iScale, iShift );     

  iScale = g_aaiDequantCoef[cCQp.rem()][0] << cCQp.per();
  iShift = 1;  
  if( cCQp.per() < 5 && pucScaleV )
  {
    iScale = pucScaleV[0] * g_aaiDequantCoef[cCQp.rem()][0];
    iShift = 5 - cCQp.per();
  }
  m_pcTransform->invTransformChromaDc( rcMbDataAccess.getMbTCoeffs().get( CIdx(4) ), iScale, iShift );

  return Err::m_nOK;
}


ErrVal
RQFGSEncoder::xReconstructMacroblock( MbDataAccess&   rcMbDataAccess,
                                      IntYuvMbBuffer& rcMbBuffer )
{
  m_pcTransform->setClipMode( false );

  Int                 iLStride  = rcMbBuffer.getLStride();
  Int                 iCStride  = rcMbBuffer.getCStride();
  Bool                b8x8      = rcMbDataAccess.getMbData().isTransformSize8x8();
  MbTransformCoeffs&  rcCoeffs  = rcMbDataAccess.getMbTCoeffs();

  rcMbBuffer.setAllSamplesToZero();
  
  if( b8x8 )
  {
    for( B8x8Idx cIdx; cIdx.isLegal(); cIdx++ )
    {
      RNOK( m_pcTransform->invTransform8x8Blk( rcMbBuffer.getYBlk( cIdx ), iLStride, rcCoeffs.get8x8( cIdx ) ) );
    }
  }
  else
  {
    for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
    {
      RNOK( m_pcTransform->invTransform4x4Blk( rcMbBuffer.getYBlk( cIdx ), iLStride, rcCoeffs.get( cIdx ) ) );
    }
  }

  RNOK( m_pcTransform->invTransformChromaBlocks( rcMbBuffer.getMbCbAddr(), iCStride, rcCoeffs.get( CIdx(0) ) ) );
  RNOK( m_pcTransform->invTransformChromaBlocks( rcMbBuffer.getMbCrAddr(), iCStride, rcCoeffs.get( CIdx(4) ) ) );

  m_pcTransform->setClipMode( true );

  return Err::m_nOK;
}


ErrVal
RQFGSEncoder::xUpdateMacroblock( MbDataAccess&  rcMbDataAccess,
                                 MbDataAccess&  rcMbDataAccessEL )
{
  rcMbDataAccess  .getMbData().setQp              ( rcMbDataAccessEL.getMbData().getQp              () );
  rcMbDataAccessEL.getMbData().setTransformSize8x8( rcMbDataAccess  .getMbData().isTransformSize8x8 () );
  RNOK( xScaleTCoeffs( rcMbDataAccessEL, false ) );
  
  RNOK( xUpdateMacroblockCoef ( rcMbDataAccess, rcMbDataAccessEL ) );

  m_iRemainingTCoeff -= 16*16+2*8*8;
  TWSOT( m_iRemainingTCoeff < 0 );

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



Int
RQFGSEncoder::xScaleLevel4x4( Int                 iLevel,
                              Int                 iIndex,
                              const QpParameter&  cQP,
                              const QpParameter&  cBaseQP )
{
  Int iSign       = ( iLevel < 0 ? -1 : 1 );
  Int iBaseScale  = g_aaiDequantCoef[cBaseQP.rem()][iIndex] << cBaseQP.per();
  Int iScale      = g_aaiDequantCoef[cQP    .rem()][iIndex] << cQP    .per();

  return iSign * ( ( abs(iLevel) * iBaseScale ) + ( iScale >> 1 ) ) / iScale;
}

Int
RQFGSEncoder::xScaleLevel8x8( Int                 iLevel,
                              Int                 iIndex,
                              const QpParameter&  cQP,
                              const QpParameter&  cBaseQP )
{
  Int iSign       = ( iLevel < 0 ? -1 : 1 );
  Int iBaseScale  = g_aaiDequantCoef64[cBaseQP.rem()][iIndex] << cBaseQP.per();
  Int iScale      = g_aaiDequantCoef64[cQP    .rem()][iIndex] << cQP    .per();

  return iSign * ( ( abs(iLevel) * iBaseScale ) + ( iScale >> 1 ) ) / iScale;
}


ErrVal
RQFGSEncoder::xSetSymbolsChroma( TCoeff*             piCoeff,
                                 TCoeff*             piCoeffBase,
                                 const QpParameter&  cQP,
                                 UInt&               uiCoeffCostDC,
                                 UInt&               uiCoeffCostAC,
                                 Bool&               bSigDC,
                                 Bool&               bSigAC,
                                 ChromaIdx           cIdx )
{
  Int iRun  = 0;

  for( Int iPos = 0; iPos < 16; iPos++ )
  {
    Int iIndex      = g_aucFrameScan[iPos];
    Int iBaseLevel  = piCoeffBase   [iIndex];
    Int iLevel      = piCoeff       [iIndex];

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
                              TCoeff*             piCoeffBase,
                              const QpParameter&  cQP,
                              UInt&               uiCoeffCost,
                              UInt&               ruiCbp,
                              LumaIdx             cIdx )
{
  Int   iRun  = 0;
  Bool  bSig  = false;

  for( Int iPos = 0; iPos < 16; iPos++ )
  {
    Int iIndex      = g_aucFrameScan[iPos];
    Int iBaseLevel  = piCoeffBase   [iIndex];
    Int iLevel      = piCoeff       [iIndex];

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
RQFGSEncoder::xScaleSymbols4x4( TCoeff*             piCoeff,
                                const QpParameter&  cQP,
                                const QpParameter&  cBaseQP )
{
  for( Int iIndex = 0; iIndex < 16; iIndex++ )
  {
    if( piCoeff[iIndex] )
    {
      piCoeff[iIndex] = xScaleLevel4x4( piCoeff[iIndex], iIndex, cQP, cBaseQP );
    }
  }
  return Err::m_nOK;
}


ErrVal
RQFGSEncoder::xScaleSymbols8x8( TCoeff*             piCoeff,
                                const QpParameter&  cQP,
                                const QpParameter&  cBaseQP )
{
  for( Int iIndex = 0; iIndex < 64; iIndex++ )
  {
    if( piCoeff[iIndex] )
    {
      piCoeff[iIndex] = xScaleLevel8x8( piCoeff[iIndex], iIndex, cQP, cBaseQP );
    }
  }
  return Err::m_nOK;
}




ErrVal
RQFGSEncoder::xUpdateSymbols( TCoeff* piCoeff,
                              TCoeff* piCoeffEL,
                              Bool&   bSigDC,
                              Bool&   bSigAC,
                              Int     iNumCoeff )
{
  piCoeff     [0] += piCoeffEL[0];
  if( piCoeff [0] )
  {
    bSigDC = true;
  }

  for( Int iIndex = 1; iIndex < iNumCoeff; iIndex++ )
  {
    piCoeff    [iIndex] += piCoeffEL[iIndex];
    if( piCoeff[iIndex] )
    {
      bSigAC = true;
    }
  }

  return Err::m_nOK;
}



ErrVal
RQFGSEncoder::xUpdateMacroblockQP( MbDataAccess&  rcMbDataAccess,
                                   Int            iNewQP )
{
  Bool          bIntra    = rcMbDataAccess.getMbData().isIntra();
  Bool          b16x16    = rcMbDataAccess.getMbData().isIntra16x16();
  Bool          b8x8      = rcMbDataAccess.getMbData().isTransformSize8x8();
  const UChar*  pucScaleY = rcMbDataAccess.getSH().getScalingMatrix( bIntra ? ( b8x8 && !b16x16 ? 6 : 0 ) : ( b8x8 ? 7 : 3 ) );
  const UChar*  pucScaleU = rcMbDataAccess.getSH().getScalingMatrix( bIntra ? 1 : 4 );
  const UChar*  pucScaleV = rcMbDataAccess.getSH().getScalingMatrix( bIntra ? 2 : 5 );

  Quantizer     cOldQuantizer;
  Quantizer     cNewQuantizer;

  cOldQuantizer             .setQp( rcMbDataAccess, false );
  rcMbDataAccess.getMbData().setQp( iNewQP );
  cNewQuantizer             .setQp( rcMbDataAccess, false );

  //===== luma =====
  if( b16x16 || ! b8x8 )
  {
    for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
    {
      RNOK( xScaleSymbols4x4( rcMbDataAccess.getMbTCoeffs().get( cIdx ),
                              cNewQuantizer .getLumaQp(),
                              cOldQuantizer .getLumaQp() ) );
    }
  }
  else
  {
    for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
    {
      RNOK( xScaleSymbols8x8( rcMbDataAccess.getMbTCoeffs().get8x8( c8x8Idx ),
                              cNewQuantizer .getLumaQp(),
                              cOldQuantizer .getLumaQp() ) );
    }
  }


  //===== chroma =====
  for( CIdx cCIdx(0); cCIdx.isLegal(8); cCIdx++ )
  {
    RNOK( xScaleSymbols4x4( rcMbDataAccess.getMbTCoeffs().get( cCIdx ),
                            cNewQuantizer .getChromaQp(),
                            cOldQuantizer .getChromaQp() ) );
  }

  return Err::m_nOK;
}



ErrVal
RQFGSEncoder::xUpdateMacroblockCoef( MbDataAccess&  rcMbDataAccess,
                                     MbDataAccess&  rcMbDataAccessEL )
{
  UInt  uiExtCbp  = 0;
  Bool  bIntra    = rcMbDataAccess.getMbData().isIntra();
  Bool  b8x8      = rcMbDataAccess.getMbData().isTransformSize8x8();

  //===== luma =====
  if( b8x8 )
  {
    for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
    {
      Bool  bSigDC  = false;
      Bool  bSigAC  = false;

      RNOK( xUpdateSymbols( rcMbDataAccess  .getMbTCoeffs().get8x8( c8x8Idx ),
                            rcMbDataAccessEL.getMbTCoeffs().get8x8( c8x8Idx ),
                            bSigDC, bSigAC, 64 ) );
      if( bSigDC || bSigAC )
      {
        uiExtCbp |= ( 0x33 << c8x8Idx.b4x4() );
      }
    }
  }
  else
  {
    for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
    {
      Bool  bSigDC  = false;
      Bool  bSigAC  = false;

      RNOK( xUpdateSymbols( rcMbDataAccess  .getMbTCoeffs().get( cIdx ),
                            rcMbDataAccessEL.getMbTCoeffs().get( cIdx ),
                            bSigDC, bSigAC, 16 ) );
      if( bSigDC || bSigAC )
      {
        uiExtCbp |= ( 1 << cIdx );
      }
    }
  }


  //===== chroma =====
  Bool  bSigDC  = false;
  Bool  bSigAC  = false;

  for( CIdx cCIdx(0); cCIdx.isLegal(8); cCIdx++ )
  {
    RNOK( xUpdateSymbols( rcMbDataAccess  .getMbTCoeffs().get( cCIdx ),
                          rcMbDataAccessEL.getMbTCoeffs().get( cCIdx ),
                          bSigDC, bSigAC, 16 ) );
  }
  UInt  uiChromaCBP = ( bSigAC ? 2 : bSigDC ? 1 : 0 );
  uiExtCbp         |= ( uiChromaCBP << 16 );


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
  Bool          b8x8      = rcMbDataAccessBase.getMbData().isTransformSize8x8();
  Bool          bLowPass  = m_pcSliceHeader->getTemporalLevel() == 0;
  const UChar*  pucScaleY = rcMbDataAccessBase.getSH().getScalingMatrix( bIntra ? ( b8x8 ? 6 : 0 ) : ( b8x8 ? 7 : 3 ) );
  const UChar*  pucScaleU = rcMbDataAccessBase.getSH().getScalingMatrix( bIntra ? 1 : 4 );
  const UChar*  pucScaleV = rcMbDataAccessBase.getSH().getScalingMatrix( bIntra ? 2 : 5 );

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
                                              pucScaleY, uiAbsSum4x4 ) );
        uiAbsSum8x8 += uiAbsSum4x4;

        RNOK( xSetSymbols4x4( rcMbDataAccess    .getMbTCoeffs().get( cIdx ),
                              rcMbDataAccessBase.getMbTCoeffs().get( cIdx ),
                              m_pcTransform->getLumaQp(),
                              uiCoeffCost8x8, uiCbp, cIdx ) );
      }
      if( uiCbp )
      {
#if COEF_SKIP
        if( uiCoeffCost8x8 <= 4 && ! bIntra && ! bLowPass )
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
#if COEF_SKIP
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
                               rcMbDataAccessBase.getMbTCoeffs().get( cIdxU ),
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
      RNOK( xSetSymbolsChroma( rcMbDataAccess    .getMbTCoeffs().get( cIdxV ),
                               rcMbDataAccessBase.getMbTCoeffs().get( cIdxV ),
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
        pcMbDataAccessCurr->getMbData().getMbCbp    () )
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






ErrVal
RQFGSEncoder::xEncodingFGS( Bool& rbFinished,
                            Bool& rbCorrupted,
                            FILE* pFile )
{
  RNOK( m_cMbDataCtrlEL    .initSlice ( *m_pcSliceHeader, PRE_PROCESS, false, NULL ) );
  RNOK( m_pcCurrMbDataCtrl->initSlice ( *m_pcSliceHeader, PRE_PROCESS, false, NULL ) );
  m_pcSliceHeader->setSliceType( m_eSliceType );
  RNOK( m_pcCabacWriter   ->startSlice( *m_pcSliceHeader ) );
  m_pcSliceHeader->setSliceType( F_SLICE );

  RNOK( xInitializeCodingPath() );

  rbCorrupted = false;
  Int iLastQP = m_pcSliceHeader->getPicQp();

  UInt  uiBitsLast   = m_pcCabacWriter->getNumberOfWrittenBits();
  UInt  uiBitsPath1  = 0;
  UInt  uiBitsPath2  = 0;
  UInt  uiBitsPath3  = 0;

  Bool  bCheck1 = ( m_uiCutLayer == m_pcSliceHeader->getQualityLevel() && m_uiCutPath == 0 );
  Bool  bCheck2 = ( m_uiCutLayer == m_pcSliceHeader->getQualityLevel() && m_uiCutPath == 1 );
  Bool  bCheck3 = ( m_uiCutLayer == m_pcSliceHeader->getQualityLevel() && m_uiCutPath == 2 );

  try
  {
    //===== SIGNIFICANCE PATH =====
    {
      for( UInt uiScanIndex = 0; uiScanIndex < 16; uiScanIndex++ )
      {
        //===== LUMINANCE =====
        for( UInt uiBlockYIdx = 0; uiBlockYIdx < 4*m_uiHeightInMB; uiBlockYIdx++ )
        for( UInt uiBlockXIdx = 0; uiBlockXIdx < 4*m_uiWidthInMB;  uiBlockXIdx++ )
        {
          if( m_paucBlockMap[uiBlockYIdx*4*m_uiWidthInMB+uiBlockXIdx] & SIGNIFICANT )
          RNOK( xEncodeNewCoefficientLuma( uiBlockYIdx, uiBlockXIdx, uiScanIndex, iLastQP ) );
          
          if( bCheck1 && m_pcCabacWriter->getNumberOfWrittenBits() - uiBitsLast >= m_uiMaxBits )
          {
            throw WriteStop();
          }
        }

        if( uiScanIndex == 0 )
        {
          //===== CHROMA DC =====
          for( UInt uiDCIdx  = 0; uiDCIdx  < 4;              uiDCIdx ++ )
          for( UInt uiPlane  = 0; uiPlane  < 2;              uiPlane ++ )
          for( UInt uiMbYIdx = 0; uiMbYIdx < m_uiHeightInMB; uiMbYIdx++ )
          for( UInt uiMbXIdx = 0; uiMbXIdx < m_uiWidthInMB;  uiMbXIdx++ )
          {
            if( m_apaucChromaDCBlockMap[uiPlane][uiMbYIdx*m_uiWidthInMB+uiMbXIdx] & SIGNIFICANT )
            RNOK( xEncodeNewCoefficientChromaDC( uiPlane, uiMbYIdx, uiMbXIdx, uiDCIdx, iLastQP ) );
          
            if( bCheck1 && m_pcCabacWriter->getNumberOfWrittenBits() - uiBitsLast >= m_uiMaxBits )
            {
              throw WriteStop();
            }
          }
        }
        else
        {
          //===== CHROMA AC =====
          for( UInt uiPlane  = 0; uiPlane  < 2;                uiPlane ++ )
          for( UInt uiB8YIdx = 0; uiB8YIdx < 2*m_uiHeightInMB; uiB8YIdx++ )
          for( UInt uiB8XIdx = 0; uiB8XIdx < 2*m_uiWidthInMB;  uiB8XIdx++ )
          {
            if( m_apaucChromaACBlockMap[uiPlane][uiB8YIdx*2*m_uiWidthInMB+uiB8XIdx] & SIGNIFICANT )
            RNOK( xEncodeNewCoefficientChromaAC( uiPlane, uiB8YIdx, uiB8XIdx, uiScanIndex, iLastQP ) );
          
            if( bCheck1 && m_pcCabacWriter->getNumberOfWrittenBits() - uiBitsLast >= m_uiMaxBits )
            {
              throw WriteStop();
            }
          }
        }
      }
    }
    if( bCheck1 )
    {
      throw WriteStop();
    }
    uiBitsPath1 = m_pcCabacWriter->getNumberOfWrittenBits() - uiBitsLast;
    uiBitsLast  = m_pcCabacWriter->getNumberOfWrittenBits();
    if( pFile )
    {
      fprintf( pFile, "\t%d", uiBitsPath1 );
    }
  

    //===== REFINEMENT PATH =====
    {
      for( UInt uiScanIndex = 0; uiScanIndex < 16; uiScanIndex++ )
      {
        //===== LUMINANCE =====
        for( UInt uiBlockYIdx = 0; uiBlockYIdx < 4*m_uiHeightInMB; uiBlockYIdx++ )
        for( UInt uiBlockXIdx = 0; uiBlockXIdx < 4*m_uiWidthInMB; uiBlockXIdx++ )
        {
          RNOK( xEncodeCoefficientLumaRef( uiBlockYIdx, uiBlockXIdx, uiScanIndex ) );
          
          if( bCheck2 && m_pcCabacWriter->getNumberOfWrittenBits() - uiBitsLast >= m_uiMaxBits )
          {
            throw WriteStop();
          }
        }

        if( uiScanIndex == 0 )
        {
          //===== CHROMA DC =====
          for( UInt uiDCIdx  = 0; uiDCIdx  < 4;              uiDCIdx ++ )
          for( UInt uiPlane  = 0; uiPlane  < 2;              uiPlane ++ )
          for( UInt uiMbYIdx = 0; uiMbYIdx < m_uiHeightInMB; uiMbYIdx++ )
          for( UInt uiMbXIdx = 0; uiMbXIdx < m_uiWidthInMB;  uiMbXIdx++ )
          {
            RNOK( xEncodeCoefficientChromaDCRef( uiPlane, uiMbYIdx, uiMbXIdx, uiDCIdx ) );
          
            if( bCheck2 && m_pcCabacWriter->getNumberOfWrittenBits() - uiBitsLast >= m_uiMaxBits )
            {
              throw WriteStop();
            }
          }
        }
        else
        {
          //===== CHROMA AC =====
          for( UInt uiPlane  = 0; uiPlane  < 2;                uiPlane ++ )
          for( UInt uiB8YIdx = 0; uiB8YIdx < 2*m_uiHeightInMB; uiB8YIdx++ )
          for( UInt uiB8XIdx = 0; uiB8XIdx < 2*m_uiWidthInMB;  uiB8XIdx++ )
          {
            RNOK( xEncodeCoefficientChromaACRef( uiPlane, uiB8YIdx, uiB8XIdx, uiScanIndex ) );
          
            if( bCheck2 && m_pcCabacWriter->getNumberOfWrittenBits() - uiBitsLast >= m_uiMaxBits )
            {
              throw WriteStop();
            }
          }
        }
      }
    }
    if( bCheck2 )
    {
      throw WriteStop();
    }
    uiBitsPath2 = m_pcCabacWriter->getNumberOfWrittenBits() - uiBitsLast;
    uiBitsLast  = m_pcCabacWriter->getNumberOfWrittenBits();
    if( pFile )
    {
      fprintf( pFile, "\t%d", uiBitsPath2 );
    }

    
    //===== SIGNIFICANCE PATH =====
    {
      for( UInt uiScanIndex = 0; uiScanIndex < 16; uiScanIndex++ )
      {
        //===== LUMINANCE =====
        for( UInt uiBlockYIdx = 0; uiBlockYIdx < 4*m_uiHeightInMB; uiBlockYIdx++ )
        for( UInt uiBlockXIdx = 0; uiBlockXIdx < 4*m_uiWidthInMB;  uiBlockXIdx++ )
        {
          RNOK( xEncodeNewCoefficientLuma( uiBlockYIdx, uiBlockXIdx, uiScanIndex, iLastQP ) );
          
          if( bCheck3 && m_pcCabacWriter->getNumberOfWrittenBits() - uiBitsLast >= m_uiMaxBits )
          {
            throw WriteStop();
          }
        }

        if( uiScanIndex == 0 )
        {
          //===== CHROMA DC =====
          for( UInt uiDCIdx  = 0; uiDCIdx  < 4;              uiDCIdx ++ )
          for( UInt uiPlane  = 0; uiPlane  < 2;              uiPlane ++ )
          for( UInt uiMbYIdx = 0; uiMbYIdx < m_uiHeightInMB; uiMbYIdx++ )
          for( UInt uiMbXIdx = 0; uiMbXIdx < m_uiWidthInMB;  uiMbXIdx++ )
          {
            RNOK( xEncodeNewCoefficientChromaDC( uiPlane, uiMbYIdx, uiMbXIdx, uiDCIdx, iLastQP ) );
          
            if( bCheck3 && m_pcCabacWriter->getNumberOfWrittenBits() - uiBitsLast >= m_uiMaxBits )
            {
              throw WriteStop();
            }
          }
        }
        else
        {
          //===== CHROMA AC =====
          for( UInt uiPlane  = 0; uiPlane  < 2;                uiPlane ++ )
          for( UInt uiB8YIdx = 0; uiB8YIdx < 2*m_uiHeightInMB; uiB8YIdx++ )
          for( UInt uiB8XIdx = 0; uiB8XIdx < 2*m_uiWidthInMB;  uiB8XIdx++ )
          {
            RNOK( xEncodeNewCoefficientChromaAC( uiPlane, uiB8YIdx, uiB8XIdx, uiScanIndex, iLastQP ) );
          
            if( bCheck3 && m_pcCabacWriter->getNumberOfWrittenBits() - uiBitsLast >= m_uiMaxBits )
            {
              throw WriteStop();
            }
          }
        }
      }
    }
    if( bCheck3 )
    {
      throw WriteStop();
    }
    uiBitsPath3 = m_pcCabacWriter->getNumberOfWrittenBits() - uiBitsLast;
    uiBitsLast  = m_pcCabacWriter->getNumberOfWrittenBits();
    if( pFile )
    {
      fprintf( pFile, "\t%d", uiBitsPath3 );
    }

    RNOK( m_pcCabacWriter->RQencodeTermBit( 1 ) );
  }
  catch( WriteStop )
  {
    rbCorrupted = true;
  }
  rbFinished      = rbCorrupted || ( m_iRemainingTCoeff == 0 );

  RNOK( m_pcCabacWriter->finishSlice() );


  RNOK( xUpdateCodingPath() );

  
  return Err::m_nOK;
}



ErrVal
RQFGSEncoder::xUpdateCodingPath()
{
  for( UInt uiMbY = 0; uiMbY < m_uiHeightInMB; uiMbY++ )
  for( UInt uiMbX = 0; uiMbX < m_uiWidthInMB;  uiMbX++ )
  {
    MbDataAccess* pcMbDataAccessBL = 0;
    MbDataAccess* pcMbDataAccessEL = 0;
    RNOK( m_pcCurrMbDataCtrl->initMb( pcMbDataAccessBL, uiMbY, uiMbX ) );
    RNOK( m_cMbDataCtrlEL    .initMb( pcMbDataAccessEL, uiMbY, uiMbX ) );

    //===== scale enhancement layer coefficients =====
    pcMbDataAccessEL->getMbData().setTransformSize8x8( pcMbDataAccessBL->getMbData().isTransformSize8x8() );    
    RNOK( xScaleTCoeffs( *pcMbDataAccessEL, false ) );
  
    //===== update coefficients, CBP, QP =====
    RNOK( xUpdateMacroblock( *pcMbDataAccessBL, *pcMbDataAccessEL, uiMbY, uiMbX ) );
  }

  return Err::m_nOK;
}


ErrVal
RQFGSEncoder::xInitializeCodingPath()
{
  for( UInt uiMbY = 0; uiMbY < m_uiHeightInMB; uiMbY++ )
  for( UInt uiMbX = 0; uiMbX < m_uiWidthInMB;  uiMbX++ )
  {
    MbDataAccess* pcMbDataAccess = 0;
    RNOK( m_pcCurrMbDataCtrl->initMb( pcMbDataAccess, uiMbY, uiMbX ) );
    
    MbData& rcMbData        = pcMbDataAccess->getMbData();
    UInt    uiMbIndex       = uiMbY * m_uiWidthInMB + uiMbX;
    Bool    bIntra4x4       =     rcMbData.isIntra4x4   ();
    Bool    bIntra16x16     =     rcMbData.isIntra16x16 ();
    Bool    bIsSignificant  = (   rcMbData.getMbCbp()          > 0 );
    Bool    bIsSigLuma      = ( ( rcMbData.getMbCbp() & 0xFF ) > 0 );
    Bool    b8x8Present     = (   pcMbDataAccess->getSH().getPPS().getTransform8x8ModeFlag() &&
                                  rcMbData.is8x8TrafoFlagPresent() );
    Bool    b8x8Transform   = ( b8x8Present && ( bIsSigLuma || bIntra4x4 ) && rcMbData.isTransformSize8x8() );
    UInt    uiMbCbp         = pcMbDataAccess->getAutoCbp();

    //===== set macroblock mode =====
    m_pauiMacroblockMap[uiMbIndex] =  ( bIntra16x16 || bIsSignificant                           ? SIGNIFICANT         : CLEAR )
                                   +  ( bIntra16x16 || bIntra4x4 || bIsSigLuma || !b8x8Present  ? TRANSFORM_SPECIFIED : CLEAR );
    
    //--- LUMA ---
    for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
    {
      UInt uiSubMbIndex = ( 2*uiMbY + c8x8Idx.y()/2 ) * 2 * m_uiWidthInMB + ( 2*uiMbX + c8x8Idx.x() / 2 );

      //===== set sub-macroblock mode =====
      m_paucSubMbMap[uiSubMbIndex] = ( ( uiMbCbp & ( 1 << c8x8Idx.b8x8Index() ) ) > 0 ? SIGNIFICANT : CLEAR );

      if( b8x8Transform )
      {
        UInt    auiBlockIdx[4]  = { ( 4*uiMbY + c8x8Idx.y()     ) * 4 * m_uiWidthInMB + ( 4*uiMbX + c8x8Idx.x()     ),
                                    ( 4*uiMbY + c8x8Idx.y()     ) * 4 * m_uiWidthInMB + ( 4*uiMbX + c8x8Idx.x() + 1 ),
                                    ( 4*uiMbY + c8x8Idx.y() + 1 ) * 4 * m_uiWidthInMB + ( 4*uiMbX + c8x8Idx.x()     ),
                                    ( 4*uiMbY + c8x8Idx.y() + 1 ) * 4 * m_uiWidthInMB + ( 4*uiMbX + c8x8Idx.x() + 1 ) };
        TCoeff* piCoeff         = rcMbData.getMbTCoeffs().get8x8( c8x8Idx );

        //===== set block mode =====
        m_paucBlockMap[auiBlockIdx[0]] = m_paucSubMbMap[uiSubMbIndex];
        m_paucBlockMap[auiBlockIdx[1]] = m_paucSubMbMap[uiSubMbIndex];
        m_paucBlockMap[auiBlockIdx[2]] = m_paucSubMbMap[uiSubMbIndex];
        m_paucBlockMap[auiBlockIdx[3]] = m_paucSubMbMap[uiSubMbIndex];

        //===== set transform coefficients =====
        for( UInt ui8x8ScanIndex = 0; ui8x8ScanIndex < 64; ui8x8ScanIndex++ )
        {
          UInt  uiS = ui8x8ScanIndex/4;
          UInt  uiB = auiBlockIdx[ui8x8ScanIndex%4];
          m_apaucLumaCoefMap[uiS][uiB] = ( piCoeff[g_aucFrameScan64[ui8x8ScanIndex]] ? SIGNIFICANT : CLEAR );
        }
      }
      else
      {
        for( S4x4Idx cIdx( c8x8Idx ); cIdx.isLegal( c8x8Idx ); cIdx++ )
        {
          UInt    uiBlockIdx  = ( 4*uiMbY + cIdx.y() ) * 4 * m_uiWidthInMB + ( 4*uiMbX + cIdx.x() );
          TCoeff* piCoeff     = rcMbData.getMbTCoeffs().get( cIdx );
          UChar   ucBlockSig  = CLEAR;

          //===== set transform coefficients =====
          for( UInt uiScanIndex = 0; uiScanIndex < 16; uiScanIndex++ )
          {
            if( piCoeff[g_aucFrameScan[uiScanIndex]] )
            {
              m_apaucLumaCoefMap[uiScanIndex][uiBlockIdx] = SIGNIFICANT;
              ucBlockSig                                  = SIGNIFICANT;
            }
            else
            {
              m_apaucLumaCoefMap[uiScanIndex][uiBlockIdx] = CLEAR;
            }
          }

          //===== set block mode =====
          m_paucBlockMap[uiBlockIdx] = ucBlockSig;
        }
      }
    }


    //--- CHROMA DC ---
    for( UInt uiPlane = 0; uiPlane < 2; uiPlane++ )
    {
      TCoeff* piCoeff     = rcMbData.getMbTCoeffs().get( CIdx(4*uiPlane) );
      UChar   ucBlockSig  = CLEAR;

      for( UInt ui = 0; ui < 4; ui++ )
      {
        if( piCoeff[g_aucIndexChromaDCScan[ui]] )
        {
          m_aapaucChromaDCCoefMap[uiPlane][ui][uiMbIndex] = SIGNIFICANT;
          ucBlockSig                                      = SIGNIFICANT;
        }
        else
        {
          m_aapaucChromaDCCoefMap[uiPlane][ui][uiMbIndex] = CLEAR;
        }
      }
      m_apaucChromaDCBlockMap[uiPlane][uiMbIndex] = ucBlockSig;
    }

    //--- CHROMA AC ---
    for( CIdx cCIdx; cCIdx.isLegal(); cCIdx++ )
    {
      UInt    ui8x8Idx    = ( 2*uiMbY + cCIdx.y() ) * 2 * m_uiWidthInMB + ( 2 * uiMbX + cCIdx.x() );
      TCoeff* piCoeff     = rcMbData.getMbTCoeffs().get( cCIdx );
      UChar   ucBlockSig  = CLEAR;

      for( UInt ui = 1; ui < 16; ui++ )
      {
        if( piCoeff[g_aucFrameScan[ui]] )
        {
          m_aapaucChromaACCoefMap[cCIdx.plane()][ui][ui8x8Idx]  = SIGNIFICANT;
          ucBlockSig                                            = SIGNIFICANT;
        }
        else
        {
          m_aapaucChromaACCoefMap[cCIdx.plane()][ui][ui8x8Idx]  = CLEAR;
        }
      }
      m_apaucChromaACBlockMap[cCIdx.plane()][ui8x8Idx] = ucBlockSig;
    }
  }

  return Err::m_nOK;
}




ErrVal
RQFGSEncoder::xEncodeNewCoefficientLuma( UInt   uiBlockYIndex,
                                         UInt   uiBlockXIndex,
                                         UInt   uiScanIndex,
                                         Int&   riLastQp )
{
  UInt    uiSigCtx      = xGetSigCtxLuma( uiScanIndex, uiBlockYIndex, uiBlockXIndex );

  UInt    uiBlockIndex  =  uiBlockYIndex    * 4 * m_uiWidthInMB +  uiBlockXIndex;
  UInt    uiSubMbIndex  = (uiBlockYIndex/2) * 2 * m_uiWidthInMB + (uiBlockXIndex/2);
  UInt    uiMbIndex     = (uiBlockYIndex/4) * 1 * m_uiWidthInMB + (uiBlockXIndex/4);
  
  UInt    uiB8x8        = ((uiBlockYIndex%4)/2) * 2 + ((uiBlockXIndex%4)/2);
  UInt    uiB4x4        =  (uiBlockYIndex%4)    * 4 +  (uiBlockXIndex%4);
  Par8x8  ePar8x8       = Par8x8(uiB8x8);
  
  B8x8Idx c8x8Idx(ePar8x8);
  B4x4Idx c4x4Idx(uiB4x4);

  //===== check if coefficient is significant or was already coded =====
  ROTRS( m_apaucLumaCoefMap[uiScanIndex][uiBlockIndex] & ( SIGNIFICANT | CODED ), Err::m_nOK );

  MbDataAccess* pcMbDataAccessEL  = 0;
  MbDataAccess* pcMbDataAccessBL  = 0;
  RNOK( m_cMbDataCtrlEL     .initMb( pcMbDataAccessEL, uiBlockYIndex/4, uiBlockXIndex/4 ) );
  RNOK( m_pcCurrMbDataCtrl ->initMb( pcMbDataAccessBL, uiBlockYIndex/4, uiBlockXIndex/4 ) );
  

  if( ! ( m_paucSubMbMap[uiSubMbIndex] & CODED ) )
  {
    //===== CBP =====
    Bool bSigCBP = m_pcCabacWriter->RQencodeCBP_8x8( *pcMbDataAccessEL, *pcMbDataAccessBL, c8x8Idx );
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
        RNOK( m_pcCabacWriter->RQencodeDeltaQp( *pcMbDataAccessEL, *pcMbDataAccessBL ) );
        riLastQp = pcMbDataAccessEL->getMbData().getQp();

        m_pauiMacroblockMap[uiMbIndex] |= SIGNIFICANT;
      }

      //===== transform size =====
      if( ! ( m_pauiMacroblockMap[uiMbIndex] & TRANSFORM_SPECIFIED ) )
      {
        RNOK( m_pcCabacWriter->RQencode8x8Flag( *pcMbDataAccessEL, *pcMbDataAccessBL ) );

        m_pauiMacroblockMap[uiMbIndex] |= TRANSFORM_SPECIFIED;
      }

      m_pauiMacroblockMap[uiMbIndex] |= CODED;
    }
  }


  if( pcMbDataAccessBL->getMbData().isTransformSize8x8() )
  {
    UInt  uiLast          = 0;
    UInt  ui8x8ScanIndex  = 4*uiScanIndex + 2*( uiBlockYIndex % 2 ) + ( uiBlockXIndex % 2 );
    RNOK( m_pcCabacWriter->RQencodeNewTCoeff_8x8( *pcMbDataAccessEL, *pcMbDataAccessBL,
                                                  c8x8Idx, ui8x8ScanIndex, uiLast, uiSigCtx ) );
    m_apaucLumaCoefMap[uiScanIndex][uiBlockIndex] |= CODED;
    if( pcMbDataAccessEL->getMbTCoeffs().get8x8( c8x8Idx )[g_aucFrameScan64[ui8x8ScanIndex]] )
    {
      m_apaucLumaCoefMap[uiScanIndex][uiBlockIndex] |= SIGNIFICANT;
    }
    m_iRemainingTCoeff--;
    m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
    ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );
    
    if( uiLast )
    {
      UInt auiBlockIdx[4] = { ( (uiBlockYIndex/4)*4 + c8x8Idx.y()     ) * 4 * m_uiWidthInMB + ( (uiBlockXIndex/4)*4 + c8x8Idx.x()     ),
                              ( (uiBlockYIndex/4)*4 + c8x8Idx.y()     ) * 4 * m_uiWidthInMB + ( (uiBlockXIndex/4)*4 + c8x8Idx.x() + 1 ),
                              ( (uiBlockYIndex/4)*4 + c8x8Idx.y() + 1 ) * 4 * m_uiWidthInMB + ( (uiBlockXIndex/4)*4 + c8x8Idx.x()     ),
                              ( (uiBlockYIndex/4)*4 + c8x8Idx.y() + 1 ) * 4 * m_uiWidthInMB + ( (uiBlockXIndex/4)*4 + c8x8Idx.x() + 1 ) };

      for( UInt ui = ui8x8ScanIndex + 1; ui < 64; ui++ )
      {
        UInt  uiS = ui/4;
        UInt  uiB = auiBlockIdx[ui%4];
        if( ! ( m_apaucLumaCoefMap[uiS][uiB] & SIGNIFICANT ) )
        {
          m_apaucLumaCoefMap[uiS][uiB] |= CODED;
          m_iRemainingTCoeff--;
          m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
          ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );
        }
      }
    }
    TWSOT( m_iRemainingTCoeff < 0 );
  }
  else
  {
    if( ! ( m_paucBlockMap[uiBlockIndex] & CODED ) )
    {
      Bool bSigBCBP = m_pcCabacWriter->RQencodeBCBP_4x4( *pcMbDataAccessEL, *pcMbDataAccessBL, c4x4Idx );
      m_paucBlockMap[uiBlockIndex] |= CODED;
      if(  bSigBCBP )
      {
        m_paucBlockMap[uiBlockIndex] |= SIGNIFICANT;
      }
      if( ! bSigBCBP )
      {
        for( UInt ui = 0; ui < 16; ui++ )
        {
          if( ! ( m_apaucLumaCoefMap[ui][uiBlockIndex] & SIGNIFICANT ) )
          {
            m_apaucLumaCoefMap[ui][uiBlockIndex] |= CODED;
            m_iRemainingTCoeff--;
            m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
            ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );
          }
        }
        TWSOT( m_iRemainingTCoeff < 0 );
        return Err::m_nOK;
      }
    }


    UInt  uiLast = 0;
    RNOK( m_pcCabacWriter->RQencodeNewTCoeff_Luma( *pcMbDataAccessEL, *pcMbDataAccessBL,
                                                   LUMA_SCAN, c4x4Idx, uiScanIndex, uiLast, uiSigCtx ) );
    m_apaucLumaCoefMap[uiScanIndex][uiBlockIndex] |= CODED;
    if( pcMbDataAccessEL->getMbTCoeffs().get( c4x4Idx )[g_aucFrameScan[uiScanIndex]] )
    {
      m_apaucLumaCoefMap[uiScanIndex][uiBlockIndex] |= SIGNIFICANT;
    }
    m_iRemainingTCoeff--;
    m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
    ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );
    
    if( uiLast )
    {
      for( UInt ui = uiScanIndex + 1; ui < 16; ui++ )
      {
        if( ! ( m_apaucLumaCoefMap[ui][uiBlockIndex] & SIGNIFICANT ) )
        {
          m_apaucLumaCoefMap[ui][uiBlockIndex] |= CODED;
          m_iRemainingTCoeff--;
          m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
          ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );
        }
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
                                              UInt    uiDCIdx,
                                              Int&    riLastQP )
{
  UInt uiSigCtx  = xGetSigCtxChromaDC( uiPlane, uiDCIdx, uiMbYIdx, uiMbXIdx );
  UInt uiMbIndex = uiMbYIdx * m_uiWidthInMB + uiMbXIdx;


  //===== check if coefficient is significant or was already coded =====
  ROTRS( m_aapaucChromaDCCoefMap[uiPlane][uiDCIdx][uiMbIndex] & ( SIGNIFICANT | CODED ), Err::m_nOK );

  MbDataAccess* pcMbDataAccessEL  = 0;
  MbDataAccess* pcMbDataAccessBL  = 0;
  RNOK( m_cMbDataCtrlEL     .initMb( pcMbDataAccessEL, uiMbYIdx, uiMbXIdx ) );
  RNOK( m_pcCurrMbDataCtrl ->initMb( pcMbDataAccessBL, uiMbYIdx, uiMbXIdx ) );
  

  if( ! ( m_pauiMacroblockMap[uiMbIndex] & CHROMA_CBP_CODED ) )
  {
    //===== CHROMA CBP =====
    Bool  bSigCBP = m_pcCabacWriter->RQencodeCBP_Chroma( *pcMbDataAccessEL, *pcMbDataAccessBL );
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
        RNOK( m_pcCabacWriter->RQencodeDeltaQp( *pcMbDataAccessEL, *pcMbDataAccessBL ) );
        riLastQP = pcMbDataAccessEL->getMbData().getQp();

        m_pauiMacroblockMap[uiMbIndex] |= SIGNIFICANT;
      }
    }
  }



  if( ! ( m_apaucChromaDCBlockMap[uiPlane][uiMbIndex] & CODED ) )
  {
    Bool bSigBCBP = m_pcCabacWriter->RQencodeBCBP_ChromaDC( *pcMbDataAccessEL, *pcMbDataAccessBL, CIdx(4*uiPlane) );
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


  UInt  uiLast = 0;
  RNOK( m_pcCabacWriter->RQencodeNewTCoeff_Chroma( *pcMbDataAccessEL, *pcMbDataAccessBL,
                                                   CHROMA_DC, CIdx(4*uiPlane), uiDCIdx, uiLast, uiSigCtx ) );
  m_aapaucChromaDCCoefMap[uiPlane][uiDCIdx][uiMbIndex] |= CODED;
  if( pcMbDataAccessEL->getMbTCoeffs().get( CIdx(4*uiPlane) )[g_aucIndexChromaDCScan[uiDCIdx]] )
  {
    m_aapaucChromaDCCoefMap[uiPlane][uiDCIdx][uiMbIndex] |= SIGNIFICANT;
  }
  m_iRemainingTCoeff--;
  m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
  ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );
  
  if( uiLast )
  {
    for( UInt ui = uiDCIdx + 1; ui < 4; ui++ )
    {
      if( ! ( m_aapaucChromaDCCoefMap[uiPlane][ui][uiMbIndex] & SIGNIFICANT ) )
      {
        m_aapaucChromaDCCoefMap[uiPlane][ui][uiMbIndex] |= CODED;
        m_iRemainingTCoeff--;
        m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
        ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );
      }
    }
  }
  TWSOT( m_iRemainingTCoeff < 0 );
  
  return Err::m_nOK;
}






ErrVal
RQFGSEncoder::xEncodeNewCoefficientChromaAC ( UInt    uiPlane,
                                              UInt    uiB8YIdx,
                                              UInt    uiB8XIdx,
                                              UInt    uiScanIndex,
                                              Int&    riLastQP )
{
  UInt uiSigCtx   = xGetSigCtxChromaAC( uiPlane, uiScanIndex, uiB8YIdx, uiB8XIdx );

  UInt uiB8Index  = (uiB8YIdx  ) * 2 * m_uiWidthInMB + (uiB8XIdx  );
  UInt uiMbIndex  = (uiB8YIdx/2) * 1 * m_uiWidthInMB + (uiB8XIdx/2);
  UInt uiCIdx     = 4*uiPlane + 2*(uiB8YIdx%2) + (uiB8XIdx%2);

  CIdx cChromaIdx(uiCIdx);


  //===== check if coefficient is significant or was already coded =====
  ROTRS( m_aapaucChromaACCoefMap[uiPlane][uiScanIndex][uiB8Index] & ( SIGNIFICANT | CODED ), Err::m_nOK );

  MbDataAccess* pcMbDataAccessEL  = 0;
  MbDataAccess* pcMbDataAccessBL  = 0;
  RNOK( m_cMbDataCtrlEL     .initMb( pcMbDataAccessEL, uiB8YIdx/2, uiB8XIdx/2 ) );
  RNOK( m_pcCurrMbDataCtrl ->initMb( pcMbDataAccessBL, uiB8YIdx/2, uiB8XIdx/2 ) );
  

  if( ! ( m_pauiMacroblockMap[uiMbIndex] & CHROMA_CBP_CODED ) )
  {
    //===== CHROMA CBP =====
    Bool  bSigCBP = m_pcCabacWriter->RQencodeCBP_Chroma( *pcMbDataAccessEL, *pcMbDataAccessBL );
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
        RNOK( m_pcCabacWriter->RQencodeDeltaQp( *pcMbDataAccessEL, *pcMbDataAccessBL ) );
        riLastQP = pcMbDataAccessEL->getMbData().getQp();

        m_pauiMacroblockMap[uiMbIndex] |= SIGNIFICANT;
      }
    }
  }



  if( ! ( m_pauiMacroblockMap[uiMbIndex] & CHROMA_CBP_AC_CODED ) )
  {
    //===== CHROMA CBP =====
    Bool  bSigCBP = m_pcCabacWriter->RQencodeCBP_ChromaAC( *pcMbDataAccessEL, *pcMbDataAccessBL );
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
    Bool bSigBCBP = m_pcCabacWriter->RQencodeBCBP_ChromaAC( *pcMbDataAccessEL, *pcMbDataAccessBL, cChromaIdx );
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


  UInt  uiLast = 0;
  RNOK( m_pcCabacWriter->RQencodeNewTCoeff_Chroma( *pcMbDataAccessEL, *pcMbDataAccessBL,
                                                   CHROMA_AC, cChromaIdx, uiScanIndex, uiLast, uiSigCtx ) );
  m_aapaucChromaACCoefMap[uiPlane][uiScanIndex][uiB8Index] |= CODED;
  if( pcMbDataAccessEL->getMbTCoeffs().get( cChromaIdx )[g_aucFrameScan[uiScanIndex]] )
  {
    m_aapaucChromaACCoefMap[uiPlane][uiScanIndex][uiB8Index] |= SIGNIFICANT;
  }
  m_iRemainingTCoeff--;
  m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
  ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );
  
  if( uiLast )
  {
    for( UInt ui = uiScanIndex + 1; ui < 16; ui++ )
    {
      if( ! ( m_aapaucChromaACCoefMap[uiPlane][ui][uiB8Index] & SIGNIFICANT ) )
      {
        m_aapaucChromaACCoefMap[uiPlane][ui][uiB8Index] |= CODED;
        m_iRemainingTCoeff--;
        m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
        ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );
      }
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
    RNOK( m_pcCabacWriter->RQencodeTCoeffRef_8x8( *pcMbDataAccessEL, *pcMbDataAccessBL,
                                                  c8x8Idx, ui8x8ScanIndex ) );    
  }
  else
  {
    RNOK( m_pcCabacWriter->RQencodeTCoeffRef_Luma( *pcMbDataAccessEL, *pcMbDataAccessBL,
                                                   LUMA_SCAN, c4x4Idx, uiScanIndex ) );
  }

  m_apaucLumaCoefMap[uiScanIndex][uiBlockIndex] |= CODED;
  m_iRemainingTCoeff--;
  m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
  ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );
  TWSOT( m_iRemainingTCoeff < 0 );

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
  

  RNOK( m_pcCabacWriter->RQencodeTCoeffRef_Chroma( *pcMbDataAccessEL, *pcMbDataAccessBL,
                                                   CHROMA_DC, CIdx(4*uiPlane), uiDCIdx ) );

  m_aapaucChromaDCCoefMap[uiPlane][uiDCIdx][uiMbIndex] |= CODED;
  m_iRemainingTCoeff--;
  m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
  ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );
  TWSOT( m_iRemainingTCoeff < 0 );

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
  

  RNOK( m_pcCabacWriter->RQencodeTCoeffRef_Chroma( *pcMbDataAccessEL, *pcMbDataAccessBL,
                                                   CHROMA_AC, cChromaIdx, uiScanIdx ) );

  m_aapaucChromaACCoefMap[uiPlane][uiScanIdx][uiB8Index] |= CODED;
  m_iRemainingTCoeff--;
  m_pauiMacroblockMap[uiMbIndex] += ( 1 << NUM_COEFF_SHIFT );
  ROT( (m_pauiMacroblockMap[uiMbIndex]>>NUM_COEFF_SHIFT) > 384 );
  TWSOT( m_iRemainingTCoeff < 0 );

  return Err::m_nOK;
}




ErrVal
RQFGSEncoder::xUpdateMacroblock( MbDataAccess&  rcMbDataAccessBL,
                                 MbDataAccess&  rcMbDataAccessEL,
                                 UInt           uiMbY,
                                 UInt           uiMbX )
{
  UInt  uiExtCbp  = 0;
  Bool  b8x8      = rcMbDataAccessBL.getMbData().isTransformSize8x8();
  UInt  uiMbIndex = uiMbY*m_uiWidthInMB + uiMbX;

  //===== luma =====
  if( b8x8 )
  {
    for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
    {
      Bool    bSig      = false;
      TCoeff* piCoeffBL = rcMbDataAccessBL.getMbTCoeffs().get8x8( c8x8Idx );
      TCoeff* piCoeffEL = rcMbDataAccessEL.getMbTCoeffs().get8x8( c8x8Idx );

      for( UInt ui8x8ScanIdx = 0; ui8x8ScanIdx < 64; ui8x8ScanIdx++ )
      {
        UInt  uiPos         = g_aucFrameScan64[ui8x8ScanIdx];
        UInt  ui4x4ScanIdx  = ui8x8ScanIdx / 4;
        UInt  uiBlk4x4Idx   = ui8x8ScanIdx % 4;
        UInt  uiBlockIndex  = (4*uiMbY+c8x8Idx.y()+(uiBlk4x4Idx/2))*4*m_uiWidthInMB + (4*uiMbX+c8x8Idx.x()+(uiBlk4x4Idx%2));

        if( m_apaucLumaCoefMap[ui4x4ScanIdx][uiBlockIndex] & CODED )
        {
          piCoeffBL[uiPos] += piCoeffEL[uiPos];
        }
        if( piCoeffBL[uiPos] )
        {
          bSig = true;
        }
      }
      if( bSig )
      {
        uiExtCbp |= ( 0x33 << c8x8Idx.b4x4() );
      }
    }
  }
  else
  {
    for( B4x4Idx c4x4Idx; c4x4Idx.isLegal(); c4x4Idx++ )
    {
      Bool    bSig      = false;
      TCoeff* piCoeffBL = rcMbDataAccessBL.getMbTCoeffs().get( c4x4Idx );
      TCoeff* piCoeffEL = rcMbDataAccessEL.getMbTCoeffs().get( c4x4Idx );

      for( UInt uiScanIdx = 0; uiScanIdx < 16; uiScanIdx++ )
      {
        UInt  uiPos         = g_aucFrameScan[uiScanIdx];
        UInt  uiBlockIndex  = (4*uiMbY+c4x4Idx.y())*4*m_uiWidthInMB + (4*uiMbX+c4x4Idx.x());

        if( m_apaucLumaCoefMap[uiScanIdx][uiBlockIndex] & CODED )
        {
          piCoeffBL[uiPos] += piCoeffEL[uiPos];
        }
        if( piCoeffBL[uiPos] )
        {
          bSig = true;
        }
      }
      if( bSig )
      {
        uiExtCbp |= ( 1 << c4x4Idx );
      }
    }
  }


  //===== chroma DC =====
  Bool  bSigDC = false;
  for( UInt uiPlane = 0; uiPlane < 2; uiPlane++ )
  {
    TCoeff* piCoeffBL = rcMbDataAccessBL.getMbTCoeffs().get( CIdx(4*uiPlane) );
    TCoeff* piCoeffEL = rcMbDataAccessEL.getMbTCoeffs().get( CIdx(4*uiPlane) );

    for( UInt uiDCIdx = 0; uiDCIdx < 4; uiDCIdx++ )
    {
      UInt uiPos = g_aucIndexChromaDCScan[uiDCIdx];

      piCoeffBL[uiPos] += piCoeffEL[uiPos];

      if( piCoeffBL[uiPos] )
      {
        bSigDC = true;
      }
    }
  }

  //===== chroma AC =====
  Bool  bSigAC = false;
  for( CIdx cCIdx; cCIdx.isLegal(); cCIdx++ )
  {
    TCoeff* piCoeffBL = rcMbDataAccessBL.getMbTCoeffs().get( cCIdx );
    TCoeff* piCoeffEL = rcMbDataAccessEL.getMbTCoeffs().get( cCIdx );

    for( UInt uiScanIdx = 1; uiScanIdx < 16; uiScanIdx++ )
    {
      UInt  uiPos     = g_aucFrameScan[uiScanIdx];
      UInt  ui8x8Idx  = (2*uiMbY+cCIdx.y())*2*m_uiWidthInMB + (2*uiMbX+cCIdx.x());

      if( m_aapaucChromaACCoefMap[cCIdx.plane()][uiScanIdx][ui8x8Idx] & CODED )
      {
        piCoeffBL[uiPos] += piCoeffEL[uiPos];
      }
      if( piCoeffBL[uiPos] )
      {
        bSigAC = true;
      }
    }
  }

  
  //===== set CBP =====
  UInt  uiChromaCBP = ( bSigAC ? 2 : bSigDC ? 1 : 0 );
  uiExtCbp         |= ( uiChromaCBP << 16 );
  rcMbDataAccessBL.getMbData().setAndConvertMbExtCbp( uiExtCbp );


  //===== set QP =====
  Int iELQP     = rcMbDataAccessEL.getMbData().getQp();
  Int iNumCoded = ( m_pauiMacroblockMap[uiMbIndex] >> NUM_COEFF_SHIFT );
  Int iQPDelta  = ( 384 - iNumCoded ) / 64;
  Int iQP       = min( 51, iELQP + iQPDelta );
  if( ! ( m_pauiMacroblockMap[uiMbIndex] & SIGNIFICANT ) )
  {
    iQP = rcMbDataAccessEL.getSH().getPicQp();
  }
  rcMbDataAccessBL.getMbData().setQp( iQP );

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




H264AVC_NAMESPACE_END

