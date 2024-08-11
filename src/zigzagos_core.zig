const c = @cImport({
    @cInclude("queue.h");
    @cInclude("ppos.h");
});
const queue_lib = @import("queue.zig");
const std = @import("std");
var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
const allocator = arena.allocator();

var t_ptrs = std.ArrayList(struct { ptr: []u8, id: i32 }).init(allocator);

var task_main: c.task_t = undefined;
var task_dispatcher: c.task_t = undefined;
var task_curr: ?*c.task_t = null;
var gid: i32 = 0;
const stack_size = 1024 * 64;

const Status = enum { ready, waiting, finished };

var q_task_ready: [*c]c.task_t = null;
var q_task_waiting: [*c]c.task_t = null;
var q_task_finished: [*c]c.task_t = null;

fn task_status_queue(status: Status) [*c][*c]c.task_t {
    switch (status) {
        Status.ready => return &q_task_ready,
        Status.waiting => return &q_task_waiting,
        Status.finished => return &q_task_finished,
    }
}

fn status2number(status: Status) c_short {
    switch (status) {
        Status.ready => return 0,
        Status.waiting => return 1,
        Status.finished => return 2,
    }
}

fn number2status(number: c_short) Status {
    switch (number) {
        0 => return Status.ready,
        1 => return Status.waiting,
        2 => return Status.finished,
        else => unreachable,
    }
}

fn update_task_status(task: *c.task_t, new_status: Status) void {
    if (task.id == task_dispatcher.id) {
        return;
    }

    const curr_queue = task_status_queue(number2status(task.status));
    if (curr_queue != null) {
        // const curr_queue_t: ?*?*c.queue_t = @ptrCast(&curr_queue);
        _ = queue_lib.queue_remove(@ptrCast(curr_queue), @ptrCast(task));
    }
    const queue_add = task_status_queue(new_status);
    task.status = status2number(new_status);
    _ = queue_lib.queue_append(@ptrCast(queue_add), @ptrCast(task));
}

fn scheduler() [*c]c.task_t {
    if (queue_lib.queue_size(@ptrCast(task_status_queue(Status.ready).*)) == 0) {
        return null;
    }
    const head_task = task_status_queue(Status.ready).*.*.id;
    var task_run: [*c]c.task_t = task_status_queue(Status.ready).*;
    var task_check: [*c]c.task_t = task_run.*.next;

    while (task_check.*.id != head_task) {
        var task_age = task_check;
        if (task_fullprio(task_check) < task_fullprio(task_run)) {
            task_age = task_run;
            task_run = task_check;
        }
        task_age.*.prio_dyn -= 1;
        task_check = task_check.*.next;
    }
    task_run.*.prio_dyn = 0;
    return task_run;
}

fn deallocate_task_stack(tid: i32) void {
    for (t_ptrs.items, 0..) |t, idx| {
        if (t.id == tid) {
            allocator.free(t.ptr);
            _ = t_ptrs.orderedRemove(idx);
            break;
        }
    } else {
        @panic("why am i here");
    }
}

// ?*const fn () callconv(.C) void
fn dispatcher() callconv(.C) void {
    const queue_ready = task_status_queue(Status.ready);
    _ = queue_lib.queue_remove(@ptrCast(queue_ready), @ptrCast(@constCast(&task_dispatcher)));

    while (queue_lib.queue_size(@ptrCast(queue_ready.*)) > 0) {
        const next_task = scheduler();
        if (next_task != null) {
            _ = task_switch(next_task);
            if (number2status(next_task.*.status) == Status.finished) {
                deallocate_task_stack(next_task.*.id);
            }
        }
    }
    task_exit(0);
}

fn inner_create_task(task: [*c]c.task_t) void {
    // c.setvbuf(c.stdout, 0, c._IONBF, 0);
    task.* = c.task_t{
        .id = gid,
        .next = null,
        .prev = null,
        .status = status2number(Status.ready),
        .prio_static = 0,
        .prio_dyn = 0,
    };

    _ = c.getcontext(&(task.*.context));
    const stack = allocator.alloc(u8, stack_size) catch {
        return;
    };
    t_ptrs.append(.{ .ptr = stack, .id = task.*.id }) catch {
        return;
    };

    task.*.context.uc_stack.ss_sp = @ptrCast(stack);
    task.*.context.uc_stack.ss_size = stack_size;
    task.*.context.uc_stack.ss_flags = 0;
    task.*.context.uc_link = 0;

    gid += 1;

    const queue_add = task_status_queue(Status.ready);
    _ = queue_lib.queue_append(@ptrCast(queue_add), @ptrCast(@constCast(task)));
}

pub export fn ppos_init() void {
    // c.setvbuf(c.stdout, 0, c._IONBF, 0);
    inner_create_task(&task_main);
    task_curr = &task_main;

    _ = task_init(&task_dispatcher, dispatcher, @ptrCast(@constCast("Dispatcher")));
    _ = task_switch(&task_dispatcher);
}

pub export fn task_init(task: [*c]c.task_t, start_func: ?*const fn () callconv(.C) void, arg: ?*anyopaque) i32 {
    // const ztask: *c.task_t = task.?;
    inner_create_task(task);
    c.makecontext(&(task.*.context), start_func, 1, arg);

    return task.*.id;
}

pub export fn task_switch(task: [*c]c.task_t) i32 {
    const prev_task = task_curr.?;
    task_curr = task;
    return c.swapcontext(&(prev_task.context), &(task.*.context));
}

pub export fn task_id() i32 {
    return task_curr.?.id;
}

pub export fn task_exit(exit_code: i32) void {
    update_task_status(task_curr.?, Status.finished);

    if (task_curr.?.id == task_dispatcher.id) {
        std.process.exit(if (exit_code == 0) 0 else 255);
    }
    _ = task_switch(&task_dispatcher);
}

pub export fn task_yield() void {
    update_task_status(task_curr.?, Status.ready);
    _ = task_switch(&task_dispatcher);
}

fn task_fullprio(task: [*c]c.task_t) i32 {
    return task.*.prio_dyn + task.*.prio_static;
}

pub export fn task_setprio(task: [*c]c.task_t, prio: c_int) void {
    if (task == null) {
        return task_setprio(task_curr, prio);
    }
    task.*.prio_static = prio;
}

pub export fn task_getprio(task: [*c]c.task_t) i32 {
    if (task == null) {
        return task_getprio(task_curr);
    }
    return task.*.prio_static;
}
