#ifndef NETWORK_HPP
#define NETWORK_HPP
#include <random>
#include "probvector.h++"
template<typename Agent,typename generator=std::mt19937>
class Network: public Probvector<Agent,generator> {
public:
  Network(const Enumvector<Agent,double>& e):
    Probvector<Agent>(e){}
  Network(Enumvector<Agent,double>&& e):
    Probvector<Agent>(std::forward<decltype(e)>(e)) {}
  Network(generator &r,const int m=-1): Probvector<Agent>(r,m) {}
  virtual ~Network(void) {}

  virtual Agent neighbor(const Agent&,generator &r) const {
    return Probvector<Agent,generator>::generate(r);
  }
  virtual double match(const Agent &a, const Agent &b) const {
    return a.match(b);
  }
};
#endif
