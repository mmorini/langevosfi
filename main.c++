#include "main.h++"
#include <fstream>
#include <sstream>
#include <vector>
// NANCY: need the following if you use atoi (compilers may not give an error)
// #include <cstdlib>  

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
Enumvector<Agent<Agentbase>,Counts> communicate(const Agents &agents,
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
  
  std::string filename = "";
  std::string line = "";

  if(mode=="e" || mode=="i"){
	std::cerr << "Provide file name " << std::endl;
	std::cin >> filename;
	std::cout << "Provided file is: " << filename << std::endl;
  }
  
  std::ifstream in (filename);
  
  int nummemes = 0;
  int numlexes = 0;
  int numagents = 0;

  //if no input file, read from command line
  if(mode != "i"){
  // language = nummemes*numlexes, population = numagents
  std::cerr << "Provide nummemes, numlexes, and numagents (e.g., 10 15 40)" << std::endl;

  nummemes = *std::istream_iterator<int>(std::cin);
  numlexes = *std::istream_iterator<int>(std::cin);
  numagents = *std::istream_iterator<int>(std::cin);
  }

  else{
  //read from file DOES NOT CHECK IF FILE IS EMPTY!!!
  // first line contains nummemes, numlexes, and numagents white space separated
  getline(in, line);
  // std::vector <std::string> linesplit; // NANCY see later
  // std::string buffer; // NANCY see later
  std::stringstream ss(line);

  // NANCY: see the commented section here to see how to make the
  // stuff you were trying to do work, but there is a much easier way
  // after the commented section.

  // while(ss >> buffer) {
  //   linesplit.push_back(buffer);
  // }

  // NANCY: The following compiles. std:: is the safe thing to do before atoi
  //        linesplit.pop_back() does not return the popped element, it just
  //        deletes it. (You had also forgotten the (), so you were naming the
  //        function, not calling it.)
  //        linesplit.back() does give you the last element, but it does not
  //        pop it.  It also gives you a std::string, but atoi, which is 
  //        a C function does not know how to deal with that. the c_str()
  //        converts it to const char *
  // nummemes = std::atoi(linesplit.back().c_str()); linesplit.pop_back();
  //numlexes =
  //numagents = 

  // NANCY: Here is the simple way to do it.
  ss >> nummemes >> numlexes >> numagents;
  // NANCY: or you can do this if you wanted the initializer syntax.
  // nummemes = *std::istream_iterator<int>(ss);
  // numlexes = *std::istream_iterator<int>(ss);
  // numagents = *std::istream_iterator<int>(ss);

  }
  Meme<Memebase>::setn(nummemes); /* 10 */
  Lexeme<Lexbase>::setn(numlexes); /* 15 */
  Agent<Agentbase>::setn(numagents); /* 40 */
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
					      std::istream_iterator<unsigned int>());
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



  // Initialize everybodies counts and write out summary.
  auto counts=communicate(agents,lexemes,memes,population,inner);
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
    file << "final\t" << population;
  return 0;
}
