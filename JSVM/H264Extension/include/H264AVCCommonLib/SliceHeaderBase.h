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




#if !defined(AFX_SLICEHEADERBASE_H__2CC1FD0F_CACB_4799_84BE_FC5FC9B9C245__INCLUDED_)
#define AFX_SLICEHEADERBASE_H__2CC1FD0F_CACB_4799_84BE_FC5FC9B9C245__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "H264AVCCommonLib/SequenceParameterSet.h"
#include "H264AVCCommonLib/PictureParameterSet.h"
#include "H264AVCCommonLib/Frame.h"
#include "H264AVCCommonLib/Tables.h"

#include "H264AVCCommonLib/HeaderSymbolWriteIf.h"
#include "H264AVCCommonLib/HeaderSymbolReadIf.h"



H264AVC_NAMESPACE_BEGIN

#include <math.h>
class FMO;

#if defined( WIN32 )
# pragma warning( disable: 4275 )
# pragma warning( disable: 4251 )
#endif


enum RplrOp
{
  RPLR_NEG   = 0,
  RPLR_POS   = 1,
  RPLR_LONG  = 2,
  RPLR_END   = 3
};



class H264AVCCOMMONLIB_API Rplr
{
public:
	Rplr( RplrOp  eRplrOp = RPLR_END,
        UInt    uiVal   = 0 )
  : m_eRplrOp ( eRplrOp )
  , m_uiVal   ( uiVal   )
  {
  }

  const RplrOp& getCommand( UInt& ruiVal ) const
  {
    ruiVal = m_uiVal;
    return m_eRplrOp;
  }

  Bool operator != ( const Rplr& rcRplr ) const
  {
    ROTRS( m_eRplrOp != rcRplr.m_eRplrOp, true );
    ROTRS( m_uiVal   != rcRplr.m_uiVal,   true );
    return false;
  }

  Rplr& operator = ( const Rplr& rcRplr ) 
  {
    m_eRplrOp = rcRplr.m_eRplrOp;
    m_uiVal   = rcRplr.m_uiVal;
    return *this;
  }

  Bool isEnd() const
  {
    return RPLR_END == m_eRplrOp;
  }

  ErrVal write( HeaderSymbolWriteIf*  pcWriteIf,
                Bool&                 rbContinue ) const
  {
    rbContinue = ( m_eRplrOp != RPLR_END );

    RNOK( pcWriteIf->writeUvlc( m_eRplrOp,    "RPLR: remapping_of_pic_nums_idc" ) );

    if( rbContinue )
    {
      switch (m_eRplrOp)
      {
      case 0:
      case 1:
        RNOK( pcWriteIf->writeUvlc( m_uiVal,  "RPLR: abs_diff_pic_num_minus1" ) );
        break;
      case 2:
        RNOK( pcWriteIf->writeUvlc( m_uiVal,  "RPLR: long_term_pic_num" ) );
        break;
      default:
        RERR(); 
      }
    }
    return Err::m_nOK;
  }

  ErrVal read( HeaderSymbolReadIf*  pcReadIf,
               Bool&                rbContinue )
  {
    UInt uiCommand;
    RNOK( pcReadIf->getUvlc( uiCommand,   "RPLR: remapping_of_pic_nums_idc" ) );
    
    m_eRplrOp   = RplrOp( uiCommand );
    rbContinue  = ( m_eRplrOp != RPLR_END );

    if( rbContinue )
    {
      switch (m_eRplrOp)
      {
      case 0:
      case 1:
        RNOK( pcReadIf->getUvlc( m_uiVal, "RPLR: abs_diff_pic_num_minus1" ) );
        break;
      case 2:
        RNOK( pcReadIf->getUvlc( m_uiVal, "RPLR: long_term_pic_num" ) );
        break;
      default:
        RVAL( Err::m_nInvalidParameter ); 
      }

    }
    return Err::m_nOK;
  }

private:
  RplrOp  m_eRplrOp;
  UInt    m_uiVal;
};



class H264AVCCOMMONLIB_API RplrBuffer :
public StatBuf<Rplr,33>
{
public:
  RplrBuffer()
  : m_bRefPicListReorderingFlag( false )
  {
  }

  ErrVal write( HeaderSymbolWriteIf* pcWriteIf ) const
  {
    RNOK( pcWriteIf->writeFlag( m_bRefPicListReorderingFlag,  "RIR: ref_pic_list_reordering_flag" ) );
    ROFRS( m_bRefPicListReorderingFlag, Err::m_nOK );

    Bool bCont = true;
    for( Int iIndex = 0; bCont; iIndex++ )
    {
      RNOK( get( iIndex ).write( pcWriteIf, bCont ) );
    }

    return Err::m_nOK;
  }

  ErrVal read( HeaderSymbolReadIf*  pcReadIf,
               UInt                 uiNumRefIdx )
  {
    RNOK( pcReadIf->getFlag( m_bRefPicListReorderingFlag,     "RIR: ref_pic_list_reordering_flag" ) );
    
    ROFRS( m_bRefPicListReorderingFlag, Err::m_nOK );

    Bool bCont    = true;
    UInt uiIndex  = 0;
    for( ; uiIndex <= uiNumRefIdx && bCont; uiIndex++ )
    {
      RNOK( get( uiIndex ).read( pcReadIf, bCont ) );
    }
    ROTR( uiIndex > uiNumRefIdx && bCont, Err::m_nInvalidParameter );

    return Err::m_nOK;
  }

  Void clear                      ()                    { setAll( Rplr() ); }
  Bool getRefPicListReorderingFlag()              const { return m_bRefPicListReorderingFlag; }
  Void setRefPicListReorderingFlag( Bool bFlag )        { m_bRefPicListReorderingFlag = bFlag; }

  Void copy( const StatBuf<Rplr,32>& rcSrcRplrBuffer)
  {
    for( Int n = 0; n < 32; n++ )
    {
      get(n) = rcSrcRplrBuffer.get( n );
      ROTVS( get(n).isEnd() );
    }
  }

protected:
  Bool m_bRefPicListReorderingFlag;
};



