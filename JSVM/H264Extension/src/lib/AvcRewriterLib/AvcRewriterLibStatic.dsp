# Microsoft Developer Studio Project File - Name="AvcRewriterLibStatic" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=AvcRewriterLibStatic - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "AvcRewriterLibStatic.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "AvcRewriterLibStatic.mak" CFG="AvcRewriterLibStatic - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "AvcRewriterLibStatic - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "AvcRewriterLibStatic - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "AvcRewriterLibStatic - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "..\..\..\build\windows\lib\AvcRewriterLib\ReleaseStatic"
# PROP BASE Intermediate_Dir "..\..\..\build\windows\lib\AvcRewriterLib\ReleaseStatic"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\..\..\lib"
# PROP Intermediate_Dir "..\..\..\build\windows\lib\AvcRewriterLib\ReleaseStatic"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /I "..\..\..\include" /I "." /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "H264AVCCOMMONLIB_LIB" /D "H264AVCDECODERLIB_LIB" /D "H264AVCENCODERLIB_LIB" /D "SHARP_AVC_REWRITE_OUTPUT" /D "USE_NAMESPACE_H264AVC" /YX"AvcRewriterLib.h" /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "AvcRewriterLibStatic - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "..\..\..\build\windows\lib\AvcRewriterLib\DebugStatic"
# PROP BASE Intermediate_Dir "..\..\..\build\windows\lib\AvcRewriterLib\DebugStatic"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\..\..\lib"
# PROP Intermediate_Dir "..\..\..\build\windows\lib\AvcRewriterLib\DebugStatic"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "..\..\..\include" /I "." /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "H264AVCCOMMONLIB_LIB" /D "H264AVCDECODERLIB_LIB" /D "H264AVCENCODERLIB_LIB" /D "SHARP_AVC_REWRITE_OUTPUT" /D "USE_NAMESPACE_H264AVC" /YX"AvcRewriterLib.h" /FD /GZ /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\..\..\lib\AvcRewriterLibStaticd.lib"

!ENDIF 

# Begin Target

# Name "AvcRewriterLibStatic - Win32 Release"
# Name "AvcRewriterLibStatic - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\H264AVCDecoderLib\BitReadBuffer.cpp
# End Source File
# Begin Source File

SOURCE=..\H264AVCDecoderLib\CabacReader.cpp
# End Source File
# Begin Source File

SOURCE=..\H264AVCDecoderLib\CabaDecoder.cpp
# End Source File
# Begin Source File

SOURCE=..\H264AVCDecoderLib\ControlMngH264AVCDecoder.cpp
# End Source File
# Begin Source File

SOURCE=..\H264AVCDecoderLib\CreaterH264AVCDecoder.cpp
# End Source File
# Begin Source File

SOURCE=..\H264AVCDecoderLib\GOPDecoder.cpp
# End Source File
# Begin Source File

SOURCE=..\H264AVCDecoderLib\H264AVCDecoder.cpp
# End Source File
# Begin Source File

SOURCE=..\H264AVCDecoderLib\H264AVCDecoderLib.cpp
# End Source File
# Begin Source File

SOURCE=..\H264AVCDecoderLib\MbDecoder.cpp
# End Source File
# Begin Source File

SOURCE=..\H264AVCDecoderLib\MbParser.cpp
# End Source File
# Begin Source File

SOURCE=..\H264AVCDecoderLib\NalUnitParser.cpp
# End Source File
# Begin Source File

SOURCE=..\H264AVCDecoderLib\SliceDecoder.cpp
# End Source File
# Begin Source File

SOURCE=..\H264AVCDecoderLib\SliceReader.cpp
# End Source File
# Begin Source File

SOURCE=..\H264AVCDecoderLib\UvlcReader.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\H264AVCDecoderLib\BitReadBuffer.h
# End Source File
# Begin Source File

SOURCE=..\H264AVCDecoderLib\CabacReader.h
# End Source File
# Begin Source File

SOURCE=..\H264AVCDecoderLib\CabaDecoder.h
# End Source File
# Begin Source File

SOURCE=..\H264AVCDecoderLib\ControlMngH264AVCDecoder.h
# End Source File
# Begin Source File

SOURCE=..\H264AVCDecoderLib\DecError.h
# End Source File
# Begin Source File

SOURCE=..\H264AVCDecoderLib\GOPDecoder.h
# End Source File
# Begin Source File

SOURCE=..\H264AVCDecoderLib\H264AVCDecoder.h
# End Source File
# Begin Source File

SOURCE=..\H264AVCDecoderLib\MbDecoder.h
# End Source File
# Begin Source File

SOURCE=..\H264AVCDecoderLib\MbParser.h
# End Source File
# Begin Source File

SOURCE=..\H264AVCDecoderLib\MbSymbolReadIf.h
# End Source File
# Begin Source File

SOURCE=..\H264AVCDecoderLib\NalUnitParser.h
# End Source File
# Begin Source File

SOURCE=..\H264AVCDecoderLib\resource.h
# End Source File
# Begin Source File

SOURCE=..\H264AVCDecoderLib\SliceDecoder.h
# End Source File
# Begin Source File

SOURCE=..\H264AVCDecoderLib\SliceReader.h
# End Source File
# Begin Source File

SOURCE=..\H264AVCDecoderLib\UvlcReader.h
# End Source File
# End Group
# End Target
# End Project
