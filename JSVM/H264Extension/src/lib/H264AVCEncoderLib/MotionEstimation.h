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





#if !defined(AFX_MOTIONESTIMATION_H__00B6343B_CCFC_466D_9F60_04EB29EE8E56__INCLUDED_)
#define AFX_MOTIONESTIMATION_H__00B6343B_CCFC_466D_9F60_04EB29EE8E56__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



#include "MotionEstimationCost.h"
#include "Distortion.h"
#include "CodingParameter.h"

#include "H264AVCCommonLib/MotionCompensation.h"



H264AVC_NAMESPACE_BEGIN


class MotionVectorCalculation;
class RateDistortionIf;
class CodingParameter;
class QuarterPelFilter;


class MotionEstimation
: public MotionCompensation
, public MotionEstimationCost
{
protected:
  typedef struct
  {
    Void init( Short sLimit, Mv& rcMvPel, Mv cMin, Mv cMax)
    {
      cMin >>= 2;
      cMax >>= 2;
      Short sPosV = cMax.getVer() - rcMvPel.getVer();
      Short sNegV = rcMvPel.getVer() - cMin.getVer();
      iNegVerLimit = min( sLimit, sNegV) - rcMvPel.getVer();
      iPosVerLimit = min( sLimit, sPosV) + rcMvPel.getVer();

      Short sPosH = cMax.getHor() - rcMvPel.getHor();
      Short sNegH = rcMvPel.getHor() - cMin.getHor();
      iNegHorLimit = min( sLimit, sNegH) - rcMvPel.getHor();
      iPosHorLimit = min( sLimit, sPosH) + rcMvPel.getHor();
    }
    Int iNegVerLimit;
    Int iPosVerLimit;
    Int iNegHorLimit;
    Int iPosHorLimit;
  }
  SearchRect;

  typedef struct
  {
    XPel* pucYRef;
    XPel* pucURef;
    XPel* pucVRef;
    Int   iYStride;
    Int   iCStride;
    Int   iBestX;
    Int   iBestY;
    UInt  uiBestRound;
    UInt  uiBestDistance;
    UInt  uiBestSad;
    UChar ucPointNr;
  }
  IntTZSearchStrukt;

protected:
	MotionEstimation();
	virtual ~MotionEstimation();

public:
  ErrVal destroy();
  virtual ErrVal uninit();

  SampleWeighting* getSW() { return m_pcSampleWeighting; }

  ErrVal initMb( UInt uiMbPosY, UInt uiMbPosX, MbDataAccess& rcMbDataAccess );

  virtual ErrVal init(  XDistortion*  pcXDistortion,
                        CodingParameter* pcCodingParameter,
                        RateDistortionIf* pcRateDistortionIf,
                        QuarterPelFilter* pcQuarterPelFilter,
                        Transform*        pcTransform,
                        SampleWeighting* pcSampleWeighting);

  UInt    getRateCost           ( UInt                  uiBits,
                                  Bool                  bSad  )            { xGetMotionCost( bSad, 0 ); return xGetCost( uiBits ); }
  ErrVal  estimateBlockWithStart( const MbDataAccess&   rcMbDataAccess,
                                  const IntFrame&       rcRefFrame,
                                  Mv&                   rcMv,         // <-- MVSTART / --> MV
                                  Mv&                   rcMvPred,
                                  UInt&                 ruiBits,
                                  UInt&                 ruiCost,
                                  UInt                  uiBlk,
                                  UInt                  uiMode,
                                  Bool                  bQPelRefinementOnly,
                                  UInt                  uiSearchRange  = 0,
                                  IntYuvMbBuffer*       pcRefPelData2  = 0 );

  
  virtual ErrVal compensateBlock( IntYuvMbBuffer *pcRecPelData, UInt uiBlk, UInt uiMode, IntYuvMbBuffer *pcRefPelData2 = NULL ) = 0;

  DFunc getDistortionFunction() { return m_cParams.getSubPelDFunc(); }

protected:

  Void          xTZSearch             ( IntYuvPicBuffer *pcPelData, Mv& rcMv, UInt& ruiSAD, UInt uiSearchRange = 0 );
  __inline Void xTZSearchHelp         ( IntTZSearchStrukt& rcStrukt, const Int iSearchX, const Int iSearchY, const UChar ucPointNr, const UInt uiDistance );
  __inline Void xTZ2PointSearch       ( IntTZSearchStrukt& rcStrukt, SearchRect rcSearchRect );
  __inline Void xTZ8PointSquareSearch ( IntTZSearchStrukt& rcStrukt, SearchRect rcSearchRect, const Int iStartX, const Int iStartY, const Int iDist );
  __inline Void xTZ8PointDiamondSearch( IntTZSearchStrukt& rcStrukt, SearchRect rcSearchRect, const Int iStartX, const Int iStartY, const Int iDist );

  Void          xPelBlockSearch ( IntYuvPicBuffer *pcPelData, Mv& rcMv, UInt& ruiSAD,                              UInt uiSearchRange = 0 );
  Void          xPelSpiralSearch( IntYuvPicBuffer *pcPelData, Mv& rcMv, UInt& ruiSAD,                              UInt uiSearchRange = 0 );
  Void          xPelLogSearch   ( IntYuvPicBuffer *pcPelData, Mv& rcMv, UInt& ruiSAD, Bool bFme,  UInt uiStep = 4, UInt uiSearchRange = 0 );
  virtual Void  xSubPelSearch   ( IntYuvPicBuffer *pcPelData, Mv& rcMv, UInt& ruiSAD, UInt uiBlk, UInt uiMode,     Bool bQPelOnly, IntYuvMbBuffer *pcRefPelData2 ) = 0;

protected:
  QuarterPelFilter* m_pcQuarterPelFilter;
  MotionVectorSearchParams m_cParams;
  Int m_iMaxLogStep;
  Mv *m_pcMvSpiralSearch;
  UInt m_uiSpiralSearchEntries;
  Mv m_cLastPelMv;
  Mv  m_acMvPredictors[3];

  XDistortion*      m_pcXDistortion;
  XDistSearchStruct m_cXDSS;
};


H264AVC_NAMESPACE_END

#endif // !defined(AFX_MOTIONESTIMATION_H__00B6343B_CCFC_466D_9F60_04EB29EE8E56__INCLUDED_)
