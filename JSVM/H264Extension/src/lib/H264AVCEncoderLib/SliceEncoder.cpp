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



#include "H264AVCEncoderLib.h"
#include "MbEncoder.h"
#include "SliceEncoder.h"
#include "MbCoder.h"
#include "CodingParameter.h"
#include "RecPicBuffer.h"
#include "H264AVCCommonLib/PocCalculator.h"
#include "H264AVCCommonLib/Transform.h"

#include "H264AVCCommonLib/CFMO.h"

// JVT-W043 {
#include "RateCtlBase.h"
#include "RateCtlQuadratic.h"
// JVT-W043 }
H264AVC_NAMESPACE_BEGIN


SliceEncoder::SliceEncoder():
  m_pcMbEncoder       ( NULL ),
  m_pcMbCoder         ( NULL ),
  m_pcControlMng      ( NULL ),
  m_pcCodingParameter ( NULL ),
  m_pcPocCalculator   ( NULL ),
  m_bInitDone         ( false ),
  m_uiFrameCount(0),
  m_eSliceType        ( I_SLICE ),
  m_bTraceEnable      ( true ),
  m_pcTransform       ( NULL )
{
}


SliceEncoder::~SliceEncoder()
{
}

ErrVal SliceEncoder::create( SliceEncoder*& rpcSliceEncoder )
{
  rpcSliceEncoder = new SliceEncoder;

  ROT( NULL == rpcSliceEncoder );

  return Err::m_nOK;
}


ErrVal SliceEncoder::destroy()
{
  delete this;
  return Err::m_nOK;
}



ErrVal SliceEncoder::init( MbEncoder* pcMbEncoder,
                           MbCoder* pcMbCoder,
                           ControlMngIf* pcControlMng,
                           CodingParameter* pcCodingParameter,
                           PocCalculator* pcPocCalculator,
                           Transform* pcTransform)
{
  ROT( m_bInitDone );
  ROT( NULL == pcMbEncoder );
  ROT( NULL == pcMbCoder );
  ROT( NULL == pcControlMng );
  ROT( NULL == pcPocCalculator );
  ROT( NULL == pcTransform );

  m_pcTransform = pcTransform;
  m_pcMbEncoder = pcMbEncoder;
  m_pcMbCoder = pcMbCoder;
  m_pcControlMng = pcControlMng;
  m_pcCodingParameter = pcCodingParameter;
  m_pcPocCalculator = pcPocCalculator;


  m_uiFrameCount = 0;
  m_eSliceType =  I_SLICE;
  m_bTraceEnable = true;

  m_bInitDone = true;
  return Err::m_nOK;
}


ErrVal SliceEncoder::uninit()
{
  ROF( m_bInitDone );
  m_pcMbEncoder =  NULL;
  m_pcMbCoder =  NULL;
  m_pcControlMng =  NULL;
  m_bInitDone = false;

  m_uiFrameCount = 0;
  m_eSliceType =  I_SLICE;
  m_bTraceEnable = false;
  return Err::m_nOK;
}






ErrVal
SliceEncoder::encodeInterPictureP( UInt&            ruiBits,
                                   Frame*        pcFrame,
                                   Frame*        pcRecSubband,
                                   Frame*        pcPredSignal,
                                   ControlData&     rcControlData,
                                   UInt             uiMbInRow,
                                   RefFrameList&    rcRefFrameList,
                                   RefFrameList& rcRefFrameListBase,
																	 PicType       ePicType )
{
  ROF( m_bInitDone );

	SliceHeader&  rcSliceHeader         = *rcControlData.getSliceHeader          ( ePicType );
  MbDataCtrl*   pcMbDataCtrl          =  rcControlData.getMbDataCtrl            ();
  Frame*     pcBaseLayerFrame      =  rcControlData.getBaseLayerRec          ();
  Frame*     pcBaseLayerResidual   =  rcControlData.getBaseLayerSbb          ();
  MbDataCtrl*   pcBaseLayerCtrl       =  rcControlData.getBaseLayerCtrl         ();
  Double        dLambda               =  rcControlData.getLambda                ();
  Int           iSpatialScalabilityType =  rcControlData.getSpatialScalabilityType(); // TMM_ESS
  UInt          uiMbAddress           =  rcSliceHeader.getFirstMbInSlice        ();
  UInt          uiLastMbAddress       =  rcSliceHeader.getLastMbInSlice         ();
  UInt          uiBits                =  m_pcMbCoder ->getBitCount              ();

  //====== initialization ======
  RNOK( pcMbDataCtrl      ->initSlice         ( rcSliceHeader, ENCODE_PROCESS, false, NULL ) );
  if( pcBaseLayerCtrl )
  {
    RNOK( pcBaseLayerCtrl ->initSlice         ( rcSliceHeader, PRE_PROCESS,    false, NULL ) );
  }
  RNOK( m_pcControlMng    ->initSliceForCoding( rcSliceHeader ) );

 	if( ePicType!=FRAME )
	{
		if( pcFrame )             RNOK( pcFrame            ->addFieldBuffer( ePicType ) );
		if( pcRecSubband )        RNOK( pcRecSubband       ->addFieldBuffer( ePicType ) );
		if( pcPredSignal )        RNOK( pcPredSignal       ->addFieldBuffer( ePicType ) );
		if( pcBaseLayerFrame )    RNOK( pcBaseLayerFrame   ->addFieldBuffer( ePicType ) );
		if( pcBaseLayerResidual ) RNOK( pcBaseLayerResidual->addFieldBuffer( ePicType ) );
	}

	//JVT-X046 {
	UInt uiCodedMBNumber=0;
	m_pcMbCoder->bSliceCodedDone=false;
	//JVT-X046 }
  //===== loop over macroblocks =====
  for( ; uiMbAddress <= uiLastMbAddress; ) //--ICU/ETRI FMO Implementation
  {
    // JVT-W043 {
    if ( bRateControlEnable && !pcJSVMParams->m_uiLayerId )
    {
      pcGenericRC->m_pcJSVMParams->current_mb_nr = uiMbAddress;
      if( pcGenericRC->m_pcJSVMParams->basicunit == pcGenericRC->m_pcJSVMParams->FrameSizeInMbs ) {
        // qp remains unchanged
      }
      // basic unit layer rate control
      else if ( pcGenericRC->m_pcJSVMParams->type == P_SLICE ) {
        if( pcGenericRC->m_pcJSVMParams->NumberofCodedMacroBlocks > 0 
          && (pcGenericRC->m_pcJSVMParams->NumberofCodedMacroBlocks % pcGenericRC->m_pcJSVMParams->BasicUnit) == 0 ) {
            // frame coding
            if( pcGenericRC->m_pcJSVMParams->frame_mbs_only_flag ) {
              pcQuadraticRC->updateRCModel();
              pcGenericRC->m_pcJSVMParams->qp = pcQuadraticRC->updateQPRC2( pcGenericRC->m_iTopFieldFlag );
            }
          }
      }
    }
    // JVT-W043 }
    ETRACE_NEWMB( uiMbAddress );

    MbDataAccess* pcMbDataAccess     = NULL;
    MbDataAccess* pcMbDataAccessBase = NULL;
    UInt          uiMbY, uiMbX;
    Double        dCost              = 0;

    rcSliceHeader.getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddress );

    RNOK( pcMbDataCtrl      ->initMb          ( pcMbDataAccess,     uiMbY, uiMbX ) );
    if( pcBaseLayerCtrl )
    {
      RNOK( pcBaseLayerCtrl ->initMb          ( pcMbDataAccessBase, uiMbY, uiMbX ) );
  	}
    RNOK( m_pcControlMng   ->initMbForCoding( *pcMbDataAccess,    uiMbY, uiMbX, false, false  ) );
    pcMbDataAccess->getMbData().deactivateMotionRefinement();
    pcMbDataAccess->setMbDataAccessBase( pcMbDataAccessBase );
    pcMbDataAccess->getMbData().setFieldFlag(ePicType!=FRAME);

	  //JVT-U106 Behaviour at slice boundaries{
    if( ! rcSliceHeader.getNoInterLayerPredFlag() )
	    m_pcMbEncoder->setIntraBLFlag(m_pbIntraBLFlag[uiMbAddress]);
	  //JVT-U106 Behaviour at slice boundaries}
    // JVT-W043 {
    if ( bRateControlEnable && !pcJSVMParams->m_uiLayerId )
    {
      pcMbDataAccess->getMbData().setQp( pcGenericRC->m_pcJSVMParams->qp );
    }
    // JVT-W043 }

    if( rcRefFrameListBase.getSize() )
    {
      RNOK( m_pcMbEncoder     ->encodeInterP    ( *pcMbDataAccess,
                                                  pcMbDataAccessBase,
                                                  iSpatialScalabilityType,
                                                  pcFrame                                  ->getPic( ePicType ),
                                                  pcFrame                                  ->getPic( ePicType ),
                                                  pcRecSubband                             ->getPic( ePicType ),
                                                  pcPredSignal                             ->getPic( ePicType ),
																									pcBaseLayerFrame    ? pcBaseLayerFrame   ->getPic( ePicType ) : NULL,
																									pcBaseLayerResidual ? pcBaseLayerResidual->getPic( ePicType ) : NULL,
                                                  rcRefFrameList,
                                                  & rcRefFrameListBase,
                                                  dLambda,
                                                  dCost,
                                                  true ) );
    }
    else
    {
      RNOK( m_pcMbEncoder     ->encodeInterP    ( *pcMbDataAccess,
                                                  pcMbDataAccessBase,
                                                  iSpatialScalabilityType,
                                                  pcFrame                                  ->getPic( ePicType ),
                                                  pcFrame                                  ->getPic( ePicType ),
                                                  pcRecSubband                             ->getPic( ePicType ),
                                                  pcPredSignal                             ->getPic( ePicType ),
                                                  pcBaseLayerFrame    ? pcBaseLayerFrame   ->getPic( ePicType ) : NULL,
                                                  pcBaseLayerResidual ? pcBaseLayerResidual->getPic( ePicType ) : NULL,
                                                  rcRefFrameList,
                                                  0,
                                                  dLambda,
                                                  dCost,
                                                  true ) );
    }

    // JVT-W043 {
    if ( bRateControlEnable && !pcJSVMParams->m_uiLayerId )
    {
      pcGenericRC->m_iNumberofHeaderBits           += pcGenericRC->m_iRCHeaderBits;
      pcGenericRC->m_iNumberofBasicUnitHeaderBits  += pcGenericRC->m_iRCHeaderBits;
      pcGenericRC->m_iNumberofTextureBits          += pcGenericRC->m_iRCTextureBits;
      pcGenericRC->m_iNumberofBasicUnitTextureBits += pcGenericRC->m_iRCTextureBits;
      pcGenericRC->m_pcJSVMParams->NumberofCodedMacroBlocks++;
    }
    // JVT-W043 }
		//JVT-X046 {
    uiCodedMBNumber++;
		if (m_uiSliceMode==1) //fixed slice size in number of MBs
		{
			if (uiCodedMBNumber >= m_uiSliceArgument)// this slice is done
			{
				RNOK( m_pcMbCoder       ->encode         ( *pcMbDataAccess,
					pcMbDataAccessBase,
					iSpatialScalabilityType,
					true, true ) );
				rcControlData.m_uiCurrentFirstMB = rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress );
				rcSliceHeader.setNumMbsInSlice(uiCodedMBNumber);
				if (rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress)==-1)
					rcControlData.m_bSliceGroupAllCoded = true;
				break;
			}
			else
			{
				RNOK( m_pcMbCoder       ->encode         ( *pcMbDataAccess,
					pcMbDataAccessBase,
					iSpatialScalabilityType,
					(uiMbAddress == uiLastMbAddress), 
					true ) );
			}
		}
		else if(m_uiSliceMode==2) //fixed slice size in number of bytes
		{
			RNOK( m_pcMbCoder       ->encode         ( *pcMbDataAccess,
				pcMbDataAccessBase,
				iSpatialScalabilityType,
				(uiMbAddress == uiLastMbAddress), 
				true ) );
			if ((rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress)==-1) && (!m_pcMbCoder->bSliceCodedDone))
				rcControlData.m_bSliceGroupAllCoded = true;
			if (m_pcMbCoder->bSliceCodedDone==true)	
			{
				rcControlData.m_uiCurrentFirstMB = uiMbAddress;
				rcSliceHeader.setNumMbsInSlice(uiCodedMBNumber-1);

				//set SliceId equal to zero because this MBData belong to the next slice
				MbData* tempMbData;
				tempMbData=pcMbDataCtrl->xGetMbData(rcControlData.m_uiCurrentFirstMB);
				tempMbData->m_uiSliceId = 0;
				break;
			}
		}
		else 
		{
			RNOK( m_pcMbCoder       ->encode         ( *pcMbDataAccess,
				pcMbDataAccessBase,
				iSpatialScalabilityType,
				(uiMbAddress == uiLastMbAddress), 
				true ) );
		}
		//JVT-X046 }
    uiMbAddress = rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress );
		//JVT-X046 {
		if (uiMbAddress==-1)
		{
			rcControlData.m_bSliceGroupAllCoded=true;
			break;
		}
		//JVT-X046 }
  }

	if( ePicType!=FRAME )
	{
		if( pcFrame )             RNOK( pcFrame            ->removeFieldBuffer( ePicType ) );
		if( pcRecSubband )        RNOK( pcRecSubband       ->removeFieldBuffer( ePicType ) );
		if( pcPredSignal )        RNOK( pcPredSignal       ->removeFieldBuffer( ePicType ) );
		if( pcBaseLayerFrame )    RNOK( pcBaseLayerFrame   ->removeFieldBuffer( ePicType ) );
		if( pcBaseLayerResidual ) RNOK( pcBaseLayerResidual->removeFieldBuffer( ePicType ) );
	}

  ruiBits += m_pcMbCoder->getBitCount() - uiBits;

  return Err::m_nOK;
}




