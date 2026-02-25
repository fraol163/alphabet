cmake_minimum_required(VERSION 3.16)
project(Alphabet VERSION 2.0.0 LANGUAGES CXX)

# C++17 standard required for std::variant, std::string_view, etc.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# AddressSanitizer support (for CI/testing)
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
if(ENABLE_ASAN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
endif()

# Export compile commands for IDE integration
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ============================================================================
# Source Files
# ============================================================================

set(ALPHABET_SOURCES
    src/main.cpp
    src/include/lexer.cpp
    src/include/parser.cpp
    src/include/compiler.cpp
    src/include/vm.cpp
    src/include/type_system.cpp
    src/include/ffi.cpp
    src/include/lsp.cpp
)

set(ALPHABET_HEADERS
    src/include/bytecode.h
    src/include/lexer.h
    src/include/parser.h
    src/include/alphabet_ast.h
    src/include/compiler.h
    src/include/vm.h
    src/include/type_system.h
    src/include/ffi.h
)

# ============================================================================
# Main Executable
# ============================================================================

add_executable(alphabet ${ALPHABET_SOURCES} ${ALPHABET_HEADERS})

target_include_directories(alphabet PRIVATE
    ${CMAKE_SOURCE_DIR}/src/include
)

# Platform-specific settings
if(WIN32)
    target_link_libraries(alphabet PRIVATE)
    set_target_properties(alphabet PROPERTIES
        WIN32_EXECUTABLE TRUE
        OUTPUT_NAME "alphabet"
    )
elseif(APPLE)
    target_link_libraries(alphabet PRIVATE dl)
    set_target_properties(alphabet PROPERTIES
        OUTPUT_NAME "alphabet"
    )
else()
    # Linux/Unix
    target_link_libraries(alphabet PRIVATE dl)
    set_target_properties(alphabet PROPERTIES
        OUTPUT_NAME "alphabet"
    )
endif()

# ============================================================================
# Tests
# ============================================================================

option(BUILD_TESTS "Build test suite" ON)

if(BUILD_TESTS)
    enable_testing()
    
    # Test executable for lexer
    add_executable(test_lexer tests/test_lexer.cpp ${ALPHABET_SOURCES})
    target_include_directories(test_lexer PRIVATE ${CMAKE_SOURCE_DIR}/src/include)
    target_compile_definitions(test_lexer PRIVATE UNIT_TEST)
    
    # Test executable for parser
    add_executable(test_parser tests/test_parser.cpp ${ALPHABET_SOURCES})
    target_include_directories(test_parser PRIVATE ${CMAKE_SOURCE_DIR}/src/include)
    target_compile_definitions(test_parser PRIVATE UNIT_TEST)
    
    # Test executable for VM
    add_executable(test_vm tests/test_vm.cpp ${ALPHABET_SOURCES})
    target_include_directories(test_vm PRIVATE ${CMAKE_SOURCE_DIR}/src/include)
    target_compile_definitions(test_vm PRIVATE UNIT_TEST)
    
    # Add tests
    add_test(NAME LexerTests COMMAND test_lexer)
    add_test(NAME ParserTests COMMAND test_parser)
    add_test(NAME VMTests COMMAND test_vm)
endif()

# ============================================================================
# Installation
# ============================================================================

include(GNUInstallDirs)

install(TARGETS alphabet
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install example files
install(DIRECTORY examples/ DESTINATION ${CMAKE_INSTALL_DATADIR}/alphabet/examples)

# ============================================================================
# CPack Configuration
# ============================================================================

set(CPACK_PACKAGE_NAME "alphabet")
set(CPACK_PACKAGE_VENDOR "Fraol Teshome")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Alphabet Programming Language - Minimalist compiled language")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")

# Package file name
set(CPACK_PACKAGE_FILE_NAME "alphabet-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")

# ============================================================================
# Windows NSIS Installer
# ============================================================================

if(WIN32)
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_NSIS_DISPLAY_NAME "Alphabet Programming Language")
    set(CPACK_NSIS_HELP_LINK "https://github.com/fraol163/alphabet")
    set(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/fraol163/alphabet")
    set(CPACK_NSIS_CONTACT "fraolteshome444@gmail.com")
    set(CPACK_NSIS_MODIFY_PATH ON)  # Add to PATH
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_PACKAGE_NAME "Alphabet Language")
endif()

# ============================================================================
# Linux Packages
# ============================================================================

if(UNIX AND NOT APPLE)
    # DEB package
    set(CPACK_GENERATOR "DEB;TGZ")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Fraol Teshome <fraolteshome444@gmail.com>")
    set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.17)")
endif()

# ============================================================================
# macOS DMG
# ============================================================================

if(APPLE)
    set(CPACK_GENERATOR "DragNDrop;TGZ")
    set(CPACK_DMG_FORMAT "UDBZ")
    set(CPACK_DMG_VOLUME_NAME "Alphabet Language")
endif()

# Include CPack module (must be last)
include(CPack)

# ============================================================================
# Custom Targets
# ============================================================================

# Run all tests
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS test_lexer test_parser test_vm
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running all tests..."
)

# Format code (requires clang-format)
find_program(CLANG_FORMAT clang-format)
if(CLANG_FORMAT)
    add_custom_target(format
        COMMAND ${CLANG_FORMAT} -i -style=file ${ALPHABET_SOURCES} ${ALPHABET_HEADERS}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Formatting source code..."
    )
endif()

# ============================================================================
# Print Configuration
# ============================================================================

message(STATUS "")
message(STATUS "Alphabet Language Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  AddressSanitizer: ${ENABLE_ASAN}")
message(STATUS "  Build Tests: ${BUILD_TESTS}")
message(STATUS "  Install Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
