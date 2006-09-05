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





#include "H264AVCCommonLib.h"

#include "H264AVCCommonLib/QuarterPelFilter.h"
#include "H264AVCCommonLib/FrameMng.h"



H264AVC_NAMESPACE_BEGIN



UInt FrameMng:: m_uiDBPMemory[256 ]  =
{
  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,152064 ,345600 ,912384 ,912384 ,0 ,0 ,0 ,0 ,0 ,0 ,912384 ,1824768 ,3110400 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,3110400 ,6912000 ,7864320 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,12582912 ,12582912 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,41656320 ,70778880 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0
};


FrameMng::FrameUnitBuffer::FrameUnitBuffer()
 :  m_pcYuvFullPelBufferCtrl( NULL )
 ,  m_pcYuvHalfPelBufferCtrl( NULL )
{
}

FrameMng::FrameUnitBuffer::~FrameUnitBuffer()
{
  AOF_DBG( m_cFreeList.empty() );
}

ErrVal FrameMng::FrameUnitBuffer::init( YuvBufferCtrl* pcYuvFullPelBufferCtrl, YuvBufferCtrl* pcYuvHalfPelBufferCtrl )
{
  ROT( NULL == pcYuvFullPelBufferCtrl );
  if( NULL == pcYuvHalfPelBufferCtrl )
  {
    m_pcYuvHalfPelBufferCtrl = pcYuvFullPelBufferCtrl;
  }
  else
  {
    m_pcYuvHalfPelBufferCtrl = pcYuvHalfPelBufferCtrl;
  }
  m_pcYuvFullPelBufferCtrl = pcYuvFullPelBufferCtrl;
  return Err::m_nOK;
}

ErrVal FrameMng::FrameUnitBuffer::uninit()
{
  FUIter iter;
  for( iter = m_cFreeList.begin(); iter != m_cFreeList.end(); iter++ )
  {
    RNOK( (*iter)->destroy() );
  }
  m_cFreeList.clear();

  m_pcYuvFullPelBufferCtrl = NULL;
  m_pcYuvHalfPelBufferCtrl = NULL;
  return Err::m_nOK;
}

ErrVal FrameMng::FrameUnitBuffer::getFrameUnit( FrameUnit*& rpcFrameUnit )
{
  if( m_cFreeList.empty() )
  {
    return FrameUnit::create( rpcFrameUnit, *m_pcYuvFullPelBufferCtrl, *m_pcYuvHalfPelBufferCtrl );
  }

  rpcFrameUnit = m_cFreeList.popBack();
  return Err::m_nOK;
}

ErrVal FrameMng::FrameUnitBuffer::releaseFrameUnit( FrameUnit* pcFrameUnit )
{
  AOT_DBG( NULL == pcFrameUnit );
  m_cFreeList.push_back( pcFrameUnit );
  return Err::m_nOK;
}


FrameMng::FrameMng()
: m_bInitDone               ( false )
, m_pcQuarterPelFilter      ( NULL )
, m_pcOriginalFrameUnit     ( NULL )
, m_pcCurrentFrameUnit      ( NULL )
{
  m_uiPrecedingRefFrameNum  = 0;
  m_iEntriesInDPB           = 0;
  m_iMaxEntriesinDPB        = 0;
  m_uiNumRefFrames          = 0;
  m_uiMaxFrameNumCurr       = 0;
  m_uiMaxFrameNumPrev       = 0;
  m_pcRefinementIntFrame    = 0;
  m_pcRefinementIntFrameSpatial = 0;
  m_pcPredictionIntFrame    = 0;

}

FrameMng::~FrameMng()
{
}


ErrVal FrameMng::create( FrameMng*& rpcFrameMng )
{
  rpcFrameMng = new FrameMng;
  ROT( NULL == rpcFrameMng );
  return Err::m_nOK;
}


ErrVal FrameMng::destroy()
{
  delete this;
  return Err::m_nOK;
}


__inline Bool FrameMng::xFindAndErase( FUList& rcFUList, FrameUnit* pcFrameUnit )
{
  FUIter iter = std::find( rcFUList.begin(), rcFUList.end(), pcFrameUnit );

  ROTRS( rcFUList.end() == iter, false );

  rcFUList.erase( iter );
  return true;
}


__inline ErrVal FrameMng::xAddToFreeList( FrameUnit* pcFrameUnit )
{
  if( pcFrameUnit->getFGSPicBuffer() )
  {
    pcFrameUnit->getFGSPicBuffer()->setUnused();
    if( ! pcFrameUnit->getFGSPicBuffer()->isUsed() )
    {
      m_cPicBufferUnusedList.push_back( pcFrameUnit->getFGSPicBuffer() );
    }
  }

  if( pcFrameUnit->getPicBuffer() )
  {
    pcFrameUnit->getPicBuffer()->setUnused();
    if( ! pcFrameUnit->getPicBuffer()->isUsed() )
    {
      m_cPicBufferUnusedList.push_back( pcFrameUnit->getPicBuffer() );
    }
  }


  RNOK( pcFrameUnit->uninit() );
  RNOK( m_cFrameUnitBuffer.releaseFrameUnit( pcFrameUnit ) );

  m_iEntriesInDPB--;
  AOT_DBG( m_iEntriesInDPB < 0 );
  return Err::m_nOK;
}


