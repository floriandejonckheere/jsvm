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




#include "H264AVCDecoderLib.h"
#include "MbDecoder.h"
#include "SliceDecoder.h"
#include "H264AVCCommonLib/SliceHeader.h"
#include "DecError.h"

#include "H264AVCCommonLib/MbDataCtrl.h"
#include "H264AVCCommonLib/Transform.h"


H264AVC_NAMESPACE_BEGIN


SliceDecoder::SliceDecoder():
  m_pcMbDecoder( NULL ),
  m_pcControlMng( NULL ),
  m_bInitDone( false)
{
  m_bReconstructionBypass = false;
}

SliceDecoder::~SliceDecoder()
{
}

ErrVal SliceDecoder::create( SliceDecoder*& rpcSliceDecoder )
{
  rpcSliceDecoder = new SliceDecoder;
  ROT( NULL == rpcSliceDecoder );
  return Err::m_nOK;
}


ErrVal SliceDecoder::destroy()
{
  ROT( m_bInitDone );
  delete this;
  return Err::m_nOK;
}

ErrVal SliceDecoder::init( MbDecoder* pcMbDecoder,
                           ControlMngIf* pcControlMng,
                           Transform* pcTransform)
{
  ROT( m_bInitDone );
  ROT( NULL == pcMbDecoder );
  ROT( NULL == pcControlMng );
  ROT( NULL == pcTransform );

  m_pcTransform = pcTransform;
  m_pcMbDecoder   = pcMbDecoder;
  m_pcControlMng  = pcControlMng;
  m_bInitDone     = true;
  return Err::m_nOK;
}


ErrVal SliceDecoder::uninit()
{
  ROF( m_bInitDone );
  m_pcMbDecoder   =  NULL;
  m_pcControlMng  =  NULL;
  m_bInitDone     = false;
  return Err::m_nOK;
}



ErrVal SliceDecoder::process( const SliceHeader& rcSH, UInt uiMbRead )
{
  ROF( m_bInitDone );

  //====== initialization ======
  UInt  uiMbAddress   = rcSH.getFirstMbInSlice();
  m_pcMbDecoder->setReconstructionBypass( m_bReconstructionBypass );

  //===== loop over macroblocks =====
  for( ; uiMbRead; uiMbAddress++ )
  {
    MbDataAccess* pcMbDataAccess;
    RNOK( m_pcControlMng->initMbForDecoding( pcMbDataAccess, uiMbAddress ) );
    RNOK( m_pcMbDecoder->process( *pcMbDataAccess ) );
    uiMbRead--;
  }

  m_pcMbDecoder->setReconstructionBypass();

  return Err::m_nOK;
}


ErrVal SliceDecoder::decodeIntra( SliceHeader&  rcSH,
                                  MbDataCtrl*   pcMbDataCtrl,
                                  IntFrame*     pcFrame,
                                  IntFrame*     pcSubband,
                                  IntFrame*     pcPredSignal,
                                  IntFrame*     pcBaseLayer,
                                  UInt          uiMbInRow,
                                  UInt          uiMbRead )
{
  ROF( m_bInitDone );

  //====== initialization ======
  UInt  uiMbAddress         = rcSH.getFirstMbInSlice();

  RNOK( pcMbDataCtrl->initSlice( rcSH, DECODE_PROCESS, true, NULL ) );

  //===== loop over macroblocks =====
  for( ; uiMbRead; uiMbAddress++ )
  {
    UInt          uiMbY       = uiMbAddress / uiMbInRow;
    UInt          uiMbX       = uiMbAddress % uiMbInRow;
    MbDataAccess* pcMbDataAccess;

    RNOK( pcMbDataCtrl  ->initMb            (  pcMbDataAccess, uiMbY, uiMbX ) );
    RNOK( m_pcControlMng->initMbForDecoding ( *pcMbDataAccess, uiMbAddress  ) );

    RNOK( m_pcMbDecoder ->decodeIntra       ( *pcMbDataAccess, pcFrame, pcSubband, pcPredSignal, pcBaseLayer ) );
    uiMbRead--;
  }

  return Err::m_nOK;
}


