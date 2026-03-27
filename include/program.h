#pragma once

#include "imgui.h"

/**
 * Class representing an instance of the program and encapsulating global state.
 */
class Program
{
	public:
	Program();
	Program(const Program&) = delete;
	Program(Program&&) = delete;
	~Program();

	void Run();

	auto operator=(const Program&) -> Program& = delete;
	auto operator=(Program&&) -> Program& = delete;

#ifdef __EMSCRIPTEN__
	friend void WebLoop(void* arg);
#endif // __EMSCRIPTEN__

	private:
	void Update();
	void Draw() const;

	ImGuiIO* imguiIO;
};

#ifdef __EMSCRIPTEN__
void WebLoop(void* arg);
#endif // __EMSCRIPTEN__
