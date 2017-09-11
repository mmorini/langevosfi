#include "main.h++"
#include <fstream>
#include <exception>
// #include <sstream>
#include <vector>

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

class program_options {
  std::ifstream instream_f;
  std::ofstream outstream_f;
  static void invalidusage(void) {
    usage();
    throw std::runtime_error("Invalid usage");
  }
public:
  bool input_from_file, input_all_from_file, output_to_file;
  std::istream &instream;
  std::ostream &outstream;
  program_options(const int argc, const char *const *const argv):
    input_from_file(false), input_all_from_file(false), output_to_file(false),
    instream(instream_f), outstream(outstream_f)
  {
    for (int i=1; i<argc; i++)
      if (argv[i] == std::string("-i"))
	if (!input_from_file && !input_all_from_file && ++i < argc) {
	  input_from_file = true;
	  // instream_f = std::ifstream(argv[i]);
	  instream_f.open(argv[i]);
	} else
	  invalidusage();
      else if (argv[i] == std::string("-I"))
	if (!input_from_file && !input_all_from_file && ++i < argc) {
	  input_from_file = input_all_from_file = true;
	  // instream_f = std::ifstream(argv[i]);
	  instream_f.open(argv[i]);
	} else
	  invalidusage();
      else if (argv[i] == std::string("-o"))
	if (!output_to_file && ++i < argc) {
	  output_to_file = true;
	  outstream_f.open(argv[i]);
	} else
	  invalidusage();
      else
	invalidusage();
  }
  ~program_options() {
    if(input_from_file) instream_f.close();
    if(output_to_file) outstream_f.close();
  }
  static void usage(void) {
    std::cerr << "Usage: <progname> [options]" <<std::endl
              << "where options may be " << std::endl
              << "\t-i <inputlanguagefile>" << std::endl
              << "\t-I <inputallfile>" << std::endl
              << "\t-o <outputlanguagefile>" << std::endl
              << "No option may be repeated. -i and -I cannot be used together" << std::endl;
  }
}; 

