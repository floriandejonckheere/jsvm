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

const UChar LoopFilter::g_aucTcTab_H241RCDO[52]  =
{
    0,  0,  0,  0,  0,  0,  0,  0, 
    0,  0,  0,  0,  0,  0,  0,  0, 
    1,  1,  1,  1,  1,  2,  2,  2,
    2,  2,  2,  2,  2,  2,  2,  3,
    3,  3,  3,  4,  4,  4,  5,  5,
    6,  6,  7,  8,  9,  9, 11, 12,
   13, 13, 16, 18
};
const UChar LoopFilter::g_aucBetaTab_H241RCDO[52]  =
{
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
    6,  7,  8,  9, 10, 11, 12, 13,
   14, 15, 16, 17, 18, 20, 22, 24,
   26, 28, 30, 32, 34, 36, 38, 40,
   42, 44, 46, 48, 50, 52, 54, 56,
   58, 60, 62, 64
};


LoopFilter::LoopFilter() :
  m_pcControlMngIf( NULL ),
  m_pcRecFrameUnit( NULL ),
	m_pcHighpassYuvBuffer( NULL )	//TMM_INTERLACE
  , m_bRCDO( false )
  , m_pcRCDOSliceHeader( NULL )
{
  m_eLFMode  = LFM_DEFAULT_FILTER;
  m_apcIntYuvBuffer[0] = m_apcIntYuvBuffer[1] = m_apcIntYuvBuffer[2] = m_apcIntYuvBuffer[3] = NULL;
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

ErrVal LoopFilter::init(  ControlMngIf*         pcControlMngIf,
                          ReconstructionBypass* pcReconstructionBypass
                        )
{
  ROT( NULL == pcControlMngIf );
  ROT( NULL == pcReconstructionBypass );
  m_pcReconstructionBypass = pcReconstructionBypass;

  m_pcControlMngIf  = pcControlMngIf;
  m_pcHighpassFrame = NULL;

  return Err::m_nOK;
}

ErrVal LoopFilter::uninit()
{
  m_pcControlMngIf = NULL;
  return Err::m_nOK;
}

ErrVal LoopFilter::process( SliceHeader& rcSH, IntFrame* pcIntFrame /*, IntYuvPicBuffer* pcHighpassYuvBuffer*/
							              , bool  bAllSliceDone)
{
   bool enhancedLayerFlag =  (rcSH.getLayerId()>0) ; //V032, added for disabling chroma deblocking at enhanced layer
  if( m_pcRCDOSliceHeader )
  {
    m_bRCDO = m_pcRCDOSliceHeader->getSPS().getRCDODeblocking();
  }
  else
  {
    m_bRCDO = rcSH.getSPS().getRCDODeblocking();
  }

  if( pcIntFrame )
  {
		RNOK( pcIntFrame->addFrameFieldBuffer() );
    m_apcIntYuvBuffer[ TOP_FIELD ] = pcIntFrame->getPic( TOP_FIELD )->getFullPelYuvBuffer();
    m_apcIntYuvBuffer[ BOT_FIELD ] = pcIntFrame->getPic( BOT_FIELD )->getFullPelYuvBuffer();
    m_apcIntYuvBuffer[ FRAME     ] = pcIntFrame->getPic( FRAME     )->getFullPelYuvBuffer();
  }
  else
  {
    m_apcIntYuvBuffer[ TOP_FIELD ] = m_apcIntYuvBuffer[ BOT_FIELD ] = m_apcIntYuvBuffer[ FRAME ] = NULL;
  }
  
    

   ROT( NULL == m_pcControlMngIf );

  m_pcRecFrameUnit = const_cast<SliceHeader&>(rcSH).getFrameUnit();

  RNOK( m_pcControlMngIf->initSlice( rcSH, POST_PROCESS ) );  
  m_bVerMixedMode =  m_bHorMixedMode = false;
  const Bool bMbAff = rcSH.isMbAff();

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

    //===== loop over macroblocks =====
    for(UInt uiMbAddress= uiFirstMbInSlice ;uiMbAddress<=uiLastMbInSlice ;)    
    {
		  MbDataAccess* pcMbDataAccess = NULL;
      UInt uiMbX, uiMbY;

      rcSH.getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddress                           );

		  RNOK( m_pcControlMngIf->initMbForFiltering( pcMbDataAccess, uiMbY, uiMbX, bMbAff ) );

		  if ( ! bMbAff && ! m_bRCDO )
		  {
      	RNOK( xFilterMbFast( *pcMbDataAccess, enhancedLayerFlag ) ); //V032 of FSL added for disabling chroma deblocking
		  }
		  else 
		  {
      	RNOK( xFilterMb( *pcMbDataAccess, enhancedLayerFlag)); //V032 of FSL added for disabling chroma deblocking
      }
	    uiMbAddress = rcSH.getFMO()->getNextMBNr(uiMbAddress );    
	  }
  }
  //<- ICU/ETRI DS

  m_apcIntYuvBuffer[ TOP_FIELD ] = 
  m_apcIntYuvBuffer[ BOT_FIELD ] = 
  m_apcIntYuvBuffer[     FRAME ] = NULL;

  return Err::m_nOK;
}

__inline 
ErrVal LoopFilter::xFilterMbFast( const MbDataAccess& rcMbDataAccess, bool enhancedLayerFlag ) //V032 of FSL, disabling chroma deblocking in enh. layer

{
  const DFP& rcDFP      = rcMbDataAccess.getDeblockingFilterParameter();
  const Int iFilterIdc  = rcDFP.getDisableDeblockingFilterIdc();

  ROTRS( iFilterIdc == 1, Err::m_nOK );

  xGetFilterStrengthFast( rcMbDataAccess, iFilterIdc );
  
  if( m_apcIntYuvBuffer[ FRAME ] )
  {
    RNOK( xLumaVerFiltering(   rcMbDataAccess, rcDFP, m_apcIntYuvBuffer[ FRAME ] ) );
    RNOK( xLumaHorFiltering(   rcMbDataAccess, rcDFP, m_apcIntYuvBuffer[ FRAME ] ) );
	//V032 of FSL for disabling chroma deblocking in enh. layer
	if (!enhancedLayerFlag || (enhancedLayerFlag && (iFilterIdc !=3 && iFilterIdc != 4)) ) 
	{ 
		RNOK( xChromaVerFiltering( rcMbDataAccess, rcDFP, m_apcIntYuvBuffer[ FRAME ] ) );
		RNOK( xChromaHorFiltering( rcMbDataAccess, rcDFP, m_apcIntYuvBuffer[ FRAME ] ) );
	}

    return Err::m_nOK;
  }

  YuvPicBuffer* pcYuvBuffer = m_pcRecFrameUnit->getPic( rcMbDataAccess.getSH().getPicType() )->getFullPelYuvBuffer();

	RNOK( xLumaVerFiltering(   rcMbDataAccess, rcDFP, pcYuvBuffer ) );
  RNOK( xLumaHorFiltering(   rcMbDataAccess, rcDFP, pcYuvBuffer ) );
  //V032 of FSL for disabling chroma deblocking in enh. layer
  if (!enhancedLayerFlag || (enhancedLayerFlag && (iFilterIdc !=3 && iFilterIdc != 4)) ) 
  { 
	RNOK( xChromaVerFiltering( rcMbDataAccess, rcDFP, pcYuvBuffer ) );
	RNOK( xChromaHorFiltering( rcMbDataAccess, rcDFP, pcYuvBuffer ) );
  }
  return Err::m_nOK;
}

ErrVal LoopFilter::xGetFilterStrengthFast( const MbDataAccess& rcMbDataAccess, const Int iFilterIdc )
{
  const MbData& rcMbData = rcMbDataAccess.getMbData();

  const Bool bFrame = ( FRAME == rcMbDataAccess.getMbPicType() );
	const Bool b8x8   = rcMbData.isTransformSize8x8();
  const Int iIntraStrength = (bFrame ? 4 : 3);
  const Short sVerMvThr    = (bFrame ? 4 : 2);

  MbMode eMbMode = rcMbData.getMbMode();

  if( eMbMode >= INTRA_4X4 )
  {
    m_aaaucBs[VER][0][0] = m_aaaucBs[VER][0][1] = m_aaaucBs[VER][0][2] = m_aaaucBs[VER][0][3] = 4;
    m_aaaucBs[VER][1][0] = m_aaaucBs[VER][1][1] = m_aaaucBs[VER][1][2] = m_aaaucBs[VER][1][3] = 3;
    m_aaaucBs[VER][2][0] = m_aaaucBs[VER][2][1] = m_aaaucBs[VER][2][2] = m_aaaucBs[VER][2][3] = 3;
    m_aaaucBs[VER][3][0] = m_aaaucBs[VER][3][1] = m_aaaucBs[VER][3][2] = m_aaaucBs[VER][3][3] = 3;

    m_aaaucBs[HOR][0][0] = m_aaaucBs[HOR][1][0] = m_aaaucBs[HOR][2][0] = m_aaaucBs[HOR][3][0] = iIntraStrength;
    m_aaaucBs[HOR][0][1] = m_aaaucBs[HOR][1][1] = m_aaaucBs[HOR][2][1] = m_aaaucBs[HOR][3][1] = 3;
    m_aaaucBs[HOR][0][2] = m_aaaucBs[HOR][1][2] = m_aaaucBs[HOR][2][2] = m_aaaucBs[HOR][3][2] = 3;
    m_aaaucBs[HOR][0][3] = m_aaaucBs[HOR][1][3] = m_aaaucBs[HOR][2][3] = m_aaaucBs[HOR][3][3] = 3;

    if( b8x8 )
    {
      m_aaaucBs[VER][1][0] = m_aaaucBs[VER][1][1] = m_aaaucBs[VER][1][2] = m_aaaucBs[VER][1][3] = 
      m_aaaucBs[VER][3][0] = m_aaaucBs[VER][3][1] = m_aaaucBs[VER][3][2] = m_aaaucBs[VER][3][3] = 
      m_aaaucBs[HOR][0][1] = m_aaaucBs[HOR][1][1] = m_aaaucBs[HOR][2][1] = m_aaaucBs[HOR][3][1] = 
      m_aaaucBs[HOR][0][3] = m_aaaucBs[HOR][1][3] = m_aaaucBs[HOR][2][3] = m_aaaucBs[HOR][3][3] = 0;
    }
  }
  else
  {
    Bool bCheckHorMv = true;
    Bool bCheckVerMv = true;
    switch( eMbMode ) 
    {
    case MODE_16x16:
      bCheckHorMv = false;
      bCheckVerMv = false;
      break;
    case MODE_16x8:
      bCheckVerMv = false;
      break;
    case MODE_8x16:
      bCheckHorMv = false;
      break;
    case MODE_SKIP:
      if( rcMbData.isInterPMb() )
      {
        bCheckHorMv = false;
        bCheckVerMv = false;
      }
      break;
    case MODE_8x8:
	  case MODE_8x8ref0:
      break;
    default:
      {
        AOT(1)        
        break;
      }
    }

    const Bool bCoded = (0 != (rcMbData.getMbCbp() & 0x0f));
    const MbData& rcMbDataLeft  = rcMbDataAccess.getMbDataLeft();
    const MbData& rcMbDataAbove = (rcMbData.getFieldFlag()? rcMbDataAccess.getMbDataAboveAbove():
                                                            rcMbDataAccess.getMbDataAbove());
    {
      Bool bLeftIntra = rcMbDataLeft.isIntra();
      for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
      {
				if( ! b8x8 || (0 == (cIdx.x() & 1)) )
				{
					m_aaaucBs[VER][cIdx.x()][cIdx.y()] = xGetVerFilterStrengthFast( rcMbData, rcMbDataLeft, cIdx, bLeftIntra, bCheckVerMv, bCoded, sVerMvThr );
				}
				else
				{
					m_aaaucBs[VER][cIdx.x()][cIdx.y()] = 0;
				}
      }
    }

    {
      Bool bAboveIntra = rcMbDataAbove.isIntra();
      for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
      {
				if( ! b8x8 || (0 == (cIdx.y() & 1)) )
				{
					m_aaaucBs[HOR][cIdx.x()][cIdx.y()] = xGetHorFilterStrengthFast( rcMbData, rcMbDataAbove, cIdx, bAboveIntra, bCheckHorMv, bCoded, sVerMvThr, iIntraStrength );
				}
				else
				{
					m_aaaucBs[HOR][cIdx.x()][cIdx.y()] = 0;
				}
      }
    }
  }

  Bool bClearAbove = ! rcMbDataAccess.isAboveMbExisting();
  Bool bClearLeft  = ! rcMbDataAccess.isLeftMbExisting();
  if( iFilterIdc == 2 || iFilterIdc == 4 ) //V032 of FSL
  {
    bClearAbove |= ! rcMbDataAccess.isAvailableAbove();
    bClearLeft  |= ! rcMbDataAccess.isAvailableLeft();
  }
  if( bClearAbove )
  {
    m_aaaucBs[HOR][0][0] = m_aaaucBs[HOR][1][0] = m_aaaucBs[HOR][2][0] = m_aaaucBs[HOR][3][0] = 0;
  }

  if( bClearLeft )
  {
    m_aaaucBs[VER][0][0] = m_aaaucBs[VER][0][1] = m_aaaucBs[VER][0][2] = m_aaaucBs[VER][0][3] = 0;
  }
  
  return Err::m_nOK;
}

__inline
UInt LoopFilter::xGetVerFilterStrengthFast( const MbData& rcMbDataCurr,
                                            const MbData& rcMbDataLeft,
                                            LumaIdx       cIdx,
                                            Bool          bLeftIntra,
                                            Bool          bCheckMv,
                                            Bool          bCoded,
                                            const Short  sVerMvThr)
{
  if( cIdx.x() )
  {
    // this is a edge inside of a macroblock
    if( bCoded )
    {
      ROTRS( rcMbDataCurr.is4x4BlkCoded( cIdx                          ), 2 );
      ROTRS( rcMbDataCurr.is4x4BlkCoded( cIdx + CURR_MB_LEFT_NEIGHBOUR ), 2 );
    }

    ROTRS( ! bCheckMv, 0 );

    if( rcMbDataCurr.isInterPMb() )
    {
      const MbMotionData& rcMbMotionData = rcMbDataCurr.getMbMotionData( LIST_0 );
      const LumaIdx cQIdx = cIdx + CURR_MB_LEFT_NEIGHBOUR;

      const RefPic& rcRefPicL0Q = rcMbMotionData.getRefPic( cQIdx );
      const RefPic& rcRefPicL0P = rcMbMotionData.getRefPic( cIdx );

      // different reference pictures
      ROTRS( rcRefPicL0Q.getFrame() != rcRefPicL0P.getFrame(), 1 );

      // check the motion vector distance
      const Mv& cMvQ = rcMbMotionData.getMv( cQIdx );
      const Mv& cMvP = rcMbMotionData.getMv( cIdx );

      ROTRS( cMvP.getAbsHorDiff( cMvQ ) >= 4, 1 );
      ROTRS( cMvP.getAbsVerDiff( cMvQ ) >= sVerMvThr, 1 );
      return 0;
    }
    return xCheckMvDataB  ( rcMbDataCurr, cIdx, rcMbDataCurr, cIdx + CURR_MB_LEFT_NEIGHBOUR, 4, sVerMvThr );
  }

  ROTRS( bLeftIntra,  4 );

  ROTRS( rcMbDataCurr.is4x4BlkCoded( cIdx                          ), 2 );
  ROTRS( rcMbDataLeft.is4x4BlkCoded( cIdx + LEFT_MB_LEFT_NEIGHBOUR ), 2 );

  return xCheckMvDataB  ( rcMbDataCurr, cIdx, rcMbDataLeft, cIdx + LEFT_MB_LEFT_NEIGHBOUR, 4, sVerMvThr );
}

__inline 
UInt LoopFilter::xGetHorFilterStrengthFast( const MbData& rcMbDataCurr,
                                            const MbData& rcMbDataAbove,
                                            LumaIdx       cIdx,
                                            Bool          bAboveIntra,
                                            Bool          bCheckMv,
                                            Bool          bCoded,
                                            const Short   sVerMvThr,
                                            Int           iIntraStrength )
{
  if( cIdx.y() )
  {
    if( bCoded )
    {
      // internal edge
      ROTRS( rcMbDataCurr.is4x4BlkCoded( cIdx                           ), 2 );
      ROTRS( rcMbDataCurr.is4x4BlkCoded( cIdx + CURR_MB_ABOVE_NEIGHBOUR ), 2 );
    }

    ROTRS( ! bCheckMv, 0 );
    if( rcMbDataCurr.isInterPMb() )
    {
      const MbMotionData& rcMbMotionData = rcMbDataCurr.getMbMotionData( LIST_0 );
      const LumaIdx cQIdx = cIdx + CURR_MB_ABOVE_NEIGHBOUR;

      const RefPic& rcRefPicL0Q = rcMbMotionData.getRefPic( cQIdx );
      const RefPic& rcRefPicL0P = rcMbMotionData.getRefPic( cIdx );

      // different reference pictures
      ROTRS( rcRefPicL0Q.getFrame() != rcRefPicL0P.getFrame(), 1 );


      // check the motion vector distance
      const Mv& cMvQ = rcMbMotionData.getMv( cQIdx );
      const Mv& cMvP = rcMbMotionData.getMv( cIdx );

      ROTRS( cMvP.getAbsHorDiff( cMvQ ) >= 4, 1 );
      ROTRS( cMvP.getAbsVerDiff( cMvQ ) >= sVerMvThr, 1 );
      return 0;
    }
    return xCheckMvDataB  ( rcMbDataCurr, cIdx, rcMbDataCurr, cIdx + CURR_MB_ABOVE_NEIGHBOUR, 4, sVerMvThr );
  }

  ROTRS( bAboveIntra , iIntraStrength );

  ROTRS( rcMbDataCurr. is4x4BlkCoded( cIdx                            ), 2 );
  ROTRS( rcMbDataAbove.is4x4BlkCoded( cIdx + ABOVE_MB_ABOVE_NEIGHBOUR ), 2 );

  return xCheckMvDataB  ( rcMbDataCurr, cIdx, rcMbDataAbove, cIdx + ABOVE_MB_ABOVE_NEIGHBOUR, 4, sVerMvThr );
}


__inline ErrVal LoopFilter::xFilterMb( const MbDataAccess& rcMbDataAccess, bool enhancedLayerFlag) //V032 of FSL
{
  if( m_bRCDO )
  {
    RNOK( xFilterMb_RCDO( rcMbDataAccess ) );
    return Err::m_nOK;
  }

  const DFP& rcDFP      = rcMbDataAccess.getDeblockingFilterParameter( m_eLFMode & LFM_NO_INTER_FILTER );
  Int iFilterIdc  = rcDFP.getDisableDeblockingFilterIdc();

  ROTRS( (m_eLFMode & LFM_NO_INTER_FILTER) && ! rcMbDataAccess.getMbData().isIntra(), Err::m_nOK );

  ROTRS( iFilterIdc == 1, Err::m_nOK );

  Bool b8x8 = rcMbDataAccess.getMbData().isTransformSize8x8();

  Bool bFieldFlag  = rcMbDataAccess.getMbData().getFieldFlag();
  m_bVerMixedMode  = (bFieldFlag != rcMbDataAccess.getMbDataLeft().getFieldFlag());
  m_bHorMixedMode  = (bFieldFlag?(bFieldFlag != rcMbDataAccess.getMbDataAboveAbove().getFieldFlag()):
                     (bFieldFlag != rcMbDataAccess.getMbDataAbove().getFieldFlag()));
  Bool bCurrFrame  = (FRAME      == rcMbDataAccess.getMbPicType());

  m_bAddEdge = true;
  if( m_bHorMixedMode && bCurrFrame )
  {
    for( B4x4Idx cIdx; cIdx.b4x4() < 4; cIdx++ )
    {
      m_aucBsHorTop[cIdx.x()] = xGetHorFilterStrength( rcMbDataAccess, cIdx, iFilterIdc );
    }
  }
  if( m_bVerMixedMode )
  {
    for( B4x4Idx cIdx; cIdx.b4x4() < 16; cIdx = B4x4Idx( cIdx + 4 ) )
    {
      m_aucBsVerBot[cIdx.y()] = xGetVerFilterStrength( rcMbDataAccess, cIdx, iFilterIdc );
    }
  }
  m_bAddEdge = false;

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

  m_bHorMixedMode  = m_bHorMixedMode && bCurrFrame;

  if( m_apcIntYuvBuffer[ FRAME ] )
  {
    const PicType ePicType = rcMbDataAccess.getMbPicType();
    RNOK( xLumaVerFiltering(   rcMbDataAccess, rcDFP, m_apcIntYuvBuffer[ ePicType ] ) );
    RNOK( xLumaHorFiltering(   rcMbDataAccess, rcDFP, m_apcIntYuvBuffer[ ePicType ] ) );
//V032 of FSL for disabling chroma deblocking in enh. layer
	if (!enhancedLayerFlag || (enhancedLayerFlag && (iFilterIdc !=3 && iFilterIdc != 4)) ) 
	{ 
		RNOK( xChromaVerFiltering( rcMbDataAccess, rcDFP, m_apcIntYuvBuffer[ ePicType ] ) );
		RNOK( xChromaHorFiltering( rcMbDataAccess, rcDFP, m_apcIntYuvBuffer[ ePicType ] ) );
	}
    return Err::m_nOK;
  }

  const SliceHeader& rcSH = rcMbDataAccess.getSH();
  const PicType ePicType = ( rcSH.getFieldPicFlag() ) ? rcSH.getPicType() : rcMbDataAccess.getMbPicType();
  YuvPicBuffer* pcYuvBuffer = m_pcRecFrameUnit->getPic( ePicType )->getFullPelYuvBuffer();

  RNOK( xLumaVerFiltering(   rcMbDataAccess, rcDFP, pcYuvBuffer ) );
  RNOK( xLumaHorFiltering(   rcMbDataAccess, rcDFP, pcYuvBuffer ) );
  //V032 of FSL for disabling chroma deblocking in enh. layer
  if (!enhancedLayerFlag || (enhancedLayerFlag && (iFilterIdc !=3 && iFilterIdc != 4)) ) 
  { 
     RNOK( xChromaVerFiltering( rcMbDataAccess, rcDFP, pcYuvBuffer ) );
     RNOK( xChromaHorFiltering( rcMbDataAccess, rcDFP, pcYuvBuffer ) );
  }
  return Err::m_nOK;
}