ErrVal SliceEncoder::encodeIntraPicture( UInt&        ruiBits,
                                         ControlData& rcControlData,
                                         Frame*    pcFrame,
                                         Frame*    pcRecSubband,
                                         Frame*    pcBaseLayer,
                                         Frame*    pcPredSignal,
                                         UInt         uiMbInRow,
                                         Double				dLambda,
																				 PicType      ePicType )
{
  ROF( m_bInitDone );

  SliceHeader&  rcSliceHeader           = *rcControlData.getSliceHeader           ( ePicType );
  MbDataCtrl*   pcMbDataCtrl      =  rcControlData.getMbDataCtrl            ();
  MbDataCtrl*   pcBaseLayerCtrl   =  rcControlData.getBaseLayerCtrl         ();
  Int           iSpatialScalabilityType =  rcControlData.getSpatialScalabilityType(); // TMM_ESS

  //====== initialization ======
  RNOK( pcMbDataCtrl      ->initSlice         ( rcSliceHeader, ENCODE_PROCESS, false, NULL ) );
  if( pcBaseLayerCtrl )
  {
    RNOK( pcBaseLayerCtrl ->initSlice         ( rcSliceHeader, PRE_PROCESS,    false, NULL ) );
  }
  RNOK( m_pcControlMng    ->initSliceForCoding( rcSliceHeader ) );

  //====== initialization ======
  UInt          uiMbAddress         = rcSliceHeader.getFirstMbInSlice ();
  UInt          uiLastMbAddress     = rcSliceHeader.getLastMbInSlice  ();
  UInt          uiBits              = m_pcMbCoder ->getBitCount       ();

  if(uiMbAddress == -1) return Err::m_nOK;
  if( ePicType!=FRAME )
	{
		if( pcFrame )      RNOK( pcFrame     ->addFieldBuffer( ePicType ) );
		if( pcRecSubband ) RNOK( pcRecSubband->addFieldBuffer( ePicType ) );
		if( pcBaseLayer )  RNOK( pcBaseLayer ->addFieldBuffer( ePicType ) );
		if( pcPredSignal ) RNOK( pcPredSignal->addFieldBuffer( ePicType ) );
	}
  
	//JVT-X046 {
	UInt uiCodedMBNumber = 0;
	m_pcMbCoder->bSliceCodedDone = false;
	//JVT-X046 }

  //===== loop over macroblocks =====
  for(  ; uiMbAddress <= uiLastMbAddress;  ) //--ICU/ETRI FMO Implementation
  {
    // JVT-W043 {
    if ( bRateControlEnable && !pcJSVMParams->m_uiLayerId )
    {
      pcGenericRC->m_pcJSVMParams->current_mb_nr = uiMbAddress;
      if( pcGenericRC->m_pcJSVMParams->basicunit == pcGenericRC->m_pcJSVMParams->FrameSizeInMbs ) {
        // qp remains unchanged
      }
      // basic unit layer rate control
      else if ( pcGenericRC->m_pcJSVMParams->RCUpdateMode == RC_MODE_1 ) {
        if( pcGenericRC->m_pcJSVMParams->NumberofCodedMacroBlocks > 0 
          && (pcGenericRC->m_pcJSVMParams->NumberofCodedMacroBlocks % pcGenericRC->m_pcJSVMParams->BasicUnit) == 0 ) {
            // frame coding
            if( pcGenericRC->m_pcJSVMParams->frame_mbs_only_flag ) {
              pcQuadraticRC->updateRCModel();
              if ( pcJSVMParams->m_uiIntraPeriod != 1 )
                pcGenericRC->m_pcJSVMParams->qp = pcQuadraticRC->updateQPRC2(pcGenericRC->m_iTopFieldFlag);
              else
                pcGenericRC->m_pcJSVMParams->qp = pcQuadraticRC->updateQPRC1(pcGenericRC->m_iTopFieldFlag);
            }
          }
      }
    }
    // JVT-W043 }
    ETRACE_NEWMB( uiMbAddress );

    MbDataAccess* pcMbDataAccess     = NULL;
    MbDataAccess* pcMbDataAccessBase = NULL;
    UInt          uiMbY, uiMbX;
    Double        dCost              = 0;

    rcSliceHeader.getMbPositionFromAddress    ( uiMbY, uiMbX, uiMbAddress );

    RNOK( pcMbDataCtrl      ->initMb          ( pcMbDataAccess,     uiMbY, uiMbX ) );
    if( pcBaseLayerCtrl )
    {
      RNOK( pcBaseLayerCtrl ->initMb          ( pcMbDataAccessBase, uiMbY, uiMbX ) );
    }
    RNOK( m_pcControlMng    ->initMbForCoding ( *pcMbDataAccess,    uiMbY, uiMbX, false, false ) );
    pcMbDataAccess->getMbData().deactivateMotionRefinement();
    pcMbDataAccess->setMbDataAccessBase( pcMbDataAccessBase );
    pcMbDataAccess->getMbData().setFieldFlag(ePicType!=FRAME);

    // JVT-W043 {
    if ( bRateControlEnable && !pcJSVMParams->m_uiLayerId )
    {
      pcMbDataAccess->getMbData().setQp( pcGenericRC->m_pcJSVMParams->qp );
    }
    // JVT-W043 }
	//JVT-U106 Behaviour at slice boundaries{
    if( ! rcSliceHeader.getNoInterLayerPredFlag() )
	     m_pcMbEncoder->setIntraBLFlag(m_pbIntraBLFlag[uiMbAddress]);
	//JVT-U106 Behaviour at slice boundaries}
    RNOK( m_pcMbEncoder     ->encodeIntra     ( *pcMbDataAccess,
                                                pcMbDataAccessBase,
                                                 pcFrame                   ->getPic( ePicType ),
                                                 pcFrame                   ->getPic( ePicType ),
                                                 pcRecSubband              ->getPic( ePicType ),
																								 pcBaseLayer ? pcBaseLayer ->getPic( ePicType ) : NULL,
																								 pcPredSignal              ->getPic( ePicType ),
                                                 dLambda,
                                                 dCost) );

    //JVT-X046 {
		uiCodedMBNumber++;
		if (m_uiSliceMode==1) //fixed slice size in number of MBs
		{		
			if (uiCodedMBNumber >= m_uiSliceArgument)//reach the fixed MB number
			{
				RNOK( m_pcMbCoder       ->encode          ( *pcMbDataAccess,
					pcMbDataAccessBase,
					iSpatialScalabilityType,
					true, true ) );
				rcControlData.m_uiCurrentFirstMB = rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress );
				rcControlData.getSliceHeader(ePicType)->setNumMbsInSlice(uiCodedMBNumber);
				if (rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress)==-1)
					rcControlData.m_bSliceGroupAllCoded = true;
				break;
			}
			else
			{
				RNOK( m_pcMbCoder       ->encode          ( *pcMbDataAccess,
					pcMbDataAccessBase,
					iSpatialScalabilityType,
					( uiMbAddress == uiLastMbAddress ), true ) );
			}
		}
		else if(m_uiSliceMode==2) //fixed slice size in number of MBs
		{
			RNOK( m_pcMbCoder       ->encode          ( *pcMbDataAccess,
				pcMbDataAccessBase,
				iSpatialScalabilityType,
				( uiMbAddress == uiLastMbAddress ), true ) );
			if ((rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress)==-1) && (!m_pcMbCoder->bSliceCodedDone))
				rcControlData.m_bSliceGroupAllCoded = true;
			if (m_pcMbCoder->bSliceCodedDone==true)// this slice is done
			{
				rcControlData.m_uiCurrentFirstMB = uiMbAddress;
				rcControlData.getSliceHeader(ePicType)->setNumMbsInSlice(uiCodedMBNumber-1);

				MbData* tempMbData;//set SliceId equal to zero because this MBData belong to the next slice
				tempMbData = pcMbDataCtrl->xGetMbData(rcControlData.m_uiCurrentFirstMB);
				tempMbData->m_uiSliceId = 0;
				break;
			}
		}
		else
		{
			RNOK( m_pcMbCoder       ->encode          ( *pcMbDataAccess,
			pcMbDataAccessBase,
			iSpatialScalabilityType,
			( uiMbAddress == uiLastMbAddress ), true ) );
		}
		//JVT-X046 }

    // JVT-W043 {
    if ( bRateControlEnable && !pcJSVMParams->m_uiLayerId )
    {
      pcGenericRC->m_iNumberofHeaderBits           += pcGenericRC->m_iRCHeaderBits;
      pcGenericRC->m_iNumberofBasicUnitHeaderBits  += pcGenericRC->m_iRCHeaderBits;
      pcGenericRC->m_iNumberofTextureBits          += pcGenericRC->m_iRCTextureBits;
      pcGenericRC->m_iNumberofBasicUnitTextureBits += pcGenericRC->m_iRCTextureBits;
      pcGenericRC->m_pcJSVMParams->NumberofCodedMacroBlocks++;
    }
    // JVT-W043 }
    uiMbAddress = rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress );
		//JVT-X046 {
		if (uiMbAddress==-1)
		{
			rcControlData.m_bSliceGroupAllCoded = true;
			break;
		}
		//JVT-X046 }
  }

  if( ePicType!=FRAME )
	{
		if( pcFrame )      RNOK( pcFrame     ->removeFieldBuffer( ePicType ) );
		if( pcRecSubband ) RNOK( pcRecSubband->removeFieldBuffer( ePicType ) );
		if( pcBaseLayer )  RNOK( pcBaseLayer ->removeFieldBuffer( ePicType ) );
		if( pcPredSignal ) RNOK( pcPredSignal->removeFieldBuffer( ePicType ) );
	}

  ruiBits += m_pcMbCoder->getBitCount() - uiBits;

  return Err::m_nOK;
}

