#pragma once
#include "IExamPlugin.h"
#include "Exam_HelperStructs.h"
#include "BehaviourTree\BehaviorTree.h"
#include "SteeringHelpers.h"


class IBaseInterface;
class IExamInterface;
class ISteeringBehaviour;
class FixedGoalTargeter;
class LearnedWorldData;

class BasicActuator;
class LearnedWorldDecomposer;
class SteeringPipeline;

class Plugin :public IExamPlugin
{
public:
	Plugin() {};
	virtual ~Plugin() {};

	void Initialize(IBaseInterface* pInterface, PluginInfo& info) override;
	void DllInit() override;
	void DllShutdown() override;

	void InitGameDebugParams(GameDebugParams& params) override;
	void ProcessEvents(const SDL_Event& e) override;

	SteeringPlugin_Output UpdateSteering(float dt) override;
	void Render(float dt) const override;

private:
	const float REMEMBERED_ITEM_DISTANCE = 2.5f;
	const float CUSTOM_TARGET_DISTANCE = 5.f;
	const float AIM_ACCURACY = 0.7f;
	const float AIM_TURN_SPEED = 1.5f;
	const float CLOSETO_HOUSE = 2.f;

	bool m_scanningComplete = false;
	AgentInfo m_PreviousInfo;

	UINT m_InventorySize = 0;
	Vector2 m_LastCheckPoint = {0, 0};
	StatisticsInfo m_LastStatistics;

	vector<ISteeringBehaviour*> m_BehaviourVec = {};
	BehaviorTree* m_pBehaviourTree = nullptr;
	TargetData m_TargetData = {};
	FixedGoalTargeter* m_pTargeter = nullptr;
	LearnedWorldData* m_pWorldData = nullptr;
	vector<ItemInfo>* m_pInventory = nullptr;

	BasicActuator* m_pActuator = nullptr;
	SteeringPipeline* m_pAvoidEnemyBehaviour = nullptr;

	//Interface, used to request data from/perform actions with the AI Framework
	IExamInterface* m_pInterface = nullptr;
	vector<HouseInfo> GetHousesInFOV() const;
	vector<EntityInfo> GetEntitiesInFOV() const;

	Elite::Vector2 m_Target = {};
	bool m_CanRun = false; //Demo purpose
	bool m_GrabItem = false; //Demo purpose
	bool m_UseItem = false; //Demo purpose
	bool m_RemoveItem = false; //Demo purpose
	float m_AngSpeed = 0.f; //Demo purpose

	float highestDT = 0;
	float lowestFPS = FLT_MAX;
};

//ENTRY
//This is the first function that is called by the host program
//The plugin returned by this function is also the plugin used by the host program
extern "C"
{
	__declspec (dllexport) IPluginBase* Register()
	{
		return new Plugin();
	}
}