#include "Application.hpp"

#include <iostream>

// ENTRY POINT
int main() 
{
	try 
	{
		Bjorn::Application app("My App", 800, 600);
		app.Run();
	}
	catch (const std::exception& e) 
	{
		std::cerr << "Incident happened: " << e.what() << "\n";
		return EXIT_FAILURE;
	}

	std::cout << "Fin." << std::endl;
	return EXIT_SUCCESS;
}