#include "Game.h"
#include "WindowUtils.h"
#include "CommonStates.h"
#include <random>
#include <memory>
#include <SpriteFont.h>
#include <sstream>
#include <Audio.h>


using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

MouseAndKeys Game::sMKIn;
Gamepads Game::sGamepads;

//random number engine
default_random_engine randEngine;

const RECTF missileSpin[]{
	{ 0,  0, 53, 48},
	{ 54, 0, 107, 48 },
	{ 108, 0, 161, 48 },
	{ 162, 0, 220, 48 },
};

const RECTF thrustAnim[]{
	{ 0,  0, 15, 16},
	{ 16, 0, 31, 16 },
	{ 32, 0, 47, 16 },
	{ 48, 0, 64, 16 },
};

Game::Game(MyD3D& d3d)
	: mPMode(nullptr), mD3D(d3d), mpSB(nullptr), mTitleSprite(mD3D),
	  mSpriteFont(std::make_shared<SpriteFont>(&d3d.GetDevice(), L"data\\fonts\\comic.spritefont"))
{
	sMKIn.Initialise(WinUtil::Get().GetMainWnd(), true, false);
	sGamepads.Initialise();
	mpSB = new SpriteBatch(&mD3D.GetDeviceCtx());
	auto* titleTexture = mD3D.GetCache().LoadTexture(&mD3D.GetDevice(), "title.dds");
	mTitleSprite.SetTex(*titleTexture);
	mTitleSprite.SetScale(Vector2(1, 1));
	mTitleSprite.mPos = Vector2(0, 0);
	mKeysPressed.resize(VK_Z + 1);

	// This is only needed in Windows desktop apps
	auto hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	//if (FAILED(hr))
		// error

	AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#ifdef _DEBUG
	eflags |= AudioEngine_Debug;
#endif
	mAudioEngine = std::make_shared<AudioEngine>(eflags);
}
  

//any memory or resources we made need releasing at the end
void Game::Release()
{
	delete mpSB;
	mpSB = nullptr;
}

//called over and over, use it to update game logic
void Game::Update(float dTime)
{
	sGamepads.Update();
	switch (state)
	{
	case State::TITLE:
		if (Game::sMKIn.IsPressed(VK_RETURN))
		{
			mPMode = new PlayMode(mD3D, mSpriteFont, mAudioEngine); 
			state = State::PLAY;
		}
		else
		{
			for (int key = VK_A; key <= VK_Z; ++key)
			{
				bool state = Game::sMKIn.IsPressed(key);
				if (state && !mKeysPressed[key])
				{
					mPlayerName += (char)key;
				}
				mKeysPressed[key] = state;
			}
		}
		break;
	case State::PLAY:
		mPMode->Update(dTime);
		if (mPMode->IsGameOver())
		{
			state = State::GAMEOVER;
			delete mPMode;
			mPMode = nullptr;
		}
		break;
	case State::GAMEOVER:
		if (Game::sMKIn.IsPressed(VK_SPACE))
		{
			state = State::TITLE;
		}
		break;
	}
}

//called over and over, use it to render things
void Game::Render(float dTime)
{
	mD3D.BeginRender(Colours::Black);


	CommonStates dxstate(&mD3D.GetDevice());
	mpSB->Begin(SpriteSortMode_Deferred, dxstate.NonPremultiplied(), &mD3D.GetWrapSampler());

	switch (state)
	{
	case State::TITLE:
		mTitleSprite.Draw(*mpSB);
		mSpriteFont->DrawString(mpSB, "ENTER NAME", XMFLOAT2(260, 350));
		mSpriteFont->DrawString(mpSB, mPlayerName.c_str(), XMFLOAT2(260, 400));
		break;
	case State::PLAY:
		mPMode->Render(dTime, *mpSB);
		break;
	case State::GAMEOVER:
		mTitleSprite.Draw(*mpSB);
		mSpriteFont->DrawString(mpSB, "PRESS SPACE TO RETURN TO TITLE SCREEN", XMFLOAT2(120, 400));
		break;
	}

	mpSB->End();


	mD3D.EndRender();
	sMKIn.PostProcess();
}

Bullet::Bullet(DirectX::SimpleMath::Vector2 pos, int direction)
	:bullet(*d3d),
	direction(direction)
{
	bullet.SetTex(*p);
	bullet.GetAnim().Init(0, 3, 15, true);
	bullet.GetAnim().Play(true);
	bullet.SetScale(Vector2(0.75f, 0.75f));
	bullet.origin = Vector2((missileSpin[0].right - missileSpin[0].left) / 2.f, (missileSpin[0].bottom - missileSpin[0].top) / 2.f);
	bullet.mPos = pos;
}

