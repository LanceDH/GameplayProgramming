#include "stdafx.h"
#include "SteeringPipeline_Implementation.h"
#include "../EliteNavigation/ENavigation.h"
#include "../LearnedWorldData.h"

#undef max

//1. TARGETER (FixedGoalTargeter)
//*******************************
Goal& FixedGoalTargeter::GetGoalRef()
{
	return m_Goal;
}

Goal FixedGoalTargeter::GetGoal()
{
	return m_Goal;
}

//2. DECOMPOSER (skip for now)
LearnedWorldDecomposer::LearnedWorldDecomposer(LearnedWorldData* pWorldData) :
	m_pWorldData(pWorldData)
{
}

Goal LearnedWorldDecomposer::DecomposeGoal(const Goal & goal, float deltaTime) {
	//if (!m_pWorldData || m_pWorldData->PathIsBlocked()) {
	//	//cout << "Graph blocked" << endl;
	//	return goal;
	//}
	//vector<Vector2> path = m_pWorldData->GetPath();
	//if (path.size() <= 1) {
	//	cout << "NoPath" << endl;
	//	return goal;
	//}

	//Goal newGoal;
	//newGoal.Position = goal.Position;

	//Graph<Node>* pGraph = m_pWorldData->GetGraph();
	//vector<Node*> vNodes = pGraph->GetNodes();

	////UINT startPos = (gridSize.y / 2 * gridSize.x) + gridSize.x / 2;
	//float distBetweenNodes = Distance(vNodes[0]->GetPosition(), vNodes[1]->GetPosition());

	//if (HasDirectPath(path, pGraph, 0, path.size()-1, distBetweenNodes, int(gridSize.x))) {
	//	cout << "Has Direct Path" << endl;
	//	return goal;
	//}

	//for (size_t i = path.size() - 2; i > 0; i--) {
	//	if (HasDirectPath(path, pGraph, 0, i, distBetweenNodes, int(gridSize.x))) {
	//		Goal newGoal;
	//		newGoal.Position = path[i];
	//		newGoal.PositionSet = true;
	//		cout << "Returned new goal" << endl;
	//		return newGoal;
	//	}
	//}
	//cout << "Fallsback" << endl;

	return goal;
}

bool LearnedWorldDecomposer::HasDirectPath(vector<Vector2> &path, Graph<Node>* pGraph, UINT start, UINT end, float distance, int rowSize) {
	Vector2 startPos = path[start];
	Vector2 endPos = path[end];
	Vector2 dir = endPos - startPos;
	Normalize(dir);
	Node* startNode = pGraph->GetClosestNodeAny(startPos, HeuristicFunctions::Chebyshev);
	Node* endNode = pGraph->GetClosestNodeAny(endPos, HeuristicFunctions::Chebyshev);
	Node* tempNode = nullptr;
	while (startNode != endNode) {
		Vector2 checkPos = startNode->GetPosition();
		dir = endPos - startNode->GetPosition();
		Normalize(dir);
		// Find first neext node in direction
		do {
			checkPos += dir*distance;
			//tempNode = pGraph->GetNeighbourInDirection(startNode, dir, rowSize); 
			tempNode = pGraph->GetClosestNodeAny(checkPos, HeuristicFunctions::Chebyshev);

		} while (tempNode == startNode);
		//tempNode = pGraph->GetClosestNodeInDirection(startPos, dir, HeuristicFunctions::Chebyshev);
		if (!tempNode->IsWalkable()) {
			return false;
		}
		startNode = tempNode;
		tempNode = nullptr;
	}

	return true;
}


//3. CONSTRAINT (Avoid Sphere Obstacles)
//**************************************
float AvoidEnemyContraint::WillViolate(const Path* pPath, AgentInfo pAgent, float maxPriority)
{
	float currPriority = 0.f;
	float smallestPriority = maxPriority;
	for (auto enemy : m_Threats)
	{
		currPriority = WillViolate(pPath, pAgent, maxPriority, enemy);
		if (currPriority < smallestPriority) smallestPriority = currPriority;
	}

	return smallestPriority;
}

