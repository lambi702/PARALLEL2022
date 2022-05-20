#include <cstring>
#include <mutex>
#include <thread>
#include <queue>
#include "tinyraytracer.hh"

#define WIDTH 512
#define HEIGHT 384
#define BUFFER_SIZE 20

class Angle
{
	float v, h, logo;
	uint frameNbr;

public:
	Angle(float a_v, float a_h, float a_logo, uint frame_Nb);
};

Angle::Angle(float a_v, float a_h, float a_logo, uint frame_Nb)
{
	v = a_v;
	h = a_h;
	logo = a_logo;
	frameNbr = frame_Nb;
}

void master(Tinyraytracer *tinyraytracer, std::queue<Angle> *qAngles, sf::Image *images);
void computeImage(Tinyraytracer *tinyraytracer, std::queue<Angle> *qAngles, sf::Image *images);

std::mutex mtImages;
std::mutex mtAngles;
bool gui = false, animate = false;
int main(int argc, char *argv[])
{


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
		static std::queue<Angle> qAngles;
		static sf::Image *cbImages = (sf::Image*) malloc(sizeof(sf::Image) * BUFFER_SIZE);
		std::thread ttt = std::thread(master,&tinyraytracer, &qAngles, cbImages);
		ttt.join();
		std::thread threads[15];
		for (int k = 0; k < 15; k++)
		{
			threads[k] = std::thread(computeImage, &tinyraytracer, &qAngles, cbImages);
			threads[k].join();
		}
	}
	else
	{
		sf::Image result = tinyraytracer.render(0, 0, 15);
		result.saveToFile("out.jpg");
	}
	return 0;
}

void master(Tinyraytracer *tinyraytracer, std::queue<Angle> *qAngles, sf::Image *images)
{
	unsigned int i = 0;
	unsigned int id = 0;
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

	sf::Image img = tinyraytracer->render(angle_v, angle_h, angle_logo);
	texture.loadFromImage(img);
	sprite.setTexture(texture);
	window.draw(sprite);
	window.display();
	while (window.isOpen())
	{
		sf::Event event;
		bool update = false;
		if (animate)
		{
			angle_logo += angle_logo >= 359. ? -359. : 1.;
			update = true;
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
					window.close();
				else
					std::cerr << "Key pressed: "
							  << "Unknown" << std::endl;
			}
			if (event.type == sf::Event::Closed)
				window.close();
		}
		if (update)
		{
			Angle angles = Angle(angle_v, angle_h, angle_logo,id);
			qAngles->push(angles);
			mtImages.lock();
			bool test = false;
			if (images[id % BUFFER_SIZE] == sf::Image ())
				test = true;
			mtImages.unlock();
			if (test)
			{
				mtImages.lock();
				texture.loadFromImage(images[id % BUFFER_SIZE]);
				images[id % BUFFER_SIZE] = sf::Image ();
				mtImages.unlock();
				window.clear();
				window.draw(sprite);
				window.display();
				id++;
			}

			// framecount++;
			// sf::Time currentTime = clock.getElapsedTime();
			// if (currentTime.asSeconds() > 1.0)
			// {
			// 	float fps = framecount / currentTime.asSeconds();
			// 	std::cout << "fps: " << fps << std::endl;
			// 	clock.restart();
			// 	framecount = 0;
			// }
		}
	}
}

void computeImage(Tinyraytracer *tinyraytracer, std::queue<Angle> *qAngles, sf::Image *images)
{
	while (true)
	{
		bool boucle = true;
		mtAngles.lock();
		bool test = qAngles->empty();
		mtAngles.unlock();
		sf::Image = tinyraytracer->render(angles.v, angles.h, angles.logo);
		while (boucle && !test)
		{
			mtAngles.lock();
			Angle angles = qAngles->front();
			qAngles.pop();
			mtAngles.unlock();

			mtImages.lock();
			test = (images[angles.frameNbr % BUFFER_SIZE] == NULL);
			mtImages.unlock();
			if (test)
			{
				mtImages.lock();
				img[angles.frameNbr % sizeImages] = tinyraytracer->render(angles.v, angles.h, angles.logo);
				mtImages.unlock();
				boucle = false;
			}
		}
	}
}
