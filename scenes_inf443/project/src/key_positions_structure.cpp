#include "key_positions_structure.hpp"


using namespace cgp;

void keyframe_structure::initialize(numarray<vec3> const& key_positions_arg, numarray<float> const& key_times_arg)
{
	key_positions = key_positions_arg;
	key_times = key_times_arg;

}