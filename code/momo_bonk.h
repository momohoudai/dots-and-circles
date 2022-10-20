/* date = April 26th 2021 5:36 pm */

#ifndef MOMO_BONK_H
#define MOMO_BONK_H

static inline b8
IsPointWithinCircle(Circ2f c, Vec2f pt){
    f32 dist_sq = Vec_DistanceSq(c.origin, pt);
    return dist_sq < (c.radius * c.radius);
}


// TODO(Momo): shift to circle
static inline b8
IsCircleOnCircle(Circ2f lhs, Circ2f rhs) {
	f32 dist_sq = Vec_DistanceSq(lhs.origin, rhs.origin);
	f32 radius_sq = lhs.radius + rhs.radius;
    radius_sq *= radius_sq;
	return dist_sq < radius_sq;
}

static inline b8
IsCircleOnLine(Circ2f circle, Line2f line) {
    // NOTE(Momo): Extend the ends of the lines based on radius of the circle, and use that to form a parametric equation of the line (ray)
    Ray2f ray = Line_ToRay(line);
    Vec2f normalized_dir = Vec_Normalize(ray.dir);
    line.min = line.min - (normalized_dir * circle.radius);
    line.max = line.max + (normalized_dir * circle.radius);
    ray = Line_ToRay(line);
    
    Vec2f origin_to_circle = circle.origin - ray.origin;
    Vec2f origin_to_closest_pt_on_line = Vec_Project(origin_to_circle, ray.dir); 
    Vec2f closest_pt_on_line = ray.origin + origin_to_closest_pt_on_line;
    
    // NOTE(Momo): Find the time of intersection of the closest_pt_on_line
    f32 time = {}; 
    if (!IsEqual(ray.dir.x, 0.f)) {
        time = (closest_pt_on_line.x - ray.origin.x)/ray.dir.x;
    }
    else if (!IsEqual(ray.dir.y, 0.f)) {
        time = (closest_pt_on_line.y - ray.origin.y)/ray.dir.y;
    }
    else {
        return false;
    }
    
    if (time < 0.f || time > 1.f) {
        return false;
    }
    
    // NOTE(Momo): At this point, we are within range of the line segment, so we just have to check if the circle's radius is greater than its distance from the line.
    f32 circle_radius_sq = circle.radius * circle.radius;
    f32 circle_dist_from_line_sq = Vec_DistanceSq(origin_to_circle, 
                                                  origin_to_closest_pt_on_line);
    return circle_radius_sq > circle_dist_from_line_sq;
}

// NOTE(Momo): DynaCircle is short for 'dynamic circle'
static inline b8
IsMovingCircleOnCircle(Circ2f dyna_circle,
					   Vec2f velocity,
					   Circ2f circle) 
{
    Line2f line = { dyna_circle.origin, dyna_circle.origin + velocity };
    circle.radius += dyna_circle.radius;
    return IsCircleOnLine(circle, line);
}

static inline b8
IsMovingCircleOnMovingCircle(Circ2f circle_a,
							 Vec2f vel_a,
							 Circ2f circle_b,
							 Vec2f vel_b) 
{
    
    f32 speed_a = Vec_LengthSq(vel_a);
    f32 speed_b = Vec_LengthSq(vel_b);
    b8 is_speed_a_zero = IsEqual(speed_a, 0.f);
    b8 is_speed_b_zero = IsEqual(speed_b, 0.f);
    
    
    if (is_speed_a_zero && is_speed_b_zero) {
        return IsCircleOnCircle(circle_a, circle_b);
    }
    else {
        Vec2f relative_vel = vel_b - vel_a; 
        return IsMovingCircleOnCircle(circle_b, relative_vel, circle_a);
    }
}

#endif //MOMO_BONK_H