// TMM_INTERLACE{
ErrVal SliceEncoder::encodeIntraPictureMbAff( UInt&				 ruiBits,
                                              ControlData& rcControlData,
                                              Frame*    pcOrgFrame,
                                              Frame*    pcFrame,
                                              Frame*    pcRecSubband,
                                              Frame*    pcBaseLayer,
                                              Frame*    pcPredSignal,
                                              UInt				 uiMbInRow,
                                              Double			 dLambda )
{
  ROF( m_bInitDone );

  SliceHeader&  rcSliceHeader           = *rcControlData.getSliceHeader           ( FRAME );
  MbDataCtrl*   pcMbDataCtrl            =  rcControlData.getMbDataCtrl            ();
  MbDataCtrl*   pcBaseLayerCtrl         =  rcControlData.getBaseLayerCtrl         ();
  MbDataCtrl*   pcBaseLayerCtrlField    =  rcControlData.getBaseLayerCtrlField   ();
  Int           iSpatialScalabilityType =  rcControlData.getSpatialScalabilityType();

  //====== initialization ======
  RNOK( pcMbDataCtrl      ->initSlice         ( rcSliceHeader, ENCODE_PROCESS, false, NULL ) );
  if( pcBaseLayerCtrl )
  {
    RNOK( pcBaseLayerCtrl ->initSlice         ( rcSliceHeader, PRE_PROCESS,    false, NULL ) );
  }

  if( pcBaseLayerCtrlField )
  {
    RNOK( pcBaseLayerCtrlField ->initSlice   ( rcSliceHeader, PRE_PROCESS,    false, NULL ) );
  }

  RNOK( m_pcControlMng    ->initSliceForCoding( rcSliceHeader ) );

  UInt uiBits = m_pcMbCoder ->getBitCount();

  Frame* apcOrgFrame  [4] = { NULL, NULL, NULL, NULL };
  Frame* apcFrame     [4] = { NULL, NULL, NULL, NULL };
  Frame* apcRecSubband[4] = { NULL, NULL, NULL, NULL };
  Frame* apcBaseLayer [4] = { NULL, NULL, NULL, NULL };
  Frame* apcPredSignal[4] = { NULL, NULL, NULL, NULL };

	RNOK( gSetFrameFieldArrays( apcOrgFrame,   pcOrgFrame   ) );
  RNOK( gSetFrameFieldArrays( apcFrame,      pcFrame      ) );
  RNOK( gSetFrameFieldArrays( apcRecSubband, pcRecSubband ) );
  RNOK( gSetFrameFieldArrays( apcBaseLayer,  pcBaseLayer  ) );
  RNOK( gSetFrameFieldArrays( apcPredSignal, pcPredSignal ) );

  YuvMbBuffer acIntYuvMbBufferPredSignal[2];
  YuvMbBuffer acIntYuvMbBufferRecSubBand[2];
  YuvMbBuffer acIntYuvMbBufferFrame     [2];

  UInt uiLastQp = rcSliceHeader.getSliceQp();

  //===== loop over macroblocks =====
  UInt          uiMbAddress     = rcSliceHeader.getFirstMbInSlice ();
  const UInt    uiLastMbAddress = rcSliceHeader.getLastMbInSlice  ();
  for( ; uiMbAddress<= uiLastMbAddress; uiMbAddress+=2 )
  {
    MbDataBuffer acMbData[2];
    Double adCost[2]  = {0,0};

    YuvMbBuffer cTempBuffer;

    UInt auiLastQpTest[2] = {uiLastQp, uiLastQp};
    Int eP;
    for( eP = 0; eP < 4; eP++ )
    {
      MbDataAccess* pcMbDataAccess     = NULL;
      MbDataAccess* pcMbDataAccessBase = NULL;
      Double        dCost              = 0;
      UInt          uiMbY, uiMbX;

      const Bool    bField = (eP < 2);
      const UInt    uiMbAddressMbAff   = uiMbAddress+(eP%2);

      rcSliceHeader.getMbPositionFromAddress         ( uiMbY, uiMbX, uiMbAddressMbAff      );

      RNOK( pcMbDataCtrl      ->initMb               (  pcMbDataAccess,     uiMbY, uiMbX ) );

      if  (eP < 2 && pcBaseLayerCtrlField)  // field case
      {
        RNOK( pcBaseLayerCtrlField         ->initMb( pcMbDataAccessBase, uiMbY, uiMbX ) );
      }
      else if  (eP >= 2 && pcBaseLayerCtrl )  //frame case
      {
        RNOK( pcBaseLayerCtrl         ->initMb( pcMbDataAccessBase, uiMbY, uiMbX ) );
      }

      RNOK( m_pcControlMng    ->initMbForCoding ( *pcMbDataAccess,    uiMbY, uiMbX, true, bField ) );
      pcMbDataAccess->getMbData().deactivateMotionRefinement();
			pcMbDataAccess->setMbDataAccessBase( pcMbDataAccessBase );

      pcMbDataAccess->setLastQp( auiLastQpTest[bField] );
      RNOK( m_pcMbEncoder     ->encodeIntra     ( *pcMbDataAccess,
                                                   pcMbDataAccessBase,
                                                   apcOrgFrame  [eP],
                                                   apcFrame     [eP],
                                                   apcRecSubband[eP],
                                                   apcBaseLayer [eP],
                                                   apcPredSignal[eP],
                                                   dLambda,
                                                   dCost ) );
      auiLastQpTest[bField] = pcMbDataAccess->getMbData().getQp();
      adCost [eP>>1] += dCost;
      if( bField )
      {
        acMbData[eP].copy( pcMbDataAccess->getMbData() );
        acIntYuvMbBufferPredSignal[eP].loadBuffer( apcPredSignal[eP]->getFullPelYuvBuffer() );
        acIntYuvMbBufferRecSubBand[eP].loadBuffer( apcRecSubband[eP]->getFullPelYuvBuffer() );
        acIntYuvMbBufferFrame     [eP].loadBuffer( apcFrame     [eP]->getFullPelYuvBuffer() );
      }
    }

    const Bool bFieldMode = ( adCost[0] < adCost[1] );
#ifdef RANDOM_MBAFF
    bFieldMode = gBoolRandom();
#endif

    // coding part

    for( eP = 0; eP < 2; eP++ )
    {
      MbDataAccess* pcMbDataAccess     = NULL;
      MbDataAccess* pcMbDataAccessBase = NULL;
      UInt          uiMbY, uiMbX;
      const UInt    uiMbAddressMbAff   = uiMbAddress+eP;

      ETRACE_NEWMB( uiMbAddressMbAff );

      rcSliceHeader.getMbPositionFromAddress    ( uiMbY, uiMbX, uiMbAddressMbAff );

      RNOK( pcMbDataCtrl      ->initMb          ( pcMbDataAccess,     uiMbY, uiMbX ) );

      if( bFieldMode && pcBaseLayerCtrlField )
      {
        RNOK( pcBaseLayerCtrlField ->initMb    ( pcMbDataAccessBase, uiMbY, uiMbX ) );
      }
      else if( !bFieldMode && pcBaseLayerCtrl )
      {
        RNOK( pcBaseLayerCtrl ->initMb          ( pcMbDataAccessBase, uiMbY, uiMbX ) );
      }

      RNOK( m_pcControlMng    ->initMbForCoding ( *pcMbDataAccess,    uiMbY, uiMbX, true, bFieldMode ) );

      if( bFieldMode )
      {
        pcMbDataAccess->getMbData().copy( acMbData[eP] );
        apcRecSubband [eP]->getFullPelYuvBuffer()->loadBuffer( &acIntYuvMbBufferRecSubBand[eP] );
        apcPredSignal [eP]->getFullPelYuvBuffer()->loadBuffer( &acIntYuvMbBufferPredSignal[eP] );
        apcFrame      [eP]->getFullPelYuvBuffer()->loadBuffer( &acIntYuvMbBufferFrame     [eP] );
      }
      pcMbDataAccess->setLastQp( uiLastQp );
      uiLastQp = pcMbDataAccess->getMbData().getQp();
			pcMbDataAccess->setMbDataAccessBase( pcMbDataAccessBase );

      RNOK( m_pcMbCoder       ->encode          ( *pcMbDataAccess,
                                                   pcMbDataAccessBase,
                                                   iSpatialScalabilityType,
                                                   ( uiMbAddressMbAff == uiLastMbAddress ),
                                                   (eP == 1) ) );
    }
  }

  ruiBits += m_pcMbCoder ->getBitCount() - uiBits;
	return Err::m_nOK;
}

