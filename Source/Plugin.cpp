#include "stdafx.h"
#include "Plugin.h"
#include "IExamInterface.h"
#include "SteeringBehaviours.h"
#include "BehaviourTree\Behaviors.h"
#include "Pipeline\SteeringPipeline_Implementation.h"
#include "LearnedWorldData.h"

//Called only once, during initialization
void Plugin::Initialize(IBaseInterface* pInterface, PluginInfo& info) {
	//_crtBreakAlloc = 178;

	//Retrieving the interface
	//This interface gives you access to certain actions the AI_Framework can perform for you
	m_pInterface = static_cast<IExamInterface*>(pInterface);
	m_pWorldData = new LearnedWorldData();

	m_InventorySize = m_pInterface->Inventory_GetCapacity();

	Seek* pSeekBehavior = new Seek();
	m_BehaviourVec.push_back(pSeekBehavior);
	pSeekBehavior->SetTarget(&m_TargetData);

	Wander* pWander = new Wander();
	m_BehaviourVec.push_back(pWander);

	Flee* pFlee = new Flee();
	m_BehaviourVec.push_back(pFlee);

	m_pTargeter = new FixedGoalTargeter();
	m_pActuator = new BasicActuator(pSeekBehavior);
	m_pAvoidEnemyBehaviour = new SteeringPipeline();
	m_pAvoidEnemyBehaviour->SetTargeters({m_pTargeter});
	m_pAvoidEnemyBehaviour->SetActuator(m_pActuator);
	m_pAvoidEnemyBehaviour->SetFallBack(pWander);

	m_pInventory = new vector<ItemInfo>();
	ItemInfo ii;
	ii.Type = eItemType::_LAST;
	for (size_t i = 0; i < m_InventorySize; i++) {
		m_pInventory->push_back(ii);
	}

	Blackboard* pBlackboard = new Blackboard();
	pBlackboard->AddData("agentInfo", AgentInfo());
	pBlackboard->AddData("worldData", m_pWorldData);
	pBlackboard->AddData("seekBehaviour", static_cast<ISteeringBehaviour*>(pSeekBehavior));
	pBlackboard->AddData("wanderBehaviour", static_cast<ISteeringBehaviour*>(pWander));
	pBlackboard->AddData("fleeBehaviour", static_cast<ISteeringBehaviour*>(pFlee));
	pBlackboard->AddData("avoidEnemyBehaviour", static_cast<ISteeringBehaviour*>(m_pAvoidEnemyBehaviour));
	pBlackboard->AddData("currentBehaviour", static_cast<ISteeringBehaviour*>(pSeekBehavior));
	pBlackboard->AddData("interface", m_pInterface);
	pBlackboard->AddData("entityiesInFov", vector<EntityInfo>());
	pBlackboard->AddData("EnemyImages", vector<EnemyImage>());
	pBlackboard->AddData("enemiesInFov", vector<EnemyInfo>());
	pBlackboard->AddData("itemsInSight", vector<EntityInfo>());
	pBlackboard->AddData("housesInFOV", vector<HouseInfo>());
	pBlackboard->AddData("targetData", &m_TargetData);
	pBlackboard->AddData("bestTurnTarget", EnemyInfo());
	pBlackboard->AddData("targetInLoS", false);
	pBlackboard->AddData("turnDelta", (int)0);
	pBlackboard->AddData("inventory", m_pInventory);
	pBlackboard->AddData("inventorySize", m_InventorySize);
	pBlackboard->AddData("firstEmpty", (UINT)0);
	pBlackboard->AddData("gunSlot", (int)-1);
	pBlackboard->AddData("medkitSlot", (int)-1);
	pBlackboard->AddData("foodSlot", (int)-1);
	pBlackboard->AddData("numOfGuns", (int)0);
	pBlackboard->AddData("numOfMedkits", (int)0);
	pBlackboard->AddData("numOfFood", (int)0);
	pBlackboard->AddData("currentHouseTarget", Vector2());
	pBlackboard->AddData("currentItemTarget", Vector2());
	pBlackboard->AddData("visitedHouses", vector<Vector2>());
	pBlackboard->AddData("unvisitedHouses", vector<Vector2>());
	pBlackboard->AddData("closestItem", EntityInfo());
	pBlackboard->AddData("customTarget", Vector2(FLT_MAX, FLT_MAX));
	pBlackboard->AddData("scanningComplete", m_scanningComplete);
	pBlackboard->AddData("customTargetDistance", CUSTOM_TARGET_DISTANCE);
	pBlackboard->AddData("aimAccuracy", AIM_ACCURACY);
	pBlackboard->AddData("closeToHouseDist", CLOSETO_HOUSE);

	m_pBehaviourTree = new BehaviorTree(pBlackboard
		, new BehaviorSelector({
			new BehaviorSelector({
				// Stay alive stuff
				new BehaviorSelector({
					new BehaviorSequence({
						new BehaviorConditional(HasMedkit)
						, new BehaviorConditional(ShouldUseMedkit)
						, new BehaviorAction(UseMedkit)
					})
					,new BehaviorSequence({
						new BehaviorConditional(HasFood)
						, new BehaviorConditional(ShouldUseFood)
						, new BehaviorAction(UseFood)
					})
				})

				// Remove excess stuff
				,new BehaviorSelector({
					new BehaviorSequence({
						new BehaviorConditional(InventoryIsFull)
						,new BehaviorConditional(IsItemInSight)
						,new BehaviorConditional(HasTooManyMedkits)
						,new BehaviorConditional(HasMedkit)
						,new BehaviorAction(UseMedkit)
					})
					,new BehaviorSequence({
						new BehaviorConditional(InventoryIsFull)
						,new BehaviorConditional(IsItemInSight)
						,new BehaviorConditional(HasTooMuchFood)
						,new BehaviorConditional(HasFood)
						,new BehaviorAction(UseFood)
					})
					,new BehaviorSequence({
						new BehaviorConditional(InventoryIsFull)
						,new BehaviorConditional(IsItemInSight)
						,new BehaviorConditional(HasTooManyGuns)
						,new BehaviorConditional(HasGun)
						,new BehaviorAction(DropGun)
					})
					
					
				})
				
				// Enemy stuff
				,new BehaviorSequence({
						new BehaviorConditional(IsEnemyInSight)
						, new BehaviorConditional(HasGun)
						, new BehaviorConditional(IsEnemyInLoS)
						, new BehaviorAction(ShootGun)
					})
				
			})

			, new BehaviorSelector({
				// Item stuff
				new BehaviorSelector({
					new BehaviorSequence({
						new BehaviorConditional(HasItemTarget)
						, new BehaviorConditional(IsEnemyInSight)
						, new BehaviorAction(ClearClosestItem)
					})
					,new BehaviorSequence({
						new BehaviorConditional(HasItemTarget)
						, new BehaviorConditional(HasFreeInventorySpot)
						, new BehaviorConditional(IsCloseToItem)
						, new BehaviorAction(PickupItem)
					})
				})


				// House stuff
				,new BehaviorSelector({
					new BehaviorSequence({ 
						new BehaviorConditional(HasHouseTarget)
						, new BehaviorConditional(IsCloseToHouseTargetCenter)
						, new BehaviorAction(ClearHouseTarget)
					})
					,new BehaviorSequence({
						new BehaviorConditional(HasNoHouseTarget)
						, new BehaviorConditional(SeesUnvisitedHouse)
						, new BehaviorAction(SetTargetHouse)
					})
				})

				// Oldest house stuff
				,new BehaviorSelector({
					new BehaviorSequence({
						new BehaviorConditional(ScanCompleted)
						,new BehaviorConditional(HasNoCustomTarget)
						, new BehaviorAction(SetCustomTargetToOldestHouse)
					})
					,new BehaviorSequence({
						new BehaviorConditional(ScanCompleted)
						, new BehaviorConditional(HasCustomTarget)
						, new BehaviorConditional(IsClosetoCustomTarget)
						, new BehaviorAction(ResetCustomTarget)
					})
				})
			})
			,new BehaviorAction(ChangeToAvoidEnemy)
		})
	);

	//Bit information about the plugin
	//Please fill this in!!
	info.BotName = "Visiting old friends";
	info.Student_FirstName = "Nikos";
	info.Student_LastName = "Vanden Broek";
	info.Student_Class = "2DAE5";
}

