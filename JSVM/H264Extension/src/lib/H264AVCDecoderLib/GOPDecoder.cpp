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




#include "H264AVCDecoderLib.h"
#include "GOPDecoder.h"
#include "SliceReader.h"
#include "SliceDecoder.h"
#include "FGSSubbandDecoder.h"
#include "CreaterH264AVCDecoder.h"
#include "ControlMngH264AVCDecoder.h"
#include "H264AVCCommonLib/TraceFile.h"
#include <math.h>

#include "H264AVCCommonLib/CFMO.h"


H264AVC_NAMESPACE_BEGIN


__inline Void printSpaces( UInt uiNum )
{
  while( uiNum-- ) printf(" ");
}




//////////////////////////////////////////////////////////////////////////
// DPB UNIT
//////////////////////////////////////////////////////////////////////////
DPBUnit::DPBUnit()
: m_iPoc                ( MSYS_INT_MIN )
, m_uiFrameNum          ( MSYS_UINT_MAX )
, m_uiTemporalLevel     ( MSYS_UINT_MAX )
, m_bKeyPicture         ( false )
, m_bExisting           ( false )
, m_bNeededForReference ( false )
, m_bOutputted          ( false )
, m_bBaseRepresentation ( false )
, m_pcFrame             ( 0 )
, m_cControlData        ()
, m_bConstrainedIntraPred( false )
{
}



DPBUnit::~DPBUnit()
{
  MbDataCtrl*   pcMbDataCtrl  = m_cControlData.getMbDataCtrl  ();
  SliceHeader*  pcSliceHeader = m_cControlData.getSliceHeader ();
  m_cControlData.getMbDataCtrl()->uninitFgsBQData(); 
  if( pcMbDataCtrl )
  {
    pcMbDataCtrl->uninit();
  }
  delete pcMbDataCtrl;
  delete pcSliceHeader;
  if( m_pcFrame )
  {
    m_pcFrame->uninit();
  }
  delete m_pcFrame;
}



ErrVal
DPBUnit::create( DPBUnit*&                    rpcDPBUnit,
                 YuvBufferCtrl&               rcYuvBufferCtrl,
                 const SequenceParameterSet&  rcSPS )
{
  rpcDPBUnit = new DPBUnit();
  ROF( rpcDPBUnit );

  MbDataCtrl* pcMbDataCtrl = 0;
  ROFRS( ( rpcDPBUnit->m_pcFrame    = new IntFrame  ( rcYuvBufferCtrl, rcYuvBufferCtrl ) ), Err::m_nERR );
  ROFRS( ( pcMbDataCtrl             = new MbDataCtrl()                                   ), Err::m_nERR );
  RNOK (   rpcDPBUnit->m_pcFrame    ->init          ()               );
           rpcDPBUnit->m_pcFrame    ->setDPBUnit    ( rpcDPBUnit   );
  RNOK (   pcMbDataCtrl             ->init          ( rcSPS        ) );
  RNOK (   rpcDPBUnit->m_cControlData.setMbDataCtrl ( pcMbDataCtrl ) );
  RNOK  (  rpcDPBUnit->m_cControlData.getMbDataCtrl()->initFgsBQData   ( pcMbDataCtrl->getSize() ) );

  return Err::m_nOK;
}



ErrVal
DPBUnit::destroy()
{
  delete this;
  return Err::m_nOK;
}



ErrVal
DPBUnit::init( Int  iPoc,
               UInt uiFrameNum,
               UInt uiTemporalLevel,
               Bool bKeyPicture,
               Bool bNeededForReference,
               Bool bConstrainedIPred )
{
  m_iPoc                = iPoc;
  m_uiFrameNum          = uiFrameNum;
  m_uiTemporalLevel     = uiTemporalLevel;
  m_bKeyPicture         = bKeyPicture;
  m_bExisting           = true;
  m_bNeededForReference = bNeededForReference;
  m_bOutputted          = false;
  m_bBaseRepresentation = false;
  m_bConstrainedIntraPred = bConstrainedIPred;
  return Err::m_nOK;
}



ErrVal
DPBUnit::initNonEx( Int   iPoc,
                    UInt  uiFrameNum )
{
  m_iPoc                = iPoc;
  m_uiFrameNum          = uiFrameNum;
  m_uiTemporalLevel     = MSYS_UINT_MAX;
  m_bKeyPicture         = false;
  m_bExisting           = false;
  m_bNeededForReference = true;
  m_bOutputted          = false;
  m_bBaseRepresentation = false;
  m_bConstrainedIntraPred = false;
  return Err::m_nOK;
}



ErrVal
DPBUnit::initBase( DPBUnit&   rcDPBUnit,
                   IntFrame*  pcFrameBaseRep )
{
  ROT( rcDPBUnit.m_bBaseRepresentation );
  m_iPoc                = rcDPBUnit.m_iPoc;
  m_uiFrameNum          = rcDPBUnit.m_uiFrameNum;
  m_uiTemporalLevel     = rcDPBUnit.m_uiTemporalLevel;
  m_bKeyPicture         = rcDPBUnit.m_bKeyPicture;
  m_bExisting           = rcDPBUnit.m_bExisting;
  m_bNeededForReference = rcDPBUnit.m_bNeededForReference;
  m_bOutputted          = rcDPBUnit.m_bOutputted;
  m_bConstrainedIntraPred = rcDPBUnit.m_bConstrainedIntraPred;
  m_bBaseRepresentation = true;
  RNOK( m_pcFrame->copyAll( pcFrameBaseRep ) );
  m_pcFrame->setPOC( m_iPoc );
  return Err::m_nOK;
}



ErrVal
DPBUnit::uninit()
{
  m_iPoc                = MSYS_INT_MIN;
  m_uiFrameNum          = MSYS_UINT_MAX;
  m_uiTemporalLevel     = MSYS_UINT_MAX;
  m_bKeyPicture         = false;
  m_bExisting           = false;
  m_bNeededForReference = false;
  m_bOutputted          = false;
  m_bBaseRepresentation = false;
  m_bConstrainedIntraPred = false;
  return Err::m_nOK;
}



ErrVal
DPBUnit::markNonRef()
{
  ROF( m_bNeededForReference );
  m_bNeededForReference = false;
  return Err::m_nOK;
}



ErrVal
DPBUnit::markOutputted()
{
  ROT( m_bOutputted );
  m_bOutputted = true;
  return Err::m_nOK;
}





//////////////////////////////////////////////////////////////////////////
// DECODED PICTURE BUFFER
//////////////////////////////////////////////////////////////////////////
DecodedPicBuffer::DecodedPicBuffer()
: m_bInitDone         ( false )
, m_pcYuvBufferCtrl   ( 0 )
, m_uiNumRefFrames    ( 0 )
, m_uiMaxFrameNum     ( 0 )
, m_uiLastRefFrameNum ( MSYS_UINT_MAX )
, m_pcCurrDPBUnit     ( 0 )
{
}



DecodedPicBuffer::~DecodedPicBuffer()
{
}



ErrVal
DecodedPicBuffer::create( DecodedPicBuffer*& rpcDecodedPicBuffer )
{
  rpcDecodedPicBuffer = new DecodedPicBuffer;
  ROT( NULL == rpcDecodedPicBuffer );
  return Err::m_nOK;
}



ErrVal
DecodedPicBuffer::destroy()
{
  ROT( m_bInitDone );
  delete this;
  return Err::m_nOK;
}



ErrVal
DecodedPicBuffer::init( YuvBufferCtrl* pcYuvBufferCtrl,
                        UInt           uiLayer )
{
  ROT( m_bInitDone );
  ROF( pcYuvBufferCtrl );

  m_pcYuvBufferCtrl   = pcYuvBufferCtrl;
  m_uiNumRefFrames    = 0;
  m_uiMaxFrameNum     = 0;
  m_uiLastRefFrameNum = MSYS_UINT_MAX;
  m_uiLayer           = uiLayer;
  m_bInitDone         = true;
  
  return Err::m_nOK;
}



__inline UInt downround2powerof2( UInt i ) { UInt r = 1; for( ; (UInt)( 1 << r ) <= i; r++ ); return ( 1 << ( r - 1 ) ); }



ErrVal
DecodedPicBuffer::initSPS( const SequenceParameterSet& rcSPS )
{
  ROF( m_bInitDone );
	UInt  uiMaxPicsInDPB  = downround2powerof2( rcSPS.getMaxDPBSize() ) + 5; // up to 3 base representations + 2 extra (should use "real DPB" size in future)
  RNOK( xCreateData( uiMaxPicsInDPB, rcSPS ) );
  m_uiNumRefFrames      = rcSPS.getNumRefFrames();
  m_uiMaxFrameNum       = ( 1 << ( rcSPS.getLog2MaxFrameNum() ) );
  
  return Err::m_nOK;
}



ErrVal
DecodedPicBuffer::uninit()
{
  ROF( m_bInitDone );

  RNOK( xDeleteData() );
  ROF ( m_cPicBufferList.empty() );
  
  m_pcYuvBufferCtrl   = 0;
  m_uiNumRefFrames    = 0;
  m_uiMaxFrameNum     = 0;
  m_uiLastRefFrameNum = MSYS_UINT_MAX;
  m_bInitDone         = false;
  return Err::m_nOK;
}



ErrVal
DecodedPicBuffer::xCreateData( UInt                         uiMaxPicsInDPB,
                               const SequenceParameterSet&  rcSPS )
{
  ROF( m_bInitDone );
  RNOK( xDeleteData() );

  while( uiMaxPicsInDPB-- )
  {
    DPBUnit*  pcDPBUnit = 0;
    RNOK( DPBUnit::create( pcDPBUnit, *m_pcYuvBufferCtrl, rcSPS ) );
    m_cFreeDPBUnitList.pushBack( pcDPBUnit );
  }
  RNOK( DPBUnit::create( m_pcCurrDPBUnit, *m_pcYuvBufferCtrl, rcSPS ) );
  RNOK( m_pcCurrDPBUnit->uninit() );
  
  return Err::m_nOK;
}



ErrVal
DecodedPicBuffer::xDeleteData()
{
  ROF( m_bInitDone );

  m_cFreeDPBUnitList += m_cUsedDPBUnitList;
  m_cUsedDPBUnitList.clear();

  while( m_cFreeDPBUnitList.size() )
  {
    DPBUnit*  pcDPBUnit = m_cFreeDPBUnitList.popFront();
    pcDPBUnit->destroy();
  }
  if( m_pcCurrDPBUnit )
  {
    m_pcCurrDPBUnit->destroy();
    m_pcCurrDPBUnit = 0;
  }
  return Err::m_nOK;
}



