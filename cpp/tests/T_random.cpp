#include <algorithm>
#include <iostream>
#include <map>
#include <vector>
#include <cstdlib>
#include <ctime>
#include "random.hpp"

struct item
{
  int value;
  double rate;
  item(int i, double d) : value(i), rate(d) {}
  double get_rate() const { return rate; }
};

typedef std::vector<item> items;

int main(int argc, char* argv[])
{
  size_t n = (argc > 1 ? ::atoi(argv[1]) : 100);
  pads::random::seed(std::time(0));

  items my_items;
  my_items.push_back(item(1, .1));
  my_items.push_back(item(2, .2));
  my_items.push_back(item(3, .2));
  my_items.push_back(item(4, .5));

  std::random_shuffle(my_items.begin(), my_items.end(), pads::random::rand);

  std::map<int, int> freq;
  for (size_t i = 0; i < n; ++i) {
    const items::iterator it = pads::random::selection(my_items.begin(), my_items.end(), std::mem_fun_ref(&item::get_rate));
    if (it != my_items.end()) {
      ++freq[it->value];
    } else {
      std::cout << "oops!" << std::endl;
    }
  }
  for (std::map<int,int>::const_iterator it = freq.begin(); it != freq.end(); ++it) {
    std::cout << it->first << ": " << it->second << '\n';
  }

  return 0;
}