//Called only once
void Plugin::DllInit()
{
	//Called when the plugin is loaded
}

//Called only once
void Plugin::DllShutdown()
{
	//Called when the plugin gets unloaded
	SAFE_DELETE(m_pBehaviourTree);
	SAFE_DELETE(m_pWorldData);

	for (auto pb : m_BehaviourVec)
		SAFE_DELETE(pb);
	m_BehaviourVec.clear();
	
	SAFE_DELETE(m_pTargeter);
	SAFE_DELETE(m_pInventory);

	SAFE_DELETE(m_pActuator);
	SAFE_DELETE(m_pAvoidEnemyBehaviour);

}

//Called only once, during initialization
void Plugin::InitGameDebugParams(GameDebugParams& params)
{
	params.AutoFollowCam = true; //Automatically follow the AI? (Default = true)
	params.RenderUI = true; //Render the IMGUI Panel? (Default = true)
	params.SpawnEnemies = true; //Do you want to spawn enemies? (Default = true)
	params.EnemyCount = 20; //How many enemies? (Default = 20)
	params.GodMode = false; //GodMode > You can't die, can be usefull to inspect certain behaviours (Default = false)
	//params.LevelFile = "LevelTwo.gppl";
	params.AutoGrabClosestItem = false; //A call to Item_Grab(...) returns the closest item that can be grabbed. (EntityInfo argument is ignored)
	params.OverrideDifficulty = false;
	params.Difficulty = 3.f;
}

