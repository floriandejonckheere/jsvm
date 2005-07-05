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

  return Err::m_nOK;
}


// TMM_ESS {

////////////////////
//ESS Generic
////////////////////
ErrVal
MbData::noUpsampleMotion()
{
  clear();
  m_apcMbMotionData[0]->clear(BLOCK_NOT_AVAILABLE) ;
  m_apcMbMotionData[1]->clear(BLOCK_NOT_AVAILABLE) ;
  return Err::m_nOK;      
}

ErrVal
MbData::upsampleMotionNonDyad( MbData* rcBaseMbData, Int iMbPelOrigX, Int iMbPelOrigY, ResizeParameters* pcParameters )
{
  Int      iB4x4X , iB4x4Y;
  Int      iB8x8X , iB8x8Y;

  // initializing
  //-------------
  m_bBLSkipFlag = false;
  m_bBLQRefFlag = false;
	
  m_eMbMode = MODE_8x8; // by default, MB is splitted in 8x8 blks
  m_aBlkMode[0] = m_aBlkMode[1] = m_aBlkMode[2] = m_aBlkMode[3] = BLK_4x4; // by default, 8x8 blks are splitted in 4x4 blks
  setMbExtCbp( 0 );

  // get 4x4 blocks Infos
  //---------------------
  // loop on 4x4 blocks of current high res MB
  for ( iB4x4Y=0 ; iB4x4Y<4 ; iB4x4Y++ )
  {
    m_iPelOrigY = iMbPelOrigY + 4*iB4x4Y;
    for ( iB4x4X=0 ; iB4x4X<4 ; iB4x4X++ )
    {
      m_iPelOrigX = iMbPelOrigX + 4*iB4x4X;
			xInherit4x4BlkMotion( iB4x4X , iB4x4Y , rcBaseMbData, pcParameters );
		}
    }
  // 8x8 blocks partitioning
  //------------------------
  // loop on 8x8 blocks of current high res MB
  for ( iB8x8Y=0 ; iB8x8Y<2 ; iB8x8Y++ )
    for ( iB8x8X=0 ; iB8x8X<2 ; iB8x8X++ )
			xDecide8x8BlkPartition( iB8x8X , iB8x8Y, pcParameters );
  // macroblock mode choice
  //-----------------------
	xDecideMacroblockMode();

	// Transfer in MB structure
  //-------------------------
	RNOK( xFillMbMvData(pcParameters ) );

	// Set fwd/bwd 
	//-------------	
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
MbData::xInherit4x4BlkMotion( Int iB4x4X , Int iB4x4Y , MbData *rcBaseMbData, ResizeParameters* pcParameters )
{
    int       iPelInBaseX, iPelInBaseY;
    Int       iMbInBaseX, iMbInBaseY;
    MbMode    eMbMode;  // mb mode

#if DO_QDIV
    // operator // for equations p.87
    int       divShiftW = (int) ( floor( log( (double)pcParameters->m_iOutWidth ) / log(2.) ) + 15 );
    int       divScaleW = ( (1<<divShiftW) + pcParameters->m_iOutWidth/2 ) / pcParameters->m_iOutWidth;
    int       divShiftH = (int) ( floor( log( (double)pcParameters->m_iOutHeight ) / log(2.) ) + 15 );
    int       divScaleH = ( (1<<divShiftH) + pcParameters->m_iOutHeight/2 ) / pcParameters->m_iOutHeight;
    int       num;
#endif

    // 4x4 block inherits from base layer
    //-----------------------------------
#if DO_QDIV 
    // operator // for equations p.87
    num = ( m_iPelOrigY + 2 ) * pcParameters->m_iInHeight + pcParameters->m_iInHeight/2;
    iPelInBaseY = ((((num&0xffff)*divScaleH)>>16)+(num>>16)*divScaleH)>>(divShiftH-16);
    num = ( m_iPelOrigX + 2 ) * pcParameters->m_iInWidth + pcParameters->m_iInWidth/2;
    iPelInBaseX = ((((num&0xffff)*divScaleW)>>16)+(num>>16)*divScaleW)>>(divShiftW-16);
#else
    iPelInBaseY = ( ( m_iPelOrigY + 2 ) * pcParameters->m_iInHeight + pcParameters->m_iInHeight/2) / (Int)(pcParameters->m_iOutHeight);
    iPelInBaseX = (( m_iPelOrigX + 2 ) * pcParameters->m_iInWidth + pcParameters->m_iInWidth/2) / (Int)(pcParameters->m_iOutWidth);    
#endif

    iMbInBaseY = iPelInBaseY / 16;
    iMbInBaseX = iPelInBaseX / 16;

    // computes BL MB pos, block8x8 pos, block4x4 pos
    Int       iMBIdx = iMbInBaseY * (pcParameters->m_iInWidth/16) + iMbInBaseX; // num of mb 16x16
    Par8x8    ePar8x8 = Par8x8( ((iPelInBaseY%16)/8) * 2 + ((iPelInBaseX%16)/8) ); // num of blk 8x8 of BL MB
    LumaIdx   c4x4Idx = B4x4Idx( ((iPelInBaseY%16)/4) * 4 + ((iPelInBaseX%16)/4) ); // num of blk 4x4 of BL MB

    //current corner of blk 4x4 inherits from BL MB
    eMbMode = rcBaseMbData[iMBIdx].getMbMode();
    MbMotionData* pcmbMd0=rcBaseMbData[iMBIdx].m_apcMbMotionData[0];
    MbMotionData* pcmbMd1=rcBaseMbData[iMBIdx].m_apcMbMotionData[1];
    UInt      uiB4x4Idx = iB4x4Y * 4 + iB4x4X;

    //fill cbp
    if((rcBaseMbData[iMBIdx].getMbExtCbp()>>(iB4x4Y*4+iB4x4X))&1) 
        m_uiMbCbp|=(1<<(iB4x4Y*4+iB4x4X));

    if ( eMbMode >= INTRA_4X4 )
    {
        m_aeBl4x4MbMode[uiB4x4Idx] = INTRA_4X4;
        m_ascBl4x4RefIdx[0][uiB4x4Idx] = -1;
        m_ascBl4x4RefIdx[1][uiB4x4Idx] = -1; // signals that no list is used 
        m_acBl4x4Mv[0][uiB4x4Idx].setZero();
        m_acBl4x4Mv[1][uiB4x4Idx].setZero();
    }
    else
    {
        m_aeBl4x4MbMode[uiB4x4Idx]		= MODE_8x8; // will indicates further that 4x4 blk is not intra
        m_ascBl4x4RefIdx[0][uiB4x4Idx]	= pcmbMd0->getRefIdx(ePar8x8); // reference index
        m_acBl4x4Mv[0][uiB4x4Idx]		= pcmbMd0->getMv(c4x4Idx); // motion vector
        m_ascBl4x4RefIdx[1][uiB4x4Idx]	= pcmbMd1->getRefIdx(ePar8x8); // reference index
        m_acBl4x4Mv[1][uiB4x4Idx]		= pcmbMd1->getMv(c4x4Idx); // motion vector
    } 
    return Err::m_nOK;
}

ErrVal
MbData::xDecide8x8BlkPartition( Int iB8x8X , Int iB8x8Y, ResizeParameters* pcParameters )
{
  UInt   uiB8x8Idx = iB8x8Y * 2 + iB8x8X;  
  UInt   uiB4x4Idx =g_aucConvertBlockOrder[uiB8x8Idx*4];

  // check intra block
  if ( m_aeBl4x4MbMode[uiB4x4Idx]==INTRA_4X4 && m_aeBl4x4MbMode[uiB4x4Idx+1]==INTRA_4X4 && 
       m_aeBl4x4MbMode[uiB4x4Idx+4]==INTRA_4X4 && m_aeBl4x4MbMode[uiB4x4Idx+5]==INTRA_4X4 )
  {
    m_abBl8x8Intra[uiB8x8Idx] = true;
    m_aBlkMode[uiB8x8Idx] = BLK_8x8;
   }
  else
  {
    m_abBl8x8Intra[uiB8x8Idx] = false;
    m_aBlkMode[uiB8x8Idx] = BLK_8x8; // default value
	xMergeBl4x4MvAndRef(uiB8x8Idx);
    xTryToMergeBl8x8(uiB8x8Idx);// 8x8 block partitioning decision
   } 
  return Err::m_nOK;
}

ErrVal
MbData::xDecideMacroblockMode( )
{
  UInt uiSumIntra=m_abBl8x8Intra[0]+ m_abBl8x8Intra[1]+m_abBl8x8Intra[2]+m_abBl8x8Intra[3];

  if ( uiSumIntra==4 )   
  {
    m_eMbMode = INTRA_4X4;
      m_aBlkMode[0]=m_aBlkMode[1]=m_aBlkMode[2]=m_aBlkMode[3]=BLK_8x8; 
   }
  else
  {
	  for ( UInt uiBlIdx=0 ; uiBlIdx<4 ; uiBlIdx++ ) if(m_abBl8x8Intra[uiBlIdx])xRemoveIntra(uiBlIdx);
       
	 xTryToMergeBl16x16();
  }	
  return Err::m_nOK;
}

////////////////////
//ESS 3/2
////////////////////
const MbData::MbClassMotionData MbData::m_mbmotiondata[9] = 
{	
	{1,4,0,{0,0,0,0},{{0,1,2,3},{0,0,0,0},{0,0,0,0},{0,0,0,0}}, {0 ,0 ,0 ,0 , 1 ,2 ,1 ,2 , 4 ,4 ,8 ,8 , 5, 6 ,9 ,10},m_eModesCorner }, //C_0
	{2,2,0,{0,1,0,0},{{0,2,0,0},{1,3,0,0},{0,0,0,0},{0,0,0,0}}, {3 ,3 , 3 ,3 , 7 ,7 ,11,11, 0 ,0 , 0 ,0 , 4 ,4 , 8 ,8},m_eModesVertical }, //V_0
	{1,4,1,{1,0,0,0},{{1,0,3,2},{0,0,0,0},{0,0,0,0},{0,0,0,0}}, {3 ,3 ,3 ,3 , 1 ,2 ,1 ,2 ,7 ,7 ,11 ,11 , 5 ,6 ,9 ,10 },m_eModesCorner },  //C_1
	{2,2,0,{0,2,0,0},{{0,1,0,0},{2,3,0,0},{0,0,0,0},{0,0,0,0}}, {12,12,12,12,13,14,13,14,0 ,0 ,0 ,0 ,1, 2 ,1 ,2},m_eModesHorizontal  }, //H_0
	{4,1,0,{0,1,2,3},{{0,0,0,0},{1,0,0,0},{2,0,0,0},{3,0,0,0}}, { 15,15, 15, 15, 12, 12, 12,12, 3 , 3 , 3, 3, 0 , 0,  0, 0},m_eModesCenter}, //Center
	{2,2,1,{1,3,0,0},{{1,0,0,0},{3,2,0,0},{0,0,0,0},{0,0,0,0}}, {15,15,15,15,13,14,13,14,3 ,3 ,3  ,3 ,1 ,2 ,1  ,2 },m_eModesHorizontal}, //H_1
	{1,4,2,{2,0,0,0},{{2,3,0,1},{0,0,0,0},{0,0,0,0},{0,0,0,0}},{12, 12,12 ,12 , 13, 14,13,14 , 4 , 4 ,8 ,8 , 5 , 6 ,9 ,10},m_eModesCorner },  //C_2
	{2,2,1,{2,3,0,0},{{2,0,0,0},{3,1,0,0},{0,0,0,0},{0,0,0,0}}, {15,15, 15,15, 7 ,7 , 11,11, 12,12, 12,12, 4 ,4 , 8 ,8},m_eModesVertical }, //V_1
	{1,4,3,{3,0,0,0},{{3,2,1,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}},{15, 15, 15, 15, 13, 14, 13, 14, 7 ,7, 11, 11, 5 ,6 ,9, 10},m_eModesCorner }  //C_3
};



const BlkMode MbData::m_eModesCorner [20]=
{	 BLK_8x8,BLK_4x8,BLK_8x4,BLK_4x4, 
	 BLK_8x8,BLK_8x8,BLK_8x8,BLK_8x8,	
	 BLK_8x8,BLK_8x8,BLK_8x4,BLK_8x4,
	 BLK_8x8,BLK_4x8,BLK_8x8,BLK_4x8,
	 BLK_8x8,BLK_4x8,BLK_8x4,BLK_4x4};

const BlkMode MbData::m_eModesVertical [10]=
{BLK_8x8, BLK_8x4, BLK_8x8, BLK_8x8, BLK_8x8, BLK_8x4, BLK_8x8, BLK_8x8, BLK_8x8  ,BLK_8x4};

const BlkMode MbData::m_eModesHorizontal [10]=
{BLK_8x8, BLK_4x8,BLK_8x8, BLK_8x8,BLK_8x8, BLK_8x8, BLK_8x8, BLK_4x8,BLK_8x8, BLK_4x8};

const BlkMode MbData::m_eModesCenter[5]=
{BLK_8x8, BLK_8x8, BLK_8x8, BLK_8x8, BLK_8x8 };


ErrVal
MbData::upsampleMotion3_2(const  MbData* apcBaseMbData[4],
							   SMbIdx   eSMbIdx,
							   Bool bDirect8x8, 
							   ResizeParameters* pcParameters)
{
    m_bDirect8x8=bDirect8x8;

	 //--- Initialisation
	 Mv* pcMv0=m_acBl4x4Mv[0];
	 Mv* pcMv1=m_acBl4x4Mv[1];
	 SChar* pscRefIdx0=m_ascBl4x4RefIdx[0];
	 SChar* pscRefIdx1=m_ascBl4x4RefIdx[1];
	 for(UInt  uiBl4x4Idx=0;uiBl4x4Idx<16;uiBl4x4Idx++,pcMv0++,pcMv1++,pscRefIdx0++,pscRefIdx1++)
	{
		pcMv0->setZero();
		pcMv1->setZero();
		*pscRefIdx0=*pscRefIdx1=-1;
	}
    m_eMbMode=INTRA_4X4;
	m_aBlkMode[0] = m_aBlkMode[1] = m_aBlkMode[2] = m_aBlkMode[3] =BLK_8x8;
    m_abBl8x8Intra[0]=m_abBl8x8Intra[1]=m_abBl8x8Intra[2]=m_abBl8x8Intra[3]=false;

	RNOK(xInheritMbMotionData3_2(apcBaseMbData,eSMbIdx));
	
	m_bBLSkipFlag = false;
    m_bBLQRefFlag = false;
  //  m_bInCropWindowFlag = true; 
	
	RNOK( m_apcMbMotionData[0]->upsampleMotionNonDyad( m_ascBl4x4RefIdx[0] , m_acBl4x4Mv[0] , pcParameters ) );
	RNOK( m_apcMbMotionData[1]->upsampleMotionNonDyad( m_ascBl4x4RefIdx[1] , m_acBl4x4Mv[1] , pcParameters ) );

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
MbData::xInheritMbMotionData3_2(const MbData* apcMbData[4],
								const SMbIdx  eSMbIdx)
{
	UInt uiBl8x8Idx,uiBl4x4Idx,uiPredIdx;
	MbMotionData* pcmbMd0,*pcmbMd1;
	UInt uiSumMode=0;
	MbMode eMbMode;
	const MbClassMotionData& mnsn=m_mbmotiondata[eSMbIdx];

	//Loop on the base corresponding MBs
	for(UInt nbMb=0;nbMb<mnsn.usNbMbBase;nbMb++)
	{
		//Get base MB mode 
		eMbMode=apcMbData[mnsn.auiIdxMbData[nbMb]]->m_eMbMode;

		if(m_bDirect8x8&&eMbMode==MODE_SKIP) eMbMode=MODE_16x16;

		//--- Fill in 4x4 blocks Motion data and 8x8 blk partitioning mode (in function of the base MB mode)	
		////////////////////////////////////////////////////////////////////
		const UInt *puiBl8x8Idx=mnsn.auiIdxBl8x8[nbMb];
		
		if(eMbMode>=INTRA_4X4)
		{
			for( UInt uiB8x8=0; uiB8x8 < mnsn.usNbBl8x8 ; uiB8x8++,puiBl8x8Idx++)
			{
				m_aBlkMode[*puiBl8x8Idx]=BLK_8x8; 
				m_abBl8x8Intra[*puiBl8x8Idx]=true;
				uiSumMode+=BLK_8x8;
			}
		}
		else
		{
			pcmbMd0=apcMbData[mnsn.auiIdxMbData[nbMb]]->m_apcMbMotionData[0];
			pcmbMd1=apcMbData[mnsn.auiIdxMbData[nbMb]]->m_apcMbMotionData[1];
			
			const UChar *pucMap=&(mnsn.puc4x4Map[nbMb*mnsn.usNbBl8x8*4]);
			for( UInt uiB8x8=0; uiB8x8<mnsn.usNbBl8x8 ; uiB8x8++,puiBl8x8Idx++)
			{
				//Fill in 8x8 blk partitioning mode 
				m_aBlkMode[*puiBl8x8Idx]=mnsn.peModeSMb[eMbMode*mnsn.usNbBl8x8 + uiB8x8];
				uiSumMode+=m_aBlkMode[*puiBl8x8Idx];

				//Fill in the Ref idx and Mv data for 4x4 blocks
				const UChar *pucBlockOrder=&(g_aucConvertBlockOrder[(*puiBl8x8Idx)*4]);
				for( uiBl4x4Idx=0;uiBl4x4Idx<4;uiBl4x4Idx++,pucMap++,pucBlockOrder++)
				{
					const UChar ucMapIdx=*pucMap;
					const UChar uc8x8Idx=g_aucConvertTo8x8Idx[ucMapIdx];
					m_acBl4x4Mv[0][*pucBlockOrder]     =pcmbMd0->getMv(B4x4Idx(ucMapIdx));
					m_ascBl4x4RefIdx[0][*pucBlockOrder]=pcmbMd0->getRefIdx(Par8x8(uc8x8Idx));	
					m_acBl4x4Mv[1][*pucBlockOrder]     =pcmbMd1->getMv(B4x4Idx(ucMapIdx));
					m_ascBl4x4RefIdx[1][*pucBlockOrder]=pcmbMd1->getRefIdx(Par8x8(uc8x8Idx));	
				}
			}
		}
	}

	//--- MB mode decision
	/////////////////////////
	UInt uiSumIntra=m_abBl8x8Intra[0]+ m_abBl8x8Intra[1]+m_abBl8x8Intra[2]+m_abBl8x8Intra[3];

	if(uiSumIntra==4) m_eMbMode =INTRA_4X4;
	else
	{	
		if((eSMbIdx==C_0)||(eSMbIdx==C_1)||(eSMbIdx==C_2)||(eSMbIdx==C_3))//Corner
		{
			m_eMbMode=((eMbMode==MODE_16x16)?MODE_16x16:MODE_8x8);
			uiPredIdx=mnsn.usLutIdx;
		}
		else if((eSMbIdx==V_0)||(eSMbIdx==V_1)) //Vertical
		{
			m_eMbMode = ((uiSumMode==4*BLK_8x8)?MODE_8x16: MODE_8x8);
			uiPredIdx=mnsn.usLutIdx*2;
		}
		else if((eSMbIdx==H_0)||(eSMbIdx==H_1)) //Horizontal
		{
			m_eMbMode = ((uiSumMode==4*BLK_8x8)?MODE_16x8: MODE_8x8);
			uiPredIdx=mnsn.usLutIdx;
		} 
		else //Center
		{
      m_eMbMode = MODE_8x8;
		 uiPredIdx=0;
		}
	
	//8x8 blocks mode final decision
    ////////////////////////////////
    //--- Merge Ref Indexes and Mvs inside a given 8x8 block  
	for ( uiBl8x8Idx=0 ; uiBl8x8Idx<4 ; uiBl8x8Idx++) xMergeBl8x8MvAndRef(uiBl8x8Idx, uiPredIdx);
	//--- Remove 8x8 INTRA blocks
	for ( uiBl8x8Idx=0 ; uiBl8x8Idx<4 ; uiBl8x8Idx++) if(m_abBl8x8Intra[uiBl8x8Idx]) xRemoveIntra(uiBl8x8Idx);
	
	//--- Try to Merge sub-partitions of the current MB 
	xTryToMergeBl16x16();
	}
	return Err::m_nOK;
}
/////////////////////
//ESS COMMON TOOLS
/////////////////////
const UChar MbData::m_aucPredictor[2][4]=
{
{1,0,3,2},
{2,3,0,1}
};

Bool
MbData::xAreIdentical(const UInt uiBlIdx1,const UInt uiBlIdx2, const bool bB8x8)
{
  Bool    bIdentical = false;
  UChar   ucB4x4Idx1,ucB4x4Idx2;
  	
  if(bB8x8)
  {if (!( m_aBlkMode[uiBlIdx1]==BLK_8x8 && m_aBlkMode[uiBlIdx2]==BLK_8x8 )) return false; 
	ucB4x4Idx1=g_aucConvertTo4x4Idx[uiBlIdx1];
	ucB4x4Idx2=g_aucConvertTo4x4Idx[uiBlIdx2];
  }
  else
  {  ucB4x4Idx1=uiBlIdx1;  ucB4x4Idx2=uiBlIdx2;}	

  if((m_ascBl4x4RefIdx[0][ucB4x4Idx1] == m_ascBl4x4RefIdx[0][ucB4x4Idx2])&& 
	 (m_ascBl4x4RefIdx[1][ucB4x4Idx1] == m_ascBl4x4RefIdx[1][ucB4x4Idx2]))
  {
	if((m_acBl4x4Mv[0][ucB4x4Idx1] == m_acBl4x4Mv[0][ucB4x4Idx2])&& 
	   (m_acBl4x4Mv[1][ucB4x4Idx1] == m_acBl4x4Mv[1][ucB4x4Idx2]))
	    bIdentical=true;
   }
  return bIdentical;
}

ErrVal
MbData::xMergeBl4x4MvAndRef(const UInt uiBlIdx)
{
  	const UChar* pucWhich=&(g_aucConvertBlockOrder[uiBlIdx*4]);
	UInt uiList;
	SChar ascChosenRefIdx[2]={MSYS_INT8_MAX,MSYS_INT8_MAX};

	UInt Bl4x4Idx;
	const UChar* pucMap=0;
	
	for(uiList=0 ; uiList < 2 ; uiList++)
	{
		SChar* pscBlkRefIdx=&(m_ascBl4x4RefIdx[uiList][*pucWhich]);
		pucMap=pucWhich;
		
		for(Bl4x4Idx=0;Bl4x4Idx<4;Bl4x4Idx++,pucMap++)
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
//8x4 or 4x8
ErrVal
MbData::xMergeBlXxYMvAndRef(const UInt uiBlIdx,const Bool bis8x4, const UInt uiWhich)
{
	UChar ucBl4x4Idx1,ucBl4x4Idx2,ucBl4x4Idx3,ucBl4x4Idx4,ucB4x4IdxDual;
	const UChar ucB4x4Idx=g_aucConvertTo4x4Idx[uiBlIdx];
	const UChar ucWhich=g_aucConvertBlockOrder[uiWhich];

	if(bis8x4)
	{ucBl4x4Idx1=4*(uiWhich<=1);
	ucBl4x4Idx2=ucBl4x4Idx1+1;
    ucBl4x4Idx3=4-ucBl4x4Idx1;
    ucBl4x4Idx4= ucBl4x4Idx3+1;
	ucB4x4IdxDual=ucB4x4Idx+4;
	}
	else
	{
	ucBl4x4Idx1=1-(uiWhich%2);
	ucBl4x4Idx2=ucBl4x4Idx1+4;
    ucBl4x4Idx3=1-ucBl4x4Idx1;
    ucBl4x4Idx4= ucBl4x4Idx3+4;
	ucB4x4IdxDual=ucB4x4Idx+1;
	}

if((m_ascBl4x4RefIdx[0][ucB4x4Idx]!=m_ascBl4x4RefIdx[0][ucB4x4IdxDual])
	  ||(m_ascBl4x4RefIdx[1][ucB4x4Idx]!=m_ascBl4x4RefIdx[1][ucB4x4IdxDual]))
	{	
	for( UInt uiList=0 ; uiList < 2 ; uiList++)
		{
		  Mv* pcBlkMv      =&(m_acBl4x4Mv[uiList][ucB4x4Idx]);
		  SChar* pscBlkRefIdx=&(m_ascBl4x4RefIdx[uiList][ucB4x4Idx]);
		
		 if(pscBlkRefIdx[ucWhich]>0)
	   {
		 pscBlkRefIdx[ucBl4x4Idx1]=pscBlkRefIdx[ucBl4x4Idx2]=pscBlkRefIdx[ucWhich];
		 pcBlkMv[ucBl4x4Idx1]=pcBlkMv[ucBl4x4Idx2]=pcBlkMv[ucWhich];
	   }
	   else
	   {
	     pscBlkRefIdx[ucBl4x4Idx3]=pscBlkRefIdx[ucBl4x4Idx4]=pscBlkRefIdx[ucBl4x4Idx1];
		 pcBlkMv[ucBl4x4Idx3]=pcBlkMv[ucBl4x4Idx4]=pcBlkMv[ucBl4x4Idx1];
	   }
		}
	}
return Err::m_nOK;
}

ErrVal
MbData::xMergeBl8x8MvAndRef(const UInt uiBlIdx,const UInt uiWhich)
{
   	if(m_aBlkMode[uiBlIdx]==BLK_4x8) xMergeBlXxYMvAndRef(uiBlIdx,false,uiWhich);	
	    else if (m_aBlkMode[uiBlIdx]==BLK_8x4) xMergeBlXxYMvAndRef(uiBlIdx,true,uiWhich);	
		else if (m_aBlkMode[uiBlIdx]==BLK_4x4) xMergeBl4x4MvAndRef(uiBlIdx);	

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
MbData::xRemoveIntra(const UInt uiBlIdx)
{	
	UChar ucPredIdx;
	ucPredIdx=m_aucPredictor[0][uiBlIdx] ;
	if(m_abBl8x8Intra[ucPredIdx])
	{
		ucPredIdx=m_aucPredictor[1][uiBlIdx] ;
		if(m_abBl8x8Intra[ucPredIdx])
			xRemoveIntra(ucPredIdx);
	}
	xCopyBl8x8(ucPredIdx,uiBlIdx);

	return Err::m_nOK;
}

ErrVal
MbData::xTryToMergeBl16x16()
{
   if( m_eMbMode == MODE_16x16) return Err::m_nOK;

   if ( (xAreIdentical(0,1,true)==true) && (xAreIdentical(2,3,true)==true) )
    {
      if ( (xAreIdentical(0,2,true)==true) ) m_eMbMode = MODE_16x16;
      else  m_eMbMode = MODE_16x8;
    }
    else if ( (xAreIdentical(0,2,true)==true) && (xAreIdentical(1,3,true)==true) ) m_eMbMode = MODE_8x16;
   
	return Err::m_nOK;
}

ErrVal
MbData::xTryToMergeBl8x8(const UInt uiB8x8Idx)
{
 UInt   uiB4x4Idx =g_aucConvertBlockOrder[uiB8x8Idx*4];
 
 if ( (xAreIdentical(uiB4x4Idx, uiB4x4Idx+1)==true) 
	  && (xAreIdentical(uiB4x4Idx+4, uiB4x4Idx+5)==true) )
    {
      if ( xAreIdentical(uiB4x4Idx, uiB4x4Idx+4)==true )
        m_aBlkMode[uiB8x8Idx] = BLK_8x8;
      else
        m_aBlkMode[uiB8x8Idx] = BLK_8x4;
    }
    else if ( (xAreIdentical(uiB4x4Idx, uiB4x4Idx+4)==true) 
		&& (xAreIdentical(uiB4x4Idx+1, uiB4x4Idx+5)==true) )
      m_aBlkMode[uiB8x8Idx] = BLK_4x8;
    else
      m_aBlkMode[uiB8x8Idx] = BLK_4x4;

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
                if(refFrNum<0) {printf ("AAAAAAAAAAAAAAAAAAAAARRRRRRRRGG!!!\n");exit(-1);}

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

// TMM_ESS }

ErrVal
MbData::storeIntraBaseCoeffs( MbTransformCoeffs& rcCoeffs )
{
  ROT( m_pcMbIntraBaseTCoeffs );
  
  ROFRS( ( m_pcMbIntraBaseTCoeffs = new MbTransformCoeffs() ), Err::m_nERR );
  m_pcMbIntraBaseTCoeffs->copyFrom( rcCoeffs );
  return Err::m_nOK;
}



H264AVC_NAMESPACE_END
