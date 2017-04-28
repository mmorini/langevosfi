#ifndef EXPERIENCE_HPP
#define EXPERIENCE_HPP

#include <utility>
#include <map>

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

	using keytype = std::pair<Meme,Lexeme>;
	std::map<keytype, double> association;
  
public:

	/* To maintain compatibility with Counts, we include these public fields... */
	double success = 0;
	int tries = 0;

	/* ...and this public function */
	double mean(void) const {return success/tries;}

	/* Increase the meme-lexeme association by a certain amount; no doubt Tanmoy would overload
	 lots of operators to allow for a terser syntax */
	void increase_association(Meme& m, Lexeme& l, double by) {
		auto key = std::make_pair(m,l);
		if(association.count(key) > 0) {
			association[key] += by;
		} else {
			association[key] = by;
		}
		++tries;
		success+=by;
	}
  
	/* Accessing the underlying experience. We can do this in two ways; either query
	 * a specific Meme,Lexeme pair... (We do this the Java way with a getter rather than the [] operator which can only take one arg)
	 */
	double get_association(Meme& m, Lexeme& l) {
		auto key = std::make_pair(m,l);
		return association.count(key) > 0 ? association[key] : 0.0;
	}   		
  
  	/* ... or with an iterator 
  	 * for now I do the laziest thing which is to expose the underlying map iterator. We could
  	 * write our own custom iterator to provide a tuple <Meme,Lexeme,double> but time is short.
  	 *
  	 * We access the Meme, Lexeme and association as iterator.first.first, iterator.first.second and iterator.second
  	 * respectively 
  	 **/
  	typename decltype(association)::const_iterator begin() const {
  		return association.begin();
  	}
  	
  	typename decltype(association)::const_iterator end() const  {
  		return association.end();
  	}
  	  
};

// This gives the overall mean success rate for a bunch of experiences
// Not sure how useful this will be
template<typename T, typename Experience>
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
