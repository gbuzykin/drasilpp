#pragma once

#include "math.h"

namespace drasilpp::rel {

struct Camera {
    Vec3f eye{0.f, 0.f, 4.f};
    Vec3f center{0.f, 0.f, 0.f};
    Vec3f up{0.f, 1.f, 0.f};
};

class OrbitCameraManipulator {
 public:
    OrbitCameraManipulator(Camera& camera) : camera_(camera) {}

    enum class DragAction { NONE = 0, MOVE, ROTATE };

    void startDragging(const Vec2f& p, DragAction action) {
        drag_action_ = action;
        p0_ = p;
        center0_ = camera_.center;
        back0_ = camera_.eye - camera_.center;
        right0_ = normalize(cross(camera_.up, back0_));
        up0_ = normalize(cross(back0_, right0_));
    }

    void stopDragging() { drag_action_ = DragAction::NONE; }

    void drag(const Vec2f& p) {
        switch (drag_action_) {
            case DragAction::MOVE: {
                camera_.center = center0_ + MOVE_SENSITIVITY * ((p0_.x - p.x) * right0_ + (p0_.y - p.y) * up0_);
                camera_.eye = camera_.center + back0_;
            } break;
            case DragAction::ROTATE: {
                const auto v0 = p0_.x * right0_ + p0_.y * up0_ - back0_;
                const auto v1 = p.x * right0_ + p.y * up0_ - back0_;
                const auto rot_axis = cross(v0, v1);

                const float l = length(rot_axis);
                const float a = l / (length(v0) * length(v1));
                const auto rotation = a >= 0.001 ?
                                          Mat4f::rotate(ROTATION_SENSITIVITY * std::asin(a), (1.f / l) * rot_axis) :
                                          Mat4f::identity();

                camera_.eye = camera_.center + back0_ * rotation;
                camera_.up = up0_ * rotation;
            } break;
            default: break;
        }
    }

    void moveZ(float delta) {
        const auto v = delta * (camera_.center - camera_.eye);
        center0_ = center0_ + v;
        camera_.center = camera_.center + v;
        camera_.eye = camera_.eye + v;
    }

 private:
    Camera& camera_;

    static constexpr float MOVE_SENSITIVITY = 5.f;
    static constexpr float ROTATION_SENSITIVITY = 200.f;

    DragAction drag_action_{DragAction::NONE};
    Vec2f p0_{};
    Vec3f center0_{};
    Vec3f back0_{};
    Vec3f right0_{};
    Vec3f up0_{};
};

}  // namespace drasilpp::rel
