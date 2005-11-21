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

#include "QualityLevelEstimation.h"
#include "H264AVCCommonLib.h"
#include "H264AVCCommonLib/CommonDefs.h"




//=============================================================================
//====================                                     ====================
//====================   F G S   P A C K E T   E N T R Y   ====================
//====================                                     ====================
//=============================================================================


const Double FGSPacketEntry::forbiddenDist = -1e300;


FGSPacketEntry::FGSPacketEntry()
: m_uiFrameID         ( MSYS_UINT_MAX )
, m_uiFGSLayer        ( MSYS_UINT_MAX )
, m_uiRate            ( MSYS_UINT_MAX )
, m_dDeltaDistortion  ( forbiddenDist )
, m_uiQualityLevel    ( MSYS_UINT_MAX )
{
}


FGSPacketEntry::~FGSPacketEntry()
{
}


Void
FGSPacketEntry::setQualityLevel( UInt uiQualityLevel )
{
  m_uiQualityLevel = uiQualityLevel;
}


ErrVal
FGSPacketEntry::init( UInt   uiFrameID,
                      UInt   uiFGSLayer,
                      UInt   uiPacketRate,
                      Double dDeltaDistortion )
{
  ROF( m_uiFrameID == MSYS_UINT_MAX );
  if( uiPacketRate )
  {
    m_uiFrameID         = uiFrameID;
    m_uiFGSLayer        = uiFGSLayer;
    m_uiRate            = uiPacketRate;
    m_dDeltaDistortion  = dDeltaDistortion;
    m_uiQualityLevel    = MSYS_UINT_MAX;
  }
  return Err::m_nOK;
}


ErrVal
FGSPacketEntry::uninit()
{
  m_uiFrameID         = MSYS_UINT_MAX;
  m_uiFGSLayer        = MSYS_UINT_MAX;
  m_uiRate            = MSYS_UINT_MAX;
  m_dDeltaDistortion  = forbiddenDist;
  m_uiQualityLevel    = MSYS_UINT_MAX;
  return Err::m_nOK;
}


Bool
FGSPacketEntry::isValid() const
{
  ROTRS( m_uiFrameID        == MSYS_UINT_MAX, false );
  ROTRS( m_uiFGSLayer       == MSYS_UINT_MAX, false );
  ROTRS( m_uiRate           == MSYS_UINT_MAX, false );
  ROTRS( m_dDeltaDistortion == forbiddenDist, false );
  return true;
}






//=======================================================================
//====================                               ====================
//====================   Q U A L I T Y   L A Y E R   ====================
//====================                               ====================
//=======================================================================

const Double QualityLayer::maxCost = 1e300;


QualityLayer::QualityLayer( const QualityLayer& rcQualityLayer )
: m_uiRate            ( rcQualityLayer.m_uiRate )
, m_dDeltaDistortion  ( rcQualityLayer.m_dDeltaDistortion )
, m_dSlope            ( rcQualityLayer.m_dSlope )
, m_cFGSPacketList    ( rcQualityLayer.m_cFGSPacketList )
{
}


QualityLayer::QualityLayer( const FGSPacketEntry& rcFGSPacketEntry )
{
  AOF( rcFGSPacketEntry.isValid() );
  m_uiRate            = rcFGSPacketEntry.m_uiRate;
  m_dDeltaDistortion  = rcFGSPacketEntry.m_dDeltaDistortion;
  m_dSlope            = m_dDeltaDistortion / (Double)m_uiRate;
  m_cFGSPacketList.push_back( const_cast<FGSPacketEntry*>( &rcFGSPacketEntry ) );
}


QualityLayer::QualityLayer( FGSPacketList& rcFGSPacketList )
: m_cFGSPacketList( rcFGSPacketList )
{
  m_uiRate            = 0;
  m_dDeltaDistortion  = 0;
  for( FGSPacketList::iterator iter = m_cFGSPacketList.begin(); iter != m_cFGSPacketList.end(); iter++ )
  {
    FGSPacketEntry*       pcFGSPacketEntry  = *iter;
    m_uiRate           += pcFGSPacketEntry->m_uiRate;
    m_dDeltaDistortion += pcFGSPacketEntry->m_dDeltaDistortion;
  }
  m_dSlope = m_dDeltaDistortion / (Double)m_uiRate;
}


QualityLayer::~QualityLayer()
{
  m_cFGSPacketList.clear();
}


FGSPacketList&
QualityLayer::getFGSPacketList()
{
  return m_cFGSPacketList;
}


