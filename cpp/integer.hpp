namespace pads {
namespace math {

typedef long long integer;

inline bool odd(const integer n) { return (n & 0x1) != 0; }
inline bool even(const integer n) { return (n & 0x1) == 0; }
inline integer abs(const integer n) { return (n < 0 ? -n : n); }

integer gcd(integer x, integer y)
{
  integer g = 1;
  while (even(x) && even(y)) {
    x >>= 1;
    y >>= 1;
    g <<= 1;
  }
  while (x) {
    while (even(x)) x >>= 1;
    while (even(y)) y >>= 1;
    const integer t = abs(x - y) / 2;
    if (x >= y) {
      x = t;
    } else {
      y = t;
    }
  }
  return g * y;
}

integer phi(integer n)
{
  integer count = 1;
  if (even(n)) {
    for (integer i = 3; i < n; i+=2) {
      if (gcd(n, i) == 1) ++count;
    }
  } else {
    for (integer i = 2; i < n; ++i) {
      if (gcd(n, i) == 1) ++count;
    }
  }
  return count;
}

integer modexp(integer a, integer b, integer n)
{
  if (b == 0) return 1;
  if (b == 1) return a % n;
  if (odd(b)) {
    const integer x = a % n;
    return (x * modexp(a, b-1, n)) % n;
  } else {
    integer x = a % n;
    x = (x * x) % n;
    return (modexp(x, b/2, n) % n);
  }
}

} // namespace math
} // namespace pads
