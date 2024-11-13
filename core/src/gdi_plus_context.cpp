#include "gdi_plus_context.h"

gdi_plus_context::gdi_plus_context() :
	handle(0, [](ULONG_PTR _handle) {
	Gdiplus::GdiplusShutdown(_handle);
		})
{
	Gdiplus::GdiplusStartupInput startup_input;
	Gdiplus::GdiplusStartup(&handle, &startup_input, NULL);
}
