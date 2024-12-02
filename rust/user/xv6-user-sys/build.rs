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
        .compile("xv6-usys");
}