__inline ErrVal FrameMng::xAddToFreeList( FUList& rcFUList )
{
  for( FUIter iter = rcFUList.begin(); iter != rcFUList.end(); iter++ )
  {
    RNOK( xAddToFreeList( *iter ) );
  }
  rcFUList.clear();
  return Err::m_nOK;
}


__inline ErrVal FrameMng::xRemove( FrameUnit* pcFrameUnit )
{
  pcFrameUnit->setUnused();

  if( pcFrameUnit->isOutputDone() )
  {
    RNOK( xAddToFreeList( pcFrameUnit ) );
    return Err::m_nOK;
  }

  m_cNonRefList.push_back( pcFrameUnit );
  return Err::m_nOK;
}


__inline ErrVal FrameMng::xRemoveFromRefList( FUList& rcFUList, FUIter iter )
{
  ROTRS( iter == rcFUList.end() ,Err::m_nOK );
  xRemove( *iter );
  rcFUList.erase( iter );
  return Err::m_nOK;
}


__inline ErrVal FrameMng::xRemoveFromRefList( FUList& rcFUList )
{
  for( FUIter iter = rcFUList.begin(); iter != rcFUList.end(); iter++ )
  {
    RNOK( xRemove( *iter ) );
  }
  rcFUList.clear();
  return Err::m_nOK;
}


ErrVal FrameMng::init( YuvBufferCtrl* pcYuvFullPelBufferCtrl, YuvBufferCtrl* pcYuvHalfPelBufferCtrl, QuarterPelFilter* pcQuarterPelFilter )
{

  ROT( m_bInitDone )

  m_uiPrecedingRefFrameNum  = 0;
  m_iEntriesInDPB           = 0;
  m_iMaxEntriesinDPB        = 0;
  m_uiNumRefFrames          = 0;
  m_uiMaxFrameNumCurr       = 0;
  m_uiMaxFrameNumPrev       = 0;
  m_bInitDone               = false;
  m_pcQuarterPelFilter      = NULL;
  m_pcOriginalFrameUnit     = NULL;
  m_pcCurrentFrameUnit      = NULL;
  m_pcCurrentFrameUnitBase  = NULL; //JVT-S036 lsj


  m_pcQuarterPelFilter = pcQuarterPelFilter;

  RNOK( m_cFrameUnitBuffer.init( pcYuvFullPelBufferCtrl, pcYuvHalfPelBufferCtrl ) );
  RNOK( FrameUnit::create( m_pcOriginalFrameUnit, *pcYuvFullPelBufferCtrl, *pcYuvFullPelBufferCtrl, true ) );

  if( m_pcRefinementIntFrame == 0)
  {
    ROF( m_pcRefinementIntFrame = new IntFrame( *pcYuvFullPelBufferCtrl, *pcYuvFullPelBufferCtrl ) );
  }
  if( m_pcRefinementIntFrameSpatial == 0)
  {
    ROF( m_pcRefinementIntFrameSpatial = new IntFrame( *pcYuvFullPelBufferCtrl, *pcYuvFullPelBufferCtrl ) );
  }
  if( m_pcPredictionIntFrame == 0)
  {
    ROF( m_pcPredictionIntFrame = new IntFrame( *pcYuvFullPelBufferCtrl, *pcYuvFullPelBufferCtrl ) );
  }

  m_bInitDone = true;

  return Err::m_nOK;
}


ErrVal FrameMng::uninit()
{
  if( NULL != m_pcRefinementIntFrame )
  {
    m_pcRefinementIntFrame->uninit();
    delete m_pcRefinementIntFrame;
    m_pcRefinementIntFrame = NULL;
  }

  if( NULL != m_pcRefinementIntFrameSpatial )
  {
    m_pcRefinementIntFrameSpatial->uninit();
    delete m_pcRefinementIntFrameSpatial;
    m_pcRefinementIntFrameSpatial = NULL;
  }

  if( NULL != m_pcPredictionIntFrame )
  {
    m_pcPredictionIntFrame->uninit();
    delete m_pcPredictionIntFrame;
    m_pcPredictionIntFrame = NULL;
  }

  if( NULL != m_pcOriginalFrameUnit )
  {
    RNOK( m_pcOriginalFrameUnit->uninit() );
    RNOK( m_pcOriginalFrameUnit->destroy() );
    m_pcOriginalFrameUnit = NULL;
  }

  RNOK( m_cFrameUnitBuffer.uninit() );

  m_uiPrecedingRefFrameNum  = 0;
  m_iEntriesInDPB           = 0;
  m_iMaxEntriesinDPB        = 0;
  m_uiNumRefFrames          = 0;
  m_uiMaxFrameNumCurr       = 0;
  m_uiMaxFrameNumPrev       = 0;
  m_bInitDone               = false;
  m_pcQuarterPelFilter      = NULL;
  m_pcOriginalFrameUnit     = NULL;
  m_pcCurrentFrameUnit      = NULL;
  m_pcCurrentFrameUnitBase  = NULL; //JVT-S036 lsj


  AOT( ! m_cShortTermList.empty() );
  AOT( ! m_cNonRefList.empty() );
  AOT( ! m_cOrderedPOCList.empty() );
  AOT( ! m_cPicBufferUnusedList.empty() );
  AOT( ! m_cPicBufferOutputList.empty() );

  m_cShortTermList.clear();
  m_cNonRefList.clear();
  m_cOrderedPOCList.clear();
  m_cPicBufferUnusedList.clear();
  m_cPicBufferOutputList.clear();

  return Err::m_nOK;
}

