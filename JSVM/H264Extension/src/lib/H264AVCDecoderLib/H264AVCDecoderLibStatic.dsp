# Microsoft Developer Studio Project File - Name="H264AVCDecoderLibStatic" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=H264AVCDecoderLibStatic - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "H264AVCDecoderLibStatic.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "H264AVCDecoderLibStatic.mak" CFG="H264AVCDecoderLibStatic - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "H264AVCDecoderLibStatic - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "H264AVCDecoderLibStatic - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "H264AVCDecoderLibStatic - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "..\..\..\build\windows\lib\H264AVCDecoderLib\ReleaseStatic"
# PROP BASE Intermediate_Dir "..\..\..\build\windows\lib\H264AVCDecoderLib\ReleaseStatic"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\..\..\lib"
# PROP Intermediate_Dir "..\..\..\build\windows\lib\H264AVCDecoderLib\ReleaseStatic"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /I "..\..\..\include" /I "." /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "H264AVCCOMMONLIB_LIB" /D "H264AVCDECODERLIB_LIB" /D "USE_NAMESPACE_H264AVC" /YX"H264AVCDecoderLib.h" /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "H264AVCDecoderLibStatic - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "..\..\..\build\windows\lib\H264AVCDecoderLib\DebugStatic"
# PROP BASE Intermediate_Dir "..\..\..\build\windows\lib\H264AVCDecoderLib\DebugStatic"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\..\..\lib"
# PROP Intermediate_Dir "..\..\..\build\windows\lib\H264AVCDecoderLib\DebugStatic"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "..\..\..\include" /I "." /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "H264AVCCOMMONLIB_LIB" /D "H264AVCDECODERLIB_LIB" /D "USE_NAMESPACE_H264AVC" /YX"H264AVCDecoderLib.h" /FD /GZ /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\..\..\lib\H264AVCDecoderLibStaticd.lib"

!ENDIF 

# Begin Target

# Name "H264AVCDecoderLibStatic - Win32 Release"
# Name "H264AVCDecoderLibStatic - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\BitReadBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\CabacReader.cpp
# End Source File
# Begin Source File

SOURCE=.\CabaDecoder.cpp
# End Source File
# Begin Source File

SOURCE=.\ControlMngH264AVCDecoder.cpp
# End Source File
# Begin Source File

SOURCE=.\CreaterH264AVCDecoder.cpp
# End Source File
# Begin Source File

SOURCE=.\GOPDecoder.cpp
# End Source File
# Begin Source File

SOURCE=.\H264AVCDecoder.cpp
# End Source File
# Begin Source File

SOURCE=.\H264AVCDecoderLib.cpp
# ADD CPP /Yc"H264AVCDecoderLib.h"
# End Source File
# Begin Source File

SOURCE=.\MbDecoder.cpp
# End Source File
# Begin Source File

SOURCE=.\MbParser.cpp
# End Source File
# Begin Source File

SOURCE=.\NalUnitParser.cpp
# End Source File
# Begin Source File

SOURCE=.\SliceDecoder.cpp
# End Source File
# Begin Source File

SOURCE=.\SliceReader.cpp
# End Source File
# Begin Source File

SOURCE=.\UvlcReader.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\BitReadBuffer.h
# End Source File
# Begin Source File

SOURCE=.\CabacReader.h
# End Source File
# Begin Source File

SOURCE=.\CabaDecoder.h
# End Source File
# Begin Source File

SOURCE=.\ControlMngH264AVCDecoder.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\CreaterH264AVCDecoder.h
# End Source File
# Begin Source File

SOURCE=.\GOPDecoder.h
# End Source File
# Begin Source File

SOURCE=.\H264AVCDecoder.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\H264AVCDecoderLib.h
# End Source File
# Begin Source File

SOURCE=.\MbDecoder.h
# End Source File
# Begin Source File

SOURCE=.\MbParser.h
# End Source File
# Begin Source File

SOURCE=.\MbSymbolReadIf.h
# End Source File
# Begin Source File

SOURCE=.\NalUnitParser.h
# End Source File
# Begin Source File

SOURCE=.\SliceDecoder.h
# End Source File
# Begin Source File

SOURCE=.\SliceReader.h
# End Source File
# Begin Source File

SOURCE=.\UvlcReader.h
# End Source File
# End Group
# End Target
# End Project
