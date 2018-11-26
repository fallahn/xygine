#source files used by xyginext library
set(PROJECT_SRC  ${PROJECT_SRC}
  ${CMAKE_CURRENT_SOURCE_DIR}/audio/AudioSourceImpl.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/audio/Mixer.hpp

  ${CMAKE_CURRENT_SOURCE_DIR}/core/App.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/Assert.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/ConfigFile.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/Console.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/ConsoleClient.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/FileSystem.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/Log.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/Message.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/MessageBus.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/State.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/StateStack.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/SysTime.hpp

  ${CMAKE_CURRENT_SOURCE_DIR}/detail/Operators.hpp

  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/Component.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/ComponentPool.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/Director.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/Entity.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/Scene.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/System.hpp

  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/components/AudioEmitter.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/components/AudioListener.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/components/Callback.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/components/Camera.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/components/CommandTarget.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/components/Drawable.hpp
  #${CMAKE_CURRENT_SOURCE_DIR}/ecs/components/NetInterpolation.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/components/ParticleEmitter.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/components/QuadTreeItem.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/components/Sprite.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/components/SpriteAnimation.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/components/Text.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/components/Transform.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/components/UIHitBox.hpp

  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/systems/AudioSystem.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/systems/CallbackSystem.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/systems/CameraSystem.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/systems/CommandSystem.hpp
  #${CMAKE_CURRENT_SOURCE_DIR}/ecs/systems/InterpolationSystem.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/systems/ParticleSystem.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/systems/QuadTree.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/systems/RenderSystem.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/systems/SpriteAnimator.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/systems/SpriteSystem.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/systems/TextRenderer.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ecs/systems/UISystem.hpp

  ${CMAKE_CURRENT_SOURCE_DIR}/graphics/SpriteSheet.hpp

  ${CMAKE_CURRENT_SOURCE_DIR}/graphics/postprocess/Antique.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/graphics/postprocess/Bloom.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/graphics/postprocess/Blur.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/graphics/postprocess/ChromeAb.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/graphics/postprocess/OldSchool.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/graphics/postprocess/PostProcess.hpp

  ${CMAKE_CURRENT_SOURCE_DIR}/gui/Gui.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/gui/GuiClient.hpp

  ${CMAKE_CURRENT_SOURCE_DIR}/network/EnetClientImpl.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/network/EnetHostImpl.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/network/NetClient.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/network/NetData.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/network/NetHost.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/network/NetImpl.hpp

  ${CMAKE_CURRENT_SOURCE_DIR}/resources/Resource.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/resources/DejaVuSans.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/resources/ResourceHandler.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/resources/ShaderResource.hpp
  
  ${CMAKE_CURRENT_SOURCE_DIR}/util/Random.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/util/String.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/util/Vector.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/util/Wavetable.hpp
  PARENT_SCOPE)
