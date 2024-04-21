const std = @import("std");
const print = std.debug.print;

const hav = @import("./haver_ref.zig");

const TokenKind = enum {
    l_brace,
    r_brace,
    comma,
    colon,
    l_bracket,
    r_bracket,
    string,
    number,
    keyword_true,
    keyword_false,
    keyword_null,
};

const Token = struct {
    kind: TokenKind,
    string: []const u8,
    number: ?f64,
};

inline fn isAlpha(char: u8) bool {
    return ((char >= 'a' and char <= 'z') or
        (char >= 'A' and char <= 'Z') or
        (char == '_'));
}

inline fn isDigit(char: u8) bool {
    return char >= '0' and char <= '9';
}

pub fn tokenize(allocator: std.mem.Allocator, string: []const u8) []Token {
    var tokens = std.ArrayList(Token).init(allocator);
    var i: u32 = 0;

    while (i < string.len) : (i += 1) {
        const token_start = i;
        var token_number: ?f64 = null;

        const token_kind: TokenKind = switch (string[i]) {
            '{' => .l_brace,
            '}' => .r_brace,
            ',' => .comma,
            ':' => .colon,
            '[' => .l_bracket,
            ']' => .r_bracket,
            '"' => t: {
                i += 1;
                while (i < string.len and string[i] != '"') {
                    i += 1;
                }
                break :t .string;
            },
            ' ', '\n', '\r', '\t' => continue,
            else => t: {
                if (isAlpha(string[i])) {
                    while (i < string.len and isAlpha(string[i])) {
                        i += 1;
                    }
                    i -= 1;

                    const ident = string[token_start .. i + 1];
                    if (std.mem.eql(u8, ident, "true")) {
                        break :t .keyword_true;
                    }
                    if (std.mem.eql(u8, ident, "false")) {
                        break :t .keyword_false;
                    }
                    if (std.mem.eql(u8, ident, "null")) {
                        break :t .keyword_null;
                    }
                }
                if (string[i] == '-' or isDigit(string[i])) {
                    i += 1;
                    while (i < string.len and
                        (isDigit(string[i]) or
                        string[i] == '.' or
                        string[i] == 'e' or
                        string[i] == 'E'))
                    {
                        i += 1;
                    }

                    i -= 1;

                    token_number = std.fmt.parseFloat(
                        f64,
                        string[token_start .. i + 1],
                    ) catch {
                        @panic("Failed to parse number!");
                    };

                    break :t .number;
                }

                @panic("Unexpected character!");
            },
        };

        const token = Token{
            .kind = token_kind,
            .string = if (token_kind == .string)
                string[token_start + 1 .. i]
            else
                string[token_start .. i + 1],
            .number = token_number,
        };

        tokens.append(token) catch {
            @panic("Failed to append token!");
        };
    }

    return tokens.toOwnedSlice() catch {
        @panic("Failed to convert to owned slice!");
    };
}

const JsonValueKind = enum {
    string,
    number,
    boolean,
    object,
    array,
    nil,
};

fn printIndent(indent: u8) void {
    for (0..indent) |_| {
        print("  ", .{});
    }
}

const JsonValue = union(JsonValueKind) {
    string: []const u8,
    number: f64,
    boolean: bool,
    object: std.StringHashMap(JsonValue),
    array: std.ArrayList(JsonValue),
    nil: void,

    fn printValue(self: JsonValue, indent: u8) void {
        switch (self) {
            .string => |str| {
                print("{s}", .{str});
            },
            .number => |num| {
                print("{d:.2}", .{num});
            },
            .boolean => |b| {
                print("{}", .{b});
            },
            .nil => {
                print("null", .{});
            },
            .object => |map| {
                print("{{\n", .{});
                var iter = map.iterator();
                while (iter.next()) |entry| {
                    printIndent(indent + 1);
                    print("{s} : ", .{entry.key_ptr.*});
                    entry.value_ptr.printValue(indent + 1);
                    print("\n", .{});
                }
                printIndent(indent);
                print("}}", .{});
            },
            .array => |arr| {
                print("[\n", .{});
                for (arr.items) |item| {
                    printIndent(indent + 1);
                    item.printValue(indent + 1);
                    print("\n", .{});
                }
                printIndent(indent);
                print("]", .{});
            },
        }
    }

    fn free(self: JsonValue) void {
        switch (self) {
            .string, .number, .boolean, .nil => {},
            .object => |map| {
                var iter = map.iterator();
                while (iter.next()) |entry| {
                    entry.value_ptr.free();
                }
                var _map = map;
                _map.deinit();
            },
            .array => |arr| {
                for (arr.items) |item| {
                    item.free();
                }
                var _arr = arr;
                _arr.deinit();
            },
        }
    }
};

const ParseResult = struct {
    value: JsonValue,
    tokens_consumed: u32,
};

