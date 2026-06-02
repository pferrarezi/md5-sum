const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const exe = b.addExecutable(.{
        .name = "md5sum",
        .root_module = b.createModule(.{
            .root_source_file = b.path("src/main.zig"),
            .target = target,
            .optimize = optimize,
        }),
    });

    b.installArtifact(exe);

    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |args| run_cmd.addArgs(args);

    const run_step = b.step("run", "Run md5sum");
    run_step.dependOn(&run_cmd.step);

    const windows_exe = b.addExecutable(.{
        .name = "md5sum",
        .root_module = b.createModule(.{
            .root_source_file = b.path("src/main.zig"),
            .target = b.resolveTargetQuery(.{
                .cpu_arch = .x86_64,
                .os_tag = .windows,
            }),
            .optimize = optimize,
        }),
    });
    const install_windows = b.addInstallArtifact(windows_exe, .{});

    const windows_step = b.step("windows", "Build md5sum.exe for x86_64 Windows");
    windows_step.dependOn(&install_windows.step);
}
