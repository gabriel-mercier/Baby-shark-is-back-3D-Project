#include "terrain.hpp"
#include <cmath>
#include <vector>
#include <cstdlib>
#include <ctime>  
#include <unordered_set>

using namespace cgp;


// Fonction pour évaluer la hauteur des dunes
float evaluate_dune_height(float x, float y) {
    int const n = 15;
    vec2 p_i[n] = { {-25,-25}, {20,20}, {-10,15}, {15,15}, {-20,-20},
                    {0,0}, {-15,10}, {10,-15}, {-5,20}, {25,25},
                    {-25,0}, {0,25}, {25,-25}, {20,0}, {0,-20} };
    float h_i[n] = { 2.0f, -1.0f, 1.5f, 2.5f, 4.0f,
                     0.5f, 1.0f, 2.0f, -0.5f, 8.0f,
                     1.0f, 2.0f, -1.5f, 3.0f, 2.0f };
    float sigma_i[n] = { 24.0f, 9.0f, 12.0f, 12.0f, 15.0f,
                         6.0f, 9.0f, 12.0f, 15.0f, 18.0f,
                         9.0f, 9.0f, 12.0f, 12.0f, 9.0f };
    float s = 0.0;
    float d = 0.0;
    for (int i = 0; i < n; i++) {
        d = norm(vec2{ x, y } - p_i[i]) / sigma_i[i];
        s += h_i[i] * std::exp(-d * d);
    }

    return s;
}

mesh create_dune_mesh(int N, float size) {
    mesh terrain = mesh_primitive_grid(vec3(-size / 2, -size / 2, 0), vec3(size / 2, -size / 2, 0), vec3(size / 2, size / 2, 0), vec3(-size / 2, size / 2, 0), N, N);
    for (size_t i = 0; i < terrain.position.size(); ++i) { // Modify the heights of the vertices
        terrain.position[i].z = evaluate_dune_height(terrain.position[i].x, terrain.position[i].y);
    }
    terrain.fill_empty_field();

    return terrain;
}

float rand_interval(float min, float max) {
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

std::vector<cgp::vec3> generate_positions_on_terrain(int N, float terrain_radius, float trans, float bordure) {
    std::vector<cgp::vec3> list;
    float x, y;

    std::srand(static_cast<unsigned int>(std::time(0)));

    for (int i = 0; i < N; i++) {
        float num = terrain_radius / 2 - bordure;
        x = rand_interval(-num,num );
        y = rand_interval(-num, num);
        list.push_back(cgp::vec3{ x, y, evaluate_dune_height(x, y)-trans });
    }

    return list;
}

std::vector<vec3> generate_positions_around_points(int N, float terrain_radius, float trans, const std::vector<vec3>& points, float spread) {
    std::vector<vec3> list;
    float x, y;

    std::srand(static_cast<unsigned int>(std::time(0)));

    for (int i = 0; i < N; i++) {
        const vec3& center = points[rand() % points.size()];

        x = center.x + rand_interval(-spread, spread);
        y = center.y + rand_interval(-spread, spread);

        if (fabs(x)<terrain_radius/2 && fabs(y) < terrain_radius/2)

        {
            float distance = std::sqrt(std::pow(x - center.x, 2) + std::pow(y - center.y, 2));
            float probability = std::exp(-std::pow(distance / spread, 2)); // Gaussian distribution

            if (rand_interval(0.0f, 1.0f) < probability) {
                list.push_back(vec3{ x, y, evaluate_dune_height(x, y) - trans });
            }
            else {
                i--;
            }
        }
        else {
            i--;
        }
    }

    return list;
}