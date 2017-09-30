#ifndef NETWORK_HPP
#define NETWORK_HPP

// Example of how to do hypercubic stuff
// Network<Meme<Memebase>> test(Network<Meme<Memebase>>::hypercubic_adjacency(2)); (four memes on a bypercube)

#include "probvector.h++"
#include "myutil.h++"
#include <random>
#include <functional>
#include <utility>
#include <istream>
#include <bitset>
#include <limits>
#include <cassert>

namespace Network {

static const char NETWORK_HPP_SCCS_ID[] __attribute__((used)) = "@(#)network.h++: $Id$";

// Forward declaration to allow friend declaration
template<typename A, typename P> class Network;
template<typename A, typename P> std::istream& operator>> (std::istream&, Network<A,P>&);

template<typename agent,typename probvector=Probvector::Probvector<agent,std::mt19937>>
class Network: public probvector {
  friend std::istream& operator>> <> (std::istream&, Network&);
public:
  using Agent=agent;
  using Probvector=probvector;
  using base_Enumvector=typename Network::Enumvector;
  typedef Enumvector::Enumvector<Agent,probvector> AdjacencyMatrix;
  // static_cast in the following does overload resolution since std::function constructor is templatized and fails to do so.
  // Need the std::function since the map is also templatized
  Network(const AdjacencyMatrix &a): probvector(a.map(std::function<double(const probvector*)>(static_cast<double(probvector::*)(void)const>(&probvector::norm)))), adjacency(a), adjacency_is_diag(false) {}
  Network(AdjacencyMatrix &&a): probvector(a.map(std::function<double(const probvector*)>(static_cast<double(probvector::*)(void)const>(&probvector::norm)))), adjacency(std::forward<decltype(a)>(a)), adjacency_is_diag(false) {}
  Network(const probvector& p, AdjacencyMatrix &&a): probvector(p), adjacency(std::forward<decltype(a)>(a)), adjacency_is_diag(false) {copynorm();}
  Network(probvector&& p, AdjacencyMatrix &&a): probvector(std::forward<decltype(p)>(p)), adjacency(std::forward<decltype(a)>(a)), adjacency_is_diag(false) {copynorm();}
  Network(const probvector& p): Network(p, AdjacencyMatrix(p)) {}
  Network(probvector&& p): Network(std::forward<decltype(p)>(p), AdjacencyMatrix(p)) {}
  Network(const int m=-1): Network(probvector(m)) {}
  Network(typename probvector::Generator &r,const int m=-1): Network(probvector(r,m)) {}
  Network(const base_Enumvector& e): Network(probvector(e,false)) {}
  Network(const base_Enumvector& e, const AdjacencyMatrix &a): Network(probvector(e,false),a) {}
  Network(const base_Enumvector& e, AdjacencyMatrix &&a): Network(probvector(e,false),std::forward<decltype(a)>(a)) {}
  Network(base_Enumvector&& e): Network(probvector(std::forward<decltype(e)>(e),false)) {}
  Network(base_Enumvector&& e, const AdjacencyMatrix &a): Network(probvector(std::forward<decltype(e)>(e),false),a) {}
  Network(base_Enumvector&& e, AdjacencyMatrix &&a): Network(probvector(std::forward<decltype(e)>(e),false),std::forward<decltype(a)>(a)) {}

  Network(const probvector& p, const AdjacencyMatrix &a): probvector(p), adjacency(a), adjacency_is_diag(false) {copynorm();}
  Network(probvector&& p, const AdjacencyMatrix &a): probvector(std::forward<decltype(p)>(p)), adjacency(a), adjacency_is_diag(false) {copynorm();}

  constexpr Network(const Network&) = default;
  constexpr Network(Network&&n): probvector(static_cast<probvector&&>(n)), adjacency(std::move(n.adjacency)), adjacency_is_diag(std::move(n.adjacency_is_diag)) {}

  constexpr const AdjacencyMatrix& getmatrix() const {return adjacency;}
  const AdjacencyMatrix& resetadjacency(const AdjacencyMatrix& a) {
    adjacency = a; copynorm(); return adjacency;
  }
  const AdjacencyMatrix& resetadjacency(AdjacencyMatrix&& a) {
    adjacency = std::forward<decltype(a)>(a); copynorm(); return adjacency;
  }

  static const AdjacencyMatrix hypercubic_adjacency(const int dim=Agent::getn()) {
    AdjacencyMatrix r;
    int i=0;
    for (const auto a: indices(r)) {
      if (i<dim) {
	base_Enumvector e;
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
  static const AdjacencyMatrix bitset_adjacency(const int dim=Agent::getn()) {
    assert(std::numeric_limits<decltype(dim)>::is_specialized);
    assert(std::numeric_limits<decltype(dim)>::radix==2);
    AdjacencyMatrix r;
    int i=0;
    for (const auto a: indices(r)) {
      if (i<dim) {
	base_Enumvector e;
	int j=0;
	for (const auto b: indices(e)) {
	  if (j < dim)
	    e[b] = std::bitset<std::numeric_limits<decltype(dim)>::digits>(i&j++).count()/static_cast<double>(util::count_bits(dim));
	  else
	    e[b] = 0;
	}
	r[a] = e;
      } else {
	r[a] *= 0;
      }
      i++;
    }
    return r;
  }
  static const AdjacencyMatrix diagonal_adjacency(const int dim=Agent::getn()) {
    AdjacencyMatrix r;
    int i=0;
    for (const auto a: indices(r))
      if (i++<dim) {
	base_Enumvector e;
	for (const auto b: indices(r[a]))
	  e[b] = a==b;
	r[a] = e;
      } else
	r[a] *= 0;
    return r;
  }
  static const AdjacencyMatrix basematch_adjacency(const int dim=Agent::getn()) {
    AdjacencyMatrix r;
    int i=0;
    for (const auto a: indices(r))
      if (i++<dim) {
	base_Enumvector e;
	for (const auto b: indices(r[a]))
	  e[b] = a.match(b);
	r[a] = e;
      } else
	r[a] *= 0;
    return r;
  }

  virtual Network& operator=(const base_Enumvector& e) override {
    probvector::operator=(e);
    for (auto &a: adjacency) a = e;
    copynorm();
    return *this;
  }
  virtual Network& operator=(base_Enumvector&& e) override {
    probvector::operator=(std::forward<decltype(e)>(e));
    for (auto &a: adjacency) a = e;
    copynorm();
    return *this;
  }
  virtual Network& operator=(const probvector& e) override{
    probvector::operator=(e);
    for (auto &a: adjacency) a = e;
    copynorm();
    return *this;
  }
  virtual Network& operator=(probvector&& e) override{
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
    return adjacency_is_diag?a:adjacency[a].generate(r);
  }
  virtual double match(const Agent &a, const Agent &b) const {
    // return a.match(b);
    return adjacency[a][b];
  }
  constexpr bool isdiag() const {
    return adjacency_is_diag;
  }
protected:
  void declarediag(bool state=true) {
    adjacency_is_diag = state;
  }
private:
  AdjacencyMatrix adjacency;
  bool adjacency_is_diag;
  void copynorm() {
    for (const auto i: indices(adjacency)) adjacency[i] *= (*const_cast<const Network*>(this))[i]/adjacency[i].norm();
  }
};

}
#endif
