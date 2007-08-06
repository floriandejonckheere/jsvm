/*
********************************************************************************

NOTE - One of the two copyright statements below may be chosen
       that applies for the software.

********************************************************************************

This software module was based the software developed by

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

/*
********************************************************************************

NOTE - One of the two copyright statements below may be chosen
that applies for the software.

********************************************************************************
This software module was originally created by

Bao, Yiliang (Nokia Research Center, Nokia Inc.)

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

To the extent that Nokia Inc.  owns patent rights that would be required to
make, use, or sell the originally developed software module or portions thereof
included in the ISO/IEC 14496-10:2005 Amd.1 (Scalable Video Coding) in a
conforming product, Nokia Inc.  will assure the ISO/IEC that it is willing
to negotiate licenses under reasonable and non-discriminatory terms and
conditions with applicants throughout the world.

Nokia Inc. retains full right to modify and use the code for its own
purpose, assign or donate the code to a third party and to inhibit third
parties from using the code for products that do not conform to MPEG-related
ITU Recommendations and/or ISO/IEC International Standards.

This copyright notice must be included in all copies or derivative works.
Copyright (c) ISO/IEC 2005.

********************************************************************************

COPYRIGHT AND WARRANTY INFORMATION

Copyright 2005, International Telecommunications Union, Geneva

The Nokia Inc.  hereby donate this source code to the ITU, with the following
understanding:
1. Nokia Inc. retain the right to do whatever they wish with the
contributed source code, without limit.
2. Nokia Inc. retain full patent rights (if any exist) in the technical
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
#include "H264AVCCommonLib/FGSCoder.h"
#include "H264AVCCommonLib/IntFrame.h"

#include "H264AVCCommonLib/CFMO.h"


H264AVC_NAMESPACE_BEGIN


ErrVal
FGSCoder::xInit( YuvBufferCtrl** apcYuvFullPelBufferCtrl,
                 Transform*      pcTransform )
{
  m_papcYuvFullPelBufferCtrl  = apcYuvFullPelBufferCtrl;
  m_pcTransform               = pcTransform;
  m_bInit                     = true;

  m_bPicInit                  = false;
  m_uiWidthInMB               = 0;
  m_uiHeightInMB              = 0;
  m_pcCurrMbDataCtrl          = NULL;

  return Err::m_nOK;
}


ErrVal
FGSCoder::xInitSPS( const SequenceParameterSet& rcSPS )
{
  UInt uiSize = rcSPS.getFrameWidthInMbs() * rcSPS.getFrameHeightInMbs();

  if( uiSize > m_cMbDataCtrlEL.getSize() )
  {
    RNOK( m_cMbDataCtrlEL.uninit() );
    RNOK( m_cMbDataCtrlEL.init  ( rcSPS ) );
  }

  return Err::m_nOK;
}


ErrVal
FGSCoder::xInitBaseLayerSbb( UInt uiLayerId )
{
  YuvBufferCtrl* pcYuvBufferCtrl = m_papcYuvFullPelBufferCtrl[uiLayerId];

  if( m_pcBaseLayerSbb )
  {
    RNOK( m_pcBaseLayerSbb->uninit() );
    delete m_pcBaseLayerSbb;
  }
  ROFS( ( m_pcBaseLayerSbb = new IntFrame( *pcYuvBufferCtrl, *pcYuvBufferCtrl ) ) );
  RNOK( m_pcBaseLayerSbb->init() );
  RNOK( m_pcBaseLayerSbb->setZero() );

  return Err::m_nOK;
}


ErrVal
FGSCoder::xUninit()
{
  m_cMbDataCtrlEL.uninit();

  m_bInit                     = false;
  m_papcYuvFullPelBufferCtrl  = 0;
  m_pcTransform               = 0;

  m_bPicInit                  = false;
  m_uiWidthInMB               = 0;
  m_uiHeightInMB              = 0;
  m_pcCurrMbDataCtrl          = 0;

  if( m_pcBaseLayerSbb )
  {
    RNOK( m_pcBaseLayerSbb->uninit() );
    delete m_pcBaseLayerSbb;
    m_pcBaseLayerSbb = 0;
  }

  return Err::m_nOK;
}


//--ICU/ETRI FMO 1206
ErrVal
FGSCoder::xInitializeCodingPath(SliceHeader* pcSliceHeader)
{
	//--ICU/ETRI FMO Implementation 1206
  UInt uiFirstMbInSlice;
  UInt uiLastMbInSlice;

  FMO* pcFMO = pcSliceHeader->getFMO();
  for(Int iSliceGroupID=0;!pcFMO->SliceGroupCompletelyCoded(iSliceGroupID);iSliceGroupID++)  
  {
    if (false == pcFMO->isCodedSG(iSliceGroupID))
    {
    continue;
    }

    uiFirstMbInSlice = pcFMO->getFirstMacroblockInSlice(iSliceGroupID);
    uiLastMbInSlice = pcFMO->getLastMBInSliceGroup(iSliceGroupID);

    for(UInt uiMbAddress= uiFirstMbInSlice ;uiMbAddress<=uiLastMbInSlice ;)
    {
      UInt uiMbY  = uiMbAddress / m_uiWidthInMB;
      UInt uiMbX  = uiMbAddress % m_uiWidthInMB;
    
      MbDataAccess* pcMbDataAccess = 0;
      RNOK( m_pcCurrMbDataCtrl->initMb( pcMbDataAccess, uiMbY, uiMbX ) );

      if( !pcMbDataAccess->getMbData().isIntra() )
        pcMbDataAccess->getMbData().activateMotionRefinement();

      if(pcSliceHeader !=NULL)
        uiMbAddress = pcSliceHeader->getFMO()->getNextMBNr(uiMbAddress );
      else
        uiMbAddress ++;
    }
  }

  return Err::m_nOK;
}




ErrVal
FGSCoder::xScale4x4Block( TCoeff*            piCoeff,
                          const UChar*       pucScale,
                          UInt               uiStart,
                          const QpParameter& rcQP )
{
  if( pucScale )
  {
    Int iAdd = ( rcQP.per() <= 3 ? ( 1 << ( 3 - rcQP.per() ) ) : 0 );
    
    for( UInt ui = uiStart; ui < 16; ui++ )
    {
      piCoeff[ui] = ( ( piCoeff[ui] * g_aaiDequantCoef[rcQP.rem()][ui] * pucScale[ui] + iAdd ) << rcQP.per() ) >> 4;
    }
  }
  else
  {
    for( UInt ui = uiStart; ui < 16; ui++ )
    {
      piCoeff[ui] *= ( g_aaiDequantCoef[rcQP.rem()][ui] << rcQP.per() );
    }
  }

  return Err::m_nOK;
}


ErrVal
FGSCoder::xScale8x8Block( TCoeff*            piCoeff,
                          const UChar*       pucScale,
                          const QpParameter& rcQP )
{
  Int iAdd = ( rcQP.per() <= 5 ? ( 1 << ( 5 - rcQP.per() ) ) : 0 );

  if( pucScale )
  {
    for( UInt ui = 0; ui < 64; ui++ )
    {
      piCoeff[ui] = ( ( piCoeff[ui] * g_aaiDequantCoef64[rcQP.rem()][ui] * pucScale[ui] + iAdd ) << rcQP.per() ) >> 6;
    }
  }
  else
  {
    for( UInt ui = 0; ui < 64; ui++ )
    {
      piCoeff[ui] = ( ( piCoeff[ui] * g_aaiDequantCoef64[rcQP.rem()][ui] * 16 + iAdd ) << rcQP.per() ) >> 6;
    }
  }

  return Err::m_nOK;
}



ErrVal
FGSCoder::xReconstructMacroblock( MbDataAccess&   rcMbDataAccess,
                                  IntYuvMbBuffer& rcMbBuffer )
{
  m_pcTransform->setClipMode( false );

  Int                 iLStride  = rcMbBuffer.getLStride();
  Int                 iCStride  = rcMbBuffer.getCStride();
  Bool                b8x8      = rcMbDataAccess.getMbData().isTransformSize8x8();
  MbTransformCoeffs   cCoeffs;
  cCoeffs.copyFrom( rcMbDataAccess.getMbTCoeffs() );

  rcMbBuffer.loadBuffer( m_pcBaseLayerSbb->getPic(rcMbDataAccess.getMbPicType())->getFullPelYuvBuffer() );//TMM_INTERLACE

  Bool                bIntra    = rcMbDataAccess.getMbData().isIntra();
  UInt                uiUScalId = ( bIntra ? 1 : 4 );
  UInt                uiVScalId = ( bIntra ? 2 : 5 );
  const UChar*        pucScaleU = rcMbDataAccess.getSH().getScalingMatrix( uiUScalId );
  const UChar*        pucScaleV = rcMbDataAccess.getSH().getScalingMatrix( uiVScalId );

//  TCoeff lumaDcCoeffs[16];
  if ( rcMbDataAccess.getMbData().isIntra16x16() )
  {
    Quantizer cQuantizer;
    cQuantizer.setQp( rcMbDataAccess, false );
    const QpParameter&  cLQp      = cQuantizer.getLumaQp  ();
    const UChar*        pucScaleY = rcMbDataAccess.getSH().getScalingMatrix( 0 );

    //===== INTRA_16x16 =====
    for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
    {
      RNOK( xScale4x4Block( cCoeffs.get( cIdx ), pucScaleY, 1, cLQp ) );
    }
    const Int aaiDequantDcCoef[6] = {  10, 11, 13, 14, 16, 18 };

    Int iScale  = aaiDequantDcCoef[cLQp.rem()] << cLQp.per();
    if( pucScaleY )
    {
      iScale  *= pucScaleY[0];
      iScale >>= 4;
    }
    // perform scaling only
    TCoeff* piCoeff = cCoeffs.get( B4x4Idx(0) );

    for( Int uiDCIdx = 0; uiDCIdx < 16; uiDCIdx++ )
    {
      TCoeff a = piCoeff[16*uiDCIdx] * iScale;
      Int    b = piCoeff[16*uiDCIdx] * iScale;
      if( a != b )
      {
        printf("Short overflow in Intra16x16 DC-coeffs.\n");
        piCoeff[16*uiDCIdx] = max( (Int)MSYS_SHORT_MIN, min( (Int)MSYS_SHORT_MAX, b ) );
      }
      else
      {
        piCoeff[16*uiDCIdx] *= iScale;
      }
    }

    const QpParameter&  cCQp      = cQuantizer.getChromaQp();
    //===== chroma =====
    for( CIdx cIdx; cIdx.isLegal(); cIdx++ )
    {
      RNOK( xScale4x4Block( cCoeffs.get( cIdx ), ( cIdx.plane() ? pucScaleV : pucScaleU ), 1, cCQp ) );
    }

    iScale = g_aaiDequantCoef[cCQp.rem()][0] << cCQp.per();
    for( CIdx cCIdx; cCIdx.isLegal(); cCIdx++ )
    {
      cCoeffs.get( cCIdx )[0] *= iScale;
    }



//     // backup luma DC
    TCoeff *piCoeffs;
//     UInt   uiDCIdx;
    piCoeffs = cCoeffs.get( B4x4Idx(0) );
//     for( uiDCIdx = 0; uiDCIdx < 16; uiDCIdx++ )
//       lumaDcCoeffs[uiDCIdx] = piCoeffs[16*uiDCIdx] ;

    // inverse transform on luma DC
    RNOK( m_pcTransform->invTransformDcCoeff( piCoeffs, 1 ) );

    // inverse transform on entire MB
    for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
    {
      RNOK( m_pcTransform->invTransform4x4Blk( rcMbBuffer.getYBlk( cIdx ), iLStride, cCoeffs.get( cIdx ) ) );
    }

//     // restore luma DC
//     for( uiDCIdx = 0; uiDCIdx < 16; uiDCIdx++ )
//       piCoeffs[16*uiDCIdx] = lumaDcCoeffs[uiDCIdx];
  }
  else if( b8x8 )
  {
    for( B8x8Idx cIdx; cIdx.isLegal(); cIdx++ )
    {
      RNOK( m_pcTransform->invTransform8x8Blk( rcMbBuffer.getYBlk( cIdx ), iLStride, cCoeffs.get8x8( cIdx ) ) );
    }
  }
  else
  {
    for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
    {
      RNOK( m_pcTransform->invTransform4x4Blk( rcMbBuffer.getYBlk( cIdx ), iLStride, cCoeffs.get( cIdx ) ) );
    }
  }


  if( !rcMbDataAccess.getMbData().isIntra_BL() )
  {
    Int                 iScale;
    // scaling has already been performed on DC coefficients
    iScale = ( pucScaleU ? pucScaleU[0] : 16 );
    m_pcTransform->invTransformChromaDc( cCoeffs.get( CIdx(0) ), iScale );     
    iScale = ( pucScaleV ? pucScaleV[0] : 16 );
    m_pcTransform->invTransformChromaDc( cCoeffs.get( CIdx(4) ), iScale );
  }

  RNOK( m_pcTransform->invTransformChromaBlocks( rcMbBuffer.getMbCbAddr(), iCStride, cCoeffs.get( CIdx(0) ) ) );
  RNOK( m_pcTransform->invTransformChromaBlocks( rcMbBuffer.getMbCrAddr(), iCStride, cCoeffs.get( CIdx(4) ) ) );

  m_pcTransform->setClipMode( true );

  return Err::m_nOK;
}



ErrVal
FGSCoder::reconstruct( IntFrame* pcRecResidual, Bool bDecoder )
{
  ROF( m_bInit );
  ROF( m_bPicInit );
  ROF( pcRecResidual );

  UInt            uiLayer         = m_pcSliceHeader->getLayerId(); 
  YuvBufferCtrl*  pcYuvBufferCtrl = m_papcYuvFullPelBufferCtrl[ uiLayer ];
  IntYuvMbBuffer  cMbBuffer;

  RNOK( m_pcCurrMbDataCtrl->initSlice ( *m_pcSliceHeader, PRE_PROCESS, bDecoder, NULL ) );

	const PicType ePicType = m_pcSliceHeader->getPicType();
	const Bool    bMbAff   = m_pcSliceHeader->isMbAff   ();

	if( ePicType!=FRAME )
	{
		RNOK( pcRecResidual->addFieldBuffer     ( ePicType ) );
		RNOK( m_pcBaseLayerSbb->addFieldBuffer( ePicType ) );//TMM_INTERLACE
	}
	else if( bMbAff )
	{
		RNOK( pcRecResidual->addFrameFieldBuffer() );
		RNOK( m_pcBaseLayerSbb->addFrameFieldBuffer() );//TMM_INTERLACE
	}

	//===== loop over macroblocks =====
 for( UInt uiMbY = 0; uiMbY < m_uiHeightInMB; uiMbY++ )
  for( UInt uiMbX = 0; uiMbX < m_uiWidthInMB;  uiMbX++ )
  {
    MbDataAccess* pcMbDataAccessBL = NULL;
   	RNOK( m_pcCurrMbDataCtrl->initMb( pcMbDataAccessBL, uiMbY, uiMbX         ) );
    RNOK( pcYuvBufferCtrl   ->initMb(                   uiMbY, uiMbX, bMbAff ) );

    RNOK( xReconstructMacroblock    ( *pcMbDataAccessBL, cMbBuffer           ) );
		const PicType eMbPicType = pcMbDataAccessBL->getMbPicType();
		RNOK( pcRecResidual->getPic( eMbPicType )->getFullPelYuvBuffer()->loadBuffer( &cMbBuffer ) );
  }

	if( ePicType!=FRAME )
	{
		RNOK( pcRecResidual->removeFieldBuffer     ( ePicType ) );
	}
	else if( bMbAff )
	{
		RNOK( pcRecResidual->removeFrameFieldBuffer()           );
	}

  return Err::m_nOK;
}

H264AVC_NAMESPACE_END
