#"@(#)Makefile $Id$"
CXXFLAGS += -std=c++14 -O4 -pedantic -Wall
ETAGS = etags
HFILES = selfiterator.h++ enum.h++ enumvector.h++ myutil.h++ \
	meme.h++ lex.h++ language.h++ agent.h++ network.h++ \
	probvector.h++ counts.h++
CFILES = main.c++
MFILES = Makefile
main: main.o
main.o: main.c++ selfiterator.h++.gch enum.h++.gch enumvector.h++.gch \
	myutil.h++.gch meme.h++ lex.h++ language.h++.gch agent.h++ \
	network.h++.gch probvector.h++.gch counts.h++.gch
%.o: %.c++
	$(COMPILE.cc) -c '$<'
%: %.o
	$(LINK.cc) -o '$@' $^

.SUFFIXES: .h++.gch
%.h++.gch: %.h++
	$(COMPILE.cc) -c '$<'
probvector.h++.gch: enumvector.h++.gch myutil.h++.gch
network.h++.gch: probvector.h++.gch
language.h++.gch: enumvector.h++.gch probvector.h++.gch counts.h++.gch
counts.h++.gch: enumvector.h++.gch
lex.h++.gch meme.h++.gch agent.h++.gch:
	$(error lex, meme, or agent header files can't be precompiled)

TAGS: $(MFILES) $(CFILES) $(HFILES)
	'$(ETAGS)' $^ 

clean:
	rm -f main main.o selfiterator.h++.gch enum.h++.gch enumvector.h++.gch \
	   myutil.h++.gch language.h++.gch network.h++.gch probvector.h++.gch \
	   counts.h++.gch TAGS