// TMM_INTERLACE}


ErrVal SliceEncoder::encodeHighPassPicture( UInt&         ruiMbCoded,
                                            UInt&         ruiBits,
                                            SliceHeader&  rcSH,
                                            Frame*     pcOrgFrame,
                                            Frame*     pcFrame,
                                            Frame*     pcResidual,
                                            Frame*     pcPredSignal,
 																						Frame*     pcBaseSubband,
                                            Frame*     pcBaseLayer,
                                            MbDataCtrl*   pcMbDataCtrl,
                                            MbDataCtrl*   pcMbDataCtrlBaseMotion,
                                            UInt          uiMbInRow,
                                            Double        dLambda,
                                            Int           iMaxDeltaQp,
                                            Int          iSpatialScalabilityType,
																						PicType      ePicType )
{
  ROF( m_bInitDone );

  RNOK( pcMbDataCtrl  ->initSlice         ( rcSH, PRE_PROCESS, false, NULL ) );
  RNOK( m_pcControlMng->initSliceForCoding( rcSH              ) );

  //====== initialization ======
  UInt            uiMbAddress       = rcSH.getFirstMbInSlice();
  UInt            uiLastMbAddress   = rcSH.getLastMbInSlice ();
  UInt            uiBits            = m_pcMbCoder->getBitCount();
  Int             iQPRes            = rcSH.getSliceQp();
  Int             iQPIntra          = rcSH.getSliceQp(); //- 2;

  YuvMbBuffer  cZeroBuffer;
  cZeroBuffer.setAllSamplesToZero();

	if( ePicType!=FRAME )
	{
    if( pcOrgFrame )    RNOK( pcOrgFrame   ->addFieldBuffer( ePicType ) ); //TMM
		if( pcFrame )       RNOK( pcFrame      ->addFieldBuffer( ePicType ) );
		if( pcResidual )    RNOK( pcResidual   ->addFieldBuffer( ePicType ) );
		if( pcBaseSubband ) RNOK( pcBaseSubband->addFieldBuffer( ePicType ) );
		if( pcBaseLayer )   RNOK( pcBaseLayer  ->addFieldBuffer( ePicType ) );
		if( pcPredSignal )  RNOK( pcPredSignal ->addFieldBuffer( ePicType ) );
	}

  //===== loop over macroblocks =====
  for(  ruiMbCoded = 0; uiMbAddress <= uiLastMbAddress;  ) //--ICU/ETRI FMO Implementation
  {
    // JVT-W043 {
    if ( bRateControlEnable && !pcJSVMParams->m_uiLayerId )
    {
      pcGenericRC->m_pcJSVMParams->number = pcGenericRC->m_pcJSVMParams->current_frame_number;      
      pcJSVMParams->CurrGopLevel = pcGenericRC->getCurrGopLevel( pcGenericRC->m_pcJSVMParams->number );
      pcGenericRC->m_pcJSVMParams->nal_reference_idc = (pcJSVMParams->CurrGopLevel == 0) ? 0 : 1;
      pcGenericRC->m_pcJSVMParams->current_mb_nr = uiMbAddress;

      if( pcGenericRC->m_pcJSVMParams->basicunit == pcGenericRC->m_pcJSVMParams->FrameSizeInMbs ) {
        // qp remains unchanged
      }
      // basic unit layer rate control
      else if ( pcGenericRC->m_pcJSVMParams->type == P_SLICE ) {
        if( pcGenericRC->m_pcJSVMParams->NumberofCodedMacroBlocks > 0 
          && (pcGenericRC->m_pcJSVMParams->NumberofCodedMacroBlocks % pcGenericRC->m_pcJSVMParams->BasicUnit) == 0 ) {
            // frame coding
            if( pcGenericRC->m_pcJSVMParams->frame_mbs_only_flag ) {
              pcQuadraticRC->updateRCModel();
              pcGenericRC->m_pcJSVMParams->qp = pcQuadraticRC->updateQPRC2( pcGenericRC->m_iTopFieldFlag );
              iQPRes = pcGenericRC->m_pcJSVMParams->qp;
            }
          }
      }
    }
    // JVT-W043 }
    ETRACE_NEWMB( uiMbAddress );

    MbDataAccess* pcMbDataAccess = NULL;
    Bool          bCoded;
    UInt          uiMbY, uiMbX;

    rcSH.getMbPositionFromAddress           ( uiMbY, uiMbX, uiMbAddress );

    RNOK( pcMbDataCtrl    ->initMb          (  pcMbDataAccess,    uiMbY, uiMbX ) );
    RNOK( m_pcControlMng  ->initMbForCoding ( *pcMbDataAccess,    uiMbY, uiMbX, false, false ) );
    pcMbDataAccess->getMbData().deactivateMotionRefinement();

    MbDataAccess* pcMbDataAccessBase  = 0;

    if( pcMbDataCtrlBaseMotion)
    {
      RNOK( pcMbDataCtrlBaseMotion->initMb  ( pcMbDataAccessBase, uiMbY, uiMbX ) );
    }
    pcMbDataAccess->setMbDataAccessBase( pcMbDataAccessBase );
    pcMbDataAccess->getMbData().setFieldFlag(ePicType!=FRAME);

    if( pcMbDataAccess->getMbData().isIntra() )
    {
      Double dCost = 0;
      //JVT-U106 Behaviour at slice boundaries{
      if( ! rcSH.getNoInterLayerPredFlag() )
		    m_pcMbEncoder->setIntraBLFlag(m_pbIntraBLFlag[uiMbAddress]);
	    //JVT-U106 Behaviour at slice boundaries}

      pcMbDataAccess->getMbData().setQp( iQPIntra );

      RNOK( m_pcMbEncoder ->encodeIntra   ( *pcMbDataAccess,
                                             pcMbDataAccessBase,
                                             pcFrame                  ->getPic( ePicType ),
                                             pcFrame                  ->getPic( ePicType ),
                                             pcResidual               ->getPic( ePicType ),
																						 pcBaseLayer ? pcBaseLayer->getPic( ePicType ) : NULL,
																						 pcPredSignal             ->getPic( ePicType ),
                                             dLambda,
                                             dCost ) );

      RNOK( m_pcMbCoder   ->encode        ( *pcMbDataAccess,
                                             pcMbDataAccessBase,
                                             iSpatialScalabilityType,
                                             (uiMbAddress == uiLastMbAddress ),
                                             true ) );
			ruiMbCoded++;
    }
    else
    {
      pcMbDataAccess->getMbData().setQp( iQPRes );

      m_pcTransform->setClipMode( false );
      RNOK( m_pcMbEncoder ->encodeResidual  ( *pcMbDataAccess,
                                               pcOrgFrame->getPic( ePicType ),
                                               pcFrame   ->getPic( ePicType ),
                                               pcResidual->getPic( ePicType ),
                                               pcBaseSubband ? pcBaseSubband->getPic( ePicType ) : NULL,
                                               bCoded,
                                               dLambda,
                                               iMaxDeltaQp ) );

      if( ! pcMbDataAccess->getSH().getNoInterLayerPredFlag() && ! pcMbDataAccess->getSH().getAdaptiveBaseModeFlag() && pcMbDataAccessBase->getMbData().getInCropWindowFlag() )
      {
        ROF( pcMbDataAccess->getMbData().getResidualPredFlag() );
        pcMbDataAccess->getMbData().setBLSkipFlag( true );
      }

      m_pcTransform->setClipMode( true );

      RNOK( m_pcMbCoder->encode( *pcMbDataAccess, pcMbDataAccessBase, iSpatialScalabilityType, (uiMbAddress == uiLastMbAddress ), true ) );

      if( bCoded )
      {
        ruiMbCoded++;
      }
      // JVT-W043 {
      if ( bRateControlEnable && !pcJSVMParams->m_uiLayerId )
      {
        pcGenericRC->m_iNumberofHeaderBits           += pcGenericRC->m_iRCHeaderBits;
        pcGenericRC->m_iNumberofBasicUnitHeaderBits  += pcGenericRC->m_iRCHeaderBits;
        pcGenericRC->m_iNumberofTextureBits          += pcGenericRC->m_iRCTextureBits;
        pcGenericRC->m_iNumberofBasicUnitTextureBits += pcGenericRC->m_iRCTextureBits;
        pcGenericRC->m_pcJSVMParams->NumberofCodedMacroBlocks++;
      }
      // JVT-W043 }
      // Update the state of the baselayer residual data -- it may be reused in subsequent layers - ASEGALL@SHARPLABS.COM
      if( pcBaseSubband && ( pcMbDataAccess->getMbData().isIntra() || ! pcMbDataAccess->getMbData().getResidualPredFlag() ) )	
      {
        YuvPicBuffer* pcBaseResidual = pcBaseSubband->getPic( ePicType )->getFullPelYuvBuffer();
        pcBaseResidual->clearCurrMb();
      }

	    RNOK( pcPredSignal->getFullPelYuvBuffer()->loadBuffer( &cZeroBuffer ) );
    }

    uiMbAddress = rcSH.getFMO()->getNextMBNr(uiMbAddress);
  }

  if( ePicType!=FRAME )
	{
		if( pcFrame )       RNOK( pcFrame      ->removeFieldBuffer( ePicType ) );
		if( pcResidual )    RNOK( pcResidual   ->removeFieldBuffer( ePicType ) );
		if( pcBaseSubband ) RNOK( pcBaseSubband->removeFieldBuffer( ePicType ) );
		if( pcBaseLayer )   RNOK( pcBaseLayer  ->removeFieldBuffer( ePicType ) );
		if( pcPredSignal )  RNOK( pcPredSignal ->removeFieldBuffer( ePicType ) );
	}

  ruiBits += m_pcMbCoder->getBitCount() - uiBits;

  return Err::m_nOK;
}