ErrVal SliceDecoder::decodeInterP( SliceHeader&   rcSH,
                                   MbDataCtrl*    pcMbDataCtrl,
                                   MbDataCtrl*    pcMbDataCtrlBase,
                                   IntFrame*      pcFrame,
                                   IntFrame*      pcSubband,
                                   IntFrame*      pcPredSignal,
                                   IntFrame*      pcBaseLayer,
                                   IntFrame*      pcBaseLayerSubband,
                                   RefFrameList&  rcRefFrameList,
                                   UInt           uiMbInRow, 
                                   UInt           uiMbRead )
{
  ROF( m_bInitDone );
  m_pcMbDecoder->setReconstructionBypass( m_bReconstructionBypass );

  //====== initialization ======
  UInt  uiMbAddress         = rcSH.getFirstMbInSlice();

  RNOK( pcMbDataCtrl->initSlice( rcSH, DECODE_PROCESS, true, NULL ) );

  //===== loop over macroblocks =====
  for( ; uiMbRead; uiMbAddress++ )
  {
    UInt          uiMbY               = uiMbAddress / uiMbInRow;
    UInt          uiMbX               = uiMbAddress % uiMbInRow;
    MbDataAccess* pcMbDataAccess      = 0;
    MbDataAccess* pcMbDataAccessBase  = 0;

    RNOK( pcMbDataCtrl  ->initMb            (  pcMbDataAccess, uiMbY, uiMbX ) );
    if( pcMbDataCtrlBase )
    {
      RNOK( pcMbDataCtrlBase->initMb        ( pcMbDataAccessBase, uiMbY, uiMbX  ) );
    }
    RNOK( m_pcControlMng->initMbForDecoding ( *pcMbDataAccess, uiMbAddress  ) );

    RNOK( m_pcMbDecoder ->decodeInterP      ( *pcMbDataAccess,
                                              pcMbDataAccessBase,
                                              pcFrame,
                                              pcSubband,
                                              pcPredSignal,
                                              pcBaseLayer,
                                              pcBaseLayerSubband,
                                              rcRefFrameList ) );
    uiMbRead--;
  }

  m_pcMbDecoder->setReconstructionBypass();
  return Err::m_nOK;
}



ErrVal SliceDecoder::decodeHighPass( SliceHeader& rcSH,
                                     MbDataCtrl*  pcMbDataCtrl,
                                     MbDataCtrl*  pcMbDataCtrlBase,
                                     IntFrame*    pcFrame, 
                                     IntFrame*    pcResidual,
                                     IntFrame*    pcPredSignal,
                                     IntFrame*    pcBaseSubband,
                                     IntFrame*    pcBaseLayer,
                                     UInt         uiMbInRow,
                                     UInt         uiMbRead )
{
  ROF( m_bInitDone );
  m_pcMbDecoder->setReconstructionBypass( m_bReconstructionBypass );

  //====== initialization ======
  UInt  uiMbAddress         = rcSH.getFirstMbInSlice();

  if( pcMbDataCtrlBase )
  {
    RNOK( pcMbDataCtrlBase->initSlice( rcSH, PRE_PROCESS, true, NULL     ) );
  }
  RNOK( pcMbDataCtrl    ->initSlice( rcSH, DECODE_PROCESS, true, NULL  ) );

  //===== loop over macroblocks =====
  for( ; uiMbRead; uiMbAddress++ )
  {
    UInt          uiMbY       = uiMbAddress / uiMbInRow;
    UInt          uiMbX       = uiMbAddress % uiMbInRow;
    MbDataAccess* pcMbDataAccess;
    MbDataAccess* pcMbDataAccessBase = NULL;

    if( pcMbDataCtrlBase )
    {
      RNOK( pcMbDataCtrlBase->initMb            (  pcMbDataAccessBase, uiMbY, uiMbX ) );
    }
    RNOK( pcMbDataCtrl    ->initMb            (  pcMbDataAccess,     uiMbY, uiMbX ) );
    RNOK( m_pcControlMng  ->initMbForDecoding ( *pcMbDataAccess,     uiMbAddress  ) );

    if( pcMbDataAccess->getMbData().isIntra() )
    {
      pcMbDataAccess->getSH().setSliceType( I_SLICE );
      RNOK( m_pcMbDecoder ->decodeIntra       ( *pcMbDataAccess, pcFrame, pcResidual, pcPredSignal, pcBaseLayer ) );
      pcMbDataAccess->getSH().setSliceType( P_SLICE );
    }
    else
    {
      m_pcTransform->setClipMode( false );
      RNOK( m_pcMbDecoder ->decodeResidual    ( *pcMbDataAccess, pcMbDataAccessBase, pcFrame, pcResidual, pcPredSignal, pcBaseSubband ) );
      m_pcTransform->setClipMode( true );
    }
    uiMbRead--;
  }

  m_pcMbDecoder->setReconstructionBypass();
  return Err::m_nOK;
}




H264AVC_NAMESPACE_END
