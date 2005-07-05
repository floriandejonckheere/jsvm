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
, m_bResidual           ( false )
, m_bExisting           ( false )
, m_bNeededForReference ( false )
, m_bOutputted          ( false )
, m_bBaseRepresentation ( false )
, m_pcFrame             ( 0 )
, m_cControlData        ()
{
}

DPBUnit::~DPBUnit()
{
  MbDataCtrl*   pcMbDataCtrl  = m_cControlData.getMbDataCtrl  ();
  SliceHeader*  pcSliceHeader = m_cControlData.getSliceHeader ();
  if( pcMbDataCtrl )
  {
    pcMbDataCtrl->uninit();
  }
  delete pcMbDataCtrl;
  delete pcSliceHeader;
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
               Bool bResidual,
               Bool bNeededForReference )
{
  m_iPoc                = iPoc;
  m_uiFrameNum          = uiFrameNum;
  m_uiTemporalLevel     = uiTemporalLevel;
  m_bKeyPicture         = bKeyPicture;
  m_bResidual           = bResidual;
  m_bExisting           = true;
  m_bNeededForReference = bNeededForReference;
  m_bOutputted          = false;
  m_bBaseRepresentation = false;
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
  m_bResidual           = false;
  m_bExisting           = false;
  m_bNeededForReference = true;
  m_bOutputted          = false;
  m_bBaseRepresentation = false;
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
  m_bResidual           = rcDPBUnit.m_bResidual;
  m_bExisting           = rcDPBUnit.m_bExisting;
  m_bNeededForReference = rcDPBUnit.m_bNeededForReference;
  m_bOutputted          = rcDPBUnit.m_bOutputted;
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
  m_bResidual           = false;
  m_bExisting           = false;
  m_bNeededForReference = false;
  m_bOutputted          = false;
  m_bBaseRepresentation = false;
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

ErrVal
DPBUnit::unmarkResidual()
{
  ROF( m_bResidual );
  m_bResidual = false;
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
, m_iMaxKeyPoc        ( MSYS_INT_MIN )
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
  m_iMaxKeyPoc        = MSYS_INT_MIN;
  m_uiLayer           = uiLayer;
  m_bInitDone         = true;
  
  return Err::m_nOK;
}

__inline UInt downround2powerof2( UInt i ) { UInt r = 1; for( ; ( 1 << r ) <= i; r++ ); return ( 1 << ( r - 1 ) ); }

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
  m_iMaxKeyPoc        = MSYS_INT_MIN;
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
DecodedPicBuffer::xUpdateMemory()
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

  //===== clear buffer =====
  RNOK( xClearBuffer() );

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
    RNOK( xUpdateMemory() );                                                      // memory update 
    RNOK( xOutput( rcOutputList, rcUnusedList ) );         // output
  }
  RNOK( xDumpDPB() );

  m_pcCurrDPBUnit = m_cFreeDPBUnitList.popFront();                                // new current DPB unit
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
    if(  p->isResidual () )   printf("Residual   ");
    printf("\n");
  }
  printf("\n");
  return Err::m_nOK;
}


ErrVal
DecodedPicBuffer::xInitPrdList0( RefFrameList&  rcList )
{
  Bool  bBaseRep    = m_pcCurrDPBUnit->isKeyPic ();
  Int   iMaxPoc     = m_pcCurrDPBUnit->getPoc   ();
  UInt  uiTempLevel = m_pcCurrDPBUnit->getTLevel();
  UInt  uiSize      = m_pcCurrDPBUnit->getCtrlData().getSliceHeader()->getNumRefIdxActive( LIST_0 );

  while( uiSize-- )
  {
    DPBUnit*              pNext = 0;
    DPBUnitList::iterator iter  = m_cUsedDPBUnitList.begin();
    DPBUnitList::iterator end   = m_cUsedDPBUnitList.end  ();
    for( ; iter != end; iter++ )
    {
      if( (*iter)->getPoc() < iMaxPoc &&
          ( !pNext || (*iter)->getPoc() > pNext->getPoc() ) &&
          (*iter)->isBaseRep() == bBaseRep &&
          ( bBaseRep || (*iter)->getTLevel() < uiTempLevel ) )
      {
        pNext = (*iter);
      }
    }
    ROF( pNext );

    iMaxPoc = pNext->getPoc();
    rcList.add( pNext->getFrame() );
  }

  return Err::m_nOK;
}


ErrVal
DecodedPicBuffer::xInitPrdList1( RefFrameList&  rcList )
{
  Bool  bBaseRep    = m_pcCurrDPBUnit->isKeyPic ();
  Int   iMinPoc     = m_pcCurrDPBUnit->getPoc   ();
  UInt  uiTempLevel = m_pcCurrDPBUnit->getTLevel();
  UInt  uiSize      = m_pcCurrDPBUnit->getCtrlData().getSliceHeader()->getNumRefIdxActive( LIST_1 );

  while( uiSize-- )
  {
    DPBUnit*              pNext = 0;
    DPBUnitList::iterator iter  = m_cUsedDPBUnitList.begin();
    DPBUnitList::iterator end   = m_cUsedDPBUnitList.end  ();
    for( ; iter != end; iter++ )
    {
      if( (*iter)->getPoc() > iMinPoc &&
          ( !pNext || (*iter)->getPoc() < pNext->getPoc() ) && 
          (*iter)->isBaseRep() == bBaseRep &&
          ( bBaseRep || (*iter)->getTLevel() < uiTempLevel ) )
      {
        pNext = (*iter);
      }
    }
    ROF( pNext );

    iMinPoc = pNext->getPoc();
    rcList.add( pNext->getFrame() );
  }

  return Err::m_nOK;
}


ErrVal
DecodedPicBuffer::xInitUpdList0( DPBUnit*       pcDPBUnit,
                                 RefFrameList&  rcList,
                                 CtrlDataList&  rcCtrl,
                                 UInt           uiUpdLevel )
{
  Bool  bBaseRep    = pcDPBUnit->isKeyPic ();
  Int   iMaxPoc     = pcDPBUnit->getPoc   ();
  UInt  uiTempLevel = pcDPBUnit->getTLevel();
  UInt  uiSize      = pcDPBUnit->getCtrlData().getSliceHeader()->getNumRefIdxUpdate( uiUpdLevel, LIST_0 );

  while( uiSize-- )
  {
    DPBUnit*              pNext = 0;
    DPBUnitList::iterator iter  = m_cUsedDPBUnitList.begin();
    DPBUnitList::iterator end   = m_cUsedDPBUnitList.end  ();
    for( ; iter != end; iter++ )
    {
      if( (*iter)->getPoc() < iMaxPoc &&
          ( !pNext || (*iter)->getPoc() > pNext->getPoc() ) &&
          (*iter)->isResidual() &&
          (*iter)->getTLevel() == uiUpdLevel + 1 )
      {
        pNext = (*iter);
      }
    }
    if( pNext )
    {
      iMaxPoc = pNext->getPoc();
      rcList.add( pNext->getFrame() );
      rcCtrl.add( &pNext->getCtrlData() );
    }
    else
    {
      break;
    }
  }

  return Err::m_nOK;
}


ErrVal
DecodedPicBuffer::xInitUpdList1( DPBUnit*       pcDPBUnit,
                                 RefFrameList&  rcList,
                                 CtrlDataList&  rcCtrl,
                                 UInt           uiUpdLevel )
{
  Bool  bBaseRep    = pcDPBUnit->isKeyPic ();
  Int   iMinPoc     = pcDPBUnit->getPoc   ();
  UInt  uiTempLevel = pcDPBUnit->getTLevel();
  UInt  uiSize      = pcDPBUnit->getCtrlData().getSliceHeader()->getNumRefIdxUpdate( uiUpdLevel, LIST_1 );

  while( uiSize-- )
  {
    DPBUnit*              pNext = 0;
    DPBUnitList::iterator iter  = m_cUsedDPBUnitList.begin();
    DPBUnitList::iterator end   = m_cUsedDPBUnitList.end  ();
    for( ; iter != end; iter++ )
    {
      if( (*iter)->getPoc() > iMinPoc &&
          ( !pNext || (*iter)->getPoc() < pNext->getPoc() ) &&
          (*iter)->isResidual() &&
          (*iter)->getTLevel() == uiUpdLevel + 1 )
      {
        pNext = (*iter);
      }
    }
    if( pNext )
    {
      iMinPoc = pNext->getPoc();
      rcList.add( pNext->getFrame() );
      rcCtrl.add( &pNext->getCtrlData() );
    }
    else
    {
      break;
    }
  }

  return Err::m_nOK;
}


ErrVal
DecodedPicBuffer::xDumpRefList( ListIdx       eListIdx,
                                RefFrameList& rcList )
{
#if 1 // NO_DEBUG
  return Err::m_nOK;
#endif

  printf("\nList %d =", eListIdx );
  for( UInt uiIndex = 1; uiIndex <= rcList.getSize(); uiIndex++ )
  {
    printf(" %d", rcList[uiIndex]->getPOC() );
  }
  printf("\n");
  return Err::m_nOK;
}


ErrVal
DecodedPicBuffer::xSetAllComplete( Int iMaxKeyPoc )
{
  DPBUnitList::iterator iter  = m_cUsedDPBUnitList.begin();
  DPBUnitList::iterator end   = m_cUsedDPBUnitList.end  ();
  for( ; iter != end; iter++ )
  {
    if( (*iter)->getPoc() <= iMaxKeyPoc )
    {
      (*iter)->getCtrlData().setComplete( true );
    }
  }
  return Err::m_nOK;
}