__inline ErrVal LoopFilter::xFilterMb_RCDO( const MbDataAccess& rcMbDataAccess )
{
  const DFP&  rcDFP       = rcMbDataAccess.getDeblockingFilterParameter( m_eLFMode & LFM_NO_INTER_FILTER );
  Int         iFilterIdc  = rcDFP.getDisableDeblockingFilterIdc();

  ROTRS( (m_eLFMode & LFM_NO_INTER_FILTER) && ! rcMbDataAccess.getMbData().isIntra(), Err::m_nOK );
  ROTRS( iFilterIdc == 1, Err::m_nOK );

  Bool  bFieldFlag  = rcMbDataAccess.getMbData().getFieldFlag();
  m_bVerMixedMode   = ( bFieldFlag != rcMbDataAccess.getMbDataLeft().getFieldFlag() );
  m_bHorMixedMode   = ( bFieldFlag ? ( bFieldFlag != rcMbDataAccess.getMbDataAboveAbove().getFieldFlag() ) 
                                   : ( bFieldFlag != rcMbDataAccess.getMbDataAbove     ().getFieldFlag() ) );

  if( m_apcIntYuvBuffer[ FRAME ] )
  {
    const PicType ePicType = rcMbDataAccess.getMbPicType();
    RNOK( xLumaVerFiltering_H241RCDO(   rcMbDataAccess, rcDFP, m_apcIntYuvBuffer[ ePicType ], &rcMbDataAccess, NULL, NULL ) );
    RNOK( xLumaHorFiltering_H241RCDO(   rcMbDataAccess, rcDFP, m_apcIntYuvBuffer[ ePicType ], &rcMbDataAccess, NULL, NULL ) );
    RNOK( xChromaVerFiltering_H241RCDO( rcMbDataAccess, rcDFP, m_apcIntYuvBuffer[ ePicType ] ) );
    RNOK( xChromaHorFiltering_H241RCDO( rcMbDataAccess, rcDFP, m_apcIntYuvBuffer[ ePicType ] ) );
    return Err::m_nOK;
  }

  const SliceHeader&  rcSliceHeader = rcMbDataAccess.getSH();
  const PicType       ePicType      = ( rcSliceHeader.getFieldPicFlag() ? rcSliceHeader.getPicType() : rcMbDataAccess.getMbPicType() );
  YuvPicBuffer*       pcYuvBuffer   = m_pcRecFrameUnit->getPic( ePicType )->getFullPelYuvBuffer();
  RNOK( xLumaVerFiltering_H241RCDO(   rcMbDataAccess, rcDFP, pcYuvBuffer ) );
  RNOK( xLumaHorFiltering_H241RCDO(   rcMbDataAccess, rcDFP, pcYuvBuffer ) );
  RNOK( xChromaVerFiltering_H241RCDO( rcMbDataAccess, rcDFP, pcYuvBuffer ) );
  RNOK( xChromaHorFiltering_H241RCDO( rcMbDataAccess, rcDFP, pcYuvBuffer ) );
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


#define Clip1Y(a)          ((a)>255 ? 255 :((a)<0?0:(a)))
#define Clip1C(a)          ((a)>255 ? 255 :((a)<0?0:(a)))
#define Clip3(mn,mx,val)   (((val)<(mn))?(mn):(((val)>(mx))?(mx):(val)))


__inline Void LoopFilter::xFilter_H241RCDO( Pel* pFlt, const Int& iOffset, const Int& iIndexA, const Int& iIndexB, const Int& blk_pos, const Bool& bLum )
{
  const Int iTc   = g_aucTcTab_H241RCDO   [ iIndexA ];
  Int       P2    = pFlt[-3*iOffset];
  Int       P1    = pFlt[-2*iOffset];
  Int       P0    = pFlt[-1*iOffset];
  Int       Q0    = pFlt[         0];
  Int       Q1    = pFlt[ 1*iOffset];
  Int       Q2    = pFlt[ 2*iOffset];
  Int       iDelta;

  if( bLum )
  {
    if( blk_pos ) // not on macroblock edge
    {
      iDelta = Clip3( -iTc, iTc, ( ( Q0 + ( ( P2 + Q1 ) >> 1 ) ) >> 1 ) - ( ( P0 + ( ( Q2 + P1 ) >> 1 ) ) >> 1 ) );
    }
    else
    {
      iDelta = Clip3( -iTc, iTc, ( ( Q0 + ( Q1 >> 1 ) ) >> 1 ) - ( ( P0 + ( Q2 >> 1 ) ) >> 1 ) );
    }

    pFlt[-2*iOffset] = Clip1Y(P1 +(iDelta/2));
    pFlt[-1*iOffset] = Clip1Y(P0 + iDelta    );
    pFlt[         0] = Clip1Y(Q0 - iDelta    );
    pFlt[ 1*iOffset] = Clip1Y(Q1 -(iDelta/2));
  }
  else
  {
    iDelta           = Clip3 ( -iTc, iTc, ( ( ( ( Q0 - P0 ) << 2 ) + P1 - Q1 + 4 ) >> 3 ) );
    pFlt[-iOffset]   = Clip1C( P0 + iDelta );
    pFlt[       0]   = Clip1C( Q0 - iDelta );
  }
}


__inline Void LoopFilter::xFilter_H241RCDO( XPel* pFlt, const Int& iOffset, const Int& iIndexA, const Int& iIndexB, const Int& blk_pos, const Bool& bLum )
{
  const Int iTc   = g_aucTcTab_H241RCDO   [ iIndexA ];
  Int       P2    = pFlt[-3*iOffset];
  Int       P1    = pFlt[-2*iOffset];
  Int       P0    = pFlt[-1*iOffset];
  Int       Q0    = pFlt[         0];
  Int       Q1    = pFlt[ 1*iOffset];
  Int       Q2    = pFlt[ 2*iOffset];
  Int       iDelta;

  if( bLum )
  {
    if( blk_pos ) // not on macroblock edge
    {
      iDelta = Clip3( -iTc, iTc, ( ( Q0 + ( ( P2 + Q1 ) >> 1 ) ) >> 1 ) - ( ( P0 + ( ( Q2 + P1 ) >> 1 ) ) >> 1 ) );
    }
    else
    {
      iDelta = Clip3( -iTc, iTc, ( ( Q0 + ( Q1 >> 1 ) ) >> 1 ) - ( ( P0 + ( Q2 >> 1 ) ) >> 1 ) );
    }

    pFlt[-2*iOffset] = Clip1Y(P1 +(iDelta/2));
    pFlt[-1*iOffset] = Clip1Y(P0 + iDelta    );
    pFlt[         0] = Clip1Y(Q0 - iDelta    );
    pFlt[ 1*iOffset] = Clip1Y(Q1 -(iDelta/2));
  }
  else
  {
    iDelta           = Clip3 ( -iTc, iTc, ( ( ( ( Q0 - P0 ) << 2 ) + P1 - Q1 + 4 ) >> 3 ) );
    pFlt[-iOffset]   = Clip1C( P0 + iDelta );
    pFlt[       0]   = Clip1C( Q0 - iDelta );
  }
}


__inline ErrVal LoopFilter::xLumaVerFiltering_H241RCDO( const MbDataAccess& rcMbDataAccess, const DFP& rcDFP, YuvPicBuffer* pcYuvBuffer )
{
  Int   iCurrQp = rcMbDataAccess.getMbDataCurr().getQpLF();
  Int   iStride = pcYuvBuffer->getLStride();
  Pel*  pPelLum = pcYuvBuffer->getMbLumAddr();

  Bool  b8x8            = rcMbDataAccess.getMbData().isTransformSize8x8();
  Bool  filterBlockEdge = false;
  Bool  bFilterTopOnly  = false;
  Bool  bFilterBotOnly  = false;
  Short sHorMvThr       = 4;
  Short sVerMvThr       = ( FRAME == rcMbDataAccess.getMbPicType() ? 4 : 2 );

  Int   CheckMVandRefVal;
  Int   xBlk, yBlk, iLeftQp = 0, iLeftQpTop = 0, iLeftQpBot = 0;



  //===== LEFT MACROBLOCK EDGE =====
  B4x4Idx cIdx=0;
  for( yBlk = 0; yBlk <= 8; yBlk+=8 )
  {
    filterBlockEdge       = rcMbDataAccess.isLeftMbExisting();
    bFilterTopOnly        = false;
    bFilterBotOnly        = false;
    if( filterBlockEdge )
    {
      Bool  bLeftIntra    = rcMbDataAccess.getMbDataLeft().isIntra();
      iLeftQp             = rcMbDataAccess.getMbDataLeft().getQpLF();
      if( m_bVerMixedMode )
      {
        if      ( rcMbDataAccess.getMbData().getFieldFlag() && yBlk == 0 )
        {
          bLeftIntra      = ( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataLeft().isIntra() : rcMbDataAccess.getMbDataAboveLeft().isIntra() );
          iLeftQp         = ( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataLeft().getQpLF() : rcMbDataAccess.getMbDataAboveLeft().getQpLF() );
        }
        else if ( rcMbDataAccess.getMbData().getFieldFlag() )
        {
          bLeftIntra      = ( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataBelowLeft().isIntra() : rcMbDataAccess.getMbDataLeft().isIntra() );
          iLeftQp         = ( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataBelowLeft().getQpLF() : rcMbDataAccess.getMbDataLeft().getQpLF() );
        }
        else // current pair is frame, left pair is field
        {
          Bool bLI_Top    = ( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataLeft().isIntra() : rcMbDataAccess.getMbDataAboveLeft().isIntra() );
          Bool bLI_Bot    = ( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataBelowLeft().isIntra() : rcMbDataAccess.getMbDataLeft().isIntra() );
          iLeftQpTop      = ( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataLeft().getQpLF() : rcMbDataAccess.getMbDataAboveLeft().getQpLF() );
          iLeftQpBot      = ( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataBelowLeft().getQpLF() : rcMbDataAccess.getMbDataLeft().getQpLF() );
          if( ( bLI_Top ^ bLI_Bot ) && LFM_DEFAULT_FILTER != m_eLFMode )
          {
            bLeftIntra    = true;
            bFilterTopOnly= bLI_Top;
            bFilterBotOnly= bLI_Bot;
          }
          else
          {
            bLeftIntra    = bLI_Top || bLI_Bot;
          }
        }
      }
      filterBlockEdge     = ( LFM_DEFAULT_FILTER == m_eLFMode || bLeftIntra );
    }
    if( filterBlockEdge )
    {
      filterBlockEdge     = ! ( rcDFP.getDisableDeblockingFilterIdc() == 1 ||
                              ( rcDFP.getDisableDeblockingFilterIdc() == 2 && ! rcMbDataAccess.isAvailableLeft() ) );
    }
    if( ! m_bVerMixedMode )
    {
      if( filterBlockEdge )
      {
        Int iQp           = ( iLeftQp + iCurrQp + 1) >> 1;
        Int iIndexB       = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);
        Int iStridex2     = 2*iStride;
        Int iStridex5     = 5*iStride;
        Int iD            = ( abs( (Int)pPelLum[iStridex2-2] -   (Int)pPelLum[iStridex2-1]                                    )
                            + abs( (Int)pPelLum[iStridex2  ] - ( (Int)pPelLum[iStridex2+1] << 1 ) + (Int)pPelLum[iStridex2+2] )
                            + abs( (Int)pPelLum[iStridex5-2] -   (Int)pPelLum[iStridex5-1]                                    )
                            + abs( (Int)pPelLum[iStridex5  ] - ( (Int)pPelLum[iStridex5+1] << 1 ) + (Int)pPelLum[iStridex5+2] ) );
        filterBlockEdge   = ( iD < g_aucBetaTab_H241RCDO [ iIndexB ] );
      }
      if( filterBlockEdge )
      {
        filterBlockEdge   = ( rcMbDataAccess.getMbDataCurr().isIntra() || 
                              rcMbDataAccess.getMbDataLeft().isIntra() );
        if( ! filterBlockEdge && m_pcHighpassYuvBuffer )
        {
          filterBlockEdge = ( m_pcHighpassYuvBuffer->isCurr4x4BlkNotZero( cIdx ) ||
                              m_pcHighpassYuvBuffer->isCurr4x4BlkNotZero( cIdx + VER_OFFSET_H241RCDO ) ||
                              m_pcHighpassYuvBuffer->isLeft4x4BlkNotZero( cIdx + LEFT_MB_LEFT_NEIGHBOUR ) ||
                              m_pcHighpassYuvBuffer->isLeft4x4BlkNotZero( cIdx + LEFT_MB_LEFT_NEIGHBOUR + VER_OFFSET_H241RCDO ) );
        }
        if( ! filterBlockEdge )
        {
          filterBlockEdge = ( rcMbDataAccess.getMbDataCurr().is4x4BlkCoded( cIdx ) ||
                              rcMbDataAccess.getMbDataCurr().is4x4BlkCoded( cIdx + VER_OFFSET_H241RCDO ) ||
                              rcMbDataAccess.getMbDataLeft().is4x4BlkCoded( cIdx + LEFT_MB_LEFT_NEIGHBOUR ) ||
                              rcMbDataAccess.getMbDataLeft().is4x4BlkCoded( cIdx + LEFT_MB_LEFT_NEIGHBOUR + VER_OFFSET_H241RCDO ) );
        }
        if( ! filterBlockEdge )
        {
          if( rcMbDataAccess.getMbDataCurr().isInterPMb() && rcMbDataAccess.getMbDataLeft().isInterPMb() )
          {
            CheckMVandRefVal= xCheckMvDataP( rcMbDataAccess.getMbDataCurr(), cIdx, rcMbDataAccess.getMbDataLeft(), cIdx + LEFT_MB_LEFT_NEIGHBOUR, sHorMvThr, sVerMvThr );
          }
          else
          {
            CheckMVandRefVal= xCheckMvDataB( rcMbDataAccess.getMbDataCurr(), cIdx, rcMbDataAccess.getMbDataLeft(), cIdx + LEFT_MB_LEFT_NEIGHBOUR, sHorMvThr, sVerMvThr );
          }
          filterBlockEdge = ( CheckMVandRefVal > 0 );
        }
      }
    }

    if( filterBlockEdge )
    {
      if( m_bVerMixedMode && ! rcMbDataAccess.getMbData().getFieldFlag() )
      {
        if( ! bFilterBotOnly )
        {
          Int iQp     = ( iLeftQpTop + iCurrQp + 1) >> 1;
          Int iIndexA = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
          Int iIndexB = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);
          xFilter_H241RCDO( pPelLum,           1, iIndexA, iIndexB, 0, true );
          xFilter_H241RCDO( pPelLum+2*iStride, 1, iIndexA, iIndexB, 0, true );
          xFilter_H241RCDO( pPelLum+4*iStride, 1, iIndexA, iIndexB, 0, true );
          xFilter_H241RCDO( pPelLum+6*iStride, 1, iIndexA, iIndexB, 0, true );
        }
        if( ! bFilterTopOnly )
        {
          Int iQp     = ( iLeftQpBot + iCurrQp + 1) >> 1;
          Int iIndexA = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
          Int iIndexB = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);
          xFilter_H241RCDO( pPelLum+  iStride, 1, iIndexA, iIndexB, 0, true );
          xFilter_H241RCDO( pPelLum+3*iStride, 1, iIndexA, iIndexB, 0, true );
          xFilter_H241RCDO( pPelLum+5*iStride, 1, iIndexA, iIndexB, 0, true );
          xFilter_H241RCDO( pPelLum+7*iStride, 1, iIndexA, iIndexB, 0, true );
        }
      }
      else
      {
        Int iQp     = ( iLeftQp + iCurrQp + 1) >> 1;
        Int iIndexA = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
        Int iIndexB = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);

        xFilter_H241RCDO( pPelLum,           1, iIndexA, iIndexB, 0, true );
        xFilter_H241RCDO( pPelLum+  iStride, 1, iIndexA, iIndexB, 0, true );
        xFilter_H241RCDO( pPelLum+2*iStride, 1, iIndexA, iIndexB, 0, true );
        xFilter_H241RCDO( pPelLum+3*iStride, 1, iIndexA, iIndexB, 0, true );
        xFilter_H241RCDO( pPelLum+4*iStride, 1, iIndexA, iIndexB, 0, true );
        xFilter_H241RCDO( pPelLum+5*iStride, 1, iIndexA, iIndexB, 0, true );
        xFilter_H241RCDO( pPelLum+6*iStride, 1, iIndexA, iIndexB, 0, true );
        xFilter_H241RCDO( pPelLum+7*iStride, 1, iIndexA, iIndexB, 0, true );
      }
    }

    pPelLum += 8*iStride; //for the next 8x8 vertical
    if( yBlk == 0 )
    {
      cIdx = cIdx + 8;
    }
  } //end of the loop


  //===== INNER MACROBLOCK EDGES =====
  pPelLum  -= 16*iStride-4;
  Int iIndexA   = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iCurrQp, 0, 51);
  Int iIndexB   = gClipMinMax( rcDFP.getSliceBetaOffset()    + iCurrQp, 0, 51);
  for( xBlk = 1; xBlk < 4; xBlk++ )
  {
    cIdx = cIdx - 7;
    for(  yBlk = 0; yBlk <= 8; yBlk += 8 )
    {
      filterBlockEdge     = (  xBlk == 2 || ! b8x8 );
      if( filterBlockEdge )
      {
        filterBlockEdge   = ! ( rcDFP.getDisableDeblockingFilterIdc() == 1 );
      }
      if( filterBlockEdge )
      {
        Int iStridex2     = 2*iStride;
        Int iStridex5     = 5*iStride;
        Int iD            = ( abs( (Int)pPelLum[iStridex2-3] - ( (Int)pPelLum[iStridex2-2] << 1 ) + (Int)pPelLum[iStridex2-1] )
                            + abs( (Int)pPelLum[iStridex2  ] - ( (Int)pPelLum[iStridex2+1] << 1 ) + (Int)pPelLum[iStridex2+2] )
                            + abs( (Int)pPelLum[iStridex5-3] - ( (Int)pPelLum[iStridex5-2] << 1 ) + (Int)pPelLum[iStridex5-1] )
                            + abs( (Int)pPelLum[iStridex5  ] - ( (Int)pPelLum[iStridex5+1] << 1 ) + (Int)pPelLum[iStridex5+2] ) );
        filterBlockEdge   = ( iD < g_aucBetaTab_H241RCDO [ iIndexB ] );
      }
      if( filterBlockEdge )
      {
        filterBlockEdge   = rcMbDataAccess.getMbDataCurr().isIntra();

        if( ! filterBlockEdge &&  m_pcHighpassYuvBuffer )
        {
          filterBlockEdge = ( m_pcHighpassYuvBuffer->isCurr4x4BlkNotZero( cIdx ) ||
                              m_pcHighpassYuvBuffer->isCurr4x4BlkNotZero( cIdx + VER_OFFSET_H241RCDO ) ||
                              m_pcHighpassYuvBuffer->isCurr4x4BlkNotZero( cIdx + CURR_MB_LEFT_NEIGHBOUR ) ||
                              m_pcHighpassYuvBuffer->isCurr4x4BlkNotZero( cIdx + CURR_MB_LEFT_NEIGHBOUR + VER_OFFSET_H241RCDO ) );
        }
        if( ! filterBlockEdge )
        {
          filterBlockEdge = ( rcMbDataAccess.getMbDataCurr().is4x4BlkCoded( cIdx ) ||
                              rcMbDataAccess.getMbDataCurr().is4x4BlkCoded( cIdx + VER_OFFSET_H241RCDO ) ||
                              rcMbDataAccess.getMbDataCurr().is4x4BlkCoded( cIdx + CURR_MB_LEFT_NEIGHBOUR ) ||
                              rcMbDataAccess.getMbDataCurr().is4x4BlkCoded( cIdx + CURR_MB_LEFT_NEIGHBOUR + VER_OFFSET_H241RCDO ) );
        }
        if( ! filterBlockEdge )
        {
          if( rcMbDataAccess.getMbDataCurr().isInterPMb() ) 
          {
            CheckMVandRefVal= xCheckMvDataP( rcMbDataAccess.getMbDataCurr(), cIdx, rcMbDataAccess.getMbDataCurr(), cIdx+CURR_MB_LEFT_NEIGHBOUR, sHorMvThr, sVerMvThr );
          }
          else
          {
            CheckMVandRefVal= xCheckMvDataB( rcMbDataAccess.getMbDataCurr(), cIdx, rcMbDataAccess.getMbDataCurr(), cIdx+CURR_MB_LEFT_NEIGHBOUR, sHorMvThr, sVerMvThr );
          }
          filterBlockEdge = ( CheckMVandRefVal > 0 );
        }
      }

      if( filterBlockEdge )
      {
        xFilter_H241RCDO( pPelLum,           1, iIndexA, iIndexB, 1, true );
        xFilter_H241RCDO( pPelLum+  iStride, 1, iIndexA, iIndexB, 1, true );
        xFilter_H241RCDO( pPelLum+2*iStride, 1, iIndexA, iIndexB, 1, true );
        xFilter_H241RCDO( pPelLum+3*iStride, 1, iIndexA, iIndexB, 1, true );
        xFilter_H241RCDO( pPelLum+4*iStride, 1, iIndexA, iIndexB, 1, true );
        xFilter_H241RCDO( pPelLum+5*iStride, 1, iIndexA, iIndexB, 1, true );
        xFilter_H241RCDO( pPelLum+6*iStride, 1, iIndexA, iIndexB, 1, true );
        xFilter_H241RCDO( pPelLum+7*iStride, 1, iIndexA, iIndexB, 1, true );
      }

      pPelLum += 8*iStride; //for the next 8x8 vertical
      if( yBlk == 0 )
      {
        cIdx = cIdx + 8;
      }
    } //end of the 1st loop

    pPelLum -= 16*iStride-4;  //back to the original position
  } //end of the 2nd loop

  return Err::m_nOK;
}


__inline ErrVal LoopFilter::xChromaVerFiltering_H241RCDO ( const MbDataAccess& rcMbDataAccess, const DFP& rcDFP, YuvPicBuffer* pcYuvBuffer )
{
  Int   iCurrQp = rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.getMbDataCurr().getQpLF() );
  Int   iStride = pcYuvBuffer->getCStride();
  Pel*  pPelCb  = pcYuvBuffer->getMbCbAddr();
  Pel*  pPelCr  = pcYuvBuffer->getMbCrAddr();
  Bool  filterBlockEdge, bFilterTopOnly, bFilterBotOnly;
  Int   yBlk, iLeftQp = 0, iLeftQpTop = 0, iLeftQpBot = 0;

  //===== filtering of left macroblock edge =====
  for( yBlk = 0; yBlk <= 4; yBlk+=4 )
  {
    filterBlockEdge   = rcMbDataAccess.isLeftMbExisting();
    bFilterTopOnly    = false;
    bFilterBotOnly    = false;
    if( filterBlockEdge )
    {
      Bool bLeftIntra = rcMbDataAccess.getMbDataLeft().isIntra();
      iLeftQp         = rcMbDataAccess.getMbDataLeft().getQpLF();
      if( m_bVerMixedMode )
      {
        if      ( rcMbDataAccess.getMbData().getFieldFlag() && yBlk == 0 )
        {
          bLeftIntra      = ( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataLeft().isIntra() : rcMbDataAccess.getMbDataAboveLeft().isIntra() );
          iLeftQp         = ( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataLeft().getQpLF() : rcMbDataAccess.getMbDataAboveLeft().getQpLF() );
        }
        else if ( rcMbDataAccess.getMbData().getFieldFlag() )
        {
          bLeftIntra      = ( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataBelowLeft().isIntra() : rcMbDataAccess.getMbDataLeft().isIntra() );
          iLeftQp         = ( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataBelowLeft().getQpLF() : rcMbDataAccess.getMbDataLeft().getQpLF() );
        }
        else // current pair is frame, left pair is field
        {
          Bool bLI_Top    = ( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataLeft().isIntra() : rcMbDataAccess.getMbDataAboveLeft().isIntra() );
          Bool bLI_Bot    = ( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataBelowLeft().isIntra() : rcMbDataAccess.getMbDataLeft().isIntra() );
          iLeftQpTop      = ( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataLeft().getQpLF() : rcMbDataAccess.getMbDataAboveLeft().getQpLF() );
          iLeftQpBot      = ( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataBelowLeft().getQpLF() : rcMbDataAccess.getMbDataLeft().getQpLF() );
          if( ( bLI_Top ^ bLI_Bot ) && LFM_DEFAULT_FILTER != m_eLFMode )
          {
            bLeftIntra    = true;
            bFilterTopOnly= bLI_Top;
            bFilterBotOnly= bLI_Bot;
          }
          else
          {
            bLeftIntra    = bLI_Top || bLI_Bot;
          }
        }
      }
      filterBlockEdge = ( LFM_DEFAULT_FILTER == m_eLFMode || bLeftIntra );
    }
    if( filterBlockEdge )
    {
      filterBlockEdge = ! ( rcDFP.getDisableDeblockingFilterIdc() == 1 ||
                          ( rcDFP.getDisableDeblockingFilterIdc() == 2 && ! rcMbDataAccess.isAvailableLeft() ) );
    }
    if( filterBlockEdge && ! m_bVerMixedMode )
    {
      filterBlockEdge = ( rcMbDataAccess.getMbDataCurr().isIntra() || rcMbDataAccess.getMbDataLeft().isIntra() );
    }

    if( filterBlockEdge )
    {
      if( m_bVerMixedMode && ! rcMbDataAccess.getMbData().getFieldFlag() )
      {
        if( ! bFilterBotOnly )
        {
          Int   iLQp    = rcMbDataAccess.getSH().getChromaQp( iLeftQpTop );
          Int   iQp     = ( iCurrQp + iLQp + 1 ) >> 1;
          Int   iIndexA = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
          Int   iIndexB = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);
          xFilter_H241RCDO( pPelCb,           1, iIndexA, iIndexB, 0, false );
          xFilter_H241RCDO( pPelCb+2*iStride, 1, iIndexA, iIndexB, 0, false );
          xFilter_H241RCDO( pPelCr,           1, iIndexA, iIndexB, 0, false );
          xFilter_H241RCDO( pPelCr+2*iStride, 1, iIndexA, iIndexB, 0, false );
        }
        if( ! bFilterTopOnly )
        {
          Int   iLQp    = rcMbDataAccess.getSH().getChromaQp( iLeftQpBot );
          Int   iQp     = ( iCurrQp + iLQp + 1 ) >> 1;
          Int   iIndexA = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
          Int   iIndexB = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);
          xFilter_H241RCDO( pPelCb+  iStride, 1, iIndexA, iIndexB, 0, false );
          xFilter_H241RCDO( pPelCb+3*iStride, 1, iIndexA, iIndexB, 0, false );
          xFilter_H241RCDO( pPelCr+  iStride, 1, iIndexA, iIndexB, 0, false );
          xFilter_H241RCDO( pPelCr+3*iStride, 1, iIndexA, iIndexB, 0, false );
        }
      }
      else
      {
        Int   iLQp    = rcMbDataAccess.getSH().getChromaQp( iLeftQp );
        Int   iQp     = ( iCurrQp + iLQp + 1 ) >> 1;
        Int   iIndexA = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
        Int   iIndexB = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);
        xFilter_H241RCDO( pPelCb,           1, iIndexA, iIndexB, 0, false );
        xFilter_H241RCDO( pPelCb+  iStride, 1, iIndexA, iIndexB, 0, false );
        xFilter_H241RCDO( pPelCb+2*iStride, 1, iIndexA, iIndexB, 0, false );
        xFilter_H241RCDO( pPelCb+3*iStride, 1, iIndexA, iIndexB, 0, false );
        xFilter_H241RCDO( pPelCr,           1, iIndexA, iIndexB, 0, false );
        xFilter_H241RCDO( pPelCr+  iStride, 1, iIndexA, iIndexB, 0, false );
        xFilter_H241RCDO( pPelCr+2*iStride, 1, iIndexA, iIndexB, 0, false );
        xFilter_H241RCDO( pPelCr+3*iStride, 1, iIndexA, iIndexB, 0, false );
      }
    }

    pPelCb += 4*iStride; 
    pPelCr += 4*iStride; 
  }

  //Inside of the macroblock
  pPelCb -= 8*iStride-4;
  pPelCr -= 8*iStride-4;
  Int iIndexA = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iCurrQp, 0, 51);
  Int iIndexB = gClipMinMax( rcDFP.getSliceBetaOffset()    + iCurrQp, 0, 51);
  for( yBlk = 0; yBlk <= 4; yBlk += 4 )
  {
    filterBlockEdge   = ! ( rcDFP.getDisableDeblockingFilterIdc() == 1 );
    if( filterBlockEdge )
    {
      filterBlockEdge = rcMbDataAccess.getMbDataCurr().isIntra();
    }

    if( filterBlockEdge )
    {
      xFilter_H241RCDO( pPelCb,           1, iIndexA, iIndexB, 1, false );
      xFilter_H241RCDO( pPelCb+  iStride, 1, iIndexA, iIndexB, 1, false );
      xFilter_H241RCDO( pPelCb+2*iStride, 1, iIndexA, iIndexB, 1, false );
      xFilter_H241RCDO( pPelCb+3*iStride, 1, iIndexA, iIndexB, 1, false );
      xFilter_H241RCDO( pPelCr,           1, iIndexA, iIndexB, 1, false );
      xFilter_H241RCDO( pPelCr+  iStride, 1, iIndexA, iIndexB, 1, false );
      xFilter_H241RCDO( pPelCr+2*iStride, 1, iIndexA, iIndexB, 1, false );
      xFilter_H241RCDO( pPelCr+3*iStride, 1, iIndexA, iIndexB, 1, false );
    }

    pPelCb += 4*iStride; //for the next 4x4 vertical
    pPelCr += 4*iStride; //for the next 4x4 vertical
  }

  return Err::m_nOK;
}


