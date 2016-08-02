#include "main.h++"
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

static const char MAIN_CPP_SCCS_ID[] __attribute__((used)) = "@(#)main.c++: $Id$";

// Everything below uses r as the random number generator.  If we
// change the type of the generator, then Language and Network (and
// Probvector if you use it directly) need to be provided that type as
// an additional parameter.
std::mt19937 r;

// This is used by Enum template in prints
constexpr const char memeid [] = "M";
constexpr const char lexid[] = "L";
constexpr const char agentid[] = "A";

// Define the variables holding the sizes.
template<> int Enum<memeid>::n = 0;
template<> int Enum<lexid>::n = 0;
template<> int Enum<agentid>::n = 0;

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
  Lexemes() {}
  Lexemes(std::mt19937&r, const int m=-1): Network(r,m){}
  Lexeme<Lexbase> neighbor(const Lexeme<Lexbase>& l, std::mt19937&) const {return l;}
  double match(const Lexeme<Lexbase> &l1, const Lexeme<Lexbase> &l2) const {return l1==l2;}
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
  Agents(std::mt19937&r, const int m=-1): Network(r,m){}
};

class AgentLanguage: public Language<Memes,Lexemes> {
public:
  AgentLanguage(){}
  AgentLanguage(const Memes m,std::mt19937& r):Language(m,r){}
  AgentLanguage(const Language& l):Language(l) {}
  AgentLanguage(Language && l): Language(std::forward<Language>(l)) {}
  ~AgentLanguage(){}
};

class Population: public Enumvector<Agent<Agentbase>,AgentLanguage> {
public:
  Population(){}
  Population(const Enumvector& e): Enumvector(e) {}
  Population(Enumvector && e): Enumvector(std::forward<Enumvector>(e)) {}
};

// communicate makes n communication attempts, and returns the
// communication success of each agent.  Currently it iterates the
// procedure: choose an agent, make it choose a meme according to the
// marginal of its language, choose a lex according to its own
// language, send it to a random neighbor, who interprets it according
// to her own (presumably different) language.
auto communicate(const Agents &agents,
		 const Lexemes &lexemes,
		 const Memes &memes,
		 const Population &population,
		 const int n) {
  Enumvector<Agent<Agentbase>,Counts> retval;
  for (auto rounds: range(n)) {
    (void)rounds;
    const Agent<Agentbase> &a1(agents.generate(r));
    const Meme<Memebase> &m1(population[a1].memegen(r));
    const Lexeme<Lexbase> &l1(population[a1].lexgen(m1,r));
    const Agent<Agentbase> &a2(agents.neighbor(a1,r));
    const Lexeme<Lexbase> &l2(population[a1].transmit(lexemes,l1,r,population[a2]));
    const Meme<Memebase> &m2(population[a2].memegen(l2,r));
    retval[a1]+=population[a1].match(memes,m1,m2,population[a2]);
  }
  return retval;
}

// OK now the main loop
int main(void) {

  // language = nummemes*numlexes, population = numagents
  std::cerr << "Provide nummemes, numlexes, and numagents (e.g., 10 15 40)" << std::endl;
  Meme<Memebase>::setn(*std::istream_iterator<int>(std::cin)); /* 10 */
  Lexeme<Lexbase>::setn(*std::istream_iterator<int>(std::cin)); /* 15 */
  Agent<Agentbase>::setn(*std::istream_iterator<int>(std::cin)); /* 40 */
  std::cout <<   "nummemes  = " << Meme<Memebase>::getn()
            << ", numlexes  = " << Lexeme<Lexbase>::getn()
            << ", numagents = " << Agent<Agentbase>::getn() << std::endl;
  
  // The following two parameters are effectively in the exponent, so careful
  std::cerr << "Provide mutrate and penalty (e.g., 1 100)" << std::endl;
  const auto mutrate = *std::istream_iterator<double>(std::cin); /* 1 */
  const auto penalty = *std::istream_iterator<double>(std::cin); /* 100 */
  std::cout <<   "mutrate = " << mutrate
            << ", penalty = " << penalty << std::endl;

  // Inner iterations measures the fitness of the language
  // Outer iterations converges
  std::cerr << "Provide inner and outer iteration count (e.g., " <<
    8*Agent<Agentbase>::getn()*Lexeme<Lexbase>::getn() << " " <<
    3*Agent<Agentbase>::getn()*Lexeme<Lexbase>::getn()*
    Meme<Memebase>::getn()*Meme<Memebase>::getn() << ")" << std::endl;
  const auto inner=*std::istream_iterator<int>(std::cin),
             outer=*std::istream_iterator<int>(std::cin);
  std::cout <<   "inner = " << inner
            << ", outer = " << outer << std::endl;

  // Seed the random number generator. Needs a sequence of unsigned intergers
  // to generate a seed.
  std::cerr << "Provide unsigned integers and end file to seed random number generator" << std::endl;
  const std::vector<unsigned int> seed_vector((std::istream_iterator<unsigned int>(std::cin)),
					      std::istream_iterator<unsigned int>());
  std::seed_seq seeds(seed_vector.begin(), seed_vector.end());
  r.seed(seeds);
  std::cout << "Random number generator seeded with ";
  for (const auto s: seed_vector) std::cout << s << " ";
  std::cout << std::endl;
    

  // OK, this generates a random probabilities for the network of memes.
  Memes memes(r);
  Lexemes lexemes(r);
  Agents agents(r);
  Population population;

  // Generate an entire population as a set of random languages:
  // everyone has the same marginals; write it out.
  for (auto &a: population)
    a = AgentLanguage(memes,r);
  std::cout << "\t" << population;

  // Initialize everybodies counts and write out summary.
  auto counts=communicate(agents,lexemes,memes,population,inner);
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
    counts=communicate(agents,lexemes,memes,population,inner); summarize(counts);

    // Look at each agent's language
    for(auto a: indices(counts)) {
      // Accept new language if it did not get tried, else accept
      // randomly if it is worse and deterministically if it is better.
      if(counts[a].tries>0 && oldcounts[a].tries>0) {
	auto delta = counts[a].mean() - oldcounts[a].mean();
	if (delta < 0 &&
	    std::generate_canonical<double, 20>(r) >
	    exp(penalty*delta)) {
	  // std::move promises the oldpop and oldcounts array element
	  // won't be used any more, so steal whatever data structure you can.
	  population[a] = std::move(oldpop[a]);
	  counts[a] = std::move(oldcounts[a]);
	}
      }
    } 
  }
  std::cout << "\t" << population;
  return 0;
}
