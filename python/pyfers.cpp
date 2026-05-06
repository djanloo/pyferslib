#include <pybind11/pybind11.h>
#include "FERSlib.h"
#undef max
#undef min

#include <string>
#include <thread>
#include <pybind11/numpy.h>

namespace py = pybind11;

int open_device(const std::string& path)
{
    int handle = -1;
    std::string path_buf = path;  // mutable copy — FERS_OpenDevice takes char*
    int ret;
    {
        py::gil_scoped_release release;
        ret = FERS_OpenDevice(path_buf.data(), &handle);
    }
    if (ret != 0) {
        char err[1024] = {0};
        FERS_GetLastError(err);
        throw std::runtime_error(err);
    }
    return handle;
}

py::dict get_board_info(int handle)
{
    FERS_BoardInfo_t info;
    memset(&info, 0, sizeof(info));
    int ret;
    {
        py::gil_scoped_release release;
        ret = FERS_GetBoardInfo(handle, &info);
    }
    if (ret != 0) {
        char err[1024] = {0};
        FERS_GetLastError(err);
        throw std::runtime_error(err);
    }
    py::dict d;
    d["pid"]     = info.pid;
    d["model"]   = std::string(info.ModelName);
    d["fpga_fw"] = info.FPGA_FWrev;
    d["uc_fw"]   = info.uC_FWrev;
    return d;
}

int close_device(int handle)
{
    int ret;
    {
        py::gil_scoped_release release;
        ret = FERS_CloseDevice(handle);
    }
    if (ret != 0) {
        char err[1024] = {0};
        FERS_GetLastError(err);
        throw std::runtime_error(err);
    }
    return 0;
}

void init_tdl_chains(int handle, py::array_t<float, py::array::c_style | py::array::forcecast> arr)
{
    if (arr.ndim() != 2)
        throw std::runtime_error("DelayAdjust must be a 2D array");
    if (arr.shape(0) != FERSLIB_MAX_NTDL || arr.shape(1) != FERSLIB_MAX_NNODES)
        throw std::runtime_error("Invalid shape for DelayAdjust");

    auto buf = arr.request();
    float (*data)[FERSLIB_MAX_NNODES] = reinterpret_cast<float (*)[FERSLIB_MAX_NNODES]>(buf.ptr);
    int ret;
    {
        py::gil_scoped_release release;
        ret = FERS_InitTDLchains(handle, data);
    }
    if (ret != 0) {
        char err[1024] = {0};
        FERS_GetLastError(err);
        throw std::runtime_error(err);
    }
}

bool tdl_chains_initialized(int handle)
{
    bool ret;
    {
        py::gil_scoped_release release;
        ret = FERS_TDLchainsInitialized(handle);
    }
    return ret;
}

PYBIND11_MODULE(pyfers, m)
{
    m.def("open_device",           &open_device);
    m.def("get_board_info",        &get_board_info);
    m.def("close_device",          &close_device);
    m.def("init_tdl_chains",       &init_tdl_chains);
    m.def("tdl_chains_initialized",&tdl_chains_initialized);
}