ErrVal
DecodedPicBuffer::xClearBuffer()
{

	//===== remove non-output/non-ref pictures =====
  //--- store in temporary list ---
  DPBUnitList cTempList;
  DPBUnitList::iterator iter  = m_cUsedDPBUnitList.begin();
  DPBUnitList::iterator end   = m_cUsedDPBUnitList.end  ();
  for( ; iter != end; iter++ )
  {
    Bool  bNoOutput = ( !(*iter)->isExisting() || (*iter)->isBaseRep() || (*iter)->isOutputted() );
    Bool  bNonRef   = ( !(*iter)->isNeededForRef() );
		if( bNonRef && bNoOutput )
    {
      cTempList.push_back( *iter );
    }
  }
  //--- move to free list ---
  while( cTempList.size() )
  {
    DPBUnit*  pcDPBUnit = cTempList.popFront();
    RNOK( pcDPBUnit->uninit() );
    m_cUsedDPBUnitList.remove( pcDPBUnit );
    m_cFreeDPBUnitList.push_back( pcDPBUnit );
  }

  return Err::m_nOK;
}



ErrVal
DecodedPicBuffer::xUpdateMemory( SliceHeader* pcSliceHeader )
{
  ROTRS( pcSliceHeader && pcSliceHeader->getNalRefIdc() == NAL_REF_IDC_PRIORITY_LOWEST, Err::m_nOK );

  if( pcSliceHeader && pcSliceHeader->getAdaptiveRefPicBufferingFlag() )
  {
    RNOK( xMMCO( pcSliceHeader ) );
  }
  else
  {
    RNOK( xSlidingWindow() );
  }

  //===== clear buffer =====
  RNOK( xClearBuffer() );

  return Err::m_nOK;
}


ErrVal
DecodedPicBuffer::xMMCO( SliceHeader* pcSliceHeader )
{
  ROF( pcSliceHeader );

  MmcoOp            eMmcoOp;
  const MmcoBuffer& rcMmcoBuffer  = pcSliceHeader->getMmcoBuffer();
  Int               iIndex        = 0;
  UInt              uiVal1, uiVal2;

  while( MMCO_END != ( eMmcoOp = rcMmcoBuffer.get( iIndex++ ).getCommand( uiVal1, uiVal2 ) ) )
  {
    ErrVal nRet = Err::m_nOK;

    switch( eMmcoOp )
    {
    case MMCO_SHORT_TERM_UNUSED:
      RNOK( xMarkShortTermUnused( m_pcCurrDPBUnit, uiVal1 ) );
      break;
    case MMCO_RESET:
    case MMCO_MAX_LONG_TERM_IDX:
    case MMCO_ASSIGN_LONG_TERM:
    case MMCO_LONG_TERM_UNUSED:
    case MMCO_SET_LONG_TERM:
    default:
      fprintf( stderr,"\nERROR: MMCO COMMAND currently not supported in the software\n\n" );
      ROT(1);
    }
  }

  return Err::m_nOK;
}


ErrVal
DecodedPicBuffer::xMarkShortTermUnused( DPBUnit* pcCurrentDPBUnit, UInt uiDiffOfPicNums )
{
  ROF( pcCurrentDPBUnit );

  UInt uiCurrPicNum = pcCurrentDPBUnit->getFrameNum();
  Int  iPicNumN     = (Int)uiCurrPicNum - (Int)uiDiffOfPicNums - 1;

  DPBUnit*              pcDPBUnit = 0;
  DPBUnitList::iterator iter      = m_cUsedDPBUnitList.begin();
  DPBUnitList::iterator end       = m_cUsedDPBUnitList.end  ();
  for( ; iter != end; iter++ )
  {
    if( (*iter)->isNeededForRef() && (*iter)->getPicNum(uiCurrPicNum,m_uiMaxFrameNum) == iPicNumN )
    {
      (*iter)->markNonRef();
#if 1 // BUG_FIX -> also mark "base representation as unused for ref."
#else
      return Err::m_nOK;
#endif
    }
  }
#if 1 // BUG_FIX
#else
  ROT(1);
#endif
  return Err::m_nOK;
}


ErrVal
DecodedPicBuffer::xSlidingWindow()
{
  //===== get number of reference frames =====
  UInt uiCurrNumRefFrames     = 0;
  DPBUnitList::iterator iter  = m_cUsedDPBUnitList.begin();
  DPBUnitList::iterator end   = m_cUsedDPBUnitList.end  ();
  for( ; iter != end; iter++ )
  {
    if( (*iter)->isNeededForRef() && !(*iter)->isBaseRep() )
    {
      uiCurrNumRefFrames++;
    }
  }
  ROTRS( uiCurrNumRefFrames <= m_uiNumRefFrames, Err::m_nOK );

  //===== sliding window reference picture update =====
  //--- look for last ref frame that shall be removed ---
  UInt uiRefFramesToRemove = uiCurrNumRefFrames - m_uiNumRefFrames;
  iter                     = m_cUsedDPBUnitList.begin();
  for( ; iter != end; iter++ )
  {
    if( (*iter)->isNeededForRef() && !(*iter)->isBaseRep() )
    {
      uiRefFramesToRemove--;
      if( uiRefFramesToRemove == 0 )
      {
        break;
      }
    }
  }
  ROT( uiRefFramesToRemove );
  //--- delete reference label ---
  end   = ++iter;
  iter  = m_cUsedDPBUnitList.begin();
  for( ; iter != end; iter++ )
  {
    if( (*iter)->isNeededForRef() )
    {
      RNOK( (*iter)->markNonRef() );
    }
  }

  return Err::m_nOK;
}



ErrVal
DecodedPicBuffer::xOutput( PicBufferList&   rcOutputList,
                           PicBufferList&   rcUnusedList )
{
  ROTRS( m_cFreeDPBUnitList.size(), Err::m_nOK ); // no need for output
  
  //===== smallest non-ref/output poc value =====
  Int       iMinOutputPoc   = MSYS_INT_MAX;
  DPBUnit*  pcElemToRemove  = 0;
  DPBUnitList::iterator iter =  m_cUsedDPBUnitList.begin();
  DPBUnitList::iterator end  =  m_cUsedDPBUnitList.end  ();
  for( ; iter != end; iter++ )
  {
    Bool bOutput = ( !(*iter)->isOutputted() && (*iter)->isExisting() && !(*iter)->isBaseRep() && !(*iter)->isNeededForRef() );
    if(  bOutput && (*iter)->getPoc() < iMinOutputPoc )
    {
      iMinOutputPoc   = (*iter)->getPoc();
      pcElemToRemove  = (*iter);
    }
  }
  ROF( pcElemToRemove ); // error, nothing can be removed

  //===== copy all output elements to temporary list =====
  DPBUnitList cOutputList;
  Int iMaxPoc = iMinOutputPoc;
  Int iMinPoc = MSYS_INT_MAX;
  iter        = m_cUsedDPBUnitList.begin();
  for( ; iter != end; iter++ )
  {
    Bool bOutput = ( (*iter)->getPoc() <= iMinOutputPoc && !(*iter)->isOutputted() );
    if( bOutput )
    {
      if( !(*iter)->isOutputted() )
      {
        RNOK( (*iter)->markOutputted() );
      }
      if( (*iter)->isExisting() && !(*iter)->isBaseRep() )
      {
        cOutputList.push_back( *iter );
        if( (*iter)->getPoc() < iMinPoc )
        {
          iMinPoc = (*iter)->getPoc();
        }
      }
    }
  }

  //===== real output =====
  for( Int iPoc = iMinPoc; iPoc <= iMaxPoc; iPoc++ )
  {
    iter = cOutputList.begin();
    end  = cOutputList.end  ();
    for( ; iter != end; iter++ )
    {
      if( (*iter)->getPoc() == iPoc )
      {
        DPBUnit* pcDPBUnit = *iter;
        cOutputList.remove( pcDPBUnit );

        //----- output -----
        ROT( m_cPicBufferList.empty() );
        PicBuffer*  pcPicBuffer = m_cPicBufferList.popFront();
        pcDPBUnit->getFrame()->store( pcPicBuffer );
        rcOutputList.push_back( pcPicBuffer );
        rcUnusedList.push_back( pcPicBuffer );
        break; // only one picture per Poc
      }
    }
  }
  ROT( cOutputList.size() );

  //===== clear buffer =====
  RNOK( xClearBuffer() );

  //===== check =====
  ROT( m_cFreeDPBUnitList.empty() ); // this should never happen

  return Err::m_nOK;
}



ErrVal
DecodedPicBuffer::xClearOutputAll( PicBufferList&   rcOutputList,
                                   PicBufferList&   rcUnusedList,
                                   Int&             riMaxPoc )
{
  //===== create output list =====
  DPBUnitList cOutputList;
  Int         iMinPoc = MSYS_INT_MAX;
  Int         iMaxPoc = MSYS_INT_MIN;
  DPBUnitList::iterator iter =  m_cUsedDPBUnitList.begin();
  DPBUnitList::iterator end  =  m_cUsedDPBUnitList.end  ();
  for( ; iter != end; iter++ )  
  {
    Bool bOutput = ( !(*iter)->isOutputted() && (*iter)->isExisting() && !(*iter)->isBaseRep() );
    if(  bOutput )
    {
      cOutputList.push_back( *iter );
      if( (*iter)->getPoc() < iMinPoc ) iMinPoc = (*iter)->getPoc();
      if( (*iter)->getPoc() > iMaxPoc ) iMaxPoc = (*iter)->getPoc();
    }
  }

  //===== real output =====
  for( Int iPoc = iMinPoc; iPoc <= iMaxPoc; iPoc++ )
  {
    iter = cOutputList.begin();
    end  = cOutputList.end  ();
    for( ; iter != end; iter++ )
    {
      if( (*iter)->getPoc() == iPoc )
      {
        DPBUnit* pcDPBUnit = *iter;
        cOutputList.remove( pcDPBUnit );

        //----- output -----
        ROT( m_cPicBufferList.empty() );
        PicBuffer*  pcPicBuffer = m_cPicBufferList.popFront();
        pcDPBUnit->getFrame()->store( pcPicBuffer );
        rcOutputList.push_back( pcPicBuffer );
        rcUnusedList.push_back( pcPicBuffer );
        if( iPoc > riMaxPoc )
        {
          riMaxPoc = iPoc;
        }
        break; // only one picture per Poc
      }
    }
  }
  ROT( cOutputList.size() );

  //===== uninit all elements and move to free list =====
  while( m_cUsedDPBUnitList.size() )
  {
    DPBUnit* pcDPBUnit = m_cUsedDPBUnitList.popFront();
    RNOK( pcDPBUnit->uninit() );
    m_cFreeDPBUnitList.push_back( pcDPBUnit );
  }

  return Err::m_nOK;
}



