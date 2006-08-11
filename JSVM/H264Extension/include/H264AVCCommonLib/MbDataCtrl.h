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





#if !defined(AFX_MBDATACTRL_H__50D2B462_28AB_46CA_86AC_35502BD296BC__INCLUDED_)
#define AFX_MBDATACTRL_H__50D2B462_28AB_46CA_86AC_35502BD296BC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


H264AVC_NAMESPACE_BEGIN

#if defined( WIN32 )
# pragma warning( disable: 4251 )
#endif








class H264AVCCOMMONLIB_API MbDataCtrl
{
public:
	MbDataCtrl();
  ~MbDataCtrl();

public:
  ErrVal getBoundaryMask( Int iMbY, Int iMbX, UInt& ruiMask ) const ;
  ErrVal initMb( MbDataAccess*& rpcMbDataAccess, UInt uiMbY, UInt uiMbX, const Int iForceQp = -1 );
  ErrVal init( const SequenceParameterSet& rcSPS );
//	TMM_EC {{
  ErrVal initMbTDEnhance( MbDataAccess*& rpcMbDataAccess, MbDataCtrl *pcMbDataCtrl, MbDataCtrl *pcMbDataCtrlRef, UInt uiMbY, UInt uiMbX, const Int iForceQp = -1 );
//  TMM_EC }}

  ErrVal uninit();
  ErrVal reset();
  ErrVal initSlice( SliceHeader& rcSH, ProcessingState eProcessingState, Bool bDecoder, MbDataCtrl* pcMbDataCtrl );

  Bool isPicDone( const SliceHeader& rcSH );
  Bool isFrameDone( const SliceHeader& rcSH );
  UInt  getSize() { return m_uiSize; }

  const MbData& getMbData( UInt uiIndex )   const { AOT_DBG( uiIndex >= m_uiSize );  return m_pcMbData[ uiIndex ]; }
  const Bool isPicCodedField( )              const { return m_bPicCodedField; }

  MbData& getMbDataByIndex( UInt uiIndex )        { AOT_DBG( uiIndex >= m_uiSize );  return m_pcMbData[ uiIndex ]; }

  ErrVal saveQpAndCbp( MbDataCtrl *pSrcMbDataCtrl )
  {
    for( UInt uiMbIdx = 0; uiMbIdx < getSize(); uiMbIdx++ )
    {
      getMbDataByIndex( uiMbIdx ).setQp      ( pSrcMbDataCtrl->getMbData( uiMbIdx ).getQp() );
      getMbDataByIndex( uiMbIdx ).setTransformSize8x8 ( pSrcMbDataCtrl->getMbData( uiMbIdx ).isTransformSize8x8() );
      getMbDataByIndex( uiMbIdx ).setMbExtCbp( pSrcMbDataCtrl->getMbData( uiMbIdx ).getMbExtCbp() );
    }

    return Err::m_nOK;
  }

  ErrVal clear() { return xResetData(); }

  MbData& getMbData( UInt uiMbX, UInt uiMbY )   { AOT_DBG( uiMbY*m_uiMbStride+uiMbX+m_uiMbOffset >= m_uiSize );  return m_pcMbData[uiMbY*m_uiMbStride+uiMbX+m_uiMbOffset]; }
//	TMM_EC {{
	MbData& getMbData( UInt uiMbX, UInt uiMbY ) const  { AOT_DBG( uiMbY*m_uiMbStride+uiMbX+m_uiMbOffset >= m_uiSize );  return m_pcMbData[uiMbY*m_uiMbStride+uiMbX+m_uiMbOffset]; }
//  TMM_EC }}

  ErrVal        switchMotionRefinement();

  ErrVal        copyMotion    ( MbDataCtrl& rcMbDataCtrl );
	// TMM_ESS {
  ErrVal        copyMotionBL  ( MbDataCtrl& rcMbDataCtrl, ResizeParameters* pcParameters  );
  ErrVal        upsampleMotion ( MbDataCtrl& rcBaseMbDataCtrl, ResizeParameters* pcParameters );
	// TMM_ESS }
//JVT-T054{
  ErrVal        copyMbCBP( MbDataCtrl& rcBaseMbDataCtrl, ResizeParameters* pcParameters );
  ErrVal        initMbCBP( MbDataCtrl& rcBaseMbDataCtrl, ResizeParameters* pcParameters );
//JVT-T054}
  //--ICU/ETRI FMO Implementation
  const Int getSliceGroupIDofMb(Int mb);

  ErrVal        initFgsBQData             ( UInt uiNumMb );
  ErrVal        uninitFgsBQData           ();
  ErrVal        storeFgsBQLayerQpAndCbp   ();
  ErrVal        switchFgsBQLayerQpAndCbp  ();

//TMM_WP
  SliceHeader*  getSliceHeader      ()  { return  m_pcSliceHeader;      }
//TMM_WP

