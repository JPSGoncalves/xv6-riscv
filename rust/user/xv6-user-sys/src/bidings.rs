use core::ffi::{c_int};

extern "C" {
    pub fn sleep(ms: c_int) -> c_int;
    pub fn exit(code: c_int) -> !;
}
