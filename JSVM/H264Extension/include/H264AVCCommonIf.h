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





#if !defined(AFX_H264AVCCOMMONIF_H__625AA7B6_0241_4166_8D3A_BC831985BE5F__INCLUDED_)
#define AFX_H264AVCCOMMONIF_H__625AA7B6_0241_4166_8D3A_BC831985BE5F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#if defined( WIN32 )
# if !defined( MSYS_WIN32 )
#   define MSYS_WIN32
# endif
#endif

#if defined( _DEBUG ) || defined( DEBUG )
# if !defined( _DEBUG )
#   define _DEBUG
# endif
# if !defined( DEBUG )
#   define DEBUG
# endif
#endif


typedef int ErrVal;

class Err
{
public:
  static const ErrVal m_nOK;
  static const ErrVal m_nERR;
  static const ErrVal m_nEndOfStream;
  static const ErrVal m_nEndOfFile;
  static const ErrVal m_nEndOfBuffer;
  static const ErrVal m_nInvalidParameter;
  static const ErrVal m_nDataNotAvailable;
};

#include <assert.h>
#include <iostream>
#include "Typedefs.h"
#include "Macros.h"
#include "MemList.h"


#define min(x,y) ((x)<(y)?(x):(y))
#define max(x,y) ((x)>(y)?(x):(y))

typedef MemCont< UChar > BinData;
typedef MemList< UChar > BinDataList;
typedef MemAccessor< UChar > BinDataAccessor;

#include <list>
#include <algorithm>

typedef UChar   Pel;
typedef Short   TCoeff;
typedef Short   XPel;
typedef Long    XXPel;

template< class T >
class MyList : public std::list< T >
{
public:
  typedef typename std::list<T>::iterator MyIterator;

  MyList& operator += ( const MyList& rcMyList) { if( ! rcMyList.empty() ) { insert( this->end(), rcMyList.begin(), rcMyList.end());} return *this; } // leszek
  T popBack()                           { T cT = this->back(); this->pop_back(); return cT;  }
  T popFront()                          { T cT = this->front(); this->pop_front(); return cT; }
  Void pushBack( const T& rcT )         { if( sizeof(T) == 4) { if( rcT != NULL ){ push_back( rcT);} } }
  Void pushFront( const T& rcT )        { if( sizeof(T) == 4) { if( rcT != NULL ){ push_front( rcT);} } }
  MyIterator find( const T& rcT ) {  return std::find( this->begin(), this->end(), rcT ); } // leszek
};

class ExtBinDataAccessor : public BinDataAccessor
{
public:
  ExtBinDataAccessor() : BinDataAccessor() , m_pcMediaPacket (NULL ){}
  ExtBinDataAccessor( BinDataAccessor& rcAccessor, Void* pcMediaPacket = NULL )
    :  BinDataAccessor( rcAccessor )
    ,  m_pcMediaPacket (pcMediaPacket ){}

  Void* getMediaPacket() { return m_pcMediaPacket; }
private:
  Void* m_pcMediaPacket;
};


typedef MyList< ExtBinDataAccessor* > ExtBinDataAccessorList;
typedef MyList< ExtBinDataAccessorList* > ExtBinDataAccessorListList;

class PicBuffer
{
public:
  PicBuffer( Pel* pcBuffer = NULL, Void* pcMediaPacket  = NULL, UInt64 ui64Cts = 0) : m_pcMediaPacket(pcMediaPacket), m_pcBuffer(pcBuffer), m_iInUseCout(0), m_ui64Cts(ui64Cts) {}
  Void setUnused() { m_iInUseCout--; }
  Void setUsed()   { m_iInUseCout++; }
  Bool isUsed()    { return 0 != m_iInUseCout; }
  Pel* getBuffer() { return m_pcBuffer; }
  operator Pel*()  { return m_pcBuffer; }
  Void* getMediaPacket() { return m_pcMediaPacket; }
  UInt64& getCts() { return m_ui64Cts; }

  Void setCts( UInt64 ui64 ) { m_ui64Cts = ui64; } // HS: decoder robustness

private:
  Void*   m_pcMediaPacket;
  Pel*    m_pcBuffer;
  Int     m_iInUseCout;
  UInt64  m_ui64Cts;
};

typedef MyList< PicBuffer* > PicBufferList;


#endif // !defined(AFX_H264AVCCOMMONIF_H__625AA7B6_0241_4166_8D3A_BC831985BE5F__INCLUDED_)
