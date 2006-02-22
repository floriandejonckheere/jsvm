#include <stdio.h>	
#include "ResizeParameters.h"

#define ROTREPORT(x,t) {if(x) {::printf("\n%s\n",t); assert(0); return Err::m_nInvalidParameter;} }

Void
ResizeParameters::init()
{
  xCleanGopParameters(m_acCurrentGop);
  initRefListPoc();
}

Void
ResizeParameters::xCleanGopParameters ( PictureParameters * pc )
{
  for (Int i=0; i<MAX_PICT_PARAM_NB; i++)
  {
    xCleanPictureParameters(&pc[i]);
  }
}

Void
ResizeParameters::xCleanPictureParameters ( PictureParameters * pc )
{
  pc->m_iPosX = -1;
  pc->m_iPosY = -1;
  pc->m_iOutWidth  = -1;
  pc->m_iOutHeight = -1;
  pc->m_iBaseChromaPhaseX = -1;  
  pc->m_iBaseChromaPhaseY = 0;
}


Void
ResizeParameters::xInitPictureParameters ( PictureParameters * pc )
{
  pc->m_iPosX = m_iPosX;
  pc->m_iPosY = m_iPosY;
  pc->m_iOutWidth  = m_iOutWidth;
  pc->m_iOutHeight = m_iOutHeight;
  pc->m_iBaseChromaPhaseX = m_iBaseChromaPhaseX;
  pc->m_iBaseChromaPhaseY = m_iBaseChromaPhaseY;
}

Void
ResizeParameters::setCurrentPictureParametersWith ( Int index )
{
  if (m_iExtendedSpatialScalability < ESS_PICT)
    return;

  PictureParameters * pc;
  pc = &m_acCurrentGop[index % MAX_PICT_PARAM_NB];

  m_iPosX      = pc->m_iPosX;
  m_iPosY      = pc->m_iPosY;
  m_iOutWidth  = pc->m_iOutWidth;
  m_iOutHeight = pc->m_iOutHeight;
  m_iBaseChromaPhaseX = pc->m_iBaseChromaPhaseX;
  m_iBaseChromaPhaseY = pc->m_iBaseChromaPhaseY;
}

Void
ResizeParameters::setPictureParametersByOffset ( Int iIndex, Int iOl, Int iOr, Int iOt, Int iOb, Int iBcpx, Int iBcpy )
{
  setPictureParametersByValue(iIndex,
                              iOl*2, iOt*2,
                              m_iGlobWidth - (iOl*2) - (iOr *2), m_iGlobHeight - (iOt*2) - (iOb *2),
                              iBcpx, iBcpy
                             );
}

Void
ResizeParameters::setPictureParametersByValue ( Int index, Int px, Int py, Int ow, Int oh, Int bcpx, Int bcpy )
{
  if (m_iExtendedSpatialScalability == ESS_PICT)
  {
    m_iPosX       = px;
    m_iPosY       = py;
    m_iOutWidth   = ow;
    m_iOutHeight  = oh;
  }

  m_iBaseChromaPhaseX = bcpx;  
  m_iBaseChromaPhaseY = bcpy;

  PictureParameters * pc = &m_acCurrentGop[index % MAX_PICT_PARAM_NB];

  pc->m_iPosX      = m_iPosX;
  pc->m_iPosY      = m_iPosY;
  pc->m_iOutWidth  = m_iOutWidth;
  pc->m_iOutHeight = m_iOutHeight;
  pc->m_iBaseChromaPhaseX = m_iBaseChromaPhaseX;
  pc->m_iBaseChromaPhaseY = m_iBaseChromaPhaseY;
}

ErrVal
ResizeParameters::readPictureParameters ( Int index )
{
    Int iPosX, iPosY, iOutWidth, iOutHeight;  

    if ( fscanf(m_pParamFile,"%d,%d,%d,%d\n",&iPosX,&iPosY,&iOutWidth,&iOutHeight) == 0 )
    {
        printf("\nResizeParameters::readPictureParameters () : cannot read picture params\n");
        return Err::m_nERR;
    }

    ROTREPORT( iPosX % 2 , "Cropping Window must be even aligned" );
    ROTREPORT( iPosY % 2 , "Cropping Window must be even aligned" );  

    setPictureParametersByValue(index, iPosX, iPosY, iOutWidth, iOutHeight, m_iBaseChromaPhaseX, m_iBaseChromaPhaseY );  
      
    return Err::m_nOK;
}

Void
ResizeParameters::initRefListPoc ()
{
	for (Int i=0; i<MAX_REFLIST_SIZE;i++)
	  {m_aiRefListPoc[0][i]=-1;m_aiRefListPoc[1][i]=-1;}
}

Void
ResizeParameters::print ()
{
  printf("m_iExtendedSpatialScalability = %d\n", m_iExtendedSpatialScalability);
  printf("m_iSpatialScalabilityType     = %d\n", m_iSpatialScalabilityType);
  printf("m_bCrop                       = %d\n", m_bCrop);
  printf("m_iInWidth                    = %d\n", m_iInWidth);
  printf("m_iInHeight                   = %d\n", m_iInHeight);
  printf("m_iGlobWidth                  = %d\n", m_iGlobWidth);
  printf("m_iGlobHeight                 = %d\n", m_iGlobHeight);
  printf("m_iChromaPhaseX               = %d\n", m_iChromaPhaseX);
  printf("m_iChromaPhaseY               = %d\n", m_iChromaPhaseY);
  printf("m_iPosX                       = %d\n", m_iPosX);
  printf("m_iPosY                       = %d\n", m_iPosY);
  printf("m_iOutWidth                   = %d\n", m_iOutWidth);
  printf("m_iOutHeight                  = %d\n", m_iOutHeight);
  printf("m_iBaseChromaPhaseX           = %d\n", m_iBaseChromaPhaseX);
  printf("m_iBaseChromaPhaseY           = %d\n", m_iBaseChromaPhaseY);
  printf("m_iIntraUpsamplingType        = %d\n", m_iIntraUpsamplingType);
}
