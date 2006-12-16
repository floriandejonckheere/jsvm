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




#if !defined(AFX_NALUNITPARSER_H__D5B74729_6F04_42E9_91AE_2E28937F9F3A__INCLUDED_)
#define AFX_NALUNITPARSER_H__D5B74729_6F04_42E9_91AE_2E28937F9F3A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



H264AVC_NAMESPACE_BEGIN


class BitReadBuffer;


class NalUnitParser
{
public:
  NalUnitParser                 ();
  virtual ~NalUnitParser        ();

  static ErrVal create          ( NalUnitParser*&   rpcNalUnitParser  );

  ErrVal        init            ( BitReadBuffer*    pcBitReadBuffer   );
  ErrVal        destroy         ();

  ErrVal        initNalUnit     ( BinDataAccessor*  pcBinDataAccessor, 
                                  //Bool*             KeyPicFlag, 
                                  UInt&             uiNumBytesRemoved,            //FIX_FRAG_CAVLC
	                                Bool              bPreParseHeader     = true,
                                  Bool              bConcatenated       = false,  //FRAG_FIX
		                              Bool              bCheckGap           = false,  //TMM_EC
                                  UInt*             puiNumFragments     = 0,
                                  UChar**           ppucFragBuffers     = 0 ); 
  ErrVal        closeNalUnit    ();

  NalUnitType   getNalUnitType  ()      { return m_eNalUnitType;    }
//  TMM_EC {{
  Bool          isTrueNalUnit   ()      { return *(int*)m_pucBuffer != 0xdeadface;    }
  ErrVal        setNalUnitType  (NalUnitType eNalRefUnitType)    { m_eNalUnitType=eNalRefUnitType; return Err::m_nOK;}
//  TMM_EC }}
  NalRefIdc     getNalRefIdc    ()      { return m_eNalRefIdc;      }
  UInt          getLayerId      ()      { return m_uiLayerId;       }
  UInt          getTemporalLevel()      { return m_uiTemporalLevel; }
  UInt          getQualityLevel ()      { return m_uiQualityLevel;  }
  UInt          getPriorityId ()        { return m_uiPriorityId;  }

  Bool          getUseBasePredFlag()    { return m_bUseBasePredFlag;}
  Bool          getLayerBaseFlag()      { return m_bLayerBaseFlag;}
  Bool          getDiscardableFlag ()    { return m_bDiscardableFlag;}

  Bool          getFragmentedFlag()     { return m_bFGSFragFlag;}
  Bool          getLastFragmentFlag()   { return m_bFGSLastFragFlag;}
  UInt          getFragmentOrder()      { return m_uiFGSFragOrder;}

  //JVT-P031
  UInt getBytesLeft();
  UInt getBitsLeft();
  ErrVal initSODBNalUnit( BinDataAccessor* pcBinDataAccessor );
  UInt getNalHeaderSize( BinDataAccessor* pcBinDataAccessor );

  Void setCheckAllNALUs(Bool b) { m_bCheckAllNALUs = b;}
  Void setDecodedLayer( UInt uiLayer) { m_uiDecodedLayer = uiLayer;}
  //~JVT-P031
  ErrVal  readAUDelimiter       ();
  ErrVal  readEndOfSeqence      ();
  ErrVal  readEndOfStream       ();

protected:
  Void    xTrace                ( Bool  bDDIPresent     );
  ErrVal  xConvertPayloadToRBSP ( UInt& ruiPacketLength );
  ErrVal  xConvertRBSPToSODB    ( UInt  iPacketLength,
                                  UInt& ruiBitsInPacket );

protected:
  BitReadBuffer *m_pcBitReadBuffer;
//TMM_EC {{
public:
  UChar         *m_pucBuffer;
protected:
//TMM_EC }}
  NalUnitType   m_eNalUnitType;
  NalRefIdc     m_eNalRefIdc;

  UInt          m_uiPriorityId;
  UInt          m_uiTemporalLevel;
  UInt          m_uiLayerId;
  UInt          m_uiQualityLevel;

  // JVT T083
  Bool          m_bLayerBaseFlag;
  Bool          m_bUseBasePredFlag;
  Bool          m_bDiscardableFlag;
  Bool          m_bFGSFragFlag;
  Bool          m_bFGSLastFragFlag;
  UInt          m_uiFGSFragOrder;
  Bool          m_bExtensionFlag;
  UInt          m_uiTl0FrameIdx;

  Bool          m_bCheckAllNALUs;
  UInt          m_uiDecodedLayer;
  //~JVT-P031
  UInt          m_uiBitsInPacketSaved; //FRAG_FIX
};


H264AVC_NAMESPACE_END


#endif // !defined(AFX_NALUNITPARSER_H__D5B74729_6F04_42E9_91AE_2E28937F9F3A__INCLUDED_)
