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
#include "H264AVCCommonLib/Tables.h"
#include "H264AVCCommonLib/ContextTables.h"
#include "H264AVCCommonLib/TraceFile.h"

#include "CabaEncoder.h"
#include "CabacWriter.h"


H264AVC_NAMESPACE_BEGIN


const int MAX_COEFF[9] = { 8,16,16,16,15, 4, 4,15,15};
const int COUNT_THR[9] = { 3, 4, 4, 4, 3, 2, 2, 3, 3};


CabacWriter::CabacWriter():
  m_cBLFlagCCModel(1,1),
  m_cBLSkipCCModel(1,4),
  m_cBLQRefCCModel(1,4),
  m_cBCbpCCModel( NUM_BLOCK_TYPES, NUM_BCBP_CTX ),
  m_cMapCCModel( NUM_BLOCK_TYPES, NUM_MAP_CTX ),
  m_cLastCCModel( NUM_BLOCK_TYPES, NUM_LAST_CTX ),
  m_cRefCCModel( 1, 10 ),
  m_cSigCCModel( 1, 10 ),
  m_cOneCCModel( NUM_BLOCK_TYPES, NUM_ABS_CTX ),
  m_cAbsCCModel( NUM_BLOCK_TYPES, NUM_ABS_CTX ),
  m_cChromaPredCCModel( 1, 4 ),
  m_cMbTypeCCModel( 3, NUM_MB_TYPE_CTX ),
  m_cBlockTypeCCModel( 2, NUM_B8_TYPE_CTX ),
  m_cMvdCCModel( 2, NUM_MV_RES_CTX ),
  m_cRefPicCCModel( 2, NUM_REF_NO_CTX ),
  m_cBLPredFlagCCModel( 2, NUM_BL_PRED_FLAG_CTX ),
  m_cResPredFlagCCModel( 1, 4 ),
  m_cDeltaQpCCModel( 1, NUM_DELTA_QP_CTX ),
  m_cIntraPredCCModel( 9, NUM_IPR_CTX ),
  m_cCbpCCModel( 3, NUM_CBP_CTX ),
  m_cBCbpEnhanceCCModel( NUM_BLOCK_TYPES, NUM_BCBP_CTX ),
  m_cCbpEnhanceCCModel( 3, NUM_CBP_CTX ),
  m_cTransSizeCCModel( 1, NUM_TRANSFORM_SIZE_CTX ),
  m_pcSliceHeader( NULL ),
  m_uiBitCounter( 0 ),
  m_uiPosCounter( 0 ),
  m_uiLastDQpNonZero(0),
  m_bTraceEnable(true)
{
}

CabacWriter::~CabacWriter()
{
}


ErrVal CabacWriter::xInitContextModels( const SliceHeader& rcSliceHeader )
{
  Int  iQp    = rcSliceHeader.getPicQp();
  Bool bIntra = rcSliceHeader.isIntra();
  Int  iIndex = rcSliceHeader.getCabacInitIdc();

  if( bIntra )
  {
    RNOK( m_cMbTypeCCModel.initBuffer(      (Short*)INIT_MB_TYPE_I,       iQp ) );
    RNOK( m_cBlockTypeCCModel.initBuffer(   (Short*)INIT_B8_TYPE_I,       iQp ) );
    RNOK( m_cMvdCCModel.initBuffer(         (Short*)INIT_MV_RES_I,        iQp ) );
    RNOK( m_cRefPicCCModel.initBuffer(      (Short*)INIT_REF_NO_I,        iQp ) );
    RNOK( m_cBLPredFlagCCModel.initBuffer(  (Short*)INIT_BL_PRED_FLAG_I,  iQp ) );
    RNOK( m_cResPredFlagCCModel.initBuffer( (Short*)INIT_RES_PRED_FLAG_I, iQp ) );
    RNOK( m_cDeltaQpCCModel.initBuffer(     (Short*)INIT_DELTA_QP_I,      iQp ) );
    RNOK( m_cIntraPredCCModel.initBuffer(   (Short*)INIT_IPR_I,           iQp ) );
    RNOK( m_cChromaPredCCModel.initBuffer(  (Short*)INIT_CIPR_I,          iQp ) );
    RNOK( m_cBLFlagCCModel.initBuffer(      (Short*)INIT_BL_FLAG,         iQp ) );
    RNOK( m_cBLSkipCCModel.initBuffer(      (Short*)INIT_BL_SKIP,         iQp ) );
    RNOK( m_cBLQRefCCModel.initBuffer(      (Short*)INIT_BL_QREF,         iQp ) );

    RNOK( m_cCbpCCModel.initBuffer(         (Short*)INIT_CBP_I,           iQp ) );
    RNOK( m_cBCbpCCModel.initBuffer(        (Short*)INIT_BCBP_I,          iQp ) );
    RNOK( m_cMapCCModel.initBuffer(         (Short*)INIT_MAP_I,           iQp ) );
    RNOK( m_cLastCCModel.initBuffer(        (Short*)INIT_LAST_I,          iQp ) );
    RNOK( m_cRefCCModel.initBuffer(         (Short*)INIT_REF,             iQp ) );
    RNOK( m_cSigCCModel.initBuffer(         (Short*)INIT_SIG,             iQp ) );
    RNOK( m_cOneCCModel.initBuffer(         (Short*)INIT_ONE_I,           iQp ) );
    RNOK( m_cAbsCCModel.initBuffer(         (Short*)INIT_ABS_I,           iQp ) );

    RNOK( m_cCbpEnhanceCCModel.initBuffer(  (Short*)INIT_CBP_ENH_I,       iQp ) );
    RNOK( m_cBCbpEnhanceCCModel.initBuffer( (Short*)INIT_BCBP_ENH_I,      iQp ) );

    RNOK( m_cTransSizeCCModel.initBuffer(   (Short*)INIT_TRANSFORM_SIZE_I,iQp ) );
  }
  else
  {
    RNOK( m_cMbTypeCCModel.initBuffer(      (Short*)INIT_MB_TYPE_P      [iIndex], iQp ) );
    RNOK( m_cBlockTypeCCModel.initBuffer(   (Short*)INIT_B8_TYPE_P      [iIndex], iQp ) );
    RNOK( m_cMvdCCModel.initBuffer(         (Short*)INIT_MV_RES_P       [iIndex], iQp ) );
    RNOK( m_cRefPicCCModel.initBuffer(      (Short*)INIT_REF_NO_P       [iIndex], iQp ) );
    RNOK( m_cBLPredFlagCCModel.initBuffer(  (Short*)INIT_BL_PRED_FLAG_P [iIndex], iQp ) );
    RNOK( m_cResPredFlagCCModel.initBuffer( (Short*)INIT_RES_PRED_FLAG_P[iIndex], iQp ) );
    RNOK( m_cDeltaQpCCModel.initBuffer(     (Short*)INIT_DELTA_QP_P     [iIndex], iQp ) );
    RNOK( m_cIntraPredCCModel.initBuffer(   (Short*)INIT_IPR_P          [iIndex], iQp ) );
    RNOK( m_cChromaPredCCModel.initBuffer(  (Short*)INIT_CIPR_P         [iIndex], iQp ) );
    RNOK( m_cBLFlagCCModel.initBuffer(      (Short*)INIT_BL_FLAG,                 iQp ) );
    RNOK( m_cBLSkipCCModel.initBuffer(      (Short*)INIT_BL_SKIP,                 iQp ) );
    RNOK( m_cBLQRefCCModel.initBuffer(      (Short*)INIT_BL_QREF,                 iQp ) );

    RNOK( m_cCbpCCModel.initBuffer(         (Short*)INIT_CBP_P          [iIndex], iQp ) );
    RNOK( m_cBCbpCCModel.initBuffer(        (Short*)INIT_BCBP_P         [iIndex], iQp ) );
    RNOK( m_cMapCCModel.initBuffer(         (Short*)INIT_MAP_P          [iIndex], iQp ) );
    RNOK( m_cLastCCModel.initBuffer(        (Short*)INIT_LAST_P         [iIndex], iQp ) );
    RNOK( m_cSigCCModel.initBuffer(         (Short*)INIT_SIG                    , iQp ) );
    RNOK( m_cRefCCModel.initBuffer(         (Short*)INIT_REF                    , iQp ) );
    RNOK( m_cOneCCModel.initBuffer(         (Short*)INIT_ONE_P          [iIndex], iQp ) );
    RNOK( m_cAbsCCModel.initBuffer(         (Short*)INIT_ABS_P          [iIndex], iQp ) );

    RNOK( m_cCbpEnhanceCCModel.initBuffer(  (Short*)INIT_CBP_ENH_P      [iIndex], iQp ) );
    RNOK( m_cBCbpEnhanceCCModel.initBuffer( (Short*)INIT_BCBP_ENH_P     [iIndex], iQp ) );

    RNOK( m_cTransSizeCCModel.initBuffer(   (Short*)INIT_TRANSFORM_SIZE_P,        iQp ) );
  }  

  return Err::m_nOK;
}



ErrVal CabacWriter::create( CabacWriter*& rpcCabacWriter )
{
  rpcCabacWriter = new CabacWriter;

  ROT( NULL == rpcCabacWriter );

  return Err::m_nOK;
}

ErrVal CabacWriter::destroy()
{
  delete this;
  return Err::m_nOK;
}

ErrVal CabacWriter::init(  BitWriteBufferIf *pcBitWriteBufferIf )
{
  ROT( NULL == pcBitWriteBufferIf );

  RNOK( CabaEncoder::init( pcBitWriteBufferIf ) );

  return Err::m_nOK;
}

ErrVal CabacWriter::uninit()
{
  RNOK( CabaEncoder::uninit() );
  return Err::m_nOK;
}


ErrVal CabacWriter::startSlice( const SliceHeader& rcSliceHeader )
{
  m_pcSliceHeader     = &rcSliceHeader;
  m_uiLastDQpNonZero  = 0;

  RNOK( xInitContextModels( rcSliceHeader ) );

  RNOK( CabaEncoder::start() );

  return Err::m_nOK;
}


ErrVal CabacWriter::finishSlice()
{
  RNOK( CabaEncoder::finish() );

  return Err::m_nOK;
}


ErrVal CabacWriter::refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx )
{
  UInt uiRefFrame = rcMbDataAccess.getMbMotionData( eLstIdx ).getRefIdx();
  RNOK( xRefFrame( rcMbDataAccess, uiRefFrame, eLstIdx, PART_8x8_0 ) );
  return Err::m_nOK;
}

ErrVal CabacWriter::refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  )
{
  UInt uiRefFrame = rcMbDataAccess.getMbMotionData( eLstIdx ).getRefIdx( eParIdx );
  RNOK( xRefFrame( rcMbDataAccess, uiRefFrame, eLstIdx, ParIdx8x8( eParIdx ) ) );
  return Err::m_nOK;
}

ErrVal CabacWriter::refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  )
{
  UInt uiRefFrame = rcMbDataAccess.getMbMotionData( eLstIdx ).getRefIdx( eParIdx );
  RNOK( xRefFrame( rcMbDataAccess, uiRefFrame, eLstIdx, ParIdx8x8( eParIdx ) ) );
  return Err::m_nOK;
}

ErrVal CabacWriter::refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  )
{
  UInt uiRefFrame = rcMbDataAccess.getMbMotionData( eLstIdx ).getRefIdx( eParIdx );
  RNOK( xRefFrame( rcMbDataAccess, uiRefFrame, eLstIdx, ParIdx8x8( eParIdx ) ) );
  return Err::m_nOK;
}


