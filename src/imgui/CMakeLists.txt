cmake_minimum_required(VERSION 3.0.0)

# куда пукнуть либу
SET(LIBRARY_OUTPUT_PATH "C:\\cpp\\build")

# команда на билд libimgui.dll из всех .cpp файлов (все функции из них будут собраны в .dll)
add_library(imgui SHARED 
    imgui.cpp 
    imgui_widgets.cpp 
    imgui_tables.cpp 
    imgui_draw.cpp 
    imgui_demo.cpp 
    imgui_impl_dx11.cpp 
    imgui_impl_win32.cpp)

# явно указываю использование этих std библиотек потому что без этого не компилится
target_link_libraries(imgui PRIVATE 
    "d3d11.lib" 
    "dxgi.lib" 
    "d3dcompiler.lib" 
    "dwmapi.lib")