__inline ErrVal LoopFilter::xLumaVerFiltering_H241RCDO( const MbDataAccess& rcMbDataAccess, 
                                                        const DFP&          rcDFP, 
                                                        IntYuvPicBuffer*    pcYuvBuffer, 
                                                        const MbDataAccess* pcMbDataAccessMot, //HZL added
                                                        RefFrameList*       pcRefFrameList0,   //HZL added
                                                        RefFrameList*       pcRefFrameList1 )  //HZL added													   
{
  Int   iCurrQp = rcMbDataAccess.getMbDataCurr().getQpLF();
  Int   iStride = pcYuvBuffer->getLStride();
  XPel* pPelLum = pcYuvBuffer->getMbLumAddr();

  Bool  b8x8            = rcMbDataAccess.getMbData().isTransformSize8x8();
  Bool  filterBlockEdge = false;
  Bool  bFilterTopOnly  = false;
  Bool  bFilterBotOnly  = false;
  Short sHorMvThr       = 4;
  Short sVerMvThr       = ( FRAME == rcMbDataAccess.getMbPicType() ? 4 : 2 );

  Int   CheckMVandRefVal;
  Int   xBlk, yBlk, iLeftQp = 0, iLeftQpTop = 0, iLeftQpBot = 0;

  B4x4Idx cIdx=0;
  for( yBlk = 0; yBlk <= 8; yBlk+=8 ) //On the left edge of a macroblock
  {
    filterBlockEdge     = rcMbDataAccess.isLeftMbExisting();
    bFilterTopOnly        = false;
    bFilterBotOnly        = false;
    if( filterBlockEdge )
    {
      Bool  bLeftIntra  = rcMbDataAccess.getMbDataLeft().isIntra();
      iLeftQp           = rcMbDataAccess.getMbDataLeft().getQpLF();
      if( m_bVerMixedMode )
      {
        if      ( rcMbDataAccess.getMbData().getFieldFlag() && yBlk == 0 )
        {
          bLeftIntra      = ( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataLeft().isIntra() : rcMbDataAccess.getMbDataAboveLeft().isIntra() );
          iLeftQp         = ( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataLeft().getQpLF() : rcMbDataAccess.getMbDataAboveLeft().getQpLF() );
        }
        else if ( rcMbDataAccess.getMbData().getFieldFlag() )
        {
          bLeftIntra      = ( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataBelowLeft().isIntra() : rcMbDataAccess.getMbDataLeft().isIntra() );
          iLeftQp         = ( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataBelowLeft().getQpLF() : rcMbDataAccess.getMbDataLeft().getQpLF() );
        }
        else // current pair is frame, left pair is field
        {
          Bool bLI_Top    = ( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataLeft().isIntra() : rcMbDataAccess.getMbDataAboveLeft().isIntra() );
          Bool bLI_Bot    = ( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataBelowLeft().isIntra() : rcMbDataAccess.getMbDataLeft().isIntra() );
          iLeftQpTop      = ( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataLeft().getQpLF() : rcMbDataAccess.getMbDataAboveLeft().getQpLF() );
          iLeftQpBot      = ( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataBelowLeft().getQpLF() : rcMbDataAccess.getMbDataLeft().getQpLF() );
          if( ( bLI_Top ^ bLI_Bot ) && LFM_DEFAULT_FILTER != m_eLFMode )
          {
            bLeftIntra    = true;
            bFilterTopOnly= bLI_Top;
            bFilterBotOnly= bLI_Bot;
          }
          else
          {
            bLeftIntra    = bLI_Top || bLI_Bot;
          }
        }
      }
      filterBlockEdge   = ( LFM_DEFAULT_FILTER == m_eLFMode || bLeftIntra );
    }
    if( filterBlockEdge )
    {
      filterBlockEdge   = ! ( rcDFP.getDisableDeblockingFilterIdc() == 1 ||
                            ( rcDFP.getDisableDeblockingFilterIdc() == 2 && ! rcMbDataAccess.isAvailableLeft() ) );
    }
    if( ! m_bVerMixedMode )
    {
      if( filterBlockEdge )
      {
        Int iQp           = ( iLeftQp + iCurrQp + 1) >> 1;
        Int iIndexB       = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);
        Int iStridex2     = 2*iStride;
        Int iStridex5     = 5*iStride;
        Int iD            = ( abs( (Int)pPelLum[iStridex2-2] -   (Int)pPelLum[iStridex2-1]                                    )
                            + abs( (Int)pPelLum[iStridex2  ] - ( (Int)pPelLum[iStridex2+1] << 1 ) + (Int)pPelLum[iStridex2+2] )
                            + abs( (Int)pPelLum[iStridex5-2] -   (Int)pPelLum[iStridex5-1]                                    )
                            + abs( (Int)pPelLum[iStridex5  ] - ( (Int)pPelLum[iStridex5+1] << 1 ) + (Int)pPelLum[iStridex5+2] ) );
        filterBlockEdge   = ( iD < g_aucBetaTab_H241RCDO [ iIndexB ] );
      }
      if( filterBlockEdge )
      {
        filterBlockEdge   = ( rcMbDataAccess.getMbDataCurr().isIntra() || 
                              rcMbDataAccess.getMbDataLeft().isIntra() );
        if( ! filterBlockEdge &&  m_pcHighpassYuvBuffer )
        {
          filterBlockEdge = ( m_pcHighpassYuvBuffer->isCurr4x4BlkNotZero( cIdx ) ||
                              m_pcHighpassYuvBuffer->isCurr4x4BlkNotZero( cIdx + VER_OFFSET_H241RCDO ) ||
                              m_pcHighpassYuvBuffer->isLeft4x4BlkNotZero( cIdx + LEFT_MB_LEFT_NEIGHBOUR ) ||
                              m_pcHighpassYuvBuffer->isLeft4x4BlkNotZero( cIdx + LEFT_MB_LEFT_NEIGHBOUR + VER_OFFSET_H241RCDO ) );
        }
        if( ! filterBlockEdge )
        {
          filterBlockEdge = ( rcMbDataAccess.getMbDataCurr().is4x4BlkCoded( cIdx ) ||
                              rcMbDataAccess.getMbDataCurr().is4x4BlkCoded( cIdx + VER_OFFSET_H241RCDO ) ||
                              rcMbDataAccess.getMbDataLeft().is4x4BlkCoded( cIdx + LEFT_MB_LEFT_NEIGHBOUR ) ||
                              rcMbDataAccess.getMbDataLeft().is4x4BlkCoded( cIdx + LEFT_MB_LEFT_NEIGHBOUR + VER_OFFSET_H241RCDO ) );
        }
        if( ! filterBlockEdge )
        {
          ROF( pcMbDataAccessMot );
          if ( pcMbDataAccessMot->getMbDataCurr().isInterPMb() && pcMbDataAccessMot->getMbDataLeft().isInterPMb() )
          {
            if( pcRefFrameList0 )
              CheckMVandRefVal  = xCheckMvDataP_RefIdx( pcMbDataAccessMot->getMbDataCurr(), cIdx, pcMbDataAccessMot->getMbDataLeft(), cIdx + LEFT_MB_LEFT_NEIGHBOUR, sHorMvThr, sVerMvThr, *pcRefFrameList0, *pcRefFrameList1 );
            else
              CheckMVandRefVal  = xCheckMvDataP       ( pcMbDataAccessMot->getMbDataCurr(), cIdx, pcMbDataAccessMot->getMbDataLeft(), cIdx + LEFT_MB_LEFT_NEIGHBOUR, sHorMvThr, sVerMvThr );
          }
          else
          {
            if( pcRefFrameList0 && pcRefFrameList1 )
              CheckMVandRefVal  = xCheckMvDataB_RefIdx( pcMbDataAccessMot->getMbDataCurr(), cIdx, pcMbDataAccessMot->getMbDataLeft(), cIdx + LEFT_MB_LEFT_NEIGHBOUR, sHorMvThr, sVerMvThr, *pcRefFrameList0, *pcRefFrameList1 );
            else
              CheckMVandRefVal  = xCheckMvDataB       ( pcMbDataAccessMot->getMbDataCurr(), cIdx, pcMbDataAccessMot->getMbDataLeft(), cIdx + LEFT_MB_LEFT_NEIGHBOUR, sHorMvThr, sVerMvThr );
          }
          filterBlockEdge = ( CheckMVandRefVal > 0 );
        }
      }
    }

    if( filterBlockEdge )
    {
      if( m_bVerMixedMode && ! rcMbDataAccess.getMbData().getFieldFlag() )
      {
        if( ! bFilterBotOnly )
        {
          Int iQp     = ( iLeftQpTop + iCurrQp + 1) >> 1;
          Int iIndexA = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
          Int iIndexB = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);
          xFilter_H241RCDO( pPelLum,           1, iIndexA, iIndexB, 0, true );
          xFilter_H241RCDO( pPelLum+2*iStride, 1, iIndexA, iIndexB, 0, true );
          xFilter_H241RCDO( pPelLum+4*iStride, 1, iIndexA, iIndexB, 0, true );
          xFilter_H241RCDO( pPelLum+6*iStride, 1, iIndexA, iIndexB, 0, true );
        }
        if( ! bFilterTopOnly )
        {
          Int iQp     = ( iLeftQpBot + iCurrQp + 1) >> 1;
          Int iIndexA = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
          Int iIndexB = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);
          xFilter_H241RCDO( pPelLum+  iStride, 1, iIndexA, iIndexB, 0, true );
          xFilter_H241RCDO( pPelLum+3*iStride, 1, iIndexA, iIndexB, 0, true );
          xFilter_H241RCDO( pPelLum+5*iStride, 1, iIndexA, iIndexB, 0, true );
          xFilter_H241RCDO( pPelLum+7*iStride, 1, iIndexA, iIndexB, 0, true );
        }
      }
      else
      {
        Int iQp     = ( iLeftQp + iCurrQp + 1) >> 1;
        Int iIndexA = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
        Int iIndexB = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);
        xFilter_H241RCDO( pPelLum,           1, iIndexA, iIndexB, 0, true );
        xFilter_H241RCDO( pPelLum+  iStride, 1, iIndexA, iIndexB, 0, true );
        xFilter_H241RCDO( pPelLum+2*iStride, 1, iIndexA, iIndexB, 0, true );
        xFilter_H241RCDO( pPelLum+3*iStride, 1, iIndexA, iIndexB, 0, true );
        xFilter_H241RCDO( pPelLum+4*iStride, 1, iIndexA, iIndexB, 0, true );
        xFilter_H241RCDO( pPelLum+5*iStride, 1, iIndexA, iIndexB, 0, true );
        xFilter_H241RCDO( pPelLum+6*iStride, 1, iIndexA, iIndexB, 0, true );
        xFilter_H241RCDO( pPelLum+7*iStride, 1, iIndexA, iIndexB, 0, true );
      }
    }

    pPelLum += 8*iStride; //for the next 8x8 vertical
    if( yBlk == 0 )
    {
      cIdx = cIdx + 8;
    }
  } //end of the loop


  //Inside of the macroblock
  pPelLum  -= 16*iStride-4;
  Int iIndexA   = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iCurrQp, 0, 51);
  Int iIndexB   = gClipMinMax( rcDFP.getSliceBetaOffset()    + iCurrQp, 0, 51);
  for( xBlk = 1; xBlk < 4; xBlk++ )
  {
    cIdx = cIdx - 7;
    for(  yBlk = 0; yBlk <= 8; yBlk += 8 )
    {
      filterBlockEdge     = (  xBlk == 2 || ! b8x8 );
      if( filterBlockEdge )
      {
        filterBlockEdge   = ! ( rcDFP.getDisableDeblockingFilterIdc() == 1 );
      }
      if( filterBlockEdge )
      {
        Int iStridex2     = 2*iStride;
        Int iStridex5     = 5*iStride;
        Int iD            = ( abs( (Int)pPelLum[iStridex2-3] - ( (Int)pPelLum[iStridex2-2] << 1 ) + (Int)pPelLum[iStridex2-1] )
                            + abs( (Int)pPelLum[iStridex2  ] - ( (Int)pPelLum[iStridex2+1] << 1 ) + (Int)pPelLum[iStridex2+2] )
                            + abs( (Int)pPelLum[iStridex5-3] - ( (Int)pPelLum[iStridex5-2] << 1 ) + (Int)pPelLum[iStridex5-1] )
                            + abs( (Int)pPelLum[iStridex5  ] - ( (Int)pPelLum[iStridex5+1] << 1 ) + (Int)pPelLum[iStridex5+2] ) );
        filterBlockEdge   = ( iD < g_aucBetaTab_H241RCDO [ iIndexB ] );
      }
      if( filterBlockEdge )
      {
        filterBlockEdge   = rcMbDataAccess.getMbDataCurr().isIntra();

        if( ! filterBlockEdge && m_pcHighpassYuvBuffer )
        {
          filterBlockEdge = ( m_pcHighpassYuvBuffer->isCurr4x4BlkNotZero( cIdx ) ||
                              m_pcHighpassYuvBuffer->isCurr4x4BlkNotZero( cIdx + VER_OFFSET_H241RCDO ) ||
                              m_pcHighpassYuvBuffer->isCurr4x4BlkNotZero( cIdx + CURR_MB_LEFT_NEIGHBOUR ) ||
                              m_pcHighpassYuvBuffer->isCurr4x4BlkNotZero( cIdx + CURR_MB_LEFT_NEIGHBOUR + VER_OFFSET_H241RCDO ) );
        }
        if( ! filterBlockEdge )
        {
          filterBlockEdge = ( rcMbDataAccess.getMbDataCurr().is4x4BlkCoded( cIdx ) ||
                              rcMbDataAccess.getMbDataCurr().is4x4BlkCoded( cIdx + VER_OFFSET_H241RCDO ) ||
                              rcMbDataAccess.getMbDataCurr().is4x4BlkCoded( cIdx + CURR_MB_LEFT_NEIGHBOUR ) ||
                              rcMbDataAccess.getMbDataCurr().is4x4BlkCoded( cIdx + CURR_MB_LEFT_NEIGHBOUR + VER_OFFSET_H241RCDO ) );
        }
        if( ! filterBlockEdge )
        {
          ROF( pcMbDataAccessMot );
          if ( pcMbDataAccessMot->getMbDataCurr().isInterPMb() )
          {
            if( pcRefFrameList0 )
              CheckMVandRefVal  = xCheckMvDataP_RefIdx( pcMbDataAccessMot->getMbDataCurr(), cIdx, pcMbDataAccessMot->getMbDataCurr(), cIdx + CURR_MB_LEFT_NEIGHBOUR, sHorMvThr, sVerMvThr, *pcRefFrameList0, *pcRefFrameList1 );
            else
              CheckMVandRefVal  = xCheckMvDataP       ( pcMbDataAccessMot->getMbDataCurr(), cIdx, pcMbDataAccessMot->getMbDataCurr(), cIdx + CURR_MB_LEFT_NEIGHBOUR, sHorMvThr, sVerMvThr );
          }
          else
          {
            if( pcRefFrameList0 && pcRefFrameList1 )
              CheckMVandRefVal  = xCheckMvDataB_RefIdx( pcMbDataAccessMot->getMbDataCurr(), cIdx, pcMbDataAccessMot->getMbDataCurr(), cIdx + CURR_MB_LEFT_NEIGHBOUR, sHorMvThr, sVerMvThr, *pcRefFrameList0, *pcRefFrameList1 );
            else
              CheckMVandRefVal  = xCheckMvDataB       ( pcMbDataAccessMot->getMbDataCurr(), cIdx, pcMbDataAccessMot->getMbDataCurr(), cIdx + CURR_MB_LEFT_NEIGHBOUR, sHorMvThr, sVerMvThr );
          }
          filterBlockEdge = ( CheckMVandRefVal > 0 );
        }
      }

      if( filterBlockEdge )
      {
        xFilter_H241RCDO( pPelLum,           1, iIndexA, iIndexB, 1, true );
        xFilter_H241RCDO( pPelLum+  iStride, 1, iIndexA, iIndexB, 1, true );
        xFilter_H241RCDO( pPelLum+2*iStride, 1, iIndexA, iIndexB, 1, true );
        xFilter_H241RCDO( pPelLum+3*iStride, 1, iIndexA, iIndexB, 1, true );
        xFilter_H241RCDO( pPelLum+4*iStride, 1, iIndexA, iIndexB, 1, true );
        xFilter_H241RCDO( pPelLum+5*iStride, 1, iIndexA, iIndexB, 1, true );
        xFilter_H241RCDO( pPelLum+6*iStride, 1, iIndexA, iIndexB, 1, true );
        xFilter_H241RCDO( pPelLum+7*iStride, 1, iIndexA, iIndexB, 1, true );
      }

      pPelLum += 8*iStride; //for the next 8x8 vertical
      if( yBlk == 0 )
      {
        cIdx = cIdx + 8;
      }
    } //end of the 1st loop

    pPelLum -= 16*iStride-4;  //back to the original position
  } //end of the 2nd loop

  return Err::m_nOK;
}


__inline ErrVal LoopFilter::xChromaVerFiltering_H241RCDO ( const MbDataAccess& rcMbDataAccess, const DFP& rcDFP, IntYuvPicBuffer* pcYuvBuffer )
{
  Int   iCurrQp = rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.getMbDataCurr().getQpLF() );
  Int   iStride = pcYuvBuffer->getCStride();
  XPel* pPelCb  = pcYuvBuffer->getMbCbAddr();
  XPel* pPelCr  = pcYuvBuffer->getMbCrAddr();
  Bool  filterBlockEdge, bFilterTopOnly, bFilterBotOnly;
  Int   yBlk, iLeftQp = 0, iLeftQpTop = 0, iLeftQpBot = 0;

  //===== filtering of left macroblock edge =====
  for( yBlk = 0; yBlk <= 4; yBlk+=4 )
  {
    filterBlockEdge   = rcMbDataAccess.isLeftMbExisting();
    bFilterTopOnly    = false;
    bFilterBotOnly    = false;
    if( filterBlockEdge )
    {
      Bool bLeftIntra = rcMbDataAccess.getMbDataLeft().isIntra();
      iLeftQp         = rcMbDataAccess.getMbDataLeft().getQpLF();
      if( m_bVerMixedMode )
      {
        if      ( rcMbDataAccess.getMbData().getFieldFlag() && yBlk == 0 )
        {
          bLeftIntra      = ( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataLeft().isIntra() : rcMbDataAccess.getMbDataAboveLeft().isIntra() );
          iLeftQp         = ( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataLeft().getQpLF() : rcMbDataAccess.getMbDataAboveLeft().getQpLF() );
        }
        else if ( rcMbDataAccess.getMbData().getFieldFlag() )
        {
          bLeftIntra      = ( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataBelowLeft().isIntra() : rcMbDataAccess.getMbDataLeft().isIntra() );
          iLeftQp         = ( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataBelowLeft().getQpLF() : rcMbDataAccess.getMbDataLeft().getQpLF() );
        }
        else // current pair is frame, left pair is field
        {
          Bool bLI_Top    = ( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataLeft().isIntra() : rcMbDataAccess.getMbDataAboveLeft().isIntra() );
          Bool bLI_Bot    = ( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataBelowLeft().isIntra() : rcMbDataAccess.getMbDataLeft().isIntra() );
          iLeftQpTop      = ( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataLeft().getQpLF() : rcMbDataAccess.getMbDataAboveLeft().getQpLF() );
          iLeftQpBot      = ( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataBelowLeft().getQpLF() : rcMbDataAccess.getMbDataLeft().getQpLF() );
          if( ( bLI_Top ^ bLI_Bot ) && LFM_DEFAULT_FILTER != m_eLFMode )
          {
            bLeftIntra    = true;
            bFilterTopOnly= bLI_Top;
            bFilterBotOnly= bLI_Bot;
          }
          else
          {
            bLeftIntra    = bLI_Top || bLI_Bot;
          }
        }
      }
      filterBlockEdge = ( LFM_DEFAULT_FILTER == m_eLFMode || bLeftIntra );
    }
    if( filterBlockEdge )
    {
      filterBlockEdge = ! ( rcDFP.getDisableDeblockingFilterIdc() == 1 ||
                          ( rcDFP.getDisableDeblockingFilterIdc() == 2 && ! rcMbDataAccess.isAvailableLeft() ) );
    }
    if( filterBlockEdge && ! m_bVerMixedMode )
    {
      filterBlockEdge = ( rcMbDataAccess.getMbDataCurr().isIntra() || rcMbDataAccess.getMbDataLeft().isIntra() ); 
    }

    if( filterBlockEdge )
    {
      if( m_bVerMixedMode && ! rcMbDataAccess.getMbData().getFieldFlag() )
      {
        if( ! bFilterBotOnly )
        {
          Int   iLQp    = rcMbDataAccess.getSH().getChromaQp( iLeftQpTop );
          Int   iQp     = ( iCurrQp + iLQp + 1 ) >> 1;
          Int   iIndexA = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
          Int   iIndexB = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);
          xFilter_H241RCDO( pPelCb,           1, iIndexA, iIndexB, 0, false );
          xFilter_H241RCDO( pPelCb+2*iStride, 1, iIndexA, iIndexB, 0, false );
          xFilter_H241RCDO( pPelCr,           1, iIndexA, iIndexB, 0, false );
          xFilter_H241RCDO( pPelCr+2*iStride, 1, iIndexA, iIndexB, 0, false );
        }
        if( ! bFilterTopOnly )
        {
          Int   iLQp    = rcMbDataAccess.getSH().getChromaQp( iLeftQpBot );
          Int   iQp     = ( iCurrQp + iLQp + 1 ) >> 1;
          Int   iIndexA = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
          Int   iIndexB = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);
          xFilter_H241RCDO( pPelCb+  iStride, 1, iIndexA, iIndexB, 0, false );
          xFilter_H241RCDO( pPelCb+3*iStride, 1, iIndexA, iIndexB, 0, false );
          xFilter_H241RCDO( pPelCr+  iStride, 1, iIndexA, iIndexB, 0, false );
          xFilter_H241RCDO( pPelCr+3*iStride, 1, iIndexA, iIndexB, 0, false );
        }
      }
      else
      {
        Int   iLQp    = rcMbDataAccess.getSH().getChromaQp( iLeftQp );
        Int   iQp     = ( iCurrQp + iLQp + 1 ) >> 1;
        Int   iIndexA = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
        Int   iIndexB = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);
        xFilter_H241RCDO( pPelCb,           1, iIndexA, iIndexB, 0, false );
        xFilter_H241RCDO( pPelCb+  iStride, 1, iIndexA, iIndexB, 0, false );
        xFilter_H241RCDO( pPelCb+2*iStride, 1, iIndexA, iIndexB, 0, false );
        xFilter_H241RCDO( pPelCb+3*iStride, 1, iIndexA, iIndexB, 0, false );
        xFilter_H241RCDO( pPelCr,           1, iIndexA, iIndexB, 0, false );
        xFilter_H241RCDO( pPelCr+  iStride, 1, iIndexA, iIndexB, 0, false );
        xFilter_H241RCDO( pPelCr+2*iStride, 1, iIndexA, iIndexB, 0, false );
        xFilter_H241RCDO( pPelCr+3*iStride, 1, iIndexA, iIndexB, 0, false );
      }
    }

    pPelCb += 4*iStride; 
    pPelCr += 4*iStride; 
  }

  //Inside of the macroblock
  pPelCb -= 8*iStride-4;
  pPelCr -= 8*iStride-4;
  Int iIndexA = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iCurrQp, 0, 51);
  Int iIndexB = gClipMinMax( rcDFP.getSliceBetaOffset()    + iCurrQp, 0, 51);
  for( yBlk = 0; yBlk <= 4; yBlk += 4 )
  {
    filterBlockEdge   = ! ( rcDFP.getDisableDeblockingFilterIdc() == 1 );
    if( filterBlockEdge )
    {
      filterBlockEdge = rcMbDataAccess.getMbDataCurr().isIntra();
    }

    if( filterBlockEdge )
    {
      xFilter_H241RCDO( pPelCb,           1, iIndexA, iIndexB, 1, false );
      xFilter_H241RCDO( pPelCb+  iStride, 1, iIndexA, iIndexB, 1, false );
      xFilter_H241RCDO( pPelCb+2*iStride, 1, iIndexA, iIndexB, 1, false );
      xFilter_H241RCDO( pPelCb+3*iStride, 1, iIndexA, iIndexB, 1, false );
      xFilter_H241RCDO( pPelCr,           1, iIndexA, iIndexB, 1, false );
      xFilter_H241RCDO( pPelCr+  iStride, 1, iIndexA, iIndexB, 1, false );
      xFilter_H241RCDO( pPelCr+2*iStride, 1, iIndexA, iIndexB, 1, false );
      xFilter_H241RCDO( pPelCr+3*iStride, 1, iIndexA, iIndexB, 1, false );
    }

    pPelCb += 4*iStride; //for the next 4x4 vertical
    pPelCr += 4*iStride; //for the next 4x4 vertical
  }

  return Err::m_nOK;
}



