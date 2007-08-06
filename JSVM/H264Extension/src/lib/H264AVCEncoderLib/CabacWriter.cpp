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
	m_cFieldFlagCCModel   ( 1,                3),
  m_cFldMapCCModel      ( NUM_BLOCK_TYPES,  NUM_MAP_CTX),
  m_cFldLastCCModel     ( NUM_BLOCK_TYPES,  NUM_LAST_CTX),
  m_cBLSkipCCModel(1,4),
  m_cBCbpCCModel( NUM_BLOCK_TYPES, NUM_BCBP_CTX ),
  m_cMapCCModel( NUM_BLOCK_TYPES, NUM_MAP_CTX ),
  m_cLastCCModel( NUM_BLOCK_TYPES, NUM_LAST_CTX ),
  m_cOneCCModel( NUM_BLOCK_TYPES, NUM_ABS_CTX ),
  m_cAbsCCModel( NUM_BLOCK_TYPES, NUM_ABS_CTX ),
  m_cChromaPredCCModel( 1, 4 ),
  m_cMbTypeCCModel( 3, NUM_MB_TYPE_CTX ),
  m_cBlockTypeCCModel( 2, NUM_B8_TYPE_CTX ),
  m_cMvdCCModel( 2, NUM_MV_RES_CTX ),
  m_cRefPicCCModel( 2, NUM_REF_NO_CTX ),
  m_cBLPredFlagCCModel( 2, NUM_BL_PRED_FLAG_CTX ),
  m_cResPredFlagCCModel( 1, NUM_RES_PRED_FLAG_CTX ),
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
, m_pcNextCabacWriter( NULL )
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
    RNOK( m_cBLSkipCCModel.initBuffer(      (Short*)INIT_BL_SKIP,         iQp ) );

    RNOK( m_cCbpCCModel.initBuffer(         (Short*)INIT_CBP_I,           iQp ) );
    RNOK( m_cBCbpCCModel.initBuffer(        (Short*)INIT_BCBP_I,          iQp ) );
    RNOK( m_cMapCCModel.initBuffer(         (Short*)INIT_MAP_I,           iQp ) );
    RNOK( m_cLastCCModel.initBuffer(        (Short*)INIT_LAST_I,          iQp ) );
    RNOK( m_cOneCCModel.initBuffer(         (Short*)INIT_ONE_I,           iQp ) );
    RNOK( m_cAbsCCModel.initBuffer(         (Short*)INIT_ABS_I,           iQp ) );

    RNOK( m_cCbpEnhanceCCModel.initBuffer(  (Short*)INIT_CBP_ENH_I,       iQp ) );
    RNOK( m_cBCbpEnhanceCCModel.initBuffer( (Short*)INIT_BCBP_ENH_I,      iQp ) );

    RNOK( m_cTransSizeCCModel.initBuffer(   (Short*)INIT_TRANSFORM_SIZE_I,iQp ) );
    RNOK( m_cFieldFlagCCModel.initBuffer(   (Short*)INIT_MB_AFF_I,        iQp ) );
    RNOK( m_cFldMapCCModel.initBuffer(      (Short*)INIT_FLD_MAP_I,       iQp ) );
    RNOK( m_cFldLastCCModel.initBuffer(     (Short*)INIT_FLD_LAST_I,      iQp ) );
  }
  else
  {
    RNOK( m_cMbTypeCCModel.initBuffer(      (Short*)INIT_MB_TYPE_P        [iIndex], iQp ) );
    RNOK( m_cBlockTypeCCModel.initBuffer(   (Short*)INIT_B8_TYPE_P        [iIndex], iQp ) );
    RNOK( m_cMvdCCModel.initBuffer(         (Short*)INIT_MV_RES_P         [iIndex], iQp ) );
    RNOK( m_cRefPicCCModel.initBuffer(      (Short*)INIT_REF_NO_P         [iIndex], iQp ) );
    RNOK( m_cBLPredFlagCCModel.initBuffer(  (Short*)INIT_BL_PRED_FLAG_P   [iIndex], iQp ) );
    RNOK( m_cResPredFlagCCModel.initBuffer( (Short*)INIT_RES_PRED_FLAG_P  [iIndex], iQp ) );
    RNOK( m_cDeltaQpCCModel.initBuffer(     (Short*)INIT_DELTA_QP_P       [iIndex], iQp ) );
    RNOK( m_cIntraPredCCModel.initBuffer(   (Short*)INIT_IPR_P            [iIndex], iQp ) );
    RNOK( m_cChromaPredCCModel.initBuffer(  (Short*)INIT_CIPR_P           [iIndex], iQp ) );
    RNOK( m_cBLSkipCCModel.initBuffer(      (Short*)INIT_BL_SKIP,                   iQp ) );

    RNOK( m_cCbpCCModel.initBuffer(         (Short*)INIT_CBP_P            [iIndex], iQp ) );
    RNOK( m_cBCbpCCModel.initBuffer(        (Short*)INIT_BCBP_P           [iIndex], iQp ) );
    RNOK( m_cMapCCModel.initBuffer(         (Short*)INIT_MAP_P            [iIndex], iQp ) );
    RNOK( m_cLastCCModel.initBuffer(        (Short*)INIT_LAST_P           [iIndex], iQp ) );
    RNOK( m_cOneCCModel.initBuffer(         (Short*)INIT_ONE_P            [iIndex], iQp ) );
    RNOK( m_cAbsCCModel.initBuffer(         (Short*)INIT_ABS_P            [iIndex], iQp ) );

    RNOK( m_cCbpEnhanceCCModel.initBuffer(  (Short*)INIT_CBP_ENH_P        [iIndex], iQp ) );
    RNOK( m_cBCbpEnhanceCCModel.initBuffer( (Short*)INIT_BCBP_ENH_P       [iIndex], iQp ) );

    RNOK( m_cTransSizeCCModel.initBuffer(   (Short*)INIT_TRANSFORM_SIZE_P [iIndex], iQp ) );
    RNOK( m_cFieldFlagCCModel.initBuffer(   (Short*)INIT_MB_AFF_P         [iIndex], iQp ) );
    RNOK( m_cFldMapCCModel.initBuffer(      (Short*)INIT_FLD_MAP_P[ iIndex ],       iQp ) );
    RNOK( m_cFldLastCCModel.initBuffer(     (Short*)INIT_FLD_LAST_P[ iIndex ],      iQp ) );
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
  if( m_pcNextCabacWriter )
  {
    m_pcNextCabacWriter->destroy();
  }
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
  if( m_pcNextCabacWriter )
  {
    m_pcNextCabacWriter->uninit();
  }
  RNOK( CabaEncoder::uninit() );
  return Err::m_nOK;
}


