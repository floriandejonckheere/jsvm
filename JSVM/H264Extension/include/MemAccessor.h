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


#if !defined(AFX_MEMACCESSOR_H__F7B8E14C_79CB_4DAF_A1BE_9AA64D4A2DDA__INCLUDED_)
#define AFX_MEMACCESSOR_H__F7B8E14C_79CB_4DAF_A1BE_9AA64D4A2DDA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <list>


template< class T >
class MemAccessor
{
public:
  MemAccessor() :
    m_pcT( NULL ), m_pcOrigT( NULL ), m_uiSize( 0 ), m_uiUsableSize( 0 ) {}

  MemAccessor( T* pcT, UInt uiSize, T* pcOrigT=NULL, UInt uiUsableSize=0 ) :
    m_pcT( pcT ), m_pcOrigT( pcOrigT ), m_uiSize( uiSize ), m_uiUsableSize( uiUsableSize )
  {
    if( NULL == m_pcOrigT ) m_pcOrigT = m_pcT;
    if( 0 == m_uiUsableSize ) m_uiUsableSize = uiSize;
  }

  MemAccessor( const MemAccessor< T >& rcMemAccessor ) :
    m_pcT( rcMemAccessor.m_pcT ), m_pcOrigT( rcMemAccessor.m_pcOrigT ), m_uiSize( rcMemAccessor.m_uiSize ) , m_uiUsableSize( rcMemAccessor.m_uiUsableSize ) {}

  virtual ~MemAccessor() {}

public:
  MemAccessor< T >& operator=( const MemAccessor< T >& rcMemAccessor )
  {
    m_pcT = rcMemAccessor.m_pcT;
    m_pcOrigT = rcMemAccessor.m_pcOrigT;
    m_uiSize = rcMemAccessor.m_uiSize;
    m_uiUsableSize = rcMemAccessor.m_uiUsableSize;

    return *this;
  }

public:
  Void set( const MemAccessor< T >& rcMemAccessor )
  {
    m_pcT = rcMemAccessor.m_pcT;
    m_pcOrigT = rcMemAccessor.m_pcOrigT;
    m_uiSize = rcMemAccessor.m_uiSize;
    m_uiUsableSize = rcMemAccessor.m_uiUsableSize;
  }

  Void set( T* pcT, UInt uiSize, T* pcOrigT=NULL, UInt uiUsableSize=0 )
  {
    m_pcT = pcT;
    m_uiSize = uiSize;
    m_pcOrigT = pcOrigT;
    m_uiUsableSize = uiUsableSize;
    if( NULL == pcOrigT ) { m_pcOrigT = pcT; }
    if( 0 == uiUsableSize ) { m_uiUsableSize = m_uiSize; }
  }

  Void clear()
  {
    m_pcT = NULL;
    m_pcOrigT = NULL;
    m_uiSize = 0;
    m_uiUsableSize = 0;
  }

  Void deleteData() { if( m_pcOrigT ) { delete[] m_pcOrigT; } m_pcOrigT = NULL; m_pcT = NULL; m_uiSize = 0; m_uiUsableSize = 0; }

  T* data() const { return m_pcT; }
  T* origData() const { return m_pcOrigT; }
  UInt size() const { return m_uiSize; }
  UInt usableSize() const { return m_uiUsableSize; }
  UInt byteSize() const { return sizeof( T ) * m_uiSize; }
  UInt usableByteSize() const { return sizeof( T ) * m_uiUsableSize; }

public:
  ErrVal increasePos( UInt uiPos )
  {
    ROT( uiPos >= m_uiSize );
    m_pcT += uiPos;
    m_uiSize -= uiPos;
    return Err::m_nOK;
  }


  ErrVal decreasePos( UInt uiPos )
  {
    ROT( m_pcT - uiPos < m_pcOrigT );
    m_pcT -= uiPos;
    m_uiSize += uiPos;
    return Err::m_nOK;
  }

  ErrVal resetPos()
  {
    m_uiSize += m_pcT - m_pcOrigT;
    m_pcT = m_pcOrigT;
    return Err::m_nOK;
  }

  ErrVal increaseEndPos( UInt uiPos )
  {
    ROT( uiPos > (m_uiUsableSize - m_uiSize) );
    m_uiSize += uiPos;
    return Err::m_nOK;
  }

  ErrVal decreaseEndPos( UInt uiPos )
  {
    ROT( uiPos > m_uiSize );
    m_uiSize -= uiPos;
    return Err::m_nOK;
  }

  ErrVal resetEndPos()
  {
    m_uiSize = m_uiUsableSize - (m_pcT - m_pcOrigT);
    return Err::m_nOK;
  }

private:
  T* m_pcT;
  T* m_pcOrigT;
  UInt m_uiSize;
  UInt m_uiUsableSize;
};



template< class T >
class MemAccessList
{
public:
  MemAccessList() : m_uiSize( 0 ) {}

  MemAccessList( const MemAccessList< T >& rcMemAccessList ) : m_uiSize( 0 )
  {
    m_uiSize = rcMemAccessList.m_uiSize;
    typename MemAccessorList::const_iterator pcPair;
    for( pcPair = rcMemAccessList.m_cMemAccessorList.begin(); pcPair != rcMemAccessList.m_cMemAccessorList.end(); pcPair++  )
    {
      m_cMemAccessorList.push_back( MemAccessor< T >( pcPair->data(), pcPair->size(), pcPair->origData(), pcPair->usableSize() ) );
    }
  }

