import sys
sys.path.append("builddir")
sys.path.append("build")

import pyfers

h = pyfers.open_device("eth:192.18")