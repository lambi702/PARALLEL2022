#include <iostream>
#include <queue>

int main(){
  std::priority_queue<int> queue;
  queue.push(3);
  queue.push(2);
  queue.push(1);

  for(int i = 2; i >= 0; i--){
    std::cout << queue.top() << std::endl;
    queue.pop();
  }
  queue.pop();
}