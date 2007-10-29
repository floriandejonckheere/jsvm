# the SOURCE definiton lets you move your makefile to another position
CONFIG 			= CONSOLE

# set directories to your wanted values
SRC_DIR			= ../../../../src/test/MCTFPreProcessor
INC_DIR			= ../../../../include
LIB_DIR			= ../../../../../../lib
BIN_DIR			= ../../../../../../bin

SRC_DIR1		=
SRC_DIR2		=
SRC_DIR3		=
SRC_DIR4		=

USER_INC_DIRS	= -I$(SRC_DIR)
USER_LIB_DIRS	=

# intermediate directory for object files
OBJ_DIR			= ./objects

# set executable name
PRJ_NAME		= MCTFPreProcessor

# defines to set
DEFS			= -DMSYS_LINUX -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -DMSYS_UNIX_LARGEFILE

# set objects
OBJS          		= 	\
				$(OBJ_DIR)/MCTF.o \
				$(OBJ_DIR)/MCTFPreProcessor.o \
				$(OBJ_DIR)/MCTFPreProcessorTest.o \
				$(OBJ_DIR)/PreProcessor.o \
				$(OBJ_DIR)/PreProcessorParameter.o \

# set libs to link with
#LIBS			= -ldl

DEBUG_LIBS		=
RELEASE_LIBS		=

STAT_LIBS		= -lpthread
DYN_LIBS		=


DYN_DEBUG_LIBS		= -lH264AVCVideoIoLibd -lH264AVCEncoderLibd -lH264AVCCommonLibd 
DYN_DEBUG_PREREQS	= $(LIB_DIR)/libH264AVCVideoIoLibd.a $(LIB_DIR)/libH264AVCEncoderLibd.a $(LIB_DIR)/libH264AVCCommonLibd.a
STAT_DEBUG_LIBS		=  -lH264AVCVideoIoLibStaticd -lH264AVCEncoderLibStaticd -lH264AVCCommonLibStaticd
STAT_DEBUG_PREREQS	= $(LIB_DIR)/libH264AVCVideoIoLibStaticd.a $(LIB_DIR)/libH264AVCEncoderLibStaticd.a $(LIB_DIR)/libH264AVCCommonLibStaticd.a

DYN_RELEASE_LIBS	= -lH264AVCVideoIoLib -lH264AVCEncoderLib -lH264AVCCommonLib 
DYN_RELEASE_PREREQS	= $(LIB_DIR)/libH264AVCVideoIoLib.a $(LIB_DIR)/libH264AVCEncoderLib.a $(LIB_DIR)/libH264AVCCommonLib.a
STAT_RELEASE_LIBS	= -lH264AVCVideoIoLibStatic -lH264AVCEncoderLibStatic -lH264AVCCommonLibStatic
STAT_RELEASE_PREREQS	= $(LIB_DIR)/libH264AVCVideoIoLibStatic.a $(LIB_DIR)/libH264AVCEncoderLibStatic.a  $(LIB_DIR)/libH264AVCCommonLibStatic.a


# name of the base makefile
MAKE_FILE_NAME		= ../../common/makefile.base

# include the base makefile
include $(MAKE_FILE_NAME)
