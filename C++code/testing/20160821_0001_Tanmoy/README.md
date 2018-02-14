#Test by Tanmoy
##20160821
##Mac arch

nummemes  = 10, numlexes  = 15, numagents = 40

mutrate = 1, penalty = 100

inner = 100, outer = 1000

Random number generator seeded with 123 

##main what (*sccs what main*)

  >  main.c++: $Id$

  >  main.h++: $Id$

  >  selfiterator.h++: $Id$

  >  language.h++: $Id$

  >  counts.h++: $Id$

  >  agent.h++: $Id$

  >  lex.h++: $Id$

  >  meme.h++: $Id$

  >  enum.h++: $Id$

  >  network.h++: $Id$

  >  probvector.h++: $Id$

  >  myutil.h++: $Id$

  >  enumvector.h++: $Id$

##Command:

> (what main; echo 10 15 40 1 100 100 1000 123 | ./main) > check.txt 2>&1

##check.txt md5sum: *d991185a60b725f6cf4eac59a5c88f68*

##Note: 

> in order to have git update the what Id checksums, we need to
      commit AND check out to give it a chance to update th checksum.

