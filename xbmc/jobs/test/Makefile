SRCS=	\
	TestFileOperationJob.cpp \
	TestJobManager.cpp

LIB=jobsTest.a

INCLUDES += -I../../../lib/gtest/include

include ../../../Makefile.include
-include $(patsubst %.cpp,%.P,$(patsubst %.c,%.P,$(SRCS)))