ErrVal
DecodedPicBuffer::xStorePicture( DPBUnit*       pcDPBUnit,
                                 PicBufferList& rcOutputList,
                                 PicBufferList& rcUnusedList,
                                 Bool           bTreatAsIdr )
{
  ROF( pcDPBUnit == m_pcCurrDPBUnit ); // check

  //---- fill border ----
  RNOK( m_pcYuvBufferCtrl->initMb() );
  RNOK( pcDPBUnit->getFrame()->extendFrame( NULL ) );

  if( bTreatAsIdr )
  {
    //===== IDR pictures =====
    Int iDummy;
    RNOK( xClearOutputAll( rcOutputList, rcUnusedList, iDummy ) ); // clear and output all pictures
    m_cUsedDPBUnitList.push_back( pcDPBUnit );                                    // store current picture
  }
  else
  {
    //===== non-IDR picture =====
    m_cUsedDPBUnitList.push_back( pcDPBUnit );                                    // store current picture
    RNOK( xUpdateMemory( pcDPBUnit->getCtrlData().getSliceHeader() ) );        // memory update 
    RNOK( xOutput( rcOutputList, rcUnusedList ) );         // output
  }
  RNOK( xDumpDPB() );

  m_pcCurrDPBUnit = m_cFreeDPBUnitList.popFront();                                // new current DPB unit
  m_pcCurrDPBUnit->getCtrlData().setSliceHeader( 0 );
  m_pcCurrDPBUnit->getCtrlData().getMbDataCtrl()->reset();
  m_pcCurrDPBUnit->getCtrlData().getMbDataCtrl()->clear();

  return Err::m_nOK;
}



ErrVal
DecodedPicBuffer::xCheckMissingPics( SliceHeader*   pcSliceHeader,
                                     PicBufferList& rcOutputList,
                                     PicBufferList& rcUnusedList )
{
  ROTRS( pcSliceHeader->isIdrNalUnit(), Err::m_nOK );
  ROTRS( ( ( m_uiLastRefFrameNum + 1 ) % m_uiMaxFrameNum ) == pcSliceHeader->getFrameNum(), Err::m_nOK );

  UInt  uiMissingFrames = pcSliceHeader->getFrameNum() - m_uiLastRefFrameNum - 1;
  if( pcSliceHeader->getFrameNum() <= m_uiLastRefFrameNum )
  {
    uiMissingFrames += m_uiMaxFrameNum;
  }

  if( ! pcSliceHeader->getSPS().getRequiredFrameNumUpdateBehaviourFlag() )
  {
    printf("\nLOST PICTURES = %d\n", uiMissingFrames );
    ROT(1);
  }
  else
  {
    for( UInt uiIndex = 1; uiIndex <= uiMissingFrames; uiIndex++ )
    {
      Bool  bTreatAsIdr = ( m_cUsedDPBUnitList.empty() );
      Int   iPoc        = ( bTreatAsIdr ? 0 : m_cUsedDPBUnitList.back()->getPoc() );
      UInt  uiFrameNum  = ( m_uiLastRefFrameNum + uiIndex ) % m_uiMaxFrameNum;

      RNOK( m_pcCurrDPBUnit->initNonEx( iPoc, uiFrameNum ) );
      RNOK( xStorePicture( m_pcCurrDPBUnit, rcOutputList, rcUnusedList, bTreatAsIdr ) );
    }
  }

  m_uiLastRefFrameNum = ( m_uiLastRefFrameNum + uiMissingFrames ) % m_uiMaxFrameNum;
  return Err::m_nOK;
}



ErrVal
DecodedPicBuffer::xDumpDPB()
{
#if 1 // NO_DEBUG
  return Err::m_nOK;
#endif

  printf("\nDECODED PICTURE BUFFER (Layer %d)\n", m_uiLayer );
  DPBUnitList::iterator iter  = m_cUsedDPBUnitList.begin();
  DPBUnitList::iterator end   = m_cUsedDPBUnitList.end  ();
  for( Int iIndex = 0; iter != end; iter++, iIndex++ )
  {
    DPBUnit* p = (*iter);
    printf("\tPOS=%d:\tFN=%d\tPoc=%d\t%s\t", iIndex, p->getFrameNum(), p->getPoc(), (p->isNeededForRef()?"REF":"   ") );
    if(  p->isOutputted() )   printf("Outputted  ");
    if( !p->isExisting () )   printf("NotExisting   ");
    if(  p->isBaseRep  () )   printf("BasRep   ");
    printf("\n");
  }
  printf("\n");
  return Err::m_nOK;
}




ErrVal
DecodedPicBuffer::xInitPrdListPSlice( RefFrameList&  rcList )
{
  Bool  bBaseRep        = m_pcCurrDPBUnit->isKeyPic ();
  UInt  uiCurrFrameNum  = m_pcCurrDPBUnit->getFrameNum();

  //----- generate decreasing POC list -----
  for( Int iMaxPicNum = (Int)uiCurrFrameNum; true; )
  {
    DPBUnit*              pNext = 0;
    DPBUnitList::iterator iter  = m_cUsedDPBUnitList.begin();
    DPBUnitList::iterator end   = m_cUsedDPBUnitList.end  ();
    for( ; iter != end; iter++ )
    {
      if( (*iter)->isNeededForRef() &&
          (*iter)->getPicNum(uiCurrFrameNum,m_uiMaxFrameNum) < iMaxPicNum &&
          ( !pNext || (*iter)->getPicNum(uiCurrFrameNum,m_uiMaxFrameNum)  > pNext->getPicNum(uiCurrFrameNum,m_uiMaxFrameNum) 
                   ||((*iter)->getPicNum(uiCurrFrameNum,m_uiMaxFrameNum) == pNext->getPicNum(uiCurrFrameNum,m_uiMaxFrameNum) && (*iter)->isBaseRep() == bBaseRep) ) )
      {
        pNext = (*iter);
      }
    }
    if( !pNext )
    {
      break;
    }
    iMaxPicNum = pNext->getPicNum(uiCurrFrameNum,m_uiMaxFrameNum);
    rcList.add( pNext->getFrame() );
  }

  return Err::m_nOK;
}



ErrVal
DecodedPicBuffer::xInitPrdListsBSlice( RefFrameList&  rcList0,
                                       RefFrameList&  rcList1 )
{
  RefFrameList  cDecreasingPocList;
  RefFrameList  cIncreasingPocList;
  Bool  bBaseRep    = m_pcCurrDPBUnit->isKeyPic ();
  Int   iCurrPoc    = m_pcCurrDPBUnit->getPoc   ();

  //----- generate decreasing POC list -----
  for( Int iMaxPoc = iCurrPoc; true; )
  {
    DPBUnit*              pNext = 0;
    DPBUnitList::iterator iter  = m_cUsedDPBUnitList.begin();
    DPBUnitList::iterator end   = m_cUsedDPBUnitList.end  ();
    for( ; iter != end; iter++ )
    {
      if( (*iter)->isNeededForRef() &&
          (*iter)->getPoc() < iMaxPoc && ( !pNext || (*iter)->getPoc() > pNext->getPoc() ) &&
          (*iter)->isBaseRep() == bBaseRep )
      {
        pNext = (*iter);
      }
    }
    if( !pNext )
    {
      break;
    }
    iMaxPoc = pNext->getPoc();
    cDecreasingPocList.add( pNext->getFrame() );
  }

  //----- generate increasing POC list -----
  for( Int iMinPoc = iCurrPoc; true; )
  {
    DPBUnit*              pNext = 0;
    DPBUnitList::iterator iter  = m_cUsedDPBUnitList.begin();
    DPBUnitList::iterator end   = m_cUsedDPBUnitList.end  ();
    for( ; iter != end; iter++ )
    {
      if( (*iter)->isNeededForRef() &&
          (*iter)->getPoc() > iMinPoc && ( !pNext || (*iter)->getPoc() < pNext->getPoc() ) &&
          (*iter)->isBaseRep() == bBaseRep )
      {
        pNext = (*iter);
      }
    }
    if( !pNext )
    {
      break;
    }
    iMinPoc = pNext->getPoc();
    cIncreasingPocList.add( pNext->getFrame() );
  }

  //----- list 0 and list 1 -----
  UInt uiPos;
  for( uiPos = 0; uiPos < cDecreasingPocList.getSize(); uiPos++ )
  {
    rcList0.add( cDecreasingPocList.getEntry(uiPos) );
  }
  for( uiPos = 0; uiPos < cIncreasingPocList.getSize(); uiPos++ )
  {
    rcList0.add( cIncreasingPocList.getEntry(uiPos) );
    rcList1.add( cIncreasingPocList.getEntry(uiPos) );
  }
  for( uiPos = 0; uiPos < cDecreasingPocList.getSize(); uiPos++ )
  {
    rcList1.add( cDecreasingPocList.getEntry(uiPos) );
  }

  //----- check for element switching -----
  if( rcList1.getActive() >= 2 && rcList0.getActive() == rcList1.getActive() )
  {
    Bool bSwitch = true;
    for( UInt uiPos = 0; uiPos < rcList1.getActive(); uiPos++ )
    {
      if( rcList0.getEntry(uiPos) != rcList1.getEntry(uiPos) )
      {
        bSwitch = false;
        break;
      }
    }
    if( bSwitch )
    {
      rcList1.switchFirst();
    }
  }

  return Err::m_nOK;
}



