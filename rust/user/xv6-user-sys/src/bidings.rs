use core::ffi::{c_int, c_void};

extern "C" {
    pub fn sleep(ms: c_int) -> c_int;
    pub fn exit(code: c_int) -> !;
    pub fn write(fd: c_int, data: *const c_void, len: c_int) -> c_int;
}
