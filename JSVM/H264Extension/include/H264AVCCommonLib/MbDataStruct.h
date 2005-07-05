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






#if !defined(AFX_MBDATASTRUCT_H__353D9AA9_2CC4_4959_94DB_97456E3C2454__INCLUDED_)
#define AFX_MBDATASTRUCT_H__353D9AA9_2CC4_4959_94DB_97456E3C2454__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


H264AVC_NAMESPACE_BEGIN


class H264AVCCOMMONLIB_API MbDataStruct
{
public:
  MbDataStruct();

  Void reset();
  Void initMbData( UChar ucQp, UInt uiSliceId )                 { m_uiSliceId = uiSliceId;  m_ucQp = ucQp;  }
  Void copyFrom( const MbDataStruct& rcMbDataStruct );
  Void clear();
  UChar getQpLF()                                         const { return isPCM() ? 0 : m_ucQp; }
  UChar getQp()                                           const { return m_ucQp; }
  Void  setQp( UChar ucQp )                                     { m_ucQp = ucQp; }
  Void  clearIntraPredictionModes( Bool bAll );

  Bool  isIntra   ( Par8x8  ePar8x8 )                     const { return getBlockFwdBwd( ePar8x8 ) == 0; }
  Bool  isInterP  ( Par8x8  ePar8x8 )                     const { return getBlockFwdBwd( ePar8x8 ) == 1; }

  Bool  isIntra   ( LumaIdx cIdx    )                     const { return isIntra ( Par8x8( ((cIdx.y()>>1)<<1) + (cIdx.x()>>1) ) ) ; }
  Bool  isInterP  ( LumaIdx cIdx    )                     const { return isInterP( Par8x8( ((cIdx.y()>>1)<<1) + (cIdx.x()>>1) ) ) ; }

  UInt  getFwdBwd ( LumaIdx cIdx    )                     const { return getBlockFwdBwd( Par8x8( ((cIdx.y()>>1)<<1) + (cIdx.x()>>1) ) ); }

