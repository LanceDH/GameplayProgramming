#pragma once
#include "IExamPlugin.h"
class WorldInfo;

struct MapTile;

struct MapConnection {
	MapTile* Start;
	MapTile* End;
	float distance;
};

struct MapTile {
	int Row;
	int Col;
	Elite::Vector2 Position;
	vector<MapConnection> Connections;

	MapTile() {
		Connections = vector<MapConnection>();
	}
};

class InfluenceMap {
public:
	InfluenceMap(WorldInfo worldInfo);
	~InfluenceMap();

private:
	static const int HEIGHT = 100;
	static const int WIDTH = 100;

	WorldInfo m_WorldInfo = {};
	MapTile* m_MapBase[100][100];

	void AddSurroundingConnections(MapTile* tile);
};

