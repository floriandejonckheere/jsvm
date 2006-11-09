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




#if !defined(AFX_PICTUREPARAMETERSET_H__8ED333BE_D213_4BFF_A379_67DDDA7F090C__INCLUDED_)
#define AFX_PICTUREPARAMETERSET_H__8ED333BE_D213_4BFF_A379_67DDDA7F090C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "H264AVCCommonLib/HeaderSymbolReadIf.h"
#include "H264AVCCommonLib/HeaderSymbolWriteIf.h"
#include "H264AVCCommonLib/ScalingMatrix.h"


//--ICU/ETRI FMO Implementation
#include <math.h>

H264AVC_NAMESPACE_BEGIN


//--ICU/ETRI FMO Implementation
const unsigned MAXNumSliceGroupsMinus1 =8; //it is also defined at cfmo.h


class H264AVCCOMMONLIB_API PictureParameterSet
{
protected:
  PictureParameterSet         ();
  virtual ~PictureParameterSet();

public:
  static ErrVal create  ( PictureParameterSet*& rpcPPS );
  ErrVal        destroy ();

  NalUnitType           getNalUnitType                          ()            const { return m_eNalUnitType; }
  UInt                  getLayerId                              ()            const { return m_uiLayerId; }
  UInt                  getPicParameterSetId                    ()            const { return m_uiPicParameterSetId; }
  UInt                  getSeqParameterSetId                    ()            const { return m_uiSeqParameterSetId; }
  Bool                  getEntropyCodingModeFlag                ()            const { return m_bEntropyCodingModeFlag; }
  Bool                  getPicOrderPresentFlag                  ()            const { return m_bPicOrderPresentFlag; }
  UInt                  getNumRefIdxActive                      ( ListIdx e ) const { return m_auiNumRefIdxActive[e]; }
  Bool                  getWeightedPredFlag                     ()            const { return m_bWeightedPredFlag; }
  UInt                  getWeightedBiPredIdc                    ()            const { return m_uiWeightedBiPredIdc; }
  UInt                  getPicInitQp                            ()            const { return m_uiPicInitQp; }
  Int                   getChomaQpIndexOffset                   ()            const { return m_iChomaQpIndexOffset; }
  Bool                  getDeblockingFilterParametersPresentFlag()            const { return m_bDeblockingFilterParametersPresentFlag; }
  Bool                  getConstrainedIntraPredFlag             ()            const { return m_bConstrainedIntraPredFlag; }
  Bool                  getRedundantPicCntPresentFlag           ()            const { return m_bRedundantPicCntPresentFlag; } //JVT-Q054 Red. Picture
  Bool                  getTransform8x8ModeFlag                 ()            const { return m_bTransform8x8ModeFlag; }
  Bool                  getPicScalingMatrixPresentFlag          ()            const { return m_bPicScalingMatrixPresentFlag; }
  const ScalingMatrix&  getPicScalingMatrix                     ()            const { return m_cPicScalingMatrix; }
  Int                   get2ndChromaQpIndexOffset               ()            const { return m_iSecondChromaQpIndexOffset; }


  //--ICU/ETRI FMO Implementation : FMO stuff start
  UInt          getNumSliceGroupsMinus1() const {return m_uiNumSliceGroupsMinus1;}
  UInt          getSliceGroupMapType() const {return  m_uiSliceGroupMapType;  }
  UInt          getRunLengthMinus1 (Int i) const {return m_uiRunLengthMinus1[i];}
  UInt          getTopLeft (Int i) const {return m_uiTopLeft[i];}
  UInt          getBottomRight (Int i) const {return m_uiBottomRight[i];}
  Bool          getSliceGroupChangeDirection_flag () const {return m_bSliceGroupChangeDirection_flag;}
  UInt          getSliceGroupChangeRateMinus1 () const {return m_uiSliceGroupChangeRateMinus1;}
  UInt          getNumSliceGroupMapUnitsMinus1() const {return m_uiNumSliceGroupMapUnitsMinus1;}
  UInt          getSliceGroupId(Int i) const {return m_uiSliceGroupId[i];}
  UInt*         getArrayRunLengthMinus1 () const {return (UInt*)m_uiRunLengthMinus1;}
  UInt*         getArrayTopLeft () const {return (UInt*)m_uiTopLeft;}
  UInt*         getArrayBottomRight () const {return (UInt*)m_uiBottomRight;}
  UInt*         getArraySliceGroupId() const {return (UInt*)m_uiSliceGroupId;}
  UInt          getSliceGroupChangeCycle() const {return m_uiSliceGroupChangeCycle;}
  UInt          getLog2MaxSliceGroupChangeCycle(UInt uiPicSizeInMapUnits) const {return UInt(ceil( (log ( uiPicSizeInMapUnits*(m_uiSliceGroupChangeRateMinus1+1.)+ 1. ))/log(2.) ));};
  //--ICU/ETRI FMO Implementation : FMO stuff end



