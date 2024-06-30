#include "imgui.h"
#include "imgui_internal.h"
#include "nfd_glfw3.h"

#include "ApplicationLayer.h"
#include "ImageUtility.h"
#include "Application.h"


bool IsScrollbarActive();


void ApplicationLayer::OnAttach()
{
}

void ApplicationLayer::OnDetach() 
{

}

void ApplicationLayer::OnUpdate(float ts)
{
    
}

void ApplicationLayer::OnUIRender() 
{
    ImGui::SetNextWindowContentSize(ImVec2{ static_cast<float>(m_ImageWidth), static_cast<float>(m_ImageHeight) });
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Image", NULL, ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_AlwaysVerticalScrollbar);
    ImGui::PopStyleVar();

    if (m_Texture)
    {
        ImGui::Image(reinterpret_cast<void*>(m_Texture), ImVec2(static_cast<float>(m_ImageWidth), static_cast<float>(m_ImageHeight)));

        for (const auto& rect : m_Selections)
        {
            // Reapply window position and scroll position to the relative coords
            ImVec2 win_pos = ImGui::GetCursorScreenPos();

            float x_scroll = ImGui::GetScrollX() / ImGui::GetScrollMaxX();
            float y_scroll = ImGui::GetScrollY() / ImGui::GetScrollMaxY();

            ImVec2 p1 = { rect.p1.x + win_pos.x - x_scroll, rect.p1.y + win_pos.y - y_scroll };
            ImVec2 p2 = { rect.p2.x + win_pos.x - x_scroll, rect.p2.y + win_pos.y - y_scroll };

            ImGui::GetForegroundDrawList()
                ->AddRectFilled(p1, p2, ImColor(255, 0, 255, 60));
        }

        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        {
            if (!m_IsDragging && !IsScrollbarActive())
                m_IsDragging = true;

            if (!IsScrollbarActive())
            {
                ImGuiIO& io = ImGui::GetIO();
                ImGui::GetForegroundDrawList()->AddRect(io.MouseClickedPos[0], io.MousePos, ImGui::GetColorU32(ImGuiCol_Button), 0.0f, ImDrawFlags_None, 2.0f);
            }
        }
        else
        {
            if (m_IsDragging)
            {
                CaptureRectangle();
            }
        }
    }
    ImGui::End();

    ImGui::Begin("Workspace");
    if (!m_Texture)
    {
        if (ImGui::Button("Open File"))
        {
            OpenImageFileDialog();
        }

        if (ImGui::Button("Last File"))
        {
            LoadTextureFromFile("/home/andosius/Desktop/PDF2IMG/converted/2023_9_1.jpeg", &m_Texture, &m_ImageWidth, &m_ImageHeight);
        }
    }
    else
    {
        ImGui::Text("Das hier wird bald aufgefüllt!");
    }
    ImGui::End();
}

std::string ApplicationLayer::OpenImageFileDialog()
{
    std::string value = "";
    NFD_Init();

    nfdu8char_t* file_path;
    nfdu8filteritem_t filter[1] = {{"JPEG Image File", "jpg,jpeg"}};
    nfdopendialogu8args_t args = {0};
    args.filterList = filter;
    args.filterCount = 1;
    NFD_GetNativeWindowFromGLFWWindow(IB::Application::Get().GetWindowHandle(), &args.parentWindow);
    nfdresult_t result = NFD_OpenDialogU8_With(&file_path, &args);

    if (result == NFD_OKAY)
    {
        value = std::string(file_path);
        LoadTextureFromFile(file_path, &m_Texture, &m_ImageWidth, &m_ImageHeight);
        NFD_FreePathU8(file_path);
    }

    NFD_Quit();

    return value;
}

void ApplicationLayer::CaptureRectangle()
{
    m_IsDragging = false;

    // Get mouse position and move position to window space
    ImVec2 mouse_pos = ImGui::GetMousePos();
    ImVec2 win_pos = ImGui::GetCursorScreenPos();

    ImVec2 target = {mouse_pos.x - win_pos.x, mouse_pos.y - win_pos.y};

    // Get the origin by removing the mouse difference
    ImVec2 move_mouse_diff = ImGui::GetMouseDragDelta(0);
    ImVec2 origin = {target.x - move_mouse_diff.x, target.y - move_mouse_diff.y};

    // Get top left and bottom right positions

    ImVec2 p1 = ImVec2(std::min(target.x, origin.x), std::min(target.y, origin.y));
    ImVec2 p2 = ImVec2(std::max(target.x, origin.x), std::max(target.y, origin.y));

    // Add the scroll to the points so we don't render on the very top only
    float x_scroll = ImGui::GetScrollX() / ImGui::GetScrollMaxX();
    float y_scroll = ImGui::GetScrollY() / ImGui::GetScrollMaxY();

    p1.x += x_scroll;
    p1.y += y_scroll;

    p2.x += x_scroll;
    p2.y += y_scroll;

    m_Selections.emplace_back(Rectangle{p1, p2});
}

bool IsScrollbarActive()
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    ImGuiID active_id = ImGui::GetActiveID();

    return
        active_id && (active_id == ImGui::GetWindowScrollbarID(window, ImGuiAxis_X) || 
        active_id == ImGui::GetWindowScrollbarID(window, ImGuiAxis_Y));
}
