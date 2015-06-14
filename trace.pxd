
cdef extern from "libtrace.h":
    ctypedef struct Trace:
        pass

    Trace* trace_create(const char* uri)