enum MmcoOp
{
  MMCO_END                = 0,
	MMCO_SHORT_TERM_UNUSED  = 1,
	MMCO_LONG_TERM_UNUSED   = 2,
	MMCO_ASSIGN_LONG_TERM   = 3,
	MMCO_MAX_LONG_TERM_IDX  = 4,
	MMCO_RESET              = 5,
  MMCO_SET_LONG_TERM      = 6
};



class Mmco
{
public:
  Mmco( MmcoOp  eMmcoOp  = MMCO_END,
        UInt    uiVal1   = 0,
        UInt    uiVal2   = 0 )
  : m_eMmcoOp ( eMmcoOp )
  , m_uiVal1  ( uiVal1  )
  , m_uiVal2  ( uiVal2  )
  {
  }

  Bool operator != ( const Mmco& rcMmco ) const
  {
    ROTRS( m_eMmcoOp != rcMmco.m_eMmcoOp, true );
    ROTRS( m_uiVal1  != rcMmco.m_uiVal1,  true );
    ROTRS( m_uiVal2  != rcMmco.m_uiVal2,  true );
    return false;
  }

  Mmco& operator = ( const Mmco& rcMmco ) 
  {
    m_eMmcoOp = rcMmco.m_eMmcoOp;
    m_uiVal1  = rcMmco.m_uiVal1;
    m_uiVal2  = rcMmco.m_uiVal2;
    return *this;
  }

  const MmcoOp& getCommand( UInt&   ruiVal1,
                            UInt&   ruiVal2 ) const { ruiVal1 = m_uiVal1; ruiVal2 = m_uiVal2; return m_eMmcoOp; }
  Bool          isCommand ( MmcoOp  eMmcoOp ) const { return eMmcoOp == m_eMmcoOp; }
  UInt          getVal1   ()                  const { return m_uiVal1; }
  UInt          getVal2   ()                  const { return m_uiVal2; }
  Bool          isEnd     ()                  const { return MMCO_END == m_eMmcoOp; }

  
  ErrVal write( HeaderSymbolWriteIf*  pcWriteIf,
                Bool&                 rbContinue ) const
  {
    RNOK( pcWriteIf->writeUvlc( m_eMmcoOp,    "DRPM: memory_mangement_control_operation" ) );

    rbContinue = (m_eMmcoOp != MMCO_END);

    if( (m_eMmcoOp != MMCO_END) && (m_eMmcoOp != MMCO_RESET) )
    {
      RNOK( pcWriteIf->writeUvlc( m_uiVal1,   "DRPM: MMCO Param" ) );

      if( m_eMmcoOp == MMCO_ASSIGN_LONG_TERM )
      {
        RNOK( pcWriteIf->writeUvlc( m_uiVal2, "DRPM: MMCO Param" ) );
      }
    }
    return Err::m_nOK;
  }


  ErrVal read( HeaderSymbolReadIf*  pcReadIf,
               Bool&                rbContinue )
  {
    UInt uiCommand;
    RNOK( pcReadIf->getUvlc( uiCommand,     "DRPM: memory_mangement_control_operation" ) );

    ROTR( MMCO_SET_LONG_TERM < uiCommand, Err::m_nInvalidParameter );
    m_eMmcoOp = MmcoOp( uiCommand );

    rbContinue = (m_eMmcoOp != MMCO_END);

    if( (m_eMmcoOp != MMCO_END) && (m_eMmcoOp != MMCO_RESET) )
    {
      RNOK( pcReadIf->getUvlc( m_uiVal1,    "DRPM: MMCO Param" ) );

      if( m_eMmcoOp == MMCO_ASSIGN_LONG_TERM )
      {
        RNOK( pcReadIf->getUvlc( m_uiVal2,  "DRPM: MMCO Param" ) );
      }
    }
    return Err::m_nOK;
  }

private:
  MmcoOp  m_eMmcoOp;
  UInt    m_uiVal1;
  UInt    m_uiVal2;
};




