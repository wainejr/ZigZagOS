const std = @import("std");
const stdout = std.io.getStdOut().writer();
const stderr = std.io.getStdErr().writer();

const c = @cImport({
    @cInclude("queue.h");
});

pub export fn queue_size(queue: [*c]c.queue_t) i32 {
    var n_elems: i32 = 0;
    const ini_elem: ?*c.queue_t = queue;
    var elem: ?*c.queue_t = queue;
    while (elem != null) {
        n_elems += 1;
        elem = elem.?.*.next;
        if (elem == ini_elem) {
            break;
        }
    }
    return n_elems;
}

const call_print = ?*const fn (?*anyopaque) callconv(.C) void;

pub export fn queue_print(name: ?[*:0]u8, queue_arg: [*c]c.queue_t, print_elem: call_print) void {
    // std.debug.print("string len {d}\n", name.?.len);
    const queue: ?*c.queue_t = queue_arg;
    stdout.print("{s}", .{name.?}) catch {};
    stdout.print("[", .{}) catch {};
    if (queue == null) {
        stdout.print("]\n", .{}) catch {};
        return;
    }
    const ini_elem = queue.?;
    var elem = queue.?;
    while (true) {
        print_elem.?(elem);
        elem = elem.next;
        if (elem == ini_elem) {
            break;
        }
        stdout.print(" ", .{}) catch {};
    }
    stdout.print("]\n", .{}) catch {};
}

pub export fn queue_append(queue_arg: [*c][*c]c.queue_t, elem_arg: [*c]c.queue_t) i32 {
    const queue: ?*?*c.queue_t = queue_arg;
    const elem: ?*c.queue_t = elem_arg;

    if (queue == null) {
        stderr.print("queue is null\n", .{}) catch {};
        return -1;
    }
    if (elem == null) {
        stderr.print("elem is null\n", .{}) catch {};
        return -1;
    }
    if (elem.?.next != null or elem.?.prev != null) {
        stderr.print("elem is in another queue\n", .{}) catch {};
        return -1;
    }

    // Empty queue
    if (queue.?.* == null) {
        queue.?.* = elem.?;
        elem.?.next = elem;
        elem.?.prev = elem;
        return 0;
    }
    const first_elem = queue.?.*.?;
    const last_elem: *c.struct_queue_t = first_elem.prev;

    elem.?.next = first_elem;
    elem.?.prev = last_elem;
    first_elem.prev = elem.?;
    last_elem.next = elem.?;

    return 0;
}

pub export fn queue_remove(queue_arg: [*c][*c]c.queue_t, elem_arg: [*c]c.queue_t) i32 {
    const queue: ?*?*c.queue_t = queue_arg;
    const elem: ?*c.queue_t = elem_arg;

    if (queue == null) {
        stderr.print("queue is null\n", .{}) catch {};
        return -1;
    }
    if (queue.?.* == null) {
        stderr.print("queue is empty\n", .{}) catch {};
        return -1;
    }
    if (elem == null) {
        stderr.print("elem is null\n", .{}) catch {};
        return -1;
    }

    // Check is in queue
    var is_elem_in = false;
    var elem_aux = queue.?.*.?;
    while (true) {
        if (elem_aux == elem) {
            is_elem_in = true;
            break;
        }
        elem_aux = elem_aux.next.?;
        if (elem_aux == queue.?.*.?) {
            break;
        }
    }
    if (!is_elem_in) {
        stderr.print("elem not in queue\n", .{}) catch {};
        return -1;
    }

    defer {
        elem.?.next = null;
        elem.?.prev = null;
    }

    if (queue.?.* == elem and elem.?.next == elem) {
        queue.?.* = null;
        return 0;
    }

    if (queue.?.* == elem) {
        queue.?.* = elem.?.next;
    }
    elem.?.next.*.prev = elem.?.prev;
    elem.?.prev.*.next = elem.?.next;

    return 0;
}
