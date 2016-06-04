#include "types.h"

#include <map>
#include <iostream>

void hello_world() {
  printf("=========\n");

#if 0
  for(auto &&outer_p : map) {
    auto &&m = outer_p.second;

    for(auto &&inner_p : m) {
      printf("table[%s][%s] = %s\n",
	     outer_p.first.c_str(),
	     inner_p.first.c_str(),
	     inner_p.second.c_str());
    }
  }
#endif

  auto i = bar_t();

  i.a = "foo";
  i.b.a = "bar";

  print_bar(i);

  auto m = std::map<std::string, std::string> { };

  m["baz"] = "buz";
  printf("BBBB %s\n", m["baz"].c_str());

  printf("=========\n");
}

void print_bar(bar_t &p_bar) {
  printf("bar_t {\n\ta : %s,\n\tb : {\n\t\ta : %s\n\t}\n}\n", p_bar.a.c_str(), p_bar.b.a.c_str());
}