class H264AVCCOMMONLIB_API MmcoBuffer :
public StatBuf<Mmco,32>
{
  public:
  Void clear()
  {
    setAll( MmcoOp() );
  }

  ErrVal write( HeaderSymbolWriteIf* pcWriteIf ) const
  {
    Bool bCont = true;
    for( Int iIndex = 0; bCont; iIndex++ )
    {
      RNOK( get( iIndex ).write( pcWriteIf, bCont ) );
    }
    return Err::m_nOK;
  }

  ErrVal read( HeaderSymbolReadIf* pcReadIf )
  {
    Bool bCont = true;
    for( Int iIndex = 0; bCont; iIndex++ )
    {
      RNOK( get( iIndex ).read( pcReadIf, bCont ) );
    }
    return Err::m_nOK;
  }

  Void copy( const StatBuf<Mmco,32>& rcSrcMmcoBuffer)
  {
    for( Int n = 0; n < 32; n++ )
    {
      get(n) = rcSrcMmcoBuffer.get( n );
      ROTVS( get(n).isEnd() );
    }
  }
};





class H264AVCCOMMONLIB_API SliceHeaderBase
{
public:
  class H264AVCCOMMONLIB_API PredWeight
  {
  public:
    PredWeight() : m_bLumaWeightFlag( false ), m_bChromaWeightFlag( false ), m_iLumaWeight( 0 ), m_iLumaOffset( 0 )
    {
      m_aiChromaWeight[0] = m_aiChromaWeight[1] = 0;
      m_aiChromaOffset[0] = m_aiChromaOffset[1] = 0;
    }
    ~PredWeight() {}

    ErrVal createRandomParameters(); // just for encoder testing

    Void scaleL1Weight( Int iDistScaleFactor )
    {
      iDistScaleFactor >>= 2;
      if( (iDistScaleFactor > 128) || (iDistScaleFactor < -64) )
      {
        iDistScaleFactor = 32;
      }

      m_iLumaWeight       = iDistScaleFactor;
      m_aiChromaWeight[0] = iDistScaleFactor;
      m_aiChromaWeight[1] = iDistScaleFactor;
    }
    Void scaleL0Weight( const PredWeight& rcPredWeightL1 )
    {
      m_iLumaWeight       = 64 - rcPredWeightL1.m_iLumaWeight;
      m_aiChromaWeight[0] = 64 - rcPredWeightL1.m_aiChromaWeight[0];
      m_aiChromaWeight[1] = 64 - rcPredWeightL1.m_aiChromaWeight[1];
    }
    ErrVal init( Int iLumaWeight, Int iChromaCbWeight, Int iChromaCrWeight )
    {
      m_iLumaWeight       = iLumaWeight;
      m_aiChromaWeight[0] = iChromaCbWeight;
      m_aiChromaWeight[1] = iChromaCrWeight;
      return Err::m_nOK;
    }

//TMM_WP
    ErrVal initOffsets( Int iLumaOffset, Int iChromaCbOffset, Int iChromaCrOffset )
    {
        m_iLumaOffset = iLumaOffset;
        m_aiChromaOffset[0] = iChromaCbOffset;
        m_aiChromaOffset[1] = iChromaCrOffset;
        return Err::m_nOK;
    }

    ErrVal setOffsets(  const Double *pafOffsets );
    ErrVal getOffsets( Double *afOffset);
//TMM_WP
    Bool  getLumaWeightFlag()                                  const { return m_bLumaWeightFlag; }
    Bool  getChromaWeightFlag()                                const { return m_bChromaWeightFlag; }
    Int   getLumaWeight()                                      const { return m_iLumaWeight; }
    Int   getLumaOffset()                                      const { return m_iLumaOffset; }
    Int   getChromaWeight( UInt uiChromaPlane )                const { return m_aiChromaWeight[uiChromaPlane]; }
    Int   getChromaOffset( UInt uiChromaPlane )                const { return m_aiChromaOffset[uiChromaPlane]; }

    Void  setLumaWeightFlag( Bool bLumaWeightFlag )                  { m_bLumaWeightFlag = bLumaWeightFlag; }
    Void  setChromaWeightFlag( Bool bChromaWeightFlag )              { m_bChromaWeightFlag = bChromaWeightFlag; }
    Void  setLumaWeight( Int iLumaWeight )                           { m_iLumaWeight = iLumaWeight; }
    Void  setLumaOffset( Int iLumaOffset )                           { m_iLumaOffset = iLumaOffset; }
    Void  setChromaWeight( UInt uiChromaPlane, Int iChromaWeight )   { m_aiChromaWeight[uiChromaPlane] = iChromaWeight; }
    Void  setChromaOffset( UInt uiChromaPlane, Int iChromaOffset )   { m_aiChromaOffset[uiChromaPlane] = iChromaOffset; }

//TMM_WP
    ErrVal getPredWeights( Double *afWeight);
    ErrVal setPredWeightsAndFlags( const Int iLumaScale, 
                                   const Int iChromaScale, 
                                   const Double *pfWeight, 
                                   Double fDiscardThr );
//TMM_WP

    ErrVal write( HeaderSymbolWriteIf*  pcWriteIf ) const;
    ErrVal read ( HeaderSymbolReadIf*   pcReadIf  );

