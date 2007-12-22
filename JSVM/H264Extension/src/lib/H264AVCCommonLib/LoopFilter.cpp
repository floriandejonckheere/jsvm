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
#include "H264AVCCommonLib/YuvPicBuffer.h"
#include "H264AVCCommonLib/Frame.h"

#include "H264AVCCommonLib/LoopFilter.h"
#include "H264AVCCommonLib/ReconstructionBypass.h"
#include "H264AVCCommonLib/YuvMbBuffer.h"

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
	m_pcHighpassYuvBuffer( NULL )	//TMM_INTERLACE
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
                         ,Bool                   bEncoder
                         )
{
  ROT( NULL == pcControlMngIf );
  ROT( NULL == pcReconstructionBypass );
  m_pcReconstructionBypass = pcReconstructionBypass;

  m_pcControlMngIf  = pcControlMngIf;
  m_pcHighpassFrame = NULL;
  m_bEncoder = bEncoder;
  return Err::m_nOK;
}

ErrVal LoopFilter::uninit()
{
  m_pcControlMngIf = NULL;
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



//JVT-X046 {
ErrVal LoopFilter::process( SliceHeader&  rcSH,
                            Frame*     pcIntFrame,
                            MbDataCtrl*   pcMbDataCtrlMot, // if NULL -> all intra
                            MbDataCtrl*   pcMbDataCtrlRes,
                            UInt          uiMbInRow,
                            RefFrameList* pcRefFrameList0,
                            RefFrameList* pcRefFrameList1,			
                            bool		  bAllSliceDone, 
                            bool          spatial_scalable_flg
                            )
{
  bool enhancedLayerFlag = (rcSH.getDependencyId()>0) ; //V032 FSL added for disabling chroma deblocking
 
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

  Frame* apcFrame        [4] = { NULL, NULL, NULL, NULL };
  Frame* apcHighpassFrame[4] = { NULL, NULL, NULL, NULL };

  if( NULL!= pcIntFrame )
  {
		RNOK( pcIntFrame->addFrameFieldBuffer() );
    apcFrame[ TOP_FIELD ] = pcIntFrame->getPic( TOP_FIELD );
    apcFrame[ BOT_FIELD ] = pcIntFrame->getPic( BOT_FIELD );
		apcFrame[ FRAME     ] = pcIntFrame->getPic( FRAME     );
  }

  if( NULL != m_pcHighpassFrame )
  {
		RNOK( m_pcHighpassFrame->addFrameFieldBuffer() );
    apcHighpassFrame[ TOP_FIELD ] = m_pcHighpassFrame->getPic( TOP_FIELD );
    apcHighpassFrame[ BOT_FIELD ] = m_pcHighpassFrame->getPic( BOT_FIELD );
		apcHighpassFrame[ FRAME     ] = m_pcHighpassFrame->getPic( FRAME     );
  }

  const PicType ePicType = rcSH.getPicType();
  const Bool    bMbAff   = rcSH.isMbaffFrame();
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
        RNOK( xRecalcCBP( *pcMbDataAccessRes ) );
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
 
    //TMM_INTERLACE {
    if( m_eLFMode & LFM_EXTEND_INTRA_SUR ) 
    {
      if(bMbAff) 
      {
	for(uiMbAddress= uiFirstMbInSlice ;uiMbAddress<=uiLastMbInSlice ;)  
	{
     UInt          uiMbY             = uiMbAddress / uiMbInRow;
     UInt          uiMbX             = uiMbAddress % uiMbInRow;

     MbDataAccess* pcMbDataAccessRes = 0;

     RNOK(   pcMbDataCtrlRes ->initMb            (  pcMbDataAccessRes, uiMbY, uiMbX ) );
          RNOK(   m_pcControlMngIf->initMbForFiltering(  *pcMbDataAccessRes, uiMbY, uiMbX, true  ) ); 

          UInt uiMask = 0;
          PicType eMbPicType = ((uiMbY%2) ? BOT_FIELD : TOP_FIELD);

          RNOK( pcMbDataCtrlRes->getBoundaryMask_MbAff( uiMbY, uiMbX, uiMask ) );

          if( uiMask /*&& eMbPicType!=BOT_FIELD*/)
          {
            IntYuvMbBufferExtension cBuffer;
            cBuffer.setAllSamplesToZero();
            cBuffer.loadSurrounding_MbAff( apcFrame        [ eMbPicType ]->getFullPelYuvBuffer(), uiMask );
            RNOK( m_pcReconstructionBypass->padRecMb_MbAff( &cBuffer                            , uiMask ));
            apcFrame        [ eMbPicType ]->getFullPelYuvBuffer()->loadBuffer_MbAff( &cBuffer   , uiMask );
          }

          uiMbAddress = rcSH.getFMO()->getNextMBNr(uiMbAddress );
        }
      }
      else
  	  {
  
        for(uiMbAddress= uiFirstMbInSlice ;uiMbAddress<=uiLastMbInSlice ;)  
        {
          UInt          uiMbY            = uiMbAddress / uiMbInRow;
          UInt          uiMbX            = uiMbAddress % uiMbInRow;

          MbDataAccess* pcMbDataAccessRes = 0;

          RNOK(   pcMbDataCtrlRes ->initMb            (  pcMbDataAccessRes, uiMbY, uiMbX ) );
          RNOK(   m_pcControlMngIf->initMbForFiltering(  *pcMbDataAccessRes, uiMbY, uiMbX, false  ) ); 

          UInt uiMask = 0;
        RNOK( pcMbDataCtrlRes->getBoundaryMask( uiMbY, uiMbX, uiMask ) );
  
        if( uiMask )
  		  {
          IntYuvMbBufferExtension cBuffer;
          cBuffer.setAllSamplesToZero();
            cBuffer.loadSurrounding( apcFrame        [ ePicType ]->getFullPelYuvBuffer());
          RNOK( m_pcReconstructionBypass->padRecMb( &cBuffer, uiMask ) );
            apcFrame        [ ePicType ]->getFullPelYuvBuffer()->loadBuffer( &cBuffer);
          }
      		
          uiMbAddress = rcSH.getFMO()->getNextMBNr(uiMbAddress );
  		  }
  	  }
    }
   //TMM_INTERLACE }
  }
  //<- ICU/ETRI DS

  // Hanke@RWTH: Reset pointer
  setHighpassFramePointer();
  m_pcHighpassYuvBuffer = NULL; // fix (HS)

  return Err::m_nOK;
}


