#define NOMINMAX
#include "Server.h"
#include <algorithm>

bool operator==(const IP_Endpoint& a, const IP_Endpoint& b) { return a.address == b.address && a.port == b.port; }

Server::Server()
{
}

void Server::Init()
{
	WORD winsock_version = 0x202;
	WSADATA winsock_data;
	if (WSAStartup(winsock_version, &winsock_data))
	{
		printf("WSAStartup failed: %d\n", WSAGetLastError());
		return;
	}
	int address_family = AF_INET;
	int type = SOCK_DGRAM;
	int protocol = IPPROTO_UDP;
	sock = socket(address_family, type, protocol);

	if (sock == INVALID_SOCKET)
	{
		printf("socket failed: %d\n", WSAGetLastError());
		return;
	}

	local_address.sin_family = AF_INET;
	local_address.sin_port = htons(PORT);
	local_address.sin_addr.s_addr = INADDR_ANY;
	if (bind(sock, (SOCKADDR*)&local_address, sizeof(local_address)) == SOCKET_ERROR)
	{
		printf("bind failed: %d\n", WSAGetLastError());
		return;
	}
	u_long enabled = 1;
	ioctlsocket(sock, FIONBIO, &enabled);

	sleep_granularity_ms = 1;
	sleep_granularity_was_set = timeBeginPeriod(sleep_granularity_ms) == TIMERR_NOERROR;
	QueryPerformanceFrequency(&clock_frequency);

	for (uint16 player_i = 0; player_i < MAX_CLIENTS; ++player_i)
	{
		hittable_objects.push_back(&player_objects[player_i]);
		player_objects[player_i].setPlayerSlot(player_i);
		player_objects[player_i].setPlayerStatus(Player::Status::DEATH);

		client_endpoints[player_i] = {};
	}
	printf("Server started\n");

	for (uint16 i = 0; i < MAX_PROJECTILES * MAX_CLIENTS; ++i)
	{
		projectil_objects[i].setOwnerSlot(i / (MAX_PROJECTILES));
		projectil_objects[i].setProjectilNumber(i % MAX_PROJECTILES);
		projectil_objects[i].setProjectilStatus(Projectil::Projectil_Status::DISABLED);
	}
	map_border[0].getBody().setPosition(-MAP_WIDTH / 2.0f, 0.0f);
	map_border[0].getBody().setSize(0.0, MAP_HEIGHT);
	map_border[1].getBody().setPosition(0.0f, MAP_HEIGHT / 2.0f);
	map_border[1].getBody().setSize(MAP_WIDTH, 0.0f);
	map_border[2].getBody().setPosition(MAP_WIDTH / 2.0f, 0.0f);
	map_border[2].getBody().setSize(0.0, MAP_HEIGHT);
	map_border[3].getBody().setPosition(0.0f, -MAP_HEIGHT / 2.0f);
	map_border[3].getBody().setSize(MAP_WIDTH, 0.0f);

	time_without_players = 0.0f;
	Wall* tmpWall = new Wall(Vector(100, 70), Vector(150, 165), "strom");

	this->other_collision_objects.push_back(tmpWall);
	this->hittable_objects.push_back(tmpWall);
}

void Server::Run()
{

	bool32 updated = false;
	std::string input;
	int timeoutCounter = 0;
	while (is_running)
	{
		has_player = false;
		updated = false;
		LARGE_INTEGER tick_start_time;
		QueryPerformanceCounter(&tick_start_time);
		HandleCurrentInputs();
		UpdateGame();
		SendGameStateToAll();

		float32 time_taken_s = time_since(tick_start_time, clock_frequency);
		while (time_taken_s < SECONDS_PER_TICK)
		{
			if (sleep_granularity_was_set)
			{
				DWORD time_to_wait_ms = DWORD((SECONDS_PER_TICK - time_taken_s) * 1000);
				if (time_to_wait_ms > 0)
				{
					Sleep(time_to_wait_ms);
				}
			}

			time_taken_s = time_since(tick_start_time, clock_frequency);
		}
		time_since_spawn += SECONDS_PER_TICK;
		if (has_player)
		{
			time_without_players = 0.0f;
		}
		else
		{
			if (time_without_players >= EMPTY_SERVER_TIMEOUT)
			{
				is_running = false;
			}
			time_without_players += SECONDS_PER_TICK;
			if (timeoutCounter != (int)(EMPTY_SERVER_TIMEOUT - time_without_players))
			{
				timeoutCounter = (int)(EMPTY_SERVER_TIMEOUT - time_without_players);
				printf("Server shutdown in %d\n", timeoutCounter);
			}

		}
	}
}

