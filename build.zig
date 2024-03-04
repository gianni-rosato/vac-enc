const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    // create the executable
    const exe = b.addExecutable(.{
        .name = "vac-enc",
        .target = target,
        .optimize = optimize,
    });

    // include the "include" directory containing wavreader.h
    exe.addIncludePath(.{ .path = "include" });

    // add our C source files, main.c & wavreader.c
    exe.addCSourceFiles(.{
        .files = &.{
            "src/main.c",
            "src/wavreader.c",
        },
        .flags = &.{
            "-std=c99",
        },
    });

    // link libc
    exe.linkLibC();

    // add system libraries for opus, libopusenc, and soxr from the pkg-config
    exe.linkSystemLibrary("opus");
    exe.linkSystemLibrary("libopusenc");
    exe.linkSystemLibrary("soxr");

    b.installArtifact(exe);
}
