#pragma once

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/hash_set.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/typed_array.hpp>

namespace godot {

class BuildGraph : public Object {
    GDCLASS(BuildGraph, Object)

public:
    struct Edge {
        ObjectID other;
        String my_socket_id;
        String other_socket_id;
    };

private:
    static BuildGraph *singleton;

    // object -> список рёбер, идущих от него
    HashMap<ObjectID, Vector<Edge>> adjacency;
    // чтобы не подписываться на tree_exiting дважды на один и тот же узел
    HashSet<ObjectID> watched;

    void _watch(Node3D *p_node);

protected:
    static void _bind_methods();

public:
    BuildGraph();
    ~BuildGraph();

    static BuildGraph *get_singleton();

    void connect_nodes(Node3D *a, Node3D *b,
                        const String &a_socket_id = String(),
                        const String &b_socket_id = String());
    void disconnect_nodes(Node3D *a, Node3D *b);
    void remove_node(Node3D *node);
    bool are_connected(Node3D *a, Node3D *b) const;
    TypedArray<Node3D> get_connected(Node3D *node) const;
    void clear();
};

} // namespace godot