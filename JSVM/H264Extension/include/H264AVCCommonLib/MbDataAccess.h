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





#if !defined(AFX_MBDATAACCESS_H__710205A4_CFE9_496D_A469_C47EC8D2FBD2__INCLUDED_)
#define AFX_MBDATAACCESS_H__710205A4_CFE9_496D_A469_C47EC8D2FBD2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "H264AVCCommonLib/SliceHeader.h"
#include "H264AVCCommonLib/MbData.h"
#include "H264AVCCommonLib/Tables.h"

H264AVC_NAMESPACE_BEGIN


class H264AVCCOMMONLIB_API MbDataAccess
{
private:
  enum PredictionType
  {
    PRED_A,
    PRED_B,
    PRED_C,
    MEDIAN
  };

public:
   Void *operator new( size_t stAllocateBlock, Void* pv )
   {
     return ( NULL == pv ) ? (::operator new( stAllocateBlock )) : pv;
   }

  MbDataAccess( MbDataAccess& rcMbDataAccess, MbData& rcMbData ) :
    m_rcMbCurr(                     rcMbData ),
		m_rcMbComplementary(            rcMbDataAccess.m_rcMbComplementary ),
    m_rcMbLeft(                     rcMbDataAccess.m_rcMbLeft ),
    m_rcMbAbove(                    rcMbDataAccess.m_rcMbAbove ),
    m_rcMbAboveLeft(                rcMbDataAccess.m_rcMbAboveLeft ),
    m_rcMbAboveRight(               rcMbDataAccess.m_rcMbAboveRight ),
    m_rcMbAboveAbove(               rcMbDataAccess.m_rcMbAboveAbove ),
    m_rcMbAboveAboveLeft(           rcMbDataAccess.m_rcMbAboveAboveLeft ),
    m_rcMbAboveAboveRight(          rcMbDataAccess.m_rcMbAboveAboveRight ),
    m_rcMbBelowLeft(                rcMbDataAccess.m_rcMbBelowLeft ),
    m_rcMbUnavailable(              rcMbDataAccess.m_rcMbUnavailable ),
    m_rcMbColocatedTop(             rcMbDataAccess.m_rcMbColocatedTop ),
    m_rcMbColocatedBot(             rcMbDataAccess.m_rcMbColocatedBot ),
    m_rcSliceHeader(                rcMbDataAccess.m_rcSliceHeader ),
    m_rcDFP(                        rcMbDataAccess.m_rcDFP ),
    m_cMv3D_A(                      rcMbDataAccess.m_cMv3D_A ),
    m_cMv3D_B(                      rcMbDataAccess.m_cMv3D_B ),
    m_cMv3D_C(                      rcMbDataAccess.m_cMv3D_C ),
    m_uiAvailable(                  rcMbDataAccess.m_uiAvailable ),
    m_uiPosX(                       rcMbDataAccess.m_uiPosX ),
    m_uiPosY(                       rcMbDataAccess.m_uiPosY ),
    m_bTopMb(                       rcMbDataAccess.m_bTopMb ),
		m_bUseTopField(                 rcMbDataAccess.m_bUseTopField ),
		m_bColocatedField(              rcMbDataAccess.m_bColocatedField ),
		m_bIsTopRowMb(                  rcMbDataAccess.m_bIsTopRowMb ),
    m_bMbAff(                       rcMbDataAccess.m_bMbAff ),
    m_ucLastMbQp(                   rcMbDataAccess.m_ucLastMbQp ),
		m_eMbPicType(                   rcMbDataAccess.m_eMbPicType ),
    m_pcMbDataAccessBase(           rcMbDataAccess.m_pcMbDataAccessBase )
  {
    m_rcMbCurr.copyFrom( rcMbDataAccess.getMbData() );
    //--- set motion data connections ---
    getMbMotionData( LIST_0 ).setFieldMode( rcMbDataAccess.getMbMotionData( LIST_0 ).getFieldMode() );
    getMbMotionData( LIST_1 ).setFieldMode( rcMbDataAccess.getMbMotionData( LIST_1 ).getFieldMode() );
  }


	MbDataAccess( MbData&          rcMbCurr,
		            MbData&          rcMbComplementary,
                const MbData&    rcMbLeft,
                const MbData&    rcMbAbove,
                const MbData&    rcMbAboveLeft,
                const MbData&    rcMbAboveRight,
                const MbData&    rcMbAboveAbove,
                const MbData&    rcMbAboveAboveLeft,
                const MbData&    rcMbAboveAboveRight,
                const MbData&    rcMbBelowLeft,
                const MbData&    rcMbUnavailable,
                const MbData&    rcMbColocatedTop,
                const MbData&    rcMbColocatedBot,
                SliceHeader&     rcSliceHeader,
                const DFPScalable& rcDFP,
                UInt             uiPosX,
                UInt             uiPosY,
								Bool             bTopMb,
								Bool             bUseTopField,
								Bool             bColocatedField,
                SChar               ucLastMbQp )
                : m_rcMbCurr                    ( rcMbCurr                ),
                  m_rcMbComplementary           ( rcMbComplementary       ),
                  m_rcMbLeft                    ( rcMbLeft                ),
                  m_rcMbAbove                   ( rcMbAbove               ),
                  m_rcMbAboveLeft               ( rcMbAboveLeft           ),
                  m_rcMbAboveRight              ( rcMbAboveRight          ),
                  m_rcMbAboveAbove              ( rcMbAboveAbove          ),
                  m_rcMbAboveAboveLeft          ( rcMbAboveAboveLeft      ),
                  m_rcMbAboveAboveRight         ( rcMbAboveAboveRight     ),
                  m_rcMbBelowLeft               ( rcMbBelowLeft           ),
                  m_rcMbUnavailable             ( rcMbUnavailable         ),
                  m_rcMbColocatedTop            ( rcMbColocatedTop      ),
                  m_rcMbColocatedBot            ( rcMbColocatedBot      ),
                  m_rcSliceHeader               ( rcSliceHeader           ),
                  m_rcDFP                       ( rcDFP                   ),
                  m_cMv3D_A                     ( 0, 0, 0                 ),
                  m_cMv3D_B                     ( 0, 0, 0                 ),
                  m_cMv3D_C                     ( 0, 0, 0                 ),
                  m_uiAvailable                 ( 0                       ),
                  m_uiPosX                      ( uiPosX                  ),
                  m_uiPosY                      ( uiPosY                  ),
                  m_bTopMb                      ( bTopMb                  ),
                  m_bUseTopField                ( bUseTopField            ),
				          m_bColocatedField             ( bColocatedField         ),
                  m_bMbAff                      ( rcSliceHeader.isMbAff() ),
                  m_ucLastMbQp                  ( ucLastMbQp              ),
                  m_pcMbDataAccessBase          ( NULL                    )
  {
    m_bIsTopRowMb = ( m_rcSliceHeader.getFieldPicFlag() ? ! m_rcSliceHeader.getBottomFieldFlag() : ( m_uiPosY % 2 == 0 ) );
    m_eMbPicType  = ( m_rcSliceHeader.getFieldPicFlag()
                      ? ( m_rcSliceHeader.getBottomFieldFlag()   ? BOT_FIELD : TOP_FIELD )
                      : ( m_rcMbCurr.getFieldFlag() ? ( m_bTopMb ? TOP_FIELD : BOT_FIELD ) : FRAME ) );
    setAvailableMask();
  }


	~MbDataAccess()
  {
  }


public:

