#ifndef PROBVECTOR_HPP
#define PROBVECTOR_HPP

#include "enumvector.h++"
#include "myutil.h++"
#include "selfiterator.h++"
#include <utility>
#include <random>
#include <cmath>

// #include <iostream>

static const char PROBVECTOR_HPP_SCCS_ID[] __attribute__((used)) = "@(#)probvector.h++: $Id$";

namespace ProbVector {

template<typename E,typename generator=std::mt19937> class Probvector:
  public EnumVector::Enumvector<E,double> {
public:
  using Generator = generator;
  using Index = E;
  using Enumvector = typename Probvector::Enumvector;
  virtual ~Probvector(void) {}
  Probvector(const int mask=-1, double norm=1.0):
    Enumvector(mask>0?1:0), weight(norm) {
    if(mask>0)
      for(const auto &num: indices(mask,*this))
	(*this)[num] = 0; 
    normalize(false);
  }
  Probvector(const Enumvector& e, bool unnorm=true):
    Enumvector(e){normalize(unnorm);}
  Probvector(Enumvector&& e, bool unnorm=true):
    Enumvector(std::forward<decltype(e)>(e))
  {normalize(unnorm);}
  Probvector(generator &r,const int mask= -1,double norm=1.0):weight(norm) {
    int num = 0;
    for(auto &d: *this) {
      if (mask<=0 || num++ < mask) 
	d = std::generate_canonical<double, 20>(r);
      else d = 0;
    }
    normalize(false);
  }
  constexpr Probvector(const Probvector&) = default;
  constexpr Probvector(Probvector&&p): Enumvector(static_cast<Enumvector&&>(p)),
			     weight(p.weight), setupdone(p.setupdone), s(std::move(p.s)) {}

  Probvector& cshift(int n=1) {
    Enumvector::cshift(n);
    setupdone=false;
    return *this;
  }
  Probvector& shuffle(generator &g) {
    Enumvector::shuffle(g);
    setupdone=false;
    return *this;
  }
  Probvector& permute(const EnumVector::Enumvector<E,E> &p) {
    Enumvector::permute(p);
    setupdone=false;
    return *this;
  }
  virtual Probvector& operator=(const Enumvector& e) override {
    Enumvector::operator=(e);
    normalize();
    setupdone=false;
    return *this;
  }
  virtual Probvector& operator=(Enumvector&& e) override {
    Enumvector::operator=(std::forward<decltype(e)>(e));
    normalize();
    setupdone=false;
    return *this;
  }
  virtual Probvector& operator=(const Probvector&) = default;
  virtual Probvector& operator=(Probvector&&) = default;
  Probvector operator*(const double c) const {
    Probvector r(*this);
    return r *= c;
  }
  virtual Probvector& operator*=(const double c) {
    weight *= c;
    return *this;
  }
  virtual Probvector& operator+=(const Probvector& e) {
    for (const auto i: indices(e))
      (*this)[i] += e.weight/weight * e[i];
    return *this;
  }
  Probvector operator+ (const Probvector& e) const {
    Probvector r(*this);
    return r+=e;
  }


  typename Enumvector::const_reference
  operator[] (const E& e) const {
    return Enumvector::operator[](e);
  }
  constexpr double norm(void) const {
    return weight;
  }
  double& norm(void) {
    return weight;
  }
  //template<typename T> friend auto indices(const T &);
  E generate(generator &r) const
  {
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
  void reinforce(const E&e, double by) {
//   	std::cout << "Probvector::reinforce" << std::endl;
  	(*this)[e] += by * ((*this)[e])*(1.0-(*this)[e]);
  	// If by < 1 we stay in the allowed range; but we'll apply a [0,1] cap
  	if((*this)[e] < 0.0) (*this)[e] = 0.0;
  	else if((*this)[e] > 1.0) (*this)[e] = 1.0;
  	normalize(false);
  	setupdone = false;
  }
  double entropy(bool weighted=false) {
    double retval = 0.0;
    for (auto p: *this)
      if (p>0.0) retval += p*std::log(p);
    if(weighted) retval *= weight;
    return retval;
  }
private:
  double weight = 1.0;
  mutable bool setupdone = false;
  mutable Enumvector s;
  void normalize(bool updateweight=true) {
    auto sum = 0.0;
    for (auto d: *this) sum += d;
    if (sum > 0)
      for (auto &d: *this) d /= sum;
    else
      Enumvector::assign(1.0/Enumvector::numsize());
    if (updateweight) weight=sum;
  }
  void setup() const {
    if (setupdone) return;
    auto cumsum=0.0;
    for(const auto e:indices(s))
      s[e]=cumsum += (*this)[e];
    setupdone = true;
  }
  typename Enumvector::reference
  operator[] (const E& e) {
    return Enumvector::operator[](e);
  }
};

template<typename E, typename generator=std::mt19937>
inline const Probvector<E,generator> unitprob() {
  return Probvector<E,generator>(EnumVector::unitvec<E,double>());
}

}

#endif
