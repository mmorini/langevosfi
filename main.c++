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
};

// communicate makes n communication attempts, and returns the
// communication success of each agent.  Currently it iterates the
// procedure: choose an agent, make it choose a meme according to the
// marginal of its language, choose a lex according to its own
// language, send it to a random neighbor, who interprets it according
// to her own (presumably different) language.

// In model A we keep track of the overall success of our interactions.
// The 'match' function tells us how successful this is, and we just keep adding the success.
Enumvector<Agent<Agentbase>,Experience<Meme<Memebase>,Lexeme<Lexbase>>> communicate_modelA(const Agents &agents,
		 const Lexemes &lexemes,
		 const Memes &memes,
		 const Population<AgentLanguage> &population,
		 const int n) {
  Enumvector<Agent<Agentbase>,Experience<Meme<Memebase>,Lexeme<Lexbase>>> retval;
  for (auto rounds: range(n)) {
    (void)rounds;
    const Agent<Agentbase> &a1(agents.generate(r));
    const Meme<Memebase> &m1(population[a1].memegen(r));
    const Lexeme<Lexbase> &l1(population[a1].lexgen(m1,r));
    const Agent<Agentbase> &a2(agents.neighbor(a1,r));
    const Lexeme<Lexbase> &l2(population[a1].transmit(lexemes,l1,r,population[a2]));
    const Meme<Memebase> &m2(population[a2].memegen(l2,r));
    retval[a1].increase_association( m1, l1, population[a1].match(memes,m1,m2,population[a2]) );
  }
  return retval;
}

// In model B we instead randomly increase / decrease the association. We interpret match
// as a number m between 0 and 1 (perfect mismatch -> perfect match).
// The signalling agent records a success w.p m^2; a failure w.p. (1-m)^2 and does nothing otherwise
// (Other rules are available, but I like this one)
Enumvector<Agent<Agentbase>,Experience<Meme<Memebase>,Lexeme<Lexbase>>> communicate_modelB(const Agents &agents,
		 const Lexemes &lexemes,
		 const Memes &memes,
		 const Population<ReinforcementLearnerLanguage> &population,
		 const int n) {
  Enumvector<Agent<Agentbase>,Experience<Meme<Memebase>,Lexeme<Lexbase>>> retval;
  for (auto rounds: range(n)) {
    (void)rounds;
    const Agent<Agentbase> &a1(agents.generate(r));
    const Meme<Memebase> &m1(population[a1].memegen(r));
    const Lexeme<Lexbase> &l1(population[a1].lexgen(m1,r));
    const Agent<Agentbase> &a2(agents.neighbor(a1,r));
    const Lexeme<Lexbase> &l2(population[a1].transmit(lexemes,l1,r,population[a2]));
    const Meme<Memebase> &m2(population[a2].memegen(l2,r));
    const auto ran = std::generate_canonical<double, 20>(r);
    const double match = population[a1].match(memes,m1,m2,population[a2]);
//     std::cout << "::communicate_modelB match->" << match << std::endl;
    if(ran < match*match)
	    retval[a1].increase_association( m1, l1, 1.0 );
	else if( ran < 1.0 - 2.0 * match * (1.0 - match) ) 
	    retval[a1].increase_association( m1, l1, -1.0 );	
  }
  return retval;
}


// OK now the main loop

// This is Tanmoy's original Model A

// I have to create a separate function for Model B as I don't yet understand how
// to hot swap types (e.g. between Memes and BitstringMemes) in heavily templated code
// like this.

