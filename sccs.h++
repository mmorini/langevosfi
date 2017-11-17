#ifndef SCCS_HPP
#define SCCS_HPP

#include <vector>
#include <string>
#include <iostream>
#include <cstring>
#include "h5util.h++"

namespace SCCS {

  class sccs_id {
    static std::vector<const char*>& idstore(const char*id=0) {
      static std::vector<const char*> allids;
      if (id)
	allids.push_back(id);
      return allids;
    }
    const char *id=0;
  public:
    sccs_id(const char *id): id(idstore(id?id:"").back()) {}
    operator const char* () const {return id;}
    static std::vector<const char*> getallids() {return idstore();}
    static const H5::DataType& DataType(void) {
      return H5Util::DataType(std::string());
    }
  };

  static const sccs_id SCCS_HPP_SCCS_ID __attribute__((used)) = "@(#)sccs.h++: $Id$";

}
#endif