  // ICU/ETRI FGS_MOT_USE
	Bool xGetDirect8x8InferenceFlagPublic() { return m_bDirect8x8InferenceFlag; }
  Void xSetDirect8x8InferenceFlag(Bool b) { m_bDirect8x8InferenceFlag = b; }

  UInt GetMbStride() { return m_uiMbStride; }
  Void SetMbStride(UInt Stride) { m_uiMbStride = Stride; }  

  // JVT-S054 (ADD)
  UInt getSliceId() const { return m_uiSliceId;}

protected:
  const MbData& xGetOutMbData()            const { return m_pcMbData[m_uiSize]; }
  const MbData& xGetRefMbData( UInt uiSliceId, Int uiCurrSliceID, Int iMbY, Int iMbX, Bool bLoopFilter ); 
  const MbData& xGetColMbData( UInt uiIndex );

  ErrVal xCreateData( UInt uiSize );
  ErrVal xDeleteData();
  ErrVal xResetData();

  Bool xGetDirect8x8InferenceFlag() { return m_bDirect8x8InferenceFlag; }

// TMM_ESS_UNIFIED {
  ErrVal  xUpsampleMotionDyad(MbDataCtrl& rcBaseMbDataCtrl, ResizeParameters* pcParameters );
  ErrVal  xUpsampleMotionESS(MbDataCtrl& rcBaseMbDataCtrl, ResizeParameters* pcParameters );
// TMM_ESS_UNIFIED }

protected:
  DynBuf<DFP*>        m_cpDFPBuffer;
  MbTransformCoeffs*  m_pcMbTCoeffs;
  MbMotionData*       m_apcMbMotionData[2];
  MbMotionData*       m_apcMbMotionDataBase[2];
  MbMvData*           m_apcMbMvdData[2];
  MbData*             m_pcMbData;
  MbDataAccess*       m_pcMbDataAccess;
  SliceHeader*        m_pcSliceHeader;
  UChar               m_ucLastMbQp;
  UInt                m_uiMbStride;
  UInt                m_uiMbOffset;
  Int                 m_iMbPerLine;
  Int                 m_iMbPerColumn;
  UInt                m_uiSize;
  UInt                m_uiMbProcessed;
  UInt                m_uiSliceId;
  ProcessingState     m_eProcessingState;
  const MbDataCtrl*   m_pcMbDataCtrl0L1;
  Bool                m_bUseTopField;
  Bool                m_bPicCodedField;
  Bool                m_bInitDone;
  Bool                m_bDirect8x8InferenceFlag;
  UChar*        m_pacFgsBQMbQP;
  UInt*         m_pauiFgsBQMbCbp;
  UInt*         m_pauiFgsBQBCBP;
  Bool*         m_pabFgsBQ8x8Trafo;

};



class ControlData
{
public:
  ControlData   ();
  ~ControlData  ();

  Void          clear               ();
  ErrVal        init                ( SliceHeader*  pcSliceHeader,
                                      MbDataCtrl*   pcMbDataCtrl,
                                      Double        dLambda );
  ErrVal        init                ( SliceHeader*  pcSliceHeader );

  Double        getLambda           ()  { return  m_dLambda;            }
  SliceHeader*  getSliceHeader      ()  { return  m_pcSliceHeader;      }
  MbDataCtrl*   getMbDataCtrl       ()  { return  m_pcMbDataCtrl;       }
  Bool          isInitialized       ()  { return  m_pcMbDataCtrl != 0;  }

  ErrVal        setMbDataCtrl       ( MbDataCtrl* pcMbDataCtrl )
  {
    m_pcMbDataCtrl = pcMbDataCtrl;
    return Err::m_nOK;
  }
  ErrVal        setSliceHeader      ( SliceHeader* pcSliceHeader )
  {
    m_pcSliceHeader = pcSliceHeader;
    return Err::m_nOK;
  }

  ErrVal        activateMbDataCtrlForQpAndCbp( Bool bNormalMbDataCtrl )
  {
    // restore the state first
    if( m_bIsNormalMbDataCtrl != bNormalMbDataCtrl )
    {
      switchFGSLayerQpAndCbp();
    }
    m_bIsNormalMbDataCtrl = bNormalMbDataCtrl;

    return Err::m_nOK;
  }

  ErrVal        saveMbDataQpAndCbp()
  {
    // should not happen, not designed so general
    ROT( ! m_bIsNormalMbDataCtrl );
    storeFGSLayerQpAndCbp();
    return Err::m_nOK;
  }

