#include "PlanetData.hpp"

using namespace godot;

PlanetData::OctreeNode::OctreeNode(const Vector3& p_center, float p_half_size)
    : center(p_center),
      half_size(p_half_size),
      value(-1.0f),
      is_leaf(true) {

    for (int i = 0; i < 8; ++i)
        children[i] = nullptr;
}

PlanetData::OctreeNode::~OctreeNode() {
    for (int i = 0; i < 8; ++i)
        delete children[i];
}

PlanetData::PlanetData() {
    root = new OctreeNode(Vector3(0,0,0), 8.0f);
    initialize_default();
}

PlanetData::~PlanetData() {
    delete root;
}

void PlanetData::_bind_methods() {}

void PlanetData::expand_root_to_fit(const Vector3& pos) {

    while (
        Math::abs(pos.x - root->center.x) > root->half_size ||
        Math::abs(pos.y - root->center.y) > root->half_size ||
        Math::abs(pos.z - root->center.z) > root->half_size
    ) {
        float new_half = root->half_size * 2.0f;

        Vector3 dir(
            pos.x >= root->center.x ? 1 : -1,
            pos.y >= root->center.y ? 1 : -1,
            pos.z >= root->center.z ? 1 : -1
        );

        Vector3 new_center =
            root->center + dir * root->half_size;

        OctreeNode* new_root =
            new OctreeNode(new_center, new_half);

        new_root->is_leaf = false;

        int child_index = 0;
        if (dir.x < 0) child_index |= 1;
        if (dir.y < 0) child_index |= 2;
        if (dir.z < 0) child_index |= 4;

        new_root->children[child_index] = root;

        root = new_root;
    }
}

void PlanetData::set_block(const Vector3& pos, uint8_t type) {

    expand_root_to_fit(pos);

    set_block_recursive(root, pos, (float)type);
}

float PlanetData::get_block(const Vector3& pos) const {
    return get_block_recursive(root, pos);
}

float PlanetData::get_density(
    const Vector3i& pos,
    int step) const
{
    if (!root)
        return -1.0f;

    Vector3 p((float)pos.x, (float)pos.y, (float)pos.z);

    float target_size = (float)step;

    return get_density_recursive(root, p, target_size);
}

void PlanetData::set_block_recursive(
    OctreeNode* node,
    const Vector3& pos,
    float value)
{
    if (node->half_size <= MIN_NODE_SIZE) {
        node->is_leaf = true;
        node->value = value;
        return;
    }

    if (node->is_leaf) {
        node->is_leaf = false;
    }

    int index = 0;
    if (pos.x >= node->center.x) index |= 1;
    if (pos.y >= node->center.y) index |= 2;
    if (pos.z >= node->center.z) index |= 4;

    if (!node->children[index]) {

        float child_half = node->half_size * 0.5f;

        Vector3 offset(
            (index & 1) ? child_half : -child_half,
            (index & 2) ? child_half : -child_half,
            (index & 4) ? child_half : -child_half
        );

        node->children[index] =
            new OctreeNode(node->center + offset, child_half);
    }

    set_block_recursive(node->children[index], pos, value);
}

float PlanetData::get_block_recursive(
    const OctreeNode* node,
    const Vector3& pos) const
{
    if (!node)
        return -1.0f;

    if (node->is_leaf)
        return node->value;

    int index = 0;
    if (pos.x >= node->center.x) index |= 1;
    if (pos.y >= node->center.y) index |= 2;
    if (pos.z >= node->center.z) index |= 4;

    return get_block_recursive(node->children[index], pos);
}

float PlanetData::get_density_recursive(
    const OctreeNode* node,
    const Vector3& pos,
    float target_size) const
{
    if (!node)
        return -1.0f;

    // ✅ если дошли до листа
    if (node->is_leaf)
        return node->value;

    // ✅ если узел уже соответствует нужному LOD
    if (node->half_size * 2.0f <= target_size)
        return node->value;

    int index = 0;

    if (pos.x >= node->center.x) index |= 1;
    if (pos.y >= node->center.y) index |= 2;
    if (pos.z >= node->center.z) index |= 4;

    return get_density_recursive(
        node->children[index],
        pos,
        target_size
    );
}

void PlanetData::initialize_default() {
    for(int i=-4; i<4; i++)
    for(int j=-4; j<4; j++)
    {
        set_block(Vector3(i,0,j), 1.0);
    }
}