#include "utility/obj_loader.h"

#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
using Materials = std::map<std::string, Material>;

struct FaceVertex {
    int vertex = -1;
    int normal = -1;
};

int resolve_index(const std::string& token, std::size_t count) {
    const int raw = std::stoi(token);
    const long index = raw > 0 ? raw-1 : static_cast<long>(count)+raw;
    if (raw == 0 || index < 0 || static_cast<std::size_t>(index) >= count)
        throw std::runtime_error("OBJ face references an invalid vertex index");
    return static_cast<int>(index);
}

FaceVertex face_vertex(const std::string& token, std::size_t vertex_count, std::size_t normal_count) {
    const auto first = token.find('/');
    FaceVertex result;
    result.vertex = resolve_index(token.substr(0, first), vertex_count);
    if (first != std::string::npos) {
        const auto second = token.find('/', first+1);
        if (second != std::string::npos && second+1 < token.size())
            result.normal = resolve_index(token.substr(second+1), normal_count);
    }
    return result;
}

void load_materials(const std::filesystem::path& path, Materials& materials) {
    std::ifstream input(path);
    if (!input) return;
    std::string line;
    std::string active;
    while (std::getline(input, line)) {
        std::istringstream row(line);
        std::string kind;
        row >> kind;
        if (kind == "newmtl") {
            row >> active;
            materials[active] = Material{};
        } else if (!active.empty() && kind == "Kd") {
            row >> materials[active].diffuse.x >> materials[active].diffuse.y >> materials[active].diffuse.z;
        } else if (!active.empty() && kind == "Ks") {
            row >> materials[active].specular.x >> materials[active].specular.y >> materials[active].specular.z;
        } else if (!active.empty() && kind == "Ns") {
            row >> materials[active].shininess;
        } else if (!active.empty() && kind == "d") {
            row >> materials[active].opacity;
        } else if (!active.empty() && kind == "Tr") {
            double transparency = 0.0;
            if (row >> transparency) materials[active].opacity = 1.0-transparency;
        } else if (!active.empty() && kind == "Ni") {
            row >> materials[active].refractive_index;
        }
    }
}
}

World ObjLoader::load(const std::filesystem::path& path) {
    std::ifstream input(path);
    if (!input) throw std::runtime_error("Cannot open OBJ file: " + path.string());
    std::vector<Vector3> vertices;
    std::vector<Vector3> normals;
    Materials materials;
    Material active_material;
    World world;
    std::string line;
    std::size_t line_number = 0;
    while (std::getline(input, line)) {
        ++line_number;
        std::istringstream row(line);
        std::string kind;
        row >> kind;
        if (kind == "mtllib") {
            std::string material_file;
            while (row >> material_file) load_materials(path.parent_path()/material_file,materials);
        } else if (kind == "usemtl") {
            std::string name;
            row >> name;
            if (const auto found = materials.find(name); found != materials.end()) active_material = found->second;
        } else if (kind == "v") {
            Vector3 p;
            if (!(row >> p.x >> p.y >> p.z)) throw std::runtime_error("Invalid vertex at OBJ line " + std::to_string(line_number));
            vertices.push_back(p);
        } else if (kind == "vn") {
            Vector3 n;
            if (!(row >> n.x >> n.y >> n.z)) throw std::runtime_error("Invalid normal at OBJ line " + std::to_string(line_number));
            normals.push_back(n.normalized());
        } else if (kind == "f") {
            std::vector<FaceVertex> face;
            std::string token;
            while (row >> token) {
                if (!token.empty() && token[0] == '#') break;
                face.push_back(face_vertex(token, vertices.size(), normals.size()));
            }
            if (face.size() < 3) throw std::runtime_error("Face has fewer than three vertices at OBJ line " + std::to_string(line_number));
            for (std::size_t i = 1; i+1 < face.size(); ++i) {
                const FaceVertex corners[] = {face[0],face[i],face[i+1]};
                Triangle triangle;
                triangle.a = vertices[corners[0].vertex];
                triangle.b = vertices[corners[1].vertex];
                triangle.c = vertices[corners[2].vertex];
                triangle.material = active_material;
                triangle.has_vertex_normals = corners[0].normal >= 0 && corners[1].normal >= 0 && corners[2].normal >= 0;
                if (triangle.has_vertex_normals) {
                    triangle.normal_a = normals[corners[0].normal];
                    triangle.normal_b = normals[corners[1].normal];
                    triangle.normal_c = normals[corners[2].normal];
                }
                world.add_triangle(triangle);
            }
        }
    }
    if (world.triangles().empty()) throw std::runtime_error("OBJ contains no faces: " + path.string());
    return world;
}
