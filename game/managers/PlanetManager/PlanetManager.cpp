// PlanetManager.cpp
#include "PlanetManager.hpp"

namespace godot {

void PlanetManager::initialize(const PlanetContext& p_context) {
    context = p_context;

    // 1. Создаём хранилище блоков
    planet_data.instantiate();

    // 2. Создаём генератор из параметров планеты
    biome_generator.instantiate();
    biome_generator->initialize(context.seed, context.radius);

    // 3. Создаём ChunkManager и передаём ему контекст + генератор
    chunk_manager = memnew(ChunkManager);
    chunk_manager->initialize(planet_data, biome_generator);
    add_child(chunk_manager);

    // 4. (будущее) ForestManager получит тот же biome_generator
    // forest_manager = memnew(ForestManager);
    // forest_manager->initialize(biome_generator);
    // add_child(forest_manager);
}

void PlanetManager::_ready() {
    // initialize() должен быть вызван до _ready()
    // например из WorldManager перед add_child()
}

void PlanetManager::update(const Vector3& camera_pos) {
    if (chunk_manager)  chunk_manager->update(camera_pos);
    // if (forest_manager) forest_manager->update(camera_pos);
}

void PlanetManager::_bind_methods() {
    // экспозиция в Godot при необходимости
}

} // namespace godot