  const MbData&   getMbData             ()    const { return m_rcMbCurr; }
  MbData&         getMbData             ()          { return m_rcMbCurr; }
  const MbData&   getMbDataCurr         ()    const { return m_rcMbCurr; }
  const MbData&   getMbDataLeft         ()    const { return m_rcMbLeft; }
  const MbData&   getMbDataAbove        ()    const { return m_rcMbAbove; }
	const MbData&   getMbDataAboveAbove   ()    const { return m_rcMbAboveAbove; }
  const MbData&   getMbDataAboveLeft    ()    const { return m_rcMbAboveLeft; }
  const MbData&   getMbDataBelowLeft    ()    const { return m_rcMbBelowLeft; }
  const MbData&   getMbDataComplementary()    const { return m_rcMbComplementary; }
  Bool            isTopMb               ()    const { return m_bTopMb; }
	PicType         getMbPicType          ()    const { return m_eMbPicType; }
  Bool            isFieldMbInMbaffFrame ()    const { return ( m_bMbAff ) && ( getMbPicType() != FRAME ); }
  Void            setFieldMode          ( Bool bFieldFlag )
  {
    //----- set field flag in MbDataStruct -----
    m_rcMbCurr         .setFieldFlag( bFieldFlag );
    m_rcMbComplementary.setFieldFlag( bFieldFlag );
    //----- set field mode in MbMotionData -----
    m_rcMbCurr         .getMbMotionData( LIST_0 ).setFieldMode( bFieldFlag );
    m_rcMbCurr         .getMbMotionData( LIST_1 ).setFieldMode( bFieldFlag );
    m_rcMbComplementary.getMbMotionData( LIST_0 ).setFieldMode( bFieldFlag );
    m_rcMbComplementary.getMbMotionData( LIST_1 ).setFieldMode( bFieldFlag );
    //----- set MbPicType -----
    m_eMbPicType = ( m_rcSliceHeader.getFieldPicFlag()
                     ? ( m_rcSliceHeader.getBottomFieldFlag() ? BOT_FIELD : TOP_FIELD )
                     : ( bFieldFlag ? ( isTopMb() ? TOP_FIELD : BOT_FIELD ) : FRAME ) );
    setAvailableMask();
  }
  UInt      getNumActiveRef       ( ListIdx eListIdx) const
  {
    if( m_rcSliceHeader.getPicType() == FRAME && FRAME != getMbPicType() )
    {
      return m_rcSliceHeader.getNumRefIdxActive( eListIdx ) * 2;
    }

    return m_rcSliceHeader.getNumRefIdxActive( eListIdx );
  }

  Bool getDefaultFieldFlag() const
  {
    if( xIsAvailable( m_rcMbLeft ) )
    {
      return m_rcMbLeft.getFieldFlag();
    }
    if( xIsAvailable( m_rcMbAboveAbove ) )
    {
      return m_rcMbAboveAbove.getFieldFlag();
    }
    return false;
  }

  UInt    getConvertBlkMode ( Par8x8 ePar8x8 );
  ErrVal  setConvertBlkMode ( Par8x8 ePar8x8, UInt uiBlockMode );
  UInt    getConvertMbType  ();
  ErrVal  setConvertMbType  ( UInt uiMbType );
  Bool    isSkippedMb       ()                    const
  {
    ROTRS( m_rcSliceHeader.isIntra(), false );
    if( m_rcSliceHeader.isInterB() )
    {
      return (m_rcMbCurr.getMbMode() == MODE_SKIP) &&
             (m_rcMbCurr.getMbCbp () == 0)         &&
            !(m_rcMbCurr.getResidualPredFlag(PART_16x16));
    }
    return m_rcMbCurr.getMbMode() == MODE_SKIP;
  }

  Void    setLastQp   ( Int iQp  )  { m_ucLastMbQp = (UChar)iQp; }

  Void    addDeltaQp  ( Int iDQp )              { m_rcMbCurr.setQp( (m_ucLastMbQp + iDQp+(MAX_QP+1)) % (MAX_QP+1) ); }
  Void    resetQp     ()                        { m_rcMbCurr.setQp( m_ucLastMbQp );  }
  Int     getDeltaQp  ()                        { return (m_rcMbCurr.getQp() - m_ucLastMbQp); }
  UChar   getLastQp  ()                   const { return m_ucLastMbQp; }


  const SliceHeader&       getSH           ()                    const { return m_rcSliceHeader;}
  SliceHeader&             getSH           ()                          { return m_rcSliceHeader;}
  const MbTransformCoeffs& getMbTCoeffs    ()                    const { return m_rcMbCurr.getMbTCoeffs    (); }
  const MbMvData&          getMbMvdData    ( ListIdx eListIdx )  const { return m_rcMbCurr.getMbMvdData    ( eListIdx ); }
  const MbMotionData&      getMbMotionData ( ListIdx eListIdx )  const { return m_rcMbCurr.getMbMotionData ( eListIdx ); }
  MbTransformCoeffs&       getMbTCoeffs    ()                          { return m_rcMbCurr.getMbTCoeffs    (); }
  MbMvData&                getMbMvdData    ( ListIdx eListIdx )        { return m_rcMbCurr.getMbMvdData    ( eListIdx ); }
  MbMotionData&            getMbMotionData ( ListIdx eListIdx )        { return m_rcMbCurr.getMbMotionData ( eListIdx ); }

  Int   mostProbableIntraPredMode ( LumaIdx cIdx );
  Int   encodeIntraPredMode       ( LumaIdx cIdx );
  Int   decodeIntraPredMode       ( LumaIdx cIdx );

  UInt  getAvailableMask  () const { return m_uiAvailable; }
  Bool  isAvailableLeft   () const { return xIsAvailable( m_rcMbLeft  ); }
  Bool  isAvailableAbove  () const { B4x4Idx cIdx; return xIsAvailable( xGetBlockAbove( cIdx ) ); }

  Bool  isLeftMbExisting  () const { return m_uiPosX != 0; }
  Bool  isAboveMbExisting       () const { return ( ! ((m_uiPosY == 0) || ((m_uiPosY == 1) && m_rcMbCurr.getFieldFlag()))); }

  Bool isConstrainedInterLayerPred( )
  {
  	return getSH().getBaseLayerUsesConstrainedIntraPred();
  }

  UInt getCtxChromaPredMode ()                  const;
  UInt getCtxCoeffCount     ( LumaIdx cIdx )    const;
  UInt getCtxCoeffCount     ( ChromaIdx cIdx )  const;
  UInt getCtxMbSkipped      ()                  const;
  UInt getCtxBLSkipFlag     ()                  const;
  UInt getCtxDirectMbWoCoeff()                  const;
  UInt getCtxCodedBlockBit  ( UInt uiBitPos )   const;
  UInt getCtxRefIdx         ( ListIdx eLstIdx, ParIdx8x8 eParIdx ) const;
  UInt getCtxMbIntra4x4     ()                  const;
  UInt getCtxMbType         ()                  const;
  UInt getCtx8x8Flag        ()                  const;

  UInt  getCtxFieldFlag()  const; // TMM_INTERLACE

  UInt  getLeftLumaCbp          ( LumaIdx cIdx )  const;
  UInt  getAboveLumaCbp         ( LumaIdx cIdx )  const;
  UInt  getLeftChromaCbp        ()                const;
  UInt  getAboveChromaCbp       ()                const;
  UInt  getAutoCbp              ()                const;

  UInt  getLeftLumaCbpFGS       ( LumaIdx cIdx )  const;
  UInt  getAboveLumaCbpFGS      ( LumaIdx cIdx )  const;
  UInt  getLeftChromaCbpFGS     ()                const;
  UInt  getAboveChromaCbpFGS    ()                const;

  Void  getMvdLeft    ( Mv& rcMv, ListIdx eListIdx, LumaIdx cIdx )  const;
  Void  getMvdAbove   ( Mv& rcMv, ListIdx eListIdx, LumaIdx cIdx )  const;

  Void  getMvPredictor( Mv& rcMv, SChar scRef, ListIdx eListIdx );
  Void  getMvPredictor( Mv& rcMv, SChar scRef, ListIdx eListIdx, ParIdx16x8 eParIdx );
  Void  getMvPredictor( Mv& rcMv, SChar scRef, ListIdx eListIdx, ParIdx8x16 eParIdx );
  Void  getMvPredictor( Mv& rcMv, SChar scRef, ListIdx eListIdx, ParIdx8x8  eParIdx );
  Void  getMvPredictor( Mv& rcMv, SChar scRef, ListIdx eListIdx, ParIdx8x8  eParIdx, SParIdx8x4 eSParIdx );
  Void  getMvPredictor( Mv& rcMv, SChar scRef, ListIdx eListIdx, ParIdx8x8  eParIdx, SParIdx4x8 eSParIdx );
  Void  getMvPredictor( Mv& rcMv, SChar scRef, ListIdx eListIdx, ParIdx8x8  eParIdx, SParIdx4x4 eSParIdx );
  
  Void  setMvPredictorsBL( const Mv& rcMvBL, ListIdx eListIdx );
  Void  setMvPredictorsBL( const Mv& rcMvBL, ListIdx eListIdx, ParIdx16x8 eParIdx );
  Void  setMvPredictorsBL( const Mv& rcMvBL, ListIdx eListIdx, ParIdx8x16 eParIdx );
  Void  setMvPredictorsBL( const Mv& rcMvBL, ListIdx eListIdx, ParIdx8x8  eParIdx );
  Void  setMvPredictorsBL( const Mv& rcMvBL, ListIdx eListIdx, ParIdx8x8  eParIdx, SParIdx8x4 eSParIdx );
  Void  setMvPredictorsBL( const Mv& rcMvBL, ListIdx eListIdx, ParIdx8x8  eParIdx, SParIdx4x8 eSParIdx );
  Void  setMvPredictorsBL( const Mv& rcMvBL, ListIdx eListIdx, ParIdx8x8  eParIdx, SParIdx4x4 eSParIdx );


