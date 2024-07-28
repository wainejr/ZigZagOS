const std = @import("std");
const testing = std.testing;
const queue = @import("queue.zig");
const zz = @import("zigzagos_core.zig");

export const z_queue_size = queue.queue_size;
export const z_queue_print = queue.queue_print;
export const z_queue_append = queue.queue_append;
export const z_queue_remove = queue.queue_remove;

export const z_ppos_init = zz.ppos_init;

export fn add(a: i32, b: i32) i32 {
    return a + b;
}

test "basic add functionality" {
    try testing.expect(add(3, 7) == 10);
}
