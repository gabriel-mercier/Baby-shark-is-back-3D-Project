#include "scene.hpp"
#include "terrain.hpp"
#include "fonctions.hpp"
#include "interpolation.hpp"
#include <cmath>
#include <omp.h>

using namespace cgp;

//User - Configurable Parameters
int nb_fish = 100;
int L_terrain = 30;
int nb_seaw = 30;
int nb_islet = 3;
int nb_crater = 30;

//Non - Configurable Parameters
int N_terrain_samples = 100;
float view_height = 1.5f;
int nb_bubble = 3;
float volume_factor = 1.2f;
std::vector<int> crater_bubble_indices(nb_crater);
std::vector<vec3> craters(nb_crater);
std::vector<vec3> seaws(nb_seaw);
std::vector<vec2> limits = { vec2(-L_terrain / 2, -L_terrain / 2), vec2(L_terrain / 2, -L_terrain / 2), vec2(L_terrain / 2, L_terrain / 2), vec2(-L_terrain / 2, L_terrain / 2) };

void scene_structure::simulation_step(float dt) {
	std::vector<vec3> new_p(nb_fish);
	std::vector<vec3> new_v(nb_fish);

#pragma omp parallel for //Parallel calculation
	for (int i = 0; i < nb_fish; i++) {
		vec3& pi = p[i];
		vec3 vi = v[i];
		vec3 F = vec3(0, 0, 0);

		for (int j = 0; j < nb_fish; j++) {
			if (i == j) continue;
			vec3& pj = p[j];
			vec3& vj = v[j];
			float angle = angle_between(vi, pj - pi);
			if (fabs(angle) < Pi / 1.5f) {  //Field of view
				float distance = norm(pi - pj);
				if (distance > 0) {
					if (distance < 1.5f) {
						F += (pi - pj) / pow(distance, 2); //Repulsion
					}
					else if (distance < 2) {
						F += exp(-3 * distance) * (vj - vi); //Same vitess
					}
					else if (distance < 3) {
						F += (pj - pi) * pow(distance, 1); // Attraction
					}
				}
			}
		}
		//Start Comment here to switch camera
		vec3& cam = camera_control.camera_model.position_camera; //Camera repulsion
		float cam_dist = norm(pi - cam);
		if (cam_dist < 6) {
			F += (pi - cam) * plateau_decay(cam_dist, 7, 2.5f, 5);
		}
		//End Comment here to switch camera
		int k = 0;
		for (vec4 position : lmesh_center) { //Collision with scene's objects 
			vec3 place = { position[0], position[1], position[2] };
			place += trans_mesh[k];
			F += (pi - place) * plateau_decay(norm(pi - place), 7, volume_factor * position[3] * L_terrain / 30, 10);
			k++;
		}

		float ground_angle = angle_between(vi, vec3(0, 0, 1)); //Ground's Repulsion
		float z = evaluate_dune_height(p[i][0], p[i][1]);
		vec3 dir = { 0, 0, 1 };
		F += dir * plateau_decay(fabs(z - p[i][2]), 12, 0.75f, 8);

		std::pair<float, vec2> result = distance_to_closest_border_with_normal(vec2(pi.x, pi.y), limits); //Walls Repulsion
		F += vec3(result.second.x, result.second.y, 0) * plateau_decay(result.first, 15, 1.5f * L_terrain / 30, 4);
		float z_dist = fabs((pi.z - L_terrain /2));
		F += vec3(0, 0, -1) * plateau_decay(z_dist, 20, 1 * L_terrain / 30, 8);


		vi = vi + dt * F;

		if (norm(vi) > 4 * L_terrain / 30) vi = 4 * L_terrain / 30 * normalize(vi); //Avoid divergence

		new_v[i] = vi;
		new_p[i] = pi + dt * vi;

		float l = L_terrain / 2;
		float acceptance=0.4f;
		if (std::abs(new_p[i].x) > l+ acceptance || std::abs(new_p[i].y) > l+ acceptance || std::abs(new_p[i].z) > l+ acceptance) { //Deal with out of bound's fish
			float x = rand_interval(-l, l);
			float y = rand_interval(-l, l);
			float z = evaluate_dune_height(x, y);
			new_p[i] = { x, y, rand_interval(1, l - z - 1) + z };
			new_v[i] = vec3(rand_interval(-1, 1), rand_interval(-1, 1), rand_interval(-1, 1));
			std::cerr << "Error : fish out of bound !!" << std::endl;
		}
	}

	for (int i = 0; i < nb_fish; i++) {
		p[i] = new_p[i];
		v[i] = new_v[i];
	}
}

