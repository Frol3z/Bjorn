#include "Application.hpp"

#include <iostream>

// ENTRY POINT
int main() 
{
	Felina::Application app{ "Felina Renderer", 1280, 720 };
	try 
	{
		app.Init();
		app.Run();
	}
	catch (const std::exception& e) 
	{
		std::cerr << "[Main] An exception has been caught: " << e.what() << "\n";
		app.CleanUp();
		return EXIT_FAILURE;
	}

	app.CleanUp();
	return EXIT_SUCCESS;
}