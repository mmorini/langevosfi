#ifndef MYUTIL_HPP
#define MYUTIL_HPP

#include <cmath>
#include <random>

static const char MYUTIL_HPP_SCCS_ID[] __attribute__((used)) = "@(#)myutil.h++: $Id$";

constexpr const double infinity=1.e100;
inline auto probit(const double p) {
  return p<=0.0?-infinity:p>=1.0?infinity:log(p/(1-p));
}
inline auto invprobit(const double probit) {
  return probit<=-infinity?0:probit>=infinity?1:
    1/(1+exp(-probit));
}
template<typename generator>
double BoxMueller(const double mu, const double sigma, generator &r)
{
// Box Mueller transform in polar form
  const double epsilon = pow(2.,-20);

  static double z0, z1;
  static bool generate = false;
  generate = !generate;

  if (!generate)
    return z1 * sigma + mu;

  double u1, u2, s;
  do
    {
      u1 = 2*std::generate_canonical<double,20>(r)-1;
      u2 = 2*std::generate_canonical<double,20>(r)-1;
      s = u1*u1 + u2*u2;
    }
  while ( s <= epsilon || s >= 1 );

  const auto m = sqrt(-2*log(s)/s);

  z0 = u1 * m;
  z1 = u2 * m;
  return z0 * sigma + mu;
}
#endif
