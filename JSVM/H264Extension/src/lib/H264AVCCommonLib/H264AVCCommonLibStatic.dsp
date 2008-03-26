# Microsoft Developer Studio Project File - Name="H264AVCCommonLibStatic" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=H264AVCCommonLibStatic - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "H264AVCCommonLibStatic.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "H264AVCCommonLibStatic.mak" CFG="H264AVCCommonLibStatic - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "H264AVCCommonLibStatic - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "H264AVCCommonLibStatic - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "H264AVCCommonLibStatic - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "..\..\..\build\windows\lib\H264AVCCommonLib\ReleaseStatic"
# PROP BASE Intermediate_Dir "..\..\..\build\windows\lib\H264AVCCommonLib\ReleaseStatic"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\..\..\lib"
# PROP Intermediate_Dir "..\..\..\build\windows\lib\H264AVCCommonLib\ReleaseStatic"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /I "..\..\..\include" /I "." /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "H264AVCVIDEOLIB_LIB" /D "H264AVCCOMMONLIB_LIB" /D "USE_NAMESPACE_H264AVC" /YX"H264AVCCommonLib.h" /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "H264AVCCommonLibStatic - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "..\..\..\build\windows\lib\H264AVCCommonLib\DebugStatic"
# PROP BASE Intermediate_Dir "..\..\..\build\windows\lib\H264AVCCommonLib\DebugStatic"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\..\..\lib"
# PROP Intermediate_Dir "..\..\..\build\windows\lib\H264AVCCommonLib\DebugStatic"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "..\..\..\include" /I "." /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "H264AVCVIDEOLIB_LIB" /D "H264AVCCOMMONLIB_LIB" /D "USE_NAMESPACE_H264AVC" /YX"H264AVCCommonLib.h" /FD /GZ /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\..\..\lib\H264AVCCommonLibStaticd.lib"

!ENDIF 

# Begin Target

# Name "H264AVCCommonLibStatic - Win32 Release"
# Name "H264AVCCommonLibStatic - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\CabacContextModel.cpp
# End Source File
# Begin Source File

SOURCE=.\CabacContextModel2DBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\CFMO.cpp
# End Source File
# Begin Source File

SOURCE=.\DownConvert.cpp
# End Source File
# Begin Source File

SOURCE=.\Frame.cpp
# End Source File
# Begin Source File

SOURCE=.\H264AVCCommonLib.cpp
# ADD CPP /Yc"H264AVCCommonLib.h"
# End Source File
# Begin Source File

SOURCE=.\Hrd.cpp
# End Source File
# Begin Source File

SOURCE=.\IntraPrediction.cpp
# End Source File
# Begin Source File

SOURCE=.\LoopFilter.cpp
# End Source File
# Begin Source File

SOURCE=.\MbData.cpp
# End Source File
# Begin Source File

SOURCE=.\MbDataAccess.cpp
# End Source File
# Begin Source File

SOURCE=.\MbDataCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\MbDataStruct.cpp
# End Source File
# Begin Source File

SOURCE=.\MbMvData.cpp
# End Source File
# Begin Source File

SOURCE=.\MbTransformCoeffs.cpp
# End Source File
# Begin Source File

SOURCE=.\MotionCompensation.cpp
# End Source File
# Begin Source File

SOURCE=.\MotionVectorCalculation.cpp
# End Source File
# Begin Source File

SOURCE=.\Mv.cpp
# End Source File
# Begin Source File

SOURCE=.\ParameterSetMng.cpp
# End Source File
# Begin Source File

SOURCE=.\PictureParameterSet.cpp
# End Source File
# Begin Source File

SOURCE=.\PocCalculator.cpp
# End Source File
# Begin Source File

SOURCE=.\Quantizer.cpp
# End Source File
# Begin Source File

SOURCE=.\QuarterPelFilter.cpp
# End Source File
# Begin Source File

SOURCE=.\ReconstructionBypass.cpp
# End Source File
# Begin Source File

SOURCE=.\ResizeParameters.cpp
# End Source File
# Begin Source File

SOURCE=.\SampleWeighting.cpp
# End Source File
# Begin Source File

SOURCE=.\ScalingMatrix.cpp
# End Source File
# Begin Source File

SOURCE=.\Sei.cpp
# End Source File
# Begin Source File

SOURCE=.\SequenceParameterSet.cpp
# End Source File
# Begin Source File

SOURCE=.\SliceHeader.cpp
# End Source File
# Begin Source File

SOURCE=.\SliceHeaderBase.cpp
# End Source File
# Begin Source File

SOURCE=.\Tables.cpp
# End Source File
# Begin Source File

SOURCE=.\TraceFile.cpp
# End Source File
# Begin Source File

SOURCE=.\Transform.cpp
# End Source File
# Begin Source File

SOURCE=.\Vui.cpp
# End Source File
# Begin Source File

SOURCE=.\YuvBufferCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\YuvMbBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\YuvPicBuffer.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\CabacContextModel.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\CabacContextModel2DBuffer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\CabacTables.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\CFMO.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\CommonBuffers.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\CommonDefs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\CommonTypes.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\ContextTables.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\ControlMngIf.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\DownConvert.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\Frame.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\GlobalFunctions.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonIf.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\HeaderSymbolReadIf.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\HeaderSymbolWriteIf.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\Hrd.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\IntraPrediction.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\LoopFilter.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\Macros.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\MbData.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\MbDataAccess.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\MbDataCtrl.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\MbDataStruct.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\MbMvData.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\MbTransformCoeffs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\MotionCompensation.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\MotionVectorCalculation.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\Mv.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\ParameterSetMng.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\PictureParameterSet.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\PocCalculator.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\Quantizer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\QuarterPelFilter.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\ReconstructionBypass.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\ResizeParameters.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\SampleWeighting.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\ScalingMatrix.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\Sei.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\SequenceParameterSet.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\SliceHeader.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\SliceHeaderBase.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\Tables.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\TraceFile.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\Transform.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\Vui.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\YuvBufferCtrl.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\YuvMbBuffer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCCommonLib\YuvPicBuffer.h
# End Source File
# End Group
# End Target
# End Project
