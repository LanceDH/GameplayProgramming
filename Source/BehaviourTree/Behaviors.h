#pragma once
#include "../stdafx.h"
#include "Blackboard.h"
#include "BehaviorTree.h"
//#include "../SteeringAgent.h"
#include "../SteeringBehaviours.h"
#include "Pipeline\SteeringPipeline_Implementation.h"
#include "../LearnedWorldData.h"

#pragma region Conditionals

bool HasTarget(Blackboard* pBlackboard) {
	// Get data
	bool hasTarget = false;
	bool dataAvailable = pBlackboard->GetData("TargetSet", hasTarget);
	if (!dataAvailable) {
		return false;
	}

	return hasTarget;
}

bool IsFarAway(Blackboard* pBlackboard) {
	// Get data
	AgentInfo* pAgentInfo;
	Vector2 targetPos = ZeroVector2;
	float closeToRadius = 0.f;
	bool dataAvailable = pBlackboard->GetData("Agent", pAgentInfo)
		&& pBlackboard->GetData("Target", targetPos)
		&& pBlackboard->GetData("CloseToRadius", closeToRadius);
	if (!dataAvailable || !pAgentInfo){
		return false;
	}

	float distance = Elite::Distance(pAgentInfo->Position, targetPos);
	
	return distance > closeToRadius;
}

bool IsBoxClosed(Blackboard* pBlackboard) {
	// Get data
	bool isOpen = false;
	bool dataAvailable = pBlackboard->GetData("IsBoxOpen", isOpen);
	if (!dataAvailable) {
		return false;
	}

	if (isOpen) {
		printf("	Already open! \n");
	}

	return !isOpen;
}

bool IsEnemyInSight(Blackboard* pBlackboard) {
	vector<EnemyInfo> enemiesInFov;

	vector<EntityInfo> entities = {};
	bool dataAvailable = pBlackboard->GetData("enemiesInFov", enemiesInFov);
	if (!dataAvailable) {
		return false;
	}

	return enemiesInFov.size() > 0;

}

bool IsEnemyImageInSight(Blackboard* pBlackboard) {
	bool enemyInSight = false;

	vector<EnemyImage> threats = {};
	bool dataAvailable = pBlackboard->GetData("EnemyImages", threats);
	if (!dataAvailable) {
		return false;
	}

	return threats.size() > 0;
}

bool IsNoEnemyImageInSight(Blackboard* pBlackboard) {
	return !IsEnemyImageInSight(pBlackboard);
}

bool SeesUnvisitedHouse(Blackboard* pBlackboard) {
	vector<Vector2> visitedHouses;
	vector<HouseInfo> housesInFOV;
	bool dataAvailable = pBlackboard->GetData("visitedHouses", visitedHouses)
		&& pBlackboard->GetData("housesInFOV", housesInFOV);
	if (!dataAvailable) {
		return false;
	}

	vector<Vector2> unvisitedHouses;

	for (HouseInfo h : housesInFOV) {
		bool isvisited = false;
		for (Vector2 pos : visitedHouses) {
			if (Distance(h.Center, pos) <= 0.01f) {
				isvisited = true;
				break;
			}
		}
		if (!isvisited) {
			unvisitedHouses.push_back(h.Center);
		}
	}

	pBlackboard->ChangeData("unvisitedHouses", unvisitedHouses);

	return unvisitedHouses.size() > 0;
}

bool HasNoHouseTarget(Blackboard* pBlackboard) {
	Vector2 pos;
	bool dataAvailable = pBlackboard->GetData("currentHouseTarget", pos);
	if (!dataAvailable) {
		return false;
	}

	return pos.x == FLT_MAX || pos.y == FLT_MAX;
}

bool HasHouseTarget(Blackboard* pBlackboard) {
	return !HasNoHouseTarget(pBlackboard);
}

bool IsItemInSight(Blackboard* pBlackboard) {
	vector<EntityInfo> items;
	bool dataAvailable = pBlackboard->GetData("itemsInSight", items);
	if (!dataAvailable) {
		return false;
	}

	return items.size() > 0;
}

bool IsCloseToHouseTargetCenter(Blackboard* pBlackboard) {
	Vector2 pos;
	AgentInfo agentInfo;
	float closeDist;
	bool dataAvailable = pBlackboard->GetData("currentHouseTarget", pos)
		&& pBlackboard->GetData("closeToHouseDist", closeDist)
		&& pBlackboard->GetData("agentInfo", agentInfo);
	if (!dataAvailable) {
		return false;
	}

	return Distance(pos, agentInfo.Position) <= closeDist;
}