ErrVal CabacWriter::motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx )
{
  return xMotionPredFlag( rcMbDataAccess, rcMbDataAccess.getMbMotionData( eLstIdx ).getMotPredFlag(), eLstIdx, PART_8x8_0 );
}
ErrVal CabacWriter::motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx )
{
  return xMotionPredFlag( rcMbDataAccess, rcMbDataAccess.getMbMotionData( eLstIdx ).getMotPredFlag(eParIdx), eLstIdx, ParIdx8x8( eParIdx ) );
}
ErrVal CabacWriter::motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx )
{
  return xMotionPredFlag( rcMbDataAccess, rcMbDataAccess.getMbMotionData( eLstIdx ).getMotPredFlag(eParIdx), eLstIdx, ParIdx8x8( eParIdx ) );
}
ErrVal CabacWriter::motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8 eParIdx )
{
  return xMotionPredFlag( rcMbDataAccess, rcMbDataAccess.getMbMotionData( eLstIdx ).getMotPredFlag(eParIdx), eLstIdx, ParIdx8x8( eParIdx ) );
}


ErrVal CabacWriter::xRefFrame( MbDataAccess& rcMbDataAccess, UInt uiRefFrame, ListIdx eLstIdx, ParIdx8x8 eParIdx )
{
  AOT_DBG( (Int)uiRefFrame == -1 );
  AOT_DBG( (Int)uiRefFrame == -2 );
  AOT_DBG( (Int)uiRefFrame == 0 );

  UInt uiCtx = rcMbDataAccess.getCtxRefIdx( eLstIdx, eParIdx );

  RNOK( CabaEncoder::writeSymbol( ( uiRefFrame==1 ? 0 : 1 ), m_cRefPicCCModel.get( 0, uiCtx ) ) );

  if ( uiRefFrame > 1 )
  {
    RNOK( CabaEncoder::writeUnarySymbol( uiRefFrame-2, &m_cRefPicCCModel.get( 0, 4 ), 1 ) );
  }

  ETRACE_T( "RefFrame" );
  ETRACE_TY( "ae(v)" );
  ETRACE_CODE(uiRefFrame);
  ETRACE_N;

  return Err::m_nOK;
}


ErrVal CabacWriter::xMotionPredFlag( MbDataAccess& rcMbDataAccess, Bool bFlag, ListIdx eLstIdx, ParIdx8x8 eParIdx )
{
  UInt  uiCode  = ( bFlag ? 1: 0 );

  UInt  uiCtx = 0; // version 1
  RNOK( CabaEncoder::writeSymbol( uiCode, m_cBLPredFlagCCModel.get( eLstIdx, uiCtx ) ) );

  ETRACE_T( "MotionPredFlag" );
  ETRACE_TY( "ae(v)" );
  ETRACE_CODE( uiCode );
  ETRACE_N;

  return Err::m_nOK;
}


ErrVal CabacWriter::blockModes( MbDataAccess& rcMbDataAccess )
{
  for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
  {
    RNOK( xWriteBlockMode( rcMbDataAccess.getConvertBlkMode( c8x8Idx.b8x8Index() ) ) )
  }
  return Err::m_nOK;
}

ErrVal CabacWriter::xWriteBlockMode( UInt uiBlockMode )
{
  UInt uiSymbol;

  if( ! m_pcSliceHeader->isInterB() )
  {
    uiSymbol = (0 == uiBlockMode) ? 1 : 0;
    RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cBlockTypeCCModel.get( 0, 1 ) ) );
    if( !uiSymbol )
    {
      uiSymbol = (1 == uiBlockMode) ? 0 : 1;
      RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cBlockTypeCCModel.get( 0, 3 ) ) );
      if( uiSymbol )
      {
        uiSymbol = (2 == uiBlockMode) ? 1:0;
        RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cBlockTypeCCModel.get( 0, 4 ) ) );
      }
    }
  }
  else
  {
    uiSymbol = ( uiBlockMode ? 1 : 0 );
    RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cBlockTypeCCModel.get( 1, 0 ) ) );
    if( uiSymbol )
    {
      uiSymbol = (3 > uiBlockMode) ? 0 : 1;
      RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cBlockTypeCCModel.get( 1, 1 ) ) );
      if( uiSymbol )
      {
        uiSymbol = (7 > uiBlockMode) ? 0 : 1;
        RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cBlockTypeCCModel.get( 1, 2 ) ) );
        if( uiSymbol )
        {
          uiBlockMode -= 7;
          uiSymbol = ( uiBlockMode >> 2 ) & 1;
          RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cBlockTypeCCModel.get( 1, 3 ) ) );
          if( ! uiSymbol )
          {
            uiSymbol = ( uiBlockMode >> 1 ) & 1;
            RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cBlockTypeCCModel.get( 1, 3 ) ) );
          }
          uiSymbol = uiBlockMode & 1;
          RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cBlockTypeCCModel.get( 1, 3 ) ) );
          uiBlockMode += 7; // just for correct trace file
        }
        else
        {
          uiSymbol = (5 > uiBlockMode) ? 0 : 1;
          RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cBlockTypeCCModel.get( 1, 3 ) ) );
          uiSymbol = ( 1 == (1 & uiBlockMode) ) ? 0 : 1;
          RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cBlockTypeCCModel.get( 1, 3 ) ) );
        }
      }
      else
      {
        uiSymbol = (1 == uiBlockMode) ? 0 : 1;
        RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cBlockTypeCCModel.get( 1, 3 ) ) );
      }
    }
  }

  ETRACE_T( "BlockMode" );
  ETRACE_TY( "ae(v)" );
  ETRACE_CODE(uiBlockMode);
  ETRACE_N;

  return Err::m_nOK;
}



ErrVal CabacWriter::blFlag( MbDataAccess& rcMbDataAccess )
{
  UInt uiSymbol = ( rcMbDataAccess.getMbData().getMbMode() == INTRA_BL ? 1 : 0 );

  RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cBLFlagCCModel.get( 0, 0 ) ) );

  ETRACE_T( "BLFlag:" );
  ETRACE_TY( "ae(v)" );
  ETRACE_CODE( uiSymbol );
  ETRACE_N;

  return Err::m_nOK;
}


ErrVal CabacWriter::skipFlag( MbDataAccess& rcMbDataAccess, Bool bNotAllowed )
{
  ROTRS( m_pcSliceHeader->isIntra(), Err::m_nOK );

  UInt uiSymbol = bNotAllowed ? 0 : rcMbDataAccess.isSkippedMb() ? 1 : 0;

  if( m_pcSliceHeader->isInterB() )
  {
    RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cMbTypeCCModel.get( 2, 7 + rcMbDataAccess.getCtxDirectMbWoCoeff() ) ) );
    rcMbDataAccess.getMbData().setSkipFlag(uiSymbol!=0);
  }
  else
  {
    RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cMbTypeCCModel.get( 1, rcMbDataAccess.getCtxMbSkipped() ) ) );
  }


  ROTRS( 0 == uiSymbol, Err::m_nOK );

  m_uiLastDQpNonZero = 0; // no DeltaQP for Skipped Macroblock
  ETRACE_T( "MbMode" );
  ETRACE_TY( "ae(v)" );
  ETRACE_CODE( 0 );
  ETRACE_N;

  return Err::m_nOK;
}


ErrVal CabacWriter::BLSkipFlag( MbDataAccess& rcMbDataAccess )
{
  UInt uiSymbol = rcMbDataAccess.getMbData().getBLSkipFlag() ? 1 : 0;
  UInt uiCtx    = rcMbDataAccess.getCtxBLSkipFlag();

  RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cBLSkipCCModel.get( 0, uiCtx ) ) );

  ETRACE_T( "BLSkipFlag" );
  ETRACE_TY( "ae(v)" );
  ETRACE_CODE( uiSymbol );
  ETRACE_N;

  return Err::m_nOK;
}


ErrVal CabacWriter::BLQRefFlag( MbDataAccess& rcMbDataAccess )
{
  UInt uiSymbol = rcMbDataAccess.getMbData().getBLQRefFlag() ? 1 : 0;
  UInt uiCtx    = 0;

  RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cBLQRefCCModel.get( 0, uiCtx ) ) );

  ETRACE_T( "BLQRefFlag" );
  ETRACE_TY( "ae(v)" );
  ETRACE_CODE( uiSymbol );
  ETRACE_N;

  return Err::m_nOK;
}


ErrVal CabacWriter::resPredFlag( MbDataAccess& rcMbDataAccess )
{
  UInt  uiSymbol = ( rcMbDataAccess.getMbData().getResidualPredFlag( PART_16x16 ) ? 1 : 0 );

  UInt  uiCtx = 0;
  RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cResPredFlagCCModel.get( 0, uiCtx ) ) );

  ETRACE_T( "ResidualPredFlag " );
  ETRACE_TY( "ae(v)" );
  ETRACE_CODE( uiSymbol );
  ETRACE_N;

  return Err::m_nOK;
}

