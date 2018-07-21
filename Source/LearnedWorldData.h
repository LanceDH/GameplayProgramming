#pragma once
#include "IExamPlugin.h"
#include "IExamInterface.h"
#include "EliteNavigation/ENavigation.h"

struct Box {
	Vector2 TL;
	Vector2 BR;
	Box() {};
	Box(Vector2 tl, Vector2 br) : TL(tl), BR(br) {};
	bool PointIsInBox(Vector2 pos) {
		bool a = pos.x > TL.x;
		bool b = pos.x < BR.x;
		bool c = pos.y > BR.y;
		bool d = pos.y < TL.y;
		return a && b && c && d;
	}
};

struct EnemyImage {
	Elite::Vector2 Position;
	float Size;
	float LastSeen = 0.f;
	float TimePerc;
};
struct LearnedHouse {
	float lastVisit;
	Elite::Vector2 Position;
	Elite::Vector2 Size;
	Elite::Vector2 SizeInside;
	Box Walls[8];
	LearnedHouse() {
		for (size_t i = 0; i < 8; i++) {
			Walls[i] = Box();
		};
	};
	bool PointsIsInHouse(Vector2 pos) {
		Vector2 halfsize = Size * 0.5f;
		return pos.x > Position.x - halfsize.x && pos.x < Position.x + halfsize.x
			&& pos.y > Position.y - halfsize.y && pos.y < Position.y + halfsize.y;
	}
};

class LearnedWorldData {
public:
	LearnedWorldData();
	~LearnedWorldData();
	void GenerateScanPoints(WorldInfo wi);
	void Draw(IExamInterface* pInterface);
	void Update(const float dTime, const AgentInfo& agentInfo);
	void AddBuilding(const HouseInfo hi, const float lastVisit);
	void AddThreat(const EnemyInfo ei);
	void AddItemToPickupList(EntityInfo ii);
	void RemoveItemFromPickupList(EntityInfo ii);
	void UpdateScanPoints(const AgentInfo &agenInfo);
	void EnteredHouse(float time, const AgentInfo &agentInfo);
	void LeftHouse(float time);

	bool PointIsInWall(Vector2 pos) const;
	bool HasScanPoints() const { return m_ScanPoints.size() > 0; };
	Vector2 GetNextScanPoint() const { return m_ScanPoints[m_ScanPoints.size() - 1]; };
	LearnedHouse* GetCurrentHouse() const { return m_CurrentBuilding; };
	LearnedHouse* GetOldestHouse() const;
	EntityInfo GetClosestItemToPickup(Vector2 location) const;
	int NumItemsToPickUp() const { return m_ItemsToPickUp.size(); }
	vector<EnemyImage> GetEnemyImages() const { return m_Threats; }
	vector<LearnedHouse*> GetHouses() const { return m_Buildings; };
	bool ThreatIsNew(const EnemyInfo ei);
	bool HouseIsNew(const HouseInfo hi);

private:
	const float ENEMY_SIZE_MULT = 3.f;
	const float ENEMY_LEEWAY = 2.f;
	const float ENEMY_LIFETIME = 1.5f;
	const float SCAN_SPACING = 50;
	const float SCAN_ACCEPTANCE = 30;
	const float HOUSE_WALL_THINKNESS = 2.f;

	bool m_HasGeneratedScanpoints = false;

	vector<EnemyImage> m_Threats = {};
	vector<LearnedHouse*> m_Buildings = {};
	LearnedHouse* m_CurrentBuilding = nullptr;

	std::vector<Vector2> m_ScanPoints;
	vector<EntityInfo> m_ItemsToPickUp;

	bool PosIsInHouse(Vector2 pos) const;
	
	bool TileIsInThreat(Vector2 pos) const;

	void UpdateWallsForDoor(Vector2 pos, float size = 0.5f);
};

