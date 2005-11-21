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


#ifndef _QUALITY_LEVEL_ESTIMATION_H
#define _QUALITY_LEVEL_ESTIMATION_H


#include "Typedefs.h"
#include "H264AVCCommonLib.h"




class FGSPacketEntry;
class QualityLayer;
typedef MyList<QualityLayer>    QualityLayerList;
typedef MyList<FGSPacketEntry*> FGSPacketList;




class FGSPacketEntry
{
  friend class QualityLayer;

protected:
  static const Double forbiddenDist;
  
public:
  FGSPacketEntry ();
  ~FGSPacketEntry();

  Void    setQualityLevel ( UInt    uiQualityLevel );
  ErrVal  init            ( UInt    uiFrameID,
                            UInt    uiFGSLayer,
                            UInt    uiPacketRate,
                            Double  dDeltaDistortion );
  ErrVal  uninit          ();
  
  Bool    isValid         () const;
  UInt    getQualityLevel () const  { return m_uiQualityLevel; }
  UInt    getFGSLayer     () const  { return m_uiFGSLayer; }
  UInt    getFrameID      () const  { return m_uiFrameID; }

protected:
  UInt    m_uiFrameID;
  UInt    m_uiFGSLayer;
  UInt    m_uiRate;
  Double  m_dDeltaDistortion;
  UInt    m_uiQualityLevel;
};





class QualityLayer
{
public:
  static const Double maxCost;
  
public:
  QualityLayer  ( const QualityLayer&   rcQualityLayer   );
  QualityLayer  ( const FGSPacketEntry& rcFGSPacketEntry );
  QualityLayer  ( FGSPacketList&        rcFGSPacketList  );
  ~QualityLayer ();

  FGSPacketList&  getFGSPacketList  ();
  Void            add               ( FGSPacketList&      rcFGSPacketList );
  Void            remove            ( FGSPacketList&      rcFGSPacketList );
  Double          getSeperateArea   ( const QualityLayer& rcQualityLayer );
  Double          getCombinedArea   ( const QualityLayer& rcQualityLayer );
  Void            merge             ( const QualityLayer& rcQualityLayer );

  Void            conditionedMerge  (       QualityLayer& rcQualityLayer );
  Bool            isMergingPossible (       QualityLayer& rcQualityLayer,
                                            Double&       dMergeCost );

public:
  Double  getSlope    ()                          const { return m_dSlope; }
  Bool    operator >  ( const QualityLayer& ql1 ) const { return ql1.getSlope() < getSlope(); } // operator for sorting

protected:
  UInt          m_uiRate;
  Double        m_dDeltaDistortion;
  Double        m_dSlope;
  FGSPacketList m_cFGSPacketList;
};




class QualityLevelEstimation
{
public:
  QualityLevelEstimation  ();
  ~QualityLevelEstimation ();

  ErrVal  init                  ( UInt    uiNumFGSLayers,
                                  UInt    uiNumFrames );
  ErrVal  uninit                ();

  ErrVal  addPacket             ( UInt    uiFGSLayer,
                                  UInt    uiFrameNumInCodingOrder,
                                  UInt    uiPacketSize,
                                  Double  dDeltaDistortion );
  ErrVal  optimizeQualityLevel  ( UInt    uiMinLevel,
                                  UInt    uiMaxLevel );
  UInt    getQualityLevel       ( UInt    uiFGSLayer,
                                  UInt    uiFrameNumInCodingOrder ) const;

private:
  UInt              m_uiNumFGSPackets;
  UInt              m_uiNumFrames;
  FGSPacketEntry*   m_aacFGSPacketEntry[MAX_QUALITY_LEVELS];
};



#endif // _QUALITY_LEVEL_ESTIMATION_H
