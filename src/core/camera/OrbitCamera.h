#pragma once

#include <glm/fwd.hpp>

#include "AbstractCamera.h"
#include "Trackball.h"

namespace OGL4Core2::Core {
    class OrbitCamera : public AbstractCamera {
    public:
        explicit OrbitCamera(float dolly = 5.0f);
        ~OrbitCamera() override = default;

        void mouseMoveControl(MouseControlMode mode, double oldX, double oldY, double newX, double newY) override;
        void mouseScrollControl(double xoffset, double yoffset) override;

        [[nodiscard]] inline const glm::mat4& viewMx() const {
            return viewMx_;
        };

        void drawGUI();

        void rotate(float p1x, float p1y, float p2x, float p2y);

        void moveDolly(float d);

    private:
        void updateMx();

        float defaultDolly_;

        Trackball t_;
        float dolly_;
        float panx_;
        float pany_;
        glm::mat4 viewMx_;
    };
} // namespace OGL4Core2::Core
