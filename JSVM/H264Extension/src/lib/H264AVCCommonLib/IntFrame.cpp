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
#include "H264AVCCommonLib/IntFrame.h"
#include "H264AVCCommonLib/QuarterPelFilter.h"

H264AVC_NAMESPACE_BEGIN

IntFrame::IntFrame( YuvBufferCtrl& rcYuvFullPelBufferCtrl, YuvBufferCtrl& rcYuvHalfPelBufferCtrl, PicType ePicType )
: m_cFullPelYuvBuffer     ( rcYuvFullPelBufferCtrl, ePicType ),
  m_cHalfPelYuvBuffer     ( rcYuvHalfPelBufferCtrl, ePicType ),
  m_ePicType              ( ePicType ),
  m_bHalfPel              ( false ),
  m_bExtended             ( false ),
	m_bPocIsSet             ( false ),
	m_iPoc                  ( 0 ),
  m_pcIntFrameTopField    ( NULL ),
  m_pcIntFrameBotField    ( NULL ),
  m_uiFrameIdInGop        ( 0 ),
  m_pcDPBUnit             ( NULL ),
  m_bUnusedForRef       ( false), // JVT-Q065 EIDR
  m_piChannelDistortion   ( 0 )     // JVT-R057 LA-RDO
{
}

IntFrame::~IntFrame()
{
}

ErrVal IntFrame::create( IntFrame*& rpcIntFrame, YuvBufferCtrl& rcYuvFullPelBufferCtrl, YuvBufferCtrl& rcYuvHalfPelBufferCtrl, PicType ePicType )
{
  rpcIntFrame = new IntFrame( rcYuvFullPelBufferCtrl, rcYuvHalfPelBufferCtrl, ePicType );
  ROT( NULL == rpcIntFrame );
  return Err::m_nOK;
}

ErrVal IntFrame::destroy()
{
  delete this;
  return Err::m_nOK;
}

ErrVal IntFrame::init()
{
  ASSERT( m_ePicType==FRAME );
  XPel* pData = 0;
  RNOK( getFullPelYuvBuffer()->init( pData ) );

	m_bPocIsSet = false;
  m_bExtended = false;
	m_bHalfPel  = false;
  return Err::m_nOK;
}

ErrVal IntFrame::initHalfPel( XPel*& rpucYuvBuffer )
{
  RNOK( getHalfPelYuvBuffer()->init( rpucYuvBuffer ) );
    m_bHalfPel = true;
  return Err::m_nOK;
}

ErrVal IntFrame::initHalfPel( )
  {
    XPel* pHPData = 0;
    return initHalfPel(pHPData);
  }


ErrVal IntFrame::uninit()
{
  if( m_ePicType==FRAME && NULL != m_pcIntFrameTopField )
  {
		// remove the default yuv memory from buffers
    RNOK( m_pcIntFrameTopField->uninit () );
		RNOK( m_pcIntFrameTopField->destroy() );
    m_pcIntFrameTopField = NULL;
  }

  if( m_ePicType==FRAME && NULL != m_pcIntFrameBotField )
  {
		// remove the default yuv memory from buffers
    RNOK( m_pcIntFrameBotField->uninit () );
		RNOK( m_pcIntFrameBotField->destroy() );
    m_pcIntFrameBotField = NULL;
  }

	// remove the default yuv memory from buffers
	RNOK( getFullPelYuvBuffer()->uninit() );
  RNOK( getHalfPelYuvBuffer()->uninit() );

	m_bPocIsSet = false;
  m_bHalfPel  = false;
  m_bExtended = false;

  return Err::m_nOK;
}

ErrVal IntFrame::uninitHalfPel()
{
  RNOK( getHalfPelYuvBuffer()->uninit() );

	m_bHalfPel  = false;
	m_bExtended = false;

  if( m_ePicType==FRAME && NULL != m_pcIntFrameTopField )
  {
    RNOK( m_pcIntFrameTopField->uninitHalfPel() );
  }
  if( m_ePicType==FRAME && NULL != m_pcIntFrameBotField )
  {
    RNOK( m_pcIntFrameBotField->uninitHalfPel() );
  }

	return Err::m_nOK;
}

ErrVal IntFrame::load( PicBuffer* pcPicBuffer )
{
  ASSERT( m_ePicType==FRAME );
  RNOK( getFullPelYuvBuffer()->loadFromPicBuffer( pcPicBuffer ) );
  return Err::m_nOK;
}

ErrVal IntFrame::store( PicBuffer* pcPicBuffer )
{
  ASSERT( m_ePicType==FRAME );
  RNOK( getFullPelYuvBuffer()->storeToPicBuffer( pcPicBuffer ) );
  return Err::m_nOK;
}

IntFrame* IntFrame::getPic( PicType ePicType )
{
  ASSERT( m_ePicType==FRAME );
	switch( ePicType )
	{
	case FRAME:
		return this;
		break;
	case TOP_FIELD:
		ASSERT( m_pcIntFrameTopField != NULL );
		return m_pcIntFrameTopField;
		break;
	case BOT_FIELD:
		ASSERT( m_pcIntFrameBotField != NULL );
		return m_pcIntFrameBotField;
		break;
	default:
		return NULL;
		break;
	}
	return NULL;
}

