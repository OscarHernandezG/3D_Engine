#ifndef __Light_H__
#define __Light_H__

#include "Color.h"
#include "MathGeoLib/Math/float3.h"


struct Light
{
	Light();

	void Init();
	void SetPos(float x, float y, float z);
	void SetPos(float3 pos);
	void Active(bool active);
	void Render();

	Color ambient;
	Color diffuse;
	float3 position;

	int ref;
	bool on;
};
#endif // !__Light_H__

