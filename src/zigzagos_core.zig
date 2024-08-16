const c = @cImport({
    @cInclude("queue.h");
    @cInclude("ppos.h");
    @cInclude("signal.h");
    @cInclude("sys/time.h");
});

const stdout = std.io.getStdOut().writer();
const stderr = std.io.getStdErr().writer();

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
const quantum_size = 20;

const Status = enum { ready, waiting, finished, sleep };

var q_task_ready: [*c]c.task_t = null;
var q_task_waiting: [*c]c.task_t = null;
var q_task_finished: [*c]c.task_t = null;
var q_task_sleep: [*c]c.task_t = null;

var g_timer: c.itimerval = undefined;
var g_timer_action: c.struct_sigaction = undefined;

fn task_status_queue(status: Status) [*c][*c]c.task_t {
    switch (status) {
        Status.ready => return &q_task_ready,
        Status.waiting => return &q_task_waiting,
        Status.finished => return &q_task_finished,
        Status.sleep => return &q_task_sleep,
    }
}

fn status2number(status: Status) c_short {
    switch (status) {
        Status.ready => return 0,
        Status.waiting => return 1,
        Status.finished => return 2,
        Status.sleep => return 3,
    }
}

fn number2status(number: c_short) Status {
    switch (number) {
        0 => return Status.ready,
        1 => return Status.waiting,
        2 => return Status.finished,
        3 => return Status.sleep,
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
    update_sleep_tasks();

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
    task_run.*.quantum = quantum_size;
    task_run.*.n_activations += 1;
    update_task_status(task_curr.?, Status.ready);
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
    const queue_sleep = task_status_queue(Status.sleep);
    _ = queue_lib.queue_remove(@ptrCast(queue_ready), @ptrCast(@constCast(&task_dispatcher)));

    while (queue_lib.queue_size(@ptrCast(queue_ready.*)) > 0 or queue_lib.queue_size(@ptrCast(queue_sleep.*)) > 0) {
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
        .exit_code = 0,
        .prio_static = 0,
        .prio_dyn = 0,
        .quantum = quantum_size,
        .sys_task = false,
        .n_activations = 0,
        .exec_time = 0,
        .start_time = systime(),
        .proc_time = 0,
        .awake_time = -1,
        .tasks_waiting = null,
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

var total_time: u32 = 0;

pub export fn systime() u32 {
    return total_time;
}

fn update_sleep_tasks() void {
    const n_sleep = queue_lib.queue_size(@ptrCast(q_task_sleep));
    var t_sleep_check = q_task_sleep;
    for (0..@intCast(n_sleep)) |_| {
        const next_task_check = t_sleep_check.*.next;
        if (systime() >= t_sleep_check.*.awake_time) {
            task_awake(t_sleep_check, &q_task_sleep);
        }
        t_sleep_check = next_task_check;
    }
}

fn timer_handler(_: i32) callconv(.C) void {
    total_time += 1;

    if (task_curr == null) {
        return;
    }
    task_curr.?.proc_time += 1;
    task_curr.?.exec_time = systime() - task_curr.?.start_time;

    if (task_curr.?.*.sys_task) {
        return;
    }
    task_curr.?.*.quantum -= 1;
    if (task_curr.?.*.quantum == 0) {
        task_yield();
    }
}

fn init_timer() void {
    // registra a ação para o sinal de timer SIGALRM (sinal do timer)
    g_timer_action.__sigaction_handler.sa_handler = timer_handler;
    _ = c.sigemptyset(&g_timer_action.sa_mask);
    g_timer_action.sa_flags = 0;
    if (c.sigaction(c.SIGALRM, &g_timer_action, 0) < 0) {
        // c.perror("Erro em sigaction: ");
        std.process.exit(1);
    }
    // ajusta valores do temporizador
    g_timer.it_value.tv_usec = 10; // primeiro disparo, em micro-segundos
    g_timer.it_value.tv_sec = 0; // primeiro disparo, em segundos
    g_timer.it_interval.tv_usec = 1000; // disparos subsequentes, em micro-segundos
    g_timer.it_interval.tv_sec = 0; // disparos subsequentes, em segundos

    // arma o temporizador ITIMER_REAL
    if (c.setitimer(c.ITIMER_REAL, &g_timer, 0) < 0) {
        // perror("Erro em setitimer: ");
        std.process.exit(1);
    }
}
pub export fn ppos_init() void {
    // c.setvbuf(c.stdout, 0, c._IONBF, 0);
    inner_create_task(&task_main);
    task_curr = &task_main;

    _ = task_init(&task_dispatcher, dispatcher, @ptrCast(@constCast("Dispatcher")));
    task_dispatcher.sys_task = true;

    init_timer();
    _ = task_switch(&task_dispatcher);
}

pub export fn task_sleep(t: i32) void {
    task_curr.?.awake_time = @intCast(systime());
    task_curr.?.awake_time += t;
    task_curr.?.status = status2number(Status.sleep);
    task_suspend(task_status_queue(Status.sleep));
}

pub export fn task_suspend(queue: [*c][*c]c.task_t) void {
    _ = queue_lib.queue_remove(@ptrCast(task_status_queue(Status.ready)), @ptrCast(task_curr));
    _ = queue_lib.queue_append(@ptrCast(queue), @ptrCast(task_curr));
    task_yield();
}

pub export fn task_awake(task: [*c]c.task_t, queue: [*c][*c]c.task_t) void {
    task.*.status = status2number(Status.ready);
    _ = queue_lib.queue_remove(@ptrCast(queue), @ptrCast(task));
    _ = queue_lib.queue_append(@ptrCast(task_status_queue(Status.ready)), @ptrCast(task));
}

pub export fn task_init(task: [*c]c.task_t, start_func: ?*const fn () callconv(.C) void, arg: ?*anyopaque) i32 {
    // const ztask: *c.task_t = task.?;
    inner_create_task(task);
    c.makecontext(&(task.*.context), start_func, 1, arg);

    return task.*.id;
}

pub export fn task_wait(task: [*c]c.task_t) i32 {
    if (task.*.status == status2number(Status.finished)) {
        return task.*.exit_code;
    }

    task_suspend(@ptrCast(&(task.*.tasks_waiting)));

    return task.*.exit_code;
}

pub export fn task_switch(task: [*c]c.task_t) i32 {
    const prev_task = task_curr.?;
    task_curr = task;
    task_curr.?.n_activations += 1;
    return c.swapcontext(&(prev_task.context), &(task.*.context));
}

pub export fn task_id() i32 {
    return task_curr.?.id;
}

pub export fn task_exit(exit_code: i32) void {
    const t = task_curr.?;
    t.*.exec_time = systime() - t.start_time;
    t.*.exit_code = exit_code;

    while (t.*.tasks_waiting != null) {
        const tw: [*c]c.task_t = @ptrCast(@alignCast(t.*.tasks_waiting));
        tw.*.status = status2number(Status.ready);
        task_awake(tw, @ptrCast(&(t.*.tasks_waiting)));
    }

    update_task_status(task_curr.?, Status.finished);
    stdout.print("Task {} exit: execution time {} ms, processor time {} ms, {} activations\n", .{ t.id, t.exec_time, t.proc_time, t.n_activations }) catch {};

    if (task_curr.?.id == task_dispatcher.id) {
        std.process.exit(if (exit_code == 0) 0 else 255);
    }
    _ = task_switch(&task_dispatcher);
}

pub export fn task_yield() void {
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
