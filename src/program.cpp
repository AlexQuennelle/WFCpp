#include "program.h"
#include "utils.h"

#include <imgui.h>
#include <iostream>
#include <raylib.h>
#include <rlImGui.h>
#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif // !__EMSCRIPTEN__

Program::Program() : generator(16)
{
	SetTextColor(INFO);
	std::cout << "Initializing program\n";
	ClearStyles();

	InitWindow(800, 800, NAME);
	SetTargetFPS(60);
	rlImGuiSetup(true);
	this->imguiIO = &ImGui::GetIO(); // NOLINT

	this->generator.Step();
	// this->generator.Step();
	this->generator.ToTex();

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

	this->Draw();

	rlImGuiEnd();
	EndDrawing();
}

void Program::Draw() const
{
	ClearBackground({.r = 100, .g = 149, .b = 237, .a = 255});

	DrawTextureEx(generator.GetTexture(), {.x = 0.0f, .y = 0.0f}, 0.0f,
				  800.0f / 16.0f, WHITE);

	// ImGui demo
	bool open = true;
	ImGuiWindowFlags flags{ImGuiWindowFlags_NoSavedSettings
						   | ImGuiWindowFlags_AlwaysAutoResize};
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
