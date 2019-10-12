AUTOMAKE_ARGS = --add-missing

lib_LIBRARIES = libutil.a
libutil_a_SOURCES = src/anyutil.cc \
		    src/bayesutil.cc \
		    src/csvutil.cc \
		    src/dateutil.cc \
		    src/floatingpoint.cc \
		    src/graphutil.cc \
		    src/statutil.cc \
		    src/stringutil.cc
AM_CPPFLAGS = -I ./include -std=c++17
ACLOCAL_AMFLAGS = -I /usr/local/share/aclocal
check_PROGRAMS	= utiltests
utiltests_SOURCES= tests/utiltests.cc

LDADD = $(top_builddir)/libutil.a $(BOOST_FILESYSTEM_LIB)

TESTS	= utiltests