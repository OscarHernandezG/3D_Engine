#include "Globals.h"
#include "Application.h"
#include "ModuleImGui.h"
#include "ModuleWindow.h"

#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl2.h"
#include "SDL/include/SDL_opengl.h"

#include "pcg-c-0.94/extras/entropy.h"

#include "MathGeoLib/Geometry/GeometryAll.h"

#include "Primitive.h"

//RAM and CPU usage
#include "windows.h"
#include "psapi.h"

ModuleImGui::ModuleImGui(Application* app, bool start_enabled) : Module(app, start_enabled)
{
}

ModuleImGui::~ModuleImGui()
{}

// Load assets
bool ModuleImGui::Start()
{
	LOG("Loading Intro assets");

	bool ret = true;

	pcg32_srandom_r(&rng, 42u, 54u);

	return ret;
}

update_status ModuleImGui::PreUpdate(float dt)
{
	update_status status = UPDATE_CONTINUE;

	ImGui_ImplOpenGL2_NewFrame();
	ImGui_ImplSDL2_NewFrame(App->window->window);
	ImGui::NewFrame();

	if (showDemoWindow)
		ImGui::ShowDemoWindow(&showDemoWindow);

	if (exampleWindow)
		CreateExampleWindow();

	if (mathGeoLibWindow)
		CreateMGLWindow();

	if (randomNumberWindow)
		CreateRandomNumberWindow();

	if (aboutWindow)
		CreateAboutWindow();

	if (configurationWindow)
		CreateConfigWindow();

	if (consoleWindow)
		CreateConsole();

	status = CreateMainMenuBar();

	created = true;


	return status;
}

// Load assets
bool ModuleImGui::CleanUp()
{
	textBuff.clear();
	LOG("Unloading Intro scene");
	return true;
}

// Update
update_status ModuleImGui::Update(float dt)
{
	return UPDATE_CONTINUE;
}

update_status ModuleImGui::PostUpdate(float dt)
{

	PROCESS_MEMORY_COUNTERS counter;
	GetProcessMemoryInfo(GetCurrentProcess(), &counter, sizeof(counter));

	float ramInUse = counter.WorkingSetSize;
	ramInUse /= 1024;
	ramInUse /= 1024;

	ramLog.push_back(ramInUse);
	if (ramLog.size() > 75)
	{
		ramLog.erase(ramLog.begin());
	}
	test++;


	return UPDATE_CONTINUE;
}

void ModuleImGui::DrawUI()
{
	ImGui::Render();
	ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
}

update_status ModuleImGui::Save(JSON_Object* object)
{

	json_object_dotset_boolean(object, "editableValues.DemoWindow", showDemoWindow);
	json_object_dotset_boolean(object, "editableValues.ExampeWindow", exampleWindow);
	json_object_dotset_boolean(object, "editableValues.MathGeoLibWindow", mathGeoLibWindow);
	json_object_dotset_boolean(object, "editableValues.randomNumberWindow", randomNumberWindow);
	json_object_dotset_boolean(object, "editableValues.aboutWindow", aboutWindow);
	json_object_dotset_boolean(object, "editableValues.configurationWindow", configurationWindow);
	json_object_dotset_boolean(object, "editableValues.consoleWindow", consoleWindow);
	json_object_dotset_number(object, "window.height", heightPos);
	json_object_dotset_number(object, "window.width", widthPos);

	return UPDATE_CONTINUE;
}

update_status ModuleImGui::Load(JSON_Object * object)
{
	showDemoWindow = json_object_dotget_boolean(object, "editableValues.DemoWindow");
	exampleWindow = json_object_dotget_boolean(object, "editableValues.ExampeWindow");
	mathGeoLibWindow = json_object_dotget_boolean(object, "editableValues.MathGeoLibWindow");
	randomNumberWindow = json_object_dotget_boolean(object, "editableValues.randomNumberWindow");
	aboutWindow = json_object_dotget_boolean(object, "editableValues.aboutWindow");
	configurationWindow = json_object_dotget_boolean(object, "editableValues.configurationWindow");
	consoleWindow = json_object_dotget_boolean(object, "editableValues.consoleWindow");
				   
	heightPos = json_object_dotget_number(object, "window.height");
	widthPos = json_object_dotget_number(object, "window.width");

	SDL_SetWindowSize(App->window->window, widthPos, heightPos);
				   
	return UPDATE_CONTINUE;
}



