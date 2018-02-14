#ifndef MAIN_DECLS_HPP
#define MAIN_DECLS_HPP

#include "main.h++"
#include <random>
#include <utility>
#include <ostream>
#include <istream>
#include <iterator>
#include <cctype>
#include "sccs.h++"

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

static const SCCS::sccs_id MAIN_DECLS_HPP_SCCS_ID __attribute__((used)) = "@(#)main_decls.h++: $Id$";

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
class Memebase: public Enum::Enum<Enum::memeid> {
public:
  // virtual ~Memebase() override = default;
  using Enum::Enum;
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
class Memes: public Network::Network<Meme::Meme<Memebase>> {
 public:
  using Meme = typename Memes::Agent;
  using base_Enumvector = typename Memes::Enumvector;
  Memes(const Network& n): Network(n) {}
  Memes(Network&& n): Network(std::forward<decltype(n)>(n)) {}
  // All other constructors enforce basematch_adjacency
  Memes(): Network(Probvector(-1),basematch_adjacency()) {}
  Memes(const base_Enumvector& e): Network(e,basematch_adjacency()) {}
  Memes(base_Enumvector&& e): Network(std::forward<decltype(e)>(e),basematch_adjacency()) {}
  Memes(std::mt19937&r, const int m=-1): Network(Probvector(r,m),basematch_adjacency()) {}
  // If slicing is an issue, define the virtual = operators
};

class BitstringMemes: public Memes {
  unsigned bits = util::count_bits(Meme::getn()); // It's not clear if this is evaluated only when the class is constructed... test this!
public:
  using Memes::Memes;
  BitstringMemes(const Memes &m): Memes(m) {}
  BitstringMemes(Memes &&m): Memes(std::forward<decltype(m)>(m)) {}
  // BitstringMemes(const Network& n): Memes(n) {}
  // BitstringMemes(Network&& n): Memes(std::forward<decltype(n)>(n)) {}
  // Override other constructors to enforce bitset_adjacency
  BitstringMemes(): Memes(Network(Probvector(-1),bitset_adjacency())) {}
  BitstringMemes(const base_Enumvector& e): Memes(Network(e,bitset_adjacency())) {}
  BitstringMemes(base_Enumvector&& e): Memes(Network(std::forward<decltype(e)>(e),bitset_adjacency())) {}
  BitstringMemes(std::mt19937&r, const int m): Memes(Network(Probvector(r,m),bitset_adjacency())) {}
  BitstringMemes(std::mt19937&r): BitstringMemes(r,-1) {}
  // If slicing is an issue, define the virtual = operators

	// ... and overload the virtual functions

	// Neighbor: we randomly tweak a bit
  Meme neighbor(const Meme& m, std::mt19937& r) const override {
		std::uniform_int_distribution<> bit_twiddler(0, bits-1);
		int was = static_cast<int>(m);
		int is;
		do {
			is = was ^ (1 << bit_twiddler(r));
		} while(is >= Meme::getn());
		return Meme(is);
	}

	// Match: we ask how many bits are in common, and return as a fraction 0 to 1
	double match(const Meme& a, const Meme& b) const override {
// 		std::cout << "BitstringMemes::match" << std::endl;
	  return static_cast<double>(util::common_bits(static_cast<int>(a), static_cast<int>(b),bits)) / static_cast<double>(bits);
	}
};


// This is essentially a repeat of what we did above for Meme.
class Lexbase: public Enum::Enum<Enum::lexid> {
public:
  // virtual ~Lexbase() override = default;
  using Enum::Enum;
  Lexbase(const Enum &n): Enum(n) {}
};
class Lexemes: public Network::Network<Lex::Lexeme<Lexbase>> {
public:
  Lexemes(const Network& n): Network(n) {}
  Lexemes(Network&& n): Network(std::forward<decltype(n)>(n)) {}
  // All others enforce digonal_adjacency
  Lexemes(const int m=-1): Network(m,diagonal_adjacency()) {declarediag();}
  Lexemes(std::mt19937&r, const int m=-1): Network(Probvector(r,m),diagonal_adjacency()){declarediag();}
  Lexemes(const Probvector& p): Network(p,diagonal_adjacency()){declarediag();}
  // The following two are no longer needed: they follow from the diagonal_adjacency
  // Lexeme<Lexbase> neighbor(const Lexeme<Lexbase>& l, std::mt19937&) const {return l;}
  // double match(const Lexeme<Lexbase> &l1, const Lexeme<Lexbase> &l2) const {return l1==l2;}
  // If slicing is an issue, define the virtual = operators
};

// This is essentially a repeat of what we did above for Meme.
class Agentbase: public Enum::Enum<Enum::agentid> {
public:
  // virtual ~Agentbase() override {}
  using Enum::Enum;
  Agentbase(const Enum &n): Enum(n) {}
};
class Agents: public Network::Network<Agent::Agent<Agentbase>> {
public:
  Agents(const Network& n): Network(n) {}
  Agents(Network&& n): Network(std::forward<decltype(n)>(n)) {}
  Agents() {}
  Agents(std::mt19937&r, const int m=-1): Network(r,m){}
  // If slicing is an issue, define the virtual = operators
};

class AgentLanguage: public Language::Language<Memes,Lexemes> {
public:
  using base_Enumvector=AgentLanguage::Enumvector;
  AgentLanguage() = default;
  using Language::Language;
  AgentLanguage(const Language& l):Language(l) {}
  AgentLanguage(Language && l): Language(std::forward<decltype(l)>(l)) {}
  // virtual ~AgentLanguage() override = default;
};

/**
  * This overrides the lexmutate method to do the reinforcement learning based on some experience.
  * We need another parameter lambda which is the scale of the reinforcement learning
  */
class ReinforcementLearnerLanguage: public Language::Language<BitstringMemes,Lexemes> {
   double lambda;
public:
  ReinforcementLearnerLanguage() = default; // default constructor needed for i/o library istream_iterator implementation
	ReinforcementLearnerLanguage(double lambda) : lambda(lambda) { }
	ReinforcementLearnerLanguage(double lambda, const BitstringMemes &m, const int mask=-1):  Language(m,mask), lambda(lambda) {}
	ReinforcementLearnerLanguage(double lambda, const Memes &m,std::mt19937& r, const int mask=-1):Language(m,r,mask), lambda(lambda) {}
    ReinforcementLearnerLanguage(double lambda, const BitstringMemes &m, const Language &l):Language(m,l), lambda(lambda) {}


