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





#include "H264AVCCommonLib.h"
#include "H264AVCCommonLib/MbData.h"
#include "H264AVCCommonLib/MbDataCtrl.h"
#include "H264AVCCommonLib/FrameUnit.h"

#include <math.h>

H264AVC_NAMESPACE_BEGIN


#define MEDIAN(a,b,c)  ((a)>(b)?(a)>(c)?(b)>(c)?(b):(c):(a):(b)>(c)?(a)>(c)?(a):(c):(b))


MbDataCtrl::MbDataCtrl():
  m_pcMbTCoeffs     ( NULL ),
  m_pcMbData        ( NULL ),
  m_pcMbDataAccess  ( NULL ),
  m_pcSliceHeader   ( NULL ),
  m_ucLastMbQp      ( 0 ),
  m_uiMbStride      ( 0 ),
  m_uiMbOffset      ( 0 ),
  m_iMbPerLine      ( 0 ),
  m_iMbPerColumn    ( 0 ),
  m_uiSize          ( 0 ),
  m_uiMbProcessed   ( 0 ),
  m_uiSliceId       ( 0 ),
  m_eProcessingState( PRE_PROCESS),
  m_pcMbDataCtrl0L1 ( NULL ),
  m_bUseTopField    ( false ),
  m_bPicCodedField  ( false ),
  m_bInitDone       ( false )
{
  m_apcMbMvdData    [LIST_0]  = NULL;
  m_apcMbMvdData    [LIST_1]  = NULL;
  m_apcMbMotionData [LIST_0]  = NULL;
  m_apcMbMotionData [LIST_1]  = NULL;
}

MbDataCtrl::~MbDataCtrl()
{
  xDeleteData();
  AOT_DBG( m_bInitDone );
}

ErrVal MbDataCtrl::xCreateData( UInt uiSize )
{
  uiSize++;

  ROT( NULL == ( m_pcMbTCoeffs         = new MbTransformCoeffs [ uiSize ] ) );
  ROT( NULL == ( m_apcMbMotionData[0]  = new MbMotionData      [ uiSize ] ) );
  ROT( NULL == ( m_apcMbMotionData[1]  = new MbMotionData      [ uiSize ] ) );
  ROT( NULL == ( m_apcMbMvdData[0]     = new MbMvData          [ uiSize ] ) );
  ROT( NULL == ( m_apcMbMvdData[1]     = new MbMvData          [ uiSize ] ) );
  ROT( NULL == ( m_pcMbData            = new MbData            [ uiSize ] ) );

  for( UInt uiIdx = 0; uiIdx < uiSize; uiIdx++ )
  {
    m_pcMbData[ uiIdx ].init( m_pcMbTCoeffs        + uiIdx,
                              m_apcMbMvdData   [0] + uiIdx,
                              m_apcMbMvdData   [1] + uiIdx,
                              m_apcMbMotionData[0] + uiIdx,
                              m_apcMbMotionData[1] + uiIdx );
  }

  // clear outside mb data
  m_pcMbData[uiSize-1].getMbTCoeffs().setAllCoeffCount( 0 );
  m_pcMbData[uiSize-1].initMbData( 0, MSYS_UINT_MAX );

  return Err::m_nOK;
}

ErrVal MbDataCtrl::xDeleteData()
{
  H264AVC_DELETE_CLASS( m_pcMbDataAccess );

  H264AVC_DELETE( m_pcMbTCoeffs );
  H264AVC_DELETE( m_apcMbMvdData[1] );
  H264AVC_DELETE( m_apcMbMvdData[0] );
  H264AVC_DELETE( m_apcMbMotionData[1] );
  H264AVC_DELETE( m_apcMbMotionData[0] );
  H264AVC_DELETE( m_pcMbData );
  m_uiSize          = 0;
  return Err::m_nOK;
}

