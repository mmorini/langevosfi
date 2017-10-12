#include "main.h++"
#include <H5Cpp.h>
#include <fstream>
#include <exception>
#include <sstream>
#include <vector>
#include <functional>
#include <regex>

namespace Enum { // extend
  // This is used by Enum template in prints
  extern const char memeid [] = "M";
  extern const char lexid[] = "L";
  extern const char agentid[] = "A";
  // Define the variables holding the sizes.
  template<> int Enum<memeid>::n = 0;
  template<> int Enum<lexid>::n = 0;
  template<> int Enum<agentid>::n = 0;
}

#include "main_decls.h++"

static const char MAIN_CPP_SCCS_ID[] __attribute__((used)) = "@(#)main.c++: $Id$";

// All the classes used are defined in main_decls.h++ (which is included via main.h++)

// communicate makes n communication attempts, and returns the
// communication success of each agent.  Currently it iterates the
// procedure: choose an agent, make it choose a meme according to the
// marginal of its language, choose a lex according to its own
// language, send it to a random neighbor, who interprets it according
// to her own (presumably different) language.

// In model A we keep track of the overall success of our interactions.
// The 'match' function tells us how successful this is, and we just keep adding the success.
// In model B we instead randomly increase / decrease the association. We interpret match
// as a number m between 0 and 1 (perfect mismatch -> perfect match).
// The signalling agent records a success w.p m^2; a failure w.p. (1-m)^2 and does nothing otherwise
// (Other rules are available, but I like this one)

typedef Enumvector::Enumvector<Agent::Agent<Agentbase>,
			       Experience::Experience<Meme::Meme<Memebase>,
						      Lex::Lexeme<Lexbase>>> Counts_t;

template<ModelType model>
Counts_t communicate_model(const Agents &agents,
		 const Lexemes &lexemes,
		 const Memes &memes,
 	         const Population<typename chooselang<model>::Language> &population,
		 const int n, const int b1, const int b2, const int b3, const int b4,
		 const int al) {
  Counts_t retval;
  for (auto rounds: SelfIterator::range(n)) {
    (void)rounds;
    const auto &a1(agents.generate(r));
    // These should be uninitialized, but interface requires initialization.
    Agent::Agent<Agentbase> a2(0);
    Meme::Meme<Memebase> m1(0);
    Lex::Lexeme<Lexbase> l1(0);
    const std::function<void(void)>
      &aupdate=[&a2,&a1,&agents]()->void{a2 = agents.neighbor(a1,r);},
      &mupdate=[&m1,&a1,&population]()->void{m1 = population[a1].memegen(r);},
      &lupdate=[&l1,&a1,&m1,&population]()->void{l1 = population[a1].lexgen(m1,r);};
    const auto& update1(al==1?aupdate:mupdate),
      update2(al==2?aupdate:al==1?mupdate:lupdate),
      update3(al==3?aupdate:lupdate);
    for (auto partners: SelfIterator::range(b1)) {
      (void) partners;
      update1();
      for (auto comms: SelfIterator::range(b2)) {
	(void) comms;
	update2();
	for (auto lexes: SelfIterator::range(b3)) {
	  (void) lexes;
	  update3();
	  for (auto trans: SelfIterator::range(b4)) {
	    (void) trans;
	    const auto &l2(population[a1].transmit(lexemes,l1,r,population[a2]));
	    const auto &m2(population[a2].memegen(l2,r));
	    switch(model) {
	    case B: {
	      const auto ran = std::generate_canonical<double, 20>(r);
	      const auto match = population[a1].match(memes,m1,m2,population[a2]);
	      // std::cout << "::communicate_modelB match->" << match << std::endl;
	      if(ran < match*match)
		retval[a1].increase_association( m1, l1, 1.0 );
	      else if( ran < 1.0 - 2.0 * match * (1.0 - match) ) 
		retval[a1].increase_association( m1, l1, -1.0 );
	      break;
	    }
	      // Make case A the default to catch case P
	    default:
	      retval[a1].increase_association( m1, l1, population[a1].match(memes,m1,m2,population[a2]) );
	      break;
	    }
	  }
	}
      }
    }
  }
  return retval;
}

