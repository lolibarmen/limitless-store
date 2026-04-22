#pragma once
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/mutex.hpp>
#include <unordered_map>
#include <vector>
#include <GameStructs/Blocks.hpp>
#include <BlockGenerator/BlockGenerator.hpp>
#include <Utils/SpatialHash.hpp>

namespace godot {

class BlockSource : public Resource {
    GDCLASS(BlockSource, Resource)

private:
    Ref<BlockGenerator> generator;
    std::unordered_map<Vector3i, BlockData, Vector3iHash> edits;
    Ref<Mutex> mutex;

protected:
    static void _bind_methods();

public:
    BlockSource() = default;
    ~BlockSource() override = default;

    void init(Ref<BlockGenerator> p_generator);

    BlockData get_block(const Vector3i& world_pos) const;
    void      set_block(const Vector3i& world_pos, BlockData data);
    void      reset_block(const Vector3i& world_pos);
    bool      has_edit(const Vector3i& world_pos) const;

    // Обвёртки
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

    // Новый метод — заполняет массив блоков для чанка
    void fill_chunk(
        std::vector<BlockData>& out,
        const Vector3i& chunk_coord,
        int stride,
        int step
    ) const;

    Ref<BlockGenerator> get_generator() const { return generator; }
    void                set_generator(Ref<BlockGenerator> g) { generator = g; }
};

} // namespace godot