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
#include <list>



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
    SUB_SEQ_INFO                          = 10,
    SCALABLE_SEI                          = 22,
		SUB_PIC_SEI														= 23,
    //{{Quality level estimation and modified truncation- JVTO044 and m12007
    //France Telecom R&D-(nathalie.cammas@francetelecom.com)
    QUALITYLEVEL_SEI                      = 25,
	//}}Quality level estimation and modified truncation- JVTO044 and m12007
	  RESERVED_SEI                          = 26,
  	NON_REQUIRED_SEI					            = 24
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
		static ErrVal create ( ScalableSei*&			rpcSeiMessage);
		ErrVal write				 ( HeaderSymbolWriteIf	*pcWriteIf);
		ErrVal read					 ( HeaderSymbolReadIf		*pcReadIf);

		Void setNumLayersMinus1( UInt ui )																				{ m_num_layers_minus1 = ui;	}
		Void setLayerId ( UInt uilayer, UInt uiId )																{ m_layer_id															[uilayer] = uiId; }
		Void setFGSlayerFlag ( UInt uilayer, Bool bFlag )													{ m_fgs_layer_flag												[uilayer] = bFlag; }
		Void setSubPicLayerFlag ( UInt uilayer, Bool bFlag)												{ m_sub_pic_layer_flag[uilayer] = bFlag; }
		Void setSubRegionLayerFlag ( UInt uilayer, Bool bFlag)										{ m_sub_region_layer_flag									[uilayer] = bFlag; }
		Void setProfileLevelInfoPresentFlag ( UInt uilayer, Bool bFlag)						{ m_profile_level_info_present_flag				[uilayer] = bFlag; }
		Void setDecodingDependencyInfoPresentFlag ( UInt uilayer, Bool bFlag )		{ m_decoding_dependency_info_present_flag	[uilayer] = bFlag; }
		Void setBitrateInfoPresentFlag ( UInt uilayer, Bool bFlag )								{ m_bitrate_info_present_flag							[uilayer] = bFlag; }
		Void setFrmRateInfoPresentFlag ( UInt uilayer, Bool bFlag )								{ m_frm_rate_info_present_flag						[uilayer] = bFlag; }
		Void setFrmSizeInfoPresentFlag ( UInt uilayer, Bool bFlag )								{ m_frm_size_info_present_flag						[uilayer] = bFlag; }
		Void setLayerDependencyInfoPresentFlag ( UInt uilayer, Bool bFlag )				{ m_layer_dependency_info_present_flag		[uilayer] = bFlag; }
		Void setInitParameterSetsInfoPresentFlag ( UInt uilayer, Bool bFlag )			{ m_init_parameter_sets_info_present_flag	[uilayer] = bFlag; }
		Void setLayerProfileIdc ( UInt uilayer, UInt uiIdc )											{ m_layer_profile_idc											[uilayer] = uiIdc; }
		Void setLayerConstraintSet0Flag ( UInt uilayer, Bool bFlag )							{ m_layer_constraint_set0_flag						[uilayer] = bFlag; }
		Void setLayerConstraintSet1Flag ( UInt uilayer, Bool bFlag )							{ m_layer_constraint_set1_flag						[uilayer] = bFlag; }
		Void setLayerConstraintSet2Flag ( UInt uilayer, Bool bFlag )							{ m_layer_constraint_set2_flag						[uilayer] = bFlag; }
		Void setLayerConstraintSet3Flag ( UInt uilayer, Bool bFlag )							{ m_layer_constraint_set3_flag						[uilayer] = bFlag; }
		Void setLayerLevelIdc ( UInt uilayer, UInt uiIdc )												{ m_layer_level_idc												[uilayer] = uiIdc; }
		Void setTemporalLevel ( UInt uilayer, UInt uiLevel )											{ m_temporal_level												[uilayer] = uiLevel; }
		Void setDependencyId ( UInt uilayer, UInt uiId )													{ m_dependency_id													[uilayer] = uiId; }
		Void setQualityLevel ( UInt uilayer, UInt uiLevel )												{ m_quality_level													[uilayer] = uiLevel; }
		Void setAvgBitrate ( UInt uilayer, UInt uiBitrate )												{ m_avg_bitrate														[uilayer] = uiBitrate; }
		Void setMaxBitrate ( UInt uilayer, UInt uiBitrate )												{ m_max_bitrate														[uilayer] = uiBitrate; }
		Void setConstantFrmRateIdc ( UInt uilayer, UInt uiFrmrate )								{ m_constant_frm_rate_idc									[uilayer] = uiFrmrate; }
		Void setAvgFrmRate ( UInt uilayer, UInt uiFrmrate )												{ m_avg_frm_rate													[uilayer] = uiFrmrate; }
		Void setFrmWidthInMbsMinus1 ( UInt uilayer, UInt uiWidth )								{ m_frm_width_in_mbs_minus1								[uilayer] = uiWidth; }
		Void setFrmHeightInMbsMinus1 ( UInt uilayer, UInt uiHeight )							{ m_frm_height_in_mbs_minus1							[uilayer] = uiHeight; }
		Void setBaseRegionLayerId ( UInt uilayer, UInt uiId )											{ m_base_region_layer_id									[uilayer] = uiId; }
		Void setDynamicRectFlag ( UInt uilayer, Bool bFlag )											{ m_dynamic_rect_flag											[uilayer] = bFlag; }
		Void setHorizontalOffset ( UInt uilayer, UInt uiOffset )									{ m_horizontal_offset											[uilayer] = uiOffset; }
		Void setVerticalOffset ( UInt uilayer, UInt uiOffset )										{ m_vertical_offset												[uilayer] = uiOffset; }
		Void setRegionWidth ( UInt uilayer, UInt uiWidth )												{ m_region_width													[uilayer] = uiWidth; }
		Void setRegionHeight ( UInt uilayer, UInt uiHeight )											{ m_region_height													[uilayer] = uiHeight; }
		Void setNumDirectlyDependentLayers ( UInt uilayer, UInt uiNum )						{ m_num_directly_dependent_layers					[uilayer] = uiNum; }
		Void setDirectlyDependentLayerIdDelta( UInt uilayer, UInt uiTar, UInt uiDelta ){ m_directly_dependent_layer_id_delta[uilayer][uiTar] = uiDelta;	}
		Void setNumInitSeqParameterSetMinus1 ( UInt uilayer, UInt uiNum )					{ m_num_init_seq_parameter_set_minus1			[uilayer] = uiNum; }
		Void setInitSeqParameterSetIdDelta ( UInt uilayer, UInt uiSPS, UInt uiTar){ m_init_seq_parameter_set_id_delta				[uilayer][uiSPS] = uiTar;	}
		Void setNumInitPicParameterSetMinus1 ( UInt uilayer, UInt uiNum )					{ m_num_init_pic_parameter_set_minus1			[uilayer] = uiNum; }
		Void setInitPicParameterSetIdDelta ( UInt uilayer, UInt uiPPS, UInt uiTar){ m_init_pic_parameter_set_id_delta				[uilayer][uiPPS] = uiTar; }
