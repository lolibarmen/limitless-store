#pragma once

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/variant/aabb.hpp>
#include <SemanticShape/SemanticShape.hpp>
#include <unordered_map>
#include <vector>
#include <functional>
#include <atomic>

namespace godot {

class SemanticWorld : public Object {
    GDCLASS(SemanticWorld, Object)

private:
    std::unordered_map<uint64_t, Ref<SemanticShape>> _shapes;
    std::atomic<uint64_t> _next_id{1};
    std::vector<uint64_t> _dirty_shapes;
    
    // Callback для dirty propagation (быстрее сигналов)
    std::function<void(uint64_t, const AABB&)> _on_shape_changed;

protected:
    static void _bind_methods();

public:
    SemanticWorld() = default;
    ~SemanticWorld() override = default;

    // --- Регистрация фигур ---
    uint64_t register_shape(Ref<SemanticShape> shape);
    void unregister_shape(uint64_t id);
    Ref<SemanticShape> get_shape(uint64_t id) const;
    
    // --- Запросы ---
    int get_shape_count() const { return (int)_shapes.size(); }
    TypedArray<SemanticShape> get_all_shapes() const;
    TypedArray<SemanticShape> get_shapes_by_type(const String& type) const;
    
    // --- Dirty propagation ---
    void mark_shape_dirty(uint64_t id);
    const std::vector<uint64_t>& get_dirty_shapes() const { return _dirty_shapes; }
    void clear_dirty_flags();
    
    // --- Callback для внешних систем (ChunkManager и т.д.) ---
    void set_on_shape_changed(std::function<void(uint64_t, const AABB&)> callback);
};

} // namespace godot