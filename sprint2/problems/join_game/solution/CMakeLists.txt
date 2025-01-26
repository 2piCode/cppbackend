cmake_minimum_required(VERSION 3.11)

project(game_server CXX)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

find_package(Boost 1.78.0 REQUIRED log log_setup)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(REQUEST_HANDLER_SOURCES
    src/request_handler/request_handler.h
    src/request_handler/request_handler.cpp
    src/request_handler/file_handler.h
    src/request_handler/file_handler.cpp
    src/request_handler/logging_request_handler.h
    src/request_handler/api_handler.h
    src/request_handler/api_handler.cpp)

set(MODEL_SOURCES src/model/model.h src/model/model.cpp src/model/tagged.h)

set(UTILS_SOURCES src/utils/boost_json.cpp src/utils/sdk.h src/utils/logger.h)

set(HTTP_SERVER_SOURCES src/http_server/http_server.h
                        src/http_server/http_server.cpp)

set(APP_SOURCES src/app/game.cpp src/app/player.cpp)

add_executable(
  game_server
  src/main.cpp
  src/json_loader.h
  src/json_loader.cpp
  src/json_converter.h
  src/json_converter.cpp
  ${MODEL_SOURCES}
  ${HTTP_SERVER_SOURCES}
  ${UTILS_SOURCES}
  ${REQUEST_HANDLER_SOURCES}
  ${APP_SOURCES})

target_include_directories(
  game_server
  PRIVATE CONAN_PKG::boost
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(
  game_server
  PRIVATE Threads::Threads
  PRIVATE CONAN_PKG::boost
  PRIVATE Boost::log)
