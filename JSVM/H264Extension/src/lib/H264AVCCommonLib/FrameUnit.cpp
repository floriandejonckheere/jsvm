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




#include "H264AVCCommonLib.h"
#include "H264AVCCommonLib/FrameUnit.h"


H264AVC_NAMESPACE_BEGIN


FrameUnit::FrameUnit( YuvBufferCtrl& rcYuvFullPelBufferCtrl, YuvBufferCtrl& rcYuvHalfPelBufferCtrl, Bool bOriginal )
 : m_cFrame         ( rcYuvFullPelBufferCtrl, rcYuvHalfPelBufferCtrl )
 , m_cMbDataCtrl    ( )
 , m_pcPicBuffer    ( NULL )
 , m_uiFrameNumber  ( MSYS_UINT_MAX )
 , m_bOriginal      ( bOriginal )
 , m_bInitDone      ( false )
 , m_cResidual     ( rcYuvFullPelBufferCtrl, rcYuvHalfPelBufferCtrl )
 , m_bBaseRepresentation ( false )   //bug-fix base_rep
 , m_cFGSFrame      ( rcYuvFullPelBufferCtrl, rcYuvHalfPelBufferCtrl )
 , m_cFGSIntFrame   ( rcYuvFullPelBufferCtrl, rcYuvHalfPelBufferCtrl )
 , m_pcFGSPicBuffer ( NULL)
 , m_uiFGSReconCount(0)
 , m_cFGSRecon0     ( rcYuvFullPelBufferCtrl, rcYuvHalfPelBufferCtrl )
 , m_cFGSRecon1     ( rcYuvFullPelBufferCtrl, rcYuvHalfPelBufferCtrl )
 , m_cFGSRecon2     ( rcYuvFullPelBufferCtrl, rcYuvHalfPelBufferCtrl )
 , m_cFGSRecon3     ( rcYuvFullPelBufferCtrl, rcYuvHalfPelBufferCtrl )
{
    m_uiStatus = 0;
    m_apcFGSRecon[0] = & m_cFGSRecon0;
    m_apcFGSRecon[1] = & m_cFGSRecon1;
    m_apcFGSRecon[2] = & m_cFGSRecon2;
    m_apcFGSRecon[3] = & m_cFGSRecon3;
}

FrameUnit::~FrameUnit()
{

}

ErrVal FrameUnit::create( FrameUnit*& rpcFrameUnit, YuvBufferCtrl& rcYuvFullPelBufferCtrl, YuvBufferCtrl& rcYuvHalfPelBufferCtrl, Bool bOriginal )
{
  rpcFrameUnit = new FrameUnit( rcYuvFullPelBufferCtrl, rcYuvHalfPelBufferCtrl, bOriginal);
  ROT( NULL == rpcFrameUnit );
  return Err::m_nOK;
}


ErrVal FrameUnit::destroy()
{
  AOT_DBG( m_bInitDone );

  delete this;
  return Err::m_nOK;
}


ErrVal FrameUnit::init( const SliceHeader& rcSH, PicBuffer *pcPicBuffer )
{
  ROT( NULL == pcPicBuffer );
  ROT( NULL != m_pcPicBuffer );

  m_pcPicBuffer   = pcPicBuffer;
  m_uiFrameNumber = rcSH.getFrameNum();

  RNOK( m_cFrame.   init( m_pcPicBuffer->getBuffer(), this ) );
  m_pcPicBuffer->setUsed();

  ROTRS( m_bOriginal, Err::m_nOK );
  RNOK( m_cMbDataCtrl.init( rcSH.getSPS() ) );

  m_uiStatus = 0;

  UInt uiStamp = m_cFrame.stamp() + 1;
  m_cFrame.   stamp() = uiStamp;

  m_cResidual.init( false );
  m_cResidual.getFullPelYuvBuffer()->clear();
  m_bInitDone = true;

  m_cFGSIntFrame.init( false);
  m_pcFGSPicBuffer = NULL;

  m_bConstrainedIntraPred = rcSH.getPPS().getConstrainedIntraPredFlag();

  for( UInt uiLayerIdx = 0; uiLayerIdx < MAX_FGS_LAYERS + 1; uiLayerIdx ++ )
  {
    RNOK( m_apcFGSRecon[uiLayerIdx]->init( false ) );
  }

  m_cMbDataCtrl.initFgsBQData(m_cMbDataCtrl.getSize());

  return Err::m_nOK;
}