ErrVal
DecodedPicBuffer::initCurrDPBUnit( DPBUnit*&      rpcCurrDPBUnit,
                                   PicBuffer*&    rpcPicBuffer,
                                   Bool           bResidual,
                                   SliceHeader*   pcSliceHeader,
                                   PicBufferList& rcOutputList,
                                   PicBufferList& rcUnusedList )
{
  ROF( m_bInitDone );

  //===== insert pic buffer in list =====
  m_cPicBufferList.push_back( rpcPicBuffer );
  rpcPicBuffer = 0;

  //===== check missing pictures =====
  RNOK( xCheckMissingPics( pcSliceHeader, rcOutputList, rcUnusedList ) );

  //===== set key pictures to complete status =====
  if( pcSliceHeader->getPoc() > m_iMaxKeyPoc ) // HS: this is not the WD
  {
    // new GOP -> mark all pictures as complete, since they cannot be update anymore
    RNOK( xSetAllComplete( m_iMaxKeyPoc ) );
    if( pcSliceHeader->getKeyPictureFlag() )
    {
      m_iMaxKeyPoc = pcSliceHeader->getPoc();
    }
  }

  //===== initialize current DPB unit =====
  RNOK( m_pcCurrDPBUnit->init( pcSliceHeader->getPoc(),
                               pcSliceHeader->getFrameNum(),
                               pcSliceHeader->getTemporalLevel(),
                               pcSliceHeader->getKeyPictureFlag(),
                               bResidual,
                               pcSliceHeader->getNalRefIdc() > 0 ) );
  ROT( pcSliceHeader->getKeyPictureFlag() && !pcSliceHeader->getNalRefIdc() ); // just a check
  m_pcCurrDPBUnit->getFrame()->setPOC       ( pcSliceHeader->getPoc() );
  SliceHeader* pcOldSH = m_pcCurrDPBUnit->getCtrlData().getSliceHeader();
  delete pcOldSH;
  m_pcCurrDPBUnit->getCtrlData().clear();
  m_pcCurrDPBUnit->getCtrlData().setSliceHeader( pcSliceHeader );
  m_pcCurrDPBUnit->getCtrlData().getMbDataCtrl()->reset();
  m_pcCurrDPBUnit->getCtrlData().getMbDataCtrl()->clear();

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

  RNOK( xInitPrdList0( rcList0 ) );
  RNOK( xDumpRefList ( LIST_0, rcList0 ) );
  ROTRS( m_pcCurrDPBUnit->getCtrlData().getSliceHeader()->isInterP(),  Err::m_nOK );

  RNOK( xInitPrdList1( rcList1 ) );
  RNOK( xDumpRefList ( LIST_1, rcList1 ) );

  return Err::m_nOK;
}


