#include "splay_tree.hpp"
#include <iostream>
#include <string>
#include <time.h>
#include <tbb/tbb_allocator.h>

int main()
{
  pads::lcg r(::time(0));
  pads::splay_tree<int, int, std::less<int>, tbb::tbb_allocator<pads::node<int, int> > > tree;

  for (int i = 0; i < 20; ++i) {
    tree.insert(i, r.random_integer());
  }
 
  std::cout << tree << std::endl;

  return 0;
}
