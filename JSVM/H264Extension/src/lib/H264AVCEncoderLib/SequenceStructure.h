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

#if !defined(AFX_SEQUENCESTRUCTURE_H__268768B8_4D1D_484A_904E_586985833BAC__INCLUDED_)
#define AFX_SEQUENCESTRUCTURE_H__268768B8_4D1D_484A_904E_586985833BAC__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


typedef std::string String;  


H264AVC_NAMESPACE_BEGIN


class FrameSpec
{
public:
  FrameSpec  ();
  ~FrameSpec ();

  Void              uninit            ();
  ErrVal            init              ( UChar       ucType,
                                        UInt        uiContFrameNum,
                                        Bool        bAnchor,
                                        UInt        uiFramesSkipped,
                                        Bool        bKeyPicture,
                                        UInt        uiLayer,
                                        MmcoBuffer* pcMmcoBuf,
                                        RplrBuffer* pcRplrBufL0,
                                        RplrBuffer* pcRplrBufL1 );

  Bool              isInitialized     ()                  const;
  UInt              getContFrameNumber()                  const;
  SliceType         getSliceType      ()                  const;
  NalUnitType       getNalUnitType    ()                  const;
  NalRefIdc         getNalRefIdc      ()                  const;
  Bool              isSkipped         ()                  const;
  Bool              isKeyPicture      ()                  const;
  Bool              isAnchor          ()                  const;
  UInt              getFramesSkipped  ()                  const;
  UInt              getTemporalLayer  ()                  const;
  const MmcoBuffer* getMmcoBuf        ()                  const;
  const RplrBuffer* getRplrBuf        ( ListIdx eLstIdx)  const;

private:
  Bool          m_bInit;
  UInt          m_uiContFrameNumber;
  SliceType     m_eSliceType;
  NalUnitType   m_eNalUnitType;
  NalRefIdc     m_eNalRefIdc;
  Bool          m_bKeyPicture;
  Bool          m_bAnchor;
  UInt          m_uiFramesSkipped;
  UInt          m_uiTemporalLayer;
  MmcoBuffer*   m_pcMmcoBuf; 
  RplrBuffer*   m_apcRplrBuf[2]; 
};



class  FormattedStringParser
{
public:
  static  ErrVal  separatString               ( const String&   rcString, 
                                                String&         rcFDString, 
                                                String&         rcMmcoString, 
                                                String&         rcRplrStringL0,
                                                String&         rcRplrStringL1 );
  static  ErrVal  extractRplr                 ( const String&   rcString,
                                                RplrBuffer&     rcRplrBuf); 
  static  ErrVal  extractMmco                 ( const String&   rcString,
                                                MmcoBuffer&     rcMmcoBuf );
  static  ErrVal  extractSingleRplrCommand   ( const String&   rcString,
                                                Rplr&           rcRplr );
  static  ErrVal  extractSingleMmcoCommand    ( const String&   rcString,
                                                Mmco&           rcMmco );
  static  ErrVal  extractFrameDescription     ( const String&   rcString,
                                                UChar&          rucType,
                                                UInt&           ruiIncrement,
                                                Bool&           rbKeyPicture,
                                                UInt&           ruiLayer );
  static  Bool    isFrameSequencePart         ( const String&   rcString );
  static  ErrVal  extractRepetitions          ( const String&   rcString,
                                                String&         rcNoRepString,
                                                UInt&           ruiNumberOfRepetitions );
  static  ErrVal  getNumberOfFrames           ( const String&   rcString,
                                                UInt&           ruiNumberOfFrames );
  static  ErrVal  extractNextFrameDescription ( const String&   rcString,
                                                String&         rcFDString,
                                                UInt&           ruiStartPos );
  static  ErrVal  getNumberOfParts            ( const String&   rcString,
                                                UInt&           ruiNumberOfParts );
  static  ErrVal  extractPart                 ( const String&   rcString,
                                                String&         rcPartString,
                                                UInt&           ruiStartPos );

private:
  static const String sm_cSetOfTypes;
  static const String sm_cSetOfDigits;
  static const String sm_cSetOfPartStart;
};




class SequenceStructure  
{
private:

  class FrameDescriptor
  {
  public:
    FrameDescriptor             ();
    ~FrameDescriptor            ();

    Void    uninit              ();
    ErrVal  init                ( const String&   rcString,
                                  UInt            uiLastAnchorFrameNumIncrement );
    ErrVal  reduceFramesSkipped ( UInt            uiNotCoded );

    ErrVal  check               ()  const;
    Bool    isIDR               ()  const;
    Bool    isSkipped           ()  const;
    Bool    isCoded             ()  const;
    Bool    isReference         ()  const;
    Bool    isAnchor            ()  const;
    Bool    isKeyPicture        ()  const;
    UInt    getIncrement        ()  const;
    UInt    getFramesSkipped    ()  const;

    ErrVal  setFrameSpec        ( FrameSpec&      rcFrameSpec,
                                  UInt            uiFrameNumOffset ) const;

  private:
    Bool        m_bInit;
    UChar       m_ucType;
    UInt        m_uiFrameNumIncrement;
    Bool        m_bAnchor;
    UInt        m_uiFramesSkipped;
    Bool        m_bKeyPicture;
    UInt        m_uiLayer;
    MmcoBuffer* m_pcMmcoBuf; 
    RplrBuffer* m_apcRplrBuf[2]; 
  };


