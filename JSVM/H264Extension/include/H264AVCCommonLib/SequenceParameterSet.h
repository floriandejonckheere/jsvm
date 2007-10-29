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




#if !defined(AFX_SEQUENCEPARAMETERSET_H__66281283_5BFB_429A_B722_6DDE7A11D086__INCLUDED_)
#define AFX_SEQUENCEPARAMETERSET_H__66281283_5BFB_429A_B722_6DDE7A11D086__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "H264AVCCommonLib/HeaderSymbolReadIf.h"
#include "H264AVCCommonLib/HeaderSymbolWriteIf.h"
#include "H264AVCCommonLib/ScalingMatrix.h"

// TMM_ESS 
#include "ResizeParameters.h"

// JVT-V068 HRD {
#include "H264AVCCommonLib/Vui.h"
// JVT-V068 HRD }

H264AVC_NAMESPACE_BEGIN

// JVT-V068 HRD {
class CodingParameter;
// JVT-V068 HRD }

class H264AVCCOMMONLIB_API SequenceParameterSet
{
protected:
  typedef struct
  {
    Bool bValid;
    UInt uiMaxMbPerSec;
    UInt uiMaxFrameSize;
    UInt uiMaxDPBSizeX2;
    UInt uiMaxBitRate;
    UInt uiMaxCPBSize;
    UInt uiMaxVMvRange;
    UInt uiMinComprRatio;
    UInt uiMaxMvsPer2Mb;
  } LevelLimit;

  SequenceParameterSet          ();
  virtual ~SequenceParameterSet ();

public:
  static ErrVal create                    ( SequenceParameterSet*& rpcSPS );
  ErrVal        destroy                   ();

	SequenceParameterSet& operator = ( const SequenceParameterSet& rcSPS );

  static UInt   getLevelIdc               ( UInt uiMbY, UInt uiMbX, UInt uiOutFreq, UInt uiMvRange, UInt uiNumRefPic );
  UInt          getMaxDPBSize             () const;

  static Short  getMaxIntMvVer            ( UInt uiLevelIdc, Bool bField ) { return m_aLevelLimit[uiLevelIdc].uiMaxVMvRange / (bField?2:1); }
  static Short  getMaxIntMvHor            () { return 8192; }

  NalUnitType           getNalUnitType                        ()          const { return m_eNalUnitType; }
  UInt                  getLayerId                            ()          const { return m_uiLayerId; }
  Profile               getProfileIdc                         ()          const { return m_eProfileIdc;}
  Bool                  getConstrainedSet0Flag                ()          const { return m_bConstrainedSet0Flag; }
  Bool                  getConstrainedSet1Flag                ()          const { return m_bConstrainedSet1Flag; }
  Bool                  getConstrainedSet2Flag                ()          const { return m_bConstrainedSet2Flag; }
  Bool                  getConstrainedSet3Flag                ()          const { return m_bConstrainedSet3Flag; }
  UInt                  getLevelIdc                           ()          const { return m_uiLevelIdc;}
  UInt                  getSeqParameterSetId                  ()          const { return m_uiSeqParameterSetId;}
  Bool                  getSeqScalingMatrixPresentFlag        ()          const { return m_bSeqScalingMatrixPresentFlag; }
  const ScalingMatrix&  getSeqScalingMatrix                   ()          const { return m_cSeqScalingMatrix; }
  UInt                  getLog2MaxFrameNum                    ()          const { return m_uiLog2MaxFrameNum;}
	UInt                  getPicOrderCntType                    ()          const { return m_uiPicOrderCntType;}
  UInt                  getLog2MaxPicOrderCntLsb              ()          const { return m_uiLog2MaxPicOrderCntLsb;}
  Bool                  getDeltaPicOrderAlwaysZeroFlag        ()          const { return m_bDeltaPicOrderAlwaysZeroFlag;}
  Int                   getOffsetForNonRefPic                 ()          const { return m_iOffsetForNonRefPic;}
  Int                   getOffsetForTopToBottomField          ()          const { return m_iOffsetForTopToBottomField;}
  UInt                  getNumRefFramesInPicOrderCntCycle     ()          const { return m_uiNumRefFramesInPicOrderCntCycle; }
  Int                   getOffsetForRefFrame                  ( UInt ui ) const { return m_piOffsetForRefFrame[ui]; }
  UInt                  getNumRefFrames                       ()          const { return m_uiNumRefFrames;}
  Bool                  getGapsInFrameNumValueAllowedFlag     ()          const { return m_bGapsInFrameNumValueAllowedFlag;}
  UInt                  getFrameWidthInMbs                    ()          const { return m_uiFrameWidthInMbs;}
  UInt                  getFrameHeightInMbs                   ()          const { return m_uiFrameHeightInMbs;}
  Bool                  getDirect8x8InferenceFlag             ()          const { return m_bDirect8x8InferenceFlag;}
  UInt                  getMbInFrame                          ()          const { return m_uiFrameWidthInMbs * m_uiFrameHeightInMbs;}
  Bool                  getInitState                          ()          const { return m_bInitDone; }
 
