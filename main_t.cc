#include <cstring>
#include <thread>

#include "tinyraytracer.hh"

/*
#define WIDTH 1024
#define HEIGHT 768
*/
#define WIDTH 512
#define HEIGHT 384

typedef{
    float angle_v;
    float angle_h;
    float angle_logo;
}Param;


int main(int argc, char *argv[])
{
    bool gui = false, animate = false;

    if (argc > 1)
        for (int i = 1; i < argc; i++)
        {
            bool full = (!strcmp(argv[i], "-full"));
            gui |= full | (!strcmp(argv[i], "-gui"));
            animate |= full | (!strcmp(argv[i], "-animate"));
        }

    sf::Image background;
    if (!background.loadFromFile("envmap.jpg"))
    {
        std::cerr << "Error: can not load the environment map" << std::endl;
        return -1;
    }

    sf::Image logo;
    if (!logo.loadFromFile("logo.png"))
    {
        std::cerr << "Error: can not load logo" << std::endl;
        return -1;
    }

    Tinyraytracer tinyraytracer(WIDTH, HEIGHT, background, logo, Vec3f(-4, 2, -10));

    Material ivory(1.0, Vec4f(0.6, 0.3, 0.1, 0.0), Vec3f(0.4, 0.4, 0.3), 50.);
    //  Material      glass(1.5, Vec4f(0.0,  0.5, 0.1, 0.8), Vec3f(0.6, 0.7, 0.8),  125.);
    Material red_rubber(1.0, Vec4f(0.9, 0.1, 0.0, 0.0), Vec3f(0.3, 0.1, 0.1), 10.);
    Material mirror(1.0, Vec4f(0.0, 10.0, 0.8, 0.0), Vec3f(1.0, 1.0, 1.0), 1425.);

    tinyraytracer.add_sphere(Sphere(Vec3f(-3, 0, -16), 2, ivory));
    //  tinyraytracer.add_sphere(Sphere(Vec3f(-1.0, -1.5, -12), 2,      glass));
    tinyraytracer.add_sphere(Sphere(Vec3f(1.5, -0.5, -18), 3, red_rubber));
    tinyraytracer.add_sphere(Sphere(Vec3f(7, 5, -18), 4, mirror));

    tinyraytracer.add_light(Light(Vec3f(-20, 20, 20), 1.5));
    tinyraytracer.add_light(Light(Vec3f(30, 50, -25), 1.8));
    tinyraytracer.add_light(Light(Vec3f(30, 20, 30), 1.7));

    if (gui)
    {
        sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "TinyRT");
        sf::Image result;
        sf::Texture texture;
        sf::Sprite sprite;
        float angle_h = 0., angle_v = 0.;
        float angle_logo = 15.;
        sf::Clock clock;
        clock.restart();

        window.setFramerateLimit(150);
        window.clear();
        window.display();

        sf::Image img = tinyraytracer.render(angle_v, angle_h, angle_logo);
        texture.loadFromImage(img);
        sprite.setTexture(texture);
        window.draw(sprite);
        window.display();

        int nb_threads = thread::hardware_concurrency();
        std::thread* images = malloc((nb_threads-1) * sizeof(std::thread));
        std::thread master (); // TODO
    }
}

void master(Tinyraytracer* tiny){
    
}