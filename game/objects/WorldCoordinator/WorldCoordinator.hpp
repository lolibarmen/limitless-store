#pragma once
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <SemanticShape/SemanticShape.hpp>

namespace godot {

class WorldCoordinator : public Node3D {
    GDCLASS(WorldCoordinator, Node3D)

private:
    TypedArray<Ref<SemanticShape>> _shapes;
    bool _registering = false;
    bool _exiting = false;

    void _register_shapes();
    void _attach_render_nodes();
    void _detach_render_nodes();

protected:
    static void _bind_methods();

public:
    WorldCoordinator() = default;
    ~WorldCoordinator() override = default;

    void set_shapes(const TypedArray<Ref<SemanticShape>>& shapes);
    TypedArray<Ref<SemanticShape>> get_shapes() const;

    void _enter_tree() override;
    void _exit_tree() override;
};

} // namespace godot
