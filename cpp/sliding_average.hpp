#ifndef _SLIDING_AVERAGE_H_
#define _SLIDING_AVERAGE_H_

#include <algorithm>
#include <vector>

template<typename T, int N, class C = std::vector<T> >
class sliding_average
{
  C c;
  T sum;
  size_t pos;

public:
  typedef T value_type;
  typedef C container_type;
  enum { capacity = N };

  sliding_average()
    : pos(0), sum(0)
  {}

  double add(const T& t)
  {
    if (c.size() < N) {
      c.push_back(t);
    } else {
      sum -= c[pos];
      c[pos] = t;
    }

    sum += t;
    pos = (pos + 1) % N;
    return mean();
  }

  void clear()
  {
    c.clear();
    sum = 0;
    pos = 0;
  }

  double mean() const
  {
    return ((double) sum) / c.size();
  }
};

#endif // _SLIDING_AVERAGE_H_
