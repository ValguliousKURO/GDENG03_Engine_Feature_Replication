#include <DX3D/UI/DebugWindowUI.h>
#include <DX3D/EventBroadcasting/EventBroadcastManager.h>
#include <DX3D/EventBroadcasting/EventNames.h>
#include <DX3D/EventBroadcasting/Parameters.h>

dx3d::DebugWindowUI::DebugWindowUI(const BaseDesc& desc) : BaseUI(desc)
{
    EventBroadcastManager::getInstance().addObserver(
        EventNames::ON_DEBUG_LOG_ENTRY,
        [this](Parameters& params)
        {
            addLog(params.GetStringExtra("Message", ""));
        }
    );
}

dx3d::DebugWindowUI::~DebugWindowUI()
{
    EventBroadcastManager::getInstance().RemoveObserver(EventNames::ON_DEBUG_LOG_ENTRY);

}

void dx3d::DebugWindowUI::addLog(std::string entry)
{
    m_LogEntries.push_back(std::move(entry));
    m_ScrollToBottom = true;
}

void dx3d::DebugWindowUI::draw()
{
    if (m_ShowDebug)
    {
        if (ImGui::Begin("Debug Log", &m_ShowDebug))
        {
            ImGui::TextColored(
                ImVec4(1.0f, 0.55f, 0.25f, 1.0f), "Debug Log");
            ImGui::Separator();

            ImGui::BeginChild("LogEntries", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_HorizontalScrollbar);
            for (const std::string& entry : m_LogEntries)
            {
                ImGui::TextUnformatted(entry.c_str());
                ImGui::Separator();
            }

            if (m_ScrollToBottom)
            {
                ImGui::SetScrollHereY(1.0f);
                m_ScrollToBottom = false;
            }
            ImGui::EndChild();
        }
        ImGui::End();
    }
}
