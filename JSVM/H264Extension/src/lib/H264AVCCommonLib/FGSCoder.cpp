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

  ::memset( m_apaucLumaCoefMap,       0x00,   16*sizeof(UChar*) );
  ::memset( m_aapaucChromaDCCoefMap,  0x00, 2* 4*sizeof(UChar*) );
  ::memset( m_aapaucChromaACCoefMap,  0x00, 2*16*sizeof(UChar*) );
  ::memset( m_apaucChromaDCBlockMap,  0x00,    2*sizeof(UChar*) );
  ::memset( m_apaucChromaACBlockMap,  0x00,    2*sizeof(UChar*) );
  ::memset( m_apaucScanPosMap,        0x00,    5*sizeof(UChar*) );
  m_paucBlockMap        = 0;
  m_paucSubMbMap        = 0;
  m_pauiMacroblockMap   = 0;

  ::memset( m_apaucBQLumaCoefMap,       0x00,   16*sizeof(UChar*) );
  ::memset( m_aapaucBQChromaDCCoefMap,  0x00, 2* 4*sizeof(UChar*) );
  ::memset( m_aapaucBQChromaACCoefMap,  0x00, 2*16*sizeof(UChar*) );
  ::memset( m_apaucBQChromaDCBlockMap,  0x00,    2*sizeof(UChar*) );
  ::memset( m_apaucBQChromaACBlockMap,  0x00,    2*sizeof(UChar*) );
  m_paucBQBlockMap        = 0;
  m_paucBQSubMbMap        = 0;
  m_pauiBQMacroblockMap   = 0;

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

    Int  i;
    for( i = 0; i < 16; i++ ) 
    {
      delete  [] m_apaucLumaCoefMap        [i];
      delete  [] m_aapaucChromaACCoefMap[0][i];
      delete  [] m_aapaucChromaACCoefMap[1][i];
    }
    for( i = 0; i < 4; i++ ) 
    {
      delete  [] m_aapaucChromaDCCoefMap[0][i];
      delete  [] m_aapaucChromaDCCoefMap[1][i];
    }
    for( i = 0; i < 5; i++ ) 
    {
      delete  [] m_apaucScanPosMap[i];
    }
    delete    [] m_apaucChromaDCBlockMap[0];
    delete    [] m_apaucChromaDCBlockMap[1];
    delete    [] m_apaucChromaACBlockMap[0];
    delete    [] m_apaucChromaACBlockMap[1];
    delete    [] m_paucBlockMap;
    delete    [] m_paucSubMbMap;
    delete    [] m_pauiMacroblockMap;

    for( i = 0; i < 16; i++ )
    {
      ROFS ( ( m_apaucLumaCoefMap        [i] = new UChar[uiSize*16] ) );
      ROFS ( ( m_aapaucChromaACCoefMap[0][i] = new UChar[uiSize*4 ] ) );
      ROFS ( ( m_aapaucChromaACCoefMap[1][i] = new UChar[uiSize*4 ] ) );
    }
    for( i = 0; i < 4; i++ )
    {
      ROFS ( ( m_aapaucChromaDCCoefMap[0][i] = new UChar[uiSize   ] ) );
      ROFS ( ( m_aapaucChromaDCCoefMap[1][i] = new UChar[uiSize   ] ) );
    }
    for( i = 0; i < 5; i++ )
    {
      ROFS ( ( m_apaucScanPosMap[i]          = new UChar[uiSize*16] ) );
    }
    ROFS   ( ( m_apaucChromaDCBlockMap[0]    = new UChar[uiSize   ] ) );
    ROFS   ( ( m_apaucChromaDCBlockMap[1]    = new UChar[uiSize   ] ) );
    ROFS   ( ( m_apaucChromaACBlockMap[0]    = new UChar[uiSize*4 ] ) );
    ROFS   ( ( m_apaucChromaACBlockMap[1]    = new UChar[uiSize*4 ] ) );
    ROFS   ( ( m_paucBlockMap                = new UChar[uiSize*16] ) );
    ROFS   ( ( m_paucSubMbMap                = new UChar[uiSize*4 ] ) );
    ROFS   ( ( m_pauiMacroblockMap           = new UInt [uiSize   ] ) );

    for( i = 0; i < 16; i++ ) 
    {
      delete  [] m_apaucBQLumaCoefMap        [i];
      delete  [] m_aapaucBQChromaACCoefMap[0][i];
      delete  [] m_aapaucBQChromaACCoefMap[1][i];
    }
    for( i = 0; i < 4; i++ ) 
    {
      delete  [] m_aapaucBQChromaDCCoefMap[0][i];
      delete  [] m_aapaucBQChromaDCCoefMap[1][i];
    }
    delete    [] m_apaucBQChromaDCBlockMap[0];
    delete    [] m_apaucBQChromaDCBlockMap[1];
    delete    [] m_apaucBQChromaACBlockMap[0];
    delete    [] m_apaucBQChromaACBlockMap[1];
    delete    [] m_paucBQBlockMap;
    delete    [] m_paucBQSubMbMap;
    delete    [] m_pauiBQMacroblockMap;

    for( i = 0; i < 16; i++ )
    {
      ROFS ( ( m_apaucBQLumaCoefMap        [i] = new UChar[uiSize*16] ) );
      ROFS ( ( m_aapaucBQChromaACCoefMap[0][i] = new UChar[uiSize*4 ] ) );
      ROFS ( ( m_aapaucBQChromaACCoefMap[1][i] = new UChar[uiSize*4 ] ) );
    }
    for( i = 0; i < 4; i++ )
    {
      ROFS ( ( m_aapaucBQChromaDCCoefMap[0][i] = new UChar[uiSize   ] ) );
      ROFS ( ( m_aapaucBQChromaDCCoefMap[1][i] = new UChar[uiSize   ] ) );
    }
    ROFS   ( ( m_apaucBQChromaDCBlockMap[0]    = new UChar[uiSize   ] ) );
    ROFS   ( ( m_apaucBQChromaDCBlockMap[1]    = new UChar[uiSize   ] ) );
    ROFS   ( ( m_apaucBQChromaACBlockMap[0]    = new UChar[uiSize*4 ] ) );
    ROFS   ( ( m_apaucBQChromaACBlockMap[1]    = new UChar[uiSize*4 ] ) );
    ROFS   ( ( m_paucBQBlockMap                = new UChar[uiSize*16] ) );
    ROFS   ( ( m_paucBQSubMbMap                = new UChar[uiSize*4 ] ) );
    ROFS   ( ( m_pauiBQMacroblockMap           = new UInt [uiSize   ] ) );
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
  Int i;

  m_cMbDataCtrlEL.uninit();

  m_bInit                     = false;
  m_papcYuvFullPelBufferCtrl  = 0;
  m_pcTransform               = 0;

  m_bPicInit                  = false;
  m_uiWidthInMB               = 0;
  m_uiHeightInMB              = 0;
  m_pcCurrMbDataCtrl          = 0;

  for( i = 0; i < 16; i++ ) 
  {
    delete  [] m_apaucLumaCoefMap        [i];
    delete  [] m_aapaucChromaACCoefMap[0][i];
    delete  [] m_aapaucChromaACCoefMap[1][i];
  }
  for( i = 0; i < 4; i++ ) 
  {
    delete  [] m_aapaucChromaDCCoefMap[0][i];
    delete  [] m_aapaucChromaDCCoefMap[1][i];
  }
  for( i = 0; i < 5; i++ ) 
  {
    delete  [] m_apaucScanPosMap[i];
  }
  delete    [] m_apaucChromaDCBlockMap[0];
  delete    [] m_apaucChromaDCBlockMap[1];
  delete    [] m_apaucChromaACBlockMap[0];
  delete    [] m_apaucChromaACBlockMap[1];
  delete    [] m_paucBlockMap;
  delete    [] m_paucSubMbMap;
  delete    [] m_pauiMacroblockMap;

  for( i = 0; i < 16; i++ ) 
  {
    delete  [] m_apaucBQLumaCoefMap        [i];
    delete  [] m_aapaucBQChromaACCoefMap[0][i];
    delete  [] m_aapaucBQChromaACCoefMap[1][i];
  }
  for( i = 0; i < 4; i++ ) 
  {
    delete  [] m_aapaucBQChromaDCCoefMap[0][i];
    delete  [] m_aapaucBQChromaDCCoefMap[1][i];
  }
  delete    [] m_apaucBQChromaDCBlockMap[0];
  delete    [] m_apaucBQChromaDCBlockMap[1];
  delete    [] m_apaucBQChromaACBlockMap[0];
  delete    [] m_apaucBQChromaACBlockMap[1];
  delete    [] m_paucBQBlockMap;
  delete    [] m_paucBQSubMbMap;
  delete    [] m_pauiBQMacroblockMap;

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
  int i, uiSize = m_uiWidthInMB * m_uiHeightInMB;

  for( i = 0; i < 16; i++ )
  {
    memcpy( m_apaucBQLumaCoefMap[i], m_apaucLumaCoefMap[i], uiSize*16*sizeof(UChar));
    memcpy( m_aapaucBQChromaACCoefMap[0][i], m_aapaucChromaACCoefMap[0][i], uiSize*4*sizeof(UChar) );
    memcpy( m_aapaucBQChromaACCoefMap[1][i], m_aapaucChromaACCoefMap[1][i], uiSize*4*sizeof(UChar) );
  }
  for( i = 0; i < 4; i++ )
  {
    memcpy ( m_aapaucBQChromaDCCoefMap[0][i], m_aapaucChromaDCCoefMap[0][i], uiSize*sizeof(UChar) );
    memcpy ( m_aapaucBQChromaDCCoefMap[1][i], m_aapaucChromaDCCoefMap[1][i], uiSize*sizeof(UChar) );
  }
  memcpy   ( m_apaucBQChromaDCBlockMap[0], m_apaucChromaDCBlockMap[0], uiSize*sizeof(UChar) );
  memcpy   ( m_apaucBQChromaDCBlockMap[1], m_apaucChromaDCBlockMap[1], uiSize*sizeof(UChar) );
  memcpy   ( m_apaucBQChromaACBlockMap[0], m_apaucChromaACBlockMap[0], uiSize*4*sizeof(UChar) );
  memcpy   ( m_apaucBQChromaACBlockMap[1], m_apaucChromaACBlockMap[1], uiSize*4*sizeof(UChar) );
  memcpy   ( m_paucBQBlockMap, m_paucBlockMap, uiSize*16*sizeof(UChar) );
  memcpy   ( m_paucBQSubMbMap, m_paucSubMbMap, uiSize*4*sizeof(UChar) );
  memcpy   ( m_pauiBQMacroblockMap, m_pauiMacroblockMap,  uiSize*sizeof(UInt) );

  return Err::m_nOK;
}

