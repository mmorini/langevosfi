#ifndef LANGUAGE_HPP
#define LANGUAGE_HPP

#include "enumvector.h++"
#include "probvector.h++"
#include "experience.h++"
#include <utility>

static const char LANGUAGE_HPP_SCCS_ID[] __attribute__((used)) = "@(#)language.h++: $Id$";

template<typename mprobvector, typename lprobvector>
class Language: public Enumvector<typename mprobvector::Index,typename lprobvector::Probvector> {
 public:
  using Lexeme = typename lprobvector::Index;
  using Meme = typename mprobvector::Index;
  using lgenerator = typename lprobvector::Generator;
  using mgenerator = typename mprobvector::Generator;
  using Mprobvector = typename mprobvector::Probvector;
  using Lprobvector = typename lprobvector::Probvector;
  Language(void) {extractmarginal();}
  Language(const Enumvector<Meme,Lprobvector>& e):
    Enumvector<Meme,Lprobvector>(e)
      {extractmarginal();}
  Language(Enumvector<Meme,Lprobvector>&& e):
    Enumvector<Meme,Lprobvector>
    (std::forward<decltype(e)>(e))
      {extractmarginal();}
  Language(const Mprobvector &marg, mgenerator &r, const int mask=-1):
    marginal(marg)
      {
	for(auto &m: *this)
	  m = Lprobvector(r,mask);
	newmarginal();
      }
  Language(Mprobvector &&marg, mgenerator &r, const int mask=-1):
    marginal(std::forward<Mprobvector>(marg))
      {
	for(auto &m: *this)
	  m = Lprobvector(r,mask);
	newmarginal();
      }
  Language(const Mprobvector &marg, const int mask=-1):
    marginal(marg)
     {
       if(mask>0)
	 for(auto &m: *this)
	   m = Lprobvector(mask);
       newmarginal();
     }
  Language(Mprobvector &&marg, const int mask=-1):
    marginal(std::forward<Mprobvector>(marg))
     {
       if(mask>0)
	 for(auto &m: *this)
	   m = Lprobvector(mask);
       newmarginal();
     }
  Language(const Mprobvector &marg, const Language &lang):
    Enumvector<Meme,Lprobvector>(lang),
    marginal(marg)
      {newmarginal();}
  Language(const Mprobvector &marg, Language &&lang):
    Enumvector<Meme,Lprobvector>(std::forward<Language>(lang)),
    marginal(marg)
      {newmarginal();}
  Language(Mprobvector &&marg, const Language &lang):
    Enumvector<Meme,Lprobvector>(lang),
    marginal(std::forward<Mprobvector>(marg))
      {newmarginal();}
  Language(Mprobvector &&marg, Language &&lang):
    Enumvector<Meme,Lprobvector>(std::forward<Language>(lang)),
    marginal(std::forward<Mprobvector>(marg))
      {newmarginal();}
  Language(const Language &lang, const int shift=0):
    Enumvector<Meme,Lprobvector>(lang),
    marginal(lang.marginal)
      {
	for (auto a: indices(lang.cache))
	  cache[a]=lang.cache[a]?new Mprobvector(*lang.cache[a]):0;
	cshift(shift);
      }
  Language(const Language &lang, lgenerator &g):
    Enumvector<Meme,Lprobvector>(lang),
    marginal(lang.marginal)
      {
	permute(g);
      }
  Language(Language &&lang, const int shift=0):
    Enumvector<Meme,Lprobvector>(std::forward<Language>(lang)),
    marginal(std::forward<decltype(lang.marginal)>(lang.marginal)),
    cache(std::forward<decltype(lang.cache)>(lang.cache))
      {
	cshift(shift);
	lang.initCache();
      }
  Language(Language &&lang, lgenerator &g):
    Enumvector<Meme,Lprobvector>(std::forward<Language>(lang)),
    marginal(std::forward<decltype(lang.marginal)>(lang.marginal)),
    cache(std::forward<decltype(lang.cache)>(lang.cache))
      {
	permute(g);
	lang.initCache();
      }
  Language& cshift(const int shift=1) {
	if (shift != 0) {
	  for (auto &a: *this)
	    a.cshift(shift);
	  cache.cshift(shift);
	}
	return *this;
      }
  Language& permute(lgenerator &g) {
    Enumvector<Lexeme,Lexeme> p(static_cast<Lexeme>(0));
	for (auto a: indices(p)) p[a] = a;
	p.shuffle(g);
	for (auto &a: *this)
	    a.permute(p);
	cache.permute(p);
	return *this;
      }
  virtual Language& operator=(const Language & l) {
    Enumvector<Meme,Lprobvector>::operator=(l);
    marginal = l.marginal;
    if (l.cachedead) {
      cache = std::move(l.cache);
      l.initCache();
    } else
      initCache();
    return *this;
  }
  virtual Language& operator=(Language &&l) {
    Enumvector<Meme,Lprobvector>::operator=(std::forward<decltype(l)>(l));
    marginal = std::move(l.marginal);
    cache = std::move(l.cache);
    l.initCache();
    return *this;
  }
  virtual Language& operator=(Enumvector<Meme,Lprobvector>&& e) override {
    Enumvector<Meme,Lprobvector>::operator=(std::forward<decltype(e)>(e));
    extractmarginal();
    return *this;
  }
  virtual Language& operator=(const Enumvector<Meme,Lprobvector>& e) override {
    Enumvector<Meme,Lprobvector>::operator=(e);
    extractmarginal();
    return *this;
  }
  virtual ~Language(void) {deleteCache();}
  virtual Meme memegen(mgenerator &r) const {
    return marginal.generate(r);
  }
  virtual Meme randommeme(mgenerator &r,
			  const Experience<Meme,Lexeme> &experiences = Experience<Meme,Lexeme>()) const {
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
			 const Experience<Meme,Lexeme> &experience = Experience<Meme,Lexeme>()) {
    // for (auto& m: *this)
    //     m.mutate(sigma,r);
    (*this)[randommeme(r)].mutate(sigma,r);
    deleteCache();
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
  double match(const Network &memes,
	     const Meme &m1, const Meme &m2, const Language &) const {
    return memes.match(m1,m2);
  }
  template<typename Network>
  auto transmit(const Network &lexemes,
	        const Lexeme &l1, lgenerator &r, const Language &) const
    -> typename std::remove_reference<decltype(lexemes.neighbor(l1,r))>::type {
    return lexemes.neighbor(l1,r);
  }

 private:
  Mprobvector marginal;
  constexpr const Mprobvector& getmarginal(void) const {
    return marginal;
  }
  mutable bool cachedead = false;
  mutable Enumvector<Lexeme,Mprobvector*> cache = static_cast<decltype(cache)>(nullptr);
  
  void initCache(void) const {
    for (auto& p: cache)
      p = 0;
  }
  Mprobvector& Cachelookup(const Lexeme l) const {
    cachedead = false;
    if (!cache[l]) {
      Enumvector<Meme,double> p;
      for (auto m: indices(p))
	p[m] = marginal[m]*(*this)[m][l];
      cache[l] = new Mprobvector(std::move(p));
    }
    return *cache[l];
  }
protected: // Expose this to subclasses to use in lexmutate
  void deleteCache(void) const {
    for (auto& c:cache) {
      if(c!=0) {
	delete c;
	c = 0;
      }
    }
  }
private:
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

template<typename mprobvector, typename lprobvector>
inline const Language<mprobvector,lprobvector> unitlang(Language<mprobvector,lprobvector>*r=0) {
  Language<mprobvector,lprobvector> retval;
  using Lprobvector=typename lprobvector::Probvector;
  Lprobvector lprob(unitprob<typename lprobvector::Index, typename lprobvector::Generator>());
  lprob += Lprobvector()*(1.0/retval.numsize()/retval.numsize());
  for (auto i: indices(retval)) {
    retval[i] = lprob;
    lprob.cshift();
  }
  retval.decache();
  return r?*r = std::move(retval):retval;
}

#endif
