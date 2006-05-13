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




#include "H264AVCCommonLib.h"

#include "H264AVCCommonLib/SliceHeader.h"
#include "H264AVCCommonLib/PictureParameterSet.h"
#include "H264AVCCommonLib/IntFrame.h"



H264AVC_NAMESPACE_BEGIN



SliceHeader::SliceHeader( const SequenceParameterSet& rcSPS,
                          const PictureParameterSet&  rcPPS )
: SliceHeaderBase   ( rcSPS, rcPPS ),
  m_uiLastMbInSlice ( 0 ), //--ICU/ETRI FMO Implementation
  m_pcFrameUnit     ( 0 )
{
  m_auiNumRefIdxActive[LIST_0] = m_rcPPS.getNumRefIdxActive( LIST_0 );
  m_auiNumRefIdxActive[LIST_1] = m_rcPPS.getNumRefIdxActive( LIST_1 );

  ANOK( xInitScalingMatrix() );
}




ErrVal
SliceHeader::xInitScalingMatrix()
{
  if( ! m_rcSPS.getSeqScalingMatrixPresentFlag() && ! m_rcPPS.getPicScalingMatrixPresentFlag() )
  {
    m_acScalingMatrix.setAll( NULL );
    return Err::m_nOK;
  }

  for( UInt n = 0; n < m_acScalingMatrix.size(); n++ )
  {
    const UChar* puc = m_rcPPS.getPicScalingMatrix().get(n);
    if( puc == NULL )
    {
      puc = m_rcSPS.getSeqScalingMatrix().get(n);
    }
    if( puc == NULL &&  ( 1 == n || 2 == n || 4 == n || 5 == n ) )
    {
      puc = m_acScalingMatrix.get( n - 1);
    }
    if( puc == NULL )
    {
      switch(n)
      {
      case 0 : puc = g_aucScalingMatrixDefault4x4Intra; break;
      case 3 : puc = g_aucScalingMatrixDefault4x4Inter; break;
      case 6 : puc = g_aucScalingMatrixDefault8x8Intra; break;
      case 7 : puc = g_aucScalingMatrixDefault8x8Inter; break;
      default:
        AF()
      }
    }
    m_acScalingMatrix.set( n, puc );
  }

  return Err::m_nOK;
}




SliceHeader::~SliceHeader()
{
}



ErrVal
SliceHeader::compare( const SliceHeader* pcSH,
                      Bool&              rbNewFrame ) const
{
  rbNewFrame = true;

  if( isIdrNalUnit() )
  {
    ROTRS( NULL == pcSH,                                    Err::m_nOK ); //very first frame
    ROTRS( ! pcSH->isIdrNalUnit(),                          Err::m_nOK ); //previous no idr
    ROTRS( getIdrPicId() != pcSH->getIdrPicId(),            Err::m_nOK );
  }

  ROTRS( NULL == pcSH,                                      Err::m_nOK );
  ROTRS( getFrameNum() != pcSH->getFrameNum(),              Err::m_nOK );
  ROTRS( getNalRefIdc() == 0 && pcSH->getNalRefIdc() != 0,  Err::m_nOK );
  ROTRS( getNalRefIdc() != 0 && pcSH->getNalRefIdc() == 0,  Err::m_nOK );
  ROTRS( getPicOrderCntLsb() != pcSH->getPicOrderCntLsb(),  Err::m_nOK );

  rbNewFrame = false;

  return Err::m_nOK;
}




Int
SliceHeader::getDistScaleFactor( SChar sL0RefIdx,
                                 SChar sL1RefIdx ) const
{
  const Frame*  pcFrameL0 = getRefPic( sL0RefIdx, LIST_0 ).getFrame();
  const Frame*  pcFrameL1 = getRefPic( sL1RefIdx, LIST_1 ).getFrame();
  Int           iDiffPocD = pcFrameL1->getPOC() - pcFrameL0->getPOC();
  if( iDiffPocD == 0 )
  {
    return 1024;
  }
  else
  {
    Int iDiffPocB = getPoc() - pcFrameL0->getPOC();
    Int iTDB      = gClipMinMax( iDiffPocB, -128, 127 );
    Int iTDD      = gClipMinMax( iDiffPocD, -128, 127 );
    Int iX        = (0x4000 + abs(iTDD/2)) / iTDD;
    Int iScale    = gClipMinMax( (iTDB * iX + 32) >> 6, -1024, 1023 );
    return iScale;
  }
}

//TMM_EC {{
Int
SliceHeader::getDistScaleFactorVirtual( SChar sL0RefIdx,
                                 SChar sL1RefIdx,
																 RefFrameList& rcRefFrameListL0, 
																 RefFrameList& rcRefFrameListL1 ) const
{
  const IntFrame*  pcFrameL0 = rcRefFrameListL0[sL0RefIdx];

  const IntFrame*  pcFrameL1 = rcRefFrameListL1[sL1RefIdx];
  Int           iDiffPocD = pcFrameL1->getPOC() - pcFrameL0->getPOC();
  if( iDiffPocD == 0 )
  {
    return 1024;
  }
  else
  {
    Int iDiffPocB = getPoc() - pcFrameL0->getPOC();
    Int iTDB      = gClipMinMax( iDiffPocB, -128, 127 );
    Int iTDD      = gClipMinMax( iDiffPocD, -128, 127 );
    Int iX        = (0x4000 + (iTDD>>1)) / iTDD;
    Int iScale    = gClipMinMax( (iTDB * iX + 32) >> 6, -1024, 1023 );
    return iScale;
  }
}
//TMM_EC }}