  Void  getMvPredictors       ( Mv* pcMv     ) const;
  Void  getMvPredictorSkipMode();
  Void  getMvPredictorSkipMode( Mv& cMvPred  );
  Bool  getMvPredictorDirect( ParIdx8x8 eParIdx, 
                              Bool& rbOneMv, 
                              Bool bFaultTolerant, 
                              RefFrameList* pcL0RefFrameList=NULL, 
                              RefFrameList* pcL1RefFrameList=NULL );
//	TMM_EC {{
  Bool  getMvPredictorDirectVirtual ( ParIdx8x8 eParIdx, Bool& rbOneMv, Bool bFaultTolerant, RefFrameList& rcRefFrameListL0, RefFrameList& rcRefFrameListL1  );
//  TMM_EC }}
  const DFP& getDeblockingFilterParameter(Bool bInterlayer=false)     const { if (bInterlayer) return m_rcDFP.getInterlayerDeblockingFilterParameter();
                                                                                          else return m_rcDFP.getDeblockingFilterParameter();}

  Void getMvSkipMode( Mv& rcMv )
  {
    xGetMvPredictor( rcMv, 1, LIST_0, MEDIAN, B4x4Idx(0), B4x4Idx(3) );

    if( ( m_cMv3D_A.getRef()==BLOCK_NOT_AVAILABLE ||
          m_cMv3D_B.getRef()==BLOCK_NOT_AVAILABLE      ) ||
        ( m_cMv3D_A.getHor()==0 && m_cMv3D_A.getVer()==0 && m_cMv3D_A.getRef()==1 ) ||
        ( m_cMv3D_B.getHor()==0 && m_cMv3D_B.getVer()==0 && m_cMv3D_B.getRef()==1 )   )
    {
      rcMv.setZero();
    }
  }


  MbDataAccess* getMbDataAccessBase() const { return m_pcMbDataAccessBase; }
  Void          setMbDataAccessBase( MbDataAccess* p ) { m_pcMbDataAccessBase = p; }

  UInt  getMbX() const { return m_uiPosX; }
  UInt  getMbY() const { return m_uiPosY; }
  Void  setAvailableMask ();

protected:
  Void  xSetMvPredictorsBL          ( const Mv& rcMvPredBL, ListIdx eListIdx, LumaIdx cIdx, LumaIdx cIdxEnd );

  Void  xGetMvPredictor             ( Mv& rcMvPred, SChar scRef, ListIdx eListIdx, PredictionType ePredType, LumaIdx cIdx, LumaIdx cIdxEnd );
  Void  xSetNeighboursMvPredictor   ( ListIdx eListIdx, LumaIdx cIdx, LumaIdx cIdxEnd );
  Void  xGetMvPredictorUseNeighbours( Mv& rcMvPred, SChar scRef, PredictionType ePredType );

  __inline const MbData& xGetMbLeft           () const;
  __inline const MbData& xGetMbAbove          () const;
  __inline const MbData& xGetBlockLeft        ( LumaIdx&   rcIdx ) const;
  __inline const MbData& xGetBlockLeftBottom  ( LumaIdx&   rcIdx ) const;
  __inline const MbData& xGetBlockAbove       ( LumaIdx&   rcIdx ) const;
  __inline const MbData& xGetBlockAboveLeft   ( LumaIdx&   cIdx ) const;
  __inline const MbData& xGetBlockAboveRight  ( LumaIdx&   cIdx ) const;

  __inline UInt  xGetLeftCodedBlockBit    ( UInt uiBit   )  const;
  __inline UInt  xGetAboveCodedBlockBit   ( UInt uiBit   )  const;
  __inline SChar xGetRefIdxLeft ( ListIdx eListIdx, ParIdx8x8 eParIdx )       const;
  __inline SChar xGetRefIdxAbove( ListIdx eListIdx, ParIdx8x8 eParIdx )       const;
  
  enum MvRefConversion
  {
    ONE_TO_ONE,
    FRM_TO_FLD,
    FLD_TO_FRM
  };
  __inline const MbData& xGetBlockColocated                 ( LumaIdx&  rcIdx, MvRefConversion& eMvRefConversion )  const;
  __inline const MbData& xGetBlockColocatedNonInterlaced    () const;

  __inline Bool xGetMotPredFlagLeft ( ListIdx eListIdx, ParIdx8x8 eParIdx )       const;
  __inline Bool xGetMotPredFlagAbove( ListIdx eListIdx, ParIdx8x8 eParIdx )       const;

  
  __inline Void          xGetColocatedMvRefIdx              ( Mv& rcMv, SChar& rscRefIdx, LumaIdx cIdx ) const;
  __inline const RefPic& xGetColocatedMvRefPic              ( Mv& rcMv, SChar& rscRefIdx, LumaIdx cIdx ) const;

  __inline Void          xGetColocatedMvsRefIdxNonInterlaced( Mv acMv[], SChar& rscRefIdx, ParIdx8x8 eParIdx ) const;
  __inline const RefPic& xGetColocatedMvsRefPicNonInterlaced( Mv acMv[], SChar& rscRefIdx, ParIdx8x8 eParIdx ) const;
 
	Bool xSpatialDirectMode ( ParIdx8x8 eParIdx, Bool b8x8, RefFrameList* pcL0RefFrameList, RefFrameList* pcL1RefFrameList );
  Bool xTemporalDirectMode( ParIdx8x8 eParIdx, Bool b8x8, Bool bFaultTolerant);
  Bool xTemporalDirectModeMvRef( Mv acMv[], SChar ascRefIdx[], LumaIdx cIdx, Bool bFaultTolerant );
  Bool xTemporalDirectModeMvsRefNonInterlaced( Mv aacMv[][4], SChar ascRefIdx[], ParIdx8x8 eParIdx, Bool bFaultTolerant );
//	TMM_EC {{
	Bool xTemporalDirectModeVirtual( ParIdx8x8 eParIdx, Bool b8x8, Bool bFaultTolerant, RefFrameList& rcRefFrameListL0, RefFrameList& rcRefFrameListL1 );
	Bool xTemporalDirectModeMvRefVirtual( Mv acMv[], SChar ascRefIdx[], LumaIdx cIdx, Bool bFaultTolerant, RefFrameList& rcRefFrameListL0, RefFrameList& rcRefFrameListL1);
//  TMM_EC }}

  Bool  xIsAvailable     ( const MbData& rcMbData )  const
  {
    return   ( rcMbData.getSliceId() == m_rcMbCurr.getSliceId() );
  }
  Bool  xIsAvailableIntra( const MbData& rcMbData )  const
  {
    Bool  bConstrainedIntraPred = m_rcSliceHeader.getPPS().getConstrainedIntraPredFlag();
    return ( xIsAvailable( rcMbData ) && ( rcMbData.isIntra() || ! bConstrainedIntraPred ) );
  }

  __inline Bool xCheckMv (const Mv& rcMv) const
  {
    UInt uiLevel = getSH().getSPS().getLevelIdc();
    Int  iMaxVerMv = getSH().getSPS().getMaxIntMvVer(uiLevel, getSH().getFieldPicFlag() || m_rcMbCurr.getFieldFlag());
    Int  iMaxHorMv = getSH().getSPS().getMaxIntMvHor();

    return ((rcMv.getHor() >= (-iMaxHorMv)) && (rcMv.getHor() < iMaxHorMv) && (rcMv.getVer() >= (-iMaxVerMv)) && (rcMv.getVer() < iMaxVerMv));
  }

private:
  static const BlkMode m_aucBMTabB0[13];
  static const UChar   m_aucBMTabB1[13];
  static const BlkMode m_aucBMTabP[4];
  static const MbMode  m_aausInterBMbType0[23];
  static const UShort  m_aausInterBMbType1[23];
  static const UChar   m_aucMbType1x2[9];
  static const UChar   m_aucMbType2x1[9];
  static const UChar   m_aucChroma2LumaIdx[8];
  static const UChar   m_auc4x4Idx28x8Idx[16];

public:
  MbData&        m_rcMbCurr;
	MbData&        m_rcMbComplementary;
  const MbData&  m_rcMbLeft;
  const MbData&  m_rcMbAbove;
  const MbData&  m_rcMbAboveLeft;
  const MbData&  m_rcMbAboveRight;
  const MbData&  m_rcMbAboveAbove;
  const MbData&  m_rcMbAboveAboveLeft;
  const MbData&  m_rcMbAboveAboveRight;
  const MbData&  m_rcMbBelowLeft;
  const MbData&  m_rcMbUnavailable;
  const MbData&  m_rcMbColocatedTop;
  const MbData&  m_rcMbColocatedBot;

