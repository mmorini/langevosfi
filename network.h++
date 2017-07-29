#ifndef NETWORK_HPP
#define NETWORK_HPP

#include "probvector.h++"
#include <random>

static const char NETWORK_HPP_SCCS_ID[] __attribute__((used)) = "@(#)network.h++: $Id$";

template<typename Agent,typename probvector=Probvector<Agent,std::mt19937>>
class Network: public probvector {
  typedef Enumvector<Agent,probvector> AdjacencyMatrix;
  AdjacencyMatrix adjacency;
  void copynorm() {
    for (const auto i: indices(adjacency)) adjacency[i] = (*const_cast<const Network*>(this))[i];
  }
public:
  Network(const int m=-1): probvector(m), adjacency(*static_cast<const probvector*>(this)) {copynorm();}
  Network(typename probvector::Generator &r,const int m=-1): probvector(r,m), adjacency(*static_cast<const probvector*>(this)) {copynorm();}
  Network(const AdjacencyMatrix &a): probvector(a.map(probvector::norm)), adjacency(a) {}
  Network(AdjacencyMatrix &&a): probvector(a.map(probvector::norm)), adjacency(std::forward<decltype(a)>(a)) {}
  Network(const Enumvector<Agent,double>& e): probvector(e), adjacency(*static_cast<const probvector*>(this)) {copynorm();}
  Network(Enumvector<Agent,double>&& e): probvector(std::forward<decltype(e)>(e)), adjacency(*static_cast<const probvector*>(this)) {copynorm();}
  virtual ~Network(void) {}

  virtual Agent neighbor(const Agent& a,typename probvector::Generator &r) const {
    return adjacency[a].generate(r);
  }
  virtual double match(const Agent &a, const Agent &b) const {
    return a.match(b);
  }
};
#endif
