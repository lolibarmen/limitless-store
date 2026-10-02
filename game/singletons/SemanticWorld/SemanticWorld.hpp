#pragma once

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/mutex.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <SemanticShape/SemanticShape.hpp>
#include <MeshGenerator/MeshGenerator.hpp>
#include <unordered_map>
#include <vector>
#include <atomic>
#include <cstdint>

namespace godot {

// Константа может пригодиться SemanticShape для определения состояния выгрузки
constexpr int LOD_UNLOADED = -1;

class SemanticWorld : public Object {
    GDCLASS(SemanticWorld, Object)

private:
    std::unordered_map<uint64_t, Ref<SemanticShape>> _shapes;
    Ref<Mutex> _shapes_mutex;
    std::atomic<uint64_t> _next_id{1};

protected:
    static void _bind_methods();

public:
    SemanticWorld();
    ~SemanticWorld() override = default;

    static SemanticWorld* get_singleton();

    uint64_t register_shape(Ref<SemanticShape> shape);
    void unregister_shape(uint64_t id);

    Ref<SemanticShape> get_shape(uint64_t id) const;
    
    std::vector<Ref<SemanticShape>> get_shapes_snapshot() const;

    int get_shape_count() const;
    TypedArray<SemanticShape> get_all_shapes() const;
    TypedArray<SemanticShape> get_shapes_by_type(const String& type) const;

    // Теперь принимаем только массив ID чанков
    void on_zone_changed(const std::vector<uint64_t>& chunk_ids);
};

} // namespace godot