__inline ErrVal LoopFilter::xLumaHorFiltering_H241RCDO( const MbDataAccess& rcMbDataAccess, const DFP& rcDFP, YuvPicBuffer* pcYuvBuffer )
{
  Int   iCurrQp = rcMbDataAccess.getMbDataCurr().getQpLF();
  Int   iStride = pcYuvBuffer->getLStride();
  Pel*  pPelLum = pcYuvBuffer->getMbLumAddr();

  Bool  b8x8            = rcMbDataAccess.getMbData().isTransformSize8x8();
  Bool  filterBlockEdge = false;
  Bool  bFilterTopOnly  = false;
  Bool  bFilterBotOnly  = false;
  Short sHorMvThr       = 4;
  Short sVerMvThr       = ( FRAME == rcMbDataAccess.getMbPicType() ? 4 : 2 );

  Int   CheckMVandRefVal;
  Int   xBlk, yBlk, iAboveQp = 0, iAboveQpTop = 0, iAboveQpBot = 0;

  B4x4Idx cIdx = 0;
  for( xBlk = 0; xBlk <= 8; xBlk += 8 ) //On the above edge of a macroblock
  {
    filterBlockEdge     = rcMbDataAccess.isAboveMbExisting();
    bFilterTopOnly      = false;
    bFilterBotOnly      = false;
    if( filterBlockEdge )
    {
      Bool bAboveIntra  = ( rcMbDataAccess.getMbData().getFieldFlag() ? rcMbDataAccess.getMbDataAboveAbove().isIntra() : rcMbDataAccess.getMbDataAbove().isIntra() );
      iAboveQp          = ( rcMbDataAccess.getMbData().getFieldFlag() ? rcMbDataAccess.getMbDataAboveAbove().getQpLF() : rcMbDataAccess.getMbDataAbove().getQpLF() );
      if( m_bHorMixedMode )
      {
        if      ( rcMbDataAccess.getMbData().getFieldFlag() && rcMbDataAccess.isTopMb() )
        {
          bAboveIntra   = rcMbDataAccess.getMbDataAbove().isIntra();
          iAboveQp      = rcMbDataAccess.getMbDataAbove().getQpLF();
        }
        else if ( rcMbDataAccess.getMbData().getFieldFlag() )
        {
          bAboveIntra   = rcMbDataAccess.getMbDataAboveAbove().isIntra();
          iAboveQp      = rcMbDataAccess.getMbDataAboveAbove().getQpLF();
        }
        else
        {
          AOF( rcMbDataAccess.isTopMb() );
          Bool bLI_Top    = rcMbDataAccess.getMbDataAboveAbove().isIntra();
          Bool bLI_Bot    = rcMbDataAccess.getMbDataAbove     ().isIntra();
          iAboveQpTop     = rcMbDataAccess.getMbDataAboveAbove().getQpLF();
          iAboveQpBot     = rcMbDataAccess.getMbDataAbove     ().getQpLF();
          if( ( bLI_Top ^ bLI_Bot ) && LFM_DEFAULT_FILTER != m_eLFMode )
          {
            bAboveIntra   = true;
            bFilterTopOnly= bLI_Top;
            bFilterBotOnly= bLI_Bot;
          }
          else
          {
            bAboveIntra   = bLI_Top || bLI_Bot;
          }
        }
      }
      filterBlockEdge   = ( LFM_DEFAULT_FILTER == m_eLFMode || bAboveIntra );
    }
    if( filterBlockEdge )
    {
      filterBlockEdge   = ! ( rcDFP.getDisableDeblockingFilterIdc() == 1 ||
                            ( rcDFP.getDisableDeblockingFilterIdc() == 2 && ! rcMbDataAccess.isAvailableAbove() ) );
    }
    if( ! m_bHorMixedMode )
    {
      if( filterBlockEdge )
      {
        Int iQp           = ( iAboveQp + iCurrQp + 1) >> 1;
        Int iIndexB       = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);
        Int iStridex2     = 2*iStride;
        Int iD            = ( abs( (Int)pPelLum[2-iStridex2] -   (Int)pPelLum[2-iStride]                                    )
                            + abs( (Int)pPelLum[2]           - ( (Int)pPelLum[2+iStride] << 1 ) + (Int)pPelLum[2+iStridex2] )
                            + abs( (Int)pPelLum[5-iStridex2] -   (Int)pPelLum[5-iStride]                                    )
                            + abs( (Int)pPelLum[5]           - ( (Int)pPelLum[5+iStride] << 1 ) + (Int)pPelLum[5+iStridex2] ) );
        filterBlockEdge   = ( iD < g_aucBetaTab_H241RCDO [ iIndexB ] );
      }
      if( filterBlockEdge )
      {
        filterBlockEdge   = ( rcMbDataAccess.getMbDataCurr ().isIntra() || 
                              rcMbDataAccess.getMbDataAbove().isIntra() );
        if( ! filterBlockEdge && m_pcHighpassYuvBuffer )
        {
          filterBlockEdge = ( m_pcHighpassYuvBuffer->isCurr4x4BlkNotZero ( cIdx ) ||
                              m_pcHighpassYuvBuffer->isCurr4x4BlkNotZero ( cIdx + HOR_OFFSET_H241RCDO ) ||
                              m_pcHighpassYuvBuffer->isAbove4x4BlkNotZero( cIdx + ABOVE_MB_ABOVE_NEIGHBOUR ) ||
                              m_pcHighpassYuvBuffer->isAbove4x4BlkNotZero( cIdx + ABOVE_MB_ABOVE_NEIGHBOUR + HOR_OFFSET_H241RCDO ) );
        }
        if( ! filterBlockEdge )
        {
          filterBlockEdge = ( rcMbDataAccess.getMbDataCurr ().is4x4BlkCoded( cIdx ) ||
                              rcMbDataAccess.getMbDataCurr ().is4x4BlkCoded( cIdx + HOR_OFFSET_H241RCDO ) ||
                              rcMbDataAccess.getMbDataAbove().is4x4BlkCoded( cIdx + ABOVE_MB_ABOVE_NEIGHBOUR ) ||
                              rcMbDataAccess.getMbDataAbove().is4x4BlkCoded( cIdx + ABOVE_MB_ABOVE_NEIGHBOUR + HOR_OFFSET_H241RCDO ) );
        }
        if( ! filterBlockEdge )
        {
          if( rcMbDataAccess.getMbDataCurr().isInterPMb() && rcMbDataAccess.getMbDataAbove().isInterPMb() )
          {
            CheckMVandRefVal= xCheckMvDataP( rcMbDataAccess.getMbDataCurr(), cIdx, rcMbDataAccess.getMbDataAbove(), cIdx + ABOVE_MB_ABOVE_NEIGHBOUR, sHorMvThr, sVerMvThr );
          }
          else
          {
            CheckMVandRefVal= xCheckMvDataB( rcMbDataAccess.getMbDataCurr(), cIdx, rcMbDataAccess.getMbDataAbove(), cIdx + ABOVE_MB_ABOVE_NEIGHBOUR, sHorMvThr, sVerMvThr );
          }
          filterBlockEdge = ( CheckMVandRefVal > 0 );
        }
      }
    }

    if( filterBlockEdge )
    {
      if( m_bHorMixedMode && ! rcMbDataAccess.getMbData().getFieldFlag() )
      {
        if( ! bFilterBotOnly )
        {
          Int     iQp           = ( iAboveQpTop + iCurrQp + 1) >> 1;
          Int     iIndexA       = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
          Int     iIndexB       = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);
          xFilter_H241RCDO( pPelLum,   iStride, iIndexA, iIndexB, 0, true );
          xFilter_H241RCDO( pPelLum+2, iStride, iIndexA, iIndexB, 0, true );
          xFilter_H241RCDO( pPelLum+4, iStride, iIndexA, iIndexB, 0, true );
          xFilter_H241RCDO( pPelLum+6, iStride, iIndexA, iIndexB, 0, true );
        }
        if( ! bFilterTopOnly )
        {
          Int     iQp           = ( iAboveQpBot + iCurrQp + 1) >> 1;
          Int     iIndexA       = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
          Int     iIndexB       = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);
          xFilter_H241RCDO( pPelLum+1, iStride, iIndexA, iIndexB, 0, true );
          xFilter_H241RCDO( pPelLum+3, iStride, iIndexA, iIndexB, 0, true );
          xFilter_H241RCDO( pPelLum+5, iStride, iIndexA, iIndexB, 0, true );
          xFilter_H241RCDO( pPelLum+7, iStride, iIndexA, iIndexB, 0, true );
        }
      }
      else
      {
        Int     iQp           = ( iAboveQp + iCurrQp + 1) >> 1;
        Int     iIndexA       = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
        Int     iIndexB       = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);
        xFilter_H241RCDO( pPelLum,   iStride, iIndexA, iIndexB, 0, true );
        xFilter_H241RCDO( pPelLum+1, iStride, iIndexA, iIndexB, 0, true );
        xFilter_H241RCDO( pPelLum+2, iStride, iIndexA, iIndexB, 0, true );
        xFilter_H241RCDO( pPelLum+3, iStride, iIndexA, iIndexB, 0, true );
        xFilter_H241RCDO( pPelLum+4, iStride, iIndexA, iIndexB, 0, true );
        xFilter_H241RCDO( pPelLum+5, iStride, iIndexA, iIndexB, 0, true );
        xFilter_H241RCDO( pPelLum+6, iStride, iIndexA, iIndexB, 0, true );
        xFilter_H241RCDO( pPelLum+7, iStride, iIndexA, iIndexB, 0, true );
      }
    }

    pPelLum += 8; //for the next 8x8 vertical
    if( xBlk == 0 )
    {
      cIdx = cIdx + 2; //value order: 0,2
    }
  } //end of the loop


  //Inside of the macroblock
  pPelLum += 4*iStride - 16; 
  Int iIndexA  = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iCurrQp, 0, 51);
  Int iIndexB  = gClipMinMax( rcDFP.getSliceBetaOffset()    + iCurrQp, 0, 51);
  for( yBlk = 1; yBlk < 4; yBlk++ )
  {
    cIdx = cIdx + 2;
    for( xBlk = 0; xBlk <= 8; xBlk += 8 )
    {
      filterBlockEdge     = ( yBlk == 2 || ! b8x8 );
      if( filterBlockEdge )
      {
        filterBlockEdge   = ! ( rcDFP.getDisableDeblockingFilterIdc() == 1 );
      }
      if( filterBlockEdge )
      {
        Int iStridex2     = 2*iStride;
        Int iStridex3     = 3*iStride;
        Int iD            = ( abs( (Int)pPelLum[2-iStridex3] - ( (Int)pPelLum[2-iStridex2] << 1 ) + (Int)pPelLum[2-iStride  ] )
                            + abs( (Int)pPelLum[2          ] - ( (Int)pPelLum[2+iStride  ] << 1 ) + (Int)pPelLum[2+iStridex2] )
                            + abs( (Int)pPelLum[5-iStridex3] - ( (Int)pPelLum[5-iStridex2] << 1 ) + (Int)pPelLum[5-iStride  ] )
                            + abs( (Int)pPelLum[5          ] - ( (Int)pPelLum[5+iStride  ] << 1 ) + (Int)pPelLum[5+iStridex2] ) );
        filterBlockEdge   = ( iD < g_aucBetaTab_H241RCDO [ iIndexB ] );
      }
      if( filterBlockEdge )
      {
        filterBlockEdge   = rcMbDataAccess.getMbDataCurr().isIntra();

        if( ! filterBlockEdge && m_pcHighpassYuvBuffer )
        {
          filterBlockEdge = ( m_pcHighpassYuvBuffer->isCurr4x4BlkNotZero( cIdx ) ||
                              m_pcHighpassYuvBuffer->isCurr4x4BlkNotZero( cIdx + HOR_OFFSET_H241RCDO ) ||
                              m_pcHighpassYuvBuffer->isCurr4x4BlkNotZero( cIdx + CURR_MB_ABOVE_NEIGHBOUR ) ||
                              m_pcHighpassYuvBuffer->isCurr4x4BlkNotZero( cIdx + CURR_MB_ABOVE_NEIGHBOUR + HOR_OFFSET_H241RCDO ) );
        }
        if( ! filterBlockEdge )
        {
          filterBlockEdge = ( rcMbDataAccess.getMbDataCurr().is4x4BlkCoded( cIdx ) ||
                              rcMbDataAccess.getMbDataCurr().is4x4BlkCoded( cIdx + HOR_OFFSET_H241RCDO ) ||
                              rcMbDataAccess.getMbDataCurr().is4x4BlkCoded( cIdx + CURR_MB_ABOVE_NEIGHBOUR ) ||
                              rcMbDataAccess.getMbDataCurr().is4x4BlkCoded( cIdx + CURR_MB_ABOVE_NEIGHBOUR + HOR_OFFSET_H241RCDO ) );
        }
        if( ! filterBlockEdge )
        {
          if( rcMbDataAccess.getMbDataCurr().isInterPMb() ) 
          {
            CheckMVandRefVal= xCheckMvDataP( rcMbDataAccess.getMbDataCurr(), cIdx, rcMbDataAccess.getMbDataCurr(), cIdx+CURR_MB_ABOVE_NEIGHBOUR, sHorMvThr, sVerMvThr );
          }
          else
          {
            CheckMVandRefVal= xCheckMvDataB( rcMbDataAccess.getMbDataCurr(), cIdx, rcMbDataAccess.getMbDataCurr(), cIdx+CURR_MB_ABOVE_NEIGHBOUR, sHorMvThr, sVerMvThr );
          }
          filterBlockEdge = ( CheckMVandRefVal > 0 );
        }
      }

      if( filterBlockEdge )
      {
        xFilter_H241RCDO( pPelLum,   iStride, iIndexA, iIndexB, 1, true );
        xFilter_H241RCDO( pPelLum+1, iStride, iIndexA, iIndexB, 1, true );
        xFilter_H241RCDO( pPelLum+2, iStride, iIndexA, iIndexB, 1, true );
        xFilter_H241RCDO( pPelLum+3, iStride, iIndexA, iIndexB, 1, true );
        xFilter_H241RCDO( pPelLum+4, iStride, iIndexA, iIndexB, 1, true );
        xFilter_H241RCDO( pPelLum+5, iStride, iIndexA, iIndexB, 1, true );
        xFilter_H241RCDO( pPelLum+6, iStride, iIndexA, iIndexB, 1, true );
        xFilter_H241RCDO( pPelLum+7, iStride, iIndexA, iIndexB, 1, true );
      }

      pPelLum += 8; //for the next 8x8 vertical
      if( xBlk == 0 )
      {
        cIdx = cIdx + 2;  
      }
    } //end of the 1st loop

    pPelLum += 4*iStride - 16;  //back to the original position
  } //end of the 2nd loop

  return Err::m_nOK;
}


__inline ErrVal LoopFilter::xChromaHorFiltering_H241RCDO ( const MbDataAccess& rcMbDataAccess, const DFP& rcDFP, YuvPicBuffer* pcYuvBuffer )
{
  Int   iCurrQp   = rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.getMbDataCurr().getQpLF() );
  Int   iStride   = pcYuvBuffer->getCStride();
  Pel*  pPelCb    = pcYuvBuffer->getMbCbAddr();
  Pel*  pPelCr    = pcYuvBuffer->getMbCrAddr();
  Bool  filterBlockEdge, bFilterTopOnly, bFilterBotOnly;
  Int   xBlk, iAboveQp = 0, iAboveQpTop = 0, iAboveQpBot = 0;

  for( xBlk = 0; xBlk <= 4; xBlk += 4 )
  {
    filterBlockEdge   = rcMbDataAccess.isAboveMbExisting();
    bFilterTopOnly    = false;
    bFilterBotOnly    = false;
    if( filterBlockEdge )
    {
      Bool bAboveIntra  = ( rcMbDataAccess.getMbData().getFieldFlag() ? rcMbDataAccess.getMbDataAboveAbove().isIntra() : rcMbDataAccess.getMbDataAbove().isIntra() );
      iAboveQp          = ( rcMbDataAccess.getMbData().getFieldFlag() ? rcMbDataAccess.getMbDataAboveAbove().getQpLF() : rcMbDataAccess.getMbDataAbove().getQpLF() );
      if( m_bHorMixedMode )
      {
        if      ( rcMbDataAccess.getMbData().getFieldFlag() && rcMbDataAccess.isTopMb() )
        {
          bAboveIntra   = rcMbDataAccess.getMbDataAbove().isIntra();
          iAboveQp      = rcMbDataAccess.getMbDataAbove().getQpLF();
        }
        else if ( rcMbDataAccess.getMbData().getFieldFlag() )
        {
          bAboveIntra   = rcMbDataAccess.getMbDataAboveAbove().isIntra();
          iAboveQp      = rcMbDataAccess.getMbDataAboveAbove().getQpLF();
        }
        else
        {
          AOF( rcMbDataAccess.isTopMb() );
          Bool bLI_Top    = rcMbDataAccess.getMbDataAboveAbove().isIntra();
          Bool bLI_Bot    = rcMbDataAccess.getMbDataAbove     ().isIntra();
          iAboveQpTop     = rcMbDataAccess.getMbDataAboveAbove().getQpLF();
          iAboveQpBot     = rcMbDataAccess.getMbDataAbove     ().getQpLF();
          if( ( bLI_Top ^ bLI_Bot ) && LFM_DEFAULT_FILTER != m_eLFMode )
          {
            bAboveIntra   = true;
            bFilterTopOnly= bLI_Top;
            bFilterBotOnly= bLI_Bot;
          }
          else
          {
            bAboveIntra   = bLI_Top || bLI_Bot;
          }
        }
      }
      filterBlockEdge   = ( LFM_DEFAULT_FILTER == m_eLFMode || bAboveIntra );
    }
    if( filterBlockEdge )
    {
      filterBlockEdge = ! ( rcDFP.getDisableDeblockingFilterIdc() == 1 ||
                          ( rcDFP.getDisableDeblockingFilterIdc() == 2 && ! rcMbDataAccess.isAvailableAbove() ) );
    }
    if( filterBlockEdge && ! m_bHorMixedMode )
    {
      filterBlockEdge = ( rcMbDataAccess.getMbDataCurr ().isIntra() || 
                          rcMbDataAccess.getMbDataAbove().isIntra() );
    }

    if( filterBlockEdge )
    {
      if( m_bHorMixedMode && ! rcMbDataAccess.getMbData().getFieldFlag() )
      {
        if( ! bFilterBotOnly )
        {
          Int   iAQp      = rcMbDataAccess.getSH().getChromaQp( iAboveQpTop );
          Int   iQp       = ( iCurrQp + iAQp + 1 ) >> 1;
          Int   iIndexA   = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
          Int   iIndexB   = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);
          xFilter_H241RCDO( pPelCb,   iStride, iIndexA, iIndexB, 0, false );
          xFilter_H241RCDO( pPelCb+2, iStride, iIndexA, iIndexB, 0, false );
          xFilter_H241RCDO( pPelCr,   iStride, iIndexA, iIndexB, 0, false );
          xFilter_H241RCDO( pPelCr+2, iStride, iIndexA, iIndexB, 0, false );
        }
        if( ! bFilterTopOnly )
        {
          Int   iAQp      = rcMbDataAccess.getSH().getChromaQp( iAboveQpBot );
          Int   iQp       = ( iCurrQp + iAQp + 1 ) >> 1;
          Int   iIndexA   = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
          Int   iIndexB   = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);
          xFilter_H241RCDO( pPelCb+1, iStride, iIndexA, iIndexB, 0, false );
          xFilter_H241RCDO( pPelCb+3, iStride, iIndexA, iIndexB, 0, false );
          xFilter_H241RCDO( pPelCr+1, iStride, iIndexA, iIndexB, 0, false );
          xFilter_H241RCDO( pPelCr+3, iStride, iIndexA, iIndexB, 0, false );
        }
      }
      else
      {
        Int   iAQp      = rcMbDataAccess.getSH().getChromaQp( iAboveQp );
        Int   iQp       = ( iCurrQp + iAQp + 1 ) >> 1;
        Int   iIndexA   = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
        Int   iIndexB   = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);
        xFilter_H241RCDO( pPelCb,   iStride, iIndexA, iIndexB, 0, false );
        xFilter_H241RCDO( pPelCb+1, iStride, iIndexA, iIndexB, 0, false );
        xFilter_H241RCDO( pPelCb+2, iStride, iIndexA, iIndexB, 0, false );
        xFilter_H241RCDO( pPelCb+3, iStride, iIndexA, iIndexB, 0, false );
        xFilter_H241RCDO( pPelCr,   iStride, iIndexA, iIndexB, 0, false );
        xFilter_H241RCDO( pPelCr+1, iStride, iIndexA, iIndexB, 0, false );
        xFilter_H241RCDO( pPelCr+2, iStride, iIndexA, iIndexB, 0, false );
        xFilter_H241RCDO( pPelCr+3, iStride, iIndexA, iIndexB, 0, false );
      }
    }

    pPelCb += 4; 
    pPelCr += 4; 
  }

  //Inside of the macroblock
  pPelCb += 4*iStride - 8;
  pPelCr += 4*iStride - 8;
  Int iIndexA = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iCurrQp, 0, 51);
  Int iIndexB = gClipMinMax( rcDFP.getSliceBetaOffset()    + iCurrQp, 0, 51);
  for( xBlk = 0; xBlk <= 4; xBlk += 4 )
  {
    filterBlockEdge   = ! ( rcDFP.getDisableDeblockingFilterIdc() == 1 );
    if( filterBlockEdge )
    {
      filterBlockEdge = rcMbDataAccess.getMbDataCurr().isIntra();
    }

    if( filterBlockEdge )
    {
      xFilter_H241RCDO( pPelCb,   iStride, iIndexA, iIndexB, 1, false );
      xFilter_H241RCDO( pPelCb+1, iStride, iIndexA, iIndexB, 1, false );
      xFilter_H241RCDO( pPelCb+2, iStride, iIndexA, iIndexB, 1, false );
      xFilter_H241RCDO( pPelCb+3, iStride, iIndexA, iIndexB, 1, false );
      xFilter_H241RCDO( pPelCr,   iStride, iIndexA, iIndexB, 1, false );
      xFilter_H241RCDO( pPelCr+1, iStride, iIndexA, iIndexB, 1, false );
      xFilter_H241RCDO( pPelCr+2, iStride, iIndexA, iIndexB, 1, false );
      xFilter_H241RCDO( pPelCr+3, iStride, iIndexA, iIndexB, 1, false );
    }

    pPelCb += 4; //for the next 4x4 vertical
    pPelCr += 4; //for the next 4x4 vertical
  } //end of the 1st loop

  return Err::m_nOK;
}



