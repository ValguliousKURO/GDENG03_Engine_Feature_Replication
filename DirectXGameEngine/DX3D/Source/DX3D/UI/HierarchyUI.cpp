#include <DX3D/UI/HierarchyUI.h>
#include <DX3D/Game/GameObject.h>
#include <DX3D/EventBroadcasting/EventBroadcastManager.h>
#include <DX3D/EventBroadcasting/EventNames.h>
#include <DX3D/EventBroadcasting/Parameters.h>

#include <DX3D/Core/DebugLogManager.h>
dx3d::HierarchyUI::HierarchyUI(const BaseDesc& desc) : BaseUI(desc)
{
    EventBroadcastManager::getInstance().addObserver(
        EventNames::ON_EDITOR_PLAY_MODE_CHANGED,
        [this](Parameters& params)
        {
            m_isPlayMode = params.GetBoolExtra("IsPlayMode", false);
        }
    );

    EventBroadcastManager::getInstance().addObserver(
        EventNames::ON_GAMEOBJECT_SELECTED,
        [this](Parameters& params)
        {
            m_selectedObject = params.GetGameObjectPtr("Selected", nullptr);
        }
    );
}

dx3d::HierarchyUI::~HierarchyUI()
{
    EventBroadcastManager::getInstance().RemoveObserver(EventNames::ON_EDITOR_PLAY_MODE_CHANGED);
    EventBroadcastManager::getInstance().RemoveObserver(EventNames::ON_GAMEOBJECT_SELECTED);
}

void dx3d::HierarchyUI::draw()
{
    if (m_showHierarchy)
    {
        if (ImGui::Begin("Hierarchy", &m_showHierarchy))
        {
            ImGui::TextColored(
                m_isPlayMode ? ImVec4(1.0f, 0.55f, 0.25f, 1.0f) : ImVec4(0.25f, 0.9f, 0.45f, 1.0f),
                m_isPlayMode ? "Mode: Play (scene editing locked)" : "Mode: Edit"
            );
            ImGui::Separator();

            ImGui::BeginDisabled(m_isPlayMode);

            // Add GameObject button
            if (ImGui::Button("Add New GameObject"))
            {
                ImGui::OpenPopup("GameObjectOptionsPopup");
            }
            if (ImGui::Button("Spawn 50 Rigid Body Cubes"))
            {
                Parameters param;
                param.PutExtra("Count", static_cast<size_t>(50));
                EventBroadcastManager::getInstance().postEvent(EventNames::ON_SPAWN_RIGID_BODY_CUBE_BATCH, param);
                DebugLogManager::getInstance().customLog("Spawned 50 rigid body cubes.");
            }

            if (ImGui::BeginPopup("GameObjectOptionsPopup"))
            {
                Parameters param;
                if (ImGui::MenuItem("Add Empty Game Object"))
                {
                    EventBroadcastManager::getInstance().postEvent(EventNames::ON_ADD_EMPTY_GAMEOBJECT);
                    DebugLogManager::getInstance().customLog("Added Empty GameObject.");
                }
                if (ImGui::MenuItem("Add Cube"))
                {
                    param.PutExtra("Key", "Cube");
                    EventBroadcastManager::getInstance().postEvent(EventNames::ON_ADD_3D_OBJECT, param);
					//DX3DLogInfo("Added Cube GameObject.")
					DebugLogManager::getInstance().customLog("Added Cube GameObject.");
                }
             
                if (ImGui::MenuItem("Add Sphere"))
                {
                    param.PutExtra("Key", "Sphere");
                    EventBroadcastManager::getInstance().postEvent(EventNames::ON_ADD_3D_OBJECT, param);
                }
                if (ImGui::MenuItem("Add Capsule"))
                {
                    param.PutExtra("Key", "Capsule");
                    EventBroadcastManager::getInstance().postEvent(EventNames::ON_ADD_3D_OBJECT, param);
                }
                if (ImGui::MenuItem("Add Cylinder"))
                {
                    param.PutExtra("Key", "Cylinder");
                    EventBroadcastManager::getInstance().postEvent(EventNames::ON_ADD_3D_OBJECT, param);
                }
                if (ImGui::MenuItem("Add Plane"))
                {
                    param.PutExtra("Key", "Plane");
                    EventBroadcastManager::getInstance().postEvent(EventNames::ON_ADD_3D_OBJECT, param);
                }
                if (ImGui::MenuItem("Add Point Light"))
                {
                    param.PutExtra("Key", "Point Light");
                    EventBroadcastManager::getInstance().postEvent(EventNames::ON_ADD_3D_OBJECT, param);
                    DebugLogManager::getInstance().customLog("Added Point Light GameObject.");
                }

                ImGui::Separator();
                if (ImGui::BeginMenu("Models"))
                {
                    if (!m_objModelNames || m_objModelNames->empty())
                    {
                        ImGui::MenuItem("No OBJ models found", nullptr, false, false);
                    }
                    else
                    {
                        for (const auto& modelName : *m_objModelNames)
                        {
                            if (ImGui::MenuItem(modelName.c_str()))
                            {
                                param.PutExtra("Key", "Obj");
                                param.PutExtra("ModelName", modelName);
                                EventBroadcastManager::getInstance().postEvent(EventNames::ON_ADD_3D_OBJECT, param);
                                DX3DLogInfo("Added OBJ GameObject: {}", modelName);
                            }
                        }
                    }
                    ImGui::EndMenu();
                }

                ImGui::Separator();
                if (ImGui::MenuItem("Add 50 Rigid Body Cubes"))
                {
                    param.PutExtra("Count", static_cast<size_t>(50));
                    EventBroadcastManager::getInstance().postEvent(EventNames::ON_SPAWN_RIGID_BODY_CUBE_BATCH, param);
                    DebugLogManager::getInstance().customLog("Spawned 50 rigid body cubes.");
                }
                ImGui::TextDisabled("Single-object rigid bodies are attached from the Inspector.");
                ImGui::EndPopup();
            }

            ImGui::EndDisabled();
            ImGui::Separator();

            // Make sure we actually have a list set
            if (!m_gameObjects || m_gameObjects->empty())
            {
                ImGui::Text("No game objects.");
                ImGui::End();
                return;
            }

            std::vector<GameObject*> rootObjects;
            for (auto& [key, vec] : *m_gameObjects) // note the * to dereference
            {
                for (auto& objPtr : vec)
                {
                    GameObject* obj = objPtr.get();
                    if (obj && !obj->isDeleted() && obj->getParent() == nullptr)
                    {
                        rootObjects.push_back(obj);
                    }
                }
            }

            for (GameObject* rootObj : rootObjects)
            {
                drawGameObjectNode(rootObj);
            }

            ImGui::Spacing();
            ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x, std::max(40.0f, ImGui::GetContentRegionAvail().y)));
            if (!m_isPlayMode && ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GAMEOBJECT_PTR"))
                {
                    GameObject* draggedObj = *(GameObject**)payload->Data;
                    if (draggedObj && draggedObj->getParent() != nullptr)
                    {
                        Parameters param;
                        param.PutExtra("Child", draggedObj);
                        param.PutExtra("Parent", static_cast<GameObject*>(nullptr));
                        EventBroadcastManager::getInstance().postEvent(EventNames::ON_SET_PARENT, param);
                    }
                }
                ImGui::EndDragDropTarget();
            }
        }
        ImGui::End();
    }
}

