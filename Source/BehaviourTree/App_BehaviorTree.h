#ifndef BEHAVIOR_TREE_APPLICATION_H
#define BEHAVIOR_TREE_APPLICATION_H
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/interfaces/IApp.h"
#include "../App_SteeringBehaviours/Pipeline/SteeringPipeline.h"
class Guard;

//-----------------------------------------------------------------
// Application
//-----------------------------------------------------------------
class App_BehaviorTree : public IApp
{
public:
	//Constructor & Destructor
	App_BehaviorTree(Context* context);
	virtual ~App_BehaviorTree() final;

	//App Functions
	void Start() override;
	void ProcessEvents(const SDL_Event& e) override;
	void Update(float deltaTime) override;
	void Render(float deltaTime) const override;

private:
	//Datamembers
	Guard* m_pGuard = nullptr;
	Goal m_Goal = {};

	//C++ make the class non-copyable
	App_BehaviorTree(const App_BehaviorTree&) {};
	App_BehaviorTree& operator=(const App_BehaviorTree&) {};
};
#endif