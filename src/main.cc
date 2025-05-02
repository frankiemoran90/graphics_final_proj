//==============================================================================================
// Originally written in 2016 by Peter Shirley <ptrshrl@gmail.com>
//
// To the extent possible under law, the author(s) have dedicated all copyright and related and
// neighboring rights to this software to the public domain worldwide. This software is
// distributed without any warranty.
//
// You should have received a copy (see file COPYING.txt) of the CC0 Public Domain Dedication
// along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
//==============================================================================================

#include "rtweekend.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstdio>
#include <cstring>
#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "material.h"
#include "texture.h"
#include "sphere.h"
#include "cone.h"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <webp/encode.h>
#include <chrono>

using namespace std;

/* Constants */
const double r_earth =  6371;
const double r_moon =  1737.4;
const double r_sun = 696340;
const double earth_2_moon = 384400;
const double earth_2_sun = 1496000000;
const double sqrt_3 = 1.732;

string read_config(ifstream &conf, const string &key, string default_value) {
    string line;
    getline(conf, line);
    stringstream ss(line);
    string confkey, value;
    ss >> confkey >> value;
    if (key != confkey)
        cerr << "Key " << key << " not found in config file\n";
    return (key == confkey) ? value : default_value;
}

int read_config(ifstream &conf, const string &key, int default_value) {
    string line;
    getline(conf, line);
    stringstream ss(line);
    string confkey, value;
    ss >> confkey >> value;
    if (key != confkey)
        cerr << "Key " << key << " not found in config file\n";
    return (key == confkey) ? stoi(value) : default_value;
}

double read_config(ifstream &conf, const string &key, double default_value) {
    string line;
    getline(conf, line);
    stringstream ss(line);
    string confkey, value;
    ss >> confkey >> value;
    if (key != confkey)
        cerr << "Key " << key << " not found in config file\n";
    return (key == confkey) ? stod(value) : default_value;
}

vec3 read_config(ifstream &conf, const string &key, vec3 default_value) {
    string line;
    getline(conf, line);
    stringstream ss(line);
    string confkey;
    double x, y, z;
    ss >> confkey;
    if (key != confkey) {
        cerr << "Key " << key << " not found in config file\n";
        return default_value;
    }
    if (ss >> x >> y >> z)
        return vec3(x, y, z);
    cerr << "Could not read 3 values for " << key << "\n";
    return default_value;
}

//-----------------------------------------------------------------------------
// moon_picture -- static picture of moon
//-----------------------------------------------------------------------------
void moon_picture(hittable_list &world, point3 moon_pos) {

    /* Earth */
    auto checker = make_shared<checker_texture>(0.32, color(.2, .3, .1), color(.9, .9, .9));
    auto ground_material = make_shared<lambertian>(checker);
    world.add(make_shared<sphere>(point3(0,-1000,0), 1000, ground_material));

    /* Moon */
    auto moon_material = make_shared<metal>(color(1, 1, 1) * 10000.0, 0.0);
    auto moonlight = make_shared<diffuse_light>(color(170, 190, 255));
    world.add(make_shared<sphere>(moon_pos, 1.0, moonlight));

    /* Ground Objects */
    auto checker2 = make_shared<checker_texture>(0.32, color(.6, .0, .0), color(.9, .9, .9));
    auto object_material = make_shared<lambertian>(checker2);

    world.add(make_shared<sphere>(point3(1, 1, 1), 1.0, object_material));

    
    /* Make the sun */
    auto difflight = make_shared<diffuse_light>(color(1, 1, 1));
    //world.add(make_shared<sphere>(point3(600, -200, 600), 400.0, difflight));

    // auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    // world.add(make_shared<sphere>(point3(3, 1.5, 2.5), 1.5, material3));

    auto red = color(0.65, 0.05, 0.05);

    auto albedo = red * red;
    auto sphere_material = make_shared<lambertian>(albedo);
    world.add(make_shared<sphere>(point3(-2, 0, 1.0), 1.2, sphere_material));

    cout << "Built world: " << world.objects.size() << " objects\n";
}

