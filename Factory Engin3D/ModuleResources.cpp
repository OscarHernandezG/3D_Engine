#include "Globals.h"
#include "Application.h"
#include "ModuleResources.h"

#include "glew-2.1.0/include/GL/glew.h"

#include "ModuleParticles.h"

#pragma comment( lib, "DevIL/libx86/DevIL.lib" )
#pragma comment( lib, "DevIL/libx86/ILU.lib" )
#pragma comment( lib, "DevIL/libx86/ILUT.lib" )

#include "DevIL/includex86/IL/il.h"
#include "DevIL/includex86/IL/ilu.h"
#include "DevIL/includex86/IL/ilut.h"

#include <fstream>

using namespace std;
ModuleResources::ModuleResources(Application * app, bool start_enabled): Module(app, start_enabled)
{
}

ModuleResources::~ModuleResources()
{
}



bool ModuleResources::Init()
{
	return true;
}

update_status ModuleResources::Update()
{
	if (particleTextureUV.empty())
	{
		uint textureID = 0;
		float texture[]
		{
			0.0f, 0.0f,
			1.0f, 0.0f,
			0.0f, 1.0f,
			1.0f, 1.0f,
		};

		glGenBuffers(1, (GLuint*)&(textureID));
		glBindBuffer(GL_ARRAY_BUFFER, textureID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, texture, GL_STATIC_DRAW);
		//12 = All vertex positions (2 * 6) 2 = vertices and 6 = pos x-y
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		defaultTextureUV.textureIDs.push_back(textureID);

		defaultTextureUV.columns = defaultTextureUV.rows = 1;

		App->particle->particleAnimation = defaultTextureUV;
		particleTextureUV.push_back(defaultTextureUV);
	}
	return UPDATE_CONTINUE;
}

bool ModuleResources::CleanUp()
{
	return true;
}

bool ModuleResources::ExistFile(const char * path)
{
	bool ret = false;

	ifstream loadFile(path);
	if (loadFile.is_open())
		ret = true;

	return ret;
}

void ModuleResources::ReadFolder(const char * path, std::vector<AssetsHierarchy*>& filesStr)
{
	for (std::vector<AssetsHierarchy*>::iterator iterator = filesStr.begin(); iterator != filesStr.end(); ++iterator)
	{
		delete *iterator;
	}

	filesStr.clear();
	string currPath(path);
	
	if (GetFileAttributes(currPath.data()) != INVALID_FILE_ATTRIBUTES)
	{
		currPath.append("*");
		WIN32_FIND_DATA data;
		HANDLE file;
		if ((file = FindFirstFile(currPath.data(), &data)) != INVALID_HANDLE_VALUE)
		{
			do {
				if (strcmp(data.cFileName, "..") && strcmp(data.cFileName, ".") && strcmp(data.cFileName, "Scenes"))
				{
					int vector = filesStr.size();

					AssetsHierarchy* internal = new AssetsHierarchy(data.cFileName); //Set Name
					filesStr.push_back(internal);									//Push empty childs

					string newPath(path);
					newPath.append(data.cFileName);
					newPath += "\\";

					ReadFolder(newPath.data(), filesStr[vector]->childFiles);		//Get childs
				}
			} while (FindNextFile(file, &data) != 0);
		}
		FindClose(file);
	}
	/*else if (ExistFile(path))
	{
		bool hi = true;
	}*/
}

void ModuleResources::Remove(Resource* resource)
{
	if (resource &&	IsResourceLoaded(resource))
	{
		resource->usage--;

		if (resource->usage <= 0)
		{
			//resources.remove(resource);
			//delete resource;
		}
	}
}


Resource* ModuleResources::FindLoadedResource(const char* path, ResourceType type)
{
	for (list<Resource*>::const_iterator iterator = resources.begin(); iterator != resources.end(); ++iterator)
	{
		if ((*iterator)->type == type)
			if (strcmp((*iterator)->file.data(), path) == 0)
			{
				return *iterator;
			}
	}
	return nullptr;
}

bool ModuleResources::IsResourceLoaded(Resource* res)
{
	for (list<Resource*>::const_iterator iterator = resources.begin(); iterator != resources.end(); ++iterator)
	{
		if (*iterator == res)
			return true;

	}
	return false;
}

ResourceMesh* ModuleResources::LoadMesh(uint name)
{
	string assetName = std::to_string(name).data();
	assetName += ".fty";
	
	ResourceMesh* mesh = (ResourceMesh*)FindLoadedResource(assetName.data(), ResourceType::mesh);

	if (mesh)
	{
		return mesh;
	}
	else
	{
		char* buffer = App->importer->LoadFile("", LlibraryType_MESH, name);
		if (buffer)
		{
			if (RealLoadMesh(buffer, mesh, assetName.data()))
			{
				mesh->uuid = name;
				resources.push_back(mesh);
			}
		}
		return mesh;
	}
}

