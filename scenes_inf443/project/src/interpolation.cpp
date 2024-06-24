#include "interpolation.hpp"


using namespace cgp;

vec3 cardinal_spline_interpolation(float t, float t0, float t1, float t2, float t3, vec3 const& p0, vec3 const& p1, vec3 const& p2, vec3 const& p3, float K);

int find_index_of_interval(float t, numarray<float> const& intervals);


vec3 interpolation(float t, numarray<vec3> const& key_positions, numarray<float> const& key_times) {
    int idx = find_index_of_interval(t, key_times);

    if (idx < 1 ) {
        return key_positions[0];
    }
    if (idx >= key_times.size() - 2) {
        return key_positions[key_times.size() - 1];
    }

    float t0 = key_times[idx - 1]; 
    float t1 = key_times[idx]; 
    float t2 = key_times[idx + 1]; 
    float t3 = key_times[idx + 2];

    vec3 const& p0 = key_positions[idx - 1]; 
    vec3 const& p1 = key_positions[idx]; 
    vec3 const& p2 = key_positions[idx + 1]; 
    vec3 const& p3 = key_positions[idx + 2]; 

    float K = 0.5f; //Tension

    vec3 p = cardinal_spline_interpolation(t, t0, t1, t2, t3, p0, p1, p2, p3, K);

    return p;
}

vec3 cardinal_spline_interpolation(float t, float t0, float t1, float t2, float t3, vec3 const& p0, vec3 const& p1, vec3 const& p2, vec3 const& p3, float K)
{
    float const s = (t - t1) / (t2 - t1);

    vec3 const d1 = 2.0f * K * (p2 - p0) / (t2 - t0);
    vec3 const d2 = 2.0f * K * (p3 - p1) / (t3 - t1);

    vec3 const p = (2 * s * s * s - 3 * s * s + 1) * p1
        + (s * s * s - 2 * s * s + s) * d1
        + (-2 * s * s * s + 3 * s * s) * p2
        + (s * s * s - s * s) * d2;

    return p;
}
int find_index_of_interval(float t, numarray<float> const& intervals) {
    int const N = intervals.size();
    if (N < 2) {
        std::cerr << "Error: Intervals should have at least two values; current size=" << intervals.size() << std::endl;
        return -1; 
    }
    if (t < intervals[0] || t > intervals[N - 1]) {
        std::cerr << "Error: current time t=" << t << " is out of bounds [" << intervals[0] << ", " << intervals[N - 1] << "]" << std::endl;
        return -1; 
    }

    int k = 0;
    while (k + 1 < N && intervals[k + 1] < t)
        ++k;

    // Ensure we do not go out of bounds
    if (k < 0 || k >= N - 1) {
        std::cerr << "Error: Computed index " << k << " is out of valid range [0, " << N - 2 << "]" << std::endl;
        return -1;
    }

    return k;
}