ErrVal
DecodedPicBuffer::xPrdListRemapping ( RefFrameList&   rcList,
                                      ListIdx         eListIdx,
                                      SliceHeader*    pcSliceHeader )
{
  ROF( pcSliceHeader );
  const RplrBuffer& rcRplrBuffer = pcSliceHeader->getRplrBuffer( eListIdx );

  //===== re-odering =====
  if( rcRplrBuffer.getRefPicListReorderingFlag() )
  {
    Bool  bBaseRep          = pcSliceHeader->getKeyPictureFlag();
    UInt  uiCurrentFrameNum = pcSliceHeader->getFrameNum();
    UInt  uiPicNumPred      = pcSliceHeader->getFrameNum();
    UInt  uiIndex           = 0;
    UInt  uiCommand         = 0;
    UInt  uiIdentifier      = 0;

    while( RPLR_END != ( uiCommand = rcRplrBuffer.get(uiIndex).getCommand(uiIdentifier) ) )
    {
      IntFrame* pcFrame = 0;

      if( uiCommand == RPLR_LONG )
      {
        //===== long term index =====
        ROT(1); // long-term indices are currently not supported by the software
      }
      else
      {
        //===== short term index =====
        UInt  uiAbsDiff = uiIdentifier + 1;
        
        //----- set short-term index (pic num) -----
        if( uiCommand == RPLR_NEG )
        {
          if( uiPicNumPred < uiAbsDiff )
          {
            uiPicNumPred -= ( uiAbsDiff - m_uiMaxFrameNum );
          }
          else
          {
            uiPicNumPred -=   uiAbsDiff;
          }
        }
        else // uiCommand == RPLR_POS
        {
          if( uiPicNumPred + uiAbsDiff > m_uiMaxFrameNum - 1 )
          {
            uiPicNumPred += ( uiAbsDiff - m_uiMaxFrameNum );
          }
          else
          {
            uiPicNumPred +=   uiAbsDiff;
          }
        }
        uiIdentifier = uiPicNumPred;
        
        //----- get frame -----
        DPBUnitList::iterator iter = m_cUsedDPBUnitList.begin();
        DPBUnitList::iterator end  = m_cUsedDPBUnitList.end  ();
        for( ; iter != end; iter++ )
        {
          if( (*iter)->isNeededForRef() &&
              (*iter)->getFrameNum() == uiIdentifier &&
              (!pcFrame || (*iter)->isBaseRep() == bBaseRep ) )
          {
            pcFrame = (*iter)->getFrame();
          }
        }
        if( !pcFrame )
        {
          fprintf( stderr, "\nERROR: MISSING PICTURE !!!!\n\n");
          ROT(1);
        }
        //----- find picture in reference list -----
        UInt uiRemoveIndex = MSYS_UINT_MAX;
        for( UInt uiPos = uiIndex; uiPos < rcList.getActive(); uiPos++ ) // active is equal to size !!!
        {
          if( rcList.getEntry( uiPos ) == pcFrame )
          {
            uiRemoveIndex = uiPos;
            break;
          }
        }

        //----- reference list re-ordering -----
        rcList.setElementAndRemove( uiIndex, uiRemoveIndex, pcFrame );
        uiIndex++;
      } // short-term RPLR
    } // while command
  }

  //===== set final size =====
  ROT( pcSliceHeader->getNumRefIdxActive( eListIdx ) > rcList.getActive() );
  rcList.setActive( pcSliceHeader->getNumRefIdxActive( eListIdx ) );

  return Err::m_nOK;
}



ErrVal
DecodedPicBuffer::xDumpRefList( ListIdx       eListIdx,
                                RefFrameList& rcList )
{
#if 1 // NO_DEBUG
  return Err::m_nOK;
#endif

  printf("List %d =", eListIdx );
  for( UInt uiIndex = 1; uiIndex <= rcList.getActive(); uiIndex++ )
  {
    printf(" %d", rcList[uiIndex]->getPOC() );
  }
  printf("\n");
  return Err::m_nOK;
}



ErrVal
DecodedPicBuffer::initPicCurrDPBUnit( DPBUnit*&      rpcCurrDPBUnit,
                                   PicBuffer*&    rpcPicBuffer,
                                   Bool           bResidual,  //--TM this variable should be removed
                                   SliceHeader*   pcSliceHeader,
                                   PicBufferList& rcOutputList,
                                   PicBufferList& rcUnusedList )
{
    ROF( m_bInitDone );

  //===== insert pic buffer in list =====
  m_cPicBufferList.push_back( rpcPicBuffer );
  rpcPicBuffer = 0;

  SliceHeader* pcOldSH = m_pcCurrDPBUnit->getCtrlData().getSliceHeader();
  delete pcOldSH;
  m_pcCurrDPBUnit->getCtrlData().clear();
  m_pcCurrDPBUnit->getCtrlData().getMbDataCtrl()->reset();
  m_pcCurrDPBUnit->getCtrlData().getMbDataCtrl()->clear();

  return Err::m_nOK;
}


ErrVal
DecodedPicBuffer::initCurrDPBUnit( DPBUnit*&      rpcCurrDPBUnit,
                                   PicBuffer*&    rpcPicBuffer,
                                   SliceHeader*   pcSliceHeader,
                                   PicBufferList& rcOutputList,
                                   PicBufferList& rcUnusedList )
{
  ROF( m_bInitDone );

  
  //===== check missing pictures =====
	RNOK( xCheckMissingPics( pcSliceHeader, rcOutputList, rcUnusedList ) );
  //===== initialize current DPB unit =====
	RNOK( m_pcCurrDPBUnit->init( pcSliceHeader->getPoc(),
                               pcSliceHeader->getFrameNum(),
                               pcSliceHeader->getTemporalLevel(),
                               pcSliceHeader->getKeyPictureFlag(),
                               pcSliceHeader->getNalRefIdc() > 0,
                               pcSliceHeader->getPPS().getConstrainedIntraPredFlag() ) );
  ROT( pcSliceHeader->getKeyPictureFlag() && !pcSliceHeader->getNalRefIdc() ); // just a check
  m_pcCurrDPBUnit->getFrame()->setPOC       ( pcSliceHeader->getPoc() );
  m_pcCurrDPBUnit->getCtrlData().setSliceHeader( pcSliceHeader );

  //===== set DPB unit =====
  rpcCurrDPBUnit = m_pcCurrDPBUnit;
  return Err::m_nOK;
}



ErrVal
DecodedPicBuffer::clear( PicBufferList& rcOutputList,
                         PicBufferList& rcUnusedList,
                         Int&           riMaxPoc )
{
  RNOK( xClearOutputAll( rcOutputList, rcUnusedList, riMaxPoc ) ); // clear and output all pictures
  ROF ( m_cPicBufferList.empty() );
  return Err::m_nOK;
}



DPBUnit*
DecodedPicBuffer::getLastUnit()
{
  ROTRS( m_cUsedDPBUnitList.empty(), 0 );
  return m_cUsedDPBUnitList.back();
}



DPBUnit*
DecodedPicBuffer::getDPBUnit( Int iPoc )
{
  DPBUnit*              pcDPBUnit = 0;
  DPBUnitList::iterator iter      = m_cUsedDPBUnitList.begin();
  DPBUnitList::iterator end       = m_cUsedDPBUnitList.end  ();
  for( ; iter != end; iter++ )
  {
    if( (*iter)->getPoc() == iPoc && !(*iter)->isBaseRep() )
    {
      pcDPBUnit = *iter;
      break;
    }
  }
  return pcDPBUnit;
}



ErrVal
DecodedPicBuffer::setPrdRefLists( DPBUnit* pcCurrDPBUnit )
{
  ROF( m_pcCurrDPBUnit == pcCurrDPBUnit );
  ROF( m_pcCurrDPBUnit->getCtrlData().getSliceHeader() );

  RefFrameList& rcList0 = m_pcCurrDPBUnit->getCtrlData().getPrdFrameList( LIST_0 );
  RefFrameList& rcList1 = m_pcCurrDPBUnit->getCtrlData().getPrdFrameList( LIST_1 );
  
  rcList0.reset();
  rcList1.reset();
  ROTRS( m_pcCurrDPBUnit->getCtrlData().getSliceHeader()->isIntra(),   Err::m_nOK );

  if( m_pcCurrDPBUnit->getCtrlData().getSliceHeader()->isInterP() )
  {
    RNOK( xInitPrdListPSlice( rcList0 ) );
    RNOK( xPrdListRemapping ( rcList0, LIST_0, m_pcCurrDPBUnit->getCtrlData().getSliceHeader() ) );
    RNOK( xDumpRefList( LIST_0, rcList0 ) );
  }
  else
  {
    RNOK( xInitPrdListsBSlice( rcList0, rcList1 ) );
    RNOK( xPrdListRemapping  ( rcList0, LIST_0, m_pcCurrDPBUnit->getCtrlData().getSliceHeader() ) );
    RNOK( xPrdListRemapping  ( rcList1, LIST_1, m_pcCurrDPBUnit->getCtrlData().getSliceHeader() ) );
    RNOK( xDumpRefList( LIST_0, rcList0 ) );
    RNOK( xDumpRefList( LIST_1, rcList1 ) );
  }

  return Err::m_nOK;
}



ErrVal
DecodedPicBuffer::store( DPBUnit*&        rpcDPBUnit,
                         PicBufferList&   rcOutputList,
                         PicBufferList&   rcUnusedList,
                         IntFrame*        pcFrameBaseRep )
{
  RNOK( xStorePicture( rpcDPBUnit, rcOutputList, rcUnusedList,
                       rpcDPBUnit->getCtrlData().getSliceHeader()->isIdrNalUnit() ) );
  if( rpcDPBUnit->isNeededForRef() )
  {
    m_uiLastRefFrameNum = rpcDPBUnit->getFrameNum();
  }
  ROFRS( pcFrameBaseRep, Err::m_nOK );

  //===== store base representation =====
  //--- get DPB unit ---
  if( m_cFreeDPBUnitList.empty() )
  {
    // not sure whether this always works ...
    RNOK( xOutput( rcOutputList, rcUnusedList ) );
  }
  DPBUnit*  pcBaseRep = m_cFreeDPBUnitList.popFront();
  //--- init unit and extend picture ---
  RNOK( pcBaseRep->initBase( *rpcDPBUnit, pcFrameBaseRep ) );
  RNOK( m_pcYuvBufferCtrl->initMb() );
  RNOK( pcBaseRep->getFrame()->extendFrame( NULL ) );
  //--- store just before normal representation of the same picture
  DPBUnitList::iterator iter  = m_cUsedDPBUnitList.begin();
  DPBUnitList::iterator end   = m_cUsedDPBUnitList.end  ();
  for( ; iter != end; iter++ )
  {
    if( (*iter) == rpcDPBUnit )
    {
      break;
    }
  }
  ROT( iter == end );
  m_cUsedDPBUnitList.insert( iter, pcBaseRep );
  RNOK( xDumpDPB() );

  //===== reset DPB unit =====
  rpcDPBUnit = 0;

  return Err::m_nOK;
}



ErrVal
DecodedPicBuffer::update( DPBUnit*  pcDPBUnit )
{
  ROF( pcDPBUnit );

  //---- fill border ----
  RNOK( m_pcYuvBufferCtrl->initMb() );
  RNOK( pcDPBUnit->getFrame()->extendFrame( NULL ) );

  return Err::m_nOK;
}











