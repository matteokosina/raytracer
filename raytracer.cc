#include "utility/bmp_writer.h"
#include "utility/obj_loader.h"
#include "view/renderer.h"

#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
struct Options {
    std::string input = "teapot_n_glass.obj";
    std::string output = "raytracer.bmp";
    int width = 1280;
    int height = 720;
    TriangleIntersectionAlgorithm intersection_algorithm = TriangleIntersectionAlgorithm::Badouel;
};

int parse_dimension(const char* value, const char* flag) {
    const int dimension = std::stoi(value);
    if (dimension <= 0 || dimension > 16384) throw std::runtime_error(std::string(flag)+" must be between 1 and 16384");
    return dimension;
}

Options parse_options(int argc, char** argv) {
    Options options;
    for (int i=1;i<argc;++i) {
        const std::string arg = argv[i];
        auto next = [&](const char* flag) -> const char* {
            if (i+1 >= argc) throw std::runtime_error(std::string("Missing value for ")+flag);
            return argv[++i];
        };
        if (arg == "-w") options.width = parse_dimension(next("-w"), "-w");
        else if (arg == "-h") options.height = parse_dimension(next("-h"), "-h");
        else if (arg == "-o") options.output = next("-o");
        else if (arg == "-i" || arg == "--input") options.input = next("--input");
        else if (arg == "--moeller" || arg == "--mt")
            options.intersection_algorithm = TriangleIntersectionAlgorithm::MollerTrumbore;
        else if (arg == "--help" || arg == "-help") {
            std::cout << "Usage: raytracer [-i scene.obj] [-w pixels] [-h pixels] [-o image.bmp] [--moeller|--mt]\n"
                      << "Default triangle intersection: Badouel\n";
            std::exit(0);
        } else throw std::runtime_error("Unknown option: "+arg);
    }
    return options;
}
}

int main(int argc, char** argv) {
    try {
        const Options options = parse_options(argc,argv);
        // Flags ändern Algorithmus und Ausgabe, nicht den gemeinsamen Renderablauf.
        // Die Messung umfasst Laden, Rendering und Schreiben der BMP-Datei.
        const auto start = std::chrono::steady_clock::now();
        // Die OBJ-Szene liefert Geometrie, Materialien und die Kameragrenzen.
        const World world = ObjLoader::load(options.input);
        const Camera camera(world.bounds_min(),world.bounds_max(),static_cast<double>(options.width)/options.height);
        RenderStats stats;
        const auto pixels = Renderer::render(world,camera,options.width,options.height,&stats,
            options.intersection_algorithm,[](int percent) {
                constexpr int bar_width = 30;
                const int completed = bar_width*percent/100;
                std::cerr << "\rRendering [" << std::string(completed,'#')
                          << std::string(bar_width-completed,'-') << "] "
                          << std::setw(3) << percent << '%' << std::flush;
                if (percent == 100) std::cerr << '\n';
            });
        // Erst nach vollständigem Rendering wird der Pixelpuffer als BMP geschrieben.
        BmpWriter::write(options.output,options.width,options.height,pixels);
        const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();
        std::cout << "Loaded " << world.triangles().size() << " triangles\n"
                  << "Intersection algorithm: "
                  << (options.intersection_algorithm == TriangleIntersectionAlgorithm::MollerTrumbore
                      ? "Moller-Trumbore" : "Badouel") << "\n"
                  << "Rendered " << stats.primary_rays << " rays; triangle tests: " << stats.triangle_tests << "\n"
                  << "Wrote " << options.output << " in " << seconds << " s\n";
    } catch (const std::exception& error) {
        std::cerr << "raytracer: " << error.what() << "\n";
        return 1;
    }
    return 0;
}
