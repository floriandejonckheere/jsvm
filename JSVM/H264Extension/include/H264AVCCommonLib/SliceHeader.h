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




#if !defined(AFX_SLICEHEADER_H__G31F1842_FFCD_42AD_A981_7BD2736A4431__INCLUDED_)
#define AFX_SLICEHEADER_H__G31F1842_FFCD_42AD_A981_7BD2736A4431__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "H264AVCCommonLib/SliceHeaderBase.h"
#include "H264AVCCommonLib/CFMO.h"



H264AVC_NAMESPACE_BEGIN

#if defined( WIN32 )
# pragma warning( disable: 4275 )
# pragma warning( disable: 4251 )
#endif


class H264AVCCOMMONLIB_API SliceHeader : public SliceHeaderSyntax
{
public:
  SliceHeader();
  SliceHeader( const NalUnitHeader&         rcNalUnitHeader );
  SliceHeader( const PrefixHeader&          rcPrefixHeader );
  SliceHeader( const SequenceParameterSet&  rcSPS,
               const PictureParameterSet&   rcPPS );
  SliceHeader( const SliceHeader&           rcSliceHeader );
  virtual ~SliceHeader();

  ErrVal  init      ( const SequenceParameterSet& rcSPS,
                      const PictureParameterSet&  rcPPS );
  Void    copy      ( const SliceHeader&          rcSliceHeader );
  ErrVal  copyPrefix( const PrefixHeader&         rcPrefixHeader );
  
  Bool    isFirstSliceOfNextAccessUnit( const SliceHeader* pcLastSliceHeader ) const;

  //>>> remove
  Void          setLayerCGSSNR            ( UInt ui ) { m_uiLayerCGSSNR = ui;}
  Void          setQualityLevelCGSSNR     ( UInt ui ) { m_uiQualityLevelCGSSNR = ui;}
  Void          setBaseLayerCGSSNR        ( UInt ui ) { m_uiBaseLayerCGSSNR = ui;}
  Void          setBaseQualityLevelCGSSNR ( UInt ui ) { m_uiBaseQualityLevelCGSSNR = ui;}
  Void          setQLDiscardable          ( UInt ui ) { m_uiQLDiscardable = ui; }
  Void          setBaseLayerId            ( UInt ui ) { m_uiBaseLayerId = ui; }
  UInt          getLayerCGSSNR            ()  const   { return m_uiLayerCGSSNR;}
  UInt          getQualityLevelCGSSNR     ()  const   { return m_uiQualityLevelCGSSNR;}
  UInt          getBaseLayerCGSSNR        ()  const   { return m_uiBaseLayerCGSSNR;}
  UInt          getBaseQualityLevelCGSSNR ()  const   { return m_uiBaseQualityLevelCGSSNR;}
  UInt          getQLDiscardable          ()  const   { return m_uiQLDiscardable; }
  UInt          getBaseLayerId            ()  const   { return m_uiBaseLayerId; }

  ErrVal  compare           ( const SliceHeader*          pcSH,
                              Bool&                       rbNewPic,
                              Bool&                       rbNewFrame ) const;
  ErrVal  compareRedPic     ( const SliceHeader*          pcSH,
                              Bool&                       rbNewFrame ) const;
  ErrVal  sliceHeaderBackup ( SliceHeader*                pcSH       );
  ErrVal  FMOInit           ();
  ErrVal  FMOUninit         ();
  Int     getNumMbInSlice   ();
  //<<< remove

  Void              getMbPositionFromAddress    ( UInt& ruiMbY, UInt& ruiMbX,                   UInt uiMbAddress ) const;
  Void              getMbPosAndIndexFromAddress ( UInt& ruiMbY, UInt& ruiMbX, UInt& ruiMbIndex, UInt uiMbAddress ) const;
  UInt              getMbIndexFromAddress       (                                               UInt uiMbAddress ) const;
  UInt              getMbAddressFromPosition    ( UInt uiMbY, UInt uiMbX ) const;
  Bool              isFieldPair                 ( UInt uiFrameNum, PicType ePicType, Bool bIsRefPic ) const;
  Int               getDistScaleFactorWP        ( const Frame* pcFrameL0, const Frame*  pcFrameL1 ) const;
  const PredWeight& getPredWeight               ( ListIdx eListIdx, UInt uiRefIdx, Bool bFieldFlag ) const;
  PredWeight&       getPredWeight               ( ListIdx eListIdx, UInt uiRefIdx, Bool bFieldFlag );

  UChar           getChromaQp             ( UChar   ucLumaQp )  const { return g_aucChromaScale[ gClipMinMax( ucLumaQp + getPPS().getChomaQpIndexOffset(), 0, 51 ) ]; }
  const Bool      isScalingMatrixPresent  ( UInt    uiMatrix )  const { return SliceHeaderSyntax::getScalingMatrix().get( uiMatrix ) != 0; }
  const UChar*    getScalingMatrix        ( UInt    uiMatrix )  const { return SliceHeaderSyntax::getScalingMatrix().get( uiMatrix ); }

