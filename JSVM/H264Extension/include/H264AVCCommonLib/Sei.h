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



#if !defined(AFX_SEI_H__06FFFAD0_FB36_4BF0_9392_395C7389C1F4__INCLUDED_)
#define AFX_SEI_H__06FFFAD0_FB36_4BF0_9392_395C7389C1F4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



#include "H264AVCCommonLib/CommonBuffers.h"
#include "H264AVCCommonLib/HeaderSymbolReadIf.h"
#include "H264AVCCommonLib/HeaderSymbolWriteIf.h"
#include <list>



H264AVC_NAMESPACE_BEGIN



class ParameterSetMng;


#if defined( WIN32 )
# pragma warning( disable: 4251 )
#endif


class H264AVCCOMMONLIB_API SEI
{

public:
  enum MessageType
  {
    SUB_SEQ_INFO                          = 10,
    SCALABLE_SEI                          = 19,
    //{{Quality level estimation and modified truncation- JVTO044 and m12007
    //France Telecom R&D-(nathalie.cammas@francetelecom.com)
    QUALITYLEVEL_SEI                      = 20,
	DEADSUBSTREAM_SEI                     = 21,
    RESERVED_SEI                          = 22
    //}}Quality level estimation and modified truncation- JVTO044 and m12007
  };


  class H264AVCCOMMONLIB_API SEIMessage
  {
  public:
    virtual ~SEIMessage()                                                       {}
    MessageType     getMessageType()                                      const { return m_eMessageType; }
    virtual ErrVal  write         ( HeaderSymbolWriteIf* pcWriteIf ) = 0;
    virtual ErrVal  read          ( HeaderSymbolReadIf*   pcReadIf ) = 0;

  protected:
    SEIMessage( MessageType eMessageType) : m_eMessageType( eMessageType ) {}

  private:
    MessageType m_eMessageType;
  };



  class H264AVCCOMMONLIB_API ReservedSei : public SEIMessage
  {
  protected:
    ReservedSei( UInt uiSize = 0 ) : SEIMessage(RESERVED_SEI), m_uiSize(uiSize) {}

  public:
    static ErrVal create( ReservedSei*&         rpcReservedSei,
                          UInt                  uiSize );
    ErrVal        write ( HeaderSymbolWriteIf*  pcWriteIf );
    ErrVal        read  ( HeaderSymbolReadIf*   pcReadIf );
  
  private:
    UInt m_uiSize;
  };


  class H264AVCCOMMONLIB_API SubSeqInfo : public SEIMessage
  {
  protected:
    SubSeqInfo()
      : SEIMessage(SUB_SEQ_INFO)
      , m_uiSubSeqLayerNum      (0)
	    , m_uiSubSeqId            (0)
	    , m_bFirstRefPicFlag      (false)
	    , m_bLeadingNonRefPicFlag (false)
	    , m_bLastPicFlag          (false)
	    , m_bSubSeqFrameNumFlag   (false)
      , m_uiSubSeqFrameNum      (0)
    {}

  public:
    static ErrVal create( SubSeqInfo*&          rpcSEIMessage );
    ErrVal        write ( HeaderSymbolWriteIf*  pcWriteIf );
    ErrVal        read  ( HeaderSymbolReadIf*   pcReadIf );
    ErrVal        init  ( UInt                  uiSubSeqLayerNum,
	                        UInt                  uiSubSeqId,
	                        Bool                  bFirstRefPicFlag,
	                        Bool                  bLeadingNonRefPicFlag,
	                        Bool                  bLastPicFlag        = false,
	                        Bool                  bSubSeqFrameNumFlag = false,
                          UInt                  uiSubSeqFrameNum    = 0 );

    UInt getSubSeqId      ()  const { return m_uiSubSeqId; }
    UInt getSubSeqLayerNum()  const { return m_uiSubSeqLayerNum; }