__inline ErrVal LoopFilter::xLumaHorFiltering_H241RCDO( const MbDataAccess& rcMbDataAccess, 
                                                       const DFP&          rcDFP, 
                                                       IntYuvPicBuffer*    pcYuvBuffer, 
                                                       const MbDataAccess* pcMbDataAccessMot, //HZL added
                                                       RefFrameList*       pcRefFrameList0,   //HZL added
                                                       RefFrameList*       pcRefFrameList1 )  //HZL added													   
{
  Int   iCurrQp = rcMbDataAccess.getMbDataCurr().getQpLF();
  Int   iStride = pcYuvBuffer->getLStride();
  XPel* pPelLum = pcYuvBuffer->getMbLumAddr();

  Bool  b8x8            = rcMbDataAccess.getMbData().isTransformSize8x8();
  Bool  filterBlockEdge = false;
  Bool  bFilterTopOnly  = false;
  Bool  bFilterBotOnly  = false;
  Short sHorMvThr       = 4;
  Short sVerMvThr       = ( FRAME == rcMbDataAccess.getMbPicType() ? 4 : 2 );

  Int   CheckMVandRefVal;
  Int   xBlk, yBlk, iAboveQp = 0, iAboveQpTop = 0, iAboveQpBot = 0;

  B4x4Idx cIdx = 0;
  for( xBlk = 0; xBlk <= 8; xBlk += 8 ) //On the above edge of a macroblock
  {
    filterBlockEdge     = rcMbDataAccess.isAboveMbExisting();
    bFilterTopOnly      = false;
    bFilterBotOnly      = false;
    if( filterBlockEdge )
    {
      Bool bAboveIntra  = ( rcMbDataAccess.getMbData().getFieldFlag() ? rcMbDataAccess.getMbDataAboveAbove().isIntra() : rcMbDataAccess.getMbDataAbove().isIntra() );
      iAboveQp          = ( rcMbDataAccess.getMbData().getFieldFlag() ? rcMbDataAccess.getMbDataAboveAbove().getQpLF() : rcMbDataAccess.getMbDataAbove().getQpLF() );
      if( m_bHorMixedMode )
      {
        if      ( rcMbDataAccess.getMbData().getFieldFlag() && rcMbDataAccess.isTopMb() )
        {
          bAboveIntra   = rcMbDataAccess.getMbDataAbove().isIntra();
          iAboveQp      = rcMbDataAccess.getMbDataAbove().getQpLF();
        }
        else if ( rcMbDataAccess.getMbData().getFieldFlag() )
        {
          bAboveIntra   = rcMbDataAccess.getMbDataAboveAbove().isIntra();
          iAboveQp      = rcMbDataAccess.getMbDataAboveAbove().getQpLF();
        }
        else
        {
          AOF( rcMbDataAccess.isTopMb() );
          Bool bLI_Top    = rcMbDataAccess.getMbDataAboveAbove().isIntra();
          Bool bLI_Bot    = rcMbDataAccess.getMbDataAbove     ().isIntra();
          iAboveQpTop     = rcMbDataAccess.getMbDataAboveAbove().getQpLF();
          iAboveQpBot     = rcMbDataAccess.getMbDataAbove     ().getQpLF();
          if( ( bLI_Top ^ bLI_Bot ) && LFM_DEFAULT_FILTER != m_eLFMode )
          {
            bAboveIntra   = true;
            bFilterTopOnly= bLI_Top;
            bFilterBotOnly= bLI_Bot;
          }
          else
          {
            bAboveIntra   = bLI_Top || bLI_Bot;
          }
        }
      }
      filterBlockEdge   = ( LFM_DEFAULT_FILTER == m_eLFMode || bAboveIntra );
    }
    if( filterBlockEdge )
    {
      filterBlockEdge   = ! ( rcDFP.getDisableDeblockingFilterIdc() == 1 ||
                            ( rcDFP.getDisableDeblockingFilterIdc() == 2 && ! rcMbDataAccess.isAvailableAbove() ) );
    }
    if( ! m_bHorMixedMode )
    {
      if( filterBlockEdge )
      {
        Int iQp           = ( iAboveQp + iCurrQp + 1) >> 1;
        Int iIndexB       = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);
        Int iStridex2     = 2*iStride;
        Int iD            = ( abs( (Int)pPelLum[2-iStridex2] -   (Int)pPelLum[2-iStride]                                    )
                            + abs( (Int)pPelLum[2]           - ( (Int)pPelLum[2+iStride] << 1 ) + (Int)pPelLum[2+iStridex2] )
                            + abs( (Int)pPelLum[5-iStridex2] -   (Int)pPelLum[5-iStride]                                    )
                            + abs( (Int)pPelLum[5]           - ( (Int)pPelLum[5+iStride] << 1 ) + (Int)pPelLum[5+iStridex2] ) );
        filterBlockEdge   = ( iD < g_aucBetaTab_H241RCDO [ iIndexB ] );
      }
      if( filterBlockEdge )
      {
        filterBlockEdge   = ( rcMbDataAccess.getMbDataCurr ().isIntra() || 
                              rcMbDataAccess.getMbDataAbove().isIntra() );
        if( ! filterBlockEdge && m_pcHighpassYuvBuffer )
        {
          filterBlockEdge = ( m_pcHighpassYuvBuffer->isCurr4x4BlkNotZero ( cIdx ) ||
                              m_pcHighpassYuvBuffer->isCurr4x4BlkNotZero ( cIdx + HOR_OFFSET_H241RCDO ) ||
                              m_pcHighpassYuvBuffer->isAbove4x4BlkNotZero( cIdx + ABOVE_MB_ABOVE_NEIGHBOUR ) ||
                              m_pcHighpassYuvBuffer->isAbove4x4BlkNotZero( cIdx + ABOVE_MB_ABOVE_NEIGHBOUR + HOR_OFFSET_H241RCDO ) );
        }
        if( ! filterBlockEdge )
        {
          filterBlockEdge = ( rcMbDataAccess.getMbDataCurr ().is4x4BlkCoded( cIdx ) ||
                              rcMbDataAccess.getMbDataCurr ().is4x4BlkCoded( cIdx + HOR_OFFSET_H241RCDO ) ||
                              rcMbDataAccess.getMbDataAbove().is4x4BlkCoded( cIdx + ABOVE_MB_ABOVE_NEIGHBOUR ) ||
                              rcMbDataAccess.getMbDataAbove().is4x4BlkCoded( cIdx + ABOVE_MB_ABOVE_NEIGHBOUR + HOR_OFFSET_H241RCDO ) );
        }
        if( ! filterBlockEdge )
        {
          ROF( pcMbDataAccessMot );
          if ( pcMbDataAccessMot->getMbDataCurr().isInterPMb() && pcMbDataAccessMot->getMbDataAbove().isInterPMb() )
          {
            if( pcRefFrameList0 )
              CheckMVandRefVal= xCheckMvDataP_RefIdx( pcMbDataAccessMot->getMbDataCurr(), cIdx, pcMbDataAccessMot->getMbDataAbove(), cIdx + ABOVE_MB_ABOVE_NEIGHBOUR, sHorMvThr, sVerMvThr, *pcRefFrameList0, *pcRefFrameList1 );
            else
              CheckMVandRefVal= xCheckMvDataP       ( pcMbDataAccessMot->getMbDataCurr(), cIdx, pcMbDataAccessMot->getMbDataAbove(), cIdx + ABOVE_MB_ABOVE_NEIGHBOUR, sHorMvThr, sVerMvThr );
          }
          else
          {
            if( pcRefFrameList0 && pcRefFrameList1 )
              CheckMVandRefVal= xCheckMvDataB_RefIdx( pcMbDataAccessMot->getMbDataCurr(), cIdx, pcMbDataAccessMot->getMbDataAbove(), cIdx + ABOVE_MB_ABOVE_NEIGHBOUR, sHorMvThr, sVerMvThr, *pcRefFrameList0, *pcRefFrameList1 );
            else
              CheckMVandRefVal= xCheckMvDataB       ( pcMbDataAccessMot->getMbDataCurr(), cIdx, pcMbDataAccessMot->getMbDataAbove(), cIdx + ABOVE_MB_ABOVE_NEIGHBOUR, sHorMvThr, sVerMvThr );
          }
          filterBlockEdge   = ( CheckMVandRefVal > 0 );
        }
      }
    }

    if( filterBlockEdge )
    {
      if( m_bHorMixedMode && ! rcMbDataAccess.getMbData().getFieldFlag() )
      {
        if( ! bFilterBotOnly )
        {
          Int     iQp           = ( iAboveQpTop + iCurrQp + 1) >> 1;
          Int     iIndexA       = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
          Int     iIndexB       = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);
          xFilter_H241RCDO( pPelLum,   iStride, iIndexA, iIndexB, 0, true );
          xFilter_H241RCDO( pPelLum+2, iStride, iIndexA, iIndexB, 0, true );
          xFilter_H241RCDO( pPelLum+4, iStride, iIndexA, iIndexB, 0, true );
          xFilter_H241RCDO( pPelLum+6, iStride, iIndexA, iIndexB, 0, true );
        }
        if( ! bFilterTopOnly )
        {
          Int     iQp           = ( iAboveQpBot + iCurrQp + 1) >> 1;
          Int     iIndexA       = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
          Int     iIndexB       = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);
          xFilter_H241RCDO( pPelLum+1, iStride, iIndexA, iIndexB, 0, true );
          xFilter_H241RCDO( pPelLum+3, iStride, iIndexA, iIndexB, 0, true );
          xFilter_H241RCDO( pPelLum+5, iStride, iIndexA, iIndexB, 0, true );
          xFilter_H241RCDO( pPelLum+7, iStride, iIndexA, iIndexB, 0, true );
        }
      }
      else
      {
        Int     iQp           = ( iAboveQp + iCurrQp + 1) >> 1;
        Int     iIndexA       = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
        Int     iIndexB       = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);
        xFilter_H241RCDO( pPelLum,   iStride, iIndexA, iIndexB, 0, true );
        xFilter_H241RCDO( pPelLum+1, iStride, iIndexA, iIndexB, 0, true );
        xFilter_H241RCDO( pPelLum+2, iStride, iIndexA, iIndexB, 0, true );
        xFilter_H241RCDO( pPelLum+3, iStride, iIndexA, iIndexB, 0, true );
        xFilter_H241RCDO( pPelLum+4, iStride, iIndexA, iIndexB, 0, true );
        xFilter_H241RCDO( pPelLum+5, iStride, iIndexA, iIndexB, 0, true );
        xFilter_H241RCDO( pPelLum+6, iStride, iIndexA, iIndexB, 0, true );
        xFilter_H241RCDO( pPelLum+7, iStride, iIndexA, iIndexB, 0, true );
      }
    }

    pPelLum += 8; //for the next 8x8 vertical
    if( xBlk == 0 )
    {
      cIdx = cIdx + 2; //value order: 0,2
    }
  } //end of the loop


  //Inside of the macroblock
  pPelLum += 4*iStride - 16; 
  Int iIndexA  = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iCurrQp, 0, 51);
  Int iIndexB  = gClipMinMax( rcDFP.getSliceBetaOffset()    + iCurrQp, 0, 51);
  for( yBlk = 1; yBlk < 4; yBlk++ )
  {
    cIdx = cIdx + 2;
    for( xBlk = 0; xBlk <= 8; xBlk += 8 )
    {
      filterBlockEdge     = ( yBlk == 2 || ! b8x8 );
      if( filterBlockEdge )
      {
        filterBlockEdge   = ! ( rcDFP.getDisableDeblockingFilterIdc() == 1 );
      }
      if( filterBlockEdge )
      {
        Int iStridex2     = 2*iStride;
        Int iStridex3     = 3*iStride;
        Int iD            = ( abs( (Int)pPelLum[2-iStridex3] - ( (Int)pPelLum[2-iStridex2] << 1 ) + (Int)pPelLum[2-iStride  ] )
                            + abs( (Int)pPelLum[2          ] - ( (Int)pPelLum[2+iStride  ] << 1 ) + (Int)pPelLum[2+iStridex2] )
                            + abs( (Int)pPelLum[5-iStridex3] - ( (Int)pPelLum[5-iStridex2] << 1 ) + (Int)pPelLum[5-iStride  ] )
                            + abs( (Int)pPelLum[5          ] - ( (Int)pPelLum[5+iStride  ] << 1 ) + (Int)pPelLum[5+iStridex2] ) );
        filterBlockEdge   = ( iD < g_aucBetaTab_H241RCDO [ iIndexB ] );
      }
      if( filterBlockEdge )
      {
        filterBlockEdge   = rcMbDataAccess.getMbDataCurr().isIntra();

        if( ! filterBlockEdge && m_pcHighpassYuvBuffer )
        {
          filterBlockEdge = ( m_pcHighpassYuvBuffer->isCurr4x4BlkNotZero( cIdx ) ||
                              m_pcHighpassYuvBuffer->isCurr4x4BlkNotZero( cIdx + HOR_OFFSET_H241RCDO ) ||
                              m_pcHighpassYuvBuffer->isCurr4x4BlkNotZero( cIdx + CURR_MB_ABOVE_NEIGHBOUR ) ||
                              m_pcHighpassYuvBuffer->isCurr4x4BlkNotZero( cIdx + CURR_MB_ABOVE_NEIGHBOUR + HOR_OFFSET_H241RCDO ) );
        }
        if( ! filterBlockEdge )
        {
          filterBlockEdge = ( rcMbDataAccess.getMbDataCurr().is4x4BlkCoded( cIdx ) ||
                              rcMbDataAccess.getMbDataCurr().is4x4BlkCoded( cIdx + HOR_OFFSET_H241RCDO ) ||
                              rcMbDataAccess.getMbDataCurr().is4x4BlkCoded( cIdx + CURR_MB_ABOVE_NEIGHBOUR ) ||
                              rcMbDataAccess.getMbDataCurr().is4x4BlkCoded( cIdx + CURR_MB_ABOVE_NEIGHBOUR + HOR_OFFSET_H241RCDO ) );
        }
        if( ! filterBlockEdge )
        {
          ROF( pcMbDataAccessMot );
          if ( pcMbDataAccessMot->getMbDataCurr().isInterPMb() )
          {
            if( pcRefFrameList0 )
              CheckMVandRefVal= xCheckMvDataP_RefIdx( pcMbDataAccessMot->getMbDataCurr(), cIdx, pcMbDataAccessMot->getMbDataCurr(), cIdx + CURR_MB_ABOVE_NEIGHBOUR, sHorMvThr, sVerMvThr, *pcRefFrameList0, *pcRefFrameList1 );
            else
              CheckMVandRefVal= xCheckMvDataP       ( pcMbDataAccessMot->getMbDataCurr(), cIdx, pcMbDataAccessMot->getMbDataCurr(), cIdx + CURR_MB_ABOVE_NEIGHBOUR, sHorMvThr, sVerMvThr );
          }
          else
          {
            if( pcRefFrameList0 && pcRefFrameList1 )
              CheckMVandRefVal= xCheckMvDataB_RefIdx( pcMbDataAccessMot->getMbDataCurr(), cIdx, pcMbDataAccessMot->getMbDataCurr(), cIdx + CURR_MB_ABOVE_NEIGHBOUR, sHorMvThr, sVerMvThr, *pcRefFrameList0, *pcRefFrameList1 );
            else
              CheckMVandRefVal= xCheckMvDataB       ( pcMbDataAccessMot->getMbDataCurr(), cIdx, pcMbDataAccessMot->getMbDataCurr(), cIdx + CURR_MB_ABOVE_NEIGHBOUR, sHorMvThr, sVerMvThr );
          }
          filterBlockEdge = ( CheckMVandRefVal > 0 );
        }
      }

      if( filterBlockEdge )
      {
        xFilter_H241RCDO( pPelLum,   iStride, iIndexA, iIndexB, 1, true );
        xFilter_H241RCDO( pPelLum+1, iStride, iIndexA, iIndexB, 1, true );
        xFilter_H241RCDO( pPelLum+2, iStride, iIndexA, iIndexB, 1, true );
        xFilter_H241RCDO( pPelLum+3, iStride, iIndexA, iIndexB, 1, true );
        xFilter_H241RCDO( pPelLum+4, iStride, iIndexA, iIndexB, 1, true );
        xFilter_H241RCDO( pPelLum+5, iStride, iIndexA, iIndexB, 1, true );
        xFilter_H241RCDO( pPelLum+6, iStride, iIndexA, iIndexB, 1, true );
        xFilter_H241RCDO( pPelLum+7, iStride, iIndexA, iIndexB, 1, true );
      }

      pPelLum += 8; //for the next 8x8 vertical
      if( xBlk == 0 )
      {
        cIdx = cIdx + 2; 
      }
    } //end of the 1st loop

    pPelLum += (4*iStride - 16);  //back to the original position
  } //end of the 2nd loop

  return Err::m_nOK;
}



__inline ErrVal LoopFilter::xChromaHorFiltering_H241RCDO ( const MbDataAccess& rcMbDataAccess, const DFP& rcDFP, IntYuvPicBuffer* pcYuvBuffer )
{
  Int   iCurrQp   = rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.getMbDataCurr().getQpLF() );
  Int   iStride   = pcYuvBuffer->getCStride();
  XPel* pPelCb    = pcYuvBuffer->getMbCbAddr();
  XPel* pPelCr    = pcYuvBuffer->getMbCrAddr();
  Bool  filterBlockEdge, bFilterTopOnly, bFilterBotOnly;
  Int   xBlk, iAboveQp = 0, iAboveQpTop = 0, iAboveQpBot = 0;

  for( xBlk = 0; xBlk <= 4; xBlk += 4 )
  {
    filterBlockEdge   = rcMbDataAccess.isAboveMbExisting();
    bFilterTopOnly    = false;
    bFilterBotOnly    = false;
    if( filterBlockEdge )
    {
      Bool bAboveIntra  = ( rcMbDataAccess.getMbData().getFieldFlag() ? rcMbDataAccess.getMbDataAboveAbove().isIntra() : rcMbDataAccess.getMbDataAbove().isIntra() );
      iAboveQp          = ( rcMbDataAccess.getMbData().getFieldFlag() ? rcMbDataAccess.getMbDataAboveAbove().getQpLF() : rcMbDataAccess.getMbDataAbove().getQpLF() );
      if( m_bHorMixedMode )
      {
        if      ( rcMbDataAccess.getMbData().getFieldFlag() && rcMbDataAccess.isTopMb() )
        {
          bAboveIntra   = rcMbDataAccess.getMbDataAbove().isIntra();
          iAboveQp      = rcMbDataAccess.getMbDataAbove().getQpLF();
        }
        else if ( rcMbDataAccess.getMbData().getFieldFlag() )
        {
          bAboveIntra   = rcMbDataAccess.getMbDataAboveAbove().isIntra();
          iAboveQp      = rcMbDataAccess.getMbDataAboveAbove().getQpLF();
        }
        else
        {
          AOF( rcMbDataAccess.isTopMb() );
          Bool bLI_Top    = rcMbDataAccess.getMbDataAboveAbove().isIntra();
          Bool bLI_Bot    = rcMbDataAccess.getMbDataAbove     ().isIntra();
          iAboveQpTop     = rcMbDataAccess.getMbDataAboveAbove().getQpLF();
          iAboveQpBot     = rcMbDataAccess.getMbDataAbove     ().getQpLF();
          if( ( bLI_Top ^ bLI_Bot ) && LFM_DEFAULT_FILTER != m_eLFMode )
          {
            bAboveIntra   = true;
            bFilterTopOnly= bLI_Top;
            bFilterBotOnly= bLI_Bot;
          }
          else
          {
            bAboveIntra   = bLI_Top || bLI_Bot;
          }
        }
      }
      filterBlockEdge = ( LFM_DEFAULT_FILTER == m_eLFMode || bAboveIntra );
    }
    if( filterBlockEdge )
    {
      filterBlockEdge = ! ( rcDFP.getDisableDeblockingFilterIdc() == 1 ||
                          ( rcDFP.getDisableDeblockingFilterIdc() == 2 && ! rcMbDataAccess.isAvailableAbove() ) );
    }
    if( filterBlockEdge && ! m_bHorMixedMode )
    {
      filterBlockEdge = ( rcMbDataAccess.getMbDataCurr ().isIntra() || 
                          rcMbDataAccess.getMbDataAbove().isIntra() );
    }

    if( filterBlockEdge )
    {
      if( m_bHorMixedMode && ! rcMbDataAccess.getMbData().getFieldFlag() )
      {
        if( ! bFilterBotOnly )
        {
          Int   iAQp      = rcMbDataAccess.getSH().getChromaQp( iAboveQpTop );
          Int   iQp       = ( iCurrQp + iAQp + 1 ) >> 1;
          Int   iIndexA   = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
          Int   iIndexB   = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);
          xFilter_H241RCDO( pPelCb,   iStride, iIndexA, iIndexB, 0, false );
          xFilter_H241RCDO( pPelCb+2, iStride, iIndexA, iIndexB, 0, false );
          xFilter_H241RCDO( pPelCr,   iStride, iIndexA, iIndexB, 0, false );
          xFilter_H241RCDO( pPelCr+2, iStride, iIndexA, iIndexB, 0, false );
        }
        if( ! bFilterTopOnly )
        {
          Int   iAQp      = rcMbDataAccess.getSH().getChromaQp( iAboveQpBot );
          Int   iQp       = ( iCurrQp + iAQp + 1 ) >> 1;
          Int   iIndexA   = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
          Int   iIndexB   = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);
          xFilter_H241RCDO( pPelCb+1, iStride, iIndexA, iIndexB, 0, false );
          xFilter_H241RCDO( pPelCb+3, iStride, iIndexA, iIndexB, 0, false );
          xFilter_H241RCDO( pPelCr+1, iStride, iIndexA, iIndexB, 0, false );
          xFilter_H241RCDO( pPelCr+3, iStride, iIndexA, iIndexB, 0, false );
        }
      }
      else
      {
        Int   iAQp      = rcMbDataAccess.getSH().getChromaQp( iAboveQp );
        Int   iQp       = ( iCurrQp + iAQp + 1 ) >> 1;
        Int   iIndexA   = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
        Int   iIndexB   = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);
        xFilter_H241RCDO( pPelCb,   iStride, iIndexA, iIndexB, 0, false );
        xFilter_H241RCDO( pPelCb+1, iStride, iIndexA, iIndexB, 0, false );
        xFilter_H241RCDO( pPelCb+2, iStride, iIndexA, iIndexB, 0, false );
        xFilter_H241RCDO( pPelCb+3, iStride, iIndexA, iIndexB, 0, false );
        xFilter_H241RCDO( pPelCr,   iStride, iIndexA, iIndexB, 0, false );
        xFilter_H241RCDO( pPelCr+1, iStride, iIndexA, iIndexB, 0, false );
        xFilter_H241RCDO( pPelCr+2, iStride, iIndexA, iIndexB, 0, false );
        xFilter_H241RCDO( pPelCr+3, iStride, iIndexA, iIndexB, 0, false );
      }
    }

    pPelCb += 4; 
    pPelCr += 4; 
  }

  //Inside of the macroblock
  pPelCb += 4*iStride - 8;
  pPelCr += 4*iStride - 8;
  Int iIndexA = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iCurrQp, 0, 51);
  Int iIndexB = gClipMinMax( rcDFP.getSliceBetaOffset()    + iCurrQp, 0, 51);
  for( xBlk = 0; xBlk <= 4; xBlk += 4 )
  {
    filterBlockEdge   = ! ( rcDFP.getDisableDeblockingFilterIdc() == 1 );
    if( filterBlockEdge )
    {
      filterBlockEdge = rcMbDataAccess.getMbDataCurr().isIntra();
    }

    if( filterBlockEdge )
    {
      xFilter_H241RCDO( pPelCb,   iStride, iIndexA, iIndexB, 1, false );
      xFilter_H241RCDO( pPelCb+1, iStride, iIndexA, iIndexB, 1, false );
      xFilter_H241RCDO( pPelCb+2, iStride, iIndexA, iIndexB, 1, false );
      xFilter_H241RCDO( pPelCb+3, iStride, iIndexA, iIndexB, 1, false );
      xFilter_H241RCDO( pPelCr,   iStride, iIndexA, iIndexB, 1, false );
      xFilter_H241RCDO( pPelCr+1, iStride, iIndexA, iIndexB, 1, false );
      xFilter_H241RCDO( pPelCr+2, iStride, iIndexA, iIndexB, 1, false );
      xFilter_H241RCDO( pPelCr+3, iStride, iIndexA, iIndexB, 1, false );
    }

    pPelCb += 4; //for the next 4x4 vertical
    pPelCr += 4; //for the next 4x4 vertical
  } //end of the 1st loop

  return Err::m_nOK;
}