  Void  setNalUnitType                          ( NalUnitType e )           { m_eNalUnitType                            = e; }
  Void  setLayerId                              ( UInt        ui )          { m_uiLayerId                               = ui; }
  Void  setPicParameterSetId                    ( UInt        ui )          { m_uiPicParameterSetId                     = ui; }
  Void  setSeqParameterSetId                    ( UInt        ui )          { m_uiSeqParameterSetId                     = ui; }
  Void  setEntropyCodingModeFlag                ( Bool        b )           { m_bEntropyCodingModeFlag                  = b; }
  Void  setPicOrderPresentFlag                  ( Bool        b )           { m_bPicOrderPresentFlag                    = b; }
  Void  setNumRefIdxActive                      ( ListIdx     e, UInt ui )  { m_auiNumRefIdxActive[e]                   = ui; }
  Void  setWeightedPredFlag                     ( Bool        b )           { m_bWeightedPredFlag                       = b; }
  Void  setWeightedBiPredIdc                    ( UInt        ui )          { m_uiWeightedBiPredIdc                     = ui; }
  Void  setPicInitQp                            ( UInt        ui )          { m_uiPicInitQp                             = ui; }
  Void  setChomaQpIndexOffset                   ( Int         i )           { m_iChomaQpIndexOffset                     = i; }
  Void  setDeblockingFilterParametersPresentFlag( Bool        b )           { m_bDeblockingFilterParametersPresentFlag  = b; }
  Void  setConstrainedIntraPredFlag             ( Bool        b )           { m_bConstrainedIntraPredFlag               = b; }
  Void  setRedundantPicCntPresentFlag           ( Bool        b )           { m_bRedundantPicCntPresentFlag             = b; }  // JVT-Q054 Red. Picture
  Void  setTransform8x8ModeFlag                 ( Bool        b )           { m_bTransform8x8ModeFlag                   = b; }
  Void  setPicScalingMatrixPresentFlag          ( Bool        b )           { m_bPicScalingMatrixPresentFlag            = b; }
  Void  set2ndChromaQpIndexOffset               ( Int         i )           { m_iSecondChromaQpIndexOffset              = i; }


