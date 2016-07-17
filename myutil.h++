#ifndef MYUTIL_HPP
#define MYUTIL_HPP

#include <cmath>

static const char MYUTIL_HPP_SCCS_ID[] __attribute__((used)) = "@(#)myutil.h++: $Id$";

const double infinity=1.e100;
inline auto probit(const double p) {
  return p<=0.0?-infinity:p>=1.0?infinity:log(p/(1-p));
}
inline auto invprobit(const double probit) {
  return probit<=-infinity?0:probit>=infinity?1:
    1/(1+exp(-probit));
}
#endif
