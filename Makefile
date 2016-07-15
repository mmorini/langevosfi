CXXFLAGS += -std=c++14 -O4 -pedantic -Wall
ETAGS = etags
HFILES = selfiterator.h++ enum.h++ enumvector.h++ myutil.h++ \
	meme.h++ lex.h++ language.h++ agent.h++ network.h++ \
	probvector.h++ counts.h++
CFILES = main.c++
MFILES = Makefile
main: main.o
main.o: main.c++ selfiterator.h++ enum.h++ enumvector.h++ \
	myutil.h++ meme.h++ lex.h++ language.h++ agent.h++ \
	network.h++ probvector.h++ counts.h++
%.o: %.c++
	$(COMPILE.cc) -c '$<'
%: %.o
	$(LINK.cc) -o '$@' $^
TAGS: $(MFILES) $(CFILES) $(HFILES)
	'$(ETAGS)' $^ 
