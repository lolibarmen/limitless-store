#pragma once
#include <godot_cpp/classes/resource.hpp>
#include <unordered_map>
#include <GameStructs/Blocks.hpp>
#include <BlockGenerator/BlockGenerator.hpp>

#include <Utils/SpatialHash.hpp>

namespace godot {


class BlockSource : public Resource {
    GDCLASS(BlockSource, Resource)

private:
    Ref<BlockGenerator> generator;
    std::unordered_map<Vector3i, BlockData, Vector3iHash> edits;

protected:
    static void _bind_methods();

public:
    BlockSource() = default;
    ~BlockSource() override = default;

    // Явная инициализация — вызывается из менеджера
    void init(Ref<BiomeSource> biome_source, int seed);

    BlockData     get_block(const Vector3i& world_pos) const;
    void          set_block(const Vector3i& world_pos, BlockData data);
    void          reset_block(const Vector3i& world_pos);
    bool          has_edit(const Vector3i& world_pos) const;

    Ref<BlockGenerator> get_generator() const { return generator; }
    void                set_generator(Ref<BlockGenerator> g) { generator = g; }
};

} // namespace godot