#pragma once

#include <godot_cpp/classes/mesh_instance3d.hpp>

namespace godot {

class TestMeshInstance3D : public MeshInstance3D {
    GDCLASS(TestMeshInstance3D, MeshInstance3D)

private:
    int frame_count = 0;
    bool mesh_was_set = false;

protected:
    static void _bind_methods();

public:
    TestMeshInstance3D();
    ~TestMeshInstance3D();

    void _ready() override;
    void _process(double delta) override;
    void _notification(int p_what);

    // Переопределяем set_mesh для отслеживания
    void set_mesh(const Ref<Mesh> &p_mesh);
    Ref<Mesh> get_mesh() const;

    void _enter_tree();
    void _exit_tree();
};

} // namespace godot