//Create Windows----------------------------------------------------------
void ModuleImGui::CreateExampleWindow()
{
	ImGui::Begin("Hello World!", &exampleWindow);				// Create a window called "Hello, world!" and append into it.

	ImGui::Checkbox("Demo Window", &showDemoWindow);			// Edit bools storing our window open/close state
	ImGui::Checkbox("Another Window", &show_another_window);

	ImGui::SliderFloat("float", &f, 0.0f, 1.0f);				// Edit 1 float using a slider from 0.0f to 1.0f    
	ImGui::ColorEdit3("clear color", (float*)&clear_color);		// Edit 3 floats representing a color

	if (ImGui::Button("Button"))								// Buttons return true when clicked (most widgets return true when edited/activated)
		counter++;

	ImGui::SameLine();
	ImGui::Text("counter = %d", counter);

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();
}

void ModuleImGui::CreateMGLWindow()
{
	ImGui::Begin("MathGeoLib", &mathGeoLibWindow);
	ImGui::Text("Move the sliders to move the sphere in the X axis.");
	ImGui::Text("Both spheres are 1 unit in radius");

	ImGui::NewLine();

	ImGui::SliderFloat("Sphere 1 position", &sphere1Pos, 0, 5);
	ImGui::SliderFloat("Sphere 2 positon", &sphere2Pos, 0, 5);

	Sphere sphere1;
	sphere1.pos = { sphere1Pos,0,0 };
	sphere1.r = 1;

	Sphere sphere2;
	sphere2.pos = { sphere2Pos,0,0 };
	sphere2.r = 1;

	spheresColliding = sphere1.Intersects(sphere2);


	ImGui::NewLine();
	if (spheresColliding)
	{
		ImGui::Text("The spheres are colliding");
	}
	else
	{
		ImGui::Text("The spheres are not colliding");
	}
	ImGui::End();
}

void ModuleImGui::CreateRandomNumberWindow()
{
	ImGui::Begin("PCG", &randomNumberWindow);
	ImGui::Text("Create random numbers.");

	if (ImGui::Button("Get a random number (0.0-1.0)", ImVec2(300, 50)))
	{
		randomDoubleNum = ldexp(pcg32_random_r(&rng), -32);
		randNumTextDouble = to_string(randomDoubleNum);
	}

	if (randomDoubleNum > -1)
	{
		ImGui::Text("Your random number is:");
		ImGui::SameLine();
		ImGui::Text(randNumTextDouble.data());
	}


	ImGui::Text("Introduce min value:"); ImGui::SameLine();
	isMinSelected = ImGui::InputInt("", &num1);

	ImGui::Text("Introduce max value:"); ImGui::SameLine();
	isMaxSelected = ImGui::InputInt(" ", &num2);

	if (num1 > num2)
	{
		if (isMinSelected)
			num2 = num1;
		else if (isMaxSelected)
			num1 = num2;
	}

	string buttonText = "Get a random number between " + to_string(num1) + " and " + to_string(num2);
	if (ImGui::Button(buttonText.data(), ImVec2(400, 50)))
	{
		int range = num2 - num1 + 1;
		randomIntNum = pcg32_boundedrand_r(&rng, range);
		randomIntNum += num1;
		randNumTextInt = to_string(randomIntNum);

	}

	ImGui::Text("Your random number is:");
	ImGui::SameLine();
	ImGui::Text(randNumTextInt.data());

	ImGui::End();
}