//JVT-S036 lsj{
ErrVal FrameMng::RefreshOrederedPOCList()
{
  //===== store reference in ordered POC List =====
  FUIter  iter;
  for( iter = m_cOrderedPOCList.begin(); iter != m_cOrderedPOCList.end(); iter++ )
  {
    if( (*iter)->getMaxPOC() == m_pcCurrentFrameUnit->getMaxPOC() )
    {
      break;
    }
  }

  (*iter) = m_pcCurrentFrameUnit;

	return Err::m_nOK;
}
//JVT-S036 lsj}


ErrVal FrameMng::initSlice( SliceHeader *rcSH )
{
  m_uiMaxFrameNumCurr = ( 1 << ( rcSH->getSPS().getLog2MaxFrameNum() ) );
  m_uiMaxFrameNumPrev = ( 1 << ( rcSH->getSPS().getLog2MaxFrameNum() ) );
  m_uiNumRefFrames    = rcSH->getSPS().getNumRefFrames();
  if( rcSH->getSPS().getProfileIdc() == MULTI_VIEW_PROFILE )
  {
    m_iMaxEntriesinDPB= rcSH->getSPS().getMaxDPBSize();
  }
  else
  {
    m_iMaxEntriesinDPB= min( 48, rcSH->getSPS().getMaxDPBSize() + 3 );
  }

  if( ! m_iMaxEntriesinDPB )
  {
    printf("WARNING: Size of Decoded picture buffer is less than 1 frame!");
  }

  if( m_pcRefinementIntFrame )
  {
    RNOK( m_pcRefinementIntFrame->init( false ) );
  }
  if( m_pcRefinementIntFrameSpatial )
  {
    RNOK( m_pcRefinementIntFrameSpatial->init( false ) );
  }
  if( m_pcPredictionIntFrame )
  {
    RNOK( m_pcPredictionIntFrame->init( false ) );
  }

  return Err::m_nOK;
}

ErrVal FrameMng::initSPS( const SequenceParameterSet& rcSPS )
{
  m_uiMaxFrameNumCurr = ( 1 << ( rcSPS.getLog2MaxFrameNum() ) );
  m_uiMaxFrameNumPrev = ( 1 << ( rcSPS.getLog2MaxFrameNum() ) );
  m_uiNumRefFrames    = rcSPS.getNumRefFrames();
  if( rcSPS.getProfileIdc() == MULTI_VIEW_PROFILE )
  {
    m_iMaxEntriesinDPB= rcSPS.getMaxDPBSize();
  }
  else
  {
    m_iMaxEntriesinDPB= min( 48, rcSPS.getMaxDPBSize() + 3 );
  }

  if( ! m_iMaxEntriesinDPB )
  {
    printf("WARNING: Size of Decoded picture buffer is less than 1 frame!");
  }

  if( m_pcRefinementIntFrame )
  {
    RNOK( m_pcRefinementIntFrame->init( false ) );
  }
  if( m_pcRefinementIntFrameSpatial )
  {
    RNOK( m_pcRefinementIntFrameSpatial->init( false ) );
  }
  if( m_pcPredictionIntFrame )
  {
    RNOK( m_pcPredictionIntFrame->init( false ) );
  }

  return Err::m_nOK;
}


ErrVal FrameMng::initPic( SliceHeader& rcSH )
{
  rcSH.setFrameUnit( m_pcCurrentFrameUnit );

  m_pcCurrentFrameUnit->setPoc( rcSH.getPoc() );

  return Err::m_nOK;
}


ErrVal FrameMng::initFrame( SliceHeader& rcSH, PicBuffer* pcPicBuffer )
{
  ROF( m_bInitDone );

  //===== check frame numbers for reference pictures =====
  if( ! rcSH.isIdrNalUnit() )
  {
    RNOK( xCheckMissingFrameNums( rcSH ) );
  }

  RNOK( m_cFrameUnitBuffer.getFrameUnit( m_pcCurrentFrameUnit ) );
  RNOK( m_pcCurrentFrameUnit->init( rcSH, pcPicBuffer ) );

  rcSH.setFrameUnit( m_pcCurrentFrameUnit );

  return Err::m_nOK;
}



