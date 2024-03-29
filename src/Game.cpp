#include "Game.h"
#include "SDL_Instance.h"
#include "SceneManager.h"
#include "MenuManager.h"
#include "Client.h"
#include "HUD.h"

Game::Game()
{
	exeName = "Chuckie Egg";
	done = false;
	instance = new SDL_Instance(this, 1080, 720);
	Timer realTime = Timer(0.02f);

	Initialize();

	while (!done)
	{
		if (realTime.IsDone())
		{
			HandleInput();
			Update();
			Render();
		}
	}

	CleanExit();
}

Game::~Game()
{
	
}

void Game::Initialize()
{
	texManager = new TextureManager(*instance, "./assets/");
	texManager->LoadTexture("spritesheet");

	sceneMngr = new SceneManager(this, "./assets/levels.data");
	instance->SetRenderScale(sceneMngr->GetScalar().x, sceneMngr->GetScalar().y);

	audioMngr = new AudioManager(this, glm::vec2(250, 380));
	audioMngr->LoadClip("jump", "./assets/audio/player_jump.wav");
	audioMngr->LoadClip("pickup", "./assets/audio/player_pickup.wav");
	audioMngr->LoadClip("fall", "./assets/audio/player_fall.wav");
	audioMngr->LoadClip("walk", "./assets/audio/player_walk.wav");
	audioMngr->LoadMusic("./assets/audio/music.wav");
	audioMngr->SetLoopClip("walk");

	menuMngr = new MenuManager(this, "./assets/menus.data");

	hud = new HUD(this, glm::vec2(150, 30), 22);
}

int x = 0;
void Game::Update()
{
	if (server != nullptr)
	{
		if (server->online)
		{
			server->CheckIncoming();

			if (!playing && server->GetPlayerCount() > 1)
				menuMngr->SetActiveMenu(menuMngr->FindMenuByTag("load"));
		}
		else server = nullptr;
	}

	if (client != nullptr)
	{
		if (client->IsOnline())
		{
			client->CheckIncoming(this);
		}
		else client = nullptr;
	}

	if (playing && !paused)
	{
		sceneMngr->Update();
		hud->Update();
	}
	else menuMngr->Update();

	audioMngr->Update();
}

void Game::HandleInput()
{
	while (SDL_PollEvent(&event))
	{
		if (playing && !paused)
			sceneMngr->HandleInput(event);
		else menuMngr->HandleInput(event);

		audioMngr->HandleInput(event);

		switch (event.type)
		{
			case SDL_QUIT:
				done = true;
				break;

			case SDL_KEYDOWN:
				if (!event.key.repeat)
				{
					switch (event.key.keysym.sym)
					{
						case SDLK_ESCAPE:
							SetGameState("pause");
							break;
						case SDLK_F11:
							instance->ToggleFullscreen();
							break;
					}
				}
				break;

			case SDL_KEYUP:
				break;
		}
	}
}

void Game::Render()
{
	SDL_RenderClear(instance->GetRenderer());
	SDL_RenderSetLogicalSize(instance->GetRenderer(), sceneMngr->GetScalar().x, sceneMngr->GetScalar().y);

	if (playing && !paused)
	{
		sceneMngr->Render();
		hud->Render();
	}
	else menuMngr->Render();

	audioMngr->Render();

	SDL_RenderPresent(instance->GetRenderer());
}

void Game::CleanExit(char* message)
{
	if (message != "")
		std::cout << message << std::endl;

	if (instance != nullptr) instance->CleanUp();
	if (audioMngr != nullptr) audioMngr->CleanUp();
	if (server != nullptr) server->Quit();
	exit(1);
}

void Game::SetGameState(char* state)
{
	if (StrLib::str_contains(state, "quit"))
		CleanExit();
	else if (StrLib::str_contains(state, "play"))
		SetPlaying(!playing);
	else if (StrLib::str_contains(state, "gameover"))
	{
		SetPlaying(false);
		menuMngr->SetActiveMenu(menuMngr->FindMenuByTag("gameover"));
	}
	else if (StrLib::str_contains(state, "pause"))
		SetPaused(!paused);
	else if (StrLib::str_contains(state, "host-game"))
		server = new Server(this, 2);
	else if (StrLib::str_contains(state, "find-game"))
		client = new Client("127.0.0.1");
}

void Game::SetPlaying(bool value)
{
	playing = value;
	paused = false;

	if (!value)
	{
		sceneMngr->Initialize(this);
		hud->SaveScores("./assets/saves/");
		menuMngr->SetActiveMenu(menuMngr->FindMenuByTag("main"));

		if (server != nullptr)
		{
			server->online = false;
			server->Quit();
		}
		else if (client != nullptr)
			client->Quit();
	}
	else
	{
		if (server != nullptr)
			server->CirculateMsg(-1, "5*\n");
	}
}

void Game::SetPaused(bool value)
{
	paused = value;

	if (value)
		menuMngr->SetActiveMenu(menuMngr->FindMenuByTag("pause"));
}

char* &Game::GetName() { return exeName; }
SDL_Instance* &Game::GetSDLInstance() { return instance; }
TextureManager* &Game::GetTextureManager() { return texManager; }
SceneManager* &Game::GetSceneManager() { return sceneMngr; }
MenuManager* &Game::GetMenuManager() { return menuMngr; }
AudioManager* &Game::GetAudioManager() { return audioMngr; }
SDL_Event &Game::GetEvent() { return event; }
Client* &Game::GetClient() { return client; }
Server* &Game::GetServer() { return server; }
HUD* &Game::GetHUD() { return hud; }