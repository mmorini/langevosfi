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

  template<bool b,typename T = void>
  struct ConvertedType {
    inline operator bool() const {return b;}
    typedef T type;
  };

  declare_member_check(has_DirectType,DirectType)

  template<typename T>
  ConvertedType<false> DirectType(const volatile T* =nullptr) {return ConvertedType<false>();}
  ConvertedType<false,H5std_string> DirectType(const H5std_string& =H5std_string()) {return  ConvertedType<false,H5std_string>();}
  ConvertedType<false,H5std_string> DirectType(const char * =nullptr) {return ConvertedType<false,H5std_string>();}
  template<typename T, typename std::enable_if<decltype(has_DirectType(static_cast<T*>(nullptr)))::value,int>::type = 0> 
  decltype(T::DirectType()) DirectType(const T& =std::declval<T>()) {return T::DirectType();}
  template<typename T, typename std::enable_if<std::is_fundamental<T>::value &&
					       !std::is_pointer<T>::value,int>::type = 0>
  ConvertedType<true> DirectType(const T& =std::declval<T>()) {return ConvertedType<true>();}
  template<typename T, typename std::enable_if<!decltype(has_DirectType(static_cast<T*>(nullptr)))::value &&
					       decltype(util::has_base_template<std::vector>(static_cast<T*>(nullptr)))::value,int>::type = 0>
  decltype(DirectType(std::declval<typename T::value_type>())) DirectType(const T& = std::declval<T>()) {return DirectType(typename T::value_type());}


  declare_member_check(has_memDataType,memDataType)

  template<typename T,
	   typename std::enable_if<decltype(has_memDataType(static_cast<T*>(nullptr)))::value,int>::type=0>
  inline decltype(std::declval<T>().memDataType()) memDataType(const T& v) {
    return v.memDataType();
  }
  
  template<typename T,
	   typename std::enable_if<!decltype(has_memDataType(static_cast<T*>(nullptr)))::value,int>::type=0>
  inline decltype(DataType(std::declval<const T&>())) memDataType(const T& v) {
    return DataType(v);
  }
  

  template<typename T>
  struct h5iospec {
    H5::DataType fDataType, mDataType;
    H5::DataSpace fDataSpace, mDataSpace;
    decltype(h5pack(std::declval<const T>())) Data;
    h5iospec(const T& d): fDataType(DataType(d)), mDataType(memDataType(d)),
			  fDataSpace(choosespace(h5dims(d))),
			  mDataSpace(fDataSpace),
			  Data(h5pack(d)) {/* can't do it here, only compType has 
					      pack; fDataType.pack(); */}
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
  inline void vectorwrite(const H5::DataSet &d, const std::vector<T> &v) {
    const auto dtype(DataType(v));
    const auto dostream(H5Util::DirectType(v));
    if (dostream) {
      d.write(v.data(),dtype); // d.write(v.data(),dtype,mspace,space);
    } else {
      vectorwrite(d,v,typename decltype(dostream)::type());
    }
  }

  template<typename T, 
	   typename U=typename decltype(h5pack(std::declval<typename T::value_type>()))::value_type,
	   typename std::enable_if<!std::is_object<decltype(std::declval<T>().h5pack())>::value,int>::type = 0>
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
