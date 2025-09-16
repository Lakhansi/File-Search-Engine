cmake_minimum_required(VERSION 3.16)

project(FileSearchApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_PATH "C:/Qt/6.9.1/mingw_64")
set(CMAKE_PREFIX_PATH ${QT_PATH})

# Find Qt components
find_package(Qt6 REQUIRED COMPONENTS Core Qml Quick)

# Set up automoc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# List the .cpp files that are in the SAME directory as this CMakeLists.txt
add_executable(FileSearchApp
    main.cpp
    index.h index.cpp
    indexer.h indexer.cpp
    trie.h trie.cpp
    utils.h
    resources.qrc
    # appbackend.h appbackend.cpp

)

target_include_directories(FileSearchApp PRIVATE src)
target_link_libraries(FileSearchApp PRIVATE stdc++fs)
target_link_libraries(FileSearchApp PRIVATE Qt6::Core Qt6::Qml Qt6::Quick)

