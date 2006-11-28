# the SOURCE definiton lets you move your makefile to another position
CONFIG 				= CONSOLE

# set directories to your wanted values
SRC_DIR				= ../../../../src/test/H264AVCDecoderLibTest
INC_DIR				= ../../../../include
LIB_DIR				= ../../../../../../lib
BIN_DIR				= ../../../../../../bin

SRC_DIR1		=
SRC_DIR2		=
SRC_DIR3		=
SRC_DIR4		=

USER_INC_DIRS	= -I$(SRC_DIR) 
USER_LIB_DIRS	=

# intermediate directory for object files
OBJ_DIR				= ./objects

# set executable name
PRJ_NAME			= H264AVCDecoderLibTest

# defines to set
DEFS				= -DMSYS_LINUX -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -DMSYS_UNIX_LARGEFILE

# set objects
OBJS          		= 	\
					$(OBJ_DIR)/H264AVCDecoderLibTest.o \
					$(OBJ_DIR)/DecoderParameter.o \
					$(OBJ_DIR)/H264AVCDecoderTest.o \

# set libs to link with
LIBS				= -ldl

DEBUG_LIBS			=
RELEASE_LIBS		=

STAT_LIBS			= -lpthread
DYN_LIBS			=


DYN_DEBUG_LIBS		= -lH264AVCDecoderLibd -lH264AVCCommonLibd -lH264AVCVideoIoLibd
DYN_DEBUG_PREREQS		= $(LIB_DIR)/libH264AVCDecoderLibd.a $(LIB_DIR)/libH264AVCCommonLibd.a $(LIB_DIR)/libH264AVCVideoIoLibd.a
STAT_DEBUG_LIBS		= -lH264AVCDecoderLibStaticd -lH264AVCCommonLibStaticd -lH264AVCVideoIoLibStaticd
STAT_DEBUG_PREREQS		= $(LIB_DIR)/libH264AVCDecoderLibStaticd.a $(LIB_DIR)/libH264AVCCommonLibStaticd.a $(LIB_DIR)/libH264AVCVideoIoLibStaticd.a

DYN_RELEASE_LIBS	= -lH264AVCDecoderLib -lH264AVCCommonLib -lH264AVCVideoIoLib
DYN_RELEASE_PREREQS	= $(LIB_DIR)/libH264AVCDecoderLib.a $(LIB_DIR)/libH264AVCCommonLib.a $(LIB_DIR)/libH264AVCVideoIoLib.a
STAT_RELEASE_LIBS	= -lH264AVCDecoderLibStatic -lH264AVCCommonLibStatic -lH264AVCVideoIoLibStatic
STAT_RELEASE_PREREQS	= $(LIB_DIR)/libH264AVCDecoderLibStatic.a $(LIB_DIR)/libH264AVCCommonLibStatic.a $(LIB_DIR)/libH264AVCVideoIoLibStatic.a


# name of the base makefile
MAKE_FILE_NAME		= ../../common/makefile.base

# include the base makefile
include $(MAKE_FILE_NAME)