void Bullet::Init(MyD3D& d3d)
{
	vector<RECTF> frames2(missileSpin, missileSpin + sizeof(missileSpin) / sizeof(missileSpin[0]));
	p = d3d.GetCache().LoadTexture(&d3d.GetDevice(), "missile.dds", "missile", true, &frames2);
	Bullet::d3d = &d3d;
}

void Bullet::Render(SpriteBatch& batch)
{
	bullet.Draw(batch);
}

void Bullet::Update(float dTime)
{
	bullet.mPos.y += 300 * dTime * direction;
	bullet.GetAnim().Update(dTime);
	
}

bool Bullet::OutOfBounds()
{
	return bullet.mPos.y < 0;
}


void Enemy::Init(MyD3D& d3d, DirectX::SimpleMath::Vector2 pos)
{
	ID3D11ShaderResourceView* p = d3d.GetCache().LoadTexture(&d3d.GetDevice(), "shipYellow_manned.dds");

	sprite.SetTex(*p);
	sprite.SetScale(Vector2(0.5f, 0.5f));
	//sprite.origin = Vector2((missileSpin[0].right - missileSpin[0].left) / 2.f, (missileSpin[0].bottom - missileSpin[0].top) / 2.f);
	sprite.mPos = pos;
}

void Enemy::Render(SpriteBatch& batch)
{
	sprite.Draw(batch);
}

void Enemy::Update(float dTime)
{

	//bullet.mPos.y -= MISSILE_SPEED * dTime;
	sprite.mPos.x += xSpeed * dTime;
	

	sprite.GetAnim().Update(dTime);
	
}

void Enemy::MoveDown()
{
	sprite.mPos.y += 20;
}

bool Enemy::CheckSwitchDirection(const RECTF& playArea)
{
	if ((xSpeed > 0 && sprite.mPos.x > playArea.right) || (xSpeed < 0 && sprite.mPos.x < playArea.left))
	{
		xSpeed = -xSpeed;
		return true;
	}
	return false;
}


PlayMode::PlayMode(MyD3D & d3d, std::shared_ptr<SpriteFont> spriteFont, std::shared_ptr<AudioEngine> audioEngine)
	:mD3D(d3d), mPlayer(d3d), mSpriteFont(spriteFont), mAudioEngine(audioEngine)
{
	InitBgnd();
	InitPlayer();
	Bullet::Init(d3d);
	for (float y = 10; y < 180; y += 30)
	{
		for (float x = 100; x < 500; x += 70)
		{
			mEnemies.emplace_back(d3d);
			mEnemies.back().Init(d3d, Vector2(x, y));
		}
	}
	
	mLivesTexture = mD3D.GetCache().LoadTexture(&mD3D.GetDevice(), "ship.dds");
}

void PlayMode::UpdateBullets(float dTime)
{
	if (mRespawnTimer <= 0 && mPlayerBullets.size() < 3 && Game::sMKIn.IsPressed(VK_SPACE) && !mFireDown)
	{
		mPlayerBullets.emplace_back(Vector2(mPlayer.mPos.x + mPlayer.GetScreenSize().x / 2.f - 20, mPlayer.mPos.y), -1);
	}

	mFireDown = Game::sMKIn.IsPressed(VK_SPACE);

	for (int bulletI = mPlayerBullets.size() - 1; bulletI >= 0; --bulletI)
	{
		mPlayerBullets[bulletI].Update(dTime);
		if (mPlayerBullets[bulletI].OutOfBounds())
			mPlayerBullets.erase(begin(mPlayerBullets) + bulletI);
	}

	// enemy firing
	mEnemyBulletTimer -= dTime;
	if (mEnemyBulletTimer <= 0 && !mEnemies.empty())
	{
		uniform_int_distribution<int> range(0, mEnemies.size() - 1);
		int chosenI = range(randEngine);
		auto& enemySprite = mEnemies[chosenI].GetSprite();
		mEnemyBullets.emplace_back(Vector2(enemySprite.mPos.x + enemySprite.GetScreenSize().x / 8.f, enemySprite.mPos.y), 1);
		mEnemyBulletTimer = 60.f / mEnemies.size();
	}
	for (int bulletI = mEnemyBullets.size() - 1; bulletI >= 0; --bulletI)
	{
		mEnemyBullets[bulletI].Update(dTime);
		if (mEnemyBullets[bulletI].OutOfBounds())
			mEnemyBullets.erase(begin(mEnemyBullets) + bulletI);
	}
}



void PlayMode::UpdateBgnd(float dTime)
{
	//scroll the background layers
	int i = 0;
	for (auto& s : mBgnd)
		s.Scroll(dTime*(i++)*SCROLL_SPEED, 0);
}