  SliceHeader&  m_rcSliceHeader;
  const DFPScalable&          m_rcDFP;

  Mv3D  m_cMv3D_A;
  Mv3D  m_cMv3D_B;
  Mv3D  m_cMv3D_C;

  UInt      m_uiAvailable;
  UInt      m_uiPosX;
  UInt      m_uiPosY;
	Bool      m_bTopMb;
	Bool      m_bUseTopField;
	Bool      m_bColocatedField;
  Bool      m_bIsTopRowMb;
  Bool      m_bMbAff;
  SChar     m_ucLastMbQp;
  PicType   m_eMbPicType;

  MbDataAccess* m_pcMbDataAccessBase;
};

__inline Void MbDataAccess::getMvPredictor( Mv& rcMv, SChar scRef, ListIdx eListIdx )
{
  xGetMvPredictor( rcMv, scRef, eListIdx, MEDIAN, B4x4Idx( 0 ), B4x4Idx( 3 ) );
}
__inline Void MbDataAccess::getMvPredictor( Mv& rcMv, SChar scRef, ListIdx eListIdx, ParIdx16x8 eParIdx )
{
  PredictionType ePred = ( eParIdx == PART_16x8_0 ? PRED_B : PRED_A );
  xGetMvPredictor( rcMv, scRef, eListIdx, ePred, B4x4Idx( eParIdx ), B4x4Idx( eParIdx+3 ) );
}
__inline Void MbDataAccess::getMvPredictor( Mv& rcMv, SChar scRef, ListIdx eListIdx, ParIdx8x16 eParIdx )
{
  PredictionType ePred = ( eParIdx == PART_8x16_0 ? PRED_A : PRED_C );
  xGetMvPredictor( rcMv, scRef, eListIdx, ePred, B4x4Idx( eParIdx ), B4x4Idx( eParIdx+1 ) );
}
__inline Void MbDataAccess::getMvPredictor( Mv& rcMv, SChar scRef, ListIdx eListIdx, ParIdx8x8 eParIdx )
{
  xGetMvPredictor( rcMv, scRef, eListIdx, MEDIAN, B4x4Idx( eParIdx ), B4x4Idx( eParIdx+1 ) );
}
__inline Void MbDataAccess::getMvPredictor( Mv& rcMv, SChar scRef, ListIdx eListIdx, ParIdx8x8 eParIdx, SParIdx8x4 eSParIdx )
{
  xGetMvPredictor( rcMv, scRef, eListIdx, MEDIAN, B4x4Idx( eParIdx+eSParIdx ), B4x4Idx( eParIdx+eSParIdx+1 ) );
}
__inline Void MbDataAccess::getMvPredictor( Mv& rcMv, SChar scRef, ListIdx eListIdx, ParIdx8x8 eParIdx, SParIdx4x8 eSParIdx )
{
  xGetMvPredictor( rcMv, scRef, eListIdx, MEDIAN, B4x4Idx( eParIdx+eSParIdx ), B4x4Idx( eParIdx+eSParIdx ) );
}
__inline Void MbDataAccess::getMvPredictor( Mv& rcMv, SChar scRef, ListIdx eListIdx, ParIdx8x8 eParIdx, SParIdx4x4 eSParIdx )
{
  xGetMvPredictor( rcMv, scRef, eListIdx, MEDIAN, B4x4Idx( eParIdx+eSParIdx ), B4x4Idx( eParIdx+eSParIdx ) );
}


__inline Void MbDataAccess::setMvPredictorsBL( const Mv& rcMv, ListIdx eListIdx )
{
  xSetMvPredictorsBL( rcMv, eListIdx, B4x4Idx( 0 ), B4x4Idx( 3 ) );
}
__inline Void MbDataAccess::setMvPredictorsBL( const Mv& rcMv, ListIdx eListIdx, ParIdx16x8 eParIdx )
{
  xSetMvPredictorsBL( rcMv, eListIdx, B4x4Idx( eParIdx ), B4x4Idx( eParIdx+3 ) );
}
__inline Void MbDataAccess::setMvPredictorsBL( const Mv& rcMv, ListIdx eListIdx, ParIdx8x16 eParIdx )
{
  xSetMvPredictorsBL( rcMv, eListIdx, B4x4Idx( eParIdx ), B4x4Idx( eParIdx+1 ) );
}
__inline Void MbDataAccess::setMvPredictorsBL( const Mv& rcMv, ListIdx eListIdx, ParIdx8x8 eParIdx )
{
  xSetMvPredictorsBL( rcMv, eListIdx, B4x4Idx( eParIdx ), B4x4Idx( eParIdx+1 ) );
}
__inline Void MbDataAccess::setMvPredictorsBL( const Mv& rcMv, ListIdx eListIdx, ParIdx8x8 eParIdx, SParIdx8x4 eSParIdx )
{
  xSetMvPredictorsBL( rcMv, eListIdx, B4x4Idx( eParIdx+eSParIdx ), B4x4Idx( eParIdx+eSParIdx+1 ) );
}
__inline Void MbDataAccess::setMvPredictorsBL( const Mv& rcMv, ListIdx eListIdx, ParIdx8x8 eParIdx, SParIdx4x8 eSParIdx )
{
  xSetMvPredictorsBL( rcMv, eListIdx, B4x4Idx( eParIdx+eSParIdx ), B4x4Idx( eParIdx+eSParIdx ) );
}
__inline Void MbDataAccess::setMvPredictorsBL( const Mv& rcMv, ListIdx eListIdx, ParIdx8x8 eParIdx, SParIdx4x4 eSParIdx )
{
  xSetMvPredictorsBL( rcMv, eListIdx, B4x4Idx( eParIdx+eSParIdx ), B4x4Idx( eParIdx+eSParIdx ) );
}




__inline SChar MbDataAccess::xGetRefIdxLeft( ListIdx eListIdx, ParIdx8x8 eParIdx ) const
{
  B4x4Idx       cIdx( eParIdx );
  const MbData& rcMbData  = xGetBlockLeft( cIdx );

  if( rcMbData.getBLSkipFlag() || rcMbData.getMbMotionData( eListIdx ).getMotPredFlag( ParIdx8x8( cIdx.b4x4() ) ) )
  {
    return BLOCK_NOT_AVAILABLE;
  }
  if( m_rcSliceHeader.isInterB() )
  {
    if( rcMbData.getMbMode() == MODE_SKIP || ( rcMbData.getMbMode() == MODE_8x8 && rcMbData.getBlkMode( Par8x8( 2*(cIdx.y()/2) + cIdx.x()/2 ) ) == BLK_SKIP ) )
    return BLOCK_NOT_AVAILABLE;
  }

  SChar  scRefIdx  = rcMbData.getMbMotionData( eListIdx ).getRefIdx( ParIdx8x8( cIdx.b4x4() ) );

  if( scRefIdx > BLOCK_NOT_AVAILABLE )
  {
    if( rcMbData.getFieldFlag() && ! m_rcMbCurr.getFieldFlag() )
    {
      scRefIdx = ((scRefIdx-1)>>1)+1;
    }
    else if( ! rcMbData.getFieldFlag() && m_rcMbCurr.getFieldFlag() )
    {
      scRefIdx = ((scRefIdx-1)<<1)+1; 
    }
  }

  return scRefIdx;
}

__inline SChar MbDataAccess::xGetRefIdxAbove( ListIdx eListIdx, ParIdx8x8 eParIdx ) const
{
  B4x4Idx       cIdx( eParIdx );
  const MbData& rcMbData  = xGetBlockAbove( cIdx );

  if( rcMbData.getBLSkipFlag() || rcMbData.getMbMotionData( eListIdx ).getMotPredFlag( ParIdx8x8( cIdx.b4x4() ) ) )
  {
    return BLOCK_NOT_AVAILABLE;
  }
  if( m_rcSliceHeader.isInterB() )
  {
    if( rcMbData.getMbMode() == MODE_SKIP || ( rcMbData.getMbMode() == MODE_8x8 && rcMbData.getBlkMode( Par8x8( 2*(cIdx.y()/2) + cIdx.x()/2 ) ) == BLK_SKIP ) )
    return BLOCK_NOT_AVAILABLE;
  }

  SChar  scRefIdx  = rcMbData.getMbMotionData( eListIdx ).getRefIdx( ParIdx8x8( cIdx.b4x4() ) );

  if( scRefIdx > BLOCK_NOT_AVAILABLE )
  {
    if( rcMbData.getFieldFlag() && ! m_rcMbCurr.getFieldFlag() )
    {
      scRefIdx = ((scRefIdx-1)>>1)+1;
    }
    else if( ! rcMbData.getFieldFlag() && m_rcMbCurr.getFieldFlag() )
    {
      scRefIdx = ((scRefIdx-1)<<1)+1;
    }
  }

  return scRefIdx;
}



