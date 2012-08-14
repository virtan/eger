AM_CPPFLAGS = -I.. \
	      -I/opt/local/include

ACLOCAL_AMFLAGS = -Im4

lib_LTLIBRARIES = libeger.la

libeger_la_SOURCES = logger.cc \
		     writer.cc
libeger_la_DEPENDENCIES = types.h \
			  logger.h \
			  writer.h
libeger_la_CPPFLAGS = $(AM_CPPFLAGS) $(BOOST_CPPFLAGS)
libeger_la_CXXFLAGS = $(AM_CXXFLAGS) $(BOOST_CXXFLAGS)
libeger_la_LDFLAGS = $(AM_LDFLAGS) $(BOOST_LDFLAGS)
libeger_la_LIBADD = -lpthread $(BOOST_THREAD_LIB)
