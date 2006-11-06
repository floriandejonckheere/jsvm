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
#include "H264AVCCommonLib/Tables.h"
#include "H264AVCCommonLib/MbDataCtrl.h"
#include "H264AVCCommonLib/ControlMngIf.h"
#include "H264AVCCommonLib/IntYuvPicBuffer.h"
#include "H264AVCCommonLib/IntFrame.h"

#include "H264AVCCommonLib/FrameUnit.h"
#include "H264AVCCommonLib/LoopFilter.h"
#include "H264AVCCommonLib/ReconstructionBypass.h"
#include "H264AVCCommonLib/IntYuvMbBuffer.h"

#include "H264AVCCommonLib/CFMO.h"


H264AVC_NAMESPACE_BEGIN



const UChar LoopFilter::g_aucBetaTab[52]  =
{
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  2,  2,  2,  3,
    3,  3,  3,  4,  4,  4,  6,  6,  7,  7,
    8,  8,  9,  9, 10, 10, 11, 11, 12, 12,
   13, 13, 14, 14, 15, 15, 16, 16, 17, 17,
   18, 18
};



const LoopFilter::AlphaClip LoopFilter::g_acAlphaClip[52] =
{
  {  0, { 0, 0, 0, 0, 0} },
  {  0, { 0, 0, 0, 0, 0} },
  {  0, { 0, 0, 0, 0, 0} },
  {  0, { 0, 0, 0, 0, 0} },
  {  0, { 0, 0, 0, 0, 0} },
  {  0, { 0, 0, 0, 0, 0} },
  {  0, { 0, 0, 0, 0, 0} },
  {  0, { 0, 0, 0, 0, 0} },
  {  0, { 0, 0, 0, 0, 0} },
  {  0, { 0, 0, 0, 0, 0} },

  {  0, { 0, 0, 0, 0, 0} },
  {  0, { 0, 0, 0, 0, 0} },
  {  0, { 0, 0, 0, 0, 0} },
  {  0, { 0, 0, 0, 0, 0} },
  {  0, { 0, 0, 0, 0, 0} },
  {  0, { 0, 0, 0, 0, 0} },
  {  4, { 0, 0, 0, 0, 0} },
  {  4, { 0, 0, 0, 1, 1} },
  {  5, { 0, 0, 0, 1, 1} },
  {  6, { 0, 0, 0, 1, 1} },

  {  7, { 0, 0, 0, 1, 1} },
  {  8, { 0, 0, 1, 1, 1} },
  {  9, { 0, 0, 1, 1, 1} },
  { 10, { 0, 1, 1, 1, 1} },
  { 12, { 0, 1, 1, 1, 1} },
  { 13, { 0, 1, 1, 1, 1} },
  { 15, { 0, 1, 1, 1, 1} },
  { 17, { 0, 1, 1, 2, 2} },
  { 20, { 0, 1, 1, 2, 2} },
  { 22, { 0, 1, 1, 2, 2} },

  { 25, { 0, 1, 1, 2, 2} },
  { 28, { 0, 1, 2, 3, 3} },
  { 32, { 0, 1, 2, 3, 3} },
  { 36, { 0, 2, 2, 3, 3} },
  { 40, { 0, 2, 2, 4, 4} },
  { 45, { 0, 2, 3, 4, 4} },
  { 50, { 0, 2, 3, 4, 4} },
  { 56, { 0, 3, 3, 5, 5} },
  { 63, { 0, 3, 4, 6, 6} },
  { 71, { 0, 3, 4, 6, 6} },

  { 80, { 0, 4, 5, 7, 7} },
  { 90, { 0, 4, 5, 8, 8} },
 { 101, { 0, 4, 6, 9, 9} },
 { 113, { 0, 5, 7,10,10} },
 { 127, { 0, 6, 8,11,11} },
 { 144, { 0, 6, 8,13,13} },
 { 162, { 0, 7,10,14,14} },
 { 182, { 0, 8,11,16,16} },
 { 203, { 0, 9,12,18,18} },
 { 226, { 0,10,13,20,20} },

 { 255, { 0,11,15,23,23} },
 { 255, { 0,13,17,25,25} }
} ;




LoopFilter::LoopFilter() :
  m_pcControlMngIf( NULL ),
  m_pcIntYuvBuffer(NULL),
  m_pcRecFrameUnit( NULL )
{
  m_eLFMode  = LFM_DEFAULT_FILTER;
}



LoopFilter::~LoopFilter()
{
}


ErrVal LoopFilter::create( LoopFilter*& rpcLoopFilter )
{
  rpcLoopFilter = new LoopFilter;

  ROT( NULL == rpcLoopFilter );

  return Err::m_nOK;
}


ErrVal LoopFilter::destroy()
{
  delete this;

  return Err::m_nOK;
}


ErrVal LoopFilter::init(  ControlMngIf* pcControlMngIf
                         ,ReconstructionBypass*       pcReconstructionBypass
                        )
{
  ROT( NULL == pcControlMngIf );
  ROT( NULL == pcReconstructionBypass );
  m_pcReconstructionBypass = pcReconstructionBypass;

  m_pcControlMngIf  = pcControlMngIf;
  m_pcHighpassFrame = NULL; // Hanke@RWTH

  return Err::m_nOK;
}



ErrVal LoopFilter::uninit()
{
  m_pcControlMngIf = NULL;
  return Err::m_nOK;
}

ErrVal LoopFilter::process( SliceHeader& rcSH, IntYuvPicBuffer* pcIntYuvPicBuffer, IntYuvPicBuffer* pcHighpassYuvBuffer
							, bool  bAllSliceDone)
{
  // Hanke@RWTH // switch off filter modifications
  setHighpassFramePointer();
  m_pcHighpassYuvBuffer = pcHighpassYuvBuffer;

  m_pcIntYuvBuffer = pcIntYuvPicBuffer;
  ROT( NULL == m_pcControlMngIf );

  m_pcRecFrameUnit = const_cast<SliceHeader&>(rcSH).getFrameUnit();

  RNOK( m_pcControlMngIf->initSlice( rcSH, POST_PROCESS ) );  

  //-> ICU/ETRI DS FMO Process
  UInt uiFirstMbInSlice;
  UInt uiLastMbInSlice;

  FMO* pcFMO = rcSH.getFMO();  

  for(Int iSliceGroupID=0;!pcFMO->SliceGroupCompletelyCoded(iSliceGroupID);iSliceGroupID++)   
  {
	if (false == pcFMO->isCodedSG(iSliceGroupID) && false == bAllSliceDone)
	{
	  continue;
	}

	uiFirstMbInSlice = pcFMO->getFirstMacroblockInSlice(iSliceGroupID);
	uiLastMbInSlice = pcFMO->getLastMBInSliceGroup(iSliceGroupID);

    //===== loop over macroblocks use raster scan =====  
    for(UInt uiMbAddress= uiFirstMbInSlice ;uiMbAddress<=uiLastMbInSlice ;)    
	{
      MbDataAccess* pcMbDataAccess;
      RNOK( m_pcControlMngIf->initMbForFiltering( pcMbDataAccess, uiMbAddress ) );

	  RNOK( xFilterMb( *pcMbDataAccess ) );	

	  uiMbAddress = rcSH.getFMO()->getNextMBNr(uiMbAddress );    
	}  
  }
  //<- ICU/ETRI DS

  m_pcHighpassYuvBuffer = NULL;
  m_pcIntYuvBuffer = NULL;
  return Err::m_nOK;
}


__inline ErrVal LoopFilter::xFilterMb( const MbDataAccess& rcMbDataAccess )
{
  const DFP& rcDFP      = rcMbDataAccess.getDeblockingFilterParameter( m_eLFMode & LFM_NO_INTER_FILTER );
  Int iFilterIdc  = rcDFP.getDisableDeblockingFilterIdc();

  ROTRS( (m_eLFMode & LFM_NO_INTER_FILTER) && ! rcMbDataAccess.getMbData().isIntra(), Err::m_nOK );

  ROTRS( iFilterIdc == 1, Err::m_nOK );

  Bool b8x8 = rcMbDataAccess.getMbData().isTransformSize8x8();

  for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
  {
    if( !b8x8 || ( ( cIdx.x() & 1 ) == 0 ) )
    {
      m_aaaucBs[VER][cIdx.x()][cIdx.y()]  = xGetVerFilterStrength( rcMbDataAccess, cIdx, iFilterIdc );
    }
    else
    {
      m_aaaucBs[VER][cIdx.x()][cIdx.y()]  = 0;
    }
    if( !b8x8 || ( ( cIdx.y() & 1 ) == 0 ) )
    {
      m_aaaucBs[HOR][cIdx.x()][cIdx.y()]  = xGetHorFilterStrength( rcMbDataAccess, cIdx, iFilterIdc );
    }
    else
    {
      m_aaaucBs[HOR][cIdx.x()][cIdx.y()]  = 0;
    }
  }

  if( m_pcIntYuvBuffer )
  {
    RNOK( xLumaVerFiltering(   rcMbDataAccess, rcDFP, m_pcIntYuvBuffer ) );
    RNOK( xLumaHorFiltering(   rcMbDataAccess, rcDFP, m_pcIntYuvBuffer ) );
    RNOK( xChromaVerFiltering( rcMbDataAccess, rcDFP, m_pcIntYuvBuffer ) );
    RNOK( xChromaHorFiltering( rcMbDataAccess, rcDFP, m_pcIntYuvBuffer ) );
    return Err::m_nOK;
  }
  YuvPicBuffer* pcYuvBuffer = m_pcRecFrameUnit->getFrame().getFullPelYuvBuffer();

  RNOK( xLumaVerFiltering(   rcMbDataAccess, rcDFP, pcYuvBuffer ) );
  RNOK( xLumaHorFiltering(   rcMbDataAccess, rcDFP, pcYuvBuffer ) );
  RNOK( xChromaVerFiltering( rcMbDataAccess, rcDFP, pcYuvBuffer ) );
  RNOK( xChromaHorFiltering( rcMbDataAccess, rcDFP, pcYuvBuffer ) );

  return Err::m_nOK;
}