  private:
	  UInt  m_uiSubSeqLayerNum;
	  UInt  m_uiSubSeqId;
	  Bool  m_bFirstRefPicFlag;
	  Bool  m_bLeadingNonRefPicFlag;
	  Bool  m_bLastPicFlag;
	  Bool  m_bSubSeqFrameNumFlag;
    UInt  m_uiSubSeqFrameNum;
  };


  class H264AVCCOMMONLIB_API ScalableSei : public SEIMessage
  {
  protected:
    ScalableSei ();
    ~ScalableSei();

  public:
    static ErrVal create  ( ScalableSei*&         rpcSeiMessage );
    ErrVal        write   ( HeaderSymbolWriteIf*  pcWriteIf );
    ErrVal        read    ( HeaderSymbolReadIf*   pcReadIf );
  
    UInt  getMaxHorFrameDimInMB       ()                const { return m_uiMaxHorFrameDimInMB; }
    UInt  getMaxVerFrameDimInMB       ()                const { return m_uiMaxVerFrameDimInMB; }
    UInt  getFrameRateUnitNom         ()                const { return m_uiFrameRateUnitNom; }
    UInt  getFrameRateUnitDenom       ()                const { return m_uiFrameRateUnitDenom; }
    UInt  getMaxDecStages             ()                const { return m_uiMaxDecStages; }
    UInt  getNumLayers                ()                const { return m_uiNumLayers; }
    Bool  getBaseLayerIsAVC           ()                const { return m_bBaseLayerIsAVC; }
    UInt  getAVCTempResStages         ()                const { return m_uiAVCTempResStages; }
    UInt  getSpatialResolutionFactor  ( UInt uiLayer )  const { return m_uiSpatialResolutionFactor  [uiLayer]; }
    UInt  getTemporalResolutionFactor ( UInt uiLayer )  const { return m_uiTemporalResolutionFactor [uiLayer]; }

    Void  setMaxHorFrameDimInMB       ( UInt ui )                { m_uiMaxHorFrameDimInMB = ui; }
    Void  setMaxVerFrameDimInMB       ( UInt ui )                { m_uiMaxVerFrameDimInMB = ui; }
    Void  setFrameRateUnitNom         ( UInt ui )                { m_uiFrameRateUnitNom = ui; }
    Void  setFrameRateUnitDenom       ( UInt ui )                { m_uiFrameRateUnitDenom = ui; }
    Void  setMaxDecStages             ( UInt ui )                { m_uiMaxDecStages = ui; }
    Void  setNumLayers                ( UInt ui )                { m_uiNumLayers = ui; }
    Void  setBaseLayerIsAVC           ( Bool b  )                { m_bBaseLayerIsAVC = b; }
    Void  setAVCTempResStages         ( UInt ui )                { m_uiAVCTempResStages = ui; }
    Void  setSpatialResolutionFactor  ( UInt uiLayer, UInt ui )  { m_uiSpatialResolutionFactor  [uiLayer] = ui; }
    Void  setTemporalResolutionFactor ( UInt uiLayer, UInt ui )  { m_uiTemporalResolutionFactor [uiLayer] = ui; }

  private:
    UInt  m_uiMaxHorFrameDimInMB;
    UInt  m_uiMaxVerFrameDimInMB;
    UInt  m_uiFrameRateUnitNom;
    UInt  m_uiFrameRateUnitDenom;
    UInt  m_uiMaxDecStages;
    UInt  m_uiNumLayers;
    Bool  m_bBaseLayerIsAVC;
    UInt  m_uiAVCTempResStages;
    UInt  m_uiSpatialResolutionFactor   [MAX_LAYERS];
    UInt  m_uiTemporalResolutionFactor  [MAX_LAYERS];
  };
  
  //{{Quality level estimation and modified truncation- JVTO044 and m12007
  //France Telecom R&D-(nathalie.cammas@francetelecom.com)
  class H264AVCCOMMONLIB_API QualityLevelSEI : public SEIMessage
  {
	protected:
    QualityLevelSEI ();
    ~QualityLevelSEI();

