#pragma once
#include "odin.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h> 
#include <winsock2.h>
#include <windows.h> // windows.h must be included AFTER winsock2.h
#include <string>
#include <vector>
#include <mutex>
#include "Player.h"
#include "Projectil.h"
#include "MapBorder.h"
#pragma comment (lib, "ws2_32.lib")
#pragma comment (lib, "winmm.lib")
const float32 	TURN_SPEED = 1.0f;	// how fast player turns
const float32 	ACCELERATION = 20.0f;
const float32 	PLAYER_SPEED = 250.0f;
const float32	PROJECTIL_SPEED = 500.0f;
const uint32	TICKS_PER_SECOND = 60;
const float32	SECONDS_PER_TICK = 1.0f / float32(TICKS_PER_SECOND);
const uint16	MAX_CLIENTS = 4;
const float32	CLIENT_TIMEOUT = 5.0f;
const uint16	MAX_AMMO = 4;
const uint16	MAX_PROJECTILES = MAX_AMMO + MAX_AMMO/2;
const float32	MAP_HEIGHT = 680;
const float32	MAP_WIDTH = 960;
const uint8		MAP_BORDERS = 4;
const float32	RESPAWN_TIME = 3;
const float32	RELOAD_TIME = 2;
const float32	RECOIL_TIME = 0.5;
const float32	EMPTY_SERVER_TIMEOUT = 10.0f;
const float32	NEXT_RESPAWN = 1.0;
static float32 time_since_spawn = 0.0;

enum class Client_Message : uint8
{
	Join,		// tell server we're new here
	Leave,		// tell server we're leaving
	Input 		// tell server our user input
};

enum class Server_Message : uint8
{
	Join_Result,// tell client they're accepted/rejected
	State 		// tell client game state
};

enum class Game_Object_Type : uint8
{
	Player,
	Projectil
};

enum class Player_Input_Action : uint8
{
	NONE,
	DOWN = 1,
	UP = 2,
	RIGHT = 4,
	LEFT = 8,
	FIRE = 16
};

struct IP_Endpoint
{
	uint32 address;
	uint16 port;
};

struct Player_State
{
	float32 x, y, facing, speed;
};

struct Player_Input
{
	bool32 up, down, left, right;
};

static float32 time_since(LARGE_INTEGER t, LARGE_INTEGER frequency)
{
	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);

	return float32(now.QuadPart - t.QuadPart) / float32(frequency.QuadPart);
}

class Server
{
public:
	Server();
	void Init();
	void Run();
	void Listen();
	//bool Receive();
	bool AddNewClient(IP_Endpoint& from_endpoint, SOCKADDR_IN& from);
	bool RemoveClient(IP_Endpoint& from_endpoint, SOCKADDR_IN& from);
	bool SendGameStateToAll();
	void FillBufferWithGameState();
	void ParseBuffer();
	void HandlePlayerInput();
	void UpdateGame();
	void RespawnPlayer(uint16 playerSlot);
	// void HandleState(int8* buffer, int32 bytes_read);

	virtual ~Server();

private:
	void HandlePlayerUpdate(uint16 playerSlot);
	void HandlePlayerProjectilesUpdate(uint16 playerSlot);
	void CheckHits(float deltaTime);

	bool traceLine(const Vector & pociatocnyVektor, const Vector & koncovyVektor, Vector& vecIntersection, Player* firedBy, Hittable*& zasiahnuty);
	bool lineAABBIntersection(const AABB& aabbBox, const Vector& v0, const Vector& v1, Vector& vecIntersection, float& flFraction);
	bool clipLine(int dimension, const AABB& aabbBox, const Vector&v0, const Vector&v1, float &f_low, float& f_high);

private:
	SOCKET sock;
	SOCKADDR_IN local_address;
	UINT sleep_granularity_ms;
	bool32 sleep_granularity_was_set;
	LARGE_INTEGER clock_frequency;
	int8 buffer[SOCKET_BUFFER_SIZE];
	int8 listenBuffer[SOCKET_BUFFER_SIZE];
	IP_Endpoint client_endpoints[MAX_CLIENTS];
	float32 time_since_heard_from_clients[MAX_CLIENTS];
	Player player_objects[MAX_CLIENTS];
	Projectil projectil_objects[MAX_CLIENTS * MAX_PROJECTILES];
	Player_Input client_inputs[MAX_CLIENTS];
	std::vector<Hittable*> hittable_objects;
	MapBorder map_border[4];
	bool is_running = true;
	bool has_player = false;
	float32 time_without_players;
	std::mutex send_buf_mtx;
};

