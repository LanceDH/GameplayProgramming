#include "stdafx.h"
#include "SteeringBehaviours.h"
#include "IExamInterface.h"

//SEEK
//****
SteeringPlugin_Output Seek::CalculateSteering(float deltaT, AgentInfo pAgent)
{
	SteeringPlugin_Output steering = {};

	steering.LinearVelocity = (*m_pTargetRef).Position - pAgent.Position; //Desired Velocity
	steering.LinearVelocity.Normalize(); //Normalize Desired Velocity
	steering.LinearVelocity *= pAgent.MaxLinearSpeed; //Rescale to Max Speed
	
	//DEBUG RENDERING
	/*if (pAgent->CanRenderBehaviour() && m_pContext)
	{
		m_pContext->pRenderer->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, steering.LinearVelocity.Magnitude(), { 0, 1, 0 ,0.5f }, 0.40f);
	}*/

	return steering;
}

//WANDER (base> SEEK)
//******
SteeringPlugin_Output Wander::CalculateSteering(float deltaT, AgentInfo pAgent)
{
	//Calculate WanderOffset
	auto offset = pAgent.LinearVelocity;
	offset.Normalize();
	offset *= m_Offset;

	//WanderCircle Offset (Polar to Cartesian Coordinates)
	Vector2 circleOffset = { cos(m_WanderAngle) * m_Radius, sin(m_WanderAngle) * m_Radius };

	//Change the WanderAngle slightly for next frame
	m_WanderAngle += randomFloat() * m_AngleChange - (m_AngleChange * .5f); //RAND[-angleChange/2,angleChange/2]

																			//Set target as Seek::Target
	auto newTarget = TargetData(pAgent.Position + offset + circleOffset);
	Seek::m_pTargetRef = &newTarget;

	//DEBUG RENDERING
	/*if (pAgent->CanRenderBehaviour() && m_pContext)
	{
		auto pos = pAgent->GetPosition();
		m_pContext->pRenderer->DrawSegment(pos, pos + offset, { 0,0,1 ,0.5f});
		m_pContext->pRenderer->DrawCircle(pos + offset, m_Radius, { 0,0,1 ,0.5f }, -0.7f);
		m_pContext->pRenderer->DrawSolidCircle(pos + offset + circleOffset, 0.1f, { 0,0 }, { 0, 1, 0 ,0.5f }, -0.75f);
	}*/

	//Return Seek Output (with our wander target)
	return Seek::CalculateSteering(deltaT, pAgent);
}

//PURSUIT (base> SEEK)
//*******
SteeringPlugin_Output Pursuit::CalculateSteering(float deltaT, AgentInfo pAgent)
{
	auto distanceToTarget = Elite::Distance(pAgent.Position, (*m_pTargetRef).Position);
	auto offsetFromTarget = distanceToTarget / pAgent.MaxLinearSpeed;
	auto targetDirection = (*m_pTargetRef).LinearVelocity.GetNormalized();

	auto newTarget = TargetData((*m_pTargetRef).Position + (targetDirection * offsetFromTarget));
	Seek::m_pTargetRef = &newTarget;

	//DEBUG RENDERING
	/*if (pAgent->CanRenderBehaviour() && m_pContext)
	{
		m_pContext->pRenderer->DrawSolidCircle(newTarget.Position, 0.3f, { 0,0 }, { 1, 0, 1, 0.5f }, 0.4f);
	}*/

	return Seek::CalculateSteering(deltaT, pAgent);
}

//FLEE
//****
SteeringPlugin_Output Flee::CalculateSteering(float deltaT, AgentInfo pAgent)
{
	auto steering = Seek::CalculateSteering(deltaT, pAgent);
	
	steering.LinearVelocity = steering.LinearVelocity * -1.0f;

	//DEBUG RENDERING
	/*if (pAgent->CanRenderBehaviour() && m_pContext)
	{
		auto pos = pAgent->GetPosition();
		auto magn = steering.LinearVelocity.Magnitude();
		m_pContext->pRenderer->DrawDirection(pos, steering.LinearVelocity, magn, { 0, 1, 0 ,0.5f }, 0.40f);
	}*/

	return steering;
}

