#version 330 core

// Fragment shader
in struct fragment_data {
    vec3 position;  // Position in the world space
    vec3 normal;    // Normal in the world space
    vec3 color;     // Current color on the fragment
    vec2 uv;        // Current uv-texture on the fragment
} fragment;

// Output color
layout(location=0) out vec4 FragColor;

// Uniforms
uniform sampler2D image_texture;    // Texture image
uniform mat4 view;                  // View matrix
uniform vec3 light;                 // Light position
uniform vec3 fogColor = vec3(0.2, 0.5, 0.6); // Fog color
uniform float d_max = 50.0;        // Max distance for full fog effect

// Phong and texture settings
struct phong_structure {
    float ambient;
    float diffuse;
    float specular;
    float specular_exponent;
};
struct texture_settings_structure {
    bool use_texture;
    bool texture_inverse_v;
    bool two_sided;
};
struct material_structure {
    vec3 color;
    float alpha;
    phong_structure phong;
    texture_settings_structure texture_settings;
}; 
uniform material_structure material;

void main() {
    // Compute the position of the camera
    mat3 O = transpose(mat3(view));
    vec3 last_col = vec3(view * vec4(0.0, 0.0, 0.0, 1.0));
    vec3 camera_position = -O * last_col;

    // Distance from the fragment to the camera
    float distance = length(fragment.position - camera_position);
    float alpha_f = min(distance / d_max, 1.0);

    // Normal calculations
    vec3 N = normalize(fragment.normal);
    if (material.texture_settings.two_sided && gl_FrontFacing == false) {
        N = -N;
    }

    // Lighting calculations
    vec3 L = normalize(light - fragment.position);
    float diffuse_component = max(dot(N, L), 0.0);
    float specular_component = 0.0;
    if (diffuse_component > 0.0) {
        vec3 R = reflect(-L, N);
        vec3 V = normalize(camera_position - fragment.position);
        specular_component = pow(max(dot(R, V), 0.0), material.phong.specular_exponent);
    }

    // Texture handling
    vec2 uv_image = fragment.uv;
    if (material.texture_settings.texture_inverse_v) {
        uv_image.y = 1.0 - uv_image.y;
    }
    vec4 color_image_texture = texture(image_texture, uv_image);
    if (!material.texture_settings.use_texture) {
        color_image_texture = vec4(1.0, 1.0, 1.0, 1.0);
    }

    // Final color calculations
    vec3 color_object = fragment.color * material.color * color_image_texture.rgb;
    vec3 color_shading = (material.phong.ambient + material.phong.diffuse * diffuse_component) * color_object + material.phong.specular * specular_component * vec3(1.0, 1.0, 1.0);

    // Apply fog effect
    vec3 final_color = mix(color_shading, fogColor, alpha_f);

    // Set the output color
    FragColor = vec4(final_color, material.alpha * color_image_texture.a);
}