MbSymbolWriteIf* CabacWriter::getSymbolWriteIfNextSlice()
{
  if( !m_pcNextCabacWriter )
  {
    CabacWriter::create( m_pcNextCabacWriter );
    m_pcNextCabacWriter->init( m_pcBitWriteBufferIf->getNextBitWriteBuffer( false ) );
  }
  return m_pcNextCabacWriter;
}


ErrVal CabacWriter::startSlice( const SliceHeader& rcSliceHeader )
{
  m_pcSliceHeader     = &rcSliceHeader;
  m_uiLastDQpNonZero  = 0;

  RNOK( xInitContextModels( rcSliceHeader ) );

  RNOK( CabaEncoder::start() );

  return Err::m_nOK;
}

//FIX_FRAG_CAVLC
ErrVal CabacWriter::getLastByte(UChar &uiLastByte, UInt &uiLastBitPos)
{
  RNOK(CabaEncoder::getLastByte(uiLastByte, uiLastBitPos ));
  return Err::m_nOK;
}
ErrVal CabacWriter::setFirstBits(UChar ucByte, UInt uiLastBitPos)
{
  RNOK( CabaEncoder::setFirstBits(ucByte, uiLastBitPos));
  return Err::m_nOK;
}
//~FIX_FRAG_CAVLC
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
  return xMotionPredFlag( rcMbDataAccess.getMbMotionData( eLstIdx ).getMotPredFlag(), eLstIdx );
}
ErrVal CabacWriter::motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx )
{
  return xMotionPredFlag( rcMbDataAccess.getMbMotionData( eLstIdx ).getMotPredFlag(eParIdx), eLstIdx );
}
ErrVal CabacWriter::motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx )
{
  return xMotionPredFlag( rcMbDataAccess.getMbMotionData( eLstIdx ).getMotPredFlag(eParIdx), eLstIdx );
}
ErrVal CabacWriter::motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8 eParIdx )
{
  return xMotionPredFlag( rcMbDataAccess.getMbMotionData( eLstIdx ).getMotPredFlag(eParIdx), eLstIdx );
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
  ETRACE_CODE( uiRefFrame -1 );
  ETRACE_N;

  return Err::m_nOK;
}


