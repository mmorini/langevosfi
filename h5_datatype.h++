#ifndef H5_DATATYPE_HPP
#define H5_DATATYPE_HPP

#include <H5Cpp.h>
#include <type_traits>
#include <cstring>

namespace H5Util {

  template<typename T, typename std::enable_if<std::is_constructible<const H5::DataType&,decltype(T::DataType())>::value,int>::type = 0> 
  inline  const H5::DataType& DataType(const T& =std::declval<T>()) {
    return T::DataType();
  }
  template<typename T, typename std::enable_if<std::is_fundamental<T>::value,int>::type = 0>
  inline  const H5::DataType& DataType(const T& =std::declval<T>()) {
    return H5::PredType::NATIVE_OPAQUE;
  }
  template<> inline  const H5::DataType& DataType(const char&) {
    return H5::PredType::NATIVE_CHAR;
  }
  template<> inline  const H5::DataType& DataType(const signed char&) {
    return H5::PredType::NATIVE_SCHAR;
  }
  template<> inline  const H5::DataType& DataType(const unsigned char&) {
    return H5::PredType::NATIVE_UCHAR;
  }
  template<> inline  const H5::DataType& DataType(const short&) {
    return H5::PredType::NATIVE_SHORT;
  }
  template<> inline  const H5::DataType& DataType(const unsigned short&) {
    return H5::PredType::NATIVE_USHORT;
  }
  template<> inline  const H5::DataType& DataType(const int&) {
    return H5::PredType::NATIVE_INT;
  }
  template<> inline const H5::DataType& DataType(const bool&) {
    static_assert(sizeof(bool)==sizeof(char),"Code assumes bool is a char");
    return H5::PredType::NATIVE_CHAR;
  }
  template<> inline  const H5::DataType& DataType(const unsigned int&) {
    return H5::PredType::NATIVE_UINT;
  }
  template<> inline  const H5::DataType& DataType(const long&) {
    return H5::PredType::NATIVE_LONG;
  }
  template<> inline  const H5::DataType& DataType(const unsigned long&) {
    return H5::PredType::NATIVE_ULONG;
  }
  template<> inline  const H5::DataType& DataType(const long long&) {
    return H5::PredType::NATIVE_LLONG;
  }
  template<> inline  const H5::DataType& DataType(const unsigned long long&) {
    return H5::PredType::NATIVE_ULLONG;
  }
  template<> inline  const H5::DataType& DataType(const float&) {
    return H5::PredType::NATIVE_FLOAT;
  }
  template<> inline  const H5::DataType& DataType(const double&) {
    return H5::PredType::NATIVE_DOUBLE;
  }
  template<> inline  const H5::DataType& DataType(const long double&) {
    return H5::PredType::NATIVE_LDOUBLE;
  }
  inline const H5::DataType& DataType(const H5std_string&) {
    // 0 is Dummy for H5::StrType() or H5::PredType::C_S1
    static auto retval(H5::StrType(0,H5T_VARIABLE));
    return retval;
  }
  template<typename T>
  inline H5::DataType DataType(const std::vector<T>& v) {
    return DataType(T());
  }
  inline H5::DataType DataType(const char *s) {
    // 0 is Dummy for H5::StrType() or H5::PredType::C_S1
    return H5::StrType(0,std::strlen(s)+1);
  }

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
  

}
// Needs to be done after H5Util::DataType(const char*) is defined
#include "sccs.h++"
namespace H5Util { // extend
  static const SCCS::sccs_id H5_DATATYPE_HPP_SCCS_ID __attribute__((used)) = "@(#)h5_datatype.h++: $Id$";
}

#endif
