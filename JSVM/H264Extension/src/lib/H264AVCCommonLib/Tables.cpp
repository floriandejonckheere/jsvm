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
#include "H264AVCCommonLib/Tables.h"


H264AVC_NAMESPACE_BEGIN


const UChar g_aucFrameScan[16] =
{
  0, 1, 4, 8, 5, 2, 3, 6, 9, 12, 13, 10, 7, 11, 14, 15
};

const UChar g_aucIndexChromaDCScan[4] =
{
  0, 16, 32, 48
};

const UChar g_aucLumaFrameDCScan[16] =
{
  0, 16, 64, 128, 80, 32, 48, 96, 144, 192, 208, 160, 112, 176, 224, 240
};

const Int g_aaiQuantCoef[6][16] =
{
  { 13107, 8066,13107, 8066,
     8066, 5243, 8066, 5243,
    13107, 8066,13107, 8066,
     8066, 5243, 8066, 5243
  },
  { 11916, 7490,11916, 7490,
     7490, 4660, 7490, 4660,
    11916, 7490,11916, 7490,
     7490, 4660, 7490, 4660
  },
  { 10082, 6554,10082, 6554,
     6554, 4194, 6554, 4194,
    10082, 6554,10082, 6554,
     6554, 4194, 6554, 4194
  },
  {  9362, 5825, 9362, 5825,
     5825, 3647, 5825, 3647,
     9362, 5825, 9362, 5825,
     5825, 3647, 5825, 3647
  },
  {  8192, 5243, 8192, 5243,
     5243, 3355, 5243, 3355,
     8192, 5243, 8192, 5243,
     5243, 3355, 5243, 3355
  },
  {  7282, 4559, 7282, 4559,
     4559, 2893, 4559, 2893,
     7282, 4559, 7282, 4559,
     4559, 2893, 4559, 2893
  }
};


const Int g_aaiDequantCoef[6][16] =
{
  { 10, 13, 10, 13,
    13, 16, 13, 16,
    10, 13, 10, 13,
    13, 16, 13, 16
  },
  { 11, 14, 11, 14,
    14, 18, 14, 18,
    11, 14, 11, 14,
    14, 18, 14, 18
  },
  { 13, 16, 13, 16,
    16, 20, 16, 20,
    13, 16, 13, 16,
    16, 20, 16, 20
  },
  { 14, 18, 14, 18,
    18, 23, 18, 23,
    14, 18, 14, 18,
    18, 23, 18, 23
  },
  { 16, 20, 16, 20,
    20, 25, 20, 25,
    16, 20, 16, 20,
    20, 25, 20, 25
  },
  {
    18, 23, 18, 23,
    23, 29, 23, 29,
    18, 23, 18, 23,
    23, 29, 23, 29
  }
};


const UChar g_aucChromaScale[52]=
{
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,
   12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,
   28,29,29,30,31,32,32,33,34,34,35,35,36,36,37,37,
   37,38,38,38,39,39,39,39
};




const UChar g_aucFrameScan64[64] =
{
   0,  1,  8, 16,  9,  2,  3, 10,
  17, 24, 32, 25, 18, 11,  4,  5,
  12, 19, 26, 33, 40, 48, 41, 34,
  27, 20, 13,  6,  7, 14, 21, 28,
  35, 42, 49, 56, 57, 50, 43, 36,
  29, 22, 15, 23, 30, 37, 44, 51,
  58, 59, 52, 45, 38, 31, 39, 46,
  53, 60, 61, 54, 47, 55, 62, 63
};

