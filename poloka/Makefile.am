CLEANFILES = *~
MAINTAINERCLEANFILES = \
	Makefile.in \
	stamp-*

lib_LTLIBRARIES = libpoloka-dao.la

src_includedir = $(includedir)/poloka
src_include_HEADERS = \
	daophotio.h \
	daostar.h

libpoloka_dao_la_SOURCES = \
	$(src_include_HEADERS) \
	daophotio.cc \
	daostar.cc

libpoloka_dao_la_CPPFLAGS = @POLOKA_CORE_CFLAGS@

libpoloka_dao_la_LIBADD = @POLOKA_CORE_LIBS@