bool HasGun(Blackboard* pBlackboard) {
	int gunSlot;
	bool dataAvailable = pBlackboard->GetData("gunSlot", gunSlot);
	if (!dataAvailable) {
		return false;
	}

	return gunSlot >= 0;
}

bool HasNoGun(Blackboard* pBlackboard) {
	return !HasGun(pBlackboard);
}

bool GunHasAmmo(Blackboard* pBlackboard) {
	int gunSlot;
	IExamInterface* pInterface = nullptr;
	bool dataAvailable = pBlackboard->GetData("gunSlot", gunSlot)
		&& pBlackboard->GetData("interface", pInterface);
	if (!dataAvailable && !pInterface) {
		return false;
	}

	ItemInfo ii;
	pInterface->Inventory_GetItem(gunSlot, ii);
	int ammo = pInterface->Item_GetMetadata(ii, "ammo");
	return ammo > 0;
}

bool GunHasNoAmmo(Blackboard* pBlackboard) {
	return !GunHasAmmo(pBlackboard);
}

bool HasFreeInventorySpot(Blackboard* pBlackboard) {
	UINT firstEmpty;
	UINT inventorySize;
	bool dataAvailable = pBlackboard->GetData("firstEmpty", firstEmpty)
		&& pBlackboard->GetData("inventorySize", inventorySize);
	if (!dataAvailable) {
		return false;
	}

	return firstEmpty < inventorySize;
}

bool PickedUpTrash(Blackboard* pBlackboard) {
	UINT firstEmpty;
	IExamInterface* pInterface = nullptr;
	bool dataAvailable = pBlackboard->GetData("firstEmpty", firstEmpty)
		&& pBlackboard->GetData("interface", pInterface);
	if (!dataAvailable && !pInterface) {
		return false;
	}

	ItemInfo ii;
	pInterface->Inventory_GetItem(firstEmpty, ii);

	return ii.Type == eItemType::GARBAGE;
}

bool HasItemTarget(Blackboard* pBlackboard) {
	//currentItemTarget
	EntityInfo closestItem;
	bool dataAvailable = pBlackboard->GetData("closestItem", closestItem);
	if (!dataAvailable) {
		return false;
	}

	return closestItem.Type != eEntityType::_LAST;
}

bool HasNoItemTarget(Blackboard* pBlackboard) {
	return !HasItemTarget(pBlackboard);
}

bool IsCloseToItem(Blackboard* pBlackboard) {
	EntityInfo closestItem;
	AgentInfo agentInfo;
	bool dataAvailable = pBlackboard->GetData("closestItem", closestItem)
		&& pBlackboard->GetData("agentInfo", agentInfo);
	if (!dataAvailable) {
		return false;
	}

	// * 0.5f because the engine can think we try to get that item right behind us we can't see, rather than the one right infront of us
	return Distance(closestItem.Location, agentInfo.Position) <= agentInfo.GrabRange *0.5f;
}

bool IsNotCloseToItem(Blackboard* pBlackboard) {
	return !IsCloseToItem(pBlackboard);
}

bool HasMedkit(Blackboard* pBlackboard) {
	int medkitSlot;
	bool dataAvailable = pBlackboard->GetData("medkitSlot", medkitSlot);
	if (!dataAvailable) {
		return false;
	}

	return medkitSlot >= 0;
}

bool ShouldUseMedkit(Blackboard* pBlackboard) {
	int medkitSlot;
	IExamInterface* pInterface = nullptr;
	AgentInfo agentInfo;
	bool dataAvailable = pBlackboard->GetData("medkitSlot", medkitSlot)
		&& pBlackboard->GetData("interface", pInterface)
		&& pBlackboard->GetData("agentInfo", agentInfo);
	if (!dataAvailable && !pInterface) {
		return false;
	}

	ItemInfo ii;
	pInterface->Inventory_GetItem(medkitSlot, ii);
	int health = pInterface->Item_GetMetadata(ii, "health");
	// <1.f failsafe, because on higher difficulties metkits can heal >= agent health
	return 10.f - agentInfo.Health  >= health || agentInfo.Health < 1.f;
}

