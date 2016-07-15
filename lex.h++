#ifndef LEX_HPP
#define LEX_HPP

static const char LEX_HPP_SCCS_ID[] __attribute__((used)) = "@(#)lex.h++: $Id$";

class Lexeme: public Lexbase {
public:
  explicit Lexeme(const int&  n):Lexbase(n){}
  Lexeme(const Lexbase& m):Lexbase(m){}
  Lexeme(const Enum& a): Lexbase(a){} 
};
#endif
