#pragma once
#include <godot_cpp/classes/resource.hpp>
// #include <godot_cpp/classes/fast_noise_lite.hpp> // если нужны шумы
// #include <XSource/XTypes.hpp>                    // enum и структуры данных
// #include <YSource/YSource.hpp>                   // зависимость уровнем выше

namespace godot {

class XGenerator : public Resource {
    GDCLASS(XGenerator, Resource)

private:
    int seed = 0;

    // Ref<FastNoiseLite> noise_something;  // шумы если нужны
    // float something_scale = 0.003f;      // масштабы шумов

    // Ref<YSource> y_source;               // зависимость уровнем выше
                                            // (биом для блоков, лес для деревьев)

    void setup_noise();                     // инициализация шумов через seed

    // XData classify(...) const;           // логика классификации по значениям шумов

protected:
    static void _bind_methods();

public:
    XGenerator() = default;
    ~XGenerator() override = default;

    // Единственная точка входа.
    // Принимает зависимость уровнем выше и seed.
    // Если зависимости нет (корневой генератор) — только seed.
    void init(/* Ref<YSource> p_y_source, */ int p_seed);

    // Основной метод — возвращает данные по позиции в мире.
    // XData get_x(const Vector3i& world_pos) const;

    int get_seed() const { return seed; }
};

} // namespace godot