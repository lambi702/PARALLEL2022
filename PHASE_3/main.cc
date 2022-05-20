#include <iostream>
#include <cstring>
#include <thread>
#include <queue>
#include <mutex>
#include <vector>
#include "tinyraytracer.hh"
/*
#define WIDTH 1024
#define HEIGHT 768
*/
#define WIDTH 512
#define HEIGHT 384
#define Q_MAX 20

struct ImgPriority {
  sf::Image image;
  int order;

  ImgPriority(sf::Image image,int order)
    : image(image), order(order)
    {
    }
};

struct cmpPriority {
  bool operator()(ImgPriority const& i1, ImgPriority const& i2){
    return i1.order > i2.order; // Faire sortir l'élément ayant la plus petite priorité
  }
};

struct Angle
{
	float v;
	float h;
	float logo;
	unsigned long long int frameNb;
};

bool boolWindow = true;
void compute(Tinyraytracer tinyraytracer);
std::mutex mx;
std::priority_queue<ImgPriority,std::vector<ImgPriority>,cmpPriority> qImages;
std::queue<Angle> qAngles;

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
		std::vector<std::thread> vThreads;
		for (size_t i = 0; i < (std::thread::hardware_concurrency() - 1); i++)
			vThreads.push_back(std::thread(compute, tinyraytracer));
		uint64_t frameCounter = 0;
		float fps = 30.;

		sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "TinyRT");
		sf::Image result;
		sf::Texture texture;
		sf::Sprite sprite;
		float angle_h = 0., angle_v = 0., z_red = -0.5, size_mirror = 4.;
		float angle_logo = 15.;
		sf::Clock clock;
		clock.restart();

		window.setFramerateLimit(150);
		window.clear();
		window.display();

		sf::Image img = tinyraytracer.render(angle_v, angle_h, angle_logo, z_red, size_mirror);
		texture.loadFromImage(img);
		sprite.setTexture(texture);
		window.draw(sprite);
		window.display();

		while (window.isOpen())
		{
			sf::Event event;
			bool update = false, cond = false;
			if (animate)
			{
				mx.lock();
				if (qAngles.size() < Q_MAX)
				{
					angle_logo += 6. / fps;
					if (angle_logo>=360.)
						angle_logo -= 360.; 
							update = true;
				}
				mx.unlock();
			}
			if (window.pollEvent(event))
			{
				if (event.type == sf::Event::KeyPressed)
				{
					if (event.key.code == sf::Keyboard::Left)
					{
						angle_h += angle_h >= 359. ? -359. : 1.;
						update = true;
						std::cerr << "Key pressed: "
								  << "Left" << std::endl;
					}
					else if (event.key.code == sf::Keyboard::Right)
					{
						angle_h -= angle_h < 1. ? -359 : 1.;
						update = true;
						std::cerr << "Key pressed: "
								  << "Right" << std::endl;
					}
					else if (event.key.code == sf::Keyboard::Up)
					{
						angle_v += angle_v >= 359. ? -359. : 1.;
						update = true;
						std::cerr << "Key pressed: "
								  << "Up" << std::endl;
					}
					else if (event.key.code == sf::Keyboard::Down)
					{
						angle_v -= angle_v < 1. ? -359 : 1.;
						update = true;
						std::cerr << "Key pressed: "
								  << "Down" << std::endl;
					}
					else if (event.key.code == sf::Keyboard::Space)
						std::cerr << "Key pressed: "
								  << "Space" << std::endl;
					else if (event.key.code == sf::Keyboard::Q)
					{
						for (size_t i = 0; i < std::thread::hardware_concurrency() - 1; i++)
							vThreads.at(i).join();
						window.close();
						mx.lock();
						boolWindow = false;
						mx.unlock();
					}
					else
						std::cerr << "Key pressed: "
								  << "Unknown" << std::endl;
				}
				if (event.type == sf::Event::Closed)
				{
					mx.lock();
					boolWindow = false;
					mx.unlock();

					window.close();
				}
			}
			if (update)
			{
				static unsigned framecount = 0;
				Angle angle;
				angle.v = angle_v;
				angle.h = angle_h;
				angle.logo = angle_logo;
				angle.frameNb = frameCounter;

				mx.lock();
				qAngles.push(angle);
				frameCounter++;
				cond = (!qImages.empty() && qImages.size() > 10);
				mx.unlock();
				if (cond)
				{
					mx.lock();
					ImgPriority ip = qImages.top();
					qImages.pop();
					mx.unlock();

					texture.loadFromImage(ip.image);
					window.clear();
					window.draw(sprite);
					window.display();
					framecount++;
					sf::Time currentTime = clock.getElapsedTime();
					if (currentTime.asSeconds() > 1.0)
					{
						fps = framecount / currentTime.asSeconds();
						std::cout << "fps: " << fps << std::endl;
						clock.restart();
						framecount = 0;
					}
				}
			}
		}
	}
	else
	{
		sf::Image result = tinyraytracer.render(0, 0, 15);
		result.saveToFile("out.jpg");
	}
	return 0;
}

void compute(Tinyraytracer tinyraytracer)
{
	bool cond = false;
	while (boolWindow)
	{
		mx.lock();
		cond = !qAngles.empty();
		mx.unlock();
		if (cond)
		{
			mx.lock();
			Angle next = qAngles.front();
			qAngles.pop();
			mx.unlock();

			sf::Image result = tinyraytracer.render(next.v, next.h, next.logo);
			ImgPriority ip = ImgPriority(result,next.frameNb);
			mx.lock();
			qImages.push(ip);
			mx.unlock();
		}
	}
}