  const Bool isInterPMb()                                 const { return m_usFwdBwd == 0x1111; }
  Bool isBlockFwdBwd  ( Par8x8 ePar8x8, ListIdx eLstIdx ) const { const UInt ui = 1<<(eLstIdx + (ePar8x8<<2)); return ( m_usFwdBwd & ui) == ui; }
  UInt getBlockFwdBwd ( Par8x8 ePar8x8 )                  const { return ( m_usFwdBwd >> (ePar8x8<<2) ) & 3; }
  BlkMode getBlkMode( Par8x8 ePar8x8 )                    const { return m_aBlkMode[ePar8x8]; }
  Void setBlkMode( Par8x8 ePar8x8, BlkMode eBlkMode )           { m_aBlkMode[ePar8x8] = eBlkMode; }
  Bool is4x4BlkCoded  ( LumaIdx cLumaIdx )                const { return (0 != ((m_uiMbCbp >> cLumaIdx) & 1)); }
  UInt getCbpChroma4x4()                                  const { return m_uiMbCbp >> 28; }
  UInt getMbCbp       ()                                  const { return m_uiMbCbp >> 24; }
  UInt getMbExtCbp    ()                                  const { return m_uiMbCbp; }
  Void setMbExtCbp    ( UInt uiCbp )                            { m_uiMbCbp = uiCbp; }
  Void setAndConvertMbExtCbp( UInt uiExtCbp );
  Void setMbCbp       ( UInt uiCbp );
  MbMode  getMbMode         ()                            const { return m_eMbMode; }
  Bool    isInter8x8        ()                            const { return m_eMbMode == INTRA_4X4-2 || m_eMbMode == INTRA_4X4-1; }
  Bool    isIntra4x4        ()                            const { return m_eMbMode == INTRA_4X4; }
  Bool    isIntra16x16      ()                            const { return m_eMbMode  > INTRA_4X4 && m_eMbMode < MODE_PCM; }
  Bool    isIntra           ()                            const { return m_eMbMode >= INTRA_4X4; }
  Bool    isSkiped          ()                            const { return m_eMbMode == MODE_SKIP; }
  Bool    isPCM             ()                            const { return m_eMbMode == MODE_PCM; }
  Bool    isAcCoded         ()                            const { AOF_DBG(isIntra16x16()); return m_eMbMode>=(INTRA_4X4 + 13); }
  UChar   intraPredMode     ()                            const { AOF_DBG(isIntra16x16()); return (m_eMbMode-(INTRA_4X4+1)) & 3; }
  SChar&  intraPredMode(LumaIdx cIdx)                           { return m_ascIPredMode[cIdx]; }
  SChar   intraPredMode(LumaIdx cIdx)                     const { return m_ascIPredMode[cIdx]; }
  UInt    getCbpChroma16x16 ()                            const { AOF_DBG(isIntra16x16()); return m_aucACTab[(m_eMbMode-(INTRA_4X4+1))>>2 ]; }
  Void setMbMode( MbMode eMbMode )                              { m_eMbMode = eMbMode; }
  UInt getSliceId()                                       const { return m_uiSliceId;}
  Void setChromaPredMode( UChar ucChromaPredMode )              { m_ucChromaPredMode = ucChromaPredMode; }
  UChar getChromaPredMode()                               const { return m_ucChromaPredMode; }
  Void setFwdBwd( UShort usFwdBwd )                             { m_usFwdBwd = usFwdBwd; }
  UShort getFwdBwd()                                      const { return m_usFwdBwd; }
  Void addFwdBwd( Par8x8 ePar8x8, UInt uiFwdBwdBlk )            { m_usFwdBwd |= uiFwdBwdBlk<<(ePar8x8*4); }
  Void setBCBPAll( UInt uiBit )                                 { m_uiBCBP = (uiBit) ? 0xffff : 0; }
  UInt getBCBP( UInt uiPos )                              const { return ((m_uiBCBP >> uiPos) & 1); }
  Void setBCBP( UInt uiPos, UInt uiBit )                        { m_uiBCBP |= (uiBit << uiPos); }
  Void setBCBP( UInt uiBCBP )                                   { m_uiBCBP = uiBCBP; }
  UInt getBCBP()                                          const { return m_uiBCBP; }
  Bool getSkipFlag()                                      const { return m_bSkipFlag; }
  Void setSkipFlag( Bool b)                                     { m_bSkipFlag = b; }


  UShort  getResidualPredFlags  ()                        const { return   m_usResidualPredFlags; }
  Bool    getResidualPredFlag   ( LumaIdx     cIdx      ) const { return ( m_usResidualPredFlags & ( 1 << cIdx.b4x4() ) ) != 0; }
  Bool    getResidualPredFlag   ( ParIdx16x16 eParIdx   ) const { return getResidualPredFlag( B4x4Idx( eParIdx ) ); }
  Bool    getResidualPredFlag   ( ParIdx16x8  eParIdx   ) const { return getResidualPredFlag( B4x4Idx( eParIdx ) ); }
  Bool    getResidualPredFlag   ( ParIdx8x16  eParIdx   ) const { return getResidualPredFlag( B4x4Idx( eParIdx ) ); }
  Bool    getResidualPredFlag   ( ParIdx8x8   eParIdx,
                                  SParIdx8x8  eSParIdx  ) const { return getResidualPredFlag( B4x4Idx( eParIdx+eSParIdx ) ); }
  Bool    getResidualPredFlag   ( ParIdx8x8   eParIdx,
                                  SParIdx8x4  eSParIdx  ) const { return getResidualPredFlag( B4x4Idx( eParIdx+eSParIdx ) ); }
  Bool    getResidualPredFlag   ( ParIdx8x8   eParIdx,
                                  SParIdx4x8  eSParIdx  ) const { return getResidualPredFlag( B4x4Idx( eParIdx+eSParIdx ) ); }
  Bool    getResidualPredFlag   ( ParIdx8x8   eParIdx,
                                  SParIdx4x4  eSParIdx  ) const { return getResidualPredFlag( B4x4Idx( eParIdx+eSParIdx ) ); }
// TMM_ESS 
  Bool    getInCropWindowFlag   ()                        const { return   m_bInCropWindowFlag; }