void Server::Listen()
{
	while (is_running)
	{
		int flags = 0;
		ZeroMemory(&listenBuffer, SOCKET_BUFFER_SIZE);
		SOCKADDR_IN from;
		int from_size = sizeof(from);
		while (true)
		{
			int bytes_received = recvfrom(sock, listenBuffer, SOCKET_BUFFER_SIZE, flags, (SOCKADDR*)&from, &from_size);
			if (bytes_received == SOCKET_ERROR)
			{
				int error = WSAGetLastError();
				if (error != WSAEWOULDBLOCK)
				{
					printf("recvfrom returned SOCKET_ERROR, WSAGetLastError() %d\n", error);
				}

				break;
			}

			IP_Endpoint from_endpoint;
			from_endpoint.address = from.sin_addr.S_un.S_addr;
			from_endpoint.port = from.sin_port;

			switch ((uint8)listenBuffer[0])
			{
			case (uint8)Client_Message::Join:
				AddNewClient(from_endpoint, from);
				break;
			case (uint8)Client_Message::Leave:
				RemoveClient(from_endpoint, from);
				break;
			case (uint8)Client_Message::Input:
				HandlePlayerInput();
				break;
			default:
				break;
			}
		}
	}
}

bool Server::AddNewClient(IP_Endpoint& from_endpoint, SOCKADDR_IN& from)
{
	printf("Client_Message::Join from %d.%d.%d.%d:%d\n", from.sin_addr.S_un.S_un_b.s_b1,
		from.sin_addr.S_un.S_un_b.s_b2,
		from.sin_addr.S_un.S_un_b.s_b3,
		from.sin_addr.S_un.S_un_b.s_b4,
		from.sin_port);
	int flags = 0;
	int from_size = sizeof(from);
	uint16 slot = uint16(-1);
	for (uint16 i = 0; i < MAX_CLIENTS; ++i)
	{
		if (client_endpoints[i].address == 0)
		{
			slot = i;
			break;
		}
	}
	uint16 bytesRead = 1;
	uint16 nameLength;
	memcpy(&nameLength, &listenBuffer[bytesRead], sizeof(nameLength));
	bytesRead += sizeof(nameLength);
	std::string playerName(&listenBuffer[bytesRead], nameLength);
	bytesRead += nameLength * sizeof(char);

	this->player_objects[slot].setPlayerName(playerName);
	send_buf_mtx.lock();
	ZeroMemory(&buffer, SOCKET_BUFFER_SIZE);
	buffer[0] = (int8)Server_Message::Join_Result;
	if (slot != uint16(-1))
	{
		printf("client will be assigned to slot %hu\n", slot);
		buffer[1] = 1;
		uint16 bytesWritten = 2;
		memcpy(&buffer[bytesWritten], &slot, sizeof(slot));
		bytesWritten += sizeof(slot);
		memcpy(&buffer[bytesWritten], &this->mapNumber, sizeof(this->mapNumber));
		bytesWritten += sizeof(this->mapNumber);
		flags = 0;
		if (sendto(sock, buffer, bytesWritten + 1, flags, (SOCKADDR*)&from, from_size) != SOCKET_ERROR)
		{
			send_buf_mtx.unlock();
			client_endpoints[slot] = from_endpoint;
			time_since_heard_from_clients[slot] = 0.0f;
			client_inputs[slot] = {};
			this->player_objects[slot].setDeaths(0);
			this->player_objects[slot].setKills(0);
			this->RespawnPlayer(slot);
		}
		else
		{
			send_buf_mtx.unlock();
			printf("sendto failed: %d\n", WSAGetLastError());
			return false;
		}
	}
	else
	{
		printf("could not find a slot for player\n");
		buffer[1] = 0;

		flags = 0;
		if (sendto(sock, buffer, 2, flags, (SOCKADDR*)&from, from_size) == SOCKET_ERROR)
		{
			printf("sendto failed: %d\n", WSAGetLastError());
		}
		send_buf_mtx.unlock();
		return false;
	}
	return true;
}

