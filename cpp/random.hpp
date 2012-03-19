#ifndef _PADS_RANDOM_H_
#define _PADS_RANDOM_H_

#include <algorithm>
#include <math.h>
#include <stdlib.h>

namespace pads {
namespace random {

/// Basic Functions ///

// Initialization function, should be called before using generate().
void seed(long int s)
{
  ::srand48(s);
}

// Returns non-negative double-precision floating-point values uniformly distributed between [0, 1[.
double generate()
{
  return ::drand48();
}

/// Convenience Functions ///

// Returns double-precision floating-point values uniformly distributed between [low, high[.
double range(double low, double high)
{
  if (low == high) return low;
  if (low > high) return range(high, low);
  return (low + (high - low) * generate());
}

// Returns integer values uniformly distributed between [low, high].
// e.g.: char c = integer_range<char>('a', 'z');
template<typename T>
T integer_range(double low, double high)
{
  return static_cast<T>(::nearbyint(range(low, high)));
}

// Can be used with std::random_shuffle.
ptrdiff_t rand(ptrdiff_t n)
{
  return static_cast<ptrdiff_t>(n * generate());
}

// Random selection with probability rates.
// RAI first, last : random access iterators range
// Predicate pred  : return the probability rate of the current item ([0.0, 1.0])
// Returns the selected item.
template<typename RAI, typename Predicate>
RAI selection(RAI first, RAI last, Predicate pred)
{
  double r = generate();
  for (RAI it = first; it != last; ++it) {
    const double d = pred(*it);
    if (r <= d) return it;
    r -= d;
  }
  return last;
}

} // namespace random
} // namespace pads

#endif // _PADS_RANDOM_H_