  Void    setResidualPredFlags  ( UShort      usFlags   )       { m_usResidualPredFlags = usFlags; } 
  Void    setResidualPredFlag   ( Bool        bFlag,
                                  LumaIdx     cIdx      );
  Void    setResidualPredFlag   ( Bool        bFlag,
                                  ParIdx16x16 eParIdx   );
  Void    setResidualPredFlag   ( Bool        bFlag,
                                  ParIdx16x8  eParIdx   );
  Void    setResidualPredFlag   ( Bool        bFlag,
                                  ParIdx8x16  eParIdx   );
  Void    setResidualPredFlag   ( Bool        bFlag,
                                  ParIdx8x8   eParIdx,
                                  SParIdx8x8  eSParIdx  );
  Void    setResidualPredFlag   ( Bool        bFlag,
                                  ParIdx8x8   eParIdx,
                                  SParIdx8x4  eSParIdx  );
  Void    setResidualPredFlag   ( Bool        bFlag,
                                  ParIdx8x8   eParIdx,
                                  SParIdx4x8  eSParIdx  );
  Void    setResidualPredFlag   ( Bool        bFlag,
                                  ParIdx8x8   eParIdx,
                                  SParIdx4x4  eSParIdx  );
// TMM_ESS 
  Void    setInCropWindowFlag   ( Bool        bFlag  )       { m_bInCropWindowFlag = bFlag; } 


  ErrVal  save( FILE* pFile );
  ErrVal  load( FILE* pFile );


  ErrVal  upsampleMotion    ( const MbDataStruct& rcMbDataStruct,
                              Par8x8              ePar8x8, Bool bDirect8x8 );

  Void    setBLSkipFlag         ( Bool b )  { m_bBLSkipFlag = b; }
  Void    setBLQRefFlag         ( Bool b )  { m_bBLQRefFlag = b; }
  Bool    getBLSkipFlag         () const    { return m_bBLSkipFlag; }
  Bool    getBLQRefFlag         () const    { return m_bBLQRefFlag; }

  Bool is8x8TrafoFlagPresent()                          const;
  Bool isTransformSize8x8   ()                          const     { return m_bTransformSize8x8; }
  Void setTransformSize8x8  ( Bool bTransformSize8x8)             { m_bTransformSize8x8 = bTransformSize8x8; }

protected:
  UInt    m_uiSliceId;
  Bool    m_bBLSkipFlag;
  Bool    m_bBLQRefFlag;
  MbMode  m_eMbMode;
  UInt    m_uiMbCbp;
  UInt    m_uiBCBP;
  UShort  m_usFwdBwd;
  UChar   m_ucChromaPredMode;
  BlkMode m_aBlkMode[4];
  SChar   m_ascIPredMode[16];
  UChar   m_ucQp;

  UShort  m_usResidualPredFlags;

  Bool    m_bTransformSize8x8;
  Bool    m_bSkipFlag;

  // TMM_ESS 
  Bool    m_bInCropWindowFlag;  // indicates if the scaled base layer MB is inside the cropping window

protected:
  static const UChar m_aucACTab[6];
};




__inline
Void
MbDataStruct::setResidualPredFlag( Bool bFlag, LumaIdx cIdx )
{
  m_usResidualPredFlags &= ~(UShort)(1<<cIdx.b4x4());
  ROFVS( bFlag );
  m_usResidualPredFlags += (1<<cIdx.b4x4());
}


__inline
Void
MbDataStruct::setResidualPredFlag( Bool bFlag, ParIdx16x16 eParIdx )
{
  setResidualPredFlag( bFlag, B4x4Idx( 0) );  
  setResidualPredFlag( bFlag, B4x4Idx( 1) );  
  setResidualPredFlag( bFlag, B4x4Idx( 2) );  
  setResidualPredFlag( bFlag, B4x4Idx( 3) );  
  setResidualPredFlag( bFlag, B4x4Idx( 4) );  
  setResidualPredFlag( bFlag, B4x4Idx( 5) );  
  setResidualPredFlag( bFlag, B4x4Idx( 6) );  
  setResidualPredFlag( bFlag, B4x4Idx( 7) );  
  setResidualPredFlag( bFlag, B4x4Idx( 8) );  
  setResidualPredFlag( bFlag, B4x4Idx( 9) );  
  setResidualPredFlag( bFlag, B4x4Idx(10) );  
  setResidualPredFlag( bFlag, B4x4Idx(11) );  
  setResidualPredFlag( bFlag, B4x4Idx(12) );  
  setResidualPredFlag( bFlag, B4x4Idx(13) );  
  setResidualPredFlag( bFlag, B4x4Idx(14) );  
  setResidualPredFlag( bFlag, B4x4Idx(15) );  
}

