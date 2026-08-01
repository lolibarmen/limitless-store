#include "BlockSource.hpp"

using namespace godot;

void BlockSource::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_block_material", "world_pos"), &BlockSource::get_block_material);
    ClassDB::bind_method(D_METHOD("get_block_density", "world_pos"), &BlockSource::get_block_density);
    ClassDB::bind_method(D_METHOD("set_block_material", "world_pos", "material"), &BlockSource::set_block_material);
    ClassDB::bind_method(D_METHOD("set_block_density", "world_pos", "density"), &BlockSource::set_block_density);
}