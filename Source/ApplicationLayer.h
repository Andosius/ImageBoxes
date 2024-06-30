#pragma once

#include <vector>
#include <string>

#include <GLFW/glfw3.h>

#include "Layer.h"


struct Rectangle
{
    ImVec2 p1;
    ImVec2 p2;
};


class ApplicationLayer : public IB::Layer
{
public:
    void OnAttach() override;
    void OnDetach() override;

    void OnUpdate(float ts) override;
    void OnUIRender() override;

private:
    std::string OpenImageFileDialog();
    void CaptureRectangle();

private:
    int m_ImageWidth = 0;
    int m_ImageHeight = 0;
    GLuint m_Texture = 0;

    bool m_IsDragging = false;
    ImVec2 m_MouseDownPosition = ImVec2(-FLT_MAX, -FLT_MAX);
    std::vector<Rectangle> m_Selections = std::vector<Rectangle>();
};