// HS: decoder robustness
ErrVal FrameUnit::init( const SliceHeader& rcSH, FrameUnit& rcFrameUnit )
{
  ROT( NULL != m_pcPicBuffer );

  m_uiFrameNumber = rcSH.getFrameNum();

  RNOK( m_cFrame.   init( NULL, this ) );
  m_cFrame.getFullPelYuvBuffer()->copy( rcFrameUnit.getFrame().getFullPelYuvBuffer() );
  m_cFrame.getFullPelYuvBuffer()->fillMargin();
  
  RNOK( m_cMbDataCtrl.init( rcSH.getSPS() ) );

  m_uiStatus = 0;

  UInt uiStamp = m_cFrame.stamp() + 1;
  m_cFrame.   stamp() = uiStamp;

  m_cResidual.init( false );
  m_cResidual.getFullPelYuvBuffer()->clear();
  m_bInitDone = true;

  m_cFGSIntFrame.init( false);
  m_cFGSIntFrame.getFullPelYuvBuffer()->copy( rcFrameUnit.getFGSIntFrame()->getFullPelYuvBuffer() );
  m_cFGSIntFrame.getFullPelYuvBuffer()->fillMargin();
  m_pcFGSPicBuffer = NULL;

  m_bConstrainedIntraPred = rcSH.getPPS().getConstrainedIntraPredFlag();

  setOutputDone();

  return Err::m_nOK;
}

//JVT-S036 lsj start
ErrVal FrameUnit::copyBase( const SliceHeader& rcSH, FrameUnit& rcFrameUnit )
{
  ROT( NULL != m_pcPicBuffer );

  m_uiFrameNumber = rcSH.getFrameNum();

  RNOK( m_cFrame.   init( NULL, this ) );
  m_cFrame.getFullPelYuvBuffer()->copy( rcFrameUnit.getFrame().getFullPelYuvBuffer() );
  m_cFrame.getFullPelYuvBuffer()->fillMargin();
  
  RNOK( m_cMbDataCtrl.init( rcSH.getSPS() ) );

  m_iMaxPOC = rcFrameUnit.getMaxPOC();
  m_uiStatus = rcFrameUnit.getStatus();  //JVT-S036 lsj

  UInt uiStamp = m_cFrame.stamp() + 1;
  m_cFrame.   stamp() = uiStamp;
  m_cFrame.setPOC(rcFrameUnit.getFrame().getPOC());//JVT-S036 lsj

  m_pcPicBuffer = rcFrameUnit.getPicBuffer();
  m_pcPicBuffer->setUsed(); //JVT-S036 lsj

  m_cResidual.init( false );
  m_cResidual.getFullPelYuvBuffer()->clear();
  m_bInitDone = true;
  
  m_cFGSIntFrame.init( false);
  m_pcFGSPicBuffer = NULL;

  m_bConstrainedIntraPred = rcSH.getPPS().getConstrainedIntraPredFlag();

  return Err::m_nOK;
}
ErrVal FrameUnit::uninitBase()  
{ 
	m_cFrame.uninit();
	m_pcPicBuffer->setUnused();
	m_pcPicBuffer = NULL;
	
	return Err::m_nOK;
}
//JVT-S036 lsj end

Void FrameUnit::setPoc( Int iPoc )
{
  m_cFrame.setPOC( iPoc );
  for( UInt uiLayerIdx = 0; uiLayerIdx < MAX_FGS_LAYERS + 1; uiLayerIdx ++ )
    m_apcFGSRecon[uiLayerIdx]->setPOC( iPoc );

  m_iMaxPOC = iPoc;
}


ErrVal FrameUnit::uninit()
{
  m_uiStatus = 0;
  m_bBaseRepresentation = false; //bug-fix base_rep
  m_pcPicBuffer = NULL;
  m_pcFGSPicBuffer = NULL;
  RNOK( m_cFGSFrame.uninit() );
  m_cFGSIntFrame.uninit();

  for( UInt uiLayerIdx = 0; uiLayerIdx < MAX_FGS_LAYERS + 1; uiLayerIdx ++ )
    RNOK( m_apcFGSRecon[uiLayerIdx]->uninit() );

  m_cMbDataCtrl.uninitFgsBQData();

  m_uiFrameNumber = 0;
  m_uiStatus      = 0;
  m_iMaxPOC       = 0;

  if( ! m_bOriginal )
  {
    RNOK( m_cMbDataCtrl.uninit() );
  }
  RNOK( m_cFrame.uninit() );

  m_cResidual.uninit();
  m_bInitDone = false;
  return Err::m_nOK;
}


Void FrameUnit::setUnused()
{
  m_uiStatus &= ~REFERENCE;
  getFrame().stamp()++;
}

ErrVal FrameUnit::setFGS( PicBuffer* pcPicBuffer )
{
  m_pcFGSPicBuffer = pcPicBuffer;

  RNOK( m_cFGSFrame.   init( m_pcFGSPicBuffer->getBuffer(), this ) );
  m_pcFGSPicBuffer->setUsed();

  m_cFGSFrame.setPOC( m_cFrame.getPOC() );

  return Err::m_nOK;
}

H264AVC_NAMESPACE_END