Void
QualityLayer::add( FGSPacketList& rcFGSPacketList )
{
  for( FGSPacketList::iterator iter = rcFGSPacketList.begin(); iter != rcFGSPacketList.end(); iter++ )
  {
    FGSPacketEntry*        pcFGSPacketEntry  = *iter;
    m_uiRate            += pcFGSPacketEntry->m_uiRate;
    m_dDeltaDistortion  += pcFGSPacketEntry->m_dDeltaDistortion;
    m_cFGSPacketList.push_back( pcFGSPacketEntry );
  }
  m_dSlope = m_dDeltaDistortion / (Double)m_uiRate;
}


Void
QualityLayer::remove( FGSPacketList& rcFGSPacketList )
{
  for( FGSPacketList::iterator iter = rcFGSPacketList.begin(); iter != rcFGSPacketList.end(); iter++ )
  {
    FGSPacketEntry*        pcFGSPacketEntry  = *iter;
    m_uiRate            -= pcFGSPacketEntry->m_uiRate;
    m_dDeltaDistortion  -= pcFGSPacketEntry->m_dDeltaDistortion;
    m_cFGSPacketList.remove( pcFGSPacketEntry );
  }
  m_dSlope = m_dDeltaDistortion / (Double)m_uiRate;
}


Double
QualityLayer::getSeperateArea( const QualityLayer& rcQualityLayer )
{
  Double dArea  =                m_dDeltaDistortion * (Double)               m_uiRate / 2.0;
  dArea        +=                m_dDeltaDistortion * (Double)rcQualityLayer.m_uiRate;
  dArea        += rcQualityLayer.m_dDeltaDistortion * (Double)rcQualityLayer.m_uiRate / 2.0;
  return dArea;
}


Double
QualityLayer::getCombinedArea( const QualityLayer& rcQualityLayer )
{
  Double dRate = m_uiRate           + rcQualityLayer.m_uiRate;
  Double dDist = m_dDeltaDistortion + rcQualityLayer.m_dDeltaDistortion;
  Double dArea = dRate * dDist / 2.0;
  return dArea;
}


Void
QualityLayer::merge( const QualityLayer& rcQualityLayer )
{
  m_uiRate            += rcQualityLayer.m_uiRate;
  m_dDeltaDistortion  += rcQualityLayer.m_dDeltaDistortion;
  m_dSlope             = m_dDeltaDistortion / (Double)m_uiRate;
  m_cFGSPacketList    += rcQualityLayer.m_cFGSPacketList;
}


Void
QualityLayer::conditionedMerge( QualityLayer& rcQualityLayer )
{
  FGSPacketList     cElementsForMerge;
  FGSPacketList&    rcNextPacketList = rcQualityLayer.getFGSPacketList();

  //===== get packets that can be merged =====
  for( FGSPacketList::iterator iter = rcNextPacketList.begin(); iter != rcNextPacketList.end(); iter++ )
  {
    UInt  uiFGSLayer  = (*iter)->getFGSLayer() - 1;
    UInt  uiFrameID   = (*iter)->getFrameID ();
    Bool  bOK         = ( uiFGSLayer == 0 );
    if( ! bOK )
    {
      bOK = true;
      for( FGSPacketList::iterator baseIter = m_cFGSPacketList.begin(); baseIter != m_cFGSPacketList.end(); baseIter++ )
      {
        if( (*baseIter)->getFrameID() == uiFrameID && (*baseIter)->getFGSLayer() == uiFGSLayer )
        {
          bOK = false;
          break;
        }
      }
    }

    if( bOK )
    {
      cElementsForMerge.push_back( *iter );
    }
  }

  this         ->add   ( cElementsForMerge );
  rcQualityLayer.remove( cElementsForMerge );
}



