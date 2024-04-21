const std = @import("std");
const math = std.math;
pub const earth_radius = 6372.8;

pub fn square(a: f64) f64 {
    return a * a;
}

pub fn radFromDeg(deg: f64) f64 {
    return 0.01745329251994329577 * deg;
}

pub fn referenceHaversine(x0: f64, y0: f64, x1: f64, y1: f64) f64 {
    var lat1 = y0;
    var lat2 = y1;
    const lon1 = x0;
    const lon2 = x1;

    const d_lat = radFromDeg(lat2 - lat1);
    const d_lon = radFromDeg(lon2 - lon1);
    lat1 = radFromDeg(lat1);
    lat2 = radFromDeg(lat2);
    const a = square(@sin(d_lat / 2.0)) + @cos(lat1) * @cos(lat2) * square(@sin(d_lon / 2.0));
    const c = 2.0 * math.asin(math.sqrt(a));

    return earth_radius * c;
}