__inline
Void
MbDataStruct::setResidualPredFlag( Bool bFlag, ParIdx16x8 eParIdx )
{
  UInt ui = eParIdx;

  setResidualPredFlag( bFlag, B4x4Idx(ui+ 0) );  
  setResidualPredFlag( bFlag, B4x4Idx(ui+ 1) );  
  setResidualPredFlag( bFlag, B4x4Idx(ui+ 2) );  
  setResidualPredFlag( bFlag, B4x4Idx(ui+ 3) );  
  setResidualPredFlag( bFlag, B4x4Idx(ui+ 4) );  
  setResidualPredFlag( bFlag, B4x4Idx(ui+ 5) );  
  setResidualPredFlag( bFlag, B4x4Idx(ui+ 6) );  
  setResidualPredFlag( bFlag, B4x4Idx(ui+ 7) );  
}

__inline
Void
MbDataStruct::setResidualPredFlag( Bool bFlag, ParIdx8x16 eParIdx )
{
  UInt ui = eParIdx;

  setResidualPredFlag( bFlag, B4x4Idx(ui+ 0) );  
  setResidualPredFlag( bFlag, B4x4Idx(ui+ 1) );  
  setResidualPredFlag( bFlag, B4x4Idx(ui+ 4) );  
  setResidualPredFlag( bFlag, B4x4Idx(ui+ 5) );  
  setResidualPredFlag( bFlag, B4x4Idx(ui+ 8) );  
  setResidualPredFlag( bFlag, B4x4Idx(ui+ 9) );  
  setResidualPredFlag( bFlag, B4x4Idx(ui+12) );  
  setResidualPredFlag( bFlag, B4x4Idx(ui+13) );  
}

__inline
Void
MbDataStruct::setResidualPredFlag( Bool bFlag, ParIdx8x8 eParIdx, SParIdx8x8 eSParIdx )
{
  UInt ui = eParIdx+eSParIdx;

  setResidualPredFlag( bFlag, B4x4Idx(ui+ 0) );  
  setResidualPredFlag( bFlag, B4x4Idx(ui+ 1) );  
  setResidualPredFlag( bFlag, B4x4Idx(ui+ 4) );  
  setResidualPredFlag( bFlag, B4x4Idx(ui+ 5) );  
}

__inline
Void
MbDataStruct::setResidualPredFlag( Bool bFlag, ParIdx8x8 eParIdx, SParIdx8x4 eSParIdx )
{
  UInt ui = eParIdx+eSParIdx;

  setResidualPredFlag( bFlag, B4x4Idx(ui+ 0) );  
  setResidualPredFlag( bFlag, B4x4Idx(ui+ 1) );  
}

__inline
Void
MbDataStruct::setResidualPredFlag( Bool bFlag, ParIdx8x8 eParIdx, SParIdx4x8 eSParIdx )
{
  UInt ui = eParIdx+eSParIdx;

  setResidualPredFlag( bFlag, B4x4Idx(ui+ 0) );  
  setResidualPredFlag( bFlag, B4x4Idx(ui+ 4) );  
}

__inline
Void
MbDataStruct::setResidualPredFlag( Bool bFlag, ParIdx8x8 eParIdx, SParIdx4x4 eSParIdx )
{
  UInt ui = eParIdx+eSParIdx;

  setResidualPredFlag( bFlag, B4x4Idx(ui+ 0) );  
}



H264AVC_NAMESPACE_END

#endif // !defined(AFX_MBDATASTRUCT_H__353D9AA9_2CC4_4959_94DB_97456E3C2454__INCLUDED_)