void scene_structure::gestion_timer() { //Bubbles timers/key frames/key positions
	keyframe.initialize(key_positions, key_times);
	keyframe_1.initialize(key_positions_1, key_times_1);
	keyframe_2.initialize(key_positions_2, key_times);

	int N = key_times.size();
	timer_i.t_min = key_times[1];
	timer_i.t_max = key_times[N - 2];
	timer_i.t = timer_i.t_min;
	int N_1 = key_times_1.size();
	timer_i_1.t_min = key_times_1[1];
	timer_i_1.t_max = key_times_1[N_1 - 2];
	timer_i_1.t = timer_i_1.t_min;
	int N_2 = key_times_2.size();
	timer_i_2.t_min = key_times_2[1];
	timer_i_2.t_max = key_times_2[N_2 - 2];
	timer_i_2.t = timer_i_2.t_min;
}

void scene_structure::creation_mesh_terrain() {
	mesh terrain_mesh = create_dune_mesh(N_terrain_samples, L_terrain);
	terrain.initialize_data_on_gpu(terrain_mesh);
	terrain.material.phong.specular = 0.0f;
	terrain.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/sand1.jpg", GL_CLAMP_TO_BORDER,
		GL_CLAMP_TO_BORDER);

}

void scene_structure::creation_skybox() {
	image_structure image_skybox_template = image_load_file(project::path + "assets/skybox2_483.jpg");
	std::vector<image_structure> image_grid = image_split_grid(image_skybox_template, 4, 3);
	skybox.initialize_data_on_gpu();
	skybox.texture.initialize_cubemap_on_gpu(image_grid[1], image_grid[7], image_grid[5], image_grid[3], image_grid[10], image_grid[4]);
	opengl_shader_structure shader_environment_map;
	shader_environment_map.load(project::path + "shaders/environment_map/environment_map.vert.glsl", project::path + "shaders/environment_map/environment_map.frag.glsl");
	for (auto& shape_it : shapes) {
		mesh_drawable& shape = shape_it.second;
		shape.shader = shader_environment_map;
		shape.supplementary_texture["image_skybox"] = skybox.texture;
	}
	skybox.model.rotation = rotation_transform::from_axis_angle(vec3(1, 0, 0), Pi / 2);
}

