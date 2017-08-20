#source files used by xyginext library
set(PROJECT_SRC 
  ${PROJECT_DIR}/core/App.cpp
  ${PROJECT_DIR}/core/FileSystem.cpp
  ${PROJECT_DIR}/core/MessageBus.cpp
  ${PROJECT_DIR}/core/State.cpp
  ${PROJECT_DIR}/core/StateStack.cpp
  ${PROJECT_DIR}/core/SysTime.cpp

  ${PROJECT_DIR}/ecs/Component.cpp
  ${PROJECT_DIR}/ecs/Director.cpp
  ${PROJECT_DIR}/ecs/Entity.cpp
  ${PROJECT_DIR}/ecs/EntityManager.cpp
  ${PROJECT_DIR}/ecs/Scene.cpp
  ${PROJECT_DIR}/ecs/System.cpp
  ${PROJECT_DIR}/ecs/SystemManager.cpp

  ${PROJECT_DIR}/ecs/components/Sprite.cpp
  ${PROJECT_DIR}/ecs/components/Text.cpp
  ${PROJECT_DIR}/ecs/components/Transform.cpp

  ${PROJECT_DIR}/ecs/systems/CommandSystem.cpp
  ${PROJECT_DIR}/ecs/systems/SpriteRenderer.cpp
  ${PROJECT_DIR}/ecs/systems/TextRenderer.cpp

  ${PROJECT_DIR}/resources/FontResource.cpp
  ${PROJECT_DIR}/resources/ShaderResource.cpp
  
  ${PROJECT_DIR}/util/ConfigFile.cpp
  ${PROJECT_DIR}/util/Random.cpp)
