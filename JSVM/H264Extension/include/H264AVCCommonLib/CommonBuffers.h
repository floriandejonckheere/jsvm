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




#if !defined(AFX_COMMONBUFFERS_H__CBFE313E_2382_4ECC_9D41_416668E3507D__INCLUDED_)
#define AFX_COMMONBUFFERS_H__CBFE313E_2382_4ECC_9D41_416668E3507D__INCLUDED_


H264AVC_NAMESPACE_BEGIN


template<class T>
class DynBuf
{
public:
  DynBuf(): m_uiBufferSize(0), m_pT(NULL) {}

  ErrVal init( UInt uiBufferSize )
  {
    ROT( NULL != m_pT );
    m_pT = new T [ uiBufferSize ];
    ROT( NULL == m_pT );
    m_uiBufferSize = uiBufferSize;
    return Err::m_nOK;
  }

  ErrVal uninit()
  {
    if(NULL != m_pT )
    {
    delete [] m_pT;
    m_pT = NULL;
    }
    m_uiBufferSize = 0;
    
    return Err::m_nOK;
  }

  T& get( UInt uiOffset ) const
  {
    AOT_DBG(uiOffset >= m_uiBufferSize);
    return m_pT[uiOffset];
  }

  Void set( UInt uiOffset, T t )
  {
    AOT_DBG(uiOffset >= m_uiBufferSize);
    m_pT[uiOffset] = t;
  }

  Void setAll( const T& rcT )
  {
    for( UInt n = 0; n < m_uiBufferSize; n++ )
    {
      m_pT[n] = rcT;
    }
  }

  T& operator[] (const UInt uiOffset)
  {
    AOT_DBG(uiOffset >= m_uiBufferSize);
    return m_pT[uiOffset];
  }

  const T& operator[] (const UInt uiOffset) const
  {
    AOT_DBG(uiOffset >= m_uiBufferSize);
    return m_pT[uiOffset];
  }

  Void clear()
  {
    ::memset( m_pT, 0, sizeof(T) * m_uiBufferSize );
  }

  UInt size() const
  {
    return m_uiBufferSize;
  }

protected:
  UInt m_uiBufferSize;
  T*   m_pT;
};




template< class T, UInt uiSize >
class StatBuf
{
public:
  StatBuf() {}

  ErrVal get( T& rcT, UInt uiOffset ) const
  {
    ROT(uiOffset >= uiSize);
    rcT = m_aT[uiOffset];
    return Err::m_nOK;
  }

  const ErrVal get( const T& rcT, UInt uiOffset ) const
  {
    ROT(uiOffset >= uiSize);
    rcT = m_aT[uiOffset];
    return Err::m_nOK;
  }

  const Bool isValidOffset( UInt uiOffset ) const { return uiOffset < uiSize; }

  const T& get( UInt uiOffset ) const
  {
    AOT_DBG(uiOffset >= uiSize);
    return m_aT[uiOffset];
  }

  T& get( UInt uiOffset )
  {
    AOT_DBG(uiOffset >= uiSize);
    return m_aT[uiOffset];
  }

  Void set( UInt uiOffset, T t )
  {
    AOT_DBG(uiOffset >= uiSize);
    m_aT[uiOffset] = t;
  }

  Void setAll( const T& rcT )
  {
    for( UInt n = 0; n < uiSize; n++ )
    {
      m_aT[n] = rcT;
    }
  }

  T& operator[] (const UInt uiOffset)
  {
    AOT_DBG(uiOffset >= uiSize);
    return m_aT[uiOffset];
  }

  const T& operator[] (const UInt uiOffset) const
  {
    AOT_DBG(uiOffset >= uiSize);
    return m_aT[uiOffset];
  }

  Void clear() { ::memset( m_aT, 0, sizeof(T) * uiSize); }

  UInt size() const  { return uiSize; }

protected:
  T m_aT[uiSize];
};

#define X_DATA_LIST_SIZE 64

template< class T >
class XDataList
{
public:
  XDataList  ();
  ~XDataList ();

