#ifndef MEME_HPP
#define MEME_HPP

static const char MEME_HPP_SCCS_ID[] __attribute__((used)) = "@(#)meme.h++: $Id$";

template<class Memebase>
class Meme: public Memebase {
public:
  virtual ~Meme(){}
  explicit constexpr Meme(const int&  n):Memebase(n){}
  constexpr Meme(const Memebase& m):Memebase(m){}
  constexpr Meme(const typename Memebase::Enum& m): Memebase(m){}
  constexpr Meme() {}
};
#endif
