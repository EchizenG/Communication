#--------------------------------------------------------------
#               Quectel OpenLinux
#--------------------------------------------------------------
QL_SDK_PATH   ?= $(shell pwd)/../..
QL_EXP_TARGETS = eth_server
QL_EXP_LDLIBS  =

#--------------------------------------------------------
# TOOLS BASIC CONFIG
# Note: No Need to change them
#--------------------------------------------------------
CPPFLAGS += -I./                     \
            -I./inc                  \
            -I../../include          \
            -I$(SDKTARGETSYSROOT)/usr/include               \
            -I$(SDKTARGETSYSROOT)/usr/include               \
            -I$(SDKTARGETSYSROOT)/usr/include/data          \
            -I$(SDKTARGETSYSROOT)/usr/include/dsutils       \
						-Wno-deprecated \

LDFLAGS += -L./                          \
           -L$(SDKTARGETSYSROOT)/usr/lib \
           -lrt

#---------------------
# Source code files
#---------------------
LOCAL_SRC_FILES = server.cpp
				  #client.cpp



eth_server:
#	$(CC) $(CPPFLAGS) $(CFLAGS) $(LOCAL_SRC_FILES)
	$(COMPILE.cc) $(CPPFLAGS) $(LDFLAGS) $(LOCAL_SRC_FILES)
	$(LINK.cc) *.o -lstdc++ -pthread -std=c++11 $(LDFLAGS) $(USR_LIB) $(STD_LIB) -o $@

.PHPNY: all

clean:
	rm -rf $(QL_EXP_TARGETS) *.o

.PHONY:checkmake
checkmake:
	@echo -e "CURDIR =		\n	${CURDIR}"
	@echo -e "\nMAKE_VERSION =	\n	${MAKE_VERSION}"
	@echo -e "\nMAKEFILE_LIST =	\n	${MAKEFILE_LIST}"
	@echo -e "\nCOMPILE.c =		\n	${COMPILE.c}"
	@echo -e "\nCOMPILE.cc =	\n	${COMPILE.cc}"
	@echo -e "\nCOMPILE.cpp =	\n	${COMPILE.cpp}"
	@echo -e "\nLINK.cc =		\n	${LINK.cc}"
	@echo -e "\nLINK.o =		\n	${LINK.o}"
	@echo -e "\nCPPFLAGS =		\n	${CPPFLAGS}"
	@echo -e "\nCFLAGS =		\n	${CFLAGS}"
	@echo -e "\nCXXFLAGS =		\n	${CXXFLAGS}"
	@echo -e "\nLDFLAGS =		\n	${LDFLAGS}"
	@echo -e "\nLDLIBS =		\n	${LDLIBS}"