//////////////////////////////////////////////////////////////////////////
// MCTF DECODER
//////////////////////////////////////////////////////////////////////////
MCTFDecoder::MCTFDecoder()
: m_pcH264AVCDecoder              ( 0 )
, m_pcSliceReader                 ( 0 )
, m_pcSliceDecoder                ( 0 )
, m_pcNalUnitParser               ( 0 )
, m_pcControlMng                  ( 0 )
, m_pcLoopFilter                  ( 0 )
, m_pcHeaderSymbolReadIf          ( 0 )
, m_pcParameterSetMng             ( 0 )
, m_pcPocCalculator               ( 0 )
, m_pcYuvFullPelBufferCtrl        ( 0 )
, m_pcDecodedPictureBuffer        ( 0 )
, m_pcMotionCompensation          ( 0 )
, m_pcQuarterPelFilter            ( 0 )
, m_bInitDone                     ( false )
, m_bCreateDone                   ( false )
, m_bWaitForIdr                   ( true )
, m_uiFrameWidthInMb              ( 0 )
, m_uiFrameHeightInMb             ( 0 )
, m_uiMbNumber                    ( 0 )
, m_pcResidual                    ( 0 )
, m_pcILPrediction                ( 0 )
, m_pcPredSignal                  ( 0 )
, m_pcBaseLayerResidual           ( 0 )
, m_pcBaseLayerFrame              ( 0 )
, m_pcBaseLayerCtrl               ( 0 )
, m_pcCurrDPBUnit                 ( 0 )
, m_uiLayerId                     ( 0 )
, m_bActive                       ( false )
, m_uiQualityLevelForPrediction   ( 3 )
#if MULTIPLE_LOOP_DECODING
, m_bCompletelyDecodeLayer        ( false )
#endif
, m_pcResizeParameter             ( 0 ) //TMM_ESS
, m_iMbProcessed                           (-1) //--ICU/ETRI FMO Implementation
{
  ::memset( m_apcFrameTemp, 0x00, sizeof( m_apcFrameTemp ) );
}



MCTFDecoder::~MCTFDecoder()
{
}



ErrVal
MCTFDecoder::create( MCTFDecoder*& rpcMCTFDecoder )
{
  rpcMCTFDecoder = new MCTFDecoder;
  ROT( NULL == rpcMCTFDecoder );
  return Err::m_nOK;
}



ErrVal
MCTFDecoder::destroy()
{
  ROT( m_bInitDone );

  delete this;
  return Err::m_nOK;
}



ErrVal
MCTFDecoder::init( H264AVCDecoder*      pcH264AVCDecoder,
                   SliceReader*         pcSliceReader,
                   SliceDecoder*        pcSliceDecoder,
                   RQFGSDecoder*        pcRQFGSDecoder,
                   NalUnitParser*       pcNalUnitParser,
                   ControlMngIf*        pcControlMng,
                   LoopFilter*          pcLoopFilter,
                   HeaderSymbolReadIf*  pcHeaderSymbolReadIf,
                   ParameterSetMng*     pcParameterSetMng,
                   PocCalculator*       pcPocCalculator,
                   YuvBufferCtrl*       pcYuvFullPelBufferCtrl,
                   DecodedPicBuffer*    pcDecodedPictureBuffer,
                   MotionCompensation*  pcMotionCompensation,
                   QuarterPelFilter*    pcQuarterPelFilter )
{
  ROT( NULL == pcH264AVCDecoder );
  ROT( NULL == pcSliceReader );
  ROT( NULL == pcSliceDecoder );
  ROT( NULL == pcNalUnitParser );
  ROT( NULL == pcControlMng );
  ROT( NULL == pcLoopFilter );
  ROT( NULL == pcHeaderSymbolReadIf );
  ROT( NULL == pcParameterSetMng );
  ROT( NULL == pcPocCalculator );
  ROT( NULL == pcQuarterPelFilter );
  ROT( NULL == pcYuvFullPelBufferCtrl );
  ROT( NULL == pcDecodedPictureBuffer );
  ROT( NULL == pcMotionCompensation );
  ROT( NULL == pcRQFGSDecoder );

  m_pcH264AVCDecoder              = pcH264AVCDecoder;
  m_pcSliceReader                 = pcSliceReader;
  m_pcSliceDecoder                = pcSliceDecoder ;
  m_pcNalUnitParser               = pcNalUnitParser;
  m_pcControlMng                  = pcControlMng;
  m_pcLoopFilter                  = pcLoopFilter;
  m_pcHeaderSymbolReadIf          = pcHeaderSymbolReadIf;
  m_pcParameterSetMng             = pcParameterSetMng;
  m_pcPocCalculator               = pcPocCalculator;
  m_pcYuvFullPelBufferCtrl        = pcYuvFullPelBufferCtrl;
  m_pcDecodedPictureBuffer        = pcDecodedPictureBuffer;
  m_pcMotionCompensation          = pcMotionCompensation;
  m_pcQuarterPelFilter            = pcQuarterPelFilter;
  m_pcRQFGSDecoder                = pcRQFGSDecoder;

  
  m_bInitDone                     = true;
  m_bCreateDone                   = false;
  m_bWaitForIdr                   = true;
  m_bActive                       = false;
  m_uiFrameWidthInMb              = 0;
  m_uiFrameHeightInMb             = 0;
  m_uiMbNumber                    = 0;

  m_pcResidual                    = 0;
  m_pcILPrediction                = 0;
  m_pcBaseLayerFrame              = 0;
  m_pcBaseLayerResidual           = 0;
  m_pcPredSignal                  = 0;
  m_pcBaseLayerCtrl               = 0;
  m_pcCurrDPBUnit                 = 0;

  m_uiLayerId                     = 0;

  m_iMbProcessed                  = -1;

  return Err::m_nOK;
}



ErrVal
MCTFDecoder::uninit()
{
  m_pcSliceReader             = NULL;
  m_pcSliceDecoder            = NULL;
  m_pcNalUnitParser           = NULL;
  m_pcControlMng              = NULL;
  m_pcLoopFilter              = NULL;
  m_pcHeaderSymbolReadIf      = NULL;
  m_pcParameterSetMng         = NULL;
  m_pcPocCalculator           = NULL;
  m_pcYuvFullPelBufferCtrl    = NULL;
  m_pcMotionCompensation      = NULL;
  m_uiFrameWidthInMb          = 0;
  m_uiFrameHeightInMb         = 0;

  RNOK( xDeleteData() );

  m_bInitDone                 = false;

  return Err::m_nOK;
}




ErrVal
MCTFDecoder::initSlice0( SliceHeader* rcSH )
{
  ROTRS( m_bActive, Err::m_nOK );

  //===== get and set relevant parameters =====
  m_uiLayerId         = rcSH->getLayerId();
  m_uiFrameWidthInMb  = rcSH->getSPS().getFrameWidthInMbs();
  m_uiFrameHeightInMb = rcSH->getSPS().getFrameHeightInMbs();
  m_uiMbNumber        = m_uiFrameWidthInMb * m_uiFrameHeightInMb;

  //===== re-allocate dynamic memory =====
  RNOK( xDeleteData() );
  RNOK( xCreateData( rcSH->getSPS() ) );

  //===== initialize DPB =====
  RNOK( m_pcDecodedPictureBuffer->initSPS( rcSH->getSPS() ) );

  //===== initialize some parameters =====
  m_bActive         = true;
  m_bInitDone       = true;

  return Err::m_nOK;
}



ErrVal
MCTFDecoder::process( SliceHeader*&  rpcSliceHeader,
                      PicBuffer*     pcPicBuffer,
                      PicBufferList& rcPicBufferOutputList,
                      PicBufferList& rcPicBufferUnusedList,
                      Bool           bReconstructionLayer )
{
  ROF  ( m_bInitDone );
  ROTRS( m_bWaitForIdr && !rpcSliceHeader->isIdrNalUnit(), Err::m_nOK );
  m_bWaitForIdr = false;

  //===== decoding =====
  if( rpcSliceHeader->getSliceType() == F_SLICE )
  {
    RNOK( xDecodeFGSRefinement( rpcSliceHeader ) );
  }
  else
  {
    RNOK( xDecodeBaseRepresentation( rpcSliceHeader, pcPicBuffer, rcPicBufferOutputList, rcPicBufferUnusedList, bReconstructionLayer ) );
  }

  //===== clear unused pic buffer ====
  if( pcPicBuffer )
  {
    rcPicBufferUnusedList.push_back( pcPicBuffer );
  }

  //===== delete slice header (if not stored) =====
  delete rpcSliceHeader;
  rpcSliceHeader = NULL;


  return Err::m_nOK;
}



ErrVal
MCTFDecoder::finishProcess( PicBufferList&  rcPicBufferOutputList,
                            PicBufferList&  rcPicBufferUnusedList,
                            Int&            riMaxPoc )
{
  RNOK( m_pcDecodedPictureBuffer->clear( rcPicBufferOutputList, rcPicBufferUnusedList, riMaxPoc ) );

  return Err::m_nOK;
}


ErrVal
MCTFDecoder::getBaseLayerPWTable( SliceHeader::PredWeightTable*& rpcPredWeightTable,
                                  ListIdx                        eListIdx,
                                  Int                            iPoc )
{
  DPBUnit*      pcBaseDPBUnit = m_pcDecodedPictureBuffer->getDPBUnit( iPoc );
  ROF( pcBaseDPBUnit );
  SliceHeader*  pcSliceHeader = pcBaseDPBUnit->getCtrlData().getSliceHeader();
  ROF( pcSliceHeader );
  rpcPredWeightTable          = &pcSliceHeader->getPredWeightTable( eListIdx );
  return Err::m_nOK;
}


