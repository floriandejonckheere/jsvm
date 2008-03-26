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




#if !defined(AFX_CONTROLMNG_H__CE22E161_6ACE_4E6F_9E1B_08F0A99F9742__INCLUDED_)
#define AFX_CONTROLMNG_H__CE22E161_6ACE_4E6F_9E1B_08F0A99F9742__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "H264AVCCommonLib/ControlMngIf.h"
#include "H264AVCCommonLib/ParameterSetMng.h"
#include "H264AVCCommonLib/PocCalculator.h"
#include "SliceReader.h"
#include "NalUnitParser.h"
#include "SliceDecoder.h"
#include "BitReadBuffer.h"
#include "UvlcReader.h"
#include "MbParser.h"
#include "H264AVCCommonLib/LoopFilter.h"
#include "MbDecoder.h"
#include "H264AVCCommonLib/Transform.h"
#include "H264AVCCommonLib/IntraPrediction.h"
#include "H264AVCCommonLib/MotionCompensation.h"
#include "H264AVCCommonLib/YuvBufferCtrl.h"
#include "H264AVCCommonLib/QuarterPelFilter.h"
#include "CabacReader.h"
#include "H264AVCCommonLib/SampleWeighting.h"
#include "GOPDecoder.h"
#include "H264AVCDecoder.h"

H264AVC_NAMESPACE_BEGIN

class ControlMngH264AVCDecoder : public ControlMngIf
{
protected:
	ControlMngH264AVCDecoder();
	virtual ~ControlMngH264AVCDecoder();

public:
  static ErrVal create  ( ControlMngH264AVCDecoder*&  rpcControlMngH264AVCDecoder );
  ErrVal        destroy ();
  ErrVal        init    ( UvlcReader*                 pcUvlcReader,
                          MbParser*                   pcMbParser,
                          MotionCompensation*         pcMotionCompensation,
                          YuvBufferCtrl*              pcYuvFullPelBufferCtrl[MAX_LAYERS],
                          CabacReader*                pcCabacReader,
                          SampleWeighting*            pcSampleWeighting,
                          LayerDecoder*               apcLayerDecoder       [MAX_LAYERS] );
  ErrVal        uninit  ();

  ErrVal initMbForParsing     ( MbDataAccess*& rpcMbDataAccess, UInt uiMbIndex );
  ErrVal initMbForDecoding    ( MbDataAccess*& rpcMbDataAccess, UInt uiMbY, UInt uiMbX, Bool bMbAff );
  ErrVal initMbForFiltering   ( MbDataAccess*& rpcMbDataAccess, UInt uiMbY, UInt uiMbX, Bool bMbAff );

  ErrVal initSlice0           ( SliceHeader*                rcSH);
  ErrVal initSPS              ( SequenceParameterSet&       rcSequenceParameterSet, UInt  uiLayer );
  ErrVal initParameterSets    ( const SequenceParameterSet& rcSPS,
                                const PictureParameterSet&  rcPPSLP,
                                const PictureParameterSet&  rcPPSHP  )    { return Err::m_nERR; }

  ErrVal initSliceForCoding   ( const SliceHeader& rcSH ) { return Err::m_nERR; }
  ErrVal initSliceForReading  ( const SliceHeader& rcSH );
  ErrVal initSliceForDecoding ( const SliceHeader& rcSH );
  ErrVal initSliceForFiltering( const SliceHeader& rcSH );
  ErrVal initSliceForWeighting( const SliceHeader& rcSH ) { return m_pcSampleWeighting->initSlice( rcSH ); }
  ErrVal finishSlice          ( const SliceHeader& rcSH, Bool& rbPicDone, Bool& rbFrameDone );

  ErrVal initMbForCoding      ( MbDataAccess& rcMbDataAccess, UInt uiMbY, UInt uiMbX, Bool bMbAff, Bool bFieldFlag ) { return Err::m_nERR; }
  ErrVal initMbForDecoding    ( MbDataAccess& rcMbDataAccess, UInt uiMbY, UInt uiMbX, Bool bMbAff );
  ErrVal initMbForFiltering   ( MbDataAccess& rcMbDataAccess, UInt uiMbY, UInt uiMbX, Bool bMbAff );

protected:
  UInt                      m_uiCurrLayer;
  UInt                      m_auiMbXinFrame           [MAX_LAYERS]; 
  UInt                      m_auiMbYinFrame           [MAX_LAYERS];
  MbDataCtrl*               m_pcMbDataCtrl;

  UvlcReader*               m_pcUvlcReader;
  MbParser*                 m_pcMbParser;
  MotionCompensation*       m_pcMotionCompensation;
  YuvBufferCtrl*            m_apcYuvFullPelBufferCtrl [MAX_LAYERS];
  CabacReader*              m_pcCabacReader;
  SampleWeighting*          m_pcSampleWeighting;
  LayerDecoder*             m_apcLayerDecoder         [MAX_LAYERS];
  Bool                      m_uiInitialized           [MAX_LAYERS];
};

H264AVC_NAMESPACE_END

#endif // !defined(AFX_CONTROLMNG_H__CE22E161_6ACE_4E6F_9E1B_08F0A99F9742__INCLUDED_)