//EVADE (base> FLEE)
//*****
SteeringPlugin_Output Evade::CalculateSteering(float deltaT, AgentInfo pAgent)
{
	auto distanceToTarget = Elite::Distance(pAgent.Position, (*m_pTargetRef).Position);

	auto offsetFromTarget = distanceToTarget / pAgent.MaxLinearSpeed;
	auto targetDirection = (*m_pTargetRef).LinearVelocity.GetNormalized();

	auto newTarget = TargetData((*m_pTargetRef).Position + (targetDirection * offsetFromTarget));
	Flee::m_pTargetRef = &newTarget;

	//DEBUG RENDERING
	/*if (pAgent->CanRenderBehaviour() && m_pContext)
	{
		m_pContext->pRenderer->DrawSolidCircle(newTarget.Position, 0.3f, { 0,0 }, { 1, 0, 1, 0.5f }, 0.4f);
	}*/

	return Flee::CalculateSteering(deltaT, pAgent);
}

//ARRIVE
//******
SteeringPlugin_Output Arrive::CalculateSteering(float deltaT, AgentInfo pAgent)
{
	SteeringPlugin_Output steering = {};

	steering.LinearVelocity = (*m_pTargetRef).Position - pAgent.Position; //Total_Velocity to Target
	auto distance = steering.LinearVelocity.Normalize() - m_TargetRadius; //Total_Distance to Target + Normalize Total_Velocity 

	if (distance < m_SlowRadius) //Inside SlowRadius
	{
		steering.LinearVelocity *= pAgent.MaxLinearSpeed * (distance / (m_SlowRadius + m_TargetRadius)); //Slow DownS
	}
	else
	{
		steering.LinearVelocity *= pAgent.MaxLinearSpeed; //Move to target at max speed
	}

	//DEBUG RENDERING
	/*if (pAgent->CanRenderBehaviour() && m_pContext)
	{
		auto pos = pAgent->GetPosition();
		m_pContext->pRenderer->DrawCircle(pos, m_SlowRadius, { 0,0,1 ,0.5f }, 0.7f);
		m_pContext->pRenderer->DrawCircle(pos, m_TargetRadius, { 1.0f, 0.271f, 0.f ,0.5f }, 0.7f);
		m_pContext->pRenderer->DrawDirection(pos, steering.LinearVelocity, steering.LinearVelocity.Magnitude(), { 0, 1, 0 ,0.5f }, 0.40f);
	}
*/
	return steering;
}

//ALIGN
//*****
SteeringPlugin_Output Align::CalculateSteering(float deltaT, AgentInfo pAgent)
{
	SteeringPlugin_Output steering = {};

	auto rotation = (*m_pTargetRef).Orientation - pAgent.Orientation;//->GetRotation();

	//Wrap Angle
	auto a = fmodf(rotation + b2_pi, 2 * b2_pi);
	rotation = a >= 0 ? (a - b2_pi) : (a + b2_pi);

	auto rotationSize = abs(rotation);

	if (rotationSize <= 0 || rotationSize < m_TargetAngle)
		return steering; //Should be empty :)

	auto targetRotationSpeed = pAgent.MaxAngularSpeed;
	if (rotationSize < m_SlowAngle)
	{
		targetRotationSpeed = pAgent.MaxAngularSpeed * rotationSize / m_SlowAngle;
	}

	targetRotationSpeed *= rotation / rotationSize;
	steering.AngularVelocity = targetRotationSpeed - pAgent.AngularVelocity;
	steering.AngularVelocity /= m_TimeToTarget;

	auto angularSpeed = abs(steering.AngularVelocity);
	if (angularSpeed > pAgent.MaxAngularSpeed)
	{
		steering.AngularVelocity /= angularSpeed;
		steering.AngularVelocity *= pAgent.MaxAngularSpeed;
	}

	steering.LinearVelocity = ZeroVector2;
	return steering;
}

