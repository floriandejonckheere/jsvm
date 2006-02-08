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




#ifndef __MSYS_TYPEDEFS_H_D64BE9B4_A8DA_11D3_AFE7_005004464B79
#define __MSYS_TYPEDEFS_H_D64BE9B4_A8DA_11D3_AFE7_005004464B79

#include <limits.h>



#if defined( MSYS_WIN32 )
#if defined( _CHAR_UNSIGNED )
#define MSYS_CHAR_UNSIGNED
#endif
#elif defined( MSYS_LINUX )
#if defined( __CHAR_UNSIGNED__ )
#define MSYS_CHAR_UNSIGNED
#endif
#endif

#if !defined( MSYS_NO_NAMESPACE_MSYS )
# define MSYS_NAMESPACE_BEGIN     namespace msys {
# define MSYS_NAMESPACE_END       }
#else
# define MSYS_NO_DIRECTIVE_USING_NAMESPACE_MSYS
# define MSYS_NAMESPACE_BEGIN
# define MSYS_NAMESPACE_END
#endif

// begin namespace msys

#if !(defined MSYS_TYPE_VOID)
  #define MSYS_TYPE_VOID void
#endif
#if !(defined MSYS_NO_TYPE_VOID )
  typedef MSYS_TYPE_VOID Void;
#endif

#if !(defined MSYS_TYPE_BOOL)
  #define MSYS_TYPE_BOOL bool
#endif
#if !(defined MSYS_NO_TYPE_BOOL )
  typedef MSYS_TYPE_BOOL Bool;
#endif

#if !(defined MSYS_TYPE_SCHAR)
  #define MSYS_TYPE_SCHAR signed char
#endif
#if !(defined MSYS_NO_TYPE_SCHAR )
  typedef MSYS_TYPE_SCHAR SChar;
#endif

#if !(defined MSYS_TYPE_CHAR)
  #define MSYS_TYPE_CHAR char
#endif
#if !(defined MSYS_NO_TYPE_CHAR )
  typedef MSYS_TYPE_CHAR Char;
#endif

#if !(defined MSYS_TYPE_UCHAR)
  #define MSYS_TYPE_UCHAR unsigned char
#endif
#if !(defined MSYS_NO_TYPE_UCHAR )
  typedef MSYS_TYPE_UCHAR UChar;
#endif

#if !(defined MSYS_TYPE_INT8)
  #define MSYS_TYPE_INT8 signed char
#endif
#if !(defined MSYS_NO_TYPE_INT8 )
  typedef MSYS_TYPE_INT8 Int8;
#endif

#if !(defined MSYS_TYPE_UINT8)
  #define MSYS_TYPE_UINT8 unsigned char
#endif
#if !(defined MSYS_NO_TYPE_UINT8 )
  typedef MSYS_TYPE_UINT8 UInt8;
#endif

#if !(defined MSYS_TYPE_SHORT)
  #define MSYS_TYPE_SHORT short
#endif
#if !(defined MSYS_NO_TYPE_SHORT )
  typedef MSYS_TYPE_SHORT Short;
#endif

#if !(defined MSYS_TYPE_USHORT)
  #define MSYS_TYPE_USHORT unsigned short
#endif
#if !(defined MSYS_NO_TYPE_USHORT )
  typedef MSYS_TYPE_USHORT UShort;
#endif

#if !(defined MSYS_TYPE_INT16)
  #define MSYS_TYPE_INT16 short
#endif
#if !(defined MSYS_NO_TYPE_INT16 )
  typedef MSYS_TYPE_INT16 Int16;
#endif

#if !(defined MSYS_TYPE_UINT16)
  #define MSYS_TYPE_UINT16 unsigned short
#endif
#if !(defined MSYS_NO_TYPE_UINT16 )
  typedef MSYS_TYPE_UINT16 UInt16;
#endif

#if !(defined MSYS_TYPE_INT)
  #define MSYS_TYPE_INT int
#endif
#if !(defined MSYS_NO_TYPE_INT )
  typedef MSYS_TYPE_INT Int;
#endif

#if !(defined MSYS_TYPE_UINT)
  #define MSYS_TYPE_UINT unsigned int
#endif
#if !(defined MSYS_NO_TYPE_UINT )
  typedef MSYS_TYPE_UINT UInt;
#endif

#if !(defined MSYS_TYPE_INT32)
  #define MSYS_TYPE_INT32 int
#endif
#if !(defined MSYS_NO_TYPE_INT32 )
  typedef MSYS_TYPE_INT32 Int32;
#endif

#if !(defined MSYS_TYPE_UINT32)
  #define MSYS_TYPE_UINT32 unsigned int
#endif
#if !(defined MSYS_NO_TYPE_UINT32 )
  typedef MSYS_TYPE_UINT32 UInt32;
#endif

#if !(defined MSYS_TYPE_LONG)
  #define MSYS_TYPE_LONG long
#endif
#if !(defined MSYS_NO_TYPE_LONG )
  typedef MSYS_TYPE_LONG Long;
#endif

#if !(defined MSYS_TYPE_ULONG)
  #define MSYS_TYPE_ULONG unsigned long
