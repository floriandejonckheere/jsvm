
#include "H264AVCEncoderLib.h"

#include "MotionEstimationCost.h"


H264AVC_NAMESPACE_BEGIN


MotionEstimationCost::MotionEstimationCost():
  m_pcRateDistortionIf      ( NULL ),
  m_puiComponentCostOriginP ( NULL ),
  m_puiComponentCost        ( NULL ),
  m_puiVerCost              ( NULL ),
  m_puiHorCost              ( NULL ),
  m_uiCost                  ( 0 ),
  m_iCostScale              ( 0 ),
  m_iSearchLimit            ( 0xdeaddead )
{
}

MotionEstimationCost::~MotionEstimationCost()
{
}


ErrVal MotionEstimationCost::xUninit()
{

  if( NULL != m_puiComponentCostOriginP )
  {
    delete [] m_puiComponentCostOriginP;
    m_puiComponentCostOriginP = NULL;
  }


  return Err::m_nOK;
}

UInt MotionEstimationCost::getComponentBits( Int iVal )
{
  UInt uiLength = 1;
  UInt uiTemp = ( iVal <= 0) ? (-iVal<<1)+1: (iVal<<1);


  AOF_DBG( uiTemp );

  while( 1 != uiTemp )
  {
    uiTemp >>= 1;
    uiLength += 2;
  }

  return uiLength;
}


ErrVal MotionEstimationCost::xInitRateDistortionModel( Int iSubPelSearchLimit, RateDistortionIf* pcRateDistortionIf )
{
  ROT( NULL == pcRateDistortionIf )
  m_pcRateDistortionIf = pcRateDistortionIf;

  // make it larger
  iSubPelSearchLimit += 4;
  iSubPelSearchLimit *= 8;

  if( m_iSearchLimit != iSubPelSearchLimit )
  {
    RNOK( xUninit() )

    m_iSearchLimit = iSubPelSearchLimit;

    m_puiComponentCostOriginP = new UInt[ 4 * iSubPelSearchLimit ];
    iSubPelSearchLimit *= 2;

    m_puiComponentCost = m_puiComponentCostOriginP + iSubPelSearchLimit;

    for( Int n = -iSubPelSearchLimit; n < iSubPelSearchLimit; n++)
    {
      m_puiComponentCost[n] = getComponentBits( n );
    }
  }

  return Err::m_nOK;
}


H264AVC_NAMESPACE_END