	Bool                  getFrameMbsOnlyFlag()                             const { return m_bFrameMbsOnlyFlag; }
	Bool                  getMbAdaptiveFrameFieldFlag()                     const { return m_bMbAdaptiveFrameFieldFlag; }

  Void  setNalUnitType                        ( NalUnitType e )           { m_eNalUnitType                          = e;  }
  Void  setLayerId                            ( UInt        ui )          { m_uiLayerId                             = ui; }
  Void  setProfileIdc                         ( Profile     e  )          { m_eProfileIdc                           = e;  }
  Void  setConstrainedSet0Flag                ( Bool        b  )          { m_bConstrainedSet0Flag                  = b;  }
  Void  setConstrainedSet1Flag                ( Bool        b  )          { m_bConstrainedSet1Flag                  = b;  }
  Void  setConstrainedSet2Flag                ( Bool        b  )          { m_bConstrainedSet2Flag                  = b;  }
  Void  setConstrainedSet3Flag                ( Bool        b  )          { m_bConstrainedSet3Flag                  = b;  }
  Void  setLevelIdc                           ( UInt        ui )          { m_uiLevelIdc                            = ui; }
  Void  setSeqParameterSetId                  ( UInt        ui )          { m_uiSeqParameterSetId                   = ui; }
  Void  setSeqScalingMatrixPresentFlag        ( Bool        b  )          { m_bSeqScalingMatrixPresentFlag          = b;  }
  Void  setLog2MaxFrameNum                    ( UInt        ui )          { m_uiLog2MaxFrameNum                     = ui; }
  Void  setPicOrderCntType                    ( UInt        ui )          { m_uiPicOrderCntType                     = ui; }
  Void  setLog2MaxPicOrderCntLsb              ( UInt        ui )          { m_uiLog2MaxPicOrderCntLsb               = ui; }
  Void  setDeltaPicOrderAlwaysZeroFlag        ( Bool        b  )          { m_bDeltaPicOrderAlwaysZeroFlag          = b;  }
  Void  setOffsetForNonRefPic                 ( Int         i  )          { m_iOffsetForNonRefPic                   = i;  }
  Void  setOffsetForTopToBottomField          ( Int         i  )          { m_iOffsetForTopToBottomField            = i;  }
  Void  setNumRefFramesInPicOrderCntCycle     ( UInt        ui )          { m_uiNumRefFramesInPicOrderCntCycle      = ui; }
  Void  setOffsetForRefFrame                  ( UInt        ui, 
		                                            Int         i  )          { m_piOffsetForRefFrame[ui]               = i;  }
  Void  setNumRefFrames                       ( UInt        ui )          { m_uiNumRefFrames                        = ui; }
  Void  setGapsInFrameNumValueAllowedFlag     ( Bool        b  )          { m_bGapsInFrameNumValueAllowedFlag       = b;  }
  Void  setFrameWidthInMbs                    ( UInt        ui )          { m_uiFrameWidthInMbs                     = ui; }
  Void  setFrameHeightInMbs                   ( UInt        ui )          { m_uiFrameHeightInMbs                    = ui; }
  Void  setDirect8x8InferenceFlag             ( Bool        b  )          { m_bDirect8x8InferenceFlag               = b;  }
  Void  setInitState                          ( Bool        b  )          { m_bInitDone                             = b;  }

  Void setMGSVect                             ( UInt ui, UInt uiVect )    { m_uiMGSVect[ui] = uiVect; }
  UInt getMGSCoeffStart                       ( UInt uiNum )        const { return uiNum ? getMGSCoeffStart( uiNum - 1 ) + m_uiMGSVect[uiNum - 1] : 0; }
  UInt getMGSCoeffStop                        ( UInt uiNum )        const { return         getMGSCoeffStart( uiNum + 1 );                              }
  UInt getNumberOfQualityLevelsCGSSNR         () const
  {
    UInt uiQLs = 0;
    for( ; getMGSCoeffStop( uiQLs ) != 16; uiQLs++ ) {}
    return uiQLs + 1;
  }