    Bool  operator!=( const PredWeight& rcPredWeight ) const
    {
      ROTRS( m_bLumaWeightFlag    != rcPredWeight.m_bLumaWeightFlag,    true );
      ROTRS( m_bChromaWeightFlag  != rcPredWeight.m_bChromaWeightFlag,  true );
      ROTRS( m_iLumaWeight        != rcPredWeight.m_iLumaWeight,        true );
      ROTRS( m_iLumaOffset        != rcPredWeight.m_iLumaOffset,        true );
      ROTRS( m_aiChromaWeight[0]  != rcPredWeight.m_aiChromaWeight[0],  true );
      ROTRS( m_aiChromaWeight[1]  != rcPredWeight.m_aiChromaWeight[1],  true );
      ROTRS( m_aiChromaOffset[0]  != rcPredWeight.m_aiChromaOffset[0],  true );
      ROTRS( m_aiChromaOffset[1]  != rcPredWeight.m_aiChromaOffset[1],  true );
      return false;
    }
    Bool  operator==( const PredWeight& rcPredWeight ) const
    {
      return !(*this != rcPredWeight);
    }

    Void  copy( const PredWeight& rcPredWeight )
    {
      m_bLumaWeightFlag   = rcPredWeight.m_bLumaWeightFlag;
      m_bChromaWeightFlag = rcPredWeight.m_bChromaWeightFlag;
      m_iLumaWeight       = rcPredWeight.m_iLumaWeight;
      m_iLumaOffset       = rcPredWeight.m_iLumaOffset;
      m_aiChromaWeight[0] = rcPredWeight.m_aiChromaWeight[0];
      m_aiChromaWeight[1] = rcPredWeight.m_aiChromaWeight[1];
      m_aiChromaOffset[0] = rcPredWeight.m_aiChromaOffset[0];
      m_aiChromaOffset[1] = rcPredWeight.m_aiChromaOffset[1];
    }

  private:
    Bool  m_bLumaWeightFlag;
    Bool  m_bChromaWeightFlag;
    Int   m_iLumaWeight;
    Int   m_iLumaOffset;
    Int   m_aiChromaWeight[2];
    Int   m_aiChromaOffset[2];
  };

  class H264AVCCOMMONLIB_API PredWeightTable : public DynBuf<PredWeight>
  {
  public:
    ErrVal initDefaults( UInt uiLumaWeightDenom, UInt uiChromaWeightDenom );
    ErrVal createRandomParameters();

    ErrVal write( HeaderSymbolWriteIf*  pcWriteIf,  UInt uiNumber ) const;
    ErrVal read ( HeaderSymbolReadIf*   pcReadIf,   UInt uiNumber );

//TMM_WP
    ErrVal setPredWeightsAndFlags( const Int iLumaScale, 
                                   const Int iChromaScale, 
                                   const Double(*pafWeight)[3], 
                                   Double fDiscardThr );

    ErrVal setOffsets(  const Double(*pafOffsets)[3] );
//TMM_WP
    ErrVal copy ( const PredWeightTable& rcPredWeightTable );
  };

  class H264AVCCOMMONLIB_API DeblockingFilterParameter
  {
  public:
    DeblockingFilterParameter( UInt uiDisableDeblockingFilterIdc  = 0,
                               Int  iSliceAlphaC0Offset           = 0,
                               Int  iSliceBetaOffset              = 0 )
    : m_uiDisableDeblockingFilterIdc( uiDisableDeblockingFilterIdc )
    , m_iSliceAlphaC0Offset         ( iSliceAlphaC0Offset )
    , m_iSliceBetaOffset            ( iSliceBetaOffset )
    {
    }

    ErrVal write( HeaderSymbolWriteIf*  pcWriteIf ) const;
    ErrVal read ( HeaderSymbolReadIf*   pcReadIf );

    DeblockingFilterParameter* getCopy()  const
    {
      return new DeblockingFilterParameter( m_uiDisableDeblockingFilterIdc, m_iSliceAlphaC0Offset, m_iSliceBetaOffset );
    }
    
    Void setDisableDeblockingFilterIdc( UInt uiDisableDeblockingFilterIdc ) { m_uiDisableDeblockingFilterIdc = uiDisableDeblockingFilterIdc; }
    Void setSliceAlphaC0Offset        ( Int  iSliceAlphaC0Offset )          { AOT_DBG( 1 & iSliceAlphaC0Offset);  m_iSliceAlphaC0Offset = iSliceAlphaC0Offset; }
    Void setSliceBetaOffset           ( Int  iSliceBetaOffset )             { AOT_DBG( 1 & iSliceBetaOffset);     m_iSliceBetaOffset = iSliceBetaOffset; }

    UInt getDisableDeblockingFilterIdc()  const { return m_uiDisableDeblockingFilterIdc;}
    Int  getSliceAlphaC0Offset()          const { return m_iSliceAlphaC0Offset;}
    Int  getSliceBetaOffset()             const { return m_iSliceBetaOffset;}

  private:
    UInt m_uiDisableDeblockingFilterIdc;
    Int  m_iSliceAlphaC0Offset;
    Int  m_iSliceBetaOffset;
  };



protected:
	SliceHeaderBase         ( const SequenceParameterSet& rcSPS,
                            const PictureParameterSet&  rcPPS );
	virtual ~SliceHeaderBase();


public:
  ErrVal    read    ( HeaderSymbolReadIf*   pcReadIf  );
  ErrVal    write   ( HeaderSymbolWriteIf*  pcWriteIf ) const;


