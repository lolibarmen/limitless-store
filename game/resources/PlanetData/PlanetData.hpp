#pragma once
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/vector3i.hpp>
#include <cstdint>

namespace godot {

class PlanetData : public Resource {
    GDCLASS(PlanetData, Resource)

public:
    // Перечисление типов блоков — доступно и из GDScript через константы
    enum BlockType : uint8_t {
        BLOCK_AIR   = 0,
        BLOCK_STONE = 1,
        BLOCK_DIRT  = 2,
        BLOCK_GRASS = 3,
        // ... добавляй по необходимости, максимум 255
    };

private:
    static constexpr float MIN_NODE_SIZE = 1.0f;

    struct OctreeNode {
        Vector3  center;
        float    half_size;
        float    value;      // leaf: плотность; branch: среднее дочерних
        uint8_t  type;       // leaf: тип блока; branch: мажоритарный тип
        bool     is_leaf;
        OctreeNode* children[8];

        OctreeNode(const Vector3& p_center, float p_half_size);
        ~OctreeNode();

        // Пересчитывает value (среднее) и type (мажоритарный) по дочерним нодам
        void update_aggregate();
    };

    OctreeNode* root;

    float   get_block_recursive(const OctreeNode* node, const Vector3& pos, int lod) const;
    uint8_t get_type_recursive (const OctreeNode* node, const Vector3& pos, int lod) const;

    void set_block_recursive(OctreeNode* node, const Vector3& pos,
                             int lod, float value, uint8_t type);

    void expand_root_to_fit(const Vector3& pos);

protected:
    static void _bind_methods();

public:
    PlanetData() : root(new OctreeNode(Vector3(0, 0, 0), 64.0f)) {}
    ~PlanetData() { delete root; }

    void initialize_default() {};

    // Запись: теперь принимает и тип блока
    void    set_block(const Vector3i& pos, int lod, float density, int type = BLOCK_AIR);

    // Чтение плотности (как раньше)
    float   get_block(const Vector3i& pos, int lod) const;

    // Чтение типа блока
    int     get_block_type(const Vector3i& pos, int lod) const;
};

} // namespace godot