// BUG_FIX liuhui{
		Void setStdAVCOffset( UInt uiOffset )                                     { m_std_AVC_Offset = uiOffset;}
		UInt getStdAVCOffset()const { return m_std_AVC_Offset; }
// BUG_FIX liuhui}

		UInt getNumLayersMinus1() const {return m_num_layers_minus1;}
		UInt getLayerId ( UInt uilayer ) const { return m_layer_id[uilayer]; }
		Bool getFGSLayerFlag ( UInt uilayer ) const { return m_fgs_layer_flag[uilayer]; }
		Bool getSubPicLayerFlag ( UInt uilayer ) { return m_sub_pic_layer_flag[uilayer]; }
		Bool getSubRegionLayerFlag ( UInt uilayer ) const { return m_sub_region_layer_flag[uilayer]; }
		Bool getProfileLevelInfoPresentFlag ( UInt uilayer ) const { return m_profile_level_info_present_flag[uilayer]; }
		Bool getDecodingDependencyInfoPresentFlag ( UInt uilayer ) const { return m_decoding_dependency_info_present_flag[uilayer]; }
		Bool getBitrateInfoPresentFlag ( UInt uilayer ) const { return m_bitrate_info_present_flag[uilayer]; }
		Bool getFrmRateInfoPresentFlag ( UInt uilayer ) const { return m_frm_rate_info_present_flag[uilayer]; }
		Bool getFrmSizeInfoPresentFlag ( UInt uilayer ) const { return m_frm_size_info_present_flag[uilayer]; }
		Bool getLayerDependencyInfoPresentFlag ( UInt uilayer ) const { return m_layer_dependency_info_present_flag[uilayer]; }
		Bool getInitParameterSetsInfoPresentFlag ( UInt uilayer ) const { return m_init_parameter_sets_info_present_flag[uilayer]; }

		UInt getLayerProfileIdc ( UInt uilayer ) const { return m_layer_profile_idc[uilayer]; }
		Bool getLayerConstraintSet0Flag ( UInt uilayer ) const { return m_layer_constraint_set0_flag[uilayer]; }
		Bool getLayerConstraintSet1Flag ( UInt uilayer ) const { return m_layer_constraint_set1_flag[uilayer]; }
		Bool getLayerConstraintSet2Flag ( UInt uilayer ) const { return m_layer_constraint_set2_flag[uilayer]; }
		Bool getLayerConstraintSet3Flag ( UInt uilayer ) const { return m_layer_constraint_set3_flag[uilayer]; }
		UInt getLayerLevelIdc ( UInt uilayer ) const { return m_layer_level_idc[uilayer]; }
		UInt getTemporalLevel ( UInt uilayer ) const { return m_temporal_level[uilayer]; }
		UInt getDependencyId ( UInt uilayer ) const { return m_dependency_id[uilayer]; }
		UInt getQualityLevel ( UInt uilayer ) const { return m_quality_level[uilayer]; }
		UInt getAvgBitrate ( UInt uilayer ) const { return m_avg_bitrate[uilayer]; }
		UInt getMaxBitrate ( UInt uilayer ) const { return m_max_bitrate[uilayer]; }
		UInt getConstantFrmRateIdc ( UInt uilayer ) const { return m_constant_frm_rate_idc[uilayer]; }
		UInt getAvgFrmRate ( UInt uilayer ) const { return m_avg_frm_rate[uilayer]; }
		UInt getFrmWidthInMbsMinus1 ( UInt uilayer ) const { return m_frm_width_in_mbs_minus1[uilayer]; }
		UInt getFrmHeightInMbsMinus1 ( UInt uilayer ) const { return m_frm_height_in_mbs_minus1[uilayer]; }
		UInt getBaseRegionLayerId ( UInt uilayer ) const { return m_base_region_layer_id[uilayer]; }
		Bool getDynamicRectFlag ( UInt uilayer ) const { return m_dynamic_rect_flag[uilayer]; }
		UInt getHorizontalOffset ( UInt uilayer ) const { return m_horizontal_offset[uilayer]; }
		UInt getVerticalOffset ( UInt uilayer ) const { return m_vertical_offset[uilayer]; }
		UInt getRegionWidth ( UInt uilayer ) const { return m_region_width[uilayer]; }
		UInt getRegionHeight ( UInt uilayer ) const { return m_region_height[uilayer]; }
		UInt getNumDirectlyDependentLayers ( UInt uilayer ) const { return m_num_directly_dependent_layers[uilayer]; }