__inline Void LoopFilter::xFilter( Pel* pFlt, const Int& iOffset, const Int& iIndexA, const Int& iIndexB, const UChar& ucBs, const Bool& bLum )
{
  const Int iAlpha = g_acAlphaClip[ iIndexA ].ucAlpha;

  Int P0 = pFlt[-iOffset];
  Int Q0 = pFlt[       0];

  Int iDelta = Q0 - P0;
  Int iAbsDelta  = abs( iDelta  );

  AOF_DBG( ucBs );

  ROFVS( iAbsDelta < iAlpha );


  const Int iBeta = g_aucBetaTab [ iIndexB ];

  Int P1  = pFlt[-2*iOffset];
  Int Q1  = pFlt[   iOffset];

  ROFVS( (abs(P0 - P1) < iBeta) && (abs(Q0 - Q1) < iBeta) );

  if( ucBs < 4 )
  {
    Int C0 = g_acAlphaClip[ iIndexA ].aucClip[ucBs];

    if( bLum )
    {
      Int P2 = pFlt[-3*iOffset] ;
      Int Q2 = pFlt[ 2*iOffset] ;
      Int aq = (( abs( Q2 - Q0 ) < iBeta ) ? 1 : 0 );
      Int ap = (( abs( P2 - P0 ) < iBeta ) ? 1 : 0 );

      if( ap )
      {
        pFlt[-2*iOffset] = P1 + gClipMinMax((P2 + ((P0 + Q0 + 1)>>1) - (P1<<1)) >> 1, -C0, C0 );
      }

      if( aq  )
      {
        pFlt[   iOffset] = Q1 + gClipMinMax((Q2 + ((P0 + Q0 + 1)>>1) - (Q1<<1)) >> 1, -C0, C0 );
      }

      C0 += ap + aq -1;
    }

    C0++;
    Int iDiff      = gClipMinMax(((iDelta << 2) + (P1 - Q1) + 4) >> 3, -C0, C0 ) ;
    pFlt[-iOffset] = gClip( P0 + iDiff );
    pFlt[       0] = gClip( Q0 - iDiff );
    return;
  }


  if( ! bLum )
  {
    pFlt[         0] = ((Q1 << 1) + Q0 + P1 + 2) >> 2;
    pFlt[  -iOffset] = ((P1 << 1) + P0 + Q1 + 2) >> 2;
  }
  else
  {
    Int P2 = pFlt[-3*iOffset] ;
    Int Q2 = pFlt[ 2*iOffset] ;
    Bool bEnable  = (iAbsDelta < ((iAlpha >> 2) + 2));
    Bool aq       = bEnable & ( abs( Q2 - Q0 ) < iBeta );
    Bool ap       = bEnable & ( abs( P2 - P0 ) < iBeta );
    Int PQ0 = P0 + Q0;

    if( aq )
    {
      pFlt[         0] = (P1 + ((Q1 + PQ0) << 1) +  Q2 + 4) >> 3;
      pFlt[   iOffset] = (PQ0 +Q1 + Q2 + 2) >> 2;
      pFlt[ 2*iOffset] = (((pFlt[ 3*iOffset] + Q2) <<1) + Q2 + Q1 + PQ0 + 4) >> 3;
    }
    else
    {
      pFlt[         0] = ((Q1 << 1) + Q0 + P1 + 2) >> 2;
    }

    if( ap )
    {
      pFlt[  -iOffset] = (Q1 + ((P1 + PQ0) << 1) +  P2 + 4) >> 3;
      pFlt[-2*iOffset] = (PQ0 +P1 + P2 + 2) >> 2;
      pFlt[-3*iOffset] = (((pFlt[-4*iOffset] + P2) <<1) + pFlt[-3*iOffset] + P1 + PQ0 + 4) >> 3;
    }
    else
    {
      pFlt[  -iOffset] = ((P1 << 1) + P0 + Q1 + 2) >> 2;
    }
  }
}




__inline ErrVal LoopFilter::xLumaVerFiltering( const MbDataAccess& rcMbDataAccess, const DFP& rcDFP, YuvPicBuffer* pcYuvBuffer )
{
  Int   iCurrQp = rcMbDataAccess.getMbDataCurr().getQpLF();
  Int   iStride = pcYuvBuffer->getLStride();
  Pel*  pPelLum = pcYuvBuffer->getMbLumAddr();

  //===== filtering of left macroblock edge =====
  {
    Int iLeftQp = rcMbDataAccess.getMbDataLeft().getQpLF();
    Int iQp     = ( iLeftQp + iCurrQp + 1) >> 1;
    Int iIndexA = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
    Int iIndexB = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);

    for( Int yBlk = 0; yBlk < 4; yBlk++)
    {
      const UChar ucBs = m_aaaucBs[VER][0][yBlk];
      if( 0 != ucBs )
      {
        xFilter( pPelLum,           1, iIndexA, iIndexB, ucBs, true );
        xFilter( pPelLum+  iStride, 1, iIndexA, iIndexB, ucBs, true );
        xFilter( pPelLum+2*iStride, 1, iIndexA, iIndexB, ucBs, true );
        xFilter( pPelLum+3*iStride, 1, iIndexA, iIndexB, ucBs, true );
      }
      pPelLum += 4*iStride;
    }
    pPelLum -= 16*iStride-4;
  }


  Int iIndexA = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iCurrQp, 0, 51);
  Int iIndexB = gClipMinMax( rcDFP.getSliceBetaOffset()    + iCurrQp, 0, 51);

  for( Int xBlk = 1; xBlk < 4; xBlk++)
  {
    for( Int yBlk = 0; yBlk < 4; yBlk++)
    {
      const UChar ucBs = m_aaaucBs[VER][xBlk][yBlk];
      if( 0 != ucBs )
      {
        xFilter( pPelLum,           1, iIndexA, iIndexB, ucBs, true );
        xFilter( pPelLum+  iStride, 1, iIndexA, iIndexB, ucBs, true );
        xFilter( pPelLum+2*iStride, 1, iIndexA, iIndexB, ucBs, true );
        xFilter( pPelLum+3*iStride, 1, iIndexA, iIndexB, ucBs, true );
      }
      pPelLum += 4*iStride;
    }
    pPelLum -= 16*iStride-4;
  }

  return Err::m_nOK;
}




__inline ErrVal LoopFilter::xLumaHorFiltering( const MbDataAccess& rcMbDataAccess, const DFP& rcDFP, YuvPicBuffer* pcYuvBuffer )
{
  Int   iCurrQp = rcMbDataAccess.getMbDataCurr().getQpLF();
  Int   iStride = pcYuvBuffer->getLStride();
  Pel*  pPelLum = pcYuvBuffer->getMbLumAddr();

  //===== filtering of upper macroblock edge =====
  {
    //-----  any other combination than curr = FRM, above = FLD  -----
    Int iAboveQp  = rcMbDataAccess.getMbDataAbove().getQpLF();
    Int iQp       = ( iAboveQp + iCurrQp + 1) >> 1;
    Int iIndexA   = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
    Int iIndexB   = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);

    for( Int xBlk = 0; xBlk < 4; xBlk++)
    {
      const UChar ucBs = m_aaaucBs[HOR][xBlk][0];
      if( 0 != ucBs )
      {
        xFilter( pPelLum,   iStride, iIndexA, iIndexB, ucBs, true );
        xFilter( pPelLum+1, iStride, iIndexA, iIndexB, ucBs, true );
        xFilter( pPelLum+2, iStride, iIndexA, iIndexB, ucBs, true );
        xFilter( pPelLum+3, iStride, iIndexA, iIndexB, ucBs, true );
      }
      pPelLum += 4;
    }
    pPelLum -= 16;
  }

  pPelLum += 4*iStride;
  Int iIndexA = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iCurrQp, 0, 51);
  Int iIndexB = gClipMinMax( rcDFP.getSliceBetaOffset()    + iCurrQp, 0, 51);

  for( Int yBlk = 1; yBlk < 4; yBlk++)
  {
    for( Int xBlk = 0; xBlk < 4; xBlk++)
    {
      const UChar ucBs = m_aaaucBs[HOR][xBlk][yBlk];
      if( 0 != ucBs )
      {
        xFilter( pPelLum,   iStride, iIndexA, iIndexB, ucBs, true );
        xFilter( pPelLum+1, iStride, iIndexA, iIndexB, ucBs, true );
        xFilter( pPelLum+2, iStride, iIndexA, iIndexB, ucBs, true );
        xFilter( pPelLum+3, iStride, iIndexA, iIndexB, ucBs, true );
      }
      pPelLum += 4;
    }
    pPelLum += 4*iStride - 16;
  }

  return Err::m_nOK;
}




__inline ErrVal LoopFilter::xChromaHorFiltering( const MbDataAccess& rcMbDataAccess, const DFP& rcDFP, YuvPicBuffer* pcYuvBuffer )
{
  Int   iCurrQp = rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.getMbDataCurr().getQpLF() );
  Int   iStride = pcYuvBuffer->getCStride();
  Pel*  pPelCb  = pcYuvBuffer->getMbCbAddr();
  Pel*  pPelCr  = pcYuvBuffer->getMbCrAddr();

  //===== filtering of upper macroblock edge =====
  {
    //-----  any other combination than curr = FRM, above = FLD  -----
    Int iAboveQp  = rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.getMbDataAbove().getQpLF() );
    Int iQp       = ( iAboveQp + iCurrQp + 1) >> 1;
    Int iIndexA   = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
    Int iIndexB   = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);

    for( Int xBlk = 0; xBlk < 4; xBlk++)
    {
      const UChar ucBs = m_aaaucBs[HOR][xBlk][0];
      if( 0 != ucBs )
      {
        xFilter( pPelCb,   iStride, iIndexA, iIndexB, ucBs, false );
        xFilter( pPelCb+1, iStride, iIndexA, iIndexB, ucBs, false );
        xFilter( pPelCr,   iStride, iIndexA, iIndexB, ucBs, false );
        xFilter( pPelCr+1, iStride, iIndexA, iIndexB, ucBs, false );
      }
      pPelCb += 2;
      pPelCr += 2;
    }
    pPelCb -= 8;
    pPelCr -= 8;
  }

  pPelCb += 4*iStride;
  pPelCr += 4*iStride;

  // now we filter the remaining edge
  Int iIndexA = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iCurrQp, 0, 51);
  Int iIndexB = gClipMinMax( rcDFP.getSliceBetaOffset()    + iCurrQp, 0, 51);

  for( Int xBlk = 0; xBlk < 4; xBlk++)
  {
    const UChar ucBs = m_aaaucBs[HOR][xBlk][2];
    if( 0 != ucBs )
    {
      xFilter( pPelCb,   iStride, iIndexA, iIndexB, ucBs, false );
      xFilter( pPelCb+1, iStride, iIndexA, iIndexB, ucBs, false );
      xFilter( pPelCr,   iStride, iIndexA, iIndexB, ucBs, false );
      xFilter( pPelCr+1, iStride, iIndexA, iIndexB, ucBs, false );
    }
    pPelCb += 2;
    pPelCr += 2;
  }

  return Err::m_nOK;
}



