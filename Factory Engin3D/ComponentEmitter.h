#ifndef __Emitter_H__
#define __Emitter_H__

#include "Component.h"
#include "GameObject.h"

#include "Timer.h"

#include <list>

struct ColorTime
{
	float4 color = float4::one;
	float position = 0.0f;
	std::string name = " ";
	//open window for change particle color
	bool changingColor = false;

	bool operator<(const ColorTime &color) const
	{
		return position < color.position;
	}
};

struct StartValues
{
	// Start values
	float life = 5.0f;
	float speed = 3.0f;
	float size = 1.0f;
	float rotation = 0.0f;
	std::list<ColorTime> color;
	bool timeColor = false;

	AABB colision = AABB(float3(-0.5f, -0.5f, -0.5f), float3(0.5f, 0.5f, 0.5f));

	float3 particleDirection = float3::unitY;

	StartValues()
	{
		ColorTime colorTime;
		colorTime.name = "Start Color";
		color.push_back(colorTime);
	}
};

enum ShapeType {
	ShapeType_BOX,
	ShapeType_SPHERE,
	ShapeType_SPHERE_CENTER,
	ShapeType_SPHERE_BORDER
};
class ComponentEmitter : Component
{
public:
	ComponentEmitter(GameObject* gameObject);
	~ComponentEmitter();

	void Update();
	float3 RandPos();
	void Inspector();

	void EditColor(ColorTime & colorTime);

	ImVec4 EqualsFloat4(const float4 float4D);

public:
	Timer timer;

	std::list<Particle*> particles;

	// Particle texture
	ResourceTexture* texture = nullptr;

private:
	// General info
	//---------------------------------------
	// Duration of the particle emitter
	float duration = 1.0f;
	
	// Loop the particle (if true the particle emitter will never stop)
	bool loop = true;
	Timer loopTimer;
	// Warm up the particle emiter (if true the particle emitter will be already started at play-time)
	bool preWarm = true;

	float gravity = 0.0f;

	//Posibility space where particle is created
	AABB boxCreation = AABB(float3(-0.5f, -0.5f, -0.5f), float3(0.5f, 0.5f, 0.5f));
	Sphere SphereCreation = Sphere(float3::zero, 1.0f);

	ShapeType shapeType = ShapeType_BOX;

	float nextPos = 0.0f;
	float4 nextColor = float4::zero;

	StartValues startValues;
	//---------------------------------------

	// Emission info
	//---------------------------------------
	// Number of particles created per second
	int rateOverTime = 10;
	//---------------------------------------

	
};
#endif // !__Emitter_H__