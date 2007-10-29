/**************************************************************************
// JVT-V068 HRD
**************************************************************************/
// JVT-V068 HRD {

#include "H264AVCCommonLib.h"
#include "H264AVCCommonLib/Vui.h"
#include "H264AVCCommonLib/SequenceParameterSet.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
// h264 namespace begin
H264AVC_NAMESPACE_BEGIN


VUI::BitstreamRestriction::BitstreamRestriction( SequenceParameterSet* pcSPS ):
  m_bBitstreamRestrictionFlag           ( false ),
  m_bMotionVectorsOverPicBoundariesFlag ( true ),
  m_uiMaxBytesPerPicDenom               ( 0 ),
  m_uiMaxBitsPerMbDenom                 ( 1 ),
  m_uiLog2MaxMvLengthHorizontal         ( 16 ),
  m_uiLog2MaxMvLengthVertical           ( 16 ),
  m_uiMaxDecFrameReordering             ( pcSPS->getMaxDPBSize() ),
  m_uiMaxDecFrameBuffering              ( pcSPS->getMaxDPBSize() )
{
}


ErrVal VUI::BitstreamRestriction::write( HeaderSymbolWriteIf* pcWriteIf ) const
{
  RNOK( pcWriteIf->writeFlag( m_bBitstreamRestrictionFlag,              "VUI: bitstream_restriction_flag"));
  ROFRS( m_bBitstreamRestrictionFlag, Err::m_nOK );

  RNOK( pcWriteIf->writeFlag( getMotionVectorsOverPicBoundariesFlag(),  "VUI: motion_vectors_over_pic_boundaries_flag"));
  RNOK( pcWriteIf->writeUvlc( getMaxBytesPerPicDenom(),                 "VUI: max_bytes_per_pic_denom"));
  RNOK( pcWriteIf->writeUvlc( getMaxBitsPerMbDenom(),                   "VUI: max_bits_per_mb_denom"));
  RNOK( pcWriteIf->writeUvlc( getLog2MaxMvLengthHorizontal(),           "VUI: log2_max_mv_length_horizontal"));
  RNOK( pcWriteIf->writeUvlc( getLog2MaxMvLengthVertical(),             "VUI: log2_max_mv_length_vertical"));
  RNOK( pcWriteIf->writeUvlc( getMaxDecFrameReordering(),               "VUI: max_dec_frame_reordering"));
  RNOK( pcWriteIf->writeUvlc( getMaxDecFrameBuffering(),                "VUI: max_dec_frame_buffering"));
  return Err::m_nOK;
}


ErrVal VUI::BitstreamRestriction::read( HeaderSymbolReadIf *pcReadIf )
{
  RNOKS( pcReadIf->getFlag( m_bBitstreamRestrictionFlag,           "VUI: bitstream_restriction_flag"));
  ROFRS( m_bBitstreamRestrictionFlag, Err::m_nOK );

  RNOKS( pcReadIf->getFlag( m_bMotionVectorsOverPicBoundariesFlag, "VUI: motion_vectors_over_pic_boundaries_flag"));

  RNOKS( pcReadIf->getUvlc( m_uiMaxBytesPerPicDenom,               "VUI: max_bytes_per_pic_denom"));
  ROTRS( m_uiMaxBytesPerPicDenom > 16, Err::m_nInvalidParameter );

  RNOKS( pcReadIf->getUvlc( m_uiMaxBitsPerMbDenom,                 "VUI: max_bits_per_mb_denom"));
  ROTRS( m_uiMaxBitsPerMbDenom > 16, Err::m_nInvalidParameter );

  RNOKS( pcReadIf->getUvlc( m_uiLog2MaxMvLengthHorizontal,         "VUI: log2_max_mv_length_horizontal"));
  ROTRS( m_uiLog2MaxMvLengthHorizontal > 16, Err::m_nInvalidParameter );

  RNOKS( pcReadIf->getUvlc( m_uiLog2MaxMvLengthVertical,           "VUI: log2_max_mv_length_vertical"));
  ROTRS( m_uiLog2MaxMvLengthVertical > 16, Err::m_nInvalidParameter );

  RNOKS( pcReadIf->getUvlc( m_uiMaxDecFrameReordering,             "VUI: max_dec_frame_reordering"));

  UInt uiTmp;
  RNOKS( pcReadIf->getUvlc( uiTmp,                                 "VUI: max_dec_frame_buffering"));
  ROTRS(uiTmp>16, Err::m_nInvalidParameter);
  ROTRS(getMaxDecFrameReordering() > uiTmp, Err::m_nInvalidParameter);
  setMaxDecFrameBuffering(uiTmp);

  return Err::m_nOK;
}


