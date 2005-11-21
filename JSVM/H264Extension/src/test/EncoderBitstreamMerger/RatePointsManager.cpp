/*
********************************************************************************

NOTE - One of the two copyright statements below may be chosen
       that applies for the software.

********************************************************************************
This software module was originally developed by

PATEUX Stéphane (France Télécom)

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

To the extent that France Télécom owns patent rights that would be required to
make, use, or sell the originally developed software module or portions thereof
included in the ISO/IEC 14496-10:2005 Amd.1 (Scalable Video Coding) in a
conforming product, France Télécom will assure the ISO/IEC that it is willing
to negotiate licenses under reasonable and non-discriminatory terms and
conditions with applicants throughout the world.

France Télécom retains full right to modify and use the code for its own
purpose, assign or donate the code to a third party and to inhibit third
parties from using the code for products that do not conform to MPEG-related
ITU Recommendations and/or ISO/IEC International Standards.

This copyright notice must be included in all copies or derivative works.
Copyright (c) ISO/IEC 2005.

********************************************************************************

COPYRIGHT AND WARRANTY INFORMATION

Copyright 2005, International Telecommunications Union, Geneva

The France Télécom hereby donate this source code to the ITU, with the following
understanding:
    1. France Télécom retain the right to do whatever they wish with the
       contributed source code, without limit.
    2. France Télécom retain full patent rights (if any exist) in the technical
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
#include "RatePointsManager.h"

using namespace h264;


void RatePointManager::SetValidPoints(UInt* uiFGSIndex[MAX_FGS_LAYERS], UInt uiNFrames)
{
  // Search for Valid RD points
  m_adValidDisto[0] = m_adDisto[0];
  m_adValidRate[0] = m_adRate[0];
  m_adRDSlope[0] = 0;
  int iValidPosAdd = 1;
  int iPoint;
  m_adRDSlope[0] = 1000000000;
  UInt uiFGSValidIndex[MAX_FGS_LAYERS+1];
  UInt uiFGS;
  for(uiFGS = 0; uiFGS <= MAX_FGS_LAYERS; uiFGS++)
  {
      uiFGSValidIndex[uiFGS] = uiFGS;
  }
  UInt uiMaxFGSLayerUsed = 0;
  Int iCount = 0;
  for(iPoint = 1; iPoint < m_iNbPoint; iPoint++)
  {
      if(m_adRate[iPoint] > 0)
          iCount++;
  }
  iCount = iCount/4;
  uiMaxFGSLayerUsed = iCount+1;
  for(iPoint=1; iPoint < m_iNbPoint; iPoint++)
  {
    bool b_insertionOK=true;
    bool b_rejectPoint=false;
    do 
    {
      m_adValidDisto[iValidPosAdd] = m_adDisto[iPoint];
      m_adValidRate[iValidPosAdd] = m_adRate[iPoint];
      double d_deltaRate = m_adValidRate[iValidPosAdd] - m_adValidRate[iValidPosAdd-1];
      double d_deltaDisto = m_adValidDisto[iValidPosAdd-1] - m_adValidDisto[iValidPosAdd];
      double rdslope;
      if ( (d_deltaDisto<=0) || (d_deltaRate==0) )
        rdslope = -1;
      else
        rdslope = d_deltaDisto/d_deltaRate;
     
	 m_adRDSlope[iValidPosAdd] = rdslope;
     for(uiFGS = 1; uiFGS <= MAX_FGS_LAYERS; uiFGS++)
     {
         if(iPoint <= (Int)uiFGSIndex[uiFGS][uiNFrames] && iPoint > (Int)uiFGSIndex[uiFGS-1][uiNFrames])
             uiFGSValidIndex[uiFGS] = (UInt)iValidPosAdd;
     }
     if (rdslope <0)
      {
        b_rejectPoint=true;
      }
      else if ( (iValidPosAdd>1) && (m_adRDSlope[iValidPosAdd-1]>=m_adRDSlope[iValidPosAdd-2]) ) 
      {
        b_insertionOK=false;
        iValidPosAdd--;
      }
      else 
      {
        b_insertionOK = true;
      }
    } while ( (!b_rejectPoint) && (!b_insertionOK) && (iValidPosAdd>0) );
    if (!b_rejectPoint)
      iValidPosAdd++;
  }
  
  m_iNbValidPoint = iValidPosAdd;
  m_iNbValidPoint = uiMaxFGSLayerUsed;
  for(uiFGS = 1; uiFGS < (UInt)m_iNbValidPoint; uiFGS++)
  {
      double dSlopeFGS = (m_adRDSlope[uiFGSValidIndex[uiFGS]] > 0 ? m_adRDSlope[uiFGSValidIndex[uiFGS]] : m_adRDSlope[uiFGSValidIndex[uiFGS]-1]);
      m_adRDSlope[uiFGS] = dSlopeFGS;
      m_adValidRate[uiFGS] = m_adRate[uiFGSIndex[uiFGS][uiNFrames]];
      printf("dSlope %f ValidRate %f \n",m_adRDSlope[uiFGS], m_adValidRate[uiFGS]);
  }
}