__inline ErrVal LoopFilter::xChromaVerFiltering( const MbDataAccess& rcMbDataAccess, const DFP& rcDFP, YuvPicBuffer* pcYuvBuffer )
{
  Int   iCurrQp = rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.getMbDataCurr().getQpLF() );
  Int   iStride = pcYuvBuffer->getCStride();
  Pel*  pPelCb  = pcYuvBuffer->getMbCbAddr();
  Pel*  pPelCr  = pcYuvBuffer->getMbCrAddr();

  //===== filtering of left macroblock edge =====
  {
    //-----  curr == FRM && left == FRM  or  curr == FLD && left == FLD  -----
    Int iLeftQp = rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.getMbDataLeft().getQpLF() );
    Int iQp     = ( iLeftQp + iCurrQp + 1) >> 1;
    Int iIndexA = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
    Int iIndexB = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);

    for( Int yBlk = 0; yBlk < 4; yBlk++)
    {
      const UChar ucBs = m_aaaucBs[VER][0][yBlk];
      if( 0 != ucBs )
      {
        xFilter( pPelCb,         1, iIndexA, iIndexB, ucBs, false );
        xFilter( pPelCb+iStride, 1, iIndexA, iIndexB, ucBs, false );
        xFilter( pPelCr,         1, iIndexA, iIndexB, ucBs, false );
        xFilter( pPelCr+iStride, 1, iIndexA, iIndexB, ucBs, false );
      }
      pPelCb += 2*iStride;
      pPelCr += 2*iStride;
    }
  }

  pPelCb -= 8*iStride-4;
  pPelCr -= 8*iStride-4;

  Int iIndexA = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iCurrQp, 0, 51);
  Int iIndexB = gClipMinMax( rcDFP.getSliceBetaOffset()    + iCurrQp, 0, 51);
  for( Int yBlk = 0; yBlk < 4; yBlk++)
  {
    const UChar ucBs = m_aaaucBs[VER][2][yBlk];
    if( 0 != ucBs )
    {
      xFilter( pPelCb,         1, iIndexA, iIndexB, ucBs, false );
      xFilter( pPelCb+iStride, 1, iIndexA, iIndexB, ucBs, false );
      xFilter( pPelCr,         1, iIndexA, iIndexB, ucBs, false );
      xFilter( pPelCr+iStride, 1, iIndexA, iIndexB, ucBs, false );
    }
    pPelCb += 2*iStride;
    pPelCr += 2*iStride;
  }

  return Err::m_nOK;
}


UChar LoopFilter::xCheckMvDataB( const MbData& rcQMbData, const LumaIdx cQIdx,
                                 const MbData& rcPMbData, const LumaIdx cPIdx,
                                 const Short   sHorMvThr, const Short   sVerMvThr )
{
  const MbMotionData& rcMbMotionDataL0Q = rcQMbData.getMbMotionData( LIST_0 );
  const MbMotionData& rcMbMotionDataL1Q = rcQMbData.getMbMotionData( LIST_1 );
  const MbMotionData& rcMbMotionDataL0P = rcPMbData.getMbMotionData( LIST_0 );
  const MbMotionData& rcMbMotionDataL1P = rcPMbData.getMbMotionData( LIST_1 );

  const RefPic& rcRefPicL0Q = rcMbMotionDataL0Q.getRefPic( cQIdx );
  const RefPic& rcRefPicL1Q = rcMbMotionDataL1Q.getRefPic( cQIdx );
  const RefPic& rcRefPicL0P = rcMbMotionDataL0P.getRefPic( cPIdx );
  const RefPic& rcRefPicL1P = rcMbMotionDataL1P.getRefPic( cPIdx );

  UInt uiNumberOfUsedPic;
  {
    // check the number of used ref frames
    UInt uiQNumberOfUsedPic = (rcRefPicL0Q.isAssigned() ? 1:0) + (rcRefPicL1Q.isAssigned() ? 1:0);
    UInt uiPNumberOfUsedPic = (rcRefPicL0P.isAssigned() ? 1:0) + (rcRefPicL1P.isAssigned() ? 1:0);
    ROTRS( uiPNumberOfUsedPic != uiQNumberOfUsedPic, 1 );
    uiNumberOfUsedPic = uiPNumberOfUsedPic;
  }


  if( 1 == uiNumberOfUsedPic )
  {
    // this is the easy part
    // check whether they ref diff ref pic or not
    const RefPic& rcRefPicQ = ( rcRefPicL0Q.isAssigned() ? rcRefPicL0Q : rcRefPicL1Q );
    const RefPic& rcRefPicP = ( rcRefPicL0P.isAssigned() ? rcRefPicL0P : rcRefPicL1P );

    ROTRS( rcRefPicQ.getFrame() != rcRefPicP.getFrame(), 1 );

    // check the motion vector distance
    const Mv& cMvQ = (rcRefPicL0Q.isAssigned() ? rcMbMotionDataL0Q.getMv( cQIdx ) : rcMbMotionDataL1Q.getMv( cQIdx ));
    const Mv& cMvP = (rcRefPicL0P.isAssigned() ? rcMbMotionDataL0P.getMv( cPIdx ) : rcMbMotionDataL1P.getMv( cPIdx ));

    ROTRS( cMvP.getAbsHorDiff( cMvQ ) >= sHorMvThr, 1 );
    ROTRS( cMvP.getAbsVerDiff( cMvQ ) >= sVerMvThr, 1 );
    return 0;
  }


  // both ref pic are used for both blocks
  if( rcRefPicL1P.getFrame() != rcRefPicL0P.getFrame() )
  {
    // at least two diff ref pic are in use
    if( rcRefPicL1P.getFrame() != rcRefPicL1Q.getFrame() )
    {
      ROTRS( rcRefPicL1P.getFrame() != rcRefPicL0Q.getFrame(), 1 );
      ROTRS( rcRefPicL0P.getFrame() != rcRefPicL1Q.getFrame(), 1 );

      // rcRefPicL0P == rcRefPicL1Q && rcRefPicL1P == rcRefPicL0Q
      // check the motion vector distance
      const Mv& cMvQ0 = rcMbMotionDataL0Q.getMv( cQIdx );
      const Mv& cMvP0 = rcMbMotionDataL0P.getMv( cPIdx );
      const Mv& cMvQ1 = rcMbMotionDataL1Q.getMv( cQIdx );
      const Mv& cMvP1 = rcMbMotionDataL1P.getMv( cPIdx );

      ROTRS( cMvP0.getAbsHorDiff( cMvQ1 ) >= sHorMvThr, 1 );
      ROTRS( cMvP0.getAbsVerDiff( cMvQ1 ) >= sVerMvThr, 1 );
      ROTRS( cMvP1.getAbsHorDiff( cMvQ0 ) >= sHorMvThr, 1 );
      ROTRS( cMvP1.getAbsVerDiff( cMvQ0 ) >= sVerMvThr, 1 );

      return 0;
    }

    // rcRefPicL1P == rcRefPicL1Q
    ROTRS( rcRefPicL0P.getFrame() != rcRefPicL0Q.getFrame(), 1 );

    // rcRefPicL0P == rcRefPicL0Q && rcRefPicL1P == rcRefPicL1Q
    // check the motion vector distance
    const Mv& cMvQ0 = rcMbMotionDataL0Q.getMv( cQIdx );
    const Mv& cMvP0 = rcMbMotionDataL0P.getMv( cPIdx );
    const Mv& cMvQ1 = rcMbMotionDataL1Q.getMv( cQIdx );
    const Mv& cMvP1 = rcMbMotionDataL1P.getMv( cPIdx );

    ROTRS( cMvP0.getAbsHorDiff( cMvQ0 ) >= sHorMvThr, 1 );
    ROTRS( cMvP0.getAbsVerDiff( cMvQ0 ) >= sVerMvThr, 1 );
    ROTRS( cMvP1.getAbsHorDiff( cMvQ1 ) >= sHorMvThr, 1 );
    ROTRS( cMvP1.getAbsVerDiff( cMvQ1 ) >= sVerMvThr, 1 );
    return 0;
  }

  //  rcRefPicL1P == rcRefPicL0P
  ROTRS( rcRefPicL1Q.getFrame() != rcRefPicL0Q.getFrame(), 1 ) ;
  ROTRS( rcRefPicL0P.getFrame() != rcRefPicL0Q.getFrame(), 1 ) ;

  // rcRefPicL0P == rcRefPicL0Q == rcRefPicL1P == rcRefPicL1Q
  // check the motion vector distance
  const Mv& cMvQ0 = rcMbMotionDataL0Q.getMv( cQIdx );
  const Mv& cMvP0 = rcMbMotionDataL0P.getMv( cPIdx );
  const Mv& cMvQ1 = rcMbMotionDataL1Q.getMv( cQIdx );
  const Mv& cMvP1 = rcMbMotionDataL1P.getMv( cPIdx );

  Bool              bSameListCond  = ( (cMvP0.getAbsHorDiff( cMvQ0 ) >= sHorMvThr) );
  bSameListCond = ( bSameListCond || ( (cMvP0.getAbsVerDiff( cMvQ0 ) >= sVerMvThr) ) );
  bSameListCond = ( bSameListCond || ( (cMvP1.getAbsHorDiff( cMvQ1 ) >= sHorMvThr) ) );
  bSameListCond = ( bSameListCond || ( (cMvP1.getAbsVerDiff( cMvQ1 ) >= sVerMvThr) ) );
  Bool              bDiffListCond  = ( (cMvP0.getAbsHorDiff( cMvQ1 ) >= sHorMvThr) );
  bDiffListCond = ( bDiffListCond || ( (cMvP0.getAbsVerDiff( cMvQ1 ) >= sVerMvThr) ) );
  bDiffListCond = ( bDiffListCond || ( (cMvP1.getAbsHorDiff( cMvQ0 ) >= sHorMvThr) ) );
  bDiffListCond = ( bDiffListCond || ( (cMvP1.getAbsVerDiff( cMvQ0 ) >= sVerMvThr) ) );

  ROTRS( bSameListCond && bDiffListCond, 1 );

  return 0;
}




UChar LoopFilter::xCheckMvDataP( const MbData& rcQMbData, const LumaIdx cQIdx,
                                 const MbData& rcPMbData, const LumaIdx cPIdx,
                                 const Short   sHorMvThr, const Short   sVerMvThr )
{
  const MbMotionData& rcMbMotionDataL0Q = rcQMbData.getMbMotionData( LIST_0 );
  const MbMotionData& rcMbMotionDataL0P = rcPMbData.getMbMotionData( LIST_0 );

  const RefPic& rcRefPicL0Q = rcMbMotionDataL0Q.getRefPic( cQIdx );
  const RefPic& rcRefPicL0P = rcMbMotionDataL0P.getRefPic( cPIdx );

  // different reference pictures
  ROTRS( rcRefPicL0Q.getFrame() != rcRefPicL0P.getFrame(), 1 );

  // check the motion vector distance
  const Mv& cMvQ = rcMbMotionDataL0Q.getMv( cQIdx );
  const Mv& cMvP = rcMbMotionDataL0P.getMv( cPIdx );

  ROTRS( cMvP.getAbsHorDiff( cMvQ ) >= sHorMvThr, 1 );
  ROTRS( cMvP.getAbsVerDiff( cMvQ ) >= sVerMvThr, 1 );
  return 0;
}


