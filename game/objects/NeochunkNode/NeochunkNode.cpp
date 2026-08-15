#include "NeochunkNode.hpp"
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void NeochunkNode::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_lod_level", "level"), &NeochunkNode::set_lod_level);
    ClassDB::bind_method(D_METHOD("set_chunk_size", "size"), &NeochunkNode::set_chunk_size);
    ClassDB::bind_method(D_METHOD("set_block_source", "source"), &NeochunkNode::set_block_source);
    ClassDB::bind_method(D_METHOD("get_block_source"), &NeochunkNode::get_block_source);
}

NeochunkNode::NeochunkNode() {}
NeochunkNode::~NeochunkNode() {}

void NeochunkNode::add_debug_box() {
    if(_debug_mesh_instance) return;
    _debug_mesh_instance = memnew(MeshInstance3D);

    BoxMesh* box_mesh = memnew(BoxMesh);
    box_mesh->set_size(Vector3(1.0, 1.0, 1.0) * _chunk_size);
    Ref<Mesh> mesh = Ref<Mesh>(box_mesh);
    _debug_mesh_instance->set_mesh(mesh);

    Ref<Shader> shader = Ref<Shader>(memnew(Shader));
    shader->set_code(R"(
        shader_type spatial;
        render_mode wireframe, unshaded;
        uniform vec4 line_color : source_color;
        void fragment() {
            ALBEDO = line_color.rgb;
        }
    )");

    Ref<ShaderMaterial> debug_material = Ref<ShaderMaterial>(memnew(ShaderMaterial));
    debug_material->set_shader(shader);
    debug_material->set_shader_parameter("line_color", Color(0.2f, 0.5f, 1.0f));

    _debug_mesh_instance->set_material_override(debug_material);
    add_child(_debug_mesh_instance);
}

void NeochunkNode::_ready() {
    add_to_group("mineable");

    // Создаем узел меша как дочерний объект.
    // add_child() автоматически передает владение родителю (NeochunkNode)
    _mesh_node = memnew(ChunkMeshNode);
    add_child(_mesh_node);

    // Пробрасываем конфигурацию в дочерний узел
    _mesh_node->set_lod_level(_lod_level);
    _mesh_node->set_chunk_size(_chunk_size);
    _mesh_node->set_block_source(_block_source);

    // Запускаем конвейер
    generate();
}

void NeochunkNode::generate() {
    // Этап 1: Поверхность (Меш и Коллизия)
    if (_mesh_node) {
        _mesh_node->generate_mesh();
    }

    // В будущем:
    // if (_structures_node) _structures_node->generate();
    // if (_foliage_node) _foliage_node->generate();
}

void NeochunkNode::set_lod_level(int level) {
    _lod_level = level;
    if (_mesh_node) _mesh_node->set_lod_level(level);
}

void NeochunkNode::set_chunk_size(float size) {
    _chunk_size = size;
    if (_mesh_node) _mesh_node->set_chunk_size(size);
}

void NeochunkNode::set_block_source(Ref<BlockSource> source) {
    _block_source = source;
    if (_mesh_node) _mesh_node->set_block_source(source);
}