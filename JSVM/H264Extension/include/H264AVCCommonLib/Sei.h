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



#if !defined(AFX_SEI_H__06FFFAD0_FB36_4BF0_9392_395C7389C1F4__INCLUDED_)
#define AFX_SEI_H__06FFFAD0_FB36_4BF0_9392_395C7389C1F4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



#include "H264AVCCommonLib/CommonBuffers.h"
#include "H264AVCCommonLib/HeaderSymbolReadIf.h"
#include "H264AVCCommonLib/HeaderSymbolWriteIf.h"
// JVT-V068 HRD {
#include "H264AVCCommonLib/Vui.h"
#include "H264AVCCommonLib/Hrd.h"
// JVT-V068 HRD }
#include <list>

#define MAX_NUM_LAYER 6



H264AVC_NAMESPACE_BEGIN



class ParameterSetMng;


#if defined( WIN32 )
# pragma warning( disable: 4251 )
#endif


class H264AVCCOMMONLIB_API SEI
{

public:
  enum MessageType
  {
    // JVT-V068 HRD {
    BUFFERING_PERIOD                      = 0,
    PIC_TIMING                            = 1,
    AVC_COMPATIBLE_HRD_SEI                = 30,
//    RESERVED_SEI                          = 31, JVT-W052

//JVT-W052 wxwan
		INTEGRITY_CHECK_SEI												= 31,
		//RESERVED_SEI													=32,//JVT-W049
//JVT-W052 wxwan
//JVT-W049 {
	  REDUNDANT_PIC_SEI                     = 32,
    //RESERVED_SEI                          = 33,//JVT-X032
//JVT-W049 }
    // JVT-V068 HRD }
//JVT-X032 {
    TL_SWITCHING_POINT_SEI                = 33,
    RESERVED_SEI                          = 34,
//JVT-X032 }
    SUB_SEQ_INFO                          = 10,
  MOTION_SEI                            = 18,
    SCALABLE_SEI                          = 22,
    SUB_PIC_SEI                            = 23,
    //{{Quality level estimation and modified truncation- JVTO044 and m12007
    //France Telecom R&D-(nathalie.cammas@francetelecom.com)
    //QUALITYLEVEL_SEI                      = 25,//SEI changes update
  //}}Quality level estimation and modified truncation- JVTO044 and m12007
 PRIORITYLEVEL_SEI                      = 25,//SEI changes update
	// JVT-S080 LMI {
  SCALABLE_SEI_LAYERS_NOT_PRESENT       = 26,
    SCALABLE_SEI_DEPENDENCY_CHANGE        = 27,
    // JVT-T073    RESERVED_SEI                          = 28,
  // JVT-S080 LMI }
    //  JVT-T073 {
  SCENE_INFO_SEI                        = 9,
  SCALABLE_NESTING_SEI                  = 28,
  PR_COMPONENT_INFO_SEI                 = 29,
  //RESERVED_SEI                          = 31, // JVT-V068

    //  JVT-T073 }
    NON_REQUIRED_SEI                      = 24
  };


  class H264AVCCOMMONLIB_API SEIMessage
  {
  public:
    virtual ~SEIMessage()                                                       {}
    MessageType     getMessageType()                                      const { return m_eMessageType; }
    virtual ErrVal  write         ( HeaderSymbolWriteIf* pcWriteIf ) = 0;
    virtual ErrVal  read          ( HeaderSymbolReadIf*   pcReadIf ) = 0;

  protected:
    SEIMessage( MessageType eMessageType) : m_eMessageType( eMessageType ) {}

  private:
    MessageType m_eMessageType;
  };



  class H264AVCCOMMONLIB_API ReservedSei : public SEIMessage
  {
  protected:
    ReservedSei( UInt uiSize = 0 ) : SEIMessage(RESERVED_SEI), m_uiSize(uiSize) {}

  public:
    static ErrVal create( ReservedSei*&         rpcReservedSei,
                          UInt                  uiSize );
    ErrVal        write ( HeaderSymbolWriteIf*  pcWriteIf );
    ErrVal        read  ( HeaderSymbolReadIf*   pcReadIf );

  private:
    UInt m_uiSize;
  };


  class H264AVCCOMMONLIB_API SubSeqInfo : public SEIMessage
  {
  protected:
    SubSeqInfo()
      : SEIMessage(SUB_SEQ_INFO)
      , m_uiSubSeqLayerNum      (0)
      , m_uiSubSeqId            (0)
      , m_bFirstRefPicFlag      (false)
      , m_bLeadingNonRefPicFlag (false)
      , m_bLastPicFlag          (false)
      , m_bSubSeqFrameNumFlag   (false)
      , m_uiSubSeqFrameNum      (0)
    {}

  public:
    static ErrVal create( SubSeqInfo*&          rpcSEIMessage );
    ErrVal        write ( HeaderSymbolWriteIf*  pcWriteIf );
    ErrVal        read  ( HeaderSymbolReadIf*   pcReadIf );
    ErrVal        init  ( UInt                  uiSubSeqLayerNum,
                          UInt                  uiSubSeqId,
                          Bool                  bFirstRefPicFlag,
                          Bool                  bLeadingNonRefPicFlag,
                          Bool                  bLastPicFlag        = false,
                          Bool                  bSubSeqFrameNumFlag = false,
                          UInt                  uiSubSeqFrameNum    = 0 );

    UInt getSubSeqId      ()  const { return m_uiSubSeqId; }
    UInt getSubSeqLayerNum()  const { return m_uiSubSeqLayerNum; }

  private:
    UInt  m_uiSubSeqLayerNum;
    UInt  m_uiSubSeqId;
    Bool  m_bFirstRefPicFlag;
    Bool  m_bLeadingNonRefPicFlag;
    Bool  m_bLastPicFlag;
    Bool  m_bSubSeqFrameNumFlag;
    UInt  m_uiSubSeqFrameNum;
  };

  class H264AVCCOMMONLIB_API ScalableSei: public SEIMessage
  {
  protected:
    ScalableSei ();
    ~ScalableSei();

  public:
    static ErrVal create ( ScalableSei*&      rpcSeiMessage);
//TMM_FIX
    ErrVal    destroy ();
//TMM_FIX
    ErrVal write         ( HeaderSymbolWriteIf  *pcWriteIf);
    ErrVal read           ( HeaderSymbolReadIf    *pcReadIf);

    //Void setTlevelNestingFlag( Bool bFlag )                                   { m_temporal_level_nesting_flag = bFlag; }//SEI changes update
		Void setTlevelNestingFlag( Bool bFlag )                                   { m_temporal_id_nesting_flag = bFlag;                         }//SEI changes update
		Void setPriorityIdSettingFlag( Bool bFlag )                               { m_priority_id_setting_flag = bFlag;                         }//JVT-W053
		Void setPriorityIdSettingUri (UInt index,UChar ucchar )										{ priority_id_setting_uri[index] = ucchar;                    }//JVT-W053
		Void setNumLayersMinus1( UInt ui )                                        { m_num_layers_minus1 = ui;                                   }
    Void setLayerId ( UInt uilayer, UInt uiId )                               { m_layer_id                             [uilayer] = uiId;    }
  //JVT-S036 lsj start
    //Void setFGSlayerFlag ( UInt uilayer, Bool bFlag )                          { m_fgs_layer_flag                        [uilayer] = bFlag;   }
    //Void setSimplePriorityId ( UInt uilayer, UInt uiLevel )                    { m_simple_priority_id                    [uilayer] = uiLevel; }//SEI changes update
    Void setPriorityId ( UInt uilayer, UInt uiLevel )                         { m_priority_id                          [uilayer] = uiLevel; }//SEI changes update
		Void setDiscardableFlag  (UInt uilayer, Bool bFlag)                       { m_discardable_flag                     [uilayer] = bFlag;   }
    Void setTemporalLevel ( UInt uilayer, UInt uiLevel )                      { m_temporal_level                       [uilayer] = uiLevel; }
    Void setDependencyId ( UInt uilayer, UInt uiId )                          { m_dependency_id                        [uilayer] = uiId;    }
    Void setQualityLevel ( UInt uilayer, UInt uiLevel )                        { m_quality_level                       [uilayer] = uiLevel; }
    Void setSubPicLayerFlag ( UInt uilayer, Bool bFlag)                        { m_sub_pic_layer_flag[uilayer] = bFlag; }
    Void setSubRegionLayerFlag ( UInt uilayer, Bool bFlag)                    { m_sub_region_layer_flag                  [uilayer] = bFlag; }
    //Void setIroiSliceDivisionInfoPresentFlag ( UInt uilayer, Bool bFlag )        { m_iroi_slice_division_info_present_flag    [uilayer] = bFlag; }
		Void setIroiSliceDivisionInfoPresentFlag ( UInt uilayer, Bool bFlag )        { m_iroi_division_info_present_flag    [uilayer] = bFlag; }//JVT-W051
    Void setProfileLevelInfoPresentFlag ( UInt uilayer, Bool bFlag)            { m_profile_level_info_present_flag        [uilayer] = bFlag; }
  //JVT-S036 lsj end

    Void setBitrateInfoPresentFlag ( UInt uilayer, Bool bFlag )                { m_bitrate_info_present_flag              [uilayer] = bFlag; }
    Void setFrmRateInfoPresentFlag ( UInt uilayer, Bool bFlag )                { m_frm_rate_info_present_flag            [uilayer] = bFlag; }
    Void setFrmSizeInfoPresentFlag ( UInt uilayer, Bool bFlag )                { m_frm_size_info_present_flag            [uilayer] = bFlag; }
    Void setLayerDependencyInfoPresentFlag ( UInt uilayer, Bool bFlag )        { m_layer_dependency_info_present_flag    [uilayer] = bFlag; }
    //Void setInitParameterSetsInfoPresentFlag ( UInt uilayer, Bool bFlag )      { m_init_parameter_sets_info_present_flag  [uilayer] = bFlag; }//SEI changes update
    Void setParameterSetsInfoPresentFlag ( UInt uilayer, Bool bFlag )       { m_parameter_sets_info_present_flag  [uilayer] = bFlag; }//SEI changes update
    Void setExactInterlayerPredFlag ( UInt uilayer, Bool bFlag )            { m_exact_interlayer_pred_flag  [uilayer] = bFlag; }        //JVT-S036 lsj
		//SEI changes update {
		Void setExactSampleValueMatchFlag ( UInt uilayer, Bool bFlag )            { m_exact_sample_value_match_flag  [uilayer] = bFlag; }
//JVT-W046 {    
		//Void setAvcLayerConversionFlag ( UInt uilayer, Bool bFlag )               { m_avc_layer_conversion_flag [uilayer]         = bFlag;     }
		//Void setAvcInfoFlag ( UInt uilayer, UInt bType, Bool bFlag )              { m_avc_info_flag             [uilayer][bType]  = bFlag;     }
		//Void setAvcConversionTypeIdc ( UInt uilayer, UInt uiIdc )                 { m_avc_conversion_type_idc   [uilayer]         = uiIdc;     } 
		//Void setAvcProfileLevelIdc ( UInt uilayer, UInt bType, Int32 uiIdc )      { m_avc_profile_level_idc     [uilayer][bType]  = uiIdc;     }
		//Void setAvcAvgBitrate ( UInt uilayer, UInt bType, UInt uiBitrate )        { m_avc_avg_bitrate           [uilayer][bType]  = uiBitrate; }
		//Void setAvcMaxBitrate ( UInt uilayer, UInt bType, UInt uiBitrate )        { m_avc_max_bitrate           [uilayer][bType]  = uiBitrate; }
		Void setLayerConversionFlag ( UInt uilayer, Bool bFlag )                        { m_layer_conversion_flag [uilayer]         = bFlag;     }
		Void setRewritingInfoFlag ( UInt uilayer, UInt bType, Bool bFlag )              { m_rewriting_info_flag             [uilayer][bType]  = bFlag;     }
		Void setConversionTypeIdc ( UInt uilayer, UInt uiIdc )                          { m_conversion_type_idc   [uilayer]                   = uiIdc;     } 
		Void setRewritingProfileLevelIdc ( UInt uilayer, UInt bType, Int32 uiIdc )      { m_rewriting_profile_level_idc     [uilayer][bType]  = uiIdc;     }
		Void setRewritingAvgBitrate ( UInt uilayer, UInt bType, UInt uiBitrate )        { m_rewriting_avg_bitrate           [uilayer][bType]  = uiBitrate; }
		Void setRewritingMaxBitrate ( UInt uilayer, UInt bType, UInt uiBitrate )        { m_rewriting_max_bitrate           [uilayer][bType]  = uiBitrate; }
//JVT-W046 }
		//SEI changes update }
		Void setLayerOutputFlag      ( UInt uilayer,Bool bFlag )                       {m_layer_output_flag													[uilayer] = bFlag; }//JVT-W047 wxwan
	//Void setLayerProfileIdc ( UInt uilayer, UInt uiIdc )                      { m_layer_profile_idc                      [uilayer] = uiIdc; }
		
