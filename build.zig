const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const strip = b.option(bool, "strip", "Whether to strip symbols from the binary, defaults to true") orelse true;

    const bin = b.addExecutable(.{
        .name = "vac-enc",
        .target = target,
        .optimize = .ReleaseFast, // Enforce ReleaseFast as the default
        .link_libc = true,
        .strip = strip,
    });

    // If using Windows, add the include path for the win32 directory
    if (target.result.os.tag == .windows) {
        bin.addIncludePath(b.path("win32"));
    }

    bin.addCSourceFiles(.{
        .files = &.{
            "src/decode.c",
            "src/flac.c",
            "src/main.c",
            "src/unicode_support.c",
            "src/wavreader.c",
        },
        .flags = &.{
            "-std=c99",
            "-D_POSIX_C_SOURCE=200809L",
        },
    });

    bin.linkSystemLibrary("libopusenc");
    bin.linkSystemLibrary("opus");
    bin.linkSystemLibrary("soxr");

    b.installArtifact(bin);
}
