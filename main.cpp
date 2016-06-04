#include <map>
#include <chrono>
#include <fstream>
#include <stdexcept>

#include <lua/lua.hpp>
#include <luabind/luabind.hpp>

#include "types.h"
#include "lua_script.h"

using table_t = std::map<std::string, std::string>;

char const *map_get(table_t &p_table, std::string const &p_index) {
  auto itr = p_table.find(p_index);

  if(itr == p_table.end()) {
    return nullptr;
  }

  return itr -> second.c_str();
}

void map_set(table_t &p_table,
	     std::string const &p_index,
	     std::string const &p_value)
{
  p_table[p_index] = p_value;
}

int main(int argc, char const *argv[]) {
  assert(argc > 1);

  // Initialize a Lua state object.
  auto lua = std::unique_ptr<lua_State, void (*)(lua_State *)> {
    luaL_newstate(), lua_close
  };

  luaL_openlibs(lua.get());
  luabind::open(lua.get());

  luabind::module(lua.get(), "vault") [
    luabind::class_<foo_t>("foo")
      .def(luabind::constructor<>())
      .def_readwrite("a", &foo_t::a),
    luabind::class_<bar_t>("bar")
      .def(luabind::constructor<>())
      .def_readwrite("a", &bar_t::a)
      .def_readwrite("b", &bar_t::b)
      .def("get", &bar_t::get, luabind::dependency(luabind::result, _1))
      .def("print", &print_bar),
    luabind::class_<table_t>("map")
      .def(luabind::constructor<>())
      .def("get", map_get)
      .def("set", map_set)
  ];

  auto lua_stack_size = lua_gettop(lua.get()) + 1;
  
  // Build a Lua script object.
  auto lua_script = lua_script_t::construct
    (argv[1], std::ifstream(argv[1]));

  if(!lua_script) {
    fprintf(stderr, "Failed to load script file!\n");
    exit(1);
  }

  // Create environment we will use for user code.
  auto error = luaL_dofile(lua.get(), "environ.lua");

  if(error != 0) {
    fprintf(stderr, "Execution error: %s\n",
	    lua_tostring(lua.get(), -1));
    
    exit(1);
  }

  // Get the usercode registration function.  Normally
  // we could cache this function on the stack for multiple
  // uses, but here we don't for simplicity.
  lua_getglobal(lua.get(), "register_usercode");

  if(lua_isnil(lua.get(), -1)) {
    fprintf(stderr, "Unable to find required function"
	    "'register_usercode'");

    exit(1);
  }

  // Load the script.
  error = lua_script -> load_into(*lua);
  
  if(error != 0) {
    fprintf(stderr, "Load error: %s\n",
	    lua_tostring(lua.get(), -1));
    
    exit(1);
  }
  
  // Call the registration function.
  error = lua_pcall(lua.get(), 1, 1, 0);

  if(error != 0) {
    fprintf(stderr, "Execution error: %s\n",
	    lua_tostring(lua.get(), -1));
    
    exit(1);
  }
    
  // Get the ID of the registered function.
  if(!lua_isnumber(lua.get(), -1)) {
    fprintf(stderr, "Expected user function registration to "
	    "return an integer, but got '%s'", luaL_typename(lua.get(), -1));

    exit(1);
  }

  auto usercode_ID = lua_tonumber(lua.get(), -1);
  lua_pop(lua.get(), 1);

  // Cache the function we will use to call usercode.
  lua_getglobal(lua.get(), "execute_usercode");
  
  if(lua_isnil(lua.get(), -1)) {
    fprintf(stderr, "Unable to find required function"
	    "'execute_usercode'");

    exit(1);
  }

  //========================
  // Run the Lua iterations.
  //========================
  auto start_time = std::chrono::high_resolution_clock::now();

  for(auto i = 0; i < 10000000; ++i) {
    assert(lua_gettop(lua.get()) == lua_stack_size);

    // Push a copy of the execution function.
    lua_pushvalue(lua.get(), -1);

    // Push the name of the usercode.
    lua_pushnumber(lua.get(), usercode_ID);

    // Run the script in a protected environment.
    error = lua_pcall(lua.get(), 1, 0, 0);

    if(error != 0) {
      fprintf(stderr, "Execution error: %s\n",
	      lua_tostring(lua.get(), -1));

      exit(1);
    }
  }
  
  auto total_time = std::chrono::high_resolution_clock::now() -
    start_time;
  
  auto lua_nanoseconds = std::chrono::duration_cast
    <std::chrono::nanoseconds>(total_time);

  std::cerr << "10000000 Lua executions took "
	    << lua_nanoseconds.count() << "ns"
	    << std::endl;

  //========================
  // Run the C++ iterations.
  //========================
  start_time = std::chrono::high_resolution_clock::now();

  for(auto i = 0; i < 10000000; ++i) {
    hello_world();
  }

  total_time = std::chrono::high_resolution_clock::now() -
    start_time;
  
  auto cpp_nanoseconds = std::chrono::duration_cast
    <std::chrono::nanoseconds>(total_time);
  
  std::cerr << "10000000 C++ executions took "
	    << cpp_nanoseconds.count() << "ns"
	    << std::endl;

  auto ratio = lua_nanoseconds.count() /
    static_cast<double>(cpp_nanoseconds.count());

  // Dump the execution time ratio.
  std::cerr << "Execution time ratio is "
	    << ratio
	    << ":1"
	    << std::endl;
}
