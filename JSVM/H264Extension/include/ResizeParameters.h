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
      m_bBaseFrameFromBotFieldFlag = false;
      m_bBaseBotFieldSyncFlag      = false;
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
    Int m_iIntraUpsamplingType;   // 1:lanczos, 2:½ pel + bilin ¼ pel

    Int m_iResampleMode;   // SSUN, 28Nov2005
    
    Bool m_bBaseFrameFromBotFieldFlag;
    Bool m_bBaseBotFieldSyncFlag;
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
    Int   m_aiRefListPoc[2][MAX_REFLIST_SIZE]; 

protected:
    PictureParameters m_acCurrentGop[MAX_PICT_PARAM_NB];

private:
    Void xCleanGopParameters     ( PictureParameters * pc );
    Void xCleanPictureParameters ( PictureParameters * pc );
    Void xInitPictureParameters  ( PictureParameters * pc );
} ;

#undef INTRA_UPSAMPLING_TYPE_DEFAULT
//H264AVC_NAMESPACE_END

#endif 
