#include "stdafx.h"
#include "SteeringPipeline.h"
//#include "IAssignmentInterface.h"

float Path::GetMaxPriority()
{
	/*if (!m_pAgent)
		return 0.f;*/

	return (m_pAgent.Position - m_Goal.Position).Magnitude();
}

SteeringPlugin_Output SteeringPipeline::CalculateSteering(float deltaT, AgentInfo pAgent)
{
	Goal currGoal = {}; //Goal used by the pipeline

						//1. TARGETER
						//Try to merge the targeter's goals
	for (auto pTargeter : m_Targeters)
	{
		Goal targeterGoal = pTargeter->GetGoal();
		if (currGoal.CanMergeGoal(targeterGoal))
		{
			currGoal.UpdateGoal(targeterGoal);
		}
	}


	//2. DECOMPOSER
	//Decompose the current goal
	for (auto pDecomposer : m_Decomposers)
	{
		currGoal = pDecomposer->DecomposeGoal(currGoal, deltaT);
	}

	//3. PATH & CONSTRAINTS
	//Check for constraint violations
	if (!m_pPath) m_pPath = m_pActuator->CreatePath();

	float shortestViolation, currentViolation, maxViolation;
	Constraint* pViolatingConstraint = nullptr;

	for (UINT i = 0; i < m_MaxConstraintSteps; ++i)
	{
		//Get path to goal
		m_pActuator->UpdatePath(m_pPath, pAgent, currGoal);

		//No constraints => calculate steering using the actuator
		if (m_Constraints.size() == 0)
		{
			return m_pActuator->CalculateSteering(m_pPath, deltaT, pAgent);
		}

		//Find Violating Constraint
		shortestViolation = maxViolation = m_pPath->GetMaxPriority();
		for (auto pConstraint : m_Constraints)
		{
			if (i == 0)
				pConstraint->SetSuggestionUsed(false);

			//Check if this constraint is violated earlier than the (current) shortestviolation
			currentViolation = pConstraint->WillViolate(m_pPath, pAgent, shortestViolation);
			if (currentViolation < shortestViolation && currentViolation > 0)
			{
				shortestViolation = currentViolation;
				pViolatingConstraint = pConstraint;
			}

		}

		//Check Most Violating Constraint
		if (pViolatingConstraint && shortestViolation < maxViolation)
		{
			currGoal = pViolatingConstraint->Suggest(m_pPath);
			pViolatingConstraint->SetSuggestionUsed(true);
		}
		else
		{
			//if (pAgent.CanRenderBehaviour() && m_pContext)
			//{
			//	m_pContext->pRenderer->DrawSegment(pAgent->GetPosition(), m_pPath->GetGoal().Position, { 1,1,0,0.5f });
			//	//m_pContext->pRenderer->DrawSolidCircle(m_pPath->GetGoal().Position, 0.5f, { 0,0 }, { 1,1,0,1 });
			//	m_pContext->pRenderer->DrawSolidCircle(currGoal.Position, 0.5f, { 0.f,0.f }, { 1.f,1.f,0.f });
			//	m_pContext->pRenderer->DrawSegment(m_pPath->GetGoal().Position, m_Targeters[0]->GetGoal().Position, { 1,1,0,0.5f });
			//}

			//Found a goal (in peace with all constraints)
			return m_pActuator->CalculateSteering(m_pPath, deltaT, pAgent);
		}
	}

	//ConstraintStep reached (no solution found, use fallback behaviour for now)
	if (m_pFallbackBehaviour)
		return m_pFallbackBehaviour->CalculateSteering(deltaT, pAgent);

	return SteeringPlugin_Output();
}
