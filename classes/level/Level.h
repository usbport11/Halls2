#ifndef levelH
#define levelH

#define LGT_NONE 0
#define LGT_DUNGEON 1
#define LGT_CAVE 2

#define TT_NONE 0
#define TT_FLOOR 1
#define TT_WALL_FULL 2
#define TT_WALL_PART 3 
#define TT_LSTART 4
#define TT_LEND 5

#include <string.h>
#include "../system/random.h"

class MLevel {
protected:
	char** Map;
	int Type;
	int TilesCount[2];
	int nStartPoint[2];
	int nEndPoint[2];
	
	void FillMap(int x , int y, int Width, int Height, char Value);
	bool AllocateMap();
	bool AllocateMap(int TilesCountX, int TilesCountY);
	bool ReallocateMap(int TilesCountX, int TilesCountY);
	bool ConvertToTiles();
public:
	MLevel();
	MLevel(int TilesCountX, int TilesCountY);
	char GetValue(int x, int y);
	virtual bool Generate();
	virtual int GetType();
	void Clear();
	void Close();
};

#endif
