#include <pybind11/pybind11.h>
#include "FERSlib.h"

namespace py = pybind11;

int open_device(std::string path)
{
    int handle = -1;

    int ret = FERS_OpenDevice(const_cast<char *>(path.c_str()), &handle);

    if (ret != 0)
    {
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

    int ret = FERS_GetBoardInfo(handle, &info);

    if (ret != 0)
    {
        char err[1024] = {0};
        FERS_GetLastError(err);
        throw std::runtime_error(err);
    }

    py::dict d;

    d["pid"] = info.pid;
    d["model"] = std::string(info.ModelName);
    d["fpga_fw"] = info.FPGA_FWrev;
    d["uc_fw"] = info.uC_FWrev;

    return d;
}

int close_device(int handle)
{
    int ret = FERS_CloseDevice(handle);

    if (ret != 0)
    {
        char err[1024] = {0};
        FERS_GetLastError(err);
        throw std::runtime_error(err);
    }

    return 0;
}

PYBIND11_MODULE(pyfers, m)
{
    m.def("open_device", &open_device);
    m.def("get_board_info", &get_board_info);
    m.def("close_device", &close_device);
}