void PlayMode::UpdateInput(float dTime)
{
	if (mRespawnTimer > 0)
		return;

	Vector2 mouse{ Game::sMKIn.GetMousePos(false) };
	bool keypressed = Game::sMKIn.IsPressed(VK_UP) || Game::sMKIn.IsPressed(VK_DOWN) ||
		Game::sMKIn.IsPressed(VK_RIGHT) || Game::sMKIn.IsPressed(VK_LEFT);
	bool sticked = false;
	if (Game::sGamepads.IsConnected(0) &&
		(Game::sGamepads.GetState(0).leftStickX != 0 || Game::sGamepads.GetState(0).leftStickX != 0))
		sticked = true;

	if (keypressed || (mouse.Length() >VERY_SMALL) || sticked)
	{
		//move the ship around
		Vector2 pos(0, 0);
		if (Game::sMKIn.IsPressed(VK_UP))
			pos.y -= SPEED * dTime;
		else if (Game::sMKIn.IsPressed(VK_DOWN))
			pos.y += SPEED * dTime;
		if (Game::sMKIn.IsPressed(VK_RIGHT))
			pos.x += SPEED * dTime;
		else if (Game::sMKIn.IsPressed(VK_LEFT))
			pos.x -= SPEED * dTime;

		pos += mouse * MOUSE_SPEED * dTime;

		if (sticked)
		{
			DBOUT("left stick x=" << Game::sGamepads.GetState(0).leftStickX << " y=" << Game::sGamepads.GetState(0).leftStickY);
			pos.x += Game::sGamepads.GetState(0).leftStickX * PAD_SPEED * dTime;
			pos.y -= Game::sGamepads.GetState(0).leftStickY * PAD_SPEED * dTime;
		}

		//keep it within the play area
		pos += mPlayer.mPos;
		if (pos.x < mPlayArea.left)
			pos.x = mPlayArea.left;
		else if (pos.x > mPlayArea.right)
			pos.x = mPlayArea.right;
		if (pos.y < mPlayArea.top)
			pos.y = mPlayArea.top;
		else if (pos.y > mPlayArea.bottom)
			pos.y = mPlayArea.bottom;

		mPlayer.mPos = pos;
		mThrusting = GetClock() + 0.2f;
	}
}

void PlayMode::UpdateCollisions()
{
	//going through the list backwards so that items can be deleted without skipping items
	for (int bulletI = mPlayerBullets.size() - 1; bulletI >= 0; --bulletI)
	{
		auto& bulletSprite = mPlayerBullets[bulletI].bullet;
		auto bulletSize = bulletSprite.GetScreenSize();
		float bulletWidth = bulletSize.x / 4;   // 4 frames of animation

		bool collided = false;

		for (int enemyI = mEnemies.size() - 1; enemyI >= 0; --enemyI)
		{
			auto& enemySprite = mEnemies[enemyI].GetSprite();
			auto enemySize = enemySprite.GetScreenSize();
			float enemyWidth = enemySize.x;   

			if (
				bulletSprite.mPos.x < enemySprite.mPos.x + enemyWidth &&
				bulletSprite.mPos.x + bulletWidth > enemySprite.mPos.x &&
				bulletSprite.mPos.y < enemySprite.mPos.y + enemySize.y &&
				bulletSprite.mPos.y + bulletSize.y > enemySprite.mPos.y
				) 
			{
				// Collision detected!
				mPlayerBullets.erase(begin(mPlayerBullets) + bulletI);
				mEnemies.erase(begin(mEnemies) + enemyI);
				mScore += 10;
				collided = true;
				break;
			}
		}

		if (!collided)
		{
			//Check for collisions between player and enemy bullets
			for (int enemyBulletI = mEnemyBullets.size() - 1; enemyBulletI >= 0; --enemyBulletI)
			{
				auto& enemyBulletSprite = mEnemyBullets[enemyBulletI].bullet;
				auto enemyBulletSize = enemyBulletSprite.GetScreenSize();
				float enemyBulletWidth = enemyBulletSize.x / 4;   // 4 frames of animation
				if (
					bulletSprite.mPos.x < enemyBulletSprite.mPos.x + enemyBulletWidth &&
					bulletSprite.mPos.x + bulletWidth > enemyBulletSprite.mPos.x &&
					bulletSprite.mPos.y < enemyBulletSprite.mPos.y + enemyBulletSize.y &&
					bulletSprite.mPos.y + bulletSize.y > enemyBulletSprite.mPos.y
					)
				{
					// Collision detected!
					mPlayerBullets.erase(begin(mPlayerBullets) + bulletI);
					mEnemyBullets.erase(begin(mEnemyBullets) + enemyBulletI);
					collided = true;
					break;
				}
			}
		}
	}

	//check for enemy bullet collision with player 
	if (mRespawnTimer <= 0)
	{
		auto playerSize = mPlayer.GetScreenSize();
		for (int bulletI = mEnemyBullets.size() - 1; bulletI >= 0; --bulletI)
		{
			auto& bulletSprite = mEnemyBullets[bulletI].bullet;
			auto bulletSize = bulletSprite.GetScreenSize();
			float bulletWidth = bulletSize.x / 4;   // 4 frames of animation

			if (
				bulletSprite.mPos.x < mPlayer.mPos.x + playerSize.x &&
				bulletSprite.mPos.x + bulletWidth > mPlayer.mPos.x &&
				bulletSprite.mPos.y < mPlayer.mPos.y + playerSize.y &&
				bulletSprite.mPos.y + bulletSize.y > mPlayer.mPos.y
				)
			{
				// Collision detected!
				mEnemyBullets.erase(begin(mEnemyBullets) + bulletI);
				mLives--;
				mRespawnTimer = 3;
				break;
			}
		}
	}
}


