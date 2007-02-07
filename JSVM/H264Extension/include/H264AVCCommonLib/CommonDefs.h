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





#if !defined(AFX_COMMONDEFS_H__4CE634CE_B48D_4812_8098_9CAEA258BAA2__INCLUDED_)
#define AFX_COMMONDEFS_H__4CE634CE_B48D_4812_8098_9CAEA258BAA2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef UChar CoefMap;
typedef UChar RefCtx;

#define _JSVM_VERSION_ "8.1" //added by jerome.vieron@thomson.net

#define MB_BUFFER_WIDTH 24
#define MB_BASE_WIDTH   16

#define DC_PRED         2
#define OUTSIDE         -1
#define DOUBLE_MAX      1.7e+308


#define NO_LEFT_REF        1
#define NO_ABOVE_REF       2
#define NO_ABOVELEFT_REF   4
#define NO_ABOVERIGHT_REF  8


H264AVC_NAMESPACE_BEGIN

enum PicType
{
  NOT_SPECIFIED   = 0x00,
  TOP_FIELD       = 0x01,
  BOT_FIELD       = 0x02,
  FRAME           = 0x03,
	MAX_FRAME_TYPE  = 0x04
};

enum ParIdx16x16
{
  PART_16x16   = 0x00
};
enum ParIdx16x8
{
  PART_16x8_0   = 0x00,
  PART_16x8_1   = 0x08
};
enum ParIdx8x16
{
  PART_8x16_0   = 0x00,
  PART_8x16_1   = 0x02
};
enum Par8x8
{
  B_8x8_0    = 0x00,
  B_8x8_1    = 0x01,
  B_8x8_2    = 0x02,
  B_8x8_3    = 0x03
};
enum ParIdx8x8
{
  PART_8x8_0    = 0x00,
  PART_8x8_1    = 0x02,
  PART_8x8_2    = 0x08,
  PART_8x8_3    = 0x0A
};
enum SParIdx8x8
{
  SPART_8x8   = 0x00
};
enum SParIdx8x4
{
  SPART_8x4_0   = 0x00,
  SPART_8x4_1   = 0x04
};
enum SParIdx4x8
{
  SPART_4x8_0   = 0x00,
  SPART_4x8_1   = 0x01
};
enum SParIdx4x4
{
  SPART_4x4_0   = 0x00,
  SPART_4x4_1   = 0x01,
  SPART_4x4_2   = 0x04,
  SPART_4x4_3   = 0x05
};

enum NeighbourBlock
{
  CURR_MB_LEFT_NEIGHBOUR   = -1,
  LEFT_MB_LEFT_NEIGHBOUR   = +3,
  CURR_MB_ABOVE_NEIGHBOUR  = -4,
  ABOVE_MB_ABOVE_NEIGHBOUR = +12,
  CURR_MB_RIGHT_NEIGHBOUR  = +1,
  RIGHT_MB_RIGHT_NEIGHBOUR = -3
};
enum ListIdx
{
  LIST_0 = 0x00,
  LIST_1 = 0x01
};

enum ProcessingState
{
  PRE_PROCESS     = 0,
  PARSE_PROCESS   = 1,
  DECODE_PROCESS  = 2,
  ENCODE_PROCESS  = 3,
  POST_PROCESS    = 4
};


enum SliceType
{
  P_SLICE  = 0,
  B_SLICE  = 1,
  I_SLICE  = 2,
  F_SLICE  = 3
};


enum NalRefIdc
{
  NAL_REF_IDC_PRIORITY_LOWEST  = 0,
  NAL_REF_IDC_PRIORITY_LOW     = 1,
  NAL_REF_IDC_PRIORITY_HIGH    = 2,
  NAL_REF_IDC_PRIORITY_HIGHEST = 3
};

enum NalUnitType
{
  NAL_UNIT_EXTERNAL                 = 0,
  NAL_UNIT_CODED_SLICE              = 1,
  NAL_UNIT_CODED_SLICE_DATAPART_A   = 2,
  NAL_UNIT_CODED_SLICE_DATAPART_B   = 3,
  NAL_UNIT_CODED_SLICE_DATAPART_C   = 4,
  NAL_UNIT_CODED_SLICE_IDR          = 5,
  NAL_UNIT_SEI                      = 6,
  NAL_UNIT_SPS                      = 7,
  NAL_UNIT_PPS                      = 8,
  NAL_UNIT_ACCESS_UNIT_DELIMITER    = 9,
  NAL_UNIT_END_OF_SEQUENCE          = 10,
  NAL_UNIT_END_OF_STREAM            = 11,
  NAL_UNIT_FILLER_DATA              = 12,

  NAL_UNIT_CODED_SLICE_SCALABLE     = 20,
  NAL_UNIT_CODED_SLICE_IDR_SCALABLE = 21
};

#define NAL_UNIT_HEADER_SVC_EXTENSION_BYTES      3

