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




#if !defined(AFX_FGSSUBBANDDECODER_H__02F9A3A4_BE1C_4D88_86D9_98AF451F04CD__INCLUDED_)
#define AFX_FGSSUBBANDDECODER_H__02F9A3A4_BE1C_4D88_86D9_98AF451F04CD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "H264AVCCommonLib/CabacContextModel2DBuffer.h"
#include "CabaDecoder.h"
#include "H264AVCCommonLib/MbDataCtrl.h"
#include "H264AVCCommonLib/Transform.h"


H264AVC_NAMESPACE_BEGIN


class YuvBufferCtrl;
class Transform;
class MbDataCtrl;
class CabacReader;



class RQFGSDecoder  
{
private:
  enum
  {
    RQ_QP_DELTA = 6
  };
  class ReadStop
  {
  };

protected:
	RQFGSDecoder         ();
	virtual ~RQFGSDecoder();

public:
  static ErrVal     create                ( RQFGSDecoder*&              rpcFGSSubbandDecoder );
  ErrVal            destroy               ();

  ErrVal            init                  ( YuvBufferCtrl**             apcYuvFullPelBufferCtrl,
                                            Transform*                  pcTransform,
                                            CabacReader*                pcCabacReader );
  ErrVal            uninit                ();

  ErrVal            initPicture           ( SliceHeader*                pcSliceHeader,
                                            MbDataCtrl*                 pcCurrMbDataCtrl );
  ErrVal            decodeNextLayer       ( SliceHeader*                pcSliceHeader );
  ErrVal            reconstruct           ( IntFrame*                   pcRecResidual );
  ErrVal            finishPicture         ();

  Bool              isInitialized         ()    { return m_bPicInit; }
  Bool              isFinished            ()    { return m_bPicFinished; }
  Bool              changed               ()    { return m_bPicChanged; }
  SliceHeader*      getSliceHeader        ()    { return m_pcCurrSliceHeader; }
  MbDataCtrl*       getMbDataCtrl        ()     { return m_pcCurrMbDataCtrl; }


private:
  ErrVal            xInitSPS              ( const SequenceParameterSet& rcSPS );
  ErrVal            xScaleBaseLayerCoeffs ();

  ErrVal            xUpdateMacroblock     ( MbDataAccess&               rcMbDataAccess,
                                            MbDataAccess&               rcMbDataAccessEL,
                                            Bool                        bRefinement );
  ErrVal            xUpdateMacroblockQP   ( MbDataAccess&               rcMbDataAccess,
                                            MbDataAccess&               rcMbDataAccessEL,
                                            Int                         iNewQP );
  ErrVal            xUpdateMacroblockCoef ( MbDataAccess&               rcMbDataAccess,
                                            MbDataAccess&               rcMbDataAccessEL );

  ErrVal            xScale4x4Block        ( TCoeff*                     piCoeff,
                                            const UChar*                pucScale,
                                            UInt                        uiStart,
                                            const QpParameter&          rcQP );
  ErrVal            xScale8x8Block        ( TCoeff*                     piCoeff,
                                            const UChar*                pucScale,
                                            const QpParameter&          rcQP );
  ErrVal            xScaleTCoeffs         ( MbDataAccess&               rcMbDataAccess,
                                            Bool                        bBaseLayer );
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
  ErrVal            xUpdateSymbols        ( TCoeff*                     piCoeff,
                                            TCoeff*                     piCoeffEL,
                                            Bool&                       bSigDC,
                                            Bool&                       bSigAC,
                                            Int                         iNumCoeff );
  ErrVal            xReconstructMacroblock( MbDataAccess&               rcMbDataAccess,
                                            IntYuvMbBuffer&             rcMbBuffer );




  ErrVal            xDecodingFGS                  ();
  ErrVal            xInitializeCodingPath         ();
  ErrVal            xUpdateCodingPath             ();
  ErrVal            xUpdateMacroblock             ( MbDataAccess&       rcMbDataAccessBL,
                                                    MbDataAccess&       rcMbDataAccessEL,
                                                    UInt                uiMbY,
                                                    UInt                uiMbX );
  
  ErrVal            xDecodeNewCoefficientLuma     ( UInt                uiBlockYIndex,
                                                    UInt                uiBlockXIndex,
                                                    UInt                uiScanIndex,
                                                    Int&                riLastQp );
  ErrVal            xDecodeNewCoefficientChromaDC ( UInt                uiPlane,
                                                    UInt                uiMbYIdx,
                                                    UInt                uiMbXIdx,
                                                    UInt                uiDCIdx,
                                                    Int&                riLastQP );
  ErrVal            xDecodeNewCoefficientChromaAC ( UInt                uiPlane,
                                                    UInt                uiB8YIdx,
                                                    UInt                uiB8XIdx,
                                                    UInt                uiScanIndex,
                                                    Int&                riLastQP );
  
  ErrVal            xDecodeCoefficientLumaRef     ( UInt                uiBlockYIndex,
                                                    UInt                uiBlockXIndex,
                                                    UInt                uiScanIndex );
  ErrVal            xDecodeCoefficientChromaDCRef ( UInt                uiPlane,
                                                    UInt                uiMbYIdx,
                                                    UInt                uiMbXIdx,
                                                    UInt                uiDCIdx );
  ErrVal            xDecodeCoefficientChromaACRef ( UInt                uiPlane,
                                                    UInt                uiB8YIdx,
                                                    UInt                uiB8XIdx,
                                                    UInt                uiScanIdx );


private:
  Bool              m_bInit;
  YuvBufferCtrl**   m_papcYuvFullPelBufferCtrl;
  Transform*        m_pcTransform;
  CabacReader*      m_pcCabacReader;

  Bool              m_bPicInit;
  Bool              m_bPicChanged;
  Bool              m_bPicFinished;
  UInt              m_uiWidthInMB;
  UInt              m_uiHeightInMB;
  MbDataCtrl        m_cMbDataCtrlEL;
  MbDataCtrl*       m_pcCurrMbDataCtrl;
  SliceHeader*      m_pcCurrSliceHeader;


  enum
  {
    CLEAR               = 0x00,
    SIGNIFICANT         = 0x01, // was significant in base layer or during the current path
    CODED               = 0x02, // was coded during the current path
    TRANSFORM_SPECIFIED = 0x04, // transform size was specified in base layer or during current path
    CHROMA_CBP_CODED    = 0x08,
    CHROMA_CBP_AC_CODED = 0x10,

    NUM_COEFF_SHIFT     = 16
  };

  UChar*            m_apaucLumaCoefMap         [16];
  UChar*            m_aapaucChromaDCCoefMap [2][ 4];
  UChar*            m_aapaucChromaACCoefMap [2][16];
  UChar*            m_paucBlockMap;
  UChar*            m_apaucChromaDCBlockMap [2];
  UChar*            m_apaucChromaACBlockMap [2];
  UChar*            m_paucSubMbMap;
  UInt*             m_pauiMacroblockMap;
};



H264AVC_NAMESPACE_END


#endif // !defined(AFX_FGSSUBBANDDECODER_H__02F9A3A4_BE1C_4D88_86D9_98AF451F04CD__INCLUDED_)