class program_options final {
  std::ifstream instream_f;
  std::ofstream outstream_f;
  static void invalidusage(void) {
    usage();
    throw std::runtime_error("Invalid usage");
  }
  std::regex h5match;
public:
  bool input_from_file, input_all_from_file, output_to_file,
    instream_is_hdf5, outstream_is_hdf5;
  ModelType model;
  std::istream &instream;
  std::ostream &outstream;
  H5::H5File h5infile, h5outfile;
  program_options(const int argc, const char *const *const argv):
    h5match(".*\\.h5"),
    input_from_file(false), input_all_from_file(false), output_to_file(false),
    instream_is_hdf5(false), outstream_is_hdf5(false),
    model(A), instream(instream_f), outstream(outstream_f)
  {
    for (int i=1; i<argc; i++)
      if (argv[i] == std::string("-i"))
	if (!input_from_file && !input_all_from_file && ++i < argc) {
	  input_from_file = true;
	  if (std::regex_match(argv[i],h5match)) {
	    instream_is_hdf5 = true;
	    h5infile.openFile(argv[i],H5F_ACC_RDONLY);
	  } else {
	    instream_f.open(argv[i]);
	  }
	} else
	  invalidusage();
      else if (argv[i] == std::string("-I"))
	if (!input_from_file && !input_all_from_file && ++i < argc) {
	  input_from_file = input_all_from_file = true;
	  if (std::regex_match(argv[i],h5match)) {
	    instream_is_hdf5 = true;
	    h5infile.openFile(argv[i],H5F_ACC_RDONLY);
	  } else {
	    instream_f.open(argv[i]);
	  }
	} else
	  invalidusage();
      else if (argv[i] == std::string("-o"))
	if (!output_to_file && ++i < argc) {
	  output_to_file = true;
	  if (std::regex_match(argv[i],h5match)) {
	    outstream_is_hdf5 = true;
	    h5outfile.openFile(argv[i],H5F_ACC_EXCL);
	  } else {
	    outstream_f.open(argv[i]);
	  }
	} else
	  invalidusage();
      else if (argv[i] == std::string("-m"))
	if (++i < argc)
	  std::istringstream(argv[i]) >> model;
	else
	  invalidusage();
      else
	invalidusage();
    if (instream_is_hdf5) {
      std::cerr<<"Input hdf5 not yet supported"<<std::endl;
      invalidusage();
    }
    if (outstream_is_hdf5) {
      std::cerr<<"Output hdf5 not yet supported"<<std::endl;
      std::cerr<<"No output will be produced"<<std::endl;
    }
  }
  ~program_options() {
    if(input_from_file) {
      if(instream_is_hdf5)
	h5infile.close();
      else
	instream_f.close();
    }
    if(output_to_file) {
      if(outstream_is_hdf5)
	h5outfile.close();
      else
	outstream_f.close();
    }
  }
  static void usage(void) {
    std::cerr << "Usage: <progname> [options]" <<std::endl
              << "where options may be " << std::endl
              << "\t-i <inputlanguagefile>" << std::endl
              << "\t-I <inputallfile>" << std::endl
              << "\t-o <outputlanguagefile>" << std::endl
              << "\t-m A|B" << std::endl
              << "No option may be repeated. -i and -I cannot be used together" << std::endl;
  }
}; 

// OK now the main loop

// This is Tanmoy's original Model A

// I have to create a separate function for Model B as I don't yet understand how
// to hot swap types (e.g. between Memes and BitstringMemes) in heavily templated code
// like this.

