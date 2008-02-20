#ifndef _RESIZE_PARAMETERS_H_
#define _RESIZE_PARAMETERS_H_

#include "Typedefs.h"
#include "H264AVCCommonIf.h"

#define MAX_PICT_PARAM_NB          128
#define	MAX_REFLIST_SIZE		   32
//H264AVC_NAMESPACE_BEGIN

#define INTRA_UPSAMPLING_TYPE_DEFAULT     2

#define SST_RATIO_1   0
#define SST_RATIO_2   1
#define SST_RATIO_3_2 2
#define SST_RATIO_X   3

#define ESS_NONE      0
#define ESS_SEQ       1
#define ESS_PICT      2

struct PictureParameters {
    Int  m_iPosX;          // Position     Xorig
    Int  m_iPosY;          //              Yorig
    Int  m_iOutWidth;      // Size of the upsampled baselayer
    Int  m_iOutHeight;     //  
    Int  m_iBaseChromaPhaseX;
    Int  m_iBaseChromaPhaseY;
};


class ResizeParameters {
public:
    ResizeParameters()
    { 
      m_iExtendedSpatialScalability = ESS_NONE;
      m_iSpatialScalabilityType = SST_RATIO_1;
      m_bCrop = false;
  
      m_iChromaPhaseX = -1;  
      m_iChromaPhaseY = 0;
      m_iBaseChromaPhaseX = -1;  
      m_iBaseChromaPhaseY = 0;
      m_iIntraUpsamplingType = INTRA_UPSAMPLING_TYPE_DEFAULT;

      m_iResampleMode = 0;   // SSUN, 28Nov2005
      m_bBaseFrameMbsOnlyFlag      = true;
      m_bBaseIsMbAff               = false;
      m_bFrameMbsOnlyFlag          = true;
      m_bBaseFieldPicFlag          = false;
      m_bFieldPicFlag              = false;
      m_bIsMbAff                   = false;
      m_bBaseBotFieldFlag          = false;
      m_bBotFieldFlag              = false;
      m_bInterlaced								 = false; 

     m_pParamFile = NULL;
#ifndef DOWN_CONVERT_STATIC
    init();
#endif
    };

    Void init ();

    Void  setCurrentPictureParametersWith ( Int index );
    Void setPictureParametersByOffset ( Int iIndex, Int iOl, Int iOr, Int iOt, Int iOb, Int iBcpx, Int iBcpy );
    Void setPictureParametersByValue ( Int index, Int px, Int py, Int ow, Int oh, Int bcpx, Int bcpy );

    const PictureParameters* getCurrentPictureParameters ( Int index ) 
                    { return &m_acCurrentGop[index%MAX_PICT_PARAM_NB];} 
    Int  getLeftOffset   ( Int index ) const 
                    { return (m_acCurrentGop[index%MAX_PICT_PARAM_NB].m_iPosX) /2; }
    Int  getRightOffset  ( Int index ) const 
                     { return (m_iGlobWidth - m_acCurrentGop[index%MAX_PICT_PARAM_NB].m_iPosX - m_acCurrentGop[index%MAX_PICT_PARAM_NB].m_iOutWidth) /2; }
    Int  getTopOffset    ( Int index ) const 
                     { return (m_acCurrentGop[index%MAX_PICT_PARAM_NB].m_iPosY) /2; }
    Int  getBottomOffset ( Int index ) const 
                     { return (m_iGlobHeight - m_acCurrentGop[index%MAX_PICT_PARAM_NB].m_iPosY - m_acCurrentGop[index%MAX_PICT_PARAM_NB].m_iOutHeight) /2; }
    Int  getBaseChromaPhaseX ( Int index ) const 
                     { return m_acCurrentGop[index%MAX_PICT_PARAM_NB].m_iBaseChromaPhaseX; }
    Int  getBaseChromaPhaseY ( Int index ) const 
                     { return m_acCurrentGop[index%MAX_PICT_PARAM_NB].m_iBaseChromaPhaseY; }

