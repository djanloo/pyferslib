cmake_minimum_required(VERSION 3.16)

project(ferslib VERSION 1.0.0 LANGUAGES C CXX)

# -------------------------
# C/C++ standards
# -------------------------
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -------------------------
# Compiler flags
# -------------------------
if(NOT WIN32)
    add_compile_definitions(_POSIX_C_SOURCE=200809L)
endif()

if(MSVC)
    add_compile_options(/W3)
else()
    add_compile_options(-Wall -Wextra)
endif()

# -------------------------
# Dependencies
# -------------------------
find_package(Threads REQUIRED)

if(WIN32)
    set(PLATFORM_LIBS ws2_32 winusb setupapi)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBUSB REQUIRED libusb-1.0)
    set(PLATFORM_LIBS         ${LIBUSB_LIBRARIES})
    set(PLATFORM_INCLUDE_DIRS ${LIBUSB_INCLUDE_DIRS})
endif()

# -------------------------
# Sources
# -------------------------
set(FERS_SRC
    src/FERS_LLeth.c
    src/FERS_LLtdl.c
    src/FERS_LLusb.cpp
    src/FERS_MultiPlatform.c
    src/FERS_adapters.c
    src/FERS_configure.c
    src/FERS_configure_5202.c
    src/FERS_configure_5203.c
    src/FERS_configure_5204.c
    src/FERS_paramparser.c
    src/FERS_readout.c
    src/FERSlib.c
)

# -------------------------
# Library
# -------------------------
add_library(ferslib SHARED ${FERS_SRC})

target_include_directories(ferslib
    PUBLIC  include
    PRIVATE ${PLATFORM_INCLUDE_DIRS}
)

target_link_libraries(ferslib
    PRIVATE Threads::Threads
    PRIVATE ${PLATFORM_LIBS}
)

set_target_properties(ferslib PROPERTIES
    VERSION   1.0.0
    SOVERSION 1
)

if(WIN32)
    target_compile_definitions(ferslib PRIVATE _CAEN_FERS_EXPORT)
endif()

if(MINGW)
    target_compile_definitions(ferslib PRIVATE _MINGW_HAS_STDBOOL)
    target_compile_options(ferslib PRIVATE -include stdbool.h)
endif()

# -------------------------
# Install
# -------------------------
install(TARGETS ferslib
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/ DESTINATION include)

# -------------------------
# Test executable
# -------------------------
add_executable(test_load tests/test_load.c)
target_link_libraries(test_load PRIVATE ferslib)

# -------------------------
# pkg-config
# -------------------------
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/ferslib.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/ferslib.pc
    @ONLY
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ferslib.pc
    DESTINATION lib/pkgconfig
)

# -------------------------
# Python bindings (pybind11)
# -------------------------
option(BUILD_PYTHON_BINDINGS "Build Python bindings" ON)

if(BUILD_PYTHON_BINDINGS)
    find_package(pybind11 CONFIG REQUIRED)

    pybind11_add_module(pyfers python/pyfers.cpp)

    target_link_libraries(pyfers PRIVATE ferslib)

    if(WIN32)
        target_compile_definitions(pyfers PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
    endif()

    set_target_properties(pyfers PROPERTIES
        CXX_VISIBILITY_PRESET     hidden
        VISIBILITY_INLINES_HIDDEN ON
    )

    install(TARGETS pyfers LIBRARY DESTINATION .)
endif()
