#ifndef LEX_HPP
#define LEX_HPP

static const char LEX_HPP_SCCS_ID[] __attribute__((used)) = "@(#)lex.h++: $Id$";

template<class Lexbase>
class Lexeme: public Lexbase {
public:
  virtual ~Lexeme(){}
  explicit Lexeme(const int&  n):Lexbase(n){}
  Lexeme(const Lexbase& m):Lexbase(m){}
  Lexeme(const typename Lexbase::Enum& a): Lexbase(a){} 
};
#endif