// BUG_FIX liuhui{
		UInt getNumDirectlyDependentLayerIdDelta( UInt uilayer, UInt uiIndex ) const { return m_directly_dependent_layer_id_delta[uilayer][uiIndex]; }
// BUG_FIX liuhui}
		//
		UInt getNumInitSPSMinus1 ( UInt uilayer ) const { return m_num_init_seq_parameter_set_minus1[uilayer]; }
		UInt getNumInitPPSMinus1 ( UInt uilayer ) const { return m_num_init_pic_parameter_set_minus1[uilayer]; }
// BUG_FIX liuhui{
		UInt getInitSPSIdDelta ( UInt uilayer, UInt uiIndex ) const { return m_init_seq_parameter_set_id_delta[uilayer][uiIndex]; }
		UInt getInitPPSIdDelta ( UInt uilayer, UInt uiIndex ) const { return m_init_pic_parameter_set_id_delta[uilayer][uiIndex]; }
// BUG_FIX liuhui}

	private:
// BUG_FIX liuhui{
		UInt m_std_AVC_Offset;
// BUG_FIX liuhui}
		UInt m_num_layers_minus1;
		UInt m_layer_id[MAX_SCALABLE_LAYERS];
		Bool m_fgs_layer_flag[MAX_SCALABLE_LAYERS];
		Bool m_sub_pic_layer_flag[MAX_SCALABLE_LAYERS];
		Bool m_sub_region_layer_flag[MAX_SCALABLE_LAYERS];
		Bool m_profile_level_info_present_flag[MAX_SCALABLE_LAYERS];
		Bool m_decoding_dependency_info_present_flag[MAX_SCALABLE_LAYERS];
		Bool m_bitrate_info_present_flag[MAX_SCALABLE_LAYERS];
		Bool m_frm_rate_info_present_flag[MAX_SCALABLE_LAYERS];
		Bool m_frm_size_info_present_flag[MAX_SCALABLE_LAYERS];
		Bool m_layer_dependency_info_present_flag[MAX_SCALABLE_LAYERS];
		Bool m_init_parameter_sets_info_present_flag[MAX_SCALABLE_LAYERS];

		UInt m_layer_profile_idc[MAX_SCALABLE_LAYERS];
		Bool m_layer_constraint_set0_flag[MAX_SCALABLE_LAYERS];
		Bool m_layer_constraint_set1_flag[MAX_SCALABLE_LAYERS];
		Bool m_layer_constraint_set2_flag[MAX_SCALABLE_LAYERS];
		Bool m_layer_constraint_set3_flag[MAX_SCALABLE_LAYERS];
		UInt m_layer_level_idc[MAX_SCALABLE_LAYERS];

		UInt m_temporal_level[MAX_SCALABLE_LAYERS];
		UInt m_dependency_id[MAX_SCALABLE_LAYERS];
		UInt m_quality_level[MAX_SCALABLE_LAYERS];

		UInt m_avg_bitrate[MAX_SCALABLE_LAYERS];
		UInt m_max_bitrate[MAX_SCALABLE_LAYERS];

		UInt m_constant_frm_rate_idc[MAX_SCALABLE_LAYERS];
		UInt m_avg_frm_rate[MAX_SCALABLE_LAYERS];

		UInt m_frm_width_in_mbs_minus1[MAX_SCALABLE_LAYERS];
		UInt m_frm_height_in_mbs_minus1[MAX_SCALABLE_LAYERS];

		UInt m_base_region_layer_id[MAX_SCALABLE_LAYERS];
		Bool m_dynamic_rect_flag[MAX_SCALABLE_LAYERS];
		UInt m_horizontal_offset[MAX_SCALABLE_LAYERS];
		UInt m_vertical_offset[MAX_SCALABLE_LAYERS];
		UInt m_region_width[MAX_SCALABLE_LAYERS];
		UInt m_region_height[MAX_SCALABLE_LAYERS];

		//UInt m_roi_id[MAX_SCALABLE_LAYERS];
