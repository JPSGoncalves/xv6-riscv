use core::ffi::{c_int, c_uint, c_void, c_char, c_short};

// Structure equivalent to `struct stat`
#[repr(C)]
pub struct stat {
    // Add fields matching the C `struct stat` definition
    _data: [u8; 0]
}

// System call bindings
extern "C" {
    pub fn fork() -> c_int;
    pub fn exit(status: c_int) -> !; // `!` for noreturn functions
    pub fn wait(status: *mut c_int) -> c_int;
    pub fn pipe(fds: *mut c_int) -> c_int;
    pub fn write(fd: c_int, buf: *const c_void, count: c_int) -> c_int;
    pub fn read(fd: c_int, buf: *mut c_void, count: c_int) -> c_int;
    pub fn close(fd: c_int) -> c_int;
    pub fn kill(pid: c_int, sig: c_int) -> c_int;
    pub fn exec(path: *const c_char, argv: *mut *mut c_char) -> c_int;
    pub fn open(path: *const c_char, flags: c_int) -> c_int;
    pub fn mknod(path: *const c_char, mode: c_short, dev: c_short) -> c_int;
    pub fn unlink(path: *const c_char) -> c_int;
    pub fn fstat(fd: c_int, buf: *mut stat) -> c_int;
    pub fn link(oldpath: *const c_char, newpath: *const c_char) -> c_int;
    pub fn mkdir(path: *const c_char) -> c_int;
    pub fn chdir(path: *const c_char) -> c_int;
    pub fn dup(fd: c_int) -> c_int;
    pub fn getpid() -> c_int;
    pub fn sbrk(increment: c_int) -> *mut c_void;
    pub fn sleep(seconds: c_int) -> c_int;
    pub fn uptime() -> c_int;
}

// Additional utilities in ulib.c
extern "C" {
    pub fn stat(path: *const c_char, buf: *mut stat) -> c_int;
    pub fn strcpy(dest: *mut c_char, src: *const c_char) -> *mut c_char;
    pub fn memmove(dest: *mut c_void, src: *const c_void, count: c_int) -> *mut c_void;
    pub fn strchr(s: *const c_char, c: c_char) -> *mut c_char;
    pub fn strcmp(s1: *const c_char, s2: *const c_char) -> c_int;
    pub fn fprintf(fd: c_int, format: *const c_char, ...) -> c_int;
    pub fn printf(format: *const c_char, ...) -> c_int;
    pub fn gets(buf: *mut c_char, max: c_int) -> *mut c_char;
    pub fn strlen(s: *const c_char) -> c_uint;
    pub fn memset(dest: *mut c_void, c: c_int, count: c_uint) -> *mut c_void;
    pub fn malloc(size: c_uint) -> *mut c_void;
    pub fn free(ptr: *mut c_void);
    pub fn atoi(s: *const c_char) -> c_int;
    pub fn memcmp(s1: *const c_void, s2: *const c_void, n: c_uint) -> c_int;
    pub fn memcpy(dest: *mut c_void, src: *const c_void, count: c_uint) -> *mut c_void;
}
