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
#include "H264AVCCommonLib/MbData.h"



H264AVC_NAMESPACE_BEGIN


ErrVal
MbData::saveAll( FILE* pFile )
{
  RNOK( MbDataStruct::          save( pFile ) );
  RNOK( m_pcMbTCoeffs         ->save( pFile ) );
  RNOK( m_apcMbMotionData [0] ->save( pFile ) );
  RNOK( m_apcMbMvdData    [0] ->save( pFile ) );
  RNOK( m_apcMbMotionData [1] ->save( pFile ) );
  RNOK( m_apcMbMvdData    [1] ->save( pFile ) );

  return Err::m_nOK;
}


ErrVal
MbData::loadAll( FILE* pFile )
{
  RNOK( MbDataStruct::          load( pFile ) );
  RNOK( m_pcMbTCoeffs         ->load( pFile ) );
  RNOK( m_apcMbMotionData [0] ->load( pFile ) );
  RNOK( m_apcMbMvdData    [0] ->load( pFile ) );
  RNOK( m_apcMbMotionData [1] ->load( pFile ) );
  RNOK( m_apcMbMvdData    [1] ->load( pFile ) );

  return Err::m_nOK;
}

ErrVal
MbData::copyMotion( MbData& rcMbData,
                    UInt    uiSliceId )
{
  m_uiSliceId   = ( uiSliceId != MSYS_UINT_MAX ? uiSliceId : m_uiSliceId );
  m_bSkipFlag   = rcMbData.m_bSkipFlag;
  m_bBLSkipFlag = rcMbData.m_bBLSkipFlag;
  m_bBLQRefFlag = rcMbData.m_bBLQRefFlag;
  m_eMbMode     = rcMbData.m_eMbMode;
  m_aBlkMode[0] = rcMbData.m_aBlkMode[0];
  m_aBlkMode[1] = rcMbData.m_aBlkMode[1];
  m_aBlkMode[2] = rcMbData.m_aBlkMode[2];
  m_aBlkMode[3] = rcMbData.m_aBlkMode[3]; 
  m_usFwdBwd    = rcMbData.m_usFwdBwd;

  m_apcMbMotionData[0]->copyFrom( *rcMbData.m_apcMbMotionData[0] );
  m_apcMbMotionData[1]->copyFrom( *rcMbData.m_apcMbMotionData[1] );

  m_apcMbMvdData[0]->copyFrom( *rcMbData.m_apcMbMvdData[0] );
  m_apcMbMvdData[1]->copyFrom( *rcMbData.m_apcMbMvdData[1] );

  return Err::m_nOK;
}


ErrVal
MbData::copyMotionBL( MbData& rcMbData,
                      Bool    bDirect8x8,
                      UInt    uiSliceId )
{
  m_uiSliceId   = ( uiSliceId != MSYS_UINT_MAX ? uiSliceId : m_uiSliceId );
  m_bBLSkipFlag = false;
  m_bBLQRefFlag = false;
  m_eMbMode     = ( rcMbData.m_eMbMode     == MODE_SKIP ? MODE_8x8   : rcMbData.m_eMbMode     );
  m_aBlkMode[0] = ( rcMbData.m_aBlkMode[0] == BLK_SKIP || rcMbData.m_eMbMode == MODE_SKIP ? ( bDirect8x8 ? BLK_8x8 : BLK_4x4 ) : rcMbData.m_aBlkMode[0] );
  m_aBlkMode[1] = ( rcMbData.m_aBlkMode[1] == BLK_SKIP || rcMbData.m_eMbMode == MODE_SKIP ? ( bDirect8x8 ? BLK_8x8 : BLK_4x4 ) : rcMbData.m_aBlkMode[1] );
  m_aBlkMode[2] = ( rcMbData.m_aBlkMode[2] == BLK_SKIP || rcMbData.m_eMbMode == MODE_SKIP ? ( bDirect8x8 ? BLK_8x8 : BLK_4x4 ) : rcMbData.m_aBlkMode[2] );
  m_aBlkMode[3] = ( rcMbData.m_aBlkMode[3] == BLK_SKIP || rcMbData.m_eMbMode == MODE_SKIP ? ( bDirect8x8 ? BLK_8x8 : BLK_4x4 ) : rcMbData.m_aBlkMode[3] );

  if( rcMbData.m_eMbMode == MODE_SKIP || rcMbData.m_eMbMode == MODE_8x8)
  {
    UInt uiFwdBwd = 0;
    for( Int n = 3; n >= 0; n--)
    {
      uiFwdBwd <<= 4;
      uiFwdBwd += (0 < rcMbData.m_apcMbMotionData[0]->getRefIdx( Par8x8(n) )) ? 1:0;
      uiFwdBwd += (0 < rcMbData.m_apcMbMotionData[1]->getRefIdx( Par8x8(n) )) ? 2:0;
    }
    m_usFwdBwd = uiFwdBwd;
  }
  else
  {
    m_usFwdBwd    = rcMbData.m_usFwdBwd;
  }

  m_apcMbMotionData[0]->copyFrom( *rcMbData.m_apcMbMotionData[0] );
  m_apcMbMotionData[1]->copyFrom( *rcMbData.m_apcMbMotionData[1] );

  m_apcMbMvdData[0]->copyFrom( *rcMbData.m_apcMbMvdData[0] );
  m_apcMbMvdData[1]->copyFrom( *rcMbData.m_apcMbMvdData[1] );

  setResidualAvailFlagsBase( rcMbData.getResidualAvailFlags() );

  return Err::m_nOK;
}