ErrVal CabacWriter::mbMode( MbDataAccess& rcMbDataAccess )
{
  UInt uiMbMode     = rcMbDataAccess.getConvertMbType();
  UInt uiOrigMbMode = uiMbMode;

  if( m_pcSliceHeader->isIntra() )
  {
    UInt uiSymbol;
    uiSymbol = ( 0 == uiMbMode) ? 0 : 1;
    RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cMbTypeCCModel.get( 0, rcMbDataAccess.getCtxMbIntra4x4() ) ) );

    if( uiSymbol )
    {
      uiSymbol = ( 25 == uiMbMode) ? 1 : 0;
      RNOK( CabaEncoder::writeTerminatingBit( uiSymbol ) );

      if( ! uiSymbol )
      {
        uiSymbol = ( 13 > uiMbMode) ? 0 : 1;
        RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cMbTypeCCModel.get( 0, 4 ) ) );
        uiMbMode -= 12* uiSymbol + 1;

        uiSymbol = ( 4 > uiMbMode) ? 0 : 1;
        RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cMbTypeCCModel.get( 0, 5 ) ) );

        if( uiSymbol )
        {
          uiSymbol = ( 8 > uiMbMode) ? 0 : 1;
          RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cMbTypeCCModel.get( 0, 6 ) ) );
        }

        uiSymbol = (uiMbMode>>1) & 1;
        RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cMbTypeCCModel.get( 0, 7 ) ) );

        uiSymbol = uiMbMode & 1;
        RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cMbTypeCCModel.get( 0, 8 ) ) );
      }
    }
  }
  else
  {
    UInt uiSymbol, uiIntra16x16Symbol = 0;

    if( ! m_pcSliceHeader->isInterB() )
    {
      uiSymbol = ( 6 > uiMbMode) ? 0 : 1;
      RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cMbTypeCCModel.get( 1, 4 ) ) );
      if( uiSymbol )
      {
        uiSymbol = ( 6 == uiMbMode) ? 0 : 1;
        RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cMbTypeCCModel.get( 1, 7 ) ) );

        if( uiSymbol )
        {
          uiIntra16x16Symbol = uiMbMode - 6;
        }
      }
      else
      {
        uiSymbol = (uiMbMode>>1) & 1;
        RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cMbTypeCCModel.get( 1, 5 ) ) );
        if( uiSymbol )
        {
          uiSymbol = 1-(uiMbMode&1);
          RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cMbTypeCCModel.get( 1, 7 ) ) );
        }
        else
        {
          uiSymbol = 1-(uiMbMode&1);
          RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cMbTypeCCModel.get( 1, 6 ) ) );
        }
      }
    }
    else
    {
      ROFRS( uiMbMode, Err::m_nERR );

      uiMbMode--;
      uiSymbol = ( uiMbMode ? 1 : 0 );
      RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cMbTypeCCModel.get( 2, rcMbDataAccess.getCtxMbType() ) ) );
      if( uiSymbol )
      {
        uiSymbol = ( 3 > uiMbMode) ? 0 : 1;
        RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cMbTypeCCModel.get( 2, 4 ) ) );
        if( uiSymbol )
        {
          uiSymbol = ( 11 > uiMbMode) ? 0 : 1;
          RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cMbTypeCCModel.get( 2, 5 ) ) );
          if( uiSymbol )
          {
            if( ( uiSymbol = ( 22 == uiMbMode ) ) || 11 == uiMbMode )
            {
              RNOK( CabaEncoder::writeSymbol(        1, m_cMbTypeCCModel.get( 2, 6 ) ) );
              RNOK( CabaEncoder::writeSymbol(        1, m_cMbTypeCCModel.get( 2, 6 ) ) );
              RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cMbTypeCCModel.get( 2, 6 ) ) );
            }
            else
            {
              if( uiMbMode < 23 )
              {
                uiMbMode -= 12;
              }
              else if( uiMbMode < 24 )
              {
                uiMbMode -= 13;
              }
              else
              {
                uiIntra16x16Symbol = uiMbMode - 23;
                uiMbMode = 11;
              }

              uiSymbol   = (uiMbMode>>3) & 1;
              RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cMbTypeCCModel.get( 2, 6 ) ) );
              uiSymbol   = (uiMbMode>>2) & 1;
              RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cMbTypeCCModel.get( 2, 6 ) ) );
              uiSymbol   = (uiMbMode>>1) & 1;
              RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cMbTypeCCModel.get( 2, 6 ) ) );
              uiSymbol   = uiMbMode & 1;
              RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cMbTypeCCModel.get( 2, 6 ) ) );
            }
          }
          else
          {
            uiMbMode -= 3;
            uiSymbol = (uiMbMode>>2) & 1;
            RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cMbTypeCCModel.get( 2, 6 ) ) );
            uiSymbol = (uiMbMode>>1) & 1;
            RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cMbTypeCCModel.get( 2, 6 ) ) );
            uiSymbol = uiMbMode & 1;
            RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cMbTypeCCModel.get( 2, 6 ) ) );
          }
        }
        else
        {
          uiSymbol = uiMbMode >> 1;
          RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cMbTypeCCModel.get( 2, 6 ) ) );
        }
      }
    }

    if( uiIntra16x16Symbol )
    {
      uiSymbol = ( 25 == uiIntra16x16Symbol) ? 1 : 0;
      RNOK( CabaEncoder::writeTerminatingBit( uiSymbol ) );

      if( ! uiSymbol )
      {
        uiSymbol = ( uiIntra16x16Symbol < 13 ? 0 : 1 );
        RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cMbTypeCCModel.get( 1, 8 ) ) );
        uiIntra16x16Symbol -= ( 12 * uiSymbol + 1 );

        uiSymbol = ( 4 > uiIntra16x16Symbol) ? 0 : 1;
        RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cMbTypeCCModel.get( 1, 9 ) ) );

        if( uiSymbol )
        {
          uiSymbol = ( 8 > uiIntra16x16Symbol) ? 0 : 1;
          RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cMbTypeCCModel.get( 1, 9 ) ) );
        }

        uiSymbol = (uiIntra16x16Symbol>>1) & 1;
        RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cMbTypeCCModel.get( 1, 10 ) ) );

        uiSymbol = uiIntra16x16Symbol & 1;
        RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cMbTypeCCModel.get( 1, 10 ) ) );
      }
    }
  }

  ETRACE_T( "MbMode" );
  ETRACE_TY( "ae(v)" );
  ETRACE_CODE( uiOrigMbMode );
  ETRACE_N;

  return Err::m_nOK;
}





ErrVal CabacWriter::xWriteMvdComponent( Short sMvdComp, UInt uiAbsSum, UInt uiCtx )
{
  //--- set context ---
  UInt uiLocalCtx = uiCtx;
  if( uiAbsSum >= 3)
  {
    uiLocalCtx += ( uiAbsSum > 32) ? 3 : 2;
  }

  //--- first symbol: if non-zero ---
  UInt uiSymbol = ( 0 == sMvdComp) ? 0 : 1;
  RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cMvdCCModel.get( 0, uiLocalCtx ) ) );
  ROTRS( 0 == uiSymbol, Err::m_nOK );

  //--- absolute value and sign
  UInt uiSign = 0;
  if( 0 > sMvdComp )
  {
    uiSign   = 1;
    sMvdComp = -sMvdComp;
  }
  RNOK( CabaEncoder::writeExGolombMvd( sMvdComp-1, &m_cMvdCCModel.get( 1, uiCtx ), 3 ) );
  RNOK( CabaEncoder::writeEPSymbol( uiSign ) );

  return Err::m_nOK;
}


ErrVal CabacWriter::mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx )
{
  Mv cMv = rcMbDataAccess.getMbMvdData( eLstIdx ).getMv();
  RNOK( xWriteMvd( rcMbDataAccess, cMv, B4x4Idx(0), eLstIdx ) );
  return Err::m_nOK;
}

ErrVal CabacWriter::mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  )
{
  Mv cMv = rcMbDataAccess.getMbMvdData( eLstIdx ).getMv( eParIdx );
  RNOK( xWriteMvd( rcMbDataAccess, cMv, B4x4Idx(eParIdx), eLstIdx ) );
  return Err::m_nOK;
}

ErrVal CabacWriter::mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  )
{
  Mv cMv = rcMbDataAccess.getMbMvdData( eLstIdx ).getMv( eParIdx );
  RNOK( xWriteMvd( rcMbDataAccess, cMv, B4x4Idx(eParIdx), eLstIdx ) );
  return Err::m_nOK;
}

ErrVal CabacWriter::mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  )
{
  Mv cMv = rcMbDataAccess.getMbMvdData( eLstIdx ).getMv( eParIdx );
  RNOK( xWriteMvd( rcMbDataAccess, cMv, B4x4Idx(eParIdx), eLstIdx ) );
  return Err::m_nOK;
}

ErrVal CabacWriter::mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx8x4 eSParIdx )
{
  Mv cMv = rcMbDataAccess.getMbMvdData( eLstIdx ).getMv( eParIdx, eSParIdx );
  RNOK( xWriteMvd( rcMbDataAccess, cMv, B4x4Idx(eParIdx+eSParIdx), eLstIdx ) );
  return Err::m_nOK;
}

ErrVal CabacWriter::mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx4x8 eSParIdx )
{
  Mv cMv = rcMbDataAccess.getMbMvdData( eLstIdx ).getMv( eParIdx, eSParIdx );
  RNOK( xWriteMvd( rcMbDataAccess, cMv, B4x4Idx(eParIdx+eSParIdx), eLstIdx ) );
  return Err::m_nOK;
}

ErrVal CabacWriter::mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx4x4 eSParIdx )
{
  Mv cMv = rcMbDataAccess.getMbMvdData( eLstIdx ).getMv( eParIdx, eSParIdx );
  RNOK( xWriteMvd( rcMbDataAccess, cMv, B4x4Idx(eParIdx+eSParIdx), eLstIdx ) );
  return Err::m_nOK;
}


ErrVal  CabacWriter::mvdQPel ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx                      )
{
  Mv cMv = rcMbDataAccess.getMbMvdData( eLstIdx ).getMv();
  RNOK( xWriteMvdQPel( rcMbDataAccess, cMv, B4x4Idx(0), eLstIdx ) );
  return Err::m_nOK;
}
ErrVal  CabacWriter::mvdQPel ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  )
{
  Mv cMv = rcMbDataAccess.getMbMvdData( eLstIdx ).getMv( eParIdx );
  RNOK( xWriteMvdQPel( rcMbDataAccess, cMv, B4x4Idx(eParIdx), eLstIdx ) );
  return Err::m_nOK;
}
ErrVal  CabacWriter::mvdQPel ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  )
{
  Mv cMv = rcMbDataAccess.getMbMvdData( eLstIdx ).getMv( eParIdx );
  RNOK( xWriteMvdQPel( rcMbDataAccess, cMv, B4x4Idx(eParIdx), eLstIdx ) );
  return Err::m_nOK;
}
ErrVal  CabacWriter::mvdQPel ( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  )
{
  Mv cMv = rcMbDataAccess.getMbMvdData( eLstIdx ).getMv( eParIdx );
  RNOK( xWriteMvdQPel( rcMbDataAccess, cMv, B4x4Idx(eParIdx), eLstIdx ) );
  return Err::m_nOK;
}


ErrVal CabacWriter::xWriteMvdQPel( MbDataAccess& rcMbDataAccess, Mv cMv, LumaIdx cIdx, ListIdx eLstIdx )
{
  Short sHor = cMv.getHor();
  Short sVer = cMv.getVer();

  RNOK( xWriteMvdComponentQPel( sHor, 0, 0 ) );
  RNOK( xWriteMvdComponentQPel( sVer, 0, 0 ) );

  return Err::m_nOK;
}


ErrVal CabacWriter::xWriteMvdComponentQPel( Short sMvdComp, UInt uiAbsSum, UInt uiCtx )
{
  UInt  uiSymbol  = ( sMvdComp == 0 ? 0 : 1 );
  RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cBLQRefCCModel.get( 0, 3 ) ) );
  ROTRS( uiSymbol == 0 , Err::m_nOK );

  uiSymbol = ( sMvdComp < 0 ? 0 : 1 );
  RNOK( CabaEncoder::writeEPSymbol( uiSymbol ) );

  return Err::m_nOK;
}





ErrVal CabacWriter::xWriteMvd( MbDataAccess& rcMbDataAccess, Mv cMv, LumaIdx cIdx, ListIdx eLstIdx )
{
  Mv    cMvA;
  Mv    cMvB;

  rcMbDataAccess.getMvdAbove( cMvA, eLstIdx, cIdx );
  rcMbDataAccess.getMvdLeft ( cMvB, eLstIdx, cIdx );

  Short sHor = cMv.getHor();
  Short sVer = cMv.getVer();

  RNOK( xWriteMvdComponent( sHor, cMvA.getAbsHor() + cMvB.getAbsHor(), 0 ) );

  ETRACE_T( "Mvd: x" );
  ETRACE_TY( "ae(v)" );
  ETRACE_V( sHor );
  ETRACE_T( " above " );
  ETRACE_V( cMvA.getHor() );
  ETRACE_T( " left " );
  ETRACE_V( cMvB.getHor() );
  ETRACE_N;

  RNOK( xWriteMvdComponent( sVer, cMvA.getAbsVer() + cMvB.getAbsVer(), 5 ) );

  ETRACE_T( "Mvd: y" );
  ETRACE_TY( "ae(v)" );
  ETRACE_V( sVer );
  ETRACE_T( " above " );
  ETRACE_V( cMvA.getVer() );
  ETRACE_T( " left " );
  ETRACE_V( cMvB.getVer() );
  ETRACE_N;

  return Err::m_nOK;
}




