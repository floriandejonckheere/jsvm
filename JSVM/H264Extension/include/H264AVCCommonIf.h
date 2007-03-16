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
typedef Short   XPel;
typedef Long    XXPel;

class TCoeff
{
public:
	TCoeff()                                  { sCoeffValue=0; sLevelValue=0; };
	TCoeff( Short val )                       { sCoeffValue = val; sLevelValue=val; };
	~TCoeff()                                 { };

	operator Short() const { return sCoeffValue; };	
	TCoeff &operator=  (const Short val )      { sCoeffValue =  val; return *this; };
	TCoeff &operator+= (const Short val )      { sCoeffValue += val;  return *this; };
	TCoeff &operator-= (const Short val )      { sCoeffValue -= val;  return *this; };
	TCoeff &operator*= (const Short val )      { sCoeffValue *= val;  return *this; };
	TCoeff &operator/= (const Short val )      { sCoeffValue /= val;  return *this; };
	
	Short getLevel()                           { return sLevelValue; };
	Short getCoeff()                           { return sCoeffValue; };	
	Void  setLevel( Short val )                { sLevelValue = val; };
	Void  setCoeff( Short val )                { sCoeffValue = val; };	

private: 	
	Short sCoeffValue;
	Short sLevelValue;	
};

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

enum PicStruct
{
  PS_NOT_SPECIFIED = -1,
  PS_FRAME         =  0, // frame	field_pic_flag shall be 0	1
  PS_TOP           =  1, // top field	field_pic_flag shall be 1, bottom_field_flag shall be 0 1
  PS_BOT           =  2, // bottom field field_pic_flag shall be 1, bottom_field_flag shall be 1 1
  PS_TOP_BOT       =  3, // top field, bottom field, in that order field_pic_flag shall be 0 2
  PS_BOT_TOP       =  4  // bottom field, top field, in that order field_pic_flag shall be 0 2
};

class PicBuffer
{
public:
	PicBuffer( Pel* pcBuffer = NULL, Void* pcMediaPacket  = NULL, Int64 i64Cts = 0) 
		: m_pcMediaPacket( pcMediaPacket )
		, m_pcBuffer     ( pcBuffer )
		, m_iInUseCout   ( 0 )
		, m_i64Cts       ( i64Cts )
    , m_ePicStruct   ( PS_NOT_SPECIFIED )
    , m_iTopPoc      ( 0 )
    , m_iBotPoc      ( 0 )
    , m_iFramePoc    ( 0 )
    , m_uiIdrPicId   ( 0 )
    , m_bFieldCoded  ( false )
	{}

  Void setUnused() { m_iInUseCout--; }
  Void setUsed()   { m_iInUseCout++; }
  Bool isUsed()    { return 0 != m_iInUseCout; }
  Pel* getBuffer() { return m_pcBuffer; }
  operator Pel*()  { return m_pcBuffer; }
  Void* getMediaPacket() { return m_pcMediaPacket; }
  Int64& getCts()            { return m_i64Cts; }
  Void setCts( Int64 i64 )   { m_i64Cts = i64; } // HS: decoder robustness
  
	Void setPicStruct     ( PicStruct e  ) { m_ePicStruct  = e;  }
	Void setFieldCoding   ( Bool      b  ) { m_bFieldCoded = b;  }
  Void setIdrPicId      ( UInt      ui ) { m_uiIdrPicId  = ui; }
  Void setTopPoc        ( Int       i  ) { m_iTopPoc     = i;  }
  Void setBotPoc        ( Int       i  ) { m_iBotPoc     = i;  }
  Void setFramePoc      ( Int       i  ) { m_iFramePoc   = i;  }

	PicStruct getPicStruct() const  { return m_ePicStruct; }
  Bool isFieldCoded     () const  { return m_bFieldCoded; }
  UInt getIdrPicId      () const  { return m_uiIdrPicId; }
  Int  getTopPoc        () const  { return m_iTopPoc; }
  Int  getBotPoc        () const  { return m_iBotPoc; }
  Int  getFramePoc      () const  { return m_iFramePoc; }

private:
  Void*   m_pcMediaPacket;
  Pel*    m_pcBuffer;
  Int     m_iInUseCout;
	Int64			m_i64Cts;
	PicStruct m_ePicStruct;
  Int       m_iTopPoc;
  Int       m_iBotPoc;
  Int       m_iFramePoc;
  UInt      m_uiIdrPicId;
  Bool      m_bFieldCoded;
};

typedef MyList< PicBuffer* > PicBufferList;


#endif // !defined(AFX_H264AVCCOMMONIF_H__625AA7B6_0241_4166_8D3A_BC831985BE5F__INCLUDED_)