// TMM_INTERLACE{
ErrVal SliceEncoder::encodeHighPassPictureMbAff( UInt&				ruiMbCoded,
                                                 UInt&				ruiBits,
                                                 SliceHeader&	rcSH,
                                                 Frame*    pcOrgFrame,
                                                 Frame*    pcFrame,
                                                 Frame*    pcResidual,
                                                 Frame*    pcPredSignal,
     																						 Frame*    pcBaseSubband,
                                                 Frame*    pcBaseLayer,
                                                 MbDataCtrl*	pcMbDataCtrl,
                                                 MbDataCtrl*  pcMbDataCtrlBaseMotion,
                                                 MbDataCtrl*   pcMbDataCtrlInterlBaseMotion,
                                                 UInt					uiMbInRow,
                                                 Double				dLambda,
                                                 Int					iMaxDeltaQp,
                                                 Int					iSpatialScalabilityType )
{
  ROF( m_bInitDone );

  //====== initialization ======
  RNOK( pcMbDataCtrl  ->initSlice         ( rcSH, PRE_PROCESS, false, NULL ) );
  RNOK( m_pcControlMng->initSliceForCoding( rcSH              ) );

  UInt uiBits   = m_pcMbCoder->getBitCount();
  Int  iQPRes   = rcSH.getSliceQp           ();
  Int  iQPIntra = rcSH.getSliceQp           (); //- 2;

  YuvMbBuffer  cZeroBuffer;
  cZeroBuffer.setAllSamplesToZero();

  Frame*	apcFrame      [4] = { NULL, NULL, NULL, NULL };
  Frame*	apcOrgFrame   [4] = { NULL, NULL, NULL, NULL };
  Frame*	apcResidual   [4] = { NULL, NULL, NULL, NULL };
  Frame*	apcPredSignal [4] = { NULL, NULL, NULL, NULL };
  Frame*	apcBaseSubband[4] = { NULL, NULL, NULL, NULL };
  Frame*	apcBaseLayer  [4] = { NULL, NULL, NULL, NULL };

	RNOK( gSetFrameFieldArrays( apcFrame,       pcFrame       ) );
	RNOK( gSetFrameFieldArrays( apcOrgFrame,    pcOrgFrame    ) );
  RNOK( gSetFrameFieldArrays( apcResidual,    pcResidual    ) );
  RNOK( gSetFrameFieldArrays( apcPredSignal,  pcPredSignal  ) );
  RNOK( gSetFrameFieldArrays( apcBaseSubband, pcBaseSubband ) );
  RNOK( gSetFrameFieldArrays( apcBaseLayer,   pcBaseLayer   ) );

  //===== loop over macroblocks =====
  UInt       uiMbAddress     = rcSH.getFirstMbInSlice();
  const UInt uiLastMbAddress = rcSH.getLastMbInSlice  ();
  for( ; uiMbAddress <= uiLastMbAddress; uiMbAddress+=2 )
  {
    for( Int eP = 0; eP < 2; eP++ )
    {
      MbDataAccess* pcMbDataAccess     = NULL;
      MbDataAccess* pcMbDataAccessBase = NULL;
      UInt          uiMbY, uiMbX;
      const UInt    uiMbAddressMbAff   = uiMbAddress+eP;
      Bool          bCoded;

      ETRACE_NEWMB( uiMbAddressMbAff );

      rcSH.getMbPositionFromAddress           ( uiMbY, uiMbX, uiMbAddressMbAff );

      RNOK( pcMbDataCtrl    ->initMb          (  pcMbDataAccess,    uiMbY, uiMbX ) );
      const Bool bField = pcMbDataAccess->getMbData().getFieldFlag();

      RNOK( m_pcControlMng  ->initMbForCoding ( *pcMbDataAccess,    uiMbY, uiMbX, true, bField ) );

      pcMbDataAccess->getMbData().deactivateMotionRefinement();

      if( bField && pcMbDataCtrlInterlBaseMotion)   // field case
      {
        RNOK( pcMbDataCtrlInterlBaseMotion ->      initMb  ( pcMbDataAccessBase, uiMbY, uiMbX ) );
      }
      else if( !bField && pcMbDataCtrlBaseMotion)   // frame case
      {
        RNOK( pcMbDataCtrlBaseMotion       ->      initMb  ( pcMbDataAccessBase, uiMbY, uiMbX ) );
      }

      const PicType eMbPicType = pcMbDataAccess->getMbPicType();
      const UInt    uiLI       = eMbPicType - 1;

      if( pcMbDataAccess->getMbData().isIntra() )
      {
        Double dCost = 0;
        pcMbDataAccess->getMbData().setQp( iQPIntra );
			  pcMbDataAccess->setMbDataAccessBase( pcMbDataAccessBase );

        RNOK( m_pcMbEncoder ->encodeIntra( *pcMbDataAccess,
					                                  pcMbDataAccessBase,
                                            apcFrame     [uiLI],
                                            apcFrame     [uiLI],
                                            apcResidual  [uiLI],
                                            apcBaseLayer [uiLI],
                                            apcPredSignal[uiLI],
																						dLambda,
																						dCost ) );

        RNOK( m_pcMbCoder   ->encode     ( *pcMbDataAccess,
					                                  pcMbDataAccessBase,
																						iSpatialScalabilityType,
																						(uiMbAddressMbAff == uiLastMbAddress ),
																						(eP == 1)  ) );
		
        ruiMbCoded++;
      }
      else
      {
        pcMbDataAccess->getMbData().setQp( iQPRes );
			  pcMbDataAccess->setMbDataAccessBase( pcMbDataAccessBase );

        m_pcTransform->setClipMode( false );
        RNOK( m_pcMbEncoder ->encodeResidual  ( *pcMbDataAccess,
                                                 apcOrgFrame   [uiLI],
                                                 apcFrame      [uiLI],
                                                 apcResidual   [uiLI],
                                                 apcBaseSubband[uiLI],
                                                 bCoded,
                                                 dLambda,
                                                 iMaxDeltaQp ) );

        if( ! pcMbDataAccess->getSH().getNoInterLayerPredFlag() && ! pcMbDataAccess->getSH().getAdaptiveBaseModeFlag() && pcMbDataAccessBase->getMbData().getInCropWindowFlag() )
        {
          ROF( pcMbDataAccess->getMbData().getResidualPredFlag() );
          pcMbDataAccess->getMbData().setBLSkipFlag( true );
        }

        m_pcTransform->setClipMode( true );

        RNOK( m_pcMbCoder->encode( *pcMbDataAccess,
                                    pcMbDataAccessBase,
                                    iSpatialScalabilityType,
                                    (uiMbAddressMbAff == uiLastMbAddress ),
                                    (eP == 1) ) );

        if( bCoded )
        {
          ruiMbCoded++;
        }
       
         RNOK( apcPredSignal[uiLI]->getFullPelYuvBuffer()->loadBuffer( &cZeroBuffer ) );
      }
    }
  }

  ruiBits += m_pcMbCoder->getBitCount() - uiBits;

  return Err::m_nOK;
}
// TMM_INTERLACE}