ErrVal
MCTFDecoder::getBaseLayerData ( IntFrame*&    pcFrame,
                                IntFrame*&    pcResidual,
                                MbDataCtrl*&  pcMbDataCtrl,
                                Bool&         rbConstrainedIPred,
                                Bool          bSpatialScalability,
                                Int           iPoc )
{
  pcFrame                     = 0;
  pcResidual                  = 0;
  pcMbDataCtrl                = 0;
  SliceHeader*  pcSliceHeader = 0;
  DPBUnit*      pcBaseDPBUnit = m_pcDecodedPictureBuffer->getDPBUnit( iPoc );
  ROF( pcBaseDPBUnit );

  pcFrame       = m_pcILPrediction;
  pcResidual    = m_pcResidual;
  pcMbDataCtrl  = pcBaseDPBUnit->getCtrlData().getMbDataCtrl  ();
  pcSliceHeader = pcBaseDPBUnit->getCtrlData().getSliceHeader ();
  rbConstrainedIPred = pcBaseDPBUnit->isConstrIPred();

  if( bSpatialScalability )
  {
    RNOK( m_apcFrameTemp[0]->copy( pcFrame ) );
    pcFrame = m_apcFrameTemp[0];

#if MULTIPLE_LOOP_DECODING
    if( pcSliceHeader->getPPS().getConstrainedIntraPredFlag() && !m_bCompletelyDecodeLayer )
#else
    if( pcSliceHeader->getPPS().getConstrainedIntraPredFlag() )
#endif
    {
      m_pcLoopFilter->setFilterMode( LoopFilter::LFMode( LoopFilter::LFM_NO_INTER_FILTER + LoopFilter::LFM_EXTEND_INTRA_SUR ) );
      RNOK( m_pcLoopFilter->process(*pcSliceHeader,
                                     pcFrame,
                                     pcMbDataCtrl,
                                     pcMbDataCtrl,
                                     m_uiFrameWidthInMb,
                                     NULL,
                                     NULL,
                                     pcBaseDPBUnit->getCtrlData().getSpatialScalability()) );  // SSUN@SHARP
      m_pcLoopFilter->setFilterMode();
    }
    else
    {
      m_pcLoopFilter->setHighpassFramePointer( pcResidual );
      RNOK( m_pcLoopFilter->process(*pcSliceHeader,
                                     pcFrame,
                                     pcMbDataCtrl,
                                     pcMbDataCtrl,
                                     m_uiFrameWidthInMb,
                                    &pcBaseDPBUnit->getCtrlData().getPrdFrameList( LIST_0 ),
                                    &pcBaseDPBUnit->getCtrlData().getPrdFrameList( LIST_1 ),
                                     pcBaseDPBUnit->getCtrlData().getSpatialScalability()) );  // SSUN@SHARP
    }
  }
  
  return Err::m_nOK;
}




ErrVal 
MCTFDecoder::xCreateData( const SequenceParameterSet& rcSPS )
{
  UInt uiIndex;


  //========== CREATE FRAME MEMORIES ==========
  for( uiIndex = 0; uiIndex < NUM_TMP_FRAMES;  uiIndex++ )
  {
    ROFRS ( ( m_apcFrameTemp  [ uiIndex ]   = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                            *m_pcYuvFullPelBufferCtrl ) ), Err::m_nERR );
    RNOK  (   m_apcFrameTemp  [ uiIndex ]   ->init        () );
  }

  ROFRS   ( ( m_pcResidual                  = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                            *m_pcYuvFullPelBufferCtrl ) ), Err::m_nERR );
  RNOK    (   m_pcResidual                  ->init        () );
  
  ROFRS   ( ( m_pcILPrediction              = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                            *m_pcYuvFullPelBufferCtrl ) ), Err::m_nERR );
  RNOK    (   m_pcILPrediction              ->init        () );
  
  ROFRS   ( ( m_pcPredSignal                = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                            *m_pcYuvFullPelBufferCtrl ) ), Err::m_nERR );
  RNOK    (   m_pcPredSignal                ->init        () );
  
  ROFRS   ( ( m_pcBaseLayerFrame            = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                            *m_pcYuvFullPelBufferCtrl ) ), Err::m_nERR );
  RNOK    (   m_pcBaseLayerFrame            ->init        () );
  
  ROFRS   ( ( m_pcBaseLayerResidual         = new IntFrame( *m_pcYuvFullPelBufferCtrl,
                                                            *m_pcYuvFullPelBufferCtrl ) ), Err::m_nERR );
  RNOK    (   m_pcBaseLayerResidual         ->init        () );



  //========== CREATE MACROBLOCK DATA MEMORIES ==========
  ROFRS   ( ( m_pcBaseLayerCtrl = new MbDataCtrl() ), Err::m_nERR );
  RNOK    (   m_pcBaseLayerCtrl ->init          ( rcSPS ) );



  //========== CREATE UPDATE WEIGHTS ARRAY and WRITE BUFFER ==========
  UInt  uiNum4x4Blocks        = m_uiFrameWidthInMb * m_uiFrameHeightInMb * 4 * 4;
  ROT ( m_cDownConvert    .init   ( m_uiFrameWidthInMb<<4, m_uiFrameHeightInMb<<4 ) );

  return Err::m_nOK;
}



ErrVal 
MCTFDecoder::xDeleteData()
{
  UInt uiIndex;

  //========== DELETE FRAME MEMORIES ==========
  for( uiIndex = 0; uiIndex < NUM_TMP_FRAMES; uiIndex++ )
  {
    if( m_apcFrameTemp[ uiIndex ] )
    {
      RNOK(   m_apcFrameTemp[ uiIndex ]->uninit() );
      delete  m_apcFrameTemp[ uiIndex ];
      m_apcFrameTemp[ uiIndex ] = 0;
    }
  }

  if( m_pcResidual )
  {
    RNOK(   m_pcResidual->uninit() );
    delete  m_pcResidual;
    m_pcResidual = 0;
  }

  if( m_pcILPrediction )
  {
    RNOK(   m_pcILPrediction->uninit() );
    delete  m_pcILPrediction;
    m_pcILPrediction = 0;
  }

  if( m_pcPredSignal )
  {
    RNOK(   m_pcPredSignal->uninit() );
    delete  m_pcPredSignal;
    m_pcPredSignal = 0;
  }

  if( m_pcBaseLayerFrame )
  {
    RNOK(   m_pcBaseLayerFrame->uninit() );
    delete  m_pcBaseLayerFrame;
    m_pcBaseLayerFrame = 0;
  }

  if( m_pcBaseLayerResidual )
  {
    RNOK(   m_pcBaseLayerResidual->uninit() );
    delete  m_pcBaseLayerResidual;
    m_pcBaseLayerResidual = 0;
  }


  //========== DELETE MACROBLOCK DATA MEMORIES (and SLICE HEADER) ==========
  if( m_pcBaseLayerCtrl )
  {
    RNOK( m_pcBaseLayerCtrl->uninit() );
    delete m_pcBaseLayerCtrl;
    m_pcBaseLayerCtrl = 0;
  }

  return Err::m_nOK;
}




ErrVal
MCTFDecoder::xZeroIntraMacroblocks( IntFrame*    pcFrame,
                                    ControlData& rcCtrlData )
{
  MbDataCtrl*       pcMbDataCtrl  = rcCtrlData.getMbDataCtrl       ();
  SliceHeader*      pcSliceHeader = rcCtrlData.getSliceHeader      ();
  IntYuvPicBuffer*  pcPicBuffer   = pcFrame  ->getFullPelYuvBuffer ();

  RNOK( pcMbDataCtrl->initSlice( *pcSliceHeader, PRE_PROCESS, true, NULL ) );

  IntYuvMbBuffer cZeroMbBuffer;
  cZeroMbBuffer.setAllSamplesToZero();

  for( UInt uiMbIndex = 0; uiMbIndex < m_uiMbNumber; uiMbIndex++ )
  {
    UInt          uiMbY           = uiMbIndex / m_uiFrameWidthInMb;
    UInt          uiMbX           = uiMbIndex % m_uiFrameWidthInMb;
    MbDataAccess* pcMbDataAccess  = 0;

    RNOK( pcMbDataCtrl            ->initMb( pcMbDataAccess, uiMbY, uiMbX ) );
    RNOK( m_pcYuvFullPelBufferCtrl->initMb(                 uiMbY, uiMbX ) );

    if( pcMbDataAccess->getMbData().isIntra() )
    {
      pcPicBuffer->loadBuffer( &cZeroMbBuffer );
    }
  }

  return Err::m_nOK;
}



ErrVal
MCTFDecoder::xAddBaseLayerResidual( ControlData& rcControlData,
                                    IntFrame*    pcFrame )
{
  ROFRS( rcControlData.getBaseLayerSbb(), Err::m_nOK );

  MbDataCtrl*       pcMbDataCtrl  = rcControlData.getMbDataCtrl       ();
  SliceHeader*      pcSliceHeader = rcControlData.getSliceHeader      ();
  IntYuvPicBuffer*  pcBLResidual  = rcControlData.getBaseLayerSbb     ()->getFullPelYuvBuffer();
  IntYuvPicBuffer*  pcPicBuffer   = pcFrame     ->getFullPelYuvBuffer ();
  IntYuvMbBuffer    cBLResBuffer;
  IntYuvMbBuffer    cMbBuffer;

  RNOK( pcMbDataCtrl->initSlice( *pcSliceHeader, PRE_PROCESS, true, NULL ) );

  for( UInt uiMbIndex = 0; uiMbIndex < m_uiMbNumber; uiMbIndex++ )
  {
    UInt          uiMbY           = uiMbIndex / m_uiFrameWidthInMb;
    UInt          uiMbX           = uiMbIndex % m_uiFrameWidthInMb;
    MbDataAccess* pcMbDataAccess  = 0;

    RNOK( pcMbDataCtrl            ->initMb( pcMbDataAccess, uiMbY, uiMbX ) );
    RNOK( m_pcYuvFullPelBufferCtrl->initMb(                 uiMbY, uiMbX ) );

    if( ! pcMbDataAccess->getMbData().isIntra() && pcMbDataAccess->getMbData().getResidualPredFlag( PART_16x16 ) )
    {
      cMbBuffer   .loadBuffer ( pcPicBuffer  );
      cBLResBuffer.loadBuffer ( pcBLResidual );
      cMbBuffer   .add        ( cBLResBuffer );
      pcPicBuffer->loadBuffer ( &cMbBuffer );
    }
  }

  return Err::m_nOK;
}



ErrVal
MCTFDecoder::initSlice( SliceHeader* pcSliceHeader, UInt uiLastLayer )
{
  ROFRS( m_bActive, Err::m_nOK );

  //===== calculate POC =====
  if( pcSliceHeader && pcSliceHeader->getLayerId() == m_uiLayerId )
  {
    RNOK( m_pcPocCalculator->calculatePoc( *pcSliceHeader ) );
  }

  //===== check if an FGS enhancement needs to be reconstructed =====
  if( m_pcRQFGSDecoder->isInitialized  ()                                                               &&
      m_pcRQFGSDecoder->getSliceHeader ()->getLayerId() == m_uiLayerId                                  &&
    (!pcSliceHeader                                                                                     ||
      pcSliceHeader->getLayerId       () != m_pcRQFGSDecoder->getSliceHeader()->getLayerId      ()      ||
      pcSliceHeader->getTemporalLevel () != m_pcRQFGSDecoder->getSliceHeader()->getTemporalLevel()      ||
      pcSliceHeader->getQualityLevel  () != m_pcRQFGSDecoder->getSliceHeader()->getQualityLevel () + 1  ||
      pcSliceHeader->getPoc           () != m_pcRQFGSDecoder->getSliceHeader()->getPoc          ()        ) )
  {
    Bool bHighestLayer = ( uiLastLayer == m_uiLayerId );
    RNOK( xReconstructLastFGS( bHighestLayer ) );
  }

  return Err::m_nOK;
}


