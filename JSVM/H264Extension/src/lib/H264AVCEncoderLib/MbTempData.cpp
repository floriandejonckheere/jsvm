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

#include "MbTempData.h"

#include "H264AVCCommonLib/MbData.h"
#include "H264AVCCommonLib/YuvMbBuffer.h"


  
H264AVC_NAMESPACE_BEGIN


ErrVal IntMbTempData::init( MbDataAccess& rcMbDataAccess )
{
  m_pcMbDataAccess = new( m_pcMbDataAccess ) MbDataAccess( rcMbDataAccess, *this );
  clear();
  return Err::m_nOK;
}

ErrVal IntMbTempData::uninit()
{
  return Err::m_nOK;
}


IntMbTempData::IntMbTempData() :
m_pcMbDataAccess( NULL )
{
  m_pcMbDataAccess = NULL;  
  clear();
  
  MbData::init( this, &m_acMbMvdData[0], &m_acMbMvdData[1], &m_acMbMotionData[0], &m_acMbMotionData[1] );
}


IntMbTempData::~IntMbTempData()
{
  delete m_pcMbDataAccess;
  m_pcMbDataAccess = NULL;
}


Void IntMbTempData::clear()
{
  MbData::clear();
  YuvMbBuffer::setZero();

  MbDataStruct::clear();
  CostData::clear();
  MbTransformCoeffs::clear();
}



Void IntMbTempData::clearCost()
{
  CostData::clear();
}



Void IntMbTempData::copyTo( MbDataAccess& rcMbDataAccess )
{
  rcMbDataAccess.getMbData()            .copyFrom( *this );
  rcMbDataAccess.getMbTCoeffs()         .copyFrom( *this );

  rcMbDataAccess.getMbMvdData(LIST_0)   .copyFrom( m_acMbMvdData[LIST_0] );
  rcMbDataAccess.getMbMotionData(LIST_0).copyFrom( m_acMbMotionData[LIST_0] );

  if( rcMbDataAccess.getSH().isBSlice() )
  {
    rcMbDataAccess.getMbMvdData(LIST_1)   .copyFrom( m_acMbMvdData[LIST_1] );
    rcMbDataAccess.getMbMotionData(LIST_1).copyFrom( m_acMbMotionData[LIST_1] );
  }
}


Void IntMbTempData::copyResidualDataTo( MbDataAccess& rcMbDataAccess )
{
  rcMbDataAccess.getMbData    ().setBCBP              ( getBCBP             () );
  rcMbDataAccess.getMbData    ().setMbExtCbp          ( getMbExtCbp         () );
  rcMbDataAccess.getMbData    ().setQp                ( getQp               () );
  rcMbDataAccess.getMbData    ().setQp4LF             ( getQp4LF            () );
  rcMbDataAccess.getMbTCoeffs ().copyFrom             ( *this                  );
  rcMbDataAccess.getMbData    ().setTransformSize8x8  ( isTransformSize8x8  () );
  rcMbDataAccess.getMbData    ().setResidualPredFlag  ( getResidualPredFlag () );
}


Void IntMbTempData::loadChromaData( IntMbTempData& rcMbTempData )
{
  ::memcpy( get(CIdx(0)), rcMbTempData.get(CIdx(0)), sizeof(TCoeff)*128);
  setChromaPredMode( rcMbTempData.getChromaPredMode() );
  YuvMbBuffer::loadChroma( rcMbTempData );
  distU()  = rcMbTempData.distU();
  distV()  = rcMbTempData.distV();
  getTempYuvMbBuffer().loadChroma( rcMbTempData.getTempYuvMbBuffer() );
}


H264AVC_NAMESPACE_END
