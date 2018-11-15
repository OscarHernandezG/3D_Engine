#include "Application.h"

#include "GameObject.h"
#include "Texture.h"
#include "pcg-c-basic-0.9/pcg_basic.h"


GameObject::GameObject(GameObject* father, const char * name)
{
	this->father = father;

	if (name != nullptr)
		this->name = name;
	else this->name = "noName";
}

GameObject::GameObject(float3 position, Quat rotation, float3 scale, GameObject* father, const char* name)
{
	TransformInfo* info = new TransformInfo();
	info->position = position;
	info->rotation = rotation;
	info->scale = scale;
	info->whichInfo = UsingInfo_TRS;

	CreateGameObject(info);
	delete info;

	this->father = father;

	if (name != nullptr)
		this->name = name;
	else this->name = "noName";
}


GameObject::~GameObject()
{
	transform = nullptr;

	RemoveComponents();

	RemoveChilds();
}

void GameObject::RemoveChilds()
{
	for (list<GameObject*>::iterator iterator = childs.begin(); iterator != childs.end(); ++iterator)
	{
		delete (*iterator);
	}
	childs.clear();
}

void GameObject::RemoveComponents()
{
	for (list<Component*>::iterator iterator = components.begin(); iterator != components.end(); ++iterator)
	{
		delete (*iterator);
	}
	components.clear();
}

void GameObject::RealDelete()
{
	transform = nullptr;

	if (father)
		father->childs.remove(this);

	father = nullptr;
	//delete this;
}

void GameObject::Update(float dt)
{
	if (isActive)
	{
		for (list<Component*>::iterator iterator = components.begin(); iterator != components.end(); ++iterator)
		{
			(*iterator)->Update(dt);
		}

		for (list<GameObject*>::iterator iterator = childs.begin(); iterator != childs.end(); ++iterator)
		{
			(*iterator)->Update(dt);
		}
	}
}

bool GameObject::SetParent(GameObject* parent)
{
	bool ret = false;
	if (parent != nullptr)
	{
		this->father = parent;
		parent->childs.push_back(this);
		ret = true;
	}

	return ret;
}

void GameObject::CreateFromJson(JSON_Object* info, vector<uint>& meshesToLoad)
{
	UID = json_object_get_number(info, "UUID");
	parentUUID = json_object_get_number(info, "Parent UUID");

	JSON_Array* components = json_object_get_array(info, "Components");

	for (int i = 0; i < json_array_get_count(components); i++) {
		JSON_Object* comp = json_array_get_object(components, i);

		ComponentType type = (ComponentType)(int)json_object_get_number(comp, "Type");
		if (type != ComponentType_GEOMETRY)
			Component* newComponent = AddComponent(type, LoadComponentInfo(comp, type));

		else if (type == ComponentType_GEOMETRY)
		{
			uint compUUID = json_object_get_number(comp, "UUID");

			Mesh* mesh = new Mesh(this);
			mesh->SetUUID(compUUID);

			GeometryInfo inf;
			inf.geometry = mesh;
			Component* newComponent = AddComponent(type, &inf);

			meshesToLoad.push_back(compUUID);
		}
	}
}

ComponentInfo* GameObject::LoadComponentInfo(JSON_Object* info, ComponentType type)
{
	ComponentInfo* ret = nullptr;

	switch (type)
	{
	case ComponentType_TRANSFORM:
	{
		TransformInfo* compInfo = new TransformInfo();

//// Position
////------------------------------------------------------------------------
		JSON_Object* position = json_object_get_object(info, "Position");

		float3 pos;
		pos.x = json_object_get_number(position, "X");
		pos.y = json_object_get_number(position, "Y");
		pos.z = json_object_get_number(position, "Z");

//// Scale
////------------------------------------------------------------------------
		JSON_Object* scale = json_object_get_object(info, "Scale");

		float3 size;
		size.x = json_object_get_number(scale, "X");
		size.y = json_object_get_number(scale, "Y");
		size.z = json_object_get_number(scale, "Z");

//// Rotation
////------------------------------------------------------------------------
		JSON_Object* rotation = json_object_get_object(info, "Rotation");

		Quat rot;
		rot.x = json_object_get_number(rotation, "X");
		rot.y = json_object_get_number(rotation, "Y");
		rot.z = json_object_get_number(rotation, "Z");
		rot.w = json_object_get_number(rotation, "W");

////------------------------------------------------------------------------
		compInfo->UUID = json_object_get_number(info, "UUID");

		
		compInfo->position = pos;
		compInfo->rotation = rot.Normalized();
		compInfo->scale = size;


		ret = (ComponentInfo*)compInfo;
	}
		break;
	case ComponentType_GEOMETRY:
	{
		GeometryInfo* compInfo = new GeometryInfo();

		compInfo->UUID = json_object_get_number(info, "UUID");

		ret = (ComponentInfo*)compInfo;
	}
		break;
	case ComponentType_CAMERA:
		// TODO
		break;
	case ComponentType_TEXTURE:
		// TODO
		break;
	case ComponentType_LIGHT:
		// TODO
		break;
	default:
		break;
	}

	return ret;
}