  //===== get properties =====
  Bool                              isH264AVCCompatible           ()  const { return ( m_eNalUnitType == NAL_UNIT_CODED_SLICE  ||
                                                                                       m_eNalUnitType == NAL_UNIT_CODED_SLICE_IDR ); }
  Bool                              isIdrNalUnit                  ()  const { return ( m_eNalUnitType == NAL_UNIT_CODED_SLICE_IDR || 
                                                                                       m_eNalUnitType == NAL_UNIT_CODED_SLICE_IDR_SCALABLE ); }
  Int                               getPicQp                      ()  const { return m_rcPPS.getPicInitQp() + getSliceQpDelta(); }
  UInt                              getMbInPic                    ()  const { return m_rcSPS.getMbInFrame(); }

  
  //===== get parameter sets =====
  const PictureParameterSet&        getPPS                        ()  const { return m_rcPPS; }
  const SequenceParameterSet&       getSPS                        ()  const { return m_rcSPS; }


  //===== get parameters =====
  NalRefIdc                         getNalRefIdc                  ()  const { return m_eNalRefIdc; }
  NalUnitType                       getNalUnitType                ()  const { return m_eNalUnitType; }
  UInt                              getLayerId                    ()  const { return m_uiLayerId; }
  UInt                              getTemporalLevel              ()  const { return m_uiTemporalLevel; }
  UInt                              getQualityLevel               ()  const { return m_uiQualityLevel; }
  UInt                              getFirstMbInSlice             ()  const { return m_uiFirstMbInSlice; }
  SliceType                         getSliceType                  ()  const { return m_eSliceType; }
  UInt                              getPicParameterSetId          ()  const { return m_uiPicParameterSetId; }
  UInt                              getFrameNum                   ()  const { return m_uiFrameNum; }
  UInt                              getNumMbsInSlice              ()  const { return m_uiNumMbsInSlice; }
  Bool                              getFgsComponentSep            ()  const { return m_bFgsComponentSep; }
  UInt                              getIdrPicId                   ()  const { return m_uiIdrPicId; }
  UInt                              getPicOrderCntLsb             ()  const { return m_uiPicOrderCntLsb; }
  Int                               getDeltaPicOrderCntBottom     ()  const { return m_iDeltaPicOrderCntBottom; }
  Int                               getDeltaPicOrderCnt    (UInt ui)  const { return m_aiDeltaPicOrderCnt[ui]; }
  Bool                              getBasePredWeightTableFlag    ()  const { return m_bBasePredWeightTableFlag; }
  UInt                              getLumaLog2WeightDenom        ()  const { return m_uiLumaLog2WeightDenom; }
  UInt                              getChromaLog2WeightDenom      ()  const { return m_uiChromaLog2WeightDenom; }
  const PredWeightTable&            getPredWeightTable   (ListIdx e)  const { return m_acPredWeightTable[e]; }
  PredWeightTable&                  getPredWeightTable   (ListIdx e)        { return m_acPredWeightTable[e]; }
  const PredWeight&                 getPredWeight        (ListIdx e,
                                                          UInt   ui)  const { return m_acPredWeightTable[e].get(ui-1); }
  PredWeight&                       getPredWeight        (ListIdx e,
                                                          UInt   ui)        { return m_acPredWeightTable[e].get(ui-1); }

//TMM_WP
  ErrVal copyWeightedPred(PredWeightTable& pcPredWeightTable, UInt uiLumaLogWeightDenom,
                          UInt uiChromaWeightDenom, ListIdx eListIdx, Bool bDecoder);
//TMM_WP

  Bool                              getDirectSpatialMvPredFlag    ()  const { return m_bDirectSpatialMvPredFlag; }
  Bool                              getKeyPictureFlag             ()  const { return m_bKeyPictureFlag; }
  UInt                              getBaseLayerId                ()  const { return m_uiBaseLayerId; }
  UInt                              getBaseQualityLevel           ()  const { return m_uiBaseQualityLevel; }
  Bool                              getAdaptivePredictionFlag     ()  const { return m_bAdaptivePredictionFlag; }
  Bool                              getNumRefIdxActiveOverrideFlag()  const { return m_bNumRefIdxActiveOverrideFlag; }
  UInt                              getNumRefIdxActive ( ListIdx e )  const { return m_auiNumRefIdxActive[e]; }
  const RplrBuffer&                 getRplrBuffer      ( ListIdx e )  const { return m_acRplrBuffer      [e]; }
  RplrBuffer&                       getRplrBuffer      ( ListIdx e )        { return m_acRplrBuffer      [e]; }
  UInt                              getNumRefIdxUpdate ( UInt    ui,
                                                         ListIdx e )  const { return m_aauiNumRefIdxActiveUpdate[ui][e]; }
  Bool                              getNoOutputOfPriorPicsFlag    ()  const { return m_bNoOutputOfPriorPicsFlag; }
  Bool                              getAdaptiveRefPicBufferingFlag()  const { return m_bAdaptiveRefPicBufferingModeFlag; }
  const MmcoBuffer&                 getMmcoBuffer                 ()  const { return m_cMmmcoBuffer; }
  MmcoBuffer&                       getMmcoBuffer                 ()        { return m_cMmmcoBuffer; }
  //TMM_EC {{
  Void								setDefualtMmcoBuffer(UInt uiDecompositionStages, Bool Number2)
  {
	m_cMmmcoBuffer.clear();

	UInt uiCount=0;
	UInt	uiGopSize	=	1 << uiDecompositionStages;
	for( Int iIndex=uiGopSize/2-2;iIndex>=0;iIndex-- )
	{
		m_cMmmcoBuffer.set( uiCount++, Mmco(MMCO_SHORT_TERM_UNUSED,iIndex));
	}
	if(!Number2)m_cMmmcoBuffer.set(uiCount++,Mmco(MMCO_SHORT_TERM_UNUSED,uiGopSize/2));
	else       m_cMmmcoBuffer.set(uiCount++,Mmco(MMCO_SHORT_TERM_UNUSED,uiGopSize-1));

	m_cMmmcoBuffer.set(uiCount++,Mmco(MMCO_END));

    return ;	  
  }

