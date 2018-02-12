#include "Classes/Beast/BeastCommandLineTool.h"

#if defined(__DAVAENGINE_BEAST__)

#include "Classes/CommandLine/Private/CommandLineModuleTestUtils.h"
#include <REPlatform/Global/CommandLineModule.h>

#include <TArc/Testing/ConsoleModuleTestExecution.h>
#include <TArc/Testing/TArcUnitTests.h>

#include <Base/BaseTypes.h>
#include <FileSystem/FileSystem.h>
#include <Render/Highlevel/Landscape.h>
#include <Render/Highlevel/RenderBatch.h>
#include <Render/Highlevel/RenderObject.h>
#include <Render/Material/NMaterial.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Scene.h>
#include <Scene3D/SceneFileV2.h>

namespace BCLTestDetail
{
const DAVA::String projectStr = "~doc:/Test/BeastCommandLineTool/";
const DAVA::String scenePathnameStr = projectStr + "DataSource/3d/Scene/testScene.sc2";
const DAVA::String outPathnameStr = projectStr + "DataSource/3d/Scene/lightmap/";
}

DAVA_TARC_TESTCLASS(BeastCommandLineToolTest)
{
    void TestScene() const
    {
        //GFX_COMPLETE fix beast test
        return;

        using namespace DAVA;

        ScopedPtr<Scene> scene(new Scene());
        TEST_VERIFY(scene->LoadScene(BCLTestDetail::scenePathnameStr) == DAVA::SceneFileV2::eError::ERROR_NO_ERROR);

        uint32 entityCount = scene->GetChildrenCount();
        for (uint32 e = 0; e < entityCount; ++e)
        {
            Entity* child = scene->GetChild(e);

            RenderObject* ro = GetRenderObject(child);
            if (ro != nullptr)
            {
                NMaterial* material = nullptr;
                if (ro->GetType() == RenderObject::TYPE_LANDSCAPE)
                {
                    Landscape* landscape = static_cast<Landscape*>(ro);
                    material = landscape->GetPageMaterials(0, 0);
                }
                else
                {
                    uint32 rbCount = ro->GetRenderBatchCount();
                    for (uint32 r = 0; r < rbCount; ++r)
                    {
                        RenderBatch* rb = ro->GetRenderBatch(r);
                        TEST_VERIFY(rb != nullptr);
                        if (rb)
                        {
                            material = rb->GetMaterial();
                        }
                    }
                }

                //test material
                if (material != nullptr)
                {
                    const UnorderedMap<FastName, MaterialTextureInfo*>& textures = material->GetLocalTextures();
                    for (auto& tx : textures)
                    {
                        if (tx.first == FastName("lightmap"))
                        {
                            TEST_VERIFY(FileSystem::Instance()->Exists(tx.second->path));
                            TEST_VERIFY(tx.second->path.GetDirectory().GetAbsolutePathname() == FilePath(BCLTestDetail::outPathnameStr).GetAbsolutePathname());
                        }
                    }
                }
            }
        }
        TEST_VERIFY(FileSystem::Instance()->Exists(BCLTestDetail::outPathnameStr + "landscape.png"));
    }

    DAVA::Vector<DAVA::eGPUFamily> gpuLoadingOrder;
    std::unique_ptr<DAVA::CommandLineModule> tool;
    bool testCompleted = false;

    DAVA_TEST (BeastTest)
    {
        using namespace DAVA;

        gpuLoadingOrder = DAVA::Texture::GetGPULoadingOrder();
        Texture::SetGPULoadingOrder({ eGPUFamily::GPU_ORIGIN });

        CommandLineModuleTestUtils::CreateProjectInfrastructure(BCLTestDetail::projectStr);
        CommandLineModuleTestUtils::SceneBuilder::CreateFullScene(BCLTestDetail::scenePathnameStr, BCLTestDetail::projectStr);

        Vector<String> cmdLine =
        {
          "ResourceEditor",
          "-beast",
          "-file",
          FilePath(BCLTestDetail::scenePathnameStr).GetAbsolutePathname(),
          "-output",
          FilePath(BCLTestDetail::outPathnameStr).GetAbsolutePathname(),
        };

        tool.reset(new BeastCommandLineTool(cmdLine));
        DAVA::ConsoleModuleTestExecution::InitModule(tool.get());
    }

    void Update(DAVA::float32 timeElapsed, const DAVA::String& testName) override
    {
        if (tool)
        {
            testCompleted = DAVA::ConsoleModuleTestExecution::ProcessModule(tool.get());
        }
    }

    bool TestComplete(const DAVA::String& testName) const override
    {
        if (testCompleted && tool)
        {
            DAVA::ConsoleModuleTestExecution::FinalizeModule(tool.get());

            TestScene();
            CommandLineModuleTestUtils::ClearTestFolder(BCLTestDetail::projectStr);
            DAVA::Texture::SetGPULoadingOrder(gpuLoadingOrder);
        }

        return testCompleted;
    }

    BEGIN_FILES_COVERED_BY_TESTS()
    FIND_FILES_IN_TARGET(TArc)
    DECLARE_COVERED_FILES("BeastCommandLineTool.cpp")
    END_FILES_COVERED_BY_TESTS();
};

#endif //BEAST