ErrVal MbDataCtrl::xResetData()
{
  UInt uiIdx;
  for( uiIdx = 0; uiIdx < m_uiSize; uiIdx++ )
  {
    m_pcMbData[ uiIdx ].reset();
  }
  for( uiIdx = 0; uiIdx < m_uiSize; uiIdx++ )
  {
    m_pcMbTCoeffs[ uiIdx ].clear();
  }
  for( uiIdx = 0; uiIdx < m_uiSize; uiIdx++ )
  {
    m_apcMbMvdData[0][ uiIdx ].clear();
  }
  for( uiIdx = 0; uiIdx < m_uiSize; uiIdx++ )
  {
    m_apcMbMvdData[1][ uiIdx ].clear();
  }
  for( uiIdx = 0; uiIdx < m_uiSize; uiIdx++ )
  {
    m_apcMbMotionData[0][ uiIdx ].reset();
  }
  for( uiIdx = 0; uiIdx < m_uiSize; uiIdx++ )
  {
    m_apcMbMotionData[1][ uiIdx ].reset();
  }
  return Err::m_nOK;
}


Bool MbDataCtrl::isPicDone( const SliceHeader& rcSH )
{
  return ( m_uiMbProcessed == rcSH.getSPS().getMbInFrame() || m_uiMbProcessed == rcSH.getMbInPic());
}

Bool MbDataCtrl::isFrameDone( const SliceHeader& rcSH )
{
  return ( m_uiMbProcessed == rcSH.getSPS().getMbInFrame());
}


ErrVal MbDataCtrl::init( const SequenceParameterSet& rcSPS )
{
  AOT_DBG( m_bInitDone );

  UInt uiSize = rcSPS.getMbInFrame();

  ROT( 0 == uiSize );
  if( m_uiSize == uiSize )
  {
    RNOK( xResetData() );
  }
  else
  {
    RNOK( xDeleteData() );
    RNOK( xCreateData( uiSize ) );
    m_uiSize = uiSize;
  }

  m_iMbPerLine = rcSPS.getFrameWidthInMbs();

  RNOK( m_cpDFPBuffer.init( uiSize+1 ) );
  m_cpDFPBuffer.clear();

  m_bInitDone     = true;

  return Err::m_nOK;
}


ErrVal
MbDataCtrl::copyMotion( MbDataCtrl& rcMbDataCtrl )
{
  for( UInt n = 0; n < m_uiSize; n++ )
  {
    RNOK( m_pcMbData[n].copyMotion( rcMbDataCtrl.m_pcMbData[n], m_uiSliceId ) );
  }
  return Err::m_nOK;
}


ErrVal
MbDataCtrl::copyMotionBL( MbDataCtrl& rcMbDataCtrl )
{
  Bool bDirect8x8 = rcMbDataCtrl.xGetDirect8x8InferenceFlag();

  for( UInt n = 0; n < m_uiSize; n++ )
  {
    RNOK( m_pcMbData[n].copyMotionBL( rcMbDataCtrl.m_pcMbData[n], bDirect8x8, m_uiSliceId ) );
  }
  return Err::m_nOK;
}




ErrVal
MbDataCtrl::upsampleMotion( MbDataCtrl& rcMbDataCtrl )
{
  Bool bDirect8x8 = rcMbDataCtrl.xGetDirect8x8InferenceFlag();

  for( Int iMbY = 0; iMbY < m_iMbPerColumn; iMbY+=2 )
  for( Int iMbX = 0; iMbX < m_iMbPerLine;   iMbX+=2 )
  for( Int iPar = 0; iPar < 4;              iPar++  )
  {
    MbData& rcMbDes = m_pcMbData[(iMbY+(iPar/2))*m_uiMbStride+(iMbX+(iPar%2))];
    MbData& rcMbSrc = rcMbDataCtrl.m_pcMbData[(iMbY/2)*rcMbDataCtrl.m_uiMbStride+(iMbX/2)];
    Par8x8  ePar    = Par8x8( iPar );

    RNOK( rcMbDes.upsampleMotion( rcMbSrc, ePar, bDirect8x8 ) );
  }

  return Err::m_nOK;
}




ErrVal MbDataCtrl::uninit()
{
  m_ucLastMbQp      = 0;
  m_uiMbStride      = 0;
  m_uiMbOffset      = 0;
  m_iMbPerLine      = 0;
  m_iMbPerColumn    = 0;
  m_uiMbProcessed   = 0;
  m_uiSliceId       = 0;
  m_pcMbDataCtrl0L1 = 0;

  for( UInt n = 0; n < m_cpDFPBuffer.size(); n++ )
  {
    delete m_cpDFPBuffer.get( n );
    m_cpDFPBuffer.set( n, NULL );
  }
  RNOK( m_cpDFPBuffer.uninit() );

  m_bInitDone = false;
  return Err::m_nOK;
}