#endif
#if !(defined MSYS_NO_TYPE_ULONG )
  typedef MSYS_TYPE_ULONG ULong;
#endif


#if defined( MSYS_WIN32 ) || defined( WIN32 )
  #if !(defined MSYS_TYPE_INT64)
    #define MSYS_TYPE_INT64 __int64
  #endif
  #if !(defined MSYS_NO_TYPE_INT64 )
    typedef MSYS_TYPE_INT64 Int64;
  #endif
  
  #if !(defined MSYS_TYPE_UINT64)
    #define MSYS_TYPE_UINT64 unsigned __int64
  #endif
  #if !(defined MSYS_NO_TYPE_UINT64 )
    typedef MSYS_TYPE_UINT64 UInt64;
  #endif
#elif defined( MSYS_LINUX )
  #if !(defined MSYS_TYPE_INT64)
    #define MSYS_TYPE_INT64 long long int
  #endif
  #if !(defined MSYS_NO_TYPE_INT64 )
    typedef MSYS_TYPE_INT64 Int64;
  #endif

  #if !(defined MSYS_TYPE_UINT64)
    #define MSYS_TYPE_UINT64 unsigned long long int
  #endif
  #if !(defined MSYS_NO_TYPE_UINT64 )
    typedef MSYS_TYPE_UINT64 UInt64;
  #endif
#endif


#if !(defined MSYS_TYPE_FLOAT)
  #define MSYS_TYPE_FLOAT float
#endif
#if !(defined MSYS_NO_TYPE_FLOAT )
  typedef MSYS_TYPE_FLOAT Float;
#endif

#if !(defined MSYS_TYPE_DOUBLE)
  #define MSYS_TYPE_DOUBLE double
#endif
#if !(defined MSYS_NO_TYPE_DOUBLE )
  typedef MSYS_TYPE_DOUBLE Double;
#endif


#define MSYS_CHAR_BIT		      8

#define MSYS_SCHAR_MIN		    (-128)
#define MSYS_SCHAR_MAX		    127
#define MSYS_UCHAR_MAX		    0xFF
#define MSYS_INT8_MIN		      MSYS_SCHAR_MIN
#define MSYS_INT8_MAX		      MSYS_SCHAR_MAX
#define MSYS_UINT8_MAX		    MSYS_UCHAR_MAX

#if defined( MSYS_CHAR_UNSIGNED )
#define MSYS_CHAR_MIN		      0
#define MSYS_CHAR_MAX		      MSYS_UCHAR_MAX
#else
#define MSYS_CHAR_MIN		      MSYS_SCHAR_MIN
#define MSYS_CHAR_MAX		      MSYS_SCHAR_MAX
#endif

#define MSYS_SHORT_MIN		    (-32768)
#define MSYS_SHORT_MAX		    32767
#define MSYS_USHORT_MAX		    0xFFFF
#define MSYS_INT16_MIN		    MSYS_SHORT_MIN
#define MSYS_INT16_MAX		    MSYS_SHORT_MAX
#define MSYS_UINT16_MAX		    MSYS_USHORT_MAX

#define MSYS_INT_MIN		      (-2147483647 - 1)
#define MSYS_INT_MAX		      2147483647
#define MSYS_UINT_MAX		      0xFFFFFFFFU
#define MSYS_INT32_MIN		    MSYS_INT_MIN
#define MSYS_INT32_MAX		    MSYS_INT_MAX
#define MSYS_UINT32_MAX		    MSYS_UINT_MAX

#define MSYS_LONG_MIN		      (-2147483647L - 1L)
#define MSYS_LONG_MAX		      2147483647L
#define MSYS_ULONG_MAX		    0xFFFFFFFFUL

#if defined( MSYS_WIN32 ) || defined( WIN32 )
#define MSYS_INT64_MIN		    (-9223372036854775807i64 - 1i64)
#define MSYS_INT64_MAX		    9223372036854775807i64
#define MSYS_UINT64_MAX		    0xFFFFFFFFFFFFFFFFui64
#elif defined( MSYS_LINUX )
#define MSYS_INT64_MIN		    (-9223372036854775807LL - 1LL)
#define MSYS_INT64_MAX		    9223372036854775807LL
#define MSYS_UINT64_MAX		    0xFFFFFFFFFFFFFFFFULL
#endif


#if !(defined MSYS_TYPE_ERRVAL)
  #define MSYS_TYPE_ERRVAL int
#endif
#if !(defined MSYS_NO_TYPE_ERRVAL )
  typedef MSYS_TYPE_ERRVAL ErrVal;
#endif


// define status of lifecycle of an object instance 
#define INST_STATE_UNINITIALIZED  (0)
#define INST_STATE_INITIALIZING   (1)
#define INST_STATE_INITIALIZED    (2)
#define INST_STATE_UNINITIALIZING (3)

#if !(defined MSYS_TYPE_INSTSTATE)
  #define MSYS_TYPE_INSTSTATE unsigned int
#endif
#if !(defined MSYS_NO_TYPE_INSTSTATE )
  typedef MSYS_TYPE_INSTSTATE InstState;
#endif

  
#endif //__TYPEDEFS_H_D64BE9B4_A8DA_11D3_AFE7_005004464B79
