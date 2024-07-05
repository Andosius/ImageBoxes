#pragma once

#include <vector>
#include <string>

#include <GLFW/glfw3.h>
#include "imgui_internal.h"

#include "Layer.h"


struct Rectangle
{
    ImVec2 TopLeft;
    ImVec2 BottomRight;
};


class ApplicationLayer : public IB::Layer
{
public:
    void OnAttach() override;
    void OnDetach() override;

    void OnUpdate(float ts) override;
    void OnUIRender() override;

private:
    void DrawRectangles(ImRect& inner_rect);

    std::string OpenImageFileDialog();
    void CaptureRectangle();

private:
    int m_ImageWidth = 0;
    int m_ImageHeight = 0;

    GLuint m_Texture = 0;

    bool m_IsDragging = false;

    std::vector<Rectangle> m_Selections = std::vector<Rectangle>();
    std::size_t m_SelectedIdx = 0;
};