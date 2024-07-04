#include <string>
#include <iostream>
#include <cstddef>

#include "imgui.h"
#include "imgui_internal.h"
#include "nfd_glfw3.h"

#include "ApplicationLayer.h"
#include "ImageUtility.h"
#include "Application.h"


bool IsInsideWindow(ImVec2& mouse, ImVec2& win_pos, ImVec2& size);


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
    ImGui::ShowDemoWindow();

    // Image Start
    ImGui::SetNextWindowContentSize(ImVec2{ static_cast<float>(m_ImageWidth), static_cast<float>(m_ImageHeight) });
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Image", nullptr, ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_AlwaysVerticalScrollbar);
    ImGui::PopStyleVar();

    {
        if (m_Texture)
        {
            ImVec2 cursor = ImGui::GetCursorPos();
            
            ImGui::Image(reinterpret_cast<ImTextureID>(m_Texture), ImVec2(m_ImageWidth, m_ImageHeight));
            
            ImGui::SetCursorPos(cursor);
            ImGui::InvisibleButton("canvas", ImVec2(m_ImageWidth, m_ImageHeight), ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);

            ImRect rect = ImGui::GetCurrentWindow()->InnerRect;
            ImVec2 window_pos = ImGui::GetWindowPos();
            ImVec2 window_size = rect.GetSize();

            DrawRectangles(rect);

            ImGuiIO& io = ImGui::GetIO();

            if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left) && IsInsideWindow(io.MousePos, window_pos, window_size))
            {
                if (!m_IsDragging)
                {
                    m_IsDragging = true;
                }

                ImGui::GetForegroundDrawList()->AddRect(io.MouseClickedPos[0], io.MousePos, ImGui::GetColorU32(ImGuiCol_Button), 0.0f, ImDrawFlags_None, 2.0f);
            }
            else
            {
                if (m_IsDragging && IsInsideWindow(io.MousePos, window_pos, window_size) && !ImGui::IsKeyDown(ImGuiKey_Escape))
                {
                    CaptureRectangle();
                }
                else
                {
                    m_IsDragging = false;
                }
            }
        }
    }

    ImGui::End();
    // Image End

    // Workspace Start
    ImGui::Begin("Workspace", nullptr, ImGuiWindowFlags_None);

    {
        if (!m_Texture)
        {
            if (ImGui::Button("Open File"))
            {
                OpenImageFileDialog();
            }

            if (ImGui::Button("Test File (Linux)"))
            {
                LoadTextureFromFile("/home/andosius/Desktop/PDF2IMG/converted/2023_9_1.jpeg", &m_Texture, &m_ImageWidth, &m_ImageHeight);
            }
        }
        else // For future reference: std::iter_swap(v.begin() + first_target, v.begin() + second_target) swaps positions. Useful for realigning elements
        {
            for (std::vector<Rectangle>::iterator it = m_Selections.begin(); it != m_Selections.end();)
            {
                int idx = std::distance(m_Selections.begin(), it);
            
                ImGui::PushID(idx);

                ImGui::Text("Rectangle @((%.1f, %.1f), (%.1f, %.1f))", it->TopLeft.x, it->TopLeft.y, it->BottomRight.x, it->BottomRight.y);
                ImGui::SameLine();
                
                if (ImGui::Button("Delete"))
                {
                    it = m_Selections.erase(it);
                    ImGui::PopID();
                    continue;
                }

                ImGui::Separator();

                ImGui::PopID();

                it++;
            }
        }   
    } 

    ImGui::End();
    // Workspace End
}

void ApplicationLayer::DrawRectangles(ImRect& inner_rect)
{
    ImVec2 window_pos = ImGui::GetWindowPos();
    ImVec2 window_size = inner_rect.GetSize();

    float max_x = window_pos.x + window_size.x;
    float max_y = window_pos.y + window_size.y;

    for (const auto& rect : m_Selections)
    {
        // Reapply window position and scroll position to the relative coords
        ImVec2 win_pos = ImGui::GetCursorScreenPos();

        float x_scroll = ImGui::GetScrollX() / ImGui::GetScrollMaxX();
        float y_scroll = ImGui::GetScrollY() / ImGui::GetScrollMaxY();

        ImVec2 p1 = { std::min(rect.TopLeft.x + win_pos.x - x_scroll, max_x), std::min(rect.TopLeft.y + win_pos.y - y_scroll, max_y) };
        ImVec2 p2 = { std::min(rect.BottomRight.x + win_pos.x - x_scroll, max_x), std::min(rect.BottomRight.y + win_pos.y - y_scroll, max_y) };

        ImGui::GetForegroundDrawList()
            ->AddRectFilled(p1, p2, ImColor(255, 0, 255, 60));
    }
}

std::string ApplicationLayer::OpenImageFileDialog()
{
    std::string value = "";
    NFD_Init();

    nfdu8char_t* file_path;
    nfdu8filteritem_t filter[1] = {{"Image File", "jpg,jpeg,png"}};
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

bool IsInsideWindow(ImVec2& mouse, ImVec2& win_pos, ImVec2& size)
{
    return mouse.x >= win_pos.x && mouse.x <= win_pos.x + size.x &&
            mouse.y >= win_pos.y && mouse.y <= win_pos.y + size.y;
}