#![no_main]
#![no_std]

use core::ffi::{c_int, c_char};
use xv6rs_user::{sleep, exit};

use panic_halt as _;

#[no_mangle]
pub extern "C" fn main() {

    sleep(10);
    //exit(0)
}
