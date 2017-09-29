#ifndef MYUTIL_HPP
#define MYUTIL_HPP

#include <cmath>
#include <random>
#include <utility>

static const char MYUTIL_HPP_SCCS_ID[] __attribute__((used)) = "@(#)myutil.h++: $Id$";

namespace util {

constexpr double infinity=1.e100;
inline double probit(const double p) {
  return p<=0.0?-infinity:p>=1.0?infinity:std::log(p/(1-p));
}
inline double invprobit(const double probit) {
  return probit<=-infinity?0:probit>=infinity?1:
    1/(1+std::exp(-probit));
}
template<typename generator>
double BoxMueller(const double mu, const double sigma, generator &r)
{
// Box Mueller transform in polar form
  const double epsilon = std::pow(2.,-20);

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

  const double m = std::sqrt(-2*log(s)/s);

  z0 = u1 * m;
  z1 = u2 * m;
  return z0 * sigma + mu;
}

template<class RandomIt, class UniformRandomBitGenerator>
void myshuffle(RandomIt first, RandomIt last,
	       UniformRandomBitGenerator& g)
{
  const auto n = last - first;
  for (auto i = n-1; i > 0; --i) {
    using std::swap;
    swap(first[i], first[i*std::generate_canonical<double,20>(g)]);
  }
}

/*

// Determine how many of the first n bits differ between two integers
// (If they don't come unsigned, they will me made unsigned)
unsigned common_bits(int b1, int b2, int n) { 
	unsigned x = static_cast<unsigned>(b1) ^ static_cast<unsigned>(b2); // Find the xor of b1, b2; now count the number of bits that are clear
	unsigned m = 0;
	for(int i=0; i<n; i++) {
		if((x & 1) == 0) m++;
		x>>=1; // Shift one bit to the right
	} // There is probably a more efficient way to do this...
	return m;
}

*/

constexpr unsigned common_bits (const int b1, const int b2, const int n) {
  return b2?common_bits(~(b1^b2),0,n):
    b1&((1U<<n)-1)? common_bits(b1&(b1-1),0,n)+1: 0;
}

/*
// Count how many bits are required to store the set of numbers 0, 1, ..., n-1
// Note if n<=0, we will get 0.
// (If everyone used unsigned in preference to int habitually, this would never happen)
unsigned count_bits(int n) {
	unsigned b = 0;
	while(1<<b < n) ++b;
	return b;
}
*/

constexpr unsigned count_bits(const int n) {
  // parentheses around n-1 to silence compiler warning
  return n<=1?count_bits(((n-1)>>1)+1)+1:0;
}

}

#endif