ErrVal
MbData::upsampleMotion( MbData& rcMbData, Par8x8 ePar8x8, Bool bDirect8x8 )
{
  RNOK( MbDataStruct::upsampleMotion( rcMbData, ePar8x8, bDirect8x8 ) );
  m_bBLSkipFlag = false;
  m_bBLQRefFlag = false;
  
  RNOK( m_apcMbMotionData[0]->upsampleMotion( *rcMbData.m_apcMbMotionData[0], ePar8x8 ) );
  RNOK( m_apcMbMotionData[1]->upsampleMotion( *rcMbData.m_apcMbMotionData[1], ePar8x8 ) );

  UInt uiFwdBwd = 0;
  uiFwdBwd += (0 < m_apcMbMotionData[0]->getRefIdx( B_8x8_0 )) ? 1:0;
  uiFwdBwd += (0 < m_apcMbMotionData[1]->getRefIdx( B_8x8_0 )) ? 2:0;
  m_usFwdBwd = (uiFwdBwd<<12)|(uiFwdBwd<<8)|(uiFwdBwd<<4)|(uiFwdBwd);

  UShort usResidualAvailFlagsBase;

  usResidualAvailFlagsBase  = rcMbData.isLumaResidualAvailable(ePar8x8) ? 15 : 0;
  usResidualAvailFlagsBase |= rcMbData.isChromaResidualAvailable() ? (1 << 4) : 0;
  
  setResidualAvailFlagsBase( usResidualAvailFlagsBase );

  return Err::m_nOK;
}

// TMM_ESS_UNIFIED {
ErrVal
MbData::noUpsampleMotion()
{
    clear();
    setResidualAvailFlagsBase( 0 );
    m_apcMbMotionData[0]->clear(BLOCK_NOT_AVAILABLE) ;
    m_apcMbMotionData[1]->clear(BLOCK_NOT_AVAILABLE) ;
    return Err::m_nOK;      
}

#define sign(x) (x>0?1:-1)
#define CALC_COND(a) ( sign(a) * (( (abs(a)+2) >> 2 )) << 2 )
#define MINBLOCKSIZE(c1,c2)((c1<=0)? c2:((c2<=0)?c1: min(c1,c2)))

const UChar MbData::m_aucPredictor[2][4]= {{1,0,3,2},{2,3,0,1}};
const Char MbData::m_acSuffixMbMode[2][7]={{8,16,16,8,8,0,-1},{8,16,8,16,8,0,-1}};
const Char MbData::m_acSuffixBlkMode[2][4]={{8,8,4,4},{8,4,8,4}};
const MbMode MbData::m_aeBuildMbMode[2][2]={{MODE_8x8,MODE_8x16},{MODE_16x8,MODE_16x16}};
const BlkMode MbData::m_aeBuildBlkMode[2][2]={{BLK_4x4,BLK_4x8},{BLK_8x4,BLK_8x8}};
const Char MbData::m_acComputeMbSize[2][5]={{ 8, 4,16, 4, 8},{16,16,16,16,16}};

const MbData::InfoBaseDim MbData::m_aMapInfoBaseDim[21] = 
{
    {{B4x4_Border,B8x8_Border},{{0,1},{1,2}}}, // -12  -4 -> 0 4  0
    {{NO_Border,NO_Border},{{0,0},{0,0}}},    //None             1
    {{NO_Border,NO_Border},{{0,0},{0,0}}},    //None             2 
    {{NO_Border,B8x8_Border},{{0,0},{1,2}}},   //  -8   4 -> 1 4  3
    {{NO_Border,NO_Border},{{0,0},{1,1}}},    //  -8   8 -> 1 5  4 
    {{MB_Border,B4x4_Border},{{3,4},{4,5}}},  //  -4  12 -> 2 6  5
    {{MB_Border,B8x8_Border},{{3,4},{5,6}}},   //  -4   4  -> 2 4 6
    {{MB_Border,NO_Border},{{3,4},{5,5}}},    //  -4   8  -> 2 5 7
    {{NO_Border,NO_Border},{{3,3},{4,4}}},    //  0   12  -> 3 6 8
    {{NO_Border,NO_Border},{{3,3},{4,4}}},    //  0   16  -> 3 7 9
    {{B4x4_Border,B4x4_Border},{{2,3},{4,5}}},    //  0   8  -> 3 5 10
    {{B4x4_Border,MB_Border},{{2,3},{3,4}}},    //  4  -12  -> 4 0 11
    {{NO_Border,MB_Border},{{2,2},{3,4}}},    //  4  -8  -> 4 1 12
    {{B8x8_Border,MB_Border},{{1,2},{3,4}}},    //  4  -4  -> 4 2 13
    {{B4x4_Border,B4x4_Border},{{0,1},{2,3}}},    //  8  0  -> 5 3 14
    {{NO_Border,NO_Border},{{2,2},{3,3}}},    //  8  -8  -> 5 1 15
    {{B8x8_Border,NO_Border},{{1,2},{3,3}}},    //  8  -4  -> 5 2 16
    {{NO_Border,NO_Border},{{1,1},{2,2}}},    //  12  0  -> 6 3 17
    {{NO_Border,NO_Border},{{0,0},{0,0}}},    //None             18
    {{B8x8_Border,B4x4_Border},{{1,2},{2,3}}},    //  12  -4  -> 6 3 19
    {{NO_Border,NO_Border},{{1,1},{2,2}}}    //  16  0  -> 7 3 20
};

