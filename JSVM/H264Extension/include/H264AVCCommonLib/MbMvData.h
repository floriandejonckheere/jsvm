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






#if !defined(AFX_MBMVDATA_H__06960F25_0FB8_4A65_935D_B06282FFDF6E__INCLUDED_)
#define AFX_MBMVDATA_H__06960F25_0FB8_4A65_935D_B06282FFDF6E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


H264AVC_NAMESPACE_BEGIN

class H264AVCCOMMONLIB_API MbMvData
{
public:
  Void copyFrom( const MbMvData& rcMbMvData, const ParIdx8x8 eParIdx );
  Void copyFrom( const MbMvData& rcMbMvData );

  MbMvData()
  {
    clear();  
  }

  Void clear()
  {
    m_acMv[ 0 ].setZero();
    m_acMv[ 1 ].setZero();
    m_acMv[ 2 ].setZero();
    m_acMv[ 3 ].setZero();
    m_acMv[ 4 ].setZero();
    m_acMv[ 5 ].setZero();
    m_acMv[ 6 ].setZero();
    m_acMv[ 7 ].setZero();
    m_acMv[ 8 ].setZero();
    m_acMv[ 9 ].setZero();
    m_acMv[10 ].setZero();
    m_acMv[11 ].setZero();
    m_acMv[12 ].setZero();
    m_acMv[13 ].setZero();
    m_acMv[14 ].setZero();
    m_acMv[15 ].setZero();
  }

  Void clear( ParIdx8x8 eParIdx )
  {
    Mv* pcMv = &m_acMv[ eParIdx ];
    pcMv[ 0 ].setZero();
    pcMv[ 1 ].setZero();
    pcMv[ 4 ].setZero();
    pcMv[ 5 ].setZero();
  }

  Void  setFirstMv( const Mv& rcMv )                                          { m_acMv[ 0 ] = rcMv; }
  Void  setFirstMv( const Mv& rcMv, ParIdx16x8 eParIdx  )                     { m_acMv[ eParIdx ] = rcMv; }
  Void  setFirstMv( const Mv& rcMv, ParIdx8x16 eParIdx  )                     { m_acMv[ eParIdx ] = rcMv; }
  Void  setFirstMv( const Mv& rcMv, ParIdx8x8  eParIdx  )                     { m_acMv[ eParIdx ] = rcMv; }
  Void  setFirstMv( const Mv& rcMv, ParIdx8x8  eParIdx, SParIdx8x4 eSParIdx ) { m_acMv[ eParIdx+eSParIdx ] = rcMv; }
  Void  setFirstMv( const Mv& rcMv, ParIdx8x8  eParIdx, SParIdx4x8 eSParIdx ) { m_acMv[ eParIdx+eSParIdx ] = rcMv; }
  Void  setFirstMv( const Mv& rcMv, ParIdx8x8  eParIdx, SParIdx4x4 eSParIdx ) { m_acMv[ eParIdx+eSParIdx ] = rcMv; }

  Void  setAllMv( const Mv& rcMv );
  Void  setAllMv( const Mv& rcMv, ParIdx16x8 eParIdx  );
  Void  setAllMv( const Mv& rcMv, ParIdx8x16 eParIdx  );
  Void  setAllMv( const Mv& rcMv, ParIdx8x8  eParIdx  );
  Void  setAllMv( const Mv& rcMv, ParIdx8x8  eParIdx, SParIdx8x4 eSParIdx );
  Void  setAllMv( const Mv& rcMv, ParIdx8x8  eParIdx, SParIdx4x8 eSParIdx );
  Void  setAllMv( const Mv& rcMv, ParIdx8x8  eParIdx, SParIdx4x4 eSParIdx );

  Void  setMv( const Mv& rcMv, LumaIdx cIdx );

  const Mv& getMv()                                           const { return m_acMv[ 0 ]; }
  const Mv& getMv( ParIdx16x8 eParIdx   )                     const { return m_acMv[ eParIdx ]; }
  const Mv& getMv( ParIdx8x16 eParIdx   )                     const { return m_acMv[ eParIdx ]; }
  const Mv& getMv( ParIdx8x8  eParIdx   )                     const { return m_acMv[ eParIdx ]; }
  const Mv& getMv( ParIdx8x8  eParIdx, SParIdx8x4 eSParIdx )  const { return m_acMv[ eParIdx + eSParIdx ]; }
  const Mv& getMv( ParIdx8x8  eParIdx, SParIdx4x8 eSParIdx )  const { return m_acMv[ eParIdx + eSParIdx ]; }
  const Mv& getMv( ParIdx8x8  eParIdx, SParIdx4x4 eSParIdx )  const { return m_acMv[ eParIdx + eSParIdx ]; }
  const Mv& getMv( LumaIdx cIdx )                             const { return m_acMv[ cIdx.b4x4() ]; }