void ModuleImGui::CreateAboutWindow()
{
	ImGui::Begin("About", &aboutWindow);
	ImGui::Text("Factory Engin3D");
	ImGui::Separator();
	ImGui::TextWrapped("This is an incredible description ^^");
	if (ImGui::Button("Our repository", ImVec2(150, 25)))
		ShellExecuteA(NULL, "Open", "https://github.com/OscarHernandezG/3D_Engine", NULL, NULL, SW_SHOWNORMAL);
	ImGui::Text("Did it by:");
	if (ImGui::Button("Oscar Hernandez", ImVec2(150, 25)))
		ShellExecuteA(NULL, "Open", "https://github.com/OscarHernandezG", NULL, NULL, SW_SHOWNORMAL);
	ImGui::SameLine();
	if (ImGui::Button("Aleix Gabarro", ImVec2(150, 25)))
		ShellExecuteA(NULL, "Open", "https://github.com/aleixgab", NULL, NULL, SW_SHOWNORMAL);
	ImGui::Separator();
	ImGui::Text("Libraries that we used in this engine");
	///	Todo: ask libraries their version

	//IMGUI BUTTON Link
	string nameStr = "ImGui ";
	nameStr += ImGui::GetVersion();
	if (ImGui::Button(nameStr.data(), ImVec2(125, 20)))
		ShellExecuteA(NULL, "Open", "https://github.com/ocornut/imgui/releases", NULL, NULL, SW_SHOWNORMAL);

	//SDL BUTTON Link
	SDL_version current;
	SDL_VERSION(&current);
	char nameChar[25];
	sprintf_s(nameChar, 25, "SDL %d.%d.%d", current.major, current.minor, current.patch);
	if (ImGui::Button(nameChar, ImVec2(125, 20)))
		ShellExecuteA(NULL, "Open", "https://www.libsdl.org/download-2.0.php", NULL, NULL, SW_SHOWNORMAL);

	//OPENGL BUTTON Link
	int major = 3, minor = 1;
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);
	sprintf_s(nameChar, 25, "OPENGL %i.%i", major, minor);
	if (ImGui::Button(nameChar, ImVec2(125, 20)))
		ShellExecuteA(NULL, "Open", "https://www.opengl.org", NULL, NULL, SW_SHOWNORMAL);

	//MathGeoLib BUTTON Link
	sprintf_s(nameChar, 25, "MathGeoLib 1.5");
	if (ImGui::Button(nameChar, ImVec2(125, 20)))
		ShellExecuteA(NULL, "Open", "https://github.com/juj/MathGeoLib/releases/tag/v1.5", NULL, NULL, SW_SHOWNORMAL);

	//PCG BUTTON Link
	sprintf_s(nameChar, 25, "PCG 0.94");
	if (ImGui::Button(nameChar, ImVec2(125, 20)))
		ShellExecuteA(NULL, "Open", "http://www.pcg-random.org/download.html", NULL, NULL, SW_SHOWNORMAL);

	//Parson Link
	sprintf_s(nameChar, 25, "Parson");
	if (ImGui::Button(nameChar, ImVec2(125, 20)))
		ShellExecuteA(NULL, "Open", "https://github.com/kgabis/parson", NULL, NULL, SW_SHOWNORMAL);

	//Assimp Link
	sprintf_s(nameChar, 25, "Assimp");
	if (ImGui::Button(nameChar, ImVec2(125, 20)))
		ShellExecuteA(NULL, "Open", "http://www.assimp.org", NULL, NULL, SW_SHOWNORMAL);

	///---------------------------------
	ImGui::Separator();
	ImGui::TextWrapped("MIT License Copyright(c) 2018 Oscar Hernandez and Aleix Gabarro	Permission is hereby granted, free of charge, to any person obtaining a copy of this software"
		"and associated documentation files(the Software), to deal in the Software without restriction, including without limitation the rights to use, copy, modify,"
		"merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:");
	ImGui::TextWrapped("The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.");
	ImGui::TextWrapped("THE SOFTWARE IS PROVIDED AS IS, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A"
		"PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,"
		"WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.");

	ImGui::End();
}

