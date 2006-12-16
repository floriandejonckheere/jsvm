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
  m_pcCurrMbDataCtrl          = 0;

  m_pcCoefMap = NULL;
  m_pcBQCoefMap = NULL;
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

    delete[] m_pcCoefMap;
    m_pcCoefMap = new MbFGSCoefMap[uiSize];
    delete[] m_pcBQCoefMap;
    m_pcBQCoefMap = new MbFGSCoefMap[uiSize];
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

  delete[] m_pcCoefMap;
  delete[] m_pcBQCoefMap;
  if( m_pcBaseLayerSbb )
  {
    RNOK( m_pcBaseLayerSbb->uninit() );
    delete m_pcBaseLayerSbb;
    m_pcBaseLayerSbb = 0;
  }

  return Err::m_nOK;
}


ErrVal
FGSCoder::xStoreBQLayerSigMap()
{
  UInt uiSize = m_uiWidthInMB * m_uiHeightInMB;
  memcpy( m_pcBQCoefMap, m_pcCoefMap, uiSize*sizeof(MbFGSCoefMap) );

  return Err::m_nOK;
}

ErrVal
FGSCoder::xSwitchBQLayerSigMap()
{
  MbFGSCoefMap *switch_temp = m_pcCoefMap;
  m_pcCoefMap = m_pcBQCoefMap;
  m_pcBQCoefMap = switch_temp;

  return Err::m_nOK;
}


//--ICU/ETRI FMO 1206
ErrVal
FGSCoder::xInitializeCodingPath(SliceHeader* pcSliceHeader)
{
  //--ICU/ETRI FMO Implementation 1206
  UInt uiFirstMbInSlice;
  UInt uiLastMbInSlice;

  if(pcSliceHeader !=NULL)
  {
    uiFirstMbInSlice  = pcSliceHeader->getFirstMbInSlice();
    uiLastMbInSlice  = pcSliceHeader->getLastMbInSlice();
  }
  else
  {
    uiFirstMbInSlice =0;
    uiLastMbInSlice  = (m_uiWidthInMB*m_uiHeightInMB) -1;

  }


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

      MbData& rcMbData        = pcMbDataAccess->getMbData();
      UInt    uiMbIndex       = uiMbY * m_uiWidthInMB + uiMbX;
      Bool    bIntra4x4       =     rcMbData.isIntra4x4   ();
      Bool    bIntra16x16     =     rcMbData.isIntra16x16 ();
      Bool    bIsSignificant  = (   rcMbData.getMbCbp()          > 0 );
      Bool    bIsSigLuma      = ( ( rcMbData.getMbCbp() & 0x0F ) > 0 );
      Bool    b8x8Present     = (   pcMbDataAccess->getSH().getPPS().getTransform8x8ModeFlag() &&
                                    rcMbData.is8x8TrafoFlagPresent() );
      Bool    b8x8Transform   = ( b8x8Present && ( bIsSigLuma || bIntra4x4 ) && rcMbData.isTransformSize8x8() );
      UInt    uiMbCbp         = pcMbDataAccess->getAutoCbp();

      if( ! pcMbDataAccess->getMbData().isIntra() )
        pcMbDataAccess->getMbData().activateMotionRefinement();
      //===== set macroblock mode =====

      MbFGSCoefMap &rcMbFGSCoefMap = m_pcCoefMap[uiMbIndex];

      rcMbFGSCoefMap.resetNumCoded();
      rcMbFGSCoefMap.resetMbRefCtx();
      rcMbFGSCoefMap.resetMbCoefMap();

      rcMbFGSCoefMap.getMbMap() = ( bIntra16x16 || bIsSignificant                           ? SIGNIFICANT         : CLEAR )
                                + ( bIntra16x16 || bIntra4x4 || bIsSigLuma || !b8x8Present  ? TRANSFORM_SPECIFIED : CLEAR );
      //--- LUMA ---
      for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
      {
        rcMbFGSCoefMap.getB8x8Map( c8x8Idx ) = ( ( uiMbCbp & ( 1 << c8x8Idx.b8x8Index() ) ) > 0 ? SIGNIFICANT : CLEAR );
        if( b8x8Transform )
        {
          if( pcMbDataAccess->getSH().getPPS().getEntropyCodingModeFlag() )
          {
            for( S4x4Idx cIdx( c8x8Idx ); cIdx.isLegal( c8x8Idx ); cIdx++ )
            {
              rcMbFGSCoefMap.getLumaScanPos( cIdx ) =  ((cIdx.x()%2)==0 && (cIdx.y()%2) == 0) ? 64 : 0;
              rcMbFGSCoefMap.getB4x4Map( cIdx ) = rcMbFGSCoefMap.getB8x8Map( c8x8Idx );
            }
          }
          else
          {
            for( S4x4Idx cIdx( c8x8Idx ); cIdx.isLegal( c8x8Idx ); cIdx++ )
            {
              rcMbFGSCoefMap.getLumaScanPos( cIdx ) = 16;
              rcMbFGSCoefMap.getB4x4Map( cIdx ) = rcMbFGSCoefMap.getB8x8Map( c8x8Idx );
            }
          }
          //===== set transform coefficients =====
          CoefMap *pcCoefMap = rcMbFGSCoefMap.getCoefMap( c8x8Idx );
          TCoeff  *piCoeff   = rcMbData.getMbTCoeffs().get8x8( c8x8Idx );
          for( UInt ui8x8ScanIndex = 0; ui8x8ScanIndex < 64; ui8x8ScanIndex++ )
          {
            if( 0 != piCoeff[g_aucFrameScan64[ui8x8ScanIndex]] )
            {
              pcCoefMap[ui8x8ScanIndex] = SIGNIFICANT;
              if (piCoeff[g_aucFrameScan64[ui8x8ScanIndex]] < 0)
                pcCoefMap[ui8x8ScanIndex] |= BASE_SIGN;
            }
            if( pcMbDataAccess->getSH().getPPS().getEntropyCodingModeFlag() )
            {
              S4x4Idx cIdx2 = S4x4Idx( c8x8Idx ) ;
              if( !( pcCoefMap[ui8x8ScanIndex] & (SIGNIFICANT|CODED) ) && rcMbFGSCoefMap.getLumaScanPos( cIdx2 ) == 64 )
                rcMbFGSCoefMap.getLumaScanPos( cIdx2 ) = ui8x8ScanIndex;
            }
            else
            {
              S4x4Idx cIdx2 = S4x4Idx( c8x8Idx ) + (ui8x8ScanIndex & 3);
              if( !( pcCoefMap[ui8x8ScanIndex] & (SIGNIFICANT|CODED) ) && rcMbFGSCoefMap.getLumaScanPos( cIdx2 ) == 16 )
                rcMbFGSCoefMap.getLumaScanPos( cIdx2 ) = ui8x8ScanIndex >> 2;
            }
          }
        }
        else
        {
          for( S4x4Idx cIdx( c8x8Idx ); cIdx.isLegal( c8x8Idx ); cIdx++ )
          {
            TCoeff* piCoeff     = rcMbData.getMbTCoeffs().get( cIdx );
            UChar   ucBlockSig  = CLEAR;

            UChar& rucScanPos = rcMbFGSCoefMap.getLumaScanPos( cIdx );
            rucScanPos = 16;
            CoefMap* pcCoefMap = rcMbFGSCoefMap.getCoefMap( cIdx );
            //===== set transform coefficients =====
            for( UInt uiScanIndex = 0; uiScanIndex < 16; uiScanIndex++ )
            {
              if( piCoeff[g_aucFrameScan[uiScanIndex]] )
              {
                pcCoefMap[uiScanIndex]  = SIGNIFICANT;
                ucBlockSig              = SIGNIFICANT;

                if (piCoeff[g_aucFrameScan[uiScanIndex]] < 0)
                  pcCoefMap[uiScanIndex] |= BASE_SIGN;
              }

              if( !( pcCoefMap[uiScanIndex] & (SIGNIFICANT|CODED) ) && rucScanPos == 16 )
                rucScanPos = uiScanIndex;
            }

            //===== set block mode =====
            rcMbFGSCoefMap.getB4x4Map( cIdx ) = ucBlockSig;
          }
        }
      }

      //--- CHROMA DC ---
      for( CPlaneIdx cCPlaneIdx; cCPlaneIdx.isLegal(); ++cCPlaneIdx )
      {
        UChar ucBlockSig = CLEAR;
        UChar &rucScanPos = rcMbFGSCoefMap.getChromaDCScanPos( cCPlaneIdx );
        rucScanPos = 4;
        for( CIdx cCIdx( cCPlaneIdx ); cCIdx.isLegal( cCPlaneIdx ); cCIdx++ )
        {
          TCoeff   iCoeff    = rcMbData.getMbTCoeffs().get( cCIdx )[0];
          CoefMap& rcCoefMap = rcMbFGSCoefMap.getCoefMap( cCIdx )[0];
          if( 0 != iCoeff )
          {
            rcCoefMap  = SIGNIFICANT;
            ucBlockSig = SIGNIFICANT;

            if( iCoeff < 0 )
              rcCoefMap |= BASE_SIGN;
          }

          if( !( rcCoefMap & (SIGNIFICANT|CODED) ) && rucScanPos == 4 )
            rucScanPos = cCIdx&3;
        }
        rcMbFGSCoefMap.getChromaDCMbMap( cCPlaneIdx ) = ucBlockSig;
      }
      //--- CHROMA AC ---
      for( CIdx cCIdx; cCIdx.isLegal(); cCIdx++ )
      {
        TCoeff* piCoeff     = rcMbData.getMbTCoeffs().get( cCIdx );
        UChar   ucBlockSig  = CLEAR;
        CoefMap* pcCoefMap = rcMbFGSCoefMap.getCoefMap( cCIdx );
        UChar &rucScanPos = rcMbFGSCoefMap.getChromaACScanPos( cCIdx );
        rucScanPos = 16;
        for( UInt ui = 1; ui < 16; ui++ )
        {
          if( piCoeff[g_aucFrameScan[ui]] )
          {
            pcCoefMap[ui] = SIGNIFICANT;
            ucBlockSig    = SIGNIFICANT;
            if (piCoeff[g_aucFrameScan[ui]] < 0)
              pcCoefMap[ui] |= BASE_SIGN;
          }

          if( !( pcCoefMap[ui] & (SIGNIFICANT|CODED) ) && rucScanPos == 16 )
            rucScanPos = ui;
        }
        rcMbFGSCoefMap.getChromaACBlockMap( cCIdx ) = ucBlockSig;
      }

      //--ICU/ETRI FMO Implementation
      if(pcSliceHeader !=NULL)
        uiMbAddress = pcSliceHeader->getFMO()->getNextMBNr(uiMbAddress );
      else
        uiMbAddress ++;
    }
  }

  return Err::m_nOK;
}


