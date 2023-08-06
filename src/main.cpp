#include "gui.h"

int main(int, char**)
{
    if (OverLayGui::OverlayInit())
        return 1;

    if (OverLayGui::OverlayStart())
        return 1;

    OverLayGui::OverlayDestroy();

    return 0;
}