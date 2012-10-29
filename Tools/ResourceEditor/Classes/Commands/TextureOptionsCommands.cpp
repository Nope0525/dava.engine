#include "TextureOptionsCommands.h"
#include "../SceneEditor/EditorSettings.h"
#include "../SceneEditor/SceneValidator.h"

using namespace DAVA;

ReloadTexturesAsCommand::ReloadTexturesAsCommand(Texture::TextureFileFormat format)
    :   Command(COMMAND_CLEAR_UNDO_QUEUE)
    ,   fileFormat(format)
{
}

void ReloadTexturesAsCommand::Execute()
{
    Texture::SetDefaultFileFormat(fileFormat);
    
    EditorSettings::Instance()->SetTextureViewFileFormat(fileFormat);
    EditorSettings::Instance()->Save();
    
    SceneValidator::Instance()->ReloadTextures(fileFormat);
}