		Void setLayerProfileIdc ( UInt uilayer, UInt uiIdc )                      { m_layer_profile_level_idc                      [uilayer] = uiIdc; }//JVT-W051
  //SEI changes update {
		//  Void setLayerConstraintSet0Flag ( UInt uilayer, Bool bFlag )              { m_layer_constraint_set0_flag            [uilayer] = bFlag; }
  //  Void setLayerConstraintSet1Flag ( UInt uilayer, Bool bFlag )              { m_layer_constraint_set1_flag            [uilayer] = bFlag; }
  //  Void setLayerConstraintSet2Flag ( UInt uilayer, Bool bFlag )              { m_layer_constraint_set2_flag            [uilayer] = bFlag; }
  //  Void setLayerConstraintSet3Flag ( UInt uilayer, Bool bFlag )              { m_layer_constraint_set3_flag            [uilayer] = bFlag; }
  //  Void setLayerLevelIdc ( UInt uilayer, UInt uiIdc )                        { m_layer_level_idc                        [uilayer] = uiIdc; }
   //SEI changes update }
  //JVT-S036 lsj start
    //Void setProfileLevelInfoSrcLayerIdDelta ( UInt uilayer, UInt uiIdc ) { m_profile_level_info_src_layer_id_delta [uilayer] = uiIdc; }//SEI changes update

    Void setAvgBitrate ( UInt uilayer, UInt uiBitrate )                        { m_avg_bitrate                    [uilayer] = uiBitrate; }
    Void setMaxBitrateLayer ( UInt uilayer, UInt uiBitrate )                    { m_max_bitrate_layer                [uilayer] = uiBitrate; }
    //Void setMaxBitrateDecodedPicture ( UInt uilayer, UInt uiBitrate )                { m_max_bitrate_decoded_picture            [uilayer] = uiBitrate; }
		Void setMaxBitrateDecodedPicture ( UInt uilayer, UInt uiBitrate )                { m_max_bitrate_layer_representation            [uilayer] = uiBitrate; }//JVT-W051
    Void setMaxBitrateCalcWindow ( UInt uilayer, UInt uiBitrate )                  { m_max_bitrate_calc_window              [uilayer] = uiBitrate; }
  //JVT-S036 lsj end


    Void setConstantFrmRateIdc ( UInt uilayer, UInt uiFrmrate )                { m_constant_frm_rate_idc                  [uilayer] = uiFrmrate; }
    Void setAvgFrmRate ( UInt uilayer, UInt uiFrmrate )                        { m_avg_frm_rate                          [uilayer] = uiFrmrate; }
    //Void setFrmRateInfoSrcLayerIdDelta( UInt uilayer, UInt uiFrmrate)          { m_frm_rate_info_src_layer_id_delta      [uilayer] = uiFrmrate; } //JVT-S036 lsj SEI changes update
    Void setFrmWidthInMbsMinus1 ( UInt uilayer, UInt uiWidth )                { m_frm_width_in_mbs_minus1                [uilayer] = uiWidth; }
    Void setFrmHeightInMbsMinus1 ( UInt uilayer, UInt uiHeight )              { m_frm_height_in_mbs_minus1              [uilayer] = uiHeight; }
    //Void setFrmSizeInfoSrcLayerIdDelta ( UInt uilayer, UInt uiFrmsize)          { m_frm_size_info_src_layer_id_delta      [uilayer] = uiFrmsize; } //JVT-S036 lsj SEI changes update
    Void setBaseRegionLayerId ( UInt uilayer, UInt uiId )                      { m_base_region_layer_id                  [uilayer] = uiId; }
    Void setDynamicRectFlag ( UInt uilayer, Bool bFlag )                      { m_dynamic_rect_flag                      [uilayer] = bFlag; }
    Void setHorizontalOffset ( UInt uilayer, UInt uiOffset )                  { m_horizontal_offset                      [uilayer] = uiOffset; }
    Void setVerticalOffset ( UInt uilayer, UInt uiOffset )                    { m_vertical_offset                        [uilayer] = uiOffset; }
    Void setRegionWidth ( UInt uilayer, UInt uiWidth )                        { m_region_width                          [uilayer] = uiWidth; }
    Void setRegionHeight ( UInt uilayer, UInt uiHeight )                      { m_region_height                          [uilayer] = uiHeight; }
    //Void setSubRegionInfoSrcLayerIdDelta ( UInt uilayer, UInt uiSubRegion )          { m_sub_region_info_src_layer_id_delta            [uilayer] = uiSubRegion; } //JVT-S036 lsj SEI changes update
  //JVT-S036 lsj start
    Void setRoiId ( UInt uilayer, UInt RoiId )                        { m_roi_id[uilayer]  = RoiId; }
    //Void setIroiSliceDivisionType ( UInt uilayer, UInt bType )                { m_iroi_division_type[uilayer] = bType; }
		Void setIroiGridFlag ( UInt uilayer, Bool bFlag )                { m_iroi_grid_flag[uilayer] = bFlag; }//SEI changes update
    //Void setGridSliceWidthInMbsMinus1 ( UInt uilayer, UInt bWidth )              { m_grid_slice_width_in_mbs_minus1[uilayer] = bWidth; }
    //Void setGridSliceHeightInMbsMinus1 ( UInt uilayer, UInt bHeight )            { m_grid_slice_height_in_mbs_minus1[uilayer] = bHeight; }
		Void setGridSliceWidthInMbsMinus1 ( UInt uilayer, UInt bWidth )              { m_grid_width_in_mbs_minus1[uilayer] = bWidth; }//JVT-W051
		Void setGridSliceHeightInMbsMinus1 ( UInt uilayer, UInt bHeight )            { m_grid_height_in_mbs_minus1[uilayer] = bHeight; }//JVT-W051

    Void setROINum(UInt iDependencyId, UInt iNumROI)      { m_aiNumRoi[iDependencyId] = iNumROI; }
    Void setROIID(UInt iDependencyId, UInt* iROIId)
    {
      for (UInt i =0; i < m_aiNumRoi[iDependencyId]; ++i)
      {
        m_aaiRoiID[iDependencyId][i] = iROIId[i];
      }
    }
    Void setSGID(UInt iDependencyId, UInt* iSGId)
    {
      for (UInt i =0; i < m_aiNumRoi[iDependencyId]; ++i)
      {
        m_aaiSGID[iDependencyId][i] = iSGId[i];
      }
    }
    Void setSLID(UInt iDependencyId, UInt* iSGId)
    {
      for (UInt i =0; i < m_aiNumRoi[iDependencyId]; ++i)
      {
        m_aaiSLID[iDependencyId][i] = iSGId[i];
      }
    }

    // JVT-S054 (REPLACE) ->
    //Void setNumSliceMinus1 ( UInt uilayer, UInt bNum )                     { m_num_rois_minus1[uilayer] = bNum; }
    Void setNumSliceMinus1 ( UInt uilayer, UInt bNum )
    {
      if ( m_num_rois_minus1[uilayer] != bNum )
      {
        if ( m_first_mb_in_roi[uilayer] != NULL )
        {
          free(m_first_mb_in_roi[uilayer]);
          m_first_mb_in_roi[uilayer] = NULL;
        }
        if ( m_roi_width_in_mbs_minus1[uilayer] != NULL )
        {
          free(m_roi_width_in_mbs_minus1[uilayer]);
          m_roi_width_in_mbs_minus1[uilayer] = NULL;
        }
        if ( m_roi_height_in_mbs_minus1[uilayer] != NULL )
        {
          free(m_roi_height_in_mbs_minus1[uilayer]);
          m_roi_height_in_mbs_minus1[uilayer] = NULL;
        }
      }

      m_num_rois_minus1[uilayer] = bNum;

      if ( m_first_mb_in_roi[uilayer] == NULL )
        m_first_mb_in_roi[uilayer] = (UInt*)malloc((bNum+1)*sizeof(UInt));

      if ( m_roi_width_in_mbs_minus1[uilayer] == NULL )
        m_roi_width_in_mbs_minus1[uilayer] = (UInt*)malloc((bNum+1)*sizeof(UInt));

      if ( m_roi_height_in_mbs_minus1[uilayer] == NULL )
        m_roi_height_in_mbs_minus1[uilayer] = (UInt*)malloc((bNum+1)*sizeof(UInt));
      //SEI changes update {
      //if ( sizeof(m_slice_id[uilayer]) != (m_frm_width_in_mbs_minus1[uilayer]+1)*(m_frm_height_in_mbs_minus1[uilayer]+1)*sizeof(UInt) )
      //{
      //  free(m_slice_id[uilayer]);
      //  m_slice_id[uilayer] = NULL;
      //}
      //if ( m_slice_id[uilayer] == NULL )
      //  m_slice_id[uilayer] = (UInt*)malloc((m_frm_width_in_mbs_minus1[uilayer]+1)*(m_frm_height_in_mbs_minus1[uilayer]+1)*sizeof(UInt));
			//SEI changes update }
    }
    // JVT-S054 (REPLACE) <-

    Void setFirstMbInSlice ( UInt uilayer, UInt uiTar, UInt bNum )              { m_first_mb_in_roi[uilayer][uiTar] = bNum; }
    Void setSliceWidthInMbsMinus1 ( UInt uilayer, UInt uiTar, UInt bWidth )          { m_roi_width_in_mbs_minus1[uilayer][uiTar] = bWidth; }
    Void setSliceHeightInMbsMinus1 ( UInt uilayer, UInt uiTar, UInt bHeight )        { m_roi_height_in_mbs_minus1[uilayer][uiTar] = bHeight; }
    //Void setSliceId ( UInt uilayer, UInt uiTar, UInt bId )                  { m_slice_id[uilayer][uiTar] = bId; }//SEI changes update
    //JVT-S036 lsj end
    Void setNumDirectlyDependentLayers ( UInt uilayer, UInt uiNum )            { m_num_directly_dependent_layers          [uilayer] = uiNum; }
    Void setDirectlyDependentLayerIdDeltaMinus1( UInt uilayer, UInt uiTar, UInt uiDelta ) { m_directly_dependent_layer_id_delta_minus1[uilayer][uiTar] = uiDelta;} ///JVT-S036 lsj
    Void setLayerDependencyInfoSrcLayerIdDelta( UInt uilayer, UInt uiDelta )      { m_layer_dependency_info_src_layer_id_delta      [uilayer] = uiDelta;} //JVT-S036 lsj
    //SEI changes update {
		//Void setNumInitSeqParameterSetMinus1 ( UInt uilayer, UInt uiNum )          { m_num_init_seq_parameter_set_minus1      [uilayer] = uiNum; }
		//Void setInitSeqParameterSetIdDelta           ( UInt uilayer, UInt uiSPS, UInt uiTar){ m_init_seq_parameter_set_id_delta        [uilayer][uiSPS] = uiTar;  }
		//Void setNumInitPicParameterSetMinus1         ( UInt uilayer, UInt uiNum )          { m_num_init_pic_parameter_set_minus1      [uilayer] = uiNum; }
    //Void setInitPicParameterSetIdDelta           ( UInt uilayer, UInt uiPPS, UInt uiTar){ m_init_pic_parameter_set_id_delta        [uilayer][uiPPS] = uiTar; }
    //Void setInitParameterSetsInfoSrcLayerIdDelta (UInt uilayer, UInt uiDelta)  { m_init_parameter_sets_info_src_layer_id_delta[uilayer] = uiDelta; } //JVT-S036 lsj
		Void setNumInitSeqParameterSetMinus1         ( UInt uilayer, UInt uiNum )             { m_num_seq_parameter_set_minus1             [uilayer] = uiNum;          }
    Void setInitSeqParameterSetIdDelta           ( UInt uilayer, UInt uiSPS, UInt uiTar)  { m_seq_parameter_set_id_delta               [uilayer][uiSPS] = uiTar;   }
    Void setNumInitSubsetSeqParameterSetMinus1   ( UInt uilayer, UInt uiNum )             { m_num_subset_seq_parameter_set_minus1      [uilayer] = uiNum;          }
    Void setInitSubsetSeqParameterSetIdDelta     ( UInt uilayer, UInt uiSSPS, UInt uiTar) { m_subset_seq_parameter_set_id_delta        [uilayer][uiSSPS] = uiTar;  }
		Void setNumInitPicParameterSetMinus1         ( UInt uilayer, UInt uiNum )             { m_num_pic_parameter_set_minus1             [uilayer] = uiNum;          }
    Void setInitPicParameterSetIdDelta           ( UInt uilayer, UInt uiPPS, UInt uiTar)  { m_pic_parameter_set_id_delta               [uilayer][uiPPS] = uiTar;   }
    Void setInitParameterSetsInfoSrcLayerIdDelta ( UInt uilayer, UInt uiDelta)            { m_parameter_sets_info_src_layer_id_delta   [uilayer] = uiDelta;        } //JVT-S036 lsj
    //SEI changes update }
		// BUG_FIX liuhui{
    Void setStdAVCOffset( UInt uiOffset )                                     { m_std_AVC_Offset = uiOffset;}
    UInt getStdAVCOffset()const { return m_std_AVC_Offset; }
// BUG_FIX liuhui}

