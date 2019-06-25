set(PROJECT_SRC 
  ${PROJECT_SRC}
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/TextEditor.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/WindowFunctions.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_demo.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui-SFML.cpp
  PARENT_SCOPE)