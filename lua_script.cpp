#include <memory>
#include <istream>
#include <sstream>
#include <stdexcept>

#include "lua_script.h"

static int lua_writer
  (lua_State *p_lua, void const *p_data, size_t p_size, void *p_ostream)
{
  static_cast<std::ostringstream *>(p_ostream) ->
    write(static_cast<char const *>(p_data), p_size);

  return !*static_cast<std::ostringstream *>(p_ostream);
}

char const *lua_script_t::lua_reader
  (lua_State *p_lua, void *p_data, size_t *p_size) noexcept
{
  return static_cast<lua_script_t *>(p_data) ->
    m_reader(p_lua, p_data, p_size);
}

// We only need a single call to the reader to read the entire
// chunk.  So, during reader execution we set the reader to be
// a pointer to a function that just returns null.  That ensures
// it gets invoked if the Lua runtime calls the reader again.
char const *lua_script_t::lua_reader_real
  (lua_State *p_lua, void *p_data, size_t *p_size) noexcept
{
  lua_script_t *self = static_cast<lua_script_t *>(p_data);

  self -> m_reader = lua_reader_noop;
  *p_size  = self -> m_bytecode.size();

  return self -> m_bytecode.data();
}

char const *lua_script_t::lua_reader_noop
  (lua_State *p_lua, void *p_data, size_t *p_size) noexcept
{
  static_cast<lua_script_t *>(p_data) -> m_reader =
    lua_reader_real;

  return nullptr;
}

boost::optional<lua_script_t> lua_script_t::construct
  (std::string p_name, std::istream &p_istream) noexcept try
{
  auto lua = std::unique_ptr<lua_State, void (*)(lua_State *)> {
    luaL_newstate(), lua_close
  };

  return construct(std::move(p_name), p_istream, *lua);
} catch(std::exception const &e) {
  std::cout << e.what() << std::endl;
  return { };
} catch(...) {
  return { };
}

boost::optional<lua_script_t> lua_script_t::construct
  (std::string p_name, std::istream &&p_istream) noexcept
{
  return lua_script_t::construct(std::move(p_name), p_istream);
}

boost::optional<lua_script_t> lua_script_t::construct
  (std::string p_name, std::istream &p_istream, lua_State &p_lua) noexcept try
{
  return lua_script_t { std::move(p_name), p_istream, p_lua };
} catch(std::exception const &e) {
  std::cout << e.what() << std::endl;
  return { };
} catch(...) {
  return { };
}

boost::optional<lua_script_t> lua_script_t::construct
  (std::string p_name, std::istream &&p_istream, lua_State &p_lua) noexcept
{
  return construct(std::move(p_name), p_istream, p_lua);
}

lua_script_t::lua_script_t(std::string p_name, std::istream &p_istream, lua_State &p_lua)
  : m_name(std::move(p_name))
{
  // Replace with a SCOPE_EXIT that ensures the Lua
  // stack is balanced upon function exit.
  auto stack_size = lua_gettop(&p_lua);

  // Read all of the data from the buffer.
  std::string line;

  while(std::getline(p_istream, line)) {
    m_bytecode += line;
    m_bytecode += '\n';
  }

  // Load the data into a Lua state object.
  if(luaL_loadstring(&p_lua, m_bytecode.c_str())) {
    std::runtime_error error(lua_tostring(&p_lua, -1));
    lua_pop(&p_lua, 1); // Remove error information.
    throw error;
  }

  // Dump the compiled bytecode.
  std::ostringstream ostream;

  if(lua_dump(&p_lua, lua_writer, &ostream)) {
    std::runtime_error error("Failed to dump bytecode.");
    lua_pop(&p_lua, 1); // Remove the function from the stack.
    throw error;
  }

  // Remove the function from the stack.
  lua_pop(&p_lua, 1); 

  // Save the bytecode.
  m_bytecode = ostream.str();

  // Replace with a SCOPE_EXIT that ensures the Lua
  // stack is balanced upon function exit.
  assert(lua_gettop(&p_lua) == stack_size);
}

int lua_script_t::load_into(lua_State &p_lua) {
  // Need to reset the reader in case it was swapped to
  // the noop version the last time we loaded our bytecode.
  m_reader = lua_reader_real;

  return lua_load
    (&p_lua, lua_reader, this, m_name.c_str());
}