    // JVT-U085 LMI
    //Bool getTlevelNestingFlag() const { return m_temporal_level_nesting_flag; }//SEI changes update
		Bool getTlevelNestingFlag() const { return m_temporal_id_nesting_flag; }//SEI changes update
		//JVT-W053 wxwan
		Bool getPriorityIdSettingFlag() const { return m_priority_id_setting_flag ; }
		Char* getPriorityIdSetUri() { return priority_id_setting_uri; }
		UChar getPriorityIdSettingUri (UInt index) const { return priority_id_setting_uri[index]; }
		//JVT-W053 wxwan

    UInt getNumLayersMinus1() const {return m_num_layers_minus1;}
    UInt getLayerId ( UInt uilayer ) const { return m_layer_id[uilayer]; }
   //JVT-S036 lsj start
//    Bool getFGSLayerFlag ( UInt uilayer ) const { return m_fgs_layer_flag[uilayer]; }
    //UInt getSimplePriorityId ( UInt uilayer ) const { return  m_simple_priority_id [uilayer]; }//SEI changes update
    UInt getPriorityId ( UInt uilayer ) const { return  m_priority_id [uilayer]; }//SEI changes update
    Bool getDiscardableFlag  (UInt uilayer) const { return  m_discardable_flag [uilayer]; }
    UInt getTemporalLevel ( UInt uilayer ) const { return m_temporal_level[uilayer]; }
    UInt getDependencyId ( UInt uilayer ) const { return m_dependency_id[uilayer]; }
    UInt getQualityLevel ( UInt uilayer ) const { return m_quality_level[uilayer]; }

    Bool getSubPicLayerFlag ( UInt uilayer ) { return m_sub_pic_layer_flag[uilayer]; }
    Bool getSubRegionLayerFlag ( UInt uilayer ) const { return m_sub_region_layer_flag[uilayer]; }
    //Bool getIroiSliceDivisionInfoPresentFlag ( UInt uilayer ) const { return m_iroi_slice_division_info_present_flag[uilayer]; }
		Bool getIroiSliceDivisionInfoPresentFlag ( UInt uilayer ) const { return m_iroi_division_info_present_flag[uilayer]; }//JVT-W051
    Bool getProfileLevelInfoPresentFlag ( UInt uilayer ) const { return m_profile_level_info_present_flag[uilayer]; }
   //JVT-S036 lsj end
    Bool getBitrateInfoPresentFlag ( UInt uilayer ) const { return m_bitrate_info_present_flag[uilayer]; }
    Bool getFrmRateInfoPresentFlag ( UInt uilayer ) const { return m_frm_rate_info_present_flag[uilayer]; }
    Bool getFrmSizeInfoPresentFlag ( UInt uilayer ) const { return m_frm_size_info_present_flag[uilayer]; }
    Bool getLayerDependencyInfoPresentFlag ( UInt uilayer ) const { return m_layer_dependency_info_present_flag[uilayer]; }
    //Bool getInitParameterSetsInfoPresentFlag ( UInt uilayer ) const { return m_init_parameter_sets_info_present_flag[uilayer]; }//SEI changes update
		Bool getParameterSetsInfoPresentFlag ( UInt uilayer ) const { return m_parameter_sets_info_present_flag[uilayer]; }//SEI changes update

    Bool getExactInterlayerPredFlag ( UInt uilayer )  const { return m_exact_interlayer_pred_flag  [uilayer]; }        //JVT-S036 lsj
		//SEI changes update {
    Bool getExactSampleValueMatchFlag ( UInt uilayer )  const { return m_exact_sample_value_match_flag  [uilayer]; }
		//JVT-W046 {
		//Bool   getAvcLayerConversionFlag ( UInt uilayer )          const { return m_avc_layer_conversion_flag  [uilayer];        }
		//Bool   getAvcInfoFlag ( UInt uilayer, UInt bType )         const { return m_avc_info_flag  [uilayer][bType]; }
		//UInt   getAvcConversionTypeIdc ( UInt uilayer )            const { return m_avc_conversion_type_idc          [uilayer];        }
		//Int32  getAvcProfileLevelIdc ( UInt uilayer, UInt bType )  const { return m_avc_profile_level_idc          [uilayer][bType]; }
		//UInt   getAvcAvgBitrate ( UInt uilayer, UInt bType )       const { return m_avc_avg_bitrate          [uilayer][bType]; }
		//UInt   getAvcMaxBitrate ( UInt uilayer, UInt bType )       const { return m_avc_max_bitrate          [uilayer][bType]; }
		Bool   getLayerConversionFlag ( UInt uilayer )                   const { return m_layer_conversion_flag          [uilayer];        }
		Bool   getRewritingInfoFlag ( UInt uilayer, UInt bType )         const { return m_rewriting_info_flag            [uilayer][bType]; }
		UInt   getConversionTypeIdc ( UInt uilayer )                     const { return m_conversion_type_idc            [uilayer];        }
		Int32  getRewritingProfileLevelIdc ( UInt uilayer, UInt bType )  const { return m_rewriting_profile_level_idc    [uilayer][bType]; }
		UInt   getRewritingAvgBitrate ( UInt uilayer, UInt bType )       const { return m_rewriting_avg_bitrate          [uilayer][bType]; }
		UInt   getRewritingMaxBitrate ( UInt uilayer, UInt bType )       const { return m_rewriting_max_bitrate          [uilayer][bType]; }
		//JVT-W046 }
    //SEI changes update }
		//JVT-W047 wxwan
		Bool getLayerOutputFlag( UInt uilayer ) const { return m_layer_output_flag[uilayer]; }
		//JVT-W047 wxwan
    //UInt getLayerProfileIdc ( UInt uilayer ) const { return m_layer_profile_idc[uilayer]; }
		UInt getLayerProfileIdc ( UInt uilayer ) const { return m_layer_profile_level_idc[uilayer]; }//JVT-W051
		//SEI changes update {
    //Bool getLayerConstraintSet0Flag ( UInt uilayer ) const { return m_layer_constraint_set0_flag[uilayer]; }
    //Bool getLayerConstraintSet1Flag ( UInt uilayer ) const { return m_layer_constraint_set1_flag[uilayer]; }
    //Bool getLayerConstraintSet2Flag ( UInt uilayer ) const { return m_layer_constraint_set2_flag[uilayer]; }
    //Bool getLayerConstraintSet3Flag ( UInt uilayer ) const { return m_layer_constraint_set3_flag[uilayer]; }
    //UInt getLayerLevelIdc ( UInt uilayer ) const { return m_layer_level_idc[uilayer]; }
    //SEI changes update }
  //JVT-S036 lsj start
    //UInt getProfileLevelInfoSrcLayerIdDelta ( UInt uilayer) const { return m_profile_level_info_src_layer_id_delta [uilayer];}//SEI changes update

    UInt getAvgBitrate ( UInt uilayer ) const { return m_avg_bitrate[uilayer]; }
    UInt getMaxBitrateLayer ( UInt uilayer ) const { return m_max_bitrate_layer[uilayer]; }
    //UInt getMaxBitrateDecodedPicture ( UInt uilayer ) const { return m_max_bitrate_decoded_picture[uilayer]; }
		UInt getMaxBitrateDecodedPicture ( UInt uilayer ) const { return m_max_bitrate_layer_representation[uilayer]; }//JVT-W051
    UInt getMaxBitrateCalcWindow ( UInt uilayer ) const { return m_max_bitrate_calc_window[uilayer]; }
  //JVT-S036 lsj end


    UInt getConstantFrmRateIdc ( UInt uilayer ) const { return m_constant_frm_rate_idc[uilayer]; }
    UInt getAvgFrmRate ( UInt uilayer ) const { return m_avg_frm_rate[uilayer]; }
    //UInt getFrmRateInfoSrcLayerIdDelta ( UInt uilayer ) const { return m_frm_rate_info_src_layer_id_delta[uilayer]; } //JVT-S036 lsj SEI changes update
    UInt getFrmWidthInMbsMinus1 ( UInt uilayer ) const { return m_frm_width_in_mbs_minus1[uilayer]; }
    UInt getFrmHeightInMbsMinus1 ( UInt uilayer ) const { return m_frm_height_in_mbs_minus1[uilayer]; }
    //UInt getFrmSizeInfoSrcLayerIdDelta ( UInt uilayer ) const { return m_frm_size_info_src_layer_id_delta[uilayer]; } //JVT-S036 lsj SEI changes update
    UInt getBaseRegionLayerId ( UInt uilayer ) const { return m_base_region_layer_id[uilayer]; }
    Bool getDynamicRectFlag ( UInt uilayer ) const { return m_dynamic_rect_flag[uilayer]; }
    UInt getHorizontalOffset ( UInt uilayer ) const { return m_horizontal_offset[uilayer]; }
    UInt getVerticalOffset ( UInt uilayer ) const { return m_vertical_offset[uilayer]; }
    UInt getRegionWidth ( UInt uilayer ) const { return m_region_width[uilayer]; }
    UInt getRegionHeight ( UInt uilayer ) const { return m_region_height[uilayer]; }
    //UInt getSubRegionInfoSrcLayerIdDelta ( UInt uilayer ) const { return m_sub_region_info_src_layer_id_delta[uilayer]; } ///JVT-S036 lsj SEI changes update
  //JVT-S036 lsj start
    UInt getRoiId ( UInt uilayer ) const { return m_roi_id[uilayer]; }
    //UInt getIroiSliceDivisionType ( UInt uilayer ) const { return m_iroi_division_type[uilayer]; }
		//SEI changes update {
		Bool getIroiGridFlag ( UInt uilayer ) const { return m_iroi_grid_flag[uilayer]; }
    //UInt getGridSliceWidthInMbsMinus1 ( UInt uilayer ) const { return m_grid_slice_width_in_mbs_minus1[uilayer]; }
    //UInt getGridSliceHeightInMbsMinus1 ( UInt uilayer ) const { return m_grid_slice_height_in_mbs_minus1[uilayer]; }
		UInt getGridSliceWidthInMbsMinus1 ( UInt uilayer ) const { return m_grid_width_in_mbs_minus1[uilayer]; }//JVT-W051
		UInt getGridSliceHeightInMbsMinus1 ( UInt uilayer ) const { return m_grid_height_in_mbs_minus1[uilayer]; }//JVT-W051
    UInt getNumSliceMinus1 ( UInt uilayer ) const { return m_num_rois_minus1[uilayer]; }
    UInt getFirstMbInSlice ( UInt uilayer, UInt uiTar )  const { return m_first_mb_in_roi[uilayer][uiTar]; }
    UInt getSliceWidthInMbsMinus1 ( UInt uilayer, UInt uiTar ) const { return m_roi_width_in_mbs_minus1[uilayer][uiTar]; }
    UInt getSliceHeightInMbsMinus1 ( UInt uilayer, UInt uiTar ) const { return m_roi_height_in_mbs_minus1[uilayer][uiTar]; }
    //UInt getSliceId ( UInt uilayer, UInt uiTar ) const { return m_slice_id[uilayer][uiTar]; }
		//SEI changes update }
  //JVT-S036 lsj end

