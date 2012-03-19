#include "integer.hpp"

namespace pads {
namespace math {

class primes
{
public:
  primes()
    : lkp(997)
  {
    const int p[168] = {
      2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97,
      101,103,107,109,113,127,131,137,139,149,151,157,163,167,173,179,181,191,193,197,199,
      211,223,227,229,233,239,241,251,257,263,269,271,277,281,283,293,
      307,311,313,317,331,337,347,349,353,359,367,373,379,383,389,397,
      401,409,419,421,431,433,439,443,449,457,461,463,467,479,487,491,499,
      503,509,521,523,541,547,557,563,569,571,577,587,593,599,
      601,607,613,617,619,631,641,643,647,653,659,661,673,677,683,691,
      701,709,719,727,733,739,743,751,757,761,769,773,787,797,
      809,811,821,823,827,829,839,853,857,859,863,877,881,883,887,
      907,911,919,929,937,941,947,953,967,971,977,983,991,997,
    };
    _primes.insert(&p[0], &p[168]);
  }

  typedef std::set<integer>::const_iterator const_iterator;
  const_iterator begin() const { return _primes.begin(); }
  const_iterator end()   const { return _primes.end();   }

  typedef std::map<integer, size_t> factors;
  void factorize(integer n, factors& f)
  {
    f.clear();
    if (n < 2) return;

    if (_primes.count(n)) {
      f[n] = 1;
    } else {
      integer p, d = n;
      for (const_iterator it = _primes.begin(), end = _primes.end(); it != end; ++it) {
        p = *it;
        size_t count = 0;
        while (d % p == 0) {
          ++count;
          d /= p;
        }
        if (count) f[p] = count;
      }

      const integer max = (integer) std::sqrt((double)n) + 1;
      for (integer i = lkp+2; d != 1 && i < max; i += 2) {
        if (n % i == 0) {
          _primes.insert(i);
          size_t count = 0;
          do {
            d /= i;
            ++count;
          } while (n % i == 0);
          if (count) f[i] = count;
        }
      }

      if (d != 1) {
        _primes.insert(d);
        f[d] = 1;
      }
    }
  }

  bool is_prime(integer n)
  {
    if (n < 2) return false;
    if (n == 2) return true;
    if (even(n)) return false;
    if (_primes.count(n)) return true;

    for (const_iterator it = _primes.begin(), end = _primes.end(); it != end; ++it) {
      if (n % (*it) == 0) return false;
    }

    const integer max = (integer) std::sqrt((double)n);
    for (integer i = lkp+2; i < max; i += 2) {
      if (n % i == 0) {
        _primes.insert(i);
        return false;
      }
    }

    _primes.insert(n);
    return true;
  }

  bool fast_miller_rabin(integer n)
  {
    if (_primes.count(n)) return true;
    if (miller_rabin_witness(n,  2)) return false;
    if (miller_rabin_witness(n,  3)) return false;
    if (miller_rabin_witness(n,  5)) return false;
    if (miller_rabin_witness(n,  7)) return false;
    if (miller_rabin_witness(n, 11)) return false;
    if (miller_rabin_witness(n, 13)) return false;
    if (miller_rabin_witness(n, 17)) return false;
    if (miller_rabin_witness(n, 31)) return false;
    if (miller_rabin_witness(n, 73)) return false;
    if (miller_rabin_witness(n, 61)) return false;
    _primes.insert(n);
    return true;
  }

  integer next_prime(integer n)
  {
    if (n < 2) return 2;
    if (n == 2) return 3;
    n += (odd(n)?2:1); // next odd
    while (!is_prime(n)) n += 2;
    return n;
  }

  integer prev_prime(integer n)
  {
    if (n <= 2) return 0;
    if (n == 3) return 2;
    n -= (odd(n)?2:1); // prev odd
    while (!is_prime(n)) n -= 2;
    return n;
  }

private:
  std::set<integer> _primes;
  const integer lkp; // last known prime

  bool miller_rabin_witness(integer n, integer a)
  {
    integer t = 0;
    integer u = n-1;
    while (even(u)) { ++t; u >>= 1; }
    integer x = modexp(a, u, n);
    for (integer i = 1; i <= t; ++i) {
      integer y = (x * x) % n;
      if (y == 1 && x != 1 && x != n-1) return true;
      x = y;
    }
    if (x != 1) return true;
    return false;
  }
};

} // namespace math
} // namespace pads
