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
#include "CreaterH264AVCDecoder.h"
#include "ControlMngH264AVCDecoder.h"
#include "H264AVCCommonLib/TraceFile.h"
#include <math.h>
#include "H264AVCCommonLib/CFMO.h"
#include "H264AVCCommonLib/ReconstructionBypass.h"


H264AVC_NAMESPACE_BEGIN



//////////////////////////////////////////////////////////////////////////
// DPB UNIT
//////////////////////////////////////////////////////////////////////////

DPBUnit::DPBUnit()
: m_uiFrameNum            ( MSYS_UINT_MAX )
, m_uiTemporalId          ( MSYS_UINT_MAX )
, m_bUseBasePred          ( false )
, m_bExisting             ( false )
, m_bNeededForReference   ( false )
, m_bOutputted            ( false )
, m_bBaseRepresentation   ( false )
, m_pcFrame               ( NULL )
, m_cControlData          ()
, m_pcMbDataCtrlBL        ( NULL )
, m_bConstrainedIntraPred ( false )
{
}

DPBUnit::~DPBUnit()
{
  MbDataCtrl*   pcMbDataCtrl  = m_cControlData.getMbDataCtrl  ();
  if( pcMbDataCtrl )
  {
    pcMbDataCtrl->uninit();
	  delete pcMbDataCtrl;
		pcMbDataCtrl = NULL;
  }
  if( m_pcMbDataCtrlBL )
  {
    m_pcMbDataCtrlBL->uninit();
    delete m_pcMbDataCtrlBL;
    m_pcMbDataCtrlBL = NULL;
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
                 const SequenceParameterSet&  rcSPS,
                 UInt                         uiDependencyId )
{
  rpcDPBUnit = new DPBUnit();
  ROF( rpcDPBUnit );

  MbDataCtrl* pcMbDataCtrl = 0;
  ROFS( ( rpcDPBUnit->m_pcFrame    = new Frame      ( rcYuvBufferCtrl, rcYuvBufferCtrl ) ) );
  ROFS( ( pcMbDataCtrl             = new MbDataCtrl ()                                   ) );
  RNOK(   rpcDPBUnit->m_pcFrame    ->init           ()               );
          rpcDPBUnit->m_pcFrame    ->setDPBUnit     ( rpcDPBUnit   );
  RNOK(   pcMbDataCtrl             ->init           ( rcSPS        ) );
  RNOK(   rpcDPBUnit->m_cControlData.setMbDataCtrl  ( pcMbDataCtrl ) );

  if( uiDependencyId == 0 )
  {
    ROFS( ( rpcDPBUnit->m_pcMbDataCtrlBL = new MbDataCtrl() ) );
    RNOK(   rpcDPBUnit->m_pcMbDataCtrlBL->init( rcSPS ) );
  }

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
  m_uiTemporalId          = rcSH.getTemporalId();
  m_bUseBasePred          = rcSH.getUseRefBasePicFlag();
  m_bExisting             = true;
  m_bNeededForReference   = rcSH.getNalRefIdc() > 0;
  m_bOutputted            = false;
  m_bBaseRepresentation   = false;
  m_bConstrainedIntraPred = rcSH.getPPS().getConstrainedIntraPredFlag();
  m_uiQualityId           = rcSH.getQualityId();
  return Err::m_nOK;
}


ErrVal
DPBUnit::initNonEx( Int   iPoc,
                    UInt  uiFrameNum )
{
  m_pcFrame->setPoc( iPoc );
  m_uiFrameNum            = uiFrameNum;
  m_uiTemporalId          = MSYS_UINT_MAX;
  m_bUseBasePred          = false;
  m_bExisting             = false;
  m_bNeededForReference   = true;
  m_bOutputted            = false;
  m_bBaseRepresentation   = false;
  m_bConstrainedIntraPred = false;
  return Err::m_nOK;
}



ErrVal
DPBUnit::initBase( DPBUnit& rcDPBUnit,
                   Frame*   pcFrameBaseRep )
{
  ROT( rcDPBUnit.m_bBaseRepresentation );
  m_uiFrameNum            = rcDPBUnit.m_uiFrameNum;
  m_uiTemporalId          = rcDPBUnit.m_uiTemporalId;
  m_bUseBasePred          = rcDPBUnit.m_bUseBasePred;
  m_bExisting             = rcDPBUnit.m_bExisting;
  m_bNeededForReference   = rcDPBUnit.m_bNeededForReference;
  m_bOutputted            = rcDPBUnit.m_bOutputted;
  m_bConstrainedIntraPred = rcDPBUnit.m_bConstrainedIntraPred;
  m_bBaseRepresentation   = true;
  m_uiQualityId           = rcDPBUnit.m_uiQualityId;

	const SliceHeader&  rcSH      = *rcDPBUnit.getCtrlData().getSliceHeader();
  const PicType       ePicType  = rcSH.getPicType();
	RNOK( m_pcFrame->addFrameFieldBuffer() );
	m_pcFrame->setPoc( rcSH );
  RNOK( m_pcFrame->copy( pcFrameBaseRep, ePicType ) );

  return Err::m_nOK;
}


ErrVal
DPBUnit::uninit()
{
  m_uiFrameNum            = MSYS_UINT_MAX;
  m_uiTemporalId          = MSYS_UINT_MAX;
  m_bUseBasePred          = false;
  m_bExisting             = false;
  m_bNeededForReference   = false;
  m_bOutputted            = false;
  m_bBaseRepresentation   = false;
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
DecodedPicBuffer::initSPS( const SequenceParameterSet& rcSPS, UInt uiDependencyId )
{
  ROF( m_bInitDone );
	UInt  uiMaxPicsInDPB  = downround2powerof2( rcSPS.getMaxDPBSize() ) + 5; // up to 3 base representations + 2 extra (should use "real DPB" size in future)
  RNOK( xCreateData( uiMaxPicsInDPB, rcSPS, uiDependencyId ) );
  m_uiNumRefFrames      = rcSPS.getNumRefFrames();
  m_uiMaxFrameNum       = ( 1 << ( rcSPS.getLog2MaxFrameNum() ) );
  return Err::m_nOK;
}


ErrVal
DecodedPicBuffer::uninit()
{
  ROF ( m_bInitDone );
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
                               const SequenceParameterSet&  rcSPS,
                               UInt                         uiDependencyId )
{
  ROF ( m_bInitDone );
  RNOK( xDeleteData() );

  while( uiMaxPicsInDPB-- )
  {
    DPBUnit* pcDPBUnit = NULL;
    RNOK( DPBUnit::create( pcDPBUnit, *m_pcYuvBufferCtrl, rcSPS, uiDependencyId ) );
    m_cFreeDPBUnitList.pushBack( pcDPBUnit );
  }
  RNOK( DPBUnit::create( m_pcCurrDPBUnit, *m_pcYuvBufferCtrl, rcSPS, uiDependencyId ) );
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

  if( pcSliceHeader && pcSliceHeader->getDecRefPicMarking().getAdaptiveRefPicMarkingModeFlag() )
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

  Mmco              eMmcoOp;
  const DecRefPicMarking& rcMmcoBuffer  = pcSliceHeader->getDecRefPicMarking();
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


ErrVal
DecodedPicBuffer::MMCOBase( SliceHeader* pcSliceHeader, UInt mCurrFrameNum ) 
{
	ROF( pcSliceHeader );

  Mmco                    eMmcoOp;
  const DecRefPicMarking& rcMmcoBaseBuffer = pcSliceHeader->getDecRefBasePicMarking();
  Int                     iIndex        = 0;
  UInt                    uiVal1, uiVal2;

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
  UInt    uiCurrPicNum = ( eCurrentPicType==FRAME ? mCurrFrameNum
		                                           : mCurrFrameNum*2+1 );
  UInt    uiPicNumN    = uiCurrPicNum - uiDiffOfPicNums - 1;
	PicType ePicType;
	xSetIdentifier( uiPicNumN, ePicType, eCurrentPicType );

  DPBUnitList::iterator iter      = m_cUsedDPBUnitList.begin();
  DPBUnitList::iterator end       = m_cUsedDPBUnitList.end  ();
  for( ; iter != end; iter++ )
  {
	  if( (*iter)->isNeededForRef() && (*iter)->getFrameNum() == (Int)uiPicNumN && (*iter)->isBaseRep() ) 
    {
		  DPBUnit* pcDPBUnit = (*iter);
   	  RNOK( pcDPBUnit->uninit() );
		  m_cUsedDPBUnitList.remove(pcDPBUnit);
		  m_cFreeDPBUnitList.push_back( pcDPBUnit );
		  return Err::m_nOK;  
    }
  }
  return Err::m_nOK;
}


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


ErrVal
DecodedPicBuffer::slidingWindowBase( UInt mCurrFrameNum ) 
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
					  DPBUnit* pcDPBUnit = (*iter);
					  RNOK( pcDPBUnit->uninit() );
					  m_cUsedDPBUnitList.remove(pcDPBUnit);
					  m_cFreeDPBUnitList.push_back( pcDPBUnit );
					  return Err::m_nOK;  
				  }
			  }
		  }
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
        if( pcPicBuffer->isUsed() )
        {
          pcPicBuffer->setUnused(); 
        }
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
        if(pcPicBuffer->isUsed())
        {
          pcPicBuffer->setUnused();
        }
        pcDPBUnit->getCtrlData().setSliceHeader(NULL);
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


