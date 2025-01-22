#include "../stdafx.h"
#include "App.hpp"
#include "../objects/CueBallMap.hpp" 

App::App() :
	window_(std::make_unique<Window>()),
	text_renderer_(std::make_unique<TextRenderer>()),
	menu_(std::make_unique<Menu>(window_->GetWidth(), window_->GetHeight())),
	main_shader_(std::make_shared<Shader>(Config::vertex_path, Config::fragment_path)),
	background_shader_(std::make_shared<Shader>(Config::background_vertex_path, Config::background_fragment_path)),
	gui_shader_(std::make_shared<Shader>(Config::CueMap_vertex_path, Config::CueMap_fragment_path)),
	camera_(std::make_unique<Camera>()),
	cue_ball_map_(std::make_shared<CueBallMap>(*camera_, window_->GetGLFWWindow()))
{
	Logger::Init("log.txt");
	text_renderer_->Init();
	camera_->Init();
}

App::~App() {
	Logger::Close();
}

void App::Run()
{
	while (!window_->ShouldClose())
	{
		glfwPollEvents();

		if (window_->Resized())
			OnResize();

		OnUpdate();

		glfwSwapBuffers(window_->GetGLFWWindow());
	}
}

void App::OnUpdate()
{
	const double current_frame = glfwGetTime();
	delta_time_ = current_frame - last_frame_;
	last_frame_ = current_frame;

	HandleState();

	glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (world_)
	{
		camera_->UpdateViewMatrix(static_cast<float>(delta_time_));
		camera_->UpdateMain(main_shader_, *world_);

		environment_->Prepare();
		world_->Update(static_cast<float>(delta_time_), !in_menu_);
		world_->Draw(main_shader_);

		camera_->UpdateBackground(background_shader_);
		environment_->Draw(background_shader_);

		// Update CueBallMap visibility based on camera view and ball movement
		bool isTopDownView = camera_->IsTopDownView();
		bool ballsAreMoving = world_->AreBallsInMotion();
		bool shouldBeVisible = isTopDownView && !ballsAreMoving;
		cue_ball_map_->SetVisible(shouldBeVisible);
		if (cue_ball_map_->IsVisible()) {
			cue_ball_map_->Draw();
			cue_ball_map_->HandleMouseInput(window_->GetGLFWWindow());
		}
		
	}

	if (in_menu_)
		menu_->Draw(world_ == nullptr);

	menu_->AddText(0.0f, 0.95f, "FPS: " + std::to_string(static_cast<int>(1.0f / delta_time_)), 0.75f);

	// Show current player
	if (world_) {
		const auto& players = world_->GetPlayers();
		const auto& current_player = players[world_->GetCurrentPlayerIndex()];
		menu_->AddText(0.0f, 0.9f, "Current Player: " + current_player.GetName(), 0.75f);

		// Show the shot clock
		float clockSec = world_->GetShotClock();
		menu_->AddText(0.0f, 0.85f, "Shot Clock: " + std::to_string((int)clockSec), 0.75f);

		//short message in the center 
		const std::string msg = world_->GetMessage();
		if (!msg.empty())
		{
			menu_->AddText(0.35f, 0.95f, msg, 0.75f);
		}
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	text_renderer_->Render(menu_->GetTexts());
	glDisable(GL_BLEND);

	// Handle light toggling with numpad keys 0-9 using edge detection
	static bool key_was_pressed[10] = { false };
	GLFWwindow* window = window_->GetGLFWWindow();

	for (int key = GLFW_KEY_0; key <= GLFW_KEY_9; ++key) {
		int lightIndex = key - GLFW_KEY_0;
		bool isPressed = (glfwGetKey(window, key) == GLFW_PRESS);

		if (isPressed && !key_was_pressed[lightIndex]) {
			// Key has just been pressed this frame
			world_->ToggleLight(lightIndex);
			key_was_pressed[lightIndex] = true;
		}
		else if (!isPressed && key_was_pressed[lightIndex]) {
			// Key has been released
			key_was_pressed[lightIndex] = false;
		}
	}
}


void App::OnResize() const
{
	const int new_width = window_->GetWidth(), new_height = window_->GetHeight();

	glViewport(0, 0, new_width, new_height);

	if (camera_)
		camera_->UpdateProjectionMatrix(new_width, new_height);

	menu_->Update(new_width, new_height);

	text_renderer_->UpdateProjectionMatrix(new_width, new_height);
	text_renderer_->Update();

	// Update CueBallMap window size
	if (cue_ball_map_)
		cue_ball_map_->UpdateWindowSize();

	window_->ResetResizedFlag();
}

void App::Load()
{
	world_ = std::make_unique<World>(cue_ball_map_, *camera_);
	environment_ = std::make_unique<Environment>();

	camera_->Init();
	world_->Init();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glEnable(GL_MULTISAMPLE);
	glViewport(0, 0, window_->GetWidth(), window_->GetHeight());

	main_shader_->Bind();
	main_shader_->SetInt(1, "irradianceMap");
	main_shader_->SetInt(2, "prefilterMap");
	main_shader_->SetInt(3, "brdfLUT");
	main_shader_->SetInt(4, "material.diffuseMap");
	main_shader_->SetInt(5, "material.roughnessMap");
	main_shader_->SetInt(6, "material.normalMap");
	main_shader_->SetInt(7, "material.aoMap");
	main_shader_->SetInt(8, "material.metallicMap");
	main_shader_->Unbind();
}

void App::HandleState()
{
	GLFWwindow* window = window_->GetGLFWWindow();
	const int selected = menu_->GetSelected();

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		in_menu_ = true;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	if (selected == 0)
	{
		if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
		{
			in_menu_ = false;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

			if (!world_) {
				Load();
				world_->ResetPlayerIndex(); // Ensure player index is reset when loading the game
				// Reset the timing reference here to avoid large delta on first frame
				last_frame_ = glfwGetTime();
			}
		}
	}
	else if (selected == 1)
	{
		menu_->DrawHelp();
	}
	else if (selected == 2)
	{
		if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && world_) {
			world_->Reset();
			world_->ResetGame();
		}
	}
	else if (selected == 3)
	{
		menu_->AddText(0.6f, 0.4f, "Strike force: " + std::to_string(Config::power_coeff), 0.75f);

		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
			Config::power_coeff -= 0.02f;
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
			Config::power_coeff += 0.02f;
	}
	else if (selected == 4)
	{
		menu_->AddText(0.6f, 0.3f, "Ball friction: " + std::to_string(1.0f / Config::velocity_multiplier), 0.75f);

		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS && Config::velocity_multiplier < 1.0f)
			Config::velocity_multiplier += 0.0001f;
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
			Config::velocity_multiplier -= 0.0001f;
	}
	else if (selected == 5)
	{
		if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
			window_->SetCloseFlag();
	}

	// Toggle camera view on mouse click
	static int last_mouse_button_state = GLFW_RELEASE;
	int mouse_button_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);

	if (mouse_button_state == GLFW_PRESS && last_mouse_button_state == GLFW_RELEASE) {
		bool isCueBallMapVisible = cue_ball_map_->IsVisible();
		bool isCueBallMapWithinBounds = cue_ball_map_->IsWithinBounds();
		if (camera_ && (!isCueBallMapVisible || !isCueBallMapWithinBounds)) {
			bool top_down = camera_->IsTopDownView();
			camera_->SetTopDownView(!top_down);
			if (!top_down) {
				// Enter top-down view
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}
			else {
				// Exit top-down view
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			}
		}
	}
	last_mouse_button_state = mouse_button_state;
}
