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
#include "H264AVCCommonLib/Frame.h"
#include "H264AVCCommonLib/FrameUnit.h"
#include "H264AVCCommonLib/QuarterPelFilter.h"


H264AVC_NAMESPACE_BEGIN

Frame::Frame( YuvBufferCtrl& rcYuvFullPelBufferCtrl, YuvBufferCtrl& rcYuvHalfPelBufferCtrl, PicType ePicType ):
  m_cFullPelYuvBuffer( rcYuvFullPelBufferCtrl, ePicType ),
  m_cHalfPelYuvBuffer( rcYuvHalfPelBufferCtrl, ePicType ),
  m_ePicType         ( ePicType ),
  m_pcFrameUnit( NULL ),
  m_iStamp( 0 )
{
  m_bPocIsSet = false;
  m_iPoc      = 0;
}

Frame::~Frame()
{
}


ErrVal Frame::init( Pel* pucYuvBuffer, FrameUnit* pcFrameUnit )
{
  ROF( pcFrameUnit )

  m_pcFrameUnit = pcFrameUnit;
  m_bPocIsSet   = false;

  RNOK( m_cFullPelYuvBuffer.init( pucYuvBuffer ) );

  return Err::m_nOK;
}


ErrVal Frame::uninit()
{
  RNOK( m_cFullPelYuvBuffer.uninit() );
  RNOK( m_cHalfPelYuvBuffer.uninit() );

  m_bPocIsSet   = false;
  m_iPoc        = 0;
  m_pcFrameUnit = NULL;
  m_iStamp++;  // !!!

  return Err::m_nOK;
}

Bool Frame::isShortTerm() const
{
  return m_pcFrameUnit->isShortTerm( m_ePicType );
}

const Bool Frame::isUsed() const
{
	return m_pcFrameUnit->isUsed( m_ePicType );
}

ErrVal
Frame::extendFrame( QuarterPelFilter* pcQuarterPelFilter, Bool bFrameMbsOnlyFlag, Bool bFGS )
{
	Bool bNoHalfPel = ( NULL == pcQuarterPelFilter );

	// perform border padding on the full pel buffer
  RNOK( getFullPelYuvBuffer()->fillMargin() );

	ROF( m_pcFrameUnit );
	if( m_ePicType==FRAME )
	{
	 Frame* pcTopField = NULL;
	 Frame* pcBotField = NULL;

	 if( bFGS )
	 {
		 pcTopField = getFrameUnit()->getFGSPic( TOP_FIELD );
		 pcBotField = getFrameUnit()->getFGSPic( BOT_FIELD );
}
	 else
	 {
	   pcTopField = getFrameUnit()->getPic   ( TOP_FIELD );
	   pcBotField = getFrameUnit()->getPic   ( BOT_FIELD );
	 }

		// speed up only
		if( ! bFrameMbsOnlyFlag )
		{
			// remove the default yuv memory from buffers
			RNOK( pcTopField->getFullPelYuvBuffer()->uninit() );
			RNOK( pcBotField->getFullPelYuvBuffer()->uninit() );

			// creates private full pel buffer
			Pel* pData = NULL;
			RNOK( pcTopField->getFullPelYuvBuffer()->init( pData ) );
			RNOK( pcBotField->getFullPelYuvBuffer()->init( pData ) );

			// perform border padding on the full pel buffer
			RNOK( pcTopField->getFullPelYuvBuffer()->loadBufferAndFillMargin( getFullPelYuvBuffer() ) );
			RNOK( pcBotField->getFullPelYuvBuffer()->loadBufferAndFillMargin( getFullPelYuvBuffer() ) );
		}

		// if cond is true no sub pel buffer is used
		ROTRS( bNoHalfPel, Err::m_nOK );

		// creates private half pel buffer
		Pel* pHPData = NULL;
		RNOK( getHalfPelYuvBuffer()->init( pHPData ) );

		// create half pel samples
		ANOK( pcQuarterPelFilter->filterFrame( getFullPelYuvBuffer(), getHalfPelYuvBuffer() ) );

		// speed up only
		if( ! bFrameMbsOnlyFlag )
		{
			// creates private half pel buffer
			pHPData = NULL;
			RNOK( pcTopField->getHalfPelYuvBuffer()->init( pHPData ) );
			RNOK( pcBotField->getHalfPelYuvBuffer()->init( pHPData ) );

			// create half pel samples
			ANOK( pcQuarterPelFilter->filterFrame( pcTopField->getFullPelYuvBuffer(), pcTopField->getHalfPelYuvBuffer() ) );
			ANOK( pcQuarterPelFilter->filterFrame( pcBotField->getFullPelYuvBuffer(), pcBotField->getHalfPelYuvBuffer() ) );
		}
	}
	else
	{
    AOT( bFGS );
		Frame* pcFramePic = getFrameUnit()->getPic( FRAME );

		if( pcFramePic->getFullPelYuvBuffer()->isValid() )
		{
			// first field
			// remove the default yuv memory from buffers
			RNOK( pcFramePic->getFullPelYuvBuffer()->uninit() );

			// if cond is true no sub pel buffer is used
			ROTRS( bNoHalfPel, Err::m_nOK );

			Frame* pcTopField = getFrameUnit()->getPic( TOP_FIELD );
			Frame* pcBotField = getFrameUnit()->getPic( BOT_FIELD );

			// creates private half pel buffer
			Pel* pHPData = NULL;
			RNOK( pcTopField->getHalfPelYuvBuffer()->init( pHPData ) );
			RNOK( pcBotField->getHalfPelYuvBuffer()->init( pHPData ) );

			// create half pel samples
			ANOK( pcQuarterPelFilter->filterFrame( getFullPelYuvBuffer(), getHalfPelYuvBuffer() ) );
		}
		else
{
			// second field
			// creates private full pel buffer
			Pel* pData = NULL;
			RNOK( pcFramePic->getFullPelYuvBuffer()->init( pData ) );

			// perform border padding on the full pel buffer
			RNOK( pcFramePic->getFullPelYuvBuffer()->loadBufferAndFillMargin( getFullPelYuvBuffer() ) );

			// if cond is true no sub pel buffer is used
			ROTRS( bNoHalfPel, Err::m_nOK );

			// creates private half pel buffer
			Pel* pHPData = NULL;
			RNOK( pcFramePic->getHalfPelYuvBuffer()->init( pHPData ) );

			// create half pel samples
			ANOK( pcQuarterPelFilter->filterFrame( pcFramePic->getFullPelYuvBuffer(), pcFramePic->getHalfPelYuvBuffer() ) );
			ANOK( pcQuarterPelFilter->filterFrame(             getFullPelYuvBuffer(),             getHalfPelYuvBuffer() ) );
		}
}

	return Err::m_nOK;
}

H264AVC_NAMESPACE_END
