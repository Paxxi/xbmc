SRCS=	\
	TestScraperParser.cpp \
	TestScraperUrl.cpp

LIB=scraperTest.a

INCLUDES += -I../../lib/gtest/include

include ../../Makefile.include
-include $(patsubst %.cpp,%.P,$(patsubst %.c,%.P,$(SRCS)))