  Void    reset       ();
  ErrVal  add         ( T*    pT );
  Void    setActive   ( UInt  ui = MSYS_UINT_MAX );
  Void    incActive   ();
  Void    decActive   ();
  Void    rightShift  ();
  Void    leftShift   ();
  Void    switchFirst ();

  UInt    getSize     ()                const;
  UInt    getActive   ()                const;
  T*      getEntry    ( UInt  uiIndex ) const;
  T*      operator[]  ( UInt  uiIndex ) const   { return getEntry( uiIndex - 1 ); }

  ErrVal  setElementAndRemove( UInt uiIPos, UInt uiRPos, T* pEntry );

private:
  UInt    m_uiSize;
  UInt    m_uiActive;
  T*      m_apT[X_DATA_LIST_SIZE];
};



template< class T >
XDataList<T>::XDataList()
: m_uiSize    ( 0 )
, m_uiActive  ( 0 )
{
}
 
template< class T >
XDataList<T>::~XDataList()
{
}

template< class T >
Void
XDataList<T>::reset()
{
  m_uiSize    = 0;
  m_uiActive  = 0;
}
  
template< class T >
ErrVal
XDataList<T>::add( T* pT )
{
  ROF( m_uiSize < X_DATA_LIST_SIZE );

  m_apT[ m_uiSize++ ] = pT;
  m_uiActive          = m_uiSize;

  return Err::m_nOK;
}

template< class T >
Void
XDataList<T>::setActive( UInt ui )
{
  m_uiActive = min( ui, m_uiSize );
}

template< class T >
Void
XDataList<T>::incActive()
{
  if( m_uiActive < m_uiSize )
  {
    m_uiActive++;
  }
}

template< class T >
Void
XDataList<T>::decActive()
{
  if( m_uiActive > 0 )
  {
    m_uiActive--;
  }
}

template< class T >
UInt
XDataList<T>::getSize() const
{
  return m_uiSize;
}

template< class T >
UInt
XDataList<T>::getActive() const
{
  return m_uiActive;
}
  
template< class T >
T* 
XDataList<T>::getEntry( UInt uiIndex ) const
{
  return ( uiIndex < m_uiActive ? m_apT[ uiIndex ] : 0 );
}

template< class T >
Void
XDataList<T>::rightShift()
{
  ROTVS( m_uiSize < 2 );
  
  T* pLast = m_apT[ m_uiSize - 1 ];
  for( Int i = m_uiSize-1; i > 0; i-- )
  {
    m_apT[i] = m_apT[i-1];
  }
  m_apT[0] = pLast;
}

template< class T >
Void
XDataList<T>::leftShift()
{
  ROTVS( m_uiSize < 2 );
  
  T* pFirst = m_apT[ 0 ];
  for( Int i = 1; i < m_uiSize; i++ )
  {
    m_apT[i-1] = m_apT[i];
  }
  m_apT[ m_uiSize - 1 ] = pFirst;
}


template< class T >
Void
XDataList<T>::switchFirst()
{
  T*  pTmp = m_apT[0];
  m_apT[0] = m_apT[1];
  m_apT[1] = pTmp;
}


template< class T >
ErrVal
XDataList<T>::setElementAndRemove( UInt uiIPos, UInt uiRPos, T* pEntry )
{
  ROT( uiIPos >= X_DATA_LIST_SIZE );
  ROT( uiIPos >= m_uiSize );
  ROT( uiIPos >  uiRPos   );
  if( uiIPos != uiRPos )
  {
    if( uiRPos >= m_uiSize && m_uiSize < X_DATA_LIST_SIZE )
    {
      m_uiSize++;
    }
    ::memmove( &(m_apT[uiIPos+1]), &(m_apT[uiIPos]), (min(uiRPos,m_uiSize-1)-uiIPos)*sizeof(T*) );
  }
  m_apT[uiIPos] = pEntry;
  return Err::m_nOK;
}



class IntFrame;
class ControlData;

typedef XDataList<IntFrame>     RefFrameList;
typedef XDataList<ControlData>  CtrlDataList;



H264AVC_NAMESPACE_END


#endif //!defined(AFX_COMMONBUFFERS_H__CBFE313E_2382_4ECC_9D41_416668E3507D__INCLUDED_)
