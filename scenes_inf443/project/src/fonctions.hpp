#pragma once

#include "cgp/cgp.hpp"

cgp::vec4 mesh_center(cgp::mesh const& m, float scaling);
std::pair<float, cgp::vec2> distance_to_closest_border_with_normal(const cgp::vec2& point, const std::vector<cgp::vec2>& limits);

float angle_between(const cgp::vec3& v1, const cgp::vec3& v2);

double rand_interval(double a, double b);

double plateau_decay(double x, double plateau_level, double plateau_end, double decay_rate);