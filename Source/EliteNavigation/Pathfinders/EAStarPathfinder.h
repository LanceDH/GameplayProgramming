/*=============================================================================*/
// Copyright 2017-2018 Elite Engine
// Authors: Matthieu Delaere
/*=============================================================================*/
// EAStarPathfinder.h: A* Pathfinder implementation using IPathfinder
/*=============================================================================*/
#include "../ENavigation.h"

#ifndef ELITE_NAVIGATION_ASTARPATHFINDER
#define ELITE_NAVIGATION_ASTARPATHFINDER
namespace Elite
{
	class AStarPathfinder final : public Pathfinder<Node>
	{
	public:
		//--- Constructor & Destructor ---
		AStarPathfinder() :Pathfinder() {}
		~AStarPathfinder() = default; //Non virtual Destructor

		//--- Pathfinder Functions ---
		std::vector<Vector2> FindPath(
			Graph<Node>* pGraph,
			Node* pStartNode, Node* pEndNode,
			Heuristic heuristicFunction, int limit = -1);
	};
}
#endif