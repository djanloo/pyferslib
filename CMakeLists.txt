cmake_minimum_required(VERSION 3.16)

project(ferslib VERSION 1.0.0 LANGUAGES C CXX)

# -------------------------
# Standard + flags globali
# -------------------------
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT WIN32)
    add_compile_definitions(_POSIX_C_SOURCE=200809L)
endif()

# warning level (approx meson warning_level:2)
if (MSVC)
    add_compile_options(/W3)
else()
    add_compile_options(-Wall -Wextra)
endif()

# -------------------------
# Include dirs
# -------------------------
include_directories(include)

# -------------------------
# Dependencies
# -------------------------
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)


if(NOT WIN32)
    pkg_check_modules(LIBUSB REQUIRED libusb-1.0)
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
    PUBLIC include
    PRIVATE ${LIBUSB_INCLUDE_DIRS}
)

if(WIN32)
    target_link_libraries(ferslib PRIVATE
        ws2_32      
        winusb      
        setupapi    
    )
endif()

target_link_libraries(ferslib
    PRIVATE Threads::Threads
    PRIVATE ${LIBUSB_LIBRARIES}
)

set_target_properties(ferslib PROPERTIES
    VERSION 1.0.0
    SOVERSION 1
)

# This is for stdbool in win with mingw
if(WIN32)
    target_compile_definitions(ferslib PRIVATE _MINGW_HAS_STDBOOL)
    target_compile_options(ferslib PRIVATE -include stdbool.h)
endif()

# This set the declspec to dllexport
if(WIN32)
    target_compile_definitions(ferslib PRIVATE _CAEN_FERS_EXPORT)
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

target_link_libraries(test_load ferslib)

# -------------------------
# pkg-config equivalent
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
# pybind11 module
# -------------------------

# This is used in the presets to disable the binding for 'lib-only' preset
option(BUILD_PYTHON_BINDINGS "Build Python bindings" ON)

if(BUILD_PYTHON_BINDINGS)
    find_package(Python COMPONENTS Interpreter Development)

    execute_process(
        COMMAND python3 -m pybind11 --cmakedir
        OUTPUT_VARIABLE pybind11_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    
    set(PYBIND11_FINDPYTHON ON)
    find_package(pybind11 CONFIG REQUIRED)

    pybind11_add_module(pyfers
        python/pyfers.cpp
    )

    target_link_libraries(pyfers PRIVATE ferslib)
    set_target_properties(pyfers PROPERTIES
                                INTERPROCEDURAL_OPTIMIZATION ON
                                CXX_VISIBILITY_PRESET hidden
                                VISIBILITY_INLINES_HIDDEN ON)


    install(TARGETS pyfers
        LIBRARY DESTINATION .
    )
endif()


