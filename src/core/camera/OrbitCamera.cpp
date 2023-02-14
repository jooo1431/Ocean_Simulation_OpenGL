#include "OrbitCamera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imGuIZMOquat.h>
#include <imgui.h>

using namespace OGL4Core2::Core;

OrbitCamera::OrbitCamera(float dolly)
    : defaultDolly_(dolly),
      dolly_(dolly),
      panx_(0.0f),
      pany_(0.0f),
      viewMx_(glm::mat4(1.0)) {
    updateMx();
}

void OrbitCamera::mouseMoveControl(MouseControlMode mode, double oldX, double oldY, double newX, double newY) {
    if (mode == MouseControlMode::Left) {
        rotate(static_cast<float>(oldX), static_cast<float>(oldY), static_cast<float>(newX), static_cast<float>(newY));
    } else if (mode == MouseControlMode::Right) {
        moveDolly(4.0f * static_cast<float>(oldY - newY));
    } else if (mode == MouseControlMode::Middle) {
        panx_ += static_cast<float>(newX - oldX);
        pany_ += static_cast<float>(newY - oldY);
        updateMx();
    }
}

void OrbitCamera::mouseScrollControl([[maybe_unused]] double xoffset, double yoffset) {
    moveDolly(-0.5f * static_cast<float>(yoffset));
}

void OrbitCamera::drawGUI() {
    bool changed = false;

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 0.0f);
    if (ImGui::TreeNode("Orbit Camera")) {
        const auto& q = t_.getQ();
        ImGui::Text("q: x:% 6.3f y:% 6.3f z:% 6.3f w:% 6.3f", q.x, q.y, q.z, q.w);
        ImGui::Columns(3, "orbitcamera", false);

        ImGui::SetColumnWidth(0, 100.0f);
        changed |= ImGui::gizmo3D("#gizmo", t_.getQ());
        ImGui::NextColumn();

        ImGui::SetColumnWidth(1, 10000.0f);
        changed |= ImGui::DragFloat("Dolly", &dolly_, 0.01f, 0.0f, 50.0f);
        changed |= ImGui::DragFloat("PanX", &panx_, 0.01f, -10.0f, 10.0f);
        changed |= ImGui::DragFloat("PanY", &pany_, 0.01f, -10.0f, 10.0f);
        if (ImGui::Button("Reset")) {
            t_.reset();
            dolly_ = defaultDolly_;
            panx_ = 0.0f;
            pany_ = 0.0f;
            changed = true;
        }
        ImGui::NextColumn();

        ImGui::Columns(1);
        ImGui::Separator();
        ImGui::TreePop();
    }
    ImGui::PopStyleVar();

    if (changed) {
        updateMx();
    }
}

void OrbitCamera::rotate(float p1x, float p1y, float p2x, float p2y) {
    t_.track(p1x, p1y, p2x, p2y);
    updateMx();
}

void OrbitCamera::moveDolly(float d) {
    dolly_ += d;
    if (dolly_ < 0.0f) {
        dolly_ = 0.0f;
    }
    updateMx();
}

void OrbitCamera::updateMx() {
    viewMx_ = glm::translate(glm::mat4(1.0f), glm::vec3(panx_, pany_, -dolly_));
    viewMx_ *= t_.getRot();
}
