const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const lib = b.addLibrary(.{
        .name = "cfl",
        .root_module = b.createModule(.{
            .target = target,
            .optimize = optimize,
        }),
        .linkage = .static,
    });

    lib.addIncludePath(b.path("cfl-headers/src/main/headers"));
    lib.linkLibC();

    const c_sources = [_][]const u8{
        "cfl_array.c",
        "cfl_atomic.c",
        "cfl_bitmap.c",
        "cfl_btree.c",
        "cfl_buffer.c",
        "cfl_date.c",
        "cfl_error.c",
        "cfl_event.c",
        "cfl_hash.c",
        "cfl_iterator.c",
        "cfl_list.c",
        "cfl_llist.c",
        "cfl_lock.c",
        "cfl_log.c",
        "cfl_map.c",
        "cfl_map_str.c",
        "cfl_mem.c",
        "cfl_number.c",
        "cfl_process.c",
        "cfl_socket.c",
        "cfl_sql.c",
        "cfl_str.c",
        "cfl_sync_queue.c",
        "cfl_thread.c",
    };

    lib.addCSourceFiles(.{
        .root = b.path("cfl-lib/src/main/c"),
        .files = &c_sources,
        .flags = &.{ "-std=c99", "-Wall", "-Wextra" },
    });

    b.installArtifact(lib);

    // ZIP Generation
    const zip_step = b.step("zip", "Generate a ZIP file with lib and include folders");

    // Create a directory for the ZIP contents
    const zip_dir = b.addWriteFiles();
    const lib_file = lib.getEmittedBin();
    _ = zip_dir.addCopyFile(lib_file, "lib/libcfl.a"); // Ensure name is standard for the zip

    // Add headers
    const headers_path = "cfl-headers/src/main/headers";
    _ = zip_dir.addCopyDirectory(b.path(headers_path), "include", .{});

    // Run tar to create zip
    const zip_cmd = b.addSystemCommand(&.{ "tar", "-acvf" });
    const zip_file = zip_cmd.addOutputFileArg("cfl-library.zip");
    zip_cmd.addArg("-C");
    zip_cmd.addDirectoryArg(zip_dir.getDirectory());
    zip_cmd.addArgs(&.{ "lib", "include" });

    // Install the zip file
    const install_zip = b.addInstallFile(zip_file, "cfl-library.zip");
    zip_step.dependOn(&install_zip.step);

    // Tests
    const test_files = [_][]const u8{
        "test_cfl_array.c",
        "test_cfl_atomic.c",
        "test_cfl_bitmap.c",
        "test_cfl_btree.c",
        "test_cfl_buffer.c",
        "test_cfl_date.c",
        "test_cfl_error.c",
        "test_cfl_event.c",
        "test_cfl_hash.c",
        "test_cfl_iterator.c",
        "test_cfl_list.c",
        "test_cfl_llist.c",
        "test_cfl_lock.c",
        "test_cfl_log.c",
        "test_cfl_map.c",
        "test_cfl_map_str.c",
        "test_cfl_mem.c",
        "test_cfl_number.c",
        "test_cfl_os.c",
        "test_cfl_process.c",
        "test_cfl_socket.c",
        "test_cfl_sql.c",
        "test_cfl_str.c",
        "test_cfl_sync_queue.c",
        "test_cfl_thread.c",
    };

    const test_step = b.step("test", "Run all tests");

    for (test_files) |test_file| {
        const test_exe = b.addExecutable(.{
            .name = test_file[0 .. test_file.len - 2], // Remove .c
            .root_module = b.createModule(.{
                .target = target,
                .optimize = optimize,
            }),
        });

        test_exe.addCSourceFile(.{ .file = b.path(b.fmt("tests/{s}", .{test_file})), .flags = &.{ "-std=c99" } });
        test_exe.addIncludePath(b.path("cfl-headers/src/main/headers"));
        test_exe.addIncludePath(b.path("tests"));
        test_exe.linkLibrary(lib);
        test_exe.linkLibC();

        const run_cmd = b.addRunArtifact(test_exe);
        test_step.dependOn(&run_cmd.step);
    }
}