const Int g_aaiDequantCoef64[6][64] = 
{
  {
    20,  19, 25, 19, 20, 19, 25, 19,
    19,  18, 24, 18, 19, 18, 24, 18,
    25,  24, 32, 24, 25, 24, 32, 24,
    19,  18, 24, 18, 19, 18, 24, 18,
    20,  19, 25, 19, 20, 19, 25, 19,
    19,  18, 24, 18, 19, 18, 24, 18,
    25,  24, 32, 24, 25, 24, 32, 24,
    19,  18, 24, 18, 19, 18, 24, 18
  },
  {
    22,  21, 28, 21, 22, 21, 28, 21,
    21,  19, 26, 19, 21, 19, 26, 19,
    28,  26, 35, 26, 28, 26, 35, 26,
    21,  19, 26, 19, 21, 19, 26, 19,
    22,  21, 28, 21, 22, 21, 28, 21,
    21,  19, 26, 19, 21, 19, 26, 19,
    28,  26, 35, 26, 28, 26, 35, 26,
    21,  19, 26, 19, 21, 19, 26, 19
  },
  {
    26,  24, 33, 24, 26, 24, 33, 24,
    24,  23, 31, 23, 24, 23, 31, 23,
    33,  31, 42, 31, 33, 31, 42, 31,
    24,  23, 31, 23, 24, 23, 31, 23,
    26,  24, 33, 24, 26, 24, 33, 24,
    24,  23, 31, 23, 24, 23, 31, 23,
    33,  31, 42, 31, 33, 31, 42, 31,
    24,  23, 31, 23, 24, 23, 31, 23
  },
  {
    28,  26, 35, 26, 28, 26, 35, 26,
    26,  25, 33, 25, 26, 25, 33, 25,
    35,  33, 45, 33, 35, 33, 45, 33,
    26,  25, 33, 25, 26, 25, 33, 25,
    28,  26, 35, 26, 28, 26, 35, 26,
    26,  25, 33, 25, 26, 25, 33, 25,
    35,  33, 45, 33, 35, 33, 45, 33,
    26,  25, 33, 25, 26, 25, 33, 25
  },
  {
    32,  30, 40, 30, 32, 30, 40, 30,
    30,  28, 38, 28, 30, 28, 38, 28,
    40,  38, 51, 38, 40, 38, 51, 38,
    30,  28, 38, 28, 30, 28, 38, 28,
    32,  30, 40, 30, 32, 30, 40, 30,
    30,  28, 38, 28, 30, 28, 38, 28,
    40,  38, 51, 38, 40, 38, 51, 38,
    30,  28, 38, 28, 30, 28, 38, 28
  },
  {
    36,  34, 46, 34, 36, 34, 46, 34,
    34,  32, 43, 32, 34, 32, 43, 32,
    46,  43, 58, 43, 46, 43, 58, 43,
    34,  32, 43, 32, 34, 32, 43, 32,
    36,  34, 46, 34, 36, 34, 46, 34,
    34,  32, 43, 32, 34, 32, 43, 32,
    46,  43, 58, 43, 46, 43, 58, 43,
    34,  32, 43, 32, 34, 32, 43, 32
  }

};