ErrVal IntFrame::removeFieldBuffer( PicType ePicType )
{
  ASSERT( m_ePicType==FRAME );

  if( ePicType==TOP_FIELD )
  {
		if( NULL != m_pcIntFrameTopField )
    {
      // remove the default yuv memory from buffers
      RNOK( m_pcIntFrameTopField->uninit () );
      RNOK( m_pcIntFrameTopField->destroy() );
      m_pcIntFrameTopField = NULL;
    }
  }
  else if( ePicType==BOT_FIELD )
  {
    if( NULL != m_pcIntFrameBotField )
    {
      // remove the default yuv memory from buffers
      RNOK( m_pcIntFrameBotField->uninit () );
      RNOK( m_pcIntFrameBotField->destroy() );
      m_pcIntFrameBotField = NULL;
    }
  }

  return Err::m_nOK;
}

ErrVal IntFrame::removeFrameFieldBuffer()
{
  ASSERT( m_ePicType==FRAME );

  RNOK( removeFieldBuffer( TOP_FIELD ) );
  RNOK( removeFieldBuffer( BOT_FIELD ) );
  
  return Err::m_nOK;
}

ErrVal IntFrame::addFieldBuffer( PicType ePicType )
{
  ASSERT( m_ePicType==FRAME );

  if( ePicType==FRAME )
{
  return Err::m_nOK;
}
  if( ePicType == TOP_FIELD )
  {
    if( NULL != m_pcIntFrameTopField )
		{
			RNOK( m_pcIntFrameTopField->uninit() );
		}
		if( NULL == m_pcIntFrameTopField )
    {
			YuvBufferCtrl& rcYuvFullPelBufferCtrl = getFullPelYuvBuffer()->getBufferCtrl();
			YuvBufferCtrl& rcYuvHalfPelBufferCtrl = getHalfPelYuvBuffer()->getBufferCtrl();

			RNOK( IntFrame::create( m_pcIntFrameTopField, rcYuvFullPelBufferCtrl, rcYuvHalfPelBufferCtrl, TOP_FIELD ) );
		}

    // creates private full pel buffer
    XPel* pData = getFullPelYuvBuffer()->getBuffer();
    RNOK( m_pcIntFrameTopField->getFullPelYuvBuffer()->init( pData ) );

    m_pcIntFrameTopField->setPoc( m_iPoc );
  }
  else if( ePicType == BOT_FIELD )
  {
		if( NULL != m_pcIntFrameBotField )
		{
			RNOK( m_pcIntFrameBotField->uninit() );
		}
		if( NULL == m_pcIntFrameBotField )
{
			YuvBufferCtrl& rcYuvFullPelBufferCtrl = getFullPelYuvBuffer()->getBufferCtrl();
			YuvBufferCtrl& rcYuvHalfPelBufferCtrl = getHalfPelYuvBuffer()->getBufferCtrl();

			RNOK( IntFrame::create( m_pcIntFrameBotField, rcYuvFullPelBufferCtrl, rcYuvHalfPelBufferCtrl, BOT_FIELD ) );
		}

    // creates private full pel buffer
    XPel* pData = getFullPelYuvBuffer()->getBuffer();
    RNOK( m_pcIntFrameBotField->getFullPelYuvBuffer()->init( pData ) );

    m_pcIntFrameBotField->setPoc( m_iPoc+1 );
  }

  return Err::m_nOK;
}

ErrVal IntFrame::addFrameFieldBuffer()
{
  ASSERT( m_ePicType==FRAME );

  RNOK( addFieldBuffer( TOP_FIELD ) );
  RNOK( addFieldBuffer( BOT_FIELD ) );

  return Err::m_nOK;
}