  public:
    static ErrVal create  ( QualityLevelSEI*&         rpcSeiMessage );
    ErrVal        write   ( HeaderSymbolWriteIf*  pcWriteIf );
    ErrVal        read    ( HeaderSymbolReadIf*   pcReadIf );
	
	UInt		 getNumLevel() { return m_uiNumLevels;}
	Void		 setNumLevel(UInt ui) { m_uiNumLevels = ui;}
	UInt		 getDeltaBytesRateOfLevel(UInt ui) { return m_auiDeltaBytesRateOfLevel[ui];}
	Void		 setDeltaBytesRateOfLevel(UInt uiIndex, UInt ui) { m_auiDeltaBytesRateOfLevel[uiIndex] = ui;}
	UInt		 getQualityLevel(UInt ui) { return m_auiQualityLevel[ui];}
	Void		 setQualityLevel(UInt uiIndex, UInt ui) { m_auiQualityLevel[uiIndex] = ui;}
	UInt		 getDependencyId() { return m_uiDependencyId;}
	Void		 setDependencyId( UInt ui) { m_uiDependencyId = ui;}

  private:
	  UInt m_auiQualityLevel[MAX_NUM_RD_LEVELS];
	  UInt m_auiDeltaBytesRateOfLevel[MAX_NUM_RD_LEVELS];
	  UInt m_uiNumLevels;
	  UInt m_uiDependencyId;
  };

  class H264AVCCOMMONLIB_API DeadSubstreamSEI : public SEIMessage
  {
	protected:
    DeadSubstreamSEI ();
    ~DeadSubstreamSEI();

  public:
    static ErrVal create  ( DeadSubstreamSEI*&         rpcSeiMessage );
    ErrVal        write   ( HeaderSymbolWriteIf*  pcWriteIf );
    ErrVal        read    ( HeaderSymbolReadIf*   pcReadIf );
	
	UInt		 getDeltaBytesDeadSubstream() { return m_uiDeltaBytesDeadSubstream;}
	Void		 setDeltaBytesDeadSubstream(UInt ui) { m_uiDeltaBytesDeadSubstream = ui;}
	UInt		 getDependencyId() { return m_uiDependencyId;}
	Void		 setDependencyId( UInt ui) { m_uiDependencyId = ui;}

  private:
	  UInt m_uiDeltaBytesDeadSubstream;
	  UInt m_uiDependencyId;
  };
  //}}Quality level estimation and modified truncation- JVTO044 and m12007


  typedef MyList<SEIMessage*> MessageList;
  
  static ErrVal read  ( HeaderSymbolReadIf*   pcReadIf,
                        MessageList&          rcSEIMessageList );
  static ErrVal write ( HeaderSymbolWriteIf*  pcWriteIf,
                        HeaderSymbolWriteIf*  pcWriteTestIf,
                        MessageList*          rpcSEIMessageList );

protected:
  static ErrVal xRead               ( HeaderSymbolReadIf*   pcReadIf,
                                      SEIMessage*&          rpcSEIMessage ); 
  static ErrVal xWrite              ( HeaderSymbolWriteIf*  pcWriteIf,
                                      HeaderSymbolWriteIf*  pcWriteTestIf,
                                      SEIMessage*           pcSEIMessage );
  static ErrVal xWritePayloadHeader ( HeaderSymbolWriteIf*  pcWriteIf,
                                      MessageType           eMessageType,
                                      UInt                  uiSize );
  static ErrVal xReadPayloadHeader  ( HeaderSymbolReadIf*   pcReadIf,
                                      MessageType&          reMessageType,
                                      UInt&                 ruiSize);
  static ErrVal xCreate             ( SEIMessage*&          rpcSEIMessage,
                                      MessageType           eMessageType,
                                      UInt                  uiSize ); 
public:


};

#if defined( WIN32 )
# pragma warning( default: 4251 )
#endif

H264AVC_NAMESPACE_END


#endif // !defined(AFX_SEI_H__06FFFAD0_FB36_4BF0_9392_395C7389C1F4__INCLUDED_)