ErrVal FrameMng::xCheckMissingFrameNums( SliceHeader& rcSH )
{
  //===== check frame numbers for reference pictures =====
  if( ( ( m_uiPrecedingRefFrameNum + 1 ) % m_uiMaxFrameNumCurr) != rcSH.getFrameNum() )
  {
    UInt  uiNumMissingPictures = rcSH.getFrameNum() - m_uiPrecedingRefFrameNum - 1;
    if( rcSH.getFrameNum() <= m_uiPrecedingRefFrameNum )
    {
      uiNumMissingPictures += m_uiMaxFrameNumCurr;
    }

    if( rcSH.getSPS().getRequiredFrameNumUpdateBehaviourFlag() )
    {
      for( UInt uiIndex = 1; uiIndex <= uiNumMissingPictures; uiIndex++ )
      {
        UInt        uiFrameNum  = ( m_uiPrecedingRefFrameNum + uiIndex ) % m_uiMaxFrameNumCurr;
        FrameUnit*  pcFrameUnit = 0;
        RNOK( m_cFrameUnitBuffer.getFrameUnit( pcFrameUnit ) );
		//JVT-S036 lsj{
		if( !m_pcCurrentFrameUnit->getBaseRep() )
		{
			FUList::iterator iter = m_cShortTermList.begin();
			FUList::iterator end  = m_cShortTermList.end();
			Bool bFlag = false;
			for( ; iter != m_cShortTermList.end(); iter++ )
			{
				if( (*iter)->getBaseRep() && (*iter)->getFrameNumber() == m_pcCurrentFrameUnit->getFrameNumber())
				{
				    bFlag = true;
					break;
				}
			}
			if( bFlag )
			{
					FrameUnit* pcFrameUnitTemp = (*iter);
					RNOK(pcFrameUnit->init( rcSH, *pcFrameUnitTemp ));
			}
			else
			{
				RNOK( pcFrameUnit->init( rcSH, *m_pcCurrentFrameUnit ) );
			}
			
		}
		else
		//JVT-S036 lsj}
		{
			RNOK( pcFrameUnit->init( rcSH, *m_pcCurrentFrameUnit ) ); // HS: decoder robustness
		}

        pcFrameUnit->setFrameNumber( uiFrameNum );
        m_cShortTermList.push_front( pcFrameUnit );
        m_iEntriesInDPB++;

		RNOK( xSlidingWindowUpdate() );

      }
    }
    else
    {
      printf("\n  LOST PICTURES: %d\n", uiNumMissingPictures );
      AF();
    }

    m_uiPrecedingRefFrameNum = ( m_uiPrecedingRefFrameNum + uiNumMissingPictures ) % m_uiMaxFrameNumCurr;
  }
  return Err::m_nOK;
}



ErrVal FrameMng::setPicBufferLists( PicBufferList& rcPicBufferOutputList, PicBufferList& rcPicBufferUnusedList )
{
  rcPicBufferUnusedList += m_cPicBufferUnusedList;
  m_cPicBufferUnusedList.clear();

  rcPicBufferOutputList += m_cPicBufferOutputList;
  m_cPicBufferOutputList.clear();

  return Err::m_nOK;
}


ErrVal FrameMng::storePicture( const SliceHeader& rcSH )
{
  //===== memory managment =====
  if( rcSH.isIdrNalUnit() )
  {
    RNOK( xClearListsIDR( rcSH ) );
  }
  RNOK( xManageMemory( rcSH ) );

  //===== store current picture =====
  RNOK( xStoreCurrentPicture( rcSH ) );

  //===== set pictures for output =====
  RNOK( xSetOutputList( m_pcCurrentFrameUnit ) );

  if( rcSH.getNalRefIdc() )
  {
    m_uiPrecedingRefFrameNum = m_pcCurrentFrameUnit->getFrameNumber();
  }

  return Err::m_nOK;
}



UInt FrameMng::xSortPocOrderedList()
{
  UInt  uiFirstPosWithGreaterPoc;
  Int   iCurrPoc = m_pcCurrentFrameUnit->getFrame().getPOC();

  std::sort( m_cPocOrderedFrameList.begin(), m_cPocOrderedFrameList.end(), PocOrder() );
  for( uiFirstPosWithGreaterPoc = 0; uiFirstPosWithGreaterPoc < m_cPocOrderedFrameList.size(); uiFirstPosWithGreaterPoc++ )
  {
    if( m_cPocOrderedFrameList.get( uiFirstPosWithGreaterPoc )->getPOC() > iCurrPoc )
    {
      break;
    }
  }
  return uiFirstPosWithGreaterPoc;
}


ErrVal FrameMng::xSetInitialReferenceListPFrame( SliceHeader& rcSH )
{
  RefPicList<RefPic>& rcList = rcSH.getRefPicList( LIST_0 );

  if( ! rcSH.getUseBasePredictionFlag() ) 
    m_cShortTermList.setRefPicListFGS( rcList );
  else
    m_cShortTermList.setRefPicList( rcList );
  return Err::m_nOK;
}


