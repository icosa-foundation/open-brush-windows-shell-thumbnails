#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winreg.h>

namespace TiltThumb
{

#define TiltThumbHandlerCLSID L"{F84FAE32-0FD7-495A-A67F-59DD28E39763}"
#define TiltThumbHandlerName L"Tilt Thumb Handler"
#define TiltThumbHandlerExtension L".tilt"

#define IThumbnailProviderCLSID L"{E357FCCD-A995-4576-B01F-234630154E96}"

} // namespace TiltThumb