// TMM_INTERLACE{
ErrVal
SliceEncoder::encodeInterPicturePMbAff( UInt&         ruiBits,
                                        Frame*     pcOrgFrame,
                                        Frame*     pcFrame,
                                        Frame*     pcRecSubband,
                                        Frame*     pcPredSignal,
                                        ControlData&  rcControlData,
                                        UInt          uiMbInRow,
                                        RefFrameList& rcRefFrameList,
                                        RefFrameList& rcRefFrameListBase )
{
  ROF( m_bInitDone );

  SliceHeader&  rcSliceHeader						= *rcControlData.getSliceHeader          ( FRAME );
  MbDataCtrl*   pcMbDataCtrl						= rcControlData.getMbDataCtrl            ();
	Frame*     pcBaseLayerFrame        = rcControlData.getBaseLayerRec          ();
	Frame*     pcBaseLayerResidual     = rcControlData.getBaseLayerSbb          ();
  MbDataCtrl*   pcBaseLayerCtrl					= rcControlData.getBaseLayerCtrl         ();
  MbDataCtrl*   pcBaseLayerCtrlField  =  rcControlData.getBaseLayerCtrlField    ();
  Double        dLambda									= rcControlData.getLambda                ();
  Int           iSpatialScalabilityType = rcControlData.getSpatialScalabilityType(); // TMM_ESS
  UInt          uiBits									= m_pcMbCoder ->getBitCount              ();

  //====== initialization ======
  RNOK( pcMbDataCtrl      ->initSlice         ( rcSliceHeader, ENCODE_PROCESS, false, NULL ) );
  if( pcBaseLayerCtrl )
  {
    RNOK( pcBaseLayerCtrl ->initSlice         ( rcSliceHeader, PRE_PROCESS,    false, NULL ) );
  }
  if( pcBaseLayerCtrlField )
  {
    RNOK( pcBaseLayerCtrlField ->initSlice   ( rcSliceHeader, PRE_PROCESS,    false, NULL ) );
  }
  RNOK( m_pcControlMng    ->initSliceForCoding( rcSliceHeader ) );

  RefFrameList acRefFrameList    [2];
  RefFrameList acRefFrameListBase[2];

  YuvMbBuffer acIntYuvMbBufferPredSignal[2];
  YuvMbBuffer acIntYuvMbBufferRecSubBand[2];
  YuvMbBuffer acIntYuvMbBufferFrame     [2];

  RNOK( gSetFrameFieldLists( acRefFrameList    [0], acRefFrameList    [1], rcRefFrameList     ) );
  RNOK( gSetFrameFieldLists( acRefFrameListBase[0], acRefFrameListBase[1], rcRefFrameListBase ) );

  Frame* apcOrgFrame  [4] = { NULL, NULL, NULL, NULL };
  Frame* apcFrame     [4] = { NULL, NULL, NULL, NULL };
  Frame* apcRecSubband[4] = { NULL, NULL, NULL, NULL };
  Frame* apcPredSignal[4] = { NULL, NULL, NULL, NULL };
  Frame* apcBaseLayerF[4] = { NULL, NULL, NULL, NULL };
  Frame* apcBaseLayerR[4] = { NULL, NULL, NULL, NULL };

	RNOK( gSetFrameFieldArrays( apcFrame,      pcFrame             ) );
  RNOK( gSetFrameFieldArrays( apcOrgFrame,   pcOrgFrame          ) );
  RNOK( gSetFrameFieldArrays( apcRecSubband, pcRecSubband        ) );
  RNOK( gSetFrameFieldArrays( apcPredSignal, pcPredSignal        ) );
  RNOK( gSetFrameFieldArrays( apcBaseLayerF, pcBaseLayerFrame    ) );
  RNOK( gSetFrameFieldArrays( apcBaseLayerR, pcBaseLayerResidual ) );

  MbDataBuffer acMbData[2];
  Bool   abSkipModeAllowed[4] = {true,true,true,true};
  UInt uiLastQp = rcSliceHeader.getSliceQp();

  //===== loop over macroblocks =====
  UInt       uiMbAddress			= rcSliceHeader.getFirstMbInSlice();
  const UInt uiLastMbAddress	= rcSliceHeader.getLastMbInSlice  ();
  for( ; uiMbAddress <= uiLastMbAddress; uiMbAddress+=2 )
  {
    UInt auiLastQpTest[2] = {uiLastQp, uiLastQp};
    Double adCost[2] = {0,0};
	  Int eP;

    for( eP = 0; eP < 4; eP++ )
    {
      RefFrameList* pcRefFrameList;
      RefFrameList* pcRefFrameListBase;
      MbDataAccess* pcMbDataAccess     = NULL;
      MbDataAccess* pcMbDataAccessBase = NULL;
      Double        dCost = 0;
      UInt          uiMbY, uiMbX;

      const Bool    bField             = (eP < 2);
      const UInt    uiMbAddressMbAff   = uiMbAddress+(eP%2);

      if( bField )
      {
        pcRefFrameList     = acRefFrameList     + eP;
        pcRefFrameListBase = acRefFrameListBase + eP;
      }
      else
      {
        pcRefFrameList     = &rcRefFrameList;
        pcRefFrameListBase = &rcRefFrameListBase;
      }

      rcSliceHeader.getMbPositionFromAddress    ( uiMbY, uiMbX, uiMbAddressMbAff );

      RNOK( pcMbDataCtrl      ->initMb          ( pcMbDataAccess,     uiMbY, uiMbX ) );

      if  (eP < 2 && pcBaseLayerCtrlField)  // field case
      {
        RNOK( pcBaseLayerCtrlField         ->initMb( pcMbDataAccessBase, uiMbY, uiMbX ) );
      }
      else if  (eP >= 2 &&  pcBaseLayerCtrl ) //frame case
      {
        RNOK( pcBaseLayerCtrl             ->initMb ( pcMbDataAccessBase, uiMbY, uiMbX ) );
      }

      RNOK( m_pcControlMng  ->initMbForCoding   ( *pcMbDataAccess,    uiMbY, uiMbX, true, bField ) );
      pcMbDataAccess->getMbData().deactivateMotionRefinement();

      pcMbDataAccess->setLastQp( auiLastQpTest[bField] );

      if( 0 == eP )
      {
        abSkipModeAllowed[1] = pcMbDataAccess->getDefaultFieldFlag(); // do not move
        abSkipModeAllowed[3] = ! abSkipModeAllowed[1];
      }

      pcRefFrameListBase = pcRefFrameListBase->getSize()? pcRefFrameListBase : NULL;

      RNOK( m_pcMbEncoder     ->encodeInterP    ( *pcMbDataAccess,
                                                   pcMbDataAccessBase,
                                                   iSpatialScalabilityType,
                                                   apcOrgFrame  [eP],
                                                   apcFrame     [eP],
                                                   apcRecSubband[eP],
                                                   apcPredSignal[eP],
                                                   apcBaseLayerF[eP],
                                                   apcBaseLayerR[eP],
                                                  *pcRefFrameList,
                                                   pcRefFrameListBase,
                                                   dLambda,
                                                   dCost,
                                                   abSkipModeAllowed[eP] ) );
	  
      auiLastQpTest[bField] = pcMbDataAccess->getMbData().getQp();
      adCost [eP>>1] += dCost;

      if( bField )
      {
        acMbData[eP].copy( pcMbDataAccess->getMbData() );
        if( apcPredSignal [eP] ) acIntYuvMbBufferPredSignal[eP].loadBuffer( apcPredSignal [eP]->getFullPelYuvBuffer() );
        if( apcRecSubband [eP] ) acIntYuvMbBufferRecSubBand[eP].loadBuffer( apcRecSubband [eP]->getFullPelYuvBuffer() );
        if( apcFrame      [eP] ) acIntYuvMbBufferFrame     [eP].loadBuffer( apcFrame      [eP]->getFullPelYuvBuffer() );
      }
    }

    const Bool bFieldMode = ( adCost[0] < adCost[1] );
#ifdef RANDOM_MBAFF
    bFieldMode = gBoolRandom();
#endif
    for( eP = 0; eP < 2; eP++ )
    {
      MbDataAccess* pcMbDataAccess     = NULL;
      MbDataAccess* pcMbDataAccessBase = NULL;
      UInt          uiMbY, uiMbX;
      const UInt    uiMbAddressMbAff   = uiMbAddress+eP;

      ETRACE_NEWMB( uiMbAddressMbAff );

      rcSliceHeader.getMbPositionFromAddress    ( uiMbY, uiMbX, uiMbAddressMbAff );

      RNOK( pcMbDataCtrl      ->initMb          ( pcMbDataAccess,     uiMbY, uiMbX ) );

      if( bFieldMode && pcBaseLayerCtrlField )
      {
        RNOK( pcBaseLayerCtrlField ->initMb          ( pcMbDataAccessBase, uiMbY, uiMbX ) );
      }
      else if ( !bFieldMode && pcBaseLayerCtrl )
      {
        RNOK( pcBaseLayerCtrl       ->initMb          ( pcMbDataAccessBase, uiMbY, uiMbX ) );
      }

      RNOK( m_pcControlMng  ->initMbForCoding   ( *pcMbDataAccess,    uiMbY, uiMbX, true, bFieldMode ) );

      pcMbDataAccess->getMbData().deactivateMotionRefinement();

      if( bFieldMode )
      {
        pcMbDataAccess->getMbData().copy( acMbData[eP] );
        if( apcRecSubband [eP] )  apcRecSubband [eP]->getFullPelYuvBuffer()->loadBuffer( &acIntYuvMbBufferRecSubBand[eP] );
        if( apcPredSignal [eP] )  apcPredSignal [eP]->getFullPelYuvBuffer()->loadBuffer( &acIntYuvMbBufferPredSignal[eP] );
        if( apcFrame      [eP] )  apcFrame      [eP]->getFullPelYuvBuffer()->loadBuffer( &acIntYuvMbBufferFrame     [eP] );
      }
      pcMbDataAccess->setLastQp( uiLastQp );
      uiLastQp = pcMbDataAccess->getMbData().getQp();
			pcMbDataAccess->setMbDataAccessBase( pcMbDataAccessBase );

      RNOK( m_pcMbCoder       ->encode         ( *pcMbDataAccess,
                                                  pcMbDataAccessBase,
                                                  iSpatialScalabilityType,
                                                  (uiMbAddressMbAff == uiLastMbAddress), (eP == 1) ) );

    }
  }

  ruiBits += m_pcMbCoder ->getBitCount() - uiBits;
  return Err::m_nOK;
}
// TMM_INTERLACE}

ErrVal
SliceEncoder::encodeSlice( SliceHeader&  rcSliceHeader,
                           Frame*     pcFrame,
                           MbDataCtrl*   pcMbDataCtrl,
                           RefFrameList& rcList0,
                           RefFrameList& rcList1,
                           UInt          uiMbInRow,
                           Double        dlambda )
{
  ROF( pcFrame );
  ROF( pcMbDataCtrl );

  //===== get co-located picture =====
  MbDataCtrl* pcMbDataCtrlL1 = NULL;
  if( rcList1.getActive() && rcList1.getEntry( 0 )->getRecPicBufUnit() )
  {
    pcMbDataCtrlL1 = rcList1.getEntry( 0 )->getRecPicBufUnit()->getMbDataCtrl();
  }
  ROT( rcSliceHeader.isBSlice() && ! pcMbDataCtrlL1 );

  //===== initialization =====
  RNOK( pcMbDataCtrl  ->initSlice         ( rcSliceHeader, ENCODE_PROCESS, false, pcMbDataCtrlL1 ) );
  RNOK( m_pcControlMng->initSliceForCoding( rcSliceHeader ) );



  //===== loop over macroblocks =====
  for( UInt uiMbAddress = rcSliceHeader.getFirstMbInSlice(); uiMbAddress <= rcSliceHeader.getLastMbInSlice(); uiMbAddress = rcSliceHeader.getFMO()->getNextMBNr( uiMbAddress ) )
  {
    ETRACE_NEWMB( uiMbAddress );

    UInt          uiMbY           = uiMbAddress / uiMbInRow;
    UInt          uiMbX           = uiMbAddress % uiMbInRow;
    MbDataAccess* pcMbDataAccess  = 0;

    RNOK( pcMbDataCtrl  ->initMb          (  pcMbDataAccess, uiMbY, uiMbX ) );
    RNOK( m_pcControlMng->initMbForCoding ( *pcMbDataAccess, uiMbY, uiMbX, false, false  ) );
    pcMbDataAccess->setMbDataAccessBase   ( NULL );

    RNOK( m_pcMbEncoder ->encodeMacroblock( *pcMbDataAccess,
                                             pcFrame,
                                             rcList0,
                                             rcList1,
                                             m_pcCodingParameter->getMotionVectorSearchParams().getNumMaxIter(),
                                             m_pcCodingParameter->getMotionVectorSearchParams().getIterSearchRange(),
                                             dlambda ) );
    RNOK( m_pcMbCoder   ->encode          ( *pcMbDataAccess,
                                              NULL,
                                              SST_RATIO_1,
                                             ( uiMbAddress == rcSliceHeader.getLastMbInSlice() )
                                             ,true ) );
  }

  return Err::m_nOK;
}

