cmake_minimum_required(VERSION 3.15)
project(CppIDE LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    boost
    URL https://github.com/boostorg/boost/releases/download/boost-1.91.0-1/boost-1.91.0-1-cmake.zip
)

set(BOOST_INCLUDE_LIBRARIES asio CACHE STRING "" FORCE)
set(BOOST_ENABLE_CMAKE ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(boost)

add_executable(CppIDE "main.cpp")

add_executable(echo-server "echo-server.cpp")
target_link_libraries(echo-server PRIVATE Boost::asio)

add_executable(client "client.cpp")
target_link_libraries(client PRIVATE Boost::asio)