__inline UInt LoopFilter::xGetVerFilterStrength( const MbDataAccess&  rcMbDataAccess,
                                                 LumaIdx              cIdx,
                                                 Int                  iFilterIdc )
{
  const MbData& rcMbDataCurr  = rcMbDataAccess.getMbDataCurr();
  Short         sHorMvThr     = 4;
  Short         sVerMvThr     = 4;

  if( cIdx.x() )
  {
    // this is a edge inside of a macroblock
    ROTRS( rcMbDataCurr.isIntra(), 3 );

    if( m_pcHighpassYuvBuffer )
    {
      ROTRS( m_pcHighpassYuvBuffer->isCurr4x4BlkNotZero ( cIdx                          ), 2 );
      ROTRS( m_pcHighpassYuvBuffer->isCurr4x4BlkNotZero ( cIdx + CURR_MB_LEFT_NEIGHBOUR ), 2 );
    }
    else
    { 
      ROTRS( rcMbDataCurr.is4x4BlkCoded( cIdx                          ), 2 );
      ROTRS( rcMbDataCurr.is4x4BlkCoded( cIdx + CURR_MB_LEFT_NEIGHBOUR ), 2 );
    }

    if( rcMbDataCurr.isInterPMb() )
    {
      return xCheckMvDataP( rcMbDataCurr, cIdx, rcMbDataCurr, cIdx + CURR_MB_LEFT_NEIGHBOUR, sHorMvThr, sVerMvThr );
    }
    return xCheckMvDataB  ( rcMbDataCurr, cIdx, rcMbDataCurr, cIdx + CURR_MB_LEFT_NEIGHBOUR, sHorMvThr, sVerMvThr );
  }


  // if we get here we are on a macroblock edge
  ROTRS( iFilterIdc == 2 && ! rcMbDataAccess.isAvailableLeft(), 0 );
  ROTRS( ! rcMbDataAccess.isLeftMbExisting(),                   0 );


  const MbData& rcMbDataLeft = rcMbDataAccess.getMbDataLeft();

  ROTRS( LFM_DEFAULT_FILTER != m_eLFMode && rcMbDataCurr.isIntra() ^ rcMbDataLeft.isIntra(), 0 );

  ROTRS( rcMbDataCurr.isIntra(), 4 );
  ROTRS( rcMbDataLeft.isIntra(), 4 );

  if( m_pcHighpassYuvBuffer )
  {
    ROTRS( m_pcHighpassYuvBuffer->isCurr4x4BlkNotZero ( cIdx                          ), 2 );
    ROTRS( m_pcHighpassYuvBuffer->isLeft4x4BlkNotZero ( cIdx + LEFT_MB_LEFT_NEIGHBOUR ), 2 );
  }
  else
  {
    ROTRS( rcMbDataCurr.is4x4BlkCoded( cIdx                          ), 2 );
    ROTRS( rcMbDataLeft.is4x4BlkCoded( cIdx + LEFT_MB_LEFT_NEIGHBOUR ), 2 );
  }

  if( rcMbDataCurr.isInterPMb() && rcMbDataLeft.isInterPMb())
  {
    return xCheckMvDataP( rcMbDataCurr, cIdx, rcMbDataLeft, cIdx + LEFT_MB_LEFT_NEIGHBOUR, sHorMvThr, sVerMvThr );
  }
  return xCheckMvDataB  ( rcMbDataCurr, cIdx, rcMbDataLeft, cIdx + LEFT_MB_LEFT_NEIGHBOUR, sHorMvThr, sVerMvThr );
}



__inline UInt LoopFilter::xGetHorFilterStrength( const MbDataAccess&  rcMbDataAccess,
                                                 LumaIdx              cIdx,
                                                 Int                  iFilterIdc )
{
  const MbData& rcMbDataCurr  = rcMbDataAccess.getMbDataCurr();
  Short         sHorMvThr     = 4;
  Short         sVerMvThr     = 4;

  if( cIdx.y() )
  {
    // internal edge
    ROTRS( rcMbDataCurr.isIntra(), 3 );

    if( m_pcHighpassYuvBuffer )
    {
      ROTRS( m_pcHighpassYuvBuffer->isCurr4x4BlkNotZero ( cIdx                           ), 2 );
      ROTRS( m_pcHighpassYuvBuffer->isCurr4x4BlkNotZero ( cIdx + CURR_MB_ABOVE_NEIGHBOUR ), 2 );
    }
    else
    {
      ROTRS( rcMbDataCurr.is4x4BlkCoded( cIdx                           ), 2 );
      ROTRS( rcMbDataCurr.is4x4BlkCoded( cIdx + CURR_MB_ABOVE_NEIGHBOUR ), 2 );
    }

    if( rcMbDataCurr.isInterPMb() )
    {
      return xCheckMvDataP( rcMbDataCurr, cIdx, rcMbDataCurr, cIdx + CURR_MB_ABOVE_NEIGHBOUR, sHorMvThr, sVerMvThr );
    }
    return xCheckMvDataB  ( rcMbDataCurr, cIdx, rcMbDataCurr, cIdx + CURR_MB_ABOVE_NEIGHBOUR, sHorMvThr, sVerMvThr );
  }

  // if we get here we are on a macroblock edge
  ROTRS( iFilterIdc == 2 && ! rcMbDataAccess.isAvailableAbove(),  0 );
  ROTRS( ! rcMbDataAccess.isAboveMbExisting(),                    0 );


  const MbData& rcMbDataAbove = rcMbDataAccess.getMbDataAbove();

  ROTRS( LFM_DEFAULT_FILTER != m_eLFMode && rcMbDataCurr.isIntra() ^ rcMbDataAbove.isIntra(), 0 );
  ROTRS( rcMbDataCurr. isIntra(), 4 );
  ROTRS( rcMbDataAbove.isIntra(), 4 );

  if( m_pcHighpassYuvBuffer )
  {
    ROTRS( m_pcHighpassYuvBuffer->isCurr4x4BlkNotZero  ( cIdx                            ), 2 );
    ROTRS( m_pcHighpassYuvBuffer->isAbove4x4BlkNotZero ( cIdx + ABOVE_MB_ABOVE_NEIGHBOUR ), 2 );
  }
  else
  {
    ROTRS( rcMbDataCurr. is4x4BlkCoded( cIdx                            ), 2 );
    ROTRS( rcMbDataAbove.is4x4BlkCoded( cIdx + ABOVE_MB_ABOVE_NEIGHBOUR ), 2 );
  }

  if( rcMbDataCurr.isInterPMb() && rcMbDataAbove.isInterPMb())
  {
    return xCheckMvDataP( rcMbDataCurr, cIdx, rcMbDataAbove, cIdx + ABOVE_MB_ABOVE_NEIGHBOUR, sHorMvThr, sVerMvThr );
  }
  return xCheckMvDataB  ( rcMbDataCurr, cIdx, rcMbDataAbove, cIdx + ABOVE_MB_ABOVE_NEIGHBOUR, sHorMvThr, sVerMvThr );
}




ErrVal LoopFilter::process( SliceHeader&  rcSH,
                            IntFrame*     pcFrame,
                            MbDataCtrl*   pcMbDataCtrlMot, // if NULL -> all intra
                            MbDataCtrl*   pcMbDataCtrlRes,
                            UInt          uiMbInRow,
                            RefFrameList* pcRefFrameList0,
                            RefFrameList* pcRefFrameList1,			
							              bool		  bAllSliceDone, 
                            bool          spatial_scalable_flg)  // SSUN@SHARP
{
  ROT( NULL == m_pcControlMngIf );

  RNOK(   m_pcControlMngIf->initSliceForFiltering ( rcSH               ) );
  RNOK(   pcMbDataCtrlRes ->initSlice             ( rcSH, POST_PROCESS, false, NULL ) );
  if( pcMbDataCtrlMot )
  {
    RNOK( pcMbDataCtrlMot ->initSlice             ( rcSH, POST_PROCESS, false, NULL ) );
  }    

  //-> ICU/ETRI DS FMO Process
  UInt uiFirstMbInSlice;
  UInt uiLastMbInSlice;

  FMO* pcFMO = rcSH.getFMO();  

  for(Int iSliceGroupID=0;!pcFMO->SliceGroupCompletelyCoded(iSliceGroupID);iSliceGroupID++)   
  {
	 if (false == pcFMO->isCodedSG(iSliceGroupID) && false == bAllSliceDone)	 
	 {
		 continue;
	 }

	 uiFirstMbInSlice = pcFMO->getFirstMacroblockInSlice(iSliceGroupID);
	 uiLastMbInSlice = pcFMO->getLastMBInSliceGroup(iSliceGroupID);

    for(UInt uiMbAddress= uiFirstMbInSlice ;uiMbAddress<=uiLastMbInSlice ;)  
    //===== loop over macroblocks use raster scan =====  
	{
      UInt          uiMbY             = uiMbAddress / uiMbInRow;
      UInt          uiMbX             = uiMbAddress % uiMbInRow;
      MbDataAccess* pcMbDataAccessMot = 0;
      MbDataAccess* pcMbDataAccessRes = 0;

      if( pcMbDataCtrlMot )
	  {
        RNOK( pcMbDataCtrlMot ->initMb            (  pcMbDataAccessMot, uiMbY, uiMbX ) );
	  }
      RNOK(   pcMbDataCtrlRes ->initMb            (  pcMbDataAccessRes, uiMbY, uiMbX ) );
      RNOK(   m_pcControlMngIf->initMbForFiltering(  uiMbAddress ) );

      // Hanke@RWTH
      if( m_pcHighpassFrame ) {
        m_pcHighpassYuvBuffer = m_pcHighpassFrame->getFullPelYuvBuffer();
	  }else{
        m_pcHighpassYuvBuffer = NULL;
	  }

	
      if( 0 == (m_eLFMode & LFM_NO_FILTER))
	  {				
		RNOK( xFilterMb( pcMbDataAccessMot,
                       pcMbDataAccessRes,
                       pcFrame->getFullPelYuvBuffer(),
                       pcRefFrameList0,
                       pcRefFrameList1,
                       spatial_scalable_flg ) );  // SSUN@SHARP	  
		//*/
	  }

      if( m_eLFMode & LFM_EXTEND_INTRA_SUR )
	  {
        UInt uiMask = 0;

        RNOK( pcMbDataCtrlRes->getBoundaryMask( uiMbY, uiMbX, uiMask ) );

        if( uiMask )
		{
          IntYuvMbBufferExtension cBuffer;
          cBuffer.setAllSamplesToZero();

          cBuffer.loadSurrounding( pcFrame->getFullPelYuvBuffer() );

          RNOK( m_pcReconstructionBypass->padRecMb( &cBuffer, uiMask ) );
  	      pcFrame->getFullPelYuvBuffer()->loadBuffer( &cBuffer );
		}
	  }
	
      uiMbAddress = rcSH.getFMO()->getNextMBNr(uiMbAddress );    
	}
  }
  //<- ICU/ETRI DS

  // Hanke@RWTH: Reset pointer
  setHighpassFramePointer();

  return Err::m_nOK;
}