fn parseValue(allocator: std.mem.Allocator, tokens: []Token) ParseResult {
    var current_token: u32 = 0;

    switch (tokens[current_token].kind) {
        .string => {
            return ParseResult{
                .value = JsonValue{
                    .string = tokens[current_token].string,
                },
                .tokens_consumed = 1,
            };
        },
        .number => {
            return ParseResult{
                .value = JsonValue{
                    .number = tokens[current_token].number orelse {
                        @panic("Number token has no token associated!");
                    },
                },
                .tokens_consumed = 1,
            };
        },
        .keyword_true => {
            return ParseResult{
                .value = JsonValue{
                    .boolean = true,
                },
                .tokens_consumed = 1,
            };
        },
        .keyword_false => {
            return ParseResult{
                .value = JsonValue{
                    .boolean = false,
                },
                .tokens_consumed = 1,
            };
        },
        .keyword_null => {
            return ParseResult{
                .value = @as(JsonValue, .nil),
                .tokens_consumed = 1,
            };
        },
        .l_brace => {
            current_token += 1;
            var records = std.StringHashMap(JsonValue).init(allocator);

            while (true) {
                //- ojf: parse record
                const name_result = parseValue(allocator, tokens[current_token..]);
                current_token += name_result.tokens_consumed;
                const name = switch (name_result.value) {
                    .string => |str| str,
                    else => @panic("Expected string!"),
                };

                if (tokens[current_token].kind != .colon) {
                    @panic("Exected colon!");
                }
                current_token += 1;

                const value_result = parseValue(allocator, tokens[current_token..]);
                current_token += value_result.tokens_consumed;
                const value = value_result.value;

                records.put(name, value) catch {
                    @panic("Error inserting into list");
                };

                if (tokens[current_token].kind == .comma) {
                    current_token += 1;
                } else if (tokens[current_token].kind == .r_brace) {
                    current_token += 1;
                    return ParseResult{
                        .value = JsonValue{
                            .object = records,
                        },
                        .tokens_consumed = current_token,
                    };
                } else {
                    @panic("Expected }} or ,");
                }
            }

            @panic("Not implemented!");
        },
        .l_bracket => {
            current_token += 1;
            var values = std.ArrayList(JsonValue).init(allocator);

            while (true) {
                const value_result = parseValue(allocator, tokens[current_token..]);
                current_token += value_result.tokens_consumed;
                const value = value_result.value;

                values.append(value) catch {
                    @panic("Failed to append value to array");
                };

                if (tokens[current_token].kind == .comma) {
                    current_token += 1;
                } else if (tokens[current_token].kind == .r_bracket) {
                    current_token += 1;
                    return ParseResult{
                        .value = JsonValue{
                            .array = values,
                        },
                        .tokens_consumed = current_token,
                    };
                } else {
                    @panic("Expected ] or ,");
                }
            }
        },
        else => {
            @panic("Unexpected Token!");
        },
    }
}

fn printUsage() void {
    print("Usage: haversine_parser.exe [file name]", .{});
}

pub fn main() void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    var arg_iterator = std.process.argsWithAllocator(allocator) catch {
        @panic("Failed to process args!");
    };
    defer arg_iterator.deinit();

    //- skip exe name
    _ = arg_iterator.next();

    const file = f: {
        const arg_str = arg_iterator.next() orelse {
            printUsage();
            return;
        };

        break :f std.fs.cwd().openFile(arg_str, .{}) catch {
            printUsage();
            return;
        };
    };
    defer file.close();

    const json_string = file.readToEndAlloc(allocator, 128 * 1024 * 1024) catch {
        @panic("Couldn't read json string!");
    };
    defer allocator.free(json_string);

    const tokens = tokenize(allocator, json_string);
    defer allocator.free(tokens);

    const parseResult = parseValue(allocator, tokens);
    var parsed_value = parseResult.value;
    defer parsed_value.free();

    if (@as(JsonValueKind, parsed_value) != .object) {
        @panic("Expected an object with a points array!");
    }

    const points_value = parsed_value.object.get("pairs") orelse {
        @panic("Couldn't find pairs array");
    };
    if (@as(JsonValueKind, points_value) != .array) {
        @panic("Expected an array!");
    }

    const points_array = points_value.array;

    var distance_sum: f64 = 0;

    for (points_array.items) |point_object| {
        if (@as(JsonValueKind, point_object) != .object) {
            @panic("Expected an array of point objects!");
        }

        const x0 = n: {
            const value = point_object.object.get("x0") orelse {
                @panic("Could not find x0");
            };

            if (@as(JsonValueKind, value) != .number) {
                @panic("Expected number!");
            }

            break :n value.number;
        };

        const y0 = n: {
            const value = point_object.object.get("y0") orelse {
                @panic("Could not find y0");
            };

            if (@as(JsonValueKind, value) != .number) {
                @panic("Expected number!");
            }

            break :n value.number;
        };

        const x1 = n: {
            const value = point_object.object.get("x1") orelse {
                @panic("Could not find x1");
            };

            if (@as(JsonValueKind, value) != .number) {
                @panic("Expected number!");
            }

            break :n value.number;
        };

        const y1 = n: {
            const value = point_object.object.get("y1") orelse {
                @panic("Could not find y1");
            };

            if (@as(JsonValueKind, value) != .number) {
                @panic("Expected number!");
            }

            break :n value.number;
        };

        const distance = hav.referenceHaversine(x0, y0, x1, y1);
        distance_sum += distance;
    }

    print("Sum: {d}\n", .{distance_sum / @as(f64, @floatFromInt(points_array.items.len))});
}
