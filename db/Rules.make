# Contains common definitions to all sub-dir and the the autodeps
# rules. Do not add rules here to clarify how the Makefile works.

# common deps
prefix=$(DESTDIR)/usr/local
exec_prefix=${prefix}
BINDIR=${exec_prefix}/bin

MKDIR_P=mkdir -p

CXXFLAGS=-pedantic -Wall -ansi -Wstrict-prototypes -Wmissing-prototypes \
  -Wmissing-declarations -W -Wunused -O1

# autodeps rules. These are modified from an example in the make info page.
CPPFLAGS_DEPS=

.deps/%.cpp.d: %.cpp
	@$(MKDIR_P) .deps
	@set -e; \
	g++ $(CPPFLAGS_DEPS) -MM $< | sed 's|\($*\)\.o[ :]*|\1.o $@ : |g' > $@; \
	[ -s $@ ] || rm -f $@ ;

.deps/%.c.d: %.c
	@$(MKDIR_P) .deps
	@set -e; \
	g++ $(CPPFLAGS_DEPS) -MM $< | sed 's|\($*\)\.o[ :]*|\1.o $@ : |g' > $@; \
	[ -s $@ ] || rm -f $@ ;

# include the dependency files
DEPFILES=$(ALL_SOURCES:=.d)
-include $(DEPFILES:%=.deps/%)
