#pragma once
#include <windows.h>
#include "gameruntime/sample_registry.h"

// Shows a modal Win32 listbox dialog for picking a sample.
// Returns the selected index, or -1 if the user cancelled.
// Special cases:
//   count == 0 -> shows an error message box and returns -1.
//   count == 1 -> returns 0 immediately without showing any dialog
//                 (preserves the original behavior when a single
//                  --build_xxx flag is set).
int show_sample_selector(HINSTANCE hInst, const Cyber::Samples::SampleEntry* entries, int count);
