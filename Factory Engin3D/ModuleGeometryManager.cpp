#include "Application.h"
#include "ModuleGeometryManager.h"
#include "ModuleImporter.h"

#include "glew-2.1.0/include/GL/glew.h"

#include "Assimp/include/cimport.h"
#include "Assimp/include/scene.h"
#include "Assimp/include/postprocess.h"
#include "Assimp/include/cfileio.h"

#include "DevIL/includex86/IL/il.h"
#include "DevIL/includex86/IL/ilu.h"
#include "DevIL/includex86/IL/ilut.h"

#include <fstream>

#pragma comment( lib, "DevIL/libx86/DevIL.lib" )
#pragma comment( lib, "DevIL/libx86/ILU.lib" )
#pragma comment( lib, "DevIL/libx86/ILUT.lib" )



ModuleGeometry::ModuleGeometry(Application* app, bool start_enabled) : Module(app, start_enabled)
{
}

ModuleGeometry::~ModuleGeometry()
{}

// Load assets
bool ModuleGeometry::Start()
{
	LOG("Loading Geometry manager");


	ilutRenderer(ILUT_OPENGL);
	ilInit();
	iluInit();
	ilutInit();
	ilutRenderer(ILUT_OPENGL);
	bool ret = true;


	LoadDefaultScene();
	
	return ret;
}


update_status ModuleGeometry::PreUpdate(float dt)
{
	update_status status = UPDATE_CONTINUE;

	return status;
}

// Load assets
bool ModuleGeometry::CleanUp()
{
	return true;
}

void ModuleGeometry::DistributeFile(char* file)
{
	string filePath(file);
	string extension = filePath.substr(filePath.find_last_of(".") + 1);


	if (!extension.compare("fbx") || !extension.compare("obj"))
	{
		UpdateMesh(file);
	}
	else if (!extension.compare("png") || !extension.compare("dds"))
		UpdateTexture(file);
}


Mesh* ModuleGeometry::LoadMesh(char* path)
{
	Mesh* mesh = nullptr;
	if (path != nullptr)
	{
		char* filePath = path;
		const aiScene* scene = aiImportFile(filePath, aiProcessPreset_TargetRealtime_MaxQuality);

		if (scene != nullptr) 
		{
			if (scene->HasMeshes())
			{
				mesh = new Mesh();
				numFaces = 0u;
				for (int i = 0; i < scene->mNumMeshes; ++i)
				{
					aiMesh* newMesh = scene->mMeshes[i];

					MeshBuffer newCurrentBuffer;
					newCurrentBuffer.vertex.size = newMesh->mNumVertices * 3;
					newCurrentBuffer.vertex.buffer = new float[newCurrentBuffer.vertex.size * 3];

					memcpy(newCurrentBuffer.vertex.buffer, newMesh->mVertices, sizeof(float) * newCurrentBuffer.vertex.size);

					LOG("New mesh loaded with %d vertices", newCurrentBuffer.vertex.size);

					/*Dont do it now
					glGenBuffers(1, (GLuint*)&(newCurrentBuffer.vertex.id));
					glBindBuffer(GL_ARRAY_BUFFER, newCurrentBuffer.vertex.id);
					glBufferData(GL_ARRAY_BUFFER, sizeof(float) * newCurrentBuffer.vertex.size, newCurrentBuffer.vertex.buffer, GL_STATIC_DRAW);
					*/
					if (newMesh->HasFaces())
					{

						numFaces += newMesh->mNumFaces;
						newCurrentBuffer.index.size = newMesh->mNumFaces * 3;
						newCurrentBuffer.index.buffer = new uint[newCurrentBuffer.index.size];

						for (uint index = 0; index < newMesh->mNumFaces; ++index)
						{
							if (newMesh->mFaces[index].mNumIndices != 3)
								LOG("WARNING, geometry faces != 3 indices")
							else
							{
								memcpy(&newCurrentBuffer.index.buffer[index * 3], newMesh->mFaces[index].mIndices, sizeof(uint) * 3);
							}

						}
						/*Dont do it now
						glGenBuffers(1, (GLuint*)&(newCurrentBuffer.index.id));
						glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, newCurrentBuffer.index.id);
						glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * newCurrentBuffer.index.size, newCurrentBuffer.index.buffer, GL_STATIC_DRAW);
						*/
					}

					if (newMesh->HasTextureCoords(0))
					{
						newCurrentBuffer.texture.size = newMesh->mNumVertices * 2;
						newCurrentBuffer.texture.buffer = new float[newMesh->mNumVertices * 2];
						for (int currVertices = 0; currVertices < newMesh->mNumVertices; ++currVertices)
						{
							newCurrentBuffer.texture.buffer[currVertices * 2] = newMesh->mTextureCoords[0][currVertices].x;
							newCurrentBuffer.texture.buffer[currVertices * 2 + 1] = newMesh->mTextureCoords[0][currVertices].y;
						}
						memcpy(newCurrentBuffer.texture.buffer, newMesh->mTextureCoords[0], sizeof(float) * 2 * newMesh->mNumVertices);

						/*Dont do it now
						glGenBuffers(1, &newCurrentBuffer.texture.id);
						glBindBuffer(GL_ARRAY_BUFFER, newCurrentBuffer.texture.id);
						glBufferData(GL_ARRAY_BUFFER, newMesh->mNumVertices * sizeof(float) * 2, textCoords, GL_STATIC_DRAW);
						glBindBuffer(GL_ARRAY_BUFFER, 0);
						*/

					}

					mesh->buffers.push_back(newCurrentBuffer);

					SaveMeshImporter(newCurrentBuffer,path,i);
				}
///
			currentMeshBB = LoadBoundingBox(mesh);
			LOG("Loaded geometry with %i faces", numFaces);
			}
			
				aiReleaseImport(scene);
		}

		else
			LOG("Error loading geometry %s", filePath);

	}
	return mesh;
}

