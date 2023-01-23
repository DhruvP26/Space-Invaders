#pragma once

#include <vector>

#include "Input.h"
#include "D3D.h"
#include "SpriteBatch.h"
#include "Sprite.h"

/*
Animated missile bullet 
Player can only fire one and has to wait for it to leave the 
screen before firing again.
*/
class Bullet
{
public:
	Bullet(DirectX::SimpleMath::Vector2 pos, int direction);
	Sprite bullet;

	static void Init(MyD3D& d3d);
	static inline ID3D11ShaderResourceView* p = nullptr;
	static inline MyD3D* d3d;

	void Render(DirectX::SpriteBatch& batch);
	void Update(float dTime);
	bool OutOfBounds();
private:
	int direction;
};

class Enemy
{
public:
	Enemy(MyD3D& d3d) 
		:sprite(d3d)
	{}
	void Init(MyD3D& d3d, DirectX::SimpleMath::Vector2 pos);
	void Render(DirectX::SpriteBatch& batch);
	void Update(float dTime);
	void MoveDown();
	bool CheckSwitchDirection(const RECTF& playArea);
	Sprite GetSprite() { return sprite; }
private:
	Sprite sprite;
	inline static int xSpeed = 20;
};

//horizontal scrolling with player controlled ship
class PlayMode
{
public:
	PlayMode(MyD3D& d3d);
	void Update(float dTime);
	void UpdateEnemies(float dTime);
	void Render(float dTime, DirectX::SpriteBatch& batch);
	bool IsGameOver();

private:
	const float SCROLL_SPEED = 10.f;
	static const int BGND_LAYERS = 8;
	const float SPEED = 250;
	const float MOUSE_SPEED = 5000;
	const float PAD_SPEED = 500;

	MyD3D& mD3D;
	std::vector<Sprite> mBgnd; //parallax layers
	Sprite mPlayer;		//jet
	RECTF mPlayArea;	//don't go outside this	
	std::vector<Bullet> mPlayerBullets; 
	std::vector<Bullet> mEnemyBullets;
	std::vector<Enemy> mEnemies;

	//once we start thrusting we have to keep doing it for 
	//at least a fraction of a second or it looks whack
	float mThrusting = 0; 


	float mEnemyBulletTimer = 2;
	int mLives = 3;
	float mRespawnTimer = 0;

	bool mFireDown = false;  // is the fire button currently being held down 
	
	ID3D11ShaderResourceView* mLivesTexture;

	//setup once
	void InitBgnd();
	void InitPlayer();

	//make it move, reset it once it leaves the screen, only one at once
	void UpdateBullets(float dTime);
	//make it scroll parallax
	void UpdateBgnd(float dTime);
	//move the ship by keyboard, gamepad or mouse
	void UpdateInput(float dTime);
	//check for collision between bullets, and the player 
	void UpdateCollisions();
};


/*
Basic wrapper for a game
*/
class Game
{
public:
	enum class State { TITLE, PLAY };
	static MouseAndKeys sMKIn;
	static Gamepads sGamepads;
	State state = State::TITLE;
	Game(MyD3D& d3d);


	void Release();
	void Update(float dTime);
	void Render(float dTime);
private:
	MyD3D& mD3D;
	DirectX::SpriteBatch *mpSB = nullptr;
	//not much of a game, but this is it
	PlayMode* mPMode;
	Sprite mTitleSprite;
};