bool Server::RemoveClient(IP_Endpoint& from_endpoint, SOCKADDR_IN& from)
{
	int flags = 0;
	SOCKADDR_IN to;
	to.sin_family = AF_INET;
	to.sin_port = htons(PORT);
	int to_length = sizeof(to);
	uint16 slot;
	memcpy(&slot, &listenBuffer[1], sizeof(slot));
	if (client_endpoints[slot] == from_endpoint)
	{
		to.sin_addr.S_un.S_addr = client_endpoints[slot].address;
		to.sin_port = client_endpoints[slot].port;

		send_buf_mtx.lock();
		this->FillBufferWithPlayersStats();
		if (sendto(sock, buffer, SOCKET_BUFFER_SIZE, flags, (SOCKADDR*)&to, to_length) == SOCKET_ERROR)
		{
			printf("sendto failed: %d\n", WSAGetLastError());
		}
		send_buf_mtx.unlock();

		client_endpoints[slot] = {};
		printf("Client_Message::Disconnect from slot %d Address %d.%d.%d.%d:%d\n", slot, from.sin_addr.S_un.S_un_b.s_b1,
			from.sin_addr.S_un.S_un_b.s_b2,
			from.sin_addr.S_un.S_un_b.s_b3,
			from.sin_addr.S_un.S_un_b.s_b4,
			from.sin_port);
		this->player_objects[slot].setPlayerStatus(Player::Status::DEATH);
		this->player_objects[slot].setPlayerName("Player" + std::to_string(slot));
	}
	return true;
}

bool Server::SendGameStateToAll()
{
	int flags = 0;
	SOCKADDR_IN to;
	to.sin_family = AF_INET;
	to.sin_port = htons(PORT);
	int to_length = sizeof(to);

	send_buf_mtx.lock();
	ZeroMemory(buffer, SOCKET_BUFFER_SIZE);
	FillBufferWithGameState();
	for (uint16 i = 0; i < MAX_CLIENTS; ++i)
	{
		if (client_endpoints[i].address)
		{
			to.sin_addr.S_un.S_addr = client_endpoints[i].address;
			to.sin_port = client_endpoints[i].port;

			if (sendto(sock, buffer, SOCKET_BUFFER_SIZE, flags, (SOCKADDR*)&to, to_length) == SOCKET_ERROR)
			{
				printf("sendto failed: %d\n", WSAGetLastError());
			}
		}
	}
	send_buf_mtx.unlock();
	return true;
}

void Server::FillBufferWithGameState()
{
	ZeroMemory(buffer, SOCKET_BUFFER_SIZE);
	buffer[0] = (uint8)Server_Message::State;
	int32 bytes_written = 1;
	uint16 numberOfObjects = 0;
	bytes_written += sizeof(numberOfObjects);
	// Loop - all object states
	for (uint16 i = 0; i < MAX_CLIENTS; ++i)
	{
		if (client_endpoints[i].address)
		{
			buffer[bytes_written++] = (uint8)Game_Object_Type::Player;
			memcpy(&buffer[bytes_written], &i, sizeof(i));
			bytes_written += sizeof(i);
			player_objects[i].UploadState(buffer, bytes_written);
			++numberOfObjects;
			for (uint16 projectil_it = 0; projectil_it < MAX_PROJECTILES; ++projectil_it)
			{
				if (projectil_objects[i * MAX_PROJECTILES + projectil_it].getProjectilStatus() == Projectil::Projectil_Status::ACTIVE)
				{
					buffer[bytes_written++] = (uint8)Game_Object_Type::Projectil;
					projectil_objects[i * MAX_PROJECTILES + projectil_it].UploadState(buffer, bytes_written);
					++numberOfObjects;
				}
				else
					continue;
			}
		}
	}
	memcpy(&buffer[1], &numberOfObjects, sizeof(numberOfObjects));
}

