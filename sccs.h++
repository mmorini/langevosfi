#ifndef SCCS_HPP
#define SCCS_HPP

#include <vector>
#include <string>
#include <iostream>

namespace SCCS {

  class sccs_id {
    static std::vector<const char*> *allids; // Can't be a type with a nontrivial constructor
    const char *id=0;
  public:
    sccs_id(const char *id): id(id) {if(!allids) allids = new std::remove_reference<decltype(*allids)>::type;
                                     allids->push_back(id);}
    operator const char* () const {return id;}
    static std::string getallids() {
      std::string retval;
      for (const auto x: *allids) retval += x + std::string("\n");
      return retval;
    }
  };

  static const sccs_id SCCS_HPP_SCCS_ID __attribute__((used)) = "@(#)sccs.h++: $Id$";

}
#endif
