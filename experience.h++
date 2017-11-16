#ifndef EXPERIENCE_HPP
#define EXPERIENCE_HPP

#include <utility>
#include <map>

#include "enumvector.h++"
#include <iostream>

/**
  * The purpose of this class is to keep a record of an agent's experience in a round.
  * Model A cares only about the success rate; Model B cares above the success of
  * each construction.
  *
  * It is not quite a drop-in replacement for Counts in Tanmoy's original code:
  * The += operator is no longer overloaded, as we want to keep track of which constructions
  * have been encountered; so we need to use the increase_association method instead.
  */


namespace Experience {

// Not sure what this is; whoever does can replace it with the appropriate analog
static const char EXPERIENCE_HPP_SCCS_ID[] __attribute__((used)) = "@(#)experience.h++: $Id$";

// The forward declaration of operator<< template is needed syntactically for 
// the friend declaration.  We can avoid the friend declaration with a public
// member function called from the operator template, but this is cleaner.
// Also, we could put the definition of the operator template here, but it is
// neater to put it after the class definition.
template <typename Meme, typename Lexeme> class Experience;

template <typename Meme, typename Lexeme>
std::ostream& operator<<(std::ostream&, const Experience<Meme,Lexeme>&);

template <typename Meme, typename Lexeme> // It has to be a template cos we don't know what Meme and Lexeme are yet
class Experience final {

	using keytype = std::pair<Meme,Lexeme>;
	std::map<keytype, double> association;

        friend std::ostream& operator<< <> (std::ostream&, const Experience&);
public:

	/* To maintain compatibility with Counts, we include these public fields... */
	double success = 0.0;
	int tries = 0;

	/* ...and this public function */
        constexpr double mean(void) const {return success/tries;}

	/* Increase the meme-lexeme association by a certain amount; no doubt Tanmoy would overload
	 lots of operators to allow for a terser syntax */
	void increase_association(const Meme& m, const Lexeme& l, double by) {
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
	double get_association(const Meme& m, const Lexeme& l) const {
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
        constexpr typename decltype(association)::const_iterator begin() const {
  		return association.begin();
  	}
  	
  	constexpr typename decltype(association)::const_iterator end() const  {
  		return association.end();
  	}
  static const H5::DataType& DataType(void) {
    static H5::CompType retval;
    static bool inited(false);
    if (!inited) {
      const H5std_string s("successes"), t("tries"), c("comprehension"), a("association_array");
      const H5::DataType& stype(H5Util::DataType<decltype(success)>());
      const H5::DataType& ttype(H5Util::DataType<decltype(tries)>());
      const H5::DataType& ctype(H5Util::DataType<decltype(success/tries)>());
      /*
	Guessing I got how to deal with VarLenType as part of a CompoundType.
      */
      const H5::DataType& mtype(H5Util::DataType<decltype(association.begin()->first.first)>());
      const H5::DataType& ltype(H5Util::DataType<decltype(association.begin()->first.second)>());
      const H5::DataType& vtype(H5Util::DataType<decltype(association.begin()->second)>());
      const H5std_string m("meme"), l("lexeme"), v("association");
      size_t offset(vtype.getSize()+ltype.getSize()+mtype.getSize());
      static H5::CompType assoctype(offset);
      assoctype.insertMember(v,offset-vtype.getSize(),vtype);
      assoctype.insertMember(l,offset-ltype.getSize(),ltype);
      assoctype.insertMember(m,offset-mtype.getSize(),mtype);
      const H5::VarLenType atype(&assoctype);

      offset = stype.getSize()+ttype.getSize()+ctype.getSize()+atype.getSize();
      // atype.getSize() == sizeof(H5::hvl_t)?
      retval = H5::CompType(offset);
      retval.insertMember(c,offset-=atype.getSize(),atype);
      retval.insertMember(c,offset-=ctype.getSize(),ctype);
      retval.insertMember(t,offset-=ttype.getSize(),ttype);
      retval.insertMember(s,offset-=stype.getSize(),stype);
      inited = true;
    }
    return retval;
  } 
      
			      
  	  
};

// This gives the overall mean success rate for a bunch of experiences
// Not sure how useful this will be
template<typename T, typename Experience>
void summarize(const Enumvector::Enumvector<T,Experience> &experiences) {
	double success = 0.0;
	int tries = 0;
	for (Experience e: experiences) {
		success += e.success;
		tries += e.tries;
	}
	std::cout << "[comprehension]\t" << (success/tries) << std::endl;
}

template <typename Meme, typename Lexeme>
std::ostream& operator<<(std::ostream& o, const Experience<Meme,Lexeme>& e) {
  o << "[counts]\t\t" <<e.success << "/" << e.tries << std::endl;
  for (auto cit: e.association)
    o << "[counts]\t" << cit.first.first << " " << cit.first.second << " " << cit.second << std::endl;
  return o;
}

}

#endif