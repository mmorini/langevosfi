#include "main.h++"
#include <fstream>

static const char MAIN_CPP_SCCS_ID[] __attribute__((used)) = "@(#)main.c++: $Id$";

// This is used by Enum template in prints
extern constexpr const char memeid [] = "M";
extern constexpr const char lexid[] = "L";
extern constexpr const char agentid[] = "A";
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
  
  std::ofstream outfile;
  outfile.open(filename);

  std::ifstream infile;
  infile.open(filename);


  // language = nummemes*numlexes, population = numagents
  if(mode != "i"){
    std::cerr << "Provide nummemes, numlexes, and numagents (e.g., 10 15 40)" << std::endl;
    Meme<Memebase>::setn(*std::istream_iterator<int>(std::cin)); /* 10 */
    Lexeme<Lexbase>::setn(*std::istream_iterator<int>(std::cin)); /* 15 */
    Agent<Agentbase>::setn(*std::istream_iterator<int>(std::cin)); /* 40 */
    std::cout <<   "nummemes  = " << Meme<Memebase>::getn()
              << ", numlexes  = " << Lexeme<Lexbase>::getn()
              << ", numagents = " << Agent<Agentbase>::getn() << std::endl;
  }
  else{
    
  }

  // uniform = 1: completely ambiguous language
  //          -1: no synonymy
  //           0: random
  // uniform != 0 also sets meaning marginals = uniform, and
  //    equal-weight complete network of agents. (should this be
  //    a separate parameter?)
  // syncstart = 1: everyone has same language
  //            -1: languages rotated
  //             0: random
auto uniform = -1;
auto syncstart = -1;
  if(mode!="i"){
    std::cerr << "uniform (-1, 0 or 1), and syncstart (+1, -1, or 0)" << std::endl;
    uniform = *std::istream_iterator<int>(std::cin);
    syncstart = *std::istream_iterator<int>(std::cin);
    std::cout << "uniform = " << uniform << " syncstart = " << syncstart << std::endl;
  }
  
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
  Memes memes();
  Lexemes lexemes();
  Agents agents();
  if(mode != "i"){
    memes(uniform != 0?Memes():Memes(r));
    lexemes(uniform != 0?Lexemes():Lexemes(r));
    agents(uniform != 0?Agents():Agents(r));
  }
    // Generate an entire population; write it out.

    // The memes currently can be defaulted in the first two cases below, but trying
    // to keep it general.  Speed at initialization is unlikely to be an issue
    Population population(uniform > 0?AgentLanguage(memes):
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
  if(mode=="i"){
    population((std::istream_iterator<AgentLanguage>(infile)));
  }
  // Initialize everybodies counts and write out summary.
  auto counts=communicate(agents,lexemes,memes,population,inner);
  summarize(counts);


  if(mode=="e"){
  //write initial lang to file
  outfile << Meme<Memebase>::getn()
            << " " << Lexeme<Lexbase>::getn()
            << " " << Agent<Agentbase>::getn() << std::endl;
  outfile << "\t" << population;
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
        outfile << rounds << "\t" << population;

  }
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
    outfile << "final\t" << population;

  outfile.close();
  infile.close();
  return 0;
}
