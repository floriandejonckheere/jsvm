
#if !defined(AFX_MOTIONESTIMATIONCOST_H__46CFF00D_F656_4EA9_B8F8_81CC5610D443__INCLUDED_)
#define AFX_MOTIONESTIMATIONCOST_H__46CFF00D_F656_4EA9_B8F8_81CC5610D443__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "RateDistortionIf.h"


H264AVC_NAMESPACE_BEGIN


class MotionEstimationCost
{
protected:
  MotionEstimationCost();
  virtual ~MotionEstimationCost();

  ErrVal xUninit();

  Void xGetMotionCost( Bool bSad, Int iAdd )   { m_uiCost = m_pcRateDistortionIf->getMotionCostShift( bSad ) + iAdd;  }
  ErrVal xInitRateDistortionModel( Int iSubPelSearchLimit, RateDistortionIf* pcRateDistortionIf );

  UInt xGetVerCost( UInt y ) {  return (m_uiCost * m_puiVerCost[ y << m_iCostScale ] ) >> 16;  }
  UInt xGetHorCost( UInt x ) {  return (m_uiCost * m_puiHorCost[ x << m_iCostScale] ) >> 16;  }

  UInt xGetCost( UInt b )  { return ( m_uiCost * b ) >> 16; }

  UInt xGetCost( Mv& rcMv )
  {
    return ( m_uiCost * xGetBits( rcMv.getHor(), rcMv.getVer() ) ) >> 16;
  }
  UInt xGetCost( Int x, Int y )  {  return ( m_uiCost * xGetBits( x, y ) ) >> 16;  }
  UInt xGetBits( Int x, Int y )  {  return m_puiHorCost[ x << m_iCostScale] + m_puiVerCost[ y << m_iCostScale ];  }

  Void xSetPredictor( const Mv& rcMv )
  {
    m_puiHorCost = m_puiComponentCost - rcMv.getHor();
    m_puiVerCost = m_puiComponentCost - rcMv.getVer();
  }

  Void xSetCostScale( Int iCostScale ) { m_iCostScale = iCostScale; }
  UInt getComponentBits( Int iVal );

protected:
  RateDistortionIf* m_pcRateDistortionIf;
  UInt* m_puiComponentCostOriginP;
  UInt* m_puiComponentCost;
  UInt* m_puiVerCost;
  UInt* m_puiHorCost;
  UInt  m_uiCost;
  Int   m_iCostScale;
  Int m_iSearchLimit;
};


H264AVC_NAMESPACE_END


#endif // !defined(AFX_MOTIONESTIMATIONCOST_H__46CFF00D_F656_4EA9_B8F8_81CC5610D443__INCLUDED_)
