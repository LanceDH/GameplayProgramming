#include "../stdafx.h"
#include "InfluenceMap.h"
#include "IExamInterface.h"

InfluenceMap::InfluenceMap(WorldInfo worldInfo) {
	m_WorldInfo = worldInfo;

	for (int row = 0; row < HEIGHT - 1; row++) {
		for (int col = 0; col < WIDTH - 1; col++) {
			MapTile* tile = new MapTile();
			tile->Row = row;
			tile->Col = col;
			m_MapBase[row][col] = tile;
		}
	}

	for (int row = 0; row < HEIGHT - 1; row++) {
		for (int col = 0; col < WIDTH - 1; col++) {
			AddSurroundingConnections(m_MapBase[row][col]);
		}
	}
}

InfluenceMap::~InfluenceMap() {
	for (int row = 0; row < HEIGHT - 1; row++) {
		for (int col = 0; col < WIDTH - 1; col++) {
			SAFE_DELETE(m_MapBase[row][col]);
		}
	}
}

void InfluenceMap::AddSurroundingConnections(MapTile * tile) {
	for (size_t i = -1; i <= 1; i++) {
		for (size_t j = -1; j <= 1; j++) {
			int col = tile->Col + i;
			int row = tile->Row + j;
			if (!(i==0 && j==0) && col >= 0 && col < WIDTH && row >= 0 && row < HEIGHT) {
				MapConnection con;
				con.Start = tile;
				con.End = m_MapBase[tile->Col + i][tile->Row + j];
				con.distance = sqrt(i*i + j*j);
				tile->Connections.push_back(con);
			}
		}
	}
}