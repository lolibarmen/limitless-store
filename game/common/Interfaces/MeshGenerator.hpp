#ifndef MESH_GENERATOR_INTERFACE_H
#define MESH_GENERATOR_INTERFACE_H

#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/string.hpp>

#include "mesh_primitive.h"

namespace godot {

// Базовый интерфейс генератора меша — обычный C++ класс, НЕ наследник
// Wrapped/Node. Не регистрируется в ClassDB, не виден в GDScript и в
// редакторе как отдельный тип узла. Весь полиморфизм — чистая C++
// виртуальная диспетчеризация, разрешается на этапе компиляции.
//
// ВАЖНО ДЛЯ СБОРЩИКА: этот файл header-only и не является Godot-классом,
// поэтому его нельзя класть в game/virtual (тот каталог автоматически
// регистрирует любой найденный класс через ClassDB::register_class,
// что не скомпилируется для обычного класса). Держите его, например,
// в game/common — эта директория в CPPPATH, но не сканируется
// discover_cpp_classes(), так что ложной регистрации не будет.
class MeshGeneratorInterface {
	Dictionary primitives_by_tag; // String tag -> Array<MeshPrimitive*>

public:
	virtual ~MeshGeneratorInterface() = default;

	// Регистрация примитивов, которые алгоритм наследника сможет использовать.
	void register_primitive(const String &p_tag, MeshPrimitive *p_primitive) {
		if (p_primitive == nullptr) {
			return;
		}
		Array list;
		if (primitives_by_tag.has(p_tag)) {
			list = primitives_by_tag[p_tag];
		}
		list.push_back(p_primitive);
		primitives_by_tag[p_tag] = list;
	}

	void unregister_primitive(const String &p_tag, MeshPrimitive *p_primitive) {
		if (!primitives_by_tag.has(p_tag)) {
			return;
		}
		Array list = primitives_by_tag[p_tag];
		list.erase(p_primitive);
		primitives_by_tag[p_tag] = list;
	}

	Array get_primitives(const String &p_tag) const {
		if (primitives_by_tag.has(p_tag)) {
			return primitives_by_tag[p_tag];
		}
		return Array();
	}

	PackedStringArray get_tags() const {
		PackedStringArray result;
		Array keys = primitives_by_tag.keys();
		for (int i = 0; i < keys.size(); i++) {
			result.push_back(keys[i]);
		}
		return result;
	}

	void clear_primitives() {
		primitives_by_tag.clear();
	}

	// Точка входа: делегирует сборку чисто виртуальному методу наследника.
	Ref<ArrayMesh> generate() {
		return build_mesh();
	}

protected:
	// Наследник — обычный C++ класс, известный целиком уже на этапе
	// компиляции — реализует здесь свой алгоритм сборки меша из
	// зарегистрированных примитивов.
	virtual Ref<ArrayMesh> build_mesh() = 0;
};

} //namespace godot

#endif // MESH_GENERATOR_INTERFACE_H