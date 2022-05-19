#include <iostream>
#include <thread>

void foo() {
   std::cout << " foo is executing concurrently...\n";
}

void bar(int x) {
   std::cout << " bar is executing concurrently... " << x << "\n";
}

int main() {
   std::thread first (foo);
   std::thread second (bar,0);

   std::cout << "main, foo and bar now execute concurrently...\n";

   first.join();
   second.join();

   std::cout << "foo and bar completed.\n";

   return 0;
}