  Void setInterlayerDeblockingPresent ( Bool b ) { m_bInterlayerDeblockingPresent = b ;}
  Bool getInterlayerDeblockingPresent () const    { return m_bInterlayerDeblockingPresent; }

  Void  setFrameMbsOnlyFlag                   ( Bool        b  )          { m_bFrameMbsOnlyFlag                     = b;  }
	Void  setMbAdaptiveFrameFieldFlag           ( Bool        b  )          { m_bMbAdaptiveFrameFieldFlag             = b;  }
  
  ErrVal initOffsetForRefFrame( UInt uiSize )
  {
    ROT ( uiSize<1 );

    RNOK( m_piOffsetForRefFrame.uninit() );
    RNOK( m_piOffsetForRefFrame.init( uiSize ) );

    return Err::m_nOK;
  }

  ErrVal write( HeaderSymbolWriteIf*  pcWriteIf )       const;
  ErrVal read ( HeaderSymbolReadIf*   pcReadIf,
                NalUnitType           eNalUnitType );
//SSPS {
	ErrVal writeSubSPS( HeaderSymbolWriteIf*  pcWriteIf )       const;
  ErrVal readSubSPS ( HeaderSymbolReadIf*   pcReadIf,
                NalUnitType           eNalUnitType );
//SSPS }
// TMM_ESS {
  Void setResizeParameters    ( const ResizeParameters * params );
  Void getResizeParameters    ( ResizeParameters * params ) const;

  Void setExtendedSpatialScalability ( UInt ui ) { m_uiExtendedSpatialScalability = ui ;}
  UInt getExtendedSpatialScalability () const    { return m_uiExtendedSpatialScalability; }
  //JVT-W046 {
  Void  setChromaFormatIdc ( UInt ui ) { m_uiChromaFormatIdc = ui ;  }
  UInt  getChromaFormatIdc () const    { return m_uiChromaFormatIdc; }
  Bool  getAVCHeaderRewriteFlag ()                const { return m_bAVCHeaderRewriteFlag; }
  Void  setAVCHeaderRewriteFlag ( Bool b )        { m_bAVCHeaderRewriteFlag = b; }
  Void  setBaseChromaPhaseXPlus1 ( UInt ui)       { m_uiBaseChromaPhaseXPlus1 = ui; }
  Void  setBaseChromaPhaseYPlus1 ( UInt ui)       { m_uiBaseChromaPhaseYPlus1 = ui; }
  UInt  getBaseChromaPhaseXPlus1 () const         { return m_uiBaseChromaPhaseXPlus1 ; }
  UInt  getBaseChromaPhaseYPlus1 () const         { return m_uiBaseChromaPhaseYPlus1 ; }
  //JVT-W046 }
  // JVT-V035
  Bool getAVCRewriteFlag ()                       const { return m_bAVCRewriteFlag; }
  Bool getAVCAdaptiveRewriteFlag ()               const { return m_bAVCAdaptiveRewriteFlag; }
  Void setAVCRewriteFlag( Bool b )                { m_bAVCRewriteFlag = b; }
  Void setAVCAdaptiveRewriteFlag ( Bool b )
  { 
	  if( getAVCRewriteFlag() == false && b == true )
		  printf("WARNING: Setting AVCAdaptiveRewriteFlag when AVCRewriteFlag is false.\n");
	  m_bAVCAdaptiveRewriteFlag = b; 
  }

  // TMM_ESS }

 // Bool  getRCDOBlockSizes         () const { return m_bRCDOBlockSizes; }
 // Bool  getRCDOMotionCompensationY() const { return m_bRCDOMotionCompensationY; }
 // Bool  getRCDOMotionCompensationC() const { return m_bRCDOMotionCompensationC; }
 // Bool  getRCDODeblocking         () const { return m_bRCDODeblocking; }

 // Void  setRCDOBlockSizes         ( Bool b ) { m_bRCDOBlockSizes          = b; }
 // Void  setRCDOMotionCompensationY( Bool b ) { m_bRCDOMotionCompensationY = b; }
 // Void  setRCDOMotionCompensationC( Bool b ) { m_bRCDOMotionCompensationC = b; }
 // Void  setRCDODeblocking         ( Bool b ) { m_bRCDODeblocking          = b; }

  Void  set4TapMotionCompensationY( Bool b ) {m_b4TapMotionCompensationY = b; } // V090
  Bool  get4TapMotionCompensationY() const { return m_b4TapMotionCompensationY; }  // V090

