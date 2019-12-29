#include "Player.h"



Player::Player()
{
	float ratio = 1;

	float sirka = 50.0f * ratio;
	float vyska = 70.0f * ratio;
	this->m_playerBody.setSize(Vector(sirka, vyska));
	this->m_playerBody.setOrigin(this->m_playerBody.getSize() / 2.0f);

	this->m_aabbsize.vecMin = Vector(-sirka / 2, -vyska / 2);
	this->m_aabbsize.vecMax = Vector(sirka / 2, vyska / 2);
}

Player::Status Player::getPlayerStatus()
{
	return this->m_playerStatus;
}

Player::Direction Player::getPlayerDirection()
{
	return this->m_playerDirection;
}

Player::Action Player::getPlayerAction()
{
	return this->m_playerAction;
}

uint8 Player::getPlayerAmmo()
{
	return this->m_ammo;
}

uint16 Player::getPlayerSlot()
{
	return this->m_playerSlot;
}

uint16 Player::getKills()
{
	return this->m_kills;
}

uint16 Player::getDeaths()
{
	return this->m_deaths;
}

void Player::setCordX(float x)
{
	this->m_playerBody.getPosition().x = x;
}

void Player::setCordY(float y)
{
	this->m_playerBody.getPosition().y = y;
}

void Player::addCordX(float x)
{
	this->m_playerBody.getPosition().x += x;
}

void Player::addCordY(float y)
{
	this->m_playerBody.getPosition().y += y;
}

void Player::setPlayerSlot(uint16 playerSlot)
{
	this->m_playerSlot = playerSlot;
}

Body & Player::getBody()
{
	return this->m_playerBody;
}

AABB & Player::getAABBsize()
{
	return this->m_aabbsize;
}

bool Player::isActive()
{
	return this->m_playerStatus == Player::Status::ALIVE;
}

bool Player::CheckCollision(CollisionObject & other)
{
	Vector otherPosition = other.getBody().getPosition();
	Vector otherHalfSize = other.getBody().getSize() / 2.0f;
	Vector thisPosition = getBody().getPosition();
	Vector thisHalfSize = getBody().getSize() / 2.0f;

	return hasIntersection(otherPosition.x, otherPosition.y, otherHalfSize.x, otherHalfSize.y, 
		thisPosition.x, thisPosition.y, thisHalfSize.x, thisHalfSize.y);
}

bool Player::readyToFire()
{
	return m_ready_to_fire;
}

void Player::setPlayerStatus(Player::Status newStatus)
{
	this->m_playerStatus = newStatus;
}

void Player::setPlayerDirection(Player::Direction newDirection)
{
	this->m_playerDirection = newDirection;
}

void Player::setPlayerAction(Player::Action newAction)
{
	this->m_playerAction = newAction;
}

void Player::setAmmo(uint8 ammo)
{
	this->m_ammo = ammo;
}

void Player::setReloadTime(float time)
{
	this->m_reload_t = time;
}

void Player::setShotTime(float time)
{
	this->m_last_shot_t = time;
}

void Player::setRespawTime(float time)
{
	this->m_respawn_t = time;
}

void Player::addReloadTime(float deltaTime)
{
	this->m_reload_t += deltaTime;
}

void Player::addShotTime(float deltaTime)
{
	this->m_last_shot_t += deltaTime;
}

void Player::addRespawnTime(float deltaTime)
{
	this->m_respawn_t += deltaTime;
}

void Player::setReadyToFire(bool ready)
{
	this->m_ready_to_fire = ready;
}

void Player::Reload()
{
	this->m_last_shot_t = 0.0f;
	this->m_reload_t = 0.0f;
	this->m_ammo = 4;
	this->m_ready_to_fire = true;
}

void Player::Shoot()
{
	this->m_ready_to_fire = false;
	this->m_last_shot_t = 0.0f;
	--this->m_ammo;
	if (this->m_ammo <= 0)
		this->m_reload_t = 0.0f;
}