ErrVal CabacWriter::intraPredModeChroma( MbDataAccess& rcMbDataAccess )
{
  UInt uiCtx = rcMbDataAccess.getCtxChromaPredMode();
  UInt uiIntraPredModeChroma = rcMbDataAccess.getMbData().getChromaPredMode();

  if( 0 == uiIntraPredModeChroma )
  {
    CabaEncoder::writeSymbol( 0, m_cChromaPredCCModel.get( 0, uiCtx ) );
  }
  else
  {
    CabaEncoder::writeSymbol( 1, m_cChromaPredCCModel.get( 0, uiCtx ) );

    CabaEncoder::writeUnaryMaxSymbol( uiIntraPredModeChroma - 1,
                                          m_cChromaPredCCModel.get( 0 ) + 3,
                                          0, 2 );

  }

  ETRACE_T( "IntraPredModeChroma" );
  ETRACE_TY( "ae(v)" );
  ETRACE_CODE( uiIntraPredModeChroma );
  ETRACE_N;

  return Err::m_nOK;
}

ErrVal CabacWriter::intraPredModeLuma( MbDataAccess& rcMbDataAccess, LumaIdx cIdx )
{
  Int iIntraPredModeLuma = rcMbDataAccess.encodeIntraPredMode(cIdx);

  RNOK( CabaEncoder::writeSymbol( iIntraPredModeLuma >= 0 ? 0 : 1, m_cIntraPredCCModel.get( 0, 0 ) ) );
  if( iIntraPredModeLuma >= 0 )
  {
    RNOK( CabaEncoder::writeSymbol( (iIntraPredModeLuma & 0x01)     , m_cIntraPredCCModel.get( 0, 1 ) ) );
    RNOK( CabaEncoder::writeSymbol( (iIntraPredModeLuma & 0x02) >> 1, m_cIntraPredCCModel.get( 0, 1 ) ) );
    RNOK( CabaEncoder::writeSymbol( (iIntraPredModeLuma & 0x04) >> 2, m_cIntraPredCCModel.get( 0, 1 ) ) );
  }

  ETRACE_T( "IntraPredModeLuma" );
  ETRACE_TY( "ae(v)" );
  ETRACE_CODE( iIntraPredModeLuma );
  ETRACE_N;

  return Err::m_nOK;
}



ErrVal CabacWriter::cbp( MbDataAccess& rcMbDataAccess )
{
  UInt uiCbp = rcMbDataAccess.getMbData().getMbCbp();
  UInt uiCtx = 0, a, b;

  a = rcMbDataAccess.getLeftLumaCbp ( B4x4Idx( 0 ) );
  b = rcMbDataAccess.getAboveLumaCbp( B4x4Idx( 0 ) ) << 1;

  RNOK( CabaEncoder::writeSymbol( uiCbp & 1, m_cCbpCCModel.get( uiCtx, 3 - (a + b) ) ) );

  a = uiCbp & 1;
  b = rcMbDataAccess.getAboveLumaCbp( B4x4Idx( 2 ) ) << 1;

  RNOK( CabaEncoder::writeSymbol( (uiCbp>>1) & 1, m_cCbpCCModel.get( uiCtx, 3 - (a + b) ) ) );

  a = rcMbDataAccess.getLeftLumaCbp ( B4x4Idx( 8 ) );
  b = (uiCbp  << 1) & 2;

  RNOK( CabaEncoder::writeSymbol( (uiCbp>>2) & 1, m_cCbpCCModel.get( uiCtx, 3 - (a + b) ) ) );

  a = ( uiCbp >> 2 ) & 1;
  b = uiCbp & 2;

  RNOK( CabaEncoder::writeSymbol( (uiCbp>>3) & 1, m_cCbpCCModel.get( uiCtx, 3 - (a + b) ) ) );


  uiCtx = 1;

  UInt  uiLeftChromaCbp   = rcMbDataAccess.getLeftChromaCbp ();
  UInt  uiAboveChromaCbp  = rcMbDataAccess.getAboveChromaCbp();

  a = uiLeftChromaCbp  > 0 ? 1 : 0;
  b = uiAboveChromaCbp > 0 ? 2 : 0;

  UInt uiBit = ( 0 == (uiCbp>>4)) ? 0 : 1;

  RNOK( CabaEncoder::writeSymbol( uiBit, m_cCbpCCModel.get( uiCtx, a + b ) ) );

  if( uiBit )
  {
    a = uiLeftChromaCbp  > 1 ? 1 : 0;
    b = uiAboveChromaCbp > 1 ? 2 : 0;

    uiBit = ( 0 == (uiCbp>>5)) ? 0 : 1;

    RNOK( CabaEncoder::writeSymbol( uiBit, m_cCbpCCModel.get( ++uiCtx, a + b ) ) );
  }

  if( !uiCbp )
  {
    m_uiLastDQpNonZero = 0;
  }

  AOF_DBG( 48 >= uiCbp );

  ETRACE_T( "Cbp" );
  ETRACE_TY( "ae(v)" );
  ETRACE_CODE( uiCbp );
  ETRACE_N;

  return Err::m_nOK;
}



ErrVal CabacWriter::residualBlock( MbDataAccess&  rcMbDataAccess,
                                   LumaIdx        cIdx,
                                   ResidualMode   eResidualMode )
{
  const UChar* pucScan = g_aucFrameScan;

  switch( eResidualMode )
  {
  case LUMA_I16_DC:
    {
      pucScan = g_aucLumaFrameDCScan;
      break;
    }
  case LUMA_SCAN:
  case LUMA_I16_AC:
    {
      break;
    }
  default:
    {
      AOT(1);
      return Err::m_nERR;
    }
  }
  ETRACE_T( "LUMA:" );
  ETRACE_V( cIdx );
  ETRACE_N;

  TCoeff* piCoeff = rcMbDataAccess.getMbTCoeffs().get( cIdx );

  UInt uiNumSig = xGetNumberOfSigCoeff( piCoeff, eResidualMode, pucScan );

  RNOK( xWriteBCbp( rcMbDataAccess, uiNumSig, eResidualMode, cIdx ) );

  if( uiNumSig )
  {
    RNOK( xWriteCoeff( uiNumSig, piCoeff, eResidualMode, pucScan ) );
  }
  return Err::m_nOK;
}




ErrVal CabacWriter::residualBlock( MbDataAccess&  rcMbDataAccess,
                                   ChromaIdx      cIdx,
                                   ResidualMode   eResidualMode )
{
  const UChar* pucScan;

  switch( eResidualMode )
  {
  case CHROMA_DC:
    {
      ETRACE_T( "CHROMA_DC:" );
      pucScan = g_aucIndexChromaDCScan;
      break;
    }
  case CHROMA_AC:
    {
      ETRACE_T( "CHROMA_AC:" );
      pucScan = g_aucFrameScan;
      break;
    }
  default:
    {
      AOT(1);
      return Err::m_nERR;
    }
  }
  ETRACE_V( cIdx );
  ETRACE_N;

  TCoeff* piCoeff = rcMbDataAccess.getMbTCoeffs().get( cIdx );

  UInt uiNumSig = xGetNumberOfSigCoeff( piCoeff, eResidualMode, pucScan );

  RNOK( xWriteBCbp( rcMbDataAccess, uiNumSig, eResidualMode, cIdx ) );

  if( uiNumSig )
  {
    RNOK( xWriteCoeff( uiNumSig, piCoeff, eResidualMode, pucScan ) );
  }

  return Err::m_nOK;
}




ErrVal CabacWriter::xWriteBCbp( MbDataAccess& rcMbDataAccess, UInt uiNumSig, ResidualMode eResidualMode, LumaIdx cIdx )
{
  UInt uiBitPos;

  if( LUMA_SCAN == eResidualMode || LUMA_I16_AC == eResidualMode )
  {
    uiBitPos = cIdx;
  }
  else if( LUMA_I16_DC == eResidualMode )
  {
    uiBitPos = 26;
  }
  else
  {
    // full stop
    AOT(1);
  }

  UInt uiCtx      = rcMbDataAccess.getCtxCodedBlockBit( uiBitPos );
  UInt uiBit      = uiNumSig ? 1 : 0;

  RNOK( CabaEncoder::writeSymbol( uiBit, m_cBCbpCCModel.get( type2ctx1[ eResidualMode ], uiCtx) ) );

  rcMbDataAccess.getMbData().setBCBP( uiBitPos, uiBit);

  return Err::m_nOK;
}


ErrVal CabacWriter::xWriteBCbp( MbDataAccess& rcMbDataAccess, UInt uiNumSig, ResidualMode eResidualMode, ChromaIdx cIdx )
{
  UInt uiBitPos;

  if( CHROMA_AC == eResidualMode )
  {
    uiBitPos = 16 + cIdx;
  }
  else if( CHROMA_DC == eResidualMode )
  {
    uiBitPos = 24 + cIdx.plane();
  }
  else
  {
    AOT(1);
    return Err::m_nERR;
  }

  UInt uiCtx      = rcMbDataAccess.getCtxCodedBlockBit( uiBitPos );
  UInt uiBit      = uiNumSig ? 1 : 0;

  RNOK( CabaEncoder::writeSymbol( uiBit, m_cBCbpCCModel.get( type2ctx1[ eResidualMode ], uiCtx) ) );

  rcMbDataAccess.getMbData().setBCBP( uiBitPos, uiBit);

  return Err::m_nOK;
}


UInt CabacWriter::xGetNumberOfSigCoeff( TCoeff* piCoeff, ResidualMode eResidualMode, const UChar* pucScan )
{
  UInt uiNumSig = 0;
  UInt uiStart  = ( CHROMA_AC == eResidualMode || LUMA_I16_AC == eResidualMode )  ? 1 : 0 ;
  UInt uiStop   = ( CHROMA_DC == eResidualMode                                 ) ? 4 : 16;

  for( UInt ui = uiStart; ui < uiStop; ui++ )
  {
    if( piCoeff[ pucScan[ ui ] ] )
    {
      uiNumSig++;
    }
  }

  return uiNumSig;
}


ErrVal CabacWriter::xWriteCoeff( UInt         uiNumSig,
                                 TCoeff*      piCoeff,
                                 ResidualMode eResidualMode,
                                 const UChar* pucScan )
{
  CabacContextModel2DBuffer&  rcMapCCModel  = m_cMapCCModel;
  CabacContextModel2DBuffer&  rcLastCCModel = m_cLastCCModel;

  UInt uiCodedSig = 0;
  UInt uiStart    = 0;
  UInt uiStop     = 15;

  if( CHROMA_AC == eResidualMode || LUMA_I16_AC == eResidualMode )
  {
    uiStart = 1;
  }
  if( CHROMA_DC == eResidualMode )
  {
    uiStop = 3;
  }
  else if( LUMA_I16_DC == eResidualMode)
  {
    uiStop = 15;
  }


  UInt ui;
  //----- encode significance map -----
  for( ui = uiStart; ui < uiStop; ui++ ) // if last coeff is reached, it has to be significant
  {
    UInt uiSig = piCoeff[ pucScan[ ui ] ] ? 1 : 0;
    RNOK( CabaEncoder::writeSymbol( uiSig, rcMapCCModel.get( type2ctx2 [eResidualMode], ui ) ) );

    if( uiSig )
    {
      UInt uiLast = (++uiCodedSig == uiNumSig ? 1 : 0);

      RNOK( CabaEncoder::writeSymbol( uiLast, rcLastCCModel.get( type2ctx2 [eResidualMode], ui ) ) );

      if( uiLast)
      {
        break;
      }
    }
  }

  int   c1 = 1;
  int   c2 = 0;
  //----- encode significant coefficients -----
  ui++;
  while( (ui--) != uiStart )
  {
    UInt  uiAbs, uiSign;
    Int   iCoeff = piCoeff[ pucScan[ ui ] ];

    if( iCoeff )
    {
      if( iCoeff > 0) { uiAbs = static_cast<UInt>( iCoeff);  uiSign = 0; }
      else            { uiAbs = static_cast<UInt>(-iCoeff);  uiSign = 1; }

      UInt uiCtx    = min (c1, 4);
      UInt uiSymbol = uiAbs > 1 ? 1 : 0;
      RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cOneCCModel.get( type2ctx1 [eResidualMode], uiCtx ) ) );

      if( uiSymbol )
      {
        uiCtx  = min (c2,4);
        uiAbs -= 2;
        c1     = 0;
        c2++;
        RNOK( CabaEncoder::writeExGolombLevel( uiAbs, m_cAbsCCModel.get( type2ctx1 [eResidualMode], uiCtx ) ) );
      }
      else if( c1 )
      {
        c1++;
      }
      RNOK( CabaEncoder::writeEPSymbol( uiSign ) );
    }
  }

  return Err::m_nOK;
}


