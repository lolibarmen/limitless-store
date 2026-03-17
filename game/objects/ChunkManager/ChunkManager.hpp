#pragma once
#include <godot_cpp/classes/node3d.hpp>
#include <unordered_map>
#include <unordered_set>
#include <ChunkNode/ChunkNode.hpp>
#include <BlockSource/BlockSource.hpp>

namespace godot {

class ChunkManager : public Node3D {
    GDCLASS(ChunkManager, Node3D)

private:
    Ref<BlockSource> block_source;
    std::unordered_map<int64_t, ChunkNode*> chunks;

    int   lod         = 1;
    int   voxel_count = 8;
    float range       = 64.0f;

    static int64_t chunk_hash(Vector3i pos);

protected:
    static void _bind_methods();

public:
    ChunkManager();
    ~ChunkManager() override = default;

    void _ready()  override;
    void _process(double delta) override;

    void init(Ref<BlockSource> p_block_source);
    void update();

    ChunkNode* get_chunk_by_origin(const Vector3i& origin) const;

    int   get_lod()         const { return lod; }
    void  set_lod(int v)          { lod = v; }
    int   get_voxel_count() const { return voxel_count; }
    void  set_voxel_count(int v)  { voxel_count = v; }
    float get_range()       const { return range; }
    void  set_range(float v)      { range = v; }
};

} // namespace godot