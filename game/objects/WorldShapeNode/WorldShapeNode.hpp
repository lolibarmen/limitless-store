#pragma once
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <SemanticShape/SemanticShape.hpp>

namespace godot {

/// Класс нужен для удобной работы через редактор, тестов фигур и только.
class WorldShapeNode : public Node3D {
    GDCLASS(WorldShapeNode, Node3D)

private:
    Ref<SemanticShape> _shape;
    bool _syncing = false;

    Vector3 _get_shape_space_origin() const;
    void _sync_to_shape();
    void _sync_from_shape();
    void _request_coordinator_refresh();
    void _on_shape_changed();

protected:
    static void _bind_methods();
    void _notification(int p_what);

public:
    WorldShapeNode() = default;
    ~WorldShapeNode() override = default;

    void set_shape(const Ref<SemanticShape>& p_shape);
    Ref<SemanticShape> get_shape() const;
};

} // namespace godot