ErrVal CabacWriter::terminatingBit ( UInt uiIsLast )
{
  RNOK( CabaEncoder::writeTerminatingBit( uiIsLast ) );

  ETRACE_T( "EOS" );
  ETRACE_CODE( uiIsLast );
  ETRACE_N;

  return Err::m_nOK;
}


ErrVal CabacWriter::deltaQp( MbDataAccess& rcMbDataAccess )
{
  Int   iDQp  = rcMbDataAccess.getDeltaQp();
  UInt  uiCtx = m_uiLastDQpNonZero;
  UInt  uiDQp = ( iDQp ? 1 : 0 );

  RNOK( CabaEncoder::writeSymbol( uiDQp, m_cDeltaQpCCModel.get( 0, uiCtx ) ) );

  m_uiLastDQpNonZero = ( 0 != uiDQp ? 1 : 0 );

  if( uiDQp )
  {
    uiDQp = (UInt)( iDQp > 0 ? ( 2 * iDQp - 2 ) : ( -2 * iDQp - 1 ) );
    RNOK( CabaEncoder::writeUnarySymbol( uiDQp, &m_cDeltaQpCCModel.get( 0, 2 ), 1 ) );
  }

  ETRACE_T( "DQp" );
  ETRACE_TY( "ae(v)" );
  ETRACE_CODE ( iDQp );
  ETRACE_N;
  return Err::m_nOK;
}


ErrVal CabacWriter::samplesPCM( MbDataAccess& rcMbDataAccess )
{
  ETRACE_POS;
  ETRACE_T( "  PCM SAMPLES: " );

  RNOK( CabaEncoder::finish() );
  RNOK( m_pcBitWriteBufferIf->write(1) );
  RNOK( m_pcBitWriteBufferIf->writeAlignZero() );

  rcMbDataAccess.getMbData().setBCBPAll( 1 );

  AOF_DBG( rcMbDataAccess.getMbData().isPCM() );

  Pel* pSrc = rcMbDataAccess.getMbTCoeffs().getPelBuffer();

  const UInt uiFactor = 8*8;
  const UInt uiSize   = uiFactor*2*3;
  RNOK( m_pcBitWriteBufferIf->samples( pSrc, uiSize ) );

  ETRACE_N;
  RNOK( CabaEncoder::start() );

  return Err::m_nOK;
}

UInt CabacWriter::getNumberOfWrittenBits()
{
  return CabaEncoder::getWrittenBits();
}




ErrVal CabacWriter::transformSize8x8Flag( MbDataAccess& rcMbDataAccess ) 
{
  UInt uiSymbol = rcMbDataAccess.getMbData().isTransformSize8x8() ? 1 : 0;
  UInt uiCtx = rcMbDataAccess.getCtx8x8Flag();
 
  RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cTransSizeCCModel.get( 0, uiCtx ) ) );

  ETRACE_T( "transformSize8x8Flag:" );
  ETRACE_TY( "ae(v)" );
  ETRACE_CODE( rcMbDataAccess.getMbData().isTransformSize8x8() );
  ETRACE_N;

  return Err::m_nOK;
}



ErrVal CabacWriter::residualBlock8x8( MbDataAccess& rcMbDataAccess,
                                      B8x8Idx       c8x8Idx,
                                      ResidualMode  eResidualMode )
{
  const UChar* pucScan = g_aucFrameScan64;

  switch( eResidualMode )
  {
  case LUMA_SCAN:
    {
      break;
    }
  default:
    {
      AOT(1);
      return Err::m_nERR;
    }
  }
  ETRACE_T( "LUMA_8x8:" );
  ETRACE_V( c8x8Idx.b8x8Index() );
  ETRACE_N;

  TCoeff* piCoeff = rcMbDataAccess.getMbTCoeffs().get8x8( c8x8Idx );

  UInt ui;
  UInt uiNumSig = 0;
  for( UInt uiC = 0; uiC < 64; uiC++ )
  {
    if( piCoeff[ uiC ] )
    {
      uiNumSig++;
    }
  }


  UInt uiBitPos = c8x8Idx.b4x4();
  rcMbDataAccess.getMbData().setBCBP( uiBitPos, 1);
  rcMbDataAccess.getMbData().setBCBP( uiBitPos+1, 1);
  rcMbDataAccess.getMbData().setBCBP( uiBitPos+4, 1);
  rcMbDataAccess.getMbData().setBCBP( uiBitPos+5, 1);
  
  const UInt uiCtxOffset = 2;
  CabacContextModel2DBuffer&  rcMapCCModel  = m_cMapCCModel;
  CabacContextModel2DBuffer&  rcLastCCModel = m_cLastCCModel;

  UInt uiCodedSig = 0;
  const Int* pos2ctx_map = pos2ctx_map8x8;


  //----- encode significance map -----
  for( ui = 0; ui < 63; ui++ ) // if last coeff is reached, it has to be significant
  {
    UInt uiSig = piCoeff[ pucScan[ ui ] ] ? 1 : 0;
      RNOK( CabaEncoder::writeSymbol( uiSig, rcMapCCModel.get( uiCtxOffset, pos2ctx_map[ui] ) ) );

    if( uiSig )
    {
      UInt uiLast = (++uiCodedSig == uiNumSig ? 1 : 0);
        RNOK( CabaEncoder::writeSymbol( uiLast, rcLastCCModel.get( uiCtxOffset, pos2ctx_last8x8[ui] ) ) );

      if( uiLast)
      {
        break;
      }
    }
  }

  int   c1 = 1;
  int   c2 = 0;
  //----- encode significant coefficients -----
  ui++;
  while( (ui--) != 0 )
  {
    UInt  uiAbs, uiSign;
    Int   iCoeff = piCoeff[ pucScan[ ui ] ];

    if( iCoeff )
    {
      if( iCoeff > 0) { uiAbs = static_cast<UInt>( iCoeff);  uiSign = 0; }
      else            { uiAbs = static_cast<UInt>(-iCoeff);  uiSign = 1; }

      UInt uiCtx    = min (c1, 4);
      UInt uiSymbol = uiAbs > 1 ? 1 : 0;
      RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cOneCCModel.get( uiCtxOffset, uiCtx ) ) );

      if( uiSymbol )
      {
        uiCtx  = min (c2,4);
        uiAbs -= 2;
        c1     = 0;
        c2++;
        RNOK( CabaEncoder::writeExGolombLevel( uiAbs, m_cAbsCCModel.get( uiCtxOffset, uiCtx ) ) );
      }
      else if( c1 )
      {
        c1++;
      }
      RNOK( CabaEncoder::writeEPSymbol( uiSign ) );
    }
  }
  return Err::m_nOK;
}
















Bool
CabacWriter::RQencodeBCBP_4x4( MbDataAccess&  rcMbDataAccess,
                               MbDataAccess&  rcMbDataAccessBase,
                               LumaIdx        cIdx )
{
  UInt    uiSymbol  = 0;
  TCoeff* piCoeff   = rcMbDataAccess.getMbTCoeffs().get( cIdx );
  UInt    uiCtx     = rcMbDataAccessBase.getCtxCodedBlockBit( cIdx );
  // heiko.schwarz@hhi.de: take only new significants coefficient into account when determining the coded_block_flag
  TCoeff* piBCoeff  = rcMbDataAccessBase.getMbTCoeffs().get( cIdx );

  for( UInt ui = 0; ui < 16; ui++ )  
  {
    // heiko.schwarz@hhi.de: take only new significants coefficient into account when determining the coded_block_flag
    //if( piCoeff[ g_aucFrameScan[ui] ] )
    if( piCoeff[ g_aucFrameScan[ui] ] && !piBCoeff[ g_aucFrameScan[ui] ] )
    {
      uiSymbol = 1;
      break;
    }
  }

  ANOK( CabaEncoder::writeSymbol( uiSymbol, m_cBCbpCCModel.get( type2ctx1[LUMA_SCAN], uiCtx ) ) );
  ETRACE_T( "BCBP_4x4" );
  ETRACE_V( uiSymbol );
  ETRACE_N;

  rcMbDataAccessBase.getMbData().setBCBP( cIdx, uiSymbol );
  
  return ( uiSymbol == 1 );
}



Bool
CabacWriter::RQencodeBCBP_ChromaDC( MbDataAccess&   rcMbDataAccess,
                                    MbDataAccess&   rcMbDataAccessBase,
                                    ChromaIdx       cIdx )
{
  UInt    uiSymbol  = 0;
  TCoeff* piCoeff   = rcMbDataAccess.getMbTCoeffs().get( cIdx );
  UInt    uiCtx     = rcMbDataAccessBase.getCtxCodedBlockBit( 24 + cIdx.plane() );
  // heiko.schwarz@hhi.de: take only new significants coefficient into account when determining the coded_block_flag
  TCoeff* piBCoeff  = rcMbDataAccessBase.getMbTCoeffs().get( cIdx );

  for( UInt ui = 0; ui < 4; ui++ )  
  {
    // heiko.schwarz@hhi.de: take only new significants coefficient into account when determining the coded_block_flag
    //if( piCoeff[ g_aucIndexChromaDCScan[ui] ] )
    if( piCoeff[ g_aucIndexChromaDCScan[ui] ] && !piBCoeff[ g_aucIndexChromaDCScan[ui] ] )
    {
      uiSymbol = 1;
      break;
    }
  }

  ANOK( CabaEncoder::writeSymbol( uiSymbol, m_cBCbpCCModel.get( type2ctx1[CHROMA_DC], uiCtx ) ) );
  ETRACE_T( "BCBP_ChromaDC" );
  ETRACE_V( uiSymbol );
  ETRACE_N;

  rcMbDataAccessBase.getMbData().setBCBP( 24 + cIdx.plane(), uiSymbol );
  
  return ( uiSymbol == 1 );
}


