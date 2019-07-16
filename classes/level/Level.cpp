#include "Level.h"

MLevel::MLevel() {
	Map = NULL;
	TilesCount[0] = 0;
	TilesCount[1] = 0;
}

MLevel::MLevel(int TilesCountX, int TilesCountY) {
	Map = NULL;
	TilesCount[0] = 0;
	TilesCount[1] = 0;
	if(TilesCountX > 0) TilesCount[0] = TilesCountX;
	if(TilesCountY > 0) TilesCount[1] = TilesCountY;
}

char MLevel::GetValue(int x, int y) {
	return Map[x][y];
}

bool MLevel::AllocateMap() {
	if(TilesCount[0] <= 0 || TilesCount[1] <= 0) return false;
	if(Map) return false;
	
	Map = new char* [TilesCount[0]];
	if(!Map) return false;
	for(int i=0; i<TilesCount[0]; i++) {
		Map[i] = new char[TilesCount[1]];
		if(!Map[i]) return false;
		memset(Map[i], 0, TilesCount[1]);
	}
	
	return true;
}

bool MLevel::AllocateMap(int TilesCountX, int TilesCountY) {
	if(TilesCountX <= 0 || TilesCountY <= 0) return false;
	
	TilesCount[0] = TilesCountX;
	TilesCount[1] = TilesCountY;
	
	return AllocateMap();
}

bool MLevel::ReallocateMap(int TilesCountX, int TilesCountY) {
	if(TilesCountX <= 0 || TilesCountY <= 0) return false;
	
	TilesCount[0] = TilesCountX;
	TilesCount[1] = TilesCountY;
	
	Close();
	
	return AllocateMap();
}

bool MLevel::ConvertToTiles() {
	if(!Map) return false;
	
	for(int i=0; i<TilesCount[0] - 1; i++) {
		for(int j=0; j<TilesCount[1] - 1; j++) {
			if(Map[i][j] == TT_NONE) continue;
			//left wall
			if(i > 0) {
				if(Map[i][j] == TT_FLOOR && Map[i-1][j] == TT_NONE) Map[i-1][j] = TT_WALL_FULL;
			}
			//top wall
			if(Map[i][j] == TT_FLOOR && Map[i][j+1] == TT_NONE) Map[i][j+1] = TT_WALL_PART;
		}
	}
	
	for(int i=0; i<TilesCount[0] - 1; i++) {
		for(int j=0; j<TilesCount[1] - 1; j++) {
			if(Map[i][j] == TT_NONE) continue;
			//right walls
			if(Map[i][j] == TT_FLOOR && Map[i+1][j] == TT_NONE) Map[i+1][j] = TT_WALL_FULL;
			//bottom wall
			if(j > 0) {
				if(Map[i][j] == TT_FLOOR && Map[i][j-1] == TT_NONE) Map[i][j-1] = TT_WALL_PART;
			}
		}
	}
	
	//bottom angles
	for(int i=0; i<TilesCount[0]; i++) {
		for(int j=0; j<TilesCount[1]; j++) {
			if(Map[i][j] != TT_WALL_FULL) continue;
			if(j > 0 && Map[i][j-1] == TT_NONE) {
				Map[i][j-1] = TT_WALL_PART;
			}
		}
	}
	
	//top angels
	for(int i=0; i<TilesCount[0]-1; i++) {
		for(int j=0; j<TilesCount[1]-1; j++) {
			if(Map[i][j] != TT_NONE) continue;
			if(j > 0) {
				if(Map[i][j-1] == TT_WALL_FULL && Map[i+1][j] == TT_WALL_PART && Map[i+1][j-1] == TT_FLOOR) Map[i][j] = TT_WALL_FULL;
				if(Map[i][j-1] == TT_WALL_PART && Map[i+1][j] == TT_WALL_PART && Map[i+1][j-1] == TT_FLOOR) Map[i][j] = TT_WALL_FULL;
			}
			if(i > 0) {
				if(Map[i][j-1] == TT_WALL_FULL && Map[i-1][j] == TT_WALL_PART && Map[i-1][j-1] == TT_FLOOR) Map[i][j] = TT_WALL_FULL;
				if(Map[i][j-1] == TT_WALL_PART && Map[i-1][j] == TT_WALL_PART && Map[i-1][j-1] == TT_FLOOR) Map[i][j] = TT_WALL_FULL;
			}
		}
	}
	
	//fix top (TT_WALL_FULL - TT_WALL_PART)
	for(int i=0; i<TilesCount[0]-1; i++) {
		for(int j=0; j<TilesCount[1]-1; j++) {
			if(Map[i][j] != TT_WALL_FULL) continue;
			if(Map[i][j+1] == TT_WALL_PART) Map[i][j+1] = TT_WALL_FULL;
		}
	}
	
	//fix TT_WALL_PART over TT_WALL_PART
	for(int i=0; i<TilesCount[0]-1; i++) {
		for(int j=0; j<TilesCount[1]-1; j++) {
			if(Map[i][j] == TT_WALL_PART && Map[i][j+1] == TT_WALL_PART) Map[i][j+1] = TT_WALL_FULL;
		}
	}
	
	//set start and end points
	if(nStartPoint[0] > 0 && nStartPoint[1] > 0) Map[nStartPoint[0]][nStartPoint[1]] = TT_LSTART;
	if(nEndPoint[0] < TilesCount[0] && nEndPoint[1] < TilesCount[1]) Map[nEndPoint[0]][nEndPoint[1]] = TT_LEND;
	
	return true;
}

bool MLevel::Generate() {
	return false;
}

int MLevel::GetType() {
	return LGT_NONE;
}

void MLevel::Clear() {
	if(Map) {
		for(int i=0; i<TilesCount[0]; i++) {
			memset(Map[i], 0, TilesCount[1]);
		}
	}
}

void MLevel::Close() {
	if(Map) {
		for(int i=0; i<TilesCount[0]; i++) {
			if(Map[i]) delete [] Map[i];
		}
		delete [] Map;
	}
	Map = NULL;
}

void MLevel::FillMap(int x , int y, int Width, int Height, char Value) {
	if(!Map) return;
	if(x < 0 || y < 0 || x + Width > TilesCount[0] || y + Height > TilesCount[1]) return;
	for(int i=x; i<x + Width; i++) {
		for(int j=y; j<y + Height; j++) {
			Map[i][j] = Value;
		}
	}
}

