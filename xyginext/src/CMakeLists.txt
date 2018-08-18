#source files used by xyginext library
set(PROJECT_SRC ${PROJECT_SRC}
  ${CMAKE_CURRENT_SOURCE_DIR}/audio/AudioSourceImpl.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/audio/Mixer.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/core/App.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/ConfigFile.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/Console.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/ConsoleClient.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/FileSystem.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/MessageBus.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/State.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/StateStack.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/SysTime.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/detail/glad.c
  ${CMAKE_CURRENT_SOURCE_DIR}/detail/Operators.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/Component.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/Director.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/Entity.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/EntityManager.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/Scene.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/System.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/SystemManager.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/components/AudioEmitter.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/components/Camera.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/components/Drawable.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/components/NetInterpolation.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/components/ParticleEmitter.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/components/QuadTreeItem.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/components/Sprite.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/components/Text.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/components/Transform.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/systems/AudioSystem.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/systems/CallbackSystem.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/systems/CameraSystem.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/systems/CommandSystem.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/systems/InterpolationSystem.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/systems/ParticleSystem.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/systems/QuadTree.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/systems/QuadTreeNode.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/systems/RenderSystem.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/systems/SpriteAnimator.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/systems/SpriteSystem.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/systems/TextRenderer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/systems/TextSystem.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/systems/UISystem.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/graphics/SpriteSheet.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/graphics/postprocess/PostAntique.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/graphics/postprocess/PostBloom.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/graphics/postprocess/PostBlur.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/graphics/postprocess/PostChromeAb.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/graphics/postprocess/PostOldSchool.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/graphics/postprocess/PostProcess.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/Gui.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/GuiClient.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_demo.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui-SFML.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_tabs.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/network/EnetClientImpl.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/network/EnetHostImpl.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/network/NetClient.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/network/NetConf.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/network/NetEvent.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/network/NetHost.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/network/NetPeer.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/resources/FontResource.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/resources/ShaderResource.cpp
  
  ${CMAKE_CURRENT_SOURCE_DIR}/util/Random.cpp
  PARENT_SCOPE)
