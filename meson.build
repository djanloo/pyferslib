project(
    'ferslib',
    ['c', 'cpp'],
    version: '1.0.0',
    default_options: ['c_std=c11', 'cpp_std=c++17', 'warning_level=2'],
)

cc      = meson.get_compiler('c')
cpp     = meson.get_compiler('cpp')
host_os = host_machine.system()

inc = include_directories('include')

if host_os != 'windows'
    add_project_arguments('-D_POSIX_C_SOURCE=200809L', language: ['c', 'cpp'])
endif

src_files = files(
    'src/FERS_LLeth.c',
    'src/FERS_LLtdl.c',
    'src/FERS_LLusb.cpp',
    'src/FERS_MultiPlatform.c',
    'src/FERS_adapters.c',
    'src/FERS_configure.c',
    'src/FERS_configure_5202.c',
    'src/FERS_configure_5203.c',
    'src/FERS_configure_5204.c',
    'src/FERS_paramparser.c',
    'src/FERS_readout.c',
    'src/FERSlib.c',
)

thread_dep = dependency('threads')

if host_os == 'windows'
    platform_deps = [
        cc.find_library('ws2_32'),
        cc.find_library('winusb'),
        cc.find_library('setupapi'),
    ]
    platform_args = ['-D_CAEN_FERS_EXPORT']
else
    platform_deps = [dependency('libusb-1.0')]
    platform_args = []
endif

ferslib = library(
    'ferslib',
    src_files,
    include_directories: inc,
    dependencies: [thread_dep] + platform_deps,
    c_args:   platform_args,
    cpp_args: platform_args,
    install:  true,
    version:  '1.0.0',
    soversion: '1',
)

ferslib_dep = declare_dependency(
    link_with:         ferslib,
    include_directories: inc,
)

executable(
    'test_load',
    'tests/test_load.c',
    dependencies: ferslib_dep,
)

install_headers(
    'include/ExportTypes.h',
    'include/FERS_config.h',
    'include/FERSlib.h',
    'include/FERS_LL.h',
    'include/FERS_LLusb.h',
    'include/FERS_MultiPlatform.h',
    'include/FERS_paramparser.h',
    'include/FERS_Registers_520X.h',
    'include/FERS_Registers_5215.h',
)

pkg = import('pkgconfig')
pkg.generate(
    ferslib,
    name:        'ferslib',
    description: 'FERS hardware access library',
)

py           = import('python').find_installation('python3')
pybind11_dep = dependency('pybind11')

pyfers_args = host_os == 'windows' ? ['-DNOMINMAX', '-DWIN32_LEAN_AND_MEAN'] : []

py.extension_module(
    'pyfers',
    'python/pyfers.cpp',
    dependencies: [pybind11_dep, ferslib_dep],
    cpp_args: pyfers_args,
    install:  true,
)
