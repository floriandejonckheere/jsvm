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


H264AVC_NAMESPACE_BEGIN


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

  Bool getFGSInfoPresentFlag                  ()                          const { return m_bFGSInfoPresentFlag;       }
  Bool getFGSCycleAlignedFragment             ()                          const { return m_bFGSCycleAlignedFragment;  }
  UInt getNumFGSVectModes                     ()                          const { return m_uiNumFGSVectModes;         }
  Bool getFGSCodingMode                       ( UInt uiIndex )            const { return m_abFGSCodingMode[uiIndex];  }
  UInt getGroupingSize                        ( UInt uiIndex )            const { return m_auiGroupingSize[uiIndex];  }
  UInt getNumPosVectors                       ( UInt uiIndex )            const { return m_auiNumPosVectors[uiIndex]; }
  UInt getPosVect                             ( UInt uiIndex, UInt uiNum )const { return m_auiPosVect[uiIndex][uiNum];}

  UInt                  getPaff()                                       const { return m_uiPaff; }
	Bool                  getFrameMbsOnlyFlag()                           const { return m_bFrameMbsOnlyFlag; }
	Bool                  getMbAdaptiveFrameFieldFlag()                   const { return m_bMbAdaptiveFrameFieldFlag; }

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

  Void setFGSInfoPresentFlag                  ( Bool        b  )                        { m_bFGSInfoPresentFlag                   = b;  }
  Void setFGSCycleAlignedFragment             ( Bool        b  )                        { m_bFGSCycleAlignedFragment              = b;  }
  Void setNumFGSVectModes                     ( UInt        ui )                        { m_uiNumFGSVectModes                     = ui; }
  Void setFGSCodingMode                       ( UInt uiIndex, Bool        b  )          { m_abFGSCodingMode[uiIndex]              = b;  }
  Void setGroupingSize                        ( UInt uiIndex, UInt        ui )          { m_auiGroupingSize[uiIndex]              = ui; }
  Void setPosVect                             ( UInt uiIndex, UInt uiNum, UInt uiVect)  { m_auiPosVect[uiIndex][uiNum]            = uiVect; }

  Void setInterlayerDeblockingPresent ( Bool b ) { m_bInterlayerDeblockingPresent = b ;}
  Bool getInterlayerDeblockingPresent () const    { return m_bInterlayerDeblockingPresent; }

  Void  setPaff                               ( UInt        ui )          { m_uiPaff                                = ui; }
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

// TMM_ESS {
  Void setResizeParameters    ( const ResizeParameters * params );
  Void getResizeParameters    ( ResizeParameters * params ) const;

  Void setExtendedSpatialScalability ( UInt ui ) { m_uiExtendedSpatialScalability = ui ;}
  UInt getExtendedSpatialScalability () const    { return m_uiExtendedSpatialScalability; }

  // needs to be called after all the position vectors are set
  ErrVal checkPosVectors              ( UInt uiIndex )
  {
    UInt uiTotalVectorLength = 0;
    Bool bBadVector          = false;

    m_auiNumPosVectors[ uiIndex ]    = 0;
    while( uiTotalVectorLength < 16 && m_auiNumPosVectors[ uiIndex ] < 16 )
    {
      if( m_auiPosVect[uiIndex][m_auiNumPosVectors[uiIndex]] == 0 )
      {
        bBadVector      = true;
        break;
      }
      if( m_auiPosVect[uiIndex][m_auiNumPosVectors[uiIndex]] > (16 - uiTotalVectorLength) )
        m_auiPosVect[uiIndex][m_auiNumPosVectors[uiIndex]]  = 16 - uiTotalVectorLength;

      uiTotalVectorLength += m_auiPosVect[uiIndex][m_auiNumPosVectors[uiIndex]];
      m_auiNumPosVectors[uiIndex]++;
    }

    if( bBadVector )
    {
      // set the vector length to 1
      for( m_auiNumPosVectors[uiIndex] = 0; m_auiNumPosVectors[uiIndex] < 16; m_auiNumPosVectors[uiIndex] ++ )
        m_auiPosVect[uiIndex][m_auiNumPosVectors[uiIndex]] = 1;
    }

    return Err::m_nOK;
  }
  // TMM_ESS }

  Bool  getRCDOBlockSizes         () const { return m_bRCDOBlockSizes; }
  Bool  getRCDOMotionCompensationY() const { return m_bRCDOMotionCompensationY; }
  Bool  getRCDOMotionCompensationC() const { return m_bRCDOMotionCompensationC; }
  Bool  getRCDODeblocking         () const { return m_bRCDODeblocking; }

  Void  setRCDOBlockSizes         ( Bool b ) { m_bRCDOBlockSizes          = b; }
  Void  setRCDOMotionCompensationY( Bool b ) { m_bRCDOMotionCompensationY = b; }
  Void  setRCDOMotionCompensationC( Bool b ) { m_bRCDOMotionCompensationC = b; }
  Void  setRCDODeblocking         ( Bool b ) { m_bRCDODeblocking          = b; }

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
  Int           m_iScaledBaseLeftOffset;
  Int           m_iScaledBaseTopOffset;
  Int           m_iScaledBaseRightOffset;
  Int           m_iScaledBaseBottomOffset;
// TMM_ESS }

  Bool          m_bInterlayerDeblockingPresent;

// VW {
	UInt					m_auiNumRefIdxUpdateActiveDefault[2];
// VW }

  Bool          m_bFGSInfoPresentFlag; 
  Bool          m_bFGSCycleAlignedFragment; 
  UInt          m_uiNumFGSVectModes; 
  Bool          m_abFGSCodingMode [MAX_NUM_FGS_VECT_MODES];
  UInt          m_auiGroupingSize [MAX_NUM_FGS_VECT_MODES];
  UInt          m_auiNumPosVectors[MAX_NUM_FGS_VECT_MODES];
  UInt          m_auiPosVect [MAX_NUM_FGS_VECT_MODES] [16];
  UInt          m_uiPaff;
	Bool          m_bFrameMbsOnlyFlag;
	Bool          m_bMbAdaptiveFrameFieldFlag;

  Bool          m_bRCDOBlockSizes;
  Bool          m_bRCDOMotionCompensationY;
  Bool          m_bRCDOMotionCompensationC;
  Bool          m_bRCDODeblocking;

private:
  static const LevelLimit m_aLevelLimit[52];
};


H264AVC_NAMESPACE_END


#endif // !defined(AFX_SEQUENCEPARAMETERSET_H__66281283_5BFB_429A_B722_6DDE7A11D086__INCLUDED_)
