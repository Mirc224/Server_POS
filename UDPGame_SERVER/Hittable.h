#pragma once
#include <string>
#include "AABB.h"
#include "Body.h"
class Hittable
{
public:
	virtual AABB& getAABBsize() = 0;
	virtual bool isActive() = 0;
	virtual Body& getBody() = 0;
	virtual std::string getName()= 0;
	virtual void HittedAction() = 0;
	virtual ~Hittable() = default;
};