  // JVT-V068 HRD {
  VUI*  getVUI                     () const { return m_pcVUI; }
  Void  setVUI                     ( VUI* pcVUI )              { delete m_pcVUI; m_pcVUI = pcVUI; } 
  Void  setVUI                     ( SequenceParameterSet* pcSPS );
  UInt          getMaxCPBSize() const;
  UInt          getMaxBitRate() const;
  // JVT-V068 HRD } 

//SSPS {
	ErrVal writeSVCVUIParametersExtension ( HeaderSymbolWriteIf*  pcWriteIf )  const;
	ErrVal ReadSVCVUIParametersExtension ( HeaderSymbolReadIf*   pcReadIf );
  Bool getSVCVUIParametersPresentFlag()            const { return m_bSVCVUIParametersPresentFlag;      }
	Bool getAdditionalExtension2Flag()               const { return m_bAdditionalExtension2Flag;         }
  Bool getAdditionalExtension2DataFlag()           const { return m_bAdditionalExtension2DataFlag;     }
	Bool getSubSPS()                                 const { return m_bSubSPS;                           }
	Bool getFixedFrameRateFlag( UInt ui )            const { return m_bFixedFrameRateFlag[ui];           }
  Bool getNalHrdParametersPresentFlag( UInt ui )   const { return m_bNalHrdParametersPresentFlag[ui];  }
	Bool getVclHrdParametersPresentFlag( UInt ui )   const { return m_bVclHrdParametersPresentFlag[ui];  }
	Bool getPicStructPresentFlag( UInt ui )          const { return m_bPicStructPresentFlag[ui];         }
	Bool getTimingInfoPresentFlag( UInt ui )         const { return m_bTimingInfoPresentFlag[ui];        }
	Bool getLowDelayHrdFlag( UInt ui )               const { return m_bLowDelayHrdFlag[ui];              }
	UInt getNumLayersMinus1()                        const { return m_uiNumLayersMinus1;                 }
	UInt getDependencyId( UInt ui )                  const { return m_uiDependencyId[ui];                }
	UInt getQualityId( UInt ui )                     const { return m_uiQualityId[ui];                   }
	UInt getTemporalId( UInt ui )                    const { return m_uiTemporalId[ui];                  }
	UInt getNumUnitsInTick( UInt ui )                const { return m_uiNumUnitsInTick[ui];              }
	UInt getTimeScale( UInt ui )                     const { return m_uiTimeScale[ui];                   }
	Void setFixedFrameRateFlag( UInt ui, Bool b )          {  m_bFixedFrameRateFlag[ui]  = b;            }
  Void setNalHrdParametersPresentFlag( UInt ui, Bool b ) {  m_bNalHrdParametersPresentFlag[ui]  = b;   }
	Void setVclHrdParametersPresentFlag( UInt ui, Bool b ) {  m_bVclHrdParametersPresentFlag[ui]  = b;   }
	Void setPicStructPresentFlag( UInt ui, Bool b )        {  m_bPicStructPresentFlag[ui]  = b;          }
	Void setTimingInfoPresentFlag( UInt ui, Bool b )       {  m_bTimingInfoPresentFlag[ui]  = b;         } 
	Void setLowDelayHrdFlag( UInt ui, Bool b )             {  m_bLowDelayHrdFlag[ui]  = b;               }
	Void setNumLayersMinus1( UInt ui)                      {  m_uiNumLayersMinus1  = ui;                 }
	Void setDependencyId( UInt ui,UInt uj )                {  m_uiDependencyId[ui]  = uj;                }
	Void setQualityId( UInt ui,UInt uj )                   {  m_uiQualityId[ui]  = uj;                   }
	Void setTemporalId( UInt ui,UInt uj )                  {  m_uiTemporalId[ui]  = uj;                  }
	Void setNumUnitsInTick( UInt ui,UInt uj )              {  m_uiNumUnitsInTick[ui]  = uj;              }
	Void setTimeScale( UInt ui,UInt uj )                   {  m_uiTimeScale[ui]  = uj;                   }
  Void setSVCVUIParametersPresentFlag ( Bool b )         {  m_bSVCVUIParametersPresentFlag  = b;       }
	Void setAdditionalExtension2Flag    ( Bool b )         {  m_bAdditionalExtension2Flag     = b;       }
  Void setAdditionalExtension2DataFlag( Bool b )         {  m_bAdditionalExtension2DataFlag = b;       }
	Void setSubSPS                      ( Bool b )         {  m_bSubSPS                       = b;       }

//SSPS }

protected:
	ErrVal xReadPicOrderCntInfo         ( HeaderSymbolReadIf* pcReadIf );
  static ErrVal xGetLevelLimit        ( const LevelLimit*&    rpcLevelLimit,
                                        Int                   iLevelIdc );
  ErrVal        xReadFrext            ( HeaderSymbolReadIf*   pcReadIf );
  ErrVal        xWriteFrext           ( HeaderSymbolWriteIf*  pcWriteIf ) const;


protected:
  Bool          m_bInitDone;

