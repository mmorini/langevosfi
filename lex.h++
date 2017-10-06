#ifndef LEX_HPP
#define LEX_HPP

namespace Lex {

static const char LEX_HPP_SCCS_ID[] __attribute__((used)) = "@(#)lex.h++: $Id$";

template<class Lexbase>
class Lexeme: public Lexbase {
public:
  virtual ~Lexeme() override = default;
  using Lexbase::Lexbase;
  constexpr Lexeme(const Lexbase& m):Lexbase(m){}
};

}
#endif
