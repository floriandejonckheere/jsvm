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

#ifndef _DOWN_CONVERT_
#define _DOWN_CONVERT_


#include "ResizeParameters.h"


#ifdef DOWN_CONVERT_STATIC

#ifndef  max
#define  max(x,y)   ((x)>(y)?(x):(y))
#define  min(x,y)   ((x)<(y)?(x):(y))
#endif

#else

H264AVC_NAMESPACE_BEGIN

enum MbMapEntry
{
  INVALID_ENTRY     = 0x00,
  BASE_MODE_ALLOWED = 0x01,
  INTRA_UPS_ALLOWED = 0x02
};

#endif


class DownConvert
{
public:
  //========================
  // general main functions 
  //========================
  DownConvert   ();
  ~DownConvert  ();
  bool  init    ( int iMaxWidth, int iMaxHeight );
  void  destroy ();
  
#ifdef  DOWN_CONVERT_STATIC
  //=====================================
  // main functions for DownConvert Tool 
  //=====================================
  void  cropping                    ( unsigned char*          pucBufferY,    int   iStrideY,
                                      unsigned char*          pucBufferU,    int   iStrideU,
                                      unsigned char*          pucBufferV,    int   iStrideV,
                                      ResizeParameters*       pcParameters );
  void  upsamplingDyadic            ( unsigned char*          pucBufferY,    int   iStrideY,
                                      unsigned char*          pucBufferU,    int   iStrideU,
                                      unsigned char*          pucBufferV,    int   iStrideV,
                                      ResizeParameters*       pcParameters );
  void  upsamplingLanczos           ( unsigned char*          pucBufferY,    int   iStrideY,
                                      unsigned char*          pucBufferU,    int   iStrideU,
                                      unsigned char*          pucBufferV,    int   iStrideV,
                                      ResizeParameters*       pcParameters );
  void  upsampling6tapBilin         ( unsigned char*          pucBufferY,    int   iStrideY,
                                      unsigned char*          pucBufferU,    int   iStrideU,
                                      unsigned char*          pucBufferV,    int   iStrideV,
                                      ResizeParameters*       pcParameters );
  void  upsamplingSVC               ( unsigned char*          pucBufferY,    int   iStrideY,
                                      unsigned char*          pucBufferU,    int   iStrideU,
                                      unsigned char*          pucBufferV,    int   iStrideV,
                                      ResizeParameters*       pcParameters,  bool  bBotCoincided = false );
  void  downsamplingDyadic          ( unsigned char*          pucBufferY,    int   iStrideY,
                                      unsigned char*          pucBufferU,    int   iStrideU,
                                      unsigned char*          pucBufferV,    int   iStrideV,
                                      ResizeParameters*       pcParameters );
  void  downsamplingSVC             ( unsigned char*          pucBufferY,    int   iStrideY,
                                      unsigned char*          pucBufferU,    int   iStrideU,
                                      unsigned char*          pucBufferV,    int   iStrideV,
                                      ResizeParameters*       pcParameters,  bool  bBotCoincided = false );
#else
  //====================================
  // main functions for encoder/decoder 
  //====================================
  void  intraUpsampling             ( Frame*                  pcFrame, 
                                      Frame*                  pcBaseFrame,
                                      Frame*                  pcTempFrame,
                                      Frame*                  pcTempBaseFrame,
                                      ResizeParameters*       pcParameters,
                                      MbDataCtrl*             pcMbDataCtrlBase, 
                                      MbDataCtrl*             pcMbDataCtrlPredFrm, 
                                      MbDataCtrl*             pcMbDataCtrlPredFld, 
                                      ReconstructionBypass*   pcReconstructionBypass,
                                      Bool*                   pabBaseModeAllowedFlagArrayFrm,
                                      Bool*                   pabBaseModeAllowedFlagArrayFld,
                                      Bool                    bConstrainedIntraResamplingFlag );
  void  residualUpsampling          ( Frame*                  pcFrame, 
                                      Frame*                  pcBaseFrame,
                                      ResizeParameters*       pcParameters, 
                                      MbDataCtrl*             pcMbDataCtrlBase );
#endif


