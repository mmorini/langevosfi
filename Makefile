#"@(#)Makefile $Id$"
# CXXFLAGS += -std=c++14
CXXFLAGS += -std=c++11
CXXFLAGS += -O4 -pedantic -Wall
ETAGS = etags
HFILES = selfiterator.h++ enum.h++ enumvector.h++ myutil.h++ \
	meme.h++ lex.h++ language.h++ agent.h++ network.h++ \
	probvector.h++ counts.h++ main.h++
CFILES = main.c++
MFILES = Makefile
main: main.o
main.o: main.c++ main.h++.gch
%.o: %.c++
	$(COMPILE.cc) '$<'
%: %.o
	$(LINK.cc) -o '$@' $^

.SUFFIXES: .h++.gch
%.h++.gch: %.h++
	$(COMPILE.cc) '$<'
probvector.h++.gch: probvector.h++ enumvector.h++ myutil.h++
network.h++.gch: network.h++ probvector.h++
language.h++.gch: language.h++ enumvector.h++ probvector.h++ counts.h++
counts.h++.gch: counts.h++ enumvector.h++
main.h++.gch: main.h++ network.h++ probvector.h++ enumvector.h++ myutil.h++ \
              enum.h++ meme.h++ lex.h++ agent.h++ language.h++ counts.h++ \
              selfiterator.h++ main_decls.h++

TAGS: $(MFILES) $(CFILES) $(HFILES)
	'$(ETAGS)' $^ 

clean:
	rm -f main main.o selfiterator.h++.gch enum.h++.gch enumvector.h++.gch \
	   myutil.h++.gch language.h++.gch network.h++.gch probvector.h++.gch \
	   counts.h++.gch agent.h++.gch meme.h++.gch main.h++.gch lex.h++.gch \
	   TAGS
	rm -rf main.dSYM