void ModuleGeometry::SaveMeshImporter(MeshBuffer newCurrentBuffer, const char* path, int number)
{
	uint ranges[3] = { newCurrentBuffer.index.size, newCurrentBuffer.vertex.size, newCurrentBuffer.texture.size};

	float size = sizeof(ranges) + sizeof(uint) * newCurrentBuffer.index.size + sizeof(float) * newCurrentBuffer.vertex.size * 3;

	if(newCurrentBuffer.texture.buffer != nullptr)
		size += sizeof(float) * newCurrentBuffer.texture.size * 2;

	char* exporter = new char[size];
	char* cursor = exporter;

	uint bytes = sizeof(ranges);
	memcpy(cursor, ranges, bytes);

	cursor += bytes;
	bytes = sizeof(uint)* newCurrentBuffer.index.size;
	memcpy(cursor, newCurrentBuffer.index.buffer, bytes);

	cursor += bytes;
	bytes = sizeof(float)* newCurrentBuffer.vertex.size;
	memcpy(cursor, newCurrentBuffer.vertex.buffer, bytes);

	if (newCurrentBuffer.texture.buffer != nullptr)
	{
		cursor += bytes;
		bytes = sizeof(float)* newCurrentBuffer.texture.size;
		memcpy(cursor, newCurrentBuffer.texture.buffer, bytes);
	}

	App->importer->SaveFile(path,size,exporter, LlibraryType_MESH, number);
	
	delete[] exporter;
}

MeshBuffer ModuleGeometry::LoadMeshImporter(const char* path)
{
	char* buffer = App->importer->LoadFile(path, LlibraryType_MESH);
	char* cursor = buffer;
	MeshBuffer bufferImporter;

	uint ranges[3];

	uint bytes = sizeof(ranges);
	memcpy(ranges, cursor, bytes);

	bufferImporter.index.size = ranges[0];
	bufferImporter.vertex.size = ranges[1];
	bufferImporter.texture.size = ranges[2];

	cursor += bytes;
	bytes = sizeof(uint)* bufferImporter.index.size;
	bufferImporter.index.buffer = new uint[bufferImporter.index.size];
	memcpy(bufferImporter.index.buffer, cursor, bytes);

	glGenBuffers(1, (GLuint*)&(bufferImporter.index.id));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferImporter.index.id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * bufferImporter.index.size, bufferImporter.index.buffer, GL_STATIC_DRAW);

	cursor += bytes;
	bytes = sizeof(float)* bufferImporter.vertex.size;
	bufferImporter.vertex.buffer = new float[bufferImporter.vertex.size];
	memcpy(bufferImporter.vertex.buffer, cursor, bytes);

	glGenBuffers(1, (GLuint*)&(bufferImporter.vertex.id));
	glBindBuffer(GL_ARRAY_BUFFER, bufferImporter.vertex.id);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bufferImporter.vertex.size, bufferImporter.vertex.buffer, GL_STATIC_DRAW);

	cursor += bytes;
	bytes = sizeof(float)* bufferImporter.texture.size;
	bufferImporter.texture.buffer = new float[bufferImporter.texture.size];
	memcpy(bufferImporter.texture.buffer, cursor, bytes);

	glGenBuffers(1, &bufferImporter.texture.id);
	glBindBuffer(GL_ARRAY_BUFFER, bufferImporter.texture.id);
	glBufferData(GL_ARRAY_BUFFER, bufferImporter.texture.size * sizeof(float) * 2, bufferImporter.texture.buffer, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	delete[] buffer;
	return bufferImporter;
}

void ModuleGeometry::UpdateMesh(char* path)
{
	Mesh* tempMesh = LoadMesh(path);

	if (tempMesh != nullptr)
		if (!tempMesh->buffers.empty())
		{
			tempMesh->buffers.push_back(LoadMeshImporter(path));
			currentMesh->ClearMesh();
			currentMesh = tempMesh;

		}

	
}