int runModelA(void) {

  // language = nummemes*numlexes, population = numagents
  std::cerr << "Provide nummemes, numlexes, and numagents (e.g., 10 15 40)" << std::endl;
  Meme<Memebase>::setn(*std::istream_iterator<int>(std::cin)); /* 10 */
  Lexeme<Lexbase>::setn(*std::istream_iterator<int>(std::cin)); /* 15 */
  Agent<Agentbase>::setn(*std::istream_iterator<int>(std::cin)); /* 40 */
  std::cout <<   "nummemes  = " << Meme<Memebase>::getn()
            << ", numlexes  = " << Lexeme<Lexbase>::getn()
            << ", numagents = " << Agent<Agentbase>::getn() << std::endl;


  // uniform = 1: completely ambiguous language
  //          -1: no synonymy
  //           0: random
  // uniform != 0 also sets meaning marginals = uniform, and
  //    equal-weight complete network of agents. (should this be
  //    a separate parameter?)
  // syncstart = 1: everyone has same language
  //            -1: languages rotated
  //             0: random
  std::cerr << "uniform (-1, 0 or 1), and syncstart (+1, -1, or 0)" << std::endl;
  const auto uniform = *std::istream_iterator<int>(std::cin);
  const auto syncstart = *std::istream_iterator<int>(std::cin);
  std::cout << "uniform = " << uniform << " syncstart = " << syncstart << std::endl;
  
  // The following two parameters are effectively in the exponent, so careful
  std::cerr << "Provide mutrate and penalty (e.g., 1 100)" << std::endl;
  const auto mutrate = *std::istream_iterator<double>(std::cin); /* 1 */
  const auto penalty = *std::istream_iterator<double>(std::cin); /* 100 */
  std::cout <<   "mutrate = " << mutrate
            << ", penalty = " << penalty << std::endl;

  // Inner iterations measures the fitness of the language
  // Outer iterations converges
  std::cerr << "Provide inner and outer iteration count, printinterval (e.g., " <<
    8*Agent<Agentbase>::getn()*Lexeme<Lexbase>::getn() << " " <<
    3*Agent<Agentbase>::getn()*Lexeme<Lexbase>::getn()*
    Meme<Memebase>::getn()*Meme<Memebase>::getn() << " " <<
    1 << ")" << std::endl;
  const auto inner=*std::istream_iterator<int>(std::cin),
             outer=*std::istream_iterator<int>(std::cin),
             printinterval = *std::istream_iterator<int>(std::cin);
  std::cout <<   "inner = " << inner
            << ", outer = " << outer
	    << ", printinterval = " << printinterval
	    << std::endl;

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
  Memes memes(uniform != 0?Memes():Memes(r));
  Lexemes lexemes(uniform != 0?Lexemes():Lexemes(r));
  Agents agents(uniform != 0?Agents():Agents(r));
  // Generate an entire population; write it out.

  // The memes currently can be defaulted in the first two cases below, but trying
  // to keep it general.  Speed at initialization is unlikely to be an issue
  Population<AgentLanguage> population(uniform > 0?AgentLanguage(memes):
			uniform < 0?AgentLanguage(memes,unitlang((AgentLanguage*)0)):
			AgentLanguage(memes,r));
   if (syncstart < 0) {
     int c=0;
     for (auto &a: population)
       a.cshift(c++);
   } else if (syncstart == 0)
     for (auto &a: population)
      a.permute(r);
  std::cout << "\t" << population;

  // Initialize everybodies counts and write out summary.
  auto counts=communicate_modelA(agents,lexemes,memes,population,inner);
  summarize(counts);

  // For the number of outer loops, store the oldlanguage in a
  // temporary, mutate the language holding marginals fixed,
  // communicate, and choose the new or the old language by throwing a
  // random number.
  for (auto rounds: range(outer)) {
    if (rounds > 0 && printinterval > 0 && rounds % printinterval == 0)
      std::cout << "Round number " << rounds << std::endl
		<< "\t" << population;

    // Mark cache as moving to 'oldpop' in the next statement.
    for (auto &a: population) a.decache();
    auto oldpop = population;
    auto oldcounts = counts;

    // Mutate each language
    for (auto &a: population) a.lexmutate(mutrate,r);

    // Generate new counts and write out summary.
    counts=communicate_modelA(agents,lexemes,memes,population,inner); summarize(counts);

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

// I'm not exactly sure how to unify the two models when heavily templated types are 
// different from one implementation to the next. If we were using runtime polymorphism
// I would know what to do (use pointers to common base classes) but with compile-time
// polymorphism (templates) I'm not sure what to do. So I've just C&Ped the model A loop
// and marked where it is different.

// TODO: consult with Tanmoy on how we make the different types interchangeable so that
// the virtual functions all get called at the right time, which in turn will allow us
// to have a single run function and hot-swap the different subroutines.

// We're nearly there... the issue is that if Population is not a template, we can
// still initialise it with ReinforcementLearnerLanguage BUT this gets sliced when
// it's stored in the underlying std::vector<AgentLanguage>. The internet suggests
// we ought to be able to use a reference_wrapper but I can't figure out how to do
// this. Isn't it so lovely when several lines of text all end up the same length?

int runModelB(void) {
  // language = nummemes*numlexes, population = numagents
  std::cerr << "Provide nummemes, numlexes, and numagents (e.g., 10 15 40)" << std::endl;
  Meme<Memebase>::setn(*std::istream_iterator<int>(std::cin)); /* 10 */
  Lexeme<Lexbase>::setn(*std::istream_iterator<int>(std::cin)); /* 15 */
  Agent<Agentbase>::setn(*std::istream_iterator<int>(std::cin)); /* 40 */
  std::cout <<   "nummemes  = " << Meme<Memebase>::getn()
            << ", numlexes  = " << Lexeme<Lexbase>::getn()
            << ", numagents = " << Agent<Agentbase>::getn() << std::endl;


  // uniform = 1: completely ambiguous language
  //          -1: no synonymy
  //           0: random
  // uniform != 0 also sets meaning marginals = uniform, and
  //    equal-weight complete network of agents. (should this be
  //    a separate parameter?)
  // syncstart = 1: everyone has same language
  //            -1: languages rotated
  //             0: random
  std::cerr << "uniform (-1, 0 or 1), and syncstart (+1, -1, or 0)" << std::endl;
  const auto uniform = *std::istream_iterator<int>(std::cin);
  const auto syncstart = *std::istream_iterator<int>(std::cin);
  std::cout << "uniform = " << uniform << " syncstart = " << syncstart << std::endl;
  
  // Reinforcement learning parameter -- DIFFERENT
  
  std::cerr << "Provide reinforcement learning rate (e.g., 0.01)" << std::endl;
  const auto lambda = *std::istream_iterator<double>(std::cin); /* 0.01 */
  std::cout <<   "lambda = " << lambda << std::endl;

  // Inner iterations measures the fitness of the language
  // Outer iterations converges
  std::cerr << "Provide inner and outer iteration count, printinterval (e.g., " <<
    8*Agent<Agentbase>::getn()*Lexeme<Lexbase>::getn() << " " <<
    3*Agent<Agentbase>::getn()*Lexeme<Lexbase>::getn()*
    Meme<Memebase>::getn()*Meme<Memebase>::getn() << " " <<
    1 << ")" << std::endl;
  const auto inner=*std::istream_iterator<int>(std::cin),
             outer=*std::istream_iterator<int>(std::cin),
             printinterval = *std::istream_iterator<int>(std::cin);
  std::cout <<   "inner = " << inner
            << ", outer = " << outer
	    << ", printinterval = " << printinterval
	    << std::endl;


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
  BitstringMemes memes(uniform != 0?BitstringMemes():BitstringMemes(r)); // DIFFERENT: The type of memes is different in ModelA
  Lexemes lexemes(uniform != 0?Lexemes():Lexemes(r));
  Agents agents(uniform != 0?Agents():Agents(r));
  // Generate an entire population; write it out.

  // The memes currently can be defaulted in the first two cases below, but trying
  // to keep it general.  Speed at initialization is unlikely to be an issue
  
  
  Population<ReinforcementLearnerLanguage> population(uniform > 0?ReinforcementLearnerLanguage(lambda, memes):
			uniform < 0?ReinforcementLearnerLanguage(lambda, memes,unitlang((ReinforcementLearnerLanguage*)0)):
			ReinforcementLearnerLanguage(lambda, memes,r)); // DIFFERENT: The type of all the languages differs wrt model A, and so does population
   if (syncstart < 0) {
     int c=0;
     for (auto &a: population)
       a.cshift(c++);
   } else if (syncstart == 0)
     for (auto &a: population)
      a.permute(r);
  std::cout << "\t" << population;

  // For the number of outer loops, use the language for a round, and then
  // apply reinforcement learning to the resulting experience
  
  
  for (auto rounds: range(outer)) {
    if (rounds > 0 && printinterval > 0 && rounds % printinterval == 0)
      std::cout << "Round number " << rounds << std::endl
		<< "\t" << population;
		
	// DIFFERENT: In model B we don't need to keep track of the old language

    // Generate new counts and write out summary.
    auto counts=communicate_modelB(agents,lexemes,memes,population,inner); summarize(counts);

    // DIFFERENT: In model B we always mutate the lexicon
    for (auto a: indices(counts)) population[a].lexmutate(0.0,r,counts[a]); // SIGMA unused here, but we could make it more random...

  }
  std::cout << "\t" << population;
  return 0;
}


int main(int argc, char* argv[]) {
	if(argc>1 && (argv[1][0] == 'b' || argv[1][0] == 'B')) {
		std::cout << "Running model B (untested)" << std::endl;
		return runModelB();
	}
	else return runModelA();
}
	
