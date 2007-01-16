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
 : m_cFrame         ( rcYuvFullPelBufferCtrl, rcYuvHalfPelBufferCtrl, FRAME )
 , m_cTopField      ( rcYuvFullPelBufferCtrl, rcYuvHalfPelBufferCtrl, TOP_FIELD )
 , m_cBotField      ( rcYuvFullPelBufferCtrl, rcYuvHalfPelBufferCtrl, BOT_FIELD )
 , m_cFGSFrame      ( rcYuvFullPelBufferCtrl, rcYuvHalfPelBufferCtrl, FRAME )
 , m_cFGSTopField   ( rcYuvFullPelBufferCtrl, rcYuvHalfPelBufferCtrl, TOP_FIELD )
 , m_cFGSBotField   ( rcYuvFullPelBufferCtrl, rcYuvHalfPelBufferCtrl, BOT_FIELD )
 , m_cFGSIntFrame   ( rcYuvFullPelBufferCtrl, rcYuvHalfPelBufferCtrl, FRAME )
 , m_iMaxPoc        ( MSYS_INT_MIN )
 , m_eAvailable     ( NOT_SPECIFIED )
 , m_ePicStruct     ( PS_NOT_SPECIFIED )     
 , m_bFieldCoded    ( false )
 , m_cMbDataCtrl    ( )
 , m_pcPicBuffer    ( NULL )
 , m_uiFrameNumber  ( MSYS_UINT_MAX )
 , m_bOriginal      ( bOriginal )
 , m_bInitDone      ( false )
 , m_cResidual     ( rcYuvFullPelBufferCtrl, rcYuvHalfPelBufferCtrl, FRAME )
 , m_bBaseRepresentation ( false )   //bug-fix base_rep
 , m_pcFGSPicBuffer ( NULL)
 , m_uiFGSReconCount(0)
 , m_cFGSRecon0     ( rcYuvFullPelBufferCtrl, rcYuvHalfPelBufferCtrl, FRAME  )
 , m_cFGSRecon1     ( rcYuvFullPelBufferCtrl, rcYuvHalfPelBufferCtrl, FRAME  )
 , m_cFGSRecon2     ( rcYuvFullPelBufferCtrl, rcYuvHalfPelBufferCtrl, FRAME  )
 , m_cFGSRecon3     ( rcYuvFullPelBufferCtrl, rcYuvHalfPelBufferCtrl, FRAME  )
 , m_uiStatus       ( 0 )
{
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
	RNOK( m_cTopField.init( m_pcPicBuffer->getBuffer(), this ) );
	RNOK( m_cBotField.init( m_pcPicBuffer->getBuffer(), this ) );
  m_ePicStruct    = m_pcPicBuffer->getPicStruct();
  m_pcPicBuffer->setUsed();

	m_bOriginal = (NULL == &rcSH.getSPS());
  ROTRS( m_bOriginal, Err::m_nOK );
  RNOK( m_cMbDataCtrl.init( rcSH.getSPS() ) );

  m_uiStatus = 0;
  m_eAvailable = NOT_SPECIFIED;
  m_iMaxPoc    = 0;

	UInt uiStamp = max( m_cFrame.stamp(), max( m_cTopField.stamp(), m_cBotField.stamp() ) ) + 1;
	m_cTopField.stamp() = uiStamp;
	m_cBotField.stamp() = uiStamp;
  m_cFrame.   stamp() = uiStamp;

  m_cResidual.init();
  m_cResidual.getFullPelYuvBuffer()->clear();
  m_bInitDone = true;

  m_cFGSIntFrame.init();

  m_pcFGSPicBuffer = NULL;

  m_bConstrainedIntraPred = rcSH.getPPS().getConstrainedIntraPredFlag();

  for( UInt uiLayerIdx = 0; uiLayerIdx < MAX_FGS_LAYERS + 1; uiLayerIdx ++ )
  {
    RNOK( m_apcFGSRecon[uiLayerIdx]->init() );
  }

  m_cMbDataCtrl.initFgsBQData(m_cMbDataCtrl.getSize());

  return Err::m_nOK;
}