ErrVal MbDataCtrl::reset()
{
  m_ucLastMbQp      = 0;
  m_uiMbProcessed   = 0;
  m_uiSliceId       = 0;
  m_pcMbDataCtrl0L1 = 0;
  return Err::m_nOK;
}

ErrVal MbDataCtrl::initSlice( SliceHeader& rcSH, ProcessingState eProcessingState, Bool bDecoder, MbDataCtrl* pcMbDataCtrl )
{
  AOF_DBG( m_bInitDone );

  m_eProcessingState  = eProcessingState;
  m_pcMbDataCtrl0L1   = NULL;

  if( rcSH.isInterB() )
  {
    //if( rcSH.getNalUnitType() < NAL_UNIT_SCALABLE_LAYER_0 )
    if( rcSH.getNalUnitType() != NAL_UNIT_CODED_SLICE_IDR_SCALABLE &&
        rcSH.getNalUnitType() != NAL_UNIT_CODED_SLICE_SCALABLE     && bDecoder )
    {
      const RefPic& rcRefPic0L1 = rcSH.getRefPic( 1, LIST_1 );
      AOF_DBG( rcRefPic0L1.isAvailable() );
      const FrameUnit* pcFU = rcRefPic0L1.getPic().getFrameUnit();

      Int iCurrPoc      = rcSH.getPoc();
      m_pcMbDataCtrl0L1 = pcFU->getMbDataCtrl();
    }

    if( pcMbDataCtrl )
    {
      m_pcMbDataCtrl0L1 = pcMbDataCtrl;
    }
  }

  if( PARSE_PROCESS == m_eProcessingState || ENCODE_PROCESS == m_eProcessingState )
  {
    m_uiSliceId++;

    m_cpDFPBuffer.set( m_uiSliceId, rcSH.getDeblockingFilterParameter().getCopy() );
    m_bDirect8x8InferenceFlag = rcSH.getSPS().getDirect8x8InferenceFlag();
  }
  m_pcSliceHeader = &rcSH;


  Int iMbPerColumn  = rcSH.getSPS().getFrameHeightInMbs ();
  m_iMbPerLine      = rcSH.getSPS().getFrameWidthInMbs  ();
  m_uiMbOffset      = 0;
  m_uiMbStride      = m_iMbPerLine;
  m_iMbPerColumn    = iMbPerColumn;
  m_ucLastMbQp      = rcSH.getPicQp();

  H264AVC_DELETE_CLASS( m_pcMbDataAccess );
  return Err::m_nOK;
}


const MbData& MbDataCtrl::xGetColMbData( UInt uiIndex )
{
  return (( m_pcMbDataCtrl0L1 == NULL ) ? xGetOutMbData() : m_pcMbDataCtrl0L1->getMbData( uiIndex ));
}

const MbData& MbDataCtrl::xGetRefMbData( UInt uiSliceId, Int iMbY, Int iMbX, Bool bLoopFilter )
{
  // check whether ref mb is inside
  ROTRS( iMbX < 0,               xGetOutMbData() );
  ROTRS( iMbY < 0,               xGetOutMbData() );
  ROTRS( iMbX >= m_iMbPerLine,   xGetOutMbData() );
  ROTRS( iMbY >= m_iMbPerColumn, xGetOutMbData() );

  // get the ref mb data
  const MbData& rcMbData = getMbData( iMbY * m_uiMbStride + iMbX + m_uiMbOffset );
  // test slice id
  return (( rcMbData.getSliceId() == uiSliceId || bLoopFilter ) ? rcMbData : xGetOutMbData() );
}


