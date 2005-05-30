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






#if !defined(AFX_MBTRANSFORMCOEFFS_H__B5704512_C23C_497A_A794_36691A0D01BE__INCLUDED_)
#define AFX_MBTRANSFORMCOEFFS_H__B5704512_C23C_497A_A794_36691A0D01BE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

H264AVC_NAMESPACE_BEGIN

class H264AVCCOMMONLIB_API MbTransformCoeffs
{
public:
  MbTransformCoeffs() ;
  
  TCoeff*       get   ( LumaIdx   cLumaIdx )          { return &m_aaiLevel[cLumaIdx     ][0]; }
  const TCoeff* get   ( LumaIdx   cLumaIdx )    const { return &m_aaiLevel[cLumaIdx     ][0]; }

  TCoeff*       get   ( ChromaIdx cChromaIdx )        { return &m_aaiLevel[16+cChromaIdx][0]; }
  const TCoeff* get   ( ChromaIdx cChromaIdx )  const { return &m_aaiLevel[16+cChromaIdx][0]; }

  TCoeff*       get8x8( B8x8Idx   c8x8Idx )           { return &m_aaiLevel[4*c8x8Idx.b8x8Index()][0]; }
  const TCoeff* get8x8( B8x8Idx   c8x8Idx )     const { return &m_aaiLevel[4*c8x8Idx.b8x8Index()][0]; }

  Pel*          getPelBuffer()                        { return (Pel*) &m_aaiLevel[0][0]; }
  const Pel*    getPelBuffer()                  const { return (Pel*) &m_aaiLevel[0][0]; }

  UInt getCoeffCount( LumaIdx cLumaIdx )                   const { return m_aaucCoeffCount[cLumaIdx]; }
  UInt getCoeffCount( ChromaIdx cChromaIdx )               const { return m_aaucCoeffCount[16+cChromaIdx]; }
  Void setCoeffCount( LumaIdx cLumaIdx, UInt uiCoeffCount )      { m_aaucCoeffCount[cLumaIdx] = uiCoeffCount; }
  Void setCoeffCount( ChromaIdx cChromaIdx, UInt uiCoeffCount )  { m_aaucCoeffCount[16+cChromaIdx] = uiCoeffCount; }
  Void clear();
  Void clearAcBlk( ChromaIdx cChromaIdx );
  Void setAllCoeffCount( UChar ucCoeffCountValue = 0 );
  Void copyFrom( MbTransformCoeffs& rcMbTransformCoeffs );
  Void copyFrom( MbTransformCoeffs& rcMbTransformCoeffs, ChromaIdx cChromaIdx );
  Void reset() {}

  ErrVal  load( FILE* pFile );
  ErrVal  save( FILE* pFile );

  Void  clearLumaLevels         ();
  Void  clearLumaLevels8x8      ( B8x8Idx c8x8Idx );
  Void  clearLumaLevels8x8Block ( B8x8Idx c8x8Idx );

  Void  clearNewLumaLevels         (                        MbTransformCoeffs& rcBaseMbTCoeffs );
  Void  clearNewLumaLevels8x8      ( B8x8Idx   c8x8Idx    , MbTransformCoeffs& rcBaseMbTCoeffs );
  Void  clearNewLumaLevels8x8Block ( B8x8Idx   c8x8Idx    , MbTransformCoeffs& rcBaseMbTCoeffs );
  Void  clearNewAcBlk              ( ChromaIdx cChromaIdx , MbTransformCoeffs& rcBaseMbTCoeffs );

protected:
  TCoeff m_aaiLevel[24][16];
  UChar  m_aaucCoeffCount[24];
};



H264AVC_NAMESPACE_END


#endif // !defined(AFX_MBTRANSFORMCOEFFS_H__B5704512_C23C_497A_A794_36691A0D01BE__INCLUDED_)
