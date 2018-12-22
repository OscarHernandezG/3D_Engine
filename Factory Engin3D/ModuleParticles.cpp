#include "ModuleParticles.h"
#include <algorithm>

#include "Brofiler/Brofiler.h"

ModuleParticle::ModuleParticle(Application* app, bool start_enabled) : Module(app, start_enabled)
{

}

ModuleParticle::~ModuleParticle()
{
	delete plane;
}

update_status ModuleParticle::Update()
{
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::PapayaWhip);

	for (std::list<ComponentEmitter*>::iterator emitter = emitters.begin(); emitter != emitters.end(); ++emitter)
	{
		(*emitter)->Update();
	}

	int count = 0;

	float dt;
	if(App->time->gameState == GameState_NONE)
		dt = App->time->Getdt();
	else
		dt = App->time->GetdtGame();

	partVec.resize(activeParticles);
	
	int j = 0;
	for (int i = 0; i < MAX_PARTICLES; ++i)
	{
		if (allParticles[i].active)
		{
			allParticles[i].Update(dt); //Particles can be created here, they sould not be updated yet
			++count;
			allParticles[i].SetCamDistance();
			//partQueue.push(&allParticles[i]);
			partVec[j++] = &allParticles[i];
		}
		else
		{
			allParticles[i].camDistance = -1;
		}
	}
		LOG("Active particles %i", count);
	return UPDATE_CONTINUE;
}

void ModuleParticle::Draw()
{
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::PapayaWhip);

	if (plane == nullptr)
		plane = new ParticlePlane();

	SortParticles();

	DrawParticles();
}

void ModuleParticle::DrawParticles()
{
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::PapayaWhip);

	//while (!partQueue.empty())
	//{
	//	Particle* currPart = partQueue.top();

	//	//if (currPart->owner && currPart->owner->gameObject->canDraw)
	//		currPart->Draw();

	//	partQueue.pop();
	//}

	for (int i = 0; i < partVec.size(); ++i)
	{
		if (partVec[i]->owner && partVec[i]->owner->gameObject->canDraw)
			partVec[i]->Draw();
	}
}

void ModuleParticle::SortParticles()
{
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::PapayaWhip);

	std::sort(partVec.begin(), partVec.end(), particleCompare());
}

bool ModuleParticle::GetParticle(int& id)
{
	for (int i = lastUsedParticle; i < MAX_PARTICLES; ++i) 
	{
		if (!allParticles[i].active) 
		{
			lastUsedParticle = i;
			id = i;
			return true;
		}
	}

	for (int i = 0; i < lastUsedParticle; ++i)
	{
		if (!allParticles[i].active)
		{
			lastUsedParticle = i;
			id = i;
			return true;
		}
	}
	return false;
}

void ModuleParticle::ClearEmitters()
{
	for (std::list<ComponentEmitter*>::iterator emitter = emitters.begin(); emitter != emitters.end(); ++emitter)
	{
		(*emitter)->SoftClearEmitter();
	}
	emitters.clear();

	for (int i = 0; i < MAX_PARTICLES; ++i)
	{
		allParticles[i].active = false;
		allParticles[i].owner = nullptr;		
	}

	lastUsedParticle = 0;
}
