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


#if !defined(AFX_MEMCONT_H__2B6BDC73_9A42_4459_A731_DCB2E39E335E__INCLUDED_)
#define AFX_MEMCONT_H__2B6BDC73_9A42_4459_A731_DCB2E39E335E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MemIf.h"
#include "MemAccessor.h"



template< class T >
class MemCont : public MemIf< T >
{
public:
  class MemContHelper
  {
  public:
    MemContHelper() {}
    MemContHelper( MemIf< T >& rcMemIf ) 
    {
      rcMemIf.release( m_pcT, m_uiSize, m_pcOrigT, m_uiUsableSize );
    }
    virtual ~MemContHelper() {}

  public:
    MemContHelper& operator=( MemIf< T >& rcMemIf ) 
    { 
      rcMemIf.release( m_pcT, m_uiSize, m_pcOrigT, m_uiUsableSize );
      return *this;
    }

  public:
    T* data() const { return m_pcT; }
    T* origData() const { return m_pcOrigT; }
    UInt size() const { return m_uiSize; }
    UInt usableSize() const { return m_uiUsableSize; }

  private:
    T* m_pcT;
    T* m_pcOrigT;
    UInt m_uiSize;
    UInt m_uiUsableSize;
  };

public:
  MemCont() :
    m_pcT( NULL ), m_pcOrigT( NULL ), m_uiSize( 0 ), m_uiUsableSize( 0 ) {}

  MemCont( T* pcT, UInt uiSize, T* pcOrigT=NULL, UInt uiUsableSize=0 ) :
    m_pcT( pcT ), m_pcOrigT( pcOrigT ), m_uiSize( uiSize ) , m_uiUsableSize( uiUsableSize )
  {
    if( NULL == m_pcOrigT ) m_pcOrigT = m_pcT;
    if( 0 == m_uiUsableSize ) m_uiUsableSize = uiSize;
  }

  MemCont( const MemCont< T >& rcMemCont ) :
    m_pcT( NULL ), m_pcOrigT( NULL ), m_uiSize( 0 ), m_uiUsableSize( 0 )
  {
    if( (0 != rcMemCont.m_uiSize) && (NULL != rcMemCont.m_pcT) )
    {
      m_uiSize = rcMemCont.m_uiSize;
      m_uiUsableSize = rcMemCont.m_uiUsableSize;
      m_pcT = new T[m_uiSize];
      AOT( NULL == m_pcT );
      m_pcOrigT = m_pcT;
      ::memcpy( m_pcT, rcMemCont.m_pcT, sizeof( T ) * m_uiSize );
    }
  }

  MemCont( const MemAccessor< T >& rcMemAccessor ) :
    m_pcT( NULL ), m_pcOrigT( NULL ), m_uiSize( 0 ), m_uiUsableSize( 0 )
  {
    if( (0 != rcMemAccessor.size()) && (NULL != rcMemAccessor.data()) )
    {
      m_uiSize = rcMemAccessor.size();
      m_uiUsableSize = rcMemAccessor.usableSize() ;
      m_pcT = rcMemAccessor.data();
      m_pcOrigT = rcMemAccessor.origData();
    }
  }

  MemCont( const MemContHelper& rcMemContHelper ) :
    m_pcT( NULL ), m_pcOrigT( NULL ), m_uiSize( 0 ), m_uiUsableSize( 0 )
  {
    if( (0 != rcMemContHelper.size()) && (NULL != rcMemContHelper.data()) )
    {
      m_uiSize = rcMemContHelper.size();
      m_uiUsableSize = rcMemContHelper.usableSize() ;
      m_pcT = rcMemContHelper.data();
      m_pcOrigT = rcMemContHelper.origData();
    }
  }

  virtual ~MemCont() { if( m_pcOrigT ) { delete[] m_pcOrigT; } }

public:
  MemCont< T >& operator=( const MemCont< T >& rcMemCont )
  {
    if( this == &rcMemCont ) { return *this; }

    // delete memory if allocated
    if( m_pcOrigT ) { delete[] m_pcOrigT; }

    if( (0 != rcMemCont.m_uiSize) && (NULL != rcMemCont.m_pcT) )
    {
      m_uiSize = rcMemCont.m_uiSize;
      m_uiUsableSize = rcMemCont.m_uiUsableSize;
      m_pcT = new T[m_uiSize];
      AOT( NULL == m_pcT );
      m_pcOrigT = m_pcT;
      ::memcpy( m_pcT, rcMemCont.m_pcT, sizeof( T ) * m_uiSize );
    }
    else
    {
      m_pcT = NULL;
      m_pcOrigT = NULL;
      m_uiSize = 0;
      m_uiUsableSize = 0;
    }

    return *this;
  }

