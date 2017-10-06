#ifndef LANGUAGE_HPP
#define LANGUAGE_HPP

#include "enumvector.h++"
#include "probvector.h++"
#include "experience.h++"
#include <utility>
#include <memory>

namespace Language {

static const char LANGUAGE_HPP_SCCS_ID[] __attribute__((used)) = "@(#)language.h++: $Id$";

template<typename mprobvector, typename lprobvector>
class Language: public Enumvector::Enumvector<typename mprobvector::Index,typename lprobvector::Probvector> {
 public:
  using Lexeme = typename lprobvector::Index;
  using Meme = typename mprobvector::Index;
  using lgenerator = typename lprobvector::Generator;
  using mgenerator = typename mprobvector::Generator;
  using Mprobvector = typename mprobvector::Probvector;
  using Lprobvector = typename lprobvector::Probvector;
  using base_Enumvector = typename Language::Enumvector;
  Language(void) {extractmarginal();}
  Language(const base_Enumvector& e):
    base_Enumvector(e)
      {extractmarginal();}
  Language(base_Enumvector&& e):
    base_Enumvector
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
    base_Enumvector(lang),
    marginal(marg)
      {newmarginal();}
  Language(const Mprobvector &marg, Language &&lang):
    base_Enumvector(std::forward<Language>(lang)),
    marginal(marg)
      {newmarginal();}
  Language(Mprobvector &&marg, const Language &lang):
    base_Enumvector(lang),
    marginal(std::forward<Mprobvector>(marg))
      {newmarginal();}
  Language(Mprobvector &&marg, Language &&lang):
    base_Enumvector(std::forward<Language>(lang)),
    marginal(std::forward<Mprobvector>(marg))
      {newmarginal();}
  Language(const Language &lang, const int shift=0):
    base_Enumvector(lang),
    marginal(lang.marginal)
      {
	for (auto a: indices(lang.cache))
	  cache[a]=lang.cache[a]?std::make_shared<Mprobvector>(*lang.cache[a]):nullptr;
	cshift(shift);
      }
  Language(const Language &lang, lgenerator &g):
    base_Enumvector(lang),
    marginal(lang.marginal)
      {
	permute(g);
      }
  Language(Language &&lang, const int shift=0):
    base_Enumvector(std::forward<Language>(lang)),
    marginal(std::forward<decltype(lang.marginal)>(lang.marginal)),
    cache(std::forward<decltype(lang.cache)>(lang.cache))
      {
	cshift(shift);
	lang.initCache();
      }
  Language(Language &&lang, lgenerator &g):
    base_Enumvector(std::forward<Language>(lang)),
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
    Enumvector::Enumvector<Lexeme,Lexeme> p(static_cast<Lexeme>(0));
	for (auto a: indices(p)) p[a] = a;
	p.shuffle(g);
	for (auto &a: *this)
	    a.permute(p);
	cache.permute(p);
	return *this;
      }
  virtual Language& operator=(const Language & l) {
    base_Enumvector::operator=(l);
    marginal = l.marginal;
    deleteCache();
    if (l.cachedead) {
      cache = std::move(l.cache);
      l.initCache();
    } 
    return *this;
  }
  virtual Language& operator=(Language &&l) {
    base_Enumvector::operator=(std::forward<decltype(l)>(l));
    marginal = std::move(l.marginal);
    // deleteCache();
    cache = std::move(l.cache);
    l.initCache();
    return *this;
  }
  virtual Language& operator=(base_Enumvector&& e) override {
    base_Enumvector::operator=(std::forward<decltype(e)>(e));
    extractmarginal();
    return *this;
  }
  virtual Language& operator=(const base_Enumvector& e) override {
    base_Enumvector::operator=(e);
    extractmarginal();
    return *this;
  }
  // virtual ~Language(void) override = default; // {/*deleteCache();*/}
  virtual Meme memegen(mgenerator &r) const {
    return marginal.generate(r);
  }
  virtual Meme randommeme(mgenerator &r,
			  const Experience::Experience<Meme,Lexeme> &experiences = Experience::Experience<Meme,Lexeme>()) const {
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
			 const Experience::Experience<Meme,Lexeme> &experience = Experience::Experience<Meme,Lexeme>()) {
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
  // So that cache can be copied, we need shared_ptr, not unique_ptr
  mutable Enumvector::Enumvector<Lexeme,std::shared_ptr<Mprobvector>> cache;
  
  void initCache(void) const {
    for (auto& p: cache)
      p = nullptr;
    // Need to change deleteCache if initCache does anything else
    // initCache and deleteCache do the same thing in this implementation
  }
  Mprobvector& Cachelookup(const Lexeme l) const {
    cachedead = false;
    if (!cache[l]) {
      Enumvector::Enumvector<Meme,double> p;
      for (auto m: indices(p))
	p[m] = marginal[m]*(*this)[m][l];
      cache[l] = std::make_shared<Mprobvector>(std::move(p));
    }
    return *cache[l];
  }
protected: // Expose this to subclasses to use in lexmutate
  void deleteCache(void) const {
    for (auto& c:cache)
      c = nullptr;
    // Must call initCache if that is nontrivial
    // initCache and deleteCache do the same thing in this implementation
  }
private:
  void newmarginal() {
    for(auto m: indices(*this))
      (*this)[m].norm() = static_cast<const decltype(marginal)>(marginal)[m];
    deleteCache();
  }
  void extractmarginal() {
    typename Mprobvector::Enumvector temp;
    for (auto m: indices(*this))
      temp[m] = (*this)[m].norm();
    marginal = std::move(temp);
    marginal.norm() = 1;
    newmarginal();
  }
};

template<typename mprobvector, typename lprobvector>
inline const Language<mprobvector,lprobvector> unitlang(Language<mprobvector,lprobvector>*r=nullptr) {
  Language<mprobvector,lprobvector> retval;
  using Lprobvector=typename lprobvector::Probvector;
  Lprobvector lprob(Probvector::unitprob<typename lprobvector::Index, typename lprobvector::Generator>());
  lprob += Lprobvector()*(1.0/retval.numsize()/retval.numsize());
  for (auto i: indices(retval)) {
    retval[i] = lprob;
    lprob.cshift();
  }
  retval.decache();
  return r?*r = std::move(retval):retval;
}

}

#endif
