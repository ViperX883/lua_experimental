local lsetfenv = setfenv

local make_proxy
make_proxy = function(t, k, v)
  if type(v) ~= "table" then
    t[k] = v
    return v
  end

  local new_v = {}

  local new_v_mt = { 
    __index = function(next_t, next_k)
      return make_proxy(next_t, next_k, v[next_k])
    end
  }

  setmetatable(new_v, new_v_mt)

  t[k] = new_v
  return new_v
end

local usercode_G_mt = {
  __index = function(t, k)
    if k == "_G" then
      return nil
    end

    return make_proxy(t, k, _G[k])
  end
}

local usercode_cache = {}

register_usercode = function(usercode_fn)
  usercode_cache[#usercode_cache + 1] = lsetfenv(
    usercode_fn, setmetatable({}, usercode_G_mt))

  return #usercode_cache
end  

execute_usercode = function(id)
  return usercode_cache[id]()
end

-- Tables that we hide from user code

jit       = nil
debug     = nil
coroutine = nil
package   = nil
io        = nil

-- Functions that we hide from user code

gcinfo         = nil
module         = nil
setfenv        = nil
loadfile       = nil
loadstring     = nil
require        = nil
collectgarbage = nil
dofile         = nil
load           = nil
getfenv        = nil

os.execute   = nil
os.rename    = nil
os.setlocale = nil
os.remove    = nil
os.exit      = nil
os.tmpname   = nil
