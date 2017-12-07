#ifndef SCCS_HPP
#define SCCS_HPP

#include <vector>
#include <string>
#include <iostream>
#include <cstring>

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
    static inline const H5::DataType& DataType(void);
  };
}

// Any include needs to be able to see a complete sccs_id class.
#include "h5_datatype.h++"

namespace SCCS { // extend

  inline const H5::DataType& sccs_id::DataType(void) {
    return H5Util::DataType(std::string());
  }

  static const sccs_id SCCS_HPP_SCCS_ID __attribute__((used)) = "@(#)sccs.h++: $Id$";

}
#endif