//FACE
//****
SteeringPlugin_Output Face::CalculateSteering(float deltaT, AgentInfo pAgent)
{
	SteeringPlugin_Output steering = {};

	auto direction = (*m_pTargetRef).Position - pAgent.Position;
	auto distance = direction.Magnitude();

	if (distance == 0.f)
		return steering; //Should be empty :)

	//Assemble Align Target (= new orientation > face target)
	auto newTarget = TargetData(*m_pTargetRef);
	newTarget.Orientation = atan2f(direction.x, -direction.y);

	//ALIGN BEHAVIOUR
	Align::m_pTargetRef = &newTarget;
	return Align::CalculateSteering(deltaT, pAgent);
}

SteeringPlugin_Output FacedArrive::CalculateSteering(float deltaT, AgentInfo pAgent)
{
	SteeringPlugin_Output steering = {};

	Arrive arriveBeh = Arrive();
	arriveBeh.SetTarget(m_pTargetRef);
	steering.LinearVelocity = arriveBeh.CalculateSteering(deltaT, pAgent).LinearVelocity;

	Face faceBeh = Face();
	faceBeh.SetTarget(m_pTargetRef);
	steering.AngularVelocity = faceBeh.CalculateSteering(deltaT, pAgent).AngularVelocity;

	return steering;
}

SteeringPlugin_Output AvoidObstacle::CalculateSteering(float deltaT, AgentInfo pAgent)
{
	auto normVel = pAgent.LinearVelocity;	
	auto lengthVel = normVel.Normalize();

	auto dynamicLength = lengthVel / pAgent.MaxLinearSpeed * 20.f;
	auto ahead = pAgent.Position + (normVel * dynamicLength);
	auto ahead2 = pAgent.Position + (normVel * dynamicLength * 0.5f);

	bool set;
	auto mostThreatening = FindMostThreateningObstacle(ahead, ahead2, pAgent.Position, pAgent.AgentSize, set);

	/*if(set && m_pContext && pAgent->CanRenderBehaviour())
	{
		m_pContext->pRenderer->DrawCircle(ahead, 0.5f, { 1,1,0 }, 0.4f);
		m_pContext->pRenderer->DrawCircle(mostThreatening.center, mostThreatening.radius, { 0,1,0 }, 0.4f);
	}*/

	SteeringPlugin_Output steering = {};
	steering.LinearVelocity = ahead - mostThreatening.center;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= m_MaxAvoidanceForce;
	steering.AngularVelocity = 50.f;

	//steering. = set; //Priority Steering

	return steering;
}

Obstacle AvoidObstacle::FindMostThreateningObstacle(const Vector2& ahead, const Vector2& ahead2, const Vector2& position, float radius, bool& set)
{
	Obstacle mostThreatening = (m_Obstacles.size()>0)?m_Obstacles[0]: Obstacle();
	set = false;

	for (auto obstacle : m_Obstacles)
	{
		obstacle.radius += radius;
		auto collision = LineIntersectsCircle(ahead, ahead2, obstacle);

		// "position" is the character's current position
		if (collision && (set || Distance(position, obstacle.center) < Distance(position, mostThreatening.center))) {
			mostThreatening = obstacle;
			set = true;
		}
	}
	
	return mostThreatening;
}

float AvoidObstacle::Distance(Vector2 a, Vector2 b) const
{
	return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

bool AvoidObstacle::LineIntersectsCircle(Vector2 ahead, Vector2 ahead2, Obstacle obstacle) const
{
	return Distance(obstacle.center, ahead) <= obstacle.radius || Distance(obstacle.center, ahead2) <= obstacle.radius;
}