ErrVal
MbData::upsampleMotionESS (MbData* rcBaseMbData,
                           const UInt uiBaseMbStride,
                           const Int aiPelOrig[2], 
                           const Bool bDirect8x8,
                           ResizeParameters* pcParameters)
{
    Bool        abBaseMbIntra [4];  // flag indicating if base MB is Intra or not
    MbMode      aeMbMode      [4];
    BlkMode     aeBlkMode    [4][4];  
    BorderType  aeBorder	  [4][2]; //NO_BORDER MB_BORDER 8x8_BORDER 4x4_BORDER
    UInt        auiMbIdx	  [4][4]; 
    UInt        aui4x4Idx	  [4][4];

    // initializing
    //-------------
    xInitESS(rcBaseMbData,
            uiBaseMbStride,
            aiPelOrig,
            bDirect8x8,
            pcParameters,
            abBaseMbIntra,
            aeMbMode,
            aeBlkMode);

    // macroblock mode choice
    //-----------------------
    xInheritMbMotionData (abBaseMbIntra ,aeMbMode,aeBlkMode,aeBorder,auiMbIdx,aui4x4Idx);

    // 8x8 blocks partitioning
    //------------------------
    xInherit8x8MotionData (abBaseMbIntra ,aeMbMode,aeBlkMode,  aeBorder,auiMbIdx,aui4x4Idx);

    // Transfer in MB structure
    //-------------------------
    RNOK( xFillMbMvData(pcParameters ) );

    //--- Set fwd/bwd 
    UInt uiFwdBwd = 0;
    for( Int n = 3; n >= 0; n--)
    {
        uiFwdBwd <<= 4;
        uiFwdBwd += (0 < m_apcMbMotionData[0]->getRefIdx( Par8x8(n) )) ? 1:0;
        uiFwdBwd += (0 < m_apcMbMotionData[1]->getRefIdx( Par8x8(n) )) ? 2:0;
    }
    m_usFwdBwd = uiFwdBwd;

    return Err::m_nOK;
}

