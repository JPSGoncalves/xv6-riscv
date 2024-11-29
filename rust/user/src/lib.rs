#![no_main]
#![no_std]

use panic_halt as _;

mod bidings;

pub fn sleep(sec: i32) -> i32 {
    unsafe { bidings::sleep(sec) }
}

pub fn exit(code: i32) -> ! {
    unsafe { bidings::exit(code) }
}

