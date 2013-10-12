#include <cstdio> // cray_ray - a teeny tiny obfuscated ray tracer
#include <cmath> // tom milsom | github.com/colourblind | monochromcy.net
#include <cstdlib> // lighting, multiple spheres, reflections, fog

unsigned char BMP_HEADER[] = { 
    0x42,   // BMP signature 
    0x4d,
    0x1d,   // File size (255 * 255 * 3) + 14 + 12
    0xfa,
    0x02,
    0x00,
    0x00,   // Reserved 1
    0x00,
    0x00,   // Reserved 2
    0x00,
    0x1a,   // Data offset (14 + 12)
    0x00,
    0x00,
    0x00 
};

unsigned char DIB_HEADER[] = {
    0x0c,   // DIB header size (12)
    0x00,
    0x00,
    0x00,
    0x00,   // Width (512)
    0x02,
    0x00,   // Height (512)
    0x02,
    0x01,   // Colour planes (1)
    0x00,
    0x18,   // Bits-per-pixel (24)
    0x00
};

int SPHERES[] = {
   0x80808003,  // (0, 0, 0) r3
   0x84838202,  // (4, 3, 2) r2
   0x79857c05,  // (-7, 5, -4) r5
   0x817e8301,  // (1, -2, 3) r1
   0x84867409,  // (4, 6, -12) r9
   0x767f8003,  // (-10, -1, 0) r3
   0x7d7d8401   // (-3, -3, 4) r1
};

float clamp(float x) { return x < 0 ? 0 : x > 1 ? 1 : x; }
float random() { return (float)rand() / RAND_MAX; }

struct vector
{
    float x, y, z;

    vector() { x = y = z = 0; }
    vector(float a, float b, float c) { x = a, y = b, z = c; }

    vector operator +(vector a) { return vector(x + a.x, y + a.y, z + a.z); }
    vector operator -(vector a) { return vector(x - a.x, y - a.y, z - a.z); }
    vector operator *(float s) { return vector(x * s, y * s, z * s); }

    float operator ^(vector a) { return x * a.x + y * a.y + z * a.z; }
    vector operator %(vector a) { return vector(y * a.z - z * a.y, z * a.x - x * a.z, x * a.y - y * a.x); }
    vector operator ~() { return *this * (1.f / sqrtf(*this ^ *this)); }

    vector lerp(vector a, float f) { return vector(x * f + a.x * (1 - f), y * f + a.y * (1 - f), z * f + a.z * (1 - f)); }

    void set(float a, float b, float c) { x = a; y = b; z = c; }
    void set(vector v) { x = v.x; y = v.y; z = v.z; }
};

float hit(vector origin, vector dir, int reflection_count, vector *colour)
{
    float FOG_RADIUS = 30.f;

    if (reflection_count < 1)
        return FOG_RADIUS;

    float d = -1;

    vector a;
    vector intersection;
    vector normal;
    vector l, r;
    vector light_pos(1, 12, 3);
    float radius;

    for (int j = 0; j < 7; j ++)
    {
        vector sphere = vector((float)(SPHERES[j] >> 24 & 0xff) - 128, (float)(SPHERES[j] >> 16 & 0xff) - 128, (float)(SPHERES[j] >> 8 & 0xff) - 128);
        radius = SPHERES[j] & 0xff;
        vector oc = (origin - sphere);
        float loc = dir ^ oc;
        float t = (loc * loc) - (oc ^ oc) + (radius * radius);
        float t0 = -loc + sqrtf(t);
        float t1 = -loc - sqrtf(t);

        if (t0 >= 0 || t1 >= 0) // We'e hit our sphere
        {
            t = t0 < t1 ? t0 : t1;
            if (d < 0 || t < d)
            {
                d = t;
                intersection = origin + (dir * d);
                normal = ~(intersection - sphere);
            }
        }
    }

    if (d >= 0) // Reflection
    {
        hit(intersection + normal * 0.001f, normal, reflection_count - 1, &a);
        a = a * 0.8f; // attenuate

        // Specular
        l = ~(light_pos - intersection);
        r = ~(normal * (normal ^ l) * -2 + l);
        float spec = pow(clamp(r ^ dir), 80) * 255;
        a = a + vector(spec, spec, spec);
    }
    else
    {
        normal = vector(0, 1, 0);
        d = ((vector(0, -4, 0) - origin) ^ normal) / (dir ^ normal); // Floor at -4

        if (d > 0) // Floor
        {
            intersection = origin + dir * d; // Point of intersection
            int s = abs((int)ceil(intersection.x)) % 2 == abs((int)ceil(intersection.z)) % 2; // Checkerboard 'texture'
            a.set(s ? 0 : 255, s ? 0 : 255, s ? 255 : 0);
            l = ~(light_pos - intersection);
            float intensity = hit(intersection, l, 1, &r) < FOG_RADIUS ? 0.4 : 1;  // Trace for shadows - throw in the r vector since we don't care about the result of colour
            a = a * (l ^ normal) * intensity; // Diffuse lighting
        }
        else // Sky
        {
            a.set(255, 0, 0); // Mr Blue Sky
            d = FOG_RADIUS; // Otherwise we get into NaN shenanigans
        }
    }

    colour->set(a.lerp(vector(128, 0, 0), ((FOG_RADIUS - (d > FOG_RADIUS ? FOG_RADIUS : d)) / FOG_RADIUS)));
    return d;
}

int main()
{
    unsigned char data[786432]; // 512 * 512 * 3
    unsigned char *ptr = data;
    vector cam_dir(0, 0, -1);
    vector colour, acc;

    for (int y = 0; y < 512; y ++)
    {
        for (int x = 0; x < 512; x ++)
        {
            acc = vector();
            for (int z = 0; z < 16; z ++)
            {
                vector dir(cam_dir.x + (x - 256 + random()) * 0.0025f, cam_dir.y + (y - 256 + random()) * 0.0025f, cam_dir.z);
                hit(vector(0, 0, 15), ~dir, 99, &colour); // vector(0, 0, 15) is our camera position
                acc = acc + colour * (1.f / 16);
            }
            *(ptr ++) = (unsigned char)(acc.x > 255 ? 255 : acc.x);
            *(ptr ++) = (unsigned char)(acc.y > 255 ? 255 : acc.y);
            *(ptr ++) = (unsigned char)(acc.z > 255 ? 255 : acc.z);
        }
    }

    // Save BMP
    FILE *f = fopen("o.bmp", "wb");
    fwrite(BMP_HEADER, 14, 1, f);
    fwrite(DIB_HEADER, 12, 1, f);
    fwrite(data, 786432, 1, f);
    fclose(f);
}
