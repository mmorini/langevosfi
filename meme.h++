#ifndef MEME_HPP
#define MEME_HPP

namespace Meme {

static const char MEME_HPP_SCCS_ID[] __attribute__((used)) = "@(#)meme.h++: $Id$";

template<class Memebase>
class Meme: public Memebase {
public:
  using Memebase::Memebase;
  
  virtual ~Meme() override = default;
  constexpr Meme(const Memebase& m):Memebase(m){}
  constexpr Meme() = default;
};

}
#endif
