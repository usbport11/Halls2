#include "Dungeon.h"

MDungeon::MDungeon():MLevel() {
	MinLeafSize = 0;
	MaxLeafSize = 0;
	MinRoomSize = 0;
}

MDungeon::MDungeon(int TilesCountX, int TilesCountY, int inMinLeafSize, int inMaxLeafSize, int inMinRoomSize):MLevel(TilesCountX, TilesCountY) {
	MinLeafSize = 0;
	MaxLeafSize = 0;
	MinRoomSize = 0;
	if(inMinLeafSize > 0) MinLeafSize = inMinLeafSize;
	if(inMaxLeafSize > 0) MaxLeafSize = inMaxLeafSize;
	if(inMinRoomSize > 0) MinRoomSize = inMinRoomSize;
}

bool MDungeon::Generate() {
	if(!Map) {
		if(!AllocateMap()) return false;
	}
	else Clear();
	
	if(MinRoomSize < 3) {
		return false;
	}
	if(MinRoomSize >= MinLeafSize || MinLeafSize >= MaxLeafSize) {
		return false;
	}
	
	//create tree
	if(!SplitTree(&Tree, TilesCount[0], TilesCount[1], MinLeafSize, MaxLeafSize)) {
		return false;
	}
	
	if(!CreateRooms()) return false;
	if(!Triangulate()) return false;
	if(!CreateHalls()) return false;
	//if(!SetStartEndPoints()) return false;
	
	MST.clear();
	NodesCenters.clear();	
	ClearTree(&Tree);
	
	//disabled in this version
	//if(!ConvertToTiles()) return false;
	
	return true;
}

void MDungeon::Clear() {
	MST.clear();
	NodesCenters.clear();	
	ClearTree(&Tree);
	
	MLevel::Clear();
}

void MDungeon::Close() {
	Clear();
	MLevel::Close();
}

bool MDungeon::CreateRooms() {
	//create rooms and fill centers map
	TNode<NRectangle2>* pRoomNode;
	glm::vec2 Center;
	NRectangle2* pRoom;
	stLeaf* pLeaf = NULL;
	std::list<TNode<stLeaf>* >::iterator itl;
	int RoomsNumber = 0;
	for(itl = Tree.begin(); itl != Tree.end(); itl++) {
		pRoomNode = CreateRoomInLeaf(*itl, 0);
		//pRoomNode = CreateRoomInLeaf(*itl, MinRoomSize);
		if(!pRoomNode) continue;
		pRoom = pRoomNode->GetValueP();
		if(!pRoom) continue;
		if(pRoom->Size.x < 3 || pRoom->Size.y < 3) return false;
		//add in map
		Center.x = (float)(pRoom->Position.x + pRoom->Size.x * 0.5);
		Center.y = (float)(pRoom->Position.y + pRoom->Size.y * 0.5);
		NodesCenters.insert(std::pair<glm::vec2, TNode<NRectangle2>* >(Center, pRoomNode));
		//add room to level map grid
		FillMap(pRoom->Position.x, pRoom->Position.y, pRoom->Size.x, pRoom->Size.y, 1);
		RoomsNumber ++;
	}
	if(RoomsNumber < 2) {
		return false;
	}
	
	return true;
}

bool MDungeon::Triangulate() {
	//if rooms number less than 3 next steps useless
	if(NodesCenters.size() < 3) return false;
	
	//copy centers for triangulation
	std::map<glm::vec2, TNode<NRectangle2>*, stVec2Compare>::iterator itm;
	std::vector<glm::vec2> CentersPoints;
	for(itm = NodesCenters.begin(); itm != NodesCenters.end(); itm++) {
		CentersPoints.push_back(itm->first);
	}
	
	//triangulate by delaunay and get mst
	MDelaunay Triangulation;
	std::vector<MTriangle> Triangles = Triangulation.Triangulate(CentersPoints);
	std::vector<MEdge> Edges = Triangulation.GetEdges();
	MST = Triangulation.CreateMSTEdges();
	
	Triangulation.Clear();
	Triangles.clear();
	Edges.clear();
	CentersPoints.clear();
	
	if(MST.size() <= 0) return false;
	
	return true;
}

