#ifndef LANGUAGE_HPP
#define LANGUAGE_HPP

#include "enumvector.h++"
#include "probvector.h++"
#include "counts.h++"
#include <utility>
#include <random>
#include <iostream>

static const char LANGUAGE_HPP_SCCS_ID[] __attribute__((used)) = "@(#)language.h++: $Id$";

template<typename mprobvector, typename lprobvector>
class Language: public Enumvector<typename mprobvector::Index,lprobvector> {
 public:
  using Lexeme = typename lprobvector::Index;
  using Meme = typename mprobvector::Index;
  using lgenerator = typename lprobvector::Generator;
  using mgenerator = typename mprobvector::Generator;
  Language(void) {extractmarginal();}
  Language(const Enumvector<Meme,lprobvector>& e):
    Enumvector<Meme,lprobvector>(e)
      {extractmarginal();}
  Language(Enumvector<Meme,lprobvector>&& e):
    Enumvector<Meme,lprobvector>
    (std::forward<decltype(e)>(e))
      {extractmarginal();}
  Language(const mprobvector &marg, mgenerator &r, const int mask=-1):
    marginal(marg)
      {
	for(auto &m: *this)
	  m = lprobvector(r,mask);
	newmarginal();
      }
  Language(mprobvector &&marg, mgenerator &r, const int mask=-1):
    marginal(std::forward<mprobvector>(marg))
      {
	for(auto &m: *this)
	  m = lprobvector(r,mask);
	newmarginal();
      }
  Language(const mprobvector &marg, const int mask=-1):
    marginal(marg)
     {
       if(mask>0)
	 for(auto &m: *this)
	   m = lprobvector(mask);
       newmarginal();
     }
  Language(mprobvector &&marg, const int mask=-1):
    marginal(std::forward<mprobvector>(marg))
     {
       if(mask>0)
	 for(auto &m: *this)
	   m = lprobvector(mask);
       newmarginal();
     }
  Language(const mprobvector &marg, const Language &lang):
    Enumvector<Meme,lprobvector>(lang),
    marginal(marg)
      {newmarginal();}
  Language(const mprobvector &marg, Language &&lang):
    Enumvector<Meme,lprobvector>(std::forward<Language>(lang)),
    marginal(marg)
      {newmarginal();}
  Language(mprobvector &&marg, const Language &lang):
    Enumvector<Meme,lprobvector>(lang),
    marginal(std::forward<mprobvector>(marg))
      {newmarginal();}
  Language(mprobvector &&marg, Language &&lang):
    Enumvector<Meme,lprobvector>(std::forward<Language>(lang)),
    marginal(std::forward<mprobvector>(marg))
      {newmarginal();}
  Language(const Language &lang):
    Enumvector<Meme,lprobvector>(lang),
    marginal(lang.marginal)
      {
	for (auto a: indices(lang.cache))
	  cache[a]=lang.cache[a]?new mprobvector(*lang.cache[a]):0;
      }
  Language(Language &&lang):
    Enumvector<Meme,lprobvector>(std::forward<Language>(lang)),
    marginal(std::forward<decltype(lang.marginal)>(lang.marginal)),
    cache(std::forward<decltype(lang.cache)>(lang.cache))
      {
	lang.initCache();
      }
  Language& operator=(const Language & l) {
    Enumvector<Meme,lprobvector>::operator=(l);
    marginal = l.marginal;
    if (cachedead) {
      cache = std::move(l.cache);
      l.initCache();
    } else
      initCache();
    return *this;
  }
  Language& operator=(Language &&l) {
    Enumvector<Meme,lprobvector>::operator=(std::move(l));
    marginal = std::move(l.marginal);
    cache = std::move(l.cache);
    l.initCache();
    return *this;
  }
  virtual ~Language(void) {deleteCache();}
  virtual Meme memegen(mgenerator &r) const {
    return marginal.generate(r);
  }
  virtual Meme randommeme(mgenerator &r,
			  const Enumvector<Meme,Counts> &counts = Enumvector<Meme,Counts>()) const {
    return memegen(r);
  }
  virtual Lexeme lexgen(const Meme m, lgenerator &r) const {
    return (*this)[m].generate(r);
  }
  virtual Meme memegen(const Lexeme l, mgenerator &r) const {
    return Cachelookup(l).generate(r);
  }
  virtual void mememutate(const double sigma, mgenerator &r) {
    marginal.mutate(sigma,r);
    newmarginal();
  }
  virtual void lexmutate(const double sigma, lgenerator &r,
			 const Enumvector<Meme,Counts> &counts = Enumvector<Meme,Counts>()) {
    // for (auto& m: *this)
    //     m.mutate(sigma,r);
    (*this)[randommeme(r)].mutate(sigma,r);
    deleteCache();
  }
  friend inline auto& operator<< (std::ostream& o, const Language& e) {
    auto oldprec = o.precision(2);
    o<<"\t";
    for (auto a: indices(e.cache)) o<<a<<"\t"; o<<std::endl;
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
	        const Lexeme &l1, lgenerator &r, const Language &) const {
    return lexemes.neighbor(l1,r);
  }

 private:
  mprobvector marginal;
  const mprobvector& getmarginal(void) const {
    return marginal;
  }
  mutable bool cachedead = false;
  mutable Enumvector<Lexeme,mprobvector*> cache = static_cast<decltype(cache)>(0);
  
  void initCache(void) const {
    for (auto& p: cache)
      p = 0;
  }
  auto& Cachelookup(const Lexeme l) const {
    cachedead = false;
    if (!cache[l]) {
      Enumvector<Meme,double> p;
      for (auto m: indices(p))
	p[m] = marginal[m]*(*this)[m][l];
      cache[l] = new mprobvector(std::move(p));
    }
    return *cache[l];
  }
  void deleteCache(void) const {
    for (auto& c:cache) {
      if(c!=0) {
	delete c;
	c = 0;
      }
    }
  }
  void newmarginal() {
    for(auto m: indices(*this))
      (*this)[m].norm() = static_cast<const decltype(marginal)>(marginal)[m];
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
