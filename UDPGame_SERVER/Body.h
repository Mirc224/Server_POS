#pragma once
#include "Vector.h"
class Body
{
public:
	Body();
	Vector& getPosition();
	Vector& getOrigin();
	Vector& getSize();
	void setPosition(const Vector& othr);
	void setPosition(const float x, const float y, const float z=0);
	void setOrigin(const Vector& othr);
	void setOrigin(const float x, const float y, const float z = 0);
	void setSize(const Vector& othr);
	void setSize(const float x, const float y, const float z = 0);
	void Move(const Vector vec);
	virtual ~Body();
private:
	Vector m_position;
	Vector m_origin;
	Vector m_size;

};

