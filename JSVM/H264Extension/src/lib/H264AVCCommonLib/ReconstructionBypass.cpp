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
#include "H264AVCCommonLib/ReconstructionBypass.h"


#include "H264AVCCommonLib/Frame.h"
#include "H264AVCCommonLib/YuvMbBuffer.h"
#include "H264AVCCommonLib/MbDataCtrl.h"
#include "H264AVCCommonLib/YuvBufferCtrl.h"

H264AVC_NAMESPACE_BEGIN

ReconstructionBypass::ReconstructionBypass() 
{
}

ErrVal ReconstructionBypass::create( ReconstructionBypass*& rpcReconstructionBypass )
{
  rpcReconstructionBypass = new ReconstructionBypass;
  ROT( NULL == rpcReconstructionBypass ) ;
  return Err::m_nOK;
}

ErrVal ReconstructionBypass::destroy()
{
  delete this;
  return Err::m_nOK;
}

ErrVal ReconstructionBypass::init()
{
  return Err::m_nOK;
}

ErrVal ReconstructionBypass::uninit()
{
  return Err::m_nOK;
}
 
ErrVal ReconstructionBypass::padRecFrame( Frame* pcIntFrame, const MbDataCtrl* pcMbDataCtrl, YuvBufferCtrl* pcYuvFullPelBufferCtrl, UInt uiFrameWidthInMb, UInt uiFrameHeightInMb )
{
  YuvPicBuffer* pcIntYuvPicBuffer = pcIntFrame->getFullPelYuvBuffer();

  // loop over macroblocks
  for( UInt uiMbY = 0; uiMbY < uiFrameWidthInMb; uiMbY++ )
  {
    for( UInt uiMbX = 0; uiMbX < uiFrameHeightInMb; uiMbX++ )
    {
      UInt uiMask = 0;

      //===== init macroblock =====
      RNOK( pcMbDataCtrl   ->getBoundaryMask( uiMbY, uiMbX, uiMask ) );
			RNOK( pcYuvFullPelBufferCtrl->initMb  ( uiMbY, uiMbX, false  ) );
      if( uiMask )
      {
		    IntYuvMbBufferExtension cBuffer;
		    cBuffer.loadSurrounding( pcIntYuvPicBuffer );

        RNOK( padRecMb( &cBuffer, uiMask ) );

  	    pcIntYuvPicBuffer->loadBuffer( &cBuffer );
      }
    }
  }

  return Err::m_nOK;
}

ErrVal ReconstructionBypass::padRecMb( IntYuvMbBufferExtension* pcBuffer, UInt uiMask )
{

  Bool bAboveIntra      = 0 != (uiMask & 0x01);
  Bool bBelowIntra      = 0 != (uiMask & 0x10);
  Bool bLeftIntra       = 0 != (uiMask & 0x40);
  Bool bRightIntra      = 0 != (uiMask & 0x04);
  Bool bLeftAboveIntra  = 0 != (uiMask & 0x80);
  Bool bRightAboveIntra = 0 != (uiMask & 0x02);
  Bool bLeftBelowIntra  = 0 != (uiMask & 0x20);
  Bool bRightBelowIntra = 0 != (uiMask & 0x08);

  for( B8x8Idx cIdx; cIdx.isLegal(); cIdx++ )
  {
    switch( cIdx.b8x8Index() )
    {
    case 0:
      {
        if( bAboveIntra )
        {
          if( bLeftIntra )
          {
            pcBuffer->mergeFromLeftAbove( cIdx, bLeftAboveIntra );
          }
          else
          {
            pcBuffer->copyFromAbove( cIdx );
          }
        }
        else
        {
          if( bLeftIntra )
          {
            pcBuffer->copyFromLeft( cIdx );
          }
          else if( bLeftAboveIntra )
          {
            pcBuffer->copyFromLeftAbove( cIdx );
          }
        }
      }
      break;
    case 1:
      {
        if( bAboveIntra )
        {
          if( bRightIntra )
          {
            pcBuffer->mergeFromRightAbove( cIdx, bRightAboveIntra );
          }
          else
          {
            pcBuffer->copyFromAbove( cIdx );
          }
        }
        else
        {
          if( bRightIntra )
          {
            pcBuffer->copyFromRight( cIdx );
          }
          else if( bRightAboveIntra )
          {
            pcBuffer->copyFromRightAbove( cIdx );
          }
        }
      }
      break;
    case 2:
      {
        if( bBelowIntra )
        {
          if( bLeftIntra )
          {
            pcBuffer->mergeLeftBelow( cIdx, bLeftBelowIntra );
          }
          else
          {
            pcBuffer->copyFromBelow( cIdx );
          }
        }
        else
        {
          if( bLeftIntra )
          {
            pcBuffer->copyFromLeft( cIdx );
          }
          else if( bLeftBelowIntra )
          {
            pcBuffer->copyFromLeftBelow( cIdx );
          }
        }
      }
      break;
    case 3:
      {
        if( bBelowIntra )
        {
          if( bRightIntra )
          {
            pcBuffer->mergeRightBelow( cIdx, bRightBelowIntra );
          }
          else
          {
            pcBuffer->copyFromBelow( cIdx );
          }
        }
        else
        {
          if( bRightIntra )
          {
            pcBuffer->copyFromRight( cIdx );
          }
          else if( bRightBelowIntra )
          {
            pcBuffer->copyFromRightBelow( cIdx );
          }
        }
      }
      break;
    default:
      break;
    }
  }

  return Err::m_nOK;
}