void PlayMode::Update(float dTime)
{
	mRespawnTimer -= dTime;

	UpdateBgnd(dTime);

	UpdateBullets(dTime);

	UpdateInput(dTime);

	UpdateEnemies(dTime);
		
	UpdateCollisions();
}

void PlayMode::UpdateEnemies(float dTime)
{
	for (auto& enemy : mEnemies)
		enemy.Update(dTime);

	for (auto& enemy : mEnemies)
	{
		if (enemy.CheckSwitchDirection(mPlayArea))
		{
			for (auto& enemy2 : mEnemies)
			{
				enemy2.MoveDown();
			}
			break;
		}
	}

	
}

void PlayMode::Render(float dTime, DirectX::SpriteBatch & batch) {
	for (auto& s : mBgnd)
		s.Draw(batch);
	for (auto& bullet : mPlayerBullets)
		bullet.Render(batch);
	for (auto& bullet : mEnemyBullets)
		bullet.Render(batch);

	if (mRespawnTimer <= 0)
		mPlayer.Draw(batch);

	for (auto& enemy : mEnemies)
		enemy.Render(batch);

	//display lives
	Sprite lifeSprite(mD3D);
	for (int i = 0; i < mLives; ++i)
	{
		lifeSprite.SetTex(*mLivesTexture); 
		lifeSprite.SetScale(Vector2(0.05f, 0.05f));
		lifeSprite.mPos = Vector2(i * 20.f, 10.f);
		lifeSprite.Draw(batch);
	}

	// display score
	wstringstream ss;
	ss << "Score: " << mScore;
	mSpriteFont->DrawString(&batch, ss.str().c_str(), XMFLOAT2(0, 50));
}

bool PlayMode::IsGameOver()
{
	return mLives <= 0 || mEnemies.empty();
}

void PlayMode::InitBgnd()
{
	//a sprite for each layer
	assert(mBgnd.empty());
	mBgnd.insert(mBgnd.begin(), BGND_LAYERS, Sprite(mD3D));

	//a neat way to package pairs of things (nicknames and filenames)
	pair<string, string> files[BGND_LAYERS]{
		{ "bgnd0","backgroundlayers/nebulawetstars.dds" },
		{ "bgnd1","backgroundlayers/nebuladrystars.dds" },
	};
	int i = 0;
	for (auto& f : files)
	{
		//set each texture layer
		ID3D11ShaderResourceView *p = mD3D.GetCache().LoadTexture(&mD3D.GetDevice(), f.second, f.first);
		if (!p)
			assert(false);
		mBgnd[i++].SetTex(*p);
	}

}

void PlayMode::InitPlayer()
{
	//load a orientate the ship
	ID3D11ShaderResourceView *p = mD3D.GetCache().LoadTexture(&mD3D.GetDevice(), "ship.dds");
	mPlayer.SetTex(*p);
	mPlayer.SetScale(Vector2(0.1f, 0.1f));
	mPlayer.origin = mPlayer.GetTexData().dim / 2.f;
	//mPlayer.rotation = PI / 2.f;

	//setup the play area
	int w, h;
	WinUtil::Get().GetClientExtents(w, h);
	mPlayArea.left = mPlayer.GetScreenSize().x*0.6f;
	mPlayArea.top = h * 0.9f;
	mPlayArea.right = w - mPlayArea.left;
	mPlayArea.bottom = h * 0.9f;
	mPlayer.mPos = Vector2(w/2.0f, mPlayArea.bottom);

}