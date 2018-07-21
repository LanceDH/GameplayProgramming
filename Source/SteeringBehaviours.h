#pragma once
#include "SteeringHelpers.h"
#include "IExamInterface.h"
class SteeringAgent;
using namespace Elite;

#pragma region **ISTEERINGBEHAVIOUR** (BASE)
class ISteeringBehaviour
{
public:
	ISteeringBehaviour() {}
	virtual ~ISteeringBehaviour(){}

	virtual SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo pAgent) = 0;

	template<class T, typename std::enable_if<std::is_base_of<ISteeringBehaviour, T>::value>::type* = nullptr>
	T* As()
	{
		return static_cast<T*>(this);
	}

	int temp_id = 0;

};
#pragma endregion

///////////////////////////////////////
//SEEK
//****
class Seek : public ISteeringBehaviour
{
public:
	Seek() {};
	virtual ~Seek() {};

	//Seek Behaviour
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo pAgent) override;

	//Seek Functions
	virtual void SetTarget(const TargetData* pTarget) { m_pTargetRef = pTarget; }

protected:
	const TargetData* m_pTargetRef = nullptr;
};

//////////////////////////
//WANDER
//******
class Wander : public Seek
{
public:
	Wander() {};
	virtual ~Wander() {};

	//Wander Behaviour
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo pAgent) override;

	void SetWanderOffset(float offset) { m_Offset = offset; }
	void SetWanderRadius(float radius) { m_Radius = radius; }
	void SetMaxAngleChange(float rad) { m_AngleChange = rad; }

protected:

	float m_Offset = 6.f; //Offset (Agent Direction)
	float m_Radius = 4.f; //WanderRadius
	float m_AngleChange = ToRadians(45); //Max WanderAngle change per frame
	float m_WanderAngle = 0.f; //Internal

private:
	void SetTarget(const TargetData* pTarget) override {} //Hide SetTarget, No Target needed for Wander
};

////////////////////////////
//PURSUIT
//******
class Pursuit : public Seek
{
public:
	Pursuit() {};
	virtual ~Pursuit() {};

	//Pursuit Behaviour
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo pAgent) override;

	//Pursuit Functions (Hides Seek::SetTarget)
	virtual void SetTarget(const TargetData* pTarget) override { m_pTargetRef = pTarget; }

protected:
	const TargetData* m_pTargetRef = nullptr;
};

/////////////////////////
//FLEE
//****
class Flee : public Seek
{
public:
	Flee() {};
	virtual ~Flee() {};

	//Seek Behaviour
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo pAgent) override;
};

//////////////////////////
//EVADE
//*****
class Evade : public Flee
{
public:
	Evade() {};
	virtual ~Evade() {};

	//Evade Behaviour
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo pAgent) override;

	//Evade Functions (Hides Flee::SetTarget)
	void SetTarget(const TargetData* pTarget) override { m_pTargetRef = pTarget; }

protected:
	const TargetData* m_pTargetRef = nullptr;
};

/////////////////////////////////////////
//ARRIVE
//******
class Arrive : public ISteeringBehaviour
{
public:
	Arrive() {};
	virtual ~Arrive() {};

	//Arrive Behaviour
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo pAgent) override;

	//Arrive Functions
	virtual void SetTarget(const TargetData* pTarget) { m_pTargetRef = pTarget; }
	void SetSlowRadius(float radius) { m_SlowRadius = radius; }
	void SetTargetRadius(float radius) { m_TargetRadius = radius; }

protected:

	const TargetData* m_pTargetRef = nullptr;
	float m_SlowRadius = 15.f;
	float m_TargetRadius = 3.f;
};

/////////////////////////////////////////
//ALIGN
//******
class Align : public ISteeringBehaviour
{
public:
	Align() {}
	virtual ~Align() {}

	//Align Fuctions
	virtual void SetTarget(const TargetData* pTarget) { m_pTargetRef = pTarget; }

	void SetTargetAngle(float rad) { m_TargetAngle = rad; }
	void SetSlowAngle(float rad) { m_SlowAngle = rad; }
	void SetTimeToTarget(float time) { m_TimeToTarget = time; }

	//Align Behaviour
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo pAgent) override;

protected:
	const TargetData* m_pTargetRef = nullptr;

private:

	float m_TargetAngle = ToRadians(1);
	float m_SlowAngle = ToRadians(15);
	float m_TimeToTarget = 1.f;
};

/////////////////////////////////////////
//FACE
//******
class Face : public Align
{
public:
	Face() {}
	virtual ~Face() {}

	//Face Functions (Align::SetTarget override)
	void SetTarget(const TargetData* pTarget) override { m_pTargetRef = pTarget; }

	//Face Behaviour
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo pAgent) override;

protected:
	const TargetData* m_pTargetRef; //Allign::m_pTargetRef 'override' > hiding :)
};

//FACED_ARRIVE
//************
class FacedArrive : public ISteeringBehaviour
{
public:
	FacedArrive() {};
	virtual ~FacedArrive() {};

	//FacedArrive Functions
	virtual void SetTarget(const TargetData* pTarget) { m_pTargetRef = pTarget; }

	//Arrive Behaviour
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo pAgent) override;

protected:
	const TargetData* m_pTargetRef = nullptr;
};


//AVOID-OBSTACLE
struct Obstacle
{
	Vector2 center;
	float radius;
};

class AvoidObstacle : public ISteeringBehaviour
{
public:
	AvoidObstacle(vector<Obstacle> obstacles) :ISteeringBehaviour(),m_Obstacles(obstacles) {};
	virtual ~AvoidObstacle() {};

	//AvoidObstacle Behaviour
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo pAgent) override;

	//AvoidObstacle Functions
	void SetMaxAvoidanceForce(float max) { m_MaxAvoidanceForce = max; }

protected:

	vector<Obstacle> m_Obstacles = {};
	float m_MaxAvoidanceForce = 10.f;

private:
	Obstacle FindMostThreateningObstacle(const Vector2& ahead, const Vector2& ahead2, const Vector2& position, float radius, bool& set);
	float Distance(Vector2 a, Vector2 b) const;
	bool LineIntersectsCircle(Vector2 ahead, Vector2 ahead2, Obstacle obstacle) const;
		
};

