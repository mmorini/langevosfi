#ifndef IO_HPP
#define IO_HPP
#include <iostream>
#include "enumvector.h++"
#include "probvector.h++"
#include "language.h++"
#include "network.h++"
#include "sccs.h++"
#include <iterator>
#include <type_traits>
#include <utility>
#include <algorithm>
#include <string>
#include "myutil.h++"

/*
template<typename E=void>
static std::false_type has_Probvector_base(const volatile void*);

namespace Probvector { // extend

template<typename E, typename G>
static std::true_type has_Probvector_base(const volatile Probvector<E,G>*);

template<typename E, typename G>
static Probvector<E,G> get_Probvector_base(Probvector<E,G>&);

template<typename E, typename G>
static Probvector<E,G> get_Probvector_base(Probvector<E,G>&&);

template<typename E, typename G>
static const Probvector<E,G> get_Probvector_base(const Probvector<E,G>&);

}
*/

namespace IO {

static const SCCS::sccs_id IO_HPP_SCCS_ID __attribute__((used)) = "@(#)io.h++: $Id$";

}

namespace Enumvector { // extend 

template<typename E, typename T, typename std::enable_if<!decltype(util::has_base_template<Probvector::Probvector>(std::declval<T*>()))::value,int>::type =0>
std::ostream& operator<< (std::ostream& o, const Enumvector<E,T>& e) {
  std::copy(e.cbegin(), e.cend(), std::ostream_iterator<T>(o,"\t"));
  return o << std::endl;
}

template<typename E, typename T, typename std::enable_if<!decltype(util::has_base_template<Probvector::Probvector>(std::declval<T*>()))::value,int>::type =0>
std::istream& operator>>(std::istream& i, Enumvector<E,T>& e) {
  e = Enumvector<E,T>(std::istream_iterator<T>(i)); // virtual assignment
  return i;
}

}

namespace Probvector { // extend

template<typename E, typename G>
std::ostream& operator<< (std::ostream& o, const Probvector<E,G>& e) {
  o << e.norm() << "\t";
  return o << static_cast<const typename Probvector<E,G>::Enumvector>(e);
}

template<typename E, typename G>
std::istream& operator>>(std::istream& i, Probvector<E,G>& p) {
  const auto w(*std::istream_iterator<double>(i));
  p = *std::istream_iterator<typename Probvector<E,G>::Enumvector>(i); // virtual assignment
  p *= w;
  return i;
}

template<typename E, typename T, typename std::enable_if<decltype(util::has_base_template<Probvector>(std::declval<T*>()))::value,int>::type =0>
std::ostream& operator<< (std::ostream& o, const Enumvector::Enumvector<E,T>& e) { // partial specialization
  constexpr const int newprec = 2;
  auto oldprec = o.precision(newprec);
  o<<"[matrix]\t\t\t";
  for (const auto &a: indices(e.front())) o<<a<<"\t";
  o<<std::endl;
  // The casts is to make sure that we are talking about the probvector base of T and not for example the whole T
  // which may be a network or some such thing!
  for (const auto& a: indices(e)) o << "[matrix]\t" << a << "\t" << static_cast<typename std::add_lvalue_reference<decltype(util::get_base_template<Probvector>(e[a]))>::type>(e[a]);
  o.precision(oldprec);
  return o;
}

template<typename E, typename T, typename std::enable_if<decltype(util::has_base_template<Probvector>(std::declval<T*>()))::value,int>::type =0>
std::istream& operator>>(std::istream& i, Enumvector::Enumvector<E,T>& ee) { // partial specialization
  // Don't construct directly in ee! Need virtual assignment to avoid slicing.
  typename std::remove_reference<decltype(ee)>::type e; 
  std::string tmp;
  for (auto a __attribute__((unused)): indices(e.front())) {
    i>>tmp; // should check tmp is E of B
  }
  for (auto &vv: e) {
    // Note that T could be things like Lexemes when we are reading in a Language.
    // and Lexemes are a whole network.  But, this is supposed to read only the
    // probabilities, not the network.  So, we need this subterfuge.
    decltype(util::get_base_template<Probvector>(std::declval<T>())) v;
    i>>tmp>>v; // should check tmp is E
    vv = v;
  }
  ee = std::move(e); // virtual assignment
  return i;
}

  /*
template<typename E, typename G>
Probvector<E,G> probbase(const Probvector<E,G>&);
  */

}

namespace Language { // extend

template<typename M, typename L>
std::ostream& operator<< (std::ostream& o, const Language<M,L>& e) { // partial specialization
  return o << static_cast<const typename Language<M,L>::Enumvector&>(e);
}

template<typename M, typename L>
std::istream& operator>> (std::istream& i, Language<M,L>& e) { // partial specialization
  return i >> static_cast<typename Language<M,L>::Enumvector&>(e);
       // relies on virtual =
}

}

namespace Network {

template<typename A, typename P>
std::ostream& operator<< (std::ostream& o, const Network<A,P>& n) {
  return o << n.isdiag() << std::endl << n.getmatrix();
}

template<typename A, typename P>
std::istream& operator>> (std::istream& i, Network<A,P>& n) {
  typename Network<A,P>::AdjacencyMatrix a;
  const bool diagstate = *std::istream_iterator<bool>(i);
  i >> a;
  n = Network<A,P>(std::move(a)); // virtual =
  n.adjacency_is_diag = diagstate;
  return i;
}

}

#ifdef TEST_IO

// Compile test_io as 
// g++ -g -DTEST_IO -std=c++14 -x c++ io.h++ -o test_io
//
/* It is meant to read in the following file
2 3 2
		L0	L1	L2
M0	0.1	0.5	0.3	0.2
M1	0.9	0.4	0.2	0.4
		L0	L1	L2
M0	0.4	0.3	0.5	0.2
M1	0.6	0.7	0.2	0.1
Note the absence of extra new lines etc.  It also ignores the lexeme and meme labels.
It produces output where the agent languages are separated by an extra tab*/

#include "main.h++"

// This is used by Enum template in prints
extern constexpr const char memeid [] = "M";
extern constexpr const char lexid[] = "L";
extern constexpr const char agentid[] = "A";
// Define the variables holding the sizes.
template<> int Enum<memeid>::n = 0;
template<> int Enum<lexid>::n = 0;
template<> int Enum<agentid>::n = 0;

int main(int, char*[]) {

  std::cerr << "Provide nummemes, numlexes, and numagents (e.g., 10 15 40)" << std::endl;
  Meme<Memebase>::setn(*std::istream_iterator<int>(std::cin)); /* 10 */
  Lexeme<Lexbase>::setn(*std::istream_iterator<int>(std::cin)); /* 15 */
  Agent<Agentbase>::setn(*std::istream_iterator<int>(std::cin)); /* 40 */
  std::cout << Population<AgentLanguage>((std::istream_iterator<AgentLanguage>(std::cin))); 
  // Note double parantheses above required by C++ grammar when a constructor
  // with one parameter constructs something.

  return 0;

}
  
#endif  

#endif