  MemCont< T >& operator=( const MemAccessor< T >& rcMemAccessor )
  {
    if( m_pcOrigT == rcMemAccessor.origData() ) 
    {  
      m_uiUsableSize = rcMemAccessor.usableSize() ;
      m_pcT = rcMemAccessor.data();
      m_pcOrigT = rcMemAccessor.origData();
    }
    else 
    { 
      if( m_pcOrigT ) { delete[] m_pcOrigT; }
      if( (0 != rcMemAccessor.size()) && (NULL != rcMemAccessor.data()) )
      {
        m_uiSize = rcMemAccessor.size();
        m_uiUsableSize = rcMemAccessor.usableSize() ;
        m_pcT = rcMemAccessor.data();
        m_pcOrigT = rcMemAccessor.origData();
      }
      else
      {
        m_pcT = NULL;
        m_pcOrigT = NULL;
        m_uiSize = 0;
        m_uiUsableSize = 0;
      }
    }

    return *this;
  }

  MemCont< T >& operator=( const MemContHelper& rcMemContHelper )
  {
    if( m_pcOrigT == rcMemContHelper.origData() ) 
    {  
      m_uiUsableSize = rcMemContHelper.usableSize() ;
      m_pcT = rcMemContHelper.data();
      m_pcOrigT = rcMemContHelper.origData();
    }
    else 
    { 
      if( m_pcOrigT ) { delete[] m_pcOrigT; }
      if( (0 != rcMemContHelper.size()) && (NULL != rcMemContHelper.data()) )
      {
        m_uiSize = rcMemContHelper.size();
        m_uiUsableSize = rcMemContHelper.usableSize() ;
        m_pcT = rcMemContHelper.data();
        m_pcOrigT = rcMemContHelper.origData();
      }
      else
      {
        m_pcT = NULL;
        m_pcOrigT = NULL;
        m_uiSize = 0;
        m_uiUsableSize = 0;
      }
    }

    return *this;
  }

public:
  MemType getMemType() const { return MEM_CONT; }

  Void set( MemIf< T >& rcMemIf )
  {
    ROTV( this == &rcMemIf );
    if( m_pcOrigT ) { delete[] m_pcOrigT; }
    rcMemIf.release( m_pcT, m_uiSize, m_pcOrigT, m_uiUsableSize );
  }

  Void set( T* pcT, UInt uiSize, T* pcOrigT=NULL, UInt uiUsableSize=0 )
  {
    if( NULL == pcOrigT ) { pcOrigT = pcT; }
    if( 0 == uiUsableSize ) { uiUsableSize = uiSize; }
    if( m_pcOrigT != pcOrigT ) { if( m_pcOrigT ) { delete[] m_pcOrigT; } }
    m_pcT = pcT;
    m_pcOrigT = pcOrigT;
    m_uiSize = uiSize;
    m_uiUsableSize = uiUsableSize;
  }

  Void release( T*& rpcT, UInt& ruiSize )
  {
    if( m_pcT == m_pcOrigT )
    {
      rpcT = m_pcT;
      ruiSize = m_uiSize;
    }
    else
    {
      if( 0 != m_uiSize )
      {
        ruiSize = m_uiSize;
        rpcT = new T[ruiSize];
        AOT( NULL == rpcT );
        ::memcpy( rpcT, m_pcT, sizeof( T ) * ruiSize );
      }
      else
      {
        rpcT = NULL;
        ruiSize = 0;
      }
      if( m_pcOrigT ) { delete[] m_pcOrigT; }
    }

    m_pcT = NULL;
    m_pcOrigT = NULL;
    m_uiSize = 0;
  }

  Void release( T*& rpcT, UInt& ruiSize, T*& rpcOrigT, UInt& ruiUsableSize )
  {
    rpcT = m_pcT;
    ruiSize = m_uiSize;
    rpcOrigT = m_pcOrigT;
    ruiUsableSize = m_uiUsableSize;

    m_pcT = NULL;
    m_pcOrigT = NULL;
    m_uiSize = 0;
    m_uiUsableSize = 0;
  }

  Void release( MemAccessor< T >& rcMemAccessor )
  {
    rcMemAccessor.set( m_pcT, m_uiSize, m_pcOrigT, m_uiUsableSize );

    m_pcT = NULL;
    m_pcOrigT = NULL;
    m_uiSize = 0;
    m_uiUsableSize = 0;
  }

  Void deleteData() { if( m_pcOrigT ) { delete[] m_pcOrigT; } m_pcOrigT = NULL; m_pcT = NULL; m_uiSize = 0; m_uiUsableSize = 0; }
  Void reset() { m_pcOrigT = NULL; m_pcT = NULL; m_uiSize = 0; m_uiUsableSize = 0; }

  T* data() const { return m_pcT; }
  T* origData() const { return m_pcOrigT; }
  UInt size() const { return m_uiSize; }
  UInt usableSize() const { return m_uiUsableSize; }
  UInt byteSize() const { return sizeof( T ) * m_uiSize; }
  UInt usableByteSize() const { return sizeof( T ) * m_uiUsableSize; }

  Void setMemAccessor( MemAccessor< T >& rcMemAccessor ) { rcMemAccessor.set( m_pcT, m_uiSize, m_pcOrigT, m_uiUsableSize ); }

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


#endif // !defined(AFX_MEMCONT_H__2B6BDC73_9A42_4459_A731_DCB2E39E335E__INCLUDED_)
