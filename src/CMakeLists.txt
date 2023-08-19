cmake_minimum_required(VERSION 3.0.0)

# название, версия, языки (не обязательно)
project(prj VERSION 0.1.0 LANGUAGES C CXX)

# пихаю пути в переменные
set(imguisrc ".\\imgui")
set(imguiback ".\\backends")
set(DirectXTK ".\\DirectXTK")
set(DirectXMath ".\\DirectXMath")

#set(CMAKE_BUILD_RPATH "..\\build")

# сначала команда на билд
add_executable(prj main.cpp gui.cpp graphics.cpp)
# добавляем директорию с библиотекой (это запускает ./imgui/CMakeLists.txt)
#add_subdirectory(${imguisrc})
# зависимости для работы #include
#target_include_directories(prj PRIVATE ${imguisrc})
#target_link_directories(prj BEFORE PUBLIC ${imguisrc})
#target_include_directories(prj PRIVATE ${imguiback})
#target_link_directories(prj BEFORE PUBLIC ${imguiback})
# добавляем нужные либы
target_link_libraries(prj PRIVATE 
    d3d11.lib dwmapi.lib gdiplus.lib d2d1.lib user32.lib)

# ВАЖНО
# Зачем добаляем зависимости, а потом еще и либы и нахуя их компилить?
# При объявлении зависимостей мы указываем где искать хедеры (.h)
# В (.h) файлах содержатся только объявления функций без их описания
# Описания функций как раз и билдятся в .lib/.dll для всех внешних либ до начала сборки мейна

# Разница .lib/.dll
# .lib быстрее, но привязан к текущей сборке и чтобы его заменить, например, на 
# новую версию, надо ребилдить проект
# .dll подгружается во время выполнения проги и поэтому его можно менять без ребилда

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