enum MbMode
{
  MODE_SKIP         = 0,
  MODE_16x16        = 1,
  MODE_16x8         = 2,
  MODE_8x16         = 3,
  MODE_8x8          = 4,
  MODE_8x8ref0      = 5,
  INTRA_4X4         = 6,
  MODE_PCM          = 25+6,
  INTRA_BL          = 36
};

enum BlkMode
{
  BLK_8x8   = 8,
  BLK_8x4   = 9,
  BLK_4x8   = 10,
  BLK_4x4   = 11,
  BLK_SKIP  = 0
};

enum Profile
{
  BASELINE_PROFILE  = 66,
  MAIN_PROFILE      = 77,
  EXTENDED_PROFILE  = 88,

  HIGH_PROFILE      = 100,
  HIGH_10_PROFILE   = 110,
  HIGH_422_PROFILE  = 122,
  HIGH_444_PROFILE  = 144,

  MULTI_VIEW_PROFILE= 78,

  SCALABLE_PROFILE  = 83
};

enum DFunc
{
  DF_SAD      = 0,
  DF_SSD      = 1,
  DF_HADAMARD = 2,
  DF_YUV_SAD  = 3
};

enum SearchMode
{
  BLOCK_SEARCH  = 0,
  SPIRAL_SEARCH = 1,
  LOG_SEARCH    = 2,
  FAST_SEARCH   = 3
};


H264AVC_NAMESPACE_END



#define MIN_QP              0
#define MAX_QP              51
#define QP_BITS             15
#define QP_SHIFT1           12
#define MAX_FRAME_NUM_LOG2  9

#define YUV_X_MARGIN        32
#define YUV_Y_MARGIN        64

#define MAX_LAYERS          8
#define MAX_TEMP_LEVELS     8
#define MAX_QUALITY_LEVELS  4
#define MAX_FGS_LAYERS      3
#define MAX_DSTAGES         6
#define LOG2_GOP_ID_WRAP    4
#define PRI_ID_BITS         6
#define MAX_SCALABLE_LAYERS MAX_LAYERS * MAX_TEMP_LEVELS * MAX_QUALITY_LEVELS

//{{Quality level estimation and modified truncation- JVTO044 and m12007
//France Telecom R&D-(nathalie.cammas@francetelecom.com)
#define MAX_NUM_RD_LEVELS      50
//}}Quality level estimation and modified truncation- JVTO044 and m12007
#define MAX_SIZE_PID 64

// heiko.schwarz@hhi.fhg.de: Hack for ensuring that the scaling factors
// work with the closed-loop config files
// and the other available config files
// SHOULD BE REMOVED in the future
#define SCALING_FACTOR_HACK 1

#define MAX_NUM_INFO_ENTRIES 8
#define MAX_NUM_NON_REQUIRED_PICS 32

#define AR_FGS_MAX_BASE_WEIGHT                        32
#define AR_FGS_BASE_WEIGHT_SHIFT_BITS                 5

// default values
#define AR_FGS_DEFAULT_LOW_PASS_ENH_REF               0.0
#define AR_FGS_DEFAULT_BASE_WEIGHT_ZERO_BLOCK         0
#define AR_FGS_DEFAULT_BASE_WEIGHT_ZERO_COEFF         0
#define AR_FGS_DEFAULT_ENC_STRUCTURE                  1

#define AR_FGS_MC_INTERP_AVC                          0
#define AR_FGS_MC_INTERP_BILINEAR                     1
#define AR_FGS_MC_INTERP_4_TAP                        2
#define AR_FGS_DEFAULT_FILTER                         AR_FGS_MC_INTERP_BILINEAR
#define AR_FGS_COMPENSATE_SIGNED_FRAME                1

#define MVC_PROFILE                                   MULTI_VIEW_PROFILE  // ( MULTI_VIEW_PROFILE or HIGH_PROFILE )
#define WEIGHTED_PRED_FLAG                            0                   // (0:no weighted prediction, 1:random weights)
#define WEIGHTED_BIPRED_IDC                           0                   // (0:no weighted bi-prediction, 1:random weights, 2:implicit weights)
#define INFER_ELAYER_PRED_WEIGHTS                     0                   // (0:BL weights are not used, 1:infer enhancement layer prediction weights)

//TMM_EC {{
typedef	enum
{
	EC_NONE												=	100,
  EC_BLSKIP,
	EC_FRAME_COPY,
	EC_TEMPORAL_DIRECT,
	EC_INTRA_COPY                // =200
}	ERROR_CONCEAL;
//TMM_EC }}

#define MAX_NUM_PD_FRAGMENTS                          12
#define MAX_NUM_FGS_VECT_MODES                        8

// TMM_INTERLACE {
// #define RANDOM_MBAFF 
// TMM_INTERLACE }

#endif // !defined(AFX_COMMONDEFS_H__4CE634CE_B48D_4812_8098_9CAEA258BAA2__INCLUDED_)
