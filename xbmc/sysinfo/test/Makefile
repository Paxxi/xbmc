SRCS=	\
	TestCPUInfo.cpp \
	TestSystemInfo.cpp

LIB=sysinfoTest.a

INCLUDES += -I../../../lib/gtest/include

include ../../../Makefile.include
-include $(patsubst %.cpp,%.P,$(patsubst %.c,%.P,$(SRCS)))