bool Player::CheckCollisionMove(CollisionObject & other, Vector move)
{
	Vector otherPosition = other.getBody().getPosition();
	Vector otherHalfSize = other.getBody().getSize() / 2.0f;
	Vector thisPosition = getBody().getPosition() + move;
	Vector thisHalfSize = getBody().getSize() / 2.0f;

	return hasIntersection(otherPosition.x, otherPosition.y, otherHalfSize.x, otherHalfSize.y,
		thisPosition.x, thisPosition.y, thisHalfSize.x, thisHalfSize.y);
}

float Player::getPlayerCordX()
{
	return this->m_playerBody.getPosition().x;
}

float Player::getPlayerCordY()
{
	return this->m_playerBody.getPosition().y;
}

float Player::getRespawnTime()
{
	return this->m_respawn_t;
}

float Player::getReloadTime()
{
	return this->m_reload_t;
}

float Player::getLastShotTime()
{
	return this->m_last_shot_t;
}

std::string Player::getName()
{
	return "Hrac " + std::to_string(this->m_playerSlot);
}

void Player::HittedAction()
{
	this->m_playerStatus = Player::Status::DEATH;
	this->m_playerAction = Player::Action::KILLED;
	this->m_respawn_t = 0.0f;
	++this->m_deaths;
}

void Player::Spawn()
{
	this->m_respawn_t = 0.0f;
	this->Reload();
	this->m_playerDirection = Player::Direction::DOWN;
	this->m_playerAction = Player::Action::IDLE;
	this->m_playerStatus = Player::Status::ALIVE;
}

void Player::UploadState(int8 * buffer, int32 & bytes_written)
{
	buffer[bytes_written++] = this->m_playerStatus;
	buffer[bytes_written++] = this->m_playerDirection;
	buffer[bytes_written++] = this->m_playerAction;
	buffer[bytes_written++] = this->m_ammo;
	float tmpCord = this->m_playerBody.getPosition().x;
	memcpy(&buffer[bytes_written], &tmpCord, sizeof(tmpCord));
	bytes_written += sizeof(tmpCord);
	tmpCord = this->m_playerBody.getPosition().y;
	memcpy(&buffer[bytes_written], &tmpCord, sizeof(tmpCord));
	bytes_written += sizeof(this->m_playerBody.getPosition().y);
}

void Player::InsertState(int8 * buffer, int32 & bytes_read)
{
	this->m_playerStatus = (Player::Status)buffer[bytes_read++];
	this->m_playerDirection = (Player::Direction)buffer[bytes_read++];
	this->m_playerAction = (Player::Action)buffer[bytes_read++];
	this->m_ammo = buffer[bytes_read++];
	auto tmp = &this->m_playerBody.getPosition().x;
	memcpy(tmp, &buffer[bytes_read], sizeof(*tmp));
	bytes_read += sizeof(*tmp);
	tmp = &this->m_playerBody.getPosition().y;
	memcpy(tmp, &buffer[bytes_read], sizeof(*tmp));
	bytes_read += sizeof(*tmp);
}

void Player::Update(double deltaTime)
{
	if (this->m_playerStatus == Player::Status::ALIVE)
	{
		if (!this->m_ready_to_fire)
		{
			if (this->m_ammo <= 0)
			{
				this->m_reload_t += deltaTime;
			}
			else
			{
				this->m_last_shot_t += deltaTime;
			}
		}
	}
	else
	{
		this->m_respawn_t += deltaTime;
	}
}

void Player::Move(Vector v)
{
	this->m_playerBody.Move(v);
}

Player::~Player()
{
}

bool Player::hasIntersection(float otherPosX, float otherPosY, float otherHalfSizeX, float otherHalfSizeY, float thisPosX, float thisPosY, float thisHalfSizeX, float thisHalfSizeY)
{
	float deltaX = otherPosX - thisPosX;
	float deltaY = otherPosY - thisPosY;
	float intersectX = abs(deltaX) - (otherHalfSizeX + thisHalfSizeX);
	float intersectY = abs(deltaY) - (otherHalfSizeY + thisHalfSizeY);
	if (intersectX < 0.0f && intersectY < 0.0f)
		return true;

	return false;
}
