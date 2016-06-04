#ifndef __LUA_SCRIPT_H
#define __LUA_SCRIPT_H

#include <string>
#include <iosfwd>

#include <boost/optional.hpp>
#include <luajit-2.0/lua.hpp>

class lua_script_t {
  static char const *lua_reader
    (lua_State *p_lua, void *p_data, size_t *p_size) noexcept;

  static char const *lua_reader_real
    (lua_State *p_lua, void *p_data, size_t *p_size) noexcept;

  static char const *lua_reader_noop
    (lua_State *p_lua, void *p_data, size_t *p_size) noexcept;

  lua_Reader  m_reader = lua_reader_real;

  std::string m_name;
  std::string m_bytecode;

  lua_script_t(std::string p_name, std::istream &p_istream, lua_State &p_lua);

public:
  static boost::optional<lua_script_t>
  construct(std::string p_name, std::istream &p_istream) noexcept;

  static boost::optional<lua_script_t>
  construct(std::string p_name, std::istream &&p_istream) noexcept;

  static boost::optional<lua_script_t>
  construct(std::string p_name, std::istream &p_istream, lua_State &p_lua)
    noexcept;

  static boost::optional<lua_script_t>
  construct(std::string p_name, std::istream &&p_istream, lua_State &p_lua)
    noexcept;

  int load_into(lua_State &p_lua);
};

#endif
