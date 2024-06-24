#include "fonctions.hpp"
#include <random>

using namespace cgp;

vec4 mesh_center(mesh const& m, float scaling) {
	vec3 p_min, p_max;
	m.get_bounding_box_position(p_min, p_max);
	vec3 sum = { 0,0,0 };
	sum = (p_min + p_max) / 2;

	float dist = 0;
	for (const vec3& point : m.position) {
		dist += norm(point - sum);
	}
	dist /= m.position.size();
	vec4 ret = { sum[0],sum[1], sum[2], dist };
	return scaling*ret;
}

std::pair<float, vec2> distance_point_to_segment_with_normal(const vec2& p, const vec2& v, const vec2& w) {
	float l2 = (w.x - v.x) * (w.x - v.x) + (w.y - v.y) * (w.y - v.y);
	if (l2 == 0.0) {
		vec2 diff = p - v;
		return { norm(diff), normalize(diff) };
	}
	float t = ((p.x - v.x) * (w.x - v.x) + (p.y - v.y) * (w.y - v.y)) / l2;
	t = std::max(0.0f, std::min(1.0f, t));
	vec2 projection = v + (w - v) * t;
	vec2 diff = p - projection;
	return std::make_pair(norm(diff), normalize(diff));
}

std::pair<float, vec2> distance_to_closest_border_with_normal(const cgp::vec2& point, const std::vector<cgp::vec2>& limits) {
	float min_distance = std::numeric_limits<float>::max();
	vec2 closest_normal(0, 0);

	for (size_t i = 0; i < limits.size(); ++i) {
		vec2 v = limits[i];
		vec2 w = limits[(i + 1) % limits.size()]; 
		std::pair<float, vec2> result = distance_point_to_segment_with_normal(point, v, w);
		float distance = result.first;
		vec2 normal = result.second;
		if (distance < min_distance) {
			min_distance = distance;
			closest_normal = normal;
		}
	}
	return std::make_pair(min_distance, closest_normal);
}

float angle_between(const vec3& v1, const vec3& v2) {
	float dot_product = dot(v1,v2);
	float lengths = norm(v1) * norm(v2);
	float cosine_angle = dot_product / lengths;
	return acos(cosine_angle); 
}

double rand_interval(double a, double b) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis(a, b);
	return dis(gen);
}

double plateau_decay(double x, double plateau_level, double plateau_end, double decay_rate) {
	if (x <= plateau_end) {
		return plateau_level;
	}
	else {
		return plateau_level * std::exp(-decay_rate * (x - plateau_end));
	}
}
