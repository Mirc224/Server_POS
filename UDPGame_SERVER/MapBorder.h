#pragma once
#include "CollisionObject.h"
class MapBorder : public CollisionObject
{
public:
	MapBorder();
	MapBorder(Vector position, Vector size);
	Body& getBody() override;
	virtual ~MapBorder();
private:
	Body m_border;
};

