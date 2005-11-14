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




#if !defined(AFX_UVLCREADER_H__EA98D347_89D5_4D2D_B6D5_FB3A374CD295__INCLUDED_)
#define AFX_UVLCREADER_H__EA98D347_89D5_4D2D_B6D5_FB3A374CD295__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "MbSymbolReadIf.h"
#include "H264AVCCommonLib/Quantizer.h"

H264AVC_NAMESPACE_BEGIN

class BitReadBuffer;


class UvlcReader
: public HeaderSymbolReadIf
, public MbSymbolReadIf
, public Quantizer

{
public:
  typedef struct
  {
    UChar nVal;
    UChar nSize;
  }Vlc;

protected:
	UvlcReader();
	virtual ~UvlcReader();

public:
  static ErrVal create( UvlcReader*& rpcUvlcReader );
  ErrVal destroy();

  ErrVal init   ( BitReadBuffer* pcBitReadBuffer );
  ErrVal uninit ();

  Bool    moreRBSPData();
  ErrVal  getUvlc     ( UInt& ruiCode,                Char* pcTraceString );
  ErrVal  getCode     ( UInt& ruiCode, UInt uiLength, Char* pcTraceString );
  ErrVal  getSvlc     ( Int&  riCode,                 Char* pcTraceString );
  ErrVal  getFlag     ( Bool& rbFlag,                 Char* pcTraceString );
  ErrVal  readByteAlign();

  ErrVal  codeFromBitstream2Di( const UInt* auiCode, const UInt* auiLen, UInt uiWidth, UInt uiHeight, UInt& uiVal1, UInt& uiVal2 )
    {return xCodeFromBitstream2Di(auiCode, auiLen, uiWidth, uiHeight, uiVal1, uiVal2);};
  ErrVal  blFlag      ( MbDataAccess& rcMbDataAccess );
  Bool    isMbSkipped ( MbDataAccess& rcMbDataAccess );
  Bool    isBLSkipped ( MbDataAccess& rcMbDataAccess );
  Bool    isBLQRef    ( MbDataAccess& rcMbDataAccess );
  Bool    isEndOfSlice();
  ErrVal  blockModes  ( MbDataAccess& rcMbDataAccess );
  ErrVal  mbMode      ( MbDataAccess& rcMbDataAccess );
  ErrVal  resPredFlag ( MbDataAccess& rcMbDataAccess );

  ErrVal  mvdQPel( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx                      );
  ErrVal  mvdQPel( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  );
  ErrVal  mvdQPel( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  );
  ErrVal  mvdQPel( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  );
  
  ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx );
  ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  );
  ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  );
  ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  );
  ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx8x4 eSParIdx );
  ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx4x8 eSParIdx );
  ErrVal  mvd( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx, SParIdx4x4 eSParIdx );

  ErrVal  cbp( MbDataAccess& rcMbDataAccess );
  ErrVal  refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx );
  ErrVal  refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  );
  ErrVal  refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  );
  ErrVal  refFrame( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  );

  ErrVal  motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx );
  ErrVal  motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx16x8 eParIdx  );
  ErrVal  motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x16 eParIdx  );
  ErrVal  motionPredFlag( MbDataAccess& rcMbDataAccess, ListIdx eLstIdx, ParIdx8x8  eParIdx  );

  ErrVal  residualBlock( MbDataAccess& rcMbDataAccess, LumaIdx   cIdx, ResidualMode eResidualMode, UInt& ruiMbExtCbp);
  ErrVal  residualBlock( MbDataAccess& rcMbDataAccess, ChromaIdx cIdx, ResidualMode eResidualMode );

  ErrVal  deltaQp             ( MbDataAccess& rcMbDataAccess );
  ErrVal  intraPredModeLuma   ( MbDataAccess& rcMbDataAccess, LumaIdx cIdx );
  ErrVal  intraPredModeChroma ( MbDataAccess& rcMbDataAccess );
  ErrVal  samplesPCM          ( MbDataAccess& rcMbDataAccess );

  ErrVal  startSlice          ( const SliceHeader& rcSliceHeader );
  ErrVal  finishSlice         ( const SliceHeader& rcSliceHeader );
  
  ErrVal  transformSize8x8Flag( MbDataAccess& rcMbDataAccess);
  ErrVal  residualBlock8x8    ( MbDataAccess& rcMbDataAccess, B8x8Idx cIdx, ResidualMode eResidualMode, UInt& ruiMbExtCbp);
  ErrVal  intraPredModeLuma8x8( MbDataAccess& rcMbDataAccess, B8x8Idx cIdx ) {return intraPredModeLuma( rcMbDataAccess, cIdx );}
private:
  ErrVal xGetFlag     ( UInt& ruiCode );
  ErrVal xGetCode     ( UInt& ruiCode, UInt uiLength );
  ErrVal xGetUvlcCode ( UInt& ruiVal  );
  ErrVal xGetSvlcCode ( Int&  riVal   );
  ErrVal xGetRefFrame ( Bool bWriteBit, UInt& uiRefFrame );
  ErrVal xGetMotionPredFlag( Bool& rbFlag );
  ErrVal xGetMvd      ( Mv& cMv );
  ErrVal xGetMvdQPel  ( MbDataAccess& rcMbDataAccess, Mv& cMv );
  ErrVal xGetMvdComponentQPel( Short& sMvdComp );
  ErrVal xPredictNonZeroCnt( MbDataAccess& rcMbDataAccess, LumaIdx cIdx, UInt& uiCoeffCount, UInt& uiTrailingOnes );
  ErrVal xPredictNonZeroCnt( MbDataAccess& rcMbDataAccess, ChromaIdx cIdx, UInt& uiCoeffCount, UInt& uiTrailingOnes );
  ErrVal xGetTrailingOnes16( UInt uiLastCoeffCount, UInt& uiCoeffCount, UInt& uiTrailingOnes );
  ErrVal xCodeFromBitstream2D( const UChar* aucCode, const UChar* aucLen, UInt uiWidth, UInt uiHeight, UInt& uiVal1, UInt& uiVal2 );
  ErrVal xCodeFromBitstream2Di( const UInt* auiCode, const UInt* auiLen, UInt uiWidth, UInt uiHeight, UInt& uiVal1, UInt& uiVal2 );
  ErrVal xGetRunLevel( Int* aiLevelRun, UInt uiCoeffCnt, UInt uiTrailingOnes, UInt uiMaxCoeffs, UInt& uiTotalRun );
  ErrVal xGetLevelVLC0( Int& iLevel );
  ErrVal xGetLevelVLCN( Int& iLevel, UInt uiVlcLength );
  ErrVal xGetRun( UInt uiVlcPos, UInt& uiRun  );
  ErrVal xGetTotalRun16( UInt uiVlcPos, UInt& uiTotalRun );
  ErrVal xGetTotalRun4( UInt& uiVlcPos, UInt& uiTotalRun );
  ErrVal xGetTrailingOnes4( UInt& uiCoeffCount, UInt& uiTrailingOnes );

protected:
  BitReadBuffer*  m_pcBitReadBuffer;
  UInt            m_uiBitCounter;
  UInt            m_uiPosCounter;
  Bool            m_bRunLengthCoding;
  UInt            m_uiRun;
};

H264AVC_NAMESPACE_END

#endif // !defined(AFX_UVLCREADER_H__EA98D347_89D5_4D2D_B6D5_FB3A374CD295__INCLUDED_)