ErrVal
MbData::xInitESS(MbData* rcBaseMbData,
                 const UInt uiBaseMbStride,
                 const Int aiPelOrig[2],
                 const Bool bDirect8x8,
                 ResizeParameters* pcParameters,
                 Bool        abBaseMbIntra[4],  
                 MbMode      aeMbMode	  [4],
                 BlkMode     aeBlkMode	  [4][4])
{
    //--- Initialisation
    ///////////////////////
    m_bBLSkipFlag = false;
    m_bBLQRefFlag = false; 

    m_uiMbCbp = 0;
    m_eMbMode =   INTRA_4X4;
    m_aBlkMode[0] = m_aBlkMode[1] = m_aBlkMode[2] = m_aBlkMode[3] =BLK_8x8;

    Mv* pcMv0=m_acBl4x4Mv[0];
    Mv* pcMv1=m_acBl4x4Mv[1];
    SChar* pscRefIdx0=m_ascBl4x4RefIdx[0];
    SChar* pscRefIdx1=m_ascBl4x4RefIdx[1];
    for(UInt  uiBl4x4Idx=0 ; uiBl4x4Idx < 16 ;uiBl4x4Idx++,pcMv0++,pcMv1++,pscRefIdx0++,pscRefIdx1++)
    {
        pcMv0->setZero();
        pcMv1->setZero();
        *pscRefIdx0=*pscRefIdx1=-1;
    }

    abBaseMbIntra[0]=abBaseMbIntra[1]=abBaseMbIntra[2]=abBaseMbIntra[3]=false;

    //---- Set ResidualAvailFlagsBase  
    ///////////////////////////////////
    //added by Samsung B.K. LEE  from JVT-P089 (adapted by jerome.vieron@thomson.net)
    const Int iBaseX0 = (aiPelOrig[0]*pcParameters->m_iInWidth + pcParameters->m_iInWidth/2) / pcParameters->m_iOutWidth; 
    const Int iBaseY0 = (aiPelOrig[1]*pcParameters->m_iInHeight + pcParameters->m_iInHeight/2) / pcParameters->m_iOutHeight; 
    const Int iBaseX1 = ((aiPelOrig[0]+15)*pcParameters->m_iInWidth + pcParameters->m_iInWidth/2) / (pcParameters->m_iOutWidth); 
    const Int iBaseY1 = ((aiPelOrig[1]+15)*pcParameters->m_iInHeight + pcParameters->m_iInHeight/2) / pcParameters->m_iOutHeight; 

    const MbData* pcBaseMb = & (rcBaseMbData[(iBaseY0>>4) * uiBaseMbStride + (iBaseX0>>4)]); ;
    const Int iSMbWidth   = ( iBaseX1>>4 > iBaseX0>>4 ) ? 2 : 1;
    const Int iSMbHeight  = ( iBaseY1>>4 > iBaseY0>>4 ) ? 2 : 1;

    UShort usResidualAvailFlagsBase = 0;
    for( Int iBaseMbY = 0; iBaseMbY < iSMbHeight; iBaseMbY ++)
        for( Int iBaseMbX = 0; iBaseMbX < iSMbWidth; iBaseMbX ++)
        {  
            pcBaseMb = pcBaseMb+(iBaseMbY>>4) * uiBaseMbStride + (iBaseMbX>>4); 
            usResidualAvailFlagsBase |= pcBaseMb->isLumaResidualAvailable()   ? 15 : 0;
            usResidualAvailFlagsBase |= pcBaseMb->isChromaResidualAvailable() ? (1 << 4) : 0;
        }
    setResidualAvailFlagsBase( usResidualAvailFlagsBase );

    //--- MB_Border 8x8Border distances derivation
    //////////////////////////////////////////////////
    Int aiBaseMb[2],aiBaseCenter[2],aidBorder[2],aidBorderplus1[2];

    aiBaseCenter[0]  = ((aiPelOrig[0]+8)*pcParameters->m_iInWidth + pcParameters->m_iInWidth/2) / pcParameters->m_iOutWidth; 
    aidBorder[0]     =(((8*(aiBaseCenter[0]>>3))*pcParameters->m_iOutWidth + pcParameters->m_iInWidth/2) / pcParameters->m_iInWidth ) - (aiPelOrig[0]+8);
    aidBorderplus1[0]=(((8*((aiBaseCenter[0]>>3)+1))*pcParameters->m_iOutWidth + pcParameters->m_iInWidth/2) / pcParameters->m_iInWidth ) - (aiPelOrig[0]+8);

    aiBaseCenter[1]  = ((aiPelOrig[1]+8)*pcParameters->m_iInHeight + pcParameters->m_iInHeight/2) / pcParameters->m_iOutHeight; 
    aidBorder[1]     =(((8*(aiBaseCenter[1]>>3))*pcParameters->m_iOutHeight + pcParameters->m_iInHeight/2) / pcParameters->m_iInHeight ) - (aiPelOrig[1]+8);
    aidBorderplus1[1]=(((8*((aiBaseCenter[1]>>3)+1))*pcParameters->m_iOutHeight + pcParameters->m_iInHeight/2) / pcParameters->m_iInHeight ) - (aiPelOrig[1]+8);


    for(UInt uiDim=0; uiDim<2; uiDim++)
    {
        if(((((aiBaseCenter[uiDim])>>3)<<3)%16)==0)
        {
            m_aiMbBorder[uiDim] = aidBorder[uiDim];
            m_ai8x8Border[uiDim] = aidBorderplus1[uiDim];
        }
        else
        {
            m_aiMbBorder[uiDim] = aidBorderplus1[uiDim];
            m_ai8x8Border[uiDim] = aidBorder[uiDim];
        }

        aiBaseMb[uiDim]   = aiBaseCenter[uiDim] - 16*((m_aiMbBorder[uiDim]>-6)&&(m_aiMbBorder[uiDim]<=0)); // -6 because after quantiz,-6 gives -8

        m_aiMbBorder[uiDim]=CALC_COND(m_aiMbBorder[uiDim]);
        m_ai8x8Border[uiDim]=CALC_COND(m_ai8x8Border[uiDim]);

        if((m_aiMbBorder[uiDim]<0)&&(m_ai8x8Border[uiDim]==0)) m_aiMbBorder[uiDim]*=-1;
        else
            if((m_aiMbBorder[uiDim]==0)&&(m_ai8x8Border[uiDim]<0)) m_ai8x8Border[uiDim]*=-1;

        if (abs(m_aiMbBorder[uiDim]-m_ai8x8Border[uiDim]) == 20)
            m_ai8x8Border[uiDim] -= sign(m_ai8x8Border[uiDim])*4;
    }

    //--- MB Class derivation and Base MB association 
    ///////////////////////////////////////////////////
    Int iMbBaseIdx =(aiBaseMb[1]>>4)*uiBaseMbStride + (aiBaseMb[0]>>4);
    m_apcMbData[1]=m_apcMbData[2]=m_apcMbData[3]=0;   

    m_apcMbData[0]= &(rcBaseMbData[iMbBaseIdx]);
    if(abs(m_aiMbBorder[0])>=8)
    {
        if(abs(m_aiMbBorder[1])>=8) 
        {
            m_eClass = Corner;
        }
        else 
        {
            m_eClass = Hori;
            m_apcMbData[2]= &(rcBaseMbData[iMbBaseIdx+uiBaseMbStride]);
        }
    }
    else
    {
        if(abs(m_aiMbBorder[1])>=8) 
        {
            m_eClass = Vert;
            m_apcMbData[1]= &(rcBaseMbData[iMbBaseIdx+1]);
        }
        else 
        {
            m_eClass = Center;
            m_apcMbData[1]= &(rcBaseMbData[iMbBaseIdx+1]);
            m_apcMbData[2]= &(rcBaseMbData[iMbBaseIdx+uiBaseMbStride]);
            m_apcMbData[3]= &(rcBaseMbData[iMbBaseIdx+uiBaseMbStride+1]);
        }
    }

    //Fill in array of Base MbMode
    ///////////////////////////////
    UInt uiNbBaseMb=m_eClass>>1;
    for(UInt uiNbMb=0 ; uiNbMb < uiNbBaseMb ; uiNbMb++)
    {
        UInt uiIdxMb=uiNbMb<<(m_eClass%2);
        MbMode eMbMode= m_apcMbData[uiIdxMb]->m_eMbMode;  
        if(eMbMode>=INTRA_4X4 )  
        {
            abBaseMbIntra[uiIdxMb]=true;
            eMbMode=MODE_16x16;
        }
        else
        {
            if(eMbMode==MODE_SKIP) eMbMode = MODE_8x8;
            for(UInt uiB8x8Idx=0;uiB8x8Idx<4;uiB8x8Idx++)
            {
                BlkMode eBlkMode=m_apcMbData[uiIdxMb]->m_aBlkMode[uiB8x8Idx];
                if (eBlkMode == BLK_SKIP)
                    eBlkMode = (bDirect8x8) ? BLK_8x8 : BLK_4x4;

                aeBlkMode[uiIdxMb][uiB8x8Idx]=eBlkMode;
            }
        }

        aeMbMode[uiIdxMb]= eMbMode;
    }

    return Err::m_nOK;   
}