    UInt getNumDirectlyDependentLayers ( UInt uilayer ) const { return m_num_directly_dependent_layers[uilayer]; }
// BUG_FIX liuhui{
    UInt getNumDirectlyDependentLayerIdDeltaMinus1( UInt uilayer, UInt uiIndex ) const { return m_directly_dependent_layer_id_delta_minus1[uilayer][uiIndex]; } //JVT-S036 lsj
// BUG_FIX liuhui}
    UInt getLayerDependencyInfoSrcLayerIdDelta( UInt uilayer ) const { return m_layer_dependency_info_src_layer_id_delta[uilayer];} //JVT-S036 lsj
    //SEI changes update {
    //UInt getNumInitSPSMinus1 ( UInt uilayer ) const { return m_num_init_seq_parameter_set_minus1[uilayer]; }
//    UInt getNumInitPPSMinus1 ( UInt uilayer ) const { return m_num_init_pic_parameter_set_minus1[uilayer]; }
//// BUG_FIX liuhui{
//    UInt getInitSPSIdDelta ( UInt uilayer, UInt uiIndex ) const { return m_init_seq_parameter_set_id_delta[uilayer][uiIndex]; }
//    UInt getInitPPSIdDelta ( UInt uilayer, UInt uiIndex ) const { return m_init_pic_parameter_set_id_delta[uilayer][uiIndex]; }
//// BUG_FIX liuhui}
//    UInt getInitParameterSetsInfoSrcLayerIdDelta ( UInt uilayer ) const { return m_init_parameter_sets_info_src_layer_id_delta[uilayer]; } //JVT-S036 lsj
		UInt getNumInitSPSMinus1 ( UInt uilayer ) const { return m_num_seq_parameter_set_minus1[uilayer];        }
		UInt getNumInitSSPSMinus1( UInt uilayer ) const { return m_num_subset_seq_parameter_set_minus1[uilayer]; }
		UInt getNumInitPPSMinus1 ( UInt uilayer ) const { return m_num_pic_parameter_set_minus1[uilayer];        }
// BUG_FIX liuhui{
    UInt getInitSPSIdDelta  ( UInt uilayer, UInt uiIndex ) const { return m_seq_parameter_set_id_delta[uilayer][uiIndex];        }
		UInt getInitSSPSIdDelta ( UInt uilayer, UInt uiIndex ) const { return m_subset_seq_parameter_set_id_delta[uilayer][uiIndex]; }
    UInt getInitPPSIdDelta  ( UInt uilayer, UInt uiIndex ) const { return m_pic_parameter_set_id_delta[uilayer][uiIndex];        }
// BUG_FIX liuhui}
    UInt getInitParameterSetsInfoSrcLayerIdDelta ( UInt uilayer ) const { return m_parameter_sets_info_src_layer_id_delta[uilayer]; } //JVT-S036 lsj
		//JVT-W051 {
		//Bool getQualityLayerInfoPresentFlag ( void ) const { return m_quality_layer_info_present_flag; }
		Bool getPriorityLayerInfoPresentFlag ( void ) const { return m_priority_layer_info_present_flag; }
		//Bool getBitstreamRestrictionFlag ( UInt uilayer ) const { return m_bitstream_restriction_flag[uilayer]; }
		Bool getBitstreamRestrictionInfoPresentFlag ( UInt uilayer ) const { return m_bitstream_restriction_info_present_flag[uilayer]; }
		//SEI changes update }
		Bool getMotionVectorsOverPicBoundariesFlag ( UInt uilayer ) const { return m_motion_vectors_over_pic_boundaries_flag [uilayer]; }
		UInt getMaxBytesPerPicDenom ( UInt uilayer ) const { return m_max_bytes_per_pic_denom [uilayer]; }
		UInt getMaxBitsPerMbDenom ( UInt uilayer ) const { return m_max_bits_per_mb_denom [uilayer]; }
		UInt getLog2MaxMvLengthHorizontal ( UInt uilayer ) const { return m_log2_max_mv_length_horizontal [uilayer]; }
		UInt getLog2MaxMvLengthVertical ( UInt uilayer ) const { return m_log2_max_mv_length_vertical [uilayer]; }
		UInt getMaxDecFrameBuffering ( UInt uilayer ) const { return m_max_dec_frame_buffering [uilayer]; }
		UInt getNumReorderFrames ( UInt uilayer ) const { return m_num_reorder_frames [uilayer]; }
		//SEI changes update {
    //UInt getQlNumdIdMinus1 ( void ) const { return m_ql_num_dId_minus1; }
		//UInt getQlNumMinus1 ( UInt uilayer ) const { return m_ql_num_minus1 [uilayer]; }
		//UInt getQlDependencyId ( UInt uilayer ) const { return m_ql_dependency_id [uilayer]; }
		//UInt getQlId ( UInt uilayer, UInt uiIndex ) const { return m_ql_id [uilayer][uiIndex]; }
		//Int32 getQlProfileLevelIdc ( UInt uilayer, UInt uiIndex ) const { return m_ql_profile_level_idc [uilayer][uiIndex]; }
		//UInt getQlAvgBitrate ( UInt uilayer, UInt uiIndex ) const { return m_ql_avg_bitrate [uilayer][uiIndex]; }
		//UInt getQlMaxBitrate ( UInt uilayer, UInt uiIndex ) const { return m_ql_max_bitrate [uilayer][uiIndex]; }
    UInt getPrNumdIdMinus1 ( void ) const { return m_pr_num_dId_minus1; }
		UInt getPrNumMinus1 ( UInt uilayer ) const { return m_pr_num_minus1 [uilayer]; }
		UInt getPrDependencyId ( UInt uilayer ) const { return m_pr_dependency_id [uilayer]; }
		UInt getPrId ( UInt uilayer, UInt uiIndex ) const { return m_pr_id [uilayer][uiIndex]; }
		Int32 getPrProfileLevelIdc ( UInt uilayer, UInt uiIndex ) const { return m_pr_profile_level_idc [uilayer][uiIndex]; }
		UInt getPrAvgBitrate ( UInt uilayer, UInt uiIndex ) const { return m_pr_avg_bitrate [uilayer][uiIndex]; }
		UInt getPrMaxBitrate ( UInt uilayer, UInt uiIndex ) const { return m_pr_max_bitrate [uilayer][uiIndex]; }
		//void setQualityLayerInfoPresentFlag ( Bool bFlag ) { m_quality_layer_info_present_flag = bFlag; }	
		void setPriorityLayerInfoPresentFlag ( Bool bFlag ) { m_priority_layer_info_present_flag = bFlag; }
		//void setBitstreamRestrictionFlag ( UInt uilayer, Bool bFlag ){ m_bitstream_restriction_info_present_flag [uilayer] = bFlag; }	
		void setBitstreamRestrictionInfoPresentFlag ( UInt uilayer, Bool bFlag ){ m_bitstream_restriction_info_present_flag [uilayer] = bFlag; }
    //SEI changes update }
		void setMotionVectorsOverPicBoundariesFlag ( UInt uilayer, Bool bFlag ) { m_motion_vectors_over_pic_boundaries_flag [uilayer] = bFlag; }
		void setMaxBytesPerPicDenom ( UInt uilayer, UInt uiMaxBytesPerPicDenom ) { m_max_bytes_per_pic_denom [uilayer] = uiMaxBytesPerPicDenom; }
		void setMaxBitsPerMbDenom ( UInt uilayer, UInt uiMaxBitsPerMbDenom ) { m_max_bits_per_mb_denom [uilayer] = uiMaxBitsPerMbDenom; }
		void setLog2MaxMvLengthHorizontal ( UInt uilayer, UInt uiLog2MaxMvLengthHorizontal ) { m_log2_max_mv_length_horizontal [uilayer] = uiLog2MaxMvLengthHorizontal; }
		void setLog2MaxMvLengthVertical ( UInt uilayer, UInt uiLog2MaxMvLengthVertical ) { m_log2_max_mv_length_vertical [uilayer] = uiLog2MaxMvLengthVertical; }
		void setMaxDecFrameBuffering ( UInt uilayer, UInt uiMaxDecFrameBuffering ) { m_max_dec_frame_buffering [uilayer] = uiMaxDecFrameBuffering; }	
		void setNumReorderFrames ( UInt uilayer, UInt uiNumReorderFrames ) { m_num_reorder_frames [uilayer] = uiNumReorderFrames; }
		//SEI changes update {
    //void setQlNumdIdMinus1 (UInt uiQlNumdIdMinus1) { m_ql_num_dId_minus1 = uiQlNumdIdMinus1; }
		//void setQlNumMinus1 ( UInt uilayer, UInt uiQlNumMinus1 ) { m_ql_num_minus1 [uilayer] = uiQlNumMinus1; }
		//void setQlDependencyId ( UInt uilayer, UInt uiQlDependencyId ) { m_ql_dependency_id [uilayer] = uiQlDependencyId; }
		//void setQlId ( UInt uilayer, UInt uiIndex, UInt uiQlId ) { m_ql_id [uilayer][uiIndex] = uiQlId; }
		//void setQlProfileLevelIdx ( UInt uilayer, UInt uiIndex, Int32 uiQlProfileLevelIdc ) { m_ql_profile_level_idc [uilayer][uiIndex] = uiQlProfileLevelIdc; }
		//void setQlAvgBitrate ( UInt uilayer, UInt uiIndex, UInt uiQlAvgBitrate ) { m_ql_avg_bitrate [uilayer][uiIndex] = uiQlAvgBitrate; }
		//void setQlMaxBitrate ( UInt uilayer, UInt uiIndex, UInt uiQlMaxBitrate ) { m_ql_max_bitrate [uilayer][uiIndex] = uiQlMaxBitrate; }
    void setPrNumdIdMinus1 (UInt uiPrNumdIdMinus1) { m_pr_num_dId_minus1 = uiPrNumdIdMinus1; }
		void setPrNumMinus1 ( UInt uilayer, UInt uiPrNumMinus1 ) { m_pr_num_minus1 [uilayer] = uiPrNumMinus1; }
		void setPrDependencyId ( UInt uilayer, UInt uiPrDependencyId ) { m_pr_dependency_id [uilayer] = uiPrDependencyId; }
		void setPrId ( UInt uilayer, UInt uiIndex, UInt uiPrId ) { m_pr_id [uilayer][uiIndex] = uiPrId; }
		void setPrProfileLevelIdx ( UInt uilayer, UInt uiIndex, Int32 uiPrProfileLevelIdc ) { m_pr_profile_level_idc [uilayer][uiIndex] = uiPrProfileLevelIdc; }
		void setPrAvgBitrate ( UInt uilayer, UInt uiIndex, UInt uiPrAvgBitrate ) { m_pr_avg_bitrate [uilayer][uiIndex] = uiPrAvgBitrate; }
		void setPrMaxBitrate ( UInt uilayer, UInt uiIndex, UInt uiPrMaxBitrate ) { m_pr_max_bitrate [uilayer][uiIndex] = uiPrMaxBitrate; }
    //JVT-W051 }
    //SEI changes update }
 private:
// BUG_FIX liuhui{
    UInt m_std_AVC_Offset;
// BUG_FIX liuhui}
    // JVT-U085 LMI
    //Bool m_temporal_level_nesting_flag;//SEI changes update
    Bool m_temporal_id_nesting_flag;//SEI changes update
		Bool m_priority_id_setting_flag;//JVT-W053
		char priority_id_setting_uri[20];//JVT-W053

    UInt m_num_layers_minus1;
    UInt m_layer_id[MAX_SCALABLE_LAYERS];
  //JVT-S036 lsj start
    //Bool m_fgs_layer_flag[MAX_SCALABLE_LAYERS];
    //UInt m_simple_priority_id[MAX_SCALABLE_LAYERS];//SEI changes
    UInt m_priority_id[MAX_SCALABLE_LAYERS];//SEI changes
    Bool m_discardable_flag[MAX_SCALABLE_LAYERS];
    UInt m_temporal_level[MAX_SCALABLE_LAYERS];
    UInt m_dependency_id[MAX_SCALABLE_LAYERS];
    UInt m_quality_level[MAX_SCALABLE_LAYERS];