Bool
QualityLayer::isMergingPossible( QualityLayer&  rcQualityLayer,
                                 Double&        dMergeCost )
{
  FGSPacketList     cElementsThatCannotBeMerged;
  FGSPacketList&    rcNextPacketList = rcQualityLayer.getFGSPacketList();
  dMergeCost                         = maxCost;

  //===== check for packets that cannot be merged =====
  for( FGSPacketList::iterator iter = rcNextPacketList.begin(); iter != rcNextPacketList.end(); iter++ )
  {
    UInt  uiFGSLayer  = (*iter)->getFGSLayer() - 1;
    UInt  uiFrameID   = (*iter)->getFrameID ();
    Bool  bOK         = ( uiFGSLayer == 0 );
    if( ! bOK )
    {
      bOK = true;
      for( FGSPacketList::iterator baseIter = m_cFGSPacketList.begin(); baseIter != m_cFGSPacketList.end(); baseIter++ )
      {
        if( (*baseIter)->getFrameID() == uiFrameID && (*baseIter)->getFGSLayer() == uiFGSLayer )
        {
          bOK = false;
          break;
        }
      }
    }

    if( ! bOK )
    {
      cElementsThatCannotBeMerged.push_back( *iter );
    }
  }
  ROFRS( rcQualityLayer.getFGSPacketList().size() > cElementsThatCannotBeMerged.size(), false );

  //===== determine costs for merging =====
  dMergeCost = getSeperateArea( rcQualityLayer );

  if( cElementsThatCannotBeMerged.empty() )
  {
    dMergeCost -= getCombinedArea( rcQualityLayer ); 
  }
  else
  {
    FGSPacketList cElementsThatCanBeMerged( rcQualityLayer.getFGSPacketList() );
    for( FGSPacketList::iterator iter = cElementsThatCannotBeMerged.begin(); iter != cElementsThatCannotBeMerged.end(); iter++ )
    {
      cElementsThatCanBeMerged.remove( *iter );
    }
    QualityLayer  cCurrentQualityLayer      ( *this );
    QualityLayer  cNextQualityLayerMerged   ( cElementsThatCanBeMerged );
    QualityLayer  cNextQualityLayerNotMerged( cElementsThatCannotBeMerged );
    cCurrentQualityLayer.merge( cNextQualityLayerMerged );

    dMergeCost -= cCurrentQualityLayer.getSeperateArea( cNextQualityLayerNotMerged );
  }

  return true;
}








//=============================================================================================
//====================                                                     ====================
//====================   Q U A L I T Y   L E V E L   E S T I M A T I O N   ====================
//====================                                                     ====================
//=============================================================================================


QualityLevelEstimation::QualityLevelEstimation()
: m_uiNumFGSPackets ( 0 )
, m_uiNumFrames     ( 0 ) 
{
  ::memset( m_aacFGSPacketEntry, 0x00, MAX_QUALITY_LEVELS*sizeof(Void*) );
}


QualityLevelEstimation::~QualityLevelEstimation()
{
  ANOK( uninit() );
}


ErrVal
QualityLevelEstimation::init( UInt uiNumFGSLayers,
                              UInt uiNumFrames )
{
  m_uiNumFGSPackets = uiNumFGSLayers;
  m_uiNumFrames     = uiNumFrames;
  for( UInt uiFGSLayer = 0; uiFGSLayer < m_uiNumFGSPackets; uiFGSLayer++ )
  {
    ROFRS( ( m_aacFGSPacketEntry[uiFGSLayer] = new FGSPacketEntry [m_uiNumFrames] ), Err::m_nOK );
  }
  return Err::m_nOK;
}


ErrVal
QualityLevelEstimation::uninit()
{
  for( UInt uiFGSLayer = 0; uiFGSLayer < MAX_QUALITY_LEVELS; uiFGSLayer++ )
  {
    delete [] m_aacFGSPacketEntry[uiFGSLayer];  m_aacFGSPacketEntry[uiFGSLayer] = 0;
  }
  m_uiNumFGSPackets = 0;
  m_uiNumFrames     = 0;
  return Err::m_nOK;
}


ErrVal
QualityLevelEstimation::addPacket( UInt    uiFGSLayer,
                                   UInt    uiFrameNumInCodingOrder,
                                   UInt    uiPacketSize,
                                   Double  dDeltaDistortion )
{
  ROF( uiFGSLayer              < m_uiNumFGSPackets );
  ROF( uiFrameNumInCodingOrder < m_uiNumFrames     );

  RNOK( m_aacFGSPacketEntry[uiFGSLayer][uiFrameNumInCodingOrder].init( uiFrameNumInCodingOrder,
                                                                       uiFGSLayer,
                                                                       uiPacketSize,
                                                                       dDeltaDistortion ) );
  return Err::m_nOK;
}