__inline Bool MbDataAccess::xGetMotPredFlagLeft( ListIdx eListIdx, ParIdx8x8 eParIdx ) const
{
  B4x4Idx       cIdx( eParIdx );
  const MbData& rcMbData  = xGetBlockLeft( cIdx );

  Bool  bFlag  = rcMbData.getMbMotionData( eListIdx ).getMotPredFlag( ParIdx8x8( cIdx.b4x4() ) );

  return bFlag;
}


__inline Bool MbDataAccess::xGetMotPredFlagAbove( ListIdx eListIdx, ParIdx8x8 eParIdx ) const
{
  B4x4Idx       cIdx( eParIdx );
  const MbData& rcMbData  = xGetBlockAbove( cIdx );

  Bool  bFlag  = rcMbData.getMbMotionData( eListIdx ).getMotPredFlag( ParIdx8x8( cIdx.b4x4() ) );

  return bFlag;
}



__inline Void MbDataAccess::getMvdLeft( Mv& rcMv, ListIdx eListIdx, LumaIdx cIdx ) const
{
  const MbData& rcMbData = xGetBlockLeft( cIdx );
  rcMv = rcMbData.getMbMvdData( eListIdx ).getMv( cIdx );
  if( rcMbData.getFieldFlag() && ! m_rcMbCurr.getFieldFlag() )
  {
    rcMv.setVer( rcMv.getVer() * 2 );
  }
  else if( ! rcMbData.getFieldFlag() && m_rcMbCurr.getFieldFlag() )
  {
    rcMv.setVer( rcMv.getVer() / 2 );
  }
}

__inline Void MbDataAccess::getMvdAbove( Mv& rcMv, ListIdx eListIdx, LumaIdx cIdx ) const
{
  const MbData& rcMbData = xGetBlockAbove( cIdx );
  rcMv = rcMbData.getMbMvdData( eListIdx ).getMv( cIdx );
  if( rcMbData.getFieldFlag() && ! m_rcMbCurr.getFieldFlag() )
  {
    rcMv.setVer( rcMv.getVer() * 2 );
  }
  else if( ! rcMbData.getFieldFlag() && m_rcMbCurr.getFieldFlag() )
  {
    rcMv.setVer( rcMv.getVer() / 2 );
  }
}




__inline UInt MbDataAccess::getCtxCoeffCount( LumaIdx cIdx )  const
{
  Bool bLeftAvailable = false;
  UInt uiCoeffCount   = 0;

  {
    B4x4Idx cIdxL( cIdx.b4x4() );
    const MbData& rcMbDataLeft = xGetBlockLeft( cIdxL );

    if( xIsAvailable( rcMbDataLeft ) )
    {
      bLeftAvailable = true;
      uiCoeffCount = rcMbDataLeft.getMbTCoeffs().getCoeffCount( cIdxL );
    }
  }

  const MbData& rcMbDataAbove = xGetBlockAbove( cIdx );
  if( xIsAvailable( rcMbDataAbove ) )
  {
    uiCoeffCount += rcMbDataAbove.getMbTCoeffs().getCoeffCount( cIdx );

    if( bLeftAvailable )
    {
      uiCoeffCount  += 1;
      uiCoeffCount >>= 1;
    }
  }

  if( 4 > uiCoeffCount )
  {
    uiCoeffCount >>= 1;
  }
  else
  {
    uiCoeffCount = ( 8 > uiCoeffCount) ? 2 : 3;
  }

  return uiCoeffCount;
}


__inline UInt MbDataAccess::getCtxCoeffCount( ChromaIdx cIdx )  const
{
  Bool bLeftAvailable = false;
  UInt uiCoeffCount     = 0;

  B4x4Idx       cLumIdx( m_aucChroma2LumaIdx[ cIdx ] );
  Int           iComp = ( cIdx >> 2) << 2;

  {
    B4x4Idx cLumIdxL = cLumIdx;
    const MbData& rcMbDataLeft = xGetBlockLeft( cLumIdxL );

    if( xIsAvailable( rcMbDataLeft ) )
    {
      bLeftAvailable = true;
      uiCoeffCount = rcMbDataLeft.getMbTCoeffs().getCoeffCount( CIdx( iComp + m_auc4x4Idx28x8Idx[ cLumIdxL.b4x4() ] ) );
    }
  }

  const MbData& rcMbDataAbove = xGetBlockAbove( cLumIdx );
  if( xIsAvailable( rcMbDataAbove ) )
  {
    uiCoeffCount += rcMbDataAbove.getMbTCoeffs().getCoeffCount( CIdx( iComp + m_auc4x4Idx28x8Idx[ cLumIdx.b4x4() ] ) );
    if( bLeftAvailable )
    {
      uiCoeffCount  += 1;
      uiCoeffCount >>= 1;
    }
  }

  if( 4 > uiCoeffCount )
  {
    uiCoeffCount >>= 1;
  }
  else
  {
    uiCoeffCount = ( 8 > uiCoeffCount) ? 2 : 3;
  }

  return uiCoeffCount;
}


__inline UInt MbDataAccess::getCtxChromaPredMode() const
{
  UInt uiCtx;

  uiCtx  = ((xGetMbLeft ().getChromaPredMode() > 0) ? 1 : 0);
  uiCtx += ((xGetMbAbove().getChromaPredMode() > 0) ? 1 : 0);

  return uiCtx;
}


__inline Int MbDataAccess::mostProbableIntraPredMode( LumaIdx cIdx )
{
  B4x4Idx cIdxL( cIdx.b4x4() );
  B4x4Idx cIdxA( cIdx.b4x4() );

  const MbData& rcMbDataLeft = xGetBlockLeft( cIdxL );
  Int iLeftPredMode  = ( xIsAvailableIntra( rcMbDataLeft ) ? rcMbDataLeft.intraPredMode( cIdxL ) : OUTSIDE);

  const MbData& rcMbDataAbove = xGetBlockAbove( cIdxA );
  Int iAbovePredMode = ( xIsAvailableIntra( rcMbDataAbove ) ? rcMbDataAbove.intraPredMode( cIdxA ) : OUTSIDE);

  Int iMostProbable = min( iLeftPredMode, iAbovePredMode );

  return ( OUTSIDE == iMostProbable ) ? DC_PRED : iMostProbable;
}

__inline Int MbDataAccess::encodeIntraPredMode( LumaIdx cIdx )
{
  const Int iMostProbable   = mostProbableIntraPredMode( cIdx );
  const Int iIntraPredMode  = m_rcMbCurr.intraPredMode ( cIdx );

  ROTRS( iMostProbable == iIntraPredMode, -1 )

  return (iIntraPredMode < iMostProbable) ? iIntraPredMode : iIntraPredMode-1;
}

__inline Int MbDataAccess::decodeIntraPredMode( LumaIdx cIdx )
{
  const Int iMostProbable   = mostProbableIntraPredMode( cIdx );
  const Int iIntraPredMode  = m_rcMbCurr.intraPredMode ( cIdx );

  ROTRS( -1 == iIntraPredMode, iMostProbable )

  return (iIntraPredMode < iMostProbable) ? iIntraPredMode : iIntraPredMode+1;
}

__inline UInt MbDataAccess::getCtxFieldFlag()  const
{
  UInt uiCtx = (m_rcMbLeft.getFieldFlag() ? 1: 0);

  if( m_bMbAff && ! isTopMb() )
  {
    uiCtx += (m_rcMbAboveAbove.getFieldFlag() ? 1: 0);
  }
  else
  {
    uiCtx += (m_rcMbAbove.getFieldFlag() ? 1: 0);
  }

  return uiCtx;
}

__inline UInt MbDataAccess::getCtxMbType() const
{
  const MbData& rcMbLeft   = xGetMbLeft ();
  const MbData& rcMbAbove  = xGetMbAbove();
  Bool bSkippedLeft  = ( !rcMbLeft .getBLSkipFlag() && rcMbLeft .isSkiped() );
  Bool bSkippedAbove = ( !rcMbAbove.getBLSkipFlag() && rcMbAbove.isSkiped() );
  UInt uiCtx         = ( bSkippedLeft  ? 0 : 1 );
  uiCtx             += ( bSkippedAbove ? 0 : 1 );
  return uiCtx;
}


