#include "stdafx.h"
#include "Guard.h"
#include "../App_SteeringBehaviours/SteeringAgent.h"
#include "../App_SteeringBehaviours/SteeringBehaviours.h"

#include "Blackboard.h"
#include "BehaviorTree.h"
#include "Behaviors.h"

Guard::Guard()
{}

Guard::~Guard()
{
	for (auto pb : m_BehaviourVec)
		SAFE_DELETE(pb);
	m_BehaviourVec.clear();

	SAFE_DELETE(m_pBehaviorTree);
	SAFE_DELETE(m_pAgent);
}

void Guard::Initialize(Context* pContext)
{
	//Create agent
	m_pAgent = new SteeringAgent(pContext);
	m_pAgent->SetMaxLinearSpeed(16.f);
	m_pAgent->SetAutoOrient(true);
	m_pAgent->SetMass(0.1f);
	m_pAgent->SetRenderBehaviour(true);
	//Target
	m_Target.Position = m_pAgent->GetPosition();
	//Create behaviors
	auto pSeekBehavior = new Seek();
	pSeekBehavior->SetTarget(&m_Target);
	m_BehaviourVec.push_back(pSeekBehavior);
	auto pWanderBehavior = new Wander(pContext);
	m_BehaviourVec.push_back(pWanderBehavior);
	//Set initial steering behavior
	m_pAgent->SetSteeringBehaviour(pWanderBehavior);

	//*** Create blackboard ***
	Blackboard* pBlackboard = new Blackboard();
	pBlackboard->AddData("Agent", m_pAgent);
	pBlackboard->AddData("WanderBehaviour", static_cast<ISteeringBehaviour*>(pWanderBehavior));
	pBlackboard->AddData("SeekBehaviour", static_cast<ISteeringBehaviour*>(pSeekBehavior));
	pBlackboard->AddData("Target", m_Target.Position);
	pBlackboard->AddData("TargetSet", false);
	pBlackboard->AddData("IsBoxOpen", false);
	pBlackboard->AddData("CloseToRadius", 1.0f);

	//*** Create behavior tree ***
	m_pBehaviorTree = new BehaviorTree(pBlackboard
		,new BehaviorSelector({
			new BehaviorSequence({
				new BehaviorConditional(HasTarget)
				,new BehaviorConditional(IsFarAway)
				,new BehaviorAction(ChangeToSeek)
			})
			,new BehaviorSelector({
				new BehaviorSequence({
					new BehaviorConditional(HasTarget)
					,new BehaviorConditional(IsBoxClosed)
					,new BehaviorAction(OpenBox)
					,new BehaviorAction(PickUpItem)
				})
				,new BehaviorSequence({
					new BehaviorConditional(HasTarget)
					,new BehaviorAction(PickUpItem)
				})
				,new BehaviorAction(ChangeToWander)
			})
		}));
}

void Guard::Update(float deltaTime, const Vector2& targetPos, bool isSet)
{
	//*** Change blackboard data ***
	Blackboard* pBlackboard = m_pBehaviorTree->GetBlackboard();
	if (pBlackboard) {
		if (isSet) {
			m_Target.Position = targetPos;
			pBlackboard->ChangeData("Target", m_Target.Position);
			pBlackboard->ChangeData("TargetSet", isSet);

			int rng = rand() % 2;
			pBlackboard->ChangeData("IsBoxOpen", static_cast<bool>(rng));
		}
	}
	
	//*** Update behavior tree ***
	m_pBehaviorTree->Update();

	//Update agent
	m_pAgent->Update(deltaTime);
}