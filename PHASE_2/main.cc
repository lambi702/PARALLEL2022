#include <cstring>
#include <queue>
#include "tinyraytracer.hh"
#include <mutex>
#include <thread>

/*
#define WIDTH 1024
#define HEIGHT 768
*/

#define WIDTH 512
#define HEIGHT 384

std::mutex mtx_img;
std::mutex mtx_angles;

typedef struct angles_struct_t
{
	float angle_v;
	float angle_h;
	float angle_l;
	unsigned int count;
} Angles;

typedef struct img_struct_t
{
	sf::Image img;
	unsigned int count;
} Img_Count;

void consumer(sf::RenderWindow *window, sf::Image **circular_buffer, Tinyraytracer *Tinyraytracer);
void producer(sf::RenderWindow *window, std::queue<Angles *> *angles_queue, sf::Image *circular_buffer, int buffer_size);
void clavier(sf::RenderWindow *window, std::queue<Angles> *angles_queue);

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
		std::queue<Angles *> angles_queue;
		sf::Image *img[20];
		img[0] = &(tinyraytracer.render(0., 0., 15.));
		std::thread clavier (clavier,&window,&angles_queue);
		std::thread producer[5];
		for (int i = 0; i<5 ;i++)
			producer[i]= std::thread (producer, &window, &angles_queue,img,20)
		std::thread consumer(&window, img, &tinyraytracer)
	}
	else
	{
		sf::Image result = tinyraytracer.render(0, 0, 15);
		result.saveToFile("out.jpg");
	}
	return 0;
}

void clavier(sf::RenderWindow *window, std::queue<Angles> *angles_queue)
{
	unsigned int i = 1;
	float angle_h = 0., angle_v = 0.;
	float angle_logo = 15.;
	while (window->isOpen())
	{
		sf::Event event;
		bool update = false;
		if (animate)
		{
			angle_logo += angle_logo >= 359. ? -359. : 1.;
			update = true;
		}
		if (window->pollEvent(event))
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
				window->close();
		}
		if (update)
		{
			Angles *angles = (Angles *)malloc(sizeof(Angles));
			angles->angle_v = angle_v;
			angles->angle_h = angle_h;
			angles->angle_l = angle_logo;
			angles->count = i;
			mtx_angles.lock();
			angles_queue.push(angles);
			mtx_angles.unlock();
			i++;
		}
	}
}

void producer(sf::RenderWindow *window, std::queue<Angles *> *angles_queue, sf::Image* circular_buffer, int buffer_size)
{
	bool empty;
	sf::Image img();
	while (window->isOpen())
	{
		mtx_angles.lock();
		empty = angles_queue->empty();
		mtx_angles.unlock();
		if (!empty)
		{
			Angles *angle = angles_queue->front();
			angles_queue->pop();
			img = tinyraytracer.render(angle->angle_v, angle->angle_h, angle->angle_l);
			while (true)
			{
				mtx_img.lock();
				empty = (circular_buffer[angle->count % buffer_size] == NULL);
				mtx_img.unlock();
				if (empty)
				{
					mtx_img.lock();
					circular_buffer[angle->count % buffer_size] = img;
					mtx_img.unlock();
					break;
				}
			}
		}
	}
}

void consumer(sf::RenderWindow *window, sf::Image** circular_buffer, Tinyraytracer *Tinyraytracer)
{
	int i = 0;
	bool test = false;
	sf::Texture texture;
	sf::Sprite sprite;

	window->setFramerateLimit(150);
	while (window->isOpen())
	{
		mtx_img.lock();
		test = img[i]!=NULL;
		mtx_img.unlock();
		if (test)
		{
			mtx_img.lock();
			texture.loadFromImage(*img[i]);
			sprite.setTexture(texture);
			window->draw(sprite);
			window->display();
			mtx_img.unlock();
			i++ ;
			window->clear();
			window->draw(sprite);
			window->display();
		}
	}
}