ErrVal MbDataCtrl::initMb( MbDataAccess*& rpcMbDataAccess, UInt uiMbY, UInt uiMbX, const Int iForceQp )
{
  ROF( m_bInitDone );

  AOT_DBG( uiMbY * m_uiMbStride + uiMbX + m_uiMbOffset >= m_uiSize );

  Bool     bLf          = (m_eProcessingState == POST_PROCESS);
  UInt     uiCurrIdx    = uiMbY * m_uiMbStride + uiMbX + m_uiMbOffset;
  MbData&  rcMbDataCurr = m_pcMbData[ uiCurrIdx ];


  if( m_pcMbDataAccess )
  {
    m_ucLastMbQp = m_pcMbDataAccess->getMbData().getQp();
  }

  UInt uiSliceId = rcMbDataCurr.getSliceId();
  if( PARSE_PROCESS == m_eProcessingState || ENCODE_PROCESS == m_eProcessingState)
  {
    if( 0 == uiSliceId )
    {
      uiSliceId = m_uiSliceId;
      rcMbDataCurr.getMbTCoeffs().clear();
      rcMbDataCurr.initMbData( m_ucLastMbQp, uiSliceId );
      rcMbDataCurr.clear();
      m_uiMbProcessed++;
    }
    else
    {
      //allready assigned;
      if( ENCODE_PROCESS != m_eProcessingState )
      {
        AOT(1);
      }
      else
      {
        if( iForceQp != -1 )
        {
          m_ucLastMbQp = iForceQp;
        }
      }
    }
  }

  Bool bColocatedField = ( m_pcMbDataCtrl0L1 == NULL) ? true : m_pcMbDataCtrl0L1->isPicCodedField();

  m_pcMbDataAccess = new (m_pcMbDataAccess) MbDataAccess(
                                       rcMbDataCurr,                                      // current
                                       xGetRefMbData( uiSliceId, uiMbY,   uiMbX-1, bLf ), // left
                                       xGetRefMbData( uiSliceId, uiMbY-1, uiMbX  , bLf ), // above
                                       xGetRefMbData( uiSliceId, uiMbY-1, uiMbX-1, bLf ), // above left
                                       xGetRefMbData( uiSliceId, uiMbY-1, uiMbX+1, bLf ), // above right
                                       xGetOutMbData(),                                   // unvalid
                                       xGetColMbData( uiCurrIdx ),
                                       *m_pcSliceHeader,
                                       *m_cpDFPBuffer.get( uiSliceId ),
                                       uiMbX,
                                       uiMbY,
                                       m_ucLastMbQp );


  ROT( NULL == m_pcMbDataAccess );

  rpcMbDataAccess = m_pcMbDataAccess;

  return Err::m_nOK;
}




Void MbDataCtrl::xAddConnectionForMV( ConnectionData&  rcConnectionData,
                                      ListIdx          eListIdx,
                                      Int              iRefIdx,
                                      LumaIdx          cIdx,
                                      Int              iNumConnected,
                                      const Mv&        rcMv )
{
  if( iNumConnected == 0 )
  {
    return;
  }

  Int iNumAlreadyConnected = rcConnectionData.getConnected( eListIdx, iRefIdx, cIdx );

  if( iNumAlreadyConnected == 0 )
  {
    //===== there is not motion vector assigned to this block =====
    rcConnectionData.setConnected ( eListIdx, iRefIdx, cIdx, iNumConnected  );
    rcConnectionData.setMv        ( eListIdx, iRefIdx, cIdx, rcMv           );
    return;
  }

  const Mv& rcAssignedMv = rcConnectionData.getMv( eListIdx, iRefIdx, cIdx );


  if( rcAssignedMv.getHor() == rcMv.getHor() &&
      rcAssignedMv.getVer() == rcMv.getVer()    )
  {
    rcConnectionData.addConnected ( eListIdx, iRefIdx, cIdx, iNumConnected );
    return;
  }

  if( iNumConnected >= iNumAlreadyConnected )
  {
    //===== other connection with more valid pixels =====
    rcConnectionData.setConnected ( eListIdx, iRefIdx, cIdx, iNumConnected  );
    rcConnectionData.setMv        ( eListIdx, iRefIdx, cIdx, rcMv           );
    return;
  }
}