  UInt                              getCabacInitIdc               ()  const { return m_uiCabacInitIdc; }
  Int                               getSliceQpDelta               ()  const { return m_iSliceQpDelta; }
  const DeblockingFilterParameter&  getDeblockingFilterParameter  ()  const { return m_cDeblockingFilterParameter; }
  DeblockingFilterParameter&        getDeblockingFilterParameter  ()        { return m_cDeblockingFilterParameter; }

  UInt  getLog2MaxSliceGroupChangeCycle(UInt uiPicSizeInMapUnits) const {return UInt(ceil( (log ( uiPicSizeInMapUnits*(getPPS().getSliceGroupChangeRateMinus1()+1.)+ 1. ))/log(2.) ));};
  UInt  getSliceGroupChangeCycle() const {return m_uiSliceGroupChangeCycle;}
  FMO*  getFMO() const { return  m_pcFMO;}
  Int getNumMbInSlice();

  UInt                              getBaseWeightZeroBaseBlock()            { return m_uiBaseWeightZeroBaseBlock;     }
  UInt                              getBaseWeightZeroBaseCoeff()            { return m_uiBaseWeightZeroBaseCoeff;     }

  Void  setBaseWeightZeroBaseBlock(UInt ui)
  { 
    m_uiBaseWeightZeroBaseBlock = (ui >= AR_FGS_MAX_BASE_WEIGHT - 1) 
      ? AR_FGS_MAX_BASE_WEIGHT : ui; 
  }
  Void  setBaseWeightZeroBaseCoeff(UInt ui)
  {
    m_uiBaseWeightZeroBaseCoeff = (ui >= AR_FGS_MAX_BASE_WEIGHT - 1) 
      ? AR_FGS_MAX_BASE_WEIGHT : ui; 
  }

  Void  setArFgsUsageFlag               ( Bool b  )         { m_bArFgsUsageFlag       = b;  }
  Bool  getArFgsUsageFlag               ()                  { return m_bArFgsUsageFlag;     }

  Void  setFgsEntropyOrderFlag               ( Bool b  )         { m_bFgsEntropyOrderFlag       = b;  }
  Bool  getFgsEntropyOrderFlag               ()                  { return m_bFgsEntropyOrderFlag;     }
  
  Void  setLowPassFgsMcFilter           ( UInt ui )         { m_uiLowPassFgsMcFilter  = ui;  }
  UInt  getLowPassFgsMcFilter           ()                  { return m_uiLowPassFgsMcFilter;  }
  

  //{{Variable Lengh NAL unit header data with priority and dead substream flag
  //France Telecom R&D- (nathalie.cammas@francetelecom.com)
  UInt                              getSimplePriorityId			  ()	    { return m_uiSimplePriorityId;}
  Bool                              getDiscardableFlag			  ()		{ return m_bDiscardableFlag;}
  Bool                              getExtensionFlag              ()    	{ return m_bExtensionFlag;}
  //}}Variable Lengh NAL unit header data with priority and dead substream flag

  //JVT-P031
  Bool                              getFragmentedFlag              ()       {return m_bFragmentedFlag;}
  UInt                              getFragmentOrder               ()       {return m_uiFragmentOrder;}
  Bool                              getLastFragmentFlag            ()       {return m_bLastFragmentFlag;}
  Void                              setFragmentedFlag              (Bool b)    {m_bFragmentedFlag = b;}
  Void                              setFragmentOrder               (UInt ui)   {m_uiFragmentOrder = ui;}
  Void                              setLastFragmentFlag            (Bool b)    {m_bLastFragmentFlag = b;}
  //~JVT-P031
  Bool                              getBaseLayerUsesConstrainedIntraPred() const { return m_bBaseLayerUsesConstrainedIntraPred; }
  UInt                              getRedundantPicCnt             ()       { return m_uiRedundantPicCnt; } // JVT-Q054 Red. Picture