//Only Active in DEBUG Mode
//(=Use only for Debug Purposes)
void Plugin::ProcessEvents(const SDL_Event& e)
{

}

//Update
//This function calculates the new SteeringOutput, called once per frame
SteeringPlugin_Output Plugin::UpdateSteering(float dt)
{
	// First time running we divide the world in parts to scan.
	// This has to be done here because world size = 0 in initialize
	m_pWorldData->GenerateScanPoints(m_pInterface->World_GetInfo());

	// Update to carve doors in houses
	AgentInfo agentInfo = m_pInterface->Agent_GetInfo();
	m_pWorldData->Update(dt, agentInfo);

	// Simple checks to break on certain events
	StatisticsInfo si = m_pInterface->World_GetStats();
	if (si.NumMissedShots > m_LastStatistics.NumMissedShots) {
		int i = 5;
	}
	if (si.NumIgnoredCalls > m_LastStatistics.NumIgnoredCalls) {
		int i = 5;
	}
	if (agentInfo.Death) {
		int i = 5;
	}

	m_LastStatistics = si;


	// Steering starts here

	auto steering = SteeringPlugin_Output();

	// Update are target location depending on what what we are doing.
	Blackboard* pBlackboard = m_pBehaviourTree->GetBlackboard();
	Vector2 houseTarget;
	EntityInfo closestItemTarget;
	Vector2 customTarget;
	bool datavAilable = pBlackboard->GetData("currentHouseTarget", houseTarget)
		&& pBlackboard->GetData("closestItem", closestItemTarget)
		&& pBlackboard->GetData("customTarget", customTarget);

	// checkpoint
	Vector2 checkpointLocation = m_pInterface->World_GetCheckpointLocation();

	// First time done scanning = reset visited houses
	if (!m_scanningComplete && !m_pWorldData->HasScanPoints()) {
		cout << "Scanning done" << endl;
		m_scanningComplete = true;
		pBlackboard->ChangeData("visitedHouses", vector<Vector2>());
	}

	// If checkpoint updated, reset visited houses.
	if (Distance(m_LastCheckPoint, checkpointLocation) > 0.001f) {
		pBlackboard->ChangeData("visitedHouses", vector<Vector2>());
		m_LastCheckPoint = checkpointLocation;
	}


	if (closestItemTarget.Type != eEntityType::_LAST) {
		// We have an item target
		checkpointLocation = closestItemTarget.Location;
	}
	else if (houseTarget.x != FLT_MAX && houseTarget.y != FLT_MAX) {
		// We have a house target
		checkpointLocation = houseTarget;
	}
	else if (customTarget.x != FLT_MAX && customTarget.y != FLT_MAX) {
		// We have an 'oldest building' target
		LearnedHouse* house = m_pWorldData->GetCurrentHouse();
		// If he are in a house and the checkpoint is in it, don't scan, go for the checkpoint;
		if (!house || !house->PointsIsInHouse(checkpointLocation)) {
			checkpointLocation = customTarget;
		}
	}
	else if (m_pWorldData->HasScanPoints()) {
		// We have a scan target
		LearnedHouse* house = m_pWorldData->GetCurrentHouse();
		// If he are in a house and the checkpoint is in it, don't scan, go for the checkpoint;
		if (!house || !house->PointsIsInHouse(checkpointLocation)) {
			checkpointLocation = m_pWorldData->GetNextScanPoint();
		}
		
	}
	
	// For visualization
	m_Target = checkpointLocation;

	// Entering/leaving a house to update their last visit time
	// Time is also updated when entering, so entering the oldest house becomes the youngest once entered.
	// This way we can get the next oldest house while still inside
	if (m_PreviousInfo.IsInHouse != agentInfo.IsInHouse) {
		if (agentInfo.IsInHouse) {
			// We entered a house
			m_pWorldData->EnteredHouse(m_pInterface->World_GetStats().TimeSurvived, agentInfo);
		}
		else {
			// We left a house
			m_pWorldData->LeftHouse(m_pInterface->World_GetStats().TimeSurvived);
		}
	}
	m_PreviousInfo = agentInfo;

	//Use the navmesh to calculate the next navmesh point
	Vector2 nextTargetPos = m_pInterface->NavMesh_GetClosestPathPoint(checkpointLocation);

	m_TargetData.Position = nextTargetPos;
	m_pTargeter->GetGoalRef().Position = nextTargetPos;
	m_pTargeter->GetGoalRef().PositionSet = true;

	// Get FoV data
	vector<HouseInfo> vHousesInFOV = GetHousesInFOV();
	vector<EntityInfo> vEntitiesInFOV = GetEntitiesInFOV();

	// Updatew word data with houses
	for (HouseInfo hi : vHousesInFOV) {
		if (m_pWorldData->HouseIsNew(hi)) {
			m_pWorldData->AddBuilding(hi, m_pInterface->World_GetStats().TimeSurvived);
		}
	}

	// Turn enemies in 'temporary threats' for obstacle avoidance
	for (EntityInfo entity : vEntitiesInFOV) {
		if (entity.Type != eEntityType::ENEMY) {
			continue;
		}
		EnemyInfo ei; 
		m_pInterface->Enemy_GetInfo(entity, ei);
		if (m_pWorldData->ThreatIsNew(ei)) {
			m_pWorldData->AddThreat(ei);
		}
	}

	// Make list of all items and one for all enemies we currently see
	vector<EntityInfo> itemsInSight;
	vector<EnemyInfo> enemiesInSight;
	for (EntityInfo entity : vEntitiesInFOV) {
		if (entity.Type == eEntityType::ENEMY) {
			EnemyInfo ei;
			m_pInterface->Enemy_GetInfo(entity, ei);
			enemiesInSight.push_back(ei);
			continue;
		}
		else if (entity.Type == eEntityType::ITEM) {
			itemsInSight.push_back(entity);
			m_pWorldData->AddItemToPickupList(entity);
			continue;
		}
	}

	// Update blackboard values
	if (pBlackboard) {
		pBlackboard->ChangeData("scanningComplete", m_scanningComplete);
		pBlackboard->ChangeData("itemsInSight", itemsInSight);
		pBlackboard->ChangeData("agentInfo", agentInfo);
		pBlackboard->ChangeData("targetData", &m_TargetData);
		pBlackboard->ChangeData("enemiesInFov", enemiesInSight);
		pBlackboard->ChangeData("entityiesInFov", vEntitiesInFOV);
		pBlackboard->ChangeData("EnemyImages", m_pWorldData->GetEnemyImages());
		//pBlackboard->ChangeData("pathIsBlocked", m_pWorldData->PathIsBlocked());
		pBlackboard->ChangeData("housesInFOV", vHousesInFOV);

		// Count items
		int numGuns = 0;
		int numMedkits = 0;
		int numFood = 0;
		// Get worst of item = first to use
		int gunSlot = -1;
		int lowestAmmo = INT_MAX;
		int medkitSlot = -1;
		int lowestHealth = INT_MAX;
		int foodSlot = -1;
		int lowestFood = INT_MAX;
		// Get first empty slot in inventory
		UINT emptySlot = m_InventorySize;
		for (size_t i = 0; i < m_InventorySize; i++) {
			ItemInfo ii = m_pInventory->at(i);
			if (ii.Type == eItemType::MEDKIT) {
				numMedkits++;
				int health = m_pInterface->Item_GetMetadata(ii, "health");
				if (health < lowestHealth) {
					lowestHealth = health;
					medkitSlot = i;
				}
			}
			if (ii.Type == eItemType::PISTOL) {
				numGuns++;
				int ammo = m_pInterface->Item_GetMetadata(ii, "ammo");
				if (ammo < lowestAmmo) {
					lowestAmmo = ammo;
					gunSlot = i;
				}
			}
			if (ii.Type == eItemType::FOOD) {
				numFood++;
				int energy = m_pInterface->Item_GetMetadata(ii, "energy");
				if (energy < lowestFood) {
					lowestFood = energy;
					foodSlot = i;
				}
			}

			if (emptySlot > i && ii.Type == eItemType::_LAST) {
				emptySlot = i;
			}
		}

		pBlackboard->ChangeData("firstEmpty", emptySlot);
		pBlackboard->ChangeData("gunSlot", gunSlot);
		pBlackboard->ChangeData("numOfGuns", numGuns);
		pBlackboard->ChangeData("medkitSlot", medkitSlot);
		pBlackboard->ChangeData("numOfMedkits", numMedkits);
		pBlackboard->ChangeData("foodSlot", foodSlot);
		pBlackboard->ChangeData("numOfFood", numFood);

		// Find closest item, making it the first item target to go for
		EntityInfo closestItem;
		float shortestItemDist = FLT_MAX;
		closestItem.Type = eEntityType::_LAST;
		// Only care for item if we actually have room, which we should
		if (emptySlot < m_InventorySize) {
			for (EntityInfo ei : itemsInSight) {
				float dist = Distance(ei.Location, agentInfo.Position);
				if (dist < shortestItemDist) {
					shortestItemDist = dist;
					closestItem = ei;
				}
			}
		}
		pBlackboard->ChangeData("closestItem", closestItem);

		// If we don't see an item, check if we saw one before that we passed
		// If so, make that the item target
		if (closestItem.Type == eEntityType::_LAST && m_pWorldData->NumItemsToPickUp() > 0) {
			EntityInfo rememberItem = m_pWorldData->GetClosestItemToPickup(agentInfo.Position);
			if (Distance(rememberItem.Location, agentInfo.Position) > REMEMBERED_ITEM_DISTANCE) {
				pBlackboard->ChangeData("closestItem", m_pWorldData->GetClosestItemToPickup(agentInfo.Position));
			}
			
		}

		pBlackboard->ChangeData("targetInLoS", false);
		pBlackboard->ChangeData("turnDelta", 0);
		
	}
	m_pBehaviourTree->Update();

	// Get updated data to determine actor behaviour
	ISteeringBehaviour* pCurrentBehaviour = nullptr;
	EntityInfo closestItem;
	int turnDelta = 0;
	bool targetInLoS = false;
	datavAilable = pBlackboard->GetData("currentBehaviour", pCurrentBehaviour)
		&& pBlackboard->GetData("currentHouseTarget", houseTarget)
		&& pBlackboard->GetData("closestItem", closestItem)
		&& pBlackboard->GetData("turnDelta", turnDelta)
		&& pBlackboard->GetData("targetInLoS", targetInLoS);
	bool shouldRun = houseTarget.x != FLT_MAX && houseTarget.y != FLT_MAX;
	if (datavAilable && pCurrentBehaviour) {
		SteeringPlugin_Output out = pCurrentBehaviour->CalculateSteering(dt, agentInfo);
		
		
		out.AngularVelocity = agentInfo.MaxAngularSpeed * turnDelta;
		// Snap to direction if we don't have a target in LoS and we're not rotating to aim
		out.AutoOrientate = !targetInLoS && turnDelta == 0;

		// run when told to, but not inside buildings, or when rotating (=aiming)
		out.RunMode = shouldRun && !agentInfo.IsInHouse && out.AutoOrientate;

		return out;
	}

	printf("Something messed up and we default to wandering");
	Wander wander;
	return wander.CalculateSteering(dt, agentInfo);
}

