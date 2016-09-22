#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
/// is a strongly typed enum class representing the engine run mode
enum class eEngineRunMode : int32
{
    GUI_STANDALONE = 0, /// Run engine as standalone GUI application
    GUI_EMBEDDED, /// Run engine inside other framework, e.g. Qt
    CONSOLE_MODE, /// Run engine as standalone console application
};

/// is a strongly typed enum class representing the status of mouse mode
enum class eMouseMode : int32
{
    OFF = 0, ///< Disable any capturing, show cursor (send absolute xy)
    FRAME, ///< Capture system cursor into window rect, show cursor (send absolute xy) */
    PINNING, ///< Capture system cursor on current position, and hide cursor (send xy move delta) */
    HIDE ///< Disable any capturing, and hide cursor (send absolute xy) */
};

} // namespace DAVA
