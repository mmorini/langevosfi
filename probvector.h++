#ifndef PROBVECTOR_HPP
#define PROBVECTOR_HPP

#include "enumvector.h++"
#include "myutil.h++"
#include "selfiterator.h++"
#include <utility>
#include <random>

static const char PROBVECTOR_HPP_SCCS_ID[] __attribute__((used)) = "@(#)probvector.h++: $Id$";

template<typename E,typename generator=std::mt19937> class Probvector:
  public Enumvector<E,double> {
public:
  using Generator = generator;
  using Index = E;
  virtual ~Probvector(void) {}
  Probvector(const int mask=-1, double norm=1.0):
    Enumvector<E,double>(mask>0?1:0), weight(norm) {
    if(mask>0)
      for(const auto &num: indices(mask,*this))
	(*this)[num] = 0; 
    normalize(false);
  }
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
  Probvector& cshift(int n=1) {
    Enumvector<E,double>::cshift(n);
    setupdone=false;
    return *this;
  }
  Probvector& shuffle(generator &g) {
    Enumvector<E,double>::shuffle(g);
    setupdone=false;
    return *this;
  }
  Probvector& permute(const Enumvector<E,E> &p) {
    Enumvector<E,double>::permute(p);
    setupdone=false;
    return *this;
  }
  Probvector& operator=(const Enumvector<E,double>& e) {
    Enumvector<E,double>::operator=(e);
    normalize();
    setupdone=false;
    return *this;
  }
  Probvector& operator=(Enumvector<E,double>&& e) {
    Enumvector<E,double>::operator=(std::forward<decltype(e)>(e));
    normalize();
    setupdone=false;
    return *this;
  }
  Probvector operator*(const double c) const {
    Probvector r(*this);
    return r *= c;
  }
  Probvector& operator*=(const double c) {
    weight *= c;
    return *this;
  }
  Probvector& operator+=(const Probvector& e) {
    for (const auto i: indices(e))
      (*this)[i] += e.weight/weight * e[i];
    return *this;
  }
  Probvector operator+ (const Probvector& e) const {
    Probvector r(*this);
    return r+=e;
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
  //template<typename T> friend auto indices(const T &);
  auto generate(generator &r) const {
    setup();
    const auto ran = std::generate_canonical<double, 20>(r);
    for(auto e: indices(s))
      if(ran <= s[e]) return e;
    return indices(s).back();
  }
  virtual void mutate(const double sigma, generator &r) {
    for (auto &d: *this)
      d = invprobit(probit(d)+BoxMueller(0,sigma,r));
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
  typename Enumvector<E,double>::reference
  operator[] (const E& e) {
    return Enumvector<E,double>::operator[](e);
  }
};

template<typename E, typename generator=std::mt19937>
inline const Probvector<E,generator> unitprob() {
  return Probvector<E,generator>(unitvec<E,double>());
}

#endif
