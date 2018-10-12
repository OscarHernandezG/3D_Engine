#pragma once
#include "Globals.h"

#include "MathGeoLib/MathGeoLib.h"
#include "Color.h"

enum PrimitiveTypes
{
	Primitive_Point,
	Primitive_Ray,
	Primitive_Plane,
	Primitive_Cube,
	Primitive_Sphere,
	Primitive_Cylinder,
	Primitive_Frustum
};

class Geometry
{
public:

	Geometry();

	virtual void	Render() const;
	virtual void	InnerRender() const;
	void			SetPos(float x, float y, float z);
	void			SetRotation(float angle, const float3 &u);
	void			Scale(float x, float y, float z);
	void			WireframeRender() const;
	PrimitiveTypes	GetType() const;

public:
	
	Color color;
	float4x4 transform;
	bool axis = false, wire = false, fill = true;

protected:
	PrimitiveTypes type;
};

// ============================================
class PrimitiveCube : public Geometry
{
public :
	PrimitiveCube();
	PrimitiveCube(float3 position, float sizeX, float sizeY, float sizeZ);

	void LoadCubeBuffers(float3 position, float sizeX, float sizeY, float sizeZ);
	void LoadCubeBuffers(float3 position, float3 size)
	{
		LoadCubeBuffers(position, size.x, size.y, size.z);
	}

	~PrimitiveCube();

	void InnerRender() const;

public:
	float3 size;
	uint myIndices = 0u;
	uint myVertices = 0u;
};

//// ============================================
class SpherePrim : public Geometry
{
public:
	SpherePrim();
	SpherePrim(float radius,int hortzontalCuts, int verticalCuts);
	void InnerRender() const;
public:
	float radius;
	int horizontalCuts;
	int verticalCuts;
};

//// ============================================
class PrimitiveCylinder : public Geometry
{
public:
	PrimitiveCylinder();
	PrimitiveCylinder(float radius, float height, int faces);
	void InnerRender() const;
public:
	float radius;
	float height;
	int faces;
};

class RayLine : public Geometry
{
public:
	RayLine();
	RayLine(float3 origin, float3 destination);
	void InnerRender() const;
public:
	float3 origin;
	float3 destination;
};

// ============================================
class PrimitivePlane : public Geometry
{
public:
	PrimitivePlane();
	PrimitivePlane(float3 normal, float d);
	void InnerRender() const;
public:
	float3 normal;
	float constant;
};

// ============================================
class PrimitiveFrustum : public Geometry
{
public:
	PrimitiveFrustum();
	PrimitiveFrustum(float highSizes, float lowSize, float3 position = { 0,0,0 }, float sizeX = 1, float sizeY = 1, float sizeZ = 1);
	~PrimitiveFrustum();

	void LoadFrustumBuffers(float highSizes = 1, float lowSize = 1, float3 position = { 0,0,0 }, float sizeX = 1, float sizeY = 1, float sizeZ = 1);

	void InnerRender() const;

public:
	float3 size;
	uint myIndices = 0u;
	uint myVertices = 0u;
};