void GameObject::CreateGameObject(TransformInfo* info)
{
	UID = pcg32_random();

	this->transform = (Transform*)AddComponent(ComponentType_TRANSFORM, info);
	if(transform)
		transform->SetUUID(pcg32_random());
}

//Components
//-------------------------------------------------------------------------
	// Add
Component* GameObject::AddComponent(ComponentType type, ComponentInfo* info)
{
	Component* newComponent = nullptr;

	if (info)
		info->gameObject = this;

	switch (type)
	{
	case ComponentType_TRANSFORM:
		if (info)
			newComponent = (Component*)new Transform((TransformInfo*)info);
		if (newComponent)
		{
			components.remove(transform);
			delete transform;

			transform = (Transform*)newComponent;
		}
			break;
	case ComponentType_GEOMETRY:
		if (info)
			newComponent = (Component*)(((GeometryInfo*)info)->geometry);
		break;
	case ComponentType_CAMERA:
		newComponent = (Component*)new Camera(this);
		break;
	case ComponentType_TEXTURE:
		newComponent = (Component*)new Texture(this, (TextureInfo*)info);
		break;
	case ComponentType_LIGHT:
		break;
	default:
		break;
	}

	if (newComponent)
	{
		newComponent->type = type;
		newComponent->isActive = true;
		newComponent->gameObject = this;
		components.push_back(newComponent);
	}

	return newComponent;
}
	// Remove
void GameObject::RemoveComponent(Component* component)
{
	if (component != nullptr)
	{
		components.remove(component);
		delete component;
		component = nullptr;
	}
}
	// Get
Component* GameObject::GetComponent(ComponentType type)
{
	Component* component = nullptr;

	for (list<Component*>::iterator iterator = components.begin(); iterator != components.end(); ++iterator)
	{
		if ((*iterator)->type == type)
		{
			component = (*iterator);
			break;
		}
	}

	return component;
}

list<Component*> GameObject::GetAllComponent(ComponentType type)
{
	list<Component*> component;

	for (list<Component*>::iterator iterator = components.begin(); iterator != components.end(); ++iterator)
	{
		if ((*iterator)->type == type)
		{
			components.push_back(*iterator);
		}
	}

	return component;
}


bool GameObject::HasComponent(ComponentType type)
{
	bool ret = false;
	for (list<Component*>::iterator iterator = components.begin(); iterator != components.end(); ++iterator)
	{
		if ((*iterator)->type == type)
		{
			ret = true;
			break;
		}
	}
	return ret;
}
//-------------------------------------------------------------------------


//Position
//-------------------------------------------------------------------------
	// Set
void GameObject::SetPos(float3 pos)
{
	if (transform)
	{
		transform->SetPos(pos);
	}
}

void GameObject::Move(float3 movement)
{
	if (transform)
		transform->Move(movement);
}
	// Get
float3 GameObject::GetPos() const
{
	if (transform)
		return transform->GetPos();

	return float3::zero;
}

float3 GameObject::GetGlobalPos() const
{
	if (transform)
		return transform->GetGlobalPos();

	return float3::zero;
}
//-------------------------------------------------------------------------

float3 GameObject::GetScale() const
{
	if (transform)
		return transform->GetScale();

	return float3::one;
}

