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

//JVT-U106 Behaviour at slice boundaries{
#include "H264AVCCommonLib/ReconstructionBypass.h"
//JVT-U106 Behaviour at slice boundaries}

H264AVC_NAMESPACE_BEGIN


__inline Void printSpaces( UInt uiNum )
{
  while( uiNum-- ) printf(" ");
}




//////////////////////////////////////////////////////////////////////////
// DPB UNIT
//////////////////////////////////////////////////////////////////////////

DPBUnit::DPBUnit()
//: m_iPoc                ( MSYS_INT_MIN )
: m_uiFrameNum          ( MSYS_UINT_MAX )
, m_uiTemporalLevel     ( MSYS_UINT_MAX )
, m_bUseBasePred        ( false )
, m_bExisting           ( false )
, m_bNeededForReference ( false )
, m_bOutputted          ( false )
, m_bBaseRepresentation ( false )
, m_pcFrame             ( NULL )
, m_cControlData        ()
, m_bConstrainedIntraPred( false )
{
}



DPBUnit::~DPBUnit()
{
  MbDataCtrl*   pcMbDataCtrl  = m_cControlData.getMbDataCtrl  ();
  SliceHeader*  pcSliceHeader = m_cControlData.getSliceHeader ();
  //m_cControlData.getMbDataCtrl()->uninitFgsBQData(); 
  if( pcMbDataCtrl )
  {
//JVT-T054{
    m_cControlData.getMbDataCtrl()->uninitFgsBQData();
//JVT-T054}
    pcMbDataCtrl->uninit();
	  delete pcMbDataCtrl;
		pcMbDataCtrl = NULL;
  }

	if( pcSliceHeader)
	{
  delete pcSliceHeader;
		pcSliceHeader = NULL;
	}

  if( m_pcFrame )
  {
    m_pcFrame->uninit();
		m_pcFrame->destroy();
		m_pcFrame = NULL;
  }
}

ErrVal
DPBUnit::create( DPBUnit*&                    rpcDPBUnit,
                 YuvBufferCtrl&               rcYuvBufferCtrl,
                 const SequenceParameterSet&  rcSPS )
{
  rpcDPBUnit = new DPBUnit();
  ROF( rpcDPBUnit );

  MbDataCtrl* pcMbDataCtrl = 0;
  ROFS( ( rpcDPBUnit->m_pcFrame    = new IntFrame  ( rcYuvBufferCtrl, rcYuvBufferCtrl ) ) );
  ROFS( ( pcMbDataCtrl             = new MbDataCtrl()                                   ) );
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
DPBUnit::init( const SliceHeader& rcSH )
{
  RNOK( m_pcFrame->addFrameFieldBuffer() );

	m_pcFrame->setPoc( rcSH );
  m_uiFrameNum            = rcSH.getFrameNum();
  m_uiTemporalLevel       = rcSH.getTemporalLevel();
  m_bUseBasePred          = rcSH.getUseBasePredictionFlag();
  m_bExisting             = true;
  m_bNeededForReference   = rcSH.getNalRefIdc() > 0;
  m_bOutputted            = false;
  m_bBaseRepresentation   = false;
  m_bConstrainedIntraPred = rcSH.getPPS().getConstrainedIntraPredFlag();
//JVT-T054{
  m_uiQualityLevel      = rcSH.getQualityLevel();
//JVT-T054}
  return Err::m_nOK;
}



ErrVal
DPBUnit::initNonEx( Int   iPoc,
                    UInt  uiFrameNum )
{
  m_pcFrame->setPoc( iPoc );
  m_uiFrameNum          = uiFrameNum;
  m_uiTemporalLevel     = MSYS_UINT_MAX;
  m_bUseBasePred        = false;
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
  m_uiFrameNum          = rcDPBUnit.m_uiFrameNum;
  m_uiTemporalLevel     = rcDPBUnit.m_uiTemporalLevel;
  m_bUseBasePred        = rcDPBUnit.m_bUseBasePred;
  m_bExisting           = rcDPBUnit.m_bExisting;
  m_bNeededForReference = rcDPBUnit.m_bNeededForReference;
  m_bOutputted          = rcDPBUnit.m_bOutputted;
  m_bConstrainedIntraPred = rcDPBUnit.m_bConstrainedIntraPred;
  m_bBaseRepresentation = true;
  //JVT-T054{
  m_uiQualityLevel      = rcDPBUnit.m_uiQualityLevel;
 //JVT-T054}

	const SliceHeader& rcSH = *rcDPBUnit.getCtrlData().getSliceHeader();
  const PicType ePicType  = rcSH.getPicType();
	RNOK( m_pcFrame->addFrameFieldBuffer() );
	m_pcFrame->setPoc( rcSH );
  RNOK( m_pcFrame->copy( pcFrameBaseRep, ePicType ) );

  return Err::m_nOK;
}



ErrVal
DPBUnit::uninit()
{
  m_uiFrameNum          = MSYS_UINT_MAX;
  m_uiTemporalLevel     = MSYS_UINT_MAX;
  m_bUseBasePred        = false;
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
, m_pcYuvBufferCtrl   ( NULL )
, m_uiNumRefFrames    ( 0 )
, m_uiMaxFrameNum     ( 0 )
, m_uiLastRefFrameNum ( MSYS_UINT_MAX )
, m_pcCurrDPBUnit     ( NULL )
, m_pcLastDPBUnit     ( NULL )
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

//JVT-T054{
ErrVal
DecodedPicBuffer::CreateDPBUnit(DPBUnit *pcDPBUnit, const SequenceParameterSet&  rcSPS )
{
  RNOK( DPBUnit::create( pcDPBUnit, *m_pcYuvBufferCtrl, rcSPS ) );
  return Err::m_nOK;
}
//JVT-T054}

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
  m_pcLastDPBUnit     = NULL;
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
  
  m_pcYuvBufferCtrl   = NULL;
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
    DPBUnit* pcDPBUnit = NULL;
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
		if( pcDPBUnit )
		{
			RNOK( pcDPBUnit->destroy() );
			pcDPBUnit = NULL;
    }
  }

  if( m_pcCurrDPBUnit )
  {
    RNOK( m_pcCurrDPBUnit->destroy() );
    m_pcCurrDPBUnit = NULL;
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
    const Bool bNoOutput = ( ! (*iter)->isExisting() || (*iter)->isBaseRep() || (*iter)->isOutputted() );
    const Bool bNonRef   = ( ! (*iter)->isNeededForRef() );
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
    AOT( pcDPBUnit == NULL)

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

    switch( eMmcoOp )
    {
    case MMCO_SHORT_TERM_UNUSED:
			RNOK( xMarkShortTermUnused( pcSliceHeader->getPicType(), m_pcCurrDPBUnit, uiVal1 ) );
      break;
    case MMCO_RESET:
    case MMCO_MAX_LONG_TERM_IDX:
    case MMCO_ASSIGN_LONG_TERM:
    case MMCO_LONG_TERM_UNUSED:
    case MMCO_SET_LONG_TERM:
    default:
      fprintf( stderr,"\nERROR: MMCO COMMAND currently not supported in the software\n\n" );
      RERR();
    }
  }

  return Err::m_nOK;
}



//JVT-S036 lsj start
ErrVal
DecodedPicBuffer::xMMCOBase( SliceHeader* pcSliceHeader, UInt mCurrFrameNum ) 
{
	ROF( pcSliceHeader );

  MmcoOp            eMmcoOp;
  const MmcoBuffer& rcMmcoBaseBuffer = pcSliceHeader->getMmcoBaseBuffer();
  Int               iIndex        = 0;
  UInt              uiVal1, uiVal2;

  while( MMCO_END != (eMmcoOp = rcMmcoBaseBuffer.get( iIndex++ ).getCommand( uiVal1, uiVal2 ) ) )
 {
		switch( eMmcoOp )
		{
		case MMCO_SHORT_TERM_UNUSED:
			RNOK( xMarkShortTermUnusedBase( pcSliceHeader->getPicType(), mCurrFrameNum, uiVal1 ) );
		break;
		case MMCO_RESET:
		case MMCO_MAX_LONG_TERM_IDX:
		case MMCO_ASSIGN_LONG_TERM:
		case MMCO_LONG_TERM_UNUSED:
		case MMCO_SET_LONG_TERM:
		default:
			fprintf( stderr,"\nERROR: MMCO COMMAND currently not supported in the software\n\n" );
		RERR();
		}
 }
  return Err::m_nOK;
}

ErrVal
DecodedPicBuffer::xMarkShortTermUnusedBase( const PicType eCurrentPicType, UInt mCurrFrameNum, UInt uiDiffOfPicNums ) 
{
//  ROF( pcCurrentDPBUnit );

  //UInt uiCurrPicNum = pcCurrentDPBUnit->getFrameNum();
// TMM_INTERLACE 
  UInt uiCurrPicNum = ( eCurrentPicType==FRAME ? mCurrFrameNum
		                                           : mCurrFrameNum*2+1 );
  UInt uiPicNumN     = uiCurrPicNum - uiDiffOfPicNums - 1;
	PicType ePicType;
	xSetIdentifier( uiPicNumN, ePicType, eCurrentPicType );


  DPBUnitList::iterator iter      = m_cUsedDPBUnitList.begin();
  DPBUnitList::iterator end       = m_cUsedDPBUnitList.end  ();
  for( ; iter != end; iter++ )
  {
	  if( (*iter)->isNeededForRef() && (*iter)->getFrameNum() == (Int)uiPicNumN && (*iter)->isBaseRep() ) 
    {
//bug-fix base_rep 0925{{ 
		DPBUnit* pcDPBUnit = (*iter);
   	RNOK( pcDPBUnit->uninit() );
		m_cUsedDPBUnitList.remove(pcDPBUnit);
		m_cFreeDPBUnitList.push_back( pcDPBUnit );
		return Err::m_nOK;  
//bug-fix base_rep 0925}} 
    }
  }
  return Err::m_nOK;
}

//JVT-S036 lsj end


