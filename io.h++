#ifndef IO_HPP
#define IO_HPP
#include <istream>
#include "enumvector.h++"
#include "probvector.h++"
#include "language.h++"

static const char IO_HPP_SCCS_ID[] __attribute__((used)) = "@(#)io.h++: $Id$";

#ifndef ENUMVEC_HAS_ITER_CONSTR
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
  e = Enumvector<E,T>(std::istream_iterator<T>(i));
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
    i>>tmp; // should check tmp is name of the memes in order
    i>>v;
    getline(i, tmp);
  }
  l = std::move(e);
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
  std::string str; getline(std::cin, str); // important that langugae read starts at the begining of a line
#ifndef ENUMVEC_HAS_ITER_CONSTR
  Population population;
  for (auto& p: population) {
       p=*std::istream_iterator<AgentLanguage>(std::cin);
  }
  std::cout << population;
#else
  std::cout << Population((std::istream_iterator<AgentLanguage>(std::cin))); // Note double parantheses required by C++ grammar when a constructor with one parameter constructs something.
#endif
}
  
#endif  

#endif
