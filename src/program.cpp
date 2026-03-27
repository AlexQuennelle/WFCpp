#include "program.h"
#include "utils.h"

#include <imgui.h>
#include <iostream>
#include <raylib.h>
#include <rlImGui.h>
#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif // !__EMSCRIPTEN__

Program::Program()
{
	SetTextColor(INFO);
	std::cout << "Initializing program\n";
	ClearStyles();

	InitWindow(800, 800, NAME);
	SetTargetFPS(60);
	rlImGuiSetup(true);
	this->imguiIO = &ImGui::GetIO();

	imguiIO->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
}

void Program::Run()
{
#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop_arg(WebLoop, this, 0, 1);
#else
	while (!WindowShouldClose())
	{
		this->Update();
	}
#endif
}

void Program::Update()
{
	BeginDrawing();
	rlImGuiBegin();

	// The internal draw loops for Raylib and ImGui starts before the user
	// defined draw loop to allow for debug visualizations.

	// Draw loop start
	this->Draw();

	rlImGuiEnd();
	EndDrawing();
}

void Program::Draw() const
{
	ClearBackground({.r = 100, .g = 149, .b = 237, .a = 255});

	// ImGui demo
	bool open = true;
	ImGuiWindowFlags flags{ImGuiWindowFlags_NoSavedSettings |
						   ImGuiWindowFlags_AlwaysAutoResize};
	if (ImGui::Begin("ImGui Window", &open, flags))
	{
		ImGui::Text("Text.");
	}
	ImGui::End();
}

Program::~Program()
{
	rlImGuiShutdown();
	CloseWindow();
}