ErrVal
DecodedPicBuffer::xMarkShortTermUnused( const PicType eCurrentPicType, const DPBUnit* pcCurrentDPBUnit, UInt uiDiffOfPicNums )
{
  ROF( pcCurrentDPBUnit );

  UInt uiCurrPicNum = ( eCurrentPicType==FRAME ? pcCurrentDPBUnit->getFrameNum()
		                                           : pcCurrentDPBUnit->getFrameNum()*2+1 );
  UInt uiPicNumN     = uiCurrPicNum - uiDiffOfPicNums - 1;

  DPBUnitList::iterator iter      = m_cUsedDPBUnitList.begin();
  DPBUnitList::iterator end       = m_cUsedDPBUnitList.end  ();
  for( ; iter != end; iter++ )
  {
    if( (*iter)->isNeededForRef() && (*iter)->getPicNum(uiCurrPicNum,m_uiMaxFrameNum) == (Int)uiPicNumN )
    {
      (*iter)->markNonRef();
    }
  }
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


//JVT-S036 lsj start
ErrVal
DecodedPicBuffer::xSlidingWindowBase( UInt mCurrFrameNum ) 
{
  //===== get number of reference frames =====
  DPBUnitList::iterator iter  = m_cUsedDPBUnitList.begin();
  DPBUnitList::iterator end   = m_cUsedDPBUnitList.end  ();
  DPBUnitList::iterator iiter;
  for( ; iter != end; iter++ )
  {
	  if( (*iter)->isBaseRep() && (*iter)->getFrameNum() != mCurrFrameNum )
    {
		for( iiter = m_cUsedDPBUnitList.begin(); iiter != end; iiter++ )
		{
			if ( (*iiter)->getFrameNum() == (*iter)->getFrameNum() && !(*iiter)->isBaseRep() )
			{
				if((*iter)->isNeededForRef())  
				{
//bug-fix base_rep 0925{{ 
					DPBUnit* pcDPBUnit = (*iter);
					RNOK( pcDPBUnit->uninit() );
					m_cUsedDPBUnitList.remove(pcDPBUnit);
					m_cFreeDPBUnitList.push_back( pcDPBUnit );
					return Err::m_nOK;  
//bug-fix base_rep 0925}} 
				}
			}
		}
    }
  }
  
  return Err::m_nOK;
}
//JVT-S036 lsj end 

ErrVal
DecodedPicBuffer::xOutput( PicBufferList&   rcOutputList,
                           PicBufferList&   rcUnusedList )
{
  ROTRS( m_cFreeDPBUnitList.size(), Err::m_nOK ); // no need for output
  
  //===== smallest non-ref/output poc value =====
  Int       iMinOutputPoc   = MSYS_INT_MAX;
  DPBUnit*  pcElemToRemove = NULL;
  DPBUnitList::iterator iter =  m_cUsedDPBUnitList.begin();
  DPBUnitList::iterator end  =  m_cUsedDPBUnitList.end  ();
  for( ; iter != end; iter++ )
  {
    const Bool bOutput  = ( !(*iter)->isOutputted() && (*iter)->isExisting() && !(*iter)->isBaseRep() && !(*iter)->isNeededForRef() );
    if(  bOutput && (*iter)->getFrame()->getPoc() < iMinOutputPoc )
    {
      iMinOutputPoc  = (*iter)->getFrame()->getPoc();
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
    const Bool bOutput = ( (*iter)->getFrame()->getPoc() <= iMinOutputPoc && !(*iter)->isOutputted() );
    if( bOutput )
    {
      if( !(*iter)->isOutputted() )
      {
        RNOK( (*iter)->markOutputted() );
      }
      if( (*iter)->isExisting() && !(*iter)->isBaseRep() )
      {
        cOutputList.push_back( *iter );
        if( (*iter)->getFrame()->getPoc() < iMinPoc )
        {
          iMinPoc = (*iter)->getFrame()->getPoc();
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
      if( (*iter)->getFrame()->getPoc() == iPoc )
      {
        DPBUnit* pcDPBUnit = *iter;
        cOutputList.remove( pcDPBUnit );

        //----- output -----
        ROT( m_cPicBufferList.empty() );
        PicBuffer*  pcPicBuffer = m_cPicBufferList.popFront();
        //JVT-T054{
        if(pcPicBuffer->isUsed())
          pcPicBuffer->setUnused(); 
        //JVT-T054}
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
                                   Int&             riMaxPoc,
                                   Bool             bFinal )
{
  //===== create output list =====
  DPBUnitList cOutputList;
  Int         iMinPoc = MSYS_INT_MAX;
  Int         iMaxPoc = MSYS_INT_MIN;
  DPBUnitList::iterator iter =  m_cUsedDPBUnitList.begin();
  DPBUnitList::iterator end  =  m_cUsedDPBUnitList.end  ();
  for( ; iter != end; iter++ )  
  {
    const Bool bOutput = ( !(*iter)->isOutputted() && (*iter)->isExisting() && !(*iter)->isBaseRep() );
    if(  bOutput )
    {
      cOutputList.push_back( *iter );
      if( (*iter)->getFrame()->getPoc() < iMinPoc ) iMinPoc = (*iter)->getFrame()->getPoc();
      if( (*iter)->getFrame()->getPoc() > iMaxPoc ) iMaxPoc = (*iter)->getFrame()->getPoc();
    }
  }

  //===== real output =====
  for( Int iPoc = iMinPoc; iPoc <= iMaxPoc; iPoc++ )
  {
    iter = cOutputList.begin();
    end  = cOutputList.end  ();
    for( ; iter != end; iter++ )
    {
      if( (*iter)->getFrame()->getPoc() == iPoc )
      {
        DPBUnit* pcDPBUnit = *iter;
        cOutputList.remove( pcDPBUnit );

        //----- output -----
        ROT( m_cPicBufferList.empty() );
        PicBuffer*  pcPicBuffer = m_cPicBufferList.popFront();
        pcDPBUnit->getFrame()->store( pcPicBuffer );
        rcOutputList.push_back( pcPicBuffer );
        rcUnusedList.push_back( pcPicBuffer );
//JVT-T054{
        if(pcPicBuffer->isUsed())
          pcPicBuffer->setUnused();
        pcDPBUnit->getCtrlData().setSliceHeader(NULL);
//JVT-T054}
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
    AOT( pcDPBUnit == NULL)
    m_cFreeDPBUnitList.push_back( pcDPBUnit );
  }

  ROTRS( ! bFinal , Err::m_nOK );

  // release remaining buffers
  while( m_cPicBufferList.size() )
  {
    rcUnusedList.push_back( m_cPicBufferList.popFront() );
  }
  return Err::m_nOK;
}
//JVT-T054{
ErrVal DecodedPicBuffer::xUpdateDPBUnitList(DPBUnit *pcDPBUnit)
{
  ROF( pcDPBUnit == m_pcCurrDPBUnit ); // check
  DPBUnit*  pcElemToReplace  = 0;
  DPBUnitList::iterator iter =  m_cUsedDPBUnitList.begin();
  DPBUnitList::iterator end  =  m_cUsedDPBUnitList.end  ();
  for( ; iter != end; iter++ )
  {
    if( !(*iter)->isBaseRep() && (*iter)->getFrame()->getPoc() == pcDPBUnit->getFrame()->getPoc() 
      && (*iter)->getQualityLevel()+1 == pcDPBUnit->getQualityLevel() )
    {
      pcElemToReplace  = (*iter);
      m_cUsedDPBUnitList.remove(pcElemToReplace);
      m_cFreeDPBUnitList.push_back(pcElemToReplace);
      m_cUsedDPBUnitList.push_back(pcDPBUnit);   
      break;
    }
  }
  return Err::m_nOK;
}
//JVT-T054}

ErrVal
DecodedPicBuffer::xStorePicture( DPBUnit*       pcDPBUnit,
                                 PicBufferList& rcOutputList,
                                 PicBufferList& rcUnusedList,
                                 Bool           bTreatAsIdr,
                                 Bool           bFrameMbsOnlyFlag ,//TMM_INTERLACE
                                 Bool           bRef)  //JVT-T054
                                 
{
  ROF( pcDPBUnit == m_pcCurrDPBUnit );

  //---- fill border ----
  RNOK( m_pcYuvBufferCtrl->initMb() );
//JVT-T054{
  if(!bRef)
  {
    //JVT-T054}
    const PicType ePicType          = pcDPBUnit->getFrame()->getPicType(); //TMM_INTERLACE
    
    RNOK( pcDPBUnit->getFrame()->extendFrame( NULL, ePicType, bFrameMbsOnlyFlag ) );
    
    if( bTreatAsIdr )
    {
      //===== IDR pictures =====
   /* // JVT-Q065 EIDR
      Int iDummy;
      RNOK( xClearOutputAll( rcOutputList, rcUnusedList, iDummy ) ); // clear and output all pictures
   */ 
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
//JVT-T054{
  }
  else
  {
    //replace previous version of current frame in usedDPBUnit by the new version
    RNOK( xUpdateDPBUnitList(pcDPBUnit));
    RNOK( xUpdateMemory( pcDPBUnit->getCtrlData().getSliceHeader() ) );        // memory update 
    RNOK( xOutput( rcOutputList, rcUnusedList ) );         // output
    m_pcCurrDPBUnit = m_cFreeDPBUnitList.popBack();      
  }
//JVT-T054}
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

  if( ! pcSliceHeader->getSPS().getGapsInFrameNumValueAllowedFlag() )
  {
    printf("\nLOST PICTURES = %d\n", uiMissingFrames );
    RERR();
  }
  else
  {
    for( UInt uiIndex = 1; uiIndex <= uiMissingFrames; uiIndex++ )
    {
      const Bool  bTreatAsIdr = ( m_cUsedDPBUnitList.empty() );
      const Int   iPoc        = ( bTreatAsIdr ? 0 : m_cUsedDPBUnitList.back()->getFrame()->getPoc() );
      const UInt  uiFrameNum  = ( m_uiLastRefFrameNum + uiIndex ) % m_uiMaxFrameNum;

      RNOK( m_pcCurrDPBUnit->initNonEx( iPoc, uiFrameNum ) );
      RNOK( xStorePicture( m_pcCurrDPBUnit, rcOutputList, rcUnusedList, bTreatAsIdr, pcSliceHeader->getSPS().getFrameMbsOnlyFlag() ) ); 
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
    printf("\tPOS=%d:\tFN=%d\tPoc=%d\t%s\t", iIndex, (*iter)->getFrameNum(), (*iter)->getFrame()->getPoc(), ((*iter)->isNeededForRef()?"REF":"   ") );
    
    if(  (*iter)->isOutputted() )   printf("Outputted  ");
    if( !(*iter)->isExisting () )   printf("NotExisting   ");
    if(  (*iter)->isBaseRep  () )   printf("BasRep   ");

    printf("\n");
  }
  printf("\n");
  return Err::m_nOK;
}




ErrVal
DecodedPicBuffer::xInitPrdListPSlice( RefFrameList& rcList0,
                                      IntFrame*     pcCurrentFrame,
																		  PicType       ePicType,
																			SliceType     eSliceType )
{
  rcList0.reset();

  const Bool  bBaseRep        = m_pcCurrDPBUnit->useBasePred ();
  const UInt  uiCurrFrameNum  = m_pcCurrDPBUnit->getFrameNum();

  if( ePicType==BOT_FIELD )
  {
		RNOK( rcList0.add( m_pcCurrDPBUnit->getFrame() ) );
  }

  //----- generate decreasing POC list -----
  for( Int iMaxPicNum = (Int)uiCurrFrameNum; true; )
  {
    DPBUnit*              pNext = NULL;
    DPBUnitList::iterator iter  = m_cUsedDPBUnitList.begin();
    DPBUnitList::iterator end   = m_cUsedDPBUnitList.end  ();
    for( ; iter != end; iter++ )
    {
      if( (*iter)->isNeededForRef() &&
          (*iter)->getPicNum(uiCurrFrameNum,m_uiMaxFrameNum) < iMaxPicNum &&
          ( !pNext || (*iter)->getPicNum(uiCurrFrameNum,m_uiMaxFrameNum)  > pNext->getPicNum(uiCurrFrameNum,m_uiMaxFrameNum) 
                   ||((*iter)->getPicNum(uiCurrFrameNum,m_uiMaxFrameNum) == pNext->getPicNum(uiCurrFrameNum,m_uiMaxFrameNum) && 
                   (*iter)->isBaseRep() == bBaseRep) ) )
      {
        pNext = (*iter);
      }
    }
    if( !pNext )
    {
      break;
    }
    iMaxPicNum = pNext->getPicNum(uiCurrFrameNum,m_uiMaxFrameNum);
    RNOK( rcList0.add( pNext->getFrame() ) );
  }

  if( ePicType!=FRAME )
  {
    RNOK( xSetInitialRefFieldList( rcList0, pcCurrentFrame, ePicType, eSliceType ) ) ;
  }

  return Err::m_nOK;
}

Void DecodedPicBuffer::xSetIdentifier( UInt& uiNum, PicType& rePicType, const PicType eCurrentPicType )
{
  if( eCurrentPicType==FRAME )
  {
    rePicType = FRAME;
  }
  else
  {
    if( uiNum % 2 ) 
    {
      rePicType = eCurrentPicType;
    }
    else if( eCurrentPicType==TOP_FIELD )
    {
      rePicType = BOT_FIELD;
    }
    else
    {
      rePicType = TOP_FIELD;
    }
    uiNum /= 2;
  }
}

ErrVal DecodedPicBuffer::xSetInitialRefFieldList( RefFrameList& rcList, IntFrame* pcCurrentFrame, PicType eCurrentPicType, SliceType eSliceType ) 
{
  RefFrameList cTempList = rcList; 
  rcList.reset();

  const PicType eOppositePicType  = ( eCurrentPicType==TOP_FIELD ? BOT_FIELD : TOP_FIELD );

  //----- initialize field list for short term pictures -----
  UInt uiCurrentParityIndex  = 0;
  UInt uiOppositeParityIndex = 0;

  while( uiCurrentParityIndex < cTempList.getSize() || uiOppositeParityIndex < cTempList.getSize() )
  {
    //--- current parity ---
    while( uiCurrentParityIndex < cTempList.getSize() )
    {
      IntFrame* pcFrame = cTempList.getEntry( uiCurrentParityIndex++ );

      // assume there are no dangling fields in the DPB except the current first field
      if( ( eSliceType==P_SLICE && pcFrame != pcCurrentFrame ) ||
				    eSliceType==B_SLICE )
      {
        RNOK( rcList.add( pcFrame->getPic( eCurrentPicType ) ) );
        break;
      }
    }
    //--- opposite parity ---
    if( uiOppositeParityIndex < cTempList.getSize() )
    {
      IntFrame* pcFrame = cTempList.getEntry( uiOppositeParityIndex++ );
			RNOK( rcList.add( pcFrame->getPic( eOppositePicType ) ) );
    }
  }

  return Err::m_nOK;
}

ErrVal
DecodedPicBuffer::xInitPrdListsBSlice( RefFrameList&  rcList0,
                                       RefFrameList&  rcList1,
                                       IntFrame*     pcCurrentFrame,
																			 PicType       eCurrentPicType,
																			 SliceType     eSliceType )
{
  rcList0.reset();
  rcList1.reset();

  RefFrameList  cDecreasingPocList;
  RefFrameList  cIncreasingPocList;
  const Bool  bBaseRep    = m_pcCurrDPBUnit->useBasePred();
  const Int   iCurrPoc    = m_pcCurrDPBUnit->getFrame()->getPic( eCurrentPicType )->getPoc();

  //----- generate decreasing POC list -----
  for( Int iMaxPoc = iCurrPoc; true; )
  {
    DPBUnit*              pNext = NULL;
    DPBUnitList::iterator iter  = m_cUsedDPBUnitList.begin();
    DPBUnitList::iterator end   = m_cUsedDPBUnitList.end  ();
    for( ; iter != end; iter++ )
    {
      if( (*iter)->isNeededForRef() &&
          (*iter)->getFrame()->getPoc() < iMaxPoc && 
          ( !pNext || (*iter)->getFrame()->getPoc() > pNext->getFrame()->getPoc() ) &&
          (*iter)->isBaseRep() == bBaseRep )
      {
        pNext = (*iter);
      }
    }
    if( !pNext )
    {
      break;
    }
    iMaxPoc = pNext->getFrame()->getPoc();
    RNOK( cDecreasingPocList.add( pNext->getFrame() ) );
  }

  //----- generate increasing POC list -----
  for( Int iMinPoc = iCurrPoc; true; )
  {
    DPBUnit*              pNext = NULL;
    DPBUnitList::iterator iter  = m_cUsedDPBUnitList.begin();
    DPBUnitList::iterator end   = m_cUsedDPBUnitList.end  ();
    for( ; iter != end; iter++ )
    {
      if( (*iter)->isNeededForRef() &&
          (*iter)->getFrame()->getPoc() > iMinPoc && 
          ( ! pNext || (*iter)->getFrame()->getPoc() < pNext->getFrame()->getPoc() ) &&
          (*iter)->isBaseRep() == bBaseRep )
      {
        pNext = (*iter);
      }
    }
    if( !pNext )
    {
      break;
    }
    iMinPoc = pNext->getFrame()->getPoc();
    RNOK( cIncreasingPocList.add( pNext->getFrame() ) );
  }

  //----- list 0 and list 1 -----
  UInt uiPos;
  for( uiPos = 0; uiPos < cDecreasingPocList.getSize(); uiPos++ )
  {
    RNOK( rcList0.add( cDecreasingPocList.getEntry( uiPos ) ) );
  }
  for( uiPos = 0; uiPos < cIncreasingPocList.getSize(); uiPos++ )
  {
    RNOK( rcList0.add( cIncreasingPocList.getEntry( uiPos ) ) );
    RNOK( rcList1.add( cIncreasingPocList.getEntry( uiPos ) ) );
  }
  for( uiPos = 0; uiPos < cDecreasingPocList.getSize(); uiPos++ )
  {
    RNOK( rcList1.add( cDecreasingPocList.getEntry(uiPos) ) );
  }

  if( eCurrentPicType!=FRAME )
  {
		RNOK( xSetInitialRefFieldList( rcList0, pcCurrentFrame, eCurrentPicType, eSliceType ) );
    RNOK( xSetInitialRefFieldList( rcList1, pcCurrentFrame, eCurrentPicType, eSliceType ) );
  }

  //----- check for element switching -----
  if( rcList1.getActive() >= 2 && rcList0.getActive() == rcList1.getActive() )
  {
    Bool bSwitch = true;
    for( uiPos = 0; uiPos < rcList1.getActive(); uiPos++ )
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
  ROTRS( 0 == rcList.getSize(), Err::m_nOK );

  ROF( pcSliceHeader );
  const RplrBuffer& rcRplrBuffer = pcSliceHeader->getRplrBuffer( eListIdx );

  //===== re-odering =====
  if( rcRplrBuffer.getRefPicListReorderingFlag() )
  {
    const PicType eCurrentPicType = pcSliceHeader->getPicType();
    UInt    uiPicNumPred          = ( eCurrentPicType==FRAME ? pcSliceHeader->getFrameNum() : 
                                                               pcSliceHeader->getFrameNum()*2+1 );
    UInt    uiMaxPicNum           = ( eCurrentPicType==FRAME ? m_uiMaxFrameNum : 2*m_uiMaxFrameNum );

    Bool  bBaseRep          = pcSliceHeader->getUseBasePredictionFlag();
    UInt  uiIndex           = 0;
    RplrOp  uiCommand;
    UInt  uiIdentifier      = 0;

    while( RPLR_END != ( uiCommand = rcRplrBuffer.get(uiIndex).getCommand(uiIdentifier) ) )
    {
      IntFrame* pcFrame = NULL;

      if( uiCommand == RPLR_LONG )
      {
        //===== long term index =====
        RERR(); // long-term indices are currently not supported by the software
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
            uiPicNumPred -= ( uiAbsDiff - uiMaxPicNum );
          }
          else
          {
            uiPicNumPred -=   uiAbsDiff;
          }
        }
        else // uiCommand == RPLR_POS
        {
          if( uiPicNumPred + uiAbsDiff > uiMaxPicNum - 1 )
          {
            uiPicNumPred += ( uiAbsDiff - uiMaxPicNum );
          }
          else
          {
            uiPicNumPred +=   uiAbsDiff;
          }
        }
        uiIdentifier = uiPicNumPred;
        
				PicType ePicType;
        xSetIdentifier( uiIdentifier, ePicType, eCurrentPicType );

        //----- get frame -----
        DPBUnitList::iterator iter = m_cUsedDPBUnitList.begin();
        DPBUnitList::iterator end  = m_cUsedDPBUnitList.end  ();
        for( ; iter != end; iter++ )
        {
          if( (*iter)->isNeededForRef() &&
              (*iter)->getFrameNum() == uiIdentifier &&
              (!pcFrame || (*iter)->isBaseRep() == bBaseRep ) )
          {
            pcFrame = (*iter)->getFrame()->getPic( ePicType );
          }
        }
        if( !pcFrame )
        {
          fprintf( stderr, "\nERROR: MISSING PICTURE !!!!\n\n");
          RERR();
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
        RNOK( rcList.setElementAndRemove( uiIndex, uiRemoveIndex, pcFrame ) );
        uiIndex++;
      } // short-term RPLR
    } // while command
  }

  //===== set final size =====
  //EIDR JVT-Q065
 // ROT( pcSliceHeader->getNumRefIdxActive( eListIdx ) > rcList.getActive() );
	rcList.setActive( pcSliceHeader->getNumRefIdxActive( eListIdx ) );

  return Err::m_nOK;
}



ErrVal
DecodedPicBuffer::xDumpRefList( RefFrameList& rcList,
                                ListIdx       eListIdx)
{
#if 1 // NO_DEBUG
  return Err::m_nOK;
#endif

  printf("LIST_%d={", eListIdx );
  for( UInt uiIndex = 1; uiIndex <= rcList.getActive(); uiIndex++ )
  {
		const PicType ePicType = rcList[uiIndex]->getPicType();
		printf(" %s POC=%4d,", ePicType==FRAME ? "FRAME" : ( ePicType==TOP_FIELD ? "TOP_FIELD" : "BOT_FIELD" ), rcList[uiIndex]->getPoc() );
  }
  printf(" }\n");
  return Err::m_nOK;
}

ErrVal
DecodedPicBuffer::initPicCurrDPBUnit( PicBuffer*&    rpcPicBuffer,
                                      Bool           bRef) //JVT-T054
{
    ROF( m_bInitDone );
    if(!bRef) //JVT-T054
    {
      //===== insert pic buffer in list =====
      m_cPicBufferList.push_back( rpcPicBuffer );
      rpcPicBuffer = 0;
    }
  SliceHeader* pcOldSH = m_pcCurrDPBUnit->getCtrlData().getSliceHeader();
  delete pcOldSH;
  m_pcCurrDPBUnit->getCtrlData().clear();
  m_pcCurrDPBUnit->getCtrlData().getMbDataCtrl()->reset();
  m_pcCurrDPBUnit->getCtrlData().getMbDataCtrl()->clear();

  return Err::m_nOK;
}


ErrVal
DecodedPicBuffer::initCurrDPBUnit( DPBUnit*&      rpcCurrDPBUnit,
                                   SliceHeader*   pcSliceHeader,
                                   PicBufferList& rcOutputList,
                                   PicBufferList& rcUnusedList,
                                   Bool           bRef) //JVT-T054
{
  ROF( m_bInitDone );

  Bool bNewFrame = true;
  if( NULL != m_pcLastDPBUnit )
  {
     bNewFrame = ! pcSliceHeader->isFieldPair( m_pcLastDPBUnit->getFrameNum(), m_pcLastDPBUnit->getPicType(), m_pcLastDPBUnit->isNalRefIdc() );
  }
  if( ! bNewFrame )
  {
    // reuse the DPBUnit in case of second field
    m_cFreeDPBUnitList.push_back( m_pcCurrDPBUnit );
		DPBUnitList::iterator iter = m_cUsedDPBUnitList.find( m_pcLastDPBUnit );
    if( iter != m_cUsedDPBUnitList.end() )
		{
			m_cUsedDPBUnitList.erase( iter );
		}
    m_pcCurrDPBUnit = m_pcLastDPBUnit;
  }

  if(!bRef) //JVT-T054
  {
    //===== check missing pictures =====
	  RNOK( xCheckMissingPics( pcSliceHeader, rcOutputList, rcUnusedList ) );
  }

  //===== initialize current DPB unit =====
  RNOK( m_pcCurrDPBUnit->init( *pcSliceHeader ) );

  ROT( pcSliceHeader->getUseBasePredictionFlag() && !pcSliceHeader->getNalRefIdc() ); // just a check
  m_pcCurrDPBUnit->getCtrlData().setSliceHeader( pcSliceHeader );
  m_pcCurrDPBUnit->setPicType( pcSliceHeader->getPicType() );
  m_pcCurrDPBUnit->setNalRefIdc( pcSliceHeader->getNalRefIdc() );

  if(!bRef) //JVT-T054
// JVT-Q065 EIDR{
  if( m_pcCurrDPBUnit->getCtrlData().getSliceHeader()->isIdrNalUnit() )
  {
	  //===== IDR pictures =====
	  Int iDummy;
	  RNOK( xClearOutputAll( rcOutputList, rcUnusedList, iDummy, false  ) ); // clear and output all pictures
  }
// JVT-Q065 EIDR}

  //===== set DPB unit =====
  rpcCurrDPBUnit = m_pcCurrDPBUnit;
  m_pcLastDPBUnit = m_pcCurrDPBUnit;
  return Err::m_nOK;
}



ErrVal
DecodedPicBuffer::clear( PicBufferList& rcOutputList,
                         PicBufferList& rcUnusedList,
                         Int&           riMaxPoc )
{
  RNOK( xClearOutputAll( rcOutputList, rcUnusedList, riMaxPoc, true ) ); // clear and output all pictures
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
  DPBUnit*              pcDPBUnit = NULL;
  DPBUnitList::iterator iter      = m_cUsedDPBUnitList.begin();
  DPBUnitList::iterator end       = m_cUsedDPBUnitList.end  ();
  for( ; iter != end; iter++ )
  {
    if( (*iter)->getFrame()->getPoc() == iPoc && !(*iter)->isBaseRep() )
    {
      pcDPBUnit = *iter;
      break;
    }
  }
  return pcDPBUnit;
}

//TMM_EC {{
ErrVal              
DecodedPicBuffer::getPrdRefListsFromBase( DPBUnit*    pcCurrDPBUnit, SliceHeader* pSliceHeaderBase , DPBUnit*    pcBaseDPBUnit)//TMM_EC
{
  ROF( m_pcCurrDPBUnit == pcCurrDPBUnit );
  ROF( m_pcCurrDPBUnit->getCtrlData().getSliceHeader() );

  RefFrameList& rcList0 = m_pcCurrDPBUnit->getCtrlData().getPrdFrameList( LIST_0 );
  RefFrameList& rcList1 = m_pcCurrDPBUnit->getCtrlData().getPrdFrameList( LIST_1 );
  UInt uiPos;
  rcList0.reset();
  rcList1.reset();
//TMM_EC {{
  if ( pcBaseDPBUnit )
  {
  RefFrameList& rcBaseList0 = pcBaseDPBUnit->getCtrlData().getPrdFrameList( LIST_0 );
  RefFrameList& rcBaseList1 = pcBaseDPBUnit->getCtrlData().getPrdFrameList( LIST_1 );

  //----- generate reference list0 -----
  for( uiPos = 0; uiPos < rcBaseList0.getActive(); uiPos++ )
  {
	  UInt uiPoc=rcBaseList0.getEntry(uiPos)->getPoc();
	  DPBUnit*              pNext = 0;
      DPBUnitList::iterator iter  = m_cUsedDPBUnitList.begin();
      DPBUnitList::iterator end   = m_cUsedDPBUnitList.end  ();
      for( ; iter != end; iter++ )
      {
  //			DPBUnit*              pTmp = 0;
        if((*iter)->getFrame()->getPoc()==uiPoc)
        {
          pNext = (*iter);
        }
      }
      rcList0.add( pNext->getFrame() );
    }

    //----- generate reference list1 -----

    for(      uiPos = 0; uiPos < rcBaseList1.getActive(); uiPos++ )
    {
	  UInt uiPoc=rcBaseList1.getEntry(uiPos)->getPoc();
	  DPBUnit*              pNext = 0;
      DPBUnitList::iterator iter  = m_cUsedDPBUnitList.begin();
      DPBUnitList::iterator end   = m_cUsedDPBUnitList.end  ();
      for( ; iter != end; iter++ )
      {
        if((*iter)->getFrame()->getPoc()==uiPoc)
        {
          pNext = (*iter);
        }
      }
      rcList1.add( pNext->getFrame() );
    }
  }
  else
  {
//TMM_EC }}
   	const PicType ePicType = pSliceHeaderBase->getPicType();

    RefPicList<RefPic>& rcBaseList0 =pSliceHeaderBase->getRefPicList(ePicType, LIST_0);
    RefPicList<RefPic>& rcBaseList1 =pSliceHeaderBase->getRefPicList(ePicType, LIST_1);

    //----- generate reference list0 -----
    for( uiPos = 0; uiPos < rcBaseList0.size(); uiPos++ )
    {
      UInt uiPoc=rcBaseList0.get(uiPos).getFrame()->getPoc();
      DPBUnit*              pNext = 0;
      DPBUnitList::iterator iter  = m_cUsedDPBUnitList.begin();
      DPBUnitList::iterator end   = m_cUsedDPBUnitList.end  ();
      for( ; iter != end; iter++ )
      {
        //			DPBUnit*              pTmp = 0;
        if((*iter)->getFrame()->getPoc()==uiPoc)
        {
          pNext = (*iter);
        }
      }
      rcList0.add( pNext->getFrame() );
    }
    
    //----- generate reference list1 -----
    
    for(      uiPos = 0; uiPos < rcBaseList1.size(); uiPos++ )
    {
      UInt uiPoc=rcBaseList1.get(uiPos).getFrame()->getPoc();
      DPBUnit*              pNext = 0;
      DPBUnitList::iterator iter  = m_cUsedDPBUnitList.begin();
      DPBUnitList::iterator end   = m_cUsedDPBUnitList.end  ();
      for( ; iter != end; iter++ )
      {
        if((*iter)->getFrame()->getPoc()==uiPoc)
        {
          pNext = (*iter);
        }
      }
      rcList1.add( pNext->getFrame() );
    }//TMM_EC
  }

  return Err::m_nOK;
}
//TMM_EC }}


ErrVal
DecodedPicBuffer::setPrdRefLists( DPBUnit* pcCurrDPBUnit )
{
  ROF( m_pcCurrDPBUnit == pcCurrDPBUnit );
  ROF( m_pcCurrDPBUnit->getCtrlData().getSliceHeader() );
  SliceHeader* pcSliceHeader = m_pcCurrDPBUnit->getCtrlData().getSliceHeader();
	ROF( pcSliceHeader );
  const PicType   ePicType   = pcSliceHeader->getPicType  ();
	const SliceType eSliceType = pcSliceHeader->getSliceType();

  RefFrameList& rcList0 = m_pcCurrDPBUnit->getCtrlData().getPrdFrameList( LIST_0 );
  RefFrameList& rcList1 = m_pcCurrDPBUnit->getCtrlData().getPrdFrameList( LIST_1 );

	IntFrame* pcCurrentFrame = m_pcCurrDPBUnit->getFrame();
  
  ROTRS( pcSliceHeader->isIntra(),   Err::m_nOK );

  if( pcSliceHeader->isInterP() )
  {
    RNOK( xInitPrdListPSlice( rcList0, pcCurrentFrame, ePicType, eSliceType ) );
    RNOK( xPrdListRemapping ( rcList0, LIST_0, pcSliceHeader                ) );
    RNOK( xDumpRefList      ( rcList0, LIST_0                               ) );
  }
  else
  {
    RNOK( xInitPrdListsBSlice( rcList0, rcList1, pcCurrentFrame, ePicType, eSliceType ) );
    RNOK( xPrdListRemapping  ( rcList0, LIST_0, pcSliceHeader                         ) );
    RNOK( xPrdListRemapping  ( rcList1, LIST_1, pcSliceHeader                         ) );
    RNOK( xDumpRefList       ( rcList0, LIST_0                                        ) );
    RNOK( xDumpRefList       ( rcList1, LIST_1                                        ) );
  }

  return Err::m_nOK;
}

ErrVal
DecodedPicBuffer::store( DPBUnit*&        rpcDPBUnit,
                         PicBufferList&   rcOutputList,
                         PicBufferList&   rcUnusedList,
                         IntFrame*        pcFrameBaseRep,
                         Bool             bRef) //JVT-T054
{
    RNOK( xStorePicture( rpcDPBUnit, 
                       rcOutputList, 
                       rcUnusedList,
                       rpcDPBUnit->getCtrlData().getSliceHeader()->isIdrNalUnit(),
                       /*uiQualityLevel,*/                         //JVT-T054
                       rpcDPBUnit->getCtrlData().getSliceHeader()->getSPS().getFrameMbsOnlyFlag(), bRef) ); //TMM_INTERLACE
 
  
  if( rpcDPBUnit->isNeededForRef() )
  {
    m_uiLastRefFrameNum = rpcDPBUnit->getFrameNum();
  }

  ROFRS( pcFrameBaseRep, Err::m_nOK );
  ROTRS( bRef,           Err::m_nOK );

  // Do not store the base representation if not specified in the stream
  ROFRS( rpcDPBUnit->getCtrlData().getSliceHeader()->getStoreBaseRepresentationFlag(), Err::m_nOK );

  //===== store base representation =====
  //--- get DPB unit ---
  if( m_cFreeDPBUnitList.empty() )
  {
    // not sure whether this always works ...
    RNOK( xOutput( rcOutputList, rcUnusedList ) );
  }
  DPBUnit* pcBaseRep = NULL;
  DPBUnitList::iterator iter1 = m_cUsedDPBUnitList.begin();
  DPBUnitList::iterator end1  = m_cUsedDPBUnitList.end  ();
  for( ; iter1 != end1; iter1++ )
  {
    if( ((*iter1)->getFrameNum() == rpcDPBUnit->getFrameNum() ) &&
      (  (*iter1)->isBaseRep  ()))
    {
      pcBaseRep = (*iter1);
      m_cUsedDPBUnitList.erase( iter1 );
      break;
    }
  }

  if( NULL == pcBaseRep )
  {
    pcBaseRep = m_cFreeDPBUnitList.popFront();
  }

  //--- init unit and extend picture ---
	const PicType ePicType          = rpcDPBUnit->getCtrlData().getSliceHeader()->getPicType();
 	const Bool    bFrameMbsOnlyFlag = rpcDPBUnit->getCtrlData().getSliceHeader()->getSPS().getFrameMbsOnlyFlag();
  RNOK( pcBaseRep->initBase( *rpcDPBUnit, pcFrameBaseRep ) );
  RNOK( m_pcYuvBufferCtrl->initMb() );
  RNOK( pcBaseRep->getFrame()->extendFrame( NULL, ePicType, bFrameMbsOnlyFlag ) );
  if(!bRef) //JVT-T054
  {
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
  //JVT-T054{
  } 

  return Err::m_nOK;
}



ErrVal
DecodedPicBuffer::update( DPBUnit*  pcDPBUnit )
{
  ROF( pcDPBUnit );

  //---- fill border ----
  RNOK( m_pcYuvBufferCtrl->initMb() );
	const PicType ePicType          = pcDPBUnit->getCtrlData().getSliceHeader()->getPicType();
	const Bool    bFrameMbsOnlyFlag = pcDPBUnit->getCtrlData().getSliceHeader()->getSPS().getFrameMbsOnlyFlag();
  RNOK( pcDPBUnit->getFrame()->extendFrame( NULL, ePicType, bFrameMbsOnlyFlag ) );

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
, m_bReconstructAll               ( false )
, m_uiFrameWidthInMb              ( 0 )
, m_uiFrameHeightInMb             ( 0 )
, m_uiMbNumber                    ( 0 )
, m_pcResidual                    ( 0 )
, m_pcILPrediction                ( 0 )
, m_pcPredSignal                  ( 0 )
, m_pcBaseLayerResidual           ( 0 )
, m_pcBaseLayerFrame              ( 0 )
, m_pcBaseLayerCtrl               ( 0 )
, m_pcBaseLayerCtrlField          ( 0 )
, m_pcCurrDPBUnit                 ( 0 )
, m_pcBaseLayerCtrlEL             ( 0 )	// ICU/ETRI FGS_MOT_USE
, m_uiLayerId                     ( 0 )
, m_bActive                       ( false )
, m_uiQualityLevelForPrediction    ( 3 )
, m_pcResizeParameter             ( 0 ) //TMM_ESS
, m_iMbProcessed                           (-1) //--ICU/ETRI FMO Implementation
, m_bIsNewPic						(true)
, m_bAVCBased                     ( false ) //JVT-T054
{
  ::memset( m_apcFrameTemp, 0x00, sizeof( m_apcFrameTemp ) );
//TMM_EC {{
  m_pcVeryFirstSliceHeader = NULL;
	m_bBaseLayerLost	=	false;
  m_bEnhanceAvailable = false;
//TMM_EC }}
//JVT-T054{
  UInt uiFGSLayer;
  for(uiFGSLayer = 0; uiFGSLayer < MAX_FGS_LAYERS+1; uiFGSLayer++)
  {
    m_pcResizeParameterCGSSNR[uiFGSLayer] = 0;
  }
//JVT-T054}
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
                   QuarterPelFilter*    pcQuarterPelFilter
				   //JVT-U106 Behaviour at slice boundaries{
				   ,ReconstructionBypass* pcReconstructionBypass
				   //JVT-U106 Behaviour at slice boundaries}
				   )
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
  //JVT-U106 Behaviour at slice boundaries{
  ROT( NULL == pcReconstructionBypass );
  //JVT-U106 Behaviour at slice boundaries}

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
  //JVT-U106 Behaviour at slice boundaries{
  m_pcReconstructionBypass        = pcReconstructionBypass;
  //JVT-U106 Behaviour at slice boundaries}

  m_bInitDone                     = true;
  m_bCreateDone                   = false;
  m_bWaitForIdr                   = true;
  m_bActive                       = false;
  m_uiFrameWidthInMb              = 0;
  m_uiFrameHeightInMb             = 0;
  m_uiMbNumber                    = 0;
  m_pcResidual                    = NULL;
  
  m_pcILPrediction                = NULL;
  m_pcBaseLayerFrame              = NULL;
  m_pcBaseLayerResidual           = NULL;
  m_pcPredSignal                  = NULL;
  m_pcBaseLayerCtrl               = NULL;
  m_pcBaseLayerCtrlField          = NULL;
  m_pcCurrDPBUnit                 = NULL;

  m_uiLayerId                     = 0;

  m_iMbProcessed                  = -1;

  m_bIsNewPic					  = true;
  m_bAVCBased                     = false; //JVT-T054
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
// {{ TMM_EC
	delete m_pcVeryFirstSliceHeader;
  m_pcVeryFirstSliceHeader = NULL;
// }}TMM_EC
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
  m_uiMbNumber        = rcSH->getMbInPic();

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
//TMM_EC {{
	if( NULL == m_pcVeryFirstSliceHeader )
	{
		m_pcVeryFirstSliceHeader  = new SliceHeader( rpcSliceHeader->getSPS(), rpcSliceHeader->getPPS() );
	}
//TMM_EC }}
  m_pcH264AVCDecoder->setRCDO( rpcSliceHeader );

  m_pcH264AVCDecoder->set4Tap( rpcSliceHeader );  // V090


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

//TMM_INTERLACE {
ErrVal
MCTFDecoder::getBaseLayerDataAvailability ( IntFrame*&    pcFrame,
                                            IntFrame*&    pcResidual,
                                            MbDataCtrl*&  pcMbDataCtrl,
                                            Bool&         bBaseDataAvailable,
                                            Bool          bSpatialScalability,
                                            Int           iPoc)
{
  pcFrame                     = NULL;
  pcResidual                  = NULL;
  pcMbDataCtrl                = NULL;
 
  DPBUnit*      pcBaseDPBUnit = m_pcDecodedPictureBuffer->getDPBUnit( iPoc );
  ROF( pcBaseDPBUnit );

  pcFrame       = m_pcILPrediction;
  pcResidual    = m_pcResidual;
  pcMbDataCtrl  = pcBaseDPBUnit->getCtrlData().getMbDataCtrl  ();


  bBaseDataAvailable =  pcFrame && pcResidual && pcMbDataCtrl;

  return Err::m_nOK;
}
//TMM_INTERLACE }

//TMM_EC {{
ErrVal
MCTFDecoder::getBaseLayerUnit(Int iPoc, DPBUnit   *&pcBaseDPBUnit)
{
  pcBaseDPBUnit = m_pcDecodedPictureBuffer->getDPBUnit( iPoc );
  return Err::m_nOK;
}

//TMM_EC }}

ErrVal
MCTFDecoder::getBaseLayerData ( IntFrame*&    pcFrame,
                                IntFrame*&    pcResidual,
                                MbDataCtrl*&  pcMbDataCtrl,
                                MbDataCtrl*&  pcMbDataCtrlEL,		// ICU/ETRI FGS_MOT_USE
                                Bool&         rbConstrainedIPred,
                                Bool          bSpatialScalability,
                                Int           iPoc )
{

  DPBUnit*      pcBaseDPBUnit = m_pcDecodedPictureBuffer->getDPBUnit( iPoc );
  ROF( pcBaseDPBUnit );

  pcMbDataCtrl                = pcBaseDPBUnit->getCtrlData().getMbDataCtrl ();
  SliceHeader*  pcSliceHeader = pcBaseDPBUnit->getCtrlData().getSliceHeader();
	const PicType ePicType      = pcSliceHeader->getPicType                  ();
  rbConstrainedIPred          = pcBaseDPBUnit->isConstrIPred               ();
  pcFrame       = m_pcILPrediction;
  pcResidual    = m_pcResidual;
  pcMbDataCtrlEL  = m_pcBaseLayerCtrlEL;// TMM_INTERLACE

  if( bSpatialScalability )
  {
    RNOK(  m_apcFrameTemp[0]->copy( pcFrame, ePicType ) );
    pcFrame = m_apcFrameTemp[0];

    if( pcSliceHeader->getPPS().getConstrainedIntraPredFlag() )
    {
      m_pcLoopFilter->setFilterMode( LoopFilter::LFMode( LoopFilter::LFM_NO_INTER_FILTER + LoopFilter::LFM_EXTEND_INTRA_SUR ) );
	  RNOK( m_pcLoopFilter->process(*pcSliceHeader,
                                     pcFrame,
                                     pcMbDataCtrl,
                                     pcMbDataCtrl,
                                     m_uiFrameWidthInMb,
                                     NULL,
                                     NULL,
									                   false,
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
									                   false,
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
  	RNOK( IntFrame::create( m_apcFrameTemp  [ uiIndex ], *m_pcYuvFullPelBufferCtrl, *m_pcYuvFullPelBufferCtrl, FRAME ) );
    RNOK  (   m_apcFrameTemp  [ uiIndex ]   ->init        () );
  }

  RNOK( IntFrame::create( m_pcResidual,          *m_pcYuvFullPelBufferCtrl, *m_pcYuvFullPelBufferCtrl, FRAME ) );
  RNOK( IntFrame::create( m_pcILPrediction,      *m_pcYuvFullPelBufferCtrl, *m_pcYuvFullPelBufferCtrl, FRAME ) );
  RNOK( IntFrame::create( m_pcPredSignal,        *m_pcYuvFullPelBufferCtrl, *m_pcYuvFullPelBufferCtrl, FRAME ) );
  RNOK( IntFrame::create( m_pcBaseLayerFrame,    *m_pcYuvFullPelBufferCtrl, *m_pcYuvFullPelBufferCtrl, FRAME ) );
  RNOK( IntFrame::create( m_pcBaseLayerResidual, *m_pcYuvFullPelBufferCtrl, *m_pcYuvFullPelBufferCtrl, FRAME ) );

  RNOK    (   m_pcResidual                  ->init        () );
  RNOK    (   m_pcILPrediction              ->init        () );
  RNOK    (   m_pcPredSignal                ->init        () );
  RNOK    (   m_pcBaseLayerFrame            ->init        () );
  RNOK    (   m_pcBaseLayerResidual         ->init        () );



  //========== CREATE MACROBLOCK DATA MEMORIES ==========
  ROFS   ( ( m_pcBaseLayerCtrl = new MbDataCtrl() ) );
  RNOK    (   m_pcBaseLayerCtrl ->init          ( rcSPS ) );

  ROFRS   ( ( m_pcBaseLayerCtrlField = new MbDataCtrl() ), Err::m_nERR );
  RNOK    (   m_pcBaseLayerCtrlField ->init          ( rcSPS ) );

  ROFS   ( ( m_pcBaseLayerCtrlEL = new MbDataCtrl() ) );
  RNOK    (   m_pcBaseLayerCtrlEL ->init          ( rcSPS ) );


  //========== CREATE UPDATE WEIGHTS ARRAY and WRITE BUFFER ==========
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
      RNOK( m_apcFrameTemp[ uiIndex ]->destroy() );
      m_apcFrameTemp[ uiIndex ] = NULL;

    }
  }

  if( m_pcResidual )
  {
    RNOK(   m_pcResidual->uninit() );
    RNOK( m_pcResidual->destroy() );
    m_pcResidual = NULL;
  }

  if( m_pcILPrediction )
  {
    RNOK(   m_pcILPrediction->uninit() );
    RNOK( m_pcILPrediction->destroy() );
    m_pcILPrediction = 0;
  }

  if( m_pcPredSignal )
  {
    RNOK(   m_pcPredSignal->uninit() );
    RNOK( m_pcPredSignal->destroy() );
    m_pcPredSignal = NULL;
  }

  if( m_pcBaseLayerFrame )
  {
    RNOK(   m_pcBaseLayerFrame->uninit() );
    RNOK( m_pcBaseLayerFrame->destroy() );
    m_pcBaseLayerFrame = NULL;
  }

  if( m_pcBaseLayerResidual )
  {
    RNOK(   m_pcBaseLayerResidual->uninit() );
    RNOK( m_pcBaseLayerResidual->destroy() );
    m_pcBaseLayerResidual = NULL;
  }


  //========== DELETE MACROBLOCK DATA MEMORIES (and SLICE HEADER) ==========
  if( m_pcBaseLayerCtrl )
  {
    RNOK( m_pcBaseLayerCtrl->uninit() );
    delete m_pcBaseLayerCtrl;
    m_pcBaseLayerCtrl = 0;
  }

  if( m_pcBaseLayerCtrlField )
  {
    RNOK( m_pcBaseLayerCtrlField->uninit() );
    delete m_pcBaseLayerCtrlField;
    m_pcBaseLayerCtrlField = NULL;
  }

  // ICU/ETRI FGS_MOT_USE
  if( m_pcBaseLayerCtrlEL )
  {
    RNOK( m_pcBaseLayerCtrlEL->uninit() );
    delete m_pcBaseLayerCtrlEL;
    m_pcBaseLayerCtrlEL = 0;
  }

  return Err::m_nOK;
}




ErrVal
MCTFDecoder::xZeroIntraMacroblocks( IntFrame*    pcFrame,
                                    ControlData& rcCtrlData )
{
  MbDataCtrl*       pcMbDataCtrl  = rcCtrlData.getMbDataCtrl       ();
  SliceHeader*      pcSliceHeader = rcCtrlData.getSliceHeader      ();

  RNOK( pcMbDataCtrl->initSlice( *pcSliceHeader, PRE_PROCESS, true, NULL ) );

  IntYuvMbBuffer cZeroMbBuffer;
  cZeroMbBuffer.setAllSamplesToZero();

	const PicType ePicType = pcSliceHeader->getPicType();
	const Bool    bMbAff   = pcSliceHeader->isMbAff   ();
	if( ePicType!=FRAME )
	{
		RNOK( pcFrame->addFieldBuffer     ( ePicType ) );
	}
	else if( bMbAff )
  {
		RNOK( pcFrame->addFrameFieldBuffer()           );
	}

	//===== loop over macroblocks =====
	for(UInt uiMbAddress = 0 ; uiMbAddress < m_uiMbNumber; uiMbAddress++ )
  {
    MbDataAccess* pcMbDataAccess = NULL;
    UInt          uiMbY, uiMbX;

    pcSliceHeader->getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddress              );

    RNOK( pcMbDataCtrl            ->initMb( pcMbDataAccess, uiMbY, uiMbX ) );
		RNOK( m_pcYuvFullPelBufferCtrl->initMb (                 uiMbY, uiMbX, bMbAff ) );

    if( pcMbDataAccess->getMbData().isIntra() )
    {
			const PicType eMbPicType = pcMbDataAccess->getMbPicType();
			pcFrame->getPic( eMbPicType )->getFullPelYuvBuffer()->loadBuffer( &cZeroMbBuffer );
    }
  }
	if( ePicType!=FRAME )
	{
		RNOK( pcFrame->removeFieldBuffer     ( ePicType ) );
	}
	else if( bMbAff )
	{
		RNOK( pcFrame->removeFrameFieldBuffer()           );
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
  IntYuvMbBuffer    cBLResBuffer;
  IntYuvMbBuffer    cMbBuffer;

  IntYuvPicBuffer* apcPicBuffer [4] = { NULL, NULL, NULL, NULL };
  IntYuvPicBuffer* apcBLResidual[4] = { NULL, NULL, NULL, NULL };

  RNOK( rcControlData.getBaseLayerSbb()->addFrameFieldBuffer() );
  RNOK( pcFrame                        ->addFrameFieldBuffer() );

  apcBLResidual[ TOP_FIELD ] = rcControlData.getBaseLayerSbb()->getPic( TOP_FIELD )->getFullPelYuvBuffer();
  apcBLResidual[ BOT_FIELD ] = rcControlData.getBaseLayerSbb()->getPic( BOT_FIELD )->getFullPelYuvBuffer();
  apcBLResidual[ FRAME     ] = rcControlData.getBaseLayerSbb()->getPic( FRAME     )->getFullPelYuvBuffer();

  apcPicBuffer[ TOP_FIELD ] = pcFrame->getPic( TOP_FIELD )->getFullPelYuvBuffer();
  apcPicBuffer[ BOT_FIELD ] = pcFrame->getPic( BOT_FIELD )->getFullPelYuvBuffer();
  apcPicBuffer[ FRAME     ] = pcFrame->getPic( FRAME     )->getFullPelYuvBuffer();

  RNOK( pcMbDataCtrl->initSlice( *pcSliceHeader, PRE_PROCESS, true, NULL ) );

  const Bool bMbAff = pcSliceHeader->isMbAff();

	//===== loop over macroblocks =====
		for(UInt uiMbAddress = 0 ; uiMbAddress < m_uiMbNumber; uiMbAddress++ )
  {
    MbDataAccess* pcMbDataAccess = NULL;
    UInt          uiMbY, uiMbX;

    pcSliceHeader->getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddress              );

    RNOK( pcMbDataCtrl            ->initMb( pcMbDataAccess, uiMbY, uiMbX ) );
		RNOK( m_pcYuvFullPelBufferCtrl->initMb (                 uiMbY, uiMbX, bMbAff ) );

    if( ! pcMbDataAccess->getMbData().isIntra() && pcMbDataAccess->getMbData().getResidualPredFlag( PART_16x16 ) )
    {
			const PicType eMbPicType = pcMbDataAccess->getMbPicType();
			cMbBuffer   .loadBuffer ( apcPicBuffer [ eMbPicType ] );
			cBLResBuffer.loadBuffer ( apcBLResidual[ eMbPicType ] );
      cMbBuffer   .add        ( cBLResBuffer );
			apcPicBuffer[ eMbPicType ]->loadBuffer ( &cMbBuffer );
    }
  }
  RNOK( rcControlData.getBaseLayerSbb()->removeFrameFieldBuffer() );
  RNOK( pcFrame                        ->removeFrameFieldBuffer() );

  return Err::m_nOK;
}



ErrVal
MCTFDecoder::initSlice( SliceHeader* pcSliceHeader, UInt uiLastLayer, Bool bLastNalInAU, Bool bCGSSNRInAU ) //JVT-T054
{
  ROFRS( m_bActive, Err::m_nOK );

  //===== calculate Poc =====
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
      pcSliceHeader->getPoc           () != m_pcRQFGSDecoder->getSliceHeader()->getPoc          ()        
      || pcSliceHeader->getSliceType() != F_SLICE //JVT-T054  
     ))
  {
    Bool bHighestLayer = ( uiLastLayer == m_uiLayerId && bLastNalInAU ); //JVT-T054
if (NULL == pcSliceHeader || 
    pcSliceHeader->getQualityLevel  () == m_pcRQFGSDecoder->getSliceHeader()->getQualityLevel () + 1 ||
    (pcSliceHeader->getQualityLevel  () != m_pcRQFGSDecoder->getSliceHeader()->getQualityLevel () + 1 &&
      (pcSliceHeader->getQualityLevel  () != m_pcRQFGSDecoder->getSliceHeader()->getQualityLevel ()
			|| 0 == pcSliceHeader->getQualityLevel  ())))
	{
    Bool bHighestMGSLayer = ( pcSliceHeader == 0 ||
                              pcSliceHeader->getQualityLevel() != m_pcRQFGSDecoder->getSliceHeader()->getQualityLevel() + 1 );
    RNOK( xReconstructLastFGS( bHighestLayer, bHighestMGSLayer ) ); //JVT-T054
	}	   
	
  }

  return Err::m_nOK;
}

//JVT-T054{
ErrVal
MCTFDecoder::setILPrediction(IntFrame * pcFrame, PicType ePicType )
{
  RNOK( m_pcILPrediction->copy( pcFrame, ePicType ) );
  return Err::m_nOK;
}
//JVT-T054}


//JVT-T054{
ErrVal
MCTFDecoder::ReconstructLastFGS( Bool bHighestLayer, Bool bHighestMGSLayer )
{
  DPBUnit*      pcLastDPBUnit   = m_pcDecodedPictureBuffer->getLastUnit();
  DPBUnit* pcTemp = m_pcDecodedPictureBuffer->getCurrDPBUnit();
  m_pcDecodedPictureBuffer->setCurrDPBUnit(pcLastDPBUnit);
  m_pcDecodedPictureBuffer->setPrdRefLists( pcLastDPBUnit );
  RNOK(xReconstructLastFGS(bHighestLayer, bHighestMGSLayer) );
  m_pcDecodedPictureBuffer->setCurrDPBUnit(pcTemp);
  return Err::m_nOK;
}
//JVT-T054}

ErrVal
MCTFDecoder::xMotionCompensation( IntFrame*     pcMCFrame,
                                  RefFrameList& rcRefFrameList0,
                                  RefFrameList& rcRefFrameList1,
                                  MbDataCtrl*   pcMbDataCtrl,
                                  SliceHeader&  rcSH, 
                                  Bool          bSR  )
{
  RNOK( pcMbDataCtrl          ->initSlice( rcSH, PRE_PROCESS, true, NULL ) );
  RNOK( m_pcMotionCompensation->initSlice( rcSH              ) );

  MbDataCtrl*      pcBaseMbDataCtrl = getBaseMbDataCtrl();

//TMM_INTERLACE{
	const Bool    bMbAff   = rcSH.isMbAff();

	const PicType ePicType = rcSH.getPicType();

	RefFrameList* apcRefFrameList0[4] = { NULL, NULL, NULL, NULL };
  RefFrameList* apcRefFrameList1[4] = { NULL, NULL, NULL, NULL };
  
  if( bMbAff )
  {
    RefFrameList acRefFrameList0[2];
    RefFrameList acRefFrameList1[2];
  
    RNOK( gSetFrameFieldLists( acRefFrameList0[0], acRefFrameList0[1], rcRefFrameList0 ) );
    RNOK( gSetFrameFieldLists( acRefFrameList1[0], acRefFrameList1[1], rcRefFrameList1 ) );
  
    apcRefFrameList0[ TOP_FIELD ] = ( 0 == rcRefFrameList0.getSize() ) ? NULL : &acRefFrameList0[0];
    apcRefFrameList0[ BOT_FIELD ] = ( 0 == rcRefFrameList0.getSize() ) ? NULL : &acRefFrameList0[1];
    apcRefFrameList1[ TOP_FIELD ] = ( 0 == rcRefFrameList1.getSize() ) ? NULL : &acRefFrameList1[0];
    apcRefFrameList1[ BOT_FIELD ] = ( 0 == rcRefFrameList1.getSize() ) ? NULL : &acRefFrameList1[1];
    apcRefFrameList0[     FRAME ] = &rcRefFrameList0;
    apcRefFrameList1[     FRAME ] = &rcRefFrameList1;

		RNOK( pcMCFrame->addFrameFieldBuffer() );
	}
	else
	{
		RNOK( pcMCFrame->addFieldBuffer( ePicType ) );
    apcRefFrameList0[ ePicType ] = &rcRefFrameList0;
    apcRefFrameList1[ ePicType ] = &rcRefFrameList1;
	}
//TMM_INTERLACE}

	//===== loop over macroblocks =====
	for(UInt uiMbAddress = 0 ; uiMbAddress < m_uiMbNumber; uiMbAddress++ )
  {
    MbDataAccess* pcMbDataAccess = NULL;
    MbDataAccess* pcMbDataAccessBase  = 0;
    UInt          uiMbY, uiMbX;

    rcSH.getMbPositionFromAddress( uiMbY, uiMbX, uiMbAddress              );

    RNOK( pcMbDataCtrl            ->initMb( pcMbDataAccess, uiMbY, uiMbX ) );
    if    ( pcBaseMbDataCtrl )
    {
      RNOK( pcBaseMbDataCtrl         ->initMb( pcMbDataAccessBase, uiMbY, uiMbX ) );
    }
		RNOK( m_pcYuvFullPelBufferCtrl->initMb (                 uiMbY, uiMbX, bMbAff ) );

    RNOK( m_pcMotionCompensation  ->initMb(                 uiMbY, uiMbX, *pcMbDataAccess ) );
    pcMbDataAccess->setMbDataAccessBase(pcMbDataAccessBase);
    if( ! pcMbDataAccess->getMbData().isIntra() )
    {
      IntYuvMbBuffer cYuvMbBuffer;

			const PicType eMbPicType = pcMbDataAccess->getMbPicType();
			RNOK( m_pcMotionCompensation->xCompensateMbAllModes( *pcMbDataAccess, *apcRefFrameList0[ eMbPicType ], *apcRefFrameList1[ eMbPicType ], &cYuvMbBuffer, bSR ) );
      RNOK( m_pcMotionCompensation->compensateMbBLSkipIntra(*pcMbDataAccess, &cYuvMbBuffer, m_pcBaseLayerFrame));
			RNOK( pcMCFrame->getPic( eMbPicType )->getFullPelYuvBuffer()->loadBuffer( &cYuvMbBuffer ) );
    }
  }

  if( pcMCFrame )             RNOK( pcMCFrame->            removeFrameFieldBuffer() );

  return Err::m_nOK;
}


ErrVal
MCTFDecoder::xReconstructLastFGS( Bool bHighestLayer, Bool bHighestMGSLayer ) //JVT-T054
{
  m_pcH264AVCDecoder->setRCDO( m_pcRQFGSDecoder->getSliceHeader() );

  m_pcH264AVCDecoder->set4Tap( m_pcRQFGSDecoder->getSliceHeader() );  // V090

  DPBUnit*      pcLastDPBUnit   = m_pcDecodedPictureBuffer->getLastUnit();
//JVT-T054{
  if(!pcLastDPBUnit)
  {
    RNOK( m_pcRQFGSDecoder->finishPicture () );
    return Err::m_nOK;
  }
//JVT-T054}
  SliceHeader*  pcSliceHeader   = m_pcRQFGSDecoder->getSliceHeader();
  const PicType ePicType      = pcSliceHeader->getPicType();
  Bool          bUseBaseRepFlag     = pcSliceHeader   ->getUseBasePredictionFlag(); // HS: fix by Nokia
  ROF( pcLastDPBUnit );
//JVT-T054{
  if(pcSliceHeader != pcLastDPBUnit->getCtrlData().getSliceHeader())
  {
    RNOK( m_pcRQFGSDecoder->finishPicture () );
    return Err::m_nOK;
  }
//JVT-T054}
  ROF( pcSliceHeader == pcLastDPBUnit->getCtrlData().getSliceHeader() );
  IntFrame*     pcFrame         = pcLastDPBUnit->getFrame    ();
  ControlData&  rcControlData   = pcLastDPBUnit->getCtrlData ();
  Bool          bConstrainedIP  = pcSliceHeader   ->getPPS().getConstrainedIntraPredFlag();

  //===== reconstruct FGS =====
  if( m_pcRQFGSDecoder->changed() || ! bUseBaseRepFlag && bConstrainedIP )
  {
    RNOK( m_pcRQFGSDecoder->reconstruct( pcFrame, true ) );

    RNOK( m_pcResidual    ->copy       ( pcFrame, ePicType ) );
    RNOK( xZeroIntraMacroblocks        ( m_pcResidual, rcControlData ) );

 
    if( m_bReconstructAll )
    {
      if( bUseBaseRepFlag && pcSliceHeader->isInterP() )
      {
        RefFrameList  cRefListDiff;

        setDiffPrdRefLists( cRefListDiff, m_pcYuvFullPelBufferCtrl );

        //----- key frames: adaptive motion-compensated prediction -----
        m_pcMotionCompensation->loadAdaptiveRefPredictors(m_pcYuvFullPelBufferCtrl, m_pcPredSignal, 
                                                          m_pcPredSignal, &cRefListDiff, 
                                                          m_pcRQFGSDecoder->getMbDataCtrl(), m_pcRQFGSDecoder, 
                                                          m_pcRQFGSDecoder->getSliceHeader());

        freeDiffPrdRefLists(cRefListDiff);
      }
      else if( ! pcSliceHeader->isIntra() )
      {
        setBaseMbDataCtrl(rcControlData.getBaseLayerCtrl());

        RNOK( xMotionCompensation( m_pcPredSignal,
                                  rcControlData.getPrdFrameList( LIST_0 ),
                                  rcControlData.getPrdFrameList( LIST_1 ),
                                  m_pcRQFGSDecoder->getMbDataCtrl(),
                                  *pcSliceHeader, true ) );
      }
    }

    //----- add prediction signal and clip -----
    RNOK( pcFrame         ->add        ( m_pcPredSignal, ePicType  ) );
    RNOK( pcFrame         ->clip       () );
  }
//JVT-S036 lsj start 
  if( bHighestMGSLayer && pcSliceHeader->getStoreBaseRepresentationFlag() )  //bug-fix suffix shenqiu
	{
		if( pcSliceHeader->getAdaptiveRefPicMarkingFlag() )
		{
			RNOK(m_pcDecodedPictureBuffer->xMMCOBase(pcSliceHeader, pcSliceHeader->getFrameNum()));
		}
		else
		{
			RNOK(m_pcDecodedPictureBuffer->xSlidingWindowBase( pcSliceHeader->getFrameNum() ));
		}
	}
//JVT-S036 lsj end

 //===== get reference frame lists =====
	RefFrameList& rcRefFrameList0 = rcControlData.getPrdFrameList( LIST_0 );
	RefFrameList& rcRefFrameList1 = rcControlData.getPrdFrameList( LIST_1 );
 RNOK( m_pcRQFGSDecoder->finishPicture () );

  //===== store intra signal for inter-layer prediction =====
  RNOK( m_pcILPrediction->copy( pcFrame, ePicType ) );

  //===== loop filter =====
  if( bHighestLayer || pcSliceHeader->getStoreBaseRepresentationFlag() ) // HS: fix by Nokia
  {
    m_pcLoopFilter->setHighpassFramePointer( m_pcResidual );
    RNOK( m_pcLoopFilter->process( *pcSliceHeader,
                                    pcFrame,
                                    ( pcSliceHeader->isIntra() ? NULL : rcControlData.getMbDataCtrl() ),
                                    rcControlData.getMbDataCtrl(),
                                    m_uiFrameWidthInMb,
                                    &rcRefFrameList0,
                                    &rcRefFrameList1,
								                    false,
                                    rcControlData.getSpatialScalability()) );
  }

  //===== update picture in DPB =====
  RNOK( m_pcDecodedPictureBuffer->update( pcLastDPBUnit ) );
  //TMM_INTERLACE {
  const Bool    bFrameMbsOnlyFlag = pcLastDPBUnit->getCtrlData().getSliceHeader()->getSPS().getFrameMbsOnlyFlag();
  RNOK( m_pcH264AVCDecoder->replaceSNRCGSBaseFrame( pcLastDPBUnit->getFrame(),ePicType, bFrameMbsOnlyFlag ) ); // MGS fix by Heiko Schwarz
  //TMM_INTERLACE }
  return Err::m_nOK;
}





ErrVal
MCTFDecoder::xDecodeFGSRefinement( SliceHeader*& rpcSliceHeader )
{
  ROFRS( m_pcRQFGSDecoder->isInitialized(), Err::m_nOK );

  const PicType ePicType = rpcSliceHeader->getPicType();

  //===== check slice header =====
  if(!m_pcRQFGSDecoder->isFinished    ()                                                                 &&
	    m_pcRQFGSDecoder->getSliceHeader()->getTemporalLevel()      == rpcSliceHeader->getTemporalLevel () &&
	    m_pcRQFGSDecoder->getSliceHeader()->getPoc          ()      == rpcSliceHeader->getPoc           () &&
     (   m_pcRQFGSDecoder->getSliceHeader()->getQualityLevel () + 1  == rpcSliceHeader->getQualityLevel  ()  ||
		     m_pcRQFGSDecoder->getSliceHeader()->getQualityLevel () == rpcSliceHeader->getQualityLevel  ()
	 )
	)
  {
    if( (Int)rpcSliceHeader->getQualityLevel() <= m_uiQualityLevelForPrediction )
    {
      printf("  Frame %4d ( LId%2d, TL%2d, QL%2d, PrRef,        MR %d, QP%3d, %s  )\n",
        rpcSliceHeader->getPoc                    (),
        rpcSliceHeader->getLayerId                (),
        rpcSliceHeader->getTemporalLevel          (),
        rpcSliceHeader->getQualityLevel           (),
        rpcSliceHeader->getAdaptivePredictionFlag (),
        rpcSliceHeader->getPicQp                  (),

				(ePicType==FRAME) ?  "Frame" : ( (ePicType==TOP_FIELD) ? "TopFd" : "BotFd") );

      if( m_pcRQFGSDecoder->getSliceHeader()->getTemporalLevel()  == 0      &&
        m_pcRQFGSDecoder->getSliceHeader()->isInterP())
      {
        // m_pcRQFGSDecoder->getSliceHeader has the slice header of the base layer
        m_pcRQFGSDecoder->getSliceHeader()->setBaseWeightZeroBaseBlock(rpcSliceHeader->getBaseWeightZeroBaseBlock() );
        m_pcRQFGSDecoder->getSliceHeader()->setBaseWeightZeroBaseCoeff(rpcSliceHeader->getBaseWeightZeroBaseCoeff() );
        m_pcRQFGSDecoder->getSliceHeader()->setLowPassFgsMcFilter     (rpcSliceHeader->getLowPassFgsMcFilter() );
        m_pcRQFGSDecoder->getSliceHeader()->setArFgsUsageFlag         (rpcSliceHeader->getArFgsUsageFlag() );


        if( rpcSliceHeader->getQualityLevel() == 1) 
        {
          m_pcRQFGSDecoder->getMbDataCtrl()->storeFgsBQLayerQpAndCbp();
          m_pcRQFGSDecoder->xStoreBQLayerSigMap();
        }
      }

      RNOK( m_pcRQFGSDecoder->decodeNextLayer( rpcSliceHeader ) );

      // ICU/ETRI FGS_MOT_USE
       // 2006.10.02 ICU/ETRI FGS_MOT_USE Bug Fix
      if ( m_pcRQFGSDecoder->getSliceHeader()->getQualityLevel() <= m_uiQualityLevelForPrediction )
      {
        m_pcBaseLayerCtrlEL->copyMotion(*(m_pcRQFGSDecoder->getMbDataCtrl()));
        m_pcBaseLayerCtrlEL->SetMbStride(m_pcRQFGSDecoder->getMbDataCtrl()->GetMbStride());
        m_pcBaseLayerCtrlEL->xSetDirect8x8InferenceFlag(m_pcRQFGSDecoder->getMbDataCtrl()->xGetDirect8x8InferenceFlagPublic());	
      }
    }
  }

  return Err::m_nOK;
}






ErrVal
MCTFDecoder::xInitESSandCroppingWindow( SliceHeader&  rcSliceHeader,
                                        MbDataCtrl&   rcMbDataCtrl,
                                        ControlData&    rcControlData)
{       
  UInt uiQualityLevel = rcSliceHeader.getQualityLevel();
  ResizeParameters * pcResizeParameter = (uiQualityLevel != 0 ? m_pcResizeParameterCGSSNR[uiQualityLevel] : m_pcResizeParameter);

  //TMM_INTERLACE {
  if( rcSliceHeader.getBaseLayerId() != MSYS_UINT_MAX )
  {
/*  pcResizeParameter->m_bFrameMbsOnlyFlag					= rcSliceHeader.getSPS().getFrameMbsOnlyFlag();
  pcResizeParameter->m_bFieldPicFlag						  = rcSliceHeader.getFieldPicFlag();
  pcResizeParameter->m_bIsMbAff                   = rcSliceHeader.isMbAff();
  pcResizeParameter->m_bBotFieldFlag							= rcSliceHeader.getBottomFieldFlag();
*/
  SliceHeader* pcSliceHeader                      = rcControlData.getSliceHeader();
  pcResizeParameter->m_bFrameMbsOnlyFlag					= pcSliceHeader->getSPS().getFrameMbsOnlyFlag();
  pcResizeParameter->m_bFieldPicFlag						  = pcSliceHeader->getFieldPicFlag();
  pcResizeParameter->m_bIsMbAff                   = pcSliceHeader->isMbAff();
  pcResizeParameter->m_bBotFieldFlag							= pcSliceHeader->getBottomFieldFlag();

  //TMM_INTERLACE }  
  }

  if( rcSliceHeader.getBaseLayerId() == MSYS_UINT_MAX )
  {
    for( Int iMbY = 0; iMbY < (Int)m_uiFrameHeightInMb; iMbY++ )
    for( Int iMbX = 0; iMbX < (Int)m_uiFrameWidthInMb;  iMbX++ )
    {
      rcMbDataCtrl.getMbDataByIndex( (UInt)iMbY*m_uiFrameWidthInMb+(UInt)iMbX ).setInCropWindowFlag( true );
    }
    return Err::m_nOK;
  }

  //===== clear cropping window flags =====
  for( Int iMbY = 0; iMbY < (Int)m_uiFrameHeightInMb; iMbY++ )
  for( Int iMbX = 0; iMbX < (Int)m_uiFrameWidthInMb;  iMbX++ )
  {
    rcMbDataCtrl.getMbDataByIndex( (UInt)iMbY*m_uiFrameWidthInMb+(UInt)iMbX ).setInCropWindowFlag( false );
  }


  //===== init resize parameter =====
  pcResizeParameter->setPictureParametersByOffset( rcSliceHeader.getPoc(),
                                                     rcSliceHeader.getLeftOffset(),
                                                     rcSliceHeader.getRightOffset(),
                                                     rcSliceHeader.getTopOffset(),
                                                     rcSliceHeader.getBottomOffset(),
                                                     rcSliceHeader.getBaseChromaPhaseX(),
                                                     rcSliceHeader.getBaseChromaPhaseY() );
  pcResizeParameter->setPoc( rcSliceHeader.getPoc() );
#ifdef _JVTV074_
  m_pcResizeParameter->setResampleFilterIdx( rcSliceHeader.getResampleFilterIdx( ) );
#endif //_JVTV074_

  //===== set crop window flag: in current macroblock data (we don't need the base layer here) =====
  if( pcResizeParameter->m_iSpatialScalabilityType == SST_RATIO_1 ||
      pcResizeParameter->m_iSpatialScalabilityType == SST_RATIO_2   )
  {
    Int iMbOrigX  = pcResizeParameter->m_iPosX      / 16;
    Int iMbOrigY  = pcResizeParameter->m_iPosY      / 16;
    Int iMbEndX   = pcResizeParameter->m_iOutWidth  / 16 + iMbOrigX;
    Int iMbEndY   = pcResizeParameter->m_iOutHeight / 16 + iMbOrigY;
    for( Int iMbY = iMbOrigY; iMbY < iMbEndY; iMbY++ )
    for( Int iMbX = iMbOrigX; iMbX < iMbEndX; iMbX++ )
    {
      rcMbDataCtrl.getMbDataByIndex( (UInt)iMbY*m_uiFrameWidthInMb+(UInt)iMbX ).setInCropWindowFlag( true );
    }
  }
  else
  {
    // ESS
    if( pcResizeParameter->m_iExtendedSpatialScalability == ESS_PICT )
    {
      pcResizeParameter->setCurrentPictureParametersWith( pcResizeParameter->getPoc() );  // really ugly
    }
    Int iScaledBaseOrigX  = pcResizeParameter->m_iPosX;
    Int iScaledBaseOrigY  = pcResizeParameter->m_iPosY;
    Int iScaledBaseWidth  = pcResizeParameter->m_iOutWidth;
    Int iScaledBaseHeight = pcResizeParameter->m_iOutHeight;
   
    
   //TMM_INTERLACE{
		if ( ! m_pcResizeParameter->m_bIsMbAff )
		{
//TMM_INTERLACE}
      for( Int iMbY = 0; iMbY < (Int)m_uiFrameHeightInMb; iMbY++ )
      for( Int iMbX = 0; iMbX < (Int)m_uiFrameWidthInMb;  iMbX++ )
      {
        if( ( iMbX >= ( iScaledBaseOrigX + 15 ) / 16 ) && ( iMbX < ( iScaledBaseOrigX + iScaledBaseWidth  ) / 16 ) &&
            ( iMbY >= ( iScaledBaseOrigY + 15 ) / 16 ) && ( iMbY < ( iScaledBaseOrigY + iScaledBaseHeight ) / 16 )   )
        {
        rcMbDataCtrl.getMbDataByIndex( (UInt)iMbY*m_uiFrameWidthInMb+(UInt)iMbX ).setInCropWindowFlag( true );
        }
      }
    //TMM_INTERLACE{
		}
		else
		{
			assert( m_uiFrameHeightInMb%2==0 );
			for( Int iMbY = 0; iMbY < (Int)m_uiFrameHeightInMb; iMbY+=2 )
			for( Int iMbX = 0; iMbX < (Int)m_uiFrameWidthInMb;  iMbX++ )
			{
				if( ( iMbX >= ( iScaledBaseOrigX + 15 ) / 16 ) && ( iMbX < ( iScaledBaseOrigX + iScaledBaseWidth  ) / 16 ) &&
						( iMbY >= ( iScaledBaseOrigY + 15 ) / 16 ) && ( (iMbY+1) < ( iScaledBaseOrigY + iScaledBaseHeight ) / 16 )   )
				{
					rcMbDataCtrl.getMbDataByIndex( (UInt)iMbY*m_uiFrameWidthInMb+(UInt)iMbX ).setInCropWindowFlag( true );
					rcMbDataCtrl.getMbDataByIndex( (UInt)(iMbY+1)*m_uiFrameWidthInMb+(UInt)iMbX ).setInCropWindowFlag( true );
				}
			}
		}
//TMM_INTERLACE}
  }

  return Err::m_nOK;
}

//TMM_EC {{
ErrVal
MCTFDecoder::getBaseLayerDPB(ControlData&    rcControlData,  DPBUnit *&pcBaseDPBUnit)
{
  RNOK( m_pcH264AVCDecoder->getBaseLayerUnit( rcControlData.getSliceHeader()->getBaseLayerId(), rcControlData.getSliceHeader()->getPoc(),   pcBaseDPBUnit ));  
  return Err::m_nOK;
}
//TMM_EC }}


ErrVal
MCTFDecoder::xInitBaseLayer( ControlData&    rcControlData,
						                	SliceHeader *&rcSliceHeaderBase )
{
  //===== init =====
  rcControlData.setBaseLayerRec ( 0 );
  rcControlData.setBaseLayerSbb ( 0 );
  rcControlData.setBaseLayerCtrl( 0 );
  rcControlData.setBaseLayerCtrlField( 0 );
  
  IntFrame*   pcBaseFrame         = 0;
  IntFrame*   pcBaseResidual      = 0;
  MbDataCtrl* pcBaseDataCtrl      = 0;
  MbDataCtrl* pcBaseDataCtrlEL     = 0;

  Bool        bConstrainedIPredBL = false;
  Bool        bSpatialScalability = false; 
  Bool        bBaseDataAvailable  = false;
  UInt uiQualityLevel = rcControlData.getSliceHeader()->getQualityLevel();
  ResizeParameters * pcResizeParameter = (uiQualityLevel != 0 ? m_pcResizeParameterCGSSNR[uiQualityLevel] : m_pcResizeParameter);
  
  SliceHeader* pcSliceHeader      = rcControlData.getSliceHeader();
  const PicType ePicType          = pcSliceHeader->getPicType();
 
  if( pcSliceHeader->getBaseLayerId() != MSYS_UINT_MAX)
  {
    m_pcLoopFilter->setRCDOSliceHeader( rcControlData.getSliceHeader() );
    xGetBaseLayerData( rcControlData,
                       pcBaseFrame, 
                        pcBaseResidual, 
                        pcBaseDataCtrl, 
                        pcBaseDataCtrlEL,
                        bBaseDataAvailable, 
                        bConstrainedIPredBL, 
                        bSpatialScalability,
                        pcResizeParameter); 
    m_pcLoopFilter->setRCDOSliceHeader();
   
    ROF( bBaseDataAvailable );
  }

  //===== motion data =====
  if( pcBaseDataCtrl )
  {
   if(pcResizeParameter->m_iExtendedSpatialScalability == ESS_PICT ) 
    {
        RefFrameList& rcList0=rcControlData.getPrdFrameList( LIST_0 );
        RefFrameList& rcList1=rcControlData.getPrdFrameList( LIST_1 );
  	 
	      UInt uiIndex;
        for( uiIndex = 1; uiIndex <= rcList0.getActive(); uiIndex++ )
        pcResizeParameter->m_aiRefListPoc[0][uiIndex-1]=rcList0[uiIndex]->getPoc() ;
        for( uiIndex = 1; uiIndex <= rcList1.getActive(); uiIndex++ )
        pcResizeParameter->m_aiRefListPoc[1][uiIndex-1]=rcList1[uiIndex]->getPoc() ;
    }

    //=== create Upsampled VBL Frame ===
    RNOK( m_pcBaseLayerCtrl->initSlice( *pcSliceHeader, PRE_PROCESS, false, NULL ) );
    RNOK( pcBaseDataCtrl->switchMotionRefinement() );
    m_pcBaseLayerCtrl->setBuildInterlacePred( pcResizeParameter->m_bFieldPicFlag );

    if( m_pcRQFGSDecoder->isUseFGS( pcSliceHeader->getBaseLayerId() ) )
    {
      pcBaseDataCtrlEL->setSliceHeader( pcBaseDataCtrl->getSliceHeader() );
      pcBaseDataCtrl = pcBaseDataCtrlEL;
    }

    RNOK( m_pcBaseLayerCtrl->upsampleMotion( *pcBaseDataCtrl, pcResizeParameter) );
    if(uiQualityLevel != 0)
      RNOK( m_pcBaseLayerCtrl->initMbCBP( *pcBaseDataCtrl, pcResizeParameter ) );
    
    //===== data needed for SVC to AVC translation====/
    if( pcBaseDataCtrl && rcControlData.getSliceHeader()->getAVCRewriteFlag() )
    {
      m_pcBaseLayerCtrl->copyTCoeffs( *pcBaseDataCtrl );
      m_pcBaseLayerCtrl->copyIntraPred( *pcBaseDataCtrl );
    }
    
    rcControlData.setBaseLayerCtrl( m_pcBaseLayerCtrl );

    //=== create Upsampled VBL Field ===
    if( pcResizeParameter->m_bIsMbAff )
    {
    RNOK( m_pcBaseLayerCtrlField->initSlice( *pcSliceHeader, PRE_PROCESS, false, NULL ) );
    m_pcBaseLayerCtrlField->setBuildInterlacePred( true );
    RNOK( m_pcBaseLayerCtrlField->upsampleMotion( *pcBaseDataCtrl, pcResizeParameter) );
    rcControlData.setBaseLayerCtrlField( m_pcBaseLayerCtrlField );
    }

    RNOK( pcBaseDataCtrl->switchMotionRefinement() );

    rcControlData.getSliceHeader()->setBaseLayerUsesConstrainedIntraPred( bConstrainedIPredBL );
  }

  //===== residual data =====
  if( bBaseDataAvailable )
  {
    RNOK( m_pcBaseLayerResidual->copy( pcBaseResidual, ePicType ) );

	  RNOK( m_pcBaseLayerResidual->upsampleResidual( m_cDownConvert, pcResizeParameter, pcBaseDataCtrl, false) ); 

	  if( !rcControlData.getSliceHeader()->getAVCRewriteFlag() )
      rcControlData.setBaseLayerSbb( m_pcBaseLayerResidual );
  }

  //===== reconstruction (intra) =====
  if( pcBaseFrame )
  {
  //JVT-U106 Behaviour at slice boundaries{
	if(rcControlData.getSliceHeader()->getCIUFlag())
	{
		xConstrainedIntraUpsampling(pcBaseFrame,m_pcBaseLayerFrame,m_apcFrameTemp[0],pcBaseDataCtrl,m_pcReconstructionBypass, pcResizeParameter, ePicType);
	}
	else
	{
    RNOK( m_pcBaseLayerFrame->copy( pcBaseFrame, ePicType ) );

    RNOK( m_pcBaseLayerFrame->upsample( m_cDownConvert, pcResizeParameter, true ) );
	}
  //JVT-U106 Behaviour at slice boundaries}

    rcControlData.setBaseLayerRec( m_pcBaseLayerFrame );
  }
  if(pcBaseDataCtrl==NULL) rcSliceHeaderBase=NULL;
  else rcSliceHeaderBase=pcBaseDataCtrl->getSliceHeader();

  setMCResizeParameters(m_pcResizeParameter);

  return Err::m_nOK;
}


Void MCTFDecoder::setMCResizeParameters   (ResizeParameters*				resizeParameters)
{
  m_pcMotionCompensation->setResizeParameters(resizeParameters);
} 


//JVT-S036 lsj start
ErrVal
MCTFDecoder::xDecodeSuffixUnit( SliceHeader*&  rpcSliceHeader,
                                        PicBuffer*&    rpcPicBuffer,
                                        PicBufferList& rcOutputList,
                                        PicBufferList& rcUnusedList,
                                        Bool           bReconstructionLayer )
{

	return Err::m_nOK;
}
//JVT-S036 lsj end

//JVT-T054_FIX
ErrVal
MCTFDecoder::GetAVCFrameForDPB( SliceHeader*&  rpcSliceHeader,
                                        PicBuffer*&    rpcPicBuffer,
                                        PicBufferList& rcOutputList,
                                        PicBufferList& rcUnusedList)
{

  //JVT-T054{
  Bool bRef = (rpcSliceHeader->getQualityLevel() != 0);
  //JVT-T054}
  m_uiNumLayers[0] = m_uiNumLayers[1];

  //===== init =====
    if (m_bIsNewPic)
    RNOK( m_pcDecodedPictureBuffer->initPicCurrDPBUnit( rpcPicBuffer, bRef) ); //JVT-T054

    RNOK( m_pcDecodedPictureBuffer->initCurrDPBUnit( m_pcCurrDPBUnit, rpcSliceHeader,
                                                   rcOutputList, rcUnusedList, bRef ) ); //JVT-T054
 
   //TMM_ESS {
   //----- initialize reference lists -----
   //BUG_FIX JV
	if(rpcSliceHeader->getTrueSlice() || rpcSliceHeader->getBaseLayerId() == MSYS_UINT_MAX)
  {
    RNOK( m_pcDecodedPictureBuffer->setPrdRefLists( m_pcCurrDPBUnit ) );
  //TMM_ESS }
    //JVT-T054{
//      RNOK( xInitESSandCroppingWindow( *rpcSliceHeader, *m_pcCurrDPBUnit->getCtrlData().getMbDataCtrl()) );
      RNOK( xInitESSandCroppingWindow( *rpcSliceHeader, *m_pcCurrDPBUnit->getCtrlData().getMbDataCtrl(),m_pcCurrDPBUnit->getCtrlData() ) );

    m_pcCurrDPBUnit->getCtrlData().setBaseLayerRec ( 0 );
    m_pcCurrDPBUnit->getCtrlData().setBaseLayerSbb ( 0 );
    m_pcCurrDPBUnit->getCtrlData().setBaseLayerCtrl( 0 );
  }
  ControlData&  rcControlData   = m_pcCurrDPBUnit->getCtrlData();
  IntFrame*     pcFrame         = m_pcCurrDPBUnit->getFrame   ();
  IntFrame*     pcResidual      = m_pcResidual;
  IntFrame*     pcBaseRepFrame  = m_apcFrameTemp[0];
  MbDataCtrl*   pcMbDataCtrl    = rcControlData.getMbDataCtrl();
  Bool          bUseBaseRepresentation     = rpcSliceHeader->getUseBasePredictionFlag();
 
  RNOK( m_pcH264AVCDecoder->getAVCFrame(pcFrame, pcResidual, pcMbDataCtrl, rpcSliceHeader->getPoc()));
  pcMbDataCtrl = rcControlData.getSliceHeader()->getFrameUnit()->getMbDataCtrl();
  
  m_pcCurrDPBUnit->getCtrlData().getMbDataCtrl()->copyMotion(*pcMbDataCtrl); // MGS fix by Heiko Schwarz (moved from end of function)
  m_pcCurrDPBUnit->getFrame()->copyAll(pcFrame);
  
	m_bIsNewPic = true;
  //----- store in decoded picture buffer -----
  if( bUseBaseRepresentation )
  {
    const PicType ePicType = rpcSliceHeader->getPicType();
    //----- copy non-filtered frame -----
    //RNOK( pcFrame->copy( pcBaseRepFrame, ePicType ) );
    RNOK( pcBaseRepFrame->copy( pcFrame, ePicType ) );  //MGS fix by Heiko Schwarz
    //----- store in DPB with base representation -----
    
    RNOK( m_pcDecodedPictureBuffer->store( m_pcCurrDPBUnit, rcOutputList, rcUnusedList, pcBaseRepFrame, bRef ) ); //JVT-T054
  }
  else
  {
    
    RNOK( m_pcDecodedPictureBuffer->store( m_pcCurrDPBUnit, rcOutputList, rcUnusedList, NULL, bRef ) ); //JVT-T054
  }

  //----- set slice header to zero (slice header is stored in control data) -----
  rpcSliceHeader = 0; 

  DTRACE_NEWFRAME;
  return Err::m_nOK;
}


ErrVal
MCTFDecoder::xDecodeBaseRepresentation( SliceHeader*&  rpcSliceHeader,
                                        PicBuffer*&    rpcPicBuffer,
                                        PicBufferList& rcOutputList,
                                        PicBufferList& rcUnusedList,
                                        Bool           bReconstructionLayer )
{
//TMM_EC {{
	RNOK( getECMethod( rpcSliceHeader, m_eErrorConcealTemp));
	rpcSliceHeader->m_eErrorConceal=m_eErrorConcealTemp;
	if ( !rpcSliceHeader->getTrueSlice() || m_eErrorConcealTemp!=EC_NONE)
	{
      return xDecodeBaseRepresentationVirtual( rpcSliceHeader, rpcPicBuffer, rcOutputList, rcUnusedList, bReconstructionLayer );
	}
//TMM_EC }} 

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

  const PicType ePicType = rpcSliceHeader->getPicType();

  //JVT-T054_FIX{
  Bool bRef = rpcSliceHeader->getQualityLevel() != 0;
  if(m_bAVCBased && m_uiLayerId == 0)
  {
    bRef = rpcSliceHeader->getQualityLevel() > 1;
  }
  //JVT-T054}

  //===== init =====
  if(isNewPictureStart(rpcSliceHeader)) //TMM_EC
    RNOK( m_pcDecodedPictureBuffer->initPicCurrDPBUnit( rpcPicBuffer, bRef) );

  RNOK( m_pcDecodedPictureBuffer->initCurrDPBUnit( m_pcCurrDPBUnit, rpcSliceHeader,
                                                   rcOutputList, rcUnusedList, bRef ) );
 
   //----- initialize reference lists -----
   //BUG_FIX JV
  SliceHeader * pcSliceHeaderBase= NULL;//TMM_EC
  RNOK( m_pcDecodedPictureBuffer->setPrdRefLists( m_pcCurrDPBUnit ) );
   //JVT-T054_FIX{

  m_pcCurrDPBUnit->getCtrlData().setBaseLayerRec ( 0 );
  m_pcCurrDPBUnit->getCtrlData().setBaseLayerSbb ( 0 );
  m_pcCurrDPBUnit->getCtrlData().setBaseLayerCtrl( 0 );
  m_pcCurrDPBUnit->getCtrlData().setBaseLayerCtrlField( 0 );//TMM_INTERLACE

  ControlData&  rcControlData   = m_pcCurrDPBUnit->getCtrlData();
  IntFrame*     pcFrame         = m_pcCurrDPBUnit->getFrame   ();
  IntFrame*     pcResidual      = m_pcResidual;
  IntFrame*     pcBaseRepFrame  = m_apcFrameTemp[0];
  MbDataCtrl*   pcMbDataCtrl    = rcControlData.getMbDataCtrl();
  UInt          uiMbRead        = 0;

  Bool          bUseBaseRepresentation     = rpcSliceHeader->getUseBasePredictionFlag();
  Bool          bConstrainedIP  = rpcSliceHeader->getPPS().getConstrainedIntraPredFlag();
  Bool          bReconstructAll = bReconstructionLayer || !bConstrainedIP;
  m_bReconstructAll             = bReconstructAll;
  //***** NOTE: Motion-compensated prediction for non-key pictures is done in xReconstructLastFGS()
  bReconstructAll = bReconstructAll && bUseBaseRepresentation || ! bConstrainedIP;
	
  if (isNewPictureStart(rpcSliceHeader))
  {
    m_iMbProcessed =0;
    m_bIsNewPic = false;	
  }
  // TMM_INTERLACE{

 RNOK( xInitESSandCroppingWindow( *rpcSliceHeader, *m_pcCurrDPBUnit->getCtrlData().getMbDataCtrl(),rcControlData ) );
// RNOK( xInitBaseLayer( m_pcCurrDPBUnit->getCtrlData(), pcSliceHeaderBase) );
 RNOK( rpcSliceHeader->ReadLastBit() ); 
 // TMM_INTERLACE}

 //----- parsing -----
  RNOK( m_pcControlMng  ->initSliceForReading ( *rpcSliceHeader ) );
  RNOK( m_pcSliceReader ->read                ( *rpcSliceHeader,
                                                pcMbDataCtrl,
                                                rcControlData.getSpatialScalabilityType(),
                                                m_uiFrameWidthInMb,
                                                uiMbRead ) );
  rpcSliceHeader->setNumMbsInSlice ( uiMbRead );
 
  //TMM_INTERLACE
  RNOK( xInitBaseLayer( m_pcCurrDPBUnit->getCtrlData(), pcSliceHeaderBase) );

  //----- decoding -----
  RNOK( m_pcControlMng  ->initSliceForDecoding( *rpcSliceHeader ) );

 	//===== get reference frame lists =====
	RefFrameList& rcRefFrameList0 = rcControlData.getPrdFrameList( LIST_0 );
	RefFrameList& rcRefFrameList1 = rcControlData.getPrdFrameList( LIST_1 );

	
  if( rpcSliceHeader->isMbAff() )
  {
  	RNOK( m_pcSliceDecoder->decodeMbAff( *rpcSliceHeader,
																					pcMbDataCtrl,
                                          rcControlData.getBaseLayerCtrl(),
                                          rcControlData.getBaseLayerCtrlField(),
                                          bUseBaseRepresentation ? pcBaseRepFrame : pcFrame,
                                          pcResidual,
                                          m_pcPredSignal,
																					rcControlData.getBaseLayerRec(),
																					rcControlData.getBaseLayerSbb(),
                                         &rcRefFrameList0,
                                         &rcRefFrameList1,
                                          bReconstructAll ) ); 
  }
  else
  {
    RNOK( m_pcSliceDecoder->decode              ( *rpcSliceHeader,
                                                pcMbDataCtrl,
                                                rcControlData.getBaseLayerCtrl(),
                                                bUseBaseRepresentation ? pcBaseRepFrame : pcFrame,
                                                pcResidual,
                                                m_pcPredSignal,
                                                rcControlData.getBaseLayerRec(),
                                                rcControlData.getBaseLayerSbb(),
                                               &rcRefFrameList0,
                                               &rcRefFrameList1,
                                                bReconstructAll,
                                                m_uiFrameWidthInMb,
                                                uiMbRead ) );
  }

  m_iMbProcessed += uiMbRead;

  printf("  %s %4d ( LId%2d, TL%2d, QL%2d, %s-%c, BId%2d, AP%2d, QP%3d )\n",
		(ePicType==FRAME) ?  "Frame" : ( (ePicType==TOP_FIELD) ? "TopFd" : "BotFd"),

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

  if(isPictureDecComplete(rpcSliceHeader)) //--TM prob
  {
	  m_bIsNewPic = true;

    Bool bSpecialFlag = ( bUseBaseRepresentation && m_uiLayerId == 0 && rpcSliceHeader->getQualityLevel() == 1 );
    if(  bSpecialFlag )
    {
      RNOK( pcFrame->copy( pcBaseRepFrame, ePicType ) );

      pcBaseRepFrame          = 0;
      IntFrame*   pDummyFrame = 0;
      MbDataCtrl* pDummyCtrl  = 0;
      RNOK( m_pcH264AVCDecoder->getAVCFrame( pcBaseRepFrame, pDummyFrame, pDummyCtrl, rpcSliceHeader->getPoc() ) );
    }

    //----- store in decoded picture buffer -----
    if( bUseBaseRepresentation )
    {
      if( ! bSpecialFlag )
      {
        //----- copy non-filtered frame -----
        RNOK( pcFrame->copy( pcBaseRepFrame, ePicType ) );

        //----- loop-filtering and store in DPB as base representation -----
        m_pcLoopFilter->setHighpassFramePointer( pcResidual );
		    RNOK( m_pcLoopFilter->process( *rpcSliceHeader,
                                      pcBaseRepFrame,
                                      ( rpcSliceHeader->isIntra() ? NULL : pcMbDataCtrl ),
                                      pcMbDataCtrl,
                                      m_uiFrameWidthInMb,
                                        &rcRefFrameList0,
                                      &rcRefFrameList1,
								                      false,
                                      rcControlData.getSpatialScalability() ) );
      }
      //----- store in DPB with base representation -----
      RNOK( m_pcDecodedPictureBuffer->store( m_pcCurrDPBUnit, rcOutputList, rcUnusedList, pcBaseRepFrame, bRef) );
    }
    else
    {
      RNOK( m_pcDecodedPictureBuffer->store( m_pcCurrDPBUnit, rcOutputList, rcUnusedList, NULL, bRef) );
    }


    //----- set slice header to zero (slice header is stored in control data) -----
     rpcSliceHeader = 0; 

    //----- init FGS decoder -----
	rcControlData.getSliceHeader()->FMOInit();
	RNOK( m_pcRQFGSDecoder->initPicture( rcControlData.getSliceHeader(), rcControlData.getMbDataCtrl() ) );
    RNOK( xAddBaseLayerResidual( rcControlData, m_pcRQFGSDecoder->getBaseLayerSbb() ) );
	}	// end DecComplete

  DTRACE_NEWFRAME;
  return Err::m_nOK;
}




//TMM_EC{{
ErrVal
MCTFDecoder::xDecodeBaseRepresentationVirtual( SliceHeader*&  rpcSliceHeader,
                                        PicBuffer*&    rpcPicBuffer,
                                        PicBufferList& rcOutputList,
                                        PicBufferList& rcUnusedList,
                                        Bool           bReconstructionLayer )
{
  DPBUnit *pcBaseDPBUnit=NULL;

	printf("  Frame %4d ( LId%2d, TL%2d, QL%2d, %s-%c, BId%2d, AP%2d, QP%3d  %c)\n",
		rpcSliceHeader->getPoc                    (),
		rpcSliceHeader->getLayerId                (),
		rpcSliceHeader->getTemporalLevel          (),
		rpcSliceHeader->getQualityLevel           (),
		rpcSliceHeader->isH264AVCCompatible       () ? "AVC" : "SVC",
		rpcSliceHeader->getSliceType              () == I_SLICE ? 'I' :
		rpcSliceHeader->getSliceType              () == P_SLICE ? 'P' : 'B',
		rpcSliceHeader->getBaseLayerId            (),
		rpcSliceHeader->getAdaptivePredictionFlag () ? 1 : 0,
		rpcSliceHeader->getPicQp                  (),
		rpcSliceHeader->getTrueSlice() ? 'R' : 'V'); //TMM_EC
	

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

  const PicType ePicType = rpcSliceHeader->getPicType();
 

  //===== init =====
  //tttest
//  if (m_bIsNewPic)    // may be modified later                                
  if(isNewPictureStart(rpcSliceHeader)  )
  {
    RNOK( m_pcDecodedPictureBuffer->initPicCurrDPBUnit( rpcPicBuffer) );
  }

  RNOK( m_pcDecodedPictureBuffer->initCurrDPBUnit( m_pcCurrDPBUnit, rpcSliceHeader,
                                                   rcOutputList, rcUnusedList ) );

   //----- initialize reference lists -----
   //BUG_FIX JV
  SliceHeader * pcSliceHeaderBase= NULL;//TMM_EC
	if( rpcSliceHeader->getBaseLayerId() == MSYS_UINT_MAX)
  {
    RNOK( m_pcDecodedPictureBuffer->setPrdRefLists( m_pcCurrDPBUnit ) );
  //TMM_ESS }
//    RNOK( xInitESSandCroppingWindow( *rpcSliceHeader, *m_pcCurrDPBUnit->getCtrlData().getMbDataCtrl() ) );
    RNOK( xInitESSandCroppingWindow( *rpcSliceHeader, *m_pcCurrDPBUnit->getCtrlData().getMbDataCtrl(),m_pcCurrDPBUnit->getCtrlData() ) );

    m_pcCurrDPBUnit->getCtrlData().setBaseLayerRec ( 0 );
    m_pcCurrDPBUnit->getCtrlData().setBaseLayerSbb ( 0 );
    m_pcCurrDPBUnit->getCtrlData().setBaseLayerCtrl( 0 );
  }
  else
  { 
    RNOK( xInitBaseLayer( m_pcCurrDPBUnit->getCtrlData(),pcSliceHeaderBase) );
		if (pcSliceHeaderBase != NULL)
		{
			rpcSliceHeader->setNumRefIdxActive( LIST_0, pcSliceHeaderBase->getNumRefIdxActive(LIST_0));
			if ( rpcSliceHeader->isInterB())
			{
  			rpcSliceHeader->setNumRefIdxActive( LIST_1, pcSliceHeaderBase->getNumRefIdxActive(LIST_1));
			}
      RNOK( getBaseLayerDPB(m_pcCurrDPBUnit->getCtrlData(), pcBaseDPBUnit));
  	  RNOK( m_pcDecodedPictureBuffer->getPrdRefListsFromBase(m_pcCurrDPBUnit,pcSliceHeaderBase, pcBaseDPBUnit));  
		}
  }

  ControlData&  rcControlData   = m_pcCurrDPBUnit->getCtrlData();
  IntFrame*     pcFrame         = m_pcCurrDPBUnit->getFrame   ();
  IntFrame*     pcResidual      = m_pcResidual;
  IntFrame*     pcBaseRepFrame  = m_apcFrameTemp[0];  
  MbDataCtrl*   pcMbDataCtrl    = rcControlData.getMbDataCtrl();
  UInt          uiMbRead        = 0;

  MbDataCtrl*   pcMbDataCtrlRef = NULL;//TMM_EC 


  Bool          bUseBaseRepresentation     = rpcSliceHeader->getUseBasePredictionFlag();
  Bool          bConstrainedIP  = rpcSliceHeader->getPPS().getConstrainedIntraPredFlag();
  Bool          bReconstructAll = bReconstructionLayer || !bConstrainedIP;
  m_bReconstructAll             = bReconstructAll;
  //***** NOTE: Motion-compensated prediction for non-key pictures is done in xReconstructLastFGS()
  bReconstructAll = bReconstructAll && bUseBaseRepresentation || ! bConstrainedIP;
 
  //tttest
// if (m_bIsNewPic)                           // may be modified later TMM_EC
  if(isNewPictureStart(rpcSliceHeader))
  {
    m_iMbProcessed =0;
	  m_bIsNewPic = false;	
  }

  switch ( m_eErrorConcealTemp)
  {
  case	EC_BLSKIP:
    {
      //	pcMbDataCtrlRef	=	NULL;
    }
    break;
    case	EC_FRAME_COPY:
    {
      if ( !m_bEnhanceAvailable)
      {
        IntFrame *IntFList_0= m_pcCurrDPBUnit->getCtrlData().getPrdFrameList( LIST_0 )[1];
        pcFrame->copy(IntFList_0, ePicType);	
        if ( bUseBaseRepresentation)
        {
	        RNOK( m_pcDecodedPictureBuffer->store( m_pcCurrDPBUnit, rcOutputList, rcUnusedList, pcFrame ) );
        }
        else
        {
	        RNOK( m_pcDecodedPictureBuffer->store( m_pcCurrDPBUnit, rcOutputList, rcUnusedList) );
        }
        //----- set slice header to zero (slice header is stored in control data) -----
        m_iMbProcessed += rpcSliceHeader->getSPS().getMbInFrame();
 	      rpcSliceHeader = 0; //to confirm 
        return Err::m_nOK;
      }
    }
    break;
  case	EC_TEMPORAL_DIRECT:
    {
	    rpcSliceHeader->setDirectSpatialMvPredFlag( false);
	    IntFrame * IntFList_0 = rpcSliceHeader->getSliceType() == B_SLICE ? m_pcCurrDPBUnit->getCtrlData().getPrdFrameList( LIST_1 )[1] : m_pcCurrDPBUnit->getCtrlData().getPrdFrameList( LIST_0 )[1];
	    DPBUnit	*pcDPBUnit	=	IntFList_0->getDPBUnit();
	    pcMbDataCtrlRef	=	pcDPBUnit->getCtrlData().getMbDataCtrl();
     }
    break;
    case	EC_NONE:
    case  EC_INTRA_COPY:
    break;
	}

  
  if(m_eErrorConcealTemp!=EC_FRAME_COPY   || (m_bEnhanceAvailable))
  {
    //----- parsing -----
    RNOK( m_pcControlMng  ->initSliceForReading ( *rpcSliceHeader ) );
    RNOK( m_pcSliceReader ->readVirtual      ( *rpcSliceHeader,
                                                pcMbDataCtrl,
                                                pcMbDataCtrlRef,
                                                rcControlData.getBaseLayerCtrl(),
                                                rcControlData.getBaseLayerCtrlField(),
                                                rcControlData.getSpatialScalabilityType(),
                                                m_uiFrameWidthInMb,
                                                uiMbRead,
                                                m_eErrorConcealTemp ) );
  }

  if(m_eErrorConcealTemp!=EC_FRAME_COPY || (m_bEnhanceAvailable))
  {
    RNOK( xInitBaseLayer( m_pcCurrDPBUnit->getCtrlData(), pcSliceHeaderBase ) );
  }
 
  //----- decoding -----
  
  if(m_eErrorConcealTemp!=EC_FRAME_COPY || m_bEnhanceAvailable)
  {
  
    RNOK( m_pcControlMng  ->initSliceForDecoding( *rpcSliceHeader ) );
   
    RNOK( m_pcSliceDecoder->decode              ( *rpcSliceHeader,
                                                  pcMbDataCtrl,
                                                  rcControlData.getBaseLayerCtrl(),
                                                  bUseBaseRepresentation ? pcBaseRepFrame : pcFrame,
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
  }
 

 //if (m_pcH264AVCDecoder->IsSliceEndOfPic())   // may be modified later
  if((m_eErrorConcealTemp!=EC_FRAME_COPY  || m_bEnhanceAvailable ) && isPictureDecComplete(rpcSliceHeader)  )
  {
	  m_bIsNewPic = true;
	  //----- store in decoded picture buffer -----
	  if( bUseBaseRepresentation )
	  {
    
	  //----- copy non-filtered frame -----
	  RNOK( pcFrame->copy( pcBaseRepFrame, ePicType ) );
   
	  //----- loop-filtering and store in DPB as base representation -----
	  m_pcLoopFilter->setHighpassFramePointer( pcResidual );
	  rcControlData.getSliceHeader()->FMOInit(); //TMM_EC chen ying
		  RNOK( m_pcLoopFilter->process( *rpcSliceHeader,
								     pcBaseRepFrame,
								     ( rpcSliceHeader->isIntra() ? NULL : pcMbDataCtrl ),
								     pcMbDataCtrl,
								     m_uiFrameWidthInMb,
								     &rcControlData.getPrdFrameList( LIST_0 ),
								     &rcControlData.getPrdFrameList( LIST_1 ),
								     false,
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
	  rcControlData.getSliceHeader()->FMOInit();
	  RNOK( m_pcRQFGSDecoder->initPicture( rcControlData.getSliceHeader(), rcControlData.getMbDataCtrl() ) );

	  RNOK( xAddBaseLayerResidual( rcControlData, m_pcRQFGSDecoder->getBaseLayerSbb() ) );
  }	// end DecComplete

  DTRACE_NEWFRAME;
  return Err::m_nOK;
}
//TMM_EC}}

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

    ROFS   ( ( pcDiffFrame                  = new IntFrame( *pcYuvFullPelBufferCtrl,
                                                           *pcYuvFullPelBufferCtrl ) ) );
    pcDiffFrame->init();

    iPoc = rcBaseList.getEntry(i)->getPoc();
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
    delete diffPrdRefList.getEntry(i); //TMM_FIX
  }

  return Err::m_nOK;
}
//TMM_EC {{
ErrVal
MCTFDecoder::getECMethod( SliceHeader *rpcSliceHeader, ERROR_CONCEAL &eErrorConceal)
{

  eErrorConceal=EC_NONE;
  if(rpcSliceHeader->getLayerId()==0&&!rpcSliceHeader->getTrueSlice())
  {
    if(m_eErrorConceal==EC_BLSKIP)
    {
      m_eErrorConceal =EC_TEMPORAL_DIRECT;
      eErrorConceal   =EC_TEMPORAL_DIRECT;
    }
    //set BLSKIP to TD in any case that base layer is lost
    if(m_eErrorConceal==EC_TEMPORAL_DIRECT&&rpcSliceHeader->getUseBasePredictionFlag())
        eErrorConceal=EC_FRAME_COPY;

    if(eErrorConceal==EC_NONE)eErrorConceal=m_eErrorConceal;
    rpcSliceHeader->setTrueSlice( m_eErrorConceal	==	EC_NONE);
    rpcSliceHeader->setBaseLayerId(MSYS_UINT_MAX);
    return Err::m_nOK;
  }
  
// when base layer status will not influence the enhancement layer true packet
  if(  rpcSliceHeader->getTrueSlice()&& 
      (rpcSliceHeader->getBaseLayerId()==MSYS_UINT_MAX ||!m_bBaseLayerLost))
  {
	  eErrorConceal	=	EC_NONE;
    return Err::m_nOK;
  }
  else
  {
    eErrorConceal=m_eErrorConceal;
  }

// when base layer and enhancement layer have different frame rate and the current frame is highest temporal level
  if ((  eErrorConceal == EC_BLSKIP ) && 
         rpcSliceHeader->getPoc() % ( 1<<(m_uiDecompositionStages-m_uiDecompositionStagesBase)) != 0)
		eErrorConceal = EC_FRAME_COPY;

	//else if ( rpcSliceHeader->getKeyPictureFlag() && m_bBaseLayerLost)
  else if ( rpcSliceHeader->getUseBasePredictionFlag() && (m_bBaseLayerLost||m_eErrorConceal==EC_TEMPORAL_DIRECT))

		eErrorConceal = EC_FRAME_COPY;

// other cases, current ec method is the same as the global ec method
  if(eErrorConceal==EC_NONE)eErrorConceal=m_eErrorConceal;

  if(eErrorConceal== EC_BLSKIP )
    rpcSliceHeader->setBaseLayerId(0);

  rpcSliceHeader->setTrueSlice( eErrorConceal	==	EC_NONE);
	return	Err::m_nOK;
}
//TMM_EC }}


ErrVal
MCTFDecoder::xGetBaseLayerData( ControlData&    rcControlData,
                                IntFrame*&      rpcBaseFrame,
                                IntFrame*&      rpcBaseResidual,
                                MbDataCtrl*&    rpcBaseDataCtrl,
                                MbDataCtrl*&    rpcMbDataCtrlEL,
                                Bool&           rbBaseDataAvailable,
                                Bool&           rbConstrainedIPredBL,
                                Bool&           rbSpatialScalability,
                                ResizeParameters* pcResizeParameter)
 
{
   SliceHeader* pcSliceHeader      = rcControlData.getSliceHeader();
  
   RNOK( m_pcH264AVCDecoder->getBaseLayerDataAvailability( rpcBaseFrame, 
																							             	rpcBaseResidual, 
																							             	rpcBaseDataCtrl, 
            																							  rbBaseDataAvailable,
                                                            rbSpatialScalability,
                                                            m_uiLayerId,
                                                            pcSliceHeader->getBaseLayerId(),
                                                            pcSliceHeader->getPoc(),
                                                            pcSliceHeader->getBaseQualityLevel() ) );    

    ROF( rbBaseDataAvailable );

    pcResizeParameter->m_iResampleMode = 0; 
    pcResizeParameter->m_bBaseFrameFromBotFieldFlag	= pcSliceHeader->m_bBaseFrameFromBotFieldFlag;  
    pcResizeParameter->m_bBaseBotFieldSyncFlag			= pcSliceHeader->m_bBaseBotFieldSyncFlag;
    pcResizeParameter->m_bBaseFrameMbsOnlyFlag			= rpcBaseDataCtrl->getSliceHeader()->getSPS().getFrameMbsOnlyFlag();
    pcResizeParameter->m_bBaseFieldPicFlag					= rpcBaseDataCtrl->getSliceHeader()->getFieldPicFlag();
    pcResizeParameter->m_bBaseIsMbAff               = rpcBaseDataCtrl->getSliceHeader()->isMbAff();   
    pcResizeParameter->m_bBaseBotFieldFlag					= rpcBaseDataCtrl->getSliceHeader()->getBottomFieldFlag();
    pcResizeParameter->SetUpSampleMode();

    pcSliceHeader->m_bBaseFrameMbsOnlyFlag            = pcResizeParameter->m_bBaseFrameMbsOnlyFlag;
    pcSliceHeader->m_bBaseFieldPicFlag                = pcResizeParameter->m_bBaseFieldPicFlag;
    pcSliceHeader->m_bBaseBotFieldFlag                = pcResizeParameter->m_bBaseBotFieldFlag;
    rcControlData.setSpatialScalability     ( rbSpatialScalability );
    rcControlData.setSpatialScalabilityType ( pcResizeParameter->m_iSpatialScalabilityType );
    rbSpatialScalability = rbSpatialScalability? rbSpatialScalability : pcResizeParameter->m_iResampleMode>0;

		RNOK( m_pcH264AVCDecoder->getBaseLayerData( rpcBaseFrame, 
     	                                          rpcBaseResidual, 
     	                                          rpcBaseDataCtrl,
     	                                          rpcMbDataCtrlEL, 
     	                                          rbConstrainedIPredBL, 
     	                                          rbSpatialScalability,
				                                        m_uiLayerId,
      																					pcSliceHeader->getBaseLayerId(),
      																					pcSliceHeader->getPoc(),
      																				  pcSliceHeader->getBaseQualityLevel()));
 
 return Err::m_nOK;
}


//JVT-U106 Behaviour at slice boundaries{
ErrVal
MCTFDecoder::xConstrainedIntraUpsampling( IntFrame*             pcFrame,
										                      IntFrame*             pcUpsampling, 
										                      IntFrame*             pcTemp,
										                      MbDataCtrl*           pcBaseDataCtrl,
										                      ReconstructionBypass* pcReconstructionBypass,
										                      ResizeParameters*     pcResizeParameters,
                                          PicType ePicType ) // TMM_INTERLACE

{
	int input_width   = pcResizeParameters->m_iInWidth;
	int output_width  = pcResizeParameters->m_iGlobWidth;  
	int output_height = pcResizeParameters->m_iGlobHeight;

	if(pcResizeParameters->m_iSpatialScalabilityType)
	{
		UInt uiMbInRow=input_width>>4;
		Int** ppiMaskL,**ppiMaskC;
		Int* piXL,*piXC,*piYL,*piYC;
		Int  k,l;
		UInt  uiSliceNbr=1;

		ppiMaskL=new Int*[output_height];
		for(k=0;k<output_height;k++)
		{
			ppiMaskL[k]=new Int[output_width];
		}
		ppiMaskC=new Int*[output_height/2];
		for(k=0;k<output_height/2;k++)
		{
			ppiMaskC[k]=new Int[output_width/2];
		}

		piXL=new Int[output_width];
		piXC=new Int[output_width/2];
		piYL=new Int[output_height];
		piYC=new Int[output_height/2];
		xGetPosition(pcResizeParameters,piXL,piYL,false);
		xGetPosition(pcResizeParameters,piXC,piYC,true);

		m_apcFrameTemp[1]->setZero();
		pcTemp->copy(pcFrame, ePicType);

		//Assume slice id is ordered from 1 2 3...
		for(UInt i=0;i<pcBaseDataCtrl->getSize();i++)
		{
			UInt          uiMbY             = i / uiMbInRow;
			UInt          uiMbX             = i % uiMbInRow;
			if(pcBaseDataCtrl->getMbData(uiMbX,uiMbY).getSliceId()>uiSliceNbr)
				uiSliceNbr=pcBaseDataCtrl->getMbData(uiMbX,uiMbY).getSliceId();
		}

		 //TMM_INTERLACE  
    Bool bBaseIsMbAff= pcResizeParameters->m_bBaseIsMbAff;

    for(UInt iSliceID=1;iSliceID<=uiSliceNbr;iSliceID++)
		{
			pcFrame->copyPortion(pcTemp);
			for(UInt uiMbAddress= 0 ;uiMbAddress<pcBaseDataCtrl->getSize();uiMbAddress++)
				//===== loop over macroblocks use raster scan =====
			{
				UInt          uiMbY             = uiMbAddress / uiMbInRow;
				UInt          uiMbX             = uiMbAddress % uiMbInRow;
		    pcFrame->getFullPelYuvBuffer()->getYuvBufferCtrl().initMb(uiMbY, uiMbX,bBaseIsMbAff);// TMM_INTERLACE
		    UInt uiMask = 0;
				RNOK( pcBaseDataCtrl->getBoundaryMaskCIU( uiMbY, uiMbX, uiMask, iSliceID ) );
				if( uiMask )
				{
					IntYuvMbBufferExtension cBuffer;
					cBuffer.setAllSamplesToZero();

					cBuffer.loadSurrounding( pcFrame->getFullPelYuvBuffer() );

					RNOK( pcReconstructionBypass->padRecMb( &cBuffer, uiMask ) );
					pcFrame->getFullPelYuvBuffer()->loadBuffer( &cBuffer );
				}
				else if(!(pcBaseDataCtrl->getMbData(uiMbX,uiMbY).isIntra()&&(pcBaseDataCtrl->getMbData(uiMbX,uiMbY).getSliceId()==iSliceID)))
				{
					IntYuvMbBufferExtension cBuffer;
					cBuffer.setAllSamplesToZero();
					pcFrame->getFullPelYuvBuffer()->loadBuffer( &cBuffer );
				}
			}

      RNOK( pcUpsampling->copy( pcFrame, ePicType ) );// TMM_INTERLACE
			pcUpsampling->upsample(m_cDownConvert, pcResizeParameters, true);
			for(k=0;k<output_height;k++)
			{
				for(l=0;l<output_width;l++)
				{
					if(piXL[l]==-128||piYL[k]==-128)
						ppiMaskL[k][l]=0;
					else
					{
						Int  iX=piXL[l];
						Int  iY=piYL[k];
						if(pcBaseDataCtrl->getMbData(iX>>4,iY>>4).isIntra()&&(pcBaseDataCtrl->getMbData(iX>>4,iY>>4).getSliceId()==iSliceID))
							ppiMaskL[k][l]=1;
						else
							ppiMaskL[k][l]=0;
					}
				}          
			}

			for(k=0;k<output_height/2;k++)
			{
				for(l=0;l<output_width/2;l++)
				{
					if(piXC[l]==-128||piYC[k]==-128)
						ppiMaskL[k][l]=0;
					else
					{
						Int  iX=piXC[l];
						Int  iY=piYC[k];
						if(pcBaseDataCtrl->getMbData(iX>>3,iY>>3).isIntra()&&(pcBaseDataCtrl->getMbData(iX>>3,iY>>3).getSliceId()==iSliceID))
							ppiMaskC[k][l]=1;
						else
							ppiMaskC[k][l]=0;
					}
				}          
			}
			m_apcFrameTemp[1]->copyMask(pcUpsampling,ppiMaskL,ppiMaskC);
		}

		pcUpsampling->copy(m_apcFrameTemp[1], ePicType);// TMM_INTERLACE
		delete[]piXL;
		delete[]piYL;
		delete[]piXC;
		delete[]piYC;
		for(k=0;k<output_height;k++)
		{
			delete[]ppiMaskL[k];
		}
		delete[] ppiMaskL;
		for(k=0;k<output_height/2;k++)
		{
			delete[]ppiMaskC[k];
		}
		delete[] ppiMaskC;

	}
  else // fix by H. Schwarz
  {
    pcUpsampling->copy( pcFrame, ePicType );
  }
	return Err::m_nOK;
}

void MCTFDecoder::xGetPosition(ResizeParameters* pcResizeParameters,Int*px,Int*py,bool uv_flag)

{
	Int iratio=uv_flag?2:1;
	int input_width   = pcResizeParameters->m_iInWidth/iratio;
	int input_height  = pcResizeParameters->m_iInHeight/iratio;
	int output_width  = pcResizeParameters->m_iGlobWidth/iratio;  
	int output_height = pcResizeParameters->m_iGlobHeight/iratio;
	int crop_x0 = pcResizeParameters->m_iPosX/iratio;
	int crop_y0 = pcResizeParameters->m_iPosY/iratio;
	int crop_w = pcResizeParameters->m_iOutWidth/iratio;
	int crop_h = pcResizeParameters->m_iOutHeight/iratio;  
	int input_chroma_phase_shift_x = pcResizeParameters->m_iBaseChromaPhaseX;
	int input_chroma_phase_shift_y = pcResizeParameters->m_iBaseChromaPhaseY;
	int output_chroma_phase_shift_x = pcResizeParameters->m_iChromaPhaseX;
	int output_chroma_phase_shift_y = pcResizeParameters->m_iChromaPhaseY;

	int i, j;
	bool ratio1_flag = ( input_width == crop_w );
	unsigned short deltaa, deltab;

	for(i=0; i<crop_x0; i++)  px[i] = -128;

	for(i=crop_x0+crop_w; i<output_width; i++)  px[i] = -128;

	if(ratio1_flag)
	{
		for(i = 0; i < crop_w; i++)
		{
			px[i+crop_x0] = i*16+4*(2+output_chroma_phase_shift_x)-4*(2+input_chroma_phase_shift_x);
		}
	}
	else
	{
		deltaa = ((input_width<<16) + (crop_w>>1))/crop_w;
		if(uv_flag)
		{
			deltab = ((input_width<<14) + (crop_w>>1))/crop_w;
			for(i = 0; i < crop_w; i++)
			{
				px[i+crop_x0] = ((i*deltaa + (2 + output_chroma_phase_shift_x)*deltab + 2048)>>12) - 4*(2 + input_chroma_phase_shift_x);
				px[i+crop_x0] =(Int)(px[i+crop_x0]/16.0+0.5);
			}
		}
		else
		{
			deltab = ((input_width<<15) + (crop_w>>1))/crop_w;
			for(i = 0; i < crop_w; i++)
			{
				px[i+crop_x0] = (i*deltaa + deltab - 30720)>>12;
				px[i+crop_x0] =(Int)(px[i+crop_x0]/16.0+0.5);
			}
		}
	}

	ratio1_flag = ( input_height == crop_h );

	for(j=0; j<crop_y0; j++)   py[j] = -128;

	for(j=crop_y0+crop_h; j<output_height; j++)  py[j] = -128;

	if(ratio1_flag)
	{
		for(j = 0; j < crop_h; j++)
		{
			py[j+crop_y0] = j*16+4*(2+output_chroma_phase_shift_y)-4*(2+input_chroma_phase_shift_y);
		}
	}
	else
	{
		deltaa = ((input_height<<16) + (crop_h>>1))/crop_h;
		if(uv_flag)
		{
			deltab = ((input_height<<14) + (crop_h>>1))/crop_h;
			for(j = 0; j < crop_h; j++)
			{
				py[j+crop_y0] = ((j*deltaa + (2 + output_chroma_phase_shift_y)*deltab + 2048)>>12) - 4*(2 + input_chroma_phase_shift_y);
				py[j+crop_y0] =(Int)(py[j+crop_y0]/16.0+0.5);
			}
		}
		else
		{
			deltab = ((input_height<<15) + (crop_h>>1))/crop_h;
			for(j = 0; j < crop_h; j++)
			{
				py[j+crop_y0] = (j*deltaa + deltab - 30720)>>12;
				py[j+crop_y0] =(Int)(py[j+crop_y0]/16.0+0.5);
			}
		}
	}
}
//JVT-U106 Behaviour at slice boundaries}

H264AVC_NAMESPACE_END