//TMM_INTERLACE  {
ErrVal ReconstructionBypass::padRecMb_MbAff( IntYuvMbBufferExtension* pcBuffer, UInt uiMask )
{
  Bool bAboveIntra      = 0 != (uiMask & 0x01);
  Bool bBelowIntra      = 0 != (uiMask & 0x10);
  //Bool bLeftIntra       = 0 != (uiMask & 0x40);
  //Bool bRightIntra      = 0 != (uiMask & 0x04);
  Bool bLeftAboveIntra  = 0 != (uiMask & 0x80);
  Bool bRightAboveIntra = 0 != (uiMask & 0x02);
  Bool bLeftBelowIntra  = 0 != (uiMask & 0x20);
  Bool bRightBelowIntra = 0 != (uiMask & 0x08);
  Bool bLeftIntraTop    = 0 != (uiMask & 0x0100);
  Bool bLeftIntraBot    = 0 != (uiMask & 0x0200);
  Bool bRightIntraTop   = 0 != (uiMask & 0x0400);
  Bool bRightIntraBot   = 0 != (uiMask & 0x0800);
  Bool bTopIntra        = 0 != (uiMask & 0x01000);
  Bool bBotIntra        = 0 != (uiMask & 0x02000);

  UInt mymap[4]={0,1,2,3}; 

  if(bBotIntra)
   { 
 //TMM_JV_PAD
	   //  pcBuffer->setSubBlock();
     mymap[2]=0;
     mymap[3]=1;
 
//TMM_JV_PAD {
	 if(bAboveIntra)
	{
	bBelowIntra = bLeftIntraBot = bRightIntraBot =bLeftBelowIntra= bRightBelowIntra= 0;
	}
 //TMM_JV_PAD }  
   }
   else if(bTopIntra)
   {
    //TMM_JV_PAD
	   //pcBuffer->setSubBlock();
    mymap[0]=2;
    mymap[1]=3;

	//TMM_JV_PAD {
	if(bBelowIntra)
	{
	bAboveIntra = bLeftIntraTop = bRightIntraTop = bLeftAboveIntra= bRightAboveIntra= 0;
	}
	//TMM_JV_PAD }
   }

  //for( B8x8Idx cIdx; cIdx.isLegal(); cIdx++ )
  for( B8x8Idx cB8x8idx; cB8x8idx.isLegal(); cB8x8idx++ )
  {
    //Int iIndex=mymap[cIdx.b8x8Index()];
    Int iIndex=mymap[cB8x8idx.b8x8Index()];
    B8x8Idx cIdx( (Par8x8)iIndex );
    
    switch(cB8x8idx.b8x8Index())//cIdx.b8x8Index() )
    {
    case 0:
      {
        if( bAboveIntra )
        {
           if( bLeftIntraTop )
          {
            pcBuffer->mergeFromLeftAbove( cIdx, bLeftAboveIntra );
          }
          else
          {
            pcBuffer->copyFromAbove( cIdx );
          }
        }
        else
        {
          if( bLeftIntraTop )
          {
            pcBuffer->copyFromLeft( cIdx );
          }
          else if( bLeftAboveIntra )
          {
            pcBuffer->copyFromLeftAbove( cIdx );
          }
        }
      }
      break;
    case 1:
      {
        if( bAboveIntra )
        {
          if( bRightIntraTop )  
          {
            pcBuffer->mergeFromRightAbove( cIdx, bRightAboveIntra );
          }
          else
          {
            pcBuffer->copyFromAbove( cIdx );
          }
        }
        else
        {
          if( bRightIntraTop )
          {
            pcBuffer->copyFromRight( cIdx );
          }
          else if( bRightAboveIntra )
          {
            pcBuffer->copyFromRightAbove( cIdx );
          }
        }
      }
      break;
    case 2:
      {
        if( bBelowIntra )
        {
          if( bLeftIntraBot) 
          {
            pcBuffer->mergeLeftBelow( cIdx, bLeftBelowIntra );
          }
          else
          {
            pcBuffer->copyFromBelow( cIdx );
          }
        }
        else
        {
          if( bLeftIntraBot )
          {
            pcBuffer->copyFromLeft( cIdx);
          }
          else if( bLeftBelowIntra )
          {
            pcBuffer->copyFromLeftBelow( cIdx );
          }
        }
      }
      break;
    case 3:
      {
        if( bBelowIntra )
        {
          if( bRightIntraBot ) 
          {
            pcBuffer->mergeRightBelow( cIdx, bRightBelowIntra );
          }
          else
          {
            pcBuffer->copyFromBelow( cIdx );
          }
        }
        else
        {
          if( bRightIntraBot )
          {
            pcBuffer->copyFromRight( cIdx );
          }
          else if( bRightBelowIntra )
          {
            pcBuffer->copyFromRightBelow( cIdx );
          }
        }
      }
      break;
    default:
      break;
    }
  }

  return Err::m_nOK;
}
//TMM_INTERLACE  }

H264AVC_NAMESPACE_END




