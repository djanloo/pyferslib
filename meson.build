project(
    'ferslib',
    ['c', 'cpp'],
    version: '1.0.0',
    default_options: ['c_std=c11', 'cpp_std=c++17', 'warning_level=2'],
)

cc  = meson.get_compiler('c')
cpp = meson.get_compiler('cpp')

inc = include_directories('include')

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
libusb_dep = dependency('libusb-1.0')

ferslib = library(
    'ferslib',
    src_files,
    include_directories: inc,
    dependencies: [thread_dep, libusb_dep],
    install: true,
    version: '1.0.0',
    soversion: '1',
)

# Headers are flat in include/ - no subdir
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
    name: 'ferslib',
    description: 'FERS hardware access library',
)