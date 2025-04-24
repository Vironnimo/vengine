#pragma once

namespace Vengine {

class Scene {
   public:
    Scene() = default;
    ~Scene() = default;

    void init();

    void update(float deltaTime);

    void render();

    void cleanup();
};

}  // namespace Vengine