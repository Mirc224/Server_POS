#include "Body.h"



Body::Body()
{
}

Vector& Body::getPosition()
{
	return m_position;
}

Vector& Body::getOrigin()
{
	return m_origin;
}

Vector& Body::getSize()
{
	return m_size;
}

void Body::setPosition(const Vector & othr)
{
	m_position = othr;
}

void Body::setPosition(const float x, const float y, const float z)
{
	this->m_position.x = x;
	this->m_position.y = y;
	this->m_position.z = z;
}

void Body::setOrigin(const Vector & othr)
{
	m_origin = othr;
}

void Body::setOrigin(const float x, const float y, const float z)
{
	this->m_origin.x = x;
	this->m_origin.y = y;
	this->m_origin.z = z;
}

void Body::setSize(const Vector & othr)
{
	m_size = othr;
}

void Body::setSize(const float x, const float y, const float z)
{
	this->m_size.x = x;
	this->m_size.y = y;
	this->m_size.z = z;
}

void Body::Move(const Vector vec)
{
	this->m_position = this->m_position + vec;
}


Body::~Body()
{
}
