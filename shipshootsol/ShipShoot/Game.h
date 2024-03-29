#pragma once

#include <vector>
#include <memory>
#include "Input.h"
#include "D3D.h"
#include "SpriteBatch.h"
#include "Sprite.h"

#include "SpriteFont.h"

class AudioMgrFMOD;
class IAudioMgr;

/*
Animated missile bullet 
Player can only fire one and has to wait for it to leave the 
screen before firing again.
*/
class Bullet
{
public:
	Bullet(DirectX::SimpleMath::Vector2 pos, int direction, bool useBossBulletTexture);
	Sprite bullet;

	static void Init(MyD3D& d3d);
	static inline ID3D11ShaderResourceView* texture = nullptr;
	static inline ID3D11ShaderResourceView* texture2 = nullptr;
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
	Enemy(MyD3D& d3d, ID3D11ShaderResourceView* texture, DirectX::SimpleMath::Vector2 pos);
	void Render(DirectX::SpriteBatch& batch);
	virtual void Update(float dTime);
	virtual void MoveDown();
	virtual bool CheckSwitchDirection(const RECTF& playArea);
	virtual bool ShouldDestroy() { return false; }
	virtual int GetScore() { return 10; }
	virtual bool FiresBossBullet() { return false; }
	Sprite GetSprite() { return sprite; }
	static void ResetXSpeed() { sharedXSpeed = 20; }
	static void IncreaseXSpeed() { sharedXSpeed += 10; }
protected:				  // variables can be accessed by derived clases 
	Sprite sprite;
private:
	inline static int sharedXSpeed = 20;
	inline static int sharedXDirection = 1;
};

class BossEnemy : public Enemy
{
public:
	BossEnemy(MyD3D& d3d, ID3D11ShaderResourceView* texture, DirectX::SimpleMath::Vector2 pos):
		Enemy(d3d, texture, pos)
	{}
	virtual void Update(float dTime);
	virtual void MoveDown();
	virtual bool CheckSwitchDirection(const RECTF& playArea);
	virtual bool ShouldDestroy();
	virtual int GetScore() { return 100; }
	virtual bool FiresBossBullet() { return true; }
private:
	int xSpeed = 60;
};

class Shield
{
public:
	Shield(MyD3D& d3d, DirectX::SimpleMath::Vector2 pos);
	void Render(DirectX::SpriteBatch& batch);
	bool CheckCollision(Bullet& bullet);
	virtual bool ShouldDestroy() { return false; }
	//Sprite GetSprite() { return sprite; }
private:				
	std::vector<Sprite> pieceSprites;
};

//horizontal scrolling with player controlled ship
class PlayMode
{
public:
	PlayMode(MyD3D& d3d, std::shared_ptr<DirectX::DX11::SpriteFont> spriteFont, IAudioMgr* audio);
	~PlayMode();
	void Update(float dTime);
	void UpdateEnemies(float dTime);
	void Render(float dTime, DirectX::SpriteBatch& batch);
	bool IsGameOver();
	int GetScore() { return mScore; }

private:
	const float SCROLL_SPEED = 10.f;
	static const int BGND_LAYERS = 2;
	const float SPEED = 250;
	const float MOUSE_SPEED = 5000;
	const float PAD_SPEED = 500;

	MyD3D& mD3D;
	std::shared_ptr<DirectX::DX11::SpriteFont> mSpriteFont;
	IAudioMgr* mAudio;
	std::vector<Sprite> mBgnd; //parallax layers
	Sprite mPlayer;		//jet
	RECTF mPlayArea;	//don't go outside this	
	std::vector<Bullet> mPlayerBullets; 
	std::vector<Bullet> mEnemyBullets;
	std::vector<Enemy*> mEnemies;
	std::vector<Shield> mShields;

	//once we start thrusting we have to keep doing it for 
	//at least a fraction of a second or it looks whack
	float mThrusting = 0; 


	float mEnemyBulletTimer = 2;
	int mLives = 3;
	float mRespawnTimer = 0;
	int mScore = 0;
	int mLevel = 1;

	float mBossTimer = 5;

	bool mFireDown = false;  // is the fire button currently being held down 
	
	ID3D11ShaderResourceView* mLivesTexture;
	ID3D11ShaderResourceView* mEnemyTexture;
	ID3D11ShaderResourceView* mBossTexture;

	//setup once
	void InitBgnd();
	void InitPlayer();
	void InitEnemies();
	void InitShields();

	void NewLevel();

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
	enum class State { TITLE, PLAY, GAMEOVER };
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
	Sprite mGameOverBackgroundSprite;
	std::shared_ptr<DirectX::DX11::SpriteFont> mSpriteFont;
	std::shared_ptr<AudioMgrFMOD> mAudio;
	std::vector<bool> mKeysPressed;
	std::string mPlayerName;
	std::vector<std::pair<std::string, int> > mHighscores;
};


