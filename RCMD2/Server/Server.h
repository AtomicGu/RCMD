#pragma once
#include <stlcli/stlcli.h>
#include <stlcli/stlcliex.h>
#include "class_MasterPool.h"
#include "class_MasterAttendant.h"
#include "class_SlavePool.h"
#include "class_StatusBoard.h"

constexpr unsigned short	kMasterPort = 23333;
constexpr int				kMasterBacklog = 3;
constexpr unsigned short	kSlavePort = 23334;
constexpr int				kSlaveBacklog = 10;
constexpr int				kMaxMasterNumber = 3;
constexpr int				kMaxSlaveNumber = 10;

extern stlcli::Console g_console;

extern MasterPool g_master_pool;

extern SlavePool g_slave_pool;

extern MasterAttendant* g_attendant_pa[];

extern StatusBoard g_status_board;
