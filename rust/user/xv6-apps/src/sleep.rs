#![no_main]
#![no_std]

use panic_halt as _;
use xv6_usys::{sleep, exit};


#[no_mangle]
pub extern "C" fn main() -> isize {

    sleep(10);
    exit(0);
}
