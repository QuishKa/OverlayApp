#include "gui.h"

#ifdef _DEBUG
#define DX11_ENABLE_DEBUG_LAYER
#endif

#ifdef DX11_ENABLE_DEBUG_LAYER
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif

int main(int, char**)
{
    if (MainGui::MainGuiInit())
        return 1;

    return 0;
}