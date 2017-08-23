#pragma once

#if defined(__DAVAENGINE_BEAST__)

#include <TArc/Core/ClientModule.h>
#include <TArc/Utils/QtConnections.h>

#include <QtTools/Utils/QtDelayedExecutor.h>

#include <Beast/BeastConstants.h>

#include <Reflection/Reflection.h>

#include <memory>

class BeastModule : public DAVA::TArc::ClientModule
{
protected:
    void PostInit() override;

private:
    void OnBeastAndSave();
    void RunBeast(const QString& outputPath, Beast::eBeastMode mode);

    DAVA::TArc::QtConnections connections;
    QtDelayedExecutor delayedExecutor;

    DAVA_VIRTUAL_REFLECTION(BeastModule, DAVA::TArc::ClientModule);
};

#endif //#if defined (__DAVAENGINE_BEAST__)
