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

# Testes
enable_testing()

add_executable(test_chefvault src/test.cpp src/Database.cpp)

if(SQLITE3_FOUND)
    target_link_libraries(test_chefvault ${SQLITE3_LIBRARIES})
    target_include_directories(test_chefvault PRIVATE ${SQLITE3_INCLUDE_DIRS})
    target_compile_options(test_chefvault PRIVATE ${SQLITE3_CFLAGS_OTHER})
else()
    if(SQLITE3_LIBRARY)
        target_link_libraries(test_chefvault ${SQLITE3_LIBRARY})
    endif()
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
    target_link_libraries(test_chefvault stdc++fs)
endif()

add_test(NAME ChefVaultTests COMMAND test_chefvault)

# Configurar para sempre mostrar saída dos testes
set_tests_properties(ChefVaultTests PROPERTIES
    PASS_REGULAR_EXPRESSION "Todos os testes passaram"
    FAIL_REGULAR_EXPRESSION "Alguns testes falharam"
    TIMEOUT 60
)

# Configurar CTest para sempre mostrar saída
set(CMAKE_CTEST_OUTPUT_ON_FAILURE ON)

# Criar target customizado para testes verbosos (mostra todos os 17 testes)
add_custom_target(test-verbose
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --verbose
    DEPENDS test_chefvault
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Executando testes com saída detalhada (mostra todos os 17 testes)"
)

# Nota: Para ver todos os 17 testes individuais, use:
#   make test-verbose
#   ou
#   ctest --output-on-failure --verbose
#   ou
#   ./test.sh