ErrVal MbDataCtrl::deriveUpdateMotionFieldAdaptive( SliceHeader&      rcSH,
                                                    CtrlDataList*     pcCtrlDataList,
                                                    ConnectionArray&  rcConnectionArray,
                                                    UShort*           pusUpdateWeights,
                                                    Bool              bDecoder,
                                                    ListIdx           eListUpd )
{
  ListIdx eListPrd = ListIdx( 1-eListUpd );
  Int     iWStride = 4*m_iMbPerLine;
  Int     iMbYPrd, iMbXPrd, iMbYUpd, iMbXUpd, iBlkYUpd, iBlkXUpd, iRefIdx;


  //===== initialize arrays and set quantization parameters =====
  {
    RNOK( initSlice( rcSH, bDecoder ? PARSE_PROCESS : ENCODE_PROCESS, false, NULL ) );

    for( iMbYUpd = 0; iMbYUpd < m_iMbPerColumn; iMbYUpd++ )
    for( iMbXUpd = 0; iMbXUpd < m_iMbPerLine;   iMbXUpd++ )
    {
      MbDataAccess* pcMbDataAccess;
      RNOK( initMb( pcMbDataAccess, iMbYUpd, iMbXUpd ) );

      //----- clear weights -----
      for( Int y = 0; y < 4; y++ )
      for( Int x = 0; x < 4; x++ )
      {
        pusUpdateWeights[(4*iMbYUpd+y)*iWStride+(4*iMbXUpd+x)] = 0;
      }
    }
  }


  //====== check for empty reference list =====
  if( pcCtrlDataList->getActive() < 1 )
  {
    //---- set macroblock mode to intra: no update signal ----
    for( iMbYUpd = 0; iMbYUpd < m_iMbPerColumn; iMbYUpd++ )
    for( iMbXUpd = 0; iMbXUpd < m_iMbPerLine;   iMbXUpd++ )
    {
      getMbData( iMbXUpd, iMbYUpd ).setMbMode( INTRA_4X4 );
    }
    return Err::m_nOK;
  }


  //====== first run: derive update motion vectors and set number of connected samples ======
  {
    RNOK( rcConnectionArray.clear() );

    for( iRefIdx = 1; iRefIdx <= (Int)pcCtrlDataList->getActive(); iRefIdx++ )    
    for( iMbYPrd = 0; iMbYPrd <  m_iMbPerColumn;                   iMbYPrd++ )
    for( iMbXPrd = 0; iMbXPrd <  m_iMbPerLine;                     iMbXPrd++ )
    {
      MbDataCtrl*   pcMbDataCtrlPrd = (*pcCtrlDataList)[ iRefIdx ]->getMbDataCtrl();
      MbData&       rcMbDataPrd     = pcMbDataCtrlPrd->getMbData( iMbXPrd, iMbYPrd );
      MbMotionData& rcMbMotDataPrd  = rcMbDataPrd.getMbMotionData( eListPrd );
      MbMode        eMbModePrd      = rcMbDataPrd.getMbMode();

      for( B8x8Idx c8x8IdxPrd; c8x8IdxPrd.isLegal(); c8x8IdxPrd++ )
      {
        if( eMbModePrd>=INTRA_4X4 || rcMbMotDataPrd.getRefIdx( c8x8IdxPrd.b8x8() ) != iRefIdx )
        {
          continue;
        }

        for( S4x4Idx cIdxPrd( c8x8IdxPrd ); cIdxPrd.isLegal( c8x8IdxPrd ); cIdxPrd++ )
        {
          Int       iYPrd   = ( iMbYPrd << 4 )   +  ( cIdxPrd.y() << 2 );
          Int       iXPrd   = ( iMbXPrd << 4 )   +  ( cIdxPrd.x() << 2 );
          Mv        cMvPrd  = rcMbMotDataPrd.getMv( cIdxPrd );

          Int       iDY     = ( cMvPrd.getVer() + 2 ) >> 2;
          Int       iDX     = ( cMvPrd.getHor() + 2 ) >> 2;
          Int       iYUpd   = iYPrd + iDY;
          Int       iXUpd   = iXPrd + iDX;
          Mv        cMvUpd;

          cMvUpd.set( -cMvPrd.getHor(), -cMvPrd.getVer() );

          iMbYUpd = iYUpd >> 4;   iYUpd -= iMbYUpd << 4;    iBlkYUpd = iYUpd >> 2;    iYUpd -= iBlkYUpd << 2;
          iMbXUpd = iXUpd >> 4;   iXUpd -= iMbXUpd << 4;    iBlkXUpd = iXUpd >> 2;    iXUpd -= iBlkXUpd << 2;
    
          for( Int iBY = 0; iBY < 2; iBY++ )
          for( Int iBX = 0; iBX < 2; iBX++ )
          {
            Int iMbY  = iMbYUpd + ( ( iBlkYUpd + iBY ) >> 2 );
            Int iMbX  = iMbXUpd + ( ( iBlkXUpd + iBX ) >> 2 );

            if( iMbY < 0 || iMbY >= m_iMbPerColumn ||
                iMbX < 0 || iMbX >= m_iMbPerLine     )
            {
              continue;
            }

            ConnectionData& rcConnectionData  = rcConnectionArray.getData( iMbX, iMbY );
            B4x4Idx         cIdxUpd             ( 4* ( ( iBlkYUpd + iBY ) % 4 ) + ( iBlkXUpd + iBX ) % 4 );
            Int             iNumPel           = ( iBY ? iYUpd : 4 - iYUpd ) * ( iBX ? iXUpd : 4 - iXUpd );

            xAddConnectionForMV( rcConnectionData, eListUpd, iRefIdx, cIdxUpd, iNumPel, cMvUpd );
          }
        }
      }
    }
  }


  //====== second run: set mb modes, refidx, mv, number of connected samples, and update weights ======
  for( iMbYUpd = 0; iMbYUpd < m_iMbPerColumn; iMbYUpd++ )
  for( iMbXUpd = 0; iMbXUpd < m_iMbPerLine;   iMbXUpd++ )
  {
    MbData&         rcMbDataUpd       = getMbData( iMbXUpd, iMbYUpd );
    MbMotionData&   rcMbMotDataUpd    = rcMbDataUpd.getMbMotionData( eListUpd );
    MbMotionData&   rcMbMotDataZero   = rcMbDataUpd.getMbMotionData( eListPrd );
    ConnectionData& rcConnectionData  = rcConnectionArray.getData( iMbXUpd, iMbYUpd );
    Bool            bAllWeightsZero   = true;

    for( B8x8Idx c8x8IdxUpd; c8x8IdxUpd.isLegal(); c8x8IdxUpd++ )
    {
      Par8x8    ePar          = c8x8IdxUpd.b8x8Index();
      ParIdx8x8 eParIdx       = c8x8IdxUpd.b8x8();
      Int       iMaxConnected = -1;
      Int       iBestRefIdx   =  1;

      //----- get reference indices -----
      for( iRefIdx = 1; iRefIdx <= (Int)pcCtrlDataList->getActive(); iRefIdx++ )
      {
        Int iNumConnected = rcConnectionData.getConnected( eListUpd, iRefIdx, B4x4Idx( eParIdx + SPART_4x4_0 ) )
                          + rcConnectionData.getConnected( eListUpd, iRefIdx, B4x4Idx( eParIdx + SPART_4x4_1 ) )
                          + rcConnectionData.getConnected( eListUpd, iRefIdx, B4x4Idx( eParIdx + SPART_4x4_2 ) )
                          + rcConnectionData.getConnected( eListUpd, iRefIdx, B4x4Idx( eParIdx + SPART_4x4_3 ) );
        if( iNumConnected > iMaxConnected )
        {
          iMaxConnected = iNumConnected;
          iBestRefIdx   = iRefIdx;
        }
      }

      //----- set reference indices and motion vectors for current list -----
      rcMbMotDataUpd  .setRefIdx( iBestRefIdx,                                                               eParIdx );
      rcMbMotDataUpd  .setAllMv ( rcConnectionData.getMv(eListUpd,iBestRefIdx,B4x4Idx(eParIdx+SPART_4x4_0)), eParIdx, SPART_4x4_0 );
      rcMbMotDataUpd  .setAllMv ( rcConnectionData.getMv(eListUpd,iBestRefIdx,B4x4Idx(eParIdx+SPART_4x4_1)), eParIdx, SPART_4x4_1 );
      rcMbMotDataUpd  .setAllMv ( rcConnectionData.getMv(eListUpd,iBestRefIdx,B4x4Idx(eParIdx+SPART_4x4_2)), eParIdx, SPART_4x4_2 );
      rcMbMotDataUpd  .setAllMv ( rcConnectionData.getMv(eListUpd,iBestRefIdx,B4x4Idx(eParIdx+SPART_4x4_3)), eParIdx, SPART_4x4_3 );

      //----- clear motion data for opposite list -----
      rcMbMotDataZero .setRefIdx( BLOCK_NOT_PREDICTED,  eParIdx );
      rcMbMotDataZero .setAllMv ( Mv::ZeroMv(),         eParIdx );
    }


    //----- set weights -----
    for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
    {
      Int iNumConnected       = rcConnectionData.getConnected( eListUpd, rcMbMotDataUpd.getRefIdx( cIdx ), cIdx );
      Int iWeight             = min( 8, max( 0, ( iNumConnected - 8 ) ) ); // 50 - 100%
      Int iPos                = (4*iMbYUpd+cIdx.y())*iWStride + (4*iMbXUpd+cIdx.x());
      pusUpdateWeights[iPos]  = iWeight;
      if( iWeight )
      {
        bAllWeightsZero       = false;
      }
    }


    //----- set macroblock modes -----
    if( bAllWeightsZero )
    {
      rcMbDataUpd   .setMbMode  ( INTRA_4X4 );
      rcMbDataUpd   .setFwdBwd  ( 0x0000 );
      rcMbMotDataUpd.setRefIdx  ( BLOCK_NOT_PREDICTED );
    }
    else
    {
      rcMbDataUpd   .setMbMode ( MODE_8x8 );
      rcMbDataUpd   .setFwdBwd ( eListUpd == LIST_0 ? 0x1111 : 0x2222 );
      rcMbDataUpd   .setBlkMode( B_8x8_0, BLK_4x4 );
      rcMbDataUpd   .setBlkMode( B_8x8_1, BLK_4x4 );
      rcMbDataUpd   .setBlkMode( B_8x8_2, BLK_4x4 );
      rcMbDataUpd   .setBlkMode( B_8x8_3, BLK_4x4 );
    }
  }

  return Err::m_nOK;
}