//This function should only be used for rendering debug elements
void Plugin::Render(float dt) const
{
	// Draw world info
	m_pWorldData->Draw(m_pInterface);

	//This Render function should only contain calls to Interface->Draw_... functions
	m_pInterface->Draw_SolidCircle(m_Target, .7f, { 0,0 }, { 1, 0, 1 }, 0);

	vector<EntityInfo> vEntitiesInFOV = {};
	TargetData* targetData = nullptr;
	Blackboard* pBlackboard = m_pBehaviourTree->GetBlackboard();
	AgentInfo agentInfo = {};
	ISteeringBehaviour* pCurrentBehaviour = nullptr;
	if (pBlackboard) {
		pBlackboard->GetData("entityiesInFov", vEntitiesInFOV);
		pBlackboard->GetData("targetData", targetData);
		pBlackboard->GetData("agentInfo", agentInfo);
		pBlackboard->GetData("currentBehaviour", pCurrentBehaviour);
	}

	// Color ring around entities in FoV
	Vector3 col = {1, 0, 1};
	for each (EntityInfo info in vEntitiesInFOV) {
		if (info.Type == eEntityType::ITEM) {
			col = {0, 1, 0};
		}

		m_pInterface->Draw_Circle(info.Location, 1.5f, col);
	}
	
	// Line to next goal
	if (targetData) {
		m_pInterface->Draw_Segment(agentInfo.Position, pCurrentBehaviour->As<SteeringPipeline>()->GetPath()->GetGoal().Position, {1,1,1});
	}
}

vector<HouseInfo> Plugin::GetHousesInFOV() const
{
	vector<HouseInfo> vHousesInFOV = {};

	HouseInfo hi = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetHouseByIndex(i, hi))
		{

			vHousesInFOV.push_back(hi);
			continue;
		}

		break;
	}

	return vHousesInFOV;
}

vector<EntityInfo> Plugin::GetEntitiesInFOV() const
{
	vector<EntityInfo> vEntitiesInFOV = {};
	EntityInfo ei = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetEntityByIndex(i, ei))
		{
			vEntitiesInFOV.push_back(ei);
			continue;
		}

		break;
	}

	return vEntitiesInFOV;
}