ErrVal CabacWriter::xMotionPredFlag( Bool bFlag, ListIdx eLstIdx )
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



ErrVal CabacWriter::fieldFlag( MbDataAccess& rcMbDataAccess )
{
  UInt uiSymbol = rcMbDataAccess.getMbData().getFieldFlag() ? 1 : 0;

  RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cFieldFlagCCModel.get( 0, rcMbDataAccess.getCtxFieldFlag() ) ) );

  ETRACE_T( "FieldFlag:" );
  ETRACE_TY( "ae(v)" );
  ETRACE_CODE( rcMbDataAccess.getMbData().getFieldFlag() );
  ETRACE_N;

  return Err::m_nOK;
}
ErrVal CabacWriter::skipFlag( MbDataAccess& rcMbDataAccess, Bool bNotAllowed )
{
  ROTRS( m_pcSliceHeader->isIntra(), Err::m_nOK );

  UInt uiSymbol = bNotAllowed ? 0 : rcMbDataAccess.isSkippedMb() ? 1 : 0;

  if( m_pcSliceHeader->isInterB() )
  {
    RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cMbTypeCCModel.get( 2, 7 + rcMbDataAccess.getCtxMbSkipped() ) ) );
  }
  else
  {
    RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cMbTypeCCModel.get( 1, rcMbDataAccess.getCtxMbSkipped() ) ) );
  }
  rcMbDataAccess.getMbData().setSkipFlag(uiSymbol!=0);
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


ErrVal CabacWriter::resPredFlag( MbDataAccess& rcMbDataAccess )
{
  UInt  uiSymbol = ( rcMbDataAccess.getMbData().getResidualPredFlag( PART_16x16 ) ? 1 : 0 );
  UInt  uiCtx    = ( rcMbDataAccess.getMbData().getBLSkipFlag() ? 0 : 1 );

  RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cResPredFlagCCModel.get( 0, uiCtx ) ) );

  ETRACE_T( "ResidualPredFlag " );
  ETRACE_TY( "ae(v)" );
  ETRACE_CODE( uiSymbol );
  ETRACE_N;

  return Err::m_nOK;
}