ResourceMesh* ModuleResources::LoadMesh(const MeshNode& node)
{
	string assetName = std::to_string(node.componentUUID).data();
	assetName += ".fty";

	ResourceMesh* mesh = (ResourceMesh*)FindLoadedResource(assetName.data(), ResourceType::mesh);

	if (mesh)
	{
		return mesh;
	}
	else
	{
		char* buffer = App->importer->LoadFile("", LlibraryType_MESH, node.componentUUID);
		if (buffer)
		{
			if (RealLoadMesh(buffer, mesh, assetName.data()))
			{
				mesh->name = node.name;
				mesh->uuid = node.componentUUID;
				resources.push_back(mesh);
			}
		}
		return mesh;
	}
}

bool ModuleResources::RealLoadMesh(char* buffer, ResourceMesh*& mesh, const char* name)
{
	ResourceMesh* bufferImporter = new ResourceMesh(name);
	char* cursor = buffer;

	uint ranges[4];

	uint bytes = sizeof(ranges);
	memcpy(ranges, cursor, bytes);

	bufferImporter->index.size = ranges[0];
	bufferImporter->vertex.size = ranges[1];
	bufferImporter->texture.size = ranges[2];
	bufferImporter->color.size = ranges[3];

	// Index
	LoadIndexGPU(cursor, bytes, bufferImporter);

	// Vertex
	LoadVertexGPU(cursor, bytes, bufferImporter);

	//Texture UV
	LoadTextureUVGPU(cursor, bytes, bufferImporter);

	// Color
	//LoadColorGPU(cursor, bytes, bufferImporter);


	// Clear buffers
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	mesh = bufferImporter;
	
	delete buffer;

	return (bufferImporter != nullptr);
}

void ModuleResources::LoadColorGPU(char * &cursor, uint &bytes, ResourceMesh * bufferImporter)
{
	cursor += bytes;
	bytes = sizeof(float)* bufferImporter->color.size;
	bufferImporter->color.buffer = new float[bufferImporter->color.size];
	memcpy(bufferImporter->color.buffer, cursor, bytes);

	glGenBuffers(1, &bufferImporter->color.id);
	glBindBuffer(GL_ARRAY_BUFFER, bufferImporter->color.id);
	glBufferData(GL_ARRAY_BUFFER, bufferImporter->color.size * sizeof(float), bufferImporter->color.buffer, GL_STATIC_DRAW);
}

void ModuleResources::LoadTextureUVGPU(char * &cursor, uint &bytes, ResourceMesh * bufferImporter)
{
	cursor += bytes;
	bytes = sizeof(float)* bufferImporter->texture.size;
	bufferImporter->texture.buffer = new float[bufferImporter->texture.size];
	memcpy(bufferImporter->texture.buffer, cursor, bytes);

	glGenBuffers(1, &bufferImporter->texture.id);
	glBindBuffer(GL_ARRAY_BUFFER, bufferImporter->texture.id);
	glBufferData(GL_ARRAY_BUFFER, bufferImporter->texture.size * sizeof(float), bufferImporter->texture.buffer, GL_STATIC_DRAW);
}

void ModuleResources::LoadVertexGPU(char * &cursor, uint &bytes, ResourceMesh * bufferImporter)
{
	cursor += bytes;
	bytes = sizeof(float)* bufferImporter->vertex.size;
	bufferImporter->vertex.buffer = new float[bufferImporter->vertex.size];
	memcpy(bufferImporter->vertex.buffer, cursor, bytes);

	glGenBuffers(1, (GLuint*)&(bufferImporter->vertex.id));
	glBindBuffer(GL_ARRAY_BUFFER, bufferImporter->vertex.id);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bufferImporter->vertex.size, bufferImporter->vertex.buffer, GL_STATIC_DRAW);
}

void ModuleResources::LoadIndexGPU(char * &cursor, uint &bytes, ResourceMesh * bufferImporter)
{
	cursor += bytes;
	bytes = sizeof(uint)* bufferImporter->index.size;
	bufferImporter->index.buffer = new uint[bufferImporter->index.size];
	memcpy(bufferImporter->index.buffer, cursor, bytes);

	glGenBuffers(1, (GLuint*)&(bufferImporter->index.id));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferImporter->index.id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * bufferImporter->index.size, bufferImporter->index.buffer, GL_STATIC_DRAW);
}

