#source files used by xyginext library
set(PROJECT_SRC 
  ${PROJECT_DIR}/audio/AudioSourceImpl.cpp
  ${PROJECT_DIR}/audio/Mixer.cpp

  ${PROJECT_DIR}/core/App.cpp
  ${PROJECT_DIR}/core/ConfigFile.cpp
  ${PROJECT_DIR}/core/Console.cpp
  ${PROJECT_DIR}/core/ConsoleClient.cpp
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

  ${PROJECT_DIR}/ecs/components/AudioEmitter.cpp
  ${PROJECT_DIR}/ecs/components/Camera.cpp
  ${PROJECT_DIR}/ecs/components/NetInterpolation.cpp
  ${PROJECT_DIR}/ecs/components/Sprite.cpp
  ${PROJECT_DIR}/ecs/components/Text.cpp
  ${PROJECT_DIR}/ecs/components/Transform.cpp

  ${PROJECT_DIR}/ecs/systems/AudioSystem.cpp
  ${PROJECT_DIR}/ecs/systems/CallbackSystem.cpp
  ${PROJECT_DIR}/ecs/systems/CameraSystem.cpp
  ${PROJECT_DIR}/ecs/systems/CommandSystem.cpp
  ${PROJECT_DIR}/ecs/systems/InterpolationSystem.cpp
  ${PROJECT_DIR}/ecs/systems/SpriteAnimator.cpp
  ${PROJECT_DIR}/ecs/systems/SpriteRenderer.cpp
  ${PROJECT_DIR}/ecs/systems/TextRenderer.cpp
  ${PROJECT_DIR}/ecs/systems/UISystem.cpp

  ${PROJECT_DIR}/graphics/SpriteSheet.cpp

  ${PROJECT_DIR}/graphics/postprocess/PostChromeAb.cpp
  ${PROJECT_DIR}/graphics/postprocess/PostProcess.cpp

  ${PROJECT_DIR}/imgui/imgui.cpp
  ${PROJECT_DIR}/imgui/imgui_demo.cpp
  ${PROJECT_DIR}/imgui/imgui_draw.cpp
  ${PROJECT_DIR}/imgui/imgui_sfml.cpp

  ${PROJECT_DIR}/network/NetClient.cpp
  ${PROJECT_DIR}/network/NetConf.cpp
  ${PROJECT_DIR}/network/NetEvent.cpp
  ${PROJECT_DIR}/network/NetHost.cpp
  ${PROJECT_DIR}/network/NetPeer.cpp

  ${PROJECT_DIR}/resources/FontResource.cpp
  ${PROJECT_DIR}/resources/ShaderResource.cpp
  
  ${PROJECT_DIR}/util/Random.cpp)
