#ifndef IO_HPP
#define IO_HPP
#include <istream>
#include "enumvector.h++"
#include "probvector.h++"
#include "language.h++"
#include <iterator>

static const char IO_HPP_SCCS_ID[] __attribute__((used)) = "@(#)io.h++: $Id$";

template<typename E, typename T>
auto& operator<< (std::ostream& o, const Enumvector<E,T>& e) {
  std::copy(e.cbegin(), e.cend(), std::ostream_iterator<T>(o,"\t"));
  return o << std::endl;
}

template<typename E, typename T>
auto& operator>>(std::istream& i, Enumvector<E,T>& e) {
  e = Enumvector<E,T>(std::istream_iterator<T>(i));
  // we could make it ignore everything till end of line: that will
  // make it consistent with the print out.  Usually it is a bad idea
  // to force newlines. In any case if it is needed:
  // std::string s; getline(i,s); // check s is blank
  return i;
}

template<typename E, typename G>
auto& operator<< (std::ostream& o, const Probvector<E,G>& e) {
  o << e.norm() << "\t";
  return o << Enumvector<E,double>(e);
}

template<typename E, typename G>
auto& operator>>(std::istream& i, Probvector<E,G>& p) {
  const auto w(*std::istream_iterator<double>(i));
  p = *std::istream_iterator<Enumvector<E,double>>(i);
  p *= w;
  return i;
}

template<typename m, typename l>
auto& operator<< (std::ostream& o, const Language<m,l>& e) {
  constexpr const int newprec = 2;
  auto oldprec = o.precision(newprec);
  o<<"\t\t";
  for (auto a: indices(e.cache)) o<<a<<"\t";
  o<<std::endl;
  for (auto a: indices(e)) o << a << "\t" << e[a];
  o.precision(oldprec);
  return o;
}

template<typename m, typename l>
auto& operator>>(std::istream& i, Language<m,l>& lang) {
  std::string tmp;
  Enumvector<typename m::Index,l> e;
  for (auto j: range(l::Index::getn())) i>>tmp; // should check tmp is names of lexes
  for (auto &v: e) {
    i>>tmp>>v; // should check tmp is name of the memes in order
    getline(i, tmp);
  }
  lang = std::move(e);
  return i;
}
#ifdef TEST_IO

// Compile test_io as 
// g++ -g -DTEST_IO -std=c++14 -x c++ io.h++ -o test_io
//
// It is meant to read in the following file
// 2 3 2
// 		L0	L1	L2
// M0	0.1	0.5	0.3	0.2
// M1	0.9	0.4	0.2	0.4
// 		L0	L1	L2
// M0	0.4	0.3	0.5	0.2
// M1	0.6	0.7	0.2	0.1
// Note the absence of extra new lines etc.  It also ignores the lexeme and meme labels.
// It produces output where the agent languages are separated by an extra tab

#include "main.h++"

int main(void) {

  std::cerr << "Provide nummemes, numlexes, and numagents (e.g., 10 15 40)" << std::endl;
  Meme<Memebase>::setn(*std::istream_iterator<int>(std::cin)); /* 10 */
  Lexeme<Lexbase>::setn(*std::istream_iterator<int>(std::cin)); /* 15 */
  Agent<Agentbase>::setn(*std::istream_iterator<int>(std::cin)); /* 40 */
  std::cout << Population((std::istream_iterator<AgentLanguage>(std::cin))); 
  // Note double parantheses above required by C++ grammar when a constructor
  // with one parameter constructs something.

  return 0;

}
  
#endif  

#endif
