#pragma once

#include "MathGeoLib/MathGeoLib.h"
#include "Geometries.h"

#include "Globals.h"

template <typename T>
struct Buffer
{
	uint id;
	uint size;
	T* buffer;
};

struct MeshBuffer
{
	Buffer<uint> index;
	Buffer<float> vertex;
	Buffer<float> texture;
};

class Mesh :public Geometry
{
public:
	Mesh() {};
	~Mesh() { ClearMesh(); };

	void InnerRender() const;

	void ClearMesh();

public:
	std::vector<MeshBuffer> buffers;

};