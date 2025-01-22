#pragma once
#include "../stdafx.h"
#include "Object.hpp"
#include "Table.hpp"
#include "Cue.hpp"
#include "CueBallMap.hpp"
#include "Ball.hpp"
#include "Ceiling.hpp"
#include "Light.hpp"
#include "Player.hpp"

// Forward declarations
class CueBallMap;
class Camera;

class World
{
public:
	World(std::shared_ptr<CueBallMap> cue_ball_map, Camera& camera);
	void Update(float dt, bool in_game);
	void Draw(const std::shared_ptr<Shader>& shader) const;
	void Init() const;
	void Reset() const;

	//Collision Handling
	void HandleBallsCollision(int number) ;
	void HandleHolesFall(int number) const;
	void HandleBoundsCollision(const int number) const;

	[[nodiscard]] bool AreBallsInMotion() const;

	// Add method to toggle lights
	void ToggleLight(int index); 

	// Add getter method for lights
	const std::vector<std::shared_ptr<Light>>& GetLights() const { return lights_; }

	// Helpers
	const std::vector<std::shared_ptr<Ball>>& GetBalls() const { return balls_; }
	const std::vector<Player>& GetPlayers() const { return players_; }
	int GetCurrentPlayerIndex() const { return current_player_index_; }

	// Add method to reset player index
	void ResetPlayerIndex(); 

	// Add a method to reset score, player index, and clock
	void ResetGame();

	// For external HUD
	float GetShotClock()       const { return shot_clock_; }
	bool  IsGameOver()         const { return is_game_over_; }
	std::string GetMessage()   const { return current_message_; }


private:
	void InitializeLights(); // Add method to initialize lights
	void SwitchTurn();
	void CheckGameRules();
	void AssignBallType(int ballNumber);

	// “Ball in hand” placement
	void PlaceCueBallWithMouse();

	// Extra bounding logic for holes & cushions
	glm::vec3 ClampCueBallPosition(const glm::vec3& desiredPos) const;

	// Helper to check if all solids/stripes are cleared
	bool AreAllGroupBallsPocketed(int groupType) const;

	// Utility to display a message on screen for a few seconds
	void SetMessage(const std::string& msg, float duration);

	// A reference to the unique Camera for ray picking
	Camera& camera_;

	std::shared_ptr<Table> table_ = nullptr;
	std::shared_ptr<Cue> cue_ = nullptr;
	std::vector<std::shared_ptr<Ball>> balls_{};
	std::vector<std::shared_ptr<Light>> lights_; // Lights for the table
	std::shared_ptr<Ceiling> ceiling_ = nullptr;

	std::vector<Player> players_;
	int current_player_index_ = 0;

	// 8-ball state
	bool is_game_over_ = false;
	bool is_first_shot_ = true;
	bool is_after_break_ = true;
	bool check_game_rules_ = false;

	// For collision logic
	bool cueBallHitOtherBall_ = false;
	int firstBallContactIndex_ = -1; // -1 => cue ball contacted no ball


	// -1 => not assigned; 0 => solids; 1 => stripes
	int player1_ball_type_ = -1;
	int player2_ball_type_ = -1;

	// If a foul -> ball in hand
	bool ball_in_hand_ = false;

	// Shot clock (e.g., 30 seconds)
	float shot_clock_ = 30.0f;
	static constexpr float SHOT_CLOCK_MAX_ = 30.0f;

	// On-screen message
	std::string current_message_;
	float message_timer_ = 0.0f;

};