    Bool m_sub_pic_layer_flag[MAX_SCALABLE_LAYERS];
    Bool m_sub_region_layer_flag[MAX_SCALABLE_LAYERS];
		//JVT-W051 {
		//rename
		//Bool m_iroi_slice_division_info_present_flag[MAX_SCALABLE_LAYERS];
		Bool m_iroi_division_info_present_flag[MAX_SCALABLE_LAYERS];
		//JVT-W051 }
    Bool m_profile_level_info_present_flag[MAX_SCALABLE_LAYERS];
  //JVT-S036 lsj end
    Bool m_bitrate_info_present_flag[MAX_SCALABLE_LAYERS];
    Bool m_frm_rate_info_present_flag[MAX_SCALABLE_LAYERS];
    Bool m_frm_size_info_present_flag[MAX_SCALABLE_LAYERS];
    Bool m_layer_dependency_info_present_flag[MAX_SCALABLE_LAYERS];
    //Bool m_init_parameter_sets_info_present_flag[MAX_SCALABLE_LAYERS];//SEI changes update
    Bool m_parameter_sets_info_present_flag[MAX_SCALABLE_LAYERS];//SEI changes update
    Bool m_exact_interlayer_pred_flag[MAX_SCALABLE_LAYERS];  //JVT-S036 lsj
		//SEI changes update {
		Bool m_exact_sample_value_match_flag[MAX_SCALABLE_LAYERS];  
	//JVT-W046 {
	//Bool    m_avc_layer_conversion_flag[MAX_SCALABLE_LAYERS];
 // Bool    m_avc_info_flag[MAX_SCALABLE_LAYERS][2];
 // UInt    m_avc_conversion_type_idc[MAX_SCALABLE_LAYERS];
	//Int32   m_avc_profile_level_idc[MAX_SCALABLE_LAYERS][2];
	//UInt  m_avc_avg_bitrate[MAX_SCALABLE_LAYERS][2];
	//UInt  m_avc_max_bitrate[MAX_SCALABLE_LAYERS][2];
		Bool    m_layer_conversion_flag[MAX_SCALABLE_LAYERS];
		Bool    m_rewriting_info_flag[MAX_SCALABLE_LAYERS][2];
		UInt    m_conversion_type_idc[MAX_SCALABLE_LAYERS];
		Int32   m_rewriting_profile_level_idc[MAX_SCALABLE_LAYERS][2];
		UInt    m_rewriting_avg_bitrate[MAX_SCALABLE_LAYERS][2];
		UInt    m_rewriting_max_bitrate[MAX_SCALABLE_LAYERS][2];
 //JVT-W046 }
	//SEI changes update }
		//JVT-W051 {
		//rename
		//UInt m_layer_profile_idc[MAX_SCALABLE_LAYERS];
		Int32 m_layer_profile_level_idc[MAX_SCALABLE_LAYERS];
		//JVT-W051 }
    //SEI changes update {
		//Bool m_layer_constraint_set0_flag[MAX_SCALABLE_LAYERS];
  //  Bool m_layer_constraint_set1_flag[MAX_SCALABLE_LAYERS];
  //  Bool m_layer_constraint_set2_flag[MAX_SCALABLE_LAYERS];
  //  Bool m_layer_constraint_set3_flag[MAX_SCALABLE_LAYERS];
  //  UInt m_layer_level_idc[MAX_SCALABLE_LAYERS];
    //SEI changes update }
  //JVT-S036 lsj start
    //UInt m_profile_level_info_src_layer_id_delta[MAX_SCALABLE_LAYERS]; //SEI changes update



    UInt m_avg_bitrate[MAX_SCALABLE_LAYERS];
    UInt m_max_bitrate_layer[MAX_SCALABLE_LAYERS];//
		//JVT-W051 {
		//rename
		//UInt m_max_bitrate_decoded_picture[MAX_SCALABLE_LAYERS];//
		UInt m_max_bitrate_layer_representation[MAX_SCALABLE_LAYERS];//
		//JVT-W051 } 
    UInt m_max_bitrate_calc_window[MAX_SCALABLE_LAYERS];//

    UInt m_constant_frm_rate_idc[MAX_SCALABLE_LAYERS];
    UInt m_avg_frm_rate[MAX_SCALABLE_LAYERS];

    //UInt m_frm_rate_info_src_layer_id_delta[MAX_SCALABLE_LAYERS];//SEI changes update

    UInt m_frm_width_in_mbs_minus1[MAX_SCALABLE_LAYERS];
    UInt m_frm_height_in_mbs_minus1[MAX_SCALABLE_LAYERS];

    //UInt m_frm_size_info_src_layer_id_delta[MAX_SCALABLE_LAYERS];//SEI changes update

    UInt m_base_region_layer_id[MAX_SCALABLE_LAYERS];
    Bool m_dynamic_rect_flag[MAX_SCALABLE_LAYERS];
    UInt m_horizontal_offset[MAX_SCALABLE_LAYERS];
    UInt m_vertical_offset[MAX_SCALABLE_LAYERS];
    UInt m_region_width[MAX_SCALABLE_LAYERS];
    UInt m_region_height[MAX_SCALABLE_LAYERS];

    //UInt m_sub_region_info_src_layer_id_delta[MAX_SCALABLE_LAYERS];//SEI changes update

    UInt m_roi_id[MAX_SCALABLE_LAYERS]; //
    //SEI changes update {
		//JVT-W051 {
		//rename
		//UInt m_iroi_slice_division_type[MAX_SCALABLE_LAYERS]; //
		//UInt m_grid_slice_width_in_mbs_minus1[MAX_SCALABLE_LAYERS]; //
		//UInt m_grid_slice_height_in_mbs_minus1[MAX_SCALABLE_LAYERS]; //
		//UInt m_num_slice_minus1[MAX_SCALABLE_LAYERS];//
		//UInt m_iroi_division_type[MAX_SCALABLE_LAYERS];
		Bool m_iroi_grid_flag[MAX_SCALABLE_LAYERS];
		UInt m_grid_width_in_mbs_minus1[MAX_SCALABLE_LAYERS]; //
		UInt m_grid_height_in_mbs_minus1[MAX_SCALABLE_LAYERS]; //
		UInt m_num_rois_minus1[MAX_SCALABLE_LAYERS];//
		//JVT-W051 }
    // JVT-S054 (REPLACE) ->
    /*
    UInt m_first_mb_in_roi[MAX_SCALABLE_LAYERS][MAX_SCALABLE_LAYERS];//
    UInt m_roi_width_in_mbs_minus1[MAX_SCALABLE_LAYERS][MAX_SCALABLE_LAYERS];//
    UInt m_roi_height_in_mbs_minus1[MAX_SCALABLE_LAYERS][MAX_SCALABLE_LAYERS];//
    UInt m_slice_id[MAX_SCALABLE_LAYERS][MAX_SCALABLE_LAYERS];//
    */
		//JVT-W051 {
		//rename
		//UInt* m_first_mb_in_slice[MAX_SCALABLE_LAYERS];//
		//UInt* m_slice_width_in_mbs_minus1[MAX_SCALABLE_LAYERS];//
		//UInt* m_slice_height_in_mbs_minus1[MAX_SCALABLE_LAYERS];//
		UInt* m_first_mb_in_roi[MAX_SCALABLE_LAYERS];//
		UInt* m_roi_width_in_mbs_minus1[MAX_SCALABLE_LAYERS];//
		UInt* m_roi_height_in_mbs_minus1[MAX_SCALABLE_LAYERS];//
		//JVT-W051 }
    //UInt* m_slice_id[MAX_SCALABLE_LAYERS];//SEI changes update
    // JVT-S054 (REPLACE) <-
// BUG_FIX liuhui{
    UInt m_num_directly_dependent_layers[MAX_SCALABLE_LAYERS];
    UInt m_directly_dependent_layer_id_delta_minus1[MAX_SCALABLE_LAYERS][MAX_SCALABLE_LAYERS];//

    UInt m_layer_dependency_info_src_layer_id_delta[MAX_SCALABLE_LAYERS];//
//    UInt m_num_init_seq_parameter_set_minus1[MAX_SCALABLE_LAYERS];
//    UInt m_init_seq_parameter_set_id_delta[MAX_SCALABLE_LAYERS][32];
//    UInt m_num_init_pic_parameter_set_minus1[MAX_SCALABLE_LAYERS];
//    UInt m_init_pic_parameter_set_id_delta[MAX_SCALABLE_LAYERS][256];
//// BUG_FIX liuhui}
//    UInt m_init_parameter_sets_info_src_layer_id_delta[MAX_SCALABLE_LAYERS];//
//  //JVT-S036 lsj end
		UInt m_num_seq_parameter_set_minus1[MAX_SCALABLE_LAYERS];
    UInt m_seq_parameter_set_id_delta[MAX_SCALABLE_LAYERS][32];
		UInt m_num_subset_seq_parameter_set_minus1[MAX_SCALABLE_LAYERS];
    UInt m_subset_seq_parameter_set_id_delta[MAX_SCALABLE_LAYERS][32];
    UInt m_num_pic_parameter_set_minus1[MAX_SCALABLE_LAYERS];
    UInt m_pic_parameter_set_id_delta[MAX_SCALABLE_LAYERS][256];
// BUG_FIX liuhui}
    UInt m_parameter_sets_info_src_layer_id_delta[MAX_SCALABLE_LAYERS];//
  //JVT-S036 lsj end
		//SEI changes update }
		Bool m_layer_output_flag[MAX_SCALABLE_LAYERS];//JVT-W047 wxwan

    UInt m_aiNumRoi[MAX_SCALABLE_LAYERS];
    UInt m_aaiRoiID[MAX_SCALABLE_LAYERS][MAX_SCALABLE_LAYERS];
    UInt m_aaiSGID[MAX_SCALABLE_LAYERS][MAX_SCALABLE_LAYERS];
    UInt m_aaiSLID[MAX_SCALABLE_LAYERS][MAX_SCALABLE_LAYERS];
		//JVT-W051 & JVT064 {
		//SEI changes update {
		//Bool m_quality_layer_info_present_flag;
    Bool m_priority_layer_info_present_flag;
		//Bool m_bitstream_restriction_flag[MAX_SCALABLE_LAYERS];
		Bool m_bitstream_restriction_info_present_flag[MAX_SCALABLE_LAYERS];
		//SEI changes update }
		Bool m_motion_vectors_over_pic_boundaries_flag[MAX_SCALABLE_LAYERS];
		UInt m_max_bytes_per_pic_denom[MAX_SCALABLE_LAYERS];
		UInt m_max_bits_per_mb_denom[MAX_SCALABLE_LAYERS];
		UInt m_log2_max_mv_length_horizontal[MAX_SCALABLE_LAYERS];
		UInt m_log2_max_mv_length_vertical[MAX_SCALABLE_LAYERS];
		UInt m_num_reorder_frames[MAX_SCALABLE_LAYERS];
		UInt m_max_dec_frame_buffering[MAX_SCALABLE_LAYERS];		
		//SEI changes update {
    UInt m_pr_num_dId_minus1;
		UInt m_pr_dependency_id[MAX_LAYERS];
		UInt m_pr_num_minus1[MAX_LAYERS];
		UInt m_pr_id[MAX_LAYERS][MAX_QUALITY_LEVELS];
		Int32 m_pr_profile_level_idc[MAX_LAYERS][MAX_QUALITY_LEVELS];
		UInt m_pr_avg_bitrate[MAX_LAYERS][MAX_QUALITY_LEVELS];
		UInt m_pr_max_bitrate[MAX_LAYERS][MAX_QUALITY_LEVELS];
		//JVT-W051 & JVT064 }
    //SEI changes update }
  };

  class H264AVCCOMMONLIB_API SubPicSei : public SEIMessage
  {
  protected:
    SubPicSei ();
    ~SubPicSei();

  public:
    static ErrVal create  ( SubPicSei*&        rpcSeiMessage );
    ErrVal        write    ( HeaderSymbolWriteIf*  pcWriteIf );
    ErrVal        read    ( HeaderSymbolReadIf*    pcReadIf  );

