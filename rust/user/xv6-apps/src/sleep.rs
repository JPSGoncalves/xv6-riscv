#![no_main]
#![no_std]

use xv6_usys::{sleep, exit};

use panic_halt as _;

#[no_mangle]
pub extern "C" fn main() {

    sleep(10);
    //exit(0)
}
