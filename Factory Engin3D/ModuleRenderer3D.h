#pragma once
#include "Module.h"
#include "Globals.h"
#include "MathGeoLib/Math/float3x3.h"
#include "MathGeoLib/Math/float4x4.h"
#include "Light.h"

#define MAX_LIGHTS 8

class ModuleRenderer3D : public Module
{
public:
	ModuleRenderer3D(Application* app, bool start_enabled = true);
	~ModuleRenderer3D();

	bool Init();
	bool Start();
	update_status PreUpdate(float dt);
	update_status PostUpdate(float dt);
	bool CleanUp();

	void OnResize(int width, int height);

	void SetLightAmbient();


	math::float4x4 Perspective(float fovy, float aspect, float n, float f) const;

	update_status Save(JSON_Object * object);
	update_status Load(JSON_Object * object);

public:

	Light lights[MAX_LIGHTS];
	SDL_GLContext context;
	float3x3 NormalMatrix;
	float4x4 ModelMatrix, ViewMatrix, ProjectionMatrix;

	bool vsync = false;
	uint my_id = 0u;

	bool isFill = true;
	bool isWire = true;
	uint buffer_id = 0u;

	float2 prevSize{ SCREEN_WIDTH,SCREEN_HEIGHT };
	float2 currSize{ SCREEN_WIDTH,SCREEN_HEIGHT };

	float3 ambient_lihgt = { 1.0f,1.0f,1.0f };

};

