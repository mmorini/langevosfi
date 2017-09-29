#ifndef MEME_HPP
#define MEME_HPP

static const char MEME_HPP_SCCS_ID[] __attribute__((used)) = "@(#)meme.h++: $Id$";

namespace Meme {

template<class Memebase>
class Meme: public Memebase {
public:
  using Memebase::Memebase;
  
  virtual ~Meme(){}
  constexpr Meme(const Memebase& m):Memebase(m){}
  constexpr Meme() {}
};

}
#endif
