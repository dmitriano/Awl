#include "Awl/UpdateQueue.h"

#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>

class GameScene
{
public:

	//! The scene orientation. Possible values are 0, 1, 2, 3.
	uint8_t Rotation = 0;

	//! Indicates if 3D mode is enabled.
	bool PerspectiveMode = false;

	void Draw();
};

void GameScene::Draw()
{
	std::ostringstream out;

	out << "Drawing the scene with the following settings: Rotation=" << (int)Rotation << " PerspectiveMode=" << (PerspectiveMode ? "true" : "false") << std::endl;
	
	std::cout << out.str();
}

awl::UpdateQueue<GameScene &> updateQueue;

void UserActionsFunc()
{
	std::cout << "The user has changed Rotation\n";

	updateQueue.Push([](GameScene & scene)
	{
		scene.Rotation = 2;
	});

	std::cout << "The user has changed PerspectiveMode\n";

	updateQueue.Push([](GameScene & scene)
	{
		scene.PerspectiveMode = true;
	});
}

void SceneRenderingFunc()
{
	GameScene scene;

	scene.Draw();

	//This sleep simulates the delay between flips that can be 1000/60 = 16.66ms, for example.
	std::this_thread::sleep_for(std::chrono::milliseconds(17));
	
	updateQueue.ApplyUpdates(scene);

	scene.Draw();
}

void main()
{
	std::thread main_thread(UserActionsFunc);
	
	std::thread render_thread(SceneRenderingFunc);

	main_thread.join();

	render_thread.join();
}