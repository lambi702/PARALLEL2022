#include <iostream>
#include <queue>
#include <vector>

#include <SFML/Graphics.hpp>

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

int main(){
  std::priority_queue<ImgPriority,std::vector<ImgPriority>,cmpPriority> pQueueImg;

  pQueueImg.push(ImgPriority(sf::Image(),10));
  pQueueImg.push(ImgPriority(sf::Image(),9));
  pQueueImg.push(ImgPriority(sf::Image(),11));
  pQueueImg.push(ImgPriority(sf::Image(),8));

  for(int i = 3; i >= 0; i--){
    ImgPriority front = pQueueImg.top();
    pQueueImg.pop();
    std::cout << front.order << std::endl;
  }
}