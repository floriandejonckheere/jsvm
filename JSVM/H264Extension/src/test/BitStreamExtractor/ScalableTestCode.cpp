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
This software module was originally created for Nokia, Inc.
Author: Liu Hui (liuhui@mail.ustc.edu.cn)

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


#include "ScalableTestCode.h"

ScalableTestCode::ScalableTestCode()
{
}
ScalableTestCode::~ScalableTestCode()
{
}

ErrVal
ScalableTestCode::Destroy()
{
	delete this;
	return Err::m_nOK;
}
ErrVal
ScalableTestCode::WriteUVLC( UInt uiValue )
{
	UInt uiLength = 1;
	UInt uiTemp = ++uiValue;
	while( uiTemp != 1 )
	{
		uiTemp >>= 1;
		uiLength += 2;
	}
	RNOK( WriteCode( uiValue, uiLength ) );
	return Err::m_nOK;
}

ErrVal
ScalableTestCode::SEICode( h264::SEI::ScalableSei* pcScalableSei, ScalableTestCode *pcScalableTestCode )
{
	UInt uiNumScalableLayersMinus1 = pcScalableSei->getNumLayersMinus1();
	pcScalableTestCode->WriteUVLC( uiNumScalableLayersMinus1 );
	for( UInt uiLayer = 0; uiLayer <= uiNumScalableLayersMinus1; uiLayer++ )
	{
		pcScalableTestCode->WriteCode( pcScalableSei->getLayerId( uiLayer ), 8 );
//JVT-S036 lsj start
//		pcScalableTestCode->WriteFlag( pcScalableSei->getFGSLayerFlag( uiLayer ) ); 
	
		pcScalableTestCode->WriteCode( pcScalableSei->getSimplePriorityId( uiLayer ), 6 ); 
		pcScalableTestCode->WriteFlag( pcScalableSei->getDiscardableFlag( uiLayer ) ); 
		pcScalableTestCode->WriteCode( pcScalableSei->getTemporalLevel( uiLayer ), 3 );
		pcScalableTestCode->WriteCode( pcScalableSei->getDependencyId( uiLayer ), 3 );
		pcScalableTestCode->WriteCode( pcScalableSei->getQualityLevel( uiLayer ), 2 );

		pcScalableTestCode->WriteFlag( pcScalableSei->getSubPicLayerFlag( uiLayer ) );
		pcScalableTestCode->WriteFlag( pcScalableSei->getSubRegionLayerFlag( uiLayer ) );
		pcScalableTestCode->WriteFlag( pcScalableSei->getIroiSliceDivisionInfoPresentFlag ( uiLayer ) ); 
		pcScalableTestCode->WriteFlag( pcScalableSei->getProfileLevelInfoPresentFlag( uiLayer ) );
//JVT-S036 lsj end
		pcScalableTestCode->WriteFlag( pcScalableSei->getBitrateInfoPresentFlag( uiLayer ) );
		pcScalableTestCode->WriteFlag( pcScalableSei->getFrmRateInfoPresentFlag( uiLayer ) );
		pcScalableTestCode->WriteFlag( pcScalableSei->getFrmSizeInfoPresentFlag( uiLayer ) );
		pcScalableTestCode->WriteFlag( pcScalableSei->getLayerDependencyInfoPresentFlag( uiLayer ) );
		pcScalableTestCode->WriteFlag( pcScalableSei->getInitParameterSetsInfoPresentFlag( uiLayer ) );
		pcScalableTestCode->WriteFlag( pcScalableSei->getExactInterlayerPredFlag( uiLayer ) ); //JVT-S036 lsj

		if( pcScalableSei->getProfileLevelInfoPresentFlag( uiLayer ) )
		{
			pcScalableTestCode->WriteCode( pcScalableSei->getLayerProfileIdc( uiLayer ), 8 );
			pcScalableTestCode->WriteFlag( pcScalableSei->getLayerConstraintSet0Flag( uiLayer ) );
			pcScalableTestCode->WriteFlag( pcScalableSei->getLayerConstraintSet1Flag( uiLayer ) );
			pcScalableTestCode->WriteFlag( pcScalableSei->getLayerConstraintSet2Flag( uiLayer ) );
			pcScalableTestCode->WriteFlag( pcScalableSei->getLayerConstraintSet3Flag( uiLayer ) );
			pcScalableTestCode->WriteCode( 0, 4 );
			pcScalableTestCode->WriteCode( pcScalableSei->getLayerLevelIdc( uiLayer ), 8 );
		}
		else
		{//JVT-S036 lsj
			pcScalableTestCode->WriteUVLC( pcScalableSei->getProfileLevelInfoSrcLayerIdDelta( uiLayer ) );
		}

/*		if( pcScalableSei->getDecodingDependencyInfoPresentFlag( uiLayer ) )
		{
			pcScalableTestCode->WriteCode( pcScalableSei->getSimplePriorityId( uiLayer ), 6 ); 
			pcScalableTestCode->WriteFlag( pcScalableSei->getDiscardableFlag( uiLayer ) ); 
			pcScalableTestCode->WriteCode( pcScalableSei->getTemporalLevel( uiLayer ), 3 );
			pcScalableTestCode->WriteCode( pcScalableSei->getDependencyId( uiLayer ), 3 );
			pcScalableTestCode->WriteCode( pcScalableSei->getQualityLevel( uiLayer ), 2 );
		}
JVT-S036 lsj*/
		if( pcScalableSei->getBitrateInfoPresentFlag( uiLayer ) )
		{
			pcScalableTestCode->WriteCode( pcScalableSei->getAvgBitrate( uiLayer ), 16 );
		//JVT-S036 lsj start
			pcScalableTestCode->WriteCode( pcScalableSei->getMaxBitrateLayer( uiLayer ), 16 );
			pcScalableTestCode->WriteCode( pcScalableSei->getMaxBitrateDecodedPicture( uiLayer ), 16 );
			pcScalableTestCode->WriteCode( pcScalableSei->getMaxBitrateCalcWindow( uiLayer ), 16 );
		//JVT-S036 lsj end
		}

		if( pcScalableSei->getFrmRateInfoPresentFlag( uiLayer ) )
		{
			pcScalableTestCode->WriteCode( pcScalableSei->getConstantFrmRateIdc( uiLayer ), 2 );
			pcScalableTestCode->WriteCode( pcScalableSei->getAvgFrmRate( uiLayer ), 16 );
		}
		else
		{//JVT-S036 lsj 
			pcScalableTestCode->WriteUVLC( pcScalableSei->getFrmRateInfoSrcLayerIdDelta( uiLayer ) );
		}

		if( pcScalableSei->getFrmSizeInfoPresentFlag( uiLayer ) )
		{
			pcScalableTestCode->WriteUVLC( pcScalableSei->getFrmWidthInMbsMinus1( uiLayer ) );
			pcScalableTestCode->WriteUVLC( pcScalableSei->getFrmHeightInMbsMinus1( uiLayer ) );
		}
		else
		{//JVT-S036 lsj
			pcScalableTestCode->WriteUVLC( pcScalableSei->getFrmSizeInfoSrcLayerIdDelta( uiLayer ) );
		}

		if( pcScalableSei->getSubRegionLayerFlag( uiLayer ) )
		{
			pcScalableTestCode->WriteCode( pcScalableSei->getBaseRegionLayerId( uiLayer ), 8 );
			pcScalableTestCode->WriteFlag( pcScalableSei->getDynamicRectFlag( uiLayer ) );
			if( pcScalableSei->getDynamicRectFlag( uiLayer ) )
			{
				pcScalableTestCode->WriteCode( pcScalableSei->getHorizontalOffset( uiLayer ), 16 );
				pcScalableTestCode->WriteCode( pcScalableSei->getVerticalOffset( uiLayer ), 16 );
				pcScalableTestCode->WriteCode( pcScalableSei->getRegionWidth( uiLayer ), 16 );
				pcScalableTestCode->WriteCode( pcScalableSei->getRegionHeight( uiLayer ), 16 );
			}
		}
		else
		{//JVT-S036 lsj
			pcScalableTestCode->WriteUVLC( pcScalableSei->getSubRegionInfoSrcLayerIdDelta( uiLayer ) );
		}

	//JVT-S036 lsj start
		if( pcScalableSei->getSubPicLayerFlag( uiLayer ) )
		{
			pcScalableTestCode->WriteCode( pcScalableSei->getRoiId( uiLayer ), 3 );
		}
		if( pcScalableSei->getIroiSliceDivisionInfoPresentFlag( uiLayer ) )
		{
			pcScalableTestCode->WriteCode( pcScalableSei->getIroiSliceDivisionType( uiLayer ) , 2 );
			if( pcScalableSei->getIroiSliceDivisionType(uiLayer) == 0 )
			{
				pcScalableTestCode->WriteUVLC( pcScalableSei->getGridSliceWidthInMbsMinus1( uiLayer ) );
				pcScalableTestCode->WriteUVLC( pcScalableSei->getGridSliceHeightInMbsMinus1( uiLayer ) );
			}
			else if( pcScalableSei->getIroiSliceDivisionType(uiLayer) == 1 )
			{
				pcScalableTestCode->WriteUVLC( pcScalableSei->getNumSliceMinus1( uiLayer ) );
				for (UInt nslice = 0; nslice <= pcScalableSei->getNumSliceMinus1( uiLayer ) ; nslice ++ )
				{
					pcScalableTestCode->WriteUVLC( pcScalableSei->getFirstMbInSlice( uiLayer, nslice ) );
					pcScalableTestCode->WriteUVLC( pcScalableSei->getSliceWidthInMbsMinus1( uiLayer, nslice ) );
					pcScalableTestCode->WriteUVLC( pcScalableSei->getSliceHeightInMbsMinus1( uiLayer, nslice ) ); 
				}
			}
			else if( pcScalableSei->getIroiSliceDivisionType(uiLayer) == 2 )
			{
				pcScalableTestCode->WriteUVLC( pcScalableSei->getNumSliceMinus1( uiLayer ) );
				UInt uiFrameHeightInMb = pcScalableSei->getFrmHeightInMbsMinus1( uiLayer ) + 1;
				UInt uiFrameWidthInMb  = pcScalableSei->getFrmWidthInMbsMinus1( uiLayer ) + 1;
				UInt uiPicSizeInMbs = uiFrameHeightInMb * uiFrameWidthInMb;
				for( UInt j = 0; j < uiPicSizeInMbs; j++ )
				{
					pcScalableTestCode->WriteUVLC( pcScalableSei->getSliceId( uiLayer, j ) );
				}
			}
		}
	//JVT-S036 lsj end

		if( pcScalableSei->getLayerDependencyInfoPresentFlag( uiLayer ) )
		{
			pcScalableTestCode->WriteUVLC( pcScalableSei->getNumDirectlyDependentLayers( uiLayer ) );
			for( UInt ui = 0; ui < pcScalableSei->getNumDirectlyDependentLayers( uiLayer ); ui++ )
			{
#if 1 //BUG_FIX liuhui 0603
				pcScalableTestCode->WriteUVLC( pcScalableSei->getNumDirectlyDependentLayerIdDeltaMinus1(uiLayer, ui ) ); //JVT-S036 lsj
#endif
				//
			}
		}
		else
		{//JVT-S036 lsj
			pcScalableTestCode->WriteUVLC( pcScalableSei->getLayerDependencyInfoSrcLayerIdDelta( uiLayer ) );
		}

		if( pcScalableSei->getInitParameterSetsInfoPresentFlag( uiLayer ) )
		{
			pcScalableTestCode->WriteUVLC( pcScalableSei->getNumInitSPSMinus1( uiLayer ) );
      UInt ui;
			for( ui = 0; ui <= pcScalableSei->getNumInitSPSMinus1( uiLayer ); ui++ )
			{
#if 1 //BUG_FIX liuhui 0603
				pcScalableTestCode->WriteUVLC( pcScalableSei->getInitSPSIdDelta( uiLayer, ui ) );
#endif
				//
			}
			pcScalableTestCode->WriteUVLC( pcScalableSei->getNumInitPPSMinus1( uiLayer ) );
			for( ui = 0; ui <= pcScalableSei->getNumInitPPSMinus1( uiLayer ); ui++ )
			{
#if 1 //BUG_FIX liuhui 0603
				pcScalableTestCode->WriteUVLC( pcScalableSei->getInitPPSIdDelta( uiLayer, ui ) );
#endif
				//
			}
		}
		else
		{//JVT-S036 lsj
			pcScalableTestCode->WriteUVLC( pcScalableSei->getInitParameterSetsInfoSrcLayerIdDelta( uiLayer ) );
		}

	}// for

	return Err::m_nOK;
}