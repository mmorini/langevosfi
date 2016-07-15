#ifndef COUNTS_HPP
#define COUNTS_HPP

#include "enumvector.h++"

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
void summarize(Enumvector<T,Counts> &counts) {
  Counts totcounts;
  for (auto c: counts) totcounts += c;
  std::cout << "Comprehension " << totcounts.mean() << std::endl;
}

#endif
