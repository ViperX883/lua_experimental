INC_DIRS="-isystem $HOME/Local/include -isystem /usr/include/luajit-2.0"
LIB_DIRS="-L$HOME/Local/lib -L$PWD"

export LD_LIBRARY_PATH=$PWD:$HOME/Local/lib

g++ $INC_DIRS $LIB_DIRS -std=gnu++11 -g -ggdb -O3 -shared -fPIC -o libtypes.so \
    types.cpp \
    lua_script.cpp \
    -lluajit-5.1

g++ $INC_DIRS $LIB_DIRS -g -ggdb -std=c++11 -O3 -fPIC -rdynamic -o main main.cpp \
    -ltypes \
    -lluajit-5.1 \
    -lluabind