//-----------------------------------------------------------------------------
// moon_phases -- video from perspective of earth showing moon phases
//-----------------------------------------------------------------------------
void moon_phases(hittable_list &world, point3 moon_pos) {
    auto checker = make_shared<checker_texture>(0.32, color(.8, .0, .1), color(0, .9, 0));
   // auto ground_material = make_shared<lambertian>(checker);
    auto ground_material = make_shared<metal>(color(1, 0, 0) * 1.0, 20.0);

    //world.add(make_shared<sphere>(point3(0,0,0), 50, ground_material));

    auto moon_texture = make_shared<image_texture>("moonmap.jpg");
    //auto moon_material = make_shared<lambertian>(moon_texture);

    auto moon_material = make_shared<metal>(color(1, 1, 1) * 1.0, 0.0);
    world.add(make_shared<sphere>(moon_pos, 50, moon_material));
    //world.add(make_shared<sphere>(point3(1, 1, 1), 1.0, ground_material));


    auto sunlight = make_shared<diffuse_light>(color(1, 1, 1));
    world.add(make_shared<sphere>(point3(-10000, 0, 0), 2500, sunlight));

    // auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    // world.add(make_shared<sphere>(point3(3, 1.5, 2.5), 1.5, material3));

    auto red = color(0.65, 0.05, 0.05);

    auto albedo = red * red;
    auto sphere_material = make_shared<lambertian>(albedo);
    //world.add(make_shared<sphere>(point3(4.5, 0.3, 1.0), 0.6, sphere_material));

    cout << "Built world: " << world.objects.size() << " objects\n";
}

void earth(hittable_list &world) {
    auto earth_texture = make_shared<image_texture>("moonmap.jpg");
    auto earth_surface = make_shared<lambertian>(earth_texture);
    world.add(make_shared<sphere>(point3(0,0,0), 2, earth_surface));

    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;

    cam.vfov     = 20;
    cam.lookfrom = point3(0,0,12);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;

    //cam.render(hittable_list(globe));
}



// Write raw RGBA frames to FFmpeg pipe
// Format: width x height, 4 bytes per pixel (RGBA)
FILE* setup_video_pipe(int width, int height, int fps, const char* filename) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd),
        "ffmpeg -y -f rawvideo -pix_fmt rgba -s %dx%d -r %d -i - "
        "-c:v libx264 -preset fast -crf 22 -pix_fmt yuv420p %s",
        width, height, fps, filename);
    return popen(cmd, "w");
}

void write_frame(FILE* pipe, const uint8_t* rgba_data, size_t frame_size) {
    fwrite(rgba_data, 1, frame_size, pipe);
    fflush(pipe);
}

void close_video_pipe(FILE* pipe) {
    pclose(pipe);
}

void write_webp(const char filename[], uint32_t* pixels, int width, int height) {
    uint8_t* output;
    size_t output_size;
    output_size = WebPEncodeRGBA((uint8_t*)pixels, width, height,
                    width * 4, 75.0f, &output);

    FILE* fp = fopen(filename, "wb");
    if (fp) {
        fwrite(output, output_size, 1, fp);
        fclose(fp);
    }

    WebPFree(output);
}