__inline UInt MbDataAccess::getCtx8x8Flag() const
{
  UInt uiCtx = ( xGetMbLeft ().isTransformSize8x8() ? 1 : 0);
  uiCtx     += ( xGetMbAbove().isTransformSize8x8() ? 1 : 0);
  return uiCtx;
}


__inline UInt MbDataAccess::getCtxMbSkipped() const
{
  const Bool bFieldFlag = m_rcMbCurr.getFieldFlag();

  if( m_bMbAff && ( isTopMb() || m_rcMbComplementary.getSkipFlag()  ) )
  {
    m_rcMbCurr.setFieldFlag( getDefaultFieldFlag() );
  }

  const MbData& rcMbDataLeft   = xGetMbLeft();
  const MbData& rcMbDataAbove   = xGetMbAbove();

  UInt uiCtx = (( rcMbDataLeft .getSkipFlag()) ? 0 : 1);
  uiCtx     += (( rcMbDataAbove.getSkipFlag()) ? 0 : 1);


  m_rcMbCurr.setFieldFlag( bFieldFlag );
  return uiCtx;
}


__inline UInt MbDataAccess::getCtxBLSkipFlag() const
{
  const MbData& rcMbDataLeft  = xGetMbLeft  ();
  const MbData& rcMbDataAbove = xGetMbAbove ();

  UInt  uiCtx  = ( rcMbDataLeft .getBLSkipFlag() ? 0 : 1 );
  uiCtx       += ( rcMbDataAbove.getBLSkipFlag() ? 0 : 1 );

  return uiCtx;
}


__inline UInt MbDataAccess::getCtxDirectMbWoCoeff() const
{
  const MbData& rcMbDataLeft   = xGetMbLeft();
  const MbData& rcMbDataAbove  = xGetMbAbove();

  UInt uiCtx = (( rcMbDataLeft .getSkipFlag()) ? 0 : 1);
  uiCtx     += (( rcMbDataAbove.getSkipFlag()) ? 0 : 1);

  return uiCtx;
}


__inline UInt MbDataAccess::getCtxMbIntra4x4() const
{
  const MbData& rcMbDataLeft  = xGetMbLeft();
  const MbData& rcMbDataAbove = xGetMbAbove();
  Bool bIntraLeft  = ( !xIsAvailable( rcMbDataLeft  ) || ( !rcMbDataLeft .getBLSkipFlag() && rcMbDataLeft .isIntra4x4() ) );
  Bool bIntraAbove = ( !xIsAvailable( rcMbDataAbove ) || ( !rcMbDataAbove.getBLSkipFlag() && rcMbDataAbove.isIntra4x4() ) );
  UInt uiCtx       = ( bIntraLeft  ? 0 : 1 );
  uiCtx           += ( bIntraAbove ? 0 : 1 );
  return uiCtx;
}

__inline UInt MbDataAccess::getCtxRefIdx( ListIdx eLstIdx, ParIdx8x8 eParIdx ) const
{
  UInt uiCtx;

  uiCtx  = xGetRefIdxAbove( eLstIdx, eParIdx ) > 1 ? 2 : 0;
  uiCtx += xGetRefIdxLeft ( eLstIdx, eParIdx ) > 1 ? 1 : 0;

  return uiCtx;
}


__inline UInt MbDataAccess::getLeftLumaCbp( LumaIdx cIdx )  const
{
  const MbData& rcMbData = xGetBlockLeft( cIdx );

  if( ! xIsAvailable( rcMbData ) )
  {
    return 1;
  }
  if( !rcMbData.getBLSkipFlag() && rcMbData.isIntra16x16() )
  {
    return rcMbData.isAcCoded() ? 1 : 0;
  }
  return ( rcMbData.getMbCbp() >> m_auc4x4Idx28x8Idx[ cIdx.b4x4() ] ) & 1;
}

__inline UInt MbDataAccess::getAboveLumaCbp( LumaIdx cIdx )  const
{
  const MbData& rcMbData = xGetBlockAbove( cIdx );

  if( ! xIsAvailable( rcMbData ) )
  {
    return 1;
  }
  if( !rcMbData.getBLSkipFlag() && rcMbData.isIntra16x16() )
  {
    return rcMbData.isAcCoded() ? 1 : 0;
  }
  return ( rcMbData.getMbCbp() >> m_auc4x4Idx28x8Idx[ cIdx.b4x4() ] ) & 1;
}

__inline UInt MbDataAccess::getLeftChromaCbp()  const
{
  const MbData& rcMbData = xGetMbLeft();

  if( !rcMbData.getBLSkipFlag() && rcMbData.isIntra16x16() )
  {
    return rcMbData.getCbpChroma16x16();
  }
  return rcMbData.getMbCbp() >> 4;
}

__inline UInt MbDataAccess::getAboveChromaCbp()  const
{
  const MbData& rcMbData = xGetMbAbove();

  if( !rcMbData.getBLSkipFlag() && rcMbData.isIntra16x16() )
  {
    return rcMbData.getCbpChroma16x16();
  }
  return rcMbData.getMbCbp() >> 4;
}


__inline UInt MbDataAccess::getAutoCbp() const
{
  UInt  uiCbp = m_rcMbCurr.getMbCbp();

  if( m_rcMbCurr.isIntra16x16() )
  {
    uiCbp  = ( m_rcMbCurr.isAcCoded() ? 15 : 0 ); //luma
    uiCbp += ( m_rcMbCurr.getCbpChroma16x16() << 4 ); //chroma
  }

  return uiCbp;
}

__inline UInt MbDataAccess::getLeftLumaCbpFGS( LumaIdx cIdx )  const
{
  const MbData& rcMbData = xGetBlockLeft( cIdx );

  if( ! xIsAvailable( rcMbData ) )
  {
    return 1;
  }
  return ( rcMbData.getMbCbp() >> m_auc4x4Idx28x8Idx[ cIdx.b4x4() ] ) & 1;
}

__inline UInt MbDataAccess::getAboveLumaCbpFGS( LumaIdx cIdx )  const
{
  const MbData& rcMbData = xGetBlockAbove( cIdx );

  if( ! xIsAvailable( rcMbData ) )
  {
    return 1;
  }
  return ( rcMbData.getMbCbp() >> m_auc4x4Idx28x8Idx[ cIdx.b4x4() ] ) & 1;
}

__inline UInt MbDataAccess::getLeftChromaCbpFGS()  const
{
  const MbData& rcMbData = xGetMbLeft();

  return rcMbData.getMbCbp() >> 4;
}

__inline UInt MbDataAccess::getAboveChromaCbpFGS()  const
{
  const MbData& rcMbData = xGetMbAbove();

  return rcMbData.getMbCbp() >> 4;
}


__inline UInt MbDataAccess::xGetLeftCodedBlockBit( UInt uiBit )  const
{
  AOT_DBG( uiBit > 30 );
  if( uiBit > 26 )
  {
    B4x4Idx       cIdx( m_aucChroma2LumaIdx[ uiBit - 27 ] );
    const MbData& rcMbData  = xGetBlockLeft( cIdx );
    if( ! xIsAvailable( rcMbData ) )
    {
      return ( !m_rcMbCurr.getBLSkipFlag() && m_rcMbCurr.isIntra() ? 1 : 0 );
    }
    return rcMbData.getBCBP( m_auc4x4Idx28x8Idx[ cIdx.b4x4() ] + 27 );
  }

  if( uiBit >= 24 ) // macroblock based stuff
  {
    const MbData& rcMbData = xGetMbLeft();
    if( ! xIsAvailable( rcMbData ) )
    {
      return ( !m_rcMbCurr.getBLSkipFlag() && m_rcMbCurr.isIntra() ? 1 : 0 );
    }
    return rcMbData.getBCBP( uiBit );
  }

  if( uiBit < 16 ) // luma block based
  {
    B4x4Idx       cIdx( uiBit );
    const MbData& rcMbData = xGetBlockLeft( cIdx );
    if( ! xIsAvailable( rcMbData ) )
    {
      return ( !m_rcMbCurr.getBLSkipFlag() && m_rcMbCurr.isIntra() ? 1 : 0 );
    }
    return rcMbData.getBCBP( cIdx.b4x4() );
  }
  // chroma block based
  B4x4Idx       cIdx( m_aucChroma2LumaIdx[ uiBit - 16 ] );
  Int           iOffset   = ( uiBit >= 20 ? 20 : 16 );
  const MbData& rcMbData  = xGetBlockLeft( cIdx );
  if( ! xIsAvailable( rcMbData ) )
  {
    return ( !m_rcMbCurr.getBLSkipFlag() && m_rcMbCurr.isIntra() ? 1 : 0 );
  }
  return rcMbData.getBCBP( m_auc4x4Idx28x8Idx[ cIdx.b4x4() ] + iOffset );
}

