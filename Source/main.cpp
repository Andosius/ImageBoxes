#include "Application.h"
#include "ApplicationLayer.h"

int main()
{
    auto app = IB::Application();
    app.PushLayer<ApplicationLayer>();
    app.Run();

    app.Close();
}