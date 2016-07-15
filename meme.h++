#ifndef MEME_HPP
#define MEME_HPP
class Meme: public Memebase {
public:
  explicit Meme(const int&  n):Memebase(n){}
  Meme(const Memebase& m):Memebase(m){}
  Meme(const Enum& m): Memebase(m){} 
};
#endif
