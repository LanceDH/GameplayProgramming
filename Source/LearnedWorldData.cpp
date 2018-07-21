#include "stdafx.h"
#include "LearnedWorldData.h"


LearnedWorldData::LearnedWorldData() {
}

LearnedWorldData::~LearnedWorldData() {

	for (LearnedHouse* house : m_Buildings) {
		SAFE_DELETE(house);
	}
}

void LearnedWorldData::GenerateScanPoints(WorldInfo wi) {
	if (m_HasGeneratedScanpoints) {
		return;
	}

	Vector2 dim = wi.Dimensions * 0.5f;
	float posX = SCAN_SPACING;
	float posY = dim.y - SCAN_SPACING*0.5f;
	bool right = true;
	while (posY > -dim.y) {
		if (right) {
			posX = -dim.x + SCAN_SPACING*0.5f;
			while (posX < dim.x) {
				m_ScanPoints.push_back(Vector2(posX, posY));
				posX += SCAN_SPACING;
			}
			right = false;
		}
		else {
			posX = +dim.x - SCAN_SPACING*0.5f;
			while (posX > -dim.x) {
				
				m_ScanPoints.push_back(Vector2(posX, posY));
				posX -= SCAN_SPACING;
			}
			right = true;
		}
		posY -= SCAN_SPACING;
	}

	m_HasGeneratedScanpoints = true;
}

void LearnedWorldData::Draw(IExamInterface * pInterface) {
#ifdef DRAWSCAN
	// Draw scan circles
	float index = 0;
	int size = m_ScanPoints.size();
	for (Vector2 pos : m_ScanPoints) {
		pInterface->Draw_Circle(pos, SCAN_SPACING*0.5f, Vector3(index/size, 1, index / size));
		index+= 1.f;
	}
#endif // DRAWSCAN

#ifdef DRAWDEBUG
	// Draw buildings
	for (LearnedHouse* b : m_Buildings) {
		for (size_t i = 0; i < 8; i++) {
			Box wall = b->Walls[i];

			pInterface->Draw_Segment(wall.TL, Vector2(wall.BR.x, wall.TL.y), {0,1,1});
			pInterface->Draw_Segment(Vector2(wall.BR.x, wall.TL.y), wall.BR, {0,1,1});
			pInterface->Draw_Segment(wall.BR, Vector2(wall.TL.x, wall.BR.y), {0,1,1});
			pInterface->Draw_Segment(Vector2(wall.TL.x, wall.BR.y), wall.TL, {0,1,1});
		}
	}

	// Draw temporary threats
	for (EnemyImage e : m_Threats) {
		pInterface->Draw_Circle(e.Position, e.Size, {1, e.TimePerc, e.TimePerc});
	}
#endif // DRAWDEBUG
}

void LearnedWorldData::Update(const float dTime, const AgentInfo& agentInfo) {
	UpdateScanPoints(agentInfo);

	for (EnemyImage &e : m_Threats) {
		e.LastSeen += dTime;
		e.TimePerc = e.LastSeen / ENEMY_LIFETIME;
	}

	float lifetime = ENEMY_LIFETIME;
	m_Threats.erase(std::remove_if(m_Threats.begin(), m_Threats.end(), [lifetime](EnemyImage &e) {return e.LastSeen >= lifetime; }), m_Threats.end());

	if (agentInfo.IsInHouse && PointIsInWall(agentInfo.Position)) {
		UpdateWallsForDoor(agentInfo.Position, agentInfo.AgentSize);
	}
}

void LearnedWorldData::AddBuilding(const HouseInfo hi, const float lastVisit) {
	if (!HouseIsNew(hi)) {
		return;
	}

	LearnedHouse* b = new LearnedHouse();
	b->Position = hi.Center;
	b->Size = hi.Size;
	
	float x = b->Position.x;
	float y = b->Position.y;
	float halfWidth = b->Size.x / 2;
	float halfHeight = b->Size.y / 2;

	b->Walls[0] = Box(Vector2(x - halfWidth, y + halfHeight), Vector2(x + halfWidth, y + halfHeight - HOUSE_WALL_THINKNESS));
	b->Walls[1] = Box(Vector2(x - halfWidth, y + halfHeight), Vector2(x + halfWidth, y + halfHeight - HOUSE_WALL_THINKNESS));
	b->Walls[2] = Box(Vector2(x + halfWidth - HOUSE_WALL_THINKNESS, y + halfHeight), Vector2(x + halfWidth, y - halfHeight));
	b->Walls[3] = Box(Vector2(x + halfWidth - HOUSE_WALL_THINKNESS, y + halfHeight), Vector2(x + halfWidth, y - halfHeight));
	b->Walls[4] = Box(Vector2(x - halfWidth, y - halfHeight + HOUSE_WALL_THINKNESS), Vector2(x + halfWidth, y - halfHeight));
	b->Walls[5] = Box(Vector2(x - halfWidth, y - halfHeight + HOUSE_WALL_THINKNESS), Vector2(x + halfWidth, y - halfHeight));
	b->Walls[6] = Box(Vector2(x - halfWidth, y + halfHeight), Vector2(x - halfWidth + HOUSE_WALL_THINKNESS, y - halfHeight));
	b->Walls[7] = Box(Vector2(x - halfWidth, y + halfHeight), Vector2(x - halfWidth + HOUSE_WALL_THINKNESS, y - halfHeight));


	b->SizeInside = b->Size - Elite::Vector2(4, 4);

	m_Buildings.push_back(b);
}

bool LearnedWorldData::HouseIsNew(const HouseInfo hi) {
	for (LearnedHouse* b : m_Buildings) {
		if (Elite::Distance(hi.Center, b->Position) < 0.001f) {
			return false;
		}
	}

	return true;
}

