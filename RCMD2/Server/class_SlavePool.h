#pragma once
#include "template_Pool.hpp"
#include "class_Slave.h"

class SlavePool :public Pool<Slave>
{
public:
	SlavePool(int capacity);

public:
	void refresh_and_clean();
};

bool test_alive(Slave& slave);