  ERROR_CONCEAL getErrorConcealMode       ()                    const { return m_eErrorConcealMode; }
  Bool          isTrueSlice               ()                    const { return m_bTrueSlice; }
  Bool			    getInIDRAccess            ()		                const { return m_bInIDRAccess; }
  UInt          getMbInPic                ()                    const { AOF( parameterSetsInitialized() ); return getFieldPicFlag() ? getSPS().getMbInFrame() / 2 : getSPS().getMbInFrame(); }
  UInt          getNumMbsInSlice          ()                    const { return m_uiNumMbsInSlice; }
  UInt          getLastMbInSlice          ()                    const { return m_uiLastMbInSlice; }
  Int           getTopFieldPoc            ()                    const { return m_iTopFieldPoc;  }
  Int           getBotFieldPoc            ()                    const { return m_iBotFieldPoc;  }
  RefFrameList* getRefFrameList           ( PicType ePicType,
                                            ListIdx eLstIdx )   const { return m_aapcRefFrameList[ ePicType - 1 ][ eLstIdx ]; }
  UInt          getNumRefIdxUpdate        ( UInt    uiTempLevel,
                                            ListIdx eListIdx )  const { return m_aauiNumRefIdxActiveUpdate[uiTempLevel][eListIdx]; }
  Bool          getSCoeffResidualPredFlag ()                    const { return m_bSCoeffResidualPred; }
  Bool          isReconstructionLayer     ()                    const { return m_bReconstructionLayer; }
  const FMO*    getFMO                    ()                    const { return &m_cFMO;}
  FMO*          getFMO                    ()                          { return &m_cFMO;}
  PicType       getPicType                ()                    const;
  Int           getPoc                    ()                    const;
  Int           getPoc                    ( PicType ePicType )  const;
  Int           getDistScaleFactor        ( PicType eMbPicType,
                                            SChar   sL0RefIdx,
                                            SChar   sL1RefIdx ) const;

  Void          setErrorConcealMode       ( ERROR_CONCEAL       eErrorConcealMode       )   { m_eErrorConcealMode       = eErrorConcealMode; }
  Void          setTrueSlice              ( Bool                bTrueSlice              )   { m_bTrueSlice              = bTrueSlice; }
  Void			    setInIDRAccess            ( Bool                bInIdrAccessUnit        )	  { m_bInIDRAccess            = bInIdrAccessUnit; }
  Void          setNumMbsInSlice          ( UInt                uiNumMbsInSlice         )   { m_uiNumMbsInSlice         = uiNumMbsInSlice; }
  Void          setLastMbInSlice          ( UInt                uiLastMbInSlice         )   { m_uiLastMbInSlice         = uiLastMbInSlice; }
  Void          setTopFieldPoc            ( Int                 iTopFieldPoc            )   { m_iTopFieldPoc            = iTopFieldPoc;  }
  Void          setBotFieldPoc            ( Int                 iBotFieldPoc            )   { m_iBotFieldPoc            = iBotFieldPoc;  }
  Void          setRefFrameList           ( RefFrameList*       pcRefFrameList,
                                            PicType             ePicType,
                                            ListIdx             eListIdx                )   { m_aapcRefFrameList[ ePicType - 1 ][ eListIdx ]  = pcRefFrameList; }
  Void          setNumRefIdxUpdate        ( UInt                uiTempLevel,
                                            ListIdx             eListIdx,
                                            UInt                uiNumRefIdxActive       )   { m_aauiNumRefIdxActiveUpdate[uiTempLevel][eListIdx] = uiNumRefIdxActive;  }
  Void          setSCoeffResidualPredFlag ( ResizeParameters*   pcResizeParameters      );
  Void          setReconstructionLayer    ( Bool                bReconstructionLayer    )   { m_bReconstructionLayer = bReconstructionLayer; }
  Void          setPicType                ( PicType             ePicType                );

private:
  ErrVal  xInitScalingMatrix();

private:
  ERROR_CONCEAL	m_eErrorConcealMode;
  Bool	        m_bTrueSlice;
  Bool          m_bInIDRAccess;
  UInt          m_uiNumMbsInSlice;
  UInt          m_uiLastMbInSlice;
  Int           m_iTopFieldPoc;
  Int           m_iBotFieldPoc;
  Bool          m_bSCoeffResidualPred;      // remove
  FMO           m_cFMO;
  RefFrameList* m_aapcRefFrameList[3][2];
  UInt          m_aauiNumRefIdxActiveUpdate[MAX_TEMP_LEVELS][2]; // for MCTF preprocessor
  Bool          m_bReconstructionLayer;
  //>>> remove
  UInt          m_uiLayerCGSSNR;
  UInt          m_uiQualityLevelCGSSNR;
  UInt          m_uiBaseLayerCGSSNR;
  UInt          m_uiBaseQualityLevelCGSSNR;
  UInt          m_uiQLDiscardable;
  UInt          m_uiBaseLayerId;
  //<<< remove
};



#if defined( WIN32 )
# pragma warning( default: 4251 )
# pragma warning( default: 4275 )
#endif



H264AVC_NAMESPACE_END


#endif // !defined(AFX_SLICEHEADER_H__G31F1842_FFCD_42AD_A981_7BD2736A4431__INCLUDED_)
