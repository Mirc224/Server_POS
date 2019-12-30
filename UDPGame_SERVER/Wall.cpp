#include "Wall.h"


Wall::Wall(Vector size, Vector position, std::string name)
{

	float sirka = size.x;
	float vyska = size.y;

	this->m_objectBody.setPosition(position);
	this->m_objectBody.setSize(size);
	this->m_objectBody.setOrigin(this->m_objectBody.getSize() / 2.0f);

	this->m_aabbsize.vecMin = Vector(-sirka / 2, -vyska / 2);
	this->m_aabbsize.vecMax = Vector(sirka / 2, vyska / 2);
	this->m_name = name;
}

std::string Wall::getName()
{
	return this->m_name;
}


Body & Wall::getBody()
{
	return this->m_objectBody;
}

AABB & Wall::getAABBsize()
{
	return this->m_aabbsize;
}

bool Wall::isActive()
{
	return true;
}

void Wall::HittedAction()
{
}

bool Wall::countAsKill()
{
	return false;
}

Wall::~Wall()
{
}
