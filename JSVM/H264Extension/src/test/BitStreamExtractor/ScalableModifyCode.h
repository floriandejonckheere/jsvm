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
This software module was originally created for Nokia, Inc.
Author: Liu Hui (liuhui@mail.ustc.edu.cn)

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

#ifndef _SCALABLE_MODIFY_CODE_
#define _SCALABLE_MODIFY_CODE_

#include "H264AVCCommonLib.h"
#include "H264AVCCommonLib/Sei.h"

//class ScalableCodeIf
//{
//protected:
//	ScalableCodeIf()	{}
//	virtual ~ScalableCodeIf()	{}
//
//public:
//	virtual ErrVal WriteUVLC( UInt uiValue ) = 0;
//	virtual ErrVal WriteFlag( Bool bFlag ) = 0;
//	virtual ErrVal WriteCode( UInt uiValue, UInt uiLength ) = 0;
//	virtual SEICode	( h264::SEI::ScalableSei* pcScalableSei, ScalableCodeIf *pcScalableCodeIf ) = 0;
//	virtual UInt	 getNumberOfWrittenBits() = 0;
//};

class ScalableModifyCode// : public ScalableCodeIf
{
public:
	ScalableModifyCode();
	virtual ~ScalableModifyCode();

public:
	//static ErrVal Create( ScalableModifyCode* pcScalableModifyCode );
	ErrVal Destroy( Void );
	ErrVal init( ULong* pulStream );
	ErrVal WriteUVLC( UInt uiValue );
	ErrVal WriteFlag( Bool bFlag );
	ErrVal WriteCode( UInt uiValue, UInt uiLength );
	ErrVal SEICode	( h264::SEI::ScalableSei* pcScalableSei, ScalableModifyCode *pcScalableModifyCode );
	UInt	 getNumberOfWrittenBits() { return m_uiBitsWritten; }
	ErrVal Write		( UInt uiBits, UInt uiNumberOfBits );
	ErrVal WritePayloadHeader ( enum h264::SEI::MessageType eType, UInt uiSize );
	ErrVal WriteTrailingBits ();
	ErrVal WriteAlignZero ();
	ErrVal flushBuffer();
	ErrVal ConvertRBSPToPayload( UChar* m_pucBuffer, UChar pucStreamPacket[], UInt& uiBits, UInt uiHeaderBytes );

protected:
	ULong  xSwap( ULong ul )
	{
		// heiko.schwarz@hhi.fhg.de: support for BSD systems as proposed by Steffen Kamp [kamp@ient.rwth-aachen.de]
#ifdef MSYS_BIG_ENDIAN
		return ul;
#else
		UInt ul2;

		ul2  = ul>>24;
		ul2 |= (ul>>8) & 0x0000ff00;
		ul2 |= (ul<<8) & 0x00ff0000;
		ul2 |= ul<<24;

		return ul2;
#endif
	}

	BinData *m_pcBinData;
	ULong *m_pulStreamPacket;
	UInt m_uiBitCounter;
	UInt m_uiPosCounter;
	UInt m_uiDWordsLeft;
	UInt m_uiBitsWritten;
	UInt m_iValidBits;
	ULong m_ulCurrentBits;
	UInt m_uiCoeffCost;
};

#endif