bool HasFood(Blackboard* pBlackboard) {
	int foodSlot;
	bool dataAvailable = pBlackboard->GetData("foodSlot", foodSlot);
	if (!dataAvailable) {
		return false;
	}

	return foodSlot >= 0;
}

bool ShouldUseFood(Blackboard* pBlackboard) {
	int foodSlot;
	IExamInterface* pInterface = nullptr;
	AgentInfo agentInfo;
	bool dataAvailable = pBlackboard->GetData("foodSlot", foodSlot)
		&& pBlackboard->GetData("interface", pInterface)
		&& pBlackboard->GetData("agentInfo", agentInfo);
	if (!dataAvailable && !pInterface) {
		return false;
	}

	ItemInfo ii;
	pInterface->Inventory_GetItem(foodSlot, ii);
	int energy = pInterface->Item_GetMetadata(ii, "energy");
	return 10.f - agentInfo.Energy >= energy;
}

bool IsEnemyInLoS(Blackboard* pBlackboard) {
	vector<EnemyInfo> enemiesInFov;
	AgentInfo agentInfo;
	IExamInterface* pInterface = nullptr;
	int gunSlot;
	float aimAccuracy;
	bool dataAvailable = pBlackboard->GetData("enemiesInFov", enemiesInFov)
		&& pBlackboard->GetData("agentInfo", agentInfo)
		&& pBlackboard->GetData("interface", pInterface)
		&& pBlackboard->GetData("gunSlot", gunSlot)
		&& pBlackboard->GetData("aimAccuracy", aimAccuracy);
	if (!dataAvailable || !pInterface) {
		return false;
	}

	ItemInfo ii;
	pInterface->Inventory_GetItem(gunSlot, ii);
	float range = pInterface->Item_GetMetadata(ii, "range");

	// https://stackoverflow.com/questions/1073336/circle-line-segment-collision-detection-algorithm
	
	bool enemyInLos = false;
	Vector2 toCenter;
	//Vector2 straight;
	Vector2 dotPoint;

	EnemyInfo bestTurnTarget;
	bestTurnTarget.Type = eEnemyType::_LAST;
	float shortestTurn = FLT_MIN;
	int turnDelta = 0;

	float halfPi = 1.57075f;
	Vector2 straight = Vector2(cos(agentInfo.Orientation - halfPi), sin(agentInfo.Orientation - halfPi));
	Normalize(straight);
	straight *= range;

#ifdef DRAWDEBUG
	pInterface->Draw_Segment(agentInfo.Position, agentInfo.Position + straight, Vector3(0, 1, 0), 0);
#endif // DRAWDEBUG



	for (EnemyInfo ei: enemiesInFov) {
		toCenter = agentInfo.Position - ei.Location ;
		Normalize(straight);
		straight *= range;

		float r = (ei.Size * ei.Size) * aimAccuracy; // Need to make the circle smaller because the engine can't aim straight
		float a = straight.Dot(straight);
		float b = 2 * toCenter.Dot(straight);
		float c = toCenter.Dot(toCenter) - r;
		float discriminant = b*b - 4 * a*c;
		
#ifdef DRAWDEBUG
		float visDisc = abs(discriminant / 10000);
		if (visDisc <= 15.f) {
			Vector3 col = Vector3(1 - visDisc / 15.f, visDisc / 15.f, 0);
			pInterface->Draw_Circle(ei.Location, visDisc, col, 0);
		}
#endif // DRAWDEBUG
		
		if (discriminant > 0) {
			cout << "		Target in LoS!" << endl;
			enemyInLos = true;
		}
		else if (discriminant < shortestTurn) {
			shortestTurn = discriminant;
			bestTurnTarget = ei;
			turnDelta = (Cross(straight, ei.Location - agentInfo.Position) > 0) ? 1 : -1;
		}
	}

	pBlackboard->ChangeData("targetInLoS", enemyInLos);
	pBlackboard->ChangeData("bestTurnTarget", bestTurnTarget);
	pBlackboard->ChangeData("turnDelta", turnDelta);

	//cout << "NotInLoS" << endl;

	return enemyInLos;

}

bool IsNoEnemyInLoS(Blackboard* pBlackboard) {
	return !IsEnemyInLoS(pBlackboard);
}