__inline UInt MbDataAccess::xGetAboveCodedBlockBit( UInt uiBit )  const
{
  AOT_DBG( uiBit > 30 );
  if( uiBit > 26 )
  {
    B4x4Idx       cIdx( m_aucChroma2LumaIdx[ uiBit - 27 ] );
    const MbData& rcMbData  = xGetBlockAbove( cIdx );
    if( ! xIsAvailable( rcMbData ) )
    {
      return ( !m_rcMbCurr.getBLSkipFlag() && m_rcMbCurr.isIntra() ? 1 : 0 );
    }
    return rcMbData.getBCBP( m_auc4x4Idx28x8Idx[ cIdx.b4x4() ] + 27 );
  }

  if( uiBit >= 24 ) // macroblock based stuff
  {
    const MbData& rcMbData = xGetMbAbove();
    if( ! xIsAvailable( rcMbData ) )
    {
      return ( !m_rcMbCurr.getBLSkipFlag() && m_rcMbCurr.isIntra() ? 1 : 0 );
    }
    return rcMbData.getBCBP( uiBit );
  }
  if( uiBit < 16 ) // luma block based
  {
    B4x4Idx       cIdx( uiBit );
    const MbData& rcMbData = xGetBlockAbove( cIdx );
    if( ! xIsAvailable( rcMbData ) )
    {
      return ( !m_rcMbCurr.getBLSkipFlag() && m_rcMbCurr.isIntra() ? 1 : 0 );
    }
    return rcMbData.getBCBP( cIdx.b4x4() );
  }
  // chroma block based
  B4x4Idx       cIdx( m_aucChroma2LumaIdx[ uiBit - 16 ] );
  Int           iOffset   = ( uiBit >= 20 ? 20 : 16 );
  const MbData& rcMbData  = xGetBlockAbove( cIdx );
  if( ! xIsAvailable( rcMbData ) )
  {
    return ( !m_rcMbCurr.getBLSkipFlag() && m_rcMbCurr.isIntra() ? 1 : 0 );
  }
  return rcMbData.getBCBP( m_auc4x4Idx28x8Idx[ cIdx.b4x4() ] + iOffset );
}


__inline UInt MbDataAccess::getCtxCodedBlockBit( UInt uiBitPos ) const
{
  UInt uiCtx;
  uiCtx  = xGetLeftCodedBlockBit ( uiBitPos );
  uiCtx += xGetAboveCodedBlockBit( uiBitPos ) << 1;
  return uiCtx;
}


__inline Void MbDataAccess::xGetColocatedMvRefIdx( Mv& rcMv, SChar& rscRefIdx, LumaIdx cIdx ) const
{
  ListIdx         eListIdx         = LIST_0;
  MvRefConversion eMvRefConversion = ONE_TO_ONE;
  const MbData&   rcMbColocated    = xGetBlockColocated( cIdx, eMvRefConversion );

  if( ( rscRefIdx = rcMbColocated.getMbMotionDataBase( eListIdx ).getRefIdx( cIdx ) ) < BLOCK_NOT_AVAILABLE )
  {
    eListIdx  = LIST_1;
    rscRefIdx = rcMbColocated.getMbMotionDataBase( eListIdx ).getRefIdx( cIdx );
  }
  if( rscRefIdx < BLOCK_NOT_AVAILABLE )
  {
    rcMv = Mv::ZeroMv();
  }
  else
  {
    rcMv = rcMbColocated.getMbMotionDataBase( eListIdx ).getMv( cIdx );
  }
}



__inline Void MbDataAccess::xGetColocatedMvsRefIdxNonInterlaced( Mv acMv[], SChar& rscRefIdx, ParIdx8x8 eParIdx ) const
{
  ListIdx         eListIdx         = LIST_0;
  const MbData&   rcMbColocated    = xGetBlockColocatedNonInterlaced();
  if( ( rscRefIdx = rcMbColocated.getMbMotionDataBase( eListIdx ).getRefIdx( eParIdx ) ) < BLOCK_NOT_AVAILABLE )
  {
    eListIdx  = LIST_1;
    rscRefIdx = rcMbColocated.getMbMotionDataBase( eListIdx ).getRefIdx( eParIdx );
  }
  if( rscRefIdx < BLOCK_NOT_AVAILABLE )
  {
    acMv[0] = acMv[1] = acMv[2] = acMv[3] = Mv::ZeroMv();
  }
  else
  {
    acMv[0] = rcMbColocated.getMbMotionDataBase( eListIdx ).getMv( eParIdx, SPART_4x4_0 );
    acMv[1] = rcMbColocated.getMbMotionDataBase( eListIdx ).getMv( eParIdx, SPART_4x4_1 );
    acMv[2] = rcMbColocated.getMbMotionDataBase( eListIdx ).getMv( eParIdx, SPART_4x4_2 );
    acMv[3] = rcMbColocated.getMbMotionDataBase( eListIdx ).getMv( eParIdx, SPART_4x4_3 );
  }
}


__inline const RefPic& MbDataAccess::xGetColocatedMvRefPic( Mv& rcMv, SChar& rscRefIdx, LumaIdx cIdx ) const
{
  ListIdx         eListIdx         = LIST_0;
  MvRefConversion eMvRefConversion = ONE_TO_ONE;
  const MbData&   rcMbColocated    = xGetBlockColocated( cIdx, eMvRefConversion );
  if( ( rscRefIdx = rcMbColocated.getMbMotionDataBase( eListIdx ).getRefIdx( cIdx ) ) < BLOCK_NOT_AVAILABLE  )
  {
    eListIdx  = LIST_1;
    rscRefIdx = rcMbColocated.getMbMotionDataBase( eListIdx ).getRefIdx( cIdx );
  }
  if( rscRefIdx < BLOCK_NOT_AVAILABLE  )
  {
    rcMv = Mv::ZeroMv();
  }
  else
  {
    rcMv = rcMbColocated.getMbMotionDataBase( eListIdx ).getMv( cIdx );
  }

    //--- mv scaling ----
  if( eMvRefConversion == FRM_TO_FLD )
  {
    rcMv.setFrameToFieldPredictor();
  }
  else if( eMvRefConversion == FLD_TO_FRM )
  {
    rcMv.setFieldToFramePredictor();
  }
  return rcMbColocated.getMbMotionDataBase( eListIdx ).getRefPic( cIdx );
}

__inline const RefPic& MbDataAccess::xGetColocatedMvsRefPicNonInterlaced( Mv acMv[], SChar& rscRefIdx, ParIdx8x8 eParIdx ) const
{
  ListIdx         eListIdx         = LIST_0;
   const MbData&   rcMbColocated    = xGetBlockColocatedNonInterlaced();
  if( ( rscRefIdx = rcMbColocated.getMbMotionDataBase( eListIdx ).getRefIdx( eParIdx ) ) < BLOCK_NOT_AVAILABLE )
  {
    eListIdx  = LIST_1;
    rscRefIdx = rcMbColocated.getMbMotionDataBase( eListIdx ).getRefIdx( eParIdx );
  }
  if( rscRefIdx < BLOCK_NOT_AVAILABLE )
  {
    acMv[0] = acMv[1] = acMv[2] = acMv[3] = Mv::ZeroMv();
  }
  else
  {
    acMv[0] = rcMbColocated.getMbMotionDataBase( eListIdx ).getMv( eParIdx, SPART_4x4_0 );
    acMv[1] = rcMbColocated.getMbMotionDataBase( eListIdx ).getMv( eParIdx, SPART_4x4_1 );
    acMv[2] = rcMbColocated.getMbMotionDataBase( eListIdx ).getMv( eParIdx, SPART_4x4_2 );
    acMv[3] = rcMbColocated.getMbMotionDataBase( eListIdx ).getMv( eParIdx, SPART_4x4_3 );
  }

  return rcMbColocated.getMbMotionDataBase( eListIdx ).getRefPic( eParIdx );
}


__inline const MbData& MbDataAccess::xGetMbLeft() const
{
  if( m_rcMbCurr.getFieldFlag() == m_rcMbLeft.getFieldFlag() ) // same frame/field mode
  {
  return m_rcMbLeft;
}
  return ( isTopMb() ? m_rcMbLeft : m_rcMbAboveLeft );
}

