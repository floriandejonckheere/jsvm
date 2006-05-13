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




#if !defined(AFX_COMMONTYPES_H__CBFE313E_2382_4ECC_9D41_416668E3507D__INCLUDED_)
#define AFX_COMMONTYPES_H__CBFE313E_2382_4ECC_9D41_416668E3507D__INCLUDED_



H264AVC_NAMESPACE_BEGIN


class CostData
{
public:
  CostData( Double fRdCost = 0.0, UInt uiAddBits = 0, UInt uiBits = 0, UInt uiYDist = 0, UInt uiUDist = 0, UInt uiVDist = 0):  m_fRdCost(fRdCost),  m_uiAddBits(uiAddBits), m_uiBits(uiBits),  m_uiYDist(uiYDist),  m_uiUDist(uiUDist),  m_uiVDist(uiVDist) {}
  virtual ~CostData() {}

  CostData( const CostData& rcCostData)  { CostData( rcCostData.m_fRdCost, rcCostData.m_uiBits, rcCostData.m_uiYDist, rcCostData.m_uiUDist, rcCostData.m_uiVDist); }

  Void add( const CostData& rcCostData ) const
  {
    const_cast<CostData*>(this)->m_uiYDist += rcCostData.m_uiYDist;
    const_cast<CostData*>(this)->m_uiUDist += rcCostData.m_uiUDist;
    const_cast<CostData*>(this)->m_uiVDist += rcCostData.m_uiVDist;
    const_cast<CostData*>(this)->m_uiBits  += rcCostData.m_uiBits;
    const_cast<CostData*>(this)->m_fRdCost += rcCostData.m_fRdCost;
  }
  const UInt   distY()      const { return m_uiYDist; }
  const UInt   distU()      const { return m_uiUDist; }
  const UInt   distV()      const { return m_uiVDist; }
  const UInt   bits()       const { return m_uiBits; }
  const UInt   coeffCost()  const { return m_uiCoeffCost; }
  const Double rdCost()     const { return m_fRdCost; }
  const UInt&  addBits()    const { return m_uiAddBits; }

  UInt&   distY()       { return m_uiYDist; }
  UInt&   distU()       { return m_uiUDist; }
  UInt&   distV()       { return m_uiVDist; }
  UInt&   bits()        { return m_uiBits; }
  UInt&   addBits()     { return m_uiAddBits; }
  UInt&   coeffCost()   { return m_uiCoeffCost; }
  Double& rdCost()      { return m_fRdCost; }

  Bool operator < ( const CostData& rcCostData) { return m_fRdCost < rcCostData.m_fRdCost; }
  Bool operator > ( const CostData& rcCostData) { return m_fRdCost > rcCostData.m_fRdCost; }
  CostData& operator=( const CostData& rcCostData)
  {
    m_fRdCost       = rcCostData.m_fRdCost;
    m_uiCoeffCost   = rcCostData.m_uiCoeffCost;
    m_uiAddBits     = rcCostData.m_uiAddBits;
    m_uiBits        = rcCostData.m_uiBits;
    m_uiVDist       = rcCostData.m_uiVDist;
    m_uiUDist       = rcCostData.m_uiUDist;
    m_uiYDist       = rcCostData.m_uiYDist;
    return *this;
  }
  Void clear()
  {
    m_fRdCost     = DOUBLE_MAX;
    m_uiAddBits    = 0;
    m_uiBits       = 0;
    m_uiYDist      = 0;
    m_uiUDist      = 0;
    m_uiVDist      = 0;
    m_uiCoeffCost  = 0;
  }
protected:
  Double m_fRdCost;
  UInt   m_uiAddBits;
  UInt   m_uiBits;
  UInt   m_uiYDist;
  UInt   m_uiUDist;
  UInt   m_uiVDist;
  UInt   m_uiCoeffCost;
};