AABB* ModuleGeometry::LoadBoundingBox(Mesh* mesh)
{
	AABB* boundingBox = nullptr;
	if (mesh != nullptr)
	{
		float3 max, min;
		std::vector<MeshBuffer>::iterator iterator = mesh->buffers.begin();
		max.x = (*iterator).vertex.buffer[0];
		max.y = (*iterator).vertex.buffer[1];
		max.z = (*iterator).vertex.buffer[2];

		min = max;


		for (iterator; iterator != mesh->buffers.end(); ++iterator)
		{
			int vertexSize = (*iterator).vertex.size / 3;
			float* buffer = (*iterator).vertex.buffer;
			for (int i = 0; i < vertexSize; ++i)
			{
				Higher(max.x, buffer[i * 3]);
				Higher(max.y, buffer[i * 3 + 1]);
				Higher(max.z, buffer[i * 3 + 2]);

				Lower(min.x, buffer[i * 3]);
				Lower(min.y, buffer[i * 3 + 1]);
				Lower(min.z, buffer[i * 3 + 2]);
			}
		}

		boundingBox = new AABB(min, max);

		App->camera->Position = CalcBBPos(boundingBox);
		App->camera->Look(CalcBBPos(boundingBox), mesh->GetPos(), false);
	}
	return boundingBox;
}

float3 ModuleGeometry::CalcBBPos(math::AABB* boundingBox) const
{
	float3 distance{ 0,0,0 };
	if (boundingBox != nullptr)
	{
		float3 size = boundingBox->Size();

		float reScale = 1.25;
		distance.x = (size.x / 2) / math::Tan(0.33333333333 * reScale);
		distance.y = (size.y / 2) / math::Tan(0.33333333333 * reScale);
		distance.z = (size.z / 2) / math::Tan(0.33333333333 * reScale);
	}
	return distance;
}

float3 ModuleGeometry::GetBBPos() const
{
	float3 distance{ 0,0,0 };
	if (currentMeshBB != nullptr)
	{
		float3 size = currentMeshBB->Size();

		float reScale = 1.25;
		distance.x = (size.x / 2) / math::Tan(0.33333333333 * reScale);
		distance.y = (size.y / 2) / math::Tan(0.33333333333 * reScale);
		distance.z = (size.z / 2) / math::Tan(0.33333333333 * reScale);
	}
	return distance;
}

float3 ModuleGeometry::GetCurrentMeshPivot() const
{
	return currentMesh->GetPos();
}


void ModuleGeometry::Higher(float& val1, float val2)
{
	val1 = val1 > val2 ? val1 : val2;
}

void ModuleGeometry::Lower(float& val1, float val2)
{
	val1 = val1 < val2 ? val1 : val2;
}

uint ModuleGeometry::LoadTexture(char* path) const
{
	bool isSuccess = true;
	uint newTextureID = 0;

	ilGenImages(1, &newTextureID);
	ilBindImage(newTextureID);

	if ((bool)ilLoadImage(path))
	{
		ILinfo info;
		iluGetImageInfo(&info);
		if (info.Origin == IL_ORIGIN_UPPER_LEFT)
		{
			iluFlipImage();
		}

		if (ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE))
		{
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

			glGenTextures(1, &newTextureID);
			glBindTexture(GL_TEXTURE_2D, newTextureID);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

			glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_FORMAT), ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT),
				0, ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE, ilGetData());
		}
		else
		{
			isSuccess = false;
			LOG("Image conversion error %s", iluErrorString(ilGetError()));
		}
	}
	else
	{
		isSuccess = false;
		LOG("Error loading texture %s", iluErrorString(ilGetError()));
	}

	ilDeleteImages(1, &newTextureID);

	if (!isSuccess)
		newTextureID = 0;

	return newTextureID;
}

void ModuleGeometry::UpdateTexture(char* path)
{
	uint tempTexture = LoadTexture(path);
	if (tempTexture != 0)
	{
		if (textureID != 0)
		{
			glDeleteTextures(1, &textureID);
		}
		textureID = tempTexture;
	}
}


// Update
update_status ModuleGeometry::Update(float dt)
{
	return UPDATE_CONTINUE;
}

update_status ModuleGeometry::PostUpdate(float dt)
{
	return UPDATE_CONTINUE;
}

void ModuleGeometry::Draw3D(bool fill, bool wire) const
{
	PrimitivePlane plane;
	plane.color = { 1, 1, 1, 1 };
	plane.axis = true;
	plane.Render();

	if (currentMesh != nullptr) 
	{
		currentMesh->fill = fill;
		currentMesh->wire = wire;
		currentMesh->Render();
	}
}

void ModuleGeometry::LoadDefaultScene()
{
	currentMesh = LoadMesh("assets\\models\\BakerHouse.fbx");
	textureID = LoadTexture("assets\\textures\\Baker_house.dds");
}