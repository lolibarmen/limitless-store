#pragma once

#include <godot_cpp/classes/node3d.hpp>
#include <BlockSource/BlockSource.hpp>
#include <ChunkMeshNode/ChunkMeshNode.hpp>

namespace godot {

class NeochunkNode : public Node3D {
    GDCLASS(NeochunkNode, Node3D)

private:
    int _lod_level = 0;
    float _chunk_size = 16.0f;

    Ref<BlockSource> _block_source;

    // Дочерние узлы конвейера
    ChunkMeshNode* _mesh_node = nullptr;
    // В будущем здесь появятся:
    // ChunkStructuresNode* _structures_node = nullptr;
    // ChunkFoliageNode* _foliage_node = nullptr;

    MeshInstance3D* _debug_mesh_instance = nullptr;
    void add_debug_box();

protected:
    static void _bind_methods();

public:
    NeochunkNode();
    ~NeochunkNode();

    void _ready() override;

    // Запуск полного конвейера генерации
    void generate();

    void set_lod_level(int level);
    void set_chunk_size(float size);
    void set_block_source(Ref<BlockSource> source);

    Ref<BlockSource> get_block_source() const { return _block_source; }
};

} // namespace godot