VUI::AspectRatioInfo::AspectRatioInfo():
  m_bAspectRatioInfoPresentFlag   ( false ),
  m_uiAspectRatioIdc              ( 0 ),
  m_uiSarWith                     ( 0 ),
  m_uiSarHeight                   ( 0 )
{
}

VUI::VideoSignalType::VideoSignalType():
  m_bVideoSignalTypePresentFlag   ( false ),
  m_uiVideoFormat                 ( 5 ),
  m_bVideoFullRangeFlag           ( false ),
  m_bColourDescriptionPresentFlag ( false ),
  m_uiColourPrimaries             ( 2 ),
  m_uiTransferCharacteristics     ( 2 ),
  m_uiMatrixCoefficients          ( 2 )
{
}

VUI::ChromaLocationInfo::ChromaLocationInfo():
  m_bChromaLocationInfoPresentFlag( false ),
  m_uiChromaLocationFrame         ( 0 ),
  m_uiChromaLocationField         ( 0 )
{
}

VUI::TimingInfo::TimingInfo():
  m_uiNumUnitsInTick              ( 0 ),
  m_uiTimeScale                   ( 0 ),
  m_bFixedFrameRateFlag           ( 0 )
{
}

ErrVal VUI::AspectRatioInfo::read( HeaderSymbolReadIf *pcReadIf )
{
  RNOKS( pcReadIf->getFlag( m_bAspectRatioInfoPresentFlag,       "VUI: aspect_ratio_info_present_flag"));
  ROFRS( m_bAspectRatioInfoPresentFlag, Err::m_nOK );

  RNOKS( pcReadIf->getCode( m_uiAspectRatioIdc, 8,               "VUI: aspect_ratio_idc"));

  if( m_uiAspectRatioIdc == 0xFF ) //Extendet_SAR
  {
    RNOKS( pcReadIf->getCode( m_uiSarWith, 16,                   "VUI: sar_width"));
    ROTRS(0 == m_uiSarWith, Err::m_nInvalidParameter);

    RNOKS( pcReadIf->getCode( m_uiSarHeight, 16,                 "VUI: sar_height"));
    ROTRS(0 == m_uiSarHeight, Err::m_nInvalidParameter);
  }
  return Err::m_nOK;
}

ErrVal VUI::VideoSignalType::read( HeaderSymbolReadIf *pcReadIf )
{
  RNOKS( pcReadIf->getFlag( m_bVideoSignalTypePresentFlag,       "VUI: video_signal_type_present_flag"));
  ROFRS( m_bVideoSignalTypePresentFlag, Err::m_nOK );

  RNOKS( pcReadIf->getCode( m_uiVideoFormat, 3,                  "VUI: video_format"));
  RNOKS( pcReadIf->getFlag( m_bVideoFullRangeFlag,               "VUI: video_full_range_flag"));
  RNOKS( pcReadIf->getFlag( m_bColourDescriptionPresentFlag,     "VUI: colour_description_present_flag"));

  if( getColourDescriptionPresentFlag() )
  {
    RNOKS( pcReadIf->getCode( m_uiColourPrimaries, 8,            "VUI: colour_primaries"));
    RNOKS( pcReadIf->getCode( m_uiTransferCharacteristics, 8,    "VUI: transfer_characteristics"));
    RNOKS( pcReadIf->getCode( m_uiMatrixCoefficients, 8,         "VUI: matrix_coefficients"));
  }
  return Err::m_nOK;
}