  virtual void lexmutate(const double sigma, Lexemes::Generator &r,
			 const Experience::Experience<Language::Meme,Language::Lexeme> &experience = Experience::Experience<Language::Meme,Language::Lexeme>()) override {
		// Go through each association and boost/suppress by the relevant amount
// 		std::cout << "ReinforcementLearnerLanguage::lexmutate" << std::endl;
		for(auto& assn : experience) {
			// assn.first.first = meme, assn.first.second = lexeme, assn.second = boost amount
			(*this)[assn.first.first].reinforce(assn.first.second, lambda * assn.second);
		}
		deleteCache();
  }
  // virtual ~ReinforcementLearnerLanguage () override = default;
};

// I would like instead of this for any subclass of AgentLanguage to be stored in the Enumvector
// without object slicing so that virtual functions are called properly, but I am not sure if
// this can be done.
template<typename AgentLanguage>
class Population: public Enumvector::Enumvector<Agent::Agent<Agentbase>,AgentLanguage> {
public:
  using base_Enumvector=typename Population::Enumvector;
  Population() = default;
  Population(const AgentLanguage &l): base_Enumvector(l) {}
  Population(const base_Enumvector& e): base_Enumvector(e) {}
  Population(base_Enumvector&& e): base_Enumvector(std::forward<decltype(e)>(e)) {}
};

template<typename AgentLanguage>
inline std::ostream& operator<< (std::ostream& o, const Population<AgentLanguage> &e) {
  std::copy(e.cbegin(), e.cend(), std::ostream_iterator<AgentLanguage>(o));
  return o << std::endl;
}

template<typename AgentLanguage>
Enumvector::Enumvector<Agent::Agent<Agentbase>,Counts::Counts> communicate(const Agents &, const Lexemes &, const Memes &,
									   const Population<AgentLanguage> &, int=1, int=1, int=1, int=1, int=1);
enum ModelType {A, B, C, P};
inline
std::ostream& operator<<(std::ostream &o, const ModelType m) {
  return o << (m==A?"A":m==B?"B":m==C?"C":m==P?"P":"Invalid");
}
inline
std::istream& operator>>(std::istream &i, ModelType &m) {
  char model;
  i >> model;
  model = std::toupper(model);
  m = model == 'B'?B: model == 'C'?C: model == 'P'?P: A;
  return i;
}


template<enum ModelType m> class chooselang final{
  // Make this the default (A, C, P)
  // template<> class chooselang<A>{
public:
  typedef AgentLanguage Language;
  static Language langinit(const int uniform, const double lambda,
				const Memes& memes, std::mt19937& r) {
    return uniform > 0?Language(memes):
      uniform < 0?Language(memes,unitlang(static_cast<Language*>(nullptr))): // Koenig lookup needs cast
      Language(memes,r);
  }
};
template<> class chooselang<B> final{
public:
  typedef ReinforcementLearnerLanguage Language;
  static Language langinit(const int uniform, const double lambda,
				const Memes& memes, std::mt19937& r) {
    return uniform > 0?Language(lambda, memes):
      uniform < 0?Language(lambda, memes,unitlang(static_cast<Language*>(nullptr))): // Koenig lookup needs cast
      Language(lambda, memes,r); // DIFFERENT: The type of all the languages differs wrt model A, and so does population
  }
};

#endif
