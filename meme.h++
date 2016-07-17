#ifndef MEME_HPP
#define MEME_HPP

static const char MEME_HPP_SCCS_ID[] __attribute__((used)) = "@(#)meme.h++: $Id$";

template<class Memebase>
class Meme: public Memebase {
public:
  virtual ~Meme(){}
  explicit Meme(const int&  n):Memebase(n){}
  Meme(const Memebase& m):Memebase(m){}
  Meme(const typename Memebase::Enum& m): Memebase(m){} 
};
#endif