ErrVal
MCTFDecoder::xReconstructLastFGS( Bool bHighestLayer )
{
  DPBUnit*      pcLastDPBUnit   = m_pcDecodedPictureBuffer->getLastUnit();
  SliceHeader*  pcSliceHeader   = m_pcRQFGSDecoder->getSliceHeader();
  Bool          bKeyPicFlag     = pcSliceHeader   ->getKeyPictureFlag(); // HS: fix by Nokia
  ROF( pcLastDPBUnit );
  ROF( pcSliceHeader == pcLastDPBUnit->getCtrlData().getSliceHeader() );
  IntFrame*     pcFrame         = pcLastDPBUnit->getFrame    ();
  ControlData&  rcControlData   = pcLastDPBUnit->getCtrlData ();

  //===== reconstruct FGS =====
  if( m_pcRQFGSDecoder->changed() )
  {
    RNOK( m_pcRQFGSDecoder->reconstruct( pcFrame ) );
    RNOK( xAddBaseLayerResidual        ( rcControlData, pcFrame ) );
    RNOK( m_pcResidual    ->copy       ( pcFrame ) )
    RNOK( xZeroIntraMacroblocks        ( m_pcResidual, rcControlData ) );

    if( bKeyPicFlag && pcSliceHeader->isInterP() )
    {
      RefFrameList  cRefListDiff;

      setDiffPrdRefLists( cRefListDiff, m_pcYuvFullPelBufferCtrl );

      //----- key frames: adaptive motion-compensated prediction -----
      m_pcMotionCompensation->loadAdaptiveRefPredictors(
        m_pcYuvFullPelBufferCtrl, m_pcPredSignal, 
        m_pcPredSignal, &cRefListDiff, 
        m_pcRQFGSDecoder->getMbDataCtrl(), m_pcRQFGSDecoder, 
        m_pcRQFGSDecoder->getSliceHeader());

      freeDiffPrdRefLists(cRefListDiff);
    }

    //----- add prediction signal and clip -----
    RNOK( pcFrame         ->add        ( m_pcPredSignal ) );
    RNOK( pcFrame         ->clip       () );
  }
  RNOK( m_pcRQFGSDecoder->finishPicture () );

  //===== store intra signal for inter-layer prediction =====
  RNOK( m_pcILPrediction->copy( pcFrame ) );

  //===== loop filter =====
#if MULTIPLE_LOOP_DECODING
  if( bHighestLayer || bKeyPicFlag || m_bCompletelyDecodeLayer /* for in-layer mc prediction */ ) // HS: fix by Nokia
#else
  if( bHighestLayer || bKeyPicFlag ) // HS: fix by Nokia
#endif
  {
    m_pcLoopFilter->setHighpassFramePointer( m_pcResidual );
    RNOK( m_pcLoopFilter->process( *pcSliceHeader,
                                    pcFrame,
                                    ( pcSliceHeader->isIntra() ? NULL : rcControlData.getMbDataCtrl() ),
                                    rcControlData.getMbDataCtrl(),
                                    m_uiFrameWidthInMb,
                                   &rcControlData.getPrdFrameList( LIST_0 ),
                                   &rcControlData.getPrdFrameList( LIST_1 ),
                                    rcControlData.getSpatialScalability()) );  // SSUN@SHARP
  }

  //===== update picture in DPB =====
  RNOK( m_pcDecodedPictureBuffer->update( pcLastDPBUnit ) );

  return Err::m_nOK;
}





ErrVal
MCTFDecoder::xDecodeFGSRefinement( SliceHeader*& rpcSliceHeader )
{
  ROFRS( m_pcRQFGSDecoder->isInitialized(), Err::m_nOK );

  //===== check slice header =====
  if(!m_pcRQFGSDecoder->isFinished    ()                                                                 &&
      m_pcRQFGSDecoder->getSliceHeader()->getTemporalLevel()      == rpcSliceHeader->getTemporalLevel () &&
      m_pcRQFGSDecoder->getSliceHeader()->getPoc          ()      == rpcSliceHeader->getPoc           () &&
      m_pcRQFGSDecoder->getSliceHeader()->getQualityLevel () + 1  == rpcSliceHeader->getQualityLevel  ()  )
  {
    if( rpcSliceHeader->getQualityLevel() <= (UInt)m_uiQualityLevelForPrediction )
    {
      printf("  Frame %4d ( LId%2d, TL%2d, QL%2d, PrRef,              QP%3d )\n",
        rpcSliceHeader->getPoc                    (),
        rpcSliceHeader->getLayerId                (),
        rpcSliceHeader->getTemporalLevel          (),
        rpcSliceHeader->getQualityLevel           (),
        rpcSliceHeader->getPicQp                  () );
      UInt uiRecLayer = rpcSliceHeader->getQualityLevel();

      if( m_pcRQFGSDecoder->getSliceHeader()->getTemporalLevel()  == 0      &&
        m_pcRQFGSDecoder->getSliceHeader()->isInterP())
      {
        // m_pcRQFGSDecoder->getSliceHeader has the slice header of the base layer
        m_pcRQFGSDecoder->getSliceHeader()->setBaseWeightZeroBaseBlock(rpcSliceHeader->getBaseWeightZeroBaseBlock() );
        m_pcRQFGSDecoder->getSliceHeader()->setBaseWeightZeroBaseCoeff(rpcSliceHeader->getBaseWeightZeroBaseCoeff() );
        m_pcRQFGSDecoder->getSliceHeader()->setLowPassFgsMcFilter     (rpcSliceHeader->getLowPassFgsMcFilter() );

        if( rpcSliceHeader->getQualityLevel() == 1) 
        {
          m_pcRQFGSDecoder->getMbDataCtrl()->storeFgsBQLayerQpAndCbp();
          m_pcRQFGSDecoder->xStoreBQLayerSigMap();
        }
      }

      RNOK( m_pcRQFGSDecoder->decodeNextLayer( rpcSliceHeader ) );
    }
  }

  return Err::m_nOK;
}




ErrVal
MCTFDecoder::xInitBaseLayer( ControlData& rcControlData )
{
  //===== init =====
  rcControlData.setBaseLayerRec ( 0 );
  rcControlData.setBaseLayerSbb ( 0 );
  rcControlData.setBaseLayerCtrl( 0 );

  
  IntFrame*   pcBaseFrame         = 0;
  IntFrame*   pcBaseResidual      = 0;
  MbDataCtrl* pcBaseDataCtrl      = 0;
  Bool        bConstrainedIPredBL = false;
  Bool        bSpatialScalability = false;
  Bool        bBaseDataAvailable  = false;

  if( rcControlData.getSliceHeader()->getBaseLayerId() != MSYS_UINT_MAX )
  {
    //TMM_ESS { 
    SliceHeader* pcSliceHeader = rcControlData.getSliceHeader();
    Int          poc           = pcSliceHeader->getPoc();
    m_pcResizeParameter->setPictureParametersByOffset( poc,
                                                       pcSliceHeader->getLeftOffset(),
                                                       pcSliceHeader->getRightOffset(),
                                                       pcSliceHeader->getTopOffset(),
                                                       pcSliceHeader->getBottomOffset(),
                                                       pcSliceHeader->getBaseChromaPhaseX(),
                                                       pcSliceHeader->getBaseChromaPhaseY() );
    m_pcResizeParameter->setPOC( poc );
    //TMM_ESS }   
      
    RNOK( m_pcH264AVCDecoder->getBaseLayerData( pcBaseFrame, pcBaseResidual, pcBaseDataCtrl, bConstrainedIPredBL, bSpatialScalability,
                                                m_uiLayerId,
                                                rcControlData.getSliceHeader()->getBaseLayerId(),
                                                rcControlData.getSliceHeader()->getPoc() ) );    
    bBaseDataAvailable = pcBaseFrame && pcBaseResidual && pcBaseDataCtrl;
    ROF( bBaseDataAvailable );

    rcControlData.setSpatialScalability     ( bSpatialScalability );
    rcControlData.setSpatialScalabilityType ( m_pcResizeParameter->m_iSpatialScalabilityType );
  }


  //===== motion data =====
  if( pcBaseDataCtrl )
  {
    RNOK( m_pcBaseLayerCtrl->initSlice( *rcControlData.getSliceHeader(), PRE_PROCESS, false, NULL ) );

    // TMM_ESS {
    if (m_pcResizeParameter->m_iSpatialScalabilityType == SST_RATIO_1) 
    {
      RNOK( m_pcBaseLayerCtrl->copyMotionBL  ( *pcBaseDataCtrl, m_pcResizeParameter ) );
    }
    else
    {
      if(m_pcResizeParameter->m_iExtendedSpatialScalability == ESS_PICT ) 
      {
        // BUGFIX_JV{
        RefFrameList& rcList0=rcControlData.getPrdFrameList( LIST_0 );
        RefFrameList& rcList1=rcControlData.getPrdFrameList( LIST_1 );
  	 
	      UInt uiIndex;
        for( uiIndex = 1; uiIndex <= rcList0.getActive(); uiIndex++ )
        m_pcResizeParameter->m_aiRefListPoc[0][uiIndex-1]=rcList0[uiIndex]->getPOC() ;
        for( uiIndex = 1; uiIndex <= rcList1.getActive(); uiIndex++ )
        m_pcResizeParameter->m_aiRefListPoc[1][uiIndex-1]=rcList1[uiIndex]->getPOC() ;
        // BUGFIX_JV }
      }
      RNOK( m_pcBaseLayerCtrl->upsampleMotion( *pcBaseDataCtrl, m_pcResizeParameter) );
    }

    rcControlData.getMbDataCtrl()->copyBaseResidualAvailFlags( *m_pcBaseLayerCtrl );
    rcControlData.setBaseLayerCtrl( m_pcBaseLayerCtrl );

    rcControlData.getSliceHeader()->setBaseLayerUsesConstrainedIntraPred( bConstrainedIPredBL );
  }


  //===== residual data =====
  if( bBaseDataAvailable )
  {
    RNOK( m_pcBaseLayerResidual->copy( pcBaseResidual ) ); 
  	// TMM_ESS 
	  RNOK( m_pcBaseLayerResidual->upsampleResidual( m_cDownConvert, m_pcResizeParameter, pcBaseDataCtrl, false) ); 
		
    rcControlData.setBaseLayerSbb( m_pcBaseLayerResidual );
  }


  //===== reconstruction (intra) =====
  if( pcBaseFrame )
  {
    RNOK( m_pcBaseLayerFrame->copy( pcBaseFrame ) );
    // TMM_ESS
    RNOK( m_pcBaseLayerFrame->upsample( m_cDownConvert, m_pcResizeParameter, true ) );

    rcControlData.setBaseLayerRec( m_pcBaseLayerFrame );
  }

  return Err::m_nOK;
}





