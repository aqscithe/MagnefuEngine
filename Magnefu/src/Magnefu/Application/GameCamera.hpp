#pragma once

#include "Magnefu/Application/Input/Input.h"
#include "Magnefu/Core/Camera.hpp"



namespace Magnefu
{
    struct GameCamera
    {


        void                            init(bool enabled = true, f32 rotation_speed = 10.f, f32 movement_speed = 10.f, f32 movement_delta = 0.1f);
        void                            reset();

        void                            update(Magnefu::InputService* input, u32 window_width, u32 window_height, f32 delta_time);
        void                            apply_jittering(f32 x, f32 y);

        Magnefu::Camera                 camera;

        f32                             target_yaw;
        f32                             target_pitch;

        f32                             mouse_sensitivity;
        f32                             movement_delta;
        u32                             ignore_dragging_frames;

        vec3s                           target_movement;

        bool                            enabled;
        bool                            mouse_dragging;

        f32                             rotation_speed;
        f32                             movement_speed;
    }; // struct GameCamera

}