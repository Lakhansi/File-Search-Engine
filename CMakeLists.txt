cmake_minimum_required(VERSION 3.16)

project(FileSearchApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(FileSearchApp main.cpp
    index.h index.cpp
    indexer.h indexer.cpp
    trie.h trie.cpp
    utils.h utils.cpp)

include(GNUInstallDirs)
install(TARGETS FileSearchApp
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
target_link_libraries(FileSearchApp PRIVATE stdc++fs)