// HS: decoder robustness
ErrVal FrameUnit::init( const SliceHeader& rcSH, FrameUnit& rcFrameUnit )
{
  ROT( NULL != m_pcPicBuffer );

  m_uiFrameNumber = rcSH.getFrameNum();

  RNOK( getPic( TOP_FIELD )->init( NULL, this ) );
	RNOK( getPic( BOT_FIELD )->init( NULL, this ) );
  RNOK( getPic( FRAME     )->init( NULL, this ) );

  RNOK( getPic( TOP_FIELD )->getFullPelYuvBuffer()->copy( rcFrameUnit.getPic( TOP_FIELD )->getFullPelYuvBuffer() ) );
	RNOK( getPic( BOT_FIELD )->getFullPelYuvBuffer()->copy( rcFrameUnit.getPic( BOT_FIELD )->getFullPelYuvBuffer() ) );
  RNOK( getPic( FRAME     )->getFullPelYuvBuffer()->copy( rcFrameUnit.getPic( FRAME     )->getFullPelYuvBuffer() ) );

  RNOK( getPic( TOP_FIELD )->getFullPelYuvBuffer()->fillMargin() );
	RNOK( getPic( BOT_FIELD )->getFullPelYuvBuffer()->fillMargin() );
  RNOK( getPic( FRAME     )->getFullPelYuvBuffer()->fillMargin() );
  
  RNOK( m_cMbDataCtrl.init( rcSH.getSPS() ) );

  m_uiStatus = 0;
	m_eAvailable = NOT_SPECIFIED;
  m_iMaxPoc    = 0;

	UInt uiStamp = max( m_cFrame.stamp(), max( m_cTopField.stamp(), m_cBotField.stamp() ) ) + 1;
	m_cTopField.stamp() = uiStamp;
	m_cBotField.stamp() = uiStamp;
  m_cFrame.   stamp() = uiStamp;

  m_cResidual.init();
  m_cResidual.getFullPelYuvBuffer()->clear();
  m_bInitDone = true;

  m_cFGSIntFrame.init();
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

  RNOK( getPic( TOP_FIELD )->init( NULL, this ) );
	RNOK( getPic( BOT_FIELD )->init( NULL, this ) );
  RNOK( getPic( FRAME     )->init( NULL, this ) );

  RNOK( getPic( TOP_FIELD )->getFullPelYuvBuffer()->copy( rcFrameUnit.getPic( TOP_FIELD )->getFullPelYuvBuffer() ) );
	RNOK( getPic( BOT_FIELD )->getFullPelYuvBuffer()->copy( rcFrameUnit.getPic( BOT_FIELD )->getFullPelYuvBuffer() ) );
  RNOK( getPic( FRAME     )->getFullPelYuvBuffer()->copy( rcFrameUnit.getPic( FRAME     )->getFullPelYuvBuffer() ) );

  RNOK( getPic( TOP_FIELD )->getFullPelYuvBuffer()->fillMargin() );
	RNOK( getPic( BOT_FIELD )->getFullPelYuvBuffer()->fillMargin() );
  RNOK( getPic( FRAME     )->getFullPelYuvBuffer()->fillMargin() );
  
  
  RNOK( m_cMbDataCtrl.init( rcSH.getSPS() ) );

  m_iMaxPoc = rcFrameUnit.getMaxPoc();
  m_uiStatus = rcFrameUnit.getStatus();  //JVT-S036 lsj

  UInt uiStamp = max( m_cFrame.stamp(), max( m_cTopField.stamp(), m_cBotField.stamp() ) ) + 1;
	m_cTopField.stamp() = uiStamp;
	m_cBotField.stamp() = uiStamp;
  m_cFrame.   stamp() = uiStamp;
    
  m_cTopField.setPoc(rcFrameUnit.getPic(TOP_FIELD)->getPoc());//JVT-S036 lsj
  m_cBotField.setPoc(rcFrameUnit.getPic(BOT_FIELD)->getPoc());//JVT-S036 lsj
  m_cFrame.   setPoc(rcFrameUnit.getPic(FRAME)    ->getPoc());//JVT-S036 lsj

  m_pcPicBuffer = rcFrameUnit.getPicBuffer();
  m_pcPicBuffer->setUsed(); //JVT-S036 lsj

  m_cResidual.init( );
  m_cResidual.getFullPelYuvBuffer()->clear();
  m_bInitDone = true;
  
  m_cFGSIntFrame.init();
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



Void FrameUnit::setTopFieldPoc( Int iPoc )
{
  m_cTopField.setPoc( iPoc );
  m_cFrame   .setPoc( m_cBotField.isPocAvailable() ? min( m_cBotField.getPoc(), iPoc ) : iPoc );
  m_iMaxPoc   =     ( m_cBotField.isPocAvailable() ? max( m_cBotField.getPoc(), iPoc ) : iPoc );
}

Void FrameUnit::setBotFieldPoc( Int iPoc )
{
  m_cBotField.setPoc( iPoc );
  m_cFrame   .setPoc( m_cTopField.isPocAvailable() ? min( m_cTopField.getPoc(), iPoc ) : iPoc );
  m_iMaxPoc   =     ( m_cTopField.isPocAvailable() ? max( m_cTopField.getPoc(), iPoc ) : iPoc );
}




ErrVal FrameUnit::uninit()
{
  m_uiStatus = 0;
  m_bBaseRepresentation = false; //bug-fix base_rep
  m_pcPicBuffer = NULL;
  m_pcFGSPicBuffer = NULL;
  RNOK( m_cFGSFrame.uninit() );
  RNOK( m_cFGSTopField.uninit() );
  RNOK( m_cFGSBotField.uninit() );
  RNOK( m_cFGSIntFrame.uninit() );

  for( UInt uiLayerIdx = 0; uiLayerIdx < MAX_FGS_LAYERS + 1; uiLayerIdx ++ )
    RNOK( m_apcFGSRecon[uiLayerIdx]->uninit() );

  m_cMbDataCtrl.uninitFgsBQData();

  m_uiFrameNumber = 0;
  m_uiStatus      = 0;
  m_iMaxPoc       = 0;

  if( ! m_bOriginal )
  {
    RNOK( m_cMbDataCtrl.uninit() );
  }
  RNOK( m_cFrame.uninit() );
  RNOK( m_cTopField.   uninit() );
  RNOK( m_cBotField.   uninit() );
  RNOK( m_cResidual.uninit() );

  m_bInitDone = false;
  return Err::m_nOK;
}

RefPic FrameUnit::getRefPic( PicType ePicType, const RefPic& rcRefPic ) const
{
  const Frame* pcPic = getPic( ePicType );
  if( pcPic->stamp() == rcRefPic.getStamp() )
  {
    return RefPic( pcPic, pcPic->stamp() );
  }
  return RefPic( pcPic, 0 );
}

Void FrameUnit::setUnused( PicType ePicType )
{
  m_uiStatus &= ~(ePicType + ( ePicType << 2));
  getPic( ePicType )->stamp()++;
  if( ePicType==FRAME )
{
    getPic( TOP_FIELD )->stamp()++;
    getPic( BOT_FIELD )->stamp()++;
  }
  else
  {
    getPic( FRAME     )->stamp()++;
  }
}

ErrVal FrameUnit::setFGS( PicBuffer*& rpcPicBuffer )
{
  if( m_pcFGSPicBuffer == NULL )
{
    m_pcFGSPicBuffer = rpcPicBuffer;
  RNOK( m_cFGSFrame.   init( m_pcFGSPicBuffer->getBuffer(), this ) );
    RNOK( m_cFGSTopField. init( m_pcFGSPicBuffer->getBuffer(), this ) );
    RNOK( m_cFGSBotField. init( m_pcFGSPicBuffer->getBuffer(), this ) );
  m_pcFGSPicBuffer->setUsed();
    rpcPicBuffer = NULL;
  }

	getFGSIntFrame()->store( m_pcFGSPicBuffer );

  m_cFGSFrame.setPoc( m_cFrame.getPoc() );
	m_cFGSTopField.setPoc( m_cTopField.getPoc() );
	m_cFGSBotField.setPoc( m_cBotField.getPoc() );

  return Err::m_nOK;
}
Void FrameUnit::addPic( PicType ePicType, Bool bFieldCoded, UInt uiIdrPicId )
{ 
  m_eAvailable = PicType( m_eAvailable + ePicType ); 
  AOT_DBG( m_eAvailable > FRAME );
  
  m_bFieldCoded = bFieldCoded;

  ROTVS( m_ePicStruct >= PS_BOT_TOP ); // other values are set explicitly
  if( ePicType==FRAME )
  {
    m_ePicStruct = PS_FRAME;
  }
  else
  {
    if( m_eAvailable == FRAME )
    {
      m_ePicStruct = (( m_cTopField.getPoc() < m_cBotField.getPoc() ) ?  PS_TOP_BOT : PS_BOT_TOP );
    }
    else
    {
      m_ePicStruct = (( ePicType == TOP_FIELD ) ?  PS_TOP : PS_BOT );
    }
  }
}


H264AVC_NAMESPACE_END
