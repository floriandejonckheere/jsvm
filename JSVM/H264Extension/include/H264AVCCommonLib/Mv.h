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





#if !defined(AFX_MV_H__91E1E1F3_0D6C_4222_9D71_EAB2E415688C__INCLUDED_)
#define AFX_MV_H__91E1E1F3_0D6C_4222_9D71_EAB2E415688C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



H264AVC_NAMESPACE_BEGIN


class H264AVCCOMMONLIB_API Mv
{
public:
	__inline const UInt xGetFilter() const;

  Mv()
    : m_sHor( 0 ),
      m_sVer( 0 )
  {}
  Mv( Short sHor, Short sVer )
    : m_sHor( sHor ),
      m_sVer( sVer )
  {}

  Short getHor()                const           { return m_sHor; }
  Short getVer()                const           { return m_sVer; }
  Short getAbsHor()             const           { return abs( m_sHor ); }
  Short getAbsVer()             const           { return abs( m_sVer ); }

  Void  setZero()                               { /*m_sHor = m_sVer = 0;*/ *((UInt*)&m_sHor) = 0;}
  Void  setHor( Short sHor )                    { m_sHor = sHor; }
  Void  setVer( Short sVer )                    { m_sVer = sVer; }
  Void  set( Short sHor, Short sVer )           { m_sHor = sHor; m_sVer = sVer; }

  Short getAbsHorDiff( const Mv& rcMv )  const  { return abs( m_sHor - rcMv.m_sHor ); }
  Short getAbsVerDiff( const Mv& rcMv )  const  { return abs( m_sVer - rcMv.m_sVer ); }

  const UInt  getFilter() const { return (m_sHor & 3) + (m_sVer & 3)*4;}
  const Mv operator + ( const Mv& rcMv ) const
  {
    return Mv( m_sHor + rcMv.m_sHor, m_sVer + rcMv.m_sVer );
  }
  const Mv operator - ( const Mv& rcMv ) const
  {
    return Mv( m_sHor - rcMv.m_sHor, m_sVer - rcMv.m_sVer );
  }
  const Mv& operator += ( const Mv& rcMv )
  {
    m_sHor += rcMv.m_sHor;
    m_sVer += rcMv.m_sVer;
    return *this;
  }
  const Mv& operator -= ( const Mv& rcMv )
  {
    m_sHor -= rcMv.m_sHor;
    m_sVer -= rcMv.m_sVer;
    return *this;
  }

  const Mv operator << ( Short sShift ) const
  {
    return Mv( m_sHor << sShift, m_sVer << sShift );
  }
  const Mv operator >> ( Short sShift ) const
  {
    return Mv( m_sHor >> sShift, m_sVer >> sShift );
  }

  const Mv& operator <<= ( Short sShift )
  {
    m_sHor <<= sShift;
    m_sVer <<= sShift;
    return *this;
  }
  const Mv& operator >>= ( Short sShift )
  {
    m_sHor >>= sShift;
    m_sVer >>= sShift;
    return *this;
  }

  Bool operator== ( const Mv& rcMv )
  {
    return (m_sHor==rcMv.m_sHor && m_sVer==rcMv.m_sVer);
  }

  Void limitComponents( const Mv& rcMvMin, const Mv& rcMvMax )
  {
    m_sHor = min( rcMvMax.m_sHor, max( rcMvMin.m_sHor, m_sHor ) );
    m_sVer = min( rcMvMax.m_sVer, max( rcMvMin.m_sVer, m_sVer ) );
  }

  static const Mv& ZeroMv() { return m_cMvZero; }

  const Mv scaleMv( Int iScale )
  {
    return Mv( (iScale * getHor() + 128) >> 8, (iScale * getVer() + 128) >> 8);
  }

public:
	static const Mv* m_cMvZero1;
  static const Mv m_cMvZero;

  Short m_sHor;
  Short m_sVer;
};




enum RefIdxValues
{
  BLOCK_NOT_AVAILABLE =  0,
  BLOCK_NOT_PREDICTED = -1
};



class H264AVCCOMMONLIB_API Mv3D :
public Mv
{
public:
  Mv3D()
    : Mv     (),
      m_scRef( BLOCK_NOT_AVAILABLE )
  {}
  Mv3D( const Mv& rcMv, SChar scRef )
    : Mv     ( rcMv ),
      m_scRef( scRef )
  {}
  Mv3D( Short sHor, Short sVer, SChar scRef )
    : Mv     ( sHor, sVer ),
      m_scRef( scRef )
  {}

  SChar getRef()                const               { return m_scRef; }

  Void  setZero()                                   { Mv::setZero(); m_scRef = BLOCK_NOT_AVAILABLE; }
  Void  setRef( SChar scRef )                       { m_scRef = scRef; }
  Void  setMv( const Mv& rcMv )                     { Mv::operator= ( rcMv); }
  Void  set( Short sHor, Short sVer, SChar scRef )  { Mv::set( sHor, sVer); m_scRef = scRef; }
  Void  set( const Mv& rcMv, SChar scRef )          { Mv::operator= ( rcMv ); m_scRef = scRef; }

  Mv3D& minRefIdx( Mv3D& rcMv3D )  { return (((UChar)(rcMv3D.getRef()-1)) < ((UChar)(getRef()-1)) ? rcMv3D : *this); }
  Bool operator== ( const RefIdxValues eRefIdxValues )  { return ( eRefIdxValues == m_scRef ); }
  Bool operator!= ( const RefIdxValues eRefIdxValues )  { return ( eRefIdxValues != m_scRef ); }

  operator const SChar()                                { return m_scRef; }

  const Mv3D& operator= (const Mv& rcMv)  { set( rcMv.getHor(), rcMv.getVer(), BLOCK_NOT_AVAILABLE); return *this; }

private:
  SChar m_scRef;
};


H264AVC_NAMESPACE_END


#endif // !defined(AFX_MV_H__91E1E1F3_0D6C_4222_9D71_EAB2E415688C__INCLUDED_)