  ErrVal  save( FILE* pFile );
  ErrVal  load( FILE* pFile );

  ErrVal  upsampleMotion( const MbMvData& rcMbMvData, Par8x8 ePar8x8 );

private:
  MbMvData( const MbMvData& )                   {}

public:
  Mv  m_acMv[16];
};


#if defined( WIN32 )
# pragma warning( disable: 4251 )
#endif

class H264AVCCOMMONLIB_API MbMotionData :
public MbMvData
{
public:
  Void copyFrom( const MbMotionData& rcMbMotionData, const ParIdx8x8  eParIdx  );
  Void copyFrom( const MbMotionData& rcMbMotionData );

  MbMotionData()
    : MbMvData        (        ),
      m_usMotPredFlags( 0x0000 )
  {
    m_ascRefIdx[ 0 ] = m_ascRefIdx[ 1 ] = m_ascRefIdx[ 2 ] = m_ascRefIdx[ 3 ] = BLOCK_NOT_AVAILABLE;
    m_usMotPredFlags=0; 
  
    m_acRefPic[ 0 ].setFrame( NULL ); 
    m_acRefPic[ 1 ].setFrame( NULL );
    m_acRefPic[ 2 ].setFrame( NULL );
    m_acRefPic[ 3 ].setFrame( NULL );
  }


  Void reset()
  {
    clear( BLOCK_NOT_AVAILABLE );
    m_acRefPic[ 0 ].setFrame( NULL );
    m_acRefPic[ 1 ].setFrame( NULL );
    m_acRefPic[ 2 ].setFrame( NULL );
    m_acRefPic[ 3 ].setFrame( NULL );
  }

  Void clear( RefIdxValues eRefIdxValues )
  {
    MbMvData::clear();
    m_usMotPredFlags = 0x0000;
    m_ascRefIdx[ 0 ] = m_ascRefIdx[ 1 ] = m_ascRefIdx[ 2 ] = m_ascRefIdx[ 3 ] = eRefIdxValues;
  }

  Void  setRefIdx( SChar scRefIdx );
  Void  setRefIdx( SChar scRefIdx, ParIdx16x8 eParIdx  );
  Void  setRefIdx( SChar scRefIdx, ParIdx8x16 eParIdx  );
  Void  setRefIdx( SChar scRefIdx, ParIdx8x8  eParIdx  );
  
  SChar getRefIdx()                      const  { return m_ascRefIdx[ 0         ]; }
  SChar getRefIdx( ParIdx16x8 eParIdx  ) const  { return m_ascRefIdx[ m_auiBlk2Part[ eParIdx ] ]; }
  SChar getRefIdx( ParIdx8x16 eParIdx  ) const  { return m_ascRefIdx[ m_auiBlk2Part[ eParIdx ] ]; }
  SChar getRefIdx( ParIdx8x8  eParIdx  ) const  { return m_ascRefIdx[ m_auiBlk2Part[ eParIdx ] ]; }
  SChar getRefIdx( LumaIdx cIdx )        const  { return m_ascRefIdx[ m_auiBlk2Part[ cIdx.b4x4() ] ]; }
  SChar getRefIdx( Par8x8 ePar8x8 )      const  { return m_ascRefIdx[ ePar8x8]; }


  Bool  getMotPredFlag()                     const  { return xGetMotPredFlag( 0 ); }
  Bool  getMotPredFlag( ParIdx16x8 eParIdx ) const  { return xGetMotPredFlag( eParIdx ); }
  Bool  getMotPredFlag( ParIdx8x16 eParIdx ) const  { return xGetMotPredFlag( eParIdx ); }
  Bool  getMotPredFlag( ParIdx8x8  eParIdx ) const  { return xGetMotPredFlag( eParIdx ); }
  Bool  getMotPredFlag( LumaIdx    cIdx    ) const  { return xGetMotPredFlag( cIdx.b4x4() ); }

  Void  setMotPredFlag( Bool bFlag );
  Void  setMotPredFlag( Bool bFlag, ParIdx16x8 eParIdx );
  Void  setMotPredFlag( Bool bFlag, ParIdx8x16 eParIdx );
  Void  setMotPredFlag( Bool bFlag, ParIdx8x8  eParIdx );
  Void  setMotPredFlag( Bool bFlag, LumaIdx    cIdx    );



  Void  setRefPic( const Frame* pcRefFrame )
  {
    m_acRefPic[ 0 ].setFrame( pcRefFrame );
    m_acRefPic[ 1 ].setFrame( pcRefFrame );
    m_acRefPic[ 2 ].setFrame( pcRefFrame );
    m_acRefPic[ 3 ].setFrame( pcRefFrame );
  }
  Void  setRefPic( const Frame* pcRefFrame, ParIdx16x8 eParIdx  )
  {
    UInt uiOffset = m_auiBlk2Part[ eParIdx ];
    m_acRefPic[ uiOffset     ].setFrame( pcRefFrame );
    m_acRefPic[ uiOffset + 1 ].setFrame( pcRefFrame );
  }
  Void  setRefPic( const Frame* pcRefFrame, ParIdx8x16 eParIdx  )
  {
    UInt uiOffset = m_auiBlk2Part[ eParIdx ];
    m_acRefPic[ uiOffset     ].setFrame( pcRefFrame );
    m_acRefPic[ uiOffset + 2 ].setFrame( pcRefFrame );
  }
  Void  setRefPic( const Frame* pcRefFrame, ParIdx8x8  eParIdx  )
  {
    m_acRefPic[ m_auiBlk2Part[ eParIdx ] ].setFrame( pcRefFrame );
  }

  const RefPic& getRefPic( ParIdx8x8 eParIdx )        const { return m_acRefPic[ m_auiBlk2Part[ eParIdx ] ];  }
  const RefPic& getRefPic( LumaIdx cIdx )             const { return m_acRefPic[ m_auiBlk2Part[ cIdx.b4x4() ] ];  }

  Void  getMvRef         ( Mv& rcMv, SChar& rscRef, LumaIdx cIdx )                            const;
  Void  getMv3D          ( Mv3D& rcMv3D,            LumaIdx cIdx )                            const;
  Void  getMvRefNeighbour( Mv& rcMv, SChar& rscRef, LumaIdx cIdx )    const;
  Void  getMv3DNeighbour ( Mv3D& rcMv3D,            LumaIdx cIdx )    const;

  ErrVal  save( FILE* pFile );
  ErrVal  load( FILE* pFile );

  
  ErrVal  upsampleMotion( const MbMotionData& rcMbMvData, Par8x8 ePar8x8 );

// TMM_ESS {
  ErrVal upsampleMotionNonDyad( SChar* pscBl4x4RefIdx  , Mv* acBl4x4Mv , ResizeParameters* pcParameters );
  ErrVal upsampleMotionNonDyad( SChar* scBl8x8RefIdx , Mv* acBl4x4Mv , ResizeParameters* pcParameters , Mv deltaMv[4] ); 
// TMM_ESS }
private:
  Bool  xGetMotPredFlag ( UInt  uiPos )  const
  {
    return ( ( m_usMotPredFlags & (1 << uiPos) ) >> uiPos ? true : false );
  }
  Void  xSetMotPredFlag ( Bool  bFlag, UInt  uiPos )
  {
    AOF(uiPos<16);
    m_usMotPredFlags &= ~(UShort)(1<<uiPos);
    ROFVS( bFlag );
    m_usMotPredFlags += (1<<uiPos);
  }

private:
  MbMotionData( const MbMotionData& )  {}

private:
  static const UInt   m_auiBlk2Part[16];

public:
  SChar   m_ascRefIdx[4];
  RefPic  m_acRefPic [4];
  UShort  m_usMotPredFlags;
};