  IntFrame*     getBaseLayerRec     ()  { return  m_pcBaseLayerRec;     }
  IntFrame*     getBaseLayerSbb     ()  { return  m_pcBaseLayerSbb;     }
  MbDataCtrl*   getBaseLayerCtrl    ()  { return  m_pcBaseLayerCtrl;    }
  ControlData*  getBaseCtrlData     ()  { return  m_pcBaseCtrlData;     }
  UInt          getUseBLMotion      ()  { return  m_uiUseBLMotion;      }
  
  Void          setBaseLayerRec     ( IntFrame*   pcBaseLayerRec  )   { m_pcBaseLayerRec    = pcBaseLayerRec;   }
  Void          setBaseLayerSbb     ( IntFrame*   pcBaseLayerSbb  )   { m_pcBaseLayerSbb    = pcBaseLayerSbb;   }
  Void          setBaseLayerCtrl    ( MbDataCtrl* pcBaseLayerCtrl )   { m_pcBaseLayerCtrl   = pcBaseLayerCtrl;  }
  Void          setBaseCtrlData     ( ControlData*pcBaseCtrlData  )   { m_pcBaseCtrlData    = pcBaseCtrlData;   }
  Void          setUseBLMotion      ( UInt        uiUseBLMotion   )   { m_uiUseBLMotion     = uiUseBLMotion;    }

  Void          setLambda           ( Double d ) { m_dLambda = d; }

  Void          setScalingFactor    ( Double  d ) { m_dScalingFactor      = d; }
  Double        getScalingFactor    ()  const     { return m_dScalingFactor;      }

  Void          setBaseLayer        ( UInt  uiBaseLayerId, UInt  uiBaseLayerIdMotion )
  {
    m_uiBaseLayerId = uiBaseLayerId; m_uiBaseLayerIdMotion = uiBaseLayerIdMotion; 
  }

  UInt          getBaseLayerId    () { return m_uiBaseLayerId; }
  UInt          getBaseLayerIdMotion()  { return m_uiBaseLayerIdMotion; }

// TMM_ESS {
  Void          setSpatialScalabilityType ( int iSST )   { m_iSpatialScalabilityType = iSST; }
  Int           getSpatialScalabilityType ()             { return m_iSpatialScalabilityType; }
  Void          setSpatialScalability ( Bool b )         { m_bSpatialScalability = b; }
  Bool          getSpatialScalability ()                 { return m_bSpatialScalability; }
// TMM_ESS }


  RefFrameList& getPrdFrameList     ( UInt uiList )   { return m_acPrdFrameList          [uiList]; }
  
  ErrVal        initFGSData             ( UInt uiNumMb );
  ErrVal        uninitFGSData           ();
  ErrVal        storeFGSLayerQpAndCbp   ();
  ErrVal        switchFGSLayerQpAndCbp  ();

  ErrVal        initBQData            ( UInt uiNumMb );
  ErrVal        uninitBQData          ();
  ErrVal        storeBQLayerQpAndCbp  ();
  ErrVal        switchBQLayerQpAndCbp ();

private:
  MbDataCtrl*   m_pcMbDataCtrl;
  SliceHeader*  m_pcSliceHeader;

  Double        m_dLambda;

  IntFrame*     m_pcBaseLayerRec;
  IntFrame*     m_pcBaseLayerSbb;
  MbDataCtrl*   m_pcBaseLayerCtrl;
  ControlData*  m_pcBaseCtrlData;
  UInt          m_uiUseBLMotion;

  Double        m_dScalingFactor;

  UInt          m_uiBaseLayerId;
  UInt          m_uiBaseLayerIdMotion;

  Int           m_iSpatialScalabilityType; // TMM_ESS
  Bool          m_bSpatialScalability;     // TMM_ESS  

  
  UChar*        m_pacFGSMbQP;
  UInt*         m_pauiFGSMbCbp;
  Bool*         m_pabFGS8x8Trafo;
  Bool          m_bIsNormalMbDataCtrl;

  //===== base quality data (CBP, QP, 8x8flag) =====
  UChar*        m_pacBQMbQP;
  UInt*         m_pauiBQMbCbp;
  Bool*         m_pabBQ8x8Trafo;
  MbMode*       m_paeBQMbMode;
  UShort*       m_pusBQFwdBwd;
  MbMotionData* m_paacBQMotionData[2];

  RefFrameList  m_acPrdFrameList[2];

};






#if defined( WIN32 )
# pragma warning( default: 4251 )
#endif



H264AVC_NAMESPACE_END


#endif // !defined(AFX_MBDATACTRL_H__50D2B462_28AB_46CA_86AC_35502BD296BC__INCLUDED_)
