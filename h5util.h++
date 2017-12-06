#ifndef H5UTIL_HPP
#define H5UTIL_HPP

#include <H5Cpp.h>
#include <type_traits>
#include <utility>
#include <exception>
#include <cstring>
#include <algorithm>

#include "h5_datatype.h++"
// Needs to be done after H5Util::DataType(const char*) is defined
#include "sccs.h++"
namespace H5Util { // extend

  static const SCCS::sccs_id H5UTIL_HPP_SCCS_ID __attribute__((used)) = "@(#)h5util.h++: $Id$";

}

#include "h5_dataspace.h++"

namespace H5Util {
  template<typename T>

  template<typename T,typename U=T>
  inline void vectorwrite(const H5::DataSet &d, const std::vector<T> &v) {
    hsize_t m(0);
    const auto space = d.getSpace();
    const auto mspace = space;
    std::for_each(v.begin(), v.end(),
		  [&d,&mspace,&space,&m](const U &s) {
		    const hsize_t count[] = {1};
		    space.selectHyperslab(H5S_SELECT_SET,count, &m);
		    d.write(s,DataType(s),scalarspace(),space);
		    m++;});
  }
  template<typename T,typename U>
  inline void vectorwrite(const H5::DataSet &d, const std::vector<T> &v, const U&) {
    vectorwrite<T,U>(d,v);
  }



}

#endif
