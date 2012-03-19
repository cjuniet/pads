#include "dsaa.hpp"
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <limits>
#include <map>
#include <vector>
#include <sys/time.h>

const int N = 100;

int main()
{
  std::srand(std::time(0));

  dsaa::lcg r;
  std::vector<int> v(N);
  for (int i = 0; i < N; ++i) {
    v[i] = i;
  }
  std::random_shuffle(v.begin(), v.end());
  timeval t0, t;

#if 0
  std::cout << "dsaa::DS : ";
  dsaa::DSL<int> dsl(std::numeric_limits<int>::max());
  gettimeofday(&t0, 0);
  for (int i = 0; i < 100000; ++i) {
    dsl.insert(i);
  }
  gettimeofday(&t, 0); std::cout << (1000000*(t.tv_sec-t0.tv_sec)+(t.tv_usec-t0.tv_usec)) << " us / "; t0 = t;
  for (int i = 0; i < 100000; ++i) {
    dsl.contains(i);
  }
  gettimeofday(&t, 0); std::cout << (1000000*(t.tv_sec-t0.tv_sec)+(t.tv_usec-t0.tv_usec)) << " us" << std::endl;
#endif

#if 0
#if 0
  //std::cout << "dsaa::BST : ";
  dsaa::BST<int> bst1;
  //gettimeofday(&t0, 0);
  for (int i = 0; i < N; ++i) {
    bst1.insert(v[i]);
  }
  std::cout << bst1 << std::endl;

  //dsaa::BST<int> bst2 = bst1;
  //bst2.remove(5);
  //std::cout << bst2 << std::endl;
#else
  dsaa::BST<int> bst;
  bst.insert(5);
  bst.insert(2);
  bst.insert(1);
  bst.insert(4);
  bst.insert(3);
  bst.insert(8);
  bst.insert(7);
  bst.insert(6);
  std::cout << bst << std::endl;
#endif
#endif

#if 1
  dsaa::splay_tree<int> st;
  st.insert(1);
  st.insert(2);
  st.insert(3);
  st.insert(4);
  st.insert(5);
  st.insert(6);
  st.insert(7);
  st.insert(8);
  st.contains(4);
  std::cout << st << std::endl;
#endif

#if 0
  gettimeofday(&t, 0); std::cout << (1000000*(t.tv_sec-t0.tv_sec)+(t.tv_usec-t0.tv_usec)) << " us / "; t0 = t;
  for (int i = 0; i < 100000; ++i) {
    bst.contains(i);
  }
  gettimeofday(&t, 0); std::cout << (1000000*(t.tv_sec-t0.tv_sec)+(t.tv_usec-t0.tv_usec)) << " us" << std::endl;

  std::cout << "std::map : ";
  std::map<int, int> c;
  gettimeofday(&t0, 0);
  for (int i = 0; i < 100000; ++i) {
    c[i] = i;
  }
  gettimeofday(&t, 0); std::cout << (1000000*(t.tv_sec-t0.tv_sec)+(t.tv_usec-t0.tv_usec)) << " us / "; t0 = t;
  for (int i = 0; i < 100000; ++i) {
    c.find(i);
  }
  gettimeofday(&t, 0); std::cout << (1000000*(t.tv_sec-t0.tv_sec)+(t.tv_usec-t0.tv_usec)) << " us" << std::endl;
#endif

  return 0;
}