ErrVal DecodedPicBuffer::xUpdateDPBUnitList(DPBUnit *pcDPBUnit)
{
  ROF( pcDPBUnit == m_pcCurrDPBUnit ); // check
  DPBUnit*  pcElemToReplace  = 0;
  DPBUnitList::iterator iter =  m_cUsedDPBUnitList.begin();
  DPBUnitList::iterator end  =  m_cUsedDPBUnitList.end  ();
  for( ; iter != end; iter++ )
  {
    if( !(*iter)->isBaseRep() && (*iter)->getFrame()->getPoc() == pcDPBUnit->getFrame()->getPoc() && (*iter)->getQualityId()+1 == pcDPBUnit->getQualityId() )
    {
      pcElemToReplace  = (*iter);
      m_cUsedDPBUnitList.remove(pcElemToReplace);
      m_cFreeDPBUnitList.push_back(pcElemToReplace);
      m_cUsedDPBUnitList.push_back(pcDPBUnit);   
      RNOK( pcDPBUnit->switchMbDataCtrlBL( pcElemToReplace ) );
      break;
    }
  }
  return Err::m_nOK;
}


ErrVal
DecodedPicBuffer::xStorePicture( DPBUnit*       pcDPBUnit,
                                 PicBufferList& rcOutputList,
                                 PicBufferList& rcUnusedList,
                                 Bool           bTreatAsIdr,
                                 Bool           bFrameMbsOnlyFlag,
                                 Bool           bRef )
                                 
{
  ROF( pcDPBUnit == m_pcCurrDPBUnit );

  if( pcDPBUnit->isExisting() && pcDPBUnit->getCtrlData().getSliceHeader()->isH264AVCCompatible() )
  {
    MbDataCtrl* pcMbDataCtrl = pcDPBUnit->getCtrlData().getMbDataCtrl();
    ROF( pcMbDataCtrl );
    RNOK( pcDPBUnit->storeMbDataCtrlBL( pcMbDataCtrl ) );
  }

  //---- fill border ----
  RNOK( m_pcYuvBufferCtrl->initMb() );
  RNOK( pcDPBUnit->getFrame()->extendFrame( NULL, pcDPBUnit->getFrame()->getPicType(), bFrameMbsOnlyFlag ) );

  if( !bRef )
  {
    if( bTreatAsIdr )
    {
      //===== IDR pictures =====
      RNOK( xClearOutputAll( rcOutputList, rcUnusedList, false ) );       // clear and output all pictures
      m_cUsedDPBUnitList.push_back( pcDPBUnit );                          // store current picture
    }
    else
    {
      //===== non-IDR picture =====
      m_cUsedDPBUnitList.push_back( pcDPBUnit );                          // store current picture
      RNOK( xUpdateMemory( pcDPBUnit->getCtrlData().getSliceHeader() ) ); // memory update 
      RNOK( xOutput( rcOutputList, rcUnusedList ) );                      // output
    }
    RNOK( xDumpDPB() );

    m_pcCurrDPBUnit = m_cFreeDPBUnitList.popFront();                      // new current DPB unit
  }
  else
  {
    //replace previous version of current frame in usedDPBUnit by the new version
    RNOK( xUpdateDPBUnitList(pcDPBUnit));
    RNOK( xUpdateMemory( pcDPBUnit->getCtrlData().getSliceHeader() ) );   // memory update 
    RNOK( xOutput( rcOutputList, rcUnusedList ) );                        // output
    m_pcCurrDPBUnit = m_cFreeDPBUnitList.popBack();      
  }
  m_pcCurrDPBUnit->getCtrlData().setSliceHeader( 0 );
  m_pcCurrDPBUnit->getCtrlData().getMbDataCtrl()->reset();
  m_pcCurrDPBUnit->getCtrlData().getMbDataCtrl()->clear();

  return Err::m_nOK;
}



ErrVal
DecodedPicBuffer::xCheckMissingPics( const SliceHeader*   pcSliceHeader,
                                     PicBufferList& rcOutputList,
                                     PicBufferList& rcUnusedList )
{
  ROTRS( pcSliceHeader->getIdrFlag(), Err::m_nOK );
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
      RNOK( xStorePicture( m_pcCurrDPBUnit, rcOutputList, rcUnusedList, bTreatAsIdr, pcSliceHeader->getSPS().getFrameMbsOnlyFlag(), false ) ); 
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
                                      Frame*     pcCurrentFrame,
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


Void 
DecodedPicBuffer::xSetIdentifier( UInt& uiNum, PicType& rePicType, const PicType eCurrentPicType )
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


ErrVal 
DecodedPicBuffer::xSetInitialRefFieldList( RefFrameList& rcList, Frame* pcCurrentFrame, PicType eCurrentPicType, SliceType eSliceType ) 
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
      Frame* pcFrame = cTempList.getEntry( uiCurrentParityIndex++ );

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
      Frame* pcFrame = cTempList.getEntry( uiOppositeParityIndex++ );
			RNOK( rcList.add( pcFrame->getPic( eOppositePicType ) ) );
    }
  }

  return Err::m_nOK;
}

ErrVal
DecodedPicBuffer::xInitPrdListsBSlice( RefFrameList&  rcList0,
                                       RefFrameList&  rcList1,
                                       Frame*         pcCurrentFrame,
																			 PicType        eCurrentPicType,
																			 SliceType      eSliceType )
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
  const RefPicListReOrdering& rcRplrBuffer = pcSliceHeader->getRefPicListReordering( eListIdx );

  //===== re-odering =====
  if( rcRplrBuffer.getRefPicListReorderingFlag() )
  {
    const PicType eCurrentPicType = pcSliceHeader->getPicType();
    UInt    uiPicNumPred          = ( eCurrentPicType==FRAME ? pcSliceHeader->getFrameNum() : 
                                                               pcSliceHeader->getFrameNum()*2+1 );
    UInt    uiMaxPicNum           = ( eCurrentPicType==FRAME ? m_uiMaxFrameNum : 2*m_uiMaxFrameNum );

    Bool  bBaseRep          = pcSliceHeader->getUseRefBasePicFlag();
    UInt  uiIndex           = 0;
    ReOrderingOfPicNumsIdc  uiCommand;
    UInt  uiIdentifier      = 0;

    while( RPLR_END != ( uiCommand = rcRplrBuffer.get(uiIndex).getCommand(uiIdentifier) ) )
    {
      Frame* pcFrame = NULL;

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
                                      Bool           bRef)
{
  ROF( m_bInitDone );
  if( ! bRef )
  {
    //===== insert pic buffer in list =====
    m_cPicBufferList.push_back( rpcPicBuffer );
    rpcPicBuffer = 0;
  }
  m_pcCurrDPBUnit->getCtrlData().clear();
  m_pcCurrDPBUnit->getCtrlData().getMbDataCtrl()->reset();
  m_pcCurrDPBUnit->getCtrlData().getMbDataCtrl()->clear();

  return Err::m_nOK;
}


ErrVal
DecodedPicBuffer::initCurrDPBUnit( DPBUnit*&      rpcCurrDPBUnit,
                                   SliceHeader*   pcSliceHeader,
                                   PicBufferList& rcOutputList,
                                   PicBufferList& rcUnusedList )
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

  if( ! pcSliceHeader->getQualityId() )
  {
    //===== check missing pictures =====
	  RNOK( xCheckMissingPics( pcSliceHeader, rcOutputList, rcUnusedList ) );
  }

  //===== initialize current DPB unit =====
  RNOK( m_pcCurrDPBUnit->init( *pcSliceHeader ) );

  ROT( pcSliceHeader->getUseRefBasePicFlag() && !pcSliceHeader->getNalRefIdc() ); // just a check
  m_pcCurrDPBUnit->getCtrlData().setSliceHeader( pcSliceHeader );
  m_pcCurrDPBUnit->setPicType( pcSliceHeader->getPicType() );
  m_pcCurrDPBUnit->setNalRefIdc( pcSliceHeader->getNalRefIdc() );

  //===== set DPB unit =====
  rpcCurrDPBUnit = m_pcCurrDPBUnit;
  m_pcLastDPBUnit = m_pcCurrDPBUnit;
  return Err::m_nOK;
}



ErrVal
DecodedPicBuffer::clear( PicBufferList& rcOutputList,
                         PicBufferList& rcUnusedList )
{
  RNOK( xClearOutputAll( rcOutputList, rcUnusedList, true ) ); // clear and output all pictures
  ROF ( m_cPicBufferList.empty() );
  return Err::m_nOK;
}



DPBUnit*
DecodedPicBuffer::getLastUnit()
{
  ROTRS( m_cUsedDPBUnitList.empty(), 0 );
  return m_cUsedDPBUnitList.back();
}


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
  MbDataCtrl*   pcMbDataCtrl0L1 = 0;

	Frame* pcCurrentFrame = m_pcCurrDPBUnit->getFrame();
  
  if( pcSliceHeader->isIntraSlice() )
  {
  }
  else if( pcSliceHeader->isPSlice() )
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

    if( pcSliceHeader->isH264AVCCompatible() )
    {
      const Frame*       pcFrame0L1  = rcList1[1];
      DPBUnitList::iterator iter        = m_cUsedDPBUnitList.begin ();
      DPBUnitList::iterator end         = m_cUsedDPBUnitList.end   ();
      for( ; iter != end; iter++ )
      {
        DPBUnit*    pcDPBUnit     = (*iter);
        Frame*   pcFrame       = pcDPBUnit->getFrame();
        MbDataCtrl* pcMbDataCtrl  = pcDPBUnit->getMbDataCtrlBL();

        if( pcFrame == pcFrame0L1 )
        {
          pcMbDataCtrl0L1 = pcMbDataCtrl;
        }
      }
      ROF( pcMbDataCtrl0L1 );
    }
  }

  m_pcCurrDPBUnit->getCtrlData().setMbDataCtrl0L1( pcMbDataCtrl0L1 );

  return Err::m_nOK;
}

