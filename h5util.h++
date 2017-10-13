#ifndef H5UTIL_HPP
#define H5UTIL_HPP

#include <H5Cpp.h>
#include <type_traits>
#include <exception>

namespace H5Util {

static const char H5UTIL_HPP_SCCS_ID[] __attribute__((used)) = "@(#)h5util.h++: $Id$";

  template<typename T, typename std::enable_if<std::is_constructible<const H5::DataType&,decltype(T::H5DataType())>::value,int>::type = 0> 
  inline  const H5::DataType& H5DataType() {
    return T::H5DataType();
  }
  template<typename T, typename std::enable_if<std::is_fundamental<T>::value,int>::type = 0>
  inline  const H5::DataType& H5DataType() {
    // throw std::runtime_error("Unknown H5 Data Type");
    return H5::PredType::NATIVE_OPAQUE;
  }
  template<> inline  const H5::DataType& H5DataType<char>() {
    return H5::PredType::NATIVE_CHAR;
  }
  template<> inline  const H5::DataType& H5DataType<signed char>() {
    return H5::PredType::NATIVE_SCHAR;
  }
  template<> inline  const H5::DataType& H5DataType<unsigned char>() {
    return H5::PredType::NATIVE_UCHAR;
  }
  template<> inline  const H5::DataType& H5DataType<short>() {
    return H5::PredType::NATIVE_SHORT;
  }
  template<> inline  const H5::DataType& H5DataType<unsigned short>() {
    return H5::PredType::NATIVE_USHORT;
  }
  template<> inline  const H5::DataType& H5DataType<int>() {
    return H5::PredType::NATIVE_INT;
  }
  template<> inline  const H5::DataType& H5DataType<unsigned int>() {
    return H5::PredType::NATIVE_UINT;
  }
  template<> inline  const H5::DataType& H5DataType<long>() {
    return H5::PredType::NATIVE_LONG;
  }
  template<> inline  const H5::DataType& H5DataType<unsigned long>() {
    return H5::PredType::NATIVE_ULONG;
  }
  template<> inline  const H5::DataType& H5DataType<long long>() {
    return H5::PredType::NATIVE_LLONG;
  }
  template<> inline  const H5::DataType& H5DataType<unsigned long long>() {
    return H5::PredType::NATIVE_ULLONG;
  }
  template<> inline  const H5::DataType& H5DataType<float>() {
    return H5::PredType::NATIVE_FLOAT;
  }
  template<> inline  const H5::DataType& H5DataType<double>() {
    return H5::PredType::NATIVE_DOUBLE;
  }
  template<> inline  const H5::DataType& H5DataType<long double>() {
    return H5::PredType::NATIVE_LDOUBLE;
  }

}
#endif