ErrVal CabacWriter::resPredFlag_FGS( MbDataAccess& rcMbDataAccess, Bool bBaseCoeff )
{
  UInt  uiSymbol = ( rcMbDataAccess.getMbData().getResidualPredFlag( PART_16x16 ) ? 1 : 0 );
  UInt  uiCtx    = ( bBaseCoeff ? 2 : 3 );

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
  ETRACE_DECLARE( UInt uiOrigMbMode = uiMbMode );

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


ErrVal CabacWriter::cbp( MbDataAccess& rcMbDataAccess, UInt uiStart, UInt uiStop )
{
  Bool bInterlaced = FRAME != rcMbDataAccess.getMbPicType();
  ROF( bInterlaced == rcMbDataAccess.getMbData().getFieldFlag() );
  UInt uiCbp = rcMbDataAccess.getMbData().calcMbCbp( uiStart, uiStop );
  UInt uiCtx = 0, a, b;

  a = rcMbDataAccess.getLeftLumaCbp ( B4x4Idx( 0 ), uiStart, uiStop );
  b = rcMbDataAccess.getAboveLumaCbp( B4x4Idx( 0 ), uiStart, uiStop ) << 1;

  RNOK( CabaEncoder::writeSymbol( uiCbp & 1, m_cCbpCCModel.get( uiCtx, 3 - (a + b) ) ) );

  a = uiCbp & 1;
  b = rcMbDataAccess.getAboveLumaCbp( B4x4Idx( 2 ), uiStart, uiStop ) << 1;

  RNOK( CabaEncoder::writeSymbol( (uiCbp>>1) & 1, m_cCbpCCModel.get( uiCtx, 3 - (a + b) ) ) );

  a = rcMbDataAccess.getLeftLumaCbp ( B4x4Idx( 8 ), uiStart, uiStop );
  b = (uiCbp  << 1) & 2;

  RNOK( CabaEncoder::writeSymbol( (uiCbp>>2) & 1, m_cCbpCCModel.get( uiCtx, 3 - (a + b) ) ) );

  a = ( uiCbp >> 2 ) & 1;
  b = uiCbp & 2;

  RNOK( CabaEncoder::writeSymbol( (uiCbp>>3) & 1, m_cCbpCCModel.get( uiCtx, 3 - (a + b) ) ) );


  uiCtx = 1;

  UInt  uiLeftChromaCbp   = rcMbDataAccess.getLeftChromaCbp ( uiStart, uiStop );
  UInt  uiAboveChromaCbp  = rcMbDataAccess.getAboveChromaCbp( uiStart, uiStop );

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
                                   ResidualMode   eResidualMode,
                                   UInt           uiStart,
                                   UInt           uiStop )
{
  const Bool bFrame    = ( FRAME == rcMbDataAccess.getMbPicType());
  const UChar* pucScan = ( eResidualMode==LUMA_I16_DC ? ((bFrame) ? g_aucLumaFrameDCScan : g_aucLumaFieldDCScan) : ((bFrame) ? g_aucFrameScan : g_aucFieldScan) );

  ETRACE_T( "LUMA:" );
  ETRACE_V( cIdx );
  ETRACE_N;

  TCoeff* piCoeff = rcMbDataAccess.getMbTCoeffs().get( cIdx );

  ROT( uiStart == uiStop );
  UInt uiNumSig = xGetNumberOfSigCoeff( piCoeff, eResidualMode, pucScan, uiStart, uiStop );
  RNOK( xWriteBCbp( rcMbDataAccess, uiNumSig, eResidualMode, cIdx, uiStart, uiStop ) );

  if( uiNumSig )
  {
    RNOK( xWriteCoeff( uiNumSig, piCoeff, eResidualMode, pucScan, bFrame, uiStart, uiStop ) );
  }
  return Err::m_nOK;
}



ErrVal CabacWriter::residualBlock( MbDataAccess&  rcMbDataAccess,
                                   ChromaIdx      cIdx,
                                   ResidualMode   eResidualMode,
                                   UInt           uiStart,
                                   UInt           uiStop )
{
  const Bool bFrame    = ( FRAME == rcMbDataAccess.getMbPicType());
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
      pucScan = (bFrame ? g_aucFrameScan : g_aucFieldScan);
      break;
    }
  default:
    {
      AF();
      return Err::m_nERR;
    }
  }
  ETRACE_V( cIdx );
  ETRACE_N;

  TCoeff* piCoeff = rcMbDataAccess.getMbTCoeffs().get( cIdx );

  ROT( uiStart == uiStop );
  UInt uiNumSig = xGetNumberOfSigCoeff( piCoeff, eResidualMode, pucScan, uiStart, uiStop );
  RNOK( xWriteBCbp( rcMbDataAccess, uiNumSig, eResidualMode, cIdx, uiStart, uiStop ) );

  if( uiNumSig )
  {
    RNOK( xWriteCoeff( uiNumSig, piCoeff, eResidualMode, pucScan, bFrame, uiStart, uiStop ) );
  }

  return Err::m_nOK;
}



ErrVal CabacWriter::xWriteBCbp( MbDataAccess& rcMbDataAccess, UInt uiNumSig, ResidualMode eResidualMode, LumaIdx cIdx, UInt uiStart, UInt uiStop )
{
  UInt uiBitPos = 0;

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
    AF();
  }
  UInt uiCtx      = rcMbDataAccess.getCtxCodedBlockBit( uiBitPos, uiStart, uiStop );
  UInt uiBit      = uiNumSig ? 1 : 0;

  RNOK( CabaEncoder::writeSymbol( uiBit, m_cBCbpCCModel.get( type2ctx1[ eResidualMode ], uiCtx) ) );

  rcMbDataAccess.getMbData().setBCBP( uiBitPos, uiBit);

  return Err::m_nOK;
}

ErrVal CabacWriter::xWriteBCbp( MbDataAccess& rcMbDataAccess, UInt uiNumSig, ResidualMode eResidualMode, ChromaIdx cIdx, UInt uiStart, UInt uiStop )
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
    AF();
    return Err::m_nERR;
  }

  UInt uiCtx      = rcMbDataAccess.getCtxCodedBlockBit( uiBitPos, uiStart, uiStop );
  UInt uiBit      = uiNumSig ? 1 : 0;

  RNOK( CabaEncoder::writeSymbol( uiBit, m_cBCbpCCModel.get( type2ctx1[ eResidualMode ], uiCtx) ) );

  rcMbDataAccess.getMbData().setBCBP( uiBitPos, uiBit);

  return Err::m_nOK;
}