  //--ICU/ETRI FMO Implementation : FMO stuff start
  Void setNumSliceGroupsMinus1(UInt   uiNumSliceGroupsMinus1) {m_uiNumSliceGroupsMinus1 =uiNumSliceGroupsMinus1;}
  Void setSliceGroupMapType(UInt          uiSliceGroupMapType) {m_uiSliceGroupMapType =uiSliceGroupMapType;  }
  Void setRunLengthMinus1 (UInt        uiRunLengthMinus1,  Int i) {m_uiRunLengthMinus1[i] = uiRunLengthMinus1;}
  Void setTopLeft (UInt          uiTopLeft, Int i) {m_uiTopLeft[i] = uiTopLeft;}
  Void setBottomRight (UInt          uiBottomRight, Int i){m_uiBottomRight[i] = uiBottomRight;}
  Void setSliceGroupChangeDirection_flag (Bool         SliceGroupChangeDirection_flag){m_bSliceGroupChangeDirection_flag = SliceGroupChangeDirection_flag;}
  Void setSliceGroupChangeRateMinus1 (UInt         SliceGroupChangeRateMinus1 ){m_uiSliceGroupChangeRateMinus1 = SliceGroupChangeRateMinus1;}
  Void setNumSliceGroupMapUnitsMinus1 (UInt         uiNumSliceGroupMapUnitsMinus1){ m_uiNumSliceGroupMapUnitsMinus1 = uiNumSliceGroupMapUnitsMinus1;}
  Void setSliceGroupId(UInt         uiSliceGroupId, Int i) {m_uiSliceGroupId[i] = uiSliceGroupId;}
  Void setArrayRunLengthMinus1 (UInt*        uiRunLengthMinus1)
  {
    for(UInt i=0;i<=getNumSliceGroupsMinus1();i++)
      m_uiRunLengthMinus1[i] = uiRunLengthMinus1[i];
  }
  Void setArrayTopLeft (UInt*          uiTopLeft)
  {
    for(UInt i=0;i<getNumSliceGroupsMinus1();i++)
      m_uiTopLeft[i] = uiTopLeft[i];
  }
  Void setArrayBottomRight (UInt*          uiBottomRight)
  {
    for(UInt i=0;i<getNumSliceGroupsMinus1();i++)
    m_uiBottomRight[i] = uiBottomRight[i];
  }
  Void setArraySliceGroupId(UInt*         uiSliceGroupId)
  {
    for(UInt i=0;i<=getNumSliceGroupsMinus1();i++)
    m_uiSliceGroupId[i] = uiSliceGroupId[i];
  }
  Void setSliceGroupChangeCycle(UInt SliceGroupChangeCycle){ m_uiSliceGroupChangeCycle = SliceGroupChangeCycle;}
  //--ICU/ETRI FMO Implementation : FMO stuff end


  ErrVal write      ( HeaderSymbolWriteIf*  pcWriteIf ) const;
  ErrVal read       ( HeaderSymbolReadIf*   pcReadIf,
                      NalUnitType           eNalUnitType );

protected:
  ErrVal xWriteFrext( HeaderSymbolWriteIf*  pcWriteIf ) const;
  ErrVal xReadFrext ( HeaderSymbolReadIf*   pcReadIf );

protected:
  NalUnitType   m_eNalUnitType;
  UInt          m_uiLayerId;
  UInt          m_uiPicParameterSetId;
  UInt          m_uiSeqParameterSetId;
  Bool          m_bEntropyCodingModeFlag;
  Bool          m_bPicOrderPresentFlag;


  //--ICU/ETRI FMO Implementation : FMO stuff start
  UInt          m_uiNumSliceGroupsMinus1;
  UInt          m_uiSliceGroupMapType;
  UInt          m_uiRunLengthMinus1[MAXNumSliceGroupsMinus1];
  UInt          m_uiTopLeft[MAXNumSliceGroupsMinus1];
  UInt          m_uiBottomRight[MAXNumSliceGroupsMinus1];
  Bool      m_bSliceGroupChangeDirection_flag;
  UInt      m_uiSliceGroupChangeRateMinus1;
  UInt      m_uiNumSliceGroupMapUnitsMinus1;
  UInt          m_uiSliceGroupId[MAXNumSliceGroupsMinus1];
  UInt          m_uiSliceGroupChangeCycle;
  //--ICU/ETRI FMO Implementation : FMO stuff end

  UInt          m_auiNumRefIdxActive[2];
  Bool          m_bWeightedPredFlag;
  UInt          m_uiWeightedBiPredIdc;
  UInt          m_uiPicInitQp;
  Int           m_iChomaQpIndexOffset;
  Bool          m_bDeblockingFilterParametersPresentFlag;
  Bool          m_bConstrainedIntraPredFlag;
  Bool          m_bTransform8x8ModeFlag;
  Bool          m_bPicScalingMatrixPresentFlag;
  ScalingMatrix m_cPicScalingMatrix;
  Int           m_iSecondChromaQpIndexOffset;
  Bool          m_bRedundantPicCntPresentFlag;  //JVT-Q054 Red. Picture u(1)
};


H264AVC_NAMESPACE_END


#endif // !defined(AFX_PICTUREPARAMETERSET_H__8ED333BE_D213_4BFF_A379_67DDDA7F090C__INCLUDED_)