void scene_structure::creation_wall() {
	float l = L_terrain / 2;
	mesh wall_mesh = mesh_primitive_quadrangle({ l,-l,0 }, { l,l,0 }, { l,l,l }, { l,-l,l });
	wall.initialize_data_on_gpu(wall_mesh);
	wall.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/window_blue.png", GL_REPEAT, GL_REPEAT);
	wall.material.phong.specular = 0;

	mesh ceiling_mesh = mesh_primitive_quadrangle({ -l,-l,0 }, { l,-l,0 }, { l,l,0 }, { -l,l,0 });
	ceiling.initialize_data_on_gpu(ceiling_mesh);
	ceiling.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/noir+star.png", GL_REPEAT, GL_REPEAT);
	ceiling.material.phong.specular = 0;
	ceiling.model.rotation= rotation_transform::from_axis_angle(vec3(1, 0, 0), Pi );
	ceiling.model.translation = vec3(0, 0, l);
}
void scene_structure::creation_mesh_fish() {
	mesh fish_mesh = mesh_load_file_obj(project::path + "assets/Poisson3/shark2.obj");
	fish_mesh.centered();
	fish.initialize_data_on_gpu(fish_mesh);
	fish.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/Poisson3/Sport_Shark_Diffuse.png",GL_REPEAT,GL_REPEAT);
	fish.model.scaling = 0.3f * L_terrain / 30;
	
	opengl_shader_structure shader_custom;
	shader_custom.load(
		project::path + "shaders/mesh_custom/mesh_custom.vert.glsl",
		project::path + "shaders/mesh_custom/mesh_custom.frag.glsl");
	fish.shader = shader_custom;

	p.resize(nb_fish);
	v.resize(nb_fish);
	float l = L_terrain / 2;
	for (int i = 0; i < nb_fish; i++) {
		float x = rand_interval(-l, l);
		float y = rand_interval(-l, l);
		float z = evaluate_dune_height(x, y);
		vec3 p0 = { x, y, rand_interval(1, l-z-1) + z };
		vec3 v0 = vec3(rand_interval(-1, 1), rand_interval(-1, 1), rand_interval(-1, 1));

		p[i] = p0;
		v[i] = v0;
	}
}
void scene_structure::creation_mesh_decoration() {
	mesh volume_mesh = mesh_primitive_sphere(); //Mesh to show bounding volume
	volume.initialize_data_on_gpu(volume_mesh);
	volume.material.alpha = 0.1f;
	volume.material.phong.specular = 0;
	volume.material.color = { 1,0,0 };

	mesh skull_mesh = mesh_load_file_obj(project::path + "assets/skull/skull.obj"); //Skull decoration
	skull.initialize_data_on_gpu(skull_mesh);
	skull.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/skull/skull.jpg");
	vec3 skull_trans = vec3(0, L_terrain / 3, evaluate_dune_height(0, L_terrain / 3));
	trans_mesh.push_back(skull_trans); 
	skull.model.translation = skull_trans;
	float skull_scaling = 0.3f * L_terrain / 30;
	skull.model.scaling = skull_scaling;
	vec4 cent = mesh_center(skull_mesh, skull_scaling);
	lmesh_center.push_back(cent); 

	mesh arch_mesh = mesh_load_file_obj(project::path + "assets/arch.obj"); //Arch decoration
	arch_mesh.rotate({ 1, 0,0 }, Pi / 2);
	arch.initialize_data_on_gpu(arch_mesh);
	arch.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/sand1.jpg");
	vec3 arch_trans = vec3(-L_terrain / 4, -L_terrain / 5, evaluate_dune_height(-L_terrain / 4, -L_terrain / 5) + 1 * L_terrain / 30);
	trans_mesh.push_back(arch_trans);
	arch.model.translation = arch_trans;
	
	float arch_scaling = 0.1f*L_terrain/40;
	arch.model.scaling = arch_scaling;
	vec4 arch_cent = mesh_center(arch_mesh, arch_scaling);
	lmesh_center.push_back(arch_cent);

	mesh chest_mesh = mesh_load_file_obj(project::path + "assets/chest/13019_aquarium_treasure_chest_v1_L2.obj"); //Chest decoration
	chest.initialize_data_on_gpu(chest_mesh);
	vec3 chest_trans = { L_terrain / 3, L_terrain / 5,evaluate_dune_height(L_terrain / 3,L_terrain / 5) };
	chest.model.translation = chest_trans;
	trans_mesh.push_back(chest_trans);
	chest.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/chest/aquarium_treasure_chest_diffuse.jpg", GL_REPEAT, GL_REPEAT);
	float chest_scaling = 0.1f * L_terrain / 30;
	chest.model.scaling = chest_scaling;
	lmesh_center.push_back(mesh_center(chest_mesh, chest_scaling));

	mesh seaw_mesh = mesh_load_file_obj(project::path + "assets/seaweed_m.obj"); //Sea weed
	seaw.initialize_data_on_gpu(seaw_mesh);
	vec3 seaw_trans = { 0,0,evaluate_dune_height(0,0)-0.1f };
	seaw.model.translation = seaw_trans;
	seaw.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/green1.jpg", GL_REPEAT, GL_REPEAT);
	float seaw_scaling = 10 * L_terrain / 30;
	seaw.model.scaling = seaw_scaling;
	seaw.model.scaling_xyz = vec3(1, 1, 4);
	seaw.model.rotation = rotation_transform::from_axis_angle({ 1, 0,0 }, Pi / 2);
	std::vector<vec3> islets = generate_positions_on_terrain(nb_islet, L_terrain, 0,5);
	seaws = generate_positions_around_points(nb_seaw, L_terrain, 0.5f,islets,3); //Generate sea weed's cluster
	random_floats.reserve(nb_seaw);
	for (int i = 0; i < nb_seaw; ++i) {
		random_floats.push_back(rand_interval(1.0f, 7.0f));
	}
	opengl_shader_structure shader_custom_seaw;
	shader_custom_seaw.load(
		project::path + "shaders/mesh_custom/mesh_custom.vert - Copie.glsl",
		project::path + "shaders/mesh_custom/mesh_custom.frag - Copie.glsl");
	seaw.shader = shader_custom_seaw;

	mesh castle_mesh = mesh_load_file_obj(project::path + "assets/Chateau.obj");
	castle_mesh.rotate({ 1,0,0 }, Pi / 2);
	castle_mesh.rotate({ 0,0,1 }, Pi );
	castle.initialize_data_on_gpu(castle_mesh);
	castle.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/castle_text.png", GL_REPEAT, GL_REPEAT);
	vec3 castle_trans = vec3(L_terrain / 4, -L_terrain / 5, evaluate_dune_height(L_terrain / 4, -L_terrain / 4)-6.5f * L_terrain / 35);
	castle.model.translation = castle_trans;
	trans_mesh.push_back(castle_trans);
	float castle_scaling = 2.1* L_terrain / 30;
	castle.model.scaling = castle_scaling;
	lmesh_center.push_back(mesh_center(castle_mesh, castle_scaling));

}
void scene_structure::creation_mesh_bubble_crater() {
	mesh crater_mesh = mesh_load_file_obj(project::path + "assets/Volcano_OBJ.obj");
	crater.initialize_data_on_gpu(crater_mesh);
	crater.model.scaling = 0.008f * L_terrain / 30;
	crater.model.rotation = rotation_transform::from_axis_angle({ 1, 0,0 }, Pi / 2);
	crater.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/sand1.jpg");
	crater.material.phong.specular = 0.0f;
	craters = generate_positions_on_terrain(nb_crater, L_terrain, 0.6f * L_terrain / 30,1); //Generate uniformly random positions
	for (int i = 0; i < nb_crater; ++i) { //Choice of which bubble is assigned to each crater
		crater_bubble_indices[i] = std::rand() % nb_bubble;
	}

	mesh bubble_mesh = mesh_primitive_quadrangle({ -0.5f,0,0 }, { 0.5f,0,0 }, { 0.5f,0,1 }, { -0.5f,0,1 });
	bubble.initialize_data_on_gpu(bubble_mesh);
	bubble.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/bubble.png"); //Semi-transparent 
	bubble.material.phong = { 0.4f, 0.6f,0,1 };
	bubble.model.scaling = 0.5f * L_terrain / 30;
}
void scene_structure::initialize() {
	std::cout << "Start function scene_structure::initialize()" << std::endl;
	camera_control.initialize(inputs, window); 
	camera_control.set_rotation_axis_z();
	camera_projection.field_of_view = Pi / 2;
	//Start Comment here to switch camera
	camera_control.camera_model.position_camera.z = view_height + evaluate_dune_height(0, 0);
	//End Comment here to switch camera
	display_info();
	
	global_frame.initialize_data_on_gpu(mesh_primitive_frame());

	gestion_timer();

	creation_mesh_terrain();

	creation_skybox();

	creation_wall();

	creation_mesh_fish();
	
	creation_mesh_decoration();

	creation_mesh_bubble_crater();
}

