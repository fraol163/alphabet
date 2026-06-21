cmake_minimum_required(VERSION 3.16)
project(AlphabetWasmLsp LANGUAGES CXX)

# This CMakeLists.txt is a thin wrapper that uses the main project's source
# tree to compile the Alphabet Language Server (src/lsp.cpp + lexer/parser)
# into a Node.js-friendly WASM binary. We deliberately do NOT use
# add_subdirectory() on the main project — we need to compile the same
# .cpp files but with emcc + a Node-targeted linker flag set, which
# conflicts with the main project's native-binary build.
#
# Usage:
#   emcmake cmake -S . -B build-wasm-lsp \
#       -DCMAKE_TOOLCHAIN_FILE=<emsdk>/.../Emscripten.cmake
#   cmake --build build-wasm-lsp --target alphabet_lsp

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Path to the main Alphabet project. CMakeLists.txt lives at
# editors/vscode-alphabet/build-tools/wasm-lsp/, so we need 4 ".." to
# reach the Alphabet_Language project root.
get_filename_component(ALPHABET_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../.." ABSOLUTE)
message(STATUS "Alphabet project root: ${ALPHABET_ROOT}")

include_directories(
    ${ALPHABET_ROOT}/src/include
    ${ALPHABET_ROOT}/build/generated
)

# Locate emcc/em++/emar from EMSDK.
get_filename_component(EMSCRIPTEN_BIN_DIR "$ENV{EMSDK}/upstream/emscripten" ABSOLUTE)
if(NOT EXISTS "${EMSCRIPTEN_BIN_DIR}/emcc")
    message(FATAL_ERROR "Cannot find emcc in ${EMSCRIPTEN_BIN_DIR}. Source emsdk_env.sh first.")
endif()
set(CMAKE_C_COMPILER   "${EMSCRIPTEN_BIN_DIR}/emcc")
set(CMAKE_CXX_COMPILER "${EMSCRIPTEN_BIN_DIR}/em++")
set(CMAKE_AR           "${EMSCRIPTEN_BIN_DIR}/emar")
set(CMAKE_RANLIB       "${EMSCRIPTEN_BIN_DIR}/emranlib")

# rcs = insert/replace, create if missing, write symbol table
set(CMAKE_STATIC_LINKER_FLAGS "rcs" CACHE STRING "Archive flags for emar")

# Version header stub (real one comes from main project build).
if(NOT EXISTS "${ALPHABET_ROOT}/build/generated/version.h")
    file(WRITE "${ALPHABET_ROOT}/build/generated/version.h"
        "#pragma once\n#define ALPHABET_VERSION \"0.0.0-dev\"\n")
endif()

# Library sources — same as main CMakeLists.txt but stripped of files that
# need OS syscalls the LSP loop doesn't use.
set(LSP_LIB_SOURCES
    ${ALPHABET_ROOT}/src/compiler.cpp
    ${ALPHABET_ROOT}/src/lexer.cpp
    ${ALPHABET_ROOT}/src/parser.cpp
    ${ALPHABET_ROOT}/src/type_system.cpp
    ${ALPHABET_ROOT}/src/lsp.cpp
    ${ALPHABET_ROOT}/src/linter.cpp
    ${ALPHABET_ROOT}/src/bytecode_io.cpp
    ${ALPHABET_ROOT}/src/nl_to_code.cpp
)

# Compile sources to .o files using em++. We bypass add_library(STATIC)
# because the Emscripten toolchain + CMake's static-lib rule has a
# longstanding bug where <AR_FLAGS> doesn't get substituted, leaving
# the literal "AR_FLAGS" in the command line. We compile manually.
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/lsp_objects")
set(LSP_OBJECTS "")
foreach(src ${LSP_LIB_SOURCES})
    get_filename_component(name "${src}" NAME_WE)
    set(obj "${CMAKE_BINARY_DIR}/lsp_objects/${name}.o")
    add_custom_command(
        OUTPUT "${obj}"
        COMMAND ${CMAKE_CXX_COMPILER}
            -DFOR_WASM=1
            -s USE_PTHREADS=0
            -O2
            -std=c++17
            -I${ALPHABET_ROOT}/src/include
            -I${ALPHABET_ROOT}/build/generated
            -c "${src}"
            -o "${obj}"
        DEPENDS "${src}"
        COMMENT "Compiling ${name}.cpp (Emscripten)"
    )
    list(APPEND LSP_OBJECTS "${obj}")
endforeach()

add_custom_target(alphabet_lsp_wasm_objects ALL DEPENDS ${LSP_OBJECTS})

# Archive the objects into a static lib using emar directly.
add_custom_command(
    OUTPUT "${CMAKE_BINARY_DIR}/libalphabet_lsp_wasm.a"
    COMMAND ${CMAKE_AR} ${CMAKE_STATIC_LINKER_FLAGS}
        "${CMAKE_BINARY_DIR}/libalphabet_lsp_wasm.a"
        ${LSP_OBJECTS}
    DEPENDS ${LSP_OBJECTS}
    COMMENT "Archiving libalphabet_lsp_wasm.a (Emscripten)"
)
add_custom_target(alphabet_lsp_wasm_archive ALL
    DEPENDS "${CMAKE_BINARY_DIR}/libalphabet_lsp_wasm.a")

# LSP entry executable — link directly with em++ bypassing cmake linker.
# Output goes to .js (Emscripten wrapper); emcc also produces .wasm alongside.
add_custom_command(
    OUTPUT "${CMAKE_BINARY_DIR}/alphabet.js"
    COMMAND ${CMAKE_CXX_COMPILER}
        -DFOR_WASM=1
        -s USE_PTHREADS=0
        -std=c++17
        -O2
        -s EXPORTED_FUNCTIONS='["_alphabet_lsp_main","_malloc","_free"]'
        -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap","UTF8ToString","stringToUTF8","lengthBytesUTF8","getValue","setValue"]'
        -s ALLOW_MEMORY_GROWTH=1
        -s MAXIMUM_MEMORY=2gb
        -s MODULARIZE=1
        -s EXPORT_NAME='AlphabetLspModule'
        -s WASM=1
        -s ENVIRONMENT=node
        -s NODERAWFS=1
        -s ASSERTIONS=0
        -I${ALPHABET_ROOT}/src/include
        -I${ALPHABET_ROOT}/build/generated
        "${ALPHABET_ROOT}/src/wasm_lsp_main.cpp"
        ${LSP_OBJECTS}
        -o "${CMAKE_BINARY_DIR}/alphabet.js"
    DEPENDS ${LSP_OBJECTS} "${ALPHABET_ROOT}/src/wasm_lsp_main.cpp"
    COMMENT "Linking alphabet.js + alphabet.wasm (Emscripten)"
)
add_custom_target(alphabet_lsp ALL
    DEPENDS "${CMAKE_BINARY_DIR}/alphabet.js"
    COMMENT "Alphabet LSP WASM build complete")
add_dependencies(alphabet_lsp alphabet_lsp_wasm_objects)
