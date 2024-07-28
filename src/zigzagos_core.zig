const c = @cImport({
    @cInclude("queue.h");
    @cInclude("ppos.h");
});
const std = @import("std");
var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
const allocator = arena.allocator();

var t_ptrs = std.ArrayList(struct { ptr: []u8, id: i32 }).init(allocator);

var task_main: c.task_t = undefined;
var task_curr: ?*c.task_t = null;
var gid: i32 = 0;
const stack_size = 1024 * 64;

pub export fn ppos_init() void {
    // c.setvbuf(c.stdout, 0, c._IONBF, 0);
    task_main = c.task_t{
        .id = gid,
        .next = null,
        .prev = null,
        .status = 0,
    };

    _ = c.getcontext(&(task_main.context));
    const stack = allocator.alloc(u8, stack_size) catch {
        return;
    };
    t_ptrs.append(.{ .ptr = stack, .id = 0 }) catch {
        return;
    };

    task_main.context.uc_stack.ss_sp = @ptrCast(stack);
    task_main.context.uc_stack.ss_size = stack_size;
    task_main.context.uc_stack.ss_flags = 0;
    task_main.context.uc_link = 0;

    gid += 1;
    task_curr = &task_main;
}
// c.task_init

pub export fn task_init(task: [*c]c.task_t, start_func: ?*const fn () callconv(.C) void, arg: ?*anyopaque) i32 {
    task.*.id = gid;
    task.*.status = 0;
    task.*.next = null;
    task.*.prev = null;
    _ = c.getcontext(&(task.*.context));

    const stack = allocator.alloc(u8, stack_size) catch {
        return -1;
    };
    t_ptrs.append(.{ .ptr = stack, .id = gid }) catch {
        return -1;
    };

    task.*.context.uc_stack.ss_sp = @ptrCast(stack);
    task.*.context.uc_stack.ss_size = stack_size;
    task.*.context.uc_stack.ss_flags = 0;
    task.*.context.uc_link = 0;
    c.makecontext(&(task.*.context), start_func, 1, arg);
    gid += 1;

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
    for (t_ptrs.items, 0..) |t, idx| {
        if (t.id == task_id()) {
            // allocator.free(t.ptr);
            _ = t_ptrs.orderedRemove(idx);
            break;
        }
    } else {
        std.process.exit(0);
    }

    if (task_curr == &task_main) {
        arena.deinit();
        std.process.exit(if (exit_code == 0) 0 else 1);
    }
    _ = task_switch(&task_main);
}