__inline ErrVal LoopFilter::xFilterMb( const MbDataAccess*  pcMbDataAccessMot,
                                       const MbDataAccess*  pcMbDataAccessRes,
                                       IntYuvPicBuffer*     pcYuvBuffer,
                                       RefFrameList*        pcRefFrameList0,
                                       RefFrameList*        pcRefFrameList1,
                                       bool                 spatial_scalable_flg)  // SSUN@SHARP
{

  const DFP& rcDFP      = pcMbDataAccessRes->getDeblockingFilterParameter(m_eLFMode & LFM_NO_INTER_FILTER);
  const Int iFilterIdc  = rcDFP.getDisableDeblockingFilterIdc();

  ROTRS( (m_eLFMode & LFM_NO_INTER_FILTER) && ! pcMbDataAccessRes->getMbData().isIntra(), Err::m_nOK );

  ROTRS( iFilterIdc == 1, Err::m_nOK );

  Bool b8x8 = pcMbDataAccessRes->getMbData().isTransformSize8x8();

  for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
  {
    if( !b8x8 || ( ( cIdx.x() & 1 ) == 0 ) )
    {
      m_aaaucBs[VER][cIdx.x()][cIdx.y()]  = xGetVerFilterStrength_RefIdx( pcMbDataAccessMot,
                                                                          pcMbDataAccessRes,
                                                                          cIdx,
                                                                          iFilterIdc,
                                                                          pcRefFrameList0,
                                                                          pcRefFrameList1,
                                                                          spatial_scalable_flg );  // SSUN@SHARP
    }
    else
    {
      m_aaaucBs[VER][cIdx.x()][cIdx.y()]  = 0;
    }
    if( !b8x8 || ( ( cIdx.y() & 1 ) == 0 ) )
    {
      m_aaaucBs[HOR][cIdx.x()][cIdx.y()]  = xGetHorFilterStrength_RefIdx( pcMbDataAccessMot,
                                                                          pcMbDataAccessRes,
                                                                          cIdx,
                                                                          iFilterIdc,
                                                                          pcRefFrameList0,
                                                                          pcRefFrameList1,
                                                                          spatial_scalable_flg );  // SSUN@SHARP
    }
    else
    {
      m_aaaucBs[HOR][cIdx.x()][cIdx.y()]  = 0;
    }
  }

  RNOK( xLumaVerFiltering(   *pcMbDataAccessRes, rcDFP, pcYuvBuffer ) );
  RNOK( xLumaHorFiltering(   *pcMbDataAccessRes, rcDFP, pcYuvBuffer ) );
  RNOK( xChromaVerFiltering( *pcMbDataAccessRes, rcDFP, pcYuvBuffer ) );
  RNOK( xChromaHorFiltering( *pcMbDataAccessRes, rcDFP, pcYuvBuffer ) );

  return Err::m_nOK;
}




__inline UInt LoopFilter::xGetVerFilterStrength_RefIdx( const MbDataAccess* pcMbDataAccessMot,
                                                        const MbDataAccess* pcMbDataAccessRes,
                                                        LumaIdx             cIdx,
                                                        Int                 iFilterIdc,
                                                        RefFrameList*       pcRefFrameList0,
                                                        RefFrameList*       pcRefFrameList1,
                                                        bool                spatial_scalable_flg )  // SSUN@SHARP
{
  // SSUN@SHARP JVT-P013r1
  if(spatial_scalable_flg)
  { 
    if( cIdx.x() )
    {
      const MbData& rcMbDataCurr  = pcMbDataAccessRes->getMbDataCurr();
      if( rcMbDataCurr.isIntra_BL() )
      {
        ROTRS( rcMbDataCurr.is4x4BlkCoded( cIdx                          ), 1 );
        ROTRS( rcMbDataCurr.is4x4BlkCoded( cIdx + CURR_MB_LEFT_NEIGHBOUR ), 1 );
        return( 0 );
      }
    }
    else if(pcMbDataAccessRes->isAvailableLeft() || ( pcMbDataAccessRes->isLeftMbExisting() && iFilterIdc != 2 )){
      const MbData& rcMbDataCurr  = pcMbDataAccessRes->getMbDataCurr();
      const MbData& rcMbDataLeft = pcMbDataAccessRes->getMbDataLeft();
      if(rcMbDataCurr.isIntra_BL() && rcMbDataLeft.isIntra_BL()){
        ROTRS( rcMbDataCurr.is4x4BlkCoded( cIdx ), 1 );
        ROTRS( rcMbDataLeft.is4x4BlkCoded( cIdx + LEFT_MB_LEFT_NEIGHBOUR), 1 );
        return(0);
      }
      else if(rcMbDataCurr.isIntra_BL()){
        ROTRS( rcMbDataLeft.isIntra_nonBL(), 4 );
        ROTRS( rcMbDataCurr.is4x4BlkCoded( cIdx ), 2 );
        if( m_pcHighpassYuvBuffer && !rcMbDataLeft.isIntra()) {
          ROTRS( rcMbDataLeft.is4x4BlkCoded( cIdx + LEFT_MB_LEFT_NEIGHBOUR), 2 ); // bugfix for FRExt-Mode, Hanke@RWTH
          ROTRS( m_pcHighpassYuvBuffer->isLeft4x4BlkNotZero ( cIdx + LEFT_MB_LEFT_NEIGHBOUR ), 2 );
        }
        else{
          ROTRS( rcMbDataLeft.is4x4BlkCoded( cIdx + LEFT_MB_LEFT_NEIGHBOUR), 2 );
        }
        return(1);
      }
      else if(rcMbDataLeft.isIntra_BL()){
        ROTRS( rcMbDataCurr.isIntra_nonBL(), 4 );
        ROTRS( rcMbDataLeft.is4x4BlkCoded( cIdx + LEFT_MB_LEFT_NEIGHBOUR), 2 );
        if( m_pcHighpassYuvBuffer ) {
          ROTRS( rcMbDataCurr.is4x4BlkCoded( cIdx ), 2 ); // bugfix for FRExt-Mode, Hanke@RWTH
          ROTRS( m_pcHighpassYuvBuffer->isCurr4x4BlkNotZero ( cIdx ), 2 );
        }
        else {
          ROTRS( rcMbDataCurr.is4x4BlkCoded( cIdx ), 2 );
        }
        return(1);
      }
    }
  } 
  // SSUN@SHARP end of JVT-P013r1
  else{  
    //-- Samsung 2005.02.xx
    if(pcMbDataAccessRes->getMbDataCurr().getMbMode() == INTRA_BL)
    {
      if( cIdx.x() )
      {
        return 1;		//	if not MB_boundary
      }

      // is either in same slice or deblocking across slice boundaries is enabled (and the XXX macroblock is inside the picture)
      if( ( pcMbDataAccessRes->isAvailableLeft() || ( pcMbDataAccessRes->isLeftMbExisting() && iFilterIdc != 2 ) ) &&
            pcMbDataAccessRes->getMbDataLeft().getMbMode() == INTRA_BL )
      {
        return 1;
      }
    }
    //--
  }

  ROFRS( pcMbDataAccessMot, cIdx.x() ? 3 : ((iFilterIdc==2 && !pcMbDataAccessRes->isAvailableLeft())|| !pcMbDataAccessRes->isLeftMbExisting()) ? 0 : 4 );

  const MbData& rcMbDataCurrMot = pcMbDataAccessMot->getMbDataCurr();
  const MbData& rcMbDataCurrRes = pcMbDataAccessRes->getMbDataCurr();
  Short         sHorMvThr       = 4;
  Short         sVerMvThr       = 4;

  if( cIdx.x() )
  {
    // this is a edge inside of a macroblock
    ROTRS( rcMbDataCurrMot.isIntra(), 3 );

    // Hanke@RWTH
    if( m_pcHighpassYuvBuffer ) {
      ROTRS( rcMbDataCurrRes.is4x4BlkCoded( cIdx                          ), 2 ); // bugfix for FRExt-Mode, Hanke@RWTH
      ROTRS( m_pcHighpassYuvBuffer->isCurr4x4BlkNotZero ( cIdx                          ), 2 );
      ROTRS( rcMbDataCurrRes.is4x4BlkCoded( cIdx + CURR_MB_LEFT_NEIGHBOUR ), 2 ); // bugfix for FRExt-Mode, Hanke@RWTH
      ROTRS( m_pcHighpassYuvBuffer->isCurr4x4BlkNotZero ( cIdx + CURR_MB_LEFT_NEIGHBOUR ), 2 );
    }else{ 
      ROTRS( rcMbDataCurrRes.is4x4BlkCoded( cIdx                          ), 2 );
      ROTRS( rcMbDataCurrRes.is4x4BlkCoded( cIdx + CURR_MB_LEFT_NEIGHBOUR ), 2 );
    }
    
    if( rcMbDataCurrMot.isInterPMb() )
    {
      return xCheckMvDataP_RefIdx( rcMbDataCurrMot, cIdx, rcMbDataCurrMot, cIdx + CURR_MB_LEFT_NEIGHBOUR,
                                   sHorMvThr, sVerMvThr,
                                   *pcRefFrameList0 );
    }
    return   xCheckMvDataB_RefIdx( rcMbDataCurrMot, cIdx, rcMbDataCurrMot, cIdx + CURR_MB_LEFT_NEIGHBOUR,
                                   sHorMvThr, sVerMvThr,
                                   *pcRefFrameList0, *pcRefFrameList1 );
  }


  // if we get here we are on a macroblock edge
  ROTRS( iFilterIdc == 2 && ! pcMbDataAccessMot->isAvailableLeft(), 0 );
  ROTRS( ! pcMbDataAccessMot->isLeftMbExisting(),                   0 );


  const MbData& rcMbDataLeftMot = pcMbDataAccessMot->getMbDataLeft();
  const MbData& rcMbDataLeftRes = pcMbDataAccessRes->getMbDataLeft();

  ROTRS( LFM_DEFAULT_FILTER != m_eLFMode && rcMbDataCurrMot.isIntra() ^ rcMbDataLeftMot.isIntra(), 0 );
  
  ROTRS( rcMbDataCurrMot.isIntra(), 4 );
  ROTRS( rcMbDataLeftMot.isIntra(), 4 );

  // Hanke@RWTH
  if( m_pcHighpassYuvBuffer ) {
    ROTRS( rcMbDataCurrRes.is4x4BlkCoded( cIdx                          ), 2 ); // bugfix for FRExt-Mode, Hanke@RWTH
    ROTRS( m_pcHighpassYuvBuffer->isCurr4x4BlkNotZero ( cIdx                          ), 2 );
    ROTRS( rcMbDataLeftRes.is4x4BlkCoded( cIdx + LEFT_MB_LEFT_NEIGHBOUR ), 2 ); // bugfix for FRExt-Mode, Hanke@RWTH
    ROTRS( m_pcHighpassYuvBuffer->isLeft4x4BlkNotZero ( cIdx + LEFT_MB_LEFT_NEIGHBOUR ), 2 );
  }else{ 
    ROTRS( rcMbDataCurrRes.is4x4BlkCoded( cIdx                          ), 2 );
    ROTRS( rcMbDataLeftRes.is4x4BlkCoded( cIdx + LEFT_MB_LEFT_NEIGHBOUR ), 2 );
  }
  
  if( rcMbDataCurrMot.isInterPMb() && rcMbDataLeftMot.isInterPMb())
  {
    return xCheckMvDataP_RefIdx( rcMbDataCurrMot, cIdx, rcMbDataLeftMot, cIdx + LEFT_MB_LEFT_NEIGHBOUR,
                                 sHorMvThr, sVerMvThr,
                                 *pcRefFrameList0);
  }
  return   xCheckMvDataB_RefIdx( rcMbDataCurrMot, cIdx, rcMbDataLeftMot, cIdx + LEFT_MB_LEFT_NEIGHBOUR,
                                 sHorMvThr, sVerMvThr,
                                 *pcRefFrameList0, *pcRefFrameList1 );
}