  NalUnitType   m_eNalUnitType;
  UInt          m_uiLayerId;
  Profile       m_eProfileIdc;
  Bool          m_bConstrainedSet0Flag;
  Bool          m_bConstrainedSet1Flag;
  Bool          m_bConstrainedSet2Flag;
	Bool          m_bConstrainedSet3Flag;
  UInt          m_uiLevelIdc;
  UInt          m_uiSeqParameterSetId;
  UInt          m_uiChromaFormatIdc;//JVT-W046
	Bool          m_bSeqScalingMatrixPresentFlag;
  ScalingMatrix m_cSeqScalingMatrix;
  UInt          m_uiLog2MaxFrameNum;
	UInt          m_uiPicOrderCntType;
  UInt          m_uiLog2MaxPicOrderCntLsb;
  Bool          m_bDeltaPicOrderAlwaysZeroFlag;
  Int           m_iOffsetForNonRefPic;
  Int           m_iOffsetForTopToBottomField;
  UInt          m_uiNumRefFramesInPicOrderCntCycle;
  DynBuf<Int>   m_piOffsetForRefFrame;
  UInt          m_uiNumRefFrames;
	Bool					m_bGapsInFrameNumValueAllowedFlag;
  UInt          m_uiFrameWidthInMbs;
  UInt          m_uiFrameHeightInMbs;
  Bool          m_bDirect8x8InferenceFlag;

// TMM_ESS {
  UInt          m_uiExtendedSpatialScalability;
  UInt          m_uiChromaPhaseXPlus1;
  UInt          m_uiChromaPhaseYPlus1;
  UInt          m_uiBaseChromaPhaseXPlus1;
  UInt          m_uiBaseChromaPhaseYPlus1;
  Int           m_iScaledBaseLeftOffset;
  Int           m_iScaledBaseTopOffset;
  Int           m_iScaledBaseRightOffset;
  Int           m_iScaledBaseBottomOffset;

// TMM_ESS }

  Bool          m_bInterlayerDeblockingPresent;

// VW {
	UInt					m_auiNumRefIdxUpdateActiveDefault[2];
// VW }

  UInt          m_uiMGSVect[16];
	Bool          m_bFrameMbsOnlyFlag;
	Bool          m_bMbAdaptiveFrameFieldFlag;

  // Bool          m_bRCDOBlockSizes;
  // Bool          m_bRCDOMotionCompensationY;
  // Bool          m_bRCDOMotionCompensationC;
 // Bool          m_bRCDODeblocking;

  Bool          m_b4TapMotionCompensationY;  // V090

  Bool          m_bAVCRewriteFlag;          // V-035
  Bool          m_bAVCAdaptiveRewriteFlag;
  Bool          m_bAVCHeaderRewriteFlag;    // JVT-W046

  // JVT-V068 HRD {
  VUI*          m_pcVUI;
  // JVT-V068 HRD }

  //SSPS {
	Bool m_bSubSPS;
	Bool m_bSVCVUIParametersPresentFlag;
	Bool m_bAdditionalExtension2Flag;
	Bool m_bAdditionalExtension2DataFlag;
	Bool m_bFixedFrameRateFlag[MAX_LAYERS];
	Bool m_bNalHrdParametersPresentFlag[MAX_LAYERS];
	Bool m_bVclHrdParametersPresentFlag[MAX_LAYERS];
	Bool m_bPicStructPresentFlag[MAX_LAYERS];
	Bool m_bTimingInfoPresentFlag[MAX_LAYERS];
	Bool m_bLowDelayHrdFlag[MAX_LAYERS];
	UInt m_uiNumLayersMinus1;
	UInt m_uiDependencyId[MAX_LAYERS];
	UInt m_uiQualityId[MAX_LAYERS];
	UInt m_uiTemporalId[MAX_LAYERS];
	UInt m_uiNumUnitsInTick[MAX_LAYERS];
	UInt m_uiTimeScale[MAX_LAYERS];
	//SSPS }

private:
  static const LevelLimit m_aLevelLimit[52];
};


H264AVC_NAMESPACE_END


#endif // !defined(AFX_SEQUENCEPARAMETERSET_H__66281283_5BFB_429A_B722_6DDE7A11D086__INCLUDED_)
