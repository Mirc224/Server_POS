#include "Projectil.h"



Projectil::Projectil()
{
}

Projectil::Projectil_Status Projectil::getProjectilStatus()
{
	return m_projectilStatus;
}

Projectil::Projectil_Direction Projectil::getProjectilDirection()
{
	return m_projectilDirection;
}

uint16 Projectil::getOwnerSlot()
{
	return m_playerSlot;
}

uint8 Projectil::getProjectilNumber()
{
	return m_projectilNumber;
}

Vector Projectil::getPosition()
{
	return this->m_body.getPosition();
}

Vector Projectil::getNewPositionInDirection(float distance)
{
	switch (this->m_projectilDirection)
	{
	case Projectil::Projectil_Direction::UP:
		return this->m_body.getPosition() + Vector(0.0f, -distance);
		break;
	case Projectil::Projectil_Direction::DOWN:
		return this->m_body.getPosition() + Vector(0.0f, +distance);
		break;
	case Projectil::Projectil_Direction::LEFT:
		return this->m_body.getPosition() + Vector(-distance, 0.0f);
		break;
	case Projectil::Projectil_Direction::RIGHT:
		return this->m_body.getPosition() + Vector(distance, 0.0f);
		break;
	default:
		return this->m_body.getPosition();
		break;
	}
}

float Projectil::getCordX()
{
	return m_body.getPosition().x;
}

float Projectil::getCordY()
{
	return m_body.getPosition().y;
}

Body & Projectil::getBody()
{
	return this->m_body;
}

void Projectil::setProjectilStatus(Projectil::Projectil_Status status)
{
	m_projectilStatus = status;
}

void Projectil::setProjectilDirection(Projectil::Projectil_Direction direction)
{
	m_projectilDirection = direction;
}

void Projectil::setOwnerSlot(uint16 ownerSlot)
{
	m_playerSlot = ownerSlot;
}

void Projectil::setProjectilNumber(uint8 projectilNum)
{
	m_projectilNumber = projectilNum;
}

void Projectil::setCordX(float x)
{
	this->m_body.getPosition().x = x;
}

void Projectil::setCordY(float y)
{
	this->m_body.getPosition().y = y;
}

void Projectil::moveInDirection(float distance)
{
	switch (this->m_projectilDirection)
	{
	case Projectil::Projectil_Direction::UP:
		this->m_body.Move(Vector(0.0f, -distance));
		break;
	case Projectil::Projectil_Direction::DOWN:
		this->m_body.Move(Vector(0.0f, +distance));
		break;
	case Projectil::Projectil_Direction::LEFT:
		this->m_body.Move(Vector(-distance, 0.0f));
		break;
	case Projectil::Projectil_Direction::RIGHT:
		this->m_body.Move(Vector(distance, 0.0f));
		break;
	default:
		break;
	}
}

void Projectil::Activate(Body playerBody, Projectil::Projectil_Direction direction)
{
	Vector projectilPosition = playerBody.getPosition();
	Vector playerSize = playerBody.getSize() / 2.0;
	this->m_projectilDirection = direction;
	switch (this->m_projectilDirection)
	{
	case Projectil::Projectil_Direction::UP:
		projectilPosition.y -= playerSize.y;
		break;
	case Projectil::Projectil_Direction::DOWN:
		projectilPosition.y += playerSize.y;
		break;
	case Projectil::Projectil_Direction::LEFT:
		projectilPosition.x -= playerSize.x;
		break;
	case Projectil::Projectil_Direction::RIGHT:
		projectilPosition.x += playerSize.x;
		break;
	default:
		break;
	}
	this->m_body.setPosition(projectilPosition);
	this->m_projectilStatus = Projectil::Projectil_Status::ACTIVE;
}


void Projectil::UploadState(int8 * buffer, int32 & bytes_written)
{
	memcpy(&buffer[bytes_written], &m_playerSlot, sizeof(m_playerSlot));
	bytes_written += sizeof(m_playerSlot);
	buffer[bytes_written++] = m_projectilNumber;
	buffer[bytes_written++] = m_projectilStatus;
	buffer[bytes_written++] = m_projectilDirection;
	float tmp = m_body.getPosition().x;
	memcpy(&buffer[bytes_written], &tmp, sizeof(tmp));
	bytes_written += sizeof(tmp);
	tmp = m_body.getPosition().y;
	memcpy(&buffer[bytes_written], &tmp, sizeof(tmp));
	bytes_written += sizeof(tmp);
}

void Projectil::InsertState(int8 * buffer, int32 & bytes_read)
{
	m_projectilStatus = (Projectil::Projectil_Status)buffer[bytes_read++];
	m_projectilDirection = (Projectil::Projectil_Direction)buffer[bytes_read++];
	memcpy(&m_body.getPosition().x, &buffer[bytes_read], sizeof(m_body.getPosition().x));
	bytes_read += sizeof(m_body.getPosition().x);
	memcpy(&m_body.getPosition().y, &buffer[bytes_read], sizeof(m_body.getPosition().y));
	bytes_read += sizeof(m_body.getPosition().y);
}


Projectil::~Projectil()
{
}