Bool
CabacWriter::RQencodeBCBP_ChromaAC( MbDataAccess&  rcMbDataAccess,
                                    MbDataAccess&  rcMbDataAccessBase,
                                    ChromaIdx      cIdx )
{
  UInt    uiSymbol  = 0;
  TCoeff* piCoeff   = rcMbDataAccess.getMbTCoeffs().get( cIdx );
  UInt    uiCtx     = rcMbDataAccessBase.getCtxCodedBlockBit( 16 + cIdx );
  // heiko.schwarz@hhi.de: take only new significants coefficient into account when determining the coded_block_flag
  TCoeff* piBCoeff  = rcMbDataAccessBase.getMbTCoeffs().get( cIdx );

  for( UInt ui = 1; ui < 16; ui++ )  
  {
    // heiko.schwarz@hhi.de: take only new significants coefficient into account when determining the coded_block_flag
    //if( piCoeff[ g_aucFrameScan[ui] ] )
    if( piCoeff[ g_aucFrameScan[ui] ] && !piBCoeff[ g_aucFrameScan[ui] ] )
    {
      uiSymbol = 1;
      break;
    }
  }

  ANOK( CabaEncoder::writeSymbol( uiSymbol, m_cBCbpCCModel.get( type2ctx1[CHROMA_AC], uiCtx ) ) );
  ETRACE_T( "BCBP_ChromaAC" );
  ETRACE_V( uiSymbol );
  ETRACE_N;

  rcMbDataAccessBase.getMbData().setBCBP( 16 + cIdx, uiSymbol );
  
  return ( uiSymbol == 1 );
}






Bool
CabacWriter::RQencodeCBP_Chroma( MbDataAccess& rcMbDataAccess,
                                 MbDataAccess& rcMbDataAccessBase )
{
  UInt  uiSymbol          = ( ( rcMbDataAccess.getMbData().getMbCbp() >> 4 ) ? 1 : 0 );
  UInt  uiLeftChromaCbp   = rcMbDataAccessBase.getLeftChromaCbp ();
  UInt  uiAboveChromaCbp  = rcMbDataAccessBase.getAboveChromaCbp();
  UInt  uiCtx             = ( uiLeftChromaCbp > 0 ? 1 : 0 ) + ( uiAboveChromaCbp > 0 ? 2 : 0 );

  ANOK( CabaEncoder::writeSymbol( uiSymbol, m_cCbpCCModel.get( 1, uiCtx ) ) );
  ETRACE_T( "CBP_Chroma" );
  ETRACE_V( uiSymbol );
  ETRACE_N;

  if( uiSymbol )
  {
    rcMbDataAccessBase.getMbData().setMbCbp( rcMbDataAccessBase.getMbData().getMbCbp() | 0x10 );
  }
  return ( uiSymbol == 1 );
}

Bool
CabacWriter::RQencodeCBP_ChromaAC( MbDataAccess& rcMbDataAccess,
                                   MbDataAccess& rcMbDataAccessBase )
{
  UInt  uiSymbol          = ( ( rcMbDataAccess.getMbData().getMbCbp() >> 5 ) ? 1 : 0 );
  UInt  uiLeftChromaCbp   = rcMbDataAccessBase.getLeftChromaCbp ();
  UInt  uiAboveChromaCbp  = rcMbDataAccessBase.getAboveChromaCbp();
  UInt  uiCtx             = ( uiLeftChromaCbp > 1 ? 1 : 0 ) + ( uiAboveChromaCbp > 1 ? 2 : 0 );

  ANOK( CabaEncoder::writeSymbol( uiSymbol, m_cCbpCCModel.get( 2, uiCtx ) ) );
  ETRACE_T( "CBP_ChromaAC" );
  ETRACE_V( uiSymbol );
  ETRACE_N;

  if( uiSymbol )
  {
    rcMbDataAccessBase.getMbData().setMbCbp( ( rcMbDataAccessBase.getMbData().getMbCbp() & 0xF ) | 0x20 );
  }
  return ( uiSymbol == 1 );
}

Bool
CabacWriter::RQencodeCBP_8x8( MbDataAccess& rcMbDataAccess,
                              MbDataAccess& rcMbDataAccessBase,
                              B8x8Idx       c8x8Idx )
{
  UInt  uiSymbol        = ( ( rcMbDataAccess.getMbData().getMbCbp() >> c8x8Idx.b8x8Index() ) & 1 ? 1 : 0 );
  UInt  uiCurrentCbp    = rcMbDataAccessBase.getMbData().getMbCbp();
  UInt  uiLeftLumaCbp   = rcMbDataAccessBase.getLeftLumaCbp ( c8x8Idx );
  UInt  uiAboveLumaCbp  = rcMbDataAccessBase.getAboveLumaCbp( c8x8Idx );
  UInt  uiCtx           = 0;

  switch( c8x8Idx.b8x8Index() )
  {
  case 0:   uiCtx = 3 - uiLeftLumaCbp         - 2*uiAboveLumaCbp;       break;
  case 1:   uiCtx = 3 - ( uiCurrentCbp    &1) - 2*uiAboveLumaCbp;       break;
  case 2:   uiCtx = 3 - uiLeftLumaCbp         - ((uiCurrentCbp<<1)&2);  break;
  case 3:   uiCtx = 3 - ((uiCurrentCbp>>2)&1) - ( uiCurrentCbp    &2);  break;
  default:  AOT(1);
  }

  ANOK( CabaEncoder::writeSymbol( uiSymbol, m_cCbpCCModel.get( 0, uiCtx ) ) );
  ETRACE_T( "CBP_Luma" );
  ETRACE_V( uiSymbol );
  ETRACE_N;

  if( uiSymbol )
  {
    rcMbDataAccessBase.getMbData().setMbCbp( rcMbDataAccessBase.getMbData().getMbCbp() | ( 1 << c8x8Idx.b8x8Index() ) );
  }

  return ( uiSymbol == 1 );
}




ErrVal
CabacWriter::RQencodeDeltaQp( MbDataAccess& rcMbDataAccess,
                              MbDataAccess& rcMbDataAccessBase )
{
  Int   iDQp  = rcMbDataAccess.getDeltaQp();
  UInt  uiDQp = ( iDQp ? 1 : 0 );
  UInt  uiCtx = 0;

  RNOK( CabaEncoder::writeSymbol( uiDQp, m_cDeltaQpCCModel.get( 0, uiCtx ) ) );
  if( uiDQp )
  {
    uiDQp = (UInt)( iDQp > 0 ? ( 2 * iDQp - 2 ) : ( -2 * iDQp - 1 ) );
    RNOK( CabaEncoder::writeUnarySymbol( uiDQp, &m_cDeltaQpCCModel.get( 0, 2 ), 1 ) );
  }
  ETRACE_T( "DQP" );
  ETRACE_V( uiDQp );
  ETRACE_N;

  return Err::m_nOK;
}





ErrVal
CabacWriter::RQencodeNewTCoeff_8x8( MbDataAccess&  rcMbDataAccess,
                                    MbDataAccess&  rcMbDataAccessBase,
                                    B8x8Idx        c8x8Idx )
{
  ETRACE_T( "LUMA_8x8_NEW" );
  ETRACE_V( c8x8Idx.b8x8Index() );
  ETRACE_N;

  rcMbDataAccessBase.getMbData().setBCBP( c8x8Idx.b4x4(),   1 );
  rcMbDataAccessBase.getMbData().setBCBP( c8x8Idx.b4x4()+1, 1 );
  rcMbDataAccessBase.getMbData().setBCBP( c8x8Idx.b4x4()+4, 1 );
  rcMbDataAccessBase.getMbData().setBCBP( c8x8Idx.b4x4()+5, 1 );


  TCoeff*       piCoeff     = rcMbDataAccess    .getMbTCoeffs().get8x8( c8x8Idx );
  TCoeff*       piCoeffBase = rcMbDataAccessBase.getMbTCoeffs().get8x8( c8x8Idx );
  const UChar*  pucScan     = g_aucFrameScan64;
  const UInt    uiCtxOffset = 2;
  UInt          uiCodedSig  = 0;
  UInt          uiNumSig    = 0;
  UInt          ui;

  //----- get number of significant coeffs -----
  for( ui = 0; ui < 64; ui++ )
  {
    if( piCoeff[ ui ] && ! piCoeffBase[ ui ] )
    {
      uiNumSig++;
    }
  }

  for( ui = 0; ui < 64; ui++ )
  {
    if( ! piCoeffBase[ pucScan[ui] ] )
    {
      UInt uiSig = piCoeff[ pucScan[ ui ] ] ? 1 : 0;
      RNOK( CabaEncoder::writeSymbol( uiSig, m_cMapCCModel.get( uiCtxOffset, pos2ctx_map8x8[ui] ) ) );

      if( uiSig )
      {
        UInt  uiAbs     = ( piCoeff[pucScan[ui]] > 0 ? piCoeff[pucScan[ui]] : -piCoeff[pucScan[ui]] );
        UInt  uiSign    = ( piCoeff[pucScan[ui]] > 0 ?                    0 :                     1 );
        UInt  uiCtx     = 1; //( start c1 = 1 )
        UInt  uiSymbol  = ( uiAbs > 1 ? 1 : 0 );

        RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cOneCCModel.get( uiCtxOffset, uiCtx ) ) );

        if( uiSymbol )
        {
          uiCtx  = 0;
          uiAbs -= 2;
          RNOK( CabaEncoder::writeExGolombLevel( uiAbs, m_cAbsCCModel.get( uiCtxOffset, uiCtx ) ) );
        }
        RNOK( CabaEncoder::writeEPSymbol( uiSign ) );

        UInt uiLast = (++uiCodedSig == uiNumSig ? 1 : 0);
        if( ui < 63 )
        {
          RNOK( CabaEncoder::writeSymbol( uiLast, m_cLastCCModel.get( uiCtxOffset, pos2ctx_last8x8[ui] ) ) );
        }
        if( uiLast)
        {
          break;
        }
      }
    }
  }

  return Err::m_nOK;
}



ErrVal
CabacWriter::RQencodeTCoeffRef_8x8( MbDataAccess&  rcMbDataAccess,
                                    MbDataAccess&  rcMbDataAccessBase,
                                    B8x8Idx        c8x8Idx )
{
  ETRACE_T( "LUMA_8x8_REF" );
  ETRACE_V( c8x8Idx.b8x8Index() );
  ETRACE_N;

  TCoeff*       piCoeff     = rcMbDataAccess    .getMbTCoeffs().get8x8( c8x8Idx );
  TCoeff*       piCoeffBase = rcMbDataAccessBase.getMbTCoeffs().get8x8( c8x8Idx );
  const UChar*  pucScan     = g_aucFrameScan64;

  for( UInt ui = 0; ui < 64; ui++ )
  {
    if( piCoeffBase[ pucScan[ui] ] )
    {
      UInt uiSig = ( piCoeff[pucScan[ui]] ? 1 : 0 );
      RNOK( CabaEncoder::writeSymbol( uiSig, m_cRefCCModel.get( 0, 0 ) ) );

      if( uiSig )
      {
        UInt uiSignBL = ( piCoeffBase[pucScan[ui]] < 0 ? 1 : 0 );
        UInt uiSignEL = ( piCoeff    [pucScan[ui]] < 0 ? 1 : 0 );
        UInt uiSymbol = ( uiSignBL ^ uiSignEL );
        RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cRefCCModel.get( 0, 1 ) ) );
      }
    }
  }

  return Err::m_nOK;
}