#undef MEDIAN








ConnectionArray::ConnectionArray()
: m_bInitDone   ( false )
, m_uiSize      ( 0     )
, m_uiNumCol    ( 0     )
, m_uiNumRow    ( 0     )
, m_pcData      ( 0     )
{
}


ConnectionArray::~ConnectionArray()
{
  delete [] m_pcData;
}


ErrVal
ConnectionArray::init( const SequenceParameterSet& rcSPS )
{
  UInt uiSize = rcSPS.getMbInFrame();
  ROF( uiSize );

  if( m_uiSize != uiSize )
  {
    delete [] m_pcData;
    m_pcData  = 0;

    ROFRS( ( m_pcData = new ConnectionData [ uiSize ] ), Err::m_nERR );
    m_uiSize  = uiSize;
  }

  m_uiNumCol   = rcSPS.getFrameWidthInMbs ();
  m_uiNumRow   = rcSPS.getFrameHeightInMbs();
  m_bInitDone  = true;

  return Err::m_nOK;
}

ErrVal
ConnectionArray::clear()
{
  ROF( m_bInitDone );

  for( UInt uiIndex = 0; uiIndex < m_uiSize; uiIndex++ )
  {
    m_pcData[uiIndex].clear();
  }
  return Err::m_nOK;
}