float3 GameObject::GetGlobalScale() const
{
	if (transform)
		return transform->GetGlobalScale();

	return float3::one;
}

float4x4 GameObject::GetGlobalMatrix() const
{
	float4x4 mat = float4x4::identity;
	if (transform)
	{
		mat = transform->GetMatrix();
	}

	return mat;
}

float4x4 GameObject::GetLocalMatrix() const
{
	float4x4 mat = float4x4::identity;
	if (transform)
	{
		mat = transform->GetLocalMatrix();
	}

	return mat;
}

Quat GameObject::GetRotation() const
{
	if (transform)
		return transform->GetRotation().Normalized();
}

Quat GameObject::GetGlobalRotation() const
{
	if (transform)
		return transform->GetGlobalRotation().Normalized();
}

void GameObject::SetTransform(float4x4 trans)
{
	if (transform)
	{
		transform->SetTransform(trans);
	}
}



void GameObject::SetScale(float3 scale)
{
	if (transform)
		transform->SetScale(scale);
}

void GameObject::Scale(float3 scale)
{
	if (transform)
		transform->Scale(scale);
}

void GameObject::SetRotation(Quat rotation)
{
	if (transform)
		transform->SetRotation(rotation);
}

void GameObject::Rotate(Quat rotation)
{
	if (transform)
		transform->Rotate(rotation);
}

void GameObject::SetIdentity()
{
	if (transform)
	{
		transform->SetIdentity();
		transform->UpdateBoundingBox();
	}
}

const AABB* GameObject::GetAABB() const
{
	return &transform->boundingBox;
}

float3 GameObject::GetBBPos()
{
	float3 distance{ 0,0,0 };
	if (transform)
	{
		float3 size = transform->boundingBox.Size();

		if (!childs.empty())
		{
			size = GetGlobalAABB(AABB(float3::zero,float3::zero)).Size();
		}

		float3 reScale = GetScale();
		distance.x = (size.x / 2) / math::Tan((0.3333333 * reScale.x));
		distance.y = (size.y / 2) / math::Tan((0.3333333 * reScale.y));
		distance.z = (size.z / 2) / math::Tan((0.3333333 * reScale.z));
	}
	return distance + GetPos();
}

AABB GameObject::GetGlobalAABB(AABB localAABB)
{
	AABB globalAABB = localAABB;
	if (!childs.empty())
		for (list<GameObject*>::iterator iterator = childs.begin(); iterator != childs.end(); ++iterator)
		{
			globalAABB = (*iterator)->GetGlobalAABB(globalAABB);
		}
		
	else if(HasComponent(ComponentType_GEOMETRY))
	{
		float3 objectMin = GetAABB()->minPoint;
		float3 objectMax = GetAABB()->maxPoint;

		if (globalAABB.MinX() > objectMin.x)
			globalAABB.minPoint.x = objectMin.x;
		if (globalAABB.MinY() > objectMin.y)
			globalAABB.minPoint.y = objectMin.y;
		if (globalAABB.MinZ() > objectMin.z)
			globalAABB.minPoint.z = objectMin.z;

		if (globalAABB.MaxX() < objectMax.x)
			globalAABB.maxPoint.x = objectMax.x;
		if (globalAABB.MaxY() < objectMax.y)
			globalAABB.maxPoint.y = objectMax.y;
		if (globalAABB.MaxZ() < objectMax.z)
			globalAABB.maxPoint.z = objectMax.z;
	}
	return globalAABB;
}

void GameObject::SetABB(AABB aabb)
{
	if (transform)
	{
		transform->originalBoundingBox = aabb;

		transform->UpdateBoundingBox();
	}
}

void GameObject::SetActive(bool active)
{
	for (list<GameObject*>::iterator iterator = childs.begin(); iterator != childs.end(); ++iterator)
	{
		(*iterator)->SetActive(active);
	}
	isActive = active;
}

void GameObject::SetObjectStatic(bool isStatic)
{
	for (list<GameObject*>::iterator iterator = childs.begin(); iterator != childs.end(); ++iterator)
	{
		(*iterator)->SetObjectStatic(isStatic);
	}
	this->isStatic = isStatic;
}

int GameObject::CreateRandomUID()
{
	return 0;
}
