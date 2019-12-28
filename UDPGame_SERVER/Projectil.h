#pragma once
#include "odin.h"
#include <stdio.h>
#include <stdlib.h> 
#include <string>
#include "Body.h"
class Projectil
{
public:
	enum Projectil_Status { ACTIVE, DISABLED };
	enum Projectil_Direction { DOWN, UP, LEFT, RIGHT };
	Projectil();
	Projectil::Projectil_Status getProjectilStatus();
	Projectil::Projectil_Direction getProjectilDirection();
	uint16 getOwnerSlot();
	uint8 getProjectilNumber();
	Vector getPosition();
	Vector getNewPositionInDirection(float distance);
	float getCordX();
	float getCordY();
	Body& getBody();
	void setProjectilStatus(Projectil::Projectil_Status status);
	void setProjectilDirection(Projectil::Projectil_Direction direction);
	void setOwnerSlot(uint16 ownerSlot);
	void setProjectilNumber(uint8 projectilNum);
	void setCordX(float x);
	void setCordY(float y);
	void moveInDirection(float distance);
	void Activate(Body playerBody, Projectil::Projectil_Direction direction);
	void UploadState(int8* buffer, int32& bytes_written);
	void InsertState(int8* buffer, int32& bytes_read);
	virtual ~Projectil();
private:
	uint16 m_playerSlot;
	uint8 m_projectilNumber;
	Projectil_Status m_projectilStatus = Projectil_Status::DISABLED;
	Projectil_Direction m_projectilDirection = Projectil_Direction::DOWN;
	Body m_body;

};