__inline ErrVal LoopFilter::xLumaVerFiltering( const MbDataAccess& rcMbDataAccess, const DFP& rcDFP, YuvPicBuffer* pcYuvBuffer )
{
  Int   iCurrQp = rcMbDataAccess.getMbDataCurr().getQpLF();
  Int   iStride = pcYuvBuffer->getLStride();
  Pel*  pPelLum = pcYuvBuffer->getMbLumAddr();

  //===== filtering of left macroblock edge =====
  if( ! m_bVerMixedMode )
  {
    //-----  curr == FRM && left == FRM  or  curr == FLD && left == FLD  -----
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
  }
  else
  {
    Int iLeftQpTop = ( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataLeft     ().getQpLF() : rcMbDataAccess.getMbDataAboveLeft().getQpLF() );
    Int iLeftQpBot = ( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataBelowLeft().getQpLF() : rcMbDataAccess.getMbDataLeft     ().getQpLF() );
    Int iQpTop     = ( iLeftQpTop + iCurrQp + 1) >> 1;
    Int iQpBot     = ( iLeftQpBot + iCurrQp + 1) >> 1;

    Int iIndexATop = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQpTop, 0, 51);
    Int iIndexABot = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQpBot, 0, 51);
    Int iIndexBTop = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQpTop, 0, 51);
    Int iIndexBBot = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQpBot, 0, 51);

    if( ! rcMbDataAccess.getMbData().getFieldFlag() )
    {
      //-----  curr == FRM && left == FLD  -----
      for( Int yBlk = 0; yBlk < 4; yBlk++)
      {
        const UChar ucBsTop = m_aaaucBs[VER][0][yBlk];
        const UChar ucBsBot = m_aucBsVerBot[yBlk];
        if( 0 != ucBsTop )
        {
          xFilter( pPelLum,           1, iIndexATop, iIndexBTop, ucBsTop, true );
          xFilter( pPelLum+2*iStride, 1, iIndexATop, iIndexBTop, ucBsTop, true );
        }
        if( 0 != ucBsBot )
        {
          xFilter( pPelLum+  iStride, 1, iIndexABot, iIndexBBot, ucBsBot, true );
          xFilter( pPelLum+3*iStride, 1, iIndexABot, iIndexBBot, ucBsBot, true );
        }
        pPelLum += 4*iStride;
      }
    }
    else
    {
      //-----  curr == FLD && left == FRM  -----
      Int yBlk;
      for( yBlk = 0; yBlk < 2; yBlk++)
      {
        const UChar ucBsTop = m_aaaucBs[VER][0][yBlk];
        const UChar ucBsBot = m_aucBsVerBot[yBlk];
        if( 0 != ucBsTop )
        {
          xFilter( pPelLum,           1, iIndexATop, iIndexBTop, ucBsTop, true );
          xFilter( pPelLum+  iStride, 1, iIndexATop, iIndexBTop, ucBsTop, true );
        }
        if( 0 != ucBsBot )
        {
          xFilter( pPelLum+2*iStride, 1, iIndexATop, iIndexBTop, ucBsBot, true );
          xFilter( pPelLum+3*iStride, 1, iIndexATop, iIndexBTop, ucBsBot, true );
        }
        pPelLum += 4*iStride;
      }
      for( yBlk = 2; yBlk < 4; yBlk++)
      {
        const UChar ucBsTop = m_aaaucBs[VER][0][yBlk];
        const UChar ucBsBot = m_aucBsVerBot[yBlk];
        if( 0 != ucBsTop )
        {
          xFilter( pPelLum,           1, iIndexABot, iIndexBBot, ucBsTop, true );
          xFilter( pPelLum+  iStride, 1, iIndexABot, iIndexBBot, ucBsTop, true );
        }
        if( 0 != ucBsBot )
        {
          xFilter( pPelLum+2*iStride, 1, iIndexABot, iIndexBBot, ucBsBot, true );
          xFilter( pPelLum+3*iStride, 1, iIndexABot, iIndexBBot, ucBsBot, true );
        }
        pPelLum += 4*iStride;
      }
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




__inline ErrVal LoopFilter::xLumaHorFiltering( const MbDataAccess& rcMbDataAccess, const DFP& rcDFP, YuvPicBuffer* pcYuvBuffer )
{
  Int   iCurrQp = rcMbDataAccess.getMbDataCurr().getQpLF();
  Int   iStride = pcYuvBuffer->getLStride();
  Pel*  pPelLum = pcYuvBuffer->getMbLumAddr();

  //===== filtering of upper macroblock edge =====
  if( ! m_bHorMixedMode )
  {
    //-----  any other combination than curr = FRM, above = FLD  -----
    Int iAboveQp  = rcMbDataAccess.getMbData().getFieldFlag() && (!rcMbDataAccess.isTopMb()||rcMbDataAccess.getMbDataAboveAbove().getFieldFlag()) ?
      rcMbDataAccess.getMbDataAboveAbove().getQpLF():
    rcMbDataAccess.getMbDataAbove().getQpLF();
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
  else
  {
    //-----  curr = FRM, above = FLD  -----
    AOT_DBG( ! rcMbDataAccess.isTopMb() );
    AOT_DBG( ! rcMbDataAccess.isAboveMbExisting() );

    //===== top field filtering =====
    {
      Pel*  pPelTop     = pcYuvBuffer->getMbLumAddr();
      Int   iTopStride  = pcYuvBuffer->getLStride()*2;

      Int   iAboveQp    = rcMbDataAccess.getMbDataAboveAbove().getQpLF();
      Int   iQp         = ( iAboveQp + iCurrQp + 1) >> 1;
      Int   iIndexA     = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
      Int   iIndexB     = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);

      for( Int xBlk = 0; xBlk < 4; xBlk++)
      {
        const UChar ucBs = m_aucBsHorTop[xBlk];
        if( 0 != ucBs )
        {
          xFilter( pPelTop,   iTopStride, iIndexA, iIndexB, ucBs, true );
          xFilter( pPelTop+1, iTopStride, iIndexA, iIndexB, ucBs, true );
          xFilter( pPelTop+2, iTopStride, iIndexA, iIndexB, ucBs, true );
          xFilter( pPelTop+3, iTopStride, iIndexA, iIndexB, ucBs, true );
        }
        pPelTop += 4;
      }
    }
    //===== bottom field filtering =====
    {
      Pel*  pPelBot     = pcYuvBuffer->getMbLumAddr()+pcYuvBuffer->getLStride();
      Int   iBotStride  = pcYuvBuffer->getLStride()*2;

      Int   iAboveQp    = rcMbDataAccess.getMbDataAbove().getQpLF();
      Int   iQp         = ( iAboveQp + iCurrQp + 1) >> 1;
      Int   iIndexA     = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
      Int   iIndexB     = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);

      for( Int xBlk = 0; xBlk < 4; xBlk++)
      {
        const UChar ucBs = m_aaaucBs[HOR][xBlk][0];
        if( 0 != ucBs )
        {
          xFilter( pPelBot,   iBotStride, iIndexA, iIndexB, ucBs, true );
          xFilter( pPelBot+1, iBotStride, iIndexA, iIndexB, ucBs, true );
          xFilter( pPelBot+2, iBotStride, iIndexA, iIndexB, ucBs, true );
          xFilter( pPelBot+3, iBotStride, iIndexA, iIndexB, ucBs, true );
        }
        pPelBot += 4;
      }
    }
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
  if( ! m_bHorMixedMode )
  {
    //-----  any other combination than curr = FRM, above = FLD  -----
    Int iAboveQp  = rcMbDataAccess.getMbData().getFieldFlag() && (!rcMbDataAccess.isTopMb()||rcMbDataAccess.getMbDataAboveAbove().getFieldFlag()) ?
      rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.getMbDataAboveAbove().getQpLF()):
      rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.getMbDataAbove().getQpLF());
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
  else
  {
    //-----  curr = FRM, above = FLD  -----
    AOT_DBG( ! rcMbDataAccess.isTopMb() );

    //===== top field filtering =====
    {
      /*th
      YuvPicBuffer* pcYuvFieldBuffer = m_pcRecFrameUnit->getTopField().getFullPelYuvBuffer();
      Pel*  pPelField     = bCb ? pcYuvFieldBuffer->getMbCbAddr() : pcYuvFieldBuffer->getMbCrAddr();
      Int   iFieldStride  = pcYuvFieldBuffer->getCStride();
      */
      Pel*  pPelFieldCb   = pcYuvBuffer->getMbCbAddr();
      Pel*  pPelFieldCr   = pcYuvBuffer->getMbCrAddr();
      Int   iFieldStride  = pcYuvBuffer->getCStride() * 2;

      Int   iAboveQp      = rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.getMbDataAboveAbove().getQpLF() );
      Int   iQp           = ( iAboveQp + iCurrQp + 1) >> 1;
      Int   iIndexA       = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
      Int   iIndexB       = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);

      for( Int xBlk = 0; xBlk < 4; xBlk++)
      {
        const UChar ucBs = m_aucBsHorTop[xBlk];
        if( 0 != ucBs )
        {
          xFilter( pPelFieldCb,   iFieldStride, iIndexA, iIndexB, ucBs, false );
          xFilter( pPelFieldCb+1, iFieldStride, iIndexA, iIndexB, ucBs, false );
          xFilter( pPelFieldCr,   iFieldStride, iIndexA, iIndexB, ucBs, false );
          xFilter( pPelFieldCr+1, iFieldStride, iIndexA, iIndexB, ucBs, false );
        }
        pPelFieldCb   += 2;
        pPelFieldCr   += 2;
      }
    }
    //===== bottom field filtering =====
    {
      /*th
      YuvPicBuffer* pcYuvFieldBuffer = m_pcRecFrameUnit->getBotField().getFullPelYuvBuffer();
      Pel*  pPelField   = bCb ? pcYuvFieldBuffer->getMbCbAddr() : pcYuvFieldBuffer->getMbCrAddr();
      Int   iFieldStride  = pcYuvFieldBuffer->getCStride();
      */
      Pel*  pPelFieldCb   = pcYuvBuffer->getMbCbAddr()+pcYuvBuffer->getCStride();
      Pel*  pPelFieldCr   = pcYuvBuffer->getMbCrAddr()+pcYuvBuffer->getCStride();
      Int   iFieldStride  = pcYuvBuffer->getCStride() * 2;

      Int   iAboveQp      = rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.getMbDataAbove().getQpLF() );
      Int   iQp           = ( iAboveQp + iCurrQp + 1) >> 1;
      Int   iIndexA       = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
      Int   iIndexB       = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);

      for( Int xBlk = 0; xBlk < 4; xBlk++)
      {
        const UChar ucBs = m_aaaucBs[HOR][xBlk][0];
        if( 0 != ucBs )
        {
          xFilter( pPelFieldCb,   iFieldStride, iIndexA, iIndexB, ucBs, false );
          xFilter( pPelFieldCb+1, iFieldStride, iIndexA, iIndexB, ucBs, false );
          xFilter( pPelFieldCr,   iFieldStride, iIndexA, iIndexB, ucBs, false );
          xFilter( pPelFieldCr+1, iFieldStride, iIndexA, iIndexB, ucBs, false );
        }
        pPelFieldCb   += 2;
        pPelFieldCr   += 2;
      }
    }
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
  if( ! m_bVerMixedMode )
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
  else
  {
    Int iLeftQpTop = rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataLeft     ().getQpLF() : rcMbDataAccess.getMbDataAboveLeft().getQpLF() );
    Int iLeftQpBot = rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataBelowLeft().getQpLF() : rcMbDataAccess.getMbDataLeft     ().getQpLF() );
    Int iQpTop     = ( iLeftQpTop + iCurrQp + 1) >> 1;
    Int iQpBot     = ( iLeftQpBot + iCurrQp + 1) >> 1;
    Int iIndexATop = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQpTop, 0, 51);
    Int iIndexABot = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQpBot, 0, 51);
    Int iIndexBTop = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQpTop, 0, 51);
    Int iIndexBBot = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQpBot, 0, 51);

    if( ! rcMbDataAccess.getMbData().getFieldFlag() )
    {
      //-----  curr == FRM && left == FLD  -----
      for( Int yBlk = 0; yBlk < 4; yBlk++)
      {
        const UChar ucBsTop = m_aaaucBs[VER][0][yBlk];
        const UChar ucBsBot = m_aucBsVerBot[yBlk];
        if( 0 != ucBsTop )
        {
          xFilter( pPelCb, 1, iIndexATop, iIndexBTop, ucBsTop, false );
          xFilter( pPelCr, 1, iIndexATop, iIndexBTop, ucBsTop, false );
        }
        if( 0 != ucBsBot )
        {
          xFilter( pPelCb+iStride, 1, iIndexABot, iIndexBBot, ucBsBot, false );
          xFilter( pPelCr+iStride, 1, iIndexABot, iIndexBBot, ucBsBot, false );
        }
        pPelCb += 2*iStride;
        pPelCr += 2*iStride;
      }
    }
    else
    {
      //-----  curr == FLD && left == FRM  -----
      Int yBlk;
      for( yBlk = 0; yBlk < 2; yBlk++)
      {
        const UChar ucBsTop = m_aaaucBs[VER][0][yBlk];
        const UChar ucBsBot = m_aucBsVerBot[yBlk];
        if( 0 != ucBsTop )
        {
          xFilter( pPelCb, 1, iIndexATop, iIndexBTop, ucBsTop, false );
          xFilter( pPelCr, 1, iIndexATop, iIndexBTop, ucBsTop, false );
        }
        if( 0 != ucBsBot )
        {
          xFilter( pPelCb+iStride, 1, iIndexATop, iIndexBTop, ucBsBot, false );
          xFilter( pPelCr+iStride, 1, iIndexATop, iIndexBTop, ucBsBot, false );
        }
        pPelCb   += 2*iStride;
        pPelCr   += 2*iStride;
      }

      for( yBlk = 2; yBlk < 4; yBlk++)
      {
        const UChar ucBsTop = m_aaaucBs[VER][0][yBlk];
        const UChar ucBsBot = m_aucBsVerBot[yBlk];
        if( 0 != ucBsTop )
        {
          xFilter( pPelCb, 1, iIndexABot, iIndexBBot, ucBsTop, false );
          xFilter( pPelCr, 1, iIndexABot, iIndexBBot, ucBsTop, false );
        }
        if( 0 != ucBsBot )
        {
          xFilter( pPelCb+iStride, 1, iIndexABot, iIndexBBot, ucBsBot, false );
          xFilter( pPelCr+iStride, 1, iIndexABot, iIndexBBot, ucBsBot, false );
        }
        pPelCb += 2*iStride;
        pPelCr += 2*iStride;
      }
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
  Short         sVerMvThr     = ( FRAME == rcMbDataAccess.getMbPicType() ? 4 : 2 );

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
  ROTRS( (iFilterIdc == 2 || iFilterIdc == 4) && ! rcMbDataAccess.isAvailableLeft(), 0 ); //V032 of FSL
  ROTRS( ! rcMbDataAccess.isLeftMbExisting(),                   0 );

  if( ! m_bVerMixedMode )
  {
    const MbData& rcMbDataLeft = rcMbDataAccess.getMbDataLeft();
    ROTRS( LFM_DEFAULT_FILTER != m_eLFMode && rcMbDataCurr.isIntra() ^ rcMbDataLeft.isIntra(), 0 );
  
    ROTRS( rcMbDataCurr.isIntra(), 4 );
    ROTRS( rcMbDataLeft.isIntra(), 4 );
  
    if( m_pcHighpassYuvBuffer )
    {
// TMM_INTERLACE{    
        ROTRS( rcMbDataCurr.is4x4BlkResidual( cIdx                          ), 2 );
        ROTRS( rcMbDataLeft.is4x4BlkResidual( cIdx + LEFT_MB_LEFT_NEIGHBOUR ), 2 );
// TMM_INTERLACE}
    }
    else
    {
      ROTRS( rcMbDataCurr.is4x4BlkCoded( cIdx                          ), 2 );
      ROTRS( rcMbDataLeft.is4x4BlkCoded( cIdx + LEFT_MB_LEFT_NEIGHBOUR ), 2 );
    }
  
    return xCheckMvDataB  ( rcMbDataCurr, cIdx, rcMbDataLeft, cIdx + LEFT_MB_LEFT_NEIGHBOUR, sHorMvThr, sVerMvThr );
  }


  if( FRAME == rcMbDataAccess.getMbPicType() )
  {
    // mixed mode, current macroblock is a frame macroblock
    const MbData& rcMbDataLeft = (   rcMbDataAccess.isTopMb() &&   m_bAddEdge ? rcMbDataAccess.getMbDataBelowLeft() :
    ! rcMbDataAccess.isTopMb() && ! m_bAddEdge ? rcMbDataAccess.getMbDataAboveLeft() : rcMbDataAccess.getMbDataLeft() );

    //th this is not correct for mbaff case but I don't know if this feature is still in use
    ROTRS( LFM_DEFAULT_FILTER != m_eLFMode && rcMbDataCurr.isIntra() ^ rcMbDataLeft.isIntra(), 0 );

    ROTRS( rcMbDataCurr.isIntra(),  4 );
    ROTRS( rcMbDataLeft.isIntra(),  4 );

    B4x4Idx cIdxLeft = B4x4Idx( rcMbDataAccess.isTopMb() ? ( cIdx < 8 ? 3 : 7 ) : ( cIdx < 8 ? 11 : 15 ) );

    ROTRS( rcMbDataCurr.is4x4BlkCoded( cIdx     ), 2 );
    ROTRS( rcMbDataLeft.is4x4BlkCoded( cIdxLeft ), 2 );

    return 1;
}

  // mixed mode, current macroblock is a field macroblock
  const MbData& rcMbDataLeft = (   rcMbDataAccess.isTopMb() && cIdx > 7 ? rcMbDataAccess.getMbDataBelowLeft() :
  ! rcMbDataAccess.isTopMb() && cIdx < 8 ? rcMbDataAccess.getMbDataAboveLeft() : rcMbDataAccess.getMbDataLeft() );

  //th this is not correct for mbaff case but I don't know if this feature is still in use
  ROTRS( LFM_DEFAULT_FILTER != m_eLFMode && rcMbDataCurr.isIntra() ^ rcMbDataLeft.isIntra(), 0 );

  ROTRS( rcMbDataCurr.isIntra(),  4 );
  ROTRS( rcMbDataLeft.isIntra(),  4 );

  B4x4Idx cIdxLeft = B4x4Idx( ( ( cIdx % 8) << 1 ) + ( m_bAddEdge ? 7 : 3 ) );

  ROTRS( rcMbDataCurr.is4x4BlkCoded( cIdx     ), 2 );
  ROTRS( rcMbDataLeft.is4x4BlkCoded( cIdxLeft ), 2 );

  return 1;
}

__inline UInt LoopFilter::xGetHorFilterStrength( const MbDataAccess&  rcMbDataAccess,
                                                 LumaIdx              cIdx,
                                                 Int                  iFilterIdc )
{
  const MbData& rcMbDataCurr  = rcMbDataAccess.getMbDataCurr();
  Short         sHorMvThr     = 4;
  Short         sVerMvThr     = ( FRAME == rcMbDataAccess.getMbPicType() ? 4 : 2 );

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
  ROTRS( (iFilterIdc == 2 || iFilterIdc == 4) && ! rcMbDataAccess.isAvailableAbove(),  0 );
  ROTRS( ! rcMbDataAccess.isAboveMbExisting(),                    0 );

  if( ! m_bHorMixedMode )
  {
    const MbData& rcMbDataAbove = (rcMbDataCurr.getFieldFlag()? rcMbDataAccess.getMbDataAboveAbove():
    rcMbDataAccess.getMbDataAbove());

    ROTRS( LFM_DEFAULT_FILTER != m_eLFMode && rcMbDataCurr.isIntra() ^ rcMbDataAbove.isIntra(), 0 );

    if( FRAME == rcMbDataAccess.getMbPicType() )
    {
      ROTRS( rcMbDataCurr. isIntra(), 4 );
      ROTRS( rcMbDataAbove.isIntra(), 4 );
    }
    else
    {
      ROTRS( rcMbDataCurr. isIntra(),  3 );
      ROTRS( rcMbDataAbove.isIntra(),  3 );
    }

    if( m_pcHighpassYuvBuffer )
    {
// TMM_INTERLACE{   
        ROTRS( rcMbDataCurr. is4x4BlkResidual( cIdx                          ), 2 );
        ROTRS( rcMbDataAbove.is4x4BlkResidual( cIdx + LEFT_MB_LEFT_NEIGHBOUR ), 2 );
// TMM_INTERLACE}
    }
    else
    {
      ROTRS( rcMbDataCurr. is4x4BlkCoded( cIdx                            ), 2 );
      ROTRS( rcMbDataAbove.is4x4BlkCoded( cIdx + ABOVE_MB_ABOVE_NEIGHBOUR ), 2 );
    }
  
    return xCheckMvDataB  ( rcMbDataCurr, cIdx, rcMbDataAbove, cIdx + ABOVE_MB_ABOVE_NEIGHBOUR, sHorMvThr, sVerMvThr );
  }

  if( FRAME == rcMbDataAccess.getMbPicType() )
  {
    // mixed mode, current macroblock is a frame macroblock
    const MbData& rcMbDataAbove = ( m_bAddEdge ? rcMbDataAccess.getMbDataAboveAbove() : rcMbDataAccess.getMbDataAbove() );

    //th this is not correct for mbaff case but I don't know if this feature is still in use
    ROTRS( LFM_DEFAULT_FILTER != m_eLFMode && rcMbDataCurr.isIntra() ^ rcMbDataAbove.isIntra(), 0 );

    ROTRS( rcMbDataCurr. isIntra(),  3 );
    ROTRS( rcMbDataAbove.isIntra(),  3 );

    ROTRS( rcMbDataCurr. is4x4BlkCoded( cIdx                            ), 2 );
    ROTRS( rcMbDataAbove.is4x4BlkCoded( cIdx + ABOVE_MB_ABOVE_NEIGHBOUR ), 2 );

    return 1;
  }

	// mixed mode, current macroblock is field macroblock
  const MbData& rcMbDataAbove = ( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataAbove() : rcMbDataAccess.getMbDataAboveAbove() );

  //th this is not correct for mbaff case but I don't know if this feature is still in use
  ROTRS( LFM_DEFAULT_FILTER != m_eLFMode && rcMbDataCurr.isIntra() ^ rcMbDataAbove.isIntra(), 0 );

//  ROTRS( MSYS_UINT_MAX == rcMbDataAbove.getSliceId(), 0);  // not existing !!
  ROTRS( rcMbDataCurr. isIntra(),  3 );
  ROTRS( rcMbDataAbove.isIntra(),  3 );

  ROTRS( rcMbDataCurr. is4x4BlkCoded( cIdx                            ), 2 );
  ROTRS( rcMbDataAbove.is4x4BlkCoded( cIdx + ABOVE_MB_ABOVE_NEIGHBOUR ), 2 );

  return 1;
}

