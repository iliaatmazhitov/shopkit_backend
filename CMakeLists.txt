cmake_minimum_required(VERSION 3.25)
project(shopkit_backend)

set(CMAKE_CXX_STANDARD 20)

add_executable(shopkit_backend main.cpp shopkit/controllers/ShopController.h)
