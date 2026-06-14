// BuildBlock.cpp
#include "BuildBlock.hpp"
#include <godot_cpp/classes/node3d.hpp>

namespace godot {

Node3D *BuildBlock::get_nearest_connector(const Vector3 &world_pos) const {
    Node3D *nearest = nullptr;
    float min_dist = 1e9f;

    // Коннекторы — дочерние узлы с группой "connector"
    for (int i = 0; i < get_child_count(); i++) {
        Node3D *c = Object::cast_to<Node3D>(get_child(i));
        if (!c || !c->is_in_group("connector")) continue;

        float d = c->get_global_position().distance_to(world_pos);
        if (d < min_dist) { min_dist = d; nearest = c; }
    }
    return nearest;
}

void BuildBlock::snap_to_connector(Node3D *target_connector) {
    // Наш коннектор (первый свободный) совмещаем с target
    Node3D *my_connector = get_nearest_connector(get_global_position());
    if (!my_connector) { set_global_position(target_connector->get_global_position()); return; }

    Vector3 offset = my_connector->get_global_position() - get_global_position();
    set_global_position(target_connector->get_global_position() - offset);
}

} // namespace godot