// game/utils/VoxelBaker.hpp
#pragma once

#include "VoxelCache.hpp"
#include <SemanticShape/SemanticShape.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <vector>

namespace godot {

class VoxelBaker {
public:
    static void bake(VoxelCache& cache, const std::vector<Ref<SemanticShape>>& shapes) {
        if (shapes.empty()) return;

        const int stride = cache.stride;
        const int step = cache.step;
        const Vector3i base_coord = cache.chunk_coord;

        std::vector<Vector3> world_positions(stride * stride * stride);
        int idx = 0;
        for (int x = 0; x < stride; x++) {
            for (int y = 0; y < stride; y++) {
                for (int z = 0; z < stride; z++, idx++) {
                    world_positions[idx] = Vector3(
                        (base_coord.x + (x - 2) * step),
                        (base_coord.y + (y - 2) * step),
                        (base_coord.z + (z - 2) * step)
                    );
                }
            }
        }

        // Для каждой фигуры проверяем только те воксели, которые попадают в её AABB
        for (const auto& shape : shapes) {
            if (shape.is_null()) continue;

            AABB bounds = shape->get_aabb();
            uint16_t mat_id = shape->get_material_id();
            
            // Небольшой запас (margin), чтобы гарантированно захватить края сглаживания
            float margin = 1.0f; 
            Vector3 min_b = bounds.position - Vector3(margin, margin, margin);
            Vector3 max_b = min_b + bounds.size + Vector3(margin * 2.0f, margin * 2.0f, margin * 2.0f);

            // Проходим по всем вокселям чанка
            for (int i = 0; i < (int)world_positions.size(); i++) {
                const Vector3& wp = world_positions[i];

                // БЫСТРАЯ ПРОВЕРКА: если точка вне расширенного AABB, пропускаем дорогой SDF
                if (wp.x < min_b.x || wp.x > max_b.x ||
                    wp.y < min_b.y || wp.y > max_b.y ||
                    wp.z < min_b.z || wp.z > max_b.z) {
                    continue;
                }

                // Вычисляем SDF только для потенциально близких точек
                float dist = shape->evaluate_sdf(wp);
                
                // Если эта фигура ближе к точке, чем всё, что было найдено ранее
                if (dist < cache.sdf_data[i]) {
                    cache.sdf_data[i] = dist;
                    cache.material_data[i] = mat_id;
                }
            }
        }
    }
};

} // namespace godot