#pragma once
#include "class_Master.h"
#include "template_Pool.hpp"

class MasterPool :public Pool<Master>
{
public:
	MasterPool(int capacity);

public:
	void refresh_and_clean();
};
