#pragma once
#include <unordered_set>
#include <BWAPI.h>

#define THRESHOLD1_MINERALS 200
#define THRESHOLD1_UNUSED_SUPPLY 2

#define NWANTED_WORKERS_TOTAL 20
#define NWANTED_WORKERS_FARMING_MINERALS 100

class Data {
public:
	int currMinerals;
	int thresholdMinerals;
	int currSupply;
	int thresholdSupply;

	int nWantedWorkersTotal;
	int nWantedWorkersFarmingMinerals;

	std::unordered_set<BWAPI::Unit> unitsFarmingMinerals;

	bool step1;

	bool step3;

	BWAPI::Unit myOverlord;

	std::vector<BWAPI::TilePosition> visistedStartingPositions;

	BWAPI::Position enemyBase;

	bool enemyBaseFound;
};
