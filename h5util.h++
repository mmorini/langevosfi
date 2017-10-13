#ifndef H5UTIL_HPP
#define H5UTIL_HPP

#include <H5Cpp.h>
#include <type_traits>
#include <utility>
#include <exception>

namespace H5Util {

static const char H5UTIL_HPP_SCCS_ID[] __attribute__((used)) = "@(#)h5util.h++: $Id$";

  template<typename T, typename std::enable_if<std::is_constructible<const H5::DataType&,decltype(T::DataType())>::value,int>::type = 0> 
  inline  const H5::DataType& DataType(const T& =std::declval<T>()) {
    return T::DataType();
  }
  template<typename T, typename std::enable_if<std::is_fundamental<T>::value,int>::type = 0>
  inline  const H5::DataType& DataType(const T& =std::declval<T>()) {
    // throw std::runtime_error("Unknown H5 Data Type");
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

}
#endif
