#include "SurfaceNets.hpp"

#include <map>
#include <cstdio>

using namespace std;
using namespace godot;

static const Vector3i CORNER_OFFSET[8] = {
    {0,0,0},{1,0,0},{0,1,0},{1,1,0},
    {0,0,1},{1,0,1},{0,1,1},{1,1,1},
};

static constexpr int EDGE_TABLE[12][2] = {
    {0,1}, {2,3}, {4,5}, {6,7},
    {0,2}, {1,3}, {4,6}, {5,7},
    {0,4}, {1,5}, {2,6}, {3,7},
};

static const struct { Vector3i a1, a2, n12; } EDGE_AXES[3] = {
    { {1,0,0}, {0,1,0}, {0,0,1} },
    { {0,0,1}, {1,0,0}, {0,1,0} },
    { {0,1,0}, {0,0,1}, {1,0,0} }
};

MeshData godot::build_neochunk_mesh(const ChunkBuildInput& input) {
    MeshData result;
    
    map<Vector3i, Vector3> voxel_vertices;
    
    // Первый проход: собираем meanP для каждого вокселя
    for(int x = -1; x <= input.voxel_count; x++)
    for(int y = -1; y <= input.voxel_count; y++)
    for(int z = -1; z <= input.voxel_count; z++)
    {
        bool have_positive = false;
        bool have_negative = false;
        float densities[2][2][2];

        // Отсекаем блоки без материи
        for(Vector3i off : CORNER_OFFSET) {
            float density = input.get_sdf(Vector3i(x,y,z) + off);

            if(density >= 0.0f)
                have_positive = true;
            else if(density <= 0.0f)
                have_negative = true;

            densities[off.x][off.y][off.z] = density;
        }

        if(!have_positive || !have_negative) continue;

        std::vector<Vector3> edge_points;
        
        for(auto& edge : EDGE_TABLE) {
            Vector3i offA = CORNER_OFFSET[edge[0]];
            Vector3i offB = CORNER_OFFSET[edge[1]];

            float fA = densities[offA.x][offA.y][offA.z];
            float fB = densities[offB.x][offB.y][offB.z];

            if ((fA < 0) != (fB < 0)) {
                float t = (0 - fA) / (fB - fA);

                Vector3 A(x + offA.x, y + offA.y, z + offA.z);
                Vector3 B(x + offB.x, y + offB.y, z + offB.z);

                Vector3 P = A + t * (B - A);
                
                edge_points.push_back(P);
            }
        }

        if(edge_points.size() < 3) continue;

        // Находим среднюю точку
        Vector3 meanP;
        for(Vector3 P : edge_points) {
            meanP = meanP + P;
        }
        meanP = meanP / edge_points.size();

        float scale = (1 << input.lod_level);
        Vector3 offset = Vector3(input.chunk_size, input.chunk_size, input.chunk_size) * 0.5f;

        voxel_vertices[{x,y,z}] = meanP * scale - offset;
    }
    
    // Второй проход: строим квады между соседними вокселями
    for(auto& [coord, meanP] : voxel_vertices)
    {
        for(auto& [a1, a2, n12] : EDGE_AXES)
        {
            Vector3i c00 = coord;
            Vector3i c10 = coord - a1;
            Vector3i c01 = coord      - a2;
            Vector3i c11 = coord - a1 - a2;

            if(voxel_vertices.find(c10) == voxel_vertices.end()) continue;
            if(voxel_vertices.find(c01) == voxel_vertices.end()) continue;
            if(voxel_vertices.find(c11) == voxel_vertices.end()) continue;

            Vector3 v00 = voxel_vertices[c00];
            Vector3 v10 = voxel_vertices[c10];
            Vector3 v01 = voxel_vertices[c01];
            Vector3 v11 = voxel_vertices[c11];

            Vector3 normal = (v01 - v00).cross(v11 - v00).normalized();

            // Читаем SDF из нового кэша
            float dA = input.get_sdf(coord);
            float dB = input.get_sdf(coord + n12);
            if ((dA < 0) == (dB < 0)) continue;

            bool flip = (dB >= 0);

            // Определяем, какой воксель "твёрдый" (внутри), чтобы взять его материал
            // Vector3i solid_coord = (dA > 0) ? coord : coord + n12;
            // uint16_t material_id = input.get_material(solid_coord);
            
            // Нормализуем material_id в диапазон [0, 1] для цвета
            // float id = static_cast<float>(material_id) / 255.0f;

            // auto getVertexColor = [&](Vector3i gridCoord) -> Color {
            //     return Color(id, id, 0.0f, 1.0f);
            // };

            // Color col00 = getVertexColor(c00);
            // Color col10 = getVertexColor(c10);
            // Color col01 = getVertexColor(c01);
            // Color col11 = getVertexColor(c11);

            if (flip) {
                normal = -normal;
                // Треугольник 1: v00, v01, v11
                result.vertices.push_back(v00); result.normals.push_back(normal); //result.colors.push_back(col00);
                result.vertices.push_back(v01); result.normals.push_back(normal); //result.colors.push_back(col01);
                result.vertices.push_back(v11); result.normals.push_back(normal); //result.colors.push_back(col11);
                // Треугольник 2: v00, v11, v10
                result.vertices.push_back(v00); result.normals.push_back(normal); //result.colors.push_back(col00);
                result.vertices.push_back(v11); result.normals.push_back(normal); //result.colors.push_back(col11);
                result.vertices.push_back(v10); result.normals.push_back(normal); //result.colors.push_back(col10);
            } else {
                // Треугольник 1: v00, v10, v11
                result.vertices.push_back(v00); result.normals.push_back(normal); //result.colors.push_back(col00);
                result.vertices.push_back(v10); result.normals.push_back(normal); //result.colors.push_back(col10);
                result.vertices.push_back(v11); result.normals.push_back(normal); //result.colors.push_back(col11);
                // Треугольник 2: v00, v11, v01
                result.vertices.push_back(v00); result.normals.push_back(normal); //result.colors.push_back(col00);
                result.vertices.push_back(v11); result.normals.push_back(normal); //result.colors.push_back(col11);
                result.vertices.push_back(v01); result.normals.push_back(normal); //result.colors.push_back(col01);
            }
        }
    }

    return result;
}