ErrVal FrameMng::xSetInitialReferenceListBFrame( SliceHeader& rcSH )
{
  RefPicList<RefPic>& rcList0 = rcSH.getRefPicList( LIST_0 );
  RefPicList<RefPic>& rcList1 = rcSH.getRefPicList( LIST_1 );
  UInt                uiFirstPosWithGreaterPoc, uiPos;
  FUIter              iter;

  //====== set Poc ordered short-term list and get index with smallest Poc greater than current ======
  m_cPocOrderedFrameList.reset();
  if( ! rcSH.getUseBasePredictionFlag() ) 
    m_cShortTermList.setRefFrameListFGS( m_cPocOrderedFrameList );
  else
    m_cShortTermList.setRefFrameList( m_cPocOrderedFrameList );
  uiFirstPosWithGreaterPoc = xSortPocOrderedList();

  //===== set short term reference frames =====
  for( uiPos = uiFirstPosWithGreaterPoc - 1; uiPos != MSYS_UINT_MAX; uiPos-- )
  {
    rcList0.next().setFrame( m_cPocOrderedFrameList.get( uiPos ) );
  }
  for( uiPos = uiFirstPosWithGreaterPoc; uiPos != m_cPocOrderedFrameList.size(); uiPos++ )
  {
    rcList0.next().setFrame( m_cPocOrderedFrameList.get( uiPos ) );
    rcList1.next().setFrame( m_cPocOrderedFrameList.get( uiPos ) );
  }
  for( uiPos = uiFirstPosWithGreaterPoc - 1; uiPos != MSYS_UINT_MAX; uiPos-- )
  {
    rcList1.next().setFrame( m_cPocOrderedFrameList.get( uiPos ) );
  }

  return Err::m_nOK;
}



ErrVal FrameMng::xCheckReferenceListSwitching( SliceHeader& rcSH )
{
  RefPicList<RefPic>& rcList0 = rcSH.getRefPicList( LIST_0 );
  RefPicList<RefPic>& rcList1 = rcSH.getRefPicList( LIST_1 );

  ROTRS( rcList1.size() < 2 || rcList0.size() != rcList1.size(), Err::m_nOK );

  for( UInt uiPos = 0; uiPos < rcList1.size(); uiPos++ )
  {
    if( rcList1.get( uiPos ) != rcList0.get( uiPos ) )
    {
      return Err::m_nOK;
    }
  }
  rcList1.switchFirstEntries();
  return Err::m_nOK;
}



ErrVal FrameMng::setRefPicLists( SliceHeader& rcSH, Bool bDoNotRemap )
{
  RNOK( xSetReferenceLists( rcSH) );

  if( ! bDoNotRemap )
  {
    //===== remapping =====
    RNOK( xReferenceListRemapping( rcSH, LIST_0 ) );
    RNOK( xReferenceListRemapping( rcSH, LIST_1 ) );
  }

  return Err::m_nOK;
}


ErrVal FrameMng::xSetReferenceLists( SliceHeader& rcSH )
{
  rcSH.getRefPicList( LIST_0 ).reset( 0 );
  rcSH.getRefPicList( LIST_1 ).reset( 0 );

  if( rcSH.isIntra() )
  {
    return Err::m_nOK;
  }

  rcSH.getRefPicList( LIST_0 ).reset();
  if( rcSH.isInterB() )
  {
    rcSH.getRefPicList( LIST_1 ).reset();
  }

  //===== initial lists =====
  if( ! rcSH.isInterB() )
  {
    RNOK( xSetInitialReferenceListPFrame( rcSH ) );
  }
  else
  {
    RNOK( xSetInitialReferenceListBFrame( rcSH ) );
    RNOK( xCheckReferenceListSwitching( rcSH ) )
  }

  return Err::m_nOK;
}



ErrVal FrameMng::xClearListsIDR( const SliceHeader& rcSH  )
{
  //===== output =====
  for( FUIter iter = m_cOrderedPOCList.begin(); iter != m_cOrderedPOCList.end(); iter++ )
  {
    if( ! rcSH.getNoOutputOfPriorPicsFlag() )
    {
      if( (*iter)->getFGSPicBuffer() )
      {
        (*iter)->getFGSPicBuffer()->setCts( (UInt64)((*iter)->getMaxPOC()) ); // HS: decoder robustness
        m_cPicBufferOutputList.push_back( (*iter)->getFGSPicBuffer() );
      }
      else if ((*iter)->getPicBuffer() )  //JVT-S036 lsj
      {
        (*iter)->getPicBuffer()->setCts( (UInt64)((*iter)->getMaxPOC()) ); // HS: decoder robustness
        m_cPicBufferOutputList.push_back( (*iter)->getPicBuffer() );
      }
    }


    (*iter)->setOutputDone();

    if( xFindAndErase( m_cNonRefList, *iter ) )
    {
       RNOK( xAddToFreeList( (*iter) ) );
    }
  }
  m_cOrderedPOCList.clear();
  return Err::m_nOK;
}


UInt FrameMng::MaxRefFrames( UInt uiLevel, UInt uiNumMbs )
{
  return m_uiDBPMemory[ uiLevel ] / ( 384 * uiNumMbs );
}



ErrVal FrameMng::outputAll()
{
  FUIter  iter;
  //===== output =====
  for( iter = m_cOrderedPOCList.begin(); iter != m_cOrderedPOCList.end(); iter++ )
  {
    if( (*iter)->getFGSPicBuffer() )
    {
      (*iter)->getFGSPicBuffer()->setCts( (UInt64)((*iter)->getMaxPOC()) ); // HS: decoder robustness
      m_cPicBufferOutputList.push_back( (*iter)->getFGSPicBuffer() );
    }
    else if ((*iter)->getPicBuffer() )  //JVT-S036 lsj
    {
      (*iter)->getPicBuffer()->setCts( (UInt64)((*iter)->getMaxPOC()) ); // HS: decoder robustness
      m_cPicBufferOutputList.push_back( (*iter)->getPicBuffer() );
    }
    (*iter)->setOutputDone();
  }
  m_cOrderedPOCList.erase( m_cOrderedPOCList.begin(), iter );

  RNOK( xAddToFreeList( m_cShortTermList ) );
  RNOK( xAddToFreeList( m_cNonRefList ) );

  return Err::m_nOK;
}



