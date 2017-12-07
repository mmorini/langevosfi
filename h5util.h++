#ifndef H5UTIL_HPP
#define H5UTIL_HPP

#include <H5Cpp.h>
#include <type_traits>
#include <utility>
#include <exception>
#include <cstring>
#include <algorithm>
#include "myutil.h++"

#include "h5_datatype.h++"
// Needs to be done after H5Util::DataType(const char*) is defined
#include "sccs.h++"
namespace H5Util { // extend

  static const SCCS::sccs_id H5UTIL_HPP_SCCS_ID __attribute__((used)) = "@(#)h5util.h++: $Id$";

}

#include "h5_dataspace.h++"

namespace H5Util {

  declare_member_check(has_memDataType,memDataType)

  template<typename T,
	   typename std::enable_if<decltype(has_memDataType(std::declval<T*>())):value,int>::type=0>
  inline decltype(std::declval<T>().memDataType()) memDataType(const T& v) {
    return v.memDataType();
  }
  
  template<typename T,
	   typename std::enable_if<!decltype(has_memDataType(std::declval<T*>()))::value,int>::type=0>
  inline declType(DataType(std::declval<const T&>())) DataType memDataType(const T& v) {
    return DataType(v);
  }
  

  template<typename T>
  struct h5iospec {
    H5::DataType fDataType, mDataType;
    H5::DataSpace fDataSpace, mDataSpace;
    decltype(h5pack(declval<const T>())) Data;
    template<typename T>
    h5iospec(const T& d): fDataType(DataType(d)), mDataType(memDataType(d)), fDataSpace(choosespace(h5dims(d))), mDataSpace(fdataspace),
			    Data(h5pack(d)) {fDataType.pack();}
  };


  // write a 1-d vector with type conversion
  template<typename T,typename U>
  inline void vectorwrite(const H5::DataSet &d, const std::vector<T> &v, const U&) {
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

  // general write function
  template<typename T>
  inline void vectorwrite(const H5::Dataset &d, const std::vector<T> &v) {
    const auto space(d.getSpace());
    const std::vector<hsize_t> sz(1,v.size());
    const auto mspace(choosespace(sz));
    const auto dtype(DataType(v));
    d.write(v.data(),dtype,mspace,space);
  }

  template<typename T, 
	   typename U=typename decltype(h5pack(declval<typename T::value_type>()))::value_type,
	   typename std::enable_if<!is_object<decltype(declval<T>().h5pack())>::value,int>::type = 0>
  inline std::vector<U> h5pack(const T& o) {
    std::vector<U> retval;
    for(const auto &e: o) {
      const auto ev(h5pack(e));
      retval.insert(retval.end(),ev.begin(),ev.end());
    }
    return retval; 
  }

}

#endif
