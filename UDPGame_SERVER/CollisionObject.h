#pragma once
#include "Body.h"

class CollisionObject
{
public:
	//virtual Collider getCollider() = 0;
	virtual Body& getBody() = 0;
	virtual ~CollisionObject() = default;
};