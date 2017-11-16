#"@(#)Makefile $Id$"
# CXXFLAGS += -std=c++14
CXXFLAGS += -std=c++11 
CXXFLAGS += -O4 -pedantic -Wall
LDFLAGS += -lhdf5_cpp -lhdf5
ETAGS = 'etags'
GIT = 'git'

HFILES = selfiterator.h++ enum.h++ enumvector.h++ myutil.h++ \
	meme.h++ lex.h++ language.h++ agent.h++ network.h++ \
	probvector.h++ counts.h++ main.h++ main_decls.h++ experience.h++ \
	h5util.h++ sccs.h++
CFILES = main.c++
MFILES = Makefile

main: main.o
# main.o: main.c++ main.h++.gch
main.o: $(CFILES) $(HFILES)
%.o: %.c++
	$(COMPILE.cc) '$<'
%: %.o
	$(LINK.cc) -o '$@' $(^:%='%')

.SUFFIXES: .h++.gch
%.h++.gch: %.h++ sccs.h++
	$(COMPILE.cc) '$<'
enumvector.h++: h5util.h++
probvector.h++.gch: probvector.h++ enumvector.h++ myutil.h++ h5util.h++
network.h++.gch: network.h++ probvector.h++
language.h++.gch: language.h++ enumvector.h++ probvector.h++ counts.h++
counts.h++.gch: counts.h++ enumvector.h++
main.h++.gch: main.h++ network.h++ probvector.h++ enumvector.h++ myutil.h++ \
              enum.h++ meme.h++ lex.h++ agent.h++ language.h++ counts.h++ \
              selfiterator.h++ main_decls.h++ experience.h++ h5util.h++

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
