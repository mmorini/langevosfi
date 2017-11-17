#ifndef SCCS_HPP
#define SCCS_HPP

#include <vector>
#include <string>
#include <iostream>
#include <cstring>

namespace SCCS {

  class sccs_id {
    static const char* idstore(const char*id=0) {
      static std::vector<const char*> allids;
      if (!id)  {
	std::string retval;
	for (const auto x: allids) retval += x + std::string("\n");
	return retval.c_str();
      } else {
	allids.push_back(id);
	return id;
      }
    }
    const char *id=0;
  public:
    sccs_id(const char *id): id(idstore(id?id:"")) {}
    operator const char* () const {return id;}
    static const char* getallids() {return idstore();}
    static const H5::DataType& DataType(void) {
      static H5::DataType retval;
      static bool inited(false);
      if (!inited) {
	// 0 is Dummy for H5::StrType() or H5::PredType::C_S1
	retval = H5::StrType(0,std::strlen(getallids())+1);
	inited = true;
      }
      return retval;
    }
  };

  static const sccs_id SCCS_HPP_SCCS_ID __attribute__((used)) = "@(#)sccs.h++: $Id$";

}
#endif
