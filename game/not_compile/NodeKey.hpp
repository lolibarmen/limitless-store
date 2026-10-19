#pragma once

#include <cstdint>

namespace godot {

/// @brief Утилиты для работы с ключами узлов октодерева.
/// Ключ упаковывает глубину (depth) и пространственный Morton-код в один uint64_t.
/// Формат: [8 бит depth] [56 бит morton_code]
struct NodeKeyUtils {
    /// @brief Упаковывает глубину и Morton-код в единый ключ.
    /// @param depth Глубина узла в дереве (0 = корень).
    /// @param morton_code 56-битный interleaved код пространственных координат.
    /// @return Упакованный ключ uint64_t.
    static uint64_t pack(uint8_t depth, uint64_t morton_code) {
        return (static_cast<uint64_t>(depth) << 56) | (morton_code & 0x00FFFFFFFFFFFFFFULL);
    }

    /// @brief Извлекает глубину из ключа.
    /// @param key Упакованный ключ.
    /// @return Глубина узла.
    static uint8_t get_depth(uint64_t key) {
        return static_cast<uint8_t>(key >> 56);
    }

    /// @brief Извлекает Morton-код из ключа.
    /// @param key Упакованный ключ.
    /// @return 56-битный Morton-код.
    static uint64_t get_morton(uint64_t key) {
        return key & 0x00FFFFFFFFFFFFFFULL;
    }

    /// @brief Вычисляет ключ родительского узла.
    /// @param key Ключ текущего узла.
    /// @return Ключ родителя, или 0 если это корень (depth = 0).
    static uint64_t parent_key(uint64_t key) {
        uint8_t depth = get_depth(key);
        if (depth == 0) return 0;
        
        uint64_t morton = get_morton(key) >> 3; // Сдвиг на 3 бита (3 оси)
        return pack(depth - 1, morton);
    }

    /// @brief Вычисляет ключи всех 8 дочерних узлов.
    /// @param key Ключ родительского узла.
    /// @param children Массив из 8 элементов для записи результатов.
    static void child_keys(uint64_t key, uint64_t children[8]) {
        uint8_t depth = get_depth(key);
        uint64_t morton = get_morton(key);
        uint64_t base_morton = morton << 3; // Сдвиг влево на 3 бита
        
        for (int i = 0; i < 8; ++i) {
            children[i] = pack(depth + 1, base_morton | i);
        }
    }

    /// @brief Проверяет, является ли один узел потомком другого.
    /// @param potential_descendant Ключ потенциального потомка.
    /// @param ancestor_key Ключ потенциального предка.
    /// @return true если potential_descendant является потомком ancestor_key.
    static bool is_descendant(uint64_t potential_descendant, uint64_t ancestor_key) {
        uint8_t desc_depth = get_depth(potential_descendant);
        uint8_t anc_depth = get_depth(ancestor_key);
        
        if (desc_depth <= anc_depth) return false;
        
        uint64_t desc_morton = get_morton(potential_descendant);
        uint64_t anc_morton = get_morton(ancestor_key);
        
        // Сдвигаем morton потомка вверх, чтобы сравнить с предком
        uint64_t shifted_desc = desc_morton >> (3 * (desc_depth - anc_depth));
        
        return shifted_desc == anc_morton;
    }
};

/// @brief Вычисляет Morton-код из трёх целочисленных координат.
/// Использует побитовое перемежение (bit interleaving).
/// @param x Координата X.
/// @param y Координата Y.
/// @param z Координата Z.
/// @return 56-битный Morton-код.
inline uint64_t morton_encode(int32_t x, int32_t y, int32_t z) {
    // Преобразуем в беззнаковые для корректных битовых операций
    uint64_t ux = static_cast<uint64_t>(x);
    uint64_t uy = static_cast<uint64_t>(y);
    uint64_t uz = static_cast<uint64_t>(z);
    
    // Разделяем биты каждой координаты
    auto spread_bits = [](uint64_t v) -> uint64_t {
        v = (v | (v << 32)) & 0x001F00000000FFFFULL;
        v = (v | (v << 16)) & 0x001F0000FF0000FFULL;
        v = (v | (v << 8))  & 0x100F00F00F00F00FULL;
        v = (v | (v << 4))  & 0x10C30C30C30C30C3ULL;
        v = (v | (v << 2))  & 0x1249249249249249ULL;
        return v;
    };
    
    return spread_bits(ux) | (spread_bits(uy) << 1) | (spread_bits(uz) << 2);
}

/// @brief Декодирует Morton-код обратно в три целочисленные координаты.
/// @param morton 56-битный Morton-код.
/// @param x Выходная координата X.
/// @param y Выходная координата Y.
/// @param z Выходная координата Z.
inline void morton_decode(uint64_t morton, int32_t& x, int32_t& y, int32_t& z) {
    // Собираем биты каждой координаты
    auto compact_bits = [](uint64_t v) -> uint64_t {
        v &= 0x1249249249249249ULL;
        v = (v | (v >> 2))  & 0x10C30C30C30C30C3ULL;
        v = (v | (v >> 4))  & 0x100F00F00F00F00FULL;
        v = (v | (v >> 8))  & 0x001F0000FF0000FFULL;
        v = (v | (v >> 16)) & 0x001F00000000FFFFULL;
        v = (v | (v >> 32)) & 0x0000000000003FFFULL;
        return v;
    };
    
    x = static_cast<int32_t>(compact_bits(morton));
    y = static_cast<int32_t>(compact_bits(morton >> 1));
    z = static_cast<int32_t>(compact_bits(morton >> 2));
}

} // namespace godot