bool HasTooManyGuns(Blackboard* pBlackboard) {
	int numOfGuns;
	int numOfMedkits;
	int numOfFood;
	bool dataAvailable = pBlackboard->GetData("numOfGuns", numOfGuns)
		&& pBlackboard->GetData("numOfMedkits", numOfMedkits)
		&& pBlackboard->GetData("numOfFood", numOfFood);
	if (!dataAvailable) {
		return false;
	}

	return (numOfGuns > numOfFood && numOfGuns > numOfMedkits) 
		|| (numOfGuns == numOfFood && numOfGuns > numOfMedkits) 
		|| (numOfGuns == numOfMedkits && numOfGuns > numOfFood);
}

bool HasTooManyMedkits(Blackboard* pBlackboard) {
	int numOfGuns;
	int numOfMedkits;
	int numOfFood;
	bool dataAvailable = pBlackboard->GetData("numOfGuns", numOfGuns)
		&& pBlackboard->GetData("numOfMedkits", numOfMedkits)
		&& pBlackboard->GetData("numOfFood", numOfFood);
	if (!dataAvailable) {
		return false;
	}

	return (numOfMedkits > numOfFood && numOfMedkits > numOfGuns) 
		|| (numOfMedkits == numOfFood && numOfMedkits > numOfGuns)
		|| (numOfMedkits == numOfGuns && numOfMedkits > numOfFood);
}

bool HasTooMuchFood(Blackboard* pBlackboard) {
	int numOfGuns;
	int numOfMedkits;
	int numOfFood;
	bool dataAvailable = pBlackboard->GetData("numOfGuns", numOfGuns)
		&& pBlackboard->GetData("numOfMedkits", numOfMedkits)
		&& pBlackboard->GetData("numOfFood", numOfFood);
	if (!dataAvailable) {
		return false;
	}

	return (numOfFood > numOfGuns && numOfFood > numOfGuns)
		|| (numOfFood == numOfMedkits && numOfFood > numOfGuns)
		|| (numOfFood == numOfGuns && numOfFood > numOfMedkits);
}

bool InventoryIsFull(Blackboard* pBlackboard) {
	return !HasFreeInventorySpot(pBlackboard);
}

bool HasItemRemembered(Blackboard* pBlackboard) {
	LearnedWorldData* m_WorldData = nullptr;
	bool dataAvailable = pBlackboard->GetData("worldData", m_WorldData);
	if (!dataAvailable || !m_WorldData) {
		return false;
	}

	cout << "I 'member" << endl;
	return m_WorldData->NumItemsToPickUp() > 0;
}

bool HasCustomTarget(Blackboard* pBlackboard) {
	Vector2 target;
	bool dataAvailable = pBlackboard->GetData("customTarget", target);
	if (!dataAvailable) {
		return false;
	}
	return target.x != FLT_MAX || target.y != FLT_MAX;
}

bool HasNoCustomTarget(Blackboard* pBlackboard) {
	return !HasCustomTarget(pBlackboard);
}

bool ScanCompleted(Blackboard* pBlackboard) {
	bool scanningComplete;
	bool dataAvailable = pBlackboard->GetData("scanningComplete", scanningComplete);
	if (!dataAvailable) {
		return false;
	}
	return scanningComplete;
}

bool IsClosetoCustomTarget(Blackboard* pBlackboard) {
	Vector2 target;
	AgentInfo agentInfo;
	float customDist;
	bool dataAvailable = pBlackboard->GetData("customTarget", target)
		&& pBlackboard->GetData("agentInfo", agentInfo)
		&& pBlackboard->GetData("customTargetDistance", customDist);
	if (!dataAvailable) {
		return false;
	}
	return Distance(target, agentInfo.Position) <= customDist;
}

#pragma endregion Conditionals


//----------------------
// Behaviours
//----------------------

#pragma region Behaviours

BehaviorState ResetCustomTarget(Blackboard* pBlackboard) {
	bool dataAvailable = pBlackboard->ChangeData("customTarget", Vector2(FLT_MAX, FLT_MAX))
		&& pBlackboard->ChangeData("visitedHouses", vector<Vector2>());
	if (!dataAvailable) {
		return Failure;
	}
	cout << "ResetCustomTarget" << endl;
	return Success;
}

BehaviorState SetCustomTargetToOldestHouse(Blackboard* pBlackboard) {
	LearnedWorldData* m_WorldData = nullptr;
	bool dataAvailable = pBlackboard->GetData("worldData", m_WorldData);
	if (!dataAvailable || !m_WorldData) {
		return Failure;
	}

	cout << "Visiting oldest house" << endl;
	pBlackboard->ChangeData("customTarget", m_WorldData->GetOldestHouse()->Position);

	return Success;
}