  //===== set parameters =====
  Void  setNalRefIdc                  ( NalRefIdc   e  )  { m_eNalRefIdc                        = e;  }
  Void  setNalUnitType                ( NalUnitType e  )  { m_eNalUnitType                      = e;  }
  Void  setLayerId                    ( UInt        ui )  { m_uiLayerId                         = ui; }
  Void  setTemporalLevel              ( UInt        ui )  { m_uiTemporalLevel                   = ui; }
  Void  setQualityLevel               ( UInt        ui )  { m_uiQualityLevel                    = ui; }
  Void  setFirstMbInSlice             ( UInt        ui )  { m_uiFirstMbInSlice                  = ui; }
  Void  setSliceType                  ( SliceType   e  )  { m_eSliceType                        = e;  }
  Void  setPicParameterSetId          ( UInt        ui )  { m_uiPicParameterSetId               = ui; }
  Void  setFrameNum                   ( UInt        ui )  { m_uiFrameNum                        = ui; }
  Void  setNumMbsInSlice              ( UInt        ui )  { m_uiNumMbsInSlice                   = ui; }
  Void  setFgsComponentSep            ( Bool        b  )  { m_bFgsComponentSep                  = b;  }
  Void  setIdrPicId                   ( UInt        ui )  { m_uiIdrPicId                        = ui; }
  Void  setPicOrderCntLsb             ( UInt        ui )  { m_uiPicOrderCntLsb                  = ui; }
  Void  setDeltaPicOrderCntBottom     ( Int         i  )  { m_iDeltaPicOrderCntBottom           = i;  }
  Void  setDeltaPicOrderCnt           ( UInt        ui,
                                        Int         i  )  { m_aiDeltaPicOrderCnt[ui]            = i;  }
  Void  setBasePredWeightTableFlag    ( Bool        b  )  { m_bBasePredWeightTableFlag          = b;  }
  Void  setLumaLog2WeightDenom        ( UInt        ui )  { m_uiLumaLog2WeightDenom             = ui; }
  Void  setChromaLog2WeightDenom      ( UInt        ui )  { m_uiChromaLog2WeightDenom           = ui; }
  Void  setDirectSpatialMvPredFlag    ( Bool        b  )  { m_bDirectSpatialMvPredFlag          = b;  }
  Void  setKeyPictureFlag             ( Bool        b  )  { m_bKeyPictureFlag                   = b;  }
  Void  setBaseLayerId                ( UInt        ui )  { m_uiBaseLayerId                     = ui; }
  Void  setBaseQualityLevel           ( UInt        ui )  { m_uiBaseQualityLevel                = ui; }
  Void  setAdaptivePredictionFlag     ( Bool        b  )  { m_bAdaptivePredictionFlag           = b;  }
  Void  setNumRefIdxActiveOverrideFlag( Bool        b  )  { m_bNumRefIdxActiveOverrideFlag      = b;  }
  Void  setNumRefIdxActive            ( ListIdx     e,
                                        UInt        ui )  { m_auiNumRefIdxActive[e]             = ui; }
  Void  setNumRefIdxUpdate            ( UInt        ui,
                                        ListIdx     e,
                                        UInt        p  )  { m_aauiNumRefIdxActiveUpdate[ui][e]  = p;  }
  Void  setNoOutputOfPriorPicsFlag    ( Bool        b  )  { m_bNoOutputOfPriorPicsFlag          = b;  }
  Void  setAdaptiveRefPicBufferingFlag( Bool        b  )  { m_bAdaptiveRefPicBufferingModeFlag  = b;  }
  Void  setCabacInitIdc               ( UInt        ui )  { m_uiCabacInitIdc                    = ui; }
  Void  setSliceQpDelta               ( Int         i  )  { m_iSliceQpDelta                     = i;  }
  Void  setSliceHeaderQp              ( Int         i  )  { setSliceQpDelta( i - m_rcPPS.getPicInitQp() );  }
  
  //{{Variable Lengh NAL unit header data with priority and dead substream flag
  //France Telecom R&D- (nathalie.cammas@francetelecom.com)
  Void setSimplePriorityId			  (UInt			ui)	   { m_uiSimplePriorityId = ui;}
  Void setDiscardableFlag			  (Bool			b)	   { m_bDiscardableFlag = b;}
  Void setExtensionFlag				  (Bool         b)	   { m_bExtensionFlag = b;}
  //}}Variable Lengh NAL unit header data with priority and dead substream flag

  Void setBaseLayerUsesConstrainedIntraPred( Bool b ) { m_bBaseLayerUsesConstrainedIntraPred = b; }

  Void  setSliceGroupChangeCycle(UInt uiSliceGroupChangeCycle){m_uiSliceGroupChangeCycle = uiSliceGroupChangeCycle;};
  ErrVal FMOInit();
//	TMM_EC {{
	Bool	getTrueSlice()	const	{ return	m_bTrueSlice;}
	Void	setTrueSlice( Bool bTrueSlice)	{ m_bTrueSlice = bTrueSlice;}
	ERROR_CONCEAL	m_eErrorConceal;
	Bool	m_bTrueSlice;
//  TMM_EC }}
  Void  setRedundantPicCnt            (UInt         ui )  { m_uiRedundantPicCnt                 = ui; }  // JVT-Q054 Red. Picture
  
protected:
  ErrVal xReadH264AVCCompatible       ( HeaderSymbolReadIf*   pcReadIf );
  ErrVal xReadScalable                ( HeaderSymbolReadIf*   pcReadIf );
  ErrVal xWriteScalable               ( HeaderSymbolWriteIf*  pcWriteIf ) const;
  ErrVal xWriteH264AVCCompatible      ( HeaderSymbolWriteIf*  pcWriteIf ) const;

  
protected:
  const PictureParameterSet&  m_rcPPS;
  const SequenceParameterSet& m_rcSPS;

