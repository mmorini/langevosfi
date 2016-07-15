#ifndef LANGUAGE_HPP
#define LANGUAGE_HPP
#include <utility>
#include <random>
#include <iostream>
#include "enumvector.h++"
#include "probvector.h++"
template<typename Meme, typename Lexeme, typename generator=std::mt19937>
class Language: public Enumvector<Meme,Probvector<Lexeme,generator>> {
 public:
  Language(void) {}
  Language(const Enumvector<Meme,Probvector<Lexeme,generator>>& e):
    Enumvector<Meme,Probvector<Lexeme,generator>>(e)
      {extractmarginal();}
  Language(Enumvector<Meme,Probvector<Lexeme,generator>>&& e):
    Enumvector<Meme,Probvector<Lexeme,generator>>
    (std::forward<decltype(e)>(e))
      {extractmarginal();}
  template<typename g>
    Language(const Probvector<Meme,g> &marg, generator &r, const int mask=-1):
    marginal(marg)
      {
	for(auto &m: *this)
	  m = Probvector<Lexeme,generator>(r,mask);
	newmarginal();
      }
  template<typename g>
    Language(const Probvector<Meme,g> &&marg, generator &r, const int mask=-1):
    marginal(std::forward<decltype(marg)>(marg))
      {
	for(auto &m: *this)
	  m = Probvector<Lexeme,generator>(r,mask);
	newmarginal();
      }
  template<typename g>
    Language(const Probvector<Meme,g> &marg, const Language &lang):
    Enumvector<Meme,Probvector<Lexeme,generator>>(lang),
    marginal(marg)
      {newmarginal();}
  template<typename g>
    Language(const Probvector<Meme,g> &marg, const Language &&lang):
    Enumvector<Meme,Probvector<Lexeme,generator>>(std::forward<decltype(lang)>(lang)),
    marginal(marg)
      {newmarginal();}
  template<typename g>
    Language(const Probvector<Meme,g> &&marg, const Language &lang):
    Enumvector<Meme,Probvector<Lexeme,generator>>(lang),
    marginal(std::forward<decltype(marg)>(marg))
      {newmarginal();}
  template<typename g>
    Language(const Probvector<Meme,g> &&marg, const Language &&lang):
    Enumvector<Meme,Probvector<Lexeme,generator>>(std::forward<decltype(lang)>(lang)),
    marginal(std::forward<decltype(marg)>(marg))
      {newmarginal();}
  Language(const Language &lang):
    Enumvector<Meme,Probvector<Lexeme,generator>>(lang),
    marginal(lang.marginal)
      {
	for (auto a: indices(lang.Cache))
	  Cache[a]=lang.Cache[a]?new Probvector<Meme,generator>(*lang.Cache[a]):0;
      }
  Language(Language &&lang):
    Enumvector<Meme,Probvector<Lexeme,generator>>(std::forward<decltype(lang)>(lang)),
    marginal(std::forward<decltype(lang.marginal)>(lang.marginal)),
    Cache(std::forward<decltype(lang.Cache)>(lang.Cache))
      {
	lang.initCache();
      }
  Language& operator=(const Language & l) {
    Enumvector<Meme,Probvector<Lexeme,generator>>::operator=(l);
    marginal = l.marginal;
    if (cachedead) {
      Cache = std::move(l.Cache);
      l.initCache();
    } else
      initCache();
    return *this;
  }
  Language& operator=(Language &&l) {
    Enumvector<Meme,Probvector<Lexeme,generator>>::operator=(std::move(l));
    marginal = std::move(l.marginal);
    Cache = std::move(l.Cache);
    l.initCache();
    return *this;
  }
  virtual ~Language(void) {deleteCache();}
  virtual Meme memegen(generator &r) const {
    return marginal.generate(r);
  }
  virtual Lexeme lexgen(const Meme m, generator &r) const {
    return (*this)[m].generate(r);
  }
  virtual Meme memegen(const Lexeme l, generator &r) const {
    return Cachelookup(l).generate(r);
  }
  virtual void mememutate(const double sigma, generator &r) {
    marginal.mutate(sigma,r);
    newmarginal();
  }
  virtual void lexmutate(const double sigma, generator &r) {
    for (auto& m: *this)
      m.mutate(sigma,r);
    deleteCache();
  }
  friend inline auto& operator<< (std::ostream& o, const Language& e) {
    auto oldprec = o.precision(2);
    o<<"\t";
    for (auto a: indices(e.Cache)) o<<a<<"\t"; o<<std::endl;
    for (auto a: indices(e)) o << a << "\t" << e[a];
    o.precision(oldprec);
    return o;
  }
  virtual const Language& decache(void) const {
    cachedead = true;
    return *this;
  }
  virtual Language& decache(void) {
    cachedead = true;
    return *this;
  }
  template<typename Network>
  auto match(const Network &memes,
	     const Meme &m1, const Meme &m2, const Language &) const {
    return memes.match(m1,m2);
  }
  template<typename Network>
  auto transmit(const Network &lexemes,
	        const Lexeme &l1, generator &r, const Language &) const {
    return lexemes.neighbor(l1,r);
  }

 private:
  Probvector<Meme,generator> marginal;
  const Probvector<Meme,generator>& getmarginal(void) const {
    return marginal;
  }
  mutable bool cachedead = false;
  mutable Enumvector<Lexeme,Probvector<Meme,generator>*> Cache =
    Enumvector<Lexeme,Probvector<Meme,generator>*>(0);
  void initCache(void) {
    for (auto& p: Cache)
      p = 0;
  }
  auto& Cachelookup(const Lexeme l) const {
    cachedead = false;
    if (!Cache[l]) {
      Enumvector<Meme,double> p;
      for (auto m: indices(p))
	p[m] = marginal[m]*(*this)[m][l];
      Cache[l] = new Probvector<Meme,generator>(std::move(p));
    }
    return *Cache[l];
  }
  void deleteCache(void) const {
    for (auto& c:Cache) {
      if(c!=0) {
	delete c;
	c = 0;
      }
    }
  }
  void newmarginal() {
    for(auto m: indices(*this))
      (*this)[m].norm() = marginal[m];
    deleteCache();
  }
  void extractmarginal() {
    Enumvector<Meme,double> temp;
    for (auto m: indices(*this))
      temp[m] = (*this)[m].norm();
    marginal = std::move(temp);
    marginal.norm() = 1;
    newmarginal();
  }
};

#endif
