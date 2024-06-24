#version 330 core

// Inputs coming from VBOs
layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_normal;
layout (location = 2) in vec3 vertex_color;
layout (location = 3) in vec2 vertex_uv;

// Output variables sent to the fragment shader
out struct fragment_data
{
    vec3 position; // vertex position in world space
    vec3 normal;   // normal in world space
    vec3 color;    // vertex color 
    vec2 uv;       // vertex uv
} fragment;

// Uniform variables expected to receive from the C++ program
uniform mat4 model; // Model matrix
uniform mat4 view;  // View matrix of the camera
uniform mat4 projection; // Projection matrix
uniform float time;

void main()
{
    // Adjusting the deformation factor based on the vertex y position
    // Increase deformation exponentially as y increases
    float amplitude = 0.1 + 0.05 * vertex_position.y; // Amplitude increases with y
    float waveNumber = 4.0; // Frequency of the wave along the y-axis
    float speed = 6.0; // Speed of the wave

    // Position in the model's local space including the oscillation deformation
    float deformation = amplitude * cos(speed * time + vertex_position.y * waveNumber);
    vec3 deformed_position = vertex_position + vec3(deformation, 0, 0); // Applying deformation along the x-axis

    // Convert local position to world position
    vec4 world_position = model * vec4(deformed_position, 1.0);

    // Normal transformation to world space
    mat4 modelNormal = transpose(inverse(model));
    vec4 world_normal = modelNormal * vec4(vertex_normal, 0.0);

    // Calculate the projected position
    vec4 position_projected = projection * view * world_position;

    // Fill the fragment shader inputs
    fragment.position = world_position.xyz;
    fragment.normal = world_normal.xyz;
    fragment.color = vertex_color;
    fragment.uv = vertex_uv;

    // Output the final transformed position
    gl_Position = position_projected;
}