ErrVal VUI::ChromaLocationInfo::read( HeaderSymbolReadIf *pcReadIf )
{
  RNOKS( pcReadIf->getFlag( m_bChromaLocationInfoPresentFlag,    "VUI: chroma_location_info_present_flag"));
  ROFRS( m_bChromaLocationInfoPresentFlag, Err::m_nOK );

  RNOKS( pcReadIf->getUvlc( m_uiChromaLocationFrame,             "VUI: chroma_location_frame"));
  ROTRS( m_uiChromaLocationFrame>3, Err::m_nInvalidParameter);
  RNOKS( pcReadIf->getUvlc( m_uiChromaLocationField,             "VUI: chroma_location_field"));
  ROTRS( m_uiChromaLocationField>3, Err::m_nInvalidParameter);
  return Err::m_nOK;
}

ErrVal VUI::TimingInfo::read( HeaderSymbolReadIf *pcReadIf )
{
  RNOKS( pcReadIf->getFlag( m_bTimingInfoPresentFlag,            "VUI: timing_info_present_flag"));
  ROFRS( m_bTimingInfoPresentFlag, Err::m_nOK );

  RNOKS( pcReadIf->getCode( m_uiNumUnitsInTick, 32,              "VUI: num_units_in_tick"));
  RNOKS( pcReadIf->getCode( m_uiTimeScale, 32,                   "VUI: time_scale"));
  RNOKS( pcReadIf->getFlag( m_bFixedFrameRateFlag,               "VUI: fixed_frame_rate_flag"));
  return Err::m_nOK;
}

ErrVal VUI::AspectRatioInfo::write( HeaderSymbolWriteIf* pcWriteIf ) const
{
  RNOK( pcWriteIf->writeFlag( m_bAspectRatioInfoPresentFlag,    "VUI: aspect_ratio_info_present_flag"));
  ROFRS( m_bAspectRatioInfoPresentFlag, Err::m_nOK );

  RNOK( pcWriteIf->writeCode( m_uiAspectRatioIdc, 8,            "VUI: aspect_ratio_idc"));

  if( 0xFF == m_uiAspectRatioIdc ) //Extendet_SAR
  {
    RNOK( pcWriteIf->writeCode( m_uiSarWith, 16,                "VUI: sar_width"));
    RNOK( pcWriteIf->writeCode( m_uiSarHeight, 16,              "VUI: sar_height"));
  }
  return Err::m_nOK;
}

ErrVal VUI::VideoSignalType::write( HeaderSymbolWriteIf* pcWriteIf ) const
{
  RNOK( pcWriteIf->writeFlag( m_bVideoSignalTypePresentFlag,    "VUI: video_signal_type_present_flag"));
  ROFRS( m_bVideoSignalTypePresentFlag, Err::m_nOK );

  RNOK( pcWriteIf->writeCode( m_uiVideoFormat, 3,               "VUI: video_format"));
  RNOK( pcWriteIf->writeFlag( m_bVideoFullRangeFlag,            "VUI: video_full_range_flag"));
  RNOK( pcWriteIf->writeFlag( m_bColourDescriptionPresentFlag,  "VUI: colour_description_present_flag"));
  if( m_bColourDescriptionPresentFlag )
  {
    RNOK( pcWriteIf->writeCode( m_uiColourPrimaries, 8,         "VUI: colour_primaries"));
    RNOK( pcWriteIf->writeCode( m_uiTransferCharacteristics, 8, "VUI: transfer_characteristics"));
    RNOK( pcWriteIf->writeCode( m_uiMatrixCoefficients, 8,      "VUI: matrix_coefficients"));
  }
  return Err::m_nOK;
}

ErrVal VUI::ChromaLocationInfo::write( HeaderSymbolWriteIf* pcWriteIf ) const
{
  RNOK( pcWriteIf->writeFlag( m_bChromaLocationInfoPresentFlag, "VUI: chroma_location_info_present_flag"));
  ROFRS( m_bChromaLocationInfoPresentFlag, Err::m_nOK );

  RNOK( pcWriteIf->writeUvlc( m_uiChromaLocationFrame,          "VUI: chroma_location_frame"));
  RNOK( pcWriteIf->writeUvlc( m_uiChromaLocationField,          "VUI: chroma_location_field"));
  return Err::m_nOK;
}

