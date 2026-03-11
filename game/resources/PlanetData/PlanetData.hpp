#pragma once
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/vector3i.hpp>
#include <cstdint>

namespace godot {

class PlanetData : public Resource {
    GDCLASS(PlanetData, Resource)

private:
    static constexpr float MIN_NODE_SIZE = 1.0f;

    struct OctreeNode {
        Vector3 center;
        float   half_size;
        float   value;    // leaf: точное значение; branch: агрегат (среднее) дочерних нод
        bool    is_leaf;
        OctreeNode* children[8];

        OctreeNode(const Vector3& p_center, float p_half_size);
        ~OctreeNode();

        // Пересчитывает value как среднее существующих дочерних нод.
        // Вызывается снизу вверх после каждой записи.
        void update_aggregate();
    };

    OctreeNode* root;

    // Рекурсивно читает плотность в точке pos при заданном lod.
    // lod — размер вокселя в мировых единицах (степень двойки).
    float get_block_recursive(const OctreeNode* node,
                              const Vector3&    pos,
                              int               lod) const;

    // Рекурсивно записывает плотность и обновляет агрегаты на обратном ходу.
    void set_block_recursive(OctreeNode*    node,
                             const Vector3& pos,
                             int            lod,
                             float          value);

    // Расширяет корень дерева до тех пор, пока pos не окажется внутри.
    void expand_root_to_fit(const Vector3& pos);

protected:
    static void _bind_methods();

public:
    PlanetData();
    ~PlanetData();

    void initialize_default();

    // Записывает плотность density в воксель pos с детализацией lod.
    void  set_block(const Vector3i& pos, int lod, float density);

    // Возвращает плотность вокселя pos с детализацией lod.
    // Если lod крупнее листа — возвращает агрегированное значение ветви.
    float get_block(const Vector3i& pos, int lod) const;
};

} // namespace godot