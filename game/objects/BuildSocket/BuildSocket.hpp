#pragma once

#include <godot_cpp/classes/area3d.hpp>
#include <godot_cpp/core/class_db.hpp>

namespace godot {

class BuildSocket : public Area3D {
    GDCLASS(BuildSocket, Area3D)

private:
    String socket_id;
    String connects_to_id;

protected:
    static void _bind_methods() {
        ClassDB::bind_method(D_METHOD("set_socket_id", "id"), &BuildSocket::set_socket_id);
        ClassDB::bind_method(D_METHOD("get_socket_id"), &BuildSocket::get_socket_id);
        ClassDB::bind_method(D_METHOD("set_connects_to_id", "id"), &BuildSocket::set_connects_to_id);
        ClassDB::bind_method(D_METHOD("get_connects_to_id"), &BuildSocket::get_connects_to_id);

        ADD_PROPERTY(PropertyInfo(Variant::STRING, "socket_id"), "set_socket_id", "get_socket_id");
        ADD_PROPERTY(PropertyInfo(Variant::STRING, "connects_to_id"), "set_connects_to_id", "get_connects_to_id");
    }

public:
    BuildSocket() = default;
    ~BuildSocket() override = default;

    void set_socket_id(const String &p_id) { socket_id = p_id; }
    String get_socket_id() const { return socket_id; }

    void set_connects_to_id(const String &p_id) { connects_to_id = p_id; }
    String get_connects_to_id() const { return connects_to_id; }
};

} // namespace godot