const Int g_aaiQuantCoef64[6][64] = 
{
  {
    0x3333, 0x2fbe, 0x4189, 0x2fbe, 0x3333, 0x2fbe, 0x4189, 0x2fbe,
    0x2fbe, 0x2ca4, 0x3c79, 0x2ca4, 0x2fbe, 0x2ca4, 0x3c79, 0x2ca4,
    0x4189, 0x3c79, 0x51ec, 0x3c79, 0x4189, 0x3c79, 0x51ec, 0x3c79,
    0x2fbe, 0x2ca4, 0x3c79, 0x2ca4, 0x2fbe, 0x2ca4, 0x3c79, 0x2ca4,
    0x3333, 0x2fbe, 0x4189, 0x2fbe, 0x3333, 0x2fbe, 0x4189, 0x2fbe,
    0x2fbe, 0x2ca4, 0x3c79, 0x2ca4, 0x2fbe, 0x2ca4, 0x3c79, 0x2ca4,
    0x4189, 0x3c79, 0x51ec, 0x3c79, 0x4189, 0x3c79, 0x51ec, 0x3c79,
    0x2fbe, 0x2ca4, 0x3c79, 0x2ca4, 0x2fbe, 0x2ca4, 0x3c79, 0x2ca4,
  },
  {
    0x2e8c, 0x2b32, 0x3a84, 0x2b32, 0x2e8c, 0x2b32, 0x3a84, 0x2b32,
    0x2b32, 0x2a4a, 0x37d2, 0x2a4a, 0x2b32, 0x2a4a, 0x37d2, 0x2a4a,
    0x3a84, 0x37d2, 0x4ae6, 0x37d2, 0x3a84, 0x37d2, 0x4ae6, 0x37d2,
    0x2b32, 0x2a4a, 0x37d2, 0x2a4a, 0x2b32, 0x2a4a, 0x37d2, 0x2a4a,
    0x2e8c, 0x2b32, 0x3a84, 0x2b32, 0x2e8c, 0x2b32, 0x3a84, 0x2b32,
    0x2b32, 0x2a4a, 0x37d2, 0x2a4a, 0x2b32, 0x2a4a, 0x37d2, 0x2a4a,
    0x3a84, 0x37d2, 0x4ae6, 0x37d2, 0x3a84, 0x37d2, 0x4ae6, 0x37d2,
    0x2b32, 0x2a4a, 0x37d2, 0x2a4a, 0x2b32, 0x2a4a, 0x37d2, 0x2a4a,
  },
  {
    0x2762, 0x25cb, 0x31a6, 0x25cb, 0x2762, 0x25cb, 0x31a6, 0x25cb,
    0x25cb, 0x22ef, 0x2ed1, 0x22ef, 0x25cb, 0x22ef, 0x2ed1, 0x22ef,
    0x31a6, 0x2ed1, 0x3e6a, 0x2ed1, 0x31a6, 0x2ed1, 0x3e6a, 0x2ed1,
    0x25cb, 0x22ef, 0x2ed1, 0x22ef, 0x25cb, 0x22ef, 0x2ed1, 0x22ef,
    0x2762, 0x25cb, 0x31a6, 0x25cb, 0x2762, 0x25cb, 0x31a6, 0x25cb,
    0x25cb, 0x22ef, 0x2ed1, 0x22ef, 0x25cb, 0x22ef, 0x2ed1, 0x22ef,
    0x31a6, 0x2ed1, 0x3e6a, 0x2ed1, 0x31a6, 0x2ed1, 0x3e6a, 0x2ed1,
    0x25cb, 0x22ef, 0x2ed1, 0x22ef, 0x25cb, 0x22ef, 0x2ed1, 0x22ef,
  },
  {
    0x2492, 0x22e3, 0x2ed0, 0x22e3, 0x2492, 0x22e3, 0x2ed0, 0x22e3,
    0x22e3, 0x2024, 0x2bfb, 0x2024, 0x22e3, 0x2024, 0x2bfb, 0x2024,
    0x2ed0, 0x2bfb, 0x3a41, 0x2bfb, 0x2ed0, 0x2bfb, 0x3a41, 0x2bfb,
    0x22e3, 0x2024, 0x2bfb, 0x2024, 0x22e3, 0x2024, 0x2bfb, 0x2024,
    0x2492, 0x22e3, 0x2ed0, 0x22e3, 0x2492, 0x22e3, 0x2ed0, 0x22e3,
    0x22e3, 0x2024, 0x2bfb, 0x2024, 0x22e3, 0x2024, 0x2bfb, 0x2024,
    0x2ed0, 0x2bfb, 0x3a41, 0x2bfb, 0x2ed0, 0x2bfb, 0x3a41, 0x2bfb,
    0x22e3, 0x2024, 0x2bfb, 0x2024, 0x22e3, 0x2024, 0x2bfb, 0x2024,
  },
  {
    0x2000, 0x1e3c, 0x28f6, 0x1e3c, 0x2000, 0x1e3c, 0x28f6, 0x1e3c,
    0x1e3c, 0x1cb2, 0x2631, 0x1cb2, 0x1e3c, 0x1cb2, 0x2631, 0x1cb2,
    0x28f6, 0x2631, 0x3367, 0x2631, 0x28f6, 0x2631, 0x3367, 0x2631,
    0x1e3c, 0x1cb2, 0x2631, 0x1cb2, 0x1e3c, 0x1cb2, 0x2631, 0x1cb2,
    0x2000, 0x1e3c, 0x28f6, 0x1e3c, 0x2000, 0x1e3c, 0x28f6, 0x1e3c,
    0x1e3c, 0x1cb2, 0x2631, 0x1cb2, 0x1e3c, 0x1cb2, 0x2631, 0x1cb2,
    0x28f6, 0x2631, 0x3367, 0x2631, 0x28f6, 0x2631, 0x3367, 0x2631,
    0x1e3c, 0x1cb2, 0x2631, 0x1cb2, 0x1e3c, 0x1cb2, 0x2631, 0x1cb2,
  },
  {
    0x1c72, 0x1aae, 0x239e, 0x1aae, 0x1c72, 0x1aae, 0x239e, 0x1aae,
    0x1aae, 0x191c, 0x21c0, 0x191c, 0x1aae, 0x191c, 0x21c0, 0x191c,
    0x239e, 0x21c0, 0x2d32, 0x21c0, 0x239e, 0x21c0, 0x2d32, 0x21c0,
    0x1aae, 0x191c, 0x21c0, 0x191c, 0x1aae, 0x191c, 0x21c0, 0x191c,
    0x1c72, 0x1aae, 0x239e, 0x1aae, 0x1c72, 0x1aae, 0x239e, 0x1aae,
    0x1aae, 0x191c, 0x21c0, 0x191c, 0x1aae, 0x191c, 0x21c0, 0x191c,
    0x239e, 0x21c0, 0x2d32, 0x21c0, 0x239e, 0x21c0, 0x2d32, 0x21c0,
    0x1aae, 0x191c, 0x21c0, 0x191c, 0x1aae, 0x191c, 0x21c0, 0x191c,
  }
};



