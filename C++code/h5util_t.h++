#ifndef H5UTIL_T_HPP
#define H5UTIL_T_HPP
#include "sccs.h++"
namespace H5Util {

  static const SCCS::sccs_id H5UTIL_T_HPP_SCCS_ID __attribute__((used)) = "@(#)h5util_t.h++: $Id$";

  template<bool b,typename T = void>
  struct ConvertedType {
    inline operator bool() const {return b;}
    typedef T type;
  };
}
#endif
