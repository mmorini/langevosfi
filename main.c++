#include "main.h++"
#include <fstream>

static const char MAIN_CPP_SCCS_ID[] __attribute__((used)) = "@(#)main.c++: $Id$";

// This is used by Enum template in prints
extern const char memeid [] = "M";
extern const char lexid[] = "L";
extern const char agentid[] = "A";
// Define the variables holding the sizes.
template<> int Enum<memeid>::n = 0;
template<> int Enum<lexid>::n = 0;
template<> int Enum<agentid>::n = 0;

// All the classes used are defined in main_decls.h++ (which is included via main.h++)

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

  //sets import or export wish
  std::cerr << "Do you wish to import [i] or export [e] the language or neither [n]?" << std::endl; 
  std::string mode;
  std::cin >> mode;
  std::cout << "mode: " << mode << std::endl;
  
  std::string filename;

  if(mode=="e" || mode=="i"){
	std::cerr << "Provide file name " << std::endl;
	std::cin >> filename;
	std::cout << "Provided file is: " << filename << std::endl;
  }


  // language = nummemes*numlexes, population = numagents
  std::cerr << "Provide nummemes, numlexes, and numagents (e.g., 10 15 40)" << std::endl;
  Meme<Memebase>::setn(*std::istream_iterator<int>(std::cin)); /* 10 */
  Lexeme<Lexbase>::setn(*std::istream_iterator<int>(std::cin)); /* 15 */
  Agent<Agentbase>::setn(*std::istream_iterator<int>(std::cin)); /* 40 */
  std::cout <<   "nummemes  = " << Meme<Memebase>::getn()
            << ", numlexes  = " << Lexeme<Lexbase>::getn()
            << ", numagents = " << Agent<Agentbase>::getn() << std::endl;
  

  // not neccessary when language is imported?
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
  std::cerr << "Provide unsigned integers seed random number generator (e.g. 1 19)" << std::endl;
  const std::vector<unsigned int> seed_vector((std::istream_iterator<unsigned int>(std::cin)),
					      std::istream_iterator<unsigned int>(std::cin));
  std::seed_seq seeds(seed_vector.begin(), seed_vector.end());
  r.seed(seeds);
  std::cout << "Random number generator seeded with ";
  for (const auto s: seed_vector) std::cout << s << " ";
  std::cout << std::endl;
  
  //Not needed when lang is imported?
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

  std::ofstream file;
  file.open(filename);

  if(mode=="e"){
  //write initial lang to file
  file << "nummemes  = " << Meme<Memebase>::getn()
            << ", numlexes  = " << Lexeme<Lexbase>::getn()
            << ", numagents = " << Agent<Agentbase>::getn() << std::endl;
  file << "0\t" << population;
  }

  // For the number of outer loops, store the oldlanguage in a
  // temporary, mutate the language holding marginals fixed,
  // communicate, and choose the new or the old language by throwing a
  // random number.
  for (auto rounds: range(outer)) {
    if (rounds > 0 && printinterval > 0 && rounds % printinterval == 0){
      std::cout << "Round number " << rounds << std::endl
		<< "\t" << population;
      if(mode=="e")
        file << rounds << "\t" << population;

  }
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
	    std::exp(penalty*delta)) {
	  // std::move promises the oldpop and oldcounts array element
	  // won't be used any more, so steal whatever data structure you can.
	  population[a] = std::move(oldpop[a]);
	  counts[a] = std::move(oldcounts[a]);
	}
      }
    }
  }
  std::cout << "\t" << population;
  if(mode=="e")
    file << "final\t" << population;
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
		std::cout << "model = B" << std::endl;
		return runModelB();
	}
	else return runModelA();
}
	
