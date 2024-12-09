use cc;
use std::path::Path;

fn main() {
    // Use the `cc` crate to build a C file and statically link it.
    cc::Build::new()
        .file(
            "/home/jpgoncalves/source/xv6-riscv/user/ulib.c",
        )
        .file(
            "/home/jpgoncalves/source/xv6-riscv/user/usys.S",
        )
        .file(
            "/home/jpgoncalves/source/xv6-riscv/user/printf.c",
        )
        .file(
            "/home/jpgoncalves/source/xv6-riscv/user/umalloc.c",
        )
        .include(Path::new(
            "/home/jpgoncalves/source/xv6-riscv/user",
        ))
        .include(Path::new(
            "/home/jpgoncalves/source/xv6-riscv/kernel",
        ))
        .include(Path::new(
            "/home/jpgoncalves/source/xv6-riscv",
        ))
        .flag("-ffreestanding")
        .flag("-nostdlib")
        .flag("-mabi=lp64d")
        .flag("-mno-relax")
        .flag("-fno-common")
        .flag("-mcmodel=medany")
        .compile("xv6-usys");

    // Add linker arguments to retain all symbols in the library
    println!("cargo:rustc-link-arg=-Wl,--whole-archive");
    println!("cargo:rustc-link-lib=static=xv6-usys");
    println!("cargo:rustc-link-arg=-Wl,--no-whole-archive");
}