  //==========================
  // general helper functions 
  //==========================
  //--- delete buffers ---
  void  xDestroy                    ();
  //--- general clipping ---
  int   xClip                       ( int                     iValue,
                                      int                     imin,
                                      int                     imax );
  //--- SVC normative intra upsampling ---
  void  xCompIntraUpsampling        ( ResizeParameters*       pcParameters,
                                      bool                    bChroma,
                                      bool                    bBotFlag,
                                      bool                    bVerticalInterpolation );
  void  xVertIntraUpsampling        ( int  iBaseW,   int  iBaseH,
                                      int  iLOffset, int  iTOffset, int  iROffset, int  iBOffset,
                                      int  iYBorder, bool bBotFlag, bool bChromaFilter );
  void  xBasicIntraUpsampling       ( int  iBaseW,   int  iBaseH,   int  iCurrW,   int  iCurrH,
                                      int  iLOffset, int  iTOffset, int  iROffset, int  iBOffset,
                                      int  iShiftX,  int  iShiftY,  int  iScaleX,  int  iScaleY,
                                      int  iOffsetX, int  iOffsetY, int  iAddX,    int  iAddY,
                                      int  iDeltaX,  int  iDeltaY,  int  iYBorder, bool bChromaFilter );

#ifdef  DOWN_CONVERT_STATIC
  //=======================================
  // helper functions for DownConvert Tool 
  //=======================================
  //--- initialization ---
  void  xInitLanczosFilter          ();
  //--- place to and get from image buffer ---
  void  xCopyToImageBuffer          ( unsigned char*        pucSrc,
                                      int                   iWidth,
                                      int                   iHeight,
                                      int                   iStride );
  void  xCopyFromImageBuffer        ( unsigned char*        pucDes,
                                      int                   iWidth,
                                      int                   iHeight,
                                      int                   iStride );
  void  xInitializeWithValue        ( unsigned char*        pucBuffer,
                                      int                   iWidth,
                                      int                   iHeight,
                                      int                   iStride,
                                      unsigned char         cValue );
  //--- dyadic upsampling ---
  void  xCompUpsamplingDyadic       ( int                   iBaseW,
                                      int                   iBaseH,     
                                      bool                  bChroma );
  //--- Lanczos upsampling ---
  void  xCompUpsamplingLanczos      ( ResizeParameters*     pcParameters,
                                      bool                  bChroma );
  void  xUpsamplingDataLanczos      ( int                   iInLength,
                                      int                   iOutLength,
                                      long                  spos );
  void  xGetNumDenomLanczos         ( int                   iInWidth,
                                      int                   iOutWidth,
                                      int&                  riNumerator,
                                      int&                  riDenominator );
  long  xGetFilterLanczos           ( long                  x );
  //--- 6-tap + bilinear upsampling ---
  void  xCompUpsampling6tapBilin    ( ResizeParameters*     pcParameters,
                                      bool                  bChroma );
  void  xUpsamplingData6tapBilin    ( int                   iInLength,
                                      int                   iOutLength );
  //--- dyadic downsampling ---
  void  xCompDownsamplingDyadic     ( int                   iCurrW,
                                      int                   iCurrH );
  //--- SVC non-normative downsampling ---
  void  xCompDownsampling           ( ResizeParameters*     pcParameters,
                                      bool                  bChroma, 
                                      bool                  bBotFlag, 
                                      bool                  bVerticalDownsampling );
  void  xVertDownsampling           ( int                   iBaseW,
                                      int                   iBaseH,
                                      bool                  bBotFlag );
  void  xBasicDownsampling          ( int  iBaseW,   int  iBaseH,   int  iCurrW,   int  iCurrH,
                                      int  iLOffset, int  iTOffset, int  iROffset, int  iBOffset,
                                      int  iShiftX,  int  iShiftY,  int  iScaleX,  int  iScaleY,
                                      int  iAddX,    int  iAddY,    int  iDeltaX,  int  iDeltaY );
#else
  //======================================
  // helper functions for encoder/decoder 
  //======================================
  //--- place to and get from image buffer ---
  void  xCopyToImageBuffer          ( const short*          psSrc,
                                      int                   iWidth,
                                      int                   iHeight,
                                      int                   iStride );
  void  xCopyFromImageBuffer        ( short*                psDes,
                                      int                   iWidth,
                                      int                   iHeight,
                                      int                   iStride );
  void  xInitializeWithValue        ( short*                psBuffer,
                                      int                   iWidth,
                                      int                   iHeight,
                                      int                   iStride,
                                      short                 iValue );
  //--- cropping ---
  void  xCrop                       ( Frame*                pcFrame, 
                                      Frame*                pcBaseFrame,
                                      ResizeParameters*     pcParameters,
                                      short                 iValue );
  //--- SVC intra upsampling ---
  void  xIntraUpsampling            ( Frame*                pcFrame, 
                                      Frame*                pcBaseFrame,
                                      ResizeParameters*     pcParameters );
  void  xInitSliceIdList            ( MyList<unsigned int>& rcSliceIdList,
                                      ResizeParameters*     pcParameters,
                                      MbDataCtrl*           pcMbDataCtrl );
  void  xInitBaseModeAllowedFlags   ( ResizeParameters*     pcParameters,
                                      bool*                 pabBaseModeAllowedFlagArrayFrm,
                                      bool*                 pabBaseModeAllowedFlagArrayFld );
  void  xUpdateBaseModeAllowedFlags ( ResizeParameters*     pcParameters,
                                      bool*                 pabBaseModeAllowedFlagArrayFrm,
                                      bool*                 pabBaseModeAllowedFlagArrayFld );
  void  xGenerateMbMapsForSliceId   ( ResizeParameters*     pcParameters,
                                      MbDataCtrl*           pcMbDataCtrlBase,
                                      MbDataCtrl*           pcMbDataCtrlPredFrm,
                                      MbDataCtrl*           pcMbDataCtrlPredFld,
                                      unsigned int          uiCurrentSliceId );
  void  xInitMbMaps                 ( ResizeParameters*     pcParameters,
                                      bool                  bFrm,
                                      bool                  bTop,
                                      bool                  bBot );
  void  xUpdateMbMapForSliceId      ( ResizeParameters*     pcParameters,
                                      bool                  bChroma,
                                      bool                  bFieldMb,
                                      MbDataCtrl*           pcMbDataCtrlBase,
                                      MbDataCtrl*           pcMbDataCtrlPredFrm,
                                      MbDataCtrl*           pcMbDataCtrlPredFld,
                                      unsigned int          uiCurrentSliceId );
  void  xUpdateIntraPredFrame       ( Frame*                pcDesFrame,
                                      Frame*                pcSrcFrame,
                                      ResizeParameters*     pcParameters );
  //--- SVC residual upsampling ---
  void  xResidualUpsampling         ( Frame*              pcFrame, 
                                      Frame*              pcBaseFrame,
                                      ResizeParameters*   pcParameters,
                                      MbDataCtrl*         pcMbDataCtrlBase );
  void  xDetermineTransBlkIdcs      ( int                 iBaseW,
                                      int                 iBaseH,
                                      bool                bChroma,
                                      bool                bBotField,
                                      ResizeParameters*   pcRP,     
                                      MbDataCtrl*         pcMbDataCtrlBase );
  void  xCompResidualUpsampling     ( ResizeParameters*   pcParameters, 
                                      bool                bChroma, 
                                      bool                bBotFlag,
                                      bool                bVerticalInterpolation,
                                      MbDataCtrl*         pcMbDataCtrlBase );
  void  xVertResidualUpsampling     ( int  iBaseW,   int  iBaseH,
                                      int  iLOffset, int  iTOffset, int  iROffset, int  iBOffset,
                                      int  iYBorder, bool bBotFlag );
  void  xBasicResidualUpsampling    ( int  iBaseW,   int  iBaseH,   int  iCurrW,   int  iCurrH,
                                      int  iLOffset, int  iTOffset, int  iROffset, int  iBOffset,
                                      int  iShiftX,  int  iShiftY,  int  iScaleX,  int  iScaleY,
                                      int  iOffsetX, int  iOffsetY, int  iAddX,    int  iAddY,
                                      int  iDeltaX,  int  iDeltaY,  int  iYBorder );
#endif

private:
  //===== member variables =====
  int         m_iImageStride;
  int*        m_paiImageBuffer;
  int*        m_paiTmp1dBuffer;
#ifdef  DOWN_CONVERT_STATIC
  long*       m_padFilter;
  int*        m_aiTmp1dBufferInHalfpel;
  int*        m_aiTmp1dBufferInQ1pel;
  int*        m_aiTmp1dBufferInQ3pel;
  int*        m_paiTmp1dBufferOut;
#else
  int*        m_paiTransBlkIdc;
  int         m_iMbMapStride;
  MbMapEntry* m_paeMbMapFrm;
  MbMapEntry* m_paeMbMapFld;
#endif
};


#ifdef DOWN_CONVERT_STATIC
#else
H264AVC_NAMESPACE_END
#endif


#endif // _DOWN_CONVERT_


