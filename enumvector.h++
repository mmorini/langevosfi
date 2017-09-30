#ifndef ENUMVECTOR_HPP
#define ENUMVECTOR_HPP

#include <vector>
#include <utility>
#include <algorithm>
#include <iterator>
#include "selfiterator.h++"
#include "myutil.h++"

static const char ENUMVECTOR_HPP_SCCS_ID[] __attribute__((used)) = "@(#)enumvector.h++: $Id$";

namespace Enumvector {
template<typename E,typename T> class Enumvector: std::vector<T> {
private:
  template<typename D, typename P, typename R>
  static constexpr bool checktype(const std::iterator<std::input_iterator_tag,T,D,P,R> &i);
public:
  using base_vector = typename Enumvector::vector;
  using typename base_vector::value_type;
  using typename base_vector::allocator_type;
  using typename base_vector::reference;
  using typename base_vector::const_reference;
  using typename base_vector::pointer;
  using typename base_vector::const_pointer;
  using typename base_vector::iterator;
  using typename base_vector::const_iterator;
  using typename base_vector::reverse_iterator;
  using typename base_vector::const_reverse_iterator;
  using typename base_vector::difference_type;
  typedef E size_type;
  using base_vector::begin;
  using base_vector::end;
  using base_vector::rbegin;
  using base_vector::rend;
  using base_vector::cbegin;
  using base_vector::cend;
  using base_vector::crbegin;
  using base_vector::crend;
  using base_vector::front;
  using base_vector::back;
  using base_vector::data;
  using base_vector::swap;
  using base_vector::get_allocator;
  E size(void) const {return static_cast<E>(E::getn());}
  int numsize(void) const {return E::getn();}
  explicit Enumvector(/*const allocator_type& alloc=allocator_type()*/):
    base_vector(E::getn()/*,alloc*/){}
  explicit Enumvector(const value_type& val,
		  const allocator_type& alloc=allocator_type()):
    base_vector(E::getn(),val,alloc){}
  Enumvector(const Enumvector& x): base_vector(x) {}
  Enumvector(const Enumvector& x, const allocator_type& alloc):
    base_vector(x,alloc) {}
  Enumvector(Enumvector&& x):
    base_vector(std::forward<Enumvector>(x)) {}
  Enumvector(Enumvector&& x, const allocator_type& alloc):
    base_vector(std::forward<Enumvector(x)>(x),alloc) {}
  template<typename input_iterator>
  Enumvector(input_iterator i, decltype(checktype(i)) check=true):
    base_vector(E::getn()) {
    bool first = true;
    for(auto &e: static_cast<base_vector&>(*this)) {
      e = *(first?((first=false),i):++i); // Can't do *i++ which may set failbit at end
    }
  }
#define ENUMVEC_HAS_ITER_CONSTR
  Enumvector& cshift(int n=1) {
    size_t s = E::getn();
    if (s>0) {
      const Enumvector tmp(*this);
      n %= s;
      for (size_t i=0; i<s; i++)
	(*this)[static_cast<E>(static_cast<int>(i))] = std::move(tmp[static_cast<E>(static_cast<int>((i+n)%s))]);
    }
    return *this;
  }
  template<typename generator>
  Enumvector& shuffle(generator &r) {
    util::myshuffle(begin(), end(), r);
    return *this;
  }
  Enumvector& permute(const Enumvector<E,E> &reorder) {
    const Enumvector tmp(*this);
    for (const auto i: indices(reorder))
      (*this)[i] = std::move(tmp[reorder[i]]);
    return *this;
  }
  reference operator[](const E& m){return base_vector::operator[](static_cast<int>(m));}
  const_reference operator[](const E& m) const{
    return base_vector::operator[](static_cast<int>(m));
  }
  virtual Enumvector& operator= (const Enumvector& x) {
    base_vector::operator=(x); return *this;
  }
  virtual Enumvector& operator= (Enumvector&& x) {
    base_vector::operator=(std::forward<decltype(x)>(x)); return *this;
  }
  reference at(const E& m){return base_vector::at(m);}
  const_reference at(const E& m) const{return base_vector::at(m);}
  void assign(const value_type& val){base_vector::assign(E::getn(),val);}
  template<typename T2>
  Enumvector<E,typename T2::result_type> map(const T2 &f) const {
    Enumvector<E,typename T2::result_type> r;
    std::transform(cbegin(), cend(), r.begin(),
		   [f](typename std::remove_pointer<typename T2::argument_type>::type &x)
		   {return f(&x);});
    return r;
  }
};

template<typename E, typename T>
inline const Enumvector<E,T> unitvec() {
  Enumvector<E,T> res;
  res[static_cast<E>(0)] = 1;
  return res;
}

  template<typename E, typename T>
  inline
  constexpr auto indices(const Enumvector<E,T> &o)
    ->decltype(SelfIterator::indices(o)) {
    return SelfIterator::indices(o);
  }

  template<typename E, typename T>
  inline
  constexpr auto indices(const size_t s, const Enumvector<E,T> &o)
    ->decltype(SelfIterator::indices(s,o)) {
    return SelfIterator::indices(s,o);
  }

}
#endif