void scene_structure::display_info()
{
	std::cout << "\nCAMERA CONTROL:" << std::endl;
	std::cout << "-----------------------------------------------" << std::endl;
	std::cout << camera_control.doc_usage() << std::endl;
	std::cout << "-----------------------------------------------\n" << std::endl;


	std::cout << "\nSCENE INFO:" << std::endl;
	std::cout << "-----------------------------------------------" << std::endl;
	std::cout << "Welcome to the Baby Shark World !" << std::endl;
	std::cout << "You can freely moove across the aquarium but but careful around the edges, you might get teleported !" << std::endl;
	std::cout << "This scene allows the character to give the impression of being able to walk on the ground using the mouse and keyboard (keys or WSAD/ZSQD)." << std::endl;
	std::cout << "For game-like mode: Use 'Shift+F' for full screen; 'Shift+C' for mouse capture." << std::endl;
	std::cout << "-----------------------------------------------\n" << std::endl;
}


void scene_structure::display_frame()
{
	// Set the light to the current position of the camera
	environment.light = camera_control.camera_model.position();
	 
	//Start Comment here to switch camera
	//If camera out of bounds
	camera_control.camera_model.position_camera.z = view_height + evaluate_dune_height(camera_control.camera_model.position().x, camera_control.camera_model.position().y);
	float l = L_terrain / 2;
	float acceptance = 0.5f;
	if (std::abs(camera_control.camera_model.position_camera.x) > l + acceptance || std::abs(camera_control.camera_model.position_camera.y) > l + acceptance) { //Deal with out of bound's camera
		float x = rand_interval(-l, l);
		float y = rand_interval(-l, l);
		float z = evaluate_dune_height(x, y);
		camera_control.camera_model.position_camera = { x, y, view_height + z };
		vec3 v0 = vec3(rand_interval(-1, 1), rand_interval(-1, 1), rand_interval(-1, 1));
		camera_control.camera_model.position_camera = { rand_interval(-l, l), rand_interval(-l,l), rand_interval(0, l) + evaluate_dune_height(rand_interval(-l, l), rand_interval(-l, l)) };
		std::cerr << "Oups !! You got teleported !!" << std::endl;
	}
	//End Comment here to switch camera
	
	display_skybox();
	

	// Update time
	timer.update();
	timer_i.update();
	timer_i_1.update();
	timer_i_2.update();
	float t = timer_i.t;
	if (t < timer_i.t_min + 0.1f)
		keyframe.trajectory.clear();
	if (t < timer_i_1.t_min + 0.1f)
		keyframe_1.trajectory.clear();
	if (t < timer_i_2.t_min + 0.1f)
		keyframe_2.trajectory.clear();

	draw(global_frame, environment);

	if (gui.display_wireframe) {
		draw_wireframe(chest, environment);
		draw_wireframe(terrain, environment);
		draw_wireframe(volume, environment);
		draw_wireframe(skull, environment);
		draw_wireframe(fish, environment);
		draw_wireframe(crater, environment);
		draw_wireframe(bubble, environment);
		draw_wireframe(seaw, environment);
		
	}
	
	if (gui.display_volume) {
		int i = 0;
		for (vec4 position : lmesh_center) {
			vec3 place = { position[0],position[1], position[2] };
			volume.model.scaling = volume_factor*position[3];
			volume.model.translation = place+ trans_mesh[i];
			i++;
			draw(volume, environment);
		}
	}

	for (int i = 0; i < nb_crater; i++) { //Craters
		crater.model.translation = craters[i];
		draw(crater, environment);
	}
	
	draw(terrain);
	draw(skull);
	draw(chest);
	draw(arch);
	draw(ceiling);
	draw(castle);
	

	for (int i = 0; i < nb_seaw; i++) { //Sea weed with random heights
		seaw.model.scaling_xyz = vec3(1, 1, random_floats[i]);
		seaw.model.translation = seaws[i];
		draw(seaw, environment);
	}

	simulation_step(timer.scale * 0.01f);

	for (int i = 0; i < nb_fish; i++) { //Fish translation and rotation toward speed vector
		fish.model.translation = p[i];
		fish.model.rotation = rotation_transform::from_vector_transform(vec3(1, 0, 0), normalize(v[i]))* rotation_transform::from_axis_angle({ 0, 0, 1 }, Pi/2);
		draw(fish, environment);
	}

	environment.uniform_generic.uniform_float["time"] = timer.t;
	p_interpolations[0] = interpolation(t, keyframe.key_positions, keyframe.key_times);
	p_interpolations[1] = interpolation(t, keyframe_1.key_positions, keyframe_1.key_times);
	p_interpolations[2] = interpolation(t, keyframe_2.key_positions, keyframe_2.key_times);
	display_bubble_wall(p_interpolations);
	
}
void scene_structure::display_skybox() {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(false);
	draw(skybox, environment);
	glDepthMask(true);
	glDisable(GL_BLEND);
}

