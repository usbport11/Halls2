#include "Cave.h"

MCave::MCave():MLevel() {
	FlagsMap = NULL;
	RandomPercent = 0;
	SmoothCycles = 0;
	WallsToSmooth = 0;
	WallTreshold = 0;
	RoomTreshold = 0;
}

MCave::MCave(int TilesCountX, int TilesCountY, int inRandomPercent, int inSmoothCycles, int inWallsToSmooth, int inWallTreshold, int inRoomTreshold):MLevel(TilesCountX, TilesCountY) {
	FlagsMap = NULL;
	RandomPercent = 0;
	SmoothCycles = 0;
	WallsToSmooth = 0;
	WallTreshold = 0;
	RoomTreshold = 0;
	if(inRandomPercent > 0) RandomPercent = inRandomPercent;
	if(inSmoothCycles > 0) SmoothCycles = inSmoothCycles;
	if(inWallsToSmooth > 0) WallsToSmooth = inWallsToSmooth;
	if(inWallTreshold > 0) WallTreshold = inWallTreshold;
	if(inRoomTreshold > 0) RoomTreshold = inRoomTreshold;
}

std::list<NVector2> MCave::GetRegion(int i, int j, char Value) {
	std::list<NVector2> Region;
	std::queue<NVector2> Queue;
	NVector2 Cell;
	
	Queue.push(NVector2(i, j));
	FlagsMap[i][j] = 1;
	
	while(Queue.size()) {
		Cell = Queue.front();
		Region.push_back(Cell);
		for (int x = Cell.x - 1; x <= Cell.x + 1; x++) {
			for (int y = Cell.y - 1; y <= Cell.y + 1; y++) {
				if(x < TilesCount[0] && y < TilesCount[1] && x > 0 && y > 0) {
					if(Map[x][y] == Value && !FlagsMap[x][y] && (x == Cell.x || y == Cell.y) ) {
						Queue.push(NVector2(x, y));
						FlagsMap[x][y] = 1;
					}
				}
			}
        }
		Queue.pop();
	}
	
	return Region;
}

bool MCave::SmoothFloors() {
	int WallsCount;
	for(int k=0; k<SmoothCycles; k++) {
		for(int i=1; i<TilesCount[0] - 1; i++) {
			for(int j=1; j<TilesCount[1] - 1; j++) {
				WallsCount = Map[i-1][j-1] + Map[i][j-1] + Map[i+1][j-1] +
					Map[i-1][j] + Map[i+1][j] + 
					Map[i-1][j+1] + Map[i][j+1] + Map[i+1][j+1];
				if(WallsCount > WallsToSmooth) Map[i][j] = 1;
			}
		}	
	}
	
	return true;
}

bool MCave::SmoothWalls() {
	int WallsCount;
	for(int k=0; k<SmoothCycles; k++) {
		for(int i=1; i<TilesCount[0] - 1; i++) {
			for(int j=1; j<TilesCount[1] - 1; j++) {
				WallsCount = Map[i-1][j-1] + Map[i][j-1] + Map[i+1][j-1] +
					Map[i-1][j] + Map[i+1][j] + 
					Map[i-1][j+1] + Map[i][j+1] + Map[i+1][j+1];
				if(WallsCount < WallsToSmooth) Map[i][j] = 0;
			}
		}	
	}
	
	return true;
}


bool MCave::CreateRegions() {
	//work with regions
	FlagsMap = new char* [TilesCount[0]];
	if(!FlagsMap) return false;
	for(int i=0; i<TilesCount[0]; i++) {
		FlagsMap[i] = new char [TilesCount[1]];
		if(!FlagsMap[i]) return false;
		memset(FlagsMap[i], 0, TilesCount[1]);
	}
	
	//get regions
	for(int i=1; i<TilesCount[0] - 1; i++) {
		for(int j=1; j<TilesCount[1] - 1; j++) {
			if(FlagsMap[i][j] == 0 && Map[i][j] == 1) {
				RoomsRegionsList.push_back(GetRegion(i, j, 1));
			}
			if(FlagsMap[i][j] == 0 && Map[i][j] == 0) {
				WallsRegionsList.push_back(GetRegion(i, j, 0));
			}
		}
	}
	
	//remove small walls and rooms regions
	for(std::list<std::list<NVector2> >::iterator it = RoomsRegionsList.begin(); it != RoomsRegionsList.end(); it++) {
		if(it->size() < (unsigned int)RoomTreshold) {
			for(std::list<NVector2>::iterator ita = it->begin(); ita != it->end(); ita++) {
				Map[ita->x][ita->y] = 0;
			}
			RoomsRegionsList.erase(it);
		}
	}
	for(std::list<std::list<NVector2> >::iterator it = WallsRegionsList.begin(); it != WallsRegionsList.end(); it++) {
		if(it->size() < (unsigned int)WallTreshold) {
			for(std::list<NVector2>::iterator ita = it->begin(); ita != it->end(); ita++) {
				if(ita->x > 1 && ita->y > 1 && ita->x < TilesCount[0] && ita->y < TilesCount[1]) Map[ita->x][ita->y] = 1;
			}
			WallsRegionsList.erase(it);
		}
	}
	
	//remove flags map
	for(int i=0; i<TilesCount[0]; i++) {
		if(FlagsMap[i]) delete [] FlagsMap[i];
	}
	if(FlagsMap) delete [] FlagsMap;
	FlagsMap = NULL;
	
	return true;
}

