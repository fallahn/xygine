#source files used by xyginext library
set(PROJECT_SRC 
  ${PROJECT_DIR}/core/App.cpp
  ${PROJECT_DIR}/core/CommandQueue.cpp
  ${PROJECT_DIR}/core/FileSystem.cpp
  ${PROJECT_DIR}/core/MessageBus.cpp
  ${PROJECT_DIR}/core/State.cpp
  ${PROJECT_DIR}/core/StateStack.cpp
  ${PROJECT_DIR}/core/SysTime.cpp

  ${PROJECT_DIR}/ecs/Component.cpp
  ${PROJECT_DIR}/ecs/Director.cpp
  ${PROJECT_DIR}/ecs/Entity.cpp
  ${PROJECT_DIR}/ecs/EntityManager.cpp
  ${PROJECT_DIR}/ecs/Renderable.cpp
  ${PROJECT_DIR}/ecs/Scene.cpp
  ${PROJECT_DIR}/ecs/System.cpp
  ${PROJECT_DIR}/ecs/SystemManager.cpp)
