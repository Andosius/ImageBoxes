#pragma once


namespace IB
{
    class Layer
    {
    public:
        virtual ~Layer() = default;

        virtual void OnAttach() {}
        virtual void OnDetach() {}

        virtual void OnUpdate(float) {}
        virtual void OnUIRender() {}

    };
}