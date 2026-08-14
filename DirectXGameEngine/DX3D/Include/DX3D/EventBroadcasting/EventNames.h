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
		inline static const std::string ON_WINDOW_NEW = "ON_WINDOW_NEW";

		inline static const std::string ON_ADD_EMPTY_GAMEOBJECT = "ON_ADD_EMPTY_GAMEOBJECT";
		inline static const std::string ON_ADD_3D_OBJECT = "ON_ADD_3D_OBJECT";
		inline static const std::string ON_SPAWN_RIGID_BODY_CUBE_BATCH = "ON_SPAWN_RIGID_BODY_CUBE_BATCH";
		inline static const std::string ON_DELETE_GAMEOBJECT = "ON_DELETE_GAMEOBJECT";
		inline static const std::string ON_SET_GAMEOBJECT_ENABLED = "ON_SET_GAMEOBJECT_ENABLED";
		inline static const std::string ON_ADD_RIGID_BODY = "ON_ADD_RIGID_BODY";
		inline static const std::string ON_REMOVE_RIGID_BODY = "ON_REMOVE_RIGID_BODY";
		inline static const std::string ON_SET_PHYSICS_ENABLED = "ON_SET_PHYSICS_ENABLED";
		inline static const std::string ON_GAMEOBJECT_SELECTED = "ON_GAMEOBJECT_SELECTED";
		inline static const std::string ON_SET_PARENT = "ON_SET_PARENT";
		inline static const std::string ON_TRANSFORM_CHANGED = "ON_TRANSFORM_CHANGED";
		inline static const std::string ON_EDITOR_UNDO = "ON_EDITOR_UNDO";
		inline static const std::string ON_EDITOR_REDO = "ON_EDITOR_REDO";
		inline static const std::string ON_SCENE_SAVE = "ON_SCENE_SAVE";
		inline static const std::string ON_SCENE_SAVE_AS_NEW = "ON_SCENE_SAVE_AS_NEW";
		inline static const std::string ON_SCENE_LOAD = "ON_SCENE_LOAD";


		inline static const std::string ON_EDITOR_PLAY_MODE_CHANGED = "ON_EDITOR_PLAY_MODE_CHANGED";
		inline static const std::string ON_SCENE_PAUSE_STATE_CHANGED = "ON_SCENE_PAUSE_STATE_CHANGED";

		inline static const std::string ON_VIEWPORT_FOCUSED = "ON_VIEWPORT_FOCUSED";

		inline static const std::string ON_DEBUG_LOG_ENTRY = "ON_DEBUG_LOG_ENTRY";

		// for frame step
		inline static const std::string ON_SCENE_FRAME_STEP = "ON_SCENE_FRAME_STEP";

    };
}