    Int  getPoc() const { return m_iPoc; }; 
    Void setPoc( Int iPoc ) { m_iPoc = iPoc; }; 
    ErrVal readPictureParameters ( Int index ); 
    Void initRefListPoc();

    Void print ();

public:
    Int m_iExtendedSpatialScalability;
    Int m_iSpatialScalabilityType;

    Bool m_bCrop;                // if crop is needed

    // ---- Global
    Int  m_iInWidth;       // Size of the baselayer
    Int  m_iInHeight;      //
    Int  m_iGlobWidth;     // Global size  Wenh      if (!bCrop) then it's equal to iOutWidth
    Int  m_iGlobHeight;    //              Henh      if (!bCrop) then it's equal to iOutHeight
    Int m_iChromaPhaseX;
    Int m_iChromaPhaseY;

    // ----- Last Value if by picture
    Int  m_iPosX;          // Position     Xorig
    Int  m_iPosY;          //              Yorig
    Int  m_iOutWidth;      // Size of the upsampled baselayer
    Int  m_iOutHeight;     //  
    Int  m_iBaseChromaPhaseX;
    Int  m_iBaseChromaPhaseY;

    // ----- Intra Upsampling method
    Int m_iIntraUpsamplingType;   // 1:lanczos, 2:?pel + bilin ?pel

    Int m_iResampleMode;   // SSUN, 28Nov2005
    
    Bool m_bBaseFrameMbsOnlyFlag;
    Bool m_bBaseFieldPicFlag;
    Bool m_bBaseBotFieldFlag;
    Bool m_bBaseIsMbAff;

    Bool m_bFrameMbsOnlyFlag;
    Bool m_bFieldPicFlag;
    Bool m_bIsMbAff;
    Bool m_bBotFieldFlag;
    Bool m_bInterlaced; 

    void SetUpSampleMode();

    // ----- PICT LEVEL   
    Int   m_iPoc; 
    FILE  *m_pParamFile; 
    Int   m_aiNumActive[2];
    Int   m_aiRefListPoc[2][MAX_REFLIST_SIZE]; 
  	Int m_level_idc;//jzxu, 02Nov2007

protected:
    PictureParameters m_acCurrentGop[MAX_PICT_PARAM_NB];

private:
    Void xCleanGopParameters     ( PictureParameters * pc );
    Void xCleanPictureParameters ( PictureParameters * pc );
    Void xInitPictureParameters  ( PictureParameters * pc );
} ;


class PosCalcParam
{
public:
  PosCalcParam() : m_iShiftX(0), m_iShiftY(0), m_iScaleX(1), m_iScaleY(1), m_iAddX(0), m_iAddY(0), m_iDeltaX(0), m_iDeltaY(0) {}
  static Int CeilLog2( Int i )
  {
    Int s = 0; i--;
    while( i > 0 )
    {
      s++;
      i >>= 1;
    }
    return s;
  }
public:
  Int m_iShiftX;
  Int m_iShiftY;
  Int m_iScaleX;
  Int m_iScaleY;
  Int m_iAddX;
  Int m_iAddY;
  Int m_iDeltaX;
  Int m_iDeltaY;
};

