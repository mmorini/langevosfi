#include <cmath>
#include <iostream>

static const char MAIN_CPP_SCCS_ID[] __attribute__((used)) = "@(#)main.c++: $Id$";

// language = nummemes*numlexes, population = numagents
constexpr auto nummemes=10, numlexes=15, numagents=40;
// The following two parameters are effectively in the exponent, so careful
constexpr auto mutrate = 1; constexpr auto penalty = 100;
// Inner iterations measures the fitness of the language
// Outer iterations converges
constexpr auto inner=8*numagents*numlexes*nummemes,
               outer=3*numagents*numlexes*nummemes;

#include <random>
// Everything below uses r as the random number generator.  If we
// change the type of the generator, then Language and Network (and
// Probvector if you use it directly) need to be provided that type as
// an additional parameter.
std::mt19937 r;

#include "enum.h++"
#include "network.h++"

// This is used by Enum template in prints
constexpr const char memeid [] = "M";
// meme.h++ extends Memebase to define Meme.  So, define this class
// anyhow.  The rest of the program assumes that it publicly derives
// from an instantiation of the Enum template, and the functions in
// the Enum template have not been overriden.  Meme currently is an
// empty extension.
class Memebase: public Enum<memeid,nummemes> {
protected:
  explicit Memebase(const int &n): Enum(n) {}
  Memebase(const Enum &n): Enum(n) {}
};
#include "meme.h++"
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
class Memes: public Network<Meme> {
public:
  Memes(std::mt19937&r): Network<Meme>(r){}
};
// OK, this generates a random probabilities for the network of memes.
Memes memes(r);

// This is essentially a repeat of what we did above for Meme.
constexpr const char lexid[]="L";
class Lexbase: public Enum<lexid,numlexes> {
protected:
  explicit Lexbase(const int &n): Enum(n) {}
  Lexbase(const Enum &n): Enum(n) {}
};
#include "lex.h++"
class Lexemes: public Network<Lexeme> {
public:
  Lexemes(std::mt19937&r): Network<Lexeme>(r){}
  Lexeme neighbor(const Lexeme& l, std::mt19937&) const {return l;}
  double match(const Lexeme &l1, const Lexeme &l2) const {return l1==l2;}
};
Lexemes lexemes(r);

// This is essentially a repeat of what we did above for Meme.
constexpr const char agentid[] = "A";
class Agentbase: public Enum<agentid,numagents> {
protected:
  explicit Agentbase(const int &n): Enum(n) {}
  Agentbase(const Enum &n): Enum(n) {}
};
#include "agent.h++"
class Agents: public Network<Agent> {
public:
  Agents(std::mt19937&r): Network<Agent>(r){}
};
Agents agents(r);

#include "language.h++"
#include "enumvector.h++"
// Enumvector<A,B> is like Vector<B>, but indexed by A.
//
// Language is the heart of the code. It defines a number of virtual
// functions that can be overriden:
//
//       virtual Meme memegen(generator &r) const
//                     --- return a random meme
//       virtual Lexeme lexgen(const Meme m, generator &r) const
//                     --- given a meme, generate a lex
//       virtual Meme memegen(const Lexeme l, generator &r) const
//                     --- given a lex, generate a meme
//       virtual void mememutate(const double sigma, generator &r)
//                     --- Mutate the probability of memes
//       virtual void lexmutate(const double sigma, generator &r,
//                              const Enumvector<Meme,Counts> &counts)
//                     --- Mutate the language holding the marginals
//                     --- (i.e., the probability of the memes)
//                     --- constant
//                     --- If the last parameter is supplied, can
//                     --- use it to choose meme whose lex distribution
//                     --- to mutate. Currently ignored.
//       virtual Language& decache(void)
//                     --- Declare that the cache can be stolen if the
//                     --- language is copied. (Returns the Language itself).
//                     --- Can be applied to const or nonconst
//
class AgentLanguage: public Language<Meme,Lexeme> {
public:
  AgentLanguage(){}
  AgentLanguage(Memes m,std::mt19937& r):Language(m,r){}
};
Enumvector<Agent,AgentLanguage> population;

// Counts is a misnomer: it actually keeps track of the average
// overlap between the meme communicated and the meme interpreted.  It
// is a count only when the overlap is a 0/1 variable.
#include "counts.h++"
Enumvector<Agent,Counts> counts;

// selfiterator uses C++14 magic to be able to iterate over indices
// of a vector, ranges of integers, etc.
#include "selfiterator.h++"

// communicate makes n communication attempts, and returns the
// communication success of each agent.  Currently it iterates the
// procedure: choose an agent, make it choose a meme according to the
// marginal of its language, choose a lex according to its own
// language, send it to a random neighbor, who interprets it according
// to her own (presumably different) language.
auto communicate(const int n) {
  Enumvector<Agent,Counts> retval;
  for (auto rounds: range(n)) {
    (void)rounds;
    Agent a1(agents.generate(r));
    Meme m1(population[a1].memegen(r));
    Lexeme l1(population[a1].lexgen(m1,r));
    Agent a2(agents.neighbor(a1,r));
    Lexeme l2(population[a1].transmit(lexemes,l1,r,population[a2]));
    Meme m2(population[a2].memegen(l2,r));
    retval[a1]+=population[a1].match(memes,m1,m2,population[a2]);
  }
  return retval;
}

// OK now the main loop
int main(void) {
  // Generate an entire population as a set of random languages:
  // everyone has the same marginals; write it out.
  for (auto &a: population)
    a = AgentLanguage(memes,r);
  std::cout << "\t" << population;

  // Initialize everybodies counts and write out summary.
  auto counts=communicate(inner);
  summarize(counts);

  // For the number of outer loops, store the oldlanguage in a
  // temporary, mutate the language holding marginals fixed,
  // communicate, and choose the new or the old language by throwing a
  // random number.
  for (auto rounds: range(outer)) {
    (void)rounds;
    // Mark cache as moving to 'oldpop' in the next statement.
    for (auto &a: population) a.decache();
    auto oldpop = population;
    auto oldcounts = counts;

    // Mutate each language
    for (auto &a: population) a.lexmutate(mutrate,r);

    // Generate new counts and write out summary.
    counts=communicate(inner); summarize(counts);

    // Look at each agent's language
    for(auto a: indices(counts)) {
      // Accept new language if it did not get tried, else accept
      // randomly if it is worse and deterministically if it is better.
      if(counts[a].tries>0 && oldcounts[a].tries>0) {
	auto delta = counts[a].mean() - oldcounts[a].mean();
	if (delta < 0 &&
	    std::generate_canonical<double, 20>(r) >
	    exp(penalty*delta)) {
	  // std::move promises the oldpop and olcount won't be used
	  // any ore, so steal whatever data structure you can.
	  population[a] = std::move(oldpop[a]);
	  counts[a] = std::move(oldcounts[a]);
	}
      }
    } 
  }
  std::cout << "\t" << population;
  return 0;
}