// OK now the main loop
int main(const int argc, char **const argv) {

  const program_options po(argc,argv);

  if (!po.input_from_file)
    std::cerr << "Provide nummemes, numlexes, and numagents (e.g., 10 15 40)" << std::endl;
  
  const auto nummemes = *std::istream_iterator<int>(po.input_from_file?po.instream:std::cin),
             numlexes = *std::istream_iterator<int>(po.input_from_file?po.instream:std::cin),
             numagents = *std::istream_iterator<int>(po.input_from_file?po.instream:std::cin);

  Meme<Memebase>::setn(nummemes); 
  Lexeme<Lexbase>::setn(numlexes); 
  Agent<Agentbase>::setn(numagents); 
  std::cout <<   "nummemes  = " << Meme<Memebase>::getn()
            << ", numlexes  = " << Lexeme<Lexbase>::getn()
            << ", numagents = " << Agent<Agentbase>::getn() << std::endl;
  

  std::string ignore;

  // uniform = 1: completely ambiguous language
  //          -1: no synonymy
  //           0: random
  // uniform != 0 also sets meaning marginals = uniform, and
  //    equal-weight complete network of agents. (should this be
  //    a separate parameter?)
  // syncstart = 1: everyone has same language
  //            -1: languages rotated
  //             0: random
  if (!po.input_from_file)
    std::cerr << "uniform (-1, 0 or 1), and syncstart (+1, -1, or 0)" << std::endl;
  else
    std::cerr << "uniform (0 or nonzero)?" << std::endl;
  const auto uniform = *std::istream_iterator<int>(std::cin);
  const auto syncstart = po.input_from_file?0:*std::istream_iterator<int>(std::cin);
  if (!po.input_from_file)
  std::cout << "uniform = " << uniform << " syncstart = " << syncstart << std::endl;
  std::getline(std::cin,ignore);
  
  // The following two parameters are effectively in the exponent, so be careful
  std::cerr << "Provide mutrate and penalty (e.g., 1 100)" << std::endl;
  const auto mutrate = *std::istream_iterator<double>(std::cin); /* 1 */
  const auto penalty = *std::istream_iterator<double>(std::cin); /* 100 */
  std::cout <<   "mutrate = " << mutrate
            << ", penalty = " << penalty << std::endl;
  std::getline(std::cin,ignore);

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
  std::getline(std::cin,ignore);

  // Seed the random number generator. Needs a sequence of unsigned intergers
  // to generate a seed.
  std::cerr << "Provide unsigned integers seed random number generator (e.g. 1 19)" << std::endl;
  const std::vector<unsigned int> seed_vector((std::istream_iterator<unsigned int>(std::cin)),
					      std::istream_iterator<unsigned int>());
  std::seed_seq seeds(seed_vector.begin(), seed_vector.end());
  r.seed(seeds);
  std::cout << "Random number generator seeded with ";
  std::copy(seed_vector.begin(), seed_vector.end(), std::ostream_iterator<unsigned int>(std::cout));
  // for (const auto s: seed_vector) std::cout << s << " ";
  std::cout << std::endl;
  

  //Not needed when lang is imported?
  // OK, this generates a random probabilities for the network of memes.
  Memes memes(po.input_all_from_file?Memes(*std::istream_iterator<Memes::Network>(po.instream)):
	      uniform != 0?Memes():Memes(r));
  Lexemes lexemes(po.input_all_from_file?Lexemes(*std::istream_iterator<Lexemes::Network>(po.instream)):
		  uniform != 0?Lexemes():Lexemes(r));
  Agents agents(po.input_all_from_file?Agents(*std::istream_iterator<Agents::Network>(po.instream)):
		uniform != 0?Agents():Agents(r));
  // Generate an entire population; write it out.

  // The memes currently can be defaulted in the first two cases below, but trying
  // to keep it general.  Speed at initialization is unlikely to be an issue
  Population population(po.input_from_file?Population(std::istream_iterator<AgentLanguage>(po.instream)):
			Population(uniform > 0?AgentLanguage(memes):
				   uniform < 0?AgentLanguage(memes,unitlang((AgentLanguage*)0)):
				   AgentLanguage(memes,r)));
  if (!po.input_from_file) {
    if (syncstart < 0) {
      int c=0;
      for (auto &a: population)
	a.cshift(c++);
    } else if (syncstart == 0)
      for (auto &a: population)
	a.permute(r);
    std::cout << "\t" << population;
  }
  
  // Initialize everybodies counts and write out summary.
  auto counts=communicate(agents,lexemes,memes,population,inner);
  summarize(counts);

  if(po.output_to_file){
    //write initial lang to file
    po.outstream << "nummemes  = " << Meme<Memebase>::getn()
		 << ", numlexes  = " << Lexeme<Lexbase>::getn()
		 << ", numagents = " << Agent<Agentbase>::getn() << std::endl
                 << "Memes" << std::endl << "\t" << memes
                 << "Lexemes" << std::endl << "\t" << lexemes
                 << "Agents" << std::endl << "\t" << agents
		 << "Initial" << std::endl << "\t" << population
                 << "Counts" << std::endl << "\t" << counts;
  }

  // For the number of outer loops, store the oldlanguage in a
  // temporary, mutate the language holding marginals fixed,
  // communicate, and choose the new or the old language by throwing a
  // random number.
  for (auto rounds: range(outer)) {
    if(po.output_to_file)
      po.outstream << rounds << "\t" << population
		   << "Counts" << std::endl << "\t" << counts;
    if (rounds > 0 && printinterval > 0 && rounds % printinterval == 0)
	std::cout << "Round number " << rounds << std::endl
		  << "\t" << population
		  << "Counts" << std::endl << "\t" << counts;
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
  if(po.output_to_file)
    po.outstream << "final" << std::endl << "\t" << population;
  return 0;
}
