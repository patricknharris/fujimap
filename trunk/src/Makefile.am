lib_LTLIBRARIES = libfujimap.la
libfujimap_la_CPPFLAGS = -D_REENTRANT
libfujimap_la_SOURCES = \
	fujimap.cpp fujimap.hpp \
	fujimap_block.cpp fujimap_block.hpp \
	common.hpp \
	bitvec.cpp bitvec.hpp \
	key_edge.cpp key_edge.hpp

pkginclude_HEADERS = fujimap.hpp

bin_PROGRAMS = fujimap fujimap_test

fujimap_LDADD = libfujimap.la
fujimap_SOURCES = fujimap_main.cpp cmdline.h

fujimap_test_LDADD = libfujimap.la
fujimap_test_SOURCES = fujimap_test.cpp cmdline.h