const UChar g_aucScalingMatrixDefault4x4Intra[16] = 
{
   6, 13, 20, 28,
  13, 20, 28, 32,
  20, 28, 32, 37,
  28, 32, 37, 42
};

const UChar g_aucScalingMatrixDefault4x4Inter[16] = 
{
  10, 14, 20, 24,
  14, 20, 24, 27,
  20, 24, 27, 30,
  24, 27, 30, 34
};

const UChar g_aucScalingMatrixDefault8x8Intra[64] = 
{
   6, 10, 13, 16, 18, 23, 25, 27,
  10, 11, 16, 18, 23, 25, 27, 29,
  13, 16, 18, 23, 25, 27, 29, 31,
  16, 18, 23, 25, 27, 29, 31, 33,
  18, 23, 25, 27, 29, 31, 33, 36,
  23, 25, 27, 29, 31, 33, 36, 38,
  25, 27, 29, 31, 33, 36, 38, 40,
  27, 29, 31, 33, 36, 38, 40, 42
};

const UChar g_aucScalingMatrixDefault8x8Inter[64] = 
{
   9, 13, 15, 17, 19, 21, 22, 24,
  13, 13, 17, 19, 21, 22, 24, 25,
  15, 17, 19, 21, 22, 24, 25, 27,
  17, 19, 21, 22, 24, 25, 27, 28,
  19, 21, 22, 24, 25, 27, 28, 30,
  21, 22, 24, 25, 27, 28, 30, 32,
  22, 24, 25, 27, 28, 30, 32, 33,
  24, 25, 27, 28, 30, 32, 33, 35
};



H264AVC_NAMESPACE_END

