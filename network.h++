#ifndef NETWORK_HPP
#define NETWORK_HPP

// Example of how to do hypercubic stuff
// Network<Meme<Memebase>> test(Network<Meme<Memebase>>::hypercubic_adjacency(2)); (four memes on a bypercube)

#include "probvector.h++"
#include <random>
#include <functional>
#include <utility>

static const char NETWORK_HPP_SCCS_ID[] __attribute__((used)) = "@(#)network.h++: $Id$";

template<typename Agent,typename probvector=Probvector<Agent,std::mt19937>>
class Network: public probvector {
public:
  typedef Enumvector<Agent,probvector> AdjacencyMatrix;
  Network(const int m=-1): probvector(m), adjacency(*static_cast<const probvector*>(this)) {copynorm();}
  Network(typename probvector::Generator &r,const int m=-1): probvector(r,m), adjacency(*static_cast<const probvector*>(this)) {copynorm();}
  // static_cast in the following does overload resolution since std::function constructor is templatized and fails to do so.
  Network(const AdjacencyMatrix &a): probvector(a.map(std::function<double(const probvector*)>(static_cast<double(probvector::*)(void)const>(&probvector::norm)))), adjacency(a) {}
  Network(AdjacencyMatrix &&a): probvector(a.map(std::function<double(const probvector*)>(static_cast<double(probvector::*)(void)const>(&probvector::norm)))), adjacency(std::forward<decltype(a)>(a)) {}
  Network(const probvector& p): probvector(p), adjacency(*static_cast<const probvector*>(this)) {copynorm();}
  Network(probvector&& p): probvector(std::forward<decltype(p)>(p)), adjacency(*static_cast<const probvector*>(this)) {copynorm();}
  Network(const Enumvector<Agent,double>& e): probvector(e,false), adjacency(*static_cast<const probvector*>(this)) {copynorm();}
  Network(Enumvector<Agent,double>&& e): probvector(std::forward<decltype(e)>(e)), adjacency(*static_cast<const probvector*>(this)) {copynorm();}
  Network(const Network&) = default;
  Network(Network&&n): probvector(static_cast<probvector&&>(n)), adjacency(std::move(n.adjacency)) {}

  const AdjacencyMatrix& getmatrix() const {return adjacency;}
  static const AdjacencyMatrix hypercubic_adjacency(const int dim) {
    AdjacencyMatrix r;
    int i=0;
    for (const auto a: indices(r)) {
      if (i<dim) {
	Enumvector<Agent,double> e;
	int j=0;
	for (const auto b: indices(e)) {
	  const int k = i^j;
	  e[b] = j < dim && !k && !(k&(k-1));
	  j++;
	}
	r[a] = e;
      } else {
	r[a] *= 0;
      }
      i++;
    }
    return r;
  }

  virtual Network& operator=(const Enumvector<Agent,double>& e) {
    probvector::operator=(e);
    for (auto &a: adjacency) a = e;
    copynorm();
    return *this;
  }
  virtual Network& operator=(Enumvector<Agent,double>&& e) {
    probvector::operator=(std::forward<decltype(e)>(e));
    for (auto &a: adjacency) a = e;
    copynorm();
    return *this;
  }
  virtual Network& operator=(const probvector& e){
    probvector::operator=(e);
    for (auto &a: adjacency) a = e;
    copynorm();
    return *this;
  }
  virtual Network& operator=(probvector&& e){
    probvector::operator=(std::forward<decltype(e)>(e));
    for (auto &a: adjacency) a = e;
    copynorm();
    return *this;
  }
  virtual Network& operator=(const Network&) = default;
  virtual Network& operator=(Network&& n) {
    probvector::operator=(std::forward<decltype(n)>(n));
    adjacency = std::move(n.adjacency);
    return *this;
  }

  virtual ~Network(void) {}

  virtual Agent neighbor(const Agent& a,typename probvector::Generator &r) const {
    return adjacency[a].generate(r);
  }
  virtual double match(const Agent &a, const Agent &b) const {
    return a.match(b);
  }
private:
  AdjacencyMatrix adjacency;
  void copynorm() {
    for (const auto i: indices(adjacency)) adjacency[i] *= (*const_cast<const Network*>(this))[i]/adjacency[i].norm();
  }
};
#endif