//TMM_WP
ErrVal SliceEncoder::xInitDefaultWeights(Double *pdWeights, UInt uiLumaWeightDenom,
                                         UInt uiChromaWeightDenom)
{
    const Int iLumaWeight = 1 << uiLumaWeightDenom;
    const Int iChromaWeight = 1 << uiChromaWeightDenom;

    pdWeights[0] = iLumaWeight;
    pdWeights[1] = pdWeights[2] = iChromaWeight;

    return Err::m_nOK;
}


ErrVal SliceEncoder::xSetPredWeights( SliceHeader& rcSH,
                                      Frame* pOrgFrame,
                                      RefFrameList& rcRefFrameList0,
                                      RefFrameList& rcRefFrameList1)

{
  ROTRS( rcSH.isIntraSlice(), Err::m_nOK );

  const SampleWeightingParams& rcSWP = m_pcCodingParameter->getSampleWeightingParams(rcSH.getDependencyId());

  { // determine denoms
    const UInt uiLumaDenom = rcSWP.getLumaDenom();
    rcSH.setLumaLog2WeightDenom  ( ( uiLumaDenom == MSYS_UINT_MAX ) ? gIntRandom(0,7) : uiLumaDenom );

    const UInt uiChromaDenom = rcSWP.getChromaDenom();
    rcSH.setChromaLog2WeightDenom( ( uiChromaDenom == MSYS_UINT_MAX ) ? gIntRandom(0,7) : uiChromaDenom );
  }

  const Int iChromaScale = 1<<rcSH.getChromaLog2WeightDenom();
  const Int iLumaScale   = 1<<rcSH.getLumaLog2WeightDenom();

  m_pcControlMng->initSliceForWeighting(rcSH);

  if( rcSH.isBSlice() )
  {
      ROTRS( 1 != rcSH.getPPS().getWeightedBiPredIdc(), Err::m_nOK );
  }
  else
  {
    ROTRS( ! rcSH.getPPS().getWeightedPredFlag(), Err::m_nOK );
  }

  if( rcSH.isBSlice() )
  {
      RNOK( rcSH.getPredWeightTable(LIST_1).initDefaults( rcSH.getLumaLog2WeightDenom(), rcSH.getChromaLog2WeightDenom() ) );
  }
  RNOK( rcSH.getPredWeightTable(LIST_0).initDefaults( rcSH.getLumaLog2WeightDenom(), rcSH.getChromaLog2WeightDenom() ) );

  Double afFwWeight[MAX_REF_FRAMES][3];
  Double afBwWeight[MAX_REF_FRAMES][3];

  Double afFwOffsets[MAX_REF_FRAMES][3];
  Double afBwOffsets[MAX_REF_FRAMES][3];

  /* init arrays with default weights */
  for (UInt x = 0; x < MAX_REF_FRAMES; x++)
  {
      xInitDefaultWeights(afFwWeight[x], rcSH.getLumaLog2WeightDenom(), rcSH.getChromaLog2WeightDenom());
      xInitDefaultWeights(afBwWeight[x], rcSH.getLumaLog2WeightDenom(), rcSH.getChromaLog2WeightDenom());

      afFwOffsets[x][0] = afFwOffsets[x][1] = afFwOffsets[x][2] = 0;
      afBwOffsets[x][0] = afBwOffsets[x][1] = afBwOffsets[x][2] = 0;
  }

  if( rcSH.isBSlice() )
  {
      RNOK( m_pcMbEncoder->getPredWeights( rcSH, LIST_1, afBwWeight,
                                           pOrgFrame, rcRefFrameList1 ) );
      RNOK( rcSH.getPredWeightTable( LIST_1).setWeights( afBwWeight, iLumaScale, iChromaScale ) );
  }

  RNOK( m_pcMbEncoder->getPredWeights( rcSH, LIST_0, afFwWeight, pOrgFrame, rcRefFrameList0 ) );
  RNOK( rcSH.getPredWeightTable( LIST_0).setWeights( afFwWeight, iLumaScale, iChromaScale ) );

  return Err::m_nOK;
}
//TMM_WP


ErrVal
SliceEncoder::updatePictureResTransform( ControlData&     rcControlData,
                                         UInt          uiMbInRow )
{
  ROF( m_bInitDone );

  SliceHeader&  rcSliceHeader         = *rcControlData.getSliceHeader           ();
  MbDataCtrl*   pcMbDataCtrl          =  rcControlData.getMbDataCtrl            ();
  MbDataCtrl*   pcBaseLayerCtrl       =  rcControlData.getBaseLayerCtrl         ();
  UInt          uiMbAddress           =  0;
  UInt          uiLastMbAddress       =  rcSliceHeader.getMbInPic() - 1;

  //====== initialization ======
  RNOK( pcMbDataCtrl->initSlice( rcSliceHeader, DECODE_PROCESS, false, NULL ) );

  // Update the macroblock state
  // Must be done after the bit-stream has been constructed
  for( ; uiMbAddress <= uiLastMbAddress; )
  {
    UInt          uiMbY               = uiMbAddress / uiMbInRow;
    UInt          uiMbX               = uiMbAddress % uiMbInRow;
    MbDataAccess* pcMbDataAccess      = 0;
    MbDataAccess* pcMbDataAccessBase  = 0;

    RNOK( pcMbDataCtrl      ->initMb          ( pcMbDataAccess,     uiMbY, uiMbX ) );
    if( pcBaseLayerCtrl )
    {
      RNOK( pcBaseLayerCtrl ->initMb          ( pcMbDataAccessBase, uiMbY, uiMbX ) );
      pcMbDataAccess->setMbDataAccessBase( pcMbDataAccessBase );
    }

    // modify QP values (as specified in G.8.1.5.1.2)
    if( pcMbDataAccess->getMbData().getMbCbp() == 0 && ( pcMbDataAccess->getMbData().getMbMode() == INTRA_BL || pcMbDataAccess->getMbData().getResidualPredFlag() ) )
    {
      pcMbDataAccess->getMbData().setQp   ( pcMbDataAccessBase->getMbData().getQp() );
      pcMbDataAccess->getMbData().setQp4LF( pcMbDataAccessBase->getMbData().getQp() );
    }

    // if cbp==0, tranform size is not transmitted, in this case inherit the transform size from base layer
    if( pcMbDataAccess->isSCoeffPred() || ( pcMbDataAccess->getMbData().isIntraBL() && pcMbDataAccessBase->getMbData().isIntraButnotIBL() ) )
    {
      if( !( pcMbDataAccess->getMbData().getMbCbp() & 0x0F ) )
        pcMbDataAccess->getMbData().setTransformSize8x8( pcMbDataAccessBase->getMbData().isTransformSize8x8() );    
    }

    uiMbAddress = rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress );
  }

  return Err::m_nOK;
}

ErrVal
SliceEncoder::updateBaseLayerResidual( ControlData&     rcControlData,
                                         UInt          uiMbInRow )
{
  ROF( m_bInitDone );

  SliceHeader&  rcSliceHeader         = *rcControlData.getSliceHeader           ();
  MbDataCtrl*   pcMbDataCtrl          =  rcControlData.getMbDataCtrl            ();
  MbDataCtrl*   pcBaseLayerCtrl       =  rcControlData.getBaseLayerCtrl         ();
  Frame*     pcBaseLayerSbb        =  rcControlData.getBaseLayerSbb         ();
  UInt          uiMbAddress           =  0;
  UInt          uiLastMbAddress       =  rcSliceHeader.getMbInPic() - 1;
 
  //====== initialization ======
  RNOK( pcMbDataCtrl->initSlice( rcSliceHeader, DECODE_PROCESS, false, NULL ) );

  for( ; uiMbAddress <= uiLastMbAddress; )
  {
    UInt          uiMbY               = uiMbAddress / uiMbInRow;
    UInt          uiMbX               = uiMbAddress % uiMbInRow;
    MbDataAccess* pcMbDataAccess      = 0;
    MbDataAccess* pcMbDataAccessBase  = 0;

    RNOK( pcMbDataCtrl      ->initMb          ( pcMbDataAccess,     uiMbY, uiMbX ) );
    if( pcBaseLayerCtrl )
    {
      RNOK( pcBaseLayerCtrl ->initMb          ( pcMbDataAccessBase, uiMbY, uiMbX ) );
      //pcMbDataAccess->setMbDataAccessBase( pcMbDataAccessBase );
    }

    // Update the state of the baselayer residual data -- it may be reused in subsequent layers - ASEGALL@SHARPLABS.COM
    if( !pcMbDataAccess->getMbData().getResidualPredFlag() )
    {
      if( pcBaseLayerSbb && ( pcMbDataAccess->getMbData().isIntra() || ! pcMbDataAccess->getMbData().getResidualPredFlag() ) )
      {
        YuvPicBuffer* pcBaseResidual = pcBaseLayerSbb->getFullPelYuvBuffer();

        pcBaseResidual->getBufferCtrl().initMb( uiMbY, uiMbX, false);
        pcBaseResidual->clearCurrMb();
      }
    }

    uiMbAddress = rcSliceHeader.getFMO()->getNextMBNr(uiMbAddress );
  }

  return Err::m_nOK;
}

