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

#include "ScalableModifyCode.h"

ScalableModifyCode::ScalableModifyCode() :
  m_pcBinData( NULL ),
  m_pulStreamPacket( NULL ),
  m_uiBitCounter( 0 ),
  m_uiPosCounter( 0 ),
  m_uiCoeffCost ( 0 ),
  m_uiDWordsLeft    ( 0 ),
  m_uiBitsWritten   ( 0 ),
  m_iValidBits      ( 0 ),
  m_ulCurrentBits   ( 0 )
{

}

ScalableModifyCode::~ScalableModifyCode()
{
}


ErrVal
ScalableModifyCode::Destroy()
{
  delete this;
  return Err::m_nOK;
}

ErrVal
ScalableModifyCode::init( ULong* pulStream )
{
  ROT( pulStream == NULL );
  m_pulStreamPacket = pulStream;

  m_uiDWordsLeft = 0x400/4;
  m_iValidBits = 32;
  return Err::m_nOK;
}

ErrVal
ScalableModifyCode::WriteUVLC( UInt uiValue )
{
  UInt uiLength = 1;
  UInt uiTemp = ++uiValue;

  while( uiTemp != 1 )
  {
    uiTemp >>= 1;
    uiLength += 2;
  }

  RNOK( Write( uiValue, uiLength ) );
  return Err::m_nOK;
}

ErrVal
ScalableModifyCode::WriteCode( UInt uiValue, UInt uiLength )
{
  RNOK( Write( uiValue, uiLength ) );
  return Err::m_nOK;
}

ErrVal
ScalableModifyCode::WriteFlag( Bool bFlag )
{
  RNOK( Write( bFlag? 1 : 0 , 1) );
  return Err::m_nOK;
}

ErrVal
ScalableModifyCode::Write( UInt uiBits, UInt uiNumberOfBits )
{
  m_uiBitsWritten += uiNumberOfBits;

  if( (Int)uiNumberOfBits < m_iValidBits)  // one word
  {
    m_iValidBits -= uiNumberOfBits;

    m_ulCurrentBits |= uiBits << m_iValidBits;

    return Err::m_nOK;
  }


  ROT( 0 == m_uiDWordsLeft );
  m_uiDWordsLeft--;

  UInt uiShift = uiNumberOfBits - m_iValidBits;

  // add the last bits
  m_ulCurrentBits |= uiBits >> uiShift;

  *m_pulStreamPacket++ = xSwap( m_ulCurrentBits );


  // note: there is a problem with left shift with 32
  m_iValidBits = 32 - uiShift;

  m_ulCurrentBits = uiBits << m_iValidBits;

  if( 0 == uiShift )
  {
    m_ulCurrentBits = 0;
  }

  return Err::m_nOK;
}
ErrVal
ScalableModifyCode::WritePayloadHeader( enum h264::SEI::MessageType eType, UInt uiSize )
{
  //type
  {
    UInt uiTemp = eType;
    UInt uiByte = 0xFF;
    while( 0xFF == uiByte )
    {
      uiByte  = (0xFF > uiTemp) ? uiTemp : 0xff;
      uiTemp -= 0xFF;
      RNOK( WriteCode( uiByte, 8 ) );
    }
  }

  // size
  {
    UInt uiTemp = uiSize;
    UInt uiByte = 0xFF;

    while( 0xFF == uiByte )
    {
      uiByte  = (0xFF > uiTemp) ? uiTemp : 0xff;
      uiTemp -= 0xFF;
      RNOK( WriteCode( uiByte, 8 ) );
    }
  }
  return Err::m_nOK;
}

ErrVal
ScalableModifyCode::WriteAlignZero()
{
  return Write( 0, m_iValidBits & 0x7 );
}

ErrVal
ScalableModifyCode::WriteTrailingBits()
{
  RNOK( WriteFlag( 1 ) );
  RNOK( WriteAlignZero() );
  return Err::m_nOK;
}

ErrVal
ScalableModifyCode::flushBuffer()
{
  *m_pulStreamPacket = xSwap( m_ulCurrentBits );

  m_uiBitsWritten = (m_uiBitsWritten+7)/8;

  m_uiBitsWritten *= 8;

  return Err::m_nOK;
}

