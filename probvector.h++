#ifndef PROBVECTOR_HPP
#define PROBVECTOR_HPP

#include "enumvector.h++"
#include "myutil.h++"
#include <utility>
#include <random>

static const char PROBVECTOR_HPP_SCCS_ID[] __attribute__((used)) = "@(#)probvector.h++: $Id$";

template<typename E,typename generator=std::mt19937> class Probvector:
  Enumvector<E,double> {
public:
  using Generator = generator;
  using Index = E;
  virtual ~Probvector(void) {}
  Probvector(void) {}
  Probvector(const Enumvector<E,double>& e):
    Enumvector<E,double>(e){normalize();}
  Probvector(Enumvector<E,double>&& e):
    Enumvector<E,double>(std::forward<decltype(e)>(e))
  {normalize();}
  Probvector(generator &r,const int mask= -1,double norm=1.0):weight(norm) {
    int num = 0;
    for(auto &d: *this) {
      if (mask<=0 || num++ < mask) 
	d = std::generate_canonical<double, 20>(r);
      else d = 0;
    }
    normalize(false);
  }
  Probvector& operator=(const Enumvector<E,double>& e) {
    Enumvector<E,double>::operator=(e);
    normalize();
    setupdone=false;
    return *this;
  }
  Probvector& operator=(Enumvector<E,double>&& e) {
    Enumvector<E,double>::operator=(std::move(e));
    normalize();
    setupdone=false;
    return *this;
  }
  typename Enumvector<E,double>::const_reference
  operator[] (const E& e) const {
    return Enumvector<E,double>::operator[](e);
  }
  auto& norm(void) const {
    return weight;
  }
  auto& norm(void) {
    return weight;
  }
  template<typename T> friend auto indices(const T &);
  auto generate(generator &r) const {
    setup();
    auto ran = std::generate_canonical<double, 20>(r);
    for(auto e: indices(s))
      if(ran <= s[e]) return e;
    return indices(s).back();
  }
  virtual void mutate(const double sigma, generator &r) {
    std::normal_distribution<double> normal(0,sigma);
    for (auto &d: *this)
      d = invprobit(probit(d)+normal(r));
    normalize(false);
    setupdone = false;
  }
  auto entropy(bool weighted=false) {
    auto retval = 0.0;
    for (auto p: *this)
      if (p>0.0) retval += p*log(p);
    if(weighted) retval *= weight;
    return retval;
  }
  friend inline auto& operator<< (std::ostream& o, const Probvector& e) {
    o << e.weight << "\t";
    return o << Enumvector<E,double>(e);
  }
private:
  double weight = 1.0;
  mutable bool setupdone = false;
  mutable Enumvector<E,double> s;
  void normalize(bool updateweight=true) {
    auto sum = 0.0;
    for (auto d: *this) sum += d;
    if (sum > 0)
      for (auto &d: *this) d /= sum;
    else
      Enumvector<E,double>::assign(1.0/Enumvector<E,double>::numsize());
    if (updateweight) weight=sum;
  }
  void setup() const {
    if (setupdone) return;
    auto cumsum=0.0;
    for(const auto e:indices(s))
      s[e]=cumsum += (*this)[e];
    setupdone = true;
  }
};

#endif