void ModuleImGui::CreateConfigWindow()
{
	ImGui::Begin("Configuration", &configurationWindow);


	if (ImGui::CollapsingHeader("Application"))
	{
		CreateAppHeader();
	}

	if (ImGui::CollapsingHeader("Window"))
	{
		CreateWindowHeader();
	}

	if (ImGui::CollapsingHeader("Hardware"))
	{
		CreateHardwareHeader();
	}

	if (ImGui::CollapsingHeader("Render"))
	{
		CreateRenderHeader();
	}
	ImGui::End();
}

void ModuleImGui::CreateConsole()
{
	ImGui::SetWindowSize({ 400,200 }, ImGuiWindowFlags_NoResize);
	ImGui::Begin("Console", &consoleWindow);
	if (ImGui::Button("Clear", ImVec2(400, 20)))
		textBuff.clear();
	ImGui::Separator();

	ImGui::TextWrapped(textBuff.begin());
	if (canScroll)
		ImGui::SetScrollY(1.0f);
	canScroll = false;

	ImGui::End();
}

update_status ModuleImGui::CreateMainMenuBar()
{
	update_status ret = UPDATE_CONTINUE;
	if (ImGui::BeginMainMenuBar())
	{
		CreateMenu();

		CheckShortCuts();

		if (CreateOptions())
			ret = UPDATE_STOP;
	}
	ImGui::EndMainMenuBar();

	return ret;
}
//Create Windows----------------------------------------------------------


//Create Menu-------------------------------------------------------------
void ModuleImGui::CreateMenu()
{
	if (ImGui::BeginMenu("Menu"))
	{
		if (ImGui::MenuItem("Example window", "Ctrl+E", exampleWindow))
			exampleWindow = !exampleWindow;

		else if (ImGui::MenuItem("Demo window", "Ctrl+D", showDemoWindow))
			showDemoWindow = !showDemoWindow;

		else if (ImGui::MenuItem("Math window", "Ctrl+M", mathGeoLibWindow))
			mathGeoLibWindow = !mathGeoLibWindow;

		else if (ImGui::MenuItem("Random number window", "Ctrl+R", randomNumberWindow))
			randomNumberWindow = !randomNumberWindow;

		else if (ImGui::MenuItem("About", "Ctrl+A", aboutWindow))
			aboutWindow = !aboutWindow;

		if (ImGui::MenuItem("Configuration", "Ctrl+C", configurationWindow))
			configurationWindow = !configurationWindow;

		if (ImGui::MenuItem("Console", "Ctrl+GRAVE"))
			consoleWindow = !consoleWindow;

		ImGui::EndMenu();
	}
}

bool ModuleImGui::CreateOptions()
{
	if (ImGui::BeginMenu("Options"))
	{

		if (ImGui::MenuItem("Save Game", "Ctrl+S"))
			App->canSave = true;
		
		else if (ImGui::MenuItem("Load Game", "Ctrl+L"))
			App->canLoad = true;

		else if (ImGui::MenuItem("Exit", "ESC"))
			exitOption = true;

		ImGui::EndMenu();
	}
	return exitOption;
}

void ModuleImGui::CheckShortCuts()
{
	if (App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RCTRL) == KEY_REPEAT)
	{
		if (App->input->GetKey(SDL_SCANCODE_A) == KEY_DOWN)
			aboutWindow = !aboutWindow;

		else if (App->input->GetKey(SDL_SCANCODE_C) == KEY_DOWN)
			configurationWindow = !configurationWindow;

		else if (App->input->GetKey(SDL_SCANCODE_D) == KEY_DOWN)
			showDemoWindow = !showDemoWindow;

		else if (App->input->GetKey(SDL_SCANCODE_E) == KEY_DOWN)
			exampleWindow = !exampleWindow;
		
		else if (App->input->GetKey(SDL_SCANCODE_L) == KEY_DOWN)
			App->canLoad = true;

		else if (App->input->GetKey(SDL_SCANCODE_M) == KEY_DOWN)
			mathGeoLibWindow = !mathGeoLibWindow;

		else if (App->input->GetKey(SDL_SCANCODE_S) == KEY_DOWN)
			App->canSave = true;

		else if (App->input->GetKey(SDL_SCANCODE_R) == KEY_DOWN)
			randomNumberWindow = !randomNumberWindow;

		else if (App->input->GetKey(SDL_SCANCODE_GRAVE) == KEY_DOWN)
			consoleWindow = !consoleWindow;
	}
}
//Create Menu-------------------------------------------------------------