class MvScaleParam
{
public:
  MvScaleParam( ResizeParameters& rcResizeParameters )
  {
    m_iScaledW              = rcResizeParameters.m_iOutWidth;
    m_iScaledH              = rcResizeParameters.m_iOutHeight;
    m_iRefLayerW            = rcResizeParameters.m_iInWidth;
    m_iRefLayerH            = rcResizeParameters.m_iInHeight;
    m_bFieldPicFlag         = rcResizeParameters.m_bFieldPicFlag;
    m_bRefLayerFieldPicFlag = rcResizeParameters.m_bBaseFieldPicFlag;
    m_bCroppingChangeFlag   = ( rcResizeParameters.m_iExtendedSpatialScalability == ESS_PICT );
    if( m_bCroppingChangeFlag )
    {
      Int iCurrLO = rcResizeParameters.m_iPosX;
      Int iCurrRO = rcResizeParameters.m_iGlobWidth - rcResizeParameters.m_iPosX - rcResizeParameters.m_iOutWidth;
      Int iCurrTO = rcResizeParameters.m_iPosY;
      Int iCurrBO = rcResizeParameters.m_iGlobHeight - rcResizeParameters.m_iPosY - rcResizeParameters.m_iOutHeight;
      m_iLeftOffset    = iCurrLO;
      m_iTopOffset     = iCurrTO;
      m_aiNumActive[0] = rcResizeParameters.m_aiNumActive[0];
      m_aiNumActive[1] = rcResizeParameters.m_aiNumActive[1];
      for( UInt uiIdx0 = 0; uiIdx0 < (UInt)rcResizeParameters.m_aiNumActive[0]; uiIdx0++ )
      {
        Int                      iPoc  = rcResizeParameters.m_aiRefListPoc[0][uiIdx0];
        const PictureParameters* pcRP  = rcResizeParameters.getCurrentPictureParameters( iPoc );
        AOF( pcRP );
        Int iRefLO  = pcRP->m_iPosX;
        Int iRefRO  = rcResizeParameters.m_iGlobWidth - pcRP->m_iPosX - pcRP->m_iOutWidth;
        Int iRefTO  = pcRP->m_iPosY;
        Int iRefBO  = rcResizeParameters.m_iGlobHeight - pcRP->m_iPosY - pcRP->m_iOutHeight;
        m_aaidOX[0][uiIdx0] = iCurrLO - iRefLO;
        m_aaidOY[0][uiIdx0] = iCurrTO - iRefTO;
        m_aaidSW[0][uiIdx0] = iCurrRO - iRefRO + m_aaidOX[0][uiIdx0];
        m_aaidSH[0][uiIdx0] = iCurrBO - iRefBO + m_aaidOY[0][uiIdx0];
      }
      for( UInt uiIdx1 = 0; uiIdx1 < (UInt)rcResizeParameters.m_aiNumActive[1]; uiIdx1++ )
      {
        Int                      iPoc  = rcResizeParameters.m_aiRefListPoc[1][uiIdx1];
        const PictureParameters* pcRP  = rcResizeParameters.getCurrentPictureParameters( iPoc );
        AOF( pcRP );
        Int iRefLO  = pcRP->m_iPosX;
        Int iRefRO  = rcResizeParameters.m_iGlobWidth - pcRP->m_iPosX - pcRP->m_iOutWidth;
        Int iRefTO  = pcRP->m_iPosY;
        Int iRefBO  = rcResizeParameters.m_iGlobHeight - pcRP->m_iPosY - pcRP->m_iOutHeight;
        m_aaidOX[1][uiIdx1] = iCurrLO - iRefLO;
        m_aaidOY[1][uiIdx1] = iCurrTO - iRefTO;
        m_aaidSW[1][uiIdx1] = iCurrRO - iRefRO + m_aaidOX[1][uiIdx1];
        m_aaidSH[1][uiIdx1] = iCurrBO - iRefBO + m_aaidOY[1][uiIdx1];
      }
    }
  }
public:
  Int   m_iScaledW;
  Int   m_iScaledH;
  Int   m_iRefLayerW;
  Int   m_iRefLayerH;
  Int   m_iLeftOffset;
  Int   m_iTopOffset;
  Bool  m_bFieldPicFlag;
  Bool  m_bRefLayerFieldPicFlag;
  Bool  m_bCroppingChangeFlag;
  Int   m_aiNumActive[2];
  Int   m_aaidOX[2][33];
  Int   m_aaidOY[2][33];
  Int   m_aaidSW[2][33];
  Int   m_aaidSH[2][33];
  Int   m_iXMbPos;
  Int   m_iYMbPos;
};

#undef INTRA_UPSAMPLING_TYPE_DEFAULT
//H264AVC_NAMESPACE_END

#endif 
