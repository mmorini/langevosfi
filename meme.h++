#ifndef MEME_HPP
#define MEME_HPP

static const char MEME_HPP_SCCS_ID[] __attribute__((used)) = "@(#)meme.h++: $Id$";

class Meme: public Memebase {
public:
  explicit Meme(const int&  n):Memebase(n){}
  Meme(const Memebase& m):Memebase(m){}
  Meme(const Enum& m): Memebase(m){} 
};
#endif
