#include "Player.hpp"
#include <godot_cpp/classes/capsule_shape3d.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/math.hpp>
#include <SemanticWorld/SemanticWorld.hpp>

#include <cmath>

using namespace godot;

void Player::_ready() {
    collision_shape = memnew(CollisionShape3D);
    CapsuleShape3D* capsule = memnew(CapsuleShape3D);
    capsule->set_height(1.8f);
    collision_shape->set_shape(capsule);
    add_child(collision_shape);

    movement.instantiate();
    camera_ctl.instantiate();
    interaction.instantiate();

    // Создаём ToolManager как дочерний узел
    tool_mgr = memnew(ToolManager);
    add_child(tool_mgr);

    camera_ctl->setup_camera(this);

    if (!Engine::get_singleton()->is_editor_hint()) {
        _field_shape.instantiate();
        _field_shape->set_position(get_global_position());
        if (SemanticWorld* sw = SemanticWorld::get_singleton()) {
            sw->register_shape(_field_shape);
        }
    }
}

void Player::_exit_tree() {
    if (_field_shape.is_valid() && _field_shape->get_id() != 0) {
        if (SemanticWorld* sw = SemanticWorld::get_singleton()) {
            sw->unregister_shape_with_owned(_field_shape->get_id());
        }
    }
    _field_shape.unref();
}

void Player::_physics_process(double delta) {
    if (Engine::get_singleton()->is_editor_hint()) return;
    movement->update_movement(this, delta);
}

Vector3 Player::_resolve_gravity_up() {
    Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
    _bound_gravity_id = 0;

    if (!_field_shape.is_valid() || _field_shape->get_id() == 0) return up;

    _field_shape->consume_pending_gravity_id();
    const uint64_t gid = _field_shape->get_bound_gravity_id();
    if (gid == 0) return up;

    SemanticWorld* sw = SemanticWorld::get_singleton();
    if (!sw) return up;

    Ref<SemanticShape> shape = sw->get_shape(gid);
    GravityShape* gravity = Object::cast_to<GravityShape>(shape.ptr());
    if (!gravity) return up;

    _bound_gravity_id = gid;
    Vector3 gup = gravity->get_up_at(get_global_position());
    if (gup.length_squared() > 0.0001f) {
        up = gup.normalized();
    }
    return up;
}

void Player::_align_to_up(const Vector3& up) {
    Basis basis = get_global_basis();
    Vector3 forward = -basis.get_column(2);
    forward -= up * forward.dot(up);
    if (forward.length_squared() < 0.0001f) {
        forward = basis.get_column(0);
        forward -= up * forward.dot(up);
    }
    if (forward.length_squared() < 0.0001f) {
        Vector3 fallback = (Math::abs(up.y) < 0.99f) ? Vector3(0.0f, 1.0f, 0.0f) : Vector3(1.0f, 0.0f, 0.0f);
        forward = fallback.cross(up);
    }
    forward.normalize();
    Vector3 right = forward.cross(up);
    if (right.length_squared() < 0.0001f) return;
    right.normalize();
    forward = up.cross(right).normalized();

    Basis target(right, up, -forward);
    Basis current = get_global_basis().orthonormalized();
    target = target.orthonormalized();
    Quaternion q_cur = current.get_quaternion().normalized();
    Quaternion q_tgt = target.get_quaternion().normalized();
    if (q_cur.dot(q_tgt) < 0.0f) {
        q_tgt = Quaternion(-q_tgt.x, -q_tgt.y, -q_tgt.z, -q_tgt.w);
    }
    Quaternion q = q_cur.slerp(q_tgt, 0.2f).normalized();
    Basis smoothed(q);
    set_global_transform(Transform3D(smoothed, get_global_position()));
}

void Player::_process(double delta) {
    if (_field_shape.is_valid() && _field_shape->get_id() != 0) {
        _field_shape->set_position(get_global_position());
    }

    const Vector3 up = _resolve_gravity_up();
    set_up_direction(up);
    _align_to_up(up);

    Camera3D* cam = camera_ctl->get_camera();
    Dictionary ray = interaction->perform_raycast(cam, this);
    tool_mgr->update_tool(ray);
}

void Player::_input(const Ref<InputEvent>& event) {
    if (Engine::get_singleton()->is_editor_hint()) return;

    camera_ctl->process_input(this, event);

    Camera3D* cam = camera_ctl->get_camera();
    interaction->process_input(cam, tool_mgr, event);
}