ErrVal
ScalableModifyCode::ConvertRBSPToPayload( UChar* m_pucBuffer,
                                         UChar pulStreamPacket[],
                                      UInt& ruiBytesWritten,
                                      UInt  uiHeaderBytes )
{
  UInt uiZeroCount    = 0;
  UInt uiReadOffset   = uiHeaderBytes;
  UInt uiWriteOffset  = uiHeaderBytes;

  //===== NAL unit header =====
  for( UInt uiIndex = 0; uiIndex < uiHeaderBytes; uiIndex++ )
  {
    m_pucBuffer[uiIndex] = (UChar)pulStreamPacket[uiIndex];
  }

  //===== NAL unit payload =====
  for( ; uiReadOffset < ruiBytesWritten ; uiReadOffset++, uiWriteOffset++ )
  {
    if( 2 == uiZeroCount && 0 == ( pulStreamPacket[uiReadOffset] & 0xfc ) )
    {
      uiZeroCount                   = 0;
      m_pucBuffer[uiWriteOffset++]  = 0x03;
    }

    m_pucBuffer[uiWriteOffset] = (UChar)pulStreamPacket[uiReadOffset];

    if( 0 == pulStreamPacket[uiReadOffset] )
    {
      uiZeroCount++;
    }
    else
    {
      uiZeroCount = 0;
    }
  }
  if( ( 0x00 == m_pucBuffer[uiWriteOffset-1] ) && ( 0x00 == m_pucBuffer[uiWriteOffset-2] ) )
  {
    m_pucBuffer[uiWriteOffset++] = 0x03;
  }
  ruiBytesWritten = uiWriteOffset;

  return Err::m_nOK;
}




