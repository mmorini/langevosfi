#ifndef MAIN_DECLS_HPP
#define MAIN_DECLS_HPP

#include "main.h++"
#include <random>
#include <utility>
#include <ostream>
#include <iterator>

// Language is the heart of the code. It defines a number of virtual
// functions that can be overriden:
//
//       virtual Meme memegen(mgenerator &r) const
//                     --- return a random meme
//       virtual Lexeme lexgen(const Meme m, lgenerator &r) const
//                     --- given a meme, generate a lex
//       virtual Meme memegen(const Lexeme l, mgenerator &r) const
//                     --- given a lex, generate a meme
//       virtual void mememutate(const double sigma, mgenerator &r)
//                     --- Mutate the probability of memes
//       virtual void lexmutate(const double sigma, lgenerator &r,
//                              const Enumvector<Meme,Counts> &counts)
//                     --- Mutate the language holding the marginals constant
//                     --- (i.e., the probability of the memes)
//                     --- If the last parameter is supplied, can
//                     --- use it to choose meme whose lex distribution
//                     --- to mutate. Currently ignored.
//       virtual Language& decache(void)
//                     --- Declare that the cache can be stolen if the
//                     --- language is copied. (Returns the Language itself).
//                     --- Can be applied to const or nonconst
//
// Enumvector<A,B> is like Vector<B>, but indexed by A.
//
// Counts is a misnomer: it actually keeps track of the average
// overlap between the meme communicated and the meme interpreted.  It
// is a count only when the overlap is a 0/1 variable.
// selfiterator uses C++14 magic to be able to iterate over indices
// of a vector, ranges of integers, etc.

static const char MAIN_DECLS_HPP_SCCS_ID[] __attribute__((used)) = "@(#)main_decls.h++: $Id$";

// Everything below uses r as the random number generator.  If we
// change the type of the generator, then Language and Network (and
// Probvector if you use it directly) need to be provided that type as
// an additional parameter.
std::mt19937 r;

// This is used by Enum template in prints
extern const char memeid [];
extern const char lexid[];
extern const char agentid[];

// meme.h++ extends Memebase to define Meme.  So, define this class
// anyhow.  The rest of the program assumes that it publicly derives
// from an instantiation of the Enum template, and the functions in
// the Enum template have not been overriden.  Meme currently is an
// empty extension.
class Memebase: public Enum<memeid> {
public:
  virtual ~Memebase(){}
protected:
  explicit Memebase(const int &n): Enum(n) {}
  Memebase(const Enum &n): Enum(n) {}
};
// Memes must inherit publicly from Network<Meme> but can provide
// implementation for two virtual functions:
//
//        virtual Meme neighbor(const Meme&, generator &r) const;
//
// that uses the random number generator r to return a neighboring
// Meme and
//
//        virtual double match(const Meme &a, const Meme &b) const;
//
// that returns a number between 0 and 1 indicating how substitutable
// the two memes a and b are.  Both have default implementations.
class Memes: public Network<Meme<Memebase>> {
public:
  Memes() {}
  Memes(const Enumvector<Meme<Memebase>,double>& e): Network(e) {}
  Memes(Enumvector<Meme<Memebase>,double>&& e): Network(std::forward<decltype(e)>(e)) {}
  Memes(std::mt19937&r, const int m=-1): Network(r,m){}
  // If slicing is an issue, define the virtual = operators
};

class BitstringMemes: public Memes {
	unsigned bits = count_bits(Meme<Memebase>::getn()); // It's not clear if this is evaluated only when the class is constructed... test this!
public:
	using Memes::Memes; // We can inherit all the constructors here
	
	// ... and overload the virtual functions
	
	// Neighbor: we randomly tweak a bit
	Meme<Memebase> neighbor(const Meme<Memebase>& m, std::mt19937& r) const override {
		std::uniform_int_distribution<> bit_twiddler(0, bits-1);
		int was = static_cast<int>(m);
		int is;
		do {
			is = was ^ (1 << bit_twiddler(r));
		} while(is >= Meme<Memebase>::getn());
		return Meme<Memebase>(is);
	}
	
	// Match: we ask how many bits are in common, and return as a fraction 0 to 1
	double match(const Meme<Memebase>& a, const Meme<Memebase>& b) const override {
// 		std::cout << "BitstringMemes::match" << std::endl;
		return static_cast<double>(common_bits(static_cast<int>(a), static_cast<int>(b),bits)) / static_cast<double>(bits);
	}
};


// This is essentially a repeat of what we did above for Meme.
class Lexbase: public Enum<lexid> {
public:
  virtual ~Lexbase(){}
protected:
  explicit Lexbase(const int &n): Enum(n) {}
  Lexbase(const Enum &n): Enum(n) {}
};
class Lexemes: public Network<Lexeme<Lexbase>> {
public:
  Lexemes(const int m=-1): Network(m) {}
  Lexemes(std::mt19937&r, const int m=-1): Network(r,m){}
  Lexemes(const Probvector<Lexeme<Lexbase>>& p): Network(p){}
  Lexeme<Lexbase> neighbor(const Lexeme<Lexbase>& l, std::mt19937&) const {return l;}
  double match(const Lexeme<Lexbase> &l1, const Lexeme<Lexbase> &l2) const {return l1==l2;}
  // If slicing is an issue, define the virtual = operators
};