#if defined( WIN32 )
# pragma warning( default: 4251 )
#endif



__inline 
Void MbMotionData::setRefIdx( SChar scRefIdx )
{
  m_ascRefIdx[ 0 ] = m_ascRefIdx[ 1 ] = m_ascRefIdx[ 2 ] = m_ascRefIdx[ 3 ] = scRefIdx;
}

__inline 
Void MbMotionData::setRefIdx( SChar scRefIdx, ParIdx16x8 eParIdx )
{
  m_ascRefIdx[ m_auiBlk2Part[eParIdx]   ] = scRefIdx;
  m_ascRefIdx[ m_auiBlk2Part[eParIdx]+1 ] = scRefIdx;
}

__inline 
Void MbMotionData::setRefIdx( SChar scRefIdx, ParIdx8x16 eParIdx )
{
  m_ascRefIdx[ m_auiBlk2Part[eParIdx]   ] = scRefIdx;
  m_ascRefIdx[ m_auiBlk2Part[eParIdx]+2 ] = scRefIdx;
}

__inline Void MbMotionData::setRefIdx( SChar scRefIdx, ParIdx8x8 eParIdx )
{
  m_ascRefIdx[ m_auiBlk2Part[eParIdx]   ] = scRefIdx;
}


__inline Void MbMvData::setAllMv( const Mv& rcMv )
{
  const register Mv cMv = rcMv;
  m_acMv[ 0 ] = cMv;
  m_acMv[ 1 ] = cMv;
  m_acMv[ 2 ] = cMv;
  m_acMv[ 3 ] = cMv;
  m_acMv[ 4 ] = cMv;
  m_acMv[ 5 ] = cMv;
  m_acMv[ 6 ] = cMv;
  m_acMv[ 7 ] = cMv;
  m_acMv[ 8 ] = cMv;
  m_acMv[ 9 ] = cMv;
  m_acMv[ 10 ] = cMv;
  m_acMv[ 11 ] = cMv;
  m_acMv[ 12 ] = cMv;
  m_acMv[ 13 ] = cMv;
  m_acMv[ 14 ] = cMv;
  m_acMv[ 15 ] = cMv;
}