ErrVal
QualityLevelEstimation::optimizeQualityLevel( UInt uiMinLevel,
                                              UInt uiMaxLevel )
{
  ROT( uiMaxLevel - uiMinLevel + 1 < 3 );

  UInt              uiMaxNumQualityLayers = uiMaxLevel - uiMinLevel + 1;
  QualityLayerList  cQualityLayerList;

  //===== get initial quality layer list =====
  {
    //----- put all valid packets into list -----
    for( UInt uiFGSLayer  = 0; uiFGSLayer < m_uiNumFGSPackets; uiFGSLayer ++ )
    for( UInt uiFrame     = 0; uiFrame    < m_uiNumFrames;     uiFrame    ++ )
    {
      if( m_aacFGSPacketEntry[uiFGSLayer][uiFrame].isValid() )
      {
        cQualityLayerList.push_back( m_aacFGSPacketEntry[uiFGSLayer][uiFrame] );
      }
    }
  }
  //----- sort list -----
  cQualityLayerList.sort( std::greater<QualityLayer>() );
  //----- make sure that FGSLayers are in the right order -----
  {
    for( UInt uiFGSLayer = 1; uiFGSLayer < m_uiNumFGSPackets; uiFGSLayer++ )
    {
      QualityLayerList::iterator  iter  = cQualityLayerList.begin ();
      QualityLayerList::iterator  iend  = cQualityLayerList.end   ();
      for( ; iter != iend; )
      {
        if( (*(iter->getFGSPacketList().begin()))->getFGSLayer() == uiFGSLayer )
        {
          UInt  uiFrame = (*(iter->getFGSPacketList().begin()))->getFrameID();
          QualityLayerList::iterator  iterParent = iter;
          for( iterParent++; iterParent != iend; iterParent++ )
          {
            if( (*(iterParent->getFGSPacketList().begin()))->getFrameID () == uiFrame &&
                (*(iterParent->getFGSPacketList().begin()))->getFGSLayer() == uiFGSLayer - 1 )
            {
              break;
            }
          }
          if( iterParent != iend )
          {
            iterParent++; cQualityLayerList.insert( iterParent, *iter );
            QualityLayerList::iterator inext = iter;  inext++;
            cQualityLayerList.erase( iter );
            iter = inext;
          }
          else
          {
            iter++;
          }
        }
        else
        {
          iter++;
        }
      }
    }
  }


  //===== lowest cost merging =====
  while( true )
  {
    //===== get minimum merge cost (without considerung non-mergeable elements) =====
    Double                      dMinMergeCost     = QualityLayer::maxCost;
    QualityLayerList::iterator  cMinMergeCostIter = cQualityLayerList.begin();
    for( QualityLayerList::iterator currIter = cQualityLayerList.begin(); currIter != cQualityLayerList.end(); currIter++ )
    {
      QualityLayerList::iterator inext      = currIter; inext++;
      Double                     dMergeCost = dMinMergeCost;
      if( inext != cQualityLayerList.end() && currIter->isMergingPossible( *inext, dMergeCost ) )
      {
        if( dMergeCost < dMinMergeCost )
        {
          dMinMergeCost     = dMergeCost;
          cMinMergeCostIter = currIter;
        }
      }
    }

    //===== check for finish =====
    if( dMinMergeCost >= 0 && cQualityLayerList.size() <= uiMaxNumQualityLayers )
    {
      break;
    }
    ROT( dMinMergeCost == QualityLayer::maxCost );

    //===== merge =====
    QualityLayerList::iterator  inext = cMinMergeCostIter; inext++;
    cMinMergeCostIter->conditionedMerge( *inext );
    if( inext->getFGSPacketList().empty() )
    {
      cQualityLayerList.erase( inext );
    }
  }

  //===== assign quality layers to FGS packets =====
  UInt uiQLayer = uiMaxLevel;
  for( QualityLayerList::iterator qiter = cQualityLayerList.begin(); qiter != cQualityLayerList.end(); qiter++, uiQLayer-- )
  {
    FGSPacketList& rcFGSPacketList = qiter->getFGSPacketList();
    for( FGSPacketList::iterator piter = rcFGSPacketList.begin(); piter != rcFGSPacketList.end(); piter++ )
    {
      (*piter)->setQualityLevel( uiQLayer );
    }
  }

  return Err::m_nOK;
}


UInt
QualityLevelEstimation::getQualityLevel( UInt uiFGSLayer,
                                         UInt uiFrameNumInCodingOrder ) const
{
  ROF( uiFGSLayer              < m_uiNumFGSPackets );
  ROF( uiFrameNumInCodingOrder < m_uiNumFrames     );

  return m_aacFGSPacketEntry[uiFGSLayer][uiFrameNumInCodingOrder].getQualityLevel();
}