//Create Headers----------------------------------------------------------
void ModuleImGui::CreateAppHeader()
{

	static char appName[64];
	sprintf_s(appName,64,App->aplicationName.data());
	if(ImGui::InputText("Aplication Name", appName, 64, ImGuiInputTextFlags_EnterReturnsTrue))
		App->ChangeAppName(appName);

	ImGui::SliderInt("Max Fps", &App->capFrames, 30, 144);

	ImGui::Checkbox("Cap Fps", &App->toCap);

	char graphTitle[25];

	sprintf_s(graphTitle, 25, "Framerate %.1f", App->fpsLog[App->fpsLog.size() - 1]);
	ImGui::PlotHistogram("##Framerate", &App->fpsLog[0], App->fpsLog.size(), 0, graphTitle, 0.0f, 150.0f, ImVec2(310, 100));

	sprintf_s(graphTitle, 25, "Milliseconds %.1f", App->msLog[App->msLog.size() - 1]);
	ImGui::PlotHistogram("##Milliseconds", &App->msLog[0], App->msLog.size(), 0, graphTitle, 0.0f, 40.0f, ImVec2(310, 100));

	sprintf_s(graphTitle, 25, "RAM Usage %.1f", ramLog[ramLog.size() - 1]);
	ImGui::PlotHistogram("##RAM", &ramLog[0], ramLog.size(), 0, graphTitle, 0.0f, 125.0f, ImVec2(310, 100));
}

void ModuleImGui::CreateWindowHeader()
{
	if (ImGui::Checkbox("Fullscreen", &App->window->fullscreen))
	{
		App->window->fulldesktop = false;
		App->window->SetFullscreen();
	}
	ImGui::SameLine();
	if (ImGui::Checkbox("Borderless", &App->window->borderless))
	{
		App->window->SetBorderless();
	}
	if (ImGui::Checkbox("Full Desktop", &App->window->fulldesktop))
	{
		App->window->fullscreen = false;
		App->window->SetFullscreen(true);
	}
	if (ImGui::SliderFloat("Brightness", &brightnessPos, 0.0f, 1.0f))
	{
		SDL_SetWindowBrightness(App->window->window, brightnessPos);
	}
	if (ImGui::SliderInt("Width", &widthPos, 640, 1920))
	{
		SDL_SetWindowSize(App->window->window, widthPos, heightPos);
	}
	if (ImGui::SliderInt("Height", &heightPos, 480, 1080))
	{
		SDL_SetWindowSize(App->window->window, widthPos, heightPos);
	}
}

void ModuleImGui::CreateHardwareHeader()
{
	ImVec4 color(1.0f, 1.0f, 0.1f, 1.0f);

	//CPU--------------------------------------------------------
	CreateCPUInfo(color);

	ImGui::Separator();

	//GPU--------------------------------------------------------
	CreateGPUInfo(color);
}

void ModuleImGui::CreateRenderHeader()
{
	bool fill = false;
	fill = ImGui::Checkbox("Fill primitives", &App->renderer3D->isFill);
	ImGui::Checkbox("Wireframe", &App->renderer3D->isWire);

	if (!App->renderer3D->isFill && !App->renderer3D->isWire)
	{
		if (fill)
			App->renderer3D->isWire = true;
		else
			App->renderer3D->isFill = true;
	}
}
//Create Headers----------------------------------------------------------


