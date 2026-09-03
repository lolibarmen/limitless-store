#pragma once

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/aabb.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <unordered_map>

namespace godot {

class SemanticWorld; // forward declaration

class SemanticShape : public Resource {
    GDCLASS(SemanticShape, Resource)

private:
    uint64_t _id = 0;
    SemanticWorld* _world = nullptr;
    
    // Ключ - просто int. Табличку соответствий ты ведёшь сам.
    std::unordered_map<int, float> _properties;
    
    Dictionary _custom_properties;

protected:
    // Protected: наследники должны иметь возможность записывать результат
    // в recompute_aabb() (шаблонный метод Template Method Pattern)
    AABB _aabb;
    bool _aabb_dirty = true;

    static void _bind_methods();

public:
    SemanticShape() = default;
    virtual ~SemanticShape() = default;

    void set_id(uint64_t id) { _id = id; }
    uint64_t get_id() const { return _id; }

    void set_world(SemanticWorld* world) { _world = world; }
    SemanticWorld* get_world() const { return _world; }

    // Чистый int API. Никаких enum.
    void set_property(int prop_id, float value);
    float get_property(int prop_id, float default_value = 0.0f) const;
    bool has_property(int prop_id) const;
    void clear_property(int prop_id);

    void set_custom(const String& key, const Variant& value);
    Variant get_custom(const String& key, const Variant& default_value = Variant()) const;
    Dictionary get_all_custom() const { return _custom_properties; }

    virtual AABB get_aabb() const;
    
    // Переопределяется в наследниках (SemanticCurve, SemanticVolume и т.д.)
    // Наследник записывает результат в _aabb и сбрасывает _aabb_dirty
    virtual void recompute_aabb() { _aabb_dirty = true; }
    
    void notify_shape_changed();

    // Возвращаем строку по умолчанию, чтобы класс не был абстрактным
    virtual String get_shape_type() const { return "shape"; }
};

} // namespace godot