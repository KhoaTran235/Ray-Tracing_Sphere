#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <cstdint>

struct Vector3 {
    double x, y, z;

    Vector3() : x(0), y(0), z(0) {}
    Vector3(double x, double y, double z) : x(x), y(y), z(z) {}
    Vector3 operator+(const Vector3& v) const { return Vector3(x + v.x, y + v.y, z + v.z); }
    Vector3 operator-(const Vector3& v) const { return Vector3(x - v.x, y - v.y, z - v.z); }
    Vector3 operator*(double t) const { return Vector3(x * t, y * t, z * t); }
    double dot(const Vector3& v) const { return x * v.x + y * v.y + z * v.z; }
    double len() const { return std::sqrt(x * x + y * y + z * z); }
    Vector3 normalize() const { 
        double len = std::sqrt(x * x + y * y + z * z);
        return Vector3(x / len, y / len, z / len);
    }
};

struct Ray {
    Vector3 origin, direction;

    Ray(const Vector3& origin, const Vector3& direction) : origin(origin), direction(direction.normalize()) {}
};

struct Light {
    Vector3 pos;
    float intensity;

    Light(const Vector3& pos, const float& intensity) : pos(pos), intensity(intensity) {}
};

struct Sphere {
    Vector3 center;
    double radius;
    Vector3 color;
    double albedo;

    Sphere(const Vector3& center, double radius, const Vector3& color, double albedo) : center(center), radius(radius), color(color), albedo(albedo) {}

    bool intersect(const Ray& ray, double& t) const {
        Vector3 oc = ray.origin - center;
        double a = ray.direction.dot(ray.direction);
        double b = 2.0 * oc.dot(ray.direction);
        double c = oc.dot(oc) - radius * radius;
        double delta = b * b - 4 * a * c;
        if (delta < 0) {
            return false;
        } else {
            double t1 = (-b - std::sqrt(delta)) / (2.0 * a);
            double t2 = (-b + std::sqrt(delta)) / (2.0 * a);
            if (t1 < t2) 
                t = t1;
            else t = t2;
            return t > 0; 
        }
    }
};

Vector3 trace(const Ray& ray, const Sphere& sphere, const Light& light) {
    double t;
    if (sphere.intersect(ray, t)) {
        Vector3 intersection_point = ray.origin + ray.direction * t;        // p
        Vector3 normal = (intersection_point - sphere.center).normalize();  // n
        Vector3 light_dir = (light.pos - intersection_point).normalize();   // omega
        double distance = (light.pos - intersection_point).len();
        double cosine = std::max(0.0, normal.dot(light_dir));
        double L = light.intensity / (distance * distance);
        
        return sphere.color * (sphere.albedo * L * cosine);
    }
    return Vector3(0.1, 0.1, 0.1);  // Background color
}

#pragma pack(push, 1)
struct BMPHeader {
    uint16_t fileType{0x4D42};
    uint32_t fileSize{0};
    uint16_t reserved1{0};
    uint16_t reserved2{0};
    uint32_t offsetData{54};
};

struct BMPInfoHeader {
    uint32_t size{40};
    int32_t width{0};
    int32_t height{0};
    uint16_t planes{1};
    uint16_t bitCount{24};
    uint32_t compression{0};
    uint32_t sizeImage{0};
    int32_t xPixelsPerMeter{0};
    int32_t yPixelsPerMeter{0};
    uint32_t colorsUsed{0};
    uint32_t colorsImportant{0};
};
#pragma pack(pop)

void writeBMP(const char* filename, int width, int height, const std::vector<Vector3>& framebuffer) {
    BMPHeader header;
    BMPInfoHeader infoHeader;
    header.fileSize = sizeof(BMPHeader) + sizeof(BMPInfoHeader) + width * height * 3;
    infoHeader.width = width;
    infoHeader.height = height;

    std::ofstream outFile(filename, std::ios::binary);
    outFile.write((char*)&header, sizeof(header));
    outFile.write((char*)&infoHeader, sizeof(infoHeader));

    for (int j = height - 1; j >= 0; --j) {
        for (int i = 0; i < width; ++i) {
            auto& pixel = framebuffer[j * width + i];
            uint8_t r = static_cast<uint8_t>(255.999 * pixel.x);
            uint8_t g = static_cast<uint8_t>(255.999 * pixel.y);
            uint8_t b = static_cast<uint8_t>(255.999 * pixel.z);
            outFile.write((char*)&b, 1);
            outFile.write((char*)&g, 1);
            outFile.write((char*)&r, 1);
        }
    }

    outFile.close();
}

int main() {
    const int width = 800;
    const int height = 600;
    std::vector<Vector3> framebuffer(width * height);

    Sphere sphere(Vector3(0, 0, -1), 0.5, Vector3(1, 0, 0), 1);
    Light light(Vector3(1, 1, 0), 1.5);
    Vector3 camera(0, 0, 0);


    double viewport_height = 2.0;
    double viewport_width = (double)width / height * viewport_height;
    double focal_length = 1.0;

    Vector3 horizontal(viewport_width, 0, 0);
    Vector3 vertical(0, viewport_height, 0);
    Vector3 lower_left_corner = camera - horizontal * 0.5 - vertical * 0.5 - Vector3(0, 0, focal_length);

    for (int j = 0; j < height; ++j) {
        for (int i = 0; i < width; ++i) {
            double u = (double)i / (width - 1);
            double v = (double)j / (height - 1);
            Ray ray(camera, lower_left_corner + horizontal * u + vertical * v - camera);
            framebuffer[j * width + i] = trace(ray, sphere, light);
        }
    }
    writeBMP("output.bmp", width, height, framebuffer);
    return 0;
}