// FGS FMO ICU/ETRI
ErrVal
FGSCoder::xUpdateCodingPath(SliceHeader* pcSliceHeader)
{
  //--ICU/ETRI FMO Implementation 1206
  UInt uiFirstMbInSlice;
  UInt uiLastMbInSlice;

  if(pcSliceHeader !=NULL)
  {
    uiFirstMbInSlice  = pcSliceHeader->getFirstMbInSlice();
    uiLastMbInSlice  = pcSliceHeader->getLastMbInSlice();
  }
  else
  {
    uiFirstMbInSlice =0;
    uiLastMbInSlice  = (m_uiWidthInMB*m_uiHeightInMB) -1;

  }

  for(UInt uiMbAddress= uiFirstMbInSlice ;uiMbAddress<=uiLastMbInSlice ;)
  {
    UInt uiMbY  = uiMbAddress / m_uiWidthInMB;
    UInt uiMbX  = uiMbAddress % m_uiWidthInMB;

    MbDataAccess* pcMbDataAccessBL = 0;
    MbDataAccess* pcMbDataAccessEL = 0;
    RNOK( m_pcCurrMbDataCtrl->initMb( pcMbDataAccessBL, uiMbY, uiMbX ) );
    RNOK( m_cMbDataCtrlEL    .initMb( pcMbDataAccessEL, uiMbY, uiMbX ) );

    //===== scale enhancement layer coefficients =====
    pcMbDataAccessEL->getMbData().setTransformSize8x8( pcMbDataAccessBL->getMbData().isTransformSize8x8() );
    RNOK( xScaleTCoeffs( *pcMbDataAccessEL, false ) );
    //===== update coefficients, CBP, QP =====
    RNOK( xUpdateMacroblock( *pcMbDataAccessBL, *pcMbDataAccessEL, uiMbY, uiMbX ) );

    //--ICU/ETRI FMO Implementation
    if(pcSliceHeader !=NULL)
      uiMbAddress = pcSliceHeader->getFMO()->getNextMBNr(uiMbAddress );
    else
      uiMbAddress ++;

  }
  return Err::m_nOK;
}