ErrVal
MCTFDecoder::xDecodeBaseRepresentation( SliceHeader*&  rpcSliceHeader,
                                        PicBuffer*&    rpcPicBuffer,
                                        PicBufferList& rcOutputList,
                                        PicBufferList& rcUnusedList,
                                        Bool           bReconstructionLayer )
{
  printf("  Frame %4d ( LId%2d, TL%2d, QL%2d, %s-%c, BId%2d, AP%2d, QP%3d )\n",
    rpcSliceHeader->getPoc                    (),
    rpcSliceHeader->getLayerId                (),
    rpcSliceHeader->getTemporalLevel          (),
    rpcSliceHeader->getQualityLevel           (),
    rpcSliceHeader->isH264AVCCompatible       () ? "AVC" : "SVC",
    rpcSliceHeader->getSliceType              () == I_SLICE ? 'I' :
    rpcSliceHeader->getSliceType              () == P_SLICE ? 'P' : 'B',
    rpcSliceHeader->getBaseLayerId            (),
    rpcSliceHeader->getAdaptivePredictionFlag () ? 1 : 0,
    rpcSliceHeader->getPicQp                  () );

  //===== infer prediction weights when required =====
  if( rpcSliceHeader->getPPS().getWeightedBiPredIdc() == 1 && 
      rpcSliceHeader->getSliceType()                  == B_SLICE &&
      rpcSliceHeader->getBaseLayerId()                != MSYS_UINT_MAX &&
      rpcSliceHeader->getBasePredWeightTableFlag() )
  {
    SliceHeader::PredWeightTable* pcPredWeightTableL0 = NULL;
    SliceHeader::PredWeightTable* pcPredWeightTableL1 = NULL;
    RNOK( m_pcH264AVCDecoder->getBaseLayerPWTable( pcPredWeightTableL0, rpcSliceHeader->getBaseLayerId(), LIST_0, rpcSliceHeader->getPoc() ) );
    RNOK( m_pcH264AVCDecoder->getBaseLayerPWTable( pcPredWeightTableL1, rpcSliceHeader->getBaseLayerId(), LIST_1, rpcSliceHeader->getPoc() ) );
    RNOK( rpcSliceHeader->getPredWeightTable( LIST_0 ).copy( *pcPredWeightTableL0 ) );
    RNOK( rpcSliceHeader->getPredWeightTable( LIST_1 ).copy( *pcPredWeightTableL1 ) );
  }
  else if( rpcSliceHeader->getPPS().getWeightedPredFlag() && 
           rpcSliceHeader->getSliceType()             == P_SLICE &&
           rpcSliceHeader->getBaseLayerId()           != MSYS_UINT_MAX &&
           rpcSliceHeader->getBasePredWeightTableFlag() )
  {
    SliceHeader::PredWeightTable* pcPredWeightTableL0 = NULL;
    RNOK( m_pcH264AVCDecoder->getBaseLayerPWTable( pcPredWeightTableL0, rpcSliceHeader->getBaseLayerId(), LIST_0, rpcSliceHeader->getPoc() ) );
    RNOK( rpcSliceHeader->getPredWeightTable( LIST_0 ).copy( *pcPredWeightTableL0 ) );
  }


  m_uiNumLayers[0] = m_uiNumLayers[1];

  //===== init =====
  if(isNewPictureStart(rpcSliceHeader))
    RNOK( m_pcDecodedPictureBuffer->initPicCurrDPBUnit( m_pcCurrDPBUnit, rpcPicBuffer, 0, rpcSliceHeader,
                                                     rcOutputList, rcUnusedList ) );

  RNOK( m_pcDecodedPictureBuffer->initCurrDPBUnit( m_pcCurrDPBUnit, rpcPicBuffer, rpcSliceHeader,
                                                   rcOutputList, rcUnusedList ) );
 
  //TMM_ESS {
   //----- initialize reference lists -----
   //BUG_FIX JV
    RNOK( m_pcDecodedPictureBuffer->setPrdRefLists( m_pcCurrDPBUnit ) );
  //TMM_ESS }

  RNOK( xInitBaseLayer( m_pcCurrDPBUnit->getCtrlData() ) );

  ControlData&  rcControlData   = m_pcCurrDPBUnit->getCtrlData();
  IntFrame*     pcFrame         = m_pcCurrDPBUnit->getFrame   ();
  IntFrame*     pcResidual      = m_pcResidual;
  IntFrame*     pcBaseRepFrame  = m_apcFrameTemp[0];
  MbDataCtrl*   pcMbDataCtrl    = rcControlData.getMbDataCtrl();
  UInt          uiMbRead        = 0;

  Bool          bKeyPicture     = rpcSliceHeader->getKeyPictureFlag();
  Bool          bConstrainedIP  = rpcSliceHeader->getPPS().getConstrainedIntraPredFlag();
#if MULTIPLE_LOOP_DECODING
  Bool          bReconstructAll = m_bCompletelyDecodeLayer || bReconstructionLayer || !bConstrainedIP;
#else
  Bool          bReconstructAll = bReconstructionLayer || !bConstrainedIP;
#endif
  
  if(isNewPictureStart(rpcSliceHeader))
    m_iMbProcessed =0;

  //----- parsing -----
  RNOK( m_pcControlMng  ->initSliceForReading ( *rpcSliceHeader ) );
  RNOK( m_pcSliceReader ->read                ( *rpcSliceHeader,
                                                pcMbDataCtrl,
                                                rcControlData.getBaseLayerCtrl(),
                                                rcControlData.getSpatialScalabilityType(),
                                                m_uiFrameWidthInMb,
                                                uiMbRead ) );

  //----- decoding -----
  RNOK( m_pcControlMng  ->initSliceForDecoding( *rpcSliceHeader ) );
  RNOK( m_pcSliceDecoder->decode              ( *rpcSliceHeader,
                                                pcMbDataCtrl,
                                                rcControlData.getBaseLayerCtrl(),
                                                bKeyPicture ? pcBaseRepFrame : pcFrame,
                                                pcResidual,
                                                m_pcPredSignal,
                                                rcControlData.getBaseLayerRec(),
                                                rcControlData.getBaseLayerSbb(),
                                               &rcControlData.getPrdFrameList( LIST_0 ),
                                               &rcControlData.getPrdFrameList( LIST_1 ),
                                                bReconstructAll,
                                                m_uiFrameWidthInMb,
                                                uiMbRead ) );

  m_iMbProcessed += uiMbRead;

  if(isPictureDecComplete(rpcSliceHeader)) //--TM prob
  {
  //----- store in decoded picture buffer -----
  if( bKeyPicture )
  {
    //----- copy non-filtered frame -----
    RNOK( pcFrame->copy( pcBaseRepFrame ) );
    //----- loop-filtering and store in DPB as base representation -----
    m_pcLoopFilter->setHighpassFramePointer( pcResidual );
    RNOK( m_pcLoopFilter->process( *rpcSliceHeader,
                                   pcBaseRepFrame,
                                   ( rpcSliceHeader->isIntra() ? NULL : pcMbDataCtrl ),
                                   pcMbDataCtrl,
                                   m_uiFrameWidthInMb,
                                   &rcControlData.getPrdFrameList( LIST_0 ),
                                   &rcControlData.getPrdFrameList( LIST_1 ),
                                   rcControlData.getSpatialScalability() ) );  // SSUN@SHARP
    //----- store in DPB with base representation -----
    RNOK( m_pcDecodedPictureBuffer->store( m_pcCurrDPBUnit, rcOutputList, rcUnusedList, pcBaseRepFrame ) );
  }
  else
  {
    RNOK( m_pcDecodedPictureBuffer->store( m_pcCurrDPBUnit, rcOutputList, rcUnusedList ) );
  }


  //----- set slice header to zero (slice header is stored in control data) -----
  rpcSliceHeader = 0;

  //----- init FGS decoder -----
      RNOK( m_pcRQFGSDecoder->initPicture( rcControlData.getSliceHeader(), rcControlData.getMbDataCtrl() ) );
  }

  DTRACE_NEWFRAME;
  return Err::m_nOK;
}



Bool MCTFDecoder::isPictureDecComplete(SliceHeader* rpcSliceHeader)
{
  Bool bPictureComplete ;
  if(m_iMbProcessed  == rpcSliceHeader->getMbInPic())
	  bPictureComplete = true;
  else 
	  bPictureComplete = false;
  return bPictureComplete;
}


const Bool MCTFDecoder::isNewPictureStart(SliceHeader* rpcSliceHeader)
{
  if(m_iMbProcessed ==-1 || (m_iMbProcessed  == rpcSliceHeader->getMbInPic()) )  return true;
  else return false;
}


ErrVal
MCTFDecoder::setDiffPrdRefLists( RefFrameList& diffPrdRefList, 
                                 YuvBufferCtrl* pcYuvFullPelBufferCtrl )
{
  DPBUnit* pcCurrDPBUnit = m_pcDecodedPictureBuffer->getLastUnit();
  ROF( pcCurrDPBUnit );

  ROTRS( pcCurrDPBUnit->getCtrlData().getSliceHeader()->isIntra(),   Err::m_nOK );

  RefFrameList& rcBaseList = pcCurrDPBUnit->getCtrlData().getPrdFrameList( LIST_0 );

  if(rcBaseList.getSize() ==0 )
    return Err::m_nERR;

  for(UInt i=0; i< rcBaseList.getActive(); i++)
  {
    IntFrame  *pcDiffFrame, *enhFrame;
    Int iPoc;

    ROFRS   ( ( pcDiffFrame                  = new IntFrame( *pcYuvFullPelBufferCtrl,
                                                           *pcYuvFullPelBufferCtrl ) ), Err::m_nERR );
    pcDiffFrame->init();

    iPoc = rcBaseList.getEntry(i)->getPOC();
    enhFrame = m_pcDecodedPictureBuffer->getDPBUnit(iPoc)->getFrame();

    pcDiffFrame->subtract(enhFrame, rcBaseList.getEntry(i));
    RNOK( pcDiffFrame->extendFrame( NULL ) );
    RNOK( diffPrdRefList.add( pcDiffFrame ) );
  }

  return Err::m_nOK;
}


ErrVal
MCTFDecoder::freeDiffPrdRefLists( RefFrameList& diffPrdRefList)
{
  for(UInt i=0; i< diffPrdRefList.getSize(); i++)
  {
    diffPrdRefList.getEntry(i)->uninit();
    free(diffPrdRefList.getEntry(i));
  }

  return Err::m_nOK;
}



H264AVC_NAMESPACE_END

