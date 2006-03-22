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




#if !defined(AFX_MBDATA_H__6F1A2BEC_47BC_4944_BE36_C0E96ED39557__INCLUDED_)
#define AFX_MBDATA_H__6F1A2BEC_47BC_4944_BE36_C0E96ED39557__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "H264AVCCommonLib/Mv.h"
#include "H264AVCCommonLib/MbMvData.h"
#include "H264AVCCommonLib/MbTransformCoeffs.h"
#include "H264AVCCommonLib/MbDataStruct.h"

H264AVC_NAMESPACE_BEGIN

// TMM_ESS_UNIFIED {
enum MbClass
{
  Corner = 0x02,  
  Vert   = 0x04,  
  Hori   = 0x05,
  Center = 0x08
};

enum BorderType
{
  NO_Border   =  0x00,  
  MB_Border   =  0x01,  
  B8x8_Border =  0x02,
  B4x4_Border =  0x03
};
// TMM_ESS_UNIFIED } 

class H264AVCCOMMONLIB_API MbData :
public MbDataStruct
{
public:
  MbData()
  : m_pcMbTCoeffs         ( 0 )
  , m_bHasMotionRefinement( false )
  {
    m_apcMbMvdData   [ LIST_0 ]  = NULL;
    m_apcMbMvdData   [ LIST_1 ]  = NULL;
    m_apcMbMotionData[ LIST_0 ]  = NULL;
    m_apcMbMotionData[ LIST_1 ]  = NULL;
  }

  ~MbData()
  {
  }

  Void init(  MbTransformCoeffs*  pcMbTCoeffs,
              MbMvData*           pcMbMvdDataList0,
              MbMvData*           pcMbMvdDataList1,
              MbMotionData*       pcMbMotionDataList0,
              MbMotionData*       pcMbMotionDataList1,
              MbMotionData*       pcMbMotionDataBaseList0 = NULL,
              MbMotionData*       pcMbMotionDataBaseList1 = NULL)
  {
    AOT( m_bHasMotionRefinement );
    m_pcMbTCoeffs           = pcMbTCoeffs;
    m_apcMbMvdData[0]       = pcMbMvdDataList0;
    m_apcMbMvdData[1]       = pcMbMvdDataList1;
    m_apcMbMotionData[0]    = pcMbMotionDataList0;
    m_apcMbMotionData[1]    = pcMbMotionDataList1;
    m_apcMbMotionDataBase[0] = pcMbMotionDataBaseList0;
    m_apcMbMotionDataBase[1] = pcMbMotionDataBaseList1;
  }

public:
  MbTransformCoeffs&        getMbTCoeffs    ()                          { return *m_pcMbTCoeffs; }
  MbMvData&                 getMbMvdData    ( ListIdx eListIdx )        { return *m_apcMbMvdData   [ eListIdx ]; }
  MbMotionData&             getMbMotionData ( ListIdx eListIdx )        { return *m_apcMbMotionData[ eListIdx ]; }

  const MbTransformCoeffs&  getMbTCoeffs    ()                    const { return *m_pcMbTCoeffs; }
  const MbMvData&           getMbMvdData    ( ListIdx eListIdx )  const { return *m_apcMbMvdData   [ eListIdx ]; }
  const MbMotionData&       getMbMotionData ( ListIdx eListIdx )  const { return *m_apcMbMotionData[ eListIdx ]; }

  MbMotionData&             getMbMotionDataBase ( ListIdx eListIdx )        { return m_bHasMotionRefinement ? *m_apcMbMotionDataBase[eListIdx] : *m_apcMbMotionData[ eListIdx ]; }
  const MbMotionData&       getMbMotionDataBase ( ListIdx eListIdx )  const { return m_bHasMotionRefinement ? *m_apcMbMotionDataBase[eListIdx] : *m_apcMbMotionData[ eListIdx ]; }

  Void                      switchMotionRefinement();
  Void                      activateMotionRefinement();
  Void                      deactivateMotionRefinement()                    { m_bHasMotionRefinement = false; }

  operator MbTransformCoeffs& ()                                        { return *m_pcMbTCoeffs; }


  ErrVal  loadAll( FILE* pFile );
  ErrVal  saveAll( FILE* pFile );

  ErrVal  copyMotion    ( MbData& rcMbData, UInt    uiSliceId = MSYS_UINT_MAX );
  ErrVal  copyMotionBL  ( MbData& rcMbData, Bool bDirect8x8, UInt    uiSliceId = MSYS_UINT_MAX );
  ErrVal  upsampleMotion( MbData& rcMbData, Par8x8  ePar8x8, Bool bDirect8x8   );

	// TMM_ESS_UNIFIED {
  ErrVal upsampleMotionESS( MbData* pcBaseMbData,const UInt uiBaseMbStride,const Int aiPelOrig[2],const Bool bDirect8x8,ResizeParameters* pcParameters);
  ErrVal  noUpsampleMotion(); 
  // TMM_ESS_UNIFIED }

protected:
  MbTransformCoeffs*  m_pcMbTCoeffs;
  MbMvData*           m_apcMbMvdData[2];
  MbMotionData*       m_apcMbMotionData[2];

  MbMode              m_eMbModeBase;
  BlkMode             m_aBlkModeBase[4];
  MbMotionData*       m_apcMbMotionDataBase[2];
  Bool                m_bHasMotionRefinement;

// TMM_ESS_UNIFIED {
  typedef struct  
  {       
   BorderType aeBorderType	[2];
   UChar ucIdx4x4Base		[2][2];
  } InfoBaseDim;

  static const UChar		    m_aucPredictor   [2][4];
  static const BlkMode      m_aeBuildBlkMode [2][2];
  static const MbMode		    m_aeBuildMbMode  [2][2];
  static const Char			    m_acSuffixMbMode [2][7];
  static const Char			    m_acSuffixBlkMode[2][4];
  static const Char			    m_acComputeMbSize[2][5];
  static const InfoBaseDim	m_aMapInfoBaseDim[];

  MbData*                   m_apcMbData     [4];
  SChar                     m_ascBl4x4RefIdx[2][16];// ref index of list_0/1 for each 4x4 blk
  Mv                        m_acBl4x4Mv	    [2][16];// motion vectors of list_0/1 for each 4x4 blk
  Int                       m_aiMbBorder	[2];
  Int                       m_ai8x8Border	[2];
  MbClass                   m_eClass;               //current Mb Class

  ErrVal xInitESS             ( MbData*           pcBaseMbData,
                                const UInt uiBaseMbStride,
                                const Int         aiPelOrig[2],
                                const             Bool bDirect8x8,
                                ResizeParameters* pcParameters,
                                Bool              abBaseMbIntra[4],  
                                MbMode            aeMbMode	  [4],
                                BlkMode           aeBlkMode	  [4][4]);

  ErrVal xInitInfoBaseDim     (InfoBaseDim*     pInf, 
                               const UChar     ucDim);
  
  ErrVal xInitUpsampleInfo    ( BorderType      aeBorder [4][2],
                                UInt            auiMbIdx [4][4], 
                                UInt            aui4x4Idx[4][4]	 );

  ErrVal xInheritMbMotionData ( const Bool      abBaseMbIntra[4] ,
							                  const MbMode    aeMbMode     [4],
                                const BlkMode   aeBlkMode    [4][4],
                                BorderType      aeBorder     [4][2],
                                UInt            auiMbIdx     [4][4]	, 
                                UInt		        aui4x4Idx    [4][4]	);

  ErrVal xInherit8x8MotionData( const Bool       abBaseMbIntra[4] ,
							                  const MbMode     aeMbMode     [4],
                                const BlkMode    aeBlkMode    [4][4],
                                const BorderType aeBorder     [4][2],
							                  const UInt       auiMbIdx     [4][4], 
                                const UInt		   aui4x4Idx    [4][4]);

  ErrVal xFillMbMvData		  ( ResizeParameters* pcParameters );

  Char   xGetMbModeSize		  ( const UInt      uiB8x8Idx , 
                              const UChar     ucDim,
                              const MbMode    aeMbMode     [4],   
                              const UInt      auiMbIdx     [4][4]	);

  Char   xGetSubMbModeSize	( const UInt      uiB8x8Idx , 
							                const UChar     ucDim,
							                const BlkMode   aeBlkMode    [4][4], 
							                const UInt      auiMbIdx     [4][4] , 
							                const UInt	    aui4x4Idx    [4][4]);

  Void   xComputeMbModeSize	  ( Char&           cSuffix, 
              							    const UChar     ucDim, 
                                const Bool      bIsBase8x8,
                                const BlkMode   aeBlkMode    [4][4], 
                                const UInt      auiMbIdx     [4][4], 
                                const UInt      aui4x4Idx    [4][4]);

  Char   xComputeSubMbModeSize( const UInt          uiB8x8Idx , 
                                const UChar         ucDim, 
                                const Bool          abBaseMbIntra[4],
							                  const MbMode        aeMbMode     [4],
                                const BlkMode       aeBlkMode    [4][4],
                                const BorderType    aeBorder     [4][2],
							                  const UInt          auiMbIdx     [4][4], 
                                const UInt		      aui4x4Idx    [4][4] );
							  
  ErrVal xMergeBl8x8MvAndRef( const UInt uiBlIdx	);
  ErrVal xFillMvandRefBl4x4	( const UInt uiBlIdx, const UChar* pucWhich, const UInt uiList, const SChar* psChosenRefIdx );
  ErrVal xCopyBl8x8			    ( const UInt uiBlIdx, const UInt uiBlIdxCopy ); 
  ErrVal xRemoveIntra		    ( const UInt uiBlIdx, const Bool* abBl8x8Intra );
// TMM_ESS_UNIFIED }
};


H264AVC_NAMESPACE_END


#endif // !defined(AFX_MBDATA_H__6F1A2BEC_47BC_4944_BE36_C0E96ED39557__INCLUDED_)
