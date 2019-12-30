#pragma once
#include "CollisionObject.h"
#include "Hittable.h"
class Wall : public CollisionObject, public Hittable
{
public:
	Wall(Vector size, Vector position, std::string name = "object");
	std::string getName() override;
	Body& getBody() override;
	AABB& getAABBsize() override;
	bool isActive() override;
	void HittedAction() override;
	bool countAsKill() override;
	virtual ~Wall();
private:
	Body m_objectBody;
	AABB m_aabbsize;
	std::string m_name;
};

