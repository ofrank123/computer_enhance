const std = @import("std");
const Allocator = std.mem.Allocator;

const math = std.math;

const hav = @import("./haver_ref.zig");

pub fn print(comptime fmt: []const u8, args: anytype) void {
    const stdout = std.io.getStdOut().writer();
    stdout.print(fmt, args) catch return;
}

fn printUsage() void {
    print("Usage: haversine_generator.exe [uniform/cluster] [random seed] [number of coordinate pairs to generate]", .{});
}

const GenerationMode = enum {
    uniform,
    cluster,
};

const Pair = struct {
    x0: f64,
    y0: f64,
    x1: f64,
    y1: f64,
};

fn generateUniformPoint(rand: *std.Random) Pair {
    return .{
        .x0 = 360.0 * rand.float(f64) - 180.0,
        .y0 = 180.0 * rand.float(f64) - 90.0,
        .x1 = 360.0 * rand.float(f64) - 180.0,
        .y1 = 180.0 * rand.float(f64) - 90.0,
    };
}

const ClusterData = struct {
    cluster_point_x: f64 = 0.0,
    cluster_point_y: f64 = 0.0,
    cluster_size: f64 = 0.0,
    change_factor: u64,
};

fn generateClusterPoint(index: u64, rand: *std.Random, cluster_data: *ClusterData) Pair {
    if (index % cluster_data.change_factor == 0) {
        cluster_data.cluster_size = 360.0 * rand.float(f64);

        cluster_data.cluster_point_x = (360.0 - cluster_data.cluster_size) * rand.float(f64) - (180.0 - cluster_data.cluster_size / 2.0);
        cluster_data.cluster_point_y = (180.0 - cluster_data.cluster_size) * rand.float(f64) - (90.0 - cluster_data.cluster_size / 2.0);
    }
    const cluster_point_x = cluster_data.cluster_point_x;
    const cluster_point_y = cluster_data.cluster_point_y;
    const cluster_size = cluster_data.cluster_size;

    return .{
        .x0 = cluster_point_x + cluster_size * rand.float(f64) - cluster_size / 2.0,
        .y0 = cluster_point_y + cluster_size * rand.float(f64) - cluster_size / 2.0,
        .x1 = cluster_point_x + cluster_size * rand.float(f64) - cluster_size / 2.0,
        .y1 = cluster_point_y + cluster_size * rand.float(f64) - cluster_size / 2.0,
    };
}

pub fn main() !void {
    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();
    const allocator: Allocator = arena.allocator();

    var arg_iterator = try std.process.argsWithAllocator(allocator);

    //- ojf: executable
    _ = arg_iterator.next();

    const generation_mode: GenerationMode = mode: {
        const arg_str = arg_iterator.next() orelse {
            printUsage();
            return;
        };

        if (std.mem.eql(u8, arg_str, "uniform")) {
            break :mode .uniform;
        } else if (std.mem.eql(u8, arg_str, "cluster")) {
            break :mode .cluster;
        }

        printUsage();
        return;
    };

    const seed = s: {
        const arg_str = arg_iterator.next() orelse {
            printUsage();
            return;
        };

        break :s std.fmt.parseInt(u64, arg_str, 10) catch {
            printUsage();
            return;
        };
    };

    var rng = std.Random.DefaultPrng.init(seed);
    var rand = rng.random();

    const num_points = n: {
        const arg_str = arg_iterator.next() orelse {
            printUsage();
            return;
        };

        break :n std.fmt.parseInt(u64, arg_str, 10) catch {
            printUsage();
            return;
        };
    };

    //- ojf: write json + reference answers
    const points_file_name = try std.fmt.allocPrint(allocator, "haversine-data_{d}.json", .{num_points});
    const points_file = try std.fs.cwd().createFile(points_file_name, .{});
    defer points_file.close();

    const answers_file_name = try std.fmt.allocPrint(allocator, "haversine-answers_{d}.f64", .{num_points});
    const answers_file = try std.fs.cwd().createFile(answers_file_name, .{});
    defer answers_file.close();

    var haversine_sum: f64 = 0;
    var cluster_data = ClusterData{
        .change_factor = num_points / 64,
    };

    _ = try points_file.write("{\"pairs\":[\n");
    for (0..num_points) |i| {
        const point: Pair = switch (generation_mode) {
            .uniform => p: {
                break :p generateUniformPoint(&rand);
            },
            .cluster => p: {
                break :p generateClusterPoint(i, &rand, &cluster_data);
            },
        };

        const line = try std.fmt.allocPrint(allocator, "{{\"x0\":{d:.16},\"y0\":{d:.16},\"x1\":{d:.16},\"y1\":{d:.16}}}{s}\n", .{
            point.x0,
            point.y0,
            point.x1,
            point.y1,
            if (i == num_points - 1) "" else ",",
        });
        _ = try points_file.write(line);

        //- ojf: accumulate
        const haversine_answer = hav.referenceHaversine(point.x0, point.y0, point.x1, point.y1);
        _ = try answers_file.write(&std.mem.toBytes(haversine_answer));
        haversine_sum += haversine_answer;
    }
    _ = try points_file.write("]}\n");

    print("Method: {s}\n", .{if (generation_mode == .uniform) "Uniform" else "Cluster"});
    print("Random Seed: {d}\n", .{seed});
    print("Pair Count: {d}\n", .{num_points});
    print("Expected Sum: {d}\n", .{haversine_sum / @as(f64, @floatFromInt(num_points))});
}
