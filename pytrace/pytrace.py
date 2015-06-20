from _trace import ffi, lib


class Trace(object):
    def __init__(self, uri):
        if not isinstance(uri, str):
            raise TypeError("uri must be string (got %r)" % (uri, ))

        trace = lib.trace_create(uri)
        if trace == ffi.NULL:
            raise MemoryError("Could not allocate trace")
        self._trace = ffi.gc(trace, lib.trace_destroy)

        pkt = lib.trace_create_packet()
        if pkt == ffi.NULL:
            raise MemoryError("Could not allocate packet")
        self._pkt = ffi.gc(pkt, lib.trace_destroy_packet)