Goal AvoidEnemyContraint::Suggest(const Path* pPath)
{
	return m_SuggestedGoal; //Should be calculated during WillViolate
}

//Internal single sphere check
float AvoidEnemyContraint::WillViolate(const Path* pPath, AgentInfo pAgent, float maxPriority, const EnemyImage& thread)
{
	auto currGoal = pPath->GetGoal();
	auto agent2Sphere = thread.Position - pAgent.Position; //Line between Agent & Sphere
	auto dir2Target = (currGoal.Position - pAgent.Position).GetNormalized(); //Direction to Target/Goal
	auto dist2ClosestPoint = Elite::Dot(agent2Sphere, dir2Target); //Projection of Agent2Sphere line on vector dir2Target
	
	if (dist2ClosestPoint < 0.f || dist2ClosestPoint >= maxPriority) //Previous closest point is closer? Or currentPoint behind us?
		return numeric_limits<float>::max();

	auto closestPointToObstacle = pAgent.Position + (dist2ClosestPoint * dir2Target); //Calculate closestPointToObstacle (on dir2Target)

	auto dirOffset = (closestPointToObstacle - thread.Position); //Calculate offset to move closestPoint outside the obstacle (if needed)
	auto distFromObstacleCenter = dirOffset.Normalize(); //Get offset direction + length to the closestPoint

	if (distFromObstacleCenter > thread.Size + m_AvoidMargin) //Does path cross the obstacle?
		return numeric_limits<float>::max();

	auto newTarget = thread.Position + ((thread.Size + m_AvoidMargin)*dirOffset); //Calculate new target (sub-goal)
	
	auto dist2NewTarget = Elite::Distance(pAgent.Position, newTarget); //Distance to newTarget (sub-goal)		
	if(dist2NewTarget >= maxPriority) //Final Check: current newTarget -> closer than prev calculated subgoal?
		return numeric_limits<float>::max();

	if (m_pWorldData->PointIsInWall(newTarget)) {
		cout << "- - - - - - Steers into wall - - - - - -" << endl;
		// https://stackoverflow.com/questions/3306838/algorithm-for-reflecting-a-point-across-a-line
		Normalize(dir2Target);

		float a = dir2Target.y / abs(dir2Target.x);
		float d = ((newTarget.x - pAgent.Position.x) + (newTarget.y - pAgent.Position.y) * a) / (1 + pow(a, 2));
		Vector2 mirrored;
		mirrored.x = 2 * d - (newTarget.x - pAgent.Position.x);
		mirrored.y = 2 * d * a - (newTarget.y - pAgent.Position.y);
		mirrored = pAgent.Position + mirrored ;
		
		if (m_pWorldData->PointIsInWall(mirrored)) {
			// Even the mirror is in a wall
			return numeric_limits<float>::max();
		}

		newTarget = mirrored;
	}

	//Current target is potential new (sub)goal
	m_SuggestedGoal.Position = newTarget;
	m_SuggestedGoal.PositionSet = true;

	return dist2NewTarget; //return distance to newTarget (= new priority)
}

//4. ACTUATOR (Basic Seek Path Point)
//***********************************
Path* BasicActuator::CreatePath()
{
	SAFE_DELETE(m_pPath);

	m_pPath = new Path;
	return m_pPath;
}

void BasicActuator::UpdatePath(Path* pPath, AgentInfo pAgent, const Goal& goal)
{
	pPath->SetGoal(goal);
	pPath->SetAgent(pAgent);
}

SteeringPlugin_Output BasicActuator::CalculateSteering(const Path* pPath, float deltaT, AgentInfo pAgent)
{
	auto goal = pPath->GetGoal();

	//Context::Current.renderer->DrawSolidCircle(goal.Position, 0.5f, { 0.f,0.f }, { 0.f,1.f,1.f });

	if (goal.PositionSet && m_pSeekBehaviour)
	{
		auto newTarget = TargetData(goal.Position);
		m_pSeekBehaviour->SetTarget(&newTarget);
		return m_pSeekBehaviour->CalculateSteering(deltaT, pAgent);
	}

	return SteeringPlugin_Output(); //empty
}