ErrVal
FGSCoder::xInitializeCodingPath()
{
  for( UInt uiMbY = 0; uiMbY < m_uiHeightInMB; uiMbY++ )
  for( UInt uiMbX = 0; uiMbX < m_uiWidthInMB;  uiMbX++ )
  {
    MbDataAccess* pcMbDataAccess = 0;
    RNOK( m_pcCurrMbDataCtrl->initMb( pcMbDataAccess, uiMbY, uiMbX ) );

    MbData& rcMbData        = pcMbDataAccess->getMbData();
    UInt    uiMbIndex       = uiMbY * m_uiWidthInMB + uiMbX;
    Bool    bIntra4x4       =     rcMbData.isIntra4x4   ();
    Bool    bIntra16x16     =     rcMbData.isIntra16x16 ();
    Bool    bIsSignificant  = (   rcMbData.getMbCbp()          > 0 );
    Bool    bIsSigLuma      = ( ( rcMbData.getMbCbp() & 0x0F ) > 0 );
    Bool    b8x8Present     = (   pcMbDataAccess->getSH().getPPS().getTransform8x8ModeFlag() &&
                                  rcMbData.is8x8TrafoFlagPresent() );
    Bool    b8x8Transform   = ( b8x8Present && ( bIsSigLuma || bIntra4x4 ) && rcMbData.isTransformSize8x8() );
    UInt    uiMbCbp         = pcMbDataAccess->getAutoCbp();

    if( ! pcMbDataAccess->getMbData().isIntra() )
      pcMbDataAccess->getMbData().activateMotionRefinement();
    //===== set macroblock mode =====
    MbFGSCoefMap &rcMbFGSCoefMap = m_pcCoefMap[uiMbIndex];

    rcMbFGSCoefMap.resetNumCoded();
    rcMbFGSCoefMap.resetMbRefCtx();
    rcMbFGSCoefMap.resetMbCoefMap();

    rcMbFGSCoefMap.getMbMap() = ( bIntra16x16 || bIsSignificant                           ? SIGNIFICANT         : CLEAR )
                              + ( bIntra16x16 || bIntra4x4 || bIsSigLuma || !b8x8Present  ? TRANSFORM_SPECIFIED : CLEAR );
    //--- LUMA ---
    for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
    {
      rcMbFGSCoefMap.getB8x8Map( c8x8Idx ) = ( ( uiMbCbp & ( 1 << c8x8Idx.b8x8Index() ) ) > 0 ? SIGNIFICANT : CLEAR );
      if( b8x8Transform )
      {
        TCoeff* piCoeff = rcMbData.getMbTCoeffs().get8x8( c8x8Idx );

        if( pcMbDataAccess->getSH().getPPS().getEntropyCodingModeFlag() )
        {
          for( S4x4Idx cIdx( c8x8Idx ); cIdx.isLegal( c8x8Idx ); cIdx++ )
          {
            rcMbFGSCoefMap.getLumaScanPos( cIdx ) =  ((cIdx.x()%2) == 0 && (cIdx.y()%2) == 0) ? 64 : 0;
            rcMbFGSCoefMap.getB4x4Map( cIdx ) = rcMbFGSCoefMap.getB8x8Map( c8x8Idx );
          }
        }
        else
        {
          for( S4x4Idx cIdx( c8x8Idx ); cIdx.isLegal( c8x8Idx ); cIdx++ )
          {
            rcMbFGSCoefMap.getLumaScanPos( cIdx ) = 16;
            rcMbFGSCoefMap.getB4x4Map( cIdx ) = rcMbFGSCoefMap.getB8x8Map( c8x8Idx );
          }
        }

        CoefMap* pcCoefMap = rcMbFGSCoefMap.getCoefMap(c8x8Idx);
        //===== set transform coefficients =====
        for( UInt ui8x8ScanIndex = 0; ui8x8ScanIndex < 64; ui8x8ScanIndex++ )
        {
          if( 0 != piCoeff[g_aucFrameScan64[ui8x8ScanIndex]] )
          {
            pcCoefMap[ui8x8ScanIndex] = SIGNIFICANT;
            if( piCoeff[g_aucFrameScan64[ui8x8ScanIndex]] < 0 )
              pcCoefMap[ui8x8ScanIndex] |= BASE_SIGN;
          }

          if( pcMbDataAccess->getSH().getPPS().getEntropyCodingModeFlag() )
          {
            S4x4Idx cIdx2 = S4x4Idx( c8x8Idx ) ;
            if( !( pcCoefMap[ui8x8ScanIndex] & (SIGNIFICANT|CODED) ) && rcMbFGSCoefMap.getLumaScanPos( cIdx2 ) == 64 )
              rcMbFGSCoefMap.getLumaScanPos( cIdx2 ) = ui8x8ScanIndex;
          }
          else
          {
            S4x4Idx cIdx2 = S4x4Idx( c8x8Idx ) + (ui8x8ScanIndex & 3);
            if( !( pcCoefMap[ui8x8ScanIndex] & (SIGNIFICANT|CODED) ) && rcMbFGSCoefMap.getLumaScanPos( cIdx2 ) == 16 )
              rcMbFGSCoefMap.getLumaScanPos( cIdx2 ) = ui8x8ScanIndex >> 2;
          }
        }
      }
      else
      {
        for( S4x4Idx cIdx( c8x8Idx ); cIdx.isLegal( c8x8Idx ); cIdx++ )
        {
          TCoeff* piCoeff     = rcMbData.getMbTCoeffs().get( cIdx );
          UChar   ucBlockSig  = CLEAR;

          UChar &rucScanPos = rcMbFGSCoefMap.getLumaScanPos( cIdx );
          rucScanPos = 16;
          CoefMap* pcCoefMap = rcMbFGSCoefMap.getCoefMap( cIdx );
          //===== set transform coefficients =====
          for( UInt uiScanIndex = 0; uiScanIndex < 16; uiScanIndex++ )
          {
            if( piCoeff[g_aucFrameScan[uiScanIndex]] )
            {
              pcCoefMap[uiScanIndex] = SIGNIFICANT;
              ucBlockSig             = SIGNIFICANT;
              if (piCoeff[g_aucFrameScan[uiScanIndex]] < 0)
                pcCoefMap[uiScanIndex] |= BASE_SIGN;
            }

            if( !( pcCoefMap[uiScanIndex] & (SIGNIFICANT|CODED) ) && rucScanPos == 16 )
              rucScanPos = uiScanIndex;
          }

          //===== set block mode =====
          rcMbFGSCoefMap.getB4x4Map( cIdx ) = ucBlockSig;
        }
      }
    }


    //--- CHROMA DC ---
    for( CPlaneIdx cCPlaneIdx; cCPlaneIdx.isLegal(); ++cCPlaneIdx )
    {
      UChar ucBlockSig = CLEAR;
      UChar &rucScanPos = rcMbFGSCoefMap.getChromaDCScanPos( cCPlaneIdx );
      rucScanPos = 4;
      for( CIdx cCIdx( cCPlaneIdx ); cCIdx.isLegal( cCPlaneIdx ); cCIdx++ )
      {
        TCoeff   iCoeff    = rcMbData.getMbTCoeffs().get( cCIdx )[0];
        CoefMap& rcCoefMap = rcMbFGSCoefMap.getCoefMap( cCIdx )[0];
        if( iCoeff )
        {
          rcCoefMap  = SIGNIFICANT;
          ucBlockSig = SIGNIFICANT;

          if( iCoeff < 0 )
            rcCoefMap |= BASE_SIGN;
        }

        if( !( rcCoefMap & (SIGNIFICANT|CODED) ) && rucScanPos == 4 )
          rucScanPos = cCIdx&3;
      }
      rcMbFGSCoefMap.getChromaDCMbMap( cCPlaneIdx ) = ucBlockSig;
    }

    //--- CHROMA AC ---
    for( CIdx cCIdx; cCIdx.isLegal(); cCIdx++ )
    {
      TCoeff* piCoeff     = rcMbData.getMbTCoeffs().get( cCIdx );
      CoefMap* pcCoefMap  = rcMbFGSCoefMap.getCoefMap( cCIdx );
      UChar   ucBlockSig  = CLEAR;
      UChar &rucScanPos = rcMbFGSCoefMap.getChromaACScanPos( cCIdx );
      rucScanPos = 16;
      for( UInt ui = 1; ui < 16; ui++ )
      {
        if( piCoeff[g_aucFrameScan[ui]] )
        {
          pcCoefMap[ui] = SIGNIFICANT;
          ucBlockSig    = SIGNIFICANT;

          if (piCoeff[g_aucFrameScan[ui]] < 0)
            pcCoefMap[ui] |= BASE_SIGN;
        }

        if( !( pcCoefMap[ui] & (SIGNIFICANT|CODED) ) && rucScanPos == 16 )
          rucScanPos = ui;
      }
      rcMbFGSCoefMap.getChromaACBlockMap( cCIdx ) = ucBlockSig;
    }
  }

  return Err::m_nOK;
}


ErrVal
FGSCoder::xUpdateCodingPath()
{
  for( UInt uiMbY = 0; uiMbY < m_uiHeightInMB; uiMbY++ )
  for( UInt uiMbX = 0; uiMbX < m_uiWidthInMB;  uiMbX++ )
  {
    MbDataAccess* pcMbDataAccessBL = 0;
    MbDataAccess* pcMbDataAccessEL = 0;
    RNOK( m_pcCurrMbDataCtrl->initMb( pcMbDataAccessBL, uiMbY, uiMbX ) );
    RNOK( m_cMbDataCtrlEL    .initMb( pcMbDataAccessEL, uiMbY, uiMbX ) );

    //===== scale enhancement layer coefficients =====
    pcMbDataAccessEL->getMbData().setTransformSize8x8( pcMbDataAccessBL->getMbData().isTransformSize8x8() );
    RNOK( xScaleTCoeffs( *pcMbDataAccessEL, false ) );
    //===== update coefficients, CBP, QP =====
    RNOK( xUpdateMacroblock( *pcMbDataAccessBL, *pcMbDataAccessEL, uiMbY, uiMbX ) );
  }

  return Err::m_nOK;
}


