import os

from cffi import FFI

ffi = FFI()

ffi.set_source("pytrace._trace",
               "#include <libtrace.h>",
               libraries=["c", "trace"],
               )

with open(os.path.join(os.path.dirname(__file__), "libtrace.h")) as f:
    ffi.cdef(f.read())

if __name__ == "__main__":
    ffi.compile()