//JVT-X046 }
__inline ErrVal LoopFilter::xFilterMb( MbDataAccess*        pcMbDataAccessMot,
                                       MbDataAccess*        pcMbDataAccessRes,
                                       YuvPicBuffer*     pcYuvBuffer,
                                       RefFrameList*        pcRefFrameList0,
                                       RefFrameList*        pcRefFrameList1,
                                       bool                 spatial_scalable_flg,   // SSUN@SHARP
									                     bool					enhancedLayerFlag)                //V032 of FSL
{
  const DBFilterParameter& rcDFP = ( m_eLFMode & LFM_NO_INTER_FILTER ? pcMbDataAccessRes->getSH().getInterLayerDeblockingFilterParameter() : pcMbDataAccessRes->getSH().getDeblockingFilterParameter() );
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
      if( rcMbDataCurr.isIntraBL() )
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
      if(rcMbDataCurr.isIntraBL() && rcMbDataLeft.isIntraBL())
      {
        ROTRS( rcMbDataCurr.is4x4BlkCoded( cIdx ), 1 );
        ROTRS( rcMbDataLeft.is4x4BlkCoded( cIdx + LEFT_MB_LEFT_NEIGHBOUR), 1 );
        return(0);
      }
      else if(rcMbDataCurr.isIntraBL()){
        ROTRS( rcMbDataLeft.isIntraButnotIBL(), 4 );
        ROTRS( rcMbDataCurr.is4x4BlkCoded( cIdx ), 2 );
        //th
        ROTRS( rcMbDataLeft.is4x4BlkCoded   ( cIdx + LEFT_MB_LEFT_NEIGHBOUR), 2 );
        ROTRS( rcMbDataLeft.is4x4BlkResidual( cIdx + LEFT_MB_LEFT_NEIGHBOUR), 2 ); 

        return(1);
      }
      else if(rcMbDataLeft.isIntraBL())
      {
        ROTRS( rcMbDataCurr.isIntraButnotIBL(), 4 );
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


  // current macroblock is a field macroblock
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
      if( rcMbDataCurr.isIntraBL() )
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
      if(rcMbDataCurr.isIntraBL() && rcMbDataAbove.isIntraBL())
      {
        ROTRS( rcMbDataCurr.is4x4BlkCoded( cIdx ), 1 );
        ROTRS( rcMbDataAbove.is4x4BlkCoded( cIdx + ABOVE_MB_ABOVE_NEIGHBOUR), 1 );
        return(0);
      }
      if(rcMbDataCurr.isIntraBL())
      {
        ROTRS( rcMbDataAbove.isIntraButnotIBL(), 4 );
        ROTRS( rcMbDataCurr.is4x4BlkCoded( cIdx ), 2 );
        //th
        ROTRS( rcMbDataAbove.is4x4BlkCoded( cIdx + ABOVE_MB_ABOVE_NEIGHBOUR), 2 );
        ROTRS( rcMbDataAbove.is4x4BlkResidual( cIdx + ABOVE_MB_ABOVE_NEIGHBOUR), 2 ); 

        return(1);
      }
      else if(rcMbDataAbove.isIntraBL())
      {
        ROTRS( rcMbDataCurr.isIntraButnotIBL(), 4 );
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

  Frame* pcRefPicL0Q = rcRefFrameList0[ rcMbMotionDataL0Q.getRefIdx( cQIdx ) ];
  Frame* pcRefPicL0P = rcRefFrameList0[ rcMbMotionDataL0P.getRefIdx( cPIdx ) ];

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

  Frame* pcRefPicL0Q = rcRefFrameList0[ rcMbMotionDataL0Q.getRefIdx( cQIdx ) ];
  Frame* pcRefPicL1Q = rcRefFrameList1[ rcMbMotionDataL1Q.getRefIdx( cQIdx ) ];
  Frame* pcRefPicL0P = rcRefFrameList0[ rcMbMotionDataL0P.getRefIdx( cPIdx ) ];
  Frame* pcRefPicL1P = rcRefFrameList1[ rcMbMotionDataL1P.getRefIdx( cPIdx ) ];

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
    Frame* pcRefPicQ = ( pcRefPicL0Q ? pcRefPicL0Q : pcRefPicL1Q );
    Frame* pcRefPicP = ( pcRefPicL0P ? pcRefPicL0P : pcRefPicL1P );
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

__inline ErrVal LoopFilter::xLumaVerFiltering( const MbDataAccess& rcMbDataAccess, const DBFilterParameter& rcDFP, YuvPicBuffer* pcYuvBuffer )
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

__inline ErrVal LoopFilter::xLumaHorFiltering( const MbDataAccess& rcMbDataAccess, const DBFilterParameter& rcDFP, YuvPicBuffer* pcYuvBuffer )
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

__inline ErrVal LoopFilter::xChromaHorFiltering( const MbDataAccess& rcMbDataAccess, const DBFilterParameter& rcDFP, YuvPicBuffer* pcYuvBuffer )
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

__inline ErrVal LoopFilter::xChromaVerFiltering( const MbDataAccess& rcMbDataAccess, const DBFilterParameter& rcDFP, YuvPicBuffer* pcYuvBuffer )
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