__inline UInt LoopFilter::xGetHorFilterStrength_RefIdx( const MbDataAccess* pcMbDataAccessMot,
                                                        const MbDataAccess* pcMbDataAccessRes,
                                                        LumaIdx             cIdx,
                                                        Int                 iFilterIdc,
                                                        RefFrameList*       pcRefFrameList0,
                                                        RefFrameList*       pcRefFrameList1,
                                                        bool                spatial_scalable_flg )  // SSUN@SHARP
{
  // SSUN@SHARP JVT-P013r1
  if(spatial_scalable_flg){  
    if( cIdx.y() )
    {
      const MbData& rcMbDataCurr  = pcMbDataAccessRes->getMbDataCurr();
      if( rcMbDataCurr.isIntra_BL() )
      {
        ROTRS( rcMbDataCurr.is4x4BlkCoded( cIdx                          ), 1 );
        ROTRS( rcMbDataCurr.is4x4BlkCoded( cIdx + CURR_MB_ABOVE_NEIGHBOUR ), 1 );
        return( 0 );
      }
    }
    else if(pcMbDataAccessRes->isAvailableAbove() || ( pcMbDataAccessRes->isAboveMbExisting() && iFilterIdc != 2 )){
      const MbData& rcMbDataCurr = pcMbDataAccessRes->getMbDataCurr();
      const MbData& rcMbDataAbove = pcMbDataAccessRes->getMbDataAbove();
      if(rcMbDataCurr.isIntra_BL() && rcMbDataAbove.isIntra_BL()){
        ROTRS( rcMbDataCurr.is4x4BlkCoded( cIdx ), 1 );
        ROTRS( rcMbDataAbove.is4x4BlkCoded( cIdx + ABOVE_MB_ABOVE_NEIGHBOUR), 1 );
        return(0);
      }
      if(rcMbDataCurr.isIntra_BL()){
        ROTRS( rcMbDataAbove.isIntra_nonBL(), 4 );
        ROTRS( rcMbDataCurr.is4x4BlkCoded( cIdx ), 2 );
        if( m_pcHighpassYuvBuffer && !rcMbDataAbove.isIntra() ) {
          ROTRS( rcMbDataAbove.is4x4BlkCoded( cIdx + ABOVE_MB_ABOVE_NEIGHBOUR), 2 ); // bugfix for FRExt-Mode, Hanke@RWTH
          ROTRS( m_pcHighpassYuvBuffer->isAbove4x4BlkNotZero ( cIdx + ABOVE_MB_ABOVE_NEIGHBOUR ), 2 );
        }
        else{
          ROTRS( rcMbDataAbove.is4x4BlkCoded( cIdx + ABOVE_MB_ABOVE_NEIGHBOUR), 2 );
        }
        return(1);
      }
      else if(rcMbDataAbove.isIntra_BL()){
        ROTRS( rcMbDataCurr.isIntra_nonBL(), 4 );
        ROTRS( rcMbDataAbove.is4x4BlkCoded( cIdx + ABOVE_MB_ABOVE_NEIGHBOUR), 2 );
        if( m_pcHighpassYuvBuffer ) {
          ROTRS( rcMbDataCurr.is4x4BlkCoded( cIdx ), 2 ); // bugfix for FRExt-Mode, Hanke@RWTH
          ROTRS( m_pcHighpassYuvBuffer->isCurr4x4BlkNotZero ( cIdx ), 2 );
        }
        else {
          ROTRS( rcMbDataCurr.is4x4BlkCoded( cIdx ), 2 );
        }
        return(1);
      }
    }
  }
  // SSUN@SHARP end of JVT-P013r1
  else{  
    //-- Samsung 2005.02.xx
    if(pcMbDataAccessRes->getMbDataCurr().getMbMode() == INTRA_BL)
    {
      if( cIdx.y() )
      {
        return 1;		//	if not MB_boundary
      }

      // is either in same slice or deblocking across slice boundaries is enabled (and the XXX macroblock is inside the picture)
      if( ( pcMbDataAccessRes->isAvailableAbove() || ( pcMbDataAccessRes->isAboveMbExisting() && iFilterIdc != 2 ) ) &&
            pcMbDataAccessRes->getMbDataAbove().getMbMode() == INTRA_BL )
      {
        return 1;
      }
    }
    //--
  }
  ROFRS( pcMbDataAccessMot, cIdx.y() ? 3 : ((iFilterIdc==2 && !pcMbDataAccessRes->isAvailableAbove())|| !pcMbDataAccessRes->isAboveMbExisting()) ? 0 : 4 );

  const MbData& rcMbDataCurrMot = pcMbDataAccessMot->getMbDataCurr();
  const MbData& rcMbDataCurrRes = pcMbDataAccessRes->getMbDataCurr();
  Short         sHorMvThr       = 4;
  Short         sVerMvThr       = 4;

  if( cIdx.y() )
  {
    // internal edge
    ROTRS( rcMbDataCurrMot.isIntra(), 3 );

    // Hanke@RWTH
    if( m_pcHighpassYuvBuffer ) {
      ROTRS( rcMbDataCurrRes.is4x4BlkCoded( cIdx                           ), 2 ); // bugfix for FRExt-Mode, Hanke@RWTH
      ROTRS( m_pcHighpassYuvBuffer->isCurr4x4BlkNotZero ( cIdx                           ), 2 );
      ROTRS( rcMbDataCurrRes.is4x4BlkCoded( cIdx + CURR_MB_ABOVE_NEIGHBOUR ), 2 ); // bugfix for FRExt-Mode, Hanke@RWTH
      ROTRS( m_pcHighpassYuvBuffer->isCurr4x4BlkNotZero ( cIdx + CURR_MB_ABOVE_NEIGHBOUR ), 2 );
    }else{
      ROTRS( rcMbDataCurrRes.is4x4BlkCoded( cIdx                           ), 2 );
      ROTRS( rcMbDataCurrRes.is4x4BlkCoded( cIdx + CURR_MB_ABOVE_NEIGHBOUR ), 2 );
    }
    
    if( rcMbDataCurrMot.isInterPMb() )
    {
      return xCheckMvDataP_RefIdx( rcMbDataCurrMot, cIdx, rcMbDataCurrMot, cIdx + CURR_MB_ABOVE_NEIGHBOUR,
                                   sHorMvThr, sVerMvThr,
                                   *pcRefFrameList0 );
    }
    return   xCheckMvDataB_RefIdx( rcMbDataCurrMot, cIdx, rcMbDataCurrMot, cIdx + CURR_MB_ABOVE_NEIGHBOUR,
                                   sHorMvThr, sVerMvThr,
                                   *pcRefFrameList0, *pcRefFrameList1 );
  }

  // if we get here we are on a macroblock edge
  ROTRS( iFilterIdc == 2 && ! pcMbDataAccessMot->isAvailableAbove(),  0 );
  ROTRS( ! pcMbDataAccessMot->isAboveMbExisting(),                    0 );


  const MbData& rcMbDataAboveMot = pcMbDataAccessMot->getMbDataAbove();
  const MbData& rcMbDataAboveRes = pcMbDataAccessRes->getMbDataAbove();
  
  ROTRS( LFM_DEFAULT_FILTER != m_eLFMode && rcMbDataCurrMot.isIntra() ^ rcMbDataAboveMot.isIntra(), 0 );
  
  ROTRS( rcMbDataCurrMot. isIntra(), 4 );
  ROTRS( rcMbDataAboveMot.isIntra(), 4 );

  // Hanke@RWTH
  if( m_pcHighpassYuvBuffer ) {
    ROTRS( rcMbDataCurrRes. is4x4BlkCoded( cIdx                            ), 2 ); // bugfix for FRExt-Mode, Hanke@RWTH
    ROTRS( m_pcHighpassYuvBuffer->isCurr4x4BlkNotZero  ( cIdx                            ), 2 );
    ROTRS( rcMbDataAboveRes.is4x4BlkCoded( cIdx + ABOVE_MB_ABOVE_NEIGHBOUR ), 2 ); // bugfix for FRExt-Mode, Hanke@RWTH
    ROTRS( m_pcHighpassYuvBuffer->isAbove4x4BlkNotZero ( cIdx + ABOVE_MB_ABOVE_NEIGHBOUR ), 2 );
  }else{
    ROTRS( rcMbDataCurrRes. is4x4BlkCoded( cIdx                            ), 2 );
    ROTRS( rcMbDataAboveRes.is4x4BlkCoded( cIdx + ABOVE_MB_ABOVE_NEIGHBOUR ), 2 );
  }
  
  if( rcMbDataCurrMot.isInterPMb() && rcMbDataAboveMot.isInterPMb())
  {
    return xCheckMvDataP_RefIdx( rcMbDataCurrMot, cIdx, rcMbDataAboveMot, cIdx + ABOVE_MB_ABOVE_NEIGHBOUR,
                                 sHorMvThr, sVerMvThr,
                                 *pcRefFrameList0 );
  }
  return   xCheckMvDataB_RefIdx( rcMbDataCurrMot, cIdx, rcMbDataAboveMot, cIdx + ABOVE_MB_ABOVE_NEIGHBOUR,
                                 sHorMvThr, sVerMvThr,
                                 *pcRefFrameList0, *pcRefFrameList1 );

}









UChar LoopFilter::xCheckMvDataP_RefIdx( const MbData& rcQMbData,
                                        const LumaIdx cQIdx,
                                        const MbData& rcPMbData,
                                        const LumaIdx cPIdx,
                                        const Short   sHorMvThr,
                                        const Short   sVerMvThr,
                                        RefFrameList& rcRefFrameList0 )
{
  const MbMotionData& rcMbMotionDataL0Q = rcQMbData.getMbMotionData( LIST_0 );
  const MbMotionData& rcMbMotionDataL0P = rcPMbData.getMbMotionData( LIST_0 );

  IntFrame* pcRefPicL0Q = rcRefFrameList0[ rcMbMotionDataL0Q.getRefIdx( cQIdx ) ];
  IntFrame* pcRefPicL0P = rcRefFrameList0[ rcMbMotionDataL0P.getRefIdx( cPIdx ) ];

  // different reference pictures
  ROTRS( pcRefPicL0Q != pcRefPicL0P, 1 );

  // check the motion vector distance
  const Mv& cMvQ = rcMbMotionDataL0Q.getMv( cQIdx );
  const Mv& cMvP = rcMbMotionDataL0P.getMv( cPIdx );

  ROTRS( cMvP.getAbsHorDiff( cMvQ ) >= sHorMvThr, 1 );
  ROTRS( cMvP.getAbsVerDiff( cMvQ ) >= sVerMvThr, 1 );
  
  return 0;
}