// BUG_FIX liuhui{
		UInt m_num_directly_dependent_layers[MAX_SCALABLE_LAYERS];
		UInt m_directly_dependent_layer_id_delta[MAX_SCALABLE_LAYERS][MAX_SCALABLE_LAYERS];

		UInt m_num_init_seq_parameter_set_minus1[MAX_SCALABLE_LAYERS];
		UInt m_init_seq_parameter_set_id_delta[MAX_SCALABLE_LAYERS][32];
		UInt m_num_init_pic_parameter_set_minus1[MAX_SCALABLE_LAYERS];
		UInt m_init_pic_parameter_set_id_delta[MAX_SCALABLE_LAYERS][256];
// BUG_FIX liuhui}
	};

	class H264AVCCOMMONLIB_API SubPicSei : public SEIMessage
	{
	protected:
		SubPicSei ();
		~SubPicSei();

	public:
		static ErrVal create	( SubPicSei*&				rpcSeiMessage );
		ErrVal				write		( HeaderSymbolWriteIf*	pcWriteIf );
		ErrVal				read		( HeaderSymbolReadIf*		pcReadIf  );	

		UInt getLayerId	()					const	{ return m_uiLayerId;				}
		Void setLayerId ( UInt uiLayerId) { m_uiLayerId = uiLayerId;	}

	private:
		UInt m_uiLayerId;
	};
  
  //{{Quality level estimation and modified truncation- JVTO044 and m12007
  //France Telecom R&D-(nathalie.cammas@francetelecom.com)
  class H264AVCCOMMONLIB_API QualityLevelSEI : public SEIMessage
  {
	protected:
    QualityLevelSEI ();
    ~QualityLevelSEI();

  public:
    static ErrVal create  ( QualityLevelSEI*&         rpcSeiMessage );
    ErrVal        write   ( HeaderSymbolWriteIf*  pcWriteIf );
    ErrVal        read    ( HeaderSymbolReadIf*   pcReadIf );
	
	UInt		 getNumLevel() { return m_uiNumLevels;}
	Void		 setNumLevel(UInt ui) { m_uiNumLevels = ui;}
	UInt		 getDeltaBytesRateOfLevel(UInt ui) { return m_auiDeltaBytesRateOfLevel[ui];}
	Void		 setDeltaBytesRateOfLevel(UInt uiIndex, UInt ui) { m_auiDeltaBytesRateOfLevel[uiIndex] = ui;}
	UInt		 getQualityLevel(UInt ui) { return m_auiQualityLevel[ui];}
	Void		 setQualityLevel(UInt uiIndex, UInt ui) { m_auiQualityLevel[uiIndex] = ui;}
	UInt		 getDependencyId() { return m_uiDependencyId;}
	Void		 setDependencyId( UInt ui) { m_uiDependencyId = ui;}

  private:
	  UInt m_auiQualityLevel[MAX_NUM_RD_LEVELS];
	  UInt m_auiDeltaBytesRateOfLevel[MAX_NUM_RD_LEVELS];
	  UInt m_uiNumLevels;
	  UInt m_uiDependencyId;
  };
  //}}Quality level estimation and modified truncation- JVTO044 and m12007


  class H264AVCCOMMONLIB_API NonRequiredSei : public SEIMessage
  {
  protected:
	  NonRequiredSei ();
	  ~NonRequiredSei();

  public:
	  static ErrVal create	(NonRequiredSei*&			rpcSeiMessage);
	  ErrVal		destroy ();  
	  ErrVal		write	(HeaderSymbolWriteIf*		pcWriteIf);
	  ErrVal		read	(HeaderSymbolReadIf*		pcReadIf);

	  UInt			getNumInfoEntriesMinus1()					const{ return m_uiNumInfoEntriesMinus1;}
	  UInt			getEntryDependencyId(UInt uiLayer)			const{ return m_uiEntryDependencyId[uiLayer];}
	  UInt			getNumNonRequiredPicsMinus1(UInt uiLayer)	const{ return m_uiNumNonRequiredPicsMinus1[uiLayer];}
	  UInt			getNonRequiredPicDependencyId(UInt uiLayer, UInt uiNonRequiredLayer)	const{ return m_uiNonRequiredPicDependencyId[uiLayer][uiNonRequiredLayer];}
	  UInt			getNonRequiredPicQulityLevel(UInt uiLayer, UInt uiNonRequiredLayer)		const{ return m_uiNonRequiredPicQulityLevel[uiLayer][uiNonRequiredLayer];}
	  UInt			getNonRequiredPicFragmentOrder(UInt uiLayer, UInt uiNonRequiredLayer)	const{ return m_uiNonRequiredPicFragmentOrder[uiLayer][uiNonRequiredLayer];}


	  Void			setNumInfoEntriesMinus1(UInt ui)					{ m_uiNumInfoEntriesMinus1 = ui;}
	  Void			setEntryDependencyId(UInt uiLayer, UInt ui)			{ m_uiEntryDependencyId[uiLayer] = ui;}
	  Void			setNumNonRequiredPicsMinus1(UInt uiLayer, UInt ui)	{ m_uiNumNonRequiredPicsMinus1[uiLayer] = ui;}
	  Void			setNonNonRequiredPicDependencyId(UInt uiLayer, UInt uiNonRequiredLayer, UInt ui)		{m_uiNonRequiredPicDependencyId[uiLayer][uiNonRequiredLayer] = ui;}
	  Void			setNonNonRequiredPicQulityLevel(UInt uiLayer, UInt uiNonRequiredLayer, UInt ui)			{m_uiNonRequiredPicQulityLevel[uiLayer][uiNonRequiredLayer] = ui;}
	  Void			setNonNonRequiredPicFragmentOrder(UInt uiLayer, UInt uiNonRequiredLayer, UInt ui)		{m_uiNonRequiredPicFragmentOrder[uiLayer][uiNonRequiredLayer] = ui;}


  private:
	  UInt		m_uiNumInfoEntriesMinus1;
	  UInt		m_uiEntryDependencyId[MAX_NUM_INFO_ENTRIES];
	  UInt		m_uiNumNonRequiredPicsMinus1[MAX_NUM_INFO_ENTRIES];
	  UInt		m_uiNonRequiredPicDependencyId[MAX_NUM_INFO_ENTRIES][MAX_NUM_NON_REQUIRED_PICS];
	  UInt		m_uiNonRequiredPicQulityLevel[MAX_NUM_INFO_ENTRIES][MAX_NUM_NON_REQUIRED_PICS];
	  UInt		m_uiNonRequiredPicFragmentOrder[MAX_NUM_INFO_ENTRIES][MAX_NUM_NON_REQUIRED_PICS];
  };//shenqiu 05-09-15

  typedef MyList<SEIMessage*> MessageList;
  
  static ErrVal read  ( HeaderSymbolReadIf*   pcReadIf,
                        MessageList&          rcSEIMessageList );
  static ErrVal write ( HeaderSymbolWriteIf*  pcWriteIf,
                        HeaderSymbolWriteIf*  pcWriteTestIf,
                        MessageList*          rpcSEIMessageList );

protected:
  static ErrVal xRead               ( HeaderSymbolReadIf*   pcReadIf,
                                      SEIMessage*&          rpcSEIMessage ); 
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
                                      UInt                  uiSize ); 
public:


};

#if defined( WIN32 )
# pragma warning( default: 4251 )
#endif

H264AVC_NAMESPACE_END


#endif // !defined(AFX_SEI_H__06FFFAD0_FB36_4BF0_9392_395C7389C1F4__INCLUDED_)
