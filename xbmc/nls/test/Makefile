SRCS=	\
	TestCharsetConverter.cpp \
	TestLangCodeExpander.cpp \
	TestLocale.cpp \

LIB=nlsTest.a

INCLUDES += -I../../../lib/gtest/include

include ../../../Makefile.include
-include $(patsubst %.cpp,%.P,$(patsubst %.c,%.P,$(SRCS)))
