cmake_minimum_required(VERSION 3.10)
project(CookBookCLI)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

# Arquivos fonte
set(SOURCES
    src/main.cpp
    src/Database.cpp
)

add_executable(cookbook ${SOURCES})

find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(SQLITE3 sqlite3)
endif()

if(SQLITE3_FOUND)
    target_link_libraries(cookbook ${SQLITE3_LIBRARIES})
    target_include_directories(cookbook PRIVATE ${SQLITE3_INCLUDE_DIRS})
    target_compile_options(cookbook PRIVATE ${SQLITE3_CFLAGS_OTHER})
else()
    # Fallback: procurar SQLite3 manualmente
    find_library(SQLITE3_LIBRARY sqlite3)
    if(SQLITE3_LIBRARY)
        target_link_libraries(cookbook ${SQLITE3_LIBRARY})
    else()
        message(FATAL_ERROR "SQLite3 library not found. Please install libsqlite3-dev")
    endif()
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
    target_link_libraries(cookbook stdc++fs)
endif()

