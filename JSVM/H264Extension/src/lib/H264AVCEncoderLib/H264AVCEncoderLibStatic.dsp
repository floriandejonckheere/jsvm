# Microsoft Developer Studio Project File - Name="H264AVCEncoderLibStatic" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=H264AVCEncoderLibStatic - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "H264AVCEncoderLibStatic.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "H264AVCEncoderLibStatic.mak" CFG="H264AVCEncoderLibStatic - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "H264AVCEncoderLibStatic - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "H264AVCEncoderLibStatic - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "H264AVCEncoderLibStatic - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "..\..\..\build\windows\lib\H264AVCEncoderLib\ReleaseStatic"
# PROP BASE Intermediate_Dir "..\..\..\build\windows\lib\H264AVCEncoderLib\ReleaseStatic"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\..\..\lib"
# PROP Intermediate_Dir "..\..\..\build\windows\lib\H264AVCEncoderLib\ReleaseStatic"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /I "..\..\..\include" /I "." /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "H264AVCVIDEOLIB_LIB" /D "H264AVCCOMMONLIB_LIB" /D "H264AVCENCODERLIB_LIB" /D "SHARP_AVC_REWRITE_OUTPUT" /D EXCEL=1 /D "USE_NAMESPACE_H264AVC" /YX"H264AVCEncoderLib.h" /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "H264AVCEncoderLibStatic - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "..\..\..\build\windows\lib\H264AVCEncoderLib\DebugStatic"
# PROP BASE Intermediate_Dir "..\..\..\build\windows\lib\H264AVCEncoderLib\DebugStatic"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\..\..\lib"
# PROP Intermediate_Dir "..\..\..\build\windows\lib\H264AVCEncoderLib\DebugStatic"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "..\..\..\include" /I "." /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "H264AVCVIDEOLIB_LIB" /D "H264AVCCOMMONLIB_LIB" /D "H264AVCENCODERLIB_LIB" /D "SHARP_AVC_REWRITE_OUTPUT" /D "USE_NAMESPACE_H264AVC" /YX"H264AVCEncoderLib.h" /FD /GZ /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\..\..\lib\H264AVCEncoderLibStaticd.lib"

!ENDIF 

# Begin Target

# Name "H264AVCEncoderLibStatic - Win32 Release"
# Name "H264AVCEncoderLibStatic - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\BitCounter.cpp
# End Source File
# Begin Source File

SOURCE=.\BitWriteBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\CabacWriter.cpp
# End Source File
# Begin Source File

SOURCE=.\CabaEncoder.cpp
# End Source File
# Begin Source File

SOURCE=.\CodingParameter.cpp
# End Source File
# Begin Source File

SOURCE=.\ControlMngH264AVCEncoder.cpp
# End Source File
# Begin Source File

SOURCE=.\CreaterH264AVCEncoder.cpp
# End Source File
# Begin Source File

SOURCE=.\Distortion.cpp
# End Source File
# Begin Source File

SOURCE=.\GOPEncoder.cpp
# End Source File
# Begin Source File

SOURCE=.\H264AVCEncoder.cpp
# End Source File
# Begin Source File

SOURCE=.\H264AVCEncoderLib.cpp
# ADD CPP /Yc"H264AVCEncoderLib.h"
# End Source File
# Begin Source File

SOURCE=.\InputPicBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\IntraPredictionSearch.cpp
# End Source File
# Begin Source File

SOURCE=.\MbCoder.cpp
# End Source File
# Begin Source File

SOURCE=.\MbEncoder.cpp
# End Source File
# Begin Source File

SOURCE=.\MbTempData.cpp
# End Source File
# Begin Source File

SOURCE=.\MotionEstimation.cpp
# End Source File
# Begin Source File

SOURCE=.\MotionEstimationCost.cpp
# End Source File
# Begin Source File

SOURCE=.\MotionEstimationQuarterPel.cpp
# End Source File
# Begin Source File

SOURCE=.\NalUnitEncoder.cpp
# End Source File
# Begin Source File

SOURCE=.\PicEncoder.cpp
# End Source File
# Begin Source File

SOURCE=.\RateCtlBase.cpp
# End Source File
# Begin Source File

SOURCE=.\RateCtlQuadratic.cpp
# End Source File
# Begin Source File

SOURCE=.\RateDistortion.cpp
# End Source File
# Begin Source File

SOURCE=.\RecPicBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\Scheduler.cpp
# End Source File
# Begin Source File

SOURCE=.\SequenceStructure.cpp
# End Source File
# Begin Source File

SOURCE=.\SliceEncoder.cpp
# End Source File
# Begin Source File

SOURCE=.\UvlcWriter.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\BitCounter.h
# End Source File
# Begin Source File

SOURCE=.\BitWriteBuffer.h
# End Source File
# Begin Source File

SOURCE=.\BitWriteBufferIf.h
# End Source File
# Begin Source File

SOURCE=.\CabacWriter.h
# End Source File
# Begin Source File

SOURCE=.\CabaEncoder.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\CodingParameter.h
# End Source File
# Begin Source File

SOURCE=.\ControlMngH264AVCEncoder.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\CreaterH264AVCEncoder.h
# End Source File
# Begin Source File

SOURCE=.\Distortion.h
# End Source File
# Begin Source File

SOURCE=.\DistortionIf.h
# End Source File
# Begin Source File

SOURCE=.\GOPEncoder.h
# End Source File
# Begin Source File

SOURCE=.\H264AVCEncoder.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCEncoderLib.h
# End Source File
# Begin Source File

SOURCE=.\InputPicBuffer.h
# End Source File
# Begin Source File

SOURCE=.\IntraPredictionSearch.h
# End Source File
# Begin Source File

SOURCE=.\MbCoder.h
# End Source File
# Begin Source File

SOURCE=.\MbEncoder.h
# End Source File
# Begin Source File

SOURCE=.\MbSymbolWriteIf.h
# End Source File
# Begin Source File

SOURCE=.\MbTempData.h
# End Source File
# Begin Source File

SOURCE=.\MotionEstimation.h
# End Source File
# Begin Source File

SOURCE=.\MotionEstimationCost.h
# End Source File
# Begin Source File

SOURCE=.\MotionEstimationQuarterPel.h
# End Source File
# Begin Source File

SOURCE=.\NalUnitEncoder.h
# End Source File
# Begin Source File

SOURCE=.\PicEncoder.h
# End Source File
# Begin Source File

SOURCE=.\RateDistortion.h
# End Source File
# Begin Source File

SOURCE=.\RateDistortionIf.h
# End Source File
# Begin Source File

SOURCE=.\RecPicBuffer.h
# End Source File
# Begin Source File

SOURCE=.\Scheduler.h
# End Source File
# Begin Source File

SOURCE=.\SequenceStructure.h
# End Source File
# Begin Source File

SOURCE=.\SliceEncoder.h
# End Source File
# Begin Source File

SOURCE=.\UvlcWriter.h
# End Source File
# End Group
# End Target
# End Project