ErrVal
ScalableModifyCode::SEICode( h264::SEI::ScalableSei* pcScalableSei, ScalableModifyCode *pcScalableModifyCode )
{
  UInt uiNumScalableLayersMinus1 = pcScalableSei->getNumLayersMinus1();
  pcScalableModifyCode->WriteUVLC( uiNumScalableLayersMinus1 );
  for( UInt uiLayer = 0; uiLayer <= uiNumScalableLayersMinus1; uiLayer++ )
  {
    pcScalableModifyCode->WriteCode( pcScalableSei->getLayerId( uiLayer ), 8 );
//JVT-S036 lsj start
//    pcScalableModifyCode->WriteFlag( pcScalableSei->getFGSLayerFlag( uiLayer ) );

    pcScalableModifyCode->WriteCode( pcScalableSei->getSimplePriorityId( uiLayer ), 6 );
    pcScalableModifyCode->WriteFlag( pcScalableSei->getDiscardableFlag( uiLayer ) );
    pcScalableModifyCode->WriteCode( pcScalableSei->getTemporalLevel( uiLayer ), 3 );
    pcScalableModifyCode->WriteCode( pcScalableSei->getDependencyId( uiLayer ), 3 );
    pcScalableModifyCode->WriteCode( pcScalableSei->getQualityLevel( uiLayer ), 2 );
    pcScalableModifyCode->WriteFlag( pcScalableSei->getSubPicLayerFlag( uiLayer ) );
    pcScalableModifyCode->WriteFlag( pcScalableSei->getSubRegionLayerFlag( uiLayer ) );
    pcScalableModifyCode->WriteFlag( pcScalableSei->getIroiSliceDivisionInfoPresentFlag ( uiLayer ) );
    pcScalableModifyCode->WriteFlag( pcScalableSei->getProfileLevelInfoPresentFlag( uiLayer ) );
//JVT-S036 lsj end
    pcScalableModifyCode->WriteFlag( pcScalableSei->getBitrateInfoPresentFlag( uiLayer ) );
    pcScalableModifyCode->WriteFlag( pcScalableSei->getFrmRateInfoPresentFlag( uiLayer ) );
    pcScalableModifyCode->WriteFlag( pcScalableSei->getFrmSizeInfoPresentFlag( uiLayer ) );
    pcScalableModifyCode->WriteFlag( pcScalableSei->getLayerDependencyInfoPresentFlag( uiLayer ) );
    pcScalableModifyCode->WriteFlag( pcScalableSei->getInitParameterSetsInfoPresentFlag( uiLayer ) );
    pcScalableModifyCode->WriteFlag( pcScalableSei->getExactInterlayerPredFlag( uiLayer ) ); //JVT-S036 lsj

    if( pcScalableSei->getProfileLevelInfoPresentFlag( uiLayer ) )
    {
      pcScalableModifyCode->WriteCode( pcScalableSei->getLayerProfileIdc( uiLayer ), 8 );
      pcScalableModifyCode->WriteFlag( pcScalableSei->getLayerConstraintSet0Flag( uiLayer ) );
      pcScalableModifyCode->WriteFlag( pcScalableSei->getLayerConstraintSet1Flag( uiLayer ) );
      pcScalableModifyCode->WriteFlag( pcScalableSei->getLayerConstraintSet2Flag( uiLayer ) );
      pcScalableModifyCode->WriteFlag( pcScalableSei->getLayerConstraintSet3Flag( uiLayer ) );
      pcScalableModifyCode->WriteCode( 0, 4 );
      pcScalableModifyCode->WriteCode( pcScalableSei->getLayerLevelIdc( uiLayer ), 8 );
    }
    else
    {//JVT-S036 lsj
      pcScalableModifyCode->WriteUVLC( pcScalableSei->getProfileLevelInfoSrcLayerIdDelta( uiLayer ) );
    }

  /*  if( pcScalableSei->getDecodingDependencyInfoPresentFlag( uiLayer ) )
    {

      pcScalableModifyCode->WriteCode( pcScalableSei->getSimplePriorityId( uiLayer ), 6 );
      pcScalableModifyCode->WriteFlag( pcScalableSei->getDiscardableFlag( uiLayer ) );

      pcScalableModifyCode->WriteCode( pcScalableSei->getTemporalLevel( uiLayer ), 3 );
      pcScalableModifyCode->WriteCode( pcScalableSei->getDependencyId( uiLayer ), 3 );
      pcScalableModifyCode->WriteCode( pcScalableSei->getQualityLevel( uiLayer ), 2 );
    }
JVT-S036 lsj*/
    if( pcScalableSei->getBitrateInfoPresentFlag( uiLayer ) )
    {
      pcScalableModifyCode->WriteCode( pcScalableSei->getAvgBitrate( uiLayer ), 16 );
    //JVT-S036 lsj start
      pcScalableModifyCode->WriteCode( pcScalableSei->getMaxBitrateLayer( uiLayer ), 16 );
      pcScalableModifyCode->WriteCode( pcScalableSei->getMaxBitrateDecodedPicture( uiLayer ), 16 );
      pcScalableModifyCode->WriteCode( pcScalableSei->getMaxBitrateCalcWindow( uiLayer ), 16 );
    //JVT-S036 lsj end
    }

    if( pcScalableSei->getFrmRateInfoPresentFlag( uiLayer ) )
    {
      pcScalableModifyCode->WriteCode( pcScalableSei->getConstantFrmRateIdc( uiLayer ), 2 );
      pcScalableModifyCode->WriteCode( pcScalableSei->getAvgFrmRate( uiLayer ), 16 );
    }
    else
    {//JVT-S036 lsj
      pcScalableModifyCode->WriteUVLC( pcScalableSei->getFrmRateInfoSrcLayerIdDelta( uiLayer ) );
    }

    if( pcScalableSei->getFrmSizeInfoPresentFlag( uiLayer ) )
    {
      pcScalableModifyCode->WriteUVLC( pcScalableSei->getFrmWidthInMbsMinus1( uiLayer ) );
      pcScalableModifyCode->WriteUVLC( pcScalableSei->getFrmHeightInMbsMinus1( uiLayer ) );
    }
    else
    {//JVT-S036 lsj
      pcScalableModifyCode->WriteUVLC( pcScalableSei->getFrmSizeInfoSrcLayerIdDelta( uiLayer ) );
    }

    if( pcScalableSei->getSubRegionLayerFlag( uiLayer ) )
    {
      pcScalableModifyCode->WriteCode( pcScalableSei->getBaseRegionLayerId( uiLayer ), 8 );
      pcScalableModifyCode->WriteFlag( pcScalableSei->getDynamicRectFlag( uiLayer ) );
      if( pcScalableSei->getDynamicRectFlag( uiLayer ) )
      {
        pcScalableModifyCode->WriteCode( pcScalableSei->getHorizontalOffset( uiLayer ), 16 );
        pcScalableModifyCode->WriteCode( pcScalableSei->getVerticalOffset( uiLayer ), 16 );
        pcScalableModifyCode->WriteCode( pcScalableSei->getRegionWidth( uiLayer ), 16 );
        pcScalableModifyCode->WriteCode( pcScalableSei->getRegionHeight( uiLayer ), 16 );
      }
    }
    else
    {//JVT-S036 lsj
      pcScalableModifyCode->WriteUVLC( pcScalableSei->getSubRegionInfoSrcLayerIdDelta( uiLayer ) );
    }

  //JVT-S036 lsj start
    if( pcScalableSei->getSubPicLayerFlag( uiLayer ) )
    {
      pcScalableModifyCode->WriteCode( pcScalableSei->getRoiId( uiLayer ), 3 );
    }
    if( pcScalableSei->getIroiSliceDivisionInfoPresentFlag( uiLayer ) )
    {
      pcScalableModifyCode->WriteCode( pcScalableSei->getIroiSliceDivisionType( uiLayer ) , 2 );
      if( pcScalableSei->getIroiSliceDivisionType(uiLayer) == 0 )
      {
        pcScalableModifyCode->WriteUVLC( pcScalableSei->getGridSliceWidthInMbsMinus1( uiLayer ) );
        pcScalableModifyCode->WriteUVLC( pcScalableSei->getGridSliceHeightInMbsMinus1( uiLayer ) );
      }
      else if( pcScalableSei->getIroiSliceDivisionType(uiLayer) == 1 )
      {
        pcScalableModifyCode->WriteUVLC( pcScalableSei->getNumSliceMinus1( uiLayer ) );
        for (UInt nslice = 0; nslice <= pcScalableSei->getNumSliceMinus1( uiLayer ) ; nslice ++ )
        {
          pcScalableModifyCode->WriteUVLC( pcScalableSei->getFirstMbInSlice( uiLayer, nslice ) );
          pcScalableModifyCode->WriteUVLC( pcScalableSei->getSliceWidthInMbsMinus1( uiLayer, nslice ) );
          pcScalableModifyCode->WriteUVLC( pcScalableSei->getSliceHeightInMbsMinus1( uiLayer, nslice ) );
        }
      }
      else if( pcScalableSei->getIroiSliceDivisionType(uiLayer) == 2 )
      {
        // JVT-S054 (REPLACE) ->
        /*
        pcScalableModifyCode->WriteUVLC( pcScalableSei->getNumSliceMinus1( uiLayer ) );
        UInt uiFrameHeightInMb = pcScalableSei->getFrmHeightInMbsMinus1( uiLayer ) + 1;
        UInt uiFrameWidthInMb  = pcScalableSei->getFrmWidthInMbsMinus1( uiLayer ) + 1;
        UInt uiPicSizeInMbs = uiFrameHeightInMb * uiFrameWidthInMb;
        for( UInt j = 0; j < uiPicSizeInMbs; j++ )
        {
          pcScalableModifyCode->WriteUVLC( pcScalableSei->getSliceId( uiLayer, j ) );
        }
        */
        pcScalableModifyCode->WriteUVLC( pcScalableSei->getNumSliceMinus1( uiLayer ) );
        UInt uiFrameHeightInMb = pcScalableSei->getFrmHeightInMbsMinus1( uiLayer ) + 1;
        UInt uiFrameWidthInMb  = pcScalableSei->getFrmWidthInMbsMinus1( uiLayer ) + 1;
        UInt uiPicSizeInMbs = uiFrameHeightInMb * uiFrameWidthInMb;
        UInt uiWriteBits = ( UInt) ceil( log((Double)(pcScalableSei->getNumSliceMinus1( uiLayer ) + 1) ) / log(2.) );
        if (uiWriteBits == 0)
          uiWriteBits = 1;
        for( UInt j = 0; j < uiPicSizeInMbs; j++ )
        {
          pcScalableModifyCode->WriteCode( pcScalableSei->getSliceId( uiLayer, j ), uiWriteBits );
        }
        // JVT-S054 (REPLACE) <-
      }
    }
  //JVT-S036 lsj end

    if( pcScalableSei->getLayerDependencyInfoPresentFlag( uiLayer ) )
    {
      pcScalableModifyCode->WriteUVLC( pcScalableSei->getNumDirectlyDependentLayers( uiLayer ) );
      for( UInt ui = 0; ui < pcScalableSei->getNumDirectlyDependentLayers( uiLayer ); ui++ )
      {
       //BUG_FIX liuhui 0603
        pcScalableModifyCode->WriteUVLC( pcScalableSei->getNumDirectlyDependentLayerIdDeltaMinus1(uiLayer, ui ) ); //JVT-S036 lsj
        //
      }
    }
    else
    {//JVT-S036 lsj
      pcScalableModifyCode->WriteUVLC( pcScalableSei->getLayerDependencyInfoSrcLayerIdDelta( uiLayer ) );
    }

    if( pcScalableSei->getInitParameterSetsInfoPresentFlag( uiLayer ) )
    {
      pcScalableModifyCode->WriteUVLC( pcScalableSei->getNumInitSPSMinus1( uiLayer ) );
      UInt ui;
      for( ui = 0; ui <= pcScalableSei->getNumInitSPSMinus1( uiLayer ); ui++ )
      {
     //BUG_FIX liuhui 0603
        pcScalableModifyCode->WriteUVLC( pcScalableSei->getInitSPSIdDelta( uiLayer, ui ) );
      //
      }
      pcScalableModifyCode->WriteUVLC( pcScalableSei->getNumInitPPSMinus1( uiLayer ) );
      for( ui = 0; ui <= pcScalableSei->getNumInitPPSMinus1( uiLayer ); ui++ )
      {
       //BUG_FIX liuhui 0603
        pcScalableModifyCode->WriteUVLC( pcScalableSei->getInitPPSIdDelta( uiLayer, ui ) );
        //
      }
    }
    else
    {//JVT-S036 lsj
      pcScalableModifyCode->WriteUVLC( pcScalableSei->getInitParameterSetsInfoSrcLayerIdDelta( uiLayer ) );
    }

  }// for

  return Err::m_nOK;
}
//ErrVal
//ScalableModifyCode::Create( ScalableModifyCode* pcScalableModifyCode )
//{
//  pcScalableModifyCode = new ScalableModifyCode;
//
//  ROT( pcScalableModifyCode == NULL );
//
//  return Err::m_nOK;
//}

