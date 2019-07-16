#ifndef caveH
#define caveH

#include "Level.h"
#include "Leaf.h"
#include "../delaunay/delaunay.h"
#include <vector>
#include <list>
#include <queue>
#include <iostream>

class MCave: public MLevel {
private:
	int RandomPercent;
	int SmoothCycles;
	int WallsToSmooth;
	int WallTreshold ;
	int RoomTreshold;
	char** FlagsMap;
	std::list<std::list<NVector2> > RoomsRegionsList;
	std::list<std::list<NVector2> > WallsRegionsList;
	
	std::list<NVector2> GetRegion(int i, int j, char Value);
	bool SmoothFloors();
	bool SmoothWalls();
	bool CreateRegions();
	bool CreateHalls();
	bool Triangulate();
public:
	MCave();
	MCave(int TilesCountX, int TilesCountY, int inRandomPercent, int inSmoothCycles, int inWallsToSmooth, int inWallTreshold, int inRoomTreshold);
	bool Generate();
	void Clear();
	void Close();
	int GetType();
};

#endif