void Server::FillBufferWithPlayersStats()
{
	int32 bytesWritten = 0;
	uint16 numberOfPlayers = 0;
	ZeroMemory(buffer, SOCKET_BUFFER_SIZE);
	buffer[bytesWritten++] = (uint8)Server_Message::GameStats;
	bytesWritten += sizeof(numberOfPlayers);
	for (uint16 i = 0; i < MAX_CLIENTS; ++i)
	{
		if (client_endpoints[i].address)
		{
			player_objects[i].UploadPlayerStats(buffer, bytesWritten);
			++numberOfPlayers;
		}
	}
	memcpy(&buffer[1], &numberOfPlayers, sizeof(numberOfPlayers));
}

void Server::ParseBuffer()
{
	Server_Message type_of_message = (Server_Message)buffer[0];
	int32 bytes_read = 1;
	switch (type_of_message)
	{
	case Server_Message::State:
		//HandleState(buffer, bytes_read);
		break;
	default:
		break;
	}
}

void Server::HandlePlayerInput()
{
	uint16 playerSlot = -1;
	uint8 playerActions = 0;
	int bytesRead = 1;
	memcpy(&playerSlot, &listenBuffer[bytesRead], sizeof(playerSlot));
	bytesRead += sizeof(playerSlot);
	playerActions = listenBuffer[bytesRead++];
	time_since_heard_from_clients[playerSlot] = 0.0f;
	bool fireRequest = false;
	input_mutex.lock();
	this->ResetClientInput(playerSlot);
	Player_Input& playerIn = client_inputs[playerSlot];
	if (playerActions & 0x10)
	{
		playerActions &= ~(1 << 4);
		playerIn.fire = true;
	}
	switch ((Player_Input_Action)playerActions)
	{
	case Player_Input_Action::UP:
		playerIn.up = true;
		break;
	case Player_Input_Action::DOWN:
		playerIn.down = true;
		break;
	case Player_Input_Action::LEFT:
		playerIn.left = true;
		break;
	case Player_Input_Action::RIGHT:
		playerIn.right = true;
		break;
	default:
		break;
	}
	input_mutex.unlock();
}

void Server::UpdateGame()
{
	for (uint16 i = 0; i < MAX_CLIENTS; ++i)
	{
		if (client_endpoints[i].address)
		{
			has_player = true;
			time_since_heard_from_clients[i] += SECONDS_PER_TICK;
			if (time_since_heard_from_clients[i] > CLIENT_TIMEOUT)
			{
				printf("client %hu timed out\n", i);
				client_endpoints[i] = {};
				player_objects[i].setPlayerStatus(Player::Status::DEATH);
				this->player_objects[i].setPlayerName("Player" + std::to_string(i));
			}
			else
			{
				HandlePlayerUpdate(i);
				CheckHits(TICKS_PER_SECOND);
				HandlePlayerProjectilesUpdate(i);
			}
		}
	}

}
void Server::RespawnPlayer(uint16 playerSlot)
{
	float inerval_down_X = 0;
	float inerval_up_X = 0;
	float inerval_down_Y = 0;
	float inerval_up_Y = 0;
	Player& tmpPlayer = player_objects[playerSlot];
	Vector tmpPlayerSize = tmpPlayer.getBody().getSize();
	int possibleWidth = (int)MAP_WIDTH - tmpPlayerSize.x;
	int possibleHeight = (int)MAP_HEIGHT - tmpPlayerSize.y;;
	float randX;
	float randY;
	bool colide = true;
	while (colide)
	{
		colide = false;
		randX = rand() % possibleWidth - possibleWidth / 2.0f;
		randY = rand() % possibleHeight - possibleHeight / 2.0f;
		tmpPlayer.getBody().setPosition(randX, randY);
		for (uint16 i = 0; i < MAX_CLIENTS; ++i)
		{
			if (!client_endpoints[i].address)
				continue;
			if (&tmpPlayer == &player_objects[i])
				continue;
			if (tmpPlayer.CheckCollision(player_objects[i]))
			{
				colide = true;
				break;
			}
		}
		if (!colide)
		{
			for (uint16 object_i = 0; object_i < other_collision_objects.size(); ++object_i)
			{
				if (tmpPlayer.CheckCollision(*other_collision_objects[object_i]))
				{
					colide = true;
					break;
				}
			}
		}
	}
	tmpPlayer.Spawn();
}