//JVT-S080 LMI {
ErrVal
ScalableModifyCode::SEICode( h264::SEI::ScalableSeiLayersNotPresent* pcScalableSeiLayersNotPresent, ScalableModifyCode *pcScalableModifyCode )
{
  UInt uiNumScalableLayers = pcScalableSeiLayersNotPresent->getNumLayers();
  UInt uiLayer;
  pcScalableModifyCode->WriteUVLC( uiNumScalableLayers );
  for( uiLayer = 0; uiLayer < uiNumScalableLayers; uiLayer++ )
  {
    pcScalableModifyCode->WriteCode( pcScalableSeiLayersNotPresent->getLayerId( uiLayer ), 8);
  }
  return Err::m_nOK;
}

ErrVal
ScalableModifyCode::SEICode  ( h264::SEI::ScalableSeiDependencyChange* pcScalableSeiDependencyChange, ScalableModifyCode *pcScalableModifyCode )
{
  UInt uiNumScalableLayersMinus1 = pcScalableSeiDependencyChange->getNumLayersMinus1();
     UInt uiLayer, uiDirectLayer;
  pcScalableModifyCode->WriteUVLC( uiNumScalableLayersMinus1 );
  for( uiLayer = 0; uiLayer <= uiNumScalableLayersMinus1; uiLayer++ )
  {
    pcScalableModifyCode->WriteCode( pcScalableSeiDependencyChange->getLayerId( uiLayer ), 8);
    pcScalableModifyCode->WriteFlag( pcScalableSeiDependencyChange->getLayerDependencyInfoPresentFlag( uiLayer ) );
    if ( pcScalableSeiDependencyChange->getLayerDependencyInfoPresentFlag( uiLayer ) )
    {
          pcScalableModifyCode->WriteUVLC( pcScalableSeiDependencyChange->getNumDirectDependentLayers( uiLayer ) );
      for ( uiDirectLayer = 0; uiDirectLayer < pcScalableSeiDependencyChange->getNumDirectDependentLayers( uiLayer ); uiDirectLayer++)
              pcScalableModifyCode->WriteUVLC(pcScalableSeiDependencyChange->getDirectDependentLayerIdDeltaMinus1( uiLayer, uiDirectLayer ));
    }
    else
             pcScalableModifyCode->WriteUVLC(pcScalableSeiDependencyChange->getLayerDependencyInfoSrcLayerIdDeltaMinus1( uiLayer ) );
  }
  return Err::m_nOK;
}
//JVT-S080 LMI }