ResourceTexture* ModuleResources::LoadTexture(const char* path)
{
	if (path == "noTexture")
		return nullptr;

	Resource* resourceLoaded = FindLoadedResource(path, ResourceType::texture);
	if (resourceLoaded)
	{
		ResourceTexture* texture = (ResourceTexture*)resourceLoaded;
		return texture;
	}
	else
	{
		ResourceTexture* texture = nullptr;

		uint opengGlTexture = 0u;

		if (!RealLoadTexture(path, opengGlTexture))
			opengGlTexture = 0u;
		else
		{
			texture = new ResourceTexture(path);
			texture->SetID(opengGlTexture);

			std::string name = path;
			name = name = name.substr(name.find_last_of("\\") + 1);


			texture->name = name;
			resources.push_back((Resource*)texture);
		}

	return texture;
	}
}

bool ModuleResources::RealLoadTexture(const char* path, uint& opengGlTexture)
{
	bool ret = true;

	ILuint newTextureID = 0;
	ilGenImages(1, &newTextureID);
	ilBindImage(newTextureID);

	if ((bool)ilLoadImage(path))
	{
		ilEnable(IL_FILE_OVERWRITE);

		ILuint size;
		ILubyte *data;

		ilSetInteger(IL_DXTC_FORMAT, IL_DXT5);
		size = ilSaveL(IL_DDS, NULL, 0);
		if (size > 0) {
			data = new ILubyte[size];
			if (ilSaveL(IL_DDS, data, size) > 0)
				App->importer->SaveFile(path, size, (char*)data, LlibraryType_TEXTURE);
		}

		ILinfo info;
		iluGetImageInfo(&info);
		if (info.Origin == IL_ORIGIN_UPPER_LEFT)
		{
			iluFlipImage();
		}

		if (ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE))
		{
			glPixelStorei(GL_RGBA , 1);

			glGenTextures(1, &opengGlTexture);
			glBindTexture(GL_TEXTURE_2D, opengGlTexture);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

			glTexParameteri(GL_TEXTURE_2D, GL_RGBA16, GL_NEAREST);

			glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_FORMAT), ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT),
				0, ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE, ilGetData());
		}
		else
		{
			ret = false;
			LOG("Image conversion error, %s", iluErrorString(ilGetError()));
		}
	}
	else
	{
		ret = false;
		LOG("Error loading texture, %s", iluErrorString(ilGetError()));
	}

	ilDeleteImages(1, &newTextureID);

	return ret;
}

void ModuleResources::GetResources(std::vector<Resource*> &resource, ResourceType type)
{
	for (std::list<Resource*>::iterator iterator = resources.begin(); iterator != resources.end(); ++iterator)
	{
		if ((*iterator)->type == type)
		{
			resource.push_back(*iterator);
		}
	}
}


ParticleUV ModuleResources::LoadTextureUV(int rows, int columns)
{
	ParticleUV ret = defaultTextureUV;
	if (rows > 0 && rows > 0)
		if (!FindTextureUV(rows, columns, ret))
		{
			uint textureID = 0;
			ret.textureIDs.clear();
			ret.rows = rows;
			ret.columns = columns;

			float rowsScale = 1.0f / rows;
			float columnsScale = 1.0f / columns;

			for (int i = 0; i < rows; ++i)
			{
				for (int j = 0; j < columns; ++j)
				{
					float texture[]
					{
						j * columnsScale,					1.0f - (i * rowsScale + rowsScale),
						j * columnsScale + columnsScale,	1.0f - (i *rowsScale + rowsScale),
						j * columnsScale,					1.0f - i * rowsScale,
						j * columnsScale + columnsScale,	1.0f - i * rowsScale,
					};


					LOG("Texture UV: \n%.2f %.2f\n%.2f %.2f\n%.2f %.2f\n%.2f %.2f\n", texture[0], texture[1], texture[2], texture[3], texture[4], texture[5], texture[6], texture[7])

						glGenBuffers(1, (GLuint*)&(textureID));
					glBindBuffer(GL_ARRAY_BUFFER, textureID);
					glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, texture, GL_STATIC_DRAW);
					glBindBuffer(GL_ARRAY_BUFFER, 0);

					ret.textureIDs.push_back(textureID);
				}
			}
		}
	return ret;
}

bool ModuleResources::FindTextureUV(int rows, int columns, ParticleUV& textureUV)
{
	for (std::list<ParticleUV>::const_iterator iterator = particleTextureUV.begin(); iterator != particleTextureUV.end(); ++iterator)
	{
		if ((*iterator).rows == rows && (*iterator).columns == columns)
		{
			textureUV = *iterator;
			return true;
		}
	}
	return false;
}