ErrVal
DecodedPicBuffer::setUpdRefLists( DPBUnit* pcCurrDPBUnit,
                                  UInt     uiUpdLevel )
{
  ROF( pcCurrDPBUnit->getCtrlData().getSliceHeader() );

  RefFrameList& rcList0 = pcCurrDPBUnit->getCtrlData().getUpdFrameList( uiUpdLevel, LIST_0 );
  RefFrameList& rcList1 = pcCurrDPBUnit->getCtrlData().getUpdFrameList( uiUpdLevel, LIST_1 );
  CtrlDataList& rcCtrl0 = pcCurrDPBUnit->getCtrlData().getUpdCtrlList ( uiUpdLevel, LIST_0 );
  CtrlDataList& rcCtrl1 = pcCurrDPBUnit->getCtrlData().getUpdCtrlList ( uiUpdLevel, LIST_1 );

  rcList0.reset();
  rcList1.reset();
  rcCtrl0.reset();
  rcCtrl1.reset();
  RNOK( xInitUpdList0( pcCurrDPBUnit, rcList0, rcCtrl0, uiUpdLevel ) );
  RNOK( xInitUpdList1( pcCurrDPBUnit, rcList1, rcCtrl1, uiUpdLevel ) );
#if 0 // NO_DEBUG
  printf("\nUPDATE:");
#endif
  RNOK( xDumpRefList ( LIST_0, rcList0 ) );
  RNOK( xDumpRefList ( LIST_1, rcList1 ) );

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
DecodedPicBuffer::update( DPBUnit*  pcDPBUnit,
                          Bool      bUnmarkResidual )
{
  ROF( pcDPBUnit );

  //---- fill border ----
  RNOK( m_pcYuvBufferCtrl->initMb() );
  RNOK( pcDPBUnit->getFrame()->extendFrame( NULL ) );

  if( bUnmarkResidual )
  {
    RNOK( pcDPBUnit->unmarkResidual() );
  }
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
// *LMH: Inverse MCTF
, m_pcCurrSliceHeader             ( 0 )
, m_pcResidual                    ( 0 )
, m_pcPredSignal                  ( 0 )
, m_pcBaseLayerResidual           ( 0 )
, m_pcBaseLayerFrame              ( 0 )
, m_pcBaseLayerCtrl               ( 0 )
, m_pcCurrDPBUnit                 ( 0 )
, m_uiLayerId                     ( 0 )
, m_pusUpdateWeights              ( 0 )
, m_bActive                       ( false )
, m_bLowComplxUpdFlag             ( 1 )
, m_uiQualityLevelForPrediction   ( 3 )
#if MULTIPLE_LOOP_DECODING
, m_bCompletelyDecodeLayer        ( false )
#endif
, m_pcResizeParameter             ( 0 ) //TMM_ESS
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
  m_bLowComplxUpdFlag             = 1;

// *LMH: Inverse MCTF
  m_pcCurrSliceHeader             = 0;
  m_pcResidual                    = 0;
  m_pcBaseLayerFrame              = 0;
  m_pcBaseLayerResidual           = 0;
  m_pcPredSignal                  = 0;
  m_pcBaseLayerCtrl               = 0;
  m_pcCurrDPBUnit                 = 0;
  m_pusUpdateWeights              = 0;

  m_uiLayerId                     = 0;

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
// *LMH: Inverse MCTF
  m_pcCurrSliceHeader         = 0;

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
  RNOK( m_pcDecodedPictureBuffer->initSPS( rcSH->getSPS() ) );  // + 1 (prev. Anchor) + 1 (base rep.)

  //===== initialize some parameters =====
  m_bActive         = true;
  m_bInitDone       = true;

  return Err::m_nOK;
}



ErrVal
MCTFDecoder::process( SliceHeader*&  rpcSliceHeader,
                      PicBuffer*     pcPicBuffer,
                      PicBufferList& rcPicBufferOutputList,
                      PicBufferList& rcPicBufferUnusedList )
{
  ROF  ( m_bInitDone );
  ROTRS( m_bWaitForIdr && !rpcSliceHeader->isIdrNalUnit(), Err::m_nOK );
  m_bWaitForIdr = false;


  //===== decoding =====
  if( rpcSliceHeader->getSliceType() == F_SLICE )
  {
    if( m_uiQualityLevelForPrediction > 0 )
    {
      RNOK( xDecodeFGSRefinement( rpcSliceHeader ) );
    }
  }
  else if( !rpcSliceHeader->getKeyPictureFlag() )
  {
    RNOK( xDecodeHighPassSignal ( rpcSliceHeader, pcPicBuffer, rcPicBufferOutputList, rcPicBufferUnusedList ) );
  }
  else
  {
    RNOK( xDecodeLowPassSignal  ( rpcSliceHeader, pcPicBuffer, rcPicBufferOutputList, rcPicBufferUnusedList ) );
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
MCTFDecoder::getBaseLayerMotionAndResidual( IntFrame*&   pcResidual,
                                            MbDataCtrl*& pcMbDataCtrl,
                                            Int          iPoc )
{
  pcResidual            = 0;
  pcMbDataCtrl          = 0;
  DPBUnit*  pcLastUnit  = m_pcDecodedPictureBuffer->getLastUnit();

  if( pcLastUnit && pcLastUnit->getCtrlData().getSliceHeader() && pcLastUnit->getCtrlData().getSliceHeader()->getPoc() == iPoc )
  {
    pcMbDataCtrl  = pcLastUnit->getCtrlData().getMbDataCtrl();
    pcResidual    = m_pcResidual;
  }
  
  return Err::m_nOK;
}
 


ErrVal
MCTFDecoder::getReconstructedBaseLayer( IntFrame*&   pcFrame,
                                        Bool         bSpatialScalability,
                                        Int          iPoc )
{
  pcFrame                     = 0;
  MbDataCtrl*   pcMbDataCtrl  = 0;
  SliceHeader*  pcSliceHeader = 0;
  DPBUnit*      pcBaseDPBUnit = m_pcDecodedPictureBuffer->getDPBUnit( iPoc );

  if( pcBaseDPBUnit )
  {
    pcFrame       = pcBaseDPBUnit->getFrame   ();
    pcMbDataCtrl  = pcBaseDPBUnit->getCtrlData().getMbDataCtrl  ();
    pcSliceHeader = pcBaseDPBUnit->getCtrlData().getSliceHeader ();
  }

#if MULTIPLE_LOOP_DECODING
  if( pcFrame && ! m_bCompletelyDecodeLayer )
#else
  if( pcFrame )
#endif
  {
    Bool bConstrainedIntra = pcSliceHeader->getPPS().getConstrainedIntraPredFlag();
    if ( bConstrainedIntra && bSpatialScalability )
    {
      RNOK( m_apcFrameTemp[0]->copy( pcFrame ) );
      pcFrame = m_apcFrameTemp[0];

      m_pcLoopFilter->setFilterMode( LoopFilter::LFMode( LoopFilter::LFM_NO_INTER_FILTER + LoopFilter::LFM_EXTEND_INTRA_SUR ) );
      RNOK( m_pcLoopFilter->process(*pcSliceHeader,
                                     pcFrame,
                                     pcMbDataCtrl,
                                     pcMbDataCtrl,
                                     m_uiFrameWidthInMb,
                                     NULL,
                                     NULL ) );
      m_pcLoopFilter->setFilterMode();
    }
    else if( bSpatialScalability )
    {
      RNOK( m_pcLoopFilter->process(*pcSliceHeader,
                                     pcFrame,
                                     pcMbDataCtrl,
                                     pcMbDataCtrl,
                                     m_uiFrameWidthInMb,
                                    &pcBaseDPBUnit->getCtrlData().getPrdFrameList( LIST_0 ),
                                    &pcBaseDPBUnit->getCtrlData().getPrdFrameList( LIST_1 ) ) );
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
  MbDataCtrl*   pcMbDataCtrl    = 0;
  ROFRS   ( (   pcMbDataCtrl    = new MbDataCtrl() ), Err::m_nERR );
  RNOK    (     pcMbDataCtrl    ->init          ( rcSPS ) );
  RNOK    (     m_cControlDataUpd.setMbDataCtrl ( pcMbDataCtrl ) );

  ROFRS   ( ( m_pcBaseLayerCtrl = new MbDataCtrl() ), Err::m_nERR );
  RNOK    (   m_pcBaseLayerCtrl ->init          ( rcSPS ) );



  //========== CREATE UPDATE WEIGHTS ARRAY and WRITE BUFFER ==========
  UInt  uiNum4x4Blocks        = m_uiFrameWidthInMb * m_uiFrameHeightInMb * 4 * 4;
  ROFRS( ( m_pusUpdateWeights = new UShort[ uiNum4x4Blocks      ] ), Err::m_nERR );



  //========== RE-INITIALIZE OBJECTS ==========
  RNOK( m_cConnectionArray.init   ( rcSPS ) );
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
  MbDataCtrl*   pcMbDataCtrl  = m_cControlDataUpd.getMbDataCtrl  ();
  SliceHeader*  pcSliceHeader = m_cControlDataUpd.getSliceHeader ();
  if( pcMbDataCtrl )
  {
    RNOK( pcMbDataCtrl->uninit() );
  }
  delete pcMbDataCtrl;
  delete pcSliceHeader;
  m_cControlDataUpd.setMbDataCtrl ( 0 );
  m_cControlDataUpd.setSliceHeader( 0 );

  if( m_pcBaseLayerCtrl )
  {
    RNOK( m_pcBaseLayerCtrl->uninit() );
    delete m_pcBaseLayerCtrl;
    m_pcBaseLayerCtrl = 0;
  }
  

  //========== DELETE UPDATE WEIGHTS ARRAY and WRITE BUFFER ==========
  delete [] m_pusUpdateWeights;
  m_pusUpdateWeights  = 0;


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
MCTFDecoder::xClipIntraMacroblocks( IntFrame*    pcFrame,
                                   ControlData& rcCtrlData, Bool bClipAll )
{
  MbDataCtrl*       pcMbDataCtrl  = rcCtrlData.getMbDataCtrl       ();
  SliceHeader*      pcSliceHeader = rcCtrlData.getSliceHeader      ();
  IntYuvPicBuffer*  pcPicBuffer   = pcFrame  ->getFullPelYuvBuffer ();
  IntYuvMbBuffer    cMbBuffer;

  RNOK( pcMbDataCtrl->initSlice( *pcSliceHeader, PRE_PROCESS, false, NULL ) );

  for( UInt uiMbIndex = 0; uiMbIndex < m_uiMbNumber; uiMbIndex++ )
  {
    UInt          uiMbY           = uiMbIndex / m_uiFrameWidthInMb;
    UInt          uiMbX           = uiMbIndex % m_uiFrameWidthInMb;
    MbDataAccess* pcMbDataAccess  = 0;

    RNOK( pcMbDataCtrl            ->initMb( pcMbDataAccess, uiMbY, uiMbX ) );
    RNOK( m_pcYuvFullPelBufferCtrl->initMb(                 uiMbY, uiMbX ) );

    if( bClipAll || pcMbDataAccess->getMbData().isIntra() )
    {
      cMbBuffer   .loadBuffer( pcPicBuffer );
      cMbBuffer   .clip      ();
      pcPicBuffer->loadBuffer( &cMbBuffer );
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



// *LMH: Inverse MCTF
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
    RNOK( xReconstructLastFGS() );
  }

// *LMH: Inverse MCTF
  if( m_pcCurrSliceHeader != 0                                                           &&
      m_pcCurrSliceHeader->getLayerId() == m_uiLayerId                                   &&
    (!pcSliceHeader                                                                      ||
      pcSliceHeader->getLayerId       () != m_pcCurrSliceHeader->getLayerId      ()      ||
      pcSliceHeader->getTemporalLevel () != m_pcCurrSliceHeader->getTemporalLevel()      ||
      pcSliceHeader->getPoc           () != m_pcCurrSliceHeader->getPoc          ()        ) )
  {
    // After all slices of the current picture, pic, have been decoded from the bitstream,
    // the inverse motion compensated temporal filtering process is invoked
#if MULTIPLE_LOOP_DECODING
    Bool bIntraOnly   = ( !m_bCompletelyDecodeLayer && uiLastLayer != m_uiLayerId );
#else
    Bool bIntraOnly   = ( uiLastLayer != m_uiLayerId );
#endif
    RNOK( xInvokeMCTF( m_pcCurrSliceHeader, bIntraOnly ) );
    m_pcCurrSliceHeader = 0;
  }
  return Err::m_nOK;
}



ErrVal
MCTFDecoder::xReconstructLastFGS()
{
  if( m_pcRQFGSDecoder->changed() )
  {
    DPBUnit*      pcLastUnit      = m_pcDecodedPictureBuffer->getLastUnit();
    SliceHeader*  pcSliceHeader   = m_pcRQFGSDecoder->getSliceHeader();
    ROF( pcLastUnit );
    ROF( pcSliceHeader == pcLastUnit->getCtrlData().getSliceHeader() );

    IntFrame*     pcFrame         = pcLastUnit->getFrame    ();
    ControlData&  rcControlData   = pcLastUnit->getCtrlData ();

    RNOK( m_pcRQFGSDecoder->reconstruct( pcFrame, pcSliceHeader->getKeyPictureFlag() ) );

    RNOK( xAddBaseLayerResidual   ( rcControlData, pcFrame ) );
    RNOK( m_pcResidual    ->copy  ( pcFrame ) )
    RNOK( xZeroIntraMacroblocks   ( m_pcResidual, rcControlData ) );

    if( pcSliceHeader->getKeyPictureFlag() )
    {
      RNOK( pcFrame         ->add   ( m_pcPredSignal ) );
      RNOK( xClipIntraMacroblocks   ( pcFrame, rcControlData, !pcLastUnit->isResidual() ) );
    }

    //===== update picture in DPB =====
    RNOK( m_pcDecodedPictureBuffer->update( pcLastUnit ) );
  }
  else if( !m_pcRQFGSDecoder->getSliceHeader()->getKeyPictureFlag() )
  {
    //===== restore base layer intra coefficients =====
    MbDataCtrl* pcMbDataCtrl = m_pcDecodedPictureBuffer->getLastUnit()->getCtrlData().getMbDataCtrl();
    RNOK( pcMbDataCtrl          ->initSlice( *m_pcRQFGSDecoder->getSliceHeader(), PRE_PROCESS, true, NULL ) );
    RNOK( m_pcMotionCompensation->initSlice( *m_pcRQFGSDecoder->getSliceHeader()              ) );

    for( UInt uiMbIndex = 0; uiMbIndex < m_uiMbNumber; uiMbIndex++ )
    {
      UInt          uiMbY           = uiMbIndex / m_uiFrameWidthInMb;
      UInt          uiMbX           = uiMbIndex % m_uiFrameWidthInMb;
      MbDataAccess* pcMbDataAccess  = 0;

      RNOK( pcMbDataCtrl            ->initMb( pcMbDataAccess, uiMbY, uiMbX ) );
      if( pcMbDataAccess->getMbData().isIntra() )
      {
        pcMbDataAccess->getMbTCoeffs().copyFrom( pcMbDataAccess->getMbData().getIntraBaseCoeffs() );
      }
    }
  }

  RNOK( m_pcRQFGSDecoder->finishPicture () );
  

  return Err::m_nOK;
}





ErrVal
MCTFDecoder::xDecodeFGSRefinement( SliceHeader*& rpcSliceHeader )
{
  ROFRS( m_pcRQFGSDecoder->isInitialized(), Err::m_nOK );

  printf("  Frame %4d ( LId%2d, TL%2d, QL%2d, PrRef,              QP%3d )\n",
    rpcSliceHeader->getPoc                    (),
    rpcSliceHeader->getLayerId                (),
    rpcSliceHeader->getTemporalLevel          (),
    rpcSliceHeader->getQualityLevel           (),
    rpcSliceHeader->getPicQp                  () );

  //===== check slice header =====
  if(!m_pcRQFGSDecoder->isFinished    ()                                                                 &&
      m_pcRQFGSDecoder->getSliceHeader()->getTemporalLevel()      == rpcSliceHeader->getTemporalLevel () &&
      m_pcRQFGSDecoder->getSliceHeader()->getPoc          ()      == rpcSliceHeader->getPoc           () &&
      m_pcRQFGSDecoder->getSliceHeader()->getQualityLevel () + 1  == rpcSliceHeader->getQualityLevel  ()  )
  {
    if( rpcSliceHeader->getQualityLevel() <= m_uiQualityLevelForPrediction )
    {
      RNOK( m_pcRQFGSDecoder->decodeNextLayer( rpcSliceHeader ) );
    }
  }

  return Err::m_nOK;
}






ErrVal
MCTFDecoder::xInitBaseLayerMotionAndResidual( ControlData& rcControlData )
{
  //===== init =====
  rcControlData.setBaseLayerRec ( 0 );
  rcControlData.setBaseLayerSbb ( 0 );
  rcControlData.setBaseLayerCtrl( 0 );

  
  IntFrame*   pcBaseResidual      = 0;
  MbDataCtrl* pcBaseDataCtrl      = 0;
  Bool        bSpatialScalability = false;
  Bool        bBaseDataAvailable  = false;

  if( rcControlData.getSliceHeader()->getBaseLayerId() != MSYS_UINT_MAX )
  {
      //TMM_ESS { 
      SliceHeader* pcSliceHeader=rcControlData.getSliceHeader();
      Int poc = pcSliceHeader->getPoc();
      m_pcResizeParameter->setPictureParametersByOffset(poc,
                                                      pcSliceHeader->getLeftOffset(),
                                                      pcSliceHeader->getRightOffset(),
                                                      pcSliceHeader->getTopOffset(),
                                                      pcSliceHeader->getBottomOffset(),
                                                      pcSliceHeader->getBaseChromaPhaseX(),
                                                      pcSliceHeader->getBaseChromaPhaseY()
                                                    );
      m_pcResizeParameter->setPOC( poc );
     //TMM_ESS }   
      
    RNOK( m_pcH264AVCDecoder->getBaseLayerMotionAndResidual( pcBaseResidual, pcBaseDataCtrl, bSpatialScalability,
                                                             m_uiLayerId,
                                                             rcControlData.getSliceHeader()->getBaseLayerId(),
                                                             rcControlData.getSliceHeader()->getPoc() ) );    
    bBaseDataAvailable = pcBaseResidual && pcBaseDataCtrl;
    ROF( bBaseDataAvailable );

    rcControlData.setSpatialScalability( bSpatialScalability );
    rcControlData.setSpatialScalabilityType(m_pcResizeParameter->m_iSpatialScalabilityType);
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
      {RNOK( m_pcDecodedPictureBuffer->fillPredictionLists_ESS( m_pcResizeParameter ) );}
   
      RNOK( m_pcBaseLayerCtrl->upsampleMotion( *pcBaseDataCtrl, m_pcResizeParameter) );
    }
    rcControlData.setBaseLayerCtrl( m_pcBaseLayerCtrl );
  }


  //===== residual data =====
  if( bBaseDataAvailable )
  {
    RNOK( m_pcBaseLayerResidual->copy( pcBaseResidual ) ); 
	// TMM_ESS 
	RNOK( m_pcBaseLayerResidual->upsampleResidual( m_cDownConvert, m_pcResizeParameter, pcBaseDataCtrl, false) ); 
		
    rcControlData.setBaseLayerSbb( m_pcBaseLayerResidual );
  }

  return Err::m_nOK;
}







ErrVal
MCTFDecoder::xInitBaseLayerReconstruction( ControlData& rcControlData )
{
  //===== init =====
  rcControlData.setBaseLayerRec ( 0 );
  IntFrame*   pcBaseFrame       = 0;

  if( rcControlData.getSliceHeader()->getBaseLayerId() != MSYS_UINT_MAX )
  {
    //TMM_ESS { 
      SliceHeader* pcSliceHeader=rcControlData.getSliceHeader();
      Int poc = pcSliceHeader->getPoc();
      m_pcResizeParameter->setPictureParametersByOffset(poc,
                                                      pcSliceHeader->getLeftOffset(),
                                                      pcSliceHeader->getRightOffset(),
                                                      pcSliceHeader->getTopOffset(),
                                                      pcSliceHeader->getBottomOffset(),
                                                      pcSliceHeader->getBaseChromaPhaseX(),
                                                      pcSliceHeader->getBaseChromaPhaseY()
                                                    );
      m_pcResizeParameter->setPOC( poc );
     //TMM_ESS }   
    
    RNOK( m_pcH264AVCDecoder->getReconstructedBaseLayer( pcBaseFrame,
                                                         m_uiLayerId,
                                                         rcControlData.getSliceHeader()->getBaseLayerId(),
                                                         rcControlData.getSpatialScalability(),
                                                         rcControlData.getSliceHeader()->getTemporalLevel() > 0,
                                                         rcControlData.getSliceHeader()->getPoc() ) );
    ROF( pcBaseFrame );
  }

  //==== reconstructed (intra) data =====
  if( pcBaseFrame )
  {
    RNOK( m_pcBaseLayerFrame->copy( pcBaseFrame ) );
	// TMM_ESS 
	RNOK( m_pcBaseLayerFrame->upsample(m_cDownConvert, m_pcResizeParameter, true) );

    rcControlData.setBaseLayerRec( m_pcBaseLayerFrame );
  }


  return Err::m_nOK;
}




ErrVal
MCTFDecoder::xDecodeLowPassSignal( SliceHeader*&  rpcSliceHeader,
                                   PicBuffer*&    rpcPicBuffer,
                                   PicBufferList& rcOutputList,
                                   PicBufferList& rcUnusedList )
{
  printf("  Frame %4d ( LId%2d, TL%2d, QL%2d, %s-%c, BId%2d, AP%2d, QP%3d )\n",
    rpcSliceHeader->getPoc                    (),
    rpcSliceHeader->getLayerId                (),
    rpcSliceHeader->getTemporalLevel          (),
    rpcSliceHeader->getQualityLevel           (),
    rpcSliceHeader->isH264AVCCompatible       () ? "AVC" : " LP",
    rpcSliceHeader->getSliceType              () == I_SLICE ? 'I' : 'P',
    rpcSliceHeader->getBaseLayerId            (),
    rpcSliceHeader->getAdaptivePredictionFlag () ? 1 : 0,
    rpcSliceHeader->getPicQp                  () );


  //===== init =====
  Bool bResidual = false;
  RNOK( m_pcDecodedPictureBuffer->initCurrDPBUnit( m_pcCurrDPBUnit, rpcPicBuffer, bResidual, rpcSliceHeader,
                                                   rcOutputList, rcUnusedList ) );
  RNOK( xInitBaseLayerMotionAndResidual ( m_pcCurrDPBUnit->getCtrlData() ) );
  RNOK( xInitBaseLayerReconstruction    ( m_pcCurrDPBUnit->getCtrlData() ) );

  ControlData&  rcControlData = m_pcCurrDPBUnit->getCtrlData();
  IntFrame*     pcFrame       = m_pcCurrDPBUnit->getFrame   ();
  IntFrame*     pcResidual    = m_pcResidual;
  IntFrame*     pcLPFrame     = m_apcFrameTemp[0];
  MbDataCtrl*   pcMbDataCtrl  = rcControlData.getMbDataCtrl();
  UInt          uiMbRead      = 0;
  
  //----- initialize reference lists -----
  RNOK( m_pcDecodedPictureBuffer->setPrdRefLists( m_pcCurrDPBUnit ) );


  //----- decode and reconstruct low-pass picture and residual -----
  if( rpcSliceHeader->isIntra() )
  {
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
    RNOK( m_pcSliceDecoder->decodeIntra         ( *rpcSliceHeader,
                                                  pcMbDataCtrl,
                                                  pcLPFrame,
                                                  pcResidual,
                                                  m_pcPredSignal,
                                                  rcControlData.getBaseLayerRec(),
                                                  m_uiFrameWidthInMb,
                                                  uiMbRead ) );
  }
  else
  {
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
    RNOK( m_pcSliceDecoder->decodeInterP        ( *rpcSliceHeader,
                                                  pcMbDataCtrl,
                                                  rcControlData.getBaseLayerCtrl(),
                                                  pcLPFrame,
                                                  pcResidual,
                                                  m_pcPredSignal,
                                                  rcControlData.getBaseLayerRec(),
                                                  rcControlData.getBaseLayerSbb(),
                                                  rcControlData.getPrdFrameList( LIST_0 ),
                                                  m_uiFrameWidthInMb,
                                                  uiMbRead ) );
  }


  //----- store non-filtered frame and residual -----
  RNOK( pcFrame->copy( pcLPFrame ) );

  //----- loop-filtering and store in DPB as base representation -----
  RNOK( m_pcLoopFilter->process( *rpcSliceHeader,
                                 pcLPFrame,
                                 ( rpcSliceHeader->isIntra() ? NULL : pcMbDataCtrl ),
                                 pcMbDataCtrl,
                                 m_uiFrameWidthInMb,
                                 &rcControlData.getPrdFrameList( LIST_0 ),
                                 &rcControlData.getPrdFrameList( LIST_1 ) ) );
  RNOK( m_pcDecodedPictureBuffer->store( m_pcCurrDPBUnit, rcOutputList, rcUnusedList, pcLPFrame ) );

  //----- set slice header to zero (slice header is stored in control data) -----
  rpcSliceHeader = 0;

  if( m_uiQualityLevelForPrediction > 0 )
  {
    RNOK( m_pcRQFGSDecoder->initPicture( rcControlData.getSliceHeader(), rcControlData.getMbDataCtrl() ) );
  }
// *LMH: Inverse MCTF
  m_pcCurrSliceHeader = rcControlData.getSliceHeader();

  DTRACE_NEWFRAME;
  return Err::m_nOK;
}





ErrVal
MCTFDecoder::xDecodeHighPassSignal( SliceHeader*&   rpcSliceHeader,
                                    PicBuffer*&     rpcPicBuffer,
                                    PicBufferList&  rcOutputList,
                                    PicBufferList&  rcUnusedList )
{
  printf("  Frame %4d ( LId%2d, TL%2d, QL%2d, %s-%c, BId%2d, AP%2d, QP%3d )\n",
    rpcSliceHeader->getPoc                    (),
    rpcSliceHeader->getLayerId                (),
    rpcSliceHeader->getTemporalLevel          (),
    rpcSliceHeader->getQualityLevel           (),
    rpcSliceHeader->isH264AVCCompatible       () ? "AVC" : " HP",
    rpcSliceHeader->getSliceType              () == B_SLICE ? 'B' : 'P',
    rpcSliceHeader->getBaseLayerId            (),
    rpcSliceHeader->getAdaptivePredictionFlag () ? 1 : 0,
    rpcSliceHeader->getPicQp                  () );

  //===== init =====
  Bool bResidual = true;
  RNOK( m_pcDecodedPictureBuffer->initCurrDPBUnit( m_pcCurrDPBUnit, rpcPicBuffer, bResidual, rpcSliceHeader,
                                                   rcOutputList, rcUnusedList ) );
  RNOK( xInitBaseLayerMotionAndResidual( m_pcCurrDPBUnit->getCtrlData() ) );

  ControlData&  rcControlData = m_pcCurrDPBUnit->getCtrlData();
  IntFrame*     pcFrame       = m_pcCurrDPBUnit->getFrame   ();
  IntFrame*     pcResidual    = m_pcResidual;
  MbDataCtrl*   pcMbDataCtrl  = rcControlData.getMbDataCtrl();
  UInt          uiMbRead      = 0;
  
  //----- initialize reference lists -----
  RNOK( m_pcDecodedPictureBuffer->setPrdRefLists( m_pcCurrDPBUnit ) );


  //----- parsing -----
  RNOK( m_pcControlMng  ->initSliceForReading ( *rpcSliceHeader ) );
  
  RNOK( m_pcSliceReader ->read                ( *rpcSliceHeader,
                                                pcMbDataCtrl,
                                                rcControlData.getBaseLayerCtrl(),
                                                rcControlData.getSpatialScalabilityType(),
                                                m_uiFrameWidthInMb,
                                                uiMbRead ) );

  //----- decode motion vectors -----
  if( rpcSliceHeader->getBaseLayerId() == MSYS_UINT_MAX || rpcSliceHeader->getAdaptivePredictionFlag() )
  {
    RNOK( xCalcMv( rpcSliceHeader, pcMbDataCtrl, rcControlData.getBaseLayerCtrl() ) );
  }

  //----- decode residual signals -----
  RNOK( m_pcControlMng  ->initSliceForDecoding( *rpcSliceHeader ) );
  RNOK( m_pcSliceDecoder->decodeHighPass      ( *rpcSliceHeader,
                                                pcMbDataCtrl,
                                                rcControlData.getBaseLayerCtrl(),
                                                pcFrame,
                                                pcResidual,
                                                rcControlData.getBaseLayerSbb(),
                                                m_uiFrameWidthInMb,
                                                uiMbRead ) );

  //----- updating -----
  RNOK( m_pcDecodedPictureBuffer->store( m_pcCurrDPBUnit, rcOutputList, rcUnusedList ) );

  //----- set slice header to zero (slice header is stored in control data) -----
  rpcSliceHeader = 0;

  if( m_uiQualityLevelForPrediction > 0 )
  {
    RNOK( m_pcRQFGSDecoder->initPicture( rcControlData.getSliceHeader(), rcControlData.getMbDataCtrl() ) );
  }
// *LMH: Inverse MCTF
  m_pcCurrSliceHeader = rcControlData.getSliceHeader();

  DTRACE_NEWFRAME;

  return Err::m_nOK;
}


// *LMH: Inverse MCTF
ErrVal
MCTFDecoder::xInvokeMCTF( SliceHeader*  pcSliceHeader,
                          Bool          bIntraOnly ) //HS: only intra macroblocks in non-key pictures are reconstructed
// After all slices of the current picture, pic, have been decoded from the bitstream,
// the inverse motion compensated temporal filtering process is invoked
// If picture pic is marked as "residual picture",
//   The "check for inverse prediction process" in subclause S.8.8.1 is invoked with pic and NULL as inputs
// Otherwise (i.e. picture pic is not marked as "residual picture"),
//   The "check for inverse update process" in subclause S.8.8.2 is invoked with pic and NULL as inputs
{
  DPBUnit*  pcLastDPBUnit = m_pcDecodedPictureBuffer->getLastUnit();
  ROF( pcLastDPBUnit );
  ROF( pcLastDPBUnit->getCtrlData().getSliceHeader() == pcSliceHeader );


  if( pcLastDPBUnit->isResidual() )
  {
    RNOK( xCheckForInversePrediction( pcLastDPBUnit, NULL, bIntraOnly ) );
  }
  // Otherwise (i.e. picture pic is not marked as "residual picture"),
 	// The "check for inverse update process" in subclause S.8.8.2 is invoked with pic and NULL as inputs
  else
  {
    // HS: loop filter for low-pass frames
    if( pcLastDPBUnit->isKeyPic() && ! bIntraOnly )
    {
      IntFrame*     pcFrame         = pcLastDPBUnit->getFrame   ();
      ControlData&  rcControlData   = pcLastDPBUnit->getCtrlData();
      RefFrameList& rcRefFrameList0 = rcControlData.getPrdFrameList( 0 );
      RefFrameList& rcRefFrameList1 = rcControlData.getPrdFrameList( 1 );
      MbDataCtrl*   pcMbDataCtrl    = rcControlData.getMbDataCtrl();

      RNOK( m_pcLoopFilter->process( *pcSliceHeader,
                                      pcFrame,
                                      ( pcSliceHeader->isIntra() ? NULL : pcMbDataCtrl ),
                                      pcMbDataCtrl,
                                      m_uiFrameWidthInMb,
                                     &rcRefFrameList0,
                                     &rcRefFrameList1 ) );
      RNOK( m_pcDecodedPictureBuffer->update( pcLastDPBUnit ) );
    }

    // Otherwise (i.e. picture pic is not marked as "residual picture"),
    // The "check for inverse update process" in subclause S.8.8.2 is invoked with pic and NULL as inputs
    RNOK( xCheckForInverseUpdate( pcLastDPBUnit, NULL, bIntraOnly ) );
  }

  return Err::m_nOK;
}




ErrVal
MCTFDecoder::xCheckForInversePrediction( DPBUnit* pcPrdDPBUnit,
                                         DPBUnit* pcUpdDPBUnit,
                                         Bool     bIntraOnly )
// Inputs to this process are
// - A picture prdPic that is to be checked in this process if it can be inverse predicted.
// - A picture updPic that is inverse updated prior to invoking this process.
//   If updPic is NULL (-1), it indicates that no picture is inverse updated prior to invoking this process.
//
// Outputs of this process are zero or more modified pictures (including prdPic), which replace the input version in the decoded picture buffer.
//
// For each picture refPic in the reference picture lists RefPicListX (with X being 0 or 1) of prdPic,
// - If all of the following conditions are true,
// 	 - refPic is not marked as "base representation"
// 	 - curTemporalLevelrefPic is equal to curTemporalLevelprdPic
//   The following applies:
// 	 - Let Y be a template variable that is derived by Y = 1 - X.
// 	 - The variable curActiveUpdLYrefPic[curTemporalLevelrefPic] is incremented by 1,
// 	 - The "check for inverse update process" in subclause S.8.8.2 is invoked with refPic and prdPic as input.
//
// - If all of the following conditions are true,
//   - refPic is marked as "base representation"
// 	 - updPic is equal to NULL
//   The following applies:
//   - The variable curActivePrdLXprdPic is incremented by 1,
//
// - If all of the following conditions are true,
//   - refPic is not marked as "base representation"
// 	 - updPic is equal to NULL or updPic is equal to refPic
// 	 - curTemporalLevelrefPic is equal to curTemporalLevelprdPic + 1
//   The following applies:
// 	 - The variable curActivePrdLXprdPic is incremented by 1,
//
// If all of the following conditions are true for picture prdPic,
// - curActivePrdL0prdPic is equal to maxActivePrdL0prdPic
// - curActivePrdL1prdPic is equal to maxActivePrdL1prdPic
// The following applies:
// - The inverse prediction step is performed on prdPic. The motion compensated prediction process in subclause S.8.8.2 of [1] is invoked with prdPic as input and the output is a modified picture prdPic, which replaces the input version in the decoded picture buffer.
// - The marking "residual picture" is removed for the picture prdPic.
// - The variable curTemporalLevelprdPic is incremented by 1,
// - The "check for inverse update process" in subclause S.8.8.2 is invoked with prdPic and NULL as input.
{
  ControlData&  rcPrdPicControlData   = pcPrdDPBUnit->getCtrlData();
  SliceHeader*  pcPrdPicSliceHeader   = rcPrdPicControlData.getSliceHeader();
  UInt          uiPredictionLevel     = rcPrdPicControlData.getCurTemporalLevel();
  if( uiPredictionLevel >= pcPrdPicSliceHeader->getDecompositionStages() )
  {
    rcPrdPicControlData.setComplete( true ); // HS: that's not the WD !!!!
    return Err::m_nOK;
  }
  RefFrameList& rcPrdFrameList0       = rcPrdPicControlData.getPrdFrameList( LIST_0 );
  RefFrameList& rcPrdFrameList1       = rcPrdPicControlData.getPrdFrameList( LIST_1 );
  Int           iIdInRefList;


  //===== list 0 =====
// For each picture refPic in the reference picture lists RefPicListX (with X being 0 or 1) of prdPic,
  for( iIdInRefList = 1; iIdInRefList <= rcPrdFrameList0.getSize(); iIdInRefList++ )
  {
    DPBUnit* pcDPBUnitRef = rcPrdFrameList0[iIdInRefList]->getDPBUnit();

// - If all of the following conditions are true,
// 	 - curTemporalLevelrefPic is equal to curTemporalLevelprdPic
    if (!pcDPBUnitRef->getCtrlData().isComplete() && // HS: that's not in the WD
         pcDPBUnitRef->getCtrlData().getCurTemporalLevel() == uiPredictionLevel )
    {
//   The following applies:
// 	 - Let Y be a template variable that is derived by Y = 1 - X.
// 	 - The variable curActiveUpdLYrefPic[curTemporalLevelrefPic] is incremented by 1,
// 	 - The "check for inverse update process" in subclause S.8.8.2 is invoked with refPic and prdPic as input.
      pcDPBUnitRef->getCtrlData().incCurActiveUpdL1( pcDPBUnitRef->getCtrlData().getCurTemporalLevel() );
      RNOK( xCheckForInverseUpdate( pcDPBUnitRef, pcPrdDPBUnit, bIntraOnly) );
    }


// - If all of the following conditions are true,
//   - refPic is marked as "base representation"
// 	 - updPic is equal to NULL
    if ( pcDPBUnitRef->getCtrlData().isComplete() && !pcUpdDPBUnit )
    {
//   The following applies:
//   - The variable curActivePrdLXprdPic is incremented by 1,
      rcPrdPicControlData.incCurActivePrdL0();
    }

// - If all of the following conditions are true,
//   - refPic is not marked as "base representation"
// 	 - updPic is equal to NULL or updPic is equal to refPic
// 	 - curTemporalLevelrefPic is equal to curTemporalLevelprdPic + 1
    if ( !pcDPBUnitRef->getCtrlData().isComplete() &&
         ( !pcUpdDPBUnit || pcUpdDPBUnit == pcDPBUnitRef ) &&
         pcDPBUnitRef->getCtrlData().getCurTemporalLevel() >= uiPredictionLevel + 1 ) // bug fix by HS (>= instead of ==) -> important for some low-delay coding structures
    {
//   The following applies:
// 	 - The variable curActivePrdLXprdPic is incremented by 1,
      rcPrdPicControlData.incCurActivePrdL0();
    }
  }

  
  //===== list 1 =====
// For each picture refPic in the reference picture lists RefPicListX (with X being 0 or 1) of prdPic,
  for( iIdInRefList = 1; iIdInRefList <= rcPrdFrameList1.getSize(); iIdInRefList++ )
  {
    DPBUnit* pcDPBUnitRef = rcPrdFrameList1[iIdInRefList]->getDPBUnit();

// - If all of the following conditions are true,
// 	 - curTemporalLevelrefPic is equal to curTemporalLevelprdPic
    if (!pcDPBUnitRef->getCtrlData().isComplete() && // HS: that's not in the WD
         pcDPBUnitRef->getCtrlData().getCurTemporalLevel() == uiPredictionLevel )
    {
//   The following applies:
// 	 - Let Y be a template variable that is derived by Y = 1 - X.
// 	 - The variable curActiveUpdLYrefPic[curTemporalLevelrefPic] is incremented by 1,
// 	 - The "check for inverse update process" in subclause S.8.8.2 is invoked with refPic and prdPic as input.
      pcDPBUnitRef->getCtrlData().incCurActiveUpdL0( pcDPBUnitRef->getCtrlData().getCurTemporalLevel() );
      RNOK( xCheckForInverseUpdate( pcDPBUnitRef, pcPrdDPBUnit, bIntraOnly ) );
    }

// - If all of the following conditions are true,
//   - refPic is marked as "base representation"
// 	 - updPic is equal to NULL
    if ( pcDPBUnitRef->getCtrlData().isComplete() && !pcUpdDPBUnit )
    {
//   The following applies:
//   - The variable curActivePrdLXprdPic is incremented by 1,
      rcPrdPicControlData.incCurActivePrdL1();
    }

// - If all of the following conditions are true,
//   - refPic is not marked as "base representation"
// 	 - updPic is equal to NULL or updPic is equal to refPic
// 	 - curTemporalLevelrefPic is equal to curTemporalLevelprdPic + 1
    if ( !pcDPBUnitRef->getCtrlData().isComplete() &&
         ( !pcUpdDPBUnit || pcUpdDPBUnit == pcDPBUnitRef ) &&
         pcDPBUnitRef->getCtrlData().getCurTemporalLevel() == uiPredictionLevel + 1 ) // bug fix by HS (>= instead of ==) -> important for some low-delay coding structures
    {
//   The following applies:
// 	 - The variable curActivePrdLXprdPic is incremented by 1,
      rcPrdPicControlData.incCurActivePrdL1();
    }
  }
  
// If all of the following conditions are true for picture prdPic,
// - curActivePrdL0prdPic is equal to maxActivePrdL0prdPic
// - curActivePrdL1prdPic is equal to maxActivePrdL1prdPic
  if ( rcPrdPicControlData.getCurActivePrdL0() == pcPrdPicSliceHeader->getNumRefIdxActive( LIST_0 ) && 
       rcPrdPicControlData.getCurActivePrdL1() == pcPrdPicSliceHeader->getNumRefIdxActive( LIST_1 ) )
  {
// The following applies:
// - The inverse prediction step is performed on prdPic. The motion compensated prediction process in subclause S.8.8.4 of [1] is invoked with prdPic as input and the output is a modified picture prdPic, which replaces the input version in the decoded picture buffer.
// - The marking "residual picture" is removed for the picture prdPic.
// - The variable curTemporalLevelprdPic is incremented by 1,
// - The "check for inverse update process" in subclause S.8.8.2 is invoked with prdPic and NULL as input.
    RNOK( xInversePrediction( pcPrdDPBUnit, bIntraOnly ) );
    rcPrdPicControlData.incCurTemporalLevel();
    if ( rcPrdPicControlData.getCurTemporalLevel() == pcPrdPicSliceHeader->getDecompositionStages() )
    {
      rcPrdPicControlData.setComplete( true ); // HS: that's not the WD !!!!
    }
    else
    {
      RNOK( xCheckForInverseUpdate( pcPrdDPBUnit, NULL, bIntraOnly ) );
    }
  }

  return Err::m_nOK;
}

ErrVal
MCTFDecoder::xCheckForInverseUpdate( DPBUnit* pcUpdDPBUnit,
                                     DPBUnit* pcPrdDPBUnit,
                                     Bool     bIntraOnly )
// Inputs to this process are
// - A picture updPic that is to be checked in this process if it can be inverse updated.
// - A picture prdPic that is checked for inverse prediction prior to invoking this process.
//   If prdPic is NULL, it indicates that no picture is checked for inverse prediction prior to invoking this process.
//
// Output of this process is possibly the modified picture updPic, which replace the input version in the decoded picture buffer.
//
// If all of the following conditions are true for picture updPic,
// - updPic is not marked as "residual picture"
// - updPic is not marked as "base representation"
// - curTemporalLevelupdPic is less than decomposition_stages
// - curActiveUpdL0updPic[curTemporalLevelupdPic] is equal to maxActiveUpdL0updPic[curTemporalLevelupdPic]
// - curActiveUpdL1updPic[curTemporalLevelupdPic] is equal to maxActiveUpdL1updPic[curTemporalLevelupdPic]
// The following applies:
// - The inverse update step is performed on updPic. The motion compensated update process in subclause S.8.8.1 of [1] is invoked with updPic and curTemporalLevelupdPic as inputs and the output is a modified picture updPic, which replaces the input version in the decoded picture buffer.
// - The variable curTemporalLevelupdPic is incremented by 1,
// - For each picture refPic in the update picture lists updPicListX (with X being 0 or 1) of updPic, the following applies:
//   - If (prdPic is equal to NULL or prdPic is not equal to refPic)
//     - The "check for inverse prediction process" in subclause S.8.8.1 is invoked with refPic and updPic as inputs.
// - The "check for inverse update process" in subclause S.8.8.2 is invoked with updPic and NULL as inputs.
{
  ControlData&  rcUpdPicControlData   = pcUpdDPBUnit->getCtrlData();
  SliceHeader*  pcUpdPicSliceHeader   = rcUpdPicControlData.getSliceHeader();
  UInt          uiUpdateLevel         = rcUpdPicControlData.getCurTemporalLevel();
  if( uiUpdateLevel >= pcUpdPicSliceHeader->getDecompositionStages() )
  {
    rcUpdPicControlData.setComplete( true ); // HS: that's not the WD !!!!
    return Err::m_nOK;
  }
  RefFrameList& rcUpdFrameList0       = rcUpdPicControlData.getUpdFrameList( uiUpdateLevel, LIST_0 );
  RefFrameList& rcUpdFrameList1       = rcUpdPicControlData.getUpdFrameList( uiUpdateLevel, LIST_1 );
  Int           iIdInUpdList;

// If all of the following conditions are true for picture updPic,
// - updPic is not marked as "residual picture"
// - updPic is not marked as "base representation"
// - curTemporalLevelupdPic is less than decomposition_stages
// - curActiveUpdL0updPic[curTemporalLevelupdPic] is equal to maxActiveUpdL0updPic[curTemporalLevelupdPic]
// - curActiveUpdL1updPic[curTemporalLevelupdPic] is equal to maxActiveUpdL1updPic[curTemporalLevelupdPic]
  if (!pcUpdDPBUnit->isResidual() &&
      !rcUpdPicControlData.isComplete() && // HS: that's not in the WD !!!
       uiUpdateLevel < pcUpdPicSliceHeader->getDecompositionStages() &&
       rcUpdPicControlData.getCurActiveUpdL0(uiUpdateLevel) == pcUpdPicSliceHeader->getNumRefIdxUpdate( uiUpdateLevel, LIST_0 ) &&
       rcUpdPicControlData.getCurActiveUpdL1(uiUpdateLevel) == pcUpdPicSliceHeader->getNumRefIdxUpdate( uiUpdateLevel, LIST_1 ) )
  {
// The following applies:
// - The inverse update step is performed on updPic. The motion compensated update process in subclause S.8.8.1 of [1] is invoked with updPic and curTemporalLevelupdPic as inputs and the output is a modified picture updPic, which replaces the input version in the decoded picture buffer.
// - The variable curTemporalLevelupdPic is incremented by 1,
    RNOK( xInverseUpdate( pcUpdDPBUnit, bIntraOnly ) );
    rcUpdPicControlData.incCurTemporalLevel();

// - For each picture refPic in the update picture lists updPicListX (with X being 0 or 1) of updPic, the following applies:
//   - If (prdPic is equal to NULL or prdPic is not equal to refPic)
//     - The "check for inverse prediction process" in subclause S.8.8.1 is invoked with refPic and updPic as inputs.
    //===== list 0 =====
    for( iIdInUpdList = 1; iIdInUpdList <= rcUpdFrameList0.getSize(); iIdInUpdList++ )
    {
      DPBUnit* pcDPBUnitRef = rcUpdFrameList0[iIdInUpdList]->getDPBUnit();
      
      if( pcPrdDPBUnit == NULL || pcPrdDPBUnit != pcDPBUnitRef )
      {
        RNOK( xCheckForInversePrediction( pcDPBUnitRef, pcUpdDPBUnit, bIntraOnly ) );
      }
    }

    //===== list 1 =====
    for( iIdInUpdList = 1; iIdInUpdList <= rcUpdFrameList1.getSize(); iIdInUpdList++ )
    {
      DPBUnit* pcDPBUnitRef = rcUpdFrameList1[iIdInUpdList]->getDPBUnit();
      
      if( pcPrdDPBUnit == NULL || pcPrdDPBUnit != pcDPBUnitRef )
      {
        RNOK( xCheckForInversePrediction( pcDPBUnitRef, pcUpdDPBUnit, bIntraOnly ) );
      }
    }

    if( rcUpdPicControlData.getCurTemporalLevel() == pcUpdPicSliceHeader->getDecompositionStages() )
    {
      rcUpdPicControlData.setComplete( true ); // HS: that's not the WD !!!!
    }
    else
    {
// - The "check for inverse update process" in subclause S.8.8.2 is invoked with updPic and NULL as inputs.
      RNOK( xCheckForInverseUpdate( pcUpdDPBUnit, NULL, bIntraOnly ) );
    }
  }

  return Err::m_nOK;
}


ErrVal
MCTFDecoder::xInversePrediction( DPBUnit* pcDPBUnit,
                                 Bool     bIntraOnly )
{
  //===== PREDICTION =====
  ControlData&  rcControlData   = pcDPBUnit->getCtrlData();
  IntFrame*     pcFrame         = pcDPBUnit->getFrame   ();
  IntFrame*     pcMCFrame       = m_apcFrameTemp  [0];
  IntFrame*     pcResidual      = m_apcFrameTemp  [2];

  RNOK( pcResidual->copy( pcFrame ) ); // Hanke@RWTH

  //===== reconstruct intra =====
  RNOK( xInitBaseLayerReconstruction( rcControlData ) );
  RNOK( xReconstructIntra           ( pcFrame,
                                      rcControlData.getBaseLayerRec(),
                                      m_apcFrameTemp[0],
                                      m_apcFrameTemp[1],
                                      rcControlData.getMbDataCtrl(),
                                     *rcControlData.getSliceHeader() ) );
  
  if( ! bIntraOnly )
  {
#if 0 // NO_DEBUG
    printf("xInversePrediction(FR%d, TL%d)\n", pcDPBUnit->getPoc(), rcControlData.getCurTemporalLevel());
#endif
    //===== prediction =====
    RNOK( xMotionCompensation         ( pcMCFrame,
                                        &rcControlData.getPrdFrameList( LIST_0 ),
                                        &rcControlData.getPrdFrameList( LIST_1 ),
                                        rcControlData.getMbDataCtrl(),
                                        *rcControlData.getSliceHeader() ) );


    RNOK( pcFrame ->inversePrediction ( pcMCFrame, pcFrame ) );

    //===== de-blocking =====
    // Hanke@RWTH: set pointer to current residual frame
    m_pcLoopFilter->setHighpassFramePointer( pcResidual ); 

    RNOK( m_pcLoopFilter->process     ( *rcControlData.getSliceHeader(),
                                         pcFrame,
                                         rcControlData.getMbDataCtrl (),
                                         rcControlData.getMbDataCtrl (),
                                         m_uiFrameWidthInMb,
                                        &rcControlData.getPrdFrameList( LIST_0 ),
                                        &rcControlData.getPrdFrameList( LIST_1 ) ) );
  }

  //===== update picture in DPB =====
  RNOK( m_pcDecodedPictureBuffer->update( pcDPBUnit, true ) );

  return Err::m_nOK;
}


ErrVal
MCTFDecoder::xInverseUpdate( DPBUnit* pcDPBUnit,
                             Bool     bIntraOnly )
{
  //===== get reference lists =====
  RNOK ( m_pcDecodedPictureBuffer->setUpdRefLists( pcDPBUnit, pcDPBUnit->getCtrlData().getCurTemporalLevel() ) );
  ROTRS( bIntraOnly, Err::m_nOK ); 

  if( pcDPBUnit->getCtrlData().getUpdCtrlList( pcDPBUnit->getCtrlData().getCurTemporalLevel(), LIST_0 ).getSize() ||
      pcDPBUnit->getCtrlData().getUpdCtrlList( pcDPBUnit->getCtrlData().getCurTemporalLevel(), LIST_1 ).getSize()    )
  {
    //===== UPDATE =====
    ControlData&  rcControlData   = pcDPBUnit->getCtrlData();
    SliceHeader*  pcSliceHeader   = rcControlData     .getSliceHeader ();
    MbDataCtrl*   pcMbDataCtrl    = m_cControlDataUpd .getMbDataCtrl  ();
    m_cControlDataUpd.setSliceHeader( pcSliceHeader );
    IntFrame*     pcMCFrame0      = m_apcFrameTemp  [0];
    IntFrame*     pcMCFrame1      = m_apcFrameTemp  [1];
    IntFrame*     pcFrame         = pcDPBUnit->getFrame();
    UInt          uiUpdLevel      = rcControlData.getCurTemporalLevel();
    MbDecoder*    pcMbDecoder     = m_pcSliceDecoder->getMbDecoder();
    
    if( m_bLowComplxUpdFlag )
    {
      pcMbDecoder->setUpdateWeightsBuf( m_pusUpdateWeights );
      pcMbDecoder->setMotCompType     ( UPDT_MC );
    }

#if 0 // NO_DEBUG
    printf("xInverseUpdate    (FR%d, TL%d)\n", pcDPBUnit->getPoc(), rcControlData.getCurTemporalLevel());
#endif

    //===== list 0 motion compensation =====
    RNOK( pcMbDataCtrl->reset() );
    RNOK( pcMbDataCtrl->clear() );
    RNOK( pcMbDataCtrl->deriveUpdateMotionFieldAdaptive ( *pcSliceHeader,
                                                          &rcControlData.getUpdCtrlList( uiUpdLevel, LIST_0 ),
                                                          m_cConnectionArray,
                                                          m_pusUpdateWeights,
                                                          true, LIST_0 ) );
    m_pcQuarterPelFilter->setClipMode                   ( false );
    RNOK( xMotionCompensation                           ( pcMCFrame0,
                                                          &rcControlData.getUpdFrameList( uiUpdLevel, LIST_0 ),
                                                          &rcControlData.getUpdFrameList( uiUpdLevel, LIST_1 ),
                                                          pcMbDataCtrl,
                                                          *pcSliceHeader ) );
    m_pcQuarterPelFilter->setClipMode                   ( true );
    RNOK( pcMCFrame0->adaptiveWeighting                 ( m_pusUpdateWeights, m_bLowComplxUpdFlag ) );

    //===== list 1 motion compensation =====
    RNOK( pcMbDataCtrl->reset() );
    RNOK( pcMbDataCtrl->clear() );
    RNOK( pcMbDataCtrl->deriveUpdateMotionFieldAdaptive ( *pcSliceHeader,
                                                          &rcControlData.getUpdCtrlList( uiUpdLevel, LIST_1 ),
                                                          m_cConnectionArray,
                                                          m_pusUpdateWeights,
                                                          false, LIST_1 ) );
    m_pcQuarterPelFilter->setClipMode                   ( false );
    RNOK( xMotionCompensation                           ( pcMCFrame1,
                                                          &rcControlData.getUpdFrameList( uiUpdLevel, LIST_0 ),
                                                          &rcControlData.getUpdFrameList( uiUpdLevel, LIST_1 ),
                                                          pcMbDataCtrl,
                                                          *pcSliceHeader ) );
    m_pcQuarterPelFilter->setClipMode                   ( true );
    RNOK( pcMCFrame1->adaptiveWeighting                 ( m_pusUpdateWeights, m_bLowComplxUpdFlag ) );

    //===== inverse update =====
    RNOK( pcFrame->inverseUpdate                        ( pcMCFrame0, pcMCFrame1, pcFrame ) );

    //===== update picture in DPB =====
    RNOK( m_pcDecodedPictureBuffer->update( pcDPBUnit ) );

    if( m_bLowComplxUpdFlag )
    {
      pcMbDecoder->setMotCompType(PRED_MC);
    }

    //----- clear slice header reference -----
    m_cControlDataUpd.setSliceHeader( NULL );
  }

  return Err::m_nOK;
}



ErrVal
MCTFDecoder::xReconstructIntra( IntFrame*     pcFrame,
                                IntFrame*     pcBaseLayerRec,
                                IntFrame*     pcBaseRepFrame, // temporary usage
                                IntFrame*     pcPredSignal,   // temporary usage
                                MbDataCtrl*   pcMbDataCtrl,
                                SliceHeader&  rcSH )
{
  Bool            bCalcMv         = false;
  Bool            bFaultTolerant  = false;
  MbDecoder*      pcMbDecoder     = m_pcSliceDecoder->getMbDecoder();


  RNOK( pcMbDataCtrl          ->initSlice( rcSH, PRE_PROCESS, true, NULL ) );
  RNOK( m_pcMotionCompensation->initSlice( rcSH              ) );

  for( UInt uiMbIndex = 0; uiMbIndex < m_uiMbNumber; uiMbIndex++ )
  {
    UInt          uiMbY           = uiMbIndex / m_uiFrameWidthInMb;
    UInt          uiMbX           = uiMbIndex % m_uiFrameWidthInMb;
    MbDataAccess* pcMbDataAccess  = 0;

    RNOK( pcMbDataCtrl            ->initMb( pcMbDataAccess, uiMbY, uiMbX ) );
    RNOK( m_pcYuvFullPelBufferCtrl->initMb(                 uiMbY, uiMbX ) );


    RNOK( pcMbDecoder->reconstructIntraPred( *pcMbDataAccess, pcBaseRepFrame, pcPredSignal, pcBaseLayerRec ) );
    RNOK( pcMbDecoder->reconstructIntra    ( *pcMbDataAccess, pcFrame,        pcPredSignal ) );
  }

  return Err::m_nOK;
}



ErrVal
MCTFDecoder::xMotionCompensation( IntFrame*        pcMCFrame,
                                 RefFrameList*    pcRefFrameList0,
                                 RefFrameList*    pcRefFrameList1,
                                 MbDataCtrl*      pcMbDataCtrl,
                                 SliceHeader&     rcSH )
{
  Bool            bCalcMv         = false;
  Bool            bFaultTolerant  = false;
  MbDecoder*      pcMbDecoder     = m_pcSliceDecoder->getMbDecoder();

  RNOK( pcMbDataCtrl          ->initSlice( rcSH, PRE_PROCESS, true, NULL ) );
  RNOK( m_pcMotionCompensation->initSlice( rcSH              ) );

  for( UInt uiMbIndex = 0; uiMbIndex < m_uiMbNumber; uiMbIndex++ )
  {
    UInt          uiMbY           = uiMbIndex / m_uiFrameWidthInMb;
    UInt          uiMbX           = uiMbIndex % m_uiFrameWidthInMb;
    MbDataAccess* pcMbDataAccess  = 0;


    RNOK( pcMbDataCtrl            ->initMb( pcMbDataAccess, uiMbY, uiMbX                  ) );
    RNOK( m_pcYuvFullPelBufferCtrl->initMb(                 uiMbY, uiMbX ) );
    RNOK( m_pcMotionCompensation  ->initMb(                 uiMbY, uiMbX, *pcMbDataAccess ) );

    RNOK( pcMbDecoder->compensatePrediction( *pcMbDataAccess, pcMCFrame,
                                             *pcRefFrameList0, *pcRefFrameList1,
                                             bCalcMv, bFaultTolerant ) );
  }

  return Err::m_nOK;
}






ErrVal
MCTFDecoder::xCalcMv( SliceHeader*  pcSliceHeader,
                      MbDataCtrl*   pcMbDataCtrl,
                      MbDataCtrl*   pcMbDataCtrlBase )
{
  UInt        uiMbNumber  = m_uiFrameWidthInMb * m_uiFrameHeightInMb;
  MbDecoder*  pcMbDecoder = m_pcSliceDecoder->getMbDecoder();

  RNOK( pcMbDataCtrl          ->initSlice( *pcSliceHeader, PRE_PROCESS, true, NULL ) );
  RNOK( m_pcMotionCompensation->initSlice( *pcSliceHeader              ) );

  for( UInt uiMbIndex = 0; uiMbIndex < uiMbNumber; uiMbIndex++ )
  {
    UInt          uiMbY           = uiMbIndex / m_uiFrameWidthInMb;
    UInt          uiMbX           = uiMbIndex % m_uiFrameWidthInMb;
    MbDataAccess* pcMbDataAccess  = 0;
    MbDataAccess* pcMbDataAccessBase  = 0;

    if( pcMbDataCtrlBase && pcSliceHeader->getAdaptivePredictionFlag() )
    {
      RNOK( pcMbDataCtrlBase->initMb( pcMbDataAccessBase, uiMbY, uiMbX ) );
    }

    //===== init macroblock =====
    RNOK( pcMbDataCtrl          ->initMb( pcMbDataAccess, uiMbY, uiMbX                  ) );
    RNOK( m_pcMotionCompensation->initMb(                 uiMbY, uiMbX, *pcMbDataAccess ) );

    //===== motion compensation for macroblock =====
    RNOK( pcMbDecoder->calcMv( *pcMbDataAccess, pcMbDataAccessBase ) );
  }

  return Err::m_nOK;
}

// TMM_ESS {
ErrVal
DecodedPicBuffer::fillPredictionLists_ESS( ResizeParameters *pcResizeParameters )
{
  pcResizeParameters->initRefListPoc();
  ROTRS( m_pcCurrDPBUnit->getCtrlData().getSliceHeader()->isIntra(),   Err::m_nOK );
  
  Bool  bBaseRep    = m_pcCurrDPBUnit->isKeyPic ();
  Int   iMaxPoc     = m_pcCurrDPBUnit->getPoc   ();
  Int   iMinPoc     = iMaxPoc;
  UInt  uiTempLevel = m_pcCurrDPBUnit->getTLevel();
  UInt  uiSize0      = m_pcCurrDPBUnit->getCtrlData().getSliceHeader()->getNumRefIdxActive( LIST_0 );
  
  Int idx=0;
 
   //===== list 0 =====
  while( uiSize0-- )
  {
    DPBUnit*              pNext = 0;
    DPBUnitList::iterator iter  = m_cUsedDPBUnitList.begin();
    DPBUnitList::iterator end   = m_cUsedDPBUnitList.end  ();
    for( ; iter != end; iter++ )
    {
      if( (*iter)->getPoc() < iMaxPoc &&
          ( !pNext || (*iter)->getPoc() > pNext->getPoc() ) &&
          (*iter)->isBaseRep() == bBaseRep &&
          ( bBaseRep || (*iter)->getTLevel() < uiTempLevel ) )
      {
        pNext = (*iter);
      }
    }
    ROF( pNext );
	iMaxPoc = pNext->getPoc();
	pcResizeParameters->m_aiRefListPoc[0][idx++]=iMaxPoc;  
  }
  
  //===== list 1 =====
  ROTRS( m_pcCurrDPBUnit->getCtrlData().getSliceHeader()->isInterP(),  Err::m_nOK );
  UInt  uiSize1      = m_pcCurrDPBUnit->getCtrlData().getSliceHeader()->getNumRefIdxActive( LIST_1 );
  idx=0;
  while( uiSize1-- )
  {
    DPBUnit*              pNext = 0;
    DPBUnitList::iterator iter  = m_cUsedDPBUnitList.begin();
    DPBUnitList::iterator end   = m_cUsedDPBUnitList.end  ();
    for( ; iter != end; iter++ )
    {
      if( (*iter)->getPoc() > iMinPoc &&
          ( !pNext || (*iter)->getPoc() < pNext->getPoc() ) && 
          (*iter)->isBaseRep() == bBaseRep &&
          ( bBaseRep || (*iter)->getTLevel() < uiTempLevel ) )
      {
        pNext = (*iter);
      }
    }
    ROF( pNext );
	iMinPoc = pNext->getPoc();
    pcResizeParameters->m_aiRefListPoc[1][idx++]=iMinPoc;  
  }

  return Err::m_nOK;
}
// TMM_ESS }

H264AVC_NAMESPACE_END