ErrVal
MbData::xInitUpsampleInfo(BorderType   aeBorder [4][2],
                          UInt          auiMbIdx [4][4]	 , 
                          UInt          aui4x4Idx[4][4]		)
{
    //mapIdx= ((MB_BORDER/4 + 3)*3  +  ((8x8_BORDER/4 + 3)%3)   )-1
    UInt uiMapIdxX= 8+ (m_aiMbBorder[0]>>2)*3 + ((m_ai8x8Border[0]>>2)+3)%3;     
    UInt uiMapIdxY= 8+ (m_aiMbBorder[1]>>2)*3 + ((m_ai8x8Border[1]>>2)+3)%3;

    const InfoBaseDim& rInfX=m_aMapInfoBaseDim[uiMapIdxX];
    const InfoBaseDim& rInfY=m_aMapInfoBaseDim[uiMapIdxY];

    UInt uiB8x8Idx;
    for( Int x=0;x<2;x++)
    {
        for( Int y=0;y<2;y++)
        {
            uiB8x8Idx=(y<<1) + x;

            UInt uidxbaseX0=rInfX.ucIdx4x4Base[x][0];
            UInt uidxbaseX1=rInfX.ucIdx4x4Base[x][1];
            UInt uidxbaseY0=rInfY.ucIdx4x4Base[y][0];
            UInt uidxbaseY1=rInfY.ucIdx4x4Base[y][1];

            aui4x4Idx[uiB8x8Idx][0]=((uidxbaseY0%4)<<2)|(uidxbaseX0%4);
            aui4x4Idx[uiB8x8Idx][1]=((uidxbaseY0%4)<<2)|(uidxbaseX1%4);
            aui4x4Idx[uiB8x8Idx][2]=((uidxbaseY1%4)<<2)|(uidxbaseX0%4);
            aui4x4Idx[uiB8x8Idx][3]=((uidxbaseY1%4)<<2)|(uidxbaseX1%4);

            auiMbIdx[uiB8x8Idx][0]=((uidxbaseY0>>2)<<1)|(uidxbaseX0>>2);
            auiMbIdx[uiB8x8Idx][1]=((uidxbaseY0>>2)<<1)|(uidxbaseX1>>2);
            auiMbIdx[uiB8x8Idx][2]=((uidxbaseY1>>2)<<1)|(uidxbaseX0>>2);
            auiMbIdx[uiB8x8Idx][3]=((uidxbaseY1>>2)<<1)|(uidxbaseX1>>2);

            aeBorder[uiB8x8Idx][0]=rInfX.aeBorderType[x];
            aeBorder[uiB8x8Idx][1]=rInfY.aeBorderType[y];
        }
    }
    return Err::m_nOK;
}


ErrVal
MbData::xInheritMbMotionData(const Bool       abBaseMbIntra[4] ,
							   const MbMode     aeMbMode     [4],
                               const BlkMode    aeBlkMode    [4][4],
                               BorderType	aeBorder     [4][2],
							   UInt       auiMbIdx     [4][4]	, 
                               UInt		aui4x4Idx    [4][4])
{
   UInt uiNbBaseMb=m_eClass>>1;
   Bool bIsIntra=true;
   for(UInt uiNbMb=0 ; uiNbMb < uiNbBaseMb ; uiNbMb++) 
     bIsIntra&=abBaseMbIntra[uiNbMb<<(m_eClass%2)]; 

   if(bIsIntra) m_eMbMode = INTRA_4X4;
   else
	{
		//Remplissage des idx des blocks de base
		xInitUpsampleInfo(aeBorder,auiMbIdx,aui4x4Idx);

		if( m_eClass == Center) m_eMbMode = MODE_8x8;
		else 
		{
			Bool bIsBase8x8=false;
			Char cSuffix[2]; 
			MbMode eMbMode;

			if( m_eClass == Corner) 
			{
				eMbMode= aeMbMode[0];
				bIsBase8x8 =( eMbMode==MODE_8x8);
				cSuffix[0] =  m_acSuffixMbMode[0][eMbMode];  
				cSuffix[1] =  m_acSuffixMbMode[1][eMbMode]; 
				xComputeMbModeSize(cSuffix[0],0,bIsBase8x8,aeBlkMode,auiMbIdx,aui4x4Idx);
				xComputeMbModeSize(cSuffix[1],1,bIsBase8x8,aeBlkMode,auiMbIdx,aui4x4Idx);
			}
			else
			{
				UChar ucDim = ( m_eClass == Vert );
				cSuffix[ucDim] = 16;

                for(UInt uiNbMb=0 ; uiNbMb < uiNbBaseMb ; uiNbMb++)
				{
					UInt uiIdxMb=uiNbMb<<(m_eClass%2);
                    eMbMode=aeMbMode[uiIdxMb];
					bIsBase8x8 |=( eMbMode==MODE_8x8);
					cSuffix[ucDim] = min( cSuffix[ucDim], m_acSuffixMbMode[ucDim][eMbMode]); 
				}

				xComputeMbModeSize(cSuffix[ucDim],ucDim,bIsBase8x8,aeBlkMode,auiMbIdx,aui4x4Idx);
           		cSuffix[1-ucDim]=((abs(m_aiMbBorder[1-ucDim])==4)?4:8);
			}
			if ((cSuffix[0]==4)||(cSuffix[1]==4)) m_eMbMode = MODE_8x8;
			else  m_eMbMode=m_aeBuildMbMode[(cSuffix[0]>>3) -1 ][(cSuffix[1]>>3) -1];
		}
	}
 return Err::m_nOK;
}