ErrVal VUI::TimingInfo::write( HeaderSymbolWriteIf* pcWriteIf ) const
{
  RNOK( pcWriteIf->writeFlag( m_bTimingInfoPresentFlag,         "VUI: timing_info_present_flag"));
  ROFRS( m_bTimingInfoPresentFlag, Err::m_nOK );

  RNOK( pcWriteIf->writeCode( m_uiNumUnitsInTick, 32,           "VUI: num_units_in_tick"));
  RNOK( pcWriteIf->writeCode( m_uiTimeScale, 32,                "VUI: time_scale"));
  RNOK( pcWriteIf->writeFlag( m_bFixedFrameRateFlag,            "VUI: fixed_frame_rate_flag"));
  return Err::m_nOK;
}


VUI::VUI( SequenceParameterSet* pcSPS):
  m_bVuiParametersPresentFlag     ( false ),
  m_bOverscanInfoPresentFlag      ( false ),
  m_bOverscanAppropriateFlag      ( false ),
  m_cBitstreamRestriction         ( pcSPS )
{
  m_eProfileIdc = pcSPS->getProfileIdc();
}

VUI::~VUI()
{
}

ErrVal VUI::InitHrd( UInt uiIndex, HRD::HrdParamType eHrdType, UInt uiBitRate, UInt uiCpbSize)
{
  HRD& rcHrd = (eHrdType == HRD::NAL_HRD) ? m_acNalHrd[uiIndex] : m_acVclHrd[uiIndex];
   
  if( rcHrd.getHrdParametersPresentFlag()) 
  {
    // we provide only one set of parameters. the following code is pretty hard coded
    rcHrd.setCpbCnt(1);
    UInt uiExponentBR  = max(6, gGetNumberOfLSBZeros(uiBitRate));
    UInt uiExponentCpb = max(4, gGetNumberOfLSBZeros(uiCpbSize));
    rcHrd.setBitRateScale(uiExponentBR-6);
    rcHrd.setCpbSizeScale(uiExponentCpb-4);
    rcHrd.init(1);
    for( UInt CpbCnt=0; CpbCnt<1; CpbCnt++) 
    {
      Int iBitNumLSBZeros = gGetNumberOfLSBZeros(uiBitRate);
      Int iCpbNumLSBZeros = gGetNumberOfLSBZeros(uiCpbSize);
      // we're doing a bit complicated rounding here to find the nearest value
      // probably we should use the exact or bigger bit rate value:
      //for values with 6 or more LSBZeros:  
      if( iBitNumLSBZeros >= 5)
      {
        rcHrd.getCntBuf(CpbCnt).setBitRateValue((UInt)floor ( ((Double)(uiBitRate) /  (1 << uiExponentBR))));
      }
      else//for values with less than 6 LSBZeros
      {
        rcHrd.getCntBuf(CpbCnt).setBitRateValue((UInt)floor ( ((Double)(uiBitRate) /  (1 << uiExponentBR)) + 0.5));
      }

      //for values with 4 or more LSBZeros:  
      if( iCpbNumLSBZeros >= 3)
      {
        rcHrd.getCntBuf(CpbCnt).setCpbSizeValue((UInt)floor ( ((Double)(uiCpbSize) /  (1<< uiExponentCpb)) ));
      }
      else//for values with less than 4 LSBZeros
      {
        rcHrd.getCntBuf(CpbCnt).setCpbSizeValue((UInt)floor ( ((Double)(uiCpbSize) /  (1<< uiExponentCpb)) + 0.5));
      }
      //0 VBR 1 CBR      
      rcHrd.getCntBuf(CpbCnt).setVbrCbrFlag(0);// 1: stuffing needed
    }
    rcHrd.setInitialCpbRemovalDelayLength(24);
    rcHrd.setCpbRemovalDelayLength(24);
    rcHrd.setDpbOutputDelayLength(24);
    rcHrd.setTimeOffsetLength(0);
  }
  return Err::m_nOK;
}