__inline Void MbMvData::setAllMv( const Mv& rcMv, ParIdx16x8 eParIdx  )
{
  Mv* pcMv = m_acMv + eParIdx;
  const register Mv cMv = rcMv;
  pcMv[ 0 ] = cMv;
  pcMv[ 1 ] = cMv;
  pcMv[ 2 ] = cMv;
  pcMv[ 3 ] = cMv;
  pcMv[ 4 ] = cMv;
  pcMv[ 5 ] = cMv;
  pcMv[ 6 ] = cMv;
  pcMv[ 7 ] = cMv;
}


__inline Void MbMvData::setAllMv( const Mv& rcMv, ParIdx8x16 eParIdx  )
{
  Mv* pcMv = m_acMv + eParIdx;
  const register Mv cMv = rcMv;
  pcMv[ 0 ] = cMv;
  pcMv[ 1 ] = cMv;
  pcMv[ 4 ] = cMv;
  pcMv[ 5 ] = cMv;
  pcMv[ 8 ] = cMv;
  pcMv[ 9 ] = cMv;
  pcMv[ 12 ] = cMv;
  pcMv[ 13 ] = cMv;
}

__inline Void MbMvData::setAllMv( const Mv& rcMv, ParIdx8x8 eParIdx  )
{
  Mv* pcMv = m_acMv + eParIdx;
  const register Mv cMv = rcMv;
  pcMv[ 0 ] = cMv;
  pcMv[ 1 ] = cMv;
  pcMv[ 4 ] = cMv;
  pcMv[ 5 ] = cMv;
}

__inline Void MbMvData::setAllMv( const Mv& rcMv, ParIdx8x8 eParIdx, SParIdx8x4 eSParIdx )
{
  Mv* pcMv = m_acMv + eParIdx + eSParIdx;
  const register Mv cMv = rcMv;
  pcMv[ 0 ] = cMv;
  pcMv[ 1 ] = cMv;
}

__inline Void MbMvData::setAllMv( const Mv& rcMv, ParIdx8x8 eParIdx, SParIdx4x8 eSParIdx )
{
  Mv* pcMv = m_acMv + eParIdx + eSParIdx;
  const register Mv cMv = rcMv;
  pcMv[ 0 ] = cMv;
  pcMv[ 4 ] = cMv;
}

__inline Void MbMvData::setAllMv( const Mv& rcMv, ParIdx8x8 eParIdx, SParIdx4x4 eSParIdx )
{
  Mv* pcMv = m_acMv + eParIdx + eSParIdx;
  pcMv[ 0 ] = rcMv;
}