ErrVal
CabacWriter::RQencodeNewTCoeff_Luma ( MbDataAccess&   rcMbDataAccess,
                                      MbDataAccess&   rcMbDataAccessBase,
                                      ResidualMode    eResidualMode,
                                      LumaIdx         cIdx )
{
  ETRACE_T( "LUMA_4x4_NEW" );
  ETRACE_V( cIdx.b4x4() );
  ETRACE_N;

  TCoeff*       piCoeff     = rcMbDataAccess    .getMbTCoeffs().get( cIdx );
  TCoeff*       piCoeffBase = rcMbDataAccessBase.getMbTCoeffs().get( cIdx );
  const UChar*  pucScan     = ( eResidualMode == LUMA_I16_DC ? g_aucLumaFrameDCScan : g_aucFrameScan );
  UInt          uiStart     = ( eResidualMode == LUMA_I16_AC ? 1 : 0 );
  UInt          uiStop      = 16;
  UInt          uiNumSig    = 0;

  for( UInt ui = uiStart; ui < uiStop; ui++ )  
  {
    if( piCoeff[ pucScan[ui] ] && ! piCoeffBase[pucScan[ui]] )
    {
      uiNumSig++;
    }
  }

  RNOK( xRQencodeNewTCoeffs( piCoeff, piCoeffBase, uiNumSig, uiStart, uiStop, eResidualMode, pucScan ) );
  return Err::m_nOK;
}



ErrVal
CabacWriter::RQencodeTCoeffRef_Luma ( MbDataAccess&   rcMbDataAccess,
                                      MbDataAccess&   rcMbDataAccessBase,
                                      ResidualMode    eResidualMode,
                                      LumaIdx         cIdx )
{
  ETRACE_T( "LUMA_4x4_REF" );
  ETRACE_V( cIdx.b4x4() );
  ETRACE_N;

  TCoeff*       piCoeff     = rcMbDataAccess    .getMbTCoeffs().get( cIdx );
  TCoeff*       piCoeffBase = rcMbDataAccessBase.getMbTCoeffs().get( cIdx );
  const UChar*  pucScan     = ( eResidualMode == LUMA_I16_DC ? g_aucLumaFrameDCScan : g_aucFrameScan );
  UInt          uiStart     = ( eResidualMode == LUMA_I16_AC ? 1 : 0 );
  UInt          uiStop      = 16;

  RNOK( xRQencodeTCoeffsRef( piCoeff, piCoeffBase, uiStart, uiStop, eResidualMode, pucScan ) );
  return Err::m_nOK;
}



ErrVal
CabacWriter::RQencodeNewTCoeff_Chroma ( MbDataAccess&   rcMbDataAccess,
                                        MbDataAccess&   rcMbDataAccessBase,
                                        ResidualMode    eResidualMode,
                                        ChromaIdx       cIdx )
{
  ETRACE_T( "CHROMA_4x4_NEW" );
  ETRACE_V( cIdx );
  ETRACE_N;

  TCoeff*       piCoeff     = rcMbDataAccess    .getMbTCoeffs().get( cIdx );
  TCoeff*       piCoeffBase = rcMbDataAccessBase.getMbTCoeffs().get( cIdx );
  const UChar*  pucScan     = ( eResidualMode == CHROMA_DC ? g_aucIndexChromaDCScan : g_aucFrameScan );
  UInt          uiStart     = ( eResidualMode == CHROMA_AC ? 1 : 0  );
  UInt          uiStop      = ( eResidualMode == CHROMA_DC ? 4 : 16 );
  UInt          uiNumSig    = 0;

  for( UInt ui = uiStart; ui < uiStop; ui++ )  
  {
    if( piCoeff[ pucScan[ui] ] && ! piCoeffBase[pucScan[ui]] )
    {
      uiNumSig++;
    }
  }

  RNOK( xRQencodeNewTCoeffs( piCoeff, piCoeffBase, uiNumSig, uiStart, uiStop, eResidualMode, pucScan ) );
  return Err::m_nOK;
}



ErrVal
CabacWriter::RQencodeTCoeffRef_Chroma ( MbDataAccess&   rcMbDataAccess,
                                        MbDataAccess&   rcMbDataAccessBase,
                                        ResidualMode    eResidualMode,
                                        ChromaIdx       cIdx )
{
  ETRACE_T( "CHROMA_4x4_REF" );
  ETRACE_V( cIdx );
  ETRACE_N;

  TCoeff*       piCoeff     = rcMbDataAccess    .getMbTCoeffs().get( cIdx );
  TCoeff*       piCoeffBase = rcMbDataAccessBase.getMbTCoeffs().get( cIdx );
  const UChar*  pucScan     = ( eResidualMode == CHROMA_DC ? g_aucIndexChromaDCScan : g_aucFrameScan );
  UInt          uiStart     = ( eResidualMode == CHROMA_AC ? 1 : 0  );
  UInt          uiStop      = ( eResidualMode == CHROMA_DC ? 4 : 16 );

  RNOK( xRQencodeTCoeffsRef( piCoeff, piCoeffBase, uiStart, uiStop, eResidualMode, pucScan ) );
  return Err::m_nOK;
}




ErrVal
CabacWriter::xRQencodeNewTCoeffs( TCoeff*      piCoeff,
                                  TCoeff*      piCoeffBase,
                                  UInt         uiNumSig,
                                  UInt         uiStart,
                                  UInt         uiStop,
                                  ResidualMode eResidualMode,
                                  const UChar* pucScan )
{
  UInt uiCodedSig = 0;

  for( UInt ui = uiStart; ui < uiStop; ui++ ) // if last coeff is reached, it has to be significant
  {
    if( ! piCoeffBase[pucScan[ui]] )
    {
      UInt uiSig = piCoeff[ pucScan[ ui ] ] ? 1 : 0;
      RNOK( CabaEncoder::writeSymbol( uiSig, m_cMapCCModel.get( type2ctx2 [eResidualMode], ui ) ) );

      if( uiSig )
      {
        UInt  uiAbs     = ( piCoeff[pucScan[ui]] > 0 ? piCoeff[pucScan[ui]] : -piCoeff[pucScan[ui]] );
        UInt  uiSign    = ( piCoeff[pucScan[ui]] > 0 ?                    0 :                     1 );
        UInt  uiCtx     = 1;
        UInt  uiSymbol  = ( uiAbs > 1 ? 1 : 0 );
      
        RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cOneCCModel.get( type2ctx1 [eResidualMode], uiCtx ) ) );

        if( uiSymbol )
        {
          uiCtx  = 0;
          uiAbs -= 2;
          RNOK( CabaEncoder::writeExGolombLevel( uiAbs, m_cAbsCCModel.get( type2ctx1 [eResidualMode], uiCtx ) ) );
        }
        RNOK( CabaEncoder::writeEPSymbol( uiSign ) );

        
        UInt uiLast = (++uiCodedSig == uiNumSig ? 1 : 0);
        if( ui < uiStop - 1 )
        {
          RNOK( CabaEncoder::writeSymbol( uiLast, m_cLastCCModel.get( type2ctx2 [eResidualMode], ui ) ) );
        }
        if( uiLast)
        {
          break;
        }
      }
    }
  }

  return Err::m_nOK;
}






ErrVal
CabacWriter::xRQencodeTCoeffsRef( TCoeff*      piCoeff,
                                  TCoeff*      piCoeffBase,
                                  UInt         uiStart,
                                  UInt         uiStop,
                                  ResidualMode eResidualMode,
                                  const UChar* pucScan )
{
  for( UInt ui = uiStart; ui < uiStop; ui++ )
  {
    if( piCoeffBase[pucScan[ui]] )
    {
      UInt uiSig = ( piCoeff[pucScan[ui]] ? 1 : 0 );
      RNOK( CabaEncoder::writeSymbol( uiSig, m_cRefCCModel.get( 0, 0 ) ) );

      if( uiSig )
      {
        UInt uiSignBL = ( piCoeffBase[pucScan[ui]] < 0 ? 1 : 0 );
        UInt uiSignEL = ( piCoeff    [pucScan[ui]] < 0 ? 1 : 0 );
        UInt uiSymbol = ( uiSignBL ^ uiSignEL );
        RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cRefCCModel.get( 0, 1 ) ) );
      }
    }
  }

  return Err::m_nOK;
}



ErrVal
CabacWriter::RQencode8x8Flag( MbDataAccess& rcMbDataAccess,
                              MbDataAccess& rcMbDataAccessBase ) 
{
  UInt uiSymbol = rcMbDataAccess.getMbData().isTransformSize8x8() ? 1 : 0;
  UInt uiCtx    = rcMbDataAccessBase.getCtx8x8Flag();
 
  RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cTransSizeCCModel.get( 0, uiCtx ) ) );
  ETRACE_T( "TRAFO_8x8" );
  ETRACE_V( uiSymbol );
  ETRACE_N;

  rcMbDataAccessBase.getMbData().setTransformSize8x8( rcMbDataAccess.getMbData().isTransformSize8x8() );

  return Err::m_nOK;
}


ErrVal
CabacWriter::RQencodeTermBit ( UInt uiBit )
{
  RNOK( CabaEncoder::writeTerminatingBit( uiBit ) );
  ETRACE_T( "EOS" );
  ETRACE_V( uiBit );
  ETRACE_N;

  return Err::m_nOK;
}









ErrVal
CabacWriter::RQencodeNewTCoeff_8x8( MbDataAccess&   rcMbDataAccess,
                                    MbDataAccess&   rcMbDataAccessBase,
                                    B8x8Idx         c8x8Idx,
                                    UInt            uiScanIndex,
                                    UInt&           ruiLast,
                                    UInt            uiSigCtx )
{
  ruiLast                   = 0;
  TCoeff*       piCoeff     = rcMbDataAccess    .getMbTCoeffs().get8x8( c8x8Idx );
  TCoeff*       piCoeffBase = rcMbDataAccessBase.getMbTCoeffs().get8x8( c8x8Idx );
  const UChar*  pucScan     = g_aucFrameScan64;
  const UInt    uiCtxOffset = 2;

  ROT( piCoeffBase[pucScan[uiScanIndex]] );

  ETRACE_T( "LUMA_8x8_NEW" );
  ETRACE_V( c8x8Idx.b8x8Index() );
  ETRACE_V( uiScanIndex );
  ETRACE_N;

  if( uiScanIndex == 0 )
  {
    rcMbDataAccessBase.getMbData().setBCBP( c8x8Idx.b4x4(),   1 );
    rcMbDataAccessBase.getMbData().setBCBP( c8x8Idx.b4x4()+1, 1 );
    rcMbDataAccessBase.getMbData().setBCBP( c8x8Idx.b4x4()+4, 1 );
    rcMbDataAccessBase.getMbData().setBCBP( c8x8Idx.b4x4()+5, 1 );
  }

  //===== SIGNIFICANCE BIT =====
  UInt uiSig = ( piCoeff[pucScan[uiScanIndex]] ? 1 : 0 );
  RNOK( CabaEncoder::writeSymbol( uiSig, m_cMapCCModel.get( uiCtxOffset, pos2ctx_map8x8[uiScanIndex] ) ) );
  //RNOK( CabaEncoder::writeSymbol( uiSig, m_cSigCCModel.get( 0, uiSigCtx ) ) );

  if( uiSig )
  {
    UInt  uiAbs     = ( piCoeff[pucScan[uiScanIndex]] > 0 ? piCoeff[pucScan[uiScanIndex]] : -piCoeff[pucScan[uiScanIndex]] );
    UInt  uiSign    = ( piCoeff[pucScan[uiScanIndex]] > 0 ?                             0 :                              1 );

    //===== SIGN =====
    RNOK( CabaEncoder::writeEPSymbol( uiSign ) );
    
    //===== ABSOLUTE VALUE =====
    UInt  uiCtx     = 1;
    UInt  uiSymbol  = ( uiAbs > 1 ? 1 : 0 );
    RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cOneCCModel.get( uiCtxOffset, uiCtx ) ) );
    if( uiSymbol )
    {
      uiCtx  = 0;
      uiAbs -= 2;
      RNOK( CabaEncoder::writeExGolombLevel( uiAbs, m_cAbsCCModel.get( uiCtxOffset, uiCtx ) ) );
    }

    //===== LAST SYMBOL =====
    ruiLast = 1;
    if( uiScanIndex < 63 )
    {
      for( UInt ui = uiScanIndex + 1; ui < 64; ui++ )
      {
        if( piCoeff[pucScan[ui]] && ! piCoeffBase[pucScan[ui]] )
        {
          ruiLast = 0;
          break;
        }
      }
      RNOK( CabaEncoder::writeSymbol( ruiLast, m_cLastCCModel.get( uiCtxOffset, pos2ctx_last8x8[uiScanIndex] ) ) );
    }
  }

  return Err::m_nOK;
}



