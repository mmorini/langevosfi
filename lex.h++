#ifndef LEX_HPP
#define LEX_HPP

static const char LEX_HPP_SCCS_ID[] __attribute__((used)) = "@(#)lex.h++: $Id$";

namespace Lex {

template<class Lexbase>
class Lexeme: public Lexbase {
public:
  virtual ~Lexeme(){}
  explicit constexpr Lexeme(const int&  n):Lexbase(n){}
  constexpr Lexeme(const Lexbase& m):Lexbase(m){}
  constexpr Lexeme(const typename Lexbase::Enum& a): Lexbase(a){} 
};

}
#endif
