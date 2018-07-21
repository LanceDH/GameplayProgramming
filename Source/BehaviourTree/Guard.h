#pragma once
//Include
#include "../App_SteeringBehaviours/SteeringHelpers.h"
class BehaviorTree;
class SteeringAgent;
class ISteeringBehaviour;

//GUARD Class
class Guard
{
public:
	Guard();
	~Guard();

	void Initialize(Context* pContext);
	void Update(float deltaTime, const Vector2& targetPos, bool isSet);

private:
	SteeringAgent* m_pAgent = nullptr;
	vector<ISteeringBehaviour*> m_BehaviourVec = {};
	TargetData m_Target = {};
	BehaviorTree* m_pBehaviorTree = nullptr;
};