void Server::LoadMap(uint16 mapNumber)
{
	for (uint16 i = 0; i < other_collision_objects.size(); ++i)
	{
		delete other_collision_objects[i];
		other_collision_objects[i] = nullptr;
	}
	other_collision_objects.clear();
	hittable_objects.clear();

	for (uint16 player_i = 0; player_i < MAX_CLIENTS; ++player_i)
	{
		hittable_objects.push_back(&player_objects[player_i]);
	}


	Wall* tmpObject = nullptr;
	std::string mapPath = "map" + std::to_string(mapNumber) + ".txt";
	std::ifstream mapfile(mapPath);

	int objType;
	float width = 0;
	float heigth = 0;
	float cordX = 0;
	float cordY = 0;
	std::string objName;
	while (mapfile >> objType)
	{
		switch ((Map_Object_Type)objType)
		{
		case Map_Object_Type::TREE:
			mapfile >> width;
			mapfile >> heigth;
			mapfile >> cordX;
			mapfile >> cordY;
			mapfile >> objName;
			tmpObject = new Wall(Vector(width, heigth), Vector(cordX, cordY), objName);
			other_collision_objects.push_back(tmpObject);
			hittable_objects.push_back(tmpObject);
		default:
			break;

		}
	}
	other_collision_objects.shrink_to_fit();
	hittable_objects.shrink_to_fit();
	mapfile.close();
}

void Server::HandleCurrentInputs()
{
	bool playerUp = false;
	bool playerDown = false;
	bool playerLeft = false;
	bool playerRight = false;
	bool playerFire = false;
	for (uint16 player_i = 0; player_i < MAX_CLIENTS; ++player_i)
	{
		if (!client_endpoints[player_i].address)
			continue;
		Player& tmpPlayer = player_objects[player_i];
		if (tmpPlayer.getPlayerStatus() == Player::Status::ALIVE)
		{
			input_mutex.lock();
			Player_Input& tmpPlayerIn = client_inputs[player_i];
			playerUp = tmpPlayerIn.up;
			playerDown = tmpPlayerIn.down;
			playerLeft = tmpPlayerIn.left;
			playerRight = tmpPlayerIn.right;
			playerFire = tmpPlayerIn.fire;
			input_mutex.unlock();

			Vector move(0, 0);
			if (playerUp)
			{
				move = Vector(0.0, -1 * SECONDS_PER_TICK * PLAYER_SPEED);
				tmpPlayer.setPlayerDirection(Player::Direction::UP);
				tmpPlayer.setPlayerAction(Player::Action::MOVE);
			}
			else if (playerDown)
			{
				move = Vector(0.0, 1 * SECONDS_PER_TICK * PLAYER_SPEED);
				tmpPlayer.setPlayerDirection(Player::Direction::DOWN);
				tmpPlayer.setPlayerAction(Player::Action::MOVE);
			}
			else if (playerLeft)
			{
				move = Vector(-1 * SECONDS_PER_TICK * PLAYER_SPEED, 0.0);
				tmpPlayer.setPlayerDirection(Player::Direction::LEFT);
				tmpPlayer.setPlayerAction(Player::Action::MOVE);
			}
			else if (playerRight)
			{
				move = Vector(1 * SECONDS_PER_TICK * PLAYER_SPEED, 0.0);
				tmpPlayer.setPlayerDirection(Player::Direction::RIGHT);
				tmpPlayer.setPlayerAction(Player::Action::MOVE);
			}
			else
			{
				tmpPlayer.setPlayerAction(Player::Action::IDLE);
			}

			bool notCollide = true;
			for (uint16 i = 0; i < MAX_CLIENTS; ++i)
			{
				if (!client_endpoints[i].address)
					continue;
				if (&tmpPlayer == &player_objects[i])
					continue;
				if (tmpPlayer.CheckCollisionMove(player_objects[i], move))
				{
					notCollide = false;
					break;
				}
			}
			if (notCollide)
			{
				for (uint16 i = 0; i < MAP_BORDERS; ++i)
				{
					if (tmpPlayer.CheckCollisionMove(map_border[i], move))
					{
						notCollide = false;
						break;
					}
				}
				if (notCollide)
				{
					for (uint16 object_i = 0; object_i < this->other_collision_objects.size(); ++object_i)
					{
						if (tmpPlayer.CheckCollisionMove(*other_collision_objects[object_i], move))
						{
							notCollide = false;
							break;
						}
					}
					if (notCollide)
						tmpPlayer.Move(move);
				}
			}
			if (playerFire)
			{
				if (tmpPlayer.readyToFire() && tmpPlayer.getPlayerAmmo() > 0)
				{
					uint16 offset = player_i * MAX_PROJECTILES;
					for (uint16 i = 0; i < MAX_PROJECTILES; ++i)
					{
						if (projectil_objects[offset + i].getProjectilStatus() == Projectil::DISABLED)
						{
							projectil_objects[offset + i].Activate(tmpPlayer.getBody(), (Projectil::Projectil_Direction)tmpPlayer.getPlayerDirection());
							//printf("Projectil %d of player %d was Activated \n", projectil_objects[offset + i].getProjectilNumber(), projectil_objects[offset + i].getOwnerSlot());
							tmpPlayer.Shoot();
							break;
						}
					}
				}
			}
		}
		else
		{
			tmpPlayer.setPlayerAction(Player::Action::KILLED);
		}

	}
}