void render(const char conffile[]) {
    ifstream conf(conffile);
    string filename = read_config(conf, "filename", "test");
    uint32_t w = read_config(conf, "w", 1200);
    uint32_t h = read_config(conf, "h", 1024);
    uint32_t num_frames = read_config(conf, "num_frames", 30);
    uint32_t worldid = read_config(conf, "world", 0);
    int min_coord = read_config(conf, "min_coord", -11);
    int max_coord = read_config(conf, "max_coord", 11);
    
    // Store camera settings locally
    int samples_per_pixel = read_config(conf, "samples_per_pixel", 10);
    int max_depth = read_config(conf, "max_depth", 40);
    double vfov = read_config(conf, "vfov", 90.0);
    double defocus_angle = read_config(conf, "defocus_angle", 0.1);
    double focus_dist = read_config(conf, "focus_dist", 10.0);
    vec3 lookfrom0 = read_config(conf, "lookfrom0", vec3(-2,2,5));
    vec3 lookat0 = read_config(conf, "lookat0", vec3(0,0,0));
    vec3 vup0 = read_config(conf, "vup0", vec3(0,1,0));
    vec3 lookfrom1 = read_config(conf, "lookfrom1", vec3(13,2,3));
    vec3 lookat1 = read_config(conf, "lookat1", vec3(0,0,0));
    vec3 vup1 = read_config(conf, "vup1", vec3(0,1,0));
    vec3 bg_color = read_config(conf, "bg_color", vec3(1,1,1));

    //hittable_list world;
    // 
    camera cam;
    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width = w;
    cam.image_height = h;
    cam.samples_per_pixel = samples_per_pixel;
    cam.max_depth = max_depth;
    cam.background = bg_color;
    cam.vfov = vfov;
    cam.lookfrom = lookfrom0;
    cam.lookat = lookat0;
    cam.vup = vup0;
    cam.defocus_angle = defocus_angle;
    cam.focus_dist = focus_dist;

    cout << left
         << setw(20) << "filename:" << filename << '\n'
         << setw(20) << "w:" << w << '\n'
         << setw(20) << "h:" << h << '\n'
         << setw(20) << "world:" << worldid << '\n'
         << setw(20) << "min_coord:" << min_coord << '\n'
         << setw(20) << "max_coord:" << max_coord << '\n'
         << setw(20) << "samples_per_pixel:" << samples_per_pixel << '\n'
         << setw(20) << "max_depth:" << max_depth << '\n'
         << setw(20) << "vfov:" << vfov << '\n'
         << setw(20) << "defocus_angle:" << defocus_angle << '\n'
         << setw(20) << "focus_dist:" << focus_dist << '\n'
         << setw(20) << "lookfrom0:" << lookfrom0 << '\n'
         << setw(20) << "lookat0:" << lookat0 << '\n'
         << setw(20) << "vup0:" << vup0 << '\n'
         << setw(20) << "lookfrom1:" << lookfrom1 << '\n'
         << setw(20) << "lookat1:" << lookat1 << '\n'
         << setw(20) << "vup1:" << vup1 << '\n'
         << setw(20) << "bg_color:" << bg_color << '\n';

    const float per_frame = 1.0 / num_frames;
    const vec3 delta_lookfrom = lookfrom1 - lookfrom0;
    const vec3 delta_lookat = lookat1 - lookat0;
    const vec3 delta_vup = vup1 - vup0;

    constexpr int fps = 30;
    string video_filename = filename + ".mp4";
    FILE* pipe = num_frames > 1 ? setup_video_pipe(cam.image_width, cam.image_height, fps, video_filename.c_str()) : nullptr;
    const size_t frame_size = cam.image_width * cam.image_height * 4;
    uint32_t* rgba_buffer = new uint32_t[cam.image_width * cam.image_height];

    /* Video Loop */
    for (int frame = 0; frame < num_frames; frame++) {
        float f = frame * per_frame;
        cam.lookfrom = lookfrom0 + f * delta_lookfrom;
        cam.lookat = lookat0 + f * delta_lookat;
        cam.vup = vup0 + f * delta_vup;
        float r = 200; // raidus of moon orbit
        float phase_res = 100;
        hittable_list world;
       // point3 moon_video_pos = point3(0, 5 - (5 *f), 0);
       point3 moon_video_pos = point3(-5 + 10 * f, 5, 0);
       point3 moon_phase_pos = point3(r * cos((float)frame / phase_res), 
                                      r/10 * sin((float)frame / phase_res), 
                                      r * sin((float)frame / phase_res));
        point3 moon_phase_pos_cam = point3(r * cos((float)frame / phase_res), 
                                      0, 
                                      r * sin((float)frame / phase_res));
        /* Decide what world were gonna build */
        switch (worldid) {
            case 0: 
                moon_picture(world, point3(0, 5, 0));
                break;
            case 1:
                moon_picture(world, moon_video_pos);
                break;
            case 2:
                moon_phases(world, moon_phase_pos);
                cam.lookat = moon_phase_pos_cam;
                break;
            case 3:
                earth(world);
                break;
            default:
                cerr << "Invalid world number\n";
                return;
        }
        cam.render(world, rgba_buffer, filename, frame);
        if (frame == 0) {
            write_webp(filename.c_str(), rgba_buffer, cam.image_width, cam.image_height);
        }
        if (num_frames > 1) {
            write_frame(pipe, (uint8_t*)rgba_buffer, frame_size);
        }
        printf("\nFrame Num: %d\n", frame);
    }
    if (num_frames > 1) {
        close_video_pipe(pipe);
    }
    delete[] rgba_buffer;
}


/* Main entry point */
int main(int argc, char *argv[]) {
    if (argc < 2) {
        render("raytrace.conf");
        return 0;
    }
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 1; i < argc; i++) {
        render(argv[i]);
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::cout << "Total time: " << elapsed.count() << " seconds" << std::endl;
    return 0;
}