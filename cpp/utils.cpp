#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

using namespace std;

static float hue2rgb(float p, float q, float t) {
    if (t < 0.0f) t += 1.0f;
    if (t > 1.0f) t -= 1.0f;
    if (t < 1.0f/6.0f) return p + (q - p) * 6.0f * t;
    if (t < 1.0f/2.0f) return q;
    if (t < 2.0f/3.0f) return p + (q - p) * (2.0f/3.0f - t) * 6.0f;
    return p;
}

static void hls_to_rgb(float h, float l, float s, int& r, int& g, int& b) {
    float rf, gf, bf;

    if (s == 0.0f) {
        rf = gf = bf = l;
    } else {
        float q = (l < 0.5f) ? (l * (1.0f + s)) : (l + s - l * s);
        float p = 2.0f * l - q;
        rf = hue2rgb(p, q, h + 1.0f/3.0f);
        gf = hue2rgb(p, q, h);
        bf = hue2rgb(p, q, h - 1.0f/3.0f);
    }

    r = static_cast<int>(rf * 255.0f);
    g = static_cast<int>(gf * 255.0f);
    b = static_cast<int>(bf * 255.0f);
}

std::string numberToHEX(float h, float l, float s) {
    int r, g, b;
    hls_to_rgb(h, l, s, r, g, b);

    std::ostringstream oss;
    oss << std::hex << std::setfill('0')
        << std::setw(2) << r
        << std::setw(2) << g
        << std::setw(2) << b;

    return oss.str();
}

std::string visualizer(const std::string& text, const std::string& hexcode) {
    int hexint = std::stoi(hexcode, nullptr, 16);

    int r = (hexint >> 16) & 0xFF;
    int g = (hexint >> 8) & 0xFF;
    int b = hexint & 0xFF;

    std::ostringstream oss;
    oss << "\033[38;2;255;255;255;48;2;"
        << r << ";" << g << ";" << b << "m"
        << text
        << "\033[0m";

    return oss.str();
}
