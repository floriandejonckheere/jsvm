/*
********************************************************************************

NOTE - One of the two copyright statements below may be chosen
       that applies for the software.

********************************************************************************

This software module was based the software developed by

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

/*
********************************************************************************

NOTE - One of the two copyright statements below may be chosen
that applies for the software.

********************************************************************************
This software module was originally created by

Bao, Yiliang (Nokia Research Center, Nokia Inc.)

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

To the extent that Nokia Inc.  owns patent rights that would be required to
make, use, or sell the originally developed software module or portions thereof
included in the ISO/IEC 14496-10:2005 Amd.1 (Scalable Video Coding) in a
conforming product, Nokia Inc.  will assure the ISO/IEC that it is willing
to negotiate licenses under reasonable and non-discriminatory terms and
conditions with applicants throughout the world.

Nokia Inc. retains full right to modify and use the code for its own
purpose, assign or donate the code to a third party and to inhibit third
parties from using the code for products that do not conform to MPEG-related
ITU Recommendations and/or ISO/IEC International Standards.

This copyright notice must be included in all copies or derivative works.
Copyright (c) ISO/IEC 2005.

********************************************************************************

COPYRIGHT AND WARRANTY INFORMATION

Copyright 2005, International Telecommunications Union, Geneva

The Nokia Inc.  hereby donate this source code to the ITU, with the following
understanding:
1. Nokia Inc. retain the right to do whatever they wish with the
contributed source code, without limit.
2. Nokia Inc. retain full patent rights (if any exist) in the technical
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


#ifndef _FGS_CODER_H_
#define _FGS_CODER_H_


#include "H264AVCCommonLib.h"
#include "H264AVCCommonLib/Transform.h"
#include "H264AVCCommonLib/MbDataCtrl.h"

class IntFrame;


H264AVC_NAMESPACE_BEGIN


class FGSCoder
{
public:

  enum
  {
    RQ_QP_DELTA = 6
  };

  FGSCoder()
    : m_bInit                     ( false )
    , m_bPicInit                  ( false )
    , m_papcYuvFullPelBufferCtrl  ( 0 )
    , m_pcTransform               ( 0 )
    , m_uiWidthInMB               ( 0 )
    , m_uiHeightInMB              ( 0 )
    , m_pcCurrMbDataCtrl          ( 0 )
    , m_pcBaseLayerSbb            ( 0 )
  {
  }

  Void getCoeffSigMap         ( UInt uiMbX, UInt uiMbY, S4x4Idx cIdx,    UChar *pucSigMap );
  Void getCoeffSigMap         ( UInt uiMbX, UInt uiMbY, B8x8Idx c8x8Idx, UChar *pucSigMap );
  Void getCoeffSigMap         ( UInt uiMbX, UInt uiMbY, CIdx cIdx,       UChar *pucSigMap );
  Void getCoeffSigMapChroma8x8( UInt uiMbX, UInt uiMbY, UInt uiPlane,    UChar *pucSigMap );

  ErrVal            xStoreBQLayerSigMap();
  ErrVal            xSwitchBQLayerSigMap();

  static Bool isSignificant(UChar ucCoeffState)
  {
    return ( ( ucCoeffState & SIGNIFICANT ) != 0 );
  }

  IntFrame*   getBaseLayerSbb()   { return m_pcBaseLayerSbb;   }
  MbDataCtrl* getMbDataCtrl()     { return m_pcCurrMbDataCtrl; }
  MbDataCtrl* getMbDataCtrlEL()     { return &m_cMbDataCtrlEL; }

  enum
  {
    CLEAR               = 0x00,
    SIGNIFICANT         = 0x01, // was significant in base layer or during the current path
    CODED               = 0x02, // was coded during the current path
    TRANSFORM_SPECIFIED = 0x04, // transform size was specified in base layer or during current path
    CHROMA_CBP_CODED    = 0x08,
    CHROMA_CBP_AC_CODED = 0x10,
    BASE_SIGN           = 0x20,
    NEWSIG              = 0x40, // new significant only during the current path

    NUM_COEFF_SHIFT     = 16
  };

protected:

  ErrVal            xInit                 ( YuvBufferCtrl** apcYuvFullPelBufferCtrl,
                                            Transform*      pcTransform );
  ErrVal            xInitSPS              ( const SequenceParameterSet& rcSPS );
  ErrVal            xUninit               ();
  ErrVal            xScaleTCoeffs         ( MbDataAccess&               rcMbDataAccess,
                                            Bool                        bBaseLayer  );
  ErrVal            xReconstructMacroblock( MbDataAccess&               rcMbDataAccess,
                                            IntYuvMbBuffer&             rcMbBuffer );

  // FMO FGS ICU/ETRI
  ErrVal            xInitializeCodingPath         (SliceHeader* pcSliceHeader);
  ErrVal            xUpdateCodingPath             (SliceHeader* pcSliceHeader);

  ErrVal            xInitializeCodingPath         ();
  ErrVal            xUpdateCodingPath             ();
  ErrVal            xClearCodingPath              ();

  ErrVal            xUpdateMacroblock             ( MbDataAccess&       rcMbDataAccessBL,
                                                    MbDataAccess&       rcMbDataAccessEL,
                                                    UInt                uiMbY,
                                                    UInt                uiMbX );
  ErrVal            xClearBaseCoeffs( MbDataAccess& rcMbDataAccess, MbDataAccess* pcMbDataAccessBase );

  ErrVal            xInitBaseLayerSbb     ( UInt uiLayerId );

  Bool              m_bInit;
  Bool              m_bPicInit;
  YuvBufferCtrl**   m_papcYuvFullPelBufferCtrl;
  Transform*        m_pcTransform;
  MbDataCtrl        m_cMbDataCtrlEL;
  MbDataCtrl*       m_pcCurrMbDataCtrl;

  UInt              m_uiWidthInMB;
  UInt              m_uiHeightInMB;
  Bool              m_bFgsComponentSep;

  UChar*            m_apaucLumaCoefMap         [16];
  UChar*            m_aapaucChromaDCCoefMap [2][ 4];
  UChar*            m_aapaucChromaACCoefMap [2][16];
  UChar*            m_paucBlockMap;
  UChar*            m_apaucChromaDCBlockMap [2];
  UChar*            m_apaucChromaACBlockMap [2];
  UChar*            m_paucSubMbMap;
  UInt*             m_pauiMacroblockMap;
  UChar*            m_apaucScanPosMap         [5];

  UChar*            m_apaucBQLumaCoefMap         [16];
  UChar*            m_aapaucBQChromaDCCoefMap [2][ 4];
  UChar*            m_aapaucBQChromaACCoefMap [2][16];
  UChar*            m_paucBQBlockMap;
  UChar*            m_apaucBQChromaDCBlockMap [2];
  UChar*            m_apaucBQChromaACBlockMap [2];
  UChar*            m_paucBQSubMbMap;
  UInt*             m_pauiBQMacroblockMap;

  UInt              m_uiLumaCbpRun;
  Bool              m_bLastLumaCbpFlag;
  UInt              m_uiChromaCbpRun;
  UInt              m_uiLastChromaCbp;
  UInt              m_uiLumaCbpNextMbX;
  UInt              m_uiLumaCbpNextMbY;
  UInt              m_uiLumaCbpNext8x8Idx;
  UInt              m_uiChromaCbpNextMbX;
  UInt              m_uiChromaCbpNextMbY;

  IntFrame*         m_pcBaseLayerSbb;

private:

  Void xUpdateCoeffMap(TCoeff& cBL, TCoeff cEL, UChar& sm)  
  {
    if ((cEL))                                                
    {
      if( sm | SIGNIFICANT && cEL < 0 ) // set sign only when base layer not significant
      {
        sm  |= BASE_SIGN;
      }
      sm    |= SIGNIFICANT;
      cBL   += cEL;
    }
  }

  Int               xScaleLevel4x4        ( Int                         iLevel,
                                            Int                         iIndex,
                                            const QpParameter&          cQP,
                                            const QpParameter&          cBaseQP );
  Int               xScaleLevel8x8        ( Int                         iLevel,
                                            Int                         iIndex,
                                            const QpParameter&          cQP,
                                            const QpParameter&          cBaseQP );
  ErrVal            xScaleSymbols4x4      ( TCoeff*                     piCoeff,
                                            const QpParameter&          cQP,
                                            const QpParameter&          cBaseQP );
  ErrVal            xScaleSymbols8x8      ( TCoeff*                     piCoeff,
                                            const QpParameter&          cQP,
                                            const QpParameter&          cBaseQP );
  ErrVal            xScale4x4Block        ( TCoeff*                     piCoeff,
                                            const UChar*                pucScale,
                                            UInt                        uiStart,
                                            const QpParameter&          rcQP );
  ErrVal            xScale8x8Block        ( TCoeff*                     piCoeff,
                                            const UChar*                pucScale,
                                            const QpParameter&          rcQP );
  ErrVal            xUpdateSymbols        ( TCoeff*                     piCoeff,
                                            TCoeff*                     piCoeffEL,
                                            Bool&                       bSigDC,
                                            Bool&                       bSigAC,
                                            Int                         iNumCoeff );
};



H264AVC_NAMESPACE_END

#endif  // _FGS_CODER_H_