const UChar g_aucConvertBlockOrder[17] =
{
   0,  1,  4,  5,
   2,  3,  6,  7,
   8,  9, 12, 13,
  10, 11, 14, 15, 0xff,
};

// TMM_ESS {
const UChar g_aucConvertTo8x8Idx[16]=
{		
    0 ,0 ,1 , 1,
    0 ,0 ,1 , 1,
    2 ,2 ,3 , 3,
    2 ,2 ,3 ,3
};

const UChar g_aucConvertTo4x4Idx[4]=
{  0,2,8,10 };
// TMM_ESS }

class LumaIdx
{
protected:
  LumaIdx( Int iIdx = 0 )          : m_iIdx( iIdx )       {}

public:
  __inline LumaIdx   operator + ( SParIdx4x4     eSParIdx )  const { return LumaIdx( m_iIdx + eSParIdx ); }
  __inline LumaIdx   operator + ( NeighbourBlock eBlock )    const { return LumaIdx( m_iIdx + eBlock ); }
  __inline LumaIdx   operator + ( ParIdx8x8      eParIdx )   const { return LumaIdx( m_iIdx + eParIdx ); }

  operator Int()                                    const { return m_iIdx;    }
  Int x()                                           const { return m_iIdx&3;  }
  Int y()                                           const { return m_iIdx>>2; }
  Int b4x4()                                        const { return m_iIdx;    }

protected:
  Int m_iIdx;
};


class ChromaIdx
{
protected:
  ChromaIdx( Int iIdx = 0 ) : m_iIdx( iIdx )              {}
public:
  operator Int()                                    const { return m_iIdx;        }
  ChromaIdx   operator + ( Int i )                  const { return ChromaIdx( m_iIdx + i ); }
  ChromaIdx   operator - ( Int i )                  const { return ChromaIdx( m_iIdx - i ); }
  ChromaIdx&  operator+= ( Int i )                        { m_iIdx+=i; return *this; }
  ChromaIdx&  operator-= ( Int i )                        { m_iIdx-=i; return *this; }
  Int x()                                           const { return m_iIdx&1;      }
  Int y()                                           const { return (m_iIdx>>1)&1; }
  Int plane()                                       const { return m_iIdx>>2;     }

protected:
  Int m_iIdx;
};


class B8x8Idx :
public LumaIdx
{
public:
  B8x8Idx( Par8x8 ePar8x8 ) : m_iSIdx( ePar8x8<<2 )       { convert(); }
  B8x8Idx()  : m_iSIdx( 0 )                               { convert(); }
  B8x8Idx  operator + (Int i)                       const { return B8x8Idx( (m_iSIdx>>2) + i ); }
  B8x8Idx& operator +=(Int i)                             { m_iSIdx+=i<<2; convert(); return *this; }
  B8x8Idx& operator ++(Int)                               { m_iSIdx+=4;    convert(); return *this; }
  B8x8Idx& operator --(Int)                               { m_iSIdx-=4;    convert(); return *this; }
  Bool      isLegal()                               const { return m_iSIdx < 16; }
  const Int xxxgetSIdx()                            const { return m_iSIdx; }
  ParIdx8x8 b8x8()                                  const { return ParIdx8x8(m_iIdx); }
  Par8x8    b8x8Index()                             const { return Par8x8(m_iSIdx>>2); }

protected:
  Int convert()                                           { return m_iIdx = g_aucConvertBlockOrder[m_iSIdx]; }
  B8x8Idx(Int i) : m_iSIdx( i<<2 )                        { convert(); }

protected:
  Int m_iSIdx;
};


class B4x4Idx :
public LumaIdx
{
public:
  B4x4Idx( UInt ui4x4Idx = 0)  : LumaIdx( ui4x4Idx )      {}
  B4x4Idx& operator ++(Int)                               { m_iIdx++; return *this; }
  Bool isLegal()                                    const { return m_iIdx < 16; }
};


