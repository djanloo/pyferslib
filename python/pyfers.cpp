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

PYBIND11_MODULE(pyfers, m)
{
    m.def("open_device", &open_device);
}