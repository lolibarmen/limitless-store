#pragma once
#include <godot_cpp/classes/resource.hpp>
#include <unordered_map>
// #include <XSource/XTypes.hpp>       // enum и структуры данных
// #include <XGenerator/XGenerator.hpp>
// #include <utils/SpatialHash.hpp>    // Vector3iHash / Vector2iHash

namespace godot {

class XSource : public Resource {
    GDCLASS(XSource, Resource)

private:
    Ref<XGenerator> generator;

    // Хранилище изменений игрока.
    // Ключ — позиция в мировых координатах.
    // Если позиция есть в edits — возвращаем её, иначе идём в генератор.
    std::unordered_map<Vector3i, XData, Vector3iHash> edits;

protected:
    static void _bind_methods();

public:
    XSource() = default;
    ~XSource() override = default;

    // Вызывается из WorldCoordinator.
    // Создаёт и инициализирует генератор внутри.
    void init(/* Ref<YSource> y_source, */ int seed);

    // --- основной интерфейс ---

    // Сначала смотрит в edits, потом идёт в генератор.
    XData get_x(const Vector3i& world_pos) const;

    // Записать изменение игрока.
    void set_x(const Vector3i& world_pos, XData data);

    // Вернуть позицию к процедурному состоянию.
    void reset_x(const Vector3i& world_pos);

    bool has_edit(const Vector3i& world_pos) const;

    // --- доступ к генератору если нужен снаружи ---
    Ref<XGenerator> get_generator() const { return generator; }
};

} // namespace godot
```

---

Два правила которые держат все источники и генераторы консистентными:
```
Generator:  init(Ref<YSource>, int seed) → get_x(Vector3i)
Source:     init(Ref<YSource>, int seed) → get_x(Vector3i)
                                         → set_x / reset_x / has_edit