#include "PlanetData.hpp"
using namespace godot;

// ---------------------------------------------------------------------------
// OctreeNode
// ---------------------------------------------------------------------------

PlanetData::OctreeNode::OctreeNode(const Vector3& p_center, float p_half_size)
    : center(p_center),
      half_size(p_half_size),
      value(-1.0f),
      type(PlanetData::BLOCK_AIR),   // <-- инициализируем тип
      is_leaf(true)
{
    for (int i = 0; i < 8; ++i)
        children[i] = nullptr;
}

PlanetData::OctreeNode::~OctreeNode() {
    for (int i = 0; i < 8; ++i)
        delete children[i];
}

void PlanetData::OctreeNode::update_aggregate() {
    if (is_leaf) return;

    float sum   = 0.0f;
    int   count = 0;

    // Мажоритарное голосование: считаем частоту каждого типа
    int type_votes[256] = {};

    for (int i = 0; i < 8; ++i) {
        if (children[i]) {
            sum += children[i]->value;
            ++count;
            type_votes[children[i]->type]++;
        }
    }

    if (count > 0) {
        value = sum / static_cast<float>(count);

        // Находим тип с максимальным числом голосов
        uint8_t majority = 0;
        for (int t = 1; t < 256; ++t) {
            if (type_votes[t] > type_votes[majority])
                majority = static_cast<uint8_t>(t);
        }
        type = majority;
    }
}

// ---------------------------------------------------------------------------
// _bind_methods
// ---------------------------------------------------------------------------

void PlanetData::_bind_methods() {
    // Регистрируем константы типов — они будут видны в GDScript как
    // PlanetData.BLOCK_STONE и т.д.
    // BIND_ENUM_CONSTANT(BLOCK_AIR);
    // BIND_ENUM_CONSTANT(BLOCK_STONE);
    // BIND_ENUM_CONSTANT(BLOCK_DIRT);
    // BIND_ENUM_CONSTANT(BLOCK_GRASS);

    ClassDB::bind_method(
        D_METHOD("set_block", "index", "lod", "density", "type"),
        &PlanetData::set_block
    );
    ClassDB::bind_method(
        D_METHOD("get_block", "index", "lod"),
        &PlanetData::get_block
    );
    ClassDB::bind_method(
        D_METHOD("get_block_type", "index", "lod"),
        &PlanetData::get_block_type
    );
}

// ---------------------------------------------------------------------------
// Публичный API
// ---------------------------------------------------------------------------

void PlanetData::set_block(const Vector3i& pos, int lod, float density, int type) {
    expand_root_to_fit(Vector3(pos));
    set_block_recursive(root, Vector3(pos), lod, density,
                        static_cast<uint8_t>(type));
}

float PlanetData::get_block(const Vector3i& pos, int lod) const {
    return get_block_recursive(root, Vector3(pos), lod);
}

int PlanetData::get_block_type(const Vector3i& pos, int lod) const {
    return static_cast<int>(get_type_recursive(root, Vector3(pos), lod));
}

// ---------------------------------------------------------------------------
// set_block_recursive — добавлен параметр type
// ---------------------------------------------------------------------------

void PlanetData::set_block_recursive(
    OctreeNode*    node,
    const Vector3& pos,
    int            lod,
    float          val,
    uint8_t        blk_type)
{
    if (node->half_size * 2.0f <= static_cast<float>(lod)) {
        for (int i = 0; i < 8; ++i) {
            delete node->children[i];
            node->children[i] = nullptr;
        }
        node->is_leaf = true;
        node->value   = val;
        node->type    = blk_type;   // <-- записываем тип
        return;
    }

    if (node->is_leaf) {
        float   old_value = node->value;
        uint8_t old_type  = node->type;   // <-- сохраняем тип
        float   child_half = node->half_size * 0.5f;
        node->is_leaf = false;

        for (int i = 0; i < 8; ++i) {
            Vector3 offset(
                (i & 1) ? child_half : -child_half,
                (i & 2) ? child_half : -child_half,
                (i & 4) ? child_half : -child_half
            );
            node->children[i]        = new OctreeNode(node->center + offset, child_half);
            node->children[i]->value = old_value;
            node->children[i]->type  = old_type;   // <-- наследуем тип
        }
    }

    int index = 0;
    if (pos.x >= node->center.x) index |= 1;
    if (pos.y >= node->center.y) index |= 2;
    if (pos.z >= node->center.z) index |= 4;

    if (!node->children[index]) {
        float   child_half = node->half_size * 0.5f;
        Vector3 offset(
            (index & 1) ? child_half : -child_half,
            (index & 2) ? child_half : -child_half,
            (index & 4) ? child_half : -child_half
        );
        node->children[index]        = new OctreeNode(node->center + offset, child_half);
        node->children[index]->value = node->value;
        node->children[index]->type  = node->type;   // <-- наследуем тип
    }

    set_block_recursive(node->children[index], pos, lod, val, blk_type);
    node->update_aggregate();
}

// ---------------------------------------------------------------------------
// get_type_recursive — новый метод, симметричен get_block_recursive
// ---------------------------------------------------------------------------

uint8_t PlanetData::get_type_recursive(
    const OctreeNode* node,
    const Vector3&    pos,
    int               lod) const
{
    if (!node)
        return BLOCK_AIR;

    if (node->half_size * 2.0f <= static_cast<float>(lod))
        return node->type;

    if (node->is_leaf)
        return node->type;

    int index = 0;
    if (pos.x >= node->center.x) index |= 1;
    if (pos.y >= node->center.y) index |= 2;
    if (pos.z >= node->center.z) index |= 4;

    if (!node->children[index])
        return node->type;   // агрегат текущей ноды

    return get_type_recursive(node->children[index], pos, lod);
}

float PlanetData::get_block_recursive(
    const OctreeNode* node,
    const Vector3&    pos,
    int               lod) const
{
    if (!node)
        return -1.0f;

    if (node->half_size * 2.0f <= static_cast<float>(lod))
        return node->value;

    if (node->is_leaf)
        return node->value;

    int index = 0;
    if (pos.x >= node->center.x) index |= 1;
    if (pos.y >= node->center.y) index |= 2;
    if (pos.z >= node->center.z) index |= 4;

    // ИСПРАВЛЕНИЕ: дочерний узел отсутствует — возвращаем значение
    // текущей ноды (агрегат), а не хардкод -1.0f
    if (!node->children[index])
        return node->value;

    return get_block_recursive(node->children[index], pos, lod);
}

void PlanetData::expand_root_to_fit(const Vector3& pos) {
    while (
        Math::abs(pos.x - root->center.x) > root->half_size ||
        Math::abs(pos.y - root->center.y) > root->half_size ||
        Math::abs(pos.z - root->center.z) > root->half_size
    ) {
        float   new_half   = root->half_size * 2.0f;
        Vector3 dir(
            pos.x >= root->center.x ?  1.0f : -1.0f,
            pos.y >= root->center.y ?  1.0f : -1.0f,
            pos.z >= root->center.z ?  1.0f : -1.0f
        );
        Vector3     new_center = root->center + dir * root->half_size;
        OctreeNode* new_root   = new OctreeNode(new_center, new_half);
        new_root->is_leaf = false;

        int child_index = 0;
        if (dir.x < 0) child_index |= 1;
        if (dir.y < 0) child_index |= 2;
        if (dir.z < 0) child_index |= 4;

        new_root->children[child_index] = root;
        new_root->value = root->value;
        root = new_root;
    }
}