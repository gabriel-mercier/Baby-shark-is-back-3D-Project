#pragma once

#include "cgp/cgp.hpp"
#include "environment.hpp"
#include "key_positions_structure.hpp"

using cgp::mesh;
using cgp::mesh_drawable;
using cgp::vec3;
using cgp::timer_basic;

struct gui_parameters {
    bool display_frame = true;
    bool display_wireframe = false;
    bool display_volume = false;
};

struct scene_structure : cgp::scene_inputs_generic {
    camera_controller_2d_displacement camera_control; //Comment here to switch camera
    //camera_controller_orbit_euler camera_control; //Uncomment here to switch camera
    camera_projection_perspective camera_projection;
    window_structure window;

    mesh_drawable global_frame;         
    environment_structure environment;  
    input_devices inputs;               
    gui_parameters gui;                  

    // ****************************** //
    // Elements of the scene
    // ****************************** //

    timer_basic timer;
    cgp::timer_interval timer_i;
    cgp::timer_interval timer_i_1;
    cgp::timer_interval timer_i_2;
    keyframe_structure keyframe;
    keyframe_structure keyframe_1;
    keyframe_structure keyframe_2;

    numarray<vec3> key_positions = { {0, 0, 0},{0, 0, 1}, {1, 0, 6}, {1, 0, 9},{2, 0, 12},{2, 1, 15},{2, 1.5, 18}, {1.5, 1, 21},{1.5, 0, 24},{1, 0, 27},{0, 0, 29},{-1, 0, 30} };
    numarray<vec3> key_positions_1 = { {0, 0, 0}, {0, 0, 1}, {1, 0.5, 5}, {1.5, 1, 8}, {2, 1.5, 10}, {2.5, 2, 12}, {2.5, 2.5, 14}, {2, 2, 16}, {1.5, 1.5, 18}, {1, 1, 20}, {0.5, 0.5, 22}, {0, 0, 24} };
    numarray<vec3> key_positions_2 = { {0, 0, 0}, {0, 0, 1}, {0.6, -0.2, 3}, {1, -0.5, 4.5}, {1.5, -0.8, 6}, {2, -1, 7.5}, {2.5, -1.2, 9}, {3, -1.5, 10.5}, {3.5, -1.5, 12}, {4, -1, 13.5}, {4.5, -0.5, 15}, {5, 0, 16} };

    numarray<float> key_times = { 0.0f, 1.0f, 2.0f, 2.5f, 3.0f, 3.5f, 3.75f, 4.5f, 5.0f, 6.0f, 7.0f, 8.0f };
    numarray<float> key_times_1 = { 0.0f, 3.0f, 5.0f, 5.5f, 6.0f, 6.5f, 6.75f, 7.5f, 8.0f, 9.0f, 10.0f, 11.0f };
    numarray<float> key_times_2 = { 0.0f, 7.0f, 8.0f, 8.5f, 9.0f, 9.5f, 9.75f, 10.5f, 11.0f, 12.0f, 13.0f, 14.0f };

    mesh_drawable terrain;
    mesh_drawable chest;
    mesh_drawable fish;
    mesh_drawable crater;
    mesh_drawable bubble;
    mesh_drawable skull;
    mesh_drawable volume;
    mesh_drawable arch;
    mesh_drawable seaw;
    mesh_drawable wall;
    mesh_drawable ceiling;
    mesh_drawable castle;

    std::vector<vec3> p; //Fishes' position
    std::vector<vec3> v; //Fishes' speed
    std::vector<vec4> lmesh_center; //Store the mesh's center for the bounding volume handling collisions
    std::vector<vec3> trans_mesh; //Store the translation for the bounding volume handling collisions

    cgp::skybox_drawable skybox;
    std::map<std::string, mesh_drawable> shapes;
    std::vector<float> random_floats;
    vec3 p_interpolations[3];

    // ****************************** //
    // Functions
    // ****************************** //

    void simulation_step(float dt);
    void initialize();   
    void display_frame(); 
    void display_gui();  
    void gestion_timer();
    void creation_mesh_terrain();
    void creation_skybox();
    void creation_wall();
    void creation_mesh_fish();
    void creation_mesh_decoration();
    void creation_mesh_bubble_crater();
    void display_skybox();
    void display_bubble_wall(vec3 p_interpolations[3]);

    void mouse_move_event();
    void mouse_click_event();
    void keyboard_event();
    void idle_frame();

    void display_info();
};