bool MDungeon::CreateHalls() {
	std::vector<NRectangle2> Halls;
	TNode<NRectangle2>* pNode0;
	TNode<NRectangle2>* pNode1;
	for(unsigned int i=0; i<MST.size(); i++) {
		pNode0 = NodesCenters[MST[i].p1];
		pNode1 = NodesCenters[MST[i].p2];
		Halls = CreateHalls5(pNode0->GetValueP(), pNode1->GetValueP());
		if(Halls.size() <= 0) return false;
		//connect rooms
		pNode0->AddConnection(pNode1);
		pNode1->AddConnection(pNode0);
		//add hall to level map grid
		for(unsigned int j=0; j<Halls.size(); j++) {
			FillMap(Halls[j].Position.x, Halls[j].Position.y, Halls[j].Size.x, Halls[j].Size.y, 1);
		}
		Halls.clear();
	}
	
	return true;
}

bool MDungeon::SetStartEndPoints() {
	std::map<glm::vec2, TNode<NRectangle2>*, stVec2Compare>::iterator itm;
	std::vector<glm::vec2> possibleStartPoints;
	unsigned int neighborsLimit = 3;
	int hopsLimit = 0;
	int maxHops = 0;
	int hops = 0;
	glm::vec2 randomStartPoint = glm::vec2(0, 0);
	glm::vec2 randomEndPoint = glm::vec2(0, 0);
	
	//find points that can be start points
	for(itm = NodesCenters.begin(); itm != NodesCenters.end(); itm++) {
		if(itm->second->Neighbors.size() >= neighborsLimit) possibleStartPoints.push_back(itm->first);
	}
	if(possibleStartPoints.size() <= 0) {
		return false;
	}
	
	//select random start point
	randomStartPoint = possibleStartPoints[rand() % possibleStartPoints.size()];
	
	//find end point with max hops count
	for(unsigned int i=0; i<possibleStartPoints.size(); i++) {
		if(randomStartPoint == possibleStartPoints[i]) continue;
		hops = HopsNumber(NodesCenters[randomStartPoint], NodesCenters[possibleStartPoints[i]]);
		if(hops > maxHops) {
			maxHops = hops;
			randomEndPoint = possibleStartPoints[i];
		}
	}
	possibleStartPoints.clear();
	
	if(randomStartPoint == glm::vec2(0, 0) || randomEndPoint == glm::vec2(0, 0)){
		return false;
	} 
	
	//simple method: set center of room (in case when room size smaller or equal to 3)
	//complex method: random set start/end point inside selected rooms, ignore edges (other cases)
	
	//start point
	if(NodesCenters[randomStartPoint]->GetValueP()->Size.x <= 3) 
		nStartPoint[0] = (int)randomStartPoint.x;
	else 
		nStartPoint[0] = NodesCenters[randomStartPoint]->GetValueP()->Position.x + rand() % (NodesCenters[randomStartPoint]->GetValueP()->Size.x - 2) + 1;
	if(NodesCenters[randomStartPoint]->GetValueP()->Size.y <= 3) 
		nStartPoint[1] = (int)randomStartPoint.y;
	else 
		nStartPoint[1] = NodesCenters[randomStartPoint]->GetValueP()->Position.y + rand() % (NodesCenters[randomStartPoint]->GetValueP()->Size.y - 2) + 1;
	
	//end point
	if(NodesCenters[randomEndPoint]->GetValueP()->Size.x <= 3) 
		nEndPoint[0] = (int)randomEndPoint.x;
	else 
		nEndPoint[0] = NodesCenters[randomEndPoint]->GetValueP()->Position.x + rand() % (NodesCenters[randomEndPoint]->GetValueP()->Size.x - 2) + 1;
	if(NodesCenters[randomEndPoint]->GetValueP()->Size.y <= 3) 
		nEndPoint[1] = (int)randomEndPoint.y;
	else 
		nEndPoint[1] = NodesCenters[randomEndPoint]->GetValueP()->Position.y + rand() % (NodesCenters[randomEndPoint]->GetValueP()->Size.y - 2) + 1;
	
	return true;
}

int MDungeon::GetType() {
	return LGT_DUNGEON;
}