__inline const MbData& MbDataAccess::xGetMbAbove() const
{
  if( ! m_bMbAff )
  {
    return m_rcMbAbove;
  }

  if( ! m_rcMbCurr.getFieldFlag() || ( isTopMb() && ! m_rcMbAbove.getFieldFlag() ) )
  {
  return m_rcMbAbove;
}
  return m_rcMbAboveAbove;
}


__inline const MbData& MbDataAccess::xGetBlockLeft( LumaIdx& rcIdx ) const
{
  if( rcIdx.x() )  // inside current macroblock
  {
    rcIdx = rcIdx + CURR_MB_LEFT_NEIGHBOUR;
    return m_rcMbCurr;
  }

  if( ! m_bMbAff )
  {
  rcIdx = rcIdx + LEFT_MB_LEFT_NEIGHBOUR;
  return m_rcMbLeft;
}

  if( m_rcMbCurr.getFieldFlag() == m_rcMbLeft.getFieldFlag() ) // same frame/field mode
  {
  rcIdx = rcIdx + LEFT_MB_LEFT_NEIGHBOUR;
  return m_rcMbLeft;
}
  if( m_rcMbCurr.getFieldFlag() && ! m_rcMbLeft.getFieldFlag() ) // current field, neighbour frame
  {
    Int i4x4 = rcIdx.b4x4();
    rcIdx = B4x4Idx( i4x4 % 8 ? 11 : 3 );
    return ( i4x4 > 4 && isTopMb() ? m_rcMbBelowLeft : i4x4 <= 4 && ! isTopMb() ? m_rcMbAboveLeft : m_rcMbLeft );
  }
  // current frame, neighbour field
  if( isTopMb() )
  {
    rcIdx = B4x4Idx( rcIdx.b4x4() > 4 ?  7 :  3 );
    return m_rcMbLeft;
  }
  rcIdx = B4x4Idx( rcIdx.b4x4() > 4 ? 15 : 11 );
  return m_rcMbAboveLeft;
}

__inline const MbData& MbDataAccess::xGetBlockLeftBottom( LumaIdx& rcIdx ) const
{
  if( rcIdx.x() )  // inside current macroblock
  {
    rcIdx = rcIdx + CURR_MB_LEFT_NEIGHBOUR;
    return m_rcMbCurr;
  }

  if( ! m_bMbAff )
  {
    rcIdx = rcIdx + LEFT_MB_LEFT_NEIGHBOUR;
    return m_rcMbLeft;
  }

  if( m_rcMbCurr.getFieldFlag() == m_rcMbLeft.getFieldFlag() ) // same frame/field mode
  {
    rcIdx = rcIdx + LEFT_MB_LEFT_NEIGHBOUR;
    return m_rcMbLeft;
  }
  if( m_rcMbCurr.getFieldFlag() && ! m_rcMbLeft.getFieldFlag() ) // current field, neighbour frame
  {
    Int i4x4 = rcIdx.b4x4();
    rcIdx = B4x4Idx( i4x4 % 8 ? 15 : 7 );
    return ( i4x4 > 4 && isTopMb() ? m_rcMbBelowLeft : i4x4 <= 4 && ! isTopMb() ? m_rcMbAboveLeft : m_rcMbLeft );
  }
  // current frame, neighbour field
  if( isTopMb() )
  {
    rcIdx = B4x4Idx( rcIdx.b4x4() > 4 ?  7 :  3 );
    return m_rcMbBelowLeft;
  }
  rcIdx = B4x4Idx( rcIdx.b4x4() > 4 ? 15 : 11 );
  return m_rcMbLeft;
}

__inline const MbData& MbDataAccess::xGetBlockAbove( LumaIdx& rcIdx ) const
{
  if( rcIdx.y() ) // inside current macroblock
  {
    rcIdx = rcIdx + CURR_MB_ABOVE_NEIGHBOUR;
    return m_rcMbCurr;
  }
  // outside current macroblock
  rcIdx = rcIdx + ABOVE_MB_ABOVE_NEIGHBOUR;

  if( ! m_bMbAff )
  {
  return m_rcMbAbove;
}

  if( ! m_rcMbCurr.getFieldFlag() || ( isTopMb() && ! m_rcMbAbove.getFieldFlag() ) )
  {
    return m_rcMbAbove;
  }
  return m_rcMbAboveAbove;
}

__inline const MbData& MbDataAccess::xGetBlockAboveLeft( LumaIdx& cIdx ) const
{
  if( cIdx.x() )
  {
    if( cIdx.y() ) // inside current macroblock
    {
      cIdx = cIdx + CURR_MB_ABOVE_NEIGHBOUR + CURR_MB_LEFT_NEIGHBOUR;
      return m_rcMbCurr;
    }
    // inside above macroblock
    cIdx = cIdx + CURR_MB_LEFT_NEIGHBOUR;
    return xGetBlockAbove( cIdx );
  }
  if( cIdx.y() ) // inside left macroblock
  {
    cIdx = cIdx + CURR_MB_ABOVE_NEIGHBOUR;
    return xGetBlockLeftBottom( cIdx );
  }
  // inside above left macroblock
  cIdx = B4x4Idx( ! m_rcMbCurr.getFieldFlag() && ! isTopMb() && m_rcMbAboveLeft.getFieldFlag() ? 7 : 15 );

  if( ! m_rcMbCurr.getFieldFlag() && ! isTopMb() && m_rcMbAboveLeft.getFieldFlag() )
  {
    return m_rcMbLeft;
  }
  if( ! m_rcMbCurr.getFieldFlag() || ( isTopMb() && ! m_rcMbAboveLeft.getFieldFlag() ) )
  {
  return m_rcMbAboveLeft;
}
  return m_rcMbAboveAboveLeft;
}

__inline const MbData& MbDataAccess::xGetBlockAboveRight( LumaIdx& cIdx ) const
{
  if( cIdx.x() < 3 )
  {
    if( cIdx.y() ) // inside current macroblock
    {
      cIdx = cIdx + CURR_MB_ABOVE_NEIGHBOUR + CURR_MB_RIGHT_NEIGHBOUR;

      if( cIdx.b4x4() == 2 || cIdx.b4x4() == 10 )
      {
        return m_rcMbUnavailable;
      }
      return m_rcMbCurr;
    }
    // inside above macroblock pair
    cIdx = cIdx + CURR_MB_RIGHT_NEIGHBOUR;
    return xGetBlockAbove( cIdx );
  }

  if( cIdx.y() || ( ! m_rcMbCurr.getFieldFlag() && ! isTopMb() ) ) // not available ( inside right mb or inside above right macroblock for a bottom macroblock )
  {
    cIdx = cIdx + RIGHT_MB_RIGHT_NEIGHBOUR;
    return m_rcMbUnavailable;
  }
  // inside above rigth macroblock pair
  cIdx = B4x4Idx( 12 );
  if( ! m_rcMbCurr.getFieldFlag() || ( isTopMb() && ! m_rcMbAboveRight.getFieldFlag() ) )
  {
  return m_rcMbAboveRight;
}
  return m_rcMbAboveAboveRight;
}

__inline const MbData& MbDataAccess::xGetBlockColocated( LumaIdx& rcIdx, MvRefConversion& eMvRefConversion ) const
{
  Bool bCurrField = getSH().getFieldPicFlag() || m_rcMbCurr.        getFieldFlag();
  Bool bColField  = m_bColocatedField         || m_rcMbColocatedTop.getFieldFlag();

  if( bCurrField == bColField )
  {
    eMvRefConversion = ONE_TO_ONE;
    return ( m_bIsTopRowMb ? m_rcMbColocatedTop : m_rcMbColocatedBot );
  }

  if( bCurrField )
  {
    eMvRefConversion    = FRM_TO_FLD;
    const MbData& rcCol = ( rcIdx.b4x4() < 8 ? m_rcMbColocatedTop : m_rcMbColocatedBot );
    rcIdx               = B4x4Idx( rcIdx.x() + 4 * ( ( 2 * rcIdx.y() ) % 4 ) );
    return rcCol;
  }

  eMvRefConversion  = FLD_TO_FRM;
  rcIdx             = B4x4Idx( rcIdx.x() + 4 * ( rcIdx.y() / 2 ) + ( m_bIsTopRowMb ? 0 : 8 ) );
  return ( m_bUseTopField ? m_rcMbColocatedTop : m_rcMbColocatedBot );
}

__inline const MbData& MbDataAccess::xGetBlockColocatedNonInterlaced() const
{
  return ( m_bIsTopRowMb ? m_rcMbColocatedTop : m_rcMbColocatedBot );
}

H264AVC_NAMESPACE_END


#endif // !defined(AFX_MBDATAACCESS_H__710205A4_CFE9_496D_A469_C47EC8D2FBD2__INCLUDED_)