template<ModelType model>
int runModel(const program_options& po) {

  const auto nummemes = *std::istream_iterator<int>(po.input_from_file?po.instream:std::cin),
             numlexes = *std::istream_iterator<int>(po.input_from_file?po.instream:std::cin),
             numagents = *std::istream_iterator<int>(po.input_from_file?po.instream:std::cin);

  Meme::Meme<Memebase>::setn(nummemes); 
  Lex::Lexeme<Lexbase>::setn(numlexes); 
  Agent::Agent<Agentbase>::setn(numagents); 
  std::cout <<   "nummemes  = " << Meme::Meme<Memebase>::getn()
            << ", numlexes  = " << Lex::Lexeme<Lexbase>::getn()
            << ", numagents = " << Agent::Agent<Agentbase>::getn() << std::endl;
  

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
  else if (!po.input_all_from_file)
    std::cerr << "uniform (0 or nonzero)?" << std::endl;
  const auto uniform = po.input_all_from_file?0:*std::istream_iterator<int>(std::cin);
  const auto syncstart = po.input_from_file?0:*std::istream_iterator<int>(std::cin);
  if (!po.input_all_from_file)
    std::cout << "uniform = " << uniform << " ";
  if (!po.input_from_file)
    std::cout << " syncstart = " << syncstart;
  if (!po.input_all_from_file) {
    std::cout << std::endl;
    std::getline(std::cin,ignore);
  }

  double mutrate=0, penalty=0, lambda=0; // Not quite right! Two problems: first these should be
		                         // declared const, and secondly they should be in a union
				         // which depends on model.
  switch (model) {
  case B:
    // Reinforcement learning parameter -- DIFFERENT
    
    std::cerr << "Provide reinforcement learning rate (e.g., 0.01)" << std::endl;
    lambda = *std::istream_iterator<double>(std::cin); /* 0.01 */
    std::cout <<   "lambda = " << lambda << std::endl;
    break;
  default:
    // case A is default to catch case P as well.
    // The following two parameters are effectively in the exponent, so careful
    std::cerr << "Provide mutrate and penalty (e.g., 1 100)" << std::endl;
    mutrate = *std::istream_iterator<double>(std::cin); /* 1 */
    penalty = *std::istream_iterator<double>(std::cin); /* 100 */
    std::cout <<   "mutrate = " << mutrate
	      << ", penalty = " << penalty << std::endl;
    break;
  }
  std::getline(std::cin,ignore);

  // Inner iterations measures the fitness of the language
  // Outer iterations converges
  std::cerr << "Provide inner and outer iteration count, printinterval (e.g., " <<
    8*Agent::Agent<Agentbase>::getn()*Lex::Lexeme<Lexbase>::getn() << " " <<
    3*Agent::Agent<Agentbase>::getn()*Lex::Lexeme<Lexbase>::getn()*
    Meme::Meme<Memebase>::getn()*Meme::Meme<Memebase>::getn() << " " <<
    1 << ")" << std::endl;
  const auto inner=util::clamplow(*std::istream_iterator<int>(std::cin),1),
             outer=util::clamplow(*std::istream_iterator<int>(std::cin),1),
             printinterval = util::clamplow(*std::istream_iterator<int>(std::cin),1);
  std::cout <<   "inner = " << inner
            << ", outer = " << outer
	    << ", printinterval = " << printinterval
	    << std::endl;
  std::getline(std::cin,ignore);

  std::cerr << "Provide blocking parameters for levels 1-4 (e.g., 1 1 1 1)" << std::endl;
  const auto b1 = util::clamplow(*std::istream_iterator<int>(std::cin),1),
             b2 = util::clamplow(*std::istream_iterator<int>(std::cin),1),
             b3 = util::clamplow(*std::istream_iterator<int>(std::cin),1),
             b4 = util::clamplow(*std::istream_iterator<int>(std::cin),1);
  std::cout << " Blocking: level 1 = " << b1
	    << "           level 2 = " << b2
	    << "           level 3 = " << b3
            << "           level 4 = " << b4 << std::endl;
  std::getline(std::cin,ignore);

  std::cerr << "Provide level of agent (1, 2, or 3)" << std::endl;
  const auto al = util::clamp(*std::istream_iterator<int>(std::cin),1,3);
  std::getline(std::cin,ignore);

  std::cout<<"Loop structure:"<<std::endl;
  std::cout<<"\t"<<outer<<"x {"<<inner<<" agents x[";
  std::cout<<b1<<(al==1?"agents":"memes")<<" x(";
  std::cout<<b2<<(al==2?"agents":al==1?"memes":"lexes")<<" x<";
  std::cout<<b3<<(al==3?"agents":"lexes")<<" x";
  std::cout<<b1<<"communications>)]}"<<std::endl;
  

  // Seed the random number generator. Needs a sequence of unsigned intergers
  // to generate a seed.
  std::cerr << "Provide unsigned integers and end file to seed random number generator (e.g. 1 19)" << std::endl;
  const std::vector<unsigned int> seed_vector((std::istream_iterator<unsigned int>(std::cin)),
					      std::istream_iterator<unsigned int>());
  std::seed_seq seeds(seed_vector.begin(), seed_vector.end());
  r.seed(seeds);
  std::cout << "Random number generator seeded with ";
  // std::copy(seed_vector.begin(), seed_vector.end(), std::ostream_iterator<unsigned int>(std::cout));
  for (const auto s: seed_vector) std::cout << s << " ";
  std::cout << std::endl;
  
  //Not needed when lang is imported?
  // OK, this generates a random probabilities for the network of memes.
  // Don't use random numbers in the wrong branch
  Memes pmemes(model != B?
	       po.input_all_from_file?Memes(*std::istream_iterator<Memes::Network>(po.instream)):
	       model==P?
	       uniform != 0?Memes(Memes::Network(Memes::bitset_adjacency())):Memes(Memes::Network(Memes::Probvector(r),Memes::bitset_adjacency())):
	       uniform != 0?Memes():Memes(r):
	       Memes());
  BitstringMemes bmemes(model == B?
			po.input_all_from_file?BitstringMemes(*std::istream_iterator<BitstringMemes::Network>(po.instream)):
			uniform != 0?BitstringMemes():BitstringMemes(r):
			BitstringMemes()); // DIFFERENT: The type of memes is different in ModelA
  // model A is default, catches model P
  Memes &memes(model==B?bmemes:pmemes);
  Lexemes lexemes(po.input_all_from_file?Lexemes(*std::istream_iterator<Lexemes::Network>(po.instream)):
		  uniform != 0?Lexemes():Lexemes(r));
  Agents agents(po.input_all_from_file?Agents(*std::istream_iterator<Agents::Network>(po.instream)):
		uniform != 0?Agents():Agents(r));
  // Generate an entire population; write it out.

  // The memes currently can be defaulted in the first two cases below, but trying
  // to keep it general.  Speed at initialization is unlikely to be an issue
  Population<typename chooselang<model>::Language> population(po.input_from_file?
							      Population<typename chooselang<model>::Language>
							      (std::istream_iterator<typename chooselang<model>::Language>(po.instream)):
							      Population<typename chooselang<model>::Language>
							      (chooselang<model>::langinit(uniform,lambda,memes,r)));
  if (!po.input_from_file) {
    if (syncstart < 0) {
      int c=0;
      for (auto &a: population)
	a.cshift(c++);
    } else if (syncstart == 0)
      for (auto &a: population)
	a.permute(r);
    std::cout << population;
  }

  Counts_t counts;
  if (model!=B) {
    // Initialize everybodies counts and write out summary.
    // A is default! model==B below is false, but need to mask compiler
    // trying to compile call to A in a dead branch
    counts=communicate_model<model==B?B:A>(agents,lexemes,memes,population,inner,b1,b2,b3,b4,al);
    summarize(counts);
  }

  if(po.output_to_file){
    if (po.outstream_is_hdf5)
      ; // Incomplete
    else
      //write initial lang to file
      po.outstream << "nummemes  = " << Meme::Meme<Memebase>::getn()
		   << ", numlexes  = " << Lex::Lexeme<Lexbase>::getn()
		   << ", numagents = " << Agent::Agent<Agentbase>::getn() << std::endl
		   << "Memes" << std::endl << memes
		   << "Lexemes" << std::endl << lexemes
		   << "Agents" << std::endl << agents
		   << "Initial" << std::endl << population
		   << "Counts" << std::endl << "\t" << counts;
  }

  // Model A: For the number of outer loops, store the oldlanguage in a
  // temporary, mutate the language holding marginals fixed,
  // communicate, and choose the new or the old language by throwing a
  // random number.
  // Model B: For the number of outer loops, use the language for a round, and then
  // apply reinforcement learning to the resulting experience
    
  for (auto rounds: SelfIterator::range(outer)) {
    if (rounds > 0 && printinterval > 0 && rounds % printinterval == 0)
      std::cout << "Round number " << rounds << std::endl
		<< population;
    switch(model) {
    case B: {
      // DIFFERENT: In model B we don't need to keep track of the old language
      
      // Generate new counts and write out summary.
      auto counts=communicate_model<model>(agents,lexemes,memes,population,inner,b1,b2,b3,b4,al); summarize(counts);
      
      if(po.output_to_file) {
	if (po.outstream_is_hdf5)
	  ; // Incomplete
	else
	  po.outstream << rounds << population
		       << "Counts" << std::endl << "\t" << counts;
      }
      if (rounds > 0 && printinterval > 0 && rounds % printinterval == 0)
	std::cout << "Round number " << rounds << std::endl
		  << population
		  << "Counts" << std::endl << "\t" << counts;

      // DIFFERENT: In model B we always mutate the lexicon
      for (auto a: indices(counts)) population[a].lexmutate(0.0,r,counts[a]); // SIGMA unused here, but we could make it more random...
      break;
    }
      // A is default, catches P
    default: {
      if(po.output_to_file) {
	if (po.outstream_is_hdf5)
	  ; // Incomplete
	else
	  po.outstream << rounds << population
		       << "Counts" << std::endl << "\t" << counts;
      }
      if (rounds > 0 && printinterval > 0 && rounds % printinterval == 0)
	std::cout << "Round number " << rounds << std::endl
		  << population
		  << "Counts" << std::endl << "\t" << counts;
      // Mark cache as moving to 'oldpop' in the next statement.
      for (auto &a: population) a.decache();
      auto oldpop = population;
      auto oldcounts = counts;
      
      // Mutate each language
      for (auto &a: population) a.lexmutate(mutrate,r);
      
      // Generate new counts and write out summary.
      // A is default, catching P
      // The model==B test is necessary to stop compiler trying to 
      // compile the wrong thing even in a dead branch
      counts=communicate_model<model==B?B:A>(agents,lexemes,memes,population,inner,b1,b2,b3,b4,al); summarize(counts);
      
      // Look at each agent's language
      for(auto a: SelfIterator::indices(counts)) {
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
      break; }
    }
  }
  std::cout <<  population;
  if(po.output_to_file) {
    if (po.outstream_is_hdf5)
      ; // Incomplete
    else
      po.outstream << "final" << std::endl << population;
  }
  return 0;
}


int main(int argc, char* argv[]) {
  const program_options po(argc,argv);
  
  if (!po.input_from_file)
    std::cerr << "Provide nummemes, numlexes, and numagents (e.g., 10 15 40)" << std::endl;
  
  std::cout << "model = " << po.model << std::endl;

  // Need constexpr for template choice
  return (po.model==B?runModel<B>:runModel<A>)(po);
}
