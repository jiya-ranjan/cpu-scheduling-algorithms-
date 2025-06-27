#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <algorithm>  // for std::max

#define EPSILON 0.0009765625

struct Vec3 {
    double x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(double xx, double yy, double zz) : x(xx), y(yy), z(zz) {}

    Vec3 operator+(const Vec3& v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
    Vec3 operator-(const Vec3& v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
    Vec3 operator*(double s) const { return Vec3(x * s, y * s, z * s); }
    Vec3 operator/(double s) const { return Vec3(x / s, y / s, z / s); }

    double dot(const Vec3& v) const { return x * v.x + y * v.y + z * v.z; }
    Vec3 cross(const Vec3& v) const {
        return Vec3(
            y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x
        );
    }

    double length() const { return std::sqrt(x*x + y*y + z*z); }

    Vec3 normalize() const {
        double len = length();
        if(len == 0) return Vec3(0, 0, 0);
        return (*this) / len;
    }

    Vec3 reflect(const Vec3& normal) const {
        // r = d - 2(d · n)n
        double dotProd = this->dot(normal);
        return *this - normal * (2 * dotProd);
    }
};

inline double clamp(double x, double lower, double upper) {
    return x < lower ? lower : (x > upper ? upper : x);
}

struct Ray {
    Vec3 orig;
    Vec3 dir;
};

struct Light {
    Vec3 pos;
    Vec3 col;
    Vec3 aten;
};

struct Texture {
    double amb;
    double dif;
    double spec;
    double shin;
    double refl;
};

struct Sphere {
    int pgm;
    int tex;
    Vec3 cent;
    double rad;
};

char *image;
int width, height, depth;
int lightCount, pigmentCount, textureCount, sphereCount;
double aspect, fovy, w, h;
Vec3 eye, at, up, fwd, left, *pigments, *pixels;
Light *lights;
Texture *textures;
Sphere *spheres;
unsigned char *data;

void initialize(int argc, char **argv)
{
    if(argc < 2)
    {
        printf("Usage: %s <filename> [depth]\n", argv[0]);
        exit(EXIT_SUCCESS);
    }

    if(argc < 3)
        depth = 4;
    else
        depth = atoi(argv[2]);

    FILE *input = fopen(argv[1], "r");

    if(input == NULL)
    {
        printf("Cannot open specified input file\n");
        exit(EXIT_FAILURE);
    }

    char placeholder[256];
    image = (char*) malloc(256);

    fscanf(input, "%s", image);
    fscanf(input, "%d %d", &width, &height);
    aspect = (double) width / (double) height;
    pixels = (Vec3*) malloc(height * width * sizeof(Vec3));

    fscanf(input, "%lf %lf %lf", &eye.x, &eye.y, &eye.z);
    fscanf(input, "%lf %lf %lf", &at.x, &at.y, &at.z);
    fscanf(input, "%lf %lf %lf", &up.x, &up.y, &up.z);
    fscanf(input, "%lf", &fovy);

    up = up.normalize();
    fwd = (at - eye).normalize();
    left = up.cross(fwd).normalize();
    up = fwd.cross(left).normalize();
    at = eye + fwd;
    h = 2 * tan(fovy / 2);
    w = aspect * h;

    fscanf(input, "%d", &lightCount);
    lights = (Light*) malloc(lightCount * sizeof(Light));

    for(int i = 0; i < lightCount; i++)
    {
        fscanf(input, "%lf %lf %lf", &lights[i].pos.x, &lights[i].pos.y, &lights[i].pos.z);
        fscanf(input, "%lf %lf %lf", &lights[i].col.x, &lights[i].col.y, &lights[i].col.z);
        fscanf(input, "%lf %lf %lf", &lights[i].aten.x, &lights[i].aten.y, &lights[i].aten.z);
    }

    fscanf(input, "%d", &pigmentCount);
    pigments = (Vec3*) malloc(pigmentCount * sizeof(Vec3));

    for(int i = 0; i < pigmentCount; i++)
        fscanf(input, "%s %lf %lf %lf", placeholder, &pigments[i].x, &pigments[i].y, &pigments[i].z);

    fscanf(input, "%d", &textureCount);
    textures = (Texture*) malloc(textureCount * sizeof(Texture));

    for(int i = 0; i < textureCount; i++)
        fscanf(input, "%lf %lf %lf %lf %lf", &textures[i].amb, &textures[i].dif, &textures[i].spec, &textures[i].shin, &textures[i].refl);

    fscanf(input, "%d", &sphereCount);
    spheres = (Sphere*) malloc(sphereCount * sizeof(Sphere));

    for(int i = 0; i < sphereCount; i++)
    {
        fscanf(input, "%d %d %s", &spheres[i].pgm, &spheres[i].tex, placeholder);
        fscanf(input, "%lf %lf %lf", &spheres[i].cent.x, &spheres[i].cent.y, &spheres[i].cent.z);
        fscanf(input, "%lf", &spheres[i].rad);
    }

    fclose(input);
}

int compare(double a, double b)
{
    double d = a - b;

    if(d < -EPSILON)
        return -1;
    else if(d > EPSILON)
        return 1;
    else
        return 0;
}

Vec3 intensity(int index, double distance)
{
    Vec3 col = lights[index].col;
    Vec3 aten = lights[index].aten;
    return col / (aten.x + distance * aten.y + distance * distance * aten.z);
}

Vec3 pigment(int index)
{
    return pigments[spheres[index].pgm];
}

Texture texture(int index)
{
    return textures[spheres[index].tex];
}

double intersect(Ray ray, Sphere sphere)
{
    Vec3 d = ray.orig - sphere.cent;
    double a = ray.dir.dot(ray.dir);
    double b = 2 * ray.dir.dot(d);
    double c = d.dot(d) - sphere.rad * sphere.rad;
    double dsc = b * b - 4 * a * c;

    int state = compare(dsc, 0.0);
    double result = INFINITY;

    if(state == 0)
    {
        double t = -b / (2 * a);

        if(compare(0.0, t) < 0 && compare(t, INFINITY) < 0)
            result = t;
    }
    else if(state == 1)
    {
        double s = sqrt(dsc);
        double t0 = (-b - s) / (2 * a);
        double t1 = (-b + s) / (2 * a);

        if(compare(0.0, t0) < 0 && compare(t0, INFINITY) < 0)
            result = t0;
        else if(compare(0.0, t1) < 0 && compare(t1, result) < 0)
            result = t1;
    }

    return result;
}

Vec3 traceLight(Ray ray, int index, double t, int step);

Vec3 traceRay(Ray ray, int step)
{
    int index = -1;
    double t = INFINITY;

    for(int i = 0; i < sphereCount; i++)
    {
        double z = intersect(ray, spheres[i]);

        if(compare(z, t) < 0)
        {
            t = z;
            index = i;
        }
    }

    if(compare(t, INFINITY) >= 0)
        return Vec3(0.5, 0.5, 0.5);

    return traceLight(ray, index, t, step);
}

Vec3 traceLight(Ray ray, int index, double t, int step)
{
    Vec3 point = ray.orig + ray.dir * (t - EPSILON);
    Vec3 normal = (point - spheres[index].cent).normalize();
    Vec3 color = pigment(index) * texture(index).amb * lights[0].col;

    if((ray.orig - spheres[index].cent).length() < spheres[index].rad)
        normal = normal * -1;

    for(int i = 1; i < lightCount; i++)
    {
        double tn = INFINITY;
        Vec3 direction = lights[i].pos - point;
        double distance = direction.length();
        Ray light{point, direction.normalize()};

        for(int j = 0; j < sphereCount; j++)
        {
            double z = intersect(light, spheres[j]);

            if(compare(z, tn) < 0 && compare(z, distance) < 0)
                tn = z;
        }

        if(compare(tn, INFINITY) >= 0)
        {
            Vec3 diffuse = pigment(index) * texture(index).dif * std::max(normal.dot(light.dir), 0.0);
            Vec3 viewDir = (ray.orig - point).normalize();
            Vec3 reflectDir = light.dir.reflect(normal);
            Vec3 specular = texture(index).spec * std::pow(std::max(viewDir.dot(reflectDir), 0.0), texture(index).shin);
            color = color + intensity(i, distance) * (diffuse + specular);
        }
    }

    if(compare(0.0, texture(index).refl) >= 0)
        return color;

    if(step <= 0)
        return (1.0 - texture(index).refl) * color;

    ray.orig = point;
    ray.dir = ray.dir.reflect(normal);

    return (1.0 - texture(index).refl) * color + texture(index).refl * traceRay(ray, step - 1);
}

void render()
{
    pixels = (Vec3*) malloc(height * width * sizeof(Vec3));

    for(int i = 0; i < width * height; i++)
    {
        double x = w * (i % width) / width - w / 2;
        double y = h * (i / width) / height - h / 2;

        Vec3 origin = at - left * x - up * y;
        Vec3 direction = (origin - eye).normalize();

        pixels[i] = traceRay(Ray{origin, direction}, depth);
    }
}

void finalize()
{
    data = (unsigned char*) malloc(height * width * 3);

    for(int i = 0; i < height * width; i++)
    {
        data[i * 3 + 0] = (unsigned char) clamp(pixels[i].x * 255.0, 0.0, 255.0);
        data[i * 3 + 1] = (unsigned char) clamp(pixels[i].y * 255.0, 0.0, 255.0);
        data[i * 3 + 2] = (unsigned char) clamp(pixels[i].z * 255.0, 0.0, 255.0);
    }

    free(lights);
    free(pigments);
    free(textures);
    free(spheres);
    free(pixels);

    FILE *output = fopen(image, "wb");

    if(output == NULL)
    {
        printf("Cannot create specified output file\n");
        exit(EXIT_FAILURE);
    }

    fprintf(output, "P6\n%d %d\n255\n", width, height);
    fwrite(data, 1, width * height * 3, output);
    fclose(output);

    free(image);
    free(data);
}

int main(int argc, char **argv)
{
    initialize(argc, argv);
    render();
    finalize();
    return 0;
}
