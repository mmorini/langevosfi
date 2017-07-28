#ifndef IO_HPP
#define IO_HPP
#include <istream>
#include "enumvector.h++"
#include "probvector.h++"
#include "language.h++"

static const char IO_HPP_SCCS_ID[] __attribute__((used)) = "@(#)io.h++: $Id$";

#ifndef ENUMVEC_ITER_CONSTR
// operator<< (std::ostream&, Enumvector&) should probably be 
// defined here for consistency.  It is currently in enumvector.h++
template<typename E, typename T>
auto& operator>>(std::istream& i, Enumvector<E,T>& e) {
  for (auto &v: e) i>>v;
  // we could make it ignore everything till end of line: that will
  // make it consistent with the print out.  Usually it is a bad idea
  // to force newlines. In any case if it is needed:
  // std::string s; getline(i,s); // check s is blank
  return i;
}
#else
template<typename E, typename T>
auto& operator>>(std::istream& i, Enumvector<E,T>& e) {
  e = std::istream_iterator<T>(i)
  // we could make it ignore everything till end of line: that will
  // make it consistent with the print out.  Usually it is a bad idea
  // to force newlines. In any case if it is needed:
  // std::string s; getline(i,s); // check s is blank
  return i;
}
#endif

// operator<< (std::ostream&, Probvector&) should probably be 
// defined here for consistency.  It is currently in probvector.h++
template<typename E, typename G>
auto& operator>>(std::istream& i, Probvector<E,G>& p) {
  const auto w(*std::istream_iterator<double>(i));
  p = *std::istream_iterator<Enumvector<E,double>>(i);
  p *= w;
  return i;
}

// operator<< (std::ostream&, Language&) should probably be 
// defined here for consistency.  It is currently in probvector.h++
template<typename mprobvector, typename lprobvector>
auto& operator>>(std::istream& i, Language<mprobvector,lprobvector>& l) {
  std::string tmp;
  Enumvector<typename mprobvector::Index,lprobvector> e;
  getline(i, tmp); // should check tmp is names of lexes
  for (auto &v: e) {
    i>>tmp>>v; // should check tmp is name of the memes in order
    getline(i, tmp); // should not be necessary, tmp should be blank
  }
  l = std::move(e);
  return i;
}
#ifdef TEST_IO
#include "main.h++"

std::mt19937 r;

constexpr const char memeid [] = "M";
constexpr const char lexid[] = "L";
constexpr const char agentid[] = "A";

template<> int Enum<memeid>::n = 0;
template<> int Enum<lexid>::n = 0;
template<> int Enum<agentid>::n = 0;

class Memebase: public Enum<memeid> {
public:
  virtual ~Memebase(){}
protected:
  explicit Memebase(const int &n): Enum(n) {}
  Memebase(const Enum &n): Enum(n) {}
};
class Memes: public Network<Meme<Memebase>> {
public:
  Memes() {}
  Memes(const Enumvector<Meme<Memebase>,double>& e): Network(e) {}
  Memes(Enumvector<Meme<Memebase>,double>&& e): Network(std::forward<decltype(e)>(e)) {}
  Memes(std::mt19937&r, const int m=-1): Network(r,m){}
};
class Lexbase: public Enum<lexid> {
public:
  virtual ~Lexbase(){}
protected:
  explicit Lexbase(const int &n): Enum(n) {}
  Lexbase(const Enum &n): Enum(n) {}
};
class Lexemes: public Network<Lexeme<Lexbase>> {
public:
  Lexemes(const int m=-1): Network(m) {}
  Lexemes(std::mt19937&r, const int m=-1): Network(r,m){}
  Lexemes(const Probvector<Lexeme<Lexbase>>& p): Network(p){}
  Lexeme<Lexbase> neighbor(const Lexeme<Lexbase>& l, std::mt19937&) const {return l;}
  double match(const Lexeme<Lexbase> &l1, const Lexeme<Lexbase> &l2) const {return l1==l2;}
};
class Agentbase: public Enum<agentid> {
public:
  virtual ~Agentbase(){}
protected:
  explicit Agentbase(const int &n): Enum(n) {}
  Agentbase(const Enum &n): Enum(n) {}
};
class Agents: public Network<Agent<Agentbase>> {
public:
  Agents() {}
  Agents(std::mt19937&r, const int m=-1): Network(r,m){}
};
class AgentLanguage: public Language<Memes,Lexemes> {
public:
  AgentLanguage(){}
  AgentLanguage(const Enumvector<Memes::Index,Lexemes>& e): Language(e) {}
  AgentLanguage(Enumvector<Memes::Index,Lexemes>&& e): Language(std::forward<decltype(e)>(e)) {}
  AgentLanguage(const Memes &m,std::mt19937& r, const int mask=-1):Language(m,r,mask){}
  AgentLanguage(Memes &&m,std::mt19937& r, const int mask=-1):Language(std::forward<decltype(m)>(m),r,mask){}
  AgentLanguage(const Memes &m, const int mask=-1):Language(m,mask){}
  AgentLanguage(Memes &&m, const int mask=-1):Language(std::forward<decltype(m)>(m),mask){}
  AgentLanguage(const Memes &m, const Language &l):Language(m,l){}
  AgentLanguage(const Memes &m, Language &&l):Language(m,std::forward<decltype(l)>(l)){}
  AgentLanguage(Memes &&m, const Language &l):Language(std::forward<decltype(m)>(m),l){}
  AgentLanguage(Memes &&m, Language &&l):Language(std::forward<decltype(m)>(m),std::forward<decltype(l)>(l)){}
  AgentLanguage(const Language& l, const int cshift=0):Language(l, cshift) {}
  AgentLanguage(Language && l, const int cshift=0): Language(std::forward<decltype(l)>(l),cshift) {}
  AgentLanguage(const Language& l, std::mt19937& r): Language(l,r) {}
  AgentLanguage(Language&& l, std::mt19937& r): Language(std::forward<decltype(l)>(l),r) {}
  ~AgentLanguage(){}
};
class Population: public Enumvector<Agent<Agentbase>,AgentLanguage> {
public:
  Population(){}
  Population(const AgentLanguage &l): Enumvector(l) {}
};
int main(void) {
  std::cerr << "Provide nummemes, numlexes, and numagents (e.g., 10 15 40)" << std::endl;
  Meme<Memebase>::setn(*std::istream_iterator<int>(std::cin)); /* 10 */
  Lexeme<Lexbase>::setn(*std::istream_iterator<int>(std::cin)); /* 15 */
  Agent<Agentbase>::setn(*std::istream_iterator<int>(std::cin)); /* 40 */
  std::cerr << "OK, read in sizes" << std::endl;
  std::string str; getline(std::cin, str);
#ifndef ENUMVEC_ITER_CONSTR
  Population population;
  for (auto& p: population) {
       std::cerr << "Waiting for language" << std::endl;
       p=*std::istream_iterator<AgentLanguage>(std::cin);
       std::cerr << "Language read" << std::endl;
  }
#else
  Population population(std::inputiterator<AgentLanguage>(std::cin));
#endif
}
  
#endif  

#endif
