#include <iostream>

class nullstream : public std::ostream
{
  struct nullbuf : public std::streambuf
  {
    virtual int_type overflow(int_type) { return traits_type::eof(); }
  } _;

public:
  nullstream() : std::ostream(&_) {}
};

class ProgressBar
{
  std::ostream& os;
  const int count;
  double r;
  int i, n;

public:
  ProgressBar(std::ostream& o, int c)
    : os(o), count(c), r(100.0/c), i(0), n(1)
  {
    os << "0%" << std::flush;
  }

  ~ProgressBar()
  {
    while (n <= 100) next();
  }

  int operator++() { return next(); }
  operator int() const { return i; }
  operator bool() const { return i < count; }

  int next(int d = 1)
  {
    i += d;
    while (n <= (i*r) && n <= 100) {
      if (!(n % 10)) os << n << '%' << std::flush;
      else if (!(n % 2)) os << '.' << std::flush;
      ++n;
    }
    return i;
  }
};

#ifdef DEMO
#include <unistd.h>

int main(int argc, char* argv[])
{
  const int max = (argc > 1 ? atoi(argv[1]) : 100);
  const int delay = (argc > 2 ? atoi(argv[2]) * 1000 : 100000);
  int count = 0;
  for (ProgressBar i(std::cout, max); i; ++i) {
    ++count;
    usleep(delay);
  }
  std::cout << "\ncount = " << count << std::endl;
  return 0;
}
#endif