// JVT-V035
ErrVal
SliceEncoder::updatePictureAVCRewrite( ControlData& rcControlData, UInt uiMbInRow )
{
  ROF( m_bInitDone );

  SliceHeader&  rcSliceHeader   = *rcControlData.getSliceHeader();
  FMO&          rcFMO           = *rcSliceHeader.getFMO();
  MbDataCtrl*   pcMbDataCtrl    = rcControlData.getMbDataCtrl();
  MbDataCtrl*   pcBaseLayerCtrl = rcControlData.getBaseLayerCtrl();

  //====== initialization ======
  RNOK( pcMbDataCtrl->initSlice( rcSliceHeader, DECODE_PROCESS, false, NULL ) );

  if( rcSliceHeader.getTCoeffLevelPredictionFlag() == true )
  {
	  // Update the macroblock state
	  // Must be done after the bit-stream has been constructed
    for( Int iSliceGroupId = 0; ! rcFMO.SliceGroupCompletelyCoded( iSliceGroupId ); iSliceGroupId++ )
    {
      UInt  uiFirstMbInSliceGroup = rcFMO.getFirstMBOfSliceGroup( iSliceGroupId );
      UInt  uiLastMbInSliceGroup  = rcFMO.getLastMBInSliceGroup ( iSliceGroupId );

      for( UInt uiMbAddress = uiFirstMbInSliceGroup; uiMbAddress <= uiLastMbInSliceGroup; uiMbAddress = rcFMO.getNextMBNr( uiMbAddress ) )
	    {
		    UInt          uiMbY               = uiMbAddress / uiMbInRow;
		    UInt          uiMbX               = uiMbAddress % uiMbInRow;
		    MbDataAccess* pcMbDataAccess      = 0;
		    MbDataAccess* pcMbDataAccessBase  = 0;

		    RNOK( pcMbDataCtrl->initMb( pcMbDataAccess, uiMbY, uiMbX ) );

		    if( pcBaseLayerCtrl )
		    {
			    RNOK( pcBaseLayerCtrl ->initMb( pcMbDataAccessBase, uiMbY, uiMbX ) );
			    pcMbDataAccess->setMbDataAccessBase( pcMbDataAccessBase );
		    }

        // modify QP values (as specified in G.8.1.5.1.2)
        if( pcMbDataAccess->getMbData().getMbCbp() == 0 && ( pcMbDataAccess->getMbData().getMbMode() == INTRA_BL || pcMbDataAccess->getMbData().getResidualPredFlag() ) )
        {
          pcMbDataAccess->getMbData().setQp( pcMbDataAccessBase->getMbData().getQp() );
        }

		    if( pcMbDataAccess->isTCoeffPred() )
		    {
			    if( pcMbDataAccess->getMbData().getMbMode() == INTRA_BL )
			    {
				    // We're going to use the BL skip flag to correctly decode the intra prediction mode
				    AOT( pcMbDataAccess->getMbData().getBLSkipFlag() == false );

				    // Inherit the mode of the base block
				    pcMbDataAccess->getMbData().setMbMode( pcMbDataAccessBase->getMbData().getMbMode() );

				    // Inherit intra prediction modes
				    for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
					    pcMbDataAccess->getMbData().intraPredMode(cIdx) = pcMbDataAccessBase->getMbData().intraPredMode(cIdx);

				    pcMbDataAccess->getMbData().setChromaPredMode( pcMbDataAccessBase->getMbData().getChromaPredMode() );
			    }

  		    // The 8x8 transform flag is present in the bit-stream unless transform coefficients
			    // are not transmitted at the enhancement layer.  In this case, inherit the base layer
			    // transform type.  This makes intra predition work correctly, etc.
			    if( !( pcMbDataAccess->getMbData().getMbCbp() & 0x0F ) )
				    pcMbDataAccess->getMbData().setTransformSize8x8( pcMbDataAccessBase->getMbData().isTransformSize8x8() );
  	
			    xAddTCoeffs2( *pcMbDataAccess, *pcMbDataAccessBase );
		    }

        if( pcMbDataAccess->getMbAddress() != uiFirstMbInSliceGroup &&
            pcMbDataAccess->getSH().getTCoeffLevelPredictionFlag() &&
           !pcMbDataAccess->getSH().getNoInterLayerPredFlag() &&
           !pcMbDataAccess->getMbData().isIntra16x16() &&
            pcMbDataAccess->getMbData().getMbExtCbp() == 0 )
        {
          pcMbDataAccess->getMbData().setQp4LF( pcMbDataAccess->getLastQp4LF() );
        }
        else
        {
          pcMbDataAccess->getMbData().setQp4LF( pcMbDataAccess->getMbData().getQp() );
        }
	    }
    }
  }

  return Err::m_nOK;
}

ErrVal
SliceEncoder::xAddTCoeffs2( MbDataAccess& rcMbDataAccess, MbDataAccess& rcMbDataAccessBase )
{

	UInt uiBCBP = 0;
	UInt uiCoded = 0;
	Bool bCoded = false;
	Bool bChromaAC = false;
	Bool bChromaDC = false;

	// Add the luma coefficients and track the new BCBP
	if( rcMbDataAccess.getMbData().isTransformSize8x8() )
	{

		for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
		{
			bCoded = false;

			m_pcTransform->addPrediction8x8Blk( rcMbDataAccess.getMbTCoeffs().get8x8( c8x8Idx ),
				rcMbDataAccessBase.getMbTCoeffs().get8x8( c8x8Idx ),
				rcMbDataAccess.getMbData().getQp(),
				rcMbDataAccessBase.getMbData().getQp(), bCoded );

			if( rcMbDataAccess.getMbData().isIntra16x16() )
				AOT(1);

			if( bCoded )
				uiBCBP |= (0x33 << c8x8Idx.b4x4());
		}
	}
	else
	{

		for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
		{
			uiCoded = 0;

			m_pcTransform->addPrediction4x4Blk( rcMbDataAccess.getMbTCoeffs().get( cIdx ),
				rcMbDataAccessBase.getMbTCoeffs().get( cIdx ),
				rcMbDataAccess.getMbData().getQp(),
				rcMbDataAccessBase.getMbData().getQp(), uiCoded );

			if( rcMbDataAccess.getMbData().isIntra16x16() )
			{
				if( *(rcMbDataAccess.getMbTCoeffs().get( cIdx )) )
					uiCoded--;
			}

			if( uiCoded )
				uiBCBP |= 1<<cIdx;
		}

		if( rcMbDataAccess.getMbData().isIntra16x16() )
		{
			uiBCBP = uiBCBP?((1<<16)-1):0;
		}
	}

	// Add the chroma coefficients and update the BCBP
	m_pcTransform->addPredictionChromaBlocks( rcMbDataAccess.getMbTCoeffs().get( CIdx(0) ),
		rcMbDataAccessBase.getMbTCoeffs().get( CIdx(0) ),
		rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.getMbData().getQp() ),
		rcMbDataAccessBase.getSH().getChromaQp( rcMbDataAccessBase.getMbData().getQp() ),
		bChromaDC, bChromaAC );

	m_pcTransform->addPredictionChromaBlocks( rcMbDataAccess.getMbTCoeffs().get( CIdx(4) ),
		rcMbDataAccessBase.getMbTCoeffs().get( CIdx(4) ),
		rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.getMbData().getQp() ),
		rcMbDataAccessBase.getSH().getChromaQp( rcMbDataAccessBase.getMbData().getQp() ),
		bChromaDC, bChromaAC );

	uiBCBP |= (bChromaAC?2:(bChromaDC?1:0))<<16;

	// Update the CBP
	rcMbDataAccess.getMbData().setAndConvertMbExtCbp( uiBCBP );

	// Update the Intra16x16 mode
	if( rcMbDataAccess.getMbData().isIntra16x16() )
	{
		UInt uiMbType = INTRA_4X4 + 1;
		UInt uiPredMode = rcMbDataAccess.getMbData().intraPredMode();
		UInt uiChromaCbp = uiBCBP>>16;
		Bool bACcoded = (uiBCBP && ((1<<16)-1));

		uiMbType += uiPredMode;
        uiMbType += ( bACcoded ) ? 12 : 0;
        uiMbType += uiChromaCbp << 2;

		rcMbDataAccess.getMbData().setMbMode( MbMode(uiMbType) );

		// Sanity checks
		if( rcMbDataAccess.getMbData().intraPredMode() != uiPredMode )
			AOT(1);
		if( rcMbDataAccess.getMbData().getCbpChroma16x16() != uiChromaCbp )
			AOT(1);
		if( rcMbDataAccess.getMbData().isAcCoded() != bACcoded )
			AOT(1);
	}

	return Err::m_nOK;



  UInt uiScaleFactor[6] = {8, 9, 10, 11, 13, 14};

  Quantizer cSrcQuantizer, cDstQuantizer;
  cSrcQuantizer.setQp( rcMbDataAccessBase, false );
  cDstQuantizer.setQp( rcMbDataAccess, false );


  // Process luma blocks
  const QpParameter&  cSrcLQp      = cSrcQuantizer.getLumaQp  ();
  const QpParameter&  cDstLQp      = cDstQuantizer.getLumaQp  ();

  QpParameter cScaleQp;
  cScaleQp.setQp( (cSrcLQp.per()-cDstLQp.per())*6+(cSrcLQp.rem()-cDstLQp.rem()), true );

  for( B4x4Idx cIdx; cIdx.isLegal(); cIdx++ )
  {
	  TCoeff *pcDst = rcMbDataAccess.getMbData().getMbTCoeffs().get( cIdx );
	  TCoeff *pcSrc = rcMbDataAccessBase.getMbData().getMbTCoeffs().get( cIdx );

	  for( UInt n=0; n<16; n++ )
	  {
		  TCoeff cCoeff;

		  cCoeff = pcSrc[n]<<cScaleQp.per();
		  cCoeff *= uiScaleFactor[cScaleQp.rem()];
		  cCoeff += ( cCoeff>0 ) ? 4 : -4;
		  cCoeff /= 8;

		  pcDst[n] += cCoeff;

	  }
  }

  // Process chroma blocks
  const QpParameter&  cSrcCQp      = cSrcQuantizer.getChromaQp();
  const QpParameter&  cDstCQp      = cDstQuantizer.getChromaQp();

  // Set scale factor
  cScaleQp.setQp( (cSrcCQp.per()*6+cSrcCQp.rem()) - (cDstCQp.per()*6+cDstCQp.rem()), true );

  for( CIdx cIdxi; cIdxi.isLegal(); cIdxi++ )
  {
	  TCoeff *pcDst = rcMbDataAccess.getMbData().getMbTCoeffs().get( cIdxi );
	  TCoeff *pcSrc = rcMbDataAccessBase.getMbData().getMbTCoeffs().get( cIdxi );

	  for( UInt n=0; n<16; n++ )
	  {
		  TCoeff cCoeff;

		  cCoeff = pcSrc[n]<<cScaleQp.per();
		  cCoeff *= uiScaleFactor[cScaleQp.rem()];
		  cCoeff += ( cCoeff>0 ) ? 4 : -4;
		  cCoeff /= 8;

		  pcDst[n] += cCoeff;

	  }
  }

  return Err::m_nOK;

}

H264AVC_NAMESPACE_END