Void
MbData::xComputeMbModeSize(Char& cSuffix, 
							 const UChar ucDim, 
                           const Bool bIsBase8x8,
                          const BlkMode    aeBlkMode    [4][4], 
                          const UInt       auiMbIdx     [4][4] , 
                          const UInt		aui4x4Idx    [4][4])
                           
 {
	 if(cSuffix!=16)
	 {
		 cSuffix = m_acComputeMbSize[(cSuffix>>3) -1][abs(m_ai8x8Border[ucDim])>>2];
		 if((cSuffix==16)&&(bIsBase8x8)&&(abs(m_ai8x8Border[ucDim])==8)&&(abs(m_aiMbBorder[ucDim])==8))
		 {
             BlkMode eBlkMode=aeBlkMode[auiMbIdx[0][0]][g_aucConvertTo8x8Idx[aui4x4Idx[0][0]]];
             Char cSubsizeb8x8idx0= m_acSuffixBlkMode[ucDim][eBlkMode%8];

             eBlkMode=aeBlkMode[auiMbIdx[3][0]][g_aucConvertTo8x8Idx[aui4x4Idx[3][0]]];
             Char cSubsizeb8x8idx3= m_acSuffixBlkMode[ucDim][eBlkMode%8];

			 cSuffix=min(cSuffix,2*min(cSubsizeb8x8idx0,cSubsizeb8x8idx3));             
		 }
	}
}          


ErrVal 
MbData::xInherit8x8MotionData(  const Bool       abBaseMbIntra[4] ,
                                const MbMode     aeMbMode     [4],
                                const BlkMode    aeBlkMode    [4][4],
                                const BorderType	aeBorder     [4][2],
                                const UInt       auiMbIdx     [4][4]	, 
                                const UInt		aui4x4Idx    [4][4])
 {
     Bool abBl8x8Intra [4];
     abBl8x8Intra[0]=abBl8x8Intra[1]=abBl8x8Intra[2]=abBl8x8Intra[3]=false;

     if(m_eMbMode>=INTRA_4X4)
     {
         m_aBlkMode[0]= m_aBlkMode[1]= m_aBlkMode[2]= m_aBlkMode[3]=BLK_8x8; 
         abBl8x8Intra[0]=abBl8x8Intra[1]=abBl8x8Intra[2]=abBl8x8Intra[3]=true;
     }
     else
     {
         UInt uiB8x8;
         for(uiB8x8=0 ; uiB8x8<4 ; uiB8x8++)
         {
             Char cSuffixX,cSuffixY;
             cSuffixX=xComputeSubMbModeSize(uiB8x8,0,abBaseMbIntra,aeMbMode,aeBlkMode,aeBorder,auiMbIdx,aui4x4Idx);
             if(cSuffixX>0)
                 cSuffixY=xComputeSubMbModeSize(uiB8x8,1,abBaseMbIntra,aeMbMode,aeBlkMode,aeBorder,auiMbIdx,aui4x4Idx);  

             if((cSuffixX <=0)||(cSuffixY <=0))
             {
                 m_aBlkMode[uiB8x8]=BLK_8x8; 
                 abBl8x8Intra[uiB8x8]=true;
             }   
             else
             {   
                 m_aBlkMode[uiB8x8]=m_aeBuildBlkMode[(cSuffixX>>2)-1][(cSuffixY>>2)-1];

                 const UInt* pucMbIdx =auiMbIdx[uiB8x8];
                 const UInt* puc4x4Idx=aui4x4Idx[uiB8x8];
                 const UChar *pucBlockOrder=&(g_aucConvertBlockOrder[uiB8x8<<2]);

                 for(UInt uiBl4x4Idx=0;uiBl4x4Idx<4;uiBl4x4Idx++,puc4x4Idx++,pucMbIdx++,pucBlockOrder++)
                 {
                     MbMotionData* pcmbMd0=m_apcMbData[*pucMbIdx]->m_apcMbMotionData[0];
                     MbMotionData* pcmbMd1=m_apcMbData[*pucMbIdx]->m_apcMbMotionData[1];

                     const UChar ucMapIdx=*puc4x4Idx;
                     const UChar uc8x8Idx=g_aucConvertTo8x8Idx[ucMapIdx];
                     m_acBl4x4Mv[0][*pucBlockOrder]     =pcmbMd0->getMv(B4x4Idx(ucMapIdx));
                     m_ascBl4x4RefIdx[0][*pucBlockOrder]=pcmbMd0->getRefIdx(Par8x8(uc8x8Idx));	
                     m_acBl4x4Mv[1][*pucBlockOrder]     =pcmbMd1->getMv(B4x4Idx(ucMapIdx));
                     m_ascBl4x4RefIdx[1][*pucBlockOrder]=pcmbMd1->getRefIdx(Par8x8(uc8x8Idx));

                     //fill cbp
                     ///////////
                     if((m_apcMbData[*pucMbIdx]->getMbExtCbp()>>ucMapIdx)&1)
                         m_uiMbCbp|=(1<<(*pucBlockOrder));
                 }
             }
         }    

         //Homogenization step
         ////////////////////////////////
         //--- Merge Ref Indexes and Mvs inside a given 8x8 block  
         for ( uiB8x8=0 ; uiB8x8<4 ; uiB8x8++) 
             if(m_aBlkMode[uiB8x8]!= BLK_8x8) xMergeBl8x8MvAndRef(uiB8x8);

         //--- Remove 8x8 INTRA blocks
         for ( uiB8x8=0 ; uiB8x8<4 ; uiB8x8++) 
             if(abBl8x8Intra[uiB8x8]) xRemoveIntra(uiB8x8,abBl8x8Intra);
     }

     return Err::m_nOK;
 }

 Char
MbData::xGetMbModeSize(const UInt uiB8x8Idx , 
                             const UChar ucDim,
                             const MbMode     aeMbMode     [4],   
                             const UInt       auiMbIdx     [4][4]	)
{
    UInt idx0=auiMbIdx[uiB8x8Idx][0];
    UInt idx3=auiMbIdx[uiB8x8Idx][3];

    const Char SuffixDim0=m_acSuffixMbMode[ucDim][aeMbMode[idx0]];
    const Char SuffixDim3=m_acSuffixMbMode[ucDim][aeMbMode[idx3]];

    return MINBLOCKSIZE(SuffixDim0,SuffixDim3);
}