void scene_structure::display_bubble_wall(vec3 p_interpolations[3])
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(false);

	auto const& camera = camera_control.camera_model;

	// Re-orient the grass shape to always face the camera direction
	vec3 const right = camera.right();
	// Rotation such that the grass follows the right-vector of the camera, while pointing toward the z-direction
	rotation_transform R = rotation_transform::from_frame_transform({ 1,0,0 }, { 0,0,1 }, right, { 0,0,1 });
	bubble.model.rotation = R;
	for (int i = 0; i < 4; i++) { //Walls
		wall.model.rotation = rotation_transform::from_axis_angle({ 0, 0, 1 }, Pi*i/2);
		draw(wall, environment);
	}
	

	vec3 trans = { 0,0,-0.5f }; //Bubbles
	for (int i = 0; i < nb_crater; i++) {
		vec3 crat = craters[i];
		int bub = crater_bubble_indices[i];
		if (i % 2 == 0) {
			bubble.model.translation = p_interpolations[bub] + crat + trans;
		}
		else {
			vec3 p_inter = p_interpolations[bub];
			vec3 p_inter_inv = {-p_inter.x,-p_inter.y,p_inter.z};
			bubble.model.translation = p_inter_inv + crat + trans;
		}
		draw(bubble, environment);
	}
	
	glDepthMask(true);
	glDisable(GL_BLEND);
}

void scene_structure::display_gui()
{
	ImGui::Checkbox("Frame", &gui.display_frame);
	ImGui::Checkbox("Wireframe", &gui.display_wireframe);
	ImGui::Checkbox("Volume", &gui.display_volume);

}

void scene_structure::mouse_move_event()
{
	if (!inputs.keyboard.shift)
		camera_control.action_mouse_move(environment.camera_view);
}
void scene_structure::mouse_click_event()
{
	camera_control.action_mouse_click(environment.camera_view);
}
void scene_structure::keyboard_event()
{
	camera_control.action_keyboard(environment.camera_view);
}
void scene_structure::idle_frame()
{
	camera_control.idle_frame(environment.camera_view);
}