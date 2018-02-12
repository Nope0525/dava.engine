#include "Classes/CommandLine/Private/SceneConsoleHelper.h"
#include "Classes/CommandLine/Private/OptionName.h"

#include <TArc/Utils/RenderContextGuard.h>

#include <CommandLine/ProgramOptions.h>
#include <Math/Color.h>
#include <Render/Renderer.h>
#include <Render/RenderHelper.h>
#include <Render/RHI/rhi_Public.h>
#include <Scene3D/Systems/QualitySettingsSystem.h>

namespace SceneConsoleHelperDetail
{
/*
 * Flush implementation
 * temporary (hopefully!) solution to clean-up RHI's objects
 * when there is no run/render loop in the application
 */
DAVA_DEPRECATED(void Flush())
{
    static const rhi::HTexture nullTexture;
    static const rhi::Viewport nullViewport(0, 0, 1, 1);

    rhi::HSyncObject currentFrame = rhi::GetCurrentFrameSyncObject();
    while (!rhi::SyncObjectSignaled(currentFrame))
    {
        DAVA::Renderer::BeginFrame();
        DAVA::RenderHelper::CreateClearPass(nullTexture, nullTexture, 0, DAVA::Color::Clear, nullViewport);
        DAVA::Renderer::EndFrame();
    }
}
}

DAVA::FilePath SceneConsoleHelper::CreateQualityPathname(const DAVA::FilePath& qualityPathname, const DAVA::FilePath& targetPathname, const DAVA::FilePath& resourceFolder)
{
    if (qualityPathname.IsEmpty() == false)
    {
        return qualityPathname;
    }

    if (resourceFolder.IsEmpty() == false)
    {
        return resourceFolder + "/quality.yaml";
    }

    DAVA::String fullPath = targetPathname.GetAbsolutePathname();

    DAVA::String::size_type pos = fullPath.find("/Data");
    if (pos != DAVA::String::npos)
    {
        return (fullPath.substr(0, pos) + "/DataSource/quality.yaml");
    }

    return DAVA::FilePath();
}

bool SceneConsoleHelper::InitializeQualitySystem(const DAVA::ProgramOptions& options, const DAVA::FilePath& targetPathname)
{
    DAVA::FilePath resourceFolder;

    if (options.IsOptionExists(OptionName::ResourceDir))
    {
        resourceFolder = options.GetOption(OptionName::ResourceDir).AsString();
    }

    DAVA::FilePath qualityPathname = options.GetOption(OptionName::QualityConfig).AsString();
    qualityPathname = CreateQualityPathname(qualityPathname, targetPathname, resourceFolder);
    if (qualityPathname.IsEmpty())
    {
        return false;
    }

    DAVA::QualitySettingsSystem::Instance()->Load(qualityPathname);
    return true;
}

void SceneConsoleHelper::FlushRHI()
{
    DAVA::RenderContextGuard guard;
    SceneConsoleHelperDetail::Flush();
}
