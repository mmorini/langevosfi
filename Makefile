#"@(#)Makefile $Id$"
ifeq ($(origin CXX), default)
 CXX = 'h5c++'
endif
# LDFLAGS += '-lhdf5_cpp' '-lhdf5'
CXXFLAGS += '-std=c++11'
# CXXFLAGS += '-std=c++14'
CXXFLAGS += '-O4'
# CXXFLAGS += '-g'
CXXFLAGS += '-pedantic' '-Wall'
ETAGS = 'etags'
GIT = 'git'

HFILES = selfiterator.h++ enum.h++ enumvector.h++ myutil.h++ \
	meme.h++ lex.h++ language.h++ agent.h++ network.h++ \
	probvector.h++ counts.h++ main.h++ main_decls.h++ experience.h++ \
	h5_datatype.h++ h5_dataspace.h++ h5util.h++ sccs.h++
CFILES = main.cc
MFILES = Makefile

main: main.o
# main.o: main.cc main.h++.gch
main.o: $(CFILES) $(HFILES)
%.o: %.cc
	$(COMPILE.cc) '$<'
%: %.o
	$(LINK.cc) -o '$@' $(^:%='%')

.SUFFIXES: .h++.gch
%.h++.gch: %.h++ sccs.h++
	$(COMPILE.cc) '$<'
agent.h++: sccs.h++
counts.h++: enumvector.h++ sccs.h++
enumvector.h++: h5util.h++ selfiterator.h++ myutil.h++ sccs.h++
h5_dataspace.h++: h5_datatype.h++ sccs.h++
h5_datatype.h++: sccs.h++
h5util.h++: sccs.h++
io.h++: enumvector.h++ probvector.h++ language.h++ network.h++ sccs.h++ \
        myutil.h++ main.h++
language.h++: enumvector.h++ probvector.h++ experience.h++ sccs.h++
lex.h++: sccs.h++
main.cc: main.h++ main_decls.h++
main.h++: network.h++ enum.h++ meme.h++ lex.h++ agent.h++ language.h++ \
          enumvector.h++ counts.h++ selfiterator.h++ io.h++ \
          experience.h++ h5util.h++ sccs.h++
main_decls.h++: main.h++ sccs.h++
meme.h++: sccs.h++
myutil.h++: sccs.h++
network.h++: probvector.h++ myutil.h++ sccs.h++
probvectorh++: enumvector.h++ myutil.h++ selfiterator.h++ h5util.h++ sccs.h++
sccs.h++: h5util.h++
selfiterator.h++: sccs.h++

TAGS: $(MFILES) $(CFILES) $(HFILES)
	$(ETAGS) $(^:%='%')

.PHONY: clean gitclean
clean:
	$(RM) 'main' 'main.o' $(HFILES:%='%.gch') 'TAGS'
	$(RM) -r 'main.dSYM'

GITCLEAN=$(shell '$(GIT)' status -s $(CFILES:%='%') $(HFILES:%='%') $(MFILES:%='%'))
gitclean: clean
ifeq ($(GITCLEAN),)
	$(RM) $(CFILES:%='%') $(HFILES:%='%') $(MFILES:%='%')
	$(GIT) checkout .
else
	@echo 'Aborting because git modified'
	@$(GIT) status -s
endif