__inline  Void  MbMotionData::setMotPredFlag( Bool bFlag )
{
  xSetMotPredFlag( bFlag,  0 );
  xSetMotPredFlag( bFlag,  1 );
  xSetMotPredFlag( bFlag,  2 );
  xSetMotPredFlag( bFlag,  3 );
  xSetMotPredFlag( bFlag,  4 );
  xSetMotPredFlag( bFlag,  5 );
  xSetMotPredFlag( bFlag,  6 );
  xSetMotPredFlag( bFlag,  7 );
  xSetMotPredFlag( bFlag,  8 );
  xSetMotPredFlag( bFlag,  9 );
  xSetMotPredFlag( bFlag, 10 );
  xSetMotPredFlag( bFlag, 11 );
  xSetMotPredFlag( bFlag, 12 );
  xSetMotPredFlag( bFlag, 13 );
  xSetMotPredFlag( bFlag, 14 );
  xSetMotPredFlag( bFlag, 15 );
}

__inline  Void  MbMotionData::setMotPredFlag( Bool bFlag, ParIdx16x8 eParIdx )
{
  UInt ui = eParIdx;

  xSetMotPredFlag( bFlag, ui+ 0 );
  xSetMotPredFlag( bFlag, ui+ 1 );
  xSetMotPredFlag( bFlag, ui+ 2 );
  xSetMotPredFlag( bFlag, ui+ 3 );
  xSetMotPredFlag( bFlag, ui+ 4 );
  xSetMotPredFlag( bFlag, ui+ 5 );
  xSetMotPredFlag( bFlag, ui+ 6 );
  xSetMotPredFlag( bFlag, ui+ 7 );
}

__inline  Void  MbMotionData::setMotPredFlag( Bool bFlag, ParIdx8x16 eParIdx )
{
  UInt ui = eParIdx;

  xSetMotPredFlag( bFlag, ui+ 0 );
  xSetMotPredFlag( bFlag, ui+ 1 );
  xSetMotPredFlag( bFlag, ui+ 4 );
  xSetMotPredFlag( bFlag, ui+ 5 );
  xSetMotPredFlag( bFlag, ui+ 8 );
  xSetMotPredFlag( bFlag, ui+ 9 );
  xSetMotPredFlag( bFlag, ui+12 );
  xSetMotPredFlag( bFlag, ui+13 );
}

__inline  Void  MbMotionData::setMotPredFlag( Bool bFlag, ParIdx8x8  eParIdx )
{
  UInt ui = eParIdx;

  xSetMotPredFlag( bFlag, ui+ 0 );
  xSetMotPredFlag( bFlag, ui+ 1 );
  xSetMotPredFlag( bFlag, ui+ 4 );
  xSetMotPredFlag( bFlag, ui+ 5 );
}

__inline  Void  MbMotionData::setMotPredFlag( Bool bFlag, LumaIdx    cIdx    )
{
  xSetMotPredFlag( bFlag, cIdx.b4x4() );
}





__inline Void MbMvData::setMv( const Mv& rcMv, LumaIdx cIdx )
{
  m_acMv[ cIdx.b4x4() ] = rcMv;
}

__inline Void MbMotionData::getMvRef( Mv& rcMv, SChar& rscRef, LumaIdx cIdx ) const
{
  rcMv   = m_acMv[ cIdx.b4x4() ];
  rscRef = m_ascRefIdx[ m_auiBlk2Part[ cIdx.b4x4() ] ];
}

__inline Void MbMotionData::getMv3D( Mv3D& rcMv3D, LumaIdx cIdx ) const
{
  rcMv3D.set( m_acMv[ cIdx.b4x4() ], m_ascRefIdx[ m_auiBlk2Part[ cIdx.b4x4()] ] );
}

__inline Void MbMotionData::getMvRefNeighbour( Mv& rcMv, SChar& rscRef, LumaIdx cIdx ) const
{
  rcMv   = m_acMv[ cIdx.b4x4() ];
  rscRef = m_ascRefIdx[ m_auiBlk2Part[ cIdx.b4x4()] ];
}

__inline Void MbMotionData::getMv3DNeighbour( Mv3D& rcMv3D, LumaIdx cIdx ) const
{
  rcMv3D.set( m_acMv[ cIdx.b4x4() ], m_ascRefIdx[ m_auiBlk2Part[ cIdx.b4x4()] ] );
}

H264AVC_NAMESPACE_END


#endif // !defined(AFX_MBMVDATA_H__06960F25_0FB8_4A65_935D_B06282FFDF6E__INCLUDED_)