  NalRefIdc                   m_eNalRefIdc;
  NalUnitType                 m_eNalUnitType;
  UInt                        m_uiLayerId;
  UInt                        m_uiTemporalLevel;
  UInt                        m_uiQualityLevel;
  UInt                        m_uiFirstMbInSlice;
  SliceType                   m_eSliceType;
  UInt                        m_uiPicParameterSetId;
  UInt                        m_uiFrameNum;
  UInt                        m_uiNumMbsInSlice;
  Bool                        m_bFgsComponentSep;
  UInt                        m_uiIdrPicId;
  UInt                        m_uiPicOrderCntLsb;
  Int                         m_iDeltaPicOrderCntBottom;
  Int                         m_aiDeltaPicOrderCnt[2];
  Bool                        m_bBasePredWeightTableFlag;
  UInt                        m_uiLumaLog2WeightDenom;
  UInt                        m_uiChromaLog2WeightDenom;
  PredWeightTable             m_acPredWeightTable[2];

  Bool                        m_bDirectSpatialMvPredFlag;

  Bool                        m_bKeyPictureFlag;
  UInt                        m_uiBaseLayerId;
  UInt                        m_uiBaseQualityLevel;
  UInt						            m_uiBaseFragmentOrder;
  Bool                        m_bAdaptivePredictionFlag;
  Bool                        m_bNumRefIdxActiveOverrideFlag;
  UInt                        m_auiNumRefIdxActive[2];
  RplrBuffer                  m_acRplrBuffer      [2];
  UInt                        m_aauiNumRefIdxActiveUpdate[MAX_TEMP_LEVELS][2];
  Bool                        m_bNoOutputOfPriorPicsFlag;
  Bool                        m_bAdaptiveRefPicBufferingModeFlag;
  MmcoBuffer                  m_cMmmcoBuffer;
  UInt                        m_uiCabacInitIdc;
  Int                         m_iSliceQpDelta;
  DeblockingFilterParameter   m_cDeblockingFilterParameter;

  UInt                        m_uiSliceGroupChangeCycle;
  FMO*                        m_pcFMO;

  Bool                        m_bBaseLayerUsesConstrainedIntraPred;

  //{{Variable Lengh NAL unit header data with priority and dead substream flag
  //France Telecom R&D- (nathalie.cammas@francetelecom.com)
  UInt						            m_uiSimplePriorityId;
  Bool					              m_bDiscardableFlag;
  Bool                        m_bExtensionFlag;
  //}}Variable Lengh NAL unit header data with priority and dead substream flag
  
  //JVT-P031
  Bool                        m_bFragmentedFlag;
  UInt                        m_uiFragmentOrder;
  Bool                        m_bLastFragmentFlag;
  //~JVT-P031
  UInt                        m_uiRedundantPicCnt;  // JVT-Q054 Red. Picture

// TMM_ESS {
public:
  Int           getLeftOffset ()   const { return m_iScaledBaseLeftOffset; }
  Int           getRightOffset ()  const { return m_iScaledBaseRightOffset; }
  Int           getTopOffset ()    const { return m_iScaledBaseTopOffset; }
  Int           getBottomOffset () const { return m_iScaledBaseBottomOffset; }
  Int           getBaseChromaPhaseX () const { return (Int)m_uiBaseChromaPhaseXPlus1-1; }
  Int           getBaseChromaPhaseY () const { return (Int)m_uiBaseChromaPhaseYPlus1-1; }
  Void          setLeftOffset   ( Int i ) { m_iScaledBaseLeftOffset = i; }
  Void          setRightOffset  ( Int i ) { m_iScaledBaseRightOffset = i; }
  Void          setTopOffset    ( Int i ) { m_iScaledBaseTopOffset = i; }
  Void          setBottomOffset ( Int i ) { m_iScaledBaseBottomOffset = i; }
  Void          setBaseChromaPhaseX ( Int i)  { m_uiBaseChromaPhaseXPlus1 = i+1; }
  Void          setBaseChromaPhaseY ( Int i)  { m_uiBaseChromaPhaseYPlus1 = i+1; }
  Void          Print               ( );

protected:
  UInt          m_uiBaseChromaPhaseXPlus1;
  UInt          m_uiBaseChromaPhaseYPlus1;

  Int           m_iScaledBaseLeftOffset;
  Int           m_iScaledBaseTopOffset;
  Int           m_iScaledBaseRightOffset;
  Int           m_iScaledBaseBottomOffset;
// TMM_ESS }

  Bool          m_bArFgsUsageFlag;
  Bool          m_bFgsEntropyOrderFlag;
  UInt          m_uiLowPassFgsMcFilter;
  UInt          m_uiBaseWeightZeroBaseBlock;
  UInt          m_uiBaseWeightZeroBaseCoeff;
};

#define IS_KEY_PICTURE(pcSH)    ( (pcSH)->getKeyPictureFlag() )


#if defined( WIN32 )
# pragma warning( default: 4251 )
# pragma warning( default: 4275 )
#endif

typedef SliceHeaderBase::PredWeight PW;

H264AVC_NAMESPACE_END



#endif // !defined(AFX_SLICEHEADERBASE_H__2CC1FD0F_CACB_4799_84BE_FC5FC9B9C245__INCLUDED_)