UInt CabacWriter::xGetNumberOfSigCoeff( TCoeff* piCoeff, ResidualMode eResidualMode, const UChar* pucScan, UInt uiStart, UInt uiStop )
{
  UInt uiNumSig = 0;
  if( CHROMA_DC == eResidualMode || LUMA_I16_DC == eResidualMode )
  {
    if( uiStart == 0 && uiStop > 0 )
    {
      // process all DC coefficients if the range uiStart to uiStop demands the processing of scanpos 0
      uiStop  = CHROMA_DC == eResidualMode ? 4 : 16;
    }
    else
    {
      return 0;
    }
  }
  else if( CHROMA_AC == eResidualMode || LUMA_I16_AC == eResidualMode )
  {
    if( uiStop > 1 )
    {
      uiStart = max( 1, uiStart );
    }
    else
    {
      return 0;
    }
  }

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
                                 const UChar* pucScan, 
                                 Bool         bFrame,
                                 UInt         uiStart,
                                 UInt         uiStop )
{
  CabacContextModel2DBuffer&  rcMapCCModel  = (bFrame ? m_cMapCCModel : m_cFldMapCCModel );
  CabacContextModel2DBuffer&  rcLastCCModel = (bFrame ? m_cLastCCModel: m_cFldLastCCModel);
  UInt uiCodedSig = 0;

  if( CHROMA_DC == eResidualMode || LUMA_I16_DC == eResidualMode )
  {
    if( uiStart == 0 && uiStop > 0 )
    {
      uiStop = CHROMA_DC == eResidualMode ? 3 : 15;
    }
    else
    {
      ROT( 1 );
    }
  }
  else
  {
    if( CHROMA_AC == eResidualMode || LUMA_I16_AC == eResidualMode )
    {
      if( uiStop > 1 )
      {
        uiStart = max( 1, uiStart );
      }
      else
      {
        ROT( 1 );
      }
    }
    uiStop -= 1;
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


ErrVal CabacWriter::transformSize8x8Flag( MbDataAccess& rcMbDataAccess, UInt uiStart, UInt uiStop ) 
{
  UInt uiSymbol = rcMbDataAccess.getMbData().isTransformSize8x8() ? 1 : 0;
  UInt uiCtx = rcMbDataAccess.getCtx8x8Flag( uiStart, uiStop );
 
  RNOK( CabaEncoder::writeSymbol( uiSymbol, m_cTransSizeCCModel.get( 0, uiCtx ) ) );

  ETRACE_T( "transformSize8x8Flag:" );
  ETRACE_TY( "ae(v)" );
  ETRACE_CODE( rcMbDataAccess.getMbData().isTransformSize8x8() );
  ETRACE_N;

  return Err::m_nOK;
}


ErrVal CabacWriter::residualBlock8x8( MbDataAccess& rcMbDataAccess,
                                      B8x8Idx       c8x8Idx,
                                      ResidualMode  eResidualMode,
                                      UInt          uiStart,
                                      UInt          uiStop )
{
  const Bool bFrame    = ( FRAME == rcMbDataAccess.getMbPicType());
  const UChar* pucScan = (bFrame) ? g_aucFrameScan64 : g_aucFieldScan64;

  switch( eResidualMode )
  {
  case LUMA_SCAN:
    {
      break;
    }
  default:
    {
      AF();
      return Err::m_nERR;
    }
  }
  ETRACE_T( "LUMA_8x8:" );
  ETRACE_V( c8x8Idx.b8x8Index() );
  ETRACE_N;

  TCoeff* piCoeff = rcMbDataAccess.getMbTCoeffs().get8x8( c8x8Idx );

  UInt ui;
  UInt uiNumSig = 0;

  ROT( uiStart == uiStop );

  uiStart <<= 2;
  uiStop  <<= 2;
  for( UInt uiC = uiStart; uiC < uiStop; uiC++ )
  {
    if( piCoeff[pucScan[uiC]] )
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
  CabacContextModel2DBuffer&  rcMapCCModel  = ( bFrame ? m_cMapCCModel  : m_cFldMapCCModel);
  CabacContextModel2DBuffer&  rcLastCCModel = ( bFrame ? m_cLastCCModel : m_cFldLastCCModel);

  UInt uiCodedSig = 0;
  const Int* pos2ctx_map = (bFrame) ? pos2ctx_map8x8 : pos2ctx_map8x8i;

  //----- encode significance map -----
  for( ui = uiStart; ui < uiStop - 1; ui++ ) // if last coeff is reached, it has to be significant
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




H264AVC_NAMESPACE_END