ErrVal
FGSCoder::xSwitchBQLayerSigMap()
{
  int i, uiSize = m_uiWidthInMB * m_uiHeightInMB;
  UChar* tmpBuf;
  
  ROFS( (tmpBuf = new UChar[uiSize*16] )); 

  for( i = 0; i < 16; i++ )
  {
    memcpy( tmpBuf, m_apaucLumaCoefMap[i], uiSize*16*sizeof(UChar));
    memcpy( m_apaucLumaCoefMap[i], m_apaucBQLumaCoefMap[i], uiSize*16*sizeof(UChar));
    memcpy( m_apaucBQLumaCoefMap[i], tmpBuf, uiSize*16*sizeof(UChar));

    memcpy( tmpBuf, m_aapaucChromaACCoefMap[0][i], uiSize*4*sizeof(UChar) );
    memcpy( m_aapaucChromaACCoefMap[0][i], m_aapaucBQChromaACCoefMap[0][i], uiSize*4*sizeof(UChar) );
    memcpy( m_aapaucBQChromaACCoefMap[0][i], tmpBuf, uiSize*4*sizeof(UChar) );

    memcpy( tmpBuf, m_aapaucChromaACCoefMap[1][i], uiSize*4*sizeof(UChar) );
    memcpy( m_aapaucChromaACCoefMap[1][i], m_aapaucBQChromaACCoefMap[1][i], uiSize*4*sizeof(UChar) );
    memcpy( m_aapaucBQChromaACCoefMap[1][i], tmpBuf, uiSize*4*sizeof(UChar) );
  }
  for( i = 0; i < 4; i++ )
  {
    memcpy ( tmpBuf, m_aapaucChromaDCCoefMap[0][i], uiSize*sizeof(UChar) );
    memcpy ( m_aapaucChromaDCCoefMap[0][i], m_aapaucBQChromaDCCoefMap[0][i], uiSize*sizeof(UChar) );
    memcpy ( m_aapaucBQChromaDCCoefMap[0][i], tmpBuf, uiSize*sizeof(UChar) );

    memcpy ( tmpBuf, m_aapaucChromaDCCoefMap[1][i], uiSize*sizeof(UChar) );
    memcpy ( m_aapaucChromaDCCoefMap[1][i], m_aapaucBQChromaDCCoefMap[1][i], uiSize*sizeof(UChar) );
    memcpy ( m_aapaucBQChromaDCCoefMap[1][i], tmpBuf, uiSize*sizeof(UChar) );
  }
  memcpy   ( tmpBuf, m_apaucChromaDCBlockMap[0], uiSize*sizeof(UChar) );
  memcpy   ( m_apaucChromaDCBlockMap[0], m_apaucBQChromaDCBlockMap[0], uiSize*sizeof(UChar) );
  memcpy   ( m_apaucBQChromaDCBlockMap[0], tmpBuf, uiSize*sizeof(UChar) );

  memcpy   ( tmpBuf, m_apaucChromaDCBlockMap[1], uiSize*sizeof(UChar) );
  memcpy   ( m_apaucChromaDCBlockMap[1], m_apaucBQChromaDCBlockMap[1], uiSize*sizeof(UChar) );
  memcpy   ( m_apaucBQChromaDCBlockMap[1], tmpBuf, uiSize*sizeof(UChar) );

  memcpy   ( tmpBuf, m_apaucChromaACBlockMap[0], uiSize*4*sizeof(UChar) );
  memcpy   ( m_apaucChromaACBlockMap[0], m_apaucBQChromaACBlockMap[0], uiSize*4*sizeof(UChar) );
  memcpy   ( m_apaucBQChromaACBlockMap[0], tmpBuf, uiSize*4*sizeof(UChar) );

  memcpy   ( tmpBuf, m_apaucChromaACBlockMap[1], uiSize*4*sizeof(UChar) );
  memcpy   ( m_apaucChromaACBlockMap[1], m_apaucBQChromaACBlockMap[1], uiSize*4*sizeof(UChar) );
  memcpy   ( m_apaucBQChromaACBlockMap[1], tmpBuf, uiSize*4*sizeof(UChar) );

  memcpy   ( tmpBuf, m_paucBlockMap, uiSize*16*sizeof(UChar) );
  memcpy   ( m_paucBlockMap, m_paucBQBlockMap, uiSize*16*sizeof(UChar) );
  memcpy   ( m_paucBQBlockMap, tmpBuf, uiSize*16*sizeof(UChar) );

  memcpy   ( tmpBuf, m_paucSubMbMap, uiSize*4*sizeof(UChar) );
  memcpy   ( m_paucSubMbMap, m_paucBQSubMbMap, uiSize*4*sizeof(UChar) );
  memcpy   ( m_paucBQSubMbMap, tmpBuf, uiSize*4*sizeof(UChar) );

  memcpy   ( tmpBuf, m_pauiMacroblockMap,  uiSize*sizeof(UInt) );
  memcpy   ( m_pauiMacroblockMap,  m_pauiBQMacroblockMap, uiSize*sizeof(UInt) );
  memcpy   ( m_pauiBQMacroblockMap, tmpBuf,  uiSize*sizeof(UInt) );

  delete [] tmpBuf;

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
    m_pauiMacroblockMap[uiMbIndex] =  ( bIntra16x16 || bIsSignificant                           ? SIGNIFICANT         : CLEAR )
                                   +  ( bIntra16x16 || bIntra4x4 || bIsSigLuma || !b8x8Present  ? TRANSFORM_SPECIFIED : CLEAR );
    
    //--- LUMA ---
    for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
    {
      UInt uiSubMbIndex = ( 2*uiMbY + c8x8Idx.y()/2 ) * 2 * m_uiWidthInMB + ( 2*uiMbX + c8x8Idx.x() / 2 );

      //===== set sub-macroblock mode =====
      m_paucSubMbMap[uiSubMbIndex] = ( ( uiMbCbp & ( 1 << c8x8Idx.b8x8Index() ) ) > 0 ? SIGNIFICANT : CLEAR );

      if( b8x8Transform )
      {
        UInt    auiBlockIdx[4]  = { ( 4*uiMbY + c8x8Idx.y()     ) * 4 * m_uiWidthInMB + ( 4*uiMbX + c8x8Idx.x()     ),
                                    ( 4*uiMbY + c8x8Idx.y()     ) * 4 * m_uiWidthInMB + ( 4*uiMbX + c8x8Idx.x() + 1 ),
                                    ( 4*uiMbY + c8x8Idx.y() + 1 ) * 4 * m_uiWidthInMB + ( 4*uiMbX + c8x8Idx.x()     ),
                                    ( 4*uiMbY + c8x8Idx.y() + 1 ) * 4 * m_uiWidthInMB + ( 4*uiMbX + c8x8Idx.x() + 1 ) };
        TCoeff* piCoeff         = rcMbData.getMbTCoeffs().get8x8( c8x8Idx );

        //===== set block mode =====
        m_apaucScanPosMap[0][auiBlockIdx[0]] = 16;
        m_apaucScanPosMap[0][auiBlockIdx[1]] = 16;
        m_apaucScanPosMap[0][auiBlockIdx[2]] = 16;
        m_apaucScanPosMap[0][auiBlockIdx[3]] = 16;

        m_paucBlockMap[auiBlockIdx[0]] = m_paucSubMbMap[uiSubMbIndex];
        m_paucBlockMap[auiBlockIdx[1]] = m_paucSubMbMap[uiSubMbIndex];
        m_paucBlockMap[auiBlockIdx[2]] = m_paucSubMbMap[uiSubMbIndex];
        m_paucBlockMap[auiBlockIdx[3]] = m_paucSubMbMap[uiSubMbIndex];

        //===== set transform coefficients =====
        for( UInt ui8x8ScanIndex = 0; ui8x8ScanIndex < 64; ui8x8ScanIndex++ )
        {
          UInt  uiS = ui8x8ScanIndex/4;
          UInt  uiB = auiBlockIdx[ui8x8ScanIndex%4];
          m_apaucLumaCoefMap[uiS][uiB] = ( piCoeff[g_aucFrameScan64[ui8x8ScanIndex]] ? SIGNIFICANT : CLEAR );

          if (m_apaucLumaCoefMap[uiS][uiB] & SIGNIFICANT)
          {
            if (piCoeff[g_aucFrameScan64[ui8x8ScanIndex]] < 0)
              m_apaucLumaCoefMap[uiS][uiB] |= BASE_SIGN;
          }
          if( !( m_apaucLumaCoefMap[uiS][uiB] & (SIGNIFICANT|CODED) ) && m_apaucScanPosMap[0][uiB] == 16 )
            m_apaucScanPosMap[0][uiB] = uiS;
        }
      }
      else
      {
        for( S4x4Idx cIdx( c8x8Idx ); cIdx.isLegal( c8x8Idx ); cIdx++ )
        {
          UInt    uiBlockIdx  = ( 4*uiMbY + cIdx.y() ) * 4 * m_uiWidthInMB + ( 4*uiMbX + cIdx.x() );
          TCoeff* piCoeff     = rcMbData.getMbTCoeffs().get( cIdx );
          UChar   ucBlockSig  = CLEAR;

          m_apaucScanPosMap[0][uiBlockIdx] = 16;

          //===== set transform coefficients =====
          for( UInt uiScanIndex = 0; uiScanIndex < 16; uiScanIndex++ )
          {
            if( piCoeff[g_aucFrameScan[uiScanIndex]] )
            {
              m_apaucLumaCoefMap[uiScanIndex][uiBlockIdx] = SIGNIFICANT;
              ucBlockSig                                  = SIGNIFICANT;

              if (piCoeff[g_aucFrameScan[uiScanIndex]] < 0)
                m_apaucLumaCoefMap[uiScanIndex][uiBlockIdx] |= BASE_SIGN;
            }
            else
            {
              m_apaucLumaCoefMap[uiScanIndex][uiBlockIdx] = CLEAR;
            }
            if( !( m_apaucLumaCoefMap[uiScanIndex][uiBlockIdx] & (SIGNIFICANT|CODED) ) && m_apaucScanPosMap[0][uiBlockIdx] == 16 )
              m_apaucScanPosMap[0][uiBlockIdx] = uiScanIndex;
          }

          //===== set block mode =====
          m_paucBlockMap[uiBlockIdx] = ucBlockSig;
        }
      }
    }


    //--- CHROMA DC ---
    for( UInt uiPlane = 0; uiPlane < 2; uiPlane++ )
    {
      TCoeff* piCoeff     = rcMbData.getMbTCoeffs().get( CIdx(4*uiPlane) );
      UChar   ucBlockSig  = CLEAR;

      m_apaucScanPosMap[uiPlane + 1][uiMbIndex] = 4;
      for( UInt ui = 0; ui < 4; ui++ )
      {
        if( piCoeff[g_aucIndexChromaDCScan[ui]] )
        {
          m_aapaucChromaDCCoefMap[uiPlane][ui][uiMbIndex] = SIGNIFICANT;
          ucBlockSig                                      = SIGNIFICANT;

          if (piCoeff[g_aucIndexChromaDCScan[ui]] < 0)
            m_aapaucChromaDCCoefMap[uiPlane][ui][uiMbIndex] |= BASE_SIGN;
        }
        else
        {
          m_aapaucChromaDCCoefMap[uiPlane][ui][uiMbIndex] = CLEAR;
        }
        if( !( m_aapaucChromaDCCoefMap[uiPlane][ui][uiMbIndex] & (SIGNIFICANT|CODED) ) && m_apaucScanPosMap[uiPlane + 1][uiMbIndex] == 4 )
          m_apaucScanPosMap[uiPlane + 1][uiMbIndex] = ui;
      }
      m_apaucChromaDCBlockMap[uiPlane][uiMbIndex] = ucBlockSig;
    }

    //--- CHROMA AC ---
    for( CIdx cCIdx; cCIdx.isLegal(); cCIdx++ )
    {
      UInt    ui8x8Idx    = ( 2*uiMbY + cCIdx.y() ) * 2 * m_uiWidthInMB + ( 2 * uiMbX + cCIdx.x() );
      TCoeff* piCoeff     = rcMbData.getMbTCoeffs().get( cCIdx );
      UChar   ucBlockSig  = CLEAR;

      m_apaucScanPosMap[cCIdx.plane() + 3][ui8x8Idx] = 16;
      for( UInt ui = 1; ui < 16; ui++ )
      {
        if( piCoeff[g_aucFrameScan[ui]] )
        {
          m_aapaucChromaACCoefMap[cCIdx.plane()][ui][ui8x8Idx]  = SIGNIFICANT;
          ucBlockSig                                            = SIGNIFICANT;

          if (piCoeff[g_aucFrameScan[ui]] < 0)
            m_aapaucChromaACCoefMap[cCIdx.plane()][ui][ui8x8Idx] |= BASE_SIGN;
        }
        else
        {
          m_aapaucChromaACCoefMap[cCIdx.plane()][ui][ui8x8Idx]  = CLEAR;
        }
        if( !( m_aapaucChromaACCoefMap[cCIdx.plane()][ui][ui8x8Idx] & (SIGNIFICANT|CODED) ) && m_apaucScanPosMap[cCIdx.plane() + 3][ui8x8Idx] == 16 )
          m_apaucScanPosMap[cCIdx.plane() + 3][ui8x8Idx] = ui;
      }
      m_apaucChromaACBlockMap[cCIdx.plane()][ui8x8Idx] = ucBlockSig;
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
    m_pauiMacroblockMap[uiMbIndex] =  ( bIntra16x16 || bIsSignificant                           ? SIGNIFICANT         : CLEAR )
                                   +  ( bIntra16x16 || bIntra4x4 || bIsSigLuma || !b8x8Present  ? TRANSFORM_SPECIFIED : CLEAR );
    
    //--- LUMA ---
    for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
    {
      UInt uiSubMbIndex = ( 2*uiMbY + c8x8Idx.y()/2 ) * 2 * m_uiWidthInMB + ( 2*uiMbX + c8x8Idx.x() / 2 );

      //===== set sub-macroblock mode =====
      m_paucSubMbMap[uiSubMbIndex] = ( ( uiMbCbp & ( 1 << c8x8Idx.b8x8Index() ) ) > 0 ? SIGNIFICANT : CLEAR );

      if( b8x8Transform )
      {
        UInt    auiBlockIdx[4]  = { ( 4*uiMbY + c8x8Idx.y()     ) * 4 * m_uiWidthInMB + ( 4*uiMbX + c8x8Idx.x()     ),
                                    ( 4*uiMbY + c8x8Idx.y()     ) * 4 * m_uiWidthInMB + ( 4*uiMbX + c8x8Idx.x() + 1 ),
                                    ( 4*uiMbY + c8x8Idx.y() + 1 ) * 4 * m_uiWidthInMB + ( 4*uiMbX + c8x8Idx.x()     ),
                                    ( 4*uiMbY + c8x8Idx.y() + 1 ) * 4 * m_uiWidthInMB + ( 4*uiMbX + c8x8Idx.x() + 1 ) };
        TCoeff* piCoeff         = rcMbData.getMbTCoeffs().get8x8( c8x8Idx );

        //===== set block mode =====
        m_apaucScanPosMap[0][auiBlockIdx[0]] = 16;
        m_apaucScanPosMap[0][auiBlockIdx[1]] = 16;
        m_apaucScanPosMap[0][auiBlockIdx[2]] = 16;
        m_apaucScanPosMap[0][auiBlockIdx[3]] = 16;

        m_paucBlockMap[auiBlockIdx[0]] = m_paucSubMbMap[uiSubMbIndex];
        m_paucBlockMap[auiBlockIdx[1]] = m_paucSubMbMap[uiSubMbIndex];
        m_paucBlockMap[auiBlockIdx[2]] = m_paucSubMbMap[uiSubMbIndex];
        m_paucBlockMap[auiBlockIdx[3]] = m_paucSubMbMap[uiSubMbIndex];

        //===== set transform coefficients =====
        for( UInt ui8x8ScanIndex = 0; ui8x8ScanIndex < 64; ui8x8ScanIndex++ )
        {
          UInt  uiS = ui8x8ScanIndex/4;
          UInt  uiB = auiBlockIdx[ui8x8ScanIndex%4];
          m_apaucLumaCoefMap[uiS][uiB] = ( piCoeff[g_aucFrameScan64[ui8x8ScanIndex]] ? SIGNIFICANT : CLEAR );

          if (m_apaucLumaCoefMap[uiS][uiB] & SIGNIFICANT)
          {
            if (piCoeff[g_aucFrameScan64[ui8x8ScanIndex]] < 0)
              m_apaucLumaCoefMap[uiS][uiB] |= BASE_SIGN;
          }
          if( !( m_apaucLumaCoefMap[uiS][uiB] & (SIGNIFICANT|CODED) ) && m_apaucScanPosMap[0][uiB] == 16 )
            m_apaucScanPosMap[0][uiB] = uiS;
        }
      }
      else
      {
        for( S4x4Idx cIdx( c8x8Idx ); cIdx.isLegal( c8x8Idx ); cIdx++ )
        {
          UInt    uiBlockIdx  = ( 4*uiMbY + cIdx.y() ) * 4 * m_uiWidthInMB + ( 4*uiMbX + cIdx.x() );
          TCoeff* piCoeff     = rcMbData.getMbTCoeffs().get( cIdx );
          UChar   ucBlockSig  = CLEAR;

          m_apaucScanPosMap[0][uiBlockIdx] = 16;

          //===== set transform coefficients =====
          for( UInt uiScanIndex = 0; uiScanIndex < 16; uiScanIndex++ )
          {
            if( piCoeff[g_aucFrameScan[uiScanIndex]] )
            {
              m_apaucLumaCoefMap[uiScanIndex][uiBlockIdx] = SIGNIFICANT;
              ucBlockSig                                  = SIGNIFICANT;

              if (piCoeff[g_aucFrameScan[uiScanIndex]] < 0)
                m_apaucLumaCoefMap[uiScanIndex][uiBlockIdx] |= BASE_SIGN;
            }
            else
            {
              m_apaucLumaCoefMap[uiScanIndex][uiBlockIdx] = CLEAR;
            }
            if( !( m_apaucLumaCoefMap[uiScanIndex][uiBlockIdx] & (SIGNIFICANT|CODED) ) && m_apaucScanPosMap[0][uiBlockIdx] == 16 )
              m_apaucScanPosMap[0][uiBlockIdx] = uiScanIndex;
          }

          //===== set block mode =====
          m_paucBlockMap[uiBlockIdx] = ucBlockSig;
        }
      }
    }


    //--- CHROMA DC ---
    for( UInt uiPlane = 0; uiPlane < 2; uiPlane++ )
    {
      TCoeff* piCoeff     = rcMbData.getMbTCoeffs().get( CIdx(4*uiPlane) );
      UChar   ucBlockSig  = CLEAR;

      m_apaucScanPosMap[uiPlane + 1][uiMbIndex] = 4;
      for( UInt ui = 0; ui < 4; ui++ )
      {
        if( piCoeff[g_aucIndexChromaDCScan[ui]] )
        {
          m_aapaucChromaDCCoefMap[uiPlane][ui][uiMbIndex] = SIGNIFICANT;
          ucBlockSig                                      = SIGNIFICANT;

          if (piCoeff[g_aucIndexChromaDCScan[ui]] < 0)
            m_aapaucChromaDCCoefMap[uiPlane][ui][uiMbIndex] |= BASE_SIGN;
        }
        else
        {
          m_aapaucChromaDCCoefMap[uiPlane][ui][uiMbIndex] = CLEAR;
        }
        if( !( m_aapaucChromaDCCoefMap[uiPlane][ui][uiMbIndex] & (SIGNIFICANT|CODED) ) && m_apaucScanPosMap[uiPlane + 1][uiMbIndex] == 4 )
          m_apaucScanPosMap[uiPlane + 1][uiMbIndex] = ui;
      }
      m_apaucChromaDCBlockMap[uiPlane][uiMbIndex] = ucBlockSig;
    }

    //--- CHROMA AC ---
    for( CIdx cCIdx; cCIdx.isLegal(); cCIdx++ )
    {
      UInt    ui8x8Idx    = ( 2*uiMbY + cCIdx.y() ) * 2 * m_uiWidthInMB + ( 2 * uiMbX + cCIdx.x() );
      TCoeff* piCoeff     = rcMbData.getMbTCoeffs().get( cCIdx );
      UChar   ucBlockSig  = CLEAR;

      m_apaucScanPosMap[cCIdx.plane() + 3][ui8x8Idx] = 16;
      for( UInt ui = 1; ui < 16; ui++ )
      {
        if( piCoeff[g_aucFrameScan[ui]] )
        {
          m_aapaucChromaACCoefMap[cCIdx.plane()][ui][ui8x8Idx]  = SIGNIFICANT;
          ucBlockSig                                            = SIGNIFICANT;

          if (piCoeff[g_aucFrameScan[ui]] < 0)
            m_aapaucChromaACCoefMap[cCIdx.plane()][ui][ui8x8Idx] |= BASE_SIGN;
        }
        else
        {
          m_aapaucChromaACCoefMap[cCIdx.plane()][ui][ui8x8Idx]  = CLEAR;
        }
        if( !( m_aapaucChromaACCoefMap[cCIdx.plane()][ui][ui8x8Idx] & (SIGNIFICANT|CODED) ) && m_apaucScanPosMap[cCIdx.plane() + 3][ui8x8Idx] == 16 )
          m_apaucScanPosMap[cCIdx.plane() + 3][ui8x8Idx] = ui;
      }
      m_apaucChromaACBlockMap[cCIdx.plane()][ui8x8Idx] = ucBlockSig;
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
      m_pauiMacroblockMap[uiMbIndex] &= SIGNIFICANT | TRANSFORM_SPECIFIED;

      //--- LUMA ---
      for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
      {
        UInt uiSubMbIndex = ( 2*uiMbY + c8x8Idx.y()/2 ) * 2 * m_uiWidthInMB + ( 2*uiMbX + c8x8Idx.x() / 2 );

        //===== set sub-macroblock mode =====
        m_paucSubMbMap[uiSubMbIndex] = ( ( uiMbCbp & ( 1 << c8x8Idx.b8x8Index() ) ) > 0 ? SIGNIFICANT : CLEAR );

        if( b8x8Transform )
        {
          UInt    auiBlockIdx[4]  = { ( 4*uiMbY + c8x8Idx.y()     ) * 4 * m_uiWidthInMB + ( 4*uiMbX + c8x8Idx.x()     ),
            ( 4*uiMbY + c8x8Idx.y()     ) * 4 * m_uiWidthInMB + ( 4*uiMbX + c8x8Idx.x() + 1 ),
            ( 4*uiMbY + c8x8Idx.y() + 1 ) * 4 * m_uiWidthInMB + ( 4*uiMbX + c8x8Idx.x()     ),
            ( 4*uiMbY + c8x8Idx.y() + 1 ) * 4 * m_uiWidthInMB + ( 4*uiMbX + c8x8Idx.x() + 1 ) };

          //===== set block mode =====
          m_paucBlockMap[auiBlockIdx[0]] &= ~CODED;
          m_paucBlockMap[auiBlockIdx[1]] &= ~CODED;
          m_paucBlockMap[auiBlockIdx[2]] &= ~CODED;
          m_paucBlockMap[auiBlockIdx[3]] &= ~CODED;

          //===== set transform coefficients =====
          UInt ui8x8ScanIndex;
          for( ui8x8ScanIndex = 0; ui8x8ScanIndex < 64; ui8x8ScanIndex++ )
          {
            UInt  uiS = ui8x8ScanIndex/4;
            UInt  uiB = auiBlockIdx[ui8x8ScanIndex%4];
            m_apaucLumaCoefMap[uiS][uiB] &= ~CODED;
          }
          m_apaucScanPosMap[0][auiBlockIdx[0]] = 16;
          m_apaucScanPosMap[0][auiBlockIdx[1]] = 16;
          m_apaucScanPosMap[0][auiBlockIdx[2]] = 16;
          m_apaucScanPosMap[0][auiBlockIdx[3]] = 16;

          for( ui8x8ScanIndex = 0; ui8x8ScanIndex < 64; ui8x8ScanIndex++ )
          {
            UInt  uiS = ui8x8ScanIndex/4;
            UInt  uiB = auiBlockIdx[ui8x8ScanIndex%4];
            if( !( m_apaucLumaCoefMap[uiS][uiB] & (SIGNIFICANT|CODED) ) && m_apaucScanPosMap[0][uiB] == 16 )
              m_apaucScanPosMap[0][uiB] = uiS;
          }
        }
        else
        {
          for( S4x4Idx cIdx( c8x8Idx ); cIdx.isLegal( c8x8Idx ); cIdx++ )
          {
            UInt uiScanIndex;
            UInt uiBlockIdx  = ( 4*uiMbY + cIdx.y() ) * 4 * m_uiWidthInMB + ( 4*uiMbX + cIdx.x() );
            for( uiScanIndex = 0; uiScanIndex < 16; uiScanIndex++ )
              m_apaucLumaCoefMap[uiScanIndex][uiBlockIdx] &= ~CODED;
            m_apaucScanPosMap[0][uiBlockIdx] = 16;
            for( uiScanIndex = 0; uiScanIndex < 16; uiScanIndex++ )
              if( !( m_apaucLumaCoefMap[uiScanIndex][uiBlockIdx] & (SIGNIFICANT|CODED) ) && m_apaucScanPosMap[0][uiBlockIdx] == 16 )
                m_apaucScanPosMap[0][uiBlockIdx] = uiScanIndex;

            m_paucBlockMap[uiBlockIdx] &= ~CODED;
          }
        }
      }


      //--- CHROMA DC ---
      for( UInt uiPlane = 0; uiPlane < 2; uiPlane++ )
      {
        UInt ui;
        for( ui = 0; ui < 4; ui++ )
          m_aapaucChromaDCCoefMap[uiPlane][ui][uiMbIndex] &= ~CODED;
        m_apaucScanPosMap[uiPlane + 1][uiMbIndex] = 4;
        for( ui = 0; ui < 4; ui++ )
          if( !( m_aapaucChromaDCCoefMap[uiPlane][ui][uiMbIndex] & (SIGNIFICANT|CODED) ) && m_apaucScanPosMap[uiPlane + 1][uiMbIndex] == 4 )
            m_apaucScanPosMap[uiPlane + 1][uiMbIndex] = ui;

        m_apaucChromaDCBlockMap[uiPlane][uiMbIndex] &= ~CODED;
      }

      //--- CHROMA AC ---
      for( CIdx cCIdx; cCIdx.isLegal(); cCIdx++ )
      {
        UInt  ui;
        UInt  ui8x8Idx    = ( 2*uiMbY + cCIdx.y() ) * 2 * m_uiWidthInMB + ( 2 * uiMbX + cCIdx.x() );

        for( ui = 1; ui < 16; ui++ )
          m_aapaucChromaACCoefMap[cCIdx.plane()][ui][ui8x8Idx] &= ~CODED;
        m_apaucScanPosMap[cCIdx.plane() + 3][ui8x8Idx] = 16;
        for( ui = 1; ui < 16; ui++ )
          if( !( m_aapaucChromaACCoefMap[cCIdx.plane()][ui][ui8x8Idx] & (SIGNIFICANT|CODED) ) && m_apaucScanPosMap[cCIdx.plane() + 3][ui8x8Idx] == 16 )
            m_apaucScanPosMap[cCIdx.plane() + 3][ui8x8Idx] = ui;

        m_apaucChromaACBlockMap[cCIdx.plane()][ui8x8Idx] &= ~CODED;
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

      for( UInt ui8x8ScanIdx = 0; ui8x8ScanIdx < 64; ui8x8ScanIdx++ )
      {
        UInt  uiPos         = g_aucFrameScan64[ui8x8ScanIdx];
        UInt  ui4x4ScanIdx  = ui8x8ScanIdx / 4;
        UInt  uiBlk4x4Idx   = ui8x8ScanIdx % 4;
        UInt  uiBlockIndex  = (4*uiMbY+c8x8Idx.y()+(uiBlk4x4Idx/2))*4*m_uiWidthInMB + (4*uiMbX+c8x8Idx.x()+(uiBlk4x4Idx%2));

        if( m_apaucLumaCoefMap[ui4x4ScanIdx][uiBlockIndex] & CODED )
        {
          xUpdateCoeffMap(piCoeffBL[uiPos], piCoeffEL[uiPos], m_apaucLumaCoefMap[ui4x4ScanIdx][uiBlockIndex]);
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
    for( B4x4Idx c4x4Idx; c4x4Idx.isLegal(); c4x4Idx++ )
    {
      Bool    bSig      = false;
      TCoeff* piCoeffBL = rcMbDataAccessBL.getMbTCoeffs().get( c4x4Idx );
      TCoeff* piCoeffEL = rcMbDataAccessEL.getMbTCoeffs().get( c4x4Idx );

      for( UInt uiScanIdx = 0; uiScanIdx < 16; uiScanIdx++ )
      {
        UInt  uiPos         = g_aucFrameScan[uiScanIdx];
        UInt  uiBlockIndex  = (4*uiMbY+c4x4Idx.y())*4*m_uiWidthInMB + (4*uiMbX+c4x4Idx.x());

        if( m_apaucLumaCoefMap[uiScanIdx][uiBlockIndex] & CODED )
        {
          xUpdateCoeffMap(piCoeffBL[uiPos], piCoeffEL[uiPos], m_apaucLumaCoefMap[uiScanIdx][uiBlockIndex]);
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
  for( UInt uiPlane = 0; uiPlane < 2; uiPlane++ )
  {
    TCoeff* piCoeffBL = rcMbDataAccessBL.getMbTCoeffs().get( CIdx(4*uiPlane) );
    TCoeff* piCoeffEL = rcMbDataAccessEL.getMbTCoeffs().get( CIdx(4*uiPlane) );

    for( UInt uiDCIdx = 0; uiDCIdx < 4; uiDCIdx++ )
    {
      UInt uiPos = g_aucIndexChromaDCScan[uiDCIdx];

      if( m_aapaucChromaDCCoefMap[uiPlane][uiDCIdx][uiMbIndex] & CODED)
      {
        xUpdateCoeffMap(piCoeffBL[uiPos], piCoeffEL[uiPos], m_aapaucChromaDCCoefMap[uiPlane][uiDCIdx][uiMbIndex]);
      }

      if( piCoeffBL[uiPos] )
      {
        bSigDC = true;
      }
    }
  }

  //===== chroma AC =====
  Bool  bSigAC = false;
  for( CIdx cCIdx; cCIdx.isLegal(); cCIdx++ )
  {
    TCoeff* piCoeffBL = rcMbDataAccessBL.getMbTCoeffs().get( cCIdx );
    TCoeff* piCoeffEL = rcMbDataAccessEL.getMbTCoeffs().get( cCIdx );

    for( UInt uiScanIdx = 1; uiScanIdx < 16; uiScanIdx++ )
    {
      UInt  uiPos     = g_aucFrameScan[uiScanIdx];
      UInt  ui8x8Idx  = (2*uiMbY+cCIdx.y())*2*m_uiWidthInMB + (2*uiMbX+cCIdx.x());

      if( m_aapaucChromaACCoefMap[cCIdx.plane()][uiScanIdx][ui8x8Idx] & CODED )
      {
        xUpdateCoeffMap(piCoeffBL[uiPos], piCoeffEL[uiPos], m_aapaucChromaACCoefMap[cCIdx.plane()][uiScanIdx][ui8x8Idx]);
      }
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
  Int iNumCoded = ( m_pauiMacroblockMap[uiMbIndex] >> NUM_COEFF_SHIFT );
  Int iQPDelta  = ( 384 - iNumCoded ) / 64;
  Int iQP       = min( 51, iELQP + iQPDelta );
  if( ! ( m_pauiMacroblockMap[uiMbIndex] & SIGNIFICANT ) )
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
      piCoeff[16*uiDCIdx] *= iScale;

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
  for( UInt uiPlane = 0; uiPlane < 2; uiPlane++ )
  {
    TCoeff*      piCoeff = rcMbDataAccess.getMbTCoeffs().get( CIdx(4*uiPlane) );

    iScale = g_aaiDequantCoef[cCQp.rem()][0] << cCQp.per();
    /* HS: old scaling modified:
       (I did not work for scaling matrices, when QpPer became less than 5 in an FGS enhancement) */

    for( UInt uiDCIdx = 0; uiDCIdx < 4; uiDCIdx++ )
    {
      if(! bBaseLayer && ! ( m_aapaucChromaDCCoefMap[uiPlane][uiDCIdx][uiMbIndex] & CODED ) )
        // condition "! bBaseLayer" is needed. When "xScaleTCoeffs is called 
        // before first FGS layer, m_aapaucChromaDCCoefMap is not initialized
        piCoeff[16*uiDCIdx]  = 0;
      else
        piCoeff[16*uiDCIdx] *= iScale;
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
  UInt uiBlockIndex = (uiMbY * 4 + cIdx.y()) * m_uiWidthInMB * 4 + uiMbX * 4 + cIdx.x();

  for( UInt uiScanIndex = 0; uiScanIndex < 16; uiScanIndex ++ )
  {
    pucSigMap[g_aucFrameScan[uiScanIndex]] = 
      SIGNIFICANT & m_apaucLumaCoefMap[uiScanIndex][uiBlockIndex];
  }
}


// get 8x8 significance map for luma
Void FGSCoder::getCoeffSigMap( UInt uiMbX, UInt uiMbY, B8x8Idx c8x8Idx, UChar *pucSigMap )
{
  UInt  auiBlockIdx[4]  = { ( 4*uiMbY + c8x8Idx.y()     ) * 4 * m_uiWidthInMB + ( 4*uiMbX + c8x8Idx.x()     ),
    ( 4*uiMbY + c8x8Idx.y()     ) * 4 * m_uiWidthInMB + ( 4*uiMbX + c8x8Idx.x() + 1 ),
    ( 4*uiMbY + c8x8Idx.y() + 1 ) * 4 * m_uiWidthInMB + ( 4*uiMbX + c8x8Idx.x()     ),
    ( 4*uiMbY + c8x8Idx.y() + 1 ) * 4 * m_uiWidthInMB + ( 4*uiMbX + c8x8Idx.x() + 1 ) };

  for( UInt ui8x8ScanIndex = 0; ui8x8ScanIndex < 64; ui8x8ScanIndex ++ )
  {
    UInt  uiS = ui8x8ScanIndex/4;
    UInt  uiB = auiBlockIdx[ui8x8ScanIndex % 4];

    pucSigMap[g_aucFrameScan64[ui8x8ScanIndex]] = 
      SIGNIFICANT & m_apaucLumaCoefMap[uiS][uiB];
  }
}


// get 4x4 significance map for chroma
Void FGSCoder::getCoeffSigMap( UInt uiMbX, UInt uiMbY, CIdx cIdx, UChar *pucSigMap )
{
  UInt uiPlane      = cIdx.plane();
  UInt uiMbIndex    = uiMbY * m_uiWidthInMB + uiMbX;
  UInt uiBlockIndex = (uiMbY * 2 + cIdx.y()) * m_uiWidthInMB * 2 + uiMbX * 2 + cIdx.x();

  for( UInt uiScanIndex = 0; uiScanIndex < 16; uiScanIndex ++ )
  {
    pucSigMap[g_aucFrameScan[uiScanIndex]] = 
      SIGNIFICANT & ((uiScanIndex == 0) ?
      m_aapaucChromaDCCoefMap[uiPlane][cIdx.y() * 2 + cIdx.x()][uiMbIndex]
      : m_aapaucChromaACCoefMap[cIdx.plane()][uiScanIndex][uiBlockIndex]);
  }
}


// get entire 8x8 significance map for chroma
Void FGSCoder::getCoeffSigMapChroma8x8( UInt uiMbX, UInt uiMbY, UInt uiPlane, UChar *pucSigMap )
{
  UInt uiMbIndex    = uiMbY * m_uiWidthInMB + uiMbX;
  UInt uiBeginIdx, uiEndIdx;

  uiBeginIdx = (uiPlane == 0) ? 0 : 4;
  uiEndIdx   = uiBeginIdx + 4;

  for( CIdx cIdx(uiBeginIdx); cIdx.isLegal(uiEndIdx); cIdx ++ )
  {
    UInt uiBlockIndex     = (uiMbY * 2 + cIdx.y()) * m_uiWidthInMB * 2 + uiMbX * 2 + cIdx.x();
    UInt uiBlockIdxWithMb = cIdx.y() * 2 + cIdx.x();

    for( UInt uiScanIndex = 0; uiScanIndex < 16; uiScanIndex ++ )
    {
      pucSigMap[uiBlockIdxWithMb * 16 + g_aucFrameScan[uiScanIndex]] = 
        SIGNIFICANT & ((uiScanIndex == 0) ?
        m_aapaucChromaDCCoefMap[uiPlane][uiBlockIdxWithMb][uiMbIndex]
        : m_aapaucChromaACCoefMap[cIdx.plane()][uiScanIndex][uiBlockIndex]);
    }
  }
}


ErrVal
FGSCoder::xClearBaseCoeffs( MbDataAccess& rcMbDataAccess, 
                            MbDataAccess* pcMbDataAccessBase )
{
  UInt uiMbY     = pcMbDataAccessBase->getMbY();
  UInt uiMbX     = pcMbDataAccessBase->getMbX();
  UInt uiMbIndex = uiMbY * m_uiWidthInMB + uiMbX;

  m_pauiMacroblockMap[uiMbIndex] = rcMbDataAccess.getSH().getPPS().getTransform8x8ModeFlag() && rcMbDataAccess.getMbData().is8x8TrafoFlagPresent() ? CLEAR : TRANSFORM_SPECIFIED;

  //--- LUMA ---
  for( B8x8Idx c8x8Idx; c8x8Idx.isLegal(); c8x8Idx++ )
  {
    UInt uiSubMbIndex = ( 2*uiMbY + c8x8Idx.y()/2 ) * 2 * m_uiWidthInMB + ( 2*uiMbX + c8x8Idx.x() / 2 );

    //===== set sub-macroblock mode =====
    m_paucSubMbMap[uiSubMbIndex] = CLEAR;

    for( S4x4Idx cIdx( c8x8Idx ); cIdx.isLegal( c8x8Idx ); cIdx++ )
    {
      UInt    uiScanIndex;
      UInt    uiBlockIdx  = ( 4*uiMbY + cIdx.y() ) * 4 * m_uiWidthInMB + ( 4*uiMbX + cIdx.x() );

      //===== set transform coefficients =====
      for( uiScanIndex = 0; uiScanIndex < 16; uiScanIndex++ )
      {
        m_apaucLumaCoefMap[uiScanIndex][uiBlockIdx] = CLEAR;
      }
      m_apaucScanPosMap[0][uiBlockIdx] = 16;
      for( uiScanIndex = 0; uiScanIndex < 16; uiScanIndex++ )
      {
        if( !( m_apaucLumaCoefMap[uiScanIndex][uiBlockIdx] & (SIGNIFICANT|CODED) ) && m_apaucScanPosMap[0][uiBlockIdx] == 16 )
        {
          m_apaucScanPosMap[0][uiBlockIdx] = uiScanIndex;
        }
      }

      //===== set block mode =====
      m_paucBlockMap[uiBlockIdx] = CLEAR;
    }
  }

  //--- CHROMA DC ---
  for( UInt uiPlane = 0; uiPlane < 2; uiPlane++ )
  {
    UInt ui;
    for( ui = 0; ui < 4; ui++ )
    {
      m_aapaucChromaDCCoefMap[uiPlane][ui][uiMbIndex] = CLEAR;
    }
    m_apaucScanPosMap[uiPlane + 1][uiMbIndex] = 4;
    for( ui = 0; ui < 4; ui++ )
    {
      if( !( m_aapaucChromaDCCoefMap[uiPlane][ui][uiMbIndex] & (SIGNIFICANT|CODED) ) && m_apaucScanPosMap[uiPlane + 1][uiMbIndex] == 4 )
      {
        m_apaucScanPosMap[uiPlane + 1][uiMbIndex] = ui;
      }
    }
    m_apaucChromaDCBlockMap[uiPlane][uiMbIndex] = CLEAR;
  }

  //--- CHROMA AC ---
  for( CIdx cCIdx; cCIdx.isLegal(); cCIdx++ )
  {
    UInt    ui;
    UInt    ui8x8Idx    = ( 2*uiMbY + cCIdx.y() ) * 2 * m_uiWidthInMB + ( 2 * uiMbX + cCIdx.x() );

    for( ui = 1; ui < 16; ui++ )
    {
      m_aapaucChromaACCoefMap[cCIdx.plane()][ui][ui8x8Idx]  = CLEAR;
    }
    m_apaucScanPosMap[cCIdx.plane() + 3][ui8x8Idx] = 16;
    for( ui = 1; ui < 16; ui++ )
    {
      if( !( m_aapaucChromaACCoefMap[cCIdx.plane()][ui][ui8x8Idx] & (SIGNIFICANT|CODED) ) && m_apaucScanPosMap[cCIdx.plane() + 3][ui8x8Idx] == 16 )
      {
        m_apaucScanPosMap[cCIdx.plane() + 3][ui8x8Idx] = ui;
      }
    }
    m_apaucChromaACBlockMap[cCIdx.plane()][ui8x8Idx] = CLEAR;
  }

  //pcMbDataAccessBase->getMbData().setMbCbp( 0 );
  pcMbDataAccessBase->getMbTCoeffs().clear();
  pcMbDataAccessBase->getMbData().setTransformSize8x8( false );

  IntYuvMbBuffer cZeroBuffer;
  cZeroBuffer.setAllSamplesToZero();
  RNOK( m_pcBaseLayerSbb->getFullPelYuvBuffer()->loadBuffer( &cZeroBuffer ) );

  return Err::m_nOK;
}


H264AVC_NAMESPACE_END


