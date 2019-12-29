#pragma once
#include "odin.h"
#include <stdio.h>
#include <stdlib.h> 
#include <string>
#include "CollisionObject.h"
#include "Hittable.h"
class Player : public CollisionObject, public Hittable
{
public:
	enum Status : uint8 { ALIVE, DEATH };
	enum Direction : uint8 { DOWN, UP, LEFT, RIGHT };
	enum Action : uint8 { IDLE, MOVE, KILLED = 8};
	Player();
	Player::Status getPlayerStatus();
	Player::Direction getPlayerDirection();
	Player::Action getPlayerAction();
	uint8 getPlayerAmmo();
	uint16 getPlayerSlot();
	uint16 getKills();
	uint16 getDeaths();
	float getPlayerCordX();
	float getPlayerCordY();
	float getRespawnTime();
	float getReloadTime();
	float getLastShotTime();
	std::string getName() override;
	Body& getBody() override;
	AABB& getAABBsize() override;
	bool isActive() override;

	void setCordX(float x);
	void setCordY(float y);
	void addCordX(float x);
	void addCordY(float y);
	void setPlayerSlot(uint16 playerSlot);
	void setPlayerStatus(Player::Status newStatus);
	void setPlayerDirection(Player::Direction newDirection);
	void setPlayerAction(Player::Action newAction);
	void setAmmo(uint8 ammo);
	void setReloadTime(float time);
	void setShotTime(float time);
	void setRespawTime(float time);
	void addReloadTime(float deltaTime);
	void addShotTime(float deltaTime);
	void addRespawnTime(float deltaTime);
	void setReadyToFire(bool ready);

	void Reload();
	void Shoot();

	bool CheckCollisionMove(CollisionObject& other, Vector move);
	bool CheckCollision(CollisionObject& other);
	bool readyToFire();
	
	void HittedAction();
	void Spawn();
	void CountKill();
	void UploadState(int8* buffer, int32& bytes_written);
	void InsertState(int8* buffer, int32& bytes_read);
	void Update(double deltaTime);
	void Move(Vector v);
	virtual ~Player();

private:
	bool hasIntersection(float otherPosX, float otherPosY, float otherHalfSizeX, float otherHalfSizeY,
		float thisPosX, float thisPosY, float thisHalfSizeX, float thisHalfSizeY);


private:
	Status m_playerStatus = Player::Status::ALIVE;
	Direction m_playerDirection = Player::Direction::DOWN;
	Action m_playerAction = Player::Action::IDLE;
	uint8 m_ammo = 4;
	uint16 m_playerSlot;
	float m_last_shot_t;
	float m_reload_t;
	float m_respawn_t;
	bool m_ready_to_fire;
	Body m_playerBody;
	AABB m_aabbsize;
	uint16 m_kills = 0;
	uint16 m_deaths = 0;
};

