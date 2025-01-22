#pragma once
#include "../stdafx.h"
#include "../objects/World.hpp"
#include "../objects/Environment.hpp"
#include "../interface/Camera.hpp"
#include "../interface/Window.hpp"
#include "../interface/TextRenderer.hpp"
#include "../objects/CueBallMap.hpp"
#include "Logger.hpp"

class App
{
public:
	App();
	~App();

	App(const App&) = delete;
	App(App&&) = delete;
	App& operator= (const App&) = delete;
	App& operator= (App&&) = delete;

	void Run();

private:
	void OnUpdate();
	void OnResize() const;
	void Load();
	void HandleState();

	std::unique_ptr<Window> window_ = nullptr;
	std::unique_ptr<Camera> camera_ = nullptr;
	std::unique_ptr<World> world_ = nullptr;
	std::unique_ptr<Environment> environment_ = nullptr;
	std::unique_ptr<TextRenderer> text_renderer_ = nullptr;
	std::unique_ptr<Menu> menu_ = nullptr;
	std::array<std::unique_ptr<Light>, 10> lights_;
	std::array<bool, 10> lightOn_{ true, true, true, true, true, true, true, true, true, true };
	std::shared_ptr<Shader> main_shader_ = nullptr;
	std::shared_ptr<Shader> background_shader_ = nullptr;
	std::shared_ptr<Shader> gui_shader_ = nullptr; 

	// Add CueBallMap member
	std::shared_ptr<CueBallMap> cue_ball_map_ = nullptr;


	bool in_menu_{true};
	double delta_time_ = 0.0f;	
	double last_frame_ = 0.0f;
};