ErrVal
DecodedPicBuffer::store( DPBUnit*&        rpcDPBUnit,
                         PicBufferList&   rcOutputList,
                         PicBufferList&   rcUnusedList,
                         Frame*           pcFrameBaseRep )
{
  Bool bRef = ( rpcDPBUnit->getCtrlData().getSliceHeader()->getQualityId() > 0 );

	if( rpcDPBUnit->isNeededForRef() || rpcDPBUnit->getCtrlData().getSliceHeader()->getOutputFlag() )
	{
		RNOK( xStorePicture(  rpcDPBUnit, rcOutputList, rcUnusedList,
			                    rpcDPBUnit->getCtrlData().getSliceHeader()->getIdrFlag(),
			                    rpcDPBUnit->getCtrlData().getSliceHeader()->getSPS().getFrameMbsOnlyFlag(), bRef ) );
  	if( !rpcDPBUnit->getCtrlData().getSliceHeader()->getOutputFlag() )
		{
			rpcDPBUnit->markOutputted();
		}
	}
  
  if( rpcDPBUnit->isNeededForRef() )
  {
    m_uiLastRefFrameNum = rpcDPBUnit->getFrameNum();
  }

  ROFRS( pcFrameBaseRep, Err::m_nOK );
  ROTRS( bRef,           Err::m_nOK );

  // Do not store the base representation if not specified in the stream
  ROFRS( rpcDPBUnit->getCtrlData().getSliceHeader()->getStoreRefBasePicFlag(), Err::m_nOK );

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
  if( !bRef )
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





MbStatus::MbStatus()
: m_uiSliceIdc    ( MSYS_UINT_MAX )
, m_pcSliceHeader ( 0 )
{
}

MbStatus::~MbStatus()
{
}

Void
MbStatus::reset()
{
  m_uiSliceIdc    = MSYS_UINT_MAX;
  m_pcSliceHeader = 0;
}

Bool
MbStatus::canBeUpdated( const SliceHeader* pcSliceHeader )
{
  ROTRS( pcSliceHeader->getQualityId() == 0 && m_uiSliceIdc == MSYS_UINT_MAX,                 true );
  ROTRS( pcSliceHeader->getQualityId() != 0 && pcSliceHeader->getRefLayerDQId() == getDQId(), true );
  return false;
}

ErrVal
MbStatus::update( SliceHeader* pcSliceHeader )
{
  ROF( canBeUpdated( pcSliceHeader ) );
  m_uiSliceIdc    = pcSliceHeader->getFirstMbInSlice() << 7;
  m_uiSliceIdc   += pcSliceHeader->getDependencyId  () << 4;
  m_uiSliceIdc   += pcSliceHeader->getQualityId     ();
  m_pcSliceHeader = pcSliceHeader;
  return Err::m_nOK;
}





//////////////////////////////////////////////////////////////////////////
// MCTF DECODER
//////////////////////////////////////////////////////////////////////////

LayerDecoder::LayerDecoder()
: m_pcH264AVCDecoder                    ( 0 )
, m_pcNalUnitParser                     ( 0 )
, m_pcSliceReader                       ( 0 )
, m_pcSliceDecoder                      ( 0 )
, m_pcControlMng                        ( 0 )
, m_pcLoopFilter                        ( 0 )
, m_pcHeaderSymbolReadIf                ( 0 )
, m_pcParameterSetMng                   ( 0 )
, m_pcPocCalculator                     ( 0 )
, m_pcYuvFullPelBufferCtrl              ( 0 )
, m_pcDecodedPictureBuffer              ( 0 )
, m_pcMotionCompensation                ( 0 )
, m_pcReconstructionBypass              ( 0 )
#ifdef SHARP_AVC_REWRITE_OUTPUT
, m_pcRewriteEncoder                    ( 0 )
#endif
, m_bInitialized                        ( false )
, m_bSPSInitialized                     ( false )
, m_bDependencyRepresentationInitialized( false )
, m_bLayerRepresentationInitialized     ( false )
, m_uiFrameWidthInMb                    ( 0 )
, m_uiFrameHeightInMb                   ( 0 )
, m_uiMbNumber                          ( 0 )
, m_uiDependencyId                      ( 0 )
, m_uiQualityId                         ( 0 )
, m_pacMbStatus                         ( 0 )
, m_pcResizeParameter                   ( 0 )
, m_pcCurrDPBUnit                       ( 0 )
, m_pcBaseLayerCtrl                     ( 0 )
, m_pcBaseLayerCtrlField                ( 0 )
, m_pcResidual                          ( 0 )
, m_pcILPrediction                      ( 0 )
, m_pcBaseLayerFrame                    ( 0 )
, m_pcBaseLayerResidual                 ( 0 )
{
  ::memset( m_apcFrameTemp, 0x00, sizeof( m_apcFrameTemp ) );
}


LayerDecoder::~LayerDecoder()
{
  while( m_cSliceHeaderList.size() )
  {
    SliceHeader*  pcSliceHeader = m_cSliceHeaderList.popFront();
    delete        pcSliceHeader;
  }
}


ErrVal
LayerDecoder::create( LayerDecoder*& rpcLayerDecoder )
{
  rpcLayerDecoder = new LayerDecoder;
  ROT( NULL == rpcLayerDecoder );
  return Err::m_nOK;
}


ErrVal
LayerDecoder::destroy()
{
  ROT( m_bInitialized );
  delete this;
  return Err::m_nOK;
}


ErrVal
LayerDecoder::init( UInt                   uiDependencyId,
                   H264AVCDecoder*        pcH264AVCDecoder,
                   NalUnitParser*         pcNalUnitParser,
                   SliceReader*           pcSliceReader,
                   SliceDecoder*          pcSliceDecoder,
                   ControlMngIf*          pcControlMng,
                   LoopFilter*            pcLoopFilter,
                   HeaderSymbolReadIf*    pcHeaderSymbolReadIf,
                   ParameterSetMng*       pcParameterSetMng,
                   PocCalculator*         pcPocCalculator,
                   YuvBufferCtrl*         pcYuvFullPelBufferCtrl,
                   DecodedPicBuffer*      pcDecodedPictureBuffer,
                   MotionCompensation*    pcMotionCompensation,
				           ReconstructionBypass*  pcReconstructionBypass 
#ifdef SHARP_AVC_REWRITE_OUTPUT
                   ,RewriteEncoder*       pcRewriteEncoder
#endif
                   )
{
  ROT( m_bInitialized );
  ROF( pcH264AVCDecoder );
  ROF( pcNalUnitParser );
  ROF( pcSliceReader );
  ROF( pcSliceDecoder );
  ROF( pcControlMng );
  ROF( pcLoopFilter );
  ROF( pcHeaderSymbolReadIf );
  ROF( pcParameterSetMng );
  ROF( pcPocCalculator );
  ROF( pcYuvFullPelBufferCtrl );
  ROF( pcDecodedPictureBuffer );
  ROF( pcMotionCompensation );
  ROF( pcReconstructionBypass );
#ifdef SHARP_AVC_REWRITE_OUTPUT
  ROF( pcRewriteEncoder );
#endif

  m_pcH264AVCDecoder                      = pcH264AVCDecoder;
  m_pcNalUnitParser                       = pcNalUnitParser;
  m_pcSliceReader                         = pcSliceReader;
  m_pcSliceDecoder                        = pcSliceDecoder ;
  m_pcControlMng                          = pcControlMng;
  m_pcLoopFilter                          = pcLoopFilter;
  m_pcHeaderSymbolReadIf                  = pcHeaderSymbolReadIf;
  m_pcParameterSetMng                     = pcParameterSetMng;
  m_pcPocCalculator                       = pcPocCalculator;
  m_pcYuvFullPelBufferCtrl                = pcYuvFullPelBufferCtrl;
  m_pcDecodedPictureBuffer                = pcDecodedPictureBuffer;
  m_pcMotionCompensation                  = pcMotionCompensation;
  m_pcReconstructionBypass                = pcReconstructionBypass;
#ifdef SHARP_AVC_REWRITE_OUTPUT
  m_pcRewriteEncoder                      = pcRewriteEncoder;
#endif

  m_bInitialized                          = true;
  m_bSPSInitialized                       = false;
  m_bDependencyRepresentationInitialized  = false;
  m_bLayerRepresentationInitialized       = false;
  m_uiFrameWidthInMb                      = 0;
  m_uiFrameHeightInMb                     = 0;
  m_uiMbNumber                            = 0;
  m_uiDependencyId                        = uiDependencyId;
  m_uiQualityId                           = 0;

  m_pacMbStatus                           = 0;

  m_pcResizeParameter                     = 0;
  m_pcCurrDPBUnit                         = 0;
  m_pcBaseLayerCtrl                       = 0;
  m_pcBaseLayerCtrlField                  = 0;
  m_pcResidual                            = 0;
  m_pcILPrediction                        = 0;
  m_pcBaseLayerFrame                      = 0;
  m_pcBaseLayerResidual                   = 0;

  return Err::m_nOK;
}


ErrVal
LayerDecoder::uninit()
{
  ROF ( m_bInitialized );
  RNOK( xDeleteData() );
  m_bInitialized  = false;
  return Err::m_nOK;
}


ErrVal
LayerDecoder::processSliceData( PicBuffer*         pcPicBuffer,
                               PicBufferList&     rcPicBufferOutputList,
                               PicBufferList&     rcPicBufferUnusedList,
                               BinDataList&       rcBinDataList,
                               SliceDataNALUnit&  rcSliceDataNALUnit )
{
  ROF( m_bInitialized );
  ROF( pcPicBuffer );
  ROF( rcSliceDataNALUnit.isSliceHeaderPresent() );

  //===== init slice =====
  SliceHeader*  pcSliceHeader = 0;
  RNOK( xInitSlice  ( pcSliceHeader, pcPicBuffer, rcPicBufferOutputList, rcPicBufferUnusedList, rcSliceDataNALUnit ) );

  //===== parse slice =====
  RNOK( xParseSlice ( *pcSliceHeader ) );

#ifdef SHARP_AVC_REWRITE_OUTPUT
  //===== init rewriting =====
  if( rcSliceDataNALUnit.isHighestRewriteLayer() )
  {
    RNOK( m_pcRewriteEncoder->startSlice( rcBinDataList, *pcSliceHeader ) );
  }
#endif

  //===== decode slice =====
  RNOK( xDecodeSlice( *pcSliceHeader, rcPicBufferOutputList, rcPicBufferUnusedList, rcSliceDataNALUnit ) );

#ifdef SHARP_AVC_REWRITE_OUTPUT
  //===== finish rewriting =====
  if( m_pcRewriteEncoder->isSliceInProgress() )
  {
    RNOK( m_pcRewriteEncoder->finishSlice( rcBinDataList ) );
  }
#endif

  //===== finish slice =====
  RNOK( xFinishSlice( rcPicBufferOutputList, rcPicBufferUnusedList, rcSliceDataNALUnit ) );

  return Err::m_nOK;
}


ErrVal
LayerDecoder::finishProcess( PicBufferList&  rcPicBufferOutputList,
                           PicBufferList&  rcPicBufferUnusedList )
{
  ROF( m_bInitialized );
  
  RNOK ( m_pcDecodedPictureBuffer->clear( rcPicBufferOutputList, rcPicBufferUnusedList ) );
  while( m_cSliceHeaderList.size() )
  {
    SliceHeader*  pcSliceHeader = m_cSliceHeaderList.popFront();
    delete        pcSliceHeader;
  }
  return Err::m_nOK;
}


ErrVal
LayerDecoder::xInitSlice( SliceHeader*&      rpcSliceHeader,
                         PicBuffer*         pcPicBuffer,
                         PicBufferList&     rcPicBufferOutputList,
                         PicBufferList&     rcPicBufferUnusedList,
                         SliceDataNALUnit&  rcSliceDataNalUnit )
{
  //===== delete non-required slice headers =====
  if( ! m_bDependencyRepresentationInitialized )
  {
    while( m_cSliceHeaderList.size() )
    {
      SliceHeader*  pcSliceHeader = m_cSliceHeaderList.popFront();
      delete        pcSliceHeader;
    }
  }

  //===== create, read, init, and store slice header (and init SPS when required) =====
  RNOK( xReadSliceHeader          (  rpcSliceHeader, rcSliceDataNalUnit ) );
  RNOK( xInitSliceHeader          ( *rpcSliceHeader, rcSliceDataNalUnit ) );
  RNOK( xInitSPS                  ( *rpcSliceHeader ) );
  m_cSliceHeaderList.push_back    (  rpcSliceHeader );

  //===== init DPB unit (when required) =====
  RNOK( xInitDPBUnit              ( *rpcSliceHeader, pcPicBuffer, rcPicBufferOutputList, rcPicBufferUnusedList ) );

  //===== init resize parameters =====  ---> write new function xInitResizeParameters, which is somewhat nicer 
  RNOK( xInitESSandCroppingWindow ( *rpcSliceHeader, *m_pcCurrDPBUnit->getCtrlData().getMbDataCtrl(), m_pcCurrDPBUnit->getCtrlData() ) );

  //===== update parameters =====
  m_bLayerRepresentationInitialized       = true;
  m_bDependencyRepresentationInitialized  = true;

  return Err::m_nOK;
}

ErrVal
LayerDecoder::xParseSlice( SliceHeader& rcSliceHeader )
{
  ROF( m_pcCurrDPBUnit );
  ControlData&  rcControlData = m_pcCurrDPBUnit->getCtrlData();
  MbDataCtrl*   pcMbDataCtrl  = rcControlData.getMbDataCtrl();
  UInt          uiMbRead      = 0;

  RNOK( m_pcControlMng  ->initSliceForReading ( rcSliceHeader ) );
  RNOK( m_pcSliceReader ->read                ( rcSliceHeader,
                                                pcMbDataCtrl,
                                                m_pacMbStatus,
                                                m_uiFrameWidthInMb,
                                                uiMbRead ) );

  return Err::m_nOK;
}

ErrVal
LayerDecoder::xDecodeSlice( SliceHeader&            rcSliceHeader,
                           PicBufferList&          rcPicBufferOutputList,
                           PicBufferList&          rcPicBufferUnusedList,
                           const SliceDataNALUnit& rcSliceDataNalUnit )
{
  ROF( m_pcCurrDPBUnit );
  Bool          bReconstructBaseRep = rcSliceHeader.getStoreRefBasePicFlag() && ! rcSliceHeader.getQualityId();
  Bool          bReconstructAll     = rcSliceDataNalUnit.isDQIdMax();
  Bool          bReconstructMCMbs   = bReconstructAll || ( rcSliceDataNalUnit.isDependencyIdMax() && bReconstructBaseRep );
  PicType       ePicType            = rcSliceHeader.getPicType();
  SliceHeader*  pcSliceHeaderBase   = 0;
  ControlData&  rcControlData       = m_pcCurrDPBUnit->getCtrlData();
  Frame*        pcFrame             = m_pcCurrDPBUnit->getFrame   ();
  Frame*        pcBaseRepFrame      = m_apcFrameTemp[0];
  Frame*        pcRecFrame          = ( bReconstructBaseRep ? pcBaseRepFrame : pcFrame );
  MbDataCtrl*   pcMbDataCtrl        = rcControlData.getMbDataCtrl ();

  //===== get reference frame lists =====
  RNOK( m_pcDecodedPictureBuffer->setPrdRefLists( m_pcCurrDPBUnit ) );
  RefFrameList& rcRefFrameList0 = rcControlData.getPrdFrameList( LIST_0 );
  RefFrameList& rcRefFrameList1 = rcControlData.getPrdFrameList( LIST_1 );
  MbDataCtrl*   pcMbDataCtrl0L1 = rcControlData.getMbDataCtrl0L1();
  rcSliceHeader.setRefFrameList( &rcRefFrameList0, FRAME, LIST_0 );
  rcSliceHeader.setRefFrameList( &rcRefFrameList1, FRAME, LIST_1 );

  //===== init base layer =====
  RNOK( xInitBaseLayer( rcControlData, pcSliceHeaderBase ) );

  //----- decoding -----
  RNOK( m_pcControlMng->initSliceForDecoding( rcSliceHeader ) );

  if( rcSliceHeader.isMbaffFrame() )
  {
    RNOK( m_pcSliceDecoder->decodeMbAff( rcSliceHeader,
                                         pcMbDataCtrl,
                                         rcControlData.getBaseLayerCtrl(),
                                         rcControlData.getBaseLayerCtrlField(),
                                         pcRecFrame,
                                         m_pcResidual,
                                         rcControlData.getBaseLayerRec(),
                                         rcControlData.getBaseLayerSbb(),
                                         &rcRefFrameList0,
                                         &rcRefFrameList1,
                                         pcMbDataCtrl0L1,
                                         bReconstructMCMbs ) ); 
  }
  else
  {
    RNOK( m_pcSliceDecoder->decode     ( rcSliceHeader,
                                         pcMbDataCtrl,
                                         rcControlData.getBaseLayerCtrl(),
                                         pcRecFrame,
                                         m_pcResidual,
                                         rcControlData.getBaseLayerRec(),
                                         rcControlData.getBaseLayerSbb(),
                                         &rcRefFrameList0,
                                         &rcRefFrameList1,
                                         pcMbDataCtrl0L1,
                                         bReconstructMCMbs ) );
  }

  printf("  %s %4d ( LId%2d, TL%2d, QL%2d, %s-%c, BId%2d, AP%2d, QP%3d )\n",
    ePicType == FRAME ?  "Frame" : ePicType == TOP_FIELD ? "TopFd" : "BotFd",
    rcSliceHeader.getPoc                  (),
    rcSliceHeader.getDependencyId         (),
    rcSliceHeader.getTemporalId           (),
    rcSliceHeader.getQualityId            (),
    rcSliceHeader.isH264AVCCompatible     () ? "AVC" : "SVC",
    rcSliceHeader.getSliceType            () == I_SLICE ? 'I' :
    rcSliceHeader.getSliceType            () == P_SLICE ? 'P' : 'B',
    rcSliceHeader.getRefLayerDQId         (),
    rcSliceHeader.getAdaptiveBaseModeFlag () ? 1 : 0,
    rcSliceHeader.getSliceQp              () );
  ROFRS( rcSliceDataNalUnit.isLastSliceInLayerRepresentation(), Err::m_nOK );


  //===== loop filter and store picture =====
  if( bReconstructBaseRep )
  {
    //----- copy non-filtered frame -----
    RNOK( pcFrame->copy( pcBaseRepFrame, ePicType ) );

    //----- loop-filtering and store in DPB as base representation -----
    m_pcLoopFilter->setHighpassFramePointer( m_pcResidual );
    RNOK( m_pcLoopFilter->process( rcSliceHeader,
                                   pcBaseRepFrame,
                                   ( rcSliceHeader.isIntraSlice() ? NULL : pcMbDataCtrl ),
                                   pcMbDataCtrl,
                                   m_uiFrameWidthInMb,
                                   &rcRefFrameList0,
                                   &rcRefFrameList1,
                                   false,
                                   rcControlData.getSpatialScalability() ) );
  }
  RNOK( m_pcILPrediction->copy( pcFrame, ePicType ) );
  if( bReconstructAll )
  {
    m_pcLoopFilter->setHighpassFramePointer( m_pcResidual );
    RNOK( m_pcLoopFilter->process( rcSliceHeader,
                                   pcFrame,
                                   ( rcSliceHeader.isIntraSlice() ? NULL : pcMbDataCtrl ),
                                   pcMbDataCtrl,
                                   m_uiFrameWidthInMb,
                                   &rcRefFrameList0,
                                   &rcRefFrameList1,
                                   false,
                                   rcControlData.getSpatialScalability() ) );
  }
  if( bReconstructBaseRep )
  {
    //----- store in DPB with base representation -----
    RNOK( m_pcDecodedPictureBuffer->store( m_pcCurrDPBUnit, rcPicBufferOutputList, rcPicBufferUnusedList, pcBaseRepFrame ) );
  }
  else
  {
    RNOK( m_pcDecodedPictureBuffer->store( m_pcCurrDPBUnit, rcPicBufferOutputList, rcPicBufferUnusedList ) );
  }

  if( rcSliceHeader.getStoreRefBasePicFlag() && bReconstructAll )
  {
    if( rcSliceHeader.getDecRefPicMarking().getAdaptiveRefPicMarkingModeFlag() )
    {
      RNOK( m_pcDecodedPictureBuffer->MMCOBase( &rcSliceHeader, rcSliceHeader.getFrameNum() ) );
    }
    else
    {
      RNOK( m_pcDecodedPictureBuffer->slidingWindowBase( rcSliceHeader.getFrameNum() ) );
    }
  }

  DTRACE_NEWFRAME;
  return Err::m_nOK;
}

ErrVal
LayerDecoder::xFinishSlice( PicBufferList& rcPicBufferOutputList, PicBufferList& rcPicBufferUnusedList, const SliceDataNALUnit& rcSliceDataNalUnit )
{
  //===== close Nal unit =====
  RNOK  ( m_pcNalUnitParser->closeNalUnit() );
  ROFRS ( rcSliceDataNalUnit.isLastSliceInLayerRepresentation(), Err::m_nOK );
  
  //===== update parameters =====
  m_bLayerRepresentationInitialized = false;
  m_uiQualityId++;
  ROFRS ( rcSliceDataNalUnit.isLastSliceInDependencyRepresentation(), Err::m_nOK );

  //===== reset macroblock status map and update parameters =====
  for( UInt uiMb = 0; uiMb < m_uiMbNumber; uiMb++ )
  {
    m_pacMbStatus[ uiMb ].reset();
  }
  m_uiQualityId                           = 0;
  m_bDependencyRepresentationInitialized  = false;

  return Err::m_nOK;
}

ErrVal
LayerDecoder::xReadSliceHeader( SliceHeader*&      rpcSliceHeader, 
                               SliceDataNALUnit&  rcSliceDataNalUnit )
{
  //===== parse prefix header when available =====
  PrefixHeader* pcPrefixHeader = 0;
  if( rcSliceDataNalUnit.getBinDataPrefix() )
  {
    BinDataAccessor cBinDataAccessorPrefix;
    rcSliceDataNalUnit.getBinDataPrefix()->setMemAccessor( cBinDataAccessorPrefix );
    RNOK( m_pcNalUnitParser ->initNalUnit   ( cBinDataAccessorPrefix )  );
    ROF ( m_pcNalUnitParser ->getNalUnitType() == NAL_UNIT_PREFIX );
    pcPrefixHeader = new PrefixHeader( *m_pcNalUnitParser );
    ROF ( pcPrefixHeader );
    RNOK( pcPrefixHeader    ->read          ( *m_pcHeaderSymbolReadIf ) );
    RNOK( m_pcNalUnitParser ->closeNalUnit  () );
  }

  //===== parse slice header =====
  BinDataAccessor cBinDataAccessor;
  rcSliceDataNalUnit.getBinData()->setMemAccessor( cBinDataAccessor );
  RNOK( m_pcNalUnitParser ->initNalUnit   ( cBinDataAccessor )  );
  if( pcPrefixHeader )
  {
    ROT( m_uiDependencyId );
    ROF( m_pcNalUnitParser->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR || m_pcNalUnitParser->getNalUnitType() == NAL_UNIT_CODED_SLICE );
    rpcSliceHeader  = new SliceHeader( *pcPrefixHeader );
    ROF( rpcSliceHeader );
    delete pcPrefixHeader; pcPrefixHeader = 0;
    rpcSliceHeader->NalUnitHeader::copy( *m_pcNalUnitParser, false );
  }
  else
  {
    rpcSliceHeader  = new SliceHeader( *m_pcNalUnitParser );
    ROF( rpcSliceHeader );
  }
  RNOK( rpcSliceHeader->read( *m_pcParameterSetMng, *m_pcHeaderSymbolReadIf ) );

  return Err::m_nOK;
}

ErrVal
LayerDecoder::xInitSliceHeader( SliceHeader& rcSliceHeader, const SliceDataNALUnit& rcSliceDataNalUnit )
{
  //===== check =====
  ROF( rcSliceHeader.getDependencyId() == m_uiDependencyId );
  ROF( rcSliceHeader.getQualityId   () == m_uiQualityId    );

  //===== init picture order count =====
  RNOK( m_pcPocCalculator->calculatePoc( rcSliceHeader ) );

  //===== infer slice header parameters =====
  if( rcSliceHeader.getQualityId() )
  {
    const SliceHeader* pcSliceHeader = m_pacMbStatus[ rcSliceHeader.getFirstMbInSlice() ].getSliceHeader();
    ROF( pcSliceHeader );
    rcSliceHeader.setDirectSpatialMvPredFlag        ( pcSliceHeader->getDirectSpatialMvPredFlag     () );
    rcSliceHeader.setNumRefIdxActiveOverrideFlag    ( pcSliceHeader->getNumRefIdxActiveOverrideFlag () );
    rcSliceHeader.setNumRefIdxL0ActiveMinus1        ( pcSliceHeader->getNumRefIdxL0ActiveMinus1     () );
    rcSliceHeader.setNumRefIdxL1ActiveMinus1        ( pcSliceHeader->getNumRefIdxL1ActiveMinus1     () );
    rcSliceHeader.getRefPicListReorderingL0 ().copy ( pcSliceHeader->getRefPicListReorderingL0      () );
    rcSliceHeader.getRefPicListReorderingL1 ().copy ( pcSliceHeader->getRefPicListReorderingL1      () );
    rcSliceHeader.setBasePredWeightTableFlag        ( pcSliceHeader->getBasePredWeightTableFlag     () );
    rcSliceHeader.setLumaLog2WeightDenom            ( pcSliceHeader->getLumaLog2WeightDenom         () );
    rcSliceHeader.setChromaLog2WeightDenom          ( pcSliceHeader->getChromaLog2WeightDenom       () );
    rcSliceHeader.getPredWeightTableL0      ().copy ( pcSliceHeader->getPredWeightTableL0           () );
    rcSliceHeader.getPredWeightTableL1      ().copy ( pcSliceHeader->getPredWeightTableL1           () );
    rcSliceHeader.setNoOutputOfPriorPicsFlag        ( pcSliceHeader->getNoOutputOfPriorPicsFlag     () );
    rcSliceHeader.setLongTermReferenceFlag          ( pcSliceHeader->getLongTermReferenceFlag       () );
    rcSliceHeader.getDecRefPicMarking       ().copy ( pcSliceHeader->getDecRefPicMarking            () );
    rcSliceHeader.setStoreRefBasePicFlag            ( pcSliceHeader->getStoreRefBasePicFlag         () );
    rcSliceHeader.getDecRefBasePicMarking   ().copy ( pcSliceHeader->getDecRefBasePicMarking        () );
    rcSliceHeader.setSliceGroupChangeCycle          ( pcSliceHeader->getSliceGroupChangeCycle       () );
  }

  //===== infer prediction weights when required =====
  if( rcSliceHeader.getPPS().getWeightedBiPredIdc () == 1       && 
      rcSliceHeader.getSliceType                  () == B_SLICE &&
     !rcSliceHeader.getNoInterLayerPredFlag       ()            &&
      rcSliceHeader.getBasePredWeightTableFlag    ()              )
  {
    SliceHeader*  pcBaseSliceHeader = 0;
    RNOK( m_pcH264AVCDecoder->getBaseSliceHeader( pcBaseSliceHeader, rcSliceHeader.getRefLayerDependencyId() ) );
    rcSliceHeader.setLumaLog2WeightDenom  ( pcBaseSliceHeader->getLumaLog2WeightDenom  () );
    rcSliceHeader.setChromaLog2WeightDenom( pcBaseSliceHeader->getChromaLog2WeightDenom() );
    rcSliceHeader.getPredWeightTableL0().copy( pcBaseSliceHeader->getPredWeightTableL0 () );
    rcSliceHeader.getPredWeightTableL1().copy( pcBaseSliceHeader->getPredWeightTableL1 () );
  }
  else if( rcSliceHeader.getPPS().getWeightedPredFlag ()            && 
           rcSliceHeader.getSliceType                 () == P_SLICE &&
          !rcSliceHeader.getNoInterLayerPredFlag      ()            &&
           rcSliceHeader.getBasePredWeightTableFlag   ()              )
  {
    SliceHeader*  pcBaseSliceHeader = 0;
    RNOK( m_pcH264AVCDecoder->getBaseSliceHeader( pcBaseSliceHeader, rcSliceHeader.getRefLayerDependencyId() ) );
    rcSliceHeader.setLumaLog2WeightDenom  ( pcBaseSliceHeader->getLumaLog2WeightDenom  () );
    rcSliceHeader.setChromaLog2WeightDenom( pcBaseSliceHeader->getChromaLog2WeightDenom() );
    rcSliceHeader.getPredWeightTableL0().copy( pcBaseSliceHeader->getPredWeightTableL0 () );
  }

  //===== init FMO ==== !!!! check & improve that part !!!!!
  rcSliceHeader.FMOInit();
#if 1 // what is that -> is this really required ?????
  if( rcSliceHeader.getNumMbsInSlice() )
  {
    rcSliceHeader.setLastMbInSlice( rcSliceHeader.getFMO()->getLastMbInSlice( rcSliceHeader.getFirstMbInSlice(), rcSliceHeader.getNumMbsInSlice() ) );
  }
  else
  {
    rcSliceHeader.setLastMbInSlice( rcSliceHeader.getFMO()->getLastMBInSliceGroup( rcSliceHeader.getFMO()->getSliceGroupId( rcSliceHeader.getFirstMbInSlice() ) ) );
  }
  if( !rcSliceHeader.getSPS().getFrameMbsOnlyFlag() )
  {
    if( !rcSliceHeader.getFieldPicFlag() && rcSliceHeader.getSPS().getMbAdaptiveFrameFieldFlag() )
    {
      rcSliceHeader.setFirstMbInSlice( rcSliceHeader.getFirstMbInSlice() << 1 );
    }
  }
#endif

  return Err::m_nOK;
}

ErrVal
LayerDecoder::xInitSPS( const SliceHeader& rcSliceHeader )
{
  ROTRS( m_bSPSInitialized, Err::m_nOK );

  //===== init control manager =====
  RNOK( m_pcControlMng->initSlice0( const_cast<SliceHeader*>(&rcSliceHeader) ) );

  //===== set frame size parameters =====
  m_uiFrameWidthInMb  = rcSliceHeader.getSPS().getFrameWidthInMbs();
  m_uiFrameHeightInMb = rcSliceHeader.getSPS().getFrameHeightInMbs();
  m_uiMbNumber        = rcSliceHeader.getMbInPic();

  //===== re-allocate dynamic memory =====
  RNOK( xDeleteData() );
  RNOK( xCreateData( rcSliceHeader.getSPS() ) );

  //===== initialize DPB =====
  RNOK( m_pcDecodedPictureBuffer->initSPS( rcSliceHeader.getSPS(), m_uiDependencyId ) );

  //===== set initialization status =====
  m_bSPSInitialized = true;
  return Err::m_nOK;
}

ErrVal
LayerDecoder::xInitDPBUnit( SliceHeader&   rcSliceHeader,
                           PicBuffer*     pcPicBuffer, 
                           PicBufferList& rcPicBufferOutputList, 
                           PicBufferList& rcPicBufferUnusedList )
{
  if( ! m_bLayerRepresentationInitialized )
  {
    RNOK( m_pcDecodedPictureBuffer->initPicCurrDPBUnit( pcPicBuffer, m_bDependencyRepresentationInitialized ) );
  }
  RNOK( m_pcDecodedPictureBuffer->initCurrDPBUnit( m_pcCurrDPBUnit, &rcSliceHeader, rcPicBufferOutputList, rcPicBufferUnusedList ) );
  
  m_pcCurrDPBUnit->getCtrlData().setBaseLayerRec      ( 0 );
  m_pcCurrDPBUnit->getCtrlData().setBaseLayerSbb      ( 0 );
  m_pcCurrDPBUnit->getCtrlData().setBaseLayerCtrl     ( 0 );
  m_pcCurrDPBUnit->getCtrlData().setBaseLayerCtrlField( 0 );

  if( pcPicBuffer )
  {
    rcPicBufferUnusedList.push_back( pcPicBuffer );
  }

  return Err::m_nOK;
}


ErrVal
LayerDecoder::getBaseSliceHeader( SliceHeader*& rpcSliceHeader )
{
  ROF( m_bInitialized );

  DPBUnit* pcBaseDPBUnit = m_pcDecodedPictureBuffer->getLastUnit();
  ROF( pcBaseDPBUnit );
  rpcSliceHeader = pcBaseDPBUnit->getCtrlData().getSliceHeader();
  ROF( rpcSliceHeader );
  return Err::m_nOK;
}


ErrVal
LayerDecoder::getBaseLayerDataAvailability ( Frame*&       pcFrame,
                                            Frame*&       pcResidual,
                                            MbDataCtrl*&  pcMbDataCtrl,
                                            Bool&         bBaseDataAvailable )
{
  ROF( m_bInitialized );

  DPBUnit*  pcBaseDPBUnit = m_pcDecodedPictureBuffer->getLastUnit();
  ROF( pcBaseDPBUnit );
  pcMbDataCtrl            = pcBaseDPBUnit->getCtrlData().getMbDataCtrl();
  pcResidual              = m_pcResidual;
  pcFrame                 = m_pcILPrediction;
  bBaseDataAvailable      = pcFrame && pcResidual && pcMbDataCtrl;
  return Err::m_nOK;
}


ErrVal
LayerDecoder::getBaseLayerData ( Frame*&       pcFrame,
                                Frame*&       pcResidual,
                                MbDataCtrl*&  pcMbDataCtrl,
                                Bool&         rbConstrainedIPred,
                                Bool          bSpatialScalability )
{
  ROF( m_bInitialized );

  DPBUnit*      pcBaseDPBUnit   = m_pcDecodedPictureBuffer->getLastUnit();
  ROF( pcBaseDPBUnit );
  pcMbDataCtrl                  = pcBaseDPBUnit->getCtrlData().getMbDataCtrl ();
  SliceHeader*  pcSliceHeader   = pcBaseDPBUnit->getCtrlData().getSliceHeader();
	const PicType ePicType        =  pcSliceHeader->getPicType                  ();
  rbConstrainedIPred            = pcBaseDPBUnit->isConstrIPred               ();
  pcFrame                       = m_pcILPrediction;
  pcResidual                    = m_pcResidual;

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
LayerDecoder::xCreateData( const SequenceParameterSet& rcSPS )
{
  UInt uiIndex;

  //========== CREATE FRAME MEMORIES ==========
  for( uiIndex = 0; uiIndex < NUM_TMP_FRAMES;  uiIndex++ )
  {
  	RNOK( Frame::create( m_apcFrameTemp[ uiIndex ], *m_pcYuvFullPelBufferCtrl, *m_pcYuvFullPelBufferCtrl, FRAME ) );
    RNOK( m_apcFrameTemp[ uiIndex ]->init() );
  }

  RNOK( Frame::create( m_pcResidual,          *m_pcYuvFullPelBufferCtrl, *m_pcYuvFullPelBufferCtrl, FRAME ) );
  RNOK( Frame::create( m_pcILPrediction,      *m_pcYuvFullPelBufferCtrl, *m_pcYuvFullPelBufferCtrl, FRAME ) );
  RNOK( Frame::create( m_pcBaseLayerFrame,    *m_pcYuvFullPelBufferCtrl, *m_pcYuvFullPelBufferCtrl, FRAME ) );
  RNOK( Frame::create( m_pcBaseLayerResidual, *m_pcYuvFullPelBufferCtrl, *m_pcYuvFullPelBufferCtrl, FRAME ) );
  RNOK( m_pcResidual          ->init() );
  RNOK( m_pcILPrediction      ->init() );
  RNOK( m_pcBaseLayerFrame    ->init() );
  RNOK( m_pcBaseLayerResidual ->init() );

  //========== CREATE MACROBLOCK DATA MEMORIES ==========
  ROFS( ( m_pcBaseLayerCtrl = new MbDataCtrl() ) );
  RNOK(   m_pcBaseLayerCtrl ->init( rcSPS ) );
  ROFS( ( m_pcBaseLayerCtrlField = new MbDataCtrl() ) );
  RNOK(   m_pcBaseLayerCtrlField ->init( rcSPS ) );

  //========== CREATE UPDATE WEIGHTS ARRAY and WRITE BUFFER ==========
  ROT ( m_cDownConvert.init( m_uiFrameWidthInMb<<4, m_uiFrameHeightInMb<<4 ) );

  //========= CREATE STATUS MAP ======
  ROFS( ( m_pacMbStatus = new MbStatus[ m_uiMbNumber ] ) );

  return Err::m_nOK;
}


ErrVal 
LayerDecoder::xDeleteData()
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

  if( m_pacMbStatus )
  {
    delete [] m_pacMbStatus;
    m_pacMbStatus = 0;
  }
  ROF( m_cSliceHeaderList.empty() );

  return Err::m_nOK;
}


ErrVal
LayerDecoder::xInitESSandCroppingWindow( SliceHeader&  rcSliceHeader,
                                        MbDataCtrl&   rcMbDataCtrl,
                                        ControlData&    rcControlData)
{ 
  if( rcSliceHeader.getNoInterLayerPredFlag() || rcSliceHeader.getQualityId() )
  {
    for( Int iMbY = 0; iMbY < (Int)m_uiFrameHeightInMb; iMbY++ )
    for( Int iMbX = 0; iMbX < (Int)m_uiFrameWidthInMb;  iMbX++ )
    {
      rcMbDataCtrl.getMbDataByIndex( (UInt)iMbY*m_uiFrameWidthInMb+(UInt)iMbX ).setInCropWindowFlag( true );
    }
    return Err::m_nOK;
  }

  //===== clear cropping window flags =====
  for( UInt uiMbY = 0; uiMbY < m_uiFrameHeightInMb; uiMbY++ )
  for( UInt uiMbX = 0; uiMbX < m_uiFrameWidthInMb;  uiMbX++ )
  {
    rcMbDataCtrl.getMbDataByIndex( uiMbY * m_uiFrameWidthInMb + uiMbX ).setInCropWindowFlag( false );
  }

  //===== init resize parameter =====
  m_pcResizeParameter->m_bFrameMbsOnlyFlag          = rcSliceHeader.getSPS().getFrameMbsOnlyFlag();
  m_pcResizeParameter->m_bFieldPicFlag			        = rcSliceHeader.getFieldPicFlag();
  m_pcResizeParameter->m_bIsMbAff                   = rcSliceHeader.isMbaffFrame();
  m_pcResizeParameter->m_bBotFieldFlag			        = rcSliceHeader.getBottomFieldFlag();
  m_pcResizeParameter->setPictureParametersByOffset ( rcSliceHeader.getPoc(),
                                                      rcSliceHeader.getScaledRefLayerLeftOffset(),
                                                      rcSliceHeader.getScaledRefLayerRightOffset(),
                                                      rcSliceHeader.getScaledRefLayerTopOffset(),
                                                      rcSliceHeader.getScaledRefLayerBottomOffset(),
                                                      rcSliceHeader.getRefLayerChromaPhaseX(),
                                                      rcSliceHeader.getRefLayerChromaPhaseY() );
  m_pcResizeParameter->setPoc( rcSliceHeader.getPoc() );

  //===== set crop window flag: in current macroblock data (we don't need the base layer here) =====
  if( m_pcResizeParameter->m_iSpatialScalabilityType == SST_RATIO_1 ||
      m_pcResizeParameter->m_iSpatialScalabilityType == SST_RATIO_2   )
  {
    Int iMbOrigX  = m_pcResizeParameter->m_iPosX      / 16;
    Int iMbOrigY  = m_pcResizeParameter->m_iPosY      / 16;
    Int iMbEndX   = m_pcResizeParameter->m_iOutWidth  / 16 + iMbOrigX;
    Int iMbEndY   = m_pcResizeParameter->m_iOutHeight / 16 + iMbOrigY;
    for( Int iMbY = iMbOrigY; iMbY < iMbEndY; iMbY++ )
    for( Int iMbX = iMbOrigX; iMbX < iMbEndX; iMbX++ )
    {
      rcMbDataCtrl.getMbDataByIndex( (UInt)iMbY*m_uiFrameWidthInMb+(UInt)iMbX ).setInCropWindowFlag( true );
    }
  }
  else
  {
    // ESS
    if( m_pcResizeParameter->m_iExtendedSpatialScalability == ESS_PICT )
    {
      m_pcResizeParameter->setCurrentPictureParametersWith( m_pcResizeParameter->getPoc() );  // really ugly
    }
    Int iScaledBaseOrigX  = m_pcResizeParameter->m_iPosX;
    Int iScaledBaseOrigY  = m_pcResizeParameter->m_iPosY;
    Int iScaledBaseWidth  = m_pcResizeParameter->m_iOutWidth;
    Int iScaledBaseHeight = m_pcResizeParameter->m_iOutHeight;
   
    
		if ( ! m_pcResizeParameter->m_bIsMbAff )
		{
      for( Int iMbY = 0; iMbY < (Int)m_uiFrameHeightInMb; iMbY++ )
      for( Int iMbX = 0; iMbX < (Int)m_uiFrameWidthInMb;  iMbX++ )
      {
        if( ( iMbX >= ( iScaledBaseOrigX + 15 ) / 16 ) && ( iMbX < ( iScaledBaseOrigX + iScaledBaseWidth  ) / 16 ) &&
            ( iMbY >= ( iScaledBaseOrigY + 15 ) / 16 ) && ( iMbY < ( iScaledBaseOrigY + iScaledBaseHeight ) / 16 )   )
        {
        rcMbDataCtrl.getMbDataByIndex( (UInt)iMbY*m_uiFrameWidthInMb+(UInt)iMbX ).setInCropWindowFlag( true );
        }
      }
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
  }

  return Err::m_nOK;
}


ErrVal
LayerDecoder::xInitBaseLayer( ControlData&   rcControlData,
						                 SliceHeader*&  rcSliceHeaderBase )
{
  //===== init =====
  rcControlData.setBaseLayerRec       ( 0 );
  rcControlData.setBaseLayerSbb       ( 0 );
  rcControlData.setBaseLayerCtrl      ( 0 );
  rcControlData.setBaseLayerCtrlField ( 0 );
  
  Frame*        pcBaseFrame         = 0;
  Frame*        pcBaseResidual      = 0;
  MbDataCtrl*   pcBaseDataCtrl      = 0;
  Bool          bConstrainedIPredBL = false;
  Bool          bSpatialScalability = false; 
  Bool          bBaseDataAvailable  = false;
  SliceHeader*  pcSliceHeader       = rcControlData.getSliceHeader();
  const PicType ePicType            = pcSliceHeader->getPicType();
 
  //===== create resize parameters ======  !!!! not nice !!! -> completely rewrite that resize parameter thing
  ResizeParameters  cQResizeParameters;
  ResizeParameters* pcResizeParameters = m_pcResizeParameter;
  if( pcSliceHeader->getQualityId() )
  {
    pcResizeParameters                                = &cQResizeParameters;
    pcResizeParameters->m_iExtendedSpatialScalability = ESS_NONE;
    pcResizeParameters->m_bCrop                       = false;
    pcResizeParameters->m_iSpatialScalabilityType     = SST_RATIO_1;
    pcResizeParameters->m_iGlobWidth                  = m_uiFrameWidthInMb  << 4;
    pcResizeParameters->m_iGlobHeight                 = m_uiFrameHeightInMb << 4;
    pcResizeParameters->m_iOutWidth                   = pcResizeParameters->m_iGlobWidth;
    pcResizeParameters->m_iOutHeight                  = pcResizeParameters->m_iGlobHeight;
    pcResizeParameters->m_iInWidth                    = pcResizeParameters->m_iGlobWidth;
    pcResizeParameters->m_iInHeight                   = pcResizeParameters->m_iGlobHeight;
    pcResizeParameters->m_iPosX                       = 0;
    pcResizeParameters->m_iPosY                       = 0;
    pcResizeParameters->m_iChromaPhaseX               = (Int)pcSliceHeader->getSPS().getChromaPhaseXPlus1() - 1;
    pcResizeParameters->m_iChromaPhaseY               = (Int)pcSliceHeader->getSPS().getChromaPhaseYPlus1() - 1;
    pcResizeParameters->m_iBaseChromaPhaseX           = pcResizeParameters->m_iChromaPhaseX;
    pcResizeParameters->m_iBaseChromaPhaseY           = pcResizeParameters->m_iChromaPhaseY;
    pcResizeParameters->m_bFrameMbsOnlyFlag           = pcSliceHeader->getSPS().getFrameMbsOnlyFlag();
    pcResizeParameters->m_bFieldPicFlag			          = pcSliceHeader->getFieldPicFlag();
    pcResizeParameters->m_bIsMbAff                    = pcSliceHeader->isMbaffFrame();
    pcResizeParameters->m_bBotFieldFlag			          = pcSliceHeader->getBottomFieldFlag();
    pcResizeParameters->m_iPoc                        = pcSliceHeader->getPoc( ePicType );
    pcResizeParameters->m_iResampleMode               = 0;
    pcResizeParameters->m_bBaseFrameMbsOnlyFlag       = pcResizeParameters->m_bFrameMbsOnlyFlag;
    pcResizeParameters->m_bBaseFieldPicFlag           = pcResizeParameters->m_bFieldPicFlag;
    pcResizeParameters->m_bBaseIsMbAff                = pcResizeParameters->m_bIsMbAff;
    pcResizeParameters->m_bBaseBotFieldFlag           = pcResizeParameters->m_bBaseBotFieldFlag;
    pcResizeParameters->SetUpSampleMode           ();
    rcControlData      .setSpatialScalability     ( false );
    rcControlData      .setSpatialScalabilityType ( SST_RATIO_1 );
    pcSliceHeader     ->setSpatialScalabilityType ( SST_RATIO_1 );
  }

  if( ! pcSliceHeader->getNoInterLayerPredFlag() )
  {
    RNOK( xGetBaseLayerData( rcControlData,
                             pcBaseFrame, 
                             pcBaseResidual, 
                             pcBaseDataCtrl, 
                             bBaseDataAvailable, 
                             bConstrainedIPredBL, 
                             bSpatialScalability,
                             pcResizeParameters ) );
   
    ROF( bBaseDataAvailable );
  }

  //===== motion data =====
  if( pcBaseDataCtrl )
  {
    if( pcResizeParameters->m_iExtendedSpatialScalability == ESS_PICT ) 
    {
      RefFrameList& rcList0=rcControlData.getPrdFrameList( LIST_0 );
      RefFrameList& rcList1=rcControlData.getPrdFrameList( LIST_1 );
	 
      UInt uiIndex;
      for( uiIndex = 1; uiIndex <= rcList0.getActive(); uiIndex++ )
      {
        pcResizeParameters->m_aiRefListPoc[0][uiIndex-1]=rcList0[uiIndex]->getPoc() ;
      }
      for( uiIndex = 1; uiIndex <= rcList1.getActive(); uiIndex++ )
      {
        pcResizeParameters->m_aiRefListPoc[1][uiIndex-1]=rcList1[uiIndex]->getPoc() ;
      }
    }

    //=== create Upsampled VBL Frame ===
    RNOK( m_pcBaseLayerCtrl->initSlice( *pcSliceHeader, PRE_PROCESS, false, NULL ) );
    RNOK( pcBaseDataCtrl->switchMotionRefinement() );
    m_pcBaseLayerCtrl->setBuildInterlacePred( pcResizeParameters->m_bFieldPicFlag );

    RNOK( m_pcBaseLayerCtrl->upsampleMotion( *pcBaseDataCtrl, pcResizeParameters ) );
    if( pcSliceHeader->getQualityId() )
    {
      RNOK( m_pcBaseLayerCtrl->initMbCBP( *pcBaseDataCtrl, pcResizeParameters ) );
    }
    
    pcSliceHeader->setCoeffResidualPredFlag( pcBaseDataCtrl->getSliceHeader() );
    //===== data needed for residual prediction in transform domain or SVC to AVC translation====/
    Bool avcRewriteFlag = pcSliceHeader->getTCoeffLevelPredictionFlag();
  
    if( avcRewriteFlag || pcSliceHeader->getCoeffResidualPredFlag() )
    { 
      m_pcBaseLayerCtrl->copyTCoeffs( *pcBaseDataCtrl );
      if( rcControlData.getSliceHeader()->getTCoeffLevelPredictionFlag() )
      { 	
        m_pcBaseLayerCtrl->copyIntraPred( *pcBaseDataCtrl );
      }    
    }
 
    rcControlData.setBaseLayerCtrl( m_pcBaseLayerCtrl );

    //=== create Upsampled VBL Field ===
    if( pcResizeParameters->m_bIsMbAff )
    {
      RNOK( m_pcBaseLayerCtrlField->initSlice( *pcSliceHeader, PRE_PROCESS, false, NULL ) );
      m_pcBaseLayerCtrlField->setBuildInterlacePred( true );
      RNOK( m_pcBaseLayerCtrlField->upsampleMotion( *pcBaseDataCtrl, pcResizeParameters ) );
      rcControlData.setBaseLayerCtrlField( m_pcBaseLayerCtrlField );
    }

    RNOK( pcBaseDataCtrl->switchMotionRefinement() );
  }

  //===== residual data =====
  if( bBaseDataAvailable )
  {
    if (pcSliceHeader->getCoeffResidualPredFlag() )
    {
      if( ! pcBaseDataCtrl->getSliceHeader()->getNoInterLayerPredFlag() )
      {        
        RNOK( m_pcH264AVCDecoder->getBaseLayerResidual( pcBaseResidual, pcSliceHeader->getRefLayerDependencyId() ) );
      }
      else
      {
        pcBaseResidual->getFullPelYuvBuffer()->clear();
      }
    }

    RNOK( m_pcBaseLayerResidual->copy( pcBaseResidual, ePicType ) );
	  RNOK( m_pcBaseLayerResidual->upsampleResidual( m_cDownConvert, pcResizeParameters, pcBaseDataCtrl, false) ); 
    rcControlData.setBaseLayerSbb( m_pcBaseLayerResidual );
  }

  //===== reconstruction (intra) =====
  if( pcBaseFrame )
  {
	  if(rcControlData.getSliceHeader()->getConstrainedIntraResamplingFlag())
    {
		  xConstrainedIntraUpsampling(pcBaseFrame,m_pcBaseLayerFrame,m_apcFrameTemp[0],pcBaseDataCtrl,m_pcReconstructionBypass, pcResizeParameters, ePicType);
	  }
	  else
	  {
      RNOK( m_pcBaseLayerFrame->copy( pcBaseFrame, ePicType ) );
	    pcResizeParameters->m_level_idc = rcControlData.getSliceHeader()->getSPS().getLevelIdc();//jzxu 03Nov2007
      RNOK( m_pcBaseLayerFrame->upsample( m_cDownConvert, pcResizeParameters, true ) );
	  }
    rcControlData.setBaseLayerRec( m_pcBaseLayerFrame );
  }
  if(pcBaseDataCtrl==NULL) rcSliceHeaderBase=NULL;
  else rcSliceHeaderBase=pcBaseDataCtrl->getSliceHeader();

  xSetMCResizeParameters(m_pcResizeParameter);

  return Err::m_nOK;
}


Void
LayerDecoder::xSetMCResizeParameters   (ResizeParameters*				resizeParameters)
{
  m_pcMotionCompensation->setResizeParameters(resizeParameters);
} 


ErrVal
LayerDecoder::xGetBaseLayerData( ControlData&      rcControlData,
                                Frame*&           rpcBaseFrame,
                                Frame*&           rpcBaseResidual,
                                MbDataCtrl*&      rpcBaseDataCtrl,
                                Bool&             rbBaseDataAvailable,
                                Bool&             rbConstrainedIPredBL,
                                Bool&             rbSpatialScalability,
                                ResizeParameters* pcResizeParameter)
 
{
 SliceHeader* pcSliceHeader = rcControlData.getSliceHeader();

 RNOK( m_pcH264AVCDecoder->getBaseLayerDataAvailability( rpcBaseFrame, 
																						           	 rpcBaseResidual, 
																						             rpcBaseDataCtrl, 
          																							 rbBaseDataAvailable,
                                                         rbSpatialScalability,
                                                         m_uiDependencyId,
                                                         pcSliceHeader->getRefLayerDependencyId() ) );    

  ROF( rbBaseDataAvailable );

  pcResizeParameter->m_iResampleMode          = 0; 
  pcResizeParameter->m_bBaseFrameMbsOnlyFlag	= rpcBaseDataCtrl->getSliceHeader()->getSPS().getFrameMbsOnlyFlag();
  pcResizeParameter->m_bBaseFieldPicFlag			= rpcBaseDataCtrl->getSliceHeader()->getFieldPicFlag();
  pcResizeParameter->m_bBaseIsMbAff           = rpcBaseDataCtrl->getSliceHeader()->isMbaffFrame();   
  pcResizeParameter->m_bBaseBotFieldFlag			= rpcBaseDataCtrl->getSliceHeader()->getBottomFieldFlag();
  pcResizeParameter->SetUpSampleMode();

  rcControlData.setSpatialScalability     ( rbSpatialScalability );
  rcControlData.setSpatialScalabilityType ( pcResizeParameter->m_iSpatialScalabilityType );
  rcControlData.getSliceHeader()->setSpatialScalabilityType(pcResizeParameter->m_iSpatialScalabilityType);
  
  rbSpatialScalability = rbSpatialScalability? rbSpatialScalability : pcResizeParameter->m_iResampleMode>0;

	RNOK( m_pcH264AVCDecoder->getBaseLayerData( rpcBaseFrame, 
   	                                          rpcBaseResidual, 
   	                                          rpcBaseDataCtrl,
   	                                          rbConstrainedIPredBL, 
   	                                          rbSpatialScalability,
    																					pcSliceHeader->getRefLayerDependencyId() ) );
 
  return Err::m_nOK;
}


ErrVal
LayerDecoder::xConstrainedIntraUpsampling( Frame*             pcFrame,
										                      Frame*             pcUpsampling, 
										                      Frame*             pcTemp,
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

void LayerDecoder::xGetPosition(ResizeParameters* pcResizeParameters,Int*px,Int*py,bool uv_flag)

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

H264AVC_NAMESPACE_END