ErrVal VUI::init( UInt uiNumTemporalLevels, UInt uiNumFGSLevels )
{
  m_uiNumTemporalLevels = uiNumTemporalLevels;
  m_uiNumFGSLevels = uiNumFGSLevels;
  UInt uiNumLayers = uiNumTemporalLevels * uiNumFGSLevels;
  RNOK( m_acLayerInfo.init( uiNumLayers ) );
  RNOK( m_acTimingInfo.init( uiNumLayers ) );
  RNOK( m_acNalHrd.init( uiNumLayers ) );
  RNOK( m_acVclHrd.init( uiNumLayers ) );
  RNOK( m_abLowDelayHrdFlag.init( uiNumLayers ) );
  RNOK( m_abPicStructPresentFlag.init( uiNumLayers ) );

  return Err::m_nOK;
}

ErrVal VUI::LayerInfo::write( HeaderSymbolWriteIf* pcWriteIf ) const
{
  RNOK( pcWriteIf->writeCode( m_uiTemporalLevel, 3,                       "HRD::temporal_level[i]"));
  RNOK( pcWriteIf->writeCode( m_uiDependencyID, 3,                        "HRD::dependency_id[i]"));
  RNOK( pcWriteIf->writeCode( m_uiQualityLevel, 4,                        "HRD::quality_level[i]"));
  return Err::m_nOK;
}

ErrVal VUI::write( HeaderSymbolWriteIf* pcWriteIf ) const
{
  RNOK( pcWriteIf->writeFlag( m_bVuiParametersPresentFlag,            "SPS: vui_parameters_present_flag" ) );

  if (!m_bVuiParametersPresentFlag) return Err::m_nOK;

  RNOK( m_cAspectRatioInfo.write( pcWriteIf ) );

  RNOK( pcWriteIf->writeFlag( getOverscanInfoPresentFlag(),     "VUI: overscan_info_present_flag"));
  if( getOverscanInfoPresentFlag() )
  {
    RNOK( pcWriteIf->writeFlag( getOverscanAppropriateFlag(),   "VUI: overscan_appropriate_flag"));
  }

  RNOK( m_cVideoSignalType.write( pcWriteIf ) );
  RNOK( m_cChromaLocationInfo.write( pcWriteIf ) );
	if(m_eProfileIdc==SCALABLE_PROFILE)
	{
    UInt uiNumLayers = m_uiNumTemporalLevels * m_uiNumFGSLevels;
		RNOK( pcWriteIf->writeUvlc( uiNumLayers - 1,  "VUI: num_temporal_layers_minus1"));
		for(UInt i=0; i<uiNumLayers; i++)
		{
      RNOK( m_acLayerInfo.get(i).write( pcWriteIf) );
			RNOK( m_acTimingInfo.get(i).write( pcWriteIf) );
      RNOK( m_acNalHrd.get(i).write( pcWriteIf ) );
      RNOK( m_acVclHrd.get(i).write( pcWriteIf ) );
      if( m_acNalHrd.get(i).getHrdParametersPresentFlag() || m_acVclHrd.get(i).getHrdParametersPresentFlag() )
      {
        RNOK( pcWriteIf->writeFlag( m_abLowDelayHrdFlag.get(i),           "VUI: low_delay_hrd_flag[i]"));
      }
      RNOK( pcWriteIf->writeFlag( m_abPicStructPresentFlag.get(i),        "VUI: pic_struct_present_flag[i]"));
		}
	}
	else
	{
		RNOK( m_acTimingInfo.get(0).write( pcWriteIf ) ); //DL
    RNOK( m_acNalHrd.get(0).write( pcWriteIf ) );
    RNOK( m_acVclHrd.get(0).write( pcWriteIf ) );
    if( m_acNalHrd.get(0).getHrdParametersPresentFlag() || m_acVclHrd.get(0).getHrdParametersPresentFlag() )
    {
      RNOK( pcWriteIf->writeFlag( m_abLowDelayHrdFlag.get(0),           "VUI: low_delay_hrd_flag"));
    }
    RNOK( pcWriteIf->writeFlag( m_abPicStructPresentFlag.get(0),        "VUI: pic_struct_present_flag"));
	}


  RNOK( m_cBitstreamRestriction.write( pcWriteIf ) );
 
  return Err::m_nOK;
}

