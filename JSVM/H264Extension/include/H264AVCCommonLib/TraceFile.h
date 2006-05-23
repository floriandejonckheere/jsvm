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




#if !defined(AFX_TRACEFILE_H__B87E26CF_023E_4DC7_8F94_D3E38F59ABA1__INCLUDED_)
#define AFX_TRACEFILE_H__B87E26CF_023E_4DC7_8F94_D3E38F59ABA1__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



#define ENCODER_TRACE     0
#define DECODER_TRACE     0

#define MAX_LINE_LENGTH 255


H264AVC_NAMESPACE_BEGIN


class H264AVCCOMMONLIB_API TraceFile
{
public:
	TraceFile         ();
	virtual ~TraceFile();

  static ErrVal initTrace   ();
  static ErrVal openTrace   ( Char* pucBaseFilename );
  static ErrVal closeTrace  ();
  static ErrVal setLayer    ( UInt  uiLayerId );


  static ErrVal startNalUnit();
  static ErrVal startFrame  ();
  static ErrVal startSlice  ();
  static ErrVal startMb     ( Int   iMbAddress  );

  static ErrVal printHeading( Char* pcString    );

  static ErrVal countBits   ( UInt  uiBitCount );
  static ErrVal printPos    ();

  static ErrVal printString ( Char* pcString );
  static ErrVal printVal    ( UInt  uiVal );
  static ErrVal printVal    ( Int   iVal );
  static ErrVal printXVal   ( UInt  uiVal );

  static ErrVal addBits     ( UInt  uiVal, UInt uiLength );
  static ErrVal printBits   ( UInt  uiVal, UInt uiLength );
  static ErrVal printCode   ( UInt  uiVal );
  static ErrVal printCode   ( Int   iVal );
  static ErrVal printType   ( Char* pcString);

  static ErrVal newLine();

protected:
  static UInt  sm_uiLayer;
  static FILE* sm_fTrace      [MAX_LAYERS];
  static UInt  sm_uiFrameNum  [MAX_LAYERS];
  static UInt  sm_uiSliceNum  [MAX_LAYERS];
  static UInt  sm_uiPosCounter[MAX_LAYERS];
  static Char  sm_acLine      [MAX_LINE_LENGTH];
  static Char  sm_acType      [9];
  static Char  sm_acPos       [9];
  static Char  sm_acCode      [6];
  static Char  sm_acBits      [35];
};



H264AVC_NAMESPACE_END




#if ENCODER_TRACE
  #define INIT_ETRACE      if( m_bTraceEnable ) TraceFile::initTrace   ()
  #define OPEN_ETRACE      if( m_bTraceEnable ) TraceFile::openTrace   ("TraceEncoder")
  #define CLOSE_ETRACE     if( m_bTraceEnable ) TraceFile::closeTrace  ()
  
  #define ETRACE_LAYER(x)  if( m_bTraceEnable ) TraceFile::setLayer    (x) 
  #define ETRACE_NEWFRAME  if( m_bTraceEnable ) TraceFile::startFrame  ()
  #define ETRACE_NEWSLICE  if( m_bTraceEnable ) TraceFile::startSlice  ()
  #define ETRACE_NEWMB(x)  if( m_bTraceEnable ) TraceFile::startMb     (x)
  #define ETRACE_HEADER(x)                      TraceFile::printHeading(x)

  #define ETRACE_POS       if( m_bTraceEnable ) TraceFile::printPos    ()
  #define ETRACE_COUNT(i)  if( m_bTraceEnable ) TraceFile::countBits   (i)

  #define ETRACE_BITS(v,l) if( m_bTraceEnable ) TraceFile::addBits     (v,l)
  #define ETRACE_CODE(v)   if( m_bTraceEnable ) TraceFile::printCode   (v)

  #define ETRACE_TH(t)     if( m_bTraceEnable ) TraceFile::printString (t)
  #define ETRACE_T(t)      if( m_bTraceEnable ) TraceFile::printString (t)
  #define ETRACE_TY(t)     if( m_bTraceEnable ) TraceFile::printType   (t)
  #define ETRACE_V(t)      if( m_bTraceEnable ) TraceFile::printVal    (t)
  #define ETRACE_X(t)      if( m_bTraceEnable ) TraceFile::printXVal   (t)

  #define ETRACE_N         if( m_bTraceEnable ) TraceFile::newLine     ()
  #define ETRACE_DO(x)     if( m_bTraceEnable ) x
  #define ETRACE_DECLARE(x) x
#else

  #define OPEN_ETRACE
  #define INIT_ETRACE
  #define CLOSE_ETRACE

  #define ETRACE_LAYER(x)
  #define ETRACE_NEWFRAME
  #define ETRACE_NEWSLICE
  #define ETRACE_NEWMB(x)
  #define ETRACE_HEADER(x)

  #define ETRACE_POS
  #define ETRACE_COUNT(i)

  #define ETRACE_BITS(v,l)
  #define ETRACE_CODE(v)

  #define ETRACE_TH(t)
  #define ETRACE_T(t)
  #define ETRACE_TY(t)
  #define ETRACE_V(t)
  #define ETRACE_X(t)

  #define ETRACE_N
  #define ETRACE_DO(x)
  #define ETRACE_DECLARE(x) 
#endif

#if DECODER_TRACE
  #define INIT_DTRACE      TraceFile::initTrace   ()
  #define OPEN_DTRACE      TraceFile::openTrace   ("TraceDecoder")
  #define CLOSE_DTRACE     TraceFile::closeTrace  ()
  
  #define DTRACE_LAYER(x)  TraceFile::setLayer    (x) 
  #define DTRACE_NEWFRAME  TraceFile::startFrame  ()
  #define DTRACE_NEWSLICE  TraceFile::startSlice  ()
  #define DTRACE_NEWMB(x)  TraceFile::startMb     (x)
  #define DTRACE_HEADER(x) TraceFile::printHeading(x)

  #define DTRACE_POS       TraceFile::printPos    ()
  #define DTRACE_COUNT(i)  TraceFile::countBits   (i)

  #define DTRACE_BITS(v,l) TraceFile::addBits     (v,l)
  #define DTRACE_CODE(v)   TraceFile::printCode   (v)

  #define DTRACE_TH(t)     TraceFile::printString (t)
  #define DTRACE_T(t)      TraceFile::printString (t)
  #define DTRACE_TY(t)     TraceFile::printType   (t)
  #define DTRACE_V(t)      TraceFile::printVal    (t)
  #define DTRACE_X(t)      TraceFile::printXVal   (t)

  #define DTRACE_N         TraceFile::newLine     ()
  #define DTRACE_DO(x)     x
#else
  #define OPEN_DTRACE
  #define INIT_DTRACE
  #define CLOSE_DTRACE

  #define DTRACE_LAYER(x)
  #define DTRACE_NEWFRAME
  #define DTRACE_NEWSLICE
  #define DTRACE_NEWMB(x)
  #define DTRACE_HEADER(x)

  #define DTRACE_POS
  #define DTRACE_COUNT(i)

  #define DTRACE_BITS(v,l)
  #define DTRACE_CODE(v)

  #define DTRACE_TH(t)
  #define DTRACE_T(t)
  #define DTRACE_TY(t)
  #define DTRACE_V(t)
  #define DTRACE_X(t)

  #define DTRACE_N
  #define DTRACE_DO(x)
#endif

#endif // !defined(AFX_TRACEFILE_H__B87E26CF_023E_4DC7_8F94_D3E38F59ABA1__INCLUDED_)
