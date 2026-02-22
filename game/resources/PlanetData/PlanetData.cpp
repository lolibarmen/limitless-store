#include "PlanetData.h"
#include <algorithm>

using namespace godot;

void PlanetData::_bind_methods() {
    // Можно добавить свойства, если требуется
}

PlanetData::PlanetData() : world_size(Vector3i(16, 16, 16)) {
    initialize_default();
}

Vector3i PlanetData::get_offset() const {
    return Vector3i(world_size.x / 2, world_size.y / 2, world_size.z / 2);
}

int PlanetData::to_index(const Vector3i& pos) const {
    Vector3i offset = get_offset();
    Vector3i local_pos = pos + offset; // теперь координаты в диапазоне [0, size-1]
    return local_pos.z * world_size.x * world_size.y +
           local_pos.y * world_size.x +
           local_pos.x;
}

bool PlanetData::is_valid_position(const Vector3i& pos) const {
    Vector3i offset = get_offset();
    Vector3i local_pos = pos + offset;
    return local_pos.x >= 0 && local_pos.x < world_size.x &&
           local_pos.y >= 0 && local_pos.y < world_size.y &&
           local_pos.z >= 0 && local_pos.z < world_size.z;
}

void PlanetData::initialize_default() {
    int total_blocks = world_size.x * world_size.y * world_size.z;
    blocks_data.assign(total_blocks, -1.0f); // -1 означает "пусто"
    Vector3i pos;
    int index;

    pos = Vector3i(0, 0, 0);
    index = to_index(pos);
    blocks_data[index] = 1.0f;
    
    pos = Vector3i(0, 0, 1);
    index = to_index(pos);
    blocks_data[index] = 1.0f;

    pos = Vector3i(1, 0, 0);
    index = to_index(pos);
    blocks_data[index] = 1.0f;

    pos = Vector3i(1, 0, 1);
    index = to_index(pos);
    blocks_data[index] = 1.0f;
}

void PlanetData::set_block(const Vector3i& pos, uint8_t type) {
    if (is_valid_position(pos)) {
        int index = to_index(pos);
        blocks_data[index] = static_cast<float>(type);
    } else {
        // Сообщение об ошибке (зависит от используемого механизма логирования)
        print_error("Cannot set block at invalid position: ", pos);
    }
}

float PlanetData::get_block(const Vector3i& pos) const {
    if (!is_valid_position(pos)) {
        print_error("Outside planet!!", pos);
        return 0.0f;
    }
    int index = to_index(pos);
    return blocks_data[index];
}

void PlanetData::set_world_size(const Vector3i& p_size) {
    world_size = p_size;
    initialize_default(); // данные сбрасываются под новый размер
}

Vector3i PlanetData::get_world_size() const {
    return world_size;
}