ErrVal VUI::LayerInfo::read( HeaderSymbolReadIf* pcReadIf ) 
{
  RNOKS( pcReadIf->getCode( m_uiTemporalLevel, 3,               "VUI: temporal_level"));
  RNOKS( pcReadIf->getCode( m_uiDependencyID, 3,               "VUI: dependency_id"));
  RNOKS( pcReadIf->getCode( m_uiQualityLevel, 4,               "VUI: quality_level"));
  return Err::m_nOK;
}

ErrVal VUI::read( HeaderSymbolReadIf *pcReadIf )
{
  UInt uiNumLayers;
  RNOKS( m_cAspectRatioInfo.read( pcReadIf ) );

  RNOKS( pcReadIf->getFlag( m_bOverscanInfoPresentFlag,          "VUI: overscan_info_present_flag"));
  if( m_bOverscanInfoPresentFlag )
  {
    RNOKS( pcReadIf->getFlag( m_bOverscanAppropriateFlag,        "VUI: overscan_appropriate_flag"));
  }

  RNOKS( m_cVideoSignalType.read( pcReadIf ) );
  RNOKS( m_cChromaLocationInfo.read( pcReadIf ) );

	if(m_eProfileIdc==SCALABLE_PROFILE)
	{
		RNOKS( pcReadIf->getUvlc( uiNumLayers,                              "VUI: num_temporal_layers_minus1"));
    uiNumLayers++;
	}
  else
  {
    uiNumLayers = 1;
  }

  m_acLayerInfo.uninit();
  m_acLayerInfo.init( uiNumLayers );

  m_acTimingInfo.uninit();
	m_acTimingInfo.init( uiNumLayers );

  m_acNalHrd.uninit();
  m_acNalHrd.init( uiNumLayers );

  m_acVclHrd.uninit();
  m_acVclHrd.init( uiNumLayers );

  m_abLowDelayHrdFlag.uninit();
  m_abLowDelayHrdFlag.init( uiNumLayers );

  m_abPicStructPresentFlag.uninit();
  m_abPicStructPresentFlag.init( uiNumLayers );

	for(UInt i=0; i<uiNumLayers; i++)
	{
    if ( m_eProfileIdc == SCALABLE_PROFILE )
    {
      m_acLayerInfo.get(i).read(pcReadIf);
    }
    else
    {
      // fill in the LayerInfo of AVC compatible layer
      m_acLayerInfo[0].setDependencyID(0);
      m_acLayerInfo[0].setQualityLevel(0);
      m_acLayerInfo[0].setTemporalLevel(0);
    }

		RNOKS( m_acTimingInfo.get(i).read( pcReadIf ) ); //DL
    RNOKS( m_acNalHrd.get(i).read( pcReadIf ) );
    RNOKS( m_acVclHrd.get(i).read( pcReadIf ) );
    if( m_acNalHrd.get(i).getHrdParametersPresentFlag() || m_acVclHrd.get(i).getHrdParametersPresentFlag() )
    {
      RNOKS( pcReadIf->getFlag( m_abLowDelayHrdFlag[i],                "VUI: low_delay_hrd_flag"));
    }
    RNOKS( pcReadIf->getFlag( m_abPicStructPresentFlag[i],             "VUI: pic_struct_present_flag"));
	}

  RNOKS( m_cBitstreamRestriction.read( pcReadIf ) );
  return Err::m_nOK;
}

//Void VUI::setNalHrdFlag(Bool nal_hrd_parameters_present_flag)
//{
//	m_cNalHrd.setHrdParametersPresentFlag(nal_hrd_parameters_present_flag);
//}
//
//Void VUI::setVclHrdFlag(Bool vcl_hrd_parameters_present_flag)
//{
//	m_cVclHrd.setHrdParametersPresentFlag(vcl_hrd_parameters_present_flag);
//}


// JVT-V068 HRD } 

// h264 namespace end
H264AVC_NAMESPACE_END