ErrVal FGSCoder::xClearCodingPath()
{
  for( UInt uiMbY = 0; uiMbY < m_uiHeightInMB; uiMbY++ )
  {
    for( UInt uiMbX = 0; uiMbX < m_uiWidthInMB;  uiMbX++ )
    {
      MbDataAccess* pcMbDataAccess = 0;
      RNOK( m_pcCurrMbDataCtrl->initMb( pcMbDataAccess, uiMbY, uiMbX ) );

      MbData& rcMbData        = pcMbDataAccess->getMbData();
      UInt    uiMbIndex       = uiMbY * m_uiWidthInMB + uiMbX;
      Bool    bIntra4x4       =     rcMbData.isIntra4x4   ();
      Bool    bIsSigLuma      = ( ( rcMbData.getMbCbp() & 0xFF ) > 0 );
      Bool    b8x8Present     = (   pcMbDataAccess->getSH().getPPS().getTransform8x8ModeFlag() &&
        rcMbData.is8x8TrafoFlagPresent() );
      Bool    b8x8Transform   = ( b8x8Present && ( bIsSigLuma || bIntra4x4 ) && rcMbData.isTransformSize8x8() );
      UInt    uiMbCbp         = pcMbDataAccess->getAutoCbp();

      //===== set macroblock mode =====
      MbFGSCoefMap &rcMbFGSCoefMap = m_pcCoefMap[uiMbIndex];
      rcMbFGSCoefMap.resetNumCoded();
      rcMbFGSCoefMap.getMbMap() &= SIGNIFICANT | TRANSFORM_SPECIFIED;
      //--- LUMA ---
      for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
      {
        rcMbFGSCoefMap.getB8x8Map( c8x8Idx ) = ( ( uiMbCbp & ( 1 << c8x8Idx.b8x8Index() ) ) > 0 ? SIGNIFICANT : CLEAR );
        if( b8x8Transform )
        {
          if( pcMbDataAccess->getSH().getPPS().getEntropyCodingModeFlag() )
          {
          for( S4x4Idx cIdx( c8x8Idx ); cIdx.isLegal( c8x8Idx ); cIdx++ )
          {
            rcMbFGSCoefMap.getLumaScanPos( cIdx ) = ((cIdx.x()%2) == 0 && (cIdx.y()%2) == 0) ? 64:0;
            rcMbFGSCoefMap.getB4x4Map( cIdx ) &= ~CODED;
          }
          }
          else
          {
            for( S4x4Idx cIdx( c8x8Idx ); cIdx.isLegal( c8x8Idx ); cIdx++ )
            {
              rcMbFGSCoefMap.getLumaScanPos( cIdx ) = 16;
              rcMbFGSCoefMap.getB4x4Map( cIdx ) &= ~CODED;
            }
          }

          //===== set transform coefficients =====
          UInt ui8x8ScanIndex;
          CoefMap* pcCoefMap = rcMbFGSCoefMap.getCoefMap( c8x8Idx );
          for( ui8x8ScanIndex = 0; ui8x8ScanIndex < 64; ui8x8ScanIndex++ )
          {
            pcCoefMap[ui8x8ScanIndex] &= ~CODED & ~NEWSIG;
          }
          for( ui8x8ScanIndex = 0; ui8x8ScanIndex < 64; ui8x8ScanIndex++ )
          {
            if( pcMbDataAccess->getSH().getPPS().getEntropyCodingModeFlag() )
            {
              S4x4Idx cIdx2 = S4x4Idx( c8x8Idx );
              if( !( pcCoefMap[ui8x8ScanIndex] & (SIGNIFICANT|CODED) ) && rcMbFGSCoefMap.getLumaScanPos( cIdx2 ) == 64 )
                rcMbFGSCoefMap.getLumaScanPos( cIdx2 ) = ui8x8ScanIndex;
            }
            else
            {
              S4x4Idx cIdx2 = S4x4Idx( c8x8Idx ) + (ui8x8ScanIndex & 3);
              if( !( pcCoefMap[ui8x8ScanIndex] & (SIGNIFICANT|CODED) ) && rcMbFGSCoefMap.getLumaScanPos( cIdx2 ) == 16 )
                rcMbFGSCoefMap.getLumaScanPos( cIdx2 ) = ui8x8ScanIndex >> 2;
            }
          }
        }
        else
        {
          for( S4x4Idx cIdx( c8x8Idx ); cIdx.isLegal( c8x8Idx ); cIdx++ )
          {
            UInt uiScanIndex;
            CoefMap* pcCoefMap = rcMbFGSCoefMap.getCoefMap( cIdx );
            for( uiScanIndex = 0; uiScanIndex < 16; uiScanIndex++ )
              pcCoefMap[uiScanIndex] &= ~CODED & ~NEWSIG;
            UChar &rucScanPos = rcMbFGSCoefMap.getLumaScanPos( cIdx );
            rucScanPos = 16;
            for( uiScanIndex = 0; uiScanIndex < 16; uiScanIndex++ )
              if( !( pcCoefMap[uiScanIndex] & (SIGNIFICANT|CODED) ) && rucScanPos == 16 )
                rucScanPos = uiScanIndex;

            rcMbFGSCoefMap.getB4x4Map( cIdx ) &= ~CODED;
          }
        }
      }


      //--- CHROMA DC ---
      for( CPlaneIdx cCPlaneIdx; cCPlaneIdx.isLegal(); ++cCPlaneIdx )
      {
        UChar &rucScanPos = rcMbFGSCoefMap.getChromaDCScanPos( cCPlaneIdx );
        rucScanPos = 4;

        for( CIdx cCIdx( cCPlaneIdx ); cCIdx.isLegal( cCPlaneIdx ); cCIdx++ )
        {
          rcMbFGSCoefMap.getCoefMap( cCIdx )[0]  &= ~CODED & ~NEWSIG;
          if( !( rcMbFGSCoefMap.getCoefMap( cCIdx )[0] & (SIGNIFICANT|CODED) ) && rucScanPos == 4 )
            rucScanPos = cCIdx&3;
        }
        rcMbFGSCoefMap.getChromaDCMbMap( cCPlaneIdx ) &= ~CODED;
      }
      //--- CHROMA AC ---
      for( CIdx cCIdx; cCIdx.isLegal(); cCIdx++ )
      {
        UInt  ui;
        UChar &rucScanPos = rcMbFGSCoefMap.getChromaACScanPos( cCIdx );
        rucScanPos = 16;

        CoefMap *pcCoefMap = rcMbFGSCoefMap.getCoefMap( cCIdx );
        for( ui = 1; ui < 16; ui++ )
        {
          pcCoefMap[ui] &= ~CODED & ~NEWSIG;
          if( !( pcCoefMap[ui] & (SIGNIFICANT|CODED) ) && rucScanPos == 16 )
            rucScanPos = ui;
        }
        rcMbFGSCoefMap.getChromaACBlockMap( cCIdx ) &= ~CODED;
      }
    }
  }

  return Err::m_nOK;
}