ErrVal FrameMng::xSetOutputList( FrameUnit* pcFrameUnit )
{
  ROTRS( m_iEntriesInDPB <= m_iMaxEntriesinDPB, Err::m_nOK );

  //===== get minimum POC for output =====
  Int     iMinPOCtoOuput = MSYS_INT_MAX;
  FUIter  iter;
  for( iter = m_cNonRefList.begin(); iter != m_cNonRefList.end(); iter++ )
  {
    if( (*iter)->getMaxPOC() < iMinPOCtoOuput && ( (*iter) != pcFrameUnit) )
    {
      iMinPOCtoOuput = (*iter)->getMaxPOC();
    }
  }
  ROT( iMinPOCtoOuput == MSYS_INT_MAX );

  //===== output =====
  for( iter = m_cOrderedPOCList.begin(); iter != m_cOrderedPOCList.end(); iter++ )
  {
    if( (*iter)->getMaxPOC() <= iMinPOCtoOuput )
    {
      if( (*iter)->getFGSPicBuffer() )
      {
        (*iter)->getFGSPicBuffer()->setCts( (UInt64)((*iter)->getMaxPOC()) ); // HS: decoder robustness
        m_cPicBufferOutputList.push_back( (*iter)->getFGSPicBuffer() );
      }
      else if ((*iter)->getPicBuffer() )  //JVT-S036 lsj
      {
        (*iter)->getPicBuffer()->setCts( (UInt64)((*iter)->getMaxPOC()) ); // HS: decoder robustness
        m_cPicBufferOutputList.push_back( (*iter)->getPicBuffer() );
      }
      (*iter)->setOutputDone();
      if( xFindAndErase( m_cNonRefList, *iter ) )
      {
        RNOK( xAddToFreeList( *iter ) );
      }
    }
    else
    {
      break;
    }
  }
  m_cOrderedPOCList.erase( m_cOrderedPOCList.begin(), iter );

  return Err::m_nOK;
}


ErrVal FrameMng::storeFGSPicture( PicBuffer* pcPicBuffer )
{
  UInt uiFGSReconCount = m_pcCurrentFrameUnit->getFGSReconCount();
  m_pcCurrentFrameUnit->getFGSReconstruction(uiFGSReconCount)->copyAll(m_pcCurrentFrameUnit->getFGSIntFrame());
  m_pcCurrentFrameUnit->setFGSReconCount(uiFGSReconCount + 1);

  m_pcCurrentFrameUnit->setFGS( pcPicBuffer );
  m_pcCurrentFrameUnit->getFGSIntFrame()->store( pcPicBuffer );

  m_pcCurrentFrameUnit->getFGSFrame().extendFrame( m_pcQuarterPelFilter );
  
  return Err::m_nOK;
}


ErrVal FrameMng::xStoreCurrentPicture( const SliceHeader& rcSH )
{
  Frame& cBaseFrame = m_pcCurrentFrameUnit->getFrame();
  PicBuffer cTempPicBuffer(cBaseFrame.getFullPelYuvBuffer()->getBuffer());

  // Base layer
  m_pcCurrentFrameUnit->getFGSReconstruction(0)->load(& cTempPicBuffer);
  m_pcCurrentFrameUnit->setFGSReconCount(1);

  RNOK( m_pcCurrentFrameUnit->getFrame().extendFrame( m_pcQuarterPelFilter ) );

  if( rcSH.getNalRefIdc() )
  {
    //===== store as short term picture =====
    m_pcCurrentFrameUnit->setFrameNumber( rcSH.getFrameNum() );
    m_pcCurrentFrameUnit->setUsed       ();

    m_cShortTermList.push_front( m_pcCurrentFrameUnit );
    m_iEntriesInDPB++;

//JVT-S036 lsj start
	if( rcSH.getUseBasePredictionFlag() )  //bug-fix suffix shenqiu
	{
		RNOK( m_cFrameUnitBuffer.getFrameUnit( m_pcCurrentFrameUnitBase ) );
		RNOK( m_pcCurrentFrameUnitBase->copyBase( rcSH, *m_pcCurrentFrameUnit ) );
		m_pcCurrentFrameUnitBase->setBaseRep( true );
		m_cShortTermList.push_front( m_pcCurrentFrameUnitBase );
		m_iEntriesInDPB++; 
	}
//JVT-S036 lsj end
  }
  else
  {

    m_cNonRefList.push_back( m_pcCurrentFrameUnit );
    m_iEntriesInDPB++;
  }

  //===== store reference in ordered POC List =====
  FUIter  iter;
  for( iter = m_cOrderedPOCList.begin(); iter != m_cOrderedPOCList.end(); iter++ )
  {
    if( (*iter)->getMaxPOC() > m_pcCurrentFrameUnit->getMaxPOC() )
    {
      break;
    }
  }

  if( rcSH.getUseBasePredictionFlag() )  //bug-fix suffix shenqiu
  {
	m_cOrderedPOCList.insert( iter, m_pcCurrentFrameUnitBase );
  }
  else
  {
	  m_cOrderedPOCList.insert( iter, m_pcCurrentFrameUnit );
  }

  return Err::m_nOK;
}



