#pragma once
#include <Windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include "unique_handle.h"

// A GDI+ "context." GDI+ is intialized when the object is created and shut down when
// the object is destroyed. You must initialize GDI+ before using it; this object provides
// an easy RAII way to do that.
class gdi_plus_context {
public:
	gdi_plus_context();

private:
	unique_handle<ULONG_PTR> handle;
};