void dx3d::HierarchyUI::drawGameObjectNode(GameObject* obj)
{
    if (!obj || obj->isDeleted()) return;

    ImGui::PushID(obj);

    std::vector<GameObject*> validChildren;
    for (GameObject* child : obj->getChildren())
    {
        if (child && !child->isDeleted())
        {
            validChildren.push_back(child);
        }
    }

    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (m_selectedObject == obj)
    {
        nodeFlags |= ImGuiTreeNodeFlags_Selected;
    }
    if (validChildren.empty())
    {
        nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    }

    bool enabled = obj->isEnabled();
    ImGui::BeginDisabled(m_isPlayMode);
    if (ImGui::Checkbox("##Enabled", &enabled))
    {
        Parameters param;
        param.PutExtra("Target", obj);
        param.PutExtra("Enabled", enabled);
        EventBroadcastManager::getInstance().postEvent(EventNames::ON_SET_GAMEOBJECT_ENABLED, param);
    }
    ImGui::EndDisabled();

    ImGui::SameLine();

    bool isNodeOpen = ImGui::TreeNodeEx((void*)obj, nodeFlags, "%s", obj->getName().c_str());

    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
    {
        Parameters param;
        param.PutExtra("Selected", obj);
        EventBroadcastManager::getInstance().postEvent(EventNames::ON_GAMEOBJECT_SELECTED, param);
    }

    if (!m_isPlayMode && ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
    {
        ImGui::SetDragDropPayload("GAMEOBJECT_PTR", &obj, sizeof(GameObject*));
        ImGui::Text("Reparent %s", obj->getName().c_str());
        ImGui::EndDragDropSource();
    }

    if (!m_isPlayMode && ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GAMEOBJECT_PTR"))
        {
            GameObject* draggedObj = *(GameObject**)payload->Data;
            if (draggedObj && draggedObj != obj && !obj->isDescendantOf(draggedObj))
            {
                Parameters param;
                param.PutExtra("Child", draggedObj);
                param.PutExtra("Parent", obj);
                EventBroadcastManager::getInstance().postEvent(EventNames::ON_SET_PARENT, param);
            }
        }
        ImGui::EndDragDropTarget();
    }

    ImGui::SameLine(ImGui::GetWindowWidth() - 60.0f);
    ImGui::BeginDisabled(m_isPlayMode);
    if (ImGui::SmallButton("Delete"))
    {
        Parameters param;
        param.PutExtra("Target", obj);
        EventBroadcastManager::getInstance().postEvent(EventNames::ON_DELETE_GAMEOBJECT, param);
    }
    ImGui::EndDisabled();

    if (isNodeOpen && !validChildren.empty())
    {
        for (GameObject* child : validChildren)
        {
            drawGameObjectNode(child);
        }
        ImGui::TreePop();
    }

    ImGui::PopID();
}

void dx3d::HierarchyUI::setGameObjectList(const std::unordered_map<size_t, std::vector<UniquePtr<GameObject>>>* list)
{
    m_gameObjects = list;
}

void dx3d::HierarchyUI::setObjModelNames(const std::vector<std::string>* modelNames)
{
    m_objModelNames = modelNames;
}

