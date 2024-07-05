#include <string>
#include <iostream>
#include <cstddef>

#include "imgui.h"
#include "imgui_internal.h"
#include "nfd.h"
#include "fmt/format.h"

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

void ApplicationLayer::OnUpdate(float)
{
}

void ApplicationLayer::OnUIRender()
{
    ImGui::ShowDemoWindow();

    ImVec2 img_size(static_cast<float>(m_ImageWidth), static_cast<float>(m_ImageHeight));

    // Image Start
    ImGui::SetNextWindowContentSize(img_size);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Image", nullptr, ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_AlwaysVerticalScrollbar);
    ImGui::PopStyleVar();

    {
        if (m_Texture)
        {
            ImVec2 cursor = ImGui::GetCursorPos();

            ImGui::Image(reinterpret_cast<void*>(m_Texture), img_size);

            ImGui::SetCursorPos(cursor);
            ImGui::InvisibleButton("canvas", img_size, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);

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

            if (ImGui::Button("Test File (Windows)"))
            {
                LoadTextureFromFile("C:\\Users\\Ando\\Desktop\\StudentNTP_Ben-McCarty_x1280.jpg", &m_Texture, &m_ImageWidth, &m_ImageHeight);
            }
        }
        else
        {
            if (ImGui::TreeNodeEx("Markierte Stellen", ImGuiTreeNodeFlags_DefaultOpen))
            {
                for (std::size_t idx = 0; idx < m_Selections.size(); idx++)
                {
                    Rectangle& rectangle = m_Selections[idx];

                    // Add selectable element
                    std::string text = fmt::format("(({:1f}, {:1f}), ({:1f}, {:1f}))", rectangle.TopLeft.x, rectangle.TopLeft.y, rectangle.BottomRight.x, rectangle.BottomRight.y);
                    
                    if (ImGui::Selectable(text.c_str(), m_SelectedIdx == idx))
                    {
                        m_SelectedIdx = idx;
                    }

                    // Check for movement
                    if (ImGui::IsItemActive() && !ImGui::IsItemHovered())
                    {
                        float delta_y = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).y;

                        if (delta_y != 0.0f)
                        {
                            std::size_t next = idx + (delta_y < 0.0f ? -1 : 1);

                            if (next >= 0 && next < m_Selections.size())
                            {
                                Rectangle r = std::move(m_Selections[idx]);

                                m_Selections.erase(m_Selections.begin() + idx);
                                m_Selections.insert(m_Selections.begin() + next, std::move(r));

                                // Reset mouse delta so we can drag over multiple entries
                                ImGui::ResetMouseDragDelta();
                            }
                        }
                    }
                }

                ImGui::TreePop();
            }

            // Check for active entities we plan to delete and erase them from the vector
            if (ImGui::IsKeyDown(ImGuiKey_Delete) && m_SelectedIdx != m_Selections.size())
            {
                m_Selections.erase(m_Selections.begin() + m_SelectedIdx);
                m_SelectedIdx = m_Selections.size();
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

    for (std::size_t i = 0; i < m_Selections.size(); i++)
    {
        const auto& rect = m_Selections[i];
        // Reapply window position and scroll position to the relative coords
        ImVec2 win_pos = ImGui::GetCursorScreenPos();

        float x_scroll = ImGui::GetScrollX() / ImGui::GetScrollMaxX();
        float y_scroll = ImGui::GetScrollY() / ImGui::GetScrollMaxY();

        ImVec2 p1 = { std::min(rect.TopLeft.x + win_pos.x - x_scroll, max_x), std::min(rect.TopLeft.y + win_pos.y - y_scroll, max_y) };
        ImVec2 p2 = { std::min(rect.BottomRight.x + win_pos.x - x_scroll, max_x), std::min(rect.BottomRight.y + win_pos.y - y_scroll, max_y) };

        if (i == m_SelectedIdx)
        {
            ImGui::GetForegroundDrawList()
                ->AddRectFilled(p1, p2, ImColor(255, 0, 255, 60));
        }
        else
        {
            ImGui::GetForegroundDrawList()
                ->AddRectFilled(p1, p2, ImColor(143, 143, 143, 160));
        }
    }
}

std::string ApplicationLayer::OpenImageFileDialog()
{
    std::string value = "";
    NFD_Init();

    nfdu8char_t* file_path;
    nfdu8filteritem_t filter[1] = { {"Image File", "jpg,jpeg,png"} };
    nfdopendialogu8args_t args = { 0 };
    args.filterList = filter;
    args.filterCount = 1;
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

    ImVec2 target = { mouse_pos.x - win_pos.x, mouse_pos.y - win_pos.y };

    // Get the origin by removing the mouse difference
    ImVec2 move_mouse_diff = ImGui::GetMouseDragDelta(0);
    ImVec2 origin = { target.x - move_mouse_diff.x, target.y - move_mouse_diff.y };

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

    m_Selections.emplace_back(Rectangle{ p1, p2 });

    m_SelectedIdx = m_Selections.size();
}

bool IsInsideWindow(ImVec2& mouse, ImVec2& win_pos, ImVec2& size)
{
    return mouse.x >= win_pos.x && mouse.x <= win_pos.x + size.x &&
        mouse.y >= win_pos.y && mouse.y <= win_pos.y + size.y;
}