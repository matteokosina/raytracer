#include "utility/bmp_writer.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <stdexcept>

namespace {
void put_u16(std::ofstream& out, std::uint16_t value) { out.put(value & 255); out.put((value >> 8) & 255); }
void put_u32(std::ofstream& out, std::uint32_t value) {
    for (int i=0;i<4;++i) out.put((value >> (8*i)) & 255);
}
unsigned char channel(double value) {
    const double corrected = std::sqrt(std::clamp(value, 0.0, 1.0));
    return static_cast<unsigned char>(std::lround(corrected*255.0));
}
}

void BmpWriter::write(const std::filesystem::path& path, int width, int height,
                      const std::vector<Vector3>& pixels) {
    if (width <= 0 || height <= 0 || pixels.size() != static_cast<std::size_t>(width)*height)
        throw std::invalid_argument("Invalid BMP dimensions or pixel buffer size");
    const std::uint32_t stride = (static_cast<std::uint32_t>(width)*3 + 3) & ~3u;
    const std::uint32_t image_size = stride*static_cast<std::uint32_t>(height);
    std::ofstream out(path, std::ios::binary);
    if (!out) throw std::runtime_error("Cannot create BMP file: " + path.string());
    out.put('B'); out.put('M'); put_u32(out,54+image_size); put_u16(out,0); put_u16(out,0); put_u32(out,54);
    put_u32(out,40); put_u32(out,width); put_u32(out,height); put_u16(out,1); put_u16(out,24);
    put_u32(out,0); put_u32(out,image_size); put_u32(out,2835); put_u32(out,2835); put_u32(out,0); put_u32(out,0);
    // BMP speichert Zeilen von unten nach oben und Farbkanäle in BGR-Reihenfolge.
    for (int y=height-1;y>=0;--y) {
        for (int x=0;x<width;++x) {
            const auto& color = pixels[static_cast<std::size_t>(y)*width+x];
            out.put(channel(color.z)); out.put(channel(color.y)); out.put(channel(color.x));
        }
        for (std::uint32_t pad=static_cast<std::uint32_t>(width)*3;pad<stride;++pad) out.put(0);
    }
    if (!out) throw std::runtime_error("Failed while writing BMP file: " + path.string());
}