// This is essentially a repeat of what we did above for Meme.
class Agentbase: public Enum<agentid> {
public:
  virtual ~Agentbase(){}
protected:
  explicit Agentbase(const int &n): Enum(n) {}
  Agentbase(const Enum &n): Enum(n) {}
};
class Agents: public Network<Agent<Agentbase>> {
public:
  Agents() {}
  Agents(std::mt19937&r, const int m=-1): Network(r,m){}
  // If slicing is an issue, define the virtual = operators
};

class AgentLanguage: public Language<Memes,Lexemes> {
public:
  AgentLanguage(){}
  AgentLanguage(const Enumvector<Memes::Index,Lexemes>& e): Language(e) {}
  AgentLanguage(Enumvector<Memes::Index,Lexemes>&& e): Language(std::forward<decltype(e)>(e)) {}
  AgentLanguage(const Memes &m,std::mt19937& r, const int mask=-1):Language(m,r,mask){}
  AgentLanguage(Memes &&m,std::mt19937& r, const int mask=-1):Language(std::forward<decltype(m)>(m),r,mask){}
  AgentLanguage(const Memes &m, const int mask=-1):Language(m,mask){}
  AgentLanguage(Memes &&m, const int mask=-1):Language(std::forward<decltype(m)>(m),mask){}
  AgentLanguage(const Memes &m, const Language &l):Language(m,l){}
  AgentLanguage(const Memes &m, Language &&l):Language(m,std::forward<decltype(l)>(l)){}
  AgentLanguage(Memes &&m, const Language &l):Language(std::forward<decltype(m)>(m),l){}
  AgentLanguage(Memes &&m, Language &&l):Language(std::forward<decltype(m)>(m),std::forward<decltype(l)>(l)){}
  AgentLanguage(const Language& l, const int cshift=0):Language(l, cshift) {}
  AgentLanguage(Language && l, const int cshift=0): Language(std::forward<decltype(l)>(l),cshift) {}
  AgentLanguage(const Language& l, std::mt19937& r): Language(l,r) {}
  AgentLanguage(Language&& l, std::mt19937& r): Language(std::forward<decltype(l)>(l),r) {}
  ~AgentLanguage(){}
};

/**
  * This overrides the lexmutate method to do the reinforcement learning based on some experience.
  * We need another parameter lambda which is the scale of the reinforcement learning
  */
class ReinforcementLearnerLanguage: public Language<BitstringMemes,Lexemes> {
   double lambda;
public:
	ReinforcementLearnerLanguage(double lambda) : lambda(lambda) { }
	ReinforcementLearnerLanguage(double lambda, const BitstringMemes &m, const int mask=-1):  Language(m,mask), lambda(lambda) {}
	ReinforcementLearnerLanguage(double lambda, const Memes &m,std::mt19937& r, const int mask=-1):Language(m,r,mask), lambda(lambda) {}
    ReinforcementLearnerLanguage(double lambda, const BitstringMemes &m, const Language &l):Language(m,l), lambda(lambda) {}

	
	void lexmutate(const double sigma, Lexemes::Generator &r,
			 const Experience<Language::Meme,Language::Lexeme> &experience = Experience<Language::Meme,Language::Lexeme>()) {
		// Go through each association and boost/suppress by the relevant amount
// 		std::cout << "ReinforcementLearnerLanguage::lexmutate" << std::endl;
		for(auto& assn : experience) {
			// assn.first.first = meme, assn.first.second = lexeme, assn.second = boost amount
			(*this)[assn.first.first].reinforce(assn.first.second, lambda * assn.second);
		}
		deleteCache();
  }
};

// I would like instead of this for any subclass of AgentLanguage to be stored in the EnumVector
// without object slicing so that virtual functions are called properly, but I am not sure if
// this can be done.
template<typename AgentLanguage> 
class Population: public Enumvector<Agent<Agentbase>,AgentLanguage> {
public:
  Population(){}
  Population(const AgentLanguage &l): Enumvector<Agent<Agentbase>,AgentLanguage>(l) {}
  Population(const Enumvector<Agent<Agentbase>,AgentLanguage>& e): Enumvector<Agent<Agentbase>,AgentLanguage>(e) {}
  Population(Enumvector<Agent<Agentbase>,AgentLanguage>&& e): Enumvector<Agent<Agentbase>,AgentLanguage>(std::forward<decltype(e)>(e)) {}
};

template<typename AgentLanguage>
inline std::ostream& operator<< (std::ostream& o, const Population<AgentLanguage> &e) {
  std::copy(e.cbegin(), e.cend(), std::ostream_iterator<AgentLanguage>(o));
  return o << std::endl;
}

template<typename AgentLanguage>
Enumvector<Agent<Agentbase>,Counts> communicate(const Agents &, const Lexemes &, const Memes &,
						const Population<AgentLanguage> &, int);

enum ModelType {A, B};
template<enum ModelType m> class chooselang{};
template<> class chooselang<A>{
public:
  typedef AgentLanguage Language;
  static Language langinit(const int uniform, const double lambda,
				const Memes& memes, std::mt19937& r) {
    return uniform > 0?Language(memes):
      uniform < 0?Language(memes,unitlang((Language*)0)):
      Language(memes,r);
  }
};
template<> class chooselang<B>{
public:
  typedef ReinforcementLearnerLanguage Language;
  static Language langinit(const int uniform, const double lambda,
				const Memes& memes, std::mt19937& r) {
    return uniform > 0?Language(lambda, memes):
      uniform < 0?Language(lambda, memes,unitlang((Language*)0)):
      Language(lambda, memes,r); // DIFFERENT: The type of all the languages differs wrt model A, and so does population
  }
};

#endif