ControlData::ControlData()
: m_pcMbDataCtrl      ( 0   )
, m_pcSliceHeader     ( 0   )
, m_dLambda           ( 0   )
, m_pcBaseLayerRec    ( 0   )
, m_pcBaseLayerSbb    ( 0   )
, m_pcBaseLayerCtrl   ( 0   )
, m_uiUseBLMotion     ( 0   )
, m_dScalingFactor    ( 1.0 )
{
  m_bBaseRep = false;
  m_uiCurTemporalLevel = 0;
  m_uiCurActivePrdL0 = 0;
  m_uiCurActivePrdL1 = 0;
  for ( UInt uiIndex = 0; uiIndex < MAX_DSTAGES; uiIndex++ )
  {
    m_uiCurActiveUpdL0[uiIndex] = 0;
    m_uiCurActiveUpdL1[uiIndex] = 0;
  }
}

ControlData::~ControlData()
{
}

Void
ControlData::clear()
{
  m_pcBaseLayerRec      = 0;
  m_pcBaseLayerSbb      = 0;
  m_pcBaseLayerCtrl     = 0;
  m_uiUseBLMotion       = 0;
  m_dScalingFactor      = 1.0;
  m_bBaseRep = false;
  m_uiCurTemporalLevel = 0;
  m_uiCurActivePrdL0 = 0;
  m_uiCurActivePrdL1 = 0;
  for ( UInt uiIndex = 0; uiIndex < MAX_DSTAGES; uiIndex++ )
  {
    m_uiCurActiveUpdL0[uiIndex] = 0;
    m_uiCurActiveUpdL1[uiIndex] = 0;
  }
}

