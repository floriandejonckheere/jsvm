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




#if !defined(AFX_FRAMEUNIT_H__F112E873_18DC_48C6_9E5E_A46FF23388E3__INCLUDED_)
#define AFX_FRAMEUNIT_H__F112E873_18DC_48C6_9E5E_A46FF23388E3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "H264AVCCommonLib/MbDataCtrl.h"
#include "H264AVCCommonLib/Frame.h"
#include "H264AVCCommonLib/IntFrame.h"

H264AVC_NAMESPACE_BEGIN



class H264AVCCOMMONLIB_API FrameUnit
{
  enum
  {
    REFERENCE     = 0x01,
    IS_OUTPUTTED  = 0x02,
  };

protected:
	FrameUnit( YuvBufferCtrl& rcYuvFullPelBufferCtrl, YuvBufferCtrl& rcYuvHalfPelBufferCtrl, Bool bOriginal );
	virtual ~FrameUnit();

public:
  ErrVal init( const SliceHeader& rcSH, PicBuffer *pcPicBuffer );
  ErrVal uninit();

  static ErrVal create( FrameUnit*& rpcFrameUnit, YuvBufferCtrl& rcYuvFullPelBufferCtrl, YuvBufferCtrl& rcYuvHalfPelBufferCtrl, Bool bOriginal = false );
  ErrVal destroy ();

  Frame& getFrame()                 { return m_cFrame;    }
  const Frame& getFrame()    const  { return m_cFrame;    }

  Void  setFrameNumber( UInt  uiFN  )           { m_uiFrameNumber = uiFN; }
  UInt  getFrameNumber()                  const { return m_uiFrameNumber; }

  Void  setOutputDone ()                        { m_uiStatus |= IS_OUTPUTTED; }
  Bool  isOutputDone  ()                  const { return ( m_uiStatus & IS_OUTPUTTED ? true : false ); }

  Bool  isUsed        ()                  const { return ( m_uiStatus & REFERENCE ? true : false ); }
  Void  setUsed       ()                        { m_uiStatus |= REFERENCE; }
  Void  setUnused     ();

  Int   getMaxPOC     ()                  const { return m_iMaxPOC; }
  Void  setPoc( Int iPoc );

  PicBuffer*  getPicBuffer  ()            const { return m_pcPicBuffer; }
  const MbDataCtrl* getMbDataCtrl()       const { return &m_cMbDataCtrl; }
  MbDataCtrl* getMbDataCtrl()            { return &m_cMbDataCtrl;  }

  const IntFrame* getResidual() const { return &m_cResidual; }
  IntFrame* getResidual() { return &m_cResidual; }


  ErrVal setFGS( PicBuffer* pcPicBuffer );
  PicBuffer*  getFGSPicBuffer()           const { return m_pcFGSPicBuffer; }

  const IntFrame* getFGSIntFrame() const { return &m_cFGSIntFrame; }
  IntFrame* getFGSIntFrame() { return &m_cFGSIntFrame; }

  Frame& getFGSFrame()                 { return m_cFGSFrame;    }
  const Frame& getFGSFrame()    const  { return m_cFGSFrame;    }

private:
  Frame         m_cFrame;
  MbDataCtrl    m_cMbDataCtrl;
  PicBuffer*    m_pcPicBuffer;
  UInt          m_uiFrameNumber;
  UChar         m_uiStatus;
  Int           m_iMaxPOC;
  Bool          m_bOriginal;
  Bool          m_bInitDone;
  IntFrame      m_cResidual;

  IntFrame      m_cFGSIntFrame;
  PicBuffer*    m_pcFGSPicBuffer;
  Frame         m_cFGSFrame;
};



H264AVC_NAMESPACE_END


#endif // !defined(AFX_FRAMEUNIT_H__F112E873_18DC_48C6_9E5E_A46FF23388E3__INCLUDED_)