void Server::HandlePlayerUpdate(uint16 playerSlot)
{
	Player& tmpPlayer = player_objects[playerSlot];
	if (tmpPlayer.getPlayerStatus() == Player::Status::ALIVE)
	{
		if (!tmpPlayer.readyToFire())
		{
			if (tmpPlayer.getPlayerAmmo() <= 0)
			{
				if (tmpPlayer.getReloadTime() >= RELOAD_TIME)
				{
					tmpPlayer.setReloadTime(0);
					tmpPlayer.Reload();
					//printf("Player %d reloaded\n", playerSlot);
				}
				else
					tmpPlayer.Update(SECONDS_PER_TICK);
			}
			else
			{
				if (tmpPlayer.getLastShotTime() >= RECOIL_TIME)
				{
					tmpPlayer.setShotTime(0);
					tmpPlayer.setReadyToFire(true);
					//printf("Player %d ready to shoot \n", playerSlot);
				}
				else
					tmpPlayer.Update(SECONDS_PER_TICK);
			}
		}
	}
	else
	{
		if (tmpPlayer.getRespawnTime() >= RESPAWN_TIME)
		{
			this->RespawnPlayer(playerSlot);
		}
		else
			tmpPlayer.Update(SECONDS_PER_TICK);
	}
}

void Server::HandlePlayerProjectilesUpdate(uint16 playerSlot)
{
	for (uint16 i_projectil = 0; i_projectil < MAX_PROJECTILES; ++i_projectil)
	{
		Projectil& tmpProjectil = projectil_objects[playerSlot * MAX_PROJECTILES + i_projectil];
		if (tmpProjectil.getProjectilStatus() == Projectil::Projectil_Status::ACTIVE)
		{
			Vector projectilPosition = tmpProjectil.getPosition();

			if (projectilPosition.x < -(MAP_WIDTH / 2)
				|| projectilPosition.x >(MAP_WIDTH / 2)
				|| projectilPosition.y < -(MAP_HEIGHT / 2)
				|| projectilPosition.y > +(MAP_HEIGHT / 2))
			{
				tmpProjectil.setProjectilStatus(Projectil::Projectil_Status::DISABLED);
			}
			else
			{
				tmpProjectil.moveInDirection(SECONDS_PER_TICK * PROJECTIL_SPEED);
			}
		}
	}
}

void Server::CheckHits(float deltaTime)
{

	Projectil* tmpProjectil;
	Vector v0;
	Vector v1;
	Vector vecInterSection;
	Hittable* tmpHittable = nullptr;

	uint16 offset = -1;
	float32 distance = SECONDS_PER_TICK * PROJECTIL_SPEED;
	for (uint16 i = 0; i < MAX_CLIENTS; ++i)
	{
		if (!client_endpoints[i].address)
			continue;
		Player* tmpPlayer = &player_objects[i];
		offset = i * MAX_PROJECTILES;
		for (uint16 projectil_i = 0; projectil_i < MAX_PROJECTILES; ++projectil_i)
		{
			tmpProjectil = &projectil_objects[offset + projectil_i];
			if (tmpProjectil->getProjectilStatus() == Projectil::ACTIVE)
			{
				v0 = (tmpProjectil->getBody().getPosition());
				v1 = tmpProjectil->getNewPositionInDirection(distance);
				if (traceLine(v0, v1, vecInterSection, tmpPlayer, tmpHittable))
				{
					if (tmpHittable != nullptr)
					{
						tmpHittable->HittedAction();
						if (tmpHittable->countAsKill())
						{
							tmpPlayer->CountKill();
						}

						tmpProjectil->setProjectilStatus(Projectil::Projectil_Status::DISABLED);
						std::string playerName = tmpPlayer->getName();
						std::string hittedName = tmpHittable->getName();
						printf("%s zasiahol %s\n", tmpPlayer->getName().c_str(), tmpHittable->getName().c_str());
					}
				}
			}
		}
	}
}