    UInt getLayerId  ()          const  { return m_uiLayerId;        }
    Void setLayerId ( UInt uiLayerId) { m_uiLayerId = uiLayerId;  }

  private:
    UInt m_uiLayerId;
  };

  class H264AVCCOMMONLIB_API MotionSEI : public SEIMessage
  {

  protected:
    MotionSEI();
    ~MotionSEI();

  public:

    UInt m_num_slice_groups_in_set_minus1;
    UInt m_slice_group_id[8];
    Bool m_exact_sample_value_match_flag;
    Bool m_pan_scan_rect_flag;

    static ErrVal create  ( MotionSEI*&         rpcSeiMessage );
    ErrVal        write   ( HeaderSymbolWriteIf*  pcWriteIf );
    ErrVal        read    ( HeaderSymbolReadIf*   pcReadIf );
    ErrVal        setSliceGroupId(UInt id);
  UInt          getSliceGroupId(){return m_slice_group_id[0];}
  };
  //SEI changes update {
  //{{Quality level estimation and modified truncation- JVTO044 and m12007
  //France Telecom R&D-(nathalie.cammas@francetelecom.com)
  //class H264AVCCOMMONLIB_API QualityLevelSEI : public SEIMessage
  //{
  //protected:
  //  QualityLevelSEI ();
  //  ~QualityLevelSEI();

  //public:
  //  static ErrVal create  ( QualityLevelSEI*&         rpcSeiMessage );
  //  ErrVal        write   ( HeaderSymbolWriteIf*  pcWriteIf );
  //  ErrVal        read    ( HeaderSymbolReadIf*   pcReadIf );

  //UInt     getNumLevel() { return m_uiNumLevels;}
  //Void     setNumLevel(UInt ui) { m_uiNumLevels = ui;}
  ////JVT-W137
  ////UInt     getDeltaBytesRateOfLevel(UInt ui) { return m_auiDeltaBytesRateOfLevel[ui];}
  ////Void     setDeltaBytesRateOfLevel(UInt uiIndex, UInt ui) { m_auiDeltaBytesRateOfLevel[uiIndex] = ui;} //~JVT-W137
  //UInt     getQualityLevel(UInt ui) { return m_auiQualityLevel[ui];}
  //Void     setQualityLevel(UInt uiIndex, UInt ui) { m_auiQualityLevel[uiIndex] = ui;}
  //UInt     getDependencyId() { return m_uiDependencyId;}
  //Void     setDependencyId( UInt ui) { m_uiDependencyId = ui;}

  //private:
  //  UInt m_auiQualityLevel[MAX_NUM_RD_LEVELS];
  //  //UInt m_auiDeltaBytesRateOfLevel[MAX_NUM_RD_LEVELS]; JVT-W137 remove
  //  UInt m_uiNumLevels;
  //  UInt m_uiDependencyId;
  //};
  //}}Quality level estimation and modified truncation- JVTO044 and m12007
	class H264AVCCOMMONLIB_API PriorityLevelSEI : public SEIMessage
  {
  protected:
    PriorityLevelSEI ();
    ~PriorityLevelSEI();

  public:
    static ErrVal create  ( PriorityLevelSEI*&         rpcSeiMessage );
    ErrVal        write   ( HeaderSymbolWriteIf*  pcWriteIf );
    ErrVal        read    ( HeaderSymbolReadIf*   pcReadIf );

  UInt     getNumPriorityIds() { return m_uiNumPriorityIds;}
  Void     setNumPriorityIds(UInt ui) { m_uiNumPriorityIds = ui;}
  UInt     getAltPriorityId(UInt ui) { return m_auiAltPriorityId[ui];}
  Void     setAltPriorityId(UInt uiIndex, UInt ui) { m_auiAltPriorityId[uiIndex] = ui;}
  UInt     getPrDependencyId() { return m_uiPrDependencyId;}
  Void     setPrDependencyId( UInt ui) { m_uiPrDependencyId = ui;}

  private:
    UInt m_auiAltPriorityId[MAX_NUM_RD_LEVELS];
    UInt m_uiNumPriorityIds;
    UInt m_uiPrDependencyId;
  };
  //SEI changes update }

  class H264AVCCOMMONLIB_API NonRequiredSei : public SEIMessage
  {
  protected:
    NonRequiredSei ();
    ~NonRequiredSei();

  public:
    static ErrVal create  (NonRequiredSei*&      rpcSeiMessage);
    ErrVal    destroy ();
    ErrVal    write  (HeaderSymbolWriteIf*    pcWriteIf);
    ErrVal    read  (HeaderSymbolReadIf*    pcReadIf);

    UInt      getNumInfoEntriesMinus1()          const{ return m_uiNumInfoEntriesMinus1;}
    UInt      getEntryDependencyId(UInt uiLayer)      const{ return m_uiEntryDependencyId[uiLayer];}
    UInt      getNumNonRequiredPicsMinus1(UInt uiLayer)  const{ return m_uiNumNonRequiredPicsMinus1[uiLayer];}
    UInt      getNonRequiredPicDependencyId(UInt uiLayer, UInt uiNonRequiredLayer)  const{ return m_uiNonRequiredPicDependencyId[uiLayer][uiNonRequiredLayer];}
    UInt      getNonRequiredPicQulityLevel(UInt uiLayer, UInt uiNonRequiredLayer)    const{ return m_uiNonRequiredPicQulityLevel[uiLayer][uiNonRequiredLayer];}
    UInt      getNonRequiredPicFragmentOrder(UInt uiLayer, UInt uiNonRequiredLayer)  const{ return m_uiNonRequiredPicFragmentOrder[uiLayer][uiNonRequiredLayer];}


    Void      setNumInfoEntriesMinus1(UInt ui)          { m_uiNumInfoEntriesMinus1 = ui;}
    Void      setEntryDependencyId(UInt uiLayer, UInt ui)      { m_uiEntryDependencyId[uiLayer] = ui;}
    Void      setNumNonRequiredPicsMinus1(UInt uiLayer, UInt ui)  { m_uiNumNonRequiredPicsMinus1[uiLayer] = ui;}
    Void      setNonNonRequiredPicDependencyId(UInt uiLayer, UInt uiNonRequiredLayer, UInt ui)    {m_uiNonRequiredPicDependencyId[uiLayer][uiNonRequiredLayer] = ui;}
    Void      setNonNonRequiredPicQulityLevel(UInt uiLayer, UInt uiNonRequiredLayer, UInt ui)      {m_uiNonRequiredPicQulityLevel[uiLayer][uiNonRequiredLayer] = ui;}
    Void      setNonNonRequiredPicFragmentOrder(UInt uiLayer, UInt uiNonRequiredLayer, UInt ui)    {m_uiNonRequiredPicFragmentOrder[uiLayer][uiNonRequiredLayer] = ui;}


  private:
    UInt    m_uiNumInfoEntriesMinus1;
    UInt    m_uiEntryDependencyId[MAX_NUM_INFO_ENTRIES];
    UInt    m_uiNumNonRequiredPicsMinus1[MAX_NUM_INFO_ENTRIES];
    UInt    m_uiNonRequiredPicDependencyId[MAX_NUM_INFO_ENTRIES][MAX_NUM_NON_REQUIRED_PICS];
    UInt    m_uiNonRequiredPicQulityLevel[MAX_NUM_INFO_ENTRIES][MAX_NUM_NON_REQUIRED_PICS];
    UInt    m_uiNonRequiredPicFragmentOrder[MAX_NUM_INFO_ENTRIES][MAX_NUM_NON_REQUIRED_PICS];
  };//shenqiu 05-09-15

  // JVT-S080 LMI {
  class H264AVCCOMMONLIB_API ScalableSeiLayersNotPresent: public SEIMessage
  {
  protected:
      ScalableSeiLayersNotPresent ();
   ~ScalableSeiLayersNotPresent();

  public:
      static ErrVal create ( ScalableSeiLayersNotPresent*&      rpcSeiMessage);
   //TMM_FIX
      ErrVal destroy ();
   //TMM_FIX
      ErrVal write         ( HeaderSymbolWriteIf  *pcWriteIf);
      ErrVal read           ( HeaderSymbolReadIf    *pcReadIf);
      Void setNumLayers( UInt ui )                                        { m_uiNumLayers = ui;  }
      Void setLayerId ( UInt uiLayer, UInt uiId )                                { m_auiLayerId                              [uiLayer] = uiId; }
    Void setOutputFlag ( Bool bFlag )  { m_bOutputFlag = bFlag; }

      UInt getNumLayers() const {return m_uiNumLayers;}
      UInt getLayerId ( UInt uiLayer ) const { return m_auiLayerId[uiLayer]; }
    Bool getOutputFlag ( ) const { return m_bOutputFlag; }
      static UInt m_uiLeftNumLayers;
      static UInt m_auiLeftLayerId[MAX_SCALABLE_LAYERS];

  private:
      UInt m_uiNumLayers;
      UInt m_auiLayerId[MAX_SCALABLE_LAYERS];
    Bool m_bOutputFlag;

  };

  class H264AVCCOMMONLIB_API ScalableSeiDependencyChange: public SEIMessage
  {
  protected:
      ScalableSeiDependencyChange ();
   ~ScalableSeiDependencyChange();

  public:
      static ErrVal create ( ScalableSeiDependencyChange*&      rpcSeiMessage);
      ErrVal write         ( HeaderSymbolWriteIf  *pcWriteIf);
      ErrVal read           ( HeaderSymbolReadIf    *pcReadIf);
      Void setNumLayersMinus1( UInt ui )                                        { m_uiNumLayersMinus1 = ui;  }
      Void setLayerId ( UInt uiLayer, UInt uiId )                                { m_auiLayerId                              [uiLayer] = uiId; }
    Void setLayerDependencyInfoPresentFlag ( UInt uiLayer, Bool bFlag ) { m_abLayerDependencyInfoPresentFlag[uiLayer] = bFlag; }
      Void setNumDirectDependentLayers ( UInt uiLayer, UInt ui ) { m_auiNumDirectDependentLayers[uiLayer] = ui; }
    Void setDirectDependentLayerIdDeltaMinus1( UInt uiLayer, UInt uiDirectLayer, UInt uiIdDeltaMinus1 )  { m_auiDirectDependentLayerIdDeltaMinus1[uiLayer][uiDirectLayer] = uiIdDeltaMinus1; }
    Void setLayerDependencyInfoSrcLayerIdDeltaMinus1 ( UInt uiLayer, UInt uiIdDeltaMinus1 ) { m_auiLayerDependencyInfoSrcLayerIdDeltaMinus1[uiLayer] = uiIdDeltaMinus1; }
    Void setOutputFlag ( Bool bFlag )  { m_bOutputFlag = bFlag; }

      UInt getNumLayersMinus1() const {return m_uiNumLayersMinus1;}
      UInt getLayerId ( UInt uiLayer ) const { return m_auiLayerId[uiLayer]; }
      UInt getNumDirectDependentLayers ( UInt uiLayer ) const { return m_auiNumDirectDependentLayers[uiLayer]; }
    UInt getDirectDependentLayerIdDeltaMinus1( UInt uiLayer, UInt uiDirectLayer ) const { return m_auiDirectDependentLayerIdDeltaMinus1[uiLayer][uiDirectLayer]; }
    UInt getLayerDependencyInfoSrcLayerIdDeltaMinus1 ( UInt uiLayer ) const { return m_auiLayerDependencyInfoSrcLayerIdDeltaMinus1[uiLayer]; }
    Bool getLayerDependencyInfoPresentFlag ( UInt uiLayer ) const { return m_abLayerDependencyInfoPresentFlag[uiLayer]; }
    Bool getOutputFlag ( ) const { return m_bOutputFlag; }

  private:
      UInt m_uiNumLayersMinus1;
      UInt m_auiLayerId[MAX_SCALABLE_LAYERS];
      UInt m_auiNumDirectDependentLayers[MAX_SCALABLE_LAYERS];
      UInt m_auiDirectDependentLayerIdDeltaMinus1[MAX_SCALABLE_LAYERS][MAX_SCALABLE_LAYERS];
      UInt m_auiLayerDependencyInfoSrcLayerIdDeltaMinus1[MAX_SCALABLE_LAYERS];
    Bool m_abLayerDependencyInfoPresentFlag[MAX_SCALABLE_LAYERS];
    Bool m_bOutputFlag;
  };