BehaviorState SetItemTargetToClosestRemembered(Blackboard* pBlackboard) {
	EntityInfo closestItem;
	closestItem.Type = eEntityType::_LAST;
	pBlackboard->ChangeData("closestItem", closestItem);

	return Success;
}

BehaviorState ClearClosestItem(Blackboard* pBlackboard) {
	EntityInfo closestItem;
	closestItem.Type = eEntityType::_LAST;
	pBlackboard->ChangeData("closestItem", closestItem);

	return Success;
}

BehaviorState PickupItem(Blackboard* pBlackboard) {
	vector<ItemInfo>* pInventory = nullptr;
	LearnedWorldData* m_WorldData = nullptr;
	UINT firstEmpty;
	IExamInterface* pInterface = nullptr;
	EntityInfo closestItem;
	bool dataAvailable = pBlackboard->GetData("firstEmpty", firstEmpty)
		&& pBlackboard->GetData("worldData", m_WorldData)
		&& pBlackboard->GetData("inventory", pInventory)
		&& pBlackboard->GetData("interface", pInterface)
		&& pBlackboard->GetData("closestItem", closestItem);

	if (!dataAvailable || !m_WorldData || !pInventory || !pInterface) {
		cout << "Premature fail: PickupItem" << endl;
		return Failure;
	}

	ItemInfo ii;
	if (!pInterface->Item_Grab(closestItem, ii)) {
		cout << "Failed to grab item" << endl;
		return Failure;
	}
	if (!pInterface->Inventory_AddItem(firstEmpty, ii)) {
		cout << "Failed to pickup item" << endl;
	}

	m_WorldData->RemoveItemFromPickupList(closestItem);

	pInventory->at(firstEmpty) = ii;

	//cout << "Picked up item" << endl;
	if (ii.Type == eItemType::PISTOL) {
		int ammo = pInterface->Item_GetMetadata(ii, "ammo");
		float range = pInterface->Item_GetMetadata(ii, "range");
		cout << "		Gun " << ammo << " | " << range << endl;
	}
	else if (ii.Type == eItemType::FOOD){
			int energy = pInterface->Item_GetMetadata(ii, "energy");
			cout << "		Food " << energy << endl;
	}
	else if (ii.Type == eItemType::MEDKIT) {
			int health = pInterface->Item_GetMetadata(ii, "health");
			cout << "		Medkit " << health << endl;
	}
	else if (ii.Type == eItemType::GARBAGE) {
			cout << "		Trash" << endl;
	}

	if (ii.Type == eItemType::GARBAGE) {
		cout << "			Trash the trash" << endl;
		pInterface->Inventory_RemoveItem(firstEmpty);
		pInventory->at(firstEmpty).Type = eItemType::_LAST;
	}

	return Success;
}

BehaviorState UseFood(Blackboard* pBlackboard) {
	int foodSlot;
	IExamInterface* pInterface = nullptr;
	vector<ItemInfo>* pInventory = nullptr;
	bool dataAvailable = pBlackboard->GetData("foodSlot", foodSlot)
		&& pBlackboard->GetData("inventory", pInventory)
		&& pBlackboard->GetData("interface", pInterface);
	if (!dataAvailable || !pInterface || !pInventory) {
		cout << "Premature fail: UseFood" << endl;
		return Failure;
	}

	if (!pInterface->Inventory_UseItem(foodSlot)) {
		cout << "Failed to use food" << endl;
	}

	cout << "Deleted food" << endl;
	pInterface->Inventory_RemoveItem(foodSlot);
	pInventory->at(foodSlot).Type = eItemType::_LAST;

	return Success;
}

BehaviorState UseMedkit(Blackboard* pBlackboard) {
	int medkitSlot;
	IExamInterface* pInterface = nullptr;
	vector<ItemInfo>* pInventory = nullptr;
	bool dataAvailable = pBlackboard->GetData("medkitSlot", medkitSlot)
		&& pBlackboard->GetData("inventory", pInventory)
		&& pBlackboard->GetData("interface", pInterface);
	if (!dataAvailable || !pInterface || !pInventory) {
		cout << "Premature fail: UseMedkit" << endl;
		return Failure;
	}

	if (medkitSlot < 0) {
		cout << "			-- Tried using medkit with slot -1 --" << endl;
		return Failure;
	}

	if (!pInterface->Inventory_UseItem(medkitSlot)) {
		cout << "Failed to use medkit" << endl;
	}

	cout << "Deleted medkit" << endl;
	pInterface->Inventory_RemoveItem(medkitSlot);
	pInventory->at(medkitSlot).Type = eItemType::_LAST;

	return Success;
}