void ModuleImGui::CreateCPUInfo(ImVec4 color)
{
	//CPU--------------------------------------------------------
	ImGui::Text("CPUs: "); ImGui::SameLine();
	ImGui::TextColored(color, "%i (Cache: %ikb)", SDL_GetCPUCount(), SDL_GetCPUCacheLineSize());

	//RAM--------------------------------------------------------
	ImGui::Text("System RAM: "); ImGui::SameLine();
	ImGui::TextColored(color, "%.2fGB", (float)SDL_GetSystemRAM() / 1024);

	MEMORYSTATUSEX memInfo;
	memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memInfo);
	DWORDLONG physMemUsed = memInfo.ullTotalPhys - memInfo.ullAvailPhys;
	float totalRamInUse = physMemUsed / 1024;
	totalRamInUse /= 1024;
	totalRamInUse /= 1024;

	ImGui::Text("Total RAM in use:"); ImGui::SameLine();
	ImGui::TextColored(color, "%.2f GB", totalRamInUse);



	ImGui::Text("RAM used by Factory Engin3D:"); ImGui::SameLine();
	ImGui::TextColored(color, "%.2f MB", ramLog.back());


	//CAP--------------------------------------------------------
	ImGui::Text("Caps: "); ImGui::SameLine();
	string caps("");
	CheckCaps(&caps);

	ImGui::TextColored(color, caps.data());
}

void ModuleImGui::CreateGPUInfo(ImVec4 color)
{
	//GPU--------------------------------------------------------
	const GLubyte* gpuInfo = glGetString(GL_VENDOR);
	const GLubyte* renderer = glGetString(GL_RENDERER);

	ImGui::Text("GPU info:"); ImGui::SameLine();
	ImGui::TextColored(color, (char*)gpuInfo); ImGui::SameLine();
	ImGui::TextColored(color, (char*)renderer);


	//VRAM-------------------------------------------------------
	GLint nTotalMemoryInKB = 0;
	glGetIntegerv(GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX,
		&nTotalMemoryInKB);

	GLint nCurAvailMemoryInKB = 0;
	glGetIntegerv(GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX,
		&nCurAvailMemoryInKB);
 
	ImGui::Text("VRAM total: "); ImGui::SameLine();
	ImGui::TextColored(color, "%.2f MB", (float)nTotalMemoryInKB / 1024);

	ImGui::Text("VRAM available: "); ImGui::SameLine();
	ImGui::TextColored(color, "%.2f MB", (float)nCurAvailMemoryInKB / 1024);

	ImGui::Text("VRAM in use: "); ImGui::SameLine();
	ImGui::TextColored(color, "%.2f MB", ((float)nTotalMemoryInKB / 1024) - (float)nCurAvailMemoryInKB / 1024);
}

void ModuleImGui::CheckCaps(string* caps)
{
	if (SDL_Has3DNow())
		*caps += "3Dnow, ";
	if (SDL_HasAVX())
		*caps += "AVX, ";
	if (SDL_HasAVX2())
		*caps += "AVX2, ";
	if (SDL_HasAltiVec())
		*caps += "AltiVec, ";
	if (SDL_HasMMX())
		*caps += "MMX, ";
	if (SDL_HasRDTSC())
		*caps += "RDTSC , ";
	if (SDL_HasSSE())
		*caps += "SSE, ";
	if (SDL_HasSSE2())
		*caps += "SSE2, ";
	if (SDL_HasSSE3())
		*caps += "SSE3, ";
	if (SDL_HasSSE41())
		*caps += "SSE4.1, ";
	if (SDL_HasSSE42())
		*caps += "SSE4.2, ";

	*caps = caps->substr(0, caps->size() - 2);
}

void ModuleImGui::LogConsole(const char * consoleText)
{
	textBuff.appendf(consoleText);
	canScroll = true;
}