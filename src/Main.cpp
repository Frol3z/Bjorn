#include "Application.hpp"

#include <iostream>

// ENTRY POINT
int main() 
{
	try 
	{
		Felina::Application app("My App", 1280, 720);
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