  class SequencePart
  {
  public:
    SequencePart                        ()  {}
    virtual ~SequencePart               ()  {}

    virtual Void    uninit              ()                                      = 0;
    virtual ErrVal  init                ( const String& rcString)               = 0;
    virtual Void    reset               ()                                      = 0;
    virtual ErrVal  check               ()                                      = 0;

    virtual Bool    isFirstIDR          ()  const                               = 0;
    virtual UInt    getMinDPBSizeRef    ()  const                               = 0;
    virtual UInt    getMinDPBSizeNonRef ()  const                               = 0;

    virtual Bool    getNextFrameSpec    ( FrameSpec&    rcFrameSpec,
                                          UInt&         uiFrameNumPartOffset )  = 0;
  };


  class FrameSequencePart : public SequencePart
  {
  public:
    FrameSequencePart             ();
    virtual ~FrameSequencePart    ();

    Void    uninit                ();
    ErrVal  init                  ( const String& rcString );
    Void    reset                 ();
    ErrVal  check                 ();

    Bool    isFirstIDR            ()  const;
    UInt    getMinDPBSizeRef      ()  const;
    UInt    getMinDPBSizeNonRef   ()  const;
    
    Bool    getNextFrameSpec      ( FrameSpec&    rcFrameSpec,
                                    UInt&         uiFrameNumPartOffset );

  private:
    Bool              m_bInit;
    FrameDescriptor*  m_pacFrameDescriptor;
    UInt              m_uiNumberOfFrames;
    UInt              m_uiCurrentFrame;
    UInt              m_uiNumberOfRepetitions;
    UInt              m_uiCurrentRepetition;
    UInt              m_uiMinDPBSizeRef;
    UInt              m_uiMinDPBSizeNonRef;
  };


  class GeneralSequencePart : public SequencePart
  {
  public:
    GeneralSequencePart           ();
    virtual ~GeneralSequencePart  ();

    Void    uninit                ();    
    ErrVal  init                  ( const String& rcString );
    Void    reset                 ();
    ErrVal  check                 ();

    Bool    isFirstIDR            ()  const;
    UInt    getMinDPBSizeRef      ()  const;
    UInt    getMinDPBSizeNonRef   ()  const;

    Bool    getNextFrameSpec      ( FrameSpec&    rcFrameSpec,
                                    UInt&         uiFrameNumPartOffset );

  private:
    Bool              m_bInit;
    SequencePart**    m_papcSequencePart;
    UInt              m_uiNumberOfParts;
    UInt              m_uiCurrentPart;
    UInt              m_uiNumberOfRepetitions;
    UInt              m_uiCurrentRepetition;
    UInt              m_uiMinDPBSizeRef;
    UInt              m_uiMinDPBSizeNonRef;
  };


protected:
  SequenceStructure                 ();
  virtual ~SequenceStructure        ();

public:
  static ErrVal     create          ( SequenceStructure*& rpcSequenceStructure );
  ErrVal            destroy         ();
  ErrVal            uninit          ();
  ErrVal            init            ( const String&       rcString,
                                      UInt                uiNumberOfFrames );
  Void              reset           ();
  ErrVal            check           ();

  const FrameSpec&  getFrameSpec    ();
  const FrameSpec&  getNextFrameSpec();
  
  static  Bool      checkString     ( const String&       rcString );
  static  ErrVal    debugOutput     ( const String&       rcString,
                                      UInt                uiNumberOfFrames,
                                      FILE*               pcFile );


  UInt    getNumberOfTotalFrames    ()  const;
  UInt    getNumberOfIDRFrames      ()  const;
  UInt    getNumberOfIntraFrames    ()  const;
  UInt    getNumberOfInterFrames    ()  const;
  UInt    getNumberOfRefFrames      ()  const;
  UInt    getNumberOfCodedFrames    ()  const;
  UInt    getMaxAbsFrameDiffRef     ()  const;
  UInt    getMinDelay               ()  const;
  UInt    getNumTemporalLayers      ()  const;


private:
  Bool    xIsFirstIDR               ();
  Void    xInitParameters           ();
  ErrVal  xGetNextValidFrameSpec    ( FrameSpec&          rcFrameSpec,
                                      UInt                uiTotalFrames );

private:
  Bool                m_bInit;
  UInt                m_uiNumberOfTotalFrames;
  UInt                m_uiNumberOfFramesProcessed;
  UInt                m_uiFrameNumPartOffset;
  SequencePart*       m_pcSequencePart;
  FrameSpec           m_cFrameSpec;
  UInt                m_uiNumIDR;
  UInt                m_uiNumIntra;
  UInt                m_uiNumRef;
  UInt                m_uiNumCoded;
  UInt                m_uiMaxAbsFrameDiffRef;
  UInt                m_uiMinDelay;
  UInt                m_uiMaxLayer;
};



H264AVC_NAMESPACE_END

#endif // !defined(AFX_SEQUENCESTRUCTURE_H__268768B8_4D1D_484A_904E_586985833BAC__INCLUDED_)


