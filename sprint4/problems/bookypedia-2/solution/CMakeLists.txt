cmake_minimum_required(VERSION 3.11)

project(bookypedia CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake)
# include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_library(
  libbookypedia STATIC
  src/menu/menu.cpp
  src/menu/menu.h
  src/ui/view.cpp
  src/ui/view.h
  src/app/use_cases.h
  src/app/use_cases_impl.cpp
  src/app/use_cases_impl.h
  src/domain/author.cpp
  src/domain/author.h
  src/domain/domain_fwd.h
  src/util/tagged.h
  src/util/tagged_uuid.cpp
  src/util/tagged_uuid.h
  src/postgres/postgres.cpp
  src/postgres/postgres.h)
target_link_libraries(libbookypedia PUBLIC CONAN_PKG::boost Threads::Threads
                                           CONAN_PKG::libpq CONAN_PKG::libpqxx)

add_executable(bookypedia src/bookypedia.cpp src/bookypedia.h src/main.cpp)
target_link_libraries(bookypedia PRIVATE CONAN_PKG::boost libbookypedia)

add_executable(tests tests/use_case_tests.cpp tests/tagged_uuid_tests.cpp)
target_link_libraries(tests PRIVATE CONAN_PKG::catch2 CONAN_PKG::gtest
                                    libbookypedia)