Int
SliceHeader::getDistScaleFactorScal( SChar sL0RefIdx,
                                     SChar sL1RefIdx ) const
{
  IntFrame* pcFrameL0 = getRefFrameList( LIST_0 )->getEntry( sL0RefIdx-1 );
  IntFrame* pcFrameL1 = getRefFrameList( LIST_1 )->getEntry( sL1RefIdx-1 );
  Int           iDiffPocD = pcFrameL1->getPOC() - pcFrameL0->getPOC();
  if( iDiffPocD == 0 )
  {
    return 1024;
  }
  else
  {
    Int iDiffPocB = getPoc() - pcFrameL0->getPOC();
    Int iTDB      = gClipMinMax( iDiffPocB, -128, 127 );
    Int iTDD      = gClipMinMax( iDiffPocD, -128, 127 );
    Int iX        = (0x4000 + abs(iTDD/2)) / iTDD;
    Int iScale    = gClipMinMax( (iTDB * iX + 32) >> 6, -1024, 1023 );
    return iScale;
  }
}


Int
SliceHeader::getDistScaleFactorWP( const Frame* pcFrameL0, const Frame* pcFrameL1 ) const
{
  Int iDiffPocD = pcFrameL1->getPOC() - pcFrameL0->getPOC();
  if( iDiffPocD == 0 )
  {
    return 1024;
  }
  Int iDiffPocB = getPoc() - pcFrameL0->getPOC();
  Int iTDB      = gClipMinMax( iDiffPocB, -128, 127 );
  Int iTDD      = gClipMinMax( iDiffPocD, -128, 127 );
  Int iX        = (0x4000 + abs(iTDD/2)) / iTDD;
  Int iScale    = gClipMinMax( (iTDB * iX + 32) >> 6, -1024, 1023 );
  return iScale;
}


Int
SliceHeader::getDistScaleFactorWP( const IntFrame* pcFrameL0, const IntFrame* pcFrameL1 ) const
{
  Int iDiffPocD = pcFrameL1->getPOC() - pcFrameL0->getPOC();
  if( iDiffPocD == 0 )
  {
    return 1024;
  }
  Int iDiffPocB = getPoc() - pcFrameL0->getPOC();
  Int iTDB      = gClipMinMax( iDiffPocB, -128, 127 );
  Int iTDD      = gClipMinMax( iDiffPocD, -128, 127 );
  Int iX        = (0x4000 + abs(iTDD/2)) / iTDD;
  Int iScale    = gClipMinMax( (iTDB * iX + 32) >> 6, -1024, 1023 );
  return iScale;
}


// JVT-Q054 Red. Picture {
ErrVal
SliceHeader::compareRedPic( const SliceHeader* pcSH,
                           Bool&              rbNewFrame ) const
{
  rbNewFrame = true;

  ROTRS( NULL == pcSH,                                          Err::m_nOK );
  ROTRS( getIdrPicId() != pcSH->getIdrPicId(),                  Err::m_nOK );
  ROTRS( getFrameNum() != pcSH->getFrameNum(),                  Err::m_nOK );
  ROTRS( getLayerId() != pcSH->getLayerId(),                    Err::m_nOK );
  ROTRS( getQualityLevel() != pcSH->getQualityLevel(),          Err::m_nOK );
  ROTRS( getFirstMbInSlice() != pcSH->getFirstMbInSlice(),      Err::m_nOK );
  ROTRS( (getNalRefIdc() == 0 )&&(pcSH->getNalRefIdc() != 0),   Err::m_nOK );
  ROTRS( (getNalRefIdc() != 0 )&&(pcSH->getNalRefIdc() == 0),   Err::m_nOK );
  ROTRS( getPicOrderCntLsb() != pcSH->getPicOrderCntLsb(),      Err::m_nOK );

  rbNewFrame = false;

  return Err::m_nOK;
}


ErrVal
SliceHeader::sliceHeaderBackup ( SliceHeader*                pcSH )
{
  pcSH->setIdrPicId       ( getIdrPicId()       );
  pcSH->setFrameNum       ( getFrameNum()       );
  pcSH->setLayerId        ( getLayerId()        );
  pcSH->setQualityLevel   ( getQualityLevel()   );
  pcSH->setFirstMbInSlice ( getFirstMbInSlice() );
  pcSH->setNalRefIdc      ( getNalRefIdc()      );
  pcSH->setPicOrderCntLsb ( getPicOrderCntLsb() );

  return Err::m_nOK;
}



// JVT-Q054 Red. Picture }

H264AVC_NAMESPACE_END
