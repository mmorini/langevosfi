#ifndef LEX_HPP
#define LEX_HPP
class Lexeme: public Lexbase {
public:
  explicit Lexeme(const int&  n):Lexbase(n){}
  Lexeme(const Lexbase& m):Lexbase(m){}
  Lexeme(const Enum& a): Lexbase(a){} 
};
#endif