UChar LoopFilter::xCheckMvDataB_RefIdx( const MbData& rcQMbData,
                                        const LumaIdx cQIdx,
                                        const MbData& rcPMbData,
                                        const LumaIdx cPIdx,
                                        const Short   sHorMvThr,
                                        const Short   sVerMvThr,
                                        RefFrameList& rcRefFrameList0,
                                        RefFrameList& rcRefFrameList1 )
{
  const MbMotionData& rcMbMotionDataL0Q = rcQMbData.getMbMotionData( LIST_0 );
  const MbMotionData& rcMbMotionDataL1Q = rcQMbData.getMbMotionData( LIST_1 );
  const MbMotionData& rcMbMotionDataL0P = rcPMbData.getMbMotionData( LIST_0 );
  const MbMotionData& rcMbMotionDataL1P = rcPMbData.getMbMotionData( LIST_1 );

  IntFrame* pcRefPicL0Q = rcRefFrameList0[ rcMbMotionDataL0Q.getRefIdx( cQIdx ) ];
  IntFrame* pcRefPicL1Q = rcRefFrameList1[ rcMbMotionDataL1Q.getRefIdx( cQIdx ) ];
  IntFrame* pcRefPicL0P = rcRefFrameList0[ rcMbMotionDataL0P.getRefIdx( cPIdx ) ];
  IntFrame* pcRefPicL1P = rcRefFrameList1[ rcMbMotionDataL1P.getRefIdx( cPIdx ) ];

  UInt uiNumberOfUsedPic;
  {
    // check the number of used ref frames
    UInt uiQNumberOfUsedPic = ( pcRefPicL0Q ? 1 : 0 ) + ( pcRefPicL1Q ? 1 : 0 );
    UInt uiPNumberOfUsedPic = ( pcRefPicL0P ? 1 : 0 ) + ( pcRefPicL1P ? 1 : 0 );
    ROTRS( uiPNumberOfUsedPic != uiQNumberOfUsedPic, 1 );
    uiNumberOfUsedPic = uiPNumberOfUsedPic;
  }

  if( 1 == uiNumberOfUsedPic )
  {
    // this is the easy part
    // check whether they ref diff ref pic or not
    IntFrame* pcRefPicQ = ( pcRefPicL0Q ? pcRefPicL0Q : pcRefPicL1Q );
    IntFrame* pcRefPicP = ( pcRefPicL0P ? pcRefPicL0P : pcRefPicL1P );
    ROTRS( pcRefPicQ != pcRefPicP, 1 );

    // check the motion vector distance
    const Mv& cMvQ = ( pcRefPicL0Q ? rcMbMotionDataL0Q.getMv( cQIdx ) : rcMbMotionDataL1Q.getMv( cQIdx ) );
    const Mv& cMvP = ( pcRefPicL0P ? rcMbMotionDataL0P.getMv( cPIdx ) : rcMbMotionDataL1P.getMv( cPIdx ) );

    ROTRS( cMvP.getAbsHorDiff( cMvQ ) >= sHorMvThr, 1 );
    ROTRS( cMvP.getAbsVerDiff( cMvQ ) >= sVerMvThr, 1 );
    
    return 0;
  }


  // both ref pic are used for both blocks
  if( pcRefPicL1P != pcRefPicL0P )
  {
    // at least two diff ref pic are in use
    if( pcRefPicL1P != pcRefPicL1Q )
    {
      ROTRS( pcRefPicL1P != pcRefPicL0Q, 1 );
      ROTRS( pcRefPicL0P != pcRefPicL1Q, 1 );

      // rcRefPicL0P == rcRefPicL1Q && rcRefPicL1P == rcRefPicL0Q
      // check the motion vector distance
      const Mv& cMvQ0 = rcMbMotionDataL0Q.getMv( cQIdx );
      const Mv& cMvP0 = rcMbMotionDataL0P.getMv( cPIdx );
      const Mv& cMvQ1 = rcMbMotionDataL1Q.getMv( cQIdx );
      const Mv& cMvP1 = rcMbMotionDataL1P.getMv( cPIdx );

      ROTRS( cMvP0.getAbsHorDiff( cMvQ1 ) >= sHorMvThr, 1 );
      ROTRS( cMvP0.getAbsVerDiff( cMvQ1 ) >= sVerMvThr, 1 );
      ROTRS( cMvP1.getAbsHorDiff( cMvQ0 ) >= sHorMvThr, 1 );
      ROTRS( cMvP1.getAbsVerDiff( cMvQ0 ) >= sVerMvThr, 1 );

      return 0;
    }

    // rcRefPicL1P == rcRefPicL1Q
    ROTRS( pcRefPicL0P != pcRefPicL0Q, 1 );

    // rcRefPicL0P == rcRefPicL0Q && rcRefPicL1P == rcRefPicL1Q
    // check the motion vector distance
    const Mv& cMvQ0 = rcMbMotionDataL0Q.getMv( cQIdx );
    const Mv& cMvP0 = rcMbMotionDataL0P.getMv( cPIdx );
    const Mv& cMvQ1 = rcMbMotionDataL1Q.getMv( cQIdx );
    const Mv& cMvP1 = rcMbMotionDataL1P.getMv( cPIdx );

    ROTRS( cMvP0.getAbsHorDiff( cMvQ0 ) >= sHorMvThr, 1 );
    ROTRS( cMvP0.getAbsVerDiff( cMvQ0 ) >= sVerMvThr, 1 );
    ROTRS( cMvP1.getAbsHorDiff( cMvQ1 ) >= sHorMvThr, 1 );
    ROTRS( cMvP1.getAbsVerDiff( cMvQ1 ) >= sVerMvThr, 1 );
    
    return 0;
  }
 
  //  rcRefPicL1P == rcRefPicL0P
  ROTRS( pcRefPicL1Q != pcRefPicL0Q, 1 ) ;
  ROTRS( pcRefPicL0P != pcRefPicL0Q, 1 ) ;

  // rcRefPicL0P == rcRefPicL0Q == rcRefPicL1P == rcRefPicL1Q
  // check the motion vector distance
  const Mv& cMvQ0 = rcMbMotionDataL0Q.getMv( cQIdx );
  const Mv& cMvP0 = rcMbMotionDataL0P.getMv( cPIdx );
  const Mv& cMvQ1 = rcMbMotionDataL1Q.getMv( cQIdx );
  const Mv& cMvP1 = rcMbMotionDataL1P.getMv( cPIdx );

  Bool              bSameListCond  = ( (cMvP0.getAbsHorDiff( cMvQ0 ) >= sHorMvThr) );
  bSameListCond = ( bSameListCond || ( (cMvP0.getAbsVerDiff( cMvQ0 ) >= sVerMvThr) ) );
  bSameListCond = ( bSameListCond || ( (cMvP1.getAbsHorDiff( cMvQ1 ) >= sHorMvThr) ) );
  bSameListCond = ( bSameListCond || ( (cMvP1.getAbsVerDiff( cMvQ1 ) >= sVerMvThr) ) );
  Bool              bDiffListCond  = ( (cMvP0.getAbsHorDiff( cMvQ1 ) >= sHorMvThr) );
  bDiffListCond = ( bDiffListCond || ( (cMvP0.getAbsVerDiff( cMvQ1 ) >= sVerMvThr) ) );
  bDiffListCond = ( bDiffListCond || ( (cMvP1.getAbsHorDiff( cMvQ0 ) >= sHorMvThr) ) );
  bDiffListCond = ( bDiffListCond || ( (cMvP1.getAbsVerDiff( cMvQ0 ) >= sVerMvThr) ) );

  ROTRS( bSameListCond && bDiffListCond, 1 );

  return 0;
}




__inline Void LoopFilter::xFilter( XPel* pFlt, const Int& iOffset, const Int& iIndexA, const Int& iIndexB, const UChar& ucBs, const Bool& bLum )
{
  const Int iAlpha = g_acAlphaClip[ iIndexA ].ucAlpha;

  Int P0 = pFlt[-iOffset];
  Int Q0 = pFlt[       0];

  Int iDelta = Q0 - P0;
  Int iAbsDelta  = abs( iDelta  );

  AOF_DBG( ucBs );

  ROFVS( iAbsDelta < iAlpha );


  const Int iBeta = g_aucBetaTab [ iIndexB ];

  Int P1  = pFlt[-2*iOffset];
  Int Q1  = pFlt[   iOffset];

  ROFVS( (abs(P0 - P1) < iBeta) && (abs(Q0 - Q1) < iBeta) );

  if( ucBs < 4 )
  {
    Int C0 = g_acAlphaClip[ iIndexA ].aucClip[ucBs];

    if( bLum )
    {
      Int P2 = pFlt[-3*iOffset] ;
      Int Q2 = pFlt[ 2*iOffset] ;
      Int aq = (( abs( Q2 - Q0 ) < iBeta ) ? 1 : 0 );
      Int ap = (( abs( P2 - P0 ) < iBeta ) ? 1 : 0 );

      if( ap )
      {
        pFlt[-2*iOffset] = P1 + gClipMinMax((P2 + ((P0 + Q0 + 1)>>1) - (P1<<1)) >> 1, -C0, C0 );
      }

      if( aq  )
      {
        pFlt[   iOffset] = Q1 + gClipMinMax((Q2 + ((P0 + Q0 + 1)>>1) - (Q1<<1)) >> 1, -C0, C0 );
      }

      C0 += ap + aq -1;
    }

    C0++;
    Int iDiff      = gClipMinMax(((iDelta << 2) + (P1 - Q1) + 4) >> 3, -C0, C0 ) ;
    pFlt[-iOffset] = gClip( P0 + iDiff );
    pFlt[       0] = gClip( Q0 - iDiff );
    return;
  }


  if( ! bLum )
  {
    pFlt[         0] = ((Q1 << 1) + Q0 + P1 + 2) >> 2;
    pFlt[  -iOffset] = ((P1 << 1) + P0 + Q1 + 2) >> 2;
  }
  else
  {
    Int P2 = pFlt[-3*iOffset] ;
    Int Q2 = pFlt[ 2*iOffset] ;
    Bool bEnable  = (iAbsDelta < ((iAlpha >> 2) + 2));
    Bool aq       = bEnable & ( abs( Q2 - Q0 ) < iBeta );
    Bool ap       = bEnable & ( abs( P2 - P0 ) < iBeta );
    Int PQ0 = P0 + Q0;

    if( aq )
    {
      pFlt[         0] = (P1 + ((Q1 + PQ0) << 1) +  Q2 + 4) >> 3;
      pFlt[   iOffset] = (PQ0 +Q1 + Q2 + 2) >> 2;
      pFlt[ 2*iOffset] = (((pFlt[ 3*iOffset] + Q2) <<1) + Q2 + Q1 + PQ0 + 4) >> 3;
    }
    else
    {
      pFlt[         0] = ((Q1 << 1) + Q0 + P1 + 2) >> 2;
    }

    if( ap )
    {
      pFlt[  -iOffset] = (Q1 + ((P1 + PQ0) << 1) +  P2 + 4) >> 3;
      pFlt[-2*iOffset] = (PQ0 +P1 + P2 + 2) >> 2;
      pFlt[-3*iOffset] = (((pFlt[-4*iOffset] + P2) <<1) + pFlt[-3*iOffset] + P1 + PQ0 + 4) >> 3;
    }
    else
    {
      pFlt[  -iOffset] = ((P1 << 1) + P0 + Q1 + 2) >> 2;
    }
  }
}









