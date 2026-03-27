#pragma once
#include <godot_cpp/classes/node3d.hpp>
#include <unordered_map>
#include <unordered_set>
#include <ChunkNode/ChunkNode.hpp>
#include <BlockLODSource/BlockLODSource.hpp>

namespace godot {

class ChunkManager : public Node3D {
    GDCLASS(ChunkManager, Node3D)

public:
    struct Ring {
        int   lod        = 1;
        float r_inner    = 0.0f;
        float r_outer    = 0.0f;
        Ref<BlockLODSource> source;
        std::unordered_map<int64_t, ChunkNode*> chunks;
    };

private:
    static constexpr int RINGS_COUNT = 3;
    Ring rings[RINGS_COUNT];

    int   voxel_count = 8;
    float range_lod1  = 32.0f;
    float range_lod2  = 64.0f;
    float range_lod3  = 128.0f;

    static int64_t chunk_hash(Vector3i pos);

    void spawn_chunk(Ring& ring, const Vector3i& coord);
    void despawn_chunk(Ring& ring, int64_t key);

protected:
    static void _bind_methods();

public:
    ChunkManager();
    void _ready()  override;
    void _process(double delta) override;

    void init(Ref<BlockLODSource> lod1, Ref<BlockLODSource> lod2, Ref<BlockLODSource> lod3);
    void update();

    ChunkNode* get_chunk_by_origin(const Vector3i& origin) const;

    int   get_voxel_count() const { return voxel_count; }
    void  set_voxel_count(int v)  { voxel_count = v; }
    float get_range_lod1()  const { return range_lod1; }
    void  set_range_lod1(float v) { range_lod1 = v; }
    float get_range_lod2()  const { return range_lod2; }
    void  set_range_lod2(float v) { range_lod2 = v; }
    float get_range_lod3()  const { return range_lod3; }
    void  set_range_lod3(float v) { range_lod3 = v; }
};

} // namespace godot