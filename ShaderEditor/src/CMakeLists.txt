set(PROJECT_SRC 
  ${PROJECT_SRC}
  ${CMAKE_CURRENT_SOURCE_DIR}/EditorWindow.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/glad.c
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Renderer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/TextEditor.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/tinyfiledialogs.c
  ${CMAKE_CURRENT_SOURCE_DIR}/WindowFunctions.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_demo.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui-SFML.cpp
  PARENT_SCOPE)