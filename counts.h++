#ifndef COUNTS_HPP
#define COUNTS_HPP

#include "enumvector.h++"
#include <iostream>

static const char COUNTS_HPP_SCCS_ID[] __attribute__((used)) = "@(#)counts.h++: $Id$";

struct Counts {
  double success=0; int tries=0;
  auto operator += (const double s) {
    success += s; tries++;
  }
  auto operator += (const Counts c) {
    success += c.success; tries += c.tries;
  }
  auto mean(void) const {return success/tries;}
};

template<typename T>
void summarize(const Enumvector<T,Counts> &counts) {
  Counts totcounts;
  for (auto c: counts) totcounts += c;
  std::cout << "Comprehension " << totcounts.mean() << std::endl;
}

#endif