ErrVal
FGSCoder::xUpdateMacroblock( MbDataAccess&  rcMbDataAccessBL,
                             MbDataAccess&  rcMbDataAccessEL,
                             UInt           uiMbY,
                             UInt           uiMbX )
{
  UInt  uiExtCbp  = 0;
  Bool  b8x8      = rcMbDataAccessBL.getMbData().isTransformSize8x8();
  UInt  uiMbIndex = uiMbY*m_uiWidthInMB + uiMbX;
  MbFGSCoefMap &rcMbFGSCoefMap = m_pcCoefMap[uiMbIndex];
  if( ! rcMbDataAccessBL.getMbData().isIntra() && ! rcMbDataAccessEL.getMbData().getBLSkipFlag() )
  {
    //----- update motion parameters -----
    rcMbDataAccessBL.getMbData().copyFrom  ( rcMbDataAccessEL.getMbData() );
    rcMbDataAccessBL.getMbData().copyMotion( rcMbDataAccessEL.getMbData() );
  }
  //===== luma =====
  if( b8x8 )
  {
    for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
    {
      Bool    bSig      = false;
      TCoeff* piCoeffBL = rcMbDataAccessBL.getMbTCoeffs().get8x8( c8x8Idx );
      TCoeff* piCoeffEL = rcMbDataAccessEL.getMbTCoeffs().get8x8( c8x8Idx );
      CoefMap *pcCoefMap = rcMbFGSCoefMap.getCoefMap( c8x8Idx );
      for( UInt ui8x8ScanIdx = 0; ui8x8ScanIdx < 64; ui8x8ScanIdx++ )
      {
        UInt  uiPos         = g_aucFrameScan64[ui8x8ScanIdx];
        if( m_bUpdateWithoutMap || ( pcCoefMap[ui8x8ScanIdx] & CODED ) )
        {
          xUpdateCoefMap(piCoeffBL[uiPos], piCoeffEL[uiPos], pcCoefMap[ui8x8ScanIdx] );
        }
        if( piCoeffBL[uiPos] )
        {
          bSig = true;
        }
      }
      if( bSig )
      {
        uiExtCbp |= ( 0x33 << c8x8Idx.b4x4() );
      }
    }
  }
  else
  {
    for( S4x4Idx c4x4Idx; c4x4Idx.isLegal(); c4x4Idx++ )
    {
      Bool    bSig       = false;
      TCoeff* piCoeffBL  = rcMbDataAccessBL.getMbTCoeffs().get( c4x4Idx );
      TCoeff* piCoeffEL  = rcMbDataAccessEL.getMbTCoeffs().get( c4x4Idx );
      CoefMap* pcCoefMap = rcMbFGSCoefMap.getCoefMap( c4x4Idx );
      for( UInt uiScanIdx = 0; uiScanIdx < 16; uiScanIdx++ )
      {
        UInt  uiPos         = g_aucFrameScan[uiScanIdx];

        if( m_bUpdateWithoutMap || pcCoefMap[uiScanIdx] & CODED )
        {
          xUpdateCoefMap(piCoeffBL[uiPos], piCoeffEL[uiPos], pcCoefMap[uiScanIdx] );
        }
        if( piCoeffBL[uiPos] )
        {
          bSig = true;
        }
      }
      if( bSig )
      {
        uiExtCbp |= ( 1 << c4x4Idx );
      }
    }
  }


  //===== chroma DC =====
  Bool  bSigDC = false;
  for( CIdx cCIdx2; cCIdx2.isLegal(); cCIdx2++ )
  {
    CoefMap &rcCoefMap = rcMbFGSCoefMap.getCoefMap( cCIdx2)[0];
    TCoeff  &riCoeffBL = rcMbDataAccessBL.getMbTCoeffs().get( cCIdx2 )[0];

    if( m_bUpdateWithoutMap || rcCoefMap & CODED )
    {
      TCoeff  &riCoeffEL = rcMbDataAccessEL.getMbTCoeffs().get( cCIdx2 )[0];
      xUpdateCoefMap( riCoeffBL, riCoeffEL, rcCoefMap );
    }

    if( riCoeffBL )
      bSigDC = true;
  }
  //===== chroma AC =====
  Bool  bSigAC = false;
  for( CIdx cCIdx; cCIdx.isLegal(); cCIdx++ )
  {
    TCoeff* piCoeffBL = rcMbDataAccessBL.getMbTCoeffs().get( cCIdx );
    TCoeff* piCoeffEL = rcMbDataAccessEL.getMbTCoeffs().get( cCIdx );
    CoefMap* piCoefMap = rcMbFGSCoefMap.getCoefMap( cCIdx );
    for( UInt uiScanIdx = 1; uiScanIdx < 16; uiScanIdx++ )
    {
      UInt  uiPos     = g_aucFrameScan[uiScanIdx];
      if( m_bUpdateWithoutMap || piCoefMap[uiScanIdx] & CODED )
        xUpdateCoefMap( piCoeffBL[uiPos], piCoeffEL[uiPos], piCoefMap[uiScanIdx] );
      if( piCoeffBL[uiPos] )
      {
        bSigAC = true;
      }
    }
  }


  //===== set CBP =====
  UInt  uiChromaCBP = ( bSigAC ? 2 : bSigDC ? 1 : 0 );
  uiExtCbp         |= ( uiChromaCBP << 16 );
  rcMbDataAccessBL.getMbData().setAndConvertMbExtCbp( uiExtCbp );


  //===== set QP =====
  Int iELQP     = rcMbDataAccessEL.getMbData().getQp();
  Int iNumCoded = rcMbFGSCoefMap.getNumCoded();
  Int iQPDelta  = ( 384 - iNumCoded ) / 64;

  Int iQP       = min( 51, iELQP + iQPDelta );
  if( ! ( rcMbFGSCoefMap.getMbMap() & SIGNIFICANT ) )
  {
    iQP = rcMbDataAccessEL.getSH().getPicQp();
  }
  rcMbDataAccessBL.getMbData().setQp( iQP );

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
FGSCoder::xScaleTCoeffs( MbDataAccess& rcMbDataAccess,
                         Bool          bBaseLayer )
{
  const Int aaiDequantDcCoef[6] = {  10, 11, 13, 14, 16, 18 };
  Quantizer cQuantizer;
  cQuantizer.setQp( rcMbDataAccess, false );

  const QpParameter&  cLQp      = cQuantizer.getLumaQp  ();
  const QpParameter&  cCQp      = cQuantizer.getChromaQp();
  Bool                bIntra    = rcMbDataAccess.getMbData().isIntra();
  Bool                b8x8      = rcMbDataAccess.getMbData().isTransformSize8x8();
  Bool                b16x16    = rcMbDataAccess.getMbData().isIntra16x16();
  UInt                uiYScalId = ( bIntra ? ( b8x8 && !b16x16 ? 6 : 0 ) : ( b8x8 ? 7 : 3 ) );
  UInt                uiUScalId = ( bIntra ? 1 : 4 );
  UInt                uiVScalId = ( bIntra ? 2 : 5 );
  const UChar*        pucScaleY = rcMbDataAccess.getSH().getScalingMatrix( uiYScalId );
  const UChar*        pucScaleU = rcMbDataAccess.getSH().getScalingMatrix( uiUScalId );
  const UChar*        pucScaleV = rcMbDataAccess.getSH().getScalingMatrix( uiVScalId );
  Int                 iScale    = 1;

  //===== luma =====
  if( b16x16 )
  {
    //===== INTRA_16x16 =====
    for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
    {
      RNOK( xScale4x4Block( rcMbDataAccess.getMbTCoeffs().get   ( cIdx ), pucScaleY, 1, cLQp ) );
    }

    iScale  = aaiDequantDcCoef[cLQp.rem()] << cLQp.per();
    if( pucScaleY )
    {
      iScale  *= pucScaleY[0];
      iScale >>= 4;
    }
    // perform scaling only
    TCoeff* piCoeff = rcMbDataAccess.getMbTCoeffs().get( B4x4Idx(0) );

    for( Int uiDCIdx = 0; uiDCIdx < 16; uiDCIdx++ )
    {
      TCoeff a = piCoeff[16*uiDCIdx] * iScale;
      Int    b = piCoeff[16*uiDCIdx] * iScale;
      if( a != b )
      {
        printf("Short overflow in FGS Intra16x16 DC-coeffs.\n");
        // not good
        piCoeff[16*uiDCIdx] = max( (Int)MSYS_SHORT_MIN, min( (Int)MSYS_SHORT_MAX, b ) );
      }
      else
      {
        piCoeff[16*uiDCIdx] *= iScale;
      }
    }
    //===== correct CBP =====
    rcMbDataAccess.getMbData().setMbCbp( rcMbDataAccess.getAutoCbp() );
  }
  else if( b8x8 )
  {
    //===== 8x8 BLOCKS =====
    for( B8x8Idx cIdx; cIdx.isLegal(); cIdx++ )
    {
      RNOK( xScale8x8Block( rcMbDataAccess.getMbTCoeffs().get8x8( cIdx ), pucScaleY, cLQp ) );
    }
  }
  else
  {
    //===== 4x4 BLOCKS =====
    for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
    {
      RNOK( xScale4x4Block( rcMbDataAccess.getMbTCoeffs().get   ( cIdx ), pucScaleY, 0, cLQp ) );
    }
  }

  //===== chroma =====
  for( CIdx cIdx; cIdx.isLegal(); cIdx++ )
  {
    RNOK( xScale4x4Block( rcMbDataAccess.getMbTCoeffs().get( cIdx ), ( cIdx.plane() ? pucScaleV : pucScaleU ), 1, cCQp ) );
  }

  // only performs scaling, not inverse transform
  UInt  uiMbIndex = rcMbDataAccess.getMbY()*m_uiWidthInMB+rcMbDataAccess.getMbX();

  iScale = g_aaiDequantCoef[cCQp.rem()][0] << cCQp.per();
  /* HS: old scaling modified:
  (I did not work for scaling matrices, when QpPer became less than 5 in an FGS enhancement) */

  for( CIdx cCIdx; cCIdx.isLegal(); cCIdx++ )
  {
    if(! bBaseLayer && ! ( m_pcCoefMap[uiMbIndex].getCoefMap( cCIdx )[0] & CODED ) && !m_bUpdateWithoutMap )
      // condition "! bBaseLayer" is needed. When "xScaleTCoeffs is called
      // before first FGS layer, m_aapaucChromaDCCoefMap is not initialized
      rcMbDataAccess.getMbTCoeffs().get( cCIdx )[0] = 0;
    else
      rcMbDataAccess.getMbTCoeffs().get( cCIdx )[0] *= iScale;
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
  MbTransformCoeffs&  rcCoeffs  = rcMbDataAccess.getMbTCoeffs();

  rcMbBuffer.loadBuffer( m_pcBaseLayerSbb->getFullPelYuvBuffer() );

  TCoeff lumaDcCoeffs[16];
  if ( rcMbDataAccess.getMbData().isIntra16x16() )
  {
    // backup luma DC
    TCoeff *piCoeffs;
    UInt   uiDCIdx;

    piCoeffs = rcCoeffs.get( B4x4Idx(0) );
    for( uiDCIdx = 0; uiDCIdx < 16; uiDCIdx++ )
      lumaDcCoeffs[uiDCIdx] = piCoeffs[16*uiDCIdx] ;

    // inverse transform on luma DC
    RNOK( m_pcTransform->invTransformDcCoeff( piCoeffs, 1 ) );

    // inverse transform on entire MB
    for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
    {
      RNOK( m_pcTransform->invTransform4x4Blk( rcMbBuffer.getYBlk( cIdx ), iLStride, rcCoeffs.get( cIdx ) ) );
    }

    // restore luma DC
    for( uiDCIdx = 0; uiDCIdx < 16; uiDCIdx++ )
      piCoeffs[16*uiDCIdx] = lumaDcCoeffs[uiDCIdx];
  }
  else if( b8x8 )
  {
    for( B8x8Idx cIdx; cIdx.isLegal(); cIdx++ )
    {
      RNOK( m_pcTransform->invTransform8x8Blk( rcMbBuffer.getYBlk( cIdx ), iLStride, rcCoeffs.get8x8( cIdx ) ) );
    }
  }
  else
  {
    for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
    {
      RNOK( m_pcTransform->invTransform4x4Blk( rcMbBuffer.getYBlk( cIdx ), iLStride, rcCoeffs.get( cIdx ) ) );
    }
  }

  TCoeff              chromaDcCoeffs[2][4];
  UInt                uiPlane;
  Int                 iScale;
  Bool                bIntra    = rcMbDataAccess.getMbData().isIntra();
  UInt                uiUScalId = ( bIntra ? 1 : 4 );
  UInt                uiVScalId = ( bIntra ? 2 : 5 );
  const UChar*        pucScaleU = rcMbDataAccess.getSH().getScalingMatrix( uiUScalId );
  const UChar*        pucScaleV = rcMbDataAccess.getSH().getScalingMatrix( uiVScalId );

  // backup chroma DC coefficients
  for( uiPlane = 0; uiPlane < 2; uiPlane++ )
  {
    TCoeff*   piCoeff = rcCoeffs.get( CIdx(4*uiPlane) );
    for( UInt uiDCIdx = 0; uiDCIdx < 4; uiDCIdx++ )
      chromaDcCoeffs[uiPlane][uiDCIdx] = piCoeff[16*uiDCIdx];
  }

  // scaling has already been performed on DC coefficients
  iScale = ( pucScaleU ? pucScaleU[0] : 16 );
  m_pcTransform->invTransformChromaDc( rcCoeffs.get( CIdx(0) ), iScale );
  iScale = ( pucScaleV ? pucScaleV[0] : 16 );
  m_pcTransform->invTransformChromaDc( rcCoeffs.get( CIdx(4) ), iScale );

  RNOK( m_pcTransform->invTransformChromaBlocks( rcMbBuffer.getMbCbAddr(), iCStride, rcCoeffs.get( CIdx(0) ) ) );
  RNOK( m_pcTransform->invTransformChromaBlocks( rcMbBuffer.getMbCrAddr(), iCStride, rcCoeffs.get( CIdx(4) ) ) );

  // restore chroma DC coefficients
  for( uiPlane = 0; uiPlane < 2; uiPlane++ )
  {
    TCoeff*   piCoeff = rcCoeffs.get( CIdx(4*uiPlane) );
    for( UInt uiDCIdx = 0; uiDCIdx < 4; uiDCIdx++ )
      piCoeff[16*uiDCIdx] = chromaDcCoeffs[uiPlane][uiDCIdx];
  }
  m_pcTransform->setClipMode( true );

  return Err::m_nOK;
}


Int
FGSCoder::xScaleLevel4x4( Int                 iLevel,
                          Int                 iIndex,
                          const QpParameter&  cQP,
                          const QpParameter&  cBaseQP )
{
  Int iSign       = ( iLevel < 0 ? -1 : 1 );
  Int iBaseScale  = g_aaiDequantCoef[cBaseQP.rem()][iIndex] << cBaseQP.per();
  Int iScale      = g_aaiDequantCoef[cQP    .rem()][iIndex] << cQP    .per();

  return iSign * ( ( abs(iLevel) * iBaseScale ) + ( iScale >> 1 ) ) / iScale;
}

Int
FGSCoder::xScaleLevel8x8( Int                 iLevel,
                          Int                 iIndex,
                          const QpParameter&  cQP,
                          const QpParameter&  cBaseQP )
{
  Int iSign       = ( iLevel < 0 ? -1 : 1 );
  Int iBaseScale  = g_aaiDequantCoef64[cBaseQP.rem()][iIndex] << cBaseQP.per();
  Int iScale      = g_aaiDequantCoef64[cQP    .rem()][iIndex] << cQP    .per();

  return iSign * ( ( abs(iLevel) * iBaseScale ) + ( iScale >> 1 ) ) / iScale;
}


ErrVal
FGSCoder::xScaleSymbols4x4( TCoeff*             piCoeff,
                            const QpParameter&  cQP,
                            const QpParameter&  cBaseQP )
{
  for( Int iIndex = 0; iIndex < 16; iIndex++ )
  {
    if( piCoeff[iIndex] )
    {
      piCoeff[iIndex] = xScaleLevel4x4( piCoeff[iIndex], iIndex, cQP, cBaseQP );
    }
  }
  return Err::m_nOK;
}


ErrVal
FGSCoder::xScaleSymbols8x8( TCoeff*             piCoeff,
                            const QpParameter&  cQP,
                            const QpParameter&  cBaseQP )
{
  for( Int iIndex = 0; iIndex < 64; iIndex++ )
  {
    if( piCoeff[iIndex] )
    {
      piCoeff[iIndex] = xScaleLevel8x8( piCoeff[iIndex], iIndex, cQP, cBaseQP );
    }
  }
  return Err::m_nOK;
}


ErrVal
FGSCoder::xUpdateSymbols( TCoeff* piCoeff,
                          TCoeff* piCoeffEL,
                          Bool&   bSigDC,
                          Bool&   bSigAC,
                          Int     iNumCoeff )
{
  piCoeff     [0] += piCoeffEL[0];
  if( piCoeff [0] )
  {
    bSigDC = true;
  }

  for( Int iIndex = 1; iIndex < iNumCoeff; iIndex++ )
  {
    piCoeff    [iIndex] += piCoeffEL[iIndex];
    if( piCoeff[iIndex] )
    {
      bSigAC = true;
    }
  }

  return Err::m_nOK;
}


// get 4x4 significance map for luma
Void FGSCoder::getCoeffSigMap( UInt uiMbX, UInt uiMbY, S4x4Idx cIdx, UChar *pucSigMap )
{
  CoefMap *pcCoefMap = m_pcCoefMap[uiMbY * m_uiWidthInMB + uiMbX ].getCoefMap( cIdx );
  for( UInt uiScanIndex = 0; uiScanIndex < 16; uiScanIndex ++ )
  {
    pucSigMap[g_aucFrameScan[uiScanIndex]] = SIGNIFICANT & pcCoefMap[uiScanIndex];
  }
}


// get 8x8 significance map for luma
Void FGSCoder::getCoeffSigMap( UInt uiMbX, UInt uiMbY, B8x8Idx c8x8Idx, UChar *pucSigMap )
{
  CoefMap *pcCoefMap = m_pcCoefMap[uiMbY * m_uiWidthInMB + uiMbX].getCoefMap( c8x8Idx );
  for( UInt ui8x8ScanIndex = 0; ui8x8ScanIndex < 64; ui8x8ScanIndex ++ )
  {
    pucSigMap[g_aucFrameScan64[ui8x8ScanIndex]] = SIGNIFICANT & pcCoefMap[ui8x8ScanIndex];
  }
}


// get 4x4 significance map for chroma
Void FGSCoder::getCoeffSigMap( UInt uiMbX, UInt uiMbY, CIdx cIdx, UChar *pucSigMap )
{
  UInt uiMbIndex    = uiMbY * m_uiWidthInMB + uiMbX;
  CoefMap* pcCoefMap = m_pcCoefMap[uiMbIndex].getCoefMap( cIdx );
  for( UInt uiScanIndex = 0; uiScanIndex < 16; uiScanIndex ++ )
    pucSigMap[g_aucFrameScan[uiScanIndex]] = SIGNIFICANT & pcCoefMap[uiScanIndex];
}


// get entire 8x8 significance map for chroma
Void FGSCoder::getCoeffSigMapChroma8x8( UInt uiMbX, UInt uiMbY, UInt uiPlane, UChar *pucSigMap )
{
  UInt uiMbIndex    = uiMbY * m_uiWidthInMB + uiMbX;

  for( CIdx cCIdx(( CPlaneIdx(uiPlane) )); cCIdx.isLegal( CPlaneIdx(uiPlane) ); cCIdx++ )
  {
    UInt uiBlockIdxWithMb = cCIdx.y() * 2 + cCIdx.x();
    CoefMap *pcCoefMap = m_pcCoefMap[uiMbIndex].getCoefMap( cCIdx );
    for( UInt uiScanIndex = 0; uiScanIndex < 16; uiScanIndex ++ )
      pucSigMap[uiBlockIdxWithMb * 16 + g_aucFrameScan[uiScanIndex]] = SIGNIFICANT & pcCoefMap[uiScanIndex];
  }
}


ErrVal
FGSCoder::xClearBaseCoeffs( MbDataAccess& rcMbDataAccess,
                            MbDataAccess* pcMbDataAccessBase )
{
  UInt uiMbY     = pcMbDataAccessBase->getMbY();
  UInt uiMbX     = pcMbDataAccessBase->getMbX();
  UInt uiMbIndex = uiMbY * m_uiWidthInMB + uiMbX;
  MbFGSCoefMap &rcMbFGSCoefMap = m_pcCoefMap[uiMbIndex];
  rcMbFGSCoefMap.resetNumCoded();
  rcMbFGSCoefMap.resetMbRefCtx();
  rcMbFGSCoefMap.resetMbCoefMap();
  rcMbFGSCoefMap.getMbMap() = rcMbDataAccess.getSH().getPPS().getTransform8x8ModeFlag() && rcMbDataAccess.getMbData().is8x8TrafoFlagPresent() ? CLEAR : TRANSFORM_SPECIFIED;
  //--- LUMA ---
  for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
  {
    rcMbFGSCoefMap.getB8x8Map( c8x8Idx ) = CLEAR;
    for( S4x4Idx cIdx( c8x8Idx ); cIdx.isLegal( c8x8Idx ); cIdx++ )
    {
      UInt    uiScanIndex;
      CoefMap *pcCoefMap = rcMbFGSCoefMap.getCoefMap( cIdx );
      UChar &rucScanPos = rcMbFGSCoefMap.getLumaScanPos( cIdx );
      rucScanPos = 16;
      for( uiScanIndex = 0; uiScanIndex < 16; uiScanIndex++ )
        if( !( pcCoefMap[uiScanIndex] & (SIGNIFICANT|CODED) ) && rucScanPos == 16 )
          rucScanPos = uiScanIndex;

      //===== set block mode =====
      rcMbFGSCoefMap.getB4x4Map( cIdx ) = CLEAR;
    }
  }

  //--- CHROMA DC ---
  for( CPlaneIdx cCPlaneIdx; cCPlaneIdx.isLegal(); ++cCPlaneIdx )
  {
    UChar &rucScanPos = rcMbFGSCoefMap.getChromaDCScanPos( cCPlaneIdx );
    rucScanPos = 4;
    for( CIdx cCIdx( cCPlaneIdx ); cCIdx.isLegal( cCPlaneIdx ); cCIdx++ )
    {
      rcMbFGSCoefMap.getCoefMap( cCIdx )[0] = CLEAR;
      if( !( rcMbFGSCoefMap.getCoefMap( cCIdx )[0] & (SIGNIFICANT|CODED) ) && rucScanPos == 4 )
        rucScanPos = cCIdx&3;
    }
    rcMbFGSCoefMap.getChromaDCMbMap( cCPlaneIdx ) = CLEAR;
  }
  //--- CHROMA AC ---
  for( CIdx cCIdx; cCIdx.isLegal(); cCIdx++ )
  {
    UInt    ui;
    CoefMap *pcCoefMap = rcMbFGSCoefMap.getCoefMap( cCIdx );
    UChar &rucScanPos = rcMbFGSCoefMap.getChromaACScanPos( cCIdx );
    rucScanPos = 16;
    for( ui = 1; ui < 16; ui++ )
    {
      pcCoefMap[ui]  = CLEAR;
      if( !( pcCoefMap[ui] & (SIGNIFICANT|CODED) ) && rucScanPos == 16 )
        rucScanPos = ui;
    }
    rcMbFGSCoefMap.getChromaACBlockMap( cCIdx ) = CLEAR;
  }

  //pcMbDataAccessBase->getMbData().setMbCbp( 0 );
  pcMbDataAccessBase->getMbTCoeffs().clear();
  pcMbDataAccessBase->getMbData().setTransformSize8x8( false );

  IntYuvMbBuffer cZeroBuffer;
  cZeroBuffer.setAllSamplesToZero();
  RNOK( m_pcBaseLayerSbb->getFullPelYuvBuffer()->loadBuffer( &cZeroBuffer ) );

  return Err::m_nOK;
}

ErrVal
FGSCoder::xUpdateMbMaps ( MbDataAccess*      pcMbDataAccessBL,
                          MbDataAccess*      pcMbDataAccessEL,
                          MbFGSCoefMap       &rcMbFGSCoefMap,
                          Int*               piRemainingTCoeff )
{
  for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx ++ ) {
    Bool bSigCBP = ( pcMbDataAccessEL->getMbData().getMbCbp() >> c8x8Idx.b8x8Index() ) & 1;

    rcMbFGSCoefMap.getB8x8Map( c8x8Idx ) |= CODED; 
    if( bSigCBP )
      rcMbFGSCoefMap.getB8x8Map( c8x8Idx ) |= SIGNIFICANT; 

    if( !bSigCBP ) {
      //===== set coefficient and block map =====
      for( S4x4Idx cIdx(c8x8Idx); cIdx.isLegal(c8x8Idx); cIdx++ ) {
        CoefMap *pcCoefMap = rcMbFGSCoefMap.getCoefMap(cIdx);

        rcMbFGSCoefMap.getB4x4Map( cIdx ) |= CODED;
        rcMbFGSCoefMap.getLumaScanPos( cIdx ) = 
          (pcMbDataAccessEL->getMbData().isTransformSize8x8() && pcMbDataAccessEL->getSH().getPPS().getEntropyCodingModeFlag()) ? 64:16 ;

        for( UInt ui = 0; ui < 16; ui++ ) {
          if( ! ( pcCoefMap[ui] & SIGNIFICANT ) ) {
            pcCoefMap[ui] |= CODED;
            if( piRemainingTCoeff )
              *piRemainingTCoeff -= 1;
            rcMbFGSCoefMap.increaseAndCheckNumCoded( 1 ); 
          }
        }
      }
    }
    else {
      if( pcMbDataAccessBL->getMbData().isTransformSize8x8() && pcMbDataAccessBL->getSH().getPPS().getEntropyCodingModeFlag() )
      {
      }
      else if( pcMbDataAccessBL->getMbData().isTransformSize8x8() )
      {
        CoefMap *pcCoefMap = rcMbFGSCoefMap.getCoefMap(c8x8Idx);
        for( S4x4Idx cIdx(c8x8Idx); cIdx.isLegal(c8x8Idx); cIdx++ ) {
          UInt uiOffset = (cIdx.x() % 2) + (cIdx.y() %2) * 2;
          rcMbFGSCoefMap.getB4x4Map( cIdx ) |= CODED;

          // BCBP for the enhancement layer is set in pcMbDataAccessEL
          if(  pcMbDataAccessEL->getMbData().getBCBP( cIdx.b4x4() ) )
            rcMbFGSCoefMap.getB4x4Map( cIdx ) |= SIGNIFICANT;
          else {
            rcMbFGSCoefMap.getLumaScanPos( cIdx ) = 16;
            for( UInt ui = 0; ui < 16; ui++ ) {
              if( ! ( pcCoefMap[ui*4 + uiOffset] & SIGNIFICANT ) ) {
                pcCoefMap[ui*4 + uiOffset] |= CODED;
                if( piRemainingTCoeff )
                  *piRemainingTCoeff -= 1;
                rcMbFGSCoefMap.increaseAndCheckNumCoded( 1 ); 
              }
            }
          }
        }
      }
      else
      {
        for( S4x4Idx cIdx(c8x8Idx); cIdx.isLegal(c8x8Idx); cIdx++ ) {
          CoefMap *pcCoefMap = rcMbFGSCoefMap.getCoefMap(cIdx);

          rcMbFGSCoefMap.getB4x4Map( cIdx ) |= CODED;

          // BCBP for the enhancement layer is set in pcMbDataAccessEL
          if(  pcMbDataAccessEL->getMbData().getBCBP( cIdx.b4x4() ) )
            rcMbFGSCoefMap.getB4x4Map( cIdx ) |= SIGNIFICANT;
          else {
            rcMbFGSCoefMap.getLumaScanPos( cIdx ) = 16;
            for( UInt ui = 0; ui < 16; ui++ ) {
              if( ! ( pcCoefMap[ui] & SIGNIFICANT ) ) {
                pcCoefMap[ui] |= CODED;
                if( piRemainingTCoeff )
                  *piRemainingTCoeff -= 1;
                rcMbFGSCoefMap.increaseAndCheckNumCoded( 1 ); 
              }
            }
          }
        }
      }
    }
  }

  if( ( pcMbDataAccessEL->getMbData().getMbCbp() >> 4 ) <= 1 ) {
    //----- chroma AC -----
    for( CIdx cCIdx; cCIdx.isLegal(); cCIdx++ ) {
      CoefMap *pcCoefMap = rcMbFGSCoefMap.getCoefMap( cCIdx );

      rcMbFGSCoefMap.getChromaACBlockMap( cCIdx ) |= CODED; 
      rcMbFGSCoefMap.getChromaACScanPos( cCIdx ) = 16; 

      for( UInt ui = 1; ui < 16; ui++ ) {
        if( ! ( pcCoefMap[ui] & SIGNIFICANT ) ) {
          pcCoefMap[ui] |= CODED;
          if( piRemainingTCoeff )
            *piRemainingTCoeff -= 1;
          rcMbFGSCoefMap.increaseAndCheckNumCoded( 1 ); 
        }
      }
    }
  }

  if( ( pcMbDataAccessEL->getMbData().getMbCbp() >> 4 ) == 0 ) {
    //----- chroma DC -----
    for( CPlaneIdx cCPlaneIdx; cCPlaneIdx.isLegal(); ++cCPlaneIdx )
    {
      rcMbFGSCoefMap.getChromaDCMbMap( cCPlaneIdx ) |= CODED; 
      rcMbFGSCoefMap.getChromaDCScanPos( cCPlaneIdx ) = 4;

      for( UInt ui = 0; ui < 4; ui++ ) {
        CoefMap cCoefMap = rcMbFGSCoefMap.getCoefMap( CIdx( cCPlaneIdx ) + ui )[0];
        if( ! ( cCoefMap & SIGNIFICANT ) ) {
          cCoefMap |= CODED;
          if( piRemainingTCoeff )
            *piRemainingTCoeff -= 1;
          rcMbFGSCoefMap.increaseAndCheckNumCoded( 1 ); 
        }
      }
    }
  }

  return Err::m_nOK;
}


UInt 
FGSCoder::xDeriveComponentPosVectors( UInt*  puiRefPosVect,
                                      Int*   piMaxPosLuma,
                                      Int*   piMaxPosChromaAC,
                                      Int*   piMaxPosChromaDC,
                                      UInt   uiChromaStartCycle )
{
  Bool bAllowChromaDC, bAllowChromaAC;
  UInt uiCycle, uiNumFrags, uiFirstCycle, uiLastCycle;

  uiNumFrags = uiFirstCycle = 0;
  piMaxPosChromaAC[0] = 0;
  piMaxPosChromaDC[0] = -1;
  do {
    piMaxPosLuma[uiNumFrags] = puiRefPosVect[uiNumFrags];
    uiLastCycle = piMaxPosLuma[uiNumFrags];

    if( uiNumFrags > 0 ) {
      piMaxPosChromaAC[uiNumFrags] = piMaxPosChromaAC[uiNumFrags - 1];
      piMaxPosChromaDC[uiNumFrags] = piMaxPosChromaDC[uiNumFrags - 1];
    }

    // find the max position for chroma AC
    for( uiCycle = uiFirstCycle; uiCycle <= uiLastCycle; uiCycle ++ ) {
      bAllowChromaDC = (uiCycle == 0) || ((uiCycle >= uiChromaStartCycle) && ((uiCycle-uiChromaStartCycle) % 2 == 0));
      bAllowChromaAC = (uiCycle > 0) && ((uiCycle == uiChromaStartCycle) || ((uiCycle >= uiChromaStartCycle) && ((uiCycle-uiChromaStartCycle) % 3 == 1)));

      piMaxPosChromaDC[uiNumFrags] += bAllowChromaDC;
      if( piMaxPosChromaDC[uiNumFrags] > 3 )
        piMaxPosChromaDC[uiNumFrags] = 3;
      piMaxPosChromaAC[uiNumFrags] += bAllowChromaAC;
      if( piMaxPosChromaAC[uiNumFrags] > 15 )
        piMaxPosChromaAC[uiNumFrags] = 15;
    }

    uiFirstCycle = uiLastCycle + 1;
    uiNumFrags ++;
  } while( piMaxPosLuma[uiNumFrags - 1] < 15 );

  // dirty fix
  piMaxPosChromaDC[uiNumFrags - 1] = 3;
  piMaxPosChromaAC[uiNumFrags - 1] = 15;

  return uiNumFrags;
}


ErrVal
FGSCoder::xSetNumCoefficients( UInt               uiMbX, 
                               UInt               uiMbY,
                               MbFGSCoefMap       &rcMbFGSCoefMap,
                               UInt               uiMbCoeffsDecoded )
{
  // now partially it is set in xDecodeMbHeader, should be changed also
  rcMbFGSCoefMap.resetNumCoded();
  rcMbFGSCoefMap.increaseAndCheckNumCoded( uiMbCoeffsDecoded ); 
  if( uiMbCoeffsDecoded > 384 )
    return Err::m_nERR;
  else
    return Err::m_nOK;
}


H264AVC_NAMESPACE_END