ErrVal FrameMng::xMmcoMarkShortTermAsUnused( const FrameUnit* pcCurrFrameUnit, UInt uiDiffOfPicNums )
{
  UInt  uiCurrPicNum  = pcCurrFrameUnit->getFrameNumber();
  UInt  uiPicNumN     = uiCurrPicNum - uiDiffOfPicNums - 1;

  if( uiCurrPicNum <= uiDiffOfPicNums )
  {
    uiPicNumN += m_uiMaxFrameNumPrev;
  }

  FUIter iter = m_cShortTermList.findShortTerm( uiPicNumN );
  if( iter == m_cShortTermList.end() )
  {
    printf("\nMMCO not possible\n" );
    return Err::m_nOK; // HS: decoder robustness
  }

  FrameUnit* pcFrameUnit = (*iter);
  pcFrameUnit->setUnused();
  RNOK( xRemoveFromRefList( m_cShortTermList, iter ) );

  return Err::m_nOK;
}

//JVT-S036 lsj start
ErrVal FrameMng::xMmcoMarkShortTermAsUnusedBase( const FrameUnit* pcCurrFrameUnit, UInt uiDiffOfPicNums )
{
  UInt  uiCurrPicNum  = pcCurrFrameUnit->getFrameNumber();
  UInt  uiPicNumN     = uiCurrPicNum - uiDiffOfPicNums - 1;

  if( uiCurrPicNum <= uiDiffOfPicNums )
  {
    uiPicNumN += m_uiMaxFrameNumPrev;
  }

  FUIter iter = m_cShortTermList.findShortTerm( uiPicNumN );
  if( iter == m_cShortTermList.end() )
  {
    printf("\nMMCO not possible\n" );
    return Err::m_nOK; // HS: decoder robustness
  }

  FrameUnit* pcFrameUnit = (*iter);
  if(pcFrameUnit->getBaseRep() )
  {
	  pcFrameUnit->setUnused();
	  RNOK( xRemoveFromRefList( m_cShortTermList, iter ) );
  }
  
  return Err::m_nOK;
}
//JVT-S036 lsj end

ErrVal FrameMng::xManageMemory( const SliceHeader& rcSH )
{
  ROTRS( ! rcSH.getNalRefIdc(), Err::m_nOK );

  if( ! rcSH.getAdaptiveRefPicBufferingFlag() )
  {
    RNOK( xSlidingWindowUpdate() );
    return Err::m_nOK;
  }


  MmcoOp eMmcoOp;
  const MmcoBuffer& rcMmcoBuffer = rcSH.getMmcoBuffer();
  UInt uiVal1, uiVal2;
  Int iIndex = 0;
  while( MMCO_END != (eMmcoOp = rcMmcoBuffer.get( iIndex++ ).getCommand( uiVal1, uiVal2)) )
  {

    switch (eMmcoOp)
    {
    case MMCO_SHORT_TERM_UNUSED:
      RNOK( xMmcoMarkShortTermAsUnused( m_pcCurrentFrameUnit, uiVal1 ) );
      break;
    case MMCO_RESET:
    case MMCO_MAX_LONG_TERM_IDX:
    case MMCO_ASSIGN_LONG_TERM:
    case MMCO_LONG_TERM_UNUSED:
    case MMCO_SET_LONG_TERM:
    default:AF();
      break;
    }
    
  }


  return Err::m_nOK;
}



ErrVal FrameMng::xSlidingWindowUpdate()
{
  UInt  uiS = m_cShortTermList.size();

  //bug-fix suffix{{
  FUList::iterator iter = m_cShortTermList.begin();
  for( ; iter != m_cShortTermList.end(); iter++ )
   {
	   if( (*iter)->getBaseRep() ) uiS--;
  }
  //bug-fix suffix}}

  //ROT( uiS > m_uiNumRefFrames );
  //if( uiS == m_uiNumRefFrames )
  while( uiS >= m_uiNumRefFrames ) // HS: decoder robustness
  {  
	  RNOK( xRemove( m_cShortTermList.popBack() ) );
	uiS--; // HS: decoder robustness
  }
  return Err::m_nOK;
}

//JVT-S036 lsj start
ErrVal FrameMng::xSlidingWindowUpdateBase( UInt mCurrFrameNum )  
{
	FUList::iterator iter = m_cShortTermList.begin();
	FUList::iterator end  = m_cShortTermList.end();
	FUList::iterator iiter;

	for( ; iter != m_cShortTermList.end(); iter++ )
   {
	   if( (*iter)->getBaseRep() && (*iter)->getFrameNumber() != mCurrFrameNum )
    {
		for( iiter = m_cShortTermList.begin(); iiter != m_cShortTermList.end(); iiter++ )
		{
			if ( (*iiter)->getFrameNumber() == (*iter)->getFrameNumber() && !(*iiter)->getBaseRep() )
			{
				(*iter)->setUnused();
				RNOK( xRemoveFromRefList( m_cShortTermList, iter ) );
				return Err::m_nOK;
			}
		}
    }
  }
   return Err::m_nOK;
}

