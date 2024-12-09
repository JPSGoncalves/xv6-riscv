#![no_main]
#![no_std]


use panic_halt as _;
use core::ffi::{c_int, c_void};

mod bidings;

pub fn println(line: &str) {
    unsafe {
        bidings::write(1, line.as_ptr() as *const c_void, line.len() as i32);
        bidings::write(1, "\n".as_ptr() as *const c_void, 1);
    }
}

#[no_mangle]
pub unsafe extern "C" fn main() -> c_int {

    println("Entering sleep from Rust!");
    //sleep(10);
    //println("Goodby from Rust!");
    unsafe { bidings::exit(0) }
}