ErrVal LoopFilter::process( SliceHeader&  rcSH,
                            IntFrame*     pcIntFrame,
                            MbDataCtrl*   pcMbDataCtrlMot, // if NULL -> all intra
                            MbDataCtrl*   pcMbDataCtrlRes,
                            UInt          uiMbInRow,
                            RefFrameList* pcRefFrameList0,
                            RefFrameList* pcRefFrameList1,			
							bool		  bAllSliceDone, 
                            bool          spatial_scalable_flg)
{
  bool enhancedLayerFlag = (rcSH.getLayerId()>0) ; //V032 FSL added for disabling chroma deblocking
  if( m_pcRCDOSliceHeader )
  {
    m_bRCDO = m_pcRCDOSliceHeader->getSPS().getRCDODeblocking();
  }
  else
  {
    m_bRCDO = rcSH.getSPS().getRCDODeblocking();
  }

  ROT( NULL == m_pcControlMngIf );

  RNOK(   m_pcControlMngIf->initSliceForFiltering ( rcSH               ) );
  RNOK(   pcMbDataCtrlRes ->initSlice             ( rcSH, POST_PROCESS, false, NULL ) );
  if( pcMbDataCtrlMot )
  {
    RNOK( pcMbDataCtrlMot ->initSlice             ( rcSH, POST_PROCESS, false, NULL ) );
  }    
  m_bVerMixedMode = false;
  m_bHorMixedMode = false;

  RefFrameList* apcRefFrameList0[4] = { NULL, NULL, NULL, NULL };
  RefFrameList* apcRefFrameList1[4] = { NULL, NULL, NULL, NULL };

  IntFrame* apcFrame        [4] = { NULL, NULL, NULL, NULL };
  IntFrame* apcHighpassFrame[4] = { NULL, NULL, NULL, NULL };

  if( pcIntFrame != NULL )
  {
		RNOK( pcIntFrame->addFrameFieldBuffer() );
    apcFrame[ TOP_FIELD ] = pcIntFrame->getPic( TOP_FIELD );
    apcFrame[ BOT_FIELD ] = pcIntFrame->getPic( BOT_FIELD );
		apcFrame[ FRAME     ] = pcIntFrame->getPic( FRAME     );
  }

  if( m_pcHighpassFrame != NULL )
  {
		RNOK( m_pcHighpassFrame->addFrameFieldBuffer() );
    apcHighpassFrame[ TOP_FIELD ] = m_pcHighpassFrame->getPic( TOP_FIELD );
    apcHighpassFrame[ BOT_FIELD ] = m_pcHighpassFrame->getPic( BOT_FIELD );
		apcHighpassFrame[ FRAME     ] = m_pcHighpassFrame->getPic( FRAME     );
  }

  const PicType ePicType = rcSH.getPicType();
  const Bool    bMbAff   = rcSH.isMbAff();
	if( bMbAff )
  {
	  RefFrameList acRefFrameList0[2];
		RefFrameList acRefFrameList1[2];

		RNOK( gSetFrameFieldLists( acRefFrameList0[0], acRefFrameList0[1], *pcRefFrameList0 ) );
		RNOK( gSetFrameFieldLists( acRefFrameList1[0], acRefFrameList1[1], *pcRefFrameList1 ) );

    apcRefFrameList0[ TOP_FIELD ] = ( NULL == pcRefFrameList0 ) ? NULL : &acRefFrameList0[0];
    apcRefFrameList0[ BOT_FIELD ] = ( NULL == pcRefFrameList0 ) ? NULL : &acRefFrameList0[1];
    apcRefFrameList1[ TOP_FIELD ] = ( NULL == pcRefFrameList1 ) ? NULL : &acRefFrameList1[0];
    apcRefFrameList1[ BOT_FIELD ] = ( NULL == pcRefFrameList1 ) ? NULL : &acRefFrameList1[1];
    apcRefFrameList0[     FRAME ] = pcRefFrameList0;
    apcRefFrameList1[     FRAME ] = pcRefFrameList1;
  }
	else
  {
    apcRefFrameList0[ ePicType ] = pcRefFrameList0;
    apcRefFrameList1[ ePicType ] = pcRefFrameList1;
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
    UInt uiMbAddress = uiFirstMbInSlice;
    for( ;uiMbAddress<=uiLastMbInSlice ;)    
    //===== loop over macroblocks use raster scan =====  
    {
      MbDataAccess* pcMbDataAccessMot = NULL;
      MbDataAccess* pcMbDataAccessRes = NULL;
      UInt          uiMbY, uiMbX;

      rcSH.getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddress                                 );

      if( pcMbDataCtrlMot )
      {
        RNOK( pcMbDataCtrlMot ->initMb            (  pcMbDataAccessMot, uiMbY, uiMbX ) );
      }
      RNOK(   pcMbDataCtrlRes ->initMb            (  pcMbDataAccessRes, uiMbY, uiMbX ) );
      RNOK(   m_pcControlMngIf->initMbForFiltering( *pcMbDataAccessRes, uiMbY, uiMbX, bMbAff ) );

      const PicType eMbPicType = pcMbDataAccessRes->getMbPicType();

      if( m_pcHighpassFrame ) 
      {
        m_pcHighpassYuvBuffer = apcHighpassFrame[ eMbPicType ]->getFullPelYuvBuffer();
      }
      else
      {
        m_pcHighpassYuvBuffer = NULL;
      } 
    
	  
      if( 0 == (m_eLFMode & LFM_NO_FILTER))
      {				

  		  RNOK( xFilterMb( pcMbDataAccessMot,
                         pcMbDataAccessRes,
                         apcFrame        [ eMbPicType ]->getFullPelYuvBuffer(),
                         apcRefFrameList0[ eMbPicType ],
                         apcRefFrameList1[ eMbPicType ],
                         spatial_scalable_flg,
						             enhancedLayerFlag ) );  //V032 of FSL added for disabling chroma deblocking
	   }
 //{ agl@simecom FIX ------------------
      uiMbAddress = rcSH.getFMO()->getNextMBNr(uiMbAddress ); 
	}								
 
	for(uiMbAddress= uiFirstMbInSlice ;uiMbAddress<=uiLastMbInSlice ;)  
	{
     UInt          uiMbY             = uiMbAddress / uiMbInRow;
     UInt          uiMbX             = uiMbAddress % uiMbInRow;

     MbDataAccess* pcMbDataAccessRes = 0;

     RNOK(   pcMbDataCtrlRes ->initMb            (  pcMbDataAccessRes, uiMbY, uiMbX ) );
     RNOK(   m_pcControlMngIf->initMbForFiltering(  *pcMbDataAccessRes, uiMbY, uiMbX, bMbAff  ) ); // TMM_INTERLACE
 //-------------------------- agl@simecom FIX }


      if( m_eLFMode & LFM_EXTEND_INTRA_SUR )
  	  {
        UInt uiMask = 0;
  
        RNOK( pcMbDataCtrlRes->getBoundaryMask( uiMbY, uiMbX, uiMask ) );
  
        if( uiMask )
  		  {
          IntYuvMbBufferExtension cBuffer;
          cBuffer.setAllSamplesToZero();
  
      		cBuffer.loadSurrounding( pcIntFrame->getPic(ePicType)->getFullPelYuvBuffer() );
  
          RNOK( m_pcReconstructionBypass->padRecMb( &cBuffer, uiMask ) );
      		
      		pcIntFrame->getPic(ePicType)->getFullPelYuvBuffer()->loadBuffer( &cBuffer );
  		  }
  	  }
      uiMbAddress = rcSH.getFMO()->getNextMBNr(uiMbAddress );
    }
  }
  //<- ICU/ETRI DS

  // Hanke@RWTH: Reset pointer
  setHighpassFramePointer();
  m_pcHighpassYuvBuffer = NULL; // fix (HS)

  return Err::m_nOK;
}

__inline ErrVal LoopFilter::xFilterMb( MbDataAccess*        pcMbDataAccessMot,
                                       MbDataAccess*        pcMbDataAccessRes,
                                       IntYuvPicBuffer*     pcYuvBuffer,
                                       RefFrameList*        pcRefFrameList0,
                                       RefFrameList*        pcRefFrameList1,
                                       bool                 spatial_scalable_flg,   // SSUN@SHARP
									                     bool					enhancedLayerFlag)                //V032 of FSL
{
  if( m_bRCDO )
  {
    RNOK( xFilterMb_RCDO( pcMbDataAccessMot, pcMbDataAccessRes, pcYuvBuffer, pcRefFrameList0, pcRefFrameList1, spatial_scalable_flg ) );
    return Err::m_nOK;
  }

  const DFP& rcDFP      = pcMbDataAccessRes->getDeblockingFilterParameter(m_eLFMode & LFM_NO_INTER_FILTER);
  const Int iFilterIdc  = rcDFP.getDisableDeblockingFilterIdc();

  ROTRS( (m_eLFMode & LFM_NO_INTER_FILTER) && ! pcMbDataAccessRes->getMbData().isIntra(), Err::m_nOK );

  ROTRS( iFilterIdc == 1, Err::m_nOK );

  Bool b8x8 = pcMbDataAccessRes->getMbData().isTransformSize8x8();

  if( m_pcHighpassYuvBuffer )
  {
    UInt uiCbp = 0;
    for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
    {
      uiCbp += ((m_pcHighpassYuvBuffer->isCurr4x4BlkNotZero( cIdx )? 1 :0) << cIdx.b4x4());
    }
    pcMbDataAccessRes->getMbData().setMbCbpResidual( uiCbp );
  }
  else
  {
    pcMbDataAccessRes->getMbData().setMbCbpResidual( 0 );
  }

  Bool bFieldFlag = false;
  Bool bCurrFrame = false;
  const MbDataAccess* pcMbDataAccessFF = pcMbDataAccessMot ? pcMbDataAccessMot : pcMbDataAccessRes;
  if( pcMbDataAccessFF )
  {
    bFieldFlag      = pcMbDataAccessFF->getMbData().getFieldFlag();
    m_bVerMixedMode = (bFieldFlag != pcMbDataAccessFF->getMbDataLeft().getFieldFlag());
    m_bHorMixedMode = (bFieldFlag?(bFieldFlag != pcMbDataAccessFF->getMbDataAboveAbove().getFieldFlag()):
                                  (bFieldFlag != pcMbDataAccessFF->getMbDataAbove().getFieldFlag()));
    bCurrFrame      = (FRAME      == pcMbDataAccessFF->getMbPicType());
  }

  m_bAddEdge = true;
  if( m_bHorMixedMode && bCurrFrame )
  {
    for( B4x4Idx cIdx; cIdx.b4x4() < 4; cIdx++ )
    {
      m_aucBsHorTop[cIdx.x()] = xGetHorFilterStrength_RefIdx( pcMbDataAccessMot,
                                                              pcMbDataAccessRes,
                                                              cIdx,
                                                              iFilterIdc,
                                                              pcRefFrameList0,
                                                              pcRefFrameList1,
                                                              spatial_scalable_flg );  
    }
  }
  if( m_bVerMixedMode )
  {
    for( B4x4Idx cIdx; cIdx.b4x4() < 16; cIdx = B4x4Idx( cIdx + 4 ) )
    {
      m_aucBsVerBot[cIdx.y()] = xGetVerFilterStrength_RefIdx( pcMbDataAccessMot,
                                                              pcMbDataAccessRes,
                                                              cIdx,
                                                              iFilterIdc,
                                                              pcRefFrameList0,
                                                              pcRefFrameList1,
                                                              spatial_scalable_flg );  
    }
  }
  m_bAddEdge = false;

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

  m_bHorMixedMode  = m_bHorMixedMode && bCurrFrame;

  RNOK( xLumaVerFiltering(   *pcMbDataAccessRes, rcDFP, pcYuvBuffer ) );
  RNOK( xLumaHorFiltering(   *pcMbDataAccessRes, rcDFP, pcYuvBuffer ) );
 //V032 of FSL for disabling chroma deblocking in enh. layer
  if (!enhancedLayerFlag || (enhancedLayerFlag && iFilterIdc !=3 && iFilterIdc != 4) ) 
  { 
	RNOK( xChromaVerFiltering( *pcMbDataAccessRes, rcDFP, pcYuvBuffer ) );
	RNOK( xChromaHorFiltering( *pcMbDataAccessRes, rcDFP, pcYuvBuffer ) );
  }
  return Err::m_nOK;
}


__inline ErrVal LoopFilter::xFilterMb_RCDO( const MbDataAccess*  pcMbDataAccessMot,
                                           const MbDataAccess*  pcMbDataAccessRes,
                                           IntYuvPicBuffer*     pcYuvBuffer,
                                           RefFrameList*        pcRefFrameList0,
                                           RefFrameList*        pcRefFrameList1,
                                           bool                 spatial_scalable_flg )  // SSUN@SHARP
{
  const DFP& rcDFP        = pcMbDataAccessRes->getDeblockingFilterParameter(m_eLFMode & LFM_NO_INTER_FILTER);
  const Int  iFilterIdc  = rcDFP.getDisableDeblockingFilterIdc();

  ROTRS( (m_eLFMode & LFM_NO_INTER_FILTER) && ! pcMbDataAccessRes->getMbData().isIntra(), Err::m_nOK );
  ROTRS( iFilterIdc == 1, Err::m_nOK );

  const MbDataAccess* pcMbDataAccess = ( pcMbDataAccessMot ? pcMbDataAccessMot : pcMbDataAccessRes );
  ROF( pcMbDataAccess );
  Bool  bFieldFlag  = pcMbDataAccess->getMbData().getFieldFlag();
  m_bVerMixedMode   = ( bFieldFlag != pcMbDataAccess->getMbDataLeft().getFieldFlag() );
  m_bHorMixedMode   = ( bFieldFlag ? ( bFieldFlag != pcMbDataAccess->getMbDataAboveAbove().getFieldFlag() ) 
                                   : ( bFieldFlag != pcMbDataAccess->getMbDataAbove     ().getFieldFlag() ) );

  RNOK( xLumaVerFiltering_H241RCDO(   *pcMbDataAccessRes, rcDFP, pcYuvBuffer, pcMbDataAccessMot, pcRefFrameList0, pcRefFrameList1 ) );
  RNOK( xLumaHorFiltering_H241RCDO(   *pcMbDataAccessRes, rcDFP, pcYuvBuffer, pcMbDataAccessMot, pcRefFrameList0, pcRefFrameList1 ) );
  RNOK( xChromaVerFiltering_H241RCDO( *pcMbDataAccessRes, rcDFP, pcYuvBuffer ) ); 
  RNOK( xChromaHorFiltering_H241RCDO( *pcMbDataAccessRes, rcDFP, pcYuvBuffer ) );

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
   	else if(pcMbDataAccessRes->isAvailableLeft() || ( pcMbDataAccessRes->isLeftMbExisting() && iFilterIdc != 2 && iFilterIdc != 4)) //V032 of FSL
    {
      const MbData& rcMbDataCurr  = pcMbDataAccessRes->getMbDataCurr();
//			const MbData& rcMbDataLeft = pcMbDataAccessRes->getMbDataLeft();//TMM_BUG_EF{}
//TMM_INTERLACE {
			Bool	bMixFrFld = rcMbDataCurr.getFieldFlag() && ! pcMbDataAccessRes->getMbDataLeft().getFieldFlag();
			Bool	bTopMb = pcMbDataAccessRes->isTopMb();

			const MbData& rcMbDataLeft =	( bMixFrFld && bTopMb && cIdx > 7 )  ? pcMbDataAccessRes->getMbDataBelowLeft() :
																		( bMixFrFld && !bTopMb && cIdx < 8 ) ? pcMbDataAccessRes->getMbDataAboveLeft() : 
																		pcMbDataAccessRes->getMbDataLeft();
//TMM_INTERLACE }
      ROTRS( LFM_DEFAULT_FILTER != m_eLFMode && rcMbDataLeft.isIntra() ^ rcMbDataCurr.isIntra(), 0 );// bugfix, agl@simecom 
      if(rcMbDataCurr.isIntra_BL() && rcMbDataLeft.isIntra_BL())
      {
        ROTRS( rcMbDataCurr.is4x4BlkCoded( cIdx ), 1 );
        ROTRS( rcMbDataLeft.is4x4BlkCoded( cIdx + LEFT_MB_LEFT_NEIGHBOUR), 1 );
        return(0);
      }
      else if(rcMbDataCurr.isIntra_BL()){
        ROTRS( rcMbDataLeft.isIntra_nonBL(), 4 );
        ROTRS( rcMbDataCurr.is4x4BlkCoded( cIdx ), 2 );
        //th
        ROTRS( rcMbDataLeft.is4x4BlkCoded   ( cIdx + LEFT_MB_LEFT_NEIGHBOUR), 2 );
        ROTRS( rcMbDataLeft.is4x4BlkResidual( cIdx + LEFT_MB_LEFT_NEIGHBOUR), 2 ); 

        return(1);
      }
      else if(rcMbDataLeft.isIntra_BL())
      {
        ROTRS( rcMbDataCurr.isIntra_nonBL(), 4 );
        ROTRS( rcMbDataLeft.is4x4BlkCoded( cIdx + LEFT_MB_LEFT_NEIGHBOUR), 2 );

        ROTRS( rcMbDataCurr.is4x4BlkCoded   ( cIdx ), 2 );
        ROTRS( rcMbDataCurr.is4x4BlkResidual( cIdx ), 2 ); 
        
        return(1);
      }
    }
  } 
  // SSUN@SHARP end of JVT-P013r1
  else
  {  
    //-- Samsung 2005.02.xx
    if(pcMbDataAccessRes->getMbDataCurr().getMbMode() == INTRA_BL)
    {
      if( cIdx.x() )
      {
        return 1;		//	if not MB_boundary
      }

      // is either in same slice or deblocking across slice boundaries is enabled (and the XXX macroblock is inside the picture)
      if( ( pcMbDataAccessRes->isAvailableLeft() || ( pcMbDataAccessRes->isLeftMbExisting() && iFilterIdc != 2 && iFilterIdc != 4) ) && //V032 of FSL
            pcMbDataAccessRes->getMbDataLeft().getMbMode() == INTRA_BL )
      {
        return 1;
      }
    }
    //--
  }

 if( NULL == pcMbDataAccessMot )
  {
    pcMbDataAccessMot = pcMbDataAccessRes;
  }

  const MbData& rcMbDataCurrMot = pcMbDataAccessMot->getMbDataCurr();
  const MbData& rcMbDataCurrRes = pcMbDataAccessRes->getMbDataCurr();
  Short         sHorMvThr       = 4;
  Short         sVerMvThr     = ( FRAME == pcMbDataAccessMot->getMbPicType() ? 4 : 2 );

  if( cIdx.x() )
  {
    // this is a edge inside of a macroblock
    ROTRS( rcMbDataCurrMot.isIntra(), 3 );

    //th
    ROTRS( rcMbDataCurrRes.is4x4BlkResidual( cIdx), 2 ); 
    ROTRS( rcMbDataCurrRes.is4x4BlkResidual( cIdx + CURR_MB_LEFT_NEIGHBOUR), 2 ); 
    { 
      ROTRS( rcMbDataCurrRes.is4x4BlkCoded( cIdx                          ), 2 );
      ROTRS( rcMbDataCurrRes.is4x4BlkCoded( cIdx + CURR_MB_LEFT_NEIGHBOUR ), 2 );
    }
    
    if( rcMbDataCurrMot.isInterPMb() )
    {
      return xCheckMvDataP_RefIdx( rcMbDataCurrMot, cIdx, rcMbDataCurrMot, cIdx + CURR_MB_LEFT_NEIGHBOUR,
                                   sHorMvThr, sVerMvThr,
                                   *pcRefFrameList0, *pcRefFrameList1 );
    }
    return   xCheckMvDataB_RefIdx( rcMbDataCurrMot, cIdx, rcMbDataCurrMot, cIdx + CURR_MB_LEFT_NEIGHBOUR,
                                   sHorMvThr, sVerMvThr,
                                   *pcRefFrameList0, *pcRefFrameList1 );
  }


  // if we get here we are on a macroblock edge
  ROTRS( (iFilterIdc == 2 || iFilterIdc == 4) && ! pcMbDataAccessMot->isAvailableLeft(), 0 ); //V032 of FSL
  ROTRS( ! pcMbDataAccessMot->isLeftMbExisting(),                   0 );

  if( ! m_bVerMixedMode )
  {
    const MbData& rcMbDataLeftMot = pcMbDataAccessMot->getMbDataLeft();
    const MbData& rcMbDataLeftRes = pcMbDataAccessRes->getMbDataLeft();
    ROTRS( LFM_DEFAULT_FILTER != m_eLFMode && rcMbDataCurrMot.isIntra() ^ rcMbDataLeftMot.isIntra(), 0 );
    
    ROTRS( rcMbDataCurrMot.isIntra(), 4 );
    ROTRS( rcMbDataLeftMot.isIntra(), 4 );
  
      //th
      ROTRS( rcMbDataCurrRes.is4x4BlkResidual( cIdx                         ), 2 );
      ROTRS( rcMbDataLeftRes.is4x4BlkResidual( cIdx + LEFT_MB_LEFT_NEIGHBOUR), 2 ); 
    { 
      ROTRS( rcMbDataCurrRes.is4x4BlkCoded( cIdx                          ), 2 );
      ROTRS( rcMbDataLeftRes.is4x4BlkCoded( cIdx + LEFT_MB_LEFT_NEIGHBOUR ), 2 );
    }
    
    if( rcMbDataCurrMot.isInterPMb() && rcMbDataLeftMot.isInterPMb())
    {
      return xCheckMvDataP_RefIdx( rcMbDataCurrMot, cIdx, rcMbDataLeftMot, cIdx + LEFT_MB_LEFT_NEIGHBOUR,
                                   sHorMvThr, sVerMvThr,
                                   *pcRefFrameList0, *pcRefFrameList1 );
    }
    return   xCheckMvDataB_RefIdx( rcMbDataCurrMot, cIdx, rcMbDataLeftMot, cIdx + LEFT_MB_LEFT_NEIGHBOUR,
                                   sHorMvThr, sVerMvThr,
                                   *pcRefFrameList0, *pcRefFrameList1 );
  }

  if( FRAME == pcMbDataAccessMot->getMbPicType() )
  {
    // mixed mode, current macroblock is a frame macroblock
    const MbData& rcMbDataLeftMot = (  pcMbDataAccessMot->isTopMb() &&   m_bAddEdge ? pcMbDataAccessMot->getMbDataBelowLeft() :
    ! pcMbDataAccessMot->isTopMb() && ! m_bAddEdge ? pcMbDataAccessMot->getMbDataAboveLeft() : pcMbDataAccessMot->getMbDataLeft() );
    const MbData& rcMbDataLeftRes = (  pcMbDataAccessRes->isTopMb() &&   m_bAddEdge ? pcMbDataAccessRes->getMbDataBelowLeft() :
    ! pcMbDataAccessRes->isTopMb() && ! m_bAddEdge ? pcMbDataAccessRes->getMbDataAboveLeft() : pcMbDataAccessRes->getMbDataLeft() );

    //th this is not correct for mbaff case but I don't know if this feature is still in use
    ROTRS( LFM_DEFAULT_FILTER != m_eLFMode && rcMbDataCurrMot.isIntra() ^ rcMbDataLeftMot.isIntra(), 0 );

    ROTRS( rcMbDataCurrMot.isIntra(),  4 );
    ROTRS( rcMbDataLeftMot.isIntra(),  4 );

    B4x4Idx cIdxLeft = B4x4Idx( pcMbDataAccessMot->isTopMb() ? ( cIdx < 8 ? 3 : 7 ) : ( cIdx < 8 ? 11 : 15 ) );

    //th
    ROTRS( rcMbDataCurrRes.is4x4BlkResidual( cIdx     ), 2 );
    ROTRS( rcMbDataLeftRes.is4x4BlkResidual( cIdxLeft ), 2 ); 

    ROTRS( rcMbDataCurrRes.is4x4BlkCoded( cIdx     ), 2 );
    ROTRS( rcMbDataLeftRes.is4x4BlkCoded( cIdxLeft ), 2 );

    return 1;
  }


  // mixed mode, current macroblock is a field macroblock

  const MbData& rcMbDataLeftMot = (   pcMbDataAccessMot->isTopMb() && cIdx > 7 ? pcMbDataAccessMot->getMbDataBelowLeft() :
  ! pcMbDataAccessMot->isTopMb() && cIdx < 8 ? pcMbDataAccessMot->getMbDataAboveLeft() : pcMbDataAccessMot->getMbDataLeft() );
  const MbData& rcMbDataLeftRes = (   pcMbDataAccessRes->isTopMb() && cIdx > 7 ? pcMbDataAccessRes->getMbDataBelowLeft() :
  ! pcMbDataAccessRes->isTopMb() && cIdx < 8 ? pcMbDataAccessRes->getMbDataAboveLeft() : pcMbDataAccessRes->getMbDataLeft() );

  //th this is not correct for mbaff case but I don't know if this feature is still in use
  ROTRS( LFM_DEFAULT_FILTER != m_eLFMode && rcMbDataCurrMot.isIntra() ^ rcMbDataLeftMot.isIntra(), 0 );
  
  ROTRS( rcMbDataCurrMot.isIntra(), 4 );
  ROTRS( rcMbDataLeftMot.isIntra(), 4 );

  B4x4Idx cIdxLeft = B4x4Idx( ( ( cIdx % 8) << 1 ) + ( m_bAddEdge ? 7 : 3 ) );

  //th
  ROTRS( rcMbDataCurrRes.is4x4BlkResidual( cIdx     ), 2 );
  ROTRS( rcMbDataLeftRes.is4x4BlkResidual( cIdxLeft ), 2 ); 

    ROTRS( rcMbDataCurrRes.is4x4BlkCoded( cIdx                          ), 2 );
  ROTRS( rcMbDataLeftRes.is4x4BlkCoded( cIdxLeft ), 2 );
  
  return 1;
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
    else if(pcMbDataAccessRes->isAvailableAbove() || ( pcMbDataAccessRes->isAboveMbExisting() && iFilterIdc != 2 && iFilterIdc != 4)) //V032
   {
      const MbData& rcMbDataCurr = pcMbDataAccessRes->getMbDataCurr();
      const MbData& rcMbDataAbove = pcMbDataAccessRes->getMbDataAbove();
      ROTRS( LFM_DEFAULT_FILTER != m_eLFMode && rcMbDataCurr.isIntra() ^ rcMbDataAbove.isIntra(), 0 ); // bugfix, agl@simecom 
      if(rcMbDataCurr.isIntra_BL() && rcMbDataAbove.isIntra_BL())
      {
        ROTRS( rcMbDataCurr.is4x4BlkCoded( cIdx ), 1 );
        ROTRS( rcMbDataAbove.is4x4BlkCoded( cIdx + ABOVE_MB_ABOVE_NEIGHBOUR), 1 );
        return(0);
      }
      if(rcMbDataCurr.isIntra_BL())
      {
        ROTRS( rcMbDataAbove.isIntra_nonBL(), 4 );
        ROTRS( rcMbDataCurr.is4x4BlkCoded( cIdx ), 2 );
        //th
        ROTRS( rcMbDataAbove.is4x4BlkCoded( cIdx + ABOVE_MB_ABOVE_NEIGHBOUR), 2 );
        ROTRS( rcMbDataAbove.is4x4BlkResidual( cIdx + ABOVE_MB_ABOVE_NEIGHBOUR), 2 ); 

        return(1);
      }
      else if(rcMbDataAbove.isIntra_BL())
      {
        ROTRS( rcMbDataCurr.isIntra_nonBL(), 4 );
        ROTRS( rcMbDataAbove.is4x4BlkCoded( cIdx + ABOVE_MB_ABOVE_NEIGHBOUR), 2 );
        //th
        ROTRS( rcMbDataCurr.is4x4BlkCoded( cIdx ), 2 );
        ROTRS( rcMbDataCurr.is4x4BlkResidual( cIdx ), 2 ); 

        return(1);
      }
    }
  }
  // SSUN@SHARP end of JVT-P013r1
  else
  {  
    //-- Samsung 2005.02.xx
    if(pcMbDataAccessRes->getMbDataCurr().getMbMode() == INTRA_BL)
    {
      if( cIdx.y() )
      {
        return 1;		//	if not MB_boundary
      }

      // is either in same slice or deblocking across slice boundaries is enabled (and the XXX macroblock is inside the picture)
      if( ( pcMbDataAccessRes->isAvailableAbove() || ( pcMbDataAccessRes->isAboveMbExisting() && iFilterIdc != 2 && iFilterIdc != 4) ) &&
            pcMbDataAccessRes->getMbDataAbove().getMbMode() == INTRA_BL )
      {
        return 1;
      }
    }
    //--
  }
 if( NULL == pcMbDataAccessMot )
  {
    pcMbDataAccessMot = pcMbDataAccessRes;
  }

  const MbData& rcMbDataCurrMot = pcMbDataAccessMot->getMbDataCurr();
  const MbData& rcMbDataCurrRes = pcMbDataAccessRes->getMbDataCurr();
  Short         sHorMvThr       = 4;
  Short         sVerMvThr     = ( FRAME == pcMbDataAccessMot->getMbPicType() ? 4 : 2 );

  if( cIdx.y() )
  {
    // internal edge
    ROTRS( rcMbDataCurrMot.isIntra(), 3 );

    //th
    ROTRS( rcMbDataCurrRes.is4x4BlkResidual( cIdx ), 2 );
    ROTRS( rcMbDataCurrRes.is4x4BlkResidual( cIdx + CURR_MB_ABOVE_NEIGHBOUR ), 2 ); 

    {
      ROTRS( rcMbDataCurrRes.is4x4BlkCoded( cIdx                           ), 2 );
      ROTRS( rcMbDataCurrRes.is4x4BlkCoded( cIdx + CURR_MB_ABOVE_NEIGHBOUR ), 2 );
    }
    
    if( rcMbDataCurrMot.isInterPMb() )
    {
      return xCheckMvDataP_RefIdx( rcMbDataCurrMot, cIdx, rcMbDataCurrMot, cIdx + CURR_MB_ABOVE_NEIGHBOUR,
                                   sHorMvThr, sVerMvThr,
                                   *pcRefFrameList0, *pcRefFrameList1 );
    }
    return   xCheckMvDataB_RefIdx( rcMbDataCurrMot, cIdx, rcMbDataCurrMot, cIdx + CURR_MB_ABOVE_NEIGHBOUR,
                                   sHorMvThr, sVerMvThr,
                                   *pcRefFrameList0, *pcRefFrameList1 );
  }

  // if we get here we are on a macroblock edge
  ROTRS( (iFilterIdc == 2 || iFilterIdc == 4) && ! pcMbDataAccessMot->isAvailableAbove(),  0 ); //V032
  ROTRS( ! pcMbDataAccessMot->isAboveMbExisting(),                    0 );

  if( ! m_bHorMixedMode )
  {
    const MbData& rcMbDataAboveMot = (rcMbDataCurrMot.getFieldFlag()? pcMbDataAccessMot->getMbDataAboveAbove():pcMbDataAccessMot->getMbDataAbove());
    const MbData& rcMbDataAboveRes = (rcMbDataCurrRes.getFieldFlag()? pcMbDataAccessRes->getMbDataAboveAbove():pcMbDataAccessRes->getMbDataAbove());

  ROTRS( LFM_DEFAULT_FILTER != m_eLFMode && rcMbDataCurrMot.isIntra() ^ rcMbDataAboveMot.isIntra(), 0 );
  
    if( FRAME == pcMbDataAccessMot->getMbPicType() )
    {
  ROTRS( rcMbDataCurrMot. isIntra(), 4 );
  ROTRS( rcMbDataAboveMot.isIntra(), 4 );
    }
    else
    {
      ROTRS( rcMbDataCurrMot. isIntra(),  3 );
      ROTRS( rcMbDataAboveMot.isIntra(),  3 );
  }

    //th
    ROTRS( rcMbDataCurrRes. is4x4BlkResidual( cIdx                            ), 2 );
    ROTRS( rcMbDataAboveRes.is4x4BlkResidual( cIdx + ABOVE_MB_ABOVE_NEIGHBOUR ), 2 ); 

  {
    ROTRS( rcMbDataCurrRes. is4x4BlkCoded( cIdx                            ), 2 );
    ROTRS( rcMbDataAboveRes.is4x4BlkCoded( cIdx + ABOVE_MB_ABOVE_NEIGHBOUR ), 2 );
  }
  
  if( rcMbDataCurrMot.isInterPMb() && rcMbDataAboveMot.isInterPMb())
  {
    return xCheckMvDataP_RefIdx( rcMbDataCurrMot, cIdx, rcMbDataAboveMot, cIdx + ABOVE_MB_ABOVE_NEIGHBOUR,
                                 sHorMvThr, sVerMvThr,
                                 *pcRefFrameList0, *pcRefFrameList1 );
  }
  return   xCheckMvDataB_RefIdx( rcMbDataCurrMot, cIdx, rcMbDataAboveMot, cIdx + ABOVE_MB_ABOVE_NEIGHBOUR,
                                 sHorMvThr, sVerMvThr,
                                 *pcRefFrameList0, *pcRefFrameList1 );
  }

  if( FRAME == pcMbDataAccessMot->getMbPicType() )
  {
    // mixed mode, current macroblock is a frame macroblock
    const MbData& rcMbDataAboveMot = ( m_bAddEdge ? pcMbDataAccessMot->getMbDataAboveAbove() : pcMbDataAccessMot->getMbDataAbove() );
    const MbData& rcMbDataAboveRes = ( m_bAddEdge ? pcMbDataAccessRes->getMbDataAboveAbove() : pcMbDataAccessRes->getMbDataAbove() );

    //th this is not correct for mbaff case but I don't know if this feature is still in use
    ROTRS( LFM_DEFAULT_FILTER != m_eLFMode && rcMbDataCurrMot.isIntra() ^ rcMbDataAboveMot.isIntra(), 0 );

    ROTRS( rcMbDataCurrMot. isIntra(),  3 );
    ROTRS( rcMbDataAboveMot.isIntra(),  3 );

    //th
    ROTRS( rcMbDataCurrRes. is4x4BlkResidual( cIdx                            ), 2 );
    ROTRS( rcMbDataAboveRes.is4x4BlkResidual( cIdx + ABOVE_MB_ABOVE_NEIGHBOUR ), 2 ); 

    ROTRS( rcMbDataCurrRes. is4x4BlkCoded( cIdx                            ), 2 );
    ROTRS( rcMbDataAboveRes.is4x4BlkCoded( cIdx + ABOVE_MB_ABOVE_NEIGHBOUR ), 2 );

    return 1;
  }


  // mixed mode, current macroblock is field macroblock
  const MbData& rcMbDataAboveMot = ( pcMbDataAccessMot->isTopMb() ? pcMbDataAccessMot->getMbDataAbove() : pcMbDataAccessMot->getMbDataAboveAbove() );
  const MbData& rcMbDataAboveRes = ( pcMbDataAccessRes->isTopMb() ? pcMbDataAccessRes->getMbDataAbove() : pcMbDataAccessRes->getMbDataAboveAbove() );

  //th this is not correct for mbaff case but I don't know if this feature is still in use
  ROTRS( LFM_DEFAULT_FILTER != m_eLFMode && rcMbDataCurrMot.isIntra() ^ rcMbDataAboveMot.isIntra(), 0 );

