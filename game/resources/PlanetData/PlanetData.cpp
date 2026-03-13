#include "PlanetData.hpp"
using namespace godot;

// ---------------------------------------------------------------------------
// OctreeNode
// ---------------------------------------------------------------------------

PlanetData::OctreeNode::OctreeNode(const Vector3& p_center, float p_half_size)
    : center(p_center),
      half_size(p_half_size),
      value(-1.0f),
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
    if (is_leaf)
        return;

    float sum   = 0.0f;
    int   count = 0;

    for (int i = 0; i < 8; ++i) {
        if (children[i]) {
            sum += children[i]->value;
            ++count;
        }
    }

    if (count > 0)
        value = sum / static_cast<float>(count);
}

// ---------------------------------------------------------------------------
// PlanetData
// ---------------------------------------------------------------------------

PlanetData::PlanetData() {
    // Центр в 0,0,0 — целочисленные координаты вокселей не попадают
    // ровно на границу октанта, half_size с запасом покрывает рабочий объём.
    root = new OctreeNode(Vector3(0.0f, 0.0f, 0.0f), 128.0f);
    initialize_default();
}

PlanetData::~PlanetData() {
    delete root;
}

void PlanetData::_bind_methods() {
    ClassDB::bind_method(
        D_METHOD("set_block", "index", "lod", "density"),
        &PlanetData::set_block
    );
    ClassDB::bind_method(
        D_METHOD("get_block", "index", "lod"),
        &PlanetData::get_block
    );
}

// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------

void PlanetData::set_block(const Vector3i& pos, int lod, float density) {
    expand_root_to_fit(Vector3(pos));
    set_block_recursive(root, Vector3(pos), lod, density);
}

float PlanetData::get_block(const Vector3i& pos, int lod) const {
    return get_block_recursive(root, Vector3(pos), lod);
}

// ---------------------------------------------------------------------------

void PlanetData::set_block_recursive(
    OctreeNode*    node,
    const Vector3& pos,
    int            lod,
    float          value)
{
    if (node->half_size * 2.0f <= static_cast<float>(lod)) {
        for (int i = 0; i < 8; ++i) {
            delete node->children[i];
            node->children[i] = nullptr;
        }
        node->is_leaf = true;
        node->value   = value;
        return;
    }

    if (node->is_leaf) {
        float old_value  = node->value;
        float child_half = node->half_size * 0.5f;
        node->is_leaf    = false;

        for (int i = 0; i < 8; ++i) {
            Vector3 offset(
                (i & 1) ? child_half : -child_half,
                (i & 2) ? child_half : -child_half,
                (i & 4) ? child_half : -child_half
            );
            node->children[i]        = new OctreeNode(node->center + offset, child_half);
            node->children[i]->value = old_value;
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
        node->children[index] = new OctreeNode(node->center + offset, child_half);
        // Наследуем значение родителя, чтобы не было "дыр" со значением -1
        node->children[index]->value = node->value;
    }

    set_block_recursive(node->children[index], pos, lod, value);
    node->update_aggregate();
}

// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------

void PlanetData::initialize_default() {
    // Сначала явно заполняем весь рабочий объём воздухом.
    // Это гарантирует что get_block никогда не вернёт мусор
    // на границах — узлы будут существовать для всех запросов от ChunkMesh.
    // const int RANGE = 40; // чуть больше чем реальные данные (32)
    // for (int i = -RANGE; i <= RANGE; ++i)
    // for (int j = -RANGE; j <= RANGE; ++j)
    // for (int k = -RANGE; k <= RANGE; ++k)
    //     set_block(Vector3i(i, j, k), 1, -1.0f);

    // // Затем записываем землю поверх воздуха.
    // for (int i = -32; i < 32; ++i)
    // for (int j = -1;  j <  1;  ++j)
    // for (int k = -32; k < 32; ++k)
    //     set_block(Vector3i(i, j, k), 1, 1.0f);
}