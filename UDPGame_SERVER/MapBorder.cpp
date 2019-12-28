#include "MapBorder.h"



MapBorder::MapBorder()
{

}

MapBorder::MapBorder(Vector position, Vector size)
{
	this->m_border.setPosition(position);
	this->m_border.setSize(size);

}


Body & MapBorder::getBody()
{
	return this->m_border;
}

MapBorder::~MapBorder()
{
}