ErrVal IntFrame::extendFrame( QuarterPelFilter* pcQuarterPelFilter, PicType ePicType, Bool bFrameMbsOnlyFlag )
{
	ASSERT( m_ePicType==FRAME );

	const Bool bNoHalfPel = ( NULL == pcQuarterPelFilter );

  // copy the second field content into the frame buffer
  /*if( ePicType==TOP_FIELD && m_pcIntFrameTopField != NULL )
  {
    //RNOK( getFullPelYuvBuffer()->loadBuffer( m_pcIntFrameTopField->getFullPelYuvBuffer() ) );
  }
  if( ePicType==BOT_FIELD && m_pcIntFrameBotField != NULL )
  {
    //RNOK( getFullPelYuvBuffer()->loadBuffer( m_pcIntFrameBotField->getFullPelYuvBuffer() ) );
  }*/

	if( NULL != m_pcIntFrameTopField || NULL != m_pcIntFrameBotField )
{
    RNOK( removeFrameFieldBuffer() );
  }

	// perform border padding on the full pel buffer
  RNOK( getFullPelYuvBuffer()->fillMargin( ) );
  m_bExtended     = true;
  
	if( ! bFrameMbsOnlyFlag )
	{
		if( ePicType==FRAME )
		{
			if( NULL == m_pcIntFrameTopField || NULL == m_pcIntFrameBotField )
			{
				ROT( NULL != m_pcIntFrameTopField );
				ROT( NULL != m_pcIntFrameTopField );
				YuvBufferCtrl& rcYuvFullPelBufferCtrl = getFullPelYuvBuffer()->getBufferCtrl();
				YuvBufferCtrl& rcYuvHalfPelBufferCtrl = getHalfPelYuvBuffer()->getBufferCtrl();

				RNOK( IntFrame::create( m_pcIntFrameTopField, rcYuvFullPelBufferCtrl, rcYuvHalfPelBufferCtrl, TOP_FIELD ) );
				RNOK( IntFrame::create( m_pcIntFrameBotField, rcYuvFullPelBufferCtrl, rcYuvHalfPelBufferCtrl, BOT_FIELD ) );

				// creates private full pel buffer
				XPel* pData = NULL;
				RNOK( m_pcIntFrameTopField->getFullPelYuvBuffer()->init( pData ) );
				RNOK( m_pcIntFrameBotField->getFullPelYuvBuffer()->init( pData ) );
			}
		}
		else
		{
			RNOK( addFrameFieldBuffer() );
		}

  // perform border padding on the full pel buffer
		RNOK( m_pcIntFrameTopField->getFullPelYuvBuffer()->loadBufferAndFillMargin( getFullPelYuvBuffer() ) );
		RNOK( m_pcIntFrameBotField->getFullPelYuvBuffer()->loadBufferAndFillMargin( getFullPelYuvBuffer() ) );
		m_pcIntFrameTopField->setExtended();
		m_pcIntFrameBotField->setExtended();

		if( ! bNoHalfPel )
		{
			XPel* pHPData = NULL;
			RNOK( m_pcIntFrameTopField->initHalfPel( pHPData ) );
			RNOK( m_pcIntFrameBotField->initHalfPel( pHPData ) );
		}
	}

  // if cond is true no sub pel buffer is used
  ROTRS( bNoHalfPel, Err::m_nOK );

  // create half pel samples
  RNOK( pcQuarterPelFilter->filterFrame(                         getFullPelYuvBuffer(),                       getHalfPelYuvBuffer() ) );

	if( ! bFrameMbsOnlyFlag )
	{
		RNOK( pcQuarterPelFilter->filterFrame( m_pcIntFrameTopField->getFullPelYuvBuffer(), m_pcIntFrameTopField->getHalfPelYuvBuffer() ) );
		RNOK( pcQuarterPelFilter->filterFrame( m_pcIntFrameBotField->getFullPelYuvBuffer(), m_pcIntFrameBotField->getHalfPelYuvBuffer() ) );
	}

  return Err::m_nOK;
}

Void IntFrame::setTopFieldPoc( Int iPoc )
{
	ASSERT( m_ePicType==FRAME );
	if( NULL != m_pcIntFrameTopField && NULL != m_pcIntFrameBotField )
	{
	  m_pcIntFrameTopField->setPoc( iPoc );
		setPoc( m_pcIntFrameBotField->isPocAvailable() ? max( m_pcIntFrameBotField->getPoc(), iPoc ) : iPoc );
	}
}

Void IntFrame::setBotFieldPoc( Int iPoc )
{
	ASSERT( m_ePicType==FRAME );
	if( NULL != m_pcIntFrameTopField && NULL != m_pcIntFrameBotField )
	{
		m_pcIntFrameBotField->setPoc( iPoc );
		setPoc( m_pcIntFrameTopField->isPocAvailable() ? min( m_pcIntFrameTopField->getPoc(), iPoc ) : iPoc );
	}
}

// JVT-R057 LA-RDO}
Void IntFrame::initChannelDistortion()
{
	if(!m_piChannelDistortion)
	{
		UInt  uiMbY  = getFullPelYuvBuffer()->getLHeight()/4;
		UInt  uiMbX  = getFullPelYuvBuffer()->getLWidth()/4;
		UInt  uiSize = uiMbX*uiMbY;
		m_piChannelDistortion= new UInt[uiSize];
	}
}

Void IntFrame::copyChannelDistortion(IntFrame*p1)
{
	UInt  uiMbY  = getFullPelYuvBuffer()->getLHeight()/16;
	UInt  uiMbX  = getFullPelYuvBuffer()->getLWidth()/16;
	for(UInt y=0;y<uiMbY*4;y++)
	{
		for(UInt x=0;x<uiMbX*4;x++)
		{ 
			m_piChannelDistortion[y*(uiMbX*4)+x]=p1->m_piChannelDistortion[y*(uiMbX*4)+x];
		}
	}
}

Void IntFrame::zeroChannelDistortion()
{
	UInt  uiMbY  = getFullPelYuvBuffer()->getLHeight()/16;
	UInt  uiMbX  = getFullPelYuvBuffer()->getLWidth()/16;
	for(UInt y=0;y<uiMbY*4;y++)
	{
		for(UInt x=0;x<uiMbX*4;x++)
		{ 
			m_piChannelDistortion[y*(uiMbX*4)+x]=0;
		}
	}
}

// JVT-R057 LA-RDO}

H264AVC_NAMESPACE_END
