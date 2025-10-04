#if defined(_WIN32) && defined(NDEBUG)
// In Windows Release builds, link as a GUI subsystem app to avoid showing a console window.
// MSVC requires a WinMain entry point with /SUBSYSTEM:WINDOWS. This tiny shim forwards to main().
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

extern "C" int main();

int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    return main();
}
#endif
