#pragma once
#include "odin.h"
#include <stdio.h>
#include <stdlib.h> 
#include <string>
#include "CollisionObject.h"
class Player : public CollisionObject
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
	float getPlayerCordX();
	float getPlayerCordY();
	float getRespawnTime();
	float getReloadTime();
	float getLastShotTime();
	Body& getBody() override;

	void setCordX(float x);
	void setCordY(float y);
	void addCordX(float x);
	void addCordY(float y);
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

	bool CheckCollision(CollisionObject& other, Vector move);
	bool readyToFire();
	
	void Spawn(float x, float y);
	void UploadState(int8* buffer, int32& bytes_written);
	void InsertState(int8* buffer, int32& bytes_read);
	void Update(double deltaTime);
	void Move(Vector v);
	virtual ~Player();
private:
	Status m_playerStatus = Player::Status::ALIVE;
	Direction m_playerDirection = Player::Direction::DOWN;
	Action m_playerAction = Player::Action::IDLE;
	uint8 m_ammo = 4;
	float cordX = 10;
	float cordY = -200;
	float m_last_shot_t;
	float m_reload_t;
	float m_respawn_t;
	bool m_ready_to_fire;
	Body m_playerBody;
};

