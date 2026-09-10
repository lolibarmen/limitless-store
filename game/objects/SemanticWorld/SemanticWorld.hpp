#pragma once

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/mutex.hpp>
#include <godot_cpp/variant/aabb.hpp>
#include <SemanticShape/SemanticShape.hpp>
#include <unordered_map>
#include <vector>
#include <atomic>

namespace godot {

class SemanticWorld : public Object {
    GDCLASS(SemanticWorld, Object)

private:
    std::unordered_map<uint64_t, Ref<SemanticShape>> _shapes;
    Ref<Mutex> _shapes_mutex;
    std::atomic<uint64_t> _next_id{1};
    std::vector<uint64_t> _dirty_shapes;

protected:
    static void _bind_methods();

public:
    SemanticWorld();
    ~SemanticWorld() override = default;

    uint64_t register_shape(Ref<SemanticShape> shape);
    void unregister_shape(uint64_t id);
    
    std::vector<Ref<SemanticShape>> get_shapes_snapshot() const;

    int get_shape_count() const;
    TypedArray<SemanticShape> get_all_shapes() const;
    TypedArray<SemanticShape> get_shapes_by_type(const String& type) const;
    
    void mark_shape_dirty(uint64_t id);
    const std::vector<uint64_t>& get_dirty_shapes() const { return _dirty_shapes; }
    void clear_dirty_flags();
};

} // namespace godot