ErrVal
ControlData::init( SliceHeader*  pcSliceHeader,
                   MbDataCtrl*   pcMbDataCtrl,
                   Double        dLambda )
{
  ROF( pcSliceHeader );
  ROF( pcMbDataCtrl  );

  m_pcSliceHeader = pcSliceHeader;
  m_pcMbDataCtrl  = pcMbDataCtrl;
  m_dLambda       = dLambda;
  
  m_pcBaseLayerRec      = 0;
  m_pcBaseLayerSbb      = 0;
  m_pcBaseLayerCtrl     = 0;
  m_uiUseBLMotion       = 0;

  return Err::m_nOK;
}

ErrVal
ControlData::init( SliceHeader*  pcSliceHeader )
{
  ROF( pcSliceHeader );
  ROF( m_pcMbDataCtrl  );

  m_pcSliceHeader = pcSliceHeader;
  
  m_pcBaseLayerRec      = 0;
  m_pcBaseLayerSbb      = 0;
  m_pcBaseLayerCtrl     = 0;
  m_uiUseBLMotion       = 0;

  return Err::m_nOK;
}












ErrVal MbDataCtrl::getBoundaryMask( Int iMbY, Int iMbX, UInt& ruiMask ) const 
{
  UInt     uiCurrIdx    = iMbY * m_uiMbStride + iMbX + m_uiMbOffset;
  AOT( uiCurrIdx >= m_uiSize );

  ruiMask               = 0;

  ROTRS( m_pcMbData[uiCurrIdx].isIntra(), Err::m_nOK );

  Bool bLeftAvailable   = ( iMbX > 0 );
  Bool bTopAvailable    = ( iMbY > 0 );
  Bool bRightAvailable  = ( iMbX < m_iMbPerLine-1 );
  Bool bBottomAvailable = ( iMbY < m_iMbPerColumn-1 );

  if( bTopAvailable )
  {
    Int iIndex = uiCurrIdx - m_uiMbStride;
    ruiMask |= m_pcMbData[iIndex].isIntra() ? 0x01 :0;

    if( bLeftAvailable )
    {
      Int iIndex = uiCurrIdx - m_uiMbStride - 1;
      ruiMask |= m_pcMbData[iIndex].isIntra() ? 0x80 :0;
    }

    if( bRightAvailable )
    {
      Int iIndex = uiCurrIdx - m_uiMbStride + 1;
      ruiMask |= m_pcMbData[iIndex].isIntra() ? 0x02 :0;
    }
  }

  if( bBottomAvailable )
  {
    Int iIndex = uiCurrIdx + m_uiMbStride;
    ruiMask |= m_pcMbData[iIndex].isIntra() ? 0x10 :0;

    if( bLeftAvailable )
    {
      Int iIndex = uiCurrIdx  + m_uiMbStride - 1;
      ruiMask |= m_pcMbData[iIndex].isIntra() ? 0x20 :0;
    }

    if( bRightAvailable )
    {
      Int iIndex = uiCurrIdx + m_uiMbStride + 1;
      ruiMask |= m_pcMbData[iIndex].isIntra() ? 0x08 :0;
    }
  }

  if( bLeftAvailable )
  {
    Int iIndex = uiCurrIdx-1;
    ruiMask |= m_pcMbData[iIndex].isIntra() ? 0x40 :0;
  }

  if( bRightAvailable )
  {
    Int iIndex = uiCurrIdx + 1;
    ruiMask |= m_pcMbData[iIndex].isIntra() ? 0x04 :0;
  }
  return Err::m_nOK;
}

H264AVC_NAMESPACE_END

