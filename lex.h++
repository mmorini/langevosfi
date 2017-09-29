#ifndef LEX_HPP
#define LEX_HPP

static const char LEX_HPP_SCCS_ID[] __attribute__((used)) = "@(#)lex.h++: $Id$";

namespace Lex {

template<class Lexbase>
class Lexeme: public Lexbase {
public:
  virtual ~Lexeme() = default;
  using Lexbase::Lexbase;
  constexpr Lexeme(const Lexbase& m):Lexbase(m){}
};

}
#endif
