
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

class H264AVCCOMMONLIB_API MbData :
public MbDataStruct
{
public:
  MbData()
  : m_pcMbTCoeffs( 0 )
  {
    m_apcMbMvdData   [ LIST_0 ] = NULL;
    m_apcMbMvdData   [ LIST_1 ] = NULL;
    m_apcMbMotionData[ LIST_0 ] = NULL;
    m_apcMbMotionData[ LIST_1 ] = NULL;
    m_aabBaseIntra   [ 0 ][ 0 ] = false;
    m_aabBaseIntra   [ 0 ][ 1 ] = false;
    m_aabBaseIntra   [ 1 ][ 0 ] = false;
    m_aabBaseIntra   [ 1 ][ 1 ] = false;
  }
  ~MbData()
  {
  }

  UInt calcMbCbp ( UInt uiStart, UInt uiStop ) const;
  Bool calcBCBP  ( UInt uiStart, UInt uiStop, UInt uiPos ) const;

  Void init(  MbTransformCoeffs*  pcMbTCoeffs,
              MbMvData*           pcMbMvdDataList0,
              MbMvData*           pcMbMvdDataList1,
              MbMotionData*       pcMbMotionDataList0,
              MbMotionData*       pcMbMotionDataList1 )
  {
    m_pcMbTCoeffs           = pcMbTCoeffs;
    m_apcMbMvdData[0]       = pcMbMvdDataList0;
    m_apcMbMvdData[1]       = pcMbMvdDataList1;
    m_apcMbMotionData[0]    = pcMbMotionDataList0;
    m_apcMbMotionData[1]    = pcMbMotionDataList1;
  }

public:
  MbTransformCoeffs&        getMbTCoeffs    ()                          { return *m_pcMbTCoeffs; }
  MbMvData&                 getMbMvdData    ( ListIdx eListIdx )        { return *m_apcMbMvdData   [ eListIdx ]; }
  MbMotionData&             getMbMotionData ( ListIdx eListIdx )        { return *m_apcMbMotionData[ eListIdx ]; }

  const MbTransformCoeffs&  getMbTCoeffs    ()                    const { return *m_pcMbTCoeffs; }
  const MbMvData&           getMbMvdData    ( ListIdx eListIdx )  const { return *m_apcMbMvdData   [ eListIdx ]; }
  const MbMotionData&       getMbMotionData ( ListIdx eListIdx )  const { return *m_apcMbMotionData[ eListIdx ]; }

  operator MbTransformCoeffs& ()                                        { return *m_pcMbTCoeffs; }

  Bool    isBaseIntra(Int iPosX, Int iPosY) const { return m_aabBaseIntra[iPosX][iPosY]; }
  Void    setBaseIntra( Int iPosX, Int iPosY, Bool b ) { m_aabBaseIntra[iPosX][iPosY] = b; }

  Void    copy( const MbData& rcMbData );
  ErrVal  loadAll( FILE* pFile );
  ErrVal  saveAll( FILE* pFile );

  ErrVal  copyMotion    ( const MbData& rcMbData, UInt uiSliceId = MSYS_UINT_MAX );

  // functions for SVC to AVC rewrite
  ErrVal  copyTCoeffs   ( MbData& rcMbData );
  ErrVal  copyIntraPred ( MbData& rcMbData );

protected:
  MbTransformCoeffs*  m_pcMbTCoeffs;
  MbMvData*           m_apcMbMvdData[2];
  MbMotionData*       m_apcMbMotionData[2];
  Bool                m_aabBaseIntra  [2][2];
};


class MbDataBuffer : public MbData
{
public :
  MbDataBuffer()
  {
    MbData::init( &m_cMbTransformCoeffs, m_acMbMvData, m_acMbMvData+1, m_acMbMotionData, m_acMbMotionData + 1);
  }
  virtual ~MbDataBuffer() {}

  MbTransformCoeffs m_cMbTransformCoeffs;
  MbMvData          m_acMbMvData[2];
  MbMotionData      m_acMbMotionData[2];
};


H264AVC_NAMESPACE_END


#endif // !defined(AFX_MBDATA_H__6F1A2BEC_47BC_4944_BE36_C0E96ED39557__INCLUDED_)
