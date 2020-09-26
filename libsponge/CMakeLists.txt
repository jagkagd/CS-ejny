file (GLOB LIB_SOURCES "*.cc" "util/*.cc" "tcp_helpers/*.cc")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
add_library (sponge STATIC ${LIB_SOURCES})
