#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <thread>
#include "Server.h"


void main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	Server srv;
	srv.Init();
	srv.LoadMap(0);
	std::thread listenThread(&Server::Listen, &srv);
	srv.Run();
	listenThread.join();
}