Char
MbData::xGetSubMbModeSize(const UInt uiB8x8Idx , 
							  const UChar ucDim,
							  const BlkMode    aeBlkMode    [4][4], 
							  const UInt       auiMbIdx     [4][4] , 
							const UInt		aui4x4Idx    [4][4])
{
    UInt idx0=auiMbIdx[uiB8x8Idx][0];
    UInt idx3=auiMbIdx[uiB8x8Idx][3];

     BlkMode eBlkMode=aeBlkMode[idx0][g_aucConvertTo8x8Idx[aui4x4Idx[uiB8x8Idx][0]]];
     const Char SuffixDim0= m_acSuffixBlkMode[ucDim][eBlkMode%8];

     eBlkMode=aeBlkMode[idx3][g_aucConvertTo8x8Idx[aui4x4Idx[uiB8x8Idx][3]]];
     const Char SuffixDim3= m_acSuffixBlkMode[ucDim][eBlkMode%8];

    return MINBLOCKSIZE(SuffixDim0,SuffixDim3);
}

Char
MbData::xComputeSubMbModeSize(const UInt       uiB8x8Idx , 
                              const UChar      ucDim, 
                               const Bool       abBaseMbIntra[4] ,
							   const MbMode     aeMbMode     [4],
                               const BlkMode    aeBlkMode    [4][4],
                               const BorderType	aeBorder     [4][2],
							   const UInt       auiMbIdx     [4][4]	, 
                               const UInt		aui4x4Idx    [4][4])
{
    Bool bIsMbIntra;

    if((aeBorder[uiB8x8Idx][0]==MB_Border)&&(aeBorder[uiB8x8Idx][1]==MB_Border))
        bIsMbIntra=(abBaseMbIntra[0]&&abBaseMbIntra[1]&&abBaseMbIntra[2]&&abBaseMbIntra[3]);
    else
        bIsMbIntra=(abBaseMbIntra[auiMbIdx[uiB8x8Idx][0]]&&abBaseMbIntra[auiMbIdx[uiB8x8Idx][3]]);   

    if (bIsMbIntra) return -1;
    else
    {
        BorderType cBorder = aeBorder[uiB8x8Idx][ucDim];
        if(cBorder == MB_Border) return 4;
        else 
            if(cBorder == B8x8_Border)  return (xGetMbModeSize(uiB8x8Idx,ucDim,aeMbMode,auiMbIdx)/2);
            else
                if(cBorder == B4x4_Border) 	return xGetSubMbModeSize(uiB8x8Idx,ucDim,aeBlkMode,auiMbIdx,aui4x4Idx); 
    }

    return 8;
}


ErrVal
MbData::xMergeBl8x8MvAndRef(const UInt uiBlIdx)
{
    const UChar* pucWhich=&(g_aucConvertBlockOrder[uiBlIdx*4]);
    SChar ascChosenRefIdx[2]={MSYS_INT8_MAX,MSYS_INT8_MAX};
	UInt uiIdxstride=((m_aBlkMode[uiBlIdx]==BLK_8x4)?2:1);
	const UInt uiNbBl4x4=((m_aBlkMode[uiBlIdx]==BLK_4x4)?4:2);
	const UChar* pucMap=0;
	UInt uiList,Bl4x4Idx;

     for(uiList=0 ; uiList < 2 ; uiList++)
    {
        SChar* pscBlkRefIdx=&(m_ascBl4x4RefIdx[uiList][*pucWhich]);
        pucMap=pucWhich;

        for(Bl4x4Idx=0;Bl4x4Idx<uiNbBl4x4;Bl4x4Idx++,pucMap+=uiIdxstride)
        { 
            SChar scRefIdx=m_ascBl4x4RefIdx[uiList][*pucMap];
            if( scRefIdx>0) 
            {
                if(ascChosenRefIdx[uiList]>scRefIdx) ascChosenRefIdx[uiList]=scRefIdx;
            }
        }	
    }

    ascChosenRefIdx[0]=((ascChosenRefIdx[0]==MSYS_INT8_MAX)? -1:ascChosenRefIdx[0]);
    ascChosenRefIdx[1]=((ascChosenRefIdx[1]==MSYS_INT8_MAX)? -1:ascChosenRefIdx[1]);

    for(uiList=0 ; uiList < 2 ; uiList++)
    {
        if(ascChosenRefIdx[uiList]>0)
        {
            pucMap=pucWhich;
            for(Bl4x4Idx=0;Bl4x4Idx<4;Bl4x4Idx++,pucMap++)
            {
                if(m_ascBl4x4RefIdx[uiList][*pucMap]!=ascChosenRefIdx[uiList])	
                    xFillMvandRefBl4x4(Bl4x4Idx,pucWhich,uiList,ascChosenRefIdx);
            }
        }
        else
        {
            pucMap=pucWhich;
            for(Bl4x4Idx=0;Bl4x4Idx<4;Bl4x4Idx++,pucMap++)
            {
                m_ascBl4x4RefIdx[uiList][*pucMap]=-1;
                m_acBl4x4Mv[uiList][*pucMap].setZero();
            }
        }
    }

    return Err::m_nOK;
}





