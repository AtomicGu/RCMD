#pragma once
#include "class_Master.h"

class MasterPtr
{
public:
	MasterPtr(Master* p);
	Master* get();

private:
	Master* _p;
};
