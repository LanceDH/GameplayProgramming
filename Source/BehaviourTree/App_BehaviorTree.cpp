//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_BehaviorTree.h"
#include "Guard.h"

//Constructor & Destructor
App_BehaviorTree::App_BehaviorTree(Context* context) : IApp(context)
{}

App_BehaviorTree::~App_BehaviorTree()
{
	SAFE_DELETE(m_pGuard);
}

//Functions
void App_BehaviorTree::Start()
{
	//Initialization of your application. If you want access to the physics world you will need to store it yourself.
	m_pGuard = new Guard();
	m_pGuard->Initialize(IApp::m_context);
}

void App_BehaviorTree::ProcessEvents(const SDL_Event& e)
{
	//Seperate events process function because we can have multiple events per frame!
	switch (e.type)
	{
		case SDL_MOUSEBUTTONUP:
		{
			if (e.button.button == SDL_BUTTON_LEFT)
			{
				int x, y;
				SDL_GetMouseState(&x, &y);
				Vector2 pos = Vector2((float)x, (float)y);
				m_Goal.Position = m_context->pRenderer->GetActiveCamera()->ConvertScreenToWorld(pos);
				m_Goal.PositionSet = true;
				break;
			}
		}
	}
}

void App_BehaviorTree::Update(float deltaTime)
{
	//Update that is being called after the physics simulation
	(void)deltaTime;
	m_pGuard->Update(deltaTime, m_Goal.Position, m_Goal.PositionSet);
	//Reset flag
	m_Goal.PositionSet = false;

	/*auto pBb = m_pBehaviorTree->GetBlackboard();
	if (pBb != nullptr)
		pBb->ChangeData("DeltaTime", deltaTime);
	
	auto rv = m_pBehaviorTree->Update();
	if(rv == Success)
		printf("SUCCESS \n");*/
}

void App_BehaviorTree::Render(float deltaTime) const
{
	//Render function for non-Box2D rendering (actors are being automatically rendered based on their shape and state)
	//If you want to render objects in screen space you will need to convert it yourself to World Space using the active camera, for example:
	//b2Vec2 wPosition = m_context->renderer->GetActiveCamera()->ConvertScreenToWorld(b2Vec2(10, 10));
	//m_context->renderer->DrawPoint(wPosition, 10, b2Color(0, 1, 0));
}