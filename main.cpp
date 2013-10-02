#include <cstdio> // cray_ray - a teeny tiny obfuscated ray tracer
#include <cmath>  // tom milsom | github.com/colourblind | monochromcy.net
#include <cmath>  // ground-plane, multiple spheres, reflections, fog

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

int spheres[] = {
   0x80808003,  // (0, 0, 0) r3
   0x84838202,  // (4, 3, 2) r2
   0x79857c05,  // (-7, 5, -4) r5
   0x817e8301,  // (1, -2, 3) r1
   0x84867409,  // (4, 6, -12) r9
   0x767f8003,  // (-10, -1, 0) r3
   0x7d7d8401   // (-3, -3, 4) r1
};

struct vector
{
    float x, y, z;

    vector() { x = y = z = 0; }
    vector(float a, float b, float c) { x = a, y = b, z = c; }

    vector operator +(vector a) { return vector(x + a.x, y + a.y, z + a.z); }
    vector operator *(float s) { return vector(x * s, y * s, z * s); }

    float operator ^(vector a) { return x * a.x + y * a.y + z * a.z; }
    vector operator %(vector a) { return vector(y * a.z - z * a.y, z * a.x - x * a.z, x * a.y - y * a.x); }
    vector operator ~() { return *this * (1.f / sqrtf(*this ^ *this)); }

    vector lerp(vector a, float f) { return vector(x * f + a.x * (1 - f), y * f + a.y * (1 - f), z * f + a.z * (1 - f)); }
};

vector hit(vector start, vector dir, int reflection_count)
{
    if (reflection_count == 0)
        return vector(255, 0, 0);

    vector a;
    float r = -1;

    vector sphere, new_start, new_dir;
    float radius;

    for (int i = 0; i < 7; i ++)
    {
        sphere = vector((float)(spheres[i] >> 24 & 0xff) - 128, (float)(spheres[i] >> 16 & 0xff) - 128, (float)(spheres[i] >> 8 & 0xff) - 128);
        radius = spheres[i] & 0xff;
        vector oc = (start + sphere * -1);
        float loc = dir ^ oc;
        float d = (loc * loc) - (oc ^ oc) + (radius * radius);
        float d0 = -loc + sqrtf(d);
        float d1 = -loc - sqrtf(d);

        if (d0 >= 0 || d1 >= 0) // We'e hit our sphere
        {
            d = d0 < d1 ? d0 : d1;
            if (r < 0 || d < r)
            {
                r = d;
                new_start = start + (dir * r);
                new_dir = ~(new_start + sphere * -1);
            }
        }
    }

    if (r >= 0) // Reflection
    {
        new_start = new_start + new_dir * 0.001f;
        a = hit(new_start, new_dir, reflection_count - 1) * 0.8f;
    }
    else
    {
        vector p = vector(0, -4, 0);
        vector n = vector(0, 1, 0);
        r = ((p + start * -1) ^ n) / (dir ^ n);

        if (r > 0)
        {
            vector i = start + dir * r; // Point of intersection
            int s = abs((int)ceil(i.x)) % 2 == abs((int)ceil(i.z)) % 2; // Checkerboard 'texture'
            a = vector(s ? 0 : 255, s ? 0 : 255, s ? 255 : 0);
        }
        else
        {
            a = vector(255, 0, 0); // Mr Blue Sky
            r = 20; // Otherwise we get into NaN shenanigans
        }
    }

    const float FOG_RADIUS = 24.f;
    return a.lerp(vector(255, 0, 0), ((FOG_RADIUS - (r > FOG_RADIUS ? FOG_RADIUS : r)) / FOG_RADIUS));
}

int main(int argc, char **argv)
{
    unsigned char data[512 * 512 * 3];
    unsigned char *ptr = data;
    vector cam_pos(0, 0, 9);
    vector cam_up(0, 1, 0);
    vector cam_dir(0, 0, -1);

    for (int i = 0; i < 512; i ++)
    {
        for (int j = 0; j < 512; j ++)
        {
            vector dir(cam_dir.x + (j - 256) * 0.005f, cam_dir.y + (i - 256) * 0.005f, cam_dir.z);
            vector colour = hit(cam_pos, ~dir, 99);
            *(ptr ++) = (unsigned char)colour.x;
            *(ptr ++) = (unsigned char)colour.y;
            *(ptr ++) = (unsigned char)colour.z;
        }
    }

    // Save BMP
    FILE *f = fopen("out.bmp", "wb");
    fwrite(BMP_HEADER, 14, 1, f);
    fwrite(DIB_HEADER, 12, 1, f);
    fwrite(data, 512 * 512 * 3, 1, f);
    fclose(f);

    return 0;
}
