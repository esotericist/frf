cmake_minimum_required(VERSION 3.15)
project(frf)

file(GLOB_RECURSE sources      src/main/*.c src/main/*.h)
file(GLOB_RECURSE sources_test src/test/*.c)
file(GLOB_RECURSE data         resources/*)

add_executable(frf ${sources} ${data})

target_compile_options(frf PUBLIC -g -fcommon -Wall -Wfloat-conversion -Werror-implicit-function-declaration ) # whatever flags you need

target_include_directories(frf PUBLIC src/main)

file(COPY ${data} DESTINATION resources)

find_package(PkgConfig)
pkg_check_modules(GC REQUIRED bdw-gc)
pkg_check_modules(UV REQUIRED libuv)

target_link_libraries(frf PUBLIC ${GC_LDFLAGS} ${GC_LIBRARIES} ${UV_LDFLAGS} ${UV_LIBRARIES})
target_include_directories(frf PUBLIC ${GC_INCLUDE_DIRS} ${UV_INCLUDE_DIRS} )
target_compile_options(frf PUBLIC ${GC_CFLAGS_OTHER} ${UV_CFLAGS_OTHER})
