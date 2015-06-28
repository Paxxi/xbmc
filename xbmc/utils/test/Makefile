SRCS=	\
	TestAliasShortcutUtils.cpp \
	TestArchive.cpp \
	TestBase64.cpp \
	TestBitstreamStats.cpp \
	TestCrc32.cpp \
	TestCryptThreading.cpp \
	TestDatabaseUtils.cpp \
	TestEndianSwap.cpp \
	Testfastmemcpy.cpp \
	Testfstrcmp.cpp \
	TestGlobalsHandling.cpp \
	TestHTMLUtil.cpp \
	TestHttpHeader.cpp \
	TestHttpParser.cpp \
	TestHttpRangeUtils.cpp \
	TestHttpResponse.cpp \
	TestJSONVariantParser.cpp \
	TestJSONVariantWriter.cpp \
	Testlog.cpp \
	TestMathUtils.cpp \
	Testmd5.cpp \
	TestMime.cpp \
	TestPerformanceSample.cpp \
	TestPOUtils.cpp \
	TestRegExp.cpp \
	Testrfft.cpp \
	TestRingBuffer.cpp \
	TestSortUtils.cpp \
	TestStopwatch.cpp \
	TestStreamDetails.cpp \
	TestStreamUtils.cpp \
	TestStringUtils.cpp \
	TestTimeSmoother.cpp \
	TestTimeUtils.cpp \
	TestURIUtils.cpp \
	TestUrlOptions.cpp \
	TestVariant.cpp \
	TestXBMCTinyXML.cpp \
	TestXMLUtils.cpp

LIB=utilsTest.a

INCLUDES += -I../../../lib/gtest/include

include ../../../Makefile.include
-include $(patsubst %.cpp,%.P,$(patsubst %.c,%.P,$(SRCS)))
