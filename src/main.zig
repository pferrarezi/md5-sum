const std = @import("std");
const builtin = @import("builtin");

const Md5 = std.crypto.hash.Md5;
const READ_CHUNK = 64 * 1024;

/// Hashes a single file (or stdin when `path` is "-") and prints the result in
/// GNU coreutils `md5sum` format: `<hex digest>  <path>`. Returns true on
/// success, false on any failure (the message is sent to stderr).
fn hashFile(path: []const u8) bool {
    const is_stdin = std.mem.eql(u8, path, "-");

    var file: std.fs.File = undefined;
    if (is_stdin) {
        file = std.fs.File.stdin();
    } else {
        file = std.fs.cwd().openFile(path, .{}) catch |err| {
            std.debug.print("md5sum: {s}: {s}\n", .{ path, @errorName(err) });
            return false;
        };
    }
    defer if (!is_stdin) file.close();

    var md5 = Md5.init(.{});
    var buf: [READ_CHUNK]u8 = undefined;
    while (true) {
        const n = file.read(&buf) catch {
            std.debug.print("md5sum: {s}: read error\n", .{path});
            return false;
        };
        if (n == 0) break;
        md5.update(buf[0..n]);
    }

    var digest: [Md5.digest_length]u8 = undefined;
    md5.final(&digest);

    var line: [Md5.digest_length * 2 + 2]u8 = undefined;
    const hex = std.fmt.bytesToHex(digest, .lower);
    @memcpy(line[0..hex.len], &hex);
    line[hex.len] = ' ';
    line[hex.len + 1] = ' ';

    const out = std.fs.File.stdout();
    out.writeAll(&line) catch return false;
    out.writeAll(path) catch return false;
    out.writeAll("\n") catch return false;
    return true;
}

pub fn main() !void {
    // Make the Windows console interpret our UTF-8 output correctly.
    // Harmless when stdout is redirected to a file or pipe.
    if (builtin.os.tag == .windows) {
        _ = std.os.windows.kernel32.SetConsoleOutputCP(65001); // CP_UTF8
    }

    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();
    const allocator = arena.allocator();

    // argsAlloc decodes the native (UTF-16 on Windows) argv into UTF-8.
    const args = try std.process.argsAlloc(allocator);

    var ok = true;
    if (args.len < 2) {
        ok = hashFile("-");
    } else {
        for (args[1..]) |arg| {
            if (!hashFile(arg)) ok = false;
        }
    }

    std.process.exit(if (ok) 0 else 1);
}