  // JVT-S080 LMI }
// JVT-T073 {
#define MAX_PICTURES_IN_ACCESS_UNIT 50
  class H264AVCCOMMONLIB_API ScalableNestingSei : public SEIMessage
  {
  protected:
    ScalableNestingSei()
      : SEIMessage(SCALABLE_NESTING_SEI)
      , m_bAllPicturesInAuFlag  (0)
    , m_uiNumPictures         (0)
    , m_pcSEIMessage          (NULL)
    {}

  public:
    static ErrVal create( ScalableNestingSei*&  rpcSEIMessage );
  ErrVal      destroy();
    ErrVal        write ( HeaderSymbolWriteIf*  pcWriteIf );
    ErrVal        read  ( HeaderSymbolReadIf*   pcReadIf );
    ErrVal        init  ( Bool                  m_bAllPicturesInAuFlag,
                        UInt                  m_uiNumPictures,
                        UInt*                 m_auiDependencyId,
                        UInt*                 m_auiQualityLevel
            );

    Bool getAllPicturesInAuFlag()  const { return m_bAllPicturesInAuFlag; }
    UInt getNumPictures()          const { return m_uiNumPictures; }
  UInt getDependencyId( UInt uiIndex ) { return m_auiDependencyId[uiIndex]; }
  UInt getQualityLevel( UInt uiIndex ) { return m_auiQualityLevel[uiIndex]; }

  Void setAllPicturesInAuFlag( Bool bFlag ) { m_bAllPicturesInAuFlag = bFlag; }
  Void setNumPictures( UInt uiNum ) { m_uiNumPictures = uiNum; }
  Void setDependencyId( UInt uiIndex, UInt uiValue ) { m_auiDependencyId[uiIndex] = uiValue; }
  Void setQualityLevel( UInt uiIndex, UInt uiValue ) { m_auiQualityLevel[uiIndex] = uiValue; }

    // JVT-V068 {
    UInt getTemporalLevel() { return m_uiTemporalLevel; }
    Void setTemporalLevel( UInt uiValue ) { m_uiTemporalLevel = uiValue; }
    // JVT-V068 }
  
  private:
    Bool  m_bAllPicturesInAuFlag;
    UInt  m_uiNumPictures;
    UInt  m_auiDependencyId[MAX_PICTURES_IN_ACCESS_UNIT];
    UInt  m_auiQualityLevel[MAX_PICTURES_IN_ACCESS_UNIT];
    // JVT-V068 {
    UInt  m_uiTemporalLevel;
    // JVT-V068 }
    SEIMessage *m_pcSEIMessage;
  };
  //scene_info is taken as en example
  class H264AVCCOMMONLIB_API SceneInfoSei : public SEIMessage
  {
  protected:
    SceneInfoSei() : SEIMessage(SCENE_INFO_SEI)
    {}
  public:
    static ErrVal create( SceneInfoSei*& rpcSceneInfoSei );
    ErrVal    destroy ();
    ErrVal        write ( HeaderSymbolWriteIf*  pcWriteIf);
      ErrVal        read  ( HeaderSymbolReadIf*   pcReadIf );

    Bool getSceneInfoPresentFlag() const { return m_bSceneInfoPresentFlag; }
    UInt getSceneId()              const { return m_uiSceneId; }
    UInt getSceneTransitionType()  const { return m_uiSceneTransitionType; }
    UInt getSecondSceneId()        const { return m_uiSecondSceneId; }
    Void setSceneInfoPresentFlag( Bool bFlag )          { m_bSceneInfoPresentFlag = bFlag; }
    Void setSceneId( UInt uiSceneId )                   { m_uiSceneId = uiSceneId; }
    Void setSceneTransitionType( UInt uiTransitionType) { m_uiSceneTransitionType = uiTransitionType; }
    Void setSecondSceneId( UInt uiSecondId )            { m_uiSecondSceneId = uiSecondId; }
  private:
    Bool m_bSceneInfoPresentFlag;
    UInt m_uiSceneId;
    UInt m_uiSceneTransitionType;
    UInt m_uiSecondSceneId;
  };
  // JVT-T073 }

  #define MAX_SLICE_NUM 4
  class H264AVCCOMMONLIB_API PRComponentInfoSei : public SEIMessage
  {
  protected:
    PRComponentInfoSei() : SEIMessage(PR_COMPONENT_INFO_SEI) {}

  public:
    static ErrVal create ( PRComponentInfoSei*& rpcSeiMessage);
    ErrVal destroy       ();
    ErrVal write         ( HeaderSymbolWriteIf  *pcWriteIf);
    ErrVal read          ( HeaderSymbolReadIf   *pcReadIf);

    Void setNumDependencyIdMinus1( UInt uiNum         ) { m_uiNumDependencyIdMinus1 = uiNum; }
    Void setPrDependencyId       ( UInt* auiPrDepId   ) { memcpy( m_uiPrDependencyId,        auiPrDepId,   sizeof(UInt)*MAX_LAYERS                                  ); }
    Void setNumQualLevelMinus1   ( UInt* auiNumQL     ) { memcpy( m_uiNumQualityLevelMinus1, auiNumQL,     sizeof(UInt)*MAX_LAYERS                                  ); }
    Void setPrQualLevel          ( UInt* aauiQL       ) { memcpy( m_uiPrQualityLevel,        aauiQL,       sizeof(UInt)*MAX_LAYERS*MAX_QUALITY_LEVELS               ); }
    Void setNumSlice             ( UInt* aauiNumSlice ) { memcpy( m_uiNumPrSliceMinus1,      aauiNumSlice, sizeof(UInt)*MAX_LAYERS*MAX_QUALITY_LEVELS               ); }
    Void setChromaOffset         ( UInt* aaauiOffset  ) { memcpy( m_uiChromaOffset,          aaauiOffset,  sizeof(UInt)*MAX_LAYERS*MAX_QUALITY_LEVELS*MAX_SLICE_NUM ); }

  private:
    UInt m_uiNumDependencyIdMinus1;
    UInt m_uiPrDependencyId       [MAX_LAYERS];
    UInt m_uiNumQualityLevelMinus1[MAX_LAYERS];
    UInt m_uiPrQualityLevel       [MAX_LAYERS][MAX_QUALITY_LEVELS];
    UInt m_uiNumPrSliceMinus1     [MAX_LAYERS][MAX_QUALITY_LEVELS];
    UInt m_uiChromaOffset         [MAX_LAYERS][MAX_QUALITY_LEVELS][MAX_SLICE_NUM];
  };

  // JVT-V068 HRD {
  class H264AVCCOMMONLIB_API BufferingPeriod :
    public SEIMessage
  {

  public:
    class H264AVCCOMMONLIB_API SchedSel
    {
    public:
      SchedSel() : m_uiInitialCpbRemovalDelay(0), m_uiInitialCpbRemovalDelayOffset(0) {}
      SchedSel& operator = (const SchedSel& rcSchedSel)
      {
        m_uiInitialCpbRemovalDelay = rcSchedSel.m_uiInitialCpbRemovalDelay;
        m_uiInitialCpbRemovalDelayOffset = rcSchedSel.m_uiInitialCpbRemovalDelayOffset;
        return *this;
      }
      ErrVal write( HeaderSymbolWriteIf* pcWriteIf, const HRD& rcHrd );
      ErrVal read ( HeaderSymbolReadIf* pcReadIf,   const HRD& rcHrd );

      UInt getDelay() { return m_uiInitialCpbRemovalDelay; }
      UInt getDelayOffset() { return m_uiInitialCpbRemovalDelayOffset; }
      ErrVal setDelay( UInt uiInitialCpbRemovalDelay)
      {
        m_uiInitialCpbRemovalDelay = uiInitialCpbRemovalDelay;
        return Err::m_nOK;
      }
      ErrVal setDelayOffset( UInt uiInitialCpbRemovalDelayOffset)
      {
        m_uiInitialCpbRemovalDelayOffset = uiInitialCpbRemovalDelayOffset;
        return Err::m_nOK;
      }

    private:
      UInt m_uiInitialCpbRemovalDelay;
      UInt m_uiInitialCpbRemovalDelayOffset;
    };

  protected:
    BufferingPeriod( ParameterSetMng*& rpcParameterSetMng ): SEIMessage( BUFFERING_PERIOD ), m_pcParameterSetMng( rpcParameterSetMng), m_uiSeqParameterSetId( 0 ) {}

  public:
    static ErrVal create( BufferingPeriod*& rpcBufferingPeriod, ParameterSetMng*& rpcParameterSetMng );
    static ErrVal create( BufferingPeriod*& rpcBufferingPeriod, BufferingPeriod * pcBufferingPeriod );

    virtual ~BufferingPeriod();
    SchedSel& getSchedSel( HRD::HrdParamType eHrdParamType, UInt uiNum ) { return m_aacSchedSel[eHrdParamType].get( uiNum ); }
    ErrVal write( HeaderSymbolWriteIf* pcWriteIf );
    ErrVal read ( HeaderSymbolReadIf* pcReadIf );

    ErrVal setHRD( UInt uiSPSId, const HRD* apcHrd[] );

  private:
    const HRD* m_apcHrd[2];
    ParameterSetMng* m_pcParameterSetMng;

    UInt m_uiSeqParameterSetId;
    Bool m_bHrdParametersPresentFlag[2];
    StatBuf <DynBuf< SchedSel >, 2>  m_aacSchedSel;
  }; 

  class H264AVCCOMMONLIB_API PicTiming :
    public SEIMessage
  {

  public:
    class H264AVCCOMMONLIB_API ClockTimestamp
    {

    public:
      ClockTimestamp()
        : m_bClockTimestampFlag ( false )
        , m_uiCtType            ( MSYS_UINT_MAX )
        , m_bNuitFieldBasedFlag ( false )
        , m_uiCountingType      ( MSYS_UINT_MAX )
        , m_bFullTimestampFlag  ( false )
        , m_bDiscontinuityFlag  ( false )
        , m_bCntDroppedFlag     ( false )
        , m_uiNFrames           ( MSYS_UINT_MAX )
        , m_uiSeconds           ( 0 )
        , m_uiMinutes           ( 0 )
        , m_uiHours             ( 0 )
        , m_iTimeOffset         ( 0 )
      {}

      ErrVal write( HeaderSymbolWriteIf* pcWriteIf, const HRD& rcHRD );
      ErrVal read ( HeaderSymbolReadIf* pcReadIf,   const HRD& rcHRD );
      Int  get( const VUI& rcVUI, UInt uiLayerIndex );
      Void set( const VUI& rcVUI, UInt uiLayerIndex, Int iTimestamp );
    private:
      Bool m_bClockTimestampFlag;
      UInt m_uiCtType;
      Bool m_bNuitFieldBasedFlag;
      UInt m_uiCountingType;
      Bool m_bFullTimestampFlag;
      Bool m_bDiscontinuityFlag;
      Bool m_bCntDroppedFlag;
      UInt m_uiNFrames;
      UInt m_uiSeconds;
      UInt m_uiMinutes;
      UInt m_uiHours;
      Int m_iTimeOffset;
    };

  protected:
    PicTiming( const VUI& rcVUI, UInt uiLayerIndex ) 
      : SEIMessage          ( PIC_TIMING )
      , m_rcVUI             ( rcVUI )
      , m_uiLayerIndex      ( uiLayerIndex )
      , m_uiCpbRemovalDelay ( MSYS_UINT_MAX )
      , m_uiDpbOutputDelay  ( MSYS_UINT_MAX )
      , m_ePicStruct        ( PS_NOT_SPECIFIED ) 
    {}

  public:
    static ErrVal create( PicTiming*& rpcPicTiming, const VUI* pcVUI, UInt uiLayerIndex );
    static ErrVal create( PicTiming*& rpcPicTiming, ParameterSetMng* parameterSetMng, UInt uiSPSId, UInt uiLayerIndex );

    Int  getTimestamp( UInt uiNum = 0, UInt uiLayerIndex = 0 );
    ErrVal setTimestamp( UInt uiNum, UInt uiLayerIndex, Int iTimestamp );

    Int  getCpbRemovalDelay() { return m_uiCpbRemovalDelay; } 
    ErrVal setCpbRemovalDelay( UInt uiCpbRemovalDelay ); 

    UInt  getDpbOutputDelay() { return m_uiDpbOutputDelay; }                
    ErrVal setDpbOutputDelay( UInt uiDpbOutputDelay ); 

    UInt getNumClockTs();
    ErrVal write( HeaderSymbolWriteIf* pcWriteIf );
    ErrVal read ( HeaderSymbolReadIf* pcReadIf );
    PicStruct getPicStruct() const               { return m_ePicStruct; }
    Void      setPicStruct(PicStruct ePicStruct) { m_ePicStruct = ePicStruct; }

  private:
    const VUI& m_rcVUI;
    UInt m_uiLayerIndex;

    UInt m_uiCpbRemovalDelay;
    UInt m_uiDpbOutputDelay;
    PicStruct m_ePicStruct;
    StatBuf< ClockTimestamp, 3 > m_acClockTimestampBuf;
  };