  MemAccessList( T* pcT, UInt uiSize, T* pcOrigT=NULL, UInt uiUsableSize=0 ) : m_uiSize( 0 )
  {
    if( NULL == pcOrigT ) { pcOrigT = pcT; }
    if( 0 == uiUsableSize ) { uiUsableSize = uiSize; }
    push_back( pcT, uiSize, pcOrigT, uiUsableSize );
  }

  virtual ~MemAccessList() {}

  MemAccessList< T >& operator=( const MemAccessList< T >& rcMemAccessList )
  {
    // paranoia
    ROTRS( this == &rcMemAccessList, *this );

    // reset class
    reset();

    m_uiSize = rcMemAccessList.m_uiSize;
    typename MemAccessorList::const_iterator pcPair;
    for( pcPair = rcMemAccessList.m_cMemAccessorList.begin(); pcPair != rcMemAccessList.m_cMemAccessorList.end(); pcPair++  )
    {
      m_cMemAccessorList.push_back( MemAccessor< T >( pcPair->data(), pcPair->size(), pcPair->origData(), pcPair->usableSize() ) );
    }

    return *this;
  }

public:
  Void set( T* pcT, UInt uiSize, T* pcOrigT=NULL, UInt uiUsableSize=0 )
  {
    reset();
    if( NULL == pcOrigT ) { pcOrigT = pcT; }
    if( 0 == uiUsableSize ) { uiUsableSize = uiSize; }
    push_back( pcT, uiSize, pcOrigT, uiUsableSize );
  }

  Void set( MemAccessor< T >& rcMemAccessor )
  {
    reset();
    push_back( rcMemAccessor );
  }

  Void copyPayload( T*& rpcT, UInt& ruiSize )
  {
    if( 0 == m_uiSize )
    {
      rpcT = NULL;
      ruiSize = 0;
      return;
    }

    ruiSize = m_uiSize;
    rpcT = new T[ruiSize];
    ROTV( NULL == rpcT );

    UInt uiPos = 0;
    MemAccessorListIterator pcPair;
    for( pcPair = m_cMemAccessorList.begin(); pcPair != m_cMemAccessorList.end(); pcPair++ )
    {
      ROTV( uiPos + pcPair->size() > ruiSize );
      ::memcpy( &rpcT[uiPos], pcPair->data(), sizeof(T) * pcPair->size() );
      uiPos += pcPair->size();
    }

  }

  UInt listSize() const { return m_cMemAccessorList.size(); }

  T* entryData( UInt uiEntryPos )
  {
    // check if we have more than one entry
    ROTR( uiEntryPos >= m_cMemAccessorList.size(), NULL );
    typename MemAccessorList::const_iterator pcMemAccessor;
    for( pcMemAccessor = m_cMemAccessorList.begin(); uiEntryPos-- != 0; pcMemAccessor++ )
      ;
    return pcMemAccessor->data();
  }

  UInt entrySize( UInt uiEntryPos )
  {
    // check if we have more than one entry
    ROTR( uiEntryPos >= m_cMemAccessorList.size(), 0 );
    typename MemAccessorList::const_iterator pcMemAccessor;
    for( pcMemAccessor = m_cMemAccessorList.begin(); uiEntryPos-- != 0; pcMemAccessor++ )
      ;
    return pcMemAccessor->size();
  }

  UInt entryByteSize( UInt uiEntryPos ) const { return sizeof( T ) * entrySize( uiEntryPos ); }
  UInt size() const { return m_uiSize; }
  UInt byteSize() const { return m_uiSize * sizeof( T ); }

public:
  Void reset() { m_cMemAccessorList.clear(); m_uiSize = 0; }

public:
  Void push_back( T* pcT, UInt uiSize, T* pcOrigT=NULL, UInt uiUsableSize=0 )
  {
    if( NULL == pcOrigT ) { pcOrigT = pcT; }
    if( 0 == uiUsableSize ) { uiUsableSize = uiSize; }
    m_cMemAccessorList.push_back( MemAccessor< T >( pcT, uiSize, pcOrigT, uiUsableSize ) );
    m_uiSize += uiSize;
  }

  Void push_back( MemAccessor< T >& rcMemAccessor )
  {
    m_cMemAccessorList.push_back( rcMemAccessor );
    //m_uiSize += uiSize; // leszek: uiSize is not defined
	m_uiSize += rcMemAccessor.m_uiSize; // leszek: this seems to be the intention
  }

  Void push_front( T* pcT, UInt uiSize, T* pcOrigT=NULL, UInt uiUsableSize=0 )
  {
    if( NULL == pcOrigT ) { pcOrigT = pcT; }
    if( 0 == uiUsableSize ) { uiUsableSize = uiSize; }
    m_cMemAccessorList.push_front( MemAccessor< T >( pcT, uiSize, pcOrigT, uiUsableSize ) );
    m_uiSize += uiSize;
  }

  Void push_front( MemAccessor< T >& rcMemAccessor )
  {
    m_cMemAccessorList.push_front( rcMemAccessor );
    //m_uiSize += uiSize; // leszek: uiSize is not defined
	m_uiSize += rcMemAccessor.m_uiSize; // leszek: this seems to be the intention
  }

private:
  typedef std::list< MemAccessor< T > > MemAccessorList;
  typedef typename MemAccessorList::iterator MemAccessorListIterator;

private:
  MemAccessorList m_cMemAccessorList;
  UInt m_uiSize;
};



#endif // !defined(AFX_MEMACCESSOR_H__F7B8E14C_79CB_4DAF_A1BE_9AA64D4A2DDA__INCLUDED_)
