#pragma once
#include <string>

namespace dx3d
{
    class EventNames
    {
    public:
		inline static const std::string PERSPECTIVE_MODE_TOGGLE = "PERSPECTIVE_MODE_TOGGLE";
		inline static const std::string ORTHOGRAPHIC_MODE_TOGGLE = "ORTHOGRAPHIC_MODE_TOGGLE";
        inline static const std::string WIREFRAME_TOGGLE = "WIREFRAME_TOGGLE";

		inline static const std::string LIT_MODE_TOGGLE = "LIT_MODE_TOGGLE";
		inline static const std::string WIREFRAME_MODE_TOGGLE = "WIREFRAME_MODE_TOGGLE";

		inline static const std::string ON_WINDOW_DESTROY = "ON_WINDOW_DESTROY";
    };
}