class S4x4Idx :
public LumaIdx
{
public:
  S4x4Idx( B8x8Idx& rcB8x8Idx) : m_iSIdx( rcB8x8Idx.xxxgetSIdx() )  { convert(); }
  S4x4Idx()                    : m_iSIdx( 0 )                       { convert(); }
  S4x4Idx& operator ++(Int)                                         { m_iSIdx++; convert(); return *this; }
  Bool isLegal()                                              const { return m_iSIdx <16; }
  Bool isLegal( B8x8Idx& rcB8x8Idx )                          const { return m_iSIdx <(4+rcB8x8Idx.xxxgetSIdx()); }
protected:
  Void convert()                                                    { m_iIdx = g_aucConvertBlockOrder[m_iSIdx]; }

protected:
  Int m_iSIdx;
};


class CIdx :
public ChromaIdx
{
public:
  CIdx( UInt uiIdx = 0)     : ChromaIdx( uiIdx )      {}
  CIdx& operator ++(Int)                              { m_iIdx++; return *this; }
  Bool isLegal()                                const { return m_iIdx < 8; }
  Bool isLegal( Int i)                          const { return m_iIdx < i; }
};



template< class T, UInt uiMemSize = 64 >
class RefPicList
{
public:
  RefPicList() : m_uiBufSize( 0 ), m_uiSize( 0 )
  {
  }
  Void reset( UInt uiBufSize = uiMemSize )
  {
    AOT_DBG( uiBufSize > uiMemSize );
    m_uiBufSize = uiBufSize;
    m_uiSize    = 0;
  }
  ErrVal restrict( UInt uiBufSize )
  {
    ROT( uiBufSize > m_uiSize );
    m_uiBufSize = m_uiSize = uiBufSize;
    return Err::m_nOK;
  }
  Void add( const T& rcT )
  {
    AOT_DBG( full() );
    m_acT[ m_uiSize++ ] = rcT;
  }
  T& getElementAndRemove( UInt uiIPos, UInt uiRPos = MSYS_UINT_MAX )
  {
    AOT_DBG( uiIPos >= m_uiBufSize );
    AOT_DBG( uiIPos >  m_uiSize    );
    AOT_DBG( uiIPos >  uiRPos      );
    if( uiIPos != uiRPos )
    {
      if( uiRPos >= m_uiSize && ! full() )
      {
        m_uiSize++;
      }
      ::memmove( &(m_acT[uiIPos+1]), &(m_acT[uiIPos]), ( min( uiRPos, m_uiSize-1 ) - uiIPos ) * sizeof(T) );
    }
    return m_acT[uiIPos];
  }
  T& next()
  {
    AOT_DBG( full() );
    return m_acT[ m_uiSize++ ];
  }
  Bool full() const
  {
    return ( m_uiSize >= m_uiBufSize );
  }
  UInt size() const
  {
    return m_uiSize;
  }
  UInt bufSize() const
  {
    return m_uiBufSize;
  }
  const T& get( UInt uiPos ) const
  {
    AOT_DBG( uiPos >= m_uiSize );
    return m_acT[ uiPos ];
  }

  const SChar find( const T& rcT ) const
  {
    SChar nMax = size();
    for( SChar n = 0; n < nMax; n++ )
    {
      if( rcT == m_acT[n] )
      {
        return n + 1;
      }
    }
    return 0;
  }
  Void switchFirstEntries()
  {
    AOT_DBG( m_uiSize < 2 );
    T cT0old = m_acT[0];
    m_acT[0] = m_acT[1];
    m_acT[1] = cT0old;
  }
  T* begin()
  {
    return m_acT;
  }
  T* end()
  {
    return m_acT + m_uiSize;
  }

private:
  UInt  m_uiBufSize;
  UInt  m_uiSize;
  T     m_acT[uiMemSize];
};



H264AVC_NAMESPACE_END


#endif //!defined(AFX_COMMONTYPES_H__CBFE313E_2382_4ECC_9D41_416668E3507D__INCLUDED_)
