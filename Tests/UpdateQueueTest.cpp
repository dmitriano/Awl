#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>

#include "Awl/UpdateQueue.h"

#include "UnitTesting.h"

using namespace UnitTesting;

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

    Assert::IsTrue(scene.Rotation == 0 && !scene.PerspectiveMode, _T("Updates have been applyed before ApplyUpdates() is called."));

    scene.Draw();

    //GCC 4.7.3 does not have std::this_thread::sleep.
#if !(__GNUC__ == 4 && __GNUC_MINOR__ == 7)
    //This sleep simulates the delay between flips that can be 1000/60 = 16.66ms, for example. (is not compiled by GCC 4.7.3)
    std::this_thread::sleep_for(std::chrono::milliseconds(17));
#endif

    updateQueue.ApplyUpdates(scene);

    Assert::IsTrue(scene.Rotation == 2 && scene.PerspectiveMode, _T("Updates have not been applyed."));

    scene.Draw();
}

void TestUpdateQueue()
{
    std::thread main_thread(UserActionsFunc);

    std::thread render_thread(SceneRenderingFunc);

    render_thread.join();

    main_thread.join();
}
