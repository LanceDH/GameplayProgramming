//=== General Includes ===
#include "stdafx.h"
#include "EAStarPathfinder.h"
using namespace Elite;

//=== Pathfinder Functions ===
std::vector<Vector2> AStarPathfinder::FindPath(
	Graph<Node>* pGraph, Node* pStartNode, Node* pEndNode, Heuristic heuristicFunction, int limit)
{
	//Variables
	std::vector<Vector2> vPath;

	//Temp containers
	std::vector<Connection*> openList;
	std::vector<Connection*> closedList;
	//Temp variables
	Connection* pCurrentConnection = nullptr;
	bool reachedGoal = false;
	int calcCount = 0;

	//STEP 1: Start by calculating costs for first possible connections of the 'pStartNode' and adding them to the 'openList'. 
	//This will kickstart the loop!
	for (auto c : pStartNode->GetConnections())
	{
		CalculateCosts(c, pStartNode, pEndNode, heuristicFunction);
		openList.push_back(c);
	}

	//Start algorithm loop: while our open list is not empty
	while (openList.size() != 0)
	{
		//cout << openList.size() << " - " << closedList.size() << endl;
		//STEP 2.1: Get the connection with the lowest F score from the 'openList' and set it as 'pCurrentConnection'
		pCurrentConnection = openList.at(0);
		for (size_t i = 1; i < openList.size(); i++) {
			if (openList.at(i)->GetHCost() < pCurrentConnection->GetHCost()) {
			//if (openList.at(i)->GetFCost() < pCurrentConnection->GetFCost()) {
				pCurrentConnection = openList.at(i);
			}
		}

		//STEP 2.2: Pop 'pCurrentConnection' off the 'openList' and push it to the 'closedList'
		//Hint:use the "remove-erase idiom"
		openList.erase(std::remove(openList.begin(), openList.end(), pCurrentConnection), openList.end());
		closedList.push_back(pCurrentConnection);

		//STEP 2.3: Retrieve the 'connections' for the 'pCurrentConnection's 'EndNode'. 
		//We will use these to check if the goal is present and/or if the need to be added to the openlist later.
		std::vector<Connection*> vpConnections = pCurrentConnection->GetEndNode()->GetConnections();

		//STEP 2.4: If any of the retrieved successors ('vpConnections') is the goal (== connection.'EndNode' == 'pEndNode'), call it a day! 
		//Hint: use std::find_if!
		auto result = std::find_if(vpConnections.begin(), vpConnections.end()
			, [pEndNode](auto pCon) {return pCon->GetEndNode() == pEndNode; });
		//If something has been found:
		if (result != vpConnections.end())
		{
			//Set the 'HeadConnection' of this found connection ('result') equal to the 'pCurrentConnection'.
			(*result)->SetHeadConnection(pCurrentConnection);
			//Set this connection ('result') as 'pCurrentConnection' (so we can retrace the path).
			pCurrentConnection = (*result);
			//Break the loop by clearing open and breaking!
			openList.clear();
			reachedGoal = true;
			break;
		}

		//STEP 2.5: Else go over all the retrieved connections
		for (auto pC : vpConnections) {
			//2.5.1: If found in 'closedList', do nothing
			bool isFound = false;
			for (Connection* pCon : closedList) {
				if (pC == pCon) {
					isFound = true;
					break;
				}
			}
			if (!isFound) {
				//2.5.2: Else:
				//Link the connection by setting the connections its 'HeadConnection' equal to the 'pCurrentConnection' (retrace path).
				//Calculate the costs
				//Add it to the 'openList'
				pC->SetHeadConnection(pCurrentConnection);
				CalculateCosts(pC, pC->GetEndNode(), pEndNode, heuristicFunction);
				openList.push_back(pC);
			}
		}
		// Go until limit
		if (limit > 0 && ++calcCount > limit) {
			//cout << "Stopped at: " << calcCount << endl;
			vPath.clear();
			return vPath;
		}

	}


	if (!reachedGoal) {
		vPath.clear();
		//cout << "fail: " << calcCount << endl;
		return vPath;
	}

	if (calcCount > 100) {
		//cout << "success: " << calcCount << endl;
	}
	
	//STEP 3: Reconstruct path
	//As long as the 'pCurrentConnection' its 'StartNode' is not equal to the 'pStartNode'
		//Store the 'pCurrentConnection' its 'EndNode' position in the 'vPath' container
		//Change the 'pCurrentConnection' to the 'pCurrentConnection' its 'HeadConnection'
	while (pCurrentConnection->GetStartNode() != pStartNode) {
		vPath.push_back(pCurrentConnection->GetEndNode()->GetPosition());
		pCurrentConnection = pCurrentConnection->GetHeadConnection();
	}
	int i = 5;
	//To finalize add last retrieved 'EndNode' position and the position of 'pStartNode'
	vPath.push_back(pCurrentConnection->GetEndNode()->GetPosition());
	vPath.push_back(pStartNode->GetPosition());
	
	//Reverse 'vPath' to go from start to goal
	reverse(vPath.begin(), vPath.end());

	return vPath;
}