ErrVal
CabacWriter::RQencodeTCoeffRef_8x8( MbDataAccess&   rcMbDataAccess,
                                    MbDataAccess&   rcMbDataAccessBase,
                                    B8x8Idx         c8x8Idx,
                                    UInt            uiScanIndex )
{
  TCoeff*       piCoeff     = rcMbDataAccess    .getMbTCoeffs().get8x8( c8x8Idx );
  TCoeff*       piCoeffBase = rcMbDataAccessBase.getMbTCoeffs().get8x8( c8x8Idx );
  const UChar*  pucScan     = g_aucFrameScan64;

  ROF( piCoeffBase[pucScan[uiScanIndex]] );

  ETRACE_T( "LUMA_8x8_REF" );
  ETRACE_V( c8x8Idx.b8x8Index() );
  ETRACE_V( uiScanIndex );
  ETRACE_N;


  UInt uiSig = ( piCoeff[pucScan[uiScanIndex]] ? 1 : 0 );
  RNOK( CabaEncoder::writeSymbol( uiSig, m_cRefCCModel.get( 0, 0 ) ) );

  if( uiSig )
  {
    UInt uiSignBL = ( piCoeffBase[pucScan[uiScanIndex]] < 0 ? 1 : 0 );
    UInt uiSignEL = ( piCoeff    [pucScan[uiScanIndex]] < 0 ? 1 : 0 );
    UInt uiSymbol = ( uiSignBL ^ uiSignEL );
    RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cRefCCModel.get( 0, 1 ) ) );
  }

  return Err::m_nOK;
}






ErrVal
CabacWriter::RQencodeNewTCoeff_Luma ( MbDataAccess&   rcMbDataAccess,
                                      MbDataAccess&   rcMbDataAccessBase,
                                      ResidualMode    eResidualMode,
                                      LumaIdx         cIdx,
                                      UInt            uiScanIndex,
                                      UInt&           ruiLast,
                                      UInt            uiSigCtx )
{
  // == Nokia, m11509
  //ruiLast                   = 0;
  // ==
  TCoeff*       piCoeff     = rcMbDataAccess    .getMbTCoeffs().get( cIdx );
  TCoeff*       piCoeffBase = rcMbDataAccessBase.getMbTCoeffs().get( cIdx );
  const UChar*  pucScan     = g_aucFrameScan;
  UInt          uiStart     = 0;
  UInt          uiStop      = 16;

  ROT( piCoeffBase[pucScan[uiScanIndex]] );

  ETRACE_T( "LUMA_4x4_NEW" );
  ETRACE_V( cIdx.b4x4() );
  ETRACE_V( uiScanIndex );
  ETRACE_N;

  RNOK( xRQencodeNewTCoeffs( piCoeff, piCoeffBase, uiStart, uiStop, eResidualMode, pucScan, uiScanIndex, ruiLast, uiSigCtx ) );

  return Err::m_nOK;
}



ErrVal
CabacWriter::RQencodeTCoeffRef_Luma ( MbDataAccess&   rcMbDataAccess,
                                      MbDataAccess&   rcMbDataAccessBase,
                                      ResidualMode    eResidualMode,
                                      LumaIdx         cIdx,
                                      UInt            uiScanIndex )
{
  TCoeff*       piCoeff     = rcMbDataAccess    .getMbTCoeffs().get( cIdx );
  TCoeff*       piCoeffBase = rcMbDataAccessBase.getMbTCoeffs().get( cIdx );
  const UChar*  pucScan     = g_aucFrameScan;
  UInt          uiStart     = 0;
  UInt          uiStop      = 16;

  ROF( piCoeffBase[pucScan[uiScanIndex]] );

  ETRACE_T( "LUMA_4x4_REF" );
  ETRACE_V( cIdx.b4x4() );
  ETRACE_V( uiScanIndex );
  ETRACE_N;

  RNOK( xRQencodeTCoeffsRef( piCoeff, piCoeffBase, pucScan, uiScanIndex ) );
  return Err::m_nOK;
}



ErrVal
CabacWriter::RQencodeNewTCoeff_Chroma ( MbDataAccess&   rcMbDataAccess,
                                        MbDataAccess&   rcMbDataAccessBase,
                                        ResidualMode    eResidualMode,
                                        ChromaIdx       cIdx,
                                        UInt            uiScanIndex,
                                        UInt&           ruiLast,
                                        UInt            uiSigCtx )
{
  // == Nokia, m11509
  //ruiLast                   = 0;
  // ==
  TCoeff*       piCoeff     = rcMbDataAccess    .getMbTCoeffs().get( cIdx );
  TCoeff*       piCoeffBase = rcMbDataAccessBase.getMbTCoeffs().get( cIdx );
  const UChar*  pucScan     = ( eResidualMode == CHROMA_DC ? g_aucIndexChromaDCScan : g_aucFrameScan );
  UInt          uiStart     = ( eResidualMode == CHROMA_AC ? 1 : 0  );
  UInt          uiStop      = ( eResidualMode == CHROMA_DC ? 4 : 16 );

  ROT( piCoeffBase[pucScan[uiScanIndex]] );

  ETRACE_T( "CHROMA_4x4_NEW" );
  ETRACE_V( cIdx );
  ETRACE_V( uiScanIndex );
  ETRACE_N;

  RNOK( xRQencodeNewTCoeffs( piCoeff, piCoeffBase, uiStart, uiStop, eResidualMode, pucScan, uiScanIndex, ruiLast, uiSigCtx ) );

  return Err::m_nOK;
}



ErrVal
CabacWriter::RQencodeTCoeffRef_Chroma ( MbDataAccess&   rcMbDataAccess,
                                        MbDataAccess&   rcMbDataAccessBase,
                                        ResidualMode    eResidualMode,
                                        ChromaIdx       cIdx,
                                        UInt            uiScanIndex )
{
  TCoeff*       piCoeff     = rcMbDataAccess    .getMbTCoeffs().get( cIdx );
  TCoeff*       piCoeffBase = rcMbDataAccessBase.getMbTCoeffs().get( cIdx );
  const UChar*  pucScan     = ( eResidualMode == CHROMA_DC ? g_aucIndexChromaDCScan : g_aucFrameScan );
  UInt          uiStart     = ( eResidualMode == CHROMA_AC ? 1 : 0  );
  UInt          uiStop      = ( eResidualMode == CHROMA_DC ? 4 : 16 );

  ROF( piCoeffBase[pucScan[uiScanIndex]] );

  ETRACE_T( "CHROMA_4x4_REF" );
  ETRACE_V( cIdx );
  ETRACE_V( uiScanIndex );
  ETRACE_N;

  RNOK( xRQencodeTCoeffsRef( piCoeff, piCoeffBase, pucScan, uiScanIndex ) );
  return Err::m_nOK;
}




ErrVal
CabacWriter::xRQencodeNewTCoeffs( TCoeff*       piCoeff,
                                  TCoeff*       piCoeffBase,
                                  UInt          uiStart,
                                  UInt          uiStop,
                                  ResidualMode  eResidualMode,
                                  const UChar*  pucScan,
                                  UInt          uiScanIndex,
                                  UInt&         ruiLast,
                                  UInt          uiSigCtx )
{
  // == Nokia, m11509 (move EOB check to start)
  if( ruiLast )
  {
    ruiLast = 1;
    for( UInt ui = uiScanIndex; ui < uiStop; ui++ )
    {
      if( piCoeff[pucScan[ui]] && ! piCoeffBase[pucScan[ui]] )
      {
        ruiLast = 0;
        break;
      }
    }
    RNOK( CabaEncoder::writeSymbol( ruiLast, m_cLastCCModel.get( type2ctx2 [eResidualMode], uiSigCtx ) ) );
    ROTRS(ruiLast, Err::m_nOK);
  } else
    ruiLast = 0;
  // ==

  //===== SIGNIFICANCE BIT ======
  UInt uiSig = piCoeff[pucScan[uiScanIndex] ] ? 1 : 0;
  RNOK( CabaEncoder::writeSymbol( uiSig, m_cMapCCModel.get( type2ctx2[eResidualMode], uiScanIndex ) ) );

  if( uiSig )
  {
    UInt  uiAbs     = ( piCoeff[pucScan[uiScanIndex]] > 0 ? piCoeff[pucScan[uiScanIndex]] : -piCoeff[pucScan[uiScanIndex]] );
    UInt  uiSign    = ( piCoeff[pucScan[uiScanIndex]] > 0 ?                             0 :                              1 );

    //===== SIGN =====
    RNOK( CabaEncoder::writeEPSymbol( uiSign ) );
    
    //===== ABSOLUTE VALUE =====
    UInt  uiCtx     = 1;
    UInt  uiSymbol  = ( uiAbs > 1 ? 1 : 0 );
    RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cOneCCModel.get( type2ctx1[eResidualMode], uiCtx ) ) );
    if( uiSymbol )
    {
      uiCtx  = 0;
      uiAbs -= 2;
      RNOK( CabaEncoder::writeExGolombLevel( uiAbs, m_cAbsCCModel.get( type2ctx1 [eResidualMode], uiCtx ) ) );
    }

  }

  return Err::m_nOK;
}






ErrVal
CabacWriter::xRQencodeTCoeffsRef( TCoeff*       piCoeff,
                                  TCoeff*       piCoeffBase,
                                  const UChar*  pucScan,
                                  UInt          uiScanIndex )
{
  UInt uiSig = ( piCoeff[pucScan[uiScanIndex]] ? 1 : 0 );
  RNOK( CabaEncoder::writeSymbol( uiSig, m_cRefCCModel.get( 0, 0 ) ) );

  if( uiSig )
  {
    UInt uiSignBL = ( piCoeffBase[pucScan[uiScanIndex]] < 0 ? 1 : 0 );
    UInt uiSignEL = ( piCoeff    [pucScan[uiScanIndex]] < 0 ? 1 : 0 );
    UInt uiSymbol = ( uiSignBL ^ uiSignEL );
    RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cRefCCModel.get( 0, 1 ) ) );
  }

  return Err::m_nOK;
}


// == Nokia, m11509
ErrVal
CabacWriter::RQencodeCycleSymbol( UInt uiCycle )
{
  RNOK( CabaEncoder::writeEPSymbol( uiCycle == 0 ) );
  if ( uiCycle > 0 )
    RNOK( CabaEncoder::writeEPSymbol( uiCycle == 1 ) );
  // heiko.schwarz@hhi.fhg.de: return added
  return Err::m_nOK;
}
// ==


H264AVC_NAMESPACE_END
