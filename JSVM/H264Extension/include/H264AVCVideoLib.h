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
// H264AVCVideoLib.h: control file for precompiled header usage
//
// This file must not be used for any implementation stuff.
// This file is used in the project settings of the library's .dsp files
// to control the usage of pre-compiled headers in the Visual Studio.
// It can be seen quite similar as StdAfx.h usage in Microsoft projects.
//
// IT NEEDS TO BE INCLUDED AS THE VERY FIST INCLUDE IN EVERY .cpp FILE
// OF YOUR LIBRARY!!! 
// 
// Include equivalent files of other libraries if you intend to use other
// libs as you can see it here for MSysLib.h 
//////////////////////////////////////////////////////////////////////


#ifndef __H264AVCVIDEOLIB_H_D64BE9B4_A8DA_11D3_AFE7_005004464B79
#define __H264AVCVIDEOLIB_H_D64BE9B4_A8DA_11D3_AFE7_005004464B79


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
//#define _ATL_APARTMENT_THREADED
#define _ATL_FREE_THREADED


#include <atlbase.h>
//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
extern CComModule _Module;
#include <atlcom.h>
#include <atlwin.h>
#include <atlctl.h>


#include "MSysLib.h"
#include "CommonCodecLib.h"
#include "H264AVCEncoderLib.h"
#include "H264AVCDecoderLib.h"

#if defined( MSYS_WIN32 )
  #if defined( H264AVCVIDEOLIB_EXPORTS )
    #define H264AVCVIDEOLIB_API __declspec(dllexport)
  #else
    #if !defined( H264AVCVIDEOLIB_LIB )
      #define H264AVCVIDEOLIB_API __declspec(dllimport)
    #else
      #define H264AVCVIDEOLIB_API
    #endif
  #endif
#elif defined( MSYS_LINUX )
  #define H264AVCVIDEOLIB_API
#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

//begin of WTL header file section
//remember to use namespace WTL
#include <atlapp.h>
#include <atlctrls.h>
#include <atlctrlx.h>
//end of WTL header file section


#include "MSysLib.h"
#include "MediaPacketLib.h"
#include "CommonCodecLib.h"

#include <string>
#include <sstream>


#endif //__H264AVCVIDEOLIB_H_D64BE9B4_A8DA_11D3_AFE7_005004464B79
