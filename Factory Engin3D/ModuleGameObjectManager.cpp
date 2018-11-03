#include "Application.h"

#include "ModuleGameObjectManager.h"


ModuleGameObjectManager::ModuleGameObjectManager(Application * app, bool start_enabled) : Module(app, start_enabled)
{
}

ModuleGameObjectManager::~ModuleGameObjectManager()
{

}

bool ModuleGameObjectManager::Start()
{
	root = new GameObject(float3::zero, Quat::identity, float3::one, nullptr, "Scene");

	return true;
}

update_status ModuleGameObjectManager::PreUpdate(float dt)
{
	return UPDATE_CONTINUE;
}

update_status ModuleGameObjectManager::Update(float dt)
{
	root->Update(dt);

	return UPDATE_CONTINUE;
}

update_status ModuleGameObjectManager::PostUpdate(float dt)
{
	for (list<GameObject*>::iterator iterator = gameObjects.begin(); iterator != gameObjects.end(); ++iterator)
	{
		if ((*iterator)->toDelete)
		{
			delete (*iterator);
		}
	}
	return UPDATE_CONTINUE;
}

bool ModuleGameObjectManager::CleanUp()
{
	CleanAllGameObjects();

	return true;
}

void ModuleGameObjectManager::CleanAllGameObjects()
{
	// Deleting a GameObject will cause all his childs to be deleted
	// If we delete the root GameObject, all the GameObjects will be deletet recursively
	if (root != nullptr)
	{
		delete root;
		root = nullptr;
	}
}

GameObject* ModuleGameObjectManager::CreateGameObject(float3 position, Quat rotation, float3 scale, GameObject* father, char* name)
{
	GameObject* newGameObject = nullptr;

	if (father == nullptr)
		father = root;

	newGameObject = new GameObject(position, rotation, scale, father, name);
	
	father->childs.push_back(newGameObject);

	return newGameObject;
}