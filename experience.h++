#ifndef EXPERIENCE_HPP
#define EXPERIENCE_HPP

#include <pair>
#include <unordered_map>

#include "enumvector.h++"
#include <iostream>

// Not sure what this is; whoever does can replace it with the appropriate analog
// static const char COUNTS_HPP_SCCS_ID[] __attribute__((used)) = "@(#)counts.h++: $Id$";

/**
  * The purpose of this class is to keep a record of an agent's experience in a round.
  * Model A cares only about the success rate; Model B cares above the success of
  * each construction.
  *
  * It is not quite a drop-in replacement for Counts in Tanmoy's original code:
  * The += operator is no longer overloaded, as we want to keep track of which constructions
  * have been encountered; so we need to use the increase_association method instead.
  */

template <typename Meme, typename Lexeme> // It has to be a template cos we don't know what Meme and Lexeme are yet
class Experience {

  // Use a hashmap so we don't need to keep massive arrays hanging around unneccesarily 
  using keytype = std::pair<Meme,Lexeme>;
  std::unordered_map<keytype, double> association;
  
public:

  /* To maintain compatibility with Counts, we include these public fields... */
  double success = 0;
  int tries = 0;

  /* ...and this public function */
  double mean(void) const {return success/tries;}

  /* Increase the meme-lexeme association by a certain amount; no doubt Tanmoy would overload
     lots of operators to allow for a terser syntax */
  void increase_association(Meme& m, Lexeme& l, double by) {
  	auto key = make_pair(m,l);
  	if(association.count(key) > 0) {
  		association[pair] += by;
  	} else {
  		association[pair] = by;
  	}
  	++tries;
  	success+=by;
  }
  
};

// This gives the overall mean success rate for a bunch of experiences
// Not sure how useful this will be
template<typename T>
void summarize(const Enumvector<T,Experience> &experiences) {
  double success;
  int tries;
  for (Experience e: experiences) {
  	success += e.success;
  	tries += e.tries;
  }
  std::cout << "Comprehension " << (success/tries) << std::endl;
}

#endif
