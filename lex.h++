#ifndef LEX_HPP
#define LEX_HPP

#include "sccs.h++"

namespace Lex {

static const SCCS::sccs_id LEX_HPP_SCCS_ID __attribute__((used)) = "@(#)lex.h++: $Id$";

template<class Lexbase>
class Lexeme: public Lexbase {
public:
  virtual ~Lexeme() override = default;
  using Lexbase::Lexbase;
  constexpr Lexeme(const Lexbase& m):Lexbase(m){}
};

}
#endif