ErrVal
MbData::xFillMvandRefBl4x4(const UInt uiBlIdx,const UChar* pucWhich,const UInt uiList,const SChar* psChosenRefIdx)
{	
    UChar ucRealBlIdx=pucWhich[uiBlIdx];
    UChar ucPredIdx=m_aucPredictor[0][uiBlIdx];
    UChar ucRealPredIdx=pucWhich[ucPredIdx];

    if(m_ascBl4x4RefIdx[uiList][ucRealPredIdx]!=psChosenRefIdx[uiList])
    {
        ucPredIdx=m_aucPredictor[1][uiBlIdx];
        ucRealPredIdx=pucWhich[ucPredIdx];

        if(m_ascBl4x4RefIdx[uiList][ucRealPredIdx]!=psChosenRefIdx[uiList])		
            xFillMvandRefBl4x4(ucPredIdx,pucWhich,uiList,psChosenRefIdx);
    }	

    m_ascBl4x4RefIdx[uiList][ucRealBlIdx]=m_ascBl4x4RefIdx[uiList][ucRealPredIdx];
    m_acBl4x4Mv[uiList][ucRealBlIdx]=m_acBl4x4Mv[uiList][ucRealPredIdx];

    return Err::m_nOK;
}


ErrVal
MbData::xRemoveIntra(const UInt uiBlIdx, const Bool* abBl8x8Intra)
{	
    UChar ucPredIdx;
    ucPredIdx=m_aucPredictor[0][uiBlIdx] ;
    if(abBl8x8Intra[ucPredIdx])
    {
        ucPredIdx=m_aucPredictor[1][uiBlIdx] ;
        if(abBl8x8Intra[ucPredIdx])
            xRemoveIntra(ucPredIdx,abBl8x8Intra);
    }
    xCopyBl8x8(ucPredIdx,uiBlIdx);

    return Err::m_nOK;
}

ErrVal
MbData::xCopyBl8x8(const UInt uiBlIdx,const UInt uiBlIdxCopy)
{
    const UChar ucB4x4Idx=g_aucConvertTo4x4Idx[uiBlIdx];
    const UChar ucB4x4IdxCopy=g_aucConvertTo4x4Idx[uiBlIdxCopy];

    for( UInt uiList=0 ; uiList < 2 ; uiList++)
    {
        Mv* pcblkMv      =&(m_acBl4x4Mv[uiList][ucB4x4Idx]);
        SChar* pscBlkRefIdx=&(m_ascBl4x4RefIdx[uiList][ucB4x4Idx]);
        Mv* pcblkMvCopy   =&(m_acBl4x4Mv[uiList][ucB4x4IdxCopy]);
        SChar* pscBlkRefIdxCopy=&(m_ascBl4x4RefIdx[uiList][ucB4x4IdxCopy]);

        pcblkMvCopy[0]=pcblkMv[0];pcblkMvCopy[1]=pcblkMv[1];pcblkMvCopy[4]=pcblkMv[4];pcblkMvCopy[5]=pcblkMv[5];
        pscBlkRefIdxCopy[0]=pscBlkRefIdx[0]; pscBlkRefIdxCopy[1]=pscBlkRefIdx[1]; pscBlkRefIdxCopy[4]=pscBlkRefIdx[4]; pscBlkRefIdxCopy[5]=pscBlkRefIdx[5];
    }

    m_aBlkMode[uiBlIdxCopy]=m_aBlkMode[uiBlIdx];

    return Err::m_nOK;
}

ErrVal
MbData::xFillMbMvData(ResizeParameters* pcParameters )
{
    //--- IF ESS PICTURE LEVEL
    if( pcParameters->m_iExtendedSpatialScalability == ESS_PICT )
    {
        Mv      deltaMv[4];
        Int     refFrNum, refPosX, refPosY;
        Int     curFrNum = pcParameters->getPOC();
        Int     curPosX = pcParameters->getCurrentPictureParameters(curFrNum)->m_iPosX;
        Int     curPosY = pcParameters->getCurrentPictureParameters(curFrNum)->m_iPosY;

        for (UInt uilist=0; uilist<2; uilist++)
        {	for (UInt uiB8x8Idx=0; uiB8x8Idx<4 ; uiB8x8Idx++)
        {
            int refidx=	 m_ascBl4x4RefIdx[uilist][g_aucConvertTo4x4Idx[uiB8x8Idx]];
            if(refidx <=0)  deltaMv[uiB8x8Idx].set(0,0) ;
            else
            {
                refFrNum = pcParameters->m_aiRefListPoc[uilist][refidx-1];
                refPosX = pcParameters->getCurrentPictureParameters(refFrNum)->m_iPosX;
                refPosY = pcParameters->getCurrentPictureParameters(refFrNum)->m_iPosY;
                deltaMv[uiB8x8Idx].set( 4*(refPosX-curPosX) , 4*(refPosY-curPosY) );
            }
        }
        m_apcMbMotionData[uilist]->upsampleMotionNonDyad( m_ascBl4x4RefIdx[uilist] , m_acBl4x4Mv[uilist] , pcParameters , deltaMv );
        }
    }
    else
    {
        m_apcMbMotionData[0]->upsampleMotionNonDyad( m_ascBl4x4RefIdx[0] , m_acBl4x4Mv[0] , pcParameters );
        m_apcMbMotionData[1]->upsampleMotionNonDyad( m_ascBl4x4RefIdx[1] , m_acBl4x4Mv[1] , pcParameters );
    }

    return Err::m_nOK;   
}

// TMM_ESS_UNIFIED }


ErrVal
MbData::storeIntraBaseCoeffs( MbTransformCoeffs& rcCoeffs )
{
  ROT( m_pcMbIntraBaseTCoeffs );
  
  ROFRS( ( m_pcMbIntraBaseTCoeffs = new MbTransformCoeffs() ), Err::m_nERR );
  m_pcMbIntraBaseTCoeffs->copyFrom( rcCoeffs );
  return Err::m_nOK;
}



H264AVC_NAMESPACE_END
