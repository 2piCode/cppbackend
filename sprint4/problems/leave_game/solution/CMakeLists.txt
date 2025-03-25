cmake_minimum_required(VERSION 3.11)

project(game_server CXX)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

find_package(Boost 1.78.0 REQUIRED)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(REQUEST_HANDLER_SOURCES
    src/request_handler/request_handler.cpp
    src/request_handler/file_handler.cpp
    src/request_handler/api_handler/api_handler.cpp)

set(MODEL_SOURCES src/model/roads_handler.cpp)

set(UTILS_SOURCES src/utils/boost_json.cpp src/utils/command_line_parser.cpp)

set(HTTP_SERVER_SOURCES src/http_server/http_server.cpp)

set(APP_SOURCES
    src/app/game/game.cpp src/app/game/game_session_handler.cpp
    src/app/player/player.cpp src/app/player/players.cpp
    src/app/collision_detector.cpp)

set(LOOT_GENERATOR_SOURCES src/loots/loot_generator.cpp)

set(SERIALIZATION src/serialization/application_state.cpp)

set(POSTGRES_SOURCES src/postgres/repository_impl.cpp src/postgres/unit_of_work_impl.cpp)

add_library(game_server_lib STATIC ${MODEL_SOURCES} ${APP_SOURCES}
                                   ${LOOT_GENERATOR_SOURCES})

target_include_directories(game_server_lib PUBLIC CONAN_PKG::boost src)
target_link_libraries(game_server_lib PUBLIC CONAN_PKG::boost Threads::Threads)

add_executable(
  game_server
  src/main.cpp src/json_loader.cpp src/json_converter.cpp
  ${HTTP_SERVER_SOURCES} ${UTILS_SOURCES} ${REQUEST_HANDLER_SOURCES} ${SERIALIZATION} ${POSTGRES_SOURCES})

# add_executable(
#   game_server_tests
#   src/utils/boost_json.cpp
#   tests/loot_generator_tests.cpp
#   tests/get_game_state_tests.cpp
#   tests/get_map_tests.cpp
#   tests/tick_tests.cpp
#   tests/spawn_point_generator_tests.cpp
#   tests/state-serialization-tests.cpp ${POSTGRES_SOURCES})

target_link_libraries(game_server game_server_lib CONAN_PKG::libpq CONAN_PKG::libpqxx)
# target_link_libraries(game_server_tests game_server_lib CONAN_PKG::catch2 CONAN_PKG::libpq CONAN_PKG::libpqxx)

# include(CTest)
# include(${CONAN_BUILD_DIRS_CATCH2}/Catch.cmake)
# catch_discover_tests(game_server_tests)