BehaviorState ShootGun(Blackboard* pBlackboard) {
	int gunSlot;
	vector<ItemInfo>* pInventory = nullptr;
	IExamInterface* pInterface = nullptr;
	bool dataAvailable = pBlackboard->GetData("gunSlot", gunSlot)
		&& pBlackboard->GetData("inventory", pInventory)
		&& pBlackboard->GetData("interface", pInterface);
	if (!dataAvailable || !pInterface) {
		cout << "Premature fail: ShootGun" << endl;
		return Failure;
	}
	
	if (!pInterface->Inventory_UseItem(gunSlot)) {
		cout << "Failed to shoot gun? " << endl;
	} 

	if (GunHasNoAmmo(pBlackboard)) {
		cout << "Deleted gun" << endl;
		pInterface->Inventory_RemoveItem(gunSlot);
		pInventory->at(gunSlot).Type = eItemType::_LAST;
	}

	return Success;
}

BehaviorState DropGun(Blackboard* pBlackboard) {
	int gunSlot;
	vector<ItemInfo>* pInventory = nullptr;
	IExamInterface* pInterface = nullptr;
	
	bool dataAvailable = pBlackboard->GetData("gunSlot", gunSlot)
		&& pBlackboard->GetData("inventory", pInventory)
		&& pBlackboard->GetData("interface", pInterface);
	if (!dataAvailable || !pInterface || !pInventory) {
		cout << "Premature fail: DropGun" << endl;
		return Failure;
	}

	cout << "Deleted gun" << endl;
	pInterface->Inventory_RemoveItem(gunSlot);
	pInventory->at(gunSlot).Type = eItemType::_LAST;

	return Success;
}

BehaviorState DropTrash(Blackboard* pBlackboard) {
	UINT firstEmpty;
	vector<ItemInfo>* pInventory = nullptr;
	IExamInterface* pInterface = nullptr;
	bool dataAvailable = pBlackboard->GetData("firstEmpty", firstEmpty)
		&& pBlackboard->GetData("inventory", pInventory)
		&& pBlackboard->GetData("interface", pInterface);
	if (!dataAvailable || !pInterface || !pInventory) {
		return Failure;
	}
	cout << "Deleted trash" << endl;
	// Everyone gather around for a show. Watch as this trash dissapears as we know.
	// Do me a favor and try to ignore. As you watch it fall through a blatant trapdoor.
	pInterface->Inventory_RemoveItem(firstEmpty);
	pInventory->at(firstEmpty).Type = eItemType::_LAST;
	return Success;
}

BehaviorState SetTargetHouse(Blackboard* pBlackboard) {
	// Get data
	vector<Vector2> unvisitedHouses;
	bool datavAilable = pBlackboard->GetData("unvisitedHouses", unvisitedHouses);
	if (!datavAilable) {
		return Failure;
	}

	printf("	Setting house target\n");

	pBlackboard->ChangeData("currentHouseTarget", unvisitedHouses[0]);
	return Success; 
}

BehaviorState ChangeToSeek(Blackboard* pBlackboard) {
	// Get data
	ISteeringBehaviour* pSeekBehavior = nullptr;
	ISteeringBehaviour* pCurrentBehaviour = nullptr;
	TargetData* pTargetData = nullptr;
	bool datavAilable = pBlackboard->GetData("seekBehaviour", pSeekBehavior)
		&& pBlackboard->GetData("currentBehaviour", pCurrentBehaviour)
		&& pBlackboard->GetData("targetData", pTargetData);
	if (!datavAilable) {
		return Failure;
	}

	if (!pSeekBehavior || !pCurrentBehaviour || !pTargetData) {
		return Failure;
	}

	pSeekBehavior->As<Seek>()->SetTarget(pTargetData);

	if (pCurrentBehaviour != pSeekBehavior) {
		printf("Get over here \n");
		pBlackboard->ChangeData("currentBehaviour", pSeekBehavior);
	}

	return Success;
}