void LearnedWorldData::AddThreat(const EnemyInfo ei) {
	EnemyImage e;
	e.Position = ei.Location;
	e.Size = ei.Size * ENEMY_SIZE_MULT;

	m_Threats.push_back(e);
}

bool LearnedWorldData::ThreatIsNew(const EnemyInfo ei) {
	for (EnemyImage &e : m_Threats) {
		if (Elite::Distance(ei.Location, e.Position) < ENEMY_LEEWAY) {
			e.Position = ei.Location;
			return false;
		}
	}

	return true;
}

void LearnedWorldData::AddItemToPickupList(EntityInfo ii) {
	for (EntityInfo other : m_ItemsToPickUp) {
		if (abs(ii.Location.x - other.Location.x) < 0.001f && abs(ii.Location.y - other.Location.y) < 0.001f) {
			// This item is already in the list
			return;
		}
	}

	cout << "Item to remember list" << endl;
	m_ItemsToPickUp.push_back(ii);
}

void LearnedWorldData::RemoveItemFromPickupList(EntityInfo ii) {
	int toRemove = -1;
	for (size_t i = 0; i < m_ItemsToPickUp.size(); i++) {
		EntityInfo other = m_ItemsToPickUp[i];
		if (abs(ii.Location.x - other.Location.x) < 0.001f && abs(ii.Location.y - other.Location.y) < 0.001f) {
			toRemove = i;
			break;
		}
	}
	
	if (toRemove >= 0) {
		cout << "Item from remember list" << endl;
		m_ItemsToPickUp.erase(m_ItemsToPickUp.begin() + toRemove);
	}
	
}

EntityInfo LearnedWorldData::GetClosestItemToPickup(Vector2 location) const {
	EntityInfo ei;
	ei.Type = eEntityType::_LAST;
	float closestDist = FLT_MAX;

	for (EntityInfo other : m_ItemsToPickUp) {
		float dist = Distance(location, other.Location);
		if (dist < closestDist) {
			closestDist = dist;
			ei = other;
		}
	}
	
	return ei;
}

bool LearnedWorldData::PosIsInHouse(Vector2 pos) const {
	for (LearnedHouse* house : m_Buildings) {
		Vector2 halfsize = house->Size *0.5f;
		if (pos.x > house->Position.x - halfsize.x && pos.x < house->Position.x + halfsize.x
			&& pos.y > house->Position.y - halfsize.y && pos.y < house->Position.y + halfsize.y) {
			return true;
		}
	}

	return false;
}

bool LearnedWorldData::PointIsInWall(Vector2 pos) const {
	for (LearnedHouse* house : m_Buildings) {
		for (size_t i = 0; i < 8; i++) {
			Box wall = house->Walls[i];
			if (wall.PointIsInBox(pos)) {
				return true;
			}
		}
	}
	return false;
}

void LearnedWorldData::UpdateScanPoints(const AgentInfo & agenInfo) {
	Vector2 agentPos = agenInfo.Position;
	float halfSpace = SCAN_SPACING * 0.5f;
	int index = -1;
	for (size_t i = 0; i < m_ScanPoints.size(); i++) {
		Vector2 pos = m_ScanPoints[i];
		if (agentPos.x > pos.x - halfSpace && agentPos.y > agentPos.y - halfSpace
			&& agentPos.x < pos.x + halfSpace && agentPos.y < agentPos.y + halfSpace) {
			if (Distance(agentPos, pos) < SCAN_ACCEPTANCE * 0.5f) {
				index = i;
				break;
			}
		}
	}

	if (index >= 0) {
		m_ScanPoints.erase(m_ScanPoints.begin() + index);
	}
	
}

void LearnedWorldData::EnteredHouse(float time, const AgentInfo & agentInfo) {
	for (LearnedHouse* pHouse : m_Buildings) {
		if (pHouse->PointsIsInHouse(agentInfo.Position)) {
			cout << "Found entered house" << endl;
			m_CurrentBuilding = pHouse;
			m_CurrentBuilding->lastVisit = time;
			return;
		}
	}
}

void LearnedWorldData::LeftHouse(float time) {
	if (m_CurrentBuilding) {
		cout << "Updated visit time" << endl;
		m_CurrentBuilding->lastVisit = time;
	}
}


LearnedHouse * LearnedWorldData::GetOldestHouse() const {
	if (m_Buildings.size() == 0) {
		return nullptr;
	}
	
	LearnedHouse* oldest = m_Buildings[0];
	for (LearnedHouse* pHouse : m_Buildings) {
		if (pHouse->lastVisit < oldest->lastVisit) {
			oldest = pHouse;
		}
	}

	return oldest;
}

bool LearnedWorldData::TileIsInThreat(Vector2 pos) const {
	for (EnemyImage threat : m_Threats) {
		if (Distance(threat.Position, pos) <= threat.Size+0.5f) {
			return true;
		}
	}
	return false;
}

void LearnedWorldData::UpdateWallsForDoor(Vector2 pos, float size) {
	for (LearnedHouse* house : m_Buildings) {
		if (PosIsInHouse(pos)) {
			for (size_t i = 0; i < 8; i++) {
				Box& wall = house->Walls[i];
				if (wall.PointIsInBox(pos)) {
					switch (i) {
						case 0:
						case 5:
							wall.BR.x = min(wall.BR.x, pos.x - size);
							break;
						case 1:
						case 4:
							wall.TL.x = max(wall.TL.x, pos.x + size);
							break;
						case 2:
						case 7:
							wall.BR.y = max(wall.BR.y, pos.y + size);
							break;
						case 3:
						case 6:
							wall.TL.y = min(wall.TL.y, pos.y - size);
							break;

						default:
							break;
					}
				}
			}
		}
	}

}