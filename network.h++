#ifndef NETWORK_HPP
#define NETWORK_HPP

#include "probvector.h++"
#include <random>

static const char NETWORK_HPP_SCCS_ID[] __attribute__((used)) = "@(#)network.h++: $Id$";

template<typename Agent,typename probvector=Probvector<Agent,std::mt19937>>
class Network: public probvector {
public:
  Network(const int m=-1): probvector(m) {}
  Network(const Enumvector<Agent,double>& e): probvector(e){}
  Network(Enumvector<Agent,double>&& e): probvector(std::forward<decltype(e)>(e)) {}
  Network(typename probvector::Generator &r,const int m=-1): probvector(r,m) {}
  virtual ~Network(void) {}

  virtual Agent neighbor(const Agent&,typename probvector::Generator &r) const {
    return probvector::generate(r);
  }
  virtual double match(const Agent &a, const Agent &b) const {
    return a.match(b);
  }
};
#endif