//  ROTRS( MSYS_UINT_MAX == rcMbDataAboveMot.getSliceId(), 0);  // not existing !!
  ROTRS( rcMbDataCurrMot. isIntra(),  3 );
  ROTRS( rcMbDataAboveMot.isIntra(),  3 );

  //th
  ROTRS( rcMbDataCurrRes. is4x4BlkResidual( cIdx                            ), 2 );
  ROTRS( rcMbDataAboveRes.is4x4BlkResidual( cIdx + ABOVE_MB_ABOVE_NEIGHBOUR ), 2 ); 

  ROTRS( rcMbDataCurrRes. is4x4BlkCoded( cIdx                            ), 2 );
  ROTRS( rcMbDataAboveRes.is4x4BlkCoded( cIdx + ABOVE_MB_ABOVE_NEIGHBOUR ), 2 );

  return 1;
}

UChar LoopFilter::xCheckMvDataP_RefIdx( const MbData& rcQMbData,
                                        const LumaIdx cQIdx,
                                        const MbData& rcPMbData,
                                        const LumaIdx cPIdx,
                                        const Short   sHorMvThr,
                                        const Short   sVerMvThr,
                                        RefFrameList& rcRefFrameList0,
                                        RefFrameList& rcRefFrameList1  )
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
  Int   iCurrQp = rcMbDataAccess.getMbDataCurr().getQpLF();
  Int   iStride = pcYuvBuffer->getLStride();
  XPel* pPelLum = pcYuvBuffer->getMbLumAddr();

  //===== filtering of left macroblock edge =====
  if( ! m_bVerMixedMode )
  {
    //-----  curr == FRM && left == FRM  or  curr == FLD && left == FLD  -----
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
  else
  {
    Int iLeftQpTop = ( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataLeft     ().getQpLF() : rcMbDataAccess.getMbDataAboveLeft().getQpLF() );
    Int iLeftQpBot = ( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataBelowLeft().getQpLF() : rcMbDataAccess.getMbDataLeft     ().getQpLF() );
    Int iQpTop     = ( iLeftQpTop + iCurrQp + 1) >> 1;
    Int iQpBot     = ( iLeftQpBot + iCurrQp + 1) >> 1;

    Int iIndexATop = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQpTop, 0, 51);
    Int iIndexABot = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQpBot, 0, 51);
    Int iIndexBTop = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQpTop, 0, 51);
    Int iIndexBBot = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQpBot, 0, 51);

    if( ! rcMbDataAccess.getMbData().getFieldFlag() )
    {
      //-----  curr == FRM && left == FLD  -----
      for( Int yBlk = 0; yBlk < 4; yBlk++)
      {
        const UChar ucBsTop = m_aaaucBs[VER][0][yBlk];
        const UChar ucBsBot = m_aucBsVerBot[yBlk];
        if( 0 != ucBsTop )
        {
          xFilter( pPelLum,           1, iIndexATop, iIndexBTop, ucBsTop, true );
          xFilter( pPelLum+2*iStride, 1, iIndexATop, iIndexBTop, ucBsTop, true );
        }
        if( 0 != ucBsBot )
        {
          xFilter( pPelLum+  iStride, 1, iIndexABot, iIndexBBot, ucBsBot, true );
          xFilter( pPelLum+3*iStride, 1, iIndexABot, iIndexBBot, ucBsBot, true );
        }
        pPelLum += 4*iStride;
      }
    }
    else
    {
      //-----  curr == FLD && left == FRM  -----
      Int yBlk;
      for( yBlk = 0; yBlk < 2; yBlk++)
      {
        const UChar ucBsTop = m_aaaucBs[VER][0][yBlk];
        const UChar ucBsBot = m_aucBsVerBot[yBlk];
        if( 0 != ucBsTop )
        {
          xFilter( pPelLum,           1, iIndexATop, iIndexBTop, ucBsTop, true );
          xFilter( pPelLum+  iStride, 1, iIndexATop, iIndexBTop, ucBsTop, true );
        }
        if( 0 != ucBsBot )
        {
          xFilter( pPelLum+2*iStride, 1, iIndexATop, iIndexBTop, ucBsBot, true );
          xFilter( pPelLum+3*iStride, 1, iIndexATop, iIndexBTop, ucBsBot, true );
        }
        pPelLum += 4*iStride;
      }
      for( yBlk = 2; yBlk < 4; yBlk++)
      {
        const UChar ucBsTop = m_aaaucBs[VER][0][yBlk];
        const UChar ucBsBot = m_aucBsVerBot[yBlk];
        if( 0 != ucBsTop )
        {
          xFilter( pPelLum,           1, iIndexABot, iIndexBBot, ucBsTop, true );
          xFilter( pPelLum+  iStride, 1, iIndexABot, iIndexBBot, ucBsTop, true );
        }
        if( 0 != ucBsBot )
        {
          xFilter( pPelLum+2*iStride, 1, iIndexABot, iIndexBBot, ucBsBot, true );
          xFilter( pPelLum+3*iStride, 1, iIndexABot, iIndexBBot, ucBsBot, true );
        }
        pPelLum += 4*iStride;
      }
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
  Int   iCurrQp = rcMbDataAccess.getMbDataCurr().getQpLF();
  Int   iStride = pcYuvBuffer->getLStride();
  XPel* pPelLum = pcYuvBuffer->getMbLumAddr();

  //===== filtering of upper macroblock edge =====
  if( ! m_bHorMixedMode )
  {
    //-----  any other combination than curr = FRM, above = FLD  -----
    Int iAboveQp  = rcMbDataAccess.getMbData().getFieldFlag() && (!rcMbDataAccess.isTopMb()||rcMbDataAccess.getMbDataAboveAbove().getFieldFlag()) ?
      rcMbDataAccess.getMbDataAboveAbove().getQpLF():
    rcMbDataAccess.getMbDataAbove().getQpLF();
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
  else
  {
    //-----  curr = FRM, above = FLD  -----
    AOT_DBG( ! rcMbDataAccess.isTopMb() );
    AOT_DBG( ! rcMbDataAccess.isAboveMbExisting() );

    //===== top field filtering =====
    {
      XPel*  pPelTop     = pcYuvBuffer->getMbLumAddr();
      Int   iTopStride  = pcYuvBuffer->getLStride()*2;

      Int   iAboveQp    = rcMbDataAccess.getMbDataAboveAbove().getQpLF();
      Int   iQp         = ( iAboveQp + iCurrQp + 1) >> 1;
      Int   iIndexA     = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
      Int   iIndexB     = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);

      for( Int xBlk = 0; xBlk < 4; xBlk++)
      {
        const UChar ucBs = m_aucBsHorTop[xBlk];
        if( 0 != ucBs )
        {
          xFilter( pPelTop,   iTopStride, iIndexA, iIndexB, ucBs, true );
          xFilter( pPelTop+1, iTopStride, iIndexA, iIndexB, ucBs, true );
          xFilter( pPelTop+2, iTopStride, iIndexA, iIndexB, ucBs, true );
          xFilter( pPelTop+3, iTopStride, iIndexA, iIndexB, ucBs, true );
        }
        pPelTop += 4;
      }
    }
    //===== bottom field filtering =====
    {
      XPel*  pPelBot     = pcYuvBuffer->getMbLumAddr()+pcYuvBuffer->getLStride();
      Int   iBotStride  = pcYuvBuffer->getLStride()*2;

      Int   iAboveQp    = rcMbDataAccess.getMbDataAbove().getQpLF();
      Int   iQp         = ( iAboveQp + iCurrQp + 1) >> 1;
      Int   iIndexA     = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
      Int   iIndexB     = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);

      for( Int xBlk = 0; xBlk < 4; xBlk++)
      {
        const UChar ucBs = m_aaaucBs[HOR][xBlk][0];
        if( 0 != ucBs )
        {
          xFilter( pPelBot,   iBotStride, iIndexA, iIndexB, ucBs, true );
          xFilter( pPelBot+1, iBotStride, iIndexA, iIndexB, ucBs, true );
          xFilter( pPelBot+2, iBotStride, iIndexA, iIndexB, ucBs, true );
          xFilter( pPelBot+3, iBotStride, iIndexA, iIndexB, ucBs, true );
        }
        pPelBot += 4;
      }
    }
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
  Int   iCurrQp = rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.getMbDataCurr().getQpLF() );
  Int   iStride = pcYuvBuffer->getCStride();
  XPel* pPelCb  = pcYuvBuffer->getMbCbAddr();
  XPel* pPelCr  = pcYuvBuffer->getMbCrAddr();

  //===== filtering of upper macroblock edge =====
  if( ! m_bHorMixedMode )
  {
    //-----  any other combination than curr = FRM, above = FLD  -----
    Int iAboveQp  = rcMbDataAccess.getMbData().getFieldFlag() && (!rcMbDataAccess.isTopMb()||rcMbDataAccess.getMbDataAboveAbove().getFieldFlag()) ?
      rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.getMbDataAboveAbove().getQpLF()):
    rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.getMbDataAbove().getQpLF());
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
  else
  {
    //-----  curr = FRM, above = FLD  -----
    AOT_DBG( ! rcMbDataAccess.isTopMb() );

    //===== top field filtering =====
    {
      /*th
      YuvPicBuffer* pcYuvFieldBuffer = m_pcRecFrameUnit->getTopField().getFullPelYuvBuffer();
      XPel*  pPelField     = bCb ? pcYuvFieldBuffer->getMbCbAddr() : pcYuvFieldBuffer->getMbCrAddr();
      Int   iFieldStride  = pcYuvFieldBuffer->getCStride();
      */
      XPel*  pPelFieldCb   = pcYuvBuffer->getMbCbAddr();
      XPel*  pPelFieldCr   = pcYuvBuffer->getMbCrAddr();
      Int   iFieldStride  = pcYuvBuffer->getCStride() * 2;

      Int   iAboveQp      = rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.getMbDataAboveAbove().getQpLF() );
      Int   iQp           = ( iAboveQp + iCurrQp + 1) >> 1;
      Int   iIndexA       = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
      Int   iIndexB       = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);

      for( Int xBlk = 0; xBlk < 4; xBlk++)
      {
        const UChar ucBs = m_aucBsHorTop[xBlk];
        if( 0 != ucBs )
        {
          xFilter( pPelFieldCb,   iFieldStride, iIndexA, iIndexB, ucBs, false );
          xFilter( pPelFieldCb+1, iFieldStride, iIndexA, iIndexB, ucBs, false );
          xFilter( pPelFieldCr,   iFieldStride, iIndexA, iIndexB, ucBs, false );
          xFilter( pPelFieldCr+1, iFieldStride, iIndexA, iIndexB, ucBs, false );
        }
        pPelFieldCb   += 2;
        pPelFieldCr   += 2;
      }
    }
    //===== bottom field filtering =====
    {
      /*th
      YuvPicBuffer* pcYuvFieldBuffer = m_pcRecFrameUnit->getBotField().getFullPelYuvBuffer();
      XPel*  pPelField   = bCb ? pcYuvFieldBuffer->getMbCbAddr() : pcYuvFieldBuffer->getMbCrAddr();
      Int   iFieldStride  = pcYuvFieldBuffer->getCStride();
      */
      XPel*  pPelFieldCb   = pcYuvBuffer->getMbCbAddr()+pcYuvBuffer->getCStride();
      XPel*  pPelFieldCr   = pcYuvBuffer->getMbCrAddr()+pcYuvBuffer->getCStride();
      Int   iFieldStride  = pcYuvBuffer->getCStride() * 2;

      Int   iAboveQp      = rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.getMbDataAbove().getQpLF() );
      Int   iQp           = ( iAboveQp + iCurrQp + 1) >> 1;
      Int   iIndexA       = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQp, 0, 51);
      Int   iIndexB       = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQp, 0, 51);

      for( Int xBlk = 0; xBlk < 4; xBlk++)
      {
        const UChar ucBs = m_aaaucBs[HOR][xBlk][0];
        if( 0 != ucBs )
        {
          xFilter( pPelFieldCb,   iFieldStride, iIndexA, iIndexB, ucBs, false );
          xFilter( pPelFieldCb+1, iFieldStride, iIndexA, iIndexB, ucBs, false );
          xFilter( pPelFieldCr,   iFieldStride, iIndexA, iIndexB, ucBs, false );
          xFilter( pPelFieldCr+1, iFieldStride, iIndexA, iIndexB, ucBs, false );
        }
        pPelFieldCb   += 2;
        pPelFieldCr   += 2;
      }
    }
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
  Int   iCurrQp = rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.getMbDataCurr().getQpLF() );
  Int   iStride = pcYuvBuffer->getCStride();
  XPel* pPelCb  = pcYuvBuffer->getMbCbAddr();
  XPel* pPelCr  = pcYuvBuffer->getMbCrAddr();

  //===== filtering of left macroblock edge =====
  if( ! m_bVerMixedMode )
  {
    //-----  curr == FRM && left == FRM  or  curr == FLD && left == FLD  -----
    Int iLeftQp = rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.getMbDataLeft().getQpLF() );
    Int iQp     = ( iLeftQp + iCurrQp + 1 ) >> 1;
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
  else
  {
    Int iLeftQpTop = rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataLeft     ().getQpLF() : rcMbDataAccess.getMbDataAboveLeft().getQpLF() );
    Int iLeftQpBot = rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.isTopMb() ? rcMbDataAccess.getMbDataBelowLeft().getQpLF() : rcMbDataAccess.getMbDataLeft     ().getQpLF() );
    Int iQpTop     = ( iLeftQpTop + iCurrQp + 1) >> 1;
    Int iQpBot     = ( iLeftQpBot + iCurrQp + 1) >> 1;
    Int iIndexATop = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQpTop, 0, 51);
    Int iIndexABot = gClipMinMax( rcDFP.getSliceAlphaC0Offset() + iQpBot, 0, 51);
    Int iIndexBTop = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQpTop, 0, 51);
    Int iIndexBBot = gClipMinMax( rcDFP.getSliceBetaOffset()    + iQpBot, 0, 51);

    if( ! rcMbDataAccess.getMbData().getFieldFlag() )
    {
      //-----  curr == FRM && left == FLD  -----
      for( Int yBlk = 0; yBlk < 4; yBlk++)
      {
        const UChar ucBsTop = m_aaaucBs[VER][0][yBlk];
        const UChar ucBsBot = m_aucBsVerBot[yBlk];
        if( 0 != ucBsTop )
        {
          xFilter( pPelCb, 1, iIndexATop, iIndexBTop, ucBsTop, false );
          xFilter( pPelCr, 1, iIndexATop, iIndexBTop, ucBsTop, false );
        }
        if( 0 != ucBsBot )
        {
          xFilter( pPelCb+iStride, 1, iIndexABot, iIndexBBot, ucBsBot, false );
          xFilter( pPelCr+iStride, 1, iIndexABot, iIndexBBot, ucBsBot, false );
        }
        pPelCb += 2*iStride;
        pPelCr += 2*iStride;
      }
    }
    else
    {
      //-----  curr == FLD && left == FRM  -----
      Int yBlk;
      for( yBlk = 0; yBlk < 2; yBlk++)
      {
        const UChar ucBsTop = m_aaaucBs[VER][0][yBlk];
        const UChar ucBsBot = m_aucBsVerBot[yBlk];
        if( 0 != ucBsTop )
        {
          xFilter( pPelCb, 1, iIndexATop, iIndexBTop, ucBsTop, false );
          xFilter( pPelCr, 1, iIndexATop, iIndexBTop, ucBsTop, false );
        }
        if( 0 != ucBsBot )
        {
          xFilter( pPelCb+iStride, 1, iIndexATop, iIndexBTop, ucBsBot, false );
          xFilter( pPelCr+iStride, 1, iIndexATop, iIndexBTop, ucBsBot, false );
        }
        pPelCb   += 2*iStride;
        pPelCr   += 2*iStride;
      }

      for( yBlk = 2; yBlk < 4; yBlk++)
      {
        const UChar ucBsTop = m_aaaucBs[VER][0][yBlk];
        const UChar ucBsBot = m_aucBsVerBot[yBlk];
        if( 0 != ucBsTop )
        {
          xFilter( pPelCb, 1, iIndexABot, iIndexBBot, ucBsTop, false );
          xFilter( pPelCr, 1, iIndexABot, iIndexBBot, ucBsTop, false );
        }
        if( 0 != ucBsBot )
        {
          xFilter( pPelCb+iStride, 1, iIndexABot, iIndexBBot, ucBsBot, false );
          xFilter( pPelCr+iStride, 1, iIndexABot, iIndexBBot, ucBsBot, false );
        }
        pPelCb += 2*iStride;
        pPelCr += 2*iStride;
      }
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

