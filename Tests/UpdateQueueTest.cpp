/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <thread>
#include <chrono>

#include "Awl/UpdateQueue.h"
#include "Awl/Testing/UnitTest.h"

using namespace awl::testing;

namespace
{
    using TestMutex = std::recursive_mutex;
    using TestLock = std::unique_lock<TestMutex>;

    static TestMutex testMutex;

    class GameScene
    {
    public:

        //! The scene orientation. Possible values are 0, 1, 2, 3.
        uint8_t Rotation = 0;

        //! Indicates if 3D mode is enabled.
        bool PerspectiveMode = false;

        void Draw(const awl::testing::TestContext & context);
    };

    void GameScene::Draw(const awl::testing::TestContext & context)
    {
        TestLock lock(testMutex);

        context.out << _T("Drawing the scene with the following settings: Rotation=") << (int)Rotation << _T(" PerspectiveMode=") << (PerspectiveMode ? "true" : "false") << std::endl;
    }

    awl::UpdateQueue<GameScene &> updateQueue;

    static void UserActionsFunc(const awl::testing::TestContext & context)
    {
        {
            TestLock lock(testMutex);

            context.out << _T("The user has changed Rotation\n");
        }

        updateQueue.Push([](GameScene & scene)
        {
            scene.Rotation = 2;
        });

        {
            TestLock lock(testMutex);

            context.out << _T("The user has changed PerspectiveMode\n");
        }

        updateQueue.Push([](GameScene & scene)
        {
            scene.PerspectiveMode = true;
        });
    }

    static void SceneRenderingFunc(const awl::testing::TestContext & context)
    {
        GameScene scene;

        AWL_ASSERTM_TRUE(scene.Rotation == 0 && !scene.PerspectiveMode, _T("Updates have been applyed before ApplyUpdates() is called."));

        scene.Draw(context);

        std::this_thread::sleep_for(std::chrono::milliseconds(17));

        updateQueue.ApplyUpdates(scene);

        AWL_ASSERTM_TRUE(scene.Rotation == 2 && scene.PerspectiveMode, _T("Updates have not been applyed."));

        scene.Draw(context);
    }
}

AWL_TEST(UpdateQueue)
{
    //this does not test async. version yet.
    
    std::thread main_thread(UserActionsFunc, context);

    main_thread.join();

    std::thread render_thread(SceneRenderingFunc, context);

    render_thread.join();
}

