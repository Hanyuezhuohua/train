#include "backend/interpreter1.hpp"
using namespace sjtu;
bool cleaned = false;
int main(){
	std::ios::sync_with_stdio(false);
    std::cin.tie(0);
    std::cout.tie(0);
	Interpreter I;
	I();
	return 0; 
}
