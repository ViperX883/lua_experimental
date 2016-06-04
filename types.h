#ifndef __LUA_TYPES_H
#define __LUA_TYPES_H

#include <string>

extern "C" {
  struct foo_t;
  struct bar_t;
}

void hello_world();
void print_bar(bar_t &);

struct foo_t {
  std::string a;
};

struct bar_t {
  foo_t &get() { return b; }

  std::string a;
  foo_t b;
};

#endif