void Server::ResetClientInput(uint16 playerSlot)
{
	Player_Input& tmpPlayerIn = client_inputs[playerSlot];
	tmpPlayerIn.up = false;
	tmpPlayerIn.down = false;
	tmpPlayerIn.left = false;
	tmpPlayerIn.right = false;
	tmpPlayerIn.fire = false;
}

bool Server::traceLine(const Vector & pociatocnyVektor, const Vector & koncovyVektor, Vector & vecIntersection, Player* firedBy, Hittable *& zasiahnuty)
{
	float flLowestFraction = 1;
	Vector vecTestIntersection;
	float flTestFraction;
	AABB aabbSize;
	Hittable* checkedHittable = nullptr;
	for (int i = 0; i < hittable_objects.size(); ++i)
	{
		checkedHittable = hittable_objects.at(i);
		if (!checkedHittable->isActive())
			continue;
		if (checkedHittable == (Hittable*)firedBy)
			continue;
		aabbSize = checkedHittable->getAABBsize() + Point(checkedHittable->getBody().getPosition());
		if (lineAABBIntersection(aabbSize, pociatocnyVektor, koncovyVektor, vecTestIntersection, flTestFraction) && flTestFraction < flLowestFraction)
		{
			vecIntersection = vecTestIntersection;
			flLowestFraction = flTestFraction;
			zasiahnuty = checkedHittable;
		}
	}
	if (flLowestFraction < 1)
		return true;

	return false;
}

bool Server::lineAABBIntersection(const AABB & aabbBox, const Vector & v0, const Vector & v1, Vector & vecIntersection, float & flFraction)
{
	float f_low = 0;
	float f_high = 1;
	if (!clipLine(0, aabbBox, v0, v1, f_low, f_high))
	{
		return false;
	}
	if (!clipLine(1, aabbBox, v0, v1, f_low, f_high))
	{
		return false;
	}
	Vector b = v1 - v0;
	vecIntersection = v0 + b * f_low;

	flFraction = f_low;

	return true;
}

bool Server::clipLine(int d, const AABB & aabbBox, const Vector & v0, const Vector & v1, float & f_low, float & f_high)
{
	// f_low and f_high are the results from all clipping so far. We'll write our results back out to those parameters.

// f_dim_low and f_dim_high are the results we're calculating for this current dimension.
	float f_dim_low, f_dim_high;

	// Find the point of intersection in this dimension only as a fraction of the total vector http://youtu.be/USjbg5QXk3g?t=3m12s
	f_dim_low = (aabbBox.vecMin.v[d] - v0.v[d]) / (v1.v[d] - v0.v[d]);
	f_dim_high = (aabbBox.vecMax.v[d] - v0.v[d]) / (v1.v[d] - v0.v[d]);

	// Make sure low is less than high
	if (f_dim_high < f_dim_low)
		std::swap(f_dim_high, f_dim_low);

	// If this dimension's high is less than the low we got then we definitely missed. http://youtu.be/USjbg5QXk3g?t=7m16s
	if (f_dim_high < f_low)
		return false;

	// Likewise if the low is less than the high.
	if (f_dim_low > f_high)
		return false;

	// Add the clip from this dimension to the previous results http://youtu.be/USjbg5QXk3g?t=5m32s
	f_low = std::max(f_dim_low, f_low);
	f_high = std::min(f_dim_high, f_high);

	if (f_low > f_high)
		return false;

	return true;
}

Server::~Server()
{
	WSACleanup();
	for (uint16 object_i = 0; object_i < other_collision_objects.size(); ++object_i)
	{
		delete other_collision_objects[object_i];
	}
	other_collision_objects.clear();
	printf("Server shutdown - FINISH \n");
}