BehaviorState ChangeToWander(Blackboard* pBlackboard) {
	// Get data
	ISteeringBehaviour* pWanderBehavior = nullptr;
	ISteeringBehaviour* pCurrentBehaviour = nullptr;
	bool datavAilable = pBlackboard->GetData("wanderBehaviour", pWanderBehavior)
		&& pBlackboard->GetData("currentBehaviour", pCurrentBehaviour);
	if (!datavAilable) {
		return Failure;
	}

	if (!pWanderBehavior || !pCurrentBehaviour) {
		return Failure;
	}

	if (pCurrentBehaviour != pWanderBehavior) {
		printf("Free to do nothing \n");
		pBlackboard->ChangeData("currentBehaviour", pWanderBehavior);
	}

	return Success;
}

BehaviorState ChangeToFlee(Blackboard* pBlackboard) {
	// Get data
	ISteeringBehaviour* pFleeBehavior = nullptr;
	ISteeringBehaviour* pCurrentBehaviour = nullptr;
	vector<EnemyImage> threats = {};
	AgentInfo agentInfo;
	TargetData* pTargetData = nullptr;
	bool datavAilable = pBlackboard->GetData("fleeBehaviour", pFleeBehavior)
		&& pBlackboard->GetData("EnemyImages", threats)
		&& pBlackboard->GetData("agentInfo", agentInfo)
		&& pBlackboard->GetData("targetData", pTargetData)
		&& pBlackboard->GetData("currentBehaviour", pCurrentBehaviour);
	if (!datavAilable) {
		return Failure;
	}

	if (!pTargetData || !pFleeBehavior || !pCurrentBehaviour || threats.size() == 0) {
		return Failure;
	}

	EnemyImage closest = threats[0];
	float closestDist = Distance(closest.Position, agentInfo.Position);
	for (size_t i = 1; i < threats.size(); i++) {
		float dist = Distance(closest.Position, threats[i].Position);
		if (dist < closestDist) {
			closestDist = dist;
			closest = threats[i];
		}
	}

	pTargetData->Position = closest.Position;

	pFleeBehavior->As<Flee>()->SetTarget(pTargetData);


	if (pCurrentBehaviour != pFleeBehavior) {
		printf("Run away! \n");
		pBlackboard->ChangeData("currentBehaviour", pFleeBehavior);
	}

	return Success;
}

BehaviorState ChangeToAvoidEnemy(Blackboard* pBlackboard) {
	// Get data
	ISteeringBehaviour* pAvoidEnemyBehaviour = nullptr;
	ISteeringBehaviour* pCurrentBehaviour = nullptr;
	vector<EntityInfo> entities = {};
	IExamInterface* pInterface = nullptr;
	vector<EnemyImage> threats = {};
	LearnedWorldData* m_WorldData = nullptr;
	bool datavAilable = pBlackboard->GetData("avoidEnemyBehaviour", pAvoidEnemyBehaviour)
		&& pBlackboard->GetData("currentBehaviour", pCurrentBehaviour)
		&& pBlackboard->GetData("entityiesInFov", entities)
		&& pBlackboard->GetData("interface", pInterface)
		&& pBlackboard->GetData("EnemyImages", threats)
		&& pBlackboard->GetData("worldData", m_WorldData);
	if (!datavAilable || !m_WorldData) {
		return Failure;
	}

	if (!pAvoidEnemyBehaviour || !pCurrentBehaviour || !pInterface) {
		return Failure;
	}

	AvoidEnemyContraint* m_pConstraint = new AvoidEnemyContraint(threats, m_WorldData);
	static_cast<SteeringPipeline*>(pAvoidEnemyBehaviour)->SetConstraints({m_pConstraint});

	if (pCurrentBehaviour != pAvoidEnemyBehaviour) {
		printf("Dodging threats \n");
		pBlackboard->ChangeData("currentBehaviour", pAvoidEnemyBehaviour);
	}

	return Success;
}

BehaviorState ClearHouseTarget(Blackboard* pBlackboard) {
	Vector2 targetPos;
	vector<Vector2> visitedHouses;
	bool datavAilable = pBlackboard->GetData("currentHouseTarget", targetPos)
		&& pBlackboard->GetData("visitedHouses", visitedHouses);;
	if (!datavAilable) {
		return Failure;
	}

	visitedHouses.push_back(targetPos);

	pBlackboard->ChangeData("visitedHouses", visitedHouses);
	pBlackboard->ChangeData("currentHouseTarget", Vector2(FLT_MAX, FLT_MAX));

	return Success;
}

#pragma endregion Behaviours