bool MCave::CreateHalls() {
	if(RoomsRegionsList.size() <= 3) {
		std::cout<<"Room regions count <= 3"<<std::endl;
		return false;
	}
	if(WallsRegionsList.size() < 1) {
		std::cout<<"Walls regions count <= 3"<<std::endl;
		return false;
	}
	
	if(RoomsRegionsList.size() >= 2) {
		std::vector<glm::vec2> RandomPoints;
		std::list<NVector2>::iterator ita;
		std::vector<NVector2> LinePoints;
		
		//get random point from each of region
		for(std::list<std::list<NVector2> >::iterator it = RoomsRegionsList.begin(); it != RoomsRegionsList.end(); it++) {
			ita = it->begin();
			std::advance(ita, rand() % it->size());
			RandomPoints.push_back(glm::vec2(ita->x, ita->y));
		}
		if(RandomPoints.size() == 2) {
			std::cout<<"Room regioms count == 2"<<std::endl;
			LinePoints = CreateLine((int)RandomPoints[0].x, (int)RandomPoints[0].y, (int)RandomPoints[1].x, (int)RandomPoints[1].y);
			for(unsigned int j=0; j<LinePoints.size(); j++) {
				Map[LinePoints[j].x][LinePoints[j].y] = 1;
				if(LinePoints[j].x < TilesCount[0] - 1  && LinePoints[j].y < TilesCount[1] - 1) {
					Map[LinePoints[j].x + 1][LinePoints[j].y] = 1;
					Map[LinePoints[j].x][LinePoints[j].y + 1] = 1;
					Map[LinePoints[j].x + 1][LinePoints[j].y + 1] = 1;
				}
			}
			RandomPoints.clear();
			LinePoints.clear();
			return true;
		}
		
		//triangulate by delaunay and get mst
		MDelaunay Triangulation;
		std::vector<MTriangle> Triangles = Triangulation.Triangulate(RandomPoints);
		std::vector<MEdge> Edges = Triangulation.GetEdges();
		std::vector<MEdge> MST = Triangulation.CreateMSTEdges();
		
		//connect points
		for(unsigned int i=0; i<MST.size(); i++) {
			LinePoints = CreateLine((int)MST[i].p1.x, (int)MST[i].p1.y, (int)MST[i].p2.x, (int)MST[i].p2.y);
			if(!LinePoints.size()) {
				std::cout<<"Empty Line"<<std::endl;
				break;
			}
			for(unsigned int j=0; j<LinePoints.size(); j++) {
				Map[LinePoints[j].x][LinePoints[j].y] = 1;
				if(LinePoints[j].x < TilesCount[0] - 1  && LinePoints[j].y < TilesCount[1] - 1) {
					Map[LinePoints[j].x + 1][LinePoints[j].y] = 1;
					Map[LinePoints[j].x][LinePoints[j].y + 1] = 1;
					Map[LinePoints[j].x + 1][LinePoints[j].y + 1] = 1;
				}
			}
		}
		
		//clear
		LinePoints.clear();
		RandomPoints.clear();
		Triangulation.Clear();
		Triangles.clear();
		Edges.clear();
		MST.clear();
	}
	
	return true;
}

bool MCave::Generate() {
	if(!Map) {
		if(!AllocateMap()) {
			std::cout<<"Error while allocate map"<<std::endl;
			return false;
		}
	}
	else MLevel::Clear();
	
	//radomly fill map
	for(int i=1; i<TilesCount[0] - 1; i++) {
		for(int j=1; j<TilesCount[1] - 1; j++) {
			Map[i][j] = RandomChancePercent(RandomPercent);
		}
	}
	
	SmoothFloors();
	SmoothWalls();
	
	if(!CreateRegions()) {
		std::cout<<"Error while create regions"<<std::endl;
		return false;
	}
	if(!CreateHalls()) {
		std::cout<<"Error while create halls"<<std::endl;
		return false;
	}
	
	RoomsRegionsList.clear();
	WallsRegionsList.clear();
	
	return true;
}

void MCave::Clear() {
	RoomsRegionsList.clear();
	WallsRegionsList.clear();
	
	MLevel::Clear();
}

void MCave::Close() {
	Clear();
	MLevel::Close();
}

int MCave::GetType() {
	return LGT_CAVE;
}


