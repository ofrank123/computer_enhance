const std = @import("std");

pub fn build(b: *std.Build) !void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    //- ojf: reference code:
    const reference_parser = b.addExecutable(.{
        .name = "REF_haversine_parser",
        .target = target,
        .optimize = optimize,
    });
    reference_parser.addCSourceFile(.{
        .file = .{
            .path = "./reference/simple_haversine_main.cpp",
        },
        .flags = &.{},
    });
    reference_parser.linkLibC();
    b.installArtifact(reference_parser);

    const generator_exe = b.addExecutable(.{
        .name = "haversine_generator",
        .root_source_file = .{ .path = "./generate.zig" },
        .target = target,
        .optimize = optimize,
    });
    generator_exe.linkLibC();
    b.installArtifact(generator_exe);

    const parser_exe = b.addExecutable(.{
        .name = "haversine_parser",
        .root_source_file = .{ .path = "./parser.zig" },
        .target = target,
        .optimize = optimize,
    });
    b.installArtifact(parser_exe);
}
