cmake_minimum_required(VERSION 3.5)

project(otus-homeword-07-bulk LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(bulk main.cpp)

include(GNUInstallDirs)
install(TARGETS bulk
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
