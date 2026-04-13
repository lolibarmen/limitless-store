#pragma once
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/mutex.hpp>
#include <unordered_map>
#include <vector>
#include <GameStructs/Blocks.hpp>
#include <BlockGenerator/BlockGenerator.hpp>
#include <Utils/SpatialHash.hpp>

namespace godot {

class BlockLODSource : public Resource {
    GDCLASS(BlockLODSource, Resource)

private:
    Ref<BlockGenerator> generator;
    Ref<BlockLODSource> block_lod_source;
    std::unordered_map<Vector3i, BlockData, Vector3iHash> edits;
    Ref<Mutex> mutex;

protected:
    static void _bind_methods();

public:
    BlockLODSource() = default;
    ~BlockLODSource() override = default;

    void init(Ref<BlockGenerator> p_generator, Ref<BlockLODSource> p_block_lod_source);

    BlockData get_block(const Vector3i& world_pos) const;
    void      set_block(const Vector3i& world_pos, BlockData data);
    void      reset_block(const Vector3i& world_pos);
    bool      has_edit(const Vector3i& world_pos) const;

    // Новый метод — заполняет массив блоков для чанка
    void fill_chunk(
        std::vector<BlockData>& out,
        const Vector3i& chunk_coord,
        int stride,
        int step
    ) const;

    Ref<BlockGenerator> get_generator() const { return generator; }
    void                set_generator(Ref<BlockGenerator> g) { generator = g; }
    Ref<BlockLODSource> get_lod_source() const { return block_lod_source; }
};

} // namespace godot