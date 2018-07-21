#pragma once
#include "IExamPlugin.h"
#include "IExamInterface.h"
#include "SteeringPipeline.h"
#include "../LearnedWorldData.h"
#include "../EliteNavigation/ENavigation.h"
//#include "../EliteNavigation/EHeuristicFunctions.h"
//#include "../EliteNavigation/Graphs/EGraph.h"

class LearnedWorldData;

//***********************************************************************************
//1. TARGETER (FixedGoalTargeter)
//Basic Implementation
class FixedGoalTargeter : public Targeter
{
public:
	Goal& GetGoalRef();
	Goal GetGoal() override;

private:
	Goal m_Goal = {};
};

//***********************************************************************************
//2. DECOMPOSER (Path Planner)
//Could be combined with NavMesh Graph
//Skip for now...
class LearnedWorldDecomposer : public Decomposer {
public:
	LearnedWorldDecomposer(LearnedWorldData* pWorldData);
	Goal DecomposeGoal(const Goal& goal, float deltaTime) override;

private:
	Goal m_Goal = {};
	LearnedWorldData* m_pWorldData = nullptr;

	bool HasDirectPath(vector<Vector2> &path, Graph<Node>* pGraph, UINT start, UINT end, float distance, int rowSize);
};

//***********************************************************************************
//3. CONSTRAINT
//Simple constraint that checks if the character isn't walking into a SphereObstacle

//Sphere Obstacle
struct SphereObstacle
{
	Vector2 Position;
	float Radius;
};

struct EnemyImage;

class AvoidEnemyContraint : public Constraint
{
public:
	AvoidEnemyContraint(vector<EnemyImage> obstacles, LearnedWorldData* pWorldData) :m_Threats(obstacles), m_pWorldData(pWorldData){}

	float WillViolate(const Path* pPath, AgentInfo pAgent, float maxPriority) override;
	Goal Suggest(const Path* pPath) override;

private:
	float m_AvoidMargin = 1.f;
	vector<EnemyImage> m_Threats = {};
	LearnedWorldData* m_pWorldData = nullptr;
	float WillViolate(const Path* pPath, AgentInfo pAgent, float maxPriority, const EnemyImage& thread); //Internal single sphere check

	Goal m_SuggestedGoal = {};
};


//***********************************************************************************
//4. ACTUATOR (Basic Actuator, Seek path goal)
class BasicActuator : public Actuator
{
public:
	BasicActuator(Seek* pSeekBehaviour) :m_pSeekBehaviour(pSeekBehaviour) {}
	~BasicActuator() { SAFE_DELETE(m_pPath); }

	Path* CreatePath() override;
	void UpdatePath(Path* pPath, AgentInfo pAgent, const Goal& goal) override;
	SteeringPlugin_Output CalculateSteering(const Path* pPath, float deltaT, AgentInfo pAgent) override;

private:
	Path* m_pPath = nullptr;
	Seek* m_pSeekBehaviour = nullptr;
};