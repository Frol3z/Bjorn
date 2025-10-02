#include "Application.hpp"

#include <iostream>

// ENTRY POINT
int main() 
{
	Bjorn::Application app("My App", 800, 600);
	app.Run();
	std::cout << "Done." << std::endl;
}