__inline ErrVal LoopFilter::xLumaVerFiltering( const MbDataAccess& rcMbDataAccess, const DFP& rcDFP, IntYuvPicBuffer* pcYuvBuffer )
{
  Int   iStride = pcYuvBuffer->getLStride();
  XPel* pPelLum = pcYuvBuffer->getMbLumAddr();
  Int   iCurrQp = rcMbDataAccess.getMbData().getQpLF();

  //===== filtering of left macroblock edge =====
  {
    Int iLeftQp = rcMbDataAccess.getMbDataLeft().getQpLF();
    Int iQp     = ( iLeftQp + iCurrQp + 1) >> 1;

    //-----  curr == FRM && left == FRM  or  curr == FLD && left == FLD  -----
    Int iIndexA = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
    Int iIndexB = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);

    for( Int yBlk = 0; yBlk < 4; yBlk++)
    {
      const UChar ucBs = m_aaaucBs[VER][0][yBlk];
      if( 0 != ucBs )
      {
        xFilter( pPelLum,           1, iIndexA, iIndexB, ucBs, true );
        xFilter( pPelLum+  iStride, 1, iIndexA, iIndexB, ucBs, true );
        xFilter( pPelLum+2*iStride, 1, iIndexA, iIndexB, ucBs, true );
        xFilter( pPelLum+3*iStride, 1, iIndexA, iIndexB, ucBs, true );
      }
      pPelLum += 4*iStride;
    }
  }

  pPelLum -= 16*iStride-4;

  Int iIndexA = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iCurrQp, 0, 51);
  Int iIndexB = gClipMinMax( rcDFP.getSliceBetaOffset()    + iCurrQp, 0, 51);

  for( Int xBlk = 1; xBlk < 4; xBlk++)
  {
    for( Int yBlk = 0; yBlk < 4; yBlk++)
    {
      const UChar ucBs = m_aaaucBs[VER][xBlk][yBlk];
      if( 0 != ucBs )
      {
        xFilter( pPelLum,           1, iIndexA, iIndexB, ucBs, true );
        xFilter( pPelLum+  iStride, 1, iIndexA, iIndexB, ucBs, true );
        xFilter( pPelLum+2*iStride, 1, iIndexA, iIndexB, ucBs, true );
        xFilter( pPelLum+3*iStride, 1, iIndexA, iIndexB, ucBs, true );
      }
      pPelLum += 4*iStride;
    }
    pPelLum -= 16*iStride-4;
  }

  return Err::m_nOK;
}




__inline ErrVal LoopFilter::xLumaHorFiltering( const MbDataAccess& rcMbDataAccess, const DFP& rcDFP, IntYuvPicBuffer* pcYuvBuffer )
{
  Int   iStride = pcYuvBuffer->getLStride();
  XPel* pPelLum = pcYuvBuffer->getMbLumAddr();
  Int   iCurrQp = rcMbDataAccess.getMbData().getQpLF();

  //===== filtering of upper macroblock edge =====
  {
    //-----  any other combination than curr = FRM, above = FLD  -----
    Int iAboveQp  = rcMbDataAccess.getMbDataAbove().getQpLF();
    Int iQp       = ( iAboveQp + iCurrQp + 1 ) >> 1;

    Int iIndexA   = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
    Int iIndexB   = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);

    for( Int xBlk = 0; xBlk < 4; xBlk++)
    {
      const UChar ucBs = m_aaaucBs[HOR][xBlk][0];
      if( 0 != ucBs )
      {
        xFilter( pPelLum,   iStride, iIndexA, iIndexB, ucBs, true );
        xFilter( pPelLum+1, iStride, iIndexA, iIndexB, ucBs, true );
        xFilter( pPelLum+2, iStride, iIndexA, iIndexB, ucBs, true );
        xFilter( pPelLum+3, iStride, iIndexA, iIndexB, ucBs, true );
      }
      pPelLum += 4;
    }
    pPelLum -= 16;
  }

  pPelLum += 4*iStride;
  Int iIndexA = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iCurrQp, 0, 51);
  Int iIndexB = gClipMinMax( rcDFP.getSliceBetaOffset()    + iCurrQp, 0, 51);

  for( Int yBlk = 1; yBlk < 4; yBlk++)
  {
    for( Int xBlk = 0; xBlk < 4; xBlk++)
    {
      const UChar ucBs = m_aaaucBs[HOR][xBlk][yBlk];
      if( 0 != ucBs )
      {
        xFilter( pPelLum,   iStride, iIndexA, iIndexB, ucBs, true );
        xFilter( pPelLum+1, iStride, iIndexA, iIndexB, ucBs, true );
        xFilter( pPelLum+2, iStride, iIndexA, iIndexB, ucBs, true );
        xFilter( pPelLum+3, iStride, iIndexA, iIndexB, ucBs, true );
      }
      pPelLum += 4;
    }
    pPelLum += 4*iStride - 16;
  }

  return Err::m_nOK;
}





__inline ErrVal LoopFilter::xChromaHorFiltering( const MbDataAccess& rcMbDataAccess, const DFP& rcDFP, IntYuvPicBuffer* pcYuvBuffer )
{
  Int   iStride = pcYuvBuffer->getCStride();
  XPel* pPelCb  = pcYuvBuffer->getMbCbAddr();
  XPel* pPelCr  = pcYuvBuffer->getMbCrAddr();
  Int   iCurrQp = rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.getMbDataCurr().getQpLF() );

  //===== filtering of upper macroblock edge =====
  {
    //-----  any other combination than curr = FRM, above = FLD  -----
    Int iAboveQp  = rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.getMbDataAbove().getQpLF() );
    Int iQp       = ( iCurrQp + iAboveQp + 1 ) >> 1;

    Int iIndexA   = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
    Int iIndexB   = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);

    for( Int xBlk = 0; xBlk < 4; xBlk++)
    {
      const UChar ucBs = m_aaaucBs[HOR][xBlk][0];
      if( 0 != ucBs )
      {
        xFilter( pPelCb,   iStride, iIndexA, iIndexB, ucBs, false );
        xFilter( pPelCb+1, iStride, iIndexA, iIndexB, ucBs, false );
        xFilter( pPelCr,   iStride, iIndexA, iIndexB, ucBs, false );
        xFilter( pPelCr+1, iStride, iIndexA, iIndexB, ucBs, false );
      }
      pPelCb += 2;
      pPelCr += 2;
    }
    pPelCb -= 8;
    pPelCr -= 8;
  }

  pPelCb += 4*iStride;
  pPelCr += 4*iStride;

  // now we filter the remaining edge
  Int iIndexA = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iCurrQp, 0, 51);
  Int iIndexB = gClipMinMax( rcDFP.getSliceBetaOffset()    + iCurrQp, 0, 51);

  for( Int xBlk = 0; xBlk < 4; xBlk++)
  {
    const UChar ucBs = m_aaaucBs[HOR][xBlk][2];
    if( 0 != ucBs )
    {
      xFilter( pPelCb,   iStride, iIndexA, iIndexB, ucBs, false );
      xFilter( pPelCb+1, iStride, iIndexA, iIndexB, ucBs, false );
      xFilter( pPelCr,   iStride, iIndexA, iIndexB, ucBs, false );
      xFilter( pPelCr+1, iStride, iIndexA, iIndexB, ucBs, false );
    }
    pPelCb += 2;
    pPelCr += 2;
  }

  return Err::m_nOK;
}



__inline ErrVal LoopFilter::xChromaVerFiltering( const MbDataAccess& rcMbDataAccess, const DFP& rcDFP, IntYuvPicBuffer* pcYuvBuffer )
{
  Int   iStride = pcYuvBuffer->getCStride();
  XPel* pPelCb  = pcYuvBuffer->getMbCbAddr();
  XPel* pPelCr  = pcYuvBuffer->getMbCrAddr();
  Int   iCurrQp = rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.getMbDataCurr().getQpLF() );

  //===== filtering of left macroblock edge =====
  {
    //-----  curr == FRM && left == FRM  or  curr == FLD && left == FLD  -----
    Int iLeftQp = rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.getMbDataLeft().getQpLF() );
    Int iQp     = ( iCurrQp + iLeftQp + 1 ) >> 1;

    Int iIndexA = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
    Int iIndexB = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);

    for( Int yBlk = 0; yBlk < 4; yBlk++)
    {
      const UChar ucBs = m_aaaucBs[VER][0][yBlk];
      if( 0 != ucBs )
      {
        xFilter( pPelCb,         1, iIndexA, iIndexB, ucBs, false );
        xFilter( pPelCb+iStride, 1, iIndexA, iIndexB, ucBs, false );
        xFilter( pPelCr,         1, iIndexA, iIndexB, ucBs, false );
        xFilter( pPelCr+iStride, 1, iIndexA, iIndexB, ucBs, false );
      }
      pPelCb += 2*iStride;
      pPelCr += 2*iStride;
    }
  }

  pPelCb -= 8*iStride-4;
  pPelCr -= 8*iStride-4;

  Int iIndexA = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iCurrQp, 0, 51);
  Int iIndexB = gClipMinMax( rcDFP.getSliceBetaOffset()    + iCurrQp, 0, 51);
  for( Int yBlk = 0; yBlk < 4; yBlk++)
  {
    const UChar ucBs = m_aaaucBs[VER][2][yBlk];
    if( 0 != ucBs )
    {
      xFilter( pPelCb,         1, iIndexA, iIndexB, ucBs, false );
      xFilter( pPelCb+iStride, 1, iIndexA, iIndexB, ucBs, false );
      xFilter( pPelCr,         1, iIndexA, iIndexB, ucBs, false );
      xFilter( pPelCr+iStride, 1, iIndexA, iIndexB, ucBs, false );
    }
    pPelCb += 2*iStride;
    pPelCr += 2*iStride;
  }

  return Err::m_nOK;
}



H264AVC_NAMESPACE_END

