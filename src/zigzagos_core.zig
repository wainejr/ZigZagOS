const c = @cImport({
    @cInclude("queue.h");
    @cInclude("ppos_data.h");
    @cInclude("ppos.h");
});
const std = @import("std");

pub export fn ppos_init() void {
    std.debug.print("im in init\n", .{});
}
