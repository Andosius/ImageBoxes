#pragma once

#include <string>
#include <cstdint>
#include <memory>
#include <vector>
#include <functional>

#include "imgui.h"

#include "Layer.h"

struct GLFWwindow;

namespace IB
{
    struct ApplicationSpecification
    {
        std::string Name = "Image Boxes";
        int Width = 1600;
        int Height = 900;
        bool ShowDefaultWindows = false;
        bool ShowDemoWindow = true;
        bool ShowAnotherWindow = true;
        ImVec4 Background = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    };

    class Application
    {
    public:
        Application(ApplicationSpecification  = ApplicationSpecification());
        ~Application();

        static Application& Get();
        GLFWwindow* GetWindowHandle() const { return m_WindowHandle; }

        template<typename T>
        void PushLayer()
        {
            static_assert(std::is_base_of<Layer, T>::value, "Pushed type is not subclass of Layer!");
            m_LayerStack.emplace_back(std::make_shared<T>())->OnAttach();
        }
        void PushLayer(const std::shared_ptr<Layer>& layer) { m_LayerStack.emplace_back(layer); layer->OnAttach(); }

        void Run();
        void Close();

    private:
        void Init();
        void Shutdown();

    private:
        ApplicationSpecification m_Specification;
        GLFWwindow* m_WindowHandle;
        bool m_Running;
        std::vector<std::shared_ptr<Layer>> m_LayerStack;

        float m_TimeStep = 0.0f;
        float m_FrameTime = 0.0f;
        float m_LastFrameTime = 0.0f;
    };
}