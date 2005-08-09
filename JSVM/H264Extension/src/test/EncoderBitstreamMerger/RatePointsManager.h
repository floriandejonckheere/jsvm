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
#ifndef __RATEPOINTSMANAGER_H
#define __RATEPOINTSMANAGER_H

#define MAX_POINT MAX_NUM_RD_LEVELS

class RatePointManager
{
protected:
  int m_iNbPoint;
  double m_adRate[MAX_POINT];
  double m_adDisto[MAX_POINT];
  double maxRate;
  
  int m_iNbValidPoint;
  double m_adValidRate[MAX_POINT];
  double m_adValidDisto[MAX_POINT];
  double m_adRDSlope[MAX_POINT];

public:
  // default constructor
  RatePointManager()
  {
    Reset();
  }

  void Reset()
  {
    m_iNbPoint = 0;
    for(int i=0; i<MAX_POINT; i++)
    {
      m_adRate[i] = m_adDisto[i] = 0.0;
      m_adValidRate[i] = m_adValidDisto[i] = 0.0;
      m_adRDSlope[i] = 0.0;
    }

    m_iNbValidPoint = 0;
    maxRate = 0;
  }

  void SetMaxRate(double mr) { maxRate = mr; }

  double GetQualityLevelMax() { return m_adRDSlope[0]; }

  void Print()
  {
    printf("%d points in buffer\n", m_iNbPoint);
    for(int i=0; i<m_iNbPoint; i++)
      printf("( %f , %f ) \n", m_adRate[i], m_adDisto[i]);
    
  }

  void PrintValidPoints()
  {
    printf("%d valid points:\n", m_iNbValidPoint);
    for(int i=0; i<m_iNbValidPoint; i++)
      printf("( %f , %f ) slope=%f \n ", m_adValidRate[i], m_adValidDisto[i], m_adRDSlope[i]);
  }

  void PushPoint(double R, double D)
  {
    m_adRate[m_iNbPoint] = R;
    m_adDisto[m_iNbPoint] = D;
    m_iNbPoint++;
  }

  void SetValidPoints();

  int getNbValidPoints() { return m_iNbValidPoint;}
  double getValidQualityLevel(int ind) { return m_adRDSlope[ind];}
  double getValidRate(int ind) { return m_adValidRate[ind];}

};


#endif // __RATEPOINTSMANAGER_H