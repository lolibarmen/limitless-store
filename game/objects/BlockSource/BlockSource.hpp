#pragma once
#include <godot_cpp/classes/ref_counted.hpp>
#include <GameStructs/Blocks.hpp>
#include <BlockGenerator/BlockGenerator.hpp>
#include <Interfaces/Source.hpp>

namespace godot {

class BlockSource : public Resource, public Source<BlockData> {
    GDCLASS(BlockSource, Resource)

private:
    Ref<BlockGenerator> generator;

protected:
    static void _bind_methods();

    // Реализация точки расширения базового Source<T>
    BlockData generate(const Vector3i& world_pos) const override {
        return generator->get_block(world_pos);
    }

public:
    BlockSource() = default;
    ~BlockSource() override = default;

    void init(Ref<BlockGenerator> p_generator) {
        if (!p_generator.is_valid()) {
            print_error("void BlockSource::init() p_generator not valid");
        }
        generator = p_generator;
    }

    // Обёртки над Source<T>::get/set с прежними именами API
    BlockData get_block(const Vector3i& world_pos) const { return get_value(world_pos); }
    void      set_block(const Vector3i& world_pos, BlockData data) { set_value(world_pos, data); }
    void      reset_block(const Vector3i& world_pos) { reset_value(world_pos); }
    bool      has_edit(const Vector3i& world_pos) const { return Source<BlockData>::has_edit_value(world_pos); }

    int get_block_material(const Vector3i& world_pos) const { return static_cast<int>(get_block(world_pos).material); }
    float get_block_density(const Vector3i& world_pos) const { return get_block(world_pos).density; }
    void set_block_material(const Vector3i& world_pos, int material) {
        BlockData data = get_block(world_pos);
        data.material = static_cast<BlockMaterial>(material);
        set_block(world_pos, data);
    }
    void set_block_density(const Vector3i& world_pos, float density) {
        BlockData data = get_block(world_pos);
        data.density = density;
        set_block(world_pos, data);
    }

    Ref<BlockGenerator> get_generator() const { return generator; }
    void                set_generator(Ref<BlockGenerator> g) { generator = g; }
};

} // namespace godot