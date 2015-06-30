SRCS=	\
	TestLabelFormatter.cpp \
	TestSortUtils.cpp

LIB=presentationTest.a

INCLUDES += -I../../../lib/gtest/include

include ../../../Makefile.include
-include $(patsubst %.cpp,%.P,$(patsubst %.c,%.P,$(SRCS)))
