#ifndef __LUA_LIBRARY_H
#define __LUA_LIBRARY_H

template<typename T>
class lua_library_t {
  using  create_fn_t = std::function<T *()>;
  using destroy_fn_t = std::function<void (T *)>;

  std::string m_uuid;

  create_fn_t  m_create_fn  = &lua_default_create <T>;
  destroy_fn_t m_destroy_fn = &lua_default_destroy<T>;

  std::vector<luaL_reg> m_library_fns = {
    { "__gc", __gc }
  };

  lua_library_t() { }

  static int __create(lua_State *p_lua) noexcept try {
    void **obj = static_cast<void **>
      (lua_newuserdata(p_lua, sizeof(T *)));
    
    // Set the object's metatable.
    luaL_getmetatable(p_lua, instance().m_uuid.c_str());
    lua_setmetatable(p_lua, -2);
    
    // Try to create the object.
    auto ptr = std::unique_ptr<T, destroy_fn_t>
      (instance().m_create_fn(), instance().m_destroy_fn);
    
    *obj = ptr.get();
    lua_cache.emplace_back(std::move(ptr));
    
    return 1;
  } catch(std::exception const &e) {
    luaL_error(p_lua, "%s", e.what());
  } catch(...) {
    luaL_error(p_lua, "Caught unknown exception");
  }
  
  static T *__check(lua_State *p_lua, int p_index = 1) noexcept try {
    void *obj = luaL_checkudata
      (p_lua, p_index, instance().m_uuid.c_str());
    
    char buf[256] = { '\0' };
    snprintf(buf, 256, "Expected instance of type `%s'",
	     instance().m_uuid.c_str());
    
    luaL_argcheck(p_lua, obj != nullptr, p_index, buf);
    return *static_cast<T **>(obj);
  } catch(std::exception const &e) {
    luaL_error(p_lua, "%s", e.what());
  } catch(...) {
    luaL_error(p_lua, "Caught unknown exception");
  }
  
  static int __gc(lua_State *p_lua) noexcept try {
    void *obj = __check(p_lua);

    auto cmp = [=](lua_cache_ptr_t const &p_value) {
      return p_value.get() == obj;
    };
  
    auto new_end = std::remove_if
      (lua_cache.begin(), lua_cache.end(), std::move(cmp));
  
    lua_cache.erase(new_end, lua_cache.end());
    return 0;
  } catch(std::exception const &e) {
    luaL_error(p_lua, "%s", e.what());
  } catch(...) {
    luaL_error(p_lua, "Caught unknown exception");
  }

public:
  static lua_library_t &instance() {
    static lua_library_t self;
    return self;
  }

  lua_library_t &set_uuid(std::string p_uuid) {
    return (m_uuid = std::move(p_uuid), *this);
  }

  lua_library_t &set_create_fn(create_fn_t p_create_fn) {
    return (m_create_fn = std::move(p_create_fn), *this);
  }

  lua_library_t &set_destroy_fn(destroy_fn_t p_destroy_fn) {
    return (m_destroy_fn = std::move(p_destroy_fn), *this);
  }
};

template<typename T>
lua_library_t<T> &lua_library() {
  return lua_library_t<T>::instance();
}

auto const lua_foo_library = lua_library<foo_t>()
  .set_uuid("foo")
  .set_destroy_fn(lua_default_destroy<foo_t>)
  .set_create_fn (lua_default_create <foo_t>)
  //  .set_tostring(tostring_foo)
  //  .add_readwrite_string_attribute<&foo::a>("a")
;

#endif