ErrVal FrameMng::xMMCOUpdateBase( SliceHeader* rcSH )
{

  MmcoOp            eMmcoOp;
  const MmcoBuffer& rcMmcoBaseBuffer = rcSH->getMmcoBaseBuffer();
  Int               iIndex        = 0;
  UInt              uiVal1, uiVal2;

  while( MMCO_END != (eMmcoOp = rcMmcoBaseBuffer.get( iIndex++ ).getCommand( uiVal1, uiVal2 ) ) )
 {
		switch( eMmcoOp )
		{
		case MMCO_SHORT_TERM_UNUSED:
			RNOK( xMmcoMarkShortTermAsUnusedBase( m_pcCurrentFrameUnit, uiVal1 ) );
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
//JVT-S036 lsj end

ErrVal FrameMng::getRecYuvBuffer( YuvPicBuffer*& rpcRecYuvBuffer )
{
  ROT( NULL == m_pcCurrentFrameUnit );
  rpcRecYuvBuffer = m_pcCurrentFrameUnit->getFrame().getFullPelYuvBuffer();
  return Err::m_nOK;
}




FrameUnit*
FrameMng::getReconstructedFrameUnit( Int iPoc )
{
  for( FUIter iter = m_cOrderedPOCList.begin(); iter != m_cOrderedPOCList.end(); iter++ )
  {
    if( (*iter)->getMaxPOC() == iPoc )
    {
      return *iter;
    }
  }
  return 0;
}




ErrVal FrameMng::xReferenceListRemapping( SliceHeader& rcSH, ListIdx eListIdx )
{
  RefPicList<RefPic>& rcList = rcSH.getRefPicList( eListIdx );
  ROTRS( 0 == rcList.bufSize(), Err::m_nOK );

  const RplrBuffer& rcRplrBuffer = rcSH.getRplrBuffer( eListIdx );

  ROTRS( ! rcRplrBuffer.getRefPicListReorderingFlag(), Err::m_nOK );

  UInt      uiPicNumPred  = rcSH.getFrameNum();
  UInt      uiMaxPicNum   = m_uiMaxFrameNumPrev;
  UInt      uiIndex       = 0;
  UInt      uiCommand;
  UInt      uiIdentifier;

  while( RPLR_END != ( uiCommand = rcRplrBuffer.get( uiIndex ).getCommand( uiIdentifier ) ) )
  {
    FUIter    iter;
    const Frame* pcFrame = NULL;
    
    if( uiCommand == RPLR_LONG )
    //===== LONG TERM INDEX =====
    {
      AF();
    }
    else
    //===== SHORT TERM INDEX =====
    {
      UInt uiAbsDiff = uiIdentifier + 1;
      //---- set short term index ----
      if( uiCommand == RPLR_NEG )
      {
        if( uiPicNumPred < uiAbsDiff )
        {
          uiPicNumPred -= ( uiAbsDiff - uiMaxPicNum );
        }
        else
        {
          uiPicNumPred -= uiAbsDiff;
        }
      }
      else
      {
        if( uiPicNumPred + uiAbsDiff > uiMaxPicNum - 1 )
        {
          uiPicNumPred += ( uiAbsDiff - uiMaxPicNum );
        }
        else
        {
          uiPicNumPred += uiAbsDiff;
        }
      }
      uiIdentifier = uiPicNumPred;
      //---- search for short term picture ----
      iter = m_cShortTermList.findShortTerm( uiIdentifier );

      //---- check ----
      if( iter == m_cShortTermList.end() )
      {
        return Err::m_nDataNotAvailable;
      }
      else
      { // everything is fine
        //---- set frame ----
        if( ! rcSH.getUseBasePredictionFlag() ) 
        {
			if((*iter)->getBaseRep()) //JVT-S036 lsj
			{
				iter++;
			}

			if( (*iter)->getFGSPicBuffer() )
			{
				pcFrame = &( (*iter)->getFGSFrame() );
			}
			else
			{
				pcFrame = &( (*iter)->getFrame() );
			}
			}
			else
			{
				pcFrame = &((*iter)->getFrame() );
			}
      }
    }

    //---- find picture in reference list -----
    UInt uiRemoveIndex = MSYS_UINT_MAX;
    if( NULL != pcFrame )
    {
      for( UInt uiPos = uiIndex; uiPos < rcList.size(); uiPos++ )
      {
        if( rcList.get( uiPos ).getFrame() == pcFrame )
        {
          uiRemoveIndex = uiPos;
          break;
        }
      }
    }

    //----- reference list reordering -----
    rcList.getElementAndRemove( uiIndex, uiRemoveIndex ).setFrame( pcFrame );
    uiIndex++;
  }

  return Err::m_nOK;
}


H264AVC_NAMESPACE_END