  class H264AVCCOMMONLIB_API AVCCompatibleHRD :
    public SEIMessage
  {
  public:

  protected:
    AVCCompatibleHRD( VUI& rcVUI ) 
      : SEIMessage          ( AVC_COMPATIBLE_HRD_SEI )
      , m_rcVUI             ( rcVUI )
    {}

  public:
    static ErrVal create( AVCCompatibleHRD*& rpcAVCCompatibleHRD, VUI* pcVUI );

    ErrVal write( HeaderSymbolWriteIf* pcWriteIf );
    ErrVal read ( HeaderSymbolReadIf* pcReadIf );

  private:
    VUI& m_rcVUI;
  };
  // JVT-V068 HRD }

//JVT-W049 {
  class H264AVCCOMMONLIB_API RedundantPicSei: public SEIMessage
{
protected:
    RedundantPicSei ();
    ~RedundantPicSei ();

public:
    static ErrVal create  ( RedundantPicSei*&      rpcSeiMessage );
    ErrVal        write   ( HeaderSymbolWriteIf*   pcWriteIf     );
    ErrVal        read    ( HeaderSymbolReadIf*    pcReadIf      );

  Void setNumDIdMinus1( UInt ui )                                                                       { m_num_dId_minus1 = ui;                                                    }
	Void setDependencyId ( UInt uidId, UInt uiId )                                                        { m_dependency_id[uidId] = uiId;                                            }
  Void setNumQIdMinus1 ( UInt uidId, UInt uiId )                                                        { m_num_qId_minus1[uidId] = uiId;                                           }
	Void setQualityId ( UInt uidId, UInt uiqId, UInt uiId )                                               { m_quality_id[uidId][uiqId] = uiId;                                        }
	Void setNumRedundantPicsMinus1 ( UInt uidId, UInt uiqId, UInt uiredupic )                             { m_num_redundant_pics_minus1[uidId][uiqId] = uiredupic;                    }
	Void setRedundantPicCntMinus1 ( UInt uidId, UInt uiqId, UInt uiredupic,UInt uiredupicnt )             { m_redundant_pic_cnt_minus1[uidId][uiqId][uiredupic] = uiredupicnt;        }
	Void setPicMatchFlag ( UInt uidId, UInt uiqId, UInt uiredupic, Bool bFlag )                           { m_pic_match_flag[uidId][uiqId][uiredupic] = bFlag;                        }
	Void setMbTypeMatchFlag ( UInt uidId, UInt uiqId, UInt uiredupic, Bool bFlag )                        { m_mb_type_match_flag[uidId][uiqId][uiredupic] = bFlag;                    }
	Void setMotionMatchFlag ( UInt uidId, UInt uiqId, UInt uiredupic, Bool bFlag )                        { m_motion_match_flag[uidId][uiqId][uiredupic] = bFlag;                     }
	Void setResidualMatchFlag ( UInt uidId, UInt uiqId, UInt uiredupic, Bool bFlag )                      { m_residual_match_flag[uidId][uiqId][uiredupic] = bFlag;                   }
	Void setIntraSamplesMatchFlag ( UInt uidId, UInt uiqId, UInt uiredupic, Bool bFlag )                  { m_intra_samples_match_flag[uidId][uiqId][uiredupic] = bFlag;              }
    
	UInt getNumDIdMinus1( )                                                                         const { return m_num_dId_minus1;                                                  }
	UInt getDependencyId ( UInt uidId )                                                             const { return m_dependency_id[uidId];                                            }	
  UInt getNumQIdMinus1 ( UInt uidId )                                                             const { return m_num_qId_minus1[uidId];                                           }
	UInt getQualityId ( UInt uidId, UInt uiqId )                                                    const { return m_quality_id[uidId][uiqId];                                        }
	UInt getNumRedundantPicsMinus1 ( UInt uidId, UInt uiqId )                                       const { return m_num_redundant_pics_minus1[uidId][uiqId];                         }
	UInt getRedundantPicCntMinus1 ( UInt uidId, UInt uiqId, UInt uiredupic )                        const { return m_redundant_pic_cnt_minus1[uidId][uiqId][uiredupic];               }
	Bool getPicMatchFlag ( UInt uidId, UInt uiqId, UInt uiredupic )                                 const { return m_pic_match_flag[uidId][uiqId][uiredupic];                         }
	Bool getMbTypeMatchFlag ( UInt uidId, UInt uiqId, UInt uiredupic )                              const { return m_mb_type_match_flag[uidId][uiqId][uiredupic];                     }
	Bool getMotionMatchFlag ( UInt uidId, UInt uiqId, UInt uiredupic )                              const { return m_motion_match_flag[uidId][uiqId][uiredupic];                      }
	Bool getResidualMatchFlag ( UInt uidId, UInt uiqId, UInt uiredupic )                            const { return m_residual_match_flag[uidId][uiqId][uiredupic];                    }
	Bool getIntraSamplesMatchFlag ( UInt uidId, UInt uiqId, UInt uiredupic )                        const { return m_intra_samples_match_flag[uidId][uiqId][uiredupic];               }

private:
	UInt m_num_dId_minus1;
	UInt m_dependency_id[MAX_LAYERS];
  UInt m_num_qId_minus1[MAX_LAYERS];
	UInt m_quality_id[MAX_LAYERS][MAX_QUALITY_LEVELS];
	UInt m_num_redundant_pics_minus1[MAX_LAYERS][MAX_QUALITY_LEVELS];
	UInt m_redundant_pic_cnt_minus1[MAX_LAYERS][MAX_QUALITY_LEVELS][MAX_REDUNDANT_PICTURES_NUM];
	Bool m_pic_match_flag[MAX_LAYERS][MAX_QUALITY_LEVELS][MAX_REDUNDANT_PICTURES_NUM];
	Bool m_mb_type_match_flag[MAX_LAYERS][MAX_QUALITY_LEVELS][MAX_REDUNDANT_PICTURES_NUM];
	Bool m_motion_match_flag[MAX_LAYERS][MAX_QUALITY_LEVELS][MAX_REDUNDANT_PICTURES_NUM];
	Bool m_residual_match_flag[MAX_LAYERS][MAX_QUALITY_LEVELS][MAX_REDUNDANT_PICTURES_NUM];
	Bool m_intra_samples_match_flag[MAX_LAYERS][MAX_QUALITY_LEVELS][MAX_REDUNDANT_PICTURES_NUM];
};
//JVT-W049 }

	//JVT-W052 wxwan
	class H264AVCCOMMONLIB_API IntegrityCheckSEI : public SEIMessage
	{
	protected:
		IntegrityCheckSEI() : SEIMessage(INTEGRITY_CHECK_SEI)
		{}
	public:
		static ErrVal create( IntegrityCheckSEI*& rpcIntegrityCheckSEI );
		ErrVal    destroy ();
		ErrVal        write ( HeaderSymbolWriteIf*  pcWriteIf);
		ErrVal        read  ( HeaderSymbolReadIf*   pcReadIf );
		UInt					getNumInfoEntriesMinus1()							{ return m_uinuminfoentriesminus1; }
		UInt          getEntryDependencyId(UInt uilayer)		{ return m_uientrydependency_id[uilayer]; }
		UInt          getQualityLayerCRC  (UInt uilayer)		{ return m_uiquality_layer_crc [uilayer]; }
		Void          setNumInfoEntriesMinus1(UInt ui )     { m_uinuminfoentriesminus1 = ui; } 
		Void					setEntryDependencyId(UInt uilayer, UInt ui)    { m_uientrydependency_id[uilayer] = ui; }
		Void					setQualityLayerCRC  (UInt uilayer, UInt ui)		 { m_uiquality_layer_crc [uilayer] = ui; }
	private:
		UInt					m_uinuminfoentriesminus1;
		UInt					m_uientrydependency_id[ MAX_LAYERS ];
		UInt					m_uiquality_layer_crc [ MAX_LAYERS ];
	};
	//JVT-W052 wxwan
  //JVT-X032 {
  class H264AVCCOMMONLIB_API TLSwitchingPointSei: public SEIMessage
{
protected:
  TLSwitchingPointSei ();
  ~TLSwitchingPointSei ();

public:
  static ErrVal create  ( TLSwitchingPointSei*&      rpcSeiMessage );
  ErrVal        write   ( HeaderSymbolWriteIf*   pcWriteIf     );
  ErrVal        read    ( HeaderSymbolReadIf*    pcReadIf      );

  Void setDeltaFrameNum( UInt ui )                                                                       { m_delta_frame_num = ui;                                                    }
    
	Int getDeltaFrameNum( )                                                                         const { return m_delta_frame_num;                                                  }

private:
	Int m_delta_frame_num;
};
//JVT-X032 }
  typedef MyList<SEIMessage*> MessageList;

  static ErrVal read  ( HeaderSymbolReadIf*   pcReadIf,
                        MessageList&          rcSEIMessageList
                        // JVT-V068 {
                        ,ParameterSetMng* pcParameterSetMng
                        // JVT-V068 }
                      );
  static ErrVal write ( HeaderSymbolWriteIf*  pcWriteIf,
                        HeaderSymbolWriteIf*  pcWriteTestIf,
                        MessageList*          rpcSEIMessageList );
  //JVT-T073 {
  static ErrVal writeNesting        ( HeaderSymbolWriteIf*  pcWriteIf,
                                      HeaderSymbolWriteIf*  pcWriteTestIf,
                                      MessageList*          rpcSEIMessageList );
  static ErrVal xWriteNesting       ( HeaderSymbolWriteIf*  pcWriteIf,
                                      HeaderSymbolWriteIf*  pcWriteTestIf,
                                      SEIMessage*           pcSEIMessage,
                    UInt&                 uiBits );
 //JVT-T073 }

  // JVT-V068 {
  static ErrVal writeScalableNestingSei( HeaderSymbolWriteIf*  pcWriteIf,
                                         HeaderSymbolWriteIf*  pcWriteTestIf,
                                         MessageList*          rpcSEIMessageList );
  // JVT-V068 }

protected:
  static ErrVal xRead               ( HeaderSymbolReadIf*   pcReadIf,
                                      SEIMessage*&          rpcSEIMessage 
                                      // JVT-V068 {
                                      ,ParameterSetMng* pcParameterSetMng
                                      // JVT-V068 }
                                    );
  static ErrVal xWrite              ( HeaderSymbolWriteIf*  pcWriteIf,
                                      HeaderSymbolWriteIf*  pcWriteTestIf,
                                      SEIMessage*           pcSEIMessage );
  static ErrVal xWritePayloadHeader ( HeaderSymbolWriteIf*  pcWriteIf,
                                      MessageType           eMessageType,
                                      UInt                  uiSize );
  static ErrVal xReadPayloadHeader  ( HeaderSymbolReadIf*   pcReadIf,
                                      MessageType&          reMessageType,
                                      UInt&                 ruiSize);
  static ErrVal xCreate             ( SEIMessage*&          rpcSEIMessage,
                                      MessageType           eMessageType,
                                      // JVT-V068 {
                                      ParameterSetMng*& rpcParameterSetMng,
                                      // JVT-V068 }
                                      UInt                  uiSize );
public:


};

#if defined( WIN32 )
# pragma warning( default: 4251 )
#endif

H264AVC_NAMESPACE_END


#endif // !defined(AFX_SEI_H__06FFFAD0_FB36_